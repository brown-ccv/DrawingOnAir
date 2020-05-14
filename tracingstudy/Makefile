
include $(G)/src/gbuild/Makefile.gbuild.init

PROJECT_NAME = tracingstudy

# See $G/src/G3D/VRG3D/Makefile.lib for a description of DEFINES
# relevant to VRG3D.
DEFINES=NO_SDL_MAIN USE_VRPN USE_SPACENAV
-include Makefile.localdefines


G3D_VER  =-7.27.07
VRPN_VER =7.15

INCLUDE_DIRS = $(G)/src/G3D/G3D$(G3D_VER)/include \
               $(G)/src/vrpn/$(VRPN_VER) \
               $(G)/src/VRG3D \
               $(G)/src/vrbase \
               ../lib

LIBS       = VRG3D vrpn quat vrbase drawonair
DEBUG_LIBS = GLG3Dd G3Dd
OPT_LIBS   = GLG3D  G3D

LIB_DIRS       = $(G)/src/G3D/G3D$(G3D_VER)/$(GBUILDSTR)

DEBUG_LIB_DIRS = $(G)/src/vrpn/$(VRPN_VER)/vrpn/build/client/$(GBUILDSTR)/Debug \
                 $(G)/src/vrpn/$(VRPN_VER)/quat/build/$(GBUILDSTR)/Debug \
                 $(G)/src/VRG3D/build/lib/$(GBUILDSTR)/Debug \
                 $(G)/src/vrbase/build/$(GBUILDSTR)/Debug \
                 ../lib/build/$(GBUILDSTR)/Debug

OPT_LIB_DIRS   = $(G)/src/vrpn/$(VRPN_VER)/vrpn/build/client/$(GBUILDSTR)/Release \
                 $(G)/src/vrpn/$(VRPN_VER)/quat/build/$(GBUILDSTR)/Release \
                 $(G)/src/VRG3D/build/lib/$(GBUILDSTR)/Release \
                 $(G)/src/vrbase/build/$(GBUILDSTR)/Release \
                 ../lib/build/$(GBUILDSTR)/Release



# Architecture-specific settings:

ifeq ($(GARCH),linux)
  LIB_DIRS   := $(LIB_DIRS) /usr/X11R6/lib
  LIBS       := dl X11 Xext pthread GLU GL z jpeg png zip SDL $(LIBS)
endif

ifeq ($(GARCH),OSX)
  LIB_DIRS   := $(LIB_DIRS) /usr/X11R6/lib
  LIBS       := $(LIBS) dl X11 Xext Xi Xmu pthread GLU z GL jpeg png zip
  FRAMEWORKS = Cocoa Carbon SDL OpenGL
  ifneq ($(findstring USE_SPACENAV,$(DEFINES)),) 
    FRAMEWORKS := $(FRAMEWORKS) 3DconnexionClient
  endif
endif

ifeq ($(GARCH),WIN32)
  LIBS     := $(LIBS) zlib jpeg png comctl32 user32 gdi32 advapi32 ws2_32
endif


# Adjust settings based on particular DEFINES:

ifneq ($(findstring USE_GLUT,$(DEFINES)),)
  LIBS := $(LIBS) glut
  ifneq ($(GARCH), WIN32)
    LIBS := $(LIBS) Xmu Xi
  endif
else
  ifeq ($(GARCH), WIN32)
    LIBS := $(LIBS) /NODEFAULTLIB:glut32
  endif
endif


ifneq ($(findstring USE_TRACKD,$(DEFINES)),) 
  G_INCLUDE_DIRS := $(G_INCLUDE_DIRS) trackdAPI
  ifeq ($(GARCH), WIN32)
    LIBS := $(LIBS) trackdAPI_MTs
  else
    LIBS := $(LIBS) trackdAPI
  endif
endif


include $(G)/src/gbuild/Makefile.gbuild.defines


all: progg
debug: progg
opt: progo

include $(G)/src/gbuild/Makefile.gbuild.rules


