
#include <ConfigVal.H>
#include "PointModelSprites.H"
using namespace G3D;
#define VAR_INC_SIZE 10000

// TODO: Add this to G3D's glext.h header
#ifndef GL_VERTEX_PROGRAM_POINT_SIZE
#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
#endif


/* 
  For rendering with the PointSprite extension:
  - Send only one vertex per point splat
  Tex0.xy -> reserved for glPontSprite
  Tex1.x  =  Column index into the Color Swath Table
  Tex1.y  =  G3D::Line weight in range [0->1] (Row index in Color Swath Table)
  Tex1.z  =  Size of the point in RoomSpace coordinates
  Tex2.x  =  BrushTip index
  Tex2.y  =  Number of BrushTips in the texture
  in vertex prog:
    calculate point size;
  in frag prog:
    Tex0.x = Tex2.x/Tex2.y + Tex0.x/Tex2.y;
*/


namespace DrawOnAir {


PointModelSprites::PointModelSprites(GfxMgrRef gfxMgr) : PointModel()
{
  debugAssert(Shader::supportsVertexShaders());
  debugAssert(Shader::supportsPixelShaders());
  _gfxMgr = gfxMgr;
  _lastIndexInBSPTree = -1;
  _numPointsThatFitInVarArea = 0;
  _nextModelID = 0;
  _brushTipTex = _gfxMgr->getTexture("BrushTips");

  std::string vshader = MinVR::ConfigVal("PointModelVS","share/shaders/PointModelSprite.glsl.vrt",false);
  std::string pshader = MinVR::ConfigVal("PointModelPS","share/shaders/PointModelSprite.glsl.frg",false);
  cout << vshader << endl;
  cout << pshader << endl;
  _shader = Shader::fromFiles(vshader, pshader);
  _shader->args.set("avgScreenResolution", MinVR::ConfigVal("AvgScreenResolution",1280,false));
  _shader->args.set("spriteTex", _brushTipTex);
  _shader->args.set("colorTex", _gfxMgr->getTexture("ColorSwatchTable"));
  _shader->args.set("scale", 1.0/_gfxMgr->getRoomToVirtualSpaceScale());

  updateVARArea();
}  

PointModelSprites::~PointModelSprites()
{
}

void
PointModelSprites::updateVARArea()
{
  if (_pointsArray.size() >= _numPointsThatFitInVarArea) {
    while (_numPointsThatFitInVarArea < _pointsArray.size()) {
      _numPointsThatFitInVarArea += VAR_INC_SIZE;
    }

    if (_numPointsThatFitInVarArea != 0) {
      cout << "PointModelSprites: Allocating new VARArea (size = " << _numPointsThatFitInVarArea << ")" << endl;
    }

    // add 8 * number of individual VARs to be stored in the VAR array to the 
    // calculated size because the VAR arrays are byte aligned in memory.
    const int numVARs = 3; // _pointsVAR, _texCoord1Array, _texCoord2Array
    size_t sizeNeeded = (sizeof(Vector2) + 2*sizeof(Vector3))*_numPointsThatFitInVarArea + 8*numVARs;
    _varArea = VARArea::create(sizeNeeded, VARArea::WRITE_ONCE);
    
    if (_varArea.isNull()) {
      // TODO: do an emergency save of the artwork here..
      // OR, add support for rendering without VARAreas.
      alwaysAssertM(_varArea.notNull(), "PointModelSprites constructor: Ran out of VAR Area!");
    }
  }

  if (_pointsArray.size()) {
    _varArea->reset();
    _pointsVAR    = VAR(_pointsArray, _varArea);
    debugAssert(_pointsVAR.valid());
    _texCoord1VAR = VAR(_texCoord1Array, _varArea);
    debugAssert(_texCoord1VAR.valid());
    _texCoord2VAR = VAR(_texCoord2Array, _varArea);
    debugAssert(_texCoord2VAR.valid());
  }
}

int
PointModelSprites::addModel(const Array<Vector3> &points, const Array<double> &weights, 
                       const double &colorSwatchIndex, const double &lineWidth, const int &brushTip)
{
  debugAssert(points.size() == weights.size());
  int modelID = _nextModelID;
  _nextModelID++;

  int numBrushTips = _brushTipTex->width()/_brushTipTex->height();

  // add any new points and weights
  Array<int> indices;
  for (int i=0;i<points.size();i++) {
    int n = _pointsArray.size();

    _pointsArray.append(points[i]);
    _texCoord1Array.append(Vector3(colorSwatchIndex, weights[i], lineWidth*weights[i]));
    _texCoord2Array.append(Vector2(brushTip, numBrushTips));

    indices.append(n);
    _sortedIndices.append(n);
  }
  _indicesByModelID.set(modelID, indices);

  updateVARArea();

  
  // update BSP Tree
  for (int i=0;i<indices.size();i++) {
    if (indices[i] > _lastIndexInBSPTree) {
      if (_bspTree.isNull()) {
        _bspTree = new PointBSPTree(points[i], indices[i]);
      }
      else {
        _bspTree->insert(points[i], indices[i]);
      }
      _lastIndexInBSPTree = indices[i];
    }
  }
  

  debugAssertGLOk();
  return modelID;
}

void
PointModelSprites::updateModel(int modelID, const Array<Vector3> &points, const Array<double> &weights, 
                          const double &colorSwatchIndex, const double &lineWidth, const int &brushTip)
{
  debugAssert(points.size() == weights.size());

  int numBrushTips = _brushTipTex->width()/_brushTipTex->height();
  Array<int> indices = _indicesByModelID[modelID];

  // update existing points and weights
  for (int i=0;i<indices.size();i++) {
    _pointsArray[indices[i]] = points[i];
    _texCoord1Array[indices[i]] = Vector3(colorSwatchIndex, weights[i], lineWidth*weights[i]);
    _texCoord2Array[indices[i]] = Vector2(brushTip, numBrushTips);
  }

  // add any new points and weights
  for (int i=indices.size();i<points.size();i++) {
    int n = _pointsArray.size();
    _pointsArray.append(points[i]);
    _texCoord1Array.append(Vector3(colorSwatchIndex, weights[i], lineWidth*weights[i]));
    _texCoord2Array.append(Vector2(brushTip, numBrushTips));    
    indices.append(n);
    _sortedIndices.append(n);
  }
  _indicesByModelID.set(modelID, indices);

  updateVARArea();


  
  // update BSP Tree
  
  // BUG: You need to actually rebuild the tree here when an update
  // changes the location of the points in a mark because the tree
  // will be outdated.  But, rebuilding the tree is too slow to do
  // this each frame, so need to implement something that handles this
  // case.

  for (int i=0;i<indices.size();i++) {
    if (indices[i] > _lastIndexInBSPTree) {
      if (_bspTree.isNull()) {
        _bspTree = new PointBSPTree(points[i], indices[i]);
      }
      else {
        _bspTree->insert(points[i], indices[i]);
      }
      _lastIndexInBSPTree = indices[i];
    }
  }
  

  /***
  // Rebuild BSP Tree
  _bspTree = NULL;
  Array<int> keys = _indicesByModelID.getKeys();
  for (int k=0;k<keys.size();k++) {
    Array<int> indices = _indicesByModelID[keys[k]];
    for (int i=0;i<indices.size();i++) {
      if (_bspTree.isNull()) { 
        _bspTree = new PointBSPTree(_pointsArray[indices[i]], indices[i]);
      }
      else {
        _bspTree->insert(_pointsArray[indices[i]], indices[i]);
      }
    }
  }
  _lastIndexInBSPTree = _pointsArray.size()-1;
  ***/

  debugAssertGLOk();
}


void
PointModelSprites::deleteModel(int modelID)
{
  // Rebuild all these arrays leaving the values from the points in this model out of them
  Array<Vector3> pointsArray;
  Array<Vector3> texCoord1Array;
  Array<Vector2> texCoord2Array;

  _sortedIndices.clear();
  
  // take this model out
  _indicesByModelID.remove(modelID);

  // get all the remaining models
  Array<int> keys = _indicesByModelID.getKeys();

  // generate new data arrays that contain only info from the remaining models
  for (int i=0;i<keys.size();i++) {
    Array<int> newIndices;
    Array<int> indices = _indicesByModelID[keys[i]];
    for (int j=0;j<indices.size();j++) {
      _sortedIndices.append(pointsArray.size());
      newIndices.append(pointsArray.size());
      pointsArray.append(_pointsArray[indices[j]]);
      texCoord1Array.append(_texCoord1Array[indices[j]]);
      texCoord2Array.append(_texCoord2Array[indices[j]]);
    }

    // update the indices for this model
    _indicesByModelID.set(keys[i], newIndices);
  }

  // replace old arrays with the new
  _pointsArray    = pointsArray;
  _texCoord1Array = texCoord1Array;
  _texCoord2Array = texCoord2Array;

  // rebuild the BSP tree
  _bspTree = NULL;
  for (int k=0;k<keys.size();k++) {
    Array<int> indices = _indicesByModelID[keys[k]];
    for (int i=0;i<indices.size();i++) {
      if (_bspTree.isNull()) { 
        _bspTree = new PointBSPTree(_pointsArray[indices[i]], indices[i]);
      }
      else {
        _bspTree->insert(_pointsArray[indices[i]], indices[i]);
      }
    }
  }
  _lastIndexInBSPTree = _pointsArray.size()-1;
}


void 
PointModelSprites::quickSortPointsByDepth(int left, int right)
{
  double pivot; // stores the distance to the camera from teh numbers array at the pivot pt
  int ipivot;   // stores the index of the point from the _sortedIndices array
  int l_hold, r_hold;

  l_hold = left;
  r_hold = right;
  pivot = _depthArray[left];
  ipivot = _sortedIndices[left];
  while (left < right) {
    while ((_depthArray[right] <= pivot) && (left < right))
      right--;
    if (left != right) {
      _depthArray[left] = _depthArray[right];
      _sortedIndices[left] = _sortedIndices[right];
      left++;
    }
    while ((_depthArray[left] >= pivot) && (left < right))
      left++;
    if (left != right) {
      _depthArray[right] = _depthArray[left];
      _sortedIndices[right] = _sortedIndices[left];
      right--;
    }
  }
  _depthArray[left] = pivot;
  _sortedIndices[left] = ipivot;
  pivot = left;
  left = l_hold;
  right = r_hold;
  if (left < pivot)
    quickSortPointsByDepth(left, pivot-1);
  if (right > pivot)
    quickSortPointsByDepth(pivot+1, right);
}


PosedModel::Ref 
PointModelSprites::pose(const CoordinateFrame& cframe) 
{
  _frame = cframe;  // the virtual to room space transformation

  if (_bspTree.notNull()) {
    _sortedIndices.clear();
    // camera is in room space
    // points are in virtual space
    Vector3 camPosVS = _frame.pointToObjectSpace(_gfxMgr->getCamera()->getCameraPos());
    _bspTree->getSortedListOfIndices(camPosVS, _sortedIndices);
  }

  /**
  if (_sortedIndices.size()) {
    Vector3 camPos = _gfxMgr->getCamera()->getCameraPos();
    Vector3 lookVec = _gfxMgr->getCamera()->getLookVec();

    if (_depthArray.size() != _sortedIndices.size()) {
      _depthArray.resize(_sortedIndices.size());
    }
    for (int i=0;i<_sortedIndices.size();i++) {
      _depthArray[i] = (_frame.pointToWorldSpace(_pointsArray[_sortedIndices[i]]) - camPos).dot(lookVec);
    }

    quickSortPointsByDepth(0, _sortedIndices.size()-1);
  } 
  **/  

  _shader->args.set("scale", 1.0/_gfxMgr->getRoomToVirtualSpaceScale());

  return this;
}



// This renders using the static arrays of points/weights/indices so one call to render
// will render the points from all the pointmodels in the program.
void 
PointModelSprites::render(RenderDevice* renderDevice) const 
{ 
  if (!_pointsArray.size()) {
    return;
  }

  debugAssertGLOk();
  renderDevice->pushState();

  glPushAttrib(GL_ALL_ATTRIB_BITS);  
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  glEnable(GL_POINT_SPRITE_NV);
  glTexEnvf( GL_POINT_SPRITE_NV, GL_COORD_REPLACE_NV, GL_TRUE );

  renderDevice->setObjectToWorldMatrix(_frame);
  renderDevice->disableLighting();
  renderDevice->setDepthWrite(false);
  renderDevice->setColor(Color3::white());
  renderDevice->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
  renderDevice->setTexture(0, _brushTipTex);
  renderDevice->setTexture(1, _gfxMgr->getTexture("ColorSwatchTable"));

  renderDevice->setShader(_shader);


  // Render using VARs
  if (_sortedIndices.size()) {
    debugAssert(_pointsVAR.valid());
    debugAssert(_texCoord1VAR.valid());
    debugAssert(_texCoord2VAR.valid());

    renderDevice->beginIndexedPrimitives(); 
    renderDevice->setVertexArray(_pointsVAR);
    renderDevice->setTexCoordArray(1, _texCoord1VAR);
    renderDevice->setTexCoordArray(2, _texCoord2VAR);
    renderDevice->sendIndices(PrimitiveType::POINTS, _sortedIndices);
    renderDevice->endIndexedPrimitives();
  }
  

  // Render without VARs
  /**
  renderDevice->beginPrimitive(PrimitiveType::POINTS);
  for (int  i=0;i<_sortedIndices.size();i++) {
    renderDevice->setTexCoord(1, _texCoord1Array[i]);
    renderDevice->setTexCoord(2, _texCoord2Array[i]);
    renderDevice->sendVertex(_pointsArray[i]);
  }
  renderDevice->endPrimitive();
  **/



  glDisable(GL_POINT_SPRITE_NV);
  glDisable(GL_POINT_SMOOTH);
  glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
  glPopAttrib();

  renderDevice->popState();
  debugAssertGLOk();
}


} // end namespace

