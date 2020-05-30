
#include "MoveMarksHCI.H"
#include <ConfigVal.H>

using namespace G3D;
namespace DrawOnAir {

MoveMarksHCI::MoveMarksHCI(Array<std::string>     motionTriggers, 
                           Array<std::string>     onTriggers,
                           Array<std::string>     offTriggers,
                           ArtworkRef             artwork,
                           BrushRef               brush,
                           CavePaintingCursorsRef cursors,
  EventMgrRef            eventMgr,
  GfxMgrRef              gfxMgr,
                           HistoryRef             history)
{
  _artwork = artwork;
  _brush = brush;
  _cursors = cursors;
  _eventMgr = eventMgr;
  _gfxMgr = gfxMgr;
  _history = history;
  _enabled = false;

  _fsa = new Fsa("MoveMarksHCI");
  _fsa->addState("Start");
  _fsa->addState("Dragging");
  _fsa->addArc("Move", "Start", "Start", motionTriggers);
  _fsa->addArcCallback("Move", this, &MoveMarksHCI::motion);
  _fsa->addArc("On", "Start", "Dragging", onTriggers);
  _fsa->addArcCallback("On", this, &MoveMarksHCI::on);
  _fsa->addArc("DragMove", "Dragging", "Dragging", motionTriggers);
  _fsa->addArcCallback("DragMove", this, &MoveMarksHCI::dragMotion);
  _fsa->addArc("Off", "Dragging", "Start", offTriggers);
  _fsa->addArcCallback("Off", this, &MoveMarksHCI::off);
}

MoveMarksHCI::~MoveMarksHCI()
{
}

void
MoveMarksHCI::setEnabled(bool b) 
{
  if ((b) && (!_enabled)) {
    _eventMgr->addFsaRef(_fsa);
    _cursors->setBrushCursor(CavePaintingCursors::POINTER_CURSOR);
    _drawID = _gfxMgr->addDrawCallback(this, &MoveMarksHCI::draw);
    buildBSPTree(_artwork, _tree, MinVR::ConfigVal("Selection_HandleLen", 0.025, false), _gfxMgr);
  }
  else if ((!b) && (_enabled)) {
    _eventMgr->removeFsaRef(_fsa);
    _gfxMgr->removeDrawCallback(_drawID);
    if (_selectedMark.notNull()) {
      _selectedMark->setHighlighted(false, _artwork);
      _selectedMark = NULL;
    }
  }
  _enabled = b;
}

bool
MoveMarksHCI::canReleaseFocus()
{
  return (_fsa->getCurrentState() == "Start");
}


void 
MoveMarksHCI::motion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  Vector3 pos = _gfxMgr->roomPointToVirtualSpace(e->getCoordinateFrameData().translation);

  double s = MinVR::ConfigVal("Selection_CursorBoxSize", 0.2, false) / 2.0;
  Vector3 s3 = Vector3(s,s,s);
  _intersectedBoxes.clear();
  _tree.getIntersectingMembers(AABox(pos-s3, pos+s3), _intersectedBoxes);
  
  MarkRef selection;
  MarkBox selBox;
  if (_intersectedBoxes.size() == 1) {
    selection = _intersectedBoxes[0].mark;
    _indexOfClosestPt = _intersectedBoxes[0].sample;
    selBox = _intersectedBoxes[0];
  }
  else if (_intersectedBoxes.size() > 1) {
    selection = _intersectedBoxes[0].mark;
    _indexOfClosestPt = _intersectedBoxes[0].sample;
    selBox = _intersectedBoxes[0];

    double dist = (pos - _intersectedBoxes[0].box.center()).length();
    for (int i=0;i<_intersectedBoxes.size();i++) {
      double d = (pos - _intersectedBoxes[i].box.center()).length();
      if (d < dist) {
        dist = d;
        selection = _intersectedBoxes[i].mark;
        _indexOfClosestPt = _intersectedBoxes[i].sample;
        selBox = _intersectedBoxes[i];
      }
    }
  }

  if (selection.isNull()) {
    // No mark is selected
    if (_selectedMark.notNull()) {
      _selectedMark->setHighlighted(false, _artwork);
      _selectedMark = NULL;
    }
  }
  else {

    // Selected the same mark, but a different point on it
    if ((_selectedMark == selection) && (_selectedBox.sample != selBox.sample)) {
      _selectedBox = selBox;
    }
    else {
      // Selected a different mark
      if (_selectedMark.notNull()) {
        _selectedMark->setHighlighted(false, _artwork);
        _selectedMark = NULL;
      }

      _selectedMark = selection;
      _selectedBox = selBox;
      _selectedMark->setHighlighted(true, _artwork);
    }
  }



  /****
  Array<int> hiddenLayers = _artwork->getHiddenLayers();
  Array<MarkRef> touchingMarks;
  Array<MarkRef> marks = _brush->getArtwork()->getMarks();
  for (int i=0;i<marks.size();i++) {
    if ((marks[i]->getNumSamples()) && 
        (!hiddenLayers.contains(marks[i]->getInitialBrushState()->layerIndex)) &&
        (marks[i]->getInitialBrushState()->frameIndex == _artwork->getCurrentFrame())) {
      if (marks[i]->boundingBoxContains(pos)) {
        touchingMarks.append(marks[i]);
      }
    }
  }

  MarkRef selection;
  // if more than one mark is touching the cursor pick the one closest to the cursor
  if (touchingMarks.size() == 1) {
    selection = touchingMarks[0];
  }
  else if (touchingMarks.size() > 1) {
    // check start and end points of each mark, and select the closest one
    double dist = inf();
    for (int i=0;i<touchingMarks.size();i++) {
      if (touchingMarks[i]->getNumSamples() >= 2) {
        double newdist = G3D::min((touchingMarks[i]->getSamplePosition(0) - pos).length(),
            (touchingMarks[i]->getSamplePosition(touchingMarks[i]->getNumSamples()-1) - pos).length());
        if (newdist < dist) {
          dist = newdist;
          selection = touchingMarks[i];
        }
      }
    }
  }
  if (_selectedMark.notNull()) {
    _selectedMark->setHighlighted(false);
    _selectedMark = NULL;
  }
  if (selection.notNull()) {
    selection->setHighlighted(true);
    _selectedMark = selection;
    _indexOfClosestPt = 0;
    //double d = selection->approxDistanceToMark(pos, _indexOfClosestPt);
    //cout << "Selection" << endl;
  }
  else {
    //cout << "No" << endl;
  }

  _pointsToDisplay.clear();
  if (touchingMarks.size()) {
    for (int i=0;i<touchingMarks.size();i++) {
      if (touchingMarks[i]->getNumSamples() >= 2) {
        _pointsToDisplay.append(touchingMarks[i]->getSamplePosition(0));
        _pointsToDisplay.append(touchingMarks[i]->getSamplePosition(touchingMarks[i]->getNumSamples()-1));
      }
    }
  }
  ***/
}

void 
MoveMarksHCI::on(MinVR::EventRef e)
{
  _lastFrame = CoordinateFrame();
  if (_selectedMark.notNull()) {
    _selectedMark->startEditing(_artwork);
  }
}

void
MoveMarksHCI::dragMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  CoordinateFrame eVS = _gfxMgr->roomToVirtualSpace(e->getCoordinateFrameData());
  eVS.rotation.orthonormalize();

  if (_selectedMark.notNull()) {
    if (_lastFrame != CoordinateFrame()) {
      CoordinateFrame delta = eVS * _lastFrame.inverse();
      delta.rotation.orthonormalize();

      // Setup transformation to rotate around a point on the point on
      // the mark that is closest to the tracker

      //CoordinateFrame R = CoordinateFrame(-delta.translation) * delta;
      //CoordinateFrame T = delta.translation;


      Vector3 markPt = _selectedMark->getSamplePosition(_indexOfClosestPt);
      CoordinateFrame xform = 
        CoordinateFrame(eVS.translation - _lastFrame.translation) *
        CoordinateFrame(markPt) *  // translate back to VS coords
        CoordinateFrame(delta.rotation, Vector3::zero()) * // rotate about markPt
        CoordinateFrame(-markPt);   // convert VS coords to system centered at markPt
      xform.rotation.orthonormalize();
      
      _selectedMark->transformBy(xform);

      //_selectedMark->transformBy(CoordinateFrame(eVS.translation - _lastFrame.translation));

    }
    _lastFrame = eVS;
  }
}

void
MoveMarksHCI::off(MinVR::EventRef e)
{
  if (_selectedMark.notNull()) {
    _selectedMark->stopEditing(_artwork);
  }
  // Need to rebuild the tree, since this mark moved.
  // TODO: Faster to just remove and reinsert the entries for this mark
  buildBSPTree(_artwork, _tree, MinVR::ConfigVal("Selection_HandleLen", 0.025, false), _gfxMgr);
}

void
MoveMarksHCI::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  rd->pushState();
  rd->setObjectToWorldMatrix(virtualToRoomSpace);

  double r = MinVR::ConfigVal("Selection_BoxSize", 0.01, false);
  r = _gfxMgr->roomVectorToVirtualSpace(Vector3(r,0,0)).length();
  for (int i=0;i<_intersectedBoxes.size();i++) {
    Vector3 p1 = _intersectedBoxes[i].box.center();
    Vector3 p2 = _intersectedBoxes[i].mark->getSamplePosition(_intersectedBoxes[i].sample);
    Color3 col = Color3::orange();
    
    if ((_intersectedBoxes[i].mark == _selectedBox.mark) &&
        (_intersectedBoxes[i].sample == _selectedBox.sample)) {
      col = Color3::red();
    }

    
    if ((_fsa->getCurrentState() == "Dragging") && (_intersectedBoxes[i].mark == _selectedMark)) {
      // don't draw handles because it looks weird when you stretch them
    }
    else {
      rd->setColor(col);
      rd->disableLighting();
      rd->beginPrimitive(PrimitiveType::LINES);
      rd->sendVertex(p1);
      rd->sendVertex(p2);
      rd->endPrimitive();
    }

    rd->enableLighting();
    Draw::sphere(Sphere(p1, r), rd, col, Color4::clear());
  }

  rd->popState();
}



} // end namespace
