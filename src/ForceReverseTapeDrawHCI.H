/**
 * \author Dan Keefe (dfk)
 *
 * \file  ForceReverseTapeDrawHCI.H
 * \brief
 *
 */

#include <EventMgr.H>
#ifndef FORCEREVERSETAPEDRAWHCI_H
#define FORCEREVERSETAPEDRAWHCI_H

//#include "VRBase.H"
#include "DirectDrawHCI.H"
#include "ForceNetInterface.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class ForceReverseTapeDrawHCI> ForceReverseTapeDrawHCIRef;
/**
*/
class ForceReverseTapeDrawHCI : public DirectDrawHCI
{
public:
  ForceReverseTapeDrawHCI(G3D::Array<std::string>     brushOnTriggers,
                   G3D::Array<std::string>     brushMotionTriggers, 
                   G3D::Array<std::string>     brushOffTriggers,
                   G3D::Array<std::string>     handMotionTriggers,
                   BrushRef               brush,
                   CavePaintingCursorsRef cursors,
                   //ForceNetInterface*     forceNetInterface,
                   EventMgrRef            eventMgr);
  virtual ~ForceReverseTapeDrawHCI();

  void setEnabled(bool b);

private:
  ForceReverseTapeDrawHCI(const ForceReverseTapeDrawHCI &);
  ForceReverseTapeDrawHCI& operator=(const ForceReverseTapeDrawHCI &);

  //ForceNetInterface *_forceNetInterface;
};


} // end namespace

#endif
