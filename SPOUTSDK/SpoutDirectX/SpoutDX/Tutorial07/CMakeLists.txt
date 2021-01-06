#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutDX Tutorial07 CMakeList        #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#
# Modifications : spout.zeal.co                                                #
# 06/01/21 - DPI awareness needed for scaled displays                          #
#/-------------------------------------- . -----------------------------------\#

add_executable(Tutorial07 WIN32
  Tutorial07.cpp
  DDSTextureLoader.cpp
  DDSTextureLoader.h
  Tutorial07.rc
  Resource.h
  #Tutorial07_PS.hlsl
  #Tutorial07_VS.hlsl
)

# DPI awareness needed for scaled displays
set_property(TARGET Tutorial07 PROPERTY VS_DPI_AWARE "PerMonitor")

target_link_libraries(Tutorial07
  PRIVATE 
    Spout_static
)

target_compile_definitions(Tutorial07
  PRIVATE
    SPOUT_IMPORT_DLL
  _UNICODE
  UNICODE
)

target_link_libraries(Tutorial07
  PRIVATE
    Spout_static
    SpoutDX
    d3d11
    d3dcompiler
    dxguid
)

add_custom_command(TARGET Tutorial07 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/Tutorial07.fx
          "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_command(TARGET Tutorial07 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/seafloor.dds
          "${CMAKE_CURRENT_BINARY_DIR}")