
#include "ForceBlendHCI.H"
#include <G3DOperators.h>
#include <ConfigVal.H>
#include "BrushState.H"
using namespace G3D;
namespace DrawOnAir {


/*
 each ruling lies within a plane defined by the normal and the side vec where 
 side vec = normal x tangent.  

 within that plane the ruling is described by the equation p(t) = a t^2 where
 a's value is linked to the position of the left hand.

 for haptics, try line constraints probably in the direction perpendicular to the
 rulings, but to start just use the rulings themselves.

 to draw for gfx and haptics, turn into polylines created by regularly sampling
 the p(t) function.

 as the pen hand moves over the surface want to fix the points that it moves over,
 additionally, may want to move the 0 of p(t) out to the point that was jsut fixed.
 
 also, need to blend the effect of fixing that position in space out over space
 along the Mark.  maybe have a 2nd array that stores a number based on how movable
 a point is.  then, each point needs a default location and a value for how easy
 it is to move it from that default location.  when you move right over a point,
 you update its default location and fix it.  you update the default locations for
 neighboring points as well, but don't completely fix them.


TODO: 
 - work out details of fixing the positions in space and how that updates as you
   start smudging
 - transfer to real marks, deal with the focal point issue

*/


ForceBlendHCI::ForceBlendHCI(
    Array<std::string>     brushOnTriggers,
    Array<std::string>     brushMotionTriggers, 
    Array<std::string>     brushOffTriggers,
    Array<std::string>     handMotionTriggers,
    BrushRef               brush,
    CavePaintingCursorsRef cursors,
    //ForceNetInterface*     forceNetInterface,
  MinVR::EventMgrRef            eventMgr,
  MinVR::GfxMgrRef              gfxMgr)
{
  _brush = brush;
  _cursors = cursors;
 // _forceNetInterface = forceNetInterface;
  _eventMgr = eventMgr;
  _gfxMgr = gfxMgr;
  _enabled = false;

  _fsa = new MinVR::Fsa("ForceBlendHCIHCI");
  _fsa->addState("Start");
  _fsa->addState("Drawing");
  _fsa->addArc("BrushMove", "Start", "Start", brushMotionTriggers);
  _fsa->addArcCallback("BrushMove", this, &ForceBlendHCI::brushMotion);
  _fsa->addArc("BrushPMove", "Start", "Start", MinVR::splitStringIntoArray("Brush_Physical_Tracker"));
  _fsa->addArcCallback("BrushPMove", this, &ForceBlendHCI::brushPhysicalFrameChange);
  _fsa->addArc("BrushOn", "Start", "Drawing", brushOnTriggers);
  _fsa->addArcCallback("BrushOn", this, &ForceBlendHCI::brushOn);
  _fsa->addArc("BrushDrawMove", "Drawing", "Drawing", brushMotionTriggers);
  _fsa->addArcCallback("BrushDrawMove", this, &ForceBlendHCI::brushDrawMotion);
  _fsa->addArc("BrushDrawPMove", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_Physical_Tracker"));
  _fsa->addArcCallback("BrushDrawPMove", this, &ForceBlendHCI::brushPhysicalFrameChange);
  _fsa->addArc("BrushPressure", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure", this, &ForceBlendHCI::brushPressureChange);
  _fsa->addArc("BrushOff", "Drawing", "Start", brushOffTriggers);
  _fsa->addArcCallback("BrushOff", this, &ForceBlendHCI::brushOff);
  _fsa->addArc("HandMove", "Start", "Start", handMotionTriggers);
  _fsa->addArcCallback("HandMove", this, &ForceBlendHCI::handMotion);
  _fsa->addArc("HandDrawMove", "Drawing", "Drawing", handMotionTriggers);
  _fsa->addArcCallback("HandDrawMove", this, &ForceBlendHCI::handMotion);
  _fsa->addArc("ToggleGuide", "Start", "Start", MinVR::splitStringIntoArray("kbd_SPACE_down"));
  _fsa->addArcCallback("ToggleGuide", this, &ForceBlendHCI::toggleGuide);

  setup();
}

ForceBlendHCI::~ForceBlendHCI()
{
}


void
ForceBlendHCI::setup()
{
  _guideOn = true;
  _pts.clear();
  _fixed.clear();

  Vector3 lineStart(-0.4, 0, 0.05);
  Vector3 lineStop(0.4, 0, 0.05);
  double alpha = 0.5;

  int n = (lineStop - lineStart).magnitude() / MinVR::ConfigVal("ForceBlendHCI_Spacing",0.01,false);

  for (int j=0;j<n;j++) {
    Array<Vector3> p;
    Array<Vector3> d;
    Array<double> f;
    double a = (double)j/n;
    for (double t=0;t<1.0;t+=0.005) {
      Vector3 pt;
      pt[0] = lineStart.lerp(lineStop, a)[0];
      pt[1] = alpha*t*t;
      pt[2] = t;
      p.append(pt);
      d.append(pt);
      f.append(0.0);
    }
    _pts.append(p);
    _defaultPos.append(d);
    _fixed.append(f);
  }
}   

void
ForceBlendHCI::updateRulings()
{
  // Update the rulings based on hand movement

  // find closest point on line to the hand pos
  //Vector3 closest = G3D::LineSegment::fromTwoPoints(lineStart, lineStop).closestPoint(_handPos);
  
  // find the ruling that the hand position belongs to, in this simple case, the
  // ruling is defined by the x = _handPos[0] plane

  // find hand pos w/in the frame of this plane
  Vector2 handPos2D(_handPos[2], _handPos[1]);

  // solve for alpha, alpha = y/x^2
  double alpha = handPos2D[1] / (handPos2D[0] * handPos2D[0]);

  cout << "alpha = " << alpha << endl;

  // update point positions based on new alpha
  for (int j=0;j<_pts.size();j++) {
    Array<Vector3> &ruling = _pts[j];
    double a = (double)j/_pts[j].size();
    for (int i=0;i<ruling.size();i++) {
      double t = (double)i/(double)ruling.size();
      ruling[i][1] = alpha*t*t;
      if (_fixed[j][i] == 0.0) {
        _defaultPos[j][i] = ruling[i];
      }
      else {
        ruling[i] = ruling[i].lerp(_defaultPos[j][i], _fixed[j][i]);
      }
    }
  }
}

void
ForceBlendHCI::updateWeights(Vector3 brushPos)
{
  // Update the fixed array based on brush movement over the surface

  for (int i=0;i<_pts.size();i++) {
    for (int j=0;j<_pts[i].size();j++) {
      if (_fixed[i][j] < 1.0) {
        double d = (brushPos - _pts[i][j]).magnitude();
        const double r = 0.15;
        double f = 1.0 - d/r;
        if (f > 0) {
          _fixed[i][j] = f;
        }
      }    
    }
  }

  // TODO: may need to set weights that are close to the original line, but may
  // not if you have to get pigment from the line anyway.
}



void
ForceBlendHCI::setEnabled(bool b) 
{
  if ((b) && (!_enabled)) {
    _eventMgr->addFsaRef(_fsa);
    _cursors->setBrushCursor(CavePaintingCursors::DEFAULT_CURSOR);
    _brush->state->maxPressure = MinVR::ConfigVal("ForceBlendHCI_MaxPressure", 0.0, false);
    _brush->state->brushInterface = "ForceBlendHCI";
    _poseID = _gfxMgr->addPoseCallback(this, &ForceBlendHCI::pose);
    _drawID = _gfxMgr->addDrawCallback(this, &ForceBlendHCI::draw);
  }
  else if ((!b) && (_enabled)) {
    _eventMgr->removeFsaRef(_fsa);
    _gfxMgr->removePoseCallback(_poseID);
    _gfxMgr->removeDrawCallback(_drawID);
  }
  _enabled = b;

  if (_enabled) {
    setup();
  }
}

bool
ForceBlendHCI::canReleaseFocus()
{
  return (_fsa->getCurrentState() == "Start");
}


void
ForceBlendHCI::toggleGuide(MinVR::EventRef e)
{
  _guideOn = !_guideOn;
}

void 
ForceBlendHCI::brushMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
}

void 
ForceBlendHCI::brushOn(MinVR::EventRef e)
{
  //_brush->startNewMark();
}

void 
ForceBlendHCI::brushPressureChange(MinVR::EventRef e)
{
  /**
  _brush->state->pressure = e->get1DData();
  //cout << "new pressure " << _brush->state->pressure << endl;

  // For this style of controlling the brush, pressure changes both the width and the color 
  // value of the mark.
  if (_brush->state->maxPressure != 0.0) {
    _brush->state->width = _brush->state->pressure/_brush->state->maxPressure * _brush->state->size;
    _brush->state->colorValue = _brush->state->pressure/_brush->state->maxPressure;
  }
  **/
}

void
ForceBlendHCI::brushPhysicalFrameChange(MinVR::EventRef e)
{
  _brush->state->physicalFrameInRoomSpace = e->getCoordinateFrameData();
}

void 
ForceBlendHCI::brushDrawMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  updateWeights(e->getCoordinateFrameData().translation);
  
  //_brush->addSampleToMark();
/**
  double rad = ConfigVal("CavePaintingCursors_BlendToolSize",0.05,false);
  Sphere s(e->getCoordinateFrameData().translation, rad);
  for (int i=0;i<_pts.size();i++) {
    Array<Vector3> &p = _pts[i];
    for (int j=0;j<p.size();j++) {
      if ((s.contains(p[j])) && (_alphas[i][j] < 1.0)) {
        _alphas[i][j] += 0.05;
        _fixed[i][j] = true;
      }
    }
  }
  **/
}

void
ForceBlendHCI::brushOff(MinVR::EventRef e)
{
  //_brush->endMark();
}

void
ForceBlendHCI::handMotion(MinVR::EventRef e)
{
  _handPos = e->getCoordinateFrameData().translation;
  updateRulings();
}


void
ForceBlendHCI::pose(Array<PosedModel::Ref>& posedModels, const CoordinateFrame& frame)
{
  if (_fsa->getCurrentState() != "Start")
    return;

  // Update position based on hand motion


  /**
  if (_forceNetInterface != NULL) {

    for (int i=1;i<_pts.size();i++) {
      Array<Vector3> verts;
      Array<Vector3> &left = _pts[i-1];
      Array<Vector3> &right = _pts[i];
      double tleft = (double)(i-1)/(double)_pts.size();
      double tright = (double)i/(double)_pts.size();
      for (int j=0;j<left.size();j++) {
        verts.append(right[j]);
        verts.append(left[j]);
      }
      _forceNetInterface->setGeometry(i, PrimitiveType::TRIANGLE_STRIP, verts);
    }

  }
  **/  
}


void
ForceBlendHCI::draw(RenderDevice *rd, const CoordinateFrame& frame)
{
  rd->pushState();

  rd->setCullFace(RenderDevice::CULL_NONE);
  rd->setDepthWrite(false);

  /**
  rd->disableLighting();
  rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
  rd->setTexture(0, _gfxMgr->getTexture("ForceBlendTex"));
  for (int i=1;i<_pts.size();i++) {
    Array<Vector3> &left = _pts[i-1];
    Array<Vector3> &right = _pts[i];
    rd->beginPrimitive(PrimitiveType::TRIANGLE_STRIP);
    double tleft = (double)(i-1)/(double)_pts.size();
    double tright = (double)i/(double)_pts.size();
    for (int j=0;j<left.size();j++) {
      double s = (double)j/(double)left.size();
      rd->setTexCoord(0, Vector2(s, tleft)); 
      rd->setColor(Color4(1,1,1,_alphas[i-1][j]));
      rd->sendVertex(left[j]);

      rd->setTexCoord(0, Vector2(s, tright)); 
      rd->setColor(Color4(1,1,1,_alphas[i][j]));
      rd->sendVertex(right[j]);
    }
    rd->endPrimitive();
  }
  **/

  if (_guideOn) {
    rd->setRenderMode(RenderDevice::RENDER_WIREFRAME);
    rd->setColor(Color4(0.61,0.72,0.92,0.33));
    for (int i=1;i<_pts.size();i++) {
      Array<Vector3> &left = _pts[i-1];
      Array<Vector3> &right = _pts[i];
      rd->beginPrimitive(PrimitiveType::TRIANGLE_STRIP);
      for (int j=0;j<left.size();j++) {
        rd->sendVertex(left[j]);
        rd->sendVertex(right[j]);
      }
      rd->endPrimitive();
    }
  }

  /**
  rd->setPointSize(5.0);
  rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
  rd->beginPrimitive(PrimitiveType::POINTS);
  for (int i=0;i<_pts.size();i++) {
    Array<Vector3> &p = _pts[i];
    for (int j=0;j<p.size();j++) {
      rd->setColor(Color4(0,0,0,_alphas[i][j]));
      rd->sendVertex(p[j]);
    }
  }
  rd->endPrimitive();
  **/

  rd->setLineWidth(10.0);
  rd->setColor(Color3::black());
  rd->beginPrimitive(PrimitiveType::LINES);
  rd->sendVertex(_pts[0][0]);
  rd->sendVertex(_pts[_pts.size()-1][0]);
  rd->endPrimitive();

  rd->popState();
}



} // end namespace

