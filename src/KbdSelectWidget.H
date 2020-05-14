/**
 * /author Dan Keefe (dfk)
 *
 * /file  KbdSelectWidget.H
 * /brief 
 *
 */ 

#ifndef KBDSELECTWIDGET_H
#define KBDSELECTWIDGET_H

//#include "VRBase.H"
#include <Event.h>
#include <EventMgr.H>

#include "Artwork.H"
#include "HCIMgr.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class KbdSelectWidget> KbdSelectWidgetRef;
/**  
*/
class KbdSelectWidget : public WidgetHCI
{
public:

  KbdSelectWidget(ArtworkRef artwork, MinVR::EventMgrRef eventMgr, HCIMgrRef hciMgr);  
  virtual ~KbdSelectWidget();

  void activate();

  void selectNextMark(MinVR::EventRef e);
  void selectLastMark(MinVR::EventRef e);
  void deleteMark(MinVR::EventRef e);
  void saveMark(MinVR::EventRef e);
  void deactivate(MinVR::EventRef e);
  
  void startTypeName(MinVR::EventRef e);
  void typeName(MinVR::EventRef e);

private:
  ArtworkRef  _artwork;
  MinVR::EventMgrRef _eventMgr;
  MinVR::FsaRef      _fsa;
  int         _selectedMark;
  std::string _name;

private:
  KbdSelectWidget(const KbdSelectWidget &);
  KbdSelectWidget& operator=(const KbdSelectWidget &);
};


} // end namespace

#endif

