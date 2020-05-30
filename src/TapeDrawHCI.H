/**
 * \author Dan Keefe (dfk)
 *
 * \file  TapeDrawHCI.H
 * \brief
 *
 */


#ifndef TAPEDRAWHCI_H
#define TAPEDRAWHCI_H

//#include "VRBase.H"
#include <EventMgr.H>
#include <GfxMgr.H>
#include "BrushState.H"
#include "CavePaintingCursors.H"
#include "HCIMgr.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class TapeDrawHCI> TapeDrawHCIRef;
/**
*/
class TapeDrawHCI : public StylusHCI
{
public:
  TapeDrawHCI(G3D::Array<std::string>     brushOnTriggers,
              G3D::Array<std::string>     brushMotionTriggers, 
              G3D::Array<std::string>     brushOffTriggers,
              G3D::Array<std::string>     handMotionTriggers,
              BrushRef          brush,
              CavePaintingCursorsRef cursors,
              EventMgrRef            eventMgr,
    GfxMgrRef              gfxMgr);
  virtual ~TapeDrawHCI();

  void setEnabled(bool b);
  bool canReleaseFocus();

  // interactor methods
  void headMotion(MinVR::EventRef e);
  void handMotion(MinVR::EventRef e);
  void brushMotion(MinVR::EventRef e);
  void brushOn(MinVR::EventRef e);
  void brushMotionWhilePainting(MinVR::EventRef e);
  void brushOff(MinVR::EventRef e);

  void brushPressureChange(MinVR::EventRef e);

protected:
  BrushRef          _brush;
  CavePaintingCursorsRef _cursors;
  EventMgrRef            _eventMgr;
  FsaRef                 _fsa;
  GfxMgrRef              _gfxMgr;
  bool                   _enabled;
  G3D::Vector3                _handPos;
  G3D::Vector3                _lastBrushPos;

private:
  TapeDrawHCI(const TapeDrawHCI &);
  TapeDrawHCI& operator=(const TapeDrawHCI &);
};


} // end namespace
#endif
