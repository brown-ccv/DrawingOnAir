#include "PointModel.H"
using namespace G3D;
namespace DrawOnAir {


PointModel::PointModel()
{
}  

PointModel::~PointModel()
{
}


PosedModel::Ref 
PointModel::pose(const CoordinateFrame& cframe) 
{
  return NULL;
}

void 
PointModel::render(RenderDevice* renderDevice) const 
{ 
}

std::string 
PointModel::name() const 
{
  return "PointModel";
}

void 
PointModel::getCoordinateFrame(CoordinateFrame& c) const 
{
  c = _frame;
}


/****   These come from PosedModel, but don't make so much sense for a PointModel   ****
 --------------------------------------------------------------------------------------*/

const MeshAlg::Geometry& 
PointModel::objectSpaceGeometry() const 
{
  alwaysAssertM(false, "PointModel doesn't have geometry.");
  static MeshAlg::Geometry g;
  return g;
}

const Array<int>& 
PointModel::triangleIndices() const 
{
  alwaysAssertM(false, "PointModel doesn't have geometry.");
  static Array<int> a;
  return a;
}


const Array<MeshAlg::Face>& 
PointModel::faces() const 
{
  alwaysAssertM(false, "PointModel doesn't have geometry.");
  static Array<MeshAlg::Face> f;
  return f;
}

const Array<MeshAlg::Edge>& 
PointModel::edges() const 
{
  alwaysAssertM(false, "PointModel doesn't have geometry.");
  static Array<MeshAlg::Edge> e;
  return e;
}

const Array<MeshAlg::Vertex>& 
PointModel::vertices() const 
{
  alwaysAssertM(false, "PointModel doesn't have geometry.");
  static Array<MeshAlg::Vertex> v;
  return v;
}

const Array<MeshAlg::Face>& 
PointModel::weldedFaces() const 
{
  alwaysAssertM(false, "PointModel doesn't have geometry.");
  static Array<MeshAlg::Face> f;
  return f;
}

const Array<MeshAlg::Edge>& 
PointModel::weldedEdges() const 
{
  alwaysAssertM(false, "PointModel doesn't have geometry.");
  static Array<MeshAlg::Edge> e;
  return e;
}

const Array<MeshAlg::Vertex>& 
PointModel::weldedVertices() const 
{
  alwaysAssertM(false, "PointModel doesn't have geometry.");
  static Array<MeshAlg::Vertex> v;
  return v;
}

bool 
PointModel::hasTexCoords() const 
{
  alwaysAssertM(false, "PointModel doesn't have geometry.");
  return false;
}

const Array<Vector2>&  
PointModel::texCoords() const 
{
  alwaysAssertM(false, "PointModel doesn't have geometry.");
  static Array<Vector2> v;
  return v;
}


void 
PointModel::getObjectSpaceBoundingSphere(Sphere& s) const 
{
}

void 
PointModel::getObjectSpaceBoundingBox(Box& b) const 
{
}

int 
PointModel::numBoundaryEdges() const 
{
  alwaysAssertM(false, "PointModel doesn't have geometry.");
  return 0;
}

int 
PointModel::numWeldedBoundaryEdges() const 
{
  alwaysAssertM(false, "PointModel doesn't have geometry.");
  return 0;
}

const Array<Vector3>& 
PointModel::objectSpaceFaceNormals(bool normalize) const 
{
  alwaysAssertM(false, "PointModel doesn't have geometry.");
  static Array<Vector3> v;
  return v;
}

} // end namespace

