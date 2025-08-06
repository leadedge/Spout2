#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | Spout CMakeList                     #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#
# Modifications : spout.zeal.co                                                #
# 28/12/20 - Add additional SpoutSDK library for dynamic build                 #
# 29/12/20 - Add Win32 or x64 build                                            #
# 30/12/20 - Add copy of build files to Binary folders                         #
# 15/01/21 - Rename SpoutSDK files to Spout                                    #
#            Generate Spout.dll instead of SpoutSDK.dll                        #
# 02/02/21 - Support single config generators (ninja, etc) by Joakim Kilby     #
# 24/06/23 - Modifications for MINGW build (PR#93-vkedwardli)                  #
# 08/07/23 - Remove MSVC Universal C Runtime dependency (PR#95-vkedwardli)     #
# 09/08/23 - Add comctl32 to SpoutLink library list (PR#96-scribam)            #
# 16/12/23 - Remove shlwapi from Spoutlink list                                #
# 20/12/23 - SpoutMessageBox changes in SpoutUtils for MinGW build             #
#/-------------------------------------- . -----------------------------------\#

set(SpoutSources
  Spout.h
  SpoutCommon.h
  SpoutCopy.h
  SpoutDirectX.h
  SpoutFrameCount.h
  SpoutGL.h
  SpoutGLextensions.h
  SpoutReceiver.h
  SpoutSender.h
  SpoutSenderNames.h
  SpoutSharedMemory.h
  SpoutUtils.h
  Spout.cpp
  SpoutCopy.cpp
  SpoutDirectX.cpp
  SpoutFrameCount.cpp
  SpoutGL.cpp
  SpoutGLextensions.cpp
  SpoutReceiver.cpp
  SpoutSender.cpp
  SpoutSenderNames.cpp
  SpoutSharedMemory.cpp
  SpoutUtils.cpp
 )

set(SpoutLink
    opengl32
    kernel32
    user32
    gdi32
    winspool
    comdlg32
    comctl32
    advapi32
    shell32
    ole32
    oleaut32
    uuid
    odbc32
    odbccp32
    d3d9
    d3d11
    dxgi
    version
)

if(MINGW)
  set(SpoutLink ${SpoutLink} winmm)
endif()

add_library(Spout_static STATIC ${SpoutSources} )
target_link_libraries(Spout_static PRIVATE  ${SpoutLink} )
target_compile_definitions(Spout_static PRIVATE SPOUT_BUILD_DLL)

add_library(Spout SHARED ${SpoutSources} )
target_link_libraries(Spout PRIVATE  ${SpoutLink} )
target_compile_definitions(Spout PRIVATE SPOUT_BUILD_DLL)

if(NOT MSVC)
  target_compile_options(Spout_static PRIVATE -msse4)
  target_compile_options(Spout PRIVATE -msse4)
endif()

get_property(GeneratorisMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(GeneratorisMultiConfig)
	set(ConfigOutputDirectory $<CONFIG>/)
endif()

set(SPOUT_OUTPUT_STATIC_LIB "${SPOUT_OUTPUT_STATIC_LIB_PREFIX}Spout${SPOUT_OUTPUT_STATIC_LIB_SUFFIX}")
set(SPOUT_OUTPUT_SHARED_LIB "${SPOUT_OUTPUT_SHARED_LIB_PREFIX}Spout${SPOUT_OUTPUT_SHARED_LIB_SUFFIX}")
set(SPOUT_OUTPUT_IMPORT_LIB "${SPOUT_OUTPUT_IMPORT_LIB_PREFIX}Spout${SPOUT_OUTPUT_IMPORT_LIB_SUFFIX}")

# Copy binaries to the BUILD/Binaries folder
# Spout
add_custom_command(TARGET Spout POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy $<TARGET_FILE:Spout> ${CMAKE_BINARY_DIR}/Binaries/${SPOUT_OUTPUT_ARCH_DEST}/${SPOUT_OUTPUT_SHARED_LIB}
)
add_custom_command(TARGET Spout POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy ${CMAKE_CURRENT_BINARY_DIR}/${ConfigOutputDirectory}${SPOUT_OUTPUT_IMPORT_LIB} ${CMAKE_BINARY_DIR}/Binaries/${SPOUT_OUTPUT_ARCH_DEST}/${SPOUT_OUTPUT_IMPORT_LIB}
)

# Spout_static
add_custom_command(TARGET Spout_static POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E 
  copy $<TARGET_FILE:Spout_static> ${CMAKE_BINARY_DIR}/Binaries/${SPOUT_OUTPUT_ARCH_DEST}/${SPOUT_OUTPUT_STATIC_LIB}
)



