/**
 * \author Dan Keefe (dfk)
 *
 * \file  CopyToNextFrameHCI.H
 * \brief
 *
 */


#ifndef COPYTONEXTFRAMEHCI_H
#define COPYTONEXTFRAMEHCI_H

#include "Selection.H"

//#include "VRBase.H"
#include <Event.h>
#include <EventMgr.H>
#include <GfxMgr.H>

#include "Artwork.H"
#include "BrushState.H"
#include "CavePaintingCursors.H"
#include "HCIMgr.H"
#include "History.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class CopyToNextFrameHCI> CopyToNextFrameHCIRef;
/** 
*/
class CopyToNextFrameHCI : public StylusHCI
{
public:
  CopyToNextFrameHCI(G3D::Array<std::string>     motionTriggers, 
            G3D::Array<std::string>     onTriggers,
            G3D::Array<std::string>     offTriggers,
            ArtworkRef             artwork,
            BrushRef               brush,
            CavePaintingCursorsRef cursors,
            EventMgrRef            eventMgr,
            GfxMgrRef              gfxMgr,
            HistoryRef             history);
  virtual ~CopyToNextFrameHCI();

  void setEnabled(bool b);
  bool canReleaseFocus();

  // interactor methods
  void motion(MinVR::EventRef e);
  void on(MinVR::EventRef e);
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
  G3D::Array<MarkBox>         _intersectedBoxes;
  MarkBox                _selectedBox;
  G3D::KDTree<MarkBox>     _tree;
  int                    _treeFrame;
  int                    _drawid;

private:
  CopyToNextFrameHCI(const CopyToNextFrameHCI &);
  CopyToNextFrameHCI& operator=(const CopyToNextFrameHCI &);
};


} // end namespace
#endif
