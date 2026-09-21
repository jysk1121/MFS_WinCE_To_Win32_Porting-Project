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
extern CTimeCheck g_AgingCheck;
#endif

//------------------------------------------------------------------
//	Implement
//------------------------------------------------------------------
#define DISPLAY_LOYALTY_FEE						1
#define DISPLAY_LINE_FEE						2
#define DISPLAY_LOYALTY_AND_LINE_FEE			3
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_MX_WelcomeProc()
RETURN TYPE  : TRUE : Business Flow를 처리 했다.
FALSE : Business Flow를 처리하지 않았다.
PARAMETER    : 
DESCRIPTION  : Business Flow의 시작을 판단한다.
-------------------------------------------------------------------*/
BOOL CTranCmn::BIZ_MX_WelcomeProc(BOOL bForceShowing)
{
	if ((m_pDevCmn->fnSCR_GetCurrentScreenNo() != 101) || (bForceShowing == TRUE))
	{
		int		i;
		CString strWelcome = _T("");
		CString tmp;
		BOOL bExistWelComeMsg = FALSE;

		// Initialize for transaction
		fnAPP_SetEJournalData(TRUE);
		fnAPP_InitializeDS();
		MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID,		L"");		// [#2351] US Justin Empty AID

		// Get Welcome Message.
		for (i = 0; i < 3; i++)
		{
			tmp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WELCOME_MESSAGE1+i);
			tmp.TrimRight(); //2010.02.16 공백 제거하고 표시하게 수정함

			if (tmp.GetLength() > 0)	// Welcome Message가 1개라도 있는 경우에만 유효함 KSK 2010.03.08
				bExistWelComeMsg = TRUE;

			strWelcome += tmp + L"\n";
		}

		// Setting Screen 
		{

			SetBackImageOnWelcomeScreenLayout();		// [#2537] US Justin 2018.03.06 Combine US,CA,MX AP data 

			m_pDevCmn->fnSCR_DisplayPrevSet(101);

			// 1. Setting Welcome Msg and Card reader Animation
			if(bExistWelComeMsg!=TRUE)	
				strWelcome = L"";
			SetCardReadScreenValue(strWelcome);			// [#2537] US Justin 2018.03.06 Combine US,CA,MX AP data 

			// 2. Setting Additional Function button
			SetAdditionalFunctionButton();				// [#2537] US Justin 2018.03.06 Combine US,CA,MX AP data 
			
			// 3. Setting Advertisement
			SetAdvertisementOnWelcomeScreenLayout();	// [#2537] US Justin 2018.03.06 Combine US,CA,MX AP data 

			// 4. Setting Weather
			SetWeatherOnWelcomeScreenLayout();			// [#2537] US Justin 2018.03.06 Combine US,CA,MX AP data 

			// 5. Setting Notice
			SetNoticeOnWelcomeScreenLayout();			// [#2537] US Justin 2018.03.06 Combine US,CA,MX AP data 

			// 6. ETC
			m_pDevCmn->fnSCR_SetDisplayData(L"APProximity", L"off");

			// 7. TID QR CODE	[#J007]
			SetTidQRCodeOnWelcomeScreenLayout();
		}

		m_pDevCmn->fnSCR_DisplayScreen(101);
		m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);

		// [#2205] US KSK 2013.06.28
		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
		{
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_FLICKER_OPTION) == 0)	// ALWAY 인 경우에만 FLICKER ON
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_PIN, FLICKER_ON);

			// [#2333] NH KSK 2015.03.06 EPP LED GUIDE 제어
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
			{
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_LED_GUIDE_OPTION) == 0)			// Always인 경우 LED GUIDE ON
					m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_CONTINUE);			
				else if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_LED_GUIDE_OPTION) == 1)	// Only Idle인 경우 LED GUIDE ON
					m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_CONTINUE);
				else
					m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_OFF);
			}
			// end of [#2333]
		}
		// end of [#2205]

#ifdef APP_AGING_MODE
		g_AgingCheck.SetTargetTimeAfterSec(5);
#endif

		return FALSE;
	}

#ifdef APP_AGING_MODE
	if (g_AgingCheck.IsElapsedTimes() == TRUE)
	{
		// [#2205] US KSK 2013.06.28
		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
			m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);
		else
			SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);
		// end of [#2205]

		BIZ_MX_StartFlowProc(FID_MX_NOR_COMMON);
		return TRUE;
	}
#else

	// 1. CHECK ADA
	// Disable ADA
	/*
	if ((m_pDevCmn->fnSNS_GetEnhancedAudio() == TRUE) && ((DidMediaExist() == FALSE))) 
	{
		TurnOffCardReaderFlicker(TRUE);	

		BIZ_MX_StartFlowProc(FID_NH_ADA_COMMON);

		return TRUE;
	}
	// 2. CHECH MCU EVENT : 카드가 없는 상태에서 새로 넣었을 경우만 거래 Flow를 시작하도록 처리
	else 
	*/
	if ((DidMediaExist() == FALSE) && (LIB_MainMenuDeviceEvent() == DEV_MCU) && (m_pDevCmn->fnMCU_GetDeviceStatus() == NORMAL))	// [#2325] NH KSK 2015.01.22 LIB_MainMenuDeviceEvent return 변경으로 인해 비교문 변경
	{
		TurnOffCardReaderFlicker();			// [#2351] US Justin 2015.06.30

		BIZ_MX_StartFlowProc(FID_MX_NOR_COMMON);

		return TRUE;
	}
#endif

	return FALSE;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_MX_NOR_InputCWAmount()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : 출금 금액을 사용자로 부터 입력 받는다.
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_MX_NOR_InputCWAmount(BOOL bUseFastCash)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_MX_NOR_InputCWAmount]\n"));

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
		LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113111), ABORT_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
		/*
		m_pDevCmn->fnSCR_DisplayPrevSet(122);
		m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
		m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113111));
		m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
		m_pDevCmn->fnSCR_DisplayScreen(122);
		m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
		*/
		NVDump('O', 'C', "00", L"P_MXNOR", L"NG_36");

		return RES_CASH_NOT_AVAILABLE;
	}

	///////////////////////////////////
	//	FAST CASH
	///////////////////////////////////
	if ((m_pDevCmn->m_FastCaseNo == 0) && (bUseFastCash == TRUE))		// Fast Cash
	{
		NVDump('O', 'C', "00", L"P_MXNOR", L"FastCash");

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

					// [#RWC6-12] Denomination Selection Support
					// Other/Choose Bills
					if ( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DENOMINATION_SELECT_ENABLE) ==  ENABLE) && (TranCode == TC_WITHDRAWAL) ) // RWC6-564 Other in FastCash
						m_pDevCmn->fnSCR_DisplayString(12, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_112005));
					else
						m_pDevCmn->fnSCR_DisplayString(11, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_112004));
						// end of [#RWC6-12]

					nIndex = 4;

					for (i=0; i<CDU_CST_MAXFASTCASH; i++)
					{
						if (m_pDevCmn->m_nFastCash[i])
						{
							strDispFastCash.Format(L"   %s %d   ", SCREEN_CURRENCY, m_pDevCmn->m_nFastCash[i]);
							m_pDevCmn->fnSCR_DisplayString(nIndex++, strDispFastCash);
						}
					}

					// US 방출 모드일 경우 화면 Warning 문구 표시
					if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
						m_pDevCmn->fnSCR_DisplayString(10, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_112003));

				}

				m_pDevCmn->fnSCR_DisplayScreen(112, KEYIN_TIME_OUT, PIN_MENU_MODE);
				bShowScreen = FALSE;
			}

			///////////////////////////////////
			// GET KEY STRING
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_37");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}
				else if (GetKeyStr == S_TIMEOVER)
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_MXNOR", L"NG_38");	// [#2024] NH KSK 2011.02.24
					return RES_USER_TIMEOUT;
				}
				else if (GetKeyStr == S_OTHER)
				{
					bOtherAmountRoof = TRUE;
					break;
				}
				else if (GetKeyStr.GetLength() > 0)
				{
					// remove... symbol, white space
					GetKeyStr.Replace(SCREEN_CURRENCY, L"");
					GetKeyStr.TrimLeft();
					GetKeyStr.TrimRight();

					nInputAmount = Asc2Int(GetKeyStr);

					NHDEBUG(DBG_INFO, (_T("REMOVE SYMBOL CHAR - VALUE [%s] - [%d]\n"), GetKeyStr, nInputAmount));

					if (!IsNum(GetKeyStr) || IsZero(GetKeyStr))
					{
						LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113110), ABORT_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
						/*
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113110));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);
						m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
						*/

						NVDump('O', 'C', "00", L"P_MXNOR", L"NG_39");	// [#2024] NH KSK 2011.02.24
						return RES_PROGRAM_ERROR;
					}
					else if (!m_pDevCmn->fbCDU_IsDispensible(nInputAmount))
					{
						LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113111), ABORT_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
						/*
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113111));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);
						m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
						*/
						NVDump('O', 'C', "00", L"P_MXNOR", L"NG_40");	// [#2024] NH KSK 2011.02.24
						return RES_CASH_NOT_AVAILABLE;
					}
					else if (nInputAmount > m_pDevCmn->m_MoneyMaxOut)
					{
						LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113108), ABORT_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
						/*
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113108));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);
						m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
						*/
						NVDump('O', 'C', "00", L"P_MXNOR", L"NG_41");	// [#2024] NH KSK 2011.02.24
						return RES_CASH_MAX_OVER;
					}
					else if (nInputAmount < m_pDevCmn->m_MoneyMinOut)
					{
						LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113109), ABORT_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
						/*
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113109));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);
						m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
						*/
						NVDump('O', 'C', "00", L"P_MXNOR", L"NG_42");	// [#2024] NH KSK 2011.02.24
						return RES_CASH_MIN_OVER;
					}

					// CENT 포함 12자리
					m_sUserSelection.strMoney.Format(L"%010d00", nInputAmount);

					return RES_OK;
				}
			}

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
		NVDump('O', 'C', "00", L"P_MXNOR", L"CWOtherAmount");

		int	nRetryCount = 3;

		if (bUseFastCash == FALSE)
			bOtherAmountRoof = TRUE;

		bShowScreen = TRUE;
		g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
		while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0) && (bOtherAmountRoof == TRUE))
		{
			///////////////////////////////////
			// CHECK STATUS

			m_pDevCmn->fnCDU_GetCashDispenseInfo();
			if (m_pDevCmn->m_FastCaseNo == -1 || m_pDevCmn->fnAPL_GetAvailTrans() == TRAN_WITH_NOT)
			{
				NHDEBUG(DBG_INFO, (_T("FastCashNo(-1) or TRAN_WITH_NOT DETECTED\n")));
				LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113111), ABORT_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
				/*
				m_pDevCmn->fnSCR_DisplayPrevSet(122);
				m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
				m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113111));
				m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
				m_pDevCmn->fnSCR_DisplayScreen(122);
				m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
				*/
				NVDump('O', 'C', "00", L"P_MXNOR", L"NG_43");	// [#2024] NH KSK 2011.02.24
				return RES_CASH_NOT_AVAILABLE;
			}

			///////////////////////////////////
			// DISPLAY SCREEN
			if (bShowScreen == TRUE)
			{
				NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

				CString			strtmp_Amount;
				CString			strdisp_Amount;
				CStringArray	strtmp_ArrayAmount;
				BOOL			bUseComma = FALSE;

				// [#2115] MX KSK 2012.02.07
				int				nCashDenomination[8] = { 0, };

				if (m_pDevCmn->m_strCurrencyID == CURRENCY_TYPE)
				{
					nCashDenomination[0] = CASH_DENOMINATION1;
					nCashDenomination[1] = CASH_DENOMINATION2;
					nCashDenomination[2] = CASH_DENOMINATION3;
					nCashDenomination[3] = CASH_DENOMINATION4;
					nCashDenomination[4] = CASH_DENOMINATION5;
					nCashDenomination[5] = CASH_DENOMINATION6;
					nCashDenomination[6] = CASH_DENOMINATION7;
					nCashDenomination[7] = CASH_DENOMINATION8;
				}
				else
				{
					nCashDenomination[0] = MULTI_CASH_DENOMINATION1;
					nCashDenomination[1] = MULTI_CASH_DENOMINATION2;
					nCashDenomination[2] = MULTI_CASH_DENOMINATION3;
					nCashDenomination[3] = MULTI_CASH_DENOMINATION4;
					nCashDenomination[4] = MULTI_CASH_DENOMINATION5;
					nCashDenomination[5] = MULTI_CASH_DENOMINATION6;
					nCashDenomination[6] = MULTI_CASH_DENOMINATION7;
					nCashDenomination[7] = MULTI_CASH_DENOMINATION8;
				}
				// end of [#2115]

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
					m_pDevCmn->fnSCR_DisplayString(4, L"DOLLAR");		// 방출 모드와 상관없이 화면에는 모두 $로 표시
					m_pDevCmn->fnSCR_DisplayString(5, L"INT_DOLLAR");

					// Multiple
					m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113102));

					strMultipleMsg.Empty();
					strtmp_ArrayAmount.RemoveAll();

					for(int i=0; i<8; i++)
					{
						if (m_pDevCmn->WithAvail & (DENOMINATION_BIT1<<i))
						{
							strdisp_Amount.Format(L"%s %d ", SCREEN_CURRENCY, nCashDenomination[i]);	// 방출 모드와 상관없이 화면에는 모두 $로 표시
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

					strTemp.Format(L"%s %d", SCREEN_CURRENCY, m_pDevCmn->m_MoneyMaxOut);		// 방출 모드와 상관없이 화면에는 모두 $로 표시

					m_pDevCmn->fnSCR_DisplayString(9, strTemp);

					m_pDevCmn->fnSCR_DisplayString(10, L"1");	// min
					m_pDevCmn->fnSCR_DisplayString(11, L"4");	// max

					// auto run
					m_pDevCmn->fnSCR_DisplayString(12, L"off");

					// US 방출 모드시 문구 추가
					if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
						m_pDevCmn->fnSCR_DisplayString(13, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113600));

				}

				m_pDevCmn->fnSCR_DisplayScreen(113, KEYIN_TIME_OUT, PIN_MENU_MODE);
				bShowScreen = FALSE;
			}

			///////////////////////////////////
			// GET KEY STRING
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					NVDump('O', 'C', "00", L"P_MXNOR", L"NG_44");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}
				else if (GetKeyStr == S_TIMEOVER)
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_MXNOR", L"NG_45");	// [#2024] NH KSK 2011.02.24
					return RES_USER_TIMEOUT;
				}
				else if (GetKeyStr.GetLength() > 0)
				{
					nInputAmount = Asc2Int(GetKeyStr);

					NHDEBUG(DBG_INFO, (_T("Input Amount to convert integer [%d]\n"), nInputAmount));

					if (nInputAmount > m_pDevCmn->m_MoneyMaxOut)
					{
						LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113104), INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
						/*
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_INFO);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113104));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);
						m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);
						*/
						nRetryCount--;
						bShowScreen = TRUE;
					}
					else if (nInputAmount < m_pDevCmn->m_MoneyMinOut)
					{
						LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113105), INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
						/*
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_INFO);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113105));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);
						m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);
						*/
						nRetryCount--;
						bShowScreen = TRUE;
					}
					else if (m_pDevCmn->fbCDU_IsDispensible(nInputAmount))
					{
						// CENT 포함 12자리
						m_sUserSelection.strMoney.Format(L"%010d00", nInputAmount);
						return RES_OK;
					}
					else
					{
						LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113106), INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
						/*
						m_pDevCmn->fnSCR_DisplayPrevSet(122);
						m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_INFO);
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113106));
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayScreen(122);
						m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);
						*/
						nRetryCount--;
						bShowScreen = TRUE;
					}
				}
			}

			Delay_Msg(50);
		}

		if (nRetryCount <= 0)
		{
			NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
			/*
			m_pDevCmn->fnSCR_DisplayPrevSet(122);
			m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
			m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112));
			m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
			m_pDevCmn->fnSCR_DisplayScreen(122);
			m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
			*/
			NVDump('O', 'C', "00", L"P_MXNOR", L"NG_46");	// [#2024] NH KSK 2011.02.24
			return RES_CASH_RETRY_OVER;
		}

		if (g_TimeCheck.IsElapsedTimes() == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
			NVDump('O', 'C', "00", L"P_MXNOR", L"NG_47");	// [#2024] NH KSK 2011.02.24
			return RES_USER_TIMEOUT;
		}
	}

	NHDEBUG(DBG_INFO, (_T("ABNORMAL PROCESSING\n")));
	NVDump('O', 'C', "00", L"P_MXNOR", L"NG_48");	// [#2024] NH KSK 2011.02.24
	return RES_USER_EXIT;
}
// [#2137] MX PCS 2012.07.24
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_MX_NOR_SurchargeFeeNotice()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Display Surcharge [133번 스크린]
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_MX_NOR_SurchargeFeeNotice()
{
	NVDump('O', 'C', "00", L"P_MXNOR", L"SurFeeNotice");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_MX_NOR_SurchargeFeeNotice]\n"));

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;
//	int			nTotalSurchargeAmount = 0;	[#2147] MX KSK 2012.08.18

	// [#2159] US Justin 2012.10.19 Display surcharge even surcharge amount is zero
	// Host에서 수신받은 수수료 Check (못받았을 경우는 수수료 화면 Display 하지 않음)
	// if (Asc2Int(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT)) <= 0)
	//	return RES_OK;
	// End of [#2159]

	// [#2147] MX KSK 2012.08.17 WINCE5.0과 코드를 동일하게 하도록 수정

	// [#2142] MX KMK 2012.08.10 
	// 화면에 뿌려주는 값은 Surcharge Amount + TAX이며
	// PESO / USD(DOMESTIC CARD) 일때는 '!' field (=Amount2 + qbC)
	// USD(INTERNATIONAL CARD) 일때는 'qbB' field에 저 값이 들어가 있다
//	if ( (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
//		&& (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_INTERNATIONAL_TYPE) )
//	{
//		// USD Mode (International Card)일 경우 qbB 필드의 값을 가져다 쓴다
//		// 지금 상황에선 LINE_USAGE_FEE라는 필드명이 무효한 상황... 그냥 qbB이다
//		nTotalSurchargeAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));
//	}
//	else
//	{
//		// 그 외의 경우에는 '!' 필드(=surcharge amount+tax)의 값을 가져온다
//		// 소스상에서는 surcharge amount + tax 계산시킴
//		int nSurchargeAmount = Asc2Int(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT));
//		int nIVATAX = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX));
//		nTotalSurchargeAmount = nSurchargeAmount + nIVATAX;
//	}
//
//	if ( nTotalSurchargeAmount == 0 )
//	{
//		// 수수료값이 0일 경우 이 화면을 보여주지 않고 다음 화면으로 넘어간다
//		return RES_OK;
//	}

	int nSurcharge		= Asc2Int(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT));
	int nTax			= Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX));
	int nLineUsageFee	= Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));
	int nDisplaySurcharge = 0;

	if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
	{
		// USD 방출 모드인 경우
		if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_DOMESTIC_TYPE) // Domestic Card인 경우
			nDisplaySurcharge = nSurcharge + nTax;
		else																				  // International Card인 경우		
			nDisplaySurcharge = nLineUsageFee;
	}
	else
	{
		// Peso 방출 모드인 경우
		nDisplaySurcharge = nSurcharge + nTax;
	}

	// [#2159] US Justin 2012.10.19 Display surcharge even surcharge amount is zero
	//if (nDisplaySurcharge <= 0)
	//	return RES_OK;
	// [#2159]
	// end of [#2147]


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

			// [#2159] US Justin 2012.10.19 Display surcharge even surcharge amount is zero
			/*
			m_pDevCmn->fnSCR_DisplayPrevSet(133);

			// Setting Screen
			{
				CString strTemp;

				// Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_133001));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_133002));

				// ************ Contents. *******************
				// "FOR PERFORMING THIS TRANASTION"
				m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_133003));

				// "Owner"
				CString strTempText1,strTempText2;
				strTempText1 = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER);	
				strTempText1.TrimLeft();
				strTempText1.TrimRight();
				m_pDevCmn->fnSCR_DisplayString(5, strTempText1);
				
				// WILL CHARGE XX.XX TAX INCLUDED
				strTempText1.Format(_T("%s"),m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_133005));

// 				strTempText2.Format(_T("%s %s"), SCREEN_CURRENCY,MakeMoneyCent(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT)));
				strTempText2.Format(_T("%s %s"), SCREEN_CURRENCY, MakeMoneyCent( Int2Asc(nDisplaySurcharge) ) );	// [#2147] MX KSK 2012.08.18
				strTempText1.Replace(_T("XX.XX"),strTempText2);
	
				m_pDevCmn->fnSCR_DisplayString(6, strTempText1);
				m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_133006));
			}
			m_pDevCmn->fnSCR_DisplayScreen(133, KEYIN_TIME_OUT, PIN_MENU_MODE);
			*/
			
			if (nDisplaySurcharge <= 0)
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(136);

				// Setting Screen
				{
					CString strOwner, strTempText;
					// Surcharge "Owner"
					strOwner.Format(L"%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER) );
					strOwner.TrimLeft();
					strOwner.TrimRight();

					// APValue1, Title
					m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_136001));
					
					// APvalue2, "Please note that as a X_OWNER client,"
					strTempText.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_136002) );
					strTempText.Replace(_T("X_OWNER"), strOwner);
					m_pDevCmn->fnSCR_DisplayString(2, strTempText);

					// APValue3 "if you use your X_OWNER debit card at this ATM"
					strTempText.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_136003) );
					strTempText.Replace(_T("X_OWNER"), strOwner);
					m_pDevCmn->fnSCR_DisplayString(3, strTempText);

					// APValue4 "you will not be charged a fee for a cash withdrawal,"
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_136004));

					// APValue5 "a balance inquiry or a PIN change."
					m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_136005));

					// APValue6 "Continue"
					m_pDevCmn->fnSCR_DisplayString(18, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_136006));
				}
				m_pDevCmn->fnSCR_DisplayScreen(136, KEYIN_TIME_OUT, PIN_MENU_MODE);
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(133);

				// Setting Screen
				{
					CString strTemp;

					// Title
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_133001));

					// Sub Title
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_133002));

					// ************ Contents. *******************
					// "FOR PERFORMING THIS TRANASTION"
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_133003));

					// "Owner"
					CString strTempText1,strTempText2;
					strTempText1 = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER);	
					strTempText1.TrimLeft();
					strTempText1.TrimRight();
					m_pDevCmn->fnSCR_DisplayString(5, strTempText1);
					
					// WILL CHARGE XX.XX TAX INCLUDED
					strTempText1.Format(_T("%s"),m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_133005));

					// [#2281] US Justin 2014.06.18 Add PESO (USD Dispensing, Domesitc card used)
					// strTempText2.Format(_T("%s %s"), SCREEN_CURRENCY, MakeMoneyCent( Int2Asc(nDisplaySurcharge) ) );	// [#2147] MX KSK 2012.08.18
					if( (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)&&(MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_DOMESTIC_TYPE) )
						strTempText2.Format(_T("%s %s PESOS"), SCREEN_CURRENCY, MakeMoneyCent( Int2Asc(nDisplaySurcharge) ) );
					// [#2332] MX Justin 2015.02.12 MX Screen Text Modification
					else if( (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)&&(MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) != MX_DOMESTIC_TYPE) )
						strTempText2.Format(_T("%s %s USD"), SCREEN_CURRENCY, MakeMoneyCent( Int2Asc(nDisplaySurcharge) ) );
					// End of [#2332]
					else
						strTempText2.Format(_T("%s %s"), SCREEN_CURRENCY, MakeMoneyCent( Int2Asc(nDisplaySurcharge) ) );
					// End of [#2281]

					strTempText1.Replace(_T("XX.XX"),strTempText2);
		
					m_pDevCmn->fnSCR_DisplayString(6, strTempText1);
//					m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_133006));			// [#2164] MX Justin 2012.11.08 Change Screen Text. => Included in  133005.
				}
				m_pDevCmn->fnSCR_DisplayScreen(133, KEYIN_TIME_OUT, PIN_MENU_MODE);
			}
			// End of [#2159]

			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(1);
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
				if (LIB_IsReversalCondition())
				{
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

					if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
						MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 5);	// Customer canceled
				}

				NVDump('O', 'C', "00", L"P_MXNOR", L"NG_14");
				g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				if (P_NH_NOR_NeedMoreTime() != RES_YES )
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_MXNOR", L"NG_17_1");

					if (LIB_IsReversalCondition())
					{
						MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

						if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
							MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 5);	// Customer canceled
					}
					g_sBizFlowInfo.nReasonforCancel = RES_USER_TIMEOUT;
					return RES_USER_TIMEOUT;
				}
				bShowScreen = TRUE;
			}
			else if (GetKeyStr == S_YES)
			{
				return RES_OK;
			}
		}
#endif

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}
// end of [#2137]

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_MX_NOR_fnAPP_BankFeeDisplay()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Display Bank Name and Fee
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_MX_NOR_fnAPP_BankFeeDisplay() // BANK FEE Display , Maguire : 0 or NULL 일경우 모두 체크(HOST 수신쪽에서 0 혹은 NULL 일경우 수신안함) 
{
	NVDump('O', 'C', "00", L"P_MXNOR", L"BankFeeDisp");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_MX_NOR_fnAPP_BankFeeDisplay]\n"));

	CString		GetKeyStr;
	CString		strTemp, strTemp2;
	BOOL		bShowScreen = TRUE;

	int			nDisplayField = 0;
	int			nLenLoyaltyFee = 0, nLenLineUsageFee = 0, nLenBankName = 0;

	nLenLoyaltyFee   = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE).GetLength();
	nLenLineUsageFee = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE).GetLength();
	nLenBankName     = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_BANK_NAME).GetLength();
		
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(nLenLoyaltyFee == 0 && nLenLineUsageFee == 0) // 2개의 FEE 모두 수신 못했을 경우 BANK FEE DISPLAY 안함.
		return RES_OK; 

		if(nLenLoyaltyFee != 0 && nLenLineUsageFee != 0)
			nDisplayField = DISPLAY_LOYALTY_AND_LINE_FEE;
		else if(nLenLoyaltyFee != 0)
			nDisplayField = DISPLAY_LOYALTY_FEE;
		else if(nLenLineUsageFee != 0)
			nDisplayField = DISPLAY_LINE_FEE;
		else
			return	RES_OK;
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

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

			m_pDevCmn->fnSCR_DisplayPrevSet(130);

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
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_130001));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_130002));

				switch(nDisplayField)		
				{
					/////////////////////////////////////////////////////////////////////////////////
					// [#2137] MX PCS 2012.06.27 " qbC 필드는 화면에 나타나지 않는다."
				case DISPLAY_LOYALTY_AND_LINE_FEE: // Bank Name, Loyalty Fee, Line Usage Fee
					strTemp.Format(_T("%s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_130003));
					m_pDevCmn->fnSCR_DisplayString(4, strTemp);
					if(nLenBankName != 0)
					{
						strTemp.Format(_T("< %s >"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_BANK_NAME));
						m_pDevCmn->fnSCR_DisplayString(5, strTemp);
					}
					// 은행 이름이 전문으로 날라오기 때문에 예외 처리를 해주어야 할 듯...없으면 화면 문구가 이상해짐.
					else
					{
						strTemp.Format(_T("< %s >"), L"BANK");
						m_pDevCmn->fnSCR_DisplayString(5, strTemp);
					}
					strTemp.Format(_T("%s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_130004));
					strTemp2.Format(_T("%s"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE));
					strTemp.Replace(_T("XXX"), strTemp2);
					m_pDevCmn->fnSCR_DisplayString(6, strTemp);

					strTemp.Format(_T("%s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_130007));
					m_pDevCmn->fnSCR_DisplayString(7, strTemp);

					strTemp.Format(_T("%s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_130004));
					strTemp2.Format(_T("%s"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));
					strTemp.Replace(_T("XXX"), strTemp2);
					m_pDevCmn->fnSCR_DisplayString(8, strTemp);
					break;

				case DISPLAY_LOYALTY_FEE:	// Bank Name, Loyalty Fee
					strTemp.Format(_T("%s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_130003));
					m_pDevCmn->fnSCR_DisplayString(5, strTemp);

					if(nLenBankName != 0)
					{
						strTemp.Format(_T("< %s >"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_BANK_NAME));
						m_pDevCmn->fnSCR_DisplayString(6, strTemp);
					}
					// 은행 이름이 전문으로 날라오기 때문에 예외 처리를 해주어야 할 듯...없으면 화면 문구가 이상해짐.
					else
					{
						strTemp.Format(_T("< %s >"), L"BANK");
						m_pDevCmn->fnSCR_DisplayString(5, strTemp);
					}
					strTemp.Format(_T("%s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_130004));
					strTemp2.Format(_T("%s"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE));
					strTemp.Replace(_T("XXX"), strTemp2);
					m_pDevCmn->fnSCR_DisplayString(7, strTemp);
					break;

				case DISPLAY_LINE_FEE: // Bank Name,Line Usage Fee
					strTemp.Format(_T("%s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_130006));
					m_pDevCmn->fnSCR_DisplayString(4, strTemp);

					if(nLenBankName != 0)
					{
						strTemp.Format(_T("< %s >"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_BANK_NAME));
						m_pDevCmn->fnSCR_DisplayString(5, strTemp);
					}
					// 은행 이름이 전문으로 날라오기 때문에 예외 처리를 해주어야 할 듯...없으면 화면 문구가 이상해짐.
					else
					{
						strTemp.Format(_T("< %s >"), L"BANK");
						m_pDevCmn->fnSCR_DisplayString(5, strTemp);
					}
					strTemp.Format(_T("%s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_130004));
					strTemp2.Format(_T("%s"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));
					strTemp.Replace(_T("XXX"), strTemp2);
					m_pDevCmn->fnSCR_DisplayString(6, strTemp);

					strTemp.Format(_T("%s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_130005));
					m_pDevCmn->fnSCR_DisplayString(7, strTemp);
					break;
				default:
					break;
				}
			}

			m_pDevCmn->fnSCR_DisplayScreen(130, KEYIN_TIME_OUT, PIN_MENU_MODE);
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
				if (LIB_IsReversalCondition())
				{
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

					if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
						MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 5);	// Customer canceled
				}

				NVDump('O', 'C', "00", L"P_MXNOR", L"NG_14");
				g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				if (P_NH_NOR_NeedMoreTime() != RES_YES )
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_MXNOR", L"NG_17_1");

					if (LIB_IsReversalCondition())
					{
						MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

						if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
							MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 5);	// Customer canceled
					}

					g_sBizFlowInfo.nReasonforCancel = RES_USER_TIMEOUT;
					return RES_USER_TIMEOUT;
				}

				bShowScreen = TRUE;
			}
			else if (GetKeyStr == S_YES)
			{
				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	if (LIB_IsReversalCondition())
	{
		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 5);	// Customer canceled
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	g_sBizFlowInfo.nReasonforCancel = RES_USER_TIMEOUT;
	return RES_USER_TIMEOUT;
}
// end of  [#631]


// [#2137] MX PCS 2012.06.27 "Prosa 전문 수신 후 수수료 화면."
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_MX_NOR_ExchangeFeeNotice()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Host에서 수신받은 Exchage Fee 표시
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_MX_NOR_ExchangeFeeNotice()
{
	NVDump('O', 'C', "00", L"P_MXNOR", L"ShowTransSum");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_MX_NOR_ExchangeFeeNotice]\n"));

	/////////////////////////////////////////////////////////////////////
	// 받는 필드별 디스플레이 정보
	// 거래 종류에 상관없이 페소이면 A,B,C,D 필드
	// 달러이면 D,E 필드를 사용함.

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// [#2142] MX KMK 2012.08.16 5.0 소스 반영, 출금거래에서 qbE 미수신시 거래 종료 처리
	if (TranCode == TC_WITHDRAWAL)
	{
		if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE).GetLength() <= 0)
			return RES_NG;

		// [#2399] MX Justin 2016.02.26 Proceed for zero surcharge.
		CString strExcFee = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE);
		CString strTotalFee = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TOTAL_FEE);

		if( (Asc2Int(strExcFee) <= 0) && (Asc2Int(strTotalFee) <= 0) )
			return RES_OK;
		// End of [#2399]
	}
	// end of [#2142]

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

			m_pDevCmn->fnSCR_DisplayPrevSet(132);

			// Setting Screen
			{
				CString txtFromDat, realValue;

				// [#2138] 132번 화면 수정
				// [APValue2] 'Exchange Fee Notice'
				txtFromDat = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_132001);
				m_pDevCmn->fnSCR_DisplayString(1, txtFromDat);

				// [APValue3] 'AMOUNT REQUESTED    $XXX.XX'
				txtFromDat = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_132002);
				int nRequestAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
				realValue.Format(_T("%s%s"), SCREEN_CURRENCY, MakeMoneyCent(Int2Asc(nRequestAmount)));
				txtFromDat.Replace(_T("XXX.XX"), realValue);	// XXX.XX -> realValue
				m_pDevCmn->fnSCR_DisplayString(2, txtFromDat);

				// [APValue4] '#SURCHARGE_OWNER#  will access an US Dollar'
				txtFromDat = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_132003);
				realValue.Format(_T("%s"), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER));// <== 이 부분에 Surcharge Owner 들어감.
				txtFromDat.Replace(_T("X_OWNER"), realValue);	// X_OWNER -> realValue
				m_pDevCmn->fnSCR_DisplayString(3, txtFromDat);
				
				// [APValue5] 'Exchange Fee of X_FEE   of the Requested of'
				txtFromDat = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_132004);
//				realValue.Format(_T("%s"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE));
				/////////////////////////////////////////////////////////////////////////////////////////////////////////////
				// 이 곳에서 Exchange Fee를 가지고 퍼센트를 계산함.
				//CString strExchangeFeePercent;
				char	chTemp[1024] = { 0, };
				double	dwRequestedAmount, dwExchangeFee;

				dwRequestedAmount = (double)(0.01 * Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT)));	// cent 제거
				sprintf(chTemp, "%S", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE));
				dwExchangeFee = atof(chTemp);

				//[#2210] MX Justin 2013.07.12 Exchange Fee Format USD/Percent 
				//[#2149] US Justin 2012.09.21 Display Exchange Fee Amount instead of %%%%
				// strExchangeFeePercent.Format(L"%.2f", ((dwExchangeFee / dwRequestedAmount)*100));
				//strExchangeFeePercent.Format(L"$%.2f", dwExchangeFee);
				// End of [#2149]
				/////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//txtFromDat.Replace(_T("X_FEE"), strExchangeFeePercent);	// X_FEE -> realValue

				CString strTemp2;
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EXCHANGEFEE_PERCENTAGE) == ENABLE)
					strTemp2.Format(L"%.2f%%", ((dwExchangeFee / dwRequestedAmount)*100));
				else
					strTemp2.Format(L"$%0.2f(USD)", dwExchangeFee); 
				txtFromDat.Replace(_T("X_FEE"), strTemp2);	// X_FEE -> realValue
				// End of [#2210]

				m_pDevCmn->fnSCR_DisplayString(4, txtFromDat);

				// [APValue6] 'withdrawal. This is in addition to any other fees'
				txtFromDat = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_132005);
				m_pDevCmn->fnSCR_DisplayString(5, txtFromDat);

				// [APValue7] 'associated with this transaction.'
				txtFromDat = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_132006);
				m_pDevCmn->fnSCR_DisplayString(6, txtFromDat);

				// [APValue8] 'DO YOU WISH TO CONTINUE?'
				txtFromDat = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_132007);
				m_pDevCmn->fnSCR_DisplayString(7, txtFromDat);

				// end of [#2138]
			}

			m_pDevCmn->fnSCR_DisplayScreen(132, KEYIN_TIME_OUT, PIN_MENU_MODE);
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
				if (LIB_IsReversalCondition())
				{
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

					if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
						MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 5);	// Customer canceled
				}

				NVDump('O', 'C', "00", L"P_MXNOR", L"NG_14");
				g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_MXNOR", L"NG_17_1");	// [#2024] NH KSK 2011.02.24

				if (P_NH_NOR_NeedMoreTime() != RES_YES )
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_MXNOR", L"NG_17_1");

					if (LIB_IsReversalCondition())
					{
						MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

						if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
							MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 5);	// Customer canceled
					}

					g_sBizFlowInfo.nReasonforCancel = RES_USER_TIMEOUT;
					return RES_USER_TIMEOUT;
				}
				bShowScreen = TRUE;
			}
			else if (GetKeyStr == S_YES)
			{
				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	if (LIB_IsReversalCondition())
	{
		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 5);	// Customer canceled
	}

	g_sBizFlowInfo.nReasonforCancel = RES_USER_TIMEOUT;
	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}
// [#2137] MX PCS 2012.07.25
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_MX_NOR_CheckCustomer()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : HOST에서 수신받은 정보를 바탕으로 DOMESTIC / INTERNATIONAL을 판단함
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_MX_NOR_CheckCustomer()
{
	NHDEBUG(DBG_INFO, (_T("CTranCmn::P_MX_NOR_CheckCustomer()\n")));

	// 예외 처리
	if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
	{
		if (TranCode == TC_WITHDRAWAL)
		{
			if ( !(((MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_HYOSUNG_TYPE)
				&&(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE)==ENABLE)) ||
				((MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE)
				&&(m_STD3_TDL_Data.m_bProceedDCC==TRUE))) )
			{
				if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE).GetLength() > 0)
				{
					// qbG�� ���� �޴� ������ Domestic Card�� (���� ȭ���� ���� ������)
					MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE, MX_DOMESTIC_TYPE);
				}
				else
				{
					NHDEBUG(DBG_INFO, (_T("  International Card \n")));
					NHDEBUG(DBG_INFO, (_T("  qbE = [%s]\n"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE) ));
					NHDEBUG(DBG_INFO, (_T("  qbF = [%s]\n"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TOTAL_FEE) ));

					// International Card
					MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE, MX_INTERNATIONAL_TYPE);

					// International
					if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE).GetLength() <= 0 ||									// qbE : Exchange Fee
	// 					MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_TOTAL_AMOUNT).GetLength() <= 0)
						MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TOTAL_FEE).GetLength() <= 0)	// [#2142] MX KMK 2012.08.16		// qbF : Total Fee = qbB(Surcharge+tax) + qbE(Exchange Fee)
					{
						// qbE�Ǵ� qbF�� �ȿ� ���� �ŷ� ���� ó��
						NHDEBUG(DBG_INFO, (_T("  Return NG \n")));
						return RES_NG;
					}

					// [#2180] MX Justin 2013.03.08 International Card EMV transaction
					/*
					// EMV �ŷ��ε��� qbE Field�� �� ���� �ŷ� ���� ó���� �ʿ��� (����ó��)
					if (m_pDevCmn->fnMCU_IsEmvTransaction())
						return RES_NG;
					*/
					// End of [#2180]
				}
			}
		}
		else if (TranCode == TC_INQUIRY)
		{
			if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE).GetLength() > 0)
			{
				// qbG를 수신 받는 경우는 Domestic Card임 (모든 화폐는 페소 단위임)
				MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE, MX_DOMESTIC_TYPE);
			}
			else
			{
				// International Card
				MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE, MX_INTERNATIONAL_TYPE);

				// [#2180] MX Justin 2013.03.08 International Card EMV transaction
				// EMV 거래인데도 qbE Field가 온 경우 거래 종료 처리가 필요함 (예외처리)
				/*
				if (m_pDevCmn->fnMCU_IsEmvTransaction())
					return RES_NG;
				*/
				// End of [#2180]
			}
		}
	}

	NHDEBUG(DBG_INFO, (_T("  Return OK \n")));
	return RES_OK;
}

// [#2149] MX Justin 2012.09.21 Customer Final Confirmation before Host communication.
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_MX_NOR_CustomerFinalConfirmation()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Customer Final Confirmation before Host communication.
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_MX_NOR_CustomerFinalConfirmation()
{
	NVDump('O', 'C', "00", L"P_MX_NOR", L"CustomerFinalConfirmation");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_MX_NOR_CustomerFinalConfirmation]\n"));

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

			m_pDevCmn->fnSCR_DisplayPrevSet(134);
			// Setting Screen
			{
				m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_134001));
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_134002));
			}

			m_pDevCmn->fnSCR_DisplayScreen(134, KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(1);
#endif
		}


#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			return RES_OK;
		}
#endif
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_YES)
			{
				return RES_OK;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				return RES_USER_TIMEOUT;
			}
			return RES_USER_EXIT;
		}
		Delay_Msg(50);
	}
	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}

// End of [#2149]

// [#2266] MX Justin 2014.05.07 Exchange Rate when a Domestic Card is used at USD Dispensing ATM
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_MX_NOR_ExchangeRateForDomesticCard()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Display Exchange Rate when a domestic card is used at USD Dispensing ATM.
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_MX_NOR_ExchangeRateForDomesticCard()
{
	NVDump('O', 'C', "00", L"P_MXNOR", L"ExcRate4DomesticCard");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_MX_NOR_ExchangeRateForDomesticCard()]\n"));

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	if (TranCode != TC_WITHDRAWAL)
		return RES_NG;

	if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE).GetLength() <= 0)
		return RES_NG;

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

			m_pDevCmn->fnSCR_DisplayPrevSet(139);

			// Setting Screen
			{
				CString txtFromDat, realValue;

				// Withdrawal Summary
				txtFromDat = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_139001);
				m_pDevCmn->fnSCR_DisplayString(1, txtFromDat);

				// Do you agree?
				txtFromDat = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_139002);
				m_pDevCmn->fnSCR_DisplayString(2, txtFromDat);

				// AMOUNT REQUESTED in dollars    XXX.XX'
				txtFromDat = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_139003);
				int nRequestAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
				realValue.Format(_T("%s"), MakeMoneyCent(Int2Asc(nRequestAmount)));
				txtFromDat.Replace(_T("XXX.XX"), realValue);	
				m_pDevCmn->fnSCR_DisplayString(3, txtFromDat);

				// Exchange Rate XXX.XX
				txtFromDat = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_139004);
				realValue.Format(_T("%s"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE));
				txtFromDat.Replace(_T("XXX.XX"), realValue);	// X_OWNER -> realValue
				m_pDevCmn->fnSCR_DisplayString(4, txtFromDat);
				
				// AMOUNT REQUESTED in Pesos    XXX.XX' => Converting to PESOS with exchange Rate
				txtFromDat = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_139005);
				char	chTemp[1024] = { 0,};
				WideToMulti(chTemp, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE), sizeof(chTemp));
				float fConvertAmt =(float)( ((float)atof(chTemp)) * (0.01 * nRequestAmount) );
				realValue.Format(L"%.2f", fConvertAmt);
				txtFromDat.Replace(_T("XXX.XX"), realValue);	
				m_pDevCmn->fnSCR_DisplayString(5, txtFromDat);

			}
			m_pDevCmn->fnSCR_DisplayScreen(139, KEYIN_TIME_OUT, PIN_MENU_MODE);
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
				if (LIB_IsReversalCondition())
				{
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

					if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
						MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 5);	// Customer canceled
				}

				NVDump('O', 'C', "00", L"P_MXNOR", L"NG_14");
				g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				if (P_NH_NOR_NeedMoreTime() != RES_YES )
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_MXNOR", L"NG_17_1");

					if (LIB_IsReversalCondition())
					{
						MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

						if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
							MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 5);	// Customer canceled
					}
					g_sBizFlowInfo.nReasonforCancel = RES_USER_TIMEOUT;
					return RES_USER_TIMEOUT;
				}
				bShowScreen = TRUE;
			}
			else if (GetKeyStr == S_YES)
			{
				return RES_OK;
			}
		}
		Delay_Msg(50);
	}
	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}
// End of [#2266]
