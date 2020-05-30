/**
 * /author Dan Keefe (dfk)
 *
 * /file  SlideMark.H
 * /brief 
 *
 */ 


#ifndef SLIDEMARK_H
#define SLIDEMARK_H

//#include "VRBase.H"
#include "Mark.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class SlideMark> SlideMarkRef;
/**
*/
class SlideMark : public Mark
{
public:
  SlideMark(const std::string &name,
    GfxMgrRef gfxMgr);

  virtual ~SlideMark();

  MarkRef copy();

  virtual void addMarkSpecificSample(BrushStateRef brushState);
  
  virtual std::string markDescription() { return "SlideMark"; }

  virtual void pose(G3D::Array<G3D::PosedModel::Ref>& posedModels, const G3D::CoordinateFrame& frame);
  virtual void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame& frame);

  virtual void setHighlighted(bool highlighted, ArtworkRef artwork);
  virtual void show();
  virtual void hide();

  virtual void setName(const std::string &name);

  virtual void transformBy(G3D::CoordinateFrame frame);

private:
  SlideMark(const SlideMark &);
  SlideMark& operator=(const SlideMark &);

  G3D::CoordinateFrame _frame;
  double          _size;
  bool            _highlighted;
  bool            _hidden;
};


} // end namespace

#endif

