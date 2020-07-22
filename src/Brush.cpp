
#include <G3DOperators.h>
#include <ConfigVal.H>

#include "Brush.H"

#include "AnnotationMark.H"
#include "FlatTubeMark.H"
#include "PointSplatMark.H"
#include "RibbonMark.H"
#include "SlideMark.H"
#include "TubeMark.H"

#include "SplineFit.H"
using namespace G3D;
namespace DrawOnAir {

Brush::Brush(BrushStateRef brushState, ArtworkRef artwork, GfxMgrRef gfxMgr
  , EventMgrRef eventMgr, HistoryRef history)
{
  state = brushState;
  _artwork = artwork;
  _gfxMgr = gfxMgr;
  _eventMgr = eventMgr;
  _history = history;
  _newMarkType = 4;
  _slideTextureName = "";
  _fixedWidth = 0.0;
  
  _nameChangeFsa = new Fsa("Brush_MarkNameChange");
  _nameChangeFsa->addState("Start");
  _nameChangeFsa->addArc("TextEnter", "Start", "Start", MinVR::splitStringIntoArray("TextInputWidget_Input"));
  _nameChangeFsa->addArcCallback("TextEnter", this, &Brush::updateNameText);
  _nameChangeFsa->addArc("DoneTextEnter", "Start", "Start", MinVR::splitStringIntoArray("TextInputWidget_InputDone"));
  _nameChangeFsa->addArcCallback("DoneTextEnter", this, &Brush::stopNameInput); 
}
  
Brush::~Brush()
{
}

MarkRef
Brush::startNewMark()
{
  // Create a new Mark based on the current state settings
  Vector3 headPos = _eventMgr->getCurrentHeadFrame().translation;
  headPos = _gfxMgr->roomPointToVirtualSpace(headPos);

  static int aCounter = 0;
  static int rCounter = 0;
  static int tCounter = 0;
  static int fCounter = 0;

  if (_slideTextureName != "") {
    state->superSampling = 0;
    currentMark = new SlideMark(_slideTextureName, _gfxMgr);
    _slideTextureName = "";
  }
  else if (_newMarkType == 0) {
    state->twoSided = true;
    state->textureApp = BrushState::TEXAPP_STAMP;
    state->superSampling = 0;
    currentMark = new RibbonMark("RibbonMark" + MinVR::intToString(rCounter), _gfxMgr, _artwork->getTriStripModel());
    rCounter++;
  }
  else if (_newMarkType == 1) {
    state->twoSided = true;
    state->textureApp = BrushState::TEXAPP_STRETCH;
    state->superSampling = 0;
    currentMark = new RibbonMark("RibbonMark" + MinVR::intToString(rCounter), _gfxMgr, _artwork->getTriStripModel());
    rCounter++;
  }
  else if (_newMarkType == 2) {
    state->twoSided = false;
    state->textureApp = BrushState::TEXAPP_STAMP;
    state->superSampling = 0;
    currentMark = new RibbonMark("RibbonMark" + MinVR::intToString(rCounter), _gfxMgr, _artwork->getTriStripModel());
    rCounter++;
  }
  else if (_newMarkType == 3) {
    state->twoSided = false;
    state->textureApp = BrushState::TEXAPP_STRETCH;
    state->superSampling = 0;
    currentMark = new RibbonMark("RibbonMark" + MinVR::intToString(rCounter), _gfxMgr, _artwork->getTriStripModel());
    rCounter++;
  }
  else if (_newMarkType == 4) {
    state->textureApp = BrushState::TEXAPP_STAMP;
    state->superSampling = 0;
    currentMark = new TubeMark("TubeMark" + MinVR::intToString(tCounter), _gfxMgr, _artwork->getTriStripModel());
    tCounter++;
  }
  else if (_newMarkType == 5) {
    state->textureApp = BrushState::TEXAPP_STRETCH;
    state->superSampling = 0;
    currentMark = new TubeMark("TubeMark" + MinVR::intToString(tCounter), _gfxMgr, _artwork->getTriStripModel());
    tCounter++;
  }
  else if (_newMarkType == 6) {
    state->textureApp = BrushState::TEXAPP_STAMP;
    state->superSampling = 0;
    currentMark = new FlatTubeMark("FlatTubeMark" + MinVR::intToString(fCounter), _gfxMgr, _artwork->getTriStripModel());
    fCounter++;
  }
  else if (_newMarkType == 7) {
    state->textureApp = BrushState::TEXAPP_STRETCH;
    state->superSampling = 0;
    currentMark = new FlatTubeMark("FlatTubeMark" + MinVR::intToString(fCounter), _gfxMgr, _artwork->getTriStripModel());
    fCounter++;
  }
  else if (_newMarkType == 8) {
    state->superSampling = 0;
    currentMark = new AnnotationMark("Annotation" + MinVR::intToString(aCounter), _gfxMgr, _artwork->getAnnotationModel());
    aCounter++;
  }
  else {
    cerr << "Unknown mark type!" << endl;
    debugAssert(false);
  }

  /***
  else {
    currentMark = new PointSplatMark("PointSplatMark", state, 
        ConfigVal("PointSplatMark_SuperSamples", 5, false),
        headPos, 
        _gfxMgr->getRoomToVirtualSpaceFrame(), 
        _gfxMgr->getRoomToVirtualSpaceScale(), 
        _artwork->getPointModel());
    state->superSampling = ConfigVal("PointsPerBrushWidth", 26, false);
  }
  ***/

  _artwork->addMark(currentMark);
  return currentMark;
}


void
Brush::addSampleToMark()
{
  if (_fixedWidth != 0.0) {
    state->width = _fixedWidth;
    state->size = _fixedWidth;
  }

  // Keefe Jan 2012: New trackers are sampling too fast and creating too much geometry
  if (currentMark->getNumSamples()) {
	  Vector3 lastPos = currentMark->getSamplePosition(currentMark->getNumSamples()-1);
	  Vector3 samplePosition = _gfxMgr->roomPointToVirtualSpace(state->frameInRoomSpace.translation);
	  if ((samplePosition - lastPos).length() > MinVR::ConfigVal("MinBrushMovementForDrawing", 0.0, false)) {
		  currentMark->addSample(state);
	  }
  }
  else {
	currentMark->addSample(state);
  }
}

void
Brush::endMark()
{
  currentMark->commitGeometry(_artwork);
  _history->storeMarkCreated(currentMark, _artwork);
  if (state->layerIndex > _artwork->getMaxLayerID()) {
    _artwork->setMaxLayerID(state->layerIndex);
  }

#ifdef USE_SPLINES
  if (MinVR::ConfigVal("TestSplineFit", false, false)) {
    Array<Vector3> ctrlPoints;
    Array<Vector3> segPoints;
    double samplen = MinVR::ConfigVal("SplineResampleLength", 0.001, false);
    MarkRef splineMark = SplineFit(currentMark, samplen, ctrlPoints, segPoints, 
                                   _artwork->getTriStripModel(), _gfxMgr);
    _artwork->addMark(splineMark);
    splineMark->commitGeometry(_artwork);
    _history->storeMarkCreated(splineMark, _artwork);
  }
#endif

  // TODO: make this is dynamic cast?
  // If we just made an annotation, then now enter the text
  if (_newMarkType == 8) {
    startNameInput(currentMark);
  }
  currentMark = NULL;
}

void
Brush::makeNextMarkASlide(const std::string &textureName)
{
  _slideTextureName = textureName;
}



void
Brush::startNameInput(MarkRef nameMark)
{
  _nameMark = nameMark;
  _nameMark->setName("");
  _eventMgr->addFsaRef(_nameChangeFsa);
  _textInputWidget->activateViaHCIMgr();
}

void
Brush::updateNameText(MinVR::EventRef e)
{
  _nameMark->setName(e->getMsgData());
}

void
Brush::stopNameInput(MinVR::EventRef e)
{
  _eventMgr->removeFsaRef(_nameChangeFsa);
}
  
} // end namespace
