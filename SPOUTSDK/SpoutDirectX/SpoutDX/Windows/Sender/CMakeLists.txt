#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutDX Sender CMakeList #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#
# Modifications : spout.zeal.co                                                #
# 06/01/21 - DPI awareness needed for scaled displays                          #
#/-------------------------------------- . -----------------------------------\#

add_executable(WinSpoutDXsender WIN32
  framework.h
  targetver.h
  Resource.h
  WinSpoutDX.h
  WinSpoutDX.cpp
  WinSpoutDX.rc
)

target_link_libraries(WinSpoutDXsender
  PRIVATE 
    Spout_static
)

# DPI awareness needed for scaled displays
set_property(TARGET WinSpoutDXsender PROPERTY VS_DPI_AWARE "PerMonitor")

target_compile_definitions(WinSpoutDXsender
  PRIVATE
    SPOUT_IMPORT_DLL
  _UNICODE
  UNICODE
)

target_link_libraries(WinSpoutDXsender
  PRIVATE
    Spout_static
    SpoutDX
    d3d11
    d3dcompiler
    dxguid
)

add_custom_command(TARGET WinSpoutDXsender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/koala-on-tree.bmp
          "${CMAKE_CURRENT_BINARY_DIR}")