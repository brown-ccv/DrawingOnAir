
#include "TracingTrial.H"
#include "ScoreTrials.H"

TracingTrial::TracingTrial(
      int                    trialNum,
      int                    totalNumTrials,
      int                    conditionID,
      std::string            subjectID,
      MarkRef                promptMark,
      std::string            promptFile,
      int                    promptIndex,
      bool                   training,
      ArtworkRef             artwork,
      CavePaintingCursorsRef cursors,
      EventMgrRef            eventMgr,
      GfxMgrRef              gfxMgr,
      HCIMgrRef              hciMgr,
      DrawingStudy*          drawingStudyApp)
{
  _totalNumTrials = totalNumTrials;
  _done = false;
  _artwork = artwork;
  _cursors = cursors;
  _eventMgr = eventMgr;
  _gfxMgr = gfxMgr;
  _hciMgr = hciMgr;
  _drawingStudyApp = drawingStudyApp;
  _promptMark = promptMark;
  _name = "TracingTrial" + intToString(trialNum);
  _runCalled = false;

  _outputFilename = "trial-data/" + subjectID + "/trial" + intToString(trialNum);
  if (subjectID != "demo") {
    if ((!ConfigVal("AllowDataOverwrite", false, false)) && (fileExists(_outputFilename + ".3DArt"))) {
      alwaysAssertM(false, "Output file already exists: " + _outputFilename);
    }
    if ((!ConfigVal("AllowDataOverwrite", false, false)) && (fileExists(_outputFilename + ".trace-data"))) {
      alwaysAssertM(false, "Output file already exists: " + _outputFilename);
    }
  }

  _drawID = -1;
  _startTrialTime = -1.0;
  _startPaintingTime = -1.0;
  _stopPaintingTime = -1.0;

  // Initialize Data
  _data.trialNum = trialNum;
  _data.conditionID = conditionID;
  _data.subjectID = subjectID;
  _data.promptFile = promptFile;
  _data.promptIndex = promptIndex;
  _data.training = training;
  _data.outputFile = _outputFilename + ".3DArt";

  _data.totalTime = -1.0;
  _data.paintingTime = -1.0;
  _data.timeout = false;
  _data.throwOutData = false;

  _fsa = new Fsa("TracingTrial");
  _fsa->addState("Start");
  _fsa->addState("Drawing");
  _fsa->addState("Pause");
  _fsa->addState("PauseBtnDown");
  _fsa->addState("Done");

  _fsa->addArc("BrushOn", "Start", "Drawing", splitStringIntoArray("Brush_Btn_down"));
  _fsa->addArcCallback("BrushOn", this, &TracingTrial::brushOn);

  _fsa->addArc("BrushOff", "Drawing", "Pause", splitStringIntoArray("Brush_Btn_up"));
  _fsa->addArcCallback("BrushOff", this, &TracingTrial::brushOff);

  _fsa->addArc("DrawingTimeoutArc", "Start", "Done", splitStringIntoArray("DrawingTimeout_" + _name));  
  _fsa->addArcCallback("DrawingTimeoutArc", this, &TracingTrial::drawingTimeout);

  _fsa->addArc("PauseDown", "Pause", "PauseBtnDown", splitStringIntoArray("Brush_Btn_down"));  
  _fsa->addArc("PauseUp", "PauseBtnDown", "Pause", splitStringIntoArray("Brush_Btn_up"));
  _fsa->addArcCallback("PauseUp", this, &TracingTrial::pauseBtnUp);

  _fsa->addArc("BadData", "Pause", "Pause", splitStringIntoArray("kbd_SPACE_down"));
  _fsa->addArcCallback("BadData", this, &TracingTrial::markDataAsBad);

  _fsa->addArc("PauseDone", "Pause", "Done", splitStringIntoArray("PauseTimeout_" + _name));
  _fsa->addArcCallback("PauseDone", this, &TracingTrial::done);



  _instFSA = new Fsa("TracingTrialInstructions");
  _instFSA->addState("Start");
  _instFSA->addState("BrushBtnDown");

  _instFSA->addArc("Timeout", "Start", "Start", splitStringIntoArray("InstructionsTimeout_" + _name));
  _instFSA->addArcCallback("Timeout", this, &TracingTrial::stopInstructions);

  _instFSA->addArc("BtnDown", "Start", "BrushBtnDown", splitStringIntoArray("Brush_Btn_down"));  
  _instFSA->addArc("BtnUp", "BrushBtnDown", "Start", splitStringIntoArray("Brush_Btn_up"));
  _instFSA->addArcCallback("BtnUp", this, &TracingTrial::instStartNewTimer);
  


  // Load up and save the prompt mark
  /**
  BinaryInput bi(promptFile, G3D_LITTLE_ENDIAN, false);
  Color3 bgColor;
  ArtworkIO::deserialize(bi, _artwork, bgColor, NULL, NULL, NULL);
  debugAssert(promptIndex < _artwork->getMarks().size());
  _promptMark = _artwork->getMarks()[promptIndex];
  _artwork->clear();
  **/

  _data.promptLength = _promptMark->getArcLength();

  _startTextPos = _promptMark->getSamplePosition(0);
  _endTextPos = _promptMark->getSamplePosition(_promptMark->getNumSamples()-1);
  Vector3 dir = (_endTextPos - _startTextPos).unit();
  _startTextPos -= 0.05*dir;
  _endTextPos += 0.05*dir;

  _promptsToDraw.append(_promptMark);

  /**
  // Create a bunch of marks to form a dotted representation of the prompt
  int i=1;
  while ((i+5) < _promptMark->getNumSamples()) {
    MarkRef dash = new TubeMark("dash", _gfxMgr, _artwork->getTriStripModel());
    for (int c=0;c<5;c++) {
      BrushStateRef bs = _promptMark->getBrushState(i)->copy();
      dash->addSample(bs);
      if ((c==0) || (c==4)) {
        dash->addSample(bs);
      }
      i++;
    }
    i += 5;
    _promptsToDraw.append(dash);
  }
  **/

  /***  
  _promptTunnel = new TubeMark("prompttunnel", _gfxMgr, _artwork->getTriStripModel());
  for (int i=0;i<_promptMark->getNumSamples();i++) {
    BrushStateRef bs = _promptMark->getBrushState(i)->copy();
    bs->twoSided = true;
    bs->textureApp = BrushState::TEXAPP_STAMP;
    bs->size = 0.05;
    bs->width = 0.05;
    bs->brushTip = 14;
    bs->colorSwatchCoord = ConfigVal("TunnelColorSwatch", 0.25, false);
    _promptTunnel->addSample(bs);
  }
  ***/

  double cSpacing = _promptMark->getArcLength() / 10.0;
  double cLast = 0.0;
  for (int i=0;i<_promptMark->getNumSamples();i++) {
    double a = _promptMark->getArcLength(0, i);
    if (a - cLast > cSpacing) {
      _cylinderIndices.append(i);
      cLast = a;
    }
  }

}


TracingTrial::TracingTrial(const std::string      &filename, 
                           ArtworkRef             artwork,
                           CavePaintingCursorsRef cursors,
                           EventMgrRef            eventMgr,
                           GfxMgrRef              gfxMgr,
                           HCIMgrRef              hciMgr,
                           DrawingStudy*          drawingStudyApp)

{
  _artwork = artwork;
  _cursors = cursors;
  _eventMgr = eventMgr;
  _gfxMgr = gfxMgr;
  _hciMgr = hciMgr;
  _drawingStudyApp = drawingStudyApp;

  // Load trial data
  BinaryInput bi(filename, G3D_LITTLE_ENDIAN, false);
  deserialize(bi);

  // fix older data used for first 2 subjects
  if (_data.metrics.size() == 9) {
    Array<double> newMetrics;
    newMetrics.append(_data.metrics[0]);
    newMetrics.append(_data.metrics[1]);
    newMetrics.append(_data.metrics[2]);

    // Scores reported to the user during training, make 0=bad 100=good
    double worstPos = ConfigVal("WorstPositionScore", 5.0, false);
    double avgPos = (_data.metrics[1] + _data.metrics[2])/2.0;
    double posScore = clamp(100.0 * (worstPos - avgPos)/worstPos, 0.0, 100.0);
    newMetrics.append(posScore);

    newMetrics.append(_data.metrics[3]);

    double worstSmooth = ConfigVal("WorstSmoothScore",  25.0, false);
    double smoothScore = clamp(100.0 * (worstSmooth - _data.metrics[3])/worstSmooth , 0.0, 100.0);
    newMetrics.append(smoothScore);

    newMetrics.append(_data.metrics[4]);
    newMetrics.append(_data.metrics[5]);
    newMetrics.append(_data.metrics[6]);
    newMetrics.append(_data.metrics[7]);
    newMetrics.append(_data.metrics[8]);    

    _data.metrics = newMetrics;
  }
}


void
TracingTrial::rescoreData(Array<MarkRef> prompts)
{
  // Load up the drawn and prompt marks and recompute metrics
  
  // Load up the drawn mark
  BinaryInput bi2(_data.outputFile, G3D_LITTLE_ENDIAN, false);
  Color3 bgColor;
  ArtworkIO::deserialize(bi2, _artwork, bgColor, NULL, NULL, NULL);
  Array<MarkRef> marks = _artwork->getMarks();
  if (marks.size() == 2) {
    _drawnMark = marks[1];
  }
  else if (marks.size() > 2) {
    // sometimes more than 2 marks got saved, when the user drew more
    // than one by accident, if so assume the one with the most
    // samples is the real drawn mark.
    int longest = 1;
    for (int i=2;i<marks.size();i++) {
      if (marks[i]->getNumSamples() > marks[longest]->getNumSamples()) {
        longest = i;
      }
    }
    _drawnMark = marks[longest];
  }
  _artwork->clear();
  
  /***
   // Load up the prompt mark
   BinaryInput bi3(_data.promptFile, G3D_LITTLE_ENDIAN, false);
   ArtworkIO::deserialize(bi3, _artwork, bgColor, NULL, NULL, NULL);
   _promptMark = _artwork->getMarks()[_data.promptIndex];
   _artwork->clear();
  ***/
  _promptMark = prompts[_data.promptIndex];
  
  if (_drawnMark.notNull()) {
    computeMetrics();
  }
}


TracingTrial::~TracingTrial()
{
  _fsa = NULL;
  _promptMark = NULL;
}

bool
TracingTrial::getCompleted()
{
  return _done;
}

void
TracingTrial::showInstructionsThenRun()
{
  _eventMgr->addFsaRef(_instFSA);  
  double time = ConfigVal("InstructionsTime", 10, false);
  _eventMgr->queueTimerEvent(new Event("InstructionsTimeout_" + _name), time);

  _instDrawID = _gfxMgr->addDrawCallback(this, &TracingTrial::drawInstructions);
}

void
TracingTrial::stopInstructions(VRG3D::EventRef e)
{
  _eventMgr->removeFsaRef(_instFSA);
  _gfxMgr->removeDrawCallback(_instDrawID);
  run();
}

void
TracingTrial::instStartNewTimer(VRG3D::EventRef e)
{
  double time = ConfigVal("InstructionsTime", 10, false);
  _eventMgr->queueTimerEvent(new Event("InstructionsTimeout_" + _name), time);
}

void
TracingTrial::drawInstructions(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  std::string s1,s2;
  s1 = "Next, you'll draw using";
  if (_data.conditionID == 0) {
    s2 = "Directional Drag Drawing";
  }
  else if (_data.conditionID == 1) {
    s2 = "Two-Handed Drawing";
  }
  else if (_data.conditionID == 2) {
    s2 = "Freehand in Sand";
  }
  else if (_data.conditionID == 3) {
    s2 = "Freehand";
  }

  double textSize = 0.05;
  Vector3 textPos = _gfxMgr->screenPointToRoomSpaceFilmplane(Vector2(0, 0.5));
  _gfxMgr->getDefaultFont()->draw3D(rd, s1, textPos, textSize, Color3::yellow(), Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
  textPos = _gfxMgr->screenPointToRoomSpaceFilmplane(Vector2(0, 0.25));
  _gfxMgr->getDefaultFont()->draw3D(rd, s2, textPos, textSize, Color3::red(), Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);

  if (_instFSA->getCurrentState() == "BrushBtnDown") {
    std::string s = "Release button to continue.";
    textPos = _gfxMgr->screenPointToRoomSpaceFilmplane(Vector2(0, -0.1));
    _gfxMgr->getDefaultFont()->draw3D(rd, s, textPos, textSize, Color3::red(), Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
  }
}

void
TracingTrial::run()
{
  _runCalled = true;
  _drawnMark = NULL;
  _startTrialTime = -1.0;
  _startPaintingTime = -1.0;
  _stopPaintingTime = -1.0;
  _data.totalTime = -1.0;
  _data.paintingTime = -1.0;
  _data.timeout = false;
  _data.throwOutData = false;
    
  _startTrialTime = SynchedSystem::getLocalTime();
  _eventMgr->addFsaRef(_fsa);

  double time = ConfigVal("DrawingTime", 10, false);
  _eventMgr->queueTimerEvent(new Event("DrawingTimeout_" + _name), time);


  _artwork->clear();
  if (ConfigVal("TracingTrial_DottedPrompt",1,false) == 1) {
    for (int i=0;i<_promptsToDraw.size();i++) {
      _artwork->addMark(_promptsToDraw[i]);
    }
  }
  else {
    _artwork->addMark(_promptMark);
  }

  _initialNMarks = _artwork->getMarks().size();


  _gfxMgr->setRoomToVirtualSpaceFrame(_promptMark->getRoomToVirtualFrameAtStart());
  _gfxMgr->setRoomToVirtualSpaceScale(_promptMark->getRoomToVirtualScaleAtStart());
  _drawID = _gfxMgr->addDrawCallback(this, &TracingTrial::draw);
}

void
TracingTrial::brushOn(VRG3D::EventRef e)
{
  _startPaintingTime = SynchedSystem::getLocalTime();
}

void
TracingTrial::brushOff(VRG3D::EventRef e)
{
  if ((_drawnMark.isNull()) || (_drawnMark->getArcLength() < 0.5*_promptMark->getArcLength())) {
    // probably got an accidental push and quick release of the brush
    // btn, restart the trial
    _fsa->jumpToState("Start");
    _eventMgr->removeFsaRef(_fsa);
    if (_drawID != -1) {
      _gfxMgr->removeDrawCallback(_drawID);
      _drawID = -1;
    }
    _artwork->clear();

    run();
  }
  else {
    // normal situation..
    stopTiming();
    computeMetrics();    
    double ptime = ConfigVal("TracingPauseTime", 1.0, false);
    if (_data.training) {
      ptime = ConfigVal("TracingTrainingPauseTime", 5.0, false);
    }
    _eventMgr->queueTimerEvent(new Event("PauseTimeout_" + _name), ptime);
  }
}

void
TracingTrial::drawingTimeout(VRG3D::EventRef e)
{
  _data.timeout = true;
  stopTiming();
  computeMetrics();
  done(e);
}

void
TracingTrial::stopTiming()
{
  _stopPaintingTime = SynchedSystem::getLocalTime();
  _data.totalTime = _stopPaintingTime - _startTrialTime;
  if (_data.timeout) {
    _data.paintingTime = 0.0;
  }
  else {
    _data.paintingTime = _stopPaintingTime - _startPaintingTime;
  }
}

void
TracingTrial::pauseBtnUp(VRG3D::EventRef e)
{
  // We were paused, then the user pressed the brush btn, now, it has
  // been released, restart the pause b/c we may have missed the
  // timeout while the btn was held down.
  
  double ptime = ConfigVal("TracingPauseTime", 1.0, false);
  if (_data.training) {
    ptime = ConfigVal("TracingTrainingPauseTime", 5.0, false);
  }
  _eventMgr->queueTimerEvent(new Event("PauseTimeout_" + _name), ptime);  
}


void
TracingTrial::markDataAsBad(VRG3D::EventRef e)
{
  _data.throwOutData = true;
}


void
TracingTrial::done(VRG3D::EventRef e)
{
  _eventMgr->removeFsaRef(_fsa);
  if (_drawID != -1) {
    _gfxMgr->removeDrawCallback(_drawID);
  }
  writeData();
  _artwork->clear();

  _done = true;
}


void
TracingTrial::writeData()
{
  _drawingStudyApp->saveArtwork(_outputFilename + ".3DArt");  

  // Save trial
  BinaryOutput bo(_outputFilename + ".trial-data", G3D_LITTLE_ENDIAN);
  serialize(bo);
  bo.commit();
}



void
TracingTrial::updateEachFrame()
{
  if (_runCalled) {
    if (_drawnMark.isNull()) {
      int n = _artwork->getMarks().size();
      if (n > _initialNMarks) { // first n marks are the prompt and tunnel,etc..
        _drawnMark = _artwork->getMarks()[n-1];
      }
    }
  }    
}


void 
TracingTrial::drawCylinderOnMark(RenderDevice *rd, MarkRef m, int sample)
{
  double w = ConfigVal("TracingTrial_CylinderW", 0.001, false);
  double r = ConfigVal("TracingTrial_CylinderR", 0.025, false);

  Vector3 start = m->getSamplePosition(sample) - w*m->getBrushState(sample)->drawingDir;
  Vector3 end = m->getSamplePosition(sample) + w*m->getBrushState(sample)->drawingDir;

  _cursors->drawLineAsCylinder(start, end, r, r, rd, Color3::white(), 60, _gfxMgr->getTexture("Bullseye"), 1.0);
}


void drawMeter(RenderDevice *rd, Vector3 center, double width, double height, double value)
{
  Vector3 side = Vector3(width, 0, 0);
  Vector3 up = Vector3(0, height, 0);
  Vector3 a = center  - side + up;
  Vector3 b = center  + side + up;
  Vector3 c = center  + side - up;
  Vector3 d = center  - side - up;
  
  Vector3 offset = Vector3(0,0,0.001);
  side = 0.95*side;
  up = 0.95*up;
  Vector3 a3 = center + offset - side - up + 2.0*value*up;
  Vector3 b3 = center + offset + side - up + 2.0*value*up;
  Vector3 c3 = center + offset + side - up;
  Vector3 d3 = center + offset - side - up;

  rd->pushState();
  rd->disableLighting();
  rd->setCullFace(RenderDevice::CULL_NONE);

  rd->setColor(Color3::white());
  rd->beginPrimitive(RenderDevice::QUADS);
  rd->sendVertex(a);
  rd->sendVertex(b);
  rd->sendVertex(c);
  rd->sendVertex(d);
  rd->endPrimitive();

  rd->setColor(Color3::green());
  rd->beginPrimitive(RenderDevice::QUADS);
  rd->sendVertex(a3);
  rd->sendVertex(b3);
  rd->sendVertex(c3);
  rd->sendVertex(d3);
  rd->endPrimitive();

  rd->popState();
}


void
TracingTrial::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  rd->pushState();

  std::string s;
  Color3 textCol = Color3(0.61,0.72,0.92);
  if (_fsa->getCurrentState() == "Pause") {
    if (_data.throwOutData) {
      s = "Pause: Marked this data as bad.";
    }
    else {
      s = "Pause";
    }
  }
  else if (_fsa->getCurrentState() == "PauseBtnDown") {
    s = "Release button to go to next line.";
    textCol = Color3::red();
  }
  else {
    s = "Please trace the curve.";
  }

  Vector3 textPos = _gfxMgr->screenPointToRoomSpaceFilmplane(Vector2(0, 0.85));
  double textSize = 0.05;
  _gfxMgr->getDefaultFont()->draw3D(rd, s, textPos, textSize, textCol, Color4::clear(), 
                                    GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);

  if ((_fsa->getCurrentState() == "Pause") && (_data.training) && (_data.metrics.size() >= 4)) {
    double ts = 0.025;
    double w = 0.1;
    double h = 0.25;

    Vector3 p = Vector3(-0.25, 0.1, 0.25);
    textPos = p + Vector3(0, (h+ts), 0); 
    drawMeter(rd, p, w, h, _posScore/100.0);   
     s = "Position Score = " + intToString(iRound(_posScore));
     _gfxMgr->getDefaultFont()->draw3D(rd, s, textPos, ts, Color3::green(), Color4::clear(), 
                                      GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);

    p = Vector3(0.25, 0.1, 0.25);
    textPos = p + Vector3(0, (h+ts), 0); 
    drawMeter(rd, p, w, h, _smoothScore/100.0);   
    s = "Direction Score = " + intToString(iRound(_smoothScore));
    _gfxMgr->getDefaultFont()->draw3D(rd, s, textPos, ts, Color3::green(), Color4::clear(), 
                                      GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);  
  }

  if (_fsa->getCurrentState() == "Start") {
    double total     = ConfigVal("DrawingTime", 10, false);
    double elapsed   = SynchedSystem::getLocalTime() - _startTrialTime;
    int remaining = iRound(total - elapsed);
    textPos = _gfxMgr->screenPointToRoomSpaceFilmplane(Vector2(-0.9, 0.85));
    _gfxMgr->getDefaultFont()->draw3D(rd, intToString(remaining), textPos, textSize, textCol, 
                                      Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
  }


  s = intToString(_data.trialNum) + "/" + intToString(_totalNumTrials);
  textPos = _gfxMgr->screenPointToRoomSpaceFilmplane(Vector2(0.95, -0.95));
  _gfxMgr->getDefaultFont()->draw3D(rd, s, textPos, 0.3*textSize, Color3::black(), Color4::clear(), 
                                    GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);


  // draw start and end markers
  CoordinateFrame f;
  f.translation = _gfxMgr->virtualPointToRoomSpace(_startTextPos);
  _gfxMgr->getDefaultFont()->draw3D(rd, "Start", f, 
                                    ConfigVal("TracingTrial_StartTextSize", 0.02, false), 
                                    Color3::yellow(), Color4::clear(),
                                    GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
  f.translation = _gfxMgr->virtualPointToRoomSpace(_endTextPos);
  _gfxMgr->getDefaultFont()->draw3D(rd, "End", f, 
                                    ConfigVal("TracingTrial_StartTextSize", 0.02, false), 
                                    Color3::orange(), Color4::clear(),
                                    GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);


  drawCylinderOnMark(rd, _promptMark, 0);
  drawCylinderOnMark(rd, _promptMark, _promptMark->getNumSamples()-1);

  if (_data.training) {
    for (int i=0;i<_cylinderIndices.size();i++) {
      drawCylinderOnMark(rd, _promptMark, _cylinderIndices[i]);
    }
  }
  
  rd->popState();
}

void
TracingTrial::computeMetrics()
{
  double m;
  _data.metrics.clear();

  if (_drawnMark.notNull()) {
    double arclen = ConfigVal("ResampleLength", 0.001, false);
    MarkRef resampPrompt = ScoreTrials::resampleMarkByArclength(_promptMark, arclen);
    MarkRef resampDrawn;

    /**
    if (ConfigVal("UseSplineFit", false, false)) {
      Array<Vector3> ctrlPoints;
      Array<Vector3> segPoints;
      SplineFit(_drawnMark, ctrlPoints, segPoints);
      resampDrawn = SplineToMark(_drawnMark, arclen, ctrlPoints);
      //_artwork->addMark(resampDrawn);
      // TODO: display segPoints and ctrlPoints if you want to
      // visually check the algorithm.
    }
    else {**/
      resampDrawn = ScoreTrials::resampleMarkByArclength(_drawnMark, arclen);
      //}
 
    _data.metrics.append(ScoreTrials::c0NormalizedAreaBetweenCurves(resampPrompt, resampDrawn));
    _data.metrics.append(ScoreTrials::c0AvgDistToPromptAlongDrawn(resampPrompt, resampDrawn));
    _data.metrics.append(ScoreTrials::c0AvgDistToDrawnAlongPrompt(resampPrompt, resampDrawn));

    // Scores reported to the user during training, make 0=bad 100=good
    double worstPos = ConfigVal("WorstPositionScore", 5.0, false);
    double avgPos = (_data.metrics[1] + _data.metrics[2])/2.0;
    _posScore = clamp(100.0 * (worstPos - avgPos)/worstPos, 0.0, 100.0);
    _data.metrics.append(_posScore);

    _data.metrics.append(ScoreTrials::c1AvgAngleBetweenTangents(resampPrompt, resampDrawn));

    double worstSmooth = ConfigVal("WorstSmoothScore",  25.0, false);
    _smoothScore = clamp(100.0 * (worstSmooth - _data.metrics[4])/worstSmooth , 0.0, 100.0);
    _data.metrics.append(_smoothScore);

    _data.metrics.append(ScoreTrials::c2AvgAngleBetween2ndDeriv(resampPrompt, resampDrawn));
    _data.metrics.append(ScoreTrials::startPointsError(_promptMark, _drawnMark));
    _data.metrics.append(ScoreTrials::endPointsError(_promptMark, _drawnMark));
    _data.metrics.append(ScoreTrials::avgDiffInWidth(resampPrompt, resampDrawn));
    _data.metrics.append(ScoreTrials::avgDiffInDerivOfWidth(resampPrompt, resampDrawn));
    _data.metrics.append(ScoreTrials::maxDistError(resampPrompt, resampDrawn));
  }
}

void
TracingTrial::printDataFileHeader()
{
  cout << "subid, trial, cond, prompt, plen, ttotal, tpaint, timeout, training, baddata, ";
  cout << "c0area, c0distd, c0distp, pos, c1, smooth, c2, estart, eend, diffw, diffdw" << endl;
}


void
TracingTrial::printMetrics()
{
  std::string s = _data.subjectID + ", " +
    intToString(_data.trialNum) + ", " + 
    intToString(_data.conditionID) + ", " + 
    intToString(_data.promptIndex) + ", " +
    realToString(_data.promptLength) + ", " + 
    realToString(_data.totalTime) + ", " +
    realToString(_data.paintingTime) + ", " +
    intToString((int)_data.timeout) + ", " +
    intToString((int)_data.training) + ", " +
    intToString((int)_data.throwOutData);

  for (int i=0;i<_data.metrics.size();i++) {
    s = s + ", " + realToString(_data.metrics[i]);
  }

  cout << s << endl;
}

/** Each time you change the format of this, increase the
    SERIALIZE_VERSION_NUMBER and respond accordingly in the
    deserialize method.  In this method, you can safely get rid of the
    old code because we don't want to create files in an old format.
*/
#define SERIALIZE_VERSION_NUMBER 1
void
TracingTrial::serialize(BinaryOutput &b)
{
  b.writeInt8(SERIALIZE_VERSION_NUMBER);

  b.writeInt32(_data.trialNum);
  b.writeInt32(_data.conditionID);
  b.writeString(_data.subjectID);
  b.writeString(_data.promptFile);
  b.writeInt32(_data.promptIndex);
  b.writeFloat64(_data.promptLength);
  b.writeFloat64(_data.totalTime);
  b.writeFloat64(_data.paintingTime);
  b.writeInt32((int)_data.timeout);
  b.writeInt32((int)_data.training);
  b.writeInt32((int)_data.throwOutData);
  b.writeString(_data.outputFile);
  b.writeInt32(_data.metrics.size());
  for (int i=0;i<_data.metrics.size();i++) {
    b.writeFloat64(_data.metrics[i]);
  }
}

/** Never delete the code from an old version from this method so we
    can always stay backwards compatable.  Important, since this is in
    binary, and we can't really reverse engineer the file format.
*/
void
TracingTrial::deserialize(BinaryInput &b)
{
  int version = b.readInt8();

  if (version == SERIALIZE_VERSION_NUMBER) {
    _data.trialNum = b.readInt32();
    _data.conditionID = b.readInt32();
    _data.subjectID = b.readString();
    _data.promptFile = b.readString();
    _data.promptIndex = b.readInt32();
    _data.promptLength = b.readFloat64();
    _data.totalTime = b.readFloat64();
    _data.paintingTime = b.readFloat64();
    _data.timeout = (bool)b.readInt32();
    _data.training = (bool)b.readInt32();
    _data.throwOutData = (bool)b.readInt32();
    _data.outputFile = b.readString();
    int n = b.readInt32();
    for (int i=0;i<n;i++) {
      _data.metrics.append(b.readFloat64());
    }   
  }
  else if (version == 0) {
    _data.trialNum = b.readInt32();
    _data.conditionID = b.readInt32();
    _data.subjectID = b.readString();
    _data.promptFile = b.readString();
    _data.promptIndex = b.readInt32();
    _data.promptLength = b.readFloat64();
    double tunnelWidth = b.readFloat64();
    _data.totalTime = b.readFloat64();
    _data.paintingTime = b.readFloat64();
    _data.timeout = (bool)b.readInt32();
    bool error = (bool)b.readInt32();
    int timesLeftTunnel = b.readInt32();
    _data.outputFile = b.readString();
    int n = b.readInt32();
    for (int i=0;i<n;i++) {
      _data.metrics.append(b.readFloat64());
    }   
    _data.training = false;
    _data.throwOutData = false;
  }
  else {
    alwaysAssertM(false, "TracingTrial: Unknown version in deserialize = " + intToString(version));
  }
}


