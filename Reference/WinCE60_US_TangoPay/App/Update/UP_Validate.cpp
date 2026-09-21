#include "stdafx.h"
#include "UP_Validate.h"
//#include "unzip.h"

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Validate
 FUNCTION NAME: CUP_Validate()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUP_Validate::CUP_Validate()
{
	// 자신을 등록한다.
	m_strName = STATE_UP_VALIDATE;
	CUpdateState::Register(m_strName, this);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Validate
 FUNCTION NAME: ~CUP_Validate()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUP_Validate::~CUP_Validate()
{
}


/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Validate
 FUNCTION NAME: Process()
 RETURN TYPE  : 이동 할 다음 STATE
 PARAMETER    : STATE간 공유하는 정보.
 DESCRIPTION  : STATE의 정의된 동작을 처리하는 진입접 이다.
-------------------------------------------------------------------*/
CString CUP_Validate::Process(CUpdateStateInfo &StateInfo)
{
	if (CheckValidate(StateInfo) == FALSE)
		return STATE_UP_REBOOT;

	return STATE_UP_UPDATE;
}

BOOL CUP_Validate::CheckValidate(CUpdateStateInfo &StateInfo)
{
	// copy 하면서 이미 검증이 되었다.
	if (StateInfo.m_eUpdateType == UT_MASTER_FILE ||
		StateInfo.m_eUpdateType == UT_PATCH_FILE)
	{
		return TRUE;
	}

	// ZIP File 만 검증한다.
	if (StateInfo.m_eUpdateType == UT_MASTER_ZIP ||
		StateInfo.m_eUpdateType == UT_PATCH_ZIP)
	{
		HZIP		hz;
		ZRESULT		zr;
		ZIPENTRY	ze;
		int			nNumItems, i;
		CString		strFileName;
		CString		strTempFileName, strInfo;
		CString		strTemp;
		CString     strUpdateRoot;

		// create folder \\Update
		CreateDirectory(L"\\Update", NULL);
		strTemp.Format(L"%s", StateInfo.m_strUpdateRoot);
		CreateDirectory(strTemp, NULL);

		if (StateInfo.m_eUpdateType == UT_MASTER_ZIP)
			strUpdateRoot = GetUpdateFileName(UT_MASTER_FILE, StateInfo.m_strUpdateRoot);
		else
			strUpdateRoot = GetUpdateFileName(UT_PATCH_FILE, StateInfo.m_strUpdateRoot);
		CreateDirectory(strUpdateRoot, NULL);

		PrepareScreen(SCR_FILECTRL);
		ShowString(1, L"Check update files");
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

		strInfo.Format(L"[ Extracting... ]");
		ShowString(2, strInfo);
		ShowScreen(SCR_FILECTRL);

		NHDEBUG(1, (L" -- EXTRACTING ZIP... \n"));
		// zip file name
		strFileName = GetUpdateFileName(StateInfo.m_eUpdateType, StateInfo.m_strUpdateRoot);

		// [#2012] NH KJW 2011.01.20 unzip을 CmnLib로 이동하여 공용화함
		//hz = OpenZip(strFileName, 0);
		hz = uzOpenZip(strFileName, 0);
		// end of [#2012]

		// Get number of files
		// [#2012] NH KJW 2011.01.20
		//GetZipItem(hz, -1, &ze);
		uzGetZipItem(hz, -1, &ze);
		// end of [#2012]
		nNumItems = ze.index;

		for (i = 0; i < nNumItems; i++)
		{
			// [#2012] NH KJW 2011.01.20
			//GetZipItem(hz, i, &ze);
			uzGetZipItem(hz, i, &ze);
			// end of [#2012]

			strTemp = ze.name;

			strTempFileName.Format(L"%s\\%s", strUpdateRoot, ze.name);

// 			// Show process information to customer
// 			strInfo.Format(L"[ Extracting... ]  %3d%% - %4d/%4d%s%s",
// 									(int)((((float)(i+1)/(float)nNumItems)*100.0)+0.5),
// 									i+1, nNumItems, SCR_CMD_DELIMITER, ze.name);
// 			ShowString(2, strInfo);
// 			UpdateScreen();
// 
// 			WaitForMilliSecond(10);

			// unzip file
			// [#2012] NH KJW 2011.01.20
			//zr = UnzipItem(hz, i, strTempFileName);
			zr = uzUnzipItem(hz, i, strTempFileName);
			// end of [#2012]
			if (ZR_OK != zr)
			{
				NHERROR((_T("UnzipItem Failed.. Return(0x%08X)\n"), zr));
				break;
			}

			WaitForMilliSecond(10);
		}

		// [#2012] NH KJW 2011.01.20
		//CloseZip(hz);
		uzCloseZip(hz);
		// end of [#2012]
		NHDEBUG(1, (L" -- EXTRACTING ZIP... DONE\n"));

		// success to validate
		if (nNumItems > 0 && i >= nNumItems)
		{
			return TRUE;
		}
	}

	return FALSE;
}