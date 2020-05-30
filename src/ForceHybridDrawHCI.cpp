#include <ConfigVal.H>
#include <G3DOperators.h>
#include "ForceHybridDrawHCI.H"
#include "RibbonMark.H"
#include "TubeMark.H"
#include "Shadows.H"
using namespace G3D;
namespace DrawOnAir {

ForceHybridDrawHCI::ForceHybridDrawHCI(Array<std::string>     brushOnTriggers,
                                       Array<std::string>     brushMotionTriggers,
                                       Array<std::string>     brushOffTriggers,
                                       Array<std::string>     handMotionTriggers,
                                       Array<std::string>     handDownTriggers,
                                       Array<std::string>     handUpTriggers,
                                       BrushRef               brush,
                                       CavePaintingCursorsRef cursors,
                                      // ForceNetInterface*     forceNetInterface,
  EventMgrRef            eventMgr,
  GfxMgrRef              gfxMgr)
{
 // _forceNetInterface = forceNetInterface;
  _brush = brush;
  _cursors = cursors;
  _eventMgr = eventMgr;
  _gfxMgr = gfxMgr;
  _enabled = false;
  _redrawCutoff = -1;
  _hybridState = DragDrawing;
  _hybridStateToStart = DragDrawing;
  _reverseTape = false;
  _handCloseToBrush = false;
  _backupLength = MinVR::ConfigVal("DragDrawing_BackupLength", 0.05, false);
  _lineLength = MinVR::ConfigVal("DragDrawing_LineLength", 0.15, false);

  _fsa = new Fsa("ForceHybridDrawHCI");
  _fsa->addState("Start");
  _fsa->addState("Drawing");

  _fsa->addArc("BrushMove", "Start", "Start", brushMotionTriggers);
  _fsa->addArcCallback("BrushMove", this, &ForceHybridDrawHCI::brushMotion);
  _fsa->addArc("BrushPMove", "Start", "Start", MinVR::splitStringIntoArray("Brush_Physical_Tracker"));
  _fsa->addArcCallback("BrushPMove", this, &ForceHybridDrawHCI::brushPhysicalFrameChange);
  _fsa->addArc("BrushPressure2", "Start", "Start", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure2", this, &ForceHybridDrawHCI::brushPressureChange);
  _fsa->addArc("HandMove1", "Start", "Start", handMotionTriggers);
  _fsa->addArcCallback("HandMove1", this, &ForceHybridDrawHCI::handMotion);

  _fsa->addArc("BrushOn", "Start", "Drawing", brushOnTriggers);
  _fsa->addArcCallback("BrushOn", this, &ForceHybridDrawHCI::brushOn);

  _fsa->addArc("BrushDrawMove", "Drawing", "Drawing", brushMotionTriggers);
  _fsa->addArcCallback("BrushDrawMove", this, &ForceHybridDrawHCI::brushDrawMotion);
  _fsa->addArc("BrushDrawPMove", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_Physical_Tracker"));
  _fsa->addArcCallback("BrushDrawPMove", this, &ForceHybridDrawHCI::brushPhysicalFrameChange);
  _fsa->addArc("BrushPressure", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure", this, &ForceHybridDrawHCI::brushPressureChange);
  _fsa->addArc("AddSample", "Drawing", "Drawing", MinVR::splitStringIntoArray("HybridDraw_AddSample"));
  _fsa->addArcCallback("AddSample", this, &ForceHybridDrawHCI::addSample);
  _fsa->addArc("Resize", "Drawing", "Drawing", MinVR::splitStringIntoArray("HybridDraw_Resize"));
  _fsa->addArcCallback("Resize", this, &ForceHybridDrawHCI::resizeFromBrushBackup);
  _fsa->addArc("Reverse", "Drawing", "Drawing", MinVR::splitStringIntoArray("HybridDraw_ReverseTape"));
  _fsa->addArcCallback("Reverse", this, &ForceHybridDrawHCI::reverseTape);

  _fsa->addArc("HandDown", "Drawing", "Drawing", handDownTriggers);
  _fsa->addArcCallback("HandDown", this, &ForceHybridDrawHCI::handBtnDown);
  _fsa->addArc("HandMove2", "Drawing", "Drawing", handMotionTriggers);
  _fsa->addArcCallback("HandMove2", this, &ForceHybridDrawHCI::handMotion);
  _fsa->addArc("HandUp", "Drawing", "Drawing", handUpTriggers);
  _fsa->addArcCallback("HandUp", this, &ForceHybridDrawHCI::handBtnUp);

  _fsa->addArc("BrushOff", "Drawing", "Start", brushOffTriggers);
  _fsa->addArcCallback("BrushOff", this, &ForceHybridDrawHCI::brushOff);


  _fsa->addArc("ValueChange1", "Start", "Start", MinVR::splitStringIntoArray("Brush_ColorValue"));
  _fsa->addArcCallback("ValueChange1", this, &ForceHybridDrawHCI::brushColorValueChange);
  _fsa->addArc("ValueChange2", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_ColorValue"));
  _fsa->addArcCallback("ValueChange2", this, &ForceHybridDrawHCI::brushColorValueChange);

  _fsa->addArc("HeadMove1", "Start", "Start", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove1", this, &ForceHybridDrawHCI::headMotion);
  _fsa->addArc("HeadMove2", "Drawing", "Drawing", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove2", this, &ForceHybridDrawHCI::headMotion);

  _fsa->addArc("SetDragFirst1", "Start", "Start", MinVR::splitStringIntoArray("HybridDraw_StartWithDrag"));
  _fsa->addArcCallback("SetDragFirst1", this, &ForceHybridDrawHCI::setDragFirst);
  _fsa->addArc("SetDragFirst2", "Drawing", "Drawing", MinVR::splitStringIntoArray("HybridDraw_StartWithDrag"));
  _fsa->addArcCallback("SetDragFirst2", this, &ForceHybridDrawHCI::setDragFirst);

  _fsa->addArc("SetTapeFirst1", "Start", "Start", MinVR::splitStringIntoArray("HybridDraw_StartWithTape"));
  _fsa->addArcCallback("SetTapeFirst1", this, &ForceHybridDrawHCI::setTapeFirst);
  _fsa->addArc("SetTapeFirst2", "Drawing", "Drawing", MinVR::splitStringIntoArray("HybridDraw_StartWithTape"));
  _fsa->addArcCallback("SetTapeFirst2", this, &ForceHybridDrawHCI::setTapeFirst);

  _fsa->addArc("BackupLength1", "Start", "Start", MinVR::splitStringIntoArray("Drag_BackupLength"));
  _fsa->addArcCallback("BackupLength1", this, &ForceHybridDrawHCI::setDragBackupLength);
  _fsa->addArc("BackupLength2", "Drawing", "Drawing", MinVR::splitStringIntoArray("Drag_BackupLength"));
  _fsa->addArcCallback("BackupLength2", this, &ForceHybridDrawHCI::setDragBackupLength);

  _fsa->addArc("LineLength1", "Start", "Start", MinVR::splitStringIntoArray("Drag_LineLength"));
  _fsa->addArcCallback("LineLength1", this, &ForceHybridDrawHCI::setDragLineLength);
  _fsa->addArc("LineLength2", "Drawing", "Drawing", MinVR::splitStringIntoArray("Drag_LineLength"));
  _fsa->addArcCallback("LineLength2", this, &ForceHybridDrawHCI::setDragLineLength);
}


ForceHybridDrawHCI::~ForceHybridDrawHCI()
{
}

void
ForceHybridDrawHCI::setEnabled(bool b)
{  
  if ((b) && (!_enabled)) {
    _eventMgr->addFsaRef(_fsa);
    _cursors->setBrushCursor(CavePaintingCursors::NO_CURSOR);
    _brush->state->maxPressure = 1.0;
    _brush->state->brushInterface = "ForceHybridDraw";
    _brush->state->brushModel = BrushState::BRUSHMODEL_DEFAULT;
    //_forceNetInterface->startHybridDrawing();
    _dcbid = _gfxMgr->addDrawCallback(this, &ForceHybridDrawHCI::draw);
  }
  else if ((!b) && (_enabled)) {
    _eventMgr->removeFsaRef(_fsa);
    //_forceNetInterface->stopForces();
    _gfxMgr->removeDrawCallback(_dcbid);
  }
  _enabled = b;
}


bool
ForceHybridDrawHCI::canReleaseFocus()
{
  return (_fsa->getCurrentState() == "Start");
}


void
ForceHybridDrawHCI::setTapeFirst(MinVR::EventRef e)
{
  _hybridStateToStart = TapeDrawing;
  if (_fsa->getCurrentState() == "Start") {
    _hybridState = _hybridStateToStart;
  }
}

void
ForceHybridDrawHCI::setDragFirst(MinVR::EventRef e)
{
  _hybridStateToStart = DragDrawing;
  if (_fsa->getCurrentState() == "Start") {
    _hybridState = _hybridStateToStart;
  }
}

void
ForceHybridDrawHCI::setDragBackupLength(MinVR::EventRef e)
{
  _backupLength = e->get1DData();
}

void
ForceHybridDrawHCI::setDragLineLength(MinVR::EventRef e)
{
  _lineLength = e->get1DData();
}

void
ForceHybridDrawHCI::handBtnDown(MinVR::EventRef e)
{
  _hybridState = TapeDrawing;
}

void
ForceHybridDrawHCI::handBtnUp(MinVR::EventRef e)
{
  _hybridState = DragDrawing;
  _reverseTape = false;
}

void
ForceHybridDrawHCI::handMotion(MinVR::EventRef e)
{
  _handPos = e->getCoordinateFrameData().translation;
  _brush->state->handFrame = e->getCoordinateFrameData();

  _handCloseToBrush = false;
  if ((_handPos - _brush->state->frameInRoomSpace.translation).length() < 
    MinVR::ConfigVal("TapeBtnProximityVal", 0.3, false)) {
    _handCloseToBrush = true;
  }
}

void
ForceHybridDrawHCI::headMotion(MinVR::EventRef e)
{
  _brush->state->headFrame = e->getCoordinateFrameData();
}

void 
ForceHybridDrawHCI::brushMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
}

void 
ForceHybridDrawHCI::brushOn(MinVR::EventRef e)
{
  _hybridState = _hybridStateToStart;
  _redrawCutoff = -1;
  _reverseTape = false;
  _inRedrawState = false;
  _brush->startNewMark();
}

void
ForceHybridDrawHCI::reverseTape(MinVR::EventRef e)
{
  _reverseTape = true;
}

void
ForceHybridDrawHCI::brushColorValueChange(MinVR::EventRef e)
{
  _brush->state->colorValue = e->get1DData();
}

void 
ForceHybridDrawHCI::brushPressureChange(MinVR::EventRef e)
{
  _brush->state->pressure = e->get1DData();

  if (MinVR::ConfigVal("ConstantPressure", 0.0, false) != 0.0) {
    _brush->state->pressure = MinVR::ConfigVal("ConstantPressure", 0.0);
  }

  // For this style of controlling the brush, pressure changes both the width and the color 
  // value of the mark.
  if (_brush->state->maxPressure != 0.0) {
    _brush->state->width = _brush->state->pressure/_brush->state->maxPressure * _brush->state->size;
    //_brush->state->colorValue = _brush->state->pressure/_brush->state->maxPressure;
  }

  // This could happen twice per frame if the brush and the pressure
  // both change in the same frame, which is likely, but if we don't
  // do this here, then it won't update when we just change pressure,
  // which is desireable.
  //if ((_redrawCutoff != -1) && (_fsa->getCurrentState() == "Drawing")) {
  //  smoothOutRedrawEdits();
  //}
}

void
ForceHybridDrawHCI::brushPhysicalFrameChange(MinVR::EventRef e)
{
  _brush->state->physicalFrameInRoomSpace = e->getCoordinateFrameData();
}

void 
ForceHybridDrawHCI::brushDrawMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
}

void
ForceHybridDrawHCI::addSample(MinVR::EventRef e)
{
  //_brush->state->width = _brush->state->pressure/_brush->state->maxPressure * _brush->state->size;
  //_brush->state->colorValue = _brush->state->pressure/_brush->state->maxPressure;
  _inRedrawState = false;
  
  if (_hybridState == DragDrawing) {
    Vector3 p1 = _brush->state->physicalFrameInRoomSpace.translation;
    Vector3 p2 = _brush->state->frameInRoomSpace.translation;
    _brush->state->drawingDir = (p1 - p2).unit();
    _brush->addSampleToMark();
    if (_redrawCutoff != -1) {
      smoothOutRedrawEdits();
    }
    else {
      smoothOutPressure();
    }
  }
  else { // TapeDrawing
    _brush->state->drawingDir = (_handPos - _brush->state->frameInRoomSpace.translation).unit();
    if (_reverseTape) {
      _brush->state->drawingDir = -_brush->state->drawingDir;
    }
    _brush->addSampleToMark();
    if (_redrawCutoff != -1) {
      smoothOutRedrawEdits();
    }
    else {
      smoothOutPressure();
    }
  }
}

void
ForceHybridDrawHCI::resizeFromBrushBackup(MinVR::EventRef e)
{
  _inRedrawState = true;
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

  int n = _brush->currentMark->getNumSamples()-1;
  _startDrawingPt = _brush->currentMark->getBrushState(n)->frameInRoomSpace.translation + 
    _lineLength*_brush->currentMark->getBrushState(n)->drawingDir;

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
ForceHybridDrawHCI::brushOff(MinVR::EventRef e)
{
  _brush->endMark();
  _brush->state->width = _brush->state->size;
  _brush->state->colorValue = 1.0;
  _redrawLines.clear();
  _hybridState = _hybridStateToStart;
  _inRedrawState = false;
}


void
ForceHybridDrawHCI::smoothOutPressure()
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

void
ForceHybridDrawHCI::smoothOutRedrawEdits()
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
ForceHybridDrawHCI::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  _cursors->drawPressureMeter();

  double lineRad = MinVR::ConfigVal("Guideline_Rad",0.001, false);
  int    guideNFaces = MinVR::ConfigVal("Guideline_NFaces", 10, false);
  double glr = MinVR::ConfigVal("GuideLineTexRepeat", 0.015, false);
  Color3 baseCol = MinVR::ConfigVal("Guideline_BaseColor", Color3::white(), false);
  Color3 backupCol = MinVR::ConfigVal("Guideline_BackupColor", Color3::yellow(), false);
  
  if (_hybridState == DragDrawing) {
    Color3 flatBrushColor = Color3(1.0, 1.0, 0.8);
    _cursors->drawFlatBrush(rd, _brush->state->frameInRoomSpace, _brush->state->size, flatBrushColor, 
                            _gfxMgr->getTexture("BrushTexture"));

    Vector3 d = _brush->state->frameInRoomSpace.translation;
    Vector3 b = _brush->state->physicalFrameInRoomSpace.translation;
    if (_dispConstLenDragLine) {
      b = d + MinVR::ConfigVal("DragConstLineLength", 0.15, false)*(b-d).unit();
    }

    Vector3 w1, w2, r1, r2;
    // No red line because backup is turned off if line length is very short
    if (_backupLength == 0.0) {
      w1 = b;
      w2 = d;
      r1 = d;
      r2 = d;
    }
    else {
      w1 = b;
      w2 = d + _backupLength*(b-d).unit();
      r1 = w2;
      r2 = d;
    }

    if (_inRedrawState) {
      _cursors->drawSphere(rd, CoordinateFrame(_startDrawingPt), lineRad, Color3::green());
    }
    else if ((b-d).length() < _lineLength) {
      _startDrawingPt = d + _lineLength*(b-d).unit();
      _cursors->drawSphere(rd, CoordinateFrame(_startDrawingPt), lineRad, Color3::green());
    }

    _cursors->drawLineAsCylinder(w1, w2, lineRad, lineRad, rd, baseCol, guideNFaces,
                                 _gfxMgr->getTexture("GuideLineTex"), (w2-w1).length()/glr);
 
    _cursors->drawLineAsCylinder(r1, r2, lineRad, lineRad, rd, backupCol, guideNFaces,
                                 _gfxMgr->getTexture("GuideLineTex"), (r2-r1).length()/glr);
    
 
    // draw an indication for the sphere where if we press down on the
    // hand btn we'll transition to starting with tape drawing, but
    // only if not already painting.
    if ((_handCloseToBrush) && (_fsa->getCurrentState() == "Start")) {
      double r = MinVR::ConfigVal("TapeBtnProximityVal", 0.3, false);
      Sphere s = Sphere(_brush->state->frameInRoomSpace.translation, r);
      Draw::sphere(s, rd, Color4::clear(), flatBrushColor);
    }

  }
  else {  // TapeDrawing
    double rad = MinVR::ConfigVal("CavePaintingCursors_HandCursorSize",0.05,false);
    _cursors->drawSphere(rd, _brush->state->physicalFrameInRoomSpace, rad/3.0, Color3(0.61,0.72,0.92));
    Vector3 lineEndPt = _handPos;
    if (!_reverseTape) {
      _cursors->drawDrawingGuides();
    }
    else {
      Vector3 dir = (_brush->state->frameInRoomSpace.translation - _handPos).unit();
      lineEndPt = _brush->state->frameInRoomSpace.translation + 0.4*dir;

      // also draw a line from the hand to the brush that extends the tangent line drawn below
      _cursors->drawLineAsCylinder(_brush->state->frameInRoomSpace.translation, 
                                   _handPos, lineRad, lineRad, rd, backupCol, guideNFaces,
                                   _gfxMgr->getTexture("GuideLineTex"), 
                                   (_brush->state->frameInRoomSpace.translation - _handPos).length()/glr);

    }
      
    _cursors->drawLineAsCylinder(_brush->state->frameInRoomSpace.translation, 
                                 lineEndPt, lineRad, lineRad, rd, baseCol, guideNFaces,
                                 _gfxMgr->getTexture("GuideLineTex"), 
                                 (_brush->state->frameInRoomSpace.translation - lineEndPt).length()/glr);


    Color3 flatBrushColor = Color3(1.0, 1.0, 0.8);
    _cursors->drawFlatBrush(rd, _brush->state->frameInRoomSpace, _brush->state->size, flatBrushColor, 
                            _gfxMgr->getTexture("BrushTexture"));
  }

  rd->pushState();
  rd->setObjectToWorldMatrix(virtualToRoomSpace);
  rd->disableLighting();
  rd->setColor(MinVR::ConfigVal("BackupPointsColor", Color3(0.8, 0.35, 0.35), false));
  rd->setPointSize(2.0);
  int smax = 1;
  if (getShadowsOn()) {
    smax = 2;
  }
  for (int s=0;s<smax;s++) {
    if (s==1) {
      pushShadowState(rd);
    }
    for (int i=0;i<_redrawLines.size();i++) {
      rd->beginPrimitive(PrimitiveType::POINTS);
      for (int j=0;j<_redrawLines[i].size();j++) {
        rd->sendVertex(_redrawLines[i][j]);
      }
      rd->endPrimitive();
    }
    if (s==1) {
      popShadowState(rd);
    }
  }
  rd->popState();
}


} // end namespace

