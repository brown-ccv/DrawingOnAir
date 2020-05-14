#include "BrushTipPicker.H"
#include <ConfigVal.H>
using namespace G3D;
namespace DrawOnAir {

  
BrushTipPicker::BrushTipPicker(MinVR::GfxMgrRef              gfxMgr,
                               MinVR::EventMgrRef            eventMgr,
                               //ForceNetInterface*     forceNetInterface,
                               HCIMgrRef              hciMgr,
                               BrushRef          brush,
                               CavePaintingCursorsRef cursors,
                               Array<std::string>     btnDownEvents,
                               Array<std::string>     trackerEvents,
                               Array<std::string>     btnUpEvents,
                               Array<std::string>     handBtnEvents) :
                  PickerWidget(gfxMgr,
                               eventMgr,
                              // forceNetInterface,
                               hciMgr,
                               brush,
                               cursors,
                               btnDownEvents,
                               trackerEvents,
                               btnUpEvents,
                               handBtnEvents,
                               false,
                    MinVR::ConfigVal("Picker_ItemSize", Vector3(0.1, 0.1, 0.03), false),
                    MinVR::ConfigVal("Picker_ItemSpacing", Vector2(0.01, 0.01), false),
                    MinVR::ConfigVal("NumBrushTips", 1))
{
  // lookup brushtip palettes
  std::string dataDir = MinVR::ConfigVal("DataDir", "share", false);
  FileSystem::getFiles(dataDir + "/images/brushtips*.jpg", _paletteFiles, true);
  if (MinVR::ConfigValMap::map->containsKey("UserDataDir")) {
    Array<std::string> userFiles;
    std::string userDataDir = MinVR::ConfigVal("UserDataDir", ".", false);
    FileSystem::getFiles(userDataDir + "/brushtips*.jpg", userFiles, true);
    _paletteFiles.append(userFiles);
  }

  std::string whiteFile = dataDir + "/images/white-brushtips.jpg";

  for (int i=0;i<_paletteFiles.size();i++) {
    Texture::Settings settings;
    settings.wrapMode = WrapMode::CLAMP;
    settings.interpolateMode = Texture::NEAREST_NO_MIPMAP;
    Texture::Ref t = Texture::fromTwoFiles(whiteFile, _paletteFiles[i], TextureFormat::RGBA8(), Texture::DIM_2D, settings);
    _tipTextures.append(t);
    settings.wrapMode = WrapMode::TILE;
    Texture::Ref ttile = Texture::fromTwoFiles(whiteFile, _paletteFiles[i], TextureFormat::RGBA8(), Texture::DIM_2D, settings);
    _tipTileTextures.append(ttile);
  }
  _curPalette = 0;
  debugAssert(_tipTextures.size());
  _gfxMgr->setTextureEntry("BrushTips", _tipTextures[0]);
  debugAssert(_tipTileTextures.size());
  _gfxMgr->setTextureEntry("BrushTipsTile", _tipTileTextures[0]); 
}

BrushTipPicker::~BrushTipPicker()
{
}




std::string 
BrushTipPicker::getCurrentPaletteName()
{ 
  if (_paletteFiles.size()) {
    return _paletteFiles[_curPalette];
  }
  return "Default";
}
  
bool 
BrushTipPicker::setCurrentPaletteByName(const std::string &name) 
{
  int num = _paletteFiles.findIndex(name);
  if (num != -1) {
    _curPalette = num;
    _gfxMgr->setTextureEntry("BrushTips", _tipTextures[_curPalette]);
    _gfxMgr->setTextureEntry("BrushTipsTile", _tipTileTextures[_curPalette]);
    return true;
  }
  return false;
}

void
BrushTipPicker::drawItem(int itemNum, const CoordinateFrame &itemFrame, RenderDevice *rd)
{
  if (itemNum == 0) {
    CoordinateFrame textFrame = _frame;
    textFrame.translation += (_pickerBBox.low()[1] - 0.25*_itemSize[1])*_frame.rotation.column(1);
    _gfxMgr->getDefaultFont()->draw3D(rd, "Press SPACE or the Hand Btn to cycle through available brush tip masks.",
    textFrame, 0.25*_itemSize[1],
    Color3::white(), Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_TOP);
  }

  rd->pushState();

  rd->setObjectToWorldMatrix(itemFrame);
  rd->setCullFace(RenderDevice::CULL_NONE);
  rd->disableLighting();
  rd->setColor(Color3::white());
  rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
  rd->setAlphaTest(RenderDevice::ALPHA_GREATER, 0.05);

  Texture::Ref tex = _gfxMgr->getTexture("BrushTips");
  rd->setTexture(0, tex);

  double s1 = (double)itemNum*(double)tex->height()/(double)tex->width();
  double s2 = (double)(itemNum+1)*(double)tex->height()/(double)tex->width();
  Vector3 halfSize = _itemSize / 2.0;

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
BrushTipPicker::selectionMade(int itemNum)
{
  _brush->state->brushTip = itemNum;
}

void
BrushTipPicker::spacePressed()
{
  if (_tipTextures.size() > 1) {
    _curPalette++;
    if (_curPalette >= _tipTextures.size()) {
      _curPalette = 0;
    }
    _gfxMgr->setTextureEntry("BrushTips", _tipTextures[_curPalette]);
    _gfxMgr->setTextureEntry("BrushTipsTile", _tipTileTextures[_curPalette]);
  }
}

void
BrushTipPicker::handBtnPressed()
{
  spacePressed();
}

} // end namespace

