
#include <G3DOperators.h>
#include "Mark.H"

#include "BrushState.H"


using namespace G3D;
namespace DrawOnAir {

Mark::Mark(const std::string &name, 
           bool shouldBeDrawn, bool shouldBePosed, 
           MinVR::GfxMgrRef gfxMgr)
{
  _gfxMgr = gfxMgr;
  _name = name;
  _shouldBeDrawn = shouldBeDrawn;
  _shouldBePosed = shouldBePosed;
  _roomToVirtualFrameAtStart = _gfxMgr->getRoomToVirtualSpaceFrame();
  _roomToVirtualScaleAtStart = _gfxMgr->getRoomToVirtualSpaceScale();
  _poseDirty = true;
}

void
Mark::trimEnd(int newEndPt)
{
  int size = newEndPt + 1;
  _samplePositions.resize(size, true);
  _sampleIsASuperSample.resize(size, true);
  _brushStates.resize(size, true);
  _arcLengths.resize(size, true);
}

void
Mark::addSample(BrushStateRef brushState)
{
  Vector3 samplePosition = _gfxMgr->roomPointToVirtualSpace(brushState->frameInRoomSpace.translation);


  // Expand bounding box
  if (getNumSamples() == 0) {
    _bbox = AABox(samplePosition);
  }
  else {
    MinVR::growAABox(_bbox, samplePosition);
  }

  // If superSampling is turned off just add it.
  if (brushState->superSampling == 0) {
    _samplePositions.append(samplePosition);
    _sampleIsASuperSample.append(false);
    _brushStates.append(brushState->copy());
    if (_samplePositions.size() == 1) {
      _arcLengths.append(0.0);
    }
    else {
      double l = (_samplePositions.last() - _samplePositions[_samplePositions.size()-2]).length();
      debugAssert(_arcLengths.size());
      _arcLengths.append(_arcLengths.last() + l);
    }
    addMarkSpecificSample(brushState);
  }
  else {
    // Otherwise, add it according to supersampling criteria
    if (getNumSamples() == 0) {
      // For the first sample, no interpolation..
      _samplePositions.append(samplePosition);
      _sampleIsASuperSample.append(false);
      _brushStates.append(brushState->copy());
      if (_samplePositions.size() == 1) {
        _arcLengths.append(0.0);
      }
      else {
        double l = (_samplePositions.last() - _samplePositions[_samplePositions.size()-2]).length();
        debugAssert(_arcLengths.size());
        _arcLengths.append(_arcLengths.last() + l);
      }
      addMarkSpecificSample(brushState);
    }
    else {
      double sampleInterval = brushState->size/brushState->superSampling;
      Vector3 lastSamplePosition = _samplePositions.last();
      double l = (samplePosition - lastSamplePosition).magnitude();
      int num = iClamp(iRound(l / sampleInterval),1,10000);
 
      // add n samples
      int last = num-1;
      for (int i=1;i<num;i++) {
        double a = (double)i/(double)num;
        Vector3 sampleP = lastSamplePosition.lerp(samplePosition, a);
        BrushStateRef sampleBS = _brushStates.last()->lerp(brushState, a);
        _samplePositions.append(sampleP);
        _sampleIsASuperSample.append(i != last);
        _brushStates.append(sampleBS);
        if (_samplePositions.size() == 1) {
          _arcLengths.append(0.0);
        }
        else {
          double l = (_samplePositions.last() - _samplePositions[_samplePositions.size()-2]).length();
          debugAssert(_arcLengths.size());
          _arcLengths.append(_arcLengths.last() + l);
        }
        addMarkSpecificSample(sampleBS);
      }
    }
  }
  _poseDirty = true;
}


double
Mark::getArcLength()
{
  if (_arcLengths.size()) {
    return _arcLengths.last();
  }
  return 0.0;
}

double
Mark::getArcLength(int fromSample, int toSample)
{
  if (_arcLengths.size() <= fromSample) {
    return 0.0;
  }
  else if (_arcLengths.size() <= toSample) {
    return _arcLengths.last() - _arcLengths[fromSample];
  }
  else {
    return _arcLengths[toSample] - _arcLengths[fromSample];
  }
}

Array<BrushStateRef>
Mark::getBrushStatesSubArray(int start, int stop)
{
  Array<BrushStateRef> bs;
  for (int i=start;i<stop;i++) {
    bs.append(_brushStates[i]);
  }
  return bs;
}


bool
Mark::contains(const Vector3 &point, double widthScaleFactor)
{
  if (!boundingBoxContains(point)) {
    return false;
  }

  double threshold = widthScaleFactor * getInitialBrushState()->size;
  // this is too slow.. try incrementing by 2 to speed it up.
  for (int i=0;i<_samplePositions.size();i+=1) {
    if ((point - _samplePositions[i]).magnitude() < threshold) {
      return true;
    }
  }
  return false;
}

bool
Mark::approxContains(const Vector3 &point, double widthScaleFactor)
{
  int i;
  return approxContains(point, widthScaleFactor, i);
}

bool
Mark::approxContains(const Vector3 &point, double widthScaleFactor, int &closestIndex)
{
  if (!boundingBoxContains(point)) {
    return false;
  }

  double threshold = widthScaleFactor * getInitialBrushState()->size;

  int n = 1;
  if (_samplePositions.size() > 100) {
    n = iRound((double)_samplePositions.size() / 100.0);
  }

  closestIndex = 0;
  for (int i=0;i<_samplePositions.size();i+=n) {
    if ((point - _samplePositions[i]).magnitude() < threshold) {
      closestIndex = i;
      return true;
    }
  }
  return false;
}


double
Mark::distanceToMark(const Vector3 &point)
{
  int c;
  return distanceToMark(point, c);
}

double
Mark::distanceToMark(const Vector3 &point, int &closestIndex)
{
  closestIndex = 0;
  if (_samplePositions.size() == 0) {
    return -1.0;
  }
  else if (_samplePositions.size() == 1) {
    return (_samplePositions[0] - point).length();
  }

  double minDist = (point - _samplePositions[0]).magnitude();
  for (int i=1;i<_samplePositions.size();i++) {
    G3D::LineSegment l = G3D::LineSegment::fromTwoPoints(_samplePositions[i-1], _samplePositions[i]);
    double d = l.distance(point);
    if (d < minDist) {
      minDist = d;
      double a = (point - _samplePositions[i-1]).length();
      double b = (point - _samplePositions[i]).length();
      if (a < b) {
        closestIndex = i-1;
      }
      else {
        closestIndex = i;
      }
    }
  } 
  return minDist;
}

double
Mark::approxDistanceToMark(const Vector3 &point)
{
  int i;
  return approxDistanceToMark(point, i);
}

double
Mark::approxDistanceToMark(const Vector3 &point, int &closestIndex)
{
  if (_samplePositions.size() == 0) {
    return -1.0;
  }

  closestIndex = 0;
  double minDist = (point - _samplePositions[0]).magnitude();

  int n = 1;
  if (_samplePositions.size() > 100) {
    n = iRound((double)_samplePositions.size() / 100.0);
  }

  for (int i=1;i<_samplePositions.size();i+=n) {
    double d = (point - _samplePositions[i]).magnitude();
    if (d < minDist) {
      minDist = d;
      closestIndex = i;
    }
  }
  return minDist;
}


void
Mark::setShouldBeDrawn(bool b)
{
  _shouldBeDrawn = b;
  setPoseDirty();
}

void
Mark::setShouldBePosed(bool b)
{
  _shouldBePosed = b;
  setPoseDirty();
}

void
Mark::saveTransformEvent(CoordinateFrame frame)
{
  // TODO: This is going to generate a ton of data, we should be able
  // to combine the transforms, I think, but I can't get it to work.
  
  /***
  if ((_transformHistVSFrame.size()) &&
      (_transformHistVSFrame.last() == _gfxMgr->getRoomToVirtualSpaceFrame()) &&
      (_transformHistVSScale.last() == _gfxMgr->getRoomToVirtualSpaceScale())) {
    _transformHistXform[_transformHistXform.size()-1] = 
      frame * _transformHistXform[_transformHistXform.size()-1];
    _transformHistXform[_transformHistXform.size()-1].rotation.orthonormalize();
  }
  else {
  ***/
    _transformHistVSFrame.append(_gfxMgr->getRoomToVirtualSpaceFrame());
    _transformHistVSScale.append(_gfxMgr->getRoomToVirtualSpaceScale());
    _transformHistXform.append(frame);  
    //}
}

void
Mark::transformBy(CoordinateFrame frame)
{
  saveTransformEvent(frame);

  // transform and rebuild bounding box too.
  for (int i=0;i<_samplePositions.size();i++) {
    _samplePositions[i] = frame.pointToWorldSpace(_samplePositions[i]);
    if (i == 0) {
      _bbox = AABox(_samplePositions[i]);
    }
    else {
      MinVR::growAABox(_bbox, _samplePositions[i]);
    }
  }

  setPoseDirty();
}

BrushStateRef
Mark::getInitialBrushState()
{
  if (_brushStates.size()) {
    return _brushStates[0];
  }
  return NULL;
}



/** Each time you change the format of this, increase the
    SERIALIZE_VERSION_NUMBER and respond accordingly in the
    deserialize method.  In this method, you can safely get rid of the
    old code because we don't want to create files in an old format.
*/
#define SERIALIZE_VERSION_NUMBER 1
void
Mark::serialize(BinaryOutput &b)
{
  b.writeInt8(SERIALIZE_VERSION_NUMBER);
  b.writeString(_name);
  _roomToVirtualFrameAtStart.serialize(b);
  b.writeFloat64(_roomToVirtualScaleAtStart);
  b.writeInt32(_brushStates.size());
  for (int i=0;i<_brushStates.size();i++) {
    _brushStates[i]->serialize(b);
  }
  b.writeInt32(_transformHistVSFrame.size());
  for (int i=0;i<_transformHistVSFrame.size();i++) {
    _transformHistVSFrame[i].serialize(b);
  }
  b.writeInt32(_transformHistVSScale.size());
  for (int i=0;i<_transformHistVSScale.size();i++) {
    b.writeFloat64(_transformHistVSScale[i]);
  }
  b.writeInt32(_transformHistXform.size());
  for (int i=0;i<_transformHistXform.size();i++) {
    _transformHistXform[i].serialize(b);
  }
}


/** Never delete the code from an old version from this method so we
    can always stay backwards compatable.  Important, since this is in
    binary, and we can't really reverse engineer the file format.
*/
void
Mark::deserialize(BinaryInput &b)
{
  int version = b.readInt8();

  if (version == 1) {
    _name = b.readString();
    _roomToVirtualFrameAtStart.deserialize(b);
    _roomToVirtualFrameAtStart.rotation.orthonormalize();
    _roomToVirtualScaleAtStart = b.readFloat64();
    int n = b.readInt32();
    Array<BrushStateRef> brushStates;
    for (int i=0;i<n;i++) {
      BrushStateRef bs = new BrushState();
      bs->deserialize(b);
      brushStates.append(bs);
    }

    n = b.readInt32();
    Array<CoordinateFrame> vsFrames;
    for (int i=0;i<n;i++) {
      CoordinateFrame cf;
      cf.deserialize(b);
      cf.rotation.orthonormalize();
      vsFrames.append(cf);
    }

    n = b.readInt32();
    Array<double> vsScales;
    for (int i=0;i<n;i++) {
      double s = b.readFloat64();
      vsScales.append(s);
    }

    n = b.readInt32();
    Array<CoordinateFrame> xforms;
    for (int i=0;i<n;i++) {
      CoordinateFrame cf;
      cf.deserialize(b);
      cf.rotation.orthonormalize();
      xforms.append(cf);
    }

    // Rebuild the mark by adding samples..
    double origScale = _gfxMgr->getRoomToVirtualSpaceScale();
    CoordinateFrame origFrame = _gfxMgr->getRoomToVirtualSpaceFrame();
    _gfxMgr->setRoomToVirtualSpaceScale(_roomToVirtualScaleAtStart);
    _gfxMgr->setRoomToVirtualSpaceFrame(_roomToVirtualFrameAtStart);
    for (int i=0;i<brushStates.size();i++) {
      Vector3 posVS = _gfxMgr->roomPointToVirtualSpace(brushStates[i]->frameInRoomSpace.translation);
      // for now, all samples are being saved, so don't do any extra supersampling
      brushStates[i]->superSampling = 0;
      addSample(brushStates[i]);
    }

    // This should reproduce any editing operations that were performed after the mark
    // was created.
    for (int i=0;i<xforms.size();i++) {
      _gfxMgr->setRoomToVirtualSpaceScale(vsScales[i]);
      _gfxMgr->setRoomToVirtualSpaceFrame(vsFrames[i]);
      transformBy(xforms[i]);
      //cout << i << " " << xforms[i] << endl;
    }

    _gfxMgr->setRoomToVirtualSpaceScale(origScale);
    _gfxMgr->setRoomToVirtualSpaceFrame(origFrame);

  }
  else if (version == 0) {
    _name = b.readString();
    _roomToVirtualFrameAtStart.deserialize(b);
    _roomToVirtualScaleAtStart = b.readFloat64();
    int n = b.readInt32();
    Array<BrushStateRef> brushStates;
    for (int i=0;i<n;i++) {
      BrushStateRef bs = new BrushState();
      bs->deserialize(b);
      brushStates.append(bs);
    }

    // Rebuild the mark by adding samples..
    double origScale = _gfxMgr->getRoomToVirtualSpaceScale();
    CoordinateFrame origFrame = _gfxMgr->getRoomToVirtualSpaceFrame();
    _gfxMgr->setRoomToVirtualSpaceScale(_roomToVirtualScaleAtStart);
    _gfxMgr->setRoomToVirtualSpaceFrame(_roomToVirtualFrameAtStart);
    for (int i=0;i<brushStates.size();i++) {
      Vector3 posVS = _gfxMgr->roomPointToVirtualSpace(brushStates[i]->frameInRoomSpace.translation);
      // for now, all samples are being saved, so don't do any extra supersampling
      brushStates[i]->superSampling = 0;
      addSample(brushStates[i]);
    }
    _gfxMgr->setRoomToVirtualSpaceScale(origScale);
    _gfxMgr->setRoomToVirtualSpaceFrame(origFrame);

  }
  else {
    alwaysAssertM(false, "Unrecognized Mark version " + MinVR::intToString(version));
  }
}


} // end namespace

