“SpoutReceiver2.dll” and “SpoutSender2.dll” are 32bit FreeframeGL plugins and must be copied into the plugin folder of the host application.

For “Resolume Avenue” or “Resolume Arena”, copy them to the plugin “vfx” folder. 

Typically this might be (for a 32 bit Windows system) :

    C:\Program Files\Resolume Arena 4.1.8\plugins\vfx\
        or for a 64bit system it might be :
    C:\Program Files (x86)\Resolume Arena 4.1.8\plugins\vfx


The plugins are compiled to use DirectX 11 by default.
Versions for DirectX 9 or memoryshare mode may be necessary for
systems not compatible with DirectX 11 or the GL/DX interop OpenGL extensions.

