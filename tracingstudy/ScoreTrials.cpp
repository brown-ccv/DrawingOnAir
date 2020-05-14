#include "ScoreTrials.H"


MarkRef 
ScoreTrials::resampleMarkByArclength(MarkRef origMark, double sampleInterval)
{
  MarkRef mnew = origMark->copy();
  // strip all samples except for the first one
  mnew->trimEnd(0);
  
  BrushStateRef bs1 = origMark->getBrushState(0);
  BrushStateRef bs2 = origMark->getBrushState(1);
  int bs2index = 1;
  bool done = false;
  while (!done) {
    double d = (bs2->frameInRoomSpace.translation - bs1->frameInRoomSpace.translation).length();
    if (d >= sampleInterval) {
      double alpha = sampleInterval/d;
      BrushStateRef bsnew = bs1->lerp(bs2, alpha);
      mnew->addSample(bsnew);
      bs1 = bsnew;
    }
    else {
      bs2index++;
      if (bs2index >= origMark->getNumSamples()) {
        done = true;
      }
      else {
        bs2 = origMark->getBrushState(bs2index);
      }
    }
  }
  // add the last sample just to make sure that we really captured that point
  BrushStateRef bsnew = origMark->getBrushState(origMark->getNumSamples()-1)->copy();
  mnew->addSample(bsnew);
  return mnew;
}

MarkRef 
ScoreTrials::resampleMarkByArclength(MarkRef origMark, double sampleInterval, double totalTime, Array<double> &times)
{
  double timeInc = totalTime / origMark->getNumSamples();

  MarkRef mnew = origMark->copy();
  // strip all samples except for the first one
  mnew->trimEnd(0);
  times.append(0.0);
  
  BrushStateRef bs1 = origMark->getBrushState(0);
  BrushStateRef bs2 = origMark->getBrushState(1);
  int bs2index = 1;
  bool done = false;
  while (!done) {
    double d = (bs2->frameInRoomSpace.translation - bs1->frameInRoomSpace.translation).length();
    if (d >= sampleInterval) {
      double alpha = sampleInterval/d;
      BrushStateRef bsnew = bs1->lerp(bs2, alpha);
      mnew->addSample(bsnew);
      times.append((double)bs2index*timeInc - (1.0-alpha)*timeInc);
      bs1 = bsnew;
    }
    else {
      bs2index++;
      if (bs2index >= origMark->getNumSamples()) {
        done = true;
      }
      else {
        bs2 = origMark->getBrushState(bs2index);
      }
    }
  }
  // add the last sample just to make sure that we really captured that point
  BrushStateRef bsnew = origMark->getBrushState(origMark->getNumSamples()-1)->copy();
  mnew->addSample(bsnew);
  times.append(totalTime);
  return mnew;
}


// 1st derivative
Vector3 
ScoreTrials::firstTangent(MarkRef mark, int i) {
  Vector3 tangent;
  if (i == 0) {
    tangent = (mark->getSamplePosition(1) - mark->getSamplePosition(0)).unit();
  }
  else if (i == mark->getNumSamples()-1) {
    tangent = (mark->getSamplePosition(i) - mark->getSamplePosition(i-1)).unit();
  }
  else {
    tangent = (mark->getSamplePosition(i+1) - mark->getSamplePosition(i-1)).unit();
  }
  return tangent;
}

// 2nd derivative
Vector3 
ScoreTrials::secondTangent(MarkRef mark, int i) {
  Vector3 tangent;
  if (i == 0) {
    tangent = (firstTangent(mark, 1) - firstTangent(mark, 0)).unit();
  }
  else if (i == mark->getNumSamples()-1) {
    tangent = (firstTangent(mark, i) - firstTangent(mark, i-1)).unit();
  }
  else {
    tangent = (firstTangent(mark, i+1) - firstTangent(mark, i-1)).unit();
  }
  return tangent;
}




/** Computes the area between the prompt and drawn curves divided by
    the length of the prompt curve, so area/arclength, units are in
    mm.
*/
double
ScoreTrials::c0NormalizedAreaBetweenCurves(MarkRef promptMark, MarkRef drawnMark)
{
  int ip = 0;
  int endp = promptMark->getNumSamples()-1;

  int id = 0;
  int incd = 1;
  int endd = drawnMark->getNumSamples()-1;

  // reverse the traversal of the drawn curve if necessary
  double lnorm = (drawnMark->getSamplePosition(0) - promptMark->getSamplePosition(0)).length();
  double lrev  = (drawnMark->getSamplePosition(drawnMark->getNumSamples()-1) - promptMark->getSamplePosition(0)).length();
  if (lrev < lnorm) {
    // reverse it
    id = drawnMark->getNumSamples()-1;
    incd = -1;
    endd = 0;
  }

  double areaBetweenCurves = 0.0;
  while ((ip != endp) || (id != endd)) {
    double ap = 0.0;
    double ad = 0.0;

    if (ip < promptMark->getNumSamples()-1) {
      ap = Triangle(promptMark->getSamplePosition(ip), promptMark->getSamplePosition(ip+1), drawnMark->getSamplePosition(id)).area();
    }
    if ((id+incd < drawnMark->getNumSamples()) && (id+incd >= 0)) {
      ad = Triangle(drawnMark->getSamplePosition(id), drawnMark->getSamplePosition(id+incd), promptMark->getSamplePosition(ip)).area();
    }
    
    if ((ap == 0.0) && (ad == 0.0)) {
      if (ip != endp) 
        ip++;
      if (id != endd) 
        id+= incd;
    }
    else if (ap == 0.0) {
      areaBetweenCurves += ad;
      if (id != endd) 
        id += incd;
    }
    else if (ad == 0.0) {
      areaBetweenCurves += ap;
      if (ip != endp) 
        ip++;
    }
    else {
      if (ap < ad) {
        areaBetweenCurves += ap;
        if (ip != endp) 
          ip++;
      }
      else {
        areaBetweenCurves += ad;
        if (id != endd) 
          id += incd;
      }
    }

    //cout << ip << " " << id << " " << ap << " " << ad << endl;
  }

  double promptMarkLength = 0.0;
  for (int i=1;i<promptMark->getNumSamples();i++) {
    promptMarkLength += (promptMark->getSamplePosition(i) - promptMark->getSamplePosition(i-1)).length();
  }

  double e = areaBetweenCurves / promptMarkLength;
  // convert units to mm
  e = e * 304.8;
  //cout << "Num Samples = " << promptMark->getNumSamples() << " " << drawnMark->getNumSamples() << endl;
  //cout << "Area between curves = " << areaBetweenCurves << endl;
  //cout << "Length of prompt curve = " << promptMarkLength << endl;
  //cout << "C0: Normalized Area Between Curves = " << e << endl;
  return e;
}




/** Computes the average distance to the prompt curve along the length
    of the drawn curve, units in mm.
*/
double
ScoreTrials::c0AvgDistToPromptAlongDrawn(MarkRef promptMark, MarkRef drawnMark, int start, int end)
{
  double sum=0.0;
  if (start == -1) {
    start = 0;
  }
  if (end == -1) {
    end = drawnMark->getNumSamples();
  }
  for (int i=start;i<end;i++) {
    sum += promptMark->distanceToMark(drawnMark->getSamplePosition(i));
  }
  double e = sum / drawnMark->getNumSamples();
  // convert units to mm.
  e = e * 304.8;
  //cout << "C0: Avg Dist to Prompt Along Drawn = " << e << endl;
  return e;
}


double
ScoreTrials::c0AvgDistToDrawnAlongPrompt(MarkRef promptMark, MarkRef drawnMark, int start, int end)
{
  double sum=0.0;
  if (start == -1) {
    start = 0;
  }
  if (end == -1) {
    end = promptMark->getNumSamples();
  }
  for (int i=start;i<end;i++) {
    sum += drawnMark->distanceToMark(promptMark->getSamplePosition(i));
  }
  double e = sum / promptMark->getNumSamples();
  // convert units to mm.
  e = e * 304.8;
  // cout << "C0: Avg Dist to Drawn Along Prompt = " << e << endl;
  return e;
}


// Error in aligning the start points
double
ScoreTrials::startPointsError(MarkRef promptMark, MarkRef drawnMark)
{
  double dsnorm = (drawnMark->getSamplePosition(0) - promptMark->getSamplePosition(0)).length();
  double dsrev  = (drawnMark->getSamplePosition(drawnMark->getNumSamples()-1) - promptMark->getSamplePosition(0)).length();
  double dstart = G3D::min(dsnorm, dsrev);
  double e = dstart * 304.8;
  //cout << "Start Points Error = " << e << endl;
  return e;
}

double 
ScoreTrials::endPointsError(MarkRef promptMark, MarkRef drawnMark)
{
  double derev  = (drawnMark->getSamplePosition(0) - promptMark->getSamplePosition(promptMark->getNumSamples()-1)).length();
  double denorm = (drawnMark->getSamplePosition(drawnMark->getNumSamples()-1) - promptMark->getSamplePosition(promptMark->getNumSamples()-1)).length();
  double dend   = G3D::min(denorm, derev);
  double e = dend * 304.8;
  //cout << "End Points Error = " << e << endl;
  return e;
}


double
ScoreTrials::c1AvgAngleBetweenTangents(MarkRef promptMark, MarkRef drawnMark, int start, int end)
{
  if (drawnMark->getNumSamples() <= 2) {
    return -1;
  }

  // foreach point along drawn curve
  //   find abs val of diff between tangent on drawn curve and tangent
  //   at closest point on the prompt curve

  double sum = 0.0;
  if (start == -1) {
    start = 0;
  }
  if (end == -1) {
    end = drawnMark->getNumSamples();
  }

  for (int i=start;i<end;i++) {
    Vector3 dtangent;
    if (i == 0) {
      dtangent = (drawnMark->getSamplePosition(1) - drawnMark->getSamplePosition(0)).unit();
    }
    else if (i == drawnMark->getNumSamples()-1) {
      dtangent = (drawnMark->getSamplePosition(i) - drawnMark->getSamplePosition(i-1)).unit();
    }
    else {
      dtangent = (drawnMark->getSamplePosition(i+1) - drawnMark->getSamplePosition(i-1)).unit();
    }

    // c = index of closest point on the prompt
    int c;
    double d = promptMark->distanceToMark(drawnMark->getSamplePosition(i), c);
    
    Vector3 ptangent;
    if (c == 0) {
      ptangent = (promptMark->getSamplePosition(1) - promptMark->getSamplePosition(0)).unit();
    }
    else if (c == promptMark->getNumSamples()-1) {
      ptangent = (promptMark->getSamplePosition(c) - promptMark->getSamplePosition(c-1)).unit();
    }
    else {
      ptangent = (promptMark->getSamplePosition(c+1) - promptMark->getSamplePosition(c-1)).unit();
    }
    
    double dot = ptangent.dot(dtangent);
    //cout << "d = " << dot << endl;
    double angle = toDegrees(aCos(dot));
    //cout << "a = " << angle << endl;
    sum += fabs(angle);
  }
 
  double avg = sum / (double)drawnMark->getNumSamples();
  //cout << "C1: Avg Angle Between Tangents = " << avg << endl;
  return avg;
}



double
ScoreTrials::c2AvgAngleBetween2ndDeriv(MarkRef promptMark, MarkRef drawnMark, int start, int end)
{
  if (drawnMark->getNumSamples() <= 2) {
    return -1;
  }

  double sum = 0.0;
  if (start == -1) {
    start = 0;
  }
  if (end == -1) {
    end = drawnMark->getNumSamples();
  }

  for (int i=start;i<end;i++) {

    Vector3 dtangent = secondTangent(drawnMark, i);

    // c = index of closest point on the prompt
    int c;
    double d = promptMark->distanceToMark(drawnMark->getSamplePosition(i), c);
    
    Vector3 ptangent = secondTangent(promptMark, c);
    
    double dot = ptangent.dot(dtangent);
    //cout << "d = " << dot << endl;
    double angle = toDegrees(aCos(dot));
    //cout << "a = " << angle << endl;
    sum += fabs(angle);
  }
 
  double avg = sum / (double)drawnMark->getNumSamples();
  //cout << "C2: Avg Angle Between 2nd Deriv = " << avg << endl;
  return avg;
}


double
ScoreTrials::avgDiffInWidth(MarkRef promptMark, MarkRef drawnMark)
{
  if (drawnMark->getNumSamples() <= 2) {
    return -1;
  }

  double sum = 0.0;
  for (int i=0;i<drawnMark->getNumSamples();i++) {
    // c = index of closest point on the prompt
    int c;
    double d = promptMark->distanceToMark(drawnMark->getSamplePosition(i), c);
    
    sum += fabs(promptMark->getBrushState(c)->width - drawnMark->getBrushState(i)->width);
  }
  double avg = sum / (double)drawnMark->getNumSamples();
  //cout << "Avg Diff in Width = " << avg << endl;
  return avg;
}


double
ScoreTrials::avgDiffInDerivOfWidth(MarkRef promptMark, MarkRef drawnMark)
{
  if (drawnMark->getNumSamples() <= 2) {
    return -1;
  }

  Array<double> pw, dw;
  dw.append(0);
  for (int i=1;i<drawnMark->getNumSamples();i++) {
    double deltaW = drawnMark->getBrushState(i)->width - drawnMark->getBrushState(i-1)->width;
    dw.append(deltaW);
  }
  pw.append(0);
  for (int i=1;i<promptMark->getNumSamples();i++) {
    double deltaW = promptMark->getBrushState(i)->width - promptMark->getBrushState(i-1)->width;
    pw.append(deltaW);
  }

  double sum = 0.0;
  for (int i=0;i<drawnMark->getNumSamples();i++) {
    // c = index of closest point on the prompt
    int c;
    double d = promptMark->distanceToMark(drawnMark->getSamplePosition(i), c);
    
    sum += fabs(pw[c] - dw[i]);
  }
  double avg = sum / (double)drawnMark->getNumSamples();
  //cout << "Avg Diff in Derivative of Width = " << avg << endl;
  return avg;
}


double
ScoreTrials::maxDistError(MarkRef promptMark, MarkRef drawnMark)
{
  double max=0.0;
  for (int i=0;i<drawnMark->getNumSamples();i++) {
    double d = promptMark->distanceToMark(drawnMark->getSamplePosition(i));
    if (d > max) {
      max = d;
    }
  }
  max = max * 304.8;
  //cout << "Max Distance Error = " << max << endl;
  return max;
}


double
ScoreTrials::totalAngleBetweenAdjTangents(MarkRef mark, int start, int end)
{
  if (mark->getNumSamples() <= 2) {
    return -1;
  }

  double sum = 0.0;
  if (start == -1) {
    start = 0;
  }
  if (end == -1) {
    end = mark->getNumSamples();
  }
  for (int i=start+1;i<end;i++) {
    Vector3 tanA = firstTangent(mark, i-1);
    Vector3 tanB = firstTangent(mark, i);

    double dot = tanA.dot(tanB);
    //cout << "d = " << dot << endl;
    double angle = toDegrees(aCos(dot));
    //cout << "a = " << angle << endl;
    sum += fabs(angle);
  }

  return sum;
}

void
ScoreTrials::changeInXYZ(MarkRef mark, Vector3 &total, Vector3 &pos, Vector3 &neg, int start, int end)
{
  if (mark->getNumSamples() <= 2) {
    return;
  }

  if (start == -1) {
    start = 0;
  }
  if (end == -1) {
    end = mark->getNumSamples();
  }

  for (int i=start+1;i<end;i++) {
    Vector3 diff = mark->getSamplePosition(i) - mark->getSamplePosition(i-1);
    for (int d=0;d<3;d++) {
      if (diff[d] > 0) {
        pos[d] += diff[d];
      }
      else if (diff[d] < 0) {
        neg[d] += -diff[d];
      }
      total[d] += fabs(diff[d]);
    }
  }
}


Vector3
ScoreTrials::totalAngleBetweenAdjTangentsInXYZPlanes(MarkRef mark, int start, int end)
{
  if (mark->getNumSamples() <= 2) {
    return Vector3::zero();
  }

  Vector3 sum;
  if (start == -1) {
    start = 0;
  }
  if (end == -1) {
    end = mark->getNumSamples();
  }
  for (int i=start+1;i<end;i++) {
    Vector3 tanA = firstTangent(mark, i-1);
    Vector3 tanB = firstTangent(mark, i);

    Vector3 tanAYZ = Vector3(0,tanA[1],tanA[2]).unit();
    Vector3 tanBYZ = Vector3(0,tanB[1],tanB[2]).unit();
    Vector3 tanAXZ = Vector3(tanA[0],0,tanA[2]).unit();
    Vector3 tanBXZ = Vector3(tanB[0],0,tanB[2]).unit();
    Vector3 tanAXY = Vector3(tanA[0],tanA[1],0).unit();
    Vector3 tanBXY = Vector3(tanB[0],tanB[1],0).unit();

    double aYZ = fabs(toDegrees(aCos(tanAYZ.dot(tanBYZ))));
    double aXZ = fabs(toDegrees(aCos(tanAXZ.dot(tanBXZ))));
    double aXY = fabs(toDegrees(aCos(tanAXY.dot(tanBXY))));

    sum = sum + Vector3(aYZ, aXZ, aXY);
  }

  return sum;
}

Vector3
ScoreTrials::totalAngleBetweenAdjTangentsInXYZByDrawingDir(MarkRef mark, int start, int end)
{
  if (mark->getNumSamples() <= 2) {
    return Vector3::zero();
  }

  Vector3 sum;
  if (start == -1) {
    start = 0;
  }
  if (end == -1) {
    end = mark->getNumSamples();
  }

  for (int i=start+1;i<end;i++) {
    Vector3 tanA = firstTangent(mark, i-1);
    Vector3 tanB = firstTangent(mark, i);
    double a = fabs(toDegrees(aCos(tanA.dot(tanB))));
    //Vector3 avgTan = (0.5*(tanA + tanB)).unit();
    Vector3 avgTan = tanB;

    sum[0] += a*fabs(avgTan[0]);
    sum[1] += a*fabs(avgTan[1]);
    sum[2] += a*fabs(avgTan[2]);

    //cout << "a = " << a << " avgTan=" << avgTan << " sum=" << sum << endl;
  }

  return sum;
}


void
ScoreTrials::curvature(MarkRef mark, Array<double> &curvatures, Array<Vector3> &centerOfCurvatures)
{
  /**
     see: http://www-math.mit.edu/18.013A/HTML/chapter15/section04.html

     K =  | (a(v*v) - v(a*v)) / (v*v) |
          -----------------------------
                     v*v    
  **/

  // assume the mark has already been resampled, otherwise may need to
  // add delta t into calculation.

  Array<Vector3> v;
  Array<Vector3> a;
  for (int i=0;i<mark->getNumSamples()-1;i++) {
    v.append(mark->getSamplePosition(i+1) - mark->getSamplePosition(i));
    if (v.size() < 2) {
      a.append(Vector3::zero());
    }
    else {
      a.append(v[v.size()-1] - v[v.size()-2]);
    }
  }

  for (int i=0;i<v.size();i++) {
    double vdotv = v[i].dot(v[i]);
    double adotv = a[i].dot(v[i]);
    
    // curvature
    double K = ( ((vdotv*a[i] - adotv*v[i]) / vdotv).length() ) / vdotv;
    curvatures.append(K);
    
    // normal vector
    Vector3 aunit = a[i].unit();
    Vector3 vunit = v[i].unit();
    Vector3 N = (aunit - aunit.dot(vunit)*vunit).unit();

    // radius of curvature
    double r = 1/K;

    // center of curvature
    Vector3 q = mark->getSamplePosition(i) + r*N;
    centerOfCurvatures.append(q);
  }

}
