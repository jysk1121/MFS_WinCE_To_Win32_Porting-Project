#ifndef __TRAN_BIZ_FLOW_H__
#define __TRAN_BIZ_FLOW_H__

typedef struct
{
	BOOL				bProcessing;
	int					nPrevFlowID;
	int					nCurFlowID;
	int					nNextFlowID;
	BIZ_RETURN			nReasonforCancel;
} BIZ_FLOW_INFO;

BIZ_FLOW_INFO			g_sBizFlowInfo;

protected:
	CStringArray		m_arHistory;
	PRE_BALANCE_STATUS	m_PreBalanceStatus;
	BOOL				m_bPowerOffReversal;
	CDualBalanceInquiry	m_DualBalance;										// [#2292] US Justin 2014.10.9 Dual Balance

//	void	BIZ_StartFlowProc(FLOW_ID eFlowID);
	void	BIZ_StartFlowProc(FLOW_ID eFlowID, int nEventKind = DEV_MCU);	// [#2325] NH KSK 2015.01.22

	FLOW_ID	F_NH_NOR_CleanUp();

	// Normal Flow
	FLOW_ID	F_NH_NOR_Common();
	FLOW_ID	F_NH_NOR_PreBalance();
	FLOW_ID	F_NH_NOR_SelectTran();
	FLOW_ID	F_NH_NOR_Withdrawal();
	FLOW_ID	F_NH_NOR_Inquiry();
	FLOW_ID	F_NH_NOR_Transfer();
	FLOW_ID	F_NH_NOR_PinChange();											// [#2150] US Justin 2012.10.04 Dynamic Flow ... Add Pin Change
	FLOW_ID	F_NH_NOR_Deposit();												// [#GLDV-3005] US Kook 2022.01.04 Side Car (BNA test purpose)
	FLOW_ID	F_NH_NOR_Reversal();
	FLOW_ID	F_NH_NOR_Receipt();
	FLOW_ID	F_NH_NOR_Cancel();
	FLOW_ID	F_NH_NOR_Exit();

	// ADA Mode Flow
	FLOW_ID	F_NH_ADA_Common();

	// [#2375] US Justin 2015.10.28 Disable Merged ADA Function
	#if !(US_VERSION)
	FLOW_ID	F_NH_ADA_Reversal();
	FLOW_ID	F_NH_ADA_Receipt();
	FLOW_ID	F_NH_ADA_Cancel();
	FLOW_ID	F_NH_ADA_Exit();
	#endif
	// End of [#2375]

	FLOW_ID	F_NH_NOR_ReadCardOrRFID();										// [#2144] US Justin 2012.08.30		Additional Service => Start ATM => Read Card
	FLOW_ID	F_NH_CardlessTransaction();										// [#2396] US Justin 2016.02.10 PAI Pin 4

	FLOW_ID	F_NH_BitcoinTransaction();										// [#RWC6-68] PAI Bitcoin button

	#if (APP_PAYDIANT_CCA)
	FLOW_ID	F_NH_PaypalCCATransaction();									// [#2446] US Justin 2016.09.22 Paypal CCA
	#endif

	// [#2471] US Justin 2017.02.01	Enable Pin4 and Popmoney to all customers
	/*
	#if (APP_CUSTOM_PAI)
//	FLOW_ID	F_NH_CardlessTransaction();										// [#2396] US Justin 2016.02.10 PAI Pin 4
	FLOW_ID	F_NH_POPMoney();												// [#2350] US Justin 2015.06.17 POP Money
	FLOW_ID	F_NH_Pin4Transaction();											// [#2396] US Justin 2016.02.10 PAI Pin 4
	#endif
	*/

	#if (APP_PIN4_CASHPICKUP)
	FLOW_ID	F_NH_Pin4Transaction();
	#endif

	#if (APP_POPMONEY)
	FLOW_ID	F_NH_POPMoney();
	#endif
	// End of [#2471]

	// [#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
	#if (APP_GPAY_GIFTCARD_PURCHASE)
	FLOW_ID	F_NH_GivePay_PurchaseGiftCard(BOOL bPiggyBack = FALSE);
	#endif
	// End of [#2513]

#endif