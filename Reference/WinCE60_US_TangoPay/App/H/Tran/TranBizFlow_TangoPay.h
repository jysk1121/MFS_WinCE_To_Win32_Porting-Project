#pragma once

#if (APP_TANGOPAY)

// [RWC6-676] SKKim 2024.03.28 added new files
//
// TangoPay Flows
//

public:
	FLOW_ID F_NH_TangoPay_StartFlow();
	FLOW_ID F_NH_TangoPay_PreStaingFlow();
	FLOW_ID F_NH_TangoPay_AcceptCash();
	FLOW_ID F_NH_TangoPay_EndFlow();

	FLOW_ID F_NH_TangoPay_CancelFlow();
	FLOW_ID F_NH_TangoPay_CleanUpFlow();
#endif