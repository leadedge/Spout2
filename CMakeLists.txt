#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeLists.txt              | Main Spout CMakeList                #
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
# 20/07/24 - Add SpoutDX12 library                                             #
# 21/07/24 - Add SpoutDX9 library                                              #
# 14/08/25 - mingw-w64 Linux cross build compatibility PR #122                 #
#          - Remove Examples option from library build                         #  
#/-------------------------------------- . -----------------------------------\#

cmake_minimum_required(VERSION 3.15)

project(Spout 
	LANGUAGES CXX 
	VERSION 2.0
	HOMEPAGE_URL https://spout.zeal.co
)

include(GNUInstallDirs)

# Artifact default output directory
set(SPOUT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Install default output directory
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/INSTALL CACHE PATH "..." FORCE)
endif()

if ("${PROJECT_NAME}" STREQUAL "${CMAKE_PROJECT_NAME}")
	set(SPOUT_NO_GLOBAL_WARNING ON)
else ("${PROJECT_NAME}" STREQUAL "${CMAKE_PROJECT_NAME}")
	set(SPOUT_NO_GLOBAL_WARNING OFF)
endif ("${PROJECT_NAME}" STREQUAL "${CMAKE_PROJECT_NAME}")

OPTION(SPOUT_NO_GLOBAL_WARNING "" ${SPOUT_NO_GLOBAL_WARNING_default})
OPTION(SPOUT_BUILD_ARM "Build for Windows on ARM" OFF)
OPTION(SPOUT_BUILD_CMT "For Visual Studio - build /MT to link runtime libraries" ON)
OPTION(SPOUT_BUILD_LIBRARY "Build C-compatible cross compiler library" ON)
OPTION(SPOUT_BUILD_SPOUTDX "Build SpoutDX DirectX 9/11/12 libraries" OFF)
# Install option check boxes
OPTION(SKIP_INSTALL_ALL "Install headers and libraries" OFF) # Default do not skip all install
OPTION(SKIP_INSTALL_HEADERS "Install headers" OFF)
OPTION(SKIP_INSTALL_LIBRARIES "Install libraries" OFF)

if (NOT WIN32)
	message(FATAL_ERROR "Spout is not supported outside of MS Windows")
endif (NOT WIN32)

if (SPOUT_BUILD_ARM)
	if (NOT SPOUT_NO_GLOBAL_WARNING)
		message(WARNING "sse2neon will be added globally")
	endif (NOT SPOUT_NO_GLOBAL_WARNING)

	# Use SS2NEON so SIMD code doesn't have to be rewritten for ARM
	# Download sse2neon.h
	File(DOWNLOAD https://github.com/DLTcollab/sse2neon/raw/master/sse2neon.h ${PROJECT_SOURCE_DIR}/SPOUTSDK/sse2neon/sse2neon.h)
	# Add compiler include_directory and /Zc:preprocessor define
	include_directories(SPOUTSDK/sse2neon)
	if (MSVC)
		add_compile_options(/Zc:preprocessor)
	endif (MSVC)
endif (SPOUT_BUILD_ARM)

if (SPOUT_BUILD_CMT AND MSVC)
	if (NOT SPOUT_NO_GLOBAL_WARNING)
		message(WARNING "MSVC runtime will be set to static globally")
	endif (NOT SPOUT_NO_GLOBAL_WARNING)

	# https://gitlab.kitware.com/cmake/cmake/-/issues/18390
	add_compile_options(
		$<$<CONFIG:>:/MT> #---------|
		$<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
		$<$<CONFIG:Release>:/MT> #--|
	)
endif (SPOUT_BUILD_CMT AND MSVC)

add_subdirectory(SPOUTSDK/SpoutGL)

if (SPOUT_BUILD_LIBRARY)
  add_subdirectory(SPOUTSDK/SpoutLibrary)
endif (SPOUT_BUILD_LIBRARY)

if (SPOUT_BUILD_SPOUTDX)
	add_subdirectory(SPOUTSDK/SpoutDirectX/SpoutDX)
	add_subdirectory(SPOUTSDK/SpoutDirectX/SpoutDX/SpoutDX9)
	add_subdirectory(SPOUTSDK/SpoutDirectX/SpoutDX/SpoutDX12)
endif (SPOUT_BUILD_SPOUTDX)

if (NOT SKIP_INSTALL_ALL AND NOT SKIP_INSTALL_LIBRARIES)
	include(CMakePackageConfigHelpers)

	write_basic_package_version_file(
			"${CMAKE_CURRENT_BINARY_DIR}/spout2-config-version.cmake"
			VERSION ${PROJECT_VERSION}
			COMPATIBILITY SameMajorVersion
	)

	configure_package_config_file(
			"${CMAKE_CURRENT_SOURCE_DIR}/spout2-config.cmake.in"
			"${CMAKE_CURRENT_BINARY_DIR}/spout2-config.cmake"
			INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spout2
	)

	install(EXPORT SpoutTargets
		FILE spout2-targets.cmake
		NAMESPACE Spout2::
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spout2
	)
	install(FILES
		"${CMAKE_CURRENT_BINARY_DIR}/spout2-config.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/spout2-config-version.cmake"
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spout2
	)
endif (NOT SKIP_INSTALL_ALL AND NOT SKIP_INSTALL_LIBRARIES)
