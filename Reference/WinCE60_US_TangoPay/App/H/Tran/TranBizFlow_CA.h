#ifndef __TRAN_BIZ_FLOW_CA_H__
#define __TRAN_BIZ_FLOW_CA_H__

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
	void	BIZ_CA_StartFlowProc(FLOW_ID eFlowID);

	// Normal Flow
	FLOW_ID	F_CA_NOR_Common();
	FLOW_ID	F_CA_NOR_SelectTran();
	FLOW_ID	F_CA_NOR_Withdrawal();

	FLOW_ID	F_CA_ADA_Common();//[#2077] CA PCS 2011.06.23

	//FLOW_ID	F_CA_ADA_Withdrawal();//[#2077] CA PCS 2011.06.23

#endif 