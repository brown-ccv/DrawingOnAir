
#ifndef SCORETRIALS_H
#define SCORETRIALS_H

#include <DrawOnAir.H>

class ScoreTrials
{
public:
  static double c0NormalizedAreaBetweenCurves(MarkRef promptMark, MarkRef drawnMark);
  static double c0AvgDistToDrawnAlongPrompt(MarkRef promptMark, MarkRef drawnMark, int start=-1, int stop=-1);
  static double c0AvgDistToPromptAlongDrawn(MarkRef promptMark, MarkRef drawnMark, int start=-1, int stop=-1);
  static double c1AvgAngleBetweenTangents(MarkRef promptMark, MarkRef drawnMark, int start=-1, int stop=-1);
  static double c2AvgAngleBetween2ndDeriv(MarkRef promptMark, MarkRef drawnMark, int start=-1, int stop=-1);

  static double startPointsError(MarkRef promptMark, MarkRef drawnMark);
  static double endPointsError(MarkRef promptMark, MarkRef drawnMark);

  static MarkRef resampleMarkByArclength(MarkRef origMark, double sampleInterval);
  static MarkRef resampleMarkByArclength(MarkRef origMark, double sampleInterval, double totalTime, Array<double> &times);

  static double avgDiffInWidth(MarkRef promptMark, MarkRef drawnMark);
  static double avgDiffInDerivOfWidth(MarkRef promptMark, MarkRef drawnMark);
  
  static double maxDistError(MarkRef promptMark, MarkRef drawnMark);

  static void changeInXYZ(MarkRef mark, Vector3 &total, Vector3 &pos, Vector3 &neg, int start=-1, int stop=-1);
  static double totalAngleBetweenAdjTangents(MarkRef mark, int start=-1, int stop=-1);
  static Vector3 totalAngleBetweenAdjTangentsInXYZPlanes(MarkRef mark, int start=-1, int stop=-1);
  static Vector3 totalAngleBetweenAdjTangentsInXYZByDrawingDir(MarkRef mark, int start=-1, int stop=-1);

  static Vector3 firstTangent(MarkRef mark, int i);
  static Vector3 secondTangent(MarkRef mark, int i);

  static void curvature(MarkRef mark, Array<double> &curvatures, Array<Vector3> &centerOfCurvatures);

};


#endif

