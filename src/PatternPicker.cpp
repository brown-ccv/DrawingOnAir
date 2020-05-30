#include "PatternPicker.H"
#include <ConfigVal.H>
using namespace G3D;
namespace DrawOnAir {

  
PatternPicker::PatternPicker(GfxMgrRef              gfxMgr,
                             EventMgrRef            eventMgr,
                             //ForceNetInterface*     forceNetInterface,
                             HCIMgrRef              hciMgr,
                             BrushRef               brush,
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
                    MinVR::ConfigVal("NumPatterns", 1))
{
  // lookup pattern palettes
  std::string dataDir = MinVR::ConfigVal("DataDir", "share", false);
  FileSystem::getFiles(dataDir + "/images/patterns*.jpg", _paletteFiles, true);
  if (MinVR::ConfigValMap::map->containsKey("UserDataDir")) {
    Array<std::string> userFiles;
    std::string userDataDir = MinVR::ConfigVal("UserDataDir", ".", false);
    FileSystem::getFiles(userDataDir + "/patterns*.jpg", userFiles, true);
    _paletteFiles.append(userFiles);
  }

  for (int i=0;i<_paletteFiles.size();i++) {
    Texture::Settings settings;
    settings.wrapMode = WrapMode::TILE;
    settings.interpolateMode = Texture::NEAREST_NO_MIPMAP;
    Texture::Ref ttile = Texture::fromFile(_paletteFiles[i], TextureFormat::RGB8(), Texture::DIM_2D, settings);
    _textures.append(ttile);
  }
  _curPalette = 0;
  debugAssert(_textures.size());
  _gfxMgr->setTextureEntry("PatternsTile", _textures[0]); 
}

PatternPicker::~PatternPicker()
{
}

std::string 
PatternPicker::getCurrentPaletteName()
{ 
  if (_paletteFiles.size()) {
    return _paletteFiles[_curPalette];
  }
  return "Default";
}
  
bool 
PatternPicker::setCurrentPaletteByName(const std::string &name) 
{
  int num = _paletteFiles.findIndex(name);
  if (num != -1) {
    _curPalette = num;
    _gfxMgr->setTextureEntry("PatternsTile", _textures[_curPalette]);
    return true;
  }
  return false;
}

  
void
PatternPicker::drawItem(int itemNum, const CoordinateFrame &itemFrame, RenderDevice *rd)
{
  if (itemNum == 0) {
    CoordinateFrame textFrame = _frame;
    textFrame.translation += (_pickerBBox.low()[1] - 0.25*_itemSize[1])*_frame.rotation.column(1);
    _gfxMgr->getDefaultFont()->draw3D(rd, "Press SPACE or the Hand Btn to cycle through available patterns.",
    textFrame, 0.25*_itemSize[1],
    Color3::white(), Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_TOP);
  }

  rd->pushState();

  rd->setBlendFunc(RenderDevice::BLEND_ONE, RenderDevice::BLEND_ZERO);
  rd->setObjectToWorldMatrix(itemFrame);
  rd->setCullFace(RenderDevice::CULL_NONE);
  rd->disableLighting();
  rd->setColor(Color3::white());

  Texture::Ref tex = _gfxMgr->getTexture("PatternsTile");
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
PatternPicker::selectionMade(int itemNum)
{
  _brush->state->pattern = itemNum;
  // set brush color to white
}

void
PatternPicker::spacePressed()
{
  if (_textures.size() > 1) {
    _curPalette++;
    if (_curPalette >= _textures.size()) {
      _curPalette = 0;
    }
    _gfxMgr->setTextureEntry("PatternsTile", _textures[_curPalette]);
  }
}

void
PatternPicker::handBtnPressed()
{
  spacePressed();
}

} // end namespace

