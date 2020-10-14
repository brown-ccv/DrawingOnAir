# This file is part of the DrawingOnAir cmake build system.  
# See the main DrawingOnAir/CMakeLists.txt file for details.


set(HEADERFILES
    src/AnnotationMark.H
    src/AnnotationModel.H
    src/Artwork.H
    src/ArtworkIO.H
    src/BlendHCI.H
    src/Brush.H
    src/BrushModelPicker.H
    src/BrushSizePicker.H
    src/BrushState.H
    src/BrushTipPicker.H
    src/CavePainting.H
    src/CavePaintingCursors.H
    src/ColorPicker.H
    src/ColorSwatchPicker.H
    src/CopyToNextFrameHCI.H
    src/DeleteHCI.H
    src/DirectDrawHCI.H
    src/DragDrawHCI.H
    src/DrawOnAir.H
    src/DrawingStudy.H
    src/FishtankDepthCues.H
    src/FlatTubeMark.H
    src/ForceBlendHCI.H
    src/ForceHybridDrawHCI.H
    src/ForceMovingPlaneDrawHCI.H
    src/ForceNetInterface.H
    src/ForceOneHandDrawHCI.H
    src/ForceReverseTapeDrawHCI.H
    src/ForceTapeDrawHCI.H
    src/FrameWidget.H
    src/HCIMgr.H
    src/History.H
    src/HybridDrawHCI.H
    src/KbdSelectWidget.H
    src/LayerWidget.H
    src/LightingIO.H
    src/LoadFilePicker.H
    src/Mark.H
    src/MarkPicker.H
    src/MarkingMenu.H
    src/MoveMarksHCI.H
    src/PatternPicker.H
    src/PickerWidget.H
    src/PlaceSlideHCI.H
    src/PointModel.H
    src/PointModelSW.H
    src/PointModelSprites.H
    src/PointSplatMark.H
    src/ReframeArtworkHCI.H
    src/ReverseTapeDrawHCI.H
    src/RibbonMark.H
    src/Selection.H
    src/SlideMark.H
    src/SlidePicker.H
    src/SplineFit.H
    src/TapeDrawHCI.H
    src/TextInputWidget.H
    src/TriStripModel.H
    src/TubeMark.H
    src/VRML2Parser.H
    src/WorkingLimits.H
)

set(SOURCEFILES
    src/AnnotationMark.cpp
    src/AnnotationModel.cpp
    src/Artwork.cpp
    src/ArtworkIO.cpp
    src/BlendHCI.cpp
    src/Brush.cpp
    src/BrushModelPicker.cpp
    src/BrushSizePicker.cpp
    src/BrushState.cpp
    src/BrushTipPicker.cpp
    src/CavePainting.cpp
    src/CavePaintingCursors.cpp
    src/ColorPicker.cpp
    src/ColorSwatchPicker.cpp
    src/CopyToNextFrameHCI.cpp
    src/DeleteHCI.cpp
    src/DirectDrawHCI.cpp
    src/DragDrawHCI.cpp
    src/DrawingStudy.cpp
    src/FishtankDepthCues.cpp
    src/FlatTubeMark.cpp
    src/ForceBlendHCI.cpp
    src/ForceHybridDrawHCI.cpp
    src/ForceMovingPlaneDrawHCI.cpp
    src/ForceNetInterface.cpp
    src/ForceOneHandDrawHCI.cpp
    src/ForceReverseTapeDrawHCI.cpp
    src/ForceTapeDrawHCI.cpp
    src/FrameWidget.cpp
    src/HCIMgr.cpp
    src/History.cpp
    src/HybridDrawHCI.cpp
    src/KbdSelectWidget.cpp
    src/LayerWidget.cpp
    src/LightingIO.cpp
    src/LoadFilePicker.cpp
    src/Mark.cpp
    src/MarkPicker.cpp
    src/MarkingMenu.cpp
    src/MoveMarksHCI.cpp
    src/PatternPicker.cpp
    src/PickerWidget.cpp
    src/PlaceSlideHCI.cpp
    src/PointModel.cpp
    src/PointModelSW.cpp
    src/PointModelSprites.cpp
    src/PointSplatMark.cpp
    src/ReframeArtworkHCI.cpp
    src/ReverseTapeDrawHCI.cpp
    src/RibbonMark.cpp
    src/Selection.cpp
    src/SlideMark.cpp
    src/SlidePicker.cpp
    src/SplineFit.cpp
    src/TapeDrawHCI.cpp
    src/TextInputWidget.cpp
    src/TriStripModel.cpp
    src/TubeMark.cpp
    src/VRML2Parser.cpp
    src/WorkingLimits.cpp
)

set(EXTRAFILES
    src/Drawingonair_config.h.in
)

set(SHADERFILES
    src/shaders/PointLine.glsl.vrt
    src/shaders/PointModelBillboard.glsl.vrt
    src/shaders/PointModelSprite.glsl.frg
    src/shaders/PointModelSprite.glsl.vrt
)

set_source_files_properties(${EXTRAFILES} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(${SHADERFILES} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Header Files" FILES ${HEADERFILES})
source_group("Source Files" FILES ${SOURCEFILES})
source_group("Shaders" FILES ${SHADERFILES})

#add_library(DrawingOnAir SHARED ${HEADERFILES} ${SOURCEFILES} ${EXTRAFILES} ${SHADERFILES})

add_library(DrawingOnAir ${HEADERFILES} ${SOURCEFILES} ${EXTRAFILES} ${SHADERFILES})


# Using target_include_directories() rather than just include_directories() is
# critical in order to support generating a DrawingOnAirConfig.cmake file.  It supports
# generator expressions, so we can point to two different include dirs depending
# on whether building or using the installed version.
target_include_directories(DrawingOnAir PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>  # for headers when building
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DEST}>        # for client in install mode
)

find_package(MinVR REQUIRED)
target_link_libraries(DrawingOnAir PUBLIC MinVR::MinVR MinVR::MinVR_G3D)
add_definitions(-DG3D_NO_FFMPEG)

include(AutoBuildVRG3DBase)
AutoBuild_use_package_VRG3DBase(DrawingOnAir PUBLIC)

# Add external dependency on G3D
#find_package(G3D MODULE REQUIRED)
#target_include_directories(DrawingOnAir PUBLIC ${G3D_INCLUDE_DIRS})
#target_link_libraries(DrawingOnAir PUBLIC ${G3D_LIBRARIES})
#target_link_libraries(DrawingOnAir PUBLIC ${PNG_LIBRARIES})
#target_link_libraries(DrawingOnAir PUBLIC ${JPEG_LIBRARIES})
#target_link_libraries(DrawingOnAir PUBLIC ${ZIP_LIBRARIES})
#target_link_libraries(DrawingOnAir PUBLIC ${ZLIB_LIBRARIES})

#find_package(VRG3DBase MODULE REQUIRED)
#target_include_directories(DrawingOnAir PUBLIC ${VRG3DBase_INCLUDE_DIR})
#target_link_libraries(DrawingOnAir PUBLIC ${VRG3DBase_LIBRARY})



#if(${G3D_INCLUDE_DIRS})
#set_property(GLOBAL PROPERTY DRAW_ON_AIR_G3D_INCLUDE_DIR "${G3D_INCLUDE_DIRS}")
#else()


# Add external dependency on VRG3D (includes VRBase)
#find_package(VRG3D MODULE REQUIRED)
#target_include_directories(DrawingOnAir PUBLIC ${VRG3D_INCLUDE_DIRS})
#target_link_libraries(DrawingOnAir PUBLIC ${VRG3D_LIBRARIES})

# Add external dependency on VRPN
#find_package(VRPN MODULE REQUIRED)
#target_include_directories(DrawingOnAir PUBLIC ${VRPN_INCLUDE_DIRS})
#target_link_libraries(DrawingOnAir PUBLIC ${VRPN_LIBRARIES})

# Add external dependency on OpenGL
#include(AutoBuildOpenGL)
#AutoBuild_use_package_OpenGL(DrawingOnAir PUBLIC)

#find_package(GLUT REQUIRED)
#target_link_libraries(DrawingOnAir PUBLIC ${GLUT_LIBRARY})


install(TARGETS DrawingOnAir EXPORT DrawingOnAirTargets COMPONENT CoreLib
  LIBRARY DESTINATION ${INSTALL_LIB_DEST}
  ARCHIVE DESTINATION ${INSTALL_LIB_DEST}
  RUNTIME DESTINATION ${INSTALL_BIN_DEST}
)

install(FILES ${HEADERFILES} DESTINATION ${INSTALL_INCLUDE_DEST} COMPONENT CoreLib)
install(FILES ${SHADERFILES} DESTINATION ${INSTALL_SHADERS_DEST} COMPONENT CoreLib)
