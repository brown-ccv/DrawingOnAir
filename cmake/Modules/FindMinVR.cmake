
# Includes
find_path(MINVR_INCLUDE_DIR 
    api/MinVR.h
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/include/VRG3D
    ${CMAKE_INSTALL_PREFIX}/include/vrg3d
    ${CMAKE_INSTALL_PREFIX}/include 
    $ENV{MinVR_ROOT}/include/VRG3D
    $ENV{MinVR_ROOT}/include/vrg3d
    $ENV{MinVR_ROOT}/include
    /usr/local/include/VRG3D
    /usr/local/include/vrg3d
    /usr/local/include
)

find_path(MinVRG3D_INCLUDE_DIR 
    VRG3DApp.h
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/include/MinVR/plugins/G3D/VRG3D/include
    ${MINVR_PROJECT_ROOT}/plugins/G3D/VRG3D/include
    /usr/local/include/MinVR/plugins/G3D/VRG3D/include
    
)

# libMinVR
find_library(MINVR_OPT_LIBRARIES 
  NAMES 
    MinVR
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/lib/VRG3D/Release
    ${CMAKE_INSTALL_PREFIX}/lib/VRG3D
    ${CMAKE_INSTALL_PREFIX}/lib/vrg3d/Release
    ${CMAKE_INSTALL_PREFIX}/lib/vrg3d
    ${CMAKE_INSTALL_PREFIX}/lib/Release
    ${CMAKE_INSTALL_PREFIX}/lib
    $ENV{VRG3D_ROOT}/lib/Release
    $ENV{VRG3D_ROOT}/lib
    /usr/local/lib
)

find_library(MINVR_DEBUG_LIBRARIES 
  NAMES 
    MinVRd
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/lib/VRG3D/Debug
    ${CMAKE_INSTALL_PREFIX}/lib/VRG3D
    ${CMAKE_INSTALL_PREFIX}/lib/vrg3d/Debug
    ${CMAKE_INSTALL_PREFIX}/lib/vrg3d
    ${CMAKE_INSTALL_PREFIX}/lib/Debug
    ${CMAKE_INSTALL_PREFIX}/lib
    $ENV{VRG3D_ROOT}/lib/Debug
    $ENV{VRG3D_ROOT}/lib
    /usr/local/lib
)

find_library(MINVR_G3D_OPT_LIBRARIES 
  NAMES 
    MinVR_G3D
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/lib/VRG3D/Release
    ${CMAKE_INSTALL_PREFIX}/lib/VRG3D
    ${CMAKE_INSTALL_PREFIX}/lib/vrg3d/Release
    ${CMAKE_INSTALL_PREFIX}/lib/vrg3d
    ${CMAKE_INSTALL_PREFIX}/lib/Release
    ${CMAKE_INSTALL_PREFIX}/lib
    $ENV{VRG3D_ROOT}/lib/Release
    $ENV{VRG3D_ROOT}/lib
    /usr/local/lib
)

find_library(MINVR_G3D_DEBUG_LIBRARIES 
  NAMES 
    MinVR_G3Dd
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/lib/VRG3D/Release
    ${CMAKE_INSTALL_PREFIX}/lib/VRG3D
    ${CMAKE_INSTALL_PREFIX}/lib/vrg3d/Release
    ${CMAKE_INSTALL_PREFIX}/lib/vrg3d
    ${CMAKE_INSTALL_PREFIX}/lib/Release
    ${CMAKE_INSTALL_PREFIX}/lib
    $ENV{VRG3D_ROOT}/lib/Release
    $ENV{VRG3D_ROOT}/lib
    /usr/local/lib
)

set(MinVR_INCLUDE_DIRS
    ${MINVR_INCLUDE_DIR}
    ${MinVRG3D_INCLUDE_DIR}
)

unset(MINVR_LIBRARIES)
unset(MINVRG3D_LIBRARIES)
if (MINVR_OPT_LIBRARIES)
  list(APPEND MINVR_LIBRARIES optimized ${MINVR_OPT_LIBRARIES})
endif()

if (MINVR_DEBUG_LIBRARIES)
  list(APPEND MINVR_LIBRARIES debug ${MINVR_DEBUG_LIBRARIES})
endif()

if (MINVRG3D_OPT_LIBRARIES)
  list(APPEND MINVR_LIBRARIES optimized ${MINVR_G3D_OPT_LIBRARIES})
endif()

if (MINVRG3D_DEBUG_LIBRARIES)
  list(APPEND MINVR_LIBRARIES debug ${MINVR_G3D_DEBUG_LIBRARIES})
endif()



include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBNAME_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(
    MINVR  
    DEFAULT_MSG
    MinVR_INCLUDE_DIRS
    MINVR_LIBRARIES
	MinVRG3D_INCLUDE_DIR
	MINVRG3D_LIBRARIES
)

mark_as_advanced(
    MINVR_OPT_LIBRARIES
    MINVR_DEBUG_LIBRARIES
    MINVR_G3D_OPT_LIBRARIES
    MINVR_G3D_DEBUG_LIBRARIES
    MINVR_INCLUDE_DIR
    MINVR_G3D_INCLUDE_DIR
	MINVRG3D_LIBRARIES
)
