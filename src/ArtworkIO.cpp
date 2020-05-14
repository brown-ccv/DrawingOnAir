
#include "ArtworkIO.H"
#include <G3DOperators.h>

using namespace G3D;
namespace DrawOnAir {


/** Each time you change the format of this, increase the
    SERIALIZE_VERSION_NUMBER and respond accordingly in the
    deserialize method.  In this method, you can safely get rid of the
    old code because we don't want to create files in an old format.
*/
#define SERIALIZE_VERSION_NUMBER 0
void
ArtworkIO::serialize(G3D::BinaryOutput &b,
                     ArtworkRef artwork, 
                     Color3 backgroundColor,
                     ColorSwatchPickerRef colorPicker,
                     BrushTipPickerRef brushTipPicker,
                     PatternPickerRef patternPicker)
{
  b.writeInt8(SERIALIZE_VERSION_NUMBER);

  std::string coltexname = "Default";
  if (colorPicker.notNull()) {
    coltexname = colorPicker->getCurrentPaletteName();
  }
  std::string btiptexname = "Default";
  if (brushTipPicker.notNull()) {
    btiptexname = brushTipPicker->getCurrentPaletteName();
  }
  std::string pattexname = "Default";
  if (patternPicker.notNull()) {
    pattexname = patternPicker->getCurrentPaletteName();
  }

  artwork->serialize(b);
  backgroundColor.serialize(b);
  b.writeString(coltexname);
  b.writeString(btiptexname);
  b.writeString(pattexname);
}

/** Never delete the code from an old version from this method so we
    can always stay backwards compatable.  Important, since this is in
    binary, and we can't really reverse engineer the file format.
*/
void
ArtworkIO::deserialize(G3D::BinaryInput &b,
                       ArtworkRef artwork, 
                       Color3 &backgroundColor,
                       ColorSwatchPickerRef colorPicker,
                       BrushTipPickerRef brushTipPicker,
                       PatternPickerRef patternPicker)
{
  int version = b.readInt8();

  if (version == 0) {

    artwork->deserialize(b);
    backgroundColor.deserialize(b);
    std::string coltexname = b.readString();
    std::string btiptexname = b.readString();
    std::string pattexname = b.readString();
    
    if (colorPicker.notNull()) {
      colorPicker->setCurrentPaletteByName(coltexname);
    }
    if (brushTipPicker.notNull()) {
      brushTipPicker->setCurrentPaletteByName(btiptexname);
    }
    if (patternPicker.notNull()) {
      patternPicker->setCurrentPaletteByName(pattexname);
    }

  }
  else {
    alwaysAssertM(false, "Unrecognized ArtworkIO version " + MinVR::intToString(version));
  }
}



} // end namespace
