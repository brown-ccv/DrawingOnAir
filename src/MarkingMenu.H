/**
 * /author Dan Keefe (dfk)
 *
 * /file  MarkingMenu.H
 * /brief A 2.5D version of a Bill Buxton style Marking Menu.
 *
 */ 


#ifndef MARKINGMENU_H
#define MARKINGMENU_H

//#include "VRBase.H"
#include <Event.h>
#include <EventMgr.H>
#include <GfxMgr.H>
#include "BrushState.H"
#include "CavePaintingCursors.H"
#include "ForceNetInterface.H"
#include "HCIMgr.H"
#include "History.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class MarkingMenu> MarkingMenuRef;
/**
   A Marking Menu in the style of Alias and Bill Buxton that works in 3D environments with
   a 3D input device.  This is to be used in conjuction with some other class that figures
   out when the input device is hovering over this menu.  When activate() is called, this
   class assumes that the input device is already hovering over the menu, ready to click and
   drag to select an option.
*/
class MarkingMenu : public WidgetHCI
{
public:
  MarkingMenu(MinVR::GfxMgrRef              gfxMgr,
              MinVR::EventMgrRef            eventMgr,
              HCIMgrRef              hciMgr,
              CavePaintingCursorsRef cursors,
              BrushRef          brush,
              //ForceNetInterface*     forceNetInterface,
              G3D::Array<std::string>     btnDownTriggers,
              G3D::Array<std::string>     trackerTriggers,
              G3D::Array<std::string>     btnUpTriggers,
              G3D::Array<std::string>     images,
              G3D::Array<std::string>     helpNames,
              G3D::Array<std::string>     eventNames,
              G3D::CoordinateFrame        initialFrame);

  virtual ~MarkingMenu();

  /// Return true if the pointer position is hovering over the location of this menu,
  /// i.e. should it be activated.
  bool pointerOverWidget(G3D::Vector3 pointerPosRoomSpace);

  /// Called by an external menu managaer to start up the menu, assumes overMenu is true.
  void activate();

  void show();
  void hide();
  bool getHidden() { return _hidden; }


  void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);
  void trackerMove(MinVR::EventRef e);
  void clickOn(MinVR::EventRef e);
  void trackerDrag(MinVR::EventRef e);
  void clickOff(MinVR::EventRef e);

protected:
  void deactivate();

  void drawColoredCircle(G3D::RenderDevice *rd, const G3D::Color4 &color, double radius, 
                         G3D::Vector3 centerPt, G3D::Vector3 normal, G3D::Vector3 right);
  void drawTexturedCircle(G3D::RenderDevice *rd, const std::string &texname, double radius, 
                          G3D::Vector3 centerPt, G3D::Vector3 normal, G3D::Vector3 right, float alpha);
  void drawText(G3D::RenderDevice *rd, std::string text, G3D::Vector3 centerPt, G3D::Vector3 normal, G3D::Vector3 right, float alpha);

  void drawTexturedSphereSection(const G3D::Sphere&       sphere,
                                 G3D::RenderDevice*       renderDevice,
                                 const G3D::Color4&       color,
                                 bool                top,
                                 bool                bottom,
                                 const std::string&  texname);

  BrushRef          _brush;
  CavePaintingCursorsRef _cursors;
  MinVR::EventMgrRef            _eventMgr;
  //ForceNetInterface*     _forceNetInterface;
  MinVR::FsaRef                 _fsa;
  G3D::GFontRef               _font;
  MinVR::GfxMgrRef              _gfxMgr;
  int                    _cbid;
  G3D::Array<std::string>     _images;
  G3D::Array<std::string>     _helpNames;
  G3D::Array<std::string>     _eventNames;  
  G3D::Array<G3D::Vector3>         _itemPts;
  G3D::Vector3                _trackerPos;
  G3D::Vector3                _norm;
  G3D::Vector3                _right;
  int                    _highlighted;
  G3D::CoordinateFrame        _frame;
  bool                   _hidden;

  // Constants determined by ConfigVals
  double                 SPHERE_RAD;
  double                 SPHERE_HIGH_RAD;
  double                 SELECTION_RAD;
  double                 ITEM_SPACING;
  double                 OFFSET_DIST;

private:
  MarkingMenu(const MarkingMenu &);
  MarkingMenu& operator=(const MarkingMenu &);
};

/***
class OopsMarkingMenu : public MarkingMenu
{
public:
  OopsMarkingMenu(Array<std::string> trackerTriggers,
                  Array<std::string> btnDownTriggers,
                  Array<std::string> btnUpTriggers,
                  Array<std::string> disableTriggers,
                  Array<std::string> enableTriggers,
                  Array<std::string> images,
                  Array<std::string> helpNames,
                  Array<std::string> eventNames,
                  HistoryRef         history) :
    MarkingMenu(trackerTriggers, btnDownTriggers, btnUpTriggers,
                disableTriggers, enableTriggers, images,
                helpNames, eventNames)
  {
    _history = history;
  }

  virtual ~OopsMarkingMenu() {};
  
  void activate(VRG3D::EventRef e)
  {
    _helpNames[1] = "Undo: " + _history->getUndoDescription();
    _helpNames[2] = "Redo: " + _history->getRedoDescription();
    MarkingMenu::activate(e);
  }

private:
  HistoryRef _history;
};
**/
  
} // end namespace

#endif

