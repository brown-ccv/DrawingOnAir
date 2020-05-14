
#ifndef POINTMODEL_H
#define POINTMODEL_H

//#include "VRBase.H"
#include <G3D/G3D.h>
#include <GLG3D/GLG3D.h>

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class PointBSPTree> PointBSPTreeRef;
class PointBSPTree : public G3D::ReferenceCountedObject
{
public:
  PointBSPTree(const G3D::Vector3 &point, const int &index) {
    points.append(point);
    indices.append(index);

    static int planeType = 0;
    planeType++;
    int type = planeType % 3;
    if (type == 0) {
      plane = G3D::Plane(G3D::Vector3(1,0,0), point);
    }
    else if (type == 1) {
      plane = G3D::Plane(G3D::Vector3(0,1,0), point);
    }
    else if (type == 2) {
      plane = G3D::Plane(G3D::Vector3(0,0,1), point);
    }
  }

  virtual ~PointBSPTree() {}

  // points and indices contained in this plane
  G3D::Array<G3D::Vector3>          points;
  G3D::Array<int>              indices;
  // this plane
  G3D::Plane                   plane;

  // everything in front of this plane
  PointBSPTreeRef  front;
  // everything behind this plane
  PointBSPTreeRef  back;

  void insert(const G3D::Vector3 &p, const int &index) {
    if (plane.fuzzyContains(p)) {
      points.append(p);
      indices.append(index);
    }
    else if (plane.halfSpaceContains(p)) {
      if (front.isNull()) {
        front = new PointBSPTree(p, index);
      }
      else {
        front->insert(p,index);
      }
    }
    else {
      if (back.isNull()) {
        back = new PointBSPTree(p, index);
      }
      else {
        back->insert(p,index);
      }
    }
  }

  void getSortedListOfIndices(const G3D::Vector3 &eye, G3D::Array<int> &sortedIndices) {
    if (plane.fuzzyContains(eye)) { // eye conincident
      if (front.notNull()) {
        front->getSortedListOfIndices(eye, sortedIndices);
      }
      sortedIndices.append(indices);
      if (back.notNull()) {
        back->getSortedListOfIndices(eye, sortedIndices);
      }
    }
    else if (plane.halfSpaceContains(eye)) { // eye in front
      if (back.notNull()) {
        back->getSortedListOfIndices(eye, sortedIndices);
      }
      sortedIndices.append(indices);
      if (front.notNull()) {
        front->getSortedListOfIndices(eye, sortedIndices);
      }
    }
    else { // eye behind
      if (front.notNull()) {
        front->getSortedListOfIndices(eye, sortedIndices);
      }
      sortedIndices.append(indices);
      if (back.notNull()) {
        back->getSortedListOfIndices(eye, sortedIndices);
      }
    }
  }

private:
  PointBSPTree(const PointBSPTree &);
  PointBSPTree& operator=(const PointBSPTree &);
};




typedef G3D::ReferenceCountedPointer<class PointModel> PointModelRef;
/**
*/
class PointModel : public G3D::PosedModel 
{
public:
  PointModel();
  virtual ~PointModel();

  /// Returns a modelID number
  virtual int  addModel(const G3D::Array<G3D::Vector3> &points, const G3D::Array<double> &weights, 
      const double &colorSwatchIndex, const double &lineWidth, const int &brushTip) = 0;

  virtual void updateModel(int modelID, const G3D::Array<G3D::Vector3> &points, const G3D::Array<double> &weights, 
      const double &colorSwatchIndex, const double &lineWidth, const int &brushTip) = 0;
  
  virtual void deleteModel(int modelID) = 0;

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
  virtual void  getObjectSpaceBoundingBox(G3D::Box&) const;
  virtual void  render(G3D::RenderDevice* renderDevice) const;
  virtual int   numBoundaryEdges() const;
  virtual int   numWeldedBoundaryEdges() const;
  virtual const G3D::Array<G3D::Vector3>& objectSpaceFaceNormals(bool normalize = true) const;

protected:
  G3D::CoordinateFrame _frame;

private:
  PointModel(const PointModel &);
  PointModel& operator=(const PointModel &);
};


} // end namespace

#endif
