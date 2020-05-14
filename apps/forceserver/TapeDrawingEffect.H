/**
 * \author Daniel Keefe (dfk)
 *
 * \file  TapeDrawingEffect.H
 * \brief 
 *
 */

#ifndef TAPEDRAWINGEFFECT_H
#define TAPEDRAWINGEFFECT_H

#include "DrawOnAir.H"
#include "ForceEffect.H"

// OpenHaptics includes
#include <HD/hd.h>
#include <HL/hl.h>

typedef ReferenceCountedPointer<class TapeDrawingEffect> TapeDrawingEffectRef;
/** 
*/
class TapeDrawingEffect : public ForceEffect
{
public:
  TapeDrawingEffect(double eventUpdateRate);
  virtual ~TapeDrawingEffect();

  void setReverse(bool r) { _reverse = r; }
  void setPressureFromPushing(bool p) { _pressureFromPushing = p; }

  // Called to update the controller based on input
  virtual void phantomBtnDown();
  virtual void phantomBtnUp();
  virtual void phantomMovement(CoordinateFrame newFrame);
  virtual void handBtnDown();
  virtual void handBtnUp();
  virtual void handMovement(CoordinateFrame newFrame);
  virtual void pressureSensorUpdate(double newValue);
  
  // Called once per frame
  virtual void renderHaptics();
  virtual void renderGraphics();

private:
  CoordinateFrame _phantomFrame;
  Vector3         _handPos;
  Vector3         _lastPointOnPath;
  Vector3         _endPt;
  bool            _painting;
  HLuint          _lineShapeId;
  double          _lineShapeSnapDistance;
  bool            _reverse;
  bool            _pressureFromPushing;

  enum DrawingOnAirState {
    DRAWING_FORWARD,
    FLUID_BACKUP,
    HOLDING_HAND_TOO_CLOSE
  };
  DrawingOnAirState      _drawingState;
  Array<Vector3>         _markPath;
  Array<Vector3>         _handPath;
  int                    _redrawClosestPtIndex;
  Vector3                _handRedrawOffset;
  bool                   _disableBackup;
};


#endif
