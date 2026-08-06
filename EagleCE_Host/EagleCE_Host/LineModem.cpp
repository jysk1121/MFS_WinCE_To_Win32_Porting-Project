/** ***************************************************************
*	@file LineModem.cpp
*	@date 2017/10/20
*	@author MFS
*	@brief 실질적인 통신을 수행하는 모뎀라인을 구현한 소스파일입니다. 
*******************************************************************/
#include "stdafx.h"
#include "LineModem.h"

#include "Queue.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"

//------------------------------------------------------------------
//	Define extern function
//------------------------------------------------------------------
extern int		put_data(char *recv_data, int recv_size);
extern int		put_event(char recv_evt);

//------------------------------------------------------------------
//	Define Return Value
//------------------------------------------------------------------
#define RET_OK	0
#define RET_NG	1

#define TAPI_VERSION_1_0		0x00010003
#define TAPI_VERSION_1_4		0x00010004
#define TAPI_VERSION_2_0		0x00020000
#define TAPI_VERSION_3_0		0x00030000
#define TAPI_USE_VERSION		TAPI_VERSION_3_0

#define	RING_EVENTCHECK_INTERVAL		2

// Settable Data Bits
#define DATABIT_7        7
#define DATABIT_8        8

#define _INTIALSTRING_REG	_T("Drivers\\Unimodem\\Init")

CLineModem::CLineModem() : m_eStartCheckSerial(FALSE, FALSE), m_eStartCheckLine(FALSE, FALSE)
{
	CLineCtrl::Register(LINE_DIALUP, this);

	m_hLineApp = NULL;
	m_nLineID = -1;
	m_hCall = NULL;
	m_hComm = NULL;
	m_nMakeCallRequestID = 0;
	m_bCheckSerial = FALSE;
	m_bCheckLine = FALSE;
	m_hLineEvent = NULL;

	m_dwEvtMask = 0;
	memset(&m_CommTimeouts, 0, sizeof(m_CommTimeouts));
	memset(&m_dcb, 0, sizeof(m_dcb));

	memset(&m_CurLineInfo, 0, sizeof(m_CurLineInfo));
	memset(&m_TmpLineInfo, 0, sizeof(m_TmpLineInfo));

	// Create Thread.
	m_hLineEventThread = CreateThread(NULL,					/* Must be NULL */ 
							 0,								/* Stack size */ 
							 LineEventThreadProc,			/* Thread Start Routine */ 
							 (void*)this,					/* Thread Parameter */ 
							 0,								/* Creation Flag */ 
							 &m_nLineEventThread);			/* Thread ID */ 

}

CLineModem::~CLineModem()
{
	m_bExitThread = TRUE;

	Sleep(50);	

	// 스레드를 종료한다.
	if (m_hLineEventThread != NULL)
	{
		::TerminateThread(m_hLineEventThread, 0);
	}
}

int	CLineModem::LineOpen(LPCTSTR Dest_Info,		// Phone Number
						 LPCTSTR dest_port,		// not used
						 LPCTSTR option,		// option: "1" - ams init string, "0" - host init string 
						 LPCTSTR mode)			// mode: "1" - ready for ams, "0" - host or ams connect mode
{
	//LOG(Info, _T("LineOpen Call (Host : %s, Mode : %s)"), Dest_Info, mode);

	long	dwReturn = 0;
	BOOL	bAMSInitString = FALSE;
	BOOL	bDeviceTest = FALSE;
	BOOL	bOutgoingCall = ((mode[0] == '0') ? TRUE : FALSE);


	if (option[0] == '1')
		bAMSInitString = TRUE;

	// Host Line Test는 추후 대응
	//if (option == Int2Asc(MODEM_HOST_DEV_TEST) || option == Int2Asc(MODEM_RMS_DEV_TEST))
	//	bDeviceTest = TRUE;

	// SetInitialString
	SetModemInitString(bAMSInitString);

	m_bConnectStatus = FALSE;

	// Initialize TAPI
	if (InitializeTAPI() == FALSE)
	{
		ClosePhoneLine();
		LOG(Error, _T("InitializeTAPI failed InitString(%d)"), bAMSInitString);

		return 12;		// Initialize fail
	}

	// Open the current line.
	if ((dwReturn = OpenPhoneLine()) != 0)
	{
		ClosePhoneLine();
		LOG(Error, _T("OpenPhoneLine failed err(0x%08X) RMSInitString(%d)"), dwReturn, bAMSInitString);

		return 13;	// lineOpen fail
	}

	m_bRMSMode = bAMSInitString;
	m_bOutgoingCall = bOutgoingCall;

	// Make Call
	if (bOutgoingCall == TRUE && bDeviceTest == FALSE)
	{
		// We want to be notified for everything
		if ((dwReturn = lineSetStatusMessages(m_CurLineInfo.hLine, 0x1FFFFFF, 0)) != 0)
		{
			ClosePhoneLine();
			LOG(Error, _T("lineSetStatusMessages failed err(0x%08X)"), dwReturn);

			return 13;	// lineOpen fail
		}

		if ((dwReturn = MakePhoneCall(Dest_Info)) <= 0)
		{
			ClosePhoneLine();
			LOG(Error, _T("MakePhoneCall failed err(0x%08X)"), dwReturn);

			/* failed in MakePhoneCall() */
			switch (dwReturn)
			{
			case LINEERR_ADDRESSBLOCKED:		return 15;
			case LINEERR_INVALTERMINALID:		return 16;
			case LINEERR_CALLUNAVAIL:			return 17;

			case LINEERR_DIALBILLING:
			case LINEERR_DIALDIALTONE:
			case LINEERR_DIALPROMPT:
			case LINEERR_DIALQUIET:				return 18;

			case LINEERR_INUSE:
			case LINEERR_INVALADDRESS:
			case LINEERR_INVALADDRESSID:
			case LINEERR_INVALADDRESSMODE:
			case LINEERR_INVALBEARERMODE:
			case LINEERR_INVALCALLPARAMS:
			case LINEERR_INVALCOUNTRYCODE:		return 19;
			
			case LINEERR_OPERATIONFAILED:		return 20;
			case LINEERR_RESOURCEUNAVAIL:		return 21;

			default:							return 14;
			}
		}

		// Start Thread to Check Line Event
		m_bCheckLine = TRUE;
		m_eStartCheckLine.Set();
	}
	else if (bOutgoingCall == FALSE && bDeviceTest == FALSE)
	{
		// We want to be notified for everything
		if ((dwReturn = lineSetStatusMessages(m_CurLineInfo.hLine, 0x1FFFFFF, 0)) != 0)
		{
			ClosePhoneLine();
			LOG(Error, _T("lineSetStatusMessages failed err(0x%08X)"), dwReturn);

			return 13;	// lineOpen fail
		}

		// Start Thread to Check Line Event
		m_bCheckLine = TRUE;
		m_eStartCheckLine.Set();
	}

	return RET_OK;	// OK.
}

int	CLineModem::LineClose()
{
	//LOG(Info, _T("LineClose Call"));

	m_bConnectStatus = FALSE;

	ClosePhoneLine();

	return RET_OK;	// OK.
}

int	CLineModem::LineSendData(BYTE *send_data, int len)
{

	DWORD	dwErrorFlags;
	COMSTAT	comstate;
	DWORD	dwEvent;
	DWORD	dwNumberOfBytesWritten;

	//LOG(Info, _T("LineSendData Call (%d)"), len);

	if (m_hComm == NULL)
	{
		LOG(Error, _T("LineSendData m_hComm is null"));
		return RET_NG;
	}

	::PurgeComm(m_hComm, PURGE_TXABORT | PURGE_TXCLEAR);

   	ClearCommError(m_hComm, &dwErrorFlags , &comstate);

	if (WriteFile(m_hComm, send_data, len, &dwNumberOfBytesWritten, NULL) == FALSE)
	{
		LOG(Error, _T("WriteFile error (%d)"), dwNumberOfBytesWritten);
		ClosePhoneLine();

		return RET_NG;
	}


	WaitCommEvent(m_hComm, &dwEvent, NULL);
	ClearCommError(m_hComm, &dwErrorFlags , &comstate);

	//LOG(Info, _T("LineSendData Call end"));

	return RET_OK;	// OK.
}

int CLineModem::LineAccept(BYTE opt)				/*accept option: '0x01' - call accept, '0x00' - call deny*/
{
	if (opt && m_bNewCall)
	{
		if (Answer() == 0)
		{
			return RET_OK;
		}
	}

	return RET_NG;
}

int CLineModem::LineCallState(int nMode)
{
	if (nMode == 0)
	{
		if (m_bNewCall == TRUE)
			return RET_OK;
		else
			return RET_NG;
	}

	return m_bConnectStatus;
}

BOOL CLineModem::SetupComPort(HANDLE hComm, BOOL bRMSMode)
{

	DWORD			dwErrorFlags;
	COMSTAT			comstate;

	//LOG(Info, _T("SetupComPort Call (%d)"), bRMSMode);

	// only clear error state.
	ClearCommError(hComm, &dwErrorFlags , &comstate);

	if (::GetCommMask(hComm, &m_dwEvtMask) == FALSE)
	{
		LOG(Error, _T("GetCommMask failed err(0x%08X)"), GetLastError());
		return FALSE;
	}

	if (::GetCommState(hComm, &m_dcb) == FALSE)
	{
		LOG(Error, _T("GetCommState failed err(0x%08X)"), GetLastError());
		return FALSE;
	}

	if (::GetCommTimeouts(hComm, &m_CommTimeouts) == FALSE)
	{
		LOG(Error, _T("GetCommTimeouts failed err(0x%08X)"), GetLastError());
		return FALSE;
	}

	// Set the comm mask so we receive error signals.
	if (::SetCommMask(hComm, EV_RXCHAR | EV_ERR |EV_BREAK |EV_TXEMPTY) == FALSE)
	{
		LOG(Error, _T("SetCommMask failed err(0x%08X)"), GetLastError());
		return FALSE;
	}

	m_dcb.DCBlength	= sizeof(DCB);
	m_dcb.fBinary		= TRUE;
	m_dcb.fParity		= TRUE;
	m_dcb.fErrorChar	= FALSE;
	m_dcb.fNull			= FALSE;
	m_dcb.fAbortOnError = FALSE;

	// RTS / CTS / DSR / DTR을 명시적으로 제어해줘야 함
	m_dcb.fRtsControl = RTS_CONTROL_DISABLE;
	m_dcb.fDtrControl = DTR_CONTROL_DISABLE;
	m_dcb.fOutxCtsFlow = 0;
	m_dcb.fOutxDsrFlow = 0;

	if (bRMSMode == FALSE)
	{
		// ATM position
		m_dcb.ByteSize	= DATABIT_7;
		m_dcb.Parity	= EVENPARITY;
		m_dcb.StopBits	= ONESTOPBIT;
	}
	else
	{
		// AMS position
		m_dcb.ByteSize	= DATABIT_8;
		m_dcb.Parity	= NOPARITY;
		m_dcb.StopBits	= ONESTOPBIT;
	}
	
	if (::SetCommState(hComm, &m_dcb) == FALSE)
	{
		LOG(Error, _T("SetCommState failed err(0x%08X)"), GetLastError());
		return FALSE;
	}

	// Serial Read / Write 검증 완료
	//m_CommTimeouts.ReadIntervalTimeout = 100;
	//m_CommTimeouts.ReadTotalTimeoutMultiplier = 3;
	//m_CommTimeouts.ReadTotalTimeoutConstant = 30;
	//
	//m_CommTimeouts.WriteTotalTimeoutMultiplier = 50;
	//m_CommTimeouts.WriteTotalTimeoutConstant = 150;

	m_CommTimeouts.ReadIntervalTimeout = MAXDWORD;
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	m_CommTimeouts.ReadTotalTimeoutConstant = 0;

	m_CommTimeouts.WriteTotalTimeoutMultiplier = 10;
	m_CommTimeouts.WriteTotalTimeoutConstant = 100;

	if (::SetCommTimeouts(hComm, &m_CommTimeouts) == FALSE)
	{
		LOG(Error, _T("SetCommTimeouts failed err(0x%08X)"), GetLastError());
		return FALSE;
	}

	if (::PurgeComm(hComm, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR) == FALSE)
	{
		LOG(Error, _T("PurgeComm failed err(0x%08X)"), GetLastError());
		return FALSE;
	}

	//LOG(Info, _T("SetupComPort Call end (%d)"), bRMSMode);

	return TRUE;
}

VOID CLineModem::SetModemInitString(BOOL bRmsInitString)
{
	//LOG(Info, _T("SetModemInitString Call (%d)"), bRmsInitString);

	if (bRmsInitString == FALSE)
	{
		// HOST를 위한 Initial string을 설정한다.
		CString strVal = m_EagleConfig.LoadHostData(_T("setup_string"));
		CString strSet = CUtil::String_GetRegistry(_INTIALSTRING_REG, _T("1"));

		strVal.TrimLeft();
		strVal.TrimRight();

		if(strSet.GetLength() > 4 &&  strSet.Right(4) == _T("<cr>"))
			strSet = strSet.Left(strSet.GetLength() - 4);
		
		//Modem Initial String
		//현재 Registry에 있는 값과 Update하려는 값이 다른 경우에만 Reg update한다.
		if(strSet != strVal)
		{
			strVal += "<cr>";
			LOG(Info, _T("Set HOST initial String [%s]"), strVal);
			CUtil::String_SetRegistry(_INTIALSTRING_REG, _T("1"), strVal);
		}

		//S12 register 설정.
		strSet = CUtil::String_GetRegistry(_INTIALSTRING_REG, _T("2"));

		if(strSet.GetLength() > 4 &&  strSet.Right(4) == _T("<cr>"))
			strSet = strSet.Left(strSet.GetLength() - 4);
		
		if(strSet != _T("ATS12=1"))
		{
			strVal = _T("ATS12=1<cr>");

			LOG(Info, _T("Set S12 Register [%s]"), strVal);
			CUtil::String_SetRegistry(_INTIALSTRING_REG, _T("2"), strVal);
		}

		// Country Option Set
		//strSet = CUtil::String_GetRegistry(_INTIALSTRING_REG, _T("3"));

		//if(strSet.GetLength() > 4 &&  strSet.Right(4) == _T("<cr>"))
		//	strSet = strSet.Left(strSet.GetLength() - 4);

		//if(strSet != _T("AT+GCI=61"))	// Korea Code
		//{
		//	strVal = _T("AT+GCI=61<cr>");

		//	LOG(Info, _T("Set Contry Option Register [%s]"), strVal);
		//	CUtil::String_SetRegistry(_INTIALSTRING_REG, _T("3"), strVal);
		//}


	}
	else
	{
		// AMS를 위한 Initial string을 설정한다.
		CString strRMSInit = _T("AT&FE0");
		CString strSet = CUtil::String_GetRegistry(_INTIALSTRING_REG, _T("1"));

		strRMSInit.TrimLeft();
		strRMSInit.TrimRight();

		if(strSet.GetLength() > 4 &&  strSet.Right(4) == _T("<cr>"))
			strSet = strSet.Left(strSet.GetLength() - 4);

		//Registry update한다.
		if(strSet != strRMSInit)
		{
			strRMSInit += "<cr>";

			LOG(Info, _T("Set AMS initial String [%s]"), strRMSInit);
			CUtil::String_SetRegistry(_INTIALSTRING_REG, _T("1"), strRMSInit);
		}
	}
}

BOOL CLineModem::InitializeTAPI()
{
	LOG(Info, _T("InitializeTAPI Call"));

	DWORD			dwNumDevs	= 0;
	LONG			lReturn		= 0;
	DWORD			dwAPIVersion	= 0;
	BOOL			bFoundLineDev = FALSE;

	LINEINITIALIZEEXPARAMS	LineInitializeExParams;

	LineInitializeExParams.dwTotalSize = sizeof(LINEINITIALIZEEXPARAMS);
	LineInitializeExParams.dwNeededSize = sizeof(LINEINITIALIZEEXPARAMS);
	LineInitializeExParams.dwUsedSize = sizeof(LINEINITIALIZEEXPARAMS);
	LineInitializeExParams.dwOptions = LINEINITIALIZEEXOPTION_USEEVENT;

	m_TmpLineInfo.dwAPIVersion = TAPI_USE_VERSION;

	// Initialize line
	lReturn = ::lineInitializeEx(&m_hLineApp,				// the application's handle for TAPI
							 NULL,							// instance handle of client application
							 NULL,							// the address of callback function
							 NULL,							// application-supplied name
							 &dwNumDevs,				
							 &m_TmpLineInfo.dwAPIVersion,
							 &LineInitializeExParams);

	// If function "lineInitialize" fails, then return.
	if (lReturn != 0)
	{
		LOG(Error, _T("LineInitialize failed. err(0x%08X)"), lReturn);
		return FALSE;
	}

	// If there is no device, then return.
    if (dwNumDevs == 0)
    {
		LOG(Error, _T("There are no line devices available."));
		return FALSE;
	}

	// Get Line Event Handle
	m_hLineEvent = LineInitializeExParams.Handles.hEvent;

	//LOG(Info, _T("m_hLineApp(%x), dwNumDevs(%d)"), m_hLineApp, dwNumDevs);

	bFoundLineDev = FALSE;
	memset(&m_CurLineInfo, 0, sizeof(m_CurLineInfo));

    // Search line dev. to want to use
    for (DWORD dwLineID = 0; dwLineID < dwNumDevs; ++dwLineID)
    {
		memset(&m_TmpLineInfo, 0, sizeof(m_TmpLineInfo));
		if (IsNeedLineDev(dwLineID, &m_TmpLineInfo) == TRUE)
		{
			//LOG(Info, _T("Found Line Dev. ID[%d]"), dwLineID);
			m_CurLineInfo = m_TmpLineInfo;
			m_nLineID = dwLineID;
			bFoundLineDev = TRUE;
		}
    }

	if (bFoundLineDev == TRUE)
	{
		LOG(Info, _T("Found Line Dev. ID[%d]"), m_nLineID);
		return TRUE;
	}

	LOG(Error, _T("Not found Line Dev. ID[%d]"), m_nLineID);

	return FALSE;
}

BOOL CLineModem::IsNeedLineDev(DWORD dwLineID, LPLINEINFO lpLineInfo)
{
	//LOG(Info, _T("IsNeedLineDev Call"));

	DWORD	dwRet = 0;
    //LPTSTR lpszLineName = NULL; 
    //LPLINEDEVCAPS lpLineDevCaps = NULL;
    
    // Negotiate the API version number. If it fails, return to dwReturn.
    if (lineNegotiateAPIVersion (m_hLineApp,                   // TAPI registration handle
		                         dwLineID,                     // Line device to be queried
								 TAPI_VERSION_1_0,             // Least recent API version 
								 TAPI_USE_VERSION,			   // Most recent API version 
								 &(lpLineInfo->dwAPIVersion),  // Negotiated API version 
								 NULL) != 0)                   // Must be NULL; the provider-
    {
		LOG(Error, _T("lineNegotiateAPIVersion failed Line ID[%x] Call"), dwLineID);
		return FALSE;
    }
    
	//LOG(Info, _T("TAPI API Version(%x)"), lpLineInfo->dwAPIVersion);

	memset(&m_LineDevCaps, 0, sizeof(m_LineDevCaps));
	m_LineDevCaps.dwTotalSize = sizeof(LINEDEVCAPS);

	if ((dwRet = lineGetDevCaps (m_hLineApp,
		                dwLineID,
						lpLineInfo->dwAPIVersion,
						0,
						&m_LineDevCaps)) != 0)
	{
		LOG(Error, _T("lineGetDevCaps failed. Line ID[%d], err[0x%08X]"), dwLineID, dwRet);
		return FALSE;
	}


	if ((m_LineDevCaps.dwBearerModes & LINEBEARERMODE_VOICE) &&
		(m_LineDevCaps.dwMediaModes & LINEMEDIAMODE_DATAMODEM) &&
		(m_LineDevCaps.dwLineFeatures & LINEFEATURE_MAKECALL))
	{
		//LOG(Info, _T("found the line device. Line ID[%d]"), dwLineID);
		return TRUE;
	}

	return FALSE;
}

DWORD CLineModem::OpenPhoneLine()
{
	LOG(Info, _T("OpenPhoneLine Call"));

	DWORD dwReturn;

	// Open the current line.
	dwReturn = lineOpen(m_hLineApp,						// a handle to the application's registration with TAPI
						m_nLineID,						// identity of the line device
						&m_CurLineInfo.hLine,			// HLINE handle
						m_CurLineInfo.dwAPIVersion,		// the API version number
						0,								// the extension version number
						0,								// user-instance data
						LINECALLPRIVILEGE_OWNER,
						LINEMEDIAMODE_DATAMODEM,		// the media type
						NULL);							// Must set to NULL for Windows CE

	//LOG(Info, _T("LineOpen - (0x%x) (0x%x) (0x%x), (%s)"), m_CurLineInfo.hLine, m_CurLineInfo.dwNumOfAddress, m_CurLineInfo.dwPermanentLineID, m_CurLineInfo.szLineName);

	return dwReturn;
}

VOID CLineModem::ClosePhoneLine()
{
	CAutoLock	Sync(m_eMethodUse);

	//LOG(Info, _T("ClosePhoneLine Call m_hCall(%X)"), m_hCall);

	m_bCheckSerial = FALSE;
	m_bCheckLine = FALSE;


	// If lineMakeCall succeeded, then drop the call.
    if (m_hCall != NULL)
    {
		LONG lRet;

		// 연결된 Port를 Close 후에 Line Drop을 수행해야 Command로 인식함
		if (m_hComm != NULL)
		{
			::PurgeComm(m_hComm,
						PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
			::SetCommMask(m_hComm, m_dwEvtMask);
			::SetCommState(m_hComm, &m_dcb);
			::SetCommTimeouts(m_hComm, &m_CommTimeouts);
			::CloseHandle(m_hComm);
			m_hComm = NULL;
		}

		CUtil::Sleep_Wait(200);

		lRet = ::lineDrop (m_hCall, NULL, 0);

		LOG(Info, _T("Call LineDrop (0x%x)"), lRet);

		m_hCall = NULL;
    }

	// close current line
	if (m_CurLineInfo.hLine != NULL)
	{
		CUtil::Sleep_Wait(50);
		::lineClose(m_CurLineInfo.hLine);
		m_CurLineInfo.hLine = NULL;
	}

	// close handle for TAPI
	if (m_hLineApp != NULL)
	{
		CUtil::Sleep_Wait(50);
		::lineShutdown(m_hLineApp);
		m_hLineApp = NULL;
	}

	m_hLineEvent = NULL;
	m_bNewCall = FALSE;
}

long CLineModem::MakePhoneCall(LPCTSTR lpszPhoneNum)
{
	LOG(Info, _T("MakePhoneCall Call PhoneNumber(%s)"), lpszPhoneNum);

	LONG				tr = ERROR_SUCCESS;
	LINECALLPARAMS		LineCallParams;

	// Initialize g_MakeCallRequestID.
    m_nMakeCallRequestID = 0;

	// Setup our CallParams for DATAMODEM settings.
	memset(&LineCallParams, 0, sizeof(LINECALLPARAMS));

    LineCallParams.dwTotalSize = sizeof(LINECALLPARAMS) + 1;

	LineCallParams.dwBearerMode = LINEBEARERMODE_VOICE;
	LineCallParams.dwMediaMode  = LINEMEDIAMODE_DATAMODEM;                              
	LineCallParams.dwAddressMode = LINEADDRESSMODE_ADDRESSID;
	LineCallParams.dwAddressID = 0;

	LineCallParams.dwOrigAddressSize = 0;
	LineCallParams.dwOrigAddressOffset = 0;

	LineCallParams.DialParams.dwDialSpeed = 0;
	LineCallParams.DialParams.dwDigitDuration = 0;
	LineCallParams.DialParams.dwDialPause = 0;
	LineCallParams.DialParams.dwWaitForDialtone = 0;

	LineCallParams.dwDisplayableAddressOffset = sizeof(LINECALLPARAMS);
	LineCallParams.dwDisplayableAddressSize = 1;


	// Make the phone call. lpCallParams should be NULL if the default 
    // call setup parameters are requested.
	m_nMakeCallRequestID = ::lineMakeCall(m_CurLineInfo.hLine,
										 &m_hCall,
										 lpszPhoneNum,
										 NULL,
										 &LineCallParams);

	if (m_nMakeCallRequestID > 0)
	{
		//LOG(Info, _T("MakePhoneCall Call OK"));
	}

	// If the make call did not succeed but the line was opened, 
    // then close it.
    if ((m_nMakeCallRequestID <= 0) && (m_CurLineInfo.hLine))
	{
		LOG(Error, _T("MakePhoneCall Call Fail"));
		ClosePhoneLine();
		return LINEERR_CALLUNAVAIL;
	}
	
	return m_nMakeCallRequestID;
}

HANDLE CLineModem::GetComportHandle(DWORD *lpRet)
{
	//LOG(Info, _T("GetComportHandle Call"));

	char	szTemp[sizeof(VARSTRING)+255];

	VARSTRING	*pvarStrDevID = (VARSTRING *)szTemp;

	memset(pvarStrDevID, 0, sizeof(VARSTRING)+255);
	pvarStrDevID->dwTotalSize = sizeof(VARSTRING)+255;
	
	long lRet = lineGetID(m_CurLineInfo.hLine,
						  0,
						  m_hCall,
						  LINECALLSELECT_LINE,
						  pvarStrDevID,
						  _T("comm/datamodem"));
	if(lRet)
	{
		*lpRet = lRet;
		return NULL;
	}

	*lpRet = 0;

	return *((LPHANDLE)((char *)pvarStrDevID + pvarStrDevID->dwStringOffset));
}

DWORD CLineModem::Answer()
{
	LOG(Info, _T("Answer Call"));

	DWORD	tr = ERROR_SUCCESS;
	DWORD	nRet = 0;

	// 추후 리턴값 상세분류를 위해 switch를 사용.
	tr = ::lineAnswer(m_hCall, NULL, 0);
	if (tr < 0)
	{
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

	return nRet;
}

unsigned CLineModem::ThreadHandlerProc(void)	// 스레드의 Handler.
{
	//LOG(Info, _T("ThreadHandlerProc Call"));

	DWORD			dwRead;
	BOOL			bCloseLine = FALSE;
	DWORD			dwCommEvent;

	m_bExitThread = FALSE;

	while(m_bExitThread != TRUE)
	{
		bCloseLine = FALSE;

		// wait for start signal
		if (m_eStartCheckSerial.Wait(10) != WAIT_OBJECT_0)
			continue;

		while(m_bCheckSerial == TRUE)
		{
			if (WaitCommEvent(m_hComm, &dwCommEvent, NULL) == TRUE)
			{
				// we receive EV_RXCHAR event.
				if (dwCommEvent & EV_RXCHAR)
				{
					do
					{
						// Recv Buffer 초기화
						memset(m_RecvBuf, 0, sizeof(m_RecvBuf));

						if (ReadFile(m_hComm, m_RecvBuf, sizeof(m_RecvBuf), &dwRead, NULL) == TRUE)
						{
							if (dwRead > 0)
							{
								put_data(m_RecvBuf, dwRead);
							}
						}
						else
						{
							LOG(Error, _T("Failed in ReadFile(). err(0x%08X)"), GetLastError());
							bCloseLine = TRUE;
							break;
						}

					} while(dwRead > 0);
				}
			}
			else
			{
				LOG(Error, _T("Failed in WaitCommEvent(). err(0x%08X)"), GetLastError());
			}

			if (bCloseLine == TRUE)
				break;
		}

		if (bCloseLine == TRUE)
		{
			m_bConnectStatus = FALSE;
			put_event(DISCONNECT_EVT);
			ClosePhoneLine();
		}
	}

	return 0;
}

DWORD _stdcall CLineModem::LineEventThreadProc( void *pThreadHandler )
{
	CLineModem *pcHandler = static_cast<CLineModem*> (pThreadHandler);

	return pcHandler->LineEventThread();
}

unsigned CLineModem::LineEventThread(void)
{
	DWORD			dwReturn;
	LINEMESSAGE		LineMsg;
	BOOL			bCloseLine;

	m_bExitThread = FALSE;

	while (m_bExitThread != TRUE)
	{
		// wait for start signal
		if (m_eStartCheckLine.Wait(10) != WAIT_OBJECT_0)
			continue;

		while (m_bCheckLine == TRUE)
		{
			bCloseLine = FALSE;

			// Get a TAPI event if available, wait for 10ms just enough to give up quontum.
			switch(WaitForSingleObject(m_hLineEvent, 10))
			{
			case WAIT_OBJECT_0:
				{
					//TAPI's got something
					if ((dwReturn = lineGetMessage(m_hLineApp, &LineMsg, 0)) != 0)
					{
						bCloseLine = TRUE;
						break;
					}

					// Process the return msg
					switch (LineMsg.dwMessageID)
					{
					case LINE_CALLSTATE:
						switch (LineMsg.dwParam1)
						{
						case LINECALLSTATE_ACCEPTED:
							LOG(Info, _T("Receive State : [LINECALLSTATE_ACCEPTED]"));
							break;

						case LINECALLSTATE_OFFERING:
							LOG(Info, _T("Receive State : [LINECALLSTATE_OFFERING]"));
							break;
						case LINECALLSTATE_IDLE:
							LOG(Info, _T("Receive State : [LINECALLSTATE_IDLE]"));
							bCloseLine = TRUE;
							break;
						case LINECALLSTATE_BUSY:
							LOG(Info, _T("Receive State : [LINECALLSTATE_BUSY]"));
							put_event(USER_LINEBUSY);
							bCloseLine = TRUE;
							break;
						case LINECALLSTATE_SPECIALINFO:
							LOG(Info, _T("Receive State : [LINECALLSTATE_SPECIALINFO]"));
							bCloseLine = TRUE;
							break;

						case LINECALLSTATE_CONNECTED:
							{
								LOG(Info, _T("Receive State : [LINECALLSTATE_CONNECTED]"));
								DWORD			lRet;

								// Get Handle
								m_hComm = GetComportHandle(&lRet);
								if (lRet != 0)
								{
									LOG(Error, _T("GetComportHandle Failed. err(0x%08X)"), lRet);
									bCloseLine = TRUE;
									break;
								}

								if (SetupComPort(m_hComm, m_bRMSMode) == FALSE)
								{
									LOG(Error, _T("SetupComPort Failed. err(0x%08X)"), lRet);
									bCloseLine = TRUE;
									break;
								}

								// start Thread to check serial
								m_bCheckSerial = TRUE;
								m_eStartCheckSerial.Set();

								m_bConnectStatus = TRUE;
								put_event(CONNECT_EVT);
							}
							break;

						case LINECALLSTATE_DISCONNECTED:
							{
								LOG(Info, _T("Receive State : [LINECALLSTATE_DISCONNECTED]"));
								bCloseLine = TRUE;

								m_bConnectStatus = FALSE;

								switch (LineMsg.dwParam2)
								{
								case LINEDISCONNECTMODE_BUSY:			put_event(USER_LINEBUSY);	break;
								case LINEDISCONNECTMODE_NOANSWER:		put_event(USER_NOANSWER);	break;
								case LINEDISCONNECTMODE_NODIALTONE:		put_event(USER_NODIALTONE);	break;
								}
							}
							break;
						}
						break;	// case LINE_CALLSTATE:

					case LINE_LINEDEVSTATE:
						switch (LineMsg.dwParam1)
						{
						case LINEDEVSTATE_RINGING:
							{
								LOG(Info, _T("Receive State : [LINEDEVSTATE_RINGING]"));

								//if ((m_CheckRingEventTime + RING_EVENTCHECK_INTERVAL) <= CTime::GetCurrentTime() && m_bNewCall == FALSE)
								//{
								//	m_CheckRingEventTime = CTime::GetCurrentTime();
								//	m_nRingCount++;
								//	if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSRINGCNT) <= m_nRingCount)
								//	{
								//		LOG(Info, _T("New Call Detected"));
								//		m_nRingCount = 0;
								//		m_bNewCall = TRUE;
								//	}
								//}
							}
							break;
							
						case LINEDEVSTATE_OUTOFSERVICE:
						case LINEDEVSTATE_DISCONNECTED:
						case LINEDEVSTATE_MAINTENANCE:
							LOG(Info, _T("Receive State : [LINEDEVSTATE_DISCONNECTED]"));
							bCloseLine = TRUE;
							break;
						}
						break;	// case LINE_LINEDEVSTATE:
					case LINE_REPLY:
						{
							// Reply from the lineMakeCall function.
							if (m_nMakeCallRequestID == LineMsg.dwParam1)
							{
								// If an error occurred on making the call.
								if (LineMsg.dwParam2 != 0)
								{
									LOG(Error, _T("Received to fail in lineMakeCall"));
									bCloseLine = TRUE;
								}
							}
						}
						break;  // case LINE_REPLY:

					case LINE_APPNEWCALL:
						{
							LOG(Info, _T("Receive State : [LINE_APPNEWCALL]"));

							if (LineMsg.dwParam3 == LINECALLPRIVILEGE_OWNER && m_CurLineInfo.hLine == (HLINE)LineMsg.hDevice && m_bNewCall == FALSE)
							{
								m_CheckRingEventTime = CTime::GetCurrentTime();
								m_nRingCount = 1;
								m_hCall = (HCALL)LineMsg.dwParam2;

								//if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSRINGCNT) <= m_nRingCount)
								{
									LOG(Info, _T("New Call Detected"));
									m_nRingCount = 0;
									m_bNewCall = TRUE;
								}
							}
						}
						break;	// case LINE_APPNEWCALL:
					}
				}
				break;
			}

			if (bCloseLine == TRUE)
			{
				m_bConnectStatus = FALSE;
				put_event(DISCONNECT_EVT);
				LineClose();
				break;
			}

		} 
		
	}

	return 0;
}