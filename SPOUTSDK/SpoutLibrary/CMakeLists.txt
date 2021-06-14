#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeList.txt               | SpoutLibrary CMakeList              #
# Author   : Alexandre Buge              |                                     #
# Started  : 08/09/2020 12:00            |                                     #
#/-------------------------------------- . -----------------------------------\#
# Modifications : spout.zeal.co                                                #
# 29/12/20 - Add Win32 or x64 build                                            #
# 30/12/20 - Add copy of build files to Binary folders                         #
# 02/02/21 - Support single config generators (ninja, etc) by Joakim Kilby     #
#/-------------------------------------- . -----------------------------------\#                                      #

add_library(SpoutLibrary SHARED 
  SpoutLibrary.h
  SpoutLibrary.cpp
)

target_link_libraries(SpoutLibrary
  PRIVATE 
    Spout_static
)

target_compile_definitions(SpoutLibrary
  PRIVATE
    SPOUT_BUILD_DLL
)

get_property(GeneratorisMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(GeneratorisMultiConfig)
	set(ConfigOutputDirectory $<CONFIG>/)
endif()

# Win32 or x64 build
if(CMAKE_SIZEOF_VOID_P EQUAL 4)

	# Copy Win32 binaries to the BUILD/Binaries folder
	add_custom_command(TARGET SpoutLibrary POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/${ConfigOutputDirectory}SpoutLibrary.lib ${CMAKE_BINARY_DIR}/Binaries/Win32/SpoutLibrary.lib
	)
	add_custom_command(TARGET SpoutLibrary POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutLibrary> ${CMAKE_BINARY_DIR}/Binaries/Win32/SpoutLibrary.dll
	)

else()

	# Copy x64 binaries to the BUILD/Binaries folder
	add_custom_command(TARGET SpoutLibrary POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/${ConfigOutputDirectory}SpoutLibrary.lib ${CMAKE_BINARY_DIR}/Binaries/x64/SpoutLibrary.lib
	)
	add_custom_command(TARGET SpoutLibrary POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutLibrary> ${CMAKE_BINARY_DIR}/Binaries/x64/SpoutLibrary.dll
	)

endif()



