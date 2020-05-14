# This file is part of the DrawingOnAir cmake build system.  
# See the main DrawingOnAir/CMakeLists.txt file for details.

project(ForceServer)


# Source:
set (SOURCEFILES 
	AnisotropicFilterEffect.cpp
	ForceEffect.cpp
	ForceServer.cpp
	HybridDrawingEffect.cpp
	ICubeXDevice.cpp
	main.cpp
	Midi.cpp
	OneHandDrawingEffect.cpp
	SoundOpenAL.cpp
	SoundSDL.cpp
	TapeDrawingEffect.cpp
)
set (HEADERFILES
	AnisotropicFilterEffect.H
	ForceEffect.H
	ForceServer.H
	HybridDrawingEffect.H
	ICubeXDevice.H
	Midi.H
	OneHandDrawingEffect.H
	SoundBase.H
	SoundOpenAL.H
	SoundSDL.H
	TapeDrawingEffect.H
)
set (CONFIGFILES
)


# Define the target
add_executable(${PROJECT_NAME} ${HEADERFILES} ${SOURCEFILES})


# Add dependency on libDrawingOnAir:
target_include_directories(${PROJECT_NAME} PUBLIC ../../src)
target_link_libraries(${PROJECT_NAME} PUBLIC DrawingOnAir)

# Add dependency on OpenHaptics (only supporting Windows for now)
find_package(OpenHaptics MODULE REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENHAPTICS_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENHAPTICS_LIBRARIES})


# Installation:
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${INSTALL_BIN_DEST}
        COMPONENT Apps)


# For better organization when using an IDE with folder structures:
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Tests")
source_group("Header Files" FILES ${HEADERFILES})
set_source_files_properties(${CONFIGFILES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group("Config Files" FILES ${CONFIGFILES})

