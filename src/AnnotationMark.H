/**
 * /author Dan Keefe (dfk)
 *
 * /file  AnnotationMark.H
 * /brief 
 *
 */ 


#ifndef ANNOTATIONMARK_H
#define ANNOTATIONMARK_H

#include <VRG3DBaseApp.h>
#include "AnnotationModel.H"
#include "Mark.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class AnnotationMark> AnnotationMarkRef;
/**
*/
class AnnotationMark : public Mark
{
public:
  AnnotationMark(const std::string &name,
                 GfxMgrRef gfxMgr,
                 AnnotationModelRef annotationModel);
  
  virtual ~AnnotationMark();

  MarkRef copy();

  virtual void addMarkSpecificSample(BrushStateRef brushState);
  
  virtual std::string markDescription() { return "AnnotationMark"; }

  virtual void pose(G3D::Array<G3D::PosedModel::Ref>& posedModels, const G3D::CoordinateFrame& frame);
  virtual void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame& frame);

  virtual void setHighlighted(bool highlighted, ArtworkRef artwork);
  virtual void show();
  virtual void hide();
  virtual void stopDrawing();

  virtual void setName(const std::string &name);

  virtual void transformBy(G3D::CoordinateFrame frame);


private:

  AnnotationMark(const AnnotationMark &);
  AnnotationMark& operator=(const AnnotationMark &);

  AnnotationModelRef _annotationModel;
  bool               _highlighted;
  int                _modelID;
  G3D::Color3             _color;
};


} // end namespace

#endif

