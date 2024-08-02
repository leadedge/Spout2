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

# Set dpi awareness
set_property(TARGET WinSpoutDXreceiver PROPERTY VS_DPI_AWARE "PerMonitor")

# Copy binaries to the BUILD/Binaries/Examples folder
add_custom_command(TARGET WinSpoutDXreceiver POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ConfigOutputDirectory}/WinSpoutDXreceiver.exe ${CMAKE_BINARY_DIR}/Binaries/Examples/WinSpoutDXreceiver.exe )

add_custom_command(TARGET WinSpoutDXreceiver POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/Binaries/${OUTPUT_ARCH_DEST}/SpoutDX.dll ${CMAKE_BINARY_DIR}/Binaries/Examples/SpoutDX.dll ) 		  

