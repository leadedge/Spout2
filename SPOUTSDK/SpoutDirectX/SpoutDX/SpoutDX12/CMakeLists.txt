#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeLists.txt              | SpoutDX12 CMakeList                 #
# Author   : Lynn Jarvis                 |                                     #
# Started  : 19/07/2024                  |                                     #
# 21/07/24 - Remove SpoutGL cpp files from SpoutDX12sources                    #
#/-------------------------------------- . -----------------------------------\#


#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX12 static library
#
set(SpoutDX12sources
  SpoutDX12.h
  SpoutDX12.cpp
  ../SpoutDX.h
  ../SpoutDX.cpp
  ../../../SpoutGL/SpoutCommon.h
  ../../../SpoutGL/SpoutCopy.h
  ../../../SpoutGL/SpoutDirectX.h
  ../../../SpoutGL/SpoutFrameCount.h
  ../../../SpoutGL/SpoutSenderNames.h
  ../../../SpoutGL/SpoutSharedMemory.h
  ../../../SpoutGL/SpoutUtils.h
  ../../../SpoutGL/SpoutCopy.cpp
  ../../../SpoutGL/SpoutDirectX.cpp
  ../../../SpoutGL/SpoutFrameCount.cpp
  ../../../SpoutGL/SpoutSenderNames.cpp
  ../../../SpoutGL/SpoutSharedMemory.cpp
  ../../../SpoutGL/SpoutUtils.cpp
)

add_library(SpoutDX12_static STATIC ${SpoutDX12sources} )
target_compile_definitions(SpoutDX12_static PRIVATE SPOUT_BUILD_DLL )
#/-------------------------------------- . -----------------------------------\#


#/-------------------------------------- . -----------------------------------\#
#
# SpoutDX12 dynamic link library
#
add_library(SpoutDX12 SHARED 
  SpoutDX12.h
  SpoutDX12.cpp
  ../SpoutDX.h
  ../SpoutDX.cpp  
)

target_link_libraries(SpoutDX12
   PRIVATE 
     Spout_static
     d3d12
)

target_compile_definitions(SpoutDX12
  PRIVATE
    SPOUT_BUILD_DLL
    SPOUTLIBRARY_EXPORTS
)
#/-------------------------------------- . -----------------------------------\#

if(NOT MSVC)
  target_compile_options(SpoutDX12_static PRIVATE -msse4)
  target_compile_options(SpoutDX12 PRIVATE -msse4)
endif()

get_property(GeneratorisMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(GeneratorisMultiConfig)
	set(ConfigOutputDirectory $<CONFIG>/)
endif()

set(SPOUT_OUTPUT_STATIC_LIB "${SPOUT_OUTPUT_STATIC_LIB_PREFIX}SpoutDX12${SPOUT_OUTPUT_STATIC_LIB_SUFFIX}")
set(SPOUT_OUTPUT_SHARED_LIB "${SPOUT_OUTPUT_SHARED_LIB_PREFIX}SpoutDX12${SPOUT_OUTPUT_SHARED_LIB_SUFFIX}")
set(SPOUT_OUTPUT_IMPORT_LIB "${SPOUT_OUTPUT_IMPORT_LIB_PREFIX}SpoutDX12${SPOUT_OUTPUT_IMPORT_LIB_SUFFIX}")

#
# Copy binaries to the BUILD/Binaries folder
#

# SpoutDX12 dll
add_custom_command(TARGET SpoutDX12 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy ${CMAKE_CURRENT_BINARY_DIR}/${ConfigOutputDirectory}${SPOUT_OUTPUT_IMPORT_LIB} ${CMAKE_BINARY_DIR}/Binaries/${SPOUT_OUTPUT_ARCH_DEST}/${SPOUT_OUTPUT_IMPORT_LIB}
)
add_custom_command(TARGET SpoutDX12 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy $<TARGET_FILE:SpoutDX12> ${CMAKE_BINARY_DIR}/Binaries/${SPOUT_OUTPUT_ARCH_DEST}/${SPOUT_OUTPUT_SHARED_LIB}
)

# SpoutDX12_static library
add_custom_command(TARGET SpoutDX12_static POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy $<TARGET_FILE:SpoutDX12_static> ${CMAKE_BINARY_DIR}/Binaries/${SPOUT_OUTPUT_ARCH_DEST}/${SPOUT_OUTPUT_STATIC_LIB}
)
