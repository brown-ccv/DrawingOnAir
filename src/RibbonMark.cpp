
#include "RibbonMark.H"

#include <G3DOperators.h>
#include <ConfigVal.H>

#include "Artwork.H"
#include "BrushState.H"
#include "TriStripModel.H"
#include "Shadows.H"
using namespace G3D;
namespace DrawOnAir {

  
RibbonMark::RibbonMark(const std::string &name, 
  GfxMgrRef gfxMgr,
                       TriStripModelRef triStripModel) :
  Mark(name, true, false, gfxMgr) 
{
  _gfxMgr = gfxMgr;
  _triStripModel = triStripModel;
  _highlighted = false;
  _hidden = false;
  _modelID = -1;
  _lastGoodUpVec = Vector3(0,0,1);
  _lastRight = Vector3(1,0,0);
  _lastGoodRightVec = Vector3(1,0,0);
  _brushTipTex = _gfxMgr->getTexture("BrushTipsTile"); 
  _patternTex = _gfxMgr->getTexture("PatternsTile"); 
}

RibbonMark::~RibbonMark()
{
}

MarkRef
RibbonMark::copy()
{
  RibbonMarkRef m = new RibbonMark(_name, _gfxMgr, _triStripModel);
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
RibbonMark::trimEnd(int newEndPt)
{
  if (newEndPt == 0) {
    newEndPt = 1;
  }
  Mark::trimEnd(newEndPt);

  int size = newEndPt + 1;
  int size2 = size*2;
  _verts.resize(size2, true);
  _norms.resize(size2, true);
  _maskTexCoords.resize(size2, true);
  _colTexCoords.resize(size2, true);
  _highTexCoords.resize(size2, true);
  _patTexCoords.resize(size2, true);

  // recalculate bounding box
  _bbox = AABox(_samplePositions[0], _samplePositions[0]);
  for (int i=1;i<_samplePositions.size();i++) {
    MinVR::growAABox(_bbox, _samplePositions[i]);
  }
}


void
RibbonMark::addMarkSpecificSample(BrushStateRef brushState)
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
  

  double highColSwatch = MinVR::ConfigVal("HighlightColorSwatch", 1.0, false);
  CoordinateFrame bframe = _gfxMgr->roomToVirtualSpace(brushState->frameInRoomSpace);
  int n = getNumSamples();

  Vector3 norm;
  if (n == 1) {
    norm = bframe.rotation.column(2);
  }
  else {
    // Go back and "fix" the normal for the last sample now that we know the proper
    // tangent for the Mark's path
    int sThis = n-1;
    int sLast = n-2;
    int sLast2 = n-3;
    if (sLast2 < 0) {
      sLast2 = sLast;
    }

    Vector3 lastTan = (_samplePositions[sThis] - _samplePositions[sLast2]).unit();
    Vector3 lastRight = (_verts.last() - _samplePositions[sLast]).unit();
    norm = lastRight.cross(lastTan).unit();
    _norms[_norms.size()-1] = norm;
    _norms[_norms.size()-2] = norm;

    // flip right to be consistent with last one if necessary
    if (right.dot(lastRight) < 0.0) {
      right = -right;
    }
  }

  

  int numBrushTips = MinVR::ConfigVal("NumBrushTips", _brushTipTex->width()/_brushTipTex->height(), false);  
  double s1 = (double)getInitialBrushState()->brushTip/(double)numBrushTips;
  double s2 = (double)(getInitialBrushState()->brushTip+1)/(double)numBrushTips;

  int numPatterns = MinVR::ConfigVal("NumPatterns", _patternTex->width()/_patternTex->height(), false);
  double ps1 = (double)getInitialBrushState()->pattern/(double)numPatterns;
  double ps2 = (double)(getInitialBrushState()->pattern+1)/(double)numPatterns;


  Vector3 pl = _samplePositions.last() - 0.5 * width * right;
  _verts.append(pl);
  _norms.append(norm);
  _colTexCoords.append(Vector2(brushState->colorSwatchCoord, brushState->colorValue));
  _highTexCoords.append(Vector2(highColSwatch, 0.0));
  _maskTexCoords.append(Vector2(s1, getArcLength()/size));
  _patTexCoords.append(Vector2(ps1, getArcLength()/size));

  Vector3 pr = _samplePositions.last() + 0.5 * width * right;
  _verts.append(pr);
  _norms.append(norm);
  _colTexCoords.append(Vector2(brushState->colorSwatchCoord, brushState->colorValue));
  _highTexCoords.append(Vector2(highColSwatch, 0.0));
  _maskTexCoords.append(Vector2(s2, getArcLength()/size));
  _patTexCoords.append(Vector2(ps2, getArcLength()/size));

  if (brushState->textureApp == BrushState::TEXAPP_STRETCH) {
    for (int s=0;s<_samplePositions.size();s++) {
      int v = 2*s;
      double l = getArcLength();
      double coord = 0.0;
      if (l != 0.0) {
        coord = getArcLength(0, s) / l;
      }
      _maskTexCoords[v][1] = coord;
      _maskTexCoords[v+1][1] = coord;
      _patTexCoords[v][1] = coord;
      _patTexCoords[v+1][1] = coord;
    }
  }
}

void
RibbonMark::updateTriStripModel()
{
  if (_modelID == -1) {
    _modelID = _triStripModel->addModel(_verts, _norms, _maskTexCoords, _patTexCoords, _colTexCoords, getInitialBrushState()->twoSided, getInitialBrushState()->layerIndex, getInitialBrushState()->frameIndex);
  }
  else {
    _triStripModel->updateModel(_modelID, _verts, _norms, _maskTexCoords, _patTexCoords, _colTexCoords);
  }  
}

void
RibbonMark::commitGeometry(ArtworkRef artwork)
{
  if (!_hidden) {
    artwork->removeFromMarksToDraw(this);
    if (_samplePositions.size() >= 2) {
      updateTriStripModel();
    }
  }
}


void 
RibbonMark::pose(Array<PosedModel::Ref>& posedModels, const CoordinateFrame& frame)
{
}

void 
RibbonMark::draw(RenderDevice *rd, const CoordinateFrame& frame)
{
  if (_verts.size() > 2) {
    rd->pushState();
    rd->setObjectToWorldMatrix(frame);

    int smax = 1;
    if (getShadowsOn()) {
      smax = 2;
    }
    for (int s=0;s<smax;s++) {

      rd->setAlphaTest(RenderDevice::ALPHA_GREATER, 0.05);
      rd->setTexture(0, _brushTipTex);  
      rd->setTextureCombineMode(0, RenderDevice::TEX_MODULATE);

      if (s==0) {
        if (getInitialBrushState()->twoSided) {
          rd->setCullFace(RenderDevice::CULL_NONE);
        }
        else {
          rd->setCullFace(RenderDevice::CULL_BACK);
        }
        rd->enableTwoSidedLighting();
        rd->setColor(Color3::white());
        rd->setShininess(125);           // 0 -> 255
        rd->setSpecularCoefficient(0.1); // 0 -> 1
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

 
      rd->beginPrimitive(PrimitiveType::TRIANGLE_STRIP);
      for (int i=0;i<_verts.size();i++) {
        rd->setNormal(_norms[i]);
        rd->setTexCoord(0, _maskTexCoords[i]);
        if (s==0) {
          rd->setTexCoord(1, _patTexCoords[i]);
          if (GLCaps::numTextureUnits() > 2) {
            if (_highlighted) {
              rd->setTexCoord(2, _highTexCoords[i]);
            }
            else {
              rd->setTexCoord(2, _colTexCoords[i]);
            }
          }
        }
        rd->sendVertex(_verts[i]);
      }
      rd->endPrimitive();

      if (s==1) {
        popShadowState(rd);
      }
    }
    rd->popState();
  }
}

void 
RibbonMark::setHighlighted(bool highlighted, ArtworkRef artwork)
{
  /** Approach to highlighting:
      - the highlighted version of the mark is always drawn in the
        draw function of this class if _highlighted is true
      - temporarily hide the model with tristripmodel and add the mark to
        the artwork's marks to draw so the draw function gets called
  **/

  if (highlighted != _highlighted) {
    _highlighted = highlighted;
   
    if (highlighted) {
      if (_modelID != -1) {
        _triStripModel->hideModel(_modelID);
        artwork->addToMarksToDraw(this);
      }
    }
    else {
      artwork->removeFromMarksToDraw(this);
      if (_modelID == -1) {
        commitGeometry(artwork);
      }
      else {
        _triStripModel->showModel(_modelID);
      }
    }
  }
}

void 
RibbonMark::show()
{
  _hidden = false;
  if (_modelID != -1) {
    _triStripModel->showModel(_modelID);
  }
}

void 
RibbonMark::hide()
{
  _hidden = true;
  if (_modelID != -1) {
    _triStripModel->hideModel(_modelID);
  }
}

void
RibbonMark::stopDrawing()
{
  show();
  _triStripModel->deleteModel(_modelID);
  _modelID = -1;
}

void
RibbonMark::startEditing(ArtworkRef artwork)
{
  if (_modelID != -1) {
    // show it before deleting in case it was hidden from being highlighted
    _triStripModel->showModel(_modelID);
    _triStripModel->deleteModel(_modelID);
    _modelID = -1;
    if (!_highlighted) {
      artwork->addToMarksToDraw(this);
    }
  }
}

void
RibbonMark::stopEditing(ArtworkRef artwork)
{
  if (!_highlighted) {
    commitGeometry(artwork);
  }
}


void
RibbonMark::transformBy(CoordinateFrame frame)
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

  for (int i=0;i<_verts.size();i++) {
    _verts[i] = frame.pointToWorldSpace(_verts[i]);
  }

  for (int i=0;i<_norms.size();i++) {
    _norms[i] = frame.normalToWorldSpace(_norms[i]);
  }

}

} // end namespace

