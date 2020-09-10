#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutDx CMakeList                   #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#

add_library(SpoutDX SHARED 
  SpoutDX.h
  SpoutDX.cpp
)

target_link_libraries(SpoutDX
  PRIVATE 
    Spout
)

target_compile_definitions(SpoutDX
  PRIVATE
    SPOUT_BUILD_DLL
    SPOUTLIBRARY_EXPORTS
)

if(SPOUT_BUILD_EXAMPLES)
  add_subdirectory(Tutorial04)
  add_subdirectory(Tutorial07)
  add_subdirectory(Windows/Sender)
  add_subdirectory(Windows/Receiver)
endif()
