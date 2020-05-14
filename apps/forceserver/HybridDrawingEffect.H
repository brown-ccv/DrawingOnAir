/**
 * \author Daniel Keefe (dfk)
 *
 * \file  HybridDrawingEffect.H
 * \brief 
 *
 */

#ifndef HYBRIDDRAWINGEFFECT_H
#define HYBRIDDRAWINGEFFECT_H

#include "DrawOnAir.H"
#include "ForceEffect.H"

// OpenHaptics includes
#include <HD/hd.h>
#include <HL/hl.h>

typedef ReferenceCountedPointer<class HybridDrawingEffect> HybridDrawingEffectRef;
/** 
*/
class HybridDrawingEffect : public ForceEffect
{
public:
  HybridDrawingEffect(double eventUpdateRate);
  virtual ~HybridDrawingEffect();

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

  void reportBrush(CoordinateFrame brush, CoordinateFrame physBrush,
                   Vector3 tangent, bool addSample);
  void startDrag();
  void startTape();

  enum HybridState {
    HybridDrag,
    HybridTape
  };
  HybridState        _hybridState;
  HybridState        _hybridStateOnBtnDown;
  CoordinateFrame    _phantomFrame;
  Vector3            _handPos;
  Array<Vector3>     _markPath;
  Array<Vector3>     _tanPath;
  Array<double>      _anglePath;
  int                _redrawClosestPtIndex;
  Vector3            _handRedrawOffset;
  Vector3            _handPreOffset;
  Vector3            _handPostOffset;
  Vector3            _brushPostOffset;

  // TapeDrawing / Drawing on Air
  enum DrawingOnAirState {
    DRAWING_FORWARD,
    FLUID_BACKUP,
    HOLDING_HAND_TOO_CLOSE
  };
  DrawingOnAirState  _drawingState;
  Vector3            _lastPointOnPath;
  Vector3            _endPt;
  bool               _painting;
  bool               _reverse;
  double             _pressureFromPushing;
  double             _pressureFromFinger;
  double             _maxFingerPressure;
  double             _totalPressure;
  HLuint             _lineShapeId;
  double             _lineShapeSnapDistance;

  // Drag Drawing
  double             _lineLength;
  double             _pathLength;
  double             _interpLineLength;
  CoordinateFrame    _dragFrame;
  Array<Vector3>     _dragHapticPath;
  Vector3            _dragLastPhantomPos;
  Queue<double>      _times;
  Queue<double>      _speeds;
  double             _dragStartTime;

  /** All tracking data (Phantom and Hand) comes in in room space and
      is coregistered.  The mark is stored, the hand and brush
      positions are reported to the drawing app, and all calculations
      are done in virtual space, allowing an offset to be applied
      between the room and virtual coordinate systems.  The hand can
      also have a virtual offset applied to its position in virtual
      space to account to smoothing constraints and the initial
      transition to tape drawing.
  */
  Vector3 _roomToVirtual;
  Vector3 _handVirtualOffset; 

  // for user studies, may want to disable tape drawing and/or backing up
  bool _disableTape;
  bool _disableBackup;

 public:
  // Added these for investigating dynamic dragging -- where length of
  // drag line changes based on curvature.
  /// Turns dynamic dragging on/off
  bool _useDynamicDraggingCurv;
};


#endif
