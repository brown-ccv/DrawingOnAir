/**
 * /author Dan Keefe (dfk)
 *
 * /file  Mark.H
 * /brief Virtual base class for CavePainting Marks
 *
 */ 


#ifndef MARK_H
#define MARK_H

#include <VRG3DBaseApp.H>
#include "BrushState.H"

namespace DrawOnAir {


//typedef ReferenceCountedPointer<class BrushState> BrushStateRef;
typedef G3D::ReferenceCountedPointer<class Artwork> ArtworkRef;

typedef G3D::ReferenceCountedPointer<class Mark> MarkRef;
/** Virtual base class for CavePainting Marks.  Supersampling: If
  brush->state->superSampling is not 0 then each time addSample() is
  called multiple samples will be created to satisfy that there are n
  samples along the length of the mark, where n is specified relative
  to the width of the Mark.
*/
class Mark : public G3D::ReferenceCountedObject
{
public:
  Mark(const std::string &name, bool shouldBeDrawn, bool shouldBePosed, MinVR::GfxMgrRef gfxMgr);
  virtual ~Mark() {}

  virtual MarkRef copy() = 0;

  /// This description is used when saving marks to a file.  Should not have any spaces in it.
  /// Should be brief, typically the same as the name of the Mark subclass.
  virtual std::string markDescription() = 0;

  /// See note in the constructor documentation about supersampling.  If supersampling is
  /// off this creates just 1 new sample, if supersampling is on, this may create multiple
  /// new samples.  samplePosition is used to update the bounding box for the mark, usually
  /// it should be set to brush->currentFrameInRoomSpace.translation
  virtual void addSample(BrushStateRef brushState);

  virtual void addMarkSpecificSample(BrushStateRef brushState) {}

  virtual void trimEnd(int newEndPt);

  /// Called after marks are done being drawn, useful for sending info to the gfx card
  virtual void commitGeometry(ArtworkRef artwork) {}

  /// Called before editing operations, most marks will put their
  /// geometry into vertex arrays for fast rendering once the initial
  /// drawing is complete, this takes it out of the vertex array and
  /// prepares for fast updates during editing.  Call stopEditing when
  /// done to go back to fast rendering.
  virtual void startEditing(ArtworkRef artwork) {}
  virtual void stopEditing(ArtworkRef artwork) {}

  virtual int            getNumSamples()              { return _samplePositions.size(); }
  virtual G3D::Vector3        getSamplePosition(int index) { return _samplePositions[index]; }
  virtual BrushStateRef  getBrushState(int index)     { return _brushStates[index]; }

  virtual G3D::Array<G3D::Vector3> getSamplePositionsArray()      { return _samplePositions; }
  virtual G3D::Array<bool>    getSampleIsASuperSampleArray() { return _sampleIsASuperSample; }
  virtual G3D::Array<BrushStateRef> getBrushStatesArray() { return _brushStates; }
  virtual G3D::Array<BrushStateRef> getBrushStatesSubArray(int start, int stop);

  virtual void transformBy(G3D::CoordinateFrame frame);
  virtual void saveTransformEvent(G3D::CoordinateFrame frame);


  virtual void setHighlighted(bool highlighted, ArtworkRef artwork) {}
  virtual void show() {}
  virtual void hide() {}
  virtual void stopDrawing() {}

  // Pose and/or Draw methods are usually overridden in subclasses
  virtual void pose(G3D::Array<G3D::PosedModel::Ref>& posedModels, const G3D::CoordinateFrame& frame) {
    _poseDirty = false;
  }

  virtual void setPoseDirty()                   { _poseDirty = true; }

  virtual void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame& frame) {}

  virtual void setName(const std::string &name) { _name = name; }
  virtual std::string getName()                 { return _name; }

  /// Returns true if the point lies within the mark.  By within we mean both within the 
  /// bounding box of the mark AND that the distance from the point to at least one sample
  /// on the Mark is less than the width of the mark multiplied by widthScaleFactor.
  virtual bool contains(const G3D::Vector3 &point, double widthScaleFactor=0.5);
  virtual bool approxContains(const G3D::Vector3 &point, double widthScaleFactor=0.5);
  virtual bool approxContains(const G3D::Vector3 &point, double widthScaleFactor, int &indexOfClosestSample);
  virtual bool boundingBoxContains(const G3D::Vector3 &point) { return _bbox.contains(point); }

  virtual G3D::AABox getBoundingBox() { return _bbox; }
  virtual void  setBoundingBox(G3D::AABox b) { _bbox = b; }

  virtual double distanceToMark(const G3D::Vector3 &point);
  virtual double distanceToMark(const G3D::Vector3 &point, int &indexOfClosestSample);
  virtual double approxDistanceToMark(const G3D::Vector3 &point);
  virtual double approxDistanceToMark(const G3D::Vector3 &point, int &indexOfClosestSample);


  double getArcLength();
  double getArcLength(int fromSample, int toSample);

  virtual G3D::CoordinateFrame getRoomToVirtualFrameAtStart() { return _roomToVirtualFrameAtStart; }
  virtual void            setRoomToVirtualFrameAtStart(G3D::CoordinateFrame f) { _roomToVirtualFrameAtStart = f; }

  virtual double getRoomToVirtualScaleAtStart() { return _roomToVirtualScaleAtStart; }
  virtual void   setRoomToVirtualScaleAtStart(double s) { _roomToVirtualScaleAtStart = s; }

  virtual bool getShouldBeDrawn() { return _shouldBeDrawn; }
  virtual void setShouldBeDrawn(bool b);

  virtual bool getShouldBePosed() { return _shouldBePosed; }
  virtual void setShouldBePosed(bool b);

  BrushStateRef getInitialBrushState();

  void serialize(class G3D::BinaryOutput& b);
  void deserialize(class G3D::BinaryInput& b);

protected:

  MinVR::GfxMgrRef              _gfxMgr;
  std::string            _name;
  bool                   _shouldBeDrawn;
  bool                   _shouldBePosed;
  bool                   _poseDirty;
  G3D::AABox                  _bbox;          // bounding box

  // stores all samples
  G3D::Array<BrushStateRef>   _brushStates;
  G3D::Array<G3D::Vector3>         _samplePositions;
  G3D::Array<bool>            _sampleIsASuperSample;

  // length from beginning of the path to each sample is cached for speed
  // in arclength calculations
  G3D::Array<double>          _arcLengths;

  // Environment state at the time the Mark was created
  G3D::CoordinateFrame        _roomToVirtualFrameAtStart;
  double                 _roomToVirtualScaleAtStart;

  // Stores the history of moving the mark around so it can be
  // recreated when the mark is read in from a file
  G3D::Array<G3D::CoordinateFrame> _transformHistVSFrame;
  G3D::Array<double>          _transformHistVSScale;
  G3D::Array<G3D::CoordinateFrame> _transformHistXform;

private:
  Mark(const Mark &);
  Mark& operator=(const Mark &);
};



} // end namespace

#endif

