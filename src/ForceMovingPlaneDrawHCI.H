/**
 * \author Dan Keefe (dfk)
 *
 * \file  ForceMovingPlaneDrawHCI.H
 * \brief
 *
 */

#include <GfxMgr.H>

#ifndef FORCEMOVINGPLANEDRAWHCI_H
#define FORCEMOVINGPLANEDRAWHCI_H

//#include "VRBase.H"

#include "BrushState.H"
#include "CavePaintingCursors.H"
#include "ForceNetInterface.H"
#include "HCIMgr.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class ForceMovingPlaneDrawHCI> ForceMovingPlaneDrawHCIRef;
/**
*/
class ForceMovingPlaneDrawHCI : public StylusHCI
{
public:
  ForceMovingPlaneDrawHCI(G3D::Array<std::string>     brushOnTriggers,
                      G3D::Array<std::string>     brushMotionTriggers, 
                      G3D::Array<std::string>     brushOffTriggers,
                      G3D::Array<std::string>     handMotionTriggers,
                      BrushRef          brush,
                      CavePaintingCursorsRef cursors,
                      //ForceNetInterface*     forceNetInterface,
                      MinVR::EventMgrRef            eventMgr,
                      MinVR::GfxMgrRef              gfxMgr);
  virtual ~ForceMovingPlaneDrawHCI();
  
  void setEnabled(bool b);
  bool canReleaseFocus();

  // interactor methods
  void brushMotion(MinVR::EventRef e);
  void brushOn(MinVR::EventRef e);
  void brushPressureChange(MinVR::EventRef e);
  void brushPhysicalFrameChange(MinVR::EventRef e);
  void brushDrawMotion(MinVR::EventRef e);
  void brushOff(MinVR::EventRef e);
  void handMotion(MinVR::EventRef e);

  void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);

private:
  ForceMovingPlaneDrawHCI(const ForceMovingPlaneDrawHCI &);
  ForceMovingPlaneDrawHCI& operator=(const ForceMovingPlaneDrawHCI &);

  BrushRef          _brush;
  CavePaintingCursorsRef _cursors;
  MinVR::EventMgrRef            _eventMgr;
  MinVR::FsaRef                 _fsa;
  MinVR::GfxMgrRef              _gfxMgr;
  bool                   _enabled;
  //ForceNetInterface      *_forceNetInterface;
  int                    _drawID;
  G3D::Vector3                _planeInitPoint;
  G3D::CoordinateFrame        _handFrame;
};


} // end namespace

#endif
