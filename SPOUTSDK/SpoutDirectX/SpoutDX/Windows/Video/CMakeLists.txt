#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutDX Video Sender CMakeList      #
# Author   : Alexandre Buge              | Demonstrates using FFmpeg           #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#
# Modifications : spout.zeal.co                                                #
# 06/01/21 - DPI awareness needed for scaled displays                          #
#/-------------------------------------- . -----------------------------------\#

add_executable(WinSpoutDXvideo WIN32
  framework.h
  targetver.h
  Resource.h
  WinSpoutDXvideo.h
  WinSpoutDXvideo.cpp
  WinSpoutDXvideo.rc
)

target_link_libraries(WinSpoutDXvideo
  PRIVATE 
    Spout_static
)

# DPI awareness needed for scaled displays
set_property(TARGET WinSpoutDXvideo PROPERTY VS_DPI_AWARE "PerMonitor")

target_compile_definitions(WinSpoutDXvideo
  PRIVATE
    SPOUT_IMPORT_DLL
  _UNICODE
  UNICODE
)

target_link_libraries(WinSpoutDXvideo
  PRIVATE
    Spout_static
    SpoutDX
    d3d11
    d3dcompiler
    dxguid
)
