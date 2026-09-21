#include "stdafx.h"
#include ".\Common\CmnLib.h"

int WINAPI Delay_Msg(DWORD dwTimeout)
{
	DWORD	dwStart = GetTickCount();
	DWORD	dwElapsed;

	// 5 mSec 이하는 5 mSec로 설정.
	if (dwTimeout <= 5)		dwTimeout = 5;

	while ((dwElapsed = GetTickCount() - dwStart) < dwTimeout)
	{
		DWORD dwStatus = MsgWaitForMultipleObjectsEx(0, NULL,
													(dwTimeout - dwElapsed), QS_ALLINPUT | QS_ALLEVENTS,
													MWMO_INPUTAVAILABLE);
		if (dwStatus == WAIT_OBJECT_0)
		{
			MSG msg;

			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if(!AfxGetApp()->PreTranslateMessage(&msg))
                {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
			}
		}
	}
	return TRUE;
}

ULONG WINAPI TimerSet(int nSecond)
{
	return (GetTickCount() + (nSecond * 1000));
}

int WINAPI CheckTimer(ULONG nSetTime)
{
	if (GetTickCount() > nSetTime)
 		return TRUE;
	else 
		return FALSE;
}
