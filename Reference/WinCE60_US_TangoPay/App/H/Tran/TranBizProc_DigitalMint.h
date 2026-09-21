#pragma once
#if (APP_DIGITALMINT)

#include "DigitalMintTransactionState.h"

typedef enum
{
	DMPM_GetPin,

	DMPM_SetPin,

	DMPM_RetryPin,

	DMPM_ConfirmCode
} DMPinMode;

typedef enum
{
	DMF_UNREGISTERED,

	DMF_ATM,

	DMF_Sidecar
} DMFlowSelection;

typedef enum
{
	DMGP_PhoneNumber,

	DMGP_PIN,

	DMGP_PINRetry,

	DMGP_ConfirmCode,

	DMGP_ScanID,

	DMGP_PhotoID,

	DMGP_NewPIN,

	DMGP_ResetPIN,

	DMGP_Enroll,

	DMGP_Fail,

	DMGP_Pending,

	// The greet process is complete!
	DMGP_Complete
} DMGreetProcID;

typedef struct _dm_transaction_state
{
	CString							SessionId;
	CString							TransactionId;
	CString							AccountId;
	int								TransactionAmount;
	DMFlowSelection					FlowSelection;
	DMCancelTransactionReasonCode	CancelCode;
	DMAccountLimitsResponse			AccountLimits;
	DMProduct						SelectedCoin;
	CString							WalletAddress;
	DMQuoteResponse					Quote;
	DMResponseType					LastResponseCode;
	bool							UsePutForCode;

	_dm_transaction_state::_dm_transaction_state()
	{
		TransactionAmount = 0;
		FlowSelection = DMF_UNREGISTERED;
		CancelCode = DMCC_CustomerChangedMind;
	};

	void Reset()
	{
		SessionId.Empty();
		TransactionId.Empty();
		AccountId.Empty();
		TransactionAmount = 0;
		FlowSelection = DMF_UNREGISTERED;
		CancelCode = DMCC_CustomerChangedMind;
		AccountLimits.Reset();
		SelectedCoin.Reset();
		WalletAddress.Empty();
		Quote.Reset();
		LastResponseCode = DMR_None;
		UsePutForCode = false;
	};
} DMTransactionState;

//
// DigitalMint Processes
//

private:
	DMTransactionState	_dmTransactionState;

	// Greet/Initialization procs
	BIZ_RETURN P_NH_DM_PleaseWait();
	BIZ_RETURN P_NH_DM_SendTOS();

	// Helper functions
	BIZ_RETURN	H_NH_DM_SetTransactionMode(DMTRANSACTIONMODE transactionMode);
	BIZ_RETURN	H_NH_DM_ParseID(DMEnrollmentRequest &enroll, CString phoneNumber, CString code, CString rawData);
	CTime		H_NH_DM_ParseDate(CString dateTime);
	CString		H_NH_DM_ParseDate(CString dateTime, CString format);

	// Helper functions for collect cash
	bool H_NH_DM_StartCashIn();
	bool H_NH_DM_StartAcceptCash();
	bool H_NH_DM_StoreCash();
	bool H_NH_DM_ReturnCash();
	int  H_NH_DM_GetTransactionLimit();

public:
	// Greet/Initialization procs
	BIZ_RETURN P_NH_DM_ChooseFlow(DMFlowSelection *selection);
	BIZ_RETURN P_NH_DM_EnterCustomerPhoneNumber(CString &phoneNumber, bool retry);
	BIZ_RETURN P_NH_DM_EnterPin(DMPinMode mode, CString &pin);
	BIZ_RETURN P_NH_DM_CreatePin(CString &pin);
	BIZ_RETURN P_NH_DM_EnterSmsCode(CString &code);
	BIZ_RETURN P_NH_DM_ScanID(CString &rawData, bool retry);
	BIZ_RETURN P_NH_DM_PhotoID();

	// Transaction procs
	BIZ_RETURN P_NH_DM_SelectCoin(DMProducts &products);
	BIZ_RETURN P_NH_DM_ConfirmRates();
	BIZ_RETURN P_NH_DM_EnterTransactionAmount(int *purchaseAmount);
	BIZ_RETURN P_NH_DM_ConfirmWallet();
	BIZ_RETURN P_NH_DM_ScanWallet(CString &walletAddress, bool retry);

	// Debit procs
	BIZ_RETURN P_NH_DM_InsertCard();
	BIZ_RETURN P_NH_DM_DebitAccount(CString &errorCode, CString &errorMessage);

	// Collect Cash procs
	BIZ_RETURN P_NH_DM_CollectCashTOS();
	BIZ_RETURN P_NH_DM_CollectCash(int &transactionAmount, CString &errorMessage);

	// End Transaction procs
	BIZ_RETURN P_NH_DM_TransactionSummary();
	BIZ_RETURN P_NH_DM_CompleteTransaction(CString &errorMessage);
	BIZ_RETURN P_NH_DM_CancelTranscation(CString &message);
	BIZ_RETURN P_NH_DM_Error(CString message);
	BIZ_RETURN P_NH_DM_ThankYou();
	BIZ_RETURN P_NH_DM_Cancel();
	BIZ_RETURN P_NH_DM_Info(CString message);
	BIZ_RETURN P_NH_DM_ReturnCard();

	// Communication procs
	DMGreetProcID	P_NH_DM_Greet(CString phoneNumber, CString &errorMessage);
	DMGreetProcID	P_NH_DM_GreetPin(CString phoneNumber, CString PIN, CString &sessionId, CString &accountId, CString &errorMessage);
	DMGreetProcID	P_NH_DM_GreetNewPin(CString phoneNumber, CString PIN, CString &sessionId, CString &accountId, CString &errorMessage);
	DMGreetProcID	P_NH_DM_GreetReconfirm(CString phoneNumber, CString &sessionId, CString &accountId, CString &errorMessage);
	DMGreetProcID	P_NH_DM_GreetResetPin(CString phoneNumber, CString PIN, CString &sessionId, CString &accountId, CString &errorMessage);
	DMGreetProcID	P_NH_DM_GreetEnroll(DMEnrollmentRequest enroll, CString &sessionId, CString &accountId, CString &errorMessage);
	DMGreetProcID	P_NH_DM_GreetCode(CString phoneNumber, CString code, CString &sessionId, CString &accountId, CString &errorMessage);
	BIZ_RETURN		P_NH_DM_EnrollPending();
	BIZ_RETURN		P_NH_DM_GetAccountLimits(CString &errorMessage);
	BIZ_RETURN		P_NH_DM_GetProducts(DMProductsResponse &products, CString &errorMessage);
	BIZ_RETURN		P_NH_DM_IntiateTransaction(DMInitiateTransactionRequest &request, DMTransactionResponse &transaction, CString &errorMessage);
	BIZ_RETURN		P_NH_DM_SetWalletAddress(CString &errorMessage);
	BIZ_RETURN		P_NH_DM_GetQuote(CString &errorMessage);
	BIZ_RETURN		P_NH_DM_ConfirmDebit(int amount, CString &errorMessage, bool showPleaseWait = true);

#endif