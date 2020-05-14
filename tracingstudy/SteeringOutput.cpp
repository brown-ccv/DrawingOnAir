
#include "SteeringStudy.H"
#include "ScoreTrials.H"



/** 
    DatasetType:

    1 - Piecewise: For each trial, skip the first 1cm of the line.
    Then, randomly skip ahead between 0 and "SegmentLength" cm.  Then,
    output consecutive segments of equal length.

    2 - Random: For each trial, output 2 non-overlapping random
    samples from the curve - 1cm at the front and 1cm at the end.
    Only take one sample, if the first sample leaves no room for
    another non-overlapping sample.  No samples smaller than 1cm.

    3 - Wholelines: Trim off the first 1cm and the last 1cm, then
    output the rest of the data.
**/
void 
SteeringStudy::printDatasetForSteeringAnalysis(int datasetType)
{
  // Load up and save the prompts
  std::string promptMarksFile = "computed-marks.3DArt";
  BinaryInput bi(promptMarksFile, G3D_LITTLE_ENDIAN, false);
  Color3 bgColor;
  ArtworkIO::deserialize(bi, _artwork, bgColor, NULL, NULL, NULL);
  Array<MarkRef> promptMarks = _artwork->getMarks();
  _artwork->clear();


  // Loop through all subjects
  int startID = ConfigVal("StartID", 1, false);
  int stopID = ConfigVal("StopID", 12, false);
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

      if ((ConfigVal("Condition",0) != t->_data.conditionID) &&
          (ConfigVal("Condition","0") != "ALL")) {
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
        
        int pstart, pend, dstart, dend;

        if (datasetType == 1) {
          // piecewise
          int segstart = 33 + iRandom(0,66);
          int segsize = ConfigVal("Piecewise_StepSize", 66, false);
          while (segstart + segsize < resampPrompt->getNumSamples() - 33) {
            pstart = segstart;
            pend = segstart + segsize;
            double dist1 = resampDrawn->distanceToMark(resampPrompt->getSamplePosition(pstart), dstart);
            double dist2 = resampDrawn->distanceToMark(resampPrompt->getSamplePosition(pend), dend);          
            outputDataForSegment(resampPrompt, pstart, pend, resampDrawn, dstart, dend, 
                                 drawingTimes, curvatures, curvCenters, subjectID, t);
            segstart += segsize;
          }
        }
        else if (datasetType == 2) {
          // random
          
          // randomly divide the segment into two
          int div = iRandom(33, resampPrompt->getNumSamples()-10);
          if (div > 98) {
            int len = iRandom(33, div);
            pstart = iRandom(33, div-len);
            pend = pstart + len;
            double dist1 = resampDrawn->distanceToMark(resampPrompt->getSamplePosition(pstart), dstart);
            double dist2 = resampDrawn->distanceToMark(resampPrompt->getSamplePosition(pend), dend);          
            outputDataForSegment(resampPrompt, pstart, pend, resampDrawn, dstart, dend, 
                                 drawingTimes, curvatures, curvCenters, subjectID, t);
          }
          int lastsp = resampPrompt->getNumSamples();
          if (div < lastsp - 98) {
            int len = iRandom(div, lastsp-33);
            pstart = iRandom(div, lastsp-len-33);
            pend = pstart + len;
            double dist1 = resampDrawn->distanceToMark(resampPrompt->getSamplePosition(pstart), dstart);
            double dist2 = resampDrawn->distanceToMark(resampPrompt->getSamplePosition(pend), dend);          
            outputDataForSegment(resampPrompt, pstart, pend, resampDrawn, dstart, dend, 
                                 drawingTimes, curvatures, curvCenters, subjectID, t);
          }
        }
        else if (datasetType == 3) {
          // wholelines
          pstart = 33;
          pend = resampPrompt->getNumSamples()-34;
          double dist1 = resampDrawn->distanceToMark(resampPrompt->getSamplePosition(pstart), dstart);
          double dist2 = resampDrawn->distanceToMark(resampPrompt->getSamplePosition(pend), dend);          
          outputDataForSegment(resampPrompt, pstart, pend, resampDrawn, dstart, dend, 
                               drawingTimes, curvatures, curvCenters, subjectID, t);
        }
      }
      i++;
    }
  }
  exit(0);
}



void
SteeringStudy::outputDataForSegment(MarkRef pMark, int pstart, int pend,
                                    MarkRef dMark, int dstart, int dend,
                                    Array<double> drawingTimes,
                                    Array<double> curvatures, Array<Vector3> curvCenters,
                                    std::string subjectID, TracingTrialRef t)
{
  
  Vector3 avgPosD;
  Vector3 avgDirD;
  for (int j=dstart;j<dend;j++) {
    avgPosD += dMark->getSamplePosition(j);
    avgDirD += ScoreTrials::firstTangent(dMark, j);
  }
  avgPosD /= (double)(dend-dstart);
  avgDirD /= (double)(dend-dstart);
  
  Vector3 avgPosP;
  Vector3 avgDirP;
  for (int j=pstart;j<pend;j++) {
    avgPosP += pMark->getSamplePosition(j);
    avgDirP += ScoreTrials::firstTangent(pMark, j);
  }
  avgPosP /= (double)(pend - pstart);
  avgDirP /= (double)(pend - pstart);

  double dtime;
  if (dend >= drawingTimes.size()) {
    dtime = drawingTimes.last() - drawingTimes[dstart];
  }
  else {
    dtime = drawingTimes[dend] - drawingTimes[dstart];
  }
  double tstart = drawingTimes[dstart];
  
  double l = pMark->getArcLength(pstart, pend);
  double lstart = pMark->getArcLength(0, pstart);
  Vector3 lXYZ;
  Vector3 lposXYZ;
  Vector3 lnegXYZ;
  ScoreTrials::changeInXYZ(pMark, lXYZ, lposXYZ, lnegXYZ, pstart, pend);
  double  totalAD = ScoreTrials::totalAngleBetweenAdjTangents(pMark, pstart, pend);
  Vector3 totalADxyz = ScoreTrials::totalAngleBetweenAdjTangentsInXYZByDrawingDir(pMark, pstart, pend);
  double c0distD = ScoreTrials::c0AvgDistToPromptAlongDrawn(pMark, dMark, dstart, dend);
  double c0distP = ScoreTrials::c0AvgDistToDrawnAlongPrompt(pMark, dMark, pstart, pend);
  double c0 = (c0distD + c0distP) / 2.0;
  double c1 = ScoreTrials::c1AvgAngleBetweenTangents(pMark, dMark, dstart, dend);
  double c2 = ScoreTrials::c2AvgAngleBetween2ndDeriv(pMark, dMark, dstart, dend);
  Vector3 adInPlanes = ScoreTrials::totalAngleBetweenAdjTangentsInXYZPlanes(pMark, pstart, pend);        

  Array<double> Kraised;
  for (int i=0;i<12;i++) {
    Kraised.append(0.0);
  }
  Vector3 avgDirRadCurv;
  int cStart = iMax(pstart, 1);
  int cEnd = iMin(pend, pMark->getNumSamples()-2);
  int numC=0;
  for (int c=cStart;c<cEnd;c++) {
    double K = curvatures[c];
    Vector3 q = curvCenters[c];
    if ((isFinite(K)) && (q.isFinite()) && (!q.isZero())) {
      double Kr = pow(K, 0.1);
      if (isFinite(Kr)) 
        Kraised[0] += Kr;
      Kr = pow(K, 0.2);
      if (isFinite(Kr))
        Kraised[1] += Kr;
      Kr = pow(K, 0.3);
      if (isFinite(Kr))
        Kraised[2] += Kr;
      Kr = pow(K, 0.4);
      if (isFinite(Kr))
        Kraised[3] += Kr;
      Kr = pow(K, 0.5);
      if (isFinite(Kr))
        Kraised[4] += Kr;
      Kr = pow(K, 0.6);
      if (isFinite(Kr))
        Kraised[5] += Kr;
      Kr = pow(K, 0.7);
      if (isFinite(Kr))
        Kraised[6] += Kr;
      Kr = pow(K, 0.8);
      if (isFinite(Kr))
        Kraised[7] += Kr;
      Kr = pow(K, 0.9);
      if (isFinite(Kr))
        Kraised[8] += Kr;
      Kr = K;
      if (isFinite(Kr))
        Kraised[9] += Kr;
      Kr = pow(K, 1.5);
      if (isFinite(Kr))
        Kraised[10] += Kr;
      Kr = pow(K, 2.0);
      if (isFinite(Kr))
        Kraised[11] += Kr;
      
      avgDirRadCurv += (q - pMark->getSamplePosition(c)).unit();
      numC++;
    }
  }
  Array<double> AvgKraised;
  for (int c=0;c<Kraised.size();c++) {
    AvgKraised.append(Kraised[c] / numC);
  }
    avgDirRadCurv /= numC;
  avgDirRadCurv.unit();
  if (!avgDirRadCurv.isFinite()) {
    avgDirRadCurv = Vector3::zero();
  }
        
  /*** Output format:

  id cond prompt tdraw tstart c0 c1 c2 l lstart lx ly lz lposx lposy lposz lnegx lnegy lnegz
  K.1 K.2 K.3 K.4 K.5 K.6 K.7 K.8 K.9 K K1.5 K2 
  AvgK.1 AvgK.2 AvgK.3 AvgK.4 AvgK.5 AvgK.6 AvgK.7 AvgK.8 AvgK.9 AvgK AvgK1.5 AvgK2 
  AvgDirRx AvgDirRy AvgDirRz
  ad adx ady adz adyz adxz adxy 
  posdx posdy posdz dirdx dirdy dirdz pospx pospy pospz dirpx dirpy dirpz
  ****/
        
  cout.setf(ios::fixed,ios::floatfield);

  cout << subjectID << ", " << t->_data.conditionID << ", " << t->_data.promptIndex << ", " <<
    dtime << ", " << tstart << ", " <<
    c0 << ", " << c1 << ", " << c2 << ", " << 
    l << ", " << lstart << ", " << lXYZ[0] << ", " << lXYZ[1] << ", " << lXYZ[2] << ", " <<
    lposXYZ[0] << ", " << lposXYZ[1] << ", " << lposXYZ[2] << ", " <<
    lnegXYZ[0] << ", " << lnegXYZ[1] << ", " << lnegXYZ[2] << ", ";

  for (int i=0;i<Kraised.size();i++) {
    cout << Kraised[i] << ", ";
  }
  for (int i=0;i<AvgKraised.size();i++) {
    cout << AvgKraised[i] << ", ";
  }
  
  cout << avgDirRadCurv[0] << ", " << avgDirRadCurv[1] << ", " << avgDirRadCurv[2] << ", " <<
    totalAD << ", " << totalADxyz[0] << ", " << totalADxyz[1] << ", " << totalADxyz[2] << ", " <<
    adInPlanes[0] << ", " << adInPlanes[1] << ", " << adInPlanes[2] << ", " <<

    avgPosD[0] << ", " << avgPosD[1] << ", " << avgPosD[2] << ", " <<
    avgDirD[0] << ", " << avgDirD[1] << ", " << avgDirD[2] << ", " <<
    avgPosP[0] << ", " << avgPosP[1] << ", " << avgPosP[2] << ", " <<
    avgDirP[0] << ", " << avgDirP[1] << ", " << avgDirP[2] << endl;
}








