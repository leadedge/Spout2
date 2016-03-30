#include <windows.h>
#include <sal.h>
#include "nvapi.h"
#include "NvApiDriverSettings.h"
#include "stdio.h"
#include "string"

class nVidia
{
    public:

        nVidia();
        ~nVidia();
		bool IsOptimus();
		bool ActivateNVIDIA(int iPreference);
		int GetNVIDIA();

	protected :

		NvDRSSessionHandle hSession;
		NvDRSProfileHandle hProfile;
		NvAPI_Status status;

};


