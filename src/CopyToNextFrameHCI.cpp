#include <ConfigVal.H>
#include "CopyToNextFrameHCI.H"
using namespace G3D;
namespace DrawOnAir {

CopyToNextFrameHCI::CopyToNextFrameHCI(Array<std::string>     motionTriggers, 
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

  _fsa = new Fsa("CopyToNextFrameHCI");
  _fsa->addState("Start");
  _fsa->addState("Pressed");
  _fsa->addArc("Move", "Start", "Start", motionTriggers);
  _fsa->addArcCallback("Move", this, &CopyToNextFrameHCI::motion);
  _fsa->addArc("On", "Start", "Pressed", onTriggers);
  _fsa->addArcCallback("On", this, &CopyToNextFrameHCI::on);
  _fsa->addArc("Off", "Pressed", "Start", offTriggers);
  _fsa->addArcCallback("Off", this, &CopyToNextFrameHCI::off);
}

CopyToNextFrameHCI::~CopyToNextFrameHCI()
{
}

void
CopyToNextFrameHCI::setEnabled(bool b) 
{
  if ((b) && (!_enabled)) {
    _eventMgr->addFsaRef(_fsa);
    _cursors->setBrushCursor(CavePaintingCursors::POINTER_CURSOR);
    _drawid = _gfxMgr->addDrawCallback(this, &CopyToNextFrameHCI::draw);
    buildBSPTree(_artwork, _tree, MinVR::ConfigVal("Selection_HandleLen", 0.025, false), _gfxMgr);
    _treeFrame = _artwork->getCurrentFrame();
  }
  else if ((!b) && (_enabled)) {
    _eventMgr->removeFsaRef(_fsa);
    _gfxMgr->removeDrawCallback(_drawid);
    if (_selectedMark.notNull()) {
      _selectedMark->setHighlighted(false, _artwork);
      _selectedMark = NULL;
    }
  }
  _enabled = b;
}

bool
CopyToNextFrameHCI::canReleaseFocus()
{
  return (_fsa->getCurrentState() == "Start");
}

void 
CopyToNextFrameHCI::motion(MinVR::EventRef e)
{
  // If some interaction, like the keyboard changes the current frame
  // on us, then the bsptree will be out of date, so check here to
  // make sure the tree we're using was built using the current frame.
  if (_treeFrame != _artwork->getCurrentFrame()) {
    buildBSPTree(_artwork, _tree, MinVR::ConfigVal("Selection_HandleLen", 0.025, false), _gfxMgr);
    _treeFrame = _artwork->getCurrentFrame();
  }
  

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
    selBox = _intersectedBoxes[0];
  }
  else if (_intersectedBoxes.size() > 1) {
    selection = _intersectedBoxes[0].mark;
    selBox = _intersectedBoxes[0];

    double dist = (pos - _intersectedBoxes[0].box.center()).length();
    for (int i=0;i<_intersectedBoxes.size();i++) {
      double d = (pos - _intersectedBoxes[i].box.center()).length();
      if (d < dist) {
        dist = d;
        selection = _intersectedBoxes[i].mark;
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

}

void 
CopyToNextFrameHCI::on(MinVR::EventRef e)
{
  if ((_selectedMark.notNull()) && 
      (_selectedMark->getNumSamples()) &&
      (_artwork->getNumFrames() > 1)) {
    _selectedMark->setHighlighted(false, _artwork);

    int to = _selectedMark->getInitialBrushState()->frameIndex + 1;
    if (to >= _artwork->getNumFrames()) {
      to = 0;
    }

    MarkRef myCopy = _selectedMark->copy();
    for (int i=0;i<myCopy->getNumSamples();i++) {
      myCopy->getBrushState(i)->frameIndex = to;
    }
    _artwork->addMark(myCopy);
    myCopy->commitGeometry(_artwork);
  }
}

void
CopyToNextFrameHCI::off(MinVR::EventRef e)
{
}


void
CopyToNextFrameHCI::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
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

    rd->setColor(col);
    rd->disableLighting();
    rd->beginPrimitive(PrimitiveType::LINES);
    rd->sendVertex(p1);
    rd->sendVertex(p2);
    rd->endPrimitive();

    rd->enableLighting();
    Draw::sphere(Sphere(p1, r), rd, col, Color4::clear());
  }

  rd->popState();
}



} // end namespace
