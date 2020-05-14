/**
 * \author Daniel Keefe (dfk)
 *
 * \file  ForceServer.H
 * \brief 
 *
 */

#ifndef FORCESERVER_H
#define FORCESERVER_H

// OpenHaptics includes
//#include <HD/hd.h>
//#include <HL/hl.h>
//#include <HDU/hduError.h>
//#include <HLU/hlu.h>

#include "DrawOnAir.H"

#include "ForceEffect.H"
#include "AnisotropicFilterEffect.H"
#include "OneHandDrawingEffect.H"
#include "TapeDrawingEffect.H"
#include "HybridDrawingEffect.H"
#include "SoundBase.H"

/** Has at most one active main ForceEffect as implemented by
    subclasses of ForceEffect, and in addition has a few simple
    effects that are built in to the implementation of the server,
    including friction, viscosity, and a plane constraint.
*/
class ForceServer
{
public:

  ForceServer(Array<std::string> handMotionTriggers, 
              Array<std::string> handBtnDownTriggers,
              Array<std::string> handBtnUpTriggers,
              Array<std::string> headMotionTriggers,
              EventMgrRef eventMgr,
              GWindow *gwindow,
              double networkEventsUpdateRate,
              int port=FORCENETINTERFACE_DEFAULT_PORT);

  virtual ~ForceServer();

  void mainloop();

  void phantomBtnDown();
  void phantomBtnUp();
  void phantomMovement(const CoordinateFrame &frame);
  void handBtnDown(EventRef e);
  void handBtnUp(EventRef e);
  void leftHandMovement(EventRef e);
  void headMovement(EventRef e);
  void tapeDrawingPropPickup(EventRef e);
  void tapeDrawingPropDrop(EventRef e);
  void mouseMove(EventRef e);
  void pressureDeviceUpdate(EventRef e);

protected:

  void initHaptics();
  void initGraphics();
  void setupWorkspace(double uniformScale);
  void processNewMessage();
  void renderFrame();

  NetworkDevice*         _networkDevice;
  ReliableConduitRef     _conduit;
  NetAddress             _address;
  NetListenerRef         _listener;
  EventMgrRef            _eventMgr;
  FsaRef                 _fsa;
  GWindow*               _gwindow;
  CoordinateFrame        _phantomFrame;
  CoordinateFrame        _headFrame;
  CoordinateFrame        _handFrame;
  Vector3                _handOffset;  
  double                 _updateRate;
  double                 _lastHandUpdate;
  double                 _lastHeadUpdate;
  double                 _lastPhantomUpdate;
  HHD                    _hHD;
  HHLRC                  _hHLRC;

  bool                   _phantomBtnPressed;
  bool                   _treatPressureAsABtn;

  // Plane constraint effect
  void                   applyPlaneEffectHaptics();
  HLuint                 _planeShapeId;
  bool                   _planeEffectOn;
  Vector3                _planeInitPoint;
  double                 _planeEffectStartTime;
  double                 _planeEffectInitialZ;

  void                   startFrictionAndViscous();
  void                   stopFrictionAndViscous();

  // Viscous effect
  HLuint                 _viscousEffect;
  float                  _viscousGain;

  // Friction effect
  HLuint                 _frictionEffect;

  // Specific ForceEffect implementations
  ForceEffectRef          _activeEffect;
  AnisotropicFilterEffectRef _aniFilterEffect;
  TapeDrawingEffectRef    _tapeDrawingEffect;
  OneHandDrawingEffectRef _oneHandDrawingEffect;
  HybridDrawingEffectRef  _hybridDrawingEffect;

  SoundBaseRef            _sound;
};

#endif
