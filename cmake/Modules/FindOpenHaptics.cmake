
# Includes
find_path(OPENHAPTICS_INCLUDE_DIR 
    HD/hd.h
  HINTS 
    c:/OpenHaptics/Academic/3.1/include
    ${CMAKE_INSTALL_PREFIX}/include/OpenHaptics-3.1
    ${CMAKE_INSTALL_PREFIX}/include/OpenHaptics
    ${CMAKE_INSTALL_PREFIX}/include 
)

# libraries
find_library(OPENHAPTICS_OPT_LIBRARIES 
  NAMES 
    hd hl
  HINTS 
    c:/OpenHaptics/Academic/3.1/lib/VS2015/x64/ReleaseAcademicEdition
    ${CMAKE_INSTALL_PREFIX}/lib/OpenHaptics-3.1/Release
    ${CMAKE_INSTALL_PREFIX}/lib/OpenHaptics-3.1
    ${CMAKE_INSTALL_PREFIX}/lib/OpenHaptics/Release
    ${CMAKE_INSTALL_PREFIX}/lib/OpenHaptics
    ${CMAKE_INSTALL_PREFIX}/lib/Release
    ${CMAKE_INSTALL_PREFIX}/lib
)

find_library(OPENHAPTICS_DEBUG_LIBRARIES 
  NAMES 
    hd hl
  HINTS 
    c:/OpenHaptics/Academic/3.1/lib/VS2015/x64/DebugAcademicEdition
    ${CMAKE_INSTALL_PREFIX}/lib/OpenHaptics-3.1/Debug
    ${CMAKE_INSTALL_PREFIX}/lib/OpenHaptics-3.1
    ${CMAKE_INSTALL_PREFIX}/lib/OpenHaptics/Debug
    ${CMAKE_INSTALL_PREFIX}/lib/OpenHaptics
    ${CMAKE_INSTALL_PREFIX}/lib/Debug
    ${CMAKE_INSTALL_PREFIX}/lib
)


set(OPENHAPTICS_INCLUDE_DIRS
    ${OPENHAPTICS_INCLUDE_DIR}
)

unset(OPENHAPTICS_LIBRARIES)
list(APPEND OPENHAPTICS_LIBRARIES optimized ${OPENHAPTICS_OPT_LIBRARIES})

if (OPENHAPTICS_DEBUG_LIBRARIES)
  list(APPEND OPENHAPTICS_LIBRARIES debug ${OPENHAPTICS_DEBUG_LIBRARIES})
endif()


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBNAME_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(
    OPENHAPTICS  
    DEFAULT_MSG
    OPENHAPTICS_INCLUDE_DIRS
    OPENHAPTICS_LIBRARIES
)

mark_as_advanced(
    OPENHAPTICS_OPT_LIBRARIES
    OPENHAPTICS_DEBUG_LIBRARIES
    OPENHAPTICS_LIBRARIES
    OPENHAPTICS_INCLUDE_DIR
    OPENHAPTICS_INCLUDE_DIRS
)
