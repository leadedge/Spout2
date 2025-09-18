## OpenGL external memory interop

The Spout library has been developed using the [WGL_NV_DX interop](https://registry.khronos.org/OpenGL/extensions/NV/WGL_NV_DX_interop2.txt) extensions, developed and maintained by Nvidia, for transfer of textures between DirectX and OpenGL.

More recently, [GL_EXT_memory_object](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_external_objects.txt) extensions with multi-vendor support and maintained by the Khronos Group have been released and can also be used as an interop method.

These extensions are based on the concept of memory objects introduced by the Vulkan API and provide potential for future development.

This branch includes source code with the OpenGL memory object extensions used as alternative to the NVidia interop. Changes are limited to SpoutGL.cpp with a minor change to Spout.cpp. Other source files remain the same.

OpenGL memory is used if the NVidia interop fails or the GL memory option is selected using "SpoutSettings". There is no change in functionality and older applications are not affected. Textures are shared between applications using either interop method.

However, the extent of support GL_EXT_memory extensions is possibly low compared to NVidia interop which has been established since 2010, and practical advantage could be minimal. 

The tools provided with this branch allow assessment of compatibility and performance. Reports are welcomed and will help to assess whether this is a useful addition. They can be contributed using the "Report" button from the demo sender or receiver or discussed here by opening an issue.

### Downloads

Download from the BUILD folder

SPOUT-2007-100.zip
1) Demo Sender and receiver with options for 
- selecting interop method
- sending diagnostic reports
2) Settings dialog with options for selecting interop method
- SpoutSettings.exe


Spout-SDK-binaries_2-007-100.zip
- Prebuilt library binary files\
Spout.dll/Spout.lib/Spout_static.lib

### Building with Cmake
Select only the option for SPOUT_BUILD_CMT as required.\
Do not select SPOUT_BUILD_LIBRARY, SPOUT_BUILD_SPOUTDX or SPOUT_BUILD_SPOUTDX_EXAMPLES\
SpoutLibary and SpoutDirectX have not been updated and could cause errors.



