#ifndef __TRAN_BIZ_PROC_AU_A_H__
#define __TRAN_BIZ_PROC_AU_A_H__

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
	////for check memory.
	//DWORD				m_dwInitPA, m_dwInitVA;

	//////////////////////////////////////
	// HELPER FUNCTION
public:
	//BOOL	IsPasswordMode();
	//BOOL	IsAdaTransaction();
	//BOOL	IsStartTransaction();

	//////////////////////////////////////
	// WORKING WITH HOST
public:
	//BOOL	BIZ_PowerOffReversal();
	//BOOL	BIZ_HostConfigProc(BOOL bShowScreen);
	//BOOL	BIZ_HostConfigExtendedProc();
	//BOOL	BIZ_AddCashProc(int nActuser);
	//BOOL	BIZ_DayTotalProc(BOOL bShow, BOOL bReal);
	//BOOL	BIZ_CheckCashDayTotalProc(BOOL bShow, BOOL bReal);
	//BOOL	BIZ_CassetteTotalProc(BOOL bReal, int nActUser);
	//BOOL	BIZ_DenominationProc();
	//BOOL	BIZ_HealthCheckProc(BOOL bShowScreen);

	//////////////////////////////////////
	// BUSINESS PROCESS
public:
	//BOOL	BIZ_WelcomeProc(BOOL bForceShowing = FALSE);

protected:
	//BIZ_RETURN	P_NH_NOR_CleanUp();
	//BIZ_RETURN	P_NH_NOR_DisplayError();

	//// Common
	//BIZ_RETURN	P_NH_NOR_ReadCard();			// Read Card
	//BIZ_RETURN	P_NH_NOR_SelectLanguage();		// Select Language
	//BIZ_RETURN	P_NH_NOR_EnterPassword();		// Enter Password

	//// Pre-Balance
	//BIZ_RETURN	P_NH_NOR_StartPreBalance();		// Pre Balance Start KSK 2010.01.25

	//// Select Transaction
	//BIZ_RETURN	P_NH_NOR_SelectTransaction();	// Select Transaction

	//// Withdrawal
	//BIZ_RETURN	P_NH_NOR_DisplaySurcharge(int nSurchargeOption);	// Display Surcharge

	//BIZ_RETURN	P_NH_NOR_ReceiveReceipt();		// Receive Receopt
	//BIZ_RETURN	P_NH_NOR_ErrorReceipt();
	//BIZ_RETURN	P_NH_NOR_SelectCWAccount();		// Select Cash Withdrawal Account
	//BIZ_RETURN	P_NH_NOR_SelectBIAccount();		// Select Balance Inquiry Account
	//BIZ_RETURN	P_NH_NOR_SelectTRAccount();		// Select Transfer Account
	//BIZ_RETURN	P_NH_NOR_InputCWAmount(BOOL bUseFastCash = TRUE);		// Input Cash Withdrawal Amount
	//BIZ_RETURN	P_NH_NOR_InputTRAmount();		// Input Transfer Amount

	//BIZ_RETURN	P_NH_NOR_CashDispense();
	//BIZ_RETURN	P_NH_NOR_SaveCWStatus(BOOL bNonCash);
	//BIZ_RETURN	P_NH_NOR_TakeCash();
	//BIZ_RETURN	P_NH_NOR_Transaction(BOOL bShowScreen);			// Transaction with HOST
	//BIZ_RETURN	P_NH_NOR_Reversal();
	//BIZ_RETURN	P_NH_NOR_ContiueReversal();
	//BIZ_RETURN	P_NH_NOR_PrintReceipt();
	//BIZ_RETURN	P_NH_NOR_TakeReceipt();
	//BIZ_RETURN	P_NH_NOR_AnotherTransaction();
	//BIZ_RETURN	P_NH_NOR_DisplayOnReceipt();
	//BIZ_RETURN	P_NH_NOR_DisplayCancel(BIZ_RETURN nReason);
	//BIZ_RETURN	P_NH_NOR_DisplayExit();

	//BIZ_RETURN	P_NH_CCW_InputAuthorizationNumber();
	//BIZ_RETURN	P_NH_CCW_InputCCWPassword();

	//// Enhanced Coupon
	//BIZ_RETURN	P_NH_NOR_SelectEnhancedCoupon();

	//// ADA
	//BIZ_RETURN	P_NH_ADA_Welcome();
	//BIZ_RETURN	P_NH_ADA_ReadCard();			// Read Card
	//BIZ_RETURN	P_NH_ADA_EnterPassword();
	//BIZ_RETURN	P_NH_ADA_SelectTransaction();	// Select Transaction
	//BIZ_RETURN	P_NH_ADA_DisplaySurcharge(int nSurchargeOption);	// Display Surcharge
	//
	//BIZ_RETURN	P_NH_ADA_ReceiveReceipt();		// Receive Receopt
	//BIZ_RETURN	P_NH_ADA_ErrorReceipt();
	//BIZ_RETURN	P_NH_ADA_SelectCWAccount();		// Select Cash Withdrawal Account
	//BIZ_RETURN	P_NH_ADA_SelectBIAccount();		// Select Balance Inquiry Account
	//BIZ_RETURN	P_NH_ADA_SelectTRAccount();		// Select Transfer Account
	//BIZ_RETURN	P_NH_ADA_InputCWAmount(BOOL bUseFastCash = TRUE);		// Input Cash Withdrawal Amount
	//BIZ_RETURN	P_NH_ADA_InputTRAmount();		// Input Transfer Amount
	//BIZ_RETURN	P_NH_ADA_Transaction();			// Transaction with HOST
	//BIZ_RETURN	P_NH_ADA_Reversal();
	//BIZ_RETURN	P_NH_ADA_CashDispense();
	//BIZ_RETURN	P_NH_ADA_TakeCash();
	//BIZ_RETURN	P_NH_ADA_PrintReceipt();
	//BIZ_RETURN	P_NH_ADA_GuideBalanceAmount();
	//BIZ_RETURN	P_NH_ADA_TakeReceipt();
	//BIZ_RETURN	P_NH_ADA_DisplayCancel(BIZ_RETURN nReason);
	//BIZ_RETURN	P_NH_ADA_DisplayExit();

#endif 