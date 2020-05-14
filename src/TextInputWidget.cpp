
#include "TextInputWidget.H"
#include <EventFilter.H>
#include <EventMgr.H>
#include <G3DOperators.h>

using namespace G3D;
namespace DrawOnAir {

/** This eats all keyboard events and sends out new message events of
    name newEventName with the name of the original keyboard event
    stored in the message.  This is useful for trapping keyboard input
    so that a Fsa can respond to text input from the whole keyboard
    without accidentally activating other Fsa's that respond to
    particular keyboard press events.
 */
class KbdFilter : public MinVR::EventFilter
{
public:
  KbdFilter(const std::string &newEventName, MinVR::EventMgrRef eventMgr) { 
    _eName = newEventName; 
    _eventMgr = eventMgr;
  }
  virtual ~KbdFilter() {};

  virtual bool filter(MinVR::EventRef e) {
    if (beginsWith(e->getName(), "kbd_")) {
      if (endsWith(e->getName(), "_down")) {
        int stop = e->getName().find("_down");
        std::string key = e->getName().substr(0,stop);
        int i=key.length()-1;
        while ((i>=0) && (key[i] != '_')) {
          i--;
        }
        key = key.substr(i+1);
        if ((key == "SHIFT") || (key == "CTRL")) {
          return false;
        }

        if (key == "SPACE") {
          key = " ";
        }
        
        if (e->getName().find("SHIFT") != e->getName().npos) {
          _eventMgr->queueEvent(new MinVR::Event(_eName, key));
        }
        else {
          _eventMgr->queueEvent(new MinVR::Event(_eName, toLower(key)));
        }
      }
      return false;
    }
    else {
      return true;
    }
  }

private:
  MinVR::EventMgrRef _eventMgr;
  std::string _eName;
};




TextInputWidget::TextInputWidget(MinVR::GfxMgrRef              gfxMgr,
                                 MinVR::EventMgrRef            eventMgr,
                                 HCIMgrRef              hciMgr,
                                 Array<std::string>     enterEvents) :
  WidgetHCI(hciMgr)
{
  _gfxMgr = gfxMgr;
  _eventMgr = eventMgr;
  _shouldDraw = false;
  
  _fsa = new MinVR::Fsa("TextInputWidget");
  _fsa->addState("Start");
  _fsa->addArc("TextIn", "Start", "Start", MinVR::splitStringIntoArray("TextInput"));
  _fsa->addArcCallback("TextIn", this, &TextInputWidget::keyPressed);
  _fsa->addArc("Enter", "Start", "Start", enterEvents);
  _fsa->addArcCallback("Enter", this, &TextInputWidget::enterText);
  
  _kbdFilter = new KbdFilter("TextInput", _eventMgr);
}

TextInputWidget::~TextInputWidget()
{
}

void
TextInputWidget::activate()
{
  _text = "";
  _eventMgr->addFsaRef(_fsa);
  _eventMgr->addEventFilter(_kbdFilter);
  //_drawID = _gfxMgr->addDrawCallback(this, &PickerWidget::draw);
  _shouldDraw = true;
}

void
TextInputWidget::deactivate()
{
  releaseFocusWithHCIMgr();
  _eventMgr->removeFsaRef(_fsa);
  _eventMgr->removeEventFilter(_kbdFilter);
  //_gfxMgr->removeDrawCallback(_drawID);
  _shouldDraw = false;
}


void
TextInputWidget::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  if (!_shouldDraw) {
    return;
  }

  Vector2 size = Vector2(200, 70);
  Vector2 pos  = Vector2(rd->width() / 2.0, rd->height() / 2.0);
  GFontRef font = _gfxMgr->getDefaultFont();

  Color4 outlineCol  = Color4(1.0,1.0,1.0,1.0);
  Color4 bgCol       = Color4(0.1,0.1,0.44,1.0);
  Color4 barCol      = Color4(0.61,0.72,0.92,1.0);
  Color4 titleCol    = Color4(1.0,1.0,1.0,1.0);
  Color4 percCol     = Color4(0.8,0.22,0.0,1.0);
  
  Vector2 topl = Vector2(pos[0]-size[0]/2.0, pos[1]-size[1]/2.0);
  Vector2 topr = Vector2(pos[0]+size[0]/2.0, pos[1]-size[1]/2.0);
  Vector2 botl = Vector2(pos[0]-size[0]/2.0, pos[1]+size[1]/2.0);
  Vector2 botr = Vector2(pos[0]+size[0]/2.0, pos[1]+size[1]/2.0);

  Vector2 titleCtr = pos - Vector2(0, size[1] / 6.0);
  Vector2 textCtr   = pos + Vector2(0, size[1] / 6.0);

  rd->push2D();
  rd->setTexture(0,NULL);
  rd->setBlendFunc(RenderDevice::BLEND_ONE, RenderDevice::BLEND_ZERO);

  // outline
  rd->setColor(outlineCol);
  rd->beginPrimitive(PrimitiveType::QUADS);
  rd->sendVertex(topl);
  rd->sendVertex(botl);
  rd->sendVertex(botr);
  rd->sendVertex(topr);
  rd->endPrimitive();

  // background
  rd->setColor(bgCol);
  rd->beginPrimitive(PrimitiveType::QUADS);
  rd->sendVertex(Vector2(topl[0] + 2, topl[1] + 2));
  rd->sendVertex(Vector2(botl[0] + 2, botl[1] - 2));
  rd->sendVertex(Vector2(botr[0] - 2, botr[1] - 2));
  rd->sendVertex(Vector2(topr[0] - 2, topr[1] + 2));
  rd->endPrimitive();

  // input box
  float left  = textCtr[0] - (size[0]-20)/2.0;
  float right = textCtr[0] + (size[0]-20)/2.0;
  rd->setColor(outlineCol);
  rd->beginPrimitive(PrimitiveType::QUADS);
  rd->sendVertex(Vector2(left, textCtr[1]-size[1]/6.0));
  rd->sendVertex(Vector2(left, textCtr[1]+size[1]/6.0));
  rd->sendVertex(Vector2(right, textCtr[1]+size[1]/6.0));
  rd->sendVertex(Vector2(right, textCtr[1]-size[1]/6.0));
  rd->endPrimitive();
  
  // title
  font->draw2D(rd, "Type Text then press Enter",
               titleCtr, 12, titleCol, Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);

  // text input
  font->draw2D(rd, _text, textCtr, 12, percCol, Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);

  rd->pop2D();
}

void
TextInputWidget::keyPressed(MinVR::EventRef e)
{
  std::string key = e->getMsgData();
  if ((key == "ENTER") || (key == "enter")) {
    _eventMgr->queueEvent(new MinVR::Event("TextInputWidget_InputDone", _text));
    deactivate();
  }
  else if ((key == "BACKSPACE") || (key == "backspace")) {
    _text = _text.substr(0, _text.length()-1);
    _eventMgr->queueEvent(new MinVR::Event("TextInputWidget_Input", _text));
  }
  else if ((key != "UNKNOWN") && (key != "unknown")) {
    _text = _text + key;
    _eventMgr->queueEvent(new MinVR::Event("TextInputWidget_Input", _text));
  }
}

void
TextInputWidget::enterText(MinVR::EventRef e)
{
  _eventMgr->queueEvent(new MinVR::Event("TextInputWidget_InputDone", _text));
  deactivate();
}


} // end namespace

