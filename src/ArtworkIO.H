/**
 * /author Dan Keefe (dfk)
 *
 * /file  ArtworkIO.H
 * /brief 
 *
 */ 

#ifndef ARTWORKIO_H
#define ARTWORKIO_H

//#include "VRBase.H"
#include <G3D//G3D.h>
#include <GLG3D/GLG3D.h>
#include "Artwork.H"
#include "BrushTipPicker.H"
#include "ColorSwatchPicker.H"
#include "PatternPicker.H"


namespace DrawOnAir {

class ArtworkIO
{
public:
  static void serialize(G3D::BinaryOutput &b,
                        ArtworkRef artwork, 
                        G3D::Color3 backgroundColor,
                        ColorSwatchPickerRef colorPicker,
                        BrushTipPickerRef brushTipPicker,
                        PatternPickerRef patternPicker);

  static void deserialize(G3D::BinaryInput &b,
                          ArtworkRef artwork, 
                          G3D::Color3 &backgroundColor,
                          ColorSwatchPickerRef colorPicker,
                          BrushTipPickerRef brushTipPicker,
                          PatternPickerRef patternPicker);

};

} // end namespace

#endif

