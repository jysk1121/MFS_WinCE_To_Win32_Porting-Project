// Modem.cpp: implementation of the CModem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Modem.h"
#include "Queue.h"
//#define NH_DEBUG
#include ".\Common\NHDbgApi.h"

#include ".\Dev\DevDefine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern int		put_data(char *recv_data, int recv_size);
extern int		put_event(char recv_evt);

#define _INTIALSTRING_REG	_T("Drivers\\Unimodem\\Init")

CComm		m_Comm;
DWORD		dwAPIVersion	= 0x00030000;
DWORD		dwHiVersion		= 0x00030000;
DWORD		dwLoVersion		= 0x00010000;
DWORD		m_dwRequestID	= 0;
DWORD		m_dwDeviceID	= 0;
HLINEAPP	m_hLineApp		= NULL;
HLINE		m_hLine			= NULL;
HCALL		m_hCall			= NULL;

// USER DEFINE
BOOL		m_nCallOrigin	= FALSE;
BOOL		gbNewCall		= FALSE;

LONG	GetLineDevCaps(HLINEAPP hLineApp, DWORD dwDeviceID, LPLINEDEVCAPS *ppLineDevCaps);
LPLINECALLPARAMS CreateCallParams(LPLINECALLPARAMS lpCallParams, LPCSTR lpszDialableAddress);
void DoLineCallState(DWORD hCall, DWORD dwCallState, DWORD dwCallStateDetail, DWORD dwCallPrivilege);
LONG GetLineID(LPVARSTRING *ppVarString);

int	SetModemInitialString(BOOL bRMSMode);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModem::CModem()
{
	CLineCtrl::Register(LINE_DIALUP, this);
}

CModem::~CModem()
{

}

int	CModem::LineOpen(LPCTSTR Dest_Info, LPCTSTR dest_port, LPCTSTR option, LPCTSTR mode)
{
	char PhoneNum[64] = "";
	int	 nreturn = 0;

	// set call origin
	// [#122] NH AIREAT 2008.04.22
	if (option[0] == '1')
	{
		m_nCallOrigin = FALSE;
		SetModemInitialString(TRUE);
	}
	else
	{
		m_nCallOrigin = TRUE;		
		SetModemInitialString(FALSE);
	}
	// end of [#122]

	// Client Mode
	if (mode[0] == '0')
	{
		sprintf(PhoneNum, "%S", Dest_Info);
		
		// Sequence Initialize -> Lineopen -> Dial
		if (Initialize() == ERROR_SUCCESS)
		{
			if (Open() == ERROR_SUCCESS)
			{
				if (option[0] == '2')	// MODEM DEVICE TEST
					return ERROR_SUCCESS;

				nreturn = Dial((LPCSTR)PhoneNum);
				if (nreturn > ERROR_SUCCESS)
				{
NHDEBUG(1, (_T("DIAL SUCCESS[%x]\n"), nreturn));
					// success
					return 0;
				}
				// Dial fail
				// detail error data return
				
				switch(nreturn)
				{
				case LINEERR_ADDRESSBLOCKED:
					nreturn = 15;		// D0301
					break;
				case LINEERR_INVALTERMINALID:
					nreturn = 16;		// D0302
					break;
				case LINEERR_CALLUNAVAIL:
					nreturn = 17;		// D0303
					break;
				case LINEERR_DIALBILLING:
				case LINEERR_DIALDIALTONE:
				case LINEERR_DIALPROMPT:
				case LINEERR_DIALQUIET:
					nreturn = 18;		// D0304
					break;
				case LINEERR_INUSE:
				case LINEERR_INVALADDRESS:
				case LINEERR_INVALADDRESSID:
				case LINEERR_INVALADDRESSMODE:
				case LINEERR_INVALBEARERMODE:
				case LINEERR_INVALCALLPARAMS:
				case LINEERR_INVALCOUNTRYCODE:
					nreturn = 19;		// D0305
					break;
				case LINEERR_OPERATIONFAILED:
					nreturn = 20;		// D0306
					break;
				case LINEERR_RESOURCEUNAVAIL:
					nreturn = 21;		// D0307
					break;
				default:
					nreturn = 14;		// D0300
					break;
				}
				return nreturn;
			}
			// Open fail
			return 13;
		}
		// Initialize fail
		return 12;
	}
	// Server Mode
	else
	{
		gbNewCall = FALSE;
		
//		sprintf(PhoneNum, "%S", Dest_Info);	// 사용안함 KSK 2008.11.26
		
		// Sequence Initialize -> Lineopen -> Dial
		if (Initialize() == ERROR_SUCCESS)
		{
			if (Open() == ERROR_SUCCESS)
			{
				// [#401] [NH] KSK 2008.8.18
				::lineSetStatusMessages(m_hLine, 0x1ffffff, 0);
				// end of [#401]
				return 0;
			}
			// Open fail
			return 13;
		}
		// Initialize fail
		return 12;
	}
}

int	CModem::LineClose()
{
	int	nRes;
	gbNewCall = FALSE;

	if (Close() == ERROR_SUCCESS)
	{
		if (Shutdown() == ERROR_SUCCESS)
		{
			nRes = 0;
		}
		else
		{
		// Shutdown fail
			nRes = 2;
		}
	}
	else
	{
		// Close fail
		nRes = 1;
	}

#if (_WIN32_WCE >= 0x600)
	{
#define IOCTL_SYSTEM_RESET	1
#define IOCTL_MODEM_RESET	2

		HANDLE hReset = CreateFile(L"RST1:", GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, NULL, NULL);
		if (hReset != NULL)
		{
			DWORD dwBuffer = IOCTL_MODEM_RESET;
			DWORD dwWritten;
			WriteFile(hReset, &dwBuffer, 1, &dwWritten, NULL);
			Sleep(100);
			CloseHandle(hReset);
		}
	}
#endif

	return nRes;
}

int	CModem::LineSendData(BYTE *send_data, int len)
{
	m_Comm.WriteData(send_data, (DWORD)len);

	return 0;
}

int CModem::LineAccept(BYTE opt)
{
	if (opt && gbNewCall == TRUE)
	{
		if (Answer() == ERROR_SUCCESS)
			return 0;
	}

	return 1;
}

int CModem::LineCallState()
{
	if (gbNewCall == TRUE)
		return 0;
	
	return 1;
}

LONG CModem::Initialize()
{
#if (_WIN32_WCE >= 0x600)
	// EV_CTS, EV_DSR 설정으로 현재 모뎀 상태 조회 코드 추가.

	HANDLE	hModemPort = NULL;
	hModemPort = CreateFile(_T("COM5:"), GENERIC_READ|GENERIC_WRITE,NULL,NULL,OPEN_EXISTING,NULL,NULL);
	if (hModemPort != NULL)
	{
		DWORD	dwCode;
		BYTE	byOutBuffer[512];
		DWORD	dwBytes;
		DWORD	dwModemStatus = 0;

		dwCode = 0x1b0034;//IOCTL_SERIAL_GET_MODEMSTATUS
		DeviceIoControl(hModemPort, dwCode, NULL, 0, (LPVOID)byOutBuffer, sizeof(byOutBuffer), &dwBytes, NULL);
		DeviceIoControl(hModemPort, dwCode, NULL, 0, (LPVOID)byOutBuffer, sizeof(byOutBuffer), &dwBytes, NULL);

		GetCommModemStatus(hModemPort, &dwModemStatus);
		GetCommModemStatus(hModemPort, &dwModemStatus);

		CloseHandle(hModemPort);
	}

#endif

	LINEINITIALIZEEXPARAMS	tmpLineInitializeExParams;
	DWORD					dwNumDevs = 0;
	LINEEXTENSIONID			Extid;
	LPLINEDEVCAPS			pLineDevCaps = 0;
	LONG					tr = ERROR_SUCCESS;

	tmpLineInitializeExParams.dwTotalSize = sizeof(LINEINITIALIZEEXPARAMS);
	tmpLineInitializeExParams.dwNeededSize = sizeof(LINEINITIALIZEEXPARAMS);
	tmpLineInitializeExParams.dwUsedSize = sizeof(LINEINITIALIZEEXPARAMS);
	tmpLineInitializeExParams.dwOptions = LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

	// Initialize line
	tr = ::lineInitializeEx(&m_hLineApp,				// the application's handle for TAPI
							 AfxGetInstanceHandle(),	// instance handle of client application
							 LineCallbackFunc,			// the address of callback function
							 NULL,						// application-supplied name
							 &dwNumDevs,				//
							 &dwAPIVersion,
							 &tmpLineInitializeExParams);

	if (tr == ERROR_SUCCESS)
	{
		for (DWORD dwDeviceID = 0; (dwDeviceID < dwNumDevs); dwDeviceID++)
		{
			// Negotiate the API versions
			tr = ::lineNegotiateAPIVersion(m_hLineApp,		// Line usage handle
										   dwDeviceID,				// Line ID
										   dwLoVersion,		// Minimum version
										   dwHiVersion,		// Maximum version
										   &dwAPIVersion,	// Negotiated version
										   &Extid);					// Available ext. ID
			if (tr == ERROR_SUCCESS)
			{
				tr = GetLineDevCaps(m_hLineApp, dwDeviceID, &pLineDevCaps);

				if (tr == ERROR_SUCCESS)
				{
					if (pLineDevCaps == NULL)	return -1;	// KSK 2009.9.9 Codesonar 지적사항 대책 (Codesonar지적오류로 보임)

					if ((pLineDevCaps->dwBearerModes & LINEBEARERMODE_VOICE) &&
						(pLineDevCaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM) &&
						(pLineDevCaps->dwLineFeatures & LINEFEATURE_MAKECALL))
					{
						m_dwDeviceID = dwDeviceID;
					}
				}

				// [CS#79] NH AIREAT 2008.3.14
				if (pLineDevCaps)
				{
					::free(pLineDevCaps);	// 2008.01.07 위치 이동 V01.02.24
					pLineDevCaps = NULL;
				}
				// end of [CS#79]
			}
		}
	}
	
	return tr;
}

LONG CModem::Open()
{
	LONG			tr = ERROR_SUCCESS;

	// Open the line for an incoming call
	tr = ::lineOpen(m_hLineApp,						// a handle to the application's registration with TAPI
					m_dwDeviceID,					// identity of the line device
					&m_hLine,						// HLINE handle
					dwAPIVersion,					// the API version number
					NULL,							// the extension version number
					(DWORD)this,					// user-instance data
					LINECALLPRIVILEGE_OWNER,		//
					LINEMEDIAMODE_DATAMODEM,		// the media type
					NULL);							// a pointer to a structure of LINECALLPARAMS
	return tr;
}

LONG CModem::Answer()
{
	LONG	tr = ERROR_SUCCESS;
	LONG	nRet = 0;

	// [CS#82] NH AIREAT 2008.3.14
	// 추후 리턴값 상세분류를 위해 switch를 사용.
	tr = ::lineAnswer(m_hCall, NULL, 0);
	if (tr < 0)
	{
		//HangUp();

		switch (tr)
		{
		case LINEERR_INUSE:
		case LINEERR_OPERATIONUNAVAIL:
		case LINEERR_INVALCALLHANDLE:
		case LINEERR_OPERATIONFAILED:
		case LINEERR_INVALCALLSTATE:
		case LINEERR_RESOURCEUNAVAIL:
		case LINEERR_INVALPOINTER:
		case LINEERR_UNINITIALIZED:
		case LINEERR_NOMEM:
		case LINEERR_USERUSERINFOTOOBIG:
		case LINEERR_NOTOWNER:
			nRet = 1;
			break;
		default:
			nRet = 1;
			break;
		}
	}
	// end of [#82]

	return nRet;

}

LONG CModem::Dial(LPCSTR pszPhoneNo)
{
	LONG				tr = ERROR_SUCCESS;
	LPLINECALLPARAMS	lpLineCallParams = NULL;

	// Setup our CallParams for DATAMODEM settings.
	lpLineCallParams = CreateCallParams(lpLineCallParams, NULL);
	if (lpLineCallParams == NULL)		return -1;
	
	CString strPhoneNo = CString(pszPhoneNo);
	
	tr = ::lineMakeCall(m_hLine,
						&m_hCall,
						strPhoneNo,
						NULL,
						lpLineCallParams);
	if (tr > ERROR_SUCCESS)
	{
		m_hCall = NULL;
		m_dwRequestID = tr;
	}
	
	return tr;
}

// [#525] US KSK 2009.05.25	현재 사용안 안하나, 추후 MODEM 문제시 적용 검토 필요 (LineMakeCall() <->LineDrop())
BOOL CModem::HangUp()
{
	LONG	tr;
	
	if (m_hCall != NULL)
	{
		tr = ::lineDrop(m_hCall, NULL, 0);
		m_hCall = NULL;
		
		return TRUE;
	}
	
	return FALSE;
}
// end of [#525]


LONG CModem::Close()
{
	LONG	tr = ERROR_SUCCESS;
/*	// 뉴질랜드 모뎀이슈 테스트코드, 모뎀 disconnect 이 잘 안되는 문제로 추가된 코드임. 적용유무는 뉴질랜드 이슈결과에 따라 결정함.
	LINECALLSTATUS stLineStatus;
NHDEBUG(1, (_T("CModem::Close() m_hCall[%x]\n"), m_hCall));
	if (m_hCall != NULL)
	{
		memset(&stLineStatus, 0, sizeof(LINECALLSTATUS));
		tr = ::lineGetCallStatus(m_hCall, &stLineStatus);
NHDEBUG(1, (_T("CModem::Close() lineGetCallStatus returned stLineStatus[%x]\n"), stLineStatus));
		if(!tr)
			tr = ::lineDrop(m_hCall, NULL, 0);
NHDEBUG(1, (_T("CModem::Close() lineDrop returned [%x] m_hCall[%x]\n"), tr, m_hCall));
	}
	m_hCall = NULL;
*/
	if (m_hLine != NULL)		tr = ::lineClose(m_hLine);
	m_hLine = NULL;
	
	m_Comm.ClosePort();

	return tr;
}

LONG CModem::Shutdown()
{
	LONG	tr = ERROR_SUCCESS;

	if (m_hLineApp != NULL)		tr = ::lineShutdown(m_hLineApp);
	m_hLineApp = NULL;

	return tr;
}


LONG GetLineDevCaps(HLINEAPP hLineApp, DWORD dwDeviceID, LPLINEDEVCAPS *ppLineDevCaps)
{
	LONG  tr = ERROR_SUCCESS;
	DWORD dwNeededSize = sizeof(LINEDEVCAPS) * 2;

	do
	{
		// Get some more memory if we don't have enough
		if ((*ppLineDevCaps == NULL) ||
			((*ppLineDevCaps)->dwTotalSize < dwNeededSize))
		{
			*ppLineDevCaps = (LPLINEDEVCAPS)::realloc(*ppLineDevCaps, dwNeededSize);
			if (*ppLineDevCaps)
				(*ppLineDevCaps)->dwTotalSize = dwNeededSize;
			else 
			{
				tr = LINEERR_NOMEM;
				break;	// 2008.01.07 추가 V01.02.24
			}
		}

		// Fill in the buffer
		tr = ::lineGetDevCaps(hLineApp,
							  dwDeviceID,
							  dwAPIVersion,
							  0,
							  *ppLineDevCaps);

		// Check how much memory we need
		// (some TSPs succeed even if the data size is too small)
		if ((tr == LINEERR_STRUCTURETOOSMALL) ||
			((tr == ERROR_SUCCESS) &&
			 ((*ppLineDevCaps)->dwTotalSize < (*ppLineDevCaps)->dwNeededSize)))
		{
			dwNeededSize = (*ppLineDevCaps)->dwNeededSize;
			tr = LINEERR_STRUCTURETOOSMALL;
		}
	}
	while (tr == LINEERR_STRUCTURETOOSMALL);

	return tr;
}

void DoLineCallState(DWORD hCall, DWORD dwCallState, DWORD dwCallStateDetail, DWORD dwCallPrivilege)
{
	LPVARSTRING pldc	= NULL;
	HANDLE		hComm	= NULL;

	switch (dwCallState)
	{
	case LINECALLSTATE_IDLE:
		::lineDeallocateCall((HCALL)hCall);
		break;

	case LINECALLSTATE_OFFERING:
		NHDEBUG(1, (_T("LINECALLSTATE_OFFERING\n")));
		break;
	case LINECALLSTATE_BUSY:
		NHDEBUG(1, (_T("LINECALLSTATE_BUSY\n")));
		break;
	case LINECALLSTATE_DIALING:	
		m_hCall = (HCALL)hCall;
		NHDEBUG(1, (_T("LINECALLSTATE_DIALING\n")));
		break;

	case LINECALLSTATE_ACCEPTED:		
		NHDEBUG(1, (_T("LINECALLSTATE_ACCEPTED\n")));
		break;

	case LINECALLSTATE_CONNECTED:	
		NHDEBUG(1, (_T("LINECALLSTATE_CONNECTED\n")));
		
		if (GetLineID(&pldc) == ERROR_SUCCESS)
		{
			if (pldc == NULL)	return;		// KSK 2009.9.9 Codesonar 지적사항 대책

			hComm = *((LPHANDLE)((LPBYTE)pldc + pldc->dwStringOffset));
			NHDEBUG(1, (_T("GET COMM HANDLE SUCCESS\n")));
			
			if (m_Comm.OpenPort(hComm, 4096, m_nCallOrigin) == TRUE)
			{
				put_event(CONNECT_EVT);
				NHDEBUG(1, (_T("COMPORT OPEN SUCCESS\n")));
			}
			else
			{
				put_event(DISCONNECT_EVT);					// [2ND] AIREAT 2009.04.18
				NHDEBUG(1, (_T("COMPORT OPEN FAIL\n")));
			}
			::free(pldc);			// [CODESONAR] /* Free Null Pointer (ID: 171) */
		}
		
		//::free(pldc);			// [CODESONAR] /* Free Null Pointer (ID: 171) */
		pldc = NULL;
		break;

	case LINECALLSTATE_DISCONNECTED:
		NHDEBUG(1, (_T("LINECALLSTATE_DISCONNECTED\n")));
		switch(dwCallStateDetail)
		{
		case LINEDISCONNECTMODE_NODIALTONE:
			put_event(USER_NODIALTONE);
			// D1800
			NHDEBUG(1, (_T("DETAIL INFO - No Dial Tone\n")));
			break;
		case LINEDISCONNECTMODE_NOANSWER:
			// D1900
			put_event(USER_NOANSWER);
			NHDEBUG(1, (_T("DETAIL INFO - No Answer\n")));
			break;
		case LINEDISCONNECTMODE_BUSY:
			// D2000
			put_event(USER_LINEBUSY);
			NHDEBUG(1, (_T("DETAIL INFO - Line Busy\n")));
			break;
		default:
			put_event(DISCONNECT_EVT);
			// D1500
			NHDEBUG(1, (_T("DETAIL INFO - Modem Dial Connection Timeout\n")));
			break;
		}	
		break;
	}
}

LONG GetLineID(LPVARSTRING *ppVarString)
{
	LONG  tr = ERROR_SUCCESS;
	DWORD dwNeededSize = sizeof(LINEDEVCAPS);

	do
	{
		// Get some more memory if we don't have enough
		if ((*ppVarString == NULL) || ((*ppVarString)->dwTotalSize < dwNeededSize))
		{
			*ppVarString = (LPVARSTRING)::realloc(*ppVarString, dwNeededSize);
			if (*ppVarString)	(*ppVarString)->dwTotalSize = dwNeededSize;
			else
			{
				tr = LINEERR_NOMEM;
				break;		// 2008.01.07 추가 V01.02.24
			}
		}

		// Fill in the buffer
		tr = ::lineGetID(m_hLine,				// hLine
						 0,						// dwAddressID
						 m_hCall,				// hCall
						 LINECALLSELECT_CALL,	// dwSelect
						 *ppVarString,			//
						 L"comm/datamodem");		// DeviceClass

		// Check how much memory we need
		// (some TSPs succeed even if the data size is too small)
		if (tr == LINEERR_STRUCTURETOOSMALL)
			dwNeededSize += 0x100;

		if ((tr == ERROR_SUCCESS) &&
			((*ppVarString)->dwTotalSize < (*ppVarString)->dwNeededSize))
		{
			dwNeededSize = (*ppVarString)->dwNeededSize;
			tr = LINEERR_STRUCTURETOOSMALL;
		}
	}
	while (tr == LINEERR_STRUCTURETOOSMALL);

	return tr;
}

int	SetModemInitialString(BOOL bRMSMode)
{
	if(bRMSMode)
	{
		CString strRMSInit = MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_RMSMODEMINITIALSTRING);		// [#468] NH AIREAT 2008.12.19 NZ 장애대응
		CString strSet = RegGetStr(_INTIALSTRING_REG, L"1");

		if(strSet.GetLength() > 4 &&  strSet.Right(4) == "<cr>")
			strSet = strSet.Left(strSet.GetLength() - 4);

		//Registry update한다.
		if(strSet != strRMSInit)
		{
			strRMSInit += "<cr>";
			RegSetStr(_INTIALSTRING_REG, L"1", strRMSInit);
		}
	}
	else	//거래모드를 위한 Initial string을 설정한다.
	{
		CString strVal = MemGetStr(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_MODEMINITSTR);	// [#175] [NH] KSK 2008.04.29
		CString strSet = RegGetStr(_INTIALSTRING_REG, L"1");

		if(strSet.GetLength() > 4 &&  strSet.Right(4) == "<cr>")
			strSet = strSet.Left(strSet.GetLength() - 4);
		
		//Modem Initial String
		//현재 Registry에 있는 값과 Update하려는 값이 다른 경우에만 Reg update한다.
		if(strSet != strVal)
		{
			strVal += "<cr>";
			RegSetStr(_INTIALSTRING_REG, L"1", strVal);
		}

		//S12 register 설정.
		strSet = RegGetStr(_INTIALSTRING_REG, L"2");
		if(strSet.GetLength() > 4 &&  strSet.Right(4) == "<cr>")
			strSet = strSet.Left(strSet.GetLength() - 4);
		
		if(strSet != "ATS12=1")
		{
			strVal = "ATS12=1<cr>";
			RegSetStr(_INTIALSTRING_REG, L"2", strVal);
		}
	}
	
	return 0;
}

// [#401] [NH] KSK 2008.8.14 RMG RING COUNT CHECK LOGIC 추가
#define	RING_EVENTCHECK_INTERVAL		2

void CALLBACK LineCallbackFunc(DWORD hDevice,
							   DWORD dwMsg,
							   DWORD dwCallbackInstance,
							   DWORD dwParam1,
							   DWORD dwParam2,
							   DWORD dwParam3)
{
	static CTime	CheckRingEventTime = CTime::GetCurrentTime();
	static int		nRingCount = 0;

	switch (dwMsg)
	{
	// hCall, LINE_CALLSTATE, hCallback, CallState, CallStateDetail, CallPrivilege
	case LINE_CALLSTATE:
		DoLineCallState(hDevice, dwParam1, dwParam2, dwParam3);
		break;

	// hLine, LINE_CLOSE, hCallback, 0, 0, 0
	case LINE_CLOSE:
		NHDEBUG(1, (_T("LINECLOSE\n")));		
		break;

	// hLine, LINE_LINEDEVSTATE, hCallback, DeviceState, DeviceStateDetail1, DeviceStateDetail2
	case LINE_LINEDEVSTATE:
		NHDEBUG(1, (_T("LINEDEVSTATE\n")));	
		// [#401] [NH] KSK 2008.8.14
		if (dwParam1 == LINEDEVSTATE_RINGING)
		{
			if ((CheckRingEventTime + RING_EVENTCHECK_INTERVAL) <= CTime::GetCurrentTime())
			{
				CheckRingEventTime = CTime::GetCurrentTime();
				nRingCount++;
				if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSRINGCNT) <= nRingCount)
				{
					nRingCount = 0;
					gbNewCall = TRUE;
				}
			}
		}
		// end of [#401]
		break;

	// 0, LINE_REPLY, hCallback, idRequest, Status, 0
	case LINE_REPLY:
		NHDEBUG(1, (_T("LINEREPLY\n")));	
		break;

	// hLine, LINE_APPNEWCALL, dwInstanceData, dwAddressID, hCall, dwPrevilige
	case LINE_APPNEWCALL:
		if ((dwParam3 == LINECALLPRIVILEGE_OWNER) && (m_hLine == (HLINE)hDevice))
		{
			// [#401] [NH] KSK 2008.8.14
			CheckRingEventTime = CTime::GetCurrentTime();
			nRingCount = 0;
			nRingCount++;
			m_hCall = (HCALL)dwParam2;
			if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSRINGCNT) <= nRingCount)
			{
				nRingCount = 0;
				gbNewCall = TRUE;
			}
//			m_hCall = (HCALL)dwParam2;
//			gbNewCall = TRUE;
			// end of [#401]

			NHDEBUG(1, (_T("LINEAPPNEWCALL\n")));
		}
		break;

	default:
			NHDEBUG(1, (_T("CallBack Default Value [%x] [%x] [%x] \n"), dwParam1, dwParam2, dwParam3));
		break;
	}
}

LPLINECALLPARAMS CreateCallParams(LPLINECALLPARAMS lpCallParams,
										  LPCSTR lpszDialableAddress)
{
	size_t	sizeDialableAddress;

	if (lpszDialableAddress == NULL)
		lpszDialableAddress = "";

	sizeDialableAddress = strlen(lpszDialableAddress) + 1;

	lpCallParams = (LPLINECALLPARAMS)LocalAlloc(LPTR, sizeof(LINECALLPARAMS) + sizeDialableAddress);
	if (lpCallParams == NULL)
		return NULL;

    lpCallParams->dwTotalSize = sizeof(LINECALLPARAMS) + sizeDialableAddress;

	lpCallParams->dwBearerMode = LINEBEARERMODE_VOICE;
	lpCallParams->dwMediaMode  = LINEMEDIAMODE_DATAMODEM;

	// [2ND] AIREAT 2009.04.18 : DOVE는 BLIND DIAL이 ENABLE 되어야 한다.
#if (_WIN32_WCE < 0x600)
	lpCallParams->dwCallParamFlags = LINECALLPARAMFLAGS_IDLE;
#endif
                                
	lpCallParams->dwAddressMode = LINEADDRESSMODE_ADDRESSID;
	lpCallParams->dwAddressID = 0;

	lpCallParams->dwOrigAddressSize = 0;
	lpCallParams->dwOrigAddressOffset = 0;

	(lpCallParams->DialParams).dwDialSpeed = 0;
	(lpCallParams->DialParams).dwDigitDuration = 0;
	(lpCallParams->DialParams).dwDialPause = 0;
	(lpCallParams->DialParams).dwWaitForDialtone = 0;

	lpCallParams->dwDisplayableAddressOffset = sizeof(LINECALLPARAMS);
	lpCallParams->dwDisplayableAddressSize = sizeDialableAddress;
	strcpy((LPSTR)lpCallParams + sizeof(LINECALLPARAMS), lpszDialableAddress);

	return lpCallParams;
}
