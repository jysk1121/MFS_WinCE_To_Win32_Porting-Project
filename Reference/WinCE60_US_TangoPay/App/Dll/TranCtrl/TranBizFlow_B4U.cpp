#include "stdafx.h"
#if(APP_B4U)

#include <windows.h>

#include ".\Scr\ScrCtrl.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"
#include ".\Tran\B4UService.h"

// [#] US William 2019.08.27
//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

#define NH_DEBUG		// Debug Message On/Off	

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

#define RECEIPT_PAPER	0
#define RECEIPT_DISPLAY	1

/////////////////////////////////////////////////////////////////////
#define B4U_TEST_MODE	0                                          //
// 0 = OFF, 1 = ON			                                       //
// CLEAR key should be enabled on AP.dat file (171 screen) as well //
/////////////////////////////////////////////////////////////////////

const int B4U_POLL_INTERNVAL = 1000 * 5; // 5 Seconds

// Transaction prestages
typedef int B4U_PRE_STAGE;
const B4U_PRE_STAGE	B4U_PRE_STAGE_CONTINUE_POLL =	0;
const B4U_PRE_STAGE	B4U_PRE_STAGE_ERROR =			1;
const B4U_PRE_STAGE	B4U_PRE_STAGE_RECEIVED =		2;

// Transaction stages
typedef int B4U_STAGE;
const B4U_STAGE	B4U_STAGE_CONTINUE_POLL =	0;
const B4U_STAGE	B4U_STAGE_ERROR =			1;
const B4U_STAGE	B4U_STAGE_PAID =			2;

#define END_POLL_ERROR_RELEASE \
	ReleaseMutex(g_hB4UVHandle); \
	Sleep(B4U_POLL_INTERNVAL*2); \
	continue

#define END_POLL_IF_B4U_DISABLED \
	if( !tranCmn->IsB4UEnabled() ){ return 2; }

//------------------------------------------------------------------
//	Define Symbol
//------------------------------------------------------------------
extern CTimeCheck	g_TimeCheck;
extern CDevCmn*		m_pDevCmn;

// Session and status mutex. Locking should be performed since this
// txn is multi-threaded
HANDLE					g_hB4UVHandle;

//------------------------------------------------------------------
//  Utility Functions
//------------------------------------------------------------------

void AddScreenData(CString &strPrintData, LPCTSTR lpszFormat, ...)
{
	WCHAR szPrintData[75] = L"";

	va_list args;
	va_start(args, lpszFormat);
//	vswprintf (g_szPrintData, lpszFormat, args);
	vswprintf_s (szPrintData, 75, lpszFormat, args);
	va_end(args);

	//strPrintData += g_szPrintData;
	strPrintData += szPrintData;
	strPrintData += FIELD_DELIMITER;
}

/**
 * This function should run as a separate thread. It will poll the B4U
 * API every x seconds. It will attempt everything in its power to ensure
 * that the session is live and updated. The function will also renew 
 * sessions that are expired or cancelled.
 * @params[in] params A pointer to an instance of CTranCmn
 */
DWORD B4UPollingThread(void *params)
{
	if (params == NULL)
	{
		return 1;
	}
	
	CString qrFilename;
	CTranCmn* tranCmn = reinterpret_cast<CTranCmn*>(params);

	END_POLL_IF_B4U_DISABLED

	if (!tranCmn->m_pDevCmn->fnAPL_GetAvailTrans())
	{
		NHDEBUG(DBG_CALL, (L"Ending polling thread because device is no longer in transaction mode\r\n"));
		return 3;
	}

	while (tranCmn->m_isB4UPolling)
	{
		// Get a lock on the session, but not longer than a loop
		DWORD waitResult = WaitForSingleObject(g_hB4UVHandle, B4U_POLL_INTERNVAL);
		switch (waitResult)
		{
		case WAIT_OBJECT_0:
			break;
		case WAIT_ABANDONED:
			NHERROR((L"Could not obtain lock on B4U session\r\n"));
			// Return to the top of the loop and try again.
			continue;
		}

		END_POLL_IF_B4U_DISABLED

		if (tranCmn->m_B4USession == NULL)
		{
			bool created;
			if (!tranCmn->CreateNewB4USession(created, B4U_SESS_TYPE_STAGE))
			{
				NHERROR((L"B4U Polling: could not start session\r\n"));
				END_POLL_ERROR_RELEASE;
			}

			if (!tranCmn->DisplayB4UQrCodeIfEnabled(qrFilename))
			{
				NHERROR((L"B4U Polling: could not save QR code\r\n"));
				END_POLL_ERROR_RELEASE;
			}
		}

		if (tranCmn->m_shouldB4UDisplayQRUpdates && !qrFilename.IsEmpty())
		{
			tranCmn->m_pDevCmn->fnAPL_DisplayQRCodeOnScreen(qrFilename);
		}

		if (!tranCmn->UpdateB4USessionStatus(tranCmn->m_B4USession->Type))
		{
			NHERROR((L"B4U session status call failed\r\n"));
			END_POLL_ERROR_RELEASE;
		}

		LPB4USessionStatus status = tranCmn->m_B4USessionStatus;

		// We're done modifying things, now...
		ReleaseMutex(g_hB4UVHandle);

		if (status->Status == B4U_NOT_PAID)
		{
			Sleep(B4U_POLL_INTERNVAL);
			continue;
		}

		NHDEBUG(DBG_CALL, (L"B4U session is at unknown state: %s. Stopping the polling.\r\n", status->Status));

		// The B4U status does not warrant polling now...
		break;
	}

	NHDEBUG(DBG_CALL, (L"Ending polling thread\r\n"));

	tranCmn->m_isB4UPolling = false;
	return 0;
}

B4U_STAGE GetTransactionFlowStage(CB4UService* service, LPB4USessionStatus status, B4USessionType sessionType)
{
	// If no status, continue
	if (status == NULL)
	{
		if (sessionType == B4U_SESS_TYPE_PRESTAGE)
		{
			return B4U_PRE_STAGE_CONTINUE_POLL;
		}
		else if (sessionType == B4U_SESS_TYPE_STAGE)
		{
			return B4U_STAGE_CONTINUE_POLL;
		}
		else
		{
			return B4U_STAGE_ERROR;
		}
	}

	// Error. Cancel TXN
	if (status->HasError())
	{
		return B4U_STAGE_ERROR;
	}

	// Check status update
	if (sessionType == B4U_SESS_TYPE_PRESTAGE)
	{
		if (status->Status == B4U_PRE_RECEVIED)
		{
			return B4U_PRE_STAGE_RECEIVED;
		}
	}
	else if (sessionType == B4U_SESS_TYPE_STAGE)
	{
		if (status->Status == B4U_NOT_PAID)
		{
			return B4U_STAGE_CONTINUE_POLL;
		}

			// TXN Approved!
		if ( (status->Status == B4U_PAID) || (status->Status == B4U_CONFIRMED) )
		{
			return B4U_STAGE_PAID;
		}
	}

	// Something isn't right. Cancel the TXN
	return B4U_STAGE_ERROR;
}

//------------------------------------------------------------------
//	Implement
//------------------------------------------------------------------
#if(APP_B4U)
BOOL CTranCmn::CreateNewB4USession(bool& created, B4USessionType sessionType)
{
	CString sDate, sTime;
	created = false;

	if (!this->IsB4UEnabled())
	{
		return false;
	}

	// Don't re-create a valid session
	if ( this->m_B4USessionStatus != NULL
	  && (((sessionType == B4U_SESS_TYPE_PRESTAGE) && (this->m_B4USessionStatus->Status == B4U_PRE_WAITING)) ||
		  ((sessionType == B4U_SESS_TYPE_STAGE) && (this->m_B4USessionStatus->Status == B4U_NOT_PAID))) )
	{
		return true;
	}

	if (this->m_B4USession == NULL)
		this->m_B4USession = new B4USession;

	this->m_B4USession->Type = sessionType;

	// Generate transaction ID, ReqDateTime, Amount
	if (this->m_B4USession->TerminalId.GetLength() <= 0)
	{
		this->m_B4USession->TerminalId			= MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID).Trim();	// [#RWC6-115] B4U: API Changes
	}

	if (sessionType == B4U_SESS_TYPE_STAGE)
	{
		sDate = GetDate();
		sTime = GetTime();	
		this->m_B4USession->TransactionId.Format(L"%s-%s%s", this->m_B4USession->TerminalId, sDate, sTime);
		this->m_B4USession->ReqDateTime.Format(L"%2s/%2s/%4s %2s:%2s:%2s", sDate.Mid(4,2), sDate.Right(2), sDate.Left(4), sTime.Left(2), sTime.Mid(2,2), sTime.Right(2));
		this->m_B4USession->Amount				= Asc2Int(m_sUserSelection.strMoney);
		this->m_B4USession->IsCallbackNeeded	= TRUE;
		this->m_B4USession->Currency			= m_pDevCmn->m_strCurrencyID;
	}
	else if (sessionType == B4U_SESS_TYPE_REPORT)
	{
		if ((this->m_B4UJournal->DispenseResult == B4U_DISP_RESULT_FULL)||(this->m_B4UJournal->DispenseResult == B4U_DISP_RESULT_PARTIAL))	// partial = true for now, B4U has to provide us to handle htis partial
			this->m_B4USession->Disbursed = L"true";
		else
			this->m_B4USession->Disbursed = L"false";

		this->m_B4USession->SequenceNumber		= MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SEQUENCENO);
		this->m_B4USession->IsCallbackNeeded	= FALSE;
		this->m_B4USession->RequestedAmount		= MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT);
		this->m_B4USession->DispensedAmount		= MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT);
	}

	// Register the session
	if (!m_B4U->RequestNewSession(*(this->m_B4UIni), *(this->m_B4USession)))
	{
		NHERROR((L"B4U opportunity registration failed\r\n"));
		return false;
	}

	NHDEBUG(DBG_INFO, (L"Finished getting session data from B4U: session %s\r\n", this->m_B4USession->TransactionId));

	created = true;
	return true;
}

BOOL CTranCmn::UpdateB4USessionStatus(B4USessionType sessionType)
{
	if (!this->IsB4UEnabled())
	{
		return false;
	}

	if (this->m_B4USession == NULL)
	{
		if (sessionType == B4U_SESS_TYPE_PRESTAGE)
		{
			this->m_B4USession = new B4USession;
			this->m_B4USession->Type = sessionType;
			this->m_B4USession->TerminalId			= MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID).Trim();
		}
		else
		{// No session, so we can't update the status
			return false;
		}
	}

	if (this->m_B4USessionStatus == NULL)
	{
		this->m_B4USessionStatus = new B4USessionStatus;
	}

	CString previousStatus = m_B4USessionStatus->Status;

	// Get the current session status
	if (!m_B4U->GetSessionStatus(*(this->m_B4USession), sessionType, *(this->m_B4UIni), *(this->m_B4USessionStatus)))
	{
		NHERROR((L"An error occurred whilst getting B4U session status\r\n"));
		return false;
	}

	if (previousStatus != m_B4USessionStatus->Status)
	{
		NHDEBUG(DBG_CALL, (L"B4U Session status changed from %s => %s\r\n", previousStatus, m_B4USessionStatus->Status));
	}

	return true;
}

void CTranCmn::StartB4UPollingTimer()
{
	if (!this->IsB4UEnabled())
	{
		return;
	}

	if (m_isB4UPolling)
	{
		NHDEBUG(DBG_CALL, (L"Polling thread already running\r\n"));
		return;
	}

	g_hB4UVHandle = CreateMutex(NULL, false, L"_B4U_sess");

	this->m_isB4UPolling = true;

#ifdef UNDER_CE
	DWORD threadId;
	m_hB4UPollingThread = CreateThread(NULL, 0, B4UPollingThread, reinterpret_cast<LPVOID>(this), 0, &threadId);
	NHDEBUG(DBG_CALL, (L"Created thread id %d for polling\r\n", threadId));
#endif
}

void CTranCmn::StopB4UPollingTimer()
{
	this->m_isB4UPolling = false;

	if (g_hB4UVHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hB4UVHandle);
	}
	g_hB4UVHandle = INVALID_HANDLE_VALUE;

	if (m_hB4UPollingThread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hB4UPollingThread);
	}
	m_hB4UPollingThread = INVALID_HANDLE_VALUE;
}

void CTranCmn::ResetB4UStateData()
{
	// Acquire lock on the session
	WaitForSingleObject(g_hB4UVHandle, 30000);

	if (m_B4UIni != NULL)			delete m_B4UIni;
	m_B4UIni = NULL;
	if (m_B4UJournal != NULL)		delete m_B4UJournal;
	m_B4UJournal = NULL;
	if (m_B4USession != NULL)		delete m_B4USession;
	m_B4USession = NULL;
	if (m_B4USessionStatus != NULL)	delete m_B4USessionStatus;
	m_B4USessionStatus = NULL;
	if (m_B4U != NULL)				delete m_B4U;
	m_B4U = NULL;

	ReleaseMutex(g_hB4UVHandle);
	
	// Kill the polling thread
	this->StopB4UPollingTimer();
}

BOOL CTranCmn::IsB4UEnabled()
{
	return MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_B4U_ENABLED) == ENABLE;
}

BOOL CTranCmn::DisplayB4UQrCodeIfEnabled(CString& filename)
{
	if (!this->IsB4UEnabled())		return false;
	if (this->m_B4USession == NULL)	return false;

	// Create the QR code
	if (!m_B4U->GenerateQRCode(m_B4USession->QrData, filename))
	{
		NHERROR((L"CTranCmn::F_NH_B4UQR could not save QR code to fs\r\n"));
		return false;
	}

	m_pDevCmn->fnAPL_DisplayQRCodeOnScreen(filename);

	return true;
}

//
FLOW_ID CTranCmn::F_NH_B4U_RequestInfo() 
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_B4U_RequestInfo]\r\n"));

	// Display please wait
	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	m_pDevCmn->fnSCR_DisplayString(1, L"Connecting to B4U...");
	m_pDevCmn->fnSCR_DisplayString(3, L"");
	m_pDevCmn->fnSCR_DisplayScreen(161);

	B4USessionType sessionType = B4U_SESS_TYPE_NONE;

	// Deny RMS Connection
	m_pDevCmn->fnNET_RMSConnectClose();		// [#2405] US Justin HalCash Online

	this->m_B4UIni = new B4UIni;
	this->m_B4UJournal = new B4UJournal;
	if (m_B4U->ReadB4UDispenseHistory(*(this->m_B4UJournal)))
	{
		NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_B4U_RequestInfo] Completed loading B4UJournal.Dat\r\n"));
	}
	else
	{
		NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_B4U_RequestInfo] Failed loading B4UJournal.Dat\r\n"));
	}

	if (!m_B4U->LoadB4UConfigIni(*(this->m_B4UIni)))
	{
		NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_B4U_RequestInfo] Failed loading B4UConfig.ini\r\n"));
		return	FID_NH_B4U_FAIL_FLOWS;
	}
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_B4U_RequestInfo] Completed loading B4UConfig.ini\r\n"));

	this->m_B4U = new CB4UService(this->m_B4UIni->Host);

	// Set Default Transaction Variables
	MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID,		L"");
	m_pDevCmn->nKindOfMedia = MEDIA_MS;								// Proceed as Magnetic stripe Mode
	m_sUserSelection.nPrintReceipt = RCPT_PAPER;					// Paper Receipt
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE, WITHDRAWAL_B4U);

	// Assign Transaction Code
	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_B4U;
	m_SourceAccount = S_CHECKING;									// From Checking Account

	CString keyString;

	/////////////////////////////////////////
	// 1. Receipt Option
	/////////////////////////////////////////
	
	if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
	{
		// Set session timeout to the B4U expiration
		g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);

		// display
		m_pDevCmn->fnSCR_DisplayPrevSet(172);
		m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172101));	// Title
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172001));	// Sub Title
		m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172002));	// F5 // DISPLAY
		m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172003));	// F6 // PRINT
		m_pDevCmn->fnSCR_DisplayScreen(172, KEYIN_TIME_OUT, PIN_MENU_MODE);
		
		while (g_TimeCheck.IsElapsedTimes() == FALSE)
		{
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				keyString = m_pDevCmn->fstrSCR_GetKeyString(1);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), keyString));

				keyString.Replace(GetCurrencySymbol(), L"");
				keyString.TrimLeft();
				keyString.TrimRight();

				if (keyString == S_CANCEL || keyString == S_EXIT)
				{
					return FID_NH_B4U_CANCEL_FLOWS;
				}
				else if (keyString == S_TIMEOVER)
				{
					return FID_NH_B4U_TIMEOUT_FLOWS;
				}
				else if (keyString.GetLength() > 0)
				{
					if(keyString == L"DISPLAY")	
					{
						m_sUserSelection.nPrintReceipt = RCPT_DISPLAY;
						break;
					}
					else if(keyString == L"PRINT")	
					{
						m_sUserSelection.nPrintReceipt = RCPT_PAPER;
						break;
					}
				}
			}
			// Update copy on screen if any
			Delay_Msg(50);
		}

		if (g_TimeCheck.IsElapsedTimes())
		{
			return FID_NH_B4U_TIMEOUT_FLOWS;
		}
	}
	else
	{	
		m_sUserSelection.nPrintReceipt = RCPT_DISPLAY;
	}

	/////////////////////////////////////////
	// 2. Enter Amount
	/////////////////////////////////////////
	// Set requested amount
	g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
	CString strTemp = L"";
	CString strTemp2 = L"";
	int nRetry = 5;
	int	nInputAmount = 0;
	BOOL		bShowScreen = TRUE;
	int nB4UMin = Asc2Int(m_B4UIni->MinimumDisplay)/100;

	while ( (g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetry > 0) )
	{
		if (bShowScreen)
		{
			m_pDevCmn->fnSCR_DisplayPrevSet(113);
			m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172004));	// Title
#if (CA_VERSION)
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172018));	// Sub Title (with CAD)
#else
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172005));	// Sub Title
#endif
			strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172006));
			strTemp2.Format(L"%d", m_pDevCmn->m_MoneyMinOut);
			strTemp.Replace(L"XXX",  strTemp2);
			m_pDevCmn->fnSCR_DisplayString(14, strTemp);		// Guide1
			strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172007));
			strTemp2.Format(L"%d", m_pDevCmn->m_MoneyMaxOut);
			strTemp.Replace(L"XXXX", strTemp2); 
			m_pDevCmn->fnSCR_DisplayString(15, strTemp);		// Guide2
			m_pDevCmn->fnSCR_DisplayString(4, L"DOLLAR");	// Currency Symbol
			m_pDevCmn->fnSCR_DisplayString(5, L"INT");
			m_pDevCmn->fnSCR_DisplayString(10, L"1");	// min
			m_pDevCmn->fnSCR_DisplayString(11, L"8");	// max
			m_pDevCmn->fnSCR_DisplayString(12, L"off"); // auto run
			m_pDevCmn->fnSCR_DisplayScreen(113, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
			bShowScreen = FALSE;
		}

		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			keyString = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), keyString));

			keyString.Replace(GetCurrencySymbol(), L"");
			keyString.TrimLeft();
			keyString.TrimRight();
 
			if (keyString == S_CANCEL || keyString == S_EXIT)
			{
				return FID_NH_B4U_CANCEL_FLOWS;
			}
			else if (keyString == S_TIMEOVER)
			{
				return FID_NH_B4U_TIMEOUT_FLOWS;
			}
			else if (keyString.GetLength() > 0)
			{
				keyString.Replace(GetCurrencySymbol(), L"");
				keyString.TrimLeft();
				nInputAmount = Asc2Int(keyString);
				NHDEBUG(DBG_INFO, (_T("Input Amount to convert integer [%d]\n"), nInputAmount));

				if (nInputAmount > 0)
				{
					if (nInputAmount < m_pDevCmn->m_MoneyMinOut)
					{
						LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113105), ABORT_SCR_TIMEOUT);
						nRetry--;
						bShowScreen = TRUE;
					}
					else if ((nB4UMin > 0) && (nInputAmount < nB4UMin))
					{
						strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113115));
						strTemp2.Format(L"%s", m_B4UIni->MinimumDisplay);
						strTemp.Replace(L"XXXX", strTemp2);
						LIB_UserPopUpNotice(SCR_ICON_INFO, strTemp, ABORT_SCR_TIMEOUT);
						nRetry--;
						bShowScreen = TRUE;
					}
					else if (nInputAmount > m_pDevCmn->m_MoneyMaxOut)
					{
						LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113104), ABORT_SCR_TIMEOUT);
						nRetry--;
						bShowScreen = TRUE;
					}
					else if ((nInputAmount % m_pDevCmn->m_MoneyMinOut) != 0)
					{
						LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113106), ABORT_SCR_TIMEOUT);
						nRetry--;
						bShowScreen = TRUE;
					}
					else
					{
						m_sUserSelection.strMoney.Format(L"%010d", nInputAmount);
						int nRequestedMoney = Asc2Int(m_sUserSelection.strMoney.Left(10));
						CString sRequestedMoney;
						sRequestedMoney.Format(L"%d00", nRequestedMoney);
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT, sRequestedMoney);
						break;
					}
				}
				else
				{
					LIB_UserPopUpNotice(SCR_ICON_INFO, L"Unknown Amount, please try again", ABORT_SCR_TIMEOUT);
					nRetry--;
					bShowScreen = TRUE;
				}
			}
		}
		Delay_Msg(50);
	}

	if (g_TimeCheck.IsElapsedTimes())
	{
		return FID_NH_B4U_TIMEOUT_FLOWS;
	}

	// [#RWC6-77] Bitload4U Updated API and Enhancement
	/////////////////////////////////////////
	// 2-1. Surcharge info
	/////////////////////////////////////////
	// Get surcharge info from B4U host
	g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);

	// Display please wait
	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161101));
	m_pDevCmn->fnSCR_DisplayString(3, L"");
	m_pDevCmn->fnSCR_DisplayScreen(161);

	// Register the session
	sessionType = B4U_SESS_TYPE_PRESTAGE;
	B4U_PRE_STAGE res;
	if (this->m_B4USession == NULL)
	{
		//bool created;
		if (this->UpdateB4USessionStatus(sessionType))
		{
			res = B4U_PRE_STAGE_RECEIVED;
			NHDEBUG(DBG_INFO, (L"Finished getting prestage data from B4U: surcharge:%s\n", m_B4USessionStatus->FeeFromServer));
		}
		else
		{
			res = B4U_PRE_STAGE_ERROR;
			NHDEBUG(DBG_INFO, (L"Failed to get prestage data from B4U: forcing to use INI surcharge\n"));
		}
		
	}
	else
	{
		NHDEBUG(DBG_INFO, (L"B4U Prestage exists. Skipping prestage creation\r\n"));
	}
	// end of [#RWC6-77]

	/////////////////////////////////////////
	// 3. Fee Acceptance
	/////////////////////////////////////////
	// Display surcharge and get user's decision
	g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);

	// display
	m_pDevCmn->fnSCR_DisplayPrevSet(172);
	m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172103));	// Title
	strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172008));
	if ( (res == B4U_PRE_STAGE_RECEIVED) && (m_B4USessionStatus->FeeFromServer.GetLength() > 0) )
	{
		strTemp.Replace(L"XXXX",m_B4USessionStatus->FeeFromServer);
	}
	else
		strTemp.Replace(L"XXXX", m_B4UIni->FeeDisplay);
	m_pDevCmn->fnSCR_DisplayString(3, strTemp);													// Sub title // Fee	
	m_pDevCmn->fnSCR_DisplayString(10, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172009));	// F5F6	// Do you accept this fee?
	m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172011));	// F7 // CANCEL
	m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172010));	// F8 // ACCEPT
	m_pDevCmn->fnSCR_DisplayScreen(172, KEYIN_TIME_OUT, PIN_MENU_MODE);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			keyString = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), keyString));

			// remove... symbol, white space
			keyString.Replace(GetCurrencySymbol(), L"");
			keyString.TrimLeft();
			keyString.TrimRight();

			if (keyString == S_CANCEL || keyString == S_EXIT)
			{
				return FID_NH_B4U_CANCEL_FLOWS;
			}
			else if (keyString == S_TIMEOVER)
			{
				return FID_NH_B4U_TIMEOUT_FLOWS;
			}
			else if (keyString.GetLength() > 0)
			{
				if(keyString == L"ACCEPT")	
				{
					return FID_NH_B4U_QR_FLOWS;
				}
			}
		}
		// Update copy on screen if any
		Delay_Msg(50);
	}

	if (g_TimeCheck.IsElapsedTimes())
	{
		return FID_NH_B4U_TIMEOUT_FLOWS;
	}

	return FID_NH_B4U_FAIL_FLOWS;
}

/**
 * Shows the QR code on the screen
 */
FLOW_ID CTranCmn::F_NH_B4U_QR() 
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_B4UQR]\r\n"));

	B4USessionStatus sessionStatus;
	B4USessionType sessionType = B4U_SESS_TYPE_STAGE;
	CString remaining;
	double timeRemaining=0;  // The number of seconds remaining in the session
	CString qrFilename = L"\\ATM\\Data\\B4UQr.png";
	CString	strTemp = L"";

	// Display please wait
	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161101));
	m_pDevCmn->fnSCR_DisplayString(3, L"");
	m_pDevCmn->fnSCR_DisplayScreen(161);

	bool created;
	this->CreateNewB4USession(created, sessionType);
	if (created)
	{
		this->UpdateB4USessionStatus(sessionType);
	}
	else 
	{
		NHERROR((L"No B4U session could be created\r\n"));
		return FID_NH_B4U_FAIL_FLOWS;
	}
	NHDEBUG(DBG_INFO, (L"Finished getting session data from B4U: Transaction ID:%s\n", m_B4USession->TransactionId));


	LIB_UserPopUpNotice(SCR_ICON_INFO, L"Please get your QR code scanner ready!", ABORT_SCR_TIMEOUT);

	int nQRTimeOut = Asc2Int(m_B4UIni->QRCodeTimeout);
	g_TimeCheck.SetTargetTimeAfterSec((int)(nQRTimeOut*1.25));
	NHDEBUG(DBG_INFO, (_T("QRTimeOut = [%d]\n"), (int)(nQRTimeOut*1.25)));

	long	nInitialTick = GetTickCount();
	int				nInitTimer = nQRTimeOut;
	int				nCurrTimer = nInitTimer;
	int				nCountDown = 0;
	int				nPrevCountDown = -1;
	BOOL			bShowScreen = TRUE;
	BOOL			bIsQrReady = FALSE;

	// Start polling the status endpoint
	this->StartB4UPollingTimer();

	CString keyString;
	while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nCurrTimer > 0))
	{
		if (nPrevCountDown != nCountDown)
		{
			// Display copy and QR code
			m_pDevCmn->fnSCR_DisplayPrevSet(171);
			m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172104));	// Title
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172012));	// Subtitle
			remaining.Format(L"TIME LIMIT: %d SECONDS LEFT.", nCurrTimer);
			m_pDevCmn->fnSCR_DisplayString(8, remaining);												// F1F2 // timer
			if (!bIsQrReady)
			{
				if (m_B4USession->QrData.GetLength()>0)
				{
					if (!m_B4U->GenerateQRCode(m_B4USession->QrData, qrFilename))
					{
						NHDEBUG(DBG_INFO, (L"B4U QR does not exist. Session canceled\r\n"));
						this->ResetB4UStateData();
						return FID_NH_B4U_FAIL_FLOWS;
					}
					bIsQrReady = TRUE;
				}
				else
				{
					NHDEBUG(DBG_INFO, (L"B4U QR does not exist. Session canceled\r\n"));
					this->ResetB4UStateData();
					return FID_NH_B4U_FAIL_FLOWS;
				}
			}
			m_pDevCmn->fnAPL_DisplayQRCodeOnScreen(qrFilename);											// F3F4-F5F6 // QR code
			strTemp.Format(L"Transaction ID: %s", m_B4USession->EncTransactionId);
			m_pDevCmn->fnSCR_DisplayString(11, strTemp);												// F7F8	// Transaction ID
			m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172011));	//F7 Cancel
			m_pDevCmn->fnSCR_DisplayScreen(171, nCurrTimer, PIN_MENU_MODE);

			nPrevCountDown = nCountDown;
		}

		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			keyString = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), keyString));

			keyString.Replace(GetCurrencySymbol(), L"");
			keyString.TrimLeft();
			keyString.TrimRight();

			if (keyString == S_CANCEL || keyString == S_EXIT)
			{
				return FID_NH_B4U_CANCEL_FLOWS;
			}
			else if ((keyString == S_TIMEOVER) && (nCurrTimer <=0))
			{
				return FID_NH_B4U_TIMEOUT_FLOWS;
			}

#if(B4U_TEST_MODE)
			else if (keyString == L"CLEAR")
			{
				m_B4USessionStatus->Status = B4U_PAID;
				m_B4USessionStatus->TransactionHash = L"SAMPLEHASHxXKeotzizjan357pedjjLJreGsjwiznq4nsG583nvlkj2adfJKsd35";
				m_B4USessionStatus->CurrencyFromServer = "CAD";
				m_B4USessionStatus->EquivalentAmount = "31.11";
				m_B4USessionStatus->Amount = 3222;
				m_B4USessionStatus->AmountDisplay = "32.22";
				return FID_NH_B4U_AUTH_FLOWS;
			}
#endif
		}
		else
		{
			// Determine if we need to make a change
			B4U_STAGE res = GetTransactionFlowStage(m_B4U, m_B4USessionStatus, sessionType);
			switch (res)
			{
			case B4U_STAGE_ERROR:
				return FID_NH_B4U_FAIL_FLOWS;
			case B4U_STAGE_PAID:
				return FID_NH_B4U_AUTH_FLOWS;
			}

			nCountDown = (GetTickCount() - nInitialTick)/1000;
			if (nPrevCountDown != nCountDown)
			{
				nCurrTimer--;
			}
		}
		Delay_Msg(50);
	}

	if ( (g_TimeCheck.IsElapsedTimes()) || (nCurrTimer <= 0) )
	{
		return FID_NH_B4U_TIMEOUT_FLOWS;
	}

	return FID_NH_B4U_FAIL_FLOWS;
}

FLOW_ID CTranCmn::F_NH_B4U_Authorization()
{
	// Kill the polling thread 
	this->StopB4UPollingTimer();

	LIB_UserPopUpNotice(SCR_ICON_INFO, L"Successfully processed B4U", ABORT_SCR_TIMEOUT);
	return FID_NH_B4U_SUCCESS_FLOWS;
}

FLOW_ID CTranCmn::F_NH_B4U_Failed()
{
	NVDump('O', 'C', "00", L"B4U", L"NG");
	LIB_UserPopUpNotice(SCR_ICON_INFO, L"Transaction Failed", ABORT_SCR_TIMEOUT);
	// [#RWC6-188] B4U Log journal entry
	if (m_sUserSelection.strMoney.GetLength() > 0)
	{
		m_pDevCmn->fnAPL_AddSerialNo(HC_ATM); 
		if(!m_B4U->WriteB4UJournalEntry(&m_pDevCmn->m_JNLMgr, *m_B4USession, *m_B4USessionStatus, B4U_TXN_ERROR_FAIL))
		{
			NHERROR((L"Failed to write B4U journal entry\r\n"));
			NVDump('F', 'C', "24", L"B4U", L""); //TODO is this correct NVDump code?
		}
	}
	// End of [#RWC6-188]
	this->ResetB4UStateData();
	return FID_NH_NOR_EXIT;
}

FLOW_ID CTranCmn::F_NH_B4U_Cancelled()
{
	NVDump('O', 'C', "00", L"B4U", L"NG_37");
	LIB_UserPopUpNotice(SCR_ICON_INFO, L"Transaction Cancelled", ABORT_SCR_TIMEOUT);
	// [#RWC6-188] B4U Log journal entry
	if (m_sUserSelection.strMoney.GetLength() > 0)
	{
		m_pDevCmn->fnAPL_AddSerialNo(HC_ATM); 
		if(!m_B4U->WriteB4UJournalEntry(&m_pDevCmn->m_JNLMgr, *m_B4USession, *m_B4USessionStatus, B4U_TXN_ERROR_CANCEL))
		{
			NHERROR((L"Failed to write B4U journal entry\r\n"));
			NVDump('F', 'C', "24", L"B4U", L""); //TODO is this correct NVDump code?
		}
	}
	// End of [#RWC6-188]
	this->ResetB4UStateData();
	return FID_NH_NOR_EXIT;
}

FLOW_ID CTranCmn::F_NH_B4U_Timeout()
{
	NVDump('O', 'C', "00", L"B4U", L"NG_38");
	LIB_UserPopUpNotice(SCR_ICON_INFO, L"Time exceeded! Please try again later.", INFO_SCR_TIMEOUT, 1);
	// [#RWC6-188] B4U Log journal entry
	if (m_sUserSelection.strMoney.GetLength() > 0)
	{
		m_pDevCmn->fnAPL_AddSerialNo(HC_ATM); 
		if(!m_B4U->WriteB4UJournalEntry(&m_pDevCmn->m_JNLMgr, *m_B4USession, *m_B4USessionStatus, B4U_TXN_ERROR_TIMEOUT))
		{
			NHERROR((L"Failed to write B4U journal entry\r\n"));
			NVDump('F', 'C', "24", L"B4U", L""); //TODO is this correct NVDump code?
		}
	}
	// End of [#RWC6-188]
	this->ResetB4UStateData();
	return FID_NH_NOR_EXIT;
}

FLOW_ID CTranCmn::F_NH_B4U_Success()
{
	////////////////////////////
	/// 1. Dispense
	////////////////////////////
	NVDump('O', 'C', "00", L"B4U", L"Dispense");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_B4U_Success]-Dispense\n"));
	BIZ_RETURN	nRes;

	// 1-1. Increase Sequence Number
	m_pDevCmn->fnAPL_AddSerialNo(HC_ATM);

	// 1-2. Approved => Assign Value for Receipt
	CString strTemp, strTemp2;
	CTime ct = CTime::GetCurrentTime();
	strTemp2.Format(L"%04d", ct.GetYear());

	// 1-3. Host Date, Settlement date
	strTemp.Format(L"%02d%02d%s",  ct.GetMonth(), ct.GetDay(), strTemp2.Right(2) );
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE, strTemp);
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSSETTLEDATE, strTemp);
	// 1-4. Host Time
	strTemp.Format(L"%02d%02d%02d",  ct.GetHour(), ct.GetMinute(), ct.GetSecond() );
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME, strTemp);

	// 1-5. Operation Code
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE, TRANTYPE_B4U);

	// 1-6. Equivalent Amount in USD
	m_B4USession->EquivalentAmount	= m_B4USessionStatus->EquivalentAmount;

	// 1-7. Cash Dispense
	nRes = P_NH_NOR_CashDispense();
	int nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));
	m_B4U->RecordB4UDispenseResult(nDispensedAmount, *(this->m_B4UJournal));
	
	if (nRes != RES_OK)			// Dispensing Failure
	{
		// Take Cash
		if (nRes == RES_DEV_CDU_ERR_PARTIAL)
		{
			P_NH_NOR_TakeCash();
			m_B4UJournal->DispenseResult = B4U_DISP_RESULT_PARTIAL;
		}
		else // not dispensed at all
		{
			m_B4UJournal->DispenseResult = B4U_DISP_RESULT_NONE;
		}

		// NO REVERSAL with ATM Processor
		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

		// Save JNL and Update Statistics
		m_pDevCmn->fnAPL_SetProcCount('6');

		if (!m_B4U->WriteB4UJournalEntry(&m_pDevCmn->m_JNLMgr, *m_B4USession, *m_B4USessionStatus, B4U_TXN))
		{
			NHERROR((L"Failed to write B4U journal entry\r\n"));
			NVDump('F', 'C', "24", L"B4U", L""); //TODO is this correct NVDump code?
		}

		LIB_UserPopUpNotice(SCR_ICON_INFO, L"Dispensing Failed!", ABORT_SCR_TIMEOUT);
		NVDump('O', 'E', "12", L"B4U", m_strNVLog);

		return FID_NH_B4U_FINALIZE_FLOWS;
	}
	// 1-9. Set as Successful Dispensing
	m_B4UJournal->DispenseResult = B4U_DISP_RESULT_FULL;
	m_pDevCmn->fnAPL_SetProcCount('6');
	MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

	// 1-10. Save JNL and Update Statistics
	if (!m_B4U->WriteB4UJournalEntry(&m_pDevCmn->m_JNLMgr, *m_B4USession, *m_B4USessionStatus, B4U_TXN))
	{
		NHERROR((L"Failed to write B4U journal entry\r\n"));
		NVDump('F', 'C', "24", L"B4U", L""); //TODO is this correct NVDump code?
	}

	// 1-11. Take Cash
	P_NH_NOR_TakeCash();

	//////////////////////////////
	///// 2. Reversal
	//////////////////////////////
	// TODO add reversal here

	//////////////////////////////
	///// 3. Receipt
	//////////////////////////////
	NVDump('O', 'C', "00", L"B4U", L"Receipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_B4U_Success]-Receipt nPrintReceipt(%d)\n", m_sUserSelection.nPrintReceipt));

#ifdef UNDER_CE
	// [#2518] US Kook 2018.01.14
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE) == ENABLE)
	{
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_MEMORIZE_ENABLE) == ENABLE)
		{
			if (m_pDevCmn->fnCAM_CaptureFace(Asc2Int(m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig)), TIME_RECEIPT_PROCESSING) != T_OK)
			{
				NVDump('F', 'C', "93", m_pDevCmn->fstrCAM_GetErrorCode(), _T("Camera Error"));	// [#2548] NH Justin 2018.04.27 Leave IN SERVICE even though the Camera is OUT OF SERVICE
			}
		}

		m_pDevCmn->fnCAM_Deinitialize();
	}
#endif // UNDER_CE

	if ( (m_sUserSelection.nPrintReceipt == RCPT_PAPER) && (LIB_IsReceiptPrintCondition() == TRUE) )
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(119);

		// Setting Screen
		{
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172109));
		}

		// [#2327] NH KSK 2015.01.28
		// NH2600 / MX2600SE�� ��� Receipt Flicker�� �������� �ʾ� SW������ ��ġ ǥ��
		#if(US_VERSION || CA_VERSION || MX_VERSION)			// [#2338] US Justin 2015.03.31 Enable HALO2 on Canadian and Mexican Version
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker ���� (NH2600 / MX2600SE)
				m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
			else
				m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
		#endif
		// end of [#2327]

		m_pDevCmn->fnSCR_DisplayScreen(119);

		if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
		{
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119201), INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
			NVDump('O', 'C', "00", L"P_NHNOR", L"NG_61");	// [#2024] NH KSK 2011.02.24
			return FID_NH_B4U_FINALIZE_FLOWS;
		}
		else
		{
			CString strPrintData;
			this->F_NH_B4U_GenerateTxnData(strPrintData, RECEIPT_PAPER);
			m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT, TRUE, TRUE);

			// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker ���� (NH2600 / MX2600SE)
			if (!(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE"))	
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_ON);

			P_NH_NOR_TakeReceipt();
		}
	}

	return FID_NH_B4U_FINALIZE_FLOWS;
}

FLOW_ID CTranCmn::F_NH_B4U_Finalize()
{
	const int nScreenout = NORMAL_SCR_TIMEOUT;
	// display
	g_TimeCheck.SetTargetTimeAfterSec(nScreenout);
	m_pDevCmn->fnSCR_DisplayPrevSet(173);
	
	CString strPrintData;
	CStringArray strTempArray;

	// [#RWC6-77] Bitload4U Updated API and Enhancement
#if(!B4U_TEST_MODE)
	B4USessionType sessionType = B4U_SESS_TYPE_REPORT;
	bool created = false;
	this->CreateNewB4USession(created, sessionType);	
#endif
	// end of [#RWC6-77]

	this->F_NH_B4U_GenerateTxnData(strPrintData, RECEIPT_DISPLAY);

	SplitString(strPrintData, FIELD_DELIMITER, strTempArray);

	for (int i = 0; i < strTempArray.GetSize(); i++)
		m_pDevCmn->fnSCR_DisplayString((i+1), strTempArray[i]);

	m_pDevCmn->fnSCR_DisplayScreen(173, nScreenout, PIN_MENU_MODE);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			CString keyString = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), keyString));

			keyString.Replace(GetCurrencySymbol(), L"");
			keyString.TrimLeft();
			keyString.TrimRight();

			if (keyString == S_CANCEL || keyString == S_EXIT)
			{
				break;
			}
			else if (keyString == S_TIMEOVER)
			{
				break;
			}
		}
		// Update copy on screen if any
		Delay_Msg(50);
	}

	// [#RWC6-132] Health Check after B4U Transaction
	if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_HEALTHYFLAG) == ENABLE)
	{
		m_pDevCmn->Disable_CardReader_RFID();
		BIZ_HealthCheckProc(TRUE);
		m_pDevCmn->fnAPL_SetHealthCheckTimer();					// [#583] NH KSK 2009.11.24 Sec�� ����ʿ� ���� Refresh Time ���� ��ġ ���� (�۽� ��)
	}

	this->ResetB4UStateData();
	return FID_NH_NOR_EXIT;
}

void CTranCmn::F_NH_B4U_GenerateTxnData(CString &strPrintData, int nType)
{
	CString		strTemp, strTemp2, strNonUSDCurrency;

#if(B4U_TEST_MODE)
	strNonUSDCurrency = " CAD";
#else
	if (m_pDevCmn->m_strCurrencyID != "USD")
		strNonUSDCurrency.Format(L" %s", m_pDevCmn->m_strCurrencyID);
	else
		strNonUSDCurrency.Empty();
#endif

	if (nType == RECEIPT_PAPER)	//  MAX CHAR: 40
	{
		// TRANSACTION =
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_FINANCIAL));

		// DATE = 
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_004), m_B4USession->ReqDateTime);
	
		// TERMINAL ID =
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TML), m_B4USession->TerminalId);

		// BTC EXCHANGE TRANSACTION INFO
		AddPrintData(strPrintData, L"---------------------------------------");
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_TXN_INFO));
		AddPrintData(strPrintData, L"---------------------------------------");

		// BTC ADDRESS = 
		strTemp.Format(L"%s", m_B4USession->BitcoinAddr);
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_ADDRESS), strTemp.Left(20));
		int nLen	 = strTemp.GetLength();
		int nLine    = nLen/20;
		int nRemain  = nLen%20;
		for (int i=1; i<nLine+1; i++)
		{
			if (i+1 > nLine)
			{
				if (nRemain > 0)
				{
					AddPrintData(strPrintData, L"%-20.20s%-20.20s", L" ", strTemp.Mid(i*20,nRemain));
				}
			}
			else
			{
				AddPrintData(strPrintData, L"%-20.20s%s%-20.20s", L" ", strTemp.Mid(i*20,20));
			}
		}
		AddPrintData(strPrintData, L"%-40.40s", L"");

		// TRANSACTION ID = 
		strTemp.Format(L"%s", m_B4USession->EncTransactionId);
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_TXN_ID), strTemp.Left(20));
		nLen	 = strTemp.GetLength();
		nLine    = nLen/20;
		nRemain  = nLen%20;
		for (int i=1; i<nLine+1; i++)
		{
			if (i+1 > nLine)
			{
				if (nRemain > 0)
				{
					AddPrintData(strPrintData, L"%-20.20s%-20.20s", L" ", strTemp.Mid(i*20,nRemain));
				}
			}
			else
			{
				AddPrintData(strPrintData, L"%-20.20s%s%-20.20s", L" ", strTemp.Mid(i*20,20));
			}
		}
		AddPrintData(strPrintData, L"%-40.40s", L"");
	
		// SEQUENCE # = 
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_035), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SEQUENCENO));

		// AMOUNT REQUESTED =
		strTemp.Format(L"%s%s%s", GetCurrencySymbol(), MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT)), strNonUSDCurrency);
		AddPrintData(strPrintData, L"%17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_010), strTemp);

		// AMOUNT DISPENSED =
		strTemp.Format(L"%s%s%s", GetCurrencySymbol(), MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT)), strNonUSDCurrency);
		AddPrintData(strPrintData, L"%17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_009), strTemp);
		
		// EQUIVALENT IN USD =
		if (strNonUSDCurrency.GetLength() > 0)
		{
			strTemp.Format(L"%s%s USD", GetCurrencySymbol(), m_B4USession->EquivalentAmount);
			AddPrintData(strPrintData, L"%17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_061), strTemp);
		}

		// BTC CHARGED =
		strTemp.Format(L"%s(BTC)", m_B4USession->BitcoinAmountDisplay);
		AddPrintData(strPrintData, L"%17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_CHARGED), m_B4USession->BitcoinAmountDisplay);

		// FEE CHARGED = //B4U wants the fee to be in USD always regardless of currency
		if (m_B4USessionStatus->FeeFromServer.GetLength() > 0)
		{
			strTemp.Format(L"%s%s USD", GetCurrencySymbol(), m_B4USessionStatus->FeeFromServer);
		}
		else
		{
			strTemp.Format(L"%s%s USD", GetCurrencySymbol(), m_B4UIni->FeeDisplay);
		}
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_FEE_CHARGED), strTemp);

		// TRANSACTION HASH = 
		strTemp.Format(L"%s", m_B4USessionStatus->TransactionHash);
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_TXN_HASH), strTemp.Left(20));
		nLen	 = strTemp.GetLength();
		nLine    = nLen/20;
		nRemain  = nLen%20;
		for (int i=1; i<nLine+1; i++)
		{
			if (i+1 > nLine)
			{
				if (nRemain > 0)
				{
					AddPrintData(strPrintData, L"%-20.20s%-20.20s", L" ", strTemp.Mid(i*20,nRemain));
				}
			}
			else
			{
				AddPrintData(strPrintData, L"%-20.20s%s%-20.20s", L" ", strTemp.Mid(i*20,20));
			}
		}

		// TRANSACTION IS APPROVAED
		AddPrintData(strPrintData, L"%-40.40s", L"");
		AddPrintData(strPrintData, L"%-40.40s", L"");
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_TXN_APPROVED));	
		if	(m_B4UJournal->DispenseResult == B4U_DISP_RESULT_PARTIAL)		AddPrintData(strPrintData, L"%-40.40s", L"(but with partial cash dispense)");
		else if (m_B4UJournal->DispenseResult == B4U_DISP_RESULT_NONE)	AddPrintData(strPrintData, L"%-40.40s", L"(but with no cash dispense)");
		AddPrintData(strPrintData, L"%-40.40s", L"");

		// Disclaimer 1
		//TODO maybe add this on INI on B4U phase 2
		AddPrintData(strPrintData, L"%-40.40s", L"");
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_1_01));
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_1_02));
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_1_03));
		AddPrintData(strPrintData, L"%-40.40s",	L"");
	
		// Disclaimer 2
		//TODO maybe add this on INI on B4U phase 2
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_01));
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_02));
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_03));
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_04));
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_05));
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_06));
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_07));
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_08));
		AddPrintData(strPrintData, L"%-40.40s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_09));
		AddPrintData(strPrintData, L"%-40.40s", L"");
	}
	else	//  MAX CHAR: 75
	{
		// 1 TRANSACTION =
		AddScreenData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_FINANCIAL));

		// 2 DATE = 
		AddScreenData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_004), m_B4USession->ReqDateTime);
	
		// 3 TERMINAL ID =
		AddScreenData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TML), m_B4USession->TerminalId);

		// 4 BTC ADDRESS = 
		strTemp.Format(L"%s", m_B4USession->BitcoinAddr);
		AddScreenData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_ADDRESS), strTemp);

		// 5 TRANSACTION ID = 
		strTemp.Format(L"%s", m_B4USession->EncTransactionId);
		AddScreenData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_TXN_ID), strTemp);
	
		// 6 SEQUENCE # = 
		AddScreenData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_035), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SEQUENCENO));

		// 7 AMOUNT REQUESTED =
		strTemp.Format(L"%s%s%s", GetCurrencySymbol(), MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT)), strNonUSDCurrency);
		AddScreenData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_010), strTemp);

		// 8 AMOUNT DISPENSED =
		strTemp.Format(L"%s%s%s", GetCurrencySymbol(), MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT)), strNonUSDCurrency);
		AddScreenData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_009), strTemp);
		
		// EQUIVALENT AMOUNT =
		if (strNonUSDCurrency.GetLength() > 0)
		{
			strTemp.Format(L"%s%s USD", GetCurrencySymbol(), m_B4USession->EquivalentAmount);
			AddScreenData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_061), strTemp);
		}

		// 9 BTC CHARGED =
		strTemp.Format(L"%s(BTC)", m_B4USession->BitcoinAmountDisplay);
		AddScreenData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_CHARGED), m_B4USession->BitcoinAmountDisplay);

		// 10 FEE CHARGED = //B4U wants the fee to be in USD always regardless of currency
		if (m_B4USessionStatus->FeeFromServer.GetLength() > 0)
		{
			strTemp.Format(L"%s%s USD", GetCurrencySymbol(), m_B4USessionStatus->FeeFromServer);
		}
		else
		{
			strTemp.Format(L"%s%s USD", GetCurrencySymbol(), m_B4UIni->FeeDisplay);
		}
		AddScreenData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_FEE_CHARGED), strTemp);

		// 11, 12 TRANSACTION HASH = 
		strTemp.Format(L"%s", m_B4USessionStatus->TransactionHash);
		AddScreenData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_TXN_HASH), strTemp.Left(45));
		int nRemaining = (strTemp.GetLength())%45;
		AddScreenData(strPrintData, L"%-25.25s%-50.50s", L" ", strTemp.Mid(45, nRemaining));

		// 13 TRANSACTION IS APPROVAED
		strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_TXN_APPROVED));
		if		(m_B4UJournal->DispenseResult == B4U_DISP_RESULT_PARTIAL)	strTemp += L"(but with partial cash dispense)";
		else if (m_B4UJournal->DispenseResult == B4U_DISP_RESULT_NONE)	strTemp += L"(but with no cash dispense)";
		AddScreenData(strPrintData, L"%-75.75s",	strTemp);

		// 14, 15 Disclaimer 1
		//TODO maybe add this on INI on B4U phase 2
		AddScreenData(strPrintData, L"%-75.75s",	L"Contact support@b4ufinancial.com if you didn't receive money owed for this");
		AddScreenData(strPrintData, L"%-75.75s",	L"with the picture of this receipt.");

		// 16, 17, 18, 19, 20 Disclaimer 2
		//TODO maybe add this on INI on B4U phase 2
		AddScreenData(strPrintData, L"%-75.75s",	L"Your transaction will be credited subject to verification, collection and");
		AddScreenData(strPrintData, L"%-75.75s",	L"the Rules and Regulations for B4U Financial and as otherwise provided by");
		AddScreenData(strPrintData, L"%-75.75s",	L"law. Update to your account balance is controlled by your wallet and the");
		AddScreenData(strPrintData, L"%-75.75s",	L"blockchain and may require up 24 hours. Please retain this receipt until");
		AddScreenData(strPrintData, L"%-75.75s",	L"you confirm receipt of funds.");
	}
}

#endif
// End of [#]

#endif // #if APP_B4U