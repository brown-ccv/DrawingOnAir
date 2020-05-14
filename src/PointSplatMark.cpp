
#include <ConfigVal.H>
#include "PointSplatMark.H"

#include "Artwork.H"
#include "BrushState.H"
using namespace G3D;

namespace DrawOnAir {

  
  PointSplatMark::PointSplatMark(const std::string &name, MinVR::GfxMgrRef gfxMgr, PointModelRef pointModel) : 
  Mark(name, false, true, gfxMgr)
{
  _pointModel = pointModel;
  _modelID = -1;
  _highlighted = false;
}

PointSplatMark::~PointSplatMark()
{
}

MarkRef
PointSplatMark::copy()
{
  PointSplatMarkRef newPSM = new PointSplatMark(_name, _gfxMgr, _pointModel);
  newPSM->_sampleWeights = _sampleWeights;
  return newPSM;
}

void
PointSplatMark::trimEnd(int newEndPt)
{
  if (newEndPt == 0) {
    newEndPt = 1;
  }
  Mark::trimEnd(newEndPt);

  _sampleWeights.resize(newEndPt + 1);
}

void
PointSplatMark::addMarkSpecificSample(BrushStateRef brushState)
{
  // weights should range from 0 -> 1
  _sampleWeights.append(brushState->width / brushState->size);
}

void 
PointSplatMark::pose(Array<PosedModel::Ref>& posedModels, const CoordinateFrame& frame)
{
  if (_poseDirty) {
    if (_modelID == -1) {
      _modelID = _pointModel->addModel(_samplePositions, _sampleWeights, 
                                       getInitialBrushState()->colorSwatchCoord, 
                                       getInitialBrushState()->size, 
                                       getInitialBrushState()->brushTip);
    }
    else if (_highlighted) {
      double colSwatch = MinVR::ConfigVal("HighlightColorSwatch", 1.0, false);
      _pointModel->updateModel(_modelID, _samplePositions, _sampleWeights, colSwatch,
                                       getInitialBrushState()->size, 
                                       getInitialBrushState()->brushTip);
    }
    else {
      _pointModel->updateModel(_modelID, _samplePositions, _sampleWeights,
                               getInitialBrushState()->colorSwatchCoord, 
                               getInitialBrushState()->size, 
                               getInitialBrushState()->brushTip);
    }
  }
}

void 
PointSplatMark::draw(RenderDevice *rd, const CoordinateFrame& frame)
{
}

void 
PointSplatMark::setHighlighted(bool highlighted)
{
  _highlighted = highlighted;
  _poseDirty = true;
}

void 
PointSplatMark::show()
{
  _poseDirty = true;
}

void 
PointSplatMark::hide()
{
  if (_modelID != -1) {
    _pointModel->deleteModel(_modelID);
    _modelID = -1;
  }
  _poseDirty = false;
}

} // end namespace

