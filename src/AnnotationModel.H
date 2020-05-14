/**
 * /author Dan Keefe (dfk)
 *
 * /file  AnnotationModel.H
 * /brief 
 *
 */ 

#ifndef ANNOTATIONMODEL_H
#define ANNOTATIONMODEL_H

//#include <VRG3DBaseApp.H>
#include <G3D/G3D.h>
#include <GLG3D/GLG3D.h>
#include <GfxMgr.H>

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class AnnotationModel> AnnotationModelRef;
/**
*/
class AnnotationModel : public G3D::ReferenceCountedObject
{
public:
  AnnotationModel(MinVR::GfxMgrRef gfxMgr);
  virtual ~AnnotationModel();

  /// Returns annotation ID number
  int  addAnnotation(const std::string &text, G3D::Vector3 startPt, G3D::Vector3 endPt, G3D::Color3 color, int layerIndex, int frameIndex);
  void editAnnotation(int id, const std::string &text, G3D::Vector3 startPt, G3D::Vector3 endPt, G3D::Color3 color);
  void deleteAnnotation(int id);


  void setMovieFrame(int frameNum);

  virtual G3D::Array<int> getHiddenLayers() { return _hiddenLayers; }
  virtual void setHiddenLayers(const G3D::Array<int> &layers) { _hiddenLayers = layers; }

  void pose(G3D::Array<G3D::PosedModel::Ref> &posedModels, const G3D::CoordinateFrame &virtualToRoomSpace);
  void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);

  void serialize(class G3D::BinaryOutput& b);
  void deserialize(class G3D::BinaryInput& b);

private:

  AnnotationModel(const AnnotationModel &);
  AnnotationModel& operator=(const AnnotationModel &);

  MinVR::GfxMgrRef                 _gfxMgr;
  int                       _nextID;
  double                    _screenSize;
  double                    _constSize;
  G3D::Table<int, G3D::Vector3>       _startPt;
  G3D::Table<int, G3D::Vector3>       _endPt;
  G3D::Table<int, G3D::Color3>        _color;
  G3D::Table<int, std::string>   _text;
  G3D::Table<int, G3D::GFont::YAlign> _textYAlign;
  G3D::Table<int, double>        _textScale;
  G3D::Table<int, int>           _layers;
  G3D::Table<int, int>           _frames;

  int                       _frame;
  G3D::Array<int>                _hiddenLayers;
};


} // end namespace

#endif

