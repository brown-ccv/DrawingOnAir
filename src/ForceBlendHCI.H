/**
 * /author Dan Keefe (dfk)
 *
 * /file  ForceBlendHCI.H
 * /brief 
 *
 */ 


#ifndef FORCEBLENDHCI_H
#define FORCEBLENDHCI_H

//#include "VRBase.H"
#include <EventMgr.H>
#include <GfxMgr.H>
#include "BrushState.H"
#include "CavePaintingCursors.H"
#include "ForceNetInterface.H"
#include "HCIMgr.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class ForceBlendHCI> ForceBlendHCIRef;
/**
*/
class ForceBlendHCI : public StylusHCI
{
public:
  ForceBlendHCI(G3D::Array<std::string>     brushOnTriggers,
             G3D::Array<std::string>     brushMotionTriggers, 
             G3D::Array<std::string>     brushOffTriggers,
             G3D::Array<std::string>     handMotionTriggers,
             BrushRef               brush,
             CavePaintingCursorsRef cursors,
             //ForceNetInterface*     forceNetInterface,
             EventMgrRef            eventMgr, 
             GfxMgrRef              gfxMgr);
  virtual ~ForceBlendHCI();

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

  void toggleGuide(MinVR::EventRef e);


  virtual void pose(G3D::Array<G3D::PosedModel::Ref>& posedModels, const G3D::CoordinateFrame& frame);
  virtual void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame& frame);

private:
  ForceBlendHCI(const ForceBlendHCI &);
  ForceBlendHCI& operator=(const ForceBlendHCI &);

  void setup();
  void updateWeights(G3D::Vector3 brushPos);
  void updateRulings();

  bool                   _enabled;
  BrushRef               _brush;
  CavePaintingCursorsRef _cursors;
  //ForceNetInterface*     _forceNetInterface;
  FsaRef                 _fsa;
  EventMgrRef            _eventMgr;
  GfxMgrRef              _gfxMgr;
  int                    _poseID, _drawID;

  bool                      _guideOn;
  G3D::Vector3                   _handPos;
  G3D::Array< G3D::Array< G3D::Vector3 > > _pts;
  G3D::Array< G3D::Array< G3D::Vector3 > > _defaultPos;
  G3D::Array< G3D::Array< double > >  _fixed;
};


} // end namespace

#endif

