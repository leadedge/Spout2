Spout2 demonstration program.

This is a version of the Spout demo program revised for compilation using the Spout SDK
based on  a tutorial project by NEHE :

http://nehe.gamedev.net/tutorial/texture_mapping/12038/

Licence : http://www.gamedev.net/page/resources/_/gdnethelp/gamedevnet-open-license-r2956

The DX9 compatible sender uses a DirectX 11 texture format (DXGI_FORMAT_B8G8R8A8_UNORM)
compatible with existing Spout recevers. The DX11 sender uses an RGBA format
(DXGI_FORMAT_R8G8B8A8_UNORM) which can be received by DirectX 11 receivers but not DX9.

A DirectX 11 receiver will receive from DirectX11 or DirectX 9 senders.

