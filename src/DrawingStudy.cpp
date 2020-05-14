#include <ConfigVal.H>
#include <G3DOperators.h>
#include "DrawingStudy.H"


#include "ArtworkIO.H"
using namespace G3D;
namespace DrawOnAir {




DrawingStudy::DrawingStudy(int argc, char **argv) : MinVR::VRG3DBaseApp(argc, argv)
{
  //_forceNetInterface = NULL;

  // Better default lighting setup
  LightingRef lighting = _gfxMgr->getLighting();
  lighting->ambientTop    = MinVR::ConfigVal("AmbientTop", Color3(0.35, 0.35, 0.35));
  lighting->ambientBottom = MinVR::ConfigVal("AmbientBottom", Color3(0.35, 0.35, 0.35));
  lighting->lightArray.clear();
  lighting->lightArray.append(GLight::directional(Vector3(0,1,1), Color3(0.5,0.5,0.5)));
  
  
  _clearColor = MinVR::ConfigVal("CLEAR_COLOR", Color3(0.35, 0.35, 0.35), false);

  _artwork = new Artwork(_gfxMgr);

  _history = new History(_eventMgr, _gfxMgr);
  _brush = new Brush(new BrushState(), _artwork, _gfxMgr, _eventMgr, _history);

  Array<std::string> brushOn     = MinVR::splitStringIntoArray("Brush_Btn_down");
  Array<std::string> brushMotion = MinVR::splitStringIntoArray("Brush_Tracker");
  Array<std::string> brushOff    = MinVR::splitStringIntoArray("Brush_Btn_up");
  Array<std::string> handOn      = MinVR::splitStringIntoArray("Hand_Btn_down");
  Array<std::string> handMotion  = MinVR::splitStringIntoArray("Hand_Tracker");
  Array<std::string> handOff     = MinVR::splitStringIntoArray("Hand_Btn_up");
 
  // Cursors
  _cavePaintingCursors = new CavePaintingCursors(_gfxMgr, _eventMgr, _brush, handMotion);


  _hciMgr = new HCIMgr(brushMotion, _eventMgr);


  _drawingDisabler = new DrawingDisabler(_eventMgr, _hciMgr, _brush, brushMotion);


  // Stylus Interactions
  _directDrawHCI = new DirectDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _eventMgr);
  _hciMgr->activateStylusHCI(_directDrawHCI);
  _dragDrawHCI = new DragDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _eventMgr, _gfxMgr);
  _tapeDrawHCI = new TapeDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _eventMgr, _gfxMgr);
  _reverseTapeDrawHCI = new ReverseTapeDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _eventMgr, _gfxMgr);

  if (MinVR::ConfigVal("UseForceDrawing", false, false)) {
    std::string host = MinVR::ConfigVal("ForceNetHost", "localhost", false);
    //int port = MinVR::ConfigVal("ForceNetPort", FORCENETINTERFACE_DEFAULT_PORT, false);
    //_forceNetInterface = new ForceNetInterface(host, port);
    //_inputDevices.append(_forceNetInterface);
    //_forceHybridDrawHCI = new ForceHybridDrawHCI(brushOn, brushMotion, brushOff, handMotion, handOn, handOff, _brush, _cavePaintingCursors, _forceNetInterface, _eventMgr, _gfxMgr);
    _hciMgr->activateStylusHCI(_forceHybridDrawHCI);
   // _forceTapeDrawHCI = new ForceTapeDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _forceNetInterface, _eventMgr, _gfxMgr);
   // _forceOneHandDrawHCI = new ForceOneHandDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _forceNetInterface, _eventMgr);
   // _forceReverseTapeDrawHCI = new ForceReverseTapeDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _forceNetInterface, _eventMgr);
  }


  // Alignment cues for use in the fishtank
  if (MinVR::ConfigVal("UseFishtankDepthCues", 1, false)) {
    _fishtankDepthCues = new FishtankDepthCues(_gfxMgr);
  }
  


  std::string artworkToLoad = MinVR::decygifyPath(MinVR::replaceEnvVars(MinVR::ConfigVal("LoadArtwork", "", false)));
  if (artworkToLoad != "") {
    cout << "Loading: " << artworkToLoad << endl;
    BinaryInput bi(artworkToLoad, G3D_LITTLE_ENDIAN, false);
    Color3 bgColor;
    ArtworkIO::deserialize(bi, _artwork, bgColor, NULL, NULL, NULL);
    _clearColor = bgColor;
  }
}

DrawingStudy::~DrawingStudy()
{
}

void
DrawingStudy::saveSnapshot(const std::string &filename)
{
  // Since the cluster nodes are on the same file system, just save a
  // snapshot from the master.
  if (MinVR::ConfigValMap::map->containsKey(string("CLUSTER_NODE_ID")) &&
    MinVR::ConfigVal("CLUSTER_NODE_ID",-1) != 0 ) {
    return;
  }

  std::string fname = _gfxMgr->getRenderDevice()->screenshot(filename);
  cout << "Saved snapshot of screen to file: " << fname << endl;
}

void
DrawingStudy::saveArtwork(const std::string &filename)
{
  if (MinVR::ConfigValMap::map->containsKey(string("CLUSTER_NODE_ID")) &&
    MinVR::ConfigVal("CLUSTER_NODE_ID",-1) != 0 ) {
    return;
  }

  cout << "Saving Artwork to file: " << filename << endl;  
  BinaryOutput bo(filename, G3D_LITTLE_ENDIAN);
  ArtworkIO::serialize(bo, _artwork, Color3(_clearColor.r, _clearColor.g, _clearColor.b), NULL, NULL, NULL);
  bo.commit();
}


void
DrawingStudy::doGraphics(RenderDevice *rd) 
{
  _gfxMgr->drawFrame();
  if (_fishtankDepthCues.notNull()) {
    _fishtankDepthCues->draw(rd, CoordinateFrame());
  }
  
  // draw some 2D things here, to make sure it's after everything else
  //_gfxMgr->drawStats();
  //vrg3dSleepMsecs(MinVR::ConfigVal("VRBaseApp_SleepTime", 0.0, false));
}

void 
DrawingStudy::doUserInput(Array<MinVR::EventRef> &events)
{
  //VRBaseApp::doUserInput(events);
}

void
DrawingStudy::disableDrawing()
{
  _drawingDisabler->disableDrawing();
}

void
DrawingStudy::enableDrawing()
{
  _drawingDisabler->enableDrawing();
}

  

} // end namespace



