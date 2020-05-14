/**
 * \author Dan Keefe (dfk)
 *
 * \file  ForceOneHandDrawHCI.H
 * \brief
 *
 */

#include <Event.h>
#ifndef FORCEONEHANDDRAWHCI_H
#define FORCEONEHANDDRAWHCI_H

//#include "VRBase.H"
#include "DirectDrawHCI.H"
#include "ForceNetInterface.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class ForceOneHandDrawHCI> ForceOneHandDrawHCIRef;
/**
*/
class ForceOneHandDrawHCI : public DirectDrawHCI
{
public:
  ForceOneHandDrawHCI(G3D::Array<std::string>     brushOnTriggers,
                      G3D::Array<std::string>     brushMotionTriggers, 
                      G3D::Array<std::string>     brushOffTriggers,
                      G3D::Array<std::string>     handMotionTriggers,
                      BrushRef               brush,
                      CavePaintingCursorsRef cursors,
                      //ForceNetInterface*     forceNetInterface,
                      MinVR::EventMgrRef            eventMgr);
  virtual ~ForceOneHandDrawHCI();

  void setEnabled(bool b);

private:
  ForceOneHandDrawHCI(const ForceOneHandDrawHCI &);
  ForceOneHandDrawHCI& operator=(const ForceOneHandDrawHCI &);

  //ForceNetInterface *_forceNetInterface;
};


} // end namespace

#endif
