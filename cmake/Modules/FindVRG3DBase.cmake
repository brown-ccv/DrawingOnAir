

# Includes
find_path(VRG3DBase_INCLUDE_DIR 
    VRG3DBaseApp.h
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/include/VRG3DBase
	${CMAKE_INSTALL_PREFIX}/include
)


# libGLG3D
find_library(VRG3DBase_LIBRARY 
  NAMES 
	VRG3DBase.lib
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/lib
)

# libGLG3D
find_library(VRG3DBase_DEBUG_LIBRARY 
  NAMES 
	VRG3DBased.lib
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/lib
)



include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBNAME_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(
    VRG3DBase
    DEFAULT_MSG
	VRG3DBase_INCLUDE_DIR
	VRG3DBase_LIBRARY
	VRG3DBase_DEBUG_LIBRARY
)

mark_as_advanced(
    VRG3DBase
	VRG3DBase_INCLUDE_DIR
	VRG3DBase_LIBRARY
	VRG3DBase_DEBUG_LIBRARY
)
