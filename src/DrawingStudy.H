/**
 * /author Dan Keefe (dfk)
 *
 * /file  DrawingStudy.H
 * /brief Starts up basic drawing support, but not all the extra
   controls for changing color etc.. that you find in the usual full
   blown drawing appliation.
 *
 */ 

#ifndef DRAWINGSTUDY_H
#define DRAWINGSTUDY_H

//#include "VRBase.H"

#include <VRG3DBaseApp.h>

#include "Artwork.H"
#include "Brush.H"
#include "BrushState.H"
#include "CavePaintingCursors.H"
#include "DirectDrawHCI.H"
#include "DragDrawHCI.H"
#include "FishtankDepthCues.H"
#include "ForceHybridDrawHCI.H"
#include "ForceNetInterface.H"
#include "ForceOneHandDrawHCI.H"
#include "ForceReverseTapeDrawHCI.H"
#include "ForceTapeDrawHCI.H"
#include "HCIMgr.H"
#include "History.H"
#include "ReverseTapeDrawHCI.H"
#include "TapeDrawHCI.H"

namespace DrawOnAir {


typedef G3D::ReferenceCountedPointer<class DrawingDisabler> DrawingDisablerRef;
/**
 */
class DrawingDisabler : public WidgetHCI
{
public:
  DrawingDisabler(EventMgrRef eventMgr, HCIMgrRef hciMgr, BrushRef brush, G3D::Array<std::string> brushMoveTriggers) 
    : WidgetHCI(hciMgr) {
    _brush = brush;
    _fsa = new Fsa("DrawingDisabler");
    _fsa->addState("DoNothing");
    _fsa->addState("CaptureBrushMovement");
    _fsa->addArc("BrushMove", "CaptureBrushMovement", "CaptureBrushMovement", brushMoveTriggers);
    _fsa->addArcCallback("BrushMove", this, &DrawingDisabler::brushMovement);
    eventMgr->addFsaRef(_fsa);
  }
  void disableDrawing() { 
    activateViaHCIMgr(); 
    _fsa->jumpToState("CaptureBrushMovement");
  }
  void enableDrawing()  { 
    releaseFocusWithHCIMgr(); 
    _fsa->jumpToState("DoNothing");
  }
  void activate()       {}

  void brushMovement(MinVR::EventRef e) {
    _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  }

private:
  DrawingDisabler(const DrawingDisabler &);
  BrushRef _brush;
  FsaRef   _fsa;
};


/**
*/
class DrawingStudy : public VRG3DBaseApp
{
public:

  DrawingStudy(int argc, char **argv);
  virtual ~DrawingStudy();

  virtual void disableDrawing();
  virtual void enableDrawing();
  
  virtual void saveSnapshot(const std::string &filename);
  virtual void saveArtwork(const std::string &filename);

  /// Overrides VRBaseApp version
  virtual void doGraphics(G3D::RenderDevice *rd);

  virtual void doUserInput(G3D::Array<MinVR::EventRef> &events);

  //virtual ForceNetInterface* forceNetInterface() { return _forceNetInterface; }
  virtual CavePaintingCursorsRef cursors() { return _cavePaintingCursors; }

protected:
  ArtworkRef                 _artwork;
  BrushRef                   _brush;
  CavePaintingCursorsRef     _cavePaintingCursors;
  DirectDrawHCIRef           _directDrawHCI;
  DragDrawHCIRef             _dragDrawHCI;
  FishtankDepthCuesRef       _fishtankDepthCues;
  ForceHybridDrawHCIRef      _forceHybridDrawHCI;
  //ForceNetInterface*         _forceNetInterface;
  ForceOneHandDrawHCIRef     _forceOneHandDrawHCI;
  ForceReverseTapeDrawHCIRef _forceReverseTapeDrawHCI;
  ForceTapeDrawHCIRef        _forceTapeDrawHCI;
  HCIMgrRef                  _hciMgr;
  HistoryRef                 _history;
  ReverseTapeDrawHCIRef      _reverseTapeDrawHCI;
  TapeDrawHCIRef             _tapeDrawHCI;


  DrawingDisablerRef               _drawingDisabler;

private:
  DrawingStudy(const DrawingStudy &);
  DrawingStudy& operator=(const DrawingStudy &);
};

} // end namespace

#endif



