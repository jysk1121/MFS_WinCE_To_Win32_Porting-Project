#pragma once
#if (APP_CUSTOM_CASHDEPOT)

#include "CashDepotTransactionState.h"

//
// Standard Flows
//

typedef struct _cd_transaction_state
{
	CString						TerminalId;
	CDConfigurationsResponse	Configurations;
	bool						RbiSelected;
	bool						ReceiptSelected;

	_cd_transaction_state::_cd_transaction_state()
	{
		Reset();
	};

	void Reset()
	{
		TerminalId.Empty();
		Configurations.Reset();
		RbiSelected = false;
		ReceiptSelected = false;
	};
} CDTransactionState;

//
/// Cryptocurrency Flows
//

typedef enum
{
	CDGP_PhoneNumber,
	CDGP_ConfirmCode,
	CDGP_Fail,
	CDGP_Complete
} CDGreetProcID;

typedef struct _cd_crypto_state
{
	CString		TerminalId;
	CDProvider	SelectedProvider;
	CString		AccessToken;
	CString		UserToken;
	CString		CustomerId;
	CString		Phone;
	CString		DisclosureToken;
	CString		Currency;
	CString		SelectedCurrency;
	int			DailyLimit;
	CString		CoinType;
	CString		CoinName;
	CString		QuoteId;
	double		Price;
	CString		CryptoRate;
	CString		CryptoPrice;
	CString		WalletAddress;
	int			TransactionAmount;
	CString		WalletTransactionId;
	CDReceipt	Receipt;

	_cd_crypto_state::_cd_crypto_state()
	{
		Reset();
	};

	void Reset()
	{
		TerminalId.Empty();
		SelectedProvider.Reset();
		AccessToken.Empty();
		UserToken.Empty();
		CustomerId.Empty();
		Phone.Empty();
		DisclosureToken.Empty();
		Currency.Empty();
		SelectedCurrency.Empty();
		DailyLimit = 0;
		CoinType.Empty();
		CoinName.Empty();
		QuoteId.Empty();
		Price = 0;
		CryptoRate.Empty();
		CryptoPrice.Empty();
		WalletAddress.Empty();
		TransactionAmount = 0;
		WalletTransactionId.Empty();
		Receipt.Reset();
	};
} CDCryptoState;

//
// CashDepot Processes
//

private:
	//
	// Standard Flows
	//

	CDTransactionState _cdTransactionState;

	// Initialization procs
	BIZ_RETURN P_NH_CD_GetConfigurations();

	// Transaction procs
	BIZ_RETURN P_NH_CD_RemainingBalanceInquiry();
	BIZ_RETURN P_NH_CD_PleaseWait();

	//
	/// Cryptocurrency Flows
	//

	CDCryptoState _cdCryptoState;

	CString GenerateUUIDv4();

	// Greet/Initialization procs
	BIZ_RETURN P_NH_CD_ChooseProvider(CDProviders &providers);
	BIZ_RETURN P_NH_CD_EnterCustomerPhoneNumber(CString &phoneNumber, bool retry);
	BIZ_RETURN P_NH_CD_EnterSmsCode(CString &code);

	// Transaction procs
	BIZ_RETURN P_NH_CD_TermsOfService();
	BIZ_RETURN P_NH_CD_ChooseCryptoCurrency(CDCryptoCurrencies &cryptoCurrencies);
	BIZ_RETURN P_NH_CD_ConfirmManualWalletAddress(CString &errorMessage);
	BIZ_RETURN P_NH_CD_EnterTransactionAmount(int *transactionAmount);
	BIZ_RETURN P_NH_CD_TransactionSummary();

	// Debit procs
	BIZ_RETURN P_NH_CD_InsertCard();
	BIZ_RETURN P_NH_CD_DebitAccount(CString &errorCode, CString &errorMessage);

	// End Transaction procs
	BIZ_RETURN P_NH_CD_Error(CString message);
	BIZ_RETURN P_NH_CD_Cancel();
	BIZ_RETURN P_NH_CD_ReturnCard();
	BIZ_RETURN P_NH_CD_PrintReceipt();

	// Communication procs
	BIZ_RETURN		P_NH_CD_GetProviders(CDProvidersResponse &providers, CString &errorMessage);
	BIZ_RETURN		P_NH_CD_GetClientToken(CDClientTokenResponse &clientToken, CString &errorMessage);
	CDGreetProcID	P_NH_CD_SendCode(CString phoneNumber, CString &errorMessage);
	CDGreetProcID	P_NH_CD_VerifyCode(CString phoneNumber, CString code, CDVerifyCodeResponse &verifyCode, CString &errorMessage);
	BIZ_RETURN		P_NH_CD_Disclosure(CString &errorMessage, CDDisclosureResponse &disclosure);
	BIZ_RETURN		P_NH_CD_DisclosureAccept(CString &errorMessage, CDDisclosureResponse &disclosure, CDDisclosureAcceptResponse &disclosureAccept);
	BIZ_RETURN		P_NH_CD_DailyLimit(CString &errorMessage, CDDailyLimitResponse &dailyLimit);
	BIZ_RETURN		P_NH_CD_CryptoCurrency(CString &errorMessage, CDCryptoCurrencyResponse &cryptoCurrency);
	BIZ_RETURN		P_NH_CD_CryptoCurrencyPrice(CString &errorMessage, CDCryptoCurrencyPriceResponse &cryptoCurrencyPrice);
	BIZ_RETURN		P_NH_CD_SendManualWalletLink(CString &errorMessage);
	BIZ_RETURN		P_NH_CD_GetManualWalletAddress(CString &errorMessage, CDGetManualWalletAddressResponse &getManualWalletAddress);
	BIZ_RETURN		P_NH_CD_CheckManualWalletAddress(CString &errorMessage, CDCheckManualWalletAddressResponse &checkManualWalletAddress);
	BIZ_RETURN		P_NH_CD_Buy(CString &errorMessage, CDBuyResponse &buy);
	BIZ_RETURN		P_NH_CD_BuyComplete(CString &errorMessage, CDBills &bills, CDBuyCompleteResponse &buyComplete);

#endif