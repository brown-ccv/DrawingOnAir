/**
 * \author Daniel Keefe (dfk)
 *
 * \file  OneHandDrawingEffect.H
 * \brief 
 *
 */

#ifndef ONEHANDDRAWINGEFFECT_H
#define ONEHANDDRAWINGEFFECT_H

#include "DrawOnAir.H"
#include "ForceEffect.H"

// OpenHaptics includes
#include <HD/hd.h>
#include <HL/hl.h>

typedef ReferenceCountedPointer<class OneHandDrawingEffect> OneHandDrawingEffectRef;
/** 
*/
class OneHandDrawingEffect : public ForceEffect
{
public:
  OneHandDrawingEffect(double eventUpdateRate);
  virtual ~OneHandDrawingEffect();

  // Called to update the controller based on input
  virtual void phantomBtnDown();
  virtual void phantomBtnUp();
  virtual void phantomMovement(CoordinateFrame newFrame);
  virtual void pressureSensorUpdate(double newValue);
  
  // Called once per frame
  virtual void renderHaptics();
  virtual void renderGraphics();

private:
  CoordinateFrame _phantomFrame;
  Vector3         _lastPointOnPath;
  Vector3         _endPt;
  bool            _painting;
  HLuint          _lineShapeId;
  double          _lineShapeSnapDistance;
};


#endif
