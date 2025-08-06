#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutDX CMakeList                   #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
# Modifications : spout.zeal.co                                                #
# 29/04/21 - Add copy of build files to Binary folders                         #
#          - Support single config generators (ninja, etc) by Joakim Kilby     #
# 13/01/23 - Add SpoutDX static library                                        #
# 24/06/23 - Modifications for MINGW build (PR#93-vkedwardli)                  #
# 21/07/24 - Remove SpoutGL cpp files from SpoutDXsources                      #
#/-------------------------------------- . -----------------------------------\#


#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX static library
#
set(SpoutDXsources
  SpoutDX.h
  SpoutDX.cpp
  ../../SpoutGL/SpoutCommon.h
  ../../SpoutGL/SpoutCopy.h
  ../../SpoutGL/SpoutDirectX.h
  ../../SpoutGL/SpoutFrameCount.h
  ../../SpoutGL/SpoutSenderNames.h
  ../../SpoutGL/SpoutSharedMemory.h
  ../../SpoutGL/SpoutUtils.h
  ../../SpoutGL/SpoutCopy.cpp
  ../../SpoutGL/SpoutDirectX.cpp
  ../../SpoutGL/SpoutFrameCount.cpp
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

set(SPOUT_OUTPUT_STATIC_LIB "${SPOUT_OUTPUT_STATIC_LIB_PREFIX}SpoutDX${SPOUT_OUTPUT_STATIC_LIB_SUFFIX}")
set(SPOUT_OUTPUT_SHARED_LIB "${SPOUT_OUTPUT_SHARED_LIB_PREFIX}SpoutDX${SPOUT_OUTPUT_SHARED_LIB_SUFFIX}")
set(SPOUT_OUTPUT_IMPORT_LIB "${SPOUT_OUTPUT_IMPORT_LIB_PREFIX}SpoutDX${SPOUT_OUTPUT_IMPORT_LIB_SUFFIX}")

# Copy binaries to the BUILD/Binaries folder
# SpoutDX
add_custom_command(TARGET SpoutDX POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy ${CMAKE_CURRENT_BINARY_DIR}/${ConfigOutputDirectory}${SPOUT_OUTPUT_IMPORT_LIB} ${CMAKE_BINARY_DIR}/Binaries/${SPOUT_OUTPUT_ARCH_DEST}/${SPOUT_OUTPUT_IMPORT_LIB}
)
add_custom_command(TARGET SpoutDX POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy $<TARGET_FILE:SpoutDX> ${CMAKE_BINARY_DIR}/Binaries/${SPOUT_OUTPUT_ARCH_DEST}/${SPOUT_OUTPUT_SHARED_LIB}
)

# SpoutDX_static
add_custom_command(TARGET SpoutDX_static POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy $<TARGET_FILE:SpoutDX_static> ${CMAKE_BINARY_DIR}/Binaries/${SPOUT_OUTPUT_ARCH_DEST}/${SPOUT_OUTPUT_STATIC_LIB}
)


# Examples
if(SPOUT_BUILD_SPOUTDX_EXAMPLES)
  add_subdirectory(Tutorial04)
  add_subdirectory(Tutorial07)
  add_subdirectory(Windows/Receiver)
  add_subdirectory(Windows/Sender)
endif()
