#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutDX Receiver CMakeList          #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#
# Modifications : spout.zeal.co                                                #
# 06/01/21 - DPI awareness needed for scaled displays                          #
#/-------------------------------------- . -----------------------------------\#

add_executable(WinSpoutDXreceiver WIN32
  framework.h
  targetver.h
  Resource.h
  WinSpoutDX.h
  WinSpoutDX.cpp
  WinSpoutDX.rc
)

target_link_libraries(WinSpoutDXreceiver
  PRIVATE 
    Spout_static
)

# DPI awareness needed for scaled displays
set_property(TARGET WinSpoutDXreceiver PROPERTY VS_DPI_AWARE "PerMonitor")

target_compile_definitions(WinSpoutDXreceiver
  PRIVATE
    SPOUT_IMPORT_DLL
  _UNICODE
  UNICODE
)

target_link_libraries(WinSpoutDXreceiver
  PRIVATE
    Spout_static
    SpoutDX
    d3d11
    d3dcompiler
    dxguid
)
