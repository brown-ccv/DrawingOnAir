/**
 * \author Dan Keefe (dfk)
 *
 * \file  HCIMgr.H
 * \brief 
 *
 */

#ifndef HCIMGR_H
#define HCIMGR_H

//#include "VRBase.H"
#include <EventMgr.H>
#include <Event.h>

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class StylusHCI> StylusHCIRef;
/** Base class for CavePainting drawing interactors and other tool or stylus-based interactors.
    One and only one of these interactors is always active in the system unless a widget is 
    activated and steals the input focus away from the stylus interactor.  Before a widget
    steals the input focus away, calls StylusHCI::canReleaseFocus(), this should return false
    when the stylus is in some mode, like actively drawing, that shouldn't be interrupted.
*/
class StylusHCI : public G3D::ReferenceCountedObject
{
public:
  StylusHCI() {}
  virtual ~StylusHCI() {}
  
  /// Respond to this is subclasses to starting to listen and respond to input events
  /// when enabled and stopping when disabled.
  virtual void setEnabled(bool b) = 0;

  /// Should return true, unless the stylus is in the middle of some task, like drawing
  /// that shouldn't be interrupted.
  virtual bool canReleaseFocus() = 0;

private:
  StylusHCI(const StylusHCI &);
};

typedef G3D::ReferenceCountedPointer<class HCIMgr> HCIMgrRef;


typedef G3D::ReferenceCountedPointer<class WidgetHCI> WidgetHCIRef;
/** Base class for CavePainting widgets that have focus for input events when
    they are activated and release focus when dismissed.  This is like a modal
    dialog box in Windows & Mouse-based interaction terms.
*/
class WidgetHCI : public G3D::ReferenceCountedObject
{
public:
  WidgetHCI(HCIMgrRef hciMgr);
  virtual ~WidgetHCI() {}

  /// This is called by HCIMgr to start up the widget, subclasses should respond to this
  /// by starting up the widget and processing input events.
  virtual void activate() = 0;

  /// This must be called by subclasses when the widget closes in order to release input
  /// focus and allow other interactions controlled by the HCIMgr to be processed.
  virtual void releaseFocusWithHCIMgr();

  /// This calls _hciMgr->activateWidget(this), you can either call
  /// this to startup a widget, or call the line above yourself.
  void activateViaHCIMgr();

  /// This is only used for widgets, like Menus, that are registered with the HCIMgr to
  /// be activated when the pointer moves over them.  Should return true if the pointer
  /// position passed in is over the location of the widget.  If the subclass widget is
  /// not going to be activated when the pointer moves over it, then you don't need to
  /// reimplement this.
  virtual bool pointerOverWidget(G3D::Vector3 pointerPosRoomSpace) { return false; }
  
protected:
  HCIMgrRef _hciMgr;

private:
  WidgetHCI(const WidgetHCI &);
  WidgetHCI& operator=(const WidgetHCI &);
};


/**
*/
class HCIMgr : public G3D::ReferenceCountedObject
{
public:
  HCIMgr(const G3D::Array<std::string> pointerMoveEvents, MinVR::EventMgrRef eventMgr);
  virtual ~HCIMgr();

  bool activateStylusHCI(StylusHCIRef toActivate);

  bool activateWidget(WidgetHCIRef toActivate);
  void closeWidget(WidgetHCIRef toClose);

  void addPointerActivatedWidget(WidgetHCIRef widget) {
    _activateOnPointerOver.append(widget);
  }

  void pointerMove(MinVR::EventRef e);

private:
  StylusHCIRef _activeStylusHCI;
  WidgetHCIRef _activeWidgetHCI;

  /// List of widgets, typically menus, to check when the pointer moves around.  If no
  /// other widget is already active, these widgets will activate when the pointer is
  /// hovering over them.
  G3D::Array<WidgetHCIRef> _activateOnPointerOver;

  MinVR::EventMgrRef _eventMgr;
  MinVR::FsaRef      _fsa;

private:
  HCIMgr(const HCIMgr &);
  HCIMgr& operator=(const HCIMgr &);
};



} // end namespace

#endif


