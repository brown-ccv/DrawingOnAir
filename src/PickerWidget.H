/**
 * /author Dan Keefe (dfk)
 *
 * /file  PickerWidget.H
 * /brief Base class for 3D widgets where you pick one thing from a selection laid
 *        out like a table.
 *
 */ 

#ifndef PICKERWIDGET_H
#define PICKERWIDGET_H

//#include "VRBase.H"
//#include <VRG3DBaseApp.h>
#include <GfxMgr.H>
#include <EventMgr.H>


#include "Brush.H"
#include "CavePaintingCursors.H"
#include "ForceNetInterface.H"
#include "HCIMgr.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class PickerWidget> PickerWidgetRef;
/**  
*/
class PickerWidget : public WidgetHCI
{
public:

  enum LayoutType {
    AUTO_LAYOUT = 0
  };

  PickerWidget(MinVR::GfxMgrRef              gfxMgr,
               MinVR::EventMgrRef            eventMgr,
               //ForceNetInterface*     forceNetInterface,
               HCIMgrRef              hciMgr,
               BrushRef               brush,
               CavePaintingCursorsRef cursors,
               G3D::Array<std::string>     btnDownEvents,
               G3D::Array<std::string>     trackerEvents,
               G3D::Array<std::string>     btnUpEvents,
               G3D::Array<std::string>     handBtnEvents,
               bool                   pointerActivates, 
               G3D::Vector3                itemSize,
               G3D::Vector2                itemSpacing,
               int                    numItems,
               int                    numColumns=AUTO_LAYOUT);
  
  virtual ~PickerWidget();

  /// You should override at least one of drawItem() or poseItem()
  virtual void drawItem(int itemNum, const G3D::CoordinateFrame &itemFrame, G3D::RenderDevice *rd) {}
  virtual void poseItem(int itemNum, const G3D::CoordinateFrame &itemFrame, G3D::Array<G3D::PosedModel::Ref> &posedModels) {}

  /// Override this to respond a selection once it is made
  virtual void selectionMade(int itemNum) {}

  virtual void activate();
  
  void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);
  void pose(G3D::Array<G3D::PosedModel::Ref> &posedModels, const G3D::CoordinateFrame &virtualToRoomSpace);

  void trackerMove(MinVR::EventRef e);
  void pickBtnPressed(MinVR::EventRef e);
  virtual void pickBtnReleased(MinVR::EventRef e);
  void trackerDrag(MinVR::EventRef e);

  void spaceKey(MinVR::EventRef e) { spacePressed(); }
  void cKey(MinVR::EventRef e) { cPressed(); }
  void vKey(MinVR::EventRef e) { vPressed(); }
  void handBtn(MinVR::EventRef e) { handBtnPressed(); }

  virtual void spacePressed() {}
  virtual void cPressed() {}
  virtual void vPressed() {}
  virtual void handBtnPressed() {}

  virtual void startWidget() {}
  virtual void closeWidget() {}

  virtual void show();
  virtual void hide();

  virtual bool pointerOverWidget(G3D::Vector3 pointerPosRoomSpace);

protected:

  void deactivate();
  void generateLayout();  

  bool                _hidden;
  BrushRef            _brush;
  CavePaintingCursorsRef _cursors;
  MinVR::EventMgrRef         _eventMgr;
  //ForceNetInterface*  _forceNetInterface;
  MinVR::GfxMgrRef           _gfxMgr;
  int                 _numItems;
  G3D::Vector3             _itemSize;
  G3D::Vector2             _itemSpacing;
  G3D::Array<G3D::Vector3>      _itemPos;
  G3D::AABox               _pickerBBox;
  G3D::Array<G3D::AABox>        _boundingBoxes;
  G3D::CoordinateFrame     _frame;
  int                 _curSelected;  
  MinVR::FsaRef              _fsa;
  G3D::CoordinateFrame     _moveOffset;
  bool                _firstMove;
  int                 _drawID;
  int                 _poseID;
  int                 _numCols;
  bool                _pointerActivates;

private:
  PickerWidget(const PickerWidget &);
  PickerWidget& operator=(const PickerWidget &);
};


} // end namespace

#endif

