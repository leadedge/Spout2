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

set(SPOUT_OUTPUT_STATIC_LIB "${SPOUT_OUTPUT_STATIC_LIB_PREFIX}SpoutDX9${SPOUT_OUTPUT_STATIC_LIB_SUFFIX}")
set(SPOUT_OUTPUT_SHARED_LIB "${SPOUT_OUTPUT_SHARED_LIB_PREFIX}SpoutDX9${SPOUT_OUTPUT_SHARED_LIB_SUFFIX}")
set(SPOUT_OUTPUT_IMPORT_LIB "${SPOUT_OUTPUT_IMPORT_LIB_PREFIX}SpoutDX9${SPOUT_OUTPUT_IMPORT_LIB_SUFFIX}")

# Copy binaries to the BUILD/Binaries folder
# SpoutDX9
add_custom_command(TARGET SpoutDX9 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy ${CMAKE_CURRENT_BINARY_DIR}/${ConfigOutputDirectory}${SPOUT_OUTPUT_IMPORT_LIB} ${CMAKE_BINARY_DIR}/Binaries/${SPOUT_OUTPUT_ARCH_DEST}/${SPOUT_OUTPUT_IMPORT_LIB}
)
add_custom_command(TARGET SpoutDX9 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy $<TARGET_FILE:SpoutDX9> ${CMAKE_BINARY_DIR}/Binaries/${SPOUT_OUTPUT_ARCH_DEST}/${SPOUT_OUTPUT_SHARED_LIB}
)

# SpoutDX9_static
add_custom_command(TARGET SpoutDX9_static POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy $<TARGET_FILE:SpoutDX9_static> ${CMAKE_BINARY_DIR}/Binaries/${SPOUT_OUTPUT_ARCH_DEST}/${SPOUT_OUTPUT_STATIC_LIB}
)
