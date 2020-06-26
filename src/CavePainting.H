/**
 * /author Dan Keefe (dfk)
 *
 * /file  CavePainting.H
 * /brief Uses the freeformdraw library to create an instance of the
 *        CavePainting application.
 *
 */ 

#ifndef CAVEPAINTING_H
#define CAVEPAINTING_H

#include <VRG3DBaseApp.h>
#include <VRG3DEvent.h>
#include <Fsa.H>
#include <SMesh.H>


#include "Artwork.H"
#include "BlendHCI.H"
#include "Brush.H"
#include "BrushModelPicker.H"
#include "BrushSizePicker.H"
#include "BrushState.H"
#include "BrushTipPicker.H"
#include "CavePaintingCursors.H"
#include "ColorPicker.H"
#include "ColorSwatchPicker.H"
#include "CopyToNextFrameHCI.H"
#include "DeleteHCI.H"
#include "DirectDrawHCI.H"
#include "DragDrawHCI.H"
#include "FishtankDepthCues.H"
#include "ForceBlendHCI.H"
#include "ForceHybridDrawHCI.H"
#include "ForceMovingPlaneDrawHCI.H"
#include "ForceNetInterface.H"
#include "ForceOneHandDrawHCI.H"
#include "ForceReverseTapeDrawHCI.H"
#include "ForceTapeDrawHCI.H"
#include "FrameWidget.H"
#include "HCIMgr.H"
#include "History.H"
#include "HybridDrawHCI.H"
#include "KbdSelectWidget.H"
#include "LayerWidget.H"
#include "LoadFilePicker.H"
//#include "Markers.H"
#include "MarkingMenu.H"
#include "MarkPicker.H"
#include "MoveMarksHCI.H"
#include "PatternPicker.H"
#include "PlaceSlideHCI.H"
#include "ReframeArtworkHCI.H"
#include "ReverseTapeDrawHCI.H"
#include "SlidePicker.H"
#include "TapeDrawHCI.H"
#include "TextInputWidget.H"
#include "WorkingLimits.H"

namespace DrawOnAir {

/** This is the main CavePainting class.  To run cavepainting, from
    your program, all you need to do is call
    CavePainting::fromArguments(argc, argv);
*/
class CavePainting : public VRG3DBaseApp
{
public:

  CavePainting(int argc, char **argv);
  virtual ~CavePainting();

  /// Overrides VRBaseApp version
  virtual void doGraphics(G3D::RenderDevice *rd);
  void onRenderGraphicsScene(const MinVR::VRGraphicsState& state);
  // Callback methods that respond to Menu events, button presses, etc..
  void onAnalogChange(const MinVR::VRAnalogEvent &event);

  void activateReframeArtwork(MinVR::EventRef e);  
  void activateBlending(MinVR::EventRef e);
  void activateBrushModelPicker(MinVR::EventRef e);
  void activateBrushSizePicker(MinVR::EventRef e);
  void activateBrushTipPicker(MinVR::EventRef e);
  void activatePatternPicker(MinVR::EventRef e);
  void activateSlidePicker(MinVR::EventRef e);
  void activateColorSwatchPicker(MinVR::EventRef e);
  void activateCopyToNextFrame(MinVR::EventRef e);
  void activateDeleteMode(MinVR::EventRef e);
  void activateLayerWidget(MinVR::EventRef e);
  void activateFrameWidget(MinVR::EventRef e);
  void activateMarkPicker(MinVR::EventRef e);
  void activateMoveMarks(MinVR::EventRef e);
  void toggleHandOffset(MinVR::EventRef e);
  void toggleFrictionAndViscosity(MinVR::EventRef e);
  void setPointSplatMarkType(MinVR::EventRef e);
  void setRibbonMarkType(MinVR::EventRef e);
  void undo(MinVR::EventRef e);
  void redo(MinVR::EventRef e);
  void saveSnapshot(MinVR::EventRef e);
  void startFilenameInputForSaveArtwork(MinVR::EventRef e);
  void saveArtwork(MinVR::EventRef e);
  void activateLoadFilePicker(MinVR::EventRef e);
  void loadArtwork(MinVR::EventRef e);
  void loadArtworkFile(const std::string &filename);
  void loadVRMLModel(const std::string &filename, double scale=1.0, bool hasNorms=false);
  void activateKbdSelectWidget(MinVR::EventRef e);

  // basic interactions, selects force-enabled versions if force interface available
  void activateHybridDrawing(MinVR::EventRef e);
  void activateTapeDrawing(MinVR::EventRef e);
  void activateFreehandDrawing(MinVR::EventRef e);
  void activateFreehandNoFrictionDrawing(MinVR::EventRef e);

  // older experimental drawing interfaces
  void activateDragDrawing(MinVR::EventRef e);
  void activateDragDrawingLong(MinVR::EventRef e);
  void activateReverseTapeDrawing(MinVR::EventRef e);
  void activateForceBlend(MinVR::EventRef e);
  void activateForceOneHandDrawing(MinVR::EventRef e);
  void activateForceMovingPlaneDrawing(MinVR::EventRef e);


  // lighting
  void selectAmbientTopLightColor(MinVR::EventRef e);
  void ambientTopLightColorChange(MinVR::EventRef e);
  void ambientTopLightColorSet(MinVR::EventRef e);
  void ambientTopLightColorCancel(MinVR::EventRef e);
  void selectAmbientBottomLightColor(MinVR::EventRef e);
  void ambientBottomLightColorChange(MinVR::EventRef e);
  void ambientBottomLightColorSet(MinVR::EventRef e);
  void ambientBottomLightColorCancel(MinVR::EventRef e);
  void selectBackgroundColor(MinVR::EventRef e);
  void backgroundColorChange(MinVR::EventRef e);
  void backgroundColorSet(MinVR::EventRef e);
  void backgroundColorCancel(MinVR::EventRef e);

  void nextFrame(MinVR::EventRef e);
  void previousFrame(MinVR::EventRef e);

  void kbdPressureUp(MinVR::EventRef e);
  void kbdPressureDown(MinVR::EventRef e);

  void toggleShowMenus(MinVR::EventRef e);
  void toggleShowShadows(MinVR::EventRef e);

  void brushPressureDeviceUpdate(MinVR::EventRef e);

  void toggleLayer(int layerID);

  // kbd shortcuts for toggling display of layers
  void toggleLayer0(MinVR::EventRef e);
  void toggleLayer1(MinVR::EventRef e);
  void toggleLayer2(MinVR::EventRef e);
  void toggleLayer3(MinVR::EventRef e);
  void toggleLayer4(MinVR::EventRef e);
  void toggleLayer5(MinVR::EventRef e);
  void toggleLayer6(MinVR::EventRef e);
  void toggleLayer7(MinVR::EventRef e);
  void toggleLayer8(MinVR::EventRef e);
  void toggleLayer9(MinVR::EventRef e);
  void toggleLayer10(MinVR::EventRef e);
  void toggleLayer11(MinVR::EventRef e);
  void toggleLayer12(MinVR::EventRef e);
  void toggleLayer13(MinVR::EventRef e);
  void toggleLayer14(MinVR::EventRef e);
  void toggleLayer15(MinVR::EventRef e);
  void toggleLayer16(MinVR::EventRef e);
  void toggleLayer17(MinVR::EventRef e);
  void toggleLayer18(MinVR::EventRef e);
  void toggleLayer19(MinVR::EventRef e);

  void toggleDynamicDragging(MinVR::EventRef e);
  void toggleConstantDragLengthDisplay(MinVR::EventRef e);

  void shutdown(MinVR::EventRef e);

  void modelDraw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);

private:
  CavePainting(const CavePainting &);
  CavePainting& operator=(const CavePainting &);

  ArtworkRef                 _artwork;
  BlendHCIRef                _blendHCI;
  BrushRef                   _brush;
  BrushModelPickerRef        _brushModelPicker;
  BrushSizePickerRef         _brushSizePicker;
  BrushTipPickerRef          _brushTipPicker;
  CavePaintingCursorsRef     _cavePaintingCursors;
  G3D::Color4                     _colorChangeCancelColor;
  ColorPickerRef             _colorPicker;
  ColorSwatchPickerRef       _colorSwatchPicker;
  CopyToNextFrameHCIRef      _copyToNextFrameHCI;
  DeleteHCIRef               _deleteHCI;
  DirectDrawHCIRef           _directDrawHCI;
  DragDrawHCIRef             _dragDrawHCI;
  FishtankDepthCuesRef       _fishtankDepthCues;
  ForceBlendHCIRef           _forceBlendHCI;
  ForceHybridDrawHCIRef      _forceHybridDrawHCI;
  ForceMovingPlaneDrawHCIRef _forceMovingPlaneDrawHCI;
  //ForceNetInterface*         _forceNetInterface;
  ForceOneHandDrawHCIRef     _forceOneHandDrawHCI;
  ForceReverseTapeDrawHCIRef _forceReverseTapeDrawHCI;
  ForceTapeDrawHCIRef        _forceTapeDrawHCI;
  FrameWidgetRef             _frameWidget;
  FsaRef                     _ambientTopFsa;
  FsaRef                     _ambientBottomFsa;
  FsaRef                     _bgColFsa;
  HCIMgrRef                  _hciMgr;
  HistoryRef                 _history;
  HybridDrawHCIRef           _hybridDrawHCI;
  KbdSelectWidgetRef         _kbdSelectWidget;
  LayerWidgetRef             _layerWidget;
  LoadFilePickerRef          _loadFilePicker;
  //MarkersRef                 _markers;
  MarkingMenuRef             _artworkMenu;
  MarkingMenuRef             _brushMenu;
  MarkingMenuRef             _modeMenu;
  MarkingMenuRef             _oopsMenu;
  MarkingMenuRef             _systemMenu;
  MarkPickerRef              _markPicker;
  MoveMarksHCIRef            _moveMarksHCI;
  PatternPickerRef           _patternPicker;
  PlaceSlideHCIRef           _placeSlideHCI;
  ReframeArtworkHCIRef       _reframeArtworkHCI;
  ReverseTapeDrawHCIRef      _reverseTapeDrawHCI;
  SlidePickerRef             _slidePicker;
  TapeDrawHCIRef             _tapeDrawHCI;
  TextInputWidgetRef         _textInputWidget;
  G3D::Vector3                    _handOffset;
  WorkingLimitsRef           _workingLimits;

  bool                       _frictionOn;
  bool                       _pressureBtnPressed;
  std::string                _lastFilename;
  bool                       _drawFloor;

  G3D::Array<SMeshRef>            _models;
};

} // end namespace

#endif



