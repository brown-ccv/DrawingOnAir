
#include "FishtankDepthCues.H"
#include <ConfigVal.H>
using namespace G3D;
namespace DrawOnAir {

FishtankDepthCues::FishtankDepthCues(GfxMgrRef gfxMgr)
{
  _gfxMgr = gfxMgr;
  // switched to calling this directly from cavepainting class
  //_gfxMgr->addDrawCallback(this, &FishtankDepthCues::draw);
}

FishtankDepthCues::~FishtankDepthCues()
{
}

void
FishtankDepthCues::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  Vector3 min = MinVR::ConfigVal("FishtankDepthCues_BoxMin", Vector3(-0.65, -0.5, -0.35), false);
  Vector3 max = MinVR::ConfigVal("FishtankDepthCues_BoxMax", Vector3(0.65, 0.5, 0.0), false);

  Vector3 pos = MinVR::ConfigVal("FishtankBox_LightPos", Vector3(0,0.4,-0.1), false);
  Color3 col = MinVR::ConfigVal("FishtankBox_LightColor", Color3(0.5,0.5,0.5), false);

  rd->pushState();
  rd->enableLighting();
  for (int i=0;i<RenderDevice::MAX_LIGHTS;i++) {
    rd->setLight(i, NULL);
  }
  rd->setLight(0, GLight::point(pos, col));

  rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
  rd->setDepthWrite(false);
  rd->setColor(Color4(1,1,1,0.5));
  rd->setTexture(0, _gfxMgr->getTexture("FishtankBox"));  
  
  rd->beginPrimitive(PrimitiveType::QUADS);

  // top face
  rd->setNormal(Vector3(0,-1,0));
  rd->setTexCoord(0, Vector2(0,0));
  rd->sendVertex(Vector3(min[0], max[1], max[2]));
  rd->setTexCoord(0, Vector2(1,0));
  rd->sendVertex(Vector3(min[0], max[1], min[2]));
  rd->setTexCoord(0, Vector2(1,1));
  rd->sendVertex(Vector3(max[0], max[1], min[2]));
  rd->setTexCoord(0, Vector2(0,1));
  rd->sendVertex(Vector3(max[0], max[1], max[2]));

  // bottom face
  rd->setNormal(Vector3(0,1,0));
  rd->setTexCoord(0, Vector2(0,0));
  rd->sendVertex(Vector3(min[0], min[1], max[2]));
  rd->setTexCoord(0, Vector2(1,0));
  rd->sendVertex(Vector3(max[0], min[1], max[2]));
  rd->setTexCoord(0, Vector2(1,1));
  rd->sendVertex(Vector3(max[0], min[1], min[2]));
  rd->setTexCoord(0, Vector2(0,1));
  rd->sendVertex(Vector3(min[0], min[1], min[2]));

  // back face
  rd->setNormal(Vector3(0,0,-1));
  rd->setTexCoord(0, Vector2(0,0));
  rd->sendVertex(Vector3(min[0], min[1], min[2]));
  rd->setTexCoord(0, Vector2(1,0));
  rd->sendVertex(Vector3(max[0], min[1], min[2]));
  rd->setTexCoord(0, Vector2(1,1));
  rd->sendVertex(Vector3(max[0], max[1], min[2]));
  rd->setTexCoord(0, Vector2(0,1));
  rd->sendVertex(Vector3(min[0], max[1], min[2]));

  // front face
  //rd->setNormal(Vector3(0,0,1));
  //rd->setTexCoord(0, Vector2(0,0));
  //rd->sendVertex(Vector3(min[0], min[1], max[2]));
  //rd->setTexCoord(0, Vector2(1,0));
  //rd->sendVertex(Vector3(min[0], max[1], max[2]));
  //rd->setTexCoord(0, Vector2(1,1));
  //rd->sendVertex(Vector3(max[0], max[1], max[2]));
  //rd->setTexCoord(0, Vector2(0,1));
  //rd->sendVertex(Vector3(max[0], min[1], max[2]));
  
  // left face
  rd->setNormal(Vector3(1,0,0));
  rd->setTexCoord(0, Vector2(0,0));
  rd->sendVertex(Vector3(min[0], min[1], max[2]));
  rd->setTexCoord(0, Vector2(1,0));
  rd->sendVertex(Vector3(min[0], min[1], min[2]));
  rd->setTexCoord(0, Vector2(1,1));
  rd->sendVertex(Vector3(min[0], max[1], min[2]));
  rd->setTexCoord(0, Vector2(0,1));
  rd->sendVertex(Vector3(min[0], max[1], max[2]));

  // right face
  rd->setNormal(Vector3(-1,0,0));
  rd->setTexCoord(0, Vector2(0,0));
  rd->sendVertex(Vector3(max[0], min[1], max[2]));
  rd->setTexCoord(0, Vector2(1,0));
  rd->sendVertex(Vector3(max[0], max[1], max[2]));
  rd->setTexCoord(0, Vector2(1,1));
  rd->sendVertex(Vector3(max[0], max[1], min[2]));
  rd->setTexCoord(0, Vector2(0,1));
  rd->sendVertex(Vector3(max[0], min[1], min[2]));
  
  rd->endPrimitive();

  rd->popState();
}


} // end namespace
