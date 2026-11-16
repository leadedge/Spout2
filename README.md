## OpenGL external memory interop

#### Updated 16th Nov 2006 based on the beta branch release 2.007.018.

The Spout library has been developed using the [WGL_NV_DX interop](https://registry.khronos.org/OpenGL/extensions/NV/WGL_NV_DX_interop2.txt) extensions, developed and maintained by Nvidia, for transfer of textures between DirectX and OpenGL.

More recently, [GL_EXT_memory_object](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_external_objects.txt) extensions with multi-vendor support and maintained by the Khronos Group have been released and can also be used as an interop method.

These extensions are based on the concept of memory objects introduced by the Vulkan API and provide potential for future development.

This branch includes source code with the OpenGL memory object extensions used as alternative to the NVidia interop. Changes are limited to SpoutGL.cpp, Spout.cpp, SpoutSenderNames.cpp and SpoutGLextensions. Other source files remain the same.

OpenGL memory is used if the NVidia interop fails or the GL memory option is selected using "SpoutSettings". There is no change in functionality and older applications are not affected. Textures are shared between applications using either interop method.

However, the extent of support GL_EXT_memory extensions is possibly low compared to NVidia interop which has been established since 2010, and practical advantage could be minimal. 

### Assessment

The demonstration programs provided with this branch allow assessment of compatibility and performance. Find them in the BUILD folder

### Building with Cmake
Select only the option for SPOUT_BUILD_CMT as required.\
Only SpoutGL source files have been modified, based on release 2.007.018\
Do not select SPOUT_BUILD_LIBRARY, SPOUT_BUILD_SPOUTDX or SPOUT_BUILD_SPOUTDX_EXAMPLES\
SpoutLibary and SpoutDirectX have not been updated and could cause errors.



