
#ifndef TRACINGTRIAL_H
#define TRACINGTRIAL_H

#include <DrawOnAir.H>

typedef ReferenceCountedPointer<class TracingTrial> TracingTrialRef;
/**
*/
class TracingTrial : public ReferenceCountedObject
{
public:

  class Data {
  public:
    // Trial Setup
    int           trialNum;     // order in the study battery
    int           conditionID;  // id of the input condition
    std::string   subjectID;    // subject id
    std::string   promptFile;   // name of the 3DArt file that the prompt comes from
    int           promptIndex;  // index of the prompt mark within that file.  i.e. the n-th mark
    double        promptLength; // arclength of the prompt mark
    bool          training;     // true if training feedback was displayed during the trial
    std::string   outputFile;   // 3DArt file where the drawn mark is saved
    // Trial Performance
    double        totalTime;    // total time on this trial1
    double        paintingTime; // time spent actually painting
    bool          timeout;      // true if no mark was drawn
    bool          throwOutData; // true if subject indicated that something went wrong and
                                // the data should be thrown out
    Array<double> metrics;      // values for scoring metrics
  };

  TracingTrial(
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
      DrawingStudy*          drawingStudyApp);

  /// Loads data from a completed trial saved in a file.  Does not
  /// actually load the promptMark and drawnMark because it's so slow.
  TracingTrial(const std::string      &filename, 
               ArtworkRef             artwork,
               CavePaintingCursorsRef cursors,
               EventMgrRef            eventMgr,
               GfxMgrRef              gfxMgr,
               HCIMgrRef              hciMgr,
               DrawingStudy*          drawingStudyApp);

  virtual ~TracingTrial();

  /// Assumes trial was just loaded from disk via constructor #2.
  /// Then, this loads up the drawnMark from the .3DArt file and
  /// selects the correct prompt from the array passed in here for
  /// speed, and then recomputes the scoring metrics.
  void rescoreData(Array<MarkRef> prompts);

  void run();
  bool getCompleted();

  Data getData() { return _data; }
  void computeMetrics();
  void printMetrics();
  void printDataFileHeader();

  void updateEachFrame();
  void stopTiming();
  void writeData();

  void serialize(class BinaryOutput& b);
  void deserialize(class BinaryInput& b);

  void brushOn(VRG3D::EventRef e);
  void brushOff(VRG3D::EventRef e);
  void pauseBtnUp(VRG3D::EventRef e);
  void done(VRG3D::EventRef e);
  void drawingTimeout(VRG3D::EventRef e);
  void markDataAsBad(VRG3D::EventRef e);

  void draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace);

  void showInstructionsThenRun();
  void stopInstructions(VRG3D::EventRef e);
  void drawInstructions(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace);
  void instStartNewTimer(VRG3D::EventRef e);

  void drawCylinderOnMark(RenderDevice *rd, MarkRef m, int sample);


  Data                   _data;

protected:
  ArtworkRef             _artwork;
  CavePaintingCursorsRef _cursors;
  EventMgrRef            _eventMgr;
  FsaRef                 _fsa;
  GfxMgrRef              _gfxMgr;
  HCIMgrRef              _hciMgr;
  DrawingStudy*          _drawingStudyApp;
  
  MarkRef                _promptMark;
  Array<MarkRef>         _promptsToDraw;
  std::string            _outputFilename;
  double                 _startTrialTime;
  double                 _startPaintingTime;
  double                 _stopPaintingTime;
  int                    _drawID;
  std::string            _name;
  MarkRef                _drawnMark;
  Vector3                _startTextPos;
  Vector3                _endTextPos;
  int                    _initialNMarks;
  bool                   _done;
  int                    _instDrawID;
  FsaRef                 _instFSA;
  int                    _totalNumTrials;
  bool                   _runCalled;
  double                 _posScore, _smoothScore;
  Array<int>             _cylinderIndices;
};


#endif

