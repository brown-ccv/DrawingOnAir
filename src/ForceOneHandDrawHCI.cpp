#include "ForceOneHandDrawHCI.H"
#include <ConfigVal.H>
using namespace G3D;
namespace DrawOnAir {

ForceOneHandDrawHCI::ForceOneHandDrawHCI(Array<std::string>     brushOnTriggers,
                                   Array<std::string>     brushMotionTriggers,
                                   Array<std::string>     brushOffTriggers,
                                   Array<std::string>     handMotionTriggers,
                                   BrushRef               brush,
                                   CavePaintingCursorsRef cursors,
                                   //ForceNetInterface*     forceNetInterface,
  MinVR::EventMgrRef            eventMgr) :
  DirectDrawHCI(brushOnTriggers, brushMotionTriggers, brushOffTriggers, 
                handMotionTriggers, brush, cursors, eventMgr)
{
 // _forceNetInterface = forceNetInterface;
}


ForceOneHandDrawHCI::~ForceOneHandDrawHCI()
{
}

void
ForceOneHandDrawHCI::setEnabled(bool b)
{  
  if ((b) && (!_enabled)) {
    //_forceNetInterface->startOneHandDrawing();
  }
  else if ((!b) && (_enabled)) {
    //_forceNetInterface->stopForces();
  }
  DirectDrawHCI::setEnabled(b);

  if (_enabled) {
    _cursors->setBrushCursor(CavePaintingCursors::DEFAULT_CURSOR);
    _brush->state->maxPressure = MinVR::ConfigVal("ForceOneHandDraw_MaxPressure", 0.03, false);
    _brush->state->brushInterface = "ForceOneHandDraw";
  }  
}

} // end namespace
