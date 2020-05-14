
#include "AnisotropicFilterEffect.H"
using namespace G3D;


AnisotropicFilterEffect::AnisotropicFilterEffect(double eventUpdateRate,
                                                 Vector3 phantomThreshold, 
                                                 Vector3 handThreshold) :
  ForceEffect(eventUpdateRate)
{
  _phantomThreshold = phantomThreshold;
  _handThreshold = handThreshold;
}

AnisotropicFilterEffect::~AnisotropicFilterEffect()
{
}

void 
AnisotropicFilterEffect::phantomMovement(CoordinateFrame newFrame) 
{
  Vector3 movement = newFrame.translation - _phantomPos;
  if ((fabs(movement[0]) < _phantomThreshold[0]) &&
      (fabs(movement[1]) < _phantomThreshold[1]) &&
      (fabs(movement[2]) < _phantomThreshold[2])) {
    newFrame.translation = _phantomPos;
  }
  else {
    _phantomPos = newFrame.translation;
  }
  
  double now = System::time();
  if (now - _lastPhantomUpdate >= _updateRate) {
    _lastPhantomUpdate = now;
    _eventBuffer.append(new Event("Brush_Tracker", newFrame));
    _eventBuffer.append(new Event("Brush_Physical_Tracker", newFrame));
  }
}


void 
AnisotropicFilterEffect::handMovement(CoordinateFrame newFrame) 
{
  Vector3 movement = newFrame.translation - _handPos;
  if ((fabs(movement[0]) < _handThreshold[0]) &&
      (fabs(movement[1]) < _handThreshold[1]) &&
      (fabs(movement[2]) < _handThreshold[2])) {
    newFrame.translation = _handPos;
  }
  else {
    _handPos = newFrame.translation;
  }

  double now = System::time();
  if (now - _lastHandUpdate >= _updateRate) {
    _lastHandUpdate = now;
    _eventBuffer.append(new Event("Hand_Tracker", newFrame));
  }
}


