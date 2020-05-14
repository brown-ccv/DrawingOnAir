/**
 * \author Dan Keefe (dfk)
 *
 * \file  ForceHybridDrawHCI.H
 * \brief
 *
 */


#ifndef FORCEHYBRIDDRAWHCI_H
#define FORCEHYBRIDDRAWHCI_H

//#include "VRBase.H"
#include <EventMgr.H>
#include <GfxMgr.H>
#include "BrushState.H"
#include "CavePaintingCursors.H"
#include "HCIMgr.H"
#include "ForceNetInterface.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class ForceHybridDrawHCI> ForceHybridDrawHCIRef;
/**
*/
class ForceHybridDrawHCI : public StylusHCI
{
public:
  ForceHybridDrawHCI(G3D::Array<std::string>     brushOnTriggers,
                     G3D::Array<std::string>     brushMotionTriggers, 
                     G3D::Array<std::string>     brushOffTriggers,
                     G3D::Array<std::string>     handMotionTriggers,
                     G3D::Array<std::string>     handDownTriggers,
                     G3D::Array<std::string>     handUpTriggers,
                     BrushRef               brush,
                     CavePaintingCursorsRef cursors,
                    // ForceNetInterface*     forceNetInterface,
                     MinVR::EventMgrRef            eventMgr,
    MinVR::GfxMgrRef              gfxMgr);
  virtual ~ForceHybridDrawHCI();

  virtual void setEnabled(bool b);
  bool canReleaseFocus();

  // interactor methods
  void brushMotion(MinVR::EventRef e);
  void brushOn(MinVR::EventRef e);
  void brushPressureChange(MinVR::EventRef e);
  void brushColorValueChange(MinVR::EventRef e);
  void brushPhysicalFrameChange(MinVR::EventRef e);
  void brushDrawMotion(MinVR::EventRef e);
  void resizeFromBrushBackup(MinVR::EventRef e);
  void addSample(MinVR::EventRef e);
  void reverseTape(MinVR::EventRef e);
  void brushOff(MinVR::EventRef e);


  void handMotion(MinVR::EventRef e);
  void headMotion(MinVR::EventRef e);

  void handBtnDown(MinVR::EventRef e);
  void handBtnUp(MinVR::EventRef e);

  void setTapeFirst(MinVR::EventRef e);
  void setDragFirst(MinVR::EventRef e);

  void setDragBackupLength(MinVR::EventRef e);
  void setDragLineLength(MinVR::EventRef e);

  virtual void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);

  void toggleConstantDragLengthDisplay() { 
    _dispConstLenDragLine = !_dispConstLenDragLine;
  }


protected:

  virtual void smoothOutRedrawEdits();
  virtual void smoothOutPressure();

  enum HybridState {
    TapeDrawing,
    DragDrawing
  };
  HybridState            _hybridState;
  HybridState            _hybridStateToStart;
  BrushRef               _brush;
  CavePaintingCursorsRef _cursors;
  MinVR::EventMgrRef            _eventMgr;
  MinVR::FsaRef                 _fsa;
  MinVR::GfxMgrRef              _gfxMgr;
  MarkRef                _backupMark;
  bool                   _enabled;
  //ForceNetInterface     *_forceNetInterface;
  int                    _redrawCutoff;
  int                    _redrawBlendStart;
  int                    _dcbid;
  G3D::Vector3                _handPos;
  bool                   _reverseTape;
  bool                   _handCloseToBrush;
  bool                   _inRedrawState;
  G3D::Vector3                _startDrawingPt;
  double                 _backupLength;
  double                 _lineLength;

  G3D::Array< G3D::Array< G3D::Vector3 > > _redrawLines;
  bool                   _dispConstLenDragLine;

private:
  ForceHybridDrawHCI(const ForceHybridDrawHCI &);
  ForceHybridDrawHCI& operator=(const ForceHybridDrawHCI &);
};


} // end namespace

#endif
