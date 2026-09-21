#include "stdafx.h"
#include ".\Scr\ScrCtrl.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"
#include ".\Common\NHDbgApi.h"

#if (APP_CUSTOM_CASHDEPOT)

#define MAX_PHONE_ATTEMPTS 2

FLOW_ID CTranCmn::F_NH_CashDepotStartFlow()
{
	NVDump('O', 'C', "-1", L"", L"START");
	NHDBG((L"[CTranCmn::F_NH_CashDepotStartFlow]\n"));

	// Check Receipt Printer : Cancel if Printer is not online
	if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
	{
		NVDump('O', 'C', "-1", L"", L"NO_PRINTER");

		return FID_NH_JUSTCASH_CANCEL_TRANSACTION;
	}

	m_sUserSelection.nPrintReceipt = RCPT_PAPER; // Paper Receipt

	CString terminalId = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	terminalId.Trim();

	_cdCryptoState.TerminalId = terminalId;

	if (P_NH_NOR_SelectLanguage() != RES_OK)
	{
		NVDump('F', 'C', "-1", L"", L"NO_LANG");

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	CString errorMessage;
	CDProvidersResponse providers;

	if (P_NH_CD_GetProviders(providers, errorMessage) != RES_OK)
	{
		P_NH_CD_Error(errorMessage);

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	if (P_NH_CD_ChooseProvider(providers.Providers) != RES_OK)
	{
		return FID_NH_CASHDEPOT_CLEANUP;
	}

	CDClientTokenResponse response;

	if (P_NH_CD_GetClientToken(response, errorMessage) != RES_OK)
	{
		return FID_NH_CASHDEPOT_CLEANUP;
	}

	_cdCryptoState.AccessToken = response.AccessToken;

	return FID_NH_CASHDEPOT_GREET;
}

FLOW_ID CTranCmn::F_NH_CashDepotGreetFlow()
{
	CString code;
	CString phoneNumber;
	CString errorMessage;

	int phoneAttempts = 0;
	CDGreetProcID greetProc = CDGP_PhoneNumber;
	CDVerifyCodeResponse verifyCode;

	NVDump('O', 'C', "-1", L"", L"START");
	NHDBG((L"[CTranCmn::F_NH_CashdepotStartFlow]\n"));

	while (greetProc != CDGP_Complete)
	{
		switch (greetProc)
		{
		case CDGP_PhoneNumber:
		{
			bool retry = false;

			if (phoneAttempts != 0)
				retry = true;

			CString strTemp;

			if (P_NH_CD_EnterCustomerPhoneNumber(strTemp, retry) != RES_OK)
			{
				NVDump('O', 'C', "-1", L"", L"ENTER_PHONE");

				return FID_NH_CASHDEPOT_CLEANUP;
			}

			phoneNumber.Format(L"+1%s", strTemp);

			phoneAttempts++;
			greetProc = P_NH_CD_SendCode(phoneNumber, errorMessage);

			if (greetProc == CDGP_ConfirmCode)
				break;

			if (phoneAttempts >= MAX_PHONE_ATTEMPTS)
				greetProc = CDGP_Fail;
			else
				greetProc = CDGP_PhoneNumber;

			break;
		}
		case CDGP_ConfirmCode:
		{
			if (P_NH_CD_EnterSmsCode(code) != RES_OK)
			{
				NVDump('O', 'C', "-1", L"", L"SEND_CODE");

				return FID_NH_CASHDEPOT_CLEANUP;
			}

			if ((greetProc = P_NH_CD_VerifyCode(phoneNumber, code, verifyCode, errorMessage)) != CDGP_Complete)
			{
				NVDump('O', 'C', "-1", L"", L"VERIFY_CODE");

				return FID_NH_CASHDEPOT_CLEANUP;
			}

			_cdCryptoState.UserToken = verifyCode.JWT;
			_cdCryptoState.CustomerId = verifyCode.CustomerId;
			break;
		}
		default:
		{
			P_NH_CD_Error(errorMessage);

			return FID_NH_CASHDEPOT_CLEANUP;
		}
		}
	}

	_cdCryptoState.Phone = phoneNumber;

	return FID_NH_CASHDEPOT_TRANSACTION;
}

FLOW_ID CTranCmn::F_NH_CashDepotTransactionFlow()
{
	CString errorMessage;
	CDDisclosureResponse disclosure;

	if (P_NH_CD_Disclosure(errorMessage, disclosure) != RES_OK)
	{
		P_NH_CD_Error(errorMessage);

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	if (P_NH_CD_TermsOfService() != RES_OK)
	{
		NVDump('O', 'C', "-1", L"", L"NO_TOS");

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	CDDisclosureAcceptResponse disclosureAccept;

	if (P_NH_CD_DisclosureAccept(errorMessage, disclosure, disclosureAccept) != RES_OK)
	{
		P_NH_CD_Error(errorMessage);

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	_cdCryptoState.DisclosureToken = disclosureAccept.DisclosureToken;

	CDDailyLimitResponse dailyLimit;

	if (P_NH_CD_DailyLimit(errorMessage, dailyLimit) != RES_OK)
	{
		P_NH_CD_Error(errorMessage);

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	_cdCryptoState.Currency = dailyLimit.Currency;
	_cdCryptoState.DailyLimit = dailyLimit.Value;

	CDCryptoCurrencyResponse cryptoCurrency;

	if (P_NH_CD_CryptoCurrency(errorMessage, cryptoCurrency) != RES_OK)
	{
		P_NH_CD_Error(errorMessage);

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	if (P_NH_CD_ChooseCryptoCurrency(cryptoCurrency.CryptoCurrencies) != RES_OK)
	{
		return FID_NH_CASHDEPOT_CLEANUP;
	}

	CDCryptoCurrencyPriceResponse cryptoCurrencyPrice;

	if (P_NH_CD_CryptoCurrencyPrice(errorMessage, cryptoCurrencyPrice) != RES_OK)
	{
		P_NH_CD_Error(errorMessage);

		return FID_NH_CASHDEPOT_CLEANUP;
	}
	
	_cdCryptoState.QuoteId = cryptoCurrencyPrice.Id;
	_cdCryptoState.Price = cryptoCurrencyPrice.Price;
	_cdCryptoState.CryptoRate = cryptoCurrencyPrice.Rates.Crypto;
	_cdCryptoState.CryptoPrice = cryptoCurrencyPrice.Rates.Fiat;
	_cdCryptoState.CoinType = cryptoCurrencyPrice.CoinType;
	_cdCryptoState.CoinName = cryptoCurrencyPrice.DisplayName;

	if (P_NH_CD_SendManualWalletLink(errorMessage) != RES_OK)
	{
		P_NH_CD_Error(errorMessage);

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	if (P_NH_CD_ConfirmManualWalletAddress(errorMessage) != RES_OK)
	{
		P_NH_CD_Error(errorMessage);

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	CDGetManualWalletAddressResponse getManualWalletAddress;

	if (P_NH_CD_GetManualWalletAddress(errorMessage, getManualWalletAddress) != RES_OK)
	{
		P_NH_CD_Error(errorMessage);

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	_cdCryptoState.WalletAddress = getManualWalletAddress.WalletAddress;

	CDCheckManualWalletAddressResponse checkManualWalletAddress;

	if (P_NH_CD_CheckManualWalletAddress(errorMessage, checkManualWalletAddress) != RES_OK)
	{
		P_NH_CD_Error(errorMessage);

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	if (checkManualWalletAddress.WalletAddress != _cdCryptoState.WalletAddress)
	{
		P_NH_CD_Error(errorMessage);

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	if (P_NH_CD_EnterTransactionAmount(&_cdCryptoState.TransactionAmount) != RES_OK)
	{
		return FID_NH_CASHDEPOT_CLEANUP;
	}

	if (P_NH_CD_TransactionSummary() != RES_OK)
	{
		NVDump('F', 'C', "1R", L"NG_6", L"");

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	return FID_NH_CASHDEPOT_DEBIT;
}

FLOW_ID CTranCmn::F_NH_CashDepotDebitFlow()
{
	CString errorMessage;
	CString errorCode;

	if (P_NH_CD_InsertCard() != RES_OK)
	{
		NVDump('F', 'C', "1R", L"NG_6", L"");

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	m_sUserSelection.strPassword.Empty();

	if (P_NH_NOR_EnterPassword() != RES_OK)
	{
		NVDump('F', 'C', "1R", L"NG_6", L"");

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	if (P_NH_CD_DebitAccount(errorCode, errorMessage) != RES_OK)
	{
		P_NH_CD_Error(errorMessage);
		NVDump('F', 'C', "1R", L"NG_6", L"");

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	CDBuyResponse buy;

	if (P_NH_CD_Buy(errorMessage, buy) != RES_OK)
	{
		P_NH_CD_Error(errorMessage);

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	_cdCryptoState.WalletTransactionId = buy.WalletTransactionId;

	return FID_NH_CASHDEPOT_END_TRANSACTION;
}

FLOW_ID CTranCmn::F_NH_CashDepotEndTransactionFlow()
{
	CDBills bills;
	CString errorMessage;
	CDBuyCompleteResponse buyComplete;

	if (P_NH_CD_BuyComplete(errorMessage, bills, buyComplete) != RES_OK)
	{
		P_NH_CD_Error(errorMessage);

		return FID_NH_CASHDEPOT_CLEANUP;
	}

	_cdCryptoState.Receipt.Total = buyComplete.Receipt.Total;
	_cdCryptoState.Receipt.Crypto = buyComplete.Receipt.Crypto;
	_cdCryptoState.Receipt.Fiat = buyComplete.Receipt.Fiat;
	_cdCryptoState.Receipt.Id = buyComplete.Receipt.Id;
	_cdCryptoState.Receipt.CryptoCurrency = buyComplete.Receipt.CryptoCurrency;
	_cdCryptoState.Receipt.Fee = buyComplete.Receipt.Fee;
	_cdCryptoState.Receipt.NetworkFee = buyComplete.Receipt.NetworkFee;
	_cdCryptoState.Receipt.Timestamp = buyComplete.Receipt.Timestamp;
	
	_cdCryptoState.Receipt.Rates.Fiat = buyComplete.Receipt.Rates.Fiat;
	_cdCryptoState.Receipt.Rates.Crypto = buyComplete.Receipt.Rates.Crypto;

	P_NH_CD_PrintReceipt();

	return FID_NH_CASHDEPOT_CLEANUP;
}

FLOW_ID CTranCmn::F_NH_CashDepotCancelTransactionFlow()
{
	P_NH_CD_ReturnCard();
	P_NH_CD_Cancel();

	return FID_NH_CASHDEPOT_CLEANUP;
}

FLOW_ID CTranCmn::F_NH_CashDepotCleanupFlow()
{
	P_NH_CD_ReturnCard();
	P_NH_NOR_DisplayExit();
	_cdCryptoState.Reset();

	return FID_NH_NOR_CLEANUP;
}

#endif