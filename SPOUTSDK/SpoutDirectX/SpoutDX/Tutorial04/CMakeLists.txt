#---------------------------------------------------------------#
# Filename : CMakeList.txt  | Spout Tutorial04 CMakeList		#
# Author   : Lynn Jarvis	|								   #
# Started  : 11/08/2025	 |								   #
#---------------------------------------------------------------#

cmake_minimum_required(VERSION 3.15)
project(Tutorial04)

# Source file folders
set(SPOUTSDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../SpoutGL)
set(SPOUTDX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Define source files
set(SPOUTDX_SOURCES
    ${SPOUTDX_DIR}/SpoutDX.cpp
    ${SPOUTDX_DIR}/SpoutDX.h
)

set(SPOUTSDK_SOURCES
    ${SPOUTSDK_DIR}/SpoutCopy.cpp
    ${SPOUTSDK_DIR}/SpoutDirectX.cpp
    ${SPOUTSDK_DIR}/SpoutFrameCount.cpp
    ${SPOUTSDK_DIR}/SpoutSenderNames.cpp
    ${SPOUTSDK_DIR}/SpoutSharedMemory.cpp
    ${SPOUTSDK_DIR}/SpoutUtils.cpp
    ${SPOUTSDK_DIR}/SpoutCopy.h
    ${SPOUTSDK_DIR}/SpoutDirectX.h
    ${SPOUTSDK_DIR}/SpoutFrameCount.h
    ${SPOUTSDK_DIR}/SpoutSenderNames.h
    ${SPOUTSDK_DIR}/SpoutSharedMemory.h
    ${SPOUTSDK_DIR}/SpoutUtils.h
)

# Tutorial04 source files
set(Tutorial04_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/Tutorial04.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Tutorial04.rc
    ${SPOUTDX_SOURCES}
    ${SPOUTSDK_SOURCES}
)

# Shader files
set(Tutorial04_SHADERS
 	${CMAKE_CURRENT_SOURCE_DIR}/Tutorial04.fx
 	${CMAKE_CURRENT_SOURCE_DIR}/Tutorial04_VS.hlsl
 	${CMAKE_CURRENT_SOURCE_DIR}/Tutorial04_PS.hlsl
)

# Add executable
add_executable(${PROJECT_NAME}
	${Tutorial04_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/directx.ico
)

# Enable C++17 support
target_compile_features(
		${PROJECT_NAME} PRIVATE
		cxx_std_17
)

# Include directories
target_include_directories(
		${PROJECT_NAME} PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

# Use ANSI build by removing Unicode flags instead of redefining them
target_compile_definitions(
		${PROJECT_NAME} PRIVATE
		_UNICODE
		UNICODE
)

# Link libraries
target_link_libraries(
		${PROJECT_NAME}
		d3d11
		d3dcompiler
		dxgi
		dxguid
		psapi
		version
		winmm
)

if (MSVC)
	target_compile_options(
			${PROJECT_NAME} PRIVATE
			/W4
	)
endif (MSVC)

# Optional: for MinGW or MSYS2 builds
if(MINGW)
	target_compile_options(
			${PROJECT_NAME} PRIVATE
			-msse4
			-Wall
	)
	target_link_options(
			${PROJECT_NAME} PRIVATE
			-municode
	)
	target_include_directories(
		${PROJECT_NAME} PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/external
	)	
endif(MINGW)

set_target_properties(
		${PROJECT_NAME} PROPERTIES
		WIN32_EXECUTABLE TRUE
		VS_DPI_AWARE "PerMonitor"
		VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries/Examples
)

# Copy FX file for runtime
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/Tutorial04.fx
		$<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Set startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Visual Studio source groups (IDE filters)
source_group("Resource Files" FILES resource.h Tutorial04.rc directx.ico)
source_group("SpoutDX" FILES ${SPOUTDX_SOURCES})
source_group("SpoutSDK" FILES ${SPOUTSDK_SOURCES})



