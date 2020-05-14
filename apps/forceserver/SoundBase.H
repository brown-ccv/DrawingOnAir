/**
 * \author Daniel Keefe (dfk)
 *
 * \file  SoundBase.H
 * \brief Base class for various implementations of sound.
 *
 */

#ifndef SOUNDBASE_H
#define SOUNDBASE_H

#include <DrawOnAir.H>
using namespace G3D;

typedef ReferenceCountedPointer<class SoundBase> SoundBaseRef;
/**
*/
class SoundBase : public ReferenceCountedObject
{
public:
  SoundBase() {};
  virtual ~SoundBase() {};

  virtual void play(const std::string &filename) {
    cerr << "Warning: Cannot play sound, need to reimplent SoundBase." << endl;
  }; 
};


#endif

