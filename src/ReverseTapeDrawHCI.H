/**
 * \author Dan Keefe (dfk)
 *
 * \file  ReverseTapeDrawHCI.H
 * \brief
 *
 */


#ifndef REVERSETAPEDRAWHCI_H
#define REVERSETAPEDRAWHCI_H

//#include "VRBase.H"
#include <GfxMgr.H>
#include <EventMgr.H>
#include "BrushState.H"
#include "CavePaintingCursors.H"
#include "HCIMgr.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class ReverseTapeDrawHCI> ReverseTapeDrawHCIRef;
/**
*/
class ReverseTapeDrawHCI : public StylusHCI
{
public:
  ReverseTapeDrawHCI(G3D::Array<std::string>     brushOnTriggers,
                     G3D::Array<std::string>     brushMotionTriggers, 
                     G3D::Array<std::string>     brushOffTriggers,
                     G3D::Array<std::string>     handMotionTriggers,
                     BrushRef               brush,
                     CavePaintingCursorsRef cursors,
                     MinVR::EventMgrRef            eventMgr,
    MinVR::GfxMgrRef              gfxMgr);
  virtual ~ReverseTapeDrawHCI();

  void setEnabled(bool b);
  bool canReleaseFocus();

  // interactor methods
  void headMotion(MinVR::EventRef e);
  void handMotion(MinVR::EventRef e);
  void brushMotion(MinVR::EventRef e);
  void brushOn(MinVR::EventRef e);
  void brushMotionWhilePainting(MinVR::EventRef e);
  void brushOff(MinVR::EventRef e);

protected:
  BrushRef               _brush;
  CavePaintingCursorsRef _cursors;
  MinVR::EventMgrRef            _eventMgr;
  MinVR::FsaRef                 _fsa;
  MinVR::GfxMgrRef              _gfxMgr;
  bool                   _enabled;
  G3D::Vector3                _handPos;

private:
  ReverseTapeDrawHCI(const ReverseTapeDrawHCI &);
  ReverseTapeDrawHCI& operator=(const ReverseTapeDrawHCI &);
};


} // end namespace
#endif
