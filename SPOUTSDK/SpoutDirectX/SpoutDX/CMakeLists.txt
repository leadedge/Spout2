#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutDX CMakeList                   #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
# Modifications : spout.zeal.co                                                #
# 29/04/21 - Add copy of build files to Binary folders                         #
#          - Support single config generators (ninja, etc) by Joakim Kilby     #
# 13/01/23 - Add SpoutDX static library                                        #
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


get_property(GeneratorisMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(GeneratorisMultiConfig)
	set(ConfigOutputDirectory $<CONFIG>/)
endif()

# Win32 or x64 build
if(CMAKE_SIZEOF_VOID_P EQUAL 4)

	# Copy Win32 binaries to the BUILD/Binaries folder
	add_custom_command(TARGET SpoutDX POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/${ConfigOutputDirectory}SpoutDX.lib ${CMAKE_BINARY_DIR}/Binaries/Win32/SpoutDX.lib
	)
	add_custom_command(TARGET SpoutDX POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutDX> ${CMAKE_BINARY_DIR}/Binaries/Win32/SpoutDX.dll
	)
	
	# Spout_static.lib
	add_custom_command(TARGET SpoutDX_static POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutDX_static> ${CMAKE_BINARY_DIR}/Binaries/Win32/SpoutDX_static.lib
	)
	
else()

	# Copy x64 binaries to the BUILD/Binaries folder
	add_custom_command(TARGET SpoutDX POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/${ConfigOutputDirectory}SpoutDX.lib ${CMAKE_BINARY_DIR}/Binaries/x64/SpoutDX.lib
	)
	add_custom_command(TARGET SpoutDX POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutDX> ${CMAKE_BINARY_DIR}/Binaries/x64/SpoutDX.dll
	)
	
	# Spout_static.lib
	add_custom_command(TARGET SpoutDX_static POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutDX_static> ${CMAKE_BINARY_DIR}/Binaries/x64/SpoutDX_static.lib
	)

endif()


# Examples
if(SPOUT_BUILD_SPOUTDX_EXAMPLES)
  add_subdirectory(Tutorial04)
  add_subdirectory(Tutorial07)
  add_subdirectory(Windows/Sender)
  add_subdirectory(Windows/Receiver)
endif()
