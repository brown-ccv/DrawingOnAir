/**
 * \author Dan Keefe (dfk)
 *
 * \file  Brush.H
 * \brief 
 *
 */

#ifndef BRUSH_H
#define BRUSH_H

//#include "VRBase.H"
#include <VRG3DBaseApp.H>

#include "Artwork.H"
#include "BrushState.H"
#include "History.H"
#include "Mark.H"
#include "TextInputWidget.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class Brush> BrushRef;
/** Holds the  interactive BrushState and has methods that interactors call to
    create Marks based on that state.
 */
class Brush : public G3D::ReferenceCountedObject
{
public:
  Brush(BrushStateRef brushState, ArtworkRef artwork, GfxMgrRef gfxMgr, 
    EventMgrRef eventMgr, HistoryRef history);
  virtual ~Brush();

  BrushStateRef state;

  /// Call these 3 methods after updating the BrushState variables to generate a new mark
  MarkRef    startNewMark();
  void       addSampleToMark();
  void       endMark();

  void       makeNextMarkASlide(const std::string &textureName);

  /// Null if not currently painting
  MarkRef    currentMark;
  bool       currentlyPainting() { return currentMark.notNull(); }

  ArtworkRef getArtwork() { return _artwork; }

  void setNewMarkType(int type) { _newMarkType = type; }
  int  getNewMarkType() { return _newMarkType; }

  void setTextInputWidget(TextInputWidgetRef textInputWidget) {
    _textInputWidget = textInputWidget;
  }
  void startNameInput(MarkRef nameMark);
  void updateNameText(MinVR::EventRef e);
  void stopNameInput(MinVR::EventRef e);

  void setFixedWidth(double w) { _fixedWidth = w; }

private:

  Brush(const Brush &);
  Brush& operator=(const Brush &);

  ArtworkRef         _artwork;
  EventMgrRef        _eventMgr;
  FsaRef             _nameChangeFsa;
  GfxMgrRef          _gfxMgr;
  HistoryRef         _history;
  MarkRef            _nameMark;
  TextInputWidgetRef _textInputWidget;
  int                _newMarkType;
  std::string        _slideTextureName;
  // set this to some non-zero value to have all marks be a fixed
  // width, disabling any pressure controls for line weight.. useful
  // for studies..
  double             _fixedWidth;
};


} // end namespace

#endif


