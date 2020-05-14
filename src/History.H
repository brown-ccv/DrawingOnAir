/**
 * /author Dan Keefe (dfk)
 *
 * /file  History.H
 * /brief History for Undo and Redo operations
 *
 */ 

#ifndef HISTORY_H
#define HISTORY_H

//#include "VRBase.H"
//#include <VRG3DBaseApp.h>
#include <EventMgr.H>
#include <GfxMgr.H>

#include "Artwork.H"
#include "Mark.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class History> HistoryRef;

class History : public G3D::ReferenceCountedObject
{ 
public: 

  class HistoryRecord {
  public:

    enum HistoryRecordType {
      UNKNOWN,
      MARK,
      REFRAME,
      DELETEMARK
    };
    
    HistoryRecord() { type = UNKNOWN; };
    virtual ~HistoryRecord() {};

    static std::string typeToString(HistoryRecordType type);
    
    HistoryRecordType type;
    double            time;
    MarkRef           mark;
    ArtworkRef        artwork;
    G3D::CoordinateFrame   origR2VSFrame;
    double            origR2VSScale;
  };
  
  History(MinVR::EventMgrRef eventMgr,  MinVR::GfxMgrRef gfxMgr);
  virtual ~History();

  /// Returns a string describing the next operation in the undo
  /// queue.  If the queue is empty, returns the string "None".
  std::string getUndoDescription();
  std::string getRedoDescription();

  bool getNextUndo(HistoryRecord &rec);

  /// Undoes the next operation in the undo queue, returns false if
  /// the queue is empty.
  bool undo();
  bool redo();

  // Routines to store records about things that can be undone..
  void storeMarkCreated(MarkRef mark, ArtworkRef artwork);
  void storeMarkDeleted(MarkRef mark, ArtworkRef artwork);
  void storeReframeStarted(G3D::CoordinateFrame origRoomToVSFrame,
                           double          origRoomToVSScale);
  
private:
  History(const History &);
  History& operator=(const History &);

  MinVR::EventMgrRef          _eventMgr;
  MinVR::GfxMgrRef            _gfxMgr;
  G3D::Array<HistoryRecord> _undoQueue;
  G3D::Array<HistoryRecord> _redoQueue;
};

} // end namespace

#endif



