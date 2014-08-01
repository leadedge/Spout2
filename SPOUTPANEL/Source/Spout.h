//
//					Spout.h
// 
#ifndef __ofxSpout__
#define __ofxSpout__

#include <windows.h>
#include <gl/GL.h>
#include <set>
#include <string>

namespace Spout {
	
	// exported functions
	extern "C" _declspec(dllexport)
	bool InitSender(char *name, unsigned int width, unsigned int height, bool& bTextureShare, bool bMemoryShare = false);

	extern "C" _declspec(dllexport)
	bool InitReceiver (char *name, unsigned int& width, unsigned int& height, bool& bTextureShare, bool bMemoryShare = false);

	extern "C" _declspec(dllexport)
	bool ReleaseSender();

	extern "C" _declspec(dllexport)
	bool ReleaseReceiver(); 

	extern "C" _declspec(dllexport)
	bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=false);

	extern "C" _declspec(dllexport)
	bool ReceiveTexture(char *name, GLuint TextureID, GLuint TextureTarget, unsigned int &width, unsigned int &height);

	extern "C" _declspec(dllexport)
	bool TextureShareCompatible();

	extern "C" _declspec(dllexport)
	bool SelectSenderDialog();

	extern "C" _declspec(dllexport)
	bool SelectSenderPanel();

	extern "C" _declspec(dllexport)
	bool GetSenderNames(std::set<std::string> *sendernames);

	extern "C" _declspec(dllexport)
	bool GetSenderInfo(char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle);

}

#endif
