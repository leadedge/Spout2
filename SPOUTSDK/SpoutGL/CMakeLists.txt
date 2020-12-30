set(SpoutSources
  Spout.h
  SpoutCommon.h
  SpoutCopy.h
  SpoutDirectX.h
  SpoutFrameCount.h
  SpoutGL.h
  SpoutGLextensions.h
  SpoutReceiver.h
  SpoutSDK.h
  SpoutSender.h
  SpoutSenderNames.h
  SpoutSharedMemory.h
  SpoutUtils.h
  SpoutCopy.cpp
  SpoutDirectX.cpp
  SpoutFrameCount.cpp
  SpoutGL.cpp
  SpoutGLextensions.cpp
  SpoutReceiver.cpp
  SpoutSDK.cpp
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
    advapi32
    shell32
    ole32
    oleaut32
    uuid
    odbc32
    odbccp32
    d3d9
    d3d11
    DXGI
    shlwapi
    Version
)

add_library(Spout STATIC ${SpoutSources} )
target_link_libraries(Spout PRIVATE  ${SpoutLink} )
target_compile_definitions(Spout PRIVATE SPOUT_BUILD_DLL)

add_library(SpoutSDK SHARED ${SpoutSources} )
target_link_libraries(SpoutSDK PRIVATE  ${SpoutLink} )
target_compile_definitions(SpoutSDK PRIVATE SPOUT_BUILD_DLL)

if(NOT MSVC)
  target_compile_options(Spout PRIVATE -msse4)
  target_compile_options(SpoutSDK PRIVATE -msse4)
endif()

# Win32 or x64 build
if(CMAKE_SIZEOF_VOID_P EQUAL 4)

	# Copy Win32 binaries to the BUILD/Binaries folder
	add_custom_command(TARGET Spout POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:Spout> ${CMAKE_BINARY_DIR}/Binaries/Win32/Static/Spout.lib
	)
	add_custom_command(TARGET SpoutSDK POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutSDK> ${CMAKE_BINARY_DIR}/Binaries/Win32/SpoutSDK.dll
	)
	add_custom_command(TARGET SpoutSDK POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/Release/SpoutSDK.lib ${CMAKE_BINARY_DIR}/Binaries/Win32/SpoutSDK.lib
	)

	# Copy Win32 binaries to the SpoutGL/Binaries folder
	add_custom_command(TARGET Spout POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:Spout> ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Win32/Static/Spout.lib
	)
	add_custom_command(TARGET SpoutSDK POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutSDK> ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Win32/SpoutSDK.dll
	)
	add_custom_command(TARGET SpoutSDK POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/Release/SpoutSDK.lib ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Win32/SpoutSDK.lib
	)

else()

	# Copy x64 binaries to the BUILD/Binaries folder
	add_custom_command(TARGET Spout POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:Spout> ${CMAKE_BINARY_DIR}/Binaries/x64/Static/Spout.lib
	)
	add_custom_command(TARGET SpoutSDK POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutSDK> ${CMAKE_BINARY_DIR}/Binaries/x64/SpoutSDK.dll
	)
	add_custom_command(TARGET SpoutSDK POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/Release/SpoutSDK.lib ${CMAKE_BINARY_DIR}/Binaries/x64/SpoutSDK.lib
	)

	# Copy x64 binaries to the SpoutGL/Binaries folder
	add_custom_command(TARGET Spout POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:Spout> ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/x64/Static/Spout.lib
	)
	add_custom_command(TARGET SpoutSDK POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy $<TARGET_FILE:SpoutSDK> ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/x64/SpoutSDK.dll
	)
	add_custom_command(TARGET SpoutSDK POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E 
		copy ${CMAKE_CURRENT_BINARY_DIR}/Release/SpoutSDK.lib ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/x64/SpoutSDK.lib
	)



endif()






