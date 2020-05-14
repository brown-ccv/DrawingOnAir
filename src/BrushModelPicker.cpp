#include <ConfigVal.H>
#include "BrushModelPicker.H"
using namespace G3D;
namespace DrawOnAir {

  
BrushModelPicker::BrushModelPicker(
    MinVR::GfxMgrRef              gfxMgr,
    MinVR::EventMgrRef            eventMgr,
    //ForceNetInterface*     forceNetInterface,
    HCIMgrRef              hciMgr,
    Array<std::string>     brushModelNames,
    BrushRef               brush,
    CavePaintingCursorsRef cursors,
    Array<std::string>     btnDownEvents,
    Array<std::string>     trackerEvents,
    Array<std::string>     btnUpEvents) :
  PickerWidget(
    gfxMgr,
    eventMgr,
    //forceNetInterface,
    hciMgr,
    brush,
    cursors,
    btnDownEvents,
    trackerEvents,
    btnUpEvents,
    Array<std::string>(),
    false,
    MinVR::ConfigVal("BrushModelPicker_ItemSize", Vector3(0.3, 0.1, 0.03), false),
    MinVR::ConfigVal("Picker_ItemSpacing", Vector2(0.01, 0.01), false),
    brushModelNames.size())
{
  _brushModelNames = brushModelNames;
  _font = gfxMgr->getDefaultFont();
}

BrushModelPicker::~BrushModelPicker()
{
}

void
BrushModelPicker::drawItem(int itemNum, const CoordinateFrame &itemFrame, RenderDevice *rd)
{
  rd->pushState();

  _font->draw3D(rd, _brushModelNames[itemNum], itemFrame, _itemSize[1]/4.0, 
      Color3::white(), Color4::clear(),
      GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);  

  rd->popState();
}

void
BrushModelPicker::selectionMade(int itemNum)
{
  _brush->state->brushModel = (BrushState::BrushModelType)itemNum;
  cout << "new brush model= " << (int)_brush->state->brushModel << endl;
}

} // end namespace

