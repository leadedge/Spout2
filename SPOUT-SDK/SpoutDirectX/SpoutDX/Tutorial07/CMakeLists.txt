#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Spout Tutorial04 CMakeList          #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
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

target_link_libraries(Tutorial07
  PRIVATE 
    Spout
)

target_compile_definitions(Tutorial07
  PRIVATE
    SPOUT_IMPORT_DLL
  _UNICODE
  UNICODE
)

target_link_libraries(Tutorial07
  PRIVATE
    Spout
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