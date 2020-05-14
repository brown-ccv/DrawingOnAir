
#include "OneHandDrawingEffect.H"


OneHandDrawingEffect::OneHandDrawingEffect(double eventUpdateRate) : ForceEffect(eventUpdateRate)
{
  _painting = false;
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

OneHandDrawingEffect::~OneHandDrawingEffect()
{
}

void
OneHandDrawingEffect::phantomBtnDown()
{
  _painting = true;
  _lastPointOnPath = _phantomFrame.translation;

  _eventBuffer.append(new Event("Brush_Btn_down"));
}

void
OneHandDrawingEffect::phantomBtnUp() 
{
  _painting = false;

  _eventBuffer.append(new Event("Brush_Btn_up"));
}

void 
OneHandDrawingEffect::phantomMovement(CoordinateFrame newFrame) 
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
OneHandDrawingEffect::pressureSensorUpdate(double newValue)
{
}

void 
OneHandDrawingEffect::renderHaptics() 
{
  Vector3 dir = _phantomFrame.vectorToWorldSpace(Vector3(0,1,0)).unit();
  _endPt = _lastPointOnPath + 5.0*dir;
  
  if (_painting) {
    Line l = Line::fromTwoPoints(_lastPointOnPath, _endPt);
    Vector3 newPointOnPath = l.closestPoint(_phantomFrame.translation);
    if ((newPointOnPath - _lastPointOnPath).dot(dir) > 0) {
      _lastPointOnPath = newPointOnPath;
      // Report the Phantom's position to the client program as being the next position
      // on the line that we want to draw, rather than it's actual position.
      CoordinateFrame phantomUpdateFrame = _phantomFrame;
      phantomUpdateFrame.translation = newPointOnPath;
      _eventBuffer.append(new Event("Brush_Tracker", phantomUpdateFrame));
      double maxPressure = ConfigVal("OneHandDrawing_MaxPressure", 0.03, false);
      double pressure = (newPointOnPath - _phantomFrame.translation).length() / maxPressure;
      _eventBuffer.append(new Event("Brush_Pressure", pressure));
    }
    
    // constrain the phantom position to the line between the hand and the phantom
    hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, _lineShapeId);
    
    hlTouchModel(HL_CONSTRAINT);
    //hlTouchModelf(HL_SNAP_DISTANCE, _lineShapeSnapDistance);
    hlMaterialf(HL_FRONT, HL_STIFFNESS, ConfigVal("TapeDrawing_Stiffness", 0.85, false));
    hlMaterialf(HL_FRONT, HL_STATIC_FRICTION, 0);
    hlMaterialf(HL_FRONT, HL_DYNAMIC_FRICTION, 0);

    glLineWidth(1.0);
    glBegin(GL_LINES);
    glVertex3d(_lastPointOnPath[0], _lastPointOnPath[1], _lastPointOnPath[2]);
    glVertex3d(_endPt[0], _endPt[1], _endPt[2]);
    glEnd();

    hlEndShape();
  }
}

void 
OneHandDrawingEffect::renderGraphics() 
{
  // draw debugging gfx
  if (_painting) {
    glBegin(GL_LINES);
    glVertex3d(_lastPointOnPath[0], _lastPointOnPath[1], _lastPointOnPath[2]);
    glVertex3d(_endPt[0], _endPt[1], _endPt[2]);
    glEnd();
  }
}
