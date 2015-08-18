//
//								Spout2.dll
//
//		C compatible dll
//		Exports suitable for compilers other than Visual Studio
//
//		This is the main DLL file (no Main)
//
//		07.11.14 - started dll
//		08.01.15 - tested with OpenFrameworks and CodeBlocks with MingW compiler
//		15.08.15 - added Init and Close OpenGL for apps that do not have an OpenGL context
//
/*
		Copyright (c) 2014-2015. Lynn Jarvis. All rights reserved.

		Redistribution and use in source and binary forms, with or without modification, 
		are permitted provided that the following conditions are met:

		1. Redistributions of source code must retain the above copyright notice, 
		   this list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright notice, 
		   this list of conditions and the following disclaimer in the documentation 
		   and/or other materials provided with the distribution.

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	AND ANY 
		EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
		OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE	ARE DISCLAIMED. 
		IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
		INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
		PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
		INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
		LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "SpoutDll.h"

#include "..\..\SpoutSDK\Spout.h"

namespace Spout2 {

	Spout spout;


	//
	// Sender
	//

	extern "C" _declspec(dllexport)
	bool CreateSender(char *name, unsigned int width, unsigned int height, DWORD dwFormat) {
		return spout.CreateSender(name, width, height, dwFormat);
	}

	extern "C" _declspec(dllexport)
	bool UpdateSender(char* Sendername, unsigned int width, unsigned int height) {
		return spout.UpdateSender(Sendername, width, height);
	}

	extern "C" _declspec(dllexport)
	void ReleaseSender(DWORD dwMsec) {
		return spout.ReleaseSender(dwMsec);
	}

	extern "C" _declspec(dllexport)
	bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO) {
		return spout.SendTexture(TextureID, TextureTarget, width, height, bInvert, HostFBO);
	}

	extern "C" _declspec(dllexport)
	bool SendImage(unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bAlignment, bool bInvert) {
		return spout.SendImage(pixels, width, height, glFormat, bAlignment, bInvert);
	}

	extern "C" _declspec(dllexport)
	bool DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x, float max_y, float aspect, bool bInvert, GLuint HostFBO ) {
		return spout.DrawToSharedTexture(TextureID, TextureTarget, width, height, max_x, max_y, aspect, bInvert, HostFBO);
	}


	// 
	// Receiver
	//

	extern "C" _declspec(dllexport)
	bool CreateReceiver(char* name, unsigned int &width, unsigned int &height, bool bUseActive) {
		return spout.CreateReceiver(name, width, height, bUseActive);
	}

	extern "C" _declspec(dllexport)
	void ReleaseReceiver() { 
		return spout.ReleaseReceiver();
	}

	extern "C" _declspec(dllexport)
	bool ReceiveTexture(char* Sendername, unsigned int &width, unsigned int &height, GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFBO) {
		return spout.ReceiveTexture(Sendername, width, height, TextureID, TextureTarget, bInvert, HostFBO);
	}

	extern "C" _declspec(dllexport)
	bool ReceiveImage(char* Sendername, unsigned int &width, unsigned int &height, unsigned char* pixels, GLenum glFormat, GLuint HostFBO) {
		return spout.ReceiveImage(Sendername, width, height, pixels, glFormat, HostFBO);
	}
	
	extern "C" _declspec(dllexport)
	bool GetImageSize (char* sendername, unsigned int &width, unsigned int &height, bool &bMemoryMode) {	
		return spout.GetImageSize (sendername, width, height, bMemoryMode);
	}

	extern "C" _declspec(dllexport)
	bool BindSharedTexture() {
		return spout.BindSharedTexture();
	}

	extern "C" _declspec(dllexport)
	bool UnBindSharedTexture() {
		return spout.UnBindSharedTexture();
	}
	
	extern "C" _declspec(dllexport)
	bool DrawSharedTexture(float max_x, float max_y, float aspect) {
		return spout.DrawSharedTexture(max_x, max_y, aspect);
	}

	extern "C" _declspec(dllexport)
	int  GetSenderCount() {
		return spout.GetSenderCount();
	}

	extern "C" _declspec(dllexport)
	bool GetSenderName(int index, char* sendername, int MaxSize) {
		return spout.GetSenderName(index, sendername, MaxSize);
	}

	extern "C" _declspec(dllexport)
	bool GetSenderInfo(char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat) {
		return spout.GetSenderInfo(sendername, width, height, dxShareHandle, dwFormat);
	}
	
	extern "C" _declspec(dllexport)
	bool GetActiveSender(char* Sendername) {
		return spout.GetActiveSender(Sendername);
	}
	
	extern "C" _declspec(dllexport)
	bool SetActiveSender(char* Sendername) {
		return spout.SetActiveSender(Sendername);
	}
	
	//
	// Utilities
	//

	extern "C" _declspec(dllexport)
	bool SetDX9(bool bDX9) { // set to use DirectX 9 (default is DirectX 11)
		return spout.SetDX9(bDX9);
	}
	
	extern "C" _declspec(dllexport)
	bool GetDX9() {
		return spout.GetDX9();
	}

	extern "C" _declspec(dllexport)
	bool GetMemoryShareMode() {
		return spout.GetMemoryShareMode();
	}
	
	extern "C" _declspec(dllexport)
	bool SetMemoryShareMode(bool bMemory) {
		return spout.SetMemoryShareMode(bMemory);
	}
	
	extern "C" _declspec(dllexport)
	int  GetVerticalSync() {
		return spout.GetVerticalSync();
	}
	
	extern "C" _declspec(dllexport)
	bool SetVerticalSync(bool bSync) {
		return spout.SetVerticalSync(bSync);
	}
	
	extern "C" _declspec(dllexport)
	bool SelectSenderPanel(const char* message) {
		return spout.SelectSenderPanel(message);
	}

	// Convert a wide string to a character array - used by PureBasic
	extern "C" _declspec(dllexport)
	int _wcstombs(void *sendername, const wchar_t *wname, int maxcount)
	{
		return (int)wcstombs((char *)sendername, wname, maxcount);
	}

	extern "C" _declspec(dllexport)
	bool InitOpenGL()
	{
		HDC hdc = NULL;
		HWND hwnd = NULL;
		HWND hwndButton = NULL;
		HGLRC hRc = NULL;

		HGLRC glContext = wglGetCurrentContext();

		if(glContext == NULL) {

			// We only need an OpenGL context with no render window because we don't draw to it
			// so create an invisible dummy button window. This is then independent from the host
			// program window (GetForegroundWindow). If SetPixelFormat has been called on the
			// host window it cannot be called again. This caused a problem in Mapio.
			// https://msdn.microsoft.com/en-us/library/windows/desktop/dd369049%28v=vs.85%29.aspx
			//
			if(!hwndButton || !IsWindow(hwndButton)) {
				hwndButton = CreateWindowA("BUTTON",
					            "SpoutOpenGL",
								WS_OVERLAPPEDWINDOW,
								0, 0, 32, 32,
								NULL, NULL, NULL, NULL);
			}

			if(!hwndButton) { 
				// printf("InitOpenGL error 1\n");
				MessageBoxA(NULL, "Error 1\n", "InitOpenGL", MB_OK);
				return false; 
			}

			hdc = GetDC(hwndButton);
			if(!hdc) { 
				// printf("InitOpenGL error 2\n"); 
				MessageBoxA(NULL, "Error 2\n", "InitOpenGL", MB_OK); 
				return false; 
			}
			
			PIXELFORMATDESCRIPTOR pfd;
			ZeroMemory( &pfd, sizeof( pfd ) );
			pfd.nSize = sizeof( pfd );
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;
			pfd.cDepthBits = 16;
			pfd.iLayerType = PFD_MAIN_PLANE;
			int iFormat = ChoosePixelFormat(hdc, &pfd);
			if(!iFormat) { 
				// printf("InitOpenGL error 3\n"); 
				MessageBoxA(NULL, "Error 3\n", "InitOpenGL", MB_OK);
				return false; 
			}

			if(!SetPixelFormat(hdc, iFormat, &pfd)) {
				DWORD dwError = GetLastError();
				// printf("InitOpenGL error 4 (Error %d (%x))\n", dwError, dwError); 
				// 2000 (0x7D0) The pixel format is invalid.
				// Caused by repeated call of  the SetPixelFormat function
				char temp[128];
				sprintf_s(temp, "InitOpenGL Error 4\nSetPixelFormat\nError %d (%x)", dwError, dwError);
				MessageBoxA(NULL, temp, "InitOpenGL", MB_OK); 
				return false; 
			}

			hRc = wglCreateContext(hdc);
			if(!hRc) { 
				// printf("InitOpenGL error 5\n"); 
				MessageBoxA(NULL, "Error 5\n", "InitOpenGL", MB_OK); 
				return false; 
			}

			wglMakeCurrent(hdc, hRc);
			if(wglGetCurrentContext() == NULL) {
				// printf("InitOpenGL error 6\n");
				MessageBoxA(NULL, "Error 6\n", "InitOpenGL", MB_OK);
				return false; 
			}
		}
		return true;
	}

	extern "C" _declspec(dllexport)
	bool CloseOpenGL()
	{		
		
		HGLRC ctx = wglGetCurrentContext();
		if(ctx != NULL) {
			wglDeleteContext(ctx);
			return true;
		}

		return false;
	}


} // end namespace Spout2
