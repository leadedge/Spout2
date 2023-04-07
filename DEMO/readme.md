### SPOUT DEMO PROGRAMS

Programs to demonstrate sending and receiving.

These programs can be used to find whether your system supports texture sharing and to help trace any problems that may arise with other programs.

The on-screen display shows graphics adapter details and texture share compatibility.

#### Graphics compatibility

Texture sharing with OpenGL progams requires compatibility for OpenGL/DirectX11 texture linking using [NVIDIA GL/DX interop extensions](https://www.khronos.org/registry/OpenGL/extensions/NV/WGL_NV_DX_interop2.txt). This is tested when a sender or receiver is first initialized. If hardware is not compatible, sending and receiving functions switch to backup methods using CPU system memory for sharing textures.

CPU sharing uses DirectX textures by way of system memory, which means that that an application with graphics that is not compatible with OpenGL/DirectX linking can still communicate with another on the same machine using compatible graphics hardware. This situation can occur with dual graphics laptop systems (NVIDIA “Optimus” or AMD “Dynamic Switchable Graphics”). This should be investigated using the respective program for graphics settings.

#### Receiver full screen

The demo receiver is useful as a full-screen display. The program can be run on different monitors and will show full screen on that monitor.

