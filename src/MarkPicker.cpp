#include "MarkPicker.H"
#include <ConfigVal.H>
using namespace G3D;
namespace DrawOnAir {

  
MarkPicker::MarkPicker(
  MinVR::GfxMgrRef              gfxMgr,
  MinVR::EventMgrRef            eventMgr,
   // ForceNetInterface*     forceNetInterface,
    HCIMgrRef              hciMgr,
    Array<std::string>     markNames,
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
    MinVR::ConfigVal("MarkPicker_ItemSize", Vector3(0.45, 0.1, 0.03), false),
    MinVR::ConfigVal("Picker_ItemSpacing", Vector2(0.01, 0.01), false),
    markNames.size(),
    2)
{
  _markNames = markNames;
  _font = gfxMgr->getDefaultFont();
}

MarkPicker::~MarkPicker()
{
}

void
MarkPicker::drawItem(int itemNum, const CoordinateFrame &itemFrame, RenderDevice *rd)
{
  rd->pushState();

  _font->draw3D(rd, _markNames[itemNum], itemFrame, _itemSize[1]/4.0, 
      Color3::white(), Color4::clear(),
      GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);  

  rd->popState();
}

void
MarkPicker::selectionMade(int itemNum)
{
  _brush->setNewMarkType(itemNum);
}

} // end namespace

