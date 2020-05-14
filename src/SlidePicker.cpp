#include <ConfigVal.H>

#include "SlidePicker.H"
using namespace G3D;
namespace DrawOnAir {

  
SlidePicker::SlidePicker(MinVR::GfxMgrRef              gfxMgr,
  MinVR::EventMgrRef            eventMgr,
                         //ForceNetInterface*     forceNetInterface,
                         HCIMgrRef              hciMgr,
                         BrushRef               brush,
                         CavePaintingCursorsRef cursors,
                         Array<std::string>     btnDownEvents,
                         Array<std::string>     trackerEvents,
                         Array<std::string>     btnUpEvents,
                         PlaceSlideHCIRef       placeSlideHCI) :
  PickerWidget(gfxMgr,
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
    MinVR::ConfigVal("Picker_ItemSize", Vector3(0.1, 0.1, 0.03), false),
    MinVR::ConfigVal("Picker_ItemSpacing", Vector2(0.01, 0.01), false),
               1)
{
  _placeSlideHCI = placeSlideHCI;

  loadSlideImages(_gfxMgr, _keyNames);

  _numItems = _keyNames.size();
  _numCols = PickerWidget::AUTO_LAYOUT;
  generateLayout();
}

void
SlidePicker::loadSlideImages(MinVR::GfxMgrRef gfxMgr, Array<std::string> &keyNames)
{
  // lookup slides
  Array<std::string> files;
  std::string dataDir = MinVR::ConfigVal("DataDir", "share", false);
  FileSystem::getFiles(dataDir + "/images/slide*.jpg", files, true);
  if (MinVR::ConfigValMap::map->containsKey("UserDataDir")) {
    Array<std::string> userFiles;
    std::string userDataDir = MinVR::ConfigVal("UserDataDir", ".", false);
    FileSystem::getFiles(userDataDir + "/slide*.jpg", userFiles, true);
    files.append(userFiles);
  }

  for (int i=0;i<files.size();i++) {
    std::string basename = filenameBaseExt(files[i]);
    std::string basenoslide = basename.substr(5);
    std::string path = filenamePath(files[i]);
    std::string alphafile = path + "alphamask" + basenoslide;

    Texture::Settings settings;
    Texture::Ref tex;
    if (FileSystem::exists(alphafile)) {
      tex = Texture::fromTwoFiles(files[i], alphafile, TextureFormat::RGBA8(), 
                                  Texture::DIM_2D, settings);
      alwaysAssertM(tex.notNull(), "Problem loading texture " + files[i] + 
                    " with alpha " + alphafile);
      cout << "Loaded slide with alpha: " << files[i] << ", " << alphafile << endl;
    }
    else {
      tex = Texture::fromFile(files[i], TextureFormat::RGB8(), Texture::DIM_2D, settings);
      alwaysAssertM(tex.notNull(), "Problem loading texture " + files[i]);
      cout << "Loaded slide: " << files[i] << endl;
    }
    gfxMgr->setTextureEntry(basename, tex); 
    keyNames.append(basename); 
  }
}

SlidePicker::~SlidePicker()
{
}

  
void
SlidePicker::drawItem(int itemNum, const CoordinateFrame &itemFrame, RenderDevice *rd)
{
  if (itemNum == 0) {
    _gfxMgr->getDefaultFont()->draw3D(rd, "Pick slide, then place in scene with brush.",
    CoordinateFrame(Vector3(0.0, -0.25, 0.0)), 0.02,
    Color3::white(), Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
  }

  rd->pushState();

  rd->setObjectToWorldMatrix(itemFrame);
  rd->setCullFace(RenderDevice::CULL_NONE);
  rd->disableLighting();
  rd->setColor(Color3::white());
  rd->setAlphaTest(RenderDevice::ALPHA_GREATER, 0.05);

  Texture::Ref tex = _gfxMgr->getTexture(_keyNames[itemNum]);
  rd->setTexture(0, tex);

  Vector3 halfSize = _itemSize / 2.0;

  rd->beginPrimitive(PrimitiveType::QUADS);
  rd->setTexCoord(0, Vector2(0,0));
  rd->sendVertex(Vector3(-halfSize[0],  halfSize[1], 0));
  rd->setTexCoord(0, Vector2(0,1));
  rd->sendVertex(Vector3(-halfSize[0], -halfSize[1], 0));
  rd->setTexCoord(0, Vector2(1,1));
  rd->sendVertex(Vector3( halfSize[0], -halfSize[1], 0));
  rd->setTexCoord(0, Vector2(1,0));
  rd->sendVertex(Vector3( halfSize[0],  halfSize[1], 0));
  rd->endPrimitive();

  rd->popState();
}


void
SlidePicker::startWidget()
{
  _selectionMade = false;
}

void
SlidePicker::selectionMade(int itemNum)
{
  _selectionMade = true;
  _placeSlideHCI->setSlideName(_keyNames[itemNum]);
}

void
SlidePicker::closeWidget()
{
  if (_selectionMade) {
    _hciMgr->activateStylusHCI(_placeSlideHCI);
  }
}

} // end namespace

