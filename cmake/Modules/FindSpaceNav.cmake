if (APPLE)
	# The 3DconnexionClient framework is the 3DxMacWare SDK, installed as part of
	# the 3DxMacWare driver software. It is located in
	# /Library/Frameworks/3DconnexionClient.framework
	# This is the API to deal with 3D Connexion devices, like the SpaceNavigator.
	find_library(3DconnexionClient 3DconnexionClient)
	if(3DconnexionClient)
		set(LIBSPACENAV_FOUND true)
		set(LibSPACENAV_INCLUDE_DIR ${3DconnexionClient})
		set(LibSPACENAV_OPT_LIBRARY ${3DconnexionClient})
		set(LIBSPACENAV_DEBUG_LIBRARY "")
		set(LIBSPACENAV_ALL_LIBRARIES "")
		if(NOT LibSPACENAV_FIND_QUIETLY)
			message(STATUS "Found LibSPACENAV")
		endif()
	else()
		if(LibSPACENAV_FIND_REQUIRED)
			message(FATAL_ERROR "LibSPACENAV not found. Please install the OSX spacenav driver from http://www.3dconnexion.com/service/drivers.html")
		elseif(NOT LibSPACENAV_FIND_QUIETLY)
			message(WARNING "LibSPACENAV not found. Please install the OSX spacenav driver from http://www.3dconnexion.com/service/drivers.html")
		endif()
	endif()
endif (APPLE)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# Check that the spacenav driver is installed
	find_file(SPNAVWIN TDxInput.dll
		HINTS "$ENV{ProgramW6432}/3Dconnexion/3Dconnexion 3DxSoftware/3DxWare64/win64" "$ENV{PROGRAMFILES}/3Dconnexion/3Dconnexion 3DxSoftware/3DxWare/win32" )
	if (EXISTS ${SPNAVWIN})
		set(LIBSPACENAV_FOUND true)
		set(LIBSPACENAV_OPT_LIBRARY "")
		set(LIBSPACENAV_DEBUG_LIBRARY "")
		set(LIBSPACENAV_ALL_LIBRARIES "")
		set(LIBSPACENAV_INCLUDE_DIRS "")
		if(NOT LibSPACENAV_FIND_QUIETLY)
			message(STATUS "Found LibSPACENAV")
		endif()
	else()
		if(LibSPACENAV_FIND_REQUIRED)
			message(FATAL_ERROR "LibSPACENAV not found. Please install the Windows spacenav driver from http://www.3dconnexion.com/service/drivers.html")
		elseif(NOT LibSPACENAV_FIND_QUIETLY)
			message(WARNING "LibSPACENAV not found. Please install the Windows spacenav driver from http://www.3dconnexion.com/service/drivers.html")
		endif()
	endif()
endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	# Linux specific code
	find_path(LIBSPACENAV_INCLUDE_DIR spnav.h
		HINTS $ENV{G}/src/libspacenav/libspnav-0.2.2 $ENV{SPACENAV_INCLUDE_DIR})

	find_library(LIBSPACENAV_OPT_LIBRARY NAMES libspnav.a spnav
		HINTS $ENV{G}/src/libspacenav/libspnav-0.2.2/build/${GBUILDSTR}/Release $ENV{SPACENAV_LIB_DIR})

	find_library(LIBSPACENAV_DEBUG_LIBRARY NAMES libspnav.a spnav
		HINTS $ENV{G}/src/libspacenav/libspnav-0.2.2/build/${GBUILDSTR}/Debug $ENV{SPACENAV_LIB_DIR})

	set(LIBSPACENAV_ALL_LIBRARIES "")
	
	#if only opt is found, use it for both
	if(LIBSPACENAV_OPT_LIBRARY AND LIBSPACENAV_DEBUG_LIBRARY)
		set(LIBSPACENAV_OPT_LIBRARIES optimized ${LIBSPACENAV_OPT_LIBRARY} )
		set(LIBSPACENAV_DEBUG_LIBRARIES debug ${LIBSPACENAV_DEBUG_LIBRARY} )
	elseif(LIBSPACENAV_OPT_LIBRARY AND NOT LIBSPACENAV_DEBUG_LIBRARY)
		set(LIBSPACENAV_OPT_LIBRARIES optimized ${LIBSPACENAV_OPT_LIBRARY} )
		set(LIBSPACENAV_DEBUG_LIBRARIES debug ${LIBSPACENAV_OPT_LIBRARY} )
	#if only debug is found, use it for both
	elseif(LIBSPACENAV_DEBUG_LIBRARY AND NOT LIBSPACENAV_OPT_LIBRARY)
		set(LIBSPACENAV_OPT_LIBRARIES optimized ${LIBSPACENAV_DEBUG_LIBRARY} )
		set(LIBSPACENAV_DEBUG_LIBRARIES debug ${LIBSPACENAV_DEBUG_LIBRARY} )
	endif()
	
	set(LIBSPACENAV_INCLUDE_DIRS ${LIBSPACENAV_INCLUDE_DIR} )

	include(FindPackageHandleStandardArgs)
	# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE
	# if all listed variables are TRUE
	find_package_handle_standard_args(LIBSPACENAV  DEFAULT_MSG
									  LIBSPACENAV_OPT_LIBRARY LIBSPACENAV_DEBUG_LIBRARY LIBSPACENAV_INCLUDE_DIR)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

mark_as_advanced(LIBSPACENAV_INCLUDE_DIR LIBSPACENAV_OPT_LIBRARY LIBSPACENAV_DEBUG_LIBRARY)