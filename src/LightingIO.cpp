
#include "LightingIO.H"
using namespace G3D;
namespace DrawOnAir {

void
LightingIO::serializeGLight(const GLight light, BinaryOutput &b)
{
  light.position.serialize(b);
  light.spotDirection.serialize(b);
  Vector3(light.attenuation[0],light.attenuation[1],light.attenuation[2]).serialize(b);
  light.color.serialize(b);
  b.writeBool8(light.specular);
  b.writeBool8(light.diffuse);
}

GLight
LightingIO::deserializeGLight(BinaryInput &b)
{
  GLight light;
  light.position.deserialize(b);
  light.spotDirection.deserialize(b);
  Vector3 atten;
  atten.deserialize(b);
  light.attenuation[0] = atten[0];
  light.attenuation[1] = atten[1];
  light.attenuation[2] = atten[2];
  light.color.deserialize(b);
  light.specular = b.readBool8();
  light.diffuse = b.readBool8();
  return light;
}


void
LightingIO::serializeLighting(LightingRef lighting, MinVR::GfxMgrRef gfxMgr, BinaryOutput &b)
{
  lighting->ambientTop.serialize(b);
  lighting->ambientBottom.serialize(b);
  b.writeString(gfxMgr->lookupTextureKeyName(lighting->environmentMap));
  lighting->environmentMapColor.serialize(b);
  b.writeInt32(lighting->lightArray.size());
  for (int i=0;i<lighting->lightArray.size();i++) {
    serializeGLight(lighting->lightArray[i], b);
  }
  b.writeInt32(lighting->shadowedLightArray.size());
  for (int i=0;i<lighting->shadowedLightArray.size();i++) {
    serializeGLight(lighting->shadowedLightArray[i], b);
  }
}

void
LightingIO::deserializeLighting(LightingRef lighting, MinVR::GfxMgrRef gfxMgr, BinaryInput &b)
{
  if (lighting.isNull()) {
    lighting = Lighting::create();
  }
  lighting->lightArray.clear();
  lighting->shadowedLightArray.clear();

  lighting->ambientTop.deserialize(b);
  lighting->ambientBottom.deserialize(b);
  std::string texname = b.readString();
  lighting->environmentMap = gfxMgr->getTexture(texname);
  lighting->environmentMapColor.deserialize(b);
  int nl = b.readInt32();
  for (int i=0;i<nl;i++) {
    GLight light = deserializeGLight(b);
    lighting->lightArray.append(light);
  }

  int nsl = b.readInt32();
  for (int i=0;i<nsl;i++) {
    GLight light = deserializeGLight(b);
    lighting->shadowedLightArray.append(light);
  }
}

} // end namespace

