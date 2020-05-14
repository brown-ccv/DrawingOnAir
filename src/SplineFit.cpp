
#ifdef USE_SPLINES

#include "SplineFit.H"
#include "Wm3BSplineFit.h"
#include "Wm3Vector3.h"
using namespace G3D;
namespace DrawOnAir {


/**  Spline Fitting Algorithm taken from: 

3D Sketch Stroke Segmentation and Fitting in Virtual Reality
Michele Fiorentino, Giuseppe Monno, Pietro Alexander Renzulli, Antonio E. Uva

    1. use their equations based on angle & speed to find major angle
       differences, these are primary segmentation points.
    2. some other thresholds based on speed and angle tell you whether
       you have a secondary segmentation point.
    3. then, there's a detail level that controls then number of
       control points (and therefore the number of individual
       B-splines w/ 4 cp's each) for each section of curve between the
       segmentation points.
    4. so, given the number of splines to use for each segment and the
       list of raw input points for each segment, compute
       interpolating B-splines using least squares approx.
**/
MarkRef
SplineFit(MarkRef m, double resampleStepSize, 
          Array<Vector3> &controlPoints, Array<Vector3> &segmentationPoints, 
          TriStripModelRef triStripModel, GfxMgrRef gfxMgr)
{
  Array<Vector3> samples = m->getSamplePositionsArray();
  Array<BrushStateRef> brushStates = m->getBrushStatesArray();

  MarkRef newMark = NULL;
  if (m->markDescription() == "TubeMark") {
    newMark = new TubeMark(m->getName(), gfxMgr, triStripModel);
  }
  else {
    return NULL;
  }
  //MarkRef newMark = m->copy();
  //m->trimEnd(0);

  // Calculate avg speed and speed at each sample.  For us, samples
  // are evenly spaced in time, so speed should be proportional to how
  // far apart the samples are from each other.  Actually, this only
  // applies to the freehand techniques.
  double avgSpeed = 0.0;
  Array<double> speed;
  speed.append(0);
  for (int i=1;i<samples.size();i++) {
    double s = (samples[i] - samples[i-1]).length();
    speed.append(s);
    avgSpeed += s;
  }
  avgSpeed /= speed.size();

  // Calculate alpha, directional deviation for each sample
  Array<double> alpha;
  for (int i=1;i<samples.size()-1;i++) {
    // m is the alpha adaptive support length
    // note: they clamped m at 10, is 10 a good value for us?
    int m = iClamp(iRound(2.0*avgSpeed/speed[i] + 0.5), 1, 10);
    cout << "m = " << m << endl;

    Vector3 v1 = (samples[i] - samples[iClamp(i-m, 0, samples.size()-1)]).unit();
    Vector3 v2 = (samples[iClamp(i+m, 0, samples.size()-1)] - samples[i]).unit();
    double angle = aCos(v1.dot(v2));
    alpha.append(angle);
    cout << "alpha = " << angle << endl;
  }

  cout << "num samples = " << samples.size() << endl;
  cout << "num alphas = " << alpha.size() << endl;

  // Once you have your alphas you need to pick a cutoff value that
  // makes sense.  I guess print them out to check this.
  double primaryCutoff = ConfigVal("PrimaryCutoff", 0.1);

  // Find the primary segmentation points -- these occur whenever you
  // see an alpha that crosses the threshold unless the last alpha was
  // already over the threshold.  In that case, take the biggest alpha
  // in the series that is over the threshold
  Array<int> primarySegPoints;
  // Note indexing in primarySegPoints matches that of the samples
  // array.  The alpha array indices are off by 1 and alphas does not
  // include an entry for the first and last sample, which we want to
  // add automatically to the primarySegPoints array.
  primarySegPoints.append(0);
  bool alreadyOver = false;
  for (int i=0;i<alpha.size();i++) {
    if (alpha[i] > primaryCutoff) {
      if (alreadyOver) {
        if (alpha[i] > alpha[primarySegPoints.last()]) {
          primarySegPoints[primarySegPoints.size()-1] = i+1;
        }
      }
      else {
        primarySegPoints.append(i+1);
        alreadyOver = true;
      }
    }
    else {
      alreadyOver = false;
    }
  }
  primarySegPoints.append(alpha.size());

  // Keep running into this issue, at least with mouse input on the
  // desktop, where the number of samples inside the segment is less
  // than the requested number of ctrl points.  The min number of ctrl
  // points is 4, so make sure there are at least 4 samples per
  // segment, otherwise remove one of the seg points.
  int minSamplesPerSeg = 8;
  Array<int> newSegPoints;
  newSegPoints.append(primarySegPoints[0]);
  for (int i=1;i<primarySegPoints.size();i++) {
    if (primarySegPoints[i] - primarySegPoints[i-1] > minSamplesPerSeg) {
      newSegPoints.append(primarySegPoints[i]);
    }
    // else, skip this seg point.. unless it is the very last one
    else if (i==primarySegPoints.size()) {
      newSegPoints.resize(newSegPoints.size()-1);
    }
  }
  primarySegPoints = newSegPoints;


  for (int i=0;i<primarySegPoints.size();i++) {
    segmentationPoints.append(samples[primarySegPoints[i]]);
  }
  cout << "Found " << primarySegPoints.size() << " primary segmentation points." << endl;


  // TODO: Find secondary segmentation points.  This process seems a
  // bit rediculous since it contains about 10 different constants
  // that apparently need to be tweaked on a per-system basis, so
  // skipping it for now.


  // Once you have all the segments, figure out how many control
  // points (and therefore how many individual B-splines) to use for
  // each segment and then call a fuction to create the interpolated
  // B-splines from the raw data.
  for (int i=1;i<primarySegPoints.size();i++) {
    int start = primarySegPoints[i-1];
    int stop = primarySegPoints[i];

    Array<Vector3> segSamples;
    double chordLength = (samples[stop] - samples[start]).length();
    double arcLength = 0.0;
    for (int s=start;s<stop;s++) {
      arcLength += (samples[s+1] - samples[s]).length();
      segSamples.append(samples[s]);
    }
    segSamples.append(samples[stop]);

    // Li is a measure for the linearity of the segment
    double Li = (1 - (chordLength / arcLength)) / avgSpeed;
    cout << "Li = " << Li << endl;
    
    // Not sure where this constant 70 comes from.  It probably isn't
    // right for us.
    double ctrlPtWeightFactor = ConfigVal("ControlPtWeightFactor", 70.0);
    int numCP = 5 + iRound(ctrlPtWeightFactor*Li);
    if (numCP > segSamples.size()) {
      numCP = segSamples.size();
    }

    cout << "num segment samples = " << segSamples.size() << endl;
    cout << "num CP = " << numCP << endl;
    
    // Call interpolating B-spline routine with numCP and segSamples,
    // save result in controlPoints array.
    Wm3::Vector3f *samples = new Wm3::Vector3f[segSamples.size()];
    for (int i=0;i<segSamples.size();i++) {
      samples[i].X() = segSamples[i][0];
      samples[i].Y() = segSamples[i][1];
      samples[i].Z() = segSamples[i][2];
    }
    Wm3::BSplineFitf *bsf = new Wm3::BSplineFitf(3, segSamples.size(), 
                                                 (const float*)samples, 3, numCP);

    //
    int numSamples = iRound(arcLength / resampleStepSize);
    float fMult = 1.0f/(numSamples - 1);
    for (int i=0;i<numSamples;i++) {
      float fT = fMult*i;
      Wm3::Vector3f pos;
      //cout << "ft=" << fT << endl;
      bsf->GetPosition(G3D::clamp(fT,0.0f,1.0f), (float*)&pos);
      int closestOrigBS = iClamp(lerp(start, stop, fT), start, stop);
      //BrushStateRef newBS = brushStates[start]->lerp(brushStates[stop], fT);
      BrushStateRef newBS = brushStates[closestOrigBS]->copy();
      Vector3 offset = ConfigVal("SplineFit_Offset", Vector3::zero(), false);
      newBS->frameInRoomSpace.translation = Vector3(pos[0], pos[1], pos[2]) + offset;
      newBS->colorSwatchCoord = 0.25;
      newMark->addSample(newBS);
      //cout << i << ": " << pos[0] << " " << pos[1] << " " << pos[2] << endl;
    } 


    delete bsf;
    delete [] samples;
  }

  return newMark;
}


} // end namespace

#endif

