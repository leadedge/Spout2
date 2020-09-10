#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Spout DX Receiver CMakeList           #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#

add_executable(WinSpoutDXReceiver WIN32
  framework.h
  targetver.h
  Resource.h
  WinSpoutDX.h
  WinSpoutDX.cpp
  WinSpoutDX.rc
)

target_link_libraries(WinSpoutDXReceiver
  PRIVATE 
    Spout
)

target_compile_definitions(WinSpoutDXReceiver
  PRIVATE
    SPOUT_IMPORT_DLL
  _UNICODE
  UNICODE
)

target_link_libraries(WinSpoutDXReceiver
  PRIVATE
    Spout
    SpoutDX
    d3d11
    d3dcompiler
    dxguid
)
