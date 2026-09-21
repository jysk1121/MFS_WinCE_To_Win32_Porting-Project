#include "stdafx.h"
#include "UP_Init.h"
#include "resource.h"
#include "UPFunctionKey.h"
#include "pwindbas.h"
#include ".\Common\CmnLib.h"

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1

// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------
CUPFunctionKey	g_FunctionKey;

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Init
 FUNCTION NAME: CUP_Init()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUP_Init::CUP_Init()
{
	// 자신을 등록한다.
	m_strName = STATE_UP_INIT;
	CUpdateState::Register(m_strName, this);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Init
 FUNCTION NAME: ~CUP_Init()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUP_Init::~CUP_Init()
{
}


/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Init
 FUNCTION NAME: Process()
 RETURN TYPE  : 이동 할 다음 STATE
 PARAMETER    : STATE간 공유하는 정보.
 DESCRIPTION  : STATE의 정의된 동작을 처리하는 진입접 이다.
-------------------------------------------------------------------*/
CString CUP_Init::Process(CUpdateStateInfo &StateInfo)
{
	//////////////////////////////////////////////////////////////////////////
	//	LOAD SCREEN

	LoadScreen();

	g_FunctionKey.Initialize(m_pScrCtrl);
	g_FunctionKey.FunctionKeyCheck(TRUE);


	NH_OS_VERSION	eOSVersion = m_SystemConfig.GetOSVersion();

	//////////////////////////////////////////////////////////////////////////
	//	SHOW INIT SCREEN.
	PrepareScreen(SCR_INFO);
	ShowString(1, L"Gathering update information.");

	// [#2300] US KSK 2014.11.11
	if (eOSVersion == NH_OS_MX5200SE				// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
		|| eOSVersion == NH_OS_MX2800SE)			// [#2518] US Kook 2018.01.20 Support MX-2800SE
		ShowString(3, L"Please wait until ATM reboot");
	else
		ShowString(2, L"Please wait until ATM reboot");
	// end of [#2300]

	ShowScreen(SCR_INFO);
	WaitForMilliSecond(100);

	//////////////////////////////////////////////////////////////////////////
	//	GET OS VERSION
	{
		StateInfo.m_strUSBUpdateSrc = m_SystemConfig.GetUpdatePath(BASEDIR_USB);
		StateInfo.m_strRMSUpdateSrc = m_SystemConfig.GetUpdatePath(BASEDIR_ROOT);
		StateInfo.m_strSDUpdateSrc = m_SystemConfig.GetUpdatePath(BASEDIR_SDCARD);
		StateInfo.m_strUpdateRoot = m_SystemConfig.GetUpdatePath(BASEDIR_ROOT);
	}

	//////////////////////////////////////////////////////////////////////////
	//	GET UPDATE SOURCE
	
	int	nRegSource = RegGetInt(_T("SOFTWARE\\ATM\\APP"), _T("RMSFileUpdate"), 0);
	if ( nRegSource == (int)FROM_RMS)
	{
		StateInfo.m_eUpdateSource = FROM_RMS;
	}
	// [#2378] US Justin 2015.11.12 Dual Step Update
	else if (nRegSource == (int)FROM_UPDATE2)
	{
		StateInfo.m_eUpdateSource = FROM_UPDATE2;
	}
	// End of [#2378]
	// [#RWC6-121] US William Remote Updates
	else if (nRegSource == (int)FROM_REMOTE)
	{
		StateInfo.m_eUpdateSource = FROM_REMOTE;
	}
	// end of [#RWC6-121]
	else
	{
		StateInfo.m_eUpdateSource = FROM_USB;
	}


	//////////////////////////////////////////////////////////////////////////
	//	MOVE OBJECT STORE SIZE FOR UPDATE FILES.
	{
		RETAILMSG(DBG_INFO, (_T("[UPDATE] MOVE OBJECT STORE SIZE FOR UPDATE FILES.\n")));

		DWORD dwStorePages, dwRamPages, dwPageSize;
		if(!GetSystemMemoryDivision(&dwStorePages, &dwRamPages, &dwPageSize))
		{
			RETAILMSG(DBG_INFO, (_T("[UPDATE] Failed to GetSystemMemoryDivision()\n")));
			return STATE_UP_REBOOT;
		}

		RETAILMSG(DBG_INFO, (_T("[UPDATE] SYSTEM DIVISION : STORE(%d), RAM(%d), SIZE(%d)\n"), dwStorePages, dwRamPages, dwPageSize));
		dwRamPages = dwStorePages;
//		dwStorePages = (DWORD)(100000000 / dwPageSize);	// 100M
//		dwStorePages = (DWORD)(133120000 / dwPageSize);	// 130M [#2368] NH KSK 2015.10.25 (이거까지 가능하나 같은데...필드 update가 문제라 적용 방안 검토 필요)
		dwStorePages = (DWORD)(122880000 / dwPageSize);	// 130M [#2368] NH KSK 2015.10.25 (이거까지 가능하나 같은데...필드 update가 문제라 적용 방안 검토 필요)

		RETAILMSG(DBG_INFO, (_T("[UPDATE] SET STORE : (%d)->(%d)\n"), dwRamPages, dwStorePages));
		if (SetSystemMemoryDivision(dwStorePages) != SYSMEM_CHANGED)
		{
			RETAILMSG(DBG_INFO, (_T("[UPDATE] Failed to SetSystemMemoryDivision()\n")));
			return STATE_UP_REBOOT;
		}
	}

	// Next STATE.
	if (StateInfo.m_eUpdateSource == FROM_RMS)
		return STATE_UP_FROM_RMS;
	else if (StateInfo.m_eUpdateSource == FROM_UPDATE2)	// [#2378] US Justin 2015.11.12 Dual Step Update
	{
		// Necessary file "/ATM/UPDATE2/patch.zip" is moved to "/Update/NH2700CE/patch.zip" by WINATM.exe
		// Will use Same routine as "RMSUPDATE"
		return STATE_UP_FROM_RMS;
	}													// End of [#2378]
	else if (StateInfo.m_eUpdateSource == FROM_REMOTE)
		return STATE_UP_FROM_REMOTE;

	return STATE_UP_FROM_USB;
}
