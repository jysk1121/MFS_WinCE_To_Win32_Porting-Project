#include "stdafx.h"
#include "UP_Reboot.h"

// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Reboot
 FUNCTION NAME: CUP_Reboot()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUP_Reboot::CUP_Reboot()
{
	// 자신을 등록한다.
	m_strName = STATE_UP_REBOOT;
	CUpdateState::Register(m_strName, this);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Reboot
 FUNCTION NAME: ~CUP_Reboot()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUP_Reboot::~CUP_Reboot()
{
}


/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Reboot
 FUNCTION NAME: Process()
 RETURN TYPE  : 이동 할 다음 STATE
 PARAMETER    : STATE간 공유하는 정보.
 DESCRIPTION  : STATE의 정의된 동작을 처리하는 진입접 이다.
-------------------------------------------------------------------*/
CString CUP_Reboot::Process(CUpdateStateInfo &StateInfo)
{
	CString strValue;
	CString	strInfo;
	CString strTime;

	// Get Update Title
	if (StateInfo.m_strUpdateTitle.GetLength() != 0)
		strInfo.Format(L"%s\n\n", StateInfo.m_strUpdateTitle);

	// Set Update Result
	if (StateInfo.m_bUpdateSuccessful == TRUE)
		strInfo += L"Update success !!";
	else
		strInfo += L"Update failed !!";

	//for (int i = 60; i > 0; i--)
	for (int i = 5; i > 0; i--)
	{
		// Set Update Time
		strTime.Format(L"\n\nAfter %02d seconds, Rebooting.", i);
		strValue = strInfo + strTime;
		
		PrepareScreen(SCR_INFO);
		ShowString(1, strValue);
		ShowScreen(SCR_INFO);
		WaitForMilliSecond(1000);
	}

	// Reboot System.
#define IOCTL_SYSTEM_RESET	1
#define IOCTL_MODEM_RESET	2

	HANDLE hReset = CreateFile(L"RST1:", GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, NULL, NULL);
//	if (hReset != NULL)
	if (hReset != INVALID_HANDLE_VALUE)		// [#2022] NH KSK 2011.02.22
	{
		DWORD dwBuffer = IOCTL_SYSTEM_RESET;
		DWORD dwWritten;
		WriteFile(hReset, &dwBuffer, 1, &dwWritten, NULL);
		CloseHandle(hReset);
	}
	// 이거 에러가 나면 어쩐당.. ㅠㅠ

	WaitForMilliSecond(10000);

	return L"";
}

