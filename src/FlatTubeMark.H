/**
 * /author Dan Keefe (dfk)
 *
 * /file  FlatTubeMark.H
 * /brief 
 *
 */ 


#ifndef FLATTUBEMARK_H
#define FLATTUBEMARK_H

//#include "VRBase.H"

#include <GfxMgr.H>
#include "Mark.H"
#include "TriStripModel.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class FlatTubeMark> FlatTubeMarkRef;
/**
*/
class FlatTubeMark : public Mark
{
public:
  FlatTubeMark(const std::string &name, 
               GfxMgrRef gfxMgr,
               TriStripModelRef triStripModel);

  virtual ~FlatTubeMark();

  MarkRef copy();

  virtual std::string markDescription() { return "FlatTubeMark"; }

  virtual void addMarkSpecificSample(BrushStateRef brushState);
  
  virtual void trimEnd(int newEndPt);

  virtual void setHighlighted(bool highlighted, ArtworkRef artwork);
  virtual void show();
  virtual void hide();
  virtual void stopDrawing();

  virtual void commitGeometry(ArtworkRef artwork);

  virtual void startEditing(ArtworkRef artwork);
  virtual void stopEditing(ArtworkRef artwork);

  virtual void transformBy(G3D::CoordinateFrame frame);

  virtual void pose(G3D::Array<G3D::PosedModel::Ref>& posedModels, const G3D::CoordinateFrame& frame);
  virtual void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame& frame);

  void updateTriStripModel();

private:
  FlatTubeMark(const FlatTubeMark &);
  FlatTubeMark& operator=(const FlatTubeMark &);

  bool                   _highlighted;
  bool                   _hidden;
  int                    _nfaces;
  G3D::Texture::Ref             _brushTipTex;
  G3D::Texture::Ref             _patternTex;
  G3D::Vector3                _lastRight;
  G3D::Vector3                _lastGoodUpVec;
  G3D::Vector3                _lastGoodRightVec;

  TriStripModelRef         _triStripModel;
  G3D::Array<int>               _modelID; // one model per face of the tube
  G3D::Array< G3D::Array<G3D::Vector3> >  _verts;
  G3D::Array< G3D::Array<G3D::Vector3> >  _norms;
  G3D::Array< G3D::Array<G3D::Vector2> >  _maskTexCoords;
  G3D::Array< G3D::Array<G3D::Vector2> >  _patTexCoords;
  G3D::Array< G3D::Array<G3D::Vector2> >  _colTexCoords;
  G3D::Array< G3D::Array<G3D::Vector2> >  _highTexCoords;
};


} // end namespace

#endif

