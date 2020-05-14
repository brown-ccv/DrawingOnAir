#ifdef USE_OPENAL

#include "SoundOpenAL.H"


static void reportError(void) {
  fprintf(stderr, "ALUT error: %s\n", alutGetErrorString(alutGetError()));
}

 

SoundOpenAL::SoundOpenAL()
{
  ALint argc = 0;
  #ifdef WIN32
    ALbyte **argv = NULL;
  #else
    char **argv = NULL;
  #endif
    if (!alutInit(0, NULL)) {
          //&argc, argv)) {
    reportError();
  }
}

SoundOpenAL::~SoundOpenAL()
{
  if (!alutExit()) {
    reportError();
  }
}


void
SoundOpenAL::setListenerLoc(const CoordinateFrame &m)
{
  ALfloat p[3];
  p[0] = m.translation[0];
  p[1] = m.translation[1];
  p[2] = m.translation[2];
  alListenerfv(AL_POSITION,p);
  
  Vector3 at = m.lookVector();
  // this obviously isn't quite right
  Vector3 up = Vector3(0,-1,0);
  ALfloat ori[6];
  ori[0] = at[0];
  ori[1] = at[1];
  ori[2] = at[2];
  ori[3] = up[0];
  ori[4] = up[1];
  ori[5] = up[2];
  alListenerfv(AL_ORIENTATION,ori);
}

void
SoundOpenAL::setListenerVel(const Vector3 &v)
{
  ALfloat vel[3];
  vel[0] = v[0];
  vel[1] = v[1];
  vel[2] = v[2];
  alListenerfv(AL_VELOCITY,vel);
}

void
SoundOpenAL::setListenerGain(const float g)
{
  alListenerf(AL_GAIN,(ALfloat)g);
}


void
SoundOpenAL::load(const std::string &filename)
{
  std::string fnamefull = decygifyPath(replaceEnvVars(filename));

  // return immediately if this file has already been loaded!
  if (_fnames.contains(fnamefull))
    return;

  cout << "SoundOpenAL: Loading " << fnamefull << endl;

  /**
  ALuint buffer = alutCreateBufferFromFile(filename.c_str());
  if (buffer == AL_NONE) {
    reportError();
  }
  ALuint source;
  alGenSources(1, &source);
  alSourcei(source, AL_BUFFER, buffer);
  _sources.append(source);
  _fnames.append(fnamefull);
  **/

  // return immediately if this file has already been loaded!
  if (_fnames.contains(fnamefull))
    return;
  
  ALenum    format;
  ALvoid    *data = NULL;
  ALsizei   size;
  ALsizei   freq;
  ALint     error;
  ALuint    buf;
  ALboolean loopy;
  
  // load wav file
  
  cout << "Loading sound: " << fnamefull << flush;

#ifdef WIN32
  alutLoadWAVFile((ALbyte*)fnamefull.c_str(), &format, &data, &size, &freq, &loopy);
#else
  ALsizei bits;  // ????
  alutLoadWAV(fnamefull.c_str(), &data, &format, &size, &bits, &freq);
#endif
  cout << endl;

  if ((error = alGetError()) != AL_NO_ERROR) {
    cerr << "Open AL Error " << fnamefull << ": " << alGetString(error) << endl;
    return;
  }

  if (!data) {
    cerr << "Unable to load sound file: " << fnamefull << endl;
    return;
  }

  // generate a buffer
  alGenBuffers(1, &buf);
  if ((error = alGetError()) != AL_NO_ERROR) {
    cerr << "Open AL Error " << filename << ": " << alGetString(error) << endl;
    return;
  }
  
  // copy wav data into an AL buffer
  alBufferData(buf, format, data, size, freq);
  if ((error = alGetError()) != AL_NO_ERROR) {
    cerr << "Open AL Error " << filename << ": " << alGetString(error) << endl;
    alDeleteBuffers(1,&buf);
    alutUnloadWAV(format, data, size, freq);
    return;
  }

#ifdef LINUX
  if (data)
    free(data);
#else
  // unload the wav data
  alutUnloadWAV(format, data, size, freq);
  if ((error = alGetError()) != AL_NO_ERROR) {
    cerr << "Open AL Error " << filename << ": " << alGetString(error) << endl;
    alDeleteBuffers(1,&buf);
    return;
  }
#endif


  // generate an AL source
  ALuint source;
  alGenSources(1, &source);
  if ((error = alGetError()) != AL_NO_ERROR) {
    cerr << "Open AL Error " << filename << ": " << alGetString(error) << endl;
    alDeleteBuffers(1,&buf);
    return;
  }
  
  alSourcei(source, AL_BUFFER, buf);

  ALfloat zeros[]   = { 0.0f, 0.0f,  0.0f };
  alSourcefv(source, AL_POSITION, zeros);
  alSourcefv(source, AL_VELOCITY, zeros );
  //alSourcefv(source, AL_ORIENTATION, back );
  alSourcef(source, AL_PITCH, 1.0f ); 
  //alSourcef(source, AL_GAIN,1.0f);

  _fnames.append(fnamefull);
  _sources.append(source);
  assert(_sources.size() == _fnames.size()); 
}


void
SoundOpenAL::play(const std::string &filename)
{
  std::string fnamefull = decygifyPath(replaceEnvVars(filename));
  cout << "SoundOpenAL: Playing " << fnamefull << endl;
  // load the sound if not already loaded
  load(fnamefull);
  alSourcePlay(getOpenALSource(fnamefull));
}


void
SoundOpenAL::stop(const std::string &filename)
{
  alSourceStop(getOpenALSource(filename));
}

void
SoundOpenAL::deleteSound(const std::string &filename)
{
  int index = _fnames.findIndex(filename);
  if (index >= 0) {
    alDeleteSources(1,&(_sources[index]));
    _sources.remove(index);
    _fnames.remove(index);
  }
}

/***
void
SoundOpenAL::setParam(const std::string &filename, const SOUNDPARAM param, 
			 const float value)
{
  // make sure sound is loaded.
  load(filename);

  ALfloat val = value;
  ALuint source = getOpenALSource(filename);
  ALfloat vec[3];
  
  switch (param) {
  case SP_LOOP:
    if (value == 0.0)
      alSourcei(source, AL_LOOPING, AL_FALSE);
    else
      alSourcei(source, AL_LOOPING, AL_TRUE);
    break;
  case SP_PITCH:
    alSourcef(source, AL_PITCH, val);
    break;
  case SP_GAIN:
    alSourcef(source, AL_GAIN, val);
    break;
  case SP_POSX:
    alGetSourcefv(source, AL_POSITION, vec);
    vec[0] = val;
    alSourcefv(source, AL_POSITION, vec);
    break;
  case SP_POSY:
    alGetSourcefv(source, AL_POSITION, vec);
    vec[1] = val;
    alSourcefv(source, AL_POSITION, vec);
    break;
  case SP_POSZ:
    alGetSourcefv(source, AL_POSITION, vec);
    vec[2] = val;
    alSourcefv(source, AL_POSITION, vec);
    break;
  case SP_VELX:
    alGetSourcefv(source, AL_VELOCITY, vec);
    vec[0] = val;
    alSourcefv(source, AL_VELOCITY, vec);
    break;
  case SP_VELY:
    alGetSourcefv(source, AL_VELOCITY, vec);
    vec[1] = val;
    alSourcefv(source, AL_VELOCITY, vec);
    break;
  case SP_VELZ:
    alGetSourcefv(source, AL_VELOCITY, vec);
    vec[2] = val;
    alSourcefv(source, AL_VELOCITY, vec);
    break;
  }
}
**/

ALuint
SoundOpenAL::getOpenALSource(const std::string &filename)
{
  ALuint i = 0;
  int index = _fnames.findIndex(filename);
  if (index >= 0) {
    return _sources[index];
  }
  cerr << "SoundOpenAL:: no source for " << filename << endl;
  return 9999;
}


#endif  // USE_OPENAL
