#include "FFGLPluginInstance.h"
#include "FFDebugMessage.h"

class WinPluginInstance : public FFGLPluginInstance 
{

public:
		
	WinPluginInstance();
  
	DWORD Load(const char *filename);
	DWORD Unload();

	// LJ Extra Functions
	DWORD getInfo();
	DWORD getPluginCaps(DWORD index);
	DWORD getNumParameters();
	DWORD getParameterType(DWORD index);
	DWORD getParameterDefault(DWORD index);
	DWORD getParameterDisplay(DWORD index);
	DWORD getParameter(DWORD index);
	DWORD SetParameter(DWORD dwIndex, DWORD dwValue);
	DWORD getType();

	// Time support
	void SetTime(double curTime);


	virtual ~WinPluginInstance();

protected:
	
	HMODULE m_ffModule;

	// LJ Function pointer from PlugMain
	plugMainUnion m_ffPlugMainUnion;

};

