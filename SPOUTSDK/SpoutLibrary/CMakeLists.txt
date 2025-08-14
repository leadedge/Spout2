#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutLibrary CMakeList              #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#
# Modifications : spout.zeal.co                                                #
# 29/12/20 - Add Win32 or x64 build                                            #
# 30/12/20 - Add copy of build files to Binary folders                         #
# 02/02/21 - Support single config generators (ninja, etc) by Joakim Kilby     #
# 09/08/23 - Use TARGET_LINKER_FILE/TARGET_LINKER_FILE_NAME (PR#97-scribam)    #
#            Old code commented out for reference                              #
#/-------------------------------------- . -----------------------------------\#                                      #

cmake_minimum_required(VERSION 3.15)
project(SpoutLibrary)

set(SpoutLibrary_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutLibrary.cpp
)
set(SpoutLibrary_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutLibrary.h
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutLibrary
#

add_library(
		${PROJECT_NAME} SHARED
		${SpoutLibrary_SOURCES}
		${SpoutLibrary_HEADERS}
)

target_link_libraries(
		${PROJECT_NAME} PRIVATE
		Spout_static
)

target_compile_definitions(
		${PROJECT_NAME} PRIVATE
		SPOUT_BUILD_DLL
)

set_target_properties(
		SpoutLibrary PROPERTIES
		SPOUT_PUBLIC_HEADER "${SpoutLibrary_HEADERS}"
		RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
		LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
		ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutLibrary install
#

if (NOT SKIP_INSTALL_ALL)
	if(NOT SKIP_INSTALL_LIBRARIES)
		install(
				TARGETS SpoutLibrary
				EXPORT SpoutTargets
				ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
				LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
				RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		)
	endif(NOT SKIP_INSTALL_LIBRARIES)
	if(NOT SKIP_INSTALL_HEADERS)
		install(
				FILES ${SpoutLibrary_HEADERS}
				DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SpoutLibrary
		)
	endif(NOT SKIP_INSTALL_HEADERS)
endif (NOT SKIP_INSTALL_ALL)
