#include <ConfigVal.H>
#include <G3DOperators.h>
#include "TapeDrawHCI.H"
#include "Mark.H"
using namespace G3D;
namespace DrawOnAir {

TapeDrawHCI::TapeDrawHCI(Array<std::string>     brushOnTriggers,
                         Array<std::string>     brushMotionTriggers,
                         Array<std::string>     brushOffTriggers,
                         Array<std::string>     handMotionTriggers,
                         BrushRef               brush,
                         CavePaintingCursorsRef cursors,
  EventMgrRef            eventMgr,
  GfxMgrRef              gfxMgr)
{
  _brush = brush;
  _cursors = cursors;
  _eventMgr = eventMgr;
  _gfxMgr = gfxMgr;
  _enabled = false;

  _fsa = new Fsa("TapeDrawHCI");
  _fsa->addState("Start");
  _fsa->addState("Painting");

  _fsa->addArc("BrushMotion", "Start", "Start", brushMotionTriggers);
  _fsa->addArc("HandMotion1", "Start", "Start", handMotionTriggers);
  _fsa->addArc("BrushOn", "Start", "Painting", brushOnTriggers);
  _fsa->addArc("BrushPaintingMotion", "Painting", "Painting", brushMotionTriggers);
  _fsa->addArc("HandMotion2", "Painting", "Painting", handMotionTriggers);
  _fsa->addArc("BrushOff", "Painting", "Start", brushOffTriggers);
  
  _fsa->addArcCallback("BrushMotion", this, &TapeDrawHCI::brushMotion);
  _fsa->addStateEnterCallback("Painting", this, &TapeDrawHCI::brushOn);
  _fsa->addArcCallback("BrushPaintingMotion", this, 
      &TapeDrawHCI::brushMotionWhilePainting);
  _fsa->addArcCallback("HandMotion1", this, &TapeDrawHCI::handMotion);
  _fsa->addArcCallback("HandMotion2", this, &TapeDrawHCI::handMotion);
  _fsa->addStateExitCallback("Painting", this, &TapeDrawHCI::brushOff);

  _fsa->addArc("HeadMove1", "Start", "Start", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove1", this, &TapeDrawHCI::headMotion);
  _fsa->addArc("HeadMove2", "Painting", "Painting", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove2", this, &TapeDrawHCI::headMotion);

  _fsa->addArc("BrushPressure1", "Start", "Start", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure1", this, &TapeDrawHCI::brushPressureChange);
  _fsa->addArc("BrushPressure2", "Painting", "Painting", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure2", this, &TapeDrawHCI::brushPressureChange);
}


TapeDrawHCI::~TapeDrawHCI()
{
}


void
TapeDrawHCI::setEnabled(bool b)
{
  if ((b) && (!_enabled)) {
    _eventMgr->addFsaRef(_fsa);
    _cursors->setBrushCursor(CavePaintingCursors::DEFAULT_CURSOR);
    _brush->state->brushInterface = "TapeDraw";
    _brush->state->maxPressure = MinVR::ConfigVal("TapeDraw_MaxPressure", 1.0, false);
  }
  else if ((!b) && (_enabled)) {
    _eventMgr->removeFsaRef(_fsa);
  }
  _enabled = b;
}

bool
TapeDrawHCI::canReleaseFocus()
{
  return (_fsa->getCurrentState() == "Start");
}

void
TapeDrawHCI::headMotion(MinVR::EventRef e)
{
  _brush->state->headFrame = e->getCoordinateFrameData();
}

void
TapeDrawHCI::handMotion(MinVR::EventRef e)
{
  _handPos = e->getCoordinateFrameData().translation;
  _brush->state->drawingDir = (_handPos - _brush->state->frameInRoomSpace.translation).unit();
}

void 
TapeDrawHCI::brushPressureChange(MinVR::EventRef e)
{
  _brush->state->pressure = e->get1DData();
  //cout << "new pressure " << _brush->state->pressure << endl;
  _brush->state->width = _brush->state->pressure/_brush->state->maxPressure * _brush->state->size;
  _brush->state->colorValue = _brush->state->pressure/_brush->state->maxPressure;
}

void
TapeDrawHCI::brushMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  _brush->state->physicalFrameInRoomSpace = e->getCoordinateFrameData();
  _brush->state->drawingDir = (_handPos - _brush->state->frameInRoomSpace.translation).unit();
}


void 
TapeDrawHCI::brushOn(MinVR::EventRef e)
{
  _brush->startNewMark();
}


void
TapeDrawHCI::brushMotionWhilePainting(MinVR::EventRef e)
{
  _brush->state->physicalFrameInRoomSpace = e->getCoordinateFrameData();
  CoordinateFrame eRoom = e->getCoordinateFrameData();
  CoordinateFrame eVirtual = _gfxMgr->roomToVirtualSpace(eRoom);

  Vector3 handVirtual = _gfxMgr->roomPointToVirtualSpace(_handPos);  
  Vector3 pointOnPath = eVirtual.translation;

  bool addThisSample = true;

  // if we've already added a sample, then project the brush position onto the
  // path that we've already begun

  // If already added at least one sample, then figure out whether to
  // add another and where exactly it should be.
  if ((_brush->currentMark.notNull()) && (_brush->currentMark->getNumSamples())) {
    Vector3 lastPointOnPath = _brush->currentMark->getSamplePosition(_brush->currentMark->getNumSamples()-1);

    if (MinVR::ConfigVal("TapeDrawing_CaveStyleAdvancement", 0, false)) {
      // Cave (non-haptic) mode, advance based on brush movement, not projection
      Vector3 delta = eVirtual.translation - _lastBrushPos;
      Vector3 dir = (handVirtual - lastPointOnPath).unit();
      double moveInDrawDir = delta.dot(dir);
      if (moveInDrawDir <= 0.0) {
        addThisSample = false;
      }
      else {
        pointOnPath = lastPointOnPath + moveInDrawDir*dir;
      }
    }
    else {
      // Normal mode, project brush position onto drawing line
      G3D::Line l = G3D::Line::fromTwoPoints(lastPointOnPath, handVirtual);
      pointOnPath = l.closestPoint(pointOnPath);

      // pointOnPath, lastPointOnPath and handVirtual all lie on a line, if we're moving forward, then
      // pointOnPath should be closer to handVirtual than lastPointOnPath
      if ((handVirtual-pointOnPath).magnitude() >= (handVirtual-lastPointOnPath).magnitude()) {
        addThisSample = false;
      }
    }
  }

  if (addThisSample) {
    _brush->state->frameInRoomSpace.translation = _gfxMgr->virtualPointToRoomSpace(pointOnPath);
    _brush->state->drawingDir = (_handPos - _brush->state->frameInRoomSpace.translation).unit();
    _brush->addSampleToMark();
  }
  _lastBrushPos = eVirtual.translation;
}

void
TapeDrawHCI::brushOff(MinVR::EventRef e)
{
  _brush->endMark();
}

} // end namespace

