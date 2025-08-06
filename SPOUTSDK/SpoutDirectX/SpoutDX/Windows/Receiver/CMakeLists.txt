#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Spout DX Receiver CMakeList         #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
# 10/12/23 : Add DPI awareness           |                                     #
#          : Copy binaries to the BUILD/Binaries/Examples folder.              #
# 12/12/23 : Remove target_link_libraries "Spout". Not used.                   #
#/-------------------------------------- . -----------------------------------\#

add_executable(WinSpoutDXreceiver WIN32
  framework.h
  targetver.h
  Resource.h
  WinSpoutDX.h
  WinSpoutDX.cpp
  WinSpoutDX.rc
)

target_compile_definitions(WinSpoutDXreceiver
  PRIVATE
    SPOUT_IMPORT_DLL
  _UNICODE
  UNICODE
)

target_link_libraries(WinSpoutDXreceiver
  PRIVATE
    SpoutDX
    d3d11
    d3dcompiler
    dxguid
)

if (MINGW)
  target_link_options(WinSpoutDXreceiver PRIVATE -municode)
endif ()

# Set dpi awareness
set_property(TARGET WinSpoutDXreceiver PROPERTY VS_DPI_AWARE "PerMonitor")

set(SPOUT_OUTPUT_STATIC_LIB "${SPOUT_OUTPUT_STATIC_LIB_PREFIX}SpoutDX${SPOUT_OUTPUT_STATIC_LIB_SUFFIX}")
set(SPOUT_OUTPUT_SHARED_LIB "${SPOUT_OUTPUT_SHARED_LIB_PREFIX}SpoutDX${SPOUT_OUTPUT_SHARED_LIB_SUFFIX}")
set(SPOUT_OUTPUT_IMPORT_LIB "${SPOUT_OUTPUT_IMPORT_LIB_PREFIX}SpoutDX${SPOUT_OUTPUT_IMPORT_LIB_SUFFIX}")

# Copy binaries to the BUILD/Binaries/Examples folder
add_custom_command(TARGET WinSpoutDXreceiver POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ConfigOutputDirectory}/WinSpoutDXreceiver.exe ${CMAKE_BINARY_DIR}/Binaries/Examples/WinSpoutDXreceiver.exe )

add_custom_command(TARGET WinSpoutDXreceiver POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/Binaries/${SPOUT_OUTPUT_ARCH_DEST}/${SPOUT_OUTPUT_SHARED_LIB} ${CMAKE_BINARY_DIR}/Binaries/Examples/${SPOUT_OUTPUT_SHARED_LIB} )

