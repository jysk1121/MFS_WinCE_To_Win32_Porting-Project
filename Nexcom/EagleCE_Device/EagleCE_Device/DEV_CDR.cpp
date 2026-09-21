#include "stdafx.h"
#include "DEV_Define.h"
#include "DeviceSimUtil.h"
#include "DevSimCDRDlg.h"
#include "DevSimTrack2Dlg.h"
#include "DEV_CDR.h"
#include <wincrypt.h>	// [PCI-SSF Fix] CryptGenRandom(CSPRNG) ???
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleDataManager.h"
#include "../../EagleCE_Framework/EagleCE_Framework/IniFile.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CDEV_CDR	*m_CDR = NULL;
int CALLBACK ICCCommand(BYTE* SendData, DWORD cmdLength, BYTE* RecvData, DWORD* RecvLen);

BYTE CDEV_CDR::m_byTempData[1024] = {0,};

#if !defined(_WIN32_WCE) && (SUPPORT_EMV)

/** **********************************************************
*	@brief		EMV kernel simulator (Win32 only)
*
*	EMVKernel_API.dll is built for WinCE/ARM and cannot be loaded on the
*	desktop. These stubs are bound to the very same function pointers the
*	WinCE build fills in from GetProcAddress(), so every existing NULL check
*	and every call site behaves as if the real kernel were present and each
*	step of the EMV flow succeeded.
*
*	TerminalActionAnalysis reports ARQC and CardActionAnalysis reports
*	ONLINE_TRANS so the transaction is taken online, which is the path the
*	simulator needs to exercise.
************************************************************/
static int  EMVSim_AppSelection(unsigned char, EMV_LIST_OF_AID*)					{ return EMV_RSLT_OK; }
static int  EMVSim_MakeAppLists(unsigned char, EMV_ICC_APPS_INFORM*, int *nAppCnt)	{ if (NULL != nAppCnt) *nAppCnt = 1; return EMV_RSLT_OK; }
static void EMVSim_RemoveAppList(USHORT, unsigned char*, int *nAppCnt)				{ if (NULL != nAppCnt) *nAppCnt = 0; }
static int  EMVSim_FinalAppSelection(USHORT, unsigned char*, int*, unsigned char *SW)	{ if (NULL != SW) { SW[0] = 0x90; SW[1] = 0x00; } return EMV_RSLT_OK; }
static int  EMVSim_StoreValFromRecord(unsigned char*, int, int, int)				{ return EMV_RSLT_OK; }
static int  EMVSim_InitApplication(int*)											{ return EMV_RSLT_OK; }
static int  EMVSim_ReadAppData(void)											{ return EMV_RSLT_OK; }
static int  EMVSim_OfflineDataAuth(void)										{ return EMV_RSLT_OK; }
static int  EMVSim_ProcessRestrict(void)										{ return EMV_RSLT_OK; }
static int  EMVSim_CardholderVerify(int, unsigned char*)						{ return EMV_RSLT_OK; }
static int  EMVSim_TerminalRiskMgmt(unsigned char)								{ return EMV_RSLT_OK; }
static byte EMVSim_TerminalActionAnalysis(void)									{ return ARQC; }
static int  EMVSim_CardActionAnalysis(unsigned char *OnlineFlag)				{ if (NULL != OnlineFlag) *OnlineFlag = (unsigned char)ONLINE_TRANS; return EMV_RSLT_OK; }
static int  EMVSim_OnlineProcess(unsigned char)									{ return EMV_RSLT_OK; }
static int  EMVSim_Completion(void)												{ return EMV_RSLT_OK; }
static int  EMVSim_CDRCommFunc(int (CALLBACK *)(BYTE*, DWORD, BYTE*, DWORD*))	{ return EMV_RSLT_OK; }

// Encodes DeviceSim.ini's [CDR] TrackISO2 as EMV tag 57 (Track 2 Equivalent
// Data): '=' becomes the 'D' separator nibble, an odd digit count is padded
// with a trailing 'F' nibble, then nibble pairs are packed into bytes - the
// reverse of the F/D handling in CSVC_Manager::Proc_Client_EMV_ReadAppData().
//
// Taken from the ini rather than prompting, so a successful chip read runs
// start to finish without interrupting the tester a second time. The Track2
// panel is reserved for the magstripe/fallback read, where a real reader
// would genuinely be handing back a freshly swiped track.
static int EncodeTrack2AsTag57(unsigned char* val)
{
	CString strTrack2 = CDevSim::ReadString(_T("CDR"), _T("TrackISO2"),
		_T("111111111111111111=222222222222"));

	strTrack2.MakeUpper();
	strTrack2.Replace(_T('='), _T('D'));

	if ((strTrack2.GetLength() % 2) != 0)
		strTrack2 += _T('F');

	int nBytes = strTrack2.GetLength() / 2;

	for (int i = 0; i < nBytes; i++)
	{
		TCHAR chHi = strTrack2[i * 2];
		TCHAR chLo = strTrack2[(i * 2) + 1];

		BYTE byHi = (chHi >= _T('A')) ? (BYTE)(chHi - _T('A') + 10) : (BYTE)(chHi - _T('0'));
		BYTE byLo = (chLo >= _T('A')) ? (BYTE)(chLo - _T('A') + 10) : (BYTE)(chLo - _T('0'));

		val[i] = (BYTE)((byHi << 4) | byLo);
	}

	return nBytes;
}

// Every other tag reports "present but empty" so callers copy nothing
// instead of reading an uninitialised buffer. Tag 57 is the one tag the ATM
// must have to complete a successful EMV read - without it
// Proc_Client_EMV_ReadAppData() logs "IC Track2 length error" and cancels
// the transaction.
static int  EMVSim_Tlv_GetVal(unsigned char* tag, int *len, unsigned char* val)
{
	if ((tag != NULL) && (val != NULL) && (strcmp((const char*)tag, "5700") == 0))
	{
		int nBytes = EncodeTrack2AsTag57(val);

		if (NULL != len)
			*len = nBytes;

		return EMV_RSLT_OK;
	}

	if (NULL != len)
		*len = 0;

	return EMV_RSLT_OK;
}
static int  EMVSim_Tlv_StoreVal(unsigned char*, int, unsigned char*)			{ return EMV_RSLT_OK; }
static void EMVSim_Initial_Variable(void)										{ }
static void EMVSim_SetDebug(BYTE)												{ }
static void EMVSim_SetKernelLogPath(unsigned char*, unsigned char*)				{ }

#endif	// !_WIN32_WCE && SUPPORT_EMV

/** **********************************************************
*	@brief		Card Command
*	@retval		없음
************************************************************/
int CALLBACK ICCCommand(BYTE* SendData, DWORD cmdLength, BYTE* RecvData, DWORD* RecvLen)
{
#if	EMULATION_CDR_DEVICE
	return TRUE;
#endif

	if(TRUE == m_CDR->m_bKeyAuthError)
	{
		LOG(Error, _T("ICCCommand() System Error."));
		m_CDR->m_strLastError = CDR_CERTIFICATION_ERROR;
		return FALSE;
	}

	if (NULL == m_CDR->DLL_ExecuteCommand)
	{
		return FALSE;
	}

	BOOL bResult = FALSE;

	//COMMAND command;
	memset(&m_CDR->m_Cmd,0, sizeof(m_CDR->m_Cmd));

	//REPLY	reply;
	memset(&m_CDR->m_Reply, 0, sizeof(m_CDR->m_Reply));

#if CDR_ENCRYPT_MODE
	BYTE byKey[ENCRYPTION_KEY_LENGTH] = { 0, };
	BYTE byEncIV[ENCRYPTION_IV_LENGTH] = { 0, };
	BYTE byEncryptedSendData[1024] = { 0, };
	int cmdEncryptedLength = 0;
#endif

	m_CDR->m_Cmd.bCommandCode = SANKYO_CDR_CMD_ICC_CONTROL;

	if(TRUE == m_CDR->GetSupportEncryption())
	{
		memcpy(byKey, m_CDR->m_byAPDUkeyData, ENCRYPTION_KEY_LENGTH);
		memcpy(byEncIV, &m_CDR->m_byAPDUkeyData[ENCRYPTION_KEY_LENGTH], ENCRYPTION_IV_LENGTH);
 
		m_CDR->EncryptKeyData_TDES_CBC(SendData, cmdLength, byEncryptedSendData, cmdEncryptedLength, byKey, byEncIV);

		m_CDR->m_Cmd.bParameterCode = SANKYO_CDR_PM_ICC_CONTROL_DES_TRIPLE_APDU_AUTOMATIC;
		m_CDR->m_Cmd.Data.lpbBody = byEncryptedSendData;
		m_CDR->m_Cmd.Data.dwSize = cmdEncryptedLength;
	}
	else
	{
		m_CDR->m_Cmd.bParameterCode = SANKYO_CDR_PM_ICC_CONTROL_AUTOMATIC;
		m_CDR->m_Cmd.Data.lpbBody = SendData;
		m_CDR->m_Cmd.Data.dwSize = cmdLength;
	}

	DWORD dwRet = 0;

	dwRet = m_CDR->DLL_ExecuteCommand(m_CDR->m_szPortNum, &m_CDR->m_Cmd, m_CDR->m_nTimeout, &m_CDR->m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		m_CDR->SetCommError(dwRet);
		return FALSE;
	}

	bResult = (PositiveReply == m_CDR->m_Reply.replyType);

	if (NegativeReply == m_CDR->m_Reply.replyType)
	{
		m_CDR->m_strLastError.Format(_T("C000%c%c"), m_CDR->m_Reply.message.negativeReply.ErrorCode.bE1, m_CDR->m_Reply.message.negativeReply.ErrorCode.bE0);
		LOG(Error, _T("ICCCommand is failed. ErrorCode : [%s]"), m_CDR->m_strLastError);
	}

	if (bResult)
	{
		if(TRUE == m_CDR->GetSupportEncryption())
		{
			DWORD dwEncryptedDataSize = m_CDR->m_Reply.message.positiveReply.Data.dwSize;
			DWORD dwTempSize = 0;

			memset(m_CDR->m_byTempData, 0x00, sizeof(m_CDR->m_byTempData));

			if (dwEncryptedDataSize)
			{
				m_CDR->DecryptKeyData_TDES_CBC(m_CDR->m_Reply.message.positiveReply.Data.bBody, dwEncryptedDataSize, m_CDR->m_byTempData, byKey, byEncIV);
				dwTempSize = m_CDR->ParsingReadData(m_CDR->m_byTempData, dwEncryptedDataSize);
				
				*RecvLen = dwTempSize;

				if(dwTempSize)
				{
					memcpy(RecvData, m_CDR->m_byTempData, dwTempSize);
				}
			}
			else
			{
				*RecvLen = dwEncryptedDataSize;
			}
		}
		else
		{
			DWORD dwCardDataSize = m_CDR->m_Reply.message.positiveReply.Data.dwSize;

			*RecvLen = dwCardDataSize;

			if (dwCardDataSize)
			{
				memcpy(RecvData, &m_CDR->m_Reply.message.positiveReply.Data.bBody, dwCardDataSize);
			}
		}
	}

	return bResult;
}


/** **********************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CDEV_CDR::CDEV_CDR()
{
	// DLL 경로 설정
	CString	strPath = _T("");

	m_CDR = this;
	
	// DLL 인스턴스 취득
	m_hDLL = LoadLibrary(EAGLE_ATM_CDR_DLL);

	DLL_GetDllInformation = (DLL_GETDLLINFORMATION)GetProcAddress(m_hDLL, _T("GetDllInformation"));

	DLL_ConnectDevice = (DLL_CONNECTDEVICE)GetProcAddress(m_hDLL, _T("ConnectDevice"));

	DLL_DisconnectDevice = (DLL_DISCONNECTDEVICE)GetProcAddress(m_hDLL, _T("DisconnectDevice"));

	DLL_CancelCommand = (DLL_CANCELCOMMAND)GetProcAddress(m_hDLL, _T("CancelCommand"));

	DLL_ExecuteCommand = (DLL_EXECUTECOMMAND)GetProcAddress(m_hDLL, _T("ExecuteCommand"));

	DLL_UpdateFirmware = (DLL_UPDATEFIRMWARE)GetProcAddress(m_hDLL, _T("UpdateFirmware"));

	DLL_ICCardTransmit = (DLL_ICCARDTRANSMIT)GetProcAddress(m_hDLL, _T("ICCardTransmit"));

	DLL_SAMTransmit = (DLL_SAMTRANSMIT)GetProcAddress(m_hDLL, _T("SAMTransmit"));

	// Load EMV Kernel
#if (SUPPORT_EMV)

#ifdef _WIN32_WCE

	m_hEMVDLL = LoadLibrary(EAGLE_ATM_EMV_KERNEL_DLL);

	EMVKernel_AppSelection = (DLL_EMVKernel_AppSelection)GetProcAddress(m_hEMVDLL, _T("EMVKernel_AppSelection"));

	if (EMVKernel_AppSelection == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_AppSelection func"));
	}

	EMVKernel_MakeAppLists = (DLL_EMVKernel_MakeAppLists)GetProcAddress(m_hEMVDLL, _T("EMVKernel_MakeAppLists"));

	if (EMVKernel_MakeAppLists == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_MakeAppLists func"));
	}

	EMVKernel_RemoveAppList = (DLL_EMVKernel_RemoveAppList)GetProcAddress(m_hEMVDLL, _T("EMVKernel_RemoveAppList"));

	if (EMVKernel_RemoveAppList == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_RemoveAppList func"));
	}

	EMVKernel_FinalAppSelection = (DLL_EMVKernel_FinalAppSelection)GetProcAddress(m_hEMVDLL, _T("EMVKernel_FinalAppSelection"));

	if (EMVKernel_FinalAppSelection == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_FinalAppSelection func"));
	}

	EMVKernel_StoreValFromRecord = (DLL_EMVKernel_StoreValFromRecord)GetProcAddress(m_hEMVDLL, _T("EMVKernel_StoreValFromRecord"));

	if (EMVKernel_StoreValFromRecord == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_StoreValFromRecord func"));
	}

	EMVKernel_InitApplication = (DLL_EMVKernel_InitApplication)GetProcAddress(m_hEMVDLL, _T("EMVKernel_InitApplication"));

	if (EMVKernel_InitApplication == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_InitApplication func"));
	}

	EMVKernel_ReadAppData = (DLL_EMVKernel_ReadAppData)GetProcAddress(m_hEMVDLL, _T("EMVKernel_ReadAppData"));

	if (EMVKernel_ReadAppData == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_ReadAppData func"));
	}

	EMVKernel_OfflineDataAuth = (DLL_EMVKernel_OfflineDataAuth)GetProcAddress(m_hEMVDLL, _T("EMVKernel_OfflineDataAuth"));

	if (EMVKernel_OfflineDataAuth == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_OfflineDataAuth func"));
	}

	EMVKernel_ProcessRestrict = (DLL_EMVKernel_ProcessRestrict)GetProcAddress(m_hEMVDLL, _T("EMVKernel_ProcessRestrict"));

	if (EMVKernel_ProcessRestrict == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_ProcessRestrict func"));
	}

	EMVKernel_CardholderVerify = (DLL_EMVKernel_CardholderVerify)GetProcAddress(m_hEMVDLL, _T("EMVKernel_CardholderVerify"));

	if (EMVKernel_CardholderVerify == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_CardholderVerify func"));
	}

	EMVKernel_TerminalRiskMgmt = (DLL_EMVKernel_TerminalRiskMgmt)GetProcAddress(m_hEMVDLL, _T("EMVKernel_TerminalRiskMgmt"));

	if (EMVKernel_TerminalRiskMgmt == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_TerminalRiskMgmt func"));
	}

	EMVKernel_TerminalActionAnalysis = (DLL_EMVKernel_TerminalActionAnalysis)GetProcAddress(m_hEMVDLL, _T("EMVKernel_TerminalActionAnalysis"));

	if (EMVKernel_TerminalActionAnalysis == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_TerminalActionAnalysis func"));
	}

	EMVKernel_CardActionAnalysis = (DLL_EMVKernel_CardActionAnalysis)GetProcAddress(m_hEMVDLL, _T("EMVKernel_CardActionAnalysis"));

	if (EMVKernel_CardActionAnalysis == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_CardActionAnalysis func"));
	}

	EMVKernel_OnlineProcess = (DLL_EMVKernel_OnlineProcess)GetProcAddress(m_hEMVDLL, _T("EMVKernel_OnlineProcess"));

	if (EMVKernel_OnlineProcess == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_OnlineProcess func"));
	}

	EMVKernel_Completion = (DLL_EMVKernel_Completion)GetProcAddress(m_hEMVDLL, _T("EMVKernel_Completion"));

	if (EMVKernel_Completion == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_Completion func"));
	}

	EMVKernel_CDRCommFunc = (DLL_EMVKernel_CDRCommFunc)GetProcAddress(m_hEMVDLL, _T("EMVKernel_CDRCommFunc"));

	if (EMVKernel_CDRCommFunc == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_CDRCommFunc func"));
	}

	EMVKernel_CDRCommFunc(ICCCommand);

	EMVKernel_Tlv_GetVal = (DLL_EMVKernel_Tlv_GetVal)GetProcAddress(m_hEMVDLL, _T("EMVKernel_Tlv_GetVal"));

	if (EMVKernel_Tlv_GetVal == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_Tlv_GetVal func"));
	}

	EMVKernel_Tlv_StoreVal = (DLL_EMVKernel_Tlv_StoreVal)GetProcAddress(m_hEMVDLL, _T("EMVKernel_Tlv_StoreVal"));

	if (EMVKernel_Tlv_StoreVal == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_Tlv_StoreVal func"));
	}

	EMVKernel_Initial_Variable = (DLL_EMVKernel_Initial_Variable)GetProcAddress(m_hEMVDLL, _T("EMVKernel_Initial_Variable"));

	if (EMVKernel_Initial_Variable == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_Initial_Variable func"));
	}

	EMVKernel_SetDebug = (DLL_EMVKernel_SetDebug)GetProcAddress(m_hEMVDLL, _T("EMVKernel_SetDebug"));

	if (EMVKernel_SetDebug == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_SetDebug func"));
	}
	else
	{
		#if (ACTIVE_EMV_KERNEL_DEBUG)
		EMVKernel_SetDebug(TRUE);
		#endif
	}

	EMVKernel_SetKernelLogPath = (DLL_EMVKernel_SetKernelLogPath)GetProcAddress(m_hEMVDLL, _T("EMVKernel_SetKernelLogPath"));

	if (EMVKernel_SetKernelLogPath == NULL)
	{
		LOG(Error, _T("Load failed EMVKernel_SetKernelLogPath func"));
	}
	else
	{
		#if (ACTIVE_EMV_KERNEL_DEBUG)
		EMVKernel_SetKernelLogPath((unsigned char*)"\\Trace", (unsigned char*)"log");
		#endif
	}

#else	// Win32: bind the simulator instead of loading the WinCE/ARM kernel

	m_hEMVDLL = NULL;

	EMVKernel_AppSelection				= EMVSim_AppSelection;
	EMVKernel_MakeAppLists				= EMVSim_MakeAppLists;
	EMVKernel_RemoveAppList				= EMVSim_RemoveAppList;
	EMVKernel_FinalAppSelection			= EMVSim_FinalAppSelection;
	EMVKernel_StoreValFromRecord		= EMVSim_StoreValFromRecord;
	EMVKernel_InitApplication			= EMVSim_InitApplication;
	EMVKernel_ReadAppData				= EMVSim_ReadAppData;
	EMVKernel_OfflineDataAuth			= EMVSim_OfflineDataAuth;
	EMVKernel_ProcessRestrict			= EMVSim_ProcessRestrict;
	EMVKernel_CardholderVerify			= EMVSim_CardholderVerify;
	EMVKernel_TerminalRiskMgmt			= EMVSim_TerminalRiskMgmt;
	EMVKernel_TerminalActionAnalysis	= EMVSim_TerminalActionAnalysis;
	EMVKernel_CardActionAnalysis		= EMVSim_CardActionAnalysis;
	EMVKernel_OnlineProcess				= EMVSim_OnlineProcess;
	EMVKernel_Completion				= EMVSim_Completion;
	EMVKernel_CDRCommFunc				= EMVSim_CDRCommFunc;
	EMVKernel_Tlv_GetVal				= EMVSim_Tlv_GetVal;
	EMVKernel_Tlv_StoreVal				= EMVSim_Tlv_StoreVal;
	EMVKernel_Initial_Variable			= EMVSim_Initial_Variable;
	EMVKernel_SetDebug					= EMVSim_SetDebug;
	EMVKernel_SetKernelLogPath			= EMVSim_SetKernelLogPath;

	EMVKernel_CDRCommFunc(ICCCommand);

	LOG(Info, _T("[Simulator] EMV kernel stubs bound (EMVKernel_API.dll is WinCE only)"));

#endif	// _WIN32_WCE
#endif

	// 포트 번호
	memset(m_szPortNum, 0, sizeof(m_szPortNum));

	// 에러 코드
	m_strLastError = _T("");

	m_dwReplyType = 0;
	m_dwReturnValue = 0;

	m_nCDRStatus = CARD_ERROR;

	memset(m_byKeyExchangeKey, 0x00, sizeof(m_byKeyExchangeKey));
	memset(m_byDeviceAuthData, 0x00, sizeof(m_byDeviceAuthData));
	memset(m_byMagkeyData, 0x00, sizeof(m_byMagkeyData));
	memset(m_byAPDUkeyData, 0x00, sizeof(m_byAPDUkeyData));

	m_bKeyAuthError = FALSE;
	m_bSupportEncrypt4CDR = FALSE;

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));
}


/** **********************************************************
*	@brief		소멸자
*	@retval		없음
************************************************************/
CDEV_CDR::~CDEV_CDR()
{
	if (m_hDLL)
	{
		FreeLibrary(m_hDLL);

		m_hDLL = NULL;
	}
}


/** **********************************************************
*	@brief		Is DLL Loaded
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::IsDllLoaded(void)
{
#if	EMULATION_DEVICE
	return TRUE;
#endif

	return (NULL != m_hDLL);
}


/** **********************************************************
*	@brief		열기
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::Open(HWND hWnd, int nPortNum, int nTimeout)
{
#if	EMULATION_CDR_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	return CDevSim::Confirm(_T("CDR"), _T("CDR_Open"));
#endif

	if (NULL == DLL_ConnectDevice)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	HRESULT hResult = 0;
	BOOL bRet = FALSE;
	CString strLog = _T("");
	strLog.Format(_T("CDEV_CDR::Open() Start"));
	LOG(Info, strLog);

	// Timeout
	m_nTimeout = nTimeout * 1000;

	wsprintf(m_szPortNum, _T("COM%d:"), nPortNum);

	hResult = DLL_ConnectDevice(m_szPortNum, 38400);

	if(EAGLE_CDR_NO_ERROR == hResult)
	{
		bRet = TRUE;
		m_strLastError = _T("");
	}
	else
	{
		m_strLastError.Format(_T("C00081"));
		strLog.Format(_T("CDEV_CDR::Open() DLL_ConnectDevice is failed. ErrorCode - [%s]"), m_strLastError);
		LOG(Error, strLog);
		bRet = FALSE;
	}

	m_CDRLock.Unlock();

	return bRet;
}


/** **********************************************************
*	@brief		닫기
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::Close()
{
#if	EMULATION_CDR_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	CDevSimCDRDlg::Destroy();
	return CDevSim::Confirm(_T("CDR"), _T("CDR_Close"));
#endif

	if (NULL == DLL_DisconnectDevice)
	{
		return FALSE;
	}

	return (EAGLE_CDR_NO_ERROR == DLL_DisconnectDevice(m_szPortNum));
}


/** **********************************************************
*	@brief		초기화
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::Initialize()
{
#if	EMULATION_CDR_DEVICE
	return CDevSim::Confirm(_T("CDR"), _T("CDR_Initialize"));
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;

	CString strLog = _T("");
	strLog.Format(_T("CDEV_CDR::Initialize() Start"));
	LOG(Info, strLog);


	//COMMAND command;

	BYTE    data[] = { 0x32, 0x30, 0x30 };

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = SANKYO_CDR_CMD_INITIAL;

	m_Cmd.bParameterCode = SANKYO_CDR_PM_INITIAL;

	m_Cmd.Data.lpbBody = data;

	m_Cmd.Data.dwSize = sizeof(data);


	//REPLY	reply;

	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	if(TRUE == m_bKeyAuthError)
	{
		LOG(Error, _T("CDEV_CDR::Initialize() System Error."));
		m_strLastError = CDR_CERTIFICATION_ERROR;
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
		strLog.Format(_T("CDEV_CDR::Initialize()  is failed. ErrorCode - [%s]"), m_strLastError);
		LOG(Error, strLog);
	}
	else
	{
		m_nCDRStatus = CARD_NORMAL;
		m_strLastError = _T("");	//Clear ErrCode
	}

	m_CDRLock.Unlock();

	return bResult;
}


/** **********************************************************
*	@brief		LED 설정
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::SetLed(BOOL bOn)
{
#if	EMULATION_CDR_DEVICE
	// LED control is cosmetic and fires continuously, so it is simulated silently.
	return TRUE;
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;


	//COMMAND command;

	BYTE    data[] = { 0x30 };

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = SANKYO_CDR_CMD_LED_CONTOL;

	m_Cmd.bParameterCode = bOn ? SANKYO_CDR_PM_LED_CONTROL_LED_GREEN : SANKYO_CDR_PM_LED_CONTROL_LED_OFF;

	m_Cmd.Data.lpbBody = data;

	m_Cmd.Data.dwSize = sizeof(data);


	//REPLY	reply;

	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
	}

	m_CDRLock.Unlock();

	return bResult;
}


/** **********************************************************
*	@brief		카드 투입
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::InsertCard()
{
#if	EMULATION_CDR_DEVICE
	// Card-present polling: this is called repeatedly in a wait loop. Rather
	// than storming a dialog on every poll, the panel is shown once and this
	// just checks whether ACCEPT has been pressed yet.
	CDevSimCDRDlg::Show();
	return CDevSimCDRDlg::IsCardDetected();
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;

	//COMMAND command;
	//memset(&command, 0, sizeof(command));

//	BYTE    data[] = { 0x39, 0x39, 0x39, 0x39, 0x39, 0x30, 0x30, 0x30 }; // org
//	BYTE    data[] = { 0x30, 0x30, 0x30, 0x30, 0x31, 0x30, 0x30, 0x30 }; // 1000ms wait
	BYTE    data[] = { 0x30, 0x30, 0x30, 0x30, 0x30, 0x35, 0x30, 0x30 }; // 500ms wait

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = SANKYO_CDR_CMD_STATUS;

	m_Cmd.bParameterCode = SANKYO_CDR_PM_STATUS_INTAKE;

	m_Cmd.Data.lpbBody = data;

	m_Cmd.Data.dwSize = sizeof(data);


	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
	}

	m_CDRLock.Unlock();

	return bResult;
}


void CDEV_CDR::CancelInsertCard()
{
#if	EMULATION_CDR_DEVICE
	CDevSimCDRDlg::Reset();
#endif
}


void CDEV_CDR::ShowRemoveCardWait(BOOL bShow)
{
#if	EMULATION_CDR_DEVICE
	CDevSimCDRDlg::ShowForRemoval(bShow);
#else
	UNREFERENCED_PARAMETER(bShow);
#endif
}


/** **********************************************************
*	@brief		IC POWER ON
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::ICCPowerOn(byte* ATR, DWORD &dwDataSize)
{
#if	EMULATION_CDR_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	// OK -> chip power-on succeeds and the EMV flow proceeds; Cancel -> it
	// fails and CSVC_Manager::Proc_Client_EMV_CardRead() falls back to the
	// magstripe read (CDevSimTrack2Dlg, via ReadCardData_ISO2()).
	if (!CDevSim::Confirm(_T("CDR"), _T("CDR_ICCPowerOn")))
	{
		dwDataSize = 0;
		return FALSE;
	}

	dwDataSize = (DWORD)CDevSim::ReadHexBytes(_T("CDR"), _T("ICCAtr"),
		_T("3B6800000073C84013009000"), ATR, 64);

	return TRUE;
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;

	CString strLog = _T("");
	strLog.Format(_T("CDEV_CDR::ICCPowerOn 1 () Start"));
	LOG(Info, strLog);

	//COMMAND command;
	BYTE    data[] = { 0x30 };

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = SANKYO_CDR_CMD_ICC_CONTROL;
	m_Cmd.bParameterCode = SANKYO_CDR_PM_ICC_CONTROL_ACTIVATE;
	m_Cmd.Data.lpbBody = data;
	m_Cmd.Data.dwSize = sizeof(data);

	//REPLY	reply;

	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
		LOG(Error, _T("ICCPowerOn is failed. ErrorCode - [%s]"), m_strLastError);
	}

	if(bResult)
	{
		m_strLastError.Empty();

		dwDataSize = m_Reply.message.positiveReply.Data.dwSize;

		//BYTE* pCardDataBuff = reply.message.positiveReply.Data.bBody;

		if (dwDataSize)
		{
			//memcpy(ATR, pCardDataBuff, dwDataSize);
			memcpy(ATR, m_Reply.message.positiveReply.Data.bBody, dwDataSize);
		}
	}

	m_CDRLock.Unlock();

	return bResult;
}


/** **********************************************************
*	@brief		IC POWER OFF
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::ICCPowerOff()
{
#if	EMULATION_CDR_DEVICE
	return CDevSim::Confirm(_T("CDR"), _T("CDR_ICCPowerOff"));
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;

	CString strLog = _T("");
	strLog.Format(_T("CDEV_CDR::ICCPowerOff() Start"));
	LOG(Info, strLog);

	//COMMAND command;
	BYTE    data[] = { 0x00 };

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = SANKYO_CDR_CMD_ICC_CONTROL;
	m_Cmd.bParameterCode = SANKYO_CDR_PM_ICC_CONTROL_DEACTIVATE;
	m_Cmd.Data.lpbBody = NULL;
	m_Cmd.Data.dwSize = 0;

	//REPLY	reply;

	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		LOG(Error, _T("ICCPowerOff is failed. ErrorCode - [%c %c]"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
	}

	strLog.Format(_T("CDEV_CDR::ICCPowerOff() End"));
	LOG(Info, strLog);

	m_CDRLock.Unlock();

	return bResult;
}


/** **********************************************************
*	@brief		Read Latch mode
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::ReadLatch(BYTE *mode)
{
#if	EMULATION_CDR_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	if (mode != NULL)
		*mode = CDevSim::ReadByte(_T("CDR"), _T("LatchMode"), 0x30);

	return TRUE;
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;
	//COMMAND command;
	CString strLog;

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = 0x36;
	m_Cmd.bParameterCode = 0x44;
	m_Cmd.Data.lpbBody = NULL;
	m_Cmd.Data.dwSize = 0;

	//REPLY	reply;

	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
		strLog.Format(_T("CDEV_CDR::ReadLatch() ReadLatch is failed. ErrorCode - [%s]"), m_strLastError);
		LOG(Error, strLog);
	}

	if(m_Reply.message.positiveReply.Data.dwSize > 0)
	{
		memcpy(mode, &m_Reply.message.positiveReply.Data.bBody, m_Reply.message.positiveReply.Data.dwSize);
	}

	m_CDRLock.Unlock();

	return bResult;
}

/** **********************************************************
*	@brief		Set Latch mode
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::SetLatch(BOOL bOn)
{
#if	EMULATION_CDR_DEVICE
	return CDevSim::Confirm(_T("CDR"), _T("CDR_SetLatch"));
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();
	
	BOOL bResult = FALSE;
	//REPLY	reply;
	//COMMAND command;
	BYTE    data[] = { 0x30 };

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = 0x37;
	m_Cmd.bParameterCode = 0x44;

	if ( bOn == TRUE )
		data[0] = 0x31;
	else
		data[0] = 0x30;

	m_Cmd.Data.lpbBody = data;
	m_Cmd.Data.dwSize = sizeof(data);

	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
	}

	m_CDRLock.Unlock();	// 하기 Initialize에서 Lock처리가 되어 여기서 해제한다.

	if (bResult == TRUE)
	{
		// 1278 Model에서는 DeviceReset명령을 적용하지 않아도 SetLetch 명령이 동작함.
		// 1270 Model에서는 DeviceReset명령을 수행해야 SetLetch 명령이 동작함.
		if(FALSE == m_bSupportEncrypt4CDR)
		{
			// Latch 적용을 위해 Device Reset 및 Initialize 수행
			bResult = DeviceReset();
		
			if (bResult == TRUE)
			{
				// Initialize 수행
				bResult = Initialize();

				if (bResult == FALSE)
				{
					LOG(Error, _T("Device Initialize is failed"));
				}
			}
			else
			{
				LOG(Error, _T("Device Reset is failed"));
			}
		}
	}

	return bResult;
}

/** **********************************************************
*	@brief		카드 제거
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::RemoveCard()
{
#if	EMULATION_CDR_DEVICE
	// Card cycle complete: re-arm the panel (and clear the captured Track2)
	// for the next InsertCard().
	CDevSimCDRDlg::Reset();
	return CDevSim::Confirm(_T("CDR"), _T("CDR_RemoveCard"));
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;
	CString strLog = _T("");


	//COMMAND command;
	//memset(&command, 0, sizeof(command));

//	BYTE    data[] = { 0x39, 0x39, 0x39, 0x39, 0x39, 0x30, 0x30, 0x30 };
//	BYTE byAPData[] = { 0x39, 0x39, 0x39, 0x39, 0x39, 0x30, 0x30, 0x30 };
	BYTE data[] = { 0x30, 0x30, 0x30, 0x30, 0x31, 0x30, 0x30, 0x30 };	// 1000ms wait

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = SANKYO_CDR_CMD_STATUS;

	m_Cmd.bParameterCode = SANKYO_CDR_PM_STATUS_WITHDRAW;

	m_Cmd.Data.lpbBody = data;

	m_Cmd.Data.dwSize = sizeof(data);


	//REPLY	reply;

	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
		//LOG(Error, _T("RemoveCard is failed. ErrorCode - [%c %c]"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
	}

	m_dwReplyType = m_Reply.replyType;

	m_dwReturnValue = dwRet;

	m_CDRLock.Unlock();

	return bResult;
}


/** **********************************************************
*	@brief		카드 읽기 (ISO1)
*	@retval		없음
************************************************************/
int CDEV_CDR::ReadCardData_ISO1(CString* pISO1)
{
#if	EMULATION_CDR_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	*pISO1 = CDevSim::ReadString(_T("CDR"), _T("TrackISO1"), _T("TEST ISO1 TRACK"));
	return pISO1->GetLength();
#endif

	int nDataSize = 0;

	if(TRUE == m_bKeyAuthError)
	{
		LOG(Error, _T("CDEV_CDR::ReadCardData_ISO1() System Error."));
		m_strLastError = CDR_CERTIFICATION_ERROR;
		return nDataSize;
	}

	if (NULL == DLL_ExecuteCommand)
	{
		return nDataSize;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;
	BYTE byKey[ENCRYPTION_KEY_LENGTH] = { 0, };
	BYTE byEncIV[ENCRYPTION_IV_LENGTH] = { 0, };

	//COMMAND command;

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = SANKYO_CDR_CMD_READ;

	if(TRUE == GetSupportEncryption())
	{
		m_Cmd.bParameterCode = SANKYO_CDR_PM_READ_ISO1_ENCRYT;
	}
	else
	{
		m_Cmd.bParameterCode = SANKYO_CDR_PM_READ_ISO1;
	}

	m_Cmd.Data.lpbBody = NULL;

	m_Cmd.Data.dwSize = 0;


	//REPLY	reply;

	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return nDataSize;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
		LOG(Error, _T("ReadCardData_ISO1 is failed. ErrorCode - [%s]"), m_strLastError);
	}

	if (bResult)
	{
		m_strLastError.Empty();

		if(TRUE == GetSupportEncryption())
		{
			DWORD dwEncryptedDataSize = m_Reply.message.positiveReply.Data.dwSize;

			memset(m_byTempData, 0x00, sizeof(m_byTempData));

			if (dwEncryptedDataSize)
			{
				memcpy(byKey, m_byMagkeyData, ENCRYPTION_KEY_LENGTH);
				memcpy(byEncIV, &m_byMagkeyData[ENCRYPTION_KEY_LENGTH], ENCRYPTION_IV_LENGTH);

				DecryptKeyData_TDES_CBC(m_Reply.message.positiveReply.Data.bBody, dwEncryptedDataSize, m_byTempData, byKey, byEncIV);
				*pISO1 = CString(m_byTempData);

				// Track Data인 경우 return의 Data Size는 의미가 없으므로 임시적으로 암호화된 Length 정보를 return함. (상위국이 Size를 보는 경우에는 수정 필요함)
				nDataSize = dwEncryptedDataSize;				
			}
		}
		else
		{
			nDataSize = m_Reply.message.positiveReply.Data.dwSize;

			if (nDataSize)
			{
				*pISO1 = CString(m_Reply.message.positiveReply.Data.bBody);
			}
		}
	}

	m_CDRLock.Unlock();

	return nDataSize;
}


/** **********************************************************
*	@brief		카드 읽기 (ISO2)
*	@retval		없음
************************************************************/
int CDEV_CDR::ReadCardData_ISO2(CString* pISO2)
{
#if	EMULATION_CDR_DEVICE
	// This is called exactly when a real reader would be asked for the
	// magstripe track - the plain (non-EMV) read, or the EMV chip-failure
	// fallback (see CSVC_Manager::Proc_Client_ReadCard()) - so the Track2
	// panel is prompted for right here rather than reusing whatever was
	// typed at InsertCard() time.
	*pISO2 = CDevSimTrack2Dlg::PromptForTrack2();
	return pISO2->GetLength();
#endif

	int nDataSize = 0;

	if(TRUE == m_bKeyAuthError)
	{
		LOG(Error, _T("CDEV_CDR::ReadCardData_ISO2() System Error."));
		m_strLastError = CDR_CERTIFICATION_ERROR;
		return nDataSize;
	}

	if (NULL == DLL_ExecuteCommand)
	{
		return nDataSize;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;
	BYTE byKey[ENCRYPTION_KEY_LENGTH] = { 0, };
	BYTE byEncIV[ENCRYPTION_IV_LENGTH] = { 0, };

	//COMMAND command;

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = SANKYO_CDR_CMD_READ;

	if(TRUE == GetSupportEncryption())
	{
		m_Cmd.bParameterCode = SANKYO_CDR_PM_READ_ISO2_ENCRYT;
	}
	else
	{
		m_Cmd.bParameterCode = SANKYO_CDR_PM_READ_ISO2;
	}

	m_Cmd.Data.lpbBody = NULL;

	m_Cmd.Data.dwSize = 0;

	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return nDataSize;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
		LOG(Error, _T("ReadCardData_ISO2 is failed. ErrorCode - [%s]"), m_strLastError);
	}

	if (bResult)
	{
		m_strLastError.Empty();

		if(TRUE == GetSupportEncryption())
		{
			DWORD dwEncryptedDataSize = m_Reply.message.positiveReply.Data.dwSize;

			memset(m_byTempData, 0x00, sizeof(m_byTempData));

			if (dwEncryptedDataSize)
			{
				memcpy(byKey, m_byMagkeyData, ENCRYPTION_KEY_LENGTH);
				memcpy(byEncIV, &m_byMagkeyData[ENCRYPTION_KEY_LENGTH], ENCRYPTION_IV_LENGTH);

				DecryptKeyData_TDES_CBC(m_Reply.message.positiveReply.Data.bBody, dwEncryptedDataSize, m_byTempData, byKey, byEncIV);
				*pISO2 = CString(m_byTempData);
				// Track Data인 경우 return의 Data Size는 의미가 없으므로 임시적으로 암호화된 Length 정보를 return함. (상위국이 Size를 보는 경우에는 수정 필요함)
				nDataSize = dwEncryptedDataSize;				
			}
		}
		else
		{
			nDataSize = m_Reply.message.positiveReply.Data.dwSize;

			if (nDataSize)
			{
				*pISO2 = CString(m_Reply.message.positiveReply.Data.bBody);
			}
		}
	}

	m_CDRLock.Unlock();

	return nDataSize;
}


/** **********************************************************
*	@brief		카드 읽기 (ISO3)
*	@retval		없음
************************************************************/
int CDEV_CDR::ReadCardData_ISO3(CString* pISO3)
{
#if	EMULATION_CDR_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	*pISO3 = CDevSim::ReadString(_T("CDR"), _T("TrackISO3"), _T("SAMPLE ISO3 TRACK"));
	return pISO3->GetLength();
#endif

	int nDataSize = 0;

	if(TRUE == m_bKeyAuthError)
	{
		LOG(Error, _T("CDEV_CDR::ReadCardData_ISO3() System Error."));
		m_strLastError = CDR_CERTIFICATION_ERROR;
		return nDataSize;
	}

	if (NULL == DLL_ExecuteCommand)
	{
		return nDataSize;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;
	BYTE byKey[ENCRYPTION_KEY_LENGTH] = { 0, };
	BYTE byEncIV[ENCRYPTION_IV_LENGTH] = { 0, };

	//COMMAND command;

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = SANKYO_CDR_CMD_READ;

	if(TRUE == GetSupportEncryption())
	{
		m_Cmd.bParameterCode = SANKYO_CDR_PM_READ_ISO3_ENCRYT;
	}
	else
	{
		m_Cmd.bParameterCode = SANKYO_CDR_PM_READ_ISO3;
	}

	m_Cmd.Data.lpbBody = NULL;

	m_Cmd.Data.dwSize = 0;

	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return nDataSize;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
		LOG(Error, _T("ReadCardData_ISO3 is failed. ErrorCode - [%s]"), m_strLastError);
	}

	if (bResult)
	{
		m_strLastError.Empty();

		if(TRUE == GetSupportEncryption())
		{
			DWORD dwEncryptedDataSize = m_Reply.message.positiveReply.Data.dwSize;

			memset(m_byTempData, 0x00, sizeof(m_byTempData));

			if (dwEncryptedDataSize)
			{
				memcpy(byKey, m_byMagkeyData, ENCRYPTION_KEY_LENGTH);
				memcpy(byEncIV, &m_byMagkeyData[ENCRYPTION_KEY_LENGTH], ENCRYPTION_IV_LENGTH);

				DecryptKeyData_TDES_CBC(m_Reply.message.positiveReply.Data.bBody, dwEncryptedDataSize, m_byTempData, byKey, byEncIV);
				*pISO3 = CString(m_byTempData);

				// Track Data인 경우 return의 Data Size는 의미가 없으므로 임시적으로 암호화된 Length 정보를 return함. (상위국이 Size를 보는 경우에는 수정 필요함)
				nDataSize = dwEncryptedDataSize;				
			}
		}
		else
		{
			nDataSize = m_Reply.message.positiveReply.Data.dwSize;

			if (nDataSize)
			{
				*pISO3 = CString(m_Reply.message.positiveReply.Data.bBody);
			}
		}
	}

	m_CDRLock.Unlock();

	return nDataSize;
}


/** **********************************************************
*	@brief		버퍼 클리어
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::ClearBuffer()
{
#if	EMULATION_CDR_DEVICE
	// Buffer housekeeping has no operator-visible effect; simulated silently.
	return TRUE;
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;


	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = SANKYO_CDR_CMD_READ;

	m_Cmd.bParameterCode = SANKYO_CDR_PM_READ_CLEAR_BUFF;

	m_Cmd.Data.lpbBody = NULL;

	m_Cmd.Data.dwSize = 0x00;


	//REPLY	reply;

	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		LOG(Error, _T("ClearBuffer is failed. ErrorCode - [%c %c]"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
	}

	m_CDRLock.Unlock();

	return bResult;
}


/** **********************************************************
*	@brief		취소
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::Cancel()
{
#if	EMULATION_CDR_DEVICE
	return CDevSim::Confirm(_T("CDR"), _T("CDR_Cancel"));
#endif

	if (NULL == DLL_CancelCommand)
	{
		return FALSE;
	}

	return (EAGLE_CDR_NO_ERROR == DLL_CancelCommand(m_szPortNum));
}


/************************************************************
*	@brief		Reset
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::DeviceReset()
{

#if	EMULATION_CDR_DEVICE
	return CDevSim::Confirm(_T("CDR"), _T("CDR_DeviceReset"));
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;

	CString strLog = _T("");
	strLog.Format(_T("CDEV_CDR::DeviceReset() Start"));
	LOG(Info, strLog);

	//COMMAND command;

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = 0x7A;

	m_Cmd.bParameterCode = 0x30;

	m_Cmd.Data.lpbBody = NULL;

	m_Cmd.Data.dwSize = 0x00;


	//REPLY	reply;
	
	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
	}
	else
	{
		m_nCDRStatus = CARD_NORMAL;
		m_strLastError = _T("");
	}

	m_CDRLock.Unlock();

	return bResult;
}

/** **********************************************************
*	@brief		에러코드 취득
*	@retval		없음
************************************************************/
CString CDEV_CDR::GetLastError(void)
{
	return m_strLastError;
}


/** **********************************************************
*	@brief		에러코드 clear
*	@retval		없음
************************************************************/
void CDEV_CDR::ClearErrorCode(void)
{
	m_strLastError.Empty();
}


/** **********************************************************
*	@brief		DLL 버전 취득
*	@retval		없음
************************************************************/
BOOL CDEV_CDR::DLLInformation(LPDLL_INFORMATION lpdll_information)
{
#if	EMULATION_CDR_DEVICE
	return TRUE;
#endif

	if (NULL == DLL_GetDllInformation)
	{
		return FALSE;
	}

	DLL_GetDllInformation(lpdll_information);

	return TRUE;
}


/** *********************************************************
*	@brief	ReplyType
*	@retval	ReplyType
************************************************************/
DWORD CDEV_CDR::GetReplyType()
{
	return m_dwReplyType;
}

DWORD CDEV_CDR::GetReturnValue()
{
	return m_dwReturnValue;
}

BOOL CDEV_CDR::GetCDR_DeviceStatus()
{
	return m_nCDRStatus;
}

/** *********************************************************
*	@brief	Setting Communication ErrorCode
*	@retval	
************************************************************/
void CDEV_CDR::SetCommError( DWORD dwRetCode )
{
	switch (dwRetCode)
	{
	case EAGLE_CDR_DEVICE_NOT_CONNECTED_ERROR:
		m_strLastError.Format(_T("C00081"));
		break;
	case EAGLE_CDR_CANCEL_COMMAND_SESSION_ERROR:
		m_strLastError.Format(_T("C00082"));
		break;
	case EAGLE_CDR_FAILED_TO_SEND_COMMAND_ERROR:
		m_strLastError.Format(_T("C00083"));
		break;
	case EAGLE_CDR_FAILED_TO_RECEIVE_REPLY_ERROR:
		m_strLastError.Format(_T("C00084"));
		break;
	case EAGLE_CDR_COMMAND_CANCELED:
		m_strLastError.Format(_T("C00085"));
		break;
	case EAGLE_CDR_REPLY_TIMEOUT:
		m_strLastError.Format(_T("C00086"));
		break;
	default: //Unknown Communication Error
		m_strLastError.Format(_T("C000FF"));
		break;
	}

	m_nCDRStatus = CARD_ERROR;

	return;
}

BOOL CDEV_CDR::GetVersionInfo()
{
#if	EMULATION_CDR_DEVICE
	return TRUE;
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;


	//COMMAND command;

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = SANKYO_CDR_CMD_REVISION;

	m_Cmd.bParameterCode = SANKYO_CDR_PM_REVISION_USER;

	m_Cmd.Data.lpbBody = NULL;

	m_Cmd.Data.dwSize = 0;


	//REPLY	reply;

	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if (NegativeReply == m_Reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), m_Reply.message.negativeReply.ErrorCode.bE1, m_Reply.message.negativeReply.ErrorCode.bE0);
	}

	if (bResult)
	{
		DWORD dwReadDataSize = m_Reply.message.positiveReply.Data.dwSize;

		//BYTE* pReadBuff = m_Reply.message.positiveReply.Data.bBody;

		if (dwReadDataSize)
		{
			//CHAR* pReadData = new CHAR[dwReadDataSize + 1];

			//memcpy(pReadData, pReadBuff, dwReadDataSize);

			//pReadData[dwReadDataSize] = 0x00;

			m_strCDR_FW_Version = CString(m_Reply.message.positiveReply.Data.bBody);

			LOG(Info, _T("Firmware Version : [%s]"), m_strCDR_FW_Version);

			if(m_strCDR_FW_Version.CompareNoCase(CDR_ENCRYPT_SUPPORT_VERSION) >= 0)
			{
				LOG(Info, _T("Get Version - CDR - This F/W supports encryption."));
				m_bSupportEncrypt4CDR = TRUE;
			}
			else
			{
				LOG(Info, _T("Get Version - CDR - This F/W not support encryption."));
				m_bSupportEncrypt4CDR = FALSE;
			}

			//delete [] pReadData;
		}
	}

	m_CDRLock.Unlock();

	return bResult;
}

/** *********************************************************
*	@brief	Device Status Read
*	@retval	Card Position Status
************************************************************/
BYTE CDEV_CDR::GetMediaStatus()
{
#if	EMULATION_CDR_DEVICE
	// Base status is auto-answered from DeviceSim.ini, no dialog - but
	// CARD_REAR_DETECT is ORed in live once the card-present panel's OK has
	// been pressed, so callers that check "is a card actually in the reader"
	// (e.g. the EMV fallback's bail-out check) see it without the tester
	// having to hand-edit the ini mid-transaction.
	BYTE byStatus = CDevSim::ReadByte(_T("CDR"), _T("MediaStatus"), CARD_EMPTY);

	if (CDevSimCDRDlg::IsCardDetected())
		byStatus |= CARD_REAR_DETECT;

	return byStatus;
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;
	BYTE byMediaStatus = 0x30;	// Default Empty

	//COMMAND command;
	//REPLY	reply;

	memset(&m_Cmd, 0, sizeof(m_Cmd));
	memset(&m_Reply, 0, sizeof(m_Reply));

	m_Cmd.bCommandCode = SANKYO_CDR_CMD_STATUS;
	m_Cmd.bParameterCode = SANKYO_CDR_PM_STATUS_INQUIRE;
	m_Cmd.Data.lpbBody = NULL;
	m_Cmd.Data.dwSize = 0;

	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &m_Cmd, m_nTimeout, &m_Reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == m_Reply.replyType);

	if(bResult)
		byMediaStatus = m_Reply.message.positiveReply.StatusCode.bSt1;

	m_CDRLock.Unlock();

	return byMediaStatus;
}


/** *********************************************************
*	@brief	IsEMV_Enable : EMV 거래 지원 여부
*	@retval	TRUE - EMV set, FALSE - MS set
************************************************************/
BOOL CDEV_CDR::IsEMV_Enable()
{
	if (CEagleDataManager::GetInstance()->m_Config.m_Device.strEMV_Enable == S_ENABLE)
		return TRUE;

	return FALSE;
}


/** *********************************************************
*	@brief	Load_AIDs_InformFromFile : EMV Config File을 Memory에 Load하는 함수
*	@retval	없음
************************************************************/
void CDEV_CDR::Load_AIDs_InformFromFile()
{
	CString		strEMVConfig;
	CString		strSection, strKey, strReturnValue;
	int			nLoadedCount = 0, nIndex = 0;

	memset(&m_sEMV_Config_Data, 0, sizeof(m_sEMV_Config_Data));

	//strEMVConfig.Format(_T("%s%s"), CUtil::GetAppPath(), EAGLE_ATM_EMV_CONFIG_PATH_NAME);
	strEMVConfig.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, EAGLE_ATM_EMV_CONFIG_PATH_NAME);

	if (CUtil::IsExistFile(strEMVConfig) == FALSE)
	{
		// Default Folder에서 File Copy하도록 로직 보완
		CString strDefaultPath = _T("");
		strDefaultPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, EAGLE_ATM_EMV_CONFIG_PATH_NAME);
		CopyFile(strDefaultPath, strEMVConfig, FALSE);
		CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
	}

	CIniFile iniEMV_AIDList(strEMVConfig);

	while(TRUE)
	{
		strSection.Format(_T("AIDLIST_%02d"), nIndex + 1);
		strKey = _T("AID_USE");

		strReturnValue = iniEMV_AIDList.ReadString(strSection, strKey, _T(""));

		if (strReturnValue.IsEmpty())
			break;

		LOG(Info, _T("%s - %s - %s"), strSection, strKey, strReturnValue);

		if (strReturnValue == S_ENABLE)
		{
			m_sEMV_Config_Data.m_sEMV_Config[nIndex].AidUse = 1;
		}
		else
		{
			m_sEMV_Config_Data.m_sEMV_Config[nIndex].AidUse = 0;
		}

		strKey = _T("AID");

		strReturnValue = iniEMV_AIDList.ReadString(strSection, strKey, _T(""));

		LOG(Info, _T("%s - %s - %s"), strSection, strKey, strReturnValue);

		if (strReturnValue.IsEmpty())
			break;

		CUtil::ConvertStringToHex(strReturnValue, m_sEMV_Config_Data.m_sEMV_Config[nIndex].szAid);

		m_sEMV_Config_Data.m_sEMV_Config[nIndex].AidLen = strReturnValue.GetLength() / 2;

		if (m_sEMV_Config_Data.m_sEMV_Config[nIndex].AidUse == 1)
		{
			// AID가 enable인 것만 List에 등록
			memcpy(m_sAIDList.AidList[nLoadedCount].Aid, m_sEMV_Config_Data.m_sEMV_Config[nIndex].szAid, m_sEMV_Config_Data.m_sEMV_Config[nIndex].AidLen);	
			m_sAIDList.AidList[nLoadedCount].AidLen = m_sEMV_Config_Data.m_sEMV_Config[nIndex].AidLen;
			nLoadedCount++;
		}

		strKey = _T("AID_NAME");

		strReturnValue = iniEMV_AIDList.ReadString(strSection, strKey, _T(""));

		if (strReturnValue.IsEmpty())
			break;

		LOG(Info, _T("%s - %s - %s"), strSection, strKey, strReturnValue);

		WideCharToMultiByte(CP_ACP, 0, strReturnValue, -1, (char*)m_sEMV_Config_Data.m_sEMV_Config[nIndex].AidName, sizeof(m_sEMV_Config_Data.m_sEMV_Config[nIndex].AidName), NULL, NULL);

		nIndex++;
	}
	
	m_sEMV_Config_Data.nTotalAidListCount = nIndex;


	CString strAID;
	CString strEMVRecordData;

	for(int i=0; i<nIndex; i++)
	{
		strEMVRecordData.Empty();
		strAID = CUtil::ConvertHexToString(m_sEMV_Config_Data.m_sEMV_Config[i].szAid, m_sEMV_Config_Data.m_sEMV_Config[i].AidLen);

		// AID별로 Default 및 Version Loading
		if(strAID.GetLength() >= 10)
		{
			CString strAID10 = strAID.Left(10);

			if(      strAID10.CompareNoCase(_T("A000000003")) == 0)		// VISA, VISA ELECTRON, VISA PLUS
				strEMVRecordData.Format(_T("%s%s"), VISA_TAC_VER_VALUE, TERMINAL_EMV_CONFIG);
			else if( strAID10.CompareNoCase(_T("A000000098")) == 0)		// US VISA
				strEMVRecordData.Format(_T("%s%s"), US_VISA_TAC_VER_VALUE, TERMINAL_EMV_CONFIG);
			else if( strAID10.CompareNoCase(_T("A000000004")) == 0)		// MasterCard, MAESTRO, CIRRUS, US MAESTRO
				strEMVRecordData.Format(_T("%s%s"), MASTER_TAC_VER_VALUE, TERMINAL_EMV_CONFIG);
			else if( strAID10.CompareNoCase(_T("A000000065")) == 0)		// JCB
				strEMVRecordData.Format(_T("%s%s"), JCB_TAC_VER_VALUE, TERMINAL_EMV_CONFIG);
			else if( strAID10.CompareNoCase(_T("A000000333")) == 0)		// UnionPay Debit, Credit, Quiasi Credit, US UnionPay
				strEMVRecordData.Format(_T("%s%s"), UP_TAC_VER_VALUE, TERMINAL_EMV_CONFIG);
			else if( strAID10.CompareNoCase(_T("A000000152")) == 0)		// PULSE D-PAS(Discover), US Discover
				strEMVRecordData.Format(_T("%s%s"), D_PAS_DISCOVERY_TAC_VER_VALUE, TERMINAL_EMV_CONFIG);
			else if( strAID10.CompareNoCase(_T("A000000025")) == 0)		// AMEX
				strEMVRecordData.Format(_T("%s%s"), AMEX_TAC_VER_VALUE, TERMINAL_EMV_CONFIG);
			else if( strAID10.CompareNoCase(_T("A000000620")) == 0)		// US DNA
				strEMVRecordData.Format(_T("%s%s"), US_DNA_TAC_VER_VALUE, TERMINAL_EMV_CONFIG);
			else														
				strEMVRecordData.Format(_T("%s%s"), OTHER_TAC_VER_VALUE, TERMINAL_EMV_CONFIG);
		}
		else
		{
			strEMVRecordData.Format(_T("%s%s"), OTHER_TAC_VER_VALUE, TERMINAL_EMV_CONFIG);
		}

		CUtil::ConvertStringToHex(strEMVRecordData, m_sEMV_Config_Data.m_sEMV_Config[i].szRecord);
		m_sEMV_Config_Data.m_sEMV_Config[i].RecordLen = strEMVRecordData.GetLength() / 2;
	}

	m_sAIDList.AidCount = nLoadedCount;

	LOG(Info, _T("Loaded EMV Config - Loaded AID Count : %d, Active AID Count : %d"), nIndex, nLoadedCount);

}


/** *********************************************************
*	@brief	Save_AIDs_InformToFile : EMV Config Data 변경을 File에 저장하는 함수
*	@retval	없음
************************************************************/
void CDEV_CDR::Save_AIDs_InformToFile()
{
	CString		strEMVConfig;
	CString		strSection, strKey, strReturnValue;
	int			nLoadedCount = 0, nIndex = 0;

	//strEMVConfig.Format(_T("%s%s"), CUtil::GetAppPath(), EAGLE_ATM_EMV_CONFIG_PATH_NAME);
	strEMVConfig.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, EAGLE_ATM_EMV_CONFIG_PATH_NAME);

	CIniFile iniEMV_AIDList(strEMVConfig);

	for(int i=0; i<m_sEMV_Config_Data.nTotalAidListCount; i++)
	{
		strSection.Format(_T("AIDLIST_%02d"), i + 1);
		strKey = _T("AID_USE");

		if (m_sEMV_Config_Data.m_sEMV_Config[i].AidUse == 1)
			iniEMV_AIDList.WriteString(strSection, strKey, S_ENABLE);
		else
			iniEMV_AIDList.WriteString(strSection, strKey, S_DISABLE);
	}

	LOG(Info, _T("Save EMV Config - Use AID List"));
}


/** *********************************************************
*	@brief	Load_US_Common_AIDs_InformFromFile : Load US Common AIDs
*	@retval	없음
************************************************************/
void CDEV_CDR::Load_US_Common_AIDs_InformFromFile()
{
	CString		strSection, strKey, strReturnValue;
	CString		strEMV_US_Common_AID;
	int			nLoadedCount = 0;

	memset(&m_sEMV_US_Common_AID_List, 0, sizeof(m_sEMV_US_Common_AID_List));

	//strEMV_US_Common_AID.Format(_T("%s%s"), CUtil::GetAppPath(), EAGLE_ATM_EMV_US_COMM_AID_PATH_NAME);
	strEMV_US_Common_AID.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, EAGLE_ATM_EMV_US_COMM_AID_PATH_NAME);

	if (CUtil::IsExistFile(strEMV_US_Common_AID) == FALSE)
	{
		// Default Folder에서 File Copy하도록 로직 보완
		CString strDefaultPath = _T("");
		strDefaultPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, EAGLE_ATM_EMV_US_COMM_AID_PATH_NAME);
		CopyFile(strDefaultPath, strEMV_US_Common_AID, FALSE);
		CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
	}

	CIniFile	intEMV_US_COMMON_AIDLIST(strEMV_US_Common_AID);

	strSection = _T("US_COMMON_AID");
	strKey =	 _T("COMMON_AID_USE");
	strReturnValue = intEMV_US_COMMON_AIDLIST.ReadString(strSection, strKey, _T(""));

	if (strReturnValue.IsEmpty())
		return;

	if (strReturnValue == S_ENABLE)
		m_sEMV_US_Common_AID_List.US_Comm_AidUse = 1;
	else
		m_sEMV_US_Common_AID_List.US_Comm_AidUse = 0;

	LOG(Info, _T("%s - %s - %s"), strSection, strKey, strReturnValue);

	while(TRUE)
	{
		strSection.Format(_T("AIDLIST_%02d"), nLoadedCount + 1);
		strKey = _T("AID");

		strReturnValue = intEMV_US_COMMON_AIDLIST.ReadString(strSection, strKey, _T(""));

		if (strReturnValue.IsEmpty())
			break;

		CUtil::ConvertStringToHex(strReturnValue, m_sEMV_US_Common_AID_List.US_Comm_Aid[nLoadedCount].Aid);
		m_sEMV_US_Common_AID_List.US_Comm_Aid[nLoadedCount].AidLen = strReturnValue.GetLength() / 2;

		LOG(Info, _T("%s - %s - %s"), strSection, strKey, strReturnValue);

		nLoadedCount++;

	}

	m_sEMV_US_Common_AID_List.AidCount = nLoadedCount;
	LOG(Info, _T("US Common AID Count : (%d)"), nLoadedCount);

}


/** *********************************************************
*	@brief	Init_EMVKernel_Varialbe : EMV Kernel 및 EMV 관련 변수 초기화
*	@retval	없음
************************************************************/
void CDEV_CDR::Init_EMVKernel_Varialbe()
{
	LOG(Info, _T("CDEV_CDR::Init_EMVKernel_Varialbe() Start"));

	m_nCandidateList = 0;
	m_nSelectedAID = 0;
	m_bOnlineProcessing = FALSE;
	m_bUnknownAID = FALSE;
	m_bShowAIDList = FALSE;

	memset(&m_sIccAppInform, 0, sizeof(m_sIccAppInform));
}


/** *********************************************************
*	@brief	EMV_SelectApplication : Select Application 수행
*	@retval	EMV_RSLT_OK : 성공, EMV_ERR_CARD_SW or EMV_ERR_NOT_SUPPORT : FallBack, else : Terminate
************************************************************/
int	 CDEV_CDR::EMV_SelectApplication()
{
	int nResult = EMV_ERR_TERMINATE;

	LOG(Info, _T("EMV_SelectApplication Start"));

	Init_EMVKernel_Varialbe();

	if (EMVKernel_AppSelection == NULL)
	{
		LOG(Error, _T("EMVKernel_AppSelection func is null"));
		return EMV_ERR_TERMINATE;
	}

	nResult = EMVKernel_AppSelection(TRUE, &m_sAIDList);

	LOG(Info, _T("EMV_SelectApplication Result (%d)"), nResult);

	return nResult;
}


/** *********************************************************
*	@brief	EMV_MakeAppLists : Make Selected AppLists
*	@retval	EMV_RSLT_OK : 성공, else : Terminate
************************************************************/
int	 CDEV_CDR::EMV_MakeAppLists()
{
	int nResult = EMV_ERR_TERMINATE;

	LOG(Info, _T("EMV_MakeAppLists Start"));

	if (EMVKernel_MakeAppLists == NULL)
	{
		LOG(Error, _T("EMVKernel_MakeAppLists func is null"));
		return EMV_ERR_TERMINATE;
	}

	m_nCandidateList = 0;
	nResult = EMVKernel_MakeAppLists(TRUE, &m_sIccAppInform, &m_nCandidateList);

	LOG(Info, _T("EMV_MakeAppLists Result (%d) AID List Count (%d)"), nResult, m_nCandidateList);

	return nResult;
}


/** *********************************************************
*	@brief	EMV_RemoveAppList : 해당 AID를 List에서 삭제
*	@retval	없음
************************************************************/
void CDEV_CDR::EMV_RemoveAppList(int nAIDIndex)
{
	LOG(Info, _T("EMV_RemoveAppList Start - nAIDIndex(%d)"), nAIDIndex);

	if (EMVKernel_RemoveAppList == NULL)
	{
		LOG(Error, _T("EMVKernel_RemoveAppList func is null"));
		return;
	}

	m_nCandidateList = 0;
	EMVKernel_RemoveAppList(m_sIccAppInform.IccAppDesc[nAIDIndex].AidLen, m_sIccAppInform.IccAppDesc[nAIDIndex].AidName, &m_nCandidateList);

	LOG(Info, _T("EMV_RemoveAppList End"));
}


/** *********************************************************
*	@brief	EMV_RemoveAppList : 해당 AID를 List에서 삭제
*	@retval	없음
************************************************************/
void CDEV_CDR::EMV_Set_SelectAID_Index(int nIndex)
{
	LOG(Info, _T("EMV_Set_SelectAID_Index (%d)"), nIndex);
	m_nSelectedAID = nIndex;
}


/** *********************************************************
*	@brief	EMV_FinalApplication : 거래할 Final select Application 수행
*	@retval	EMV_RSLT_OK : 성공
*	@retval	EMV_ERR_NOT_ACCEPT : AppCnt > 1인 경우 RemoveAppList 호출 후 AppCount가 있을 경우 MakeAppList() 호출, 없을 경우 Terminate
*	@retval	else : Terminate
************************************************************/
int	 CDEV_CDR::EMV_FinalApplication()
{
	int nResult = EMV_ERR_TERMINATE;

	BYTE	SW[4] = { 0, };

	LOG(Info, _T("EMV_FinalApplication Start"));

	if (EMVKernel_FinalAppSelection == NULL)
	{
		LOG(Error, _T("EMVKernel_FinalAppSelection func is null"));
		return EMV_ERR_TERMINATE;
	}

	m_nCandidateList = 0;
	nResult = EMVKernel_FinalAppSelection(m_sIccAppInform.IccAppDesc[m_nSelectedAID].AidLen, m_sIccAppInform.IccAppDesc[m_nSelectedAID].AidName, &m_nCandidateList, SW);

	LOG(Info, _T("EMV_FinalApplication Result (%d)"), nResult);

	return nResult;
}


/** *********************************************************
*	@brief	EMV_StoreValFromRecord : 거래할 AID의 Config Data를 커널에 전달함
*	@retval	1 : 성공, 0 : 실패
************************************************************/
int  CDEV_CDR::EMV_StoreValFromRecord()
{
	int nResult = 0, nLen = 0;
	unsigned char	szValue[1024] = { 0, };
	CString			strTemp;
	int				nSelectedIndex = 0;

	LOG(Info, _T("EMV_StoreValFromRecord Start"));

	if (EMVKernel_StoreValFromRecord == NULL)
	{
		LOG(Error, _T("EMVKernel_StoreValFromRecord func is null"));
		return 0;
	}

	// Compare AID and search EMV Data Index
	for(int i=0; i< m_sEMV_Config_Data.nTotalAidListCount; i++)
	{
		if (memcmp(m_sEMV_Config_Data.m_sEMV_Config[i].szAid, m_sIccAppInform.IccAppDesc[m_nSelectedAID].AidName, m_sIccAppInform.IccAppDesc[m_nSelectedAID].AidLen) == 0)
		{
			LOG(Info, _T("[EMV] Match Index ( %d)"), i);
			nSelectedIndex = i;
			break;
		}
	}


	nResult = EMVKernel_StoreValFromRecord(m_sEMV_Config_Data.m_sEMV_Config[nSelectedIndex].szRecord, 0, m_sEMV_Config_Data.m_sEMV_Config[nSelectedIndex].RecordLen, 0);

	//strTemp = CUtil::ConvertHexToString(m_sEMV_Config_Data.m_sEMV_Config[nSelectedIndex].szRecord, m_sEMV_Config_Data.m_sEMV_Config[nSelectedIndex].RecordLen);
	LOG(Info, _T("[EMV] Default Value Len : %d"), m_sEMV_Config_Data.m_sEMV_Config[nSelectedIndex].RecordLen);

	// Set other transaction values
	// 9A, 9F21 Tag
	{
		SYSTEMTIME localTime;
		::GetLocalTime(&localTime);

		memset(szValue, 0x0, sizeof(szValue));

		strTemp.Format(_T("%02d%02d%02d"), (localTime.wYear - 2000), localTime.wMonth, localTime.wDay);
		CUtil::ConvertStringToHex(strTemp, szValue);
		nLen = strTemp.GetLength() / 2;
		LOG(Info, _T("STORE Transaction Date : [%S%02X%s]"), EMVTag_TrDate, nLen, strTemp);
		EMV_TLV_StoreValue(EMVTag_TrDate, nLen, szValue);

		memset(szValue, 0x0, sizeof(szValue));
		strTemp.Format(_T("%02d%02d%02d"), localTime.wHour, localTime.wMinute, localTime.wSecond);
		CUtil::ConvertStringToHex(strTemp, szValue);
		nLen = strTemp.GetLength() / 2;
		LOG(Info, _T("STORE Transaction Time: [%S%02X%s]"), EMVTag_TrTime, nLen, strTemp);
		EMV_TLV_StoreValue(EMVTag_TrTime, nLen, szValue);
	}

	// 9F1C
	{
		memset(szValue, 0x0, sizeof(szValue));
		strTemp.Format(_T("%8.8s"), CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
		nLen = strTemp.GetLength();
		LOG(Info, _T("STORE Terminal ID: [%S%02X%s]"), EMVTag_TerminalId, nLen, strTemp);
		EMV_TLV_StoreValue(EMVTag_TerminalId, nLen, szValue);
	}

	// 9F41
	{
		memset(szValue, 0x0, sizeof(szValue));
		int		nSequenceNum = (CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_System.strSequence_Number) % 9999) + 1;
		strTemp.Format(_T("%08d"), nSequenceNum);
		CUtil::ConvertStringToHex(strTemp, szValue);
		nLen = strTemp.GetLength() / 2;
		LOG(Info, _T("STORE Sequence No: [%S%02X%s]"), EMVTag_TrSeqCnt, nLen, strTemp);
		EMV_TLV_StoreValue(EMVTag_TrSeqCnt, nLen, szValue);
	}

	// 9F1E
	{
		memset(szValue, 0x0, sizeof(szValue));

		strTemp.Format(_T("%-8.8s"), CEagleDataManager::GetInstance()->m_Config.m_Option.strMachine_Serial_Number);
		// Serial Number에 문자열 '-'는 0으로 변경
		strTemp.Replace(_T("-"), _T("0"));

		sprintf((char*)szValue, "%S", strTemp);
		nLen = 0x08;
		LOG(Info, _T("STORE Serial No: [%S%02X%s]"), EMVTag_IFDSerialNo, nLen, strTemp);
		EMV_TLV_StoreValue(EMVTag_IFDSerialNo, nLen, szValue);
	}

	LOG(Info, _T("EMV_StoreValFromRecord Result (%d)"), nResult);

	return nResult;
}


/** *********************************************************
*	@brief	EMV_InitApplication : Get Processing Option 수행
*	@retval	EMV_RSLT_OK : 성공
*	@retval	EMV_ERR_NOT_ACCEPT : AppCnt > 1인 경우 RemoveAppList 호출 후 AppCount가 있을 경우 MakeAppList() 호출, 없을 경우 Terminate
*	@retval	else : Terminate
************************************************************/
int  CDEV_CDR::EMV_InitApplication()
{
	int nResult = EMV_ERR_TERMINATE;

	LOG(Info, _T("EMV_InitApplication Start"));

	if (EMVKernel_InitApplication == NULL)
	{
		LOG(Error, _T("EMVKernel_InitApplication func is null"));
		return EMV_ERR_TERMINATE;
	}

	m_nCandidateList = 0;
	nResult = EMVKernel_InitApplication(&m_nCandidateList);

	LOG(Info, _T("EMV_InitApplication Result (%d) Count (%d)"), nResult, m_nCandidateList);

	return nResult;
}

/** *********************************************************
*	@brief	EMV_ReadAppData : Get Processing Option 수행
*	@retval	EMV_RSLT_OK : 성공
*	@retval	EMV_ERR_CARD_SW or EMV_ERR_NOT_SUPPORT : FallBack
*	@retval	else : Terminate
************************************************************/
int  CDEV_CDR::EMV_ReadAppData()
{
	int nResult = EMV_ERR_TERMINATE;

	LOG(Info, _T("EMV_ReadAppData Start"));

	if (EMVKernel_ReadAppData == NULL)
	{
		LOG(Error, _T("EMVKernel_ReadAppData func is null"));
		return EMV_ERR_TERMINATE;
	}

	nResult = EMVKernel_ReadAppData();

	LOG(Info, _T("EMV_ReadAppData Result (%d)"), nResult);

	return nResult;
}


/** *********************************************************
*	@brief	EMV_StoreTransValues : 거래 정보를 Kernel에 설정
*	@retval	없음
************************************************************/
void CDEV_CDR::EMV_StoreTransValues()
{
	int nLen = 0;
	unsigned char szValue[128];
	CString strTemp, strTransType, strAccountType;

	// 1. set Transaction Type
	strTransType = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType;
	strAccountType = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strFromAccountType;

	memset(szValue, 0, sizeof(szValue));

	if (strTransType == S_WITHDRAWAL)
		szValue[0] = (unsigned char)TRTYPE_CASH;
	else if (strTransType == S_BALANCEINQUIRY)
		szValue[0] = (unsigned char)TRTYPE_INQUIRY;
	else if (strTransType == S_TRANSFER)
		szValue[0] = (unsigned char)TRTYPE_TRANSFER;
	else
		szValue[0] = (unsigned char)TRTYPE_CASH;

	nLen = 0x01;
	EMV_TLV_StoreValue(EMVTag_TrType, nLen, szValue);
	LOG(Info, _T("[EMV] SET TRANSACTION TYPE (%s)"), strTransType);

	// 2. set Amount
	memset(szValue, 0x0, sizeof(szValue));

	strTemp.Format(_T("%12s%s%s"), ZERO_12, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strRequestAmount, _T("00"));	// Cent 포함
	CUtil::ConvertStringToHex(strTemp.Right(12), szValue);
	nLen = 0x06;
	EMV_TLV_StoreValue(EMVTag_AmountAuthN, nLen, szValue);
	LOG(Info, _T("[EMV] SET AMOUNT (%s)"), strTemp.Right(12));

	// 3. set Account Type
	memset(szValue, 0x0, sizeof(szValue));

	if (strAccountType == S_CHECKING)
		szValue[0] = 0x20;
	else if (strAccountType == S_SAVINGS)
		szValue[0] = 0x10;
	else if (strAccountType == S_CREDIT)
		szValue[0] = 0x30;

	nLen = 0x01;
	EMV_TLV_StoreValue(EMVTag_AccountType, nLen, szValue);
	LOG(Info, _T("[EMV] SET ACCOUNT (%s)"), strAccountType);		
}


/** *********************************************************
*	@brief	EMV_OfflineDataAuth : Offline Authentication 수행
*	@retval	EMV_RSLT_OK : 성공
*	@retval	EMV_ERR_CARD_SW or EMV_ERR_NOT_SUPPORT : FallBack
*	@retval	else : Terminate
************************************************************/
int  CDEV_CDR::EMV_OfflineDataAuth()
{
	int nResult = EMV_ERR_TERMINATE;

	LOG(Info, _T("EMV_OfflineDataAuth Start"));

	if (EMVKernel_OfflineDataAuth == NULL)
	{
		LOG(Error, _T("EMVKernel_OfflineDataAuth func is null"));
		return EMV_ERR_TERMINATE;
	}

	nResult = EMVKernel_OfflineDataAuth();

	LOG(Info, _T("EMV_OfflineDataAuth Result (%d)"), nResult);

	return nResult;
}


/** *********************************************************
*	@brief	EMV_ProcessRestrict : Process Restrict 수행
*	@retval	EMV_RSLT_OK : 성공
*	@retval	EMV_ERR_CARD_SW or EMV_ERR_NOT_SUPPORT : FallBack
*	@retval	else : Terminate
************************************************************/
int  CDEV_CDR::EMV_ProcessRestrict()
{
	int nResult = EMV_ERR_TERMINATE;

	LOG(Info, _T("EMV_ProcessRestrict Start"));

	if (EMVKernel_ProcessRestrict == NULL)
	{
		LOG(Error, _T("EMVKernel_ProcessRestrict func is null"));
		return EMV_ERR_TERMINATE;
	}

	nResult = EMVKernel_ProcessRestrict();

	LOG(Info, _T("EMV_ProcessRestrict Result (%d)"), nResult);

	return nResult;
}


/** *********************************************************
*	@brief	EMV_CardholderVerify : 카드 소지자 검증 처리
*	@retval	EMV_RSLT_OK : 성공
*	@retval	EMV_ERR_CARD_SW or EMV_ERR_NOT_SUPPORT : FallBack
*	@retval	else : Terminate
************************************************************/
int  CDEV_CDR::EMV_CardholderVerify()
{
	int nResult = EMV_ERR_TERMINATE;

	unsigned char szTemp[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };	// Security를 위해 Kernel에는 모두 0xFF로 전달

	LOG(Info, _T("EMV_CardholderVerify Start"));

	if (EMVKernel_CardholderVerify == NULL)
	{
		LOG(Error, _T("EMVKernel_CardholderVerify func is null"));
		return EMV_ERR_TERMINATE;
	}

	nResult = EMVKernel_CardholderVerify(8, szTemp);

	LOG(Info, _T("EMV_CardholderVerify Result (%d)"), nResult);

	return nResult;
}


/** *********************************************************
*	@brief	EMV_TerminalRiskMgmt : Terminal Risk Management 수행
*	@retval	EMV_RSLT_OK : 성공
*	@retval	EMV_ERR_CARD_SW or EMV_ERR_NOT_SUPPORT : FallBack
*	@retval	else : Terminate
************************************************************/
int  CDEV_CDR::EMV_TerminalRiskMgmt()
{
	int nResult = EMV_ERR_TERMINATE;

	LOG(Info, _T("EMV_TerminalRiskMgmt Start"));

	if (EMVKernel_TerminalRiskMgmt == NULL)
	{
		LOG(Error, _T("EMVKernel_TerminalRiskMgmt func is null"));
		return EMV_ERR_TERMINATE;
	}

	nResult = EMVKernel_TerminalRiskMgmt(0x00);		// Force Online 미지원이므로 0x00으로 처리

	LOG(Info, _T("EMV_TerminalRiskMgmt Result (%d)"), nResult);

	return nResult;
}


/** *********************************************************
*	@brief	EMV_TerminalActionAnalysis : Terminal Action Analysis 수행 (1st Generate AC 수행)
*	@retval	0x40 : TC, 0x80 : ARQC, 0x00 : AAC, 0xC0 : AAR(미사용)
************************************************************/
BYTE  CDEV_CDR::EMV_TerminalActionAnalysis()
{
	BYTE byResult = 0x00;

	LOG(Info, _T("EMV_TerminalActionAnalysis Start"));

	if (EMVKernel_TerminalActionAnalysis == NULL)
	{
		LOG(Error, _T("EMVKernel_TerminalActionAnalysis func is null"));
		return EMV_ERR_TERMINATE;
	}

	byResult = EMVKernel_TerminalActionAnalysis();

	LOG(Info, _T("EMV_TerminalActionAnalysis Result (0x%x)"), byResult);

	return byResult;
}


/** *********************************************************
*	@brief	EMV_CardActionAnalysis : ICC내의 자체 위험 관리 수행
*	@retval	OnlineFlag : 0x8F(ONLINE), 0x4F(OFFLINE)
*	@retval	EMV_RSLT_OK : 성공
*	@retval	else : Completion() 수행 후 Terminate
************************************************************/
int  CDEV_CDR::EMV_CardActionAnalysis(unsigned char *OnlineFlag)
{
	int nResult = EMV_ERR_TERMINATE;

	LOG(Info, _T("EMV_CardActionAnalysis Start"));

	if (EMVKernel_CardActionAnalysis == NULL)
	{
		LOG(Error, _T("EMVKernel_CardActionAnalysis func is null"));
		return EMV_ERR_TERMINATE;
	}

	nResult = EMVKernel_CardActionAnalysis(OnlineFlag);

	LOG(Info, _T("EMV_CardActionAnalysis Result (%d)"), nResult);

	return nResult;
}


/** *********************************************************
*	@brief	EMV_OnlineProcess : Host에서 수신받은 Response값을 바탕으로 2nd Generate AC 수행
*	@retval	EMV_RSLT_OK : Completion 수행
*	@retval	else : Terminate
************************************************************/
int  CDEV_CDR::EMV_OnlineProcess(unsigned char Acquirer_CID)
{
	int nResult = EMV_ERR_TERMINATE;

	LOG(Info, _T("EMV_OnlineProcess Start Acquirer_CID (0x%02x)"), Acquirer_CID);

	if (EMVKernel_OnlineProcess == NULL)
	{
		LOG(Error, _T("EMVKernel_OnlineProcess func is null"));
		return EMV_ERR_TERMINATE;
	}

	nResult = EMVKernel_OnlineProcess(Acquirer_CID);

	LOG(Info, _T("EMV_OnlineProcess Result (%d)"), nResult);

	return nResult;
}

/** *********************************************************
*	@brief	EMV_Completion : 최종 승인 여부 확인
*	@retval	6 : APPROVED, 7 : DECLINED
************************************************************/
int  CDEV_CDR::EMV_Completion()
{
	int nResult = EMV_ERR_TERMINATE;

	LOG(Info, _T("EMV_Completion Start"));

	if (EMVKernel_Completion == NULL)
	{
		LOG(Error, _T("EMVKernel_Completion func is null"));
		return EMV_ERR_TERMINATE;
	}

	nResult = EMVKernel_Completion();

	LOG(Info, _T("EMV_Completion Result (%d)"), nResult);

	return nResult;
}


/** *********************************************************
*	@brief	EMV_TLV_GetValue : ICC의 Tag값을 얻어오는 함수
*	@retval	6 : APPROVED, 7 : DECLINED
************************************************************/
int  CDEV_CDR::EMV_TLV_GetValue(unsigned char *tag, int *len, unsigned char *val)
{
	int nResult = 0;

	if (EMVKernel_Tlv_GetVal == NULL)
	{
		LOG(Error, _T("EMVKernel_Tlv_GetVal func is null"));
		return 0;
	}

	nResult = EMVKernel_Tlv_GetVal(tag, len, val);

	return nResult;

}


/** *********************************************************
*	@brief	EMV_TLV_StoreValue : ICC의 Tag값을 Store하는 함수
*	@retval	6 : APPROVED, 7 : DECLINED
************************************************************/
int  CDEV_CDR::EMV_TLV_StoreValue(unsigned char *tag, int len, unsigned char*val)
{
	int nResult = 0;

	if (EMVKernel_Tlv_StoreVal == NULL)
	{
		LOG(Error, _T("EMVKernel_Tlv_StoreVal func is null"));
		return 0;
	}

	nResult = EMVKernel_Tlv_StoreVal(tag, len, val);

	return nResult;
}


/** *********************************************************
*	@brief	Get_BrandNameForADA : AID에 Matching되는 BrandName wave file을 가져오는 함수
*	@retval	Match : 해당 Brand Name wave file명 / 실패 : Unknown.wav file명
************************************************************/
CString  CDEV_CDR::Get_BrandNameForADA(int nIndex)
{
	CString strAID, strCompareAID, strTemp, strWaveFileName;

	strAID = CUtil::ConvertHexToString(m_sIccAppInform.IccAppDesc[nIndex].AidName, m_sIccAppInform.IccAppDesc[nIndex].AidLen);
	strAID.TrimRight();


	strCompareAID = VISA_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_VISA_WAVE_FILE;

	strCompareAID = VISAELECTRONIC_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_VISAELECTRON_WAVE_FILE;

	strCompareAID = VISAPLUS_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_VISAPLUS_WAVE_FILE;

	strCompareAID = VISAPLUS_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
	if (strAID.CompareNoCase(USVISA_AID) == 0)
		return BRANDNAME_USVISA_WAVE_FILE;

	strCompareAID = MASTERCARD_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_MASTERCARD_WAVE_FILE;

	strCompareAID = MAESTRO_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_MAESTRO_WAVE_FILE;

	strCompareAID = CIRRUS_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_CIRRUS_WAVE_FILE;

	strCompareAID = USMAESTRO_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_USMAESTRO_WAVE_FILE;

	strCompareAID = DPAS_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_PLUSEDPAS_WAVE_FILE;

	strCompareAID = DISCOVER_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_USDISCOVER_WAVE_FILE;

	strCompareAID = AMEX_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_AMEX_WAVE_FILE;

	strCompareAID = JCB_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_JCB_WAVE_FILE;

	strCompareAID = UPDEBIT_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_UPDEBIT_WAVE_FILE;

	strCompareAID = UPCREDIT_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_UPCREDIT_WAVE_FILE;

	strCompareAID = USUP_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_USUP_WAVE_FILE;

	strCompareAID = DNA_AID;
	if (strCompareAID.CompareNoCase(strAID.Left(strCompareAID.GetLength())) == 0)
		return BRANDNAME_USDNA_WAVE_FILE;

	return BRANDNAME_UNKNOWN_WAVE_FILE;
}


/** *********************************************************
*	@brief		[PCI-SSF Fix] CryptoAPI(CryptGenRandom) ??? CSPRNG?? ?????? ???? ??????? ???????.
*				???? srand(GetTickCount())+rand()?? ?????????/???? ?????? ?????
*				???? ????(Magnetic/APDU/Key Exchange Key ??) ?????????? ?????????.
*	@retval		TRUE: ????, FALSE: ????
************************************************************/
BOOL CDEV_CDR::GenerateSecureRandomBytes(BYTE *pbyOutput, DWORD dwLen)
{
	HCRYPTPROV hProv = NULL;
	BOOL bResult = FALSE;

	if (pbyOutput == NULL || dwLen == 0)
		return FALSE;

	if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		bResult = CryptGenRandom(hProv, dwLen, pbyOutput);
		CryptReleaseContext(hProv, 0);
	}

	if (!bResult)
	{
		LOG(Error, _T("GenerateSecureRandomBytes() CryptAcquireContext/CryptGenRandom is failed"));
	}

	return bResult;
}

/** *********************************************************
*	@brief		처음 암호화 Section 연결시, 필요한 key 정보 및 검증 data 생성
*	@retval		0: 실패, 1: 성공
************************************************************/
void CDEV_CDR::GenerateKeyInfoAndAuthData()
{
//	BYTE byTemp[ENCRYPTION_KEY_LENGTH] = { 0x0D, 0x34, 0x31, 0xCB, 0x07, 0xF1, 0xA4, 0x92, 0x3B, 0xFE, 0xAD, 0x57, 0x3D, 0x5E, 0x01, 0x0E };
	int i = 0, j = 0;
	BYTE byXOR = 0;
	BYTE byTempKey = 0;
	// [PCI-SSF Fix] srand(GetTickCount())+rand() ??? CryptGenRandom ??? CSPRNG ???
	BYTE byRandomBuf[ENCRYPTION_KEY_LENGTH * 2] = { 0, };	// [0,KEY_LEN) : key exchange key, [KEY_LEN,KEY_LEN*2) : auth data

	GenerateSecureRandomBytes(byRandomBuf, sizeof(byRandomBuf));

	// 1. Key exchange key 생성
	for(i=0; i<ENCRYPTION_KEY_LENGTH; i++)
	{
		byXOR = 0;

		byTempKey = byRandomBuf[i];

		for(j=0; j<8; j++)
			byXOR = byXOR ^ ((byTempKey >> j) & 0x01);

		if(0x00 == byXOR)
			byTempKey ^= 0x01;

		m_byKeyExchangeKey[i] = byTempKey;
	}
	///////////////////////////////////////////////////////////

	// 2. Authentication data 생성
	for (i = 0; i < ENCRYPTION_KEY_LENGTH; i++)
	{
		m_byDeviceAuthData[i] = byRandomBuf[ENCRYPTION_KEY_LENGTH + i];
	}
	///////////////////////////////////////////////////////////
}

/** *********************************************************
*	@brief	암호화 section 연결\n
			MS data 및 IC data의 암/복호화에 사용되는 키를\n
			교환할 때 사용되는 키 교환			
*	@retval		0: 실패, 1: 성공
************************************************************/
BOOL CDEV_CDR::GenerateKeyofDeviceAttestationDataAndKey()
{
#if	EMULATION_CDR_DEVICE
	return CDevSim::Confirm(_T("CDR"), _T("CDR_GenerateAttestationKey"));
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;

	COMMAND command;

//	BYTE byDeviceAuthData[ENCRYPTION_KEY_LENGTH] = { 0x44, 0x9A, 0x81, 0xCC, 0xE1, 0xF2, 0x3A, 0xBC, 0x03, 0x4E, 0x27, 0x2E, 0x11, 0xE9, 0xCA, 0x6D };
	BYTE byMasterKey[ENCRYPTION_KEY_LENGTH] = { 0xCB, 0x86, 0x76, 0xD6, 0x9E, 0xF7, 0x92, 0xC2, 0xB3, 0xCD, 0xCD, 0x0D, 0x8C, 0x4C, 0x9D, 0x0D };
	BYTE byMixData[ENCRYPTION_KEY_LENGTH*2] = { 0, };
	BYTE bySendData[ENCRYPTION_KEY_LENGTH*2] = { 0, };
	BOOL bSame = TRUE;
	int i = 0;
	int nSendDataLen = 0;

	for (i = 0; i < ENCRYPTION_KEY_LENGTH; i++)
	{
		byMixData[i*2] = m_byDeviceAuthData[i];
		byMixData[i*2 + 1] = m_byKeyExchangeKey[i];
	}

	EncryptKeyData_TDES_ECB(byMixData, ENCRYPTION_KEY_LENGTH*2, bySendData, nSendDataLen, byMasterKey);	// T-DES 암호화

	command.bCommandCode = SANKYO_CDR_CMD_KEY_GEN;

	command.bParameterCode = SANKYO_CDR_PM_KEY_GEN_DEVICE;

	command.Data.lpbBody = bySendData;

	command.Data.dwSize = nSendDataLen;


	REPLY	reply;
	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &command, m_nTimeout, &reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == reply.replyType);

	if (NegativeReply == reply.replyType)
	{
		LOG(Error, _T("GenerateKeyofDeviceAttestationDataAndKey is failed. ErrorCode - [%c %c]"), reply.message.negativeReply.ErrorCode.bE1, reply.message.negativeReply.ErrorCode.bE0);
		m_strLastError = CDR_CERTIFICATION_ERROR;	// 암호화 key를 전달하는 명령이 실패하게 되면, 어짜피 MS/IC data Read를 못하기 때문에 암호화 장애로 처리한다.
		m_bKeyAuthError = TRUE;

#if CDR_ENCRYPT_DEBUG
		CString strTempErrorCode;
		CString strTemp, strTemp2;

		strTempErrorCode.Format(_T("C000%c%c"), reply.message.negativeReply.ErrorCode.bE1, reply.message.negativeReply.ErrorCode.bE0);

		if(WEAK_ENCRYPT_KEY == strTempErrorCode)
		{
			strTemp.Empty();
			strTemp2.Empty();
			for (i = 0; i < ENCRYPTION_KEY_LENGTH; i++)
			{
				strTemp.Format(_T("%02X "), m_byKeyExchangeKey[i]);
				strTemp2 += strTemp;
			}
			LOG(Error, _T("Key Exchange Key is weak. [%s]"), strTemp2);
		}
#endif
	}

	if (bResult)
	{
		DWORD dwRecvDataSize = reply.message.positiveReply.Data.dwSize;

		if (dwRecvDataSize)
		{
			BYTE* pRecvData = new BYTE[dwRecvDataSize + 1];
			memset(pRecvData, 0x00, dwRecvDataSize + 1);

			DecryptKeyData_TDES_ECB(reply.message.positiveReply.Data.bBody, dwRecvDataSize, pRecvData, m_byKeyExchangeKey);
			pRecvData[dwRecvDataSize] = 0x00;

			for (i = 0; i < (int)dwRecvDataSize; i++)
			{
				if (m_byDeviceAuthData[i] != pRecvData[i])
				{
					bSame = FALSE;
				}

				if (FALSE == bSame)
				{
					LOG(Error, _T("AuthData is mismatch!!!"));
					m_strLastError = CDR_CERTIFICATION_ERROR;

					m_bKeyAuthError = TRUE;
					bResult = FALSE;
					break;
				}
			}

			delete[] pRecvData;
		}
	}

	m_CDRLock.Unlock();

	return bResult;
}


/** *********************************************************
*	@brief		MS Data 암/복호화 키 교환
*	@retval		0: 실패, 1: 성공
************************************************************/
BOOL CDEV_CDR::GenerateKey4MagneticData()
{
#if	EMULATION_CDR_DEVICE
	return CDevSim::Confirm(_T("CDR"), _T("CDR_GenerateKey4MagneticData"));
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;

	COMMAND command;

	BYTE bySendData[GENERATED_KEY_DATA_LENGTH] = { 0, };
	BYTE byMagKey[ENCRYPTION_KEY_LENGTH] = { 0, };
	BYTE byIV[ENCRYPTION_IV_LENGTH] = { 0, };
	BYTE byTempKey = 0;
	BYTE byXOR = 0;
	int i = 0, j = 0;
	int nSendDataLen = 0;

	// [PCI-SSF Fix] srand(GetTickCount())+rand() ??? CryptGenRandom ??? CSPRNG ???
	{
		BYTE byRandomBuf[ENCRYPTION_IV_LENGTH + ENCRYPTION_KEY_LENGTH] = { 0, };

		GenerateSecureRandomBytes(byRandomBuf, sizeof(byRandomBuf));

		for (i = 0; i < ENCRYPTION_IV_LENGTH; i++)
		{
			byIV[i] = byRandomBuf[i];
		}

		for (i = 0; i<ENCRYPTION_KEY_LENGTH; i++)
		{
			byXOR = 0;

			byTempKey = byRandomBuf[ENCRYPTION_IV_LENGTH + i];

			// MAGKEY ???? odd parity???? ??.
			for (j = 0; j<8; j++)
				byXOR = byXOR ^ ((byTempKey >> j) & 0x01);

			if (0x00 == byXOR)
				byTempKey ^= 0x01;

			byMagKey[i] = byTempKey;
		}
	}

	memcpy(m_byMagkeyData, byMagKey, ENCRYPTION_KEY_LENGTH);
	memcpy(&m_byMagkeyData[ENCRYPTION_KEY_LENGTH], byIV, ENCRYPTION_IV_LENGTH);

	EncryptKeyData_TDES_ECB(m_byMagkeyData, GENERATED_KEY_DATA_LENGTH, bySendData, nSendDataLen, m_byKeyExchangeKey);	// T-DES 암호화

	command.bCommandCode = SANKYO_CDR_CMD_KEY_GEN;

	command.bParameterCode = SANKYO_CDR_PM_KEY_GEN_MAGNETIC;

	command.Data.lpbBody = bySendData;

	command.Data.dwSize = nSendDataLen;


	REPLY	reply;
	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &command, m_nTimeout, &reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == reply.replyType);

	if (NegativeReply == reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), reply.message.negativeReply.ErrorCode.bE1, reply.message.negativeReply.ErrorCode.bE0);
		LOG(Error, _T("GenerateKey4MagneticData is failed. ErrorCode - [%c %c]"), reply.message.negativeReply.ErrorCode.bE1, reply.message.negativeReply.ErrorCode.bE0);

		if(NO_ENCRYPT_KEY == m_strLastError)	// 암호화 키가 존재하지 않을 경우, 인증 장애로 처리한다.
		{
			m_strLastError = CDR_CERTIFICATION_ERROR;
			m_bKeyAuthError = TRUE;
		}

#if CDR_ENCRYPT_DEBUG
		CString strTempErrorCode;
		CString strTemp, strTemp2;

		strTempErrorCode.Format(_T("C000%c%c"), reply.message.negativeReply.ErrorCode.bE1, reply.message.negativeReply.ErrorCode.bE0);

		if(WEAK_ENCRYPT_KEY == strTempErrorCode)
		{
			strTemp.Empty();
			strTemp2.Empty();
			for (i = 0; i < ENCRYPTION_KEY_LENGTH; i++)
			{
				strTemp.Format(_T("%02X "), byMagKey[i]);
				strTemp2 += strTemp;
			}
			LOG(Error, _T("Magnetic Key is weak. [%s]"), strTemp2);
		}
#endif
	}

	m_CDRLock.Unlock();


	return bResult;
}


/** *********************************************************
*	@brief		IC Data 암/복호화 키 교환
*	@retval		0: 실패, 1: 성공
************************************************************/
BOOL CDEV_CDR::GenerateKey4APDUData()
{
#if	EMULATION_CDR_DEVICE
	return CDevSim::Confirm(_T("CDR"), _T("CDR_GenerateKey4APDUData"));
#endif

	if (NULL == DLL_ExecuteCommand)
	{
		return FALSE;
	}

	m_CDRLock.Lock();

	BOOL bResult = FALSE;

	COMMAND command;

	BYTE bySendData[GENERATED_KEY_DATA_LENGTH] = { 0, };
	BYTE byAPDUKey[ENCRYPTION_KEY_LENGTH] = { 0, };
	BYTE byIV[ENCRYPTION_IV_LENGTH] = { 0, };
	BYTE byTempKey = 0;
	BYTE byXOR = 0;
	int i = 0, j = 0;
	int nSendDataLen = 0;

	// [PCI-SSF Fix] srand(GetTickCount())+rand() ??? CryptGenRandom ??? CSPRNG ???
	{
		BYTE byRandomBuf[ENCRYPTION_IV_LENGTH + ENCRYPTION_KEY_LENGTH] = { 0, };

		GenerateSecureRandomBytes(byRandomBuf, sizeof(byRandomBuf));

		for (i = 0; i < ENCRYPTION_IV_LENGTH; i++)
		{
			byIV[i] = byRandomBuf[i];
		}

		for (i = 0; i<ENCRYPTION_KEY_LENGTH; i++)
		{
			byXOR = 0;

			byTempKey = byRandomBuf[ENCRYPTION_IV_LENGTH + i];

			// APDUKEY ???? odd parity???? ??.
			for (j = 0; j<8; j++)
				byXOR = byXOR ^ ((byTempKey >> j) & 0x01);

			if (0x00 == byXOR)
				byTempKey ^= 0x01;

			byAPDUKey[i] = byTempKey;
		}
	}

	memcpy(m_byAPDUkeyData, byAPDUKey, ENCRYPTION_KEY_LENGTH);
	memcpy(&m_byAPDUkeyData[ENCRYPTION_KEY_LENGTH], byIV, ENCRYPTION_IV_LENGTH);

	EncryptKeyData_TDES_ECB(m_byAPDUkeyData, GENERATED_KEY_DATA_LENGTH, bySendData, nSendDataLen, m_byKeyExchangeKey);	// T-DES 암호화

	command.bCommandCode = SANKYO_CDR_CMD_KEY_GEN;

	command.bParameterCode = SANKYO_CDR_PM_KEY_GEN_APDU;

	command.Data.lpbBody = bySendData;

	command.Data.dwSize = nSendDataLen;

	REPLY	reply;
	DWORD dwRet = 0;

	dwRet = DLL_ExecuteCommand(m_szPortNum, &command, m_nTimeout, &reply);

	if (EAGLE_CDR_NO_ERROR != dwRet)
	{
		SetCommError(dwRet);
		m_CDRLock.Unlock();

		return FALSE;
	}

	bResult = (PositiveReply == reply.replyType);

	if (NegativeReply == reply.replyType)
	{
		m_strLastError.Format(_T("C000%c%c"), reply.message.negativeReply.ErrorCode.bE1, reply.message.negativeReply.ErrorCode.bE0);
		LOG(Error, _T("GenerateKey4APDUData is failed. ErrorCode - [%c %c]"), reply.message.negativeReply.ErrorCode.bE1, reply.message.negativeReply.ErrorCode.bE0);
		
		if(NO_ENCRYPT_KEY == m_strLastError)	// 암호화 키가 존재하지 않을 경우, 인증 장애로 처리한다.
		{
			m_strLastError = CDR_CERTIFICATION_ERROR;
			m_bKeyAuthError = TRUE;
		}

#if CDR_ENCRYPT_DEBUG
		CString strTempErrorCode;
		CString strTemp, strTemp2;

		strTempErrorCode.Format(_T("C000%c%c"), reply.message.negativeReply.ErrorCode.bE1, reply.message.negativeReply.ErrorCode.bE0);

		if(WEAK_ENCRYPT_KEY == strTempErrorCode)
		{
			strTemp.Empty();
			strTemp2.Empty();
			for (i = 0; i < ENCRYPTION_KEY_LENGTH; i++)
			{
				strTemp.Format(_T("%02X "), byAPDUKey[i]);
				strTemp2 += strTemp;
			}
			LOG(Error, _T("APDU Key is weak. [%s]"), strTemp2);
		}
#endif
	}

	m_CDRLock.Unlock();

	return bResult;
}


/** *********************************************************
*	@brief		T-DES 암호화(ECB 모드)
*	@param		BYTE *pbyInput	Plain Text
*	@param		int nInputLen	InputLen
*	@param		BYTE *pbyOutput	Encrypted Text
*	@param		int &nOutputLen	Encrypted Text Length
*	@param		BYTE *pbyKey	암호화 키
*	@retval		없음.
************************************************************/
void CDEV_CDR::EncryptKeyData_TDES_ECB(BYTE *pbyInput, int nInputLen, BYTE *pbyOutput, int &nOutputLen, BYTE *pbyKey)
{
	int i = 0;
	CDES pDes;
	BYTE byTmp1[DATA_PADDING_SIZE] = { 0, }, byTmp2[DATA_PADDING_SIZE] = { 0, }, byTmp3[DATA_PADDING_SIZE] = { 0, };
	//	int nOutputLen = 0;
	BYTE byTempData[1024] = {0,};

	// [PCI-SSF Fix] byTempData(1024byte) ???? ?????÷ο? ?????? ???? ???? ???? ????
	if (nInputLen < 0 || nInputLen > (int)sizeof(byTempData))
	{
		nOutputLen = 0;
		return;
	}

	if (0 == (nInputLen % DATA_PADDING_SIZE))
		nOutputLen = nInputLen;
	else
		nOutputLen = (nInputLen / DATA_PADDING_SIZE + 1) * DATA_PADDING_SIZE;

	if(nInputLen > 0)
		memcpy(byTempData, pbyInput, nInputLen);

	for (i = 0; i < (nOutputLen / DATA_PADDING_SIZE); i++)
	{
		memcpy(byTmp1, &byTempData[i*DATA_PADDING_SIZE], DATA_PADDING_SIZE);

		pDes.ENCRYPT(byTmp1, byTmp2, pbyKey);
		pDes.DECRYPT(byTmp2, byTmp3, &pbyKey[DATA_PADDING_SIZE]);
		pDes.ENCRYPT(byTmp3, byTmp1, pbyKey);

		memcpy(&pbyOutput[i*DATA_PADDING_SIZE], byTmp1, DATA_PADDING_SIZE);
	}
}


/** *********************************************************
*	@brief		T-DES 암호화(CBC 모드)
*	@param		BYTE *pbyInput	Plain Text
*	@param		int nInputLen	InputLen
*	@param		BYTE *pbyOutput	Encrypted Text
*	@param		int &nOutputLen	Encrypted Text Length
*	@param		BYTE *pbyKey	암호화 키
*	@retval		없음.
************************************************************/
void CDEV_CDR::EncryptKeyData_TDES_CBC(BYTE *pbyInput, int nInputLen, BYTE *pbyOutput, int &nOutputLen, BYTE *pbyKey, BYTE *byEncIV)
{
	int i = 0, j = 0;
	CDES pDes;
	BYTE byTmp1[DATA_PADDING_SIZE] = { 0, }, byTmp2[DATA_PADDING_SIZE] = { 0, }, byTmp3[DATA_PADDING_SIZE] = { 0, };
	//	int nOutputLen = 0;
	BYTE byTempData[1024] = {0,};
	BYTE byIV[8] = { 0, };

	// [PCI-SSF Fix] byTempData(1024byte) ???? ?????÷ο? ?????? ???? ???? ???? ????
	// byTempData[nInputLen] = 0x80 ?е? ????? ???? ??? ?ε???(< sizeof(byTempData))?? ???
	if (nInputLen < 0 || nInputLen >= (int)sizeof(byTempData))
	{
		nOutputLen = 0;
		return;
	}

	memcpy(byIV, byEncIV, DATA_PADDING_SIZE);

	nOutputLen = (nInputLen / DATA_PADDING_SIZE + 1) * DATA_PADDING_SIZE;

	if(nInputLen > 0)
	{
		memcpy(byTempData, pbyInput, nInputLen);
		byTempData[nInputLen] = 0x80;	// 0x80 이전에 data만 real data로 판단함. 이후의 data는 모두 padding data임.
	}

	for (i = 0; i < (nOutputLen / DATA_PADDING_SIZE); i++)
	{
		memcpy(byTmp1, &byTempData[i*DATA_PADDING_SIZE], DATA_PADDING_SIZE);

		for (j = 0; j < DATA_PADDING_SIZE; j++)
			byTmp1[j] ^= byIV[j];

		pDes.ENCRYPT(byTmp1, byTmp2, pbyKey);
		pDes.DECRYPT(byTmp2, byTmp3, &pbyKey[DATA_PADDING_SIZE]);
		pDes.ENCRYPT(byTmp3, byTmp1, pbyKey);

		memcpy(byIV, byTmp1, DATA_PADDING_SIZE);
		memcpy(&pbyOutput[i*DATA_PADDING_SIZE], byTmp1, DATA_PADDING_SIZE);
	}
}


/** *********************************************************
*	@brief		T-DES 복호화(ECB 모드)
*	@param		BYTE *pbyInput	Encrypted Text
*	@param		int nInputLen	InputLen
*	@param		BYTE *pbyOutput	Plain Text
*	@param		BYTE *pbyKey	복호화 키
*	@retval		없음.
************************************************************/
void CDEV_CDR::DecryptKeyData_TDES_ECB(BYTE *pbyInput, int nInputLen, BYTE *pbyOutput, BYTE *pbyKey)
{
	int i = 0;
	CDES pDes;
	BYTE byTmp1[DATA_PADDING_SIZE] = { 0, }, byTmp2[DATA_PADDING_SIZE] = { 0, }, byTmp3[DATA_PADDING_SIZE] = { 0, };

	for (i = 0; i < (nInputLen / DATA_PADDING_SIZE); i++)
	{
		memcpy(byTmp1, &pbyInput[i*DATA_PADDING_SIZE], DATA_PADDING_SIZE);

		pDes.DECRYPT(byTmp1, byTmp2, pbyKey);
		pDes.ENCRYPT(byTmp2, byTmp3, &pbyKey[DATA_PADDING_SIZE]);
		pDes.DECRYPT(byTmp3, byTmp1, pbyKey);

		memcpy(&pbyOutput[i*DATA_PADDING_SIZE], byTmp1, DATA_PADDING_SIZE);
	}
}


/** *********************************************************
*	@brief		T-DES 복호화(CBC 모드)
*	@param		BYTE *pbyInput	Encrypted Text
*	@param		int nInputLen	InputLen
*	@param		BYTE *pbyOutput	Plain Text
*	@param		BYTE *pbyKey	복호화 키
*	@param		BYTE *byEncIV	제2의 복호화 키
*	@retval		없음.
************************************************************/
void CDEV_CDR::DecryptKeyData_TDES_CBC(BYTE *pbyInput, int nInputLen, BYTE *pbyOutput, BYTE *pbyKey, BYTE *byEncIV)
{
	int i = 0, j = 0;
	CDES pDes;
	BYTE byTmp1[DATA_PADDING_SIZE] = { 0, }, byTmp2[DATA_PADDING_SIZE] = { 0, }, byTmp3[DATA_PADDING_SIZE] = { 0, }, byTmp4[DATA_PADDING_SIZE] = { 0, };
	BYTE byIV[8] = { 0, };

	memcpy(byIV, byEncIV, DATA_PADDING_SIZE);
	for (i = 0; i < (nInputLen / DATA_PADDING_SIZE); i++)
	{
		memcpy(byTmp1, &pbyInput[i * DATA_PADDING_SIZE], DATA_PADDING_SIZE);

		pDes.DECRYPT(byTmp1, byTmp2, pbyKey);
		pDes.ENCRYPT(byTmp2, byTmp3, &pbyKey[DATA_PADDING_SIZE]);
		pDes.DECRYPT(byTmp3, byTmp4, pbyKey);

		for (j = 0; j < DATA_PADDING_SIZE; j++)
			byTmp4[j] ^= byIV[j];

		memcpy(byIV, byTmp1, DATA_PADDING_SIZE);
		memcpy(&pbyOutput[i * DATA_PADDING_SIZE], byTmp4, DATA_PADDING_SIZE);
	}
}


/** *********************************************************
*	@brief		복호화 후, real data를 얻기 위한
*	@param		BYTE *pbyInput	Plain Text(receive)
*	@param		int nInputLen	InputLen
*	@retval		Plain Text Length
************************************************************/
int CDEV_CDR::ParsingReadData(BYTE *pbyInput, int nInputLen)
{
	int nPlainDataLen = 0;
	int i = 0;

	for (i = nInputLen - 1; i >= 0; i--)
	{
		if (0x80 == pbyInput[i])
		{
			nPlainDataLen = i;
			break;
		}
	}

	return nPlainDataLen;
}


/** *********************************************************
*	@brief		버전 조회에 따른 암호화 지원 여부
*	@param		BOOL bSupportEncryption	암호화 지원 여부
*	@retval		없음.
************************************************************/
BOOL CDEV_CDR::GetSupportEncryption()
{

#if CDR_ENCRYPT_MODE
	return m_bSupportEncrypt4CDR;
#endif

	return FALSE;
}


/** *********************************************************
*	@brief		준정상 장애코드인지 판단하는 함수
*	@param	CString strErrorCode		ErrorCode
*	@retval TRUE	준정상 장애
*	@retval FALSE	이상 장애
************************************************************/
BOOL CDEV_CDR::IsUnSuccessReply()
{
	CString strCode = m_strLastError.Right(2);

	if( (strCode == _T("07")) || (strCode == _T("08")) || (strCode == _T("09")) || (strCode == _T("00")) || (strCode.IsEmpty() == TRUE))
	{
		m_strLastError.Empty();
		return TRUE;
	}
	else
		return FALSE;
}