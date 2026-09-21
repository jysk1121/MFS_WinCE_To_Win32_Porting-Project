#ifndef __TRAN_BIZ_FLOW_MX_H__
#define __TRAN_BIZ_FLOW_MX_H__

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
	int					m_nKindOfBinDetected;//[#2130] MX PCS 2012.04.03 

protected:
	void	BIZ_MX_StartFlowProc(FLOW_ID eFlowID);

//	FLOW_ID	F_NH_NOR_CleanUp();

	// Normal Flow
	FLOW_ID	F_MX_NOR_Common();
	FLOW_ID	F_MX_NOR_Withdrawal();
	FLOW_ID	F_MX_NOR_Inquiry();
//	FLOW_ID	F_NH_NOR_Transfer();
//	FLOW_ID F_NH_NOR_Reversal();
//	FLOW_ID	F_NH_NOR_Receipt();
//	FLOW_ID	F_NH_NOR_Cancel();
//	FLOW_ID	F_NH_NOR_Exit();

#endif 