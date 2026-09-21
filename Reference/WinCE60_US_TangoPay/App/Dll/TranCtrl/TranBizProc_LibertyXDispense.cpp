#include "stdafx.h"
#include ".\Scr\ScrCtrl.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"

#include ".\Common\NHDbgApi.h"
#include ".\Tran\LibertyXTypes.h"

#if (APP_LIBERTYX)

#define AUTHORIZATION_RETRIES	1 // Max of 2 attempts
#define EXECUTE_RETRIES			1 // Max of 3 attempts
#define CANCEL_RETRIES			4 // Max of 5 attempts

extern CTimeCheck	g_TimeCheck;

/**
 * Customer is provided a text box in which to enter their cash token from the LTX app
 */
BIZ_RETURN CTranCmn::P_NH_LTXD_EnterToken(CString &token)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXD_EnterToken]\n"));
	NVDump('O', 'C', "26", L"Start", L"Tok");

	CString			GetKeyStr;

	// Show screen
	m_pDevCmn->fnSCR_DisplayPrevSet(362);
	m_pDevCmn->fnSCR_DisplayScreen(362, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "21", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));

				NVDump('F', 'C', "21", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else
			{
				token = GetKeyStr;
				NVDump('O', 'C', "26", L"TOK", token);
				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_LTXD_EnterToken_ADA(CString &token)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXD_EnterToken_ADA]\n"));
	NVDump('O', 'C', "26", L"Start", L"TokADA");

	CString	GetKeyStr;
	CString input;
	BOOL	invalidRepeat = FALSE;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if (nScrTimeOut == 0)	
	{   // ADA Mode....Jack removed.
		return RES_NG;
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "26", L"NG_2", L"");
			m_pAdaCtrl->fnExp_StopPlay();
			return RES_NG;
		}

		if (invalidRepeat && m_pAdaCtrl->IsPlaying() == FALSE )
		{
			invalidRepeat = FALSE;
			replay = TRUE;
		}

		if (replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(362, L"EnterLTXToken.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(362, L"ClearTokeKey.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(362, L"LocateCancelKey.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(362, L"LocateEnterKey.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(362, L"LocationRepeatKey.wav");	
			m_pAdaCtrl->fnExp_PlayScreenWave(362);

			replay = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "21", L"NG_0", GetKeyStr);

				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));

				NVDump('F', 'C', "21", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_CLEAR)
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Clear.wav", TRUE);
				input = L"";
			}
			else if (GetKeyStr == L"." || GetKeyStr == L"00" || GetKeyStr == L"000")  // Repeat or Volume Change
			{
				m_pAdaCtrl->fnExp_StopPlay();

				// Handle volume change repeats
				if(GetKeyStr == L".")
					m_pAdaCtrl->fnExp_SetVolumeDown();
				else if(GetKeyStr == L"00")
					m_pAdaCtrl->fnExp_SetVolumeUp();

				// Repeat after a volume change
				replay = TRUE;
			}
			else if (GetKeyStr == S_ENTER)
			{
				if (input.GetLength() <= 4) 
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay("362_TokenTooShort.wav", TRUE);
				}
				else if (input.GetLength() > 12)
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay("362_TokenTooLong.wav", TRUE);
				}
				else 
				{
					token = input;
					NVDump('O', 'C', "26", L"TOK", token);
					return RES_OK;
				}

				replay = TRUE;
				input = L"";
			}
			else if (GetKeyStr.GetLength() == 1)
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay(GetKeyStr + L".wav", TRUE);
				input += GetKeyStr;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}

/**
 * Customer is asked to wait while the token is being validated
 */
BIZ_RETURN CTranCmn::P_NH_LTXD_Authorize(LXDispenseAuthorization &auth, LXDispenseAuthorizationResponse &response, LXERRORCODE &error)
{
	int attempts = 0;
	long dispensableAmount = 0l;
	long requestedAmount = 0l;

	NVDump('O', 'C', "27", L"Start", L"Auth");

	// Display screen
	if (IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
		m_pAdaCtrl->fnExp_AddWaveFile(354, L"354.wav");
		m_pAdaCtrl->fnExp_PlayScreenWave(354);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(354);
		m_pDevCmn->fnSCR_DisplayScreen(354);
	}

	Delay_Msg(100);

	while (true)
	{
		if (!this->m_LXService->AuthorizeDispense(auth, response))
		{
			NHERROR((L"LX dispense authorization failed\r\n"));
			NVDump('F', 'C', "27", L"NetF", L"");
			attempts++;
		}
		else if (response.Status != L"success")
		{
			NHERROR((L"LX authorization was not successful from LibertyX\r\n"));
			NVDump('F', 'C', "27", L"Status", response.Status);
			attempts++;
		}
		else if (response.AuthorizationNonce != auth.AuthorizationNonce)
		{
			NHERROR((L"LX execute auth nonce did not match request\r\n"));
			NVDump('F', 'C', "27", L"NONCEF", L"Mismatch");
			error = LXERR_NONCEMISMATCH;
			return RES_NG;
		}
		else
		{
			break;
		}

		if (attempts > AUTHORIZATION_RETRIES + 1)
		{
			NHERROR((L"LX dispense authorization max attempts reached\r\n"));
			NVDump('F', 'C', "27", L"MAXR", L"");
			error = LXERR_CONNERR;
			return RES_NG;
		}
	}

	dispensableAmount = Asc2Int(auth.MaxDispense) / 100;
	requestedAmount = Asc2Int(response.Amount);

	if (requestedAmount > dispensableAmount)
	{
		NVDump('F', 'C', "27", L"TOOHIGH", response.Amount);
		error = LXERR_NOTDISPENSABLE;
		return RES_NG;
	}

	if (!m_pDevCmn->fbCDU_IsDispensible(requestedAmount))
	{
		NVDump('F', 'C', "27", L"NOTDISP", response.Amount);
		error = LXERR_NOTDISPENSABLE;
		return RES_NG;
	}

	NVDump('O', 'C', "27", L"OK", L"");

	return RES_OK;
}

/**
 * Customer is asked to wait while the transaction is "executed" or finalized for this ATM
 */
BIZ_RETURN CTranCmn::P_NH_LTXD_Execute(LXDispenseExecute &execute, LXDispenseExecuteResponse &response, LXERRORCODE &error)
{
	int attempts = 0;

	NVDump('O', 'C', "28", L"Start", L"Exec");

	// Display screen
	if (IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
		m_pAdaCtrl->fnExp_AddWaveFile(354, L"354.wav");
		m_pAdaCtrl->fnExp_PlayScreenWave(354);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(354);
		m_pDevCmn->fnSCR_DisplayScreen(354);
	}

	Delay_Msg(100);

	while (true)
	{
		if (!this->m_LXService->ExecuteDispense(execute, response))
		{
			NHERROR((L"LX dispense execution failed\r\n"));
			NVDump('F', 'C', "28", L"NETF", L"");
			attempts++;
		}
		else if (response.Status != L"success")
		{
			NHERROR((L"LX execution was not successful from LibertyX\r\n"));
			NVDump('F', 'C', "28", L"Status", response.Status);
			attempts++;
		}
		else if (response.AuthorizationNonce != execute.AuthorizationNonce)
		{
			NHERROR((L"LX execute auth nonce did not match request\r\n"));
			NVDump('F', 'C', "28", L"NONCEF", L"");
			error = LXERR_NONCEMISMATCH;
			return RES_NG;
		}
		else
		{
			break;
		}

		if (attempts > EXECUTE_RETRIES + 1)
		{
			NHERROR((L"LX dispense execution max attempts reached\r\n"));
			NVDump('F', 'C', "28", L"MAXR", L"");
			error = LXERR_CONNERR;
			return RES_NG;
		}
	}

	NVDump('O', 'C', "28", L"OK", L"");

	return RES_OK;
}

/**
 * Customer is shown a dispense screen while the cash is being dispensed. Note: The ADA is combined with this proc to reduce complexity
 */
BIZ_RETURN CTranCmn::P_NH_LTXD_Dispense(CString amount, CString &dispensedAmount)
{
	CString sRequestedMoney;
	int nRequestedMoney = 0;
	BIZ_RETURN result =  RES_OK;

	NVDump('O', 'C', "29", L"Start", amount);

	// Display screen
	if (IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
		m_pAdaCtrl->fnExp_AddWaveFile(363, L"363.wav");
		m_pAdaCtrl->fnExp_PlayScreenWave(363);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(363);
		m_pDevCmn->fnSCR_DisplayScreen(363);
	}

	Delay_Msg(100);

	// Set necessary params for dispense function
	nRequestedMoney = Asc2Int(amount);
	m_sUserSelection.strMoney.Format(L"%010d", nRequestedMoney);
	sRequestedMoney.Format(L"%d00", nRequestedMoney);
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT, sRequestedMoney);

	result = P_NH_NOR_CashDispense(FALSE /* denom selected */, FALSE /* show screen */);

	dispensedAmount = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT);
	NHINFO((L"LTX dispensed $%s\r\n", dispensedAmount));

	if (result != RES_OK)
	{
		NVDump('F', 'C', "29", L"NG_1", dispensedAmount);
		LIB_UserPopUpNotice(SCR_ICON_INFO, L"Cash Dispense Error", ABORT_SCR_TIMEOUT);
	}

	NVDump('O', 'C', "29", L"DISP", dispensedAmount);

	if (IsAdaTransaction())
	{
		m_pAdaCtrl->fnExp_StopAndResetWaveFile();
		m_pAdaCtrl->fnExp_AddWaveFile(1, L"Takemoney.wav");
		if		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1500SE")		// [#2380] US Justin 2015.12.09 Cardtronics Additional VG change
			m_pAdaCtrl->fnExp_AddWaveFile(1, L"Dispenser_Location_1500.wav"); 	
		else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1800SE")		// [#2380] US Justin 2015.12.09 Cardtronics Additional VG change
			m_pAdaCtrl->fnExp_AddWaveFile(1, L"Dispenser_Location_1800.wav");
		else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
			m_pAdaCtrl->fnExp_AddWaveFile(1, L"Dispenser_Location_4000.wav");
		else
			m_pAdaCtrl->fnExp_AddWaveFile(1, L"Dispenser_Location_Other.wav");
		m_pAdaCtrl->fnExp_PlayScreenWave(1);
		m_pAdaCtrl->fnExp_WaitUntilStop();
	}

	return result;
}

BIZ_RETURN CTranCmn::P_NH_LTXD_ReportTransaction(LXDispenseTransactionReport &report, LXDispenseAuthorizationResponse &authResponse)
{
	LXDispenseReportResponse response;

	NVDump('O', 'C', "2A", L"Start", L"Report");

	// Display screen
	if (IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
		m_pAdaCtrl->fnExp_AddWaveFile(354, L"354.wav");
		m_pAdaCtrl->fnExp_PlayScreenWave(354);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(354);
		m_pDevCmn->fnSCR_DisplayScreen(354);
	}

	Delay_Msg(100);

	if (!this->m_LXService->CommitDispenseTransactionReport(report, response))
	{
		NHERROR((L"LX dispense report failed\r\n"));
		NVDump('O', 'C', "2A", L"NG_1", L"NETF");
		return RES_NG;
	}

	return RES_OK;
}

/**
 * Customer is shown a screen notifying them that a receipt is being printed
 */
BIZ_RETURN CTranCmn::P_NH_LTXD_PrintingReceipt(CString token, int journalIdx, LXError *err)
{
	CString dispensedAmount = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT);
	CString requestedAmount = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT);
	CString receiptData = this->m_LXService->MakeReceiptDataForDispense(dispensedAmount, requestedAmount, token, journalIdx, err);

	NVDump('O', 'C', "2B", L"Start", L"Print");

	// Display screen
	m_pDevCmn->fnSCR_DisplayPrevSet(364);
	m_pDevCmn->fnSCR_DisplayScreen(364);

	Delay_Msg(100); // Let UI do work

	if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
	{
		NVDump('O', 'C', "2B", L"NOSPR", L"");
		return RES_NG;
	}
	
	// Print TXN receipt
	if (!m_pDevCmn->fnSPR_PrintReceipt(TRUE, receiptData, K_1_WAIT))
	{
		NHERROR((L"Printer failed to print LTX receipt\r\n"));
		NVDump('F', 'C', "2B", L"NG_1", L"");
		return RES_NG;
	}

	NVDump('O', 'C', "2B", L"OK", L"");

	Delay_Msg(7000);  // 7 seconds?

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_LTXD_PrintingReceipt_ADA(CString token, int journalIdx, LXError *err)
{
	CString dispensedAmount = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT);
	CString requestedAmount = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT);
	CString receiptData = this->m_LXService->MakeReceiptDataForDispense(dispensedAmount, requestedAmount, token, journalIdx, err);

	NVDump('O', 'C', "2B", L"Start", L"Print");

	if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
	{
		NVDump('O', 'C', "2B", L"NOSPR", L"");
		return RES_NG;
	}

	// Print TXN receipt
	if (!m_pDevCmn->fnSPR_PrintReceipt(TRUE, receiptData, K_1_WAIT))
	{
		NHERROR((L"Printer failed to print LTX receipt\r\n"));
		NVDump('F', 'C', "2B", L"NG_1", L"");
		return RES_NG;
	}

	m_pAdaCtrl->fnExp_StopAndResetWaveFile();
	m_pAdaCtrl->fnExp_AddWaveFile(1, L"Takereceipt.wav");
	if		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1500SE")
		m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_Location_1500.wav"); 	
	else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1800SE")
		m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_Location_1800.wav");
	else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
		m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_Location_4000.wav");
	else
		m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_Location_Other.wav");
	m_pAdaCtrl->fnExp_PlayScreenWave(1);
	m_pAdaCtrl->fnExp_WaitUntilStop();

	NVDump('O', 'C', "2B", L"OK", L"");
	return RES_OK;
}

/**
 * Customer is shown a screen describing why the transaction was canceled
 */
BIZ_RETURN CTranCmn::P_NH_LTXD_Cancel(LXDispenseCancel &cancel)
{
	int attempts = 0;
	LXDispenseCancelResponse response;

	NVDump('O', 'C', "2C", L"Start", L"Cancel");

	// Display screen
	if (IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
		m_pAdaCtrl->fnExp_AddWaveFile(354, L"354.wav");
		m_pAdaCtrl->fnExp_PlayScreenWave(354);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(354);
		m_pDevCmn->fnSCR_DisplayScreen(354);
	}

	while (true)
	{
		if (!this->m_LXService->CancelDispense(cancel, response))
		{
			NHERROR((L"LX dispense cancellation failed\r\n"));
			NVDump('F', 'C', "2C", L"NETF", L"");
			attempts++;
		}
		else if (response.Status != L"success")
		{
			NHERROR((L"LX cancellation was not successful from LibertyX\r\n"));
			NVDump('F', 'C', "2C", L"Status", response.Status);
			attempts++;
		}
		else
		{
			break;
		}

		if (attempts > CANCEL_RETRIES + 1)
		{
			NHERROR((L"LX dispense cancellation max attempts reached\r\n"));
			NVDump('F', 'C', "2C", L"MAXR", L"");
			return RES_NG;
		}
	}

	NVDump('F', 'C', "2C", L"OK", L"");

	return RES_OK;

}

BIZ_RETURN CTranCmn::P_NH_LTXD_Error(CString error, CString message)
{
	NVDump('F', 'C', "2D", L"ERR", error);

	m_pDevCmn->fnSCR_DisplayPrevSet(356);
	m_pDevCmn->fnSCR_DisplayString(2, message);
	m_pDevCmn->fnSCR_DisplayScreen(356);	

	Delay_Msg(5000); // Show screen for 5 seconds

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_LTXD_Error_ADA(CString error, CString message)
{
	NVDump('F', 'C', "2D", L"ERR", error);
	return PlayLibertyXADAMessage(356);
}

/**
 * Customer is thanked for the transaction
 */
BIZ_RETURN CTranCmn::P_NH_LTXD_ThankYou()
{
	return RES_OK;
}

//
// Private functions
//

BIZ_RETURN CTranCmn::PrintReceipt()
{
	return RES_OK;
}


#endif