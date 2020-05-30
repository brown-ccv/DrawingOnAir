#include <ConfigVal.H>
#include "Selection.H"


/**
void getBounds(const DrawOnAir::MarkBox &mb, G3D::AABox &b) {
  b = mb.box;
}

bool operator==(const DrawOnAir::MarkBox &mb1, const DrawOnAir::MarkBox &mb2) {
  return (mb1.box == mb2.box);
}

unsigned int hashCode(const DrawOnAir::MarkBox &mb) {
  return mb.box.hashCode();
} 
**/


using namespace G3D;

namespace DrawOnAir {

MarkBox::MarkBox(MarkRef m, int sampleNum, Vector3 lo, Vector3 hi)
{
  mark = m;
  box = AABox(lo, hi);
  sample = sampleNum;
}

MarkBox::MarkBox(const MarkBox &other) {
    mark = other.mark;
    box = other.box;
    sample = other.sample;
}

MarkBox& MarkBox::operator=(const MarkBox &other) {
	if (this != &other) {
		mark = other.mark;
		box = other.box;
		sample = other.sample;
	}
	return *this;
}

    
MarkBox::MarkBox() {}

MarkBox::~MarkBox() {}

bool
MarkBox::operator==(const MarkBox& other) const 
{
  return ((box == other.box) && (sample == other.sample));
}

unsigned int
MarkBox::hashCode() const
{
  return box.hashCode();
}



void buildBSPTree(ArtworkRef artwork, KDTree<MarkBox> &tree, double handleLen, GfxMgrRef gfxMgr)
{
  tree.clear();
  
  Array<int> hidden = artwork->getHiddenLayers();
  double s = MinVR::ConfigVal("Selection_BoxSize", 0.01, false) / 2.0;
  s = gfxMgr->roomVectorToVirtualSpace(Vector3(s,0,0)).length();
  Vector3 s3 = Vector3(s,s,s);
  
  for (int i=0;i<artwork->getMarks().size();i++) {
    MarkRef m = artwork->getMarks()[i];
    int n = m->getNumSamples();
    if ((n > 0) &&
        (!hidden.contains(m->getInitialBrushState()->layerIndex)) &&
        (m->getInitialBrushState()->frameIndex == artwork->getCurrentFrame())) {

      if (n == 1) {
        Vector3 p = m->getSamplePosition(0);
        MarkBox mb(m, 0, p-s3, p+s3);
        tree.insert(mb);
      }
      else {
        double len = gfxMgr->roomVectorToVirtualSpace(Vector3(handleLen,0,0)).length();
        if (m->markDescription() == "AnnotationMark") {
          len = 0.0;
        }

        int sn = m->getNumSamples()-1;

        Vector3 p = m->getSamplePosition(0);
        Vector3 d = (m->getSamplePosition(0) - m->getSamplePosition(sn)).unit();
        Vector3 b = p + len*d;
        MarkBox mb(m, 0, b-s3, b+s3);
        tree.insert(mb);

        p = m->getSamplePosition(sn);
        d = -d;
        b = p + len*d;
        mb = MarkBox(m, sn, b-s3, b+s3);
        tree.insert(mb);
      }
    }
  }
}


} // end namespace
