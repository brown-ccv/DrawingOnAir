
#include "ForceEffect.H"


ForceEffect::ForceEffect(double eventUpdateRate)
{
  _updateRate = eventUpdateRate;
  _lastPhantomUpdate = -1.0;
  _lastHandUpdate = -1.0;
  _lastHeadUpdate = -1.0;
}

ForceEffect::~ForceEffect()
{
}

void
ForceEffect::phantomBtnDown()
{
  _eventBuffer.append(new Event("Brush_Btn_down"));
}

void
ForceEffect::phantomBtnUp() 
{
  _eventBuffer.append(new Event("Brush_Btn_up"));
}

void 
ForceEffect::phantomMovement(CoordinateFrame newFrame) 
{
  double now = System::time();
  if (now - _lastPhantomUpdate >= _updateRate) {
    _lastPhantomUpdate = now;
    _eventBuffer.append(new Event("Brush_Tracker", newFrame));
    _eventBuffer.append(new Event("Brush_Physical_Tracker", newFrame));
  }
}


void
ForceEffect::handBtnDown() 
{
  _eventBuffer.append(new Event("Hand_Btn_down"));
}

void 
ForceEffect::handBtnUp() 
{
  _eventBuffer.append(new Event("Hand_Btn_up"));
}

void 
ForceEffect::handMovement(CoordinateFrame newFrame) 
{
  double now = System::time();
  if (now - _lastHandUpdate >= _updateRate) {
    _lastHandUpdate = now;
    _eventBuffer.append(new Event("Hand_Tracker", newFrame));
  }
}

void 
ForceEffect::headMovement(CoordinateFrame newFrame) 
{
  double now = System::time();
  if (now - _lastHeadUpdate >= _updateRate) {
    _lastHeadUpdate = now;
    _eventBuffer.append(new Event("Head_Tracker", newFrame));
  }
}
 
void 
ForceEffect::mouseMovement(Vector2 newPosition) 
{
}

void 
ForceEffect::pressureSensorUpdate(double newValue) 
{
  _eventBuffer.append(new Event("Brush_Pressure", newValue));
}
  
void 
ForceEffect::renderHaptics() 
{
}

void 
ForceEffect::renderGraphics() 
{
}

void 
ForceEffect::getEventsForClient(Array<EventRef> &events) 
{
  /***
  cout << "events" << endl;
  for (int i=0;i<_eventBuffer.size();i++) {
    if (_eventBuffer[i]->getName() == "Hand_Tracker") {
      cout << _eventBuffer[i]->getCoordinateFrameData().translation << endl;
    }
  }
  ***/

  events.append(_eventBuffer);
  _eventBuffer.clear();
}


