/**
 * \author Daniel Keefe (dfk)
 *
 * \file  AnisotropicFilterEffect.H
 * \brief 
 *
 */

#ifndef ANIFILTEREFFECT_H
#define ANIFILTEREFFECT_H

#include "DrawOnAir.H"
#include "ForceEffect.H"

// OpenHaptics includes
#include <HD/hd.h>
#include <HL/hl.h>

typedef ReferenceCountedPointer<class AnisotropicFilterEffect> AnisotropicFilterEffectRef;
/** 
*/
class AnisotropicFilterEffect : public ForceEffect
{
public:
  AnisotropicFilterEffect(double eventUpdateRate, 
                          Vector3 phantomThreshold, Vector3 handThreshold);
  virtual ~AnisotropicFilterEffect();

  virtual void handMovement(CoordinateFrame newFrame);
  virtual void phantomMovement(CoordinateFrame newFrame);
  
private:
  Vector3  _phantomThreshold;
  Vector3  _handThreshold;
  Vector3  _phantomPos;
  Vector3  _handPos;
};


#endif
