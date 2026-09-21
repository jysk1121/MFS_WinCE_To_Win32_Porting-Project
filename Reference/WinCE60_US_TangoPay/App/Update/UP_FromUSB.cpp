#include "stdafx.h"
#include "UP_FromUSB.h"

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1

// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_FromUSB
 FUNCTION NAME: CUP_FromUSB()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUP_FromUSB::CUP_FromUSB()
{
	// 자신을 등록한다.
	m_strName = STATE_UP_FROM_USB;
	CUpdateState::Register(m_strName, this);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_FromUSB
 FUNCTION NAME: ~CUP_FromUSB()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUP_FromUSB::~CUP_FromUSB()
{
}


/*-------------------------------------------------------------------
 CLASS    NAME: CUP_FromUSB
 FUNCTION NAME: Process()
 RETURN TYPE  : 이동 할 다음 STATE
 PARAMETER    : STATE간 공유하는 정보.
 DESCRIPTION  : UPDATE 파일을 Copy 한다.
-------------------------------------------------------------------*/
CString CUP_FromUSB::Process(CUpdateStateInfo &StateInfo)
{
	int i = 0;
	// Delete File in Update Temp Folder
	DeleteFileInDirectory(L"\\Update", L"*.*", FALSE, i, 0);

	//////////////////////////////////////////////////////////////////////////
	// check SD or USB slot

	USB_WAIT_RESULT res = WaitForSDorUSBPlugin(StateInfo.m_strSDUpdateSrc, StateInfo.m_strUSBUpdateSrc);
	if (res == UPDATE_TIMEOUT)
	{
		return STATE_UP_EMERGENCY_REMOTE;
	}
	else if (res == UPDATE_SD || res == UPDATE_USB)
	{
		if (CheckFormUSB(StateInfo,res == UPDATE_USB) == TRUE)
		{
			return STATE_UP_VALIDATE;
		}
	}

	return STATE_UP_REBOOT;
}

BOOL CUP_FromUSB::CheckFormUSB(CUpdateStateInfo &StateInfo, BOOL useUsb)
{
	int		i;
	CString	strTemp;
	CString	strUpdateSrc;

	StateInfo.m_eUpdateType = UT_NONE;
	if (useUsb == TRUE)
		strUpdateSrc = StateInfo.m_strUSBUpdateSrc;
	else
		strUpdateSrc = StateInfo.m_strSDUpdateSrc;

	//////////////////////////////////////////////////////////////////////////
	// Get Update Type

	for (i = 0; i < UT_MAX; i++)
	{
		strTemp = GetUpdateFileName((UpdateType)i, strUpdateSrc);

		if (IsExistFile(strTemp) == TRUE)
		{
			StateInfo.m_eUpdateType = (UpdateType)i;
			break;
		}
	}

	if (StateInfo.m_eUpdateType == UT_NONE)
	{
		return FALSE;
	}

	//////////////////////////////////////////////////////////////////////////
	// Copy Update Files.

	CString	strSrc, strDest;

	// create folder \\Update
	CreateDirectory(L"\\Update", NULL);
	CreateDirectory(StateInfo.m_strUpdateRoot, NULL);

	strSrc = GetUpdateFileName(StateInfo.m_eUpdateType, strUpdateSrc);
	strDest = GetUpdateFileName(StateInfo.m_eUpdateType, StateInfo.m_strUpdateRoot);

	PrepareScreen(SCR_FILECTRL);
	ShowString(1, L"Gathering update information");
	ShowString(2, L"");
	ShowString(3, L"");

	// [#2300] US KSK 2014.11.11
	NH_OS_VERSION	eOSVersion = m_SystemConfig.GetOSVersion();

	if (eOSVersion == NH_OS_MX5200SE				// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
		|| eOSVersion == NH_OS_MX2800SE)			// [#2518] US Kook 2018.01.20 Support MX-2800SE
		ShowString(6, L"Please wait until ATM reboot");
	else
		ShowString(5, L"Please wait until ATM reboot");
	// end of [#2300]

	ShowScreen(SCR_FILECTRL);
	WaitForMilliSecond(100);

	if (StateInfo.m_eUpdateType == UT_MASTER_ZIP || StateInfo.m_eUpdateType == UT_PATCH_ZIP)
	{
		NHDEBUG(DBG_INFO, (L" -- COPYING ZIP...\n"));
		ShowString(2, L"[ Staging... ]");
		ShowString(3, L"");
		UpdateScreen();
		WaitForMilliSecond(100);
		if (!IsExistFile(strDest))
		{
			if (CopyFile(strSrc, strDest, FALSE) == FALSE)
			{
				NHERROR((_T("[UPDATE] Failed to Copy [%s] -> [%s]\n"), strSrc, strDest));
				return FALSE;
			}
		}
		NHDEBUG(DBG_INFO, (L" -- COPYING ZIP... DONE\n"));
	}
	else
	{
		int	nCurFileCount = 0, nTotalFileCount = 0;
		
		CreateDirectory(strDest, NULL);

		nCurFileCount = nTotalFileCount = 0;
		CountFileInDirectory(strSrc, L"*.*", nTotalFileCount);
		if (CopyFileInDirectory(strSrc, L"*.*", strDest, TRUE, nCurFileCount, nTotalFileCount) == FALSE)
		{
			NHERROR((_T("[UPDATE] Failed to Copy Directory [%s] -> [%s]\n"), strSrc, strDest));
			return FALSE;
		}

		strTemp = strDest;
		strSrc.Format(L"%s\\UpdateInfo.dat", strTemp);
		strDest.Format(L"%s\\UpdateInfo.dat", StateInfo.m_strUpdateRoot);
		CopyFile(strSrc, strDest, FALSE);
		DeleteFile(strSrc);

		strSrc.Format(L"%s\\DeleteProf.dat", strTemp);
		strDest.Format(L"%s\\DeleteProf.dat", StateInfo.m_strUpdateRoot);
		CopyFile(strSrc, strDest, FALSE);
		DeleteFile(strSrc);
	}

	return TRUE;
}

USB_WAIT_RESULT CUP_FromUSB::WaitForSDorUSBPlugin(CString strSDPath, CString strUSBPath)
{
	int		i;
	CString	strKeyStr;

	PrepareScreen(SCR_YSENO);
	ShowButton(5, FALSE);
	ShowButton(6, FALSE);
	ShowString(1, L"Searching USB memory drive");
	ShowScreen(SCR_YSENO);

	while (1)
	{
		// Search USB
		ShowButton(5, FALSE);
		ShowButton(6, FALSE);
		ShowString(1, L"");
		UpdateScreen();

		WaitForMilliSecond(100);

		// wait for USB plug it in the USB slot
		for (i = 0; i < 10; i++)
		{
			ShowString(1, L"Searching SD drive");
			UpdateScreen();
			WaitForMilliSecond(100);

			if (IsExistFile(strSDPath) == TRUE)
			{
				return UPDATE_SD;
			}

			WaitForMilliSecond(1000);

			ShowString(1, L"Searching USB memory drive");
			UpdateScreen();
			WaitForMilliSecond(100);

			if (IsExistFile(strUSBPath) == TRUE)
			{
				return UPDATE_USB;
			}

			WaitForMilliSecond(1000);
		}

		ShowButton(5, TRUE);
		ShowButton(6, TRUE);
		ShowString(1, L"Cannot find the SD or USB drive.\n\n\nPlease re-plug it in the SD or USB slot\nbefore pressing the S/W Update button.\n\nAfter 60 seconds, the ATM will install AP software from the Hyosung software repository.");
		UpdateScreen();

		WaitForMilliSecond(100);

		strKeyStr = GetKeyString(60);
		if (strKeyStr == L"EXIT")
		{
			return UPDATE_EXIT;
		}
		else if (strKeyStr == L"TIMEOVER")
		{
			return UPDATE_TIMEOUT;
		}
	}

	return UPDATE_TIMEOUT;
}

