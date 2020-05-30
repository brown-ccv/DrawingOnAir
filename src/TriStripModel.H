
#ifndef TRISTRIPMODEL_H
#define TRISTRIPMODEL_H

#include <VRG3DBaseApp.H>

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class TriStripModel> TriStripModelRef;
/**
*/
class TriStripModel : public G3D::PosedModel 
{
public:
  TriStripModel(GfxMgrRef gfxMgr);
  virtual ~TriStripModel();

  /// Returns a sub-model ID number
  virtual int  addModel(const G3D::Array<G3D::Vector3> &vertices, 
                        const G3D::Array<G3D::Vector3> &normals,
                        const G3D::Array<G3D::Vector2> &texCoord0,
                        const G3D::Array<G3D::Vector2> &texCoord1,
                        const G3D::Array<G3D::Vector2> &texCoord2,
                        bool twoSided,
                        int layer,
                        int movieFrame);

  virtual void updateModel(int modelID,
                           const G3D::Array<G3D::Vector3> &vertices, 
                           const G3D::Array<G3D::Vector3> &normals,
                           const G3D::Array<G3D::Vector2> &texCoord0,
                           const G3D::Array<G3D::Vector2> &texCoord1,
                           const G3D::Array<G3D::Vector2> &texCoord2);

  /// Use this to hide the model for a short time. The idea is that this hides 
  /// the model without deleting it, so it's quick to turn its display on/off.
  virtual void hideModel(int modelID);
  /// Show a model again after hiding it.
  virtual void showModel(int modelID);
  
  virtual void deleteModel(int modelID);

  virtual G3D::Array<int> getHiddenLayers() { return _hiddenLayers; }
  virtual void setHiddenLayers(const G3D::Array<int> &layers) { _hiddenLayers = layers; }

  virtual void setMovieFrame(int newFrame);

  virtual G3D::PosedModel::Ref pose(const G3D::CoordinateFrame& cframe);
  
  // Implementations of pure virtual functions from PosedModel
  virtual std::string name() const;
  virtual bool  hasTransparency() const { return true; }
  virtual void  getCoordinateFrame(G3D::CoordinateFrame&) const;
  virtual const G3D::MeshAlg::Geometry& objectSpaceGeometry() const;
  virtual const G3D::Array<G3D::MeshAlg::Face>& faces() const;
  virtual const G3D::Array<G3D::MeshAlg::Edge>& edges() const;
  virtual const G3D::Array<G3D::MeshAlg::Vertex>& vertices() const;
  virtual const G3D::Array<G3D::MeshAlg::Face>& weldedFaces() const;
  virtual const G3D::Array<G3D::MeshAlg::Edge>& weldedEdges() const;
  virtual const G3D::Array<G3D::MeshAlg::Vertex>& weldedVertices() const;
  virtual const G3D::Array<G3D::Vector2>& texCoords() const;
  virtual bool  hasTexCoords() const ;
  virtual const G3D::Array<int>& triangleIndices() const;
  virtual void  getObjectSpaceBoundingSphere(G3D::Sphere&) const;
  virtual void  getObjectSpaceBoundingBox(G3D::AABox&) const;
  virtual void  render(G3D::RenderDevice* renderDevice) const;
  virtual int   numBoundaryEdges() const;
  virtual int   numWeldedBoundaryEdges() const;
  virtual const G3D::Array<G3D::Vector3>& objectSpaceFaceNormals(bool normalize = true) const;

protected:

  void updateVARArea();

  G3D::CoordinateFrame  _frame;

  GfxMgrRef        _gfxMgr;
  int              _numPointsThatFitInVarArea;

  G3D::Table<int, G3D::Array< int > > _modelIndices;
  G3D::Table<int, bool>          _twoSided;
  G3D::Table<int, int>           _layers;
  G3D::Table<int, int>           _movieFrames;
  G3D::Array<int>                _hiddenLayers;
  G3D::Array<int>                _hiddenModels;
  int                       _nextModelID;
  int                       _movieFrame;

  G3D::VARArea::Ref       _varArea;
  G3D::Array<G3D::Vector3>   _vertsArray;
  G3D::VAR              _vertsVAR;
  G3D::Array<G3D::Vector3>   _normalsArray;
  G3D::VAR              _normalsVAR;
  G3D::Array<G3D::Vector2>   _texCoord0Array;
  G3D::VAR              _texCoord0VAR;
  G3D::Array<G3D::Vector2>   _texCoord1Array;  
  G3D::VAR              _texCoord1VAR;
  G3D::Array<G3D::Vector2>   _texCoord2Array;  
  G3D::VAR              _texCoord2VAR;

private:
  TriStripModel(const TriStripModel &);
  TriStripModel& operator=(const TriStripModel &);
};


} // end namespace

#endif
