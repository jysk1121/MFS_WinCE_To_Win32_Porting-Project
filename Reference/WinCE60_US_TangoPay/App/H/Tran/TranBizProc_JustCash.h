#pragma once
#if (APP_JUST_CASH)

typedef enum
{
	JCF_DISABLED,
	JCF_BITCOIN,
	JCF_CARDLESS
} JCFlowSelection;

typedef enum
{
	JCP_NONE,
	JCP_DEBIT,
	JCP_CASH
} JCPaymentSelection;

typedef struct _jc_transaction_state
{
	JCFlowSelection		FlowSelection;
	JCPaymentSelection	PaymentSelection;

	_jc_transaction_state::_jc_transaction_state()
	{
		FlowSelection = JCF_DISABLED;
		PaymentSelection = JCP_NONE;
	};

	void Reset()
	{
		FlowSelection = JCF_DISABLED;
		PaymentSelection = JCP_NONE;
	};
} JCTransactionState;

//
// Just.Cash Processes
//

private:
	JCTransactionState _jcTransactionState;

	BOOL H_NH_JC_IsJustCashAvailable();
	BOOL H_NH_JC_IsJustCashCardless();
	BOOL H_NH_JC_IsJustCashBitcoin();
	BOOL H_NH_JC_IsAtmModeAvailable();
	BOOL H_NH_JC_IsSidecarModeAvailable();

	// Helper functions for collect cash
	bool	H_NH_JC_StartCashIn();
	bool	H_NH_JC_StartAcceptCash();
	bool	H_NH_JC_StoreCash();
	bool	H_NH_JC_ReturnCash();

public:
	// Greet/Initialization procs
	BIZ_RETURN P_NH_JC_ChooseFlow(JCFlowSelection *selection);
	BIZ_RETURN P_NH_JC_ChoosePayment(JCPaymentSelection *selection);
	BIZ_RETURN P_NH_JC_EnterCustomerPhoneNumber(CString &phoneNumber);
	BIZ_RETURN P_NH_JC_EnterSmsCode(CString &code);

	// Transaction procs
	BIZ_RETURN P_NH_JC_SelectCoin();
	BIZ_RETURN P_NH_JC_Purchase();
	BIZ_RETURN P_NH_JC_ApproveTransaction();
	BIZ_RETURN P_NH_JC_Processor_Auth();
	BIZ_RETURN P_NH_JC_PrintQRCode(bool debit);
	BIZ_RETURN P_NH_JC_Commit(int nResult, BOOL bShowScreen = TRUE);
	BIZ_RETURN P_NH_JC_ConfirmWallet();

	// Debit procs
	BIZ_RETURN P_NH_JC_InsertCard();
	BIZ_RETURN P_NH_JC_EnterAmount();

	// Collect Cash procs
	BIZ_RETURN P_NH_JC_CollectCash();
	BIZ_RETURN P_NH_JC_ConfirmDebit(int amount);

	// End Transaction procs
	BIZ_RETURN P_NH_JC_CompleteTransaction();
	BIZ_RETURN P_NH_JC_Error();
	BIZ_RETURN P_NH_JC_TransactionData();
	BIZ_RETURN P_NH_JC_ThankYou();
	BIZ_RETURN P_NH_JC_Cancel();

	// Cash Code Transaction
	BIZ_RETURN P_NH_JC_Enter_CashCode();
	BIZ_RETURN P_NH_JC_Submit_CashCode();
	BIZ_RETURN P_NH_JC_Authorization();
	BIZ_RETURN P_NH_JC_UpdateTransaction(int nDispResult);

#endif