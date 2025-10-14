#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Spout DX Receiver CMakeList         #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
# 10/12/23 : Add DPI awareness           |                                     #
#          : Copy binaries to the BUILD/Binaries/Examples folder.              #
# 12/12/23 : Remove target_link_libraries "Spout". Not used.                   #
#/-------------------------------------- . -----------------------------------\#

cmake_minimum_required(VERSION 3.15)

project(WinSpoutDXreceiver)

set(WinSpoutDXreceiver_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/WinSpoutDX.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WinSpoutDX.rc
)
set(WinSpoutDXreceiver_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/framework.h
	${CMAKE_CURRENT_SOURCE_DIR}/targetver.h
	${CMAKE_CURRENT_SOURCE_DIR}/Resource.h
	${CMAKE_CURRENT_SOURCE_DIR}/WinSpoutDX.h
)

# Use /MT in Release, /MTd in Debug
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(
		${PROJECT_NAME} WIN32
		${WinSpoutDXreceiver_SOURCES}
		${WinSpoutDXreceiver_HEADERS}
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/MT>
        $<$<CONFIG:Release>:/MT>
        $<$<CONFIG:RelWithDebInfo>:/MT>
        $<$<CONFIG:MinSizeRel>:/MT>
    )
endif()

target_compile_definitions(
		${PROJECT_NAME} PRIVATE
		_UNICODE
		UNICODE
)

target_link_libraries(
		${PROJECT_NAME} PRIVATE
		SpoutDX_static
		d3d11
		d3dcompiler
		dxguid
)

if (MINGW)
	target_link_options(
			${PROJECT_NAME} PRIVATE
			-municode
	)
endif (MINGW)

set_target_properties(
		${PROJECT_NAME} PROPERTIES
		WIN32_EXECUTABLE TRUE
		VS_DPI_AWARE "PerMonitor"
		VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
# Tell Visual Studio to use WinSpoutDXreceiver as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT WinSpoutDXreceiver)
endif()



