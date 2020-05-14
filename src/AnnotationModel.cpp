
#include "AnnotationModel.H"
#include <ConfigVal.H>

using namespace G3D;

namespace DrawOnAir {

AnnotationModel::AnnotationModel(MinVR::GfxMgrRef gfxMgr)
{
  _gfxMgr = gfxMgr;
  _nextID = 0;
  _frame = 0;
  _screenSize = MinVR::ConfigVal("Annotation_ScreenSpaceTextSize", 0.035, false);
  _constSize = MinVR::ConfigVal("Annotation_Constant3DTextSize", 0.0, false);
}

AnnotationModel::~AnnotationModel()
{
}

int
AnnotationModel::addAnnotation(const std::string &text, Vector3 startPt, Vector3 endPt, 
                               Color3 color, int layerIndex, int frameIndex)
{
  _nextID++;
  _startPt.set(_nextID, startPt);
  _endPt.set(_nextID, endPt);
  _color.set(_nextID, color);
  _text.set(_nextID, text);
  _textYAlign.set(_nextID, GFont::YALIGN_BOTTOM);
  _textScale.set(_nextID, _screenSize);
  _layers.set(_nextID, layerIndex);
  _frames.set(_nextID, frameIndex);
  return _nextID;
}

void
AnnotationModel::editAnnotation(int id, const std::string &text, Vector3 startPt, Vector3 endPt, Color3 color)
{
  _startPt.set(id, startPt);
  _endPt.set(id, endPt);
  _color.set(id, color);
  _text.set(id, text);  
}

void
AnnotationModel::deleteAnnotation(int id)
{
  _startPt.remove(id);
  _endPt.remove(id);
  _color.remove(id);
  _text.remove(id);    
}


void
AnnotationModel::setMovieFrame(int frameNum) 
{
  _frame = frameNum;
}


void
AnnotationModel::pose(Array<PosedModel::Ref> &posedModels, const CoordinateFrame &virtualToRoomSpace)
{
  Array<int> keys = _startPt.getKeys();
  for (int i=0;i<keys.size();i++) {
    if (_constSize == 0.0) {
      double z = _gfxMgr->virtualPointToRoomSpace(_endPt[keys[i]])[2];
      Vector3 p1 = _gfxMgr->screenPointToRoomSpaceZEqualsPlane(Vector2(0,0), z);
      Vector3 p2 = _gfxMgr->screenPointToRoomSpaceZEqualsPlane(Vector2(0.035,0), z);
      _textScale.set(keys[i], (p2-p1).length());

      Vector2 spEnd = _gfxMgr->roomSpacePointToScreenPoint(_gfxMgr->virtualPointToRoomSpace(_endPt[keys[i]]));
      Vector2 spStart = _gfxMgr->roomSpacePointToScreenPoint(_gfxMgr->virtualPointToRoomSpace(_startPt[keys[i]]));
      Vector2 lineDir = spEnd - spStart;
      
      if (lineDir[1] > 0.0) {
        _textYAlign.set(keys[i], GFont::YALIGN_BOTTOM);
      }
      else {
        _textYAlign.set(keys[i], GFont::YALIGN_TOP);
      }
    }
    else {
      Vector3 s = _gfxMgr->virtualPointToRoomSpace(_startPt[keys[i]]);
      Vector3 e = _gfxMgr->virtualPointToRoomSpace(_endPt[keys[i]]);
      Vector3 dir = (e-s).unit();
      if (dir[1] > 0.0) {
        _textYAlign.set(keys[i], GFont::YALIGN_BOTTOM);
      }
      else {
        _textYAlign.set(keys[i], GFont::YALIGN_TOP);
      }      
    }
  }
}

void
AnnotationModel::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  Array<int> keys = _startPt.getKeys();

  rd->pushState();

  rd->setObjectToWorldMatrix(virtualToRoomSpace);
  rd->disableLighting();
  
  rd->beginPrimitive(PrimitiveType::LINES);
  for (int i=0;i<keys.size();i++) {
    if ((!_hiddenLayers.contains(_layers[keys[i]])) && (_frames[keys[i]] == _frame)) {
      rd->setColor(_color[keys[i]]);
      rd->sendVertex(_startPt[keys[i]]);
      rd->sendVertex(_endPt[keys[i]]);
    }
  }
  rd->endPrimitive();

  rd->popState();

  for (int i=0;i<keys.size();i++) {
    if ((!_hiddenLayers.contains(_layers[keys[i]])) && (_frames[keys[i]] == _frame)) {
      //f.translation = _endPt[keys[i]];
      CoordinateFrame f;
      f.translation = _gfxMgr->virtualPointToRoomSpace(_endPt[keys[i]]);
      double size = _constSize;
      if (size == 0.0) {
        size = _textScale[keys[i]];
      }
      _gfxMgr->getDefaultFont()->draw3D(rd, _text[keys[i]], f, 
                                        size, _color[keys[i]], Color4::clear(),
                                        GFont::XALIGN_CENTER, _textYAlign[keys[i]]);
    }
  }
}


/** Each time you change the format of this, increase the
    SERIALIZE_VERSION_NUMBER and respond accordingly in the
    deserialize method.  In this method, you can safely get rid of the
    old code because we don't want to create files in an old format.
*/
#define SERIALIZE_VERSION_NUMBER 0
void
AnnotationModel::serialize(G3D::BinaryOutput &b)
{
  b.writeInt8(SERIALIZE_VERSION_NUMBER);

  G3D::Array<int> keys = _startPt.getKeys();
  b.writeInt32(keys.size());
  for (int i=0;i<keys.size();i++) {
    b.writeString(_text[keys[i]]);
    _startPt[keys[i]].serialize(b);
    _endPt[keys[i]].serialize(b);
  }
}


/** Never delete the code from an old version from this method so we
    can always stay backwards compatable.  Important, since this is in
    binary, and we can't really reverse engineer the file format.
*/
void
AnnotationModel::deserialize(G3D::BinaryInput &b)
{
  int version = b.readInt8();

  if (version == 0) {
    int num = b.readInt32();
    for (int i=0;i<num;i++) {
      std::string text = b.readString();
      G3D::Vector3 startPt, endPt;
      startPt.deserialize(b);
      endPt.deserialize(b);
      addAnnotation(text, startPt, endPt, Color3::black(), 0, 0);
    }
  }
}


} // end namespace

