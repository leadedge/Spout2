#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutDX CMakeList                   #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
# Modifications : spout.zeal.co                                                #
# 29/04/21 - Add copy of build files to Binary folders                         #
#          - Support single config generators (ninja, etc) by Joakim Kilby     #
# 13/01/23 - Add SpoutDX static library                                        #
# 24/06/23 - Modifications for MINGW build (PR#93-vkedwardli)                  #
# 21/07/24 - Remove SpoutGL cpp files from SpoutDXsources                      #
# 14/08/25 : mingw-w64 Linux cross build compatibility PR #122                 #
#          : Remove Examples option and from library build                     #  
#/-------------------------------------- . -----------------------------------\#

cmake_minimum_required(VERSION 3.15)
project(SpoutDX)

set(SpoutDX_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutDX.cpp
)
set(SpoutDX_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutDX.h
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX object library
#

add_library(
		SpoutDX_obj OBJECT
		${SpoutDX_SOURCES}
		${SpoutDX_HEADERS}
)
target_compile_definitions(
		SpoutDX_obj PRIVATE
		SPOUT_BUILD_DLL
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX static library
#

add_library(
		SpoutDX_static STATIC
		$<TARGET_OBJECTS:SpoutDX_obj>
)
target_link_libraries(
		SpoutDX_static PUBLIC
		Spout_static
)
target_include_directories(
		SpoutDX_static PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)
set_target_properties(
		SpoutDX_static PROPERTIES
		SPOUT_PUBLIC_HEADER "${SpoutDX_HEADERS}"
		RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
		LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
		ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX shared library
#

add_library(
		SpoutDX SHARED
		$<TARGET_OBJECTS:SpoutDX_obj>
)
target_link_libraries(
		SpoutDX PUBLIC
		Spout_static
)
target_include_directories(
		SpoutDX PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)
set_target_properties(
		SpoutDX PROPERTIES
		SPOUT_PUBLIC_HEADER "${SpoutDX_HEADERS}"
		RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
		LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
		ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX install
#

if (NOT SKIP_INSTALL_ALL)
	if(NOT SKIP_INSTALL_LIBRARIES)
		install(
				TARGETS SpoutDX SpoutDX_static
				EXPORT SpoutTargets
				ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
				LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
				RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		)
	endif(NOT SKIP_INSTALL_LIBRARIES)
	if(NOT SKIP_INSTALL_HEADERS)
		get_target_property(spout_headers Spout SPOUT_PUBLIC_HEADER)
		install(
				FILES
				${SpoutDX_HEADERS}
				${spout_headers}
				DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SpoutDX
		)
	endif(NOT SKIP_INSTALL_HEADERS)
endif (NOT SKIP_INSTALL_ALL)
