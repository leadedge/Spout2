#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Spout CMakeList                     #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#
# Modifications : spout.zeal.co                                                #
# 28/12/20 - Add additional SpoutSDK library for dynamic build                 #
# 29/12/20 - Add Win32 or x64 build                                            #
# 30/12/20 - Add copy of build files to Binary folders                         #
# 15/01/21 - Rename SpoutSDK files to Spout                                    #
#            Generate Spout.dll instead of SpoutSDK.dll                        #
# 02/02/21 - Support single config generators (ninja, etc) by Joakim Kilby     #
# 24/06/23 - Modifications for MINGW build (PR#93-vkedwardli)                  #
# 08/07/23 - Remove MSVC Universal C Runtime dependency (PR#95-vkedwardli)     #
# 09/08/23 - Add comctl32 to SpoutLink library list (PR#96-scribam)            #
# 16/12/23 - Remove shlwapi from Spoutlink list                                #
# 20/12/23 - SpoutMessageBox changes in SpoutUtils for MinGW build             #
#/-------------------------------------- . -----------------------------------\#

cmake_minimum_required(VERSION 3.15)

set(Spout_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/Spout.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutCopy.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutDirectX.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutFrameCount.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutGL.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutGLextensions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutReceiver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutSender.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutSenderNames.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutSharedMemory.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutUtils.cpp
)
set(Spout_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/Spout.h
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutCommon.h
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutCopy.h
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutDirectX.h
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutFrameCount.h
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutGL.h
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutGLextensions.h
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutReceiver.h
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutSender.h
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutSenderNames.h
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutSharedMemory.h
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutUtils.h
)
set(SpoutLink
	opengl32
	kernel32
	user32
	gdi32
	winspool
	comdlg32
	comctl32
	advapi32
	shell32
	ole32
	oleaut32
	uuid
	odbc32
	odbccp32
	d3d9
	d3d11
	dxgi
	version
	winmm
)

#/-------------------------------------- . -----------------------------------\#
#
# Spout object library
#

add_library(
		Spout_obj OBJECT
		${Spout_SOURCES}
		${Spout_HEADERS}
)
target_compile_definitions(
		Spout_obj PRIVATE
		SPOUT_BUILD_DLL
)
target_include_directories(
		Spout_obj PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

if (MINGW)
	target_compile_options(
			Spout_obj PRIVATE
			-msse4
	)
endif (MINGW)

#/-------------------------------------- . -----------------------------------\#
#
# Spout static library
#

add_library(
		Spout_static STATIC
		$<TARGET_OBJECTS:Spout_obj>
)
target_link_libraries(
		Spout_static PUBLIC
		${SpoutLink}
)
target_include_directories(
		Spout_static PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)
set_target_properties(
		Spout_static PROPERTIES
		SPOUT_PUBLIC_HEADER "${Spout_HEADERS}"
		RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
		LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
		ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#/-------------------------------------- . -----------------------------------\#
#
# Spout shared library
#

add_library(
		Spout SHARED
		$<TARGET_OBJECTS:Spout_obj>
)
target_link_libraries(
		Spout PRIVATE
		${SpoutLink}
)
target_include_directories(
		Spout PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)
set_target_properties(
		Spout PROPERTIES
		SPOUT_PUBLIC_HEADER "${Spout_HEADERS}"
		RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
		LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
		ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#/-------------------------------------- . -----------------------------------\#
#
# SpoutGL install
#

if (NOT SKIP_INSTALL_ALL)
	if(NOT SKIP_INSTALL_LIBRARIES)
		install(
				TARGETS Spout Spout_static
				EXPORT SpoutTargets
				ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
				LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
				RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		)
	endif(NOT SKIP_INSTALL_LIBRARIES)
	if(NOT SKIP_INSTALL_HEADERS)
		install(
				FILES
				${Spout_HEADERS}
				DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SpoutGL
		)
	endif(NOT SKIP_INSTALL_HEADERS)
endif (NOT SKIP_INSTALL_ALL)
