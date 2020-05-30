/**
 * /author Dan Keefe (dfk)
 *
 * /file  Selection.H
 * /brief This file must be included before G3D/KDTree.h to work.
 *
 */ 


#ifndef SELECTION_H
#define SELECTION_H

/**

These were used in previous versions of G3D to get the templates for
KDTree to work correctly.

// Forward declarations here so functions are defined before KDTree gets included
namespace DrawOnAir {
  class MarkBox;
}
namespace G3D {
  class AABox;
}

void getBounds(const DrawOnAir::MarkBox&, G3D::AABox&); 
bool operator==(const DrawOnAir::MarkBox&, const DrawOnAir::MarkBox&); 
unsigned int hashCode(const DrawOnAir::MarkBox&); 

// Now safe to include KDTree and the rest of G3D
**/


//#include <VRBase.H>
#include <VRG3DBaseApp.h>

#include "Mark.H"
#include "Artwork.H"

namespace DrawOnAir {

class MarkBox
{
public:
  MarkBox(MarkRef m, int sampleNum, G3D::Vector3 lo, G3D::Vector3 hi);
  MarkBox();
  MarkBox(const MarkBox &other);
  MarkBox& operator=(const MarkBox &);

  virtual ~MarkBox();

  bool operator==(const MarkBox& other) const;
  
  unsigned int hashCode() const; 

  MarkRef mark;
  G3D::AABox   box;
  int     sample;
};


void buildBSPTree(ArtworkRef artwork, G3D::KDTree<MarkBox> &tree, double handleLen, GfxMgrRef gfxMgr);


} // end namespace


template<> struct BoundsTrait<class DrawOnAir::MarkBox> { 
  static void getBounds(const DrawOnAir::MarkBox &obj, G3D::AABox &out) {
    out = obj.box;
  };
};

template<> struct HashTrait<DrawOnAir::MarkBox> {
 static size_t hashCode(const DrawOnAir::MarkBox& key) { return key.hashCode(); }
};



#endif

