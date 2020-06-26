#include "CavePainting.H"

#include <ConfigVal.H>
#include <G3DOperators.h>
#include <ViewerHCI.H>
#include <G3DOperators.h>
#include "MarkingMenu.H"
#include "ArtworkIO.H"
#include <Shadows.H>

#include "SplineFit.H"
#include "VRML2Parser.H"
#include <G3DOperators.h>

using namespace G3D;
// This default worked well with the Phantom Premium 1.0 at Brown
//#define DEFAULT_VISCOUS_GAIN 0.8

// This works better with the Phantom Premium 1.5 at UofM
#define DEFAULT_VISCOUS_GAIN 0.4

namespace DrawOnAir {


CavePainting::CavePainting(int argc, char **argv) : VRG3DBaseApp(argc, argv)
{
  //_forceNetInterface = NULL;
  _frictionOn = true;
  _pressureBtnPressed = false;
  _drawFloor = MinVR::ConfigVal("DrawFloor", 1, false);
  setShadowsOn(_drawFloor);
  _lastFilename = MinVR::ConfigVal("SaveFileName", "MyDrawing", false);
  if (!endsWith(_lastFilename, ".3DArt")) {
    _lastFilename = _lastFilename + ".3DArt";
  }


  // Better default lighting setup
  LightingRef lighting = _gfxMgr->getLighting();
  lighting->ambientTop    = MinVR::ConfigVal("AmbientTop", Color3(0.35, 0.35, 0.35));
  lighting->ambientBottom = MinVR::ConfigVal("AmbientBottom", Color3(0.35, 0.35, 0.35));
  lighting->lightArray.clear();
  lighting->lightArray.append(GLight::directional(Vector3(0,1,1), Color3(0.5,0.5,0.5)));
  
  

  _clearColor = MinVR::ConfigVal("CLEAR_COLOR", Color3(0.35, 0.35, 0.35), false);

  _artwork = new Artwork(_gfxMgr);

  /* Keefe 11/15/12: This adds a dependency on $G/src/mmotion and we rarely need this functionality,
     so commenting out for now.

  std::string markersFile = ConfigVal("BatMarkersFile", "", false);
  std::string markersMotionFile = ConfigVal("MarkersMotionFile", "", false);
  if (markersFile != "") {
    debugAssert(FileSystem::exists(markersFile));
    _markers = new Markers(_gfxMgr);
    double size = ConfigVal("BatMarkersSize", 0.005, false);
    double s = ConfigVal("BatMarkersScale", 1.0, false);
    CoordinateFrame f = ConfigVal("BatMarkersFrame", CoordinateFrame(), false);
    int skip = ConfigVal("BatMarkersTimestepIncrement", 10, false);
    _markers->addMarkersFromBatDataFile(markersFile, Color3::white(), size, s, f, skip);
    _artwork->setNumFrames(_markers->getNumMarkerSets());
  }
  else if (markersMotionFile != "") {
    debugAssert(FileSystem::exists(markersMotionFile));
    _markers = new Markers(_gfxMgr);
    double size = ConfigVal("MarkersMotionSize", 0.005, false);
    double s = ConfigVal("MarkersMotionScale", 1.0, false);
    CoordinateFrame f = ConfigVal("MarkersMotionFrame", CoordinateFrame(), false);
    int skip = ConfigVal("MarkersMotionTimestepIncrement", 10, false);
    _markers->addMarkersFromMarkersMotionFile(markersMotionFile, Color3::white(), size, s, f, skip);
    _artwork->setNumFrames(_markers->getNumMarkerSets());
  }
  */

  _history = new History(_eventMgr, _gfxMgr);
  _brush = new Brush(new BrushState(), _artwork, _gfxMgr, _eventMgr, _history);
  _brush->state->colorSwatchCoord = MinVR::ConfigVal("InitialBrushColorSwatchCoord", 0.3, false);
  _brush->state->size = MinVR::ConfigVal("InitialBrushSize", 0.03, false);
  _brush->state->width = MinVR::ConfigVal("InitialBrushSize", 0.03, false);

  // Alignment cues for use in the fishtank
  if (MinVR::ConfigVal("UseFishtankDepthCues", 1, false)) {
    _fishtankDepthCues = new FishtankDepthCues(_gfxMgr);
  }


  bool interactive = !MinVR::ConfigVal("Viewer", 0, false);
  if (interactive) {

    //new DefaultMouseNav(_eventMgr, _gfxMgr);



    Array<std::string> brushOn     = MinVR::splitStringIntoArray("Brush_Btn_down");
    Array<std::string> brushMotion = MinVR::splitStringIntoArray("Brush_Tracker");
    Array<std::string> brushOff    = MinVR::splitStringIntoArray("Brush_Btn_up");
    Array<std::string> handOn      = MinVR::splitStringIntoArray("Hand_Btn_down");
    Array<std::string> handMotion  = MinVR::splitStringIntoArray("Hand_Tracker");
    Array<std::string> handOff     = MinVR::splitStringIntoArray("Hand_Btn_up");
 
    // Cursors
    _cavePaintingCursors = new CavePaintingCursors(_gfxMgr, _eventMgr, _brush, handMotion);


    _hciMgr = new HCIMgr(brushMotion, _eventMgr);

    // Stylus Interactions
    _blendHCI = new BlendHCI(brushOn, brushMotion, brushOff, _brush, _cavePaintingCursors, _eventMgr, _gfxMgr);
    _deleteHCI = new DeleteHCI(brushMotion, brushOn, brushOff, _artwork, _brush, _cavePaintingCursors, _eventMgr, _gfxMgr, _history);
    _copyToNextFrameHCI = new CopyToNextFrameHCI(brushMotion, brushOn, brushOff, _artwork, _brush, _cavePaintingCursors, _eventMgr, _gfxMgr, _history);
    _placeSlideHCI = new PlaceSlideHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _eventMgr, _gfxMgr);
    _moveMarksHCI = new MoveMarksHCI(brushMotion, brushOn, brushOff, _artwork, _brush, _cavePaintingCursors, _eventMgr, _gfxMgr, _history);
    _directDrawHCI = new DirectDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _eventMgr);
    _dragDrawHCI = new DragDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _eventMgr, _gfxMgr);
    _hybridDrawHCI = new HybridDrawHCI(brushOn, brushMotion, brushOff, handMotion, handOn, handOff, _brush, _cavePaintingCursors, _eventMgr, _gfxMgr);
    _tapeDrawHCI = new TapeDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _eventMgr, _gfxMgr);
    _reverseTapeDrawHCI = new ReverseTapeDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _eventMgr, _gfxMgr);
    _hciMgr->activateStylusHCI(_hybridDrawHCI);

    if (MinVR::ConfigVal("UseForceDrawing", false, false)) {
      std::string host = MinVR::ConfigVal("ForceNetHost", "localhost", false);
      //int port = MinVR::ConfigVal("ForceNetPort", FORCENETINTERFACE_DEFAULT_PORT, false);
      //_forceNetInterface = new ForceNetInterface(host, port);
      //_inputDevices.append(_forceNetInterface);
      //_forceTapeDrawHCI = new ForceTapeDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _forceNetInterface, _eventMgr, _gfxMgr);
      //_forceOneHandDrawHCI = new ForceOneHandDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _forceNetInterface, _eventMgr);
      //_forceReverseTapeDrawHCI = new ForceReverseTapeDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _forceNetInterface, _eventMgr);
      //_forceMovingPlaneDrawHCI = new ForceMovingPlaneDrawHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _forceNetInterface, _eventMgr, _gfxMgr);
      //_forceHybridDrawHCI = new ForceHybridDrawHCI(brushOn, brushMotion, brushOff, handMotion, handOn, handOff, _brush, _cavePaintingCursors, _forceNetInterface, _eventMgr, _gfxMgr);
      //_hciMgr->activateStylusHCI(_forceHybridDrawHCI);
    }

    //_forceBlendHCI = new ForceBlendHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _forceNetInterface, _eventMgr, _gfxMgr);
    _forceBlendHCI = new ForceBlendHCI(brushOn, brushMotion, brushOff, handMotion, _brush, _cavePaintingCursors, _eventMgr, _gfxMgr);

    // Widget Interactions
    Array<std::string> bModelNames = MinVR::splitStringIntoArray(MinVR::ConfigVal("BrushModelNames", ""));
    //_brushModelPicker  = new BrushModelPicker(_gfxMgr, _eventMgr, _forceNetInterface, _hciMgr, bModelNames, _brush, _cavePaintingCursors, brushOn, brushMotion, brushOff);
    //_brushSizePicker   = new BrushSizePicker(_gfxMgr, _eventMgr, _forceNetInterface, _hciMgr, _brush, _cavePaintingCursors, brushOn, brushMotion, brushOff, ConfigVal("MinBrushWidth", 0.01), ConfigVal("MaxBrushWidth", 0.1), 25);
    //_brushTipPicker    = new BrushTipPicker(_gfxMgr, _eventMgr, _forceNetInterface, _hciMgr, _brush, _cavePaintingCursors, brushOn, brushMotion, brushOff, handOn);
    //_colorPicker       = new ColorPicker(_gfxMgr, _eventMgr, _hciMgr, _cavePaintingCursors, brushMotion, brushOn);
    //_colorSwatchPicker = new ColorSwatchPicker(_gfxMgr, _eventMgr, _forceNetInterface, _hciMgr, _cavePaintingCursors, _brush, brushOn, brushMotion, brushOff, handOn, CoordinateFrame());
    //_loadFilePicker    = new LoadFilePicker(_gfxMgr, _eventMgr, _forceNetInterface, _hciMgr, _brush, _cavePaintingCursors, brushOn, brushMotion, brushOff);
    Array<std::string> markNames = MinVR::splitStringIntoArray(MinVR::ConfigVal("MarkNames", ""));
    //_markPicker        = new MarkPicker(_gfxMgr, _eventMgr, _forceNetInterface, _hciMgr, markNames, _brush, _cavePaintingCursors, brushOn, brushMotion, brushOff);
    //_patternPicker     = new PatternPicker(_gfxMgr, _eventMgr, _forceNetInterface, _hciMgr, _brush, _cavePaintingCursors, brushOn, brushMotion, brushOff, handOn);
    //_slidePicker       = new SlidePicker(_gfxMgr, _eventMgr, _forceNetInterface, _hciMgr, _brush, _cavePaintingCursors, brushOn, brushMotion, brushOff, _placeSlideHCI);
    //_reframeArtworkHCI = new ReframeArtworkHCI(_eventMgr, _gfxMgr, _hciMgr, _cavePaintingCursors, handMotion, handOff, brushOn, brushMotion, brushOff);
    //_kbdSelectWidget   = new KbdSelectWidget(_artwork, _eventMgr, _hciMgr);  
   // _layerWidget       = new LayerWidget(_artwork, _gfxMgr, _eventMgr, _forceNetInterface, _hciMgr, _brush, _cavePaintingCursors, brushOn, brushMotion, brushOff);
    //_hciMgr->addPointerActivatedWidget(_layerWidget);
    //_frameWidget       = new FrameWidget(_artwork, _gfxMgr, _eventMgr, _forceNetInterface, _hciMgr, _brush, _cavePaintingCursors, brushOn, brushMotion, brushOff);
    //_hciMgr->addPointerActivatedWidget(_frameWidget);
   // _textInputWidget   = new TextInputWidget(_gfxMgr, _eventMgr, _hciMgr, brushOn);
    //_brush->setTextInputWidget(_textInputWidget);

    _modeMenu = new MarkingMenu(
        _gfxMgr, _eventMgr, _hciMgr, 
        _cavePaintingCursors, _brush,
        //_forceNetInterface,
        brushOn, brushMotion, brushOff, 
      MinVR::splitStringIntoArray(MinVR::ConfigVal("ModeMenu_Images","")),
      MinVR::splitStringIntoArray(MinVR::ConfigVal("ModeMenu_HelpNames","")),
      MinVR::splitStringIntoArray(MinVR::ConfigVal("ModeMenu_EventNames","")),
        CoordinateFrame(MinVR::ConfigVal("ModeMenu_Pos", Vector3::zero())));
    _hciMgr->addPointerActivatedWidget(_modeMenu);

    _brushMenu = new MarkingMenu(
        _gfxMgr, _eventMgr, _hciMgr, 
        _cavePaintingCursors, _brush,
        //_forceNetInterface,
        brushOn, brushMotion, brushOff, 
      MinVR::splitStringIntoArray(MinVR::ConfigVal("BrushMenu_Images","")),
      MinVR::splitStringIntoArray(MinVR::ConfigVal("BrushMenu_HelpNames","")),
      MinVR::splitStringIntoArray(MinVR::ConfigVal("BrushMenu_EventNames","")),
        CoordinateFrame(MinVR::ConfigVal("BrushMenu_Pos", Vector3::zero())));
    _hciMgr->addPointerActivatedWidget(_brushMenu);


    _artworkMenu = new MarkingMenu(
        _gfxMgr, _eventMgr, _hciMgr, 
        _cavePaintingCursors, _brush,
        //_forceNetInterface,
        brushOn, brushMotion, brushOff, 
      MinVR::splitStringIntoArray(MinVR::ConfigVal("ArtworkMenu_Images","")),
      MinVR::splitStringIntoArray(MinVR::ConfigVal("ArtworkMenu_HelpNames","")),
      MinVR::splitStringIntoArray(MinVR::ConfigVal("ArtworkMenu_EventNames","")),
        CoordinateFrame(MinVR::ConfigVal("ArtworkMenu_Pos", Vector3::zero())));
    _hciMgr->addPointerActivatedWidget(_artworkMenu);

    _oopsMenu = new MarkingMenu(
        _gfxMgr, _eventMgr, _hciMgr, 
        _cavePaintingCursors, _brush,
        //_forceNetInterface,
        brushOn, brushMotion, brushOff, 
      MinVR::splitStringIntoArray(MinVR::ConfigVal("OopsMenu_Images","")),
      MinVR::splitStringIntoArray(MinVR::ConfigVal("OopsMenu_HelpNames","")),
      MinVR::splitStringIntoArray(MinVR::ConfigVal("OopsMenu_EventNames","")),
        CoordinateFrame(MinVR::ConfigVal("OopsMenu_Pos", Vector3::zero())));
    _hciMgr->addPointerActivatedWidget(_oopsMenu);

    _systemMenu = new MarkingMenu(
        _gfxMgr, _eventMgr, _hciMgr, 
        _cavePaintingCursors, _brush,
        //_forceNetInterface,
        brushOn, brushMotion, brushOff, 
      MinVR::splitStringIntoArray(MinVR::ConfigVal("SystemMenu_Images","")),
      MinVR::splitStringIntoArray(MinVR::ConfigVal("SystemMenu_HelpNames","")),
      MinVR::splitStringIntoArray(MinVR::ConfigVal("SystemMenu_EventNames","")),
        CoordinateFrame(MinVR::ConfigVal("SystemMenu_Pos", Vector3::zero())));
    _hciMgr->addPointerActivatedWidget(_systemMenu);

        

    // Indication of running out of working space, important for the phantom
    Vector3 low = MinVR::ConfigVal("WorkingLimits_Low", Vector3(-0.4, -0.4, -0.4), false);
    Vector3 high = MinVR::ConfigVal("WorkingLimits_High", Vector3(0.4, 0.4, 0.4), false);
    double dist = MinVR::ConfigVal("WorkingLimits_WarnDist", 0.1, false);
    _workingLimits = new WorkingLimits(AABox(low, high), dist, _brush->state, _gfxMgr);



    // Main CavePainting FSA
    FsaRef fsa = new Fsa("CavePainting");
    fsa->addState("Start");
    fsa->addState("FilenameTextEnter");

    // Activated by the Left Hand Button
    fsa->addArc("Reframe", "Start", "Start", handOn);
    fsa->addArcCallback("Reframe", this, &CavePainting::activateReframeArtwork);

    // Activated from the Keyboard
    fsa->addArc("KbdSel", "Start", "Start", MinVR::splitStringIntoArray("kbd_J_down"));
    fsa->addArcCallback("KbdSel", this, &CavePainting::activateKbdSelectWidget);
    fsa->addArc("ToggleFriction", "Start", "Start", MinVR::splitStringIntoArray("ToggleFriction kbd_F_down"));
    fsa->addArcCallback("ToggleFriction", this, &CavePainting::toggleFrictionAndViscosity);
    fsa->addArc("PointSplatMarkType", "Start", "Start", MinVR::splitStringIntoArray("PointSplatMarkType"));
    fsa->addArcCallback("PointSplatMarkType", this, &CavePainting::setPointSplatMarkType);
    fsa->addArc("RibbonMarkType", "Start", "Start", MinVR::splitStringIntoArray("RibbonMarkType"));
    fsa->addArcCallback("RibbonMarkType", this, &CavePainting::setRibbonMarkType);
    fsa->addArc("KbdPressureUp", "Start", "Start", MinVR::splitStringIntoArray("kbd_P_down"));
    fsa->addArcCallback("KbdPressureUp", this, &CavePainting::kbdPressureUp);
    fsa->addArc("KbdPressureDown", "Start", "Start", MinVR::splitStringIntoArray("kbd_O_down"));
    fsa->addArcCallback("KbdPressureDown", this, &CavePainting::kbdPressureDown);
    fsa->addArc("Shutdown", "Start", "Start", MinVR::splitStringIntoArray("kbd_ESC_down"));
    fsa->addArcCallback("Shutdown", this, &CavePainting::shutdown);


    fsa->addArc("ShowMenus", "Start", "Start", MinVR::splitStringIntoArray("kbd_M_down"));
    fsa->addArcCallback("ShowMenus", this, &CavePainting::toggleShowMenus);
    fsa->addArc("ShowShadows", "Start", "Start", MinVR::splitStringIntoArray("kbd_A_down"));
    fsa->addArcCallback("ShowShadows", this, &CavePainting::toggleShowShadows);

    // From the Brush Menu
    fsa->addArc("BrushModel", "Start", "Start", MinVR::splitStringIntoArray("BrushModelPicker_On"));
    fsa->addArcCallback("BrushModel", this, &CavePainting::activateBrushModelPicker);
    fsa->addArc("BrushTip", "Start", "Start", MinVR::splitStringIntoArray("BrushTipPicker_On"));
    fsa->addArcCallback("BrushTip", this, &CavePainting::activateBrushTipPicker);
    fsa->addArc("BrushSize", "Start", "Start", MinVR::splitStringIntoArray("BrushSizePicker_On"));
    fsa->addArcCallback("BrushSize", this, &CavePainting::activateBrushSizePicker);
    fsa->addArc("ColorSwatch", "Start", "Start", MinVR::splitStringIntoArray("ColorSwatch_On"));
    fsa->addArcCallback("ColorSwatch", this, &CavePainting::activateColorSwatchPicker);
    fsa->addArc("MarkPick", "Start", "Start", MinVR::splitStringIntoArray("MarkPicker_On"));
    fsa->addArcCallback("MarkPick", this, &CavePainting::activateMarkPicker);
    fsa->addArc("PatternPick", "Start", "Start", MinVR::splitStringIntoArray("PatternPicker_On"));
    fsa->addArcCallback("PatternPick", this, &CavePainting::activatePatternPicker);
    fsa->addArc("SlidePick", "Start", "Start", MinVR::splitStringIntoArray("SlidePicker_On"));
    fsa->addArcCallback("SlidePick", this, &CavePainting::activateSlidePicker);
    fsa->addArc("LayerWidget", "Start", "Start", MinVR::splitStringIntoArray("LayerWidget_On"));
    fsa->addArcCallback("LayerWidget", this, &CavePainting::activateLayerWidget);
    fsa->addArc("FrameWidget", "Start", "Start", MinVR::splitStringIntoArray("FrameWidget_On"));
    fsa->addArcCallback("FrameWidget", this, &CavePainting::activateFrameWidget);
    fsa->addArc("HandOffset", "Start", "Start", MinVR::splitStringIntoArray("ToggleHandOffset"));
    fsa->addArcCallback("HandOffset", this, &CavePainting::toggleHandOffset);

    // From the HCI Menu
    fsa->addArc("HybridDrawing", "Start", "Start", MinVR::splitStringIntoArray("HybridDrawingMode"));
    fsa->addArcCallback("HybridDrawing", this, &CavePainting::activateHybridDrawing);
    fsa->addArc("TapeDrawing", "Start", "Start", MinVR::splitStringIntoArray("TapeDrawingMode"));
    fsa->addArcCallback("TapeDrawing", this, &CavePainting::activateTapeDrawing);
    fsa->addArc("FreehandMode", "Start", "Start", MinVR::splitStringIntoArray("FreehandDrawingMode"));
    fsa->addArcCallback("FreehandMode", this, &CavePainting::activateFreehandDrawing);
    fsa->addArc("FreehandNoFrictionMode", "Start", "Start", MinVR::splitStringIntoArray("FreehandNoFrictionDrawingMode"));
    fsa->addArcCallback("FreehandNoFrictionMode", this, &CavePainting::activateFreehandNoFrictionDrawing);


    fsa->addArc("ForceOneHandDrawing", "Start", "Start", MinVR::splitStringIntoArray("ForceOneHandDrawingMode"));
    fsa->addArcCallback("ForceOneHandDrawing", this, &CavePainting::activateForceOneHandDrawing);
    fsa->addArc("Blend", "Start", "Start", MinVR::splitStringIntoArray("BlendMode"));
    fsa->addArcCallback("Blend", this, &CavePainting::activateBlending);
    fsa->addArc("ForceBlend", "Start", "Start", MinVR::splitStringIntoArray("ForceBlendMode"));
    fsa->addArcCallback("ForceBlend", this, &CavePainting::activateForceBlend);
    
    // From the Oops Menu
    fsa->addArc("Undo", "Start", "Start", MinVR::splitStringIntoArray("Undo"));
    fsa->addArcCallback("Undo", this, &CavePainting::undo);
    fsa->addArc("Redo", "Start", "Start", MinVR::splitStringIntoArray("Redo"));
    fsa->addArcCallback("Redo", this, &CavePainting::redo);
    fsa->addArc("Delete", "Start", "Start", MinVR::splitStringIntoArray("DeleteMode"));
    fsa->addArcCallback("Delete", this, &CavePainting::activateDeleteMode);
    fsa->addArc("CopyToNext", "Start", "Start", MinVR::splitStringIntoArray("CopyToNextFrameMode"));
    fsa->addArcCallback("CopyToNext", this, &CavePainting::activateCopyToNextFrame);
    fsa->addArc("MoveMarks", "Start", "Start", MinVR::splitStringIntoArray("MoveMarksMode"));
    fsa->addArcCallback("MoveMarks", this, &CavePainting::activateMoveMarks);


    // From the System Menu
    fsa->addArc("SaveSnapshot", "Start", "Start", MinVR::splitStringIntoArray("SaveSnapshot kbd_S_down"));
    fsa->addArcCallback("SaveSnapshot", this, &CavePainting::saveSnapshot);
    fsa->addArc("SaveArtwork", "Start", "FilenameTextEnter", MinVR::splitStringIntoArray("SaveArtwork"));
    fsa->addArcCallback("SaveArtwork", this, &CavePainting::startFilenameInputForSaveArtwork);
    fsa->addArc("DoneTextEnter", "FilenameTextEnter", "Start", MinVR::splitStringIntoArray("TextInputWidget_InputDone"));
    fsa->addArcCallback("DoneTextEnter", this, &CavePainting::saveArtwork);
    
    fsa->addArc("ShowFilesToLoad", "Start", "Start", MinVR::splitStringIntoArray("LoadArtwork"));
    fsa->addArcCallback("ShowFilesToLoad", this, &CavePainting::activateLoadFilePicker);
    fsa->addArc("LoadFile", "Start", "Start", MinVR::splitStringIntoArray("LoadFile"));
    fsa->addArcCallback("LoadFile", this, &CavePainting::loadArtwork);

    // From the Lighting Menu
    fsa->addArc("AmbientTop", "Start", "Start", MinVR::splitStringIntoArray("SelectAmbientTop"));
    fsa->addArcCallback("AmbientTop", this, &CavePainting::selectAmbientTopLightColor);
    fsa->addArc("AmbientBot", "Start", "Start", MinVR::splitStringIntoArray("SelectAmbientBottom"));
    fsa->addArcCallback("AmbientBot", this, &CavePainting::selectAmbientBottomLightColor);
    fsa->addArc("BGCol", "Start", "Start", MinVR::splitStringIntoArray("SelectBackgroundColor"));
    fsa->addArcCallback("BGCol", this, &CavePainting::selectBackgroundColor);

    // For advancing movie frames
    fsa->addArc("NextFrame", "Start", "Start", MinVR::splitStringIntoArray("kbd_RIGHT_down"));
    fsa->addArcCallback("NextFrame", this, &CavePainting::nextFrame);
    fsa->addArc("PrevFrame", "Start", "Start", MinVR::splitStringIntoArray("kbd_LEFT_down"));
    fsa->addArcCallback("PrevFrame", this, &CavePainting::previousFrame);

    // Kbd shortcuts for toggling display of layers
    fsa->addArc("TogLay0", "Start", "Start", MinVR::splitStringIntoArray("kbd_0_down"));
    fsa->addArcCallback("TogLay0", this, &CavePainting::toggleLayer0);
    fsa->addArc("TogLay1", "Start", "Start", MinVR::splitStringIntoArray("kbd_1_down"));
    fsa->addArcCallback("TogLay1", this, &CavePainting::toggleLayer1);
    fsa->addArc("TogLay2", "Start", "Start", MinVR::splitStringIntoArray("kbd_2_down"));
    fsa->addArcCallback("TogLay2", this, &CavePainting::toggleLayer2);
    fsa->addArc("TogLay3", "Start", "Start", MinVR::splitStringIntoArray("kbd_3_down"));
    fsa->addArcCallback("TogLay3", this, &CavePainting::toggleLayer3);
    fsa->addArc("TogLay4", "Start", "Start", MinVR::splitStringIntoArray("kbd_4_down"));
    fsa->addArcCallback("TogLay4", this, &CavePainting::toggleLayer4);
    fsa->addArc("TogLay5", "Start", "Start", MinVR::splitStringIntoArray("kbd_5_down"));
    fsa->addArcCallback("TogLay5", this, &CavePainting::toggleLayer5);
    fsa->addArc("TogLay6", "Start", "Start", MinVR::splitStringIntoArray("kbd_6_down"));
    fsa->addArcCallback("TogLay6", this, &CavePainting::toggleLayer6);
    fsa->addArc("TogLay7", "Start", "Start", MinVR::splitStringIntoArray("kbd_7_down"));
    fsa->addArcCallback("TogLay7", this, &CavePainting::toggleLayer7);
    fsa->addArc("TogLay8", "Start", "Start", MinVR::splitStringIntoArray("kbd_8_down"));
    fsa->addArcCallback("TogLay8", this, &CavePainting::toggleLayer8);
    fsa->addArc("TogLay9", "Start", "Start", MinVR::splitStringIntoArray("kbd_9_down"));
    fsa->addArcCallback("TogLay9", this, &CavePainting::toggleLayer9);
    fsa->addArc("TogLay10", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_0_down"));
    fsa->addArcCallback("TogLay10", this, &CavePainting::toggleLayer10);
    fsa->addArc("TogLay11", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_1_down"));
    fsa->addArcCallback("TogLay11", this, &CavePainting::toggleLayer11);
    fsa->addArc("TogLay12", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_2_down"));
    fsa->addArcCallback("TogLay12", this, &CavePainting::toggleLayer12);
    fsa->addArc("TogLay13", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_3_down"));
    fsa->addArcCallback("TogLay13", this, &CavePainting::toggleLayer13);
    fsa->addArc("TogLay14", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_4_down"));
    fsa->addArcCallback("TogLay14", this, &CavePainting::toggleLayer14);
    fsa->addArc("TogLay15", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_5_down"));
    fsa->addArcCallback("TogLay15", this, &CavePainting::toggleLayer15);
    fsa->addArc("TogLay16", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_6_down"));
    fsa->addArcCallback("TogLay16", this, &CavePainting::toggleLayer16);
    fsa->addArc("TogLay17", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_7_down"));
    fsa->addArcCallback("TogLay17", this, &CavePainting::toggleLayer17);
    fsa->addArc("TogLay18", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_8_down"));
    fsa->addArcCallback("TogLay18", this, &CavePainting::toggleLayer18);
    fsa->addArc("TogLay19", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_9_down"));
    fsa->addArcCallback("TogLay19", this, &CavePainting::toggleLayer19);

    fsa->addArc("TogDynDrag", "Start", "Start", MinVR::splitStringIntoArray("kbd_D_down"));
    fsa->addArcCallback("TogDynDrag", this, &CavePainting::toggleDynamicDragging);
    fsa->addArc("TogCDLD", "Start", "Start", MinVR::splitStringIntoArray("kbd_L_down"));
    fsa->addArcCallback("TogCDLD", this, &CavePainting::toggleConstantDragLengthDisplay);

  


    // This update should only be used in the Cave where you don't
    // have a forceserver but you do have a pressure sensative brush
    // switch
    fsa->addArc("PressureDeviceUpdate", "Start", "Start", 
      MinVR::splitStringIntoArray(MinVR::ConfigVal("PressureDeviceEvent", "NOP", false)));
    fsa->addArcCallback("PressureDeviceUpdate", this, &CavePainting::brushPressureDeviceUpdate);


    _eventMgr->addFsaRef(fsa);


    // Fsa's added to respond to the color picker for lighting changes
    _ambientTopFsa = new Fsa("AmbientTop");
    _ambientTopFsa->addState("Start");
    _ambientTopFsa->addArc("Change", "Start", "Start", MinVR::splitStringIntoArray("ColorPicker_ColorModified"));
    _ambientTopFsa->addArcCallback("Change", this, &CavePainting::ambientTopLightColorChange);
    _ambientTopFsa->addArc("Set", "Start", "Start", MinVR::splitStringIntoArray("ColorPicker_ColorSelected"));
    _ambientTopFsa->addArcCallback("Set", this, &CavePainting::ambientTopLightColorSet);
    _ambientTopFsa->addArc("Cancel", "Start", "Start", MinVR::splitStringIntoArray("ColorPicker_Cancel"));
    _ambientTopFsa->addArcCallback("Cancel", this, &CavePainting::ambientTopLightColorCancel);

    _ambientBottomFsa = new Fsa("AmbientBottom");
    _ambientBottomFsa->addState("Start");
    _ambientBottomFsa->addArc("Change", "Start", "Start", MinVR::splitStringIntoArray("ColorPicker_ColorModified"));
    _ambientBottomFsa->addArcCallback("Change", this, &CavePainting::ambientBottomLightColorChange);
    _ambientBottomFsa->addArc("Set", "Start", "Start", MinVR::splitStringIntoArray("ColorPicker_ColorSelected"));
    _ambientBottomFsa->addArcCallback("Set", this, &CavePainting::ambientBottomLightColorSet);
    _ambientBottomFsa->addArc("Cancel", "Start", "Start", MinVR::splitStringIntoArray("ColorPicker_Cancel"));
    _ambientBottomFsa->addArcCallback("Cancel", this, &CavePainting::ambientBottomLightColorCancel);

    _bgColFsa = new Fsa("bgCol");
    _bgColFsa->addState("Start");
    _bgColFsa->addArc("Change", "Start", "Start", MinVR::splitStringIntoArray("ColorPicker_ColorModified"));
    _bgColFsa->addArcCallback("Change", this, &CavePainting::backgroundColorChange);
    _bgColFsa->addArc("Set", "Start", "Start", MinVR::splitStringIntoArray("ColorPicker_ColorSelected"));
    _bgColFsa->addArcCallback("Set", this, &CavePainting::backgroundColorSet);
    _bgColFsa->addArc("Cancel", "Start", "Start", MinVR::splitStringIntoArray("ColorPicker_Cancel"));
    _bgColFsa->addArcCallback("Cancel", this, &CavePainting::backgroundColorCancel);
  }
  else {
    // Viewer mode
    new ViewerHCI(_eventMgr, _gfxMgr);

    Array<std::string> slideTexNames;

    SlidePicker::loadSlideImages(_gfxMgr, slideTexNames);

    FsaRef fsa = new Fsa("CavePainting");
    fsa->addState("Start");
    fsa->addArc("NextFrame", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_RIGHT_down"));
    fsa->addArcCallback("NextFrame", this, &CavePainting::nextFrame);
    fsa->addArc("PrevFrame", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_LEFT_down"));
    fsa->addArcCallback("PrevFrame", this, &CavePainting::previousFrame);

    fsa->addArc("SaveSnapshot", "Start", "Start", MinVR::splitStringIntoArray("SaveSnapshot kbd_S_down"));
    fsa->addArcCallback("SaveSnapshot", this, &CavePainting::saveSnapshot);


    // Kbd shortcuts for toggling display of layers
    fsa->addArc("TogLay0", "Start", "Start", MinVR::splitStringIntoArray("kbd_0_down"));
    fsa->addArcCallback("TogLay0", this, &CavePainting::toggleLayer0);
    fsa->addArc("TogLay1", "Start", "Start", MinVR::splitStringIntoArray("kbd_1_down"));
    fsa->addArcCallback("TogLay1", this, &CavePainting::toggleLayer1);
    fsa->addArc("TogLay2", "Start", "Start", MinVR::splitStringIntoArray("kbd_2_down"));
    fsa->addArcCallback("TogLay2", this, &CavePainting::toggleLayer2);
    fsa->addArc("TogLay3", "Start", "Start", MinVR::splitStringIntoArray("kbd_3_down"));
    fsa->addArcCallback("TogLay3", this, &CavePainting::toggleLayer3);
    fsa->addArc("TogLay4", "Start", "Start", MinVR::splitStringIntoArray("kbd_4_down"));
    fsa->addArcCallback("TogLay4", this, &CavePainting::toggleLayer4);
    fsa->addArc("TogLay5", "Start", "Start", MinVR::splitStringIntoArray("kbd_5_down"));
    fsa->addArcCallback("TogLay5", this, &CavePainting::toggleLayer5);
    fsa->addArc("TogLay6", "Start", "Start", MinVR::splitStringIntoArray("kbd_6_down"));
    fsa->addArcCallback("TogLay6", this, &CavePainting::toggleLayer6);
    fsa->addArc("TogLay7", "Start", "Start", MinVR::splitStringIntoArray("kbd_7_down"));
    fsa->addArcCallback("TogLay7", this, &CavePainting::toggleLayer7);
    fsa->addArc("TogLay8", "Start", "Start", MinVR::splitStringIntoArray("kbd_8_down"));
    fsa->addArcCallback("TogLay8", this, &CavePainting::toggleLayer8);
    fsa->addArc("TogLay9", "Start", "Start", MinVR::splitStringIntoArray("kbd_9_down"));
    fsa->addArcCallback("TogLay9", this, &CavePainting::toggleLayer9);
    fsa->addArc("TogLay10", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_0_down"));
    fsa->addArcCallback("TogLay10", this, &CavePainting::toggleLayer10);
    fsa->addArc("TogLay11", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_1_down"));
    fsa->addArcCallback("TogLay11", this, &CavePainting::toggleLayer11);
    fsa->addArc("TogLay12", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_2_down"));
    fsa->addArcCallback("TogLay12", this, &CavePainting::toggleLayer12);
    fsa->addArc("TogLay13", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_3_down"));
    fsa->addArcCallback("TogLay13", this, &CavePainting::toggleLayer13);
    fsa->addArc("TogLay14", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_4_down"));
    fsa->addArcCallback("TogLay14", this, &CavePainting::toggleLayer14);
    fsa->addArc("TogLay15", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_5_down"));
    fsa->addArcCallback("TogLay15", this, &CavePainting::toggleLayer15);
    fsa->addArc("TogLay16", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_6_down"));
    fsa->addArcCallback("TogLay16", this, &CavePainting::toggleLayer16);
    fsa->addArc("TogLay17", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_7_down"));
    fsa->addArcCallback("TogLay17", this, &CavePainting::toggleLayer17);
    fsa->addArc("TogLay18", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_8_down"));
    fsa->addArcCallback("TogLay18", this, &CavePainting::toggleLayer18);
    fsa->addArc("TogLay19", "Start", "Start", MinVR::splitStringIntoArray("kbd_SHIFT_9_down"));
    fsa->addArcCallback("TogLay19", this, &CavePainting::toggleLayer19);
    _eventMgr->addFsaRef(fsa);
  }
 
  std::string artworkToLoad = MinVR::ConfigVal("LoadArtwork", "", false);
  if (artworkToLoad != "") {
    loadArtworkFile(artworkToLoad);

    if (MinVR::ConfigVal("OverrideLightsInFile", 0, false)) {
      lighting->ambientTop    = MinVR::ConfigVal("AmbientTop", Color3(0.35, 0.35, 0.35));
      lighting->ambientBottom = MinVR::ConfigVal("AmbientBottom", Color3(0.35, 0.35, 0.35));
      _clearColor = MinVR::ConfigVal("CLEAR_COLOR", Color3(0.35, 0.35, 0.35), false);
    }
  } 

  std::string modelToLoad = MinVR::ConfigVal("LoadVRML", "", false);
  if (modelToLoad != "") {
    loadVRMLModel(modelToLoad, MinVR::ConfigVal("LoadVRMLScale", 1.0, false),
      MinVR::ConfigVal("LoadVRMLHasNorms", 0, false));
    _gfxMgr->addDrawCallback(this, &CavePainting::modelDraw);
  }
}


CavePainting::~CavePainting()
{
}

void
CavePainting::activateReframeArtwork(MinVR::EventRef e)
{
  _hciMgr->activateWidget(_reframeArtworkHCI);
}

void
CavePainting::activateBrushSizePicker(MinVR::EventRef e)
{
  _hciMgr->activateWidget(_brushSizePicker);
}

void
CavePainting::activateBrushTipPicker(MinVR::EventRef e)
{
  _hciMgr->activateWidget(_brushTipPicker);
}

void
CavePainting::activateBrushModelPicker(MinVR::EventRef e)
{
  _hciMgr->activateWidget(_brushModelPicker);
}

void
CavePainting::activateColorSwatchPicker(MinVR::EventRef e)
{
  _hciMgr->activateWidget(_colorSwatchPicker);
}

void
CavePainting::activateMarkPicker(MinVR::EventRef e)
{
  _hciMgr->activateWidget(_markPicker);
}

void
CavePainting::activatePatternPicker(MinVR::EventRef e)
{
  _hciMgr->activateWidget(_patternPicker);
}

void
CavePainting::activateSlidePicker(MinVR::EventRef e)
{
  _hciMgr->activateWidget(_slidePicker);
}

void
CavePainting::activateLayerWidget(MinVR::EventRef e)
{
  _layerWidget->show();
  //_hciMgr->activateWidget(_layerWidget);
}

void
CavePainting::activateFrameWidget(MinVR::EventRef e)
{
  _frameWidget->show();
  //_hciMgr->activateWidget(_frameWidget);
}


void
CavePainting::activateHybridDrawing(MinVR::EventRef e)
{
  /*if (_forceNetInterface != NULL) {
    _hciMgr->activateStylusHCI(_forceHybridDrawHCI);
    _forceNetInterface->turnFrictionOn();    
    _forceNetInterface->setViscousGain(DEFAULT_VISCOUS_GAIN);
  }
  else {
    _hciMgr->activateStylusHCI(_hybridDrawHCI);
  }*/
}

void
CavePainting::activateTapeDrawing(MinVR::EventRef e)
{
  /*if (_forceNetInterface != NULL) {
    _hciMgr->activateStylusHCI(_forceTapeDrawHCI);
    _forceNetInterface->turnFrictionOn();    
    _forceNetInterface->setViscousGain(DEFAULT_VISCOUS_GAIN);
  }
  else {
    _hciMgr->activateStylusHCI(_tapeDrawHCI);
  }*/
}


void
CavePainting::activateFreehandDrawing(MinVR::EventRef e)
{
  /*if (_forceNetInterface != NULL) {
    _forceNetInterface->turnFrictionOn();    
    _forceNetInterface->setViscousGain(DEFAULT_VISCOUS_GAIN);
    _hciMgr->activateStylusHCI(_directDrawHCI);
    _forceNetInterface->startAnisotropicFilter();
  }
  else {
    _hciMgr->activateStylusHCI(_directDrawHCI);
  }*/
}

void
CavePainting::activateFreehandNoFrictionDrawing(MinVR::EventRef e)
{
  /*if (_forceNetInterface != NULL) {
    _forceNetInterface->stopForces();
    _forceNetInterface->turnFrictionOff();    
    _forceNetInterface->setViscousGain(0.0);
    _hciMgr->activateStylusHCI(_directDrawHCI);
    _forceNetInterface->startAnisotropicFilter();
  }
  else {
    _hciMgr->activateStylusHCI(_directDrawHCI);
  }*/
}

void
CavePainting::activateDragDrawing(MinVR::EventRef e)
{
  _hciMgr->activateStylusHCI(_dragDrawHCI);
  _dragDrawHCI->setLineLength(MinVR::ConfigVal("DragDrawHCI_ShortLineLength", 0.01, false));
}

void
CavePainting::activateDragDrawingLong(MinVR::EventRef e)
{
  _hciMgr->activateStylusHCI(_dragDrawHCI);
  _dragDrawHCI->setLineLength(MinVR::ConfigVal("DragDrawHCI_LongLineLength", 0.15, false));
}

void
CavePainting::activateReverseTapeDrawing(MinVR::EventRef e)
{
  _hciMgr->activateStylusHCI(_reverseTapeDrawHCI);
}

void
CavePainting::activateForceBlend(MinVR::EventRef e)
{
  _hciMgr->activateStylusHCI(_forceBlendHCI);
}


void
CavePainting::activateForceOneHandDrawing(MinVR::EventRef e)
{
  _hciMgr->activateStylusHCI(_forceOneHandDrawHCI);
}

void
CavePainting::activateForceMovingPlaneDrawing(MinVR::EventRef e)
{
  _hciMgr->activateStylusHCI(_forceMovingPlaneDrawHCI);
}

void
CavePainting::activateBlending(MinVR::EventRef e)
{
  _hciMgr->activateStylusHCI(_blendHCI);
  if (MinVR::ConfigValMap::map->containsKey("BlendHCI_DefaultBrushTip")) {
    _brush->state->brushTip = MinVR::ConfigVal("BlendHCI_DefaultBrushTip", 0);
  }
}


void
CavePainting::toggleHandOffset(MinVR::EventRef e)
{
  // Toggle the offset between 0 and some good value
  if (_handOffset == Vector3::zero()) {
    _handOffset = MinVR::ConfigVal("HandOffset", Vector3(0.5, 0, 0), false);
  }
  else {
    _handOffset = Vector3::zero();
  }

  /*if (_forceNetInterface != NULL) {
    // Apply the offset on the force net server side
    _forceNetInterface->setHandOffset(_handOffset);
  }
  else {
    // TODO: Otherwise, we're not using the forcenet server, so apply it in a way that will 
    // affect incoming hand events
    cerr << "Currently no way to change the hand offset without using the forceServer." << endl;
  }*/
}

void
CavePainting::undo(MinVR::EventRef e)
{
  _history->undo();
}

void
CavePainting::redo(MinVR::EventRef e)
{
  _history->redo();
}

void
CavePainting::activateDeleteMode(MinVR::EventRef e)
{
  _hciMgr->activateStylusHCI(_deleteHCI);
}

void
CavePainting::activateCopyToNextFrame(MinVR::EventRef e)
{
  _hciMgr->activateStylusHCI(_copyToNextFrameHCI);
}

void
CavePainting::activateMoveMarks(MinVR::EventRef e)
{
  _hciMgr->activateStylusHCI(_moveMarksHCI);
}

void
CavePainting::activateKbdSelectWidget(MinVR::EventRef e)
{
  _hciMgr->activateWidget(_kbdSelectWidget);
}

void
CavePainting::toggleFrictionAndViscosity(MinVR::EventRef e)
{
  /*if (_forceNetInterface != NULL) {
    if (_frictionOn) {
      _forceNetInterface->turnFrictionOn();    
      _forceNetInterface->setViscousGain(DEFAULT_VISCOUS_GAIN);
    }
    else {
      _forceNetInterface->turnFrictionOff();    
      _forceNetInterface->setViscousGain(0.0);
    }
    _frictionOn = !_frictionOn;
  }*/
}

void
CavePainting::setPointSplatMarkType(MinVR::EventRef e)
{
  _brush->setNewMarkType(0);
}

void
CavePainting::setRibbonMarkType(MinVR::EventRef e)
{
  _brush->setNewMarkType(1);
}

void
CavePainting::saveSnapshot(MinVR::EventRef e)
{
  // Since the cluster nodes are on the same file system, just save a
  // snapshot from the master.
  if (MinVR::ConfigValMap::map->containsKey(string("CLUSTER_NODE_ID")) &&
       MinVR::ConfigVal("CLUSTER_NODE_ID",-1) != 0 ) {
    return;
  }

  std::string base  = MinVR::ConfigVal("SaveBasePath", "share/artwork",false) + "/";

  //saveStereoSnapshotOnNextFrame(base);

  std::string filename = _gfxMgr->getRenderDevice()->screenshot(base);
  cout << "Saved snapshot of screen to file: " << filename << endl;
}

void
CavePainting::startFilenameInputForSaveArtwork(MinVR::EventRef e)
{
  _hciMgr->activateWidget(_textInputWidget);

  std::string fname = filenameBaseExt(_lastFilename);
  if (endsWith(fname, ".3DArt")) {
    fname = fname.substr(0, fname.length()-6);
  }
  while ((fname.length()) && (isDigit(fname[fname.length()-1]))) {
    fname = fname.substr(0, fname.length()-1);
  }

  std::string base  = MinVR::ConfigVal("SaveBasePath", "share/artwork",false) + "/" + fname;
  std::string extension = ".3DArt";
  std::string filename = base + extension;
  if (FileSystem::exists(filename)) {
    int c=2;
    while (FileSystem::exists(base + MinVR::intToString(c) + extension)) {
      c++;
    }
    fname = fname + MinVR::intToString(c);
    filename = base + MinVR::intToString(c) + extension;
  }
  
  _textInputWidget->setText(fname);
}

void
CavePainting::saveArtwork(MinVR::EventRef e)
{
  if (MinVR::ConfigValMap::map->containsKey(string("CLUSTER_NODE_ID")) &&
    MinVR::ConfigVal("CLUSTER_NODE_ID",-1) != 0 ) {
    return;
  }

  std::string fname = e->getMsgData();
  if (fname == "") {
    fname = MinVR::ConfigVal("SaveFileName", "Drawing", false);
  }
  std::string base  = MinVR::ConfigVal("SaveBasePath", "share/artwork",false) + "/" + fname;
  std::string extension = ".3DArt";
  std::string filename = base + extension;
  if (FileSystem::exists(filename)) {
    int c=2;
    while (FileSystem::exists(base + MinVR::intToString(c) + extension)) {
      c++;
    }
    filename = base + MinVR::intToString(c) + extension;
  }

  cout << "Saving Artwork to file: " << filename << endl;
  
  BinaryOutput bo(filename, G3D_LITTLE_ENDIAN);
  ArtworkIO::serialize(bo, _artwork, Color3(_clearColor.r, _clearColor.g, _clearColor.b),
                       _colorSwatchPicker, _brushTipPicker, _patternPicker);
  bo.commit();

  _lastFilename = filenameBaseExt(filename);
}


void
CavePainting::selectAmbientTopLightColor(MinVR::EventRef e)
{
  _eventMgr->addFsaRef(_ambientTopFsa);
  _colorChangeCancelColor = _gfxMgr->getLighting()->ambientTop;
  _colorPicker->setInitialColor(_gfxMgr->getLighting()->ambientTop);
  _hciMgr->activateWidget(_colorPicker);
}

void
CavePainting::ambientTopLightColorChange(MinVR::EventRef e)
{
  Color3 col(e->get3DData()[0], e->get3DData()[1], e->get3DData()[2]);
  _gfxMgr->getLighting()->ambientTop = col;
}

void
CavePainting::ambientTopLightColorSet(MinVR::EventRef e)
{
  Color3 col(e->get3DData()[0], e->get3DData()[1], e->get3DData()[2]);
  _gfxMgr->getLighting()->ambientTop = col;
  _eventMgr->removeFsaRef(_ambientTopFsa);
}

void
CavePainting::ambientTopLightColorCancel(MinVR::EventRef e)
{
  _gfxMgr->getLighting()->ambientTop = 
    Color3(_colorChangeCancelColor.r, _colorChangeCancelColor.g, _colorChangeCancelColor.b);
  _eventMgr->removeFsaRef(_ambientTopFsa);
}

void
CavePainting::selectAmbientBottomLightColor(MinVR::EventRef e)
{
  _eventMgr->addFsaRef(_ambientBottomFsa);
  _colorChangeCancelColor = _gfxMgr->getLighting()->ambientBottom;
  _colorPicker->setInitialColor(_gfxMgr->getLighting()->ambientBottom);
  _hciMgr->activateWidget(_colorPicker);
}

void
CavePainting::ambientBottomLightColorChange(MinVR::EventRef e)
{
  Color3 col(e->get3DData()[0], e->get3DData()[1], e->get3DData()[2]);
  _gfxMgr->getLighting()->ambientBottom = col;
}

void
CavePainting::ambientBottomLightColorSet(MinVR::EventRef e)
{
  Color3 col(e->get3DData()[0], e->get3DData()[1], e->get3DData()[2]);
  _gfxMgr->getLighting()->ambientBottom = col;
  _eventMgr->removeFsaRef(_ambientBottomFsa);
}

void
CavePainting::ambientBottomLightColorCancel(MinVR::EventRef e)
{
  _gfxMgr->getLighting()->ambientBottom = 
    Color3(_colorChangeCancelColor.r, _colorChangeCancelColor.g, _colorChangeCancelColor.b);
  _eventMgr->removeFsaRef(_ambientBottomFsa);
}


void
CavePainting::selectBackgroundColor(MinVR::EventRef e)
{
  _eventMgr->addFsaRef(_bgColFsa);
  _colorChangeCancelColor = _clearColor;
  _colorPicker->setInitialColor(Color3(_clearColor.r, _clearColor.g, _clearColor.b));
  _hciMgr->activateWidget(_colorPicker);
}

void
CavePainting::backgroundColorChange(MinVR::EventRef e)
{
  Color3 col(e->get3DData()[0], e->get3DData()[1], e->get3DData()[2]);
  _clearColor = col;
}

void
CavePainting::backgroundColorSet(MinVR::EventRef e)
{
  Color3 col(e->get3DData()[0], e->get3DData()[1], e->get3DData()[2]);
  _clearColor = col;
  _eventMgr->removeFsaRef(_bgColFsa);
}

void
CavePainting::backgroundColorCancel(MinVR::EventRef e)
{
  _clearColor = _colorChangeCancelColor;
  _eventMgr->removeFsaRef(_bgColFsa);
}

void
CavePainting::nextFrame(MinVR::EventRef e)
{
  _artwork->nextFrame();
  _brush->state->frameIndex = _artwork->getCurrentFrame();
} 

void
CavePainting::previousFrame(MinVR::EventRef e)
{
  _artwork->previousFrame();
  _brush->state->frameIndex = _artwork->getCurrentFrame();
} 

void
CavePainting::kbdPressureUp(MinVR::EventRef e)
{
  _eventMgr->queueEvent(new MinVR::VRG3DEvent("Brush_Pressure", _brush->state->pressure + 0.1));
}

void
CavePainting::kbdPressureDown(MinVR::EventRef e)
{
  _eventMgr->queueEvent(new MinVR::VRG3DEvent("Brush_Pressure", _brush->state->pressure - 0.1));
}

void
CavePainting::brushPressureDeviceUpdate(MinVR::EventRef e)
{
  // Handle treating the pressure at a btn event in here and remap pressure to 0 -> 1 range

  double min = MinVR::ConfigVal("Pressure_Min", 0.05, false);
  double max = MinVR::ConfigVal("Pressure_Max", 1.0, false);
  
  double pressure = clamp((e->get1DData() - min) / (max - min), 0.0, 1.0);
  // rescale so that the minimum pressure is > 0 so we get a mark of
  // non-zero width all the time.
  pressure = (pressure + 0.1) / 1.1;

  cout << "pressure -- raw:" << e->get1DData() << " value: " << pressure << endl;

  _eventMgr->queueEvent(new MinVR::VRG3DEvent("Brush_Pressure", pressure));

  if ((!_pressureBtnPressed) && (e->get1DData() > min)) {
    _eventMgr->queueEvent(new MinVR::VRG3DEvent("Brush_Btn_down"));
    _pressureBtnPressed = true;
  }
  else if ((_pressureBtnPressed) && (e->get1DData() < min)) {
    _eventMgr->queueEvent(new MinVR::VRG3DEvent("Brush_Btn_up"));
    _pressureBtnPressed = false;
  }
}

void
CavePainting::activateLoadFilePicker(MinVR::EventRef e)
{
  _hciMgr->activateWidget(_loadFilePicker);
}


void
CavePainting::loadArtwork(MinVR::EventRef e)
{
  myRenderDevice->endFrame();
  loadArtworkFile(e->getMsgData());
  myRenderDevice->beginFrame();
}

void
CavePainting::loadArtworkFile(const std::string &artworkToLoad)
{
  std::string fname = artworkToLoad;
  fname = MinVR::decygifyPath(MinVR::replaceEnvVars(fname));
  if (fname != "") {
    _artwork->clear();
    _brush->state->frameIndex = 0;
    _brush->state->layerIndex = 0;
    cout << "Loading: " << fname << endl;
    BinaryInput bi(fname, G3D_LITTLE_ENDIAN, false);
    Color3 bgColor;
    ArtworkIO::deserialize(bi, _artwork, bgColor, 
                           _colorSwatchPicker, _brushTipPicker, _patternPicker);
    // Let loading the artwork set the number of frames, but if we
    // have markers loaded, there should be at least as many frames as
    // marker frames, even if nothing is painted on them.
    /*
	if ((_markers.notNull()) && (_markers->getNumMarkerSets() > _artwork->getNumFrames())) {
      _artwork->setNumFrames(_markers->getNumMarkerSets());
    }
    */

    _clearColor = bgColor;
  }

#ifdef USE_SPLINES
  if (ConfigVal("TestSplineFit", false, false)) {

    Array<MarkRef> marks = _artwork->getMarks();
    for (int i=marks.size()-1;i<marks.size();i++) {
      MarkRef m = marks[i];
      Array<Vector3> ctrlPoints;
      Array<Vector3> segPoints;
      double samplen = ConfigVal("SplineResampleLength", 0.001, false);
      MarkRef splineMark = SplineFit(m, samplen, ctrlPoints, segPoints, 
                                     _artwork->getTriStripModel(), _gfxMgr);
      _artwork->addMark(splineMark);
      splineMark->commitGeometry(_artwork);
      _history->storeMarkCreated(splineMark, _artwork);
    }
  }
#endif
}


void
CavePainting::loadVRMLModel(const std::string &filename, double scale, bool hasNorms)
{
  std::string fname = filename;
  fname = MinVR::decygifyPath(MinVR::replaceEnvVars(fname));
  if (fname != "") {
    SMeshRef s;
    if (hasNorms) {
      s = VRML2Parser::createSMeshWithNormsFromFile(fname, scale);
    }
    else {
      s = VRML2Parser::createSMeshFromFile(fname/*, true*/, scale);
    }
    if (s.notNull()) {
      _models.append(s);
    }
  }
}


void curvature(MarkRef mark, Array<double> &curvatures, Array<Vector3> &centerOfCurvatures)
{
  /**
     see: http://www-math.mit.edu/18.013A/HTML/chapter15/section04.html

     K =  | (a(v*v) - v(a*v)) / (v*v) |
          -----------------------------
                     v*v    
  **/

  // assume the mark has already been resampled, otherwise may need to
  // add delta t into calculation.

  Array<Vector3> v;
  Array<Vector3> a;
  for (int i=0;i<mark->getNumSamples()-1;i++) {
    v.append(mark->getSamplePosition(i+1) - mark->getSamplePosition(i));
    if (v.size() < 2) {
      a.append(Vector3::zero());
    }
    else {
      a.append(v[v.size()-1] - v[v.size()-2]);
    }
  }

  for (int i=0;i<v.size();i++) {
    double vdotv = v[i].dot(v[i]);
    double adotv = a[i].dot(v[i]);
    
    // curvature
    double K = ( ((vdotv*a[i] - adotv*v[i]) / vdotv).length() ) / vdotv;
    curvatures.append(K);
    
    // normal vector
    Vector3 aunit = a[i].unit();
    Vector3 vunit = v[i].unit();
    Vector3 N = (aunit - aunit.dot(vunit)*vunit).unit();

    // radius of curvature
    double r = 1/K;

    // center of curvature
    Vector3 q = mark->getSamplePosition(i) + r*N;
    centerOfCurvatures.append(q);
  }

}


void
CavePainting::doGraphics(RenderDevice *rd) 
{
  // TODO: this would get called more than once per frame here.. not necessary.
  /*
  if (_markers.notNull()) {
    if (_artwork->getCurrentFrame() < _markers->getNumMarkerSets()) {
      _markers->displayOneSetOnly(_artwork->getCurrentFrame());
    }
  }
  */

  // Quick hack for drawing some tracing prompts..
  if (MinVR::ConfigVal("DrawPrompt", 0, false)) {
    rd->pushState();

    rd->enableLighting();
    for (int i=0;i<RenderDevice::MAX_LIGHTS;i++) {
      rd->setLight(i, NULL);
    }
    rd->setLight(0, GLight::point(Vector3(0,0.4,-0.1), Color3(0.5, 0.5, 0.5)));
    
    double w = 0.005;
    double p = -0.4;
    double r = 0.005;
    for (int i=0;i<9;i++) {
      Draw::cylinder(Cylinder(Vector3(p, -0.15, 0.1), Vector3(p+w, -0.15, 0.1), r), rd,
                     Color3(0.7, 0.7, 0.0), Color4::clear());
      p += 0.1;
      if (i<4) {
        r += 0.0025;
      }
      else {
        r -= 0.0025;
      }
    }
    
    rd->setColor(Color3::white());
    rd->disableLighting();
    rd->beginPrimitive(PrimitiveType::LINES);
    rd->sendVertex(Vector3(-0.45, -0.15, 0.1));
    rd->sendVertex(Vector3(0.45, -0.15, 0.1));
    rd->endPrimitive();
    rd->popState();

  }

  _gfxMgr->drawFrame();


  // Test curvature calculations
  /**
  Array<MarkRef> marks = _artwork->getMarks();
  for (int i=0;i<marks.size();i++) {
    Array<double> K;
    Array<Vector3> q;
    curvature(marks[i], K, q);
    for (int j=0;j<q.size();j++) {
      Draw::sphere(Sphere(q[j], 0.01), rd, Color3::blue(), Color4::clear());
      Draw::lineSegment(LineSegment::fromTwoPoints(marks[i]->getSamplePosition(j), q[j]), rd, Color3::yellow());
    }
  }
  **/

  if (_fishtankDepthCues.notNull()) {
    _fishtankDepthCues->draw(rd, CoordinateFrame());
  }

  // draw a plane for shadows
  if (_drawFloor) {
    rd->pushState();
    rd->setColor(Color3(0.8, 0.8, 0.8));
    rd->setTexture(0, _gfxMgr->getTexture("Floor"));
    rd->setCullFace(RenderDevice::CULL_NONE);
    
    double y = MinVR::ConfigVal("ShadowPlaneY", -0.15, false) - 0.001;
    double w = MinVR::ConfigVal("FloorW", 0.8, false);
    double h = MinVR::ConfigVal("FloorD", 0.6, false);
    rd->beginPrimitive(PrimitiveType::QUADS);
    rd->setNormal(Vector3(0,1,0));
    rd->setTexCoord(0, Vector2(0,1));
    rd->sendVertex(Vector3(-w/2.0, y, h/2.0));
    rd->setTexCoord(0, Vector2(0,0));
    rd->sendVertex(Vector3(-w/2.0, y, -h/2.0));
    rd->setTexCoord(0, Vector2(1,0));
    rd->sendVertex(Vector3(w/2.0, y, -h/2.0));
    rd->setTexCoord(0, Vector2(1,1));
    rd->sendVertex(Vector3(w/2.0, y, h/2.0));
    rd->endPrimitive();
    rd->popState();
  }

  
  // draw some 2D things here, to make sure it's after everything else
  if ((_modeMenu.notNull()) && (!_modeMenu->getHidden())) {
    // only draw these if the menus are displayed, so we can turn them
    // off to get a good screenshot
    _gfxMgr->drawStats();
    if (_cavePaintingCursors.notNull()) {
      _cavePaintingCursors->drawActiveLayerStatus(rd);
    }
  }
  if (_textInputWidget.notNull()) {
    _textInputWidget->draw(rd, CoordinateFrame());
  }
  //vrg3dSleepMsecs(MinVR::ConfigVal("VRBaseApp_SleepTime", 0.0, false));
}


void CavePainting::onRenderGraphicsScene(const MinVR::VRGraphicsState& state)
{
  VRG3DBaseApp::onRenderGraphicsScene(state);
  doGraphics(myRenderDevice);
}


void CavePainting::onAnalogChange(const MinVR::VRAnalogEvent &event)
{

  if (event.getName().find("HTC_Controller_1_Trigger1") != -1
    || event.getName().find("HTC_Controller_Right_Trigger1") != -1)
  {

    //double min = MinVR::ConfigVal("Pressure_Min", 0.05, false);
    //double max = MinVR::ConfigVal("Pressure_Max", 1.0, false);
    //double evalue = event.getValue();
    //double pressure = clamp((evalue - min) / (max - min), 0.0, 1.0);
    // rescale so that the minimum pressure is > 0 so we get a mark of
    // non-zero width all the time.
    //pressure = (pressure + 0.1) / 1.1;
    
    _eventMgr->queueEvent(new MinVR::VRG3DEvent("My_Brush_Pressure", event.getValue()));

    //brushPressureDeviceUpdate(new MinVR::VRG3DEvent("", event.getValue()));
    
    /*
    if ((!_pressureBtnPressed) && (evalue > min)) {
      _eventMgr->queueEvent(new MinVR::VRG3DEvent("Brush_Btn_down"));
      _pressureBtnPressed = true;
    }
    else if ((_pressureBtnPressed) && (evalue < min)) {
      _eventMgr->queueEvent(new MinVR::VRG3DEvent("Brush_Btn_up"));
      _pressureBtnPressed = false;
    }
    */
  }
  _eventMgr->processEventQueue();
}

void
CavePainting::modelDraw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  rd->pushState();
  rd->setObjectToWorldMatrix(virtualToRoomSpace);
  rd->setShadeMode(RenderDevice::SHADE_SMOOTH);
  rd->setCullFace(RenderDevice::CULL_NONE);
  rd->setColor(MinVR::ConfigVal("VRMLModelColor", Color3::white(), false));
  for (int i=0;i<_models.size();i++) {
    _models[i]->draw(rd, 0, _gfxMgr);
  }
  rd->popState();
}



void
CavePainting::toggleShowMenus(MinVR::EventRef e)
{
  if (_modeMenu->getHidden()) {
    _modeMenu->show();
    _brushMenu->show();
    _artworkMenu->show();
    _oopsMenu->show();
    _systemMenu->show();
  }
  else {
    _modeMenu->hide();
    _brushMenu->hide();
    _artworkMenu->hide();
    _oopsMenu->hide();
    _systemMenu->hide();
  }
}

void
CavePainting::toggleShowShadows(MinVR::EventRef e)
{
  _drawFloor = !_drawFloor;
  setShadowsOn(_drawFloor);
}


void
CavePainting::toggleLayer(int layerID)
{
  Array<int> hidden = _artwork->getHiddenLayers();
  if (hidden.contains(layerID)) {
    // take layerID out of hidden layers
    Array<int> hnew;
    for (int i=0;i<hidden.size();i++) {
      if (hidden[i] != layerID) {
        hnew.append(hidden[i]);
      }
    }
    _artwork->setHiddenLayers(hnew);
  }
  else if (_artwork->getMaxLayerID() >= layerID) {
    // add layerID to hidden layers
    hidden.append(layerID);
    _artwork->setHiddenLayers(hidden);
  }
}

void
CavePainting::toggleLayer0(MinVR::EventRef e)
{
  toggleLayer(0);
}

void
CavePainting::toggleLayer1(MinVR::EventRef e)
{
  toggleLayer(1);
}

void
CavePainting::toggleLayer2(MinVR::EventRef e)
{
  toggleLayer(2);
}

void
CavePainting::toggleLayer3(MinVR::EventRef e)
{
  toggleLayer(3);
}

void
CavePainting::toggleLayer4(MinVR::EventRef e)
{
  toggleLayer(4);
}

void
CavePainting::toggleLayer5(MinVR::EventRef e)
{
  toggleLayer(5);
}

void
CavePainting::toggleLayer6(MinVR::EventRef e)
{
  toggleLayer(6);
}

void
CavePainting::toggleLayer7(MinVR::EventRef e)
{
  toggleLayer(7);
}

void
CavePainting::toggleLayer8(MinVR::EventRef e)
{
  toggleLayer(8);
}

void
CavePainting::toggleLayer9(MinVR::EventRef e)
{
  toggleLayer(9);
}

void
CavePainting::toggleLayer10(MinVR::EventRef e)
{
  toggleLayer(10);
}

void
CavePainting::toggleLayer11(MinVR::EventRef e)
{
  toggleLayer(11);
}

void
CavePainting::toggleLayer12(MinVR::EventRef e)
{
  toggleLayer(12);
}

void
CavePainting::toggleLayer13(MinVR::EventRef e)
{
  toggleLayer(13);
}

void
CavePainting::toggleLayer14(MinVR::EventRef e)
{
  toggleLayer(14);
}

void
CavePainting::toggleLayer15(MinVR::EventRef e)
{
  toggleLayer(15);
}

void
CavePainting::toggleLayer16(MinVR::EventRef e)
{
  toggleLayer(16);
}

void
CavePainting::toggleLayer17(MinVR::EventRef e)
{
  toggleLayer(17);
}

void
CavePainting::toggleLayer18(MinVR::EventRef e)
{
  toggleLayer(18);
}

void
CavePainting::toggleLayer19(MinVR::EventRef e)
{
  toggleLayer(19);
}

void
CavePainting::toggleDynamicDragging(MinVR::EventRef e)
{
  //_forceNetInterface->toggleDynamicDragging();
}

void
CavePainting::toggleConstantDragLengthDisplay(MinVR::EventRef e)
{
  _forceHybridDrawHCI->toggleConstantDragLengthDisplay();
}


void
CavePainting::shutdown(MinVR::EventRef e)
{
  /*if (_forceNetInterface != NULL) {
    _forceNetInterface->shutdown();
  }*/
  exit(0);
}


} // end namespace



