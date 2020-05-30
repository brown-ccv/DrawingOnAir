/**
 * /author Dan Keefe (dfk)
 *
 * /file  ColorPicker.H
 * /brief A 3D widget for picking a color.
 *
 */ 


#ifndef COLORPICKER_H
#define COLORPICKER_H

//#include "VRBase.H"
#include <VRG3DBaseApp.h>
#include "CavePaintingCursors.H"
#include "HCIMgr.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class ColorPicker> ColorPickerRef;

/** 
    A 3D widget for selecting colors with a 3D input device.  When a color
    is selected the ColorPicker sends out a ColorPicker_ColorSelected event
    of type EVENTTYPE_3D that contains the R,G,B values of the newly selected
    color.  If the picker is dismissed without selecting a color a
    ColorPicker_Dismissed event is generated.
*/

class ColorPicker : public WidgetHCI
{
public:

  ColorPicker(GfxMgrRef              gfxMgr,
    EventMgrRef            eventMgr,
              HCIMgrRef              hciMgr,
              CavePaintingCursorsRef cursors,
              G3D::Array<std::string>     trackerEvents,
	            G3D::Array<std::string>     btnEvents);
  
  virtual ~ColorPicker();

  void setInitialColor(G3D::Color3 color) { _initialColor = color; }
  void setInitialTrackerPos(G3D::Vector3 pos) { _initialTrackerPos = pos; }
  void activate();
  
  void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);

  void pickBtnPressed(MinVR::EventRef e);
  void trackerMoved(MinVR::EventRef e);

private:
  ColorPicker(const ColorPicker &);
  ColorPicker& operator=(const ColorPicker &);

  void deactivate();
  
  /// p is relative to the center of the color picker
  G3D::Color3  pointToColor(G3D::Vector3 p);
  /// c is returned in a coordinate space relative to the center of the widget
  G3D::Vector3 colorToPoint(G3D::Color3 c);
  double  findValue(double n1, double n2, double hue);
  bool    outsideCancelSphere();

  G3D::Vector3       _widgetCenterInRoom;
  G3D::Vector3       _trackerPos;
  FsaRef        _fsa;
  double        _startTime;
  double        _startFadeOutTime;
  std::string   _activeTracker;
  G3D::Color3        _initialColor;
  G3D::Vector3       _initialTrackerPos;
  int           _dcbid;
  G3D::CoordinateFrame _frame;

  double        CP_GEOM_RAD;
  double        CANCEL_GEOM_RAD;

  GfxMgrRef     _gfxMgr;
  EventMgrRef   _eventMgr;
};


} // end namespace

#endif

