add_library(Spout STATIC 
  Spout.h
  SpoutCommon.h
  SpoutCopy.h
  SpoutDirectX.h
  SpoutFrameCount.h
  SpoutGLDXinterop.h
  SpoutGLextensions.h
  SpoutMemoryShare.h
  SpoutReceiver.h
  SpoutSDK.h
  SpoutSender.h
  SpoutSenderNames.h
  SpoutSharedMemory.h
  SpoutUtils.h
  SpoutCopy.cpp
  SpoutDirectX.cpp
  SpoutFrameCount.cpp
  SpoutGLDXinterop.cpp
  SpoutGLextensions.cpp
  SpoutMemoryShare.cpp
  SpoutReceiver.cpp
  SpoutSDK.cpp
  SpoutSender.cpp
  SpoutSenderNames.cpp
  SpoutSharedMemory.cpp
  SpoutUtils.cpp
)

if(NOT MSVC)
  target_compile_options(Spout 
    PRIVATE
      -msse4
  )
endif()

target_compile_definitions(Spout
  PRIVATE
    SPOUT_BUILD_DLL
)

OPTION(SPOUT_OPTION_DISABLE_LEGACY_OPENGL "Disable legacy OpenGL code used for shared texture draw for OpenGL 4 compliance" OFF)
if(SPOUT_OPTION_DISABLE_LEGACY_OPENGL)
  target_compile_definitions(Spout
    PRIVATE
      SPOUT_OPTION_DISABLE_LEGACY_OPENGL
)
endif()

OPTION(SPOUT_OPTION_DISABLE_D3D11_DEVICE_DEBUG "Disable D3D11 Device when Windows 10 SDK is missing" OFF)
if(SPOUT_OPTION_DISABLE_D3D11_DEVICE_DEBUG)
  target_compile_definitions(Spout
    PRIVATE
      SPOUT_OPTION_DISABLE_D3D11_DEVICE_DEBUG
)
endif()

target_link_libraries(Spout
  PRIVATE
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