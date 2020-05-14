#include <ConfigVal.H>
#include "ReverseTapeDrawHCI.H"
#include "Mark.H"
using namespace G3D;
namespace DrawOnAir {

ReverseTapeDrawHCI::ReverseTapeDrawHCI(Array<std::string>     brushOnTriggers,
                                       Array<std::string>     brushMotionTriggers,
                                       Array<std::string>     brushOffTriggers,
                                       Array<std::string>     handMotionTriggers,
                                       BrushRef               brush,
                                       CavePaintingCursorsRef cursors,
  MinVR::EventMgrRef            eventMgr,
  MinVR::GfxMgrRef              gfxMgr)
{
  _brush = brush;
  _cursors = cursors;
  _eventMgr = eventMgr;
  _gfxMgr = gfxMgr;
  _enabled = false;

  _brush->state->maxPressure = MinVR::ConfigVal("TapeDraw_MaxPressure", 0.0, false);

  _fsa = new MinVR::Fsa("ReverseTapeDrawHCI");
  _fsa->addState("Start");
  _fsa->addState("Painting");

  _fsa->addArc("BrushMotion", "Start", "Start", brushMotionTriggers);
  _fsa->addArc("HandMotion1", "Start", "Start", handMotionTriggers);
  _fsa->addArc("BrushOn", "Start", "Painting", brushOnTriggers);
  _fsa->addArc("BrushPaintingMotion", "Painting", "Painting", brushMotionTriggers);
  _fsa->addArc("HandMotion2", "Painting", "Painting", handMotionTriggers);
  _fsa->addArc("BrushOff", "Painting", "Start", brushOffTriggers);
  
  _fsa->addArcCallback("BrushMotion", this, &ReverseTapeDrawHCI::brushMotion);
  _fsa->addStateEnterCallback("Painting", this, &ReverseTapeDrawHCI::brushOn);
  _fsa->addArcCallback("BrushPaintingMotion", this, &ReverseTapeDrawHCI::brushMotionWhilePainting);
  _fsa->addArcCallback("HandMotion1", this, &ReverseTapeDrawHCI::handMotion);
  _fsa->addArcCallback("HandMotion2", this, &ReverseTapeDrawHCI::handMotion);
  _fsa->addStateExitCallback("Painting", this, &ReverseTapeDrawHCI::brushOff);

  _fsa->addArc("HeadMove1", "Start", "Start", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove1", this, &ReverseTapeDrawHCI::headMotion);
  _fsa->addArc("HeadMove2", "Painting", "Painting", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove2", this, &ReverseTapeDrawHCI::headMotion);
}


ReverseTapeDrawHCI::~ReverseTapeDrawHCI()
{
}


void
ReverseTapeDrawHCI::setEnabled(bool b)
{
  if ((b) && (!_enabled)) {
    _eventMgr->addFsaRef(_fsa);
    _cursors->setBrushCursor(CavePaintingCursors::DEFAULT_CURSOR);
    _brush->state->brushInterface = "ReverseTapeDraw";
    _brush->state->maxPressure = 0.0;
  }
  else if ((!b) && (_enabled)) {
    _eventMgr->removeFsaRef(_fsa);
  }
  _enabled = b;
}

bool
ReverseTapeDrawHCI::canReleaseFocus()
{
  return (_fsa->getCurrentState() == "Start");
}

void
ReverseTapeDrawHCI::headMotion(MinVR::EventRef e)
{
  _brush->state->headFrame = e->getCoordinateFrameData();
}

void
ReverseTapeDrawHCI::handMotion(MinVR::EventRef e)
{
  _brush->state->handFrame = e->getCoordinateFrameData();
  _handPos = e->getCoordinateFrameData().translation;
  _brush->state->drawingDir = (_brush->state->frameInRoomSpace.translation - _handPos).unit();
}

void
ReverseTapeDrawHCI::brushMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  _brush->state->physicalFrameInRoomSpace = e->getCoordinateFrameData();
  _brush->state->drawingDir = (_brush->state->frameInRoomSpace.translation - _handPos).unit();
}


void 
ReverseTapeDrawHCI::brushOn(MinVR::EventRef e)
{
  // no support in this drawing interface for varying pressure as we draw
  _brush->state->pressure = 1.0;
  _brush->startNewMark();
}


void
ReverseTapeDrawHCI::brushMotionWhilePainting(MinVR::EventRef e)
{
  _brush->state->physicalFrameInRoomSpace = e->getCoordinateFrameData();
  CoordinateFrame eRoom = e->getCoordinateFrameData();
  CoordinateFrame eVirtual = _gfxMgr->roomToVirtualSpace(eRoom);

  Vector3 handVirtual = _gfxMgr->roomPointToVirtualSpace(_handPos);
  
  Vector3 pointOnPath = eVirtual.translation;
  bool addThisSample = true;

  // if we've already added a sample, then project the brush position onto the
  // path that we've already begun
  if ((_brush->currentMark.notNull()) && (_brush->currentMark->getNumSamples())) {
    Vector3 lastPointOnPath = _brush->currentMark->getSamplePosition(_brush->currentMark->getNumSamples()-1);
    G3D::Line l = G3D::Line::fromTwoPoints(lastPointOnPath, handVirtual);
    pointOnPath = l.closestPoint(pointOnPath);

    if ((handVirtual-pointOnPath).magnitude() <= (handVirtual-lastPointOnPath).magnitude()) {
      addThisSample = false;
    }
  }

  if (addThisSample) {
    _brush->state->frameInRoomSpace.translation = _gfxMgr->virtualPointToRoomSpace(pointOnPath);
    _brush->state->drawingDir = (_brush->state->frameInRoomSpace.translation - _handPos).unit();
    _brush->addSampleToMark();
  }
}

void
ReverseTapeDrawHCI::brushOff(MinVR::EventRef e)
{
  _brush->endMark();
}

} // end namespace

