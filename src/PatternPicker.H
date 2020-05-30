/**
 * /author Dan Keefe (dfk)
 *
 * /file  PatternPicker.H
 * /brief 
 *
 */ 
#include "PickerWidget.H"

#ifndef PATTERNPICKER_H
#define PATTERNPICKER_H



namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class PatternPicker> PatternPickerRef;
/**  
*/
class PatternPicker : public PickerWidget
{
public:
  PatternPicker(GfxMgrRef              gfxMgr,
                EventMgrRef            eventMgr,
                //ForceNetInterface*     forceNetInterface,
                HCIMgrRef              hciMgr,
                BrushRef               brush,
                CavePaintingCursorsRef cursors,
                G3D::Array<std::string>     btnDownEvents,
                G3D::Array<std::string>     trackerEvents,
                G3D::Array<std::string>     btnUpEvents,
                G3D::Array<std::string>     handBtnEvents);
  
  virtual ~PatternPicker();

  std::string getCurrentPaletteName();
  bool setCurrentPaletteByName(const std::string &name);

  virtual void drawItem(int itemNum, const G3D::CoordinateFrame &itemFrame, G3D::RenderDevice *rd);
  virtual void selectionMade(int itemNum);

  virtual void spacePressed();
  virtual void handBtnPressed();

private:
  PatternPicker(const PatternPicker &);
  PatternPicker& operator=(const PatternPicker &);
  
  int                _curPalette;
  G3D::Array<G3D::Texture::Ref>  _textures;
  G3D::Array<std::string> _paletteFiles;
};


} // end namespace

#endif

