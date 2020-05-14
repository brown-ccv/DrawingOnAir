
#include "PickerWidget.H"
#include <ConfigVal.H>
#include <G3DOperators.h>

#define NOTHING_SELECTED -2
#define PALETTE_SELECTED -1

using namespace G3D;
namespace DrawOnAir {


PickerWidget::PickerWidget(MinVR::GfxMgrRef              gfxMgr,
                           MinVR::EventMgrRef            eventMgr,
                           //ForceNetInterface*     forceNetInterface,
                           HCIMgrRef              hciMgr,
                           BrushRef               brush,
                           CavePaintingCursorsRef cursors,
                           Array<std::string>     btnDownEvents,
                           Array<std::string>     trackerEvents,
                           Array<std::string>     btnUpEvents,
                           Array<std::string>     handBtnEvents,
                           bool                   pointerActivates,
                           Vector3                itemSize,
                           Vector2                itemSpacing,
                           int                    numItems,
                           int                    numColumns) : WidgetHCI(hciMgr)
{
  _brush = brush;
  _eventMgr = eventMgr;
  //_forceNetInterface = forceNetInterface;
  _gfxMgr = gfxMgr;
  _cursors = cursors;
  _pointerActivates = pointerActivates;
  _itemSize = itemSize;
  _itemSpacing = itemSpacing;
  _numItems = numItems;
  _numCols = numColumns;
  _frame.translation = MinVR::ConfigVal("Picker_Center", Vector3::zero(), false);
  _hidden = _pointerActivates;
  generateLayout();

  _fsa = new MinVR::Fsa("_name");
  _fsa->addState("Start");
  _fsa->addState("Dragging");
  _fsa->addArc("PickBtnPressed", "Start", "Start", btnDownEvents);
  _fsa->addArcCallback("PickBtnPressed", this, &PickerWidget::pickBtnPressed);
  _fsa->addArc("PickBtnReleased", "Start", "Start", btnUpEvents);
  _fsa->addArcCallback("PickBtnReleased", this, &PickerWidget::pickBtnReleased);
  _fsa->addArc("TrackerMove", "Start", "Start", trackerEvents);
  _fsa->addArcCallback("TrackerMove", this, &PickerWidget::trackerMove);

  // trap the space key: in the default case, no action is tied to this, but some widgets, like
  // the brushtip picker cycle through palettes when space is pressed
  _fsa->addArc("SpacePressed", "Start", "Start", MinVR::splitStringIntoArray("kbd_SPACE_down"));
  _fsa->addArcCallback("SpacePressed", this, &PickerWidget::spaceKey);
  _fsa->addArc("CPressed", "Start", "Start", MinVR::splitStringIntoArray("kbd_C_down"));
  _fsa->addArcCallback("CPressed", this, &PickerWidget::cKey);
  _fsa->addArc("VPressed", "Start", "Start", MinVR::splitStringIntoArray("kbd_V_down"));
  _fsa->addArcCallback("VPressed", this, &PickerWidget::vKey);
  _fsa->addArc("HandBtnPressed", "Start", "Start", handBtnEvents);
  _fsa->addArcCallback("HandBtnPressed", this, &PickerWidget::handBtn);


  _fsa->addArc("TrackerDrag", "Dragging", "Dragging", trackerEvents);
  _fsa->addArcCallback("TrackerDrag", this, &PickerWidget::trackerDrag);
  _fsa->addArc("DragOff", "Dragging", "Start", btnUpEvents);
  
}

PickerWidget::~PickerWidget()
{
  hide();
}

void
PickerWidget::show()
{
  _hidden = false;
  _drawID = _gfxMgr->addDrawCallback(this, &PickerWidget::draw);
  _poseID = _gfxMgr->addPoseCallback(this, &PickerWidget::pose);
}

void
PickerWidget::hide()
{
  _hidden = true;
  _gfxMgr->removeDrawCallback(_drawID);
  _gfxMgr->removePoseCallback(_poseID);
}

void
PickerWidget::activate()
{
  _eventMgr->addFsaRef(_fsa);
  _cursors->setBrushCursor(CavePaintingCursors::POINTER_CURSOR);
  if (!_pointerActivates) {
    show();
  }
  /*if (_forceNetInterface != NULL) {
    _forceNetInterface->startPlaneEffect();
  }*/
  startWidget();
}

void
PickerWidget::deactivate()
{
  releaseFocusWithHCIMgr();
  _eventMgr->removeFsaRef(_fsa);
  if (!_pointerActivates) {
    hide();
  }
  /*if (_forceNetInterface != NULL) {
    _forceNetInterface->stopPlaneEffect();
  }*/
}

void
PickerWidget::generateLayout()
{
  _curSelected = NOTHING_SELECTED;
  _itemPos.clear();
  _boundingBoxes.clear();
  _pickerBBox = AABox::zero();

  int nrows, ncols;
  if (_numCols == AUTO_LAYOUT) {
    nrows = iRound(sqrt((double)_numItems));
    if (nrows < 1)
      nrows = 1;
    ncols = iCeil((float)_numItems / (float)nrows);
  }
  else {
    ncols = _numCols;
    nrows = iCeil((float)_numItems / (float)ncols);
  }

  double xSpacing = _itemSpacing[0];
  double ySpacing = _itemSpacing[1];
  
  double y = (_itemSize[1]+ySpacing)*(float)nrows*0.5 - (_itemSize[1]+ySpacing)/2.0;
  for (int r=0;r<nrows;r++) {
    double x=-(_itemSize[0]+xSpacing)*(float)ncols*0.5 + (_itemSize[0]+xSpacing)/2.0;
    for (int c=0;c<ncols;c++) {
      if (_itemPos.size() < _numItems) {
        Vector3 centerPt(x,y,0); 
        _itemPos.append(centerPt);
        
        // compute bounding box for item
        Vector3 halfBounds = _itemSize * 0.5;
        AABox b(centerPt + halfBounds);
        MinVR::growAABox(b, centerPt - halfBounds);
        _boundingBoxes.append(b);

        // make the palette's bounding box just a bit bigger than the extents of the items
        Vector3 spacing(xSpacing, ySpacing, 0);
        AABox b2(centerPt + halfBounds + spacing);
        MinVR::growAABox(b2,centerPt - halfBounds - spacing);
        MinVR::growAABox(_pickerBBox, b2);
      }
      x += _itemSize[0] + xSpacing;
    }
    y -= _itemSize[1] + ySpacing;
  }  
}



void
PickerWidget::pickBtnPressed(MinVR::EventRef e)
{
  if (_curSelected == PALETTE_SELECTED) {
    _firstMove = true;
    _fsa->jumpToState("Dragging");
  }
  else {
    if (_curSelected != NOTHING_SELECTED) {
      selectionMade(_curSelected);
    }
  }
}

void
PickerWidget::pickBtnReleased(MinVR::EventRef e)
{
  if (!_pointerActivates) {
    deactivate();
    closeWidget();
  }
}


void
PickerWidget::trackerMove(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  Vector3 trackerInPickerSpace = _frame.pointToObjectSpace(e->getCoordinateFrameData().translation);
    
  _curSelected = NOTHING_SELECTED;
  if (_pickerBBox.contains(trackerInPickerSpace)) {
    _curSelected = PALETTE_SELECTED;
  }
  else if (_pointerActivates) {
    deactivate();
    return;
  }
  for (int i=0;i<_boundingBoxes.size();i++) {
    if (_boundingBoxes[i].contains(trackerInPickerSpace)) {
      _curSelected = i;
    }
  }
}

void
PickerWidget::trackerDrag(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();

  if (_firstMove) {
    _moveOffset = e->getCoordinateFrameData().inverse() * _frame;
  }
  _firstMove = false;
  _frame = e->getCoordinateFrameData() * _moveOffset;
}


void
PickerWidget::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  rd->pushState();
  rd->setObjectToWorldMatrix(_frame);

  // Draw widget
  if (_curSelected != PALETTE_SELECTED) {
    Draw::box(_pickerBBox, rd, Color4::clear(), Color3(0.61,0.72,0.92));
  }

  // Draw a box around something if it is selected
  if (_curSelected != NOTHING_SELECTED) {
    if (_curSelected == PALETTE_SELECTED) {
      Draw::box(_pickerBBox, rd, Color4::clear(), Color3::yellow());
    }
    else {
      Draw::box(_boundingBoxes[_curSelected], rd, Color4::clear(), Color3::yellow());
    }
  }
  rd->popState();

  // Draw each item
  for (int i=0;i<_itemPos.size();i++) {
    CoordinateFrame itemFrame = _frame * CoordinateFrame(_itemPos[i]);
    drawItem(i, itemFrame, rd);
  }
}

void
PickerWidget::pose(Array<PosedModel::Ref> &posedModels, const CoordinateFrame &virtualToRoomSpace)
{
  // Pose each item
  for (int i=0;i<_itemPos.size();i++) {
    CoordinateFrame itemFrame = _frame * CoordinateFrame(_itemPos[i]);
    poseItem(i, itemFrame, posedModels);
  }
}

bool
PickerWidget::pointerOverWidget(Vector3 pointerPosRoomSpace)
{
  if (_hidden) {
    return false;
  }  
  Vector3 trackerInPickerSpace = _frame.pointToObjectSpace(pointerPosRoomSpace);
  return (_pickerBBox.contains(trackerInPickerSpace));
}



} // end namespace

