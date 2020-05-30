
#ifndef POINTMODELSW_H
#define POINTMODELSW_H

//#include "VRBase.H"

#include <GfxMgr.H>
#include "PointModel.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class PointModelSW> PointModelSWRef;
/**
*/
class PointModelSW: public PointModel 
{
public:
  PointModelSW(GfxMgrRef gfxMgr);
  virtual ~PointModelSW();

  virtual int  addModel(const G3D::Array<G3D::Vector3> &points, const G3D::Array<double> &weights, 
      const double &colorSwatchIndex, const double &lineWidth, const int &brushTip);

  virtual void updateModel(int modelID, const G3D::Array<G3D::Vector3> &points, const G3D::Array<double> &weights, 
      const double &colorSwatchIndex, const double &lineWidth, const int &brushTip);
  
  virtual void deleteModel(int modelID);

  virtual G3D::PosedModel::Ref pose(const G3D::CoordinateFrame& cframe);  
  virtual void  render(G3D::RenderDevice* renderDevice) const;

private:
  PointModelSW(const PointModelSW &);
  PointModelSW& operator=(const PointModelSW &);

  void updateVARArea();

  void setupRendering(G3D::RenderDevice* renderDevice) const;
  void finishRendering(G3D::RenderDevice* renderDevice) const;

  void quickSortPointsByDepth(int left, int right);

  GfxMgrRef        _gfxMgr;
  int              _numPointsThatFitInVarArea;
  G3D::VARArea::Ref       _varArea;
  G3D::Array<G3D::Vector3>   _pointsArray;      // each 4 points in a row are the same
  G3D::Array<G3D::Vector3>   _posedPointsArray; // points after they have offsets applied to form the 4 corners of the billboard
  G3D::VAR              _pointsVAR;        // really for posedPoints
  G3D::Array<G3D::Vector2>   _texCoord0Array;  
  G3D::VAR              _texCoord0VAR;
  G3D::Array<G3D::Vector2>   _texCoord1Array;  
  G3D::VAR              _texCoord1VAR;
  G3D::Array<G3D::Vector2>   _texCoord2Array;   // not actually used as a texture coordinate in the software only implementation
  G3D::VAR              _texCoord2VAR;

  PointBSPTreeRef  _bspTree;
  int              _lastIndexInBSPTree;
  G3D::Array<double>    _depthArray;       // used for depth sorting points
  G3D::Array<int>       _sortedIndices;    // one index for each set of 4 points in _pointsArray, sorted
                                      // each frame by depth for proper rendering of transparency
  G3D::Array<int>       _allSortedIndices; // list of all the sorted indices, including all 4 per point
                                      // used for rendering the billboard.
  bool             _useVertexProg;
  G3D::ShaderRef        _vShader;
  G3D::Texture::Ref       _brushTipTex;

  int              _nextModelID;
  G3D::Table<int, G3D::Array<int> > _indicesByModelID;  

};


} // end namespace

#endif
