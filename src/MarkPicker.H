/**
 * /author Dan Keefe (dfk)
 *
 * /file  MarkPicker.H
 * /brief 
 *
 */ 

#ifndef MARKPICKER_H
#define MARKPICKER_H

#include "PickerWidget.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class MarkPicker> MarkPickerRef;
/**  
*/
class MarkPicker : public PickerWidget
{
public:
  MarkPicker(GfxMgrRef              gfxMgr,
             EventMgrRef            eventMgr,
             //ForceNetInterface*     forceNetInterface,
             HCIMgrRef              hciMgr,
             G3D::Array<std::string>     markNames,
             BrushRef               brush,
             CavePaintingCursorsRef cursors,
             G3D::Array<std::string>     btnDownEvents,
             G3D::Array<std::string>     trackerEvents,
             G3D::Array<std::string>     btnUpEvents);
  
  virtual ~MarkPicker();

  virtual void drawItem(int itemNum, const G3D::CoordinateFrame &itemFrame, G3D::RenderDevice *rd);
  virtual void selectionMade(int itemNum);

private:
  MarkPicker(const MarkPicker &);
  MarkPicker& operator=(const MarkPicker &);
  
  G3D::Array<std::string> _markNames;
  G3D::GFontRef           _font;
};


} // end namespace

#endif

