#include "stdafx.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

//------------------------------------------------------------------
//	Define Symbol
//------------------------------------------------------------------
extern CTimeCheck	g_TimeCheck;

#ifdef APP_AGING_MODE
	#define	AGING_KEYIN_TIME 1
	extern CTimeCheck g_AgingCheck;
#endif
//------------------------------------------------------------------
//	Implement
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_AU_C_NOR_SelectTransaction()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Select Transaction
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_AU_C_NOR_SelectTransaction()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"SelectTran");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_AU_C_NOR_SelectTransaction]\n"));

	int				nIndex;
	CString			GetKeyStr;
	BOOL			bShowScreen = TRUE;

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(105);

			// Setting Screen
			{
				// History
				for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
				{
					if (nHistory == 0)
						m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

					m_pDevCmn->fnSCR_DisplayHistory(nHistory+1, m_arHistory[nHistory]);
				}

				// Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_105001));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_105002));

				nIndex = 4;

				// 1. Check Withdrawal Status
				if (m_pDevCmn->WithAvail != TRAN_WITH_NOT)
					m_pDevCmn->fnSCR_DisplayString(nIndex++, L"WITHDRAWAL");

				m_pDevCmn->fnSCR_DisplayString(nIndex++, L"BALANCE");
			}

			m_pDevCmn->fnSCR_DisplayScreen(105, KEYIN_TIME_OUT, PIN_MENU_MODE);		// [#158] KSK 2008.04.22
			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
		}

#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			if ((m_nTransactionCount % 2) == 0)
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_WITHDRAWAL));
				TranCode = TC_WITHDRAWAL;
			}
			else
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_BALANCE));
				TranCode = TC_INQUIRY;
			}
			return RES_OK;
		}
#else
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_SELECT_TRAN, L"USER CANCEL");
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
#if (AU_VERSION)	// [#2375] US JUSTIN Add Country restriction.
				if ( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME) == 0
					||  P_NH_NOR_NeedMoreTime() != RES_YES )
				{
					m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_SELECT_TRAN, L"TIMEOUT");
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					return RES_USER_TIMEOUT;
				}
				bShowScreen = TRUE;
#else
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				return RES_USER_TIMEOUT;
#endif //end of [#2375]
			}
			else if (GetKeyStr == L"WITHDRAWAL")
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_WITHDRAWAL));
				TranCode = TC_WITHDRAWAL;
				return RES_OK;
			}
			else if (GetKeyStr == L"BALANCE")
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_BALANCE));
				TranCode = TC_INQUIRY;
				return RES_OK;
			}
			//else if (GetKeyStr == L"TRANSFER")
			//{
			//	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_TRANSFER));
			//	TranCode = TC_TRANSFER;
			//	return RES_OK;
			//}
		}
#endif

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_AU_C_NOR_DisplaySurcharge()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Display Surcharge
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_AU_C_NOR_DisplaySurcharge(int nReceivedSurcharge)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"ShowSurcharge");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_DisplaySurcharge]\n"));

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// [#2362] AU KSK 2015.07.20
	if (nReceivedSurcharge != 0)
		m_nDisplayedSurchargeAmount = nReceivedSurcharge;
	else
		m_nDisplayedSurchargeAmount = SurchargeAmount;
	// end of [#2362]

	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEDISPLAY, 1);

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2071] AU KSK 2011.06.17 뉴질랜드인 경우 화면 번호 별도 사용 (문구 변경 - Paymark 인증 불구합 대응)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
				m_pDevCmn->fnSCR_DisplayPrevSet(109);
			else
				m_pDevCmn->fnSCR_DisplayPrevSet(110);
			// end of [#2071]

			// Setting Screen
			{
				CString strTemp;

				// History
				for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
				{
					if (nHistory == 0)
						m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

					m_pDevCmn->fnSCR_DisplayHistory(nHistory+1, m_arHistory[nHistory]);
				}

				// Title
				// [#2071] AU KSK 2011.06.17 뉴질랜드인 경우 화면 번호 별도 사용 (문구 변경 - Paymark 인증 불구합 대응)
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109001));
				else
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_110001));
				// end of [#2071]

				// Sub Title
				// [#2071] AU KSK 2011.06.17 뉴질랜드인 경우 화면 번호 별도 사용 (문구 변경 - Paymark 인증 불구합 대응)
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109002));
				else
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_110002));
				// end of [#2071]

				// [#2071] AU KSK 2011.06.17 뉴질랜드인 경우 화면 번호 별도 사용 (문구 변경 - Paymark 인증 불구합 대응)
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109101)); // If you continue, you will be charged
				else
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_110101));
				// end of [#2071]

//				strTemp.Format(_T("%s %s"), GetCurrencySymbol(), MakeMoneyCent(Int2Asc(SurchargeAmount)));
				strTemp.Format(_T("%s %s"), GetCurrencySymbol(), MakeMoneyCent(Int2Asc(m_nDisplayedSurchargeAmount)));	// [#2362] AU KSK 2015.07.20

				m_pDevCmn->fnSCR_DisplayString(5, strTemp);

				// [#2071] AU KSK 2011.06.17 뉴질랜드인 경우 화면 번호 별도 사용 (문구 변경 - Paymark 인증 불구합 대응)
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
					m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109102));
				else
					m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_110102));
				// end of [#2071]

				// [#2071] AU KSK 2011.06.17 뉴질랜드인 경우 화면 번호 별도 사용 (문구 변경 - Paymark 인증 불구합 대응)
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
				{
					// [#2362] AU KSK 2015.08.03 DC Payments 요청에 의해 우측 Space Trim 하도록 변경 (NH1800CE와 통일)
					strTemp.Format(L"%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER));
					strTemp.TrimRight();
					m_pDevCmn->fnSCR_DisplayString(7, strTemp);
					// end of [#2362]

					//contact Info 9,10, 11
					strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT1);
					strTemp.TrimRight();
					m_pDevCmn->fnSCR_DisplayString(8, strTemp);
					strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT2);
					strTemp.TrimRight();
					m_pDevCmn->fnSCR_DisplayString(9, strTemp);
					strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT3);
					strTemp.TrimRight();
					m_pDevCmn->fnSCR_DisplayString(10, strTemp);

					m_pDevCmn->fnSCR_DisplayString(11, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109108));
				}
				// end of [#2071]
			}

			// [#2071] AU KSK 2011.06.17 뉴질랜드인 경우 화면 번호 별도 사용 (문구 변경 - Paymark 인증 불구합 대응)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
				m_pDevCmn->fnSCR_DisplayScreen(109, KEYIN_TIME_OUT, PIN_MENU_MODE);
			else
				m_pDevCmn->fnSCR_DisplayScreen(110, KEYIN_TIME_OUT, PIN_MENU_MODE);
			// end of [#2071]

//			m_nDisplayedSurchargeAmount = SurchargeAmount;		// [#2150] US Justin 2012.10.01 Add Displayed Surcharge Amount	(for future use...) [#2362] AU KSK 2015.07.20

			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
		}

#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			return RES_OK;
		}
#else		
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT || GetKeyStr == S_NO)
			{
				CString strReason, strTemp2;
				strTemp2.Format(L"%d", SurchargeAmount);
				strReason.Format(L"USER CANCEL, FEE=%s%s", GetCurrencySymbol(), MakeMoneyCent(strTemp2));
				m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_CONFIRM_FEE, strReason);
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
//[#2000] SOOK 2010.10.16 호주향 개발 (거래 FLOW)
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
				if ( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME) == 0
					||  P_NH_NOR_NeedMoreTime() != RES_YES )
				{
					CString strReason, strTemp2;
					strTemp2.Format(L"%d", SurchargeAmount);
					strReason.Format(L"TIMEOUT, FEE=%s%s", GetCurrencySymbol(), MakeMoneyCent(strTemp2));
					m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_CONFIRM_FEE, strReason);
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					return RES_USER_TIMEOUT;
				}
				bShowScreen = TRUE;
#else
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				return RES_USER_TIMEOUT;
#endif //end of [#2000]
			}
			else if (GetKeyStr == S_YES)
			{
				return RES_OK;
			}
//			else if (GetKeyStr == S_NO)
//			{
//				return RES_USER_EXIT;
//			}
		}
#endif

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_AU_C_NOR_DisplayExit()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_AU_C_NOR_DisplayExit()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"DisplayExit");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_AU_C_NOR_DisplayExit]\n"));

	// [#2493] AU Kook 2017.07.25, Bug Fix - Last Transaction Time is not changing if there are no Exit Messages.
	CString strDate;
	strDate.Format(L"%2s%2s%4s", GetDate().Mid(4,2), GetDate().Right(2), GetDate().Left(4));
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_LOCALTRANDATE, strDate);
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_LOCALTRANTIME, GetTime());
	// end of [#2493]

	///////////////////////////////////
	// DISPLAY SCREEN

	CString strExitMsg1 = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EXIT_MESSAGE1);
	CString strExitMsg2 = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EXIT_MESSAGE2);
	CString strExitMsg3 = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EXIT_MESSAGE3);

	strExitMsg1.TrimRight();
	strExitMsg2.TrimRight();
	strExitMsg3.TrimRight();

	if ( strExitMsg1 == L"" && strExitMsg2 == L"" && strExitMsg3 == L"")
		return RES_OK;

	m_pDevCmn->fnSCR_DisplayPrevSet(123);

	// Setting Screen
	{
		// [#2220] NH KMK 2014.01.27 APP_SHOW_MODE 관련 로직 미사용하므로 주석처리
// #ifdef APP_SHOW_MODE
// 		m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_123001));
// #else
		// end of [#2220]

		// Exit
		CString strExitMsg = L""; //[#599] SOOK 2009.12.15
		CString tmp;
		for ( int i = 0; i < 3; i++)   	
		{       
			tmp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EXIT_MESSAGE1+i);
			tmp.TrimRight();//2010.02.16 공백 제거하고 표시하도록 수정함 

			strExitMsg += tmp;
			if ( i != 2 ) strExitMsg += L"\n";
		}

		m_pDevCmn->fnSCR_DisplayString(4, strExitMsg);
// #endif
	}

	m_pDevCmn->fnSCR_DisplayScreen(123);

	///////////////////////////////////
	// WAIT FOR CUSTOMER
	//m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
	//m_pDevCmn->fstrSCR_WaitTime(1);		// 너무 길어 2초로
	m_pDevCmn->fstrSCR_WaitMiliTime(500);


	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_AU_C_NOR_ErrorReceipt()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Receive Receopt
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_AU_C_NOR_ErrorReceipt()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"ErrorReceipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_AU_C_NOR_ErrorReceipt]\n"));

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(107);

			// Setting Screen
			{
				// History
				for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
				{
					if (nHistory == 0)
						m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

					m_pDevCmn->fnSCR_DisplayHistory(nHistory+1, m_arHistory[nHistory]);
				}

				// [#2220] AU KMK 2014.02.27
				// 호주 거래 FLOW에서는 Balance Enquiry > (명세표 출력불가안내) > No Receipt 선택한 경우에 이 화면으로 옴
				// Balanced details ... 문구를 출력해야 함			
				m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_RECEIPT_ERR);

				if (m_sUserSelection.nPrintReceipt == FALSE)	// No Receipt 선택 시
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_107003));	// Title(No Receipt)
				else	// 그 외의 경우는 SPR 장애임
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_107001));	// Title(Receipt Not Available)

				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_107004));		// Balance details displayed on screen ...
				// end of [#2220]
			}

			m_pDevCmn->fnSCR_DisplayScreen(107, KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
		}

#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			m_sUserSelection.nPrintReceipt = FALSE;
			return RES_OK;
		}
#else
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT || GetKeyStr == S_NO)
			{
				if (TranCode == TC_INQUIRY)
					m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_RECEIPT_SCREEN, L"USER CANCEL");
				else
					m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_RECEIPT_ERROR, L"USER CANCEL");
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
//[#2000] SOOK 2010.10.16 호주향 개발 (거래 FLOW)
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
				if ( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME) == 0
					||  P_NH_NOR_NeedMoreTime() != RES_YES )
				{
					if (TranCode == TC_INQUIRY)
						m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_RECEIPT_SCREEN, L"TIMEOUT");
					else
						m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_RECEIPT_ERROR, L"TIMEOUT");
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					return RES_USER_TIMEOUT;
				}
				bShowScreen = TRUE;
#else
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				return RES_USER_TIMEOUT;
#endif //end of [#2000]			
			}
			else if (GetKeyStr == S_YES)
			{
				m_sUserSelection.nPrintReceipt = FALSE;			// [#2219] 2013.09.04 Justin Digital Receipt, Change Variable Name
				if (TranCode == TC_INQUIRY)
					m_sUserSelection.bScreenDisplay = TRUE;
				return RES_OK;
			}
//			else if (GetKeyStr == S_NO)
//			{
//				return RES_USER_EXIT;
//			}
			// [#2219] 2013.09.04 JUSTIN Digital Receipt
			else if (GetKeyStr == S_QRCODE)
			{
				m_sUserSelection.nPrintReceipt = RCPT_QRCODE;
				return RES_OK;
			}
			// End of [#2219] 2013.09.04
		}
#endif

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_AU_C_NOR_CashDispense()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Cash Dispense
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_AU_C_NOR_CashDispense()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"CashDispense");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_AU_C_NOR_CashDispense]\n"));

	CString strMoney;
	DWORD			dwTickStart = 0, dwTickEnd = 0;

	///////////////////////////////////
	// DISPLAY SCREEN

	m_pDevCmn->fnSCR_DisplayPrevSet(117);

	// Setting Screen
	{
		// Sub Title
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_117001));
	}

	m_pDevCmn->fnSCR_DisplayScreen(117);
	dwTickStart = GetTickCount();

	///////////////////////////////////
	// DISPENSE MONEY

	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 1);	// [#2058] NH KSK 2011.05.11 Dispense 전에 Reason For Reversal값 설정 추가

	// CDU FLICKER ON
	m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_CDU, FLICKER_ON);

	// Dispense, CENT 제거.
	m_pDevCmn->fnCDU_Dispense(Asc2Int(m_sUserSelection.strMoney.Left(10)));

	// Wait
	m_pDevCmn->fnAPL_CheckDeviceAction(DEV_CDU);
	m_pDevCmn->fnAPL_CheckDevice();			// [#7] KSK 2008. 03.14

	// TOTAL AMOUNT OF LAST DISPENSED NOTES
	strMoney = m_pDevCmn->fstrCDU_GetLastDispensedAmount();
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT, strMoney);	// [#169] [NH] KSK 2008.04.25

	int	i = 0;
	int	nDispensed[4] = { 0, }, nRejected[4] = { 0, };
	int nSkewNote[4] = { 0, }, nGapNote[4] = { 0, }, nLongNote[4] = { 0, }, nShortNote[4] = { 0, }, nDoubleNote[4] = { 0, };
	CString strErrorCode, strtemp;

	// Dispense Status
	{		
		strErrorCode = m_pDevCmn->fstrAPL_GetErrorCode();

		if ((strErrorCode == L"9792401") ||			// FATAL ERROR
			(strErrorCode.Left(5) == L"97400"))		// CDU 단선
		{
			// Dispense Command가 SP로 가지 않는 경우 Dispensed / Rejected Count를 임의로 0으로 만든다.
			strtemp.Format(L"00 00 00 00");
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_DISP_COUNT, strtemp);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_REJ_COUNT,	strtemp);				
		}
		else
		{
			// Get Last Dispense Info
			for (i = 0; i < 4; i++)
			{
				nDispensed[i]  = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_DISP_CNT + i);
				nRejected[i]   = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_REJ_CNT + i);
				nSkewNote[i]   = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTSKEW_CST1 + i);
				nGapNote[i]	   = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTGAPTOCLOSE_CST1 + i);
				nLongNote[i]   = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTLONGNOTE_CST1 + i);
				nShortNote[i]  = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTSHORTNOTE_CST1 + i);
				nDoubleNote[i] = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTDOUBLE_CST1 + i);

				// [#2270] AU KSK 2014.05.26 Note Counting Disable시에는 Dispense / Reject 후 Remain Count를 AP쪽에 관리하는 매수를 차감한다.
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 1)
				{
					int nTempCount = 0;
					// Dispensed Count 먼저 차감
					nTempCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP + i) - nDispensed[i];
					MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP + i, nTempCount);

					// Reject Count도 추가로 차감
					nTempCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP + i) - nRejected[i];
					MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP + i, nTempCount);
				}
				// end of [#2270]

				NHDEBUG(1, (_T("CDiagCdu::ThreadHandlerProc : CMD_DISPENSE CALL CST [%d] DISPENSED[%d] REJECTED[%d]\n"), i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_DISP_CNT + i), MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_REJ_CNT + i)));
			}

			// Add to Reject Total
			for (i = 0; i < 4; i++)
			{
				AddSumOfDispensed(i+1, nDispensed[i]);
				AddSumOfRejected(i+1, nRejected[i]);
				AddSumOfDetectedSkew(i+1, nSkewNote[i]);
				AddSumOfDetectedGap(i+1, nGapNote[i]);
				AddSumOfDetectedLong(i+1, nLongNote[i]);
				AddSumOfDetectedShort(i+1, nShortNote[i]);
				AddSumOfDetectedDouble(i+1, nDoubleNote[i]);
			}

			// save Last dispensed Info
			strtemp.Format(L"%02d %02d %02d %02d",	nDispensed[0], nDispensed[1], nDispensed[2], nDispensed[3]);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_DISP_COUNT, strtemp);

			// save Last rejected Info
			strtemp.Format(L"%02d %02d %02d %02d",  nRejected[0], nRejected[1], nRejected[2], nRejected[3]);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_REJ_COUNT,	strtemp);
		}
	}

	// Check Error
	{
		int nRequestedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
		int nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

		// KSK 2010.04.30
		if (nDispensedAmount >= nRequestedAmount)
		{
			// 과방출 처리는 위에서 처리함.
			return RES_OK;
		}
		else
		{
//[#2000] SOOK 2010.10.18
			if ((nDispensedAmount < nRequestedAmount) && m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_CDU))	// 요구매수보다 방출매수가 작고 CDU 장애인 경우
			{
				// 1. Partial ErrorCode Check
				if ((m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "40047")	||	// CST1 - EP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "4005B")	||	// CST2 - EP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "4009F")	||	// CST3 - EP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "4007C")	||	// CST4 - EP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "97447")	||	// CST1 - SP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "9745B")	||	// CST2 - SP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "9749F")	||	// CST3 - SP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "9747C")	||	// CST4 - SP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "40041")	||	// REJECT 과다 장애1
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "40043")	||	// REJECT 과다 장애2
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "40044")	||	// REJECT 과다 장애3 (Bug Fix) KSK 2010.01.04
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "4004B")	||	// REJECT 과다 장애4
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "4005D"))		// REJECT 과다 장애5
				{
					int nRetryRequestAmount = (nRequestedAmount - nDispensedAmount)/100;

					// Check Dispensable
					if (m_pDevCmn->fbCDU_IsDispensible(nRetryRequestAmount))
					{
						// 방출이 가능함
						// misfeed일 경우에는 방출이 가능한 경우 errorcode를 clear후에 retry를 시도해야만 한다.
						m_pDevCmn->fnAPL_ClearError();
						m_pDevCmn->DeviceStatus &= ~DEV_CDU;			// Device Status에서 CDU 장애를 Clear

						m_pDevCmn->fnCDU_Dispense(nRetryRequestAmount);
						m_pDevCmn->fnAPL_CheckDeviceAction(DEV_CDU);
						m_pDevCmn->fnAPL_CheckDevice();			// [#7] KSK 2008. 03.14

						strMoney = m_pDevCmn->fstrCDU_GetLastDispensedAmount();


						int nFinalDispenseResult = Asc2Int(strMoney) + Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));
				
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT, Int2Asc(nFinalDispenseResult));

						// 9792401, 97400은 CDU에 Dispense Command가 전달되지 않았기 때문에 Skip함
						if (!(m_pDevCmn->fstrAPL_GetErrorCode() == L"9792401" || m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"97400"))
						{
							// Get Last Dispense Info
							for (i = 0; i < 4; i++)
							{
								nDispensed[i]  += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_DISP_CNT + i);
								nRejected[i]   += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_REJ_CNT + i);
								nSkewNote[i]   += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTSKEW_CST1 + i);
								nGapNote[i]	   += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTGAPTOCLOSE_CST1 + i);
								nLongNote[i]   += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTLONGNOTE_CST1 + i);
								nShortNote[i]  += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTSHORTNOTE_CST1 + i);
								nDoubleNote[i] += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTDOUBLE_CST1 + i);

								// [#2270] AU KSK 2014.05.26 Note Counting Disable시에는 Dispense / Reject 후 Remain Count를 AP쪽에 관리하는 매수를 차감한다.
								if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 1)
								{
									int nTempCount = 0;
									// Dispensed Count 먼저 차감
									nTempCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP + i) - MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_DISP_CNT + i);
									MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP + i, nTempCount);

									// Reject Count도 추가로 차감
									nTempCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP + i) - MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_REJ_CNT + i);
									MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP + i, nTempCount);
								}
								// end of [#2270]

								// Add to Reject Total
								AddSumOfDispensed(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_DISP_CNT + i));
								AddSumOfRejected(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_REJ_CNT + i));
								AddSumOfDetectedSkew(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTSKEW_CST1 + i));
								AddSumOfDetectedGap(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTGAPTOCLOSE_CST1 + i));
								AddSumOfDetectedLong(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTLONGNOTE_CST1 + i));
								AddSumOfDetectedShort(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTSHORTNOTE_CST1 + i));
								AddSumOfDetectedDouble(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTDOUBLE_CST1 + i));
							}
						}
						
						// save Last dispensed Info
						strtemp.Format(L"%02d %02d %02d %02d",	nDispensed[0], nDispensed[1], nDispensed[2], nDispensed[3]);
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_DISP_COUNT, strtemp);
						
						// save Last rejected Info
						strtemp.Format(L"%02d %02d %02d %02d",  nRejected[0], nRejected[1], nRejected[2], nRejected[3]);
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_REJ_COUNT,	strtemp);
					}
				}
			}

			nRequestedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
			nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

			// 추가 방출 수행 후 장상 방출 되었는지 다시 확인함 
			if (nDispensedAmount >= nRequestedAmount)
			{
				// 과방출 처리는 위에서 처리함.
				return RES_OK;
			}
//end of [#2000]

			// Partial 처리
			NHDEBUG(DBG_INFO, (_T("SHOW ERROR SCREEN\n")));

			// CDU FLICKER OFF
			m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_CDU, FLICKER_OFF);

			dwTickEnd = GetTickCount();

			if ((dwTickEnd - dwTickStart) < 2000)
			{
				NHDEBUG(DBG_INFO, (_T("WAIT 2SEC...\n")));
				m_pDevCmn->fstrSCR_WaitTime(2);
			}

			CString strTemp;

			///////////////////////////////////
			// DISPLAY SCREEN

			m_pDevCmn->fnSCR_DisplayPrevSet(122);
			m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
			strTemp.Format(L"%s\n", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122201));
			m_pDevCmn->fnSCR_DisplayString(4, strTemp);
			m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
			m_pDevCmn->fnSCR_DisplayScreen(122);

			m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);

			if (nDispensedAmount > 0)	// partial 처리
			{
				// CDU FLICKER ON
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_CDU, FLICKER_ON);
				return RES_DEV_CDU_ERR_PARTIAL;
			}

			return RES_DEV_CDU_ERR;		// full 처리
		}
	}

	return RES_OK;
}


/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_AU_C_NOR_InputCWAmount()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 출금 금액을 사용자로 부터 입력 받는다.
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_AU_C_NOR_InputCWAmount(BOOL bUseFastCash)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_AU_C_NOR_InputCWAmount]\n"));

	int		i, nIndex;
	int		nInputAmount = 0;
	BOOL	bOtherAmountRoof = FALSE;
	CString	strDispFastCash;
	CString	GetKeyStr;
	CString strMultipleMsg;
	CString	strTemp;
	BOOL	bShowScreen;

	// m_nFastCaseNo
	// 0 : Fast Cash Available
	// 1 : Fast Cash Unavailable, Input Other Amount
	// -1 : Unable to dispense
	m_pDevCmn->fnCDU_GetCashDispenseInfo();	// [#485] [NH] KSK 2009.1.14

	if (m_pDevCmn->m_FastCaseNo == -1 || m_pDevCmn->fnAPL_GetAvailTrans() == TRAN_WITH_NOT)
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(122);
		m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
		m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113111));
		m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
		m_pDevCmn->fnSCR_DisplayScreen(122);

		m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
		m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_FASTCASH, L"UNABLE TO DISPENSE CASH (0)");

		return RES_CASH_NOT_AVAILABLE;
	}

	///////////////////////////////////
	//	FAST CASH
	///////////////////////////////////
	if ((m_pDevCmn->m_FastCaseNo == 0) && (bUseFastCash == TRUE))		// Fast Cash
	{
		NVDump('O', 'C', "00", L"P_NHNOR", L"FastCash");

		bShowScreen = TRUE;

		g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
		while (g_TimeCheck.IsElapsedTimes() == FALSE)
		{
			///////////////////////////////////
			// CHECK STATUS

			///////////////////////////////////
			// DISPLAY SCREEN
			if (bShowScreen == TRUE)
			{
				NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

				m_pDevCmn->fnSCR_DisplayPrevSet(112);

				// Setting Screen
				{
					// History
					for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
					{
						if (nHistory == 0)
							m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

						m_pDevCmn->fnSCR_DisplayHistory(nHistory+1, m_arHistory[nHistory]);
					}

					// Title
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_112001));

					// Sub Title
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_112002));

					nIndex = 4;

					for (i=0; i<CDU_CST_MAXFASTCASH; i++)
					{
						if (m_pDevCmn->m_nFastCash[i])
						{
							strDispFastCash.Format(L"   %s %d   ", GetCurrencySymbol(), m_pDevCmn->m_nFastCash[i]);
							m_pDevCmn->fnSCR_DisplayString(nIndex++, strDispFastCash);
						}
					}
				}

				m_pDevCmn->fnSCR_DisplayScreen(112, KEYIN_TIME_OUT, PIN_MENU_MODE);
				bShowScreen = FALSE;

#ifdef APP_AGING_MODE
				g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
			}

#ifdef APP_AGING_MODE
			if (g_AgingCheck.IsElapsedTimes() == TRUE)
			{
				// CENT 포함 12자리
				m_sUserSelection.strMoney = L"000000002000";

				return RES_OK;
			}
#else
			///////////////////////////////////
			// GET KEY STRING
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_FASTCASH, L"USER CANCEL");
					return RES_USER_EXIT;
				}
				else if (GetKeyStr == S_TIMEOVER)
				{
//[#2000] SOOK 2010.10.16 호주향 개발 (거래 FLOW)
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
					if ( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME) == 0
						||  P_NH_NOR_NeedMoreTime() != RES_YES )
					{
						m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_FASTCASH, L"TIMEOUT");
						NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
						return RES_USER_TIMEOUT;
					}
					bShowScreen = TRUE;
#else
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					return RES_USER_TIMEOUT;
#endif //end of [#2000]				
				}
				else if (GetKeyStr == S_OTHER)
				{
					bOtherAmountRoof = TRUE;
					break;
				}
				else if (GetKeyStr.GetLength() > 0)
				{
					m_sUserSelection.strMoney = L"";

					// remove... symbol, white space
					GetKeyStr.Replace(GetCurrencySymbol(), L"");
					GetKeyStr.TrimLeft();
					GetKeyStr.TrimRight();

					nInputAmount = Asc2Int(GetKeyStr);

					NHDEBUG(DBG_INFO, (_T("REMOVE SYMBOL CHAR - VALUE [%s] - [%d]\n"), GetKeyStr, nInputAmount));

					if (!IsNum(GetKeyStr) || IsZero(GetKeyStr))
					{
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113110));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
						m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_FASTCASH, L"UNABLE TO DISPENSE CASH (1)");
#endif

						m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
						return RES_PROGRAM_ERROR;
					}
					else if (nInputAmount > m_pDevCmn->m_MoneyMaxOut)
					{
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113108));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
						m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_FASTCASH, L"UNABLE TO DISPENSE CASH (3)");
#endif

						m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
						return RES_CASH_MAX_OVER;
					}
					else if (nInputAmount < m_pDevCmn->m_MoneyMinOut)
					{
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113109));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
						m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_FASTCASH, L"UNABLE TO DISPENSE CASH (4)");
#endif

						m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
						return RES_CASH_MIN_OVER;
					}
					
					else if (!m_pDevCmn->fbCDU_IsDispensible(nInputAmount))
					{
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113111));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
						m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_FASTCASH, L"UNABLE TO DISPENSE CASH (2)");
#endif

						m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
						return RES_CASH_NOT_AVAILABLE;
					}
					// CENT 포함 12자리
					m_sUserSelection.strMoney.Format(L"%010d00", nInputAmount);

#if (AU_VERSION)	// [#2041] AU KSK 2011.03.31
					CString strTempAmountString;
					strTempAmountString.Format(L"Amount : $ %d", nInputAmount);
					m_arHistory.Add(strTempAmountString);
#endif

					return RES_OK;
				}
			}
#endif

			Delay_Msg(50);
		}

		if ((g_TimeCheck.IsElapsedTimes() == TRUE) && (bOtherAmountRoof == FALSE))
		{
			NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
			return RES_USER_TIMEOUT;
		}
	}

	///////////////////////////////////
	//	OTHER AMOUNT
	///////////////////////////////////
	{
		NVDump('O', 'C', "00", L"P_NHNOR", L"CWOtherAmount");

		//int	nRetryCount = 3;

		if (bUseFastCash == FALSE)
			bOtherAmountRoof = TRUE;

		bShowScreen = TRUE;
		g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
		while ((g_TimeCheck.IsElapsedTimes() == FALSE) &&  (bOtherAmountRoof == TRUE))
		{
			///////////////////////////////////
			// CHECK STATUS
		
			m_pDevCmn->fnCDU_GetCashDispenseInfo();
			if (m_pDevCmn->m_FastCaseNo == -1 || m_pDevCmn->fnAPL_GetAvailTrans() == TRAN_WITH_NOT)
			{
				NHDEBUG(DBG_INFO, (_T("FastCashNo(-1) or TRAN_WITH_NOT DETECTED\n")));
				m_pDevCmn->fnSCR_DisplayPrevSet(122);
				m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
				m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113111));
				m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
				m_pDevCmn->fnSCR_DisplayScreen(122);
#if ( AU_VERSION)	// [#2069] NH KSK 2011.06.13
				m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_OTHER_AMOUNT, L"UNABLE TO DISPENSE CASH");
#endif

				m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
				return RES_CASH_NOT_AVAILABLE;
			}

			///////////////////////////////////
			// DISPLAY SCREEN
			if (bShowScreen == TRUE)
			{
				NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

				CString			strtmp_Amount = _T("");
				CString			strdisp_Amount = _T("");
				CStringArray	strtmp_ArrayAmount;
				BOOL			bUseComma = FALSE;
				int				nCashDenomination[8] = {CASH_DENOMINATION1, CASH_DENOMINATION2, CASH_DENOMINATION3, CASH_DENOMINATION4, 
														CASH_DENOMINATION5, CASH_DENOMINATION6, CASH_DENOMINATION7, CASH_DENOMINATION8};

				m_pDevCmn->fnSCR_DisplayPrevSet(113);

				// Setting Screen
				{
					// History
					for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
					{
						if (nHistory == 0)
							m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

						m_pDevCmn->fnSCR_DisplayHistory(nHistory+1, m_arHistory[nHistory]);
					}

					// Title
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113001));

					// Sub Title
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113101));

					// Currency Symbol
					m_pDevCmn->fnSCR_DisplayString(4, L"DOLLAR");
					m_pDevCmn->fnSCR_DisplayString(5, L"INT_DOLLAR");

					// Multiple
					m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113102));

					strMultipleMsg.Empty();
					strtmp_ArrayAmount.RemoveAll();

					for(int i=0; i<8; i++)
					{
						if (m_pDevCmn->WithAvail & (DENOMINATION_BIT1<<i))
						{
							strdisp_Amount.Format(L"%s %d ", GetCurrencySymbol(), nCashDenomination[i]);	// [#285] [MX] KSK 2008.6.12
							strtmp_Amount.Format(L"%d", nCashDenomination[i]);

							if (bUseComma == FALSE)
								strMultipleMsg += strdisp_Amount;
							else
								strMultipleMsg += L"," + strdisp_Amount;

							strtmp_ArrayAmount.Add(strtmp_Amount);

							if (bUseComma == FALSE)	bUseComma = TRUE;
						}
					}

					m_pDevCmn->fnSCR_DisplayString(7, strMultipleMsg);

					// max
					m_pDevCmn->fnSCR_DisplayString(8, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113103));

					//strTemp.Format(L"%s %d", GetCurrencySymbol(), m_pDevCmn->m_MoneyMaxOut);
					int nOwnerMaxAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT)) / 100;
					strTemp.Format(L"%s %d", GetCurrencySymbol(), nOwnerMaxAmt);


					m_pDevCmn->fnSCR_DisplayString(9, strTemp);

					m_pDevCmn->fnSCR_DisplayString(10, L"1");	// min
					m_pDevCmn->fnSCR_DisplayString(11, L"4");	// max
		
					// auto run
					m_pDevCmn->fnSCR_DisplayString(12, L"off");

				}

				m_pDevCmn->fnSCR_DisplayScreen(113, KEYIN_TIME_OUT, PIN_MENU_MODE);
				bShowScreen = FALSE;

#ifdef APP_AGING_MODE
				g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
			}

#ifdef APP_AGING_MODE
			if (g_AgingCheck.IsElapsedTimes() == TRUE)
			{
				// CENT 포함 12자리
				m_sUserSelection.strMoney = L"000000002000";

				return RES_OK;
			}
#else
			///////////////////////////////////
			// GET KEY STRING
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
					m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_OTHER_AMOUNT, L"USER CANCEL");
#endif
					return RES_USER_EXIT;
				}
				else if (GetKeyStr == S_TIMEOVER)
				{
//[#2000] SOOK 2010.10.16 호주향 개발 (거래 FLOW)
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
					if ( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME) == 0
						||  P_NH_NOR_NeedMoreTime() != RES_YES )
					{
						m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_OTHER_AMOUNT, L"TIMEOUT");
						NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
						return RES_USER_TIMEOUT;
					}
					bShowScreen = TRUE;
#else
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					return RES_USER_TIMEOUT;
#endif //end of [#2000]				
				}
				else if (GetKeyStr.GetLength() > 0)
				{
					nInputAmount = Asc2Int(GetKeyStr);

					NHDEBUG(DBG_INFO, (_T("Input Amount to convert integer [%d]\n"), nInputAmount));

					if (nInputAmount > m_pDevCmn->m_MoneyMaxOut)
					{
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_INFO);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113113));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);

						m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);
						bShowScreen = TRUE;
					}
					else if (nInputAmount < m_pDevCmn->m_MoneyMinOut)
					{
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_INFO);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113113));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);

						m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);
						bShowScreen = TRUE;
					}
					else if (m_pDevCmn->fbCDU_IsDispensible(nInputAmount))
					{
						// CENT 포함 12자리
						m_sUserSelection.strMoney.Format(L"%010d00", nInputAmount);
#if (AU_VERSION)	// [#2041] AU KSK 2011.03.31
					CString strTempAmountString;
					strTempAmountString.Format(L"Amount : $%d", nInputAmount);
					m_arHistory.Add(strTempAmountString);
#endif

						return RES_OK;
					}
					else
					{
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_INFO);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113113));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);

						m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);
						bShowScreen = TRUE;
					}
				}
			}
#endif
			
			Delay_Msg(50);
		}
	}

	NHDEBUG(DBG_INFO, (_T("ABNORMAL PROCESSING\n")));
	return RES_USER_EXIT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_AU_C_NOR_EnterPassword()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_AU_C_NOR_EnterPassword()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"EnterPIN");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_AU_C_NOR_EnterPassword]\n"));

	CString			GetKeyStr;
	DWORD			dwTickStart = 0, dwTickEnd = 0;
	BOOL			bShowScreen = TRUE;
	NH_SCR_CONFIG	eFrontConfig = m_pDevCmn->m_pConfig->GetScreenConfig(SCR_FRONT);
	BOOL			bFlashScreen = (eFrontConfig.eEngine == SCR_ENG_FLASH_LITE) ? (TRUE) : (FALSE);

	m_sUserSelection.strPassword = L"";	// [#11] NH KSK 2010.10.5

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(103);

			// Setting Screen
			{
				// History
				for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
				{
					if (nHistory == 0)
						m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

					m_pDevCmn->fnSCR_DisplayHistory(nHistory+1, m_arHistory[nHistory]);
				}

				// Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103001));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103002));

				// Guide
				m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103003));

				// Attack Guide
				m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103006));
			}

			dwTickStart = GetTickCount();

			// [#2019] AU KSK 2011.02.10 512K에서 SP에서 PinBlock이 없어지는 Bug Fix 적용으로 인해 시점 공통화 (적용 SP : V06.04.04)
			// Host 송신 시 Pin Block을 호출하도록 수정 KSK 2010.07.28
			NHDEBUG(DBG_INFO, (L"CARD ACCOUNT NO : [%s]\n", m_sCardData.strAccountNo));
//			m_pDevCmn->fnSCR_DisplayScreen(103, KEYIN_TIME_OUT, PIN_PASSWORD_MODE, m_sCardData.strAccountNo, PIN_PASSWORD_MIN, PIN_PASSWORD_MAX, PIN_PASSWORD_AUTO_TRUE, PIN_PASSWORD_TERM);
			m_pDevCmn->fnSCR_DisplayScreen(103, KEYIN_TIME_OUT, PIN_PASSWORD_MODE, L"", PIN_PASSWORD_MIN, PIN_PASSWORD_MAX, PIN_PASSWORD_AUTO_TRUE, PIN_PASSWORD_TERM);
			// end of [#2019]

			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
		}

#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			if (bFlashScreen == TRUE)
			{
				if ((dwTickEnd - dwTickStart) < 2000)
					m_pDevCmn->fstrSCR_WaitTime(2);
			}

			m_sUserSelection.strPassword = L"DCA71B5293B1230F";
			return RES_OK;
		}
#else
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			dwTickEnd = GetTickCount();
			
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
#if ( AU_VERSION)// [#2000]2010.10.17 SOOK 호주 사양 적용 (거래 FLOW )	// [#2069] NH KSK 2011.06.13
				m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_INPUT_PIN, L"USER CANCEL");
#endif
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				//[#2000] SOOK 2010.10.16 호주향 개발 (거래 FLOW)
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
				if ( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME) == 0
					||  P_NH_NOR_NeedMoreTime() != RES_YES )
				{
					m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_INPUT_PIN, L"TIMEOUT");
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					return RES_USER_TIMEOUT;
				}
				bShowScreen = TRUE;
#else
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					return RES_USER_TIMEOUT;
#endif
				//end of [#2000]
			}
			else if (GetKeyStr == L"LOAD_OK")
			{
				NHDEBUG(DBG_INFO, (_T("PASSWORD PIN ENABLE..\n")));

				// BuildPinBlock은 Host 송/수신시에 Make하도록 로직 수정 KSK 2010.07.28
				m_pDevCmn->fnSCR_DisplayScreen(0, 0, PIN_PASSWORD_MODE, L"", PIN_PASSWORD_MIN, PIN_PASSWORD_MAX, PIN_PASSWORD_AUTO_TRUE, PIN_PASSWORD_TERM);	
			}
			else if (GetKeyStr == L"PIN")
			{
				// Flash Screen일 경우 2초 Delay 검증 필요
				if (bFlashScreen == TRUE)
				{
					if ((dwTickEnd - dwTickStart) < 2000)
						m_pDevCmn->fstrSCR_WaitTime(2);
				}
				
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);	// Read Complete가 올때까지 대기
				// 송/수신 후에 Build Pin Block을 하도록 위치 수정
//				m_sUserSelection.strPassword = m_pDevCmn->fstrPIN_GetPinKeyData();	// 송, 수신 후 위치로 이동 (주의사항 : PIN SP를 최종으로 update 해야함)
//				NHDEBUG(DBG_INFO, (_T("PINPAD RETURN VALUE [%s]\n"), m_sUserSelection.strPassword));

				return RES_OK;
			}
			//[#2000] SOOK 2010.10.16 호주향 개발 (거래 FLOW)
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
			// User input PIN with less than 4 digit
			else if (GetKeyStr == L"PINRETRY")
			{
				m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103004));
				m_pDevCmn->fnSCR_DisplayUpdate(103);
			}
			// First User Input
			else if (GetKeyStr == L"FIRSTUSERINPUT")
			{
				m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103003));
				m_pDevCmn->fnSCR_DisplayUpdate(103);
			}
#endif
			//end of [#2000]
		}
#endif

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}


// [#2494] AU woooz 2017.08.01
#if (APP_ALPHI)
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_NH_ATS_InsertCard()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Insert card depends on card type
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_ATS_InsertCard(int pScreenNumber,bool pLoyaltyCard)
{
	//NVDump('O', 'C', "00", L"P_NH_ATS", L"P_NH_ATS_InsertCard");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ATS_InsertCard()]\n"));

	// Setting Screen 
	SetBackImageOnWelcomeScreenLayout();
	m_pDevCmn->fnSCR_DisplayPrevSet(pScreenNumber);	


	// Value 1, 2
	if(pLoyaltyCard)
	{
		m_pDevCmn->nKindOfMedia = MEDIA_MS;

		m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_401001));	// Insert and remove your loyalty card.
	}
	else
	{
		if(m_pDevCmn->fnMCU_IsEmvEnable())					// revert card type.
			m_pDevCmn->nKindOfMedia = MEDIA_IC;		
		else
			m_pDevCmn->nKindOfMedia = MEDIA_MS;

		// set message for IC and MS 
		// Value 1, 2
		if(m_pDevCmn->fnMCU_IsEmvTransaction())			
		{
#if (US_VERSION || AU_VERSION)	
			m_pDevCmn->fnSCR_DisplayString(5, L"EMV_ON");
#endif

			m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_402004));
			m_pDevCmn->fnSCR_DisplayString(2, L"");

			m_pDevCmn->fnSCR_DisplayImage(2, FALSE);
			m_pDevCmn->fnSCR_DisplayImage(4, FALSE);
			m_pDevCmn->fnSCR_DisplayImage(5, FALSE);
			m_pDevCmn->fnSCR_DisplayImage(7, FALSE);

			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
			{
				m_pDevCmn->fnSCR_DisplayImage(3, FALSE);
				m_pDevCmn->fnSCR_DisplayImage(6, TRUE);
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayImage(3, TRUE);
				m_pDevCmn->fnSCR_DisplayImage(6, FALSE);
			}			
		}
		else
		{
			m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_402001));
			m_pDevCmn->fnSCR_DisplayString(2, L"");

			m_pDevCmn->fnSCR_DisplayImage(3, FALSE);
			m_pDevCmn->fnSCR_DisplayImage(4, FALSE);
			m_pDevCmn->fnSCR_DisplayImage(6, FALSE);
			m_pDevCmn->fnSCR_DisplayImage(7, FALSE);
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
			{
				m_pDevCmn->fnSCR_DisplayImage(2, FALSE);
				m_pDevCmn->fnSCR_DisplayImage(5, TRUE);
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayImage(2, TRUE);
				m_pDevCmn->fnSCR_DisplayImage(5, FALSE);
			}
		}
	}
	// Setting Screen 

	m_pDevCmn->fnSCR_DisplayScreen(pScreenNumber, KEYIN_TIME_OUT, PIN_MENU_MODE);

	m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);
	SetMcuExisted(FALSE);

	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
	{
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_FLICKER_OPTION) == 0)	// ALWAY 인 경우에만 FLICKER ON
			m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_PIN, FLICKER_ON);
	}

#ifdef APP_AGING_MODE
	g_AgingCheck.SetTargetTimeAfterSec(5);
#endif

	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	g_TimeCheck.SetTargetTimeAfterSec(30);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);
			else
				SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);

			return RES_3RDAP_ATMREADCARD_NORMAL;
		}
#else
		// 1. CHECK ADA for next development
		/*
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == TRUE)
		{
		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);
		else
		SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);

		m_pDevCmn->fnAPL_DeviceEnDisable(DEV_MCU, DISABLE, TRUE);
		return RES_3RDAP_ATMREADCARD_ADA;
		}
		*/

		// 2. CHECH MCU EVENT
		// 카드가 없는 상태에서 새로 넣었을 경우만 거래 Flow를 시작하도록 처리
		if ((DidMediaExist() == FALSE) && 
			(LIB_MainMenuDeviceEvent() == T_OK) &&
			(m_pDevCmn->fnMCU_GetDeviceStatus() == NORMAL))
		{
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);
			else
				SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);

			return RES_OK;
		}
		else if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 5, EVENT_IN) == DEV_SCR)
		{
			CString strKeyString = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), strKeyString));

			if (strKeyString == S_CANCEL || strKeyString == S_EXIT || strKeyString == S_TIMEOVER)
			{
				if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
					m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);
				else
					SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);

				m_pDevCmn->fnAPL_DeviceEnDisable(DEV_MCU, DISABLE, TRUE);
			}
			if (strKeyString == S_CANCEL || strKeyString == S_EXIT)
			{
				g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;
				return RES_USER_EXIT;
			}
			else if (strKeyString == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				g_sBizFlowInfo.nReasonforCancel = RES_USER_TIMEOUT;
				return RES_USER_TIMEOUT;
			}
		}
#endif
		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	g_sBizFlowInfo.nReasonforCancel = RES_USER_TIMEOUT;
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_NH_ATS_ReadCard()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ATS_ReadCard()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"ReadCard");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_ReadCard]\n"));

	BIZ_RETURN	nRes;

	CString strCardData;
	CString strPackCardData;

	// Parse Data
	strCardData = m_pDevCmn->fstrMCU_GetCardData();

	if (strCardData.GetLength() > 0)
	{
		// "[Track1],[Track2]" or "[Track2]"
		int nLocComma = strCardData.Find(',');
		if(nLocComma<0)
		{
			// Track2 Only...
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_CARDDATA_TO_BE_USED) == 0)
			{
				MakePack(strCardData, strPackCardData, strCardData.GetLength());
			}
		}
		else
		{
			// Track2 Data.
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_CARDDATA_TO_BE_USED) == 0)
			{
				CString sTrack2Data = strCardData.Mid(nLocComma+1);
				MakePack(sTrack2Data, strPackCardData, sTrack2Data.GetLength());
			}
			
			// Track1 Data.
			else
			{
				CString sTrack1Data = strCardData.Left(nLocComma);
				MakePack(sTrack1Data, strPackCardData, sTrack1Data.GetLength());
			}
		}

		NHDEBUG(DBG_INFO, (L"Track2 : [%s]\n", strPackCardData));

		nRes = RES_OK;
		m_bLoyaltyCardLoaded = TRUE;
		m_strLoyaltyCardTrack2 = strPackCardData; // [#2497] NH woooZ 2017.09.13  ALPHI server
	}

	// Check ISO 2 Track
	if (strPackCardData.GetLength() <= 0)
	{
		NHDEBUG(DBG_INFO, (L"DON'T READ ISO2 TRACK Data\n"));
		nRes = RES_CARD_ERROR;
	}

	if (nRes == RES_OK)
	{
		LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_401002), ABORT_SCR_TIMEOUT);
	}

	if (nRes != RES_OK)
	{
		CString strTemp;

		///////////////////////////////////
		// DISPLAY ERROR SCREEN

		m_pDevCmn->fnSCR_DisplayPrevSet(122);

		// Setting Screen
		{
			switch (nRes)
			{
			case RES_CARD_ERROR:	strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122101);	break;
			default:				strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122101);	break;
			}

			NVDump('O', 'C', "00", L"P_NHNOR", L"NG_1");

			m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
			m_pDevCmn->fnSCR_DisplayString(4, strTemp);
			m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
		}

		m_pDevCmn->fnSCR_DisplayScreen(122);
		m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
	}

	return nRes;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_NH_ATS_Transaction()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_ATS_Transaction(BOOL bShowScreen)
{
	NVDump('O', 'C', "00", L"P_NHATS", L"Transaction");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ATS_Transaction]\n"));

	BIZ_RETURN	nRes;
	CString		strTemp;
	int			nEnableCount = 0;
	int			nRefreshTime = 0;
	DWORD		dwStartTime;

	int			OrgTranStatus = m_pDevCmn->TranStatus;	// usually it might be 'TRAN_TRAN'.

	m_pDevCmn->TranStatus = TRAN_ATS_ALPHI;

	if (bShowScreen == TRUE)
	{
		NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

		///////////////////////////////////
		// DISPLAY SCREEN

		m_pDevCmn->fnSCR_DisplayPrevSet(150);

		// Setting Screen
		{
			// History
			for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
			{
				if (nHistory == 0)
					m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

				m_pDevCmn->fnSCR_DisplayHistory(nHistory+1, m_arHistory[nHistory]);
			}

			strTemp.Format(L"%s\n", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_150001));

			switch(m_TranCodeAlphi)
			{
			case TC_ATS_ENQUIRY:			strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_150003);	break;
			case TC_ATS_REDEMPTION:			strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_150004);	break;
			case TC_ATS_REVERSAL:			strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_150005);	break;

			default:						strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_150004);	break;
			}

			// Sub Title
			m_pDevCmn->fnSCR_DisplayString(3, strTemp);

			// Under Guide
			m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_150002));

			{
				int	i, nIndex;
				CString strCmd, strData;

				nEnableCount = 0;

				if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER ||
					TranCode == TC_DYNAMICFLOWL_1ST || TranCode == TC_DYNAMICFLOWL_2ND || TranCode == TC_PINCHANGE )			// [#2150] US Justin 2012.09.27 Enable Advertisement on Dynamic Flow Host Connection.
				{
					for (i = 0; i < 6; i++)
					{
						if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_TRAN_ADV_ENDISFLAG1+i))
							nEnableCount++;
					}

					if (nEnableCount > 0)
					{
						NH_SCR_CONFIG eFrontConfig = m_pDevCmn->m_pConfig->GetScreenConfig(SCR_FRONT);

						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayImage(2, TRUE);
						m_pDevCmn->fnSCR_DisplayImage(3, TRUE);

						strData.Format(L"%d", nEnableCount);
						m_pDevCmn->fnSCR_SetDisplayData(L"APTranAdv", strData);

						nRefreshTime = MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_TRAN_ADV_DISP_TIME);
						strData.Format(L"%d", nRefreshTime);
						m_pDevCmn->fnSCR_SetDisplayData(L"APTranAdvTime", strData);

						nIndex = 1;
						for (i = 0; i < 6; i++)
						{
							if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_TRAN_ADV_ENDISFLAG1+i))
							{
								strCmd.Format(L"APTranAdv%d", nIndex++);
								strData.Format(L"\\ATM\\ADVERTISEMENT\\%d_%d\\TRANADV_%02d.jpg", eFrontConfig.nWidth, eFrontConfig.nHeight, i+1);
								m_pDevCmn->fnSCR_SetDisplayData(strCmd, strData);

							}
						}
					}
				}

				if (nEnableCount == 0)
				{
					m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
					m_pDevCmn->fnSCR_DisplayImage(2, FALSE);
					m_pDevCmn->fnSCR_DisplayImage(3, FALSE);
				}
			}
		}
		m_pDevCmn->fnSCR_DisplayScreen(150);
		dwStartTime = GetTickCount();
	}

	///////////////////////////////////
	// CONNECT TO HOST

	m_pDevCmn->fnAPL_SetProcCount('3');

	// Communication with Host
	while(1)
	{
		if ((nRes = BIZ_ALPHI_SendHost()) != RES_OK)
			break;

		if ((nRes = BIZ_ALPHI_RecvHost()) != RES_OK)
			break;

#ifdef APP_LOCAL_MODE
		m_pDevCmn->fstrSCR_WaitTime(3);
		nRes = RES_OK;
#endif

		// Host OK
		m_pDevCmn->fnAPL_SetProcCount('5');

		if (nEnableCount > 0)
		{
			DWORD	dwDisplayTime = (GetTickCount() - dwStartTime);
			DWORD	dwTranAdvTime = (nEnableCount * nRefreshTime);

			if (dwDisplayTime < dwTranAdvTime)
			{
				m_pDevCmn->fstrSCR_WaitMiliTime(dwTranAdvTime - dwDisplayTime);
			}
		}

		// show ALPHI result screen.
		switch(m_TranCodeAlphi)
		{
		//case TC_ATS_ENQUIRY:			LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_150101), ABORT_SCR_TIMEOUT);	break;
		case TC_ATS_REDEMPTION:			LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_150101), ABORT_SCR_TIMEOUT);	break;
		case TC_ATS_REVERSAL:			LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_150102), ABORT_SCR_TIMEOUT);	break;
		default:						break;
		}

		// everything's fine. go to exit.
		break;
	}

	if ((nRes != RES_OK) && (bShowScreen == TRUE))
	{
		NHDEBUG(DBG_INFO, (_T("SHOW ERROR SCREEN\n")));

		///////////////////////////////////
		// DISPLAY SCREEN

		m_pDevCmn->fnSCR_DisplayPrevSet(122);
		m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);

		// [#2541] 2018.03.19 HJAHN 1. 팝업 문구 변경 2. 팝업 문구 한 줄로 표시
		strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122202));

		m_pDevCmn->fnSCR_DisplayString(4, strTemp);
		m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
		m_pDevCmn->fnSCR_DisplayScreen(122);

		m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);
	}

	m_pDevCmn->TranStatus = OrgTranStatus;

	return nRes;	
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_NH_ALPHI_FailureNotice()
RETURN TYPE  : -
PARAMETER    : nReason (ALPHI Response)
DESCRIPTION  : Display ALPHI Failure Notice, caused by
				1) Host Declined / Timeout
				2) Insufficient Loyalty Points
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ALPHI_FailureNotice(BIZ_RETURN nReason)
{
	NVDump('O', 'C', "00", L"P_NHALP", L"FailNotice");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ALPHI_FailureNotice]\n"));

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2541] AU HJAHN 2018.03.19 ALPHI Detailed error messages (BIT39) 
			// 1. 트렁크 호환을 위해 Screen Number  변경 ('151'->'164')
			// 2. ALPHI 거래 실패에 대한 상세 에러 메세지 표시 
			m_pDevCmn->fnSCR_DisplayPrevSet(181);

			// Setting Screen
			{
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_164001));

				m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_164002)); 
				
				if ( nReason == RES_HOST_DENIED )
				{									
					if(m_ALPHI_ResponseRedemption.Bit39 == "3036" || m_ALPHI_ResponseEnquiry.Bit39 == "3036" )
						m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_164206));
					else if(m_ALPHI_ResponseRedemption.Bit39 == "3132" || m_ALPHI_ResponseEnquiry.Bit39 == "3132" )
						m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_164212));
					else if(m_ALPHI_ResponseRedemption.Bit39 == "3134" || m_ALPHI_ResponseEnquiry.Bit39 == "3134" )
						m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_164214));
					else if(m_ALPHI_ResponseRedemption.Bit39 == "3333" || m_ALPHI_ResponseEnquiry.Bit39 == "3333" )
						m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_164233));
					else if(m_ALPHI_ResponseRedemption.Bit39 == "3336" || m_ALPHI_ResponseEnquiry.Bit39 == "3336" )
						m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_164236));
					else if(m_ALPHI_ResponseRedemption.Bit39 == "3531" || m_ALPHI_ResponseEnquiry.Bit39 == "3531" )
						m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_164251));
					else if(m_ALPHI_ResponseRedemption.Bit39 == "3931" || m_ALPHI_ResponseEnquiry.Bit39 == "3931" )
						m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_164291));
				}
				else if ( nReason == RES_HOST_SEND_ERR || nReason == RES_HOST_RECV_ERR )
				{
					m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrAPL_GetErrorMessage());
				}

				m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_164101));
				m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_164102));
			}
			
			m_pDevCmn->fnSCR_DisplayScreen(164, KEYIN_TIME_OUT, PIN_MENU_MODE);
			//end of [#2541]

			bShowScreen = FALSE;
		}
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT || GetKeyStr == S_NO)
			{
				g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;
				m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_CONFIRM_FEE, L"USER CANCEL (ALPHI)");
				return RES_USER_EXIT;
			}
			else
			if (GetKeyStr == S_TIMEOVER)
			{
				if ( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME) == 0
					||  P_NH_NOR_NeedMoreTime() != RES_YES )
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));

					g_sBizFlowInfo.nReasonforCancel = RES_USER_TIMEOUT;
					m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_CONFIRM_FEE, L"TIMEOUT (ALPHI)");
					return RES_USER_TIMEOUT;
				}
				bShowScreen = TRUE;
			}
			else
			if (GetKeyStr == S_YES)
			{
				// remove error code occurred from ALPHI transaction.
				P_NH_NOR_CleanUp();

				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}


/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_ALPHI_LoopTest()
RETURN TYPE  : -
PARAMETER    : 
DESCRIPTION  : Execute Loop Test. Used by Operator Program.
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_ALPHI_LoopTest()
{
	BIZ_RETURN nRes = RES_NG;
	m_TranCodeAlphi = TC_ATS_LOOPTEST;

	nRes = P_NH_ATS_Transaction(FALSE);

	return nRes;
}
#endif
// end of [#2494]
