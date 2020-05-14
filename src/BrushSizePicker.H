/**
 * /author Dan Keefe (dfk)
 *
 * /file  BrushSizePicker.H
 * /brief 
 *
 */ 

#ifndef BRUSHSIZEPICKER_H
#define BRUSHSIZEPICKER_H

#include "PickerWidget.H"
#include "Brush.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class BrushSizePicker> BrushSizePickerRef;
/**  
*/
class BrushSizePicker : public PickerWidget
{
public:
  BrushSizePicker(MinVR::GfxMgrRef              gfxMgr,
                  MinVR::EventMgrRef            eventMgr,
                  //ForceNetInterface*     forceNetInterface,
                  HCIMgrRef              hciMgr,
                  BrushRef               brush,
                  CavePaintingCursorsRef cursors,
                  G3D::Array<std::string>     btnDownEvents,
                  G3D::Array<std::string>     trackerEvents,
                  G3D::Array<std::string>     btnUpEvents,
                  double                 minSize,
                  double                 maxSize,
                  int                    numChoices);
  
  virtual ~BrushSizePicker();

  virtual void drawItem(int itemNum, const G3D::CoordinateFrame &itemFrame, G3D::RenderDevice *rd);
  virtual void selectionMade(int itemNum);

private:

  BrushSizePicker(const BrushSizePicker &);
  BrushSizePicker& operator=(const BrushSizePicker &);

  double _minSize;
  double _maxSize;
  double _numChoices;
};


} // end namespace

#endif

