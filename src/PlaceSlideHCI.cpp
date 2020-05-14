
#include "PlaceSlideHCI.H"
#include <G3DOperators.h>
#include <ConfigVal.H>
using namespace G3D;
namespace DrawOnAir {

PlaceSlideHCI::PlaceSlideHCI(Array<std::string>     brushOnTriggers,
                             Array<std::string>     brushMotionTriggers, 
                             Array<std::string>     brushOffTriggers,
                             Array<std::string>     handMotionTriggers, 
                             BrushRef               brush,
                             CavePaintingCursorsRef cursors,
  MinVR::EventMgrRef            eventMgr,
  MinVR::GfxMgrRef              gfxMgr)
{
  _brush = brush;
  _cursors = cursors;
  _eventMgr = eventMgr;
  _gfxMgr = gfxMgr;
  _enabled = false;

  _fsa = new MinVR::Fsa("PlaceSlideHCI");
  _fsa->addState("Start");
  _fsa->addState("Drawing");

  _fsa->addArc("BrushMove", "Start", "Start", brushMotionTriggers);
  _fsa->addArcCallback("BrushMove", this, &PlaceSlideHCI::brushMotion);

  _fsa->addArc("BrushOn", "Start", "Drawing", brushOnTriggers);
  _fsa->addArcCallback("BrushOn", this, &PlaceSlideHCI::brushOn);
  _fsa->addArc("BrushOff", "Drawing", "Start", brushOffTriggers);
  _fsa->addArcCallback("BrushOff", this, &PlaceSlideHCI::brushOff);

  _fsa->addArc("HeadMove1", "Start", "Start", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove1", this, &PlaceSlideHCI::headMotion);
  _fsa->addArc("HeadMove2", "Drawing", "Drawing", MinVR::splitStringIntoArray("Head_Tracker"));
  _fsa->addArcCallback("HeadMove2", this, &PlaceSlideHCI::headMotion);

  _fsa->addArc("HandMove1", "Start", "Start", handMotionTriggers);
  _fsa->addArcCallback("HandMove1", this, &PlaceSlideHCI::handMotion);
  _fsa->addArc("HandMove2", "Drawing", "Drawing", handMotionTriggers);
  _fsa->addArcCallback("HandMove2", this, &PlaceSlideHCI::handMotion);
}

PlaceSlideHCI::~PlaceSlideHCI()
{
}

void
PlaceSlideHCI::setEnabled(bool b) 
{
  if ((b) && (!_enabled)) {
    _eventMgr->addFsaRef(_fsa);
    _cursors->setBrushCursor(CavePaintingCursors::DEFAULT_CURSOR);
    _brush->state->maxPressure = MinVR::ConfigVal("PlaceSlideHCI_MaxPressure", 1.0, false);
    _brush->state->brushInterface = "PlaceSlide";
    _dcbid = _gfxMgr->addDrawCallback(this, &PlaceSlideHCI::draw);
  }
  else if ((!b) && (_enabled)) {
    _eventMgr->removeFsaRef(_fsa);
    _gfxMgr->removeDrawCallback(_dcbid);
  }
  _enabled = b;
}

bool
PlaceSlideHCI::canReleaseFocus()
{
  return (_fsa->getCurrentState() == "Start");
}

void 
PlaceSlideHCI::headMotion(MinVR::EventRef e)
{
  _brush->state->headFrame = e->getCoordinateFrameData();
}

void 
PlaceSlideHCI::handMotion(MinVR::EventRef e)
{
  _brush->state->handFrame = e->getCoordinateFrameData();
}

void 
PlaceSlideHCI::brushMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
}

void 
PlaceSlideHCI::brushOn(MinVR::EventRef e)
{
  _brush->makeNextMarkASlide(_slideName);
  _brush->startNewMark();

  Vector3 b = _brush->state->frameInRoomSpace.translation;
  Vector3 h = _brush->state->handFrame.translation;

  Vector3 x = (b-h).unit();
  Vector3 z = _brush->state->frameInRoomSpace.rotation.column(2);
  z = (z - z.dot(x)*x).unit();
  Vector3 y = (z.cross(x)).unit();

  Matrix3 rot(x[0], y[0], z[0],
              x[1], y[1], z[1],
              x[2], y[2], z[2]);
  rot.orthonormalize();
  CoordinateFrame frame(rot, 0.5*(b+h));
  double s = (b-h).length();

  CoordinateFrame fOrig = _brush->state->frameInRoomSpace;
  double sOrig = _brush->state->size;

  _brush->state->frameInRoomSpace = frame;
  _brush->state->size = s;
  _brush->addSampleToMark();

  _brush->state->frameInRoomSpace = fOrig;
  _brush->state->size = sOrig;

  _brush->endMark();

}

void
PlaceSlideHCI::brushOff(MinVR::EventRef e)
{
}

void
PlaceSlideHCI::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  Vector3 b = _brush->state->frameInRoomSpace.translation;
  Vector3 h = _brush->state->handFrame.translation;

  Vector3 x = (b-h).unit();
  Vector3 z = _brush->state->frameInRoomSpace.rotation.column(2);
  z = (z - z.dot(x)*x).unit();
  Vector3 y = (z.cross(x)).unit();

  double s = (b-h).length()/2.0;

  rd->pushState();

  rd->disableLighting();
  rd->setColor(Color3::white());
  rd->setAlphaTest(RenderDevice::ALPHA_GREATER, 0.05);
  rd->setCullFace(RenderDevice::CULL_NONE);

  Texture::Ref tex = _gfxMgr->getTexture(_slideName);
  rd->setTexture(0, tex);
  
  rd->beginPrimitive(PrimitiveType::QUADS);
  rd->setTexCoord(0, Vector2(0,1));
  rd->sendVertex(b - s*y);
  rd->setTexCoord(0, Vector2(1,1));
  rd->sendVertex(h - s*y);
  rd->setTexCoord(0, Vector2(1,0));
  rd->sendVertex(h + s*y);
  rd->setTexCoord(0, Vector2(0,0));
  rd->sendVertex(b + s*y);
  rd->endPrimitive();

  rd->popState();
}




} // end namespace
