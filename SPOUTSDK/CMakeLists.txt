#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Main Spout CMakeList                #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#

cmake_minimum_required(VERSION 3.15)
project(Spout 
	LANGUAGES CXX 
	VERSION 2.0
	HOMEPAGE_URL https://spout.zeal.co
)

# ensure lib and dll will be in the working directories of executables
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/${CMAKE_GENERATOR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/${CMAKE_GENERATOR})
set (SMODE_PDB_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/pdb/${CMAKE_GENERATOR})

if(NOT WIN32)
  message(STATUS "Spout is not supported outside of MS Windows")
  return()
endif()

OPTION(SPOUT_BUILD_EXAMPLES "Build Spout examples" ON)

add_subdirectory(SpoutSDK)
add_subdirectory(SpoutDirectX/SpoutDX)

OPTION(SPOUT_BUILD_LIBRARY "Build Spout cross compiler library" ON)
if(SPOUT_BUILD_LIBRARY)
  add_subdirectory(SpoutLibrary/Source)
endif()
