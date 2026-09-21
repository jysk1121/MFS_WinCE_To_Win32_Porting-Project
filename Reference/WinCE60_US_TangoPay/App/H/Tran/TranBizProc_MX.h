#ifndef __TRAN_BIZ_PROC_MX_H__
#define __TRAN_BIZ_PROC_MX_H__

#include ".\Tran\TranCmn.h"

private:
//	int					m_nBackScreenNumber;
//	NH_USERSELECTION	m_sUserSelection;
//	BOOL				m_bShowNotice;
//	BOOL				m_bAdaPasswordMode;
//	BOOL				m_bStartTransaction;
//	BOOL				m_bAdaTransaction;

	//for check memory.
//	DWORD				m_dwInitPA, m_dwInitVA;
//	DWORD				m_dwTranCount;

	//////////////////////////////////////
	// HELPER FUNCTION
public:
//	BOOL	IsPasswordMode();
//	BOOL	IsAdaTransaction();
//	BOOL	IsStartTransaction();

	//////////////////////////////////////
	// WORKING WITH HOST
public:
//	BOOL	BIZ_PowerOffReversal();
//	BOOL	BIZ_HostConfigProc(BOOL bShowScreen);
//	BOOL	BIZ_HostConfigExtendedProc();
//	BOOL	BIZ_AddCashProc();
//	BOOL	BIZ_DayTotalProc(BOOL bShow, BOOL bReal);
//	BOOL	BIZ_CheckCashDayTotalProc(BOOL bShow, BOOL bReal);
//	BOOL	BIZ_CassetteTotalProc(BOOL bReal);
//	BOOL	BIZ_DenominationProc();
//	BOOL	BIZ_HealthCheckProc(BOOL bShowScreen);

	//////////////////////////////////////
	// BUSINESS PROCESS
public:
//	BOOL	BIZ_WelcomeProc(BOOL bForceShowing = FALSE);
	BOOL	BIZ_MX_WelcomeProc(BOOL bForceShowing = FALSE);

protected:
//	BIZ_RETURN	P_NH_NOR_CleanUp();
//	BIZ_RETURN	P_NH_NOR_DisplayError();

	// Common
//	BIZ_RETURN	P_NH_NOR_ReadCard();			// Read Card
//	BIZ_RETURN	P_NH_NOR_SelectLanguage();		// Select Language
//	BIZ_RETURN	P_NH_NOR_EnterPassword();		// Enter Password

	// Pre-Balance
//	BIZ_RETURN	P_NH_NOR_StartPreBalance();		// Pre Balance Start KSK 2010.01.25

	// Select Transaction
//	BIZ_RETURN	P_NH_NOR_SelectTransaction();	// Select Transaction

	// Withdrawal
	BIZ_RETURN	P_MX_NOR_ExchangeRateForDomesticCard();				// [#2266] MX Justin 2014.05.07 Exchange Rate when a Domestic Card is used at USD Dispensing ATM
	BIZ_RETURN	P_MX_NOR_SurchargeFeeNotice();						// [#2137] MX PCS 2012.07.24
//	BIZ_RETURN	P_MX_NOR_DisplaySurcharge(int nSurchargeOption);	// Display Surcharge	[#2137] MX KSK 2012.07.25 사용 안함으로 변경
	BIZ_RETURN	P_MX_NOR_InputCWAmount(BOOL bUseFastCash = TRUE);	// Input Cash Withdrawal Amount
	BIZ_RETURN	P_MX_NOR_fnAPP_BankFeeDisplay();					// Bank name and Fee Display
	BIZ_RETURN	P_MX_NOR_ExchangeFeeNotice();						// [#2137] MX PCS 2012.06.27
	BIZ_RETURN	P_MX_NOR_CheckCustomer();							// [#2137] MX PCS 2012.07.23	
	BIZ_RETURN	P_MX_NOR_CustomerFinalConfirmation();				// [#2149] MX Justin 2012.09.21


#endif 