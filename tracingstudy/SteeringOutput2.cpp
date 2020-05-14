#include "SteeringStudy.H"
#include "ScoreTrials.H"

/** 

    This version of the routines finds mean values for all subjects
    and per subject, currently only works for piecewise data.

    data[subject][condition][prompt][sample]
       avg curvature for sample
       avg pos for sample
       avg dir for sample
       mean pos error
       mean dir error
       mean vel (equal length segments, so drawing time is ok)

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

class SegmentData
{
public:
  double curv;
  Vector3 pos;
  Vector3 dir;

  // inititally sum out these errors, then when finished looping
  // through data, divide by counter to get mean values per subject
  // per condition.
  int    counter;
  double poserr;
  double direrr;
  double tdraw;
};


void 
SteeringStudy::printDatasetWithMeansForSteeringAnalysis()
{
  // Load up and save the prompts
  std::string promptMarksFile = "computed-marks.3DArt";
  BinaryInput bi(promptMarksFile, G3D_LITTLE_ENDIAN, false);
  Color3 bgColor;
  ArtworkIO::deserialize(bi, _artwork, bgColor, NULL, NULL, NULL);
  Array<MarkRef> promptMarks = _artwork->getMarks();
  _artwork->clear();

  // Size data array appropriately for all dimensions except the number of samples:
  // data[subject][condition][prompt][sample]
  Array< Array< Array< Array< SegmentData > > > > data;
  // data for all subjects combined
  Array< Array< Array< SegmentData > > > dataallsub;


  for (int s=0;s<12;s++) {
    Array< Array< Array<SegmentData> > > sc;
    for (int c=0;c<4;c++) {
      Array< Array<SegmentData> > sp;
      for (int p=0;p<5;p++) {
        Array<SegmentData> samples;
        sp.append(samples);
      }
      sc.append(sp);
    }
    data.append(sc);
    if (s==0) {
      dataallsub = sc;
    }
  }
  
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

      /**
      if ((ConfigVal("Condition","ALL", false) != "ALL") && 
          (ConfigVal("Condition","0") != t->_data.conditionID)) {
        skip = true;
      }
      **/


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

        // smooth curvature values with a triangle filter, could use a gaussian or whatever too.
        // width of the ramp to either side of the sample
        /***
        int halfwidth = 49;  // 49 = 1.5cm
        double sigma = ConfigVal("GaussianSigma", 0.33, false);
        Array<double> newCurvatures;
        for (int c=0;c<curvatures.size();c++) {
          double sum = 0.0;
          double weight = 0.0;
          // left tail of gaussian
          for (int j=halfwidth;j>=0;j--) {
            int c2 = c-j;
            if (c2 >= 1) {
              double t = (double)(halfwidth-j)/(double)halfwidth;
              double G = exp(-0.5 * (t*t)/(sigma*sigma)) / sigma*sqrtf(twoPi());
              weight += G;
              sum += G * curvatures[c2];
              //cout << j << " " << weight << endl;
            }
          }
          //cout << "midpt" <<endl;
          // right tail of gaussian
          for (int j=1;j<halfwidth;j++) {
            int c2 = c+j;
            if (c2 <= curvatures.size()-2) {
              double t = (double)(halfwidth-j)/(double)halfwidth;
              double G = exp(-0.5 * (t*t)/(sigma*sigma)) / sigma*sqrtf(twoPi());
              weight += G;
              sum += G * curvatures[c2];
              //cout << j << " " << weight << endl;
            }
          }
         
          double weightedCurv = 0.0;
          if (weight != 0.0)
            weightedCurv = sum / weight;
          newCurvatures.append(weightedCurv);
        }
        curvatures = newCurvatures;
        ***/        

        int pstart, pend, dstart, dend;

        // skip the first and last 2.5 cm 
        int endoffset = ConfigVal("EndOffset", 82, false);
        if (ConfigVal("PrintWholelines",0,false)) {
          // for whole lines, really try to get the whole line, but we'll
          // just squeak in about .5cm to avoid any strangness near the ends.
          int endoffset = 16;
        }

        // 49 steps of .001 feet is approximately 1.5cm
        // 33 steps of .001 feet is approximately 1.0cm
        // 16 steps of .001 feet is approximately 0.5cm
        int segsize = ConfigVal("Piecewise_StepSize", 49, false);
        int segcounter = 0;
        int segstart = endoffset;
        while (segstart + segsize < resampPrompt->getNumSamples() - endoffset) {
          pstart = segstart;
          pend = segstart + segsize;
          double dist1 = resampDrawn->distanceToMark(resampPrompt->getSamplePosition(pstart), dstart);
          double dist2 = resampDrawn->distanceToMark(resampPrompt->getSamplePosition(pend), dend);     
          
          // avg position of the segment in space
          Vector3 pos = 0.5*(resampPrompt->getSamplePosition(pstart) + 
                             resampPrompt->getSamplePosition(pend));
          // avg drawing direction required for the segment
          Vector3 dir = 0.5*(ScoreTrials::firstTangent(resampPrompt, pstart) + 
                             ScoreTrials::firstTangent(resampPrompt, pend));
          
          // avg curvature along the segment
          int cStart = iMax(pstart, 1);
          int cEnd = iMin(pend, resampPrompt->getNumSamples()-2);
          double totalC=0.0;
          int numC=0;
          for (int c=cStart;c<cEnd;c++) {
            double K = curvatures[c];
            if (isFinite(K)) {
              totalC += K;
              numC++;
            }
          }
          double curv = totalC / (double)numC;
          
          // drawing time
          double dtime;
          if (dend >= drawingTimes.size()) {
            dtime = drawingTimes.last() - drawingTimes[dstart];
          }
          else {
            dtime = drawingTimes[dend] - drawingTimes[dstart];
          }
          // don't have timing info for conditions 0 and 1, so to
          // avoid confusion, just output 0 for the time.
          if ((t->_data.conditionID == 0) || (t->_data.conditionID == 1)) {
            dtime = 0.0;
          }

          
          // positional error
          double c0distD = ScoreTrials::c0AvgDistToPromptAlongDrawn(resampPrompt, resampDrawn, dstart, dend);
          double c0distP = ScoreTrials::c0AvgDistToDrawnAlongPrompt(resampPrompt, resampDrawn, pstart, pend);
          double c0 = (c0distD + c0distP) / 2.0;
          
          // directional error
          double c1 = ScoreTrials::c1AvgAngleBetweenTangents(resampPrompt, resampDrawn, dstart, dend);
          
          // save in data array
          if (data[iSubjectID-1][t->_data.conditionID][t->_data.promptIndex].size() <= segcounter) {
            SegmentData segdata;
            segdata.curv = curv;
            segdata.pos = pos;
            segdata.dir = dir;
            segdata.counter = 1;
            segdata.poserr = c0;
            segdata.direrr = c1;
            segdata.tdraw = dtime;
            data[iSubjectID-1][t->_data.conditionID][t->_data.promptIndex].append(segdata);
          }
          else {
            data[iSubjectID-1][t->_data.conditionID][t->_data.promptIndex][segcounter].curv += curv;
            data[iSubjectID-1][t->_data.conditionID][t->_data.promptIndex][segcounter].pos += pos;
            data[iSubjectID-1][t->_data.conditionID][t->_data.promptIndex][segcounter].dir += dir;
            data[iSubjectID-1][t->_data.conditionID][t->_data.promptIndex][segcounter].counter++;
            data[iSubjectID-1][t->_data.conditionID][t->_data.promptIndex][segcounter].poserr += c0;
            data[iSubjectID-1][t->_data.conditionID][t->_data.promptIndex][segcounter].direrr += c1;
            data[iSubjectID-1][t->_data.conditionID][t->_data.promptIndex][segcounter].tdraw += dtime;
          }
          
          // save in dataallsub array
          if (dataallsub[t->_data.conditionID][t->_data.promptIndex].size() <= segcounter) {
            SegmentData segdata;
            segdata.curv = curv;
            segdata.pos = pos;
            segdata.dir = dir;
            segdata.counter = 1;
            segdata.poserr = c0;
            segdata.direrr = c1;
            segdata.tdraw = dtime;
            dataallsub[t->_data.conditionID][t->_data.promptIndex].append(segdata);
          }
          else {
            dataallsub[t->_data.conditionID][t->_data.promptIndex][segcounter].curv += curv;
            dataallsub[t->_data.conditionID][t->_data.promptIndex][segcounter].pos += pos;
            dataallsub[t->_data.conditionID][t->_data.promptIndex][segcounter].dir += dir;
            dataallsub[t->_data.conditionID][t->_data.promptIndex][segcounter].counter++;
            dataallsub[t->_data.conditionID][t->_data.promptIndex][segcounter].poserr += c0;
            dataallsub[t->_data.conditionID][t->_data.promptIndex][segcounter].direrr += c1;
            dataallsub[t->_data.conditionID][t->_data.promptIndex][segcounter].tdraw += dtime;
          }
          
          segstart += segsize;
          segcounter++;
          //cout << segcounter << endl;
        }
      }
      i++;
    }
  }

  
  // find means of data and print
  for (int iSubjectID=startID;iSubjectID<=stopID;iSubjectID++) {
    for (int cond=0;cond<4;cond++) {
      for (int prompt=0;prompt<5;prompt++) {

        Array<double> tK;
        Array<double> tdyK;
        Array<double> tdzK;
        for (double p=0.1;p<=1.2;p+=0.1) {
          tK.append(0.0);
          tdyK.append(0.0);
          tdzK.append(0.0);
        }
       
        SegmentData meanForPrompt;
        meanForPrompt.curv = 0.0;
        meanForPrompt.pos = Vector3::zero();
        meanForPrompt.dir = Vector3::zero();
        meanForPrompt.poserr = 0.0;
        meanForPrompt.direrr = 0.0;
        meanForPrompt.tdraw = 0.0;
      
        for (int s=0;s<data[iSubjectID-1][cond][prompt].size();s++) {
          int n = data[iSubjectID-1][cond][prompt][s].counter;
          data[iSubjectID-1][cond][prompt][s].curv /= n;
          data[iSubjectID-1][cond][prompt][s].pos /= n;
          data[iSubjectID-1][cond][prompt][s].dir /= n;
          data[iSubjectID-1][cond][prompt][s].poserr /= n;
          data[iSubjectID-1][cond][prompt][s].direrr /= n;
          data[iSubjectID-1][cond][prompt][s].tdraw /= n;

          meanForPrompt.curv += dataallsub[cond][prompt][s].curv;
          meanForPrompt.pos += dataallsub[cond][prompt][s].pos;
          meanForPrompt.dir += dataallsub[cond][prompt][s].dir;
          meanForPrompt.poserr += dataallsub[cond][prompt][s].poserr;
          meanForPrompt.direrr += dataallsub[cond][prompt][s].direrr;
          meanForPrompt.tdraw += dataallsub[cond][prompt][s].tdraw;
          
          int ip = 0;
          for (double p=0.1;p<=1.2;p+=0.1) {
            double K = dataallsub[cond][prompt][s].curv;
            double Kp = pow(K,p);
            tK[ip] += Kp;
            tdyK[ip] += fabs(dataallsub[cond][prompt][s].dir[1])*Kp;
            tdzK[ip] += -dataallsub[cond][prompt][s].dir[2]*Kp;
            ip++;
          }


          if (!ConfigVal("PrintWholelines",0,false)) {
            cout << iSubjectID-1 << ", " << cond << ", " << prompt << ", " << s << ", "
                 << data[iSubjectID-1][cond][prompt][s].curv << ", "
                 << data[iSubjectID-1][cond][prompt][s].pos[0] << ", "
                 << data[iSubjectID-1][cond][prompt][s].pos[1] << ", "
                 << data[iSubjectID-1][cond][prompt][s].pos[2] << ", "
                 << data[iSubjectID-1][cond][prompt][s].dir[0] << ", "
                 << data[iSubjectID-1][cond][prompt][s].dir[1] << ", "
                 << data[iSubjectID-1][cond][prompt][s].dir[2] << ", "
                 << data[iSubjectID-1][cond][prompt][s].poserr << ", "
                 << data[iSubjectID-1][cond][prompt][s].direrr << ", "
                 << data[iSubjectID-1][cond][prompt][s].tdraw << endl;
          }
        }
        if (ConfigVal("PrintWholelines",0,false)) {
        
          meanForPrompt.curv /= dataallsub[cond][prompt].size();
          meanForPrompt.pos /= dataallsub[cond][prompt].size();
          meanForPrompt.dir /= dataallsub[cond][prompt].size();
          meanForPrompt.poserr /= dataallsub[cond][prompt].size();
          meanForPrompt.direrr /= dataallsub[cond][prompt].size();
          //meanForPrompt.tdraw /= dataallsub[cond][prompt].size();
        
          cout << iSubjectID-1 << ", " << cond << ", " << prompt << ", "
               << meanForPrompt.poserr << ", "
               << meanForPrompt.direrr << ", "
               << meanForPrompt.tdraw << ", "
               // this is prompt arclen, change 0.049 if you change the prompt sampling.
               << 0.049*(double)dataallsub[cond][prompt].size();
        
          for (int p=0;p<tK.size();p++) {
            cout << ", " << tK[p] << ", " << tdyK[p] << ", " << tdzK[p];  
          }
          cout << endl;
          
        }
      }
    }
  }

  /***
  cout << "------------------------------------------------" << endl;
  

  // find means of data and print
  for (int cond=0;cond<4;cond++) {
    for (int prompt=0;prompt<5;prompt++) {
      
      Array<double> tK;
      Array<double> tdyK;
      Array<double> tdzK;
      for (double p=0.1;p<=1.2;p+=0.1) {
        tK.append(0.0);
        tdyK.append(0.0);
        tdzK.append(0.0);
      }
       
      SegmentData meanForPrompt;
      meanForPrompt.curv = 0.0;
      meanForPrompt.pos = Vector3::zero();
      meanForPrompt.dir = Vector3::zero();
      meanForPrompt.poserr = 0.0;
      meanForPrompt.direrr = 0.0;
      meanForPrompt.tdraw = 0.0;
      
      for (int s=0;s<dataallsub[cond][prompt].size();s++) {
        int n = dataallsub[cond][prompt][s].counter;
        dataallsub[cond][prompt][s].curv /= n;
        dataallsub[cond][prompt][s].pos /= n;
        dataallsub[cond][prompt][s].dir /= n;
        dataallsub[cond][prompt][s].poserr /= n;
        dataallsub[cond][prompt][s].direrr /= n;
        dataallsub[cond][prompt][s].tdraw /= n;
        
        meanForPrompt.curv += dataallsub[cond][prompt][s].curv;
        meanForPrompt.pos += dataallsub[cond][prompt][s].pos;
        meanForPrompt.dir += dataallsub[cond][prompt][s].dir;
        meanForPrompt.poserr += dataallsub[cond][prompt][s].poserr;
        meanForPrompt.direrr += dataallsub[cond][prompt][s].direrr;
        meanForPrompt.tdraw += dataallsub[cond][prompt][s].tdraw;
        
        int ip = 0;
        for (double p=0.1;p<=1.2;p+=0.1) {
          double K = dataallsub[cond][prompt][s].curv;
          double Kp = pow(K,p);
          tK[ip] += Kp;
          tdyK[ip] += fabs(dataallsub[cond][prompt][s].dir[1])*Kp;
          tdzK[ip] += -dataallsub[cond][prompt][s].dir[2]*Kp;
          ip++;
        }

        if (!ConfigVal("PrintWholelines",0,false)) {
          cout << cond << ", " << prompt << ", " << s << ", "
               << dataallsub[cond][prompt][s].curv << ", "
               << dataallsub[cond][prompt][s].pos[0] << ", "
               << dataallsub[cond][prompt][s].pos[1] << ", "
               << dataallsub[cond][prompt][s].pos[2] << ", "
               << dataallsub[cond][prompt][s].dir[0] << ", "
               << dataallsub[cond][prompt][s].dir[1] << ", "
               << dataallsub[cond][prompt][s].dir[2] << ", "
               << dataallsub[cond][prompt][s].poserr << ", "
               << dataallsub[cond][prompt][s].direrr << ", "
               << dataallsub[cond][prompt][s].tdraw << endl;
        }
      }
      if (ConfigVal("PrintWholelines",0,false)) {
        
        meanForPrompt.curv /= dataallsub[cond][prompt].size();
        meanForPrompt.pos /= dataallsub[cond][prompt].size();
        meanForPrompt.dir /= dataallsub[cond][prompt].size();
        meanForPrompt.poserr /= dataallsub[cond][prompt].size();
        meanForPrompt.direrr /= dataallsub[cond][prompt].size();
        //meanForPrompt.tdraw /= dataallsub[cond][prompt].size();
        
        cout << cond << ", " << prompt << ", "
             << meanForPrompt.poserr << ", "
             << meanForPrompt.direrr << ", "
             << meanForPrompt.tdraw << ", "
             // this is prompt arclen, change 0.049 if you change the prompt sampling.
             << 0.049*(double)dataallsub[cond][prompt].size();
        
        for (int p=0;p<tK.size();p++) {
          cout << ", " << tK[p] << ", " << tdyK[p] << ", " << tdzK[p];  
        }
        cout << endl;
             
      }
    }
  }
  ***/


  exit(0);
}

