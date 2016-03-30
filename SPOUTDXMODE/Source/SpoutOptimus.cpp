#include "SpoutOptimus.h"

//
//	SpoutOptimus.cpp
//
//	Set NVIDIA graphics as Global for all applications
//
//	Based on : http://onemanmmo.com/index.php?cmd=newsitem&comment=news.1.211.0
//
//	http://www.codegur.net/17270429/forcing-hardware-accelerated-rendering
//
//	Uses NVAPI to load the base profile and save it to a file to restore later.
//	The base profile is then modified and applied to the system. 
//	The application has to be re-started before the settings take effect,
//	but other apps started subsequently should use the modified base settings.
//

nVidia::nVidia() {
	hSession = 0;
	hProfile = 0;
	status = NVAPI_ERROR;
}

nVidia::~nVidia() { }

	// LJ DEBUG - dll detection does not work on Windows 10
	bool nVidia::IsOptimus()
	{
		// Check for Optimus - nvd3d9wrap.dll is loaded into all processes when Optimus is enabled.
		if(GetModuleHandleA("nvd3d9wrap.dll")) {
			printf("Optimus graphics\n");
			return true;
		}
		printf("Optimus graphics not found\n");
		return false;
	}

	//
	// 0 - high preformance
	// 1 - integrated
	// 2 - auto select
	//
	bool nVidia::ActivateNVIDIA(int index)
	{
		// printf("nVidia::ActivateNVIDIA()\n");
		// (0) Initialize NVAPI. This must be done first of all
		status = NvAPI_Initialize();
		if (status != NVAPI_OK) {
			// will fail silently if not supported
			printf("NvAPI_Initialize error\n");
			return false;
		}

		// (1) Create the session handle to access driver settings
		hSession = 0;
		status = NvAPI_DRS_CreateSession(&hSession);
		if (status != NVAPI_OK) {
			printf("NvAPI_DRS_CreateSession error\n");
			return false;
		}

		// (2) load all the system settings into the session
		status = NvAPI_DRS_LoadSettings(hSession);
		if (status != NVAPI_OK) {
			printf("NvAPI_DRS_LoadSettings error\n");
			return false;
		}

		// (3) Obtain the Base profile. Any setting needs to be inside
		// a profile, putting a setting on the Base Profile enforces it
		// for all the processes on the system
		hProfile = 0;
		status = NvAPI_DRS_GetBaseProfile(hSession, &hProfile);
		if (status != NVAPI_OK) {
			printf("NvAPI_DRS_GetBaseProfile error\n");
			return false;
		}


		// Now modify the settings to set NVIDIA global
		// TODO : documentation
		NVDRS_SETTING drsSetting1 = {0};
		drsSetting1.version = NVDRS_SETTING_VER;
		drsSetting1.settingId = SHIM_MCCOMPAT_ID;
		drsSetting1.settingType = NVDRS_DWORD_TYPE;

		NVDRS_SETTING drsSetting2 = {0};
		drsSetting2.version = NVDRS_SETTING_VER;
		drsSetting2.settingId = SHIM_RENDERING_MODE_ID;
		drsSetting2.settingType = NVDRS_DWORD_TYPE;

		NVDRS_SETTING drsSetting3 = {0};
		drsSetting3.version = NVDRS_SETTING_VER;
		drsSetting3.settingId = SHIM_RENDERING_OPTIONS_ID;
		drsSetting3.settingType = NVDRS_DWORD_TYPE;

		// Optimus flags for enabled applications
		if(index == 0)
			drsSetting1.u32CurrentValue = SHIM_MCCOMPAT_ENABLE;			// 0
		else if(index == 1)
			drsSetting1.u32CurrentValue = SHIM_MCCOMPAT_INTEGRATED;		// 1
		else
			drsSetting1.u32CurrentValue = SHIM_MCCOMPAT_AUTO_SELECT;	// 2

		// other options
		//		SHIM_MCCOMPAT_INTEGRATED		// 1
		//		SHIM_MCCOMPAT_USER_EDITABLE
		//		SHIM_MCCOMPAT_VARYING_BIT
		//		SHIM_MCCOMPAT_AUTO_SELECT		// 2
		
		// Enable application for Optimus
		// drsSetting2.u32CurrentValue = SHIM_RENDERING_MODE_ENABLE; // 0
		if(index == 0)
			drsSetting2.u32CurrentValue = SHIM_RENDERING_MODE_ENABLE;		// 0
		else if(index == 1)
			drsSetting2.u32CurrentValue = SHIM_RENDERING_MODE_INTEGRATED;	// 1
		else
			drsSetting2.u32CurrentValue = SHIM_RENDERING_MODE_ENABLE;		// 2

		// other options
		//		SHIM_RENDERING_MODE_INTEGRATED		// 1
		//		SHIM_RENDERING_MODE_USER_EDITABLE
		//		SHIM_RENDERING_MODE_VARYING_BIT
		//		SHIM_RENDERING_MODE_AUTO_SELECT		// 2
		//		SHIM_RENDERING_MODE_OVERRIDE_BIT
		//		SHIM_MCCOMPAT_OVERRIDE_BIT
		
		// Shim rendering modes per application for Optimus
		// drsSetting3.u32CurrentValue = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE; // 0
		if(index == 0)
			drsSetting3.u32CurrentValue = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE; // 0
		else if(index == 1)
			drsSetting3.u32CurrentValue = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE | SHIM_RENDERING_OPTIONS_IGPU_TRANSCODING;	// 1
		else
			drsSetting3.u32CurrentValue = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE;		// 2

		// other options
		//		SHIM_RENDERING_OPTIONS_DISABLE_ASYNC_PRESENT,
		//		SHIM_RENDERING_OPTIONS_EHSHELL_DETECT,
		//		SHIM_RENDERING_OPTIONS_FLASHPLAYER_HOST_DETECT,
		//		SHIM_RENDERING_OPTIONS_VIDEO_DRM_APP_DETECT,
		//		SHIM_RENDERING_OPTIONS_IGNORE_OVERRIDES,
		//		SHIM_RENDERING_OPTIONS_CHILDPROCESS_DETECT,
		//		SHIM_RENDERING_OPTIONS_ENABLE_DWM_ASYNC_PRESENT,
		//		SHIM_RENDERING_OPTIONS_PARENTPROCESS_DETECT,
		//		SHIM_RENDERING_OPTIONS_ALLOW_INHERITANCE,
		//		SHIM_RENDERING_OPTIONS_DISABLE_WRAPPERS,
		//		SHIM_RENDERING_OPTIONS_DISABLE_DXGI_WRAPPERS,
		//		SHIM_RENDERING_OPTIONS_PRUNE_UNSUPPORTED_FORMATS,
		//		SHIM_RENDERING_OPTIONS_ENABLE_ALPHA_FORMAT,
		//		SHIM_RENDERING_OPTIONS_IGPU_TRANSCODING,				// 1 ** include for force integrated
		//		SHIM_RENDERING_OPTIONS_DISABLE_CUDA,
		//		SHIM_RENDERING_OPTIONS_ALLOW_CP_CAPS_FOR_VIDEO,
		//		SHIM_RENDERING_OPTIONS_ENABLE_NEW_HOOKING,
		//		SHIM_RENDERING_OPTIONS_DISABLE_DURING_SECURE_BOOT,
		//		SHIM_RENDERING_OPTIONS_INVERT_FOR_QUADRO,
		//		SHIM_RENDERING_OPTIONS_INVERT_FOR_MSHYBRID,
		//		SHIM_RENDERING_OPTIONS_REGISTER_PROCESS_ENABLE_GOLD,


		// Code from "SOP" example
		//	if( ForceIntegrated ){
		//		drsSetting1.u32CurrentValue = SHIM_MCCOMPAT_INTEGRATED;
		//		drsSetting2.u32CurrentValue = SHIM_RENDERING_MODE_INTEGRATED;
		//		drsSetting3.u32CurrentValue = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE | SHIM_RENDERING_OPTIONS_IGPU_TRANSCODING;
		//	}else{
		//		drsSetting1.u32CurrentValue = SHIM_MCCOMPAT_ENABLE;
		//		drsSetting2.u32CurrentValue = SHIM_RENDERING_MODE_ENABLE;
		//		drsSetting3.u32CurrentValue = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE;
		//	}

		status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting1);
		if (status != NVAPI_OK) {
			printf("NvAPI_DRS_SetSetting 1 error\n");
			return false;
		}

		status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting2);
		if (status != NVAPI_OK) {
			printf("NvAPI_DRS_SetSetting 2 error\n");
			return false;
		}

		status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting3);
		if (status != NVAPI_OK) {
			printf("NvAPI_DRS_SetSetting 3 error\n");
			return false;
		}

		// (5) Now apply (or save) our changes to the system
		status = NvAPI_DRS_SaveSettings(hSession);
		if (status != NVAPI_OK) {
			char temp[256];
			sprintf_s(temp, 256, "NvAPI_DRS_SaveSettings error (%x) [%x]", status, hSession);
			printf("%s\n", temp);
			// MessageBoxA(NULL, temp, "nVidia", MB_OK);
			return false;
		}

		// (6) We clean up. This is analogous to doing a free()
		NvAPI_DRS_DestroySession(hSession);
		hSession = 0;
		NvAPI_Unload();

		// printf("NVAPI Settings applied OK\nClose and restart program\n");

		return true;

} // end ActivateGlobal (index)


//
// 0 - high performance
// 1 - integrated
// 2 - auto select
//
int nVidia::GetNVIDIA()
{
	int mode = 0;
	// printf("nVidia::GetNVIDIA()\n");

	// (0) Initialize NVAPI. This must be done first of all
	status = NvAPI_Initialize();
	if (status != NVAPI_OK) {
		// will fail silently if not supported
		printf("NvAPI_Initialize error\n");
		return 0;
	}

	// (1) Create the session handle to access driver settings
	hSession = 0;
	status = NvAPI_DRS_CreateSession(&hSession);
	if (status != NVAPI_OK) {
		printf("NvAPI_DRS_CreateSession error\n");
		return 0;
	}

	// (2) load all the system settings into the session
	status = NvAPI_DRS_LoadSettings(hSession);
	if (status != NVAPI_OK) {
		printf("NvAPI_DRS_LoadSettings error\n");
		return 0;
	}

	// (3) Obtain the Base profile. Any setting needs to be inside
	// a profile, putting a setting on the Base Profile enforces it
	// for all the processes on the system
	hProfile = 0;
	status = NvAPI_DRS_GetBaseProfile(hSession, &hProfile);
	if (status != NVAPI_OK) {
		printf("NvAPI_DRS_GetBaseProfile error\n");
		return 0;
	}

	// Now get the settings
	NVDRS_SETTING drsSetting1 = {0};
	drsSetting1.version = NVDRS_SETTING_VER;
	drsSetting1.settingId = SHIM_MCCOMPAT_ID;
	drsSetting1.settingType = NVDRS_DWORD_TYPE;

	NVDRS_SETTING drsSetting2 = {0};
	drsSetting2.version = NVDRS_SETTING_VER;
	drsSetting2.settingId = SHIM_RENDERING_MODE_ID;
	drsSetting2.settingType = NVDRS_DWORD_TYPE;

	NVDRS_SETTING drsSetting3 = {0};
	drsSetting3.version = NVDRS_SETTING_VER;
	drsSetting3.settingId = SHIM_RENDERING_OPTIONS_ID;
	drsSetting3.settingType = NVDRS_DWORD_TYPE;

	//	 1,  1, 0
	//   0,  0, 8192
	//  16,  1, 0
	// Optimus flags for enabled applications
	// SHIM_MCCOMPAT_ENABLE			High performance (1)
	// SHIM_MCCOMPAT_INTEGRATED		Integrated		 (0)
	// SHIM_MCCOMPAT_AUTO_SELECT	Auto select		 (16)

	status = NvAPI_DRS_GetSetting(hSession, hProfile, SHIM_MCCOMPAT_ID, &drsSetting1);
	if(drsSetting1.u32CurrentValue == 1)
		mode = 0; // high performance
	else if(drsSetting1.u32CurrentValue == 0)
		mode = 1; // integrated
	else
		mode = 2; // Auto select

	// Enable application for Optimus
	// status = NvAPI_DRS_GetSetting(hSession, hProfile, SHIM_RENDERING_MODE_ID, &drsSetting2);
	// printf("drsSetting2.u32CurrentValue = %d\n", drsSetting2.u32CurrentValue);

	// Shim rendering modes per application for Optimus
	// status = NvAPI_DRS_GetSetting(hSession, hProfile, SHIM_RENDERING_OPTIONS_ID, &drsSetting3);
	// printf("drsSetting3.u32CurrentValue = %d\n", drsSetting3.u32CurrentValue);

	// (6) We clean up. This is analogous to doing a free()
	NvAPI_DRS_DestroySession(hSession);
	hSession = 0;
	NvAPI_Unload();

	return mode;

} // end GetNVIDIA

