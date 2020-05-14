 
#include "SteeringStudy.H"
#include "ScoreTrials.H"

Array<int>
randomOrderedList(int min, int max)
{
  Array<int> list;
  int n = max - min + 1;
  while (list.size() < n) {
    int i = iRandom(min, max);
    if (!list.contains(i)) {
      list.append(i);
    }
  }
  return list;
}


SteeringStudy::SteeringStudy(int argc, char **argv) :
  DrawingStudy(argc, argv)
{

  // This could be in a separate program.. for now just do it here..
  // This generates a file of precomputed prompt marks
  if (ConfigVal("GenMarks",0,false)) {
    generateMarks();
  }

  if (ConfigVal("GenMarks2",0,false)) {
    generateMarksForDynamicDrawingStudy();
  }


  if (ConfigVal("PrintMeanSteeringData",0,false)) {
    // code for this routine is in SteeringOutput2.cpp
    printDatasetWithMeansForSteeringAnalysis();
  }

  if (ConfigVal("PrintSteeringData",0,false)) {
    // code for this routine is in SteeringOutput.cpp
    printDatasetForSteeringAnalysis(ConfigVal("PrintSteeringData", 0, false));
  }

  if (ConfigVal("PrintData",0,false)) {
    printData();
  }

  if (ConfigVal("PrintPromptStats",0,false)) {
    printPromptStats();
  }

  if (ConfigVal("PrintPiecewiseData",0,false)) {
    printPiecewiseData();
  }

  if (ConfigVal("PrintRandomDataset",0,false)) {
    printRandomDataset();
  }

  if (ConfigVal("PrintEqualSegments",0,false)) {
    printEqualSegments();
  }

  std::string promptMarksFile = "computed-marks.3DArt";
  if (ConfigVal("Pressure", 0, false)) {
    promptMarksFile = "computed-marks-pressure.3DArt";
  }
  else {
    _brush->setFixedWidth(ConfigVal("Brush_FixedWidth", 0.0, false));
  }


  std::string subjectID = ConfigVal("SubjectID", "demo", false);
  int iSubjectID = 0;
  if (isDigit(subjectID[0])) {
    iSubjectID = stringToInt(subjectID);
  }
  int iSubjectIDmod4 = iSubjectID % 4;
  int iSubjectIDmod5 = iSubjectID % 5;
  //cout << "Subject ID as int = " << iSubjectID << " " << iSubjectIDmod4 << " " << iSubjectIDmod5 << endl;


  srand(iSubjectID);


  // Load up and save the prompts
  BinaryInput bi(promptMarksFile, G3D_LITTLE_ENDIAN, false);
  Color3 bgColor;
  ArtworkIO::deserialize(bi, _artwork, bgColor, NULL, NULL, NULL);
  Array<MarkRef> promptMarks = _artwork->getMarks();
  _artwork->clear();

  /***
      still 4 conditions: drag, tape, good freehand, baseline freehand
      have time for roughly 100 trials

      5 prompts x 4 conditions = 20 trials, repeat each 5 times = 100 trials
      first set is training

  ***/


  Array< Array<int> > latinSquare4x4;
  int size = 4;
  for (int i=0;i<size;i++) {
    Array<int> row;
    for (int j=0;j<size;j++) {
      row.append((j+i) % size);
    }
    latinSquare4x4.append(row);
  }

  Array< Array<int> > latinSquare5x5;  
  size = 5;
  for (int i=0;i<size;i++) {
    Array<int> row;
    for (int j=0;j<size;j++) {
      row.append((j+i) % size);
    }
    latinSquare5x5.append(row);
  }

  int i=0;
  if (ConfigVal("CheckPrompts", 0, false) == 1) {

    // Does 1 set of all prompt curves with drag drawing in training mode
    for (int curve=0;curve<5;curve++) {
      int promptIndex = curve;
      bool training = false;
      _trials.append(new TracingTrial(i, 5, 
                                      0, subjectID, 
                                      promptMarks[promptIndex], promptMarksFile, 
                                      promptIndex, training,
                                      _artwork, _cavePaintingCursors, 
                                      _eventMgr, _gfxMgr, _hciMgr, this));
      i++;
    }

  }
  else if (ConfigVal("SpeedTest", 0, false) == 1) {

    int promptIndex = ConfigVal("SpeedTest_Prompt", 3, false);
    bool training = false;
    int num = 2;

    // draw 3 times slowly
    for (int n=0;n<num;n++) {
      int condition = 2;
      _trials.append(new TracingTrial(i, 5, 
                                      condition, subjectID, 
                                      promptMarks[promptIndex], promptMarksFile, 
                                      promptIndex, training,
                                      _artwork, _cavePaintingCursors, 
                                      _eventMgr, _gfxMgr, _hciMgr, this));
      i++;
    }
    // draw 3 times fast
    for (int n=0;n<num;n++) {
      int condition = 2;
      _trials.append(new TracingTrial(i, 5, 
                                      condition, subjectID, 
                                      promptMarks[promptIndex], promptMarksFile, 
                                      promptIndex, training,
                                      _artwork, _cavePaintingCursors, 
                                      _eventMgr, _gfxMgr, _hciMgr, this));
      i++;
    }
    // draw 3 times with drag
    for (int n=0;n<num;n++) {
      int condition = 0;
      _trials.append(new TracingTrial(i, 5, 
                                      condition, subjectID, 
                                      promptMarks[promptIndex], promptMarksFile, 
                                      promptIndex, training,
                                      _artwork, _cavePaintingCursors, 
                                      _eventMgr, _gfxMgr, _hciMgr, this));
      i++;
    }
    // draw 3 times with tape
    for (int n=0;n<num;n++) {
      int condition = 1;
      _trials.append(new TracingTrial(i, 5, 
                                      condition, subjectID, 
                                      promptMarks[promptIndex], promptMarksFile, 
                                      promptIndex, training,
                                      _artwork, _cavePaintingCursors, 
                                      _eventMgr, _gfxMgr, _hciMgr, this));
      i++;
    }

  }
  else {

    /** The real study setup: 1st present a mini version (20 trials)
        of the study for training, this goes through each condition
        and each prompt.  Then move into the main study where you do
        20 trials of each of the 4 conditionss, so that cycles through
        the list of 5 prompts 4 times for each block of trials.

        Ordering of input condition is randomized by subject as
        determined by 4x4 latin square.

        Ordering of prompts is the same for everyone for training, and
        for the study a list of 20 random prompts is calculated by
        sampling a 5x5 latin square 4 times.  This ordering is used for
        each condition.
    **/

    // Ordering of conditions and prompts determined by latin squares
    // to control for ordering effects
    Array<int> condSequence   = latinSquare4x4[iSubjectIDmod4];

    // Pick 4 numbers randomly between 0 and 4 and use those as the
    // indices into a 5x5 latin square, so we're picking 4 of the 5
    // rows of the square at random and appending them together to
    // come up with our "random" list of 20.
    Array<int> random5 = randomOrderedList(0,4);
    debugAssert(random5.size() == 5);
    Array<int> promptSequence;
    for (int r=0;r<4;r++) {
      promptSequence.append(latinSquare5x5[random5[r]]);
    }
    debugAssert(promptSequence.size() == 20);

    //for (int s=0;s<promptSequence.size();s++) {
    //  cout << "seq = " << promptSequence[s] << endl;
    //}


    // Training Session
    for (int c=0;c<condSequence.size();c++) {
      int condition = condSequence[c];
      for (int p=0;p<5;p++) {
        int prompt = p;
        _trials.append(new TracingTrial(i, 100,
                                        condition, subjectID, 
                                        promptMarks[prompt],  promptMarksFile, 
                                        prompt, true,
                                        _artwork, _cavePaintingCursors, 
                                        _eventMgr, _gfxMgr, _hciMgr, this));
        i++;
      }
    }

    // If Demo is set, then just run the training session
    if (ConfigVal("Demo", 0, false) != 1) {

      // Real Session
      for (int c=0;c<condSequence.size();c++) {
        int condition = condSequence[c];
        for (int p=0;p<promptSequence.size();p++) {
          int prompt = promptSequence[p];
          _trials.append(new TracingTrial(i, 100,
                                          condition, subjectID, 
                                          promptMarks[prompt],  promptMarksFile, 
                                          prompt, false, 
                                          _artwork, _cavePaintingCursors, 
                                          _eventMgr, _gfxMgr, _hciMgr, this));
          i++;
        }
      }
    }
  }

  _activeTrial = -1;

  // initial brush setup
  _brush->setNewMarkType(4);
  _brush->state->colorSwatchCoord = ConfigVal("BrushColorSwatch",0.5,false);
  _brush->state->pattern = ConfigVal("BrushPattern", 13, false);
  //_brush->state->width = 0.01;
  //_brush->state->size = 0.01;


  int skip = ConfigVal("SkipToTrial",0,false);
  if (skip != 0) {
    Array<TracingTrialRef> tmp;
    for (int i=skip;i<_trials.size();i++) {
      tmp.append(_trials[i]);
    }
    _trials = tmp;
  }
}

SteeringStudy::~SteeringStudy()
{
}


void
SteeringStudy::setInputCondition(int cond)
{
  if (!_forceNetInterface) {
    return;
  }

  if (cond == 0) {
    //cout << "Activating Drag Draw Condition" << endl;
    _hciMgr->activateStylusHCI(_forceHybridDrawHCI);
    _forceNetInterface->setViscousGain(0.8);
    _forceNetInterface->turnFrictionOn();
  }
  else if (cond == 1) {
    //cout << "Activating Tape Draw Condition" << endl;
    _hciMgr->activateStylusHCI(_forceTapeDrawHCI);
    _forceNetInterface->setViscousGain(0.8);
    _forceNetInterface->turnFrictionOn();
  }
  else if (cond == 2) {
    //cout << "Activating Direct Draw Condition" << endl;
    _hciMgr->activateStylusHCI(_directDrawHCI);
    _forceNetInterface->setViscousGain(0.8);
    _forceNetInterface->turnFrictionOn();
    // This doesn't seem to really help, so let's just test without it
    //_forceNetInterface->startAnisotropicFilter();
  }
  else if (cond == 3) {
    //cout << "Activating Direct Draw, No Friction Condition" << endl;
    _hciMgr->activateStylusHCI(_directDrawHCI);
    _forceNetInterface->stopForces();
    _forceNetInterface->setViscousGain(0.0);
    _forceNetInterface->turnFrictionOff();
  }
  else {
    alwaysAssertM(false, "Unknown condition: " + intToString(cond));
  }
}


// Override this as a way of getting a callback each frame to monitor
// the status of the trials and advance them as necessary.
void
SteeringStudy::doUserInput(Array<VRG3D::EventRef> &events)
{
  // manage the study here..
  if (_activeTrial == -1) {
    // startup first trial
    _activeTrial = 0;
    setInputCondition(_trials[0]->getData().conditionID);
    _trials[0]->showInstructionsThenRun();    
  }
  if (_trials[_activeTrial]->getCompleted()) {
    _trials[_activeTrial]->printMetrics();
    _activeTrial++;
    if (_activeTrial >= _trials.size()) {
      cout << "Metrics for all trials: " << endl;
      _trials[0]->printDataFileHeader();
      for (int i=0;i<_trials.size();i++) {
        _trials[i]->printMetrics();
      }
      exit(0);
    }
    else {
      int prevcond = _trials[_activeTrial-1]->getData().conditionID;
      int cond = _trials[_activeTrial]->getData().conditionID;
      if (prevcond != cond) {
        setInputCondition(cond);
        _trials[_activeTrial]->showInstructionsThenRun();
      }
      else {
        _trials[_activeTrial]->run();
      }
    }
  }

  // trap keys to switch drawing mode
  for (int i=0;i<events.size();i++) {
    if (events[i]->getName() == "kbd_1_down") {
      setInputCondition(0);
    }
    else if (events[i]->getName() == "kbd_2_down") {
      setInputCondition(1);
    }
    else if (events[i]->getName() == "kbd_3_down") {
      setInputCondition(2);
    }
    else if (events[i]->getName() == "kbd_4_down") {
      setInputCondition(3);
    }
    else if (events[i]->getName() == "kbd_S_down") {
      saveSnapshot("snap");
    }
  }


  // call the normal function
  DrawingStudy::doUserInput(events);

  _trials[_activeTrial]->updateEachFrame();
}

void
SteeringStudy::doGraphics(RenderDevice *rd)
{
  DrawingStudy::doGraphics(rd);
 
  // draw a plane for shadows
  rd->pushState();
  rd->setColor(Color3(0.8, 0.8, 0.8));
  rd->setTexture(0, _gfxMgr->getTexture("Floor"));
  rd->setCullFace(RenderDevice::CULL_NONE);

  double y = ConfigVal("ShadowPlaneY", -0.15, false) - 0.001;
  double w = ConfigVal("FloorW", 0.8, false);
  double h = ConfigVal("FloorD", 0.6, false);
  rd->beginPrimitive(RenderDevice::QUADS);
  rd->setNormal(Vector3(0,1,0));
  rd->setTexCoord(0, Vector2(0,1));
  rd->sendVertex(Vector3(-w/2.0, y, h/2.0));
  rd->setTexCoord(0, Vector2(0,0));
  rd->sendVertex(Vector3(-w/2.0, y, -h/2.0));
  rd->setTexCoord(0, Vector2(1,0));
  rd->sendVertex(Vector3(w/2.0, y, -h/2.0));
  rd->setTexCoord(0, Vector2(1,1));
  rd->sendVertex(Vector3(w/2.0, y, h/2.0));
  rd->endPrimitive();

  rd->popState();
}


void
SteeringStudy::generateMarks()
{
  _brush->setNewMarkType(4);
  _brush->state->brushInterface = "Computed";    
  _brush->state->colorSwatchCoord = ConfigVal("PromptColorSwatch", 0.4, false);
  _brush->state->pattern = ConfigVal("PromptPattern", 13, false);
  _brush->state->brushTip = ConfigVal("PromptBrushTip", 14, false);
  bool changeWidth = false;
  double minWidth, maxWidth;
  if (ConfigVal("Pressure", 0, false)) {
    changeWidth = true;
    minWidth = ConfigVal("PromptMinWidth", 0.003, false);
    maxWidth = ConfigVal("PromptMaxWidth", 0.01, false);
  }
  else {
    minWidth = ConfigVal("PromptSize", 0.005, false);
    maxWidth = minWidth;
  }
  _brush->state->width = minWidth;
  _brush->state->size = maxWidth;
  int nsamples = 207;
  int nsamplesdiv2 = iRound(0.5*(double)nsamples);
  Vector3 start;
  double length;

  Vector3 endCurve;
  Vector3 dir;


  // arc with a little in/out movement decreasing over time
  _brush->startNewMark();
  for (int i=0;i<nsamples;i++) {
    double t = (double)i/(double)nsamples;
    double x = 0.3 - 0.6*t;
    double y = -0.075 - x*x;
    double z = 0.175*t*sin(0.5*twoPi()*(1.0-t)) + 0.1;
    Vector3 p = Vector3(x, y, z);
    _brush->state->drawingDir = (p - _brush->state->frameInRoomSpace.translation).unit();
    _brush->state->frameInRoomSpace.translation = p;
    if (changeWidth) {
      if (i < nsamplesdiv2) {
        _brush->state->width = lerp(minWidth, maxWidth, (double)i/(double)nsamplesdiv2);
      }
      else {
        _brush->state->width = lerp(maxWidth, minWidth, (double)(i-nsamplesdiv2)/(double)nsamplesdiv2);
      }
    }
    if (i > 0) {
      _brush->addSampleToMark();
      if ((i==1) || (i==nsamples-1)) {
        _brush->addSampleToMark();
      }
    }
  }
  _brush->endMark();



  // portion of a circle, then mostly flat
  _brush->startNewMark();
  for (int i=0;i<nsamples;i++) {
    double x,y,z;
    int c1 = iRound(0.5*(double)nsamples);
    if (i <= c1) {
      double t = (double)i/(double)c1;
      double a = twoPi()*(-0.1 + 0.4*t);
      x = 0.2 + 0.15*cos(a);
      y = -0.15 + 0.15*sin(a);
      z = 0.1 + 0.1*(double)i/(double)nsamples;
      endCurve = Vector3(x,y,z);
      dir = (endCurve - _brush->state->frameInRoomSpace.translation).unit();
    }
    else {
      double t = (double)(i-c1)/(double)(nsamples-c1);
      x = endCurve[0] + 0.4*dir[0]*t;
      y =  endCurve[1] + 0.4*dir[1]*t - 0.05*sin(t*0.25*twoPi());
      z = 0.1 + 0.1*(double)i/(double)nsamples;
    }
    Vector3 p = Vector3(x, y, z);
    _brush->state->drawingDir = (p - _brush->state->frameInRoomSpace.translation).unit();
    _brush->state->frameInRoomSpace.translation = p;
    if (changeWidth) {
      if (i < nsamplesdiv2) {
        _brush->state->width = lerp(minWidth, maxWidth, (double)i/(double)nsamplesdiv2);
      }
      else {
        _brush->state->width = lerp(maxWidth, minWidth, (double)(i-nsamplesdiv2)/(double)nsamplesdiv2);
      }
    }
    if (i > 0) {
      _brush->addSampleToMark();
      if ((i==1) || (i==nsamples-1)) {
        _brush->addSampleToMark();
      }
    }
  }
  _brush->endMark();



  // inspired by the side edge of the scapula
  Array<Vector3> pts;
  for (int i=0;i<4;i++) 
    pts.append(Vector3(0,0,0));
  pts.append(Vector3(-3,1,0.5));
  pts.append(Vector3(-5,4,1));
  pts.append(Vector3(-7,8,1.2));
  pts.append(Vector3(-7,11,1.5));
  pts.append(Vector3(-6.5,12,1.5));
  pts.append(Vector3(-7,13,1.2));
  pts.append(Vector3(-8,16,0.7));
  pts.append(Vector3(-8,20,0.3));
  pts.append(Vector3(-10,24,0));
  for (int i=0;i<4;i++) 
    pts.append(Vector3(-12,28,0));
  double s = 0.25/14.0;
  Vector3 o(0.1, -0.18, 0.15);
  for (int i=0;i<pts.size();i++) {
    pts[i] = s*pts[i] + o;
  }

  _brush->startNewMark();
  for (int i=0;i<nsamples;i++) {
    double t = 3.0 + (pts.size()-7)*(double)i/(double)(nsamples-1);
    Vector3 p = cyclicCatmullRomSpline(t, pts);
    _brush->state->drawingDir = (p - _brush->state->frameInRoomSpace.translation).unit();
    _brush->state->frameInRoomSpace.translation = p;
    if (changeWidth) {
      if (i < nsamplesdiv2) {
        _brush->state->width = lerp(minWidth, maxWidth, (double)i/(double)nsamplesdiv2);
      }
      else {
        _brush->state->width = lerp(maxWidth, minWidth, (double)(i-nsamplesdiv2)/(double)nsamplesdiv2);
      }
    }
    if (i > 0) {
      _brush->addSampleToMark();
      if ((i==1) || (i==nsamples-1)) {
        _brush->addSampleToMark();
      }
    }
    //cout << "t = " << t << " p = " << p << endl;
    //cout << _brush->state->drawingDir << endl;
  }
  _brush->endMark();



  // inspired by spine of the scapula
  pts.clear();
  for (int i=0;i<4;i++) 
    pts.append(Vector3(0,0,0));
  pts.append(Vector3(-2,3,1));
  pts.append(Vector3(-6,4,1.2));
  pts.append(Vector3(-10,5,1.4));
  pts.append(Vector3(-13,9,2.5));
  pts.append(Vector3(-17,11,5));
  pts.append(Vector3(-21,12,5.5));
  pts.append(Vector3(-26,11.5,6));
  for (int i=0;i<4;i++) 
    pts.append(Vector3(-28,15,7));
  s = 0.25/14.0;
  o = Vector3(0.25, -0.1, 0.15);
  for (int i=0;i<pts.size();i++) {
    pts[i] = s*pts[i] + o;
  }

  _brush->startNewMark();
  for (int i=0;i<nsamples;i++) {
    double t = 3.0 + (pts.size()-7)*(double)i/(double)(nsamples-1);
    Vector3 p = cyclicCatmullRomSpline(t, pts);
    _brush->state->drawingDir = (p - _brush->state->frameInRoomSpace.translation).unit();
    _brush->state->frameInRoomSpace.translation = p;
    if (changeWidth) {
      if (i < nsamplesdiv2) {
        _brush->state->width = lerp(minWidth, maxWidth, (double)i/(double)nsamplesdiv2);
      }
      else {
        _brush->state->width = lerp(maxWidth, minWidth, (double)(i-nsamplesdiv2)/(double)nsamplesdiv2);
      }
    }
    if (i > 0) {
      _brush->addSampleToMark();
      if ((i==1) || (i==nsamples-1)) {
        _brush->addSampleToMark();
      }
    }
  }
  _brush->endMark();



  // straight w/ bend in middle
  _brush->startNewMark();
  for (int i=0;i<nsamples;i++) {
    double x,y,z;
    int c1 = iRound(0.33*(double)nsamples);
    int c2 = iRound(0.45*(double)nsamples);
    if (i < c1) {
      double t = (double)i/(double)c1;
      x = 0.3 - 0.2*(t);
      y = 0.0;
      z = 0.15 + 0.15*(double)i/(double)nsamples;
    }
    else if (i < c2) {
      double t = (double)(i-c1)/(double)(c2-c1);
      double r = 0.3;
      double a = twoPi() * (0.75 - 0.0625*t);
      x = 0.1 + r*cos(a);
      y = r + r*sin(a);
      z = 0.15 + 0.15*(double)i/(double)nsamples;
      endCurve = Vector3(x,y,z);
      dir = (endCurve - _brush->state->frameInRoomSpace.translation).unit();
    }
    else {
      double t = (double)(i-c2)/(double)(nsamples-c2);
      x = endCurve[0] + 0.4*dir[0]*t;
      y = endCurve[1] + 0.4*dir[1]*t;
      z = 0.15 + 0.15*(double)i/(double)nsamples;
    }
    Vector3 p = Vector3(x, y, z);
    _brush->state->drawingDir = (p - _brush->state->frameInRoomSpace.translation).unit();
    _brush->state->frameInRoomSpace.translation = p;
    if (changeWidth) {
      if (i < nsamplesdiv2) {
        _brush->state->width = lerp(minWidth, maxWidth, (double)i/(double)nsamplesdiv2);
      }
      else {
        _brush->state->width = lerp(maxWidth, minWidth, (double)(i-nsamplesdiv2)/(double)nsamplesdiv2);
      }
    }
    if (i > 0) {
      _brush->addSampleToMark();
      if ((i==1) || (i==nsamples-1)) {
        _brush->addSampleToMark();
      }
    }
  }
  _brush->endMark();









  // angled line #1
  _brush->startNewMark();
  _brush->state->drawingDir = Vector3(-1,0.5,-0.25).unit();
  start = Vector3(0.15, -0.2, 0.15);
  length = 0.4;
  for (int i=0;i<nsamples;i++) {
    double t = (double)i/(double)nsamples;
    Vector3 p = start + t*length*_brush->state->drawingDir;
    _brush->state->frameInRoomSpace.translation = p;
    if (changeWidth) {
      if (i < nsamplesdiv2) {
        _brush->state->width = lerp(minWidth, maxWidth, (double)i/(double)nsamplesdiv2);
      }
      else {
        _brush->state->width = lerp(maxWidth, minWidth, (double)(i-nsamplesdiv2)/(double)nsamplesdiv2);
      }
    }
    _brush->addSampleToMark();
    if ((i==0) || (i==nsamples-1)) {
      _brush->addSampleToMark();
    }
  }
  _brush->endMark();



  // funky sin wave
  _brush->startNewMark();
  for (int i=0;i<nsamples;i++) {
    double t = (double)i/(double)nsamples;
    double x = 0.3 - 0.6*t;
    double y = 0.1*sin(twoPi()*t) - 0.15;
    double z = 0.1*sin(0.75*twoPi()*t) + 0.1;
    Vector3 p = Vector3(x, y, z);
    _brush->state->drawingDir = (p - _brush->state->frameInRoomSpace.translation).unit();
    _brush->state->frameInRoomSpace.translation = p;
    if (changeWidth) {
      if (i < nsamplesdiv2) {
        _brush->state->width = lerp(minWidth, maxWidth, (double)i/(double)nsamplesdiv2);
      }
      else {
        _brush->state->width = lerp(maxWidth, minWidth, (double)(i-nsamplesdiv2)/(double)nsamplesdiv2);
      }
    }
    if (i > 0) {
      _brush->addSampleToMark();
      if ((i==1) || (i==nsamples-1)) {
        _brush->addSampleToMark();
      }
    }
  }
  _brush->endMark();


  // right to left along X axis
  _brush->startNewMark();
  _brush->state->drawingDir = Vector3(-1,0,0).unit();
  start = Vector3(0.2, 0, 0.2);
  length = 0.4;
  for (int i=0;i<nsamples;i++) {
    double t = (double)i/(double)nsamples;
    Vector3 p = start + t*length*_brush->state->drawingDir;
    _brush->state->frameInRoomSpace.translation = p;
    if (changeWidth) {
      if (i < nsamplesdiv2) {
        _brush->state->width = lerp(minWidth, maxWidth, (double)i/(double)nsamplesdiv2);
      }
      else {
        _brush->state->width = lerp(maxWidth, minWidth, (double)(i-nsamplesdiv2)/(double)nsamplesdiv2);
      }
    }
    _brush->addSampleToMark();
    if ((i==0) || (i==nsamples-1)) {
      _brush->addSampleToMark();
    }
  }
  _brush->endMark();


  // angled line #2
  _brush->startNewMark();
  _brush->state->drawingDir = Vector3(-1,-0.3,-0.5).unit();
  start = Vector3(0.2, 0.2, 0.15);
  length = 0.3;
  for (int i=0;i<nsamples;i++) {
    double t = (double)i/(double)nsamples;
    Vector3 p = start + t*length*_brush->state->drawingDir;
    _brush->state->frameInRoomSpace.translation = p;
    if (changeWidth) {
      if (i < nsamplesdiv2) {
        _brush->state->width = lerp(minWidth, maxWidth, (double)i/(double)nsamplesdiv2);
      }
      else {
        _brush->state->width = lerp(maxWidth, minWidth, (double)(i-nsamplesdiv2)/(double)nsamplesdiv2);
      }
    }
    _brush->addSampleToMark();
    if ((i==0) || (i==nsamples-1)) {
      _brush->addSampleToMark();
    }
  }
  _brush->endMark();


  // arc with small in/out movement and  bigger Y amplitude
  _brush->startNewMark();
  for (int i=0;i<nsamples;i++) {
    double t = (double)i/(double)nsamples;
    double x = 0.3 - 0.6*t;
    double y = 0.3 - 3.33*x*x;
    double z = 0.15*t*sin(0.5*twoPi()*t) + 0.2;
    Vector3 p = Vector3(x, y, z);
    _brush->state->drawingDir = (p - _brush->state->frameInRoomSpace.translation).unit();
    _brush->state->frameInRoomSpace.translation = p;
    if (changeWidth) {
      if (i < nsamplesdiv2) {
        _brush->state->width = lerp(minWidth, maxWidth, (double)i/(double)nsamplesdiv2);
      }
      else {
        _brush->state->width = lerp(maxWidth, minWidth, (double)(i-nsamplesdiv2)/(double)nsamplesdiv2);
      }
    }
    if (i > 0) {
      _brush->addSampleToMark();
      if ((i==1) || (i==nsamples-1)) {
        _brush->addSampleToMark();
      }
    }
  }
  _brush->endMark();


  // 95% of a circle
  _brush->startNewMark();
  for (int i=0;i<nsamples;i++) {
    double t = (double)i/(double)nsamples;
    t = 0.025 + t/1.05;
    double x = 0.3*cos(twoPi()*t);
    double y = 0.3*sin(twoPi()*t);
    double z = 0.2;
    Vector3 p = Vector3(x, y, z);
    _brush->state->drawingDir = (p - _brush->state->frameInRoomSpace.translation).unit();
    _brush->state->frameInRoomSpace.translation = p;
    if (changeWidth) {
      if (i < nsamplesdiv2) {
        _brush->state->width = lerp(minWidth, maxWidth, (double)i/(double)nsamplesdiv2);
      }
      else {
        _brush->state->width = lerp(maxWidth, minWidth, (double)(i-nsamplesdiv2)/(double)nsamplesdiv2);
      }
    }
    if (i > 0) {
      _brush->addSampleToMark();
      if ((i==1) || (i==nsamples-1)) {
        _brush->addSampleToMark();
      }
    }
  }
  _brush->endMark();

 
  if (changeWidth) {
    saveArtwork("computed-marks-pressure.3DArt"); 
  }
  else {
    saveArtwork("computed-marks.3DArt"); 
  }
  exit(0);
}


void
SteeringStudy::generateMarksForDynamicDrawingStudy()
{
  _brush->setNewMarkType(4);
  _brush->state->brushInterface = "Computed";    
  _brush->state->colorSwatchCoord = ConfigVal("PromptColorSwatch", 0.4, false);
  _brush->state->pattern = ConfigVal("PromptPattern", 13, false);
  _brush->state->brushTip = ConfigVal("PromptBrushTip", 14, false);
  bool changeWidth = false;
  double minWidth, maxWidth;
  if (ConfigVal("Pressure", 0, false)) {
    changeWidth = true;
    minWidth = ConfigVal("PromptMinWidth", 0.003, false);
    maxWidth = ConfigVal("PromptMaxWidth", 0.01, false);
  }
  else {
    minWidth = ConfigVal("PromptSize", 0.005, false);
    maxWidth = minWidth;
  }
  _brush->state->width = minWidth;
  _brush->state->size = maxWidth;
  int nsamples = 207;
  int nsamplesdiv2 = iRound(0.5*(double)nsamples);
  Vector3 start;
  double length;

  Vector3 endCurve;
  Vector3 dir;


  // sin wave thingy that covers a range of frequencies & amplitudes
  _brush->startNewMark();
  for (int i=0;i<nsamples;i++) {
    double t = (double)i/(double)nsamples;
    double x = 0.3 - 0.6*t;
    double y = (0.1*t + 0.03) * sin(4.0*twoPi()*(1.0-t)*(1.0-t));
    double z = 0.1*sin(1.0*twoPi()*t) + 0.1;
    Vector3 p = Vector3(x, y, z);
    _brush->state->drawingDir = (p - _brush->state->frameInRoomSpace.translation).unit();
    _brush->state->frameInRoomSpace.translation = p;
    if (changeWidth) {
      if (i < nsamplesdiv2) {
        _brush->state->width = lerp(minWidth, maxWidth, (double)i/(double)nsamplesdiv2);
      }
      else {
        _brush->state->width = lerp(maxWidth, minWidth, (double)(i-nsamplesdiv2)/(double)nsamplesdiv2);
      }
    }
    if (i > 0) {
      _brush->addSampleToMark();
      if ((i==1) || (i==nsamples-1)) {
        _brush->addSampleToMark();
      }
    }
  }
  _brush->endMark();

  saveArtwork("DynamicDragPilot-M1.3DArt"); 
  exit(0);
}



void
SteeringStudy::printData()
{
  int outputType = ConfigVal("OutputType", 1, false);
  int startID = ConfigVal("StartID", 1, false);
  int stopID = ConfigVal("StopID", 12, false);
  int badCount = 0;
  int goodCount = 0;
  bool rescoreData = ConfigVal("Rescore", 0, false);

  Array<MarkRef> promptMarks;
  if (rescoreData) {
    // Load up and save the prompts
    std::string promptMarksFile = "computed-marks.3DArt";
    BinaryInput bi(promptMarksFile, G3D_LITTLE_ENDIAN, false);
    Color3 bgColor;
    ArtworkIO::deserialize(bi, _artwork, bgColor, NULL, NULL, NULL);
    promptMarks = _artwork->getMarks();
    _artwork->clear();
  }
  
  if (outputType == 5) {
    calcPromptStats();
  }



  // Loop through all subjects
  for (int iSubjectID=startID;iSubjectID<=stopID;iSubjectID++) {
    std::string subjectID = intToString(iSubjectID);

    std::string filebase = "trial-data/" + subjectID + "/trial";
    std::string cvalstr = "Outliers_" + subjectID;
    Array<std::string> outstr = splitStringIntoArray(ConfigVal(cvalstr, "", false));
    Array<int> outliers;
    for (int o=0;o<outstr.size();o++) {
      outliers.append(stringToInt(outstr[o]));
    }

    int c0GoodDataCount = 0;
    double c0AvgTime = 0.0;
    Array<double> c0AvgMetrics;
    int c1GoodDataCount = 0;
    double c1AvgTime = 0.0;
    Array<double> c1AvgMetrics;
    int c2GoodDataCount = 0;
    double c2AvgTime = 0.0;
    Array<double> c2AvgMetrics;
    int c3GoodDataCount = 0;
    double c3AvgTime = 0.0;
    Array<double> c3AvgMetrics;

    // [condition][prompt][metric]
    Array< Array< Array<double> > >  avgMetricsByCondAndPrompt;
    // [condition][prompt]
    Array< Array<int> >              avgMetricsByCondAndPromptCount;
    // [condition][prompt]
    Array< Array<double> >           avgMetricsByCondAndPromptPTime;

    avgMetricsByCondAndPrompt.resize(4);
    avgMetricsByCondAndPromptCount.resize(4);
    avgMetricsByCondAndPromptPTime.resize(4);
    for (int c=0;c<4;c++) {
      avgMetricsByCondAndPrompt[c].resize(5);
      avgMetricsByCondAndPromptCount[c].resize(5);
      avgMetricsByCondAndPromptPTime[c].resize(5);
    }


    int i = 0;
    while (fileExists(filebase + intToString(i) + ".trial-data")) { // loop through trials
      TracingTrialRef t = new TracingTrial(filebase + intToString(i) + ".trial-data", 
                                           _artwork, _cavePaintingCursors, 
                                           _eventMgr, _gfxMgr, _hciMgr, this);
      if (iSubjectID == 1) {
        if ((i==0) && (outputType==1)) {
          t->printDataFileHeader();
        }
      }

      // Remove bad data
      bool badTrial = false;
      if ((t->_data.timeout) || (t->_data.throwOutData) || (t->_data.paintingTime <= 0.0)) {
        badTrial = true;
      }
      for (int o=0;o<outliers.size();o++) {
        if (i == outliers[o]) {
          badTrial = true;
        }
      }

      bool skip = false;
      if ((ConfigVal("SkipTraining", 1, false)) && (t->_data.training)) {
        skip = true;
      }
      if ((ConfigVal("OnlyTraining", 0, false)) && (!t->_data.training)) {
        skip = true;
      }


      if (!skip) {
        if (badTrial) {
          badCount++;
        }
        else  { // good trial
          goodCount++;

          if (rescoreData) {
            t->rescoreData(promptMarks);
          }

          if (outputType==1) {
            t->printMetrics();
          }
          else if ((outputType==2) || (outputType==4)) {
            if (t->_data.conditionID == 0) {
              if (c0AvgMetrics.size() == 0) {
                c0AvgMetrics = t->_data.metrics;
              }
              else {
                for (int m=0;m<t->_data.metrics.size();m++) {
                  c0AvgMetrics[m] += t->_data.metrics[m];
                }
              }
              c0AvgTime += t->_data.paintingTime;
              c0GoodDataCount++;
            }
            else if (t->_data.conditionID == 1) {
              if (c1AvgMetrics.size() == 0) {
                c1AvgMetrics = t->_data.metrics;
              }
              else {
                for (int m=0;m<t->_data.metrics.size();m++) {
                  c1AvgMetrics[m] += t->_data.metrics[m];
                }
              }
              c1AvgTime += t->_data.paintingTime;
              c1GoodDataCount++;
            }
            else if (t->_data.conditionID == 2) {
              if (c2AvgMetrics.size() == 0) {
                c2AvgMetrics = t->_data.metrics;
              }
              else {
                for (int m=0;m<t->_data.metrics.size();m++) {
                  c2AvgMetrics[m] += t->_data.metrics[m];
                }
              }
              c2AvgTime += t->_data.paintingTime;
              c2GoodDataCount++;
            }
            else if (t->_data.conditionID == 3) {
              if (c3AvgMetrics.size() == 0) {
                c3AvgMetrics = t->_data.metrics;
              }
              else {
                for (int m=0;m<t->_data.metrics.size();m++) {
                  c3AvgMetrics[m] += t->_data.metrics[m];
                }
              }
              c3AvgTime += t->_data.paintingTime;
              c3GoodDataCount++;
            }
          }

          else if (outputType == 5) {
            
            if (avgMetricsByCondAndPrompt[t->_data.conditionID][t->_data.promptIndex].size() == 0) {
              avgMetricsByCondAndPrompt[t->_data.conditionID][t->_data.promptIndex] = t->_data.metrics;
              avgMetricsByCondAndPromptPTime[t->_data.conditionID][t->_data.promptIndex] = t->_data.paintingTime;
              avgMetricsByCondAndPromptCount[t->_data.conditionID][t->_data.promptIndex] = 1;
            }
            else {
              for (int m=0;m<t->_data.metrics.size();m++) {
                avgMetricsByCondAndPrompt[t->_data.conditionID][t->_data.promptIndex][m] += t->_data.metrics[m];
              }
              avgMetricsByCondAndPromptPTime[t->_data.conditionID][t->_data.promptIndex] += t->_data.paintingTime;
              avgMetricsByCondAndPromptCount[t->_data.conditionID][t->_data.promptIndex]++;
            }

          }
        }
      }
      i++;
    }

    if ((outputType == 2) || (outputType == 4)) {
      c0AvgTime = c0AvgTime / c0GoodDataCount;
      for (int m=0;m<c0AvgMetrics.size();m++) {
        c0AvgMetrics[m] = c0AvgMetrics[m] / c0GoodDataCount;
      }
      c1AvgTime = c1AvgTime / c1GoodDataCount;
      for (int m=0;m<c1AvgMetrics.size();m++) {
        c1AvgMetrics[m] = c1AvgMetrics[m] / c1GoodDataCount;
      }
      c2AvgTime = c2AvgTime / c2GoodDataCount;
      for (int m=0;m<c2AvgMetrics.size();m++) {
        c2AvgMetrics[m] = c2AvgMetrics[m] / c2GoodDataCount;
      }
      c3AvgTime = c3AvgTime / c3GoodDataCount;
      for (int m=0;m<c3AvgMetrics.size();m++) {
        c3AvgMetrics[m] = c3AvgMetrics[m] / c3GoodDataCount;
      }
    }

    else if (outputType == 5) {

      for (int c=0;c<4;c++) {
        for (int p=0;p<5;p++) {
          avgMetricsByCondAndPromptPTime[c][p] /= avgMetricsByCondAndPromptCount[c][p];

          cout << subjectID << ", " << c << ", " << p << ", " << avgMetricsByCondAndPromptPTime[c][p];
          for (int m=0;m<avgMetricsByCondAndPrompt[c][p].size();m++) {
            avgMetricsByCondAndPrompt[c][p][m] /= avgMetricsByCondAndPromptCount[c][p];
            cout << ", " << avgMetricsByCondAndPrompt[c][p][m];
          }
          cout << pArcLen[p] << ", " 
               << pXYZTotal[p][0] << ", " << pXYZTotal[p][1] << ", " << pXYZTotal[p][2] << ", " 
               << pXYZPos[p][0] << ", " << pXYZPos[p][1] << ", " << pXYZPos[p][2] << ", " 
               << pXYZNeg[p][0] << ", " << pXYZNeg[p][1] << ", " << pXYZNeg[p][2] << ", " 
               << pAD[p] << ", "
               << pADXYZ1[p][0] << ", " << pADXYZ1[p][1] << ", " << pADXYZ1[p][2] << ", "
               << pADXYZ2[p][0] << ", " << pADXYZ2[p][1] << ", " << pADXYZ2[p][2] << endl;
        }
      }
    }

  
    if (outputType == 2) {
      Array< Array<int> > latinSquare4x4;
      int size = 4;
      for (int i=0;i<size;i++) {
        Array<int> row;
        for (int j=0;j<size;j++) {
          row.append((j+i) % size);
        }
        latinSquare4x4.append(row);
      }

      int iSubjectIDmod4 = iSubjectID % 4;    
      Array<int> condSequence   = latinSquare4x4[iSubjectIDmod4];

      cout << subjectID;
      // output block order    
      cout << ", " << condSequence.findIndex(0);
      cout << ", " << c0AvgTime;
      for (int m=0;m<c0AvgMetrics.size();m++) {
        cout << ", " << c0AvgMetrics[m];
      }
      cout << ", " << condSequence.findIndex(1);
      cout << ", " << c1AvgTime;
      for (int m=0;m<c1AvgMetrics.size();m++) {
        cout << ", " << c1AvgMetrics[m];
      }
      cout << ", " << condSequence.findIndex(2);
      cout << ", " << c2AvgTime;
      for (int m=0;m<c2AvgMetrics.size();m++) {
        cout << ", " << c2AvgMetrics[m];
      }
      cout << ", " << condSequence.findIndex(3);
      cout << ", " << c3AvgTime;
      for (int m=0;m<c3AvgMetrics.size();m++) {
        cout << ", " << c3AvgMetrics[m];
      }
      cout << endl;
    }


    if (outputType == 3) {
      double total = badCount + goodCount;
      double badP = 100.0 * (double)badCount / total;
      double goodP = 100.0 * (double)goodCount / total;
      cout << "Bad trials = " << badCount << " of " << total << " " << badP << "%" << endl;
      cout << "Good trials = " << goodCount << " of " << total << " " << goodP << "%" << endl;
    }


  
    if (outputType == 4) {
      // Loop through all data again, but print out differences from
      // the means for this subject rather than the actual data.

      calcPromptStats();

      int i = 0;
      while (fileExists(filebase + intToString(i) + ".trial-data")) { // loop through trials
        TracingTrialRef t = new TracingTrial(filebase + intToString(i) + ".trial-data", 
                                             _artwork, _cavePaintingCursors, 
                                             _eventMgr, _gfxMgr, _hciMgr, this);
        // Remove bad data
        bool badTrial = false;
        if ((t->_data.timeout) || (t->_data.throwOutData) || (t->_data.paintingTime <= 0.0)) {
          badTrial = true;
        }
        for (int o=0;o<outliers.size();o++) {
          if (i == outliers[o]) {
            badTrial = true;
          }
        }
        
        bool skip = false;
        if ((ConfigVal("SkipTraining", 1, false)) && (t->_data.training)) {
          skip = true;
        }
        if ((ConfigVal("OnlyTraining", 0, false)) && (!t->_data.training)) {
          skip = true;
        }

        if (!skip) {
          if (!badTrial) {            
            if (rescoreData) {
              t->rescoreData(promptMarks);
            }

            // Find diff from the mean for each metric
            Array<double> diffMetrics;
            double diffTime;

            if (t->_data.conditionID == 0) {
              diffTime = t->_data.paintingTime - c0AvgTime;
              for (int i=0;i<t->_data.metrics.size();i++) {
                diffMetrics.append(t->_data.metrics[i] - c0AvgMetrics[i]);
              }
            }
            if (t->_data.conditionID == 1) {
              diffTime = t->_data.paintingTime - c1AvgTime;
              for (int i=0;i<t->_data.metrics.size();i++) {
                diffMetrics.append(t->_data.metrics[i] - c1AvgMetrics[i]);
              }
            }
            if (t->_data.conditionID == 2) {
              diffTime = t->_data.paintingTime - c2AvgTime;
              for (int i=0;i<t->_data.metrics.size();i++) {
                diffMetrics.append(t->_data.metrics[i] - c2AvgMetrics[i]);
              }
            }
            if (t->_data.conditionID == 3) {
              diffTime = t->_data.paintingTime - c3AvgTime;
              for (int i=0;i<t->_data.metrics.size();i++) {
                diffMetrics.append(t->_data.metrics[i] - c3AvgMetrics[i]);
              }
            }

            std::string s = t->_data.subjectID + ", " +
              intToString(t->_data.trialNum) + ", " + 
              intToString(t->_data.conditionID) + ", " + 
              intToString(t->_data.promptIndex) + ", " +
              realToString(t->_data.promptLength) + ", " + 
              realToString(t->_data.totalTime) + ", " +
              realToString(t->_data.paintingTime) + ", " +
              realToString(diffTime) + ", " +
              intToString((int)t->_data.timeout) + ", " +
              intToString((int)t->_data.training) + ", " +
              intToString((int)t->_data.throwOutData);

            for (int i=0;i<t->_data.metrics.size();i++) {
              s = s + ", " + realToString(t->_data.metrics[i]) + ", " + realToString(diffMetrics[i]);
            }
            
            // also output the stats for this prompt
            cout << s << pArcLen[t->_data.promptIndex] << ", " 
                 << pXYZTotal[t->_data.promptIndex][0] << ", " << pXYZTotal[t->_data.promptIndex][1] << ", " << pXYZTotal[t->_data.promptIndex][2] << ", " 
                 << pXYZPos[t->_data.promptIndex][0] << ", " << pXYZPos[t->_data.promptIndex][1] << ", " << pXYZPos[t->_data.promptIndex][2] << ", " 
                 << pXYZNeg[t->_data.promptIndex][0] << ", " << pXYZNeg[t->_data.promptIndex][1] << ", " << pXYZNeg[t->_data.promptIndex][2] << ", " 
                 << pAD[t->_data.promptIndex] << ", "
                 << pADXYZ1[t->_data.promptIndex][0] << ", " << pADXYZ1[t->_data.promptIndex][1] << ", " << pADXYZ1[t->_data.promptIndex][2] << ", "
                 << pADXYZ2[t->_data.promptIndex][0] << ", " << pADXYZ2[t->_data.promptIndex][1] << ", " << pADXYZ2[t->_data.promptIndex][2] << endl;
          }
        }
        i++;
      }
    }

  } // end loop through subjects


  exit(0);
}

void
SteeringStudy::calcPromptStats()
{
  Array<MarkRef> promptMarks;
  // Load up and save the prompts
  std::string promptMarksFile = "computed-marks.3DArt";
  BinaryInput bi(promptMarksFile, G3D_LITTLE_ENDIAN, false);
  Color3 bgColor;
  ArtworkIO::deserialize(bi, _artwork, bgColor, NULL, NULL, NULL);
  promptMarks = _artwork->getMarks();

  for (int i=0;i<promptMarks.size();i++) {
    double arclen = ConfigVal("ResampleLength", 0.001, false);
    MarkRef resampPrompt = ScoreTrials::resampleMarkByArclength(promptMarks[i], arclen);

    pArcLen.append(promptMarks[i]->getArcLength());

    Vector3 total, pos, neg;
    ScoreTrials::changeInXYZ(resampPrompt, total, pos, neg);
    pXYZTotal.append(total);
    pXYZPos.append(pos);
    pXYZNeg.append(neg);

    pAD.append(ScoreTrials::totalAngleBetweenAdjTangents(resampPrompt));
    pADXYZ1.append(ScoreTrials::totalAngleBetweenAdjTangentsInXYZPlanes(resampPrompt));
    pADXYZ2.append(ScoreTrials::totalAngleBetweenAdjTangentsInXYZByDrawingDir(resampPrompt));
  }
}


void
SteeringStudy::printPromptStats()
{
  calcPromptStats();

  cout << "prompt l lx ly lz lxpos lypos lzpos lxneg lyneg lzneg ad adx1 ady1 adz1 adx2 ady2 adz2" << endl;
  for (int i=0;i<pArcLen.size();i++) {
    cout << i << ", " << pArcLen[i] << ", " 
         << pXYZTotal[i][0] << ", " << pXYZTotal[i][1] << ", " << pXYZTotal[i][2] << ", " 
         << pXYZPos[i][0] << ", " << pXYZPos[i][1] << ", " << pXYZPos[i][2] << ", " 
         << pXYZNeg[i][0] << ", " << pXYZNeg[i][1] << ", " << pXYZNeg[i][2] << ", " 
         << pAD[i] << ", "
         << pADXYZ1[i][0] << ", " << pADXYZ1[i][1] << ", " << pADXYZ1[i][2] << ", "
         << pADXYZ2[i][0] << ", " << pADXYZ2[i][1] << ", " << pADXYZ2[i][2] << endl;
  }
  exit(0);
}







void
SteeringStudy::printPiecewiseData()
{
  srand(time(0));
  cout.setf(ios::fixed,ios::floatfield);

  // Load up and save the prompts
  std::string promptMarksFile = "computed-marks.3DArt";
  BinaryInput bi(promptMarksFile, G3D_LITTLE_ENDIAN, false);
  Color3 bgColor;
  ArtworkIO::deserialize(bi, _artwork, bgColor, NULL, NULL, NULL);
  Array<MarkRef> promptMarks = _artwork->getMarks();
  _artwork->clear();


  int startID = ConfigVal("StartID", 1, false);
  int stopID = ConfigVal("StopID", 12, false);

  // Loop through all subjects
  for (int iSubjectID=startID;iSubjectID<=stopID;iSubjectID++) {
    std::string subjectID = intToString(iSubjectID);

    std::string filebase = "trial-data/" + subjectID + "/trial";
    std::string cvalstr = "Outliers_" + subjectID;
    Array<std::string> outstr = splitStringIntoArray(ConfigVal(cvalstr, "", false));
    Array<int> outliers;
    for (int o=0;o<outstr.size();o++) {
      outliers.append(stringToInt(outstr[o]));
    }

    int i = 0;
    while (fileExists(filebase + intToString(i) + ".trial-data")) { // loop through trials
      TracingTrialRef t = new TracingTrial(filebase + intToString(i) + ".trial-data", 
                                           _artwork, _cavePaintingCursors, 
                                           _eventMgr, _gfxMgr, _hciMgr, this);
      // Remove bad data
      bool badTrial = false;
      if ((t->_data.timeout) || (t->_data.throwOutData) || (t->_data.paintingTime <= 0.0)) {
        badTrial = true;
      }
      for (int o=0;o<outliers.size();o++) {
        if (i == outliers[o]) {
          badTrial = true;
        }
      }

      bool skip = false;
      if ((ConfigVal("SkipTraining", 1, false)) && (t->_data.training)) {
        skip = true;
      }
      if ((ConfigVal("OnlyTraining", 0, false)) && (!t->_data.training)) {
        skip = true;
      }

      if (ConfigVal("Condition", 0, false) != t->_data.conditionID) {
        skip = true;
      }


      if ((!skip) && (!badTrial)) {

        // Load up the drawn mark
        MarkRef drawnMark;
        BinaryInput bi2(t->_data.outputFile, G3D_LITTLE_ENDIAN, false);
        Color3 bgColor;
        ArtworkIO::deserialize(bi2, _artwork, bgColor, NULL, NULL, NULL);
        Array<MarkRef> marks = _artwork->getMarks();
        if (marks.size() == 2) {
          drawnMark = marks[1];
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
          drawnMark = marks[longest];
        }
        _artwork->clear();

        
        // Resample both marks
        double arclen = ConfigVal("PiecewiseResampleLength", 0.001, false);
        MarkRef resampPrompt = ScoreTrials::resampleMarkByArclength(promptMarks[t->_data.promptIndex], arclen);
        Array<double> drawingTimes;
        MarkRef resampDrawn = ScoreTrials::resampleMarkByArclength(drawnMark, arclen, 
                                                                   t->_data.paintingTime, drawingTimes);

        // Loop through the drawn mark and for each segment output the
        // following, where the pos and dir are averages over the
        // segment and the ad is the total over the segment.

        // subid, cond, prompt, posx, posy, posz, dirx, diry, dirz, ad, adx, ady, adz, c0, c1, c2
        
        // stepsize is the number of samples per segment
        int stepsize = ConfigVal("PiecewiseStepSize", 30, false);
        // skip the first and the last segment of each curve
        int skipsize = ConfigVal("PiecewiseBufferSize", 10, false);

        bool useWholeLine = ConfigVal("PiecewiseUseWholeLine", false, false);
        if (useWholeLine) {
          skipsize = 0;
          stepsize = resampDrawn->getNumSamples()-1;
        }

        int sstart = skipsize;
        if (ConfigVal("RandomStart", true, false)) {
          sstart = iRandom(0, resampDrawn->getNumSamples()-skipsize-stepsize);
          //cout << "starting at " << sstart << endl;
        }
        int sstop = resampDrawn->getNumSamples() - skipsize - stepsize;
        int sinc = stepsize;
        
        bool randomLength = ConfigVal("PiecewiseRandomLength", false, false);
        if (randomLength) {
          // picking stepsize before the start point should yield a
          // more uniform sampling of lengths

          bool ok=false;
          while (!ok) {
            int minlen = ConfigVal("MinRandomLength", 150, false);
            stepsize = iRandom(minlen, resampDrawn->getNumSamples()-1);
            sstart = resampDrawn->getNumSamples();
            int count =0;
            while (((sstart + stepsize) > resampDrawn->getNumSamples()-1) && (count < 1000)) {
              sstart = iRandom(0, resampDrawn->getNumSamples()-1-stepsize);
              sstop = sstart + stepsize;
              count++;
            }
            if (count < 1000) {
              ok = true;
            }
          }
        }
        
        // curvature
        Array<double> curvatures;
        Array<Vector3> curvCenters;
        ScoreTrials::curvature(resampPrompt, curvatures, curvCenters);

        for (int s=sstart;s<sstop;s+=sinc) {
          

          /** 
              - rewrite c0, c1, c2, and angular dev in ScoreTrials to
                accept start and end points
              - call those routines for everything
              - except calculate the avg pos and dir for both the
                prompt and the drawn curve here.
          */
 

          int promptStart = 0;
          int promptEnd = resampPrompt->getNumSamples()-1;
          if (!useWholeLine) {
            double dist1 = resampPrompt->distanceToMark(resampDrawn->getSamplePosition(s), promptStart);
            double dist2 = resampPrompt->distanceToMark(resampDrawn->getSamplePosition(s+stepsize), promptEnd);
          }

          Vector3 avgPosD;
          Vector3 avgDirD;
          for (int j=s;j<s+stepsize;j++) {
            avgPosD += resampDrawn->getSamplePosition(j);
            avgDirD += ScoreTrials::firstTangent(resampDrawn, j);
          }
          avgPosD /= (double)stepsize;
          avgDirD /= (double)stepsize;

          Vector3 avgPosP;
          Vector3 avgDirP;
          for (int j=promptStart;j<promptEnd;j++) {
            avgPosP += resampPrompt->getSamplePosition(j);
            avgDirP += ScoreTrials::firstTangent(resampPrompt, j);
          }
          avgPosP /= (double)(promptEnd - promptStart);
          avgDirP /= (double)(promptEnd - promptStart);


          double dtime;
          double tstart;
          if (useWholeLine) {
            dtime = t->_data.paintingTime;
            tstart = 0.0;
          }
          else {
            int dtend = s+stepsize;
            if (dtend >= drawingTimes.size()) {
              dtend = drawingTimes.size()-1;
            }
            dtime = drawingTimes[dtend] - drawingTimes[s];
            tstart = drawingTimes[s];
          }
          double l = resampPrompt->getArcLength(promptStart, promptEnd);
          double lstart = resampPrompt->getArcLength(0, promptStart);
          Vector3 lXYZ;
          Vector3 lposXYZ;
          Vector3 lnegXYZ;
          ScoreTrials::changeInXYZ(resampPrompt, lXYZ, lposXYZ, lnegXYZ, promptStart, promptEnd);
          double  totalAD = ScoreTrials::totalAngleBetweenAdjTangents(resampPrompt, promptStart, promptEnd);
          Vector3 totalADxyz = ScoreTrials::totalAngleBetweenAdjTangentsInXYZByDrawingDir(resampPrompt, promptStart, promptEnd);
          double c0distD = ScoreTrials::c0AvgDistToPromptAlongDrawn(resampPrompt, resampDrawn, s, s+stepsize);
          double c0distP = ScoreTrials::c0AvgDistToDrawnAlongPrompt(resampPrompt, resampDrawn, promptStart, promptEnd);
          double c0 = (c0distD + c0distP) / 2.0;
          double c1 = ScoreTrials::c1AvgAngleBetweenTangents(resampPrompt, resampDrawn, s, s+stepsize);
          double c2 = ScoreTrials::c2AvgAngleBetween2ndDeriv(resampPrompt, resampDrawn, s, s+stepsize);
            
          double avgCurv;
          double avgCurvSq;
          Vector3 avgDirRadCurv;
          int cStart = promptStart;
          int cEnd = promptEnd;
          if (useWholeLine) {
            cStart++;
            cEnd--;
          }
          int numC=0;
          for (int c=cStart;c<cEnd;c++) {
            double K = curvatures[c];
            Vector3 q = curvCenters[c];
            if ((isFinite(K)) && (K != 0.0) && (q.isFinite()) && (!q.isZero())) {
              avgCurv += K;
              avgCurvSq += K*K;
              avgDirRadCurv += (q - resampPrompt->getSamplePosition(c)).unit();
              numC++;
            }
          }
          avgCurv /= numC;
          avgCurvSq /= numC;
          avgDirRadCurv /= numC;
          avgDirRadCurv.unit();
          if (!avgDirRadCurv.isFinite()) {
            avgDirRadCurv = Vector3::zero();
          }

          
          cout << subjectID << ", " << t->_data.conditionID << ", " << t->_data.promptIndex << ", " <<
            dtime << ", " << tstart << ", " <<
            avgPosD[0] << ", " << avgPosD[1] << ", " << avgPosD[2] << ", " <<
            avgDirD[0] << ", " << avgDirD[1] << ", " << avgDirD[2] << ", " <<
            avgPosP[0] << ", " << avgPosP[1] << ", " << avgPosP[2] << ", " <<
            avgDirP[0] << ", " << avgDirP[1] << ", " << avgDirP[2] << ", " <<
            l << ", " << lstart << ", " << lXYZ[0] << ", " << lXYZ[1] << ", " << lXYZ[2] << ", " <<
            lposXYZ[0] << ", " << lposXYZ[1] << ", " << lposXYZ[2] << ", " <<
            lnegXYZ[0] << ", " << lnegXYZ[1] << ", " << lnegXYZ[2] << ", " <<
            totalAD << ", " << totalADxyz[0] << ", " << totalADxyz[1] << ", " << totalADxyz[2] << ", " <<
            c0 << ", " << c1 << ", " << c2 << ", " << 
            avgCurv << ", " << avgCurvSq << ", " << 
            avgDirRadCurv[0] << ", " << avgDirRadCurv[1] << ", " << avgDirRadCurv[2] << endl;

          if (randomLength) {
            // hack to only go through the for loop once if we're doing random length
            s = resampDrawn->getNumSamples();
          }
          if (useWholeLine) {
            // hack to only go through the for loop once if we're doing random length
            s = resampDrawn->getNumSamples();
          }
          
        }

      }
      i++;
    } // for each trial
  } // for each subject
  exit(0);
}








/** Idea here is to output a new dataset that has more variation in
    the prompts than the original.  We do this by randomly sampling
    the original datset and outputting segments of the original
    curves, so we end up with more variation in length and other
    features of the curves.  But, we want to sample with a few
    important constraints.  1. We want the original curves to appear
    in the dataset.  2. We want to avoid sampling from the same trail
    more than once since this oversampling might bias the data more
    toward a linear relationship that isn't actually in the data.

    So, the algorithm is to loop through the data.  Each subject
    traces each of the prompts 4 times in each condition, so one of
    those times we output the trial as is.  The other 3 times we
    output a randomly picked segment of the curve in the trial.
*/
void
SteeringStudy::printRandomDataset()
{
  srand(time(0));
  cout.setf(ios::fixed,ios::floatfield);

  // Load up and save the prompts
  std::string promptMarksFile = "computed-marks.3DArt";
  BinaryInput bi(promptMarksFile, G3D_LITTLE_ENDIAN, false);
  Color3 bgColor;
  ArtworkIO::deserialize(bi, _artwork, bgColor, NULL, NULL, NULL);
  Array<MarkRef> promptMarks = _artwork->getMarks();
  _artwork->clear();


  int startID = ConfigVal("StartID", 1, false);
  int stopID = ConfigVal("StopID", 12, false);

  // Loop through all subjects
  for (int iSubjectID=startID;iSubjectID<=stopID;iSubjectID++) {
    std::string subjectID = intToString(iSubjectID);

    std::string filebase = "trial-data/" + subjectID + "/trial";
    std::string cvalstr = "Outliers_" + subjectID;
    Array<std::string> outstr = splitStringIntoArray(ConfigVal(cvalstr, "", false));
    Array<int> outliers;
    for (int o=0;o<outstr.size();o++) {
      outliers.append(stringToInt(outstr[o]));
    }

    // We want to randomly select the trial for which we output the
    // entire curve rather than a random segment, so arrage that here.

    //   [cond][prompt] = count of number seen so far
    Array< Array<int> > promptCounts;
    //   [cond][prompt][count] = 1 if you should output the whole curve
    Array< Array< Array<int> > > outputWholeCurve;
    promptCounts.resize(4);
    outputWholeCurve.resize(4);

    for (int c=0;c<4;c++) {
      promptCounts[c].resize(5);
      outputWholeCurve[c].resize(5);
      for (int p=0;p<5;p++) {
        promptCounts[c][p] = 0;
        outputWholeCurve[c][p] = randomOrderedList(1,4);
      }
    }


    int i = 0;
    while (fileExists(filebase + intToString(i) + ".trial-data")) { // loop through trials
      TracingTrialRef t = new TracingTrial(filebase + intToString(i) + ".trial-data", 
                                           _artwork, _cavePaintingCursors, 
                                           _eventMgr, _gfxMgr, _hciMgr, this);
      // Remove bad data
      bool badTrial = false;
      if ((t->_data.timeout) || (t->_data.throwOutData) || (t->_data.paintingTime <= 0.0)) {
        badTrial = true;
      }
      for (int o=0;o<outliers.size();o++) {
        if (i == outliers[o]) {
          badTrial = true;
        }
      }

      bool skip = false;
      if ((ConfigVal("SkipTraining", 1, false)) && (t->_data.training)) {
        skip = true;
      }
      if ((ConfigVal("OnlyTraining", 0, false)) && (!t->_data.training)) {
        skip = true;
      }

      if (ConfigVal("Condition", 0, false) != t->_data.conditionID) {
        skip = true;
      }


      if ((!skip) && (!badTrial)) {

        // Load up the drawn mark
        MarkRef drawnMark;
        BinaryInput bi2(t->_data.outputFile, G3D_LITTLE_ENDIAN, false);
        Color3 bgColor;
        ArtworkIO::deserialize(bi2, _artwork, bgColor, NULL, NULL, NULL);
        Array<MarkRef> marks = _artwork->getMarks();
        if (marks.size() == 2) {
          drawnMark = marks[1];
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
          drawnMark = marks[longest];
        }
        _artwork->clear();

        
        // Resample both marks
        double arclen = ConfigVal("PiecewiseResampleLength", 0.001, false);
        MarkRef resampPrompt = ScoreTrials::resampleMarkByArclength(promptMarks[t->_data.promptIndex], arclen);
        Array<double> drawingTimes;
        MarkRef resampDrawn = ScoreTrials::resampleMarkByArclength(drawnMark, arclen, 
                                                                   t->_data.paintingTime, drawingTimes);


        
        bool useWholeLine = (outputWholeCurve[t->_data.conditionID][t->_data.promptIndex][promptCounts[t->_data.conditionID][t->_data.promptIndex]] == 1);
        promptCounts[t->_data.conditionID][t->_data.promptIndex]++;

        
        // default to using the whole line
        int sstart = 0;
        int sstop = resampDrawn->getNumSamples();

        // otherwise pick start and stop at random
        if (!useWholeLine) {
          // picking stepsize before the start point yields a more
          // uniform sampling of lengths
          bool ok=false;
          while (!ok) {
            int minlen = ConfigVal("MinRandomLength", 150, false);
            int stepsize = iRandom(minlen, resampDrawn->getNumSamples()-1);
            sstart = resampDrawn->getNumSamples();
            int count =0;
            while (((sstart + stepsize) > resampDrawn->getNumSamples()-1) && (count < 1000)) {
              sstart = iRandom(0, resampDrawn->getNumSamples()-1-stepsize);
              sstop = sstart + stepsize;
              count++;
            }
            if (count < 1000) {
              ok = true;
            }
          }
        }
        
        // curvature
        Array<double> curvatures;
        Array<Vector3> curvCenters;
        ScoreTrials::curvature(resampPrompt, curvatures, curvCenters);
 
        int promptStart = 0;
        int promptEnd = resampPrompt->getNumSamples()-1;
        if (!useWholeLine) {
          double dist1 = resampPrompt->distanceToMark(resampDrawn->getSamplePosition(sstart), promptStart);
          double dist2 = resampPrompt->distanceToMark(resampDrawn->getSamplePosition(sstop-1), promptEnd);
        }

        Vector3 avgPosD;
        Vector3 avgDirD;
        for (int j=sstart;j<sstop;j++) {
          avgPosD += resampDrawn->getSamplePosition(j);
          avgDirD += ScoreTrials::firstTangent(resampDrawn, j);
        }
        avgPosD /= (double)(sstop-sstart);
        avgDirD /= (double)(sstop-sstart);
        
        Vector3 avgPosP;
        Vector3 avgDirP;
        for (int j=promptStart;j<promptEnd;j++) {
          avgPosP += resampPrompt->getSamplePosition(j);
          avgDirP += ScoreTrials::firstTangent(resampPrompt, j);
        }
        avgPosP /= (double)(promptEnd - promptStart);
        avgDirP /= (double)(promptEnd - promptStart);
        

        double dtime;
        double tstart;
        if (useWholeLine) {
          dtime = t->_data.paintingTime;
          tstart = 0.0;
        }
        else {
          int dtend = sstop-1;
          if (dtend >= drawingTimes.size()) {
            dtend = drawingTimes.size()-1;
          }
          dtime = drawingTimes[dtend] - drawingTimes[sstart];
          tstart = drawingTimes[sstart];
        }
        double l = resampPrompt->getArcLength(promptStart, promptEnd);
        double lstart = resampPrompt->getArcLength(0, promptStart);
        Vector3 lXYZ;
        Vector3 lposXYZ;
        Vector3 lnegXYZ;
        ScoreTrials::changeInXYZ(resampPrompt, lXYZ, lposXYZ, lnegXYZ, promptStart, promptEnd);
        double  totalAD = ScoreTrials::totalAngleBetweenAdjTangents(resampPrompt, promptStart, promptEnd);
        Vector3 totalADxyz = ScoreTrials::totalAngleBetweenAdjTangentsInXYZByDrawingDir(resampPrompt, promptStart, promptEnd);
        double c0distD = ScoreTrials::c0AvgDistToPromptAlongDrawn(resampPrompt, resampDrawn, sstart, sstop);
        double c0distP = ScoreTrials::c0AvgDistToDrawnAlongPrompt(resampPrompt, resampDrawn, promptStart, promptEnd);
        double c0 = (c0distD + c0distP) / 2.0;
        double c1 = ScoreTrials::c1AvgAngleBetweenTangents(resampPrompt, resampDrawn, sstart, sstop);
        double c2 = ScoreTrials::c2AvgAngleBetween2ndDeriv(resampPrompt, resampDrawn, sstart, sstop);
        
        double avgCurv;
        double avgCurvSq;
        Vector3 avgDirRadCurv;
        int cStart = promptStart;
        int cEnd = promptEnd;
        if (useWholeLine) {
          cStart++;
          cEnd--;
        }
        int numC=0;
        for (int c=cStart;c<cEnd;c++) {
          double K = curvatures[c];
          Vector3 q = curvCenters[c];
          if ((isFinite(K)) && (K != 0.0) && (q.isFinite()) && (!q.isZero())) {
            avgCurv += K;
            avgCurvSq += K*K;
            avgDirRadCurv += (q - resampPrompt->getSamplePosition(c)).unit();
            numC++;
          }
        }
        avgCurv /= numC;
        avgCurvSq /= numC;
        avgDirRadCurv /= numC;
        avgDirRadCurv.unit();
        if (!avgDirRadCurv.isFinite()) {
          avgDirRadCurv = Vector3::zero();
        }
        
        
        cout << subjectID << ", " << t->_data.conditionID << ", " << t->_data.promptIndex << ", " <<
          dtime << ", " << tstart << ", " <<
          avgPosD[0] << ", " << avgPosD[1] << ", " << avgPosD[2] << ", " <<
          avgDirD[0] << ", " << avgDirD[1] << ", " << avgDirD[2] << ", " <<
          avgPosP[0] << ", " << avgPosP[1] << ", " << avgPosP[2] << ", " <<
          avgDirP[0] << ", " << avgDirP[1] << ", " << avgDirP[2] << ", " <<
          l << ", " << lstart << ", " << lXYZ[0] << ", " << lXYZ[1] << ", " << lXYZ[2] << ", " <<
          lposXYZ[0] << ", " << lposXYZ[1] << ", " << lposXYZ[2] << ", " <<
          lnegXYZ[0] << ", " << lnegXYZ[1] << ", " << lnegXYZ[2] << ", " <<
          totalAD << ", " << totalADxyz[0] << ", " << totalADxyz[1] << ", " << totalADxyz[2] << ", " <<
          c0 << ", " << c1 << ", " << c2 << ", " << 
          avgCurv << ", " << avgCurvSq << ", " << 
          avgDirRadCurv[0] << ", " << avgDirRadCurv[1] << ", " << avgDirRadCurv[2] << endl; 
      } // end if good trial
      i++;
    } // for each trial
  } // for each subject
  exit(0);
}




void
SteeringStudy::printEqualSegments()
{
  cout.setf(ios::fixed,ios::floatfield);

  // Load up and save the prompts
  std::string promptMarksFile = "computed-marks.3DArt";
  BinaryInput bi(promptMarksFile, G3D_LITTLE_ENDIAN, false);
  Color3 bgColor;
  ArtworkIO::deserialize(bi, _artwork, bgColor, NULL, NULL, NULL);
  Array<MarkRef> promptMarks = _artwork->getMarks();
  _artwork->clear();


  int startID = ConfigVal("StartID", 1, false);
  int stopID = ConfigVal("StopID", 12, false);


  // output header
  cout << "subid, cond, prompt, tdraw, tstart, avgPosDx, avgPosDy, avgPosDz, avgDirDx, avgDirDy, avgDirDz, avgPosPx, avgPosPy, avgPosPz, avgDirPx, avgDirPy, avgDirPz, l, lstart, lx, ly, lz, lposx, lposy, lposz, lnegx, lnegy, lnegz, adtotal, adtotalx, adtotaly, adtotalz, c0, c1, c2, avgcrv, avgcrvsq, avgcrvdx, avgcrvdy, avgcrvdz, adyz, adxz, adxy" << endl;

  // Loop through all subjects
  for (int iSubjectID=startID;iSubjectID<=stopID;iSubjectID++) {
    std::string subjectID = intToString(iSubjectID);

    std::string filebase = "trial-data/" + subjectID + "/trial";
    std::string cvalstr = "Outliers_" + subjectID;
    Array<std::string> outstr = splitStringIntoArray(ConfigVal(cvalstr, "", false));
    Array<int> outliers;
    for (int o=0;o<outstr.size();o++) {
      outliers.append(stringToInt(outstr[o]));
    }



    int i = 0;
    while (fileExists(filebase + intToString(i) + ".trial-data")) { // loop through trials
      TracingTrialRef t = new TracingTrial(filebase + intToString(i) + ".trial-data", 
                                           _artwork, _cavePaintingCursors, 
                                           _eventMgr, _gfxMgr, _hciMgr, this);
      // Remove bad data
      bool badTrial = false;
      if ((t->_data.timeout) || (t->_data.throwOutData) || (t->_data.paintingTime <= 0.0)) {
        badTrial = true;
      }
      for (int o=0;o<outliers.size();o++) {
        if (i == outliers[o]) {
          badTrial = true;
        }
      }

      bool skip = false;
      if ((ConfigVal("SkipTraining", 1, false)) && (t->_data.training)) {
        skip = true;
      }
      if ((ConfigVal("OnlyTraining", 0, false)) && (!t->_data.training)) {
        skip = true;
      }


      if ((!skip) && (!badTrial)) {

        // Load up the drawn mark
        MarkRef drawnMark;
        BinaryInput bi2(t->_data.outputFile, G3D_LITTLE_ENDIAN, false);
        Color3 bgColor;
        ArtworkIO::deserialize(bi2, _artwork, bgColor, NULL, NULL, NULL);
        Array<MarkRef> marks = _artwork->getMarks();
        if (marks.size() == 2) {
          drawnMark = marks[1];
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
          drawnMark = marks[longest];
        }
        _artwork->clear();

        
        // Resample both marks
        double arclen = ConfigVal("PiecewiseResampleLength", 0.001, false);
        MarkRef resampPrompt = ScoreTrials::resampleMarkByArclength(promptMarks[t->_data.promptIndex], arclen);
        Array<double> drawingTimes;
        MarkRef resampDrawn = ScoreTrials::resampleMarkByArclength(drawnMark, arclen, 
                                                                   t->_data.paintingTime, drawingTimes);
       
        // curvature
        Array<double> curvatures;
        Array<Vector3> curvCenters;
        ScoreTrials::curvature(resampPrompt, curvatures, curvCenters);


        // Stepsize is the size of the segments
        int stepsize = ConfigVal("PiecewiseStepSize", 33, false);
        // Buffer is the portion of the curve to skip at the beginning and the end, default 1cm
        int buffer = ConfigVal("PiecewiseBufferSize", 33, false);
        for (int pstart=buffer; pstart < resampPrompt->getNumSamples()-stepsize-buffer; pstart += stepsize) {

          int pend = pstart + stepsize;
          int dstart;
          int dend;
          double dist1 = resampDrawn->distanceToMark(resampPrompt->getSamplePosition(pstart), dstart);
          double dist2 = resampDrawn->distanceToMark(resampPrompt->getSamplePosition(pend), dend);
          
          // throw this segment out if the length of the drawn prompt
          // is less than half of the length of the prompt.
          if ((dend - dstart) > iRound(0.5*(float)stepsize)) {

            Vector3 avgPosD;
            Vector3 avgDirD;
            for (int j=dstart;j<dend;j++) {
              avgPosD += resampDrawn->getSamplePosition(j);
              avgDirD += ScoreTrials::firstTangent(resampDrawn, j);
            }
            avgPosD /= (double)(dend-dstart);
            avgDirD = avgDirD.unit();
            if (!avgDirD.isFinite()) {
              avgDirD = Vector3::zero();
            }
            
            Vector3 avgPosP;
            Vector3 avgDirP;
            for (int j=pstart;j<pend;j++) {
              avgPosP += resampPrompt->getSamplePosition(j);
              avgDirP += ScoreTrials::firstTangent(resampPrompt, j);
            }
            avgPosP /= (double)(pend - pstart);
            avgDirP = avgDirP.unit();
            if (!avgDirP.isFinite()) {
              avgDirP = Vector3::zero();
            }

            double dtime;
            if (dend >= drawingTimes.size()) {
              dtime = drawingTimes.last() - drawingTimes[dstart];
            }
            else {
              dtime = drawingTimes[dend] - drawingTimes[dstart];
            }
            double tstart = drawingTimes[dstart];
            
            double l = resampPrompt->getArcLength(pstart, pend);
            double lstart = resampPrompt->getArcLength(0, pstart);
            Vector3 lXYZ;
            Vector3 lposXYZ;
            Vector3 lnegXYZ;
            ScoreTrials::changeInXYZ(resampPrompt, lXYZ, lposXYZ, lnegXYZ, pstart, pend);
            double  totalAD = ScoreTrials::totalAngleBetweenAdjTangents(resampPrompt, pstart, pend);
            Vector3 totalADxyz = ScoreTrials::totalAngleBetweenAdjTangentsInXYZByDrawingDir(resampPrompt, pstart, pend);
            double c0distD = ScoreTrials::c0AvgDistToPromptAlongDrawn(resampPrompt, resampDrawn, dstart, dend);
            double c0distP = ScoreTrials::c0AvgDistToDrawnAlongPrompt(resampPrompt, resampDrawn, pstart, pend);
            double c0 = (c0distD + c0distP) / 2.0;
            double c1 = ScoreTrials::c1AvgAngleBetweenTangents(resampPrompt, resampDrawn, dstart, dend);
            double c2 = ScoreTrials::c2AvgAngleBetween2ndDeriv(resampPrompt, resampDrawn, dstart, dend);
            Vector3 adInPlanes = ScoreTrials::totalAngleBetweenAdjTangentsInXYZPlanes(resampPrompt, pstart, pend);        

            double avgCurv;
            double avgCurvSq;
            Vector3 avgDirRadCurv;
            int cStart = iMax(pstart, 1);
            int cEnd = iMin(pend, resampPrompt->getNumSamples()-2);
            int numC=0;
            for (int c=cStart;c<cEnd;c++) {
              double K = curvatures[c];
              Vector3 q = curvCenters[c];
              if ((isFinite(K)) && (K != 0.0) && (q.isFinite()) && (!q.isZero())) {
                avgCurv += K;
                avgCurvSq += K*K;
                avgDirRadCurv += (q - resampPrompt->getSamplePosition(c)).unit();
                numC++;
              }
            }
            avgCurv /= numC;
            avgCurvSq /= numC;
            avgDirRadCurv /= numC;
            avgDirRadCurv.unit();
            if (!avgDirRadCurv.isFinite()) {
              avgDirRadCurv = Vector3::zero();
            }
            
        
            cout << subjectID << ", " << t->_data.conditionID << ", " << t->_data.promptIndex << ", " <<
              dtime << ", " << tstart << ", " <<
              avgPosD[0] << ", " << avgPosD[1] << ", " << avgPosD[2] << ", " <<
              avgDirD[0] << ", " << avgDirD[1] << ", " << avgDirD[2] << ", " <<
              avgPosP[0] << ", " << avgPosP[1] << ", " << avgPosP[2] << ", " <<
              avgDirP[0] << ", " << avgDirP[1] << ", " << avgDirP[2] << ", " <<
              l << ", " << lstart << ", " << lXYZ[0] << ", " << lXYZ[1] << ", " << lXYZ[2] << ", " <<
              lposXYZ[0] << ", " << lposXYZ[1] << ", " << lposXYZ[2] << ", " <<
              lnegXYZ[0] << ", " << lnegXYZ[1] << ", " << lnegXYZ[2] << ", " <<
              totalAD << ", " << totalADxyz[0] << ", " << totalADxyz[1] << ", " << totalADxyz[2] << ", " <<
              c0 << ", " << c1 << ", " << c2 << ", " << 
              avgCurv << ", " << avgCurvSq << ", " << 
              avgDirRadCurv[0] << ", " << avgDirRadCurv[1] << ", " << avgDirRadCurv[2] << ", " <<
              adInPlanes[0] << ", " << adInPlanes[1] << ", " << adInPlanes[2] << endl; 
          }
        }  // end if length of segment is reasonable
      } // end if not skipping this trial
      i++;
    } // for each trial
  } // for each subject
  exit(0);
}


