#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Spout DX Sender CMakeList           #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#

add_executable(WinSpoutDXSender WIN32
  framework.h
  targetver.h
  Resource.h
  WinSpoutDX.h
  WinSpoutDX.cpp
  WinSpoutDX.rc
)

target_link_libraries(WinSpoutDXSender
  PRIVATE 
    Spout
)

target_compile_definitions(WinSpoutDXSender
  PRIVATE
    SPOUT_IMPORT_DLL
  _UNICODE
  UNICODE
)

target_link_libraries(WinSpoutDXSender
  PRIVATE
    Spout
    SpoutDX
    d3d11
    d3dcompiler
    dxguid
)

add_custom_command(TARGET WinSpoutDXSender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/koala-on-tree.bmp
          "${CMAKE_CURRENT_BINARY_DIR}")