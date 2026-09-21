#include "stdafx.h"
#include "DEV_Define.h"
#include "DEV_CDM.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleConfig.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleDataManager.h"
#include "DeviceSimUtil.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString DISP_LOG_HEADER[] = {
	_T("CL1o>"), _T("CL2o>"), _T("CL3o>"), _T("CL4o>"), _T("CL1x>"), _T("CL2x>"), _T("CL3x>"), _T("CL4x>"), 
	_T("CR1o>"), _T("CR2o>"), _T("CR3o>"), _T("CR4o>"), _T("CR1x>"), _T("CR2x>"), _T("CR3x>"), _T("CR4x>"),
	_T("MI1o>"), _T("MI2o>"), _T("MI3o>"), _T("MI4o>"), _T("MI1x>"), _T("MI2x>"), _T("MI3x>"), _T("MI4x>"),
	_T("LENL>"), _T("LENR>"), _T("THIC>"), _T("THRo>"), _T("THRx>"), _T("BRET>"), _T("DIVO>"), _T("DIVo>"),
	_T("DIVF>"), _T("DIVf>"), _T("EXTo>"), _T("EXTx>"), _T("PI1O>"), _T("PI2O>"), _T("PI3O>"), _T("PI4O>"),
	_T("PI1o>"), _T("PI2o>"), _T("PI3o>"), _T("PI4o>"), _T("PI1F>"), _T("PI2F>"), _T("PI3F>"), _T("PI4F>"),
	_T("PI1f>"), _T("PI2f>"), _T("PI3f>"), _T("PI4f>"), _T("ErrE>"), _T("ErrC>"), _T("TIKo>"), _T("TIKx>"),
	_T("CL1O>"), _T("CL2O>"), _T("CL3O>"), _T("CL4O>"), _T("CR1O>"), _T("CR2O>"), _T("CR3O>"), _T("CR4O>")
};

#define DISP_LOG_HEADER_COUNT	64

const CString strLogName[] = {
	_T("CL1o"), _T("CL2o"), _T("CL3o"), _T("CL4o"), _T("CL1x"), _T("CL2x"), _T("CL3x"), _T("CL4x"), _T("CR1o"), _T("CR2o"),
	_T("CR3o"), _T("CR4o"), _T("CR1x"), _T("CR2x"), _T("CR3x"), _T("CR4x"), _T("MI1o"), _T("MI2o"), _T("MI3o"), _T("MI4o"),
	_T("MI1x"), _T("MI2x"), _T("MI3x"), _T("MI4x"), _T("LENL"), _T("LENR"), _T("THIC"), _T("THRo"), _T("THRx"), _T("BRET"),
	_T("DIVO"), _T("DIVo"), _T("DIVF"), _T("DIVf"), _T("EXTo"), _T("EXTx"), _T("PI1O"), _T("PI2O"), _T("PI3O"), _T("PI4O"),
	_T("PI1o"), _T("PI2o"), _T("PI3o"), _T("PI4o"), _T("PI1F"), _T("PI2F"), _T("PI3F"), _T("PI4F"), _T("PI1f"), _T("PI2f"),
	_T("PI3f"), _T("PI4f"), _T("ErrE"), _T("ErrC"), _T("TIKo"), _T("TIKx"), _T("CL1O"), _T("CL2O"), _T("CL3O"), _T("CL4O"), 
	_T("CR1O"), _T("CR2O"), _T("CR3O"), _T("CR4O")
};

typedef enum
{
	IDX_CL1o = 0,
	IDX_CL2o,
	IDX_CL3o,
	IDX_CL4o,
	IDX_CL1x,
	IDX_CL2x,
	IDX_CL3x,
	IDX_CL4x,
	IDX_CR1o,
	IDX_CR2o,
	IDX_CR3o,
	IDX_CR4o,
	IDX_CR1x,
	IDX_CR2x,
	IDX_CR3x,
	IDX_CR4x,
	IDX_MI1o,
	IDX_MI2o,
	IDX_MI3o,
	IDX_MI4o,
	IDX_MI1x,
	IDX_MI2x,
	IDX_MI3x,
	IDX_MI4x,
	IDX_LENL,
	IDX_LENR,
	IDX_THIC,
	IDX_THRo,
	IDX_THRx,
	IDX_BRET,
	IDX_DIVO,
	IDX_DIVo,
	IDX_DIVF,
	IDX_DIVf,
	IDX_EXTo,
	IDX_EXTx,
	IDX_PI1O,
	IDX_PI2O,
	IDX_PI3O,
	IDX_PI4O,
	IDX_PI1o,
	IDX_PI2o,
	IDX_PI3o,
	IDX_PI4o,
	IDX_PI1F,
	IDX_PI2F,
	IDX_PI3F,
	IDX_PI4F,
	IDX_PI1f,
	IDX_PI2f,
	IDX_PI3f,
	IDX_PI4f,
	IDX_ErrE,
	IDX_ErrC,
	IDX_TIKo,
} SENSOR_INDEX;

/** **********************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CDEV_CDM::CDEV_CDM()
{
	// DLL 경로 설정
	CString	strPath = _T("");
	int i=0;

	// DLL 인스턴스 취득
	m_hDll = LoadLibrary(EAGLE_ATM_CDM_DLL);

	// API 취득
	LibMFSCommCDM_OpenPort = (MFSCommCDM_OpenPort)GetProcAddress(m_hDll, _T("MFSCommCDM_OpenPort"));

	LibMFSCommCDM_ClosePort = (MFSCommCDM_ClosePort)GetProcAddress(m_hDll, _T("MFSCommCDM_ClosePort"));

	LibMFSCommCDM_Reset = (MFSCommCDM_Reset)GetProcAddress(m_hDll, _T("MFSCommCDM_Reset"));

	LibMFSCommCDM_CfgStatus = (MFSCommCDM_CfgStatus)GetProcAddress(m_hDll, _T("MFSCommCDM_CfgStatus"));

	LibMFSCommCDM_Status = (MFSCommCDM_Status)GetProcAddress(m_hDll, _T("MFSCommCDM_Status"));

	LibMFSCommCDM_MultiDispense = (MFSCommCDM_MultiDispense)GetProcAddress(m_hDll, _T("MFSCommCDM_MultiDispense"));

	LibMFSCommCDM_LastDispense = (MFSCommCDM_LastDispense)GetProcAddress(m_hDll, _T("MFSCommCDM_LastDispense"));

	LibMFSCommCDM_Diagnostic = (MFSCommCDM_Diagnostic)GetProcAddress(m_hDll, _T("MFSCommCDM_Diagnostic"));

	LibMFSCommCDM_SetBillThickness = (MFSCommCDM_SetBillThickness)GetProcAddress(m_hDll, _T("MFSCommCDM_SetBillThickness"));

	LibMFSCommCDM_GetBillThickness = (MFSCommCDM_GetBillThickness)GetProcAddress(m_hDll, _T("MFSCommCDM_GetBillThickness"));
	
	LibMFSCommCDM_SetBillSize = (MFSCommCDM_SetBillSize)GetProcAddress(m_hDll, _T("MFSCommCDM_SetBillSize"));

	LibMFSCommCDM_GetBillSize = (MFSCommCDM_GetBillSize)GetProcAddress(m_hDll, _T("MFSCommCDM_GetBillSize"));

	LibMFSCommCDM_LearnBill = (MFSCommCDM_LearnBill)GetProcAddress(m_hDll, _T("MFSCommCDM_LearnBill"));

	LibMFSCommCDM_GetDLLVersion = (MFSCommCDM_GetDLLVersion)GetProcAddress(m_hDll, _T("MFSCommCDM_GetDLLVersion"));

	LibMFSCommCDM_TestDispense = (MFSCommCDM_TestDispense)GetProcAddress(m_hDll, _T("MFSCommCDM_TestDispense"));

	LibMFSCommCDM_GetNoteLog = (MFSCommCDM_GetNoteLog)GetProcAddress(m_hDll, _T("MFSCommCDM_GetNoteLog"));

	LibMFSCommCDM_GetThickLog = (MFSCommCDM_GetThickLog)GetProcAddress(m_hDll, _T("MFSCommCDM_GetThickLog"));

	LibMFSCommCDM_GetRejectLog = (MFSCommCDM_GetRejectLog)GetProcAddress(m_hDll, _T("MFSCommCDM_GetRejectLog"));

	LibMFSCommCDM_GetDispenseLog = (MFSCommCDM_GetDispenseLog)GetProcAddress(m_hDll, _T("MFSCommCDM_GetDispenseLog"));

	LibMFSCommCDM_UpdateFirmware = (MFSCommCDM_UpdateFirmware)GetProcAddress(m_hDll, _T("MFSCommCDM_UpdateFirmware"));

	LibMFSCommCDM_CEInfoCertification = (MFSCommCDM_CEInfoCertification)GetProcAddress(m_hDll, _T("MFSCommCDM_CEInfoCertification"));

	LibMFSCommCDM_IsSendDispense = (MFSCommCDM_IsSendDispense)GetProcAddress(m_hDll, _T("MFSCommCDM_IsSendDispense"));

	LibMFSCommCDM_GetExistCEInfo = (MFSCommCDM_GetExistCEInfo)GetProcAddress(m_hDll, _T("MFSCommCDM_GetExistCEInfo"));

	m_nCbxCount = 0;

	memset(m_nCbxDenom, 0, sizeof(m_nCbxDenom));

	memset(m_nCbxRemain, 0, sizeof(m_nCbxRemain));

	m_WatchStatusThread = NULL;
	m_bThreadExit = FALSE;

	for(i=0; i<MAX_CASSETTE + 1; i++)
	{
		m_dwCbxStatus[i] = WFS_CDM_STATCUMISSING;
		m_dwPrevCbxStatus[i] = WFS_CDM_STATCUMISSING;
	}
	for(i=0; i<MAX_CASSETTE; i++)
		m_bCbxStatusEmpty[i] = FALSE;
	
	m_dwMaxRejectCount = 0;

	memset(m_byCEInfoData, 0, sizeof(m_byCEInfoData));
	memset(&m_lastDispenseInfo, 0, sizeof(CDMLASTDISPENSE));

	m_bCDMExecuteQuiryStatus = FALSE;

	m_strWDM_FW_Version = _T("00.00");

	m_bNeedToRejectLog = FALSE;
}


/** **********************************************************
*	@brief		소멸자
*	@retval		없음
************************************************************/
CDEV_CDM::~CDEV_CDM()
{
	if (m_hDll)
	{
		FreeLibrary(m_hDll);

		m_hDll = NULL;
	}
}


/** **********************************************************
*	@brief		Is DLL Loaded
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::IsDllLoaded(void)
{
#if	EMULATION_CDM_DEVICE
	return TRUE;
#endif

	return (NULL != m_hDll);
}


/** **********************************************************
*	@brief		Open Port
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_OpenPort(HWND hWnd, int nPortNum, int nWriteLogLevel)
{
#if	EMULATION_CDM_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	return CDevSim::Confirm(_T("CDM"), _T("CDM_OpenPort"));
#endif

	BOOL bResult = FALSE;

	if (NULL == LibMFSCommCDM_OpenPort)
	{
		return FALSE;
	}

	if(FALSE == LibMFSCommCDM_OpenPort(hWnd, nPortNum, nWriteLogLevel))
	{
		return FALSE;
	}

	m_bThreadExit = FALSE;

	m_WatchStatusThread = AfxBeginThread((AFX_THREADPROC)WatchStatus, this);

	if (NULL == m_WatchStatusThread)
	{
		return FALSE;
	}

	return TRUE;
}


/** **********************************************************
*	@brief		Close Port
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_ClosePort()
{
#if	EMULATION_CDM_DEVICE
	return CDevSim::Confirm(_T("CDM"), _T("CDM_ClosePort"));
#endif

	m_bThreadExit = TRUE;

	if (m_WatchStatusThread != NULL)
	{
		// Event 생성을 안해 하기 로직은 허수임 (주석을 적용해야 하나, 통신 장애시 Thread 종료가 오래 걸려 적용은 보류함)
		//WaitForSingleObject(m_WatchStatusThread, EAGLE_SLEEP_INTERVAL_5MIN);

		//DWORD	dwExitCode = STILL_ACTIVE;
		//int		nCount = 0;

		//while(dwExitCode == STILL_ACTIVE)
		//{
		//	GetExitCodeThread(m_WatchStatusThread, &dwExitCode);

		//	if (nCount > 500)	// 5초 대기
		//		break;

		//	nCount++;

		//	CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
		//}

		//CloseHandle(m_WatchStatusThread);
		::TerminateThread(m_WatchStatusThread, 0);
		m_WatchStatusThread = NULL;
	}

	if (NULL == LibMFSCommCDM_ClosePort)
	{
		return FALSE;
	}

	return LibMFSCommCDM_ClosePort();
}


/** **********************************************************
*	@brief		Reset
*	@param	BOOL bCbxStatusClear	축퇴상태의 카세트를 강제 clear
*	@retval	TRUE 성공
*	@retval	FALSE 실패
************************************************************/
BOOL CDEV_CDM::CDM_Reset(BOOL bCbxStatusClear/*=FALSE*/)
{
	BOOL bResult = FALSE;
#if	EMULATION_CDM_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	return CDevSim::Confirm(_T("CDM"), _T("CDM_Reset"));
#endif

	if (NULL == LibMFSCommCDM_Reset)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	bResult = LibMFSCommCDM_Reset();

	m_CDMLock.Unlock();

	if(FALSE == bResult)
	{
		return FALSE;
	}
	
	if(TRUE == bCbxStatusClear)
	{
		CDMSTATUS status;
		int i=0;

		ZeroMemory(&status, sizeof(CDMSTATUS));

		// 축퇴 상태인 카세트의 상태를 강제 clear
		for(int i=0; i<MAX_CASSETTE; i++)
			m_bCbxStatusEmpty[i] = FALSE;

		BOOL bResult2 = CDM_Status(&status);

		if(TRUE == bResult2)
		{
			for(i=0; i<MAX_CFG_CBX_COUNT+1; i++)
			{
				m_dwCbxStatus[i] = MakeCbxStatus(i, status.sensor[i], status.sensor[0]);
				m_dwPrevCbxStatus[i] = m_dwCbxStatus[i];
			}

			LOG(Info, _T("All Cassette Status is clear."));
		}
		else
		{
			LOG(Error, _T("CDM_Reset() CDM_Status is failed."));
		}
	}

	return TRUE;
}


#if	EMULATION_CDM_DEVICE
// Simulated cassette state helpers - defined further down, next to the
// rest of the simulator support.
static int CDM_SimCbxCount();
static int CDM_SimCbxNoteCount(int nCbxNo);
static int CDM_SimCbxStatus(int nCbxNo);
#endif	// EMULATION_CDM_DEVICE


/** **********************************************************
*	@brief		Get Configuration
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_CfgStatus()
{
	CDMCFGSTATUS	cdmcfgstatus;
	memset(&cdmcfgstatus, 0, sizeof(cdmcfgstatus));

#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	m_nCbxCount = CDM_SimCbxCount();
	m_dwMaxRejectCount = CDevSim::ReadInt(_T("CDM"), _T("MaxRejectCount"), 50);
	m_strWDM_FW_Version.Format(_T("%02d.%02d"), CDevSim::ReadInt(_T("CDM"), _T("FWMajor"), 1), CDevSim::ReadInt(_T("CDM"), _T("FWMinor"), 0));

	// On the real device cassette presence arrives through CDM_Status()'s
	// sensor bytes. The emulated CDM_Status() can only describe the first
	// cassette that way, so seed the status array straight from the
	// per-cassette simulator keys instead - ExecuteMixAlgorithm() reads
	// m_dwCbxStatus[] directly and would otherwise see every cassette but
	// #1 as MISSING.
	m_dwCbxStatus[0] = (DWORD)CDevSim::ReadInt(_T("CDM"), _T("RejectBinStatus"), WFS_CDM_STATCUOK);
	m_dwPrevCbxStatus[0] = m_dwCbxStatus[0];

	for (int nSimCbx = 1; nSimCbx <= MAX_CASSETTE; nSimCbx++)
	{
		m_dwCbxStatus[nSimCbx] = (DWORD)CDM_SimCbxStatus(nSimCbx);
		m_dwPrevCbxStatus[nSimCbx] = m_dwCbxStatus[nSimCbx];
	}

	// Cassette contents live in CBXINFO.CFG, which is all zeros in a fresh
	// simulator checkout - the mix then reports "insufficient amount" for
	// every fast cash amount and the amount screen comes up blank. Seed only
	// the entries still at zero, so cash loaded through the operator menu is
	// left alone.
	CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count.Format(_T("%d"), m_nCbxCount);

	for (int nSeedCbx = 0; nSeedCbx < __min(m_nCbxCount, MAX_CFG_CBX_COUNT); nSeedCbx++)
	{
		TCHAR szSeedKey[32] = { 0, };

		if (CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[nSeedCbx]) <= 0)
		{
			_sntprintf_s(szSeedKey, _countof(szSeedKey), _TRUNCATE, _T("Cbx%dDenomination"), nSeedCbx + 1);

			CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[nSeedCbx].Format(
				_T("%d"), CDevSim::ReadInt(_T("CDM"), szSeedKey, 20));
		}

		if (CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[nSeedCbx]) <= 0)
		{
			CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[nSeedCbx].Format(
				_T("%d"), CDM_SimCbxNoteCount(nSeedCbx + 1));
		}
	}

	return TRUE;

#endif

	BYTE byErrorCode = 0x30;

	if (NULL == LibMFSCommCDM_CfgStatus)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if(TRUE == LibMFSCommCDM_CfgStatus(&cdmcfgstatus, &byErrorCode))
	{
		m_nCbxCount = (cdmcfgstatus.cbx_type & ~0xF0) + 1;

		if(cdmcfgstatus.cbx_type & 0x40)
			m_dwMaxRejectCount = 70;
		else
			m_dwMaxRejectCount = 50;

		LOG(Info, _T("CDM CfgStatus (%02X), m_nCbxCount (%d), m_dwMaxRejectCount (%d)"), cdmcfgstatus.cbx_type, m_nCbxCount, m_dwMaxRejectCount);
		LOG(Info, _T("Firmware Version : [%02d%02d]"), cdmcfgstatus.major_no, cdmcfgstatus.minor_no);

		m_strWDM_FW_Version.Format(_T("%02d.%02d"), cdmcfgstatus.major_no, cdmcfgstatus.minor_no);

		//Save CDM Config
		CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count.Format(_T("%d"), m_nCbxCount);
		CEagleDataManager::GetInstance()->m_Config.SaveCBXInfoData(_T("cbx_cnt"), CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

		m_CDMLock.Unlock();
		return TRUE;
	}

	m_strLastError.Format(_T("D1%04X"), byErrorCode);

	// 통신 장애시 m_nCbxCount값 갱신하도록 수정
	m_nCbxCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

	m_CDMLock.Unlock();

	return FALSE;
}


/** **********************************************************
*	@brief		Get Status
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_Status(LPCDMSTATUS lpStatus, BOOL bCallbyThread/*=FALSE*/)
{
#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	ZeroMemory(lpStatus, sizeof(CDMSTATUS));
	lpStatus->error_cd = CDevSim::ReadByte(_T("CDM"), _T("StatusErrorCode"), 0);
	lpStatus->reject_cd = CDevSim::ReadByte(_T("CDM"), _T("StatusRejectCode"), 0);
	lpStatus->sensor[1] = CDevSim::ReadByte(_T("CDM"), _T("StatusSensor1"), 0x21); // CST1/CST2 Normal

	return TRUE;
#endif

	if (NULL == LibMFSCommCDM_Status)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if (LibMFSCommCDM_Status(lpStatus))
	{
		// Dispense 명령시, CDM_DispenseByAmount() 리턴하기 전에 Thread에서 호출하는 Status 명령이 성공하게 되면 장애코드를 clear함.
		// Thread에서 호출되는 Status 명령에 대해서는 성공 유무에 상관없이 기존 장애코드 유지
		if(FALSE == bCallbyThread)
			m_strLastError.Format(_T(""));

		m_CDMLock.Unlock();
		return TRUE;
	}

	// Dispense 명령시, CDM_DispenseByAmount() 리턴하기 전에 Thread에서 호출하는 Status 명령이 성공하게 되면 장애코드를 clear함.
	// Thread에서 호출되는 Status 명령에 대해서는 성공 유무에 상관없이 기존 장애코드 유지
	//	if(FALSE == bCallbyThread)	// 장애 발생시에는 의미 없기에 주석 처리함.
	m_strLastError.Format(_T("D1%04X"), lpStatus->error_cd);

	m_CDMLock.Unlock();
	return FALSE;

}


/** **********************************************************
*	@brief		Multi-Dispense
*	@retval		없음
************************************************************/
int CDEV_CDM::CDM_MultiDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult)
{
#if	EMULATION_CDM_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	ZeroMemory(lpResult, sizeof(CDMMULTIDISPENSE));

	if (CDevSim::Confirm(_T("CDM"), _T("CDM_MultiDispense"), _T("OK = dispense the requested notes, NG = simulate a dispense error")))
	{
		for (int i = 0; i < MAX_CASSETTE; i++)
			lpResult->count[i] = lpCbxItem->count[i];

		return NO_ERROR;
	}

	lpResult->error_cd = CDevSim::ReadByte(_T("CDM"), _T("DispenseErrorCode"), 0x90);
	return WFS_HARDWARE_ERROR;
#endif

	int nResult = CDM_FAILED_TO_SEND_COMMAND;

	if (NULL == LibMFSCommCDM_MultiDispense)
	{
		return WFS_HARDWARE_ERROR;
	}

	m_CDMLock.Lock();

	memset(&m_lastDispenseInfo, 0, sizeof(CDMLASTDISPENSE));

//	if (LibMFSCommCDM_MultiDispense(lpCbxItem, lpResult))
	nResult = LibMFSCommCDM_MultiDispense(lpCbxItem, lpResult, m_byCEInfoData);

	LOG(Info, _T("MultiDispense Return : [%d]"), nResult);

	if(NO_ERROR == nResult)
	{
		m_strLastError.Format(_T(""));

		m_CDMLock.Unlock();
//		return TRUE;
	} 
	else
	{
		m_strLastError.Format(_T("D1%04X"), lpResult->error_cd);
		
		// 통신장애일 경우, 방출 동작이 수행 중이라면 요청 매수만큼 방출된 것으로 판단
	//	if(0x90 == lpResult->error_cd)
		if(COMMUNICATION_ERROR == m_strLastError)
		{
			if(TRUE == CDM_IsSendDispense())
			{
				LOG(Info, _T("Communication is failed, but device is dispensing."));

				for(int i=0; i<MAX_CASSETTE; i++)
				{
					lpResult->count[i] = lpCbxItem->count[i];
					m_lastDispenseInfo.last_dispense_count[i] = lpCbxItem->count[i];
					m_lastDispenseInfo.last_pick_count[i] = lpCbxItem->count[i];
					m_lastDispenseInfo.last_divert_count[i] = 0;
				}
			}
		}

		m_CDMLock.Unlock();

#if !(FACTORY_TEST)
		if(COMMUNICATION_ERROR != m_strLastError)
			CDM_GetDispenseLog();
#endif
	}

#if (FACTORY_TEST)
	CDM_GetDispenseLog();
#endif

	return nResult;
}

/** **********************************************************
*	@brief		Test-Dispense
*	@retval		없음
************************************************************/
int CDEV_CDM::CDM_TestDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult)
{
#if	EMULATION_CDM_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	ZeroMemory(lpResult, sizeof(CDMMULTIDISPENSE));

	if (CDevSim::Confirm(_T("CDM"), _T("CDM_TestDispense"), _T("OK = simulate a successful test dispense, NG = simulate an error")))
	{
		for (int i = 0; i < MAX_CASSETTE; i++)
			lpResult->count[i] = lpCbxItem->count[i];

		return NO_ERROR;
	}

	lpResult->error_cd = CDevSim::ReadByte(_T("CDM"), _T("DispenseErrorCode"), 0x90);
	return WFS_HARDWARE_ERROR;
#endif

	int nResult = CDM_FAILED_TO_SEND_COMMAND;

	if (NULL == LibMFSCommCDM_TestDispense)
	{
		return WFS_HARDWARE_ERROR;
	}

	m_CDMLock.Lock();

	memset(&m_lastDispenseInfo, 0, sizeof(CDMLASTDISPENSE));

	nResult = LibMFSCommCDM_TestDispense(lpCbxItem, lpResult, m_byCEInfoData);

	LOG(Info, _T("TestDispense Return : [%d]"), nResult);

	if(NO_ERROR == nResult)
	{
		m_strLastError.Format(_T(""));
	} 
	else
	{
		m_strLastError.Format(_T("D1%04X"), lpResult->error_cd);

		// 통신장애일 경우, 방출 동작이 수행 중이라면 요청 매수만큼 방출된 것으로 판단
		if(COMMUNICATION_ERROR == m_strLastError)
		{
			if(TRUE == CDM_IsSendDispense())
			{
				LOG(Info, _T("Communication is failed, but device is dispensing."));

				for(int i=0; i<MAX_CASSETTE; i++)
				{
					lpResult->count[i] = lpCbxItem->count[i];
					m_lastDispenseInfo.last_dispense_count[i] = lpCbxItem->count[i];
					m_lastDispenseInfo.last_pick_count[i] = lpCbxItem->count[i];
					m_lastDispenseInfo.last_divert_count[i] = 0;
				}
			}
		}

		CString strErrorCode = m_strLastError.Right(2);

		if(_T("49") == strErrorCode)
			m_bCbxStatusEmpty[0] = TRUE;
		else if(_T("4A") == strErrorCode)
			m_bCbxStatusEmpty[1] = TRUE;
		else if(_T("4B") == strErrorCode)
			m_bCbxStatusEmpty[2] = TRUE;
		else if(_T("4C") == strErrorCode)
			m_bCbxStatusEmpty[3] = TRUE;
	}

	m_CDMLock.Unlock();
	return nResult;
}


/** **********************************************************
*	@brief		Get Note Sensor Log
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_GetNoteLog()
{
#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	return (BOOL)CDevSim::ReadInt(_T("CDM"), _T("NoteLogAvailable"), 1);
#endif

	if (NULL == LibMFSCommCDM_GetNoteLog)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if(LibMFSCommCDM_GetNoteLog())
	{
		m_CDMLock.Unlock();
		return TRUE;
	} 

	m_CDMLock.Unlock();
	return FALSE;
}

/** **********************************************************
*	@brief		Get Note Thick Log
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_GetThickLog()
{

#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	return (BOOL)CDevSim::ReadInt(_T("CDM"), _T("ThickLogAvailable"), 1);
#endif

	if (NULL == LibMFSCommCDM_GetThickLog)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if(LibMFSCommCDM_GetThickLog())
	{
		m_CDMLock.Unlock();
		return TRUE;
	} 

	m_CDMLock.Unlock();
	return FALSE;
}


/** **********************************************************
*	@brief		Last Dispense
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_LastDispense(LPCDMLASTDISPENSE lpResult)
{
#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	ZeroMemory(lpResult, sizeof(CDMLASTDISPENSE));
	return TRUE;
#endif

	BYTE byErrorCode = 0x30;

	if (NULL == LibMFSCommCDM_LastDispense)
	{
		return FALSE;
	}

	// 방출 동작 중, 통신장애 발생시에는 요청 매수만큼 방출된 것으로 판단하기 때문에
	// LastDispense 응답에도 동일하게 적용해야 함.
	if(COMMUNICATION_ERROR == m_strLastError)
	{
		if(TRUE == CDM_IsSendDispense())
		{
			for(int i=0; i<MAX_CASSETTE; i++)
			{
				lpResult->last_dispense_count[i] = m_lastDispenseInfo.last_dispense_count[i];
				lpResult->last_pick_count[i] = m_lastDispenseInfo.last_pick_count[i];
				lpResult->last_divert_count[i] = m_lastDispenseInfo.last_divert_count[i];
			}

			return TRUE;	// 결과를 성공으로 해야 매수 정산을 하기 때문에 TRUE 리턴함.
		}
	}

	m_CDMLock.Lock();

	if(LibMFSCommCDM_LastDispense(lpResult, &byErrorCode))
	{
		m_CDMLock.Unlock();
		return TRUE;
	} 

	m_strLastError.Format(_T("D1%04X"), byErrorCode);

	m_CDMLock.Unlock();
	return FALSE;
}


/** **********************************************************
*	@brief		Diagnostic
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_Diagnostic(LPCDMDIAGNOSTIC lpResult)
{
#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	ZeroMemory(lpResult, sizeof(CDMDIAGNOSTIC));
	lpResult->error_cd = CDevSim::ReadByte(_T("CDM"), _T("DiagnosticErrorCode"), 0);
	strcpy_s(lpResult->result_msg, sizeof(lpResult->result_msg), "OK (simulated)");
	return TRUE;
#endif

	if (NULL == LibMFSCommCDM_Diagnostic)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if (LibMFSCommCDM_Diagnostic(lpResult))
	{
		m_strLastError.Format(_T(""));

		m_CDMLock.Unlock();
		return TRUE;
	} 

	m_strLastError.Format(_T("D1%04X"), lpResult->error_cd);

	m_CDMLock.Unlock();
	return FALSE;
}


/** **********************************************************
*	@brief		Set Bill Thickness
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_SetBillThickness(LPCDMSETBILLTHICKNESS lpSet, LPBYTE lpbyErrorCd)
{
#if	EMULATION_CDM_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	*lpbyErrorCd = 0;
	return CDevSim::Confirm(_T("CDM"), _T("CDM_SetBillThickness"));
#endif

	if (NULL == LibMFSCommCDM_SetBillThickness)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if (LibMFSCommCDM_SetBillThickness(lpSet, lpbyErrorCd))
	{
		m_strLastError.Format(_T(""));

		m_CDMLock.Unlock();
		return TRUE;
	} 

	m_strLastError.Format(_T("D1%04X"), *lpbyErrorCd);

	m_CDMLock.Unlock();
	return FALSE;
}


/** **********************************************************
*	@brief		Get Bill Thickness
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_GetBillThickness(BYTE byCbxNo, LPCDMGETBILLTHICKNESS lpGet)
{
#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	ZeroMemory(lpGet, sizeof(CDMGETBILLTHICKNESS));
	lpGet->thickness = CDevSim::ReadByte(_T("CDM"), _T("BillThickness"), 10);
	lpGet->margin = CDevSim::ReadByte(_T("CDM"), _T("BillThicknessMargin"), 3);
	return TRUE;
#endif

	if (NULL == LibMFSCommCDM_GetBillThickness)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if (LibMFSCommCDM_GetBillThickness(byCbxNo, lpGet))
	{
		m_strLastError.Format(_T(""));

		m_CDMLock.Unlock();
		return TRUE;
	} 

	m_strLastError.Format(_T("D1%04X"), lpGet->error_cd);

	m_CDMLock.Unlock();
	return FALSE;
}


/** **********************************************************
*	@brief		Set Bill Size
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_SetBillSize(LPCDMSETBILLSIZE lpSet, LPBYTE lpbyErrorCd)
{
#if	EMULATION_CDM_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	*lpbyErrorCd = 0;
	return CDevSim::Confirm(_T("CDM"), _T("CDM_SetBillSize"));
#endif

	if (NULL == LibMFSCommCDM_SetBillSize)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if (LibMFSCommCDM_SetBillSize(lpSet, lpbyErrorCd))
	{
		m_strLastError.Format(_T(""));

		m_CDMLock.Unlock();
		return TRUE;
	} 

	m_strLastError.Format(_T("D1%04X"), *lpbyErrorCd);

	m_CDMLock.Unlock();
	return FALSE;
}


/** **********************************************************
*	@brief		Get Bill Size
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_GetBillSize(BYTE byCbxNo, LPCDMGETBILLSIZE lpGet)
{
#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	ZeroMemory(lpGet, sizeof(CDMGETBILLSIZE));
	lpGet->size = CDevSim::ReadByte(_T("CDM"), _T("BillSize"), 66);
	lpGet->margin = CDevSim::ReadByte(_T("CDM"), _T("BillSizeMargin"), 5);
	return TRUE;
#endif

	if (NULL == LibMFSCommCDM_GetBillSize)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if (LibMFSCommCDM_GetBillSize(byCbxNo, lpGet))
	{
		m_strLastError.Format(_T(""));

		m_CDMLock.Unlock();
		return TRUE;
	} 

	m_strLastError.Format(_T("D1%04X"), lpGet->error_cd);

	m_CDMLock.Unlock();
	return FALSE;
}


/** **********************************************************
*	@brief		Learn Bill
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_LearnBill(BYTE byCbxNo, BYTE byItemCount, LPCDMLEARN lpResult)
{
#if	EMULATION_CDM_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	ZeroMemory(lpResult, sizeof(CDMLEARN));

	if (CDevSim::Confirm(_T("CDM"), _T("CDM_LearnBill")))
	{
		lpResult->size_avg = CDevSim::ReadByte(_T("CDM"), _T("BillSize"), 66);
		lpResult->thickness_avg = CDevSim::ReadByte(_T("CDM"), _T("BillThickness"), 10);
		return TRUE;
	}

	lpResult->error_cd = CDevSim::ReadByte(_T("CDM"), _T("LearnBillErrorCode"), 0x90);
	return FALSE;
#endif

	if (NULL == LibMFSCommCDM_LearnBill)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if (LibMFSCommCDM_LearnBill(byCbxNo, byItemCount, lpResult, m_byCEInfoData))
	{
		m_strLastError.Format(_T(""));

		m_CDMLock.Unlock();
		return TRUE;
	} 

	m_strLastError.Format(_T("D1%04X"), lpResult->error_cd);

	m_CDMLock.Unlock();
	return FALSE;
}


/** **********************************************************
*	@brief		Get Dll Version
*	@retval		없음
************************************************************/
BOOL CDEV_CDM::CDM_GetDLLVersion(LPBYTE major_num, LPBYTE minor_num)
{
#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	major_num[0] = CDevSim::ReadByte(_T("CDM"), _T("DllVersionMajor"), 0x01);
	minor_num[0] = CDevSim::ReadByte(_T("CDM"), _T("DllVersionMinor"), 0x00);

	return TRUE;
#endif

	if (NULL == LibMFSCommCDM_GetDLLVersion)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if(LibMFSCommCDM_GetDLLVersion(major_num, minor_num))
	{
		m_CDMLock.Unlock();
		return TRUE;
	} 

	m_CDMLock.Unlock();
	return FALSE;
}


/** **********************************************************
*	@brief		Dispense By Amount
*	@retval		Dispense Result
************************************************************/
BOOL CDEV_CDM::CDM_DispenseByAmount(UINT nAmount, UINT nCbxCount, UINT nCbxDenom[4], UINT nCbxRemain[4], LPCDMMULTIDISPENSE lpResult)
{
#if	EMULATION_CDM_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	ZeroMemory(lpResult, sizeof(CDMMULTIDISPENSE));

	CString strDetail;
	strDetail.Format(_T("Requested amount : %u"), nAmount);

	if (CDevSim::Confirm(_T("CDM"), _T("CDM_DispenseByAmount"), strDetail))
		return TRUE;

	lpResult->error_cd = CDevSim::ReadByte(_T("CDM"), _T("DispenseErrorCode"), 0x90);
	return FALSE;
#endif
//////////////////////////////////////////////////////////////////////////
	memcpy(m_nCbxDenom, nCbxDenom, sizeof(UINT)*4);

	LOG(Info, _T("CbxDenom = %d, %d, %d, %d"), m_nCbxDenom[0], m_nCbxDenom[1], m_nCbxDenom[2], m_nCbxDenom[3]);

	memcpy(m_nCbxRemain, nCbxRemain, sizeof(UINT) * 4);

	int i = 0, j = 0;
	int nMixValue[] = { 100, 50, 20, 10, 5, 2, 1};	//Mix Table
	int nItemCount[5] = {0, };					//Current Item Count
	UINT nTotalAmount = 0;						//Possible Amount
	int cbx_cnt = 0, cbx_no = 0;
	int item_sub_cnt = 0;
	int item_req_cnt = 0;
	int nRemain = 0;
	int nCurrent = 0;

	// Initial Item Count
	CDMCBXITEM	cdmcbxitem;
	memset(&cdmcbxitem, 0, sizeof(cdmcbxitem));

	// Set Cassette Item/Denomination (4 CST)
	for (i = 0; i < 4; i++)
	{
		nItemCount[i] = m_nCbxRemain[i];

		nTotalAmount += m_nCbxDenom[i] * nItemCount[i];
	}

	// Request Amount Check
	if (nTotalAmount < nAmount)
	{

		return FALSE;
	}

	// Calculate Item Count
	nRemain = nAmount;

	// Loop to Mix Table Size
	for (i = 0; i < 7; i++)
	{
		if (nRemain >= nMixValue[i])
		{
			// 동일한 권종의 카세트 수 구하기
			cbx_cnt = GetSameCBXDenomCount(nMixValue[i]);

			//
			LOG(Info, _T("MixVal = %d, cnt = %d"), nMixValue[i], cbx_cnt);

			item_req_cnt = (int)(nRemain / nMixValue[i]);

			item_sub_cnt = 0;

			for (j = 0; j < cbx_cnt; j++)
			{
				// 카세트 index 구하기
				cbx_no = GetCBXIndex(nMixValue[i], j);
				nCurrent = nItemCount[cbx_no];

				if (nCurrent >= item_req_cnt)
				{
					cdmcbxitem.count[cbx_no] = item_req_cnt;
					item_sub_cnt += item_req_cnt;

					item_req_cnt = 0;

					//
					LOG(Info, _T("MixVal = %d, idx = %d"), nMixValue[i], cbx_no);

					break;
				}
				else if (nCurrent < item_req_cnt)
				{
					cdmcbxitem.count[cbx_no] = nCurrent;
					item_sub_cnt += nCurrent;
					item_req_cnt = item_req_cnt - nCurrent;
				}
			}

			nRemain = nRemain - (int)(nMixValue[i] * item_sub_cnt);

			if (nRemain == 0)
			{
				break;
			}
		}
	}

	LOG(Info, _T("CST1 = %d, CST2 = %d"), cdmcbxitem.count[0], cdmcbxitem.count[1]);

	// Return Result
	return CDM_MultiDispense(&cdmcbxitem, lpResult);
}


/** **********************************************************
*	@brief		Dispense By Amount
*	@retval		Dispense Result and Amount
************************************************************/
int CDEV_CDM::CDM_DispenseByAmount( UINT nAmount, LPCDMMULTIDISPENSE lpResult, UINT* nDispenseAmount )
{
#if	EMULATION_CDM_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	// This entry point short-circuits before reaching CDM_MultiDispense, so the
	// confirmation has to happen here or the main dispense path would never prompt.
	ZeroMemory(lpResult, sizeof(CDMMULTIDISPENSE));

	CString strDetail;
	strDetail.Format(_T("Requested amount : %u"), nAmount);

	if (CDevSim::Confirm(_T("CDM"), _T("CDM_DispenseByAmount"), strDetail))
	{
		if (nDispenseAmount != NULL)
			*nDispenseAmount = nAmount;

		return NO_ERROR;
	}

	if (nDispenseAmount != NULL)
		*nDispenseAmount = 0;

	lpResult->error_cd = CDevSim::ReadByte(_T("CDM"), _T("DispenseErrorCode"), 0x90);
	return WFS_HARDWARE_ERROR;
#endif

	BOOL bReturn = FALSE;
	BOOL bLastDispenseReturn = FALSE;
	int i = 0;
	int nResult = CDM_FAILED_TO_SEND_COMMAND;

	// Initial Item Count
	CDMCBXITEM	cdmcbxitem;
	memset(&cdmcbxitem, 0, sizeof(cdmcbxitem));

	// 카세트 개수 취득
	int nCbxCount = m_nCbxCount;

	bReturn = ExecuteMixAlgorithm(nAmount, &cdmcbxitem);

	if(FALSE == bReturn)
		//return WFS_HARDWARE_ERROR;
		return WFS_NOTDISPENSABLE;		// Mix error code 추가 (2021.03.23)

	for (i = 0; i < __min(nCbxCount, 4); i++)
	{
		LOG(Info, _T("Set >>> CST[%d] = %d"), i+1, cdmcbxitem.count[i]);
	}

	nResult = CDM_MultiDispense(&cdmcbxitem, lpResult);

//////////////////////////////////////////////////////////////////////////
	CString strNumber = _T("");
	CString strValue = _T("");

	int nTemp = 0;

	// Call LastDispensed Information
	// Current(PickUp매수) / Dispens / Reject 매수 갱신을 위해 Last Dispens 함수 호출
	CDMLASTDISPENSE last_dispense_info;
	memset(&last_dispense_info, 0, sizeof(last_dispense_info));
	// FW에서는 Dispense 명령을 받았을 때, LastDispense를 초기화 하기 때문에,
	// Dispense 명령의 CMD전송 실패시에는 LastDispense 명령을 수행하지 않는다.(이전 data가 올라올 수 있으므로)
	if(CDM_FAILED_TO_SEND_COMMAND != nResult)
		bLastDispenseReturn = CDM_LastDispense(&last_dispense_info);

	*nDispenseAmount = 0;
	for (i = 0; i < __min(nCbxCount, 4); i++)
	{
		//Calc Result Amount
		*nDispenseAmount += m_nCbxDenom[i] * lpResult->count[i];

		LOG(Info, _T("Disp <<< CST[%d] = %d"), i+1, lpResult->count[i]);

		if (bLastDispenseReturn == TRUE)
		{
			// 1. Current Count 차감
			if (m_nCbxRemain[i] > last_dispense_info.last_pick_count[i]) {
				m_nCbxRemain[i] -= last_dispense_info.last_pick_count[i];
			}
			else {
				m_nCbxRemain[i] = 0;
			}

#if (SUPPORT_NOTE_COUNTING)	// 논리적인 매수만 차감하지 않는다
			strNumber.Format(_T("%d"), i + 1);
			CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i].Format(_T("%d"), m_nCbxRemain[i]);
			CEagleDataManager::GetInstance()->m_Config.SaveCBXInfoData(_T("cbx_item_count_") + strNumber, CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);
			LOG(Info, _T("Disp <<< Current Count(CST_%d) = %d"), i+1, m_nCbxRemain[i]);
#endif

			// update dispensed count
			nTemp = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[i]);
			nTemp += last_dispense_info.last_dispense_count[i];
			CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[i].Format(_T("%d"), nTemp);
			CEagleDataManager::GetInstance()->m_Config.SaveCBXInfoData(_T("cbx_dispense_count_") + strNumber, CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[i]);
			LOG(Info, _T("Disp <<< Dispensed Count(CST_%d) = %d"), i+1, nTemp);

			// update reject count
			nTemp = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[i]);
			nTemp += last_dispense_info.last_divert_count[i];
			CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[i].Format(_T("%d"), nTemp);
			CEagleDataManager::GetInstance()->m_Config.SaveCBXInfoData(_T("cbx_reject_count_") + strNumber, CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[i]);
			LOG(Info, _T("Disp <<< Rejected Count(CST_%d) = %d"), i+1, nTemp);
		}
	}

	if (NO_ERROR != nResult)
	{
		CString strErrorCode = m_strLastError.Right(2);

		if(_T("49") == strErrorCode)
			m_bCbxStatusEmpty[0] = TRUE;
		else if(_T("4A") == strErrorCode)
			m_bCbxStatusEmpty[1] = TRUE;
		else if(_T("4B") == strErrorCode)
			m_bCbxStatusEmpty[2] = TRUE;
		else if(_T("4C") == strErrorCode)
			m_bCbxStatusEmpty[3] = TRUE;
	}

	return nResult;
}


/** **********************************************************
*	@brief		Dispense By Count
*	@retval		Dispense Result and Amount
************************************************************/
int CDEV_CDM::CDM_DispenseByCount(LPCDMMIXRESULT lpCdmMixResult, LPCDMMULTIDISPENSE lpResult, UINT* nDispenseAmount)
{
	//#if	EMULATION_CDM_DEVICE
	//	return NO_ERROR;
	//#endif

	BOOL bReturn = FALSE;
	BOOL bLastDispenseReturn = FALSE;
	int i = 0;
	int nResult = CDM_FAILED_TO_SEND_COMMAND;

	// Initial Item Count
	CDMCBXITEM	cdmcbxitem;
	memset(&cdmcbxitem, 0, sizeof(cdmcbxitem));

	// 카세트 개수 취득
	int nCbxCount = m_nCbxCount;

	for (i = 0; i < __min(nCbxCount, 4); i++)
	{
		cdmcbxitem.count[i] = (BYTE)lpCdmMixResult->nCount[i];
		LOG(Info, _T("Set >>> CST[%d] = %d"), i + 1, cdmcbxitem.count[i]);
	}

	nResult = CDM_MultiDispense(&cdmcbxitem, lpResult);

	//////////////////////////////////////////////////////////////////////////
	CString strNumber = _T("");
	CString strValue = _T("");

	int nTemp = 0;

	// Call LastDispensed Information
	// Current(PickUp매수) / Dispens / Reject 매수 갱신을 위해 Last Dispens 함수 호출
	CDMLASTDISPENSE last_dispense_info;
	memset(&last_dispense_info, 0, sizeof(last_dispense_info));
	// FW에서는 Dispense 명령을 받았을 때, LastDispense를 초기화 하기 때문에,
	// Dispense 명령의 CMD전송 실패시에는 LastDispense 명령을 수행하지 않는다.(이전 data가 올라올 수 있으므로)
	if (CDM_FAILED_TO_SEND_COMMAND != nResult)
		bLastDispenseReturn = CDM_LastDispense(&last_dispense_info);

	*nDispenseAmount = 0;
	for (i = 0; i < __min(nCbxCount, 4); i++)
	{
		//Calc Result Amount
		*nDispenseAmount += m_nCbxDenom[i] * lpResult->count[i];

		LOG(Info, _T("Disp <<< CST[%d] = %d"), i + 1, lpResult->count[i]);

		if (bLastDispenseReturn == TRUE)
		{
			// 1. Current Count 차감
			if (m_nCbxRemain[i] > last_dispense_info.last_pick_count[i]) {
				m_nCbxRemain[i] -= last_dispense_info.last_pick_count[i];
			}
			else {
				m_nCbxRemain[i] = 0;
			}

#if (SUPPORT_NOTE_COUNTING)	// 논리적인 매수만 차감하지 않는다
			strNumber.Format(_T("%d"), i + 1);
			CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i].Format(_T("%d"), m_nCbxRemain[i]);
			CEagleDataManager::GetInstance()->m_Config.SaveCBXInfoData(_T("cbx_item_count_") + strNumber, CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);
			LOG(Info, _T("Disp <<< Current Count(CST_%d) = %d"), i + 1, m_nCbxRemain[i]);
#endif

			// update dispensed count
			nTemp = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[i]);
			nTemp += last_dispense_info.last_dispense_count[i];
			CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[i].Format(_T("%d"), nTemp);
			CEagleDataManager::GetInstance()->m_Config.SaveCBXInfoData(_T("cbx_dispense_count_") + strNumber, CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[i]);
			LOG(Info, _T("Disp <<< Dispensed Count(CST_%d) = %d"), i + 1, nTemp);

			// update reject count
			nTemp = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[i]);
			nTemp += last_dispense_info.last_divert_count[i];
			CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[i].Format(_T("%d"), nTemp);
			CEagleDataManager::GetInstance()->m_Config.SaveCBXInfoData(_T("cbx_reject_count_") + strNumber, CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[i]);
			LOG(Info, _T("Disp <<< Rejected Count(CST_%d) = %d"), i + 1, nTemp);
		}
	}

	if (NO_ERROR != nResult)
	{
		CString strErrorCode = m_strLastError.Right(2);

		if (_T("49") == strErrorCode)
			m_bCbxStatusEmpty[0] = TRUE;
		else if (_T("4A") == strErrorCode)
			m_bCbxStatusEmpty[1] = TRUE;
		else if (_T("4B") == strErrorCode)
			m_bCbxStatusEmpty[2] = TRUE;
		else if (_T("4C") == strErrorCode)
			m_bCbxStatusEmpty[3] = TRUE;
	}

	return nResult;
}


/** **********************************************************
*	@brief		Get Cassette Box Count
*	@retval		없음
************************************************************/
int CDEV_CDM::GetSameCBXDenomCount(int nMixValue)
{
	int nRet = 0;

	if (m_nCbxDenom[0] == nMixValue)
	{
		nRet += 1;
	}

	if (m_nCbxDenom[1] == nMixValue)
	{
		nRet += 1;
	}

	if (m_nCbxDenom[2] == nMixValue)
	{
		nRet += 1;
	}

	if (m_nCbxDenom[3] == nMixValue)
	{
		nRet += 1;
	}

	return nRet;
}


/** **********************************************************
*	@brief		Get Cassette Box Index
*	@retval		없음
************************************************************/
int CDEV_CDM::GetCBXIndex(int nMixValue, int nIndex, int nDispType/*=0*/)
{
	int nRet = 0;
	int i = 0, j = 0;
	int nBoxNo[5] = { 0, };
	int nEqualCbxCount = 0;
	int nTempCbxNumber = 0;
	
	for (i = 0; i < __min(m_nCbxCount, 4); i++)
	{
		if (m_nCbxDenom[i] == nMixValue)
		{
			nBoxNo[j++] = i;
		}
	}

	nEqualCbxCount = j;
	
	// 방출순서 옵션에 따른 카세트 index 산출
/*LEH 추후 테스트	if(DISP_TYPE_ORDER != nDispType)	// 순차방출이 아닐 경우
	{
		if(nEqualCbxCount > 1)
		{
			for(i=0; i<nEqualCbxCount-1; i++)
			{
				for(j=i+1; j<nEqualCbxCount; j++)
				{
					if(DISP_TYPE_EQUAL == nDispType)	// 균등방출
					{
						// 잔여매수 기준으로 내림차순 정렬
						if(m_nCbxRemain[nBoxNo[i]] < m_nCbxRemain[nBoxNo[j]])
						{
							nTempCbxNumber = nBoxNo[i];
							nBoxNo[i] = nBoxNo[j];
							nBoxNo[j] = nTempCbxNumber;
						}
					}
					else								// 역순차방출
					{
						nTempCbxNumber = nBoxNo[i];
						nBoxNo[i] = nBoxNo[j];
						nBoxNo[j] = nTempCbxNumber;
					}
				}
			}
		}
	}
	////////////////////////////////////////////////////////////
*/
	nRet = nBoxNo[nIndex];

	return nRet;
}

/** **********************************************************
*	@brief		에러코드 취득
*	@retval		없음
************************************************************/
CString CDEV_CDM::GetLastError(void)
{
	return m_strLastError;
}

/** **********************************************************
*	@brief		지폐의 Size와 Thickness 정보 취득
*	@param	int nCbxCount					카세트 수
*	@param	CDM_BILL_INFO &billInfo	지폐 정보를 저장할 구조체
*	@retval	TRUE	GetBillInfo 성공
*	@retval	FALSE	GetBillInfo 실패
************************************************************/
BOOL CDEV_CDM::CDM_GetBillInfo(int nCbxCount, CDM_BILL_INFO &billInfo)
{
	BOOL bResult = FALSE;
	int i = 0;

#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	ZeroMemory(&billInfo, sizeof(CDM_BILL_INFO));

	for (int nSimCbx = 0; nSimCbx < MAX_CASSETTE; nSimCbx++)
	{
		billInfo.bySize[nSimCbx] = CDevSim::ReadByte(_T("CDM"), _T("BillSize"), 66);
		billInfo.bySizeMargin[nSimCbx] = CDevSim::ReadByte(_T("CDM"), _T("BillSizeMargin"), 5);
		billInfo.byThickness[nSimCbx] = CDevSim::ReadByte(_T("CDM"), _T("BillThickness"), 10);
		billInfo.byThicknessMargin[nSimCbx] = CDevSim::ReadByte(_T("CDM"), _T("BillThicknessMargin"), 3);
	}

	return TRUE;
#endif
	
	CDMGETBILLSIZE billSize;
	CDMGETBILLTHICKNESS billThick;

	ZeroMemory(&billSize, sizeof(CDMGETBILLSIZE));
	ZeroMemory(&billThick, sizeof(CDMGETBILLTHICKNESS));

	for(i=0; i<nCbxCount; i++)
	{
		bResult = CDM_GetBillSize(i, &billSize);
		if(FALSE == bResult)
		{
			LOG(Info, _T("CDM_GetBillSize is failed."));
			break;
		}
		else
		{
			billInfo.bySize[i] = billSize.size;
			billInfo.bySizeMargin[i] = billSize.margin;
		}

		bResult = CDM_GetBillThickness(i, &billThick);
		if(FALSE == bResult)
		{
			LOG(Info, _T("CDM_GetBillThickness is failed."));
			break;
		}
		else
		{
			billInfo.byThickness[i] = billThick.thickness;
			billInfo.byThicknessMargin[i] = billThick.margin;
		}
	}

	return bResult;
}

/** **********************************************************
*	@brief		지폐의 Size와 Thickness 정보 설정
*	@param	int nCbxNumber					카세트 번호
*	@param	CDM_BILL_INFO billInfo	지폐 정보 구조체
*	@retval	TRUE	SetBillInfo 성공
*	@retval	FALSE	SetBillInfo 실패
************************************************************/
BOOL CDEV_CDM::CDM_SetBillInfo(int nCbxNumber, CDM_BILL_INFO billInfo)
{
	BOOL bResult = FALSE;

#if	EMULATION_CDM_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	return CDevSim::Confirm(_T("CDM"), _T("CDM_SetBillInfo"));
#endif

	CDMSETBILLSIZE billSize;
	CDMSETBILLTHICKNESS billThick;
	BYTE byErrorCode[6] = {0,};

	ZeroMemory(&billSize, sizeof(CDMSETBILLSIZE));
	ZeroMemory(&billThick, sizeof(CDMSETBILLTHICKNESS));

	billSize.cbx_no = nCbxNumber;
	billSize.size = billInfo.bySize[nCbxNumber];
	billSize.margin = billInfo.bySizeMargin[nCbxNumber];

	billThick.cbx_no = nCbxNumber;
	billThick.thickness = billInfo.byThickness[nCbxNumber];
	billThick.margin = billInfo.byThicknessMargin[nCbxNumber];

	bResult = CDM_SetBillSize(&billSize, byErrorCode);
	if(FALSE == bResult)
	{
		LOG(Info, _T("CDM_SetBillSize is failed."));
		return FALSE;
	}

	bResult = CDM_SetBillThickness(&billThick, byErrorCode);
	if(FALSE == bResult)
	{
		LOG(Info, _T("CDM_SetBillThickness is failed."));
		return FALSE;
	}

	return TRUE;
}

/** **********************************************************
*	@brief	Reject Code 취득
*	@param	LPCDM_REJECT_LOG lpRejectLog : Reject 정보
*	@retval	TRUE	성공
*	@retval	FALSE	실패
************************************************************/
BOOL CDEV_CDM::CDM_GetRejectLog(LPCDM_REJECT_INFO lpRejectLog)
{
#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	ZeroMemory(lpRejectLog, sizeof(CDM_REJECT_INFO));
	return TRUE;
#endif

	BYTE byErrorCode = 0x30;

	if (NULL == LibMFSCommCDM_GetRejectLog)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if (LibMFSCommCDM_GetRejectLog(lpRejectLog, &byErrorCode))
	{
		CString strTemp = _T(""), strTemp2 = _T("");
		for(int i=0; i<lpRejectLog->byLogLen; i++)
		{
			strTemp.Format(_T("0x%02x "), lpRejectLog->byRejectInfo[i]);
			strTemp2 += strTemp;
		}
		LOG(Info, _T("Reject lpRejectLog->byRejectInfo: [%s]"), strTemp2);

		m_CDMLock.Unlock();
		return TRUE;
	} 
	else
	{
		LOG(Info, _T("LibMFSCommCDM_GetRejectLog is failed. ErrorCode:[0x%02x]"), byErrorCode);
	
		m_CDMLock.Unlock();
		return FALSE;
	}
}


/** **********************************************************
*	@brief	Sensor Log 취득
*	@retval	TRUE	성공
*	@retval	FALSE	실패
************************************************************/
BOOL CDEV_CDM::CDM_GetDispenseLog()
{
#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	return (BOOL)CDevSim::ReadInt(_T("CDM"), _T("DispenseLogAvailable"), 1);
#endif

	CDMRECVDATA recvData;
	BYTE byErrorCode = 0x30;

	if (NULL == LibMFSCommCDM_GetDispenseLog)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if (LibMFSCommCDM_GetDispenseLog(&recvData, &byErrorCode))
	{
		LOG(Info, _T("Dispense Log size: [%d]"), recvData.dwSize);

		if(recvData.dwSize >= 3)
		{
			WriteLogDataToLOG(recvData.lpbBody, recvData.dwSize);
			WriteLogDataToCSV(recvData.lpbBody, recvData.dwSize);
		}
		else
		{
			LOG(Info, _T("Dispense Log size is invalid!!!"));
		}

		m_CDMLock.Unlock();
		return TRUE;
	} 

	LOG(Info, _T("LibMFSCommCDM_GetDispenseLog is failed. ErrorCode:[0x%02x]"), byErrorCode);
	
	m_CDMLock.Unlock();
	return FALSE;
}


/** **********************************************************
*	@brief	Firmware Update
*	@retval	TRUE	성공
*	@retval	FALSE	실패
************************************************************/
BOOL CDEV_CDM::CDM_UpdateFirmware()
{
#if	EMULATION_CDM_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	return CDevSim::Confirm(_T("CDM"), _T("CDM_UpdateFirmware"), _T("OK = simulate a successful firmware update, NG = simulate a failure"));
#endif

	BYTE byErrorCode = 0x30;
	BOOL bResult = FALSE;
	CString strTempCode = _T("");
	
	if (NULL == LibMFSCommCDM_UpdateFirmware)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	for(int i=0; i<4; i++)
	{
		if (LibMFSCommCDM_UpdateFirmware(CUtil::GetAppPath(), &byErrorCode, m_byCEInfoData))
		{
			m_strLastError.Format(_T(""));

			bResult = TRUE;
			break;
		} 
		else
		{
			LOG(Info, _T("LibMFSCommCDM_UpdateFirmware is failed. ErrorCode:[0x%02x]"), byErrorCode);
			m_strLastError.Format(_T("D1%04X"), byErrorCode);

			if(3 == i)
			{
				strTempCode.Format(_T("%02X"), byErrorCode);

				bResult = FALSE;
				break;
			}
			else
			{
				LOG(Info, _T("Frimware Download is failed. one more retry..."));
			}
		}
	}

	m_CDMLock.Unlock();

	return bResult;
}


/** **********************************************************
*	@brief	CE부 인증
*	@retval	TRUE	성공
*	@retval	FALSE	실패
************************************************************/
BOOL CDEV_CDM::CDM_CEInfoCertification()
{
#if	EMULATION_CDM_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	return CDevSim::Confirm(_T("CDM"), _T("CDM_CEInfoCertification"));
#endif

	BYTE byErrorCode = 0x30;
	
	if (NULL == LibMFSCommCDM_CEInfoCertification)
	{
		return FALSE;
	}

	m_CDMLock.Lock();

	if (LibMFSCommCDM_CEInfoCertification(m_byCEInfoData, &byErrorCode))
	{	
		m_CDMLock.Unlock();
		return TRUE;
	} 

	m_strLastError.Format(_T("D1%04X"), byErrorCode);
		
	m_CDMLock.Unlock();
	return FALSE;
}


/** **********************************************************
*	@brief	방출 동작 중인지 판단
*	@retval	TRUE	방출 동작 중
*	@retval	FALSE	방출 미동작
************************************************************/
BOOL CDEV_CDM::CDM_IsSendDispense()
{
#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	// Defaults to FALSE so a simulated failure never silently claims that notes
	// were already dispensed (which would corrupt the dispense accounting).
	return (BOOL)CDevSim::ReadInt(_T("CDM"), _T("IsSendDispense"), 0);
#endif

	if (NULL == LibMFSCommCDM_IsSendDispense)
	{
		return FALSE;
	}

	return LibMFSCommCDM_IsSendDispense();
}


/** **********************************************************
*	@brief	Firmware에 CE부 인증 정보가 있는지 여부
*	@retval	TRUE	성공
*	@retval	FALSE	실패
************************************************************/
BOOL CDEV_CDM::CDM_GetExistCEInfo(BOOL &bExist)
{
#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	bExist = (BOOL)CDevSim::ReadInt(_T("CDM"), _T("ExistCEInfo"), 1);
	return TRUE;
#endif

	BYTE byErrorCode = 0x30;
	CString strCEInfoData;								// 진짜 UID 정보(16자리)
	CString strTempCEInfoData = _T("6de8b4c2ab1f590d");	// UID 뒤에 붙는 임시 정보(16자리) 
	BYTE byTempCEInfo[8] = {0,};
	
	if (NULL == LibMFSCommCDM_GetExistCEInfo)
	{
		return FALSE;
	}

	// GetExistCEInfo()로 위치 이동, Firmware에 CE부 인증 정보가 있으면 CE부 인증 CMD(0x61)를 전송하지 않으므로...
	CUtil::GetUniqueKey(strCEInfoData);

	strCEInfoData += strTempCEInfoData;
	WideCharToMultiByte(CP_ACP, 0, strCEInfoData, -1, (char*)m_byCEInfoData, CE_INFO_LENGTH, NULL, NULL);

	m_CDMLock.Lock();

	if (LibMFSCommCDM_GetExistCEInfo(m_byCEInfoData, &byErrorCode, bExist))
	{	
		m_CDMLock.Unlock();
		return TRUE;
	} 

	m_strLastError.Format(_T("D1%04X"), byErrorCode);

	m_CDMLock.Unlock();
	return FALSE;
}


/** **********************************************************
*	@brief	방출 동작 중인지 판단
*	@retval	TRUE	방출 동작 중
*	@retval	FALSE	방출 미동작
************************************************************/
BOOL CDEV_CDM::CDM_Initialize(BOOL bCbxStatusClear/*=FALSE*/)
{
	BOOL bResult = FALSE;

	CDM_Reset(bCbxStatusClear);

	CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_3000MS);	// Wait device

	CDMSTATUS stResult;
	memset(&stResult, 0, sizeof(CDMSTATUS));

	bResult = CDM_Status(&stResult);
	if (FALSE == bResult)
	{
#if !(FACTORY_TEST)
		if(COMMUNICATION_ERROR != m_strLastError)
			CDM_GetDispenseLog();
#endif
	}
	
#if (FACTORY_TEST)
	CDM_GetDispenseLog();
#endif
	
	return bResult;
}


/** **********************************************************
*	@brief		CDM Thread 생성
*	@retval	없음
************************************************************/
void CDEV_CDM::StartCDMThread()
{
	m_WatchStatusThread = AfxBeginThread((AFX_THREADPROC)WatchStatus, this);

	return;
}

/** **********************************************************
*	@brief		Status 감시 시작
*	@retval		없음
************************************************************/
UINT CDEV_CDM::WatchStatus( LPVOID pParam )
{
#if	EMULATION_CDM_DEVICE
	return TRUE;
#endif

	CDEV_CDM* pCDM = (CDEV_CDM*)pParam;

	pCDM->ThreadDeviceStatus();

	return 0;
}


BOOL	m_bDoingThread = FALSE;
/** **********************************************************
*	@brief		Status Thread 수행
*	@retval		없음
************************************************************/
void CDEV_CDM::ThreadDeviceStatus()
{
	LOG(Info, _T("[CDEV_CDM] ThreadDeviceStatus Thread Start"));

	CDMSTATUS status;
	int i=0;
	BOOL bStatusFail = FALSE;
	int nStatusRetryCnt = 0;
	BOOL bResult = FALSE;

	while (TRUE)
	{
		// Close시 Thread 종료
		if(m_bThreadExit)
			break;

		if(TRUE == m_bCDMExecuteQuiryStatus)
		{
			m_bDoingThread = TRUE;

			memset(&status, 0, sizeof(status));
			bResult = CDM_Status(&status, TRUE);
//			if (bResult == FALSE)
			if((bResult == FALSE) && (COMMUNICATION_ERROR == m_strLastError))	// Return이 False도 추가적으로 참조하도록 로직 수정
			{
				if (bStatusFail == FALSE)
				{
					nStatusRetryCnt++;

					if (nStatusRetryCnt > 100)	// 100번 Read Status Fail시 통신 장애로 간주하여 로그를 한번 남긴다.
					{
						bStatusFail = TRUE;
						LOG(Error, _T("CDM_Status Function Error - Check WDM Cable"));
					}
				}

				CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
			}
			else
			{
				for(i=0; i<MAX_CFG_CBX_COUNT+1; i++)
				{
					m_dwCbxStatus[i] = MakeCbxStatus(i, status.sensor[i], status.sensor[0]);
					
					if (m_dwPrevCbxStatus[i] != m_dwCbxStatus[i])
					{
						if(0 == i)
						{
							LOG(Info, _T("[CDEV_CDM] REJ Status Changed : [%d] -> [%d]"), m_dwPrevCbxStatus[i], m_dwCbxStatus[i]);
						}
						else
						{
							LOG(Info, _T("[CDEV_CDM] CBX #%d Status Changed : [%d] -> [%d]"), i, m_dwPrevCbxStatus[i], m_dwCbxStatus[i]);
						}

						m_dwPrevCbxStatus[i] = m_dwCbxStatus[i];
					}
				}
			}
		}

		m_bDoingThread = FALSE;
		CUtil::Sleep_Wait(CHECK_STATUS_INTERVAL);
	}

	LOG(Info, _T("[CDEV_CDM] ThreadDeviceStatus Thread Exit"));
}

/** **********************************************************
*	@brief	카세트 상태 매핑
*	@param	int nCbxNumber	: 카세트 번호
*	@param	BYTE byStatus	: EP로부터의 센서 정보
*	@param	BYTE byStatus2	: 1CBX의 Low 판단을 위함
*	@retval	DWORD dwCbxStatus : CbxStatus 값
************************************************************/
DWORD CDEV_CDM::MakeCbxStatus(int nCbxNumber, BYTE byStatus, BYTE byStatus2/*=0*/)
{
	DWORD dwCbxStatus = WFS_CDM_STATCUMISSING;
	int nCbxCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

	if(nCbxNumber > nCbxCount)
		return dwCbxStatus;

	switch(nCbxNumber)
	{
	case 0:	// Reject Bin
		if(CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strRBN_Item_Count) >= (int)m_dwMaxRejectCount)
			dwCbxStatus = WFS_CDM_STATCUFULL;
		else
			dwCbxStatus = WFS_CDM_STATCUOK;
		break;
	case 1:	// CBX #1
		if( !(byStatus & 0x01) )
			dwCbxStatus = WFS_CDM_STATCUMISSING;
		else if(byStatus2 & 0x20)
			dwCbxStatus = WFS_CDM_STATCULOW;
		else
			dwCbxStatus = WFS_CDM_STATCUOK;
		break;

	case 2:	// CBX #2
		if( !(byStatus & 0x20))
			dwCbxStatus = WFS_CDM_STATCUMISSING;
		else if(byStatus & 0x08)
			dwCbxStatus = WFS_CDM_STATCULOW;
		else
			dwCbxStatus = WFS_CDM_STATCUOK;
		break;

	case 3:	// CBX #3
		if( !(byStatus & 0x30) )
			dwCbxStatus = WFS_CDM_STATCUMISSING;
		//else if(byStatus & 0x80)
		else if(byStatus & 0x08)				// Bug Fixed
			dwCbxStatus = WFS_CDM_STATCULOW;
		else
			dwCbxStatus = WFS_CDM_STATCUOK;
		break;

	case 4:	// CBX #4
		if( !(byStatus & 0x40) )
			dwCbxStatus = WFS_CDM_STATCUMISSING;
		//else if(byStatus & 0x80)
		else if(byStatus & 0x08)				// Bug Fixed
			dwCbxStatus = WFS_CDM_STATCULOW;
		else
			dwCbxStatus = WFS_CDM_STATCUOK;
		break;

	default:
		break;
	}

	if( (0 != nCbxNumber) && (WFS_CDM_STATCUMISSING != dwCbxStatus) )
	{
		if(TRUE == m_bCbxStatusEmpty[nCbxNumber - 1])
			dwCbxStatus = WFS_CDM_STATCUEMPTY;
	}

	return dwCbxStatus;
}

#if	EMULATION_CDM_DEVICE
/** **********************************************************
*	@brief	Simulated cassette state, read from DeviceSim.ini.
*
*	Keys in the [CDM] section:
*		CbxCount          - number of cassettes fitted
*		Cbx<n>NoteCount   - notes in cassette <n> (1 based)
*		Cbx<n>Status      - WFS_CDM_STATCUxxx for cassette <n>
*		RejectBinStatus   - status of the reject bin
*
*	Cassettes beyond CbxCount report MISSING, which is what the real
*	device does for a slot with no cassette in it.
************************************************************/
static int CDM_SimCbxCount()
{
	int nCount = CDevSim::ReadInt(_T("CDM"), _T("CbxCount"), 4);

	if (nCount < 0)
		nCount = 0;

	if (nCount > MAX_CASSETTE)
		nCount = MAX_CASSETTE;

	return nCount;
}

// nCbxNo is 1 based, matching the Cbx<n>... key names.
static int CDM_SimCbxNoteCount(int nCbxNo)
{
	TCHAR szKey[32] = { 0, };

	if (nCbxNo < 1 || nCbxNo > CDM_SimCbxCount())
		return 0;

	_sntprintf_s(szKey, _countof(szKey), _TRUNCATE, _T("Cbx%dNoteCount"), nCbxNo);

	int nNoteCount = CDevSim::ReadInt(_T("CDM"), szKey, 500);

	return (nNoteCount < 0) ? 0 : nNoteCount;
}

static int CDM_SimCbxStatus(int nCbxNo)
{
	TCHAR szKey[32] = { 0, };

	if (nCbxNo < 1 || nCbxNo > CDM_SimCbxCount())
		return WFS_CDM_STATCUMISSING;

	_sntprintf_s(szKey, _countof(szKey), _TRUNCATE, _T("Cbx%dStatus"), nCbxNo);

	// Falls back to the legacy flat CbxStatus key so existing ini files
	// keep behaving the way they did.
	int nDefault = CDevSim::ReadInt(_T("CDM"), _T("CbxStatus"), WFS_CDM_STATCUOK);

	return CDevSim::ReadInt(_T("CDM"), szKey, nDefault);
}
#endif	// EMULATION_CDM_DEVICE


/** **********************************************************
*	@brief	AP에서 카세트 상태 Read
*	@param	DWORD dwCbxStatus[MAX_CASSETTE+1] : 카세트 상태 정보
*	@retval	없음
************************************************************/
void CDEV_CDM::GetCbxStatus(DWORD dwCbxStatus[MAX_CASSETTE+1])
{
#if	EMULATION_CDM_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	// Index 0 is the reject bin, 1..MAX_CASSETTE are the cassettes.
	dwCbxStatus[0] = (DWORD)CDevSim::ReadInt(_T("CDM"), _T("RejectBinStatus"), WFS_CDM_STATCUOK);

	for (int nSimCbx = 1; nSimCbx <= MAX_CASSETTE; nSimCbx++)
		dwCbxStatus[nSimCbx] = (DWORD)CDM_SimCbxStatus(nSimCbx);

	return;
#endif

	// m_dwCbxStatus[0] : Reject Bin
	// m_dwCbxStatus[1] ~ m_dwCbxStatus[6] : CBX #1 ~ CBX #6
	memcpy(dwCbxStatus, m_dwCbxStatus, sizeof(DWORD) * (MAX_CASSETTE+1));	// +1 : Reject Bin
}

/** **********************************************************
*	@brief	방출 가능 여부
*	@param	UINT nAmount : 요청 금액
*	@retval	TRUE	: 방출 가능
*	@retval	FALSE	: 방출 불가능
************************************************************/
BOOL CDEV_CDM::IsDispenable(int nAmount)
{
	BOOL bDispensable = FALSE;
	
	// Initial Item Count
	CDMCBXITEM	cdmcbxitem;
	memset(&cdmcbxitem, 0, sizeof(cdmcbxitem));

	bDispensable = ExecuteMixAlgorithm(nAmount, &cdmcbxitem);

	// Mix 실패시 CDM 장애 Clear 처리
	if (bDispensable == FALSE)
		m_strLastError.Empty();

	return bDispensable;
}


/** **********************************************************
*	@brief	MixAlgorithm 수행
*	@param	int nAmount					: 요청 금액
*	@param	LPCDMCBXITEM lpCdmCbxItem	: Mix 결과 카세트별 방출 요청 매수
*	@param	int nDispType				: 방출 순서(0-순차방출(default), 1-균등방출)
*	@retval	TRUE	: 성공
*	@retval	FALSE	: 실패
*************************************************************/
BOOL CDEV_CDM::ExecuteMixAlgorithm(int nAmount, LPCDMCBXITEM lpCdmCbxItem, int nDispType/*= 0*/)
{
	int i=0, j=0, m=0, n=0;

	// 카세트 개수 취득
	int nCbxCount = m_nCbxCount;

	UINT nCbxDenom[4];	// 카세트 권종 취득
	UINT nCbxRemain[4];	// 카세트 잔류매수 취득

	memset(nCbxDenom, 0, sizeof(nCbxDenom));
	memset(nCbxRemain, 0, sizeof(nCbxRemain));
	for (i=0; i<__min(nCbxCount, 4); i++)
	{
		nCbxDenom[i] = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i]);
		nCbxRemain[i] = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);
	}

	//////////////////////////////////////////////////////////////////////////
	memcpy(m_nCbxDenom, nCbxDenom, sizeof(UINT) * 4);
	memcpy(m_nCbxRemain, nCbxRemain, sizeof(UINT) * 4);

	int nMixValue[] = {100, 50, 20, 10, 5, 2, 1};	//Mix Denomination Table
	UINT nTotalAmount = 0;							//Possible Amount
	int cbx_cnt = 0, cbx_no = 0;
	int item_sub_cnt = 0;
	int item_req_cnt = 0;
	int nRemain = 0;
	int nCurrent = 0;
	BOOL bFind = FALSE;

	// Set Cassette Item/Denomination (Max 4 CST)
	for (i = 0; i < __min(nCbxCount, 4); i++)
	{
		if( (WFS_CDM_STATCUOK == m_dwCbxStatus[i+1]) || 
			(WFS_CDM_STATCULOW == m_dwCbxStatus[i+1]) )	// m_dwCbxStatus[0]은 Reject Bin의 상태 
		{
			nTotalAmount += m_nCbxDenom[i] * m_nCbxRemain[i];
		}
	}

	// Request Amount Check
	if (nTotalAmount < (UINT)nAmount)
	{
		m_strLastError.Format(_T("D100M0"));
		LOG(Error, _T("Insufficient amount in dipenser!!! (%s)"), m_strLastError);
		return FALSE;
	}

	// Calculate Item Count
	nRemain = nAmount;

	// Loop to Mix Table Size
	for (i = 0; i < 7; i++)
	{
		if (nRemain >= nMixValue[i])
		{
			// 동일한 권종의 카세트 수 구하기
			cbx_cnt = GetSameCBXDenomCount(nMixValue[i]);

			item_req_cnt = (int)(nRemain / nMixValue[i]);

			// V1.0.2.3 - 요청 매수가 50매를 초과할 경우 장애 처리
			if (item_req_cnt > MAX_NOTE_COUNT)
			{
				m_strLastError.Format(_T("D100M2"));
				LOG(Error, _T("Total note count is over max count(1)(%s)"), m_strLastError);
				memset(lpCdmCbxItem, 0, sizeof(CDMCBXITEM));
				return FALSE;
			}
			////////////////////////

			item_sub_cnt = 0;

			for (j = 0; j < cbx_cnt; j++)
			{
				// 카세트 index 구하기
				cbx_no = GetCBXIndex(nMixValue[i], j, nDispType);

				if (m_nCbxRemain[cbx_no] <= 0)	// 논리적인 매수가 0인 경우 skip 처리
					continue;

				if( (WFS_CDM_STATCUOK == m_dwCbxStatus[cbx_no+1]) || 
					(WFS_CDM_STATCULOW == m_dwCbxStatus[cbx_no+1]))	// m_dwCbxStatus[0]은 Reject Bin의 상태
				{
					nCurrent = m_nCbxRemain[cbx_no];

					if (nCurrent >= item_req_cnt)
					{
						lpCdmCbxItem->count[cbx_no] = item_req_cnt;
						item_sub_cnt += item_req_cnt;

						item_req_cnt = 0;

						LOG(Info, _T("Request Amount = %d, CST No = %d, Denomination = %d, Mixed Count = %d"), nAmount, (cbx_no + 1), nMixValue[i], item_sub_cnt);

						break;
					}
					else if (nCurrent < item_req_cnt)
					{
						lpCdmCbxItem->count[cbx_no] = nCurrent;
						item_sub_cnt += nCurrent;
						item_req_cnt = item_req_cnt - nCurrent;

						LOG(Info, _T("Request Amount = %d, CST No = %d, Denomination = %d, Mixed Count = %d"), nAmount, (cbx_no + 1), nMixValue[i], item_sub_cnt);
					}
				}
			}

			nRemain = nRemain - (nMixValue[i] * item_sub_cnt);

			//Mix Complete
			if (nRemain == 0)
			{
				LOG(Info, _T("Mix Completed"));

				// V1.0.2.3 - 요청 매수가 50매를 초과할 경우 장애 처리
				int nTotalCount = 0;
				for (j = 0; j < __min(nCbxCount, 4); j++)
				{
					nTotalCount += lpCdmCbxItem->count[j];
				}

				if (nTotalCount > MAX_NOTE_COUNT)
				{
					m_strLastError.Format(_T("D100M2"));
					LOG(Error, _T("Total note count is over max count(2)(%s)"), m_strLastError);
					memset(lpCdmCbxItem, 0, sizeof(CDMCBXITEM));
					return FALSE;
				}
				//////////////////////////////////

				break;
			}

			// 모든 권종($100 ~ $1)에 대해 mix를 수행했는데도, 성공하지 못했다면 최초 mix 수행한 권종 다음부터 다시 mix를 수행함.
			// 예) 1CBX:$20, 2CBX:$50, Amount:$80
			//	   => $50 1매, $20 1매 mix 후에는 $10이 남기 때문에 mix는 실패한 것으로 리텀됨.($20 4매로 방출 가능함.)
			//		  이런 경우에는 $50권종 다음인 $20부터 다시 mix를 수행해 봄.
			if( (6 == i) && (0 != nRemain) )
			{
				LOG(Info, _T("Mix Result is failed. Mix Retry"));

				bFind = FALSE;
				// Mix를 수행한 가장 작은 권종을 찾음.
				// 예) 1CBX:$20, 2CBX:$50, Amount:$180 => 최초 mix를 수행한 결과는 실패($50:3매, $20:1매)
				//     최초 mix 수행한 가장 작은 권종은 $20
				for (j = 6; j >= 0; j--)
				{
					for (m = 0; m < __min(nCbxCount, 4); m++)
					{
						if (nMixValue[j] == m_nCbxDenom[m])
						{
							bFind = TRUE;
							lpCdmCbxItem->count[m] = 0;
							break;
						}
					}

					if (TRUE == bFind)
						break;
				}
				//////////////////////////////////////////////

				bFind = FALSE;
				// mix를 수행한 가장 작은 권종보다 큰 권종의 매수를 1씩 줄여나가면서 mix를 다시 수행함.
				// 예) 1CBX:$20, 2CBX:$50, Amount:$180 => 최초 mix를 수행한 결과는 실패($50:3매, $20:1매)
				//     $50의 매수를 1 줄인 후, $100을 제외한 나머지 금액에 대해서 $20부터 다시 mix 수행함.
				//	   => 결과는 $50:2매, $20:4매로 재 산정됨.
				for (n = j - 1; n >= 0; n--)
				{
					for (m = 0; m < __min(nCbxCount, 4); m++)
					{
						if (nMixValue[n] == m_nCbxDenom[m])
						{
							if (lpCdmCbxItem->count[m] > 0)
							{
								bFind = TRUE;
								lpCdmCbxItem->count[m]--;
								LOG(Info, _T("Request Amount = %d, CST No = %d, Denomination = %d, Mixed Count = %d"), nAmount, (m + 1), nMixValue[n], lpCdmCbxItem->count[m]);
								break;
							}
						}
					}

					if (TRUE == bFind)
						break;
				}
				//////////////////////////////////////////////////////

				if (FALSE == bFind)
					break;

				i = n;

				nRemain = nAmount;

				for (j = 0; j < __min(nCbxCount, 4); j++)
				{
					nRemain = nRemain - (int)(m_nCbxDenom[j] * lpCdmCbxItem->count[j]);
				}
			}
		}
	}

	//Check mix result
	if (nRemain)
	{
		m_strLastError.Format(_T("D100M1"));
		LOG(Error, _T("Mix Result is not equal with request amount!!!"), m_strLastError);
		return FALSE;
	}

	return TRUE;
}


/** **********************************************************
*	@brief	Dispense 로그 파일 생성
*	@param	BYTE* pbyRecvLogData	: 로그 data
*	@param	DWORD dwRecvSize			: 로그 size
*	@retval	없음.
*************************************************************/
void CDEV_CDM::WriteLogDataToLOG(BYTE* pbyRecvData, DWORD dwRecvSize)
{
	DWORD dwWriten = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	CString strFilePath;
	BOOL bFileFind = FALSE;
	CString strLogData = _T("");
	CString strLogValue = _T("");
	DWORD dwHeaderCount = dwRecvSize/3;	// F/W로부터 Log Data가 3의 배수로 온다.
	DWORD dwLogSize = 0;
	int nLogDataValue = 0;
	int nLogHeaderLen = 5;
	BYTE* pbyLogData = NULL;

	SYSTEMTIME		st;
	GetLocalTime(&st);

	strFilePath.Format(_T("%s\\%s"), PATH_LOG_ABS, CDM_LOG_FILE_NAME_LOG);

	LOG(Info, _T("CDEV_CDM::WriteLogDataToLOG() Log file path :[%s]"), strFilePath);

	bFileFind = CUtil::IsExistFile(strFilePath);
	if(TRUE == bFileFind)
	{
		CUtil::DeleteFileInDirectory(PATH_LOG_ABS, CDM_LOG_FILE_NAME_LOG);
	}

	// open file
	hFile = CreateFile (strFilePath,
		GENERIC_WRITE,          // Open for writing
		0,                      // Do not share
		NULL,                   // No security
		CREATE_ALWAYS,          // Open or create
		FILE_ATTRIBUTE_NORMAL,  // Normal file
		NULL);                  // No template file

	if (INVALID_HANDLE_VALUE == hFile)
	{
		LOG(Error, _T("CDEV_CDM::WriteLogDataToLOG() CreateFile is failed."));
		return;
	}

	// LogData의 Packet은 Header(1byte) + Data(2byte)로 구성되어 있음.
	// Header에 해당하는 value는 5자리의 문자열임.(F/W사양서 참조)
	// 로그 파일에 기록할 때, Data(2byte)도 10진수로 변환해야 하기 때문에 2byte로 표현가능한 10진수 자리수는 5자리임.(65535)
	// 때문에 (header count * 10)만큼 메모리 할당을 해줌.
	// *11 한 이유 : 각 patket 뒤에 " "를 추가하기 때문에
	// +23 : 날짜정보
	pbyLogData = new BYTE[dwHeaderCount*11 + 23 + 1];
	memset(pbyLogData, 0x00, dwHeaderCount*11 + 23 + 1);

	strLogData.Format(_T("[%04d/%02d/%02d %02d:%02d:%02d]  "), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	for(int i=0; i<(int)dwHeaderCount; i++)
	{
	//	memcpy(pbyLogData + i*nLogHeaderLen + dwLogSize, DISP_LOG_HEADER[pbyRecvData[3*i]], nLogHeaderLen);
		if(pbyRecvData[3*i] < DISP_LOG_HEADER_COUNT)
			strLogData += DISP_LOG_HEADER[pbyRecvData[3*i]];
	
		nLogDataValue = (pbyRecvData[3*i + 1] << 8) | (pbyRecvData[3*i + 2]);
		strLogValue = CUtil::IntToString(nLogDataValue);
		strLogData += strLogValue;
		strLogData += _T(" ");
	}

	WideCharToMultiByte(CP_ACP, 0, strLogData, -1, (char*)pbyLogData, strLogData.GetLength(), NULL, NULL);

	if (!WriteFile(hFile, pbyLogData, strLogData.GetLength(), &dwWriten, NULL))
	{
		LOG(Error, _T("CDEV_CDM::WriteLogDataToLOG() File Write Failed."));
	}

	if ( (hFile != INVALID_HANDLE_VALUE) && (hFile != NULL)	)
		CloseHandle(hFile);

	if(pbyLogData != NULL)
	{
		delete[] pbyLogData;
		pbyLogData = NULL;
	}
}

//WORD wSensorData[55][1000] = { 0, };
WORD wSensorData[DISP_LOG_HEADER_COUNT][1000] = { 0, };

/** **********************************************************
*	@brief	Dispense 로그 파일 생성
*	@param	BYTE* pbyRecvLogData	: 로그 data
*	@param	DWORD dwRecvSize			: 로그 size
*	@retval	없음.
*************************************************************/
void CDEV_CDM::WriteLogDataToCSV(BYTE* pbyRecvData, DWORD dwRecvSize)
{
	//2. Data Analyze
	int i = 0, j = 0;
	int nTempEnd = 0;
	BOOL bCST1End = FALSE;
	int nDataCount[1000] = { 0, };	// Max 1000
	int nTempX = 0, nTempY = 0;
	DWORD dwLogLength = 0;
	BYTE* byData = pbyRecvData;
	CString strFilename;

	memset(wSensorData, 0, sizeof(wSensorData));

	dwLogLength = dwRecvSize;

	for (i = 0; i < (int)dwLogLength; i += 3) // 1Packet size = 3byte
	{
		nTempX = byData[i];

		if ((nTempX == IDX_CL2o || nTempX == IDX_CR2o) && (bCST1End == FALSE))
		{
			bCST1End = TRUE;

			// 2카세트 방출 시작(CST1 pick up 매수 이후로 설정)
			//nTempEnd = nDataCount[IDX_MI1o];
			nTempEnd = nDataCount[IDX_CL1o];

			nDataCount[IDX_CL2o] = nTempEnd;
			nDataCount[IDX_CL3o] = nTempEnd;
			nDataCount[IDX_CL4o] = nTempEnd;

			nDataCount[IDX_CL2x] = nTempEnd;
			nDataCount[IDX_CL3x] = nTempEnd;
			nDataCount[IDX_CL4x] = nTempEnd;

			nDataCount[IDX_CR2o] = nTempEnd;
			nDataCount[IDX_CR3o] = nTempEnd;
			nDataCount[IDX_CR4o] = nTempEnd;

			nDataCount[IDX_CR2x] = nTempEnd;
			nDataCount[IDX_CR3x] = nTempEnd;
			nDataCount[IDX_CR4x] = nTempEnd;

			nDataCount[IDX_MI2o] = nTempEnd;
			nDataCount[IDX_MI3o] = nTempEnd;
			nDataCount[IDX_MI4o] = nTempEnd;

			nDataCount[IDX_MI2x] = nTempEnd;
			nDataCount[IDX_MI3x] = nTempEnd;
			nDataCount[IDX_MI4x] = nTempEnd;
		}

		// Data Insert
	//	for (j = 0; j < 55; j++)
		for (j = 0; j < DISP_LOG_HEADER_COUNT; j++)
		{
			if (nTempX == j)
			{
				nTempY = nDataCount[nTempX];

				wSensorData[nTempX][nTempY] = ((BYTE)byData[i + 1] * 0x100) + (BYTE)byData[i + 2];

				nDataCount[nTempX]++;

				break;
			}
		}
	}

	// 총매수 구하기
	//int nLoopCount = nDataCount[IDX_MI1o];
	// Check1 sns를 지난 매수
	int nLoopCount = nDataCount[IDX_CL1o];
	// Check2 sns를 지난 매수
	nLoopCount += (nDataCount[IDX_CL2o] > nDataCount[IDX_CL1o]) ? nDataCount[IDX_CL2o] - nDataCount[IDX_CL1o] : 0;

	//3. Save CSV file(Length만큼 Log Data를 받아 Txt or Excel 변환)
	CString strWrite;

	SYSTEMTIME tm;
	GetLocalTime(&tm);

	FILE *fplog;

	strFilename.Format(_T("%s\\%s"), PATH_LOG_ABS, CDM_LOG_FILE_NAME_CSV);

	LOG(Info, _T("CDEV_CDM::WriteLogDataToCSV() Log file path :[%s]"), strFilename);

	BOOL bFileFind = CUtil::IsExistFile(strFilename);
	if(TRUE == bFileFind)
	{
		CUtil::DeleteFileInDirectory(PATH_LOG_ABS, CDM_LOG_FILE_NAME_CSV);
	}

	fplog = _wfopen(strFilename, _T("w+"));

	// Title set
	for (i = 0; i < DISP_LOG_HEADER_COUNT; i++)
	{
		strWrite += (strLogName[i] + _T(", "));
	}
	strWrite += _T("\n");

	fwprintf(fplog, strWrite);

	// Data set
	for (i = 0; i < nLoopCount; i++)
	{
		for (j = 0; j < DISP_LOG_HEADER_COUNT; j++)
		{
			fwprintf(fplog, _T("%d, "), wSensorData[j][i]);
		}

		fwprintf(fplog, _T("\n"));
	}

	fclose(fplog);
}


/** *********************************************************
*	@brief		Get_CBXStatus : 모든 CST 상태 조회
*	@retval		CBX 상태값 return
************************************************************/
int CDEV_CDM::Get_CBXAllStatus()
{

#if (EMULATION_CDM_DEVICE)
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	// CbxAllStatus forces a value when set; -1 (the default) derives it from
	// the per-cassette keys using the same precedence as the real device.
	int nForced = CDevSim::ReadInt(_T("CDM"), _T("CbxAllStatus"), -1);

	if (nForced >= 0)
		return nForced;

	int nSimCount = CDM_SimCbxCount();
	int nSimRet   = WFS_CDM_STATCUMISSING;

	for (int nSimCbx = 1; nSimCbx <= nSimCount; nSimCbx++)
	{
		int nSimStatus = CDM_SimCbxStatus(nSimCbx);
		int nSimNotes  = CDM_SimCbxNoteCount(nSimCbx);

		if (nSimStatus == WFS_CDM_STATCUOK)
		{
			if (nSimNotes > 0)
				return WFS_CDM_STATCUOK;

			nSimRet = WFS_CDM_STATCUEMPTY;
		}
		else if (nSimStatus == WFS_CDM_STATCULOW)
		{
			if (nSimNotes > 0)
				return WFS_CDM_STATCULOW;

			nSimRet = WFS_CDM_STATCUEMPTY;
		}
		else if (nSimStatus == WFS_CDM_STATCUEMPTY)
		{
			nSimRet = WFS_CDM_STATCUEMPTY;
		}
	}

	return nSimRet;
#endif

	int		nCBXCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);
	int		nCSTItemCount = 0;					// 논리적인 매수도 참고하도록 로직 추가  2018.09.12

	DWORD	dwCBXStatus[MAX_CASSETTE + 1];		// Reject Bin / CST1 ~ 4
	memset(dwCBXStatus, 0, sizeof(dwCBXStatus));

	GetCbxStatus(dwCBXStatus);

	// Reject Bin이 Full일 경우 우선 처리
	//	if (dwCBXStatus[0] == WFS_CDM_STATCUFULL)
	//		return WFS_CDM_STATCUFULL;

	int nRet = WFS_CDM_STATCUMISSING;

	for(int i=0; i<nCBXCount; i++)
	{
		// CST별 Item Count는 0 base
		nCSTItemCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);	// 논리적인 매수도 참고하도록 로직 추가  2018.09.12

		if (dwCBXStatus[i+1] == WFS_CDM_STATCUOK)
		{
			if (nCSTItemCount > 0)		// 논리적인 매수도 참고하도록 로직 추가  2018.09.12
			{
				nRet = WFS_CDM_STATCUOK;
				return nRet;
			}
			else
			{
				nRet = WFS_CDM_STATCUEMPTY;
			}
		}
		else if (dwCBXStatus[i+1] == WFS_CDM_STATCULOW)
		{
			if (nCSTItemCount > 0)		// 논리적인 매수도 참고하도록 로직 추가 2018.09.12
			{
				nRet = WFS_CDM_STATCULOW;
				return nRet;
			}
			else
			{
				nRet = WFS_CDM_STATCUEMPTY;
			}
		}
		else if (dwCBXStatus[i+1] == WFS_CDM_STATCUEMPTY)	// missfeed인 경우에만 CST 상태가 EMPTY로 처리됨
		{
			nRet = WFS_CDM_STATCUEMPTY;
		}
	}

	return nRet;
}


/** *********************************************************
*	@brief		Get_CBXAllNoteCount : 모든 CST note count값 조회
*	@retval		CBX 상태값 return
************************************************************/
int CDEV_CDM::Get_CBXAllNoteCount()
{

#if (EMULATION_CDM_DEVICE)
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	// CbxAllNoteCount forces a total when set; -1 (the default) sums the
	// per-cassette Cbx<n>NoteCount keys.
	int nForced = CDevSim::ReadInt(_T("CDM"), _T("CbxAllNoteCount"), -1);

	if (nForced >= 0)
		return nForced;

	int nSimCount = CDM_SimCbxCount();
	int nSimTotal = 0;

	for (int nSimCbx = 1; nSimCbx <= nSimCount; nSimCbx++)
		nSimTotal += CDM_SimCbxNoteCount(nSimCbx);

	return nSimTotal;
#endif

	int		nCBXCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);
	int		nNoteCount = 0;

	for(int i=0; i<nCBXCount; i++)
		nNoteCount += CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);

	return nNoteCount;
}


/** *********************************************************
*	@brief		CDM을 초기화 하지 않고 CBX 상태만 강제로 Clear하는 함수
*	@retval		CBX 상태값 return
************************************************************/
void CDEV_CDM::Clear_CBXStatus()
{
	// 축퇴 상태인 카세트의 상태를 강제 clear
	for(int i=0; i<MAX_CASSETTE; i++)
		m_bCbxStatusEmpty[i] = FALSE;
}


// CDM Quiry Status Start
void CDEV_CDM::CDM_QuiryStatusStart()
{
#if (EMULATION_CDM_DEVICE)
	return;
#endif
	m_bCDMExecuteQuiryStatus = TRUE;

	// Thread 기동 후 1Cycle이 완료될때까지 대기
	//while(m_bDoingThread == FALSE)	// TRUE가 될 때까지 대기
	//{
	//	CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	//}

	//// TRUE가 된 상태에서는 FALSE가 될 때까지 대기 (최대 6초)
	//int nWaitCount = 0;
	//while(m_bDoingThread == TRUE)
	//{
	//	if (m_bDoingThread == FALSE)
	//		break;

	//	if (nWaitCount > 60)	// 10초로 판단해보기
	//	{
	//		// Polling Thread가 Dead Lock이 걸린 것으로 간주
	//		LOG(Error, _T("CDM_QuiryStatusStart - Failed Stop Polling"));
	//		break;
	//	}

	//	nWaitCount++;
	//	CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	//}

}

// CDM Quiry Status Stop
void CDEV_CDM::CDM_QuiryStatusStop()
{
#if (EMULATION_CDM_DEVICE)
	return;
#endif
	m_bCDMExecuteQuiryStatus = FALSE;

	// Polling Stop시에는 현재 진행되는 Polling까지 완료된 후 return하도록 수정 (최대 6초 대기)
	int nWaitCount = 0;

	LOG(Info, _T("CDM_QuiryStatusStop - start"));

	while(m_bDoingThread == TRUE)
	{
		if (m_bDoingThread == FALSE)
			break;

		if (nWaitCount > 60)
		{
			// Polling Thread가 Dead Lock이 걸린 것으로 간주
			LOG(Error, _T("CDM_QuiryStatusStop - Failed Stop Polling"));
			break;
		}

		nWaitCount++;
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	LOG(Info, _T("CDM_QuiryStatusStop - end"));
}


/** **********************************************************
*	@brief	수표 교환시에 사용되는 Mix Algorithm 수행
*	@param	int nAmount					: 요청 금액
*	@param	LPCDMMIXRESULT lpCdmMixResult	: Mix 결과 카세트별 방출 요청 매수
*	@param	nRemainAmount				: Mix 결과 남은 요청 금액(Coin으로 Dispense하기 위함.)
*	@param	int nDispType				: 방출 순서(0-순차방출(default), 1-균등방출)
*	@retval	TRUE	: 성공
*	@retval	FALSE	: 실패
*************************************************************/
BOOL CDEV_CDM::ExecuteMixAlgorithm4MaxAvailAmount(UINT nAmount, LPCDMMIXRESULT lpCdmMixResult, UINT &nRemainAmount, BOOL bApplyLowStatus, int nDispType/* = 0*/)
{
	BOOL bResult = FALSE;
	int i = 0, j = 0, n = 0, m = 0;

	// 카세트 권종, 카세트 잔류매수 취득
	UINT nCbxDenom[4] = { 0, };
	UINT nCbxRemain[4] = { 0, };

	for (i = 0; i<__min(m_nCbxCount, 4); i++)
	{
		nCbxDenom[i] = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i]);
		nCbxRemain[i] = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);
	}

	memcpy(m_nCbxDenom, nCbxDenom, sizeof(UINT) * 4);
	memcpy(m_nCbxRemain, nCbxRemain, sizeof(UINT) * 4);
	//////////////////////////////////////////////////////////////////////////

	int nCbxCount = m_nCbxCount;
	int nMixValue[MAX_CFG_CBX_COUNT] = { 0, };
	int cbx_cnt = 0, cbx_no = 0;
	int item_sub_cnt = 0;
	int item_req_cnt = 0;
	int nRemain = 0;
	int nCurrent = 0;
	BOOL bFindMinimumDenom = FALSE;
	BOOL bFindNextDenom = FALSE;
	CDMMIXRESULT tempCdmMixResult;
	CDMMIXRESULT sortedCdmMixResult;
	UINT nTempRemainAmount = 0;
	int nTotalCount = 0;
	int nRemainCount = 0;
	BOOL bFinished = FALSE;
	BOOL bFirstTempCopy = TRUE;
	int nMixStartIndex = 0;
	int nMixValueCount = 0;

	int nOrgCbxCount = 0;
	int nOrgCbxDenom[MAX_CFG_CBX_COUNT] = { 0, };
	int nOrgCbxRemain[MAX_CFG_CBX_COUNT] = { 0, };
	DWORD dwOrgCbxStatus[MAX_CASSETTE + 1] = { 0, };
	int nOrgCbxIndex[4] = { 0, };

	memset(&tempCdmMixResult, 0, sizeof(CDMMIXRESULT));
	memset(&sortedCdmMixResult, 0, sizeof(CDMMIXRESULT));

#if	EMULATION_CDM_DEVICE
	for (i = 0; i<MAX_CFG_CBX_COUNT + 1; i++)
	{
		m_dwCbxStatus[i] = WFS_CDM_STATCUOK;
	}
#endif

	if (0 == nAmount)
	{
		LOG(Info, _T("Amount is zero"));
		return TRUE;
	}

	nOrgCbxCount = nCbxCount;
	memcpy(nOrgCbxDenom, m_nCbxDenom, sizeof(m_nCbxDenom));
	memcpy(nOrgCbxRemain, m_nCbxRemain, sizeof(m_nCbxRemain));
	memcpy(dwOrgCbxStatus, m_dwCbxStatus, sizeof(m_dwCbxStatus));

	nCbxCount = 0;
	memset(m_nCbxDenom, 0, sizeof(m_nCbxDenom));
	memset(m_nCbxRemain, 0, sizeof(m_nCbxRemain));
	memset(m_dwCbxStatus, 0, sizeof(m_dwCbxStatus));

	int nTempValue = 0;

	// Mix에 유효하지 않은 권종 제거
	for (i = 0; i < __min(nOrgCbxCount, 4); i++)
	{
		if (nOrgCbxRemain[i] <= 0)	// 논리적인 매수가 0인 경우 skip 처리
			continue;

		if (bApplyLowStatus == FALSE)
		{
			// OK가 아닌 경우 NG로 처리
			if (WFS_CDM_STATCUOK != dwOrgCbxStatus[i + 1])	// m_dwCbxStatus[0]은 Reject Bin의 상태
				continue;
		}
		else
		{
			// OK and LOW가 아닌 경우 NG로 처리
			if ((WFS_CDM_STATCUEMPTY == dwOrgCbxStatus[i + 1]) || (WFS_CDM_STATCUMISSING == dwOrgCbxStatus[i + 1]))	// m_dwCbxStatus[0]은 Reject Bin의 상태
				continue;
		}


		nOrgCbxIndex[nCbxCount] = i;

		m_nCbxDenom[nCbxCount] = nOrgCbxDenom[i];
		m_nCbxRemain[nCbxCount] = nOrgCbxRemain[i];
		m_dwCbxStatus[1 + (nCbxCount++)] = dwOrgCbxStatus[i + 1];
	}
	/////////////////////////////////////////

	// 내림차순으로 정렬하기 위함.
	for (i = 0; i < __min(nCbxCount, 4) - 1; i++)
	{
		for (j = i + 1; j < __min(nCbxCount, 4); j++)
		{
			if (m_nCbxDenom[i] < m_nCbxDenom[j])
			{
				nTempValue = nOrgCbxIndex[i];
				nOrgCbxIndex[i] = nOrgCbxIndex[j];
				nOrgCbxIndex[j] = nTempValue;

				nTempValue = m_nCbxDenom[i];
				m_nCbxDenom[i] = m_nCbxDenom[j];
				m_nCbxDenom[j] = nTempValue;

				nTempValue = m_nCbxRemain[i];
				m_nCbxRemain[i] = m_nCbxRemain[j];
				m_nCbxRemain[j] = nTempValue;

				nTempValue = m_dwCbxStatus[i + 1];
				m_dwCbxStatus[i + 1] = m_dwCbxStatus[j + 1];
				m_dwCbxStatus[j + 1] = nTempValue;
			}
		}
	}
	///////////////////////////////////////////

	// Mix에 필요한 권종 종류를 산출하는 부분(중복 권종 제거)
	int nPreCbxDenom = 0;

	for (i = 0; i < __min(nCbxCount, 4); i++)
	{
		if (nPreCbxDenom != m_nCbxDenom[i])
		{
			nMixValue[nMixValueCount++] = m_nCbxDenom[i];
			nPreCbxDenom = m_nCbxDenom[i];
		}
	}
	///////////////////////////////////////////

	// Calculate Item Count
	nRemain = nAmount;
	nRemainAmount = 0;

	while (!bFinished)
	{
		// Loop to Mix Table Size
		for (i = nMixStartIndex; i < nMixValueCount; i++)
		{
			if (nRemain >= nMixValue[i])
			{
				// 동일한 권종의 카세트 수 구하기
				cbx_cnt = GetSameCBXDenomCount(nMixValue[i]);

				item_req_cnt = (int)(nRemain / nMixValue[i]);

				item_sub_cnt = 0;

				for (j = 0; j < cbx_cnt; j++)
				{
					// 카세트 index 구하기
					cbx_no = GetCBXIndex(nMixValue[i], j, nDispType);

					if (m_nCbxRemain[cbx_no] <= 0)	// 논리적인 매수가 0인 경우 skip 처리
						continue;

					BOOL bTemp_CheckCBXStatus = FALSE;

					if (bApplyLowStatus == FALSE)
					{
						// OK인 경우만 CBX가 정상이라고 판단
						if (WFS_CDM_STATCUOK == m_dwCbxStatus[cbx_no + 1])
							bTemp_CheckCBXStatus = TRUE;
					}
					else
					{
						if ((WFS_CDM_STATCUOK == m_dwCbxStatus[cbx_no + 1]) || (WFS_CDM_STATCULOW == m_dwCbxStatus[cbx_no + 1]))
							bTemp_CheckCBXStatus = TRUE;
					}

					//if (WFS_CDM_STATCUOK == m_dwCbxStatus[cbx_no + 1])	// m_dwCbxStatus[0]은 Reject Bin의 상태
					if (bTemp_CheckCBXStatus == TRUE)
					{
						nCurrent = m_nCbxRemain[cbx_no];

						if (nCurrent >= item_req_cnt)
						{
							if ((nTotalCount + item_req_cnt) > MAX_NOTE_COUNT)
							{
								sortedCdmMixResult.nCount[cbx_no] = MAX_NOTE_COUNT - nTotalCount;
								item_sub_cnt += (MAX_NOTE_COUNT - nTotalCount);
								nTotalCount += (MAX_NOTE_COUNT - nTotalCount);

								bFinished = TRUE;
							}
							else
							{
								sortedCdmMixResult.nCount[cbx_no] = item_req_cnt;
								item_sub_cnt += item_req_cnt;
								nTotalCount += item_req_cnt;
							}

							item_req_cnt = 0;

							LOG(Info, _T("Request Amount = %d, CST No = %d, Denomination = %d, Mixed Count = %d"), nAmount, (cbx_no + 1), nMixValue[i], item_sub_cnt);

							break;
						}
						else// if (nCurrent < item_req_cnt)
						{
							if ((nTotalCount + nCurrent) > MAX_NOTE_COUNT)
							{
								sortedCdmMixResult.nCount[cbx_no] = MAX_NOTE_COUNT - nTotalCount;
								item_sub_cnt += (MAX_NOTE_COUNT - nTotalCount);
								nTotalCount += (MAX_NOTE_COUNT - nTotalCount);

								LOG(Info, _T("Request Amount = %d, CST No = %d, Denomination = %d, Mixed Count = %d"), nAmount, (cbx_no + 1), nMixValue[i], item_sub_cnt);

								bFinished = TRUE;
								break;
							}
							else
							{
								sortedCdmMixResult.nCount[cbx_no] = nCurrent;
								item_sub_cnt += nCurrent;
								item_req_cnt = item_req_cnt - nCurrent;
								nTotalCount += nCurrent;

								LOG(Info, _T("Request Amount = %d, CST No = %d, Denomination = %d, Mixed Count = %d"), nAmount, (cbx_no + 1), nMixValue[i], item_sub_cnt);
							}
						}
					}
				}

				nRemain = nRemain - (nMixValue[i] * item_sub_cnt);

				if (TRUE == bFinished)
					break;

				//Mix Complete
				if (nRemain == 0)
				{
					nRemainAmount = 0;
					LOG(Info, _T("Mix Completed"));

					bFinished = TRUE;
					break;
				}
			}	// if end
		}	// for end

		// 모든 권종($100 ~ $1)에 대해 mix를 수행했는데도, 성공하지 못했다면 최초 mix 수행한 권종 다음부터 다시 mix를 수행함.
		// 예) 1CBX:$20, 2CBX:$50, Amount:$80
		//	   => $50 1매, $20 1매 mix 후에는 $10이 남기 때문에 mix는 실패한 것으로 리텀됨.($20 4매로 방출 가능함.)
		//		  이런 경우에는 $50권종 다음인 $20부터 다시 mix를 수행해 봄.
		//		if (((nMixValueCount - 1) == i) && (0 != nRemain))
		if (0 != nRemain)
		{
			LOG(Info, _T("Mix Result is failed. Mix Retry"));

			nRemainAmount = nRemain;

			// 최대한 지폐로 다 내주고 난 후, Coin Mix를 수행하기 위함.(ex: $5, $10 설정 상태에서 $37 출금요청)
			if (TRUE == bFirstTempCopy)
			{
				nTempRemainAmount = nRemainAmount;
				memcpy(tempCdmMixResult.nCount, sortedCdmMixResult.nCount, sizeof(int) * MAX_CASSETTE);
				bFirstTempCopy = FALSE;
			}
			else
			{
				if (nRemainAmount < nTempRemainAmount)
				{
					nTempRemainAmount = nRemainAmount;
					memcpy(tempCdmMixResult.nCount, sortedCdmMixResult.nCount, sizeof(int) * MAX_CASSETTE);
				}
			}

			if (TRUE == bFinished)
				break;

			bFindMinimumDenom = FALSE;
			int nIndexFindValue = 0;

			// Mix를 수행한 가장 작은 권종을 찾음.
			// 예) 1CBX:$20, 2CBX:$50, Amount:$180 => 최초 mix를 수행한 결과는 실패($50:3매, $20:1매)
			//     최초 mix 수행한 가장 작은 권종은 $20
			for (j = (nMixValueCount - 1); j >= 0; j--)
			{
				for (m = 0; m < __min(nCbxCount, 4); m++)
				{
					if (m_nCbxRemain[m] <= 0)	// 논리적인 매수가 0인 경우 skip 처리
						continue;

					if (bApplyLowStatus == FALSE)
					{
						if (WFS_CDM_STATCUOK != m_dwCbxStatus[m + 1])	// m_dwCbxStatus[0]은 Reject Bin의 상태
							continue;
					}
					else
					{
						// CBX가 OK and LOW 경우 정상이라고 판단
						if ((WFS_CDM_STATCUEMPTY == m_dwCbxStatus[m + 1]) || (WFS_CDM_STATCUMISSING == m_dwCbxStatus[m + 1]))
							continue;
					}

					if (nMixValue[j] == m_nCbxDenom[m])
					{
						bFindMinimumDenom = TRUE;
						nIndexFindValue = m;
						//	sortedCdmMixResult.nCount[m] = 0;	// 위치 이동
						break;
					}
				}

				if (TRUE == bFindMinimumDenom)
					break;
			}
			//////////////////////////////////////////////

			bFindNextDenom = FALSE;
			// mix를 수행한 가장 작은 권종보다 큰 권종의 매수를 1씩 줄여나가면서 mix를 다시 수행함.
			// 예) 1CBX:$20, 2CBX:$50, Amount:$180 => 최초 mix를 수행한 결과는 실패($50:3매, $20:1매)
			//     $50의 매수를 1 줄인 후, $100을 제외한 나머지 금액에 대해서 $20부터 다시 mix 수행함.
			//	   => 결과는 $50:2매, $20:4매로 재 산정됨.
			for (n = j - 1; n >= 0; n--)
			{
				for (m = 0; m < __min(nCbxCount, 4); m++)
				{
					if (nMixValue[n] == m_nCbxDenom[m])
					{
						if (sortedCdmMixResult.nCount[m] > 0)
						{
							bFindNextDenom = TRUE;
							sortedCdmMixResult.nCount[m]--;
							nTotalCount--;
							LOG(Info, _T("Request Amount = %d, CST No = %d, Denomination = %d, Mixed Count = %d"), nAmount, (m + 1), nMixValue[n], sortedCdmMixResult.nCount[m]);
							break;
						}
					}
				}

				if (TRUE == bFindNextDenom)
					break;
			}
			//////////////////////////////////////////////////////

			
			if (FALSE == bFindNextDenom)
			{
				break;
			}
			else
			{
				nTotalCount -= sortedCdmMixResult.nCount[nIndexFindValue];
				sortedCdmMixResult.nCount[nIndexFindValue] = 0;	// mix를 수행한 가장 작은 권종보다 큰 권종의 매수가 있을 경우에만 기존 mix 결과를 초기화 함.
			}

			nMixStartIndex = n + 1;

			nRemain = nAmount;

			for (j = 0; j < __min(nCbxCount, 4); j++)
			{
				nRemain = nRemain - (int)(m_nCbxDenom[j] * sortedCdmMixResult.nCount[j]);
			}
		}	// if end
	}	// while end

	if (nRemain)
	{
		memcpy(sortedCdmMixResult.nCount, tempCdmMixResult.nCount, sizeof(int) * MAX_CASSETTE);
		nRemainAmount = nTempRemainAmount;
	}

	memcpy(m_nCbxDenom, nOrgCbxDenom, sizeof(nOrgCbxDenom));
	memcpy(m_nCbxRemain, nOrgCbxRemain, sizeof(nOrgCbxRemain));
	memcpy(m_dwCbxStatus, dwOrgCbxStatus, sizeof(dwOrgCbxStatus));

	for (i = 0; i < __min(nCbxCount, 4); i++)
	{
		lpCdmMixResult->nCount[nOrgCbxIndex[i]] = sortedCdmMixResult.nCount[i];
	}

	nCbxCount = nOrgCbxCount;

	return TRUE;
}
