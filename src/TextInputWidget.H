/**
 * /author Dan Keefe (dfk)
 *
 * /file  TextInputWidget.H
 * /brief 
 *
 */ 

#ifndef TEXTINPUTWIDGET_H
#define TEXTINPUTWIDGET_H

//#include "VRBase.H"
#include <VRG3DBaseApp.h>
#include "EventMgr.H"
#include "GfxMgr.H"
#include "HCIMgr.H"

namespace DrawOnAir {

typedef G3D::ReferenceCountedPointer<class TextInputWidget> TextInputWidgetRef;
/**  
*/
class TextInputWidget : public WidgetHCI
{
public:

  TextInputWidget(GfxMgrRef              gfxMgr,
                  EventMgrRef            eventMgr,
                  HCIMgrRef              hciMgr,
                  G3D::Array<std::string>     enterEvents);
  
  virtual ~TextInputWidget();

  void setText(const std::string &text) { _text = text; }

  void activate();
  
  void draw(G3D::RenderDevice *rd, const G3D::CoordinateFrame &virtualToRoomSpace);

  void keyPressed(MinVR::EventRef e);
  void enterText(MinVR::EventRef e);

protected:

  void deactivate();

  EventMgrRef            _eventMgr;
  FsaRef                 _fsa;
  G3D::GFontRef               _font;
  GfxMgrRef              _gfxMgr;
  EventFilterRef         _kbdFilter;
  std::string            _text;
  int                    _drawID;
  bool                   _shouldDraw;

private:
  TextInputWidget(const TextInputWidget &);
  TextInputWidget& operator=(const TextInputWidget &);
};


} // end namespace

#endif

