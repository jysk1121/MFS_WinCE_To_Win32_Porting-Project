
#include "stdafx.h"
#include "EagleSVCLib.h"

#include "SVC_Manager.h"

#ifdef _WIN32_WCE
#include "../../Template/pkfuncs.h"
#endif // _WIN32_WCE

#include "../../EagleCE_Framework/EagleCE_Framework/EagleDataManager.h"
#include "EagleTritonMsg.h"

#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"


// 인스턴스
CEagleSVCLib* CEagleSVCLib::m_pInstance = NULL;

#define		MINIMUM_PRIT_LINE		15

/** **********************************************************
*	@brief		인스턴스 생성
*	@retval		없음
************************************************************/
void CEagleSVCLib::CreateInstance()
{
	m_pInstance = new CEagleSVCLib();

}


/** **********************************************************
*	@brief		인스턴스 취득
*	@retval		없음
************************************************************/
CEagleSVCLib* CEagleSVCLib::GetInstance()
{
	return m_pInstance;
}


/** **********************************************************
*	@brief		인스턴스 해제
*	@retval		없음
************************************************************/
void CEagleSVCLib::ReleaseInstance()
{
	if (m_pInstance)
	{
		delete m_pInstance;
	}
}


/** **********************************************************
*	@brief CEagleSVCLib 생성자
************************************************************/
CEagleSVCLib::CEagleSVCLib()
{
}


/** **********************************************************
*	@brief CEagleSVCLib 소멸자
************************************************************/
CEagleSVCLib::~CEagleSVCLib()
{
}


#define	SETTING_ERROR_COUNT		18
static CString SettingErrorCodeTbl[SETTING_ERROR_COUNT] = { ERROR_TERMINALID, ERROR_HOST_IP, ERROR_TERMINAL_IP, ERROR_PHONE_NUMBER, ERROR_FRONT_DOOR_OPEN, 
															ERROR_SAFE_DOOR_OPEN, ERROR_ALL_CST_COUNT_EMPTY, ERROR_ALL_CST_SETTING, ERROR_ALL_CST_EMPTY, ERROR_INVALID_DENOMINATION,
															ERROR_JOURNAL_FULL, ERROR_MASTER_PW_DEFAULT, ERROR_ADMIN_PW_DEFAULT, ERROR_REPLENISHMENT_PW_DEFAULT, ERROR_PINPAD_STATE,
															ERROR_MASTERKEY_EMPTY, ERROR_PINKEY_EMPTY, ERROR_STUCK_EPP};

/** **********************************************************
*	@brief Terminal 설정 정보 체크
*	@retval	정상인 경우 FALSE
*	@retval	에러인 경우 TRUE
************************************************************/
BOOL CEagleSVCLib::IsTerminal_Error()
{
	// 최초 Device Error가 있는 경우에는 Terminal Error를 Check하지 않고 Return하도록 로직 수정 (속도개선)
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
	{
		// Error Code가 Z로 시작하지 않는 경우 에러상태 유지
		if ( Get_ErrorCode().Left(1) != _T("Z"))
			return TRUE;
	}

	int nCBXCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);
	int nTotalCSTCount = 0, i = 0;

	nTotalCSTCount = CDEV_Manager::GetInstance()->m_DEV_CDM.Get_CBXAllNoteCount();

	// Setting Error가 Clear되었는지 Check하는 로직
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
	{
		for (i=0; i<SETTING_ERROR_COUNT; i++)
		{
			if (Get_ErrorCode().IsEmpty())
				break;

			if (Get_ErrorCode().CompareNoCase(SettingErrorCodeTbl[i]) == 0)
			{
				switch(i)
				{
				case 0:
					if (CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID.CompareNoCase(DEFAULT_TERMINAL_ID) != 0)
					{
						Clear_Error();
					}
					break;

				case 1:
					if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
					{
						if (CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip.CompareNoCase(DEFAULT_HOSTIP) != 0)
						{
							Clear_Error();					
						}
					}
					else
					{
						Clear_Error();
					}
					break;

				case 2:
					if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
					{
						if (CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP == S_DISABLE)
						{
							if (CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip.CompareNoCase(DEFAULT_TERMINAL_IP) != 0)
							{
								Clear_Error();				
							}
						}
						else
						{
							Clear_Error();
						}
					}
					else
					{
						Clear_Error();
					}
					break;

				case 3:
					if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("MODEM"))
					{
						if (CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number.CompareNoCase(DEFAULT_PHONENUMBER) != 0)
						{
							Clear_Error();
						}
					}
					else
					{
						Clear_Error();
					}
					break;

				case 4:
					if (CSVC_Manager::GetInstance()->Is_DIO_Front_Door_Open() == FALSE)
					{
						Clear_Error();
					}
					break;

				case 5:
					if (CSVC_Manager::GetInstance()->Is_DIO_Safe_Door_Open() == FALSE)
					{
						Clear_Error();
					}
					break;

				case 6:
					if (nTotalCSTCount > 0)
					{
						Clear_Error();
					}
					break;

				case 7:
					if (CDEV_Manager::GetInstance()->m_DEV_CDM.Get_CBXAllStatus() != WFS_CDM_STATCUMISSING)
					{
						Clear_Error();
					}
					break;

				case 8:
					if (CDEV_Manager::GetInstance()->m_DEV_CDM.Get_CBXAllStatus() != WFS_CDM_STATCUEMPTY)
					{
						Clear_Error();
					}
					break;

				case 9:
					if (Check_Invalid_CBXAllDenomination() == FALSE)
					{
						Clear_Error();
					}
					break;

				case 10:
					if (CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastJournalNo() < MAX_JOURNAL_COUNT)
					{
						Clear_Error();
					}
					break;

				case 11:
					if (CEagleDataManager::GetInstance()->m_Config.m_System.strMaster_password.CompareNoCase(DEFAULT_MASTER_PW) != 0)
					{
						Clear_Error();
					}
					break;

				case 12:
					if (CEagleDataManager::GetInstance()->m_Config.m_System.strAdministrator_password.CompareNoCase(DEFAULT_ADMIN_PW) != 0)
					{
						Clear_Error();
					}
					break;

				case 13:
					if (CEagleDataManager::GetInstance()->m_Config.m_System.strReplenishment_password.CompareNoCase(DEFAULT_REPLENISHMENT_PW) != 0)
					{
						Clear_Error();
					}
					break;

				case 14:
					if (CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEPPState == EPP_STATE_ACTIVATED)
					{
						Clear_Error();
					}
					break;

				case 15:
					if (CSVC_Manager::GetInstance()->m_strMasterKey_KCV.IsEmpty() == FALSE)
					{
						Clear_Error();
					}
					break;

				case 16:
					if (CSVC_Manager::GetInstance()->m_strPINKey_KCV.IsEmpty() == FALSE)
					{
						Clear_Error();
					}
					break;

				case 17:
					if (CDEV_Manager::GetInstance()->m_DEV_CREPP.m_bEPPStuckState == FALSE)
					{
						Clear_Error();
					}
					break;

				default:
					break;
				}
			}
		}

		// Error Check 후에 장애가 없는 경우에 하기 로직 추가 Check하도록 로직 수정
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
			return TRUE;
	}

	// Journal Full을 제일 먼저 Check
	if (CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastJournalNo() >= MAX_JOURNAL_COUNT)
	{
		// Journal Full Error
		Set_ErrorCode(ERROR_JOURNAL_FULL);
		return TRUE;
	}

	// Stuck EPP를 2번째로 Check
	if (CDEV_Manager::GetInstance()->m_DEV_CREPP.m_bEPPStuckState == TRUE)
	{
		Set_ErrorCode(ERROR_STUCK_EPP);
		return TRUE;
	}

#if !(NETWORK_OFFLINE_MODE)
#ifdef _WIN32_WCE
	// Is Terminal Default?
	if (CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID.CompareNoCase(DEFAULT_TERMINAL_ID) == 0)
	{
		// Terminal ID Setting Error
		Set_ErrorCode(ERROR_TERMINALID);
		return TRUE;
	}
#else
	// Win32 simulator: the terminal is intentionally left unconfigured, so the
	// default terminal ID must not hold the machine out of service.
#endif	// _WIN32_WCE

#ifdef _WIN32_WCE
	// 1. Master Password Check
	if (CEagleDataManager::GetInstance()->m_Config.m_System.strMaster_password.CompareNoCase(DEFAULT_MASTER_PW) == 0)
	{
		// Master password is default
		Set_ErrorCode(ERROR_MASTER_PW_DEFAULT);
		return TRUE;
	}

	// 2. Administrator Password Check
	if (CEagleDataManager::GetInstance()->m_Config.m_System.strAdministrator_password.CompareNoCase(DEFAULT_ADMIN_PW) == 0)
	{
		// Administrator is default
		Set_ErrorCode(ERROR_ADMIN_PW_DEFAULT);
		return TRUE;
	}

	// 3. Replenishment Password Check
	if (CEagleDataManager::GetInstance()->m_Config.m_System.strReplenishment_password.CompareNoCase(DEFAULT_REPLENISHMENT_PW) == 0)
	{
		// Administrator is default
		Set_ErrorCode(ERROR_REPLENISHMENT_PW_DEFAULT);
		return TRUE;
	}
#else
	// Win32 simulator: the factory-default passwords must not hold the machine
	// out of service - the simulator ships SYSTEM.CFG unconfigured on purpose.
#endif	// _WIN32_WCE
#endif

	// 4. Invalid Denomination
	if (Check_Invalid_CBXAllDenomination() == TRUE)
	{
		Set_ErrorCode(ERROR_INVALID_DENOMINATION);
		return TRUE;
	}

	// 5. 논리적인 매수가 모두 0인 경우 Error 처리
	if (nTotalCSTCount <= 0)
	{
		Set_ErrorCode(ERROR_ALL_CST_COUNT_EMPTY);
		return TRUE;
	}

#if !(NETWORK_OFFLINE_MODE)
	// Network Checking
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
	{
		// These checks also apply to the Win32 simulator: the host connection is
		// the one thing that is not emulated, so it has to be exercised for
		// real. CSVC_Manager::Initialize() fills the terminal address in from
		// the live adapter on Win32, which leaves the host address as the value
		// the tester must configure before the terminal will go in service.

		// Check Terminal IP
		if (CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP == S_DISABLE)
		{
			if (CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip.CompareNoCase(DEFAULT_TERMINAL_IP) == 0)
			{
				// TERMINAL IP Setting Error
				Set_ErrorCode(ERROR_TERMINAL_IP);
				return TRUE;
			}
		}

		// Check Host IP
		if (CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip.CompareNoCase(DEFAULT_HOSTIP) == 0)
		{
			// HOST IP Setting Error
			Set_ErrorCode(ERROR_HOST_IP);
			return TRUE;
		}
	}
	else
	{
		if (CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number.CompareNoCase(DEFAULT_PHONENUMBER) == 0)
		{
			// Phone Number Setting Error
			Set_ErrorCode(ERROR_PHONE_NUMBER);
			return TRUE;
		}
	}

	// EPP STATE CHECK [$$$] 임시 주석 처리
	//if (CSVC_Manager::GetInstance()->m_nEPPState != EPP_STATE_ACTIVATED)
	//{
	//	Set_ErrorCode(ERROR_PINPAD_STATE);
	//	return TRUE;
	//}

	//if (CSVC_Manager::GetInstance()->m_strMasterKey_KCV.IsEmpty())
	//{
	//	Set_ErrorCode(ERROR_MASTERKEY_EMPTY);
	//	return TRUE;
	//}

	//if (CSVC_Manager::GetInstance()->m_strPINKey_KCV.IsEmpty())
	//{
	//	Set_ErrorCode(ERROR_PINKEY_EMPTY);
	//	return TRUE;
	//}
#endif

	// Reject Bin이 Full일 경우 Error 처리 (보류)

	int nCBXStatus = CDEV_Manager::GetInstance()->m_DEV_CDM.Get_CBXAllStatus();

	// 카세트가 모두 Missing인 경우 Error 처리
	if (nCBXStatus == WFS_CDM_STATCUMISSING)
	{
		Set_ErrorCode(ERROR_ALL_CST_SETTING);
		return TRUE;
	}

	if (nCBXStatus == WFS_CDM_STATCUEMPTY)
	{
		Set_ErrorCode(ERROR_ALL_CST_EMPTY);
		return TRUE;
	}

	// DIO - Front Door 열
	if (CSVC_Manager::GetInstance()->Is_DIO_Front_Door_Open())	// Device관련 부분은 Device쪽으로 변경 필요
	{
		// 상부 Door Check
		Set_ErrorCode(ERROR_FRONT_DOOR_OPEN);
		return TRUE;
	}

	// DIO - Safe Door 열림
	if (CSVC_Manager::GetInstance()->Is_DIO_Safe_Door_Open())		// Device관련 부분은 Device쪽으로 변경 필요
	{
		// 하부 금고 Door Check
		Set_ErrorCode(ERROR_SAFE_DOOR_OPEN);
		return TRUE;
	}

	return FALSE;
}


/** **********************************************************
*	@brief 장애 발생시 Error Code 설정
*	@retval	없음
************************************************************/
void CEagleSVCLib::Set_ErrorCode(CString strErrorCode, int nErrorDevice, CString strErrorMsg)
{
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE)
	{
		LOG(Error, _T("Set_ErrorCode(%s), Error Device(0x%X)"), strErrorCode, nErrorDevice);

		CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError = TRUE;
		CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode = strErrorCode;
		CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorMsg = strErrorMsg;
	}

	// 로그만 주석 처리
	if (nErrorDevice != 0)
	{
		CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice |= nErrorDevice;
	//	LOG(Error, _T("strErrorCode(%s), m_nError Device(0x%X)"), strErrorCode, CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice);
	}
}


/** **********************************************************
*	@brief	OP에서 Device 장애 발생시 ErrorCode 설정
*	@param	CString strDeviceKind	: Device 종류
*	@param	CString strErrorCode	: Error Code
*	@param	CString strErrorMsg		: Error Message
*	@retval	없음
************************************************************/
void CEagleSVCLib::Set_DeviceErrorCode(CString strDeviceKind, CString strErrorCode, CString strErrorMsg /*= _T("")*/)
{
	CString strErrorType = CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode.Left(1);

//	if(strErrorType.CompareNoCase(strDeviceKind) == 0)
	if( (strErrorType.CompareNoCase(strDeviceKind) == 0) || (FALSE == CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError) )
	{
		LOG(Error, _T("Set_DeviceErrorCode(%s)"), strErrorCode);

		CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError = TRUE;
		CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode = strErrorCode;
		CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorMsg = strErrorMsg;
	}

	if(DEVICE_CDR == strDeviceKind)
	{
		CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice |= ERROR_DEVICE_IDC;
	}
	else if(DEVICE_CDM == strDeviceKind)
	{
		CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice |= ERROR_DEVICE_CDM;
	}
	else if(DEVICE_EPP == strDeviceKind)
	{
		CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice |= ERROR_DEVICE_EPP;
	}
	else if(DEVICE_EJL == strDeviceKind)
	{

	}
	else if(DEVICE_PTR == strDeviceKind)
	{
		CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice |= ERROR_DEVICE_PTR;
	}
	else if(DEVICE_DIO == strDeviceKind)
	{
		CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice |= ERROR_DEVICE_SIU;
	}

	LOG(Info, _T("Set_DeviceErrorCode => ErrorCode:[%s], ErrorDevice:[0x%08x]"), CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode, CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice);
}


/** **********************************************************
*	@brief 장애 발생시 Error Code 취득
*	@retval	없음
************************************************************/
CString CEagleSVCLib::Get_ErrorCode()
{
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
		return CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode;

	return _T("");
}


/** **********************************************************
*	@brief 장애 발생시 Error Message 취득
*	@retval	없음
************************************************************/
CString CEagleSVCLib::Get_ErrorMsg()
{
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
	{
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorMsg.IsEmpty() == FALSE)
			return CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorMsg;
	}

	return _T("");
}


/** **********************************************************
*	@brief Error Code 초기화
*	@retval	없음
************************************************************/
void CEagleSVCLib::Clear_Error()
{
	CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError = FALSE;
	CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode.Empty();
	CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice = 0;
	CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorMsg.Empty();
}


/** **********************************************************
*	@brief	정상 상태의 Device의 ErrorCode clear
*	@param	CString strDeviceKind	: Device 종류
*	@retval	없음
************************************************************/
void CEagleSVCLib::Clear_DeviceError(CString strDeviceKind)
{
	CString strErrorType = CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode.Left(1);

	LOG(Info, _T("ErrorCode Device Kind: [%s]"), strDeviceKind);

	// 장애코드와 동일한 Device일 경우에만 All Clear
	if(strErrorType.CompareNoCase(strDeviceKind) == 0)
	{
		Clear_Error();
	}
	else	// 장애코드와 동일하지 않은 Device인 경우에는 Device 종류만 삭제(ErrorCode 유지)
	{
		if(DEVICE_CDR == strDeviceKind)
		{
			CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice &= ~(ERROR_DEVICE_IDC);
		}
		else if(DEVICE_CDM == strDeviceKind)
		{
			CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice &= ~(ERROR_DEVICE_CDM);
		}
		else if(DEVICE_EPP == strDeviceKind)
		{
			CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice &= ~(ERROR_DEVICE_EPP);
		}
		else if(DEVICE_EJL == strDeviceKind)
		{

		}
		else if(DEVICE_PTR == strDeviceKind)
		{
			CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice &= ~(ERROR_DEVICE_PTR);
		}
		else if(DEVICE_DIO == strDeviceKind)
		{
			CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice &= ~(ERROR_DEVICE_SIU);
		}
	}

	LOG(Info, _T("After Clear => ErrorCode:[%s], ErrorDevice:[0x%08x]"), CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode, CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice);
}


/** **********************************************************
*	@brief	Backup Log Files
*	@param	CString strDestDirectory	: Backup할 위치
*	@param	BOOL bCompressed			: 압축 여부
*	@retval	없음
************************************************************/
BOOL CEagleSVCLib::Backup_LogFile(CString strDestDirectory)
{
	BOOL	bResult = FALSE;
	CString strZipFileName;
	CString strErrorLogFilePath;

	SYSTEMTIME		st;
	GetLocalTime(&st);

	CreateDirectory(strDestDirectory, NULL);

	///////////////////////////////////////////////////
	// Destination의 Log File을 모두 제거한다.
	CUtil::DeleteFileInDirectory(strDestDirectory, _T("*.*"));

	///////////////////////////////////////////////////
	// copy log file from Trace Directory to Destination Directory folder

	// make file name for search.

	bResult = CUtil::IsExistFile(PATH_LOG_ABS);

	if (bResult == TRUE)
	{
		CreateDirectory(PATH_TEMP_ABS, NULL);
		CUtil::CopyFileInDirectory(PATH_LOG_ABS, _T("*.*"), PATH_TEMP_ABS);

		strZipFileName.Format(_T("%s\\CURRENT_LOG_%04d%02d%02d_%02d%02d%02d.zip"), strDestDirectory, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		bResult = CUtil::ZippingFromFolder(PATH_TEMP_ABS, strZipFileName);
		LOG(Info, _T("Current Backup log file result (%d)"), bResult);

		CUtil::DeleteFileInDirectory(PATH_TEMP_ABS, _T("*.*"));

		// Error Log는 없을 수도 있으므로 Result에 갱신하지 않도록 함
		strZipFileName.Format(_T("%s\\ERROR_LOG_%04d%02d%02d_%02d%02d%02d.zip"), strDestDirectory, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		strErrorLogFilePath.Format(_T("%s%s"), CUtil::GetAppPath(), PATH_LOG);
		CUtil::ZippingFromFolder(strErrorLogFilePath, strZipFileName);
		LOG(Info, _T("Error Backup log file end"));
	}
	else
	{
		LOG(Info, _T("Trace directory is not exist files"));
	}

	return bResult;
}


/** **********************************************************
*	@brief	Backup Log Files
*	@param	CString strDestDirectory	: Backup할 위치
*	@retval	없음
************************************************************/
BOOL CEagleSVCLib::Add_Backup_LogFile(CString strDestDirectory, CString strErrorCode, BOOL bRemainLog)
{
	BOOL	bResult = FALSE;
	CString strZipFileName;
	DWORD	nIndex = 0;
	BOOL	bFileFind = FALSE;
	CString strFilePath;
	
	SYSTEMTIME		st;
	GetLocalTime(&st);

	CreateDirectory(strDestDirectory, NULL);

	// make file name for search.
	bResult = CUtil::IsExistFile(PATH_LOG_ABS);

	if (bResult == TRUE)
	{
		if (bRemainLog == FALSE)
		{
			// File Loading 전 상태이므로 System 부분만 임의로 Loading 하도록 로직 보완
			CEagleDataManager::GetInstance()->m_Config.LoadSystem();
		}

		nIndex = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_System.strBackUpLogIndex);
		nIndex++;

		if (nIndex >= MAX_LOG_FILE_COUNT)
			nIndex = 1;	
		
		// 이전 동일 Index_xxxxx.zip이 있다면 File 삭제
		CString strDeleteFile;
		WIN32_FIND_DATA	FileData;
		HANDLE			hSearch = INVALID_HANDLE_VALUE;

		strDeleteFile.Format(_T("%s\\%03d_*.zip"), strDestDirectory, nIndex);

		hSearch = FindFirstFile(strDeleteFile, &FileData);

		if (hSearch != INVALID_HANDLE_VALUE)
		{
			WCHAR	strSrcFile[255];

			while(TRUE)
			{
				memset(strSrcFile, 0, sizeof(strSrcFile));

				// make source file full path
				wcscpy(strSrcFile, strDestDirectory);
				wcscat(strSrcFile, _T("\\"));
				wcscat(strSrcFile, FileData.cFileName);

				DeleteFile(strSrcFile);

				if (FindNextFile(hSearch, &FileData) == FALSE)
					break;
			}

			FindClose(hSearch);
		}

		// 현재 Trace 폴더에 있는 File을 open시 에러가 발생하므로 memory상에 Temp folder에 copy후 진행하도록 변경
		CreateDirectory(PATH_TEMP_ABS, NULL);
		CUtil::CopyFileInDirectory(PATH_LOG_ABS, _T("*.*"), PATH_TEMP_ABS);

		if (strErrorCode.IsEmpty())
			strZipFileName.Format(_T("%s\\%03d_LOG_%04d%02d%02d_%02d%02d.zip"), strDestDirectory, nIndex, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
		else
			strZipFileName.Format(_T("%s\\%03d_%s_LOG_%04d%02d%02d_%02d%02d.zip"), strDestDirectory, nIndex, strErrorCode, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

		bResult = CUtil::ZippingFromFolder(PATH_TEMP_ABS, strZipFileName);

		// BackUp한 Log File도 같이 Delete 시도
		CUtil::DeleteFileInDirectory(PATH_LOG_ABS, _T("*.*"));

		CUtil::DeleteFileInDirectory(PATH_TEMP_ABS, _T("*.*"));

		if (bRemainLog == TRUE)
		{
			LOG(Info, _T("Add Backup Log File success (%d)"), bResult);
		}

		CEagleDataManager::GetInstance()->m_Config.m_System.strBackUpLogIndex = CUtil::IntToString(nIndex);
		CEagleDataManager::GetInstance()->m_Config.SaveSystemData(_T("backup_log_index"), CUtil::IntToString(nIndex));
	}
	else
	{
		if (bRemainLog == TRUE)
		{
			LOG(Info, _T("Trace directory is not exist files"));
		}
	}

	return bResult;
}


/** **********************************************************
*	@brief	Backup Jnl Files
*	@param	CString strDestDirectory	: Backup할 위치
*	@retval	없음
************************************************************/
BOOL CEagleSVCLib::Backup_JnlFile(CString strDestDirectory)
{
	BOOL	bResult = FALSE;
	CString strPathJnl;
	CString strJnlFileName;
	CString strTemp;
	CString strZipFileName;

	SYSTEMTIME		st;
	GetLocalTime(&st);

	CreateDirectory(strDestDirectory, NULL);

	// Destination Folder에 이전 File이 있을지 모르므로 제거
	CUtil::DeleteFileInDirectory(strDestDirectory, _T("*.*"));

	///////////////////////////////////////////////////
	// copy log file from Trace Directory to Destination Directory folder

	// make file name for search.
	strJnlFileName = _T("MFS_EJL.db");
	strPathJnl.Format(_T("%s%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH);

	strTemp.Format(_T("%s\\%s"), strPathJnl, strJnlFileName);
	bResult = CUtil::IsExistFile(strTemp);

	if (bResult == TRUE)
	{
		LOG(Info, _T("Journal DB file zip Start"));

		strZipFileName.Format(_T("%s\\MFS_EJL_%04d%02d%02d_%02d%02d%02d.zip"), PATH_TEMP_ABS, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		bResult = CUtil::ZippingFromFile(strPathJnl, strJnlFileName, strZipFileName);

		LOG(Info, _T("Journal DB file zip result (%d)"), bResult);

		// File Size가 크므로 Temp 폴더에 압축 후 Copy하도록 로직 수정
		bResult = CUtil::CopyFileInDirectory(PATH_TEMP_ABS, _T("*.*"), strDestDirectory);

		LOG(Info, _T("Journal zip file copy result (%d)"), bResult);

		CUtil::DeleteFileInDirectory(PATH_TEMP_ABS, _T("*.*"));
	}
	else
	{
		LOG(Info, _T("Trace directory is not exist files"));
	}

	return bResult;
}


/** **********************************************************
*	@brief	Backup Config File
*	@param	CString strDestDirectory	: Backup할 위치
*	@retval	없음
************************************************************/
BOOL CEagleSVCLib::Backup_ConfigFile(CString strDestDirectory)
{
	BOOL	bResult = TRUE;
	CString strConfigPath, strTempConfigDir;
	CString strFileName;

	CString strConfigData, strAllConfigData;
	unsigned int nCRC = 0;
	BYTE byKey_16[32] = { 0, };
	BYTE *pbyAllConfigData = NULL;
	BYTE *pbyEncryptConfigData = NULL;
	DWORD dwAllConfigDataLen = 0, dwEncryptConfigDataLen = 0;

	strTempConfigDir = PATH_TEMP_ABS;

	CreateDirectory(strDestDirectory, NULL);
	CreateDirectory(strTempConfigDir, NULL);

	// Backup전에 이전 USB의 Backup Data를 Delete후 진행하도록 로직 보완
	CUtil::DeleteFileInDirectory(strDestDirectory, _T("*.*"));

	// make file name for search.
	strConfigPath.Format(_T("%s%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH);
	bResult = CUtil::IsExistFile(strConfigPath);

	if (bResult == TRUE)
	{
		// 1. ADS.CFG
		strConfigData = MergeConfigData(strConfigPath, ADS_CFG_PATH_NAME);
		if (strConfigData.IsEmpty() == TRUE)
		{
			LOG(Info, _T("Backup Config File(ADS) is failed."));
			return FALSE;
		}
		else
			LOG(Info, _T("Backup Config File(ADS) is success."));

		strAllConfigData += strConfigData;

		// 2. BINLIST.CFG
		strConfigData = MergeConfigData(strConfigPath, BINLIST_CFG_PATH_NAME);
		if (strConfigData.IsEmpty() == TRUE)
		{
			LOG(Info, _T("Backup Config File(BINLIST) is failed."));
			return FALSE;
		}
		else
			LOG(Info, _T("Backup Config File(BINLIST) is success."));

		strAllConfigData += strConfigData;

		// 3. DEVICE.CFG
		strConfigData = MergeConfigData(strConfigPath, DEVICE_CFG_PATH_NAME);
		if (strConfigData.IsEmpty() == TRUE)
		{
			LOG(Info, _T("Backup Config File(DEVICE) is failed."));
			return FALSE;
		}
		else
			LOG(Info, _T("Backup Config File(DEVICE) is success."));

		strAllConfigData += strConfigData;

		// 4. EMV_CONFIG.CFG
		strConfigData = MergeConfigData(strConfigPath, EAGLE_ATM_EMV_CONFIG_PATH_NAME);
		if (strConfigData.IsEmpty() == TRUE)
		{
			LOG(Info, _T("Backup Config File(EMV_CONFIG) is failed."));
			return FALSE;
		}
		else
			LOG(Info, _T("Backup Config File(EMV_CONFIG) is success."));

		strAllConfigData += strConfigData;

		// 5. EMV_US_COMM_AID.CFG
		strConfigData = MergeConfigData(strConfigPath, EAGLE_ATM_EMV_US_COMM_AID_PATH_NAME);
		if (strConfigData.IsEmpty() == TRUE)
		{
			LOG(Info, _T("Backup Config File(EMV_US_COMM_AID) is failed."));
			return FALSE;
		}
		else
			LOG(Info, _T("Backup Config File(EMV_US_COMM_AID) is success."));

		strAllConfigData += strConfigData;

		// 6. HOST.CFG
		strConfigData = MergeConfigData(strConfigPath, HOST_CFG_PATH_NAME);
		if (strConfigData.IsEmpty() == TRUE)
		{
			LOG(Info, _T("Backup Config File(HOST) is failed."));
			return FALSE;
		}
		else
			LOG(Info, _T("Backup Config File(HOST) is success."));

		strAllConfigData += strConfigData;

		// 7. MESSAGE.CFG
		strConfigData = MergeConfigData(strConfigPath, MESSAGE_CFG_PATH_NAME);
		if (strConfigData.IsEmpty() == TRUE)
		{
			LOG(Info, _T("Backup Config File(MESSAGE) is failed."));
			return FALSE;
		}
		else
			LOG(Info, _T("Backup Config File(MESSAGE) is success."));

		strAllConfigData += strConfigData;

		// 8. OPTION.CFG
		strConfigData = MergeConfigData(strConfigPath, OPTION_CFG_PATH_NAME);
		if (strConfigData.IsEmpty() == TRUE)
		{
			LOG(Info, _T("Backup Config File(OPTION) is failed."));
			return FALSE;
		}
		else
			LOG(Info, _T("Backup Config File(OPTION) is success."));

		strAllConfigData += strConfigData;

		// 9. SYSTEM.CFG
		strConfigData = MergeConfigData(strConfigPath, SYSTEM_CFG_PATH_NAME);
		if (strConfigData.IsEmpty() == TRUE)
		{
			LOG(Info, _T("Backup Config File(SYSTEM) is failed."));
			return FALSE;
		}
		else
			LOG(Info, _T("Backup Config File(SYSTEM) is success."));

		strAllConfigData += strConfigData;

		// Merge 한 파일 암호화 및 새 파일 생성
		HANDLE hFile = INVALID_HANDLE_VALUE;
		DWORD dwWriten = 0;

		strFileName.Format(_T("%s\\%s"), strTempConfigDir, BACKUP_CONFIGFILE_NAME);

		hFile = CreateFile(strFileName,
			GENERIC_WRITE,          // Open for writing
			0,                      // Do not share
			NULL,                   // No security
			CREATE_ALWAYS,			// create
			FILE_ATTRIBUTE_NORMAL,  // Normal file
			NULL);

		if (INVALID_HANDLE_VALUE == hFile)
		{
			LOG(Error, _T("Encrypt Config File is failed."));
			return FALSE;
		}

		dwAllConfigDataLen = strAllConfigData.GetLength();

		if (0 == (dwAllConfigDataLen % DATA_PADDING_SIZE))
			dwEncryptConfigDataLen = dwAllConfigDataLen;
		else
			dwEncryptConfigDataLen = (dwAllConfigDataLen / DATA_PADDING_SIZE + 1) * DATA_PADDING_SIZE;

		pbyEncryptConfigData = new BYTE[dwEncryptConfigDataLen + 2 + 1];		// +2 : CRC 2Byte 추가함.
		memset(pbyEncryptConfigData, 0x00, dwEncryptConfigDataLen + 2 + 1);

		pbyAllConfigData = new BYTE[dwEncryptConfigDataLen + 1];
		memset(pbyAllConfigData, 0x00, dwEncryptConfigDataLen + 1);

		// Key Information
		CUtil::ConvertStringToHex(CONFIG_KEY_32, byKey_16);

		sprintf((char*)pbyAllConfigData, "%S", strAllConfigData);
		CUtil::EncryptKeyData(pbyAllConfigData, dwEncryptConfigDataLen, pbyEncryptConfigData, byKey_16);

		nCRC = CUtil::crc16(pbyEncryptConfigData, dwEncryptConfigDataLen);
		pbyEncryptConfigData[dwEncryptConfigDataLen] = ((nCRC >> 8) & 0xff);
		pbyEncryptConfigData[dwEncryptConfigDataLen + 1] = (nCRC & 0xff);

		dwEncryptConfigDataLen += 2;

		if (!WriteFile(hFile, pbyEncryptConfigData, dwEncryptConfigDataLen, &dwWriten, NULL))
		{
			LOG(Error, _T("EncryptConfig WriteFile is failed."));
			bResult = FALSE;
		}

		if(dwEncryptConfigDataLen != dwWriten)
		{
			LOG(Error, _T("Data Length is mismatch."));
			bResult = FALSE;
		}

		CloseHandle(hFile);

		if (TRUE == bResult)
		{
			CString strDestFilePath;

			strDestFilePath.Format(_T("%s\\%s"), strDestDirectory, BACKUP_CONFIGFILE_NAME);

			int bCopyResult = CopyFile(strFileName, strDestFilePath, FALSE);
			LOG(Info, _T("Backup Config File(BackupConfig.dat) Result: [%d]"), bCopyResult);
		}

		DeleteFile(strFileName);

		// bResult == FALSE 시에는 암호화 파일 삭제
// 			if(FALSE == bResult)
// 				CUtil::DeleteFileInDirectory(strTempConfigDir, BACKUP_CONFIGFILE_NAME);

		if (pbyAllConfigData != NULL)
		{
			delete[] pbyAllConfigData;
			pbyAllConfigData = NULL;
		}

		if (pbyEncryptConfigData != NULL)
		{
			delete[] pbyEncryptConfigData;
			pbyEncryptConfigData = NULL;
		}
	}
	else
	{
		LOG(Error, _T("Config directory is not exist files"));
	}

	return bResult;
}


/** **********************************************************
*	@brief	Config 파일을 하나의 파일로 병합
*	@retval	없음
************************************************************/
CString CEagleSVCLib::MergeConfigData(CString strDestDirectory, CString strFileName)
{
	CString strTempData;
	CString strFindStartKey, strFindEndKey;
	CString strConfigData;
	CString strFilePath;
	BYTE *pbyConfigData = NULL;
	int nConfigDataLen = 0;
	BOOL bRet = FALSE;

	CFile datFile;
	CFileException e;

	strFilePath.Format(_T("%s\\%s"), strDestDirectory, strFileName);

	bRet = datFile.Open(strFilePath, CFile::modeRead, &e);
	if (FALSE == bRet)
	{
		LOG(Error, _T("[%s] file Open is failed."), strFilePath);
		strConfigData.Empty();

		return strConfigData;
	}

	nConfigDataLen = (int)datFile.GetLength();

	if (0 == nConfigDataLen)
	{
		LOG(Error, _T("[%s] file length is zero."), strFilePath);
		datFile.Close();
		strConfigData.Empty();

		return strConfigData;
	}

	pbyConfigData = new BYTE[nConfigDataLen + 1];
	memset(pbyConfigData, 0x00, nConfigDataLen + 1);

	datFile.Read(pbyConfigData, nConfigDataLen);

	datFile.Close();

	strFindStartKey.Format(_T("[%s]\r\n"), strFileName);
	strFindEndKey.Format(_T("[/%s]\r\n"), strFileName);

	strConfigData += strFindStartKey;

	strTempData = pbyConfigData;
	strConfigData += strTempData;

	strConfigData += strFindEndKey;

	if (pbyConfigData != NULL)
	{
		delete[] pbyConfigData;
		pbyConfigData = NULL;
	}

	return strConfigData;
}


/** **********************************************************
*	@brief	Upload Config File
*	@param	CString strSrcDirectory
*	@param	CString strDestDirectory
*	@retval	없음
************************************************************/
BOOL CEagleSVCLib::Upload_ConfigFile(CString strSrcDirectory, CString strDestDirectory)
{
	BOOL bResult = TRUE;
	BOOL bFileSeparateResult = TRUE;
	CString strFilePath;
	CString strTempConfigDir;

	CFile datFile;
	CFileException e;
	CString strAllConfigData;
	BYTE *pbyAllConfigData = NULL;
	BYTE *pbyEncryptConfigData = NULL;
	DWORD dwEncryptDataLen = 0;
	BYTE byKey_16[32] = { 0, };
	BOOL bRet = FALSE;
	unsigned int nCalcCRC = 0, nCRCFromFile = 0;

	strTempConfigDir = PATH_TEMP_ABS;

	CreateDirectory(strDestDirectory, NULL);
	CreateDirectory(strTempConfigDir, NULL);

	strFilePath.Format(_T("%s\\%s"), strSrcDirectory, BACKUP_CONFIGFILE_NAME);

	bRet = datFile.Open(strFilePath, CFile::modeRead, &e);
	if (FALSE == bRet)
	{
		LOG(Error, _T("MergeFile Open is failed."));
		return FALSE;
	}

	dwEncryptDataLen = (DWORD)datFile.GetLength();

	if (0 == dwEncryptDataLen)
	{
		LOG(Error, _T("MergeFile data length is zero."));
		datFile.Close();
		return FALSE;
	}

	pbyEncryptConfigData = new BYTE[dwEncryptDataLen + 1];
	memset(pbyEncryptConfigData, 0x00, dwEncryptDataLen + 1);

	datFile.Read(pbyEncryptConfigData, dwEncryptDataLen);

	datFile.Close();

	dwEncryptDataLen -= 2;

	nCRCFromFile = (pbyEncryptConfigData[dwEncryptDataLen] << 8) | pbyEncryptConfigData[dwEncryptDataLen + 1];
	nCalcCRC = CUtil::crc16(pbyEncryptConfigData, dwEncryptDataLen);

	if (nCRCFromFile == nCalcCRC)
	{
		pbyAllConfigData = new BYTE[dwEncryptDataLen + 1];
		memset(pbyAllConfigData, 0x00, dwEncryptDataLen + 1);

		// Key Information
		CUtil::ConvertStringToHex(CONFIG_KEY_32, byKey_16);
		CUtil::DecryptKeyData(pbyEncryptConfigData, dwEncryptDataLen, pbyAllConfigData, byKey_16);
		
		strAllConfigData = pbyAllConfigData;

		// 1. ADS.CFG
		bFileSeparateResult = SeparateConfigData(strTempConfigDir, ADS_CFG_PATH_NAME, strAllConfigData);
		LOG(Info, _T("Upload Config File(ADS) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
			bResult = FALSE;

		// 2. BINLIST.CFG
		bFileSeparateResult = SeparateConfigData(strTempConfigDir, BINLIST_CFG_PATH_NAME, strAllConfigData);
		LOG(Info, _T("Upload Config File(BINLIST) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
			bResult = FALSE;

		// 3. DEVICE.CFG
		bFileSeparateResult = SeparateConfigData(strTempConfigDir, DEVICE_CFG_PATH_NAME, strAllConfigData);
		LOG(Info, _T("Upload Config File(DEVICE) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
			bResult = FALSE;

		// 4. EMV_CONFIG.CFG
		bFileSeparateResult = SeparateConfigData(strTempConfigDir, EAGLE_ATM_EMV_CONFIG_PATH_NAME, strAllConfigData);
		LOG(Info, _T("Upload Config File(EMV_CONFIG) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
			bResult = FALSE;

		// 5. EMV_US_COMM_AID.CFG
		bFileSeparateResult = SeparateConfigData(strTempConfigDir, EAGLE_ATM_EMV_US_COMM_AID_PATH_NAME, strAllConfigData);
		LOG(Info, _T("Upload Config File(EMV_US_COMM_AID) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
			bResult = FALSE;

		// 6. HOST.CFG
		bFileSeparateResult = SeparateConfigData(strTempConfigDir, HOST_CFG_PATH_NAME, strAllConfigData);
		LOG(Info, _T("Upload Config File(HOST) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
			bResult = FALSE;

		// 7. MESSAGE.CFG
		bFileSeparateResult = SeparateConfigData(strTempConfigDir, MESSAGE_CFG_PATH_NAME, strAllConfigData);
		LOG(Info, _T("Upload Config File(MESSAGE) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
			bResult = FALSE;

		// 8. OPTION.CFG
		bFileSeparateResult = SeparateConfigData(strTempConfigDir, OPTION_CFG_PATH_NAME, strAllConfigData);
		LOG(Info, _T("Upload Config File(OPTION) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
			bResult = FALSE;

		// 9. SYSTEM.CFG
		bFileSeparateResult = SeparateConfigData(strTempConfigDir, SYSTEM_CFG_PATH_NAME, strAllConfigData);
		LOG(Info, _T("Upload Config File(SYSTEM) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
			bResult = FALSE;

		if(TRUE == bResult)
		{
			BOOL bCopyResult = CUtil::CopyFileInDirectory(strTempConfigDir, _T("*.CFG"), strDestDirectory);
			LOG(Info, _T("Upload Config File(Config files) Result: [%d]"), bCopyResult);
		}
		
		CUtil::DeleteFileInDirectory(strTempConfigDir, _T("*.CFG"));

		if (pbyAllConfigData != NULL)
		{
			delete[] pbyAllConfigData;
			pbyAllConfigData = NULL;
		}
	}
	else
	{
		LOG(Error, _T("CheckSum is mismatch."));
		bResult = FALSE;
	}

	if (pbyEncryptConfigData != NULL)
	{
		delete[] pbyEncryptConfigData;
		pbyEncryptConfigData = NULL;
	}

	return bResult;
}


/** **********************************************************
*	@brief	암호화된 하나의 파일을 본래의 cfg 파일로 나누어 생성
*	@param	CString strDestDirectory	: Upload할 위치
*	@param	CString strFileName			: Upload할 파일명
*	@param	CString strAllConfigData	: 복호화 한 config data
*	@retval	없음
************************************************************/
BOOL CEagleSVCLib::SeparateConfigData(CString strDestDirectory, CString strFileName, CString strAllConfigData)
{
	int nConfigIndex[2] = { 0, };
	int nUploadConfigDataLen = 0;
	int nUploadConfigDataStartIndex = 0;
	CString strUploadConfigData;
	CString strFilePath;
	CString strFindStartKey, strFindEndKey;
	BYTE *pbyUploadConfigData = NULL;
	BOOL bReturn = TRUE;

	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwWriten = 0;

	strFilePath.Format(_T("%s\\%s"), strDestDirectory, strFileName);

	strFindStartKey.Format(_T("[%s]"), strFileName);
	strFindEndKey.Format(_T("[/%s]"), strFileName);

	nConfigIndex[0] = strAllConfigData.Find(strFindStartKey);
	nConfigIndex[1] = strAllConfigData.Find(strFindEndKey);

	if ((nConfigIndex[0] != -1) && (nConfigIndex[1] != -1))
	{
		hFile = CreateFile(strFilePath,
			GENERIC_WRITE,          // Open for writing
			0,                      // Do not share
			NULL,                   // No security
			CREATE_ALWAYS,			// create
			FILE_ATTRIBUTE_NORMAL,  // Normal file
			NULL);

		if (INVALID_HANDLE_VALUE == hFile)
		{
			LOG(Error, _T("[%s] CreateFile is failed."), strFilePath);
			return FALSE;
		}

		nUploadConfigDataStartIndex = nConfigIndex[0] + strFindStartKey.GetLength() + 2;	// +2 : \r\n 까지 포함하기 위함.
		nUploadConfigDataLen = nConfigIndex[1] - nUploadConfigDataStartIndex;
		strUploadConfigData = strAllConfigData.Mid(nUploadConfigDataStartIndex, nUploadConfigDataLen);

		pbyUploadConfigData = new BYTE[nUploadConfigDataLen + 1];
		memset(pbyUploadConfigData, 0x00, nUploadConfigDataLen + 1);

		sprintf((char*)pbyUploadConfigData, "%S", strUploadConfigData);

		if (!WriteFile(hFile, pbyUploadConfigData, strUploadConfigData.GetLength(), &dwWriten, NULL))
		{
			LOG(Error, _T("[%s] Upload WriteFile is failed."), strFilePath);
			bReturn = FALSE;
		}

		// bResult == FALSE 시에는 파일 삭제
// 		if(FALSE == bReturn)
// 			CUtil::DeleteFileInDirectory(strDestDirectory, strFileName);

		CloseHandle(hFile);
	}
	else
	{
		LOG(Error, _T("[%s] ConfigIndex is not found. nConfigIndex[0]:[%d], nConfigIndex[1]:[%d]"), strFilePath, nConfigIndex[0], nConfigIndex[1]);
	}

	if (pbyUploadConfigData != NULL)
	{
		delete[] pbyUploadConfigData;
		pbyUploadConfigData = NULL;
	}

	return bReturn;
}


/** **********************************************************
*	@brief	Backup Caputre Image Files
*	@param	CString strDestDirectory	: Backup할 위치
*	@param	CString strBackupDate		: Backup할 날짜 (YYYYMMDD)
*	@retval	없음
************************************************************/
BOOL CEagleSVCLib::Backup_CaptureImageFile(CString strDestDirectory, CString strBackupDate)
{
	BOOL	bResult = FALSE;
	CString strZipFileName;
	CString strCurrentPath;
	CString strSearchFile;
	CString strSrcFileName, strDestFileName;

	WIN32_FIND_DATA	FileData;
	HANDLE hSearch = INVALID_HANDLE_VALUE;

	SYSTEMTIME		st;
	GetLocalTime(&st);

	CreateDirectory(strDestDirectory, NULL);

	///////////////////////////////////////////////////
	// Destination의 File 제거 (여러개를 할 수 있으므로 Destination File은 제거하지 않도록 함
	//CUtil::DeleteFileInDirectory(strDestDirectory, _T("*.*"));

	// search file name
	strCurrentPath.Format(_T("%s%s"),CUtil::GetAppPath(), CAPTURE_IMAGE_FILE_PATH);
	bResult = CUtil::IsExistFile(strCurrentPath);

	if (bResult == TRUE)
	{
		// 해당 날짜의 File을 추출해서 TEMP Folder에 임시로 Copy (File 저장 Format 04157_022165_1381_20190523173225.jpg
		strSearchFile.Format(_T("%s\\*%s*.jpg"), strCurrentPath, strBackupDate);

		LOG(Info, _T("Backup_CaptureImageFile() strSearchFile : [%s]"), strSearchFile);

		hSearch = FindFirstFile(strSearchFile, &FileData);

		if (hSearch != INVALID_HANDLE_VALUE)
		{
			int nFileCount = 0;

			while(TRUE)
			{
				nFileCount++;

				// CopyFileInDirectory 사용시 더 많은 시간이 소요됨 (내부적으로 한번 더 File 전체 검색을 하므로)
				//memset(wchSrcFile, 0, sizeof(wchSrcFile));
				//wcscat(wchSrcFile, FileData.cFileName);
				//CUtil::CopyFileInDirectory(strCurrentPath, wchSrcFile, PATH_TEMP);
				strSrcFileName.Format(_T("%s\\%s"), strCurrentPath, FileData.cFileName);
				strDestFileName.Format(_T("%s\\%s"), PATH_TEMP_ABS, FileData.cFileName);
				::CopyFile(strSrcFileName, strDestFileName, FALSE);

				if (FindNextFile(hSearch, &FileData) == FALSE)
					break;
			}

			FindClose(hSearch);

			LOG(Info, _T("Backup Capture Image file count (%d)"), nFileCount);

			strZipFileName.Format(_T("%s\\CAM_CaptureImageFiles_%s_%04d%02d%02d%02d%02d%02d.zip"), strDestDirectory, strBackupDate, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

			bResult = CUtil::ZippingFromFolder(PATH_TEMP_ABS, strZipFileName);

			LOG(Info, _T("Capture Image file zipping result (%s) (%d)"), strZipFileName, bResult);

			// 완료 후 Temp Folder 제거
			CUtil::DeleteFileInDirectory(PATH_TEMP_ABS, _T("*.*"));
		}
		else
		{
			// File does not exist
			bResult = FALSE;
			LOG(Error, _T("Capture Image File is not exist files (Target Date(%s)"), strBackupDate);
		}
	}
	else
	{
		bResult = FALSE;
		LOG(Error, _T("Capture Image directory is not exist files"));
	}

	return bResult;
}


/** **********************************************************
*	@brief	Add_Backup_CaptureImageFile
*	@param	CString strDestDirectory	: Backup할 위치
*	@retval	없음
************************************************************/
CString CEagleSVCLib::Add_Backup_CaptureImageFile()
{
	BOOL	bResult = FALSE;
	DWORD	nIndex = 0;
	BOOL	bFileFind = FALSE;
	CString strSrcImageFile, strDestImageFile, strDestDirectory;
	CString strBackUpedImageFileName;

	// Make Source File Name
	strSrcImageFile.Format(_T("%s\\%s"), CAPTURE_IMAGE_FILE_PATH_ABS, CLIENT_CAPTURE_IMAGE);

	// Capture File이 존재하는 경우에만 Add Back Up 수행
	if (CUtil::IsExistFile(strSrcImageFile) == FALSE)
	{
		LOG(Error, _T("Capture Image File is not found"));
		return _T("");
	}

	nIndex = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_System.strCaptureImageFile_Index);
	nIndex++;

	if (nIndex >= MAX_CAPTURE_IMAGE_FILE_COUNT)
		nIndex = 1;	

	// 이전 동일 Index_xxxxx.jpg이 있다면 File 삭제
	CString strDeleteFile;
	WIN32_FIND_DATA	FileData;
	HANDLE			hSearch = INVALID_HANDLE_VALUE;

	strDestDirectory.Format(_T("%s%s"), CUtil::GetAppPath(), CAPTURE_IMAGE_FILE_PATH);
	strDeleteFile.Format(_T("%s\\%05d_*.jpg"), strDestDirectory, nIndex);

	hSearch = FindFirstFile(strDeleteFile, &FileData);

	if (hSearch != INVALID_HANDLE_VALUE)
	{
		WCHAR	strSrcFile[255];

		while(TRUE)
		{
			memset(strSrcFile, 0, sizeof(strSrcFile));

			// make source file full path
			wcscpy(strSrcFile, strDestDirectory);
			wcscat(strSrcFile, _T("\\"));
			wcscat(strSrcFile, FileData.cFileName);

			DeleteFile(strSrcFile);

			if (FindNextFile(hSearch, &FileData) == FALSE)
				break;
		}

		FindClose(hSearch);
	}

	// Make Dest File Name
	// Camera Capture Image도 저장 - [@@@]
	int nJnlNo = CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastJournalNo();
	int nSeqNo = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_System.strSequence_Number);
	
	SYSTEMTIME		st;
	GetLocalTime(&st);

	// Journal No는 아직 저장 전이므로 +1을 해서 File에 저장하도록 함
	nJnlNo++;

	if (nJnlNo > MAX_JOURNAL_COUNT)
		nJnlNo = 1;

	strBackUpedImageFileName.Format(_T("%05d_%06d_%04d_%04d%02d%02d%02d%02d%02d.jpg"), nIndex, nJnlNo, nSeqNo, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	strDestImageFile.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), CAPTURE_IMAGE_FILE_PATH, strBackUpedImageFileName);

	bResult = ::CopyFile(strSrcImageFile, strDestImageFile, FALSE);
	LOG(Info, _T("Copy Capture Image File Result (%d) - (%s)"), bResult, strBackUpedImageFileName);
	// [@@@]

	CEagleDataManager::GetInstance()->m_Config.m_System.strCaptureImageFile_Index = CUtil::IntToString(nIndex);
	CEagleDataManager::GetInstance()->m_Config.SaveSystemData(_T("backup_captureimage_index"), CUtil::IntToString(nIndex));

	return strBackUpedImageFileName;
}


/** **********************************************************
*	@brief		HexaDump : Host Trace Data를 Logging하는 기능
*	@param		BYTE *pData		: Data
*	@param		int nLen		: Length
*	@retval		없음
************************************************************/
void CEagleSVCLib::HexaDump(BYTE *pData, int nLen)
{
	CString strTemp, strDumpData;
	BYTE	byAscii[256] = { 0, };
    int		i=0,j=0;

	for(i=0;i<nLen;i++)
	{
		if((i%16)==0)   
		{
			strTemp.Format(_T("%05d:%05d "),i,i+16);
			strDumpData += strTemp;
		}

		strTemp.Format(_T("%02x "),pData[i]);
		strDumpData += strTemp;

		if(j==7)	strDumpData += _T(" ");

		if(pData[i]>=0x20 && pData[i] <= 0x7e)
			byAscii[j] = pData[i];
		else    byAscii[j] = '.';

		j++;

		if(j==16)
		{
			//	strTemp.Format(_T("%-16.16S"),byAscii);
			//	strDumpData += strTemp;
			strDumpData += (CString)byAscii;

			strDumpData.Replace(_T("%"), _T("%%"));
			LOG(Info, _T("%s"), strDumpData);

			strDumpData.Empty();
			memset(byAscii, 0, sizeof(byAscii));
			j = 0;
		}
	}

	if(j)
	{
		for(; j<16; j++)
		{
			strDumpData += _T("   ");

			if(j && (j%8)==0)
				strDumpData += _T(" ");
		}

		//	strTemp.Format(_T("%-16.16S"),byAscii);
		//	strDumpData += strTemp;
		strDumpData += (CString)byAscii;

		strDumpData.Replace(_T("%"), _T("%%"));
		LOG(Info, _T("%s"), strDumpData);
	}
}


/** **********************************************************
*	@brief		StringHexaDump  : Host Trace Data를 Logging하는 기능
*	@param		CString strData : Data
*	@retval		없음
************************************************************/
void CEagleSVCLib::StringHexaDump(CString strData)
{
	CString strTemp, strDumpData;
	BYTE	byAscii[256] = { 0, };
	int		i=0, j=0, nLen=0;
	CHAR	*pData = NULL;

	nLen = WideCharToMultiByte(CP_ACP, 0, strData, -1, NULL, 0, NULL, NULL);

	pData = new CHAR[nLen + 1];

	if (pData != NULL)
	{
		memset(pData, 0, (nLen + 1));
		WideCharToMultiByte(CP_ACP, 0, strData, -1, pData, nLen, NULL, NULL);
	}

	for(i=0; i<nLen; i++)
	{
		if((i%16)==0)   
		{
			strTemp.Format(_T("%05d:%05d "),i,i+16);
			strDumpData += strTemp;
		}

		strTemp.Format(_T("%02x "),pData[i]);
		strDumpData += strTemp;

		if(j==7)	strDumpData += _T(" ");

		if(pData[i]>=0x20 && pData[i] <= 0x7e)
			byAscii[j] = pData[i];
		else    byAscii[j] = '.';

		j++;

		if(j==16)
		{
			//strTemp.Format(_T("%-16.16S"),byAscii);
			//strDumpData += strTemp;
			strTemp = byAscii;
			strDumpData += strTemp;

			strDumpData.Replace(_T("%"), _T("%%"));
			LOG(Info, _T("%s"), strDumpData);

			strDumpData.Empty();
			memset(byAscii, 0, sizeof(byAscii));
			j = 0;
		}
	}

	if(j)
	{
		for(; j<16; j++)
		{
			strDumpData += _T("   ");

			if(j && (j%8)==0)
				strDumpData += _T(" ");
		}

		//strTemp.Format(_T("%-16.16S"),byAscii);
		//strDumpData += strTemp;
		strTemp = byAscii;
		strDumpData += strTemp;

		strDumpData.Replace(_T("%"), _T("%%"));
		LOG(Info, _T("%s"), strDumpData);
	}

	delete [] pData;
}


/** **********************************************************
*	@brief		Recovery_ErrorDevice  : 장애난 Device의 Error Recovery 수행
*	@retval		없음
************************************************************/
BOOL CEagleSVCLib::Recovery_ErrorDevice(int nErrorDevice, BOOL bCBXStatusClear)
{
	BOOL	bResult = TRUE;

	LOG(Info, _T("Recovery Error Device Start (%x)"), nErrorDevice);

	// Error를 모두 Clear 후에 Error Code 재 Setting
	Clear_Error();

	// CARD READER
	if (nErrorDevice & ERROR_DEVICE_IDC)
	{
		LOG(Info, _T("Recovery IDC Device Start"));

		if (CSVC_Manager::GetInstance()->m_CDR_Connect == TRUE)
		{
			if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CDR.Initialize())
			{
				CString strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();

				LOG(Error, _T("Recovery Initialize - CDR - Error Code : ") + strErrorCode);

				// 에러 코드 설정
				Set_ErrorCode(strErrorCode, ERROR_DEVICE_IDC);
				bResult = FALSE;
			}
			else
			{
				CDEV_Manager::GetInstance()->m_DEV_CDR.GetVersionInfo();

				LOG(Info, _T("Recovery Initialize - CDR - Succeeded"));
			}
		}
		else
		{
			Set_ErrorCode(ERROR_IDC_PORTOPEN, ERROR_DEVICE_IDC);
			bResult = FALSE;
		}
	}

	// CDM
	if (nErrorDevice & ERROR_DEVICE_CDM)
	{
		if (CSVC_Manager::GetInstance()->m_CDM_Connect == TRUE)
		{
			CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStop();

			if(FALSE == CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_Initialize(bCBXStatusClear))
			{
				CEagleSVCLib::GetInstance()->Set_ErrorCode(CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError(), ERROR_DEVICE_CDM);
				LOG(Error, _T("Recovery Reset - CDM - Failed - Reset (%s)"), CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError());

				bResult = FALSE;
			}
			else
			{
				CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_CfgStatus();	// Version Read를 위해 추가로 Call

				LOG(Info, _T("Recovery Initialize - CDM - Succeeded"));
			}

			CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStart();
		}
		else
		{
			Set_ErrorCode(ERROR_CDM_PORTOPEN, ERROR_DEVICE_CDM);
			bResult = FALSE;
		}
	}

	// EPP
	if (nErrorDevice & ERROR_DEVICE_EPP)
	{
		if (CSVC_Manager::GetInstance()->m_EPP_Connect == FALSE)
		{
			Set_ErrorCode(ERROR_EPP_PORTOPEN, ERROR_DEVICE_EPP);
			bResult = FALSE;
		}
	}

	// PRT
	if (nErrorDevice & ERROR_DEVICE_PTR)
	{
		if (CSVC_Manager::GetInstance()->m_PRT_Connect == TRUE)
		{
			int nRet = 0;
			
			// PTR Quiry Status Stop
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();	// Polling Start 로직 누락 추가

			nRet = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_Reset();

			// PTR Quiry Status Start
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();	// Polling Start 로직 누락 추가

			if ( nRet !=  EAGLE_PRT_SUCCESS)
			{
				LOG(Error, _T("Initialize - PRT - Failed - Reset - %d"), nRet);

				// PTR은 Error Code를 Setting하지 않음
			}
			else
			{
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetVersionInfo();

				LOG(Info, _T("Initialize - PRT - Succeeded - Reset - %d"), nRet);
			}
		}
		else
		{
			Set_ErrorCode(ERROR_PTR_PORTOPEN, ERROR_DEVICE_PTR);
			bResult = FALSE;
		}
	}

	LOG(Info, _T("Recovery Error Device End - Error Device(%x) Result (%d)"), CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice, bResult);

	return bResult;
}


/** **********************************************************
*	@brief		Set_PTR_Config  : Printer 초기 Config Set
*	@retval		없음
************************************************************/
BOOL CEagleSVCLib::Set_PTR_Config()
{
	BOOL	bResult = TRUE;
	int		nPrintError = 0;

	// Mode set
	nPrintError = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_SetMode(EAGLE_PRT_TYPE_USB, EAGLE_PRT_MODE_STANDARD/*EAGLE_PRT_MODE_STANDARD*/);
	if (EAGLE_PRT_SUCCESS != nPrintError)
	{
		bResult = FALSE;
		LOG(Error, _T("PRT - USB_SetMode Failed - %d"), nPrintError);
	}

	nPrintError = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_SetRightSpacing(EAGLE_PRT_TYPE_USB, 0 /* Set dot number of right margin*/);
	if(EAGLE_PRT_SUCCESS != nPrintError)
	{
		bResult = FALSE;
		LOG(Info, _T("PTR_SetRightSpacing is failed (%d"), nPrintError);
	}

	nPrintError = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_SetLineSpacing(EAGLE_PRT_TYPE_USB, 40 /* Set dot numbers of line */);
	if(EAGLE_PRT_SUCCESS != nPrintError)
	{
		bResult = TRUE;
		LOG(Info, _T("PTR_SetLineSpacing is failed (%d)"), nPrintError);
	}

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", 40, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

	return bResult;
}


/** **********************************************************
*	@brief		SendRecvHost  : OP에서 Host 연결시 사용
*	@param		int nTransType : Transaction Type
*	@retval		성공시 : 0, 실패시 : failed code
************************************************************/
int CEagleSVCLib::SendRecvHost(int nTransType)
{
	int nRetryCnt = 0, nResult = 0;
	CString strTransType, strTemp;

#if (NETWORK_OFFLINE_MODE)
	return SUCCESS;
#endif

	// Retry 3 times
	while(nRetryCnt < 3)
	{
		if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
		{
			// TCP/IP
			if ((nRetryCnt % 2) == 0)
			{
				// Primary IP
				nResult = CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Sync_Send(CEagleTritonMsg::GetInstance()->m_pSendBuffer, CEagleTritonMsg::GetInstance()->m_nSendLen,
																				 CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip, CEagleDataManager::GetInstance()->m_Config.m_Host.strPort,
																				 CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS);
			}
			else
			{
				strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_Hostip;
				strTemp.TrimRight();

				if (strTemp.IsEmpty() == FALSE)
				{
					// Backup IP
					nResult = CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Sync_Send(CEagleTritonMsg::GetInstance()->m_pSendBuffer, CEagleTritonMsg::GetInstance()->m_nSendLen,
																					 CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_Hostip, CEagleDataManager::GetInstance()->m_Config.m_Host.strBackupPort,
																					 CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS);
				}
				else
				{
					// Primary IP
					nResult = CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Sync_Send(CEagleTritonMsg::GetInstance()->m_pSendBuffer, CEagleTritonMsg::GetInstance()->m_nSendLen,
																					 CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip, CEagleDataManager::GetInstance()->m_Config.m_Host.strPort,
																					 CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS);
				}
			}
		}
		else
		{
			// Modem
			if ((nRetryCnt % 2) == 0)
			{
				LOG(Info, _T("Primary Phone Number is %s"), CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number);

				// Primary phone number
				nResult = CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Sync_Send(CEagleTritonMsg::GetInstance()->m_pSendBuffer, CEagleTritonMsg::GetInstance()->m_nSendLen,
																				 CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number, _T("0"), _T("0"));
			}
			else
			{
				strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_phone_number;
				strTemp.TrimRight();

				LOG(Info, _T("Back Up Phone Number is %s"), strTemp);

				if (strTemp.IsEmpty() == FALSE)
				{
					// Backup phone number
					nResult = CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Sync_Send(CEagleTritonMsg::GetInstance()->m_pSendBuffer, CEagleTritonMsg::GetInstance()->m_nSendLen,
																					 CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_phone_number, _T("0"), _T("0"));
				}
				else
				{
					// Primary phone number
					nResult = CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Sync_Send(CEagleTritonMsg::GetInstance()->m_pSendBuffer, CEagleTritonMsg::GetInstance()->m_nSendLen,
																					 CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number, _T("0"), _T("0"));
				}
			}
		}

		// 통신 성공이거나 DATA 송신 후 실패시에는 Retry 수행 안함
		if ((nResult == SUCCESS) || (nResult > RECV_FAIL_ENQ))
			break;

		nRetryCnt++;
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_3000MS);	// 3초 후 Retry 수행
	}

	memset(CEagleTritonMsg::GetInstance()->m_pRecvBuffer, 0, NET_RECV_BUFF_SIZE);
	CEagleTritonMsg::GetInstance()->m_nRecvLen = 0;

	// [PCI-SSF Fix] m_pRecvBuffer 실제 용량(NET_RECV_BUFF_SIZE) 전달 - 오버플로우 방지
	CEagleTritonMsg::GetInstance()->m_nRecvLen = CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Sync_Recv(CEagleTritonMsg::GetInstance()->m_pRecvBuffer, NET_RECV_BUFF_SIZE);

	if (CEagleTritonMsg::GetInstance()->m_nRecvLen > 0)
	{
		LOG(Info, _T("------------------------RECV DATA-----------------------"));
		CEagleSVCLib::GetInstance()->HexaDump(CEagleTritonMsg::GetInstance()->m_pRecvBuffer, CEagleTritonMsg::GetInstance()->m_nRecvLen);
		LOG(Info, _T("--------------------------------------------------------"));
	}
	else
	{
		LOG(Info, _T("Receive Data is not existed"));
	}

	if (nResult == SUCCESS)
	{
		nResult = CEagleTritonMsg::GetInstance()->Triton_AnalHostData(nTransType);

		// 수신 정상시 Config 정보 저장하도록 로직 공용화

		// Save file of transaction response message
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bRecvConfigInfo == TRUE)
		{
			LOG(Info, _T("Save Host, Message, Option Config Files - Start"));
			//CEagleDataManager::GetInstance()->m_Config.Save();
			CEagleDataManager::GetInstance()->m_Config.SaveHost();
			CEagleDataManager::GetInstance()->m_Config.SaveMessage();
			CEagleDataManager::GetInstance()->m_Config.SaveOption();
			LOG(Info, _T("Save Host, Message, Option Config Files - End"));
		}
	}
	else
	{
		if (nResult == RECV_FAIL_EOT)
			CEagleTritonMsg::GetInstance()->Triton_AnalHostData(nTransType);

		// 통신 장애시 Error Code Setting
		switch(nResult)
		{
		case CONNECT_FAIL:
			Set_ErrorCode(ERROR_NET_CONNECT_FAIL, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(_T("CONNECT_FAIL")));
			break;

		case RECV_FAIL_ENQ:
			Set_ErrorCode(ERROR_NET_ENQ_FAIL, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(_T("COMM_ERROR")));
			break;

		case RECV_FAIL_ACK:
			Set_ErrorCode(ERROR_NET_ACK_FAIL, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(_T("COMM_ERROR")));
			break;

		case RECV_FAIL_DATA:
			Set_ErrorCode(ERROR_NET_DATA_FAIL, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(_T("COMM_ERROR")));
			break;

		case RECV_FAIL_EOT:
			Set_ErrorCode(ERROR_NET_EOT_FAIL, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(_T("COMM_ERROR")));
			break;

		default:
			Set_ErrorCode(ERROR_NET_EOT_FAIL, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(_T("COMM_ERROR")));
			break;
		}
	}

	// 확인 차원에서 다시한번 Line Close 수행
	CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Sync_CloseLine();

	return nResult;
}


/** **********************************************************
*	@brief		Print_DayClose  : Day Close 후 수행결과를 명세표에 Print하는 함수
*	@param		int nType : Day Close Type (Trial Day Close or Day Close)
*	@param		BOOL nResult : Host와 Day Close 통신 수행 결과값
*	@param		BOOL bPrint : Print 출력 여부
*	@retval		성공시 : TRUE, 실패시 FALSE
************************************************************/
BOOL CEagleSVCLib::SaveEJL_N_Print_DayClose(int nType, BOOL bResult, BOOL bPrint)
{
	// 영수증 출력 장애 여부 확인
	CStringA	szTemp = "";
	CStringA	szEJLTemp;
	CString		strTemp, strTemp2, strTemp3;
	CString		strTransType;
	int			nOrgX = 40;
	int			nLineCount = 0;

	SYSTEMTIME	st;
	GetLocalTime(&st);

	// EJL
	FLSEJCOMMINFO	CommInfo;
	FLSEJDAYCLINFO	DayCloseInfo;

	memset(&CommInfo, 0, sizeof(CommInfo));
	memset(&DayCloseInfo, 0, sizeof(DayCloseInfo));

	sprintf(CommInfo.log_date, "%04d/%02d/%02d", st.wYear, st.wMonth, st.wDay);

	sprintf(CommInfo.log_time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);

	CommInfo.jnl_cd = day_close;

	sprintf(CommInfo.log_srch_date, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);


	if (bPrint == TRUE)
	{
		// PTR Quiry Status Stop
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();

		// 프린터 램프 ON
		CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_FLICKING);

		if (CEagleSVCLib::GetInstance()->Set_PTR_Config() == FALSE)
		{
			LOG(Error, _T("Set PTR Config is failed"));
		}

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;

		if (nType == EAGLE_TRAN_CODE_HOST_TOTALS)
		{
			szTemp.Format("*** DAY CLOSE ***");
		}
		else
		{
			szTemp.Format("*** TRIAL DAY CLOSE ***");
		}

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;

		// 2. START
		szTemp.Format("START : %02d/%02d/%04d %02d:%02d:%02d", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;
	}

	// 1. Title
	if (nType == EAGLE_TRAN_CODE_HOST_TOTALS)
	{
		DayCloseInfo.day_close_type[0] = '1';
	}
	else
	{
		DayCloseInfo.day_close_type[0] = '0';
	}

	// [EJL]
	szEJLTemp.Format("%02d/%02d/%04d", st.wMonth, st.wDay, st.wYear);
	sprintf(DayCloseInfo.start_date, "%s", szEJLTemp);
	szEJLTemp.Format("%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
	sprintf(DayCloseInfo.start_time, "%s", szEJLTemp);

	// 3. TERMINAL ID
	if (bPrint == TRUE)
	{
		szTemp.Format("TERMINAL ID : %S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;
	}

	// [EJL]
	szEJLTemp.Format("%S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
	sprintf(DayCloseInfo.term_id, "%s", szEJLTemp);

	// [EJL]
	if (bResult == TRUE)
		DayCloseInfo.day_close_Result[0] = '1';
	else
		DayCloseInfo.day_close_Result[0] = '0';

	// HOST
	if (bResult == TRUE)
	{
		if (bPrint == TRUE)
		{
			// Host 수신시에만 Print
			// 5. HOST
			szTemp.Format("  ** HOST **");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    WITHDRAWALS = %d", CUtil::StringToInt(CEagleTritonMsg::GetInstance()->m_sTriton_TotalResp.m_strNumberOfWithdrawals_4));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}


		// [EJL]
		szEJLTemp.Format("%d", CUtil::StringToInt(CEagleTritonMsg::GetInstance()->m_sTriton_TotalResp.m_strNumberOfWithdrawals_4));
		sprintf(DayCloseInfo.host_with_cnt, "%s", szEJLTemp);

		if (bPrint == TRUE)
		{
			szTemp.Format("    BALANCE = %d", CUtil::StringToInt(CEagleTritonMsg::GetInstance()->m_sTriton_TotalResp.m_strNumberOfInquiries_4));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}

		// [EJL]
		szEJLTemp.Format("%d", CUtil::StringToInt(CEagleTritonMsg::GetInstance()->m_sTriton_TotalResp.m_strNumberOfInquiries_4));
		sprintf(DayCloseInfo.host_bal_cnt, "%s", szEJLTemp);

		if (bPrint == TRUE)
		{
			szTemp.Format("    TRANSFER = %d", CUtil::StringToInt(CEagleTritonMsg::GetInstance()->m_sTriton_TotalResp.m_strNumberOfTransfers_4));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}

		// [EJL]
		szEJLTemp.Format("%d", CUtil::StringToInt(CEagleTritonMsg::GetInstance()->m_sTriton_TotalResp.m_strNumberOfTransfers_4));
		sprintf(DayCloseInfo.host_trans_cnt, "%s", szEJLTemp);

		// Extended settlement값이 오는 경우 Extended settlement로 처리
		if (CEagleTritonMsg::GetInstance()->m_sTriton_TotalResp.m_strSettlement_Ext12.IsEmpty() == TRUE)
		{
			szTemp.Format("    SETTLEMENT = %S%S", CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(CEagleTritonMsg::GetInstance()->m_sTriton_TotalResp.m_strSettlement_8));

			// [EJL]
			szEJLTemp.Format("%d", CUtil::StringToInt(CEagleTritonMsg::GetInstance()->m_sTriton_TotalResp.m_strSettlement_8));
		}
		else
		{
			szTemp.Format("    SETTLEMENT = %S%S", CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(CEagleTritonMsg::GetInstance()->m_sTriton_TotalResp.m_strSettlement_Ext12));

			// [EJL]
			szEJLTemp.Format("%d", CUtil::StringToInt(CEagleTritonMsg::GetInstance()->m_sTriton_TotalResp.m_strSettlement_Ext12));
		}

		if (bPrint == TRUE)
		{
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}

		// [EJL]
		sprintf(DayCloseInfo.total_host_amt, "%s", szEJLTemp);

		if (bPrint == TRUE)
		{
			szTemp.Format("----------------------------------------");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}
	}
	else
	{
		if (bPrint == TRUE)
		{
			szTemp.Format("  ** HOST **");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    FAILED TO GET DATA FROM HOST");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("----------------------------------------");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}
	}

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Error, _T("PRT - %S"), szTemp);
	nLineCount++;

	if (bPrint == TRUE)
	{
		// TERMINAL
		// 5. TERMINAL
		szTemp.Format("  ** TERMINAL **");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;


		szTemp.Format("    WITHDRAWALS = %d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.Withdrawal_Count));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;
	}

	// [EJL]
	szEJLTemp.Format("%d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.Withdrawal_Count));
	sprintf(DayCloseInfo.term_with_cnt, "%s", szEJLTemp);

	if (bPrint == TRUE)
	{
		szTemp.Format("    BALANCE = %d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.BalanceInquiry_Count));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;
	}

	// [EJL]
	szEJLTemp.Format("%d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.BalanceInquiry_Count));
	sprintf(DayCloseInfo.term_bal_cnt, "%s", szEJLTemp);

	if (bPrint == TRUE)
	{
		szTemp.Format("    TRANSFER = %d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.Transfer_Count));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;
	}

	// [EJL]
	szEJLTemp.Format("%d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.Transfer_Count));
	sprintf(DayCloseInfo.term_trans_cnt, "%s", szEJLTemp);

	szTemp.Format("    SETTLEMENT = %S%S", CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.Withdrawal_Amount + _T("00")));	// Cent 추가

	// [EJL]
	szEJLTemp.Format("%S", CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.Withdrawal_Amount + _T("00"));
	sprintf(DayCloseInfo.total_term_amt, "%s", szEJLTemp);

	if (bPrint == TRUE)
	{
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;

		szTemp.Format("----------------------------------------");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;
	}

	// [EJL]
	BOOL bEJLResult = FALSE;
	bEJLResult = CDEV_Manager::GetInstance()->m_DEV_EJL.InsertJnlData(&CommInfo, &DayCloseInfo);
	LOG(Info, _T("Day Close InsertJnlData Result (%d)"), bEJLResult);

	// 거래 건수 Clear
	if (nType == EAGLE_TRAN_CODE_HOST_TOTALS)
	{
		CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.Clear();
		CEagleDataManager::GetInstance()->m_Config.SaveTotalInfo();
	}

	if (bPrint == TRUE)
	{
		if (nLineCount< MINIMUM_PRIT_LINE)
		{
			for(int i=nLineCount; i<MINIMUM_PRIT_LINE; i++)
			{
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			}
		}

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_CutPaper(EAGLE_PRT_TYPE_USB, 1, 0);

		// PTR Quiry Status Stop
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();

		// 프린터 램프 OFF
		CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_OFF);

		if (IsPTR_Available(TRUE) == FALSE)
			return FALSE;

	}

	return TRUE;
}


/** **********************************************************
*	@brief		SaveEJL_N_Print_CBXClose  : CBX Close 후 수행결과를 명세표에 Print하는 함수
*	@param		int nType : CBX Close Type (Trial CBX Close or CBX Close)
*	@retval		성공시 : TRUE, 실패시 FALSE
************************************************************/
BOOL CEagleSVCLib::SaveEJL_N_Print_CBXClose(int nType)
{
	// 영수증 출력 장애 여부 확인
	CStringA	szTemp = "";
	CString		strTemp, strTemp2, strTemp3;
	CString		strTransType;
	int			nOrgX = 40;
	int			nCSTCnt = 0, nCSTDenom = 0, nCSTAmount = 0, nTemp = 0;
	CString		strLoadedAmount, strDispensedAmount, strCurrentAmount;
	int			nLineCount = 0;

	SYSTEMTIME	st;
	GetLocalTime(&st);

	// [EJL]
	FLSEJCOMMINFO	CommInfo;
	FLSEJCSTCLINFO	CBXCloseInfo;

	memset(&CommInfo, 0, sizeof(CommInfo));
	memset(&CBXCloseInfo, 0, sizeof(CBXCloseInfo));

	sprintf(CommInfo.log_date, "%04d/%02d/%02d", st.wYear, st.wMonth, st.wDay);

	sprintf(CommInfo.log_time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);

	CommInfo.jnl_cd = cst_close;

	sprintf(CommInfo.log_srch_date, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);


	nCSTCnt = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

	// Printer Status 확인 후 인자 여부 결정
	BOOL bAvailPTRStatus = IsPTR_Available();

	// 프린터 램프 ON
	// PTR Quiry Status Stop
	if (bAvailPTRStatus == TRUE)
	{
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();

		CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_FLICKING);

		if (CEagleSVCLib::GetInstance()->Set_PTR_Config() == FALSE)
		{
			LOG(Error, _T("Set PTR Config is failed"));
		}
	}

	// 1. Title
	if (nType == EAGLE_TRAN_CODE_CBX_TOTALS)
	{
		szTemp.Format("*** CBX CLOSE ***");

		// [EJL]
		CBXCloseInfo.cst_close_type[0] = '1';
	}
	else
	{
		szTemp.Format("*** TRIAL CBX CLOSE ***");

		// [EJL]
		CBXCloseInfo.cst_close_type[0] = '0';
	}

	if (bAvailPTRStatus == TRUE)
	{
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;

		// 2. START
		szTemp.Format("START : %02d/%02d/%04d %02d:%02d:%02d", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;
	}

	// [EJL]
	sprintf(CBXCloseInfo.start_date, "%02d/%02d/%04d", st.wMonth, st.wDay, st.wYear);

	// [EJL]
	sprintf(CBXCloseInfo.start_time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);

	// 3. TERMINAL ID
	if (bAvailPTRStatus == TRUE)
	{
		szTemp.Format("TERMINAL ID : %S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
	}

	// [EJL]
	sprintf(CBXCloseInfo.term_id, "%S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);

	// 4. Linefeed
	if (bAvailPTRStatus == TRUE)
	{
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		nLineCount++;
	}

	// CST 개수 저장
	szTemp.Format("%d", nCSTCnt);
	sprintf(CBXCloseInfo.cst_count, "%s", szTemp);

	for (int i=0; i<nCSTCnt; i++)
	{
		// Denomination information
		nCSTDenom = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i]); 

		// [EJL]
		switch(i)
		{
		case 0:
			sprintf(CBXCloseInfo.denom_cst1, "%d", nCSTDenom);
			break;

		case 1:
			sprintf(CBXCloseInfo.denom_cst2, "%d", nCSTDenom);
			break;

		case 2:
			sprintf(CBXCloseInfo.denom_cst3, "%d", nCSTDenom);
			break;

		case 3:
			sprintf(CBXCloseInfo.denom_cst4, "%d", nCSTDenom);
			break;
		}

		if (bAvailPTRStatus == TRUE)
		{
			szTemp.Format("  ** CBX #%d **", i+1);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}

		nTemp = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Load_Count[i]);
		strLoadedAmount = CUtil::IntToString(nCSTDenom * nTemp) + _T("00");

		if (bAvailPTRStatus == TRUE)
		{
			szTemp.Format("    LOADED NUMBER OF BILLS = %S", CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Load_Count[i], FALSE));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}

		//[EJL]
		switch(i)
		{
		case 0:
			sprintf(CBXCloseInfo.load_bill_cst1, "%d", nTemp);
			sprintf(CBXCloseInfo.load_amt_cst1, "%S", strLoadedAmount);
			break;

		case 1:
			sprintf(CBXCloseInfo.load_bill_cst2, "%d", nTemp);
			sprintf(CBXCloseInfo.load_amt_cst2, "%S", strLoadedAmount);
			break;

		case 2:
			sprintf(CBXCloseInfo.load_bill_cst3, "%d", nTemp);
			sprintf(CBXCloseInfo.load_amt_cst3, "%S", strLoadedAmount);
			break;

		case 3:
			sprintf(CBXCloseInfo.load_bill_cst4, "%d", nTemp);
			sprintf(CBXCloseInfo.load_amt_cst4, "%S", strLoadedAmount);
			break;
		}

		nTemp = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[i]);
		strDispensedAmount = CUtil::IntToString(nCSTDenom * nTemp) + _T("00");

		if (bAvailPTRStatus == TRUE)
		{
			szTemp.Format("    DISPENSED NUMBER OF BILLS = %S", CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[i], FALSE));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}

		//[EJL]
		switch(i)
		{
		case 0:
			sprintf(CBXCloseInfo.disp_bill_cst1, "%d", nTemp);
			sprintf(CBXCloseInfo.disp_amt_cst1, "%S", strDispensedAmount);
			break;

		case 1:
			sprintf(CBXCloseInfo.disp_bill_cst2, "%d", nTemp);
			sprintf(CBXCloseInfo.disp_amt_cst2, "%S", strDispensedAmount);
			break;

		case 2:
			sprintf(CBXCloseInfo.disp_bill_cst3, "%d", nTemp);
			sprintf(CBXCloseInfo.disp_amt_cst3, "%S", strDispensedAmount);
			break;

		case 3:
			sprintf(CBXCloseInfo.disp_bill_cst4, "%d", nTemp);
			sprintf(CBXCloseInfo.disp_amt_cst4, "%S", strDispensedAmount);
			break;
		}

		nTemp = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[i]);

		if (bAvailPTRStatus == TRUE)
		{
			szTemp.Format("    REJECTED COUNT = %d", nTemp);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}

		//[EJL]
		switch(i)
		{
		case 0:
			sprintf(CBXCloseInfo.rej_bill_cst1, "%d", nTemp);
			break;

		case 1:
			sprintf(CBXCloseInfo.rej_bill_cst2, "%d", nTemp);
			break;

		case 2:
			sprintf(CBXCloseInfo.rej_bill_cst3, "%d", nTemp);
			break;

		case 3:
			sprintf(CBXCloseInfo.rej_bill_cst4, "%d", nTemp);
			break;
		}

		nTemp = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);
		strCurrentAmount = CUtil::IntToString(nCSTDenom * nTemp) + _T("00");

		if (bAvailPTRStatus == TRUE)
		{
			szTemp.Format("    CURRENT NUMBER OF BILLS = %S", CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i], FALSE));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}

		//[EJL]
		switch(i)
		{
		case 0:
			sprintf(CBXCloseInfo.rem_bill_cst1, "%d", nTemp);
			sprintf(CBXCloseInfo.rem_amt_cst1, "%S", strCurrentAmount);
			break;

		case 1:
			sprintf(CBXCloseInfo.rem_bill_cst2, "%d", nTemp);
			sprintf(CBXCloseInfo.rem_amt_cst2, "%S", strCurrentAmount);
			break;

		case 2:
			sprintf(CBXCloseInfo.rem_bill_cst3, "%d", nTemp);
			sprintf(CBXCloseInfo.rem_amt_cst3, "%S", strCurrentAmount);
			break;

		case 3:
			sprintf(CBXCloseInfo.rem_bill_cst4, "%d", nTemp);
			sprintf(CBXCloseInfo.rem_amt_cst4, "%S", strCurrentAmount);
			break;
		}

		if (bAvailPTRStatus == TRUE)
		{
			szTemp.Format("    LOADED AMOUNT = %S%S", CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strLoadedAmount));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    DISPENSED AMOUNT = %S%S", CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strDispensedAmount));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    CURRENT AMOUNT = %S%S", CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strCurrentAmount));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    VALUE OF EACH NOTE = %S%d", CURRENCY_SYMBOL, nCSTDenom);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    ------------------------------------");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			nLineCount++;
		}
	}

	// [EJL] SAVE
	BOOL bResult = FALSE;
	LOG(Info, _T("Save CBX Close start"));
	bResult = CDEV_Manager::GetInstance()->m_DEV_EJL.InsertJnlData(&CommInfo, &CBXCloseInfo);

	LOG(Info, _T("Save CBX Close Reuslt : %d"), bResult);

	if (nLineCount< MINIMUM_PRIT_LINE)
	{
		for(int i=nLineCount; i<MINIMUM_PRIT_LINE; i++)
		{
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		}
	}

	if (bAvailPTRStatus == TRUE)
	{
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_CutPaper(EAGLE_PRT_TYPE_USB, 1, 0);

		// PTR Quiry Status Stop
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();

		// 프린터 램프 OFF
		CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_OFF);
	}

	// 거래 건수 Clear
	if (nType == EAGLE_TRAN_CODE_CBX_TOTALS)
	{
		// 매수 Clear
		CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.Clear_CBX_Count();
		//CEagleDataManager::GetInstance()->m_Config.SaveDevice();
		CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.Clear_Reject_Info();
		CEagleDataManager::GetInstance()->m_Config.SaveCbxInfo();
	}

	if (IsPTR_Available(TRUE) == FALSE)
		return FALSE;

	return TRUE;
}


/** **********************************************************
*	@brief		SaveEJL_N_Print_CBXAddQTY  : CBX Add 후 수행결과를 명세표에 Print하는 함수
*	@param		int nAdded_Count : CBX Added한 number of bills
*	@retval		성공시 : TRUE, 실패시 FALSE
************************************************************/
BOOL CEagleSVCLib::SaveEJL_N_Print_CBXAddQTY(int nAdded_Count[MAX_CST_COUNT], BOOL bPrint)
{
	LOG(Info, _T("Print CBX ADD QTY - start"));
	CStringA	szTemp, szEJLTemp;
	int			nCSTCnt = 0;
	int			nOrgX = 40;
	int			nCSTIndex = 0;
	int			nLineCount = 0;

	SYSTEMTIME	st;
	GetLocalTime(&st);

	// Common Info
	FLSEJCOMMINFO commInfo;

	memset(&commInfo, 0, sizeof(commInfo));

	sprintf(commInfo.log_date, "%04d/%02d/%02d", st.wYear, st.wMonth, st.wDay);
	sprintf(commInfo.log_time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
	commInfo.jnl_cd = add_bill;
	sprintf(commInfo.log_srch_date, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	FLSEJADDBILLINFO	Addbill_info;

	memset(&Addbill_info, 0, sizeof(Addbill_info));

	nCSTCnt = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

	if (bPrint == TRUE)
	{
		// PTR Quiry Status Stop
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();

		// 프린터 램프 ON
		CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_FLICKING);

		if (CEagleSVCLib::GetInstance()->Set_PTR_Config() == FALSE)
		{
			LOG(Error, _T("Set PTR Config is failed"));
		}

		// 1. Title
		szTemp.Format("*** ADD QUANTITY IN CBX ***");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;

		// 2. START
		szTemp.Format("  START : %02d/%02d/%04d %02d:%02d:%02d", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;
	}

	// [EJL] Start Date
	szEJLTemp.Format("%02d/%02d/%04d", st.wMonth, st.wDay, st.wYear);
	sprintf(Addbill_info.start_date, "%s", szEJLTemp);
	LOG(Info, _T("START DATE : %S"), szEJLTemp);

	// [EJL] Start Time
	szEJLTemp.Format("%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
	sprintf(Addbill_info.start_time, "%s", szEJLTemp);
	LOG(Info, _T("START TIME : %S"), szEJLTemp);


	// 3. TERMINAL ID
	if (bPrint == TRUE)
	{
		szTemp.Format("  TERMINAL ID : %S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
		nLineCount++;
	}

	// [EJL] Terminal ID
	szEJLTemp.Format("%S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
	sprintf(Addbill_info.term_id, "%s", szEJLTemp);

	// 4. Linefeed
	if (bPrint == TRUE)
	{
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		nLineCount++;
	}

	// CST 개수 저장
	szEJLTemp.Format("%d", nCSTCnt);
	sprintf(Addbill_info.cst_count, "%s", szEJLTemp);

	if (nCSTCnt >= 1)
	{
		if (bPrint == TRUE)
		{
			szTemp.Format("  ** CBX #1 **");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    ADDED NUMBER OF BILLS = %S", CUtil::ConvertFromValueToAmountwithCent(CUtil::IntToString(nAdded_Count[nCSTIndex]), FALSE));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    CURRENT NUMBER OF BILLS = %S", CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[nCSTIndex], FALSE));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}

		// [EJL]
		szEJLTemp.Format("%d", nAdded_Count[nCSTIndex]);
		sprintf(Addbill_info.add_bill_cst1, szEJLTemp);

		szEJLTemp.Format("%d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[nCSTIndex]));
		sprintf(Addbill_info.remain_cst1, szEJLTemp);

		if (bPrint == TRUE)
		{
			// 4. Linefeed
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			nLineCount++;
		}
	}

	nCSTIndex++;

	if (nCSTCnt >= 2)
	{
		if (bPrint == TRUE)
		{
			szTemp.Format("  ** CBX #2 **");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    ADDED NUMBER OF BILLS = %S", CUtil::ConvertFromValueToAmountwithCent(CUtil::IntToString(nAdded_Count[nCSTIndex]), FALSE));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    CURRENT NUMBER OF BILLS = %S", CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[nCSTIndex], FALSE));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}

		// [EJL]
		szEJLTemp.Format("%d", nAdded_Count[nCSTIndex]);
		sprintf(Addbill_info.add_bill_cst2, szEJLTemp);

		szEJLTemp.Format("%d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[nCSTIndex]));
		sprintf(Addbill_info.remain_cst2, szEJLTemp);

		if (bPrint == TRUE)
		{
			// 4. Linefeed
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			nLineCount++;
		}
	}

	nCSTIndex++;

	if (nCSTCnt >= 3)
	{
		if (bPrint == TRUE)
		{
			szTemp.Format("  ** CBX #3 **");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    ADDED NUMBER OF BILLS = %S", CUtil::ConvertFromValueToAmountwithCent(CUtil::IntToString(nAdded_Count[nCSTIndex]), FALSE));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    CURRENT NUMBER OF BILLS = %S", CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[nCSTIndex], FALSE));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}

		// [EJL]
		szEJLTemp.Format("%d", nAdded_Count[nCSTIndex]);
		sprintf(Addbill_info.add_bill_cst3, szEJLTemp);

		szEJLTemp.Format("%d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[nCSTIndex]));
		sprintf(Addbill_info.remain_cst3, szEJLTemp);


		if (bPrint == TRUE)
		{
			// 4. Linefeed
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			nLineCount++;
		}
	}

	nCSTIndex++;

	if (nCSTCnt >= 4)
	{
		if (bPrint == TRUE)
		{
			szTemp.Format("  ** CBX #4 **");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    ADDED NUMBER OF BILLS = %S", CUtil::ConvertFromValueToAmountwithCent(CUtil::IntToString(nAdded_Count[nCSTIndex]), FALSE));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;

			szTemp.Format("    CURRENT NUMBER OF BILLS = %S", CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[nCSTIndex], FALSE));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}

		// [EJL]
		szEJLTemp.Format("%d", nAdded_Count[nCSTIndex]);
		sprintf(Addbill_info.add_bill_cst4, szEJLTemp);

		szEJLTemp.Format("%d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[nCSTIndex]));
		sprintf(Addbill_info.remain_cst4, szEJLTemp);

		if (bPrint == TRUE)
		{
			// 4. Linefeed
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			nLineCount++;
		}
	}

	// EJL SAVE
	if (CDEV_Manager::GetInstance()->m_DEV_EJL.InsertJnlData(&commInfo, &Addbill_info) == TRUE)
	{
		LOG(Info, (_T("ADD QTY SAVE EJL SUCCESS")));
	}
	else
	{
		LOG(Info, (_T("ADD QTY SAVE EJL FAILED (ERROR : %s"), CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastError()));
	}

	if (bPrint == TRUE)
	{
		// 최소 명세표 여백 보정 처리
		if (nLineCount < MINIMUM_PRIT_LINE)
		{
			for(int i=nLineCount; i<MINIMUM_PRIT_LINE; i++)
			{
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			}
		}

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_CutPaper(EAGLE_PRT_TYPE_USB, 1, 0);

		// PTR Quiry Status Stop
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();

		// 프린터 램프 OFF
		CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_OFF);

		if (IsPTR_Available(TRUE) == FALSE)
			return FALSE;
	}

	return TRUE;
}


/** **********************************************************
*	@brief		SaveEJL_SetDenomination  : Set Denomination 수행결과를 명세표에 Print하는 함수
*	@param		int nOldDenom : 이전 Denomination값
*	@param		int nNewDenom : 변경한 Denomination값
*	@retval		성공시 : TRUE, 실패시 FALSE
************************************************************/
BOOL CEagleSVCLib::SaveEJL_SetDenomination(int nOldDenom[MAX_CST_COUNT], int nNewDenom[MAX_CST_COUNT])
{
	LOG(Info, _T("Set Denomination INFO in EJL - start"));

	BOOL bResult = FALSE;
	CStringA	szTemp;
	szTemp.Empty();

	// 현재 시간 취득
	SYSTEMTIME st;
	GetLocalTime(&st);

	// Common Info
	FLSEJCOMMINFO CommInfo;
	memset(&CommInfo, 0, sizeof(CommInfo));

	sprintf(CommInfo.log_date, "%04d/%02d/%02d", st.wYear, st.wMonth, st.wDay);
	sprintf(CommInfo.log_time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
	CommInfo.jnl_cd = denom_info;
	sprintf(CommInfo.log_srch_date, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	int nCSTCnt = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);
	int nCSTIndex = 0;

	FLSEJSETDENOMINFO SetDenomInfo;
	memset(&SetDenomInfo, 0, sizeof(SetDenomInfo));

	szTemp.Format("%02d/%02d/%04d", st.wMonth, st.wDay, st.wYear);
	sprintf(SetDenomInfo.start_date, "%s", szTemp);

	szTemp.Format("%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
	sprintf(SetDenomInfo.start_time, "%s", szTemp);

	szTemp.Format("%S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
	sprintf(SetDenomInfo.term_id, "%s", szTemp);

	// CST 개수 저장
	szTemp.Format("%d", nCSTCnt);
	sprintf(SetDenomInfo.cst_count, "%s", szTemp);

	if (nCSTCnt >= 1)
	{
		szTemp.Format("%d", nOldDenom[nCSTIndex]);
		sprintf(SetDenomInfo.olddenom_cst1, "%s", szTemp);

		szTemp.Format("%d", nNewDenom[nCSTIndex]);
		sprintf(SetDenomInfo.newdenom_cst1, "%s", szTemp);
	}

	nCSTIndex++;

	if (nCSTCnt >= 2)
	{
		szTemp.Format("%d", nOldDenom[nCSTIndex]);
		sprintf(SetDenomInfo.olddenom_cst2, "%s", szTemp);

		szTemp.Format("%d", nNewDenom[nCSTIndex]);
		sprintf(SetDenomInfo.newdenom_cst2, "%s", szTemp);
	}

	nCSTIndex++;

	if (nCSTCnt >= 3)
	{
		szTemp.Format("%d", nOldDenom[nCSTIndex]);
		sprintf(SetDenomInfo.olddenom_cst3, "%s", szTemp);

		szTemp.Format("%d", nNewDenom[nCSTIndex]);
		sprintf(SetDenomInfo.newdenom_cst3, "%s", szTemp);
	}

	nCSTIndex++;

	if (nCSTCnt >= 4)
	{
		szTemp.Format("%d", nOldDenom[nCSTIndex]);
		sprintf(SetDenomInfo.olddenom_cst4, "%s", szTemp);

		szTemp.Format("%d", nNewDenom[nCSTIndex]);
		sprintf(SetDenomInfo.newdenom_cst4, "%s", szTemp);
	}

	bResult = CDEV_Manager::GetInstance()->m_DEV_EJL.InsertJnlData(&CommInfo, &SetDenomInfo);

	LOG(Info, _T("Set Denomination INFO in EJL - End (Result : %d"), bResult);

	return bResult;
}


/** **********************************************************
*	@brief		SaveEJL_SetChangeValue  : 변경된 설정 값을 Journal에 저장하는 함수
*	@param		CString strOld : 이전 Data
*	@param		CString strNew : 새로운 Data
*	@retval		성공시 : TRUE, 실패시 FALSE
************************************************************/
//BOOL CEagleSVCLib::SaveEJL_SetChangeValue(CString strOld, CString strNew)
//{
//	LOG(Info, _T("Set ChangeValue INFO in EJL - start"));
//
//	BOOL bResult = FALSE;
//	CStringA szTemp;
//
//	szTemp.Empty();
//
//	// 현재 시간 취득
//	SYSTEMTIME st;
//	GetLocalTime(&st);
//
//	// Common Info
//	FLSEJCOMMINFO CommInfo;
//	memset(&CommInfo, 0, sizeof(CommInfo));
//
//	sprintf(CommInfo.log_date, "%04d/%02d/%02d", st.wYear, st.wMonth, st.wDay);
//	sprintf(CommInfo.log_time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
//	CommInfo.jnl_cd = value_change;
//	sprintf(CommInfo.log_srch_date, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
//
//	EJCHANGEINFO ChangeInfo;
//	memset(&ChangeInfo, 0, sizeof(ChangeInfo));
//
//	szTemp.Format("%S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
//	sprintf(ChangeInfo.term_id, "%s", szTemp);
//	sprintf(ChangeInfo.old_value, "%S", strOld);
//	sprintf(ChangeInfo.new_value, "%S", strNew);
//
//	bResult = CDEV_Manager::GetInstance()->m_DEV_EJL.InsertJnlData(&CommInfo, &ChangeInfo);
//
//	LOG(Info, _T("Set ChangeValue INFO in EJL - End (Result : %d"), bResult);
//
//	return bResult;
//}


/** **********************************************************
*	@brief		PrintEJL_DetailData  : Journal Detail값을 명세표에 Print하는 함수
*	@param		CString strPrintData : Print할 Data
*	@retval		성공시 : TRUE, 실패시 FALSE
************************************************************/
BOOL CEagleSVCLib::PrintEJL_DetailData(CString strPrintData)
{
	CStringArray strArray_PrintData;
	CStringA	szTemp, szEJLTemp;
	int			nOrgX = 40;
	int			i = 0;

	// 시작 전에 이미 Print 상태가 장애이면 FALSE return 처리
	if (IsPTR_Available() == FALSE)
		return FALSE;

	strArray_PrintData.RemoveAll();

	CUtil::ParsingStringToStringArray(strPrintData, SCR_LINEFEED_DELEMITER, strArray_PrintData);

	// 프린터 램프 ON
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_FLICKING);

	// PTR Quiry Status Stop
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();


	if (CEagleSVCLib::GetInstance()->Set_PTR_Config() == FALSE)
	{
		LOG(Error, _T("Set PTR Config is failed"));
	}

	for (i=0; i<strArray_PrintData.GetCount(); i++)
	{
		szTemp.Format("%S", strArray_PrintData[i]);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);
	}

	if (strArray_PrintData.GetCount() < 15)
	{
		// Print후 고객이 가져갈 수 있도록 명세표 용지 보정 처리
		for(i=strArray_PrintData.GetCount(); i<MINIMUM_PRIT_LINE; i++)
		{
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
		}
	}

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_CutPaper(EAGLE_PRT_TYPE_USB, 1, 0);

	// PTR Quiry Status Stop
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();

	// 프린터 램프 OFF
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_OFF);

	if (IsPTR_Available(TRUE) == FALSE)
		return FALSE;

	return TRUE;
}


/** *********************************************************
*	@brief		Transaction Data를 저널에 저장
*	@retval		TRUE : 저장 성공, FALSE : 저장 실패
************************************************************/
BOOL CEagleSVCLib::SaveEJL_TransactionInfo()
{
	FLSEJCOMMINFO	CommInfo;
	FLSEJTRANINFO	TransInfo;
	BOOL			bResult = FALSE;
	CString strTemp, strTemp2, strTransType, strJNLEMVData;

	SYSTEMTIME	st;
	GetLocalTime(&st);

	memset(&CommInfo, 0, sizeof(CommInfo));
	memset(&TransInfo, 0, sizeof(TransInfo));

	sprintf(CommInfo.log_date, "%04d/%02d/%02d", st.wYear, st.wMonth, st.wDay);
	sprintf(CommInfo.log_time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);


	strTransType = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType;

	if (strTransType == S_WITHDRAWAL)
		CommInfo.jnl_cd = withdrawal;
	else if (strTransType == S_BALANCEINQUIRY)
		CommInfo.jnl_cd = balance;
	else if (strTransType == S_TRANSFER)
		CommInfo.jnl_cd = transfer;
	else
	{
		// Transaction이 아닌 경우에 여기를 타는 경우는 예외 처리
		//CommInfo.jnl_cd = transaction;
		LOG(Error, _T("SaveEJL_TransactionInfo - Transaction Type is invalid (%s)"), strTransType);
		return FALSE;
	}

	sprintf(CommInfo.log_srch_date, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	// Terminal ID
	sprintf(TransInfo.term_id, "%S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
	
	// Sequence No
	sprintf(TransInfo.tran_seq_no, "%S", CEagleDataManager::GetInstance()->m_Config.m_System.strSequence_Number);
	
	// Transaction Code
	sprintf(TransInfo.tran_cd, "%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType);
	
	// Card No
	sprintf(TransInfo.card_tr2, "%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strShowCardNumber);

	// AID NAME & AID
	if ((CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction == TRUE) && (CDEV_Manager::GetInstance()->m_DEV_CDR.IsEMV_Enable() == TRUE))
	{
		strJNLEMVData = JNL_ERRMSG_FIELD_EMV_DATA;

		// EMV Journal 저장 로직 변경
		// 기존 : Tag / Length(int) / Value -> Tag / Length(Hex) / Value / FS

		// EMV Data 저장 조건을 AID로 변경
		CString strAID, strAIDLabel;
		int	nLen = 0;
		unsigned char szTemp[1024] = { 0, };

		strAID = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_str_AID;
		strAIDLabel = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_str_AID_Label;

		// 84 (AID Label)
		if (strAIDLabel.GetLength() > 0)
		{
			//strTemp.Format(_T("%S%02d%s"), EMVTag_DFName, strAIDLabel.GetLength(), strAIDLabel);
			strTemp.Format(_T("%s%02X%s"), _T("84"), strAIDLabel.GetLength(), strAIDLabel);
			strTemp += (CString)FS;
			strJNLEMVData += strTemp;			
		}

		// 9F06 (AID)
		if (strAIDLabel.GetLength() > 0)
		{
			//strTemp.Format(_T("%S%02d%s"), EMVTag_TerminalAID, strAID.GetLength(), strAID);
			strTemp.Format(_T("%S%02X%s"), EMVTag_TerminalAID, strAID.GetLength(), strAID);
			strTemp += (CString)FS;
			strJNLEMVData += strTemp;
		}

		// 82 (AIP)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_AIP, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0)
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_AIP, strTemp.GetLength(), strTemp);
				strTemp.Format(_T("%s%02X%s"), _T("82"), strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 8A (ARC)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_AuthRespCod, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0)
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_AuthRespCod, strTemp.GetLength(), strTemp);
				strTemp.Format(_T("%s%02X%s"), _T("8A"), strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 91 (ARPC)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_IsuAuthData, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0)
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_IsuAuthData, strTemp.GetLength(), strTemp);
				strTemp.Format(_T("%s%02X%s"), _T("91"), strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}


		// 9B (TSI)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TSI, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_TSI, strTemp.GetLength(), strTemp);
				strTemp.Format(_T("%s%02X%s"), _T("9B"), strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 9C (Transaction Type)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TrType, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_TrType, strTemp.GetLength(), strTemp);
				strTemp.Format(_T("%s%02X%s"), _T("9C"), strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 5F2A (TCC) - Currency
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TrCurCod, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_TrCurCod, strTemp.GetLength(), strTemp);
				strTemp.Format(_T("%S%02X%s"), EMVTag_TrCurCod, strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 5F30 (Service Code)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_ServiceCod, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			// 맨 앞에 0 제거
			strTemp2.Format(_T("%d"), CUtil::StringToInt(strTemp));

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_ServiceCod, strTemp2.GetLength(), strTemp2);
				strTemp.Format(_T("%S%02X%s"), EMVTag_ServiceCod, strTemp2.GetLength(), strTemp2);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 95 (TVR)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TVR, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_TVR, strTemp.GetLength(), strTemp);
				strTemp.Format(_T("%s%02X%s"), _T("95"), strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 9F0E (IAC Denial)

		// 9F10 (IAD)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_IsuAppData, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_IsuAppData, strTemp.GetLength(), strTemp);
				strTemp.Format(_T("%S%02X%s"), EMVTag_IsuAppData, strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 9F1A (TCC) - Country
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TCountryCod, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_TCountryCod, strTemp.GetLength(), strTemp);
				strTemp.Format(_T("%S%02X%s"), EMVTag_TCountryCod, strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 9F26 (AC)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_AC, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_AC, strTemp.GetLength(), strTemp);
				strTemp.Format(_T("%S%02X%s"), EMVTag_AC, strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 9F27 (CID)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_CryptInfData, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_CryptInfData, strTemp.GetLength(), strTemp);
				strTemp.Format(_T("%S%02X%s"), EMVTag_CryptInfData, strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 9F36 (ATC)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_ATC, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_ATC, strTemp.GetLength(), strTemp);
				strTemp.Format(_T("%S%02X%s"), EMVTag_ATC, strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 9F33 (Terminal Capabilities)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TerminalCapa, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				//strTemp.Format(_T("%S%02d%s"), EMVTag_TerminalCapa, strTemp.GetLength(), strTemp);
				strTemp.Format(_T("%S%02X%s"), EMVTag_TerminalCapa, strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 9F37 (UN)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_UnpredictNo, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				strTemp.Format(_T("%S%02X%s"), EMVTag_UnpredictNo, strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// 9F39 (POS Entry Mode)
		nLen = 0;
		strTemp.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_POSEntryMode, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strTemp = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */

				strTemp.Format(_T("%S%02X%s"), EMVTag_POSEntryMode, strTemp.GetLength(), strTemp);
				strTemp += (CString)FS;
				strJNLEMVData += strTemp; 
			}
		}

		// EMV Data Field로 변경
		if (strJNLEMVData.GetLength() > 1)	// 'E Command 후의 Data 존재시
			sprintf(TransInfo.emv_data, "%S", strJNLEMVData);

	}
	else
	{
		// MS Data가 IC 카드인 경우 Fall back message journal logging
		if (CDEV_Manager::GetInstance()->m_DEV_CDR.m_bUnknownAID == FALSE)
		{
			int nIndex = 0;

			strTemp = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTrack2Data;
			nIndex = strTemp.Find(_T("="));

			// Track2 Data 오류로 인해 미송신
			if (strTemp.GetLength() > nIndex + 5)
			{
				// MS Data Check 후 Card Type이 IC인 경우에만 Fallback 정보 송신
				if (strTemp.GetAt(nIndex + 5)  == '2' || strTemp.GetAt(nIndex + 5) == '6')
				{
					strJNLEMVData = JNL_ERRMSG_FIELD_FALLBACK;
					sprintf(TransInfo.emv_data, "%S", strJNLEMVData);	// Fallback도 저널에 저장
				}
			}
		}
	}
	
	// Authorization No
	sprintf(TransInfo.auth_no, "%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strAutorizationNumber);
	
	// Transaction date
	strTemp.Format(_T("%s"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransactionDate);
	if (strTemp.IsEmpty() == FALSE)
		sprintf(TransInfo.tran_date, "%S/%S/%S%S", strTemp.Left(2), strTemp.Mid(2,2), _T("20"), strTemp.Right(2));
	
	// Transaction time
	strTemp.Format(_T("%s"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransactionTime);
	if (strTemp.IsEmpty() == FALSE)
		sprintf(TransInfo.tran_time, "%S:%S:%S", strTemp.Left(2), strTemp.Mid(2,2), strTemp.Right(2));

	// Business date
	strTemp.Format(_T("%s"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strBusinessDate);
	if (strTemp.IsEmpty() == FALSE)
		sprintf(TransInfo.business_date, "%S/%S/%S%S", strTemp.Left(2), strTemp.Mid(2,2), _T("20"), strTemp.Right(2));

	// Account
	if (strTransType == S_TRANSFER)
	{
		sprintf(TransInfo.account_type, "%S TO %S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strFromAccountType, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strToAccountType);
	}
	else
	{
		sprintf(TransInfo.account_type, "%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strFromAccountType);
	}

	// Requested
	if (strTransType == S_WITHDRAWAL)
		sprintf(TransInfo.req_amt, "%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strRequestAmount);
	else if (strTransType == S_TRANSFER)
		sprintf(TransInfo.req_amt, "%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransferAmount);
	else
		sprintf(TransInfo.req_amt, "0");

	// Dispensed
	if (strTransType == S_WITHDRAWAL)
		sprintf(TransInfo.disp_amt, "%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount);
	else
		sprintf(TransInfo.disp_amt, "0");

	// Surcharge
	int nFeeAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount);
	int nDispensedAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount);

	if (nFeeAmount > 0)
	{
		if (strTransType == S_WITHDRAWAL)
		{
			if (nDispensedAmount > 0)
				sprintf(TransInfo.surc_amt, "%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount);
			else
				sprintf(TransInfo.surc_amt, "0");
		}
		else if (strTransType == S_BALANCEINQUIRY)
		{
			if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE)	// 조회시에는 장애가 없는 경우에만 수수료 부과
			{
				sprintf(TransInfo.surc_amt, "%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount);
			}
		}
	}
	else
	{
		sprintf(TransInfo.surc_amt, "0");
	}

	// Balance는 저널에는 무조건 수신받은대로 인자
	sprintf(TransInfo.bal_amt, "%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strBalance);

	sprintf(TransInfo.avail_bal_amt, "%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strAvailableBalance);

	if (strTransType == S_WITHDRAWAL)
	{
		sprintf(TransInfo.disp_cnt,"%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispensedCount);
		sprintf(TransInfo.reject_cnt,"%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strRejectedCount);

		// Reject 관련 정보 추가
		sprintf(TransInfo.cst1_reject_info,"%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strCST_Reject_Info[0]);	
		sprintf(TransInfo.cst2_reject_info,"%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strCST_Reject_Info[1]);	
		sprintf(TransInfo.cst3_reject_info,"%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strCST_Reject_Info[2]);	
		sprintf(TransInfo.cst4_reject_info,"%S", CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strCST_Reject_Info[3]);	
	}

	// 카세트 잔여매수는 거래에 상관없이 계속 저널에 저장한다.
	strTemp.Empty();
	int nCBXCnt = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);
	for(int i=0; i<MAX_CST_COUNT; i++)
	{
		// Power Off Reversal인 경우 CBXCount을 가져오지 못하는 Bug Fix
		//if (i < CDEV_Manager::GetInstance()->m_DEV_CDM.m_nCbxCount)
		if (i < nCBXCnt)
			strTemp += CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i];
		else
			strTemp += _T("0");
		strTemp += _T(" ");
	}
	sprintf(TransInfo.cst_remain_cnt,"%S", strTemp);

	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bPowerOffFlag == TRUE)
		sprintf(TransInfo.err_code_1, "%S", POWEROFF_REVERSAL);
	else if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
		sprintf(TransInfo.err_code_1, "%S", Get_ErrorCode());

	// Reversal Fail시 처리
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetReversalError == TRUE)
		sprintf(TransInfo.err_code_2, "%S", ERROR_REVERSAL_FAILED);

	//if (CEagleDataManager::GetInstance()->m_Client_Info.m_bPowerOffFlag == TRUE)
	//	sprintf(TransInfo.err_code_3, "%S", POWEROFF_REVERSAL);

	// Camera Image File Data 추가
	if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsOpened() == TRUE)
	{
		strTemp = Add_Backup_CaptureImageFile();

		if (strTemp.IsEmpty() == FALSE)
		{
			sprintf(TransInfo.cam_img_filename, "%S", strTemp);
		}
	}

	bResult = CDEV_Manager::GetInstance()->m_DEV_EJL.InsertJnlData(&CommInfo, &TransInfo);

	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_bSaveEJLforTransRecord = FALSE;
	CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfoData(_T("IsSaveEJLTransRecord"), _T("0"));

	LOG(Info, _T("Set Transaction information in EJL - End (Result : %d"), bResult);

	return bResult;
}


/** *********************************************************
*	@brief		방출 중 reject 발생시, reject 사유 기록
*	@param		int nTestDispenseCbxNumber	: 정상 TestDispense 카세트 번호
*	@param		BYTE byTestDispenseCount	: 정상 TestDispense Count
*	@retval		없음
************************************************************/
void CEagleSVCLib::UpdateRejectInfo(int nTestDispenseCbxNumber/* = 0*/, BYTE byTestDispenseCount/* = 0*/)
{
	CString strRejectData = _T("");
	CString strWriteValue = _T("");
	CString strCurrentRejectValue, strTemp;
	CString strLog = _T(""), strLog2 = _T("");
	CStringArray strArrRejectData;
	int nNoteCount = 0;
	int nIndex = 0;
	int i = 0, j = 0;
	BYTE byRejectKind[MAX_CASSETTE][REJECT_KIND+1] = {0,};
	int nRejectCount = 0;
	int nCbxCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);
	BOOL bResult = FALSE;

	CDM_REJECT_INFO rejectInfo;

	ZeroMemory(&rejectInfo, sizeof(CDM_REJECT_INFO));
	strArrRejectData.RemoveAll();

	if(0 == nTestDispenseCbxNumber)
	{
		bResult = CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_GetRejectLog(&rejectInfo);

		if(TRUE == bResult)
		{
			for(i=0; i<MAX_CASSETTE; i++)
			{
				nRejectCount = rejectInfo.byRejectInfo[nIndex];
				byRejectKind[i][REJECT_KIND] += nRejectCount;	// total reject count

				nIndex++;

				for(j=0; j<nRejectCount; j++)
				{
					CalculateRejectKind(i, rejectInfo.byRejectInfo[nIndex], byRejectKind);
					nIndex++;
				}
			}
		}
	}
	else
	{
		// 정상적인 Test Dispense 결과 반영
		byRejectKind[nTestDispenseCbxNumber-1][REJECT_KIND-1] = byTestDispenseCount;
		byRejectKind[nTestDispenseCbxNumber-1][REJECT_KIND] += byTestDispenseCount;
		//////////////////////////////////////////////////////////
	}

	for(i=0; i<MAX_CASSETTE; i++)
	{
		strLog2.Empty();
		for(j=0; j<REJECT_KIND+1; j++)
		{
			strLog.Format(_T("%d "), byRejectKind[i][j]);
			strLog2 += strLog;
		}

		LOG(Error, _T("CBX #%d RejectInfo => [%s]"), i+1, strLog2);
	}

	for(i=0; (i<nCbxCount) && (i<MAX_CFG_CBX_COUNT); i++)
	{
		strWriteValue.Empty();
		strCurrentRejectValue.Empty();

		strRejectData = CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Reason[i];
		CUtil::ParsingStringToStringArray(strRejectData, _T(" "), strArrRejectData);

		if (strRejectData.GetLength() > (REJECT_KIND*2))
		{
			for (j = 0; j < REJECT_KIND; j++)	// -1 : test dispense 항목은 제외
			{
				nNoteCount = CUtil::StringToInt(strArrRejectData.GetAt(j));

				{
					nNoteCount += byRejectKind[i][j];

					// Current에 대한 Reject 매수를 구하기
					strTemp.Format(_T("%d"), byRejectKind[i][j]);
					strCurrentRejectValue += strTemp;
					strCurrentRejectValue += _T(" ");

					// cfg 기록 최대 한도 매수 설정.
					if(50000 < nNoteCount)
						nNoteCount = 50000;

					strWriteValue += CUtil::IntToString(nNoteCount);
					strWriteValue += _T(" ");
				}
			}

			nNoteCount = CUtil::StringToInt(strArrRejectData[j]);
			nNoteCount += byRejectKind[i][REJECT_KIND];

			// Ini 기록 최대 한도 매수 설정.
			if(50000 < nNoteCount)
				nNoteCount = 50000;

			strWriteValue += CUtil::IntToString(nNoteCount);

			LOG(Info, _T("Update Reject Info CBX #%d strWriteValue : [%s]"), i+1, strWriteValue);

			CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Reason[i] = strWriteValue;
			//CEagleDataManager::GetInstance()->m_Config.SaveCbxInfo();

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strCST_Reject_Info[i] = strCurrentRejectValue;
		}
		else
		{
			LOG(Error, _T("reject format is wronged"));
		}
	}

	CEagleDataManager::GetInstance()->m_Config.SaveCbxInfo();
	CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfoData(_T("CST1_Reject_Info"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strCST_Reject_Info[0]);
	CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfoData(_T("CST2_Reject_Info"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strCST_Reject_Info[1]);	
	CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfoData(_T("CST3_Reject_Info"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strCST_Reject_Info[2]);
	CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfoData(_T("CST4_Reject_Info"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strCST_Reject_Info[3]);
}

/** *********************************************************
*	@brief		Reject 종류별 누적 계산
*	@param		int nCbxNumber : 카세트 번호
*	@param		BYTE byRejectCode : Reject Code
*	@param		BYTE byRejectKind[MAX_CASSETTE][REJECT_KIND+1] : Reject 종류별 저장 변수
*	@retval		없음
************************************************************/
void CEagleSVCLib::CalculateRejectKind(int nCbxNumber, BYTE byRejectCode, BYTE byRejectKind[MAX_CASSETTE][REJECT_KIND+1])
{
	if(byRejectCode & 0x01)
	{
		byRejectKind[nCbxNumber][0]++;
	}
	else if(byRejectCode & 0x02)
	{
		byRejectKind[nCbxNumber][1]++;
	}
	else if(byRejectCode & 0x04)
	{
		byRejectKind[nCbxNumber][2]++;
	}
	else if(byRejectCode & 0x08)
	{
		byRejectKind[nCbxNumber][3]++;
	}
	else if(byRejectCode & 0x10)
	{
		byRejectKind[nCbxNumber][4]++;
	}
	else if(byRejectCode & 0x20)
	{
		byRejectKind[nCbxNumber][5]++;
	}
	else if(byRejectCode & 0x40)
	{
		byRejectKind[nCbxNumber][6]++;
	}
	else if(byRejectCode & 0x80)
	{
		byRejectKind[nCbxNumber][7]++;
	}
}


/** *********************************************************
*	@brief		Check_CBXAllDenomination : 모든 CST denomination값 확인
*	@retval		TRUE: 잘못된 권종 정보가 설정된 경우, FALSE : 정상인 경우
************************************************************/
BOOL CEagleSVCLib::Check_Invalid_CBXAllDenomination()
{
#if (EMULATION_CDM_DEVICE)
	return FALSE;
#endif

	int		nCBXCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);
	int		nAvailableDenom[7] = { 1, 2, 5, 10, 20, 50, 100};	// 타국가 Currency 적용시 table 수정 필요
	int		nDenomination = 0, i=0, j=0;

	for(i=0; i<nCBXCount; i++)
	{
		nDenomination = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i]);

		if (nDenomination == 0)	// 권종이 0인 경우 Invalid return
			return TRUE;

		for(j=0; j<7; j++)
		{
			if (nAvailableDenom[j] == nDenomination)
				break;
		}
		
		if (j>=7)	// Invalid case
			return TRUE;
	}

	return FALSE;
}


/** *********************************************************
*	@brief		Set_HealthCheck_Time : Heartbeat Timer Set
*	@retval		없음
************************************************************/
void CEagleSVCLib::Set_Heartbeat_Time()
{
	// Update Current Time
	Get_Heartbeat_GapTime();

	m_oletimeStart	= m_oletimeNow;
	m_DelayTime		= CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_frequency)*60;	// Minutes * 60 = second

	LOG(Info, _T("Update Heartbeat Time"));
}


/** *********************************************************
*	@brief		Get_HealthCheck_GapTime : Heartbeat Timer Set
*	@retval		Gap seconds
************************************************************/
UINT CEagleSVCLib::Get_Heartbeat_GapTime()
{
	m_oletimeNow = COleDateTime::GetCurrentTime();
	m_oletimeGap = m_oletimeNow - m_oletimeStart;

	return (UINT)m_oletimeGap.GetTotalSeconds();
}


/** *********************************************************
*	@brief		Send_HeartbeatOrScheduleDayCloseMsg : Heartbeat 또는 Day Close message 송신
*	@retval		TRUE : 성공, FALSE : 실패
************************************************************/
BOOL CEagleSVCLib::Send_HeartbeatOrScheduleDayCloseMsg(int nMsgType)
{
	LOG(Info, _T("Send_Heartbeat or Day Close Msg (%d) - Start"), nMsgType);

	BOOL	bResult = FALSE;

	// AMS Server Close
	CDEV_Manager::GetInstance()->m_DEV_HOST.AMS_Sync_ConnectClose();

	// In Service 화면에서는 광고를 임시적 Disable 처리
	if (CSVC_Manager::GetInstance()->GetServiceMode() == EAGLE_ATM_SVC_IN_SERVICE)
	{
		CSCR_Manager::GetInstance()->SetAdvertisement(FALSE);
	}

	// HOST Processing 화면 표시
	// HOST 송/수신 메시지 통지
	CSVC_Manager::GetInstance()->Client_Screen_Processing(1);

	// Send Configuration Message
	CEagleTritonMsg::GetInstance()->InitializeVariable();
	CEagleTritonMsg::GetInstance()->Triton_MakeHostMsg(nMsgType);

	if (SendRecvHost(nMsgType) != SUCCESS)
	{
		// Health Check or Schedule Auto Day Total network 장애는 무시하도록 처리
		if (Get_ErrorCode().Left(2) == _T("Z3"))
			Clear_Error();

		bResult = FALSE;
	}
	else
	{
		bResult = TRUE;
	}

	if (nMsgType == EAGLE_TRAN_CODE_DOWNLOAD_CONFIG)
		Set_Heartbeat_Time();

	LOG(Info, _T("Send_Heartbeat or Day Close Msg - End"));

	return bResult;
}


/** *********************************************************
*	@brief		IsPTR_Available : 명세표 및 용지 상태 조회
*	@retval		TRUE : 정상, FALSE : 장애
************************************************************/
BOOL CEagleSVCLib::IsPTR_Available(BOOL bCheckWait)
{
#if (EMULATION_PTR_DEVICE)
	return TRUE;
#endif

	if (bCheckWait == TRUE)
		CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

	if ((CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetStatus() != EAGLE_PRT_NORMAL) || (CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetPaperStatus() == EAGLE_PRT_PAPER_EMPTY))
	{
		LOG(Info, _T("Printer is not available PTR Status(%d), PaperStatus(%d) Error(%s)"), CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetStatus(), EAGLE_PRT_PAPER_EMPTY, CDEV_Manager::GetInstance()->m_DEV_PRT.GetLastError());
		return FALSE;
	}

	return TRUE;
}


/** *********************************************************
*	@brief		Print_SetDenomination  : Set Denomination 수행결과를 명세표에 Print하는 함수
*	@param		int nOldDenom : 이전 Denomination값
*	@param		int nNewDenom : 변경한 Denomination값
*	@retval		성공시 : TRUE, 실패시 FALSE
************************************************************/
BOOL CEagleSVCLib::Print_SetDenomination(int nOldDenom[MAX_CST_COUNT], int nNewDenom[MAX_CST_COUNT])
{
	LOG(Info, _T("Print Set Denomination INFO - start"));

	// 영수증 출력 장애 여부 확인
	CStringA szTemp = "";
	int	nOrgX = 40;
	int	nCSTCnt = 0;
	int	nLineCount = 0;
	int nCSTIndex = 0;

	SYSTEMTIME	st;
	GetLocalTime(&st);

	nCSTCnt = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

	// 프린터 램프 ON
	// PTR Quiry Status Stop
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();

	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_FLICKING);

	if (CEagleSVCLib::GetInstance()->Set_PTR_Config() == FALSE)
	{
		LOG(Error, _T("Set PTR Config is failed"));
	}

	// 1. Title
	szTemp.Format("*** SET DENOMINATION ***");

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Error, _T("PRT - %S"), szTemp);
	nLineCount++;

	// 2. START
	szTemp.Format("START : %02d/%02d/%04d %02d:%02d:%02d", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Error, _T("PRT - %S"), szTemp);
	nLineCount++;

	// 3. TERMINAL ID
	szTemp.Format("TERMINAL ID : %S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Error, _T("PRT - %S"), szTemp);

	// 4. Linefeed
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	nLineCount++;

	if (nCSTCnt >= 1)
	{
		if (nOldDenom[nCSTIndex] != nNewDenom[nCSTIndex])
		{
			szTemp.Format("  CBX #1 = %S%d -> %S%d", CURRENCY_SYMBOL, nOldDenom[nCSTIndex], CURRENCY_SYMBOL, nNewDenom[nCSTIndex]);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}
		else
		{
			szTemp.Format("  CBX #1 = %S%d (NO CHANGE)", CURRENCY_SYMBOL, nOldDenom[nCSTIndex]);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}
	}

	nCSTIndex++;

	if (nCSTCnt >= 2)
	{
		if (nOldDenom[nCSTIndex] != nNewDenom[nCSTIndex])
		{
			szTemp.Format("  CBX #2 = %S%d -> %S%d", CURRENCY_SYMBOL, nOldDenom[nCSTIndex], CURRENCY_SYMBOL, nNewDenom[nCSTIndex]);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}
		else
		{
			szTemp.Format("  CBX #2 = %S%d (NO CHANGE)", CURRENCY_SYMBOL, nOldDenom[nCSTIndex]);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}
	}

	nCSTIndex++;

	if (nCSTCnt >= 3)
	{
		if (nOldDenom[nCSTIndex] != nNewDenom[nCSTIndex])
		{
			szTemp.Format("  CBX #3 = %S%d -> %S%d", CURRENCY_SYMBOL, nOldDenom[nCSTIndex], CURRENCY_SYMBOL, nNewDenom[nCSTIndex]);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}
		else
		{
			szTemp.Format("  CBX #3 = %S%d (NO CHANGE)", CURRENCY_SYMBOL, nOldDenom[nCSTIndex]);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}
	}

	nCSTIndex++;

	if (nCSTCnt >= 4)
	{
		if (nOldDenom[nCSTIndex] != nNewDenom[nCSTIndex])
		{
			szTemp.Format("  CBX #4 = %S%d -> %S%d", CURRENCY_SYMBOL, nOldDenom[nCSTIndex], CURRENCY_SYMBOL, nNewDenom[nCSTIndex]);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}
		else
		{
			szTemp.Format("  CBX #4 = %S%d (NO CHANGE)", CURRENCY_SYMBOL, nOldDenom[nCSTIndex]);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
			nLineCount++;
		}
	}

	if (nLineCount< MINIMUM_PRIT_LINE)
	{
		for(int i=nLineCount; i<MINIMUM_PRIT_LINE; i++)
		{
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		}
	}

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_CutPaper(EAGLE_PRT_TYPE_USB, 1, 0);

	// PTR Quiry Status Start
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();

	// 프린터 램프 OFF
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_OFF);

	if (IsPTR_Available(TRUE) == FALSE)
		return FALSE;	

	LOG(Info, _T("Print Set Denomination INFO - End"));

	return TRUE;
}


/** *********************************************************
*	@brief		Set_Reboot_Time  : Reboot할 시간을 설정하는 함수
*	@retval		없음
************************************************************/
void CEagleSVCLib::Set_Reboot_Time()
{
	COleDateTime	NextRebootTime;
	SYSTEMTIME		st;
	GetLocalTime(&st);

	// Update Reboot Time
	NextRebootTime.SetDateTime(st.wYear, st.wMonth, st.wDay, 0, 0, 0);

	// +1 day
	COleDateTimeSpan	spanOneDay(1, 0, 0, 0);		// 1 day.
	NextRebootTime += spanOneDay;

	LOG(Info, _T("Update Next Reboot Day : %04d-%02d-%02d %s"), NextRebootTime.GetYear(), NextRebootTime.GetMonth(), NextRebootTime.GetDay(), CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime);

	CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootYear = CUtil::IntToString(NextRebootTime.GetYear());
	CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootMonth = CUtil::IntToString(NextRebootTime.GetMonth());
	CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootDay = CUtil::IntToString(NextRebootTime.GetDay());

	// Save File
	CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("reboot_year"), CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootYear);
	CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("reboot_month"), CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootMonth);
	CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("reboot_day"), CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootDay);
}


/** *********************************************************
*	@brief		Device_DeInitialize  : Device Close 수행
*	@retval		없음
************************************************************/
void CEagleSVCLib::Device_DeInitialize()
{
	// System Reboot전에 Device Close 수행
	CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_ClosePort();
	CDEV_Manager::GetInstance()->m_DEV_DIO.Close();
	CDEV_Manager::GetInstance()->m_DEV_CDR.Close();
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_Close();
//	CDEV_Manager::GetInstance()->m_DEV_CREPP.CloseDevice();	// Virtual Serial Port Close시 System 부하로 인해 USB 미인식 현상 발생함

	if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsOpened() == TRUE)
		CDEV_Manager::GetInstance()->m_DEV_CAMERA.Close();
}


/** *********************************************************
*	@brief		SaveEJL_TestDispense  : TestDispense 저널 저장
*	@retval		없음
************************************************************/
BOOL CEagleSVCLib::SaveEJL_TestDispense(int nTestDispResult, BYTE byReqCount[MAX_CASSETTE], BYTE byPickupCount[MAX_CASSETTE], BYTE byRejCount[MAX_CASSETTE], CString strErrorCode)
{
	LOG(Info, _T("Test Dispense INFO in EJL - start"));

	BOOL bResult = FALSE;
	CStringA	szTemp = "";
	int			nCSTCnt = 0, nCSTDenom = 0, nRemainCount = 0;

	// 현재 시간 취득
	SYSTEMTIME st;
	GetLocalTime(&st);

	// Common Info
	FLSEJCOMMINFO CommInfo;
	memset(&CommInfo, 0, sizeof(CommInfo));

	sprintf(CommInfo.log_date, "%04d/%02d/%02d", st.wYear, st.wMonth, st.wDay);
	sprintf(CommInfo.log_time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
	CommInfo.jnl_cd = test_dispense;
	sprintf(CommInfo.log_srch_date, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	nCSTCnt = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);


	FLSEJTESTDISPENSE TestDispense;
	memset(&TestDispense, 0, sizeof(TestDispense));

	szTemp.Format("%02d/%02d/%04d", st.wMonth, st.wDay, st.wYear);
	sprintf(TestDispense.start_date, "%s", szTemp);

	szTemp.Format("%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
	sprintf(TestDispense.start_time, "%s", szTemp);

	szTemp.Format("%S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
	sprintf(TestDispense.term_id, "%s", szTemp);

	// Test Dispense Result
	if(NO_ERROR == nTestDispResult)
		TestDispense.test_disp_result[0] = '1';
	else
		TestDispense.test_disp_result[0] = '0';

	// CST 개수 저장
	szTemp.Format("%d", nCSTCnt);
	sprintf(TestDispense.cst_count, "%s", szTemp);

	for(int i=0; i<nCSTCnt; i++)
	{
		// Test Request Count
		sprintf(TestDispense.test_req_bill_cst[i], "%d", (int)byReqCount[i]);

		// Test Pickup Count
		sprintf(TestDispense.test_pickup_bill_cst[i], "%d", (int)byPickupCount[i]);

		// Test Reject Count
		sprintf(TestDispense.test_rej_bill_cst[i], "%d", (int)byRejCount[i]);

		// Remain Count
		nRemainCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);
		sprintf(TestDispense.rem_bill_cst[i], "%d", nRemainCount);

		// Denomination
		nCSTDenom = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i]); 
		sprintf(TestDispense.denom_cst[i], "%d", nCSTDenom);
	}

	if(nTestDispResult != NO_ERROR)
	{
		szTemp.Format("%S", strErrorCode);
		sprintf(TestDispense.err_code, "%s", szTemp);
	}

	
	bResult = CDEV_Manager::GetInstance()->m_DEV_EJL.InsertJnlData(&CommInfo, &TestDispense);

	LOG(Info, _T("Test Dispense INFO in EJL - End (Result : %d"), bResult);

	return bResult;
}
