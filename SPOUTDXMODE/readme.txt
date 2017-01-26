SPOUTDXMODE

SpoutDXmode is a utility program to change the operation mode for Spout applications.


DirectX 9

If DirectX9c is not installed you will see buttons to download the web installer or to download the installer file. DirectX 9c is required for DirectX 9 functions.


Options

o DirectX 9 - switch between DirectX 9 and DirectX 11 functions
o Buffer    - use OpenGL pixel buffering to copy data GPU < > CPU.

Pixel buffering is used for CPU texture access and memoryshare or for applications that transfer to textures from CPU memory. Buffering is faster but might result in latency. Test for your application.


Share mode

o Texture - OpenGL/DirectX interop for sharing textures
o CPU     - CPU data transfer from DirectX shared textures
o Memory  - CPU memory for data sharing instead of DirectX

Texture mode requires a graphics adapter compatible with the NVIDIA OpenGL/DirectX interop.CPU mode is slower but does not require NVIDIA comatibility. Memory mode does not use shared textures but rather shared memory instead.

CPU sharing mode is a backup where hardware does not support texture sharing. It is not recommended if texture sharing is available. Memory mode may be faster than CPU mode but requires applications built with 2.005 or later.

All Spout applications built with the Spout 2.006 SDK will switch to the selected mode. Memory mode or CPU mode will not have any effect for Spout 2.004 applications. CPU mode will not have any effect for Spout 2.005 applications.


NVIDIA global processor

o for laptop computers with dual graphics and NVIDIA Optimus power saving technology.

The mode of operation can be changed to enable the high performance NVIDIA graphics globally for all applications. This over-rides the NVIDIA control panel settings and can be useful where there is difficulty in adjusting these settings.

This will not have any effect if the computer does not have Optimus graphics or is not NVIDIA.





