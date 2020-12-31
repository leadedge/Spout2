#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Main Spout CMakeList                #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#
# Modifications : spout.zeal.co                                                #
# 29/12/20 - Specify Spout Version 2.007                                       #
# 31/12/20 - Add /MT build option                                              #
#          - Add DirectX library build option                                  #
#/-------------------------------------- . -----------------------------------\#

cmake_minimum_required(VERSION 3.15)
project(Spout 
	LANGUAGES CXX 
	VERSION 2.007
	HOMEPAGE_URL https://spout.zeal.co
)

# ensure lib and dll will be in the working directories of executables
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/${CMAKE_GENERATOR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/${CMAKE_GENERATOR})

if(NOT WIN32)
  message(STATUS "Spout is not supported outside of MS Windows")
  return()
endif()

OPTION(SPOUT_BUILD_MT "For Visual Studio - build /MT to link runtime libraries" ON)
# https://gitlab.kitware.com/cmake/cmake/-/issues/18390
if(MSVC)
	add_compile_options(
		$<$<CONFIG:>:/MT> #---------|
		$<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
		$<$<CONFIG:Release>:/MT> #--|
	)
endif()

add_subdirectory(SPOUTSDK/SpoutGL)

OPTION(SPOUT_BUILD_DIRECTX "Build DirectX support library" ON)
if(SPOUT_BUILD_DIRECTX)
add_subdirectory(SPOUTSDK/SpoutDirectX/SpoutDX)
endif()

OPTION(SPOUT_BUILD_LIBRARY "Build C-compatible cross compiler library" ON)
if(SPOUT_BUILD_LIBRARY)
		add_subdirectory(SPOUTSDK/SpoutLibrary)
endif()

