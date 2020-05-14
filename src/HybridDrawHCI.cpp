#include <ConfigVal.H>
#include <G3DOperators.h>
#include "HybridDrawHCI.H"
using namespace G3D;
namespace DrawOnAir {

HybridDrawHCI::HybridDrawHCI(Array<std::string>     brushOnTriggers,
                             Array<std::string>     brushMotionTriggers, 
                             Array<std::string>     brushOffTriggers,
                             Array<std::string>     handMotionTriggers,
                             Array<std::string>     handOnTriggers,
                             Array<std::string>     handOffTriggers,
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
  _lineLength = MinVR::ConfigVal("DragDrawing_LineLength", 0.15, false);
  _initLineLength = MinVR::ConfigVal("DragDrawing_InitialLineLength", 0.33*_lineLength, false);
  _reverse = false;
  _dispConstLenDragLine = false;

  _fsa = new MinVR::Fsa("HybridDrawHCI");
  _fsa->addState("Start");
  _fsa->addState("Drag");
  _fsa->addState("Tape");

  // Start State
  _fsa->addArc("BrushMove", "Start", "Start", brushMotionTriggers);
  _fsa->addArcCallback("BrushMove", this, &HybridDrawHCI::brushMotion);

  _fsa->addArc("HandMove1", "Start", "Start", handMotionTriggers);
  _fsa->addArcCallback("HandMove1", this, &HybridDrawHCI::handMotion);

  _fsa->addArc("HeadMove1", "Start", "Start", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove1", this, &HybridDrawHCI::headMotion);

  _fsa->addArc("BrushPressure1", "Start", "Start", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure1", this, &HybridDrawHCI::brushPressureChange);



  // Drag State
  _fsa->addArc("BrushOn", "Start", "Drag", brushOnTriggers);
  _fsa->addArcCallback("BrushOn", this, &HybridDrawHCI::brushOn);

  _fsa->addArc("DragMove", "Drag", "Drag", brushMotionTriggers);
  _fsa->addArcCallback("DragMove", this, &HybridDrawHCI::dragMotion);

  _fsa->addArc("BrushPressure2", "Drag", "Drag", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure2", this, &HybridDrawHCI::brushPressureChange);

  _fsa->addArc("HandMove2", "Drag", "Drag", handMotionTriggers);
  _fsa->addArcCallback("HandMove2", this, &HybridDrawHCI::handMotion);

  _fsa->addArc("HeadMove2", "Drag", "Drag", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove2", this, &HybridDrawHCI::headMotion);

  _fsa->addArc("BrushOff1", "Drag", "Start", brushOffTriggers);
  _fsa->addArcCallback("BrushOff1", this, &HybridDrawHCI::brushOff);

  _fsa->addArc("DragToTape", "Drag", "Tape", handOnTriggers);
  _fsa->addArcCallback("DragToTape", this, &HybridDrawHCI::dragToTape);



  // Tape State
  _fsa->addArc("TapeMove", "Tape", "Tape", brushMotionTriggers);
  _fsa->addArcCallback("TapeMove", this, &HybridDrawHCI::tapeMotion);

  _fsa->addArc("BrushPressure3", "Tape", "Tape", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure3", this, &HybridDrawHCI::brushPressureChange);

  _fsa->addArc("HandMove3", "Tape", "Tape", handMotionTriggers);
  _fsa->addArcCallback("HandMove3", this, &HybridDrawHCI::handMotion);

  _fsa->addArc("HeadMove3", "Tape", "Tape", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove3", this, &HybridDrawHCI::headMotion);

  _fsa->addArc("BrushOff2", "Tape", "Start", brushOffTriggers);
  _fsa->addArcCallback("BrushOff2", this, &HybridDrawHCI::brushOff);

  _fsa->addArc("TapeToDrag", "Tape", "Drag", handOffTriggers);
  _fsa->addArcCallback("TapeToDrag", this, &HybridDrawHCI::tapeToDrag);

}

HybridDrawHCI::~HybridDrawHCI()
{
}

void
HybridDrawHCI::setEnabled(bool b) 
{
  if ((b) && (!_enabled)) {
    _eventMgr->addFsaRef(_fsa);
    _cursors->setBrushCursor(CavePaintingCursors::NO_CURSOR);
    _cursors->setHandCursor(CavePaintingCursors::NO_CURSOR);
    _brush->state->maxPressure = MinVR::ConfigVal("HybridDrawHCI_MaxPressure", 1.0, false);
    _brush->state->brushInterface = "HybridDraw";
    _drawID = _gfxMgr->addDrawCallback(this, &HybridDrawHCI::draw);
  }
  else if ((!b) && (_enabled)) {
    _eventMgr->removeFsaRef(_fsa);
    _gfxMgr->removeDrawCallback(_drawID);
    _cursors->setHandCursor(CavePaintingCursors::DEFAULT_CURSOR);
  }
  _enabled = b;
}

bool
HybridDrawHCI::canReleaseFocus()
{
  return (_fsa->getCurrentState() == "Start");
}

void
HybridDrawHCI::setLineLength(double d)
{
  _lineLength = d;
}

void 
HybridDrawHCI::handMotion(MinVR::EventRef e)
{
  _brush->state->handFrame = e->getCoordinateFrameData();
  _brush->state->handFrame.translation += _handOffset;

  _handPos = _brush->state->handFrame.translation;
}

void 
HybridDrawHCI::headMotion(MinVR::EventRef e)
{
  _brush->state->headFrame = e->getCoordinateFrameData();
}

void 
HybridDrawHCI::brushMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  _brush->state->physicalFrameInRoomSpace = e->getCoordinateFrameData();
  _brushFrame = _brush->state->frameInRoomSpace;
  _brushFrame.translation += _brushOffset;

  _dragFrame = _brushFrame;
}

void 
HybridDrawHCI::brushPressureChange(MinVR::EventRef e)
{
  _brush->state->pressure = e->get1DData();
  //cout << "new pressure " << _brush->state->pressure << endl;
  _brush->state->width = _brush->state->pressure/_brush->state->maxPressure * _brush->state->size;
  _brush->state->colorValue = _brush->state->pressure/_brush->state->maxPressure;
}



void 
HybridDrawHCI::brushOn(MinVR::EventRef e)
{
  _interpLineLength = _initLineLength;
  _brush->startNewMark();
}

void 
HybridDrawHCI::dragMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  _brush->state->physicalFrameInRoomSpace = e->getCoordinateFrameData();
  _brushFrame = _brush->state->frameInRoomSpace;
  _brushFrame.translation += _brushOffset;

  _dragFrame.rotation = _brushFrame.rotation;

  // grow the interpolated line
  double pathLength = _brush->currentMark->getArcLength();
  if (pathLength >= _lineLength) {
    _interpLineLength = _lineLength;
  }
  else {
    _interpLineLength = lerp(_initLineLength, _lineLength, pathLength / _lineLength);
  }


  Vector3 currentPos = _brushFrame.translation;
  Vector3 drawDir = (currentPos - _dragFrame.translation).unit();
  _brush->state->drawingDir = drawDir;

  if ((currentPos - _dragFrame.translation).length() > _interpLineLength) {
    Vector3 oldPt = _dragFrame.translation;
    _dragFrame.translation = currentPos - _interpLineLength*drawDir;

    // add this sample to the path
    _brush->state->frameInRoomSpace.translation = _dragFrame.translation;
    _markPath.append(_dragFrame.translation);
    _brush->addSampleToMark();
    _brush->state->frameInRoomSpace.translation = e->getCoordinateFrameData().translation;
  }
}

void
HybridDrawHCI::brushOff(MinVR::EventRef e)
{
  _brush->endMark();
  _brushOffset = Vector3::zero();
  _handOffset = Vector3::zero();
  _reverse = false;
}

void
HybridDrawHCI::dragToTape(MinVR::EventRef e)
{
  _lastPointOnPath = _dragFrame.translation;
  _brushOffset = _dragFrame.translation - _brush->state->frameInRoomSpace.translation;
  _lastBrushPos = Vector3::zero();

  // Apply a hand offset to make it match the tangent of the current line
  Vector3 tangent = (_brushFrame.translation - _dragFrame.translation).unit();
  G3D::Line l = G3D::Line::fromPointAndDirection(_dragFrame.translation, tangent);
  Vector3 newHandPos = l.closestPoint(_handPos);
  // Engage reverse tape drawing if moving in a direction opposite of the hand
  _reverse = false;
  if ((newHandPos - _dragFrame.translation).dot(tangent) < 0.0) {
    _reverse = true;
  }      
  Vector3 smoothingOffset = (newHandPos - _handPos);
  _handOffset = _handOffset + smoothingOffset;
  _handPos = _handPos + smoothingOffset;
}


void
HybridDrawHCI::tapeMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  _brush->state->physicalFrameInRoomSpace = e->getCoordinateFrameData();
  _brushFrame = _brush->state->frameInRoomSpace;
  _brushFrame.translation += _brushOffset;


  // dir is the drawing direction
  Vector3 dir = (_handPos - _lastPointOnPath).unit();
  if (_reverse) {
    dir = -dir;
  }

  Vector3 newPointOnPath;
  bool addSample = false;
  if (MinVR::ConfigVal("TapeDrawing_CaveStyleAdvancement", 0, false)) {
    // Cave (non-haptic) mode, advance based on brush movement, not projection
    if (_lastBrushPos != Vector3::zero()) {
      Vector3 delta = _brushFrame.translation - _lastBrushPos;
      double moveInDrawDir = delta.dot(dir);
      if (moveInDrawDir > 0.0) {
        addSample = true;
        newPointOnPath = _lastPointOnPath + moveInDrawDir*dir;
      }
    }
    _lastBrushPos = _brushFrame.translation;
  }
  else {
    Vector3 endPt = _lastPointOnPath + 5.0*dir; 
    G3D::Line l = G3D::Line::fromTwoPoints(_lastPointOnPath, endPt);
    newPointOnPath = l.closestPoint(_brushFrame.translation);
    
    double dotForward = (newPointOnPath - _lastPointOnPath).dot(dir);
    if (dotForward > 0.005) {
      addSample = true;
    }
  }
  
  if (addSample) {
    // advance brush
    _brushFrame.translation = newPointOnPath;
    
    // add this sample to the path
    CoordinateFrame origFrame = _brush->state->frameInRoomSpace;
    _brush->state->frameInRoomSpace.translation = _brushFrame.translation;
    _markPath.append(_brushFrame.translation);
    _brush->addSampleToMark();
    _brush->state->frameInRoomSpace = origFrame;
    _lastPointOnPath = newPointOnPath;
  }
  else {
    // visually restrict motion of brush, pin it to the last sample
    _brushFrame.translation = _lastPointOnPath;
  }
}


void
HybridDrawHCI::tapeToDrag(MinVR::EventRef e)
{
  _dragFrame = _brushFrame;

  if (_markPath.size()) {
    Vector3 tangent = (_handPos - _markPath.last()).unit();
    if (_reverse) {
      tangent = -tangent;
    }
    Vector3 idealBrushLoc = _markPath.last() + 0.95*_lineLength*tangent;
    _brushOffset = idealBrushLoc - _brush->state->frameInRoomSpace.translation;
  }

  _handOffset = Vector3::zero();
}




void
HybridDrawHCI::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  _cursors->drawPressureMeter();

  double lineRad = MinVR::ConfigVal("Guideline_Rad",0.001, false);
  int    guideNFaces = MinVR::ConfigVal("Guideline_NFaces", 10, false);
  double glr = MinVR::ConfigVal("GuideLineTexRepeat", 0.015, false);
  Color3 baseCol = MinVR::ConfigVal("Guideline_BaseColor", Color3::white(), false);
  Color3 backupCol = MinVR::ConfigVal("Guideline_BackupColor", Color3::red(), false);

  if ((_fsa->getCurrentState() == "Drag") || (_fsa->getCurrentState() == "Start")) {
    Color3 flatBrushColor = Color3(1.0, 1.0, 0.8);
    _cursors->drawFlatBrush(rd, _dragFrame, _brush->state->size, flatBrushColor, 
                            _gfxMgr->getTexture("BrushTexture"));

    double l = _lineLength;
    Vector3 d = _dragFrame.translation;
    Vector3 b = _brushFrame.translation;
    if (_dispConstLenDragLine) {
      b = d + l*(b-d).unit();
    }

    Vector3 w1, w2, r1, r2;
    // draw the whole line in ropeCol if it's short because we've just
    // started to draw it
    /**if ((_brush->currentMark.notNull()) && 
       (_brush->currentMark->getArcLength() < 0.5*l)) {**/
      w1 = b;
      w2 = d;
      r1 = d;
      r2 = d;
      /**    }
    else { // line length is greater than the backup dist
      w1 = b;
      w2 = d + 0.5*l*(b-d).unit();
      r1 = w2;
      r2 = d;
      }**/

    _cursors->drawLineAsCylinder(w1, w2, lineRad, lineRad, rd, baseCol, guideNFaces, 
                                 _gfxMgr->getTexture("GuideLineTex"), (w2-w1).length()/glr);
    _cursors->drawLineAsCylinder(r1, r2, lineRad, lineRad, rd, backupCol, guideNFaces, 
                                 _gfxMgr->getTexture("GuideLineTex"), (r2-r1).length()/glr);

    
    Vector3 dragLine = _brushFrame.translation - _dragFrame.translation;
    if (dragLine.length() < _interpLineLength) {
      Vector3 drawingStartsPt = _dragFrame.translation + _interpLineLength*dragLine.unit();
      _cursors->drawSphere(rd, CoordinateFrame(drawingStartsPt), lineRad, Color3::green());
    }
    

    /***
    // draw an indication for the sphere where if we press down on the
    // hand btn we'll transition to starting with tape drawing, but
    // only if not already painting.
    if ((_handCloseToBrush) && (_fsa->getCurrentState() == "Start")) {
      double r = ConfigVal("TapeBtnProximityVal", 0.3, false);
      Sphere s = Sphere(_brush->state->frameInRoomSpace.translation, r);
      Draw::sphere(s, rd, Color4::clear(), flatBrushColor);
    }
    ***/
  }
  else {  // TapeDrawing
    double rad = MinVR::ConfigVal("CavePaintingCursors_HandCursorSize",0.05,false);
    _cursors->drawSphere(rd, _brush->state->physicalFrameInRoomSpace, rad/3.0, Color3(0.61,0.72,0.92));
    Vector3 lineEndPt = _handPos;
    //  _cursors->drawDrawingGuides();
   
    if (_reverse) {
      Vector3 dir = (_brushFrame.translation - _handPos).unit();
      lineEndPt = _brushFrame.translation + MinVR::ConfigVal("ReverseTape_LineLength", 0.4, false)*dir;
      
      // also draw a line from the hand to the brush that extends the tangent line drawn below
      _cursors->drawLineAsCylinder(_brushFrame.translation, _handPos, lineRad, lineRad, 
                                   rd, backupCol, guideNFaces, _gfxMgr->getTexture("GuideLineTex"),
                                   (_brushFrame.translation - _handPos).length()/glr);
    }
    
    _cursors->drawLineAsCylinder(_brushFrame.translation, lineEndPt, lineRad, lineRad, 
                                 rd, baseCol, guideNFaces, _gfxMgr->getTexture("GuideLineTex"),
                                 (_brushFrame.translation - lineEndPt).length()/glr);

    Color3 flatBrushColor = Color3(1.0, 1.0, 0.8);
    _cursors->drawFlatBrush(rd, _brushFrame, _brush->state->size, flatBrushColor, 
                            _gfxMgr->getTexture("BrushTexture"));
  }


  // take over drawing of hand cursor here, since there may be an offset applied
  _cursors->drawSphere(rd, CoordinateFrame(_handPos), 
    MinVR::ConfigVal("CavePaintingCursors_HandCursorSize",0.05,false),
                       Color3(0.61, 0.92, 0.72));

}


} // end namespace
