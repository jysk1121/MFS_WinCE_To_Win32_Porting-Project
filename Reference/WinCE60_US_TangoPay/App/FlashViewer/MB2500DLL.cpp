#include "stdafx.h"
#include "MB2500DLL.h"

CMB2500DLLAPi::CMB2500DLLAPi()
{
	m_hMB2500DLL = NULL;
	memset(&m_MB2500DLL_FUNC, 0, sizeof(m_MB2500DLL_FUNC));

	m_hMB2500DLL = ::LoadLibrary(_T(".\\MB2500DLL.dll"));

	if (m_hMB2500DLL != NULL)
	{
		m_MB2500DLL_FUNC.pfnSIUInit		= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?SIUInit@@YA_NXZ"));
		RETAILMSG(1, (L"pfnSIUInit(0x%08X)\n", m_MB2500DLL_FUNC.pfnSIUInit));
		m_MB2500DLL_FUNC.pfnSIUDeInit = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?SIUDeInit@@YA_NXZ"));
		RETAILMSG(1, (L"pfnSIUDeInit(0x%08X)\n", m_MB2500DLL_FUNC.pfnSIUDeInit));
		m_MB2500DLL_FUNC.pfnIsSafeDoorOpen = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?IsSafeDoorOpen@@YA_NXZ"));
		RETAILMSG(1, (L"pfnIsSafeDoorOpen(0x%08X)\n", m_MB2500DLL_FUNC.pfnIsSafeDoorOpen));
		m_MB2500DLL_FUNC.pfnIsADASwitchOn = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?IsADASwitchOn@@YA_NXZ"));
		RETAILMSG(1, (L"pfnIsADASwitchOn(0x%08X)\n", m_MB2500DLL_FUNC.pfnIsADASwitchOn));
		m_MB2500DLL_FUNC.pfnIsOperatorSwitchOn = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?IsOperatorSwitchOn@@YA_NXZ"));
		RETAILMSG(1, (L"pfnIsOperatorSwitchOn(0x%08X)\n", m_MB2500DLL_FUNC.pfnIsOperatorSwitchOn));
		m_MB2500DLL_FUNC.pfnIsShutdownSwitchOn = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?IsShutdownSwitchOn@@YA_NXZ"));
		RETAILMSG(1, (L"pfnIsShutdownSwitchOn(0x%08X)\n", m_MB2500DLL_FUNC.pfnIsShutdownSwitchOn));
		m_MB2500DLL_FUNC.pfnClearOperatorSwitch = (VOID_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?ClearOperatorSwitch@@YAXXZ"));
		RETAILMSG(1, (L"pfnClearOperatorSwitch(0x%08X)\n", m_MB2500DLL_FUNC.pfnClearOperatorSwitch));
		m_MB2500DLL_FUNC.pfnClearShutdownSwitch = (VOID_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?ClearShutdownSwitch@@YAXXZ"));
		RETAILMSG(1, (L"pfnClearShutdownSwitch(0x%08X)\n", m_MB2500DLL_FUNC.pfnClearShutdownSwitch));
		m_MB2500DLL_FUNC.pfnSetGuideLight = (VOID_FUNCTION_BYTE_BYTE*)GetProcAddress(m_hMB2500DLL, _T("?SetGuideLight@@YAXEE@Z"));
		RETAILMSG(1, (L"pfnSetGuideLight(0x%08X)\n", m_MB2500DLL_FUNC.pfnSetGuideLight));
		m_MB2500DLL_FUNC.pfnIsNearSensorOn = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?IsNearSensorOn@@YA_NXZ"));
		RETAILMSG(1, (L"pfnIsNearSensorOn(0x%08X)\n", m_MB2500DLL_FUNC.pfnIsNearSensorOn));
		m_MB2500DLL_FUNC.pfnIsAntiSkimmingSensorOn = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?IsAntiSkimmingSensorOn@@YA_NXZ"));
		RETAILMSG(1, (L"pfnIsAntiSkimmingSensorOn(0x%08X)\n", m_MB2500DLL_FUNC.pfnIsAntiSkimmingSensorOn));
		
		// Function Key
		m_MB2500DLL_FUNC.pfnFDKInit = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?FDKInit@@YA_NXZ"));
		RETAILMSG(1, (L"pfnFDKInit(0x%08X)\n", m_MB2500DLL_FUNC.pfnFDKInit));
		m_MB2500DLL_FUNC.pfnFDKDeInit = (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?FDKDeInit@@YA_NXZ"));
		RETAILMSG(1, (L"pfnFDKDeInit(0x%08X)\n", m_MB2500DLL_FUNC.pfnFDKDeInit));
		m_MB2500DLL_FUNC.pfnGetFunctionKeyCode = (BYTE_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?GetFunctionKeyCode@@YAEXZ"));
		RETAILMSG(1, (L"pfnGetFunctionKeyCode(0x%08X)\n", m_MB2500DLL_FUNC.pfnGetFunctionKeyCode));
		m_MB2500DLL_FUNC.pfnClearFunctionKeyBuffer = (VOID_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?ClearFunctionKeyBuffer@@YAXXZ"));
		RETAILMSG(1, (L"pfnClearFunctionKeyBuffer(0x%08X)\n", m_MB2500DLL_FUNC.pfnClearFunctionKeyBuffer));
		
		// System NVRAM
		m_MB2500DLL_FUNC.pfnGetSystemAddr = (LPVOID_FUNCTION_INT_INT*)GetProcAddress(m_hMB2500DLL, _T("?GetSystemAddr@@YAPAXHH@Z"));
		RETAILMSG(1, (L"pfnGetSystemAddr(0x%08X)\n", m_MB2500DLL_FUNC.pfnGetSystemAddr));
		m_MB2500DLL_FUNC.pfnFreeSystemAddr = (BOOL_FUNCTION_LPVOID*)GetProcAddress(m_hMB2500DLL, _T("?FreeSystemAddr@@YA_NPAX@Z"));
		RETAILMSG(1, (L"pfnFreeSystemAddr(0x%08X)\n", m_MB2500DLL_FUNC.pfnFreeSystemAddr));
		
		// JNL NVRAM
		m_MB2500DLL_FUNC.pfnGetJNLAddr = (LPVOID_FUNCTION_INT*)GetProcAddress(m_hMB2500DLL, _T("?GetJNLAddr@@YAPAXH@Z"));
		RETAILMSG(1, (L"pfnGetJNLAddr(0x%08X)\n", m_MB2500DLL_FUNC.pfnGetJNLAddr));
		m_MB2500DLL_FUNC.pfnFreeJNLAddr = (BOOL_FUNCTION_LPVOID*)GetProcAddress(m_hMB2500DLL, _T("?FreeJNLAddr@@YA_NPAX@Z"));
		RETAILMSG(1, (L"pfnFreeJNLAddr(0x%08X)\n", m_MB2500DLL_FUNC.pfnFreeJNLAddr));
		
		// Log NVRAM
		m_MB2500DLL_FUNC.pfnWriteLogNVRAM = (INT_FUNCTION_UCHAR_INT_INT*)GetProcAddress(m_hMB2500DLL, _T("?WriteLogNVRAM@@YAHPAEHH@Z"));
		RETAILMSG(1, (L"pfnWriteLogNVRAM(0x%08X)\n", m_MB2500DLL_FUNC.pfnWriteLogNVRAM));
		m_MB2500DLL_FUNC.pfnGetLOGAddr = (LPVOID_FUNCTION_INT*)GetProcAddress(m_hMB2500DLL, _T("?GetLOGAddr@@YAPAXH@Z"));
		RETAILMSG(1, (L"pfnGetLOGAddr(0x%08X)\n", m_MB2500DLL_FUNC.pfnGetLOGAddr));
		m_MB2500DLL_FUNC.pfnFreeLOGAddr = (BOOL_FUNCTION_LPVOID*)GetProcAddress(m_hMB2500DLL, _T("?FreeLOGAddr@@YA_NPAX@Z"));
		RETAILMSG(1, (L"pfnFreeLOGAddr(0x%08X)\n", m_MB2500DLL_FUNC.pfnFreeLOGAddr));
		
		// Reserved NVRAM
		m_MB2500DLL_FUNC.pfnGetRsvdAddr = (LPVOID_FUNCTION_VOID*)GetProcAddress(m_hMB2500DLL, _T("?GetRsvdAddr@@YAPAXXZ"));
		RETAILMSG(1, (L"pfnGetRsvdAddr(0x%08X)\n", m_MB2500DLL_FUNC.pfnGetRsvdAddr));
		m_MB2500DLL_FUNC.pfnFreeRsvdAddr = (BOOL_FUNCTION_LPVOID*)GetProcAddress(m_hMB2500DLL, _T("?FreeRsvdAddr@@YA_NPAX@Z"));
		RETAILMSG(1, (L"pfnFreeRsvdAddr(0x%08X)\n", m_MB2500DLL_FUNC.pfnFreeRsvdAddr));
		
		// Write Synchronzation Function
		m_MB2500DLL_FUNC.pfnNvramCopyByte = (VOID_FUNCTION_LPBYTE_BYTE*)GetProcAddress(m_hMB2500DLL, _T("?NvramCopyByte@@YAXPAEE@Z"));
		RETAILMSG(1, (L"pfnNvramCopyByte(0x%08X)\n", m_MB2500DLL_FUNC.pfnNvramCopyByte));
		m_MB2500DLL_FUNC.pfnNvramCopyWord = (VOID_FUNCTION_LPWORD_WORD*)GetProcAddress(m_hMB2500DLL, _T("?NvramCopyWord@@YAXPAGG@Z"));
		RETAILMSG(1, (L"pfnNvramCopyWord(0x%08X)\n", m_MB2500DLL_FUNC.pfnNvramCopyWord));
		m_MB2500DLL_FUNC.pfnNvramCopyDword = (VOID_FUNCTION_LPDWORD_DWORD*)GetProcAddress(m_hMB2500DLL, _T("?NvramCopyDword@@YAXPAKK@Z"));
		RETAILMSG(1, (L"pfnNvramCopyDword(0x%08X)\n", m_MB2500DLL_FUNC.pfnNvramCopyDword));
		m_MB2500DLL_FUNC.pfnNvramCopyMemory = (VOID_FUNCTION_LPVOID_LPVOID_DWORD*)GetProcAddress(m_hMB2500DLL, _T("?NvramCopyMemory@@YAXPAX0K@Z"));
		RETAILMSG(1, (L"pfnNvramCopyMemory(0x%08X)\n", m_MB2500DLL_FUNC.pfnNvramCopyMemory));
		m_MB2500DLL_FUNC.pfnNvramSetMemory = (VOID_FUNCTION_LPVOID_INT_DWORD*)GetProcAddress(m_hMB2500DLL, _T("?NvramSetMemory@@YAXPAXHK@Z"));
		RETAILMSG(1, (L"pfnNvramSetMemory(0x%08X)\n", m_MB2500DLL_FUNC.pfnNvramSetMemory));
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
	RETAILMSG(1, (L"CALL SIUInit\n"));
	return (bool)(m_MB2500DLL_FUNC.pfnSIUInit());
}

bool CMB2500DLLAPi::SIUDeInit(void)
{
	RETAILMSG(1, (L"CALL SIUDeInit\n"));
	return (bool)(m_MB2500DLL_FUNC.pfnSIUDeInit());
}

bool CMB2500DLLAPi::IsSafeDoorOpen(void)
{
	RETAILMSG(1, (L"CALL IsSafeDoorOpen\n"));
	return (bool)(m_MB2500DLL_FUNC.pfnIsSafeDoorOpen());
}

bool CMB2500DLLAPi::IsADASwitchOn(void)
{
	RETAILMSG(1, (L"CALL IsADASwitchOn\n"));
	return (bool)(m_MB2500DLL_FUNC.pfnIsADASwitchOn());
}

bool CMB2500DLLAPi::IsOperatorSwitchOn(void)
{
	RETAILMSG(1, (L"CALL IsOperatorSwitchOn\n"));
	return (bool)(m_MB2500DLL_FUNC.pfnIsOperatorSwitchOn());
}

bool CMB2500DLLAPi::IsShutdownSwitchOn(void)
{
	RETAILMSG(1, (L"CALL IsShutdownSwitchOn\n"));
	return (bool)(m_MB2500DLL_FUNC.pfnIsShutdownSwitchOn());
}

void CMB2500DLLAPi::ClearOperatorSwitch(void)
{
	RETAILMSG(1, (L"CALL ClearOperatorSwitch\n"));
	m_MB2500DLL_FUNC.pfnClearOperatorSwitch();
}

void CMB2500DLLAPi::ClearShutdownSwitch(void)
{
	RETAILMSG(1, (L"CALL ClearShutdownSwitch\n"));
	m_MB2500DLL_FUNC.pfnClearShutdownSwitch();
}

void CMB2500DLLAPi::SetGuideLight(BYTE byFlicker, BYTE byOnOff)
{
	RETAILMSG(1, (L"CALL SetGuideLight\n"));
	m_MB2500DLL_FUNC.pfnSetGuideLight(byFlicker, byOnOff);
}

bool CMB2500DLLAPi::IsNearSensorOn(void)
{
	return (bool)(m_MB2500DLL_FUNC.pfnIsNearSensorOn());
}

bool CMB2500DLLAPi::IsAntiSkimmingSensorOn(void)
{
	return (bool)(m_MB2500DLL_FUNC.pfnIsAntiSkimmingSensorOn());
}

bool CMB2500DLLAPi::FDKInit(void)
{
	RETAILMSG(1, (L"CALL FDKInit\n"));
	return (bool)(m_MB2500DLL_FUNC.pfnFDKInit());
}

bool CMB2500DLLAPi::FDKDeInit(void)
{
	//RETAILMSG(1, (L"CALL FDKDeInit\n"));
	return (bool)(m_MB2500DLL_FUNC.pfnFDKDeInit());
}

BYTE CMB2500DLLAPi::GetFunctionKeyCode(void)
{
	//RETAILMSG(1, (L"CALL GetFunctionKeyCode\n"));
	return (BYTE)(m_MB2500DLL_FUNC.pfnGetFunctionKeyCode());
}

void CMB2500DLLAPi::ClearFunctionKeyBuffer(void)
{
	//RETAILMSG(1, (L"CALL ClearFunctionKeyBuffer\n"));
	m_MB2500DLL_FUNC.pfnClearFunctionKeyBuffer();
}

LPVOID CMB2500DLLAPi::GetSystemAddr(int index , int size )
{
	//RETAILMSG(1, (L"CALL GetSystemAddr\n"));
	return (LPVOID)(m_MB2500DLL_FUNC.pfnGetSystemAddr(index, size));
}

bool CMB2500DLLAPi::FreeSystemAddr(LPVOID	lpv_nvram)
{
	//RETAILMSG(1, (L"CALL FreeSystemAddr\n"));
	return (bool)(m_MB2500DLL_FUNC.pfnFreeSystemAddr(lpv_nvram));
}

LPVOID CMB2500DLLAPi::GetJNLAddr(int index)
{
	//RETAILMSG(1, (L"CALL GetJNLAddr\n"));
	return (LPVOID)(m_MB2500DLL_FUNC.pfnGetJNLAddr(index));
}

bool CMB2500DLLAPi::FreeJNLAddr(LPVOID	lpv_nvram)
{
	//RETAILMSG(1, (L"CALL FreeJNLAddr\n"));
	return (bool)(m_MB2500DLL_FUNC.pfnFreeJNLAddr(lpv_nvram));
}

int CMB2500DLLAPi::WriteLogNVRAM(unsigned char *ptrlogbuff, int size, int level)
{
	//RETAILMSG(1, (L"CALL WriteLogNVRAM\n"));
	return (int)(m_MB2500DLL_FUNC.pfnWriteLogNVRAM(ptrlogbuff, size, level));
}

LPVOID CMB2500DLLAPi::GetLOGAddr(int index)
{
	//RETAILMSG(1, (L"CALL GetLOGAddr\n"));
	return (LPVOID)(m_MB2500DLL_FUNC.pfnGetLOGAddr(index));
}

bool CMB2500DLLAPi::FreeLOGAddr(LPVOID	lpv_nvram)
{
	//RETAILMSG(1, (L"CALL FreeLOGAddr\n"));
	return (bool)(m_MB2500DLL_FUNC.pfnFreeLOGAddr(lpv_nvram));
}

LPVOID CMB2500DLLAPi::GetRsvdAddr(void)
{
	//RETAILMSG(1, (L"CALL GetRsvdAddr\n"));
	return (LPVOID)(m_MB2500DLL_FUNC.pfnGetRsvdAddr());
}

bool CMB2500DLLAPi::FreeRsvdAddr(LPVOID	lpv_nvram)
{
	//RETAILMSG(1, (L"CALL FreeRsvdAddr\n"));
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
	if (m_MB2500DLL_FUNC.pfnNvramCopyMemory == NULL)
		memcpy(pDest, pSrc, dwSize);
	else
		m_MB2500DLL_FUNC.pfnNvramCopyMemory(pDest, pSrc, dwSize);
}

void CMB2500DLLAPi::NvramSetMemory(LPVOID pDest, int c, DWORD dwSize)
{
	//RETAILMSG(1, (L"CALL NvramSetMemory\n"));
	if (m_MB2500DLL_FUNC.pfnNvramSetMemory == NULL)
		memset(pDest, c, dwSize);
	else
		m_MB2500DLL_FUNC.pfnNvramSetMemory(pDest, c, dwSize);
}
