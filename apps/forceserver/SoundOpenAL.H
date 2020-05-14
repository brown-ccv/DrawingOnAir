/**
 * \author Daniel Keefe (dfk)
 *
 * \file  SoundImpOpenAL.H
 * \brief An OpenAL-based implementation of a sound device to be called by
 *        SOUNDMGR.  
 *
 */

#ifdef USE_OPENAL

#ifndef SOUNDOPENAL
#define SOUNDOPENAL

#include <AL/alut.h>
#include "SoundBase.H"

class SoundOpenAL : public SoundBase
{
public:
  SoundOpenAL();
  virtual ~SoundOpenAL();

  virtual void play(const std::string &filename);

  void setListenerLoc(const CoordinateFrame &m);
  void setListenerVel(const Vector3 &v);
  void setListenerGain(const float g);

  void load(const std::string &filename);
  void stop(const std::string &filename);
  void deleteSound(const std::string &filename);
  //  void setParam(const std::string &filename, const SOUNDPARAM param, 
  //		const float value);

protected:
 
  unsigned int getOpenALSource(const std::string &filename);

  Array<ALuint> _sources;
  Array<std::string>  _fnames;
 
};


#endif

#endif // USE_OPENAL
