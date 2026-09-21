#ifndef __TRAN_BIZ_FLOW_AU_C_H__
#define __TRAN_BIZ_FLOW_AU_C_H__


public:
//	BOOL	F_NH_NOR_AddCash(int nPrintDevice);

//	FLOW_ID	F_NH_NOR_CST_Total(BOOL bPrintFlag = TRUE);
//	FLOW_ID	F_NH_NOR_Open();
//	FLOW_ID	F_NH_NOR_ExtendedOpen();
//	FLOW_ID F_NH_NOR_Total();
//	FLOW_ID F_NH_NOR_TotalTrial();
//	FLOW_ID F_NH_NOR_CST_TotalTrial();
//	FLOW_ID F_NH_NOR_SET_Denomination();
//	FLOW_ID	F_NH_NOR_Healthcheck();
//	FLOW_ID F_NH_NOR_Detail_Healthcheck();

protected:
//	CStringArray		m_arHistory;
//	PRE_BALANCE_STATUS	m_PreBalanceStatus;
//	BOOL				m_bPowerOffReversal;

protected:
//	void	BIZ_AU_C_StartFlowProc(FLOW_ID eFlowID);
	void	BIZ_AU_C_StartFlowProc(FLOW_ID eFlowID, int nEventKind = DEV_MCU);	// [#2325] NH KSK 2015.01.22

	//// Normal Flow
	FLOW_ID	F_AU_C_NOR_Common();
	FLOW_ID	F_AU_C_NOR_SelectTran();
	FLOW_ID	F_AU_C_NOR_Withdrawal();
	FLOW_ID	F_AU_C_NOR_Inquiry();
	FLOW_ID	F_AU_C_NOR_Receipt();
	FLOW_ID	F_AU_C_NOR_Exit();

	// [#2375] US Justin 2015.10.28... Specify country... TO AVOID CONFUSION
	#if (AU_VERSION)
	// [#2047] AU KSK 2011.04.13
	FLOW_ID	F_AU_ADA_Withdrawal();
	FLOW_ID	F_AU_ADA_Inquiry();
	// end of [#2047]
	#endif

#endif 