
#ifdef USE_SPLINES

#ifndef SPLINEFIT_H
#define SPLINEFIT_H

#include <DrawOnAir.H>

namespace DrawOnAir {

  MarkRef SplineFit(MarkRef m, double resampleStepSize, 
                    G3D::Array<G3D::Vector3> &controlPoints, G3D::Array<G3D::Vector3> &segmentationPoints,
                    TriStripModelRef triStripModel, GfxMgrRef gfxMgr);

} // end namespace

#endif


#endif
