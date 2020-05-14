
#include "BlendHCI.H"
#include "PointSplatMark.H"
#include <ConfigVal.H>
#include <G3DOperators.h>

using namespace G3D;
//unsigned int hashCode(const double d);

namespace DrawOnAir {

BlendHCI::BlendHCI(Array<std::string>     brushOnTriggers,
                   Array<std::string>     brushMotionTriggers, 
                   Array<std::string>     brushOffTriggers,
                   BrushRef          brush,
                   CavePaintingCursorsRef cursors,
                   MinVR::EventMgrRef            eventMgr,
                   MinVR::GfxMgrRef              gfxMgr)
{
  _brush = brush;
  _cursors = cursors;
  _eventMgr = eventMgr;
  _gfxMgr = gfxMgr;
  _enabled = false;

  _fsa = new MinVR::Fsa("BlendHCI");
  _fsa->addState("Start");
  _fsa->addState("Drawing");
  _fsa->addArc("BrushMove", "Start", "Start", brushMotionTriggers);
  _fsa->addArcCallback("BrushMove", this, &BlendHCI::brushMotion);
  _fsa->addArc("BrushOn", "Start", "Drawing", brushOnTriggers);
  _fsa->addArcCallback("BrushOn", this, &BlendHCI::brushOn);
  _fsa->addArc("BrushDrawMove", "Drawing", "Drawing", brushMotionTriggers);
  _fsa->addArcCallback("BrushDrawMove", this, &BlendHCI::brushDrawMotion);
  _fsa->addArc("BrushPressure", "Drawing", "Drawing", MinVR::splitStringIntoArray("Brush_Pressure"));
  _fsa->addArcCallback("BrushPressure", this, &BlendHCI::brushPressureChange);
  _fsa->addArc("BrushOff", "Drawing", "Start", brushOffTriggers);
  _fsa->addArcCallback("BrushOff", this, &BlendHCI::brushOff);

  double w = MinVR::ConfigVal("BlendToolWidth", 0.025, false)/2.0;
  double h = MinVR::ConfigVal("BlendToolHeight", 0.025, false)/2.0;
  double d = MinVR::ConfigVal("BlendToolDepth", 0.005, false)/2.0;
  _box = Box(Vector3(-w, -h, -d), Vector3(w, h, d));
}

BlendHCI::~BlendHCI()
{
}

void
BlendHCI::setEnabled(bool b) 
{
  if ((b) && (!_enabled)) {
    _eventMgr->addFsaRef(_fsa);
    _cursors->setBrushCursor(CavePaintingCursors::DEFAULT_CURSOR);
    _brush->state->maxPressure = MinVR::ConfigVal("BlendHCI_MaxPressure", 1.0, false);
    _brush->state->brushInterface = "Blend";
  }
  else if ((!b) && (_enabled)) {
    _eventMgr->removeFsaRef(_fsa);
  }
  _enabled = b;
}

bool
BlendHCI::canReleaseFocus()
{
  return (_fsa->getCurrentState() == "Start");
}

void 
BlendHCI::brushMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
}

void 
BlendHCI::brushOn(MinVR::EventRef e)
{
  _offsets.clear();
  _pigment.clear();
  _colors.clear();
  _marks.clear();
  _brushStates.clear();
  // no support in this drawing interface for varying pressure as we draw
  _brush->state->maxPressure = 0;
  _brush->state->superSampling = 0;
}

void 
BlendHCI::brushPressureChange(MinVR::EventRef e)
{
  //_brush->state->pressure = e->get1DData();
  //cout << "new pressure " << _brush->state->pressure << endl;
}

void 
BlendHCI::brushDrawMotion(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  CoordinateFrame eVS = _gfxMgr->roomToVirtualSpace(e->getCoordinateFrameData());
  Vector3 pos = eVS.translation;

  // Pickup Points
  Array<MarkRef> marks = _brush->getArtwork()->getMarks();
  for (int i=0;i<marks.size();i++) {
    for (int j=0;j<marks[i]->getNumSamples();j++) {
      Vector3 v = marks[i]->getSamplePosition(j) - pos;
      // see if the point lies within the blend tool
      if (_box.contains(v)) {
        // should we take this point?  flip a coin..
        if (G3D::uniformRandom() <= MinVR::ConfigVal("Blend_PickupProbability", 0.05, false)) {
          _offsets.append(v);
          // conservation of pigment: how much should we take from this point and add to
          // the pigment stored on our blending tool
          double percPigmentPickedUp = clamp((double)G3D::uniformRandom(), 
            MinVR::ConfigVal("Blend_PigmentPickupMin", 0.1, false),
            MinVR::ConfigVal("Blend_PigmentPickupMax", 0.9, false));
          double pigmentPickedUp = percPigmentPickedUp;
          _pigment.append(pigmentPickedUp);
          _colors.append(marks[i]->getInitialBrushState()->colorSwatchCoord);
          // TODO: take pigment away from the mark
          //marks[i]->setSampleWeight(j, marks[i]->getSampleWeight(j) - pigmentPickedUp);
        }
      }
    }
  }

  // Deposit points
  Vector3 headPos = _eventMgr->getCurrentHeadFrame().translation;
  headPos = _gfxMgr->roomPointToVirtualSpace(headPos);

  for (int i=0;i<_offsets.size();i++) {
    if (G3D::uniformRandom() <= MinVR::ConfigVal("Blend_DepositProbability", 0.25, false)) {
      // find a mark with the same color
      MarkRef thisMark;
      BrushStateRef thisState;
      std::string key = MinVR::realToString(_colors[i]);
      if (_marks.containsKey(key)) {
        //cout << "existing mark" << _colors[i] << endl;
        thisMark = _marks[key];
        thisState = _brushStates[MinVR::realToString(_colors[i])];
      }
      else {
        thisMark = new PointSplatMark("BlendMark", _gfxMgr, _brush->getArtwork()->getPointModel());
        _brush->getArtwork()->addMark(thisMark);
        _marks.set(key, thisMark);
        thisState = _brush->state->copy();
        thisState->colorSwatchCoord = _colors[i];
        _brushStates.set(key, thisState);
        //cout << "new mark" << _colors[i] << endl;
      }

      // Add a sample to the current Mark, this is what is usually done inside BrushState
      thisState->frameInRoomSpace = e->getCoordinateFrameData();
      Vector3 posVS = eVS.translation + _offsets[i];
      thisState->frameInRoomSpace.translation = _gfxMgr->virtualPointToRoomSpace(posVS);
      thisState->size = MinVR::ConfigVal("BlendToolBrushSize", 0.1, false);
      thisState->width = thisState->size * _pigment[i];
      thisMark->addSample(thisState);

      _pigment[i] = 0.0;
    }
  }

  // Clear out old offsets that have lost all their pigment
  int i = 0;
  while (i < _offsets.size()) {
    while ((i < _offsets.size()) && (_pigment[i] <= 0.0)) {
      _offsets.fastRemove(i);
      _pigment.fastRemove(i);
    }
    i++;    
  }
}

void
BlendHCI::brushOff(MinVR::EventRef e)
{
}


} // end namespace
