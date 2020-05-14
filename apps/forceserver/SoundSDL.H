/**
 * \author Daniel Keefe (dfk)
 *
 * \file  SoundSDL.H
 * \brief A SDL-based implementation of a sound device
 *
 */

#ifndef SOUNDSDL_H
#define SOUNDSDL_H

#include <DrawOnAir.H>
#include "SoundBase.H"

/**
*/
class SoundSDL : public SoundBase
{
public:
  SoundSDL();
  virtual ~SoundSDL();

  virtual void play(const std::string &filename); 
};


#endif

