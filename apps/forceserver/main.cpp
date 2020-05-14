/**
 * \author Daniel Keefe (dfk)
 *
 * \file  main.cpp
 * \brief ForceServer program for haptic freeform drawing
 *
 */

#include <DrawOnAir.H>
#include "ForceServer.H"
#include "ICubeXDevice.H"


int main(int argc, char** argv) 
{
  cout << "Starting Force Server." << endl;
  Log *log = new Log("log.txt");

  GWindow::Settings settings;
  settings.width           = 800;
  settings.height          = 800;
  settings.x               = 0;
  settings.y               = 0;
  settings.center          = false;

  GWindow *gwindow = Win32Window::create(settings);
  RenderDevice *rd = new RenderDevice();
  rd->init(gwindow);
  rd->resetState();
  
  ConfigValMap::map = new ConfigMap(argc, argv, log, false);
  
  EventMgrRef eventMgr = new EventMgr(log);
  Array<InputDevice*> inputDevices;
  

  if (argc >= 2) {
    // These are old setups used at Brown:
    if (std::string(argv[1]) == "cit411") {
      VRApp::setupInputDevicesFromConfigFile(VRApp::findVRG3DDataFile("cit411-devices.cfg"), log, inputDevices);
    }
    else if (std::string(argv[1]) == "cit304") {
      VRApp::setupInputDevicesFromConfigFile(VRApp::findVRG3DDataFile("cit304-with-phantom-devices.cfg"), log, inputDevices);
      if (ConfigVal("UsePressureDevice", 1, false)) {
        //ICubeXDevice::printMidiDevices();
        inputDevices.append(new ICubeXDevice("1", 20.0, 0, 1, ICubeXDevice::STAND_ALONE_MODE, false));
      }
    }

    // Default to using the UMN setup for the Samsung 3D display
    else {
      VRApp::setupInputDevicesFromConfigFile(VRApp::findVRG3DDataFile("samsung-devices.cfg"), log, inputDevices);
    }
  }



  double updateRate = 1.0/ConfigVal("UpdateHertz", 60.0, false);
  ForceServer fserver(splitStringIntoArray("Hand_Tracker"), 
      splitStringIntoArray("Hand_Btn_down"),
      splitStringIntoArray("Hand_Btn_up"),
      splitStringIntoArray(ConfigVal("HeadTracker","Head_Tracker")), 
      eventMgr, gwindow, updateRate);

  double sleepTime = ConfigVal("SleepTime", 0.0, false);

  DisplayTile myTile;
  Vector2 mousePos;
  while (1) {
    Array<EventRef> events;
    for (int i=0;i<inputDevices.size();i++) {
      inputDevices[i]->pollForInput(events);
    }
    Array<GEvent> gEvents;
    VRApp::pollWindowForGEvents(rd, gEvents);
    //VRApp::guiProcessGEvents(gEvents, events);
    VRApp::appendGEventsToEvents(rd, myTile, events, gEvents, mousePos, 1);
    for (int i=0;i<events.size();i++) {
      //cout << events[i]->getName() << endl;
      if (events[i]->getName() == "kbd_ESC_down") {
        exit(0);
      }
      eventMgr->queueEvent(events[i]);
    }
    eventMgr->processEventQueue();

    rd->beginFrame();
    rd->clear(true, true, true);

    fserver.mainloop();
    
    rd->endFrame();

    if (sleepTime != 0.0) {
      Sleep(sleepTime);
    }
  }

  return 0;
}

