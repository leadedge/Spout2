//
// SpoutSender.h
//
#pragma once

#ifndef SpoutSender_H
#define SpoutSender_H

#include "../FFGLPluginSDK.h"
#include "FFGLExtensions.h" // can't use these if using Glew 31.12.13
#include "FFGLLib.h"

#include "../../../SpoutSDK/SpoutSender.h"

class SpoutSenderSDK2 : public CFreeFrameGLPlugin
{

public:

	SpoutSenderSDK2();
	virtual ~SpoutSenderSDK2();

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
  		*ppOutInstance = new SpoutSenderSDK2();
		if (*ppOutInstance != NULL)
			return FF_SUCCESS;
		return FF_FAIL;
	}

protected:

	// Plugin parameters
	int m_initResources;
	FFGLExtensions m_extensions; // can't use these if using Glew 31.12.13
	GLint m_maxCoordsLocation;

	SpoutSender sender;

	unsigned int m_Width;
	unsigned int m_Height;
	char SenderName[256];
	char UserSenderName[256];
	bool bInitialized;
	void DrawFFGLtexture(GLuint TextureHandle, FFGLTexCoords maxCoords);

};

#endif
