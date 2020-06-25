
#include <ConfigVal.H>
#include <SynchedSystem.h>
#include "ColorPicker.H"
using namespace G3D;
#define FADE_TIME 1.0

namespace DrawOnAir {

ColorPicker::ColorPicker(GfxMgrRef              gfxMgr,
                         EventMgrRef            eventMgr,
                         HCIMgrRef              hciMgr,
                         CavePaintingCursorsRef cursors,
                         Array<std::string>     trackerEvents,
                         Array<std::string>     btnEvents) : WidgetHCI(hciMgr)
{
  _gfxMgr = gfxMgr;
  _eventMgr = eventMgr;
  CP_GEOM_RAD = MinVR::ConfigVal("ColorPicker_Radius",1.0,false);
  CANCEL_GEOM_RAD = 1.5 * CP_GEOM_RAD;

  _fsa = new Fsa("ColorPicker");
  _fsa->addState("Start");
  _fsa->addArc("PickBtnPressed", "Start", "Start", btnEvents);
  _fsa->addArc("TrackerMoved", "Start", "Start", trackerEvents);
  _fsa->addArcCallback("PickBtnPressed", this, &ColorPicker::pickBtnPressed);
  _fsa->addArcCallback("TrackerMoved", this, &ColorPicker::trackerMoved);
}

ColorPicker::~ColorPicker()
{
}


void
ColorPicker::activate()
{
  _widgetCenterInRoom = _initialTrackerPos - colorToPoint(_initialColor);
  _frame = CoordinateFrame(_widgetCenterInRoom);
  _startTime = MinVR::SynchedSystem::getLocalTime();
  _startFadeOutTime = -1;
  _dcbid = _gfxMgr->addDrawCallback(this, &ColorPicker::draw);
  _eventMgr->addFsaRef(_fsa);  
}

void
ColorPicker::deactivate()
{
  releaseFocusWithHCIMgr();
  _eventMgr->removeFsaRef(_fsa);
}

bool
ColorPicker::outsideCancelSphere()
{
  return ((_trackerPos - _widgetCenterInRoom).magnitude() > CANCEL_GEOM_RAD);
}

void
ColorPicker::trackerMoved(MinVR::EventRef e)
{
  // first time through take the first tracker we encounter
  if (_activeTracker == "")
    _activeTracker = e->getName();
  // if this tracker is inside the widget and the other one is outside, then
  // switch to using this one
  else if ((_activeTracker != e->getName()) && (outsideCancelSphere()))
    _activeTracker = e->getName();

  if (e->getName() == _activeTracker) {
    _trackerPos = e->getCoordinateFrameData().translation;
    if (outsideCancelSphere()) {
      _eventMgr->queueEvent(new MinVR::VRG3DEvent("ColorPicker_ColorModified",
          Vector3(_initialColor[0],_initialColor[1],_initialColor[2])));
    }
    else {
      Color3 c = pointToColor(_trackerPos - _widgetCenterInRoom);
      _eventMgr->queueEvent(new MinVR::VRG3DEvent("ColorPicker_ColorModified",
                                   Vector3(c[0], c[1], c[2])));
    }
  }
}

void
ColorPicker::pickBtnPressed(MinVR::EventRef e)
{
  if (outsideCancelSphere()) {
    _eventMgr->queueEvent(new MinVR::VRG3DEvent("ColorPicker_ColorModified",
      Vector3(_initialColor[0],_initialColor[1],_initialColor[2])));
    _eventMgr->queueEvent(new MinVR::VRG3DEvent("ColorPicker_Cancel"));
  }
  else {
    Color3 c = pointToColor(_trackerPos - _widgetCenterInRoom);
    _eventMgr->queueEvent(new MinVR::VRG3DEvent("ColorPicker_ColorSelected",
                                 Vector3(c[0], c[1], c[2])));
  }
  _startFadeOutTime = MinVR::SynchedSystem::getLocalTime();
  deactivate();
}


void
ColorPicker::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  rd->pushState();
  rd->setTexture(0, NULL);
  
  float alpha = 1.0;
  double displayedTime = MinVR::SynchedSystem::getLocalTime() - _startTime;
  if (displayedTime < FADE_TIME) {
    alpha = (displayedTime / FADE_TIME);
    rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA,
		     RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
  }
  else if ((_startFadeOutTime != -1) && (MinVR::SynchedSystem::getLocalTime() > _startFadeOutTime)) {
    double fadedTime = MinVR::SynchedSystem::getLocalTime() - _startFadeOutTime;
    alpha = 1.0 - (fadedTime / FADE_TIME);
    if (alpha <= 0.0) {
      rd->popState();
      _gfxMgr->removeDrawCallback(_dcbid);    
      return;
    }
    rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA,
		     RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);    
  }

  rd->disableLighting();

  Color4 cursorColor = 
    Color4(pointToColor(_trackerPos - _widgetCenterInRoom),alpha);
  Color4 cursorOutline = Color4(1,1,1,alpha);

  if (outsideCancelSphere()) {
    cursorColor   = Color4::clear();
    Draw::sphere(Sphere(Vector3(0,0,0), CANCEL_GEOM_RAD), rd,
		 Color4(1,1,1,0.15), Color4(0,0,0,0.35));
  }
  Draw::sphere(Sphere(Vector3(_trackerPos - _widgetCenterInRoom), 
		      CP_GEOM_RAD / 10.0), rd, cursorColor, cursorOutline);

  double ainc = toRadians(3.0);
  double yinc = CP_GEOM_RAD / 1800.0;

  rd->beginPrimitive(PrimitiveType::LINE_STRIP);

  double a = 0.0;
  double y = -CP_GEOM_RAD;
  while (y < CP_GEOM_RAD) {
    double r;
    if (y < 0)
      r = CP_GEOM_RAD * (y + CP_GEOM_RAD)/CP_GEOM_RAD;
    else
      r = CP_GEOM_RAD * (CP_GEOM_RAD-y)/CP_GEOM_RAD;
    Vector3 p = Vector3(r*cos(a), y, r*sin(a));
    Color4 c = Color4(pointToColor(p), alpha);
    
    rd->setColor(c);
    rd->sendVertex(p);

    a += ainc;
    y += yinc;
  }

  rd->endPrimitive();

  rd->beginPrimitive(PrimitiveType::LINE_STRIP);
  rd->setColor(Color4(Color3::white(),alpha));
  rd->sendVertex(Vector3(0,CP_GEOM_RAD,0));
  rd->setColor(Color4(Color3::black(),alpha));
  rd->sendVertex(Vector3(0,-CP_GEOM_RAD,0));
  rd->endPrimitive();

  rd->enableLighting();
  rd->setBlendFunc(RenderDevice::BLEND_ONE,RenderDevice::BLEND_ZERO);
  rd->popState();
}

Vector3
ColorPicker::colorToPoint(Color3 c)
{
  // Convert RGB to HLS
  float h,l,s;
  float maximum = G3D::max(G3D::max(c[0],c[1]),c[2]);
  float minimum = G3D::min(G3D::min(c[0],c[1]),c[2]);
  l = (maximum + minimum)/2;
  if (maximum == minimum) {
    s=0;
    h=0;
  } 
  else {
    if (l <= 0.5)
      s=(maximum-minimum)/(maximum+minimum);
    else
      s=(maximum-minimum)/(2.0-maximum-minimum);
    float delta=maximum-minimum;
    
    if (c[0]==maximum )
      h=(c[1]-c[2])/delta;
    else if ( c[1]==maximum )
      h=2.0+(c[2]-c[0])/delta;
    else if ( c[2]==maximum )
      h=4+(c[0]-c[1])/delta;
    h*=60.0;
    if ( h<0.0 ) 
      h+=360.0;
  }
  
  // Convert HLS to a point in the color widget space
  double a = toRadians(h);
  double maxr;
  if (l < 0.5)
    maxr = CP_GEOM_RAD * l*2.0;
  else
    maxr = CP_GEOM_RAD * (1.0 - ((l-0.5)*2.0));
  Vector3 p;
  p[0] = maxr*s*cos(a);
  p[1] = l*2.0*CP_GEOM_RAD - CP_GEOM_RAD;
  p[2] = maxr*s*sin(a);

  return p;
}


Color3
ColorPicker::pointToColor(Vector3 p)
{
  // Clamp offset vec so it doesn't go outside of the color space  
  Vector3 offset = p;
  float l1 = fabs(offset[1]);
  float l2 = sqrt(offset[0]*offset[0]+offset[2]*offset[2]);
  float a = atan2(l1,l2);
  float b = 2.3561945 - a;
  float d = CP_GEOM_RAD * 0.70710678 / sin(b);
  if (offset.magnitude() > d) {
    offset = offset.unit() * d; 
    p = offset;
  }

  //  Determine HLS values
  double h,l,s;
  l = (offset[1] + CP_GEOM_RAD)/(2.0*CP_GEOM_RAD);
  Vector3 v = (Vector3(offset[0],0,offset[2])).unit();
  float dot = v.dot(Vector3(1,0,0));
  if (offset[2] > 0)
	h = toDegrees(G3D::aCos(dot));
  else
	h = 360 - toDegrees(G3D::aCos(dot));

  if (offset[1] > CP_GEOM_RAD)
    s = 0.0;
  else if (offset[1] < -CP_GEOM_RAD)
    s = 1.0;
  else
    s = sqrt(offset[0]*offset[0] + offset[2]*offset[2]) /
      (CP_GEOM_RAD - fabs(offset[1]));
  if (s > 1.0)
    s = 1.0;

  // Convert HLS to RGB
  double m1, m2;
  m2 = (l<=0.5)?(l*(1.0+s)):(l+s-l*s);
  m1 = 2.0*l-m2;

  Color3 color;
  if (s==0)
    color = Color3(l,l,l);
  else {
    color = Color3(findValue(m1,m2,h+120.0),
		   findValue(m1,m2,h),
		   findValue(m1,m2,h-120.0));
  }
  return color;
}


double
ColorPicker::findValue(double n1, double n2, double hue)
{
  if(hue>360.0)
    hue-=360.0;
  else if (hue<0)
    hue+=360.0;
  
  if(hue<60)
    return n1+(n2-n1)*hue/60.0;
  else if (hue<180.0)
    return n2;
  else if (hue<240.0)
    return n1+(n2-n1)*(240.0-hue)/60.0;
  else
    return n1;
}




} // end namespace

