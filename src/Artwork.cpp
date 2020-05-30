
#include <G3DOperators.h>
#include <LoadingScreen.H>
#include "Artwork.H"
#include "AnnotationMark.H"
#include "BrushState.H"
#include "LightingIO.H"
#include "PointModel.H"
#include "PointModelSprites.H"
#include "PointModelSW.H"
#include "PointSplatMark.H"
#include "FlatTubeMark.H"
#include "RibbonMark.H"
#include "TubeMark.H"
#include "SlideMark.H"

using namespace G3D;

namespace DrawOnAir {



Artwork::Artwork(GfxMgrRef gfxMgr)
{
  _gfxMgr = gfxMgr;
  _cbidPose = _gfxMgr->addPoseCallback(this, &Artwork::pose);
  _cbidDraw = _gfxMgr->addDrawCallback(this, &Artwork::draw);
  _maxLayerID = 0;
  _frame = 0;
  _numFrames = 1;

  _annotationModel = new AnnotationModel(_gfxMgr);
  //if ((Shader::supportsPixelShaders()) && (!ConfigVal("PointModel_ForceSW", false, false))) {
  //  _pointModel = new PointModelSprites(_gfxMgr);
  //}
  //else {
  //  _pointModel = new PointModelSW(_gfxMgr);
  //}
  _triStripModel = new TriStripModel(_gfxMgr);
}

Artwork::~Artwork()
{
  _gfxMgr->removePoseCallback(_cbidPose);
  _gfxMgr->removeDrawCallback(_cbidDraw);
}

void
Artwork::addMark(MarkRef mark)
{
  _marks.append(mark);
  if (mark->getShouldBeDrawn()) {
    _marksToDraw.append(mark);
  }
  if (mark->getShouldBePosed()) {
    _marksToPose.append(mark);
  }
  mark->show();
}

void
Artwork::removeMark(MarkRef mark)
{
  int i = _marks.findIndex(mark);
  if (i != -1) {
    _marks.fastRemove(i);
  }

  i = _marksToDraw.findIndex(mark);
  if (i != -1) {
    _marksToDraw.fastRemove(i);
  }

  i = _marksToPose.findIndex(mark);
  if (i != -1) {
    _marksToPose.fastRemove(i);
  }
  mark->stopDrawing();
}

void
Artwork::removeFromMarksToDraw(MarkRef mark)
{
  int i = _marksToDraw.findIndex(mark);
  if (i != -1) {
    _marksToDraw.fastRemove(i);
  }
}

void
Artwork::addToMarksToDraw(MarkRef mark)
{
  _marksToDraw.append(mark);
}

void
Artwork::removeFromMarksToPose(MarkRef mark)
{
  int i = _marksToPose.findIndex(mark);
  if (i != -1) {
    _marksToPose.fastRemove(i);
  }
}

void
Artwork::addToMarksToPose(MarkRef mark)
{
  _marksToPose.append(mark);
}

void
Artwork::clear()
{
  for (int i=_marks.size()-1;i>=0;i--) {
    removeMark(_marks[i]);
  }
  _maxLayerID = 0;
  _frame = 0;
  _numFrames = 1;
}


void
Artwork::setHiddenLayers(const Array<int> &layers)
{
  _hiddenLayers = layers;
  _triStripModel->setHiddenLayers(_hiddenLayers);
  _annotationModel->setHiddenLayers(_hiddenLayers);
}

void
Artwork::showAllLayers()
{
  _hiddenLayers.clear();
  _triStripModel->setHiddenLayers(_hiddenLayers);
  _annotationModel->setHiddenLayers(_hiddenLayers);
}

void
Artwork::hideAllButThisLayer(int layerID)
{
  _hiddenLayers.clear();
  for (int i=0;i<=_maxLayerID;i++) {
    if (i != layerID) {
      _hiddenLayers.append(i);
    }
  }
  _triStripModel->setHiddenLayers(_hiddenLayers);
  _annotationModel->setHiddenLayers(_hiddenLayers);
}

void
Artwork::showFrame(int frame)
{
  debugAssert(frame < _numFrames);
  _frame = frame;
  _triStripModel->setMovieFrame(_frame);
  _annotationModel->setMovieFrame(_frame);
}

void
Artwork::setNumFrames(int numFrames)
{
  if (numFrames > _numFrames) {
    _numFrames = numFrames;
  }
}

void
Artwork::nextFrame()
{
  _frame++;
  if (_frame >= _numFrames) {
    _frame = 0;
  }
  showFrame(_frame);
}

void
Artwork::previousFrame()
{
  _frame--;
  if (_frame < 0) {
    _frame = _numFrames-1;
  }
  showFrame(_frame);
}

void
Artwork::pose(Array<PosedModel::Ref> &posedModels, const CoordinateFrame &virtualToRoomSpace)
{
  for (int i=0;i<_marksToPose.size();i++) {
    _marksToPose[i]->pose(posedModels, virtualToRoomSpace);
  }

  // This poses all marks that are made up of PointSplats
  if (_pointModel.notNull()) {
    posedModels.append(_pointModel->pose(virtualToRoomSpace));
  }

  posedModels.append(_triStripModel->pose(virtualToRoomSpace));
  
  _annotationModel->pose(posedModels, virtualToRoomSpace);
}

void
Artwork::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  for (int i=0;i<_marksToDraw.size();i++) {
    if ((_marksToDraw[i]->getNumSamples()) && 
        (_marksToDraw[i]->getInitialBrushState()->frameIndex == _frame) &&
        (!_hiddenLayers.contains(_marksToDraw[i]->getInitialBrushState()->layerIndex))) {
      _marksToDraw[i]->draw(rd, virtualToRoomSpace);
    }
  }
  _annotationModel->draw(rd, virtualToRoomSpace);
}

void
Artwork::copyFrameToFrame(int from, int to)
{
  for (int i=0;i<_marks.size();i++) {
    if (_marks[i]->getNumSamples()) {
      if (_marks[i]->getInitialBrushState()->frameIndex == from) {
        MarkRef myCopy = _marks[i]->copy();
        for (int i=0;i<myCopy->getNumSamples();i++) {
          myCopy->getBrushState(i)->frameIndex = to;
        }
        addMark(myCopy);
        myCopy->commitGeometry(this);
      }
    }
  }
}


/** Each time you change the format of this, increase the
    SERIALIZE_VERSION_NUMBER and respond accordingly in the
    deserialize method.  In this method, you can safely get rid of the
    old code because we don't want to create files in an old format.
*/
#define SERIALIZE_VERSION_NUMBER 0
void
Artwork::serialize(G3D::BinaryOutput &b)
{
  b.writeInt8(SERIALIZE_VERSION_NUMBER);

  // marks
  b.writeInt32(_marks.size());
  for (int i=0;i<_marks.size();i++) {
    b.writeString(_marks[i]->markDescription());
    _marks[i]->serialize(b);
  }

  // lighting
  LightingIO::serializeLighting(_gfxMgr->getLighting(), _gfxMgr, b);

  // room2virtual transform
  _gfxMgr->getRoomToVirtualSpaceFrame().serialize(b);
  b.writeFloat64(_gfxMgr->getRoomToVirtualSpaceScale());
}


/** Never delete the code from an old version from this method so we
    can always stay backwards compatable.  Important, since this is in
    binary, and we can't really reverse engineer the file format.
*/
void
Artwork::deserialize(G3D::BinaryInput &b)
{
  int version = b.readInt8();

  if (version == 0) {

    // marks
    int n = b.readInt32();
    for (int i=0;i<n;i++) {
      std::string desc = b.readString();
      if (desc == "RibbonMark") {
        MarkRef newMark = new RibbonMark("Unnamed", _gfxMgr, _triStripModel);
        newMark->deserialize(b);
        addMark(newMark);
        newMark->commitGeometry(this);
        if ((newMark->getInitialBrushState().notNull()) && 
            (newMark->getInitialBrushState()->layerIndex > _maxLayerID)) {
          _maxLayerID = newMark->getInitialBrushState()->layerIndex;
        }
        if ((newMark->getInitialBrushState().notNull()) && 
            (newMark->getInitialBrushState()->frameIndex >= _numFrames)) {
          _numFrames = newMark->getInitialBrushState()->frameIndex+1;
        }
      }
      else if (desc == "TubeMark") {
        MarkRef newMark = new TubeMark("Unnamed", _gfxMgr, _triStripModel);
        newMark->deserialize(b);
        addMark(newMark);
        newMark->commitGeometry(this);
        if ((newMark->getInitialBrushState().notNull()) && 
            (newMark->getInitialBrushState()->layerIndex > _maxLayerID)) {
          _maxLayerID = newMark->getInitialBrushState()->layerIndex;
        }
        if ((newMark->getInitialBrushState().notNull()) && 
            (newMark->getInitialBrushState()->frameIndex >= _numFrames)) {
          _numFrames = newMark->getInitialBrushState()->frameIndex+1;
        }
      }
      else if (desc == "FlatTubeMark") {
        MarkRef newMark = new FlatTubeMark("Unnamed", _gfxMgr, _triStripModel);
        newMark->deserialize(b);
        addMark(newMark);
        newMark->commitGeometry(this);
        if ((newMark->getInitialBrushState().notNull()) && 
            (newMark->getInitialBrushState()->layerIndex > _maxLayerID)) {
          _maxLayerID = newMark->getInitialBrushState()->layerIndex;
        }
        if ((newMark->getInitialBrushState().notNull()) && 
            (newMark->getInitialBrushState()->frameIndex >= _numFrames)) {
          _numFrames = newMark->getInitialBrushState()->frameIndex+1;
        }
      }
      else if (desc == "AnnotationMark") {
        MarkRef newMark = new AnnotationMark("Unnamed", _gfxMgr, _annotationModel);
        newMark->deserialize(b);
        addMark(newMark);
        newMark->commitGeometry(this);
        if ((newMark->getInitialBrushState().notNull()) && 
            (newMark->getInitialBrushState()->layerIndex > _maxLayerID)) {
          _maxLayerID = newMark->getInitialBrushState()->layerIndex;
        }
        if ((newMark->getInitialBrushState().notNull()) && 
            (newMark->getInitialBrushState()->frameIndex >= _numFrames)) {
          _numFrames = newMark->getInitialBrushState()->frameIndex+1;
        }
      }
      else if (desc == "SlideMark") {
        MarkRef newMark = new SlideMark("Unnamed", _gfxMgr);
        newMark->deserialize(b);
        addMark(newMark);
        newMark->commitGeometry(this);
        if ((newMark->getInitialBrushState().notNull()) && 
            (newMark->getInitialBrushState()->layerIndex > _maxLayerID)) {
          _maxLayerID = newMark->getInitialBrushState()->layerIndex;
        }
        if ((newMark->getInitialBrushState().notNull()) && 
            (newMark->getInitialBrushState()->frameIndex >= _numFrames)) {
          _numFrames = newMark->getInitialBrushState()->frameIndex+1;
        }
      }
      else {
        alwaysAssertM(false, "Unrecognized Mark type " + desc);
      }
      LoadingScreen::renderAndSwapBuffers(_gfxMgr, 100.0*(float)i/(float)(n-1));
    }
    
    // lighting
    LightingRef lighting = _gfxMgr->getLighting();
    LightingIO::deserializeLighting(lighting, _gfxMgr, b);
    _gfxMgr->setLighting(lighting);

    // room2virtual transform
    CoordinateFrame frame;
    frame.deserialize(b);
    _gfxMgr->setRoomToVirtualSpaceFrame(frame);
    double scale = b.readFloat64();
    _gfxMgr->setRoomToVirtualSpaceScale(scale);
 
  }
  else {
    alwaysAssertM(false, "Unrecognized Artwork version " + MinVR::intToString(version));
  }
}



} // end namespace

