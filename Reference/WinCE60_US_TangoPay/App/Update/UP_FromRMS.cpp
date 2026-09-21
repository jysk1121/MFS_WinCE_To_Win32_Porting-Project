#include "stdafx.h"
#include "UP_FromRMS.h"

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1

// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_FromRMS
 FUNCTION NAME: CUP_FromRMS()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUP_FromRMS::CUP_FromRMS()
{
	// 자신을 등록한다.
	m_strName = STATE_UP_FROM_RMS;
	CUpdateState::Register(m_strName, this);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_FromRMS
 FUNCTION NAME: ~CUP_FromRMS()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUP_FromRMS::~CUP_FromRMS()
{
}


/*-------------------------------------------------------------------
 CLASS    NAME: CUP_FromRMS
 FUNCTION NAME: Process()
 RETURN TYPE  : 이동 할 다음 STATE
 PARAMETER    : STATE간 공유하는 정보.
 DESCRIPTION  : STATE의 정의된 동작을 처리하는 진입접 이다.
-------------------------------------------------------------------*/
CString CUP_FromRMS::Process(CUpdateStateInfo &StateInfo)
{
	if (CheckFormRMS(StateInfo) == FALSE)
		return STATE_UP_REBOOT;
	
	return STATE_UP_VALIDATE;
}


BOOL CUP_FromRMS::CheckFormRMS(CUpdateStateInfo &StateInfo)
{
	int			i;
	CString		strTemp;

	StateInfo.m_eUpdateType = UT_NONE;

	//////////////////////////////////////////////////////////////////////////
	// Get Update Type

	for (i = 0; i < UT_MAX; i++)
	{
		strTemp = GetUpdateFileName((UpdateType)i, StateInfo.m_strRMSUpdateSrc);

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

	// [#11] NH KSK 2010.10.2
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
	// end of [#11]

	//////////////////////////////////////////////////////////////////////////
	// Copy Update Files.
	// [#11] NH KSK 2010.10.2 RMS update는 zip파일만 가능하므로 아래 logic은 허수로 판단되어 주석 처리함

// 	CString	strSrc, strDest;
// 
// 	if (StateInfo.m_eUpdateType == UT_MASTER_FILE || StateInfo.m_eUpdateType == UT_PATCH_FILE)
// 	{
// 		strTemp = GetUpdateFileName(StateInfo.m_eUpdateType, StateInfo.m_strUpdateRoot);
// 		strSrc.Format(L"%s\\UpdateInfo.dat", strTemp);
// 		strDest.Format(L"%s\\UpdateInfo.dat", StateInfo.m_strUpdateRoot);
// 		CopyFile(strSrc, strDest, FALSE);
// 		DeleteFile(strSrc);
// 
// 		strSrc.Format(L"%s\\DeleteProf.dat", strTemp);
// 		strDest.Format(L"%s\\DeleteProf.dat", StateInfo.m_strUpdateRoot);
// 		CopyFile(strSrc, strDest, FALSE);
// 		DeleteFile(strSrc);
// 	}

	return TRUE;
}
