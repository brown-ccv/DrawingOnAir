# DrawingOnAir

## Overview

DrawOnAir is a haptic-aided input technique for drawing controlled 3D curves through space. It's implementation was later called Cave-Painitng and used as an artistic medium that uses a 3D analog of 2D brush strokes to create 3D works of art in a fully immersive Cave environment. It is used for scientific research in the virtual reality thearther YURT at Brown University

DrawOnAir original authors are [Daniel F. Keefe](https://cse.umn.edu/cs/dan-keefe) and [David H. Laidlaw](http://cs.brown.edu/~dhl/). For more information about the research capabilities of this tool please refer to the [original paper](http://cs.brown.edu/people/jlaviola/pubs/i3d01_cavepainting.pdf) or the [2007 publication](http://vis.cs.brown.edu/docs/pdf/Keefe-2007-DOA.pdf) in the Peer-reviewed journal IEEE TRANSACTIONS ON VISUALIZATION AND COMPUTER GRAPHICS. Please contact the authors for additional information or to obtain the original source code.

This repository is a copy of the main SVN archive and uploaded to GitHub in 2020 by the Center for Computation and Visualization (CCV). It is a modified version of the software that extends its capabilites to HMD(VR) devices such as Oculus Quest and Samsung Odessy on Windows platforms.

:warning: VR mode is only supported on Windows.

## Depedencies

- [G3D](https://casual-effects.com/g3d/www/index.html) Multiplatform Graphics Engine
- [MinVR](https://github.com/MinVR/MinVR) Multi-display visualization library
  - Build it with G3D plugin (It contains the G3D version compatible and fully tested with DrawOnAir.)
- [VRG3DBase](https://github.com/brown-ccv/VRG3DBase) This library acts as middle tier dependency to support G3D events and textures load for DrawOnAir.

## How to build

1. Clone MinVR and build it with the CMake "G3D_PLUGIN" option enabled. If you want VR support you must enable "OPENVR_PLUGIN"
2. Clone VRG3DBase, add MinVR as dependecy to the CMake project and build it.
3. Clone DrawingOnAir, add VRG3DBase and VRG3DBase as depedency to the CMake project.
4. Use the following command argument to run the executable DrawTool:

### Desktop Mode

`-c Desktop_g3D.minvr desktop -f PATH_TO_DRAWONAIR\share\config\cavepainting-common.cfg -f PATH_TO_DRAWONAIR\share\config\cavepainting-desktop.cfg`

### VR Mode

`-c HTC_Vive_g3D.minvr desktop -f PATH_TO_DRAWONAIR\share\config\cavepainting-common.cfg -f PATH_TO_DRAWONAIR\share\config\cavepainting-VR.cfg`

