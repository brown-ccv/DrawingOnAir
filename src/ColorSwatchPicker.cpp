
#include <ConfigVal.H>
#include "ColorSwatchPicker.H"
using namespace G3D;
#define NOTHING_HIGHLIGHTED -1


namespace DrawOnAir {


ColorSwatchPicker::ColorSwatchPicker(GfxMgrRef              gfxMgr,
                                     EventMgrRef            eventMgr,
                                     //ForceNetInterface*     forceNetInterface,
                                     HCIMgrRef              hciMgr,
                                     CavePaintingCursorsRef cursors,
                                     BrushRef               brush,
                                     Array<std::string>     btnDownTriggers,
                                     Array<std::string>     trackerTriggers,
                                     Array<std::string>     btnUpTriggers,
                                     Array<std::string>     handBtnTriggers,
                                     CoordinateFrame        initialFrame) : WidgetHCI(hciMgr)
{
  _gfxMgr = gfxMgr;
  _eventMgr = eventMgr;
  //_forceNetInterface = forceNetInterface;
  _cursors = cursors;
  _brush = brush;
  _frame = initialFrame;
  _currentCoord = 0.0;

  _fsa = new Fsa("ColorSwatchPicker");
  _fsa->addState("Start");
  _fsa->addState("Drag");
  _fsa->addArc("TrackerMove", "Start", "Start", trackerTriggers);
  _fsa->addArcCallback("TrackerMove", this, &ColorSwatchPicker::trackerMove);
  _fsa->addArc("ClickOn", "Start", "Drag", btnDownTriggers);
  _fsa->addArcCallback("ClickOn", this, &ColorSwatchPicker::clickOn);
  _fsa->addArc("ClickOff", "Drag", "Start", btnUpTriggers);  
  _fsa->addArcCallback("ClickOff", this, &ColorSwatchPicker::clickOff);
  Array<std::string> triggers;
  triggers.append(handBtnTriggers);
  triggers.append("kbd_SPACE_down");
  _fsa->addArc("NextPalette", "Start", "Start", triggers);
  _fsa->addArcCallback("NextPalette", this, &ColorSwatchPicker::nextPalette);

  _font = _gfxMgr->getDefaultFont();

  // lookup swatch textures
  std::string dataDir = MinVR::ConfigVal("DataDir", "share", false);
  FileSystem::getFiles(dataDir + "/images/color-swatch-table*.jpg", _paletteFiles, true);
  if (MinVR::ConfigValMap::map->containsKey("UserDataDir")) {
    Array<std::string> userFiles;
    std::string userDataDir = MinVR::ConfigVal("UserDataDir", ".", false);
    FileSystem::getFiles(userDataDir + "/color-swatch-table*.jpg", userFiles, true);
    _paletteFiles.append(userFiles);
  }

  for (int i=0;i<_paletteFiles.size();i++) {
    Texture::Settings settings;
    settings.wrapMode = WrapMode::CLAMP;
    settings.interpolateMode = Texture::NEAREST_NO_MIPMAP;
    Texture::Ref t = Texture::fromFile(_paletteFiles[i], TextureFormat::RGB8(), Texture::DIM_2D, settings);
    _swatchTextures.append(t);
  }
  debugAssert(_swatchTextures.size());
  _curPalette = 0;
  _gfxMgr->setTextureEntry("ColorSwatchTable", _swatchTextures[0]);
}

ColorSwatchPicker::~ColorSwatchPicker()
{
  _gfxMgr->removeDrawCallback(_cbid);
}


std::string 
ColorSwatchPicker::getCurrentPaletteName()
{ 
  if (_paletteFiles.size()) {
    return _paletteFiles[_curPalette];
  }
  return "Default";
}
  
bool 
ColorSwatchPicker::setCurrentPaletteByName(const std::string &name) 
{
  int num = _paletteFiles.findIndex(name);
  if (num != -1) {
    _curPalette = num;
    _gfxMgr->setTextureEntry("ColorSwatchTable", _swatchTextures[_curPalette]);
    return true;
  }
  return false;
}


void
ColorSwatchPicker::hide()
{
  _gfxMgr->removeDrawCallback(_cbid);
}

void
ColorSwatchPicker::show()
{
  _cbid = _gfxMgr->addDrawCallback(this, &ColorSwatchPicker::draw); 
}

void
ColorSwatchPicker::activate()
{
  _eventMgr->addFsaRef(_fsa);
  _cursors->setBrushCursor(CavePaintingCursors::POINTER_CURSOR);
  show();
  /*if (_forceNetInterface != NULL) {
    _forceNetInterface->startPlaneEffect();
  }*/
}

void
ColorSwatchPicker::deactivate()
{
  releaseFocusWithHCIMgr();
  _eventMgr->removeFsaRef(_fsa);
  hide();
  /*if (_forceNetInterface != NULL) {
    _forceNetInterface->stopPlaneEffect();
  }*/
}

void
ColorSwatchPicker::trackerMove(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();

  Vector3 c = MinVR::ConfigVal("Picker_Center", Vector3::zero(), false);
  double w = MinVR::ConfigVal("ColorSwatchPicker_HalfWidth", 0.4, false);
  double h = MinVR::ConfigVal("ColorSwatchPicker_HalfHeight", 0.2, false);

  Rect2D r = Rect2D::xyxy(c[0]-w, c[1]-h, c[0]+w, c[1]+h);
  Vector2 p = Vector2(e->getCoordinateFrameData().translation[0],
                      e->getCoordinateFrameData().translation[1]);
  if (r.contains(p)) {
    _currentCoord = (p[0] - r.x0()) / r.width();
  }
}

void
ColorSwatchPicker::clickOn(MinVR::EventRef e)
{
  _brush->state->colorSwatchCoord = _currentCoord;
}

void
ColorSwatchPicker::clickOff(MinVR::EventRef e)
{
  deactivate();
}

void
ColorSwatchPicker::nextPalette(MinVR::EventRef e)
{
  if (_swatchTextures.size() > 1) {
    _curPalette++;
    if (_curPalette >= _swatchTextures.size()) {
      _curPalette = 0;
    }
    _gfxMgr->setTextureEntry("ColorSwatchTable", _swatchTextures[_curPalette]);
  }
}

void
ColorSwatchPicker::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  rd->pushState();

  rd->disableLighting();
  rd->setTexture(0, _gfxMgr->getTexture("ColorSwatchTable"));

  double w = MinVR::ConfigVal("ColorSwatchPicker_HalfWidth", 0.4, false);
  double h = MinVR::ConfigVal("ColorSwatchPicker_HalfHeight", 0.2, false);
  Vector3 c = MinVR::ConfigVal("Picker_Center", Vector3::zero(), false);
  
  rd->beginPrimitive(PrimitiveType::QUADS);
  rd->setTexCoord(0, Vector2(0,0));
  rd->sendVertex(c + Vector3(-w, h, 0));
  rd->setTexCoord(0, Vector2(0,1));
  rd->sendVertex(c + Vector3(-w, -h, 0));
  rd->setTexCoord(0, Vector2(1,1));
  rd->sendVertex(c + Vector3(w, -h, 0));
  rd->setTexCoord(0, Vector2(1,0));
  rd->sendVertex(c + Vector3(w, h, 0));

  double l = 1.25*w;
  double r = 1.5*w;
  
  rd->setTexCoord(0, Vector2(_currentCoord,0));
  rd->sendVertex(c + Vector3(l, h, 0));
  rd->setTexCoord(0, Vector2(_currentCoord,1));
  rd->sendVertex(c + Vector3(l, -h, 0));
  rd->setTexCoord(0, Vector2(_currentCoord,1));
  rd->sendVertex(c + Vector3(r, -h, 0));
  rd->setTexCoord(0, Vector2(_currentCoord,0));
  rd->sendVertex(c + Vector3(r, h, 0));
  rd->endPrimitive();

  _gfxMgr->getDefaultFont()->draw3D(rd, "Press SPACE or Hand Btn to cycle through available palettes.",
    CoordinateFrame(c + Vector3(0.0, -1.25*h, 0)), 0.1*h,
    Color3::white(), Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);

  rd->popState();
}


} // end namespace
