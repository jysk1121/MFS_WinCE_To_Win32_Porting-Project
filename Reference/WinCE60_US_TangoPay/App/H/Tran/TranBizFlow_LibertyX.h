#pragma once
#if(APP_LIBERTYX)

//
// [#RWC6-59]	US William 2019.10.09 LibertyX
// [#RWC6-224]	US William 2020.10.12 LibertyX Dispense
//

public:
	FLOW_ID F_NH_LibertyXInit();
	FLOW_ID	F_NH_LibertyXPurchase();
	FLOW_ID F_NH_LibertyXDispense();

private:
	CString	GetAuthorizationNumber();
	BIZ_RETURN ReverseTransaction();
#endif
