#include "PointModelSW.H"
#include <ConfigVal.H>
using namespace G3D;
#define VAR_INC_SIZE 10000

/*
  For rendering without the PointSprite extension and without fragment programs:
  - Send 4 vertices per point splat
  - Set the screenRight and screenUp directions as parameters to the shader
  Tex0.x  =  S texture coord for BrushTip texture
  Tex0.y  =  T texture coord for BrushTip texture (0 or 1)
  Tex1.x  =  Column index into the Color Swath Table
  Tex1.y  =  G3D::Line weight in range [0->1] (Row index in Color Swath Table)
  Tex2.x  =  X component of the offset of the vertex in RoomCoords
  Tex2.y  =  Y component of the offset of the vertex in RoomCoords
  in vertex prog:
    Pos.x = Tex2.x * screenRight;
    Pox.y = Tex2.y * screenUp;

  For rendering without even vertex prog support:
  - Same as above, but will need to manually offset each vertex by hand each frame
*/


namespace DrawOnAir {


PointModelSW::PointModelSW(MinVR::GfxMgrRef gfxMgr) : PointModel()
{
  _gfxMgr = gfxMgr;
  _lastIndexInBSPTree = -1;
  _numPointsThatFitInVarArea = 0;
  _nextModelID = 0;
  _useVertexProg = (Shader::supportsVertexShaders() && (!MinVR::ConfigVal("PointModel_ForceSW", false, false)));
  _brushTipTex = _gfxMgr->getTexture("BrushTips");

  if (_useVertexProg) {
    //cout << "PointModelSW: Using vertex programs." << endl;
    _vShader = Shader::fromFiles("../../kdata/shaders/PointModelBillboard.glsl.vrt", "");
  }
  else {
    //cout << "PointModelSW: Software only rendering." << endl;
  }


  updateVARArea();
}  

PointModelSW::~PointModelSW()
{
}

void
PointModelSW::updateVARArea()
{
  if (_pointsArray.size() >= _numPointsThatFitInVarArea) {
    while (_numPointsThatFitInVarArea < _pointsArray.size()) {
      _numPointsThatFitInVarArea += VAR_INC_SIZE;
    }

    if (_numPointsThatFitInVarArea != 0) {
      cout << "PointModelSW: Allocating new VARArea (size = " << _numPointsThatFitInVarArea << ")" << endl;
    }

    if (_useVertexProg) {
      // add 8 * number of individual VARs to be stored in the VAR array to the 
      // calculated size because the VAR arrays are byte aligned in memory.
      const int numVARs = 4; // _posedPointsVAR, _texCoord0Array, _texCoord1Array, _texCoord2Array
      size_t sizeNeeded = (3*sizeof(Vector2) + sizeof(Vector3))*_numPointsThatFitInVarArea + 8*numVARs;
      _varArea = VARArea::create(sizeNeeded, VARArea::WRITE_ONCE);
      
      if (_varArea.isNull()) {
        // TODO: do an emergency save of the artwork here..
        // OR, add support for rendering without VARAreas.
        alwaysAssertM(_varArea.notNull(), "PointModelSW constructor: Ran out of VAR Area!");
      }
    }
    else {
      const int numVARs = 3; // _posedPointsVAR, _texCoord0Array, _texCoord1Array
      size_t sizeNeeded = (2*sizeof(Vector2) + sizeof(Vector3))*_numPointsThatFitInVarArea + 8*numVARs;
      _varArea = VARArea::create(sizeNeeded, VARArea::WRITE_ONCE);      

      if (_varArea.isNull()) {
        alwaysAssertM(_varArea.notNull(), "PointModelSW constructor: Ran out of VAR Area!");
      }
    }
  }

  if (_pointsArray.size()) {
    _varArea->reset();
    if (_useVertexProg) {
      _pointsVAR    = VAR(_pointsArray, _varArea);
    }
    else {
      _pointsVAR    = VAR(_posedPointsArray, _varArea);
    }
    debugAssert(_pointsVAR.valid());
    _texCoord0VAR = VAR(_texCoord0Array, _varArea);
    debugAssert(_texCoord0VAR.valid());
    _texCoord1VAR = VAR(_texCoord1Array, _varArea);
    debugAssert(_texCoord1VAR.valid());
    if (_useVertexProg) {
      _texCoord2VAR = VAR(_texCoord2Array, _varArea);
      debugAssert(_texCoord2VAR.valid());
    }
  }
}

int
PointModelSW::addModel(const Array<Vector3> &points, const Array<double> &weights, 
                       const double &colorSwatchIndex, const double &lineWidth, const int &brushTip)
{
  debugAssert(points.size() == weights.size());
  int modelID = _nextModelID;
  _nextModelID++;

  int numBrushTips = MinVR::ConfigVal("NumBrushTips", _brushTipTex->width()/_brushTipTex->height(), false);
  
  double s1 = (double)brushTip/(double)numBrushTips;
  double s2 = (double)(brushTip+1)/(double)numBrushTips;

  // add any new points and weights
  Array<int> indices;
  for (int i=0;i<points.size();i++) {
    double hw = lineWidth * weights[i] / 2.0;
    int n = _pointsArray.size();
    _pointsArray.append(points[i]);
    _texCoord0Array.append(Vector2(s1, 1));
    _texCoord1Array.append(Vector2(colorSwatchIndex, weights[i]));
    _texCoord2Array.append(Vector2(-hw, hw));

    _pointsArray.append(points[i]);
    _texCoord0Array.append(Vector2(s1, 0));
    _texCoord1Array.append(Vector2(colorSwatchIndex, weights[i]));
    _texCoord2Array.append(Vector2(-hw, -hw));

    _pointsArray.append(points[i]);
    _texCoord0Array.append(Vector2(s2, 0));
    _texCoord1Array.append(Vector2(colorSwatchIndex, weights[i]));
    _texCoord2Array.append(Vector2(hw, -hw));

    _pointsArray.append(points[i]);
    _texCoord0Array.append(Vector2(s2, 1));
    _texCoord1Array.append(Vector2(colorSwatchIndex, weights[i]));
    _texCoord2Array.append(Vector2(hw, hw));

    indices.append(n);
    indices.append(n+1);
    indices.append(n+2);
    indices.append(n+3);

    _sortedIndices.append(n);
  }
  _indicesByModelID.set(modelID, indices);

  updateVARArea();

  /****
  // update BSP Tree
  for (int i=0;i<indices.size();i+=4) {
    if (indices[i] > _lastIndexInBSPTree) {
      if (_bspTree.isNull()) {
        _bspTree = new PointBSPTree(points[i/4], indices[i]);
      }
      else {
        _bspTree->insert(points[i/4], indices[i]);
      }
      _lastIndexInBSPTree = indices[i];
    }
  }
  ****/

  debugAssertGLOk();
  return modelID;
}

void
PointModelSW::updateModel(int modelID, const Array<Vector3> &points, const Array<double> &weights, 
                          const double &colorSwatchIndex, const double &lineWidth, const int &brushTip)
{
  debugAssert(points.size() == weights.size());

  int numBrushTips = MinVR::ConfigVal("NumBrushTips", _brushTipTex->width()/_brushTipTex->height(), false);

  double s1 = (double)brushTip/(double)numBrushTips;
  double s2 = (double)(brushTip+1)/(double)numBrushTips;

  Array<int> indices = _indicesByModelID[modelID];

  // update existing points and weights
  for (int i=0;i<indices.size();i+=4) {
    int i2 = i/4;
    double hw = lineWidth * weights[i2] / 2.0;

    _pointsArray[indices[i]] = points[i2];
    _texCoord0Array[indices[i]] = Vector2(s1, 1);
    _texCoord1Array[indices[i]] = Vector2(colorSwatchIndex, weights[i2]);
    _texCoord2Array[indices[i]] = Vector2(-hw, hw);
    
    _pointsArray[indices[i+1]] = points[i2];
    _texCoord0Array[indices[i+1]] = Vector2(s1, 0);
    _texCoord1Array[indices[i+1]] = Vector2(colorSwatchIndex, weights[i2]);
    _texCoord2Array[indices[i+1]] = Vector2(-hw, -hw);

    _pointsArray[indices[i+2]] = points[i2];
    _texCoord0Array[indices[i+2]] = Vector2(s2, 0);
    _texCoord1Array[indices[i+2]] = Vector2(colorSwatchIndex, weights[i2]);
    _texCoord2Array[indices[i+2]] = Vector2(hw, -hw);

    _pointsArray[indices[i+3]] = points[i2];
    _texCoord0Array[indices[i+3]] = Vector2(s2, 1);
    _texCoord1Array[indices[i+3]] = Vector2(colorSwatchIndex, weights[i2]);
    _texCoord2Array[indices[i+3]] = Vector2(hw, hw);    
  }

  // add any new points and weights
  int start=indices.size()/4;
  for (int i=start;i<points.size();i++) {
    double hw = lineWidth * weights[i] / 2.0;
    int n = _pointsArray.size();
    _pointsArray.append(points[i]);
    _texCoord0Array.append(Vector2(s1, 1));
    _texCoord1Array.append(Vector2(colorSwatchIndex, weights[i]));
    _texCoord2Array.append(Vector2(-hw, hw));

    _pointsArray.append(points[i]);
    _texCoord0Array.append(Vector2(s1, 0));
    _texCoord1Array.append(Vector2(colorSwatchIndex, weights[i]));
    _texCoord2Array.append(Vector2(-hw, -hw));

    _pointsArray.append(points[i]);
    _texCoord0Array.append(Vector2(s2, 0));
    _texCoord1Array.append(Vector2(colorSwatchIndex, weights[i]));
    _texCoord2Array.append(Vector2(hw, -hw));

    _pointsArray.append(points[i]);
    _texCoord0Array.append(Vector2(s2, 1));
    _texCoord1Array.append(Vector2(colorSwatchIndex, weights[i]));
    _texCoord2Array.append(Vector2(hw, hw));


    indices.append(n);
    indices.append(n+1);
    indices.append(n+2);
    indices.append(n+3);

    _sortedIndices.append(n);
  }
  _indicesByModelID.set(modelID, indices);

  updateVARArea();
 
  /***
  // update BSP Tree
  // BUG: I think you may need to actually rebuild the tree here because if an update
  // changes the location of the points in a mark the tree would be outdated.  Is rebuilding the
  // tree too slow to do this often?
  for (int i=0;i<indices.size();i+=4) {
    if (indices[i] > _lastIndexInBSPTree) {
      if (_bspTree.isNull()) {
        _bspTree = new PointBSPTree(points[i/4], indices[i]);
      }
      else {
        _bspTree->insert(points[i/4], indices[i]);
      }
      _lastIndexInBSPTree = indices[i];
    }
  }

  **

  // Rebuild BSP Tree
  _bspTree = NULL;
  Array<int> keys = _indicesByModelID.getKeys();
  for (int k=0;k<keys.size();k++) {
    Array<int> indices = _indicesByModelID[keys[k]];
    for (int i=0;i<indices.size();i+=4) {
      if (_bspTree.isNull()) { 
        _bspTree = new PointBSPTree(_pointsArray[indices[i]], indices[i]);
      }
      else {
        _bspTree->insert(_pointsArray[indices[i]], indices[i]);
      }
    }
  }
  _lastIndexInBSPTree = _pointsArray.size()-1;

  **/

  debugAssertGLOk();
}


void
PointModelSW::deleteModel(int modelID)
{
  // Rebuild all these arrays leaving the values from the points in this model out of them
  Array<Vector3> pointsArray;
  Array<Vector2> texCoord0Array;
  Array<Vector2> texCoord1Array;
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
    for (int j=0;j<indices.size();j+=4) {

      // for the billboard implementation, just add one point per 4 to the sortedIndices array
      _sortedIndices.append(pointsArray.size());

      // all 4 points per billboard are added to the points, texture, and indices arrays
      newIndices.append(pointsArray.size());
      pointsArray.append(_pointsArray[indices[j]]);
      texCoord0Array.append(_texCoord0Array[indices[j]]);
      texCoord1Array.append(_texCoord1Array[indices[j]]);
      texCoord2Array.append(_texCoord2Array[indices[j]]);

      newIndices.append(pointsArray.size());
      pointsArray.append(_pointsArray[indices[j+1]]);
      texCoord0Array.append(_texCoord0Array[indices[j+1]]);
      texCoord1Array.append(_texCoord1Array[indices[j+1]]);
      texCoord2Array.append(_texCoord2Array[indices[j+1]]);

      newIndices.append(pointsArray.size());
      pointsArray.append(_pointsArray[indices[j+2]]);
      texCoord0Array.append(_texCoord0Array[indices[j+2]]);
      texCoord1Array.append(_texCoord1Array[indices[j+2]]);
      texCoord2Array.append(_texCoord2Array[indices[j+2]]);

      newIndices.append(pointsArray.size());
      pointsArray.append(_pointsArray[indices[j+3]]);
      texCoord0Array.append(_texCoord0Array[indices[j+3]]);
      texCoord1Array.append(_texCoord1Array[indices[j+3]]);
      texCoord2Array.append(_texCoord2Array[indices[j+3]]);
    }

    // update the indices for this model
    _indicesByModelID.set(keys[i], newIndices);
  }

  // replace old arrays with the new
  _pointsArray    = pointsArray;
  _texCoord0Array = texCoord0Array;
  _texCoord1Array = texCoord1Array;
  _texCoord2Array = texCoord2Array;

  // rebuild the BSP tree
  /***
  _bspTree = NULL;
  for (int k=0;k<keys.size();k++) {
    Array<int> indices = _indicesByModelID[keys[k]];
    for (int i=0;i<indices.size();i+=4) {
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
}


void 
PointModelSW::quickSortPointsByDepth(int left, int right)
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
PointModelSW::pose(const CoordinateFrame& cframe) 
{
  _frame = cframe;  // contains the virtual to room space transformation


  /**
  if (_bspTree.notNull()) {
    _sortedIndices.clear();

    Vector3 cameraInVirtualSpace = _gfxMgr->roomPointToVirtualSpace(_gfxMgr->getCamera()->getCameraPos());
    _bspTree->getSortedListOfIndices(cameraInVirtualSpace, _sortedIndices);

    _allSortedIndices.clear();
    for (int i=0;i<_sortedIndices.size();i++) {
      _allSortedIndices.append(_sortedIndices[i]);
      _allSortedIndices.append(_sortedIndices[i]+1);
      _allSortedIndices.append(_sortedIndices[i]+2);
      _allSortedIndices.append(_sortedIndices[i]+3);
    }
  }**/


  if (_sortedIndices.size()) {
    Vector3 camPos = _gfxMgr->getCamera()->getCameraPos();
    Vector3 lookVec = _gfxMgr->getCamera()->getLookVec();

    if (_depthArray.size() != _sortedIndices.size()) {
      _depthArray.resize(_sortedIndices.size());
    }
    for (int i=0;i<_sortedIndices.size();i++) {
      _depthArray[i] = (_frame.pointToWorldSpace(_pointsArray[_sortedIndices[i]]) - camPos).dot(lookVec);
    }

    // QuickSort: In general better for a jumbled list, but ours is probably nearly sorted, so not sure
    // if it's better than insertion sort in this case.  TODO: test this, and/or do some quick test to see
    // if the list is actually sorted already before you try to resort it.
    //quickSortPointsByDepth(0, _sortedIndices.size()-1);
    
    /**
    // InsertionSort: Could be a good choice in this instance because the array is probably nearly sorted
    // from the last frame.
    for (int i=1;i<_sortedIndices.size();i++) {
      double d = _depthArray[i];
      int si = _sortedIndices[i];
      int j=i;
      while ((j>0) && (_depthArray[j-1] < d)) {
        _depthArray[j] = _depthArray[j-1];
        _sortedIndices[j] = _sortedIndices[j-1];
        j--;
      }
      _depthArray[j] = d;
      _sortedIndices[j] = si;
    }
    **/

    _allSortedIndices.resize(_sortedIndices.size()*4);
    for (int i=0;i<_sortedIndices.size();i++) {
      int index = _sortedIndices[i];
      int j=4*i;
      _allSortedIndices[j]   = index;
      _allSortedIndices[j+1] = index+1;
      _allSortedIndices[j+2] = index+2;
      _allSortedIndices[j+3] = index+3;
    }
  }

  // Apply offsets to the point locations to form a billboard rectangle
  Vector3 right = _gfxMgr->roomVectorToVirtualSpace(Vector3(1,0,0)).unit();
  Vector3 up    = _gfxMgr->roomVectorToVirtualSpace(Vector3(0,1,0)).unit();

  if (_useVertexProg) {
    _vShader->args.set("right", right);
    _vShader->args.set("up", up);
  }
  else {
    _posedPointsArray.resize(_pointsArray.size());
    for (int i=0;i<_pointsArray.size();i++) {
      Vector3 offset = _texCoord2Array[i][0]*right + _texCoord2Array[i][1]*up;
      _posedPointsArray[i] = _pointsArray[i] + offset;
    }
    updateVARArea();
  }
  
  return this;
}



// This renders using the static arrays of points/weights/indices so one call to render
// will render the points from all the pointmodels in the program.
void 
PointModelSW::render(RenderDevice* renderDevice) const 
{ 
  if (!_pointsArray.size()) {
    return;
  }

  debugAssertGLOk();
  renderDevice->pushState();
  renderDevice->setObjectToWorldMatrix(_frame);
  renderDevice->disableLighting();
  renderDevice->setColor(Color3::white());
  renderDevice->setDepthWrite(false);
  renderDevice->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
  //renderDevice->setAlphaTest(RenderDevice::ALPHA_GREATER, 0.05);
  renderDevice->setTexture(0, _brushTipTex);  
  renderDevice->setTextureCombineMode(0, RenderDevice::TEX_MODULATE);
  renderDevice->setTexture(1, _gfxMgr->getTexture("ColorSwatchTable"));
  renderDevice->setTextureCombineMode(1, RenderDevice::TEX_MODULATE);
  if (_useVertexProg) {
    renderDevice->setShader(_vShader);
  }

  // Render using VARs
  if (_allSortedIndices.size()) {
    debugAssert(_pointsVAR.valid());
    debugAssert(_texCoord0VAR.valid());
    debugAssert(_texCoord1VAR.valid());
    renderDevice->beginIndexedPrimitives(); 
    renderDevice->setVertexArray(_pointsVAR);
    renderDevice->setTexCoordArray(0, _texCoord0VAR);
    renderDevice->setTexCoordArray(1, _texCoord1VAR);
    if (_useVertexProg) {
      renderDevice->setTexCoordArray(2, _texCoord2VAR);  
    }
    renderDevice->sendIndices(PrimitiveType::QUADS, _allSortedIndices);
    renderDevice->endIndexedPrimitives();
  }
  

  // Render without VARs
  /**
  renderDevice->beginPrimitive(PrimitiveType::QUADS);
  for (int  i=0;i<_sortedIndices.size();i++) {
    int index = _sortedIndices[i];
    renderDevice->setTexCoord(0, _texCoord0Array[index]);
    renderDevice->setTexCoord(1, _texCoord1Array[index]);
    renderDevice->setTexCoord(2, _texCoord2Array[index]);
    renderDevice->sendVertex(_posedPointsArray[index]);

    renderDevice->setTexCoord(0, _texCoord0Array[index+1]);
    renderDevice->setTexCoord(1, _texCoord1Array[index+1]);
    renderDevice->setTexCoord(2, _texCoord2Array[index+1]);
    renderDevice->sendVertex(_posedPointsArray[index+1]);

    renderDevice->setTexCoord(0, _texCoord0Array[index+2]);
    renderDevice->setTexCoord(1, _texCoord1Array[index+2]);
    renderDevice->setTexCoord(2, _texCoord2Array[index+2]);
    renderDevice->sendVertex(_posedPointsArray[index+2]);

    renderDevice->setTexCoord(0, _texCoord0Array[index+3]);
    renderDevice->setTexCoord(1, _texCoord1Array[index+3]);
    renderDevice->setTexCoord(2, _texCoord2Array[index+3]);
    renderDevice->sendVertex(_posedPointsArray[index+3]);
  }
  renderDevice->endPrimitive();
  **/

  renderDevice->popState();
  debugAssertGLOk();
}


} // end namespace

