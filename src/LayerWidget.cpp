#include "LayerWidget.H"
#include <ConfigVal.H>
using namespace G3D;
namespace DrawOnAir {

  
LayerWidget::LayerWidget(ArtworkRef             artwork,
  MinVR::GfxMgrRef              gfxMgr,
  MinVR::EventMgrRef            eventMgr,
                     //    ForceNetInterface*     forceNetInterface,
                         HCIMgrRef              hciMgr,
                         BrushRef               brush,
                         CavePaintingCursorsRef cursors,
                         Array<std::string>     btnDownEvents,
                         Array<std::string>     trackerEvents,
                         Array<std::string>     btnUpEvents) :
  PickerWidget(gfxMgr,
               eventMgr,
              // forceNetInterface,
               hciMgr,
               brush,
               cursors,
               btnDownEvents,
               trackerEvents,
               btnUpEvents,
               Array<std::string>(),
               true,
    MinVR::ConfigVal("LayerWidget_ItemSize", Vector3(0.3, 0.03, 0.03), false),
    MinVR::ConfigVal("Picker_ItemSpacing", Vector2(0.01, 0.01), false),
               4, 2)
{
  _artwork = artwork;
  _font = gfxMgr->getDefaultFont();
}

LayerWidget::~LayerWidget()
{
}


void
LayerWidget::show()
{
  if (_hidden) {
    // regenerate layout
    _numItems = 2*(_artwork->getMaxLayerID()+2);
    generateLayout();
    
    PickerWidget::show();
  }
}

void
LayerWidget::activate()
{
  // regenerate layout
  _numItems = 2*(_artwork->getMaxLayerID()+2);
  generateLayout();

  PickerWidget::activate();
}

/**

Show Layer 1
Show Layer 2
Show Layer 3
     Create New Layer

**/

  
void
LayerWidget::drawItem(int itemNum, const CoordinateFrame &itemFrame, RenderDevice *rd)
{
  // draw some on-screen instructions?  
  // Like Photoshop layers, you can only draw into one layer at the time, the one in blue
  // but you can have more than one visible at a time (click on the checkbox on the left
  // to show/hide).
  // Animation controls automatically cycle both the drawing layer and the show/hide to 
  // make only one layer active at a time and let you animate through layers.

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

    _font->draw3D(rd, "New Layer..", textFrame, 0.8*_itemSize[1],  
                  Color3(0.1, 0.1, 0.44), Color4::clear(),
                  GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);      
  }
  else if (itemNum % 2 == 0) {
    int layerNum = itemNum/2;

    Color3 bgCol = Color3::white();
    if (layerNum == _brush->state->layerIndex) {
      bgCol = Color3(0.1, 0.1, 0.44);
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

    halfSize = Vector3(_itemSize[1]/3.0, _itemSize[1]/3.0, 0.0);
    
    CoordinateFrame offsetFrame = itemFrame;
    offsetFrame.translation += Vector3(0,0,0.001);

    rd->pushState();
    rd->setObjectToWorldMatrix(offsetFrame);
    rd->setCullFace(RenderDevice::CULL_NONE);
    rd->disableLighting();
    rd->setColor(Color3::white());
    Texture::Ref tex;
    if (_artwork->getHiddenLayers().contains(layerNum)) {
      tex = _gfxMgr->getTexture("LayerHiddenGlyph");
    }
    else {
      tex = _gfxMgr->getTexture("LayerShownGlyph");
    }
    rd->setTexture(0, tex);
    rd->beginPrimitive(PrimitiveType::QUADS);
    rd->setTexCoord(0, Vector2(0,0));
    rd->sendVertex(Vector3(-halfSize[0],  halfSize[1], 0));
    rd->setTexCoord(0, Vector2(0,1));
    rd->sendVertex(Vector3(-halfSize[0], -halfSize[1], 0));
    rd->setTexCoord(0, Vector2(1,1));
    rd->sendVertex(Vector3( halfSize[0], -halfSize[1], 0));
    rd->setTexCoord(0, Vector2(1,0));
    rd->sendVertex(Vector3( halfSize[0],  halfSize[1], 0));
    rd->endPrimitive();
    rd->popState();
  }
  else {
    int layerNum = (itemNum-1)/2;
    Color3 bgCol = Color3::white();
    Color3 textCol = Color3(0.1, 0.1, 0.44);
    if (layerNum == _brush->state->layerIndex) {
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
    _font->draw3D(rd, "Layer " + MinVR::intToString(layerNum), textFrame, 0.8*_itemSize[1],
                  textCol, Color4::clear(),
                  GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
  }

  rd->popState();
}

void
LayerWidget::selectionMade(int itemNum)
{
  if (itemNum == _numItems-1) {
  }
  else if (itemNum == _numItems-2) {
    _artwork->setMaxLayerID(_artwork->getMaxLayerID() + 1);
    _numItems = 2*(_artwork->getMaxLayerID()+2);
    generateLayout();
    _brush->state->layerIndex = _artwork->getMaxLayerID();
  }
  else if (itemNum % 2 == 0) {
    int layerNum = itemNum/2;
    Array<int> hidden = _artwork->getHiddenLayers();
    if (hidden.contains(layerNum)) {
      hidden.fastRemove(hidden.findIndex(layerNum));
      _artwork->setHiddenLayers(hidden);
    }
    else {
      hidden.append(layerNum);
      _artwork->setHiddenLayers(hidden);
    }
  }
  else {
    int layerNum = (itemNum-1)/2;
    _brush->state->layerIndex = layerNum;
  }
  
}

void
LayerWidget::pickBtnReleased(MinVR::EventRef e)
{
  if (_curSelected == _numItems-1) {
    hide();
    //deactivate();
  }
}

} // end namespace

