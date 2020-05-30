/**
 * \author Dan Keefe (dfk)
 *
 * \file  MoveMarksHCI.H
 * \brief
 *
 */


#ifndef MOVEMARKSHCI_H
#define MOVEMARKSHCI_H

// Selection needs to be included first
#include "Selection.H"

//#include "VRBase.H"
#include "Artwork.H"
#include "BrushState.H"
#include "CavePaintingCursors.H"
#include "HCIMgr.H"
#include "History.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class MoveMarksHCI> MoveMarksHCIRef;
/** 
*/
class MoveMarksHCI : public StylusHCI
{
public:
  MoveMarksHCI(G3D::Array<std::string>     motionTriggers, 
               G3D::Array<std::string>     onTriggers,
               G3D::Array<std::string>     offTriggers,
               ArtworkRef             artwork,
               BrushRef               brush,
               CavePaintingCursorsRef cursors,
    EventMgrRef            eventMgr,
    GfxMgrRef              gfxMgr,
    HistoryRef             history);
  virtual ~MoveMarksHCI();

  void setEnabled(bool b);
  bool canReleaseFocus();

  // interactor methods
  void motion(MinVR::EventRef e);
  void on(MinVR::EventRef e);
  void dragMotion(MinVR::EventRef e);
  void off(MinVR::EventRef e);

  void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);

protected:
  ArtworkRef             _artwork;
  BrushRef               _brush;
  CavePaintingCursorsRef _cursors;
  EventMgrRef            _eventMgr;
  FsaRef                 _fsa;
  GfxMgrRef              _gfxMgr;
  HistoryRef             _history;
  MarkRef                _selectedMark;
  bool                   _enabled;
  G3D::CoordinateFrame        _lastFrame;
  int                    _indexOfClosestPt;
  int                    _drawID;
  G3D::Array<MarkBox>         _intersectedBoxes;
  MarkBox                _selectedBox;
  G3D::KDTree<MarkBox>     _tree;

private:
  MoveMarksHCI(const MoveMarksHCI &);
  MoveMarksHCI& operator=(const MoveMarksHCI &);
};


} // end namespace
#endif
