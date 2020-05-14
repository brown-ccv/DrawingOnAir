/**
 * /author Dan Keefe (dfk)
 *
 * /file  LightingIO.H
 * /brief 
 *
 */ 

#ifndef LIGHTINGIO_H
#define LIGHTINGIO_H

#include <VRG3DBaseApp.h>

namespace DrawOnAir {

class LightingIO
{
public:
  static void serializeGLight(const G3D::GLight light, G3D::BinaryOutput &b);
  static G3D::GLight deserializeGLight(G3D::BinaryInput &b);

  static void serializeLighting(G3D::LightingRef lighting, MinVR::GfxMgrRef gfxMgr, G3D::BinaryOutput &b);
  static void deserializeLighting(G3D::LightingRef lighting, MinVR::GfxMgrRef gfxMgr, G3D::BinaryInput &b);

};

} // end namespace

#endif

