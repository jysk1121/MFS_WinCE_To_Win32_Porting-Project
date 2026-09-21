#include "stdafx.h"
#include ".\Tran\TranCmn.h"
#include ".\Common\NHDbgApi.h"

#if (APP_JUST_CASH)

FLOW_ID CTranCmn::F_NH_JustCashStartFlow()
{
	NVDump('O', 'C', "-1", L"", L"START");
	NHDBG((L"[CTranCmn::F_NH_JustCashStartFlow]\n"));

	if (P_NH_NOR_SelectLanguage() != RES_OK)
	{
		NVDump('F', 'C', "-1", L"", L"NO_LANG");

		return FID_NH_JUSTCASH_CLEANUP;
	}

	BOOL cardlessEnabled = H_NH_JC_IsJustCashCardless();
	BOOL bitcoinEnabled = H_NH_JC_IsJustCashBitcoin();

	if (cardlessEnabled && !bitcoinEnabled)
	{
		if (IsAdaTransaction())
		{
			m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Auto_CardlessSelected.wav", TRUE);
		}

		_jcTransactionState.FlowSelection = JCF_CARDLESS;
	}
	else if (!cardlessEnabled && bitcoinEnabled)
	{
		if (IsAdaTransaction())
		{
			m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Auto_BitcoinSelected.wav", TRUE);
		}

		_jcTransactionState.FlowSelection = JCF_BITCOIN;
	}
	else if (cardlessEnabled && bitcoinEnabled)
	{
		if (P_NH_JC_ChooseFlow(&_jcTransactionState.FlowSelection) != RES_OK)
		{
			NVDump('O', 'C', "-1", L"", L"NO_FLOW");

			return FID_NH_JUSTCASH_CLEANUP;
		}
	}
	else
	{
		NVDump('O', 'C', "-1", L"", L"NO_FLOW_AVAILABLE");
		return FID_NH_JUSTCASH_CLEANUP;
	}

	if (_jcTransactionState.FlowSelection == JCF_BITCOIN)
	{
		BOOL atmAvailable = H_NH_JC_IsAtmModeAvailable();
		BOOL sidecarAvailable = H_NH_JC_IsSidecarModeAvailable();

		if (atmAvailable && !sidecarAvailable)
		{
			if (IsAdaTransaction())
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Auto_DebitSelected.wav", TRUE);
			}

			_jcTransactionState.PaymentSelection = JCP_DEBIT;

			return FID_NH_JUSTCASH_TRANSACTION;
		}
		else if (!atmAvailable && sidecarAvailable)
		{
			if (IsAdaTransaction())
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Auto_CashSelected.wav", TRUE);
			}

			_jcTransactionState.PaymentSelection = JCP_CASH;

			return FID_NH_JUSTCASH_GREET;
		}
		else if (atmAvailable && sidecarAvailable)
		{
			if (P_NH_JC_ChoosePayment(&_jcTransactionState.PaymentSelection) != RES_OK)
			{
				NVDump('O', 'C', "-1", L"", L"NO_PAYMENT");

				return FID_NH_JUSTCASH_CLEANUP;
			}

			if (_jcTransactionState.PaymentSelection == JCP_DEBIT)
			{
				return FID_NH_JUSTCASH_TRANSACTION;
			}
			else if (_jcTransactionState.PaymentSelection == JCP_CASH)
			{
				return FID_NH_JUSTCASH_GREET;
			}
			else
			{
				NVDump('O', 'C', "-1", L"", L"NO_PAYMENT_AVAILABLE");

				return FID_NH_JUSTCASH_CLEANUP;
			}
		}
		else
		{
			NVDump('O', 'C', "-1", L"", L"NO_PAYMENT_AVAILABLE");

			return FID_NH_JUSTCASH_CLEANUP;
		}
	}
	else if (_jcTransactionState.FlowSelection == JCF_CARDLESS)
	{
		return FID_NH_JUSTCASH_CARDLESS_TRANSACTION;
	}
	else
	{
		NVDump('O', 'C', "-1", L"", L"NO_FLOW_AVAILABLE");

		return FID_NH_JUSTCASH_CLEANUP;
	}

	return FID_NH_JUSTCASH_CLEANUP;
}

FLOW_ID CTranCmn::F_NH_JustCashGreetFlow()
{
	NVDump('O', 'C', "-1", L"", L"START");
	NHDBG((L"[CTranCmn::F_NH_JustCashGreetFlow]\n"));

	m_JustCashData.ResetTransactionData();

	m_pDevCmn->TranStatus = TRAN_TRAN;
	m_pDevCmn->nKindOfMedia = MEDIA_MS; // Proceed as Magnetic stripe Mode

	TranCode = TC_JUSTCASH_BITCOIN;

	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE, WITHDRAWAL_DIGITAL_CUR_CASH_JC);

	// Check Receipt Printer : Cancel if Printer is not online
	if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
	{
		NVDump('O', 'C', "-1", L"", L"NO_PRINTER");

		return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
	}

	m_sUserSelection.nPrintReceipt = RCPT_PAPER; // Paper Receipt

	if (P_NH_JC_EnterCustomerPhoneNumber(m_JustCashData.m_strPhoneNum) != RES_OK)
	{
		NVDump('O', 'C', "-1", L"", L"NO_PHONE");

		return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
	}

	m_JustCashData.m_nTransactionStep = JUSTCASH_DIGITAL_CUR_CASH_PHONE;

	if (P_NH_NOR_Transaction(FALSE) != RES_OK)
	{
		P_NH_JC_Error();
		NVDump('O', 'C', "-1", L"", L"ERR_PHONE");

		return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
	}

	m_JustCashData.m_nTransactionStep = JUSTCASH_DIGITAL_CUR_CASH_CODE;

	if (P_NH_JC_EnterSmsCode(m_JustCashData.m_strCode) != RES_OK)
	{
		NVDump('O', 'C', "-1", L"", L"NO_PIN");

		return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
	}

	if (P_NH_NOR_Transaction(FALSE) != RES_OK)
	{
		P_NH_JC_Error();
		NVDump('O', 'C', "-1", L"", L"ERR_CODE");

		return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
	}

	// Set Other Message - FOR Journal and RMS
	CString strTemp = L"";

	// [#2503] US Justin 2017.09.06 Add Ethereum
	// strTemp.Format(L"%c%s%sBITCOIN", UNIT_DELIMITER, RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_GENERAL_COMMENT);
	strTemp.Format(L"%c%s%s", UNIT_DELIMITER, RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_GENERAL_COMMENT);

	if (m_JustCashData.m_strCurrencyType == JUSTCASH_CURRENCY_BITCOIN)
		strTemp += L"BITCOIN";
	else if (m_JustCashData.m_strCurrencyType == JUSTCASH_CURRENCY_ETHEREUM)
		strTemp += L"ETHEREUM";
	else
		strTemp += L"DITIGAL CUR.";
	// End of [#2503]

	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strTemp);

	return FID_NH_JUSTCASH_TRANSACTION;
}

FLOW_ID CTranCmn::F_NH_JustCashTransactionFlowAtm()
{
	CString errorMessage;
	CString errorCode;

	if (P_NH_JC_InsertCard() != RES_OK)
	{
		NVDump('F', 'C', "-1", L"NG_6", L"");

		return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
	}

	m_sUserSelection.strPassword.Empty();

	if (P_NH_NOR_EnterPassword() != RES_OK)
	{
		NVDump('F', 'C', "-1", L"NG_6", L"");

		return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
	}

	return FID_NH_JUSTCASH_DEBIT;
}

FLOW_ID CTranCmn::F_NH_JustCashTransactionFlowSidecar()
{
	if (P_NH_JC_SelectCoin() != RES_OK)
	{
		return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
	}

	if (P_NH_JC_ApproveTransaction() != RES_OK)
	{
		return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
	}

	BIZ_RETURN ret = RES_USER_PREV;

	P_NH_JC_PrintQRCode(false);

	while (ret == RES_USER_PREV)
	{
		ret = P_NH_JC_ConfirmWallet();

		if (ret == RES_USER_PREV)
		{
			P_NH_JC_PrintQRCode(false);
		}
		else if (ret != RES_OK)
		{
			NVDump('F', 'C', "-1", L"NG_0", L"");

			return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
		}
	}

	return FID_NH_JUSTCASH_ACCEPT_CASH;
}

FLOW_ID CTranCmn::F_NH_JustCashCardlessTransactionFlow()
{
	NVDump('O', 'C', "00", L"F_NH", L"JustCash");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_JustCashTransaction]\n"));
	BIZ_RETURN nRes;

	// Deny RMS Connection
	m_pDevCmn->fnNET_RMSConnectClose(); // [#2405] US Justin HalCash Online

	// Set Default Transaction Variables
	MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID, L"");

	m_pDevCmn->nKindOfMedia = MEDIA_MS;				// Proceed as Magnetic stripe Mode
	m_sUserSelection.nPrintReceipt = RCPT_PAPER;	// Paper Receipt

	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE, WITHDRAWAL_JUSTCASH);

	// Assign Transaction Code
	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_JUSTCASH;
	m_SourceAccount = S_CHECKING; // From Checking Account

	m_JustCashData.ResetTransactionData();

	while (1)
	{
		// Proceed with Default Language
		// m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);

		// 1. Enter Cash Code
		if ((nRes = P_NH_JC_Enter_CashCode()) != RES_OK)
			break;

		// 2. Submit Cash Code
		if ((nRes = P_NH_JC_Submit_CashCode()) != RES_OK)
		{
			P_NH_JC_Error();
			break;
		}

		// 3. Enter PIN for OFF-US Transaction ONLY
		if (m_JustCashData.m_bIsOnUsTransaction != TRUE)
		{
			if ((nRes = P_NH_NOR_EnterPassword()) != RES_OK) // nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
				break;

			m_pDevCmn->fnAPL_DeviceEnDisable(DEV_PIN, DISABLE);

			Delay_Msg(50);

			m_pDevCmn->fnAPL_BuildPinBlockWithAccountNo(m_sCardData.strAccountNo);
			m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);

			m_sUserSelection.strPassword.Format(L"%16.16s", m_pDevCmn->fstrPIN_GetPinKeyData());

			NHDEBUG(DBG_CALL, (L"[CTranCmn::Original PIN Block Generaged:%s]\n", m_sUserSelection.strPassword));
		}

		// 4. Transaction
		nRes = P_NH_JC_Authorization();

		if (nRes != RES_OK)
		{
			// JNL Save
			switch (nRes)
			{
			case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
			case RES_HOST_RECV_ERR:	m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
			case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
			}

			// 5. Update Transaction to Just.Cash
			P_NH_JC_UpdateTransaction(0);

			if (m_JustCashData.m_bIsOnUsTransaction == TRUE)
				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE); // ON-US Transaction : NO REVERSAL with ATM Processor

			m_JustCashData.m_nTransactionStep = JUSTCASH_CARDLESS_TRAN_HOSTREVERSAL;

			return FID_NH_NOR_REVERSAL;
		}

		// 6. Cash Dispense
		nRes = P_NH_NOR_CashDispense();
		int nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

		// 7. Update Transaction result to Just Cash Host
		P_NH_JC_UpdateTransaction(nDispensedAmount);

		if (nRes != RES_OK) // Dispensing Failure
		{
			// 8. Update Statistics
			if (m_JustCashData.m_bIsOnUsTransaction == TRUE)
			{
				// Regular ATM Transaction : "REVERSAL_TRAN" is set in STD1, STD2, STD3 Host Response....
				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN); // Prevent ON-US Normal journal for Dispense error
			}

			P_NH_NOR_SaveCWStatus(FALSE);

			// 9. Tack Cash
			if (nRes == RES_DEV_CDU_ERR_PARTIAL)
				P_NH_NOR_TakeCash();

			// 10. ON-US Transaction : NO REVERSAL with ATM Processor
			if (m_JustCashData.m_bIsOnUsTransaction == TRUE)
			{
				m_pDevCmn->m_JNLMgr.Save(REVERSAL_TRX);

				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
			}

			m_JustCashData.m_nTransactionStep = JUSTCASH_CARDLESS_TRAN_HOSTREVERSAL;

			return FID_NH_NOR_REVERSAL;
		}

		// 7. Set as Successful Dispensing
		m_pDevCmn->fnAPL_SetProcCount('6');

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

		// 8. Save JNL and Update Statistics
		P_NH_NOR_SaveCWStatus(FALSE);

		// 9. Take Cash
		P_NH_NOR_TakeCash();

		return FID_NH_NOR_REVERSAL;
	}

	g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;

	return FID_NH_NOR_CANCEL;
}

FLOW_ID CTranCmn::F_NH_JustCashDebitFlow()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"BitCoin");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_JustCashBitCoinTransaction]\n"));

	BIZ_RETURN nRes;

	m_JustCashData.ResetTransactionData();
	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_JUSTCASH_BITCOIN;

	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE, WITHDRAWAL_DIGITAL_CUR_JC);

	while (1)
	{
		// 1. Select Digital Currency Type
		if ((nRes = P_NH_JC_SelectCoin()) != RES_OK)
			break;

		// 2. Check Receipt Printer : Cancel if Printer is not online
		if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
		{
			LIB_UserPopUpNotice(SCR_ICON_STOP, L"PRINTER ERROR", ABORT_SCR_TIMEOUT);
			break;
		}

		m_sUserSelection.nPrintReceipt = RCPT_PAPER; // Paper Receipt

		// 3. Select Account
		if ((nRes = P_NH_NOR_SelectCWAccount()) != RES_OK)
			break;

		// 4. Get minimum amount for transaction
		m_JustCashData.m_nTransactionStep = JUSTCASH_DIGITAL_CUR_INFO;

		if ((nRes = P_NH_NOR_Transaction(FALSE)) != RES_OK)
			break;

		// 5. Input Amount
		if ((nRes = P_NH_JC_EnterAmount()) != RES_OK)
			break;

		// 6. Make EMV Data
		if (m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			if (P_NH_NOR_EMV_IC_Processing() != RES_OK)
				break;
		}

		// 7. Surcharge
		m_nDisplayedSurchargeAmount = 0;

		// 8. BitCoin Purchase (return btc price, quantity, whole unit price, public key, private key)
		if ((nRes = P_NH_JC_Purchase()) != RES_OK)
			break;

		// 9. User Selection - Approve Transaction or Decline Transaction
		if ((nRes = P_NH_JC_ApproveTransaction()) != RES_OK)
		{
			P_NH_JC_Cancel();
			break;
		}

		// 10. Set Other Message - FOR Journal and RMS
		CString strTemp = L"";

		// [#2503] US Justin 2017.09.06 Add Ethereum
		// strTemp.Format(L"%c%s%sBITCOIN", UNIT_DELIMITER, RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_GENERAL_COMMENT);
		strTemp.Format(L"%c%s%s", UNIT_DELIMITER, RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_GENERAL_COMMENT);

		if (m_JustCashData.m_strCurrencyType == JUSTCASH_CURRENCY_BITCOIN)
			strTemp += L"BITCOIN";
		else if (m_JustCashData.m_strCurrencyType == JUSTCASH_CURRENCY_ETHEREUM)
			strTemp += L"ETHEREUM";
		else
			strTemp += L"DITIGAL CUR.";
		// End of [#2503]

		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strTemp);

		// 11. ATM Processor Transaction
		nRes = P_NH_JC_Processor_Auth();
		m_JustCashData.m_nTransactionStep = JUSTCASH_DIGITAL_CUR_PROC_REVERSAL; // Change Reversal Status

		if (nRes != RES_OK)
		{
			// JNL Save
			switch (nRes)
			{
			case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
			case RES_HOST_RECV_ERR:	m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
			case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
			}

			if (m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				if (m_pDevCmn->m_bDisplayDeclined == TRUE)
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127005));
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
			}

			return FID_NH_NOR_REVERSAL;
		}

		// 12. Complete EMV Transaction
		BOOL bFinishSuccessfully = TRUE;

		if (m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			nRes = P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127004));

			if (nRes == RES_USER_TIMEOUT)
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);

			if (nRes != RES_OK)
				bFinishSuccessfully = FALSE;
		}

		// 13. Print QR Code Receipt
		if (bFinishSuccessfully)
		{
			if ((nRes = P_NH_JC_PrintQRCode(true)) != RES_OK)
			{
				// Store Printer ERROR as HOST Denial => FOR "Denial Receipt(SCREEN)"
				m_JustCashData.SetErrorCodeAndDesc(L"930"); // Printer Error
				m_pDevCmn->fnAPL_StackError(m_JustCashData.m_strErrorCode, m_JustCashData.m_strErrorDesc, DEV_NET);

				bFinishSuccessfully = FALSE;
			}
		}

		// 14. BitCoin Final Commit
		if (bFinishSuccessfully)
		{
			if ((nRes = P_NH_JC_Commit(1)) != RES_OK)
				bFinishSuccessfully = FALSE;
		}

		if (bFinishSuccessfully)
		{
			m_pDevCmn->fnAPL_SetProcCount('6');

			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));

			m_JustCashData.RecodeDispenseResult(Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT)), JUSTCASH_SERVICE_DIGITAL_CURRENCY);

			P_NH_NOR_SaveCWStatus(TRUE); // Save dispense history and Journals
		}
		else
		{
			m_JustCashData.RecodeDispenseResult(0, JUSTCASH_SERVICE_DIGITAL_CURRENCY);
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT); // Transaction Cancelled
		}

		return FID_NH_NOR_REVERSAL;
	}

	if (m_pDevCmn->fnMCU_IsEmvTransaction())
		P_EMV_RemoveCard();

	g_sBizFlowInfo.nReasonforCancel = nRes;

	return FID_NH_NOR_CANCEL;
}

FLOW_ID CTranCmn::F_NH_JustCashAcceptCashFlow()
{
	BIZ_RETURN ret = P_NH_JC_CollectCash();

	if (ret != RES_OK)
	{
		NVDump('F', 'C', "-1", L"NG_0", L"");

		return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
	}

	if (m_JustCashData.m_strDCPrice.IsEmpty())
	{
		NVDump('F', 'C', "-1", L"NG_1", L"");

		return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
	}

	return FID_NH_JUSTCASH_END_TRANSACTION;
}

FLOW_ID CTranCmn::F_NH_JustCashEndTransactionFlow()
{
	if (P_NH_JC_ApproveTransaction() != RES_OK)
	{
		return FID_NH_JUSTCASH_ACCEPT_CASH;
	}

	if (P_NH_JC_CompleteTransaction() != RES_OK)
	{
		NVDump('F', 'C', "-1", L"NG_0", L"");

		return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
	}

	P_NH_JC_PrintQRCode(false);
	P_NH_JC_TransactionData();
	P_NH_NOR_TransactionPrint();
	P_NH_JC_ThankYou();

	return FID_NH_JUSTCASH_CLEANUP;
}

FLOW_ID CTranCmn::F_NH_JustCashCancelTransactionFlow()
{
	P_NH_JC_Cancel();

	return FID_NH_JUSTCASH_CLEANUP;
}

FLOW_ID CTranCmn::F_NH_JustCashCleanupFlow()
{
	_jcTransactionState.Reset();

	return FID_NH_NOR_CLEANUP;
}

#endif