#include <VRG3DBaseApp.H>
#include <ConfigVal.H>
#include <G3DOperators.h>

#include "DirectDrawHCI.H"
using namespace G3D;
namespace DrawOnAir {

DirectDrawHCI::DirectDrawHCI(Array<std::string>     brushOnTriggers,
                             Array<std::string>     brushMotionTriggers, 
                             Array<std::string>     brushOffTriggers,
                             Array<std::string>     handMotionTriggers, 
                             BrushRef               brush,
                             CavePaintingCursorsRef cursors,
                             EventMgrRef            eventMgr)
{
  _brush = brush;
  _cursors = cursors;
  _eventMgr = eventMgr;
  _enabled = false;
  _dirCalcDistThreshold = MinVR::ConfigVal("DirectDrawHCI_DirCalcDistThreshold", 0.01, false);

  _fsa = new Fsa("DirectDrawHCI");
  _fsa->addState("Start");
  _fsa->addState("Drawing");

  _fsa->addArc("BrushMove", "Start", "Start", brushMotionTriggers);
  _fsa->addArcCallback("BrushMove", this, &DirectDrawHCI::brushMotion);
  _fsa->addArc("BrushPMove", "Start", "Start", MinVR::splitStringIntoArray("Brush_Physical_Tracker"));
  _fsa->addArcCallback("BrushPMove", this, &DirectDrawHCI::brushPhysicalFrameChange);
  _fsa->addArc("BrushOn", "Start", "Drawing", brushOnTriggers);
  _fsa->addArcCallback("BrushOn", this, &DirectDrawHCI::brushOn);
  _fsa->addArc("BrushDrawMove", "Drawing", "Drawing", brushMotionTriggers);
  _fsa->addArcCallback("BrushDrawMove", this, &DirectDrawHCI::brushDrawMotion);
  _fsa->addArc("BrushDrawPMove", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_Physical_Tracker"));
  _fsa->addArcCallback("BrushDrawPMove", this, &DirectDrawHCI::brushPhysicalFrameChange);
  _fsa->addArc("BrushPressure", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure", this, &DirectDrawHCI::brushPressureChange);
  _fsa->addArc("BrushOff", "Drawing", "Start", brushOffTriggers);
  _fsa->addArcCallback("BrushOff", this, &DirectDrawHCI::brushOff);


  _fsa->addArc("HeadMove1", "Start", "Start", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove1", this, &DirectDrawHCI::headMotion);
  _fsa->addArc("HeadMove2", "Drawing", "Drawing", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove2", this, &DirectDrawHCI::headMotion);
}

DirectDrawHCI::~DirectDrawHCI()
{
}

void
DirectDrawHCI::setEnabled(bool b) 
{
  if ((b) && (!_enabled)) {
    _eventMgr->addFsaRef(_fsa);
    _cursors->setBrushCursor(CavePaintingCursors::DEFAULT_CURSOR);
    _brush->state->maxPressure = MinVR::ConfigVal("DirectDrawHCI_MaxPressure", 1.0, false);
    _brush->state->brushInterface = "DirectDraw";
  }
  else if ((!b) && (_enabled)) {
    _eventMgr->removeFsaRef(_fsa);
  }
  _enabled = b;
}

bool
DirectDrawHCI::canReleaseFocus()
{
  return (_fsa->getCurrentState() == "Start");
}

void
DirectDrawHCI::updateDrawingDir()
{
  // Since different DrawHCI classes subclass from this, we need to
  // determine the drawing direction differently depending on the
  // current interface.
  if (_brush->state->brushInterface == "ForceOneHandDraw") {
    _brush->state->drawingDir = _brush->state->frameInRoomSpace.vectorToWorldSpace(Vector3(0,1,0)).unit();
  }
  else if (_brush->state->brushInterface == "ForceReverseTapeDraw") {
    _brush->state->drawingDir = (_brush->state->frameInRoomSpace.translation - _brush->state->handFrame.translation).unit();
  }
  else {
    // update brush heading
    Vector3 delta = _brush->state->frameInRoomSpace.translation - _lastPosForDirCalc;
    if (delta.magnitude() > _dirCalcDistThreshold) {
      _brush->state->drawingDir = delta.unit();
      _lastPosForDirCalc = _brush->state->frameInRoomSpace.translation;
    }  
  }
}

void 
DirectDrawHCI::headMotion(MinVR::EventRef e)
{
  _brush->state->headFrame = e->getCoordinateFrameData();
}

void 
DirectDrawHCI::handMotion(MinVR::EventRef e)
{
  _brush->state->handFrame = e->getCoordinateFrameData();
  updateDrawingDir();
}

void 
DirectDrawHCI::brushMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  updateDrawingDir();
}

void 
DirectDrawHCI::brushOn(MinVR::EventRef e)
{
  _brush->startNewMark();
  _brush->state->colorValue = 0.0;
}

void 
DirectDrawHCI::brushPressureChange(MinVR::EventRef e)
{
  _brush->state->pressure = e->get1DData();
  //cout << "new pressure " << _brush->state->pressure << endl;

  // For this style of controlling the brush, pressure changes both the width and the color 
  // value of the mark.
  if (_brush->state->maxPressure != 0.0) {
    _brush->state->width = _brush->state->pressure/_brush->state->maxPressure * _brush->state->size;
    //_brush->state->colorValue = _brush->state->pressure/_brush->state->maxPressure;
  }
}

void
DirectDrawHCI::brushPhysicalFrameChange(MinVR::EventRef e)
{
  _brush->state->physicalFrameInRoomSpace = e->getCoordinateFrameData();
}

void 
DirectDrawHCI::brushDrawMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  updateDrawingDir();
  _brush->addSampleToMark();
  smoothOutPressure();
}

void
DirectDrawHCI::brushOff(MinVR::EventRef e)
{
  _brush->endMark();
}


void
DirectDrawHCI::smoothOutPressure()
{
  // This shouldn't be necessary if we had a better force device with
  // more pressure levels, but since the changes are so abrupt, try to
  // smooth them out to make them look better.

  if ((_brush->currentMark.notNull()) && (_brush->currentMark->getNumSamples() > 3)) {

    int n = _brush->currentMark->getNumSamples();
    BrushStateRef curBS = _brush->currentMark->getBrushState(n-1);
    double curPressure = curBS->pressure;
    double prevPressure = _brush->currentMark->getBrushState(n-2)->pressure;

    if (curPressure != prevPressure) {
              
      int i = n-3;
      bool done = false;
      while (!done) {
        if (i == 0) {
          done = true;
        }
        else if (_brush->currentMark->getBrushState(i)->pressure == prevPressure) {
          i--;
        }
        else {
          done = true;
        }
      }

      if (i < n-3) {

        // save brush states and alphas based on the positions of the trimmed samples
        Array<BrushStateRef> trimmedStates =
          _brush->currentMark->getBrushStatesSubArray(i+1, _brush->currentMark->getNumSamples());

        // trim mark
        _brush->currentMark->trimEnd(i);

        // recreate the trimmed portion of the mark with new interpolated brush states
        BrushStateRef origBS = _brush->currentMark->getBrushState(i);
        for (int c=0;c<trimmedStates.size();c++) {

          double alpha = (double)(c+1) / (double)(trimmedStates.size());

          // interpolate the pressure measure
          trimmedStates[c]->pressure = lerp(origBS->pressure, _brush->state->pressure, alpha);
          
          // interpolate the color value
          trimmedStates[c]->colorValue = lerp(origBS->colorValue, _brush->state->colorValue, alpha);
          
          // interpolate the width
          trimmedStates[c]->width = lerp(origBS->width, _brush->state->width, alpha);
          
          _brush->currentMark->addSample(trimmedStates[c]);
        }
      }
    }
  }
}


} // end namespace
