
#include "WorkingLimits.H"
#include <ConfigVal.H>

using namespace G3D;
namespace DrawOnAir {

WorkingLimits::WorkingLimits(AABox limits, double warningDist, BrushStateRef brushState, MinVR::GfxMgrRef gfxMgr)
{
  _limits = limits;
  _warningDist = warningDist;
  _brushState = brushState;
  _alpha = 0;
  gfxMgr->addDrawCallback(this, &WorkingLimits::draw);
  gfxMgr->addPoseCallback(this, &WorkingLimits::pose);
}

WorkingLimits::~WorkingLimits()
{
}

void
WorkingLimits::pose(Array<PosedModel::Ref>& posedModels, const CoordinateFrame& frame)
{
  Vector3 p = _brushState->frameInRoomSpace.translation;
  
  if (!_limits.contains(p)) {
    _alpha = 1.0;
  }
  else {
    double d1 = abs(_limits.low()[0] - p[0]);
    double d2 = abs(_limits.low()[1] - p[1]);
    double d3 = abs(_limits.low()[2] - p[2]);
    double d4 = abs(_limits.high()[0] - p[0]);
    double d5 = abs(_limits.high()[1] - p[1]);
    double d6 = abs(_limits.high()[2] - p[2]);
    double minDist = G3D::min(d1, G3D::min(d2, G3D::min(d3, G3D::min(d4, G3D::min(d5, d6)))));
    _alpha = 0.0;
    if (minDist < _warningDist) {
      _alpha = 1.0 - (minDist / _warningDist);
    }
  }
}

void
WorkingLimits::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  Color3 c = Color3::green();
  if (_alpha != 0.0) {
    Color3 c1 = MinVR::ConfigVal("CLEAR_COLOR", Color3(0.35, 0.35, 0.35), false);
    c = c1.lerp(Color3::red(), _alpha);
  }

  rd->pushState();
  rd->disableLighting();

  /***
  Vector3 center(0.55, 0.5, 0.0);
  double radius = 0.02;

  rd->setCullFace(RenderDevice::CULL_NONE);
  rd->setColor(c);
  rd->disableLighting();
  rd->beginPrimitive(PrimitiveType::TRIANGLE_FAN);
  rd->sendVertex(center);
  const int nsections = 40;
  double a = 0.0;
  double ainc = -G3D_PI*2.0 / (double)(nsections-1);
  for (int i=0;i<nsections;i++) {
    rd->sendVertex(center + Vector3(radius*cos(a), radius*sin(a), 0.0));
    a -= ainc;
  }
  rd->endPrimitive();
  ***/

  if (_alpha != 0.0) {
    Draw::box(_limits, rd, Color4::clear(), c);
  }

  rd->popState();
}


} // end namespace
