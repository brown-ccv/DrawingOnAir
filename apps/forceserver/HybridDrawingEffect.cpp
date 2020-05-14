
#include "HybridDrawingEffect.H"


HybridDrawingEffect::HybridDrawingEffect(double eventUpdateRate) : ForceEffect(eventUpdateRate)
{
  _painting = false;
  _reverse = false;
  _totalPressure = 0.0;
  _pressureFromPushing = 0.0;
  _pressureFromFinger = 0.0;
  _maxFingerPressure = 1.0;
  _drawingState = DRAWING_FORWARD;
  _hybridStateOnBtnDown = HybridDrag;
  _hybridState = HybridDrag;
  _lineLength = ConfigVal("DragDrawing_LineLength", 0.15, false);
  _disableTape = ConfigVal("HybridDrawing_DisableTape", 0, false);
  _disableBackup = ConfigVal("HybridDrawing_DisableBackup", 0, false);
  _useDynamicDraggingCurv = false;

  _lineShapeId = hlGenShapes(1);
  // We can get a good approximation of the snap distance to use by
  // solving the following simple force formula:
  // >  F = k * x  <
  // F: Force in Newtons (N).
  // k: Stiffness control coefficient (N/mm).
  // x: Displacement (i.e. snap distance).
  const double kLineShapeForce = 7.0;
  HDdouble kStiffness;
  hdGetDoublev(HD_NOMINAL_MAX_STIFFNESS, &kStiffness);
  _lineShapeSnapDistance = kLineShapeForce / kStiffness;
}

HybridDrawingEffect::~HybridDrawingEffect()
{
}

void
HybridDrawingEffect::pressureSensorUpdate(double newValue)
{
  /***
      Pressure works like this..
      1. Pressure from pushing in tape drawing is always added on to
         the other pressure without any manipulation.
      2. Pressure from the finger device comes in in a 0->1 range, but
         we sometimes remap it.  For ex, if we switch from tape to
         drag while we still have some pressure from pushing, then we
         have some "carry over" pressure, the finger should be
         remapped so that whatever level it is currently at
         corresponds to the current total pressure, and when it is
         completely released, it corresponds to 0 pressure.
  ***/

  _pressureFromFinger = newValue;
  _totalPressure = _pressureFromFinger/_maxFingerPressure + _pressureFromPushing;
  _eventBuffer.append(new Event("Brush_Pressure", _totalPressure));
  _eventBuffer.append(new Event("Brush_ColorValue", clamp(_totalPressure, 0.0, 1.0)));
}


void
HybridDrawingEffect::startDrag()
{
  _hybridState = HybridDrag;
  _dragFrame = _phantomFrame;
  _dragLastPhantomPos = _phantomFrame.translation;
  _dragStartTime = System::time();
  if (_markPath.size()) {
    _dragFrame.translation = _markPath.last();
  }
  //_eventBuffer.append(new Event("Brush_ColorValue", 0.0));


  // remap so current pressure from finger matches total pressure
  if ((_pressureFromFinger == 0.0) || (_totalPressure == 0.0)) {
    _maxFingerPressure = 1.0;
  }
  else {
    _maxFingerPressure = _pressureFromFinger / _totalPressure;
  }
  _pressureFromPushing = 0.0;
}

void
HybridDrawingEffect::startTape()
{
  _dragHapticPath.clear();
  _hybridState = HybridTape;
  _drawingState = DRAWING_FORWARD;
  if (_markPath.size()) {
    _lastPointOnPath = _markPath.last();
  }
  else {
    _lastPointOnPath = _phantomFrame.translation;
  }
}


void
HybridDrawingEffect::phantomBtnDown()
{
  _painting = true;
  _markPath.clear();
  _tanPath.clear();
  _anglePath.clear();
  _pathLength = 0.0;
  _maxFingerPressure = 1.0;
  _pressureFromPushing = 0.0;
  _pressureFromFinger = 0.0;
  _totalPressure = 0.0;
  _interpLineLength = ConfigVal("DragDrawing_InitialLineLength", 0.1*_lineLength, false);
  _roomToVirtual = Vector3::zero();
  _handVirtualOffset = Vector3::zero();

  if (_hybridStateOnBtnDown == HybridDrag) {
    startDrag();
  }
  else {
    startTape();
  }

  _eventBuffer.append(new Event("Brush_Btn_down"));
}

void
HybridDrawingEffect::phantomBtnUp() 
{
  _painting = false;
  _hybridState = _hybridStateOnBtnDown;
  _roomToVirtual = Vector3::zero();
  _handVirtualOffset = Vector3::zero();
  _dragHapticPath.clear();
  _eventBuffer.append(new Event("Brush_Btn_up"));  
}

void
HybridDrawingEffect::handBtnDown() 
{
  if (_painting) {
    if (_hybridState == HybridDrag) {
      if (!_disableTape) {
        Vector3 dragPtToPhantom = _phantomFrame.translation - _dragFrame.translation;
        _roomToVirtual = _roomToVirtual - dragPtToPhantom;
        // This offset to the hand should have the effect of negating
        // the roomToVirtual offset above, so the hand will look in the
        // drawing program like it stays in the same place when tape
        // drawing is engaged.
        _handVirtualOffset = _handVirtualOffset + dragPtToPhantom;
        
        startTape();
        
        // Apply an additional hand offset to make it match the tangent of the current line
        Vector3 tangent = dragPtToPhantom.unit();      
        Line l = Line::fromPointAndDirection(_dragFrame.translation, tangent);
        Vector3 newHandPos = l.closestPoint(_handPos);
        // Engage reverse tape drawing if moving in a direction opposite of the hand
        if ((newHandPos - _dragFrame.translation).dot(tangent) < 0.0) {
          _reverse = true;
          _eventBuffer.append(new Event("HybridDraw_ReverseTape"));
        }      
        Vector3 smoothingOffset = (newHandPos - _handPos);
        _handVirtualOffset = _handVirtualOffset + smoothingOffset;
        _handPos = _handPos + smoothingOffset;

        // Send a hand movement event immediately to avoid a flickering
        // effect where tape is turned on but with an old hand position
        // in the drawing server.
        _eventBuffer.append(new Event("Hand_Tracker", CoordinateFrame(_handPos)));
        _eventBuffer.append(new Event("Hand_Btn_down"));
      }
      else {
        // should never get here.. for tape drawing hand btn should already be down.
      }
    }
  }
  else {
    
    // Not painting.. check here for a click near the brush if so,
    // then use it as the signal for starting the drawing with Tape
    // rather than Drag.
    if (((_handPos - _phantomFrame.translation).length() < ConfigVal("TapeBtnProximityVal", 0.3, false)) &&
        (!_disableTape)) {
      _hybridStateOnBtnDown = HybridTape;
      _eventBuffer.append(new Event("HybridDraw_StartWithTape"));
    }
    else {
      _eventBuffer.append(new Event("Hand_Btn_down"));
    }
  }
}


void 
HybridDrawingEffect::handBtnUp() 
{
  if (_disableTape) {
    return;
  }

  if (_painting) {

    // sets drag frame..
    startDrag();

    // Adjust roomtovirtual..
    Vector3 phantomInRoom = _phantomFrame.translation - _roomToVirtual;
    Vector3 phantomVirtual = phantomInRoom;
    if (_markPath.size()) {
      Vector3 tangent = (_handPos - _markPath.last()).unit();
      if (_reverse) {
        tangent = -tangent;
      }
      phantomVirtual = _markPath.last() + 0.95*_lineLength*tangent;
    }
    _roomToVirtual = phantomVirtual - phantomInRoom;
    
    _phantomFrame.translation = phantomVirtual;
    
    // Actually, this should work if it's zero, but it probably should
    // be something like the original offset that was applied when tape
    // drawing was started.
    _handVirtualOffset = Vector3::zero();

    _reverse = false;
  }
  else {
    _handVirtualOffset = Vector3::zero();
  }

  _hybridStateOnBtnDown = HybridDrag;
  _eventBuffer.append(new Event("HybridDraw_StartWithDrag"));
  _eventBuffer.append(new Event("Hand_Btn_up"));
}

void 
HybridDrawingEffect::handMovement(CoordinateFrame newFrame) 
{
  Vector3 posInRoomSpace = newFrame.translation;

  // Immediately transform into virtual space
  newFrame.translation += _roomToVirtual + _handVirtualOffset;
  
  // If backing up in tape drawing then, recompute the handVirtualOffset to maintain a 
  // smooth tangent.
  if ((_hybridState == HybridTape) && (_drawingState == FLUID_BACKUP) && (_tanPath.size())) {
    Vector3 curHandPos = newFrame.translation;
    Line l = Line::fromTwoPoints(_lastPointOnPath, _lastPointOnPath + _tanPath.last());
    Vector3 projection = l.closestPoint(_handPos);

    // don't use this offset technique if the projection is rediculous
    Vector3 tangentA = (curHandPos - _lastPointOnPath).unit();
    Vector3 tangentB = (projection - _lastPointOnPath).unit();
    if (tangentA.dot(tangentB) > 0.5) {
      Vector3 additionalOffset = (projection - curHandPos);
      _handVirtualOffset += additionalOffset;
      newFrame.translation += additionalOffset;
    }
  }

  _handPos = newFrame.translation;

  // Update the drawing program on the position of the hand (virtual coords)
  double now = System::time();
  if (now - _lastHandUpdate >= _updateRate) {
    _lastHandUpdate = now;
    _eventBuffer.append(new Event("Hand_Tracker", newFrame));
  }
}

void
HybridDrawingEffect::reportBrush(CoordinateFrame brush, CoordinateFrame physBrush, 
                                 Vector3 tangent, bool addSample)
{
  _eventBuffer.append(new Event("Brush_Tracker", brush));
  _eventBuffer.append(new Event("Brush_Physical_Tracker", physBrush));
  if (addSample) {
    _markPath.append(brush.translation);
    _tanPath.append(tangent);
    if (_markPath.size() >= 2) {
      double dot = _tanPath[_tanPath.size()-1].dot(_tanPath[_tanPath.size()-2]);
      _anglePath.append(fabs(toDegrees(aCos(dot))));
    }
    else {
      _anglePath.append(0.0);
    }
    _eventBuffer.append(new Event("HybridDraw_AddSample"));
  }
}

void 
HybridDrawingEffect::phantomMovement(CoordinateFrame newFrame) 
{
  newFrame.translation = newFrame.translation + _roomToVirtual;
  _phantomFrame = newFrame;

  if (_hybridState == HybridDrag) {

    if (!_painting) {
      _dragFrame = newFrame;
      double now = System::time();
      if (now - _lastPhantomUpdate >= _updateRate) {
        _lastPhantomUpdate = now;
        reportBrush(newFrame, newFrame, Vector3(0,1,0), false);
      }
    }
    else { // painting

      double dragToPhantomDist = (_phantomFrame.translation - _dragFrame.translation).length(); 
      double goalLineLength = _lineLength;
      
      // If not in redrawing mode
      if (_dragHapticPath.size() == 0) {

        if (ConfigVal("Drag_SpeedChangesLineLength", 0, false)) {
          // adjust interpLineLength based on speed
          HDfloat v[3];
          hdGetFloatv(HD_CURRENT_VELOCITY, v);
          Vector3 vel(v[0], v[1], v[2]);
          double speed = vel.length();
          _speeds.pushFront(speed);
          _times.pushFront(System::time());
          
          // pop off any values older than a second ago
          double window = ConfigVal("Drag_SpeedAvgWindow", 1.0, false);
          while ((_times.size()) && (_times[_times.size()-1] < System::time() - window)) {
            _speeds.popBack();
            _times.popBack();
          }
          // find gaussian weighted avg speed
          double weight = 0.0;
          double sum = 0.0;
          for (int i=0;i<_speeds.size();i++) {
            double t = (System::time() - _times[i]) / window;
            double sigma = ConfigVal("Drag_GaussianSigma", 0.33, false);
            double G = exp(-0.5 * (t*t)/(sigma*sigma)) / sigma*sqrtf(twoPi());
            weight += G;
            sum += G * _speeds[i];
            //cout << i << " " << t << " " << G << " " << _speeds[i] << endl;
          }
          speed = sum / weight;
          //cout << "Avg Speed = " << speed << "  n = " << _speeds.size() << endl;
        
          // calculate a goal length for the drag line based on speed
          double typicalSpeed = ConfigVal("Drag_TypicalSpeed", 80.0, false);
          double minSpeed = ConfigVal("Drag_MinSpeed", 20.0, false);
          double maxSpeed = ConfigVal("Drag_MaxSpeed", 250.0, false);
          
          if ((System::time() - _dragStartTime) < 0.5*window) {
            goalLineLength = _lineLength;
          }
          else if (speed >= typicalSpeed) {
            // f goes from 0 to 1 as speed moves away from typical
            // linear
            double f = clamp((speed - typicalSpeed) / (maxSpeed - typicalSpeed), 0.0, 1.0);
            // quadratic
            f = f*f;
            goalLineLength = lerp(_lineLength, 0.1*_lineLength, f);
          }
          else {
            // f goes from 0 to 1 as speed moves away from typical
            //double f = clamp((typicalSpeed - speed) / (typicalSpeed - minSpeed), 0.0, 1.0);
            //goalLineLength = lerp(_lineLength, 1.25*_lineLength, f);
            goalLineLength = _lineLength;
          }
        }
        //else if (ConfigVal("Drag_CurvatureChangesLineLength", 0, false)) {
        else if (_useDynamicDraggingCurv) {

          // Estimate instantaneous curvature
          double weight = 0.0;
          double sum = 0.0;
          double arcLen = 0.0;
          double maxArcLen = ConfigVal("Drag_CurvEstArcLen", 0.5*_lineLength);
          double sampleSpacing = ConfigVal("Drag_CurvEstSampSpacing", maxArcLen / 20.0, false);
          int lastIndex = _markPath.size()-1;
          double lastArcLen = 0.0;
          int index = _markPath.size()-2;
          while ((index > 0) && (arcLen < maxArcLen)) {
            arcLen += (_markPath[index] - _markPath[index-1]).length();
            if ((arcLen - lastArcLen) > sampleSpacing) {
              double dot = _tanPath[lastIndex].dot(_tanPath[index]);
              double angle = fabs(toDegrees(aCos(dot)));
              double t = arcLen / maxArcLen;
              double sigma = ConfigVal("Drag_GaussianSigma", 0.33, false);
              double G = exp(-0.5 * (t*t)/(sigma*sigma)) / sigma*sqrtf(twoPi());
              weight += G;
              sum += G * angle;
              
              lastIndex = index;
              lastArcLen = arcLen;
            }
            index--;
          }

          double curvature = 0.0;
          if (weight != 0.0) { 
            curvature = sum / weight;
          }

          cout << "Curvature = " << curvature << endl;

          double typicalCurv = ConfigVal("Drag_TypicalCurv", 1.0, false);
          double minCurv = ConfigVal("Drag_MinCurv", 0.0, false);
          double maxCurv = ConfigVal("Drag_MaxCurv", 4.0, false);
          
          if (curvature >= typicalCurv) {
            // f goes from 0 to 1 as curv moves away from typical
            // linear
            double f = clamp((curvature - typicalCurv) / (maxCurv - typicalCurv), 0.0, 1.0);
            goalLineLength = lerp(_lineLength, 0.2*_lineLength, f);
          }
          else {
            // f goes from 0 to 1 as curv moves away from typical
            double f = clamp((typicalCurv - curvature) / (typicalCurv - minCurv), 0.0, 1.0);
            goalLineLength = lerp(_lineLength, 1.5*_lineLength, f);
          }
        }
      }
      else { 
        // in a redraw state, so no adjustment based on speed, keep the default line length
        _interpLineLength = _lineLength;
      }

      
      // Now adjust the interpolated line length if it isn't equal to the goal
      double amtMovement = (_phantomFrame.translation - _dragLastPhantomPos).length();
      _dragLastPhantomPos = _phantomFrame.translation;
      if (goalLineLength < _interpLineLength) {
        // shrinking.. avoid brush jumping too far ahead
        _interpLineLength -= 0.33*amtMovement;
        if (_interpLineLength < goalLineLength) {
          _interpLineLength = goalLineLength;
        }

        Vector3 change = 0.33*amtMovement * (_phantomFrame.translation - _dragFrame.translation).unit();
        _roomToVirtual = _roomToVirtual - change;
        _handVirtualOffset = _handVirtualOffset + change;
      }
      else if (goalLineLength > _interpLineLength) {
        // growing.. avoid brush holding still while expecting it to draw
        _interpLineLength += 0.33*amtMovement;
        if (_interpLineLength > goalLineLength) {
          _interpLineLength = goalLineLength;
        }
      }
    
      // send new line length to drawing program
      _eventBuffer.append(new Event("Drag_LineLength", _interpLineLength));

   
      // update the drag rotation always
      _dragFrame.rotation = _phantomFrame.rotation;
    
      double backupDist = ConfigVal("DragDrawing_BackupLength", 0.5*_lineLength, false);
      bool okToBackup = true;
      if ((_interpLineLength < 0.75*_lineLength) || (_markPath.size() <= 2)) {
        okToBackup = false;
        _eventBuffer.append(new Event("Drag_BackupLength", 0.0));
      }
      else {
        _eventBuffer.append(new Event("Drag_BackupLength", backupDist));
      }

      if (_disableBackup) {
        okToBackup = false;
        _eventBuffer.append(new Event("Drag_BackupLength", 0.0));
      }

      // update the drag position as it is dragged around
      if (dragToPhantomDist >= _interpLineLength) {
        // dragging the brush around
        _dragHapticPath.clear();
        Vector3 oldPos = _dragFrame.translation;
        Vector3 drawDir = (_phantomFrame.translation - oldPos).unit();
        Vector3 newPos = _phantomFrame.translation - _interpLineLength*drawDir;
        _dragFrame.translation = newPos;
        _pathLength += (newPos - oldPos).length();
        _lastPointOnPath = _dragFrame.translation;
        reportBrush(_dragFrame, _phantomFrame, 
                    (_phantomFrame.translation - _dragFrame.translation).unit(), true);
      }
      else if ((okToBackup) && (dragToPhantomDist <= backupDist)) {
        // pushing backwards to undo

        // find sample to backup to.. algorithm: check the last n
        // samples to find the one whose distance from the phantom's
        // position is closest without being more than the length of
        // the rod we're pushing backwards.
        int iClosest = _markPath.size()-1;
        int i =  _markPath.size()-2;
        double closestDist = (_phantomFrame.translation - _markPath[iClosest]).length();
        double lengthTested = 0.0;
        bool done = false;
        while ((i>=0) && (!done)) {
          lengthTested += (_markPath[i] - _markPath[i+1]).length();
          if (lengthTested > backupDist) {
            done = true;
          }
          else {
            double dist = (_phantomFrame.translation - _markPath[i]).length();
            if ((dist < backupDist) && (dist > closestDist)) {
              iClosest = i;
              closestDist = dist;
            }
          }
          i--;
        }

        if (iClosest != _markPath.size()-1) {
          _markPath.resize(iClosest+1);
          _tanPath.resize(iClosest+1);
          _anglePath.resize(iClosest+1);
          _dragFrame.translation = _markPath.last();
          _eventBuffer.append(new Event("HybridDraw_Resize", iClosest));          
        }
        
        reportBrush(_dragFrame, _phantomFrame,
                    (_phantomFrame.translation - _dragFrame.translation).unit(), false);

        _dragHapticPath.clear();
        for (int i=0;i<_markPath.size();i++) {
          Vector3 p = _markPath[i] + backupDist*_tanPath[i];
          _dragHapticPath.append(p);
        }
        _dragHapticPath.append(_dragHapticPath.last() + _lineLength*_tanPath.last());

      }
      else {
        // moving somewhere between towing and pushing backwards
        reportBrush(_dragFrame, _phantomFrame,
                    (_phantomFrame.translation - _dragFrame.translation).unit(), false);
      }
    }
  }
  
  else { // tapedrawing..
    
    // If we are painting, brush will be reported from the haptic
    // routine, otherwise, report it here.
    if (!_painting) {
      double now = System::time();
      if (now - _lastPhantomUpdate >= _updateRate) {
        _lastPhantomUpdate = now;
        reportBrush(newFrame, newFrame, Vector3(0,1,0), false);
      }
    }

  }
}




void 
HybridDrawingEffect::renderHaptics() 
{
  // Only update state / render haptics if the brush button is held
  if (_painting) {

    if (_hybridState == HybridDrag) {
     
      if (_dragHapticPath.size()) {
        hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, _lineShapeId);
        hlTouchModel(HL_CONSTRAINT);
        glLineWidth(1.0);
        glBegin(GL_LINE_STRIP);
        for (int i=0;i<_dragHapticPath.size();i++) {
          Vector3 p = _dragHapticPath[i] - _roomToVirtual;
          glVertex3d(p[0], p[1], p[2]);
        }
        glEnd();
        hlEndShape();
      }

    }
    else { // TapeDrawing

      // dir is the drawing direction
      Vector3 dir = (_handPos - _lastPointOnPath).unit();
      if (_reverse) {
        dir = -dir;
      }
      _endPt = _lastPointOnPath + 5.0*dir;
      
      Line l = Line::fromTwoPoints(_lastPointOnPath, _endPt);
      Vector3 newPointOnPath = l.closestPoint(_phantomFrame.translation);
      
      // Pressure is calculated based on the distance of the physical stylus from its
      // projection onto the line constraint.
      double maxPressure = ConfigVal("TapeDrawing_MaxPressure", 0.03, false);
      double pressure = (newPointOnPath - _phantomFrame.translation).length() / maxPressure;

      _pressureFromPushing = clamp(pressure, 0.0, 1.0);
      _totalPressure = _pressureFromFinger/_maxFingerPressure + _pressureFromPushing;
      _eventBuffer.append(new Event("Brush_Pressure", _totalPressure));      
      _eventBuffer.append(new Event("Brush_ColorValue", clamp(_totalPressure, 0.0, 1.0)));
      
      double handTooCloseThreshold = ConfigVal("TapeDrawing_HandTooCloseThreshold", 0.05, false);
      bool handTooClose = false;
      if ((_lastPointOnPath - _handPos).length() < handTooCloseThreshold) {
        handTooClose = true;
      }
      
      bool hapticRenderFullPath = false;
      
      double dotForward = (newPointOnPath - _lastPointOnPath).dot(dir);
      
      double dotBackward = 0.0;
      if (_markPath.size() >= 2) {
        Vector3 backDir = (_markPath[_markPath.size()-2] - _markPath[_markPath.size()-1]).unit();
        Line lBack = Line::fromTwoPoints(_markPath[_markPath.size()-2], _markPath[_markPath.size()-1]);
        Vector3 newPointOnBackPath = lBack.closestPoint(_phantomFrame.translation);
        dotBackward = (newPointOnBackPath - _lastPointOnPath).dot(backDir);
      }
      
      if (dotForward > ConfigVal("TapeDrawing_ForwardMotionThreshold", 0.005, false)) {
        // moving forward
        bool addSample = false;
        if (_drawingState == DRAWING_FORWARD) {
          if (handTooClose) {
            _drawingState = HOLDING_HAND_TOO_CLOSE;
          }
          else {
            // normal operation, add a new sample
            addSample = true;
          }
        }
        else if (_drawingState == FLUID_BACKUP) {
          if (handTooClose) {
            _drawingState = HOLDING_HAND_TOO_CLOSE;
          }
          else {
            // moving out of backup and into drawing forward
            _drawingState = DRAWING_FORWARD;
            addSample = true;
          }
        }
        else if (_drawingState == HOLDING_HAND_TOO_CLOSE) {
          if (!handTooClose) {
            // not too close anymore, break out of holding pattern
            _drawingState = DRAWING_FORWARD;
            addSample = true;
          }
        }
        
        if (addSample) {
          _pathLength += (newPointOnPath - _lastPointOnPath).length();
          _lastPointOnPath = newPointOnPath;
          // Report the Phantom's position to the client program as being the next position
          // on the line that we want to draw, rather than it's actual position.
          CoordinateFrame phantomUpdateFrame = _phantomFrame;
          phantomUpdateFrame.translation = newPointOnPath;
          Vector3 tangent = (_handPos - phantomUpdateFrame.translation).unit();
          if (_reverse) {
            tangent = -tangent;
          }
          reportBrush(phantomUpdateFrame, _phantomFrame, tangent, true);
        }

        
      }
      else if (dotBackward > ConfigVal("TapeDrawing_BackwardMotionThreshold", 0.004, false)) {
        
        // moving backward
        bool backup = false;
        if (handTooClose) {
          _drawingState = HOLDING_HAND_TOO_CLOSE;
        }
        else {
          if (_drawingState == DRAWING_FORWARD) {
            _drawingState = FLUID_BACKUP;
            backup = true;
          }
          else if (_drawingState == FLUID_BACKUP) {
            backup = true;
          }
          else if (_drawingState == HOLDING_HAND_TOO_CLOSE) {
            // stay in holding pattern, only move out of holding with forward movement
          }
        }
        
        if ((backup) && (_markPath.size())) {
          
          // Want the closest sample to the stylus that is "behind" the stylus w.r.t the 
          // drawing direction.
          
          // First, find the closest sample to the current position of the stylus
          int iClosest = 0;
          double dist = (_phantomFrame.translation - _markPath[0]).magnitude();
          for (int i=1;i<_markPath.size();i++) {
            double d = (_phantomFrame.translation - _markPath[i]).magnitude();
            if (d < dist) {
              dist = d;
              iClosest = i;
            }
          }
          
          // Now, iClosest points to the closest point, but if it is in front of the stylus
          // (if we'll pass it when we start moving in the forward direction) then we want
          // to pick a point earlier in the mark to avoid getting a discontinuity in the path.
          if (iClosest>0) {
            Vector3 v = (_phantomFrame.translation - _markPath[iClosest]).unit();
            if (dir.dot(v) < 0) {
              iClosest--;
            }
          }
          
          // If iClosest is not the last point, then we should do a back up and redraw edit
          // of the line
          if (iClosest != _markPath.size()-1) {
            hapticRenderFullPath = true;
            _markPath.resize(iClosest+1);
            _tanPath.resize(iClosest+1);
            _anglePath.resize(iClosest+1);
            _lastPointOnPath = _markPath.last();
            
            CoordinateFrame phantomUpdateFrame = _phantomFrame;
            phantomUpdateFrame.translation = _markPath.last();
            reportBrush(phantomUpdateFrame, _phantomFrame, _tanPath.last(), false);
            _eventBuffer.append(new Event("HybridDraw_Resize", iClosest));
          }
        }  // if backup and markpath.size
        
      }
      else {
        // moved only a little bit, probably just noise, ignore the movement
      }
      
      
      // RENDER HAPTICS..
      
      if (_drawingState == HOLDING_HAND_TOO_CLOSE) {
        // Constrain the stylus to a point..
        hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, _lineShapeId);
        hlTouchModel(HL_CONSTRAINT);
        // I can't get these to have any effect.  Maybe they are in the wrong place??
        hlMaterialf(HL_FRONT, HL_STIFFNESS, ConfigVal("TapeDrawing_Stiffness", 0.85, false));
        hlMaterialf(HL_FRONT, HL_DAMPING, ConfigVal("TapeDrawing_Damping", 0.5, false));
        hlMaterialf(HL_FRONT, HL_STATIC_FRICTION, ConfigVal("TapeDrawing_StaticFriction", 0, false));
        hlMaterialf(HL_FRONT, HL_DYNAMIC_FRICTION, ConfigVal("TapeDrawing_DynamicFriction", 0, false));
        glBegin(GL_POINTS);
        Vector3 p = _lastPointOnPath - _roomToVirtual;
        glVertex3d(p[0], p[1], p[2]);
        glEnd();
        hlEndShape();
      }
      else {
        // Constrain the phantom position to the line between the hand and the phantom
        hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, _lineShapeId);
        hlTouchModel(HL_CONSTRAINT);
        // I can't get these to have any effect.  Maybe they are in the wrong place??
        hlMaterialf(HL_FRONT, HL_STIFFNESS, ConfigVal("TapeDrawing_Stiffness", 0.85, false));
        hlMaterialf(HL_FRONT, HL_DAMPING, ConfigVal("TapeDrawing_Damping", 0.5, false));
        hlMaterialf(HL_FRONT, HL_STATIC_FRICTION, ConfigVal("TapeDrawing_StaticFriction", 0, false));
        hlMaterialf(HL_FRONT, HL_DYNAMIC_FRICTION, ConfigVal("TapeDrawing_DynamicFriction", 0, false));
        
        glLineWidth(1.0);
        glBegin(GL_LINE_STRIP);
        
        if (hapticRenderFullPath) {
          // Here, we're in the fluid backup state, so render previous points along the path.
          for (int i=0;i<_markPath.size();i++) {
            Vector3 p = _markPath[i] - _roomToVirtual;
            glVertex3d(p[0], p[1], p[2]);
          }
        }
        else {
          // For this case, either nothing has been drawn yet, in which
          // case we want to complete the haptic line with the initial
          // click point.  Or we're moving forward, or just slightly
          // backward, so we don't render the line that we have already
          // drawn.
          Vector3 p = _lastPointOnPath - _roomToVirtual;
          glVertex3d(p[0], p[1], p[2]);
        }
        Vector3 p = _endPt;
        glVertex3d(p[0], p[1], p[2]);
        
        glEnd();
        hlEndShape();
      }
    } // end tape drawing case
  } // end brush btn held down
}

void 
HybridDrawingEffect::renderGraphics() 
{
  // draw debugging gfx
  if (_painting) {
    glBegin(GL_LINES);
    for (int i=0;i<_markPath.size();i++) {
      Vector3 p = _markPath[i] - _roomToVirtual;
      glVertex3d(p[0], p[1], p[2]);
    }
    Vector3 p = _endPt - _roomToVirtual;
    glVertex3d(p[0], p[1], p[2]);
    glEnd();
  }
}
