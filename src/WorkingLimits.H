/**
 * /author Dan Keefe (dfk)
 *
 * /file  WorkingLimits.H
 * /brief 
 *
 */ 

#ifndef WORKINGLIMITS_H
#define WORKINGLIMITS_H

//#include "VRBase.H"
#include <GfxMgr.H>
#include "BrushState.H"
#include "GfxMgr.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class WorkingLimits> WorkingLimitsRef;
/**
*/
class WorkingLimits : public G3D::ReferenceCountedObject
{
public:
  WorkingLimits(G3D::AABox limits, double warningDist, BrushStateRef brushState, GfxMgrRef gfxMgr);
  virtual ~WorkingLimits();

  void pose(G3D::Array<G3D::PosedModel::Ref>& posedModels, const G3D::CoordinateFrame& frame);
  void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);

private:
  WorkingLimits(const WorkingLimits &);
  WorkingLimits& operator=(const WorkingLimits &);

  G3D::AABox           _limits;
  BrushStateRef   _brushState;
  double          _warningDist;
  double          _alpha;
};


} // end namespace

#endif

