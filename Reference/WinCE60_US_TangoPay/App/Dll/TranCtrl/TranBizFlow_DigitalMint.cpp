#include "stdafx.h"
#include ".\Scr\ScrCtrl.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"
#include ".\Common\NHDbgApi.h"

#if (APP_DIGITALMINT)

#define MAX_PIN_ATTEMPTS	2
#define MAX_SCAN_ATTEMPTS	2
#define MAX_PHONE_ATTEMPTS	2

//------------------------------------------------------------------
//	Define Symbols
//------------------------------------------------------------------
extern CTimeCheck	g_TimeCheck;
extern CDevCmn*		m_pDevCmn;

FLOW_ID CTranCmn::F_NH_DigitalMintStartFlow()
{
	NVDump('O', 'C', "1P", L"", L"START");
	NHDBG((L"[CTranCmn::F_NH_DigitalMintStartFlow]\n"));

	F_NH_DigitalMintReportDeviceStatusFlow(true);

	if (P_NH_NOR_SelectLanguage(392) != RES_OK)
	{
		NVDump('F', 'C', "1P", L"", L"NO_LANG");

		return FID_NH_DIGITALMINT_CLEANUP;
	}

	BOOL atmAvailable = H_NH_DM_IsAtmModeAvailable(m_DMConfig);
	BOOL sidecarAvailable = H_NH_DM_IsSidecarModeAvailable(m_DMConfig);

	if (atmAvailable && !sidecarAvailable)
	{
		if (IsAdaTransaction())
		{
			m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Auto_DebitSelected.wav", TRUE);
		}

		_dmTransactionState.FlowSelection = DMF_ATM;
		H_NH_DM_SetTransactionMode(DMTXNMODE_ATM);
	}
	else if (!atmAvailable && sidecarAvailable)
	{
		if (IsAdaTransaction())
		{
			m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Auto_CashSelected.wav", TRUE);
		}

		_dmTransactionState.FlowSelection = DMF_Sidecar;
		H_NH_DM_SetTransactionMode(DMTXNMODE_SIDECAR);
	}
	else if (atmAvailable && sidecarAvailable)
	{
		if (P_NH_DM_ChooseFlow(&_dmTransactionState.FlowSelection) != RES_OK)
		{
			NVDump('O', 'C', "1P", L"", L"NO_FLOW");

			return FID_NH_DIGITALMINT_CLEANUP;
		}
	}
	else
	{
		NVDump('O', 'C', "1P", L"", L"NO_FLOW_AVAILABLE");
		return FID_NH_DIGITALMINT_CLEANUP;
	}

	return FID_NH_DIGITALMINT_GREET;
}

FLOW_ID CTranCmn::F_NH_DigitalMintGreetFlow()
{
	CString errorMessage;
	DMGreetProcID greetProc = DMGP_PhoneNumber;
	DMPinMode pinMode = DMPM_GetPin;
	int pinAttempts = 0;
	int phoneAttempts = 0;

	CString phoneNumber;
	CString pin;
	CString code;
	CString rawData;

	DMEnrollmentRequest enroll;

	NVDump('O', 'C', "1Q", L"", L"START");
	NHDBG((L"[CTranCmn::F_NH_DigitalMintStartFlow]\n"));

	while (greetProc != DMGP_Complete)
	{
		switch (greetProc)
		{
		case DMGP_PhoneNumber:
		{
			bool retry = false;

			if (phoneAttempts != 0)
			{
				retry = true;
			}

			if (P_NH_DM_EnterCustomerPhoneNumber(phoneNumber, retry) != RES_OK)
			{
				NVDump('O', 'C', "1Q", L"", L"NO_PHONE");

				return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
			}

			phoneAttempts++;
			greetProc = P_NH_DM_Greet(phoneNumber, errorMessage);

			if (greetProc == DMGP_PIN)
			{
				break;
			}

			if (phoneAttempts >= MAX_PHONE_ATTEMPTS)
			{
				greetProc = DMGP_Fail;
			}
			else
			{
				if (_dmTransactionState.CancelCode != DMCC_CustomerEnrollmentIssue)
				{
					greetProc = DMGP_PhoneNumber;
				}
			}

			break;
		}
		case DMGP_PINRetry:
		case DMGP_PIN:
			if (pinAttempts != 0)
			{
				pinMode = DMPM_RetryPin;
			}

			if (P_NH_DM_EnterPin(pinMode, pin) != RES_OK)
			{
				NVDump('O', 'C', "1Q", L"", L"NO_PIN");

				return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
			}

			if (pin == L"RESETPIN")
			{
				greetProc = P_NH_DM_GreetReconfirm(phoneNumber, _dmTransactionState.SessionId, _dmTransactionState.AccountId, errorMessage);
				break;
			}

			pinAttempts++;
			greetProc = P_NH_DM_GreetPin(phoneNumber, pin, _dmTransactionState.SessionId, _dmTransactionState.AccountId, errorMessage);

			if (greetProc == DMGP_Complete)
			{
				break;
			}

			if (pinAttempts >= MAX_PIN_ATTEMPTS)
			{
				greetProc = DMGP_Fail;
			}

			break;

		case DMGP_NewPIN:
		case DMGP_ResetPIN:
			if (P_NH_DM_CreatePin(pin) != RES_OK)
			{
				NVDump('O', 'C', "1Q", L"", L"NO_PIN");

				return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
			}

			if (greetProc == DMGP_ResetPIN)
			{
				greetProc = P_NH_DM_GreetResetPin(phoneNumber, pin, _dmTransactionState.SessionId, _dmTransactionState.AccountId, errorMessage);
			}
			else
			{
				greetProc = P_NH_DM_GreetNewPin(phoneNumber, pin, _dmTransactionState.SessionId, _dmTransactionState.AccountId, errorMessage);
			}

			break;

		case DMGP_ConfirmCode:
			if (P_NH_DM_EnterSmsCode(code) != RES_OK)
			{
				NVDump('O', 'C', "1Q", L"", L"NO_PIN");

				return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
			}

			greetProc = P_NH_DM_GreetCode(phoneNumber, code, _dmTransactionState.SessionId, _dmTransactionState.AccountId, errorMessage);
			break;

		case DMGP_ScanID:
		{
			int scanAttempts = 0;
			bool retry = false;

			while (scanAttempts < MAX_SCAN_ATTEMPTS)
			{
				if (scanAttempts != 0)
				{
					retry = true;
				}

				scanAttempts++;
				BIZ_RETURN ret = P_NH_DM_ScanID(rawData, retry);

				if (ret == RES_OK)
				{
					if (H_NH_DM_ParseID(enroll, phoneNumber, code, rawData) != RES_OK)
					{
						NVDump('O', 'C', "1Q", L"", L"BAD_ID_DATA");

						ret = RES_NG;
					}
					else
					{
						NVDump('O', 'C', "1Q", L"", L"ID_SCAN_OK");

						greetProc = DMGP_Enroll;
						break;
					}
				}

				// DMTODO: We are skipping take photo of DL
				// if (ret != RES_NG)
				// {
				// 	return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
				// }

				// if (scanAttempts >= MAX_SCAN_ATTEMPTS)
				// {
				// 	greetProc = DMGP_PhotoID;
				// 	break;
				// }

				if (ret != RES_NG || scanAttempts >= MAX_SCAN_ATTEMPTS)
				{
					return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
				}

				NVDump('O', 'C', "1Q", L"", L"BAD_ID_SCAN");
			}

			break;
		}
		case DMGP_PhotoID:
			if (P_NH_DM_PhotoID() != RES_OK)
			{
				NVDump('O', 'C', "1Q", L"", L"BAD_PHOTO_ID");

				return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
			}

			greetProc = DMGP_Enroll;
			break;

		case DMGP_Enroll:
			if (P_NH_DM_CreatePin(pin) != RES_OK)
			{
				NVDump('O', 'C', "1Q", L"", L"NO_PIN");

				return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
			}

			enroll.Pin = pin;

			greetProc = P_NH_DM_GreetEnroll(enroll, _dmTransactionState.SessionId, _dmTransactionState.AccountId, errorMessage);
			break;

		case DMGP_Pending:
			P_NH_DM_EnrollPending();

			return FID_NH_DIGITALMINT_CLEANUP;

		default:
			_dmTransactionState.CancelCode = DMCC_CustomerQuestionable;

			P_NH_DM_Error(errorMessage);

			return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
		}
	}

	return FID_NH_DIGITALMINT_TRANSACTION;
}

FLOW_ID CTranCmn::F_NH_DigitalMintTransactionFlowAtm()
{
	CString errorMessage;

	if (P_NH_DM_GetAccountLimits(errorMessage) != RES_OK)
	{
		P_NH_DM_Error(errorMessage);

		return FID_NH_DIGITALMINT_CLEANUP;
	}

	DMProductsResponse products;

	if (P_NH_DM_GetProducts(products, errorMessage) != RES_OK)
	{
		P_NH_DM_Error(errorMessage);

		return FID_NH_DIGITALMINT_CLEANUP;
	}

	DMProduct product;
	POSITION pos = NULL;
	pos = products.Products.GetHeadPosition();

	while (pos != NULL)
	{
		product = products.Products.GetNext(pos);

		if (product.Symbol == m_DMConfig.DefaultCoin)
		{
			_dmTransactionState.SelectedCoin = product;
			break;
		}
	}

	if (P_NH_DM_GetQuote(errorMessage) != RES_OK)
	{
		P_NH_DM_Error(errorMessage);

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	DMTransactionResponse transaction;
	DMInitiateTransactionRequest initiateTransaction;
	initiateTransaction.SessionHash = _dmTransactionState.SessionId;
	initiateTransaction.Side = DMSIDE_BUY;
	initiateTransaction.Symbol = _dmTransactionState.SelectedCoin.Symbol;

	if (P_NH_DM_IntiateTransaction(initiateTransaction, transaction, errorMessage) != RES_OK)
	{
		if (transaction.Code == DMR_CustomerNotApproved)
		{
			P_NH_DM_EnrollPending();
		}
		else
		{
			P_NH_DM_Error(errorMessage);
		}

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	_dmTransactionState.TransactionId = transaction.TransactionHash;
	_dmTransactionState.WalletAddress = transaction.WithdrawalAddress;

	if (P_NH_DM_ConfirmWallet() != RES_OK)
	{
		_dmTransactionState.CancelCode = DMCC_DefaultWalletIncorrect;

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	if (P_NH_DM_SetWalletAddress(errorMessage) != RES_OK)
	{
		P_NH_DM_Error(errorMessage);

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	if (P_NH_DM_ConfirmRates() != RES_OK)
	{
		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	if (P_NH_DM_EnterTransactionAmount(&_dmTransactionState.TransactionAmount) != RES_OK)
	{
		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	return FID_NH_DIGITALMINT_DEBIT;
}

FLOW_ID CTranCmn::F_NH_DigitalMintTransactionFlowSidecar()
{
	CString errorMessage;

	if (P_NH_DM_GetAccountLimits(errorMessage) != RES_OK)
	{
		P_NH_DM_Error(errorMessage);

		return FID_NH_DIGITALMINT_CLEANUP;
	}

	DMProductsResponse products;

	if (P_NH_DM_GetProducts(products, errorMessage) != RES_OK)
	{
		P_NH_DM_Error(errorMessage);

		return FID_NH_DIGITALMINT_CLEANUP;
	}

	if (P_NH_DM_SelectCoin(products.Products) != RES_OK)
	{
		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	if (P_NH_DM_GetQuote(errorMessage) != RES_OK)
	{
		P_NH_DM_Error(errorMessage);

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	DMTransactionResponse transaction;
	DMInitiateTransactionRequest initiateTransaction;
	initiateTransaction.SessionHash = _dmTransactionState.SessionId;
	initiateTransaction.Side = DMSIDE_BUY;
	initiateTransaction.Symbol = _dmTransactionState.SelectedCoin.Symbol;

	if (P_NH_DM_IntiateTransaction(initiateTransaction, transaction, errorMessage) != RES_OK)
	{
		if (transaction.Code == DMR_CustomerNotApproved)
		{
			P_NH_DM_EnrollPending();
		}
		else
		{
			P_NH_DM_Error(errorMessage);
		}

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	_dmTransactionState.TransactionId = transaction.TransactionHash;

	int scanAttempts = 0;
	bool retry = false;

	while (scanAttempts < MAX_SCAN_ATTEMPTS)
	{
		if (scanAttempts != 0)
		{
			retry = true;
		}

		scanAttempts++;
		BIZ_RETURN ret = P_NH_DM_ScanWallet(transaction.WithdrawalAddress, retry);

		if (ret == RES_OK)
		{
			_dmTransactionState.WalletAddress = transaction.WithdrawalAddress;
			ret = P_NH_DM_SetWalletAddress(errorMessage);

			if (ret == RES_NG)
			{
				P_NH_DM_Error(errorMessage);
			}

			if (ret == RES_OK)
			{
				break;
			}
		}

		if (ret != RES_NG || scanAttempts >= MAX_SCAN_ATTEMPTS)
		{
			return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
		}
	}

	return FID_NH_DIGITALMINT_ACCEPT_CASH;
}

FLOW_ID CTranCmn::F_NH_DigitalMintDebitFlow()
{
	CString errorMessage;
	CString errorCode;

	if (P_NH_DM_InsertCard() != RES_OK)
	{
		NVDump('F', 'C', "1R", L"NG_6", L"");

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	m_sUserSelection.strPassword.Empty();

	if (P_NH_NOR_EnterPassword(387) != RES_OK)
	{
		NVDump('F', 'C', "1R", L"NG_6", L"");

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	if (P_NH_DM_DebitAccount(errorCode, errorMessage) != RES_OK)
	{
		P_NH_DM_Error(errorMessage);
		NVDump('F', 'C', "1R", L"NG_6", L"");

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	if (P_NH_DM_ConfirmDebit(_dmTransactionState.TransactionAmount, errorMessage) != RES_OK)
	{
		P_NH_DM_Error(errorMessage);

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	return FID_NH_DIGITALMINT_END_TRANSACTION;
}

FLOW_ID CTranCmn::F_NH_DigitalMintCashAcceptFlow()
{
	CString errorMessage;
	CString errorCode;

	if (P_NH_DM_ConfirmRates() != RES_OK)
	{
		NVDump('F', 'C', "1S", L"NG_5", L"");

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	BIZ_RETURN ret = P_NH_DM_CollectCash(_dmTransactionState.TransactionAmount, errorMessage);

	if (ret != RES_OK)
	{
		if (ret == RES_NG)
		{
			P_NH_DM_Error(errorMessage);
		}

		NVDump('F', 'C', "1S", L"NG_6", L"");

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	if (!_dmTransactionState.TransactionAmount)
	{
		NVDump('F', 'C', "1S", L"NG_7", L"");

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	return FID_NH_DIGITALMINT_END_TRANSACTION;
}

FLOW_ID CTranCmn::F_NH_DigitalMintEndTransactionFlow()
{
	CString errorMessage;

	P_NH_DM_ReturnCard();
	P_NH_DM_TransactionSummary();

	if (P_NH_DM_CompleteTransaction(errorMessage) != RES_OK)
	{
		P_NH_DM_Error(errorMessage);

		return FID_NH_DIGITALMINT_CANCEL_TRANSACTION;
	}

	return FID_NH_DIGITALMINT_CLEANUP;
}

FLOW_ID CTranCmn::F_NH_DigitalMintCancelTransactionFlow()
{
	P_NH_DM_ReturnCard();
	P_NH_DM_Cancel();

	CString message;

	if (P_NH_DM_CancelTranscation(message) != RES_OK)
	{
		P_NH_DM_Error(message);
	}

	return FID_NH_DIGITALMINT_CLEANUP;
}

FLOW_ID CTranCmn::F_NH_DigitalMintCleanupFlow()
{
	P_NH_DM_ThankYou();
	_dmTransactionState.Reset();

	return FID_NH_NOR_CLEANUP;
}

FLOW_ID CTranCmn::F_NH_DigitalMintReportDeviceStatusFlow(bool forceUpdate)
{
	if (this->m_DMService)
	{
		H_NH_DM_ReportAtmState(m_DMConfig, forceUpdate);
		H_NH_DM_ReportSidecarState(m_DMConfig, forceUpdate);
	}

	return FID_END_OF_FLOW;
}

FLOW_ID CTranCmn::F_NH_DigitalMintReportDeviceEvent(DMKioskEventCode eventCode)
{
	if (this->m_DMService)
	{
		H_NH_DM_ReportKioskEvent(m_DMConfig, eventCode);
	}

	return FID_END_OF_FLOW;
}

#endif