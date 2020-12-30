#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Main Spout CMakeList                #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#

add_library(SpoutLibrary SHARED 
  SpoutLibrary.h
  SpoutLibrary.cpp
)

target_link_libraries(SpoutLibrary
  PRIVATE 
    Spout
)

target_compile_definitions(SpoutLibrary
  PRIVATE
    SPOUT_BUILD_DLL
)