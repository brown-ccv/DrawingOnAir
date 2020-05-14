/**
 * /author Dan Keefe (dfk)
 *
 * /file  CavePaintingCursors.H
 * /brief Draws 3D cursors for CavePainting 2-handed input
 *
 */ 

#ifndef CAVEPAINTINGCURSORS_H
#define CAVEPAINTINGCURSORS_H

//#include "VRBase.H"
#include <VRG3DBaseApp.h>
#include "Brush.H"
#include "BrushState.H"
#include "GfxMgr.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class CavePaintingCursors> CavePaintingCursorsRef;
/**
*/
class CavePaintingCursors : public G3D::ReferenceCountedObject
{
public:
  CavePaintingCursors(MinVR::GfxMgrRef gfxMgr, MinVR::EventMgrRef eventMgr, BrushRef brush,
                      G3D::Array<std::string> handTrackerTriggers);
  virtual ~CavePaintingCursors();

  enum CursorType {
    /// Doesn't draw anything.
    NO_CURSOR,
    /// For brush, checks brushState to determine what to draw, for hand draws a sphere.
    DEFAULT_CURSOR,
    /// Draws an arrow.
    POINTER_CURSOR,
    /// Draws a red arrow.
    DELETE_CURSOR,
    /// Draws a sphere on a stick
    SPHERETIP_CURSOR
  };

  void setBrushCursor(CursorType t) { _brushCursor = t; }
  void setHandCursor(CursorType t)  { _handCursor = t; }

  void setSphereTipCursorRadius(double r) { _sphereTipRad = r; }

  void moveHandTracker(MinVR::EventRef e);

  void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);

  void drawFlatBrush(G3D::RenderDevice *rd, const G3D::CoordinateFrame &frame, double width, G3D::Color3 color, G3D::Texture::Ref texture);
  void drawRoundBrush(G3D::RenderDevice *rd, const G3D::CoordinateFrame &frame);
  void drawArrow(G3D::RenderDevice *rd, const G3D::CoordinateFrame &frame, double size, G3D::Color3 color);
  void drawCylinder(G3D::RenderDevice *rd, const G3D::CoordinateFrame &frame, double size, G3D::Color3 color);
  void drawSphere(G3D::RenderDevice *rd, const G3D::CoordinateFrame &frame, double rad, G3D::Color3 color);
  void drawWireSphere(G3D::RenderDevice *rd, const G3D::CoordinateFrame &frame, double rad, G3D::Color3 color);
  void drawBlendTool(G3D::RenderDevice *rd, const G3D::CoordinateFrame &frame, G3D::Color3 color);
  void drawPlane(G3D::RenderDevice *rd, const G3D::CoordinateFrame &frame, double w, double h, G3D::Color3 color);
  void drawSphereTip(G3D::RenderDevice *rd, const G3D::CoordinateFrame &frame);

  void drawPressureMeter();
  void drawDrawingGuides();
  void drawActiveLayerStatus(G3D::RenderDevice *rd);

  void drawLineAsCylinder(const G3D::Vector3 &startPoint,
                          const G3D::Vector3 &endPoint,
                          const double &startRadius,
                          const double &endRadius,
                          G3D::RenderDevice *renderDevice,
                          const G3D::Color4 &color,
                          const int &numSections,
                          G3D::Texture::Ref tex,
                          float texRepeat);

private:
  CavePaintingCursors(const CavePaintingCursors &);
  CavePaintingCursors& operator=(const CavePaintingCursors &);

  BrushRef        _brush;
  G3D::CoordinateFrame _handFrame;
  CursorType      _brushCursor;
  CursorType      _handCursor;
  G3D::VARArea::Ref      _varArea;
  G3D::VAR             _varBrushVerts;
  G3D::VAR             _varBrushTexCoords;
  G3D::VAR             _varBrushNormals;
  G3D::Array<G3D::uint32>   _brushIndices;
  MinVR::GfxMgrRef       _gfxMgr;
  double          _sphereTipRad;
  G3D::Vector3         _lastGoodUpVec;
  G3D::Vector3         _lastGoodRightVec;
  G3D::Vector3         _lastRight;
};


} // end namespace

#endif

