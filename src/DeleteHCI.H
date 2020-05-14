/**
 * \author Dan Keefe (dfk)
 *
 * \file  DeleteHCI.H
 * \brief
 *
 */


#ifndef DELETEHCI_H
#define DELETEHCI_H

#include "Selection.H"

//#include "VRBase.H"
//#include <VRG3DBaseApp.h>

#include <EventMgr.H>
#include <GfxMgr.H>
#include <Fsa.H>

#include "Artwork.H"
#include "BrushState.H"
#include "CavePaintingCursors.H"
#include "HCIMgr.H"
#include "History.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class DeleteHCI> DeleteHCIRef;
/** 
*/
class DeleteHCI : public StylusHCI
{
public:
  DeleteHCI(G3D::Array<std::string>     motionTriggers, 
            G3D::Array<std::string>     onTriggers,
            G3D::Array<std::string>     offTriggers,
            ArtworkRef             artwork,
            BrushRef               brush,
            CavePaintingCursorsRef cursors,
            MinVR::EventMgrRef            eventMgr,
            MinVR::GfxMgrRef              gfxMgr,
            HistoryRef             history);
  virtual ~DeleteHCI();

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
  MinVR::EventMgrRef            _eventMgr;
  MinVR::FsaRef                 _fsa;
  MinVR::GfxMgrRef              _gfxMgr;
  HistoryRef             _history;
  MarkRef                _selectedMark;
  bool                   _enabled;
  G3D::Array<MarkBox>         _intersectedBoxes;
  MarkBox                _selectedBox;
  G3D::KDTree<MarkBox>     _tree;
  int                    _treeFrame;
  int                    _drawid;

private:
  DeleteHCI(const DeleteHCI &);
  DeleteHCI& operator=(const DeleteHCI &);
};


} // end namespace
#endif
