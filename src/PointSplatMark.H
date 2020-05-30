/**
 * /author Dan Keefe (dfk)
 *
 * /file  PointSplatMark.H
 * /brief 
 *
 */ 


#ifndef POINTSPLATMARK_H
#define POINTSPLATMARK_H

//#include "VRBase.H"
#include <GfxMgr.H>
#include "Mark.H"
#include "PointModel.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class PointSplatMark> PointSplatMarkRef;
/**
*/
class PointSplatMark : public Mark
{
public:
  PointSplatMark(const std::string &name, 
                 GfxMgrRef gfxMgr,
                 PointModelRef pointModel);

  virtual ~PointSplatMark();

  virtual std::string markDescription() { return "PointSplatMark"; }

  virtual MarkRef copy();

  virtual void trimEnd(int newEndPt);

  virtual void addMarkSpecificSample(BrushStateRef brushState);

  virtual void pose(G3D::Array<G3D::PosedModel::Ref>& posedModels, const G3D::CoordinateFrame& frame);
  virtual void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame& frame);

  virtual void setHighlighted(bool highlighted);
  virtual void show();
  virtual void hide();

  virtual PointModelRef getPointModel() { return _pointModel; }

  G3D::Array<double> _sampleWeights;

private:
  PointSplatMark(const PointSplatMark &);
  PointSplatMark& operator=(const PointSplatMark &);
  
  PointModelRef _pointModel;
  int           _modelID;
  bool          _highlighted;
};


} // end namespace

#endif

