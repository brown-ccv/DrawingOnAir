
#include <ConfigVal.H>
#include <G3DOperators.h>
#include "DragDrawHCI.H"


using namespace G3D;
namespace DrawOnAir {

DragDrawHCI::DragDrawHCI(Array<std::string>     brushOnTriggers,
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
  _lineLength = MinVR::ConfigVal("DragDrawHCI_ShortLineLength", 0.01, false);

  _fsa = new MinVR::Fsa("DragDrawHCI");
  _fsa->addState("Start");
  _fsa->addState("Drawing");

  _fsa->addArc("BrushMove", "Start", "Start", brushMotionTriggers);
  _fsa->addArcCallback("BrushMove", this, &DragDrawHCI::brushMotion);
  _fsa->addArc("BrushPMove", "Start", "Start", MinVR::splitStringIntoArray("Brush_Physical_Tracker"));
  _fsa->addArcCallback("BrushPMove", this, &DragDrawHCI::brushPhysicalFrameChange);
  _fsa->addArc("BrushPressureS", "Start", "Start", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressureS", this, &DragDrawHCI::brushPressureChange);

  _fsa->addArc("BrushOn", "Start", "Drawing", brushOnTriggers);
  _fsa->addArcCallback("BrushOn", this, &DragDrawHCI::brushOn);
  _fsa->addArc("BrushDrawMove", "Drawing", "Drawing", brushMotionTriggers);
  _fsa->addArcCallback("BrushDrawMove", this, &DragDrawHCI::brushDrawMotion);
  _fsa->addArc("BrushDrawPMove", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_Physical_Tracker"));
  _fsa->addArcCallback("BrushDrawPMove", this, &DragDrawHCI::brushPhysicalFrameChange);
  _fsa->addArc("BrushPressure", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure", this, &DragDrawHCI::brushPressureChange);
  _fsa->addArc("BrushOff", "Drawing", "Start", brushOffTriggers);
  _fsa->addArcCallback("BrushOff", this, &DragDrawHCI::brushOff);

  _fsa->addArc("HeadMove1", "Start", "Start", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove1", this, &DragDrawHCI::headMotion);
  _fsa->addArc("HeadMove3", "Drawing", "Drawing", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove3", this, &DragDrawHCI::headMotion);

  _fsa->addArc("HandMove1", "Start", "Start", handMotionTriggers);
  _fsa->addArcCallback("HandMove1", this, &DragDrawHCI::handMotion);
  _fsa->addArc("HandMove3", "Drawing", "Drawing", handMotionTriggers);
  _fsa->addArcCallback("HandMove3", this, &DragDrawHCI::handMotion);
}

DragDrawHCI::~DragDrawHCI()
{
}

void
DragDrawHCI::setEnabled(bool b) 
{
  if ((b) && (!_enabled)) {
    _eventMgr->addFsaRef(_fsa);
    _cursors->setBrushCursor(CavePaintingCursors::NO_CURSOR);
    _brush->state->maxPressure = MinVR::ConfigVal("DragDrawHCI_MaxPressure", 1.0, false);
    _brush->state->brushInterface = "DragDraw";
    _drawID = _gfxMgr->addDrawCallback(this, &DragDrawHCI::draw);
    _dragPoint = _brush->state->frameInRoomSpace.translation;
  }
  else if ((!b) && (_enabled)) {
    _eventMgr->removeFsaRef(_fsa);
    _gfxMgr->removeDrawCallback(_drawID);
  }
  _enabled = b;
}

bool
DragDrawHCI::canReleaseFocus()
{
  return (_fsa->getCurrentState() == "Start");
}

void 
DragDrawHCI::handMotion(MinVR::EventRef e)
{
  _brush->state->handFrame = e->getCoordinateFrameData();
}

void 
DragDrawHCI::headMotion(MinVR::EventRef e)
{
  _brush->state->headFrame = e->getCoordinateFrameData();
}

void
DragDrawHCI::setLineLength(double d)
{
  _lineLength = d;
}

void 
DragDrawHCI::brushMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  _dragPoint = _brush->state->frameInRoomSpace.translation;
}

void 
DragDrawHCI::brushOn(MinVR::EventRef e)
{
  _brush->state->pressure = 1.0;
  _interpLineLength = 0.0;
  _pathLength = 0.0;
  _brush->startNewMark();
}

void 
DragDrawHCI::brushPressureChange(MinVR::EventRef e)
{
  _brush->state->pressure = e->get1DData();
  //cout << "new pressure " << _brush->state->pressure << endl;

  _brush->state->width = _brush->state->pressure/_brush->state->maxPressure * _brush->state->size;
  _brush->state->colorValue = _brush->state->pressure/_brush->state->maxPressure;
}

void
DragDrawHCI::brushPhysicalFrameChange(MinVR::EventRef e)
{
  _brush->state->physicalFrameInRoomSpace = e->getCoordinateFrameData();
}

void 
DragDrawHCI::brushDrawMotion(MinVR::EventRef e)
{
  // grow the interpolated line
  if (_pathLength >= _lineLength) {
    _interpLineLength = _lineLength;
  }
  else {
    _interpLineLength = lerp(0, _lineLength, _pathLength / _lineLength);
  }

  Vector3 currentPos = e->getCoordinateFrameData().translation;
  Vector3 drawDir = (currentPos - _dragPoint).unit();

  _brush->state->drawingDir = drawDir;
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();

  if ((currentPos - _dragPoint).length() > _interpLineLength) {
    Vector3 oldPt = _dragPoint;
    _dragPoint = currentPos - _interpLineLength*drawDir;
    _pathLength += (_dragPoint - oldPt).length();

    // add this sample to the path
    _brush->state->frameInRoomSpace.translation = _dragPoint;
    _brush->addSampleToMark();
    _brush->state->frameInRoomSpace.translation = e->getCoordinateFrameData().translation;
  }
}

void
DragDrawHCI::brushOff(MinVR::EventRef e)
{
  _brush->endMark();
}


void
DragDrawHCI::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  CoordinateFrame f = _brush->state->frameInRoomSpace;
  f.translation = _dragPoint;
  Color3 flatBrushColor = Color3(1.0, 1.0, 0.8);
  _cursors->drawFlatBrush(rd, f, _brush->state->size, flatBrushColor, _gfxMgr->getTexture("BrushTexture"));
  _cursors->drawPressureMeter();

  rd->pushState();
  rd->setColor(Color3::white());
  rd->disableLighting();
  rd->beginPrimitive(PrimitiveType::LINES);
  rd->sendVertex(_brush->state->frameInRoomSpace.translation);
  rd->sendVertex(_dragPoint);
  rd->endPrimitive();
  rd->popState();
}


} // end namespace
