#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Main Spout CMakeList                #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#
# Modifications : spout.zeal.co                                                #
# 29/12/20 - Specify Spout Version 2.007                                       #
# 31/12/20 - Add /MT build option                                              #
#          - Add DirectX library build option                                  #
# 06/11/22 - Add Install option by Joakim Kilby                                #
# 13/01/23 - Default do not skip INSTALL project                               #
#            Add necessary SpoutGL headers to SpoutDX includes                 #
# 07/10/23 - Add SPOUT_BUILD_ARM option and download sse2neon.h                #
#            Add compiler include_directory and /Zc:preprocessor define        #
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


OPTION(SPOUT_BUILD_ARM "Build for Windows on ARM" OFF)
if(SPOUT_BUILD_ARM)
	# Use SS2NEON so SIMD code doesn't have to be rewritten for ARM
	# Download sse2neon.h
	File(DOWNLOAD https://github.com/DLTcollab/sse2neon/raw/master/sse2neon.h ${PROJECT_SOURCE_DIR}/SPOUTSDK/sse2neon/sse2neon.h)
	# Add compiler include_directory and /Zc:preprocessor define
	include_directories(SPOUTSDK/sse2neon)
	add_compile_options(/Zc:preprocessor)
endif()


OPTION(SPOUT_BUILD_CMT "For Visual Studio - build /MT to link runtime libraries" ON)
if(SPOUT_BUILD_CMT)
	# https://gitlab.kitware.com/cmake/cmake/-/issues/18390
	if(MSVC)
		add_compile_options(
			$<$<CONFIG:>:/MT> #---------|
			$<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
			$<$<CONFIG:Release>:/MT> #--|
		)
	endif()
endif()

add_subdirectory(SPOUTSDK/SpoutGL)

OPTION(SPOUT_BUILD_LIBRARY "Build C-compatible cross compiler library" ON)
if(SPOUT_BUILD_LIBRARY)
  add_subdirectory(SPOUTSDK/SpoutLibrary)
endif()

# The SpoutDX CMakeLists needs this option
OPTION(SPOUT_BUILD_SPOUTDX_EXAMPLES "Build SpoutDX examples" OFF)

OPTION(SPOUT_BUILD_SPOUTDX "Build SpoutDX DirectX11 support library" OFF)
if(SPOUT_BUILD_SPOUTDX OR SPOUT_BUILD_SPOUTDX_EXAMPLES)
add_subdirectory(SPOUTSDK/SpoutDirectX/SpoutDX)
endif()

# Install option check boxes
OPTION(SKIP_INSTALL_ALL "Install headers and libraries" OFF) # Default do not skip all install
OPTION(SKIP_INSTALL_HEADERS "Install headers" OFF)
OPTION(SKIP_INSTALL_LIBRARIES "Install libraries" OFF)

include(GNUInstallDirs)

# Install default output folder
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/INSTALL CACHE PATH "..." FORCE)
endif()

# Install options
if(NOT SKIP_INSTALL_ALL)

	if(NOT SKIP_INSTALL_LIBRARIES)

		set(Spout2InstallTargets Spout Spout_static)

		if(SPOUT_BUILD_SPOUTDX OR SPOUT_BUILD_SPOUTDX_EXAMPLES)
			list(APPEND Spout2InstallTargets SpoutDX SpoutDX_static)
		endif()

		if(SPOUT_BUILD_LIBRARY)
			list(APPEND Spout2InstallTargets SpoutLibrary)
		endif()

		install(
			TARGETS
				${Spout2InstallTargets}
			EXPORT
				spout2-targets
			LIBRARY
				DESTINATION ${CMAKE_INSTALL_LIBDIR}
			ARCHIVE
				DESTINATION ${CMAKE_INSTALL_LIBDIR}
			RUNTIME
				DESTINATION ${CMAKE_INSTALL_BINDIR}
			INCLUDES
				DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		)

		install(
			EXPORT
				spout2-targets
			DESTINATION
				${CMAKE_INSTALL_LIBDIR}/cmake/spout2
			NAMESPACE
				Spout2::
		)

		install(
			FILES
				${CMAKE_CURRENT_SOURCE_DIR}/spout2-config.cmake
			DESTINATION
				${CMAKE_INSTALL_LIBDIR}/cmake/spout2
		)
		
	endif()


	if(NOT SKIP_INSTALL_HEADERS)
	
		install(
			FILES
				SPOUTSDK/SpoutGL/Spout.h
				SPOUTSDK/SpoutGL/SpoutCommon.h
				SPOUTSDK/SpoutGL/SpoutCopy.h
				SPOUTSDK/SpoutGL/SpoutDirectX.h
				SPOUTSDK/SpoutGL/SpoutFrameCount.h
				SPOUTSDK/SpoutGL/SpoutGL.h
				SPOUTSDK/SpoutGL/SpoutGLextensions.h
				SPOUTSDK/SpoutGL/SpoutReceiver.h
				SPOUTSDK/SpoutGL/SpoutSender.h
				SPOUTSDK/SpoutGL/SpoutSenderNames.h
				SPOUTSDK/SpoutGL/SpoutSharedMemory.h
				SPOUTSDK/SpoutGL/SpoutUtils.h
			DESTINATION
				${CMAKE_INSTALL_PREFIX}/include/SpoutGL
		)

		if(SPOUT_BUILD_SPOUTDX OR SPOUT_BUILD_SPOUTDX_EXAMPLES)
			install(
				FILES
				SPOUTSDK/SpoutDirectX/SpoutDX/SpoutDX.h
				SPOUTSDK/SpoutGL/SpoutCommon.h
				SPOUTSDK/SpoutGL/SpoutCopy.h
				SPOUTSDK/SpoutGL/SpoutDirectX.h
				SPOUTSDK/SpoutGL/SpoutFrameCount.h
				SPOUTSDK/SpoutGL/SpoutSenderNames.h
				SPOUTSDK/SpoutGL/SpoutSharedMemory.h
				SPOUTSDK/SpoutGL/SpoutUtils.h
				DESTINATION
					${CMAKE_INSTALL_PREFIX}/include/SpoutDX
			)
		endif()	

		if(SPOUT_BUILD_LIBRARY)
			install(
				FILES
					SPOUTSDK/SpoutLibrary/SpoutLibrary.h
				DESTINATION
					${CMAKE_INSTALL_PREFIX}/include/SpoutLibrary
			)
		endif()

	endif()

endif()

