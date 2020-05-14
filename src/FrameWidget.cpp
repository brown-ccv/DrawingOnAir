#include "FrameWidget.H"
#include <ConfigVal.H>
using namespace G3D;
namespace DrawOnAir {

  
FrameWidget::FrameWidget(ArtworkRef             artwork,
  MinVR::GfxMgrRef              gfxMgr,
  MinVR::EventMgrRef            eventMgr,
                       //  ForceNetInterface*     forceNetInterface,
                         HCIMgrRef              hciMgr,
                         BrushRef               brush,
                         CavePaintingCursorsRef cursors,
                         Array<std::string>     btnDownEvents,
                         Array<std::string>     trackerEvents,
                         Array<std::string>     btnUpEvents) :
  PickerWidget(gfxMgr,
               eventMgr,
               //forceNetInterface,
               hciMgr,
               brush,
               cursors,
               btnDownEvents,
               trackerEvents,
               btnUpEvents,
               Array<std::string>(),
               true,
    MinVR::ConfigVal("FrameWidget_ItemSize", Vector3(0.3, 0.03, 0.03), false),
    MinVR::ConfigVal("Picker_ItemSpacing", Vector2(0.01, 0.01), false),
               1, 1)
{
  _artwork = artwork;
  _font = gfxMgr->getDefaultFont();
  _frameToPaste = -1;
}

FrameWidget::~FrameWidget()
{
}

void
FrameWidget::show()
{
  if (_hidden) {
    // regenerate layout
    _numItems = _artwork->getNumFrames()+2;
    generateLayout();
    
    PickerWidget::show();
  }
}

void
FrameWidget::activate()
{
  // regenerate layout
  _numItems = _artwork->getNumFrames()+2;
  generateLayout();

  PickerWidget::activate();
}

void
FrameWidget::drawItem(int itemNum, const CoordinateFrame &itemFrame, RenderDevice *rd)
{
  rd->pushState();

  if (itemNum == _numItems-1) {
    Color3 bgCol = Color3(0.61, 0.72, 0.92);
    Vector3 halfSize = _itemSize / 2.0;

    rd->pushState();
    rd->setColor(bgCol);
    rd->setObjectToWorldMatrix(itemFrame);
    rd->disableLighting();
    rd->setCullFace(RenderDevice::CULL_NONE);    
    rd->beginPrimitive(PrimitiveType::QUADS);
    rd->sendVertex(Vector3(-halfSize[0],  halfSize[1], 0));
    rd->sendVertex(Vector3(-halfSize[0], -halfSize[1], 0));
    rd->sendVertex(Vector3( halfSize[0], -halfSize[1], 0));
    rd->sendVertex(Vector3( halfSize[0],  halfSize[1], 0));
    rd->endPrimitive();
    rd->popState();

    CoordinateFrame textFrame = itemFrame;
    textFrame.translation += Vector3(0,0,0.001);

    _font->draw3D(rd, "Close", textFrame, 0.8*_itemSize[1], 
                  Color3(0.1, 0.1, 0.44), Color4::clear(),
                  GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);    
  }
  else if (itemNum == _numItems-2) {
    Color3 bgCol = Color3(0.61, 0.72, 0.92);
    Vector3 halfSize = _itemSize / 2.0;

    rd->pushState();
    rd->setColor(bgCol);
    rd->setObjectToWorldMatrix(itemFrame);
    rd->disableLighting();
    rd->setCullFace(RenderDevice::CULL_NONE);    
    rd->beginPrimitive(PrimitiveType::QUADS);
    rd->sendVertex(Vector3(-halfSize[0],  halfSize[1], 0));
    rd->sendVertex(Vector3(-halfSize[0], -halfSize[1], 0));
    rd->sendVertex(Vector3( halfSize[0], -halfSize[1], 0));
    rd->sendVertex(Vector3( halfSize[0],  halfSize[1], 0));
    rd->endPrimitive();
    rd->popState();

    CoordinateFrame textFrame = itemFrame;
    textFrame.translation += Vector3(0,0,0.001);

    _font->draw3D(rd, "New Frame..", textFrame, 0.8*_itemSize[1],  
                  Color3(0.1, 0.1, 0.44), Color4::clear(),
                  GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);      
  }
  else {
    if (itemNum == 0) {
      CoordinateFrame tf = itemFrame;
      tf.translation += Vector3(0,2.5*_itemSize[1],0);
      _gfxMgr->getDefaultFont()->draw3D(rd, "To copy an entire frame press C to copy",
               tf, 0.8*_itemSize[1],
               Color3::white(), Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
      
      tf.translation += Vector3(0,-_itemSize[1],0);
      _gfxMgr->getDefaultFont()->draw3D(rd, "then select a new frame and press V to paste.",
               tf, 0.8*_itemSize[1],
               Color3::white(), Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
    }

    int frameNum = itemNum;
    Color3 bgCol = Color3::white();
    Color3 textCol = Color3(0.1, 0.1, 0.44);
    if (frameNum == _brush->state->frameIndex) {
      bgCol = Color3(0.1, 0.1, 0.44);
      textCol = Color3::white();
    }
    Vector3 halfSize = _itemSize / 2.0;

    rd->pushState();
    rd->setColor(bgCol);
    rd->setObjectToWorldMatrix(itemFrame);
    rd->disableLighting();
    rd->setCullFace(RenderDevice::CULL_NONE);    
    rd->beginPrimitive(PrimitiveType::QUADS);
    rd->sendVertex(Vector3(-halfSize[0],  halfSize[1], 0));
    rd->sendVertex(Vector3(-halfSize[0], -halfSize[1], 0));
    rd->sendVertex(Vector3( halfSize[0], -halfSize[1], 0));
    rd->sendVertex(Vector3( halfSize[0],  halfSize[1], 0));
    rd->endPrimitive();
    rd->popState();

    CoordinateFrame textFrame = itemFrame;
    textFrame.translation += Vector3(0,0,0.001);
    _font->draw3D(rd, "Frame " + MinVR::intToString(frameNum), textFrame, 0.8*_itemSize[1],
                  textCol, Color4::clear(),
                  GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
  }

  rd->popState();
}

void
FrameWidget::selectionMade(int itemNum)
{
  if (itemNum == _numItems-1) {
  }
  else if (itemNum == _numItems-2) {
    _artwork->setNumFrames(_artwork->getNumFrames()+1);
    _brush->state->frameIndex = itemNum;
    _artwork->showFrame(_brush->state->frameIndex);
    _numItems = _artwork->getNumFrames()+2;
    generateLayout();
  }
  else {
    _brush->state->frameIndex = itemNum;
    _artwork->showFrame(_brush->state->frameIndex);
  }
}

void
FrameWidget::pickBtnReleased(MinVR::EventRef e)
{
  if (_curSelected == _numItems-1) {
    hide();
    //deactivate();
  }
}

void
FrameWidget::cPressed()
{
  _frameToPaste = _brush->state->frameIndex;
}

void
FrameWidget::vPressed()
{
  if (_frameToPaste != -1) {
    // Don't paste, if it would paste on to the same frame, assume that is a mistake
    if (_frameToPaste != _brush->state->frameIndex) {
      _artwork->copyFrameToFrame(_frameToPaste, _brush->state->frameIndex);
    }
  }
}

} // end namespace

