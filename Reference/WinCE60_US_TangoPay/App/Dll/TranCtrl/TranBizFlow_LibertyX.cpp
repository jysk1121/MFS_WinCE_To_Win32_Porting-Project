#include "stdafx.h"
#include ".\Scr\ScrCtrl.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

#if (APP_LIBERTYX)

#define ADA(func, ...) IsAdaTransaction() ? func##_ADA(__VA_ARGS__) : func(__VA_ARGS__)

//------------------------------------------------------------------
//	Define Symbols
//------------------------------------------------------------------
extern CTimeCheck	g_TimeCheck;
extern CDevCmn*		m_pDevCmn;

#define REVERSAL_MAX 1

#define LIBERTYX_UNKNOWN_ERR_CUSTOMER_MESSAGE L"Unknown Error. Contact LibertyX"

FLOW_ID CTranCmn::F_NH_LibertyXInit()
{
	NVDump('O', 'C', "20", L"", L"START");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_LibertyXInit]\n"));

	CString	GetKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if (nScrTimeOut == 0)							// ADA Mode....Jack removed.
		return FID_NH_NOR_CANCEL;

	if (!IsAdaTransaction())
	{
		// Show screen
		m_pDevCmn->fnSCR_DisplayPrevSet(358);
		m_pDevCmn->fnSCR_DisplayImage(2, m_LXConfig.BuyBitcoinEnabled);
		m_pDevCmn->fnSCR_DisplayImage(3, m_LXConfig.SellBitcoinEnabled);
		m_pDevCmn->fnSCR_DisplayScreen(358, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}
	
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "20", L"P_NHADA", L"NG0");
			m_pAdaCtrl->fnExp_StopPlay();
			return FID_NH_NOR_CANCEL;
		}

		if (IsAdaTransaction() && replay) 
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(358, L"358.wav");
			if (m_LXConfig.BuyBitcoinEnabled)
			{
				m_pAdaCtrl->fnExp_AddWaveFile(358, L"358_Buy.wav");
			}

			if (m_LXConfig.SellBitcoinEnabled)
			{
				m_pAdaCtrl->fnExp_AddWaveFile(358, L"358_Sell.wav");
			}

			m_pAdaCtrl->fnExp_PlayScreenWave(358);

			replay = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (IsAdaTransaction() && GetKeyStr.GetLength() > 6)
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));

				if (GetKeyStr == L"." || GetKeyStr == L"00" || GetKeyStr == L"000")  // Repeat or Volume Change
				{
					m_pAdaCtrl->fnExp_StopPlay();

					// Handle volume change repeats
					if(GetKeyStr == L".")
						m_pAdaCtrl->fnExp_SetVolumeDown();
					else if(GetKeyStr == L"00")
						m_pAdaCtrl->fnExp_SetVolumeUp();

					// Repeat after a volume change
					replay = TRUE;
					continue;
				}
				else if (GetKeyStr == L"1") 
				{
					GetKeyStr = L"BUY";
				}
				else if (GetKeyStr == L"3") 
				{
					GetKeyStr = L"SELL";
				}
			}

			if (GetKeyStr == L"BUY")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"358_BuySelected.wav", TRUE);
				}

				return FID_NH_LIBERTYX_PURCHASE;
			}
			if (GetKeyStr == L"SELL")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"358_SellSelected.wav", TRUE);
				}

				return FID_NH_LIBERTYX_DISPENSE;
			}
			else if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "20", L"NG_0", GetKeyStr);

				if( IsAdaTransaction())		
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
				}

				return FID_NH_NOR_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));

				NVDump('F', 'C', "20", L"NG_1", GetKeyStr);

				return FID_NH_NOR_CANCEL;
			}
			else if (IsAdaTransaction())
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				replay = TRUE;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	NVDump('F', 'C', "20", L"NG_2", L"CANCEL");

	return FID_NH_NOR_CANCEL;
}

FLOW_ID CTranCmn::F_NH_LibertyXPurchase()
{
	LXAuthorization	auth; // [#RWC6-59] US William 2019.10.09 LibertyX
	LXAuthorizationResponse authResponse;
	CString			errorMessage;
	CString			errorCode;
	BIZ_RETURN		result = RES_OK;
	LXCard			card;
	LXError			error;

	LXCancel		 cancel;
	LXCancelResponse cancelResponse;

	LXExecute			execute;
	LXExecuteResponse	executeResponse;

	NVDump('O', 'C', "24", L"LTXBUY", L"");

	result = ADA(P_NH_LTXP_OrderID, auth.Token);
	if (result != RES_OK) 
	{
		NVDump('F', 'C', "24", L"NG_0", auth.Token);
		return FID_NH_NOR_CANCEL;
	}

	NVDump('O', 'C', "24", L"OID", auth.Token);

	auth.Currency = CURRENCY_TYPE;
	result = ADA(P_NH_LTXP_Amount, auth.SourceAmount);
	if (result != RES_OK)
	{
		NVDump('F', 'C', "24", L"NG_1", L"");
		return FID_NH_NOR_CANCEL;
	}

	result = ADA(P_NH_LTXP_InsertCard);
	if (result != RES_OK) 
	{
		NVDump('F', 'C', "24", L"NG_2", L"");
		return FID_NH_NOR_CANCEL;
	}

	// Populate card data
	card.Name = GetCardHolderName();
	card.TID = m_LXConfig.LocationID;
	card.Type = DEBIT;
	card.Last4 = m_sCardData.strBankID;
	card.BIN = m_sCardData.strISO2Data.Left(6);
	card.AtmTID = MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID).Trim();
	auth.Card = card;
	auth.HasCard = true;

	result = ADA(P_NH_LTXP_TransactionSelection);
	if (result == RES_LIBERTYX_BILLPAY) 
	{
		NVDump('F', 'C', "24", L"NG_3", L"");

		//
		// Authorize the LX transaction
		//
		P_NH_LTXP_Authorize(auth, authResponse);

		//
		// Cancel the transaction
		//

		error.Message = L"Error processing card";
		error.Code = L"L7090";

		cancel.Card = card;
		cancel.HasCard = true;
		cancel.Error = error;
		cancel.HasError = true;
		cancel.PaymentId = authResponse.PaymentId;

		if (P_NH_LTXP_Cancel(cancel, cancelResponse, /* reverse transaction */ false) != RES_OK)
		{
			NVDump('F', 'C', "24", L"NG_4", L"");
			NHERROR((L"Cancellation failed with message \"%s\"\r\n", cancelResponse.Message));
		}

		ADA(P_NH_LTXP_BillPay);
		return FID_NH_NOR_CANCEL;
	}
	else if (result != RES_LIBERTYX_BTC)
	{
		NVDump('F', 'C', "24", L"NG_5", L"");
		return FID_NH_NOR_CANCEL;
	}
	
	m_sUserSelection.strPassword.Empty();
	result = P_NH_NOR_EnterPassword();
	if (result != RES_OK)
	{
		NVDump('F', 'C', "24", L"NG_6", L"");
		return FID_NH_NOR_CANCEL;
	}

	result = ADA(P_NH_LTXP_Confirm);
	if (result != RES_OK)
	{
		NVDump('F', 'C', "24", L"NG_7", L"");
		return FID_NH_NOR_CANCEL;
	}

	errorMessage = L"Communication error. Please contact LibertyX support for assistance.";

	result = P_NH_LTXP_Authorize(auth, authResponse);
	if (result != RES_OK)
	{
		if (!authResponse.MessageCustomer.IsEmpty())
		{
			NVDump('F', 'C', "24", L"NG_8", authResponse.Message);
			errorMessage = authResponse.MessageCustomer;
		}

		if (!this->m_LXService->WriteJournalEntry(&m_pDevCmn->m_JNLMgr, auth, authResponse, execute, executeResponse, m_pDevCmn->fstrAPL_GetErrorCode()))
		{
			NVDump('F', 'C', "24", L"NG_9", L"");
			NHERROR((L"Failed to write LTX journal entry\r\n"));
		}

		ADA(P_NH_LTXP_AuthFail, errorMessage);
		return FID_NH_NOR_CLEANUP;
	}

	result = P_NH_LTXP_Debit(errorCode, errorMessage);
	if (result != RES_OK)
	{
		NVDump('F', 'C', "24", L"NG_10", L"");

		// Cancel the TXN with LibertyX
		LXCancel cancel;
		cancel.Card = card;
		cancel.HasCard = true;
		cancel.Error.Code = errorCode;
		cancel.Error.Message = errorMessage;
		cancel.HasError = true;
		cancel.PaymentId = authResponse.PaymentId;

		P_NH_LTXP_Cancel(cancel, cancelResponse, /* reverse transaction */ true);
		ADA(P_NH_LTXP_DebitFail, errorCode, errorMessage);
		if (!this->m_LXService->WriteJournalEntry(&m_pDevCmn->m_JNLMgr, auth, authResponse, execute, executeResponse, m_pDevCmn->fstrAPL_GetErrorCode()))
		{
			NHERROR((L"Failed to write LTX journal entry\r\n"));
			NVDump('F', 'C', "24", L"NG_11", L"");
		}

		return FID_NH_NOR_CLEANUP;
	}

	// Add authorization data to the card struct
	card.Authorization = GetAuthorizationNumber();
	execute.Card = card;
	execute.HasCard = true;
	execute.PaymentId = authResponse.PaymentId;

	result = P_NH_LTXP_Execute(execute, executeResponse);

	if (!this->m_LXService->WriteJournalEntry(&m_pDevCmn->m_JNLMgr, auth, authResponse, execute, executeResponse, m_pDevCmn->fstrAPL_GetErrorCode()))
	{
		NHERROR((L"Failed to write LTX journal entry\r\n"));
		NVDump('F', 'C', "24", L"NG_12", L"");
	}

	if (result != RES_OK)
	{
		NVDump('F', 'C', "24", L"NG_13", L"");
		if (!executeResponse.MessageCustomer.IsEmpty())
		{
			errorMessage = executeResponse.MessageCustomer;
		}

		ADA(P_NH_LTXP_ExecFail, errorMessage);
		return FID_NH_NOR_CLEANUP;
	}
	
	ADA(P_NH_LTXP_Success);
	result = ADA(P_NH_LTXP_Receipt);
	if (result != RES_OK)
	{
		NVDump('F', 'C', "24", L"NG_14", L"");
		P_NH_LTXP_DigitalReceipt();
	}

	NVDump('O', 'C', "24", L"DONE", L"");
	// All done!
	return FID_NH_NOR_CLEANUP;
}

FLOW_ID CTranCmn::F_NH_LibertyXDispense()
{
	int journalIdx = 0;
	LXERRORCODE lxError;
	CString dispensedAmount;
	CString dispenseErrorCode;
	BIZ_RETURN result = RES_OK;
	bool didConfirmAmount = false;
	LXDispenseAuthorization authorization;
	LXDispenseAuthorizationResponse authorizationResponse;
	LXDispenseExecute execute;
	LXDispenseExecuteResponse executeResponse;
	LXDispenseTransactionReport report;
	CString nonce = CLibertyXService::GetNewUUID();

	NVDump('O', 'C', "25", L"LTXDISP", L"");

	// Enter token
	result = ADA(P_NH_LTXD_EnterToken, authorization.Token);
	if (result != RES_OK)
	{
		NHWARN((L"Enter token failed\r\n"));

		if (result == RES_USER_TIMEOUT) 
		{
			// TODO
			NHWARN((L"Enter token timeout\r\n"));
			NVDump('F', 'C', "25", L"NG_1", L"TIME");
			return FID_NH_NOR_CANCEL;
		}

		NVDump('F', 'C', "25", L"NG_2", L"CANCEL");
		return FID_NH_NOR_CANCEL;
	}

	// Authorize
	authorization.AuthorizationNonce = nonce;
	authorization.Currency = CURRENCY_TYPE;
	authorization.LocationId = this->m_LXConfig.LocationID;
	authorization.MaxDispense = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT);
	CLibertyXService::PopulateCashCounts(authorization.CashDenominations, m_pDevCmn);

	NVDump('O', 'C', "25", L"LTXAUTH", L"");
	result = P_NH_LTXD_Authorize(authorization, authorizationResponse, lxError);
	if (result != RES_OK) 
	{
		CString message;

		LXDispenseCancel cancel;
		cancel.AuthorizationNonce = nonce;
		cancel.HasError = true;
		cancel.Error.Code = lxError;
		cancel.Error.Message = L"Authorization failed";

		NHWARN((L"LX transaction cannot complete.\r\n"));
		NVDump('F', 'C', "25", L"NG_3", lxError);

		result = P_NH_LTXD_Cancel(cancel);
		if (result != RES_OK)
		{
			NVDump('F', 'C', "25", L"NG_5", L"CANCERR");
		}

		message = authorizationResponse.MessageCustomer.IsEmpty() ? LIBERTYX_UNKNOWN_ERR_CUSTOMER_MESSAGE : authorizationResponse.MessageCustomer;
		ADA(P_NH_LTXD_Error, L"Authorization error", message);
		
		return FID_NH_NOR_EXIT;
	}

	// Set test mode 
	if (authorizationResponse.Test)
	{
		NHERROR((L"LTX Test Mode!!!\r\n"));
		NVDump('F', 'C', "25", L"LTX_TEST", L"");
	}

	NVDump('O', 'C', "25", L"LTXEXEC", L"");
	execute.AuthorizationNonce = nonce;
	execute.Test = authorizationResponse.Test;
	result = P_NH_LTXD_Execute(execute, executeResponse, lxError);
	if (result != RES_OK)
	{
		CString message;
		LXDispenseCancel cancel;
		cancel.AuthorizationNonce = nonce;
		cancel.HasError = true;
		cancel.Error.Code = lxError;
		cancel.Error.Message = L"Execute failed";

		NHWARN((L"Canceling LX transaction due to execute failure\r\n"));
		NVDump('F', 'C', "25", L"NG_4", lxError);

		m_LXService->WriteDispenseJournalEntry(&m_pDevCmn->m_JNLMgr, authorization, authorizationResponse, executeResponse, &cancel.Error);

		result = P_NH_LTXD_Cancel(cancel);
		if (result != RES_OK)
		{
			NVDump('F', 'C', "25", L"NG_5", L"CANCERR");
		}

		message = executeResponse.MessageCustomer.IsEmpty() ? LIBERTYX_UNKNOWN_ERR_CUSTOMER_MESSAGE : executeResponse.MessageCustomer;
		ADA(P_NH_LTXD_Error, L"Transaction error", message);

		return FID_NH_NOR_CANCEL;
	}

	// Validate the execution response parameters
	NVDump('O', 'C', "25", L"LTXVERIF", L"");
	if (!this->m_LXService->VerifyDispenseExecutionV1Response(executeResponse, authorization.Token, authorizationResponse.Amount, lxError))
	{
		NHERROR((L"LX execution response parameters failed validation: error[%s]\r\n", lxError));
		NVDump('F', 'C', "25", L"NG_6", lxError);
		
		LXDispenseCancel cancel;
		cancel.AuthorizationNonce = nonce;
		cancel.HasError = true;
		cancel.Error.Code = lxError;
		cancel.Error.Message = L"Execute validation failed";

		result = P_NH_LTXD_Cancel(cancel);
		if (result != RES_OK)
		{
			NVDump('F', 'C', "25", L"NG_7", L"CANCERR");
		}

		journalIdx = m_LXService->WriteDispenseJournalEntry(&m_pDevCmn->m_JNLMgr, authorization, authorizationResponse, executeResponse, &cancel.Error);

		ADA(P_NH_LTXD_Error, L"Transaction error", LIBERTYX_UNKNOWN_ERR_CUSTOMER_MESSAGE);

		result = ADA(P_NH_LTXD_PrintingReceipt, authorization.Token, journalIdx, &cancel.Error);
		if (result != RES_OK)
		{
			// SPR error. Display on-screen receipt. TODO
		}
		
		return FID_NH_NOR_EXIT;
	}

	// Dispense the money!
#ifndef LTX_TEST_MODE
	if (!executeResponse.Test)
	{
#endif
		NVDump('O', 'C', "25", L"LTXDISP", L"");
		result = P_NH_LTXD_Dispense(authorizationResponse.Amount, dispensedAmount);
		if (result == RES_DEV_CDU_ERR)  // No dispense
		{
			// TODO - Ask LTX for handling
			NVDump('F', 'C', "25", L"NG_8", L"NO_DISP");
		}
		else  // Successful or partial dispense
		{
			// Take the cash that is presented
			// At least some cash was presented
			NVDump('F', 'C', "25", L"NG_9", L"PARTDISP");
		}

		// If the CDU has an error, report it to LTX
		if (m_pDevCmn->fnAPL_GetErrorDevice(DEV_CDU))
		{
			NVDump('F', 'C', "25", L"NG_10", L"CDUERR");
			CString error = m_pDevCmn->fstrCDU_GetErrorCode();
			CString message = m_pDevCmn->fstrAPL_GetErrorMessage();
			int dispensed = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT)) / 100;

			report.Status = "error";
			report.HasError = true;
			report.Error.Code = error;
			report.Error.Message = message.IsEmpty() ? L"CDU Hardware Error" : message;
			report.Error.IsDispenseError = true;
			report.Error.DispensedAmount.Format(L"%d", dispensed);

			ADA(P_NH_LTXD_Error, report.Error.Code, report.Error.Message);
		}
		else
		{
			report.Status = "success";
			report.HasError = false;
			NVDump('O', 'C', "25", L"DISP", L"OK");
		}
#ifndef LTX_TEST_MODE
	}
	else 
	{
		NHERROR((L"LTX Test Dispense\r\n"));

		report.Error.Code = L"TEST_COMPLETE";
		report.Error.Message = L"Test success. No cash dispensed.";
		report.HasError = true;
		report.Status = "success";
	}
#endif

	// Write journal immediately after dispense
	journalIdx = m_LXService->WriteDispenseJournalEntry(&m_pDevCmn->m_JNLMgr, authorization, authorizationResponse, executeResponse, report.HasError ? &report.Error : NULL);
	NVDump('F', 'C', "25", L"JRNL_LTX", Int2Asc(journalIdx));

	// Report transaction results
	NVDump('O', 'C', "25", L"LTXREP", L"");
	report.AuthorizationNonce = nonce;
	result = P_NH_LTXD_ReportTransaction(report, authorizationResponse);
	if (result != RES_OK)
	{
		NVDump('F', 'C', "25", L"NG_11", L"REP_ERR");
	}

	// Print the receipt
	result = ADA(P_NH_LTXD_PrintingReceipt, authorization.Token, journalIdx, report.HasError ? &report.Error : NULL);
	if (result != RES_OK)
	{
		// SPR error. Display on-screen receipt. TODO
	}

	NHINFO((L"LTX dispense transaction completed.\r\n"));
	return FID_NH_NOR_EXIT;
}

/**
 * Helper methods
 */

CString	CTranCmn::GetAuthorizationNumber()
{
	CString strTemp;

	if (m_LXConfig.HostProtocol == LX_STANDARD3)
	{
		// Triton has a different audit number format
		strTemp.Format(L"%-6.6s%-2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID));
	}
	else
	{
		// Standard 1, 2, and EPS have spaces
		strTemp.Format(L"%-6.6s %-2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID));
	}

	return strTemp;
}

BIZ_RETURN CTranCmn::ReverseTransaction()
{
	int attempts = 0;
	BIZ_RETURN reversalStatus;

	do {
		TranCode = TC_REVERSAL;

		if (m_LXConfig.HostDepModeEnabled)
		{
			m_HostConfig = HC_LTX_HOST_DEP_MODE;
		}
		else
		{
			m_HostConfig = HC_LIBERTYX;
		}

		reversalStatus = P_NH_NOR_Transaction(TRUE);
		
		if (reversalStatus != RES_OK)
		{
			NHERROR((L"Reversal failed\r\n"));
		}

	} while (reversalStatus != RES_OK && ++attempts <= REVERSAL_MAX);

	if (reversalStatus == RES_OK)
	{
		// No reversal on boot!!!
		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
	}

	return reversalStatus;
}

#endif