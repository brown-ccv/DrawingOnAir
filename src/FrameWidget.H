/**
 * /author Dan Keefe (dfk)
 *
 * /file  FrameWidget.H
 * /brief 
 *
 */ 

#ifndef FRAMEWIDGET_H
#define FRAMEWIDGET_H

#include "Artwork.H"
#include "Brush.H"
#include "PickerWidget.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class FrameWidget> FrameWidgetRef;
/**  
*/
class FrameWidget : public PickerWidget
{
public:
  FrameWidget(ArtworkRef             artwork,
    MinVR::GfxMgrRef              gfxMgr,
    MinVR::EventMgrRef            eventMgr,
              //ForceNetInterface*     forceNetInterface,
              HCIMgrRef              hciMgr,
              BrushRef               brush,
              CavePaintingCursorsRef cursors,
              G3D::Array<std::string>     btnDownEvents,
              G3D::Array<std::string>     trackerEvents,
              G3D::Array<std::string>     btnUpEvents);
  
  virtual ~FrameWidget();

  virtual void activate();

  virtual void drawItem(int itemNum, const G3D::CoordinateFrame &itemFrame, G3D::RenderDevice *rd);
  virtual void selectionMade(int itemNum);
  virtual void pickBtnReleased(MinVR::EventRef e);
  virtual void cPressed();
  virtual void vPressed();

  virtual void show();

private:
  FrameWidget(const FrameWidget &);
  FrameWidget& operator=(const FrameWidget &);

  ArtworkRef _artwork;
  G3D::GFontRef   _font;
  int        _frameToPaste;
};


} // end namespace

#endif

