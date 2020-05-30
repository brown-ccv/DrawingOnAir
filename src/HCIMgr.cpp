#include "HCIMgr.H"
using namespace G3D;
namespace DrawOnAir {

WidgetHCI::WidgetHCI(HCIMgrRef hciMgr)
{
  _hciMgr = hciMgr;
}
 
void
WidgetHCI::releaseFocusWithHCIMgr()
{
  _hciMgr->closeWidget(this);
}

void
WidgetHCI::activateViaHCIMgr()
{
  _hciMgr->activateWidget(this);
}



HCIMgr::HCIMgr(const Array<std::string> pointerMoveEvents, EventMgrRef eventMgr)
{
  _eventMgr = eventMgr;
  _fsa = new Fsa("HCIMgr");
  _fsa->addState("Start");
  _fsa->addArc("PointerMove", "Start", "Start", pointerMoveEvents);
  _fsa->addArcCallback("PointerMove", this, &HCIMgr::pointerMove);
  _eventMgr->addFsaRef(_fsa);
}
 
HCIMgr::~HCIMgr()
{
}

bool
HCIMgr::activateStylusHCI(StylusHCIRef toActivate)
{
  if (_activeWidgetHCI.isNull()) {
    if (_activeStylusHCI.notNull()) {
      if (!_activeStylusHCI->canReleaseFocus()) {
        return false;
     }
      _activeStylusHCI->setEnabled(false);
    }
    _activeStylusHCI = toActivate;
    _activeStylusHCI->setEnabled(true);
    return true;
  }
  else {
    return false;
  } 
}

bool
HCIMgr::activateWidget(WidgetHCIRef toActivate)
{
  // if there is already a widget active or if the active stylushci can't release
  // focus then return
  if (_activeWidgetHCI.notNull()) {
    cout << "Can't activate widget" << endl;
    return false;
  }

  if (_activeStylusHCI.notNull()) {
    if (!_activeStylusHCI->canReleaseFocus()) {
      return false;
    }
    _activeStylusHCI->setEnabled(false);
  }
  _activeWidgetHCI = toActivate;
  _activeWidgetHCI->activate();
  _eventMgr->removeFsaRef(_fsa);
  return true;
}

void
HCIMgr::closeWidget(WidgetHCIRef toClose)
{
  alwaysAssertM(toClose == _activeWidgetHCI, "Asked to close a widget that is not currently active.");
  _activeWidgetHCI = NULL;
  if (_activeStylusHCI.notNull()) {
    _activeStylusHCI->setEnabled(true);
  }
  _eventMgr->addFsaRef(_fsa);
}

void
HCIMgr::pointerMove(MinVR::EventRef e)
{
  // should only call this when no other widgets are active
  debugAssert(_activeWidgetHCI.isNull());
  bool activatedOne = false;
  int i=0;
  while ((i<_activateOnPointerOver.size()) && (!activatedOne)) {
    if (_activateOnPointerOver[i]->pointerOverWidget(e->getCoordinateFrameData().translation)) {
      activateWidget(_activateOnPointerOver[i]);            
      activatedOne = true;
    }
    i++;
  }
}

} // end namespace



