
#include "SlideMark.H"
#include <ConfigVal.H>
#include <G3DOperators.h>
#include "Artwork.H"
#include "BrushState.H"
#include "Shadows.H"

using namespace G3D;

namespace DrawOnAir {

  
SlideMark::SlideMark(const std::string &name, GfxMgrRef gfxMgr) :
  Mark(name, true, false, gfxMgr) 
{
  _gfxMgr = gfxMgr;
  _highlighted = false;
  _hidden = false;
}

SlideMark::~SlideMark()
{
}

MarkRef
SlideMark::copy()
{
  SlideMarkRef m = new SlideMark(_name, _gfxMgr);
  m->_samplePositions = _samplePositions;
  m->_sampleIsASuperSample = _sampleIsASuperSample;
  for (int i=0;i<_brushStates.size();i++) {
    m->_brushStates.append(_brushStates[i]->copy());
  }
  m->_arcLengths = _arcLengths;
  m->_roomToVirtualFrameAtStart = _roomToVirtualFrameAtStart;
  m->_roomToVirtualScaleAtStart = _roomToVirtualScaleAtStart;
  m->_bbox = _bbox;
  m->_frame = _frame;
  m->_size = _size;
  m->_transformHistVSFrame = _transformHistVSFrame;
  m->_transformHistVSScale = _transformHistVSScale;
  m->_transformHistXform = _transformHistXform;

  return m;
}

void
SlideMark::addMarkSpecificSample(BrushStateRef brushState)
{
  _frame = _gfxMgr->roomToVirtualSpace(brushState->frameInRoomSpace);
  _size = _gfxMgr->roomVectorToVirtualSpace(Vector3(brushState->size, 0,0)).length();

  Vector3 c = _frame.translation;
  Vector3 x = _frame.rotation.column(0);
  Vector3 y = _frame.rotation.column(1);
  Vector3 z = _frame.rotation.column(2);
  double  s = _size/2.0;
  double sz = MinVR::ConfigVal("Picker_ItemSize", Vector3(0.1, 0.1, 0.03), false)[2];

  MinVR::growAABox(_bbox, c + s*x);
  MinVR::growAABox(_bbox, c - s*x);
  MinVR::growAABox(_bbox, c + s*y);
  MinVR::growAABox(_bbox, c - s*y);
  MinVR::growAABox(_bbox, c + sz*z);
  MinVR::growAABox(_bbox, c + sz*z);
}

void 
SlideMark::pose(Array<PosedModel::Ref>& posedModels, const CoordinateFrame& frame)
{
}

void 
SlideMark::draw(RenderDevice *rd, const CoordinateFrame& frame)
{
  if ((_brushStates.size()) && (!_hidden)) {
    rd->pushState();
    rd->setObjectToWorldMatrix(frame);

    CoordinateFrame f = _frame;
    Vector3 c = f.translation;
    Vector3 x = f.rotation.column(0);
    Vector3 y = f.rotation.column(1);
    Vector3 z = f.rotation.column(2);
    double  s = _size/2.0;
    
    rd->setAlphaTest(RenderDevice::ALPHA_GREATER, 0.05);
    rd->setCullFace(RenderDevice::CULL_NONE);
    Texture::Ref tex = _gfxMgr->getTexture(_name);
    rd->setTexture(0, tex);
    
    int smax = 1;
    if (getShadowsOn()) {
      smax = 2;
    }
    for (int sh=0;sh<smax;sh++) {      
      if (sh==0) {
        rd->disableLighting();
        rd->setColor(Color3::white());
      }
      else {
        pushShadowState(rd);
      }

          
      rd->beginPrimitive(PrimitiveType::QUADS);
      rd->setTexCoord(0, Vector2(0,1));
      rd->sendVertex(c + s*x - s*y);
      rd->setTexCoord(0, Vector2(1,1));
      rd->sendVertex(c - s*x - s*y);
      rd->setTexCoord(0, Vector2(1,0));
      rd->sendVertex(c - s*x + s*y);
      rd->setTexCoord(0, Vector2(0,0));
      rd->sendVertex(c + s*x + s*y);
      rd->endPrimitive();

      if (sh==1) {
        popShadowState(rd);
      }
      
    }
    
    if (_highlighted) {
      rd->setColor(Color3::red());
      rd->setTexture(0, NULL);
      rd->beginPrimitive(PrimitiveType::LINE_STRIP);
      s = s*1.1;
      rd->sendVertex(c + s*x - s*y);
      rd->sendVertex(c - s*x - s*y);
      rd->sendVertex(c - s*x + s*y);
      rd->sendVertex(c + s*x + s*y);
      rd->sendVertex(c + s*x - s*y);
      rd->endPrimitive();
    }

    rd->popState();
  }
}

void 
SlideMark::setHighlighted(bool highlighted, ArtworkRef artwork)
{
  _highlighted = highlighted;
}

void 
SlideMark::show()
{
  _hidden = false;
}

void 
SlideMark::hide()
{
  _hidden = true;
}

void
SlideMark::setName(const std::string &name) 
{
  _name = name;
}


void
SlideMark::transformBy(CoordinateFrame frame)
{
  saveTransformEvent(frame);

  // transform and rebuild bounding box too.
  if (_brushStates.size()) {

    _samplePositions[0] = frame.pointToWorldSpace(_samplePositions[0]);
    _frame = frame * _frame;

    Vector3 c = _frame.translation;
    Vector3 x = _frame.rotation.column(0);
    Vector3 y = _frame.rotation.column(1);
    Vector3 z = _frame.rotation.column(2);
    double  s = _size/2.0;
    double sz = MinVR::ConfigVal("Picker_ItemSize", Vector3(0.1, 0.1, 0.03), false)[2];

    _bbox = AABox(c);
    MinVR::growAABox(_bbox, c + s*x);
    MinVR::growAABox(_bbox, c - s*x);
    MinVR::growAABox(_bbox, c + s*y);
    MinVR::growAABox(_bbox, c - s*y);
    MinVR::growAABox(_bbox, c + sz*z);
    MinVR::growAABox(_bbox, c + sz*z);
  }
}


} // end namespace

