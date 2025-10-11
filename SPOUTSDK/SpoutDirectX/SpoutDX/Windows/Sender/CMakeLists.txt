#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Spout DX Sender CMakeList           #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
# 10/12/23 : Add DPI awareness           |                                     #
#          : Copy binaries to the BUILD/Binaries/Examples folder.              #
# 12/12/23 : Remove target_link_libraries "Spout". Not used.                   #
#/-------------------------------------- . -----------------------------------\#

cmake_minimum_required(VERSION 3.15)
project(WinSpoutDXsender)

set(WinSpoutDXsender_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/WinSpoutDX.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WinSpoutDX.rc
)
set(WinSpoutDXsender_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/framework.h
    ${CMAKE_CURRENT_SOURCE_DIR}/targetver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Resource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WinSpoutDX.h
)

add_executable(
        ${PROJECT_NAME} WIN32
        ${WinSpoutDXsender_SOURCES}
        ${WinSpoutDXsender_HEADERS}
)
target_compile_definitions(
        ${PROJECT_NAME} PRIVATE
        _UNICODE
        UNICODE
)
target_link_libraries(
        ${PROJECT_NAME} PRIVATE
        SpoutDX
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
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries/Examples
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/koala-on-tree-640.bmp $<TARGET_FILE_DIR:${PROJECT_NAME}> )
