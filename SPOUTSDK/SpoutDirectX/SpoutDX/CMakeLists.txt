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

# Win32 or x64 build
if(CMAKE_SIZEOF_VOID_P EQUAL 4)

	# Copy Win32 binaries to the BUILD/Binaries folder
	add_custom_command(TARGET SpoutDX POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/Release/SpoutDX.lib ${CMAKE_BINARY_DIR}/Binaries/Win32/SpoutDX.lib
	)
	add_custom_command(TARGET SpoutDX POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutDX> ${CMAKE_BINARY_DIR}/Binaries/Win32/SpoutDX.dll
	)

	# Copy Win32 binaries to the SpoutDX/Binaries folder
	add_custom_command(TARGET SpoutDX POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/Release/SpoutDX.lib ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Win32/SpoutDX.lib
	)
	add_custom_command(TARGET SpoutDX POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutDX> ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Win32/SpoutDX.dll
	)

else ()

	# Copy x64 binaries to the BUILD/Binaries folder
	add_custom_command(TARGET SpoutDX POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/Release/SpoutDX.lib ${CMAKE_BINARY_DIR}/Binaries/x64/SpoutDX.lib
	)
	add_custom_command(TARGET SpoutDX POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutDX> ${CMAKE_BINARY_DIR}/Binaries/x64/SpoutDX.dll
	)

	# Copy x64 binaries to the SpoutDX/Binaries folder
	add_custom_command(TARGET SpoutDX POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/Release/SpoutDX.lib ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/x64/SpoutDX.lib
	)
	add_custom_command(TARGET SpoutDX POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutDX> ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/x64/SpoutDX.dll
	)



endif()
