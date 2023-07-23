#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutDX CMakeList                   #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
# Modifications : spout.zeal.co                                                #
# 29/04/21 - Add copy of build files to Binary folders                         #
#          - Support single config generators (ninja, etc) by Joakim Kilby     #
# 13/01/23 - Add SpoutDX static library                                        #
# 24/06/23 - Modifications for MINGW build (PR#93-vkedwardli)                  #
#/-------------------------------------- . -----------------------------------\#


#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX static library
#
# Build using source files rather than dependent Spout_static libary
#
set(SpoutDXsources
  SpoutDX.h
  SpoutDX.cpp
  ../../SpoutGL/Spout.h
  ../../SpoutGL/SpoutCommon.h
  ../../SpoutGL/SpoutCopy.h
  ../../SpoutGL/SpoutDirectX.h
  ../../SpoutGL/SpoutFrameCount.h
  ../../SpoutGL/SpoutGL.h
  ../../SpoutGL/SpoutGLextensions.h
  ../../SpoutGL/SpoutReceiver.h
  ../../SpoutGL/SpoutSender.h
  ../../SpoutGL/SpoutSenderNames.h
  ../../SpoutGL/SpoutSharedMemory.h
  ../../SpoutGL/SpoutUtils.h
  ../../SpoutGL/Spout.cpp
  ../../SpoutGL/SpoutCopy.cpp
  ../../SpoutGL/SpoutDirectX.cpp
  ../../SpoutGL/SpoutFrameCount.cpp
  ../../SpoutGL/SpoutGL.cpp
  ../../SpoutGL/SpoutGLextensions.cpp
  ../../SpoutGL/SpoutReceiver.cpp
  ../../SpoutGL/SpoutSender.cpp
  ../../SpoutGL/SpoutSenderNames.cpp
  ../../SpoutGL/SpoutSharedMemory.cpp
  ../../SpoutGL/SpoutUtils.cpp
)

add_library(SpoutDX_static STATIC ${SpoutDXsources} )
target_compile_definitions(SpoutDX_static PRIVATE SPOUT_BUILD_DLL )
#/-------------------------------------- . -----------------------------------\#


#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX dynamic link library
#
add_library(SpoutDX SHARED 
  SpoutDX.h
  SpoutDX.cpp
)

target_link_libraries(SpoutDX
   PRIVATE 
     Spout_static
)

target_compile_definitions(SpoutDX
  PRIVATE
    SPOUT_BUILD_DLL
    SPOUTLIBRARY_EXPORTS
)
#/-------------------------------------- . -----------------------------------\#

if(NOT MSVC)
  target_compile_options(SpoutDX_static PRIVATE -msse4)
  target_compile_options(SpoutDX PRIVATE -msse4)
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
  set(OUTPUT_STATIC_LIB "libSpoutDX_static.a")
  set(OUTPUT_SHARED_LIB "libSpoutDX.dll")
  set(OUTPUT_IMPORT_LIB "libSpoutDX.dll.a")
else()
  set(OUTPUT_STATIC_LIB "SpoutDX_static.lib")
  set(OUTPUT_SHARED_LIB "SpoutDX.dll")
  set(OUTPUT_IMPORT_LIB "SpoutDX.lib")
endif()

# Copy binaries to the BUILD/Binaries folder
# SpoutDX
add_custom_command(TARGET SpoutDX POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy ${CMAKE_CURRENT_BINARY_DIR}/${ConfigOutputDirectory}${OUTPUT_IMPORT_LIB} ${CMAKE_BINARY_DIR}/Binaries/${OUTPUT_ARCH_DEST}/${OUTPUT_IMPORT_LIB}
)
add_custom_command(TARGET SpoutDX POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy $<TARGET_FILE:SpoutDX> ${CMAKE_BINARY_DIR}/Binaries/${OUTPUT_ARCH_DEST}/${OUTPUT_SHARED_LIB}
)

# SpoutDX_static
add_custom_command(TARGET SpoutDX_static POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy $<TARGET_FILE:SpoutDX_static> ${CMAKE_BINARY_DIR}/Binaries/${OUTPUT_ARCH_DEST}/${OUTPUT_STATIC_LIB}
)


# Examples
if(SPOUT_BUILD_SPOUTDX_EXAMPLES)
  add_subdirectory(Tutorial04)
  add_subdirectory(Tutorial07)
  add_subdirectory(Windows/Receiver)
  add_subdirectory(Windows/Sender)
endif()
