
#include <ConfigVal.H>
#include "BrushSizePicker.H"
using namespace G3D;
namespace DrawOnAir {

  
BrushSizePicker::BrushSizePicker(MinVR::GfxMgrRef              gfxMgr,
                                 MinVR::EventMgrRef            eventMgr,
                                 //ForceNetInterface*     forceNetInterface,
                                 HCIMgrRef              hciMgr,
                                 BrushRef               brush,
                                 CavePaintingCursorsRef cursors,
                                 Array<std::string>     btnDownEvents,
                                 Array<std::string>     trackerEvents,
                                 Array<std::string>     btnUpEvents,
                                 double                 minSize,
                                 double                 maxSize,
                                 int                    numChoices) :
                  PickerWidget(gfxMgr,
                               eventMgr,
                             //  forceNetInterface,
                               hciMgr,
                               brush,
                               cursors,
                               btnDownEvents,
                               trackerEvents,
                               btnUpEvents,
                               Array<std::string>(),
                               false,
                               Vector3(1.1*maxSize, 1.1*maxSize, 0.3*maxSize),
                               MinVR::ConfigVal("Picker_ItemSpacing", Vector2(0.01, 0.01), false),
                               numChoices)
{
  _minSize = minSize;
  _maxSize = maxSize;
}

BrushSizePicker::~BrushSizePicker()
{
}

void
BrushSizePicker::drawItem(int itemNum, const CoordinateFrame &itemFrame, RenderDevice *rd)
{
  rd->pushState();

  rd->setObjectToWorldMatrix(itemFrame);
  rd->setCullFace(RenderDevice::CULL_NONE);
  rd->disableLighting();
  rd->setColor(Color3::white());
  rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);

  Texture::Ref tex = _gfxMgr->getTexture("BrushTips");
  rd->setTexture(0, tex);

  int tip = _brush->state->brushTip;
  double s1 = (double)tip*(double)tex->height()/(double)tex->width();
  double s2 = (double)(tip+1)*(double)tex->height()/(double)tex->width();

  double brushSize = ((double)itemNum/(double)(_numItems-1))*(_maxSize - _minSize) + _minSize;
  Vector3 halfSize = Vector3(brushSize/2.0, brushSize/2.0, 0.0);
  debugAssert(halfSize[0] <= _itemSize[0]/2.0);
  debugAssert(halfSize[1] <= _itemSize[1]/2.0);

  rd->beginPrimitive(PrimitiveType::QUADS);
  rd->setTexCoord(0, Vector2(s1,0));
  rd->sendVertex(Vector3(-halfSize[0],  halfSize[1], 0));
  rd->setTexCoord(0, Vector2(s1,1));
  rd->sendVertex(Vector3(-halfSize[0], -halfSize[1], 0));
  rd->setTexCoord(0, Vector2(s2,1));
  rd->sendVertex(Vector3( halfSize[0], -halfSize[1], 0));
  rd->setTexCoord(0, Vector2(s2,0));
  rd->sendVertex(Vector3( halfSize[0],  halfSize[1], 0));
  rd->endPrimitive();

  rd->popState();
}

void
BrushSizePicker::selectionMade(int itemNum)
{
  double brushSize = ((double)itemNum/(double)(_numItems-1))*(_maxSize - _minSize) + _minSize;
  _brush->state->size = brushSize;
  // Change width too here.  If the width is linked to a pressure change, then it will just overwrite
  // this when the pressure changes.  If not, then we want it to be set to the size we just selected.
  _brush->state->width = brushSize;
}

} // end namespace

