/*
	WinSpoutSDK.cpp

	Spout Demo Program

	Based on  a tutorial project by NEHE :

	http://nehe.gamedev.net/tutorial/texture_mapping/12038/

	Licence : http://www.gamedev.net/page/resources/_/gdnethelp/gamedevnet-open-license-r2956

	Also spinning cube vertex array example - Author: Kevin Harris :
	http://www.codesampler.com/oglsrc/oglsrc_1.htm
	http://www.codesampler.com/source/ogl_vertex_data.zip

	16-07-14	- removed resize of window to sender size for receiver
				- allowed user resizeable window
				- included option for preserving the aspect ratio of the sender
	19-07-14	- corrected rasterpos coords for text
	22-07-14	- added option for DX9 or DX11
	26-07-14	- recompiled Win32 binaries
	27-07-14	- CreateReceiver - bUseActive flag instead of null name
	28-07-14	- memory leak and map lock testing
	29-07-14	- major change to Spout SDK
					- remove handle management
					- changed map creation and release
				- fixed /DX9 compatible flag for SpoutPanel call
	30-07-14	- added vsync option, cleanup and recompile demos
	31-07-14	- used freopen_s for console outout			
	03-08-14	- update
	04-08-14	- Lock cube rotation to cycle speed rather than a fixed rotation amount
				- testing with SpoutPanel external sender entry
				- fixed fit to window after fullscreen restore if not checked
	07-08-14	- Framerate independent of driver vsync
	10-08-14	- Revised framerate control
	17-08-14	- Further revised framerate control
	19-08-14	- Activated event locks - More work on framerate.
	22.08.14	- Recompile with event locks and vsync mods for GitHub
	22.08.14	- Rebuild with new sendernames class
	25.08.14	- Used sleep-based vsync exclusively due to problems with full screen
	29.08.14	- updated executables and source to GitHub
	31.08.14	- used vertex array draw instead of quad
				- mod to sleep msec calcs to avoid hesitation
	01.09.14	- added name entry for sender
	03.09.14	- GitHub update
	21.09.14	- recompiled for mutex texture access locks
	22.09.14	- included adapter name and bpp in diagnostics
	23.09.14	- test for DirectX 11 support and include in diagnostics
	30.09.14	- Updated diagnostics
	11.10.14	- changed back to empty name for createreceiver
				  so that sender size change does not detect the active sender
	12.10.14	- recompiled for release
	17.10.14	- moved release of sender & receiver to just after destroywindow
				  to avoid DX11 release crash
	21.10.14	- Included DirectX version in capabilities dialog
				- Recompile for update V 2.001 beta
	24.10.14	- Capabilities check if OpenDirectX11 succeeded

*/
#define MAX_LOADSTRING 100
#define PI 3.14159265358979323846f

#if defined(__x86_64__) || defined(_M_X64)
	#define is64bit
// #elif defined(__i386) || defined(_M_IX86)
	// x86 32-bit
#endif


#include "stdafx.h"
#include "WinSpoutSDK.h"

#define SPOUT_IMPORT_DLL

// leak checking
// http://www.codeproject.com/Articles/9815/Visual-Leak-Detector-Enhanced-Memory-Leak-Detectio
//
// #include "vld.h"
//
// http://msdn.microsoft.com/en-us/library/x98tx3cf%28VS.71%29.aspx
//
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "..\..\SpoutSDK\Spout.h"

SpoutSender sender;		// Create a Spout sender object
SpoutReceiver receiver;	// Create a Spout receiver object

//
// ================= CHANGE COMPILE FLAGS HERE =================
// Rename the executable as necessary to get a sender/receiver pair
//
bool bReceiver      = false; // Compile for receiver (true) or sender (false)
bool bMemoryMode    = false; // Use memory share specifically (default is false)
bool bDX9mode       = false; // Use DirectX 9 instead of DirectX 11
bool bDX9compatible = true; // For DX11 senders only - compatible DX9 format for DX11 senders (default true)
// =============================================================

//
// Global Variables:
//
bool         bInitialized	= false; // Do-once intialization flag
bool         bTextureShare	= false; // Texture share compatibility flag
char         g_SenderName[256];      // Global sender name
unsigned int g_Width, g_Height;      // Global width and height
unsigned int width, height;          // Width and height returned by receivetexture
char         gldxcaps[1024];         // capability info

HINSTANCE    hInst;                  // current instance
HWND         hwndMain;
HMENU        hMenu;
HDC          hDC=NULL;               // Private GDI Device Context
HGLRC        hRC=NULL;               // Permanent Rendering Context
HWND         hWnd=NULL;              // Holds Our Window Handle
HINSTANCE    hInstance;              // Holds The Instance Of The Application

bool         keys[256];              // Array Used For The Keyboard Routine
bool         active = true;          // Window Active Flag Set To TRUE By Default
bool         bFullscreen = false;    // Windowed Mode
bool         bFsmenubar = false;     // Space bar to show the taskbar when fullscreen
bool         bTopmost = false;       // Set as topmost window
bool         bFitWindow	= true;      // Fit to window or preserve aspect ratio of the sender for a receiver

HWND         g_hwnd = NULL;          // global handle to the OpenGL render window
RECT         windowRect;             // Render window rectangle
RECT         clientRect;             // Render window client rectangle
LONG_PTR     dwStyle;                // original window style
int          nonFullScreenX;         // original window position
int          nonFullScreenY;
int          WindowPosLeft = 0;
int          WindowPosTop = 0;       // default window start position
unsigned int AddX, AddY;             // adjustment to client rect for reset of window size
RECT         WorkArea;
HWND         hwndForeground1;        // foreground window before setting topmost
HWND         hwndForeground2;        // foreground window before setting full screen
char         WindowTitle[256];       // Title of the window - also used for the sender name

GLuint       base;                   // Base Display List For The Font Set

// Cube rotation and drawing setup
GLfloat      rotx = 0;               // x rotation amount
GLfloat      roty = 0;               // y rotation amount

GLuint       cubeTexture;            // Cube texture
GLuint       myTexture;              // Local texture


// http://www.codesampler.com/oglsrc/oglsrc_1.htm
struct Vertex
{
    float tu, tv;
    float x, y, z;
};

Vertex g_cubeVertices[] =
{
	{ 0.0f,0.0f, -1.0f,-1.0f, 1.0f },
	{ 1.0f,0.0f,  1.0f,-1.0f, 1.0f },
	{ 1.0f,1.0f,  1.0f, 1.0f, 1.0f },
	{ 0.0f,1.0f, -1.0f, 1.0f, 1.0f },

	{ 1.0f,0.0f, -1.0f,-1.0f,-1.0f },
	{ 1.0f,1.0f, -1.0f, 1.0f,-1.0f },
	{ 0.0f,1.0f,  1.0f, 1.0f,-1.0f },
	{ 0.0f,0.0f,  1.0f,-1.0f,-1.0f },

	{ 0.0f,1.0f, -1.0f, 1.0f,-1.0f },
	{ 0.0f,0.0f, -1.0f, 1.0f, 1.0f },
	{ 1.0f,0.0f,  1.0f, 1.0f, 1.0f },
	{ 1.0f,1.0f,  1.0f, 1.0f,-1.0f },

	{ 1.0f,1.0f, -1.0f,-1.0f,-1.0f },
	{ 0.0f,1.0f,  1.0f,-1.0f,-1.0f },
	{ 0.0f,0.0f,  1.0f,-1.0f, 1.0f },
	{ 1.0f,0.0f, -1.0f,-1.0f, 1.0f },

	{ 1.0f,0.0f,  1.0f,-1.0f,-1.0f },
	{ 1.0f,1.0f,  1.0f, 1.0f,-1.0f },
	{ 0.0f,1.0f,  1.0f, 1.0f, 1.0f },
	{ 0.0f,0.0f,  1.0f,-1.0f, 1.0f },

	{ 0.0f,0.0f, -1.0f,-1.0f,-1.0f },
	{ 1.0f,0.0f, -1.0f,-1.0f, 1.0f },
	{ 1.0f,1.0f, -1.0f, 1.0f, 1.0f },
	{ 0.0f,1.0f, -1.0f, 1.0f,-1.0f }
};

// FPS calcs
double startTime, elapsedTime, frameTime;
double frameRate, fps;
double PCFreq = 0.0;
__int64 CounterStart = 0;

// Sleep sync control
double waitMillis = 0;
double millisForFrame = 16.66667; // 60fps default

// Mouse position
int         MouseXpos = 0;
int         MouseYpos = 0;
bool        bClicked = false;

int debugCounter = 0;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

// Forward declarations of functions included in this code module:
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM); // The old about
LRESULT CALLBACK Capabilities(HWND, UINT, WPARAM, LPARAM);// Show GL/DX capabilities
LRESULT CALLBACK UserSenderName(HWND, UINT, WPARAM, LPARAM); // enter a sender name 
static PSTR szText;
bool EnterSenderName(char *SenderName);

int FindPixelType();
void doFullScreen(bool bFullscreen);
void SaveOpenGLstate();
void RestoreOpenGLstate();
void GLerror();
bool OpenSender();
bool OpenReceiver();
void ShowSenderInfo();
void ShowReceiverInfo();
void StartCounter();
double GetCounter();


void GLerror() {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		// printf("GL error = %d (0x%x) %s\n", err, err, gluErrorString(err));
	}
}	

GLvoid BuildFont(GLvoid)								// Build Our Bitmap Font
{
	HFONT	font;										// Windows Font ID
	HFONT	oldfont;									// Used For Good House Keeping

	base = glGenLists(96);								// Storage For 96 Characters

	font = CreateFontA( -15,							// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_MEDIUM,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						"Verdana");						// Font Name

	oldfont = (HFONT)SelectObject(hDC, font);           // Selects The Font We Want
	wglUseFontBitmaps(hDC, 32, 96, base);				// Builds 96 Characters Starting At Character 32
	SelectObject(hDC, oldfont);							// Selects The Font We Want
	DeleteObject(font);									// Delete The Font
}


GLvoid KillFont(GLvoid)									// Delete The Font List
{
	glDeleteLists(base, 96);							// Delete All 96 Characters
}


GLvoid glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	    vsprintf_s(text, fmt, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base - 32);								// Sets The Base Character to 32
	glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}


// 
// Load a bitmap from the resources and convert to a texture
// http://nehe.gamedev.net/tutorial/loading_textures_from_a_resource_file__texturing_triangles/26001/
//
int LoadCubeTexture()
{
	int Status=FALSE; // Status Indicator
	HBITMAP hBMP;
	BITMAP BMP;

	glGenTextures(1, &cubeTexture);					// Create the cube texture

	// Load a bitmap from resources
	hBMP = (HBITMAP)LoadImageA(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDB_SPOUTBMP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	if (hBMP) {
		Status=TRUE;									// Set The Status To TRUE
		GetObject(hBMP,sizeof(BMP), &BMP);				// Get The Object
														// hBMP: Handle To Graphics Object
														// sizeof(BMP): Size Of Buffer For Object Information
														// Buffer For Object Information
		glPixelStorei(GL_UNPACK_ALIGNMENT,4);			// Pixel Storage Mode (Word Alignment / 4 Bytes)
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Mipmap Linear Filtering
 		// Generate Mipmapped Texture (3 Bytes, Width, Height And Data From The BMP)
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, BMP.bmWidth, BMP.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);
		DeleteObject(hBMP);// Delete The Bitmap Object
	}

	return Status;
}

// Initialize local texture for sharing
void InitTexture(int width, int height)
{

	if(myTexture != NULL) {
		glDeleteTextures(1, &myTexture);
		myTexture = NULL;
	}

	// Create a local texture for transfers
	glGenTextures(1, &myTexture);
	glBindTexture(GL_TEXTURE_2D, myTexture);

	// RGB for memoryshare, otherwise RGBA
	if(bMemoryMode)	glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	else			glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //  GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //  GL_LINEAR);

}


GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0) {									// Prevent A Divide By Zero By
		height=1;										// Making Height Equal One
	}

	glViewport(0, 0, width, height);					// Reset The Current Viewport
	
	// SPOUT Note - only if a Spout sender for the cube drawing demo
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	if(!bReceiver) gluPerspective(45.0f, (GLfloat)width/(GLfloat)height , 0.1f, 100.0f);
	
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix

}

int InitGL(int width, int height)						// All Setup For OpenGL Goes Here
{

	HGLRC glContext;
	hMenu = GetMenu(hWnd);
	HMENU hSubMenu;

	// A render window must be visible for Spout initialization to work
	// Open Spout and remove the sender selection option for memorymode
	if(bMemoryMode) {
		sender.SetMemoryShareMode();	// Force memoryshare
		receiver.SetMemoryShareMode();	// Must be set independently for each object
		hSubMenu = GetSubMenu(hMenu, 0); // File
		RemoveMenu(hSubMenu,  IDM_SENDERNAME, MF_BYCOMMAND);
		RemoveMenu(hSubMenu,  IDM_SPOUTSENDERS, MF_BYCOMMAND);
		RemoveMenu(hSubMenu,  0, MF_BYPOSITION); // and the separator
	}
	else { 
		// Set whether to use DirectX 9 or DirectX 11
		// DirectX 11 is default, but compatibility is tested before initializing DirectX
		// and DirectX 9 functions are used if the Operating system does not support DirectX 11
		// This can be tested with GetDX9().
		if(bDX9mode) {
			sender.SetDX9(true);
			receiver.SetDX9(true); // Must be set independently for each object
		}
		else {
			// DirectX 11 is default so thes calls do not need to be made
			// But if they are, DirectX 11 availability is tested
			// and false is returned if it cannot be set
			// so DirectX 11 compatibility can be checked here.
			// Otherwise DirectX 9 functions are used and this call has no effect
			sender.SetDX9(false);
			receiver.SetDX9(false);
			// Here the sender DX11 texure format can be set to be DX9 compatible or not
			// Spout SDK default is a compatible format
			// Has no effect for DirectX 9
			sender.SetDX9compatible(bDX9compatible);
		}
	}

	// ======= Hardware compatibility test =======
	// Determine hardware capabilities now, not later when all is initialized
	glContext = wglGetCurrentContext(); // should check if opengl context creation succeed
	if(glContext) {

		// 24.10.14 - check if OpenDirectX11 succeeded
		if(bReceiver) {
			bDX9mode = receiver.GetDX9();
			sprintf_s(gldxcaps, 1024, "Spout Receiver");
		}
		else {
			bDX9mode = sender.GetDX9();
			sprintf_s(gldxcaps, 1024, "Spout Sender");
		}

		if(bDX9mode)
			strcat_s(gldxcaps, 1024, " - DirectX 9\r\n");
		else
			strcat_s(gldxcaps, 1024, " - DirectX 11\r\n");


		// Get the Windows version.
		DWORD dwVersion = 0; 
		DWORD dwMajorVersion = 0;
		DWORD dwMinorVersion = 0; 
		DWORD dwBuild = 0;
		DWORD dwPlatformId;
		WORD wServicePackMajor = 0;
	    WORD wServicePackMinor = 0;
		WORD wSuiteMask = 0;
		BYTE wProductType = 0;

		bool bOsviX = true;
		bool bCanDetect = true;
		char output[256];
		size_t charsConverted = 0;
	
		OSVERSIONINFOEX osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	    if(!GetVersionEx((OSVERSIONINFO *)&osvi)) {
			bOsviX = false;
			// If that fails, try using the OSVERSIONINFO structure.
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			if(!GetVersionEx((OSVERSIONINFO *)&osvi))
				bCanDetect = false;

		}

		if(bCanDetect) {

			dwBuild = osvi.dwBuildNumber;
			dwMajorVersion = osvi.dwMajorVersion;
			dwMinorVersion = osvi.dwMinorVersion;
			dwPlatformId = osvi.dwPlatformId;
			// printf("Version is %d.%d Build (%d)\n", dwMajorVersion, dwMinorVersion, dwBuild);
			if(dwMajorVersion < 6) {
				strcat_s(gldxcaps, 1024, "Windows XP - ");
			}
			else {
				switch (dwMinorVersion) {
					case 0 : // vista
						strcat_s(gldxcaps, 1024, "Windows Vista - ");
						break;
					case 1 : // 7
						strcat_s(gldxcaps, 1024, "Windows 7 - ");
						break;
					case 2 : // 8
						strcat_s(gldxcaps, 1024, "Windows 8 - ");
						break;
					case 3 : // 8.1
						strcat_s(gldxcaps, 1024, "Windows 8.1 - ");
						break;
					default : // Later than October 2013
						strcat_s(gldxcaps, 1024, "Windows - ");
						break;
				}
			}
		}

		/*
		if(bOsviX) {
			// Here we can get additional service pack and other information
			wServicePackMajor = osvi.wServicePackMajor;
			wServicePackMinor = osvi.wServicePackMinor;
			sprintf_s(output, 256, "SP %d.%d - ", wServicePackMajor, wServicePackMinor);
			strcat_s(gldxcaps, 1024, output);
		}
		*/

		// DirectX 11 only available for Windows 7 (6.1) and higher

		#ifdef is64bit
				strcat_s(gldxcaps, 1024, "64bit\r\n");
		#else
				strcat_s(gldxcaps, 1024, "32bit\r\n");
		#endif

		// Additional info
		DISPLAY_DEVICE DisplayDevice;
		DisplayDevice.cb = sizeof(DISPLAY_DEVICE);
		if(EnumDisplayDevices(NULL, 0, &DisplayDevice, 0)) {
			wcstombs_s(&charsConverted, output, 129, DisplayDevice.DeviceKey, 128);
			// printf("DeviceKey = %s\n", output); // This is the registry key with all the information about the adapter

			HKEY hRegKey;
			DWORD dwSize, dwKey;  

			// Extract the subkey from the DeviceKey string
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
					strcat_s(gldxcaps, 1024, output);
					strcat_s(gldxcaps, 1024, "\r\n");
				}
				if(RegQueryValueExA(hRegKey, "DriverVersion", NULL, &dwKey, (BYTE*)output, &dwSize) == 0) {
					strcat_s(gldxcaps, 1024, "Driver : ");
					// Find the last 6 characters of the version string then
					// convert to a float and multiply to get decimal in the right place
					sprintf_s(output, 256, "%5.2f", atof(output + strlen(output)-6)*100.0);
					strcat_s(gldxcaps, 1024, output);
				} // endif DriverVersion
				RegCloseKey(hRegKey);
			} // endif RegOpenKey
		} // endif EnumDisplayDevices

		
		// Bits per pixel
		hDC=GetDC(hWnd);
		if (hDC) {
			int bitsPerPixel = GetDeviceCaps(hDC, BITSPIXEL); //to get current system's color depth
			sprintf_s(output, 256, " (%d bpp)", bitsPerPixel);
			strcat_s(gldxcaps, 1024, output);
		}

		strcat_s(gldxcaps, 1024, "\r\n");

		// Now we can call an initial hardware compatibilty check
		// This checks for the NV_DX_interop extensions and will fail
		// if the graphics deliver does not support them, or fbos
		// ======================================================
		if(wglGetProcAddress("wglDXOpenDeviceNV")) { // extensions loaded OK
			// It is possible that the extensions load OK, but that initialization will still fail
			// This occurs when wglDXOpenDeviceNV fails - noted on dual graphics machines with NVIDIA Optimus
			// Directx initialization seems OK with null hwnd, but in any case we will not use it.
			// printf("GetMemoryShareMode\n");
			bool bMem;
			if(bReceiver) 
				bMem = receiver.GetMemoryShareMode();
			else
				bMem = sender.GetMemoryShareMode();
			if (!bMem) { // Test for memoryshare initialization
				if(wglGetProcAddress("glBlitFramebufferEXT"))
					strcat_s(gldxcaps, 1024, "Compatible hardware\r\nNV_DX_interop extensions supported\r\nInterop load successful\r\nTexture sharing mode available\r\nFBO blit available");
				else
					strcat_s(gldxcaps, 1024, "Compatible hardware\r\nNV_DX_interop extensions supported\r\nInterop load successful\r\nTexture sharing mode available\r\nFBO blit not available");
			}
			else {
				strcat_s(gldxcaps, 1024,  "Compatible hardware\r\nNV_DX_interop extensions supported\r\nbut wglDXOpenDeviceNV failed to load\r\nLimited to memory share mode");
			}
		}
		else {
			// Determine whether fbo support is the reason or interop
			if(!wglGetProcAddress("glGenFramebuffersEXT"))
				strcat_s(gldxcaps, 1024,  "Hardware does not support EXT_framebuffer_object extensions\r\nTexture sharing not available\r\nLimited to memory share mode");
			else
				strcat_s(gldxcaps, 1024,  "Hardware does not support NV_DX_interop extensions\r\nTexture sharing not available\r\nLimited to memory share mode");
		}

	}
	else {
		strcat_s(gldxcaps, 1024,  "No GL context");
	}


	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	BuildFont();										// Build The Font

	// Set start time for FPS calculation
	// Initialize timing variables
	fps       = 60.0; // give a realistic starting value
	frameRate = 60.0;
	startTime = (double)timeGetTime();

	// set initial values
	g_Width  = width;
	g_Height = height;
	// Non-alphanumeric name to begin so that CreateReceiever finds the active sender
	g_SenderName[0] = 0; 

	// Update the local texture
	InitTexture(g_Width, g_Height);


	return TRUE;
}

bool OpenReceiver()
{
	//
	// SPOUT CreateReceiver
	//
	// Returns :
	// name						- name of the sender found if the name passed was null
	// width, height			- width and height of the sender
	// Returns true for success or false for initialisation failure.
	//

	// Testing of finding a given sender name - tested OK
	// strcpy_s(g_SenderName, 256,	"Spout SDK DX11 Sender 32bit");
	// Test of empty name, original method - tested OK
	// if(receiver.CreateReceiver(g_SenderName, g_Width, g_Height)) {
	// Test of user specify finding the active sender - tested OK
	// printf("OpenReceiver 1\n");
	if(receiver.CreateReceiver(g_SenderName, g_Width, g_Height, true)) {
		// Update the local texture
		InitTexture(g_Width, g_Height);
		// Check whether DirectX 11 initialization succeeded
		if(sender.GetDX9()) bDX9mode = true;
		return true;
	}
	// printf("OpenReceiver 5\n");

	return false;

} // end OpenReceiver


bool OpenSender()
{
	//
	// Initialize a Spout sender
	//

	// First load the cube image for this demo
	if (!LoadCubeTexture()) {	// Jump To Texture Loading Routine
		return false;			// If Texture Didn't Load Return FALSE
	}

	// Initialize a local texture
	InitTexture(g_Width, g_Height);

	// Give the sender a name - the window title will do
	strcpy_s(g_SenderName, 256, WindowTitle);

	// SPOUT CreateSender
	// name						- name of this sender
	// width, height			- width and height of this sender
	// dwFormat					- optional DX11 texture format
	// DXGI_FORMAT_R8G8B8A8_UNORM - DX11 < > DX11
	// DXGI_FORMAT_B8G8R8A8_UNORM - DX11 < > DX9 (default)
	// Also sender.SetDX9compatible(true / false);
	// Returns true for success or false for initialisation failure.
	
	// For success check whether DirectX 11 initialization succeeded
	if(sender.CreateSender(g_SenderName, g_Width, g_Height)) {
		if(sender.GetDX9()) bDX9mode = true; // TODO downgrade if DX11 did not initialize
		return true;
	}

	return false;

	return(sender.CreateSender(g_SenderName, g_Width, g_Height));

} // end OpenSender


// Here's Where We Do All The Drawing
int DrawGLScene(GLvoid)
{
	StartCounter();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity(); // Reset The View
	
	if(bReceiver) {

		if(!bInitialized) {
			bInitialized = OpenReceiver();
			ShowReceiverInfo();
			return TRUE;
		}

		//
		// Receive a shared texture
		//
		//	Success :
		//		Returns the sender name, width and height
		//	Failure :
		//		No sender detected
		//
		if(receiver.ReceiveTexture(g_SenderName, width, height, myTexture, GL_TEXTURE_2D)) {
			
			// Received the texture OK, but the sender or texture dimensions could have changed
			// The global name is already changed but the width and height also may be changed
			if(width != g_Width || height != g_Height) {

				// Reset the global width and height
				g_Width  = width;
				g_Height = height;

				//	METHOD 1 - the local texture has to be resized.
				InitTexture(g_Width, g_Height);
				
				return TRUE; // Return for the next round
			} // endif sender has changed
	
			// Received OK at the current size

			SaveOpenGLstate(); // Aspect ratio control

			/*
			// METHOD 1 - use the local received texture
			glPushMatrix();
			glColor4f(1.f, 1.f, 1.f, 1.f);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, myTexture); // bind shared texture
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 1.0);	glVertex2f(-1,-1); // lower left
			glTexCoord2f(0.0, 0.0);	glVertex2f(-1, 1); // upper left
			glTexCoord2f(1.0, 0.0);	glVertex2f( 1, 1); // upper right
			glTexCoord2f(1.0, 1.0);	glVertex2f( 1,-1); // lower right
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
			glPopMatrix();
			*/

			// METHOD 2 - Draw the shared texture
			glPushMatrix();
			glColor4f(1.f, 1.f, 1.f, 1.f);
			receiver.DrawSharedTexture();
			glPopMatrix();

			/*
			// METHOD 3 - use the shared texture
			glPushMatrix();
			glColor4f(1.f, 1.f, 1.f, 1.f);
			glEnable(GL_TEXTURE_2D);
			receiver.BindSharedTexture();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 1.0);	glVertex2f(-1,-1); // lower left
			glTexCoord2f(0.0, 0.0);	glVertex2f(-1, 1); // upper left
			glTexCoord2f(1.0, 0.0);	glVertex2f( 1, 1); // upper right
			glTexCoord2f(1.0, 1.0);	glVertex2f( 1,-1); // lower right
			glEnd();
			receiver.UnBindSharedTexture();
			glDisable(GL_TEXTURE_2D);
			*/

			RestoreOpenGLstate();

		} // endif received OK
		else {
			if(bInitialized) receiver.ReleaseReceiver();
			bInitialized = false; // reset for next round
			g_SenderName[0] = 0; // LJ DEBUG
		}
		ShowReceiverInfo();
		// LJ DEBUG receiver FPS debugging
		return TRUE; // Return for the next round
	} // endif receiver
	else {
		if(!bInitialized) {
			bInitialized = OpenSender();
			if(!bInitialized) {
				// Failure is final so quit the whole program
				return FALSE;
			}
			return TRUE;
		}

		//
		// Sender demo graphics
		//

		// glClearColor(0.039f, 0.392f, 0.549f, 1.0f); // Background 10, 100, 140
		// glClearColor(0.000f, 0.300f, 0.500f, 1.0f); // deep blue
		// glClearColor(0.000f, 0.200f, 0.400f, 1.0f); // deeper blue
		glClearColor(0.39f, 0.025f, 0.000f, 1.0f); // red/brown

		glPushMatrix();

		// Rotating cube
		glTranslatef(0.0f, 0.0f, -5.0f);
		glRotatef(rotx,1.0f,0.0f,0.0f);
		glRotatef(roty,0.0f,1.0f,0.0f);
		glColor3f(1.0, 1.0, 1.0);

		// Enable Texture Mapping here - not globally or text is affected
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);

	    glInterleavedArrays( GL_T2F_V3F, 0, g_cubeVertices );
		glDrawArrays( GL_QUADS, 0, 24 );

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);

		glPopMatrix();

		// Grab the screen into a local texture
		glBindTexture(GL_TEXTURE_2D, myTexture);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, g_Width, g_Height);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Send the texture
		sender.SendTexture(myTexture, GL_TEXTURE_2D, g_Width, g_Height);
	
		ShowSenderInfo();

	} // end sender

	//
	// fps
	//
	// Sleep sync control regardless of whether the driver is set to disable vsync
	// Adapted from openframeworks ofAppGlutWindow
	// jorge's fix for idle / setFrameRate()
	// https://github.com/openframeworks/openFrameworks/blob/master/CHANGELOG.md
	// Setting swap interval is not useful because the driver settings over-ride and it has no effect
	// Swap interval is returned OK depending on the NVIDAI control panel settings
	// but has no effect full screen
	//
	elapsedTime = GetCounter(); // Higher resolution than timeGetTime()
	waitMillis = 0.0;
	frameTime = elapsedTime/1000.0; // In seconds
	if (elapsedTime > millisForFrame){
		; // We do nothing, we are already slower than target frame rate
	} else {
		waitMillis = millisForFrame - elapsedTime;
		frameTime = (elapsedTime + waitMillis)/1000.0; // Seconds per frame
		int iSleep = (int)waitMillis-1; // Prevents hesitation.
		if(iSleep < 0) iSleep = 0;
		Sleep((DWORD)iSleep);
	}

	// FPS calculations
	if( frameTime  > 0.01) {
		fps	= 1.0 / frameTime;
		// damping
		frameRate *= 0.99f;
		frameRate += 0.01f*fps;
	}

	rotx += 0.5;
	roty += 0.5;

	return TRUE; // Keep Going
}

void StartCounter()
{
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li)) {
		printf("QueryPerformanceFrequency failed!\n");
	}

    PCFreq = double(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}

double GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/PCFreq;
}

void ShowReceiverInfo()
{
	GetClientRect(hWnd, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

	// Show what a receiver is receiving
	if(!bFullscreen) {

		glMatrixMode( GL_PROJECTION );
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D( 0, width, 0, height );
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadIdentity();
		glColor3f(1.0f, 1.0f, 1.0f);

		if(bInitialized) {
			if(bMemoryMode) {
				glRasterPos2i( 20, height-30 );
				glPrint("Memoryshare Receiver - fps %2.0f", frameRate, 0.0f);
			}
			else {
				glRasterPos2i( 20, height-30 );
				glPrint("Receiving from : [%s] - fps %2.0f", g_SenderName, frameRate, 0.0f);
				glRasterPos2i( 20, 40 );
				glPrint("RH click to select  a sender", 0.0f);
			}
			glRasterPos2i( 20, 20 );
 			glPrint("' f  ' full screen", 0.0f);
		}
		else {
			// Indicate no sender
			if(bMemoryMode) {
				glRasterPos2i( 20, height-30 );
				glPrint("Memoryshare Receiver", 0.0f);
			}
			else {
				glRasterPos2i( 20, height-30 );
				glPrint("Textureshare receiver", 0.0f);
			}
			glRasterPos2i( 20, height-50 );
			glPrint("No sender detected", 0.0f);
		}

		glPopMatrix();
		glMatrixMode( GL_PROJECTION );
		glPopMatrix();
		glMatrixMode( GL_MODELVIEW );

	}

} // end ShowReceiverInfo


void ShowSenderInfo()
{

	GetClientRect(hWnd, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

	// Show what a sender is sending
	if(!bFullscreen) {
		glMatrixMode( GL_PROJECTION ); // Set matrix mode
		glPushMatrix();
		// Set an ortho projection based on window size
		glLoadIdentity();
		gluOrtho2D( 0, width, 0, height );
		glMatrixMode( GL_MODELVIEW ); // Switch back to model-view matrix
		glPushMatrix();
		glLoadIdentity();
		glColor3f(1.0f, 1.0f, 1.0f);

		if(bMemoryMode) {
			glRasterPos2i( 20, height-30 );
			glPrint("Memoryshare Sender - fps %2.0f", frameRate, 0.0f);
		}
		else {
			glRasterPos2i( 20, height-30 );
			glPrint("Sending as : [%s] - fps %2.0f", g_SenderName, frameRate, 0.0f);
		}
		
		glPopMatrix();
		glMatrixMode( GL_PROJECTION );
		glPopMatrix();
		glMatrixMode( GL_MODELVIEW );
	}

} // end ShowSenderInfo


// Properly Kill The Window
GLvoid KillGLWindow(GLvoid)
{
	
	if (hRC) {											// Do We Have A Rendering Context?
		if (!wglMakeCurrent(NULL,NULL))	{				// Are We Able To Release The DC And RC Contexts?
			MessageBoxA(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC)) {					// Are We Able To Delete The RC?
			MessageBoxA(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC)) {					// Are We Able To Release The DC
		MessageBoxA(NULL,"Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd)) {					// Are We Able To Destroy The Window?
		MessageBoxA(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClassA("OpenGL",hInstance)) {			// Are We Able To Unregister Class
		MessageBoxA(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}

	KillFont();

}

/*	This Code Creates Our OpenGL Window.  Parameters Are:
 	Title			- Title To Appear At The Top Of The Window
 	width			- Width Of The GL Window
 	height			- Height Of The GL Window
 	bits			- Number Of Bits To Use For Color (8/16/24/32)
*/
BOOL CreateGLWindow(char* title, int width, int height, int bits)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values

	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	WindowRect.bottom += (long)GetSystemMetrics(SM_CYMENU); // for a menu

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPOUTICON) );	// Load The Spout Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= MAKEINTRESOURCE(IDC_WINSPOUT);		// The menu
	wc.lpszClassName	= L"OpenGL";							// Set The Class Name

	if (!RegisterClass(&wc)) {									// Attempt To Register The Window Class
		MessageBoxA(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;				// Window Extended Style
	
	// SPOUT
	// Centre on the desktop work area
	SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&WorkArea, 0);
	WindowPosLeft += ((WorkArea.right-WorkArea.left) - (WindowRect.right-WindowRect.left))/2;
	WindowPosTop  += ((WorkArea.bottom-WorkArea.top) - (WindowRect.bottom-WindowRect.top))/2;

	dwStyle = WS_OVERLAPPEDWINDOW; // default style

	// Remove maximize and minimize for a sender
	if(!bReceiver) {
		dwStyle ^= WS_THICKFRAME; // Not resizeable
		dwStyle ^= WS_MAXIMIZEBOX;
		dwStyle ^= WS_MINIMIZEBOX;
	}
	WindowPosLeft += SM_CXFIXEDFRAME;
	WindowPosTop  += SM_CYFIXEDFRAME;

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);	// Adjust Window To True Requested Size

	// Create The Window
	hWnd=CreateWindowExA(	dwExStyle,							// Extended Style For The Window
							"OpenGL",							// Class Name
							title,								// Window Title
							dwStyle |							// Defined Window Style
							WS_CLIPSIBLINGS |					// Required Window Style
							WS_CLIPCHILDREN,					// Required Window Style
							// SPOUT - give it a position instead of zero
							WindowPosLeft, WindowPosTop,		// Window Position
							WindowRect.right-WindowRect.left,	// Calculate Window Width
							WindowRect.bottom-WindowRect.top,	// Calculate Window Height
							NULL,								// No Parent Window
							NULL,								// No Menu
							hInstance,							// Instance
							NULL);
	
	if (!hWnd) {
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	// Remove the sender selection option for a sender
	hMenu = GetMenu(hWnd);
	HMENU hSubMenu;
	hSubMenu = GetSubMenu(hMenu, 0); // File
	if(!bReceiver) { // sender
		RemoveMenu(hSubMenu,  IDM_SPOUTSENDERS, MF_BYCOMMAND);
		// Remove full screen
		hSubMenu = GetSubMenu(hMenu, 1); // Window
		RemoveMenu(hSubMenu, IDM_FULLSCREEN, MF_BYCOMMAND);
		RemoveMenu(hSubMenu, IDM_FITWINDOW, MF_BYCOMMAND);
	}
	else {
		RemoveMenu(hSubMenu,  IDM_SENDERNAME, MF_BYCOMMAND);
	}

	int bitsPerPixel = bits; // default passed int
	// If there is a DC get the current pixel depth
	hDC=GetDC(hWnd);
	if (hDC) {
		bitsPerPixel = GetDeviceCaps(hDC, BITSPIXEL); //to get current system's color depth
	}

	//
	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		// bits,									// Select Our Color Depth
		(BYTE)bitsPerPixel,							// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	hDC=GetDC(hWnd);
	if (!hDC)	{						// Did We Get A Device Context?
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	PixelFormat=ChoosePixelFormat(hDC,&pfd);
	if (!PixelFormat) {	// Did Windows Find A Matching Pixel Format?
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd)) {		// Are We Able To Set The Pixel Format?
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	hRC=wglCreateContext(hDC);
	if (!hRC) {				// Are We Able To Get A Rendering Context?
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC)) {					// Try To Activate The Rendering Context
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL(width, height))	{					// Initialize Our Newly Created GL Window
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}
	return TRUE;									// Success
}


// DEBUG - does windows support alpha ?
int FindPixelType() {

	PIXELFORMATDESCRIPTOR  pfd;
	HDC  hdc;
	int  iPixelFormat;

	hdc=GetDC(hWnd);
	if (!hdc)
		return -1;

	// get the current pixel format index  
	iPixelFormat = GetPixelFormat(hdc); 
 
	// obtain a detailed description of that pixel format  
	DescribePixelFormat(hdc, iPixelFormat, 
					    sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	
	// PFD_TYPE_RGBA = 0
	// PFD_TYPE_COLORINDEX  = 1

	return(pfd.iPixelType);

}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	int wmId, wmEvent;
	HMENU hmenu;
	wmId    = LOWORD(wParam);
	wmEvent = HIWORD(wParam);

	hmenu = GetMenu(hWnd);

	switch (uMsg) {	
		
		// Check For Windows Messages
		case WM_ACTIVATE: {							// Watch For Window Activate Message
			if (!HIWORD(wParam)) {					// Check Minimization State
				active=TRUE;						// Program Is Active
			}
			else {
				active=FALSE;						// Program Is No Longer Active
			}
			return 0;								// Return To The Message Loop
		}

		case WM_SYSCOMMAND:	{						// Intercept System Commands
			switch (wParam)	{						// Check System Calls
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}

		case WM_COMMAND:

			// Parse the menu selections:
			switch (wmId) {

				// Window
				case IDM_TOPMOST:
					bTopmost = !bTopmost;
					if(bTopmost) {
						// get the current top window
						hwndForeground1 = GetForegroundWindow();
						SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 
						CheckMenuItem (hmenu, IDM_TOPMOST, MF_BYCOMMAND | MF_CHECKED);
						ShowWindow(hWnd, SW_SHOW);
					}
					else {
						SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
						CheckMenuItem (hmenu, IDM_TOPMOST, MF_BYCOMMAND | MF_UNCHECKED);
						ShowWindow(hWnd, SW_SHOW);
						// Reset the window that was top before
						if(GetWindowLong(hwndForeground1, GWL_EXSTYLE) & WS_EX_TOPMOST)
							SetWindowPos(hwndForeground1, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 
						else
							SetWindowPos(hwndForeground1, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 
					}
				break;

				case IDM_FULLSCREEN: // removed for a sender
					if(bReceiver) {
						bFullscreen = true;
						doFullScreen(bFullscreen);
					}
				break;

				case IDM_FITWINDOW: // removed for a sender
					if(bReceiver) {
						bFitWindow = !bFitWindow;
						if(bFitWindow) 
							CheckMenuItem (hmenu, IDM_FITWINDOW, MF_BYCOMMAND | MF_CHECKED);
						else 
							CheckMenuItem (hmenu, IDM_FITWINDOW, MF_BYCOMMAND | MF_UNCHECKED);
					}
				break;

				// Help
				case IDM_ABOUT:
					DialogBoxA(hInstance, MAKEINTRESOURCEA(IDD_ABOUTBOX), hWnd, About);
				break;
		
				case IDM_CAPS:
					DialogBoxA(hInstance, MAKEINTRESOURCEA(IDD_CAPSBOX), hWnd, (DLGPROC)Capabilities);
				break;

				// File
				case IDM_SPOUTSENDERS: // Receiver select a sender
					if(bReceiver) {
						// Tells spoutpanel to only list senders with DX9 compatible format
						if(bDX9mode || bDX9compatible) {
							receiver.SelectSenderPanel("/DX9");
						}
						else {
							receiver.SelectSenderPanel("/DX11"); // "/DX11" is optional default
						}
					}
					break;

				case IDM_SENDERNAME: // Sender name entry using a text dialog (for a sender)
					if(!bReceiver) {
						char sendername[256];
						strcpy_s(sendername, g_SenderName);
						if(EnterSenderName(sendername)) {
							if(strcmp(sendername, g_SenderName) != 0) {
								// Release the current sender and start again
								strcpy_s(g_SenderName, 256, sendername);
								sender.ReleaseSender();
								sender.CreateSender(g_SenderName, g_Width, g_Height);
							}
						}
					}
					break;

				case IDM_EXIT:
					PostQuitMessage(0); // Send A Quit Message
					return 0; // Jump Back
		
				default:
					break;
			}
			break;

		case WM_CLOSE: {							// Did We Receive A Close Message?
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		case WM_KEYDOWN: {							// Is A Key Being Held Down?
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}

		case WM_RBUTTONDOWN : {
			MouseXpos = LOWORD(lParam); 
			MouseYpos = HIWORD(lParam);
			bClicked = true;
			return 0;
		}
		
		case WM_RBUTTONUP : {
			bClicked = false;
			return 0;
		}

		case WM_KEYUP: {							// Has A Key Been Released?
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}

		case WM_SIZE: {								// Resize The OpenGL Window
			ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}


	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}


int APIENTRY _tWinMain(	HINSTANCE hInstance,
						HINSTANCE hPrevInstance,
						LPTSTR    lpCmdLine,
						int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(nCmdShow);

	MSG		msg;									// Windows Message Structure
	BOOL	done=FALSE;								// Bool Variable To Exit Loop

	/*
	// Debug console window so printf works
	FILE* pCout;
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("WinSpoutSDK\n");
	*/

	// suppress warnings
	msg.wParam = 0;

	// Create Our OpenGL Window

	// Global width and height
	g_Width  = 640;
	g_Height = 360; 

	if(bDX9mode)
		strcpy_s(WindowTitle, 256, "Spout ");
	else
		strcpy_s(WindowTitle, 256, "Spout ");

	#ifdef is64bit
		if(bReceiver)
			strcat(WindowTitle, "Receiver 64bit"); // Sender name
		else
			strcpy_s(WindowTitle, "Sender 64bit"); // Sender name
	#else
		if(bReceiver)
			strcat_s(WindowTitle, 256, "Receiver 32bit"); // Sender name
		else
			strcat_s(WindowTitle, 256, "Sender 32bit"); // Sender name
	#endif
	
	// HDC	GLhdc = wglGetCurrentDC();
	if (!CreateGLWindow(WindowTitle, g_Width, g_Height, 16))	{
		return 0;										// Quit If Window Was Not Created
	}

	while(!done) {										// Loop That Runs While done=FALSE

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {	// Is There A Message Waiting?
			if (msg.message == WM_QUIT) {				// Have We Received A Quit Message?
				done=TRUE;								// ESC Signalled A Quit							// If So done=TRUE
			}
			else {										// If Not, Deal With Window Messages
				TranslateMessage(&msg);					// Translate The Message
				DispatchMessage(&msg);					// Dispatch The Message
			}
		}
		else {											// If There Are No Messages
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if (active) {								// Program Active?

				// SPOUT
				if(bReceiver && !bMemoryMode && bClicked) {

					if(bFullscreen) ShowCursor(TRUE);

					// The following is debug to simulate DX9 receivers
					// This application is DX11 and will receive from all formats
					// Texture compatibility argument /DX9
					// Tells spoutpanel to only list senders with DX9 compatible format
					if(bDX9mode || bDX9compatible)
						receiver.SelectSenderPanel("/DX9");
					else
						receiver.SelectSenderPanel("/DX11");
					
					if(bFullscreen) ShowCursor(FALSE);
					bClicked = false;
				}

				else if (keys[VK_ESCAPE]) {				// Was ESC Pressed?
					keys[VK_ESCAPE]=FALSE;
					if(bReceiver && bFullscreen) {
						bFullscreen = false;
						doFullScreen(bFullscreen);
					}
				}

				else if (keys[VK_SPACE]) {					// Is the space bar Being Pressed?
					keys[VK_SPACE]=FALSE;

					// LJ DEBUG - test
					// printf("SPACE\n");
					// sender.SenderDebug(g_SenderName, sizeof(SharedTextureInfo) );
					
					/*
					if(bFullscreen) {
						bFsmenubar = !bFsmenubar;

						// HDC	GLhdc = wglGetCurrentDC();
						g_hwnd = WindowFromDC(GLhdc); 
						hWndTaskBar = FindWindowA("Shell_TrayWnd", "");
						GetWindowRect(hWndTaskBar, &rectTaskBar);

						if(bFsmenubar) {
							SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&rectWorkArea, 0);
							SetWindowPos(g_hwnd, HWND_TOP, 0, 0, (rectWorkArea.right-rectWorkArea.left), (rectWorkArea.bottom-rectWorkArea.top), SWP_SHOWWINDOW);
							SetWindowPos(hWndTaskBar, HWND_TOPMOST, 0, 0, (rectTaskBar.right-rectTaskBar.left), (rectTaskBar.bottom-rectTaskBar.top), SWP_NOMOVE | SWP_NOSIZE  | SWP_FRAMECHANGED);
							SetFocus(g_hwnd);
							ShowCursor(TRUE);
						}
						else {
							SetWindowPos(hWndTaskBar, HWND_NOTOPMOST, 0, 0, (rectTaskBar.right-rectTaskBar.left), (rectTaskBar.bottom-rectTaskBar.top), SWP_NOMOVE | SWP_NOSIZE);
							SetWindowPos(g_hwnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
							ShowCursor(FALSE);
						}
					}
					*/
				}

				else if (keys['F']) {		// Is "f" Being Pressed?
					keys['F']=FALSE;		// If So Make Key FALSE
					if(bReceiver) {
						bFullscreen = !bFullscreen;
						doFullScreen(bFullscreen);
					} // endif receiver
				} // end "F" fullscreen

				else {								// Not Time To Quit, Update Screen
					
					if(!DrawGLScene()) {			// Draw The Scene
						done=TRUE;	// Signal Quit for failure
					}

					SwapBuffers(hDC);				// Swap Buffers (Double Buffering)

					// To glFinish or not to glFinish, that is the question!
					// http://www.opengl.org/wiki/Swap_Interval
					//
					// The CPU thread can be synchronized with the buffer swap by calling glFinish() 
					// after issuing the swap. This introduces a penalty as it kills throughput advantages
					// offered by the GL pipeline when the video card is under consistent activity.
					// Video latency becomes the same as the vertical refresh period
					//
					// also see : http://www.opengl.org/wiki/Talk:Swap_Interval
					//
					// SwapBuffers() combined with glFinish() is the only method supported on
					// modern hardware to perfectly achieve vertical synchronization (vsync).
					//
					// LJ DEBUG - has no discernable effect on sync
					// glFinish();
				}

			} // endif active
		} // end draw
	} // done

	// Shutdown
	KillGLWindow();			// Kill The OpenGL Window

	// MS Leak checking
	// _CrtDumpMemoryLeaks();
	// MessageBoxA(NULL, "Exit", "WinSpout", MB_OK);
	// OutputDebugStringA("**** WinSpout Finished ****\n");

	receiver.ReleaseReceiver();
	sender.ReleaseSender();

	// LJ DEBUG
	// sender.SenderDebug(g_SenderName, sizeof(SharedTextureInfo) );
	// MessageBoxA(NULL, "Finished", "WinSpout", MB_OK);

	return (int)msg.wParam;
}

void doFullScreen(bool bFullscreen)
{
	RECT rectWorkArea;
	RECT rectTaskBar;
	HWND hWndTaskBar; 
	HWND hWndMode;
	HDC GLhdc;

	if(bFullscreen) { // set to full screen

		// get the current top window
		hwndForeground2 = GetForegroundWindow();

		// Get the client/window adjustment values
		GetWindowRect(hWnd, &windowRect);
		GetClientRect(hWnd, &clientRect);
		AddX = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
		AddY = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
		nonFullScreenX = clientRect.right - clientRect.left;
		nonFullScreenY = clientRect.bottom - clientRect.top;

		// find the OpenGL render window
		GLhdc = wglGetCurrentDC();
		g_hwnd = WindowFromDC(GLhdc); 
		GetWindowRect(g_hwnd, &windowRect); // preserve current size values
		GetClientRect(g_hwnd, &clientRect);
		dwStyle = GetWindowLongPtrA(g_hwnd, GWL_STYLE);
		SetWindowLongPtrA(g_hwnd, GWL_STYLE, WS_VISIBLE); // no other styles but visible
							
		hMenu = GetMenu(g_hwnd);
		RemoveMenu(hMenu, 2, MF_BYPOSITION); // Help
		RemoveMenu(hMenu, 1, MF_BYPOSITION); // Window
		RemoveMenu(hMenu, 0, MF_BYPOSITION); // File

		hWndTaskBar = FindWindowA("Shell_TrayWnd", "");
		GetWindowRect(hWnd, &rectTaskBar);

		// Hide or show the System Task Bar
		if(bFsmenubar) {
			SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&rectWorkArea, 0);
			SetWindowPos(g_hwnd, HWND_TOP, 0, 0, (rectWorkArea.right-rectWorkArea.left), (rectWorkArea.bottom-rectWorkArea.top), SWP_SHOWWINDOW);
			// SWP_FRAMECHANGED is important or the taskbar doesn't get repainted
			SetWindowPos(hWndTaskBar, HWND_TOPMOST, 0, 0, (rectTaskBar.right-rectTaskBar.left), (rectTaskBar.bottom-rectTaskBar.top), SWP_NOMOVE | SWP_NOSIZE  | SWP_FRAMECHANGED);
			SetFocus(g_hwnd);
			ShowCursor(TRUE);
		}
		else {
			SetWindowPos(hWndTaskBar, HWND_NOTOPMOST, 0, 0, (rectTaskBar.right-rectTaskBar.left), (rectTaskBar.bottom-rectTaskBar.top), SWP_NOMOVE | SWP_NOSIZE);
			SetWindowPos(g_hwnd, HWND_NOTOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
			SetWindowPos(g_hwnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
			ShowCursor(FALSE);
		}

	} // endif bFullscreen
	else {
		SetWindowLongPtrA(g_hwnd, GWL_STYLE, dwStyle); // restore original style

		// Restore the menu
		hMenu = LoadMenuA(hInstance, MAKEINTRESOURCEA(IDC_WINSPOUT));
		SetMenu(g_hwnd, hMenu);
		// Remove the relevant selection option
		HMENU hSubMenu;
		hSubMenu = GetSubMenu(hMenu, 0); // File
		if(bReceiver) { // receiver
			RemoveMenu(hSubMenu,  IDM_SENDERNAME, MF_BYCOMMAND);
		}
		else { // sender
			RemoveMenu(hSubMenu,  IDM_SPOUTSENDERS, MF_BYCOMMAND);
		}

		if(bTopmost) {
			hWndMode = HWND_TOPMOST;  // topmost was selected
			CheckMenuItem (hMenu, IDM_TOPMOST, MF_BYCOMMAND | MF_CHECKED);
		}
		else {
			hWndMode = HWND_TOP;
			CheckMenuItem (hMenu, IDM_TOPMOST, MF_BYCOMMAND | MF_UNCHECKED);
		}
		
		if(bReceiver) {
			if(bFitWindow) 
				CheckMenuItem (hMenu, IDM_FITWINDOW, MF_BYCOMMAND | MF_CHECKED);
			else
				CheckMenuItem (hMenu, IDM_FITWINDOW, MF_BYCOMMAND | MF_UNCHECKED);
		}

		// Restore our window
		// SetWindowPos(g_hwnd, hWndMode, windowRect.left, windowRect.top, g_Width+AddX, g_Height+AddY, SWP_SHOWWINDOW);
		SetWindowPos(g_hwnd, hWndMode, windowRect.left, windowRect.top, nonFullScreenX+AddX, nonFullScreenY+AddY, SWP_SHOWWINDOW);

		// Reset the window that was top before - could be ours
		if(GetWindowLong(hwndForeground2, GWL_EXSTYLE) & WS_EX_TOPMOST)
			SetWindowPos(hwndForeground2, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 
		else
	 		SetWindowPos(hwndForeground2, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		ShowCursor(TRUE);
		DrawMenuBar(g_hwnd);

	} // endif not bFullscreen

}


void SaveOpenGLstate()
{
	float dim[4];
	float vpScaleX, vpScaleY, vpWidth, vpHeight;
	int vpx, vpy;

	// save texture state, client state, etc.
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_TRANSFORM_BIT);

	// find the current viewport dimensions in order to scale to the aspect ratio required
	glGetFloatv(GL_VIEWPORT, dim);

	// Fit to window
	if(bFitWindow) {
		// Scale width and height to the current viewport size
		vpScaleX = dim[2]/(float)g_Width;
		vpScaleY = dim[3]/(float)g_Height;
		vpWidth  = (float)g_Width  * vpScaleX;
		vpHeight = (float)g_Height * vpScaleY;
		vpx = vpy = 0;
	}
	else {
		// Preserve aspect ratio of the sender
		// and fit to the width or the height
		vpWidth = dim[2];
		vpHeight = ((float)g_Height/(float)g_Width)*vpWidth;
		if(vpHeight > dim[3]) {
			vpHeight = dim[3];
			vpWidth = ((float)g_Width/(float)g_Height)*vpHeight;
		}
		vpx = (int)(dim[2]-vpWidth)/2;;
		vpy = (int)(dim[3]-vpHeight)/2;
	}

	glViewport((int)vpx, (int)vpy, (int)vpWidth, (int)vpHeight);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity(); // reset the current matrix back to its default state
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}


void RestoreOpenGLstate()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
		
	glPopAttrib();
		
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glPopClientAttrib();			
	glPopAttrib();

}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	char temp[1024];

	switch (message) {

		case WM_INITDIALOG:
			strcpy_s(temp, 256, WindowTitle);
			if(!bReceiver)
				strcat_s(temp,  1024, "\r\nfor sending frames to\n32bit or 64bit receivers\n\nhttp://spout.zeal.co");
			else
				strcat_s(temp,  1024, "\r\nfor receiving frames from\n32bit or 64bit senders\n\nhttp://spout.zeal.co");

			SetDlgItemTextA(hDlg, IDC_ABOUTBOXTEXT, (LPCSTR)temp);
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

// Message handler for caps dialog
LRESULT CALLBACK Capabilities(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam); // suppress warning

	switch (message) {
		
		case WM_INITDIALOG:
			SetDlgItemTextA(hDlg, IDC_CAPSTEXT, (LPCSTR)gldxcaps);
			return TRUE;

		case WM_COMMAND:

			switch(LOWORD(wParam)) {

				case IDOK :
				case IDCANCEL :
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;

				case IDC_COPY :

					if(OpenClipboard(NULL)) {
						HGLOBAL clipbuffer;
						char* buffer;
				        EmptyClipboard();
						clipbuffer = GlobalAlloc(GMEM_DDESHARE, strlen(gldxcaps)+1);
						buffer = (char*)GlobalLock(clipbuffer);
						strcpy_s(buffer, strlen(gldxcaps)+1, LPCSTR(gldxcaps));
						GlobalUnlock(clipbuffer);
				        SetClipboardData(CF_TEXT, clipbuffer);
						GlobalFree(clipbuffer);
						CloseClipboard();
						MessageBoxA(hDlg, "Diagnostics copied to the clipboard.", "Spout Demo", MB_OK);
					}
					else {
						MessageBoxA(hDlg, "Unknown clipboard open error.", "Spout Demo", MB_OK);
					}
					return TRUE;

				default:
					return FALSE;

			}
			break;
	}

	return FALSE;
}

// Message handler for sender name entry for a sender
LRESULT CALLBACK UserSenderName(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam); // suppress warning

	switch (message) {
		
		case WM_INITDIALOG:
            // Fill text entry box with passed string
            SetDlgItemTextA(hDlg, IDC_NAMETEXT, szText);
            //  Select all text in the edit field
            SendDlgItemMessage (hDlg, IDC_NAMETEXT, EM_SETSEL, 0, 0x7FFF0000L);
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK :
					// Get contents of edit field into static 256 char string
                    GetDlgItemTextA(hDlg, IDC_NAMETEXT, szText, 256);
                    EndDialog (hDlg, 1);
                    break;
				case IDCANCEL :
                    // User pressed cancel.  Just take down dialog box.
					EndDialog(hDlg, 0);
					return TRUE;
				default:
					return FALSE;
			}
			break;
	}

	return FALSE;
}


bool EnterSenderName(char *SenderName)
{
  int retvalue;
  unsigned char textin[256];

  strcpy_s(szText = (PSTR)textin, 256, SenderName); // move to a static string for the dialog

  // Show the dialog box 
  retvalue = DialogBoxA(hInstance, MAKEINTRESOURCEA(IDD_NAMEBOX), hWnd, (DLGPROC)UserSenderName);

  // OK - sender name has been entered
  if (retvalue != 0) {
		strcpy_s (SenderName, 256, szText); // Return the entered name
		return true;
  }

  // Cancel
  return false;

}


