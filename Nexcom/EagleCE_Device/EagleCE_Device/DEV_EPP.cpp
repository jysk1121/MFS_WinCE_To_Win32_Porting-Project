#include "stdafx.h"
#include "DEV_Define.h"
#include "DeviceSimUtil.h"
#include "DEV_EPP.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CDEV_EPP::CDEV_EPP()
{
	// DLL 경로 설정
	CString	strPath = _T("");

	// DLL 인스턴스 취득
	//m_hDll = LoadLibrary(EAGLE_ATM_EPP_DLL);
	m_hDll = LoadLibrary(L"MFSEPPDLL4CE.dll");

	// DLL API 취득
	LibMFSCommEPP_OpenPort = (MFSCommEPP_OpenPort)GetProcAddress(m_hDll, _T("MFSCommEPP_OpenPort"));
	LibMFSCommEPP_ClosePort = (MFSCommEPP_ClosePort)GetProcAddress(m_hDll, _T("MFSCommEPP_ClosePort"));
	LibMFSCommEPP_Reset = (MFSCommEPP_Reset)GetProcAddress(m_hDll, _T("MFSCommEPP_Reset"));
	LibMFSCommEPP_GetVersion = (MFSCommEPP_GetVersion)GetProcAddress(m_hDll, _T("MFSCommEPP_GetVersion"));
	LibMFSCommEPP_GetStatus = (MFSCommEPP_GetStatus)GetProcAddress(m_hDll, _T("MFSCommEPP_GetStatus"));
	LibMFSCommEPP_SelfTest = (MFSCommEPP_SelfTest)GetProcAddress(m_hDll, _T("MFSCommEPP_SelfTest"));
	LibMFSCommEPP_ExternalAuth = (MFSCommEPP_ExternalAuth)GetProcAddress(m_hDll, _T("MFSCommEPP_ExternalAuth"));
	LibMFSCommEPP_ChangePassword = (MFSCommEPP_ChangePassword)GetProcAddress(m_hDll, _T("MFSCommEPP_ChangePassword"));
	LibMFSCOMMEPP_NORMALKEYEVENT = (MFSCOMMEPP_NORMALKEYEVENT)GetProcAddress(m_hDll, _T("MFSCOMMEPP_NORMALKEYEVENT"));
	LibMFSCOMMEPP_NORMALKEYEVENTSTOP = (MFSCOMMEPP_NORMALKEYEVENTSTOP)GetProcAddress(m_hDll, _T("MFSCOMMEPP_NORMALKEYEVENTSTOP"));
	LibMFSCOMMEPP_SETLOCALTIME = (MFSCOMMEPP_SETLOCALTIME)GetProcAddress(m_hDll, _T("MFSCOMMEPP_SETLOCALTIME"));
	LibMFSCOMMEPP_GETPIN = (MFSCOMMEPP_GETPIN)GetProcAddress(m_hDll, _T("MFSCOMMEPP_GETPIN"));
	LibMFSCOMMEPP_GETPINCANCEL = (MFSCOMMEPP_GETPINCANCEL)GetProcAddress(m_hDll, _T("MFSCOMMEPP_GETPINCANCEL"));
	LibMFSCOMMEPP_PEKEXCHANGE = (MFSCOMMEPP_PEKEXCHANGE)GetProcAddress(m_hDll, _T("MFSCOMMEPP_PEKEXCHANGE"));
	LibMFSCOMMEPP_HOSTUNBIND = (MFSCOMMEPP_HOSTUNBIND)GetProcAddress(m_hDll, _T("MFSCOMMEPP_HOSTUNBIND"));
	LibMFSCOMMEPP_HOSTBIND = (MFSCOMMEPP_HOSTBIND)GetProcAddress(m_hDll, _T("MFSCOMMEPP_HOSTBIND"));
	LibMFSCOMMEPP_BUZZER = (MFSCOMMEPP_BUZZER)GetProcAddress(m_hDll, _T("MFSCOMMEPP_BUZZER"));
	LibMFSCOMMEPP_RSAKeyREPLACE = (MFSCOMMEPP_RSAKeyREPLACE)GetProcAddress(m_hDll, _T("MFSCOMMEPP_RSAKeyREPLACE"));
}


CDEV_EPP::~CDEV_EPP()
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
BOOL CDEV_EPP::IsDllLoaded(void)
{
#if EMULATION_EPP_DEVICE
	return TRUE;
#endif

	return (NULL != m_hDll);
}

// MFS EPP
typedef int (*EventCallBackFunction)(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength);

BOOL CDEV_EPP::EPP_OpenPort(HWND hWnd, int nPortNum)
{
#if EMULATION_EPP_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	return CDevSim::Confirm(_T("EPP"), _T("EPP_OpenPort"));
#endif
	
	// MFS EPP
	int nRet = 0;
	DWORD dwBaud = 115200;

	typedef int (*_OPENDEVICE)(int, DWORD);
	_OPENDEVICE OPENDEVICE = NULL;

	if(m_hDll)
	{
		OPENDEVICE = (_OPENDEVICE)GetProcAddress(m_hDll, L"EPPOpenDevice");
		if(OPENDEVICE)
			nRet = OPENDEVICE(nPortNum, dwBaud);
		else
			nRet = 0xFFFE;
	}
	else
		nRet = 0xFFFF;

	// Set Callback function
	typedef int (*_REGISTEVENTCALLBACKFUNCTION)(EventCallBackFunction);
	_REGISTEVENTCALLBACKFUNCTION REGISTEVENTCALLBACKFUNCTION = NULL;

	if(m_hDll)
	{
		REGISTEVENTCALLBACKFUNCTION = (_REGISTEVENTCALLBACKFUNCTION)GetProcAddress(m_hDll, L"EPPRegistEventCallBackFunction");
		if(REGISTEVENTCALLBACKFUNCTION)
			nRet = REGISTEVENTCALLBACKFUNCTION((EventCallBackFunction)EventControlFunc);
		else
			nRet = 0xFFFE;
	}
	else
		nRet = 0xFFFF;

	return nRet;

#if 0
	if (NULL == LibMFSCommEPP_OpenPort)
	{
		return FALSE;
	}

	return LibMFSCommEPP_OpenPort(hWnd, nPortNum);
#endif
}

BOOL CDEV_EPP::EPP_ClosePort()
{
#if EMULATION_EPP_DEVICE
	return CDevSim::Confirm(_T("EPP"), _T("EPP_ClosePort"));
#endif

	typedef int (*_CLOSEDEVICE)();
	_CLOSEDEVICE CLOSEDEVICE = NULL;
	int nRet = 0;

	if(m_hDll)
	{
		CLOSEDEVICE = (_CLOSEDEVICE)GetProcAddress(m_hDll, L"EPPCloseDevice");
		if(CLOSEDEVICE)
			nRet = CLOSEDEVICE();
		else
			nRet = 0xFFFE;
	}
	else
		nRet = 0xFFFF;
	return nRet;

#if 0
	if (NULL == LibMFSCommEPP_ClosePort)
	{
		return FALSE;
	}

	LibMFSCommEPP_ClosePort();

	return TRUE;
#endif
}

BOOL CDEV_EPP::EPP_Reset()
{
#if EMULATION_EPP_DEVICE
	return CDevSim::Confirm(_T("EPP"), _T("EPP_Reset"));
#endif

	if (NULL == LibMFSCommEPP_Reset)
	{
		return FALSE;
	}

	return LibMFSCommEPP_Reset();
}

BOOL CDEV_EPP::EPP_GetVersion(BYTE TYPE, LPEPPRESULT lpResult)
{
#if EMULATION_EPP_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	ZeroMemory(lpResult, sizeof(EPPRESULT));
	lpResult->error_cd = (DWORD)CDevSim::ReadInt(_T("EPP"), _T("VersionErrorCode"), 0);
	strcpy_s(lpResult->sResult, sizeof(lpResult->sResult), "EPPSIM 01.00");
	return TRUE;
#endif

	if (NULL == LibMFSCommEPP_GetVersion)
	{
		return FALSE;
	}

	return LibMFSCommEPP_GetVersion(TYPE, lpResult);
}

BOOL CDEV_EPP::EPP_GetStatus(LPEPPSTATUS lpStatus)
{
#if EMULATION_EPP_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	ZeroMemory(lpStatus, sizeof(EPPSTATUS));
	lpStatus->error_cd        = (DWORD)CDevSim::ReadInt(_T("EPP"), _T("StatusErrorCode"), 0);
	lpStatus->b_bpk           = (BOOL)CDevSim::ReadInt(_T("EPP"), _T("StatusBpk"), 1);
	lpStatus->b_fwdownload    = (BOOL)CDevSim::ReadInt(_T("EPP"), _T("StatusFwDownload"), 0);
	lpStatus->b_pwchange      = (BOOL)CDevSim::ReadInt(_T("EPP"), _T("StatusPwChange"), 0);
	lpStatus->b_initkey       = (BOOL)CDevSim::ReadInt(_T("EPP"), _T("StatusInitKey"), 1);
	lpStatus->b_pekload       = (BOOL)CDevSim::ReadInt(_T("EPP"), _T("StatusPekLoad"), 1);
	lpStatus->b_rswitch       = (BOOL)CDevSim::ReadInt(_T("EPP"), _T("StatusRSwitch"), 0);
	lpStatus->b_rswitch_active= (BOOL)CDevSim::ReadInt(_T("EPP"), _T("StatusRSwitchActive"), 0);
	lpStatus->b_hostbound     = (BOOL)CDevSim::ReadInt(_T("EPP"), _T("StatusHostBound"), 1);
	return TRUE;
#endif

	if (NULL == LibMFSCommEPP_GetStatus)
	{
		return FALSE;
	}

	return LibMFSCommEPP_GetStatus(lpStatus);
}

BOOL CDEV_EPP::EPP_SelfTest()
{
#if EMULATION_EPP_DEVICE
	return CDevSim::Confirm(_T("EPP"), _T("EPP_SelfTest"));
#endif

	if (NULL == LibMFSCommEPP_SelfTest)
	{
		return FALSE;
	}

	return LibMFSCommEPP_SelfTest();
}

BOOL CDEV_EPP::EPP_ExternalAuth()
{
#if EMULATION_EPP_DEVICE
	return CDevSim::Confirm(_T("EPP"), _T("EPP_ExternalAuth"));
#endif

	if (NULL == LibMFSCommEPP_ExternalAuth)
	{
		return FALSE;
	}

	return LibMFSCommEPP_ExternalAuth();
}

BOOL CDEV_EPP::EPP_ChangePassword()
{
#if EMULATION_EPP_DEVICE
	return CDevSim::Confirm(_T("EPP"), _T("EPP_ChangePassword"));
#endif

	if (NULL == LibMFSCommEPP_ChangePassword)
	{
		return FALSE;
	}

	return LibMFSCommEPP_ChangePassword();
}

BOOL CDEV_EPP::EPP_NORMALKEYEVENT()
{
#if EMULATION_EPP_DEVICE
	return CDevSim::Confirm(_T("EPP"), _T("EPP_NORMALKEYEVENT"));
#endif

	BYTE pData[128];
	DWORD dwLength = 0;
	int nPos = 0;
	CString str;

	memset(pData, 0, sizeof(pData));
	WORD wPkt_length = 0;

	pData[0] = 0x02;	//STX
	pData[1] = 0x30;	//MTYPE
	pData[2] = 0xb1;	//CMD
	pData[3] = 0x30;	//Parameter
	pData[4] = 00;		//Length Low
	pData[5] = 00;		//Length High

	pData[6] = 0x03;		//ETX

	int i = 0;
	BYTE CRC = CalcLRC (&pData[1], 5);	/* remainder = 0 	*/
	pData[7] = CRC;

	dwLength = 8;

	DWORD dwRet = 0;
	BYTE byRet[128];

	typedef int (*_EXECUTECOMMAND)(int, LPBYTE, DWORD, LPBYTE, DWORD&, BOOL, BOOL);
	_EXECUTECOMMAND EXECUTECOMMAND = NULL;
	int nRet = 0;

	if(m_hDll)
	{
		EXECUTECOMMAND = (_EXECUTECOMMAND)GetProcAddress(m_hDll, L"EPPExecuteCommand");
		if(EXECUTECOMMAND)
			nRet = EXECUTECOMMAND(0xb1, pData, dwLength, byRet, dwRet, 0, 1);
		else
			nRet = 0xFFFE;
	}
	else
		nRet = 0xFFFF;
	return nRet;


#if 0
	if (NULL == LibMFSCOMMEPP_NORMALKEYEVENT)
	{
		return FALSE;
	}

	return LibMFSCOMMEPP_NORMALKEYEVENT();
#endif
}

BOOL CDEV_EPP::EPP_NORMALKEYEVENTSTOP()
{
#if EMULATION_EPP_DEVICE
	return CDevSim::Confirm(_T("EPP"), _T("EPP_NORMALKEYEVENTSTOP"));
#endif

	BYTE pData[128];
	DWORD dwLength = 0;
	int nPos = 0;
	CString str;

	memset(pData, 0, sizeof(pData));
	WORD wPkt_length = 0;

	pData[0] = 0x02;	//STX
	pData[1] = 0x30;	//MTYPE
	pData[2] = 0xb1;	//CMD
	pData[3] = 0x30;	//Parameter
	pData[4] = 00;		//Length Low
	pData[5] = 00;		//Length High

	pData[6] = 0x03;		//ETX

	int i = 0;
	BYTE CRC = CalcLRC (&pData[1], 5);	/* remainder = 0 	*/
	pData[7] = CRC;

	dwLength = 8;

	DWORD dwRet = 0;
	BYTE byRet[128];

	typedef int (*_EXECUTECOMMAND)(int, LPBYTE, DWORD, LPBYTE, DWORD&, BOOL, BOOL);
	_EXECUTECOMMAND EXECUTECOMMAND = NULL;
	int nRet = 0;

	if(m_hDll)
	{
		EXECUTECOMMAND = (_EXECUTECOMMAND)GetProcAddress(m_hDll, L"EPPExecuteCommand");
		if(EXECUTECOMMAND)
			nRet = EXECUTECOMMAND(0x42, pData, dwLength, byRet, dwRet, 0, 1);
		else
			nRet = 0xFFFE;
	}
	else
		nRet = 0xFFFF;
	return nRet;


#if 0
	if (NULL == LibMFSCOMMEPP_NORMALKEYEVENTSTOP)
	{
		return FALSE;
	}

	return LibMFSCOMMEPP_NORMALKEYEVENTSTOP();
#endif
}

BOOL CDEV_EPP::EPP_SETLOCALTIME()
{
#if EMULATION_EPP_DEVICE
	return CDevSim::Confirm(_T("EPP"), _T("EPP_SETLOCALTIME"));
#endif

	if (NULL == LibMFSCOMMEPP_SETLOCALTIME)
	{
		return FALSE;
	}

	return LibMFSCOMMEPP_SETLOCALTIME();
}

BOOL CDEV_EPP::EPP_GETPIN(LPEPPGETPIN lpeppgetpin)
{
#if EMULATION_EPP_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	// No PIN material is ever fabricated here - only the command outcome is simulated.
	return CDevSim::Confirm(_T("EPP"), _T("EPP_GETPIN"), _T("OK = PIN entry completed, NG = PIN entry failed/cancelled"));
#endif

	if (NULL == LibMFSCOMMEPP_GETPIN)
	{
		return FALSE;
	}

	return LibMFSCOMMEPP_GETPIN(lpeppgetpin);
}

BOOL CDEV_EPP::EPP_GETPINCANCEL()
{
#if EMULATION_EPP_DEVICE
	return CDevSim::Confirm(_T("EPP"), _T("EPP_GETPINCANCEL"));
#endif

	if (NULL == LibMFSCOMMEPP_GETPINCANCEL)
	{
		return FALSE;
	}

	return LibMFSCOMMEPP_GETPINCANCEL();
}

BOOL CDEV_EPP::EPP_PEKEXCHANGE(LPEXCHANGEPEK lpexchangepek)
{
#if EMULATION_EPP_DEVICE
	return CDevSim::Confirm(_T("EPP"), _T("EPP_PEKEXCHANGE"));
#endif

	if (NULL == LibMFSCOMMEPP_PEKEXCHANGE)
	{
		return FALSE;
	}

	return LibMFSCOMMEPP_PEKEXCHANGE(lpexchangepek);
}

BOOL CDEV_EPP::EPP_HOSTUNBIND()
{
#if EMULATION_EPP_DEVICE
	return CDevSim::Confirm(_T("EPP"), _T("EPP_HOSTUNBIND"));
#endif

	if (NULL == LibMFSCOMMEPP_HOSTUNBIND)
	{
		return FALSE;
	}

	return LibMFSCOMMEPP_HOSTUNBIND();
}

BOOL CDEV_EPP::EPP_HOSTBIND(LPEPPHOSTBIND lpepphostbind)
{
#if EMULATION_EPP_DEVICE
	return CDevSim::Confirm(_T("EPP"), _T("EPP_HOSTBIND"));
#endif

	if (NULL == LibMFSCOMMEPP_HOSTBIND)
	{
		return FALSE;
	}

	return LibMFSCOMMEPP_HOSTBIND(lpepphostbind);
}

BOOL CDEV_EPP::EPP_BUZZER(BOOL bOn)
{
#if EMULATION_EPP_DEVICE
	// Buzzer has no desktop equivalent and no state to inspect; always succeeds
	// so the simulator does not interrupt the flow with a dialog on every beep.
	return TRUE;
#endif

	if (NULL == LibMFSCOMMEPP_BUZZER)
	{
		return FALSE;
	}

	return LibMFSCOMMEPP_BUZZER(bOn);
}

BOOL CDEV_EPP::EPP_RSAKeyREPLACE(LPEXCHANGERSAKEY lpexchangersakey)
{
#if EMULATION_EPP_DEVICE
	return CDevSim::Confirm(_T("EPP"), _T("EPP_RSAKeyREPLACE"));
#endif

	if (NULL == LibMFSCOMMEPP_RSAKeyREPLACE)
	{
		return FALSE;
	}

	return LibMFSCOMMEPP_RSAKeyREPLACE(lpexchangersakey);
}

/** **********************************************************
*	@brief		에러코드 취득
*	@retval		없음
************************************************************/
CString CDEV_EPP::GetLastError(void)
{
	return m_strLastError;
}

/** **********************************************************
*	@brief		에러코드 취득
*	@retval		없음
************************************************************/
BYTE CDEV_EPP::CalcLRC(unsigned char *pData, DWORD dwLen)
{
	unsigned char	lrc = 0;
	DWORD			i = 0;

	for(; i < dwLen; i++)
		lrc ^= *pData++;

	return lrc;
}

/**< notification command definication */
#define		PIN_ENTRY				0x30
#define		PWD_ENTRY				0x31
#define		NPWD_VERIFY				0x32/**< 알림 command */
#define		NORMAL_KEY				0x33/**< Normal key pressed */
#define		SECUKEY_ENTRY			0x34/*Master Key/Session Key를 지원하기 위해 추가 2015.10.27*/


/**< notification parameter definition */
#define		NOTICE_MESSAGE			0x30
#define		EFFECTKEY_PRESS			0x31
#define		KEYIN_COMPLETE			0x32
#define		NEXTIN_CONTINUE			0x33
#define		EXCEPT_MESSAGE			0x34/**< 예외 상황 발생 : key input일때 KEYIN_COMPLETE와 같은 효과
									다만 예외 상황이므로 예외 상황 CODE값을 정의하고 참조 */
/**< noticefication message code */
#define		KEYENTRY_START			0x30
#define		KEYENTRY_LACK			0x31/**< key입력부족(under 4) */
#define		KEYENTRY_OVER			0x32/**< key입력포화(over 12) */

/**< exception code definition */
#define		KEYENTRY_TIMEOUT		0x00
#define		PWDVERIFY_FAIL			0x01
#define		OVERALL_TIMEOUT1		0x02/**< PIN entry overall timeout(30 sec) */
#define		OVERALL_TIMEOUT2		0x03/**< PIN encryption overall timeout(3600 sec) */
//----------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////
//Event Control Functions
static CDEV_EPP	*pDevEPP;

int CDEV_EPP::EventControlFunc(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength)
{
	pDevEPP->eventHandler(usEventID, usParam, lpByte, usDataLength);
	return 0;
}

int CDEV_EPP::eventHandler(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength)
{
	CString str;
	str.Format(_T("%04X : "), usEventID & 0xFFFF);
	//ShowHexResult(str, lpByte, usDataLength);

	switch(usEventID)
	{
	case NORMAL_KEY:
		PressKeyHandler(usParam, lpByte, usDataLength);
		break;

	case PIN_ENTRY:
		break;

	case PWD_ENTRY:
		break;

	case NPWD_VERIFY:
		break;

	case SECUKEY_ENTRY:
		break;

	default:
		break;
	}

	return 0;
}

int CDEV_EPP::PressKeyHandler(unsigned short usParam, byte *lpByte, unsigned short usDataLength)
{
	if(usDataLength < 1)
		return -1;

	BYTE byParam = (BYTE)usParam;
	BYTE byKey = lpByte[0];

	CString str;
	if(byParam == EFFECTKEY_PRESS)
	{
		if((byKey >= 0x30) && (byKey <= 0x39))
			byKey -= 0x30;
		else if((byKey >= 0x41) && (byKey <= 0x46))
			byKey = byKey - 0x41 + 10;

		str.Format(L"Key Pressed : %X", byKey & 0xFF);
		//ShowResult(str);
	}
	return 0;
}
