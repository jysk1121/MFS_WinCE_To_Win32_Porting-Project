#ifndef __TRAN_BIZ_PROC_AU_C_H__
#define __TRAN_BIZ_PROC_AU_C_H__

private:
	//int					m_nBackScreenNumber;
	//int					m_nBackList[6];
	//int					m_nBackIndex;
	//int					m_nEnableCount;

	//NH_USERSELECTION	m_sUserSelection;
	//BOOL				m_bShowNotice;
	//BOOL				m_bAdaPasswordMode;
	//BOOL				m_bStartTransaction;
	//BOOL				m_bAdaTransaction;
	//BOOL				m_bPrintImage;

public:
	//S_WEATHER_INFO		m_sWeatherInfo[2];
	//S_NOTICE_INFO		m_sNoticeInfo;
	//int					m_nTransactionCount;

	// [#2497] AU Kook 2017.09.14 ATS ALPHI
	BOOL				m_bLoyaltyCardLoaded;
	BOOL				m_bLoyaltyPointRedeemed;
	int					m_TranCodeAlphi;
	CString				m_strLoyaltyCardTrack2;			// [#2497] NH woooZ 2017.09.14  ALPHI server
	CString				m_strAvailableLoyaltyPoints;	// [#2497] AU Kook 2017.09.27 ATS ALPHI
	//CString				m_strOriginalSurchargeAmount;
	// end of [#2497]

	//////////////////////////////////////
	// HELPER FUNCTION
public:
	//BOOL	IsPasswordMode();
	//BOOL	IsAdaTransaction();
	//BOOL	IsStartTransaction();

	//////////////////////////////////////
	// WORKING WITH HOST
protected:
	BIZ_RETURN	P_AU_C_NOR_EnterPassword();		// Enter Password

	//// Select Transaction
	BIZ_RETURN	P_AU_C_NOR_SelectTransaction();	// Select Transaction

	//// Withdrawal
//	BIZ_RETURN	P_AU_C_NOR_DisplaySurcharge();	// Display Surcharge
	BIZ_RETURN	P_AU_C_NOR_DisplaySurcharge(int nReceivedSurcharge = 0);	// Display Surcharge [#2362] AU KSK 2015.07.20

	//BIZ_RETURN	P_NH_NOR_ReceiveReceipt();		// Receive Receipt
	BIZ_RETURN	P_AU_C_NOR_ErrorReceipt();
	BIZ_RETURN	P_AU_C_NOR_InputCWAmount(BOOL bUseFastCash = TRUE);		// Input Cash Withdrawal Amount
	BIZ_RETURN	P_AU_C_NOR_CashDispense();
	BIZ_RETURN	P_AU_C_NOR_DisplayExit();

	// KSK 2012.08.22 ADA 관련 함수는 ADA 정의 함수로 이동 (추후 ADA.h 생성 필요)

	// [#2494] AU woooz 2017.08.01
	BIZ_RETURN	P_NH_ATS_ReadCard();
	BIZ_RETURN 	P_NH_ATS_InsertCard(int pScreenNumber,bool pLoyaltyCard);
	BIZ_RETURN	P_NH_ATS_Transaction(BOOL bShowScreen);
	BIZ_RETURN	P_NH_ALPHI_FailureNotice(BIZ_RETURN nReason);

public:
	BIZ_RETURN	BIZ_ALPHI_LoopTest();
	// End of [#2494]
#endif 