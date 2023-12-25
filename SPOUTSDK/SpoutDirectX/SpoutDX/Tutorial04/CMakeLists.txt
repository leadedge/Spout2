#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Spout Tutorial04 CMakeList          #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
# 10/12/23 : Add DPI awareness           |                                     #
#          : Copy binaries to the BUILD/Binaries/Examples folder.              #
# 12/12/23 : Remove target_link_libraries "Spout". Not used.                   #
#          : Add specific flags for MingW (vkedwardli)                         #
#/-------------------------------------- . -----------------------------------\#

add_executable(Tutorial04 WIN32
  Tutorial04.cpp
  Tutorial04.rc
  Resource.h
)

if(MINGW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -municode")
  include_directories(Tutorial04 "C:/msys64/mingw64/include/directxmath")
endif()

target_compile_definitions(Tutorial04
  PRIVATE
    SPOUT_IMPORT_DLL
  _UNICODE
  UNICODE
)

target_link_libraries(Tutorial04
  PRIVATE
    SpoutDX
    d3d11
    d3dcompiler
    dxguid
)

if(MINGW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -municode")
  include_directories(Tutorial04 "C:/msys64/mingw64/include/directxmath")
endif()

# Set dpi awareness
set_property(TARGET Tutorial04 PROPERTY VS_DPI_AWARE "PerMonitor")

# Copy binaries to the BUILD/Binaries/Examples folder
add_custom_command(TARGET Tutorial04 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ConfigOutputDirectory}/Tutorial04.exe ${CMAKE_BINARY_DIR}/Binaries/Examples/Tutorial04.exe )

add_custom_command(TARGET Tutorial04 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Tutorial04.fx ${CMAKE_BINARY_DIR}/Binaries/Examples/Tutorial04.fx ) 

add_custom_command(TARGET Tutorial04 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/Binaries/${OUTPUT_ARCH_DEST}/SpoutDX.dll ${CMAKE_BINARY_DIR}/Binaries/Examples/SpoutDX.dll ) 
