#include "TriStripModel.H"
#include <Shadows.H>

#define VAR_INC_SIZE 10000
using namespace G3D;
namespace DrawOnAir {


TriStripModel::TriStripModel(GfxMgrRef gfxMgr)
{
  _gfxMgr = gfxMgr;
  _nextModelID = 0;
  _movieFrame = 0;
  _numPointsThatFitInVarArea = 0;
}  

TriStripModel::~TriStripModel()
{
}

void
TriStripModel::setMovieFrame(int frame)
{
  _movieFrame = frame;
}

void
TriStripModel::hideModel(int modelID)
{
  if (!_hiddenModels.contains(modelID)) {
    _hiddenModels.append(modelID);
  }
}

void
TriStripModel::showModel(int modelID)
{
  int index = _hiddenModels.findIndex(modelID);
  if (index != -1) {
    _hiddenModels.fastRemove(index);
  }
}

void
TriStripModel::updateVARArea()
{
  // Resize VARArea if necessary
  if (_vertsArray.size() >= _numPointsThatFitInVarArea) {
    while (_numPointsThatFitInVarArea < _vertsArray.size()) {
      _numPointsThatFitInVarArea += VAR_INC_SIZE;
    }

    const int numVARs = 5;
    size_t sizeNeeded = (3*sizeof(Vector2) + 2*sizeof(Vector3))*_numPointsThatFitInVarArea + 8*numVARs;
    _varArea = VARArea::create(sizeNeeded, VARArea::WRITE_ONCE);
    if (_varArea.isNull()) {
      cerr << "ERROR: Out of VAR Area room!" << endl;
    }
  }

  // Store VARs
  _varArea->reset(); // this reset could be slower than needed if you're only changing a few points!
  _vertsVAR     = VAR(_vertsArray, _varArea);
  _normalsVAR   = VAR(_normalsArray, _varArea);
  _texCoord0VAR = VAR(_texCoord0Array, _varArea);
  _texCoord1VAR = VAR(_texCoord1Array, _varArea);
  _texCoord2VAR = VAR(_texCoord2Array, _varArea);
}

int
TriStripModel::addModel(const Array<Vector3> &vertices, 
                        const Array<Vector3> &normals,
                        const Array<Vector2> &texCoord0,
                        const Array<Vector2> &texCoord1,
                        const Array<Vector2> &texCoord2,
                        bool twoSided,
                        int layer,
                        int movieFrame)
{ 
  int start = _vertsArray.size();
  _vertsArray.append(vertices);
  _normalsArray.append(normals);
  _texCoord0Array.append(texCoord0);
  _texCoord1Array.append(texCoord1);
  _texCoord2Array.append(texCoord2);
  updateVARArea();

  Array<int> indices;
  int end = start+vertices.size();
  for (int i=start;i<end;i++) {
    indices.append(i);
  }
  int id = _nextModelID;
  _nextModelID++;
  _modelIndices.set(id, indices);

  _twoSided.set(id, twoSided);
  _layers.set(id, layer);
  _movieFrames.set(id, movieFrame);
    
  return id;
}

void
TriStripModel::updateModel(int modelID,
                           const Array<Vector3> &vertices, 
                           const Array<Vector3> &normals,
                           const Array<Vector2> &texCoord0,
                           const Array<Vector2> &texCoord1,
                           const Array<Vector2> &texCoord2)
{
  if (_modelIndices.containsKey(modelID)) {
    Array<int> &indices = _modelIndices[modelID];

    for (int i=0;i<indices.size();i++) {
      int index = indices[i];
      _vertsArray[index] = vertices[i];
      _normalsArray[index] = normals[i];
      _texCoord0Array[index] = texCoord0[i];
      _texCoord1Array[index] = texCoord1[i];
      _texCoord2Array[index] = texCoord2[i];
    }
    int start = indices.size();
    for (int i=start;i<vertices.size();i++) {
      indices.append(_vertsArray.size());
      _vertsArray.append(vertices[i]);
      _normalsArray.append(normals[i]);
      _texCoord0Array.append(texCoord0[i]);
      _texCoord1Array.append(texCoord1[i]);
      _texCoord2Array.append(texCoord2[i]);
    }

    updateVARArea();
  }
}

void
TriStripModel::deleteModel(int modelID)
{
  Array<Vector3> vertsArray;
  Array<Vector3> normalsArray;
  Array<Vector2> texCoord0Array;
  Array<Vector2> texCoord1Array;
  Array<Vector2> texCoord2Array;

  // take this model out
  if (!_modelIndices.containsKey(modelID)) {
    return;
  }
  _modelIndices.remove(modelID);

  // generate new data arrays that contain only info from the remaining models
  Array<int> keys = _modelIndices.getKeys();
  for (int i=0;i<keys.size();i++) {
    Array<int> newIndices;
    Array<int> &indices = _modelIndices[keys[i]];
    for (int j=0;j<indices.size();j++) {
      newIndices.append(vertsArray.size());
      vertsArray.append(_vertsArray[indices[j]]);
      normalsArray.append(_normalsArray[indices[j]]);
      texCoord0Array.append(_texCoord0Array[indices[j]]);
      texCoord1Array.append(_texCoord1Array[indices[j]]);
      texCoord2Array.append(_texCoord2Array[indices[j]]);
    }
    _modelIndices.set(keys[i], newIndices);
  }

  // replace old arrays with the new
  _vertsArray     = vertsArray;
  _normalsArray   = normalsArray;
  _texCoord0Array = texCoord0Array;
  _texCoord1Array = texCoord1Array;
  _texCoord2Array = texCoord2Array;

  updateVARArea();
}




PosedModel::Ref 
TriStripModel::pose(const CoordinateFrame& cframe) 
{
  _frame = cframe;
  return this;
}

void 
TriStripModel::render(RenderDevice* rd) const 
{ 
  if (_modelIndices.size() == 0) {
    return;
  }

  // 2 passes: 1st does normal rendering, 2nd does shadows
  int smax = 1;
  if (getShadowsOn()) {
    smax = 2;
  }
  for (int s=0;s<smax;s++) {

    rd->pushState();
    rd->setObjectToWorldMatrix(_frame);
    rd->setCullFace(RenderDevice::CULL_NONE);
    rd->setAlphaTest(RenderDevice::ALPHA_GREATER, 0.05);
    rd->setTexture(0, _gfxMgr->getTexture("BrushTipsTile"));  
    rd->setTextureCombineMode(0, RenderDevice::TEX_MODULATE);

    if (s == 0) {
      rd->setShadeMode(RenderDevice::SHADE_SMOOTH);
      rd->enableTwoSidedLighting();
      rd->setColor(Color3::white());
      rd->setShininess(125);           // 0 -> 255
      rd->setSpecularCoefficient(0.1); // 0 -> 1
      rd->setTexture(1, _gfxMgr->getTexture("PatternsTile"));
      rd->setTextureCombineMode(1, RenderDevice::TEX_MODULATE);   
      if (GLCaps::numTextureUnits() > 2) {
        rd->setTexture(2, _gfxMgr->getTexture("ColorSwatchTable"));
        rd->setTextureCombineMode(2, RenderDevice::TEX_MODULATE);   
      }
    }
    else {
      pushShadowState(rd);
    }
       
    rd->beginIndexedPrimitives();
    if (s == 0) {
      rd->setNormalArray(_normalsVAR);
      rd->setTexCoordArray(1, _texCoord1VAR);
      if (GLCaps::numTextureUnits() > 2) {
        rd->setTexCoordArray(2, _texCoord2VAR);
      }
    }
    rd->setTexCoordArray(0, _texCoord0VAR);
    rd->setVertexArray(_vertsVAR);  
    
    Array<int> keys = _modelIndices.getKeys();
    for (int m=0;m<keys.size();m++) {
      if ((!_hiddenLayers.contains(_layers[keys[m]])) && 
          (!_hiddenModels.contains(keys[m])) &&
          (_movieFrames[keys[m]] == _movieFrame)) {
        if (_twoSided[keys[m]]) {
          rd->setCullFace(RenderDevice::CULL_NONE);   
        }
        else {
          rd->setCullFace(RenderDevice::CULL_BACK);
        }
        const Array<int> &indices = _modelIndices[keys[m]];
        rd->sendIndices(PrimitiveType::TRIANGLE_STRIP, indices);
	
		/* Render without VARs */
		/*
	    rd->beginPrimitive(PrimitiveType::TRIANGLE_STRIP);
        for (int i=0;i<indices.size();i++) {
          int index = indices[i];
          rd->setNormal(_normalsArray[index]);
          rd->setTexCoord(0, _texCoord0Array[index]);
          rd->setTexCoord(1, _texCoord1Array[index]);
          rd->setTexCoord(2, _texCoord2Array[index]);
          rd->sendVertex(_vertsArray[index]);
        }
        rd->endPrimitive();
		*/
      }
    }
    rd->endIndexedPrimitives();
    
    
    

    if (s==1) {
      popShadowState(rd);
    }
    
    rd->popState();    
  }
}

std::string 
TriStripModel::name() const 
{
  return "TriStripModel";
}

void 
TriStripModel::getCoordinateFrame(CoordinateFrame& c) const 
{
  c = _frame;
}


const MeshAlg::Geometry& 
TriStripModel::objectSpaceGeometry() const 
{
  alwaysAssertM(false, "TriStripModel doesn't have geometry.");
  static MeshAlg::Geometry g;
  return g;
}

const Array<int>& 
TriStripModel::triangleIndices() const 
{
  alwaysAssertM(false, "TriStripModel doesn't have geometry.");
  static Array<int> a;
  return a;
}


const Array<MeshAlg::Face>& 
TriStripModel::faces() const 
{
  alwaysAssertM(false, "TriStripModel doesn't have geometry.");
  static Array<MeshAlg::Face> f;
  return f;
}

const Array<MeshAlg::Edge>& 
TriStripModel::edges() const 
{
  alwaysAssertM(false, "TriStripModel doesn't have geometry.");
  static Array<MeshAlg::Edge> e;
  return e;
}

const Array<MeshAlg::Vertex>& 
TriStripModel::vertices() const 
{
  alwaysAssertM(false, "TriStripModel doesn't have geometry.");
  static Array<MeshAlg::Vertex> v;
  return v;
}

const Array<MeshAlg::Face>& 
TriStripModel::weldedFaces() const 
{
  alwaysAssertM(false, "TriStripModel doesn't have geometry.");
  static Array<MeshAlg::Face> f;
  return f;
}

const Array<MeshAlg::Edge>& 
TriStripModel::weldedEdges() const 
{
  alwaysAssertM(false, "TriStripModel doesn't have geometry.");
  static Array<MeshAlg::Edge> e;
  return e;
}

const Array<MeshAlg::Vertex>& 
TriStripModel::weldedVertices() const 
{
  alwaysAssertM(false, "TriStripModel doesn't have geometry.");
  static Array<MeshAlg::Vertex> v;
  return v;
}

bool 
TriStripModel::hasTexCoords() const 
{
  alwaysAssertM(false, "TriStripModel doesn't have geometry.");
  return false;
}

const Array<Vector2>&  
TriStripModel::texCoords() const 
{
  alwaysAssertM(false, "TriStripModel doesn't have geometry.");
  static Array<Vector2> v;
  return v;
}


void 
TriStripModel::getObjectSpaceBoundingSphere(Sphere& s) const 
{
}

void 
TriStripModel::getObjectSpaceBoundingBox(AABox& b) const 
{
}

int 
TriStripModel::numBoundaryEdges() const 
{
  alwaysAssertM(false, "TriStripModel doesn't have geometry.");
  return 0;
}

int 
TriStripModel::numWeldedBoundaryEdges() const 
{
  alwaysAssertM(false, "TriStripModel doesn't have geometry.");
  return 0;
}

const Array<Vector3>& 
TriStripModel::objectSpaceFaceNormals(bool normalize) const 
{
  alwaysAssertM(false, "TriStripModel doesn't have geometry.");
  static Array<Vector3> v;
  return v;
}

} // end namespace

