#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutDX9 CMakesList                 #
# Author   : Lynn Jarvis                 |                                     #
# Started  : 21/07/2024                  |                                     #
#/-------------------------------------- . -----------------------------------\#

cmake_minimum_required(VERSION 3.15)
project(SpoutDX9)

set(SpoutDX9_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutDX9.cpp
)
set(SpoutDX9_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutDX9.h
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX9 object library
#

add_library(
		SpoutDX9_obj OBJECT
		${SpoutDX9_SOURCES}
		${SpoutDX9_HEADERS}
)
target_compile_definitions(
		SpoutDX9_obj PRIVATE
		SPOUT_BUILD_DLL
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX9 static library
#

add_library(
		SpoutDX9_static STATIC
		$<TARGET_OBJECTS:SpoutDX9_obj>
)
target_link_libraries(
		SpoutDX9_static PRIVATE
		Spout_static
)
target_include_directories(
		SpoutDX9_static PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)
set_target_properties(
		SpoutDX9_static PROPERTIES
		SPOUT_PUBLIC_HEADER "${SpoutDX9_HEADERS}"
		RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
		LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
		ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX9 shared library
#

add_library(
		SpoutDX9 SHARED
		$<TARGET_OBJECTS:SpoutDX9_obj>
)
target_link_libraries(
		SpoutDX9 PRIVATE
		Spout_static
)
target_include_directories(
		SpoutDX9 PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)
set_target_properties(
		SpoutDX9 PROPERTIES
		SPOUT_PUBLIC_HEADER "${SpoutDX9_HEADERS}"
		RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
		LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
		ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX9 install
#

if (NOT SKIP_INSTALL_ALL)
	if(NOT SKIP_INSTALL_LIBRARIES)
		install(
				TARGETS SpoutDX9 SpoutDX9_static
				EXPORT SpoutTargets
				ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
				LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
				RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		)
	endif(NOT SKIP_INSTALL_LIBRARIES)
	if(NOT SKIP_INSTALL_HEADERS)
		get_target_property(spout_headers Spout SPOUT_PUBLIC_HEADER)
		get_target_property(spoutdx_headers SpoutDX SPOUT_PUBLIC_HEADER)
		install(
				FILES
				${SpoutDX9_HEADERS}
				${spout_headers}
				${spoutdx_headers}
				DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SpoutDX/SpoutDX9
		)
	endif(NOT SKIP_INSTALL_HEADERS)
endif (NOT SKIP_INSTALL_ALL)
