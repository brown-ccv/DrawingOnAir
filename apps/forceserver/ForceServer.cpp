
#include "ForceServer.H"
#include "SoundSDL.H"
#include "SoundOpenAL.H"


// OpenHaptics includes
#include <HD/hd.h>
#include <HL/hl.h>
//#include <HDU/hduError.h>
//#include <HLU/hlu.h>


void HLCALLBACK 
button1DownCallback(HLenum event, HLuint object, HLenum thread, HLcache *cache, void *userdata) 
{
  ((ForceServer*)userdata)->phantomBtnDown();
  //cout << "button down" << endl;
}

void HLCALLBACK 
button1UpCallback(HLenum event, HLuint object, HLenum thread, HLcache *cache, void *userdata) 
{
  ((ForceServer*)userdata)->phantomBtnUp();
  //cout << "button up" << endl;
}




ForceServer::ForceServer(Array<std::string> handMotionTriggers,                          
                         Array<std::string> handBtnDownTriggers,
                         Array<std::string> handBtnUpTriggers,
                         Array<std::string> headMotionTriggers,
                         EventMgrRef eventMgr, 
                         GWindow *gwindow,
                         double networkEventsUpdateRate, 
                         int port)
{
  _eventMgr = eventMgr;
  _updateRate = networkEventsUpdateRate;
  _lastHandUpdate = -1.0;
  _lastHeadUpdate = -1.0;
  _lastPhantomUpdate = -1.0;
  _headFrame = CoordinateFrame(Vector3(0,0,1));
  _handOffset = Vector3::zero();
  _planeEffectOn = false;
  _gwindow = gwindow;
  _phantomBtnPressed = false;
  _treatPressureAsABtn = true;

  initGraphics();
  initHaptics();

  // Network Startup
  _networkDevice = NetworkDevice::instance();
  _listener = NetListener::create(port);
  NetAddress myAddress(_networkDevice->localHostName(), port);
  cout << "Force Server running on " << _networkDevice->localHostName() 
       << " on port " << port << " a.k.a. " << myAddress.toString() << endl;


  // Fsa for grabbing tracker movement
  _fsa = new Fsa("ForceServer_LeftHand");
  _fsa->addState("Start");
  _fsa->addArc("HandMove", "Start", "Start", handMotionTriggers);
  _fsa->addArcCallback("HandMove", this, &ForceServer::leftHandMovement);
  _fsa->addArc("HandBtnDown", "Start", "Start", handBtnDownTriggers);
  _fsa->addArcCallback("HandBtnDown", this, &ForceServer::handBtnDown);
  _fsa->addArc("HandBtnUp", "Start", "Start", handBtnUpTriggers);
  _fsa->addArcCallback("HandBtnUp", this, &ForceServer::handBtnUp);
  _fsa->addArc("HeadMove", "Start", "Start", headMotionTriggers);
  _fsa->addArcCallback("HeadMove", this, &ForceServer::headMovement);

  if (ConfigVal("UseTapeDrawingProp", 1, false)) {
    _fsa->addArc("GrabTapeProp", "Start", "Start", splitStringIntoArray("Buttons_TapeProp_down"));
    _fsa->addArcCallback("GrabTapeProp", this, &ForceServer::tapeDrawingPropPickup);
    _fsa->addArc("DropTapeProp", "Start", "Start", splitStringIntoArray("Buttons_TapeProp_up"));
    _fsa->addArcCallback("DropTapeProp", this, &ForceServer::tapeDrawingPropDrop);
  }

  if (ConfigVal("UseMouseForPressure", 0, false)) {
    _fsa->addArc("MouseMove", "Start", "Start", splitStringIntoArray("Mouse_Pointer"));
    _fsa->addArcCallback("MouseMove", this, &ForceServer::mouseMove);
  }

  if (ConfigVal("UsePressureDevice", 1, false)) {
    std::string pressureEvent = "ICubeX_0" + intToString(ConfigVal("ICubeXSensor", 1, false));
    _fsa->addArc("PressureUpdate", "Start", "Start", splitStringIntoArray(pressureEvent));
    _fsa->addArcCallback("PressureUpdate", this, &ForceServer::pressureDeviceUpdate);
  }

  if (ConfigVal("UseTNGForPressure", 1, false)) {
    std::string pressureEvent = ConfigVal("BrushPressureEvent", "TNG_An1", false);
    _fsa->addArc("PressureUpdate", "Start", "Start", splitStringIntoArray(pressureEvent));
    _fsa->addArcCallback("PressureUpdate", this, &ForceServer::pressureDeviceUpdate);
  }

  _eventMgr->addFsaRef(_fsa);


#ifdef USE_OPENAL
  _sound = new SoundOpenAL();
#elif USE_SDL
  _sound = new SoundSDL();
#else 
  _sound = new SoundBase();
#endif
}

ForceServer::~ForceServer()
{
  hlMakeCurrent(NULL);
  hlDeleteContext(_hHLRC);
  hdDisableDevice(_hHD);
}

void
ForceServer::initGraphics()
{
  // This simple projection makes it so that you can see the phantom
  // tip position anywhere that it moves.. note, if the tip goes off
  // screen, then the haptics won't render.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-3.0, 3.0, -3.0, 3.0, -3.0, 3.0);
}

void
ForceServer::initHaptics()
{
  // Create a haptic device instance.
  HDErrorInfo error;
  _hHD = hdInitDevice(HD_DEFAULT_DEVICE);
  if (HD_DEVICE_ERROR(error = hdGetError())) {
    cerr << "Failed to initialize haptic device." << endl;
    exit(-1);
  }
  if (HD_SUCCESS != hdGetError().errorCode) {
    fprintf(stderr, "Erorr initializing haptic device.\n");
    exit(-1);
  }
  
  // Create a haptic rendering context and activate it.
  _hHLRC = hlCreateContext(_hHD);
  hlMakeCurrent(_hHLRC);

  hlAddEventCallback(HL_EVENT_1BUTTONDOWN, HL_OBJECT_ANY, HL_CLIENT_THREAD, &button1DownCallback, this);
  hlAddEventCallback(HL_EVENT_1BUTTONUP, HL_OBJECT_ANY, HL_CLIENT_THREAD, &button1UpCallback, this);
  //hlAddEventCallback(HL_EVENT_MOTION, HL_OBJECT_ANY, HL_CLIENT_THREAD, &motionCallback, this);

  hlDisable(HL_USE_GL_MODELVIEW);

  setupWorkspace(ConfigVal("WorkspaceScale", 1.0, false));

  // Plane constraint effect
  _planeShapeId = hlGenShapes(1);

  // Ambient viscous effect
  _viscousEffect = hlGenEffects(1);
  _viscousGain = ConfigVal("Viscous_InitialGain", 0.8, false);
  // Ambient friction effect
  _frictionEffect = hlGenEffects(1);
  hlBeginFrame();
  startFrictionAndViscous();
  hlEndFrame();


  // Create specific drawing effects..
  Vector3 pt = ConfigVal("Anisotropic_PhantomThresh", Vector3(0.001, 0.001, 0.001), false);
  Vector3 ht = ConfigVal("Anisotropic_HandThresh", Vector3(0.001, 0.001, 0.001), false);
  _aniFilterEffect = new AnisotropicFilterEffect(_updateRate, pt, ht);
  _tapeDrawingEffect = new TapeDrawingEffect(_updateRate);
  _oneHandDrawingEffect = new OneHandDrawingEffect(_updateRate);
  _hybridDrawingEffect = new HybridDrawingEffect(_updateRate);
}

void
ForceServer::setupWorkspace(double uniformScale)
{
  // For the Phantom Premium A 1.0, units in mm
  // Max reported extents       = -260, -106, -61, 260, 400, 119
  // Reasonable working volume  = -130,  -70, -40, 130, 190,  90
 
  // Map this working volume to the world coordinates specified
  hlMatrixMode(HL_TOUCHWORKSPACE);
  hlLoadIdentity();

  // Make the origin the center of the reasonable working volume
  //hlTranslated(0.0, 60.0, 25.0);
  // Convert feet to mm 
  hlScaled(304.8, 304.8, 304.8);

  // Apply a uniform scale to the working volume
  hlScaled(1.0/uniformScale, 1.0/uniformScale, 1.0/uniformScale);
}



void
ForceServer::processNewMessage()
{
  if (_conduit->waitingMessageType() == ForceNetInterface::StopForces) {
    ForceNetMsg_StopForces m;
    _conduit->receive(m);
    _activeEffect = NULL;
    _treatPressureAsABtn = true;
  }
  else if (_conduit->waitingMessageType() == ForceNetInterface::AnisotropicFilter) {
    ForceNetMsg_AnisotropicFilter m;
    _conduit->receive(m);
    _activeEffect = _aniFilterEffect;
    _treatPressureAsABtn = true;
  }
  else if (_conduit->waitingMessageType() == ForceNetInterface::TapeDrawing) {
    ForceNetMsg_TapeDrawing m(false);
    _conduit->receive(m);
    _activeEffect = _tapeDrawingEffect;
    _tapeDrawingEffect->setReverse(false);
    _treatPressureAsABtn = true;
  }
  else if (_conduit->waitingMessageType() == ForceNetInterface::ReverseTapeDrawing) {
    ForceNetMsg_ReverseTapeDrawing m;
    _conduit->receive(m);
    _activeEffect = _tapeDrawingEffect;
    _tapeDrawingEffect->setReverse(true);
    _treatPressureAsABtn = false;
  }
  else if (_conduit->waitingMessageType() == ForceNetInterface::OneHandDrawing) {
    ForceNetMsg_OneHandDrawing m;
    _conduit->receive(m);
    _activeEffect = _oneHandDrawingEffect;
    _treatPressureAsABtn = false;
  }
  else if (_conduit->waitingMessageType() == ForceNetInterface::HybridDrawing) {
    ForceNetMsg_HybridDrawing m;
    _conduit->receive(m);
    _activeEffect = _hybridDrawingEffect;
    _treatPressureAsABtn = true;
  }
  else if (_conduit->waitingMessageType() == ForceNetInterface::DynamicDraggingCurv) {
    ForceNetMsg_DynamicDraggingCurv m;
    _conduit->receive(m);
    _hybridDrawingEffect->_useDynamicDraggingCurv = !_hybridDrawingEffect->_useDynamicDraggingCurv;
  }
  else if (_conduit->waitingMessageType() == ForceNetInterface::HandOffset) {
    ForceNetMsg_HandOffset m(Vector3::zero());
    _conduit->receive(m);
    _handOffset = m.handOffset;
  }
  else if (_conduit->waitingMessageType() == ForceNetInterface::PlaneEffect) {
    ForceNetMsg_PlaneEffect m;
    _conduit->receive(m);
    _planeEffectOn = true;
    _planeEffectStartTime = System::time();
    _planeEffectInitialZ = _phantomFrame.translation[2];
    hlBeginFrame();
    stopFrictionAndViscous();
    hlEndFrame();
  }
  else if (_conduit->waitingMessageType() == ForceNetInterface::StopPlaneEffect) {
    ForceNetMsg_StopPlaneEffect m;
    _conduit->receive(m);
    _planeEffectOn = false;
    hlBeginFrame();
    startFrictionAndViscous();
    hlEndFrame();
  }
  else if (_conduit->waitingMessageType() == ForceNetInterface::ViscousEffect) {
    ForceNetMsg_ViscousEffect m(_viscousGain);
    _conduit->receive(m);
    _viscousGain = m.gain;
  }
  else if (_conduit->waitingMessageType() == ForceNetInterface::FrictionEffect) {
    ForceNetMsg_FrictionEffect m(0);
    _conduit->receive(m);
    hlBeginFrame();
    if (!m.onoff) {
      stopFrictionAndViscous();
    }
    else {
      startFrictionAndViscous();
    }
    hlEndFrame();
  }
  else if (_conduit->waitingMessageType() == ForceNetInterface::PlaySound) {
    ForceNetMsg_PlaySound m("dummy");
    _conduit->receive(m);
    _sound->play(m.filename);
  }
  else if (_conduit->waitingMessageType() == ForceNetInterface::Shutdown) {
    ForceNetMsg_Shutdown m;
    _conduit->receive(m);
    exit(0);
  }
  else {
    cerr << "Received a message of unknown type, ignoring it." << endl;
  }
}


void
ForceServer::tapeDrawingPropPickup(EventRef e)
{
  if (_conduit.notNull()) {
    EventNetMsg m = EventNetMsg(new Event("ForceTapeDrawingMode"));
    _conduit->send(m.type(), m);
  }
}

void
ForceServer::tapeDrawingPropDrop(EventRef e)
{
  if (_conduit.notNull()) {
    EventNetMsg m = EventNetMsg(new Event("PaintingMode")); 
    _conduit->send(m.type(), m);
  }
}

void
ForceServer::mouseMove(EventRef e)
{
  if (_activeEffect.notNull()) {
    _activeEffect->mouseMovement(e->get2DData());
  }
}

void
ForceServer::pressureDeviceUpdate(EventRef e)
{
  // Handle treating the pressure at a btn event in here and remap pressure to 0 -> 1 range

  double min = ConfigVal("Pressure_Min", 0.05, false);
  double max = ConfigVal("Pressure_Max", 1.0, false);
  
  double pressure = clamp((e->get1DData() - min) / (max - min), 0.0, 1.0);
  // rescale so that the minimum pressure is > 0 so we get a mark of
  // non-zero width all the time.
  pressure = (pressure + 0.1) / 1.1;

  cout << "pressure -- raw:" << e->get1DData() << " value: " << pressure << endl;

  if ((_activeEffect.notNull()) && (!_planeEffectOn)) {
    _activeEffect->pressureSensorUpdate(pressure);
  }
  else if (_conduit.notNull()) {
    EventNetMsg m = EventNetMsg(new Event("Brush_Pressure", pressure)); 
    _conduit->send(m.type(), m);
  }

  if (_treatPressureAsABtn) {
    if ((!_phantomBtnPressed) && (e->get1DData() > min)) {
      phantomBtnDown();
    }
    else if ((_phantomBtnPressed) && (e->get1DData() < min)) {
      phantomBtnUp();
    }
  }
}

void
ForceServer::phantomBtnDown()
{
  if (!_phantomBtnPressed) {
    _sound->play("$(G)/share/sounds/click.wav");

    _phantomBtnPressed = true;
    _planeInitPoint = _phantomFrame.translation;
    
    if ((_activeEffect.notNull()) && (!_planeEffectOn)) {
      _activeEffect->phantomBtnDown();
    }
    else if (_conduit.notNull()) {
      EventNetMsg m = EventNetMsg(new Event("Brush_Btn_down")); 
      _conduit->send(m.type(), m);
    }
  }
}


void
ForceServer::phantomBtnUp()
{
  if (_phantomBtnPressed) {
    _sound->play("$(G)/share/sounds/click.wav");

    _phantomBtnPressed = false;
    if ((_activeEffect.notNull()) && (!_planeEffectOn)) {
      _activeEffect->phantomBtnUp();
    }
    else if (_conduit.notNull()) {
      EventNetMsg m = EventNetMsg(new Event("Brush_Btn_up")); 
      _conduit->send(m.type(), m);
    }
  }
}

void
ForceServer::leftHandMovement(EventRef e)
{
  _handFrame = e->getCoordinateFrameData();
  double s = ConfigVal("WorkspaceScale", 1.0, false);
  _handFrame.translation = s*_handFrame.translation + _handOffset;

  if (_activeEffect.notNull()) {
    _activeEffect->handMovement(_handFrame);
  }
  else if (_conduit.notNull()) {
    double now = System::time();
    if (now - _lastHandUpdate >= _updateRate) {
      _lastHandUpdate = now;
      EventNetMsg m = EventNetMsg(new Event("Hand_Tracker", _handFrame));
      _conduit->send(m.type(), m);
                     
    }
  }
}

void
ForceServer::handBtnDown(EventRef e)
{
  if (_activeEffect.notNull()) {
    _activeEffect->handBtnDown();
  }
  else if (_conduit.notNull()) {
    EventNetMsg m = EventNetMsg(new Event("Hand_Btn_down"));
    _conduit->send(m.type(), m);
  }
}

void
ForceServer::handBtnUp(EventRef e)
{
  if (_activeEffect.notNull()) {
    _activeEffect->handBtnUp();
  }
  else if (_conduit.notNull()) {
    EventNetMsg m = EventNetMsg(new Event("Hand_Btn_up"));
    _conduit->send(m.type(), m);
  }
}

void
ForceServer::headMovement(EventRef e)
{
  if (_activeEffect.notNull()) {
    _activeEffect->headMovement(e->getCoordinateFrameData());
  }
  else if (_conduit.notNull()) {
    double now = System::time();
    if (now - _lastHeadUpdate >= _updateRate) {
      _lastHeadUpdate = now;
      EventNetMsg m = EventNetMsg(new Event("Head_Tracker", e->getCoordinateFrameData()));
      _conduit->send(m.type(), m);
    }
  }
}


void
ForceServer::startFrictionAndViscous()
{
  cout << "frict start" << endl;
  hlEffectd(HL_EFFECT_PROPERTY_GAIN, ConfigVal("Friction_Gain", 0.15, false));
  hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, ConfigVal("Friction_MagnitudeCap", 0.075, false));
  hlStartEffect(HL_EFFECT_FRICTION, _frictionEffect);

  cout << "visc start" << endl;
  hlEffectd(HL_EFFECT_PROPERTY_GAIN, _viscousGain);
  hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, ConfigVal("Viscous_MagnitudeCap", 1.0, false));
  hlStartEffect(HL_EFFECT_VISCOUS, _viscousEffect);
}

void
ForceServer::stopFrictionAndViscous()
{
  cout << "frict stop" << endl;
  hlStopEffect(_frictionEffect);
  cout << "visc stop" << endl;
  hlStopEffect(_viscousEffect);
}

void
ForceServer::applyPlaneEffectHaptics()
{
  // ramp up the constraint over rampTime seconds
  const double rampTime = ConfigVal("PlaneEffectRampTime", 0.25, false);
  double z = 0.0;
  double now = System::time();
  double elapsed = now - _planeEffectStartTime;
  if (elapsed < rampTime) {
    double a = clamp(1.0 - elapsed/rampTime, 0.0, 1.0);
    z = _planeEffectInitialZ * a;
  }

  hlTouchModel(HL_CONSTRAINT);
  hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, _planeShapeId);
  hlMaterialf(HL_FRONT, HL_STIFFNESS, ConfigVal("PlaneEffect_Stiffness", 0.9, false));
  hlMaterialf(HL_FRONT, HL_STATIC_FRICTION, 0);
  hlMaterialf(HL_FRONT, HL_DYNAMIC_FRICTION, 0);
  glBegin(GL_QUADS);

  double w = 3.0;
  double h = 3.0;

  glVertex3d(-w, -h, z);
  glVertex3d( w, -h, z);
  glVertex3d( w,  h, z);
  glVertex3d(-w,  h, z);

  glEnd();
  hlEndShape();
}


void 
ForceServer::renderFrame()
{
  hlBeginFrame();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  hlEffectd(HL_EFFECT_PROPERTY_GAIN, _viscousGain);
  hlUpdateEffect(_viscousEffect);

  if (_planeEffectOn) {
    applyPlaneEffectHaptics();
  }

  // Render Drawing Effects
  if (_activeEffect.notNull()) {
    _activeEffect->renderHaptics();
  }

  hlEndFrame();

  // draw debugging gfx
  if (_activeEffect.notNull()) {
    _activeEffect->renderGraphics();
  }
  
  glColor3f(1.0, 0.0, 0.0);
  glPointSize(5.0);
  glBegin(GL_POINTS);
  glVertex3d(_phantomFrame.translation[0], _phantomFrame.translation[1], _phantomFrame.translation[2]);
  glEnd();
  glColor3f(1.0, 1.0, 0.0);
  glBegin(GL_POINTS);
  glVertex3d(_handFrame.translation[0], _handFrame.translation[1], _handFrame.translation[2]);
  glEnd();

  // Check for any haptic errors.
  HLerror error;
  while (HL_ERROR(error = hlGetError())) {
    fprintf(stderr, "HL Error: %s\n", error.errorCode);      
    if (error.errorCode == HL_DEVICE_ERROR) {
      //hduPrintError(stderr, &error.errorInfo, "Error during haptic rendering\n");
      cerr << "Error during haptic rendering" << endl;
    }
  }
}


void
ForceServer::mainloop()
{
  // Network: listen for a new connection and process any existing messages
  if (_listener->clientWaiting()) {
    _conduit = _listener->waitForConnection();
    cout << "Connected to: " << _conduit->address().toString() << endl;
  }
  if (_conduit.notNull()) {
    if (!_conduit->ok()) {
      // if the conduit is not ok, then probably disconnected, set to NULL and 
      // wait for the next connection
      cout << "Dropped connection." << endl;
      _conduit = NULL;
    }
    else {
      // respond to any waiting messages
      while (_conduit->waitingMessageType() != ForceNetInterface::NO_MSG) {
        processNewMessage();
      }
    }
  }


  // Call phantom button event callbacks if they have been triggered
  hlCheckEvents();

  // Update latest position of the phantom
  HDdouble trans[16];
  hlGetDoublev(HL_DEVICE_TRANSFORM, trans);
  Matrix3 rot(trans[0],trans[4],trans[8],
              trans[1],trans[5],trans[9],
              trans[2],trans[6],trans[10]);
  Vector3 pos(trans[12],trans[13],trans[14]);
  CoordinateFrame pf = CoordinateFrame(rot, pos);
  if (pf != _phantomFrame) {
    _phantomFrame = pf;
    if (_activeEffect.notNull()) {
      _activeEffect->phantomMovement(_phantomFrame);
    }
    else if (_conduit.notNull()) {
      double now = System::time();
      if (now - _lastPhantomUpdate >= _updateRate) {
        _lastPhantomUpdate = now;
        EventNetMsg m1 = EventNetMsg(new Event("Brush_Tracker", _phantomFrame));
        _conduit->send(m1.type(), m1);
        EventNetMsg m2 = EventNetMsg(new Event("Brush_Physical_Tracker", _phantomFrame));
        _conduit->send(m2.type(), m2);
      }
    }
  }

  // Calculate forces, render haptics and graphics
  renderFrame();

  // listen for new connection
  if (_listener->clientWaiting()) {
    _conduit = _listener->waitForConnection();
    cout << "Connected to: " << _conduit->address().toString() << endl;
  }
  if (_conduit.notNull()) {
    if (!_conduit->ok()) {
      // if the conduit is not ok, then probably disconnected, set to NULL and 
      // wait for the next connection
      cout << "Dropped connection." << endl;
      _conduit = NULL;
      _activeEffect = NULL;
      _planeEffectOn = false;
    }
    else {
      // When there is no _activeEffect the main tracker and btn
      // events are sent over to the client automatically, but when
      // one is active, they are not so the ForceEffect can have the
      // chance to trap them and modify them as needed.  So, we ask
      // here for an updated set of events.
      if (_activeEffect.notNull()) {
        Array<EventRef> events;
        _activeEffect->getEventsForClient(events);
        EventBufferNetMsg m = EventBufferNetMsg(events);
        _conduit->send(m.type(), m);
      }
      
      // respond to any waiting messages
      while (_conduit->waitingMessageType() != ForceNetInterface::NO_MSG) {
        processNewMessage();
      }
    }
  }
}


