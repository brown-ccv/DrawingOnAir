/**
 * /author Dan Keefe (dfk)
 *
 * /file  FishtankDepthCues.H
 * /brief 
 *
 */ 

#include <GfxMgr.H>

#ifndef FISHTANKDEPTHCUES_H
#define FISHTANKDEPTHCUES_H

//#include "VRBase.H"


namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class FishtankDepthCues> FishtankDepthCuesRef;
/**
*/
class FishtankDepthCues : public G3D::ReferenceCountedObject
{
public:
  FishtankDepthCues(GfxMgrRef gfxMgr);
  virtual ~FishtankDepthCues();

  void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);

private:
  FishtankDepthCues(const FishtankDepthCues &);
  FishtankDepthCues& operator=(const FishtankDepthCues &);

  GfxMgrRef _gfxMgr;
};


} // end namespace

#endif

