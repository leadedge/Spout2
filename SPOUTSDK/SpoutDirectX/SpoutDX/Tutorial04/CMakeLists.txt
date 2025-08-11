#---------------------------------------------------------------#
# Filename : CMakeList.txt  | Spout Tutorial04 CMakeList        #
# Author   : Lynn Jarvis    |                                   #
# Started  : 11/08/2025     |                                   #
#---------------------------------------------------------------#

cmake_minimum_required(VERSION 3.15)
project(Tutorial04)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Warning level
if(MSVC)
    add_compile_options(/W4)
endif()

# Source files
set(SOURCES
    Tutorial04.cpp
    ../SpoutDX.cpp
    ../../../SpoutGL/SpoutCopy.cpp
    ../../../SpoutGL/SpoutDirectX.cpp
    ../../../SpoutGL/SpoutFrameCount.cpp
    ../../../SpoutGL/SpoutSenderNames.cpp
    ../../../SpoutGL/SpoutSharedMemory.cpp
    ../../../SpoutGL/SpoutUtils.cpp
    Tutorial04.rc
)

# Header files
set(HEADERS
    resource.h
    ../SpoutDX.h
    ../../../SpoutGL/SpoutCommon.h
    ../../../SpoutGL/SpoutCopy.h
    ../../../SpoutGL/SpoutDirectX.h
    ../../../SpoutGL/SpoutFrameCount.h
    ../../../SpoutGL/SpoutSenderNames.h
    ../../../SpoutGL/SpoutSharedMemory.h
    ../../../SpoutGL/SpoutUtils.h
)

# Shader files
set(SHADERS
    Tutorial04.fx
    Tutorial04_VS.hlsl
    Tutorial04_PS.hlsl
)

# Add executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    directx.ico
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../
    ../../../SpoutGL
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    d3d11
    d3dcompiler
    dxgi
    dxguid
    psapi
)

# Copy FX file for runtime (required by your app)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/Tutorial04.fx
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Set working directory for Visual Studio (run in output dir)
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

# Use ANSI build by removing Unicode flags instead of redefining them
target_compile_definitions(${PROJECT_NAME} PRIVATE
    -D_UNICODE
    -DUNICODE
)

# Set startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Visual Studio source groups (IDE filters)
source_group("Shaders" FILES ${SHADERS})
source_group("Resource Files" FILES resource.h Tutorial04.rc directx.ico)
source_group("SpoutDX" FILES ../SpoutDX.cpp ../SpoutDX.h)
source_group("SpoutSDK" FILES
    ../../../SpoutGL/SpoutCommon.h
    ../../../SpoutGL/SpoutCopy.cpp
    ../../../SpoutGL/SpoutCopy.h
    ../../../SpoutGL/SpoutDirectX.cpp
    ../../../SpoutGL/SpoutDirectX.h
    ../../../SpoutGL/SpoutFrameCount.cpp
    ../../../SpoutGL/SpoutFrameCount.h
    ../../../SpoutGL/SpoutSenderNames.cpp
    ../../../SpoutGL/SpoutSenderNames.h
    ../../../SpoutGL/SpoutSharedMemory.cpp
    ../../../SpoutGL/SpoutSharedMemory.h
    ../../../SpoutGL/SpoutUtils.cpp
    ../../../SpoutGL/SpoutUtils.h
)

# Optional: for MinGW or MSYS2 builds
if(MINGW)
    target_link_options(Tutorial04 PRIVATE -municode)
    target_include_directories(Tutorial04 PRIVATE "C:/msys64/mingw64/include/directxmath")	
endif()

# Set DPI awareness
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DPI_AWARE "PerMonitor")

