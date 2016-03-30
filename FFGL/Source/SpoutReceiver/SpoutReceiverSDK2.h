#ifndef SpoutReceiverSDK2_H
#define SpoutReceiverSDK2_H

#include "../FFGLPluginSDK.h"

#include "../../../SpoutSDK/SpoutReceiver.h"

class SpoutReceiverSDK2 : public CFreeFrameGLPlugin
{

public:

	SpoutReceiverSDK2();
	virtual ~SpoutReceiverSDK2();

	///////////////////////////////////////////////////
	// FreeFrame plugin methods
	///////////////////////////////////////////////////
	DWORD	SetParameter(const SetParameterStruct* pParam);		
	DWORD	GetParameter(DWORD dwIndex);
	DWORD	ProcessOpenGL(ProcessOpenGLStruct* pGL);
	DWORD   InitGL(const FFGLViewportStruct *vp);
	DWORD   DeInitGL();

	///////////////////////////////////////////////////
	// Factory method
	///////////////////////////////////////////////////
	static DWORD __stdcall CreateInstance(CFreeFrameGLPlugin **ppOutInstance)
	{
  		*ppOutInstance = new SpoutReceiverSDK2();
		if (*ppOutInstance != NULL)
			return FF_SUCCESS;
		return FF_FAIL;
	}

protected:

	// Parameters
	int m_initResources;
	GLint m_maxCoordsLocation;

	SpoutReceiver receiver;

	unsigned int g_Width, g_Height;
	unsigned int width, height;
	GLuint myTexture;

	char SenderName[256];
	char UserSenderName[256];
	char HostName[MAX_PATH];
	
	bool bInitialized;
	bool bAspect; // preserve aspect ratio of received texture in draw
	bool bUseActive; // connect to the active sender
	bool bStarted; // Allow one cylce for initialization for certain apps

	void InitTexture();
	void DrawReceivedTexture(GLuint TextureID, GLuint TextureTarget,  unsigned int width, unsigned int height);
	bool WritePathToRegistry(const char *filepath, const char *subkey, const char *valuename);

};

#endif
