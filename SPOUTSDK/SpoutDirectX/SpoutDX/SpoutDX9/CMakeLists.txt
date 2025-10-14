#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutDX9 CMakesList                 #
# Author   : Lynn Jarvis                 |                                     #
# Started  : 21/07/2024                  |                                     #
# 14/08/25 : mingw-w64 Linux cross build compatibility PR #122                 #
# 11/10/25 - mingw-w64 Linux cross build compatibility                         #
#            Refactor CMake - PR #122                                          #
#/-------------------------------------- . -----------------------------------\#

cmake_minimum_required(VERSION 3.15)

project(SpoutDX9)

# Source and header files
set(SpoutDX9_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/SpoutDX9.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../../SpoutGL/SpoutFrameCount.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../../SpoutGL/SpoutSenderNames.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../../SpoutGL/SpoutSharedMemory.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../../SpoutGL/SpoutUtils.cpp
)
set(SpoutDX9_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/SpoutDX9.h
	${CMAKE_CURRENT_SOURCE_DIR}/../../../SpoutGL/SpoutCommon.h
	${CMAKE_CURRENT_SOURCE_DIR}/../../../SpoutGL/SpoutFrameCount.h
	${CMAKE_CURRENT_SOURCE_DIR}/../../../SpoutGL/SpoutSenderNames.h
	${CMAKE_CURRENT_SOURCE_DIR}/../../../SpoutGL/SpoutSharedMemory.h
	${CMAKE_CURRENT_SOURCE_DIR}/../../../SpoutGL/SpoutUtils.h
)

#--------------------------------------
# SpoutDX9 static library
#--------------------------------------
add_library(SpoutDX9_static STATIC ${SpoutDX9_SOURCES} ${SpoutDX9_HEADERS})
target_compile_definitions(SpoutDX9_static PRIVATE SPOUT_BUILD_STATIC)
target_link_libraries(SpoutDX9_static PRIVATE Spout_static)
target_include_directories(SpoutDX9_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
set_target_properties(SpoutDX9_static PROPERTIES
    VS_FOLDER "SpoutDX9_static"
    SPOUT_PUBLIC_HEADER "${SpoutDX9_HEADERS}"
    RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#--------------------------------------
# SpoutDX9 shared library (DLL)
#--------------------------------------
add_library(SpoutDX9 SHARED ${SpoutDX9_SOURCES} ${SpoutDX9_HEADERS})
target_compile_definitions(SpoutDX9 PRIVATE SPOUT_BUILD_DLL)
target_link_libraries(SpoutDX9 PRIVATE Spout_static)
target_include_directories(SpoutDX9 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
set_target_properties(SpoutDX9 PROPERTIES
	VS_FOLDER "SpoutDX9"
    SPOUT_PUBLIC_HEADER "${SpoutDX9_HEADERS}"
    RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#--------------------------------------
# SpoutDX9 install
#--------------------------------------
if (NOT SKIP_INSTALL_ALL)
    if(NOT SKIP_INSTALL_LIBRARIES)
        install(
            TARGETS SpoutDX9 SpoutDX9_static
            EXPORT SpoutTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()

    if(NOT SKIP_INSTALL_HEADERS)
        install(
            FILES
                ${SpoutDX9_HEADERS}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SpoutDX9
        )
    endif()
endif()
