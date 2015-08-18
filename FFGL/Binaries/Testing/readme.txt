The Spout freeframe dlls programs allow either DirectX9 or DirectX 11 texture sharing.

The latest will start in either DirectX 9 or DirectX 11 mode depending on the selection during installation or with the setup program "SpoutDirectX".

Older versions are included here and are useful for testing compatibility of an application.

The DX9 receiver will only receive from a DX9 sender unless the DX11 sender uses a compatible texture format (DXGI_FORMAT_B8G8R8A8_UNORM). 

The DX11 sender uses an RGBA format (DXGI_FORMAT_R8G8B8A8_UNORM) which can be received by DirectX 11 receivers but not DX9.

A DirectX 11 receiver will receive from DirectX11 or DirectX 9 senders.

Memoryshare can be useful for testing with systems which do not support the GL/DX OpenGL extensions.

