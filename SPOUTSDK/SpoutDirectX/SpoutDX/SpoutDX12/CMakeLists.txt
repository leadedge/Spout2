#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeLists.txt              | SpoutDX12 CMakeList                 #
# Author   : Lynn Jarvis                 |                                     #
# Started  : 19/07/2024                  |                                     #
# 21/07/24 - Remove SpoutGL cpp files from SpoutDX12sources                    #
#/-------------------------------------- . -----------------------------------\#

cmake_minimum_required(VERSION 3.15)
project(SpoutDX12)

set(SpoutDX12_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutDX12.cpp
)
set(SpoutDX12_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutDX12.h
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX12 object library
#

add_library(
		SpoutDX12_obj OBJECT
		${SpoutDX12_SOURCES}
		${SpoutDX12_HEADERS}
)
target_compile_definitions(
		SpoutDX12_obj PRIVATE
		SPOUT_BUILD_DLL
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX12 static library
#

add_library(
		SpoutDX12_static STATIC
		$<TARGET_OBJECTS:SpoutDX12_obj>
)
target_link_libraries(
		SpoutDX12_static PUBLIC
		SpoutDX_static
)
target_include_directories(
		SpoutDX12_static PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)
set_target_properties(
		SpoutDX12_static PROPERTIES
		SPOUT_PUBLIC_HEADER "${SpoutDX12_HEADERS}"
		RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
		LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
		ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX12 shared library
#

add_library(
		SpoutDX12 SHARED
		$<TARGET_OBJECTS:SpoutDX12_obj>
)
target_link_libraries(
		SpoutDX12 PRIVATE
		SpoutDX_static
		d3d12
)
target_include_directories(
		SpoutDX12 PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)
set_target_properties(
		SpoutDX12 PROPERTIES
		SPOUT_PUBLIC_HEADER "${SpoutDX12_HEADERS}"
		RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
		LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
		ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX12 install
#

if (NOT SKIP_INSTALL_ALL)
	if(NOT SKIP_INSTALL_LIBRARIES)
		install(
				TARGETS SpoutDX12 SpoutDX12_static
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
				${SpoutDX12_HEADERS}
				${spout_headers}
				${spoutdx_headers}
				DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SpoutDX/SpoutDX12
		)
	endif(NOT SKIP_INSTALL_HEADERS)
endif (NOT SKIP_INSTALL_ALL)
