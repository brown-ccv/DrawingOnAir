/**
 * /author Dan Keefe (dfk)
 *
 * /file  SteeringStudy.H
 * /brief Steering Law, tracing task study application.
 *
 */ 

#ifndef STEERINGSTUDY_H
#define STEERINGSTUDY_H

#include <DrawOnAir.H>

#include "TracingTrial.H"


class SteeringStudy : public DrawingStudy
{
public:
  SteeringStudy(int argc, char **argv);
  virtual ~SteeringStudy();
  
  virtual void doUserInput(Array<VRG3D::EventRef> &events);
  virtual void doGraphics(RenderDevice *rd);

  void printData();

  /// Note: keeping these old print routines around just in case, but
  /// the latest output routine and most bug free is this one:
  void printDatasetForSteeringAnalysis(int datasetType);
  void outputDataForSegment(MarkRef pMark, int pstart, int pend,
                            MarkRef dMark, int dstart, int dend,
                            Array<double> drawingTimes,
                            Array<double> curvatures, Array<Vector3> curvCenters,
                            std::string subjectID, TracingTrialRef t);

  void printDatasetWithMeansForSteeringAnalysis();

  void printEqualSegments();
  void printPiecewiseData();
  void printRandomDataset();
  void calcPromptStats();
  void printPromptStats();
  void generateMarks();
  void generateMarksForDynamicDrawingStudy();
  void setInputCondition(int cond);

private:
  Array<TracingTrialRef> _trials;
  int                    _activeTrial;


  // for prompt stats
  Array<double>  pArcLen;
  Array<Vector3> pXYZTotal;
  Array<Vector3> pXYZPos;
  Array<Vector3> pXYZNeg;
  Array<double>  pAD;
  Array<Vector3> pADXYZ1;
  Array<Vector3> pADXYZ2;
};


#endif

