
#include <ConfigVal.H>
#include <G3DOperators.h>
#include "KbdSelectWidget.H"

#include "BrushState.H"
using namespace G3D;
namespace DrawOnAir {

KbdSelectWidget::KbdSelectWidget(ArtworkRef artwork, MinVR::EventMgrRef eventMgr,
  HCIMgrRef hciMgr) : WidgetHCI(hciMgr)
{
  _artwork = artwork;
  _eventMgr = eventMgr;
  _selectedMark = 0;

  _fsa = new MinVR::Fsa("KbdSelectWidget");
  _fsa->addState("Start");
  _fsa->addState("Naming");
  _fsa->addArc("Next", "Start", "Start", MinVR::splitStringIntoArray("kbd_RIGHT_down"));
  _fsa->addArcCallback("Next", this, &KbdSelectWidget::selectNextMark);
  _fsa->addArc("Last", "Start", "Start", MinVR::splitStringIntoArray("kbd_LEFT_down"));
  _fsa->addArcCallback("Last", this, &KbdSelectWidget::selectLastMark);
  _fsa->addArc("Save", "Start", "Start", MinVR::splitStringIntoArray("kbd_S_down"));
  _fsa->addArcCallback("Save", this, &KbdSelectWidget::saveMark);
  _fsa->addArc("Del", "Start", "Start", MinVR::splitStringIntoArray("kbd_D_down"));
  _fsa->addArcCallback("Del", this, &KbdSelectWidget::deleteMark);
  _fsa->addArc("Name", "Start", "Naming", MinVR::splitStringIntoArray("kbd_N_down"));
  _fsa->addArcCallback("Name", this, &KbdSelectWidget::startTypeName);
  _fsa->addArc("Type", "Naming", "Naming", MinVR::splitStringIntoArray("ALL_STANDARD"));
  _fsa->addArcCallback("Type", this, &KbdSelectWidget::typeName);
  _fsa->addArc("End", "Start", "Start", MinVR::splitStringIntoArray("kbd_M_down"));
  _fsa->addArcCallback("End", this, &KbdSelectWidget::deactivate);
}

KbdSelectWidget::~KbdSelectWidget()
{
}

void
KbdSelectWidget::activate()
{
  _artwork->getMarks()[_selectedMark]->setHighlighted(true, _artwork);
  _eventMgr->addFsaRef(_fsa);
}

void
KbdSelectWidget::deactivate(MinVR::EventRef e)
{
  _artwork->getMarks()[_selectedMark]->setHighlighted(false, _artwork);
  _eventMgr->removeFsaRef(_fsa);
  releaseFocusWithHCIMgr();
}

void
KbdSelectWidget::selectNextMark(MinVR::EventRef e)
{
  Array<MarkRef> m = _artwork->getMarks();
  if (m.size()) {
    m[_selectedMark]->setHighlighted(false, _artwork);
    _selectedMark++;
    if (_selectedMark >= m.size())
      _selectedMark = 0;
    m[_selectedMark]->setHighlighted(true, _artwork);
    cout << "Selected mark: " << _artwork->getMarks()[_selectedMark]->getName() << endl;
  }
}

void
KbdSelectWidget::selectLastMark(MinVR::EventRef e)
{
  Array<MarkRef> m = _artwork->getMarks();
  if (m.size()) {
    m[_selectedMark]->setHighlighted(false, _artwork);
    _selectedMark--;
    if (_selectedMark < 0)
      _selectedMark = m.size()-1;
    m[_selectedMark]->setHighlighted(true, _artwork);
    cout << "Selected mark: " << _artwork->getMarks()[_selectedMark]->getName() << endl;
  }
}

void
KbdSelectWidget::saveMark(MinVR::EventRef e)
{
  std::string fname = _artwork->getMarks()[_selectedMark]->getName();
  std::string base  = MinVR::ConfigVal("SaveBasePath", "share/artwork",false) + "/" + fname + "_";
  std::string extension = ".3DArt";
  int c=0;
  while (FileSystem::exists(base + MinVR::intToString(c) + extension)) {
    c++;
  }
  std::string filename = base + MinVR::intToString(c) + extension;

  //cout << "Saving Mark to file: " << filename << endl;
  //std::string xml = _artwork->getMarks()[_selectedMark]->writeToXML();
  //writeStringToFile(xml, filename);
}

void
KbdSelectWidget::deleteMark(MinVR::EventRef e)
{
  MarkRef m = _artwork->getMarks()[_selectedMark];
  _artwork->removeMark(m);
  if (_selectedMark >= _artwork->getMarks().size()) {
    _selectedMark = 0;
  }
  _artwork->getMarks()[_selectedMark]->setHighlighted(true, _artwork);
}

void
KbdSelectWidget::startTypeName(MinVR::EventRef e)
{
  _name = "";
}

void
KbdSelectWidget::typeName(MinVR::EventRef e)
{
  std::string key;
  if (beginsWith(e->getName(), "kbd_")) {
    if (e->getName() == "kbd_ESC_down") {
      _fsa->jumpToState("Start");
      cout << "Name change cancelled." << endl;
    }
    else if (e->getName() == "kbd_ENTER_down") {
      _fsa->jumpToState("Start");
      _artwork->getMarks()[_selectedMark]->setName(_name);
      cout << "Set name to " << _name << endl;
    }
    else if (e->getName() == "kbd_BACKSPACE_down") {
      if (_name.length()) {
        _name = _name.substr(0, _name.length()-1);
        cout << "Name = " << _name << endl;
      }
    }
    else if (endsWith(e->getName(), "_down")) {
      // Get a string from the keypress event
      int stop = e->getName().find("_down");
      key = e->getName().substr(0,stop);
      int i=key.length()-1;
      while ((i>=0) && (key[i] != '_')) {
        i--;
      }
      key = key.substr(i+1);
      if ((key == "SHIFT") || (key == "CTRL")) {
        key = ""; 
      }
      else {
        if (key == "SPACE") {
          key = " ";
        }    
        else if (e->getName().find("SHIFT") == e->getName().npos) {
          key = toLower(key);
        }
      }

      _name = _name + key;
      cout << "Name = " << _name << endl;
    }
  }
}

} // end namespace

