#include "WinPluginInstance.h"

FFGLPluginInstance *FFGLPluginInstance::New()
{
  return new WinPluginInstance();
}


WinPluginInstance::WinPluginInstance() : m_ffModule(NULL)
{

}

DWORD WinPluginInstance::Load(const char *fname)
{

	if (fname==NULL || fname[0]==0)
		return FF_FAIL;

	Unload();

	m_ffModule = LoadLibrary(fname);

	if (m_ffModule==NULL) {
		return FF_FAIL;
	}

	FF_Main_FuncPtr pFreeFrameMain = (FF_Main_FuncPtr)GetProcAddress(m_ffModule, "plugMain");

	if (pFreeFrameMain==NULL) {
		FreeLibrary(m_ffModule);
		m_ffModule=NULL;
		return FF_FAIL;
	}
  
	m_ffPluginMain = pFreeFrameMain;
	   
	DWORD rval = InitPluginLibrary();

	if (rval != FF_SUCCESS) {
	    return rval;
	}

	return FF_SUCCESS;
}

DWORD WinPluginInstance::Unload()
{

  if (m_ffInstanceID != INVALIDINSTANCE) {
    // we can't call DeInstantiate because we must guarantee an active OpenGL context
    //DeInstantiateGL();
    FFDebugMessage("Failed to call DeInstantiateGL() before calling Unload()");
    return FF_FAIL;
  }
  
  DeinitPluginLibrary();
  
  if (m_ffModule!=NULL) {
		FreeLibrary(m_ffModule);
		m_ffModule=NULL;
  }

  return FF_SUCCESS;
}

DWORD WinPluginInstance::getPluginCaps(DWORD index)
{
	m_ffPlugMainUnion = m_ffPluginMain(FF_GETPLUGINCAPS, index, m_ffInstanceID);	
	return (m_ffPlugMainUnion.ivalue);
}

//
// Return value:
// 32-bit pointer to PluginInfoStruct if successful, FF_FAIL otherwise. 
//
DWORD WinPluginInstance::getInfo()
{
	PluginInfoStruct *info;

	m_ffPlugMainUnion = m_ffPluginMain(FF_GETINFO, 0, m_ffInstanceID);	
	info = (PluginInfoStruct *)(m_ffPlugMainUnion.PISvalue);

	return (DWORD)info;

}

DWORD WinPluginInstance::getNumParameters()
{
	m_ffPlugMainUnion = m_ffPluginMain(FF_GETNUMPARAMETERS, 0, m_ffInstanceID);
	return (m_ffPlugMainUnion.ivalue);
}


DWORD WinPluginInstance::getType()
{
	PluginInfoStruct *info;
	m_ffPlugMainUnion = m_ffPluginMain(FF_GETINFO, 0, m_ffInstanceID);	
	info = (PluginInfoStruct *)(m_ffPlugMainUnion.PISvalue);
	return info->PluginType;
}


//
// ParameterType: 32-bit unsigned integer. Tells the host what kind of data the parameter is. Current meaningful values:
//
//		Value 	Type 	Description
//		0 	boolean 	0.0 defined as false and anything else defined as true - e.g. checkbox
//		1 	event 		Similar to boolean but for a momentary push button style trigger. 1.0 is set momentarily to denote a simple event - e.g. pushbutton / keystroke.
//		2 	red 		The 3 colors e.g. for a colorpicker
//		3 	green
//		4 	blue
//		5 	xpos 		For x, y video interaction e.g. cursor - these define a position within the processed video frames.
//		6 	ypos
//		10 	standard 	A standard parameter representing an unspecified float value
//		11 	alpha 		Transparency value esp. for use in a 3 colour colourpicker + alpha
//		100 text 		A null terminated text input type - Note: only this type has a different data type for the moment
//		Remarks:
//		The range of values permitted for all variables (except text) is strictly 0-1
//
DWORD WinPluginInstance::getParameterType(DWORD index)
{
	m_ffPlugMainUnion = m_ffPluginMain(FF_GETPARAMETERTYPE, index, m_ffInstanceID);
	return (m_ffPlugMainUnion.ivalue);
}

DWORD WinPluginInstance::getParameterDefault(DWORD index)
{
	m_ffPlugMainUnion = m_ffPluginMain(FF_GETPARAMETERDEFAULT, index, m_ffInstanceID);
	return (m_ffPlugMainUnion.ivalue);
}


DWORD WinPluginInstance::getParameterDisplay(DWORD index)
{
	m_ffPlugMainUnion = m_ffPluginMain(FF_GETPARAMETERDISPLAY, index, m_ffInstanceID);
	return (DWORD)(m_ffPlugMainUnion.svalue); // char pointer

}

DWORD WinPluginInstance::getParameter(DWORD index)
{
	m_ffPlugMainUnion = m_ffPluginMain(FF_GETPARAMETER, index, m_ffInstanceID);
	return (m_ffPlugMainUnion.ivalue);
}

DWORD WinPluginInstance::SetParameter(DWORD index, DWORD dwValue)
{
	
	// typedef struct SetParameterStructTag {
	//	    DWORD ParameterNumber;
	//	    DWORD NewParameterValue;
	// } SetParameterStruct;
	SetParameterStruct ParamStruct;
	ParamStruct.ParameterNumber = index;
	void* pValue = (void *)&dwValue;
	memcpy(&ParamStruct.NewParameterValue, pValue, 4);
	m_ffPlugMainUnion = m_ffPluginMain(FF_SETPARAMETER, (DWORD)&ParamStruct, m_ffInstanceID);

	return (m_ffPlugMainUnion.ivalue);
}

// Time support
void WinPluginInstance::SetTime(double curTime)
{
	m_ffPluginMain(FF_SETTIME, (DWORD)(&curTime), m_ffInstanceID);

}


WinPluginInstance::~WinPluginInstance()
{
  if (m_ffModule!=NULL) {
    FFDebugMessage("plugin deleted without calling Unload()");
  }
}
