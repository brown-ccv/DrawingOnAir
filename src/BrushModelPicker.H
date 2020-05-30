/**
 * /author Dan Keefe (dfk)
 *
 * /file  BrushModelPicker.H
 * /brief 
 *
 */ 

#ifndef BRUSHMODELPICKER_H
#define BRUSHMODELPICKER_H

#include "PickerWidget.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class BrushModelPicker> BrushModelPickerRef;
/**  
*/
class BrushModelPicker : public PickerWidget
{
public:
  BrushModelPicker(
             GfxMgrRef              gfxMgr,
             EventMgrRef            eventMgr,
             //ForceNetInterface*     forceNetInterface,
             HCIMgrRef              hciMgr,
             G3D::Array<std::string>     brushModelNames,
             BrushRef               brush,
             CavePaintingCursorsRef cursors,
             G3D::Array<std::string>     btnDownEvents,
             G3D::Array<std::string>     trackerEvents,
             G3D::Array<std::string>     btnUpEvents);
  
  virtual ~BrushModelPicker();

  virtual void drawItem(int itemNum, const G3D::CoordinateFrame &itemFrame, G3D::RenderDevice *rd);
  virtual void selectionMade(int itemNum);

private:
  BrushModelPicker(const BrushModelPicker &);
  BrushModelPicker& operator=(const BrushModelPicker &);
  
  G3D::Array<std::string> _brushModelNames;
  G3D::GFontRef           _font;
};


} // end namespace

#endif

