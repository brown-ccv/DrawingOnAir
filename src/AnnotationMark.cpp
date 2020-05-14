#include <ConfigVal.H>
#include <G3DOperators.h>
#include "AnnotationMark.H"

#include "Artwork.H"
#include "BrushState.H"
#include "TriStripModel.H"

using namespace G3D;

namespace DrawOnAir {

  
AnnotationMark::AnnotationMark(const std::string &name,
                               MinVR::GfxMgrRef gfxMgr,
                               AnnotationModelRef annotationModel) :
  Mark(name, false, false, gfxMgr) 
{
  _annotationModel = annotationModel;
  _gfxMgr = gfxMgr;
  _modelID = -1;
  _highlighted = false;
  _color = MinVR::ConfigVal("Annotation_Color",Color3::black(),false);
}

AnnotationMark::~AnnotationMark()
{
}

MarkRef
AnnotationMark::copy()
{
  AnnotationMarkRef m = new AnnotationMark(_name + "*", _gfxMgr, _annotationModel);
  m->_samplePositions = _samplePositions;
  m->_sampleIsASuperSample = _sampleIsASuperSample;
  for (int i=0;i<_brushStates.size();i++) {
    m->_brushStates.append(_brushStates[i]->copy());
  }
  m->_arcLengths = _arcLengths;
  m->_roomToVirtualFrameAtStart = _roomToVirtualFrameAtStart;
  m->_roomToVirtualScaleAtStart = _roomToVirtualScaleAtStart;
  m->_bbox = _bbox;
  m->_transformHistVSFrame = _transformHistVSFrame;
  m->_transformHistVSScale = _transformHistVSScale;
  m->_transformHistXform = _transformHistXform;

  return m;
}

void
AnnotationMark::addMarkSpecificSample(BrushStateRef brushState)
{
  if (getNumSamples() == 1) {
    // first sample.. add an annotation
    _modelID = _annotationModel->addAnnotation(_name, _samplePositions.last(), _samplePositions.last(),
                                               _color, brushState->layerIndex, brushState->frameIndex);
  }
  else {
    // update based on last sample
    _annotationModel->editAnnotation(_modelID, _name, _samplePositions[0], _samplePositions.last(), _color);
  }
}

void 
AnnotationMark::pose(Array<PosedModel::Ref>& posedModels, const CoordinateFrame& frame)
{
}

void 
AnnotationMark::draw(RenderDevice *rd, const CoordinateFrame& frame)
{
}

void 
AnnotationMark::setHighlighted(bool highlighted, ArtworkRef artwork)
{
  Color3 c = _color;
  if (highlighted) {
    c = Color3::red();
  }
  _annotationModel->editAnnotation(_modelID, _name, _samplePositions[0], _samplePositions.last(),c);
}

void 
AnnotationMark::show()
{
}

void 
AnnotationMark::hide()
{
}

void
AnnotationMark::stopDrawing()
{
  if (_modelID != -1) {
    _annotationModel->deleteAnnotation(_modelID);
    _modelID = -1;
  }
}

void
AnnotationMark::setName(const std::string &name)
{
  _name = name;

  if (_modelID != -1) {
    _annotationModel->editAnnotation(_modelID, _name, _samplePositions[0], _samplePositions.last(), _color);
  }
}

void
AnnotationMark::transformBy(CoordinateFrame frame)
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

  _annotationModel->editAnnotation(_modelID, _name, _samplePositions[0], _samplePositions.last(), _color);  
}


} // end namespace

