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

# Win32 or x64 build
if(CMAKE_SIZEOF_VOID_P EQUAL 4)

	# Copy Win32 binaries to the BUILD/Binaries folder
	add_custom_command(TARGET SpoutLibrary POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/Release/SpoutLibrary.lib ${CMAKE_BINARY_DIR}/Binaries/Win32/SpoutLibrary.lib
	)
	add_custom_command(TARGET SpoutLibrary POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutLibrary> ${CMAKE_BINARY_DIR}/Binaries/Win32/SpoutLibrary.dll
	)

	# Copy Win32 binaries to the SpoutLibrary/Binaries folder
	add_custom_command(TARGET SpoutLibrary POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/Release/SpoutLibrary.lib ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Win32/SpoutLibrary.lib
	)
	add_custom_command(TARGET SpoutLibrary POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutLibrary> ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Win32/SpoutLibrary.dll
	)

else()

	# Copy x64 binaries to the BUILD/Binaries folder
	add_custom_command(TARGET SpoutLibrary POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/Release/SpoutLibrary.lib ${CMAKE_BINARY_DIR}/Binaries/x64/SpoutLibrary.lib
	)
	add_custom_command(TARGET SpoutLibrary POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutLibrary> ${CMAKE_BINARY_DIR}/Binaries/x64/SpoutLibrary.dll
	)

	# Copy x64 binaries to the SpoutLibrary/Binaries folder
	add_custom_command(TARGET SpoutLibrary POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/Release/SpoutLibrary.lib ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/x64/SpoutLibrary.lib
	)
	add_custom_command(TARGET SpoutLibrary POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutLibrary> ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/x64/SpoutLibrary.dll
	)

endif()

# Win32 or x64 build
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
else()

endif()



