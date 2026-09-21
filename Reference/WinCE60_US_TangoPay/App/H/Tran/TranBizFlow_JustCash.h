#pragma once
#if (APP_JUST_CASH)

//
// Just.Cash Flows
//

public:
	FLOW_ID F_NH_JustCashStartFlow();
	FLOW_ID F_NH_JustCashGreetFlow();
	FLOW_ID F_NH_JustCashTransactionFlowAtm();
	FLOW_ID F_NH_JustCashTransactionFlowSidecar();
	FLOW_ID F_NH_JustCashCardlessTransactionFlow();
	FLOW_ID F_NH_JustCashDebitFlow();
	FLOW_ID F_NH_JustCashAcceptCashFlow();
	FLOW_ID F_NH_JustCashEndTransactionFlow();
	FLOW_ID F_NH_JustCashCancelTransactionFlow();
	FLOW_ID F_NH_JustCashCleanupFlow();

#endif