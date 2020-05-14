
#ifndef POINTMODELSPRITES_H
#define POINTMODELSPRITES_H

//#include "VRBase.H"
#include <GfxMgr.H>

#include "PointModel.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class PointModelSprites> PointModelSpritesRef;
/**
*/
class PointModelSprites: public PointModel 
{
public:
  PointModelSprites(MinVR::GfxMgrRef gfxMgr);
  virtual ~PointModelSprites();

  virtual int  addModel(const G3D::Array<G3D::Vector3> &points, const G3D::Array<double> &weights, 
      const double &colorSwatchIndex, const double &lineWidth, const int &brushTip);

  virtual void updateModel(int modelID, const G3D::Array<G3D::Vector3> &points, const G3D::Array<double> &weights, 
      const double &colorSwatchIndex, const double &lineWidth, const int &brushTip);
  
  virtual void deleteModel(int modelID);

  virtual G3D::PosedModel::Ref pose(const G3D::CoordinateFrame& cframe);  
  virtual void  render(G3D::RenderDevice* renderDevice) const;

private:
  PointModelSprites(const PointModelSprites &);
  PointModelSprites& operator=(const PointModelSprites &);

  void updateVARArea();

  void setupRendering(G3D::RenderDevice* renderDevice) const;
  void finishRendering(G3D::RenderDevice* renderDevice) const;
  
  void quickSortPointsByDepth(int left, int right);

  MinVR::GfxMgrRef        _gfxMgr;
  int              _numPointsThatFitInVarArea;
  G3D::VARArea::Ref       _varArea;
  G3D::Array<G3D::Vector3>   _pointsArray;
  G3D::VAR              _pointsVAR;
  G3D::Array<G3D::Vector3>   _texCoord1Array;  
  G3D::VAR              _texCoord1VAR;
  G3D::Array<G3D::Vector2>   _texCoord2Array;
  G3D::VAR              _texCoord2VAR;
  G3D::Array<double>    _depthArray;       // used for depth sorting points

  PointBSPTreeRef  _bspTree;
  int              _lastIndexInBSPTree;
  G3D::Array<int>       _sortedIndices;
  G3D::ShaderRef        _shader;
  G3D::Texture::Ref       _brushTipTex;

  int                     _nextModelID;
  G3D::Table<int, G3D::Array<int> > _indicesByModelID;  

};


} // end namespace

#endif
