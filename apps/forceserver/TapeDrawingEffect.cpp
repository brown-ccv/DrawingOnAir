
#include "TapeDrawingEffect.H"


TapeDrawingEffect::TapeDrawingEffect(double eventUpdateRate) : ForceEffect(eventUpdateRate)
{
  _painting = false;
  _reverse = false;
  _pressureFromPushing = false;
  _drawingState = DRAWING_FORWARD;
  _disableBackup = ConfigVal("TapeDrawing_DisableBackup", 0, false);

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

TapeDrawingEffect::~TapeDrawingEffect()
{
}

void
TapeDrawingEffect::phantomBtnDown()
{
  _painting = true;
  _drawingState = DRAWING_FORWARD;
  _lastPointOnPath = _phantomFrame.translation;
  _markPath.clear();
  _handPath.clear();

  _eventBuffer.append(new Event("Brush_Btn_down"));
}

void
TapeDrawingEffect::phantomBtnUp() 
{
  _painting = false;
  _drawingState = DRAWING_FORWARD;
  _markPath.clear();
  _handPath.clear();

  _eventBuffer.append(new Event("Brush_Btn_up"));
}

void 
TapeDrawingEffect::phantomMovement(CoordinateFrame newFrame) 
{
  _phantomFrame = newFrame;

  double now = System::time();
  if (now - _lastPhantomUpdate >= _updateRate) {
    _lastPhantomUpdate = now;
    if (!_painting) {
      _eventBuffer.append(new Event("Brush_Tracker", newFrame));
    }
    _eventBuffer.append(new Event("Brush_Physical_Tracker", newFrame));
  }
}

void
TapeDrawingEffect::handBtnDown() 
{
  _eventBuffer.append(new Event("Hand_Btn_down"));
}

void 
TapeDrawingEffect::handBtnUp() 
{
  _eventBuffer.append(new Event("Hand_Btn_up"));
}

void 
TapeDrawingEffect::handMovement(CoordinateFrame newFrame) 
{
  _handPos = newFrame.translation;

  if ((_drawingState == FLUID_BACKUP) && (_handPath.size())) {
    Line l = Line::fromTwoPoints(_lastPointOnPath, _handPath.last());
    Vector3 projection = l.closestPoint(_handPos);

    // don't use this offset technique if the projection is rediculous
    Vector3 tangentA = (_handPos - _lastPointOnPath).unit();
    Vector3 tangentB = (projection - _lastPointOnPath).unit();
    if (tangentA.dot(tangentB) > 0.5) {
      _handRedrawOffset = projection - _handPos;
      _handPos = projection;
    }
  }
  else {
    _handPos += _handRedrawOffset;
  }
  //cout << _handPos << endl;

  newFrame.translation = _handPos;

  double now = System::time();
  if (now - _lastHandUpdate >= _updateRate) {
    _lastHandUpdate = now;
    _eventBuffer.append(new Event("Hand_Tracker", newFrame));
  }
}

void
TapeDrawingEffect::pressureSensorUpdate(double newValue)
{
  if (!_pressureFromPushing) {
    _eventBuffer.append(new Event("Brush_Pressure", newValue));
  }
}

void 
TapeDrawingEffect::renderHaptics() 
{
  // Only update state / render haptics if the brush button is held
  if (_painting) {

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
    if (_pressureFromPushing) {
      double maxPressure = ConfigVal("TapeDrawing_MaxPressure", 0.03, false);
      double pressure = (newPointOnPath - _phantomFrame.translation).length() / maxPressure;
      _eventBuffer.append(new Event("Brush_Pressure", pressure));
    }

    double handTooCloseThreshold = ConfigVal("TapeDrawing_HandTooCloseThreshold", 0.05, false);
    bool handTooClose = false;
    if ((_lastPointOnPath - _handPos).length() < handTooCloseThreshold) {
      handTooClose = true;
    }

    bool hapticRenderFullPath = false;

    double dotForward = (newPointOnPath - _lastPointOnPath).dot(dir);

    double dotBackward = 0.0;
    if ((_markPath.size() >= 2) && (!_disableBackup)) {
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
        _lastPointOnPath = newPointOnPath;
        // Report the Phantom's position to the client program as being the next position
        // on the line that we want to draw, rather than it's actual position.
        CoordinateFrame phantomUpdateFrame = _phantomFrame;
        phantomUpdateFrame.translation = newPointOnPath;
        _eventBuffer.append(new Event("Brush_Tracker", phantomUpdateFrame));
        _eventBuffer.append(new Event("TapeDraw_AddSample"));
        _markPath.append(phantomUpdateFrame.translation);
        _handPath.append(_handPos);
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
          _handPath.resize(iClosest+1);
          _lastPointOnPath = _markPath.last();
                    
          CoordinateFrame phantomUpdateFrame = _phantomFrame;
          phantomUpdateFrame.translation = _markPath.last();
          _eventBuffer.append(new Event("Brush_Tracker", phantomUpdateFrame));
          _eventBuffer.append(new Event("TapeDraw_Resize", iClosest));
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
      glVertex3d(_lastPointOnPath[0], _lastPointOnPath[1], _lastPointOnPath[2]);
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
          glVertex3d(_markPath[i][0], _markPath[i][1], _markPath[i][2]);
        }
      }
      else {
        // For this case, either nothing has been drawn yet, in which
        // case we want to complete the haptic line with the initial
        // click point.  Or we're moving forward, or just slightly
        // backward, so we don't render the line that we have already
        // drawn.
        glVertex3d(_lastPointOnPath[0], _lastPointOnPath[1], _lastPointOnPath[2]);
      }
      glVertex3d(_endPt[0], _endPt[1], _endPt[2]);
      
      glEnd();
      hlEndShape();
    }
  } // end brush btn held down
}

void 
TapeDrawingEffect::renderGraphics() 
{
  // draw debugging gfx
  if (_painting) {
    glBegin(GL_LINES);
    for (int i=0;i<_markPath.size();i++) {
      glVertex3d(_markPath[i][0], _markPath[i][1], _markPath[i][2]);
    }
    glVertex3d(_endPt[0], _endPt[1], _endPt[2]);
    glEnd();
  }
}
