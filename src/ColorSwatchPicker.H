/**
 * /author Dan Keefe (dfk)
 *
 * /file  ColorSwatchPicker.H
 * /brief A 2.5D version of a Bill Buxton style Marking Menu.
 *
 */ 


#ifndef COLORSWATCHPICKER_H
#define COLORSWATCHPICKER_H

//#include "VRBase.H"
#include <VRG3DBaseApp.h>
#include "BrushState.H"
#include "CavePaintingCursors.H"
#include "ForceNetInterface.H"
#include "HCIMgr.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class ColorSwatchPicker> ColorSwatchPickerRef;
/**
*/
class ColorSwatchPicker : public WidgetHCI
{
public:
  ColorSwatchPicker(GfxMgrRef              gfxMgr,
                   EventMgrRef            eventMgr,
                   // ForceNetInterface*     forceNetInterface,
                    HCIMgrRef              hciMgr,
                    CavePaintingCursorsRef cursors,
                    BrushRef               brush,
                    G3D::Array<std::string>     btnDownTriggers,
                    G3D::Array<std::string>     trackerTriggers,
                    G3D::Array<std::string>     btnUpTriggers,
                    G3D::Array<std::string>     handBtnTriggers,
                    G3D::CoordinateFrame        initialFrame);

  virtual ~ColorSwatchPicker();

  std::string getCurrentPaletteName();
  bool setCurrentPaletteByName(const std::string &name);

  /// Called by an external menu managaer to start up the menu, assumes overMenu is true.
  void activate();

  void show();
  void hide();

  void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);

  void trackerMove(MinVR::EventRef e);
  void clickOn(MinVR::EventRef e);
  void clickOff(MinVR::EventRef e);

  void nextPalette(MinVR::EventRef e);

protected:
  void deactivate();

  G3D::Array<G3D::Texture::Ref>      _swatchTextures;
  BrushRef               _brush;
  CavePaintingCursorsRef _cursors;
  G3D::CoordinateFrame        _frame;
  EventMgrRef            _eventMgr;
  //ForceNetInterface*     _forceNetInterface;
  FsaRef                 _fsa;
  G3D::GFontRef               _font;
  GfxMgrRef              _gfxMgr;
  double                 _currentCoord;
  int                    _cbid;
  int                    _curPalette;
  G3D::Array<std::string>     _paletteFiles;

private:
  ColorSwatchPicker(const ColorSwatchPicker &);
  ColorSwatchPicker& operator=(const ColorSwatchPicker &);
};
  
} // end namespace

#endif

