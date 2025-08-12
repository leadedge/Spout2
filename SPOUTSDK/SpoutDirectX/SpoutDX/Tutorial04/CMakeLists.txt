#---------------------------------------------------------------#
# Filename : CMakeList.txt  | Spout Tutorial04 CMakeList		#
# Author   : Lynn Jarvis	|								   #
# Started  : 11/08/2025	 |								   #
#---------------------------------------------------------------#

cmake_minimum_required(VERSION 3.15)
project(Tutorial04)

find_package(directxmath CONFIG REQUIRED)

# Warning level
if(MSVC)
	add_compile_options(/W4)
endif()

# Source files
set(Tutorial04_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/Tutorial04.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Tutorial04.rc
)

# Header files
set(Tutorial04_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/Resource.h
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
	${Tutorial04_HEADERS}
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
		SpoutDX
		d3d11
		d3dcompiler
		dxgi
		dxguid
		psapi
		version
		winmm
		Microsoft::DirectXMath
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
#			-mssse3
			-Wall
	)
	target_link_options(
			${PROJECT_NAME} PRIVATE
			-municode
	)
endif(MINGW)

set_target_properties(
		${PROJECT_NAME} PROPERTIES
		WIN32_EXECUTABLE TRUE
		VS_DPI_AWARE "PerMonitor"
		VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries/Examples
)

# Copy FX file for runtime (required by your app)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/Tutorial04.fx
		$<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Copy SpoutDX runtime library (required by your app)
add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE:SpoutDX>
		$<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Set startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Visual Studio source groups (IDE filters)
source_group("Shaders" FILES ${SHADERS})
source_group("Resource Files" FILES resource.h Tutorial04.rc directx.ico)
