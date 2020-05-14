

#include <G3DOperators.h>
#include "BrushState.H"

using namespace G3D;
namespace DrawOnAir {

void
BrushState::getDrawParametersBasedOnBrushModel(Vector3 &right, double &markWidth,
                                               Vector3 &lastGoodUpVec, Vector3 &lastGoodRightVec,
                                               Vector3 lastRightVec)
{
  BrushModelType modelToUse = brushModel;
  if (brushModel == BrushState::BRUSHMODEL_DEFAULT) {
    if (brushInterface == "DirectDraw") {
      //modelToUse = BrushState::BRUSHMODEL_CAVEPAINTING;
      //modelToUse = BrushState::BRUSHMODEL_ROUND;
      modelToUse = BrushState::BRUSHMODEL_DEFAULT;
    }
    if (brushInterface == "DragDraw") {
      modelToUse = BrushState::BRUSHMODEL_ROUND;
    }
  }

  // --------------------  CAVEPAINTING  --------------------
  if (modelToUse == BrushState::BRUSHMODEL_CAVEPAINTING) {
    right = -frameInRoomSpace.rotation.column(0);
    markWidth = width;
  }

  // --------------------  DEFAULT  --------------------
  else if (modelToUse == BrushState::BRUSHMODEL_DEFAULT) {

    /***
    Vector3 brushHandle = frameInRoomSpace.rotation.column(2);
    double d = fabs(brushHandle.dot(drawingDir));

    if (d < 0.4) {
      Vector3 norm = (brushHandle - brushHandle.dot(drawingDir)*drawingDir).unit();
      lastGoodUpVec = norm;
      right = drawingDir.cross(norm).unit();
    }
    else if (d < 0.65) {
      Vector3 norm = (brushHandle - brushHandle.dot(drawingDir)*drawingDir).unit();
      if (norm.dot(lastGoodUpVec) < 0.0) {
        norm = -norm;
      }
      double alpha = (d - 0.4) / 0.25;
      Vector3 interpNorm = norm.lerp(lastGoodUpVec, alpha).unit();
      right = drawingDir.cross(interpNorm).unit();
      cout << " interp " << right << endl;
    }
    else {
      double d2 = fabs(lastGoodUpVec.dot(drawingDir));
      Vector3 brushRight = -frameInRoomSpace.rotation.column(0);
      if (d2 < 0.7) {
        right = drawingDir.cross(lastGoodUpVec).unit();
        cout << right << " lastgood=" << lastGoodUpVec << endl;
      }
      else if (d2 < 0.8) {
        double alpha = (d2 - 0.7) / 0.1;
        Vector3 tmpRight;
        tmpRight = drawingDir.cross(lastGoodUpVec).unit();
        if (tmpRight.dot(brushRight) < 0.0) {
          tmpRight = -tmpRight;
        }
        Vector3 interpRight = tmpRight.lerp(brushRight, alpha);
        right = tmpRight;
      }
      else {
        cout << "newcase" << endl;
        // brush handle, drawing dir, and lastgoodupvec are all
        // pointing in the same dir, so use the right vec of the brush
        right = brushRight;
      }
    }
    ***/

    double dot1 = fabs(drawingDir.dot(-frameInRoomSpace.rotation.column(0).unit()));
    double cutoff1 = 0.4;
    double transition1 = 0.25;

    double dot2 = fabs(drawingDir.dot(lastGoodUpVec));
    double cutoff2 = 0.6;
    double transition2 = 0.2;

    if (dot1 <= cutoff1) {
      // default calculation
      right = -frameInRoomSpace.rotation.column(0).unit();
      right = right - right.dot(drawingDir)*drawingDir;
      lastGoodUpVec = right.cross(drawingDir).unit();
      lastGoodRightVec = right;
    }
    else {
      Vector3 rightVer2 = drawingDir.cross(lastGoodUpVec).unit();
      if (dot2 <= cutoff2) {
        lastGoodRightVec = rightVer2;
      }
      else if ((dot2 > cutoff2) && (dot2 < (cutoff2 + transition2))) {
        double alpha = (dot2 - cutoff2) / transition2;
        if (lastGoodRightVec.dot(rightVer2) < 0.0) {
          rightVer2 = -rightVer2;
        }
        rightVer2 = rightVer2.lerp(lastGoodRightVec, alpha);
      }
      else if (dot2 > (cutoff2 + transition2)) {
        rightVer2 = lastGoodRightVec;
      }

      if (dot1 < (cutoff1 + transition1)) {
        Vector3 defaultRight = -frameInRoomSpace.rotation.column(0).unit();
        defaultRight = defaultRight - defaultRight.dot(drawingDir)*drawingDir;
        if (defaultRight.dot(rightVer2) < 0.0) {
          rightVer2 = -rightVer2;
        }
        double alpha = (dot1 - cutoff1) / transition1;
        right = defaultRight.lerp(rightVer2, alpha);
      }
      else {
        right = rightVer2;
      }
    }


    /***8

    right = -frameInRoomSpace.rotation.column(0).unit();

    double d = fabs(right.dot(drawingDir));

    // Default algorithm for determining the "right" vector: The right
    // vec is the component of the vector across the flat tip of the
    // brush that is perpendicular to the drawing direction.
    right = right - right.dot(drawingDir)*drawingDir;
    
    // If for some reason the right value is bad or we don't trust it
    // because the brush is being held too close to the drawing
    // direction, try to do what the user would expect to happen,
    // which is usually to maintain the same normal which can be
    // constructed using the last good up vector.
    double cutoff = 0.4;
    double transition = 0.25;
    if ((d > cutoff) || (!right.isFinite()) || (right.isZero())) {
      // between d = 0.7 and d = 0.8 linearly interpolate between this
      // version of the right vec and the usual so there is never an
      // abrupt transition.
      Vector3 safeRight = lastGoodRightVec;
      if (fabs(drawingDir.dot(lastGoodUpVec)) < 0.7) {
        safeRight = drawingDir.cross(lastGoodUpVec).unit();
      }
      if (safeRight.dot(right) < 0.0) {
        safeRight = -safeRight;
      }
      double alpha = clamp((d - cutoff) / transition, 0.0, 1.0);
      right = right.lerp(safeRight, alpha);

      if ((!right.isFinite()) || (right.isZero())) {
        // hopefully, never get this far, but just in case..
        right = -frameInRoomSpace.rotation.column(0);
      }
    }
    else {
      // The usual algorithm worked fine, so update the last good up vec
      lastGoodUpVec = right.cross(drawingDir).unit();
    }
    ****/

    if (right.dot(lastRightVec) < 0.0) {
      right = -right;
    }

    markWidth = width;
  }
    
  // --------------------  ROUND  --------------------
  else if (modelToUse == BrushState::BRUSHMODEL_ROUND) {
    // Factor orientation out of the thickness of the mark.
    // Use orientation only to determine the surface normal
    
    // find the component of the brush's handle vec that is perpendicular to the drawing dir
    Vector3 brushHandle = frameInRoomSpace.rotation.column(2);
    Vector3 perp = (brushHandle - drawingDir*brushHandle.dot(drawingDir)).unit();
    right = drawingDir.cross(perp);
    if ((!right.isFinite()) || (right.isZero())) {
      right = -frameInRoomSpace.rotation.column(0);
    }

    // make sure the right x drawingDir points in the same direction as up
    Vector3 up = frameInRoomSpace.rotation.column(1);
    Vector3 cr = right.cross(drawingDir);
    if (cr.dot(up) < 0.0) {
      right = -right;
    }

    markWidth = width;
  }
  
  // --------------------  CHARCOAL_W_HEURISTIC  --------------------
  else if (modelToUse == BrushState::BRUSHMODEL_CHARCOAL_W_HEURISTIC) {
    // Ignore pressure in calculating width, use orientation relative to the film plane
    // Pressure controls color value only.

    double dotx = drawingDir.dot(Vector3(1,0,0));
    double doty = drawingDir.dot(Vector3(0,1,0));
    if (fabs(dotx) > fabs(doty)) {
      if (dotx < 0) {
        right = Vector3(0,1,0);
      }
      else {
        right = Vector3(0,-1,0);
      }
    }
    else {
      if (doty > 0) {
        right = Vector3(1,0,0);
      }
      else {
        right = Vector3(-1,0,0);
      }
    }

    markWidth = size * (1.0 - G3D::clamp((double)Vector3(0,0,1).dot(frameInRoomSpace.rotation.column(2)), 0.0, 0.95));
  }
}


BrushState::BrushState()
{
  // defaults:
  size = 0.03;
  colorSwatchCoord = 0;
  brushTip = 0;
  pattern = 0;
  layerIndex = 0;
  frameIndex = 0;
  superSampling = 0;
  pressure = 0;
  width = size;
  colorValue = 1.0;
  maxPressure = 0;
  brushInterface = "Unknown";
  twoSided = true;
  brushModel = BRUSHMODEL_DEFAULT;
  textureApp = TEXAPP_STAMP;
}

BrushState::~BrushState()
{
}

BrushStateRef
BrushState::copy()
{
  BrushStateRef bnew = new BrushState();

  bnew->size = size;
  bnew->colorSwatchCoord = colorSwatchCoord;
  bnew->brushTip = brushTip;
  bnew->pattern = pattern;
  bnew->layerIndex = layerIndex;
  bnew->frameIndex = frameIndex;
  bnew->superSampling = superSampling;
  bnew->twoSided = twoSided;
  bnew->textureApp = textureApp;
  bnew->brushInterface = brushInterface;
  bnew->brushModel = brushModel;
  bnew->frameInRoomSpace = frameInRoomSpace;
  bnew->physicalFrameInRoomSpace = physicalFrameInRoomSpace;
  bnew->drawingDir = drawingDir;
  bnew->width = width;
  bnew->colorValue = colorValue;
  bnew->pressure = pressure;
  bnew->maxPressure = maxPressure;
  bnew->handFrame = handFrame;
  bnew->headFrame = headFrame;

  return bnew;
}


BrushStateRef
BrushState::lerp(BrushStateRef other, double alpha)
{
  BrushStateRef bnew = copy();
  if (alpha > 0) {
    bnew->frameInRoomSpace = frameInRoomSpace.lerp(other->frameInRoomSpace, alpha);
    bnew->physicalFrameInRoomSpace = physicalFrameInRoomSpace.lerp(other->physicalFrameInRoomSpace, alpha);
    bnew->drawingDir = drawingDir.lerp(other->drawingDir, alpha);
    bnew->width = G3D::lerp(width, other->width, alpha);
    bnew->colorValue = G3D::lerp(colorValue, other->colorValue, alpha);
    bnew->pressure = G3D::lerp(pressure, other->pressure, alpha);
    bnew->handFrame = frameInRoomSpace.lerp(other->handFrame, alpha);
    bnew->headFrame = frameInRoomSpace.lerp(other->headFrame, alpha);
  }
  return bnew;
}

/** Each time you change the format of this, increase the
    SERIALIZE_VERSION_NUMBER and respond accordingly in the
    deserialize method.  In this method, you can safely get rid of the
    old code because we don't want to create files in an old format.
*/
#define SERIALIZE_VERSION_NUMBER 1
void
BrushState::serialize(BinaryOutput &b)
{
  b.writeInt8(SERIALIZE_VERSION_NUMBER);
  b.writeFloat64(size);
  b.writeFloat64(colorSwatchCoord);
  b.writeInt8(brushTip);
  b.writeInt8(pattern);
  b.writeInt8(layerIndex);
  b.writeInt8(frameIndex);
  b.writeInt8(superSampling);
  b.writeBool8(twoSided);
  b.writeInt8((int)textureApp);
  b.writeString(brushInterface);
  b.writeInt8((int)brushModel);
  frameInRoomSpace.serialize(b);
  physicalFrameInRoomSpace.serialize(b);
  drawingDir.serialize(b);
  b.writeFloat64(width);
  b.writeFloat64(colorValue);
  b.writeFloat64(pressure);
  b.writeFloat64(maxPressure);
  handFrame.serialize(b);
  headFrame.serialize(b);
}


/** Never delete the code from an old version from this method so we
    can always stay backwards compatable.  Important, since this is in
    binary, and we can't really reverse engineer the file format.
*/
void
BrushState::deserialize(BinaryInput &b)
{
  int version = b.readInt8();

  if (version == 1) {
    size = b.readFloat64();
    colorSwatchCoord = b.readFloat64();
    brushTip = b.readInt8();
    pattern = b.readInt8();
    layerIndex = b.readInt8();
    frameIndex = b.readInt8();
    superSampling = b.readInt8();
    twoSided = b.readBool8();
    textureApp = (TextureAppType)b.readInt8();
    brushInterface = b.readString();
    brushModel = (BrushModelType)b.readInt8();
    frameInRoomSpace.deserialize(b);
    physicalFrameInRoomSpace.deserialize(b);
    drawingDir.deserialize(b);
    width = b.readFloat64();
    colorValue = b.readFloat64();
    pressure = b.readFloat64();
    maxPressure = b.readFloat64();  
    handFrame.deserialize(b);
    headFrame.deserialize(b);
  }
  else if (version == 0) {
    size = b.readFloat64();
    colorSwatchCoord = b.readFloat64();
    brushTip = b.readInt8();
    pattern = b.readInt8();
    layerIndex = b.readInt8();
    superSampling = b.readInt8();
    twoSided = b.readBool8();
    textureApp = (TextureAppType)b.readInt8();
    brushInterface = b.readString();
    brushModel = (BrushModelType)b.readInt8();
    frameInRoomSpace.deserialize(b);
    physicalFrameInRoomSpace.deserialize(b);
    drawingDir.deserialize(b);
    width = b.readFloat64();
    colorValue = b.readFloat64();
    pressure = b.readFloat64();
    maxPressure = b.readFloat64();  
    handFrame.deserialize(b);
    headFrame.deserialize(b);
    // in this version, layers acted as frames do now and there was no VS frame
    frameIndex = layerIndex;
    layerIndex = 0;
  }
  else {
    alwaysAssertM(false, "Unrecognized BrushState version " + MinVR::intToString(version));
  }
}

} // end namespace
