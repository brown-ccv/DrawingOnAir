/**
 * /author Dan Keefe (dfk)
 *
 * /file  Artwork.H
 * /brief Draws CavePainting Artwork
 *
 */ 

#ifndef ARTWORK_H
#define ARTWORK_H

//#include <VRG3DBaseApp.H>
#include <GfxMgr.H>

#include "AnnotationModel.H"
#include "Mark.H"
#include "PointModel.H"
#include "TriStripModel.H"

namespace DrawOnAir {


typedef G3D::ReferenceCountedPointer<class Artwork> ArtworkRef;
/**
*/
class Artwork : public G3D::ReferenceCountedObject
{ 
public: 
  Artwork(MinVR::GfxMgrRef gfxMgr);
  virtual ~Artwork();

  void addMark(MarkRef mark);
  void removeMark(MarkRef mark);
  void removeFromMarksToDraw(MarkRef mark);
  void addToMarksToDraw(MarkRef mark);
  void removeFromMarksToPose(MarkRef mark);
  void addToMarksToPose(MarkRef mark);

  void transform(G3D::CoordinateFrame frame, int layerIndex);
  
  void pose(G3D::Array<G3D::PosedModel::Ref> &posedModels, const G3D::CoordinateFrame &virtualToRoomSpace);
  void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);

  AnnotationModelRef getAnnotationModel() { return _annotationModel; }
  PointModelRef      getPointModel()      { return _pointModel; }
  TriStripModelRef   getTriStripModel()   { return _triStripModel; }

  G3D::Array<MarkRef>& getMarks() { return _marks; }

  /// Layers: like photoshop layers except they don't lie on top of
  /// each other in 2D.  Any layer can be hidden or visible at a given
  /// time.
  int  getMaxLayerID() { return _maxLayerID; }
  void setMaxLayerID(int max) { _maxLayerID = max; }
  G3D::Array<int> getHiddenLayers() { return _hiddenLayers; }
  void setHiddenLayers(const G3D::Array<int> &layers);
  void showAllLayers();
  void hideAllButThisLayer(int layerID);

  /// Frames: these are for animated drawings.  Each frame has the
  /// same set of layers.  That is, the layers exist across frames.
  /// While many layers may be visible at a time, only 1 frame is
  /// visible at any given time.
  void showFrame(int frame);
  void nextFrame();
  void previousFrame();

  int getNumFrames() { return _numFrames; }
  int getCurrentFrame() { return _frame; }
  void setNumFrames(int numFrames);

  /// Copies all marks in the frame "from" to the frame "to"
  void copyFrameToFrame(int from, int to);

  /// Remove all marks
  void clear();

  void serialize(class G3D::BinaryOutput& b);
  void deserialize(class G3D::BinaryInput& b);

private:

  Artwork(const Artwork &);
  Artwork& operator=(const Artwork &);

  MinVR::GfxMgrRef          _gfxMgr;
  AnnotationModelRef _annotationModel;
  PointModelRef      _pointModel;
  TriStripModelRef   _triStripModel;
  int                _cbidPose, _cbidDraw;

  /// All marks in the Artwork
  G3D::Array<MarkRef>    _marks;
  /// Subset of marks for which draw() should be called each frame
  G3D::Array<MarkRef>    _marksToDraw;
  /// Subset of marks for which pose() should be called each frame
  G3D::Array<MarkRef>    _marksToPose;

  /// Stores the largest layer ID found within the marks in the
  /// artwork.  The layer id of the active drawing layer is stored in
  /// the brush state.
  int _maxLayerID;  

  /// By default, all layers are visible, this is a list of layers
  /// that are hidden.
  G3D::Array<int> _hiddenLayers;

  /// This is the current frame that is displayed.  Unlike layers,
  /// only 1 frame is shown at a time.  In interactive sessions, the
  /// frameIndex stored in the brush state should also equal this
  /// value.
  int _frame;
  int _numFrames;
};

} // end namespace

#endif



