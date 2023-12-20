#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Spout DX Sender CMakeList           #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
# 10/12/23 : Add DPI awareness           |                                     #
#          : Copy binaries to the BUILD/Binaries/Examples folder.              #
# 12/12/23 : Remove target_link_libraries "Spout". Not used.                   #
#/-------------------------------------- . -----------------------------------\#

add_executable(WinSpoutDXSender WIN32
  framework.h
  targetver.h
  Resource.h
  WinSpoutDX.h
  WinSpoutDX.cpp
  WinSpoutDX.rc
)

target_compile_definitions(WinSpoutDXSender
  PRIVATE
    SPOUT_IMPORT_DLL
  _UNICODE
  UNICODE
)

target_link_libraries(WinSpoutDXSender
  PRIVATE
    SpoutDX
    d3d11
    d3dcompiler
    dxguid
)

# Set dpi awareness
set_property(TARGET WinSpoutDXSender PROPERTY VS_DPI_AWARE "PerMonitor")

# Copy binaries to the BUILD/Binaries/Examples folder
add_custom_command(TARGET WinSpoutDXSender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ConfigOutputDirectory}/WinSpoutDXSender.exe ${CMAKE_BINARY_DIR}/Binaries/Examples/ )

add_custom_command(TARGET WinSpoutDXSender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/koala-on-tree.bmp ${CMAKE_BINARY_DIR}/Binaries/Examples/ ) 

add_custom_command(TARGET WinSpoutDXSender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/Binaries/${OUTPUT_ARCH_DEST}/SpoutDX.dll ${CMAKE_BINARY_DIR}/Binaries/Examples/ ) 		  
		  