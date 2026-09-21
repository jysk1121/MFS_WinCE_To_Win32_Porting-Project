#include "stdafx.h"
#include ".\Common\CmnLib.h"
#include "MB2500DLL.h"


CMB2500DLLAPi	gMB2500DLLApi;

bool SIUInit(void)
{
	return gMB2500DLLApi.SIUInit();
}

bool SIUDeInit(void)
{
	return gMB2500DLLApi.SIUDeInit();
}

bool IsSafeDoorOpen(void)
{
	return gMB2500DLLApi.IsSafeDoorOpen();
}

// [#GLDV-3005] US Kook 2022.04.13 support side car
bool IsSideCarDoorOpen(void)
{
	return gMB2500DLLApi.IsSideCarDoorOpen();
}
// end of [#GLDV-3005]

bool IsADASwitchOn(void)
{
	return gMB2500DLLApi.IsADASwitchOn();
}

bool IsOperatorSwitchOn(void)
{
	return gMB2500DLLApi.IsOperatorSwitchOn();
}

bool IsShutdownSwitchOn(void)
{
	return gMB2500DLLApi.IsShutdownSwitchOn();
}

void ClearOperatorSwitch(void)
{
	gMB2500DLLApi.ClearOperatorSwitch();
}

void ClearShutdownSwitch(void)
{
	gMB2500DLLApi.ClearShutdownSwitch();
}

void SetGuideLight(BYTE byFlicker, BYTE byOnOff)
{
	gMB2500DLLApi.SetGuideLight(byFlicker, byOnOff);
}

// [#2205] US KSK 2013.06.28
void SetHaloLedControl(BYTE byLedColor, BYTE byLedMode, BYTE byMcuControl)
{
	gMB2500DLLApi.SetHaloLedControl(byLedColor, byLedMode, byMcuControl);
}
// end of [#2205]

bool IsNearSensorOn(void)
{
	return gMB2500DLLApi.IsNearSensorOn();
}

bool	AntiSkimmingInit(void)
{
	return gMB2500DLLApi.AntiSkimmingInit();
}

bool	AntiSkimmingDeInit(void)
{
	return gMB2500DLLApi.AntiSkimmingDeInit();
}

bool IsAntiSkimmingSensorOn(void)
{
	return gMB2500DLLApi.IsAntiSkimmingSensorOn();
}

void	SetAntiSkimmingDetectTime(DWORD dwMillisec)
{
	gMB2500DLLApi.SetAntiSkimmingDetectTime(dwMillisec);
}

bool FDKInit(void)
{
	return gMB2500DLLApi.FDKInit();
}

bool FDKDeInit(void)
{
	return gMB2500DLLApi.FDKDeInit();
}

BYTE GetFunctionKeyCode(void)
{
	return gMB2500DLLApi.GetFunctionKeyCode();
}

void ClearFunctionKeyBuffer(void)
{
	gMB2500DLLApi.ClearFunctionKeyBuffer();
}

LPVOID GetSystemAddr(int index , int size )
{
	return gMB2500DLLApi.GetSystemAddr(index, size);
}

bool FreeSystemAddr(LPVOID	lpv_nvram)
{
	return gMB2500DLLApi.FreeSystemAddr(lpv_nvram);
}

LPVOID GetJNLAddr(int index)
{
	return gMB2500DLLApi.GetJNLAddr(index);
}

bool FreeJNLAddr(LPVOID	lpv_nvram)
{
	return gMB2500DLLApi.FreeJNLAddr(lpv_nvram);
}

int	WriteLogNVRAM(unsigned char *ptrlogbuff, int size, int level)
{
	return gMB2500DLLApi.WriteLogNVRAM(ptrlogbuff, size, level);
}

LPVOID GetLOGAddr(int index)
{
	return gMB2500DLLApi.GetLOGAddr(index);
}

bool FreeLOGAddr(LPVOID	lpv_nvram)
{
	return gMB2500DLLApi.FreeLOGAddr(lpv_nvram);
}

LPVOID GetRsvdAddr(void)
{
	return gMB2500DLLApi.GetRsvdAddr();
}

bool FreeRsvdAddr(LPVOID lpv_nvram)
{
	return gMB2500DLLApi.FreeRsvdAddr(lpv_nvram);
}

void NvramCopyByte(LPBYTE pbDest, BYTE bValue)
{
	gMB2500DLLApi.NvramCopyByte(pbDest, bValue);
}

void NvramCopyWord(LPWORD pwDest, WORD wValue)
{
	gMB2500DLLApi.NvramCopyWord(pwDest, wValue);
}

void NvramCopyDword(LPDWORD  pdwDest, DWORD dwValue)
{
	gMB2500DLLApi.NvramCopyDword(pdwDest, dwValue);
}

void NvramCopyMemory(LPVOID pDest, LPVOID pSrc, DWORD dwSize)
{
	gMB2500DLLApi.NvramCopyMemory(pDest, pSrc, dwSize);
}

void NvramSetMemory(LPVOID pDest, int c, DWORD dwSize)
{
	gMB2500DLLApi.NvramSetMemory(pDest, c, dwSize);
}

// [#2270] AU KSK 2014.05.30
void NvramCopyShort(LPSHORT psDest, SHORT sValue)
{
	gMB2500DLLApi.NvramCopyShort(psDest, sValue);
}
// end of [#2270]
