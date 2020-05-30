#include "ForceMovingPlaneDrawHCI.H"
#include <G3DOperators.h>
#include <ConfigVal.H>

using namespace G3D;
namespace DrawOnAir {

ForceMovingPlaneDrawHCI::ForceMovingPlaneDrawHCI(Array<std::string>     brushOnTriggers,
                                   Array<std::string>     brushMotionTriggers,
                                   Array<std::string>     brushOffTriggers,
                                   Array<std::string>     handMotionTriggers,
                                   BrushRef          brush,
                                   CavePaintingCursorsRef cursors,
                                   //ForceNetInterface*     forceNetInterface,
  EventMgrRef            eventMgr,
  GfxMgrRef              gfxMgr)
{
  _brush = brush;
  _cursors = cursors;
  _eventMgr = eventMgr;
  _gfxMgr = gfxMgr;
  //_forceNetInterface = forceNetInterface;
  _enabled = false;

  _fsa = new Fsa("ForceMovingPlaneDrawHCI");
  _fsa->addState("Start");
  _fsa->addState("Drawing");
  _fsa->addArc("BrushMove", "Start", "Start", brushMotionTriggers);
  _fsa->addArcCallback("BrushMove", this, &ForceMovingPlaneDrawHCI::brushMotion);
  _fsa->addArc("BrushPMove", "Start", "Start", MinVR::splitStringIntoArray("Brush_Physical_Tracker"));
  _fsa->addArcCallback("BrushPMove", this, &ForceMovingPlaneDrawHCI::brushPhysicalFrameChange);
  _fsa->addArc("HandMove", "Start", "Start", handMotionTriggers);
  _fsa->addArcCallback("HandMove", this, &ForceMovingPlaneDrawHCI::handMotion);
  _fsa->addArc("BrushOn", "Start", "Drawing", brushOnTriggers);
  _fsa->addArcCallback("BrushOn", this, &ForceMovingPlaneDrawHCI::brushOn);
  _fsa->addArc("BrushDrawMove", "Drawing", "Drawing", brushMotionTriggers);
  _fsa->addArcCallback("BrushDrawMove", this, &ForceMovingPlaneDrawHCI::brushDrawMotion);
  _fsa->addArc("BrushDrawPMove", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_Physical_Tracker"));
  _fsa->addArcCallback("BrushDrawPMove", this, &ForceMovingPlaneDrawHCI::brushPhysicalFrameChange);
  _fsa->addArc("HandDrawMove", "Drawing", "Drawing", handMotionTriggers);
  _fsa->addArcCallback("HandDrawMove", this, &ForceMovingPlaneDrawHCI::handMotion);
  _fsa->addArc("BrushPressure", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure", this, &ForceMovingPlaneDrawHCI::brushPressureChange);
  _fsa->addArc("BrushOff", "Drawing", "Start", brushOffTriggers);
  _fsa->addArcCallback("BrushOff", this, &ForceMovingPlaneDrawHCI::brushOff);

}


ForceMovingPlaneDrawHCI::~ForceMovingPlaneDrawHCI()
{
}

void
ForceMovingPlaneDrawHCI::setEnabled(bool b)
{
  if ((b) && (!_enabled)) {
    //_forceNetInterface->startMovingPlaneDrawing();
    _eventMgr->addFsaRef(_fsa);
    _cursors->setBrushCursor(CavePaintingCursors::DEFAULT_CURSOR);
    //_cursors->setHandCursor(CavePaintingCursors::PLANE);
    _brush->state->maxPressure = MinVR::ConfigVal("ForceMovingPlaneDraw_MaxPressure", 0.03, false);
    _brush->state->brushInterface = "ForceMovingPlaneDraw";
    _drawID = _gfxMgr->addDrawCallback(this, &ForceMovingPlaneDrawHCI::draw);
  }
  else if ((!b) && (_enabled)) {
    //_forceNetInterface->stopForces();
    _eventMgr->removeFsaRef(_fsa);
    _gfxMgr->removeDrawCallback(_drawID);
  }
  _enabled = b;
}

bool
ForceMovingPlaneDrawHCI::canReleaseFocus()
{
  return (_fsa->getCurrentState() == "Start");
}

void 
ForceMovingPlaneDrawHCI::brushMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
}

void 
ForceMovingPlaneDrawHCI::brushOn(MinVR::EventRef e)
{
  // no support in this drawing interface for varying pressure as we draw
  _brush->state->pressure = 1.0;
  _brush->startNewMark();
  _planeInitPoint = _brush->state->frameInRoomSpace.translation;
}

void 
ForceMovingPlaneDrawHCI::brushPressureChange(MinVR::EventRef e)
{
  _brush->state->pressure = e->get1DData();
  //cout << "new pressure " << _brush->state->pressure << endl;
}

void
ForceMovingPlaneDrawHCI::brushPhysicalFrameChange(MinVR::EventRef e)
{
  _brush->state->physicalFrameInRoomSpace = e->getCoordinateFrameData();
}

void 
ForceMovingPlaneDrawHCI::brushDrawMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  _brush->addSampleToMark();
}

void
ForceMovingPlaneDrawHCI::brushOff(MinVR::EventRef e)
{
  _brush->endMark();
}

void
ForceMovingPlaneDrawHCI::handMotion(MinVR::EventRef e)
{
  _handFrame = e->getCoordinateFrameData();
}


void
ForceMovingPlaneDrawHCI::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  Vector3 planeAxis1 = (_handFrame.translation - _planeInitPoint).unit();
  Vector3 approxNormal = _handFrame.rotation.column(2).unit();
  //Vector3 approxNormal = _phantomFrame.rotation.column(2).unit();
  Vector3 planeAxis2 = planeAxis1.cross(approxNormal).unit();
  Vector3 normal = planeAxis2.cross(planeAxis1).unit();

  Vector3 pos = _brush->state->frameInRoomSpace.translation;

  rd->pushState();
  rd->setCullFace(RenderDevice::CULL_NONE);
  rd->beginPrimitive(PrimitiveType::TRIANGLE_FAN);
  rd->setNormal(normal);
  rd->sendVertex(pos);
  
  const double r = 0.5;
  const int nsections = 40;
  double a = 0.0;
  double ainc = -twoPi() / (double)(nsections-1);
  for (int i=0;i<nsections;i++) {
    Vector3 p = pos + planeAxis1*r*cos(a) + planeAxis2*r*sin(a);
    rd->sendVertex(p);
    a -= ainc;
  }

  rd->endPrimitive();
  rd->popState();
}

} // end namespace

