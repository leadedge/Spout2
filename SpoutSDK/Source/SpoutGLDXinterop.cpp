/**

	spoutGLDXinterop.cpp

	See also - spoutDirectX, spoutSenderNames

	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		Copyright (c) 2014, Lynn Jarvis. All rights reserved.

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
		========================

		15-07-14	- ReadTexturePixels - allowed for variable OpenGL format instead of RGB only.
					- Needs testing. 
					- TODO - variable gl format for WriteTexturePixels
		21.07.14	- removed local fbo and replaced with temporary fbo within
					  texture functions due to problems with Max / Jitter
		22-07-14	- added option for DX9 or DX11
		23-07-14	- cleanup of DX9 / DX11 functions
		29-07-14	- pass format 0 for DX9 sender
		31-07-14	- Corrected DrawTexture aspect argument
		13-08-14	- OpenGL texture retained on cleanup
		14-08-14	- Corrected texture delete without context
		16-08-14	- created DrawToSharedTexture
		18-08-14	- debugging with WriteTexture method
		 -- names class revision additions --
		19-08-14	- activated event locks
		01.09.14	- removed temp fbo for texture transfers and returned to use of a common fbo
					- delete texture and fbo on cleanup
					- set texture and fbo to zero on cleanup, otherwise errors in Jitter
					- changed to vertex array draw for DrawToSharedTexture
					- Removed PAINT message from OpenDirectX9 due to crash of sender in Magic
		03.09.14	- Replaced with UpdateWindow and limited to Resolume only.
					- Cleanup
		15.09.14	- corrected access lock for DrawToSharedTexture and ReadTexturePixels
		21.09.14	- mutex texture access locks
		23.09.14	- moved general mutex texture access lock to the SpoutDirectX class
		23.09.14	- test for DirectX 11 support in UseDX9, IsDX9 and OpenDirectX
		24.09.14	- save and restore fbo for read/write/drawto texture
		28.09.14	- Added GL format argument for WriteTexturePixels
					- Added bAlignment  (4 byte alignment) flag for WriteTexturePixels
					- Changed GLformat argument from int to GLenum in ReadTexturePixels
					- Changed default GLformat from GL_RGB to GL_RGBA in ReadTexturePixels
					- Added Host FBO argument for ReadTexture, DrawToSharedTexture, WriteTexture
		12.10.14	- cleaned up CreateInterop for sender updates
		15.10.14	- added safety release of texture in CreateDX9interop in case of previous application crash
		17.10.14	- Directx 11 release context before device
		21.10.14	- removed keyed mutex lock due to reported driver problems
		21.10.14	- Allow for compatible texture formats
					  DirectX 11 format 87, DirectX9 D3DFMT_X8R8G8B8, and the default D3DFMT_A8R8G8B8
		21.10.14	- Allow DirectX texture formats to be registered in CreateInterop
		24.10.14	- Fall back to DirectX 9 if DirectX11 init fails
		24.11.14	- removed context print statement in CleanupInterop
		23.12.14	- added host fbo arg to ReadTexturePixels
					  Changed readback method to glReadPixels
		04.02.15	- Changed header default to DirectX 9 instead of DirectX 11
		09.02.15	- added invert flag to DrawSharedTexture (default true with no args)
		12.02.15	- Changed OpenDirectX to check for Intel graphics and open DirectX 9 if present
		13.02.15	- OpenDirectX9 included SendMessageTimeout before attempting to get the fg window text
		14.02.15	- Used PathStripPath function requiring shlwapi.h - (see SpoutSDK.h)
		--
		21.05.15	- OpenDirectX - adapter auto detection removed and replaced by an installer with option for DX11
					- Programmer must SetDX9(true) for compilation using DirectX 11 functions

*/

#include "spoutGLDXinterop.h"

spoutGLDXinterop::spoutGLDXinterop() {

	/*
	// Debug console window so printf works
	FILE* pCout;
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	// printf("spoutGLDXinterop\n");
	*/

	m_hWnd				= NULL;
	m_glTexture			= 0;
	m_fbo               = 0;
	m_hInteropObject	= NULL;
	m_hSharedMemory		= NULL;
	m_hInteropDevice	= NULL;
	
	m_hAccessMutex      = NULL;

	// DX9
	bUseDX9				= false; // Use DX11 (default false) or DX9 (true)
	m_pD3D				= NULL;
	m_pDevice			= NULL;
	m_dxTexture			= NULL;
	DX9format           = D3DFMT_A8R8G8B8; // default format for DX9 (21)
	
	// DX11
	g_pd3dDevice		= NULL;
	g_pSharedTexture	= NULL;
	g_pImmediateContext = NULL;
	g_driverType		= D3D_DRIVER_TYPE_NULL;
	g_featureLevel		= D3D_FEATURE_LEVEL_11_0;
	DX11format			= DXGI_FORMAT_B8G8R8A8_UNORM; // Default compatible with DX9

	m_bInitialized		= false;
	bExtensionsLoaded	= false;
	bFBOavailable		= false;
	bBLITavailable		= false;
	bPBOavailable		= false;
	bSWAPavailable		= false;

}

spoutGLDXinterop::~spoutGLDXinterop() {
	
	m_bInitialized = false;
	// Because cleanup is not here it has to be specifically called
	// This is because it can crash on exit - see cleanup for details
}


// For external access so that the local global variables are used
// Look for Intel graphics and open DirectX 9 if present
// 21.04.15 - Auto detection removed and replaced by an installer with option for DX11
bool spoutGLDXinterop::OpenDirectX(HWND hWnd, bool bDX9)
{
	// char renderadapter[256];
	// char renderdescription[256];
	// char renderversion[256];
	// char displaydescription[256];
	// char displayversion[256];

	// If user set DX9 then just use it.
	// Also check for Operating system DirectX 11 availability.
	// Note that if the program is run in "compatibility mode" DX11available will
	// detect the operating system selected for compatibility. So it will fail for 
	// Windows XP for example and this check will switch to DirectX 9
	if(bDX9 || !spoutdx.DX11available()) {
		bUseDX9 = true;
		return (OpenDirectX9(hWnd));
	}

	// Open DX11	
	if(OpenDirectX11()) {
		bUseDX9 = false; // Set to indicate intialized as DX11
		// Return here if OK - Intel auto detection removed 21.04.15
		return true;
	}

	return false;

	
	/*
		// 21.04.15 - removed auto-detection due to user problems
		// Get adapter info using both DirectX and Windows functions
		// If GetAdapterInfo sets bUseDX9 to true, change back to DX9
		GetAdapterInfo(renderadapter, renderdescription, renderversion, displaydescription, displayversion, 256, bUseDX9);
		// printf("GetAdapterInfo bDX9mode = %d\n", bUseDX9);
		// printf("renderadapter [%s]\n", renderadapter);
		// printf("renderdescription [%s]\n", renderdescription);
		// printf("renderversion [%s]\n", renderversion);
		// printf("displaydescription [%s]\n", displaydescription);
		// printf("displayversion [%s]\n", displayversion);
		if(!bUseDX9) {
			// printf("    [%s] Using DX11\n", renderadapter);
			// sprintf_s(tmp, 256, "[%s] Using DX11\n", renderadapter);
			// MessageBoxA(NULL, tmp, "Spout", MB_OK);
			return true; // Keep using DirectX 11
		}
		// printf("    GetAdapterInfo [%s] changed to DX9\n", renderadapter);
		// drop through
	}
	// printf("    [%s] Using DX9\n", renderadapter);

	// If DX11 init fails, or if bUseDX9 has been set to true
	// close DX11 and initialize DX9 instead
	spoutdx.CloseDX11(); // Release immediate context before releasing the DX11 device
	if(g_pd3dDevice != NULL) g_pd3dDevice->Release();
	g_pSharedTexture = NULL; // Important because mutex locks check for NULL
	g_pd3dDevice = NULL;
	return (OpenDirectX9(hWnd));
	*/


}


bool spoutGLDXinterop::OpenDeviceKey(const char* key, int maxsize, char *description, char *version)
{
	// Extract the subkey from the DeviceKey string
	HKEY hRegKey;
	DWORD dwSize, dwKey;  
	char output[256];
	strcpy_s(output, 256, key);
	string SubKey = strstr(output, "System");

	// Convert all slash to double slash using a C++ string function
	// to get subkey string required to extract registry information
	for (unsigned int i=0; i<SubKey.length(); i++) {
		if (SubKey[i] == '\\') {
			SubKey.insert(i, 1, '\\');
			++i; // Skip inserted char
		}
	}

	// Open the key to find the adapter details
	if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, SubKey.c_str(), NULL, KEY_READ, &hRegKey) == 0) { 
		dwSize = MAX_PATH;
		// Adapter name
		if(RegQueryValueExA(hRegKey, "DriverDesc", NULL, &dwKey, (BYTE*)output, &dwSize) == 0) {
			// printf("DriverDesc = %s\n",output);
			strcpy_s(description, maxsize, output);
			// strcat_s(result, maxsize, "\r\n");
		}
		if(RegQueryValueExA(hRegKey, "DriverVersion", NULL, &dwKey, (BYTE*)output, &dwSize) == 0) {
			// printf("Registry driverVersion = %s\n", output);
			// Find the last 6 characters of the version string then
			// convert to a float and multiply to get decimal in the right place
			sprintf_s(output, 256, "%5.2f", atof(output + strlen(output)-6)*100.0);
			// printf("Version = %s\n", output);
			strcpy_s(version, maxsize, output);
		} // endif DriverVersion
		RegCloseKey(hRegKey);
	} // endif RegOpenKey

	return true;
}

void spoutGLDXinterop::trim(char * s) {
    char * p = s;
    int l = (int)strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;

    memmove(s, p, l + 1);
}


// this function initializes and prepares Direct3D
bool spoutGLDXinterop::OpenDirectX9(HWND hWnd)
{
	HWND fgWnd = NULL;
	char fgwndName[MAX_PATH];

	// Already initialized ?
	if(m_pD3D != NULL) {
		// printf("    already initialized\n");
		return true;
	}

	// Create a IDirect3D9Ex object if not already created
	if(!m_pD3D) {
		m_pD3D = spoutdx.CreateDX9object(); 
	}

	if(m_pD3D == NULL) {
		// printf("    Could not create DX9 object\n");
		return false;
	}

	// Create a DX9 device
	if(!m_pDevice) {
		m_pDevice = spoutdx.CreateDX9device(m_pD3D, hWnd); 
	}

	if(m_pDevice == NULL) {
		// printf("    Could not create DX9 device\n");
		return false;
	}

	// Problem for FFGL plugins - might be a problem for other FFGL hosts or applications.
	// DirectX 9 device initialization creates black areas and the host window has to be redrawn.
	// But this causes a crash for a sender in Magic when the render window size is changed.
	// Not a problem for DirectX 11.
	// Not needed in Isadora.
	// Needed for Resolume.
	// For now, limit this to Resolume only.

	fgWnd = GetForegroundWindow();
	if(fgWnd) {
		// SMTO_ABORTIFHUNG : The function returns without waiting for the time-out
		// period to elapse if the receiving thread appears to not respond or "hangs."
		if(SendMessageTimeoutA(fgWnd, WM_GETTEXT, MAX_PATH, (LPARAM)fgwndName, SMTO_ABORTIFHUNG, 128, NULL) != 0) {
			// Returns the full path - get just the window name
			PathStripPathA(fgwndName);
			// console debug
			// printf("    Window text : %s\n", fgwndName);
			if(fgwndName[0]) {
				if(strstr(fgwndName, "Resolume") != NULL // Is resolume in the window title ?
				&& strstr(fgwndName, "magic") == NULL) { // Make sure it is not a user named magic project.
					// DirectX device initialization needs the window to be redrawn (creates black areas)
					// 03.05.15 - user observation that UpDateWindow does not work and Resolume GUI is still corrupted
					// UpdateWindow(fgWnd);
					// printf("    WM_PAINT : %x\n", fgWnd);
					SendMessage(fgWnd, WM_PAINT, NULL, NULL );
				}
			}
		}
		else
			printf("    SendMessageTimeout fail.\n");
	}

	return true;
}

// this function initializes and prepares Direct3D
bool spoutGLDXinterop::OpenDirectX11()
{
	// Quit if already initialized
	if(g_pd3dDevice != NULL)	return true;

	// Create a DirectX 11 device
	if(!g_pd3dDevice) g_pd3dDevice = spoutdx.CreateDX11device();
	if(g_pd3dDevice == NULL) {
		// printf("    Could not create DX11 device\n");
		return false;
	}

	return true;
}

// Must be called after DirectX initialization
//
// https://code.google.com/p/chromium/issues/detail?id=106438
//
// NOTES : On a “normal” system EnumDisplayDevices and IDXGIAdapter::GetDesc always concur
// i.e. the device that owns the head will be the device that performs the rendering. 
// On an Optimus system IDXGIAdapter::GetDesc will return whichever device has been selected for rendering.
// So on an Optimus system it is possible that IDXGIAdapter::GetDesc will return the dGPU whereas 
// EnumDisplayDevices will return the iGPU.
//
// This function compares the adapter descriptions of the two
// The string "Intel" reveals that it is an Intel device but 
// the Vendor ID could also be used
//
//	0x10DE	NVIDIA
//	0x163C	intel
//	0x8086  Intel
//	0x8087  Intel
//
bool spoutGLDXinterop::GetAdapterInfo(char *renderadapter, 
									  char *renderdescription, char *renderversion,
									  char *displaydescription, char *displayversion,
									  int maxsize, bool &bDX9)
{
	// printf("spoutGLDXinterop::GetAdapterInfo\n");
	renderadapter[0] = 0; // DirectX adapter
	renderdescription[0] = 0;
	renderversion[0] = 0;
	displaydescription[0] = 0;
	displayversion[0] = 0;

	if(bDX9) {

		// printf("DX9 mode\n");

		if(m_pDevice == NULL) return false;

		D3DADAPTER_IDENTIFIER9 adapterinfo;
		// char            Driver[MAX_DEVICE_IDENTIFIER_STRING];
		// char            Description[MAX_DEVICE_IDENTIFIER_STRING];
		// char            DeviceName[32];         // Device name for GDI (ex. \\.\DISPLAY1)
		// LARGE_INTEGER   DriverVersion;          // Defined for 32 bit components
		// DWORD           VendorId;
		// DWORD           DeviceId;
		// DWORD           SubSysId;
		// DWORD           Revision;
		// GUID            DeviceIdentifier;
		// DWORD           WHQLLevel;
		m_pD3D->GetAdapterIdentifier (D3DADAPTER_DEFAULT, 0, &adapterinfo);
		// printf("Driver = [%s]\n", adapterinfo.Driver);
		// printf("    Description = [%s]\n", adapterinfo.Description);
		// printf("DeviceName = [%s]\n", adapterinfo.DeviceName);
		// printf("DriverVersion = [%d] [%x]\n", adapterinfo.DriverVersion, adapterinfo.DriverVersion);
		// printf("VendorId = [%d] [%x]\n", adapterinfo.VendorId, adapterinfo.VendorId);
		// printf("DeviceId = [%d] [%x]\n", adapterinfo.DeviceId, adapterinfo.DeviceId);
		// printf("SubSysId = [%d] [%x]\n", adapterinfo.SubSysId, adapterinfo.SubSysId);
		// printf("Revision = [%d] [%x]\n", adapterinfo.Revision, adapterinfo.Revision);
		strcpy_s(renderadapter, maxsize, adapterinfo.Description);

	}
	else {

		// printf("DX11 mode\n");

		if(g_pd3dDevice == NULL) return false;

		IDXGIDevice * pDXGIDevice;
		g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);
		IDXGIAdapter * pDXGIAdapter;
		pDXGIDevice->GetAdapter(&pDXGIAdapter);
		DXGI_ADAPTER_DESC adapterinfo;
		pDXGIAdapter->GetDesc(&adapterinfo);
		// WCHAR Description[ 128 ];
		// UINT VendorId;
		// UINT DeviceId;
		// UINT SubSysId;
		// UINT Revision;
		// SIZE_T DedicatedVideoMemory;
		// SIZE_T DedicatedSystemMemory;
		// SIZE_T SharedSystemMemory;
		// LUID AdapterLuid;

		char output[256];
		size_t charsConverted = 0;
		wcstombs_s(&charsConverted, output, 129, adapterinfo.Description, 128);
		// printf("    Description = [%s]\n", output);
		// printf("    VendorId = [%d] [%x]\n", adapterinfo.VendorId, adapterinfo.VendorId);
		// printf("SubSysId = [%d] [%x]\n", adapterinfo.SubSysId, adapterinfo.SubSysId);
		// printf("DeviceId = [%d] [%x]\n", adapterinfo.DeviceId, adapterinfo.DeviceId);
		// printf("Revision = [%d] [%x]\n", adapterinfo.Revision, adapterinfo.Revision);
		strcpy_s(renderadapter, maxsize, output);
	}

	// LJ DEBUG - default render adapter is the DirectX one ???
	if(renderadapter) {
		strcpy_s(renderdescription, maxsize, renderadapter);
		// printf("DirectX render adapter [%s]\n", renderadapter);
	}

	// Use Windows functions to look for Intel graphics to see  if it is
	// the same render adapter that was detected with DirectX
	char driverdescription[256];
	char driverversion[256];
	char regkey[256];
	size_t charsConverted = 0;
	
	// Additional info
	DISPLAY_DEVICE DisplayDevice;
	DisplayDevice.cb = sizeof(DISPLAY_DEVICE);

	// 31.10.14 detect the adapter attached to the desktop
	// To query all display devices in the current session, 
	// call this function in a loop, starting with iDevNum set to 0, 
	// and incrementing iDevNum until the function fails. 
	// To select all display devices in the desktop, use only the display devices
	// that have the DISPLAY_DEVICE_ATTACHED_TO_DESKTOP flag in the DISPLAY_DEVICE structure.

	int nDevices = 0;
	for(int i=0; i<10; i++) { // should be much less than 10 adapters
		if(EnumDisplayDevices(NULL, i, &DisplayDevice, 0)) {
			// This will list all the devices
			nDevices++;
			// Get the registry key
			// wcstombs_s( &charsConverted, regkey, 129, DisplayDevice.DeviceKey, 128);
			wcstombs_s( &charsConverted, regkey, 129, (const wchar_t *)DisplayDevice.DeviceKey, 128);
			// printf("DeviceKey = %s\n", regkey); 
			// This is the registry key with all the information about the adapter
			OpenDeviceKey(regkey, 256, driverdescription, driverversion);
			// Is it a render adapter ?
			if(renderadapter && strcmp(driverdescription, renderadapter) == 0) {
				// printf("Windows render adapter matches : [%s] Vers [%s]\n", driverdescription, driverversion);
				strcpy_s(renderdescription, maxsize, driverdescription);
				strcpy_s(renderversion, maxsize, driverversion);
			}
			// Is it a display adapter
			if(DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
				// printf("Display adapter : [%s] Vers: %s ", driverdescription, driverversion);
				strcpy_s(displaydescription, 256, driverdescription);
				strcpy_s(displayversion, 256, driverversion);
				// printf("(Attached to desktop)\n");
			} // endif attached to desktop

		} // endif EnumDisplayDevices
	} // end search loop

	// The render adapter
	if(renderdescription) trim(renderdescription);

	// Is it Intel graphics
	if(renderdescription && strlen(renderdescription) > 0 && !strstr(renderdescription, "Intel")) {
		// printf("Not Intel (%s) can use DX11\n", renderdescription);
		// Don't change the existing bDX9 flag so that
		// it returns to initialize either DirectX or DirectX 11 as the user requires (default is DX11)
		// LJ DEBUG - comment out to simulate Intel detection
		return true;
	}

	// if(!renderdescription || strlen(renderdescription) == 0) {
		// nvd3d9wrap.dll is loaded into all processes when Optimus is enabled.
		HMODULE nvd3d9wrap = GetModuleHandleA("nvd3d9wrap.dll");
		if(nvd3d9wrap != NULL) {
			if(renderdescription)
				sprintf_s(renderdescription, maxsize, "Optimus graphics [%s]", renderdescription);
			else 
				sprintf_s(renderdescription, maxsize, "Optimus graphics integrated graphics");
		}
	// }

	bDX9 = true; // will return to re-initialize DirectX 9

	return true;
}


//
bool spoutGLDXinterop::CreateInterop(HWND hWnd, char* sendername, unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive)
{
	bool bRet = true;
	DWORD format;

	// Needs an openGL context to work
	if(!wglGetCurrentContext()) {
		// MessageBoxA(NULL, "CreateInterop - no GL context", "Warning", MB_OK);
		return false;
	}

	// printf("CreateInterop - %dx%d - format (%d) \n", width, height, dwFormat);

	// Texture format tests
	// Compatible formats
	// DXGI_FORMAT_R8G8B8A8_UNORM; // default DX11 format - not compatible with DX9 (28)
	// DXGI_FORMAT_B8G8R8A8_UNORM; // compatible DX11 format - works with DX9 (87)
	// DXGI_FORMAT_B8G8R8X8_UNORM; // compatible DX11 format - works with DX9 (88)
	// Allow for compatible DirectX 11 senders (format 87)
	// And compatible DirectX9 senders D3DFMT_X8R8G8B8 - 22
	// and the default D3DFMT_A8R8G8B8 - 21
	if(bUseDX9) {
		// DirectX 9
		if(dwFormat > 0) {
			if(dwFormat == 87) {
				// printf("CreateInterop - DX9 mode - compatible DX11 user format (%d) \n", dwFormat);
				format = (DWORD)D3DFMT_A8R8G8B8; // (21)
			}
			else if(dwFormat == D3DFMT_X8R8G8B8 || dwFormat == D3DFMT_A8R8G8B8) {
				// printf("CreateInterop - DX9 mode - compatible DX9 user format (%d) \n", dwFormat); 
				format = (DWORD)dwFormat; // (22)
			}
			else {
				// printf("CreateInterop - DX9 mode - incompatible user format (%d) \n", dwFormat);
				return false;
			}
		}
		else { // format is passed as zero so we assume a DX9 sender D3DFMT_A8R8G8B8
			format = (DWORD)DX9format;
			// printf("CreateInterop - DX9 mode - DX9format (%d) \n", format);
		}
	}
	else {
		// DirectX 11
		// Is this a DX11 or a DX9 sender texture?
		// A directX 11 receiver accepts DX9 formats
		if(!bReceive && dwFormat > 0) {
			// printf("CreateInterop - DX11 sender - user format %d \n", dwFormat);
			format = (DXGI_FORMAT)dwFormat;
		}
		else {
			// printf("CreateInterop - default DX11 format BGRA - format passed = %d \n", dwFormat);
			format = (DWORD)DX11format; // DXGI_FORMAT_B8G8R8A8_UNORM (87) default compatible with DX9
		}
	}

	// Quit now if the receiver can't access the shared memory info of the sender
	// Otherwise m_dxShareHandle is set by getSharedTextureInfo and is the
	// shared texture handle of the Sender texture
	if (bReceive && !getSharedTextureInfo(sendername)) {
		printf("CreateInterop error 1\n");
		return false;
	}

	// printf("Sender texture format = %d \n", m_TextureInfo.format);

	// Check the sender format for a DX9 receiver
	// It can only be from a DX9 sender (format 0, 22, 21)
	// or from a compatible DX11 sender (format 87)
	if(bReceive && bUseDX9) {
		if(!(m_TextureInfo.format == 0 
			|| m_TextureInfo.format == 22
			|| m_TextureInfo.format == 21
			|| m_TextureInfo.format == 87)) {
			printf("Incompatible sender texture format %d \n", m_TextureInfo.format);
			return false;
		}
	}

	// Make sure DirectX has been initialized
	// Creates a global pointer to the DirectX device (DX11 g_pd3dDevice or DX9 m_pDevice)
	if(!OpenDirectX(hWnd, bUseDX9)) {
		printf("CreateInterop error 2\n");
		return false;
	}

	// Allow for sender updates
	// When a sender size changes, the new texture has to be re-registered
	if(m_hInteropDevice != NULL &&  m_hInteropObject != NULL) {
		// printf("CreateInterop - wglDXUnregisterObjectNV\n");
		wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject);
		m_hInteropObject = NULL;
	}

	// Create an fbo for copying textures
	if(m_fbo) {
		// Delete the fbo before the texture so that any texture attachment is released
		// printf("CreateInterop - deleting fbo\n");
		glDeleteFramebuffersEXT(1, &m_fbo);
		m_fbo = 0;
	}
	glGenFramebuffersEXT(1, &m_fbo); 

	// Create a local opengl texture that will be linked to a shared DirectX texture
	if(m_glTexture) {
		// printf("CreateInterop - deleting texture\n");
		glDeleteTextures(1, &m_glTexture);
		m_glTexture = 0;
	}
	glGenTextures(1, &m_glTexture);

	// Create textures and GLDX interop objects
	if(bUseDX9)	bRet = CreateDX9interop(width, height, format, bReceive);
	else bRet = CreateDX11interop(width, height, format, bReceive);

	if(!bRet) {
		printf("CreateInterop error 3\n");
		return false;
	}

	// Now the global shared texture handle - m_dxShareHandle - has been set so a sender can be created
	// this creates the sender shared memory map and registers the sender
	if (!bReceive) {
		// We are done with the format
		// So for DirectX 9, set to zero to identify the sender as DirectX 9
		// LJ DEBUG - allow format to be registered becasue it is now tested
		// by revised SpoutPanel 2 and by the texture formats above
		// if(bUseDX9) format = 0; 
		if(!senders.CreateSender(sendername, width, height, m_dxShareHandle, format)) {
			printf("CreateInterop error 4\n");
			return false;
		}
	}

	// Set up local values for this instance
	// Needed for texture read and write size checks
	m_TextureInfo.width			= (unsigned __int32)width;
	m_TextureInfo.height		= (unsigned __int32)height;
	m_TextureInfo.shareHandle	= (unsigned __int32)m_dxShareHandle;
	m_TextureInfo.format		= format;

	// Initialize general texture transfer sync mutex - either sender or receiver can do this
	spoutdx.CreateAccessMutex(sendername, m_hAccessMutex);

	// Now it has initialized OK
	m_bInitialized = true;

	//
	// Now we have globals for this instance
	//
	// m_TextureInfo.width			- width of the texture
	// m_TextureInfo.height			- height of the texture
	// m_TextureInfo.shareHandle	- handle of the shared texture
	// m_TextureInfo.format			- format of the texture
	// m_glTexture					- a linked opengl texture
	// m_dxTexture					- a linked, shared DirectX texture created here
	// m_hInteropDevice				- handle to interop device created by wglDXOpenDeviceNV by init
	// m_hInteropObject				- handle to the connected texture created by wglDXRegisterObjectNV
	// m_hAccessMutex				- mutex for texture access lock
	// m_bInitialized				- whether it initialized OK

	// true means the init was OK, not the connection
	return true; 

}


//
// =================== DX9 ===============================
//
//		CreateDX9interop()
//	
//		bReceive		when receiving a texture from a DX application this must be set to true (default)
//						when sending a texture from GL to the DX application, set to false
//
bool spoutGLDXinterop::CreateDX9interop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive) 
{

	// printf("CreateDX9interop(%dx%d, [Format = %d], %d (m_pDevice = %x)\n", width, height, dwFormat, bReceive, m_pDevice);

	// The shared texture handle of the Sender texture "m_dxShareHandle" 
	// is already set by getSharedTextureInfo, but should be NULL for a sender
	if (!bReceive) {
		// printf("    sender - setting m_dxShareHandle to NULL\n");
		// Create a new shared DirectX resource m_dxTexture 
		// with new local handle m_dxShareHandle for a sender
		m_dxShareHandle = NULL; // A sender creates a new texture
	}
	
	// Safety in case an application has crashed
	if (m_dxTexture) {
		m_dxTexture->Release();
		// printf("   releasing texture\n");
	}
	m_dxTexture = NULL;

	// Create a shared DirectX9 texture - m_dxTexture
	// by giving it a sharehandle variable - m_dxShareHandle
	// For a SENDER : the sharehandle is NULL and a new texture is created
	// For a RECEIVER : the sharehandle is valid and becomes a handle to the existing shared texture
	// USAGE is D3DUSAGE_RENDERTARGET
	if(!spoutdx.CreateSharedDX9Texture(m_pDevice,
									   width,
									   height,
									   (D3DFORMAT)dwFormat,  // default is D3DFMT_A8R8G8B8
									   m_dxTexture,
									   m_dxShareHandle)) {
		printf("    CreateSharedDX9Texture failed\n");								   
		return false;
	}


	// Link the shared DirectX texture to the OpenGL texture
	// This registers for interop and associates the opengl texture with the dx texture
	// by calling wglDXRegisterObjectNV which returns a handle to the interop object
	// (the shared texture) (m_hInteropObject)
	m_hInteropObject = LinkGLDXtextures(m_pDevice, m_dxTexture, m_dxShareHandle, m_glTexture); 
	if(!m_hInteropObject) {
		printf("    LinkGLDXtextures failed\n");	
		return false;
	}


	return true;
}



//
// =================== DX11 ==============================
//
bool spoutGLDXinterop::CreateDX11interop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive ) 
{

	// Safety in case of application crash
	if(g_pd3dDevice != NULL) g_pd3dDevice->Release();
	g_pSharedTexture = NULL; // Important because mutex locks check for NULL

	// Create or use a shared DirectX texture that will be linked to the OpenGL texture
	// and get it's share handle for sharing textures
	if (bReceive) {
		// Retrieve the shared texture pointer via the sharehandle
		if(!spoutdx.OpenDX11shareHandle(g_pd3dDevice, &g_pSharedTexture, m_dxShareHandle)) {
			printf("CreateDX11interop - error 1\n");
			return false;
		}
	} else {
		// printf("CreateDX11interop - creating texture %dx%d (g_pSharedTexture = %x)\n", width, height, g_pSharedTexture);
		// otherwise create a new shared DirectX resource g_pSharedTexture 
		// with local handle m_dxShareHandle for a sender
		m_dxShareHandle = NULL; // A sender creates a new texture with a new share handle
		if(!spoutdx.CreateSharedDX11Texture(g_pd3dDevice,
											width, height, 
											(DXGI_FORMAT)dwFormat, // default is DXGI_FORMAT_B8G8R8A8_UNORM
											&g_pSharedTexture, m_dxShareHandle)) {
			printf("CreateDX11interop - error 2\n");
			return false;
		}
	}

	// Link the shared DirectX texture to the OpenGL texture
	// This registers for interop and associates the opengl texture with the dx texture
	// by calling wglDXRegisterObjectNV which returns a handle to the interop object
	// (the shared texture) (m_hInteropObject)
	m_hInteropObject = LinkGLDXtextures(g_pd3dDevice, g_pSharedTexture, m_dxShareHandle, m_glTexture); 
	if(!m_hInteropObject) return false;

	return true;

}

//	Link a shared DirectX texture to an OpenGL texture
//	and create a GLDX interop object handle
//
//	IN	pSharedTexture  Pointer to shared the DirectX texture
//	IN	dxShareHandle   Handle of the DirectX texture to be shared
//	IN	glTextureID     ID of the OpenGL texture that is to be linked to the shared DirectX texture
//	Returns             Handle to the GL/DirectX interop object (the shared texture)
//
HANDLE spoutGLDXinterop::LinkGLDXtextures (	void* pDXdevice,
											void* pSharedTexture,
											HANDLE dxShareHandle,
											GLuint glTexture) 
{

	HANDLE hInteropObject;

	// printf("LinkGLDXtextures (%x, %x, %x, %x)\n", pDXdevice, pSharedTexture, dxShareHandle, glTexture);
	// printf("    m_hInteropDevice = %x\n", m_hInteropDevice);

	// Prepare the DirectX device for interoperability with OpenGL
	// The return value is a handle to a GL/DirectX interop device.
	if(!m_hInteropDevice) {
		// printf("    LinkGLDXtextures creating interop device from %x\n", pDXdevice);
		m_hInteropDevice = wglDXOpenDeviceNV(pDXdevice);
	}

	if (m_hInteropDevice == NULL) {
		printf("    LinkGLDXtextures error 1\n");
		return false;
	}

	// prepare shared resource
	// wglDXSetResourceShareHandle does not need to be called for DirectX
	// version 10 and 11 resources. Calling this function for DirectX 10
	// and 11 resources is not an error but has no effect.
	// and 11 resources is not an error but has no effect.
	if (!wglDXSetResourceShareHandleNV(pSharedTexture, dxShareHandle)) {
		printf("    LinkGLDXtextures error 2\n");
		return NULL;
	}

	// Prepare the DirectX texture for use by OpenGL
	// register for interop and associate the opengl texture with the dx texture
	hInteropObject = wglDXRegisterObjectNV( m_hInteropDevice,
											pSharedTexture,	// DX texture
											glTexture,		// OpenGL texture
											GL_TEXTURE_2D,	// Must be TEXTURE_2D
											WGL_ACCESS_READ_WRITE_NV); // We will write and the receiver will read

	if(!hInteropObject) {
		printf("    wglDXRegisterObjectNV failed\n");
	}
	else {
		// printf("    wglDXRegisterObjectNV OK\n");
	}

	return hInteropObject;

}

// LJ DEBUG - TODO - not working
// Re-link a gl texture to the shared directX texture
bool spoutGLDXinterop::LinkGLtexture(GLuint glTexture) 
{
	// printf("LinkGLtexture(%d)\n", glTexture);
	if(g_pd3dDevice == NULL || g_pSharedTexture == NULL || m_dxShareHandle == NULL) {
		// printf("    null handles\n");
		return false;
	}

	if(m_hInteropDevice != NULL &&  m_hInteropObject != NULL) {
		// printf("    unregister\n");
		wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject);
		m_hInteropObject = NULL;
	}
	// printf("    unregister OK\n");

	if (m_hInteropDevice != NULL) {
		wglDXCloseDeviceNV(m_hInteropDevice);
	}

	// printf("    close device OK\n");
	m_hInteropDevice = NULL;
	m_hInteropObject = NULL;

	m_hInteropDevice = wglDXOpenDeviceNV(g_pd3dDevice);
	if (m_hInteropDevice == NULL) {
		// printf("    open device fail\n");
		return false;
	}
	// printf("    open device OK\n");

	// Prepare the DirectX texture for use by OpenGL
	// register for interop and associate the opengl texture with the dx texture
	m_hInteropObject = wglDXRegisterObjectNV(g_pd3dDevice, 
											 g_pSharedTexture,		// DX texture
											 glTexture,				// OpenGL texture
											 GL_TEXTURE_2D,			// Must be TEXTURE_2D
											 WGL_ACCESS_READ_WRITE_NV); // We will write and the receiver will read

	if(!m_hInteropObject) {
		// printf("    null InteropObject\n");
		return false;
	}

	// printf("    InteropObject OK\n");

	return true;

}

void spoutGLDXinterop::CleanupDirectX()
{
	if(bUseDX9)
		CleanupDX9();
	else
		CleanupDX11();
}


void spoutGLDXinterop::CleanupDX9()
{
	if(m_pD3D != NULL) m_pD3D->Release();
	if(m_pDevice != NULL) m_pDevice->Release();

	// 01.09.14 - texture release was missing for a receiver - caused a VRAM leak
	// If an existing texture exists, CreateTexture can fail with and "unknown error"
	// so delete any existing texture object
	if (m_dxTexture) m_dxTexture->Release();
	m_dxTexture = NULL;

	m_pD3D = NULL;
	m_pDevice = NULL;

}

void spoutGLDXinterop::CleanupDX11()
{
	if(g_pSharedTexture != NULL) g_pSharedTexture->Release();
	// 21.10.14 - removed due to problems
	// spoutdx.CloseDX11(); // release immediate context before releasing the device
	if(g_pd3dDevice != NULL) g_pd3dDevice->Release();
	g_pSharedTexture = NULL; // Important because mutex locks check for NULL
	g_pd3dDevice = NULL;

}


// this is the function that cleans up Direct3D and the gldx interop
// The exit flag is a fix - trouble is with wglDXUnregisterObjectNV
// which crashes on exit to the program but not if called
// while the program is running. Likely due to no GL context on exit
void spoutGLDXinterop::CleanupInterop(bool bExit)
{
	// Some of these things need an opengl context so check
	if(wglGetCurrentContext() != NULL) {

		// Problem here on exit, but not on change of resolution while the program is running !?
		// On exit there may be no openGL context but while the program is running there is
		if(!bExit && m_hInteropDevice != NULL && m_hInteropObject != NULL) {
			wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject);
			m_hInteropObject = NULL;
		}
		if (m_hInteropDevice != NULL) {
			wglDXCloseDeviceNV(m_hInteropDevice);
			m_hInteropDevice = NULL;
		}

		if(m_glTexture)	{
			glDeleteTextures(1, &m_glTexture);
			m_glTexture = 0;
		}

		if(m_fbo) {
			glDeleteFramebuffersEXT(1, &m_fbo);
			m_fbo = 0;
		}

	} // endif there is an opengl context

	CleanupDirectX();

	// Close general texture access mutex
	spoutdx.CloseAccessMutex(m_hAccessMutex);
	m_hAccessMutex = NULL; // Double check that the global handle is NULL

	m_bInitialized = false;

}

//
//	Load the Nvidia gl/dx extensions
//
bool spoutGLDXinterop::LoadGLextensions() 
{
	unsigned int caps = 0;

	caps = loadGLextensions();

	char buffer [33];
	_itoa_s(caps, buffer, 2);

	if(caps == 0) return false;

	if(caps & GLEXT_SUPPORT_FBO) bFBOavailable = true;
	if(caps & GLEXT_SUPPORT_FBO_BLIT) bBLITavailable = true;
	if(caps & GLEXT_SUPPORT_PBO) bPBOavailable = true;
	if(caps & GLEXT_SUPPORT_SWAP) bSWAPavailable = true;

	return true;
}

// 03.09.14 - MB mods for names map class
bool spoutGLDXinterop::getSharedTextureInfo(char* sharedMemoryName) {

	unsigned int w, h;
	HANDLE handle;
	DWORD format;
	if (!senders.FindSender(sharedMemoryName, w, h, handle, format))
	{
		return false;
	}

	m_dxShareHandle = (HANDLE)handle;
	m_TextureInfo.width = w;
	m_TextureInfo.height = h;
	m_TextureInfo.shareHandle = (__int32)handle;
	m_TextureInfo.format = format;

	return true;

}


// Set texture info to shared memory for the sender init
// width and height must have been set first
// 03.09.14 - MB mods for names map class
bool spoutGLDXinterop::setSharedTextureInfo(char* sharedMemoryName) {

	return senders.UpdateSender(sharedMemoryName, 
							m_TextureInfo.width,
							m_TextureInfo.height,
							m_dxShareHandle,
							m_TextureInfo.format);


}

// Return current sharing handle, width and height of a Sender
// Note - use the map directly - we must not use getSharedTextureInfo
// which resets the local info structure from shared memory !!!
// A receiver checks this all the time so it has to be compact
// 03.09.14 - MB mods for names map class
bool spoutGLDXinterop::getSharedInfo(char* sharedMemoryName, SharedTextureInfo* info) 
{
	return senders.getSharedInfo(sharedMemoryName, info);
} // end getSharedInfo


// Sets the given info structure to shared memory with the given name
// IMPORTANT: this modifies the local structure
// Used to change the texture dimensions before init
bool spoutGLDXinterop::setSharedInfo(char* sharedMemoryName, SharedTextureInfo* info)
{
	m_TextureInfo.width			= info->width;
	m_TextureInfo.height		= info->height;
	m_dxShareHandle				= (HANDLE)info->shareHandle; 
	// the local info structure handle "m_TextureInfo.shareHandle" gets converted 
	// into (unsigned __int32) from "m_dxShareHandle" by setSharedTextureInfo
	if(setSharedTextureInfo(sharedMemoryName)) {
		return true;
	}
	else {
		return false;
	}
}

// Utilities
//
// GLDXcompatible
//
bool spoutGLDXinterop::GLDXcompatible()
{
	//
	// ======= Hardware compatibility test =======
	//
	// Call LoadGLextensions for an initial hardware compatibilty check and
	// also load Glew for the Nvidia OpenGL/Directx interop extensions.
	// This will fail straight away for non Nvidia hardware but it needs
	// an additional check. It is possible that the extensions load OK, 
	// but that initialization will still fail. This occurs when wglDXOpenDeviceNV
	// fails. This has been noted on dual graphics machines with the NVIDIA Optimus driver.
	// "GLDXcompatible" tests for this by initializing directx and calling wglDXOpenDeviceNV
	// If OK and the debug flag has not been set all the parameters are available
	// Otherwise it is limited to memory share
	//
	HDC hdc = wglGetCurrentDC(); // OpenGl device context is needed
	if(!hdc) {
		// MessageBoxA(NULL, "Spout compatibility test\nCannot get GL device context", "OpenSpout", MB_OK);
		return false;
	}
	HWND hWnd = WindowFromDC(hdc); // can be null though
	if(!bExtensionsLoaded) bExtensionsLoaded = LoadGLextensions();
	if(bExtensionsLoaded) {
		// all OK and not debug memoryshare
		// try to set up directx and open the GL/DX interop
		if(OpenDirectX(hWnd, bUseDX9)) {
			// if it passes here all is well
			return true;
		}
	} // end hardware compatibility test
	return false;

} // end GLDXcompatible



// ----------------------------------------------------------
//		Access to texture using DX/GL interop functions
// ----------------------------------------------------------

// DRAW A TEXTURE INTO THE THE SHARED TEXTURE VIA AN FBO
bool spoutGLDXinterop::DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x, float max_y, float aspect, bool bInvert, GLuint HostFBO)
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		return false;
	}
	
	if(width != (unsigned  int)m_TextureInfo.width || height != (unsigned  int)m_TextureInfo.height) {
		return false;
	}

	// Wait for access to the texture
	if(spoutdx.CheckAccess(m_hAccessMutex)) {

		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {

			// Draw the input texture into the shared texture via an fbo

			// Bind our fbo
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
			glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
			glBindTexture(GL_TEXTURE_2D, m_glTexture);

			glColor4f(1.f, 1.f, 1.f, 1.f);
			glEnable(TextureTarget);
			glBindTexture(TextureTarget, TextureID);

			GLfloat tc[4][2] = {0};

			if(bInvert) {
				tc[0][0] = 0.0;   tc[0][1] = max_y;
				tc[1][0] = 0.0;   tc[1][1] = 0.0;
				tc[2][0] = max_x; tc[2][1] = 0.0;
				tc[3][0] = max_x; tc[3][1] = max_y;
			}
			else {
				tc[0][0] = 0.0;   tc[0][1] = 0.0;
				tc[1][0] = 0.0;   tc[1][1] = max_y;
				tc[2][0] = max_x; tc[2][1] = max_y;
				tc[3][0] = max_x; tc[3][1] = 0.0;
			}

			GLfloat verts[] =  {
							-aspect, -1.0,   // bottom left
							-aspect,  1.0,   // top left
							 aspect,  1.0,   // top right
							 aspect, -1.0 }; // bottom right

			glEnableClientState( GL_TEXTURE_COORD_ARRAY );
			glTexCoordPointer(2, GL_FLOAT, 0, tc );
			glEnableClientState(GL_VERTEX_ARRAY);		
			glVertexPointer(2, GL_FLOAT, 0, verts );
			glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);

			glBindTexture(TextureTarget, 0);
			glDisable(TextureTarget);

			glBindTexture(GL_TEXTURE_2D, 0);

			// restore the previous fbo - default is 0
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		}
	}
	spoutdx.AllowAccess(m_hAccessMutex); // Allow access to the texture

	return true;
}

 
// DRAW THE SHARED TEXTURE
bool spoutGLDXinterop::DrawSharedTexture(float max_x, float max_y, float aspect, bool bInvert)
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		return false;
	}

	// Wait for access to the texture
	if(spoutdx.CheckAccess(m_hAccessMutex)) {

		// go ahead and access the shared texture to draw it
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {

			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, m_glTexture); // bind shared texture
			glColor4f(1.f, 1.f, 1.f, 1.f);

			// Note origin of input texture at top left
			// FFGL examples have origin at bottom left
			// but needs to be this way or it comes out inverted
			// TODO - convert to vertex array
			glBegin(GL_QUADS);
			if(bInvert) {
				glTexCoord2f(0.0,	max_y);	glVertex2f(-aspect,-1.0); // lower left
				glTexCoord2f(0.0,	0.0);	glVertex2f(-aspect, 1.0); // upper left
				glTexCoord2f(max_x, 0.0);	glVertex2f( aspect, 1.0); // upper right
				glTexCoord2f(max_x, max_y);	glVertex2f( aspect,-1.0); // lower right
			}
			else {
				glTexCoord2f(0.0,	max_y);	glVertex2f(-aspect,  1.0); // lower left
				glTexCoord2f(0.0,	0.0);	glVertex2f(-aspect, -1.0); // upper left
				glTexCoord2f(max_x, 0.0);	glVertex2f( aspect, -1.0); // upper right
				glTexCoord2f(max_x, max_y);	glVertex2f( aspect,  1.0); // lower right
			}
			glEnd();

			glBindTexture(GL_TEXTURE_2D, 0); // unbind shared texture
			glDisable(GL_TEXTURE_2D);

			// unlock dx object
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			
			// drop through to manage events and return true;
		} // if lock failed just keep going
	}
	spoutdx.AllowAccess(m_hAccessMutex); // Allow access to the texture

	return true;

} // end DrawSharedTexture



#ifdef USE_PBO_EXTENSIONS
// TODO - this is not used - to be texted
// Load a texture into pixels via PBO - NOTE RGB only
bool spoutGLDXinterop::LoadTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, unsigned char *data)
{
	GLuint pixelBufferID;

	// PBO method - load into PBO - then texture
	// Note that PBO functions have been loaded explicitly
	// Presumably lets the GPU continue after the data has been transferred to the PBO
	// but it has an additional memcpy. 

	// Really needs two PBOs
	
	// "CopyMemory" seems a little faster than memcpy although no detailed timing tests done
	// Seems to have a small advantage, perhaps 3 or 4 fps at 1920x1080. 

	// The most time consuming part is loading from shared memory. 
	// There is a small speed advantage by putting the memoryshare read
	// inside here instead of via a local buffer (perhaps 1-2 fps) but then the changes in 
	// Sender image size are not made.
	if(bPBOavailable) {

		void *pboMemory;
		glGenBuffers(1, &pixelBufferID);

		// Bind buffer
		glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pixelBufferID);

		// Null existing data
		glBufferData(GL_PIXEL_UNPACK_BUFFER, width*height*3, NULL, GL_STREAM_DRAW);

		// Map buffer - returns pointer to buffer
		pboMemory = glMapBuffer (GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

		// Write data into pboMemory - presumably this is the time consuming bit
		// might be a very slight speed increase by using CopyMemory instead of memcpy
		CopyMemory(pboMemory, (void *)data, width*height*3);

		// Unmap buffer, indicating we are done writing data to it
		glUnmapBuffer (GL_PIXEL_UNPACK_BUFFER);

		// while PBO is still bound, transfer the data from the PBO to the texture
		glEnable(TextureTarget);
		glBindTexture (TextureTarget, TextureID);
		glTexSubImage2D(TextureTarget, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, (void *)0);
		glDisable(TextureTarget);

		// Unbind buffer
		glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);

		if(pixelBufferID) glDeleteBuffers(1, &TextureID);

	}
	else {

		//
		// Texture method using glTexSubImage2D may be subject to "GPU pipeline stalling"
		//
		// http://blogs.msdn.com/b/shawnhar/archive/2008/04/14/stalling-the-pipeline.aspx
		//
		glEnable(TextureTarget);
		glBindTexture (TextureTarget, TextureID);
		glTexSubImage2D(TextureTarget, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, (void *)data);
		glBindTexture (TextureTarget, 0);
		glDisable(TextureTarget);

	}

	return true;

}
#endif



// COPY IMAGE PIXELS TO THE SHARED TEXTURE  - note RGB default format
bool spoutGLDXinterop::WriteTexturePixels(unsigned char *pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bAlignment)
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		return false;
	}

	if(width != (unsigned int)m_TextureInfo.width || height != (unsigned int)m_TextureInfo.height) {
		return false;
	}

	// Wait for access to the texture
	if(spoutdx.CheckAccess(m_hAccessMutex)) {

		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			//
			// Check for alignment different from the default
			// We assume that alignment must be 1 byte rather than the default of 4
			// GL_UNPACK_ROW_ALIGNMENT affects how pixel data is read from client memory
			//
			if(!bAlignment) glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Force 1-byte alignment

			glBindTexture(GL_TEXTURE_2D, m_glTexture); // The  shared GL texture
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, glFormat, GL_UNSIGNED_BYTE, pixels);
			glBindTexture(GL_TEXTURE_2D, 0);

			if(!bAlignment) glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // Restore 4-byte alignment

			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		} // if lock failed just keep going
	}

	spoutdx.AllowAccess(m_hAccessMutex); // Allow access to the texture

	return true;

} // end WriteTexturePixels

//
// COPY A TEXTURE TO THE SHARED TEXTURE
//
bool spoutGLDXinterop::WriteTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		return false;
	}

	if(width != (unsigned  int)m_TextureInfo.width || height != (unsigned  int)m_TextureInfo.height) {
		return false;
	}

	/*
	// Basic code for debugging 0.85 - 0.90 msec
	wglDXLockObjectsNV(m_hInteropDevice, 1, &m_hInteropObject);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
	glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TextureTarget, TextureID, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glBindTexture(GL_TEXTURE_2D, m_glTexture);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	wglDXUnlockObjectsNV(m_hInteropDevice, 1, &m_hInteropObject);
	return true;
	*/

	// Original blit method with checks - 0.75 - 0.85 msec
	// Wait for access to the texture
	if(spoutdx.CheckAccess(m_hAccessMutex)) {

		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// fbo is a  local FBO and width/height are the dimensions of the texture.
			// "TextureID" is the source texture, and "m_glTexture" is destination texture
			// which should have been already created

			// bind the FBO (for both, READ_FRAMEBUFFER_EXT and DRAW_FRAMEBUFFER_EXT)
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

			// Attach the Input texture to the color buffer in our frame buffer - note texturetarget 
			#ifdef USE_GLEW
			glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, TextureTarget, TextureID, 0);
			#else
			glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TextureTarget, TextureID, 0);
			#endif
			glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

			// Default invert flag is false but then it comes out inverted
			// so do the flip to get it the right way up if the user wants that
			if(bInvert) {
				if (bBLITavailable) { // fbo blitting supported - flip the texture while copying
					// attach target texture to second attachment point
					#ifdef USE_GLEW
					glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, m_glTexture, 0);
					#else
					glFramebufferTexture2DEXT(DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, m_glTexture, 0);
					#endif
					glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);

					// copy one texture buffer to the other while flipping upside down (OpenGL and DirectX have different texture origins)
					glBlitFramebufferEXT(0, 0,			// srcX0, srcY0, 
										 width, height, // srcX1, srcY1
										 0, height,		// dstX0, dstY0,
										 width, 0,		// dstX1, dstY1,
										 GL_COLOR_BUFFER_BIT, GL_NEAREST); // GLbitfield mask, GLenum filter
				} 
				else { // no blitting supported - directly copy to texture line by line to invert
					// bind dx texture (destination)
					glBindTexture(GL_TEXTURE_2D, m_glTexture);
					// copy from fbo to the bound texture
					// Not useful - Line by line method is very slow !! ~ 12msec/frame 1920 hd
					for(unsigned int i = 0; i <height; i++) glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, i, 0, height-i-1, width, 1);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}
			else { 
				// no invert flag means leave it alone and it comes out inverted (bInvert false)
				glBindTexture(GL_TEXTURE_2D, m_glTexture);
				glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
				glBindTexture(GL_TEXTURE_2D, 0);		
			}

			// restore the previous fbo - default is 0
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

			// unbind the shared texture
			glBindTexture(GL_TEXTURE_2D, 0);

			// unlock dx object
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		}
		spoutdx.AllowAccess(m_hAccessMutex); // Allow access to the texture
		return true;

	}

	// There is no reader
	spoutdx.AllowAccess(m_hAccessMutex); // Allow access to the texture
	return false;

} // end WriteTexture


// COPY THE SHARED TEXTURE TO AN OUTPUT TEXTURE
bool spoutGLDXinterop::ReadTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		return false;
	}

	if(width != (unsigned int)m_TextureInfo.width || height != (unsigned int)m_TextureInfo.height) {
		return false;
	}

	/*
	// Basic code for debugging
	wglDXLockObjectsNV(m_hInteropDevice, 1, &m_hInteropObject);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
	glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TextureTarget, TextureID, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glBindTexture(GL_TEXTURE_2D, m_glTexture);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	wglDXUnlockObjectsNV(m_hInteropDevice, 1, &m_hInteropObject);
	return true;
	*/

	// Invert code
	if(spoutdx.CheckAccess(m_hAccessMutex)) {

		// lock interop
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			
			// bind the FBO (for both, READ_FRAMEBUFFER_EXT and DRAW_FRAMEBUFFER_EXT)
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
			
			// Attach the Input texture (the shared texture) to the color buffer in our frame buffer - note texturetarget 
			glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
			glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
			
			// Flip if the user wants that
			if(bInvert) { // fbo blitting supported - flip the texture while copying

				// attach target texture (the one we read into and return) to second attachment point
				glFramebufferTexture2DEXT(DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, TextureTarget, TextureID, 0);
				glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);

				// copy one texture buffer to the other while flipping upside down
				glBlitFramebufferEXT(0,     0,		// srcX0, srcY0, 
									 width, height, // srcX1, srcY1
									 0,     height,	// dstX0, dstY0,
									 width, 0,		// dstX1, dstY1,
									 GL_COLOR_BUFFER_BIT, GL_LINEAR); // NEAREST); // GLbitfield mask, GLenum filter
			}
			else { 
				// no invert flag means leave it alone
				// Copy from the fbo (shared texture attached) to the dest texture
				glBindTexture(TextureTarget, TextureID);
				glCopyTexSubImage2D(TextureTarget, 0, 0, 0, 0, 0, width, height);
				glBindTexture(TextureTarget, 0);
			}

			// restore the previous fbo - default is 0
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

			// unlock dx object
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		}
		spoutdx.AllowAccess(m_hAccessMutex); // Allow access to the texture
	}
	spoutdx.AllowAccess(m_hAccessMutex);

	return true;


	/*
	// Wait for access to the texture
	if(spoutdx.CheckAccess(m_hAccessMutex)) {

		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {

			// Bind our local fbo
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo); 

			// Attach the shared texture to the color buffer in our frame buffer
			// needs GL_TEXTURE_2D as a target for our shared texture
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
			if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) {
				// bind output texture (destination)
				glBindTexture(TextureTarget, TextureID);
				// copy from framebuffer (fbo) to the bound texture
				glCopyTexSubImage2D(TextureTarget, 0, 0, 0, 0, 0, width, height);
				// unbind the texture
				glBindTexture(TextureTarget, 0);
			}

			// restore the previous fbo - default is 0
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

			// unlock dx object
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		}
	}
	spoutdx.AllowAccess(m_hAccessMutex); // Allow access to the texture
	return true;
	*/

} // end ReadTexture


//
// COPY THE SHARED TEXTURE TO IMAGE PIXELS - 15-07-14 allowed for variable format instead of RGB only
bool spoutGLDXinterop::ReadTexturePixels(unsigned char *pixels, unsigned int width, unsigned int height, GLenum glFormat, GLuint HostFBO)
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) return false;
	if(width != m_TextureInfo.width || height != m_TextureInfo.height) return false;

	// retrieve opengl texture data directly to image pixels rather than via an fbo and texture
	// Wait for access to the texture
	if(spoutdx.CheckAccess(m_hAccessMutex)) {
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			/*
			//
			// glGetTexImage method
			//
			glBindTexture(GL_TEXTURE_2D, m_glTexture);
			glGetTexImage(GL_TEXTURE_2D, 0,  glFormat,  GL_UNSIGNED_BYTE, pixels);
			glBindTexture(GL_TEXTURE_2D, 0);
			*/


			//
			// fbo attachment method
			//
			// retrieve the current fbo
			// GLint previousFBO;
			// glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO);

			// Bind our local fbo
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo); 
			// Attach the shared texture to the color buffer in our frame buffer
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
			if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) {
				// read the pixels from the framebuffer
				glReadPixels(0, 0, width, height, glFormat, GL_UNSIGNED_BYTE, pixels);
			}
			// restore the previous fbo - default is 0
			// glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);


			// Unlock interop object
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);

			// drop through to manage events and return true;
		} // if lock failed just keep going
	}
	spoutdx.AllowAccess(m_hAccessMutex); // Allow access to the texture

	/*
	// ColourBar image for testing
	// http://stackoverflow.com/questions/6939422/how-do-i-create-a-color-bar-tv-test-pattern
	//
	typedef struct
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	} RGBA;
		
	const RGBA BAR_COLOUR[8] =
	{
		{ 255, 255, 255, 255 },  // 100% White
		{ 255, 255,   0, 255 },  // Yellow
		{   0, 255, 255, 255 },  // Cyan
		{   0, 255,   0, 255 },  // Green
		{ 255,   0, 255, 255 },  // Magenta
		{ 255,   0,   0, 255 },  // Red
		{   0,   0, 255, 255 },  // Blue
		{   0,   0,   0, 255 },  // Black
	};

	// Allocate frame buffer
	size_t      frameBytes = width*height*sizeof(RGBA);
	RGBA*       frame = (RGBA*)malloc(frameBytes);
	unsigned    columnWidth = width / 8;

	// Generate complete frame
	for (unsigned y = 0; y < height; y++) {
		for (unsigned x = 0; x < width; x++) {
			unsigned col_idx = x / columnWidth;
			frame[y*width+x] = BAR_COLOUR[col_idx];
		}
	}

	// transfer to the caller pixels
	memcpy((void *)pixels, (void *)frame, width*height*4);

	free((void*)frame);
	*/


	return true;

} // end ReadTexturePixels 


// BIND THE SHARED TEXTURE
// for use in an application - this locks the interop object and binds the shared texture
// Locks remain in place, so afterwards a call to UnbindSharedTxeture MUST be called
bool spoutGLDXinterop::BindSharedTexture()
{
	bool bRet = false;

	if(m_hInteropDevice == NULL || m_hInteropObject == NULL)
		return false;

	// Wait for access to the texture
	if(spoutdx.CheckAccess(m_hAccessMutex)) {
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// Bind our shared OpenGL texture
			glBindTexture(GL_TEXTURE_2D, m_glTexture);
			bRet = true;
		}
		else {
			bRet = false;
		}
	}

	// Leave locked for succcess, release interop lock and allow texture access for fail
	if(!bRet) {
		UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		spoutdx.AllowAccess(m_hAccessMutex); // Allow access to the texture
	}

	return bRet;

} // end BindSharedTexture


// UNBIND THE SHARED TEXTURE
// for use in an application - this unbinds the shared texture and unlocks the interop object
bool spoutGLDXinterop::UnBindSharedTexture()
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL)
		return false;
	
	// Unbind our shared OpenGL texture
	glBindTexture(GL_TEXTURE_2D,0);
	// unlock dx object
	UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
	// Allow access to the texture
	spoutdx.AllowAccess(m_hAccessMutex);
	
	return true;

} // end BindSharedTexture



/*

	GL/DX Interop lock

	A return value of TRUE indicates that all objects were
    successfully locked.  A return value of FALSE indicates an
    error. If the function returns FALSE, none of the objects will be locked.

	Attempting to access an interop object via GL when the object is
    not locked, or attempting to access the DirectX resource through
    the DirectX API when it is locked by GL, will result in undefined
    behavior and may result in data corruption or program
    termination. Likewise, passing invalid interop device or object
    handles to this function has undefined results, including program
    termination.

	Note that only one GL context may hold the lock on the
    resource at any given time --- concurrent access from multiple GL
    contexts is not currently supported.

	http://halogenica.net/sharing-resources-between-directx-and-opengl/

	This lock triggers the GPU to perform the necessary flushing and stalling
	to guarantee that the surface has finished being written to before reading from it. 

	DISCUSSION: The Lock/Unlock calls serve as synchronization points
    between OpenGL and DirectX. They ensure that any rendering
    operations that affect the resource on one driver are complete
    before the other driver takes ownership of it.

	This function assumes only one object to 

	Must return S_OK (0) - otherwise the error can be checked.

*/
HRESULT spoutGLDXinterop::LockInteropObject(HANDLE hDevice, HANDLE *hObject)
{
	DWORD dwError;
	HRESULT hr;

	if(hDevice == NULL || hObject == NULL || *hObject == NULL) {
		return E_HANDLE;
	}

	// lock dx object
	if(wglDXLockObjectsNV(hDevice, 1, hObject) == TRUE) {
		return S_OK;
	}
	else {
		dwError = GetLastError();
		switch (dwError) {
			case ERROR_BUSY :			// One or more of the objects in <hObjects> was already locked.
				hr = E_ACCESSDENIED;	// General access denied error
				// printf("	spoutGLDXinterop::LockInteropObject ERROR_BUSY\n");
				break;
			case ERROR_INVALID_DATA :	// One or more of the objects in <hObjects>
										// does not belong to the interop device
										// specified by <hDevice>.
				hr = E_ABORT;			// Operation aborted
				// printf("	spoutGLDXinterop::LockInteropObject ERROR_INVALID_DATA\n");
				break;
			case ERROR_LOCK_FAILED :	// One or more of the objects in <hObjects> failed to 
				hr = E_ABORT;			// Operation aborted
				// printf("	spoutGLDXinterop::LockInteropObject ERROR_LOCK_FAILED\n");
				break;
			default:
				hr = E_FAIL;			// unspecified error
				// printf("	spoutGLDXinterop::LockInteropObject UNKNOWN_ERROR\n");
				break;
		} // end switch
	} // end false

	return hr;

} // LockInteropObject


//
// Must return S_OK (0) - otherwise the error can be checked.
//
HRESULT spoutGLDXinterop::UnlockInteropObject(HANDLE hDevice, HANDLE *hObject)
{
	DWORD dwError;
	HRESULT hr;

	if(hDevice == NULL || hObject == NULL || *hObject == NULL) {
		return E_HANDLE;
	}

	if (wglDXUnlockObjectsNV(hDevice, 1, hObject) == TRUE) {
		return S_OK;
	}
	else {
		dwError = GetLastError();
		switch (dwError) {
			case ERROR_NOT_LOCKED :
				hr = E_ACCESSDENIED;
				// printf("	spoutGLDXinterop::UnLockInteropObject ERROR_NOT_LOCKED\n");
				break;
			case ERROR_INVALID_DATA :
				// printf("	spoutGLDXinterop::UnLockInteropObject ERROR_INVALID_DATA\n");
				hr = E_ABORT;
				break;
			case ERROR_LOCK_FAILED :
				hr = E_ABORT;
				// printf("	spoutGLDXinterop::UnLockInteropObject ERROR_LOCK_FAILED\n");
				break;
			default:
				hr = E_FAIL;
				// printf("	spoutGLDXinterop::UnLockInteropObject UNKNOWN_ERROR\n");
				break;
		} // end switch
	} // end fail

	return hr;

} // end UnlockInteropObject


bool spoutGLDXinterop::UseDX9(bool bDX9)
{
	if(bDX9) {
		// Set to DirectX 9
		// DirectX 11 is the default but is checked by OpenDirectX.
		bUseDX9 = bDX9;
		return true;
	}
	else {
		// Check for DirectX 11 availability if the user requested it
		if(spoutdx.DX11available()) {
			bUseDX9 = false;
			return true;
		}
		else {
			// Set to use DirectX 9 if DirectX 11 is not available
			bUseDX9 = true;
			return false;
		}
	}
}

bool spoutGLDXinterop::isDX9()
{
	// Check Operating system support for DirectX 11
	// It is checked with OpenDirectX but this might not have been called yet.
	// The user can call this after the Spout SetDX9 call to check if it succeeded.
	if(!spoutdx.DX11available()) bUseDX9 = true;

	// Otherwise return what has been set
	// This can be checked after directX initialization
	// to find out if DirectX 11 initialization failed
	return bUseDX9;
}

void spoutGLDXinterop::SetDX11format(DXGI_FORMAT textureformat)
{
	DX11format = textureformat;
}

void spoutGLDXinterop::SetDX9format(D3DFORMAT textureformat)
{
	DX9format = textureformat;
}


int spoutGLDXinterop::GetVerticalSync()
{
	if(!bExtensionsLoaded) bExtensionsLoaded = LoadGLextensions();

	// needed for both sender and receiver
	if(bSWAPavailable) {
		return(wglGetSwapIntervalEXT());
	}
	return 0;
}


bool spoutGLDXinterop::SetVerticalSync(bool bSync)
{
	if(!bExtensionsLoaded) bExtensionsLoaded = LoadGLextensions();

	if(bSWAPavailable) {
		if(bSync) {
			wglSwapIntervalEXT(1); // lock to monitor vsync
		}
		else {
			wglSwapIntervalEXT(0); // unlock from monitor vsync
		}
		return true;
	}
	return false;
}

// For debugging only - needs glu32.lib
void spoutGLDXinterop::GLerror() {
	/*
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		// printf("GL error = %d (0x%x) %s\n", err, err, gluErrorString(err));
	}
	*/
}	
