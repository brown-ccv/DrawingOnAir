
#include "TubeMark.H"
#include <ConfigVal.H>
#include <G3DOperators.h>

#include "Artwork.H"
#include "BrushState.H"
#include "TriStripModel.H"
#include "Shadows.H"
using namespace G3D;
namespace DrawOnAir {

  
TubeMark::TubeMark(const std::string &name, 
                   GfxMgrRef gfxMgr,
                   TriStripModelRef triStripModel) :
  Mark(name, true, false, gfxMgr)
{
  _triStripModel = triStripModel;
  _highlighted = false;
  _hidden = false;
  _brushTipTex = _gfxMgr->getTexture("BrushTipsTile"); 
  _patternTex = _gfxMgr->getTexture("PatternsTile");
  _lastGoodUpVec = Vector3(0,0,1);
  _lastGoodRightVec = Vector3(1,0,0);
  _lastRight = Vector3(1,0,0);

  _nfaces = MinVR::ConfigVal("MarkForm_TubeFaces", 6, false);
  _verts.resize(_nfaces);
  _norms.resize(_nfaces);
  _maskTexCoords.resize(_nfaces);
  _colTexCoords.resize(_nfaces);
  _highTexCoords.resize(_nfaces);
  _patTexCoords.resize(_nfaces);
  for (int i=0;i<_nfaces;i++) {
    _modelID.append(-1);
  }
}

TubeMark::~TubeMark()
{
}

MarkRef
TubeMark::copy()
{
  TubeMarkRef m = new TubeMark(_name, _gfxMgr, _triStripModel);
  m->_samplePositions = _samplePositions;
  m->_sampleIsASuperSample = _sampleIsASuperSample;
  for (int i=0;i<_brushStates.size();i++) {
    m->_brushStates.append(_brushStates[i]->copy());
  }
  m->_arcLengths = _arcLengths;
  m->_roomToVirtualFrameAtStart = _roomToVirtualFrameAtStart;
  m->_roomToVirtualScaleAtStart = _roomToVirtualScaleAtStart;
  m->_verts = _verts;
  m->_norms = _norms;
  m->_maskTexCoords = _maskTexCoords;
  m->_colTexCoords = _colTexCoords;
  m->_highTexCoords = _highTexCoords;
  m->_patTexCoords = _patTexCoords;
  m->_bbox = _bbox;
  m->_transformHistVSFrame = _transformHistVSFrame;
  m->_transformHistVSScale = _transformHistVSScale;
  m->_transformHistXform = _transformHistXform;

  m->_brushTipTex = _brushTipTex;
  m->_patternTex = _patternTex;

  return m;
}

void
TubeMark::trimEnd(int newEndPt)
{
  if (newEndPt == 0) {
    newEndPt = 1;
  }
  Mark::trimEnd(newEndPt);

  int size = newEndPt + 1;
  int size2 = 2*size;
  for (int i=0;i<_nfaces;i++) {
    _verts[i].resize(size2, true);
    _norms[i].resize(size2, true);
    _maskTexCoords[i].resize(size2, true);
    _colTexCoords[i].resize(size2, true);
    _highTexCoords[i].resize(size2, true);
    _patTexCoords[i].resize(size2, true);
  }

  // recalculate bounding box
  _bbox = AABox(_samplePositions[0], _samplePositions[0]);
  for (int i=1;i<_samplePositions.size();i++) {
    MinVR::growAABox(_bbox, _samplePositions[i]);
  }
}


void
TubeMark::addMarkSpecificSample(BrushStateRef brushState)
{ 
  Vector3 right;
  double width;
  brushState->getDrawParametersBasedOnBrushModel(right, width, _lastGoodUpVec, _lastGoodRightVec, _lastRight);
  _lastRight = right;

  width = _gfxMgr->roomDistanceToVirtualSpace(width);
  double size = _gfxMgr->roomDistanceToVirtualSpace(brushState->size);

  // for charcoal once you pick a right vector, stick with it for the whole mark
  if (brushState->brushModel == BrushState::BRUSHMODEL_CHARCOAL_W_HEURISTIC) {
    double dummy;
    _brushStates[0]->getDrawParametersBasedOnBrushModel(right, dummy, _lastGoodUpVec, _lastGoodRightVec, _lastRight);
  }
  right = _gfxMgr->roomVectorToVirtualSpace(right).unit();

  int n = getNumSamples();

  CoordinateFrame bframe = _gfxMgr->roomToVirtualSpace(brushState->frameInRoomSpace);
  Vector3 drawingDir = brushState->drawingDir;
  Vector3 up = right.cross(_gfxMgr->roomVectorToVirtualSpace(drawingDir).unit()).unit();

  Vector3 rvec = 0.5*width*right;
  Vector3 nvec = 0.5*width*up;
  Vector3 center = _samplePositions.last();

  Array<Vector3> verts;
  Array<Vector3> norms;
  Array<Vector2> texCoords;
  Array<Vector2> colTexCoords;
  Array<Vector2> patTexCoords;

  bool wrapTwice = false;
  double a = 0;

  int numBrushTips = MinVR::ConfigVal("NumBrushTips", _brushTipTex->width()/_brushTipTex->height(), false);
  double s1 = (double)getInitialBrushState()->brushTip/(double)numBrushTips;
  double s2 = (double)(getInitialBrushState()->brushTip+1)/(double)numBrushTips;

  int numPatterns = MinVR::ConfigVal("NumPatterns", _patternTex->width()/_patternTex->height(), false);
  double ps1 = (double)getInitialBrushState()->pattern/(double)numPatterns;
  double ps2 = (double)(getInitialBrushState()->pattern+1)/(double)numPatterns;

  for (int j=0;j<_nfaces;j++) {
    // store verts
    Vector3 thisVert = center + rvec*cos(a) + nvec*sin(a);
    if (n == 1) {
      verts.append(center);
    }
    else {
      verts.append(thisVert);
    }
    
    // store tex coords
    float u;
    if (wrapTwice) {
      // To wrap twice around the tube
      int halfverts = iRound((float)_nfaces/2.0);
      if (j < halfverts)
        u = (float)j/(float)halfverts;
      else
        u = 1.0 - (float)(j - halfverts)/(float)halfverts;      
    }
    else { 
      // to wrap once around the tube
      u = (float)j/(float)_nfaces;
    }
    double uMask = s1 + u*(s2-s1);
    double uPat = ps1 + u*(ps2-ps1);

    float repeat = MinVR::ConfigVal("TubeTextureRepeat", size, false);
    float v = getArcLength()/repeat;

    texCoords.append(Vector2(uMask, v));
    patTexCoords.append(Vector2(uPat, v));

    colTexCoords.append(Vector2(brushState->colorSwatchCoord, brushState->colorValue));
    
    // store normals
    norms.append((thisVert - center).unit());

    a -= twoPi() / (double)_nfaces;
  }
  verts.append(verts[0]);

  if (wrapTwice) {
    texCoords.append(texCoords[0]);
    patTexCoords.append(patTexCoords[0]);
  }
  else {
    texCoords.append(Vector2(s1 + 1.0*(s2-s1), texCoords[0][1]));
    patTexCoords.append(Vector2(ps1 + 1.0*(ps2-ps1), patTexCoords[0][1]));
  }
  colTexCoords.append(Vector2(brushState->colorSwatchCoord, brushState->colorValue));
  norms.append(norms[0]);

  double highColSwatch = MinVR::ConfigVal("HighlightColorSwatch", 1.0, false);

  // add this geometry to the triangle strips
  for (int i=0;i<_nfaces;i++) {
    _verts[i].append(verts[i]);
    _verts[i].append(verts[i+1]);
    _norms[i].append(norms[i]);
    _norms[i].append(norms[i+1]);
    _maskTexCoords[i].append(texCoords[i]);
    _maskTexCoords[i].append(texCoords[i+1]);
    _colTexCoords[i].append(colTexCoords[i]);
    _colTexCoords[i].append(colTexCoords[i+1]);
    _highTexCoords[i].append(Vector2(highColSwatch, 0));
    _highTexCoords[i].append(Vector2(highColSwatch, 0));
    _patTexCoords[i].append(patTexCoords[i]);
    _patTexCoords[i].append(patTexCoords[i+1]);
  }

  // if texture is stretched rather than stamped, then readjust
  // texture coords based on new length
  if (brushState->textureApp == BrushState::TEXAPP_STRETCH) {
    for (int f=0;f<_nfaces;f++) {
      for (int s=0;s<_samplePositions.size();s++) {
        int v = 2*s;
        double coord=0.0;
        double l = getArcLength();
        if (l != 0.0) {
          coord = getArcLength(0, s) / l;
        }
        _maskTexCoords[f][v][1] = coord;
        _maskTexCoords[f][v+1][1] = coord;
        _patTexCoords[f][v][1] = coord;
        _patTexCoords[f][v+1][1] = coord;
      }
    }
  }

}


void
TubeMark::updateTriStripModel()
{
  for (int i=0;i<_nfaces;i++) {
    if (_modelID[i] == -1) {
      // brushtip 0 forms a solid tube, so 1 sided rendering is ok,
      // all other brushtips act as alpha masks, so 2 sided rendering is needed
      bool twoSided =  (getInitialBrushState()->brushTip != 0);
      _modelID[i] = _triStripModel->addModel(_verts[i], _norms[i], _maskTexCoords[i], _patTexCoords[i], _colTexCoords[i], twoSided, getInitialBrushState()->layerIndex, getInitialBrushState()->frameIndex);
    }
    else {
      _triStripModel->updateModel(_modelID[i], _verts[i], _norms[i], _maskTexCoords[i], _patTexCoords[i], _colTexCoords[i]);
    }
  }
}

void
TubeMark::capEnd()
{
  // change the last ring of vertices to go to a point to produce a cap on the end
  for (int i=0;i<_nfaces;i++) {
    if ((_verts.size()) && (_verts[i].size() >= 2)) {
      _verts[i][_verts[i].size()-1] = _samplePositions.last();
      _verts[i][_verts[i].size()-2] = _samplePositions.last();
    }
  }
}

void
TubeMark::commitGeometry(ArtworkRef artwork)
{
  if (!_hidden) {
    capEnd();
    artwork->removeFromMarksToDraw(this);
    if (_samplePositions.size() >= 2) {
      updateTriStripModel();
    }
  }
}


void 
TubeMark::pose(Array<PosedModel::Ref>& posedModels, const CoordinateFrame& frame)
{
}

void 
TubeMark::draw(RenderDevice *rd, const CoordinateFrame& frame)
{
  if (_samplePositions.size() >= 2) {

    rd->pushState();
    rd->setObjectToWorldMatrix(frame);
    rd->setAlphaTest(RenderDevice::ALPHA_GREATER, 0.05);
    rd->setTexture(0, _brushTipTex);  
    rd->setTextureCombineMode(0, RenderDevice::TEX_MODULATE);
    
    int smax = 1;
    if (getShadowsOn()) {
      smax = 2;
    }
    for (int s=0;s<smax;s++) {  
      if (s==0) {
        rd->enableTwoSidedLighting();
        if (getInitialBrushState()->twoSided) {
          rd->setCullFace(RenderDevice::CULL_NONE);
        }
        else {
          rd->setCullFace(RenderDevice::CULL_BACK);
        }
        rd->setColor(Color3::white());
        rd->setShininess(125);           // 0 -> 255
        rd->setSpecularCoefficient(0.1); // 0 -> 1
        rd->setShadeMode(RenderDevice::SHADE_SMOOTH);

        rd->setTexture(1, _patternTex);  
        rd->setTextureCombineMode(1, RenderDevice::TEX_MODULATE);
        if (GLCaps::numTextureUnits() > 2) {
          rd->setTexture(2, _gfxMgr->getTexture("ColorSwatchTable"));
          rd->setTextureCombineMode(2, RenderDevice::TEX_MODULATE);   
        }
      }
      else {
        pushShadowState(rd);
      }
      
      // for each face of the tube
      for (int f=0;f<_verts.size();f++) {
        rd->beginPrimitive(PrimitiveType::TRIANGLE_STRIP);
        for (int i=0;i<_verts[f].size();i++) {
          rd->setTexCoord(0, _maskTexCoords[f][i]);
          if (s==0) {
            rd->setNormal(_norms[f][i]);
            rd->setTexCoord(1, _patTexCoords[f][i]);
            if (GLCaps::numTextureUnits() > 2) {
              if (_highlighted) {
                rd->setTexCoord(2, _highTexCoords[f][i]);
              }
              else {
                rd->setTexCoord(2, _colTexCoords[f][i]);
              }
            }
          }
          rd->sendVertex(_verts[f][i]);
        }
        rd->endPrimitive();
      }

      if (s==1) {
        popShadowState(rd);
      }
    }
    rd->popState(); 
  }
}


void 
TubeMark::setHighlighted(bool highlighted, ArtworkRef artwork)
{
  if (highlighted != _highlighted) {
    _highlighted = highlighted;
   
    if (highlighted) {
      if (_modelID[0] != -1) {
        for (int i=0;i<_modelID.size();i++) {
          _triStripModel->hideModel(_modelID[i]);
        }
        artwork->addToMarksToDraw(this);
      }
    }
    else {
      artwork->removeFromMarksToDraw(this);
      if (_modelID[0] == -1) {
        commitGeometry(artwork);
      }
      else {
        for (int i=0;i<_modelID.size();i++) {
          _triStripModel->showModel(_modelID[i]);
        }
      }
    }
  }
}

void 
TubeMark::show()
{
  _hidden = false;
  if (_modelID[0] != -1) {
    for (int i=0;i<_modelID.size();i++) {
      _triStripModel->showModel(_modelID[i]);
    }
  }
}

void 
TubeMark::hide()
{
  _hidden = true;
  if (_modelID[0] != -1) {
    for (int i=0;i<_modelID.size();i++) {
      _triStripModel->hideModel(_modelID[i]);
    }
  }
}

void
TubeMark::stopDrawing()
{
  show();
  if (_modelID[0] != -1) {
    for (int i=0;i<_modelID.size();i++) {
      _triStripModel->deleteModel(_modelID[i]);
      _modelID[i] = -1;
    }
  }
}

void
TubeMark::startEditing(ArtworkRef artwork)
{
  if (_modelID[0] != -1) {
    // show it before deleting in case it was hidden from being highlighted
    for (int i=0;i<_modelID.size();i++) {
      _triStripModel->showModel(_modelID[i]);
      _triStripModel->deleteModel(_modelID[i]);
      _modelID[i] = -1;
    }
    if (!_highlighted) {
      artwork->addToMarksToDraw(this);
    }
  }
}

void
TubeMark::stopEditing(ArtworkRef artwork)
{
  if (!_highlighted) {
    commitGeometry(artwork);
  }
}


void
TubeMark::transformBy(CoordinateFrame frame)
{
  saveTransformEvent(frame);

  // transform and rebuild bounding box too.
  for (int i=0;i<_samplePositions.size();i++) {
    _samplePositions[i] = frame.pointToWorldSpace(_samplePositions[i]);
    if (i == 0) {
      _bbox = AABox(_samplePositions[i]);
    }
    else {
      MinVR::growAABox(_bbox, _samplePositions[i]);
    }
  }

  for (int f=0;f<_nfaces;f++) {
    for (int i=0;i<_verts[f].size();i++) {
      _verts[f][i] = frame.pointToWorldSpace(_verts[f][i]);
    }
    
    for (int i=0;i<_norms[f].size();i++) {
      _norms[f][i] = frame.normalToWorldSpace(_norms[f][i]);
    }
  }
}



} // end namespace

