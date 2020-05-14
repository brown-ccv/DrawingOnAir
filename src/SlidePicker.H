/**
 * /author Dan Keefe (dfk)
 *
 * /file  SlidePicker.H
 * /brief 
 *
 */ 

#ifndef SLIDEPICKER_H
#define SLIDEPICKER_H

#include <GfxMgr.H>
#include <EventMgr.H>
#include "PickerWidget.H"
#include "PlaceSlideHCI.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class SlidePicker> SlidePickerRef;
/**  
*/
class SlidePicker : public PickerWidget
{
public:
  SlidePicker(MinVR::GfxMgrRef              gfxMgr,
    MinVR::EventMgrRef            eventMgr,
              //ForceNetInterface*     forceNetInterface,
              HCIMgrRef              hciMgr,
              BrushRef               brush,
              CavePaintingCursorsRef cursors,
              G3D::Array<std::string>     btnDownEvents,
              G3D::Array<std::string>     trackerEvents,
              G3D::Array<std::string>     btnUpEvents,
              PlaceSlideHCIRef       placeSlideHCI);
  
  virtual ~SlidePicker();

  virtual void drawItem(int itemNum, const G3D::CoordinateFrame &itemFrame, G3D::RenderDevice *rd);
  virtual void selectionMade(int itemNum);

  virtual void startWidget();
  virtual void closeWidget();

  static void loadSlideImages(MinVR::GfxMgrRef gfxMgr, G3D::Array<std::string> &keyNames);

private:
  SlidePicker(const SlidePicker &);
  SlidePicker& operator=(const SlidePicker &);

  G3D::Array<std::string> _keyNames;
  PlaceSlideHCIRef   _placeSlideHCI;
  bool               _selectionMade;
};


} // end namespace

#endif

