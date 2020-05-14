/**
 * /author Dan Keefe (dfk)
 *
 * /file  RibbonMark.H
 * /brief 
 *
 */ 


#ifndef RIBBONMARK_H
#define RIBBONMARK_H

//#include "VRBase.H"
#include <GfxMgr.H>

#include "Mark.H"
#include "TriStripModel.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class RibbonMark> RibbonMarkRef;
/**
*/
class RibbonMark : public Mark
{
public:
  RibbonMark(const std::string &name,
             MinVR::GfxMgrRef gfxMgr,
             TriStripModelRef triStripModel);

  virtual ~RibbonMark();

  MarkRef copy();

  virtual void trimEnd(int newEndPt);

  virtual void addMarkSpecificSample(BrushStateRef brushState);
  
  virtual std::string markDescription() { return "RibbonMark"; }

  virtual void pose(G3D::Array<G3D::PosedModel::Ref>& posedModels, const G3D::CoordinateFrame& frame);
  virtual void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame& frame);

  virtual void setHighlighted(bool highlighted, ArtworkRef artwork);
  virtual void show();
  virtual void hide();
  virtual void stopDrawing();

  virtual void commitGeometry(ArtworkRef artwork);
  
  virtual void startEditing(ArtworkRef artwork);
  virtual void stopEditing(ArtworkRef artwork);

  virtual void transformBy(G3D::CoordinateFrame frame);

  void updateTriStripModel();

  G3D::Vector3                _lastGoodUpVec;
  G3D::Vector3                _lastGoodRightVec;
  G3D::Vector3                _lastRight;

  bool                   _highlighted;
  bool                   _hidden;

  G3D::Texture::Ref             _brushTipTex;
  G3D::Texture::Ref             _patternTex;
  
  G3D::Array<G3D::Vector3>         _verts;
  G3D::Array<G3D::Vector3>         _norms;
  G3D::Array<G3D::Vector2>         _maskTexCoords;
  G3D::Array<G3D::Vector2>         _colTexCoords;
  G3D::Array<G3D::Vector2>         _highTexCoords;
  G3D::Array<G3D::Vector2>         _patTexCoords;

  TriStripModelRef       _triStripModel;
  int                    _modelID;

private:
  RibbonMark(const RibbonMark &);
  RibbonMark& operator=(const RibbonMark &);
};


} // end namespace

#endif

