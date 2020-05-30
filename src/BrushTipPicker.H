/**
 * /author Dan Keefe (dfk)
 *
 * /file  BrushTipPicker.H
 * /brief 
 *
 */ 

#ifndef BRUSHTIPPICKER_H
#define BRUSHTIPPICKER_H

#include "PickerWidget.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class BrushTipPicker> BrushTipPickerRef;
/**  
*/
class BrushTipPicker : public PickerWidget
{
public:
  BrushTipPicker(GfxMgrRef              gfxMgr,
                 EventMgrRef            eventMgr,
                 //ForceNetInterface*     forceNetInterface,
                 HCIMgrRef              hciMgr,
                 BrushRef          brush,
                 CavePaintingCursorsRef cursors,
                 G3D::Array<std::string>     btnDownEvents,
                 G3D::Array<std::string>     trackerEvents,
                 G3D::Array<std::string>     btnUpEvents,
                 G3D::Array<std::string>     handBtnEvents);
  
  virtual ~BrushTipPicker();

  std::string getCurrentPaletteName();
  bool setCurrentPaletteByName(const std::string &name);

  virtual void drawItem(int itemNum, const G3D::CoordinateFrame &itemFrame, G3D::RenderDevice *rd);
  virtual void selectionMade(int itemNum);

  virtual void spacePressed();
  virtual void handBtnPressed();

private:
  BrushTipPicker(const BrushTipPicker &);
  BrushTipPicker& operator=(const BrushTipPicker &);

  int               _curPalette;
  G3D::Array<G3D::Texture::Ref> _tipTextures;
  G3D::Array<G3D::Texture::Ref> _tipTileTextures;
  G3D::Array<std::string> _paletteFiles;
};


} // end namespace

#endif

