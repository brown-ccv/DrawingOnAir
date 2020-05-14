/**
 * /author Dan Keefe (dfk)
 *
 * /file  ReframeArtworkHCI.H
 * /brief Interaction for grabbing artwork layers and moving them around
 *
 */ 

#ifndef REFRAMEARTWORKHCI_H
#define REFRAMEARTWORKHCI_H

//#include "VRBase.H"
#include <GfxMgr.H>
#include <EventMgr.H>
#include "CavePaintingCursors.H"
#include "HCIMgr.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class ReframeArtworkHCI> ReframeArtworkHCIRef;
/** Interaction for grabbing artwork layers and moving them around.
 */
class ReframeArtworkHCI : public WidgetHCI
{
public:
  ReframeArtworkHCI(MinVR::EventMgrRef            eventMgr,
                    MinVR::GfxMgrRef              gfxMgr,
                    HCIMgrRef              hciMgr,
                    CavePaintingCursorsRef cursors,
                    G3D::Array<std::string>     tracker1Triggers,
		                G3D::Array<std::string>     btn1UpTriggers,
		                G3D::Array<std::string>     btn2DownTriggers,
                    G3D::Array<std::string>     tracker2Triggers,
		                G3D::Array<std::string>     btn2UpTriggers);
  virtual ~ReframeArtworkHCI();

  void activate();

  void tracker1Move(MinVR::EventRef e);
  void startScale(MinVR::EventRef e);
  void tracker1Scale(MinVR::EventRef e);
  void tracker2Scale(MinVR::EventRef e);
  void release(MinVR::EventRef e);

private:
  ReframeArtworkHCI(const ReframeArtworkHCI &);
  ReframeArtworkHCI& operator=(const ReframeArtworkHCI &);

  MinVR::EventMgrRef            _eventMgr;
  MinVR::FsaRef                 _fsa;
  MinVR::GfxMgrRef              _gfxMgr;
  CavePaintingCursorsRef _cursors;
  bool                   _initialGrab;
  G3D::CoordinateFrame        _lastFrame;
  bool                   _initialScale;
  double                 _scaleStartLength;
  double                 _scaleStartScale;
  G3D::Vector3                _scalePoint;
  G3D::Vector3                _roomScalePoint;
};

} // end namespace

#endif



