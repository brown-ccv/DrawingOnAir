#include "LoadFilePicker.H"
#include <ConfigVal.H>
using namespace G3D;
namespace DrawOnAir {

  
LoadFilePicker::LoadFilePicker(
  GfxMgrRef              gfxMgr,
  EventMgrRef            eventMgr,
    //ForceNetInterface*     forceNetInterface,
    HCIMgrRef              hciMgr,
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
    MinVR::ConfigVal("LoadFilePicker_ItemSize", Vector3(0.45, 0.1, 0.03), false),
    MinVR::ConfigVal("Picker_ItemSpacing", Vector2(0.01, 0.01), false),
    1)
{
  _font = gfxMgr->getDefaultFont();
}

LoadFilePicker::~LoadFilePicker()
{
}


void
LoadFilePicker::startWidget()
{
  _fileNames.clear();
  _baseNames.clear();
  std::string dir = MinVR::ConfigVal("LoadBasePath", "share/demo-artwork", false);
  FileSystem::getFiles(dir + "/*.3DArt", _fileNames, true);
  for (int i=0;i<_fileNames.size();i++) {
    _baseNames.append(filenameBaseExt(_fileNames[i]));
  }

  _numItems = _fileNames.size();
  generateLayout();
}

void
LoadFilePicker::drawItem(int itemNum, const CoordinateFrame &itemFrame, RenderDevice *rd)
{
  rd->pushState();

  _font->draw3D(rd, _baseNames[itemNum], itemFrame, _itemSize[1]/4.0, 
      Color3::white(), Color4::clear(),
      GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);  

  rd->popState();
}

void
LoadFilePicker::selectionMade(int itemNum)
{
  // responded to by main cavepainting class
  _eventMgr->queueEvent(new MinVR::Event("LoadFile", _fileNames[itemNum]));
}

} // end namespace

