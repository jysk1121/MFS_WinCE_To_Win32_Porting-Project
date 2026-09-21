#include "stdafx.h"
#include "MB2500DLL.h"

// In Windows, there is no GetProcAddressW, so we have to force the string
// constants to multi-byte.
#ifdef UNDER_CE
#  define _LT(x) _T(x)
#else
#  define _LT(x) x
#endif

CMB2500DLLAPi::CMB2500DLLAPi()
{
	m_hMB2500DLL = NULL;
	memset(&m_MB2500DLL_FUNC, 0, sizeof(m_MB2500DLL_FUNC));

//	RETAILMSG(1, (L"LoadLibrary MB2500DLL.dll\n"));
	m_hMB2500DLL = ::LoadLibrary(_T(".\\MB2500DLL.dll"));
	DWORD err = GetLastError();

	if (m_hMB2500DLL != NULL)
	{
		m_MB2500DLL_FUNC.pfnSIUInit		= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?SIUInit@@YA_NXZ"));
		if (m_MB2500DLL_FUNC.pfnSIUInit == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnSIUInit\n"));

		m_MB2500DLL_FUNC.pfnSIUDeInit = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?SIUDeInit@@YA_NXZ"));
		if (m_MB2500DLL_FUNC.pfnSIUDeInit == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnSIUDeInit\n"));

		m_MB2500DLL_FUNC.pfnIsSafeDoorOpen = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?IsSafeDoorOpen@@YA_NXZ"));
		if (m_MB2500DLL_FUNC.pfnIsSafeDoorOpen == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsSafeDoorOpen\n"));

		// [#GLDV-3005] US Kook 2022.04.13 support side car
		m_MB2500DLL_FUNC.pfnIsSideCarDoorOpen = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?IsSideCarDoorOpen@@YA_NXZ"));
		if (m_MB2500DLL_FUNC.pfnIsSideCarDoorOpen == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsSideCarDoorOpen\n"));
		// end of [#GLDV-3005]

		m_MB2500DLL_FUNC.pfnIsADASwitchOn = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?IsADASwitchOn@@YA_NXZ"));
		if (m_MB2500DLL_FUNC.pfnIsADASwitchOn == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsADASwitchOn\n"));

		m_MB2500DLL_FUNC.pfnIsOperatorSwitchOn = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?IsOperatorSwitchOn@@YA_NXZ"));
		if (m_MB2500DLL_FUNC.pfnIsOperatorSwitchOn == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsOperatorSwitchOn\n"));

		m_MB2500DLL_FUNC.pfnIsShutdownSwitchOn = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?IsShutdownSwitchOn@@YA_NXZ"));
		if (m_MB2500DLL_FUNC.pfnIsShutdownSwitchOn == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsShutdownSwitchOn\n"));

		m_MB2500DLL_FUNC.pfnClearOperatorSwitch = (VOID_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?ClearOperatorSwitch@@YAXXZ"));
		if (m_MB2500DLL_FUNC.pfnClearOperatorSwitch == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnClearOperatorSwitch\n"));

		m_MB2500DLL_FUNC.pfnClearShutdownSwitch = (VOID_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?ClearShutdownSwitch@@YAXXZ"));
		if (m_MB2500DLL_FUNC.pfnClearShutdownSwitch == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnClearShutdownSwitch\n"));

		m_MB2500DLL_FUNC.pfnSetGuideLight = (VOID_FUNCTION_BYTE_BYTE*)GetProcAddress(m_hMB2500DLL, _LT("?SetGuideLight@@YAXEE@Z"));
		if (m_MB2500DLL_FUNC.pfnSetGuideLight == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnSetGuideLight\n"));

		m_MB2500DLL_FUNC.pfnIsNearSensorOn = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?IsNearSensorOn@@YA_NXZ"));
		if (m_MB2500DLL_FUNC.pfnIsNearSensorOn == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsNearSensorOn\n"));

		// [#2205] US KSK 2013.06.28
		m_MB2500DLL_FUNC.pfnSetHaloLedControl = (VOID_FUNCTION_BYTE_BYTE_BYTE*)GetProcAddress(m_hMB2500DLL, _LT("?SetHaloGuideLight@@YAXEEE@Z"));
		if (m_MB2500DLL_FUNC.pfnSetHaloLedControl == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnSetHaloLedControl\n"));
		// end of [#2205]

		// Anti-Skimm
		m_MB2500DLL_FUNC.pfnAntiSkimmingInit = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?AntiSkimmingInit@@YA_NXZ"));
		if (m_MB2500DLL_FUNC.pfnAntiSkimmingInit == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnAntiSkimmingInit\n"));

		m_MB2500DLL_FUNC.pfnAntiSkimmingDeInit = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?AntiSkimmingDeInit@@YA_NXZ"));
		if (m_MB2500DLL_FUNC.pfnAntiSkimmingDeInit == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnAntiSkimmingDeInit\n"));

		m_MB2500DLL_FUNC.pfnIsAntiSkimmingSensorOn = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?IsAntiSkimmingSensorOn@@YA_NXZ"));
		if (m_MB2500DLL_FUNC.pfnIsAntiSkimmingSensorOn == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsAntiSkimmingSensorOn\n"));

		m_MB2500DLL_FUNC.pfnSetAntiSkimmingDetectTime = (VOID_FUNCTION_DWORD*)GetProcAddress(m_hMB2500DLL, _LT("?SetAntiSkimmingDetectTime@@YAXK@Z"));
		if (m_MB2500DLL_FUNC.pfnSetAntiSkimmingDetectTime == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnSetAntiSkimmingDetectTime\n"));
		
		// Function Key
		m_MB2500DLL_FUNC.pfnFDKInit = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?FDKInit@@YA_NXZ"));
		if (m_MB2500DLL_FUNC.pfnFDKInit == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnFDKInit\n"));

		m_MB2500DLL_FUNC.pfnFDKDeInit = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?FDKDeInit@@YA_NXZ"));
		if (m_MB2500DLL_FUNC.pfnFDKDeInit == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnFDKDeInit\n"));

		m_MB2500DLL_FUNC.pfnGetFunctionKeyCode = (BYTE_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?GetFunctionKeyCode@@YAEXZ"));
		if (m_MB2500DLL_FUNC.pfnGetFunctionKeyCode == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnGetFunctionKeyCode\n"));

		m_MB2500DLL_FUNC.pfnClearFunctionKeyBuffer = (VOID_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?ClearFunctionKeyBuffer@@YAXXZ"));
		if (m_MB2500DLL_FUNC.pfnClearFunctionKeyBuffer == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnClearFunctionKeyBuffer\n"));
		
		// System NVRAM
		m_MB2500DLL_FUNC.pfnGetSystemAddr = (LPVOID_FUNCTION_INT_INT*)GetProcAddress(m_hMB2500DLL, _LT("?GetSystemAddr@@YAPAXHH@Z"));
		if (m_MB2500DLL_FUNC.pfnGetSystemAddr == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnGetSystemAddr\n"));

		m_MB2500DLL_FUNC.pfnFreeSystemAddr = (BOOL_FUNCTION_LPVOID*)GetProcAddress(m_hMB2500DLL, _LT("?FreeSystemAddr@@YA_NPAX@Z"));
		if (m_MB2500DLL_FUNC.pfnFreeSystemAddr == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnFreeSystemAddr\n"));
		
		// JNL NVRAM
		m_MB2500DLL_FUNC.pfnGetJNLAddr = (LPVOID_FUNCTION_INT*)GetProcAddress(m_hMB2500DLL, _LT("?GetJNLAddr@@YAPAXH@Z"));
		if (m_MB2500DLL_FUNC.pfnGetJNLAddr == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnGetJNLAddr\n"));

		m_MB2500DLL_FUNC.pfnFreeJNLAddr = (BOOL_FUNCTION_LPVOID*)GetProcAddress(m_hMB2500DLL, _LT("?FreeJNLAddr@@YA_NPAX@Z"));
		if (m_MB2500DLL_FUNC.pfnFreeJNLAddr == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnFreeJNLAddr\n"));
		
		// Log NVRAM
		m_MB2500DLL_FUNC.pfnWriteLogNVRAM = (INT_FUNCTION_UCHAR_INT_INT*)GetProcAddress(m_hMB2500DLL, _LT("?WriteLogNVRAM@@YAHPAEHH@Z"));
		if (m_MB2500DLL_FUNC.pfnWriteLogNVRAM == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnWriteLogNVRAM\n"));

		m_MB2500DLL_FUNC.pfnGetLOGAddr = (LPVOID_FUNCTION_INT*)GetProcAddress(m_hMB2500DLL, _LT("?GetLOGAddr@@YAPAXH@Z"));
		if (m_MB2500DLL_FUNC.pfnGetLOGAddr == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnGetLOGAddr\n"));

		m_MB2500DLL_FUNC.pfnFreeLOGAddr = (BOOL_FUNCTION_LPVOID*)GetProcAddress(m_hMB2500DLL, _LT("?FreeLOGAddr@@YA_NPAX@Z"));
		if (m_MB2500DLL_FUNC.pfnFreeLOGAddr == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnFreeLOGAddr\n"));
		
		// Reserved NVRAM
		m_MB2500DLL_FUNC.pfnGetRsvdAddr = (LPVOID_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _LT("?GetRsvdAddr@@YAPAXXZ"));
		if (m_MB2500DLL_FUNC.pfnGetRsvdAddr == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnGetRsvdAddr\n"));

		m_MB2500DLL_FUNC.pfnFreeRsvdAddr = (BOOL_FUNCTION_LPVOID*)GetProcAddress(m_hMB2500DLL, _LT("?FreeRsvdAddr@@YA_NPAX@Z"));
		if (m_MB2500DLL_FUNC.pfnFreeRsvdAddr == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnFreeRsvdAddr\n"));
		
		// Write Synchronzation Function
		m_MB2500DLL_FUNC.pfnNvramCopyByte = (VOID_FUNCTION_LPBYTE_BYTE*)GetProcAddress(m_hMB2500DLL, _LT("?NvramCopyByte@@YAXPAEE@Z"));
		if (m_MB2500DLL_FUNC.pfnNvramCopyByte == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnNvramCopyByte\n"));

		m_MB2500DLL_FUNC.pfnNvramCopyWord = (VOID_FUNCTION_LPWORD_WORD*)GetProcAddress(m_hMB2500DLL, _LT("?NvramCopyWord@@YAXPAGG@Z"));
		if (m_MB2500DLL_FUNC.pfnNvramCopyWord == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnNvramCopyWord\n"));

		// [#2270] AU KSK 2014.05.30
		m_MB2500DLL_FUNC.pfnNvramCopyShort = (VOID_FUNCTION_LPSHORT_SHORT*)GetProcAddress(m_hMB2500DLL, _LT("?NvramCopyShort@@YAXPAFF@Z"));
		if (m_MB2500DLL_FUNC.pfnNvramCopyShort == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnNvramCopyShort\n"));
		// end of [#2270]

		m_MB2500DLL_FUNC.pfnNvramCopyDword = (VOID_FUNCTION_LPDWORD_DWORD*)GetProcAddress(m_hMB2500DLL, _LT("?NvramCopyDword@@YAXPAKK@Z"));
		if (m_MB2500DLL_FUNC.pfnNvramCopyDword == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnNvramCopyDword\n"));

		m_MB2500DLL_FUNC.pfnNvramCopyMemory = (VOID_FUNCTION_LPVOID_LPVOID_DWORD*)GetProcAddress(m_hMB2500DLL, _LT("?NvramCopyMemory@@YAXPAX0K@Z"));
		if (m_MB2500DLL_FUNC.pfnNvramCopyMemory == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnNvramCopyMemory\n"));

		m_MB2500DLL_FUNC.pfnNvramSetMemory = (VOID_FUNCTION_LPVOID_INT_DWORD*)GetProcAddress(m_hMB2500DLL, _LT("?NvramSetMemory@@YAXPAXHK@Z"));
		if (m_MB2500DLL_FUNC.pfnNvramSetMemory == NULL)
			RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnNvramSetMemory\n"));
	}
	else
	{
		::MessageBox(NULL, _T("ERROR"), _T("CAN'T FIND MB2500DLL.DLL"), MB_OK);
	}
}

CMB2500DLLAPi::~CMB2500DLLAPi()
{
	if (m_hMB2500DLL != NULL)
	{
		::FreeLibrary(m_hMB2500DLL);
		m_hMB2500DLL = NULL;
		memset(&m_MB2500DLL_FUNC, 0, sizeof(m_MB2500DLL_FUNC));
	}
}

bool CMB2500DLLAPi::SIUInit(void)
{
	if (m_MB2500DLL_FUNC.pfnSIUInit == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnSIUInit\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnSIUInit());
}

bool CMB2500DLLAPi::SIUDeInit(void)
{
	if (m_MB2500DLL_FUNC.pfnSIUDeInit == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnSIUDeInit\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnSIUDeInit());
}

bool CMB2500DLLAPi::IsSafeDoorOpen(void)
{
	if (m_MB2500DLL_FUNC.pfnIsSafeDoorOpen == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsSafeDoorOpen\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnIsSafeDoorOpen());
}

// [#GLDV-3005] US Kook 2022.04.13 support side car
bool CMB2500DLLAPi::IsSideCarDoorOpen(void)
{
	if (m_MB2500DLL_FUNC.pfnIsSideCarDoorOpen == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsSideCarDoorOpen\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnIsSideCarDoorOpen());
}
// end of [#GLDV-3005]

bool CMB2500DLLAPi::IsADASwitchOn(void)
{
	if (m_MB2500DLL_FUNC.pfnIsADASwitchOn == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsADASwitchOn\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnIsADASwitchOn());
}

bool CMB2500DLLAPi::IsOperatorSwitchOn(void)
{
	if (m_MB2500DLL_FUNC.pfnIsOperatorSwitchOn == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsOperatorSwitchOn\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnIsOperatorSwitchOn());
}

bool CMB2500DLLAPi::IsShutdownSwitchOn(void)
{
	if (m_MB2500DLL_FUNC.pfnIsShutdownSwitchOn == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsShutdownSwitchOn\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnIsShutdownSwitchOn());
}

void CMB2500DLLAPi::ClearOperatorSwitch(void)
{
	if (m_MB2500DLL_FUNC.pfnClearOperatorSwitch == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnClearOperatorSwitch\n"));
		return;
	}
	m_MB2500DLL_FUNC.pfnClearOperatorSwitch();
}

void CMB2500DLLAPi::ClearShutdownSwitch(void)
{
	if (m_MB2500DLL_FUNC.pfnClearShutdownSwitch == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnClearShutdownSwitch\n"));
		return;
	}
	m_MB2500DLL_FUNC.pfnClearShutdownSwitch();
}

void CMB2500DLLAPi::SetGuideLight(BYTE byFlicker, BYTE byOnOff)
{
	if (m_MB2500DLL_FUNC.pfnSetGuideLight == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnSetGuideLight\n"));
		return;
	}
	m_MB2500DLL_FUNC.pfnSetGuideLight(byFlicker, byOnOff);
}

// [#2205] US KSK 2013.06.28
void CMB2500DLLAPi::SetHaloLedControl(BYTE byLedColor, BYTE byLedMode, BYTE byMcuControl)
{
	if (m_MB2500DLL_FUNC.pfnSetHaloLedControl == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnSetHaloLedControl\n"));
		return;
	}
	m_MB2500DLL_FUNC.pfnSetHaloLedControl(byLedColor, byLedMode, byMcuControl);
}
// end of [#2205]

bool CMB2500DLLAPi::IsNearSensorOn(void)
{
	if (m_MB2500DLL_FUNC.pfnIsNearSensorOn == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsNearSensorOn\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnIsNearSensorOn());
}

bool CMB2500DLLAPi::AntiSkimmingInit(void)
{
	if (m_MB2500DLL_FUNC.pfnAntiSkimmingInit == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnAntiSkimmingInit\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnAntiSkimmingInit());
}

bool CMB2500DLLAPi::AntiSkimmingDeInit(void)
{
	if (m_MB2500DLL_FUNC.pfnAntiSkimmingDeInit == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnAntiSkimmingDeInit\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnAntiSkimmingDeInit());
}

bool CMB2500DLLAPi::IsAntiSkimmingSensorOn(void)
{
	if (m_MB2500DLL_FUNC.pfnIsAntiSkimmingSensorOn == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnIsAntiSkimmingSensorOn\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnIsAntiSkimmingSensorOn());
}

void CMB2500DLLAPi::SetAntiSkimmingDetectTime(DWORD dwMillisec)
{
	if (m_MB2500DLL_FUNC.pfnSetAntiSkimmingDetectTime == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnSetAntiSkimmingDetectTime\n"));
		return;
	}
	m_MB2500DLL_FUNC.pfnSetAntiSkimmingDetectTime(dwMillisec);
}

bool CMB2500DLLAPi::FDKInit(void)
{
	if (m_MB2500DLL_FUNC.pfnFDKInit == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnFDKInit\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnFDKInit());
}

bool CMB2500DLLAPi::FDKDeInit(void)
{
	if (m_MB2500DLL_FUNC.pfnFDKDeInit == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnFDKDeInit\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnFDKDeInit());
}

BYTE CMB2500DLLAPi::GetFunctionKeyCode(void)
{
	if (m_MB2500DLL_FUNC.pfnGetFunctionKeyCode == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnGetFunctionKeyCode\n"));
		return 0;
	}
	return (BYTE)(m_MB2500DLL_FUNC.pfnGetFunctionKeyCode());
}

void CMB2500DLLAPi::ClearFunctionKeyBuffer(void)
{
	if (m_MB2500DLL_FUNC.pfnClearFunctionKeyBuffer == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnClearFunctionKeyBuffer\n"));
		return;
	}
	m_MB2500DLL_FUNC.pfnClearFunctionKeyBuffer();
}

LPVOID CMB2500DLLAPi::GetSystemAddr(int index , int size )
{
	RETAILMSG(1, (L"CALL GetSystemAddr (%d) (%d)\n", index, size));

	if (m_MB2500DLL_FUNC.pfnGetSystemAddr == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnGetSystemAddr\n"));
		return 0;
	}
	return (LPVOID)(m_MB2500DLL_FUNC.pfnGetSystemAddr(index, size));
}

bool CMB2500DLLAPi::FreeSystemAddr(LPVOID	lpv_nvram)
{
	RETAILMSG(1, (L"CALL FreeSystemAddr\n"));
	if (m_MB2500DLL_FUNC.pfnFreeSystemAddr == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnFreeSystemAddr\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnFreeSystemAddr(lpv_nvram));
}

LPVOID CMB2500DLLAPi::GetJNLAddr(int index)
{
	RETAILMSG(1, (L"CALL GetJNLAddr\n"));
	if (m_MB2500DLL_FUNC.pfnGetJNLAddr == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnGetJNLAddr\n"));
		return 0;
	}
	return (LPVOID)(m_MB2500DLL_FUNC.pfnGetJNLAddr(index));
}

bool CMB2500DLLAPi::FreeJNLAddr(LPVOID	lpv_nvram)
{
	RETAILMSG(1, (L"CALL FreeJNLAddr\n"));
	if (m_MB2500DLL_FUNC.pfnFreeJNLAddr == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnFreeJNLAddr\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnFreeJNLAddr(lpv_nvram));
}

int CMB2500DLLAPi::WriteLogNVRAM(unsigned char *ptrlogbuff, int size, int level)
{
	if (m_MB2500DLL_FUNC.pfnWriteLogNVRAM == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnWriteLogNVRAM\n"));
		return 0;
	}
	return (int)(m_MB2500DLL_FUNC.pfnWriteLogNVRAM(ptrlogbuff, size, level));
}

LPVOID CMB2500DLLAPi::GetLOGAddr(int index)
{
	RETAILMSG(1, (L"CALL GetLOGAddr\n"));
	if (m_MB2500DLL_FUNC.pfnGetLOGAddr == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnGetLOGAddr\n"));
		return 0;
	}
	return (LPVOID)(m_MB2500DLL_FUNC.pfnGetLOGAddr(index));
}

bool CMB2500DLLAPi::FreeLOGAddr(LPVOID	lpv_nvram)
{
	RETAILMSG(1, (L"CALL FreeLOGAddr\n"));
	if (m_MB2500DLL_FUNC.pfnFreeLOGAddr == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnFreeLOGAddr\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnFreeLOGAddr(lpv_nvram));
}

LPVOID CMB2500DLLAPi::GetRsvdAddr(void)
{
	RETAILMSG(1, (L"CALL GetRsvdAddr\n"));
	if (m_MB2500DLL_FUNC.pfnGetRsvdAddr == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnGetRsvdAddr\n"));
		return 0;
	}
	return (LPVOID)(m_MB2500DLL_FUNC.pfnGetRsvdAddr());
}

bool CMB2500DLLAPi::FreeRsvdAddr(LPVOID	lpv_nvram)
{
	RETAILMSG(1, (L"CALL FreeRsvdAddr\n"));
	if (m_MB2500DLL_FUNC.pfnFreeRsvdAddr == NULL)
	{
		RETAILMSG(1, (L"Load Function Error m_MB2500DLL_FUNC.pfnFreeRsvdAddr\n"));
		return false;
	}
	return (bool)(m_MB2500DLL_FUNC.pfnFreeRsvdAddr(lpv_nvram));
}

void CMB2500DLLAPi::NvramCopyByte(LPBYTE pbDest, BYTE bValue)
{
	//RETAILMSG(1, (L"CALL NvramCopyByte\n"));
	if (m_MB2500DLL_FUNC.pfnNvramCopyByte == NULL)
		memcpy((LPVOID)pbDest, (LPVOID)&bValue, sizeof(BYTE));
	else
		m_MB2500DLL_FUNC.pfnNvramCopyByte(pbDest, bValue);
}

void CMB2500DLLAPi::NvramCopyWord(LPWORD pwDest, WORD wValue)
{
	//RETAILMSG(1, (L"CALL NvramCopyWord\n"));
	if (m_MB2500DLL_FUNC.pfnNvramCopyWord == NULL)
		memcpy((LPVOID)pwDest, (LPVOID)&wValue, sizeof(WORD));
	else
		m_MB2500DLL_FUNC.pfnNvramCopyWord(pwDest, wValue);
}

void CMB2500DLLAPi::NvramCopyDword(LPDWORD  pdwDest, DWORD dwValue)
{
	//RETAILMSG(1, (L"CALL NvramCopyDword\n"));
	if (m_MB2500DLL_FUNC.pfnNvramCopyDword == NULL)
		memcpy((LPVOID)pdwDest, (LPVOID)&pdwDest, sizeof(DWORD));
	else
		m_MB2500DLL_FUNC.pfnNvramCopyDword(pdwDest, dwValue);
}

void CMB2500DLLAPi::NvramCopyMemory(LPVOID pDest, LPVOID pSrc, DWORD dwSize)
{
	//RETAILMSG(1, (L"CALL NvramCopyMemory\n"));
	if (dwSize > 0)
	{
		if (m_MB2500DLL_FUNC.pfnNvramCopyMemory == NULL)
			memcpy(pDest, pSrc, dwSize);
		else
			m_MB2500DLL_FUNC.pfnNvramCopyMemory(pDest, pSrc, dwSize);
	}
}

void CMB2500DLLAPi::NvramSetMemory(LPVOID pDest, int c, DWORD dwSize)
{
	//RETAILMSG(1, (L"CALL NvramSetMemory\n"));
	if (pDest != NULL)			// US KMK 2013.07.01 코드소나 지적대응. NULL 예외처리 추가
	{
		if (m_MB2500DLL_FUNC.pfnNvramSetMemory == NULL)
			memset(pDest, c, dwSize);
		else
			m_MB2500DLL_FUNC.pfnNvramSetMemory(pDest, c, dwSize);
	}
}

// [#2270] AU KSK 2014.05.30
void CMB2500DLLAPi::NvramCopyShort(LPSHORT psDest, SHORT sValue)
{
	//RETAILMSG(1, (L"CALL NvramCopyShort\n"));
	if (m_MB2500DLL_FUNC.pfnNvramCopyShort == NULL)
		memcpy((LPVOID)psDest, (LPVOID)&sValue, sizeof(SHORT));
	else
		m_MB2500DLL_FUNC.pfnNvramCopyShort(psDest, sValue);
}
// end of [#2270]
