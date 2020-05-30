/**
 * \author Dan Keefe (dfk)
 *
 * \file  ForceTapeDrawHCI.H
 * \brief
 *
 */


#ifndef FORCETAPEDRAWHCI_H
#define FORCETAPEDRAWHCI_H

//#include "VRBase.H"
#include <EventMgr.H>
#include <GfxMgr.H>
#include "BrushState.H"
#include "CavePaintingCursors.H"
#include "HCIMgr.H"
//#include "ForceNetInterface.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class ForceTapeDrawHCI> ForceTapeDrawHCIRef;
/**
*/
class ForceTapeDrawHCI : public StylusHCI
{
public:
  ForceTapeDrawHCI(G3D::Array<std::string>     brushOnTriggers,
                   G3D::Array<std::string>     brushMotionTriggers, 
                   G3D::Array<std::string>     brushOffTriggers,
                   G3D::Array<std::string>     handMotionTriggers,
                   BrushRef               brush,
                   CavePaintingCursorsRef cursors,
                   //ForceNetInterface*     forceNetInterface,
                   EventMgrRef            eventMgr,
    GfxMgrRef              gfxMgr);
  virtual ~ForceTapeDrawHCI();

  virtual void setEnabled(bool b);
  bool canReleaseFocus();

  // interactor methods
  void brushMotion(MinVR::EventRef e);
  void brushOn(MinVR::EventRef e);
  void brushPressureChange(MinVR::EventRef e);
  void brushPhysicalFrameChange(MinVR::EventRef e);
  void brushDrawMotion(MinVR::EventRef e);
  void resizeFromBrushBackup(MinVR::EventRef e);
  void addSample(MinVR::EventRef e);
  void brushOff(MinVR::EventRef e);

  void handMotion(MinVR::EventRef e);
  void headMotion(MinVR::EventRef e);

  virtual void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);

protected:

  virtual void smoothOutRedrawEdits();
  virtual void smoothPressure();

  BrushRef               _brush;
  CavePaintingCursorsRef _cursors;
  EventMgrRef            _eventMgr;
  FsaRef                 _fsa;
  GfxMgrRef              _gfxMgr;
  MarkRef                _backupMark;
  bool                   _enabled;
  //ForceNetInterface     *_forceNetInterface;
  int                    _redrawCutoff;
  int                    _redrawBlendStart;
  int                    _dcbid;
  G3D::Vector3                _handPos;

  G3D::Array< G3D::Array< G3D::Vector3 > > _redrawLines;

private:
  ForceTapeDrawHCI(const ForceTapeDrawHCI &);
  ForceTapeDrawHCI& operator=(const ForceTapeDrawHCI &);
};


} // end namespace

#endif
