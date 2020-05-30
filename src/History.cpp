
#include <G3DOperators.h>
#include <SynchedSystem.h>
#include "History.H"
using namespace G3D;
#include "BrushState.H"


namespace DrawOnAir {

History::History(EventMgrRef eventMgr, GfxMgrRef gfxMgr)
{
  _eventMgr = eventMgr;
  _gfxMgr = gfxMgr;
}

History::~History()
{
}

std::string
History::HistoryRecord::typeToString(HistoryRecordType type)
{
  std::string desc;
  switch (type) {
  case MARK:
    desc = "Mark drawn";
    break;
  case REFRAME:
    desc = "Reframe artwork";
    break;
  case DELETEMARK:
    desc = "Delete mark";
    break;
  }
  return desc;
}

std::string
History::getUndoDescription()
{
  if (_undoQueue.size()) {
    HistoryRecord rec = _undoQueue.last();
    std::string desc = HistoryRecord::typeToString(rec.type) +
      " " + MinVR::intToString(iRound(MinVR::SynchedSystem::getLocalTime() - rec.time)) +
      " seconds ago.";
    return desc;
  }
  else
    return std::string("None");
}

std::string
History::getRedoDescription()
{
  if (_redoQueue.size()) {
    HistoryRecord rec = _redoQueue.last();
    std::string desc = HistoryRecord::typeToString(rec.type) +
      " " + MinVR::intToString(iRound(MinVR::SynchedSystem::getLocalTime() - rec.time)) +
      " seconds ago.";
    return desc;
  }
  else
    return std::string("None");
}


bool
History::getNextUndo(HistoryRecord &rec)
{
  if (_undoQueue.size()) {
    rec = _undoQueue.last();
    return true;
  }
  return false;
}

bool
History::undo()
{
  if (!_undoQueue.size())
    return false;

  HistoryRecord rec = _undoQueue.pop();
  _redoQueue.clear();
  _redoQueue.append(rec);
  switch (rec.type) {
  case HistoryRecord::MARK:
    rec.artwork->removeMark(rec.mark);
    break;
  case HistoryRecord::REFRAME:
    _gfxMgr->setRoomToVirtualSpaceScale(rec.origR2VSScale);
    _gfxMgr->setRoomToVirtualSpaceFrame(rec.origR2VSFrame);
    break;
  case HistoryRecord::DELETEMARK:
    rec.artwork->addMark(rec.mark);
    break;
  }
  return true;
}

bool
History::redo()
{
  if (!_redoQueue.size())
    return false;

  HistoryRecord rec = _redoQueue.pop();
  _undoQueue.append(rec);
  switch (rec.type) {
  case HistoryRecord::MARK:
    rec.artwork->addMark(rec.mark);
    break;
  case HistoryRecord::REFRAME:
    _gfxMgr->setRoomToVirtualSpaceScale(rec.origR2VSScale);
    _gfxMgr->setRoomToVirtualSpaceFrame(rec.origR2VSFrame);
    break;
  case HistoryRecord::DELETEMARK:
    rec.artwork->removeMark(rec.mark);
    break;
  }
  return true;
}


void
History::storeMarkCreated(MarkRef mark, ArtworkRef artwork)
{
  HistoryRecord rec;
  rec.type = HistoryRecord::MARK;
  rec.time = MinVR::SynchedSystem::getLocalTime();
  rec.mark = mark;
  rec.artwork = artwork;
  _undoQueue.append(rec);
}

void
History::storeReframeStarted(CoordinateFrame origRoomToVSFrame,
                             double          origRoomToVSScale)
{
  HistoryRecord rec;
  rec.type = HistoryRecord::REFRAME;
  rec.time = MinVR::SynchedSystem::getLocalTime();
  rec.origR2VSFrame = origRoomToVSFrame;
  rec.origR2VSScale = origRoomToVSScale;
  _undoQueue.append(rec);
}

void
History::storeMarkDeleted(MarkRef mark, ArtworkRef artwork)
{
  HistoryRecord rec;
  rec.type = HistoryRecord::DELETEMARK;
  rec.time = MinVR::SynchedSystem::getLocalTime();
  rec.mark = mark;
  rec.artwork = artwork;
  _undoQueue.append(rec);
}


} // end namespace

