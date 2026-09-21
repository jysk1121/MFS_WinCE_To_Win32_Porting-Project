#pragma once
#if (APP_CUSTOM_CASHDEPOT)

//
// Cash Depot Flows
//

public:
	FLOW_ID F_NH_CashDepotStartFlow();
	FLOW_ID F_NH_CashDepotGreetFlow();
	FLOW_ID F_NH_CashDepotTransactionFlow();
	FLOW_ID F_NH_CashDepotEndTransactionFlow();
	FLOW_ID F_NH_CashDepotCancelTransactionFlow();
	FLOW_ID F_NH_CashDepotDebitFlow();
	FLOW_ID F_NH_CashDepotCleanupFlow();

#endif