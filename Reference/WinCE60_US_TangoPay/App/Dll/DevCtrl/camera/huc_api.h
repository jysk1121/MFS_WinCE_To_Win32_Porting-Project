
#ifndef _HUSB_API_H_
#define _HUSB_API_H_

#ifdef UNDER_CE

#include "mcscore.h"

#define HJAPI
HJAPI int MCSCORE_SetHucConfig(MCS_Config	* pMcsConfig);

HJAPI int MCSCORE_InitThreads(); 

HJAPI int MCSCORE_DeinitThreads();

HJAPI int StartPreviewPerson (long nlX, long nlY, long nlX_width, long nlY_height);  // nlX_width is mutiples of 8

HJAPI int StartPreviewExitSlot (long nlX, long nlY, long nlX_width, long nlY_height); // nlX_width is mutiples of 8

HJAPI int StopPreviewPerson ();

HJAPI int StopPreviewExitSlot ();

HJAPI int PersonStillCapture(LPCSTR pFIleName);

HJAPI int ExitSlotStillCapture(LPCSTR pFIleName);

HJAPI int GrayColorImageOutputP(ULONG ulValLo, ULONG ulVaHi);

HJAPI int GrayColorImageOutputE(ULONG ulValLo, ULONG ulVaHi);

HJAPI int InsertTextAndColorP(int nColor, long x, long y, TCHAR* szText);

HJAPI int InsertTextAndColorE(int nColor, long x, long y, TCHAR* szText);



/*
[Parameters]
CamNum = 1 : Main Camera
CamNum = 2 : Sub Camera

[Return]
1 : OK
0 : Error
*/
HJAPI int GetCameraStatus(int CamNum);

HJAPI int NhUsbCamCE_Initialize();

HJAPI int NhUsbCamCE_Release();

// 2018-02-01 추가된 새로운 함수들
/*
[Parameters]
CamNum = 1 : Main Camera
CamNum = 2 : Sub Camera

[Return]
1 : OK
0 : Error
*/
HJAPI int CamStartPerson();
HJAPI int CamStopPerson();
HJAPI int CamStartExitSlot();
HJAPI int CamStopExitSlot();

#endif

#endif // UNDER_CE