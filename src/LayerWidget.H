/**
 * /author Dan Keefe (dfk)
 *
 * /file  LayerWidget.H
 * /brief 
 *
 */ 

#ifndef LAYERWIDGET_H
#define LAYERWIDGET_H

#include "Artwork.H"
#include "Brush.H"
#include "PickerWidget.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class LayerWidget> LayerWidgetRef;
/**  
*/
class LayerWidget : public PickerWidget
{
public:
  LayerWidget(ArtworkRef             artwork,
    MinVR::GfxMgrRef              gfxMgr,
    MinVR::EventMgrRef            eventMgr,
             // ForceNetInterface*     forceNetInterface,
              HCIMgrRef              hciMgr,
              BrushRef               brush,
              CavePaintingCursorsRef cursors,
              G3D::Array<std::string>     btnDownEvents,
              G3D::Array<std::string>     trackerEvents,
              G3D::Array<std::string>     btnUpEvents);
  
  virtual ~LayerWidget();

  virtual void activate();

  virtual void drawItem(int itemNum, const G3D::CoordinateFrame &itemFrame, G3D::RenderDevice *rd);
  virtual void selectionMade(int itemNum);
  virtual void pickBtnReleased(MinVR::EventRef e);
  virtual void show();

private:
  LayerWidget(const LayerWidget &);
  LayerWidget& operator=(const LayerWidget &);

  ArtworkRef _artwork;
  G3D::GFontRef   _font;
};


} // end namespace

#endif

