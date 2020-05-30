
#include <G3DOperators.h>
#include <ConfigVal.H>
#include "ForceTapeDrawHCI.H"
#include "RibbonMark.H"
#include "TubeMark.H"
using namespace G3D;
namespace DrawOnAir {

ForceTapeDrawHCI::ForceTapeDrawHCI(Array<std::string>     brushOnTriggers,
                                   Array<std::string>     brushMotionTriggers,
                                   Array<std::string>     brushOffTriggers,
                                   Array<std::string>     handMotionTriggers,
                                   BrushRef               brush,
                                   CavePaintingCursorsRef cursors,
                                   //ForceNetInterface*     forceNetInterface,
                                   EventMgrRef            eventMgr,
  GfxMgrRef              gfxMgr)
{
  //_forceNetInterface = forceNetInterface;
  _brush = brush;
  _cursors = cursors;
  _eventMgr = eventMgr;
  _gfxMgr = gfxMgr;
  _enabled = false;
  _redrawCutoff = -1;

  _fsa = new Fsa("ForceTapeDrawHCI");
  _fsa->addState("Start");
  _fsa->addState("Drawing");

  _fsa->addArc("BrushMove", "Start", "Start", brushMotionTriggers);
  _fsa->addArcCallback("BrushMove", this, &ForceTapeDrawHCI::brushMotion);
  _fsa->addArc("BrushPMove", "Start", "Start", MinVR::splitStringIntoArray("Brush_Physical_Tracker"));
  _fsa->addArcCallback("BrushPMove", this, &ForceTapeDrawHCI::brushPhysicalFrameChange);
  _fsa->addArc("BrushPressure2", "Start", "Start", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure2", this, &ForceTapeDrawHCI::brushPressureChange);

  _fsa->addArc("BrushOn", "Start", "Drawing", brushOnTriggers);
  _fsa->addArcCallback("BrushOn", this, &ForceTapeDrawHCI::brushOn);

  _fsa->addArc("BrushDrawMove", "Drawing", "Drawing", brushMotionTriggers);
  _fsa->addArcCallback("BrushDrawMove", this, &ForceTapeDrawHCI::brushDrawMotion);
  _fsa->addArc("BrushDrawPMove", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_Physical_Tracker"));
  _fsa->addArcCallback("BrushDrawPMove", this, &ForceTapeDrawHCI::brushPhysicalFrameChange);
  _fsa->addArc("BrushPressure", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure", this, &ForceTapeDrawHCI::brushPressureChange);
  _fsa->addArc("Resize", "Drawing", "Drawing", MinVR::splitStringIntoArray("TapeDraw_Resize"));
  _fsa->addArcCallback("Resize", this, &ForceTapeDrawHCI::resizeFromBrushBackup);
  _fsa->addArc("AddSample", "Drawing", "Drawing", MinVR::splitStringIntoArray("TapeDraw_AddSample"));
  _fsa->addArcCallback("AddSample", this, &ForceTapeDrawHCI::addSample);
  
  _fsa->addArc("BrushOff", "Drawing", "Start", brushOffTriggers);
  _fsa->addArcCallback("BrushOff", this, &ForceTapeDrawHCI::brushOff);


  _fsa->addArc("HandMove1", "Start", "Start", handMotionTriggers);
  _fsa->addArcCallback("HandMove1", this, &ForceTapeDrawHCI::handMotion);
  _fsa->addArc("HandMove2", "Drawing", "Drawing", handMotionTriggers);
  _fsa->addArcCallback("HandMove2", this, &ForceTapeDrawHCI::handMotion);

  _fsa->addArc("HeadMove1", "Start", "Start", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove1", this, &ForceTapeDrawHCI::headMotion);
  _fsa->addArc("HeadMove2", "Drawing", "Drawing", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove2", this, &ForceTapeDrawHCI::headMotion);
}


ForceTapeDrawHCI::~ForceTapeDrawHCI()
{
}

void
ForceTapeDrawHCI::setEnabled(bool b)
{  
  if ((b) && (!_enabled)) {
    _eventMgr->addFsaRef(_fsa);
    _cursors->setBrushCursor(CavePaintingCursors::DEFAULT_CURSOR);
    _brush->state->maxPressure = MinVR::ConfigVal("ForceTapeDraw_MaxPressure", 0.03, false);
    _brush->state->brushInterface = "ForceTapeDraw";
    //_forceNetInterface->startTapeDrawing(true);
    _dcbid = _gfxMgr->addDrawCallback(this, &ForceTapeDrawHCI::draw);
  }
  else if ((!b) && (_enabled)) {
    _eventMgr->removeFsaRef(_fsa);
    //_forceNetInterface->stopForces();
    _gfxMgr->removeDrawCallback(_dcbid);
  }
  _enabled = b;
}


bool
ForceTapeDrawHCI::canReleaseFocus()
{
  return (_fsa->getCurrentState() == "Start");
}

void
ForceTapeDrawHCI::headMotion(MinVR::EventRef e)
{
  _brush->state->headFrame = e->getCoordinateFrameData();
}

void
ForceTapeDrawHCI::handMotion(MinVR::EventRef e)
{
  _handPos = e->getCoordinateFrameData().translation;
  _brush->state->handFrame = e->getCoordinateFrameData();
  _brush->state->drawingDir = (_handPos - _brush->state->frameInRoomSpace.translation).unit();
}

void 
ForceTapeDrawHCI::brushMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
}

void 
ForceTapeDrawHCI::brushOn(MinVR::EventRef e)
{
  // no support in this drawing interface for varying pressure as we draw
  _brush->startNewMark();
  _redrawCutoff = -1;
  _brush->state->colorValue = 0.0;
}

void 
ForceTapeDrawHCI::brushPressureChange(MinVR::EventRef e)
{
  _brush->state->pressure = e->get1DData();
  //cout << "new pressure " << _brush->state->pressure << endl;

  // For this style of controlling the brush, pressure changes both the width and the color 
  // value of the mark.
  if (_brush->state->maxPressure != 0.0) {
    _brush->state->width = _brush->state->pressure/_brush->state->maxPressure * _brush->state->size;
    _brush->state->colorValue = _brush->state->pressure/_brush->state->maxPressure;
  }

  // This could happen twice per frame if the brush and the pressure
  // both change in the same frame, which is likely, but if we don't
  // do this here, then it won't update when we just change pressure,
  // which is desireable.
  if ((_redrawCutoff != -1) && (_fsa->getCurrentState() == "Drawing")) {
    smoothOutRedrawEdits();
  }
}

void
ForceTapeDrawHCI::brushPhysicalFrameChange(MinVR::EventRef e)
{
  _brush->state->physicalFrameInRoomSpace = e->getCoordinateFrameData();
}

void 
ForceTapeDrawHCI::brushDrawMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
}

void
ForceTapeDrawHCI::addSample(MinVR::EventRef e)
{
  _brush->addSampleToMark();
  if (_redrawCutoff != -1) {
    smoothOutRedrawEdits();
  }
  else {
    smoothPressure();
  }
}

void
ForceTapeDrawHCI::resizeFromBrushBackup(MinVR::EventRef e)
{
  int newEndPt = e->get1DData();
  Array<Vector3> removedPortion;
  for (int i=newEndPt+1;i<_brush->currentMark->getNumSamples();i++) {
    removedPortion.append(_brush->currentMark->getSamplePosition(i));
  }
  if (removedPortion.size()) {
    _redrawLines.append(removedPortion);
  }

  debugAssert(newEndPt < _brush->currentMark->getNumSamples()-1);
  _brush->currentMark->trimEnd(newEndPt);


  // Set cutoff and blend start point for smoothing calculations.
  // Don't reset them if we're still within the blend region set by a
  // previous backup.
  if ((_redrawCutoff == -1) || (_redrawCutoff > newEndPt)) {
    _redrawCutoff = newEndPt;
    
    const double preblendlength = 0.25;//_brush->state->size*2.0;
    _redrawBlendStart = _redrawCutoff;
    while ((_redrawBlendStart > 0) && 
           (_brush->currentMark->getArcLength(_redrawBlendStart, _redrawCutoff) < preblendlength)) {
      _redrawBlendStart--;
    }
    
    // the smoothing doesn't work nicely if we're near the beginning of
    // the line, so try something else.
    if ((_redrawBlendStart == 0) && (_redrawCutoff != 0)) {
      _redrawBlendStart = iCeil((float)_redrawCutoff/2.0);
    }

    debugAssert(_redrawBlendStart >= 0);
  }
}

void
ForceTapeDrawHCI::brushOff(MinVR::EventRef e)
{
  _brush->endMark();
  _brush->state->width = _brush->state->size;
  _brush->state->colorValue = 1.0;

  _redrawLines.clear();
}

void
ForceTapeDrawHCI::smoothOutRedrawEdits()
{
  // shouldn't get here, but just in case, return
  if ((_redrawCutoff == -1) ||
      (_redrawCutoff > _brush->currentMark->getNumSamples()) ||
      (_redrawBlendStart >= _brush->currentMark->getNumSamples())) {
    return;
  }


  const double postblendlength = 0.25;//_brush->state->size*2.0;
  double arclength = _brush->currentMark->getArcLength(_redrawCutoff, _brush->currentMark->getNumSamples());
  if (arclength >= postblendlength) {
    // drawing has advanced past the end of the the blending point,
    // reset the redrawCutoff so it can be activated again by backing up.
    _redrawCutoff = -1;
  }
  else if (arclength > 0.0) {
  
    const double blendlength = _brush->currentMark->getArcLength(_redrawBlendStart, _redrawCutoff) + postblendlength;
  

    // keep sample # _redrawBlendStart, trim samples starting with # _redrawBlendStart+1

    // save brush states and alphas based on the positions of the trimmed samples
    Array<BrushStateRef> trimmedStates =
      _brush->currentMark->getBrushStatesSubArray(_redrawBlendStart+1, _brush->currentMark->getNumSamples());

    // trim mark
    _brush->currentMark->trimEnd(_redrawBlendStart);

    // recreate the trimmed portion of the mark with new interpolated brush states
    BrushStateRef origBS = _brush->currentMark->getBrushState(_redrawBlendStart);
    for (int i=0;i<trimmedStates.size();i++) {

      double alpha = (double)(i+1) / (double)(trimmedStates.size());

      // interpolate the rotation component of the brush frame
      CoordinateFrame newRot = origBS->frameInRoomSpace.lerp(_brush->state->frameInRoomSpace, alpha);
      trimmedStates[i]->frameInRoomSpace.rotation = newRot.rotation;

      // interpolate the rotation component of the physical brush frame
      CoordinateFrame newRot2 = origBS->physicalFrameInRoomSpace.lerp(_brush->state->physicalFrameInRoomSpace, alpha);
      trimmedStates[i]->physicalFrameInRoomSpace.rotation = newRot2.rotation;

      // interpolate the pressure measure
      trimmedStates[i]->pressure = lerp(origBS->pressure, _brush->state->pressure, alpha);

      // interpolate the color value
      trimmedStates[i]->colorValue = lerp(origBS->colorValue, _brush->state->colorValue, alpha);

      // interpolate the width
      trimmedStates[i]->width = lerp(origBS->width, _brush->state->width, alpha);

      _brush->currentMark->addSample(trimmedStates[i]);
    }
  }
}

void
ForceTapeDrawHCI::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  rd->pushState();
  rd->setObjectToWorldMatrix(virtualToRoomSpace);
  rd->disableLighting();  
  rd->setColor(Color3(0.8, 0.35, 0.35));
  rd->setPointSize(2.0);
  for (int i=0;i<_redrawLines.size();i++) {
    rd->beginPrimitive(PrimitiveType::POINTS);
    for (int j=0;j<_redrawLines[i].size();j++) {
      rd->sendVertex(_redrawLines[i][j]);
    }
    rd->endPrimitive();
  }
  rd->popState();
}


void
ForceTapeDrawHCI::smoothPressure()
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

