#.SILENT:

include $(G)/src/gbuild/Makefile.gbuild.init

all: cmake

cmake:
	mkdir -p ./build
	mkdir -p ./build/$(GBUILDSTR)
        ifeq ($(GARCH),linux)
	  mkdir -p ./build/$(GBUILDSTR)/Release
	  mkdir -p ./build/$(GBUILDSTR)/Debug	  
	  cd ./build/$(GBUILDSTR)/Release; cmake -DCMAKE_BUILD_TYPE=Release -DGBUILDSTR=$(GBUILDSTR) ../../../
	  cd ./build/$(GBUILDSTR)/Debug; cmake -DCMAKE_BUILD_TYPE=Debug -DGBUILDSTR=$(GBUILDSTR) ../../../
        endif
        ifeq ($(GARCH),WIN32)
	  cd ./build/$(GBUILDSTR); cmake -DGBUILDSTR=$(GBUILDSTR) ../../ -G "Visual Studio 10 Win64"
        endif
        ifeq ($(GARCH),OSX)
	  cd $(SVNDIR)/vrg3d/build/$(GBUILDSTR); cmake -DGBUILDSTR=$(GBUILDSTR) ../../ -G Xcode
        endif
	@echo "Go to the build directory and make your project"

clean:  
	rm -rf ./build/

linux: linux-debug linux-opt

linux-debug:
	cd ./build/$(GBUILDSTR)/Debug; make

linux-opt:
	cd ./build/$(GBUILDSTR)/Release; make

clean-linux:
	cd ./build/$(GBUILDSTR)/Debug; make clean
	cd ./build/$(GBUILDSTR)/Release; make clean
