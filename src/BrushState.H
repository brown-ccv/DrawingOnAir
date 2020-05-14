/**
 * \author Dan Keefe (dfk)
 *
 * \file  BrushState.H
 * \brief Holds the state of the paint brush.
 *
 */

#ifndef BRUSHSTATE_H
#define BRUSHSTATE_H
#include <G3D/G3D.h>
#include <GLG3D/GLG3D.h>
//#include "VRBase.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class BrushState> BrushStateRef;
/** Holds the state of the brush: MarkForm, Material, width, and
    artworkLayer to paint onto.
 */
class BrushState : public G3D::ReferenceCountedObject
{
public:

  enum  BrushModelType {
    /// Pressure only controls line weight, but flatness rather than handle controls up vec
    BRUSHMODEL_DEFAULT = 0,
    
    /// Flat brush a la CavePaiting, twist can adjust effective line weight
    BRUSHMODEL_CAVEPAINTING = 1,
    
    /// Round where the handle controls the up vector of the form
    BRUSHMODEL_ROUND = 2,       
    
    // Angle of handle relative to filmplane controls line width, pressure controls
    // color value, initial drawing direction locks in the cross-section vector
    BRUSHMODEL_CHARCOAL_W_HEURISTIC = 3
  };


  /// The way that textures get mapped onto form
  enum TextureAppType {
    TEXAPP_STAMP = 0,
    TEXAPP_STRETCH = 1
  };

  BrushState();
  virtual ~BrushState();
  
  BrushStateRef copy();
  
  /// alpha=0 returns a copy of this, alpha=1 returns a copy of other,
  /// does linear interpolation and returns a new brushstate in between
  BrushStateRef lerp(BrushStateRef other, double alpha);
    
  /// Reports parameters useful for creating marks.  These parameters
  /// are interpreted differently based on the state of the brush model.
  void getDrawParametersBasedOnBrushModel(G3D::Vector3 &right, double &width, 
                                          G3D::Vector3 &lastGoodUpVec, G3D::Vector3 &lastGoodRightVec,
                                          G3D::Vector3 lastRightVec);
  
  void serialize(class G3D::BinaryOutput& b);
  void deserialize(class G3D::BinaryInput& b);

  
  // ** Brush State Variables: These can be set directly by Drawing Interfaces or Widgets **
  // ---------------------------------------------------------------------------------------
  
  /// These stay the same while a mark is being created
  double                 size;
  double                 colorSwatchCoord;
  int                    brushTip;
  int                    pattern;
  int                    layerIndex;
  int                    frameIndex;
  int                    superSampling;  // 0 for no SuperSampling
  bool                   twoSided;       // corresponds to CULL_NONE or CULL_BACK
  TextureAppType         textureApp;

  /// This string stores a description of the  interface controlling the brush.
  /// Intended to be passed to new Marks as they are created.
  std::string            brushInterface;
  BrushModelType         brushModel;


  // These change as a mark is being created

  G3D::CoordinateFrame        frameInRoomSpace;

  /// For tape drawing and maybe other interfaces the physical location of the 
  /// brush can be different than the virtual or proxy location.  This variable
  /// holds the actual physical location.  This is only guaranteed to be updated
  /// when one of these types of interfaces is active.
  G3D::CoordinateFrame        physicalFrameInRoomSpace;
  
  G3D::Vector3                drawingDir;
  
  double                 width;
  double                 colorValue;

  /// Pressure can be interpreted as controlling either the width or the color
  /// value of the mark as it is being created.  So, Pressure is the raw
  /// reading relative to maxPressure, while Width and ColorValue
  /// are usually set to change based on pressure changes.  Typically, 
  /// Width = (Pressure/maxPressure) * Size;
  /// ColorValue = Pressure/maxPressure;
  double                 pressure;

  /// This should be set based on the input device being used.  If the device is not
  /// pressure sensative, then set it to 0.
  double                 maxPressure;


  /// These store state info that is not currently used, but should be used in
  /// some interactions, and might be used to analyze user performance in VR.
  G3D::CoordinateFrame        handFrame;
  G3D::CoordinateFrame        headFrame;

private:
  BrushState(const BrushState &);
  BrushState& operator=(const BrushState &);
};



} // end namespace

#endif


