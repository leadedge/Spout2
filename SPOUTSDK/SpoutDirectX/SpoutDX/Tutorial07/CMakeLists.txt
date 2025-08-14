#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Spout Tutorial07 CMakeList          #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
# 10/12/23 : Add DPI awareness           |                                     #
#          : Copy binaries to the BUILD/Binaries/Examples folder.              #
# 12/12/23 : Remove target_link_libraries "Spout". Not used.                   #
#          : Add specific flags for MingW (vkedwardli)                         #
#/-------------------------------------- . -----------------------------------\#

cmake_minimum_required(VERSION 3.15)
project(Tutorial07)

find_package(directxmath CONFIG REQUIRED)

set(Tutorial07_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Tutorial07.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Tutorial07.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/DDSTextureLoader.cpp
)
set(Tutorial07_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/DDSTextureLoader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Resource.h
)

add_executable(
        ${PROJECT_NAME} WIN32
        ${Tutorial07_SOURCES}
        ${Tutorial07_HEADERS}
)
target_link_libraries(
        ${PROJECT_NAME} PRIVATE
        SpoutDX
        d3d11
        d3dcompiler
        dxguid
        Microsoft::DirectXMath
)

target_compile_definitions(
        ${PROJECT_NAME} PRIVATE
        _UNICODE
        UNICODE
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
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        VS_DPI_AWARE "PerMonitor"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries/Examples
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/Tutorial07.fx $<TARGET_FILE_DIR:${PROJECT_NAME}> )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/seafloor.dds $<TARGET_FILE_DIR:${PROJECT_NAME}> )

# Copy SpoutDX runtime library (required by your app)
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SpoutDX>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
