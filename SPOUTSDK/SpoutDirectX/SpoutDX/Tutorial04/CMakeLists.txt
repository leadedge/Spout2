#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Spout Tutorial04 CMakeList          #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#

add_executable(Tutorial04 WIN32
  Tutorial04.cpp
  Tutorial04.rc
  Resource.h
  #Tutorial04_PS.hlsl
  #Tutorial04_VS.hlsl
)

# DPI awareness needed for scaled displays
set_property(TARGET Tutorial04 PROPERTY VS_DPI_AWARE "PerMonitor")

target_link_libraries(Tutorial04
  PRIVATE 
    Spout_static
)

target_compile_definitions(Tutorial04
  PRIVATE
    SPOUT_IMPORT_DLL
  _UNICODE
  UNICODE
)

target_link_libraries(Tutorial04
  PRIVATE
    Spout_static
    SpoutDX
    d3d11
    d3dcompiler
    dxguid
)

add_custom_command(TARGET Tutorial04 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/Tutorial04.fx
          "${CMAKE_CURRENT_BINARY_DIR}")