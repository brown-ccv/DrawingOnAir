
#include "ReframeArtworkHCI.H"

using namespace G3D;
namespace DrawOnAir {

ReframeArtworkHCI::ReframeArtworkHCI(MinVR::EventMgrRef            eventMgr,
  MinVR::GfxMgrRef              gfxMgr,
                                     HCIMgrRef              hciMgr,
                                     CavePaintingCursorsRef cursors,
                                     Array<std::string>     tracker1Triggers,
                                     Array<std::string>     btn1UpTriggers,
                                     Array<std::string>     btn2DownTriggers,
                                     Array<std::string>     tracker2Triggers,
                                     Array<std::string>     btn2UpTriggers) :
  WidgetHCI(hciMgr)
{
  _eventMgr = eventMgr;
  _gfxMgr = gfxMgr;
  _cursors = cursors;
  _fsa = new MinVR::Fsa("ReframeArtworkHCI");
  _fsa->addState("Moving");
  _fsa->addState("Scaling");

  _fsa->addArc("Tracker1Move", "Moving", "Moving", tracker1Triggers);
  _fsa->addArcCallback("Tracker1Move", this, &ReframeArtworkHCI::tracker1Move);
  _fsa->addArc("SecondGrab", "Moving", "Scaling", btn2DownTriggers);
  _fsa->addArc("ReleaseMove", "Moving", "Moving", btn1UpTriggers);
  _fsa->addArcCallback("ReleaseMove", this, &ReframeArtworkHCI::release);

  _fsa->addStateEnterCallback("Scaling", this, &ReframeArtworkHCI::startScale);
  _fsa->addArc("Tracker1Scale", "Scaling", "Scaling", tracker1Triggers);
  _fsa->addArcCallback("Tracker1Scale", this, &ReframeArtworkHCI::tracker1Scale);
  _fsa->addArc("Tracker2Scale", "Scaling", "Scaling", tracker2Triggers);
  _fsa->addArcCallback("Tracker2Scale", this, &ReframeArtworkHCI::tracker2Scale);
  _fsa->addArc("Release1Scale", "Scaling", "Moving", btn1UpTriggers);
  _fsa->addArcCallback("Release1Scale", this, &ReframeArtworkHCI::release);
  _fsa->addArc("Release2Scale", "Scaling", "Moving", btn2UpTriggers);
}

ReframeArtworkHCI::~ReframeArtworkHCI()
{
}

void
ReframeArtworkHCI::activate()
{
  _eventMgr->addFsaRef(_fsa);
  _initialGrab = true;
  _cursors->setBrushCursor(CavePaintingCursors::NO_CURSOR);
}

void
ReframeArtworkHCI::tracker1Move(MinVR::EventRef e)
{
  CoordinateFrame eRoom = e->getCoordinateFrameData();

  if (_initialGrab) {
    _initialGrab = false;
  }
  else {
    CoordinateFrame delta = eRoom * _lastFrame.inverse();
    CoordinateFrame newxform = _gfxMgr->getRoomToVirtualSpaceFrame() * delta.inverse();
    newxform.rotation.orthonormalize();
    _gfxMgr->setRoomToVirtualSpaceFrame(newxform);
    /***
    // find the change in rotation/translation in LayerSpace then
    // add this to the current Layer frame.
      ArtworkLayerRef curLayer = _brush->state->ArtworkLayer;
      CoordinateFrame eV = _gfxMgr->roomToVirtualSpace(eRoom);
      CoordinateFrame lV = _gfxMgr->roomToVirtualSpace(_lastFrame);
      CoordinateFrame eL = curLayer->virtualToLayerSpace(eV);
      CoordinateFrame lL = curLayer->virtualToLayerSpace(lV);
      CoordinateFrame dL = eL * lL.inverse();
      dL.rotation.orthonormalize();
      CoordinateFrame final = curLayer->getCoordinateFrame() * dL;
      final.rotation.orthonormalize();
      curLayer->setCoordinateFrame(final);
    ***/
  }
  _lastFrame = eRoom;
}

void
ReframeArtworkHCI::release(MinVR::EventRef e)
{  
  _eventMgr->removeFsaRef(_fsa);
  releaseFocusWithHCIMgr();
}

void
ReframeArtworkHCI::startScale(MinVR::EventRef e)
{
  _initialScale = true;
}

void
ReframeArtworkHCI::tracker1Scale(MinVR::EventRef e)
{
  CoordinateFrame eRoom = e->getCoordinateFrameData();
  _lastFrame = eRoom;
}

void
ReframeArtworkHCI::tracker2Scale(MinVR::EventRef e)
{
  // The length between the 2 hands
  double length = (e->getCoordinateFrameData().translation -
                   _lastFrame.translation).magnitude();
  
  if (_initialScale) {
    _scaleStartLength = length;
    _scaleStartScale = _gfxMgr->getRoomToVirtualSpaceScale();
    _roomScalePoint = _lastFrame.translation;
    _scalePoint = _gfxMgr->roomPointToVirtualSpace(_lastFrame.translation);
    //cout << "scale point = " << _scalePoint << " " << _roomScalePoint << endl;
    _initialScale = false;
  }
  else {
    // this is exponential growth and becomes unstable..
    //double newScale = 1.0 / (length * _scaleStartScale/_scaleStartLength);
    
    // this is more reliable
    double newScale = _gfxMgr->getRoomToVirtualSpaceScale();
    if (length < _scaleStartLength) {
      double alpha = clamp(length/_scaleStartLength, 0.0, 1.0);
      newScale = lerp(7.0*_scaleStartScale, _scaleStartScale, alpha);
    }
    else if (length > _scaleStartLength) {
      double alpha = clamp((length-_scaleStartLength) / (3.0*_scaleStartLength), 0.0, 1.0);
      newScale = lerp(_scaleStartScale, 0.001*_scaleStartScale, alpha);
    }


    if ((newScale > 0.001) && (newScale < 1000)) {
      // This will scale about the origin of VirtualSpace
      _gfxMgr->setRoomToVirtualSpaceScale(newScale);
      
      // Now, correct for making it look like we're scaling around the scalePoint
      Vector3 newScalePointPosVS = _gfxMgr->roomPointToVirtualSpace(_roomScalePoint);
      Vector3 deltaVS = _scalePoint - newScalePointPosVS;
      Vector3 deltaRS = _gfxMgr->virtualVectorToRoomSpace(deltaVS);
      if ((!deltaRS.isZero()) && (deltaRS.isFinite())) {
        _gfxMgr->setRoomToVirtualSpaceFrame(_gfxMgr->getRoomToVirtualSpaceFrame()*
                                            CoordinateFrame(deltaRS));
      }
    }
    
  }
}



} // end namespace

