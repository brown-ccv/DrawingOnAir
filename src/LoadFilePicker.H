/**
 * /author Dan Keefe (dfk)
 *
 * /file  LoadFilePicker.H
 * /brief 
 *
 */ 

#include "PickerWidget.H"
#ifndef LOADFILEPICKER_H
#define LOADFILEPICKER_H



namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class LoadFilePicker> LoadFilePickerRef;
/**  
*/
class LoadFilePicker : public PickerWidget
{
public:
  LoadFilePicker(MinVR::GfxMgrRef              gfxMgr,
                 MinVR::EventMgrRef            eventMgr,
                // ForceNetInterface*     forceNetInterface,
                 HCIMgrRef              hciMgr,
                 BrushRef               brush,
                 CavePaintingCursorsRef cursors,
                 G3D::Array<std::string>     btnDownEvents,
                 G3D::Array<std::string>     trackerEvents,
                 G3D::Array<std::string>     btnUpEvents);
  
  virtual ~LoadFilePicker();

  virtual void startWidget();

  virtual void drawItem(int itemNum, const G3D::CoordinateFrame &itemFrame, G3D::RenderDevice *rd);
  virtual void selectionMade(int itemNum);

private:
  LoadFilePicker(const LoadFilePicker &);
  LoadFilePicker& operator=(const LoadFilePicker &);

  G3D::Array<std::string> _fileNames;
  G3D::Array<std::string> _baseNames;
  G3D::GFontRef           _font;
};


} // end namespace

#endif

