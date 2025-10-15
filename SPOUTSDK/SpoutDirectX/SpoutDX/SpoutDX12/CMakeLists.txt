#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeLists.txt              | SpoutDX12 CMakeList                 #
# Author   : Lynn Jarvis                 |                                     #
# Started  : 19/07/2024                  |                                     #
# 21/07/24 - Remove SpoutGL cpp files from SpoutDX12sources                    #
# 14/08/25 : mingw-w64 Linux cross build compatibility PR #122                 #
# 11/10/25 - mingw-w64 Linux cross build compatibility                         #
#            Refactor CMake - PR #122                                          #
#/-------------------------------------- . -----------------------------------\#

cmake_minimum_required(VERSION 3.15)
project(SpoutDX12)

# Source and header files
set(SpoutDX12_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/SpoutDX12.cpp
)
set(SpoutDX12_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/SpoutDX12.h
)

#--------------------------------------
# SpoutDX12 static library
#--------------------------------------
add_library(SpoutDX12_static STATIC ${SpoutDX12_SOURCES} ${SpoutDX12_HEADERS})
target_compile_definitions(SpoutDX12_static PRIVATE SPOUT_BUILD_STATIC)
target_link_libraries(SpoutDX12_static PUBLIC SpoutDX_static)
target_include_directories(SpoutDX12_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
set_target_properties(SpoutDX12_static PROPERTIES
    VS_FOLDER "SpoutDX12_static"
    SPOUT_PUBLIC_HEADER "${SpoutDX12_HEADERS}"
    RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#--------------------------------------
# SpoutDX12 shared library (DLL)
#--------------------------------------
add_library(SpoutDX12 SHARED ${SpoutDX12_SOURCES} ${SpoutDX12_HEADERS})
target_compile_definitions(SpoutDX12 PRIVATE SPOUT_BUILD_DLL)
target_link_libraries(SpoutDX12 PRIVATE SpoutDX_static d3d12)
target_include_directories(SpoutDX12 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
set_target_properties(SpoutDX12 PROPERTIES
    VS_FOLDER "SpoutDX12_dll"
    SPOUT_PUBLIC_HEADER "${SpoutDX12_HEADERS}"
    RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#--------------------------------------
# SpoutDX12 install
#--------------------------------------
if (NOT SKIP_INSTALL_ALL)
    if(NOT SKIP_INSTALL_LIBRARIES)
        install(
            TARGETS SpoutDX12 SpoutDX12_static
            EXPORT SpoutTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()

    if(NOT SKIP_INSTALL_HEADERS)
        get_target_property(spoutdx_headers SpoutDX SPOUT_PUBLIC_HEADER)
        install(
            FILES
                ${SpoutDX12_HEADERS}
                ${spoutdx_headers}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SpoutDX12
        )
    endif()
endif()
