/**
 * \author Daniel Keefe (dfk)
 *
 * \file  ForceEffect.H
 * \brief 
 *
 */

#ifndef FORCEEFFECT_H
#define FORCEEFFECT_H

#include "DrawOnAir.H"
using namespace G3D;

typedef ReferenceCountedPointer<class ForceEffect> ForceEffectRef;
/** No more than one ForceEffect is active at a time in the
    ForceServer.  This is the base class for effects.
*/
class ForceEffect : public ReferenceCountedObject
{
public:
  ForceEffect(double eventUpdateRate);
  virtual ~ForceEffect();

  // Called to update the controller based on input
  virtual void phantomBtnDown();
  virtual void phantomBtnUp();
  virtual void phantomMovement(CoordinateFrame newFrame);
  virtual void handBtnDown();
  virtual void handBtnUp();
  virtual void handMovement(CoordinateFrame newFrame);
  virtual void headMovement(CoordinateFrame newFrame); 
  virtual void mouseMovement(Vector2 newPosition);
  virtual void pressureSensorUpdate(double newValue);
  
  // Called once per frame
  virtual void renderHaptics();
  virtual void renderGraphics();
  virtual void getEventsForClient(Array<EventRef> &events);

protected:
  double          _updateRate;
  double          _lastHandUpdate;
  double          _lastHeadUpdate;
  double          _lastPhantomUpdate;
  Array<EventRef> _eventBuffer;
};


#endif
