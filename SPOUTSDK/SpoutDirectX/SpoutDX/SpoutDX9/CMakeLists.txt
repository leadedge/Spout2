#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutDX9 CMakesList                 #
# Author   : Lynn Jarvis                 |                                     #
# Started  : 21/07/2024                  |                                     #
#/-------------------------------------- . -----------------------------------\#


#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX9 static library
#
set(SpoutDX9sources
  SpoutDX9.h
  SpoutDX9.cpp
  ../../../SpoutGL/SpoutCommon.h
  ../../../SpoutGL/SpoutCopy.h
  ../../../SpoutGL/SpoutFrameCount.h
  ../../../SpoutGL/SpoutSenderNames.h
  ../../../SpoutGL/SpoutSharedMemory.h
  ../../../SpoutGL/SpoutUtils.h
  ../../../SpoutGL/SpoutCopy.cpp
  ../../../SpoutGL/SpoutFrameCount.cpp
  ../../../SpoutGL/SpoutSenderNames.cpp
  ../../../SpoutGL/SpoutSharedMemory.cpp
  ../../../SpoutGL/SpoutUtils.cpp
)

add_library(SpoutDX9_static STATIC ${SpoutDX9sources} )
target_compile_definitions(SpoutDX9_static PRIVATE SPOUT_BUILD_DLL )
#/-------------------------------------- . -----------------------------------\#


#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX9 dynamic link library
#
add_library(SpoutDX9 SHARED 
  SpoutDX9.h
  SpoutDX9.cpp
)

target_link_libraries(SpoutDX9
PRIVATE 
     Spout_static
)

target_compile_definitions(SpoutDX9
  PRIVATE
    SPOUT_BUILD_DLL
    SPOUTLIBRARY_EXPORTS
)
#/-------------------------------------- . -----------------------------------\#

if(NOT MSVC)
  target_compile_options(SpoutDX9_static PRIVATE -msse4)
  target_compile_options(SpoutDX9 PRIVATE -msse4)
endif()

get_property(GeneratorisMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(GeneratorisMultiConfig)
	set(ConfigOutputDirectory $<CONFIG>/)
endif()

# Win32 or x64 build
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(OUTPUT_ARCH_DEST "Win32")
else()
  set(OUTPUT_ARCH_DEST "x64")
endif()

if(MINGW)
  set(OUTPUT_STATIC_LIB "libSpoutDX9_static.a")
  set(OUTPUT_SHARED_LIB "libSpoutDX9.dll")
  set(OUTPUT_IMPORT_LIB "libSpoutDX9.dll.a")
else()
  set(OUTPUT_STATIC_LIB "SpoutDX9_static.lib")
  set(OUTPUT_SHARED_LIB "SpoutDX9.dll")
  set(OUTPUT_IMPORT_LIB "SpoutDX9.lib")
endif()

# Copy binaries to the BUILD/Binaries folder
# SpoutDX9
add_custom_command(TARGET SpoutDX9 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy ${CMAKE_CURRENT_BINARY_DIR}/${ConfigOutputDirectory}${OUTPUT_IMPORT_LIB} ${CMAKE_BINARY_DIR}/Binaries/${OUTPUT_ARCH_DEST}/${OUTPUT_IMPORT_LIB}
)
add_custom_command(TARGET SpoutDX9 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy $<TARGET_FILE:SpoutDX9> ${CMAKE_BINARY_DIR}/Binaries/${OUTPUT_ARCH_DEST}/${OUTPUT_SHARED_LIB}
)

# SpoutDX9_static
add_custom_command(TARGET SpoutDX9_static POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy $<TARGET_FILE:SpoutDX9_static> ${CMAKE_BINARY_DIR}/Binaries/${OUTPUT_ARCH_DEST}/${OUTPUT_STATIC_LIB}
)
