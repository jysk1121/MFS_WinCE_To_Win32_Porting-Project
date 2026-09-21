#include "stdafx.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"
#ifndef UNDER_CE
#  include <math.h>
#endif

#define	DBG_CALL		1
#define DBG_INFO		1

/**
 *  The total elapsed time to change the EPP into PIN mode.
 */
#define EPP_PIN_MODECHANGE_TIMEOUT 30000 /* 30 seconds */
#define EPP_PIN_POLL_DELAY 1000 /* 1 second */

//------------------------------------------------------------------
//	Define Symbol
//------------------------------------------------------------------
extern CTimeCheck	g_TimeCheck;

#ifdef APP_AGING_MODE
#define	AGING_KEYIN_TIME 1
	CTimeCheck g_AgingCheck;
#endif

//------------------------------------------------------------------
//	Utility Functions
//------------------------------------------------------------------



//------------------------------------------------------------------
//	Implement
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: IsPasswordMode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CTranCmn::IsPasswordMode()
{
	return m_bAdaPasswordMode;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: SetEppToPinMode()
 RETURN TYPE  : BOOL
 PARAMETER    : 
 DESCRIPTION  : Sets the EPP to PIN-entry mode. This helper function was
 added to add a delay for Pre-PCI EPPs which can only allow 1 PIN operation
 per 30 seconds. When Pre-Balance is enabled, the ATM may do more than 1
 PIN operation within 30 seconds, so the AP should delay the customer until
 the PIN can perform another operation. The error code from the EPP is (-402).
-------------------------------------------------------------------*/
BOOL CTranCmn::SetEppToPinMode()
{
	int totalElapsedTime = 0;

	while (!m_pDevCmn->fnAPL_DeviceEnDisable(DEV_PIN, 
		ENABLE, 
		FALSE, 
		PIN_PASSWORD_MODE, 
		L"", 
		PIN_PASSWORD_MIN, 
		PIN_PASSWORD_MAX, 
		PIN_PASSWORD_AUTO_TRUE, 
		PIN_PASSWORD_TERM))
	{
		if (totalElapsedTime >= EPP_PIN_MODECHANGE_TIMEOUT) 
		{
			NHERROR((L"The EPP could not change into PIN mode.\r\n"));
			return FALSE;
		}

		// Show the screen on the second iteration only
		if (totalElapsedTime == 0)
		{
			if (IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(161, L"161.wav");
				m_pAdaCtrl->fnExp_PlayScreenWave(161);
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(161);
				m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116002));
				m_pDevCmn->fnSCR_DisplayScreen(161);
			}
		}

		Delay_Msg(EPP_PIN_POLL_DELAY);
		totalElapsedTime += EPP_PIN_POLL_DELAY;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: VATReadCard()
 RETURN TYPE  : BIZ_RETURN
 PARAMETER    : 
 DESCRIPTION  : Reads the MS or EMV card and sets all necessary paramters for a transaction
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::VATReadCard(int ICProcScreenNum, int RemoveCardScreenNum, bool UpdateLanguageOnVAT)
{
	BIZ_RETURN cardReadState;
	switch (m_nEventKind)
	{
	case DEV_RFID:
		m_pDevCmn->nKindOfMedia = MEDIA_MS;
		cardReadState = RES_NOR_MS_TRANS;
		break;
	default:
		cardReadState = P_EMV_CheckTransMode();
	}

	// If the transaction is not EMV, then read the card before proceeding
	if ( cardReadState != RES_EMV_IC_FIRST )
	{
		// Read the MS
		if ( P_NH_NOR_ReadCard(m_nEventKind) != RES_OK )
		{
			NHDEBUG(DBG_CALL, (L"MS card read failure\r\n"));
			return RES_CARD_ERROR;
		}

		// We're done with the MCU now, since the MS has been read
		m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);
	}

	// Card read state machine
	while (true)
	{
		if (cardReadState == RES_NOR_MS_TRANS)
		{
			// NB: A Successful IC read will also use this branch after the EMV data has been read.
			//     It is due to the PIN entry, etc.

			// Unconditionally set language to English for VAT
			if (UpdateLanguageOnVAT)
			{
				m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_ENGLISH));
			}

			cardReadState = RES_OK;

			// This has to be done to set the amount on the chip?
			if(m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				cardReadState = P_NH_NOR_EMV_IC_Processing();
			}

			// Card read complete after PIN entry on MS read
			break;			
		}
		else if (cardReadState == RES_EMV_IC_FIRST)
		{
			NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_NOR_Common] [IC FIRST TRANSACTION]\n"));

			// Attempt an EMV read. Recycle the result to the state machine
			// cardReadState : RES_MAKE_AID, RES_EMV_FALLBACK, RES_EMV_TERMINATE 
			cardReadState = P_EMV_ICCardRead(ICProcScreenNum);
		}
		else if (cardReadState == RES_MAKE_AID)
		{
			// cardReadState : RES_MULTI_AID_SELECT, RES_MAKE_AID, RES_NOR_MS_TRANS, RES_EMV_TERMINATE, RES_EMV_FALLBACK
			cardReadState = P_EMV_MakeAIDList();
		}
		else if (cardReadState == RES_MULTI_AID_SELECT)
		{
			// cardReadState : RES_MAKE_AID, RES_NOR_MS_TRANS, RES_EMV_TERMINATE, RES_EMV_FALLBACK, RES_USER_EXIT
			cardReadState = P_EMV_SelectMultiAIDList();
		}
		else if (cardReadState == RES_EMV_FALLBACK)
		{
			// cardReadState : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			if ((cardReadState = P_EMV_ContinueFallBackProc(RemoveCardScreenNum)) != RES_OK)
			{
				NHDEBUG(DBG_CALL, (L"Fallback continuation failure\r\n"));
				break;
			}

			// NB: At this point, the transaction is MS because the IC has failed

#if (US_VERSION)
			if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_ENABLE_DISABLE) == EMV_FALLBACK_DISABLE) &&
				(LIB_IsICCard(m_sCardData.strISO2Data) == TRUE) &&
				(m_pDevCmn->m_bUnKnownAID_SkipPE != TRUE)	)				// Magnetic Stripe Transaction for Unknown IC Card => Continue
			{
				cardReadState = RES_EMV_FALLBACK_DISABLED;
				NHDEBUG(DBG_CALL, (L"EMV fallback detected, but fallback is disabled\r\n"));
				break;
			}
#endif

			// Do another MS card read
			if ((cardReadState = P_NH_NOR_ReadCard()) != RES_OK)
			{
				NHDEBUG(DBG_CALL, (L"Fallback MS card read failure\r\n"));
				break;
			}

			// We're done with the MCU now, since the MS has been read
			m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);

			// Do another cycle for the MS read
			cardReadState = RES_NOR_MS_TRANS;
		}
		else
		{
			// Break하는 조건 
			// 1: EMV_TERMINATE인 경우
			// 2: RES_USER_EXIT인 경우
			// 3: RES_USER_TIMEOUT인 경우
			break;
		}
	}

	return cardReadState;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: IsAdaTransaction()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CTranCmn::IsAdaTransaction()
{
	return m_bAdaTransaction;
}

//[#2117] US PCS 2012.01.12 "ADA 거래 시 금액 수동입력 여부 문의 함수"
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: IsAdaManualInput()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
// [#2427] Justin
/*
BOOL CTranCmn::IsAdaSuccessiveInput()
{
	return m_bAdaSuccessiveInput;
}
*/
// End of [#2427]
//end of [#2117]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: IsStartTransaction()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CTranCmn::IsStartTransaction()
{
	return m_bStartTransaction;
}

// [#2317] US Justin 2015.01.05 Domestic BIN
void CTranCmn::LoadDCCDomesticBin()
{
	m_DomesticBinRange.LoadXMLBinRangeData();
}
// End of [#2317]

// [#2375] US Justin 2015.10.27 Software TTS
long CTranCmn::GetTransactionScreenTimeOut()
{
	long nTimeOut = 0;
	if( IsAdaTransaction() )
	{
		if( m_pDevCmn->fnSNS_GetEnhancedAudio() )
			nTimeOut = ADA_VOICE_MAX_TIMEOUT;
	}
	else
		nTimeOut = MAX_DEVSCR_TIME;

	return nTimeOut;
}
// End of [#2375]

// [#2447] US Justin 2016.20.20
CString CTranCmn::GetCardHolderName()
{
	CString strHolderName = L"";
	if( m_pDevCmn->fnMCU_IsEmvTransaction() )
	{
		int nLen = 0;
		unsigned char	szTemp[1024] = {0,};
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_HolderName, &nLen, szTemp) == EMV_RSLT_OK)
			strHolderName.Format(L"%S", szTemp);
	}
	else
	{
		if( m_sCardData.strISO1Data.GetLength() > 0)
		{
			CStringArray	strTmpArray;
			SplitString(m_sCardData.strISO1Data, L"^", strTmpArray);		// Field Separator
			if(strTmpArray.GetSize() >=2)									// Second field is NAME
				strHolderName = strTmpArray.GetAt(1);
		}
	}
	NHDEBUG(DBG_CALL, (L"GetCardHolderName (Before Adjust) = [%s]\n", strHolderName));

	if(strHolderName.GetLength()>0)
	{
		strHolderName.Replace( L"$", L" ");	// Name Separator
		strHolderName.Replace( L"/", L",");	// Sur Name Separator
		strHolderName.TrimLeft();
		strHolderName.TrimRight();
	}
	NHDEBUG(DBG_CALL, (L"[CTranCmn::GetCardHolderName] return (%s)\n", strHolderName));
	return strHolderName;
}
// End of [#2447]

// [#2472] US Justin 2017.02.07 
void CTranCmn::CheckLocalDCCEligibility()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::CheckLocalDCCEligibility]\n"));
	m_pDevCmn->m_nDCCLocalAvail = DCC_PRECHECK_OFF;

	#if(APP_DCC_WITHOPTION )
		if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_OPTION_ELIGIBLE) == DCC_WITHDRAWAL_SETTING_EMV_MASTER )
		{
			// Check DCC Enabled Condition
			m_pDevCmn->m_nDCCLocalAvail = DCC_PRECHECK_DCCNA;
			if(  ( ((m_sSTD1_DynamicFlowResp.nService_DCC )&&(MemGetInt( _MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC)==ENABLE)) || (m_bUseDualHost == TRUE) )  || 		// STD1 or Dual Host DCC
				 ( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE) && (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DCC)==ENABLE) ) )	// STD3 DCC.
			{
				while (true)
				{
					if(!m_pDevCmn->fnMCU_IsEmvTransaction())					// EMV Transaction
						break;	
							 
					CString sAID = MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID);
					NHDEBUG(DBG_CALL, (L"  EMV Transaction, AID=[%s]\n", sAID));
					sAID.TrimLeft();	sAID.TrimRight();
					if( sAID.GetLength()<10 )									// Valid AID
						break;

					CString strAID10 = sAID.Left(10);
					if(strAID10.CompareNoCase(L"A000000004") != 0)				// MasterCard, MAESTRO, CIRRUS, US MAESTRO
						break;
					NHDEBUG(DBG_CALL, (L"  AID is MasterCard, Maestro, Cirrus, or US Maestro\n"));

					unsigned char szTempa[1024];
					int				nLen = 0;
					memset(szTempa, 0, sizeof(szTempa));
					if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_IsuCountryCod, &nLen, szTempa) == EMV_RSLT_OK)
					{
						CString strCountryCod = MakeUnPack(szTempa, nLen);
						NHDEBUG(DBG_INFO, (L"Success to get Issuer Country Code = [%s]\n", strCountryCod));

						if( strCountryCod.GetLength()==4 )						// Valid Country Code
						{
							#if (US_VERSION )
								if(	strCountryCod.CompareNoCase(L"0840") != 0)				// USA : 840  (NON USA)
									m_pDevCmn->m_nDCCLocalAvail = DCC_PRECHECK_DCCAVAIL;
							#elif(CA_VERSION)
								if(	strCountryCod.CompareNoCase(L"0124") != 0)				// Canada : 124 (NON CANADA)
									m_pDevCmn->m_nDCCLocalAvail = DCC_PRECHECK_DCCAVAIL;
							#endif
						}		
					}
					break;
				}
			}
		}
	#endif

	NHDEBUG(DBG_CALL, (L"  DCC LOCAL OPTION = [%d]\n", m_pDevCmn->m_nDCCLocalAvail));
}
// End of [#2472]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_WelcomeProc()
 RETURN TYPE  : TRUE : Business Flow를 처리 했다.
				FALSE : Business Flow를 처리하지 않았다.
 PARAMETER    : 
 DESCRIPTION  : Business Flow의 시작을 판단한다.
-------------------------------------------------------------------*/
BOOL CTranCmn::BIZ_WelcomeProc(BOOL bForceShowing)
{
	if ((m_pDevCmn->fnSCR_GetCurrentScreenNo() != 101) || (bForceShowing == TRUE))
	{
		int		i;
		CString strWelcome = _T("");
		CString tmp;
		BOOL bExistWelComeMsg = FALSE;	// KSK 2010.03.08

		// [#2487] AU KSK 2017.06.08
		CString strTemp, strTemp2;
		int nIdleDisplaySurcharge1, nIdleDisplaySurcharge2;

		nIdleDisplaySurcharge1 = 0;
		nIdleDisplaySurcharge2 = 0;
		// end of [#2487]

		// Initialize for transaction
		fnAPP_SetEJournalData(TRUE);
		fnAPP_InitializeDS();
		MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID,		L"");		// [#2351] US Justin Empty AID

		// Get Welcome Message.
		for (i = 0; i < 3; i++)
		{
			tmp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WELCOME_MESSAGE1+i);
			tmp.TrimRight();

			if (tmp.GetLength() > 0)		// Welcome Message가 1개라도 있는 경우에만 유효함 KSK 2010.03.08
				bExistWelComeMsg = TRUE;

			strWelcome += tmp + L"\n";
		}

		// [#2175] US Justin 2013.01.18 US EMV 
		// Setting Screen 
		{
			
			SetBackImageOnWelcomeScreenLayout();	// [#2011] NH KJW 2011.01.14 WelcomeProc의 화면표시부를 함수화하여, InsertCardProc에서 공통사용함.

			m_pDevCmn->fnSCR_DisplayPrevSet(101);	// KSK 2010.07.02 Change Background시 Check Cashing Button이 잠시 표시되는 Bug Fix

			// 1. Setting Welcome Msg and Card reader Animation
			if(bExistWelComeMsg!=TRUE)	
				strWelcome = L"";
			SetCardReadScreenValue(strWelcome);		// [#2351] US Justin Make Function... Enable Device(MCR, MCR Flicker, RFID) in the routine

			// 2. Setting Additional Function button
			SetAdditionalFunctionButton();			// [#2445] US Justin Make Function

// [#2487] AU KSK 2017.06.08
#if (AU_A_VERSION)
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
				{
					switch(LIB_CalculateSurchargeModeNReuslt(nIdleDisplaySurcharge1, nIdleDisplaySurcharge2))
					{
					case STANDARD_SURCHARGE:
						{
							strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101020);

							strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nIdleDisplaySurcharge1)));
							strTemp.Replace(L"XX.XX(1)", strTemp2);	// Withdrawal Surcharge Amount

							strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nIdleDisplaySurcharge2)));
							strTemp.Replace(L"XX.XX(2)", strTemp2);	// Balance Surcharge Amount
							m_pDevCmn->fnSCR_DisplayString(6, strTemp);
						}
						break;
					case BINLISTED_SURCHARGE:
						{
							strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101021);

							if (nIdleDisplaySurcharge1 == nIdleDisplaySurcharge2)
							{
								strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nIdleDisplaySurcharge1)));
								strTemp.Replace(L"between XX.XX(1) and XX.XX(2)", strTemp2);
							}
							else
							{
								strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nIdleDisplaySurcharge1)));
								strTemp.Replace(L"XX.XX(1)", strTemp2);	// Minimum Surcharge Amount

								strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nIdleDisplaySurcharge2)));
								strTemp.Replace(L"XX.XX(2)", strTemp2);	// Maximum Surcharge Amount
							}
							m_pDevCmn->fnSCR_DisplayString(6, strTemp);
						}
						break;
					case PERCENTAGE_N_BINLISTED_SURCHARGE:
						{
							strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101022);

							if (nIdleDisplaySurcharge1 == nIdleDisplaySurcharge2)
							{
								strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nIdleDisplaySurcharge1)));
								strTemp.Replace(L"between XX.XX(1) and XX.XX(2)", strTemp2);
							}
							else
							{
								strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nIdleDisplaySurcharge1)));
								strTemp.Replace(L"XX.XX(1)", strTemp2);	// Minimum Surcharge Amount

								strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nIdleDisplaySurcharge2)));
								strTemp.Replace(L"XX.XX(2)", strTemp2);	// Maximum Surcharge Amount
							}
													
							strTemp2.Format(L"%0.2f", 0.01 * MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE));
							strTemp.Replace(L"XX.XX(3)", strTemp2);	// percentage	

							if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_MANNER) == LESSER)
								strTemp2 = m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENG_ONLY_LESSER);
							else
								strTemp2 = strTemp2 = m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENG_ONLY_GREATER);
							strTemp.Replace(L"XX.XX(4)", strTemp2);	// Greater or Lesser

							m_pDevCmn->fnSCR_DisplayString(6, strTemp);
						}
						break;
					case PERCENTAGE_SURCHARGE:
						{
							strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101023);

							strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nIdleDisplaySurcharge1)));
							strTemp.Replace(L"XX.XX(1)", strTemp2);	// Withdrawal Surcharge Amount

							strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nIdleDisplaySurcharge2)));
							strTemp.Replace(L"XX.XX(2)", strTemp2);	// Balance Surcharge Amount

							strTemp2.Format(L"%0.2f", 0.01 * MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE));
							strTemp.Replace(L"XX.XX(3)", strTemp2);	// percentage							

							if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_MANNER) == LESSER)
								strTemp2 = m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENG_ONLY_LESSER);
							else
								strTemp2 = strTemp2 = m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENG_ONLY_GREATER);
							strTemp.Replace(L"XX.XX(4)", strTemp2);	// Greater or Lesser
							
							m_pDevCmn->fnSCR_DisplayString(6, strTemp);
						}
						break;
					default:
						break;
					}
				}
				// NZ는 미적용
#endif
// end of [#2487]

			// 3. Setting Advertisement
			SetAdvertisementOnWelcomeScreenLayout();

			// 4. Setting Weather
			SetWeatherOnWelcomeScreenLayout();

			// 5. Setting Notice
			SetNoticeOnWelcomeScreenLayout();

			// 6. ETC
			m_pDevCmn->fnSCR_SetDisplayData(L"APProximity", L"off");

			// 7. TID QR CODE	[#J007]
			SetTidQRCodeOnWelcomeScreenLayout();
		}

		m_pDevCmn->fnSCR_DisplayScreen(101);
		m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);

		// [#GLDV-2890] US Kook 2021.05.21 Support VB Flickers on MX5400
		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX5400")
		{
			// turn on all VB flickers
			// VB flicker is synchronized with MCU flicker, so skip below
			//SetGuideLight(0x00, CDU_VB_FLICKER | EPP_VB_FLICKER);		
		}
		// [#2205] US KSK 2013.06.28
		else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
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

			BIZ_StartFlowProc(FID_NH_NOR_COMMON);
			return TRUE;
		}
	#else

	// 1. CHECK ADA
	if ((m_pDevCmn->fnSNS_GetEnhancedAudio() == TRUE) && ((DidMediaExist() == FALSE))) // [#2175] US Justin 2013.01.18 US EMV 
	{
		TurnOffCardReaderFlicker(TRUE);		// [#2351] US Justin 2015.06.30

		// [#2047] AU KSK 2011.04.13
		#if (AU_VERSION)
			BIZ_AU_C_StartFlowProc(FID_NH_ADA_COMMON);
		#else
			BIZ_StartFlowProc(FID_NH_ADA_COMMON);
		#endif
		// end of [#2047]

		return TRUE;
	}
	// 2. CHECH MCU EVENT : 카드가 없는 상태에서 새로 넣었을 경우만 거래 Flow를 시작하도록 처리
	else if ((DidMediaExist() == FALSE) && (LIB_MainMenuDeviceEvent() == DEV_MCU) && (m_pDevCmn->fnMCU_GetDeviceStatus() == NORMAL))	// [#2325] NH KSK 2015.01.22 LIB_MainMenuDeviceEvent return 변경으로 인해 비교문 변경
	{
		TurnOffCardReaderFlicker();			// [#2351] US Justin 2015.06.30

		#if (AU_VERSION)					// [#2041] AU KSK 2011.03.31
			BIZ_AU_C_StartFlowProc(FID_NH_NOR_COMMON);
		#else
			BIZ_StartFlowProc(FID_NH_NOR_COMMON);
		#endif

		return TRUE;
	}
	else if ((LIB_MainMenuDeviceEvent() == DEV_RFID) && (m_pDevCmn->fnRFID_GetDeviceStatus() == NORMAL))	// [#2325] NH KSK 2015.01.22 LIB_MainMenuDeviceEvent return 변경으로 인해 비교문 변경
	{
		TurnOffCardReaderFlicker();			// [#2351] US Justin 2015.06.30

		#if (AU_VERSION)					// [#2041] AU KSK 2011.03.31
			BIZ_AU_C_StartFlowProc(FID_NH_NOR_COMMON, DEV_RFID);
		#else
			BIZ_StartFlowProc(FID_NH_NOR_COMMON, DEV_RFID);
		#endif

			return TRUE;
	}
	else if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 5, EVENT_IN) == DEV_SCR)
	{
		CString strKeyString = m_pDevCmn->fstrSCR_GetKeyString(1);

		NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), strKeyString));

		// [#2350] US Justin 2015.06.17 3rd Party Application
		FLOW_ID	eChosenFlow = FID_END_OF_FLOW;
		if (strKeyString == L"CHECKCASH")
		{
			eChosenFlow = FID_NH_CCW_FLOWS;
		}
		// [#RWC6-68] PAI Bitcoin button
#if (APP_CUSTOM_PAI)
		else if (strKeyString == L"F6")
		{	
			eChosenFlow = FID_NH_BITCOIN_SVC_FLOWS;
			/*
			int nNumBCEnabled, nEnabledBitcoins;
			GetEnabledBitcoinServices(&nNumBCEnabled, &nEnabledBitcoins);
			if( nNumBCEnabled>1 )													eChosenFlow = FID_NH_BITCOIN_SVC_FLOWS;
			else if( (nNumBCEnabled==1)&&(nEnabledBitcoins==CDLS_SVC_B4U) )			eChosenFlow = FID_NH_B4U_REQ_INFO_FLOWS;	// [#RWC6-16] Bitload4U
			*/
		}
#endif
		else if (strKeyString == L"F8")
		{			
#if (APP_CUSTOM_PAI)
			eChosenFlow = FID_NH_CARDLESS_FLOWS;
#else
			// [#2413] US Justin 2016.04.08 VG for Popmoney and Pin4
			int nNumEnabled, nEnabledServices;

			GetEnabledCardlessServices(&nNumEnabled, &nEnabledServices);

			if		(nNumEnabled > 1)													eChosenFlow = FID_NH_CARDLESS_FLOWS;
			else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_POPMONEY))		eChosenFlow = FID_NH_POPMONEY_FLOWS;
			else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_PIN4))			eChosenFlow = FID_NH_PIN4_FLOWS;
			else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_PAYPALCCA))	eChosenFlow = FID_NH_PAYPAL_FLOWS;			// [#2446] US Justin 2016.09.29 Paypal CCA
			else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_JUSTCASH))		eChosenFlow = FID_NH_JUSTCASH_START;		// [#2445] US Justin 2016.09.28 Just.Cash
			else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_LIBERTYX))		eChosenFlow = FID_NH_LIBERTYX_INIT;			// [#RWC6-59] US William 2019.10.09 LibertyX
			else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_B4U))			eChosenFlow = FID_NH_B4U_REQ_INFO_FLOWS;	// [#RWC6-16] Bitload4U
			else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_DIGITALMINT))	eChosenFlow = FID_NH_DIGITALMINT_START;
#endif
		// end of [#RWC6-68]
		}
		else if (strKeyString == L"F7")	// [RWC6-676] Start SKKim 2024.04.19
		{
#if (APP_TANGOPAY)
			eChosenFlow = FID_NH_TANGOPAY_START;
#endif
		}								// [RWC6-676] End SKKim 2024.04.19

		if( eChosenFlow != FID_END_OF_FLOW)
		{
			// Turn off MCU Flicker
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
				SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);
			else
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);

			// Disable MCU and RFID
			m_pDevCmn->fnAPL_DeviceEnDisable(DEV_MCU, DISABLE, TRUE);
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
			{
				m_pDevCmn->fnRFID_EntryDisable();
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
			}

			BIZ_StartFlowProc(eChosenFlow);
			return TRUE;
		}
		// End of [#2350]
	} 
	else
	{
		//
	}
#endif

	return FALSE;
}

BOOL CTranCmn::BIZ_WelcomeProc_TwoButtons(FLOW_ID nAddOnFlowID, BOOL bForceShowing)
{
	if ((m_pDevCmn->fnSCR_GetCurrentScreenNo() != 100) || (bForceShowing == TRUE))
	{
		int		i;
		CString strWelcome, tmp;
		BOOL bExistWelComeMsg = FALSE;	// KSK 2010.03.08

		// Initialize for transaction
		fnAPP_SetEJournalData(TRUE);
		fnAPP_InitializeDS();
		MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID,		L"");		// [#2351] US Justin Empty AID

		// Get Welcome Message.
		for (i = 0; i < 3; i++)
		{
			tmp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WELCOME_MESSAGE1+i);
			tmp.TrimRight();
			if (tmp.GetLength() > 0)		// Welcome Message가 1개라도 있는 경우에만 유효함 KSK 2010.03.08
				bExistWelComeMsg = TRUE;
			strWelcome += tmp + L"\n";
		}
		if(bExistWelComeMsg!=TRUE)	strWelcome = L"";

		// Setting Screen 
		{
			SetBackImageOnWelcomeScreenLayout();
			m_pDevCmn->fnSCR_DisplayPrevSet(100);

			////////////////////////
			// 1. APValue
			{
				// Welcome Message
				if (bExistWelComeMsg == TRUE)
					m_pDevCmn->fnSCR_DisplayString(1, strWelcome);

				// Disalbe Card and RFID
				m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);
				SetMcuExisted(FALSE);
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
				{
					m_pDevCmn->fnRFID_EntryDisable();
					m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
				}
			}

			////////////////////////
			// 2. APState 
			{
				// F7 (ATM Transaction,		APValue 17)
				// F8 (AddOn AP,			APValue 18)
				m_pDevCmn->fnSCR_DisplayString(17, L"ATM\nTransactions");
				m_pDevCmn->fnSCR_DisplayString(18, L"Value Add\nTransaction");			
				m_pDevCmn->fnSCR_DisplayString(20, L"");						// Turn off POP Money Logo File
				m_pDevCmn->fnSCR_DisplayImage(20, FALSE);						// Turn Off Green Button
			}


			// 3. Setting Advertisement
			SetAdvertisementOnWelcomeScreenLayout();

			// 4. Setting Weather
			SetWeatherOnWelcomeScreenLayout();

			// 5. Setting Notice
			SetNoticeOnWelcomeScreenLayout();

			// 6. ETC
			m_pDevCmn->fnSCR_SetDisplayData(L"APProximity", L"off");

			m_pDevCmn->fnSCR_DisplayScreen(100);
			m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
		}

		// EPP LED 
		// [#GLDV-2890] US Kook 2021.05.21 Support VB Flickers on MX5400
		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX5400")
		{
			// turn on all VB flickers
			SetGuideLight(0x00, CDU_VB_FLICKER | EPP_VB_FLICKER);		
		}
		// end of [#GLDV-2890]
		else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
		{
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_FLICKER_OPTION) == 0)	// ALWAY 인 경우에만 FLICKER ON
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_PIN, FLICKER_ON);

			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
			{
				if (	 MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_LED_GUIDE_OPTION) == 0)	// Always인 경우 LED GUIDE ON
					m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_CONTINUE);			
				else if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_LED_GUIDE_OPTION) == 1)	// Only Idle인 경우 LED GUIDE ON
					m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_CONTINUE);
				else
					m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_OFF);
			}
		}
		return FALSE;
	}

	// 1. CHECK ADA
	if( m_pDevCmn->fnSNS_GetEnhancedAudio() == TRUE )
	{
		BIZ_StartFlowProc(FID_NH_ADA_COMMON);		// Always Start ATM Transaction ===> Need to change later
		return TRUE;
	}
	// 2. Check Screen Input
	else if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 5, EVENT_IN) == DEV_SCR)
	{
		CString strKeyString = m_pDevCmn->fstrSCR_GetKeyString(1);
		NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), strKeyString));
		if (strKeyString == L"F7")
		{
			BIZ_StartFlowProc(FID_NH_NOR_READCARD);
			return TRUE;
		}
		else if (strKeyString == L"F8")
		{
			BIZ_StartFlowProc(nAddOnFlowID);
			return TRUE;
		}
	}
	return FALSE;
}

// [#] NH KJW 2011.01.14 WelcomeProc의 화면표시부를 함수화하여 InsertCardProc에서 사용함.
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: SetBackImageOnWelcomeScreenLayout(BOOL bSupportChangeBackImage)
 RETURN TYPE  : -
 PARAMETER    : TRUE : Change Background Image 기능 지원함(default)
 DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::SetBackImageOnWelcomeScreenLayout(BOOL bSupportChangeBackImage)
{
	NH_SCR_CONFIG eFrontConfig = m_pDevCmn->m_pConfig->GetScreenConfig(SCR_FRONT);

	////////////////////////
	// 0. Back Image
	if ( bSupportChangeBackImage && MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_CHANGE_ENDISFLAG))
	{
		m_nEnableCount = 0;
		for ( int i = 0; i < 6; i++)
		{
			if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_ENDISFLAG_TYPE1+i))
				m_nBackList[m_nEnableCount++] = (i+1);
		}

		if (m_nEnableCount > 0)
		{
			if (m_nBackIndex < 0 || m_nBackList[m_nBackIndex] <= 0)
				m_nBackIndex = 0;
			else
			{
				m_nBackIndex++;
				if (m_nBackIndex >= m_nEnableCount)
					m_nBackIndex = 0;
			}

			if (m_nBackScreenNumber != m_nBackList[m_nBackIndex])
			{
				m_nBackScreenNumber = m_nBackList[m_nBackIndex];

				CString	strBackName;

				if (eFrontConfig.eEngine == SCR_ENG_FLASH_LITE)
					strBackName.Format(L"%s\\%d_%d\\Touch\\Back_%d.jpg", SCREEN_PATH_BACKS, eFrontConfig.nWidth, eFrontConfig.nHeight, m_nBackScreenNumber);	// [#11] NH KSK 2010.09.14
				else
					strBackName.Format(L"%s\\%d_%d\\Function\\Back_%d.jpg", SCREEN_PATH_BACKS, eFrontConfig.nWidth, eFrontConfig.nHeight, m_nBackScreenNumber);	// [#11] NH KSK 2010.09.14

				m_pDevCmn->fnSCR_SetDisplayData(L"APBackImage", strBackName);
			}
		}
	}
	else
	{
		int	nSettingBack = MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_DEFAULT_TYPE);
		if (nSettingBack != m_nBackScreenNumber)
		{
			CString	strBackName;

			if (nSettingBack < 0 || nSettingBack > 6)
			{
				MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_DEFAULT_TYPE, 1);
				nSettingBack = 1;
			}

			m_nBackScreenNumber = nSettingBack;

			if (eFrontConfig.eEngine == SCR_ENG_FLASH_LITE)
				strBackName.Format(L"%s\\%d_%d\\Touch\\Back_%d.jpg", SCREEN_PATH_BACKS, eFrontConfig.nWidth, eFrontConfig.nHeight, m_nBackScreenNumber);	// [#11] NH KSK 2010.09.14
			else
				strBackName.Format(L"%s\\%d_%d\\Function\\Back_%d.jpg", SCREEN_PATH_BACKS, eFrontConfig.nWidth, eFrontConfig.nHeight, m_nBackScreenNumber);	// [#11] NH KSK 2010.09.14

			m_pDevCmn->fnSCR_SetDisplayData(L"APBackImage", strBackName);
		}
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: ShowWelcomeScreen(int nScrNum, int nAPValue1, int nAPValue2 )
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::SetAdvertisementOnWelcomeScreenLayout()
{
	NHDEBUG(DBG_INFO, (_T("\n")));

	NH_SCR_CONFIG eFrontConfig = m_pDevCmn->m_pConfig->GetScreenConfig(SCR_FRONT);

	////////////////////////////////////
	// 3. Setting Advertisement
	{
		int		nEnableCount, i, nIndex;
		CString strTemp, strTemp2;

#if (APP_LIBERTYX)
		int numLibertyXAds = GetNumberOfLibertyXAds(); // [#RWC6-23] US William 2019.09.09 GivePay Extra Ads	// [#RWC6-280] LibertyX Default Ads
#else
		int numLibertyXAds = 0;
#endif
		strTemp.Format(L"%d", MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_GUIDEDISP_TIME));
		m_pDevCmn->fnSCR_SetDisplayData(L"APSwitchTime", strTemp);

		// Initially set the base to the number of GP Ads	// [#RWC6-23] US William 2019.09.09 GivePay Extra Ads	// [#RWC6-280] LibertyX Default Ads
		nEnableCount = numLibertyXAds;
		for (i = 0; i < 6; i++)
		{
			if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_ENDISFLAG1+i))
				nEnableCount++;
		}

		strTemp.Format(L"%d", nEnableCount);
		m_pDevCmn->fnSCR_SetDisplayData(L"APAdv", strTemp);
		NHDEBUG(DBG_INFO, (_T("fnSCR_SetDisplayData(APAdv, %s)\n"), strTemp));

		if (nEnableCount > 0)
		{
			nIndex = 1;
			for (i = 0; i < 6 + numLibertyXAds; i++)
			{
				if (i < 6) 
				{
					if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_ENDISFLAG1+i))
					{
						strTemp.Format(L"APAdv%d", nIndex++);
						strTemp2.Format(L"%s\\%d_%d\\ADV_%02d.jpg", ADVERTISEMENT_PATH, eFrontConfig.nWidth, eFrontConfig.nHeight, i+1);
						m_pDevCmn->fnSCR_SetDisplayData(strTemp, strTemp2);

						NHDEBUG(DBG_INFO, (_T("2(%d)fnSCR_SetDisplayData1strTemp(%s, %s)\n"), i, strTemp, strTemp2));
					}
				}
				else // Handle LibertyX Ads 
				{
					int adIndex = i - 6; // First should be LTX Ad 0 (ADV_1.jpg)

					strTemp.Format(L"APAdv%d", nIndex++);
					strTemp2.Format(L"%s\\%d_%d\\ADV_%02d.jpg", LTX_ADS_PATH, eFrontConfig.nWidth, eFrontConfig.nHeight, adIndex+1);

					if (!FILE_EXISTS(strTemp2))
					{
						continue;
					}

					m_pDevCmn->fnSCR_SetDisplayData(strTemp, strTemp2);

					NHDEBUG(DBG_INFO, (_T("2(%d)fnSCR_SetDisplayData1strTemp(%s, %s)\n"), i, strTemp, strTemp2));
				}
			}
		}
	}
	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: ShowWelcomeScreen(int nScrNum, int nAPValue1, int nAPValue2 )
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::SetWeatherOnWelcomeScreenLayout()
{
	////////////////////////////////////
	// 4. Setting Weather
	NH_OS_VERSION	eOSVersion = m_pDevCmn->m_pConfig->GetOSVersion();
//	if (eOSVersion != NH_OS_NH1800SE_B)					// [#11] NH KSK 2010.09.14 1800SE 기종 Weather Service 미 지원
	if (!(eOSVersion == NH_OS_NH1800SE_B || eOSVersion == NH_OS_NH1500SE))					// [#2267] US KSK 2014.05.08
	{
		COleDateTime		setToday, setTomorrow;
		COleDateTime		sysToday, sysTomorrow;
		COleDateTimeSpan	spanOneDay(1, 0, 0, 0);		// 1 day.
		COleDateTime		CurTime = COleDateTime::GetCurrentTime();

		setToday.SetDate(m_sWeatherInfo[0].nYear, m_sWeatherInfo[0].nMonth, m_sWeatherInfo[0].nDay);
		setTomorrow.SetDate(m_sWeatherInfo[1].nYear, m_sWeatherInfo[1].nMonth, m_sWeatherInfo[1].nDay);

		sysToday.SetDate(CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay());
		sysTomorrow = sysToday + spanOneDay;

		if ((setToday.GetStatus() == COleDateTime::valid) &&
			(setTomorrow.GetStatus() == COleDateTime::valid) &&
			(setToday == sysToday) && (setTomorrow == sysTomorrow))
		{
			CString strName;
			CString strData;

			for (int i = 0; i < 2; i++)
			{
				strName.Format(L"APWeather%d", i+1);
				strData.Format(L"%d|%d|%d|%s|%d|%s|%d|%d", m_sWeatherInfo[i].nYear,
					m_sWeatherInfo[i].nMonth,
					m_sWeatherInfo[i].nDay,
					m_sWeatherInfo[i].strDayOfWeek,
					m_sWeatherInfo[i].nWeatherCode,
					m_sWeatherInfo[i].strTemperatureType.Left(1),
					m_sWeatherInfo[i].nHighestTemp,
					m_sWeatherInfo[i].nLowestTemp);

				m_pDevCmn->fnSCR_SetDisplayData(strName, strData);
			}
		}
	}
	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: SetNoticeOnWelcomeScreenLayout(int nScrNum, int nAPValue1, int nAPValue2 )
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::SetNoticeOnWelcomeScreenLayout()
{
	NH_OS_VERSION	eOSVersion = m_pDevCmn->m_pConfig->GetOSVersion();

	////////////////////////////////////
	// 5. Setting Notice
	//			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_ENABLE))
//	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_ENABLE) && eOSVersion != NH_OS_NH1800SE_B)	// [#11] NH KSK 2010.09.16 1800SE NOTICE 기능 미지원
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_ENABLE))	// [#2267] US KSK 2014.05.08
	{
		// [#2267] US KSK 2014.05.08
		if (!(eOSVersion == NH_OS_NH1800SE_B || eOSVersion == NH_OS_NH1500SE))
		{
			CString strTemp;

			m_bShowNotice = FALSE;

			if ((m_sNoticeInfo.strTitle.GetLength() > 0) ||
				(m_sNoticeInfo.strLine1.GetLength() > 0) ||
				(m_sNoticeInfo.strLine2.GetLength() > 0) ||
				(m_sNoticeInfo.strLine3.GetLength() > 0))
			{
				m_pDevCmn->fnSCR_SetDisplayData(L"APNotice1", m_sNoticeInfo.strTitle);
				m_pDevCmn->fnSCR_SetDisplayData(L"APNotice2", m_sNoticeInfo.strLine1);
				m_pDevCmn->fnSCR_SetDisplayData(L"APNotice3", m_sNoticeInfo.strLine2);
				m_pDevCmn->fnSCR_SetDisplayData(L"APNotice4", m_sNoticeInfo.strLine3);

				m_bShowNotice = TRUE;
			}
		}
		else
			m_bShowNotice = FALSE;
		// end of [#2267]
	}
	else
		m_bShowNotice = FALSE;

	return RES_OK;
}

// [#J007] US Justin
BIZ_RETURN CTranCmn::SetTidQRCodeOnWelcomeScreenLayout()
{
	#if( APP_CUSTOM_CASHDEPOT )		
		m_pDevCmn->fnSCR_DisplayImage(30, TRUE);		// QR White Background
		if( m_pDevCmn->fnAPL_MakeMobileAppQRCode(MOBILEAPP_QR_TYPE_TERMINALID) )
			m_pDevCmn->fnAPL_DisplayQRCodeOnScreen(MOBILEAPP_QR_IMAGEFILE);
	#else
		m_pDevCmn->fnSCR_DisplayImage(30, FALSE);
	#endif

	return RES_OK;
}
// End of [#J007]

// [#2351] US Justin Make CardReadVariable Function
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: SetCardReadScreenValue(CString strWelcome)
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::SetCardReadScreenValue(CString strWelcome)
{
	NH_OS_VERSION	eOSVersion = m_pDevCmn->m_pConfig->GetOSVersion();
	int	nScreenX = GetSystemMetrics(SM_CXSCREEN);
	int	nScreenY = GetSystemMetrics(SM_CYSCREEN);

	BOOL bExistWelComeMsg = FALSE;
	if(strWelcome.GetLength()>0)
		bExistWelComeMsg = TRUE;

	BOOL bRFIDAvailable = FALSE;
	if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE)== ENABLE) && (m_pDevCmn->fnRFID_GetDeviceStatus() == NORMAL) )
		bRFIDAvailable = TRUE;

	// [#RWC6-68] PAI Bitcoin button
	BOOL bButtonMode = FALSE;
	int nNumEnabled, nEnabledServices;
	GetEnabledCardlessServices(&nNumEnabled, &nEnabledServices);
	int nNumBCEnabled, nEnabledBitcoins;
	GetEnabledBitcoinServices(&nNumBCEnabled, &nEnabledBitcoins);
	if ((nNumEnabled >= 1)||(nNumBCEnabled >= 1))
		bButtonMode = TRUE;
	
#if(APP_CUSTOM_PAI)
	if (bButtonMode)
	{
		/* Current file name & location, as of 20191226
		\\ATM\Data\\AP_Background_Welcome_640_480.png
		\\ATM\Data\\AP_Background_Welcome_800_600.png
		\\ATM\Data\\AP_Background_Welcome_1024_600.png
		\\ATM\Data\\AP_Background_Welcome_1024_768.png
		*/

		CString strImagePath;
		strImagePath.Format(L"%s", AP_BG_WELCOME_FILE);
		
		if (nScreenX == 640)
			strImagePath.Replace(L"XXXX", L"640_480");
		else if (nScreenX == 800)
			strImagePath.Replace(L"XXXX", L"800_600");
		else if ((nScreenX == 1024) && (nScreenY == 600))
			strImagePath.Replace(L"XXXX", L"1024_600");
		else if ((nScreenX == 1024) && (nScreenY == 768))
			strImagePath.Replace(L"XXXX", L"1024_768");
		else
			strImagePath.Replace(L"XXXX", L"800_600");

		m_pDevCmn->fnSCR_DisplayString(27, strImagePath); // background change to PAI button mode
	}else
#endif
		m_pDevCmn->fnSCR_DisplayString(27, L"");

	const int nButtonStateMin = 1;
	const int nButtonStateMax = 20;
	// end of [#RWC6-68]
	
	// Value 1 : Welcome Message  (Instruction if Welcome Message is not exist)
	// Value 2 : Instruction

	// Value 3 : ReceiptNO or Cash No		(no RFID)
	// Value 4 : ReceiptNO or Cash No		(no RFID)

	// Value 5 : ReceiptNO or Cash No		(With RFID)
	// Value 6 : ReceiptNO or Cash No		(With RFID)

	// Value 11: Welcome Message  (Instruction if Welcome Message is not exist) (PAI)
	// Value 12: Instruction (PAI)

	// value 17:
	// value 18:

	// value 19: Bitcoin image
	// value 20: Cardless image
	
	// value 30: button mode background (PAI)

	// Image 2 : Swipe Card					(Horizontal - All except 4000W)
	// Image 3 : Insert IC Card				(Horizontal - All except 4000W)
	// Image 4 : Take Card					(Horizontal - All except 4000W)

	// Image 5 : Swipe Card					(Vertical   - 4000W)
	// Image 6 : Insert IC Card				(Vertical   - 4000W)
	// Image 7 : Take Card					(Vertical   - 4000W)

	// Image 8 : Swipe Card (with RFID)		(Available only 1024*600(2700,Halo) or 1024*768(5200SE))
	// Image 9 : Insert IC Card (with RFID)	(Available only 1024*600(2700,Halo) or 1024*768(5200SE))
	// Image 10: RFID Read					(Available only 1024*600(2700,Halo) or 1024*768(5200SE))

	// Image 12: Swipe Card	 (PAI)
	// Image 13: Insert IC Card (PAI)
	// Image 14: Take Card (PAI)
	// Image 18: Swipe Card (with RFID) (PAI)
	// Image 19: Insert IC Card (with RFID) (PAI)
	// Image 20: RFID Read (PAI)

	// [#RWC6-68] PAI Bitcoin button
	// default as FALSE for all images
	for (int i=nButtonStateMin; i<=nButtonStateMax; i++)
		m_pDevCmn->fnSCR_DisplayImage(i, FALSE);
	// end of [#RWC6-68]

	if(m_pDevCmn->fnMCU_IsEmvTransaction())			// EMV Enabled
	{
		if(m_pDevCmn->fnMCU_GetMaterialInfo() != ST_NOT_DETECT)
		{
			// [#2518] US Kook 2018.01.18 Support MX-2800SE		// [#GLDV-2505] Support MX-2800T
			if (eOSVersion == NH_OS_MX2800SE || eOSVersion == NH_OS_MX2800_F_T)
			{
				// "Hello!"
				if (!bExistWelComeMsg)
					// [#RWC6-68] PAI Bitcoin button
					#if(APP_CUSTOM_PAI)
						if (bButtonMode)
							m_pDevCmn->fnSCR_DisplayString(22, L"Hello!");
						else
							m_pDevCmn->fnSCR_DisplayString(29, L"Hello!");
					#else
						m_pDevCmn->fnSCR_DisplayString(29, L"Hello!");
					#endif
					

				if (bExistWelComeMsg)
					// [#RWC6-68] PAI Bitcoin button
					#if(APP_CUSTOM_PAI)
						if (bButtonMode)
							m_pDevCmn->fnSCR_DisplayString(11, strWelcome);
						else
					m_pDevCmn->fnSCR_DisplayString(1, strWelcome);
					#else
						m_pDevCmn->fnSCR_DisplayString(1, strWelcome);
					#endif

				#if(APP_CUSTOM_PAI)
					if (bButtonMode)
						m_pDevCmn->fnSCR_DisplayString(12, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101003));
					else
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101003));
				#else
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101003));
				#endif
				// end of [#RWC6-68]
			}
			else
			// end of [#2518]
			{
				if (bExistWelComeMsg == FALSE)
				{
					// [#RWC6-68] PAI Bitcoin button
					#if(APP_CUSTOM_PAI)
						if (bButtonMode)
						{
							m_pDevCmn->fnSCR_DisplayString(11, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101003));
							m_pDevCmn->fnSCR_DisplayString(12, L"");
						}
						else
						{
					m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101003));
					m_pDevCmn->fnSCR_DisplayString(2, L"");
				}
					#else
						m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101003));
						m_pDevCmn->fnSCR_DisplayString(2, L"");
					#endif
					// end of [#RWC6-68]
				}
				else
				{
					// [#RWC6-68] PAI Bitcoin button
					#if(APP_CUSTOM_PAI)
						if (bButtonMode)
						{
							m_pDevCmn->fnSCR_DisplayString(11, strWelcome);
							m_pDevCmn->fnSCR_DisplayString(12, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101006));
						}
						else
						{
							m_pDevCmn->fnSCR_DisplayString(1, strWelcome);
							m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101006));
						}
					#else
						m_pDevCmn->fnSCR_DisplayString(1, strWelcome);
						m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101006));
					#endif
				}
			}

			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
			{
				// [#RWC6-68] PAI Bitcoin button
				#if(APP_CUSTOM_PAI)
					if (bButtonMode)
						m_pDevCmn->fnSCR_DisplayImage(17, TRUE);
					else
						m_pDevCmn->fnSCR_DisplayImage(7, TRUE);;
				#else
					m_pDevCmn->fnSCR_DisplayImage(7, TRUE);
				#endif
				// end of [#RWC6-68]
			}
			else
			{	// [#RWC6-68] PAI Bitcoin button
				#if(APP_CUSTOM_PAI)
					if (bButtonMode)
						m_pDevCmn->fnSCR_DisplayImage(14, TRUE);
					else
						m_pDevCmn->fnSCR_DisplayImage(4, TRUE);
				#else
					m_pDevCmn->fnSCR_DisplayImage(4, TRUE);
				#endif
				// end of [#RWC6-68]
			}

			m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
			{
				m_pDevCmn->fnRFID_EntryDisable();
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
			}
			SetMcuExisted(TRUE);
		}
		else		// if(m_pDevCmn->fnMCU_GetMaterialInfo() != ST_NOT_DETECT)
		{
			// [#2518] US Kook 2018.01.18 Support MX-2800SE		// [#GLDV-2505] Support MX-2800T
			if (eOSVersion == NH_OS_MX2800SE || eOSVersion == NH_OS_MX2800_F_T)
			{
				// "Hello!"
				if (!bExistWelComeMsg)
					m_pDevCmn->fnSCR_DisplayString(29, L"Hello!");	// [#RWC6-68] PAI Bitcoin button;

				if (bExistWelComeMsg)
					// [#RWC6-68] PAI Bitcoin button
					#if(APP_CUSTOM_PAI)
						if (bButtonMode)
							m_pDevCmn->fnSCR_DisplayString(11, strWelcome);
						else
					m_pDevCmn->fnSCR_DisplayString(1, strWelcome);
					#else
							m_pDevCmn->fnSCR_DisplayString(1, strWelcome);
					#endif
					// end of [#RWC6-68]

				if (bRFIDAvailable)
					// [#RWC6-68] PAI Bitcoin button
					#if(APP_CUSTOM_PAI)
						if (bButtonMode)
							m_pDevCmn->fnSCR_DisplayString(12, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101010));
						else
							m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101010));
					#else
						m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101010));
					#endif
				else
					#if(APP_CUSTOM_PAI)
						if (bButtonMode)
							m_pDevCmn->fnSCR_DisplayString(12, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101004));
						else
							m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101004));
					#else
						m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101004));
					#endif
					// end of [#RWC6-68]
			}
			else	// if (eOSVersion == NH_OS_MX2800SE || eOSVersion == NH_OS_MX2800_F_T)
			// end of [#2518]
			{
				if (bExistWelComeMsg == FALSE)
				{
					if (bRFIDAvailable)
						// [#RWC6-68] PAI Bitcoin button
						#if(APP_CUSTOM_PAI)
							if (bButtonMode)
								m_pDevCmn->fnSCR_DisplayString(11, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101010));
							else
								m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101010));
						#else
							m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101010));
						#endif
					else
						#if(APP_CUSTOM_PAI)
							if (bButtonMode)
								m_pDevCmn->fnSCR_DisplayString(11, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101004));
							else
								m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101004));
						#else
							m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101004));
						#endif
					m_pDevCmn->fnSCR_DisplayString(2, L"");
					m_pDevCmn->fnSCR_DisplayString(12, L"");
					// end of [#RWC6-68]
				}
				else
				{
					#if(APP_CUSTOM_PAI)
						if (bButtonMode)
							m_pDevCmn->fnSCR_DisplayString(11, strWelcome);
						else
							m_pDevCmn->fnSCR_DisplayString(1, strWelcome);
					#else
					m_pDevCmn->fnSCR_DisplayString(1, strWelcome);
					#endif

					if( bRFIDAvailable )
						// [#RWC6-68] PAI Bitcoin button
						#if(APP_CUSTOM_PAI)
							if (bButtonMode)
								m_pDevCmn->fnSCR_DisplayString(12, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101010));
							else
								m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101010));
						#else
							m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101010));
						#endif
					else
						#if(APP_CUSTOM_PAI)
							if (bButtonMode)
								m_pDevCmn->fnSCR_DisplayString(12, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101005));
							else
								m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101005));
						#else
							m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101005));
						#endif
						// end of [#RWC6-68]
				}
			}

			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
			{
				// [#RWC6-68] PAI Bitcoin button
				#if(APP_CUSTOM_PAI)
					if (bButtonMode)
						m_pDevCmn->fnSCR_DisplayImage(16, TRUE);
					else
						m_pDevCmn->fnSCR_DisplayImage(6, TRUE);
				#else
					m_pDevCmn->fnSCR_DisplayImage(6, TRUE);
				#endif
				// end of [#RWC6-68]
			}
			else
			{
				if( bRFIDAvailable )
				{
					// [#RWC6-68] PAI Bitcoin button
					#if(APP_CUSTOM_PAI)
						if (bButtonMode)
						{
							m_pDevCmn->fnSCR_DisplayImage(19, TRUE);
							m_pDevCmn->fnSCR_DisplayImage(20, TRUE);
						}else
						{
							m_pDevCmn->fnSCR_DisplayImage(9, TRUE);
							m_pDevCmn->fnSCR_DisplayImage(10, TRUE);
						}
					#else
					{
						m_pDevCmn->fnSCR_DisplayImage(9, TRUE);
						m_pDevCmn->fnSCR_DisplayImage(10, TRUE);
					}
					#endif
					// end of [#RWC6-68]
					m_pDevCmn->fnRFID_EntryEnable();
					m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
				}
				else
				{
					// [#RWC6-68] PAI Bitcoin button
					#if(APP_CUSTOM_PAI)
					if (bButtonMode)
						m_pDevCmn->fnSCR_DisplayImage(13, TRUE);
					else
						m_pDevCmn->fnSCR_DisplayImage(3, TRUE);
					#else
						m_pDevCmn->fnSCR_DisplayImage(3, TRUE);
					#endif
					// end of [#RWC6-68]
				}
			}
			m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);
			SetMcuExisted(FALSE);
		}
	}
	else					// EMV Disabled
	{
		// [#2518] US Kook 2018.01.18 Support MX-2800SE		// [#GLDV-2505] Support MX-2800T
		if (eOSVersion == NH_OS_MX2800SE || eOSVersion == NH_OS_MX2800_F_T)
		{
			// "Hello!"
			if (bExistWelComeMsg)
			{
				#if(APP_CUSTOM_PAI)
					if (bButtonMode)
						m_pDevCmn->fnSCR_DisplayString(11, strWelcome);
					else
						m_pDevCmn->fnSCR_DisplayString(1, strWelcome);
				#else
				m_pDevCmn->fnSCR_DisplayString(1, strWelcome);
				#endif
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayString(29, L"Hello!");	// [#RWC6-68] PAI Bitcoin button
			}

			if (bRFIDAvailable)
				#if(APP_CUSTOM_PAI)
					if (bButtonMode)
						m_pDevCmn->fnSCR_DisplayString(12, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101011));
					else
						m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101011));
				#else
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101011));
				#endif
			else
				#if(APP_CUSTOM_PAI)
					if (bButtonMode)
						m_pDevCmn->fnSCR_DisplayString(12, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101001));
					else
						m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101001));
				#else
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101001));
				#endif
		}
		else
		// end of [#2518]
		{
			if (bExistWelComeMsg == FALSE)
			{
				if( bRFIDAvailable )
					#if(APP_CUSTOM_PAI)
						if (bButtonMode)
							m_pDevCmn->fnSCR_DisplayString(11, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101011));
						else
							m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101011));
					#else
					m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101011));
					#endif
				else
					#if(APP_CUSTOM_PAI)
						if (bButtonMode)
							m_pDevCmn->fnSCR_DisplayString(11, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101001));
						else
							m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101001));
					#else
					m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101001));
					#endif
				m_pDevCmn->fnSCR_DisplayString(2, L"");
			}
			else
			{
				#if(APP_CUSTOM_PAI)
					if (bButtonMode)
						m_pDevCmn->fnSCR_DisplayString(11, strWelcome);
					else
						m_pDevCmn->fnSCR_DisplayString(1, strWelcome);
				#else
				m_pDevCmn->fnSCR_DisplayString(1, strWelcome);
				#endif

				if( bRFIDAvailable)			
					#if(APP_CUSTOM_PAI)
						if (bButtonMode)
							m_pDevCmn->fnSCR_DisplayString(12, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101011));
						else
							m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101011));
					#else
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101011));
					#endif
				else
					#if(APP_CUSTOM_PAI)
						if (bButtonMode)
							m_pDevCmn->fnSCR_DisplayString(12, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101002));
						else
							m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101002));
					#else
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101002));
					#endif
			}
		}

		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
		{
			// [#RWC6-68] PAI Bitcoin button
			#if(APP_CUSTOM_PAI)
				if (bButtonMode)
					m_pDevCmn->fnSCR_DisplayImage(15, TRUE);
				else
					m_pDevCmn->fnSCR_DisplayImage(5, TRUE);
			#else
				m_pDevCmn->fnSCR_DisplayImage(5, TRUE);
			#endif
			// end of [#RWC6-68]
		}
		else
		{
			if (bRFIDAvailable)
			{
				// [#RWC6-68] PAI Bitcoin button
				#if(APP_CUSTOM_PAI)
					if (bButtonMode)
					{
						m_pDevCmn->fnSCR_DisplayImage(18, TRUE);
						m_pDevCmn->fnSCR_DisplayImage(20, TRUE);
					}
					else
					{	
						m_pDevCmn->fnSCR_DisplayImage(8, TRUE);
						m_pDevCmn->fnSCR_DisplayImage(10, TRUE);
					}
				#else
				{
					m_pDevCmn->fnSCR_DisplayImage(8, TRUE);
					m_pDevCmn->fnSCR_DisplayImage(10, TRUE);
				}
				#endif
				// end of [#RWC6-68]
				

				m_pDevCmn->fnRFID_EntryEnable();
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
			}
			else
			{
				// [#RWC6-68] PAI Bitcoin button
				#if(APP_CUSTOM_PAI)
				if (bButtonMode)
					m_pDevCmn->fnSCR_DisplayImage(12, TRUE);
				else
				m_pDevCmn->fnSCR_DisplayImage(2, TRUE);
				#else
				m_pDevCmn->fnSCR_DisplayImage(2, TRUE);
				#endif
				// end of [#RWC6-68]
			}
		}
		m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);
		SetMcuExisted(FALSE);
	}

	int nIndex;
	if (bRFIDAvailable)				nIndex = 5;
	else							nIndex = 3;

	if (m_pDevCmn->WithAvail == TRAN_WITH_NOT)
		m_pDevCmn->fnSCR_DisplayString(nIndex++, L"CASH_NO");

	if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
		m_pDevCmn->fnSCR_DisplayString(nIndex++, L"RECEIPT_NO");

	#if (US_VERSION)
		if( ((nScreenX == 800)||(nScreenX == 640)) && (m_pDevCmn->fnMCU_IsEmvTransaction()))			
			// [#RWC6-68] PAI Bitcoin button
			#if(APP_CUSTOM_PAI)
				if (bButtonMode)
					m_pDevCmn->fnSCR_DisplayString(15, L"EMV_ON");
				else
					m_pDevCmn->fnSCR_DisplayString(5, L"EMV_ON");
			#else
				m_pDevCmn->fnSCR_DisplayString(5, L"EMV_ON");
			#endif
			// end of [#RWC6-68]
			
	#elif (AU_VERSION)	// AU는 아직 NFC 미장착이라 ap.dat 작업이 안되어져 있음
		if(m_pDevCmn->fnMCU_IsEmvTransaction())	
			m_pDevCmn->fnSCR_DisplayString(5, L"EMV_ON");
	#endif

	return RES_OK;
}

// [#2445] US Justin 2016.09.28 Displaying Additional Function button on the Welcome Screen
BIZ_RETURN CTranCmn::SetAdditionalFunctionButton()
{
	//m_pDevCmn->fnSCR_DisplayImage(19, FALSE);
	//m_pDevCmn->fnSCR_DisplayImage(20, FALSE);
	m_pDevCmn->fnSCR_DisplayString(19, L"");
	m_pDevCmn->fnSCR_DisplayString(20, L"");

	////////////////////////////////////////////////////
	// USAGE for F8 (Additional / Cardless Programs)
	//  F8 : Text (Green Button) Or Image(logo)
	//	**** F8 BUTTON TEXT (APValue18) ********
	//	1) Green Button with Text on it    (IMAGE:20, Text:APValue18)
	// 				fnSCR_DisplayImage(20, TRUE);				// Green Button
	//				fnSCR_DisplayString(18, L"3rd Party AP");	// Button Text
	//
	//	2) LOGO  (APValue20, Button Text : APValue18)
	//				fnSCR_DisplayImage(20, FALSE);				// Turn Green Button off
	//				fnSCR_DisplayString(20, File Name);			// Image File Name
	//				fnSCR_DisplayString(18, L"   ");			// Dummy Text for activating F8
	//  COMMON - Instruction (APValue21)
	//				fnSCR_DisplayString(21, L"For Cardless transaction Press the Cardless button");

	m_pDevCmn->fnSCR_DisplayImage(1, FALSE);

	/////////////////////////////////
	///// CARDLESS/ETC SERVICES /////
	/////////////////////////////////
	int nNumEnabled, nEnabledServices;
	GetEnabledCardlessServices(&nNumEnabled, &nEnabledServices);

	// 18 = dummy for F8
	// 28 = dummy for F8 with green button

#if (APP_CUSTOM_PAI)
	if (nNumEnabled >= 1)
	{
		m_pDevCmn->fnSCR_DisplayImage(20, FALSE);
		m_pDevCmn->fnSCR_DisplayString(20, CARDLESS_LOGO_FILE);
		m_pDevCmn->fnSCR_DisplayString(18, L"  ");
	}
#else
	if (nNumEnabled > 1) // More than one application are available
	{
		//m_pDevCmn->fnSCR_DisplayImage(20, TRUE);														// Turn Green Button on
		//m_pDevCmn->fnSCR_DisplayString(18, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101013));	// Assign Text (Cardless ATM) and Activate F8
		m_pDevCmn->fnSCR_DisplayString(28, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101013));		// Assign Text (Cardless ATM) and Activate F8
		m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101014));		// Instruction : Press F8 for Cardless Transaction
	}
	else if (nNumEnabled == 1)
	{
		if (nEnabledServices == CDLS_SVC_POPMONEY)
		{
			//m_pDevCmn->fnSCR_DisplayImage(20, FALSE);													// Turn Green Button off
			m_pDevCmn->fnSCR_DisplayString(18, L"  ");													// Activate F8 Button (Blank)
			m_pDevCmn->fnSCR_DisplayString(20, POPMONEY_LOGO_FILE);
			m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101012));	// Instruction : Press F8 for Popmoney Transaction
		}
		else if (nEnabledServices == CDLS_SVC_PIN4)
		{
			// m_pDevCmn->fnSCR_DisplayImage(20, FALSE);												// Turn Green Button off
			m_pDevCmn->fnSCR_DisplayString(18, L"  ");													// Activate F8 Button (Blank)
			m_pDevCmn->fnSCR_DisplayString(20, m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_BUTTON));
			m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101015));	// Instruction : Press F8 for Pin4 Transaction
		}

	// [#2446] US Justin 2016.09.29 Paypal CCA
	#if (APP_PAYDIANT_CCA)
		else if (nEnabledServices == CDLS_SVC_PAYPALCCA)
		{
			// m_pDevCmn->fnSCR_DisplayImage(20, FALSE);													// Turn Green Button off
			m_pDevCmn->fnSCR_DisplayString(18, L"  ");														// Activate F8 Button (Blank)
			m_pDevCmn->fnSCR_DisplayString(20, m_PayPalCCA.GetPaypalCCAFileName(PAYPAL_FILE_LOGO_BUTTON));
			m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101019));		// Instruction : Press F8 for Paypal CCA Transaction
		}
	#endif
	// End of [#2446]

	#if (APP_JUST_CASH)
		else if (nEnabledServices == CDLS_SVC_JUSTCASH)
		{
			m_pDevCmn->fnSCR_DisplayString(18, L"  "); // Activate F8 Button (Blank)
			m_pDevCmn->fnSCR_DisplayString(20, m_JustCashData.GetJustCashFileName(JUSTCASH_FILE_LOGO_BUTTON));

			// [#RWC6-502] US ryan.payton 2023.01.12 Just.Cash
			BOOL cardlessEnabled = H_NH_JC_IsJustCashCardless();
			BOOL bitcoinEnabled = H_NH_JC_IsJustCashBitcoin();

			if (cardlessEnabled && !bitcoinEnabled)
				m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101028));
			else if (!cardlessEnabled && bitcoinEnabled)
				m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101027));
			else if (cardlessEnabled && bitcoinEnabled)
				m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101018));
			// End of [#RWC6-502]
		}
	#endif

	// [#RWC6-59] US William 2019.10.09 LibertyX
	#if (APP_LIBERTYX)
		else if (nEnabledServices == CDLS_SVC_LIBERTYX)
		{
			// m_pDevCmn->fnSCR_DisplayImage(20, FALSE);												// Turn Green Button off
			m_pDevCmn->fnSCR_DisplayString(18, L"  ");													// Activate F8 Button (Blank)
			m_pDevCmn->fnSCR_DisplayString(20, LIBERTYX_LOGO_FILE);
			m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101025));	// Instruction : Press F8 for Paypal CCA Transaction
		}
	#endif

	#if (APP_B4U)
		else if (nEnabledServices == CDLS_SVC_B4U)
		{
			// m_pDevCmn->fnSCR_DisplayImage(20, FALSE);												// Turn Green Button off
			m_pDevCmn->fnSCR_DisplayString(20, B4U_LOGO_FILE);
			m_pDevCmn->fnSCR_DisplayString(18, L"  ");													// Activate F6 Button (Blank)
			m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101024));	// Instruction : Press F8 for Paypal CCA Transaction
		}
	#endif

	#if (APP_DIGITALMINT)
		else if (nEnabledServices == CDLS_SVC_DIGITALMINT && m_DMService != NULL)
		{
			BOOL atmAvailable = H_NH_DM_IsAtmModeAvailable(m_DMConfig);
			BOOL sidecarAvailable = H_NH_DM_IsSidecarModeAvailable(m_DMConfig);

			if (sidecarAvailable || atmAvailable)
			{
				//m_pDevCmn->fnSCR_DisplayImage(20, FALSE);													// Turn Green Button off
				m_pDevCmn->fnSCR_DisplayString(18, L"  ");													// Activate F8 Button (Blank)
				m_pDevCmn->fnSCR_DisplayString(20, DIGITALMINT_LOGO_FILE);
				m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101026));	// Instruction : Select DigitalMint to buy bitcoin
			}
		}
	#endif
	}
#endif
	// End of [#RWC6-68]
	else
	{
		//m_pDevCmn->fnSCR_DisplayImage(20, FALSE);	// Turn Green Button off
		m_pDevCmn->fnSCR_DisplayString(20, L"");	// Turn Image off
	}

	// [#RWC6-68] PAI Bitcoin button
#if (APP_CUSTOM_PAI)
	////////////////////////////
	///// BITCOIN SERVICES /////
	////////////////////////////

	// 16 = dummy for F6
	// 26 = dummy for F6 with green button

	int nNumBCEnabled, nEnabledBitcoins;
	GetEnabledBitcoinServices(&nNumBCEnabled, &nEnabledBitcoins);

	if (nNumBCEnabled >= 1) // More than one application are available
	{
		m_pDevCmn->fnSCR_DisplayImage(19, FALSE);
		m_pDevCmn->fnSCR_DisplayString(19, BITCOIN_LOGO_FILE);
		m_pDevCmn->fnSCR_DisplayString(16, L"  ");
	}
	else
	{
		//m_pDevCmn->fnSCR_DisplayImage(19, FALSE);	// Turn Green Button off
		m_pDevCmn->fnSCR_DisplayString(19, L"");	// Turn Image off
	}
#endif
	// End of [#RWC6-68]

#if (APP_TANGOPAY)	// [RWC6-676] Start SKKim 2024.04.19
	if (P_NH_IsTangoPayAvailable() == TRUE)
	{
		m_pDevCmn->fnSCR_DisplayString(30, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_101030));
	}
#endif				// [RWC6-676] End SKKim 2024.04.19

	return RES_OK;
}
// End of [#2445]

BIZ_RETURN CTranCmn::TurnOffCardReaderFlicker(BOOL bDisableDevice)
{
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
		SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);
	else
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);

	if(bDisableDevice)
	{
		m_pDevCmn->fnAPL_DeviceEnDisable(DEV_MCU, DISABLE, TRUE);
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
		{
			m_pDevCmn->fnRFID_EntryDisable();
			m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
		}
	}
	return RES_OK;
}
// End of [#2351]

// [#2413] US Justin 2016.04.08 Voice Guidance for Popmoney and Pin4
BIZ_RETURN CTranCmn::GetEnabledCardlessServices(int* nNumEnabled, int*nEnabledServices)
{
	int nNumSvc = 0;
	int nEnSVC = CDLS_SVC_NONE;

	// Check Paydiant Service

	// PAI Additional Service
	#if (APP_POPMONEY)	// (APP_CUSTOM_PAI) [#2471] US Justin 2017.02.01 Enable Popmoney to all customers
		// POP Money
		if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_ENABLE)==ENABLE && m_pDevCmn->WithAvail != WITH_NOT_AVAILABLE)
		{
			nNumSvc++;
			nEnSVC |= CDLS_SVC_POPMONEY;
		}
	#endif
		// Pin4
	#if (APP_PIN4_CASHPICKUP)	// [#2471] US Justin 2017.02.01 Enable Popmoney to all customers
		if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE)==ENABLE)&&(m_Pin4.m_bPin4Available==TRUE) && m_pDevCmn->WithAvail != WITH_NOT_AVAILABLE)
		{
			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) != NETWORK_DIALUP)		// [#2431] US Justin 2016.06.13 Enable PAI Pin4 for TCPIP ONLY (1:Modem, 2:TCPIP or MoniAIR)
			{
				nNumSvc++;
				nEnSVC |= CDLS_SVC_PIN4;
			}
		}
	#endif

	// [#2446] US Justin 2016.09.29 Paypal CCA
	#if (APP_PAYDIANT_CCA)
		if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE)==ENABLE) && (m_PayPalCCA.m_bPaypalCCAAvailable==TRUE) && m_pDevCmn->WithAvail != WITH_NOT_AVAILABLE )
		{
			// Do not support Paypal through ADA mode. Paypal is avaiable throught standard1 message format only.
			if( (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE) && (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) != NETWORK_DIALUP) &&
				(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE) )
			{
				nNumSvc++;
				nEnSVC |= CDLS_SVC_PAYPALCCA;
			}
		}
	#endif
	// End of [#2446]

#if (!APP_CUSTOM_PAI)
	// [#2445] US Justin 2016.09.22 Just.Cash
	#if (APP_JUST_CASH)
		if (H_NH_JC_IsJustCashCardless() || H_NH_JC_IsJustCashBitcoin())
		{
			nNumSvc++;
			nEnSVC |= CDLS_SVC_JUSTCASH;
		}
	#endif
	// End of [#2445]

	#if (APP_B4U)
		if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_B4U_ENABLED) == ENABLE ) &&
			(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) != NETWORK_DIALUP) &&
			m_pDevCmn->WithAvail != WITH_NOT_AVAILABLE )
		{
				nNumSvc++;
			nEnSVC |= CDLS_SVC_B4U;
			}
	#endif

	// [#RWC6-59] US William 2019.10.09 LibertyX
	#if (APP_LIBERTYX)
		if( m_LXIsConfigured && m_LXConfig.IsEnabled() )
		{
			// Do not support Paypal through ADA mode. Paypal is avaiable throught standard1 message format only.
			if( MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) != NETWORK_DIALUP )
			{
				nNumSvc++;
				nEnSVC |= CDLS_SVC_LIBERTYX;
			}
		}
	#endif

	#if (APP_DIGITALMINT)
		if (m_DMIsConfigured && m_DMConfig.IsEnabled)
		{
			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) != NETWORK_DIALUP)
			{
				nNumSvc++;
				nEnSVC |= CDLS_SVC_DIGITALMINT;
			}
		}
	#endif
	// End of [#RWC6-59]
#endif

	*nNumEnabled = nNumSvc;
	*nEnabledServices = nEnSVC;
	return RES_OK;
}
// End of [#2413]

// [#RWC6-68] PAI Bitcoin button
BIZ_RETURN CTranCmn::GetEnabledBitcoinServices(int* nNumBCEnabled, int*nEnabledBitcoins)
{
	int nNumSvc = 0;
	int nEnSVC = CDLS_BC_NONE;

#if (APP_CUSTOM_PAI)
	#if (APP_B4U)
		if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_B4U_ENABLED) == ENABLE ) &&
			(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) != NETWORK_DIALUP) && 
			m_pDevCmn->WithAvail != WITH_NOT_AVAILABLE )
		{
			nNumSvc++;
			nEnSVC |= CDLS_BC_B4U;
		}
	#endif

	// [#RWC6-59] US William 2019.10.09 LibertyX
	#if (APP_LIBERTYX)
		if( m_LXIsConfigured && m_LXConfig.IsEnabled() )
		{
			// Do not support Paypal through ADA mode. Paypal is avaiable throught standard1 message format only.
			if( (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE) && (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) != NETWORK_DIALUP) )
			{
				nNumSvc++;
				nEnSVC |= CDLS_BC_LIBERTYX;
			}
		}
	#endif
	// End of [#RWC6-59]
#endif

	*nNumBCEnabled = nNumSvc;
	*nEnabledBitcoins = nEnSVC;
	return RES_OK;                                                                                        
}
// end of [#RWC6-68]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_PowerOffReversal()
 RETURN TYPE  : 				
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CTranCmn::BIZ_PowerOffReversal()
{
	NHDEBUG(DBG_CALL, (_T("[CTranCmn::BIZ_PowerOffReversal]\n")));

	// HOST와 통신 중 POWER OFF하여 REVERSAL 해야 하는 경우
	// STANDARD3 Protocol : STANDARD TCP/IP, WITHDRAWAL, NETFLOW 1이상
	if (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) == REVERSAL_NONE)	
	{
		// [#2362] AU KSK 2015.08.12 Power off시 reversal 처리 로직 수정
#if (AU_VERSION)	// 호주는 출금 / 조회 모두 Reversal 처리해야함
		if ((MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE) == TRANTYPE_WITHDRAWAL) ||		// 출금 거래
			(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE) == TRANTYPE_DCC)		  ||		// DCC 거래
			(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE) == TRANTYPE_INQUIRY))				// 조회 거래
#else
		if ((MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE) == TRANTYPE_WITHDRAWAL) ||		// 출금 거래
			(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE) == TRANTYPE_DCC))					// DCC 거래
#endif
		{
			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE) == STANDARD_TCPIP)
			{
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE) >= CONN_STS)
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
			}
			else
			{
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE) >= ENQIN_STS)
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
			}
		}
		// end of [#2362]
	}

#if (MX_VERSION)	// [#2137] MX KSK 2012.07.17 2nd Transaction이 아닌 경우의 Reversal Flag는 Off 시킨다.
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MULTI_CURRENCY_ENABLE) == ENABLE)
	{
		if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG) == MX_1ST_SEND)
		{
			if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_INTERNATIONAL_TYPE)
				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
		}
	}
#endif				// end of [#2137]

	// 이전 거래중 POWER OFF하여 REVERSAL FLAG가 있는 경우 REVERSAL을 시도함
	// POWER OFF 대책
	if (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) >= REVERSAL_TRAN)		// [#4] NH PSC 2008.03.14 Mac 관련 reversal 조건 변경.
	{
		if(MemGetInt(_MEM_FLD_SP_CDM,_MEM_VAR_CDMSP_FLAGENQSEND) == 1)
		{
			NVDump('F', 'E', "0R", L"97455", L"ENQFLGSET");
			//Enq 송신 Flag가 해제되어 있지 않으면(SP가 Write) 방출중 Power Off로 본다.
			// ENQ를 받으면 모두 방출하였다고 간주하여 journal만 남긴다.
			CString strMoney = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT);	// [#169] [NH] KSK 2008.04.25
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT,	strMoney);	// [#169] [NH] KSK 2008.04.25
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG,	REVERSAL_NONE);
			m_pDevCmn->m_JNLMgr.Save(NORMAL_TRX);		// [#84] NH AIREAT 2008.04.15

			// [#2477] CA Justin 2017.04.10 Leave EMV Journal
			/*
			#if (MX_VERSION)	
				// [#2115] MX KSK 2012.02.05 
				if (m_pDevCmn->fnMCU_IsEmvEnable())
					m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
				// end of [#2115]
			#elif (US_VERSION || AU_VERSION)	// [#2353] AU KSK 2015.06.28 AU EMV Data 저장 기능 추가	
				// [#2294] US JUSTIN 2014.10.07 Leave EMV Data in US VERSION
				if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
					m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
				// End of [#2294]
			#endif	
			*/
			#if (MX_VERSION)	
				if (m_pDevCmn->fnMCU_IsEmvEnable())
			#else
				if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
			#endif	
					m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
			// End of [#2477]
		}
		else
		{
			// [#12] NH KGS 2008.05.23 Incorrect Dispense 설정
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) == 0 )
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 1);
			// end of [#12]
	
			NVDump('O', 'C', "00", L"BIZ_", L"PowerOffRever");
			NHDEBUG(DBG_CALL, (_T("[CTranCmn::BIZ_OpenProc]\n")));

			//[#2185] US Justin 2013.05.14 Dual Host DCC
			m_HostConfig = HC_ATM;
			//#if (APP_CUSTOM_PAI)		// [#2499] US Justin 2017.08.21 Enable Dual Host DCC for all customers.
			if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_DUALHOSTTAN) == ON)
				m_HostConfig = HC_DUALHOST;
			//#endif
			// End of [#2185]

			// Mode_ExecReversal 함수 안에서 REVERSAL 실패해도 REVERSAL FLAG가 CLEAR됨
			m_bPowerOffReversal = TRUE;
			F_NH_NOR_Reversal();
			m_bPowerOffReversal = FALSE;
		}
#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
		// 2010.04.08 Balance 거래일 경우는 어떻게 처리 되는지 확인 필요 
		// Balance 거래일 경우는 _MEM_VAR_CDMSP_FLAGENQSEND == 0으로 세팅 되어 모두 리버설 되었다고 판단하면 되는지. 
		// KSK 2008.9.16 모두 방출일 경우에만 SURCHARGE를 SET하도록 수정
		if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE) == TRANTYPE_WITHDRAWAL)
		{
			if(MemGetInt(_MEM_FLD_SP_CDM,_MEM_VAR_CDMSP_FLAGENQSEND) == 1)
				m_pDevCmn->fnCDU_SumProc(SUM_OF_SURCHARGE);

			int nRequestedAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
			int nDispensedAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

			if ( nDispensedAmt >= nRequestedAmt || nDispensedAmt > 0)
			{
				m_pDevCmn->fnCDU_SumProc(SUM_OF_WITHDRAWAL);					
			}
		}
#else
		// [#411] [NH] KSK 2008.9.3 방출중 POWER OFF시 SUM 추가
		// 출금 / CHECK CASHING일 경우 SUM
		// KSK 2008.9.16 모두 방출일 경우에만 SURCHARGE를 SET하도록 수정
		if(MemGetInt(_MEM_FLD_SP_CDM,_MEM_VAR_CDMSP_FLAGENQSEND) == 1)
		{
			// [#2185] US Justin 2013.05.15 Dual Host DCC
			//m_pDevCmn->fnCDU_SumProc(SUM_OF_SURCHARGE);
			if(m_HostConfig == HC_DUALHOST)	m_pDevCmn->fnCDU_SumProc(SUM_OF_DHDCC_SURCHARGE);
			else						m_pDevCmn->fnCDU_SumProc(SUM_OF_SURCHARGE);
			// End of [#2185]
		}
				
//		if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE) == TRANTYPE_WITHDRAWAL)
		if( (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE) == TRANTYPE_WITHDRAWAL) ||
			(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE) == TRANTYPE_DCC) )			// [#2150] US Justin 2012.10.09 Add DCC... Need to separate DCC froM normal Withdrawal Later.
		{
			// [#2185] US Justin 2013.05.15 Dual Host DCC
			//m_pDevCmn->fnCDU_SumProc(SUM_OF_WITHDRAWAL);
			if(m_HostConfig == HC_DUALHOST)	m_pDevCmn->fnCDU_SumProc(SUM_OF_DHDCC);
			else						m_pDevCmn->fnCDU_SumProc(SUM_OF_WITHDRAWAL);
			// End of [#2185]
		}
#endif

		// [#2185] US Justin 2013.05.14 Dual Host DCC
		//#if (APP_CUSTOM_PAI)	// [#2499] US Justin 2017.08.21 Enable Dual Host DCC for all customers.
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_DUALHOSTTAN, OFF);
		//#endif
		m_HostConfig = HC_ATM;
		// End of [#2185]
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_CleanUp()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_CleanUp(BOOL clearErrors)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"CleanUp");
	m_pDevCmn->fnNET_CloseLine();

	// UPDATE SYSTEM
	m_pDevCmn->fnAPL_SetSensorInfo(TRUE);
	m_pDevCmn->fnAPL_CheckDevice();
	m_pDevCmn->fnAPL_CheckMaterial();

	// Set idle
	m_pDevCmn->TranStatus = TRAN_IDLE;

	// ERROR CLEAR
	if (!m_pDevCmn->fnAPL_CheckError() && clearErrors == TRUE)  // Clear the errors if they exist and we want them cleared
	{
		// 통신장애는 ERROR CLEAR 함
		// 단. D0307 (모뎀 장애)인 경우에는 CLEAR 하지 않음
		if ((m_pDevCmn->fstrAPL_GetErrorCode().Left(1) == L"D") &&
			(m_pDevCmn->fstrAPL_GetErrorCode() != L"D0307"))
		{
			m_pDevCmn->fnAPL_ClearError();
		}
		else if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"97301" || m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"97302")
		{
			// IC Error인 경우 Error Clear한다.
			m_pDevCmn->fnAPL_ClearError();
		}
	}

	// Remove Card Data from NVRAM
	// [#RWC6-60] US William 2019.10.18 Remove Sensitive Data
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO1DATA, L"");
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO2DATA, L"");
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO3DATA, L"");

	// Remove working keys from NVRAM
	// [#RWC6-60] US William 2019.10.18 Remove Sensitive Data
	MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1, L"");
	MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2, L"");
	MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY3, L"");
	MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY4, L"");
	MemSetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_TR31_MKB, L"");
	MemSetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_TR31_WKB, L"");

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_DisplayError()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_DisplayError()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"ShowErr");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_DisplayError]\n"));

	if (!m_pDevCmn->fnAPL_CheckError())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(2);
		{
			// Error Code and Description
			CString strTemp;
			strTemp.Format(L"%s : %s-%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_002001),
										  m_pDevCmn->fstrAPL_GetErrorProCount(),
										  m_pDevCmn->fstrAPL_GetErrorCode(1));

			m_pDevCmn->fnSCR_DisplayString(3, strTemp);
			m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrAPL_GetErrorMessage());
		}

		m_pDevCmn->fnSCR_DisplayScreen(2);
		m_pDevCmn->fstrSCR_WaitTime(2);
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_ReadCard()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_ReadCard(int nEventKind)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"ReadCard");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_ReadCard]\n"));

	BIZ_RETURN	nRes;

	// KSK 2010.04.11 Initialize variant
	m_sCardData.strISO1Data = L"";
	m_sCardData.strISO2Data = L"";
	m_sCardData.strISO3Data = L"";

	m_sCardData.strBankID = L"";
	m_sCardData.strAccountNo = L"";
	m_sCardData.strShowNumber = L"";
	// end of KSK

	CString strCardData;

	// Parse Data
#ifdef APP_AGING_MODE
	strCardData = L"393030303234333430303932343538323D303231383530313034373033313334";
#else
	// [#2325] NH KSK 2015.01.22
//	strCardData = m_pDevCmn->fstrMCU_GetCardData();
	if (nEventKind	== DEV_RFID)								// [#2375] US Justin Bug Fix		m_nEventKind => nEventKind
	{
		// Main에서 감지한 RFID Event 제거를 위한 처리 필요
		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
		strCardData = m_pDevCmn->fstrRFID_GetCardData();
		NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_ReadCard(RFID:%s)]\n", strCardData));

		// [#2378] US JUSTIN 2015.11.20 MAKE RFID DATA if it does not have proper data.
		#ifdef APP_LOCAL_MODE
			if(strCardData.GetLength()<24)
			{
				strCardData = L"393030303234333430303932343538323D303231383530313034373033313334";
				NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_ReadCard(DEMO RFID DATA:%s)]\n", strCardData));
			}
		#endif
		// End of [#2378]
	}
	else
	{
		// 기존 Card에는 없는 사항이라 일단 유지
		strCardData = m_pDevCmn->fstrMCU_GetCardData();
		NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_ReadCard(MCU:%s)]\n", strCardData));
	}
	// end of [#2325]
#endif

	nRes = LIB_CheckCardRead(strCardData);

	if (nRes != RES_OK)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		// [#2375] US Justin 2015.10.27 Add Voice Guidance
		if( IsAdaTransaction() )
		{
			// Voice Guidance....
			if( m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				return RES_USER_EXIT;
			}
			if(m_pDevCmn->fnSNS_GetEnhancedAudio())
			{
				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if(US_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFile(5,L"005.wav");
				#endif
				m_pAdaCtrl->fnExp_PlayScreenWave(5);
				m_pAdaCtrl->fnExp_WaitUntilStop();
			}
		}
		else
		{
			CString strTemp;
			m_pDevCmn->fnSCR_DisplayPrevSet(122);
			// Setting Screen
			{
				switch (nRes)
				{
				case RES_CARD_ERROR:	strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122101);	break;
				case RES_CARD_MOD_10:	strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122102);	break;
				case RES_CARD_BIN_NG:	strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122103);	break;	//[#2000] SOOK 2010.10.13 호주 사양 적용 (거래 예외 처리)
				default:				strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122101);	break;
				}

				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_1");	// [#2024] NH KSK 2011.02.24

				m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
				m_pDevCmn->fnSCR_DisplayString(4, strTemp);
				m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
			}
			m_pDevCmn->fnSCR_DisplayScreen(122);
			m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
		}
		// End of [#2375]
		NVDump('O', 'C', "00", L"P_NHNOR", L"NG_74");
		NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ReadCard]\n"));
	}
	return nRes;
}

// [#2351] US Justin 2015.06.30 Addon AP ATM Card Read...
BIZ_RETURN	CTranCmn::P_NH_NOR_ReadCardOrRFID()
{
	NVDump('O', 'C', "00", L"P_NH_AddOnAP", L"StartATMTr_ReadCard");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_ReadCardOrRFID()]\n"));

	CString			GetKeyStr;
	BOOL			bShowScreen = TRUE;
	CString			strWelcome = _T("");

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (bShowScreen == TRUE)
		{
			m_pDevCmn->fnSCR_DisplayPrevSet(301);

			SetCardReadScreenValue(strWelcome);		// Enable Device(MCR, MCR Flicker, RFID) in the routine

			m_pDevCmn->fnSCR_DisplayScreen(301, KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}

		if( (DidMediaExist()==TRUE)&&(m_pDevCmn->fnMCU_GetMaterialInfo()==ST_NOT_DETECT) )
			bShowScreen = TRUE;
		else if( (DidMediaExist()==FALSE) && (LIB_MainMenuDeviceEvent()==DEV_MCU) && (m_pDevCmn->fnMCU_GetDeviceStatus()==NORMAL) )	 
		{
			TurnOffCardReaderFlicker();
			m_nEventKind = DEV_MCU;
			return RES_ATM_READCARD_NORMAL;
		}
		else if ((LIB_MainMenuDeviceEvent() == DEV_RFID) && (m_pDevCmn->fnRFID_GetDeviceStatus() == NORMAL))
		{
			TurnOffCardReaderFlicker();
			m_nEventKind = DEV_RFID;
			return RES_ATM_READCARD_NORMAL;
		}
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				TurnOffCardReaderFlicker(TRUE);
				g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;
				return RES_ATM_READCARD_CANCEL;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				TurnOffCardReaderFlicker(TRUE);
				g_sBizFlowInfo.nReasonforCancel = RES_USER_TIMEOUT;
				return RES_ATM_READCARD_TIMEOUT;
			}
		}
		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	g_sBizFlowInfo.nReasonforCancel = RES_USER_TIMEOUT;
	return RES_ATM_READCARD_TIMEOUT;
}
// End of [#2351]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_SelectLanguage()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_SelectLanguage(int screenNumber)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"Language");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_SelectLanguage]\n"));
	
	// [#2413] US Justin 2016.04.12 VG for PIn4 and Popmoney
	if( IsAdaTransaction() )
	{
		m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);
		return RES_OK;
	}
	// End of [#2413]

	// [#2529] US Justin 2018.02.28 USE TEXT Button
	m_pDevCmn->fnSCR_SetCurrentLangMode(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE));	// [RWC6-123] Transaction Language Selection Always English
	// ENd of [#2529]

	int				i = 0;
	int				nEnabledLanguage = 0;
//	CString			strBtnName[LANGUAGE_MODE_MAX] = {};
	CString			GetKeyStr;
	BOOL			bShowScreen = TRUE;
	
	// [#2518] US Kook 2018.01.19 Support MX-2800SE
	int				nIdxOfLangFdk = 0;
	CString			strLangValueOfFdk[LANGUAGE_MODE_MAX];
	// end of [#2518]


	///////////////////////////////////
	// CHECK CONDITION

	for (i = 0; i < LANGUAGE_MODE_MAX; i++)
	{
		strLangValueOfFdk[i] = L"";
// 		strBtnName[i] = L"";

		if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ENGMODE + i) == ENABLE)
		{
// 			if (i == (ENG_MODE-1))
// 				strBtnName[i] = L"ENGLISH";
// 			else if (i == (SPN_MODE-1))
// 				strBtnName[i] = L"SPANISH";
// 			else if (i == (FRN_MODE-1))
// 				strBtnName[i] = L"FRENCH";
// 			else if (i == (CHN_MODE-1))
// 				strBtnName[i] = L"CHINESE";
// 			else if (i == (KOR_MODE-1))
// 				strBtnName[i] = L"KOREAN";
// 			else if (i == (JPN_MODE-1))
// 				strBtnName[i] = L"JAPANESE";

			nEnabledLanguage++;
		}
	}

	// check IC Language
	// [#2425] US Justin Add EMV Language Selection Option
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_LANGUAGESELECTION) != ENABLE)		// If Language Selection is disabled => Language Auto Selection.......
	{
		if (m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			int		nLen = 0;
			unsigned char szTemp[1024] = {0,};
			CString LanPrefer;
			if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_LangPrefer, &nLen, szTemp) == EMV_RSLT_OK)
			{
				LanPrefer.Format(L"%S", szTemp);
				LanPrefer.MakeLower();		// [#2226] US Justin 2013.10.22
				NHDEBUG(1, (_T("***TranBizProc_EMV***CTranCmn::P_NH_NOR_SelectLanguage() Language(%s): \n"), LanPrefer));

				for(int i=0; i<LanPrefer.GetLength(); i += 2)
				{
					// language auto selection
					if (LanPrefer.Mid(i, 2) == IC_ENGLISH && MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_ENGMODE) == ENABLE)		// OP 설정 유지로 사양 변경
					{
						m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);
						m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_ENGLISH));
						NHDEBUG(1, (_T("***TranBizProc_EMV***CTranCmn::P_NH_NOR_SelectLanguage() Select ENG \n")));
						return RES_OK;
					}
					else if (LanPrefer.Mid(i, 2) == IC_SPANISH && MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SPNMODE) == ENABLE)	// OP 설정 유지로 사양 변경
					{
						m_pDevCmn->fnSCR_SetCurrentLangMode(SPN_MODE);
						m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SPANISH));
						NHDEBUG(1, (_T("***TranBizProc_EMV***CTranCmn::P_NH_NOR_SelectLanguage() Select SPN \n")));
						return RES_OK;
					}
					else if (LanPrefer.Mid(i, 2) == IC_FRENCH && MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_FRNMODE) == ENABLE)	// OP 설정 유지로 사양 변경
					{
						m_pDevCmn->fnSCR_SetCurrentLangMode(FRN_MODE);
						m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_FRENCH));
						NHDEBUG(1, (_T("***TranBizProc_EMV***CTranCmn::P_NH_NOR_SelectLanguage() Select FRN \n")));
						return RES_OK;
					}
					// #if (AU_VERSION)   // [#2434] AU Kook 2016.08.10 Support Asian Languages for EMV Transaction // [#2442] US JUSTIN Enable EMV Asian Language selection for all countries.
					else if (LanPrefer.Mid(i, 2) == IC_CHINESE
						&& MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_CHNMODE) == ENABLE)	// OP 설정 유지로 사양 변경
					{
						m_pDevCmn->fnSCR_SetCurrentLangMode(CHN_MODE);
						m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHINESE));
						NHDEBUG(1, (_T("***TranBizProc_EMV***CTranCmn::P_NH_NOR_SelectLanguage() Select CHN \n")));
						return RES_OK;
					}
					else if (LanPrefer.Mid(i, 2) == IC_KOREAN
						&& MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_KORMODE) == ENABLE)	// OP 설정 유지로 사양 변경
					{
						m_pDevCmn->fnSCR_SetCurrentLangMode(KOR_MODE);
						m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_KOREAN));
						NHDEBUG(1, (_T("***TranBizProc_EMV***CTranCmn::P_NH_NOR_SelectLanguage() Select KOR \n")));
						return RES_OK;
					}
					else if (LanPrefer.Mid(i, 2) == IC_JAPANESE
						&& MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_JAPMODE) == ENABLE)	// OP 설정 유지로 사양 변경
					{
						m_pDevCmn->fnSCR_SetCurrentLangMode(JPN_MODE);
						m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_JAPANESE));
						NHDEBUG(1, (_T("***TranBizProc_EMV***CTranCmn::P_NH_NOR_SelectLanguage() Select JPN \n")));
						return RES_OK;
					}
					// #endif			// end of [#2434]			// [#2442] US JUSTIN Enable EMV Asian Language selection for all countries.
				}
			}
		}
	}
	// [#2425]

	// check language count
	if (nEnabledLanguage <= 1)
	{
		int	nLanguageMode = m_pDevCmn->fnSCR_GetCurrentLangMode();
		NHDEBUG(DBG_INFO, (L"ENABLE LANGUAGE COUNT (%d) - DEFAULT(%d)\n", nEnabledLanguage, nLanguageMode));

		m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
		
		switch (nLanguageMode)
		{
		case ENG_MODE:	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_ENGLISH));		break;
		case SPN_MODE:	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SPANISH));		break;
		case FRN_MODE:	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_FRENCH));		break;
		case CHN_MODE:	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHINESE));		break;
		case KOR_MODE:	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_KOREAN));		break;
		case JPN_MODE:	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_JAPANESE));	break;
		default:		m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_ENGLISH));		break;
		}
		return RES_OK;
	}

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

			m_pDevCmn->fnSCR_DisplayPrevSet(screenNumber);

			// Setting Screen
			{
				// Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_102001));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_102002));

				int	nIndex = 4;

				// [#2529] US Justin 2018.01.15 USE TEXT Button
				/*
				// [#2518] US Kook 2018.01.19 Support MX-2800SE
				if (m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_MX2800SE)
				{
					for (int i=0; i<LANGUAGE_MODE_MAX; i++)
					{
						if (MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_ENGMODE + i) == ENABLE)
						{
							strLangValueOfFdk[nIdxOfLangFdk] = GET_LANGUAGE_STRING(i+1);
							nIdxOfLangFdk++;

							m_pDevCmn->fnSCR_DisplayString(nIndex, (GET_LANGUAGE_STRING_IN_FOREIGN(i+1)));
							nIndex++;
						}
					}
				}
				else
				// end of [#2518]
				{
					// [#2186] US KMK 2013.04.11 Asian Language Support
					// 언어별 버튼 이미지 고정된 위치에 오도록 수정
					// (이미지 내부의 <<, >> 표시때문에 동적배치는 현재 못함)
					for (int i=0; i<LANGUAGE_MODE_MAX; i++)
					{
						if (MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_ENGMODE + i) == ENABLE)
							m_pDevCmn->fnSCR_DisplayString(nIndex+i, GET_LANGUAGE_STRING(i+1));
					}
					// end of [#2186]
				}
				*/
				for (int i=0; i<LANGUAGE_MODE_MAX; i++)
				{
					if (MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_ENGMODE + i) == ENABLE)
					{
						strLangValueOfFdk[nIdxOfLangFdk] = GET_LANGUAGE_STRING(i+1);

						m_pDevCmn->fnSCR_DisplayString(nIndex, (GET_LANGUAGE_STRING_IN_FOREIGN(i+1)));
					}

					nIdxOfLangFdk++;
					nIndex++;
				}
				// End of [#2529]
			}

			m_pDevCmn->fnSCR_DisplayScreen(screenNumber, KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
		}

#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
			m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_ENGLISH));
			m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);
			return RES_OK;
		}
#else
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2518] US Kook 2018.01.19 Support MX-2800SE
			// converting "F#" to "#VALUE#"
			// ENG, SPN, FRN (0, 1, 2)
			if (GetKeyStr == L"F2" || GetKeyStr == L"F4" || GetKeyStr == L"F6")
			{
				int nPressedBtn = (Asc2Int( GetKeyStr.Right(1) ) / 2) - 1;
				if( (nPressedBtn>=0) && (nPressedBtn<LANGUAGE_MODE_MAX) )
					GetKeyStr = strLangValueOfFdk[nPressedBtn];
			} // CHN, KOR, JPN (3, 4, 5)
			else if (GetKeyStr == L"F1" || GetKeyStr == L"F3" || GetKeyStr == L"F5")
			{
				int nPressedBtn = (Asc2Int( GetKeyStr.Right(1) ) / 2) + 3;
				if( (nPressedBtn>=0) && (nPressedBtn<LANGUAGE_MODE_MAX) )
					GetKeyStr = strLangValueOfFdk[nPressedBtn];
			}
			// end of [#2518]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);

				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_2");	// [#2024] NH KSK 2011.02.24

				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);

				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_3");	// [#2024] NH KSK 2011.02.24

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == L"ENGLISH")
			{
				m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_ENGLISH));
				m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);
				return RES_OK;
			}
			else if (GetKeyStr == L"SPANISH")
			{
				m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SPANISH));
				m_pDevCmn->fnSCR_SetCurrentLangMode(SPN_MODE);
				return RES_OK;
			}
			else if (GetKeyStr == L"FRENCH")
			{
				m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_FRENCH));
				m_pDevCmn->fnSCR_SetCurrentLangMode(FRN_MODE);
				return RES_OK;
			}
			else if (GetKeyStr == L"CHINESE")
			{
				m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHINESE));
				m_pDevCmn->fnSCR_SetCurrentLangMode(CHN_MODE);
				return RES_OK;
			}
			else if (GetKeyStr == L"KOREAN")
			{
				m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_KOREAN));
				m_pDevCmn->fnSCR_SetCurrentLangMode(KOR_MODE);
				return RES_OK;
			}
			else if (GetKeyStr == L"JAPANESE")
			{
				m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_JAPANESE));
				m_pDevCmn->fnSCR_SetCurrentLangMode(JPN_MODE);
				return RES_OK;
			}
		}
#endif

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);

	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_4");	// [#2024] NH KSK 2011.02.24

	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_EnterPassword()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_EnterPassword(int screenNumber)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"EnterPIN");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_EnterPassword]\n"));

	BIZ_RETURN		nRet = RES_USER_TIMEOUT;		// [#2518] US Kook 2018.01.24
	CString			GetKeyStr;
	DWORD			dwTickStart = 0, dwTickEnd = 0;
	BOOL			bShowScreen = TRUE;
	NH_SCR_CONFIG	eFrontConfig = m_pDevCmn->m_pConfig->GetScreenConfig(SCR_FRONT);
	BOOL			bFlashScreen = (eFrontConfig.eEngine == SCR_ENG_FLASH_LITE) ? (TRUE) : (FALSE);

	// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)					// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// ADA Variable
	BOOL	bStartToTimeout = FALSE;
	int		nInputCount;
	if(IsAdaTransaction())
		m_bAdaPasswordMode = TRUE;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// [#2375]

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( (IsAdaTransaction()) && (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE) )
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			m_bAdaPasswordMode = FALSE;
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_75");
			return RES_USER_EXIT;
		}
		// End of [#2375]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, KEYIN_TIME_OUT, PIN_PASSWORD_MODE, L"", PIN_PASSWORD_MIN, PIN_PASSWORD_MAX, PIN_PASSWORD_AUTO_TRUE, PIN_PASSWORD_TERM);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if (US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(103, L"103.wav"); 
				#endif
				m_pAdaCtrl->fnExp_PlayScreenWave(103);
				bStartToTimeout = TRUE;
				nInputCount = 0;
			}
			else
			{
				SetEppToPinMode();

				// [#2518] US Kook 2018.01.09
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE) == ENABLE)
				{
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ANTI_SHLDR_SURF_ENABLE) == ENABLE)
					{
#ifdef UNDER_CE
						int centerX = NH_CAM_POSITION_X_CENTER;
						int bottomY = NH_CAM_POSITION_Y_BOTTOM - 60;
						int width = NH_CAM_RESOLUTION_WIDTH;
						int height = NH_CAM_RESOLUTION_HEIGHT;

						// Change positioning for non-wyvern UI
						if (GetConfigFuncPointer()->GetOSVersion() == NH_OS_MX5200SE)
						{
							centerX = NH_CAM_NW_POSITION_X_CENTER;
							bottomY = NH_CAM_NW_POSITION_Y_BOTTOM;
							width = NH_CAM_NW_RESOLUTION_WIDTH;
							height = NH_CAM_NW_RESOLUTION_HEIGHT;
						}

						if (m_pDevCmn->fnCAM_StartAntiShoulderSurfing(centerX, bottomY, width, height) != T_OK)
						{
							NVDump('F', 'C', "93", m_pDevCmn->fstrCAM_GetErrorCode(), _T("Camera Error"));
						}
#endif // UNDER_CE
					}
				}
				// end of [#2518]

				m_pDevCmn->fnSCR_DisplayPrevSet(screenNumber);
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
					// [#2304] US Justin 2014.11.10 Display AID Label on PIN Entry Screen
					//m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103002));
					CString strTemp = _T("");
				#if (US_VERSION || CA_VERSION || MX_VERSION)
					if( m_pDevCmn->fnMCU_IsEmvTransaction() )
					{
						CString strAppName = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_EMV_APPLICATION_NAME);
						strAppName.TrimLeft();
						strAppName.TrimRight();
						if( strAppName.GetLength() > 0) 
							strTemp = strAppName;
					}
				#endif
					#if (APP_CUSTOM_PAI)
						if (m_PreBalanceStatus == PRE_BAL_ANOTHER)
							strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103010);
						else
							strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103002);
					#else
						strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103002);
					#endif
					m_pDevCmn->fnSCR_DisplayString(3, strTemp);
					// End of [#2304]

					// Guide
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103003));
				}

				dwTickStart = GetTickCount();
				// Host 송신 시 Pin Block을 호출하도록 수정 KSK 2010.07.28
				NHDEBUG(DBG_INFO, (L"CARD ACCOUNT NO : [%s]\n", m_sCardData.strAccountNo));
				//m_pDevCmn->fnSCR_DisplayScreen(screenNumber, KEYIN_TIME_OUT, PIN_PASSWORD_MODE, m_sCardData.strAccountNo, PIN_PASSWORD_MIN, PIN_PASSWORD_MAX, PIN_PASSWORD_AUTO_TRUE, PIN_PASSWORD_TERM);
				m_pDevCmn->fnSCR_DisplayScreen(screenNumber, KEYIN_TIME_OUT, 0);
			}
			// End of [#2375]
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

			//return RES_OK;
			nRet = RES_OK;		// [#2518] US Kook 2018.01.24
			break;
		}
#else

		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE))
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2375]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE = [%s]\n"), GetKeyStr));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2375]

			dwTickEnd = GetTickCount();			

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NHDEBUG(DBG_INFO, (L"ENTER PIN : CANCEL or EXIT Pressed\n"));
				// [#2375] US Justin Combine ADA and Screen Flow
				if( IsAdaTransaction())		
				{
					m_bAdaPasswordMode = FALSE;			
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay("Cancel.wav", TRUE);
					#endif
				}
				// End of [#2375]
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_5");	// [#2024] NH KSK 2011.02.24

				//return RES_USER_EXIT;
				nRet = RES_USER_EXIT;		// [#2518] US Kook 2018.01.24
				break;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_6");	// [#2024] NH KSK 2011.02.24
				//if( IsAdaTransaction())		
					m_bAdaPasswordMode = FALSE;						// [#2375] US Justin 2015.10.27

				//return RES_USER_TIMEOUT;
				nRet = RES_USER_TIMEOUT;		// [#2518] US Kook 2018.01.24
				break;
			}
			else if (GetKeyStr == L"LOAD_OK")		// Non ADA Response
			{
				NHDEBUG(DBG_INFO, (_T("PASSWORD PIN ENABLE..\n")));
				// BuildPinBlock은 Host 송/수신시에 Make하도록 로직 수정 KSK 2010.07.28
				m_pDevCmn->fnSCR_DisplayScreen(0, 0, PIN_PASSWORD_MODE, L"", PIN_PASSWORD_MIN, PIN_PASSWORD_MAX, PIN_PASSWORD_AUTO_TRUE, PIN_PASSWORD_TERM);	
			}
			else if (GetKeyStr == L"PIN")			// Non ADA Response
			{
				NHDEBUG(DBG_INFO, (L"ENTER PIN : PIN returned\n"));
				// Flash Screen일 경우 2초 Delay 검증 필요
				if (bFlashScreen == TRUE)
				{
					if ((dwTickEnd - dwTickStart) < 2000)
						m_pDevCmn->fstrSCR_WaitTime(2);
				}				
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);								// Read Complete가 올때까지 대기
	
				//m_sUserSelection.strPassword = m_pDevCmn->fstrPIN_GetPinKeyData();		// 송/수신 후에 Build Pin Block을 하도록 위치 수정
				
				//return RES_OK;
				nRet = RES_OK;		// [#2518] US Kook 2018.01.24
				break;
			}
			// [#2375] US Justin Combine ADA and Screen Flow
			else
			{
				if( IsAdaTransaction())		
				{
					if (GetKeyStr == S_CLEAR)
					{
						NHDEBUG(DBG_INFO, (L"ENTER PIN : CLEAR Pressed\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"103_1.wav", FALSE);
						#endif
						nInputCount = 0;
					}
					else if (GetKeyStr == S_ENTER)
					{
						NHDEBUG(DBG_INFO, (L"ENTER PIN : ENTER Pressed\n"));
						m_bAdaPasswordMode = FALSE;
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"ENTER.wav", TRUE);
						#endif
						m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);
						
						//return RES_OK;
						nRet = RES_OK;		// [#2518] US Kook 2018.01.24
						break;

					}
					else if (GetKeyStr.GetLength() == 1)
					{
						NHDEBUG(DBG_INFO, (L"ENTER PIN : 1 Digit Pressed\n"));

						#if (US_VERSION)
							m_pAdaCtrl->fnExp_StopPlay();
						#endif
						nInputCount++;
						if (nInputCount == 12)
							m_pDevCmn->fnSCR_SetByPassData(S_ENTER);
					}
				}
			}
			// End of [#2375]
		}
#endif
		Delay_Msg(50);
	}

	// [#2518] US Kook 2018.01.24
// 	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
// 	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_7");	// [#2024] NH KSK 2011.02.24
// 	m_bAdaPasswordMode = FALSE;						// [#2375] US Justin 2015.10.27
// 	
// 	return RES_USER_TIMEOUT;
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE) == ENABLE)
	{
#ifdef UNDER_CE
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ANTI_SHLDR_SURF_ENABLE) == ENABLE)
		{
			m_pDevCmn->fnCAM_StopAntiShoulderSurfing();
		}

		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_MEMORIZE_ENABLE) == ENABLE)
		{
			// use journal index to be used in this case. (+1)
			m_pDevCmn->fnCAM_CaptureFace(((Asc2Int(m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig)) % 9999) + 1), TIME_PIN_ENTERED);
		}
#endif // UNDER_CE
	}

	if (nRet == RES_USER_TIMEOUT)
	{
		NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
		NVDump('O', 'C', "00", L"P_NHNOR", L"NG_7");	// [#2024] NH KSK 2011.02.24
		m_bAdaPasswordMode = FALSE;						// [#2375] US Justin 2015.10.27
	}

	return nRet;
	// end of [#2518]
}

// [#2150] US Justin 2012.10.04 Dynamic Flow.. Pin Change.. 
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_InputPinChangePassword()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_InputPinChangePassword(int nType)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"EnterPIN");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_InputPinChangePassword]\n"));

	CString			GetKeyStr;
	DWORD			dwTickStart = 0, dwTickEnd = 0;
	BOOL			bShowScreen = TRUE;
	NH_SCR_CONFIG	eFrontConfig = m_pDevCmn->m_pConfig->GetScreenConfig(SCR_FRONT);
	BOOL			bFlashScreen = (eFrontConfig.eEngine == SCR_ENG_FLASH_LITE) ? (TRUE) : (FALSE);

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
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103007));

				// Sub Title
				if (nType==0)	m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103008));
				else			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103009));

				// Guide
				m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103003));
			}

			dwTickStart = GetTickCount();
			NHDEBUG(DBG_INFO, (L"CARD ACCOUNT NO : [%s]\n", m_sCardData.strAccountNo));
			m_pDevCmn->fnSCR_DisplayScreen(103, KEYIN_TIME_OUT, PIN_PASSWORD_MODE, L"", PIN_PASSWORD_MIN, PIN_PASSWORD_MAX, PIN_PASSWORD_AUTO_TRUE, PIN_PASSWORD_TERM);
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

			m_sUserSelection.strPassword = L"DCA71B5293B1230A";
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
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_5");

				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));

				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_6");

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == L"LOAD_OK")
			{
				NHDEBUG(DBG_INFO, (_T("PASSWORD PIN ENABLE..\n")));
				m_pDevCmn->fnSCR_DisplayScreen(0, 0, PIN_PASSWORD_MODE, L"", PIN_PASSWORD_MIN, PIN_PASSWORD_MAX, PIN_PASSWORD_AUTO_TRUE, PIN_PASSWORD_TERM);	
			}
			else if (GetKeyStr == L"PIN")
			{
				if (bFlashScreen == TRUE)
				{
					if ((dwTickEnd - dwTickStart) < 2000)
						m_pDevCmn->fstrSCR_WaitTime(2);
				}				
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);	// Read Complete가 올때까지 대기
				return RES_OK;
			}
		}
#endif

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_7");	// [#2024] NH KSK 2011.02.24

	return RES_USER_TIMEOUT;
}
// End of [#2150]

// [#2150] NH Justin 2012.09.27 Add Dynamic Flow Host Connection.
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_DynamicFlowConnection()
 RETURN TYPE  : - BIZ_RETURN
 PARAMETER    : - nOrder:1st or 2nd Call, sTranType=Transaction Code(001:FirstCall, 010:2ndCall Dynamic Surcharge, 011:2ndCall DCC, 020:2ndCall MPT surcharge)
 DESCRIPTION  : - Dynamic Host Connection.
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_DynamicFlowConnection(int nOrder, LPCTSTR sTranType)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"DynamicFlow");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_DynamicFlowConnection]\n"));

	int		OrgTranCode = TranCode;
	int		OrgTranStatus = m_pDevCmn->TranStatus;	// [#2362] AU KSK 2015.07.24 Tran Status 원복 코드 누락 Bug Fix

	m_sSTD1_DynamicFlowReq.TransactionType.Format(L"%s", sTranType);

	// Setting Transaction Type
	m_pDevCmn->TranResult = FALSE;
	if(nOrder==TC_DYNAMICFLOWL_1ST)
	{
		m_pDevCmn->TranStatus = TRAN_DYNAMICFLOWL_1ST;
		TranCode = TC_DYNAMICFLOWL_1ST;
	}
	else
	{
		m_pDevCmn->TranStatus = TRAN_DYNAMICFLOWL_2ND;
		TranCode = TC_DYNAMICFLOWL_2ND;
	}

	//[#2242] NH Justin 2013.12.17 DF 1st call option
	if( (nOrder==TC_DYNAMICFLOWL_1ST)&&(MemGetInt( _MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION)==DYNAMICFLOW_1STCALL_OFF) )
	{
		m_sSTD1_DynamicFlowResp.nService_DCC = 2;		// ATM will send DF 2nd Flow....
	}
	else
	{
		// [#2317] US Justin 2015.1.5 Support Domestic Bin for DCC
		//int nRes = P_NH_NOR_Transaction(TRUE);
		BOOL bProceedDCCTran = TRUE;		int nRes; 
		if( (m_sSTD1_DynamicFlowReq.TransactionType==L"001")||(m_sSTD1_DynamicFlowReq.TransactionType==L"012") )
		{
			// [#2499] US Justin 2017.08.21 Check Domestic BIN only for Dual Host DCC
			/*
			NHDEBUG(DBG_CALL, (L"DCC Tran Type = [%s], need to check Domestic BIN\n", sTranType));
			if( m_DomesticBinRange.IsCardListed(m_sCardData.strISO2Data) )
			{
				NHDEBUG(DBG_CALL, (L"Card [%s] is listed on Domestic BIN\n", m_sCardData.strISO2Data));
				bProceedDCCTran = FALSE;
			}
			else
				NHDEBUG(DBG_CALL, (L"Card [%s] is NOT listed on Domestic BIN\n", m_sCardData.strISO2Data));
			*/
			NHDEBUG(DBG_CALL, (L"DCC Tran Type = [%s], need to check Domestic BIN\n", sTranType));
			if(m_HostConfig == HC_DUALHOST)
			{
				NHDEBUG(DBG_CALL, (L"Dual Host Domestic BIN Checking\n"));
				if( m_DomesticBinRange.IsCardListed(m_sCardData.strISO2Data) )
				{
					NHDEBUG(DBG_CALL, (L"Card [%s] is listed on Domestic BIN\n", m_sCardData.strISO2Data));
					bProceedDCCTran = FALSE;
				}
			}
			NHDEBUG(DBG_CALL, (L"Card [%s], Domesit BIN Check Result = [%d]\n", m_sCardData.strISO2Data, bProceedDCCTran) );
			// End of [#2499]
		}

		// Communication with Host
		if(bProceedDCCTran==TRUE)		nRes = P_NH_NOR_Transaction(TRUE);
		else							nRes = RES_NO; 
		// End of [#2317]

		// Save Comminication result
		if(nRes == RES_OK)
		{
			if(nOrder==TC_DYNAMICFLOWL_1ST)		
			{
				m_sSTD1_DynamicFlowResp.R1_ConnectionResult = ST_OK;
				// [#2182] US Justin 2013.02.06  DCC Demo
				#ifdef APP_LOCAL_MODE
					m_sSTD1_DynamicFlowResp.nService_PINChange = 1;
					m_sSTD1_DynamicFlowResp.nService_DCC = 1;
				#endif
				// End of [#2182]
			}
			else									
				m_sSTD1_DynamicFlowResp.R2_ConnectionResult = ST_OK;
		}
	}
	// End of [#2242]

	// Clean up the communication buffer
	P_NH_NOR_CleanUp();
	TranCode = OrgTranCode;
	m_pDevCmn->TranStatus = OrgTranStatus;		// [#2362] AU KSK 2015.07.24 Tran Status 원복 코드 누락 Bug Fix
	return RES_OK;
}
// End of [#2150]


// [#2292] US Justin 2014.09.24 Implement TDL
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_TDL_Connection(int nTranType, BOOL bADAMode)
 RETURN TYPE  : - BIZ_RETURN
 PARAMETER    : 
 DESCRIPTION  : Triton TDL Connection
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_TDL_Connection(int nTranType, BOOL bADAMode)		// [#2311] US Justin 2014.11.18 Add ADA Mode TDL DYnamic Surcharge
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"TDL CON");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_TDL_Connection(%d)]\n", nTranType));

	// Store Original Transaction Code.
	int	OrgTranCode = TranCode;
	int OrgTranStatus = m_pDevCmn->TranStatus; 

	// Assign Transaction Code
	TranCode = nTranType;
	if(TranCode==TC_TDL_TIRSURCHARGE)			m_pDevCmn->TranStatus = TRAN_TDL_TIRSURCHARGE;
	else if (TranCode==TC_TDL_DCC_LOOKUP)		m_pDevCmn->TranStatus = TRAN_TDL_DCC_LOOKUP;
	else if (TranCode==TC_TDL_DCC_TRANSACTION)	m_pDevCmn->TranStatus = TRAN_TDL_DCC_TRANSACTION;
	else if (TranCode==TC_TDL_DCC_REVERSAL)		m_pDevCmn->TranStatus = TRAN_TDL_DCC_REVERSAL;
	else									return RES_NG;

	m_pDevCmn->TranResult = FALSE;

	// [#2375] US Justin 2015.10.29 Combine ADA and Screen Mode Transaction.....(US)
	#if( US_VERSION || CA_VERSION)
		BIZ_RETURN nRes = P_NH_NOR_Transaction(TRUE);
	#else
		// [#2311]  US Justin 2014.11.18 Add ADA Mode TDL Dynamic Surcharge
		//BIZ_RETURN nRes = P_NH_NOR_Transaction(TRUE);
		BIZ_RETURN nRes;
		if(bADAMode)
			nRes =P_NH_ADA_Transaction();
		else
			nRes = P_NH_NOR_Transaction(TRUE);
		// End of [#2311]
	#endif

	// Check and Store Communication result....
	bool clearErrors = TranCode == TC_TDL_DCC_LOOKUP || TranCode == TC_TDL_TIRSURCHARGE;
	P_NH_NOR_CleanUp(clearErrors);

	// Restore Original Transaction Code......
	TranCode = OrgTranCode;
	m_pDevCmn->TranStatus = OrgTranStatus;

	return nRes;
}
// End of [#2292]

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_NH_NOR_StartPreBalance()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Start Pre-Balance
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_StartPreBalance(BOOL bUSSelectAcct)	// [#2516] US Justin 2017.11.21 PAI Customization - STD3 version info / extra step for Pre Dual Balance inquiry)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"Pre-Balance");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_StartPreBalance]\n"));

	CString GetKeyStr;
	BOOL	bShowScreen = TRUE;

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
				// ICON
				m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_INFO);

				// Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_108003));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_108004));
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
			return RES_NO;
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
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_8");	// [#2024] NH KSK 2011.02.24

				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_9");	// [#2024] NH KSK 2011.02.24
				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_YES)
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_BALANCE));

				// [#2309] US Justin Balance Fee Notice
				//return RES_YES;
				#if (US_VERSION)
					// [#2516] US Justin 2017.11.21 PAI Customization - STD3 version info / extra step for Pre Dual Balance inquiry)
					/*
					BIZ_RETURN nRes = P_NH_NOR_SelectBIAccount();
					if( nRes==RES_OK)
						return RES_YES;
					else
						return nRes;
					*/
					if( bUSSelectAcct == TRUE)
					{
						BIZ_RETURN nRes = P_NH_NOR_SelectBIAccount();
						if( nRes==RES_OK)
							return RES_YES;
						else
							return nRes;
					}
					else
					{
						return RES_YES;
					}
					// End of [#2516]
				#else
					return RES_YES;
				#endif
				// End of [#2309]
			}
			else if (GetKeyStr == S_NO)
			{
				return RES_NO;
			}
		}
#endif

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_10");	// [#2024] NH KSK 2011.02.24

	return RES_USER_TIMEOUT;
}

// [#2292] US Justin 2014.10.09 Dual Balance
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_NH_NOR_StartDualBalance(bPreBal)
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Start Dual Pre-Balance
-------------------------------------------------------------------*/
// [#2387] US Justin 2016.01.14 US Cardtronics Change Dual Balance UI
/*
BIZ_RETURN CTranCmn::P_NH_NOR_StartDualBalance(BOOL bPreBal)		// [#2309] Remove Parameter	[#2340] US Justin 2015.04.17 Add Parameter Again
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"Dual Pre-Balance");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_StartDualBalance]\n"));

	CString GetKeyStr;
	BOOL	bShowScreen = TRUE;

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2340] US Justin 2015.04.17 Cardtronics Alternative Dual Balance
			//m_pDevCmn->fnSCR_DisplayPrevSet(143);
			int nScrNum = 143;
			if(bPreBal)	nScrNum = 146;
			m_pDevCmn->fnSCR_DisplayPrevSet(nScrNum);
			// End of [#2340]

			// Setting Screen
			{
				// History
				for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
				{
					if (nHistory == 0)
						m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

					m_pDevCmn->fnSCR_DisplayHistory(nHistory+1, m_arHistory[nHistory]);
				}

				// [#2309] US Justin 2014.11.14 Cardtronics Dual Balance Flow
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_BALANCE_NOFEE_NOTICE) == ENABLE)
				{
					m_pDevCmn->fnSCR_DisplayImage(7, TRUE);
					m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143010));
				}
				else
				{
					m_pDevCmn->fnSCR_DisplayImage(7, FALSE);				
				}
				// End of [#2309]

				// [#2340] US Justin 2015.04.17 Cardtronics Alternative Dual Balance
				// En/Disable ON Receipt Option
				if(bPreBal)
				{
					if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)		m_pDevCmn->fnSCR_DisplayImage(8, TRUE);
					else													m_pDevCmn->fnSCR_DisplayImage(8, FALSE);
				}
				// end of [#2340]
			}

			m_pDevCmn->fnSCR_DisplayScreen(nScrNum, KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
		}

#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			return RES_NO;
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
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_8");
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_9");
				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == L"BOTH")		// [#2309] US Justin Dual Balance Flow
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_BALANCE));
				m_DualBalance.m_bDualBalance = TRUE;
				m_DualBalance.m_nDisplaceDevice = _DUAL_BALANCE_DISPLAY_NOTSET;			// [#2340] US Justin 2015.04.17 Cardtronics Change
				return RES_YES;
			}
			else if( (GetKeyStr==L"CHECKING")||(GetKeyStr==L"SAVING") )		// [#2309] US Justin Dual Balance Flow
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_BALANCE));
				if(GetKeyStr==L"CHECKING")	m_SourceAccount = S_CHECKING;
				else						m_SourceAccount = S_SAVINGS;
				return RES_YES;
			}
			// [#2340] US Justin Dual Prebalnce - on screen. on receipt
			else if(GetKeyStr == L"ON_SCREEN")	
			{
				// [#2340] US Justin 2015.04.24
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN) == RECEIPT_ON_SCREEN_DISABLE)
					MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN, RECEIPT_ON_SCREEN_ENABLE);
				// End of [#2340]
				m_DualBalance.m_bDualBalance = TRUE;
				m_DualBalance.m_nDisplaceDevice = _DUAL_BALANCE_DISPLAY_SCREEN;
				return RES_YES;
			}
			else if(GetKeyStr == L"ON_RECEIPT")	
			{
				m_DualBalance.m_bDualBalance = TRUE;
				m_DualBalance.m_nDisplaceDevice = _DUAL_BALANCE_DISPLAY_PRINTER;
				return RES_YES;
			}
			// End of [#2340]
			else if (GetKeyStr == L"MAINMENU")
			{
				return RES_NO;
			}
		}
#endif
		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_10");
	return RES_USER_TIMEOUT;
}
// End of [#2292]
*/

// [#2309] Remove Parameter	[#2340] US Justin 2015.04.17 Add Parameter Again
BIZ_RETURN CTranCmn::P_NH_NOR_StartDualBalance(BOOL bPreBal)		
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"Dual Pre-Balance");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_StartDualBalance]\n"));

	BOOL	bShowScreen = TRUE;
	CString GetKeyStr;
	int		nScrNum;

	BOOL bContinueSelectReceipt = FALSE;
	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			nScrNum = 143;
			m_pDevCmn->fnSCR_DisplayPrevSet(nScrNum);

			// Setting Screen
			{
				// [#2444] US Justin 2016.09.01 Enable Dual Balance to All Customer
				#if( APP_TDL_OPTION )
					m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143001));
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143002));
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143003));
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143004));
					m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143005));
					m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143006));
				#elif( APP_CUSTOM_PAI )
				// [#RWC6-28] PAI Needs to Remove Dual Balance Question and Use Dual Balance Screen Directly When Enable
					m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143101));
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143102));
					m_pDevCmn->fnSCR_DisplayString(8, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143108));
					// [#RWC6-70] PAI Needs Dual Balance for Credit
					m_pDevCmn->fnSCR_DisplayString(10, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143104));	// checking
					m_pDevCmn->fnSCR_DisplayString(11, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143105));	// saving
					m_pDevCmn->fnSCR_DisplayString(12, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143107));	// credit
					// end of [#RWC6-70]
					m_pDevCmn->fnSCR_DisplayString(9, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143109));				
				// end of [#RWC6-28] 
				#elif (APP_CUSTOM_CASHDEPOT)
					m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143101));
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143102));
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143103));
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143104));
					m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143105));
					m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143110));
				#else
					m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143101));
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143102));
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143103));
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143104));
					m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143105));
					m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143106));
				#endif
				// End of [#2444]

				// Balance Fee Notice
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_BALANCE_NOFEE_NOTICE) == ENABLE)
				{
					m_pDevCmn->fnSCR_DisplayImage(7, TRUE);
					m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143010));
				}
				else
				{
					m_pDevCmn->fnSCR_DisplayImage(7, FALSE);				
				}

				// Dual Balance Button    10:Gray(Regular) button,   11:Green Button
				// [#2488] US Justin 2017.06.13 Use Green Button for PAI Dual Balance.	
				#if( APP_CUSTOM_PAI )
					m_pDevCmn->fnSCR_DisplayImage(10, TRUE);	// [#RWC6-70] PAI Needs Dual Balance for Credit
					m_pDevCmn->fnSCR_DisplayImage(11, TRUE);
				#else
					m_pDevCmn->fnSCR_DisplayImage(10, TRUE);
					m_pDevCmn->fnSCR_DisplayImage(11, FALSE);
				#endif
				// End of [#2488]
			}

			m_pDevCmn->fnSCR_DisplayScreen(nScrNum, KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
		}

#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			return RES_NO;
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
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_8");
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_9");
				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == L"BOTH")
			{
				m_DualBalance.m_bDualBalance = TRUE;
				bContinueSelectReceipt = TRUE;
				break;
			}
			else if( (GetKeyStr==L"CHECKING")||(GetKeyStr==L"SAVING")||(GetKeyStr==L"CREDITCARD") )
			{
				if(GetKeyStr==L"CHECKING")		m_SourceAccount = S_CHECKING;
				else if(GetKeyStr==L"SAVING")	m_SourceAccount = S_SAVINGS;
				else							m_SourceAccount = S_CREDITCARD;	// [#RWC6-70] PAI Needs Dual Balance for Credit
				bContinueSelectReceipt = TRUE;
				break;
			}
			else if (GetKeyStr == L"MAINMENU")
			{
				return RES_NO;
			}
		}
#endif
		Delay_Msg(50);
	}

	if( bContinueSelectReceipt != TRUE)
	{
		NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
		NVDump('O', 'C', "00", L"P_NHNOR", L"NG_10");
		return RES_USER_TIMEOUT;
	}

	bShowScreen = TRUE;

	// Selecting on Receipt or on Screen.
	NHDEBUG(DBG_CALL, (L"   Continue to select receipt or Screen\n"));
	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
			
			nScrNum = 146;
			m_pDevCmn->fnSCR_DisplayPrevSet(nScrNum);

			// Setting Screen
			{	
				// [#2444] US Justin 2016.09.01 Enable Dual Balance to All Customer
				//if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)		m_pDevCmn->fnSCR_DisplayImage(8, TRUE);
				//else													m_pDevCmn->fnSCR_DisplayImage(8, FALSE);

				//	===========   Cardtronics			   General Version  =============
				//	(APVal3)		F2 : ON SCREEN		    F2 : Display Receipt	(OLD:Paper Receipt)		// [#2488] US Justin 2017.06.12 Use Green Button for PAI Dual Balance.		
				//	(APVal4)		F4 : ON RECEIPT			F4 : Print Receipt		(OLD:No Receipt)		// [#2488] US Justin 2017.06.12 Use Green Button for PAI Dual Balance.
				//	(APVal5)		F8 : Main Menu			F8 : Main Menu
				#if( APP_TDL_OPTION )
					m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_146001));
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_146002));
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_146003));
					if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)		// Enable F4 (On Receipt)
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_146004));
					m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_146005));
				#else
					m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_146101));
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_146102));
					// [#2488] US Justin 2017.06.12 Use Green Button for PAI Dual Balance.	
					/*
					if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)		// Enable F2 (Paper Receipt)
						m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_146103));
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_146104));
					*/
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_146103));
					if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)		// Enable F4 (Print Receipt)
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_146104));
					// End of [#2488]
					
					m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_146105));
				#endif
				// End of [#2444]

				// Disable Fee Notice => Showed on 143
				m_pDevCmn->fnSCR_DisplayImage(7, FALSE);				

			}
			m_pDevCmn->fnSCR_DisplayScreen(nScrNum, KEYIN_TIME_OUT, PIN_MENU_MODE);
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
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_8");
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_9");
				return RES_USER_TIMEOUT;
			}
			else if(GetKeyStr == L"ON_SCREEN")	
			{
				// [#2488] US Justin 2017.06.12 Use Green Button for PAI Dual Balance.	
				/*
				// [#2444] US Justin 2016.09.01 Enable Dual Balance to All Customer
				// BE CAREFUL.. OPTION on General Version is reversed
				// BE CAREFUL.. OPTION on General Version is reversed
				//	===========   Cardtronics			   General Version  =============
				//	(APVal3)		F2 : ON SCREEN		    F2 : Paper Receipt
				#if( APP_TDL_OPTION )
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN) == RECEIPT_ON_SCREEN_DISABLE)
						MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN, RECEIPT_ON_SCREEN_ENABLE);

					if(m_DualBalance.m_bDualBalance==TRUE)	m_DualBalance.m_nDisplaceDevice = _DUAL_BALANCE_DISPLAY_SCREEN;
					else									m_sUserSelection.nPrintReceipt = FALSE;
				#else
					if(m_DualBalance.m_bDualBalance==TRUE)	m_DualBalance.m_nDisplaceDevice = _DUAL_BALANCE_DISPLAY_PRINTER;
					else									m_sUserSelection.nPrintReceipt = RCPT_PAPER;		
				#endif
				// End of [#2444]
				*/

				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN) == RECEIPT_ON_SCREEN_DISABLE)
					MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN, RECEIPT_ON_SCREEN_ENABLE);
				
				if(m_DualBalance.m_bDualBalance==TRUE)	m_DualBalance.m_nDisplaceDevice = _DUAL_BALANCE_DISPLAY_SCREEN;
				else									m_sUserSelection.nPrintReceipt = FALSE;
				// End of [#2488]

				return RES_YES;
			}
			else if(GetKeyStr == L"ON_RECEIPT")	
			{
				// [#2488] US Justin 2017.06.12 Use Green Button for PAI Dual Balance.	
				/*
				// [#2444] US Justin 2016.09.01 Enable Dual Balance to All Customer
				// BE CAREFUL.. OPTION on General Version is reversed
				// BE CAREFUL.. OPTION on General Version is reversed
				//	===========   Cardtronics			   General Version  =============
				//	(APVal4)		F4 : ON RECEIPT			F4 : No Receipt
				#if( APP_TDL_OPTION )
					if(m_DualBalance.m_bDualBalance==TRUE)	m_DualBalance.m_nDisplaceDevice = _DUAL_BALANCE_DISPLAY_PRINTER;
					else									m_sUserSelection.nPrintReceipt = RCPT_PAPER;		
				#else
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN) == RECEIPT_ON_SCREEN_DISABLE)
						MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN, RECEIPT_ON_SCREEN_ENABLE);

					if(m_DualBalance.m_bDualBalance==TRUE)	m_DualBalance.m_nDisplaceDevice = _DUAL_BALANCE_DISPLAY_SCREEN;
					else									m_sUserSelection.nPrintReceipt = FALSE;
				#endif
				*/			
				// End of [#2444]

				if(m_DualBalance.m_bDualBalance==TRUE)	m_DualBalance.m_nDisplaceDevice = _DUAL_BALANCE_DISPLAY_PRINTER;
				else									m_sUserSelection.nPrintReceipt = RCPT_PAPER;		
				// End of [#2488]

				return RES_YES;
			}
			else if (GetKeyStr == L"MAINMENU")
			{
				m_DualBalance.m_bDualBalance = FALSE;				// [#2432] US Justin 2016.06.15 Dual Balance Print Bug Fix
				return RES_NO;
			}
		}
		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_10");
	return RES_USER_TIMEOUT;
}
// End of [#2387]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_SelectTransaction()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Select Transaction
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_SelectTransaction()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"SelectTran");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_SelectTransaction]\n"));

	int				nIndex;
	CString			GetKeyStr;
	BOOL			bShowScreen = TRUE;
	int				nCharitySelection = 3;			// [#2289] US Justin 2014.08.18 Add Charity

	// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// ADA Variable
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CStringArray arrVG;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// [#2375]

	// [#2518] US Kook 2018.01.14 Support MX-2800SE
	int		nIdxOfRightFdk = 0;
	CString	strValueOfRightFdk[3] = { _T(""), _T(""), _T("") };	// F2, F4, F6
	// end of [#2518]

	// [#2496] Justin 2017.08.14 Just.Cash
	CString strAdditionalFtn[4];
	strAdditionalFtn[0] = strAdditionalFtn[1] = strAdditionalFtn[2] = strAdditionalFtn[3] = _T("");
	// End of [#2496]

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_78");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2375]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if (US_VERSION)
					if (m_pDevCmn->WithAvail != TRAN_WITH_NOT)
					{
						m_pAdaCtrl->fnExp_AddWaveFile(105, L"For.wav");	
						m_pAdaCtrl->fnExp_AddWaveFile(105, L"Withdrawal.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(105, L"Press2.wav");
					}
					m_pAdaCtrl->fnExp_AddWaveFile(105, L"For.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(105, L"AccountBalance.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(105, L"Press4.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(105, L"For.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(105, L"Transfer.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(105, L"Press6.wav");

					m_pAdaCtrl->fnExp_AddWaveFile(105, L"LocateCancelKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(105, L"LocationRepeatKey.wav");				
				#else
					if (m_pDevCmn->WithAvail != TRAN_WITH_NOT)
					{
						NHDEBUG(DBG_INFO, (_T("WITHDRAWAL NOT AVAILABLE\n")));
						m_pAdaCtrl->fnExp_AddWaveFile(105, L"Press2ForWith.wav");
					}
					m_pAdaCtrl->fnExp_AddWaveFile(105, L"Press4ForInq.wav");
				#endif
				m_pAdaCtrl->fnExp_PlayScreenWave(105);
				bStartToTimeout = TRUE;
			}
			else
			{
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
					nIdxOfRightFdk = 0;

					// 1. Check Withdrawal Status
					// [#2529] US Justin 2018.02.15 Remove button Image
					/*
					// [#2518] US Kook 2018.01.16 Support MX-2800SE, Dynamic FDKs for new screen.
					if (m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_MX2800SE)
					{
						if (m_pDevCmn->WithAvail != TRAN_WITH_NOT)
						{
							m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_WITHDRAWAL));
							strValueOfRightFdk[nIdxOfRightFdk] = "WITHDRAWAL";
							nIdxOfRightFdk++;
						}

						m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_BALANCE));
						strValueOfRightFdk[nIdxOfRightFdk] = "BALANCE";
						nIdxOfRightFdk++;

						m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_TRANSFER));
						strValueOfRightFdk[nIdxOfRightFdk] = "TRANSFER";
						nIdxOfRightFdk++;

// 						// CANCEL button ('EXIT' would be better?)
// 						m_pDevCmn->fnSCR_DisplayString(11, m_pDevCmn->fstrSCR_GetStringFromTextID(_T("APTextID:TC_CANCEL")));
					}
					else
					// end of [#2518]
					{
						if (m_pDevCmn->WithAvail != TRAN_WITH_NOT)
							m_pDevCmn->fnSCR_DisplayString(nIndex++, L"WITHDRAWAL");
						m_pDevCmn->fnSCR_DisplayString(nIndex++, L"BALANCE");
						m_pDevCmn->fnSCR_DisplayString(nIndex++, L"TRANSFER");
					}
					*/

					if (m_pDevCmn->WithAvail != TRAN_WITH_NOT)
					{
						m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_WITHDRAWAL));
						strValueOfRightFdk[nIdxOfRightFdk] = _T("WITHDRAWAL");
						nIdxOfRightFdk++;
					}

					m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_BALANCE));
					strValueOfRightFdk[nIdxOfRightFdk] = _T("BALANCE");
					nIdxOfRightFdk++;

					#if !(MX_VERSION)
					m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_TRANSFER));
					strValueOfRightFdk[nIdxOfRightFdk] = _T("TRANSFER");
					nIdxOfRightFdk++;
					#endif
					// End of [#2529]

					int nAdditionalFtn = 0;
					int nLoc = 0;

					//  PIN CHANGE : Gray Button + TEXT
					#if (PIN_CHANGE)
						if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)&&(m_pDevCmn->fnMCU_IsEmvTransaction()==FALSE) )
						{
							int nPinOpt = MemGetInt( _MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE);
							if( (nPinOpt==1) || (  (nPinOpt==2)&&((m_sSTD1_DynamicFlowResp.nService_PINChange==1) ) ) )
							{
								nLoc = nAdditionalFtn*2 + 1;
								m_pDevCmn->fnSCR_DisplayImage( nLoc,      TRUE);													// Gray Default Button
								m_pDevCmn->fnSCR_DisplayImage( nLoc + 10, FALSE);													// Green Button
								m_pDevCmn->fnSCR_DisplayString(nLoc + 10, L"");														// Graphic 
								m_pDevCmn->fnSCR_DisplayString(nLoc + 20, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_105003));	// Text
								strAdditionalFtn[nAdditionalFtn] = L"PINCHANGE";
								nAdditionalFtn++;
							}
						}
					#endif

// [#2496] Justin 2017.08.14 Just.Cash
// JUST.CASH BITCOIN : Green Button + TEXT
#if (APP_JUST_CASH)
					if (H_NH_JC_IsJustCashBitcoin())
					{
						nLoc = nAdditionalFtn * 2 + 1;

						m_pDevCmn->fnSCR_DisplayImage(nLoc, FALSE);															// Gray Default Button
						m_pDevCmn->fnSCR_DisplayImage(nLoc + 10, TRUE);														// Green Button
						m_pDevCmn->fnSCR_DisplayString(nLoc + 10, L"");														// Graphic
						m_pDevCmn->fnSCR_DisplayString(nLoc + 20, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_105004));	// Text

						strAdditionalFtn[nAdditionalFtn] = L"JUSTCASH";
						nAdditionalFtn++;
					}
#endif
// End of [#2496]

					// GivePay : Gray Button + Image
					#if (APP_GPAY_GIFTCARD_PURCHASE)
						if( m_GivePayData.CanMakeGPTransaction(FALSE) )
						{
							nLoc = nAdditionalFtn*2 + 1;
							m_pDevCmn->fnSCR_DisplayImage( nLoc,      TRUE);															// Gray Default Button
							m_pDevCmn->fnSCR_DisplayImage( nLoc + 10, FALSE);															// Green Button
							m_pDevCmn->fnSCR_DisplayString(nLoc + 10, m_GivePayData.GetGPImageFileName(GPAY_IMG_LOGO));					// Graphic 
							m_pDevCmn->fnSCR_DisplayString(nLoc + 20, "  "); // Assign Blank to activate Function Key					// Text
							strAdditionalFtn[nAdditionalFtn] = L"GIVEPAY";
							nAdditionalFtn++;
						}
					#endif

					// Hide unused additional function keys
					if( nAdditionalFtn < 4 )
					{
						for(int i=nAdditionalFtn; i<4; i++)
						{
							nLoc = i*2 + 1;																								// F1, F3, F5, F7
							m_pDevCmn->fnSCR_DisplayImage( nLoc,      FALSE);															// Gray Default Button (State 1,3,5,7)
							m_pDevCmn->fnSCR_DisplayImage( nLoc + 10, FALSE);															// Green Button (State 11,13,15,17)
							m_pDevCmn->fnSCR_DisplayString(nLoc + 10, L"");																// Button Image (APValue 11,13,15,17)		[#2574] US Justin GivePay Enhancement3						
						}
					}
					// End of [#2557]
				}
				m_pDevCmn->fnSCR_DisplayScreen(105, KEYIN_TIME_OUT, PIN_MENU_MODE);		// [#158] KSK 2008.04.22
			}
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

		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2375]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE = [%s]\n"), GetKeyStr));

			// [#2518] US Kook 2018.01.17 Support MX-2800SE
			// CW, TR, BI
			if (GetKeyStr == L"F2" || GetKeyStr == L"F4" || GetKeyStr == L"F6")
			{
				int nPressedBtn = (Asc2Int( GetKeyStr.Right(1) ) / 2) - 1;		// 2,4,6 => 0,1,2
				if( (nPressedBtn>=0) && (nPressedBtn<3) )
					GetKeyStr = strValueOfRightFdk[nPressedBtn];
			}
			// end of [#2518]

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2375]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NHDEBUG(DBG_INFO, (L"ENTER PIN : CANCEL or EXIT Pressed\n"));
				// [#2375] US Justin Combine ADA and Screen Flow
				if( IsAdaTransaction())		
				{
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2375]
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_11");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_12");	// [#2024] NH KSK 2011.02.24
				return RES_USER_TIMEOUT;
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
			else if (GetKeyStr == L"TRANSFER")
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_TRANSFER));
				TranCode = TC_TRANSFER;
				return RES_OK;
			}
			else if( (GetKeyStr == L"F1")||(GetKeyStr == L"F3")||(GetKeyStr == L"F5")||(GetKeyStr == L"F7") )
			{
				int nPressedBtn = (int) (0.5*Asc2Int(GetKeyStr.Right(1)) );
				CString sPressedFunction = strAdditionalFtn[nPressedBtn];

				if (sPressedFunction == L"PINCHANGE")
				{
					TranCode = TC_PINCHANGE;

					return RES_OK;
				}
				else if (sPressedFunction == L"JUSTCASH")
				{
					m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_JC_BITCOIN));
					TranCode = TC_JUSTCASH_BITCOIN;

					return RES_OK;
				}
				else if (sPressedFunction == L"GIVEPAY")
				{
					m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_GIVEPAY));
					TranCode = TC_GP_BUY_CARD;

					return RES_OK;
				}
			}	// End of [#2557]
			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			/////////////////////////////////// VOICE GUIDANCE MODE ///////////////////////////////
			else
			{
				if( IsAdaTransaction() )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));
					arrVG.RemoveAll();

					int nValidADAInput = 0;
					if( (GetKeyStr == L"2") && (m_pDevCmn->WithAvail != TRAN_WITH_NOT) ) // Withdrawal
					{
						arrVG.Add(L"2.wav");		arrVG.Add(L"Withdrawal.wav");
						TranCode = TC_WITHDRAWAL;
						nValidADAInput = 1;
					}
					else if (GetKeyStr == L"4")				// Balance Inquiry
					{
						arrVG.Add(L"4.wav");		arrVG.Add(L"AccountBalance.wav");
						TranCode = TC_INQUIRY;
						nValidADAInput = 1;
					}
					#if (US_VERSION)
					else if (GetKeyStr == L"6")				// Transfer => Only US
					{
						arrVG.Add(L"6.wav");		arrVG.Add(L"Transfer.wav");
						TranCode = TC_TRANSFER;
						nValidADAInput = 1;
					}
					#endif
					else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
					{
						nValidADAInput = 2;
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
					}
										
					if	( nValidADAInput == 0 )			// Invalid
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
							bInvalidRepeat = TRUE;
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						#endif
					}
					else if( nValidADAInput == 1)			// Valid Input
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(arrVG.GetAt(0), TRUE, arrVG.GetAt(1));
						#endif
						return RES_OK;
					}
					else if( nValidADAInput == 2)			// Repeat or Volume Control
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
				}
			}
			// End of [#2375]
		}
#endif

		Delay_Msg(50);
		
		// [#2375] US Justin 2015.11.03 US Justin US ADA
		#if(US_VERSION)
			if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
				if( P_NH_ADA_NeedMoreTime() == RES_OK )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
				else
					return RES_USER_EXIT;
			}
		#endif
		// End of [#2375]
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_13");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

// [#2496] US Justin 2017.08.18 
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_CalculateAndShowWithDrawalSurcharge()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Calculate Withdrawal Surcharge and show....
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_CalculateAndShowWithDrawalSurcharge()
{
	BIZ_RETURN	nRes;
	///////////////////////////////////////////////////////////////////////////////
	// DCC Offering is available only for US VERSION (S/W TTS) Other Countries  : Disable DCC for VG mode.... No Wave file available (English and French)
	BOOL bADAVGMode = FALSE;
	if( IsAdaTransaction() )
		bADAVGMode = TRUE;
    NVDump('O', 'C', "00", L"BIZ_", L"CalcShowWDSur");  // MX DCC
	BOOL bIsDCC2ndCall = FALSE;			// DCC Inquiry Call or CW Dynamic Surchage Inquiry call
	BOOL bDFSurcharge = FALSE;			// Using a Surcharge which was downloaded by Dnyamic Flow.
	BOOL bTerminate = FALSE;	

	// Standard1 DCC or Dual Host DCC
	if( ( (m_sSTD1_DynamicFlowResp.nService_DCC )&&(MemGetInt( _MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC)==ENABLE) ) || (m_HostConfig == HC_DUALHOST) )	// [#2185] US Justin 2013.06.05 Dual Host Bug Fix
	{
		#ifdef APP_LOCAL_MODE
			//[#2182] US Justin 2013.02.06 DCC Demo
			m_sSTD1_DynamicFlowResp.R2_ConnectionResult = ST_OK;
			#if (US_VERSION)
				m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency = _T("USD/2/CAD/2");
			#elif(CA_VERSION)
				m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency = _T("CAD/2/USD/2");
			#endif	
			float fERate = 1.0349;
			m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate.Format(L"%0.2f", fERate);
			m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount.Format(L"%012d", (int)(fERate*(Asc2Int(m_sUserSelection.strMoney))) );
			m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge.Format(L"%08d", (int)(fERate*200) );
			m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC = _T("00000200");
			bIsDCC2ndCall = TRUE;
			// End of [#2182]
		#else
			// [#2242] NH Justin 2013.12.17 DF 1st call option
			if(m_sSTD1_DynamicFlowResp.nService_DCC == 2)
			{
				P_NH_NOR_DynamicFlowConnection(TC_DYNAMICFLOWL_2ND, _T("012"));
				// [#2349] US Justin 2015.06.05 DCC LOOKUP Local Surcharge
				if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE)==ENABLE)&&
					(m_sSTD1_DynamicFlowResp.R2_ConnectionResult == ST_OK)&&
					(m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.GetLength() == 11)&&
					(m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate.GetLength() > 0 ) )
				{
					m_sSTD1_DynamicFlowResp.R1_Surcharge_DCC = L"--------";
					P_NH_NOR_DFDCC_Convert2HomeCurrency();
					bIsDCC2ndCall = TRUE;
				}
				// End of [#2349]
				else if( (m_sSTD1_DynamicFlowResp.R2_ConnectionResult == ST_OK)&&
					(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount.GetLength() == 12)&&
					(m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.GetLength() == 11)&&
					(m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate.GetLength() > 0 )&&
					(Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount) > 0)	)
				{
					bIsDCC2ndCall = TRUE;
				}
			}
			else
			{
				BOOL bSkip2ndCall = FALSE;
				if( (m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRate.GetLength()>0)&&(m_sSTD1_DynamicFlowResp.R1_DCC_BaseTargetCurrency.GetLength()==11) )
				{
					if( m_sSTD1_DynamicFlowResp.R1_Surcharge_DCC != L"        ") 
					{
						// Calculate DCC result instead of asking to Host.....
						bIsDCC2ndCall = TRUE;
						bSkip2ndCall = TRUE;

						// Copying from 1st Call Response
						m_sSTD1_DynamicFlowResp.R2_ConnectionResult = ST_OK;
						m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency = m_sSTD1_DynamicFlowResp.R1_DCC_BaseTargetCurrency;
						m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate = m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRate;
					
						P_NH_NOR_DFDCC_Convert2HomeCurrency();		// [#2330] US Justin 2015.02.10 Percentage DCC Surcharge
					}
				}
				if(!bSkip2ndCall)
				{
					bIsDCC2ndCall = TRUE;
					P_NH_NOR_DynamicFlowConnection(TC_DYNAMICFLOWL_2ND, _T("011"));
					// [#2349] US Justin 2015.06.05 DCC LOOKUP Local Surcharge
					if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE)==ENABLE)&&
						(m_sSTD1_DynamicFlowResp.R2_ConnectionResult == ST_OK)&&
						(m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.GetLength() == 11)&&
						(m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate.GetLength() > 0 ) )
					{
						m_sSTD1_DynamicFlowResp.R1_Surcharge_DCC = L"--------";
						P_NH_NOR_DFDCC_Convert2HomeCurrency();
					}
					// End of [#2349]
				}
			}
			// End of [#2240]
		#endif
	}
	// Checking Dynamic Surcharge (Standard1 first call)....NON DCC Cashwithdrawal.........
	else if( m_sSTD1_DynamicFlowResp.R1_ConnectionResult==ST_OK )
	{
		if( m_sSTD1_DynamicFlowResp.R1_Surcharge_Withdrawal.GetLength() == 8 )
		{
			if( m_sSTD1_DynamicFlowResp.R1_Surcharge_Withdrawal == L"        ")		// All Spaces on the 1st call => Need to call again.
			{
				P_NH_NOR_DynamicFlowConnection(TC_DYNAMICFLOWL_2ND, _T("010"));
			}
			else if( Asc2Int(m_sSTD1_DynamicFlowResp.R1_Surcharge_Withdrawal) > 0 )	// Surcharge from 1st Call
			{
				bDFSurcharge = TRUE;
				P_NH_NOR_DFDCC_Convert2HomeCurrency();

				if ((nRes = P_NH_NOR_DisplaySurcharge(SURCHARGE_DISP_DYNAMIC1ST)) != RES_OK)		// Dynamic Surcharge..... fc of the first call.....
				{
					bTerminate = TRUE;
				}
			}
			else
			{
				bDFSurcharge = TRUE;		// Surcharge = 0, Not showing....
			}
		}
		// else => ATM did not get fc field from the first call (DCC disabled) => Normal Surcharge Flow (Existing....)
	}
	// Standard3 TDL - Surcharge Segmentation or DCC
	else if( MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE )
	{
		// If TDL Surcharge Segmentation is enabled
		if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_TIRSURCHARGE)==ENABLE) && (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DCC)!=ENABLE) )
		{
			if( (m_STD3_TDL_Data.m_nTranType==_TDL_TRAN_DYNAMICSURCHARGE)&&(m_STD3_TDL_Data.m_bTranResult==TRUE) )
			{
				bDFSurcharge = TRUE;
				if( Asc2Int(m_STD3_TDL_Data.m_strSurchargeAmount) > 0 )
				{
					if ((nRes = P_NH_NOR_DisplaySurcharge(SURCHARGE_DISP_TDL_SEGMENTATION)) != RES_OK)
						bTerminate = TRUE;
				}
			}
		}
		// if TDL DCC is enabled.
		else if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DCC)==ENABLE)
		{
			BIZ_RETURN nResTDL = P_NH_NOR_TDL_Connection(TC_TDL_DCC_LOOKUP);
			if( (nResTDL==RES_OK)&&(m_STD3_TDL_Data.m_bTranResult==TRUE) )
			{
				CString sTDLSurchargeAmt = m_STD3_TDL_Data.UpdateSurchargeFromDCCLookup();

				// DCC Offering is available only for US VERSION (S/W TTS) Other Countries  : Disable DCC for VG mode.... No Wave file available (English and French)
				///////////////////////////////////////////////////////////////////////////////
				#if(!US_VERSION)
				if(bADAVGMode)
					m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCEligible = L"0";				// Set here otherwise ATM will skip "surcharge notice"
				#endif
				///////////////////////////////////////////////////////////////////////////////
				// DCC Offering is available only for US VERSION (S/W TTS) Other Countries  : Disable DCC for VG mode.... No Wave file available (English and French)

				if(sTDLSurchargeAmt.GetLength() == 8)										// Host assigned amount or "0" Surcharge
				{
					bDFSurcharge = TRUE;
					if( Asc2Int(sTDLSurchargeAmt) > 0 )
					{
						int nTDL_SurchargeType = SURCHARGE_DISP_TDL_SEGMENTATION;
						if(m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCEligible == L"1")		// DCC	: NO CREDIT FEE
							nTDL_SurchargeType = SURCHARGE_DISP_TDL_DCC;

						if ((nRes = P_NH_NOR_DisplaySurcharge(nTDL_SurchargeType)) != RES_OK)
							bTerminate = TRUE;
					}
				}

				if( m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCEligible == L"1")	
					m_STD3_TDL_Data.m_bProceedDCC = TRUE;
			}
			// [#2296] US Justin 2014.10.17 Cardtronics request.. Cancelling transaction when TIR request is failed.
			else
			{
				nRes = RES_USER_EXIT;
				bTerminate = TRUE;
			}
			// End of [#2296]
		}
	}

	// Check Standard1 2nd Dynamic Flow result....
	if( (bTerminate==FALSE)&&(m_sSTD1_DynamicFlowResp.R2_ConnectionResult==ST_OK) )
	{
		// Standard1  Suscharge Segmentation
		if( (!bIsDCC2ndCall)&&(m_sSTD1_DynamicFlowResp.R2_Surcharge_Withdrawal.GetLength() == 8) )			// Normal Withdrawal (fc Exist) : Get a response for "010" request	// [#2182] NH Justin 2013.03.21 Implement NHD1.6
		{
			// [#2361] US Justin... 2015.07.17 CDS Local Surcharge SPEC....
			#if(US_VERSION)
			if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION)!=DCC_CUSTOMOPTION_CDS)|| 	
				(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE)!=ENABLE) )	
			#endif
			// End of [#2361]
			{	 
				bDFSurcharge = TRUE;
				if( Asc2Int(m_sSTD1_DynamicFlowResp.R2_Surcharge_Withdrawal) > 0 )							// Surcharge from 2nd Call
				{
					if ((nRes = P_NH_NOR_DisplaySurcharge(SURCHARGE_DISP_DYNAMIC2ND)) != RES_OK)			// Dynamic Surcharge..... fc of the second call.....
						bTerminate = TRUE;
				}
				// else => ZERO Surcharge ..  Not displaying
			}
		}
		// Standard1  DCC
		else if(	(bIsDCC2ndCall)&&																		// DCC : Get a response for "011" request	// [#2182] NH Justin 2013.03.21 Implement NHD1.6 // [#2185] US Justin 2013.06.05 Dual Host Bug Fix
					(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount.GetLength() == 12)&&
					(m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.GetLength() == 11)&&
					(m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate.GetLength() > 0 )&&
					(Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount) > 0)			)	// DCC Available
		{
			// Checking Currency ID and converted Transaction Amount
			bDFSurcharge = TRUE;
			if( Asc2Int(m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC) > 0 )	
			{
				if ((nRes = P_NH_NOR_DisplaySurcharge(SURCHARGE_DISP_DYNAMICDCC)) != RES_OK)	// DCC FEE
					bTerminate = TRUE;
			}
			// else => ZERO Surcharge ..  Not displaying

			#if (APP_CUSTOM_PAI)
			if ( (m_sCardData.strISO2Data.Left(1).CompareNoCase(L"5") == 0) || (m_sCardData.strISO2Data.Left(1).CompareNoCase(L"6") == 0) ) // PAI, MasterCard starting with 5 or 6
			{
				//if (m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCDisclaimerScheme==L"1")
				//	m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCDisclaimerScheme = L"2";
				//if (m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCMarkupRate.GetLength() > 0)
				//	m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCMarkupRate.Format(L"");
				if (m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRateMarkUp.GetLength() > 0)
					m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRateMarkUp.Format(L"");
				if (m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRateMarkUp.GetLength() > 0)
					m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRateMarkUp.Format(L"");
			}
			#endif

			// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
			BOOL	bIsVisaDCC = FALSE;
			if ( ((m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCDisclaimerScheme==L"1") &&
				  (m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCMarkupRate.GetLength() > 0)) ||
				 (m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRateMarkUp.GetLength() > 0) ||
				 (m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRateMarkUp.GetLength() > 0) )
				bIsVisaDCC = TRUE;

			m_bDCCDeclinedByOperator = FALSE;
			// end of [#RWC6-2, #2585]

			if( bTerminate==FALSE)
			{
				// DCC Offering is available only for US VERSION (S/W TTS) Other Countries  : Disable DCC for VG mode.... No Wave file available (English and French)
				///////////////////////////////////////////////////////////////////////////////
				#if( (US_VERSION)||(CA_VERSION) )	// [#RWC6-12] Denomination Selection Support
					// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
					if (  
						// DynamicFlow DCC declined by operator
						  ((MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_HYOSUNG_TYPE) &&	
						   (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC) == ENABLE) && 
						   (((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE) == DYNAMICFLOW_DCC_TYPE_VISAONLY) && (bIsVisaDCC == FALSE)) ||
							((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE) == DYNAMICFLOW_DCC_TYPE_MASTERONLY) && (bIsVisaDCC == TRUE))
							)
						  ) ||	// DualHost DCC declined by operator
						  ((MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_HYOSUNG_TYPE) &&	
						   (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) == ENABLE) && 
						   (((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOST_DCC_TYPE) == DUALHOST_DCC_TYPE_VISAONLY) && (bIsVisaDCC == FALSE)) ||
						    ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOST_DCC_TYPE) == DUALHOST_DCC_TYPE_MASTERONLY) && (bIsVisaDCC == TRUE))
						   )
						  )
						  // TODO need to add PAI BIN list filter here
					   )
					{
						m_bDCCDeclinedByOperator = TRUE;
						nRes = RES_DCC_USE_LOCALCURRENCY;
					}
					else	
					// end of [#RWC6-2, #2585]
						// [#RWC6-13] PAI has requested changes to Dual Host DCC offering 
						#if (APP_CUSTOM_PAI)
						nRes = P_NH_NOR_DisplayDCCOption(DCC_CUSTOMOPTION_PLANET);	
						// end of [#RWC6-13]
						#else
						nRes = P_NH_NOR_DisplayDCCOption();				// Withdrawal Option (Normal WD / DCC WD ?)
						#endif					
				#else
					if(bADAVGMode)
						nRes = RES_DCC_USE_LOCALCURRENCY;			// ADA ==> LOCAL CURRENCY ONLY
					else
					{
						nRes = P_NH_NOR_DisplayDCCOption();			// Withdrawal Option (Normal WD / DCC WD ?)
					}
				#endif
				///////////////////////////////////////////////////////////////////////////////
				// DCC Offering is available only for US VERSION (S/W TTS) Other Countries  : Disable DCC for VG mode.... No Wave file available (English and French)

				if(nRes==RES_DCC_USE_LOCALCURRENCY)			// Withdrawal without DCC conversion
					m_sSTD1_DynamicFlowResp.nDCCTransaction = 2;								// DCC Type 0:non DCC, 1:DCC Tr, 2:DCC Denial
				else if(nRes==RES_DCC_USE_HOMECURRENCY)		// Withdrawal with DCC conversion
					m_sSTD1_DynamicFlowResp.nDCCTransaction = 1;
				else
					bTerminate = TRUE;
			}
		}
		// else  => Normal Surcharge Flow (Existing....)
	}

	// Dynamic Surcharge was not presented... USE Existing Surcharge....
	if( (bTerminate==FALSE)&&(bDFSurcharge==FALSE) )
	{
		if (LIB_CheckSurchargeDisplay() == TRUE)
		{
			if ((MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) == PERCENT_MODE) )
			{
				if ((nRes = P_NH_NOR_DisplaySurcharge(SURCHARGE_DISP_PRECENT)) != RES_OK)
					bTerminate = TRUE;
			}
			else if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) == TABLE_MODE)
			{
				if ((nRes = P_NH_NOR_DisplaySurcharge(SURCHARGE_DISP_TABLE)) != RES_OK)
					bTerminate = TRUE;
			}
			else
			{
				if ((nRes = P_NH_NOR_DisplaySurcharge(SURCHARGE_DISP_NORMAL)) != RES_OK)
					bTerminate = TRUE;	
			}
		}
	}

	// Stanard3 DCC
	if( (bTerminate==FALSE)&&(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE)&&(m_STD3_TDL_Data.m_bProceedDCC==TRUE) )
	{
		int customerOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION);
		nRes = P_NH_NOR_DisplayDCCOption(customerOption);

		if(nRes==RES_DCC_USE_LOCALCURRENCY)										// Withdrawal without DCC conversion
			m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_OfferAcceptance = L"0"; 
		else if(nRes==RES_DCC_USE_HOMECURRENCY)									// Withdrawal with DCC conversion
			m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_OfferAcceptance = L"1";
		else
			bTerminate = TRUE;
	}

	// Cancelled or Denied by a user
	if (bTerminate==TRUE)
		return RES_NG;
	// Remove Card : Performe at Cancel routine
	//{
	//	if (P_EMV_CheckTransMode() == RES_EMV_IC_FIRST)
	//		P_EMV_RemoveCard();
	//	return RES_NG;
	//}

	// Save DCC Journal Info
	CString strTemp, strTemp2;
	strTemp2 = _T("");
	if(m_sSTD1_DynamicFlowResp.nDCCTransaction==1)
	{
		strTemp.Format(L"%c%s%s%s", UNIT_DELIMITER, RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_DCC_EXCHANGERATE,  m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate);
		strTemp2 += strTemp;
		strTemp.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_DCC_CURRENCY,      m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency);
		strTemp2 += strTemp;
		strTemp.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_DCC_CONVERTEDAMT,  m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount);
		strTemp2 += strTemp;
		strTemp.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_DCC_CONVERTEDSCG,  m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge);
		strTemp2 += strTemp;
	}
	else if(m_STD3_TDL_Data.m_bProceedDCC==TRUE)
	{
		strTemp.Format(L"%c%s%s%s", UNIT_DELIMITER, RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_DCC_EXCHANGERATE,  m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferConversionRate);
		strTemp2 += strTemp;
		strTemp.Format(L"%s%s%s(%s)", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_DCC_CURRENCY,  m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferHomeCurrencyISO, m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferHomeCurrencyCode);
		strTemp2 += strTemp;
		strTemp.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_DCC_CONVERTEDAMT,  m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_ConvertedAmount);
		strTemp2 += strTemp;
		strTemp.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_DCC_CONVERTEDSCG,  m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_ConvertedSurcharge);
		strTemp2 += strTemp;
		strTemp.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_DCC_OFFERACCEPT,   m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_OfferAcceptance);
		strTemp2 += strTemp;
		// [2302] US Justin 2014.11.05 Remove Quotation Type
		//strTemp.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_DCC_QUOTETYPE,     m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferQuotationType);
		//strTemp2 += strTemp;
		// End of [#2302]
	}
	NHDEBUG(DBG_CALL, (L"Other Message for DCC = [%s]\n", strTemp2));
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strTemp2);
	return RES_OK;
}
// End of [#2496]


// [#2330] US Justin 2015.02.10 Percentage DCC Surcharge
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_CalculateLocalSurcharge()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Cancluage Local(percentage or Table) Surcharge
-------------------------------------------------------------------*/
CString	CTranCmn::P_NH_NOR_CalculateLocalSurcharge(int nMode)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_CalculateLocalSurcharge(%d)\n", nMode));

	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC)==ENABLE &&
		MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE)==ENABLE &&
		LIB_CheckSurchargeDisplay() == FALSE )
	{
		nMode = SURCHARGE_DISP_UNDER_BINRANGE;
	}

	CString sSurchargeAmount = L"";
	switch (nMode)
	{
		case SURCHARGE_DISP_NORMAL:
			sSurchargeAmount = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT);
			break;
		case SURCHARGE_DISP_PRECENT:
			{
				
				//float fPercentSurcharge = (float)(Asc2Int(m_sUserSelection.strMoney) * 0.01) * (float)(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_PERCENT) * 0.01);
				float fPercentSurcharge = (float)(Asc2Int(m_sUserSelection.strMoney) * 0.01) * (float)(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE) * 0.0001);	// [#2316] US Justin 2014.12.17 Support Decimal Percentage surcharge
				float fResultSurcharge = 0;

				// [#2335] US Justin 2015.03.12 Add Percentage surcharge Notice
				//if(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_MANNER) == GREATER)
				//	fResultSurcharge = max(fPercentSurcharge, (float)(Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT))*0.01));
				//else
				//	fResultSurcharge = min(fPercentSurcharge, (float)(Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT))*0.01));
				float fLocalSurchargeAmount =  (float)(  0.01 * Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT) ) ); 
				if(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_MANNER) == GREATER)
				{
					if( fPercentSurcharge > fLocalSurchargeAmount)
					{
						fResultSurcharge = fPercentSurcharge;
						// [#2354] US Justin 2015.07.01 Surcharge Notice
						//m_sUsedSurchargePercentage.Format(L"%0.2f %%", (float)(0.01 * MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE) ) );
						if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PERCENT_SURCHARGE_NOTICE) == SURCHARGENOTICE_BOTH )
							m_sUsedSurchargePercentage.Format(L"%s %0.2f (%0.2f%%)", GetCurrencySymbol(), fResultSurcharge, (float)(0.01 * MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE) ) );
						else
							m_sUsedSurchargePercentage.Format(L"%0.2f %%", (float)(0.01 * MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE) ) );
						// End of [#2354]
					}
					else
						fResultSurcharge = fLocalSurchargeAmount;
				}
				else
				{
					if( fPercentSurcharge < fLocalSurchargeAmount)
					{
						fResultSurcharge = fPercentSurcharge;
						// [#2354] US Justin 2015.07.01 Surcharge Notice
						//m_sUsedSurchargePercentage.Format(L"%0.2f %%", (float)(0.01 * MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE) ) );
						if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PERCENT_SURCHARGE_NOTICE) == SURCHARGENOTICE_BOTH )
							m_sUsedSurchargePercentage.Format(L"%s %0.2f (%0.2f%%)", GetCurrencySymbol(), fResultSurcharge, (float)(0.01 * MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE) ) );
						else
							m_sUsedSurchargePercentage.Format(L"%0.2f %%", (float)(0.01 * MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE) ) );
						// End of [#2354]
					}
					else
						fResultSurcharge = fLocalSurchargeAmount;
				}
				// End of [#2335]
				sSurchargeAmount.Format(L"%d", (int) (fResultSurcharge*100 + 0.001) );
			}
			break;
		case SURCHARGE_DISP_TABLE:
			{
				int nRequestAmtTemp = Asc2Int(m_sUserSelection.strMoney, 12) / 100;
				int i=0;
				for (i=0; i < 10; i++)
				{
					if (nRequestAmtTemp <= m_pDevCmn->m_nSurchTable[0][i])
					{
						m_pDevCmn->m_strTableSurcharge.Format(L"%d", m_pDevCmn->m_nSurchTable[1][i]);
						break;
					}
				}
				if (i == 10)	// More than MAX
					m_pDevCmn->m_strTableSurcharge.Format(L"%d", m_pDevCmn->m_nSurchTable[1][9]);
				sSurchargeAmount = m_pDevCmn->m_strTableSurcharge;
			}
			break;
		case SURCHARGE_DISP_UNDER_BINRANGE:
			sSurchargeAmount.Format(L"%d", 0);
			MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT, sSurchargeAmount);
			break;
	}

	return sSurchargeAmount;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_DFDCC_Convert2HomeCurrency()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Convert amount to home currency for STD1 Dynamic Flow DCC
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_DFDCC_Convert2HomeCurrency()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::fnAPP_DCC_Convert2HomeCurrency]\n"));

	// Exchange Rate
	char chTemp[1024] = { 0, };
	WideToMulti(chTemp, m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate, sizeof(chTemp));
	double fERate = (float)atof(chTemp);

	// Requested Amount
	int nReqAmount	  = Asc2Int(m_sUserSelection.strMoney);

	// Digits of Currency
	int nBCDigit      = Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Mid(4, 1) );
	int nTCDigit      = Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Right(1) );

	// Actual Surchrage Amount
	// Using Local Surcharge... (SPEC V1.61)
	if( (m_sSTD1_DynamicFlowResp.R1_Surcharge_DCC.CompareNoCase(L"--------") == 0 ) ||		// "--------" : Use Local Surcharge
		(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC)==ENABLE &&
		 MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE)==ENABLE &&
		 LIB_CheckSurchargeDisplay() == FALSE) )
	{
		int nSurchargeMode = SURCHARGE_DISP_NORMAL;
		if(		 MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) == PERCENT_MODE )	{ nSurchargeMode = SURCHARGE_DISP_PRECENT; }
		else if( MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) == TABLE_MODE )		{ nSurchargeMode = SURCHARGE_DISP_TABLE; }

		CString sLocalSurcharge = P_NH_NOR_CalculateLocalSurcharge(nSurchargeMode);
		m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC.Format(L"%08d", Asc2Int(sLocalSurcharge) );
	}
	else	// if Surcharge amount is provided
	{
		m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC = m_sSTD1_DynamicFlowResp.R1_Surcharge_DCC;
	}

	int nDCCSurcharge = Asc2Int(m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC);

	// Convert to Home Currency
	if(m_HostConfig == HC_DUALHOST)
	{
		int nConvTotalAmt	= (int) ( fERate*(nReqAmount+nDCCSurcharge)    *pow(10.0,(nTCDigit-nBCDigit)) + 0.5 );
		int nConvReqAmt		= (int) ( fERate*(nReqAmount) *pow(10.0,(nTCDigit-nBCDigit)) + 0.5 );
		int nConvDCCSur		= nConvTotalAmt - nConvReqAmt;
		m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount.Format (L"%012ld", nConvReqAmt );
		m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge.Format(L"%08d",   nConvDCCSur );
	}
	else if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION) == DCC_CUSTOMOPTION_CIBC )
	{
		int nConvTotalAmt	= (int) ( fERate*(nReqAmount+nDCCSurcharge)    *pow(10.0,(nTCDigit-nBCDigit)) + 0.5 );
		int nConvDCCSur		= (int) ( fERate*(nDCCSurcharge) *pow(10.0,(nTCDigit-nBCDigit)) + 0.5 );
		int nConvReqAmt		= nConvTotalAmt - nConvDCCSur;						
		m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount.Format (L"%012ld", nConvReqAmt );
		m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge.Format(L"%08d",   nConvDCCSur );
	}
	else
	{
		m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount.Format (L"%012ld", (int) ( fERate*(nReqAmount)    *pow(10.0,(nTCDigit-nBCDigit)) + 0.5 ) );
		m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge.Format(L"%08d",  (int) ( fERate*(nDCCSurcharge) *pow(10.0,(nTCDigit-nBCDigit)) + 0.5 ) );
	}

	NHDEBUG(DBG_CALL, (L"R2_ConnectionResult = [%s]\n", m_sSTD1_DynamicFlowResp.R2_ConnectionResult));
	NHDEBUG(DBG_CALL, (L"R2_DCC_BaseTargetCurrency = [%s]\n", m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency));
	NHDEBUG(DBG_CALL, (L"R2_DCC_ExchangeRate = [%s]\n", m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate));
	NHDEBUG(DBG_CALL, (L"R2_Surcharge_DCC = [%s]\n", m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC));
	NHDEBUG(DBG_CALL, (L"R2_DCC_ConvertedTrAmount = [%s]\n", m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount));
	NHDEBUG(DBG_CALL, (L"R2_DCC_ConvertedSurcharge = [%s]\n", m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge));

	return RES_OK;
}
// End of [#2330]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_DisplaySurcharge()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Display Surcharge
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_DisplaySurcharge(int nSurchargeOption)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"ShowSurcharge");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_DisplaySurcharge]\n"));

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEDISPLAY, 1);

	// [#2344] US Justin 2015.05.21 Cardtronics Skip Surcharge Notice for DCC Eligible Transaction
	if( nSurchargeOption == SURCHARGE_DISP_TDL_DCC && MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION) == DCC_CUSTOMOPTION_CARDTRONICS )
	{
		m_nDisplayedSurchargeAmount = Asc2Int(m_STD3_TDL_Data.m_strSurchargeAmount);
		return RES_OK;
	}
	// End of [#2344]

	// [RWC6-527] Surcharge Screen Enable - Disable
#if (MX_VERSION)
	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
	{
		if ((MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE) != ENABLE))
		{
			m_nDisplayedSurchargeAmount = Asc2Int(m_STD3_TDL_Data.m_strSurchargeAmount);
			return RES_OK;
		}
	}
#endif
	// End of [RWC6-527]

	// [#2426] US Justin Support STD1 Cardtronics Option
	if( (nSurchargeOption==SURCHARGE_DISP_DYNAMICDCC)&&(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION)==DCC_CUSTOMOPTION_CARDTRONICS) )
	{
		m_nDisplayedSurchargeAmount = Asc2Int(m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC);
		return RES_OK;
	}
	// End of [#2426]

	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC)==ENABLE &&
		MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE)==ENABLE &&
		LIB_CheckSurchargeDisplay() == FALSE )
	{
		nSurchargeOption = SURCHARGE_DISP_UNDER_BINRANGE;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Calculate Surcharge ......
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	CString sSurchargeAmount = L"";
	switch (nSurchargeOption)
	{
		// [#2330] US Justin 2015.02.10 Percentage Surcharge for DCC
		case SURCHARGE_DISP_NORMAL:
		case SURCHARGE_DISP_PRECENT:
		case SURCHARGE_DISP_TABLE:
			sSurchargeAmount = P_NH_NOR_CalculateLocalSurcharge(nSurchargeOption);
			break;
		// End of [#2330]

		case SURCHARGE_DISP_DYNAMICDCC:
			sSurchargeAmount = m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC;
			break;

		case SURCHARGE_DISP_DYNAMIC1ST:		// Surcharge segmentation, Cash Withdrawal(NON DCC).. Host Provides a surcharge amount on the 1st call of Dynamic Flow
			sSurchargeAmount = m_sSTD1_DynamicFlowResp.R1_Surcharge_Withdrawal;
			break;
		case SURCHARGE_DISP_DYNAMIC2ND:		// Surcharge segmentation, Cash Withdrawal(NON DCC).. Host Provides a surcharge amount on the 2nd call of Dynamic Flow
			sSurchargeAmount = m_sSTD1_DynamicFlowResp.R2_Surcharge_Withdrawal;
			break;
		case SURCHARGE_DISP_BALANCEINQUIRY:	// Surcharge segmentation, Balance Inquiry         .. Host Provides a surcharge amount on the 1st call of Dynamic Flow
			sSurchargeAmount = m_sSTD1_DynamicFlowResp.R1_Surcharge_BalanceInquiry;
			break;
		case SURCHARGE_DISP_PINCHANGE:		// Surcharge segmentation, Balance Inquiry         .. Host Provides a surcharge amount on the 1st call of Dynamic Flow
			sSurchargeAmount = m_sSTD1_DynamicFlowResp.R1_Surcharge_PinChange;
			break;
		
		case SURCHARGE_DISP_TDL_SEGMENTATION:	// [#2292] US Justin 2014.10.01 TDL Surcharg segmentation or DCC
		case SURCHARGE_DISP_TDL_DCC:			// [#2344] US Justin 2015.05.21 Cardtronics Skip Surcharge Notice for DCC Eligible Transaction
			sSurchargeAmount = m_STD3_TDL_Data.m_strSurchargeAmount;
			break;

		case SURCHARGE_DISP_UNDER_BINRANGE:
			sSurchargeAmount.Format(L"%d", 0);
			MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT, sSurchargeAmount);

			break;
	}

	BOOL bUseCustomized = FALSE;				// [#2437] Supporting Cardtronics ALLPOINT Surcharge Notice
	// [#2292] US Justin 2014.09.17 Add Cardtronics TDL Option
	// Adding Extra Surcharge for Cash withdrawal from CREDIT (Except TDL DCC)
	// To add credit card segmentation for US general version as well.
	#if(APP_TDL_OPTION || (US_VERSION && !APP_TDL_OPTION))	// [#RWC6-29] Add ALLPOINT for PAI
		if( ( (nSurchargeOption==SURCHARGE_DISP_NORMAL)			 ||(nSurchargeOption==SURCHARGE_DISP_PRECENT)   ||(nSurchargeOption==SURCHARGE_DISP_TABLE)  ||
			  (nSurchargeOption==SURCHARGE_DISP_TDL_SEGMENTATION)||(nSurchargeOption==SURCHARGE_DISP_DYNAMIC1ST)||(nSurchargeOption==SURCHARGE_DISP_DYNAMIC2ND) ) &&		// [#2431] US Justin 2016.06.13 
			( m_SourceAccount == S_CREDITCARD ) )
		{
			int nNewSurcharge = Asc2Int(sSurchargeAmount) + Asc2Int( MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE) );
			sSurchargeAmount.Format(L"%d", nNewSurcharge);
			// [#2335] US Justin 2015.03.12 Block Displaying % notice if an extra amount surcharge is added.....
			if(  Asc2Int( MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE) ) > 0 )
				m_sUsedSurchargePercentage = L"";
			// End of [#2335]
		}

		// [#2437] Supporting Cardtronics ALLPOINT Surcharge Notice
		#if(US_VERSION)
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_SURCHARGE) )
			{
				if( (nSurchargeOption==SURCHARGE_DISP_NORMAL)||(nSurchargeOption==SURCHARGE_DISP_PRECENT)||(nSurchargeOption==SURCHARGE_DISP_TABLE) )
					bUseCustomized = TRUE;
			}
		#endif
		// End of [#2437]
	#endif
	// End of [#2292]

	// [#RWC6-514] US ryan.payton 2023.01.10 CashDepot add surcharge
	#if (APP_CUSTOM_CASHDEPOT)
		if (_cdTransactionState.Configurations.Surcharges)
		{
			int tmpInt = _cdTransactionState.Configurations.Surcharges * 100;
			sSurchargeAmount.Format(L"%d", tmpInt);
		}
	#endif
	// End of [#RWC6-514]

	// Value for Screen Display and Voice Guidance  "250" => "2.50"
	CString	strScreenSurcharge = MakeMoneyCent(sSurchargeAmount);		// [#2375] US Justin 2015.10.29 Surcharge for Display and Voice Guidance

	// [#RWC6-504] US ryan.payton 2023.01.06 CashDepot add admin fees
#if (APP_CUSTOM_CASHDEPOT)
	CString strScreenAdminFee = L"";

	if (_cdTransactionState.Configurations.AdminFees)                        
	{
		int adminFee = _cdTransactionState.Configurations.AdminFees * 100;   
		CString strTemp;
		strTemp.Format(L"%d", adminFee);
		strScreenAdminFee = MakeMoneyCent(strTemp);

		int tmpInt = Asc2Int(sSurchargeAmount);
		tmpInt += adminFee;
		
		sSurchargeAmount.Format(L"%d", tmpInt);
	}
#endif
	// End of [#RWC6-504]

	// Value for Request ex: "200" = $2.00
	m_nDisplayedSurchargeAmount = Asc2Int(sSurchargeAmount);

	// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// ADA Variable
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CString strVGInput;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// [#2375]

	// [#2437] Supporting Cardtronics ALLPOINT Surcharge Notice
	int nScreenNumber = 109;
	if(bUseCustomized)
		nScreenNumber = 159;
	// End of [#2437]

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_81");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2375]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if (US_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFile(201, L"201.wav");
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(201, strScreenSurcharge);
					m_pAdaCtrl->fnExp_AddWaveFile(201, L"201_1.wav");
					// [#2437] Supporting Cardtronics ALLPOINT Surcharge Notice
					if(bUseCustomized)
						m_pAdaCtrl->fnExp_AddWaveFile(201, L"201_2.wav");		// ALLPOINT Surcharge Notice (won't be charged...)
					m_pAdaCtrl->fnExp_AddWaveFile(201, L"201_3.wav");			// To Accedpt this surcharge
					// End of [#2437]
					m_pAdaCtrl->fnExp_AddWaveFile(201, L"Press7.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(201, L"RefuseCancelLocationKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(201, L"LocationRepeatKey.wav");				
				#else
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(201, strScreenSurcharge);
					m_pAdaCtrl->fnExp_AddWaveFile(201, L"201_1.wav");  //"This fee is in addition...
				#endif
				m_pAdaCtrl->fnExp_PlayScreenWave(201);
				bStartToTimeout = TRUE;
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(nScreenNumber);

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
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109001));

					// Sub Title
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109002));

					// [#2437] Supporting Cardtronics ALLPOINT Surcharge Notice
					if(bUseCustomized==TRUE)
					{
						strTemp.Format(L"%s %s", GetCurrencySymbol(), strScreenSurcharge);
						CString strCustomSurcharge = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_159001);
						strCustomSurcharge.Replace( L"$$$$", strTemp );

						// ALLPOINT FULL Surcharge Notice
						m_pDevCmn->fnSCR_DisplayString(4, strCustomSurcharge);
					}
					else
					{
						// FEE NOTICE:The owner of this terminal
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109101));				

						// Owner
						m_pDevCmn->fnSCR_DisplayString(5, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER));

						// charge a fee of
						m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109102));

						// Surcharge => Amount or percentage
						if ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PERCENT_SURCHARGE_NOTICE) != SURCHARGENOTICE_AMOUNT) && (m_sUsedSurchargePercentage.GetLength() > 4))	// [#2354] US Justin 2015.07.01 Surcharge Notice
							strTemp = m_sUsedSurchargePercentage;
						else
							strTemp.Format(L"%s %s", GetCurrencySymbol(), strScreenSurcharge);																							// [#2375] US Justin 2015.10.29 Use Same variable as VG

						m_pDevCmn->fnSCR_DisplayString(7, strTemp);

						// For Cash Withdrawals.
						if (nSurchargeOption == SURCHARGE_DISP_BALANCEINQUIRY)
							m_pDevCmn->fnSCR_DisplayString(10, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109109)); // for Balance Inquiry.
						else if (nSurchargeOption == SURCHARGE_DISP_PINCHANGE)
							m_pDevCmn->fnSCR_DisplayString(10, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109111)); // for PIN Change
						else
							m_pDevCmn->fnSCR_DisplayString(10, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109107)); // for CASH WITHDRAWALS.

#if (APP_CUSTOM_CASHDEPOT)
						if (!strScreenAdminFee.IsEmpty())
						{
							strTemp.Format(L"%s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109108), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109110));	// This fee is in addition to any fee..... Admin fee is charged on foreign or...
						}
						else
#endif
						{
							strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109108));																// This fee is in addition to any fee.....
						}

						m_pDevCmn->fnSCR_DisplayString(13, strTemp);

#if (APP_CUSTOM_CASHDEPOT)
						if (!strScreenAdminFee.IsEmpty())
						{
							// Additional Admin Fee
							m_pDevCmn->fnSCR_DisplayString(14, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_109112));

							// Admin Fee => Amount
							strTemp.Format(L"%s %s", GetCurrencySymbol(), strScreenAdminFee);

							m_pDevCmn->fnSCR_DisplayString(15, strTemp);
						}
#endif
					}
					// End of [#2437]
				}

				m_pDevCmn->fnSCR_DisplayScreen(nScreenNumber, KEYIN_TIME_OUT, PIN_MENU_MODE);
			}
			// End of [#2375]

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

		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2375]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2375]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				// [#2375] US Justin Combine ADA and Screen Flow
				if( IsAdaTransaction())		
				{
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2375]

				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_14");	// [#2024] NH KSK 2011.02.24
				m_OfferAcceptance.IncreaseAcceptanceResult(OFFER_SURCHARGE_DECLINE);	// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_15");	// [#2024] NH KSK 2011.02.24
				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_YES)
			{
				m_OfferAcceptance.IncreaseAcceptanceResult(OFFER_SURCHARGE_ACCEPT);		// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
				return RES_OK;
			}
			else if (GetKeyStr == S_NO)
			{
				m_OfferAcceptance.IncreaseAcceptanceResult(OFFER_SURCHARGE_DECLINE);	// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
				return RES_USER_EXIT;
			}
			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			/////////////////////////////////// VOICE GUIDANCE MODE ///////////////////////////////
			else
			{
				if( IsAdaTransaction() )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));
					int nValidADAInput = 0;

					if(GetKeyStr == L"7")
					{
						strVGInput = L"7Accepted.wav";
						nValidADAInput = 1;					// (1) Valid Input Accept Surcharge Return RES_OK
					}
					else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
					{
						nValidADAInput = 2;					// (2) Repeat or Volume Control
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
					}
										
					if	( nValidADAInput == 0 )				// Invalid
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
							bInvalidRepeat = TRUE;
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						#endif
					}
					else if( nValidADAInput == 1)			// Valid Input
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(strVGInput, TRUE);
						#endif
						m_OfferAcceptance.IncreaseAcceptanceResult(OFFER_SURCHARGE_ACCEPT);		// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
						return RES_OK;
					}
					else if( nValidADAInput == 2)			// Repeat or Volume Control
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
				}
			}
			// End of [#2375]
		}
#endif

		Delay_Msg(50);

		// [#2375] US Justin 2015.11.03 US Justin US ADA
		#if(US_VERSION)
			if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
				if( P_NH_ADA_NeedMoreTime() == RES_OK )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
				else
					return RES_USER_EXIT;
			}
		#endif
		// End of [#2375]
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_128");	// [#2375] US Justin 2015.11.03
	return RES_USER_TIMEOUT;
}

// [#2150] US Justin 2012.10.01 Add Dynamic Currency Conversion
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_DisplayDCCOption(int nCustomerType)
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Choosing Local Currency withdrawal or Home Currency Withdrawal.
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_DisplayDCCOption(int nCustomerType)		// [#2292] US Justin 2014.10.01 Add TDL DCC
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_DisplayDCCOption]\n"));

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Calculagte DCC Value (Convert to Home Currency)
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	CString strTemp;

	// Define Customer Type
	int nCustomerOption = nCustomerType;						// CARDTRONICS or Default
	if( nCustomerOption == DCC_CUSTOMOPTION_DEFAULT)			// General, CIBC, CDS, Dual Host(PAI), Planet, Monex
		nCustomerOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION);
	NHDEBUG(DBG_INFO, (_T("DCC OFFER SCREEN : Customer Option = [%d]\n"), nCustomerOption));

	// Requested Amount for Display or Voice Guidance		//	2000 => 20.00
	CString strRequestedAmount;
	strRequestedAmount.Format(L"%0.2f", (float)(Asc2Int(m_sUserSelection.strMoney) * 0.01));
	NHDEBUG(DBG_INFO, (_T("DCC OFFER SCREEN : Requested Amount = [%s]\n"), strRequestedAmount));

	// Surcharge Amount for Display or Voice Guidance		//	2.00
	CString strSurchargeAmount = L"0";
	if( m_nDisplayedSurchargeAmount > 0 )
		strSurchargeAmount.Format(L"%0.2f", (float)(0.01*m_nDisplayedSurchargeAmount) );			

#if (APP_TDL_OPTION)
	if(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
	{
		m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_SurchargeAmount = m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_SurchargeAmount;
	}
#endif

	NHDEBUG(DBG_INFO, (_T("DCC OFFER SCREEN : Surcharge Amount = [%s]\n"), strSurchargeAmount));

	// Local Total Amount (USD)								//	22.00
	CString strLocalTotalAmount;
	strLocalTotalAmount.Format(L"%0.2f", (float)( (m_nDisplayedSurchargeAmount + Asc2Int(m_sUserSelection.strMoney)) * 0.01) );					
	NHDEBUG(DBG_INFO, (_T("DCC OFFER SCREEN : Total Amount = [%s]\n"), strLocalTotalAmount));

	// Exchange Rate
	// Display Format
	// "1 EUR = 1.65 USD"  : Cardtronics
	// "1 USD = 0.606 EUR" : CIBC
	// "0.606 EUR / USD"   : General, PAI(Dual Host), CDS, Planet, Monex
	CString strDispExchangeRate = L"";						
	CString strHomeCurrencyName = L"";						// EUR
	CString strADAExchangeRate  = L"";						// 1.65 (TDL) or 0.606 (Standard1)
	CString strADAMarkupRate	= L"";						// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message  // TODO need this for ADA

	// TDL DCC
#if (APP_TDL_OPTION)
	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE)
	{
		int nReqAmount = Asc2Int(m_sUserSelection.strMoney);
		int nConvertedTotal, nConvertedSurcharge, nConvertedRequested;
 		// Convert Exchange Rate
		char chTemp[1024] = { 0, };
		WideToMulti(chTemp, m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferConversionRate, sizeof(chTemp));
		double fERateOrg = (float)atof(chTemp);

		// [#2297] US Justin 2014.10.20 Add Converted Amount in DCC Lookk up response
		BOOL bCalculateLocally = TRUE;
		if( Dollar2Cent(m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCConvertedTranAmount) > 0)
		{
			nConvertedRequested = Dollar2Cent(m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCConvertedTranAmount);
			nConvertedTotal = nConvertedRequested + Dollar2Cent(m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCConvertedSurcharge);
			bCalculateLocally = FALSE;
		}
		// End of [#2297]					

		if (m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferQuotationType == L"0")			// Direct 
		{
			if(bCalculateLocally)		// [#2297] US Justin 2014.10.20 Add Converted Amount in DCC Lookk up response
			{
				nConvertedTotal = (int) ( 1.0 * (nReqAmount+m_nDisplayedSurchargeAmount) / fERateOrg );
				nConvertedRequested =  (int) ( 1.0 * nReqAmount/ fERateOrg );
			}

			// Exchange Rate for Display
#if (APP_CUSTOM_CASHDEPOT)
			strDispExchangeRate.Format(L"1.00 %s = %s %s",	m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferHomeCurrencyISO, m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferConversionRate, 
											MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID));
			m_STD3_TDL_Data.m_DCC_Lookup.m_strDisplayedExchangeRate = m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferConversionRate;

			strADAExchangeRate = m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferConversionRate;
#else
			strDispExchangeRate.Format(L"1 %s = %s %s",	m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferHomeCurrencyISO, m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferConversionRate, 
											MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID));
			m_STD3_TDL_Data.m_DCC_Lookup.m_strDisplayedExchangeRate = m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferConversionRate;

			strADAExchangeRate = m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferConversionRate;
#endif
		}
		////////////////////// Cardtronics Does not use Indirect Rate....=> Following is not valid for Cardtronics TDL....
		else																				// Indirect (exchange rate * USD)
		{
			if(bCalculateLocally)		// [#2297] US Justin 2014.10.20 Add Converted Amount in DCC Lookk up response
			{
				nConvertedTotal = (int) ( fERateOrg*(nReqAmount+m_nDisplayedSurchargeAmount) );
				nConvertedRequested =  (int) ( fERateOrg * nReqAmount );
			}

			CString strExchangeRate = _T("");
			double fERate = 1.0 / fERateOrg;
			//////////// Making 8 Valid Digit for Display /////
			///////////////////////////////////////////////////
			int nReqDigit = 8;
			long nCompare = 1;
			for(int i=0; i<9; i++)
			{
				if( fERate < (float)nCompare )
					break;
				nCompare*=10;
				nReqDigit--;
			}
			if(nReqDigit<=0)	strTemp.Format(L"%%0.0f");
			else				strTemp.Format(L"%%0.%df", nReqDigit);
			///////////////////////////////////////////////////
			strExchangeRate.Format(strTemp, fERate);
#if (APP_CUSTOM_CASHDEPOT)
			strDispExchangeRate.Format(L"1.00 %s = %s %s",	m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferHomeCurrencyISO, strExchangeRate, 
											MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID));
			m_STD3_TDL_Data.m_DCC_Lookup.m_strDisplayedExchangeRate = strExchangeRate;

			strADAExchangeRate = strExchangeRate;	
#else
			strDispExchangeRate.Format(L"1 %s = %s %s",	m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferHomeCurrencyISO, strExchangeRate, 
											MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID));
			m_STD3_TDL_Data.m_DCC_Lookup.m_strDisplayedExchangeRate = strExchangeRate;

			strADAExchangeRate = strExchangeRate;	
#endif
		}

		// Markup Rate
		// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message
		if ( (m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCDisclaimerScheme==L"1") &&
			 (m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCMarkupRate.GetLength() > 0) )
		{
			strADAMarkupRate = m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCMarkupRate;	
		}
		// end of [#RWC6-2, #2585]

		strHomeCurrencyName = m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferHomeCurrencyISO;

		// Update Converted Amounts
		// [#2348] US Justin 2015.06.02 Cardtronics DCC offer remove decimal point for JYP
		int nNumDecimal = m_STD3_TDL_Data.m_DCC_Lookup.GetCurrencyDecimalNumber(m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferHomeCurrencyCode);
		if(nNumDecimal<=0)	strTemp.Format(L"%%0.0f");
		else				strTemp.Format(L"%%0.%df", nNumDecimal);
		if(bCalculateLocally!= TRUE)
		{
			m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_ConvertedAmount		= m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCConvertedTranAmount;
			m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_ConvertedSurcharge	= m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCConvertedSurcharge;
			m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_PresentedAmount.Format( strTemp, 0.01*nConvertedTotal);
		}
		else
		{
			nConvertedSurcharge = nConvertedTotal - nConvertedRequested;
			m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_ConvertedAmount.Format(	 strTemp, 0.01*nConvertedRequested);
			m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_ConvertedSurcharge.Format(strTemp, 0.01*nConvertedSurcharge);
			m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_PresentedAmount.Format( 	 strTemp, 0.01*nConvertedTotal);
		}
		// End of [#2348]
	} // Non-STD3 DCC, CIBC, Cardtronics
	else 
#endif
	{
		// 1 USD = x.xxx EUR
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)
		strDispExchangeRate.Format(L"1.00 %s = %s %s",	m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Left(3), m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate, m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Mid(6,3));
		m_sSTD1_DynamicFlowResp.R2_DisplayedExchangeRate = m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate;
#else
		strDispExchangeRate.Format(L"1 %s = %s %s",	m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Left(3), m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate, m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Mid(6,3));
		m_sSTD1_DynamicFlowResp.R2_DisplayedExchangeRate = m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate;
#endif


		strHomeCurrencyName = m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Mid(6,3);
		strADAExchangeRate = m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate;

		// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message
		if (m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRateMarkUp.GetLength() > 0)
			strADAMarkupRate = m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRateMarkUp;	
		else if (m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRateMarkUp.GetLength() > 0)
			strADAMarkupRate = m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRateMarkUp;
		// end of [#RWC6-2, #2585]
	}

	NHDEBUG(DBG_INFO, (_T("DCC OFFER SCREEN : strDispExchangeRate = [%s]\n"), strDispExchangeRate));
	NHDEBUG(DBG_INFO, (_T("DCC OFFER SCREEN : strHomeCurrencyName = [%s]\n"), strHomeCurrencyName));
	NHDEBUG(DBG_INFO, (_T("DCC OFFER SCREEN : strADAExchangeRate = [%s]\n"), strADAExchangeRate));
	// End of [#2204]

	// Converted Total Amount								// 14.30
	CString strHomeTotalAmount;
#if (APP_TDL_OPTION)
	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
	{
		strHomeTotalAmount = m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_PresentedAmount;
	}
	else
#endif
	{
		// Home Currency Charge Value
		CString sTotalHome;		
		int nTotalHCValue = Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount) + Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge);
		int nTotalHCLength = Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Right(1) );
		sTotalHome.Format(L"%d", nTotalHCValue);

		if( nTotalHCLength <=0 )
			strHomeTotalAmount.Format(L"%s", sTotalHome);
		else
		{
			if( sTotalHome.GetLength() < (nTotalHCLength+1) )
			{
				CString strZeroTemp = _T("00000000000000000000");
				sTotalHome = strZeroTemp.Left( nTotalHCLength+1 - sTotalHome.GetLength() ) + sTotalHome;
			}
			strHomeTotalAmount.Format(L"%s.%s", sTotalHome.Left(sTotalHome.GetLength()-nTotalHCLength), sTotalHome.Right(nTotalHCLength));
		}
	}
	NHDEBUG(DBG_INFO, (_T("DCC OFFER SCREEN : Converted Total Amount = [%s]\n"), strHomeTotalAmount));

	// Assign Variable for Voice Guidance (Transaction result)
	m_strVG_ExchangeRate		= strADAExchangeRate;
	m_strVG_HomeCurrencyName	= strHomeCurrencyName;
	m_strVG_ConvertedAmt		= strHomeTotalAmount;	
	m_strVG_MarkupRate			= strADAMarkupRate; // [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message 


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Screen Display including Voice Guidance
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// ADA Variable
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CString strVGFile;

	// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
	BOOL	bIsVisaDCC = FALSE;
	if ( ((m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCDisclaimerScheme==L"1") &&
		  (m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCMarkupRate.GetLength() > 0)) ||
		 (m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRateMarkUp.GetLength() > 0) ||
		 (m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRateMarkUp.GetLength() > 0) )
		bIsVisaDCC = TRUE;
	// end of [#RWC6-2, #2585]

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// [#2375]

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_78");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2375]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();

				// You have requested a withdraeal amount of $20.00
				m_pAdaCtrl->fnExp_AddWaveFile(121, L"121.wav");
				m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(121, strRequestedAmount);

				// Your accountis denominated in EUR
				m_pAdaCtrl->fnExp_AddWaveFile(121, L"121_1.wav");
				//m_pAdaCtrl->fnExp_AddWaveFile(121, strHomeCurrencyName);
				strTemp.Format(L"%s.%s.%s", strHomeCurrencyName.Left(1), strHomeCurrencyName.Mid(1,1), strHomeCurrencyName.Mid(2,1));
				m_pAdaCtrl->fnExp_AddWaveFile(121, strTemp);


				// Surcharge Amount
				if( strSurchargeAmount != L"0" )
				{
					// To complete this transaction, you will be charged an access fee of $2.00
					m_pAdaCtrl->fnExp_AddWaveFile(121, L"121_2.wav");
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(121, strSurchargeAmount);
				}

				// Local Total Amount
				// plus any fee that your own bank may charge.  The total charge to your account of $22.00
				m_pAdaCtrl->fnExp_AddWaveFile(121, L"121_3.wav");
				m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(121, strLocalTotalAmount);		

				// Coverted Currency Name
				// can be converted to EUR
				m_pAdaCtrl->fnExp_AddWaveFile(121, L"121_4.wav");
				//m_pAdaCtrl->fnExp_AddWaveFile(121, strHomeCurrencyName);
				strTemp.Format(L"%s.%s.%s", strHomeCurrencyName.Left(1), strHomeCurrencyName.Mid(1,1), strHomeCurrencyName.Mid(2,1));
				m_pAdaCtrl->fnExp_AddWaveFile(121, strTemp);


				// Exchange Rate
				// by this ATM, using an exchange rate of
				m_pAdaCtrl->fnExp_AddWaveFile(121, L"121_5.wav");
				CString strATMCurrency = CURRENCY_TYPE;									// [#2375] 2015.11.25 US Justin
				if(nCustomerOption == DCC_CUSTOMOPTION_CARDTRONICS)
				{
					// 1 EUR equivalent to $1.65  => 1 EUR equivalent to 1.65 US Dollar : Exchange Rate can be long.
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(121, "1", strHomeCurrencyName);
					m_pAdaCtrl->fnExp_AddWaveFile(121, L"equivalent.wav");
					// [#2375] 2015.11.25 US Justin
					//m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(121, strADAExchangeRate);
					if( strATMCurrency.CompareNoCase(L"USD")==0 )		strTemp = L"USDollars.wav";				// [#2380] US Justin 2015.12.09 Cardtronics Additional VG Change
					else												strTemp.Format(L"%s ", CURRENCY_TYPE);	// Making 4 characters => Read like C.A.D
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(121, strADAExchangeRate, strTemp);
					// End of [#2375]
				}
				else
				{
					// 1 US dollar equivalent to 0.606 EUR  
					m_pAdaCtrl->fnExp_AddWaveFile(121, L"1.wav");
					// [#2375] 2015.11.25 US Justin
					//m_pAdaCtrl->fnExp_AddWaveFile(121, L"USDollar.wav");
					if( strATMCurrency.CompareNoCase(L"USD")==0 )
						m_pAdaCtrl->fnExp_AddWaveFile(121, L"USDollar.wav");
					else
					{
						strTemp.Format(L"%s.%s.%s", strATMCurrency.Left(1), strATMCurrency.Mid(1,1), strATMCurrency.Mid(2,1));
						m_pAdaCtrl->fnExp_AddWaveFile(121, strTemp);
					}
					// End of [#2375]
					m_pAdaCtrl->fnExp_AddWaveFile(121, L"equivalent.wav");
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(121, strADAExchangeRate, strHomeCurrencyName);
				}
				
				// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message 
				if (strADAMarkupRate.GetLength() > 0)
				{
					// [#RWC6-13] PAI has requested changes to Dual Host DCC offering screen
					if (nCustomerOption == DCC_CUSTOMOPTION_PLANET)
					{
						m_pAdaCtrl->fnExp_AddWaveFile(121, L"121_5_2.wav"); // "this includes"
						m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(121, strADAMarkupRate, L"percent.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(121, L"121_5_3.wav"); // "over wholesale rate"
					}
					// end of [#RWC6-13]
					else
					{
						m_pAdaCtrl->fnExp_AddWaveFile(121, L"121_5_1.wav");
						m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(121, strADAMarkupRate, L"percent.wav");
					}
				}
				// end of [#RWC6-2, #2585]
				
				// Converted Amount
				// This would result in a charge to your account of 1.45 EUR
				m_pAdaCtrl->fnExp_AddWaveFile(121, L"121_6.wav");
				m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(121, strHomeTotalAmount, strHomeCurrencyName);
				
				// Guide Option 
				// including the access fee.  To have this ATM perform this currency conversion, press 2
				m_pAdaCtrl->fnExp_AddWaveFile(121, L"121_7.wav");
				// To have your  own bank perform the currency conversion, press 4
				m_pAdaCtrl->fnExp_AddWaveFile(121, L"121_8.wav");

				// DCC Disclosure
				m_pAdaCtrl->fnExp_AddWaveFile(121, L"DCCDisclsr.wav");	

				m_pAdaCtrl->fnExp_AddWaveFile(121, L"LocateCancelKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(121, L"LocationRepeatKey.wav");

				m_pAdaCtrl->fnExp_PlayScreenWave(121);
				bStartToTimeout = TRUE;
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(135);
				// Setting Screen
				{
					m_pDevCmn->fnSCR_DisplayImage(1, TRUE);

					// Get Dynamic Currency Conversion Title	=> APValue2

					if (bIsVisaDCC == TRUE)
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)   // RWC6-645 DYNAMICDCC
						strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135802, nCustomerOption, bIsVisaDCC));
#else
						strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135002, nCustomerOption, bIsVisaDCC));
#endif
					else
						strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135001, nCustomerOption, bIsVisaDCC));

					m_pDevCmn->fnSCR_DisplayString(2, strTemp);

					// Requested Amount	TEXT 
					// [#2304] US Justin 2014.11.07 Change DCC Offer UI Add CDS Option
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135803, nCustomerOption, bIsVisaDCC));
#else
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135003, nCustomerOption, bIsVisaDCC));
#endif
					// Requested Amount Value - Local Currency				(EX. 20.00 USD)
					if (nCustomerOption == DCC_CUSTOMOPTION_PLANET)
					{
						strTemp.Format(L"%s %s", m_pDevCmn->m_strCurrencyID, strRequestedAmount);
					}
					else
					{
						strTemp.Format(L"%s %s", strRequestedAmount, MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID) );
					}
					m_pDevCmn->fnSCR_DisplayString(4, strTemp);

					// Surcharge Fee : if surcharge is greater than zero	(EX. 2.00 USD)
					if( strSurchargeAmount != L"0" )
					{
						// Surcharge TEXT
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
						m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135804, nCustomerOption, bIsVisaDCC));
#else
						m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135004, nCustomerOption, bIsVisaDCC));
#endif
						// Surcharge Value
						if (nCustomerOption == DCC_CUSTOMOPTION_PLANET) // [#RWC6-276] Updates to Planet Payment DCC screen/receipt
						{
							strTemp.Format(L"%s %s", m_pDevCmn->m_strCurrencyID, strSurchargeAmount);
						}
						else
						{
							strTemp.Format(L"%s %s", strSurchargeAmount, MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID) );
						}
						m_pDevCmn->fnSCR_DisplayString(6, strTemp);
					}

					// Total Amount Text 
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
					m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135805, nCustomerOption, bIsVisaDCC));
#else
					m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135005, nCustomerOption, bIsVisaDCC));
#endif
					// Total Amount Value - Local Currency					(EX. 22.00 USD)
					if (nCustomerOption == DCC_CUSTOMOPTION_PLANET)
					{
						strTemp.Format(L"%s %s", m_pDevCmn->m_strCurrencyID, strLocalTotalAmount);
					}
					else
					{
						strTemp.Format(L"%s %s", strLocalTotalAmount, MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID) );
					}
					m_pDevCmn->fnSCR_DisplayString(8, strTemp);

					// Exchange Rate Text

					m_pDevCmn->fnSCR_DisplayString(9, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135006, nCustomerOption, bIsVisaDCC));

					// Exchange Rate Value
					m_pDevCmn->fnSCR_DisplayString(10, strDispExchangeRate);

#if (MX_VERSION)
					if (bIsVisaDCC == TRUE)
#endif
					{
						// [#GSCJSD-5967] US ryan.payton 2023.02.01 Fixed Cardtronics being out of MC DCC compliance
						// DCC Offer
						if (!strADAMarkupRate.IsEmpty())
						{
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
							// Mark-up label  11162023 markup
							//strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135009, nCustomerOption, bIsVisaDCC));
							//m_pDevCmn->fnSCR_DisplayString(19, strTemp);

							// Mark-up value
							strTemp.Format(L"%s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135809, nCustomerOption, bIsVisaDCC), 
								                     m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135011, nCustomerOption, bIsVisaDCC));
							strTemp.Replace(L"XXXXX", strADAMarkupRate);
							m_pDevCmn->fnSCR_DisplayString(20, strTemp);
#else
							//Mark-up label  11162023 markup
							strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135009, nCustomerOption, bIsVisaDCC));
							m_pDevCmn->fnSCR_DisplayString(19, strTemp);

							// Mark-up value
							strTemp.Format(L"%s",m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135011, nCustomerOption, bIsVisaDCC));
							strTemp.Replace(L"XXXXX", strADAMarkupRate);
							m_pDevCmn->fnSCR_DisplayString(20, strTemp);
#endif

						}
					}

					// Home Currency TEXT ver 2
					if (bIsVisaDCC == TRUE)
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
						m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135808, nCustomerOption, bIsVisaDCC));
#else
						m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135008, nCustomerOption, bIsVisaDCC));
#endif
					else
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
						m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135807, nCustomerOption, bIsVisaDCC));
#else
						m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135007, nCustomerOption, bIsVisaDCC));
#endif
					// Home Currency VALUE ver 2
					if (nCustomerOption == DCC_CUSTOMOPTION_PLANET)
					{
						// [#RWC6-276] Updates to Planet Payment DCC screen/receipt
						strTemp.Format(L"%s %s", strHomeCurrencyName, strHomeTotalAmount);
					}
					else
					{
						strTemp.Format(L"%s %s", strHomeTotalAmount, strHomeCurrencyName);
					}

					m_pDevCmn->fnSCR_DisplayString(22, strTemp);

//					bIsVisaDCC = FALSE;  // RWC6-607 To test only Set MC

// KSK 2017.01.12 AU는 화면에 미표시 사양
#if !(AU_VERSION)
					if (bIsVisaDCC == TRUE)
					{
	#if (!MX_VERSION)
						// DCC Disclaimer ver 2
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
						m_pDevCmn->fnSCR_DisplayString(23, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135833, nCustomerOption, bIsVisaDCC));
#else
						m_pDevCmn->fnSCR_DisplayString(23, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135033, nCustomerOption, bIsVisaDCC));
#endif
	#endif
					}
					else
					{
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
						m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135843, nCustomerOption, bIsVisaDCC)); // 2, replaces 1, 11/15/2023

						// DCC Disclaimer ver 1
						m_pDevCmn->fnSCR_DisplayString(13, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135822, nCustomerOption, bIsVisaDCC));
#else
						m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135043, nCustomerOption, bIsVisaDCC)); // 2, replaces 1, 11/15/2023
						m_pDevCmn->fnSCR_DisplayString(13, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135022, nCustomerOption, bIsVisaDCC));
#endif
					}
#endif
					// End of [#RWC6-2, #2585, #GSCJSD-5967]

					if (bIsVisaDCC == TRUE)
					{
						// F7 Button => Transaction with Local Currency
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
						strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135813, nCustomerOption, bIsVisaDCC);
						m_pDevCmn->fnSCR_DisplayString(17, strTemp);

						// F8 => Transaction with home currency
						strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135814, nCustomerOption, bIsVisaDCC);
						m_pDevCmn->fnSCR_DisplayString(18, strTemp);
#else
						strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135013, nCustomerOption, bIsVisaDCC);
						m_pDevCmn->fnSCR_DisplayString(17, strTemp);

						// F8 => Transaction with home currency
						strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135014, nCustomerOption, bIsVisaDCC);
						m_pDevCmn->fnSCR_DisplayString(18, strTemp);
#endif

					}
					else
					{
						// F7 Button => Transaction with Local Currency
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
						strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135815, nCustomerOption, bIsVisaDCC);
						m_pDevCmn->fnSCR_DisplayString(17, strTemp);

						// F8 => Transaction with home currency
						strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135816, nCustomerOption, bIsVisaDCC);
						m_pDevCmn->fnSCR_DisplayString(18, strTemp);
#else
						strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135015, nCustomerOption, bIsVisaDCC);
						m_pDevCmn->fnSCR_DisplayString(17, strTemp);

						// F8 => Transaction with home currency
						strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135016, nCustomerOption, bIsVisaDCC);
						m_pDevCmn->fnSCR_DisplayString(18, strTemp);
#endif
					}
				}
				m_pDevCmn->fnSCR_DisplayScreen(135, KEYIN_TIME_OUT, PIN_MENU_MODE);
			}
			// End of [#2375]

			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
		}

#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			return RES_DCC_USE_LOCALCURRENCY;
		}
#else
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2375]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2375]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				// [#2375] US Justin Combine ADA and Screen Flow
				if( IsAdaTransaction())		
				{
#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
#endif
				}
				// End of [#2375]

				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_14");	// [#2024] NH KSK 2011.02.24
				m_OfferAcceptance.IncreaseAcceptanceResult(OFFER_DCC_DECLINE);		// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_15");						// [#2024] NH KSK 2011.02.24
				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == L"LOCALCURRENCY")
			{
				if ( (nCustomerOption==DCC_CUSTOMOPTION_CARDTRONICS) && (bIsVisaDCC==FALSE) )	// DCC ACCEPT for Cardtronics, Master DCC		// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
				{
					m_OfferAcceptance.IncreaseAcceptanceResult(OFFER_DCC_ACCEPT);		// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
					return RES_DCC_USE_HOMECURRENCY;
				}
				else
				{
					m_OfferAcceptance.IncreaseAcceptanceResult(OFFER_DCC_DECLINE);		// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
					return RES_DCC_USE_LOCALCURRENCY;
				}
			}
			else if (GetKeyStr == L"HOMECURRENCY")
			{
				if ( (nCustomerOption==DCC_CUSTOMOPTION_CARDTRONICS) && (bIsVisaDCC==FALSE) )	// DCC DECLINE for Cardtronics, Master DCC		// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
				{
					m_OfferAcceptance.IncreaseAcceptanceResult(OFFER_DCC_DECLINE);		// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
					return RES_DCC_USE_LOCALCURRENCY;
				}
				else
				{
					m_OfferAcceptance.IncreaseAcceptanceResult(OFFER_DCC_ACCEPT);		// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
					return RES_DCC_USE_HOMECURRENCY;
				}
			}
			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			/////////////////////////////////// VOICE GUIDANCE MODE ///////////////////////////////
			else
			{
				if( IsAdaTransaction() )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));

					int nValidADAInput = 0;
					if( GetKeyStr == L"2" )
					{
						strVGFile = L"2.wav";
						nValidADAInput = 1;								// (1) Accept DCC
					}
					else if ( GetKeyStr == L"4" )
					{
						strVGFile = L"4.wav";
						nValidADAInput = 2;								// (2) Decline DCC
					}
					else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
					{
						nValidADAInput = 3;								// (3) Repeat or Volume Control
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
					}
										
					if	( nValidADAInput == 0 )			// Invalid
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
							bInvalidRepeat = TRUE;
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
#endif
					}
					else if( (nValidADAInput==1)||(nValidADAInput==2) )		// Valid Input
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(strVGFile, TRUE);
#endif
						if(nValidADAInput==1) 	
						{
							m_OfferAcceptance.IncreaseAcceptanceResult(OFFER_DCC_ACCEPT);		// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
							return RES_DCC_USE_HOMECURRENCY;
						}
						else
						{
							m_OfferAcceptance.IncreaseAcceptanceResult(OFFER_DCC_DECLINE);		// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
							return RES_DCC_USE_LOCALCURRENCY;	
						}
					}
					else if( nValidADAInput == 3)			// Repeat or Volume Control
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
				}
			}
			// End of [#2375]
		}
#endif

		Delay_Msg(50);

		// [#2375] US Justin 2015.11.03 US Justin US ADA
		#if(US_VERSION)
			if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
				if( P_NH_ADA_NeedMoreTime() == RES_OK )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
				else
					return RES_USER_EXIT;
			}
		#endif
		// End of [#2375]
	}
	// End of [#2375]

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_129");	// [#2375] US Justin 2015.11.03
	return RES_USER_TIMEOUT;
}
// End of [#2150]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_ReceiveReceipt()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Receive Receipt
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_ReceiveReceipt()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"ReceiveReceipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_ReceiveReceipt]\n"));

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	///////////////////////////////////
	// CHECK CONDITION

	if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SELECTRECEIPT) != ENABLE)
	{
		m_sUserSelection.nPrintReceipt = RCPT_PAPER;			// [#2219] 2013.09.04 Justin Digital Receipt, Change Variable Name
		return RES_OK;
	}

	// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// ADA Variable
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CStringArray arrVG;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// [#2375]

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_85");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2375]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if (US_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFile(106, L"106.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(106, L"LocateCancelKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(106, L"LocationRepeatKey.wav");				
				#endif
				m_pAdaCtrl->fnExp_PlayScreenWave(106);
				bStartToTimeout = TRUE;
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(106);

				// Setting Screen
				{
					// History
					for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
					{
						if (nHistory == 0)
							m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

						m_pDevCmn->fnSCR_DisplayHistory(nHistory+1, m_arHistory[nHistory]);
					}

					// Title => AP Text

					// Sub Title
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106002));


					////////////////////////////////////////////////////////////////////
					//// Paper Receipt (F2 : Value1/State4)
					////////////////////////////////////////////////////////////////////					
					if (GetConfigFuncPointer()->SupportsCamera())		// [#2518] US Kook 2018.01.22 Support MX-2800SE
						m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106103));		// Paper Receipt		// [#GLDV-2505] Support MX-2800T
					else
						m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106003));		// Paper \n Receipt
					m_pDevCmn->fnSCR_DisplayImage(4, TRUE);					// Enable Paper Receipt Button Icon	[#2537]


					////////////////////////////////////////////////////////////////////
					//// Digital Receipt (F4 : Value2/State5) (Screen : Image-7, Text-8,9,10)
					////////////////////////////////////////////////////////////////////
					// IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT 
					// Digital Receipt Option =     MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DIGITAL_RECEIPT_OPTION)
					// AU				: 0=Disable, 1=Enable
					// Other Countries	: 0=Enable,  1=Disable
					// IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT 
					// [#2537] US JUSTIN 2018.03.07 Remove Imaged Text
					/*
				#if (CA_VERSION)
					// Digital Receipt is disabled CA 2700(BMO) and 1500SE
					if( ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DIGITAL_RECEIPT_OPTION)!=0)&&(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND).Left(6)==L"NH2700"))||
						(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND)==L"NH1500SE")   )		// [#2346] Disable Digital Receipt on 1500SE
					{
						m_pDevCmn->fnSCR_DisplayImage(5, L"off");					// Disable Digital Receipt Button Icon
						m_pDevCmn->fnSCR_DisplayString(7, L"SAVEPAPER_TREE");		// Save Tree Image (Screen)
					}
					else
					{
						m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106004));	// Digital Receipt
						m_pDevCmn->fnSCR_DisplayImage(5, L"on");					// Enable Digital Receipt Button Icon
						m_pDevCmn->fnSCR_DisplayString(7, L"SAVEPAPER_DRECEIPT");	// Digital Receipt APP Notice
					}
				#elif (AU_VERSION)
					if((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DIGITAL_RECEIPT_OPTION) == 0))			// Digital Receipt is disabled AU Only
					{
						m_pDevCmn->fnSCR_DisplayImage(5, L"off");					// Disable Digital Receipt Button Icon
						m_pDevCmn->fnSCR_DisplayString(7, L"SAVEPAPER_TREE");		// Save Tree Image (Screen)
					}
					else
					{
						m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106004));	// Digital Receipt
						m_pDevCmn->fnSCR_DisplayImage(5, L"on");					// Enable Digital Receipt Button Icon
						m_pDevCmn->fnSCR_DisplayString(7, L"SAVEPAPER_DRECEIPT");	// Digital Receipt APP Notice
					}
				#else
					// US and Mexico
					// [#2346] Disable Digital Receipt on 1500SE
					//m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106004));	// Digital Receipt
					if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND)==L"NH1500SE")
					{
						m_pDevCmn->fnSCR_DisplayImage(5, L"off");					// Disable Digital Receipt Button Icon
						m_pDevCmn->fnSCR_DisplayString(7, L"SAVEPAPER_TREE");		// Save Tree Image (Screen)
					}
					else
					{
						// [#2518] US Kook 2018.01.22 Support MX-2800SE
						if (m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_MX2800SE)
						{
							// Digital Receipt
							m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106104));
							// Introduce
							m_pDevCmn->fnSCR_DisplayString(8, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106301));
							m_pDevCmn->fnSCR_DisplayString(9, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106302));
							m_pDevCmn->fnSCR_DisplayString(10, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106303));
						}
						else
						// end of [#2518]
						{
							m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106004));	// Digital Receipt
							m_pDevCmn->fnSCR_DisplayImage(5, L"on");					// Enable Digital Receipt Button Icon
						}
						
						m_pDevCmn->fnSCR_DisplayString(7, L"SAVEPAPER_DRECEIPT");	// Digital Receipt APP Notice
					}
					// End of [#2346]
				#endif
					// End [#2263]
					*/

					BOOL bShowDigitalReceipt = TRUE;
				#if (CA_VERSION)
					// Digital Receipt is disabled CA 2700(BMO) and 1500SE
					if( ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DIGITAL_RECEIPT_OPTION)!=0)&&(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND).Left(6)==L"NH2700"))||
						(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND)==L"NH1500SE")   )		
						bShowDigitalReceipt = FALSE;
				#elif (AU_VERSION)
					// Digital Receipt is disabled AU Only
					if((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DIGITAL_RECEIPT_OPTION) == 0))			
						bShowDigitalReceipt = FALSE;
				#else
					if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND)==L"NH1500SE")
						bShowDigitalReceipt = FALSE;
				#endif

					if(bShowDigitalReceipt)
					{
						// Screen Text.. Use Hyosung Digital Receipt
						m_pDevCmn->fnSCR_DisplayString(7, L"SAVEPAPER_DRECEIPT");	// QR Image (Screen)
						m_pDevCmn->fnSCR_DisplayString(8,  m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106301));
						m_pDevCmn->fnSCR_DisplayString(9,  m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106302));
						m_pDevCmn->fnSCR_DisplayString(10, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106303));

						m_pDevCmn->fnSCR_DisplayImage(5, TRUE);						// Enable Digital Receipt Button Icon
						if (GetConfigFuncPointer()->SupportsCamera())		// [#2518] US Kook 2018.01.22 Support MX-2800SE
							m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106104));	// Digital Receipt			// [#GLDV-2505] Support MX-2800T
						else
							m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106004));	// Digital \n Receipt
					}
					else
					{
						// Screen Text.. Save Tree and Earth
						m_pDevCmn->fnSCR_DisplayString(7, L"SAVEPAPER_TREE");		// Tree Image (Screen)
						m_pDevCmn->fnSCR_DisplayString(8,  m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106201));
						m_pDevCmn->fnSCR_DisplayString(9,  m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106202));
						m_pDevCmn->fnSCR_DisplayString(10, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106203));

						m_pDevCmn->fnSCR_DisplayImage(5, FALSE);					// Disable Digital Receipt Button Icon
					}
					// end of [#2537]


					////////////////////////////////////////////////////////////////////
					//// NO Receipt (F6 : Value3/State6)
					////////////////////////////////////////////////////////////////////
					if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN) == RECEIPT_ON_SCREEN_ENABLE) ||
						(TranCode == TC_WITHDRAWAL) )
					{
						m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_106005));
						m_pDevCmn->fnSCR_DisplayImage(6, TRUE);				// [#2195] NH KSK 2013.05.15
					}
					else
						m_pDevCmn->fnSCR_DisplayImage(6, FALSE);			// [#2195] KMK_TEMP
				}
				m_pDevCmn->fnSCR_DisplayScreen(106, KEYIN_TIME_OUT, PIN_MENU_MODE);
			}
			// End of [#2375]

			bShowScreen = FALSE;
		
#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
		}

#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			m_sUserSelection.nPrintReceipt = FALSE;
			m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_RECEIPT));
			return RES_OK;
		}
#else
		
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2375]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2375]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
					m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_RECV_RECEIPT, L"USER CANCEL");
				#endif
				// [#2375] US Justin Combine ADA and Screen Flow
				if( IsAdaTransaction())		
				{
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2375]
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_16");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				//[#2000] SOOK 2010.10.16 호주향 개발 (거래 FLOW)
				#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
					if ( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME) == 0
						||  P_NH_NOR_NeedMoreTime() != RES_YES )
					{
						m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_RECV_RECEIPT, L"TIMEOUT");
						NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
						NVDump('O', 'C', "00", L"P_NHNOR", L"NG_17_1");	// [#2024] NH KSK 2011.02.24
						return RES_USER_TIMEOUT;
					}
					bShowScreen = TRUE;
				#else
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_17");	// [#2024] NH KSK 2011.02.24
					return RES_USER_TIMEOUT;
				#endif //end of [#2000]			
			}
			else if (GetKeyStr == S_YES)
			{
				m_sUserSelection.nPrintReceipt = RCPT_PAPER;			// [#2219] 2013.09.04 Justin Digital Receipt, Change Variable Name
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_RECEIPT));
				return RES_OK;
			}
			else if (GetKeyStr == S_NO)
			{
				m_sUserSelection.nPrintReceipt = FALSE;					// [#2219] 2013.09.04 Justin Digital Receipt, Change Variable Name
				return RES_OK;
			}
			// [#2219] 2013.09.04 JUSTIN Digital Receipt
			else if (GetKeyStr == S_QRCODE)
			{
				m_sUserSelection.nPrintReceipt = RCPT_QRCODE;
				return RES_OK;
			}
			// End of [#2219]
			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			/////////////////////////////////// VOICE GUIDANCE MODE ///////////////////////////////
			else
			{
				if( IsAdaTransaction() )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));
					arrVG.RemoveAll();

					int nValidADAInput = 0;
					if( GetKeyStr == L"2" )					// YES
					{
						arrVG.Add(L"2.wav");		arrVG.Add(L"YesSelect.wav");
						m_sUserSelection.nPrintReceipt = RCPT_PAPER;
						nValidADAInput = 1;
					}
					else if (GetKeyStr == L"4")				// NO
					{
						arrVG.Add(L"4.wav");		arrVG.Add(L"NoSelect.wav");
						m_sUserSelection.nPrintReceipt = FALSE;
						nValidADAInput = 1;
					}
					else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
					{
						nValidADAInput = 2;
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
					}
					
					if	( nValidADAInput == 0 )				// Invalid
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay( L"WrongKey.wav", FALSE);
							bInvalidRepeat = TRUE;
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						#endif
					}
					else if( nValidADAInput == 1)			// Valid Input
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(arrVG.GetAt(0), TRUE, arrVG.GetAt(1));
						#endif
						return RES_OK;
					}
					else if( nValidADAInput == 2)			// Repeat or Volume Control
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
				}
			}
			// End of [#2375]
		}
#endif

		Delay_Msg(50);

		// [#2375] US Justin 2015.11.03 US Justin US ADA
		#if(US_VERSION)
			if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
				if( P_NH_ADA_NeedMoreTime() == RES_OK )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
				else
					return RES_USER_EXIT;
			}
		#endif
		// End of [#2375]
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_18");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

// [#2309] US Justin 2014.11.14 Dual Balance Select Receipt
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_ReceiveReceiptWithoutQR()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Receive Receipt Without QR Code
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_ReceiveReceiptWithoutQR()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"RecRcptNOQR");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_ReceiveReceiptWithoutQR]\n"));

	// [#2413] US Justin 2016.04.12 VG for PIn4 and Popmoney
	if( IsAdaTransaction() )
		return RES_YES;
	// End of [#2413]

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
				// ICON
				m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_INFO);

				// Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_108005));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_108006));
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
			return RES_NO;
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
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_YES)
			{
				return RES_YES;
			}
			else if (GetKeyStr == S_NO)
			{
				return RES_NO;
			}
		}
#endif

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}
// [#2309]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_ErrorReceipt()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Receive Receipt
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_ErrorReceipt(BOOL bEnableDigitalReceipt)		// [#2350] US Justin 2015.06.17
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"ErrorReceipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_ErrorReceipt]\n"));

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// ADA Variable
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CStringArray arrVG;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// [#2375]

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_88");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2375]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if (US_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFile(116, L"116.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(116, L"LocateCancelKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(116, L"LocationRepeatKey.wav");				
				#endif
				m_pAdaCtrl->fnExp_PlayScreenWave(116);
				bStartToTimeout = TRUE;
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(138);							// [#2223] NH Justin 2013.10.17 Change screen Number for Error Receipt(US/CA/MX)

				// Setting Screen
				{
					// History
					for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
					{
						if (nHistory == 0)
							m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

						m_pDevCmn->fnSCR_DisplayHistory(nHistory+1, m_arHistory[nHistory]);
					}

					// ICON
					m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_RECEIPT_ERR);

					// Title
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_138001));	// [#2223] NH Justin 2013.10.17 Change screen Number for Error Receipt(US/CA/MX)

					// Sub Title
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_138002));	// [#2223] NH Justin 2013.10.17 Change screen Number for Error Receipt(US/CA/MX)

					// Digital Receipt
					// [#2350] NH Justin Add Digital Receipt option.
					if( bEnableDigitalReceipt == TRUE )
					{
						// [#2537] US Justin 2018.03.07 Combine routines
						/*
						// [#2263] CA Justin 2014.05.05 Digital Receipt Option
						//m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_138003));	// [#2223] NH Justin 2013.10.17 Change screen Number for Error Receipt(US/CA/MX)
						#if (CA_VERSION)
							if( ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DIGITAL_RECEIPT_OPTION)!=0)&&(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND).Left(6)==L"NH2700")) ||
								(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND)==L"NH1500SE")   )		// [#2346] Disable Digital Receipt on 1500SE
							{
								m_pDevCmn->fnSCR_DisplayImage(5, L"off");												// Disalbe Digital Receipt Button Icon
							}
							else
							{
								m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_138003));
								m_pDevCmn->fnSCR_DisplayImage(5, L"on");												// Enable Digital Receipt Button Icon
							}
						#elif (US_VERSION || MX_VERSION)			// [#2346] US Justin Disable QR Receipt for 1500SE
							//[#2292] US Justin 2014.10.10 Printer Error => QR Receipt is not available for "Dual Balance"
							if( MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND)==L"NH1500SE" )	// [#2346] Disable Digital Receipt on 1500SE
							{
								m_pDevCmn->fnSCR_DisplayImage(5, FALSE);												// Disalbe Digital Receipt Button Icon
							}
							else
							{
								// [#2518] US Kook 2018.01.22 Support MX-2800SE
								if (m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_MX2800SE)
								{
									// replace line separator to space from 'Digital\nReceipt'.
									CString strFdkValue = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_138003);
									strFdkValue.Replace('\n', ' ');
									m_pDevCmn->fnSCR_DisplayString(4, strFdkValue);
								}
								else
								// end of [#2518]
								{
									m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_138003));
								}
								m_pDevCmn->fnSCR_DisplayImage(5, TRUE);													// Enable Digital Receipt Button Icon
							}
							// End of [#2292]
						#else
							m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_138003));	// [#2223] NH Justin 2013.10.17 Change screen Number for Error Receipt(AU/MX)
							m_pDevCmn->fnSCR_DisplayImage(5, TRUE);														// Enable Digital Receipt Button Icon
						#endif
						// End [#2263]
						*/

						BOOL bShowDigitalReceipt = TRUE;
						#if (CA_VERSION)
							if( ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DIGITAL_RECEIPT_OPTION)!=0)&&(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND).Left(6)==L"NH2700")) ||
								(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND)==L"NH1500SE")   )	
								bShowDigitalReceipt = FALSE;
						#elif (AU_VERSION)							// Digital Receipt is disabled AU Only
							if((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DIGITAL_RECEIPT_OPTION) == 0))			
								bShowDigitalReceipt = FALSE;
						#else										// US or Mexico - Disable QR Receipt for 1500SE
							if( MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND)==L"NH1500SE" )	
								bShowDigitalReceipt = FALSE;
						#endif

						if(bShowDigitalReceipt)
						{
							m_pDevCmn->fnSCR_DisplayImage(5, TRUE);													// Enable Digital Receipt Button Icon
							if (GetConfigFuncPointer()->SupportsCamera())		// [#GLDV-2505] Support MX-2800T
								m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_138103));	// Digital Receipt
							else
								m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_138003));	// Digital \n Receuot
						}
						else
						{
							m_pDevCmn->fnSCR_DisplayImage(5, FALSE);												// Disalbe Digital Receipt Button Icon
						}
						// end of [#2537]
					}
					else
					{
						m_pDevCmn->fnSCR_DisplayImage(5, FALSE);													// Disalbe Digital Receipt Button Icon
					}
					// End of [#2350]

					// No Receipt
					m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_138004));	// [#2223] NH Justin 2013.10.17 Change screen Number for Error Receipt(US/CA/MX)		
					// end of [#2219] 2013.09.04
				}
				m_pDevCmn->fnSCR_DisplayScreen(138, KEYIN_TIME_OUT, PIN_MENU_MODE);								// [#2223] NH Justin 2013.10.17 Change screen Number for Error Receipt(US/CA/MX)
			}
			// End of [#2375]

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
		
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2375]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2375]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				// [#2375] US Justin Combine ADA and Screen Flow
				if( IsAdaTransaction())		
				{
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2375]
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_19");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_20");	// [#2024] NH KSK 2011.02.24
				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_YES)
			{
				m_sUserSelection.nPrintReceipt = FALSE;			// [#2219] 2013.09.04 Justin Digital Receipt, Change Variable Name

				// [#2220] AU KMK 2014.02.14 호주는 조회 거래 중 No Receipt 선택 시 'Balance details ...' 화면 호출
				#if (AU_VERSION)
					if (TranCode == TC_INQUIRY)
						return P_AU_C_NOR_ErrorReceipt();
					else
						return RES_OK;
				#else
					// end of [#2220]
					return RES_OK;
				#endif
			}
			else if (GetKeyStr == S_NO)
			{
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_21");	// [#2024] NH KSK 2011.02.24

				return RES_USER_EXIT;
			}
			// [#2219] 2013.09.04 JUSTIN Digital Receipt
			else if (GetKeyStr == S_QRCODE)
			{
				m_sUserSelection.nPrintReceipt = RCPT_QRCODE;
				return RES_OK;
			}
			// End of [#2219] 2013.09.04
			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			/////////////////////////////////// VOICE GUIDANCE MODE ///////////////////////////////
			else
			{
				if( IsAdaTransaction() )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));
					arrVG.RemoveAll();

					int nValidADAInput = 0;
					if( GetKeyStr == L"2" )					// YES
					{
						arrVG.Add(L"2.wav");		arrVG.Add(L"YesSelect.wav");
						m_sUserSelection.nPrintReceipt = FALSE;;
						nValidADAInput = 1;
					}
					else if (GetKeyStr == L"4")				// NO
					{
						arrVG.Add(L"4.wav");		arrVG.Add(L"NoSelect.wav");
						m_sUserSelection.nPrintReceipt = FALSE;
						nValidADAInput = 3;
					}
					else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
					{
						nValidADAInput = 2;
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
					}
					
					if	( nValidADAInput == 0 )				// Invalid
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
							bInvalidRepeat = TRUE;
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						#endif
					}
					else if( (nValidADAInput==1)||(nValidADAInput==3) )		// Valid Input
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(arrVG.GetAt(0), TRUE, arrVG.GetAt(1));
						#endif
						if(nValidADAInput==1)			return RES_OK;
						else							return RES_USER_EXIT;
					}
					else if( nValidADAInput == 2)			// Repeat or Volume Control
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
				}
			}
			// End of [#2375]
		}
#endif

		Delay_Msg(50);

		// [#2375] US Justin 2015.11.03 US Justin US ADA
		#if(US_VERSION)
			if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
				if( P_NH_ADA_NeedMoreTime() == RES_OK )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
				else
					return RES_USER_EXIT;
			}
		#endif
		// End of [#2375]
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_22");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_SelectCWAccount()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Select Cash Withdrawal Account
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_SelectCWAccount()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"SelectCWAccount");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_SelectCWAccount]\n"));

	//[#2000] SOOK 2010.10.16 호주향 개발 (거래 FLOW)
	#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
		//checking 만 선택되어 있을 경우 계좌선택 화면을 보여주지 않음 
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS) == 0
			&& MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD) == 0)
		{
			m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
			m_SourceAccount = S_CHECKING;
			return RES_OK;
		}
	#endif
	//end of [#2000]

	int				nIndex;
	CString			GetKeyStr;
	BOOL			bShowScreen = TRUE;

	// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// ADA Variable
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CStringArray arrVG;

	// Check Available  Accounts
	BOOL bCheckingAvail = FALSE;
	BOOL bSavingsAvail = FALSE;
	BOOL bCreditAvail = FALSE;
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING))		bCheckingAvail = TRUE;
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS))		bSavingsAvail = TRUE;
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD))	bCreditAvail = TRUE;

	// [#2518] US Kook 2018.01.22 Support MX-2800SE
	int		nIdxOfFdk						= 0;
	CString	strValueOfFdk[MENU_SELECT_SIZE]	= { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };
	// end of [#2518]

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// [#2375]

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_92");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2375]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if (US_VERSION)
					if ( (bCheckingAvail) || (TranCode == TC_POPMONEY) )		// [#2413] US Justin 2016.04.13 VG for Popmoney and Pin4
					{
						m_pAdaCtrl->fnExp_AddWaveFile(202, L"For.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(202, L"With_FromChecking.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(202, L"Press2.wav");
					}
					if ( (bSavingsAvail) || (TranCode == TC_POPMONEY) )			// [#2413] US Justin 2016.04.13 VG for Popmoney and Pin4
					{
						m_pAdaCtrl->fnExp_AddWaveFile(202, L"For.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(202, L"With_FromSavings.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(202, L"Press4.wav");
					}
					if ( (bCreditAvail) && (TranCode != TC_POPMONEY) )			// [#2413] US Justin 2016.04.13 VG for Popmoney and Pin4
					{
						m_pAdaCtrl->fnExp_AddWaveFile(202, L"For.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(202, L"Cash_FromCredit.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(202, L"Press6.wav");
					}
					m_pAdaCtrl->fnExp_AddWaveFile(202, L"LocateCancelKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(202, L"LocationRepeatKey.wav");				
				#else
					if (bCheckingAvail)	m_pAdaCtrl->fnExp_AddWaveFile(202, L"Press2ForChecking.wav");	// Checking
					if (bSavingsAvail)	m_pAdaCtrl->fnExp_AddWaveFile(202, L"Press4ForSavings.wav");	// Savings
					if (bCreditAvail)	m_pAdaCtrl->fnExp_AddWaveFile(202, L"Press6ForCredit.wav");		// CreditCard
				#endif
				m_pAdaCtrl->fnExp_PlayScreenWave(202);
				bStartToTimeout = TRUE;
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(111);

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
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_111001));

					// Sub Title
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_111101));

					nIndex = 4;
					nIdxOfFdk = 0;

					// [#2529] NH Justin 2018.02.16 Remove Graphic Button
					/*
					// [#2518] US Kook 2018.01.19 Support MX-2800SE
					if (m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_MX2800SE)
					{
						nIdxOfFdk++;		// starts from 'F2 (1)'
						if ( (bCheckingAvail) || (TranCode == TC_POPMONEY) )
						{
							m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
							strValueOfFdk[nIdxOfFdk] = _T("CHECKING");
							nIdxOfFdk += 2;
						}
						if ( (bSavingsAvail) || (TranCode == TC_POPMONEY) )
						{
							m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SAVING));
							strValueOfFdk[nIdxOfFdk] = _T("SAVING");
							nIdxOfFdk += 2;
						}
						if ( (bCreditAvail) && (TranCode != TC_POPMONEY) )
						{
							m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CREDIT));
							strValueOfFdk[nIdxOfFdk] = _T("CREDIT");
						}
					}
					else
					// end of [#2518]
					{
						// Checking Account En/Disable
						if ( (bCheckingAvail) || (TranCode == TC_POPMONEY) )	// [#2350] US Justin 2015.06.17 POP Money shoud provide "checking"
							m_pDevCmn->fnSCR_DisplayString(nIndex++, L"CHECKING");
						// Saving Account En/Disable
						if ( (bSavingsAvail) || (TranCode == TC_POPMONEY) )		// [#2350] US Justin 2015.06.17 POP Money shoud provide "Saving"
							m_pDevCmn->fnSCR_DisplayString(nIndex++, L"SAVING");
						// Credit Card Account En/Disable
						if ( (bCreditAvail) && (TranCode != TC_POPMONEY) )		// [#2350] US Justin 2015.06.17 POP Money shoud not provide "Credit"
							m_pDevCmn->fnSCR_DisplayString(nIndex++, L"CREDIT");
					}
					*/

					nIdxOfFdk++;		// starts from 'F2 (1)'
					if ( (bCheckingAvail) || (TranCode == TC_POPMONEY) )
					{
						m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
						strValueOfFdk[nIdxOfFdk] = _T("CHECKING");
						nIdxOfFdk += 2;
					}
					if ( (bSavingsAvail) || (TranCode == TC_POPMONEY) )
					{
						m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SAVING));
						strValueOfFdk[nIdxOfFdk] = _T("SAVING");
						nIdxOfFdk += 2;
					}
					if ( (bCreditAvail) && (TranCode != TC_POPMONEY) )
					{
						m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CREDIT));
						strValueOfFdk[nIdxOfFdk] = _T("CREDIT");
					}
					// End of [#2529]
				}

				m_pDevCmn->fnSCR_DisplayScreen(111, KEYIN_TIME_OUT, PIN_MENU_MODE);
			}
			// End of [#2375]

			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
		}

#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
			m_SourceAccount = S_CHECKING;
			return RES_OK;
		}
#else
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2375]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{	
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2375]

			// [#2518] US Kook 2018.01.22 Support MX-2800SE
			// converting "F#" to "#VALUE#"
			if (GetKeyStr == L"F1" || GetKeyStr == L"F2" || GetKeyStr == L"F3"
				|| GetKeyStr == L"F4" || GetKeyStr == L"F5" || GetKeyStr == L"F6")
			{
				int nPressedBtn = Asc2Int(GetKeyStr.Right(1)) - 1;		// F1, F2, ... => 0, 1, ...
				if( (nPressedBtn>=0) && (nPressedBtn<MENU_SELECT_SIZE) )
					GetKeyStr = strValueOfFdk[nPressedBtn];
			}
			// end of [#2518]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
					m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_FROM_ACCOUNT, L"USER CANCEL");
				#endif //end of [#2000]

				// [#2375] US Justin Combine ADA and Screen Flow
				if( IsAdaTransaction())		
				{
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2375]

				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_23");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				//[#2000] SOOK 2010.10.16 호주향 개발 (거래 FLOW)
				#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
					if ( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME) == 0
						||  P_NH_NOR_NeedMoreTime() != RES_YES )
					{
						m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_FROM_ACCOUNT, L"TIMEOUT");
						NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
						NVDump('O', 'C', "00", L"P_NHNOR", L"NG_24_1");	// [#2024] NH KSK 2011.02.24
						return RES_USER_TIMEOUT;
					}
					bShowScreen = TRUE;
				#else
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_24");	// [#2024] NH KSK 2011.02.24
					return RES_USER_TIMEOUT;
				#endif //end of [#2000]
			}
			else if (GetKeyStr == L"CHECKING")
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
				m_SourceAccount = S_CHECKING;
				return RES_OK;
			}
			else if (GetKeyStr == L"SAVING")
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SAVING));
				m_SourceAccount = S_SAVINGS;
				return RES_OK;
			}
			else if (GetKeyStr == L"CREDIT")
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CREDIT));
				m_SourceAccount = S_CREDITCARD;
				return RES_OK;
			}
			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			/////////////////////////////////// VOICE GUIDANCE MODE ///////////////////////////////
			else
			{
				if( IsAdaTransaction() )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));
					arrVG.RemoveAll();

					int nValidADAInput = 0;
					//if( (GetKeyStr == L"2")&&(bCheckingAvail) )					// Checking
					if( (GetKeyStr == L"2")&&( (bCheckingAvail)||(TranCode == TC_POPMONEY) ) )					// [#2413] US Justin 2016.04.13 VG for Popmoney and Pin4
					{
						arrVG.Add(L"2.wav");		arrVG.Add(L"With_FromChecking.wav");
						m_SourceAccount = S_CHECKING;
						nValidADAInput = 1;
					}
					//else if ( (GetKeyStr == L"4")&&(bSavingsAvail)	)			// Saving
					else if ( (GetKeyStr == L"4")&&( (bSavingsAvail) || (TranCode == TC_POPMONEY) )	)			// [#2413] US Justin 2016.04.13 VG for Popmoney and Pin4
					{
						arrVG.Add(L"4.wav");		arrVG.Add(L"With_FromSavings.wav");
						m_SourceAccount = S_SAVINGS;
						nValidADAInput = 1;
					}
					//else if ( (GetKeyStr == L"6")&&(bCreditAvail)	)			// Credit
					else if ( (GetKeyStr == L"6")&&( (bCreditAvail) && (TranCode != TC_POPMONEY) )	)			// [#2413] US Justin 2016.04.13 VG for Popmoney and Pin4
					{
						arrVG.Add(L"6.wav");		arrVG.Add(L"Cash_FromCredit.wav");
						m_SourceAccount = S_CREDITCARD;
						nValidADAInput = 1;
					}
					else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
					{
						nValidADAInput = 2;
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
					}
					
					if	( nValidADAInput == 0 )				// Invalid
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
							bInvalidRepeat = TRUE;
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						#endif
					}
					else if(nValidADAInput==1)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(arrVG.GetAt(0), TRUE, arrVG.GetAt(1));
						#endif
						return RES_OK;
					}
					else if( nValidADAInput == 2)			// Repeat or Volume Control
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
				}
			}
			// End of [#2375]
		}
#endif

		Delay_Msg(50);

		// [#2375] US Justin 2015.11.03 US Justin US ADA
		#if(US_VERSION)
			if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
				if( P_NH_ADA_NeedMoreTime() == RES_OK )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
				else
					return RES_USER_EXIT;
			}
		#endif
		// End of [#2375]
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_25");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_SelectBIAccount()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Select Balance Inquiry Account
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_SelectBIAccount()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"SelectBIAccount");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_SelectBIAccount]\n"));

	//[#2000] SOOK 2010.10.16 호주향 개발 (거래 FLOW)
	#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
		//checking 만 선택되어 있을 경우 계좌선택 화면을 보여주지 않음 
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS) == 0
			&& MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD) == 0)
		{
			m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
			m_SourceAccount = S_CHECKING;
			return RES_OK;
		}
	#endif
	//end of [#2000]

	int			nIndex;
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// ADA Variable
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CStringArray arrVG;

	// Check Available  Accounts
	BOOL bCheckingAvail = FALSE;
	BOOL bSavingsAvail = FALSE;
	BOOL bCreditAvail = FALSE;
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING))		bCheckingAvail = TRUE;
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS))		bSavingsAvail = TRUE;
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD))	bCreditAvail = TRUE;

	// Disable CATM Credit account for ADA.
	// The credit account is not available on the "better balance inquiry" screen, so CATM has
	// requested the removal of the credit account from the voice guidance on the balance inquiry screen.
	// NOTE: The better-balance inquiry screen does not have VG, so this screen is used instead.
#if (APP_TDL_OPTION)
	if (IsAdaTransaction() && MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE)==ENABLE) 
	{
		// Disable credit account when 1) Cardtronics, 2) ADA txn, and 3) dual balance is enabled
		bCreditAvail = FALSE;
	}
#endif

	// [#2518] US Kook 2018.01.22 Support MX-2800SE
	int		nIdxOfFdk						= 0;
	CString	strValueOfFdk[MENU_SELECT_SIZE]	= { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };
	// end of [#2518]

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// [#2375]

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_95");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2375]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if (US_VERSION)
					if (bCheckingAvail)
					{
						m_pAdaCtrl->fnExp_AddWaveFile(301, L"For.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(301, L"Inquiry_FromChecking.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(301, L"Press2.wav");
					}
					if (bSavingsAvail)
					{
						m_pAdaCtrl->fnExp_AddWaveFile(301, L"For.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(301, L"Inquiry_FromSavings.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(301, L"Press4.wav");
					}
					if (bCreditAvail)
					{
						m_pAdaCtrl->fnExp_AddWaveFile(301, L"For.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(301, L"Inquiry_FromCredit.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(301, L"Press6.wav");
					}
					m_pAdaCtrl->fnExp_AddWaveFile(301, L"LocateCancelKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(301, L"LocationRepeatKey.wav");				
				#else
					if (bCheckingAvail)	m_pAdaCtrl->fnExp_AddWaveFile(301, L"Press2ForChecking.wav");	// Checking
					if (bSavingsAvail)	m_pAdaCtrl->fnExp_AddWaveFile(301, L"Press4ForSavings.wav");	// Savings
					if (bCreditAvail)	m_pAdaCtrl->fnExp_AddWaveFile(301, L"Press6ForCredit.wav");		// CreditCard
				#endif
				m_pAdaCtrl->fnExp_PlayScreenWave(301);
				bStartToTimeout = TRUE;
			}
			else
			{
				// [#2309] US Justin Balance Fee Notice
			#if (US_VERSION)
				int nScrNum = 144;
			#else
				int nScrNum = 111;
			#endif
				// end of [#2309]

				m_pDevCmn->fnSCR_DisplayPrevSet(nScrNum);						// [#2309] US Justin Balance Fee Notice

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
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_111001));

					// Sub Title
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_111201));

					nIndex = 4;
					nIdxOfFdk = 0;

					// [#2529] NH Justin 2018.02.21 Add 2800 SE
					/*
					// [#2518] US Kook 2018.01.19 Support MX-2800SE
					if (m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_MX2800SE)
					{
						nIdxOfFdk++;		// starts from 'F2 (1)'
						if (bCheckingAvail)
						{
							m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
							strValueOfFdk[nIdxOfFdk] = _T("CHECKING");
							nIdxOfFdk += 2;
						}
						if (bSavingsAvail)
						{
							m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SAVING));
							strValueOfFdk[nIdxOfFdk] = _T("SAVING");
							nIdxOfFdk += 2;
						}
						if (bCreditAvail)
						{
							m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CREDIT));
							strValueOfFdk[nIdxOfFdk] = _T("CREDIT");
						}
					}
					else
					// end of [#2518]
					{
						if (bCheckingAvail)		m_pDevCmn->fnSCR_DisplayString(nIndex++, L"CHECKING");
						if (bSavingsAvail)		m_pDevCmn->fnSCR_DisplayString(nIndex++, L"SAVING");
						if (bCreditAvail)		m_pDevCmn->fnSCR_DisplayString(nIndex++, L"CREDIT");
					}
					*/

					nIdxOfFdk++;		// starts from 'F2 (1)'
					if (bCheckingAvail)
					{
						m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
						strValueOfFdk[nIdxOfFdk] = _T("CHECKING");
						nIdxOfFdk += 2;
					}
					if (bSavingsAvail)
					{
						m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SAVING));
						strValueOfFdk[nIdxOfFdk] = _T("SAVING");
						nIdxOfFdk += 2;
					}
					if (bCreditAvail)
					{
						m_pDevCmn->fnSCR_DisplayString(nIndex++, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CREDIT));
						strValueOfFdk[nIdxOfFdk] = _T("CREDIT");
					}
					// End of [#2529]

					// [#2309] US Justin Balance Fee Notice
					#if (US_VERSION)
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_BALANCE_NOFEE_NOTICE) == ENABLE)
					{
						m_pDevCmn->fnSCR_DisplayImage(7, TRUE);
						m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_143010));
					}
					else
					{
						m_pDevCmn->fnSCR_DisplayImage(7, FALSE);				
					}
					#endif
					// End of [#2309]
				}
				m_pDevCmn->fnSCR_DisplayScreen(nScrNum, KEYIN_TIME_OUT, PIN_MENU_MODE);			// [#2309] US Justin Balance Fee Notice
			}
			// End of [#2375]

			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
		}

#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
			m_SourceAccount = S_CHECKING;
			return RES_OK;
		}
#else

		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2375]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2375]

			// [#2518] US Kook 2018.01.22 Support MX-2800SE
			// converting "F#" to "#VALUE#"
			if (GetKeyStr == L"F1" || GetKeyStr == L"F2" || GetKeyStr == L"F3"
				|| GetKeyStr == L"F4" || GetKeyStr == L"F5" || GetKeyStr == L"F6")
			{
				int nPressedBtn = Asc2Int( GetKeyStr.Right(1) ) - 1;		// F1, F2, ... => 0, 1, ...
				if( (nPressedBtn>=0) && (nPressedBtn<MENU_SELECT_SIZE) )
					GetKeyStr = strValueOfFdk[nPressedBtn];
			}
			// end of [#2518]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
					m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_FROM_ACCOUNT, L"USER CANCEL");
				#endif //end of [#2000]
				
				// [#2375] US Justin Combine ADA and Screen Flow
				if( IsAdaTransaction())		
				{
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2375]

				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_26");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				//[#2000] SOOK 2010.10.16 호주향 개발 (거래 FLOW)
				#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
					if ( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME) == 0
						||  P_NH_NOR_NeedMoreTime() != RES_YES )
					{
						m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_FROM_ACCOUNT, L"TIMEOUT");
						NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
						NVDump('O', 'C', "00", L"P_NHNOR", L"NG_27_1");	// [#2024] NH KSK 2011.02.24
						return RES_USER_TIMEOUT;
					}
					bShowScreen = TRUE;
				#else
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_27");	// [#2024] NH KSK 2011.02.24
					return RES_USER_TIMEOUT;
				#endif //end of [#2000]
			}
			else if (GetKeyStr == L"CHECKING")
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
				m_SourceAccount = S_CHECKING;
				return RES_OK;
			}
			else if (GetKeyStr == L"SAVING")
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SAVING));
				m_SourceAccount = S_SAVINGS;
				return RES_OK;
			}
			else if (GetKeyStr == L"CREDIT")
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CREDIT));
				m_SourceAccount = S_CREDITCARD;
				return RES_OK;
			}
			// [#2309] US Justin Balance Fee Notice
			#if (US_VERSION)
			else if (GetKeyStr == L"MAINMENU")	// Define in Screen 144
			{
				return RES_NO;
			}
			#endif
			// End of [#2309]
			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			/////////////////////////////////// VOICE GUIDANCE MODE ///////////////////////////////
			else
			{
				if( IsAdaTransaction() )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));
					arrVG.RemoveAll();

					int nValidADAInput = 0;
					if( (GetKeyStr == L"2")&&(bCheckingAvail) )					// Checking
					{
						arrVG.Add(L"2.wav");		arrVG.Add(L"Inquiry_FromChecking.wav");
						m_SourceAccount = S_CHECKING;
						nValidADAInput = 1;
					}
					else if ( (GetKeyStr == L"4")&&(bSavingsAvail)	)			// Saving
					{
						arrVG.Add(L"4.wav");		arrVG.Add(L"Inquiry_FromSavings.wav");
						m_SourceAccount = S_SAVINGS;
						nValidADAInput = 1;
					}
					else if ( (GetKeyStr == L"6")&&(bCreditAvail)	)			// Credit
					{
						arrVG.Add(L"6.wav");		arrVG.Add(L"Inquiry_FromCredit.wav");
						m_SourceAccount = S_CREDITCARD;
						nValidADAInput = 1;
					}
					else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
					{
						nValidADAInput = 2;
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
					}
										
					if	( nValidADAInput == 0 )				// Invalid
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
							bInvalidRepeat = TRUE;
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						#endif
					}
					else if(nValidADAInput==1)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(arrVG.GetAt(0), TRUE, arrVG.GetAt(1));
						#endif
						return RES_OK;
					}
					else if( nValidADAInput == 2)			// Repeat or Volume Control
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
				}
			}
			// End of [#2375]
		}
#endif

		Delay_Msg(50);

		// [#2375] US Justin 2015.11.03 US Justin US ADA
		#if(US_VERSION)
			if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
				if( P_NH_ADA_NeedMoreTime() == RES_OK )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
				else
					return RES_USER_EXIT;
			}
		#endif
		// End of [#2375]
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_28");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_SelectTRAccount()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Select Transfer Account
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_SelectTRAccount()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_SelectTRAccount]\n"));
	
	CString		GetKeyStr;
	BOOL		bShowScreen;

	// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
	// Voice Guidance Variables
	BOOL bStartToTimeout = FALSE;
	BOOL bCheckingAvail = FALSE;
	BOOL bSavingsAvail = FALSE;
	BOOL bCreditAvail = FALSE;
	BOOL bInvalidRepeat = FALSE;
	CStringArray arrVG;
	// End of [#2375]

	// [#2518] US Kook 2018.01.25 Support MX-2800SE
	//int		nIdxOfFdk						= 0;		// [#J001] US Justin 2018.10.18 Transfer Account Bug Fix
	CString	strValueOfFdk[MENU_SELECT_SIZE]	= { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };
	// end of [#2518]

	///////////////////////////////////
	// From Account
	{
		NVDump('O', 'C', "00", L"P_NHNOR", L"FromAccount");

		bShowScreen = TRUE;

		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
		long nScrTimeOut = GetTransactionScreenTimeOut();
		if(nScrTimeOut==0)							// ADA Mode....Jack removed.
			return RES_USER_EXIT;

		// ADA Variable
		bStartToTimeout = FALSE;
		bInvalidRepeat = FALSE;

		// Check Available  Accounts
		bCheckingAvail = FALSE;
		bSavingsAvail = FALSE;
		bCreditAvail = FALSE;
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING))		bCheckingAvail = TRUE;
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS))		bSavingsAvail = TRUE;
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD))	bCreditAvail = TRUE;

		NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
		g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
		// [#2375]

		while (g_TimeCheck.IsElapsedTimes() == FALSE)
		{
			///////////////////////////////////
			// CHECK STATUS
			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( IsAdaTransaction() ) 
			{
				if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
				{
					NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
					NVDump('O', 'C', "00", L"P_NHADA", L"NG_95");
					m_pAdaCtrl->fnExp_StopPlay();
					return RES_USER_EXIT;
				}

				if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
				{
					bInvalidRepeat = FALSE;
					bShowScreen = TRUE;
				}
			}
			// End of [#2375]

			///////////////////////////////////
			// DISPLAY SCREEN
			if (bShowScreen == TRUE)
			{
				NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

				// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
				if( IsAdaTransaction())
				{
					m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
					m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

					m_pAdaCtrl->fnExp_StopAndResetWaveFile();
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_AddWaveFile(402, L"402.wav");
						if (bCheckingAvail)
						{
							m_pAdaCtrl->fnExp_AddWaveFile(402, L"For.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(402, L"Checking.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(402, L"Press2.wav");
						}
						if (bSavingsAvail)
						{
							m_pAdaCtrl->fnExp_AddWaveFile(402, L"For.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(402, L"Savings.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(402, L"Press4.wav");
						}
						if (bCreditAvail)
						{
							m_pAdaCtrl->fnExp_AddWaveFile(402, L"For.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(402, L"Credit.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(402, L"Press6.wav");
						}
						m_pAdaCtrl->fnExp_AddWaveFile(402, L"LocateCancelKey.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(402, L"LocationRepeatKey.wav");				
					#else
						if (bCheckingAvail)	m_pAdaCtrl->fnExp_AddWaveFile(402, L"Press2ForChecking.wav");	// Checking
						if (bSavingsAvail)	m_pAdaCtrl->fnExp_AddWaveFile(402, L"Press4ForSavings.wav");	// Savings
						if (bCreditAvail)	m_pAdaCtrl->fnExp_AddWaveFile(402, L"Press6ForCredit.wav");		// CreditCard
					#endif
					m_pAdaCtrl->fnExp_PlayScreenWave(402);
					bStartToTimeout = TRUE;
				}
				else
				{
					m_pDevCmn->fnSCR_DisplayPrevSet(114);

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
						m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_114001));

						// Sub Title
						m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_114002));

						// [#2529] NH Justin 2018.02.20 Add 2800 SE
						/*
						// [#2518] US Kook 2018.01.25 Support MX-2800SE
						if (m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_MX2800SE)
						{
							nIdxOfFdk = 1;		// starts from 'F2 (1)'
							if (bCheckingAvail)
							{
								m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
								strValueOfFdk[nIdxOfFdk] = _T("CHECKING");
								nIdxOfFdk += 2;
							}
							if (bSavingsAvail)
							{
								m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SAVING));
								strValueOfFdk[nIdxOfFdk] = _T("SAVING");
								nIdxOfFdk += 2;
							}
							if (bCreditAvail)
							{
								m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CREDIT));
								strValueOfFdk[nIdxOfFdk] = _T("CREDIT");
							}
						}
						else
						// end of [#2518]
						{
							if (bCheckingAvail)		m_pDevCmn->fnSCR_DisplayString(4, L"CHECKING");
							if (bSavingsAvail)		m_pDevCmn->fnSCR_DisplayString(5, L"SAVING");
							if (bCreditAvail)		m_pDevCmn->fnSCR_DisplayString(6, L"CREDIT");
						}
						*/

						if (bCheckingAvail)		
						{
							// F2
							m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
							strValueOfFdk[1] = _T("CHECKING");	// [#J001] US Justin 2018.10.18 Transfer Account Bug Fix
						}

						if (bSavingsAvail)
						{
							// F4
							m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SAVING));
							strValueOfFdk[3] = _T("SAVING");	// [#J001] US Justin 2018.10.18 Transfer Account Bug Fix
						}

						if (bCreditAvail)
						{
							// F6
							m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CREDIT));
							strValueOfFdk[5] = _T("CREDIT");	// [#J001] US Justin 2018.10.18 Transfer Account Bug Fix
						}
						// End of [#2529]

					}

					m_pDevCmn->fnSCR_DisplayScreen(114, KEYIN_TIME_OUT, PIN_MENU_MODE);
				}
				// End of [#2375]

				bShowScreen = FALSE;

#ifdef APP_AGING_MODE
				g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
			}

#ifdef APP_AGING_MODE
			if (g_AgingCheck.IsElapsedTimes() == TRUE)
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
				m_SourceAccount = S_CHECKING;
				break;
			}
#else

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
			{
				NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
				g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
				bStartToTimeout = FALSE;
			}
			// End of [#2375]

			///////////////////////////////////
			// GET KEY STRING
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
				bInvalidRepeat = FALSE;
				if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
				{
					GetKeyStr = GetKeyStr.Mid(6);
					NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
				}
				// End of [#2375]

				// [#2518] US Kook 2018.01.25 Support MX-2800SE
				// converting "F#" to "#VALUE#"
				if ( GetKeyStr == L"F1" || GetKeyStr == L"F2" || GetKeyStr == L"F3" ||
					 GetKeyStr == L"F4" || GetKeyStr == L"F5" || GetKeyStr == L"F6")
				{
					int nPressedBtn = Asc2Int( GetKeyStr.Right(1) ) - 1;		// F1, F2, ... => 0, 1, ...
					if( (nPressedBtn>=0) && (nPressedBtn<MENU_SELECT_SIZE) )
						GetKeyStr = strValueOfFdk[nPressedBtn];
				}
				// end of [#2518]

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER FDK TO STRING CONVERSION =  [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					// [#2375] US Justin Combine ADA and Screen Flow
					if( IsAdaTransaction())		
					{
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
						#endif
					}
					// End of [#2375]

					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_29");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}
				else if (GetKeyStr == S_TIMEOVER)
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_30");	// [#2024] NH KSK 2011.02.24
					return RES_USER_TIMEOUT;
				}
				else if (GetKeyStr == L"CHECKING")
				{
					m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
					m_SourceAccount = S_CHECKING;
					break;
				}
				else if (GetKeyStr == L"SAVING")
				{
					m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SAVING));
					m_SourceAccount = S_SAVINGS;
					break;
				}
				else if (GetKeyStr == L"CREDIT")
				{
					m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CREDIT));
					m_SourceAccount = S_CREDITCARD;
					break;
				}
				// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
				/////////////////////////////////// VOICE GUIDANCE MODE ///////////////////////////////
				else
				{
					if( IsAdaTransaction() )
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));
						arrVG.RemoveAll();

						int nValidADAInput = 0;
						if( (GetKeyStr == L"2")&&(bCheckingAvail) )					// Checking
						{
							arrVG.Add(L"2.wav");		arrVG.Add(L"Checking.wav");
							m_SourceAccount = S_CHECKING;
							nValidADAInput = 1;
						}
						else if ( (GetKeyStr == L"4")&&(bSavingsAvail)	)			// Saving
						{
							arrVG.Add(L"4.wav");		arrVG.Add(L"Savings.wav");
							m_SourceAccount = S_SAVINGS;
							nValidADAInput = 1;
						}
						else if ( (GetKeyStr == L"6")&&(bCreditAvail)	)			// Credit
						{
							arrVG.Add(L"6.wav");		arrVG.Add(L"Credit.wav");
							m_SourceAccount = S_CREDITCARD;
							nValidADAInput = 1;
						}
						else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
						{
							nValidADAInput = 2;
							m_pAdaCtrl->fnExp_StopPlay();
							if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
							else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
						}				
						
						if	( nValidADAInput == 0 )				// Invalid
						{
							NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
							#if (US_VERSION)
								m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
								bInvalidRepeat = TRUE;
								g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
							#endif
						}
						else if(nValidADAInput==1)
						{
							NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
							#if (US_VERSION)
								m_pAdaCtrl->fnExp_ResetAndAddPlay(arrVG.GetAt(0), TRUE, arrVG.GetAt(1));
							#endif
							break;
						}
						else if( nValidADAInput == 2)			// Repeat or Volume Control
						{
							NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
							bShowScreen = TRUE;
						}
					}
				}
				// End of [#2375]

			}
#endif

			Delay_Msg(50);

			// [#2375] US Justin 2015.11.03 US Justin US ADA
			#if(US_VERSION)
				if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
					if( P_NH_ADA_NeedMoreTime() == RES_OK )
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
					else
						return RES_USER_EXIT;
				}
			#endif
			// End of [#2375]

		}

		if (g_TimeCheck.IsElapsedTimes() == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
			NVDump('O', 'C', "00", L"P_NHNOR", L"NG_31");	// [#2024] NH KSK 2011.02.24
			return RES_USER_TIMEOUT;
		}
	}

	///////////////////////////////////
	// To Account
	{
		NVDump('O', 'C', "00", L"P_NHNOR", L"ToAccount");

		bShowScreen = TRUE;

		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
		long nScrTimeOut = GetTransactionScreenTimeOut();
		if(nScrTimeOut==0)							// ADA Mode....Jack removed.
			return RES_USER_EXIT;

		// ADA Variable
		bStartToTimeout = FALSE;
		bInvalidRepeat = FALSE;

		// Check Available  Accounts
		bCheckingAvail = FALSE;
		bSavingsAvail = FALSE;
		bCreditAvail = FALSE;
		if (m_SourceAccount == S_CHECKING)
		{
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS))
				bSavingsAvail = TRUE;

			if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) != MSG_TRITON_TYPE)
			{
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD))
					bCreditAvail = TRUE;
			}
		}
		else if (m_SourceAccount == S_SAVINGS)
		{
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING))
				bCheckingAvail = TRUE;

			if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) != MSG_TRITON_TYPE)
			{
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD))
					bCreditAvail = TRUE;
			}
		}
		else if (m_SourceAccount == S_CREDITCARD)
		{
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING))
				bCheckingAvail = TRUE;

			if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) != MSG_TRITON_TYPE)
			{
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS))
					bSavingsAvail = TRUE;
			}		
		}

		NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
		g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
		// [#2375]

		while (g_TimeCheck.IsElapsedTimes() == FALSE)
		{
			///////////////////////////////////
			// CHECK STATUS
			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( IsAdaTransaction() ) 
			{
				if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
				{
					NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
					NVDump('O', 'C', "00", L"P_NHADA", L"NG_101");
					m_pAdaCtrl->fnExp_StopPlay();
					return RES_USER_EXIT;
				}

				if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
				{
					bInvalidRepeat = FALSE;
					bShowScreen = TRUE;
				}
			}
			// End of [#2375]

			///////////////////////////////////
			// DISPLAY SCREEN
			if (bShowScreen == TRUE)
			{
				NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

				// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
				if( IsAdaTransaction())
				{
					m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
					m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

					m_pAdaCtrl->fnExp_StopAndResetWaveFile();
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_AddWaveFile(403, L"403.wav");
						if (bCheckingAvail)
						{
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"For.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"Checking.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"Press2.wav");
						}
						if (bSavingsAvail)
						{
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"For.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"Savings.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"Press4.wav");
						}
						if (bCreditAvail)
						{
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"For.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"Credit.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"Press6.wav");
						}
						m_pAdaCtrl->fnExp_AddWaveFile(403, L"LocateCancelKey.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(403, L"LocationRepeatKey.wav");				
					#else
						if (bCheckingAvail)	m_pAdaCtrl->fnExp_AddWaveFile(403, L"Press2ForChecking.wav");	// Checking
						if (bSavingsAvail)	m_pAdaCtrl->fnExp_AddWaveFile(403, L"Press4ForSavings.wav");	// Savings
						if (bCreditAvail)	m_pAdaCtrl->fnExp_AddWaveFile(403, L"Press6ForCredit.wav");		// CreditCard
					#endif
					m_pAdaCtrl->fnExp_PlayScreenWave(403);
					bStartToTimeout = TRUE;
				}
				else
				{
					m_pDevCmn->fnSCR_DisplayPrevSet(115);

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
						m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_115001));

						// Sub Title
						m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_115002));

						// ???? APValue4,5,6 are not defined on the screen ===> NEED to check whether they can be removed or not
						//if (m_SourceAccount == S_CHECKING)
						//	m_pDevCmn->fnSCR_DisplayString(4, L"CHECKING");
						//else if (m_SourceAccount == S_SAVINGS)
						//	m_pDevCmn->fnSCR_DisplayString(5, L"SAVING");
						//else if (m_SourceAccount == S_CREDITCARD)
						//	m_pDevCmn->fnSCR_DisplayString(6, L"CREDIT");
						// ???? APValue4,5,6 are not defined on the screen ===> NEED to check whether they can be removed or not

						// [#2375] US Justin 2015.10.28 Use Variables
						/*
						if (m_SourceAccount == S_CHECKING)
						{
							// Saving Account En/Disable
							if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS) == ENABLE)
								m_pDevCmn->fnSCR_DisplayString(8, L"SAVING");

							// Credit Card Account En/Disable
							if ((MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD) == ENABLE) &&
								(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) != MSG_TRITON_TYPE))
								m_pDevCmn->fnSCR_DisplayString(9, L"CREDIT");
						}
						else if (m_SourceAccount == S_SAVINGS)
						{
							// Checking Account En/Disable
							if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING) == ENABLE)
								m_pDevCmn->fnSCR_DisplayString(7, L"CHECKING");

							// Credit Card Account En/Disable
							if ((MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD) == ENABLE) &&
								(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) != MSG_TRITON_TYPE))
								m_pDevCmn->fnSCR_DisplayString(9, L"CREDIT");

						}
						else if (m_SourceAccount == S_CREDITCARD)
						{
							// Checking Account En/Disable
							if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING) == ENABLE)
								m_pDevCmn->fnSCR_DisplayString(7, L"CHECKING");

							// Saving Account En/Disable
							if ((MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS) == ENABLE) &&
								(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) != MSG_TRITON_TYPE))
								m_pDevCmn->fnSCR_DisplayString(8, L"SAVING");
						}
						*/

						// [#2529] NH Justin 2018.02.21 Add 2800 SE
						/*
						// [#2518] US Kook 2018.01.25 Support MX-2800SE
						// It seems that 114 and 115 screens are same... why we use 2 screens?
						if (m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_MX2800SE)
						{
							nIdxOfFdk = 1;		// starts from 'F2 (1)'
							if (bCheckingAvail)
							{
								m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
								strValueOfFdk[nIdxOfFdk] = _T("CHECKING");
								nIdxOfFdk += 2;
							}
							if (bSavingsAvail)
							{
								m_pDevCmn->fnSCR_DisplayString(8, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SAVING));
								strValueOfFdk[nIdxOfFdk] = _T("SAVING");
								nIdxOfFdk += 2;
							}
							if (bCreditAvail)
							{
								m_pDevCmn->fnSCR_DisplayString(9, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CREDIT));
								strValueOfFdk[nIdxOfFdk] = _T("CREDIT");
							}
						}
						else
						// end of [#2518]
						{
							if (bCheckingAvail)		m_pDevCmn->fnSCR_DisplayString(7, L"CHECKING");	// Checking
							if (bSavingsAvail)		m_pDevCmn->fnSCR_DisplayString(8, L"SAVING");	// Saving
							if (bCreditAvail)		m_pDevCmn->fnSCR_DisplayString(9, L"CREDIT");	// Credit
						}
						*/

						if (bCheckingAvail)
						{
							// F2
							m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
							strValueOfFdk[1] = _T("CHECKING");		// [#J001] US Justin 2018.10.18 Transfer Account Bug Fix
						}

						if (bSavingsAvail)
						{
							// F4
							m_pDevCmn->fnSCR_DisplayString(8, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SAVING));
							strValueOfFdk[3] = _T("SAVING");		// [#J001] US Justin 2018.10.18 Transfer Account Bug Fix
						}
						if (bCreditAvail)
						{
							// F6
							m_pDevCmn->fnSCR_DisplayString(9, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CREDIT));
							strValueOfFdk[5] = _T("CREDIT");		// [#J001] US Justin 2018.10.18 Transfer Account Bug Fix
						}
						// End of [#2529]
					}

					//m_pDevCmn->fnSCR_DisplayScreen(115, 0, PIN_MENU_MODE);
					m_pDevCmn->fnSCR_DisplayScreen(115, KEYIN_TIME_OUT, 0);
				}
				// End of [#2375]

				bShowScreen = FALSE;

#ifdef APP_AGING_MODE
				g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
			}

#ifdef APP_AGING_MODE
			if (g_AgingCheck.IsElapsedTimes() == TRUE)
			{
				m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SAVING));
				m_DestAccount = S_SAVINGS;
				return RES_OK;
			}
#else

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
			{
				NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
				g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
				bStartToTimeout = FALSE;
			}
			// End of [#2375]

			///////////////////////////////////
			// GET KEY STRING
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
				bInvalidRepeat = FALSE;
				if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
				{
					GetKeyStr = GetKeyStr.Mid(6);
					NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
				}
				// End of [#2375]

				// [#2518] US Kook 2018.01.25 Support MX-2800SE
				// converting "F#" to "#VALUE#"
				if ( GetKeyStr == L"F1" || GetKeyStr == L"F2" || GetKeyStr == L"F3" ||
					 GetKeyStr == L"F4" || GetKeyStr == L"F5" || GetKeyStr == L"F6")
				{
					int nPressedBtn = Asc2Int( GetKeyStr.Right(1) ) - 1;		// F1, F2, ... => 0, 1, ...
					if( (nPressedBtn>=0) && (nPressedBtn<MENU_SELECT_SIZE) )
						GetKeyStr = strValueOfFdk[nPressedBtn];
				}
				// end of [#2518]

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER FDK TO STRING CONVERSION =  [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					// [#2375] US Justin Combine ADA and Screen Flow
					if( IsAdaTransaction())		
					{
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
						#endif
					}
					// End of [#2375]

					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_32");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}
				else if (GetKeyStr == S_TIMEOVER)
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_33");	// [#2024] NH KSK 2011.02.24
					return RES_USER_TIMEOUT;
				}
				else if (GetKeyStr == L"CHECKING")
				{
					m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHECKING));
					m_DestAccount = S_CHECKING;
					return RES_OK;
				}
				else if (GetKeyStr == L"SAVING")
				{
					m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SAVING));
					m_DestAccount = S_SAVINGS;
					return RES_OK;
				}
				else if (GetKeyStr == L"CREDIT")
				{
					m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CREDIT));
					m_DestAccount = S_CREDITCARD;
					return RES_OK;
				}
				// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
				/////////////////////////////////// VOICE GUIDANCE MODE ///////////////////////////////
				else
				{
					if( IsAdaTransaction() )
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));
						arrVG.RemoveAll();

						int nValidADAInput = 0;
						if( (GetKeyStr == L"2")&&(bCheckingAvail) )					// Checking
						{
							arrVG.Add(L"2.wav");		arrVG.Add(L"Checking.wav");
							m_DestAccount = S_CHECKING;
							nValidADAInput = 1;
						}
						else if ( (GetKeyStr == L"4")&&(bSavingsAvail)	)			// Saving
						{
							arrVG.Add(L"4.wav");		arrVG.Add(L"Savings.wav");
							m_DestAccount = S_SAVINGS;
							nValidADAInput = 1;
						}
						else if ( (GetKeyStr == L"6")&&(bCreditAvail)	)			// Credit
						{
							arrVG.Add(L"6.wav");		arrVG.Add(L"Credit.wav");
							m_DestAccount = S_CREDITCARD;
							nValidADAInput = 1;
						}
						else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
						{
							nValidADAInput = 2;
							m_pAdaCtrl->fnExp_StopPlay();
							if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
							else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
						}
						
						if	( nValidADAInput == 0 )				// Invalid
						{
							NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
							#if (US_VERSION)
								m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
								bInvalidRepeat = TRUE;
								g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
							#endif
						}
						else if(nValidADAInput==1)
						{
							NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
							#if (US_VERSION)
								m_pAdaCtrl->fnExp_ResetAndAddPlay(arrVG.GetAt(0), TRUE, arrVG.GetAt(1));
							#endif
							return RES_OK;
						}
						else if( nValidADAInput == 2)			// Repeat or Volume Control
						{
							NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
							bShowScreen = TRUE;
						}
					}
				}
				// End of [#2375]
			}
#endif

			Delay_Msg(50);

			// [#2375] US Justin 2015.11.03 US Justin US ADA
			#if(US_VERSION)
				if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
					if( P_NH_ADA_NeedMoreTime() == RES_OK )
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
					else
						return RES_USER_EXIT;
				}
			#endif
			// End of [#2375]
		}
		NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
		NVDump('O', 'C', "00", L"P_NHNOR", L"NG_34");	// [#2024] NH KSK 2011.02.24
		return RES_USER_TIMEOUT;
	}

	NHDEBUG(DBG_INFO, (_T("ABNORMAL PROCESSING\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_35");	// [#2024] NH KSK 2011.02.24
	return RES_USER_EXIT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_InputCWAmount()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 출금 금액을 사용자로 부터 입력 받는다.
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_InputCWAmount(BOOL bUseFastCash)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_InputCWAmount]\n"));

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
	// [#2350] US Justin 205.06.18 Add Transaction Dispense Limit (POP Money use different dispense limit)
	//m_pDevCmn->fnCDU_GetCashDispenseInfo();
	if( TranCode == TC_POPMONEY)	m_pDevCmn->fnCDU_GetCashDispenseInfo( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_MAXAMT) );
	//else if( TranCode == TC_PIN4)	m_pDevCmn->fnCDU_GetCashDispenseInfo( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_MAXAMT) );		// [#2396] US Justin 2016.02.17 US Justin Pin4
	else if( TranCode == TC_PIN4)	m_pDevCmn->fnCDU_GetCashDispenseInfo( Asc2Int(m_Pin4.m_strMaxDispenseLimit) );									// [#2433] US Justin 2016.06.23 US Justin Add MaxDispenseLimit
	else							
	{
		// [#2472] US Justin 2017.02.08 Use DCC Withdrawal Amount
		// m_pDevCmn->fnCDU_GetCashDispenseInfo();
		if(m_pDevCmn->m_nDCCLocalAvail == DCC_PRECHECK_DCCAVAIL)	
			m_pDevCmn->fnCDU_GetCashDispenseInfo( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_OPTION_MAXWITHDRAWAL) );
		else												
			m_pDevCmn->fnCDU_GetCashDispenseInfo();
		// End of [#2472]
	}
	// End of [#2350]

	if (m_pDevCmn->m_FastCaseNo == -1 || m_pDevCmn->fnAPL_GetAvailTrans() == TRAN_WITH_NOT)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )		// [#2375] US Justin 2015.10.28
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
		}
		NVDump('O', 'C', "00", L"P_NHNOR", L"NG_36");	// [#2024] NH KSK 2011.02.24
		return RES_CASH_NOT_AVAILABLE;
	}

	// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
	// Voice Guidance Variables
	BIZ_RETURN	nRes;
	BOOL bStartToTimeout = FALSE;
	BOOL bInvalidRepeat = FALSE;
	int	 nRetryCount;
	CString strADAInput;
	// End of [#2375]

	///////////////////////////////////
	//	FAST CASH
	///////////////////////////////////
	if ((m_pDevCmn->m_FastCaseNo == 0) && (bUseFastCash == TRUE))		// Fast Cash
	{
		NVDump('O', 'C', "00", L"P_NHNOR", L"FastCash");

		bShowScreen = TRUE;

		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
		long nScrTimeOut = GetTransactionScreenTimeOut();
		if(nScrTimeOut==0)							// ADA Mode....Jack removed.
			return RES_USER_EXIT;

		// Initialize ADA Variable
		bStartToTimeout = FALSE;
		bInvalidRepeat = FALSE;

		CString	strFastCash, strAddWaveFile;
		int nButtonAvail[8];
		for(i=0; i<8; i++)
			nButtonAvail[i] = 0;
		nRetryCount = 3;

		NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
		g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
		// [#2375]

		while( (g_TimeCheck.IsElapsedTimes() == FALSE)&&(nRetryCount > 0) )	// [#2375] US Justin 2015.10.28 Combine ADA and Screen Flow (nRetryCount)
		{
			///////////////////////////////////
			// CHECK STATUS
			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( IsAdaTransaction() ) 
			{
				if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
				{
					NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
					NVDump('O', 'C', "00", L"P_NHADA", L"NG_105");
					m_pAdaCtrl->fnExp_StopPlay();
					return RES_USER_EXIT;
				}

				if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
				{
					bInvalidRepeat = FALSE;
					bShowScreen = TRUE;
				}
			}
			// End of [#2375]

			///////////////////////////////////
			// DISPLAY SCREEN
			if (bShowScreen == TRUE)
			{
				NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

				// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
				if( IsAdaTransaction())
				{
					m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
					m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

					m_pAdaCtrl->fnExp_StopAndResetWaveFile();
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_AddWaveFile(203, L"203.wav");

						for (i=0; i<CDU_CST_MAXFASTCASH; i++)		
						{
							if (m_pDevCmn->m_nFastCash[i])
							{
								m_pAdaCtrl->fnExp_AddWaveFile(203, L"For.wav");
								strFastCash.Format(L"%d", m_pDevCmn->m_nFastCash[i]);
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(203, strFastCash);
								strAddWaveFile.Format(L"Press%d.wav", i+1);
								m_pAdaCtrl->fnExp_AddWaveFile(203, strAddWaveFile);
								nButtonAvail[i+1] = 1;				// Enable Button
							}
						}
						m_pAdaCtrl->fnExp_AddWaveFile(203, L"OtherAmt.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(203, L"LocateCancelKey.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(203, L"LocationRepeatKey.wav");				
					#else
						for (i=0; i<CDU_CST_MAXFASTCASH; i++)		
						{
							if (m_pDevCmn->m_nFastCash[i])
							{
								strAddWaveFile.Format(L"Press%dFor.wav", i+1);
								strFastCash.Format(L"%d", m_pDevCmn->m_nFastCash[i]);
								m_pAdaCtrl->fnExp_AddWaveFile(203, strAddWaveFile);
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(203, strFastCash);
								nButtonAvail[i+1] = 1;				// Enable Button
							}
						}
						m_pAdaCtrl->fnExp_AddWaveFile(203, L"OtherAmt.wav");
					#endif
					m_pAdaCtrl->fnExp_PlayScreenWave(203);
					bStartToTimeout = TRUE;
				}
				else
				{
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
						if ( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DENOMINATION_SELECT_ENABLE) ==  ENABLE) && (TranCode == TC_WITHDRAWAL) )
							m_pDevCmn->fnSCR_DisplayString(12, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_112005));
						else
							m_pDevCmn->fnSCR_DisplayString(11, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_112004));
						// end of [#RWC6-12]

						nIndex = 4;

						for (i=0; i<CDU_CST_MAXFASTCASH; i++)
						{
							if (m_pDevCmn->m_nFastCash[i])
							{
								strDispFastCash.Format(L"   %s %d   ", GetCurrencySymbol(), m_pDevCmn->m_nFastCash[i]);

								if (GetConfigFuncPointer()->SupportsCamera())	// [#2518] US Kook 2018.01.22 Support MX-2800SE
									strDispFastCash.Trim();																								// [#GLDV-2505] Support MX-2800T

								m_pDevCmn->fnSCR_DisplayString(nIndex++, strDispFastCash);
							}
						}
					}

					m_pDevCmn->fnSCR_DisplayScreen(112, KEYIN_TIME_OUT, PIN_MENU_MODE);
				}
				// End of [#2375]

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

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
			{
				NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
				g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
				bStartToTimeout = FALSE;
			}
			// End of [#2375]

			///////////////////////////////////
			// GET KEY STRING
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
				bInvalidRepeat = FALSE;
				if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
				{
					GetKeyStr = GetKeyStr.Mid(6);
					NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
				}
				// End of [#2375]

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					// [#2375] US Justin Combine ADA and Screen Flow
					if( IsAdaTransaction())		
					{
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
						#endif
					}
					// End of [#2375]

					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_37");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}
				else if (GetKeyStr == S_TIMEOVER)
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_38");	// [#2024] NH KSK 2011.02.24
					return RES_USER_TIMEOUT;
				}
				else if (GetKeyStr == S_OTHER)
				{
					// [#RWC6-12] Denomination Selection Support
					if ( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DENOMINATION_SELECT_ENABLE) ==  ENABLE) && (TranCode == TC_WITHDRAWAL) )
					{
						return RES_DENOMINATION_SELECT;
					}
					// end of [#RWC6-12]

					bOtherAmountRoof = TRUE;
					break;
				}
				else if (GetKeyStr.GetLength() > 0)
				{
					// [#2375] US Justin 2015.10.28 Combine ADA and Screen Flow
					if (!IsAdaTransaction())			// Screen Mode....
					{
						// remove... symbol, white space
						GetKeyStr.Replace(GetCurrencySymbol(), L"");
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
							NVDump('O', 'C', "00", L"P_NHNOR", L"NG_39");	// [#2024] NH KSK 2011.02.24
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
							NVDump('O', 'C', "00", L"P_NHNOR", L"NG_40");	// [#2024] NH KSK 2011.02.24
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
							NVDump('O', 'C', "00", L"P_NHNOR", L"NG_41");	// [#2024] NH KSK 2011.02.24
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
							NVDump('O', 'C', "00", L"P_NHNOR", L"NG_42");	// [#2024] NH KSK 2011.02.24
							return RES_CASH_MIN_OVER;
						}
						
						// CENT 포함 12자리
						m_sUserSelection.strMoney.Format(L"%010d00", nInputAmount);

						return RES_OK;
					}
					else					// Voice Guidance Mode......
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));

						int nValidADAInput = 0;

						if(GetKeyStr == L"7")										// Other Amount
						{
							nValidADAInput = 1;
						}
						else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )		// Volume Control
						{
							nValidADAInput = 2;
							m_pAdaCtrl->fnExp_StopPlay();
							if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
							else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
						}
						else if ( (GetKeyStr==L"1")||(GetKeyStr==L"2")||(GetKeyStr==L"3")||(GetKeyStr==L"4")||(GetKeyStr==L"5")||(GetKeyStr==L"6") )	// Fast Cash
						{
							int nChosenBtn = Asc2Int(GetKeyStr);
							if( nButtonAvail[nChosenBtn] == 1)						// If the chosen button is Enabled
							{
								nInputAmount = m_pDevCmn->m_nFastCash[nChosenBtn-1];

								NHDEBUG(DBG_INFO, (_T("Chosen Button and FastCash Amount : Button[%s], FastCash[%d]\n"), GetKeyStr, nInputAmount));

								// Check Amount whether it is dispensible or not  ==> Meaning less : Fast Cash Amount were checked already
								if( (!m_pDevCmn->fbCDU_IsDispensible(nInputAmount)) || (nInputAmount>m_pDevCmn->m_MoneyMaxOut) || (nInputAmount<m_pDevCmn->m_MoneyMinOut) )		
								{
									nValidADAInput = 3;								// Not Dispensible.
									NVDump('O', 'C', "00", L"P_NHADA", L"NG_109");	
								}
								else
								{
									nRes = P_NH_ADA_ConfirmAmt(nInputAmount);
									if (nRes == RES_USER_PREV)
									{
										nRetryCount--;
										// [#2380] US Justin 2015.12.08 Adding VG for Cardtronics
										if(nRetryCount<=0)
										{
											#if (US_VERSION)
											if(m_pDevCmn->fnSNS_GetEnhancedAudio())
												m_pAdaCtrl->fnExp_ResetAndAddPlay(L"110_3.wav", TRUE);		// Exceed maximum number of retrial
											#endif
											NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
											NVDump('O', 'C', "00", L"P_NHADA", L"NG_114");	// [#2024] NH KSK 2011.02.24
											return RES_USER_EXIT;
										}
										// End of [#2380]
										nValidADAInput = 4;							// Try Again
									}
									else											// Confirmed or Cancelled by a user
									{
										return nRes;
									}
								}
							}
						}
						
						if ( nValidADAInput == 0 )				// Invalid
						{
							NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
							#if (US_VERSION)
								m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
								bInvalidRepeat = TRUE;
								g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
							#endif
						}
						else if( nValidADAInput==1 )			// 1:OtherAmount 
						{
							NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Other Amount\n"));
							#if (US_VERSION)
								m_pAdaCtrl->fnExp_ResetAndAddPlay(L"7.wav", TRUE);
								// [#RWC6-12] Denomination Selection Support
								if ( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DENOMINATION_SELECT_ENABLE) ==  ENABLE) && (TranCode == TC_WITHDRAWAL) )
								{
									m_pAdaCtrl->fnExp_ResetAndAddPlay(L"OtherChoose.wav", TRUE);
									return RES_DENOMINATION_SELECT;
								}
								// end of [#RWC6-12]
							#endif
							
							bOtherAmountRoof = TRUE;
							break;
						}
						else if( (nValidADAInput==2)||(nValidADAInput==4) )			// Repeat or Volume Control (2), Try again (4)
						{
							if(nValidADAInput==2)	NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
							else					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Choose Amount, nRetry = [%d]\n", nRetryCount));
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
							bShowScreen = TRUE;
						}
						else if( nValidADAInput==3 )			// 3:not dispensible.....
						{
							NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Indispensable.wav", TRUE);
							return RES_CASH_NOT_AVAILABLE;
						}
					}
					// End of [#2375]
				}
			}
#endif

			Delay_Msg(50);

			// [#2375] US Justin 2015.11.03 US Justin US ADA
			#if(US_VERSION)
				if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
					if( P_NH_ADA_NeedMoreTime() == RES_OK )
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
					else
						return RES_USER_EXIT;
				}
			#endif
			// End of [#2375]

		}
		// [#2375] US Justin 2015.10.28 Combine ADA and Screen Transactin Flow
		if (nRetryCount <= 0)
		{
			NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_114");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}
		// End of [#2375]

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
		NHDEBUG(DBG_INFO, (_T("CWOtherAmount\n")));		// [#RWC6-12] Denomination Selection Support

		bShowScreen = TRUE;
		nRetryCount = 3;

		if (bUseFastCash == FALSE)
			bOtherAmountRoof = TRUE;

		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
		long nScrTimeOut = GetTransactionScreenTimeOut();
		if(nScrTimeOut==0)							// ADA Mode....Jack removed.
			return RES_USER_EXIT;

		// ADA Variable
		bStartToTimeout = FALSE;
		bInvalidRepeat = FALSE;

		CStringArray	strtmp_ArrayAmount;
		CString			strtmp_Amount;
		CString			strInputAmount;
		int				nCashDenomination[8] = {CASH_DENOMINATION1, CASH_DENOMINATION2, CASH_DENOMINATION3, CASH_DENOMINATION4, 
												CASH_DENOMINATION5, CASH_DENOMINATION6, CASH_DENOMINATION7, CASH_DENOMINATION8};

		NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
		g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
		// [#2375]

		while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0) && (bOtherAmountRoof == TRUE))
		{
			///////////////////////////////////
			// CHECK STATUS
			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( IsAdaTransaction() ) 
			{
				if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
				{
					NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
					NVDump('O', 'C', "00", L"P_NHADA", L"NG_116");
					m_pAdaCtrl->fnExp_StopPlay();
					return RES_USER_EXIT;
				}

				if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
				{
					bInvalidRepeat = FALSE;
					bShowScreen = TRUE;
				}
			}
			// End of [#2375]

			// [#2375] US Justin 2015.10.28 Remove Duplicated Codes.... The following functions were performed at the beginning of this method.
			/*
			// [#2350] US Justin 205.06.18 Add Transaction Dispense Limit (POP Money use different dispense limit)
			//m_pDevCmn->fnCDU_GetCashDispenseInfo();
			if( TranCode == TC_POPMONEY)	m_pDevCmn->fnCDU_GetCashDispenseInfo( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_MAXAMT) );
			else							m_pDevCmn->fnCDU_GetCashDispenseInfo();
			// End of [#2350]

			if (m_pDevCmn->m_FastCaseNo == -1 || m_pDevCmn->fnAPL_GetAvailTrans() == TRAN_WITH_NOT)
			{
				NHDEBUG(DBG_INFO, (_T("FastCashNo(-1) or TRAN_WITH_NOT DETECTED\n")));
				m_pDevCmn->fnSCR_DisplayPrevSet(122);
				m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
				m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113111));
				m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
				m_pDevCmn->fnSCR_DisplayScreen(122);

				m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);

				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_43");	// [#2024] NH KSK 2011.02.24
				return RES_CASH_NOT_AVAILABLE;
			}
			*/
			// End of [#2375]

			///////////////////////////////////
			// DISPLAY SCREEN
			if (bShowScreen == TRUE)
			{
				NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

				// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
				if( IsAdaTransaction())
				{
					strtmp_ArrayAmount.RemoveAll();
					for(i=0; i<8; i++)
					{
						if (m_pDevCmn->WithAvail & (DENOMINATION_BIT1<<i))
						{
							strtmp_Amount.Format(L"%d", nCashDenomination[i]);
							strtmp_ArrayAmount.Add(strtmp_Amount);
						}
					}

					m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
					m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

					m_pAdaCtrl->fnExp_StopAndResetWaveFile();

					#if (US_VERSION)
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"204.wav");

						if( TranCode != TC_PIN4)	// [#2565] US Justin 2018.07.17 Changing Guidance of PIN4 Withdrawal Amount
						{
							m_pAdaCtrl->fnExp_AddWaveFile(204, L"AmtMultipleOf.wav");
							if (strtmp_ArrayAmount.GetSize() > 0)
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strtmp_ArrayAmount[0]);
		
							m_pAdaCtrl->fnExp_AddWaveFile(204, L"AmtMax.wav");
							strtmp_Amount.Format(L"%d", m_pDevCmn->m_MoneyMaxOut);
							m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strtmp_Amount);
						}

						m_pAdaCtrl->fnExp_AddWaveFile(204, L"ClearLocationKey.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"LocateCancelKey.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"LocationRepeatKey.wav");	

					#else
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"AmtMultipleOf.wav");

						if (strtmp_ArrayAmount.GetSize() > 0)	// KSK 2011.04.28 출금 불가일 경우에 대한 예외처리 추가
						{
							m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strtmp_ArrayAmount[0]);

							// KSK 2011.4.28 Multi Cassette일 경우 모든 권종을 표시하도록 추가 (호주 Customers요청에 의함) - 전체 적용 시 wave file 추가해야함
							#if (AU_VERSION)
							for(int i=1; i<m_pDevCmn->CSTCnt; i++)
							{
								if (strtmp_ArrayAmount.GetSize() > i)
								{
									m_pAdaCtrl->fnExp_AddWaveFile(204, L"and.wav");
									m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strtmp_ArrayAmount[i]);
								}
							}
							#endif
							// end of KSK 2011.04.28
						}										// end of KSK 2011.04.28
					
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"AmtMax.wav");
						strtmp_Amount.Format(L"%d", m_pDevCmn->m_MoneyMaxOut);
						m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strtmp_Amount);
					#endif

					m_pAdaCtrl->fnExp_PlayScreenWave(204);
					strInputAmount = L"";
					bStartToTimeout = TRUE;
				}
				else
				{
					CString			strdisp_Amount = _T("");
					BOOL			bUseComma = FALSE;

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

						// Currency Symbol Of INPUT
						m_pDevCmn->fnSCR_DisplayString(4, L"DOLLAR");
						m_pDevCmn->fnSCR_DisplayString(5, L"INT_DOLLAR");

						// Multiple of....
						if( TranCode != TC_PIN4)	// [#2565] US Justin 2018.07.17 Changing Guidance of PIN4 Withdrawal Amount
							m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113102));

						strMultipleMsg.Empty();
						strtmp_ArrayAmount.RemoveAll();

						for(int i=0; i<8; i++)
						{
							if (m_pDevCmn->WithAvail & (DENOMINATION_BIT1<<i))
							{
								strdisp_Amount.Format(L"%s %d", GetCurrencySymbol(), nCashDenomination[i]);	// [#285] [MX] KSK 2008.6.12
								strtmp_Amount.Format(L"%d", nCashDenomination[i]);

								if (bUseComma == FALSE)
									strMultipleMsg += strdisp_Amount;
								else
									strMultipleMsg += L", " + strdisp_Amount;

								strtmp_ArrayAmount.Add(strtmp_Amount);

								if (bUseComma == FALSE)	bUseComma = TRUE;
							}
						}

						if( TranCode != TC_PIN4)	// [#2565] US Justin 2018.07.17 Changing Guidance of PIN4 Withdrawal Amount
						{
							// Denominations
							m_pDevCmn->fnSCR_DisplayString(7, strMultipleMsg);

							// max
							m_pDevCmn->fnSCR_DisplayString(8, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113103));
							strTemp.Format(L"%s %d", GetCurrencySymbol(), m_pDevCmn->m_MoneyMaxOut);
							m_pDevCmn->fnSCR_DisplayString(9, strTemp);
						}

						// MIN and MAX DIGITs of INPUT
						m_pDevCmn->fnSCR_DisplayString(10, L"1");	// min
						m_pDevCmn->fnSCR_DisplayString(11, L"4");	// max
			
						// auto run
						m_pDevCmn->fnSCR_DisplayString(12, L"off");

					}

					m_pDevCmn->fnSCR_DisplayScreen(113, KEYIN_TIME_OUT, PIN_MENU_MODE);
				}
				// End of [#2375]

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

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
			{
				NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
				g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
				bStartToTimeout = FALSE;
			}
			// End of [#2375]

			///////////////////////////////////
			// GET KEY STRING
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
				bInvalidRepeat = FALSE;
				if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
				{
					GetKeyStr = GetKeyStr.Mid(6);
					NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
				}
				// End of [#2375]

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					// [#2375] US Justin Combine ADA and Screen Flow
					if( IsAdaTransaction())		
					{
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
						#endif
					}
					// End of [#2375]

					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_44");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}
				else if (GetKeyStr == S_TIMEOVER)
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_45");	// [#2024] NH KSK 2011.02.24
					return RES_USER_TIMEOUT;
				}
				else if (GetKeyStr.GetLength() > 0)
				{
					// [#2375] US Justin 2015.10.28 Combine ADA and Screen Transaction flows
					if (!IsAdaTransaction())			// Screen Mode....
					{
						nInputAmount = Asc2Int(GetKeyStr);

						NHDEBUG(DBG_INFO, (_T("Input Amount to convert integer [%d]\n"), nInputAmount));

						if (nInputAmount > m_pDevCmn->m_MoneyMaxOut)
						{
							// [#2565] US Justin 2018.07.17 Changing Guidance of PIN4 Withdrawal Amount
							//LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113104), INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function

							if( TranCode == TC_PIN4)
							{
								LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113114), INFO_SCR_TIMEOUT);
								return RES_USER_EXIT;
							}
							else
								LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113104), INFO_SCR_TIMEOUT);

							// End of [#2565]
							
							nRetryCount--;
							bShowScreen = TRUE;
						}
						else if (nInputAmount < m_pDevCmn->m_MoneyMinOut)
						{
							LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113105), INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
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
							nRetryCount--;
							bShowScreen = TRUE;
						}
					}
					else			// ADA Mode....
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));

						int nValidADAInput = 0;				

						if ((GetKeyStr.GetLength() == 1) && (GetKeyStr != L"."))			// Numbers.... 0,1,2,3,4,5,6,7,8,9
						{
							strADAInput = GetKeyStr + L".wav";
							strInputAmount += GetKeyStr;
							nValidADAInput = 1;												// (1) Need to read entered key (US)
						}
						else if (GetKeyStr == S_CLEAR)										// Clear
						{
							strADAInput = L"ClearEntry.wav";
							strInputAmount = L"";
							nValidADAInput = 1;												// (1) Need to read entered key (US)
						}
						else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )	// Repeat or Volume Change
						{
							m_pAdaCtrl->fnExp_StopPlay();
							if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
							else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
							nValidADAInput = 2;												// (2) Repeat of Volume Change
						}
						else if (GetKeyStr == S_ENTER)
						{
							NHDEBUG(DBG_INFO, (_T("INPUT AMOUNT - VALUE [%s]\n"), strInputAmount));

							#if (US_VERSION)
								// Other countries => read at "MainFrm.cpp"
								m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Enter.wav", TRUE);
							#endif	

							nValidADAInput = 3;												// (3) No Input or Zero Length
							strADAInput = L"Indispensable.wav";
							if (strInputAmount.GetLength() > 0)
							{
								nInputAmount = Asc2Int(strInputAmount);

								if (!m_pDevCmn->fbCDU_IsDispensible(nInputAmount))
								{
									strADAInput = L"Indispensable.wav";
									nValidADAInput = 4;										// (4) Not Dispensible
								}
								else if(nInputAmount > m_pDevCmn->m_MoneyMaxOut)
								{
									strADAInput = L"MaxAmt.wav";
									nValidADAInput = 5;										// (5) Over Maximum Amount
								}
								else if (nInputAmount < m_pDevCmn->m_MoneyMinOut)
								{
									strADAInput = L"LowAmt.wav";
									nValidADAInput = 6;										// (6) Less than Minimum Amount
								}
								else
								{
									nRes = P_NH_ADA_ConfirmAmt(nInputAmount);
									if (nRes == RES_USER_PREV)
										nValidADAInput = 7;									// (7) Try Again
									else								
										return nRes;										// RETURN Confirmed or Cancelled by a user
								}
							}
							nRetryCount--;
						}

						if( nValidADAInput == 1 )								// Read Entered keys (Numbers and Clear Key : US TTS Only)
						{
							NHDEBUG(DBG_INFO, (L"ADA Voice Guidance : Numbers and Clear Key \n"));
							#if (US_VERSION)
								m_pAdaCtrl->fnExp_ResetAndAddPlay(strADAInput, FALSE);
							#endif
						}
						else if( (nValidADAInput>=2)&&(nValidADAInput<=7) )		// 2(Repeat of Volume Change),  3(zero amount), 4(not dispensible), 5(too big), 6(too small) 7 (TryAgain)
						{
							if( (nValidADAInput>=3)&&(nValidADAInput<=6) ) 
							{
								NHDEBUG(DBG_INFO, (L"ADA Voice Guidance : not dispensible Amounts \n"));
								m_pAdaCtrl->fnExp_ResetAndAddPlay(strADAInput, TRUE);
							}

							// [#2380] US Justin 2015.12.08 Adding VG for Cardtronics
							if(nRetryCount<=0)
							{
								#if (US_VERSION)
								if(m_pDevCmn->fnSNS_GetEnhancedAudio())
									m_pAdaCtrl->fnExp_ResetAndAddPlay(L"110_3.wav", TRUE);		// Exceed maximum number of retrial
								#endif
								NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
								NVDump('O', 'C', "00", L"P_NHNOR", L"NG_46");	// [#2024] NH KSK 2011.02.24
								return RES_USER_EXIT;
							}
							// End of [#2380]

							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
							bShowScreen = TRUE;
						}
					}
					// End of [#2375]
				}
			}
#endif
			
			Delay_Msg(50);

			// [#2375] US Justin 2015.11.03 US Justin US ADA
			#if(US_VERSION)
				if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
					if( P_NH_ADA_NeedMoreTime() == RES_OK )
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
					else
						return RES_USER_EXIT;
				}
			#endif
			// End of [#2375]
		}

		if (nRetryCount <= 0)
		{
			NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
			// [#2375] US Justin 2015.10.28 Combine ADA and Screen Transaction flows
			if (!IsAdaTransaction())			// Screen Mode....
			{
				LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
			}
			NVDump('O', 'C', "00", L"P_NHNOR", L"NG_46");	// [#2024] NH KSK 2011.02.24
			return RES_CASH_RETRY_OVER;
		}

		if (g_TimeCheck.IsElapsedTimes() == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
			NVDump('O', 'C', "00", L"P_NHNOR", L"NG_47");	// [#2024] NH KSK 2011.02.24
			return RES_USER_TIMEOUT;
		}
	}

	NHDEBUG(DBG_INFO, (_T("ABNORMAL PROCESSING\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_48");	// [#2024] NH KSK 2011.02.24
	return RES_USER_EXIT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_DenominationSelect()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 출금 금액을 사용자로 부터 입력 받는다.
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_DenominationSelect() // [#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_DenominationSelect]\n"));

	int		nCST1Denom = 0, nCST2Denom = 0, nCST3Denom = 0, nCST4Denom = 0;
	int		nCST1Cnt = 0,	nCST2Cnt = 0,	nCST3Cnt = 0,	nCST4Cnt = 0;
	int		nCST1Max = 0,	nCST2Max = 0,	nCST3Max = 0,	nCST4Max = 0;
	int		nCST1Total = 0, nCST2Total = 0, nCST3Total = 0, nCST4Total = 0;
	int		nCSTTempCnt= 0,	nSelectedAmt=0, nCntTotal = 0,	nCurrTotal = 0, nEnteredAmt=0, nPrevEnteredAmt=0;
	int		nCstValue[CDU_MAX_CST_COUNT]; // 4
	int		nMaxValue[CDU_MAX_CST_COUNT]; // 4
	int		nTimeOutCnt = 0;
	int		nCurrADAStage = 1, nPrevADAStage = 1;	// 1=main, 2=EnterAmt, 3=SelectBill 
	int		nRetryCount = 0, nValidADAInput = 0;
 
	BOOL	bShowScreen = TRUE;
	BOOL	bAmountEntered = FALSE;
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	BOOL	bEnteringAmtMode = FALSE;
	CString	GetKeyStr;
	CString strCurrSym = L"$";
	CString	strTemp;
	CString strMultipleMsg;
	CStringArray strTempArray;
	CString	strCST1Denom, strCST2Denom, strCST3Denom, strCST4Denom, strCurrTotal, strSelectedAmt, strEnteredAmt, strADAInput, strADAInput2; //B
	BIZ_RETURN		nRes;

	// Clear previous CSTCnt
	m_sUserSelection.nCST1Cnt = 0;
	m_sUserSelection.nCST2Cnt = 0;
	m_sUserSelection.nCST3Cnt = 0;
	m_sUserSelection.nCST4Cnt = 0;

	// Clear previous CSTDenom
	m_sUserSelection.nCST1Denom = 0;
	m_sUserSelection.nCST2Denom = 0;
	m_sUserSelection.nCST3Denom = 0;
	m_sUserSelection.nCST4Denom = 0;

	nRetryCount = 3;

	// ADA Variable
	bStartToTimeout = FALSE;
	bInvalidRepeat = FALSE;

	CStringArray	strtmp_ArrayAmount;
	CString			strtmp_Amount;

	int				nCashDenomination[8] = {CASH_DENOMINATION1, CASH_DENOMINATION2, CASH_DENOMINATION3, CASH_DENOMINATION4, 
											CASH_DENOMINATION5, CASH_DENOMINATION6, CASH_DENOMINATION7, CASH_DENOMINATION8};
	
	///////////////////////////////////
	//	Denomination Select Screen
	///////////////////////////////////
	NVDump('O', 'C', "00", L"P_NHNOR", L"FastCash");

	// Set Screen TimeOut 1
	long nScrTimeOut = GetTransactionScreenTimeOut(); // ADA and Trans timeout
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	
	// Get Denomination(Value)/Number of CST
	for (int i = 0; i < CDU_MAX_CST_COUNT; i++)
	{
		nCstValue[i] = m_pDevCmn->fnCDU_GetValueOfCash(i+1);
		nMaxValue[i] = m_pDevCmn->fnCDU_GetNumberOfCash(i+1);
		switch (i)
		{
			case 0:
				if (nCstValue[i] > 0)
				{
					nCST1Max = nMaxValue[i];
					if (nCST1Max > 0)
						nCST1Denom = nCstValue[i];
				}
				break;
				
			case 1:
				if (nCstValue[i] > 0)
				{
					nCST2Max = nMaxValue[i];
					if (nCST2Max > 0) 
						nCST2Denom = nCstValue[i];
				}
				break;
			
			case 2:
				if (nCstValue[i] > 0)
				{
					nCST3Max = nMaxValue[i];
					if (nCST3Max > 0) 
						nCST3Denom = nCstValue[i];
				}
				break;
				
			case 3:
				if (nCstValue[i] > 0)
				{
					nCST4Max = nMaxValue[i];
					if (nCST4Max > 0) 
						nCST4Denom = nCstValue[i];
				}
				break;
			
			default:
				return RES_DEV_CDU_ERR;
		}
	}

	// LOOP
	while( (g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0) )
	{
		
		///////////////////////////////////
		// CHECK STATUS
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_116");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}

		// Get current total
		nCurrTotal = 0;
		if (nCST1Denom > 0)
		{
			nCST1Total = nCST1Denom * nCST1Cnt;
			nCurrTotal += nCST1Total;
		}
		if (nCST2Denom > 0)
		{
			nCST2Total = nCST2Denom * nCST2Cnt;
			nCurrTotal += nCST2Total;
		}
		if (nCST3Denom > 0)
		{
			nCST3Total = nCST3Denom * nCST3Cnt;
			nCurrTotal += nCST3Total;
		}
		if (nCST4Denom > 0)
		{
			nCST4Total = nCST4Denom * nCST4Cnt;
			nCurrTotal += nCST4Total;
		}
		nSelectedAmt = nCurrTotal;
		strSelectedAmt.Format(L"%d", nSelectedAmt);

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			if( IsAdaTransaction())
			{
				strtmp_ArrayAmount.RemoveAll();
				for(int i=0; i<8; i++)
				{
					if (m_pDevCmn->WithAvail & (DENOMINATION_BIT1<<i))
					{
						strtmp_Amount.Format(L"%d", nCashDenomination[i]);
						strtmp_ArrayAmount.Add(strtmp_Amount);
					}
				}

				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();

				#if ( (US_VERSION)||(CA_VERSION) )		// [#RWC6-12] Denomination Selection Support
					// current total
					/* uncomment this for multiple switching
					if ( ((nCurrADAStage == 2) && (nCurrTotal > 0) && (bEnteringAmtMode==FALSE)) || 
						 ((nCurrADAStage == 3) && (nCurrTotal > 0)) )
					{
						if (nPrevADAStage == 2)
						{
							m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_EnteredAmt.wav");
						}else
					*/
					if (nCurrADAStage == 3) // comment this for multiple switching
					{						// comment this for multiple switching
							m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_SelectedAmt.wav");
						m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strSelectedAmt);
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_With.wav");

						if (nCST1Cnt > 0)
						{
							strTemp.Format(L"%d.wav", nCST1Cnt);
							m_pAdaCtrl->fnExp_AddWaveFile(204, strTemp);
							m_pAdaCtrl->fnExp_AddWaveFile(204, "Of.wav");
							strTemp.Format(L"%d", nCST1Denom);
							m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strTemp);
						}
						if (nCST2Cnt > 0)
						{
							strTemp.Format(L"%d.wav", nCST2Cnt);
							m_pAdaCtrl->fnExp_AddWaveFile(204, strTemp);
							m_pAdaCtrl->fnExp_AddWaveFile(204, "Of.wav");
							strTemp.Format(L"%d", nCST2Denom);
							m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strTemp);
						}
						if (nCST3Cnt > 0)
						{
							strTemp.Format(L"%d.wav", nCST3Cnt);
							m_pAdaCtrl->fnExp_AddWaveFile(204, strTemp);
							m_pAdaCtrl->fnExp_AddWaveFile(204, "Of.wav");
							strTemp.Format(L"%d", nCST3Denom);
							m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strTemp);
						}
						if (nCST4Cnt > 0)
						{
							strTemp.Format(L"%d.wav", nCST4Cnt);
							m_pAdaCtrl->fnExp_AddWaveFile(204, strTemp);
							m_pAdaCtrl->fnExp_AddWaveFile(204, "Of.wav");
							strTemp.Format(L"%d", nCST4Denom);
							m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strTemp);
						}
					}

					// stage 1/2/3
					if (nCurrADAStage == 1)
					{
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToEnterAmt.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"press2.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToSelectBill.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"press5.wav");
						// etc	
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"LocateCancelKey.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"LocationRepeatKey.wav");
					}
					else
					if (nCurrADAStage == 2)
					{
						/* uncomment this for multiple switching
						if ( (bEnteringAmtMode==TRUE) && ((nPrevADAStage==1)||(nPrevADAStage==3)) )
						{
						*/
							m_pAdaCtrl->fnExp_AddWaveFile(204, L"204.wav");
							if( TranCode != TC_PIN4)	// [#2565] US Justin 2018.07.17 Changing Guidance of PIN4 Withdrawal Amount
							{
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"AmtMultipleOf.wav");
								if (strtmp_ArrayAmount.GetSize() > 0)
									m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strtmp_ArrayAmount[0]);
			
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"AmtMax.wav");
								strtmp_Amount.Format(L"%d", m_pDevCmn->m_MoneyMaxOut);
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strtmp_Amount);
							}
							m_pAdaCtrl->fnExp_AddWaveFile(204, L"ClearLocationKey.wav");
						/* uncomment this for multiple switching
						}
						else
						if (bEnteringAmtMode==FALSE)
						{
							if ( (nPrevADAStage == 1) || (nPrevADAStage == 3) )
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"204.wav");
							else								
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToReenterAmt.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(204, L"press2.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToChangeBill.wav");
							strTemp.Format(L"%s", strSelectedAmt);
							m_pAdaCtrl->fnExp_AddWaveFile(204, strTemp);
							m_pAdaCtrl->fnExp_AddWaveFile(204, L"press5.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToContinue.wav");
							m_pAdaCtrl->fnExp_AddWaveFile(204, L"press8.wav");
						}
						*/
					}
					else
					if (nCurrADAStage == 3)
					{
						if (nCST1Denom > 0)
						{
							// to decrease
							if (nCST1Cnt > 0)
							{
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToDecrease.wav");
								strTemp.Format(L"%d", nCST1Denom);
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strTemp);
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"press1.wav");
							}
							// to increase
							if (nCST1Denom <= (m_pDevCmn->m_MoneyMaxOut - nCurrTotal))
							{
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToIncrease.wav");
								strTemp.Format(L"%d", nCST1Denom);
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strTemp);
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"press3.wav");
							}
						}
						if (nCST2Denom > 0)
						{
							// to decrease
							if (nCST2Cnt > 0)
							{
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToDecrease.wav");
								strTemp.Format(L"%d", nCST2Denom);
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strTemp);
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"press4.wav");
							}
							// to increase
							if (nCST2Denom <= (m_pDevCmn->m_MoneyMaxOut - nCurrTotal))
							{
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToIncrease.wav");
								strTemp.Format(L"%d", nCST2Denom);
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strTemp);
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"press6.wav");
							}
						}
						if (nCST3Denom > 0)
						{
							// to decrease
							if (nCST3Cnt > 0)
							{
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToDecrease.wav");
								strTemp.Format(L"%d", nCST3Denom);
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strTemp);
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"press7.wav");
							}
							// to increase
							if (nCST3Denom <= (m_pDevCmn->m_MoneyMaxOut - nCurrTotal))
							{
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToIncrease.wav");
								strTemp.Format(L"%d", nCST3Denom);
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strTemp);
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"press9.wav");
							}
						}
						if (nCST4Denom > 0) // currently CST4 is not supported for WinCE6.0 ATMs, if CST4 is supported the number should be changed below
						{
							// to decrease
							if (nCST4Cnt > 0)
							{
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToDecrease.wav");
								strTemp.Format(L"%d", nCST4Denom);
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strTemp);
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"press5.wav");
							}
							// to increase
							if (nCST4Denom <= (m_pDevCmn->m_MoneyMaxOut - nCurrTotal))
							{
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToIncrease.wav");
								strTemp.Format(L"%d", nCST4Denom);
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strTemp);
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"press0.wav");
							}
						}

						/* uncomment this for multiple switching
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToClearAndEnterAmt.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"press2.wav");
						*/
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"204_DS_ToContinue.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(204, L"press8.wav");						
					}
					else
					{
						// error here
						return RES_NG;
					}

					nPrevADAStage = nCurrADAStage;

				#else
					m_pAdaCtrl->fnExp_AddWaveFile(204, L"AmtMultipleOf.wav");

					if (strtmp_ArrayAmount.GetSize() > 0)	// KSK 2011.04.28 출금 불가일 경우에 대한 예외처리 추가
					{
						m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strtmp_ArrayAmount[0]);

						// KSK 2011.4.28 Multi Cassette일 경우 모든 권종을 표시하도록 추가 (호주 Customers요청에 의함) - 전체 적용 시 wave file 추가해야함
						#if (AU_VERSION)
						for(int i=1; i<m_pDevCmn->CSTCnt; i++)
						{
							if (strtmp_ArrayAmount.GetSize() > i)
							{
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"and.wav");
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strtmp_ArrayAmount[i]);
							}
						}
						#endif
						// end of KSK 2011.04.28
					}										// end of KSK 2011.04.28
				
					m_pAdaCtrl->fnExp_AddWaveFile(204, L"AmtMax.wav");
					strtmp_Amount.Format(L"%d", m_pDevCmn->m_MoneyMaxOut);
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strtmp_Amount);
				#endif

				m_pAdaCtrl->fnExp_PlayScreenWave(204);
				strEnteredAmt = L"";
				bStartToTimeout = TRUE;
			}
			else
			{
				CString			strdisp_Amount = _T("");
				BOOL			bUseComma = FALSE;

				m_pDevCmn->fnSCR_DisplayPrevSet(180);

				// Setting Screen
				{
					// History
					for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
					{
						if (nHistory == 0)
							m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

						m_pDevCmn->fnSCR_DisplayHistory(nHistory+1, m_arHistory[nHistory]);
					}

					strMultipleMsg.Empty(); 
					strtmp_ArrayAmount.RemoveAll();

					for(int i=0; i<8; i++)
					{
						if (m_pDevCmn->WithAvail & (DENOMINATION_BIT1<<i))
						{
							strdisp_Amount.Format(L"%s %d", GetCurrencySymbol(), nCashDenomination[i]);	// [#285] [MX] KSK 2008.6.12
							strtmp_Amount.Format(L"%d", nCashDenomination[i]);

							if (bUseComma == FALSE)
								strMultipleMsg += strdisp_Amount;
							else
								strMultipleMsg += L", " + strdisp_Amount;

							strtmp_ArrayAmount.Add(strtmp_Amount);

							if (bUseComma == FALSE)	bUseComma = TRUE;
						}
					}

					// Title
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_180001)); 

					// Sub Title
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_180002)); 

					// Get current total
					nCurrTotal = 0;
					if (nCST1Denom > 0)
					{
						nCST1Total = nCST1Denom * nCST1Cnt;
						nCurrTotal += nCST1Total;
					}
					if (nCST2Denom > 0)
					{
						nCST2Total = nCST2Denom * nCST2Cnt;
						nCurrTotal += nCST2Total;
					}
					if (nCST3Denom > 0)
					{
						nCST3Total = nCST3Denom * nCST3Cnt;
						nCurrTotal += nCST3Total;
					}
					if (nCST4Denom > 0)
					{
						nCST4Total = nCST4Denom * nCST4Cnt;
						nCurrTotal += nCST4Total;
					}

					// Max/Min
					strTemp.Format(L"%s%s%d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_180007), strCurrSym, m_pDevCmn->m_MoneyMaxOut);
					m_pDevCmn->fnSCR_DisplayString(4, strTemp); 

					strTemp.Format(L"%s%s%d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_180008), strCurrSym, m_pDevCmn->m_MoneyMinOut);
					m_pDevCmn->fnSCR_DisplayString(5, strTemp); 
					
					// Initial Screen 
					// CST1
					if (nCST1Denom > 0)
					{
						// bill image
						strTemp.Format(L"%s%d", strCurrSym, nCST1Denom);
						m_pDevCmn->fnSCR_DisplayString(8, strTemp); 

						// total
						strTemp.Format(L"%s%d", strCurrSym, nCST1Total);
						m_pDevCmn->fnSCR_DisplayString(9, strTemp);

						// minus
						if (nCST1Cnt > 0)
						{
							strTemp.Format(L" ");
							m_pDevCmn->fnSCR_DisplayString(14, strTemp); // F1
						}

						// plus
						if (nCST1Denom <= (m_pDevCmn->m_MoneyMaxOut - nCurrTotal))
						{
							strTemp.Format(L" ");
							m_pDevCmn->fnSCR_DisplayString(15, strTemp); // F2
						}
					}
					// CST2
					if (nCST2Denom > 0)
					{
						// bill image
						strTemp.Format(L"%s%d", strCurrSym, nCST2Denom);
						m_pDevCmn->fnSCR_DisplayString(10, strTemp); 

						// total
						strTemp.Format(L"%s%d", strCurrSym, nCST2Total);
						m_pDevCmn->fnSCR_DisplayString(11, strTemp);

						// minus
						if (nCST2Cnt > 0)
						{
							strTemp.Format(L" ");
							m_pDevCmn->fnSCR_DisplayString(16, strTemp); // F3
						}

						// plus
						if (nCST2Denom <= (m_pDevCmn->m_MoneyMaxOut - nCurrTotal))
						{
							strTemp.Format(L" ");
							m_pDevCmn->fnSCR_DisplayString(17, strTemp); // F4
						}
					}
					// CST3
					if (nCST3Denom > 0)
					{
						// bill image
						strTemp.Format(L"%s%d", strCurrSym, nCST3Denom);
						m_pDevCmn->fnSCR_DisplayString(12, strTemp); 

						// total
						strTemp.Format(L"%s%d", strCurrSym, nCST3Total);
						m_pDevCmn->fnSCR_DisplayString(13, strTemp);

						// minus
						if (nCST3Cnt > 0)
						{
							strTemp.Format(L" ");
							m_pDevCmn->fnSCR_DisplayString(18, strTemp); // F5
						}

						// plus
						if (nCST3Denom <= (m_pDevCmn->m_MoneyMaxOut - nCurrTotal))
						{
							strTemp.Format(L" ");
							m_pDevCmn->fnSCR_DisplayString(19, strTemp); // F6
						}
					}
					else
					{
						strTemp.Format(L"");
						m_pDevCmn->fnSCR_DisplayString(12, strTemp); 
					}
					// CST4 (if CST4 supported, then need to change string numbers)
					if (nCST4Denom > 0)
					{
						// bill image
						strTemp.Format(L"%s%d", strCurrSym, nCST4Denom); 
						m_pDevCmn->fnSCR_DisplayString(14, strTemp); 

						// total
						strTemp.Format(L"%s%d", strCurrSym, nCST4Total);
						m_pDevCmn->fnSCR_DisplayString(15, strTemp);

						// minus
						if (nCST4Cnt > 0)
						{
							strTemp.Format(L" ");
							m_pDevCmn->fnSCR_DisplayString(20, strTemp); // F7 (if CST4 supported)
						}

						// plus
						if (nCST4Denom <= (m_pDevCmn->m_MoneyMaxOut - nCurrTotal))
						{
							strTemp.Format(L" ");
							m_pDevCmn->fnSCR_DisplayString(21, strTemp); // F8 (if CST4 supported)
						}
					}

					// Continue, Total
					if ( ((nSelectedAmt >= m_pDevCmn->m_MoneyMinOut) && (nSelectedAmt <= m_pDevCmn->m_MoneyMaxOut) && (nSelectedAmt = nCurrTotal) && (nCurrTotal > 0) ) ||
							 ((nEnteredAmt >= m_pDevCmn->m_MoneyMinOut) && (nEnteredAmt >= m_pDevCmn->m_MoneyMaxOut) && (nEnteredAmt = nCurrTotal) && (nCurrTotal > 0)) )
						m_pDevCmn->fnSCR_DisplayString(20, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_180004)); // F7 (need to change this if CST4 supported)
						
					// Entering Amount text
					strTemp.Format(L" ");
					m_pDevCmn->fnSCR_DisplayString(22, strTemp);

					// Total text, value
					strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_180003));
					m_pDevCmn->fnSCR_DisplayString(6, strTemp);
					strCurrTotal.Format(L"%s%d", strCurrSym, nCurrTotal);
					m_pDevCmn->fnSCR_DisplayString(7, strCurrTotal);

					// Exit
					m_pDevCmn->fnSCR_DisplayString(21, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_180005)); // F8 (need to change this if CST4 supported)
				}

				m_pDevCmn->fnSCR_DisplayScreen(180, KEYIN_TIME_OUT, PIN_MENU_MODE);

			}
			// End of [#2375]

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
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2375]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));
			
			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			bInvalidRepeat = FALSE; 
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2375]

			// remove... symbol, white space
			GetKeyStr.Replace(GetCurrencySymbol(), L"");
			GetKeyStr.TrimLeft();
			GetKeyStr.TrimRight();

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				// [#2375] US Justin Combine ADA and Screen Flow
				if( IsAdaTransaction())		
				{
					#if ( (US_VERSION)||(CA_VERSION) )		// [#RWC6-12] Denomination Selection Support
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2375]

				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_37");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER) // && (P_NH_NOR_NeedMoreTime() != RES_YES)  // NeedMoreTime cancelled
			{
				nTimeOutCnt++;
				if (nTimeOutCnt >= 2)
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_38");	// [#2024] NH KSK 2011.02.24
					return RES_USER_TIMEOUT;
				}
				else
					bShowScreen = TRUE;
			}
			else if (GetKeyStr == S_CONTINUE)
			{
				if ( (nCST1Cnt > 0) || (nCST2Cnt > 0) || (nCST3Cnt > 0) || (nCST4Cnt > 0) )
				{
					m_sUserSelection.nCST1Cnt = nCST1Cnt;
					m_sUserSelection.nCST2Cnt = nCST2Cnt;
					m_sUserSelection.nCST3Cnt = nCST3Cnt;
					m_sUserSelection.nCST4Cnt = nCST4Cnt;
					m_sUserSelection.nCST1Denom = nCST1Denom;
					m_sUserSelection.nCST2Denom = nCST2Denom;
					m_sUserSelection.nCST3Denom = nCST3Denom;
					m_sUserSelection.nCST4Denom = nCST4Denom;
					strSelectedAmt.Format(L"%010d00", nSelectedAmt);
					m_sUserSelection.strMoney.Format(L"%s", strSelectedAmt);

					if( IsAdaTransaction())		
					{
						#if ( (US_VERSION)||(CA_VERSION) )		// [#RWC6-12] Denomination Selection Support
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Continue.wav", TRUE);
						#endif
					}

					return RES_OK;
				}
				else
				{
					if( IsAdaTransaction())		
					{
					#if ( (US_VERSION)||(CA_VERSION) )		// [#RWC6-12] Denomination Selection Support
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
					}
					NVDump('O', 'C', "00", L"P_NHNOR", L"NG_37");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}
			}
			else if (GetKeyStr.GetLength() > 0) 
			{
				if (!IsAdaTransaction())			// Screen Mode....
				{
					//bAmountEntered = FALSE;
					//if ( (GetKeyStr.GetLength() == 1) && (GetKeyStr != L".") )
					//	strEnteredAmt += GetKeyStr;

					// Button interaction
					//else
					if ( (GetKeyStr == _T("F1")) || (GetKeyStr == _T("F2")) || (GetKeyStr == _T("F3")) ||
						 (GetKeyStr == _T("F4")) || (GetKeyStr == _T("F5")) || (GetKeyStr == _T("F6")) )
					{
						if( GetKeyStr == _T("F1") )
							nCST1Cnt--;
						else if( GetKeyStr == _T("F2") )
							if(nCST1Max >= nCST1Cnt+1)
								nCST1Cnt++;
							else
								LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_180006), INFO_SCR_TIMEOUT);
						else if( GetKeyStr == _T("F3") )
							nCST2Cnt--;
						else if( GetKeyStr == _T("F4") )
							if(nCST2Max >= nCST2Cnt+1)
								nCST2Cnt++;
							else
								LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_180006), INFO_SCR_TIMEOUT);
						else if( GetKeyStr == _T("F5") )
							nCST3Cnt--;
						else if( GetKeyStr == _T("F6") )
							if(nCST3Max >= nCST3Cnt+1)
								nCST3Cnt++;
							else
								LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_180006), INFO_SCR_TIMEOUT);
						//else if( GetKeyStr == _T("CONTINUE"))
						//{
						//	strTemp.Format(L"[GetKey=%s]", GetKeyStr);
						//	m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, strTemp);
						//}
						bShowScreen = TRUE; 
					}
					else
					//if (GetKeyStr == S_ENTER)
					{
						nEnteredAmt = Asc2Int(GetKeyStr);
						NHDEBUG(DBG_INFO, (_T("Input Amount to convert integer [%d]\n"), nEnteredAmt));
						
						if (nEnteredAmt > m_pDevCmn->m_MoneyMaxOut)
						{
							// [#2565] US Justin 2018.07.17 Changing Guidance of PIN4 Withdrawal Amount
							//LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113104), INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function

							if( TranCode == TC_PIN4)
							{
								LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113114), INFO_SCR_TIMEOUT);
								return RES_USER_EXIT;
							}
							else
								LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113104), INFO_SCR_TIMEOUT);

							// End of [#2565]
							
							nRetryCount--;
						}
						else if (nEnteredAmt < m_pDevCmn->m_MoneyMinOut)
						{
							LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113105), INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
							nRetryCount--;
						}
						else if (m_pDevCmn->fbCDU_IsDispensible(nEnteredAmt))
						{
							// CENT 포함 12자리
							strEnteredAmt.Format(L"%010d00", nEnteredAmt);
							nEnteredAmt = Asc2Int(strEnteredAmt,12)/100;
							//m_sUserSelection.strMoney.Format(L"%010d00", nEnteredAmt);
							//return RES_OK;
							//bAmountEntered = TRUE;
						}
						else
						{
							LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113106), INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
							nRetryCount--;
						}

						if ( (nEnteredAmt > 0) && (nEnteredAmt != nPrevEnteredAmt) && (nEnteredAmt <= m_pDevCmn->m_MoneyMaxOut) && (nEnteredAmt >= m_pDevCmn->m_MoneyMinOut))
						{
							SplitString(m_pDevCmn->fstrCDU_GetDispenseOfCST(nEnteredAmt), L",", strTempArray);
							for (int i = 0; i < strTempArray.GetSize(); i++)
							{
								switch (i)
								{
									case 0:
										nCST1Cnt = Asc2Int(strTempArray[i], 4);
										break;
									case 1:
										nCST2Cnt = Asc2Int(strTempArray[i], 4);
										break;
									case 2:
										nCST3Cnt = Asc2Int(strTempArray[i], 4);
										break;
									case 3:	
										nCST4Cnt = Asc2Int(strTempArray[i], 4);
										break;
								}
							}
							nPrevEnteredAmt = nEnteredAmt;
							nSelectedAmt = nEnteredAmt;
						}

						bShowScreen = TRUE;
					}
				}
				else // ADA here
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));

					nValidADAInput = 0;	
					strADAInput2 = L"";

					if ( (GetKeyStr.GetLength() == 1) && (GetKeyStr != L".")	 )		// Numbers.... 0,1,2,3,4,5,6,7,8,9
					{
						strADAInput = GetKeyStr + L".wav";

						if (nCurrADAStage == 2) // && (bEnteringAmtMode==TRUE) ) // uncomment this for multiple switching
						{
							nValidADAInput = 1;
							strEnteredAmt += GetKeyStr;
						}
						else
						{	
							if (nCurrADAStage == 1)
							{
								nValidADAInput = 8;
								if (GetKeyStr == L"2")
								{
									nCurrADAStage = 2;
									bEnteringAmtMode = TRUE;
									nEnteredAmt = 0;
									strEnteredAmt = L"";
								}
								else
								if (GetKeyStr == L"5")
									nCurrADAStage = 3;
								
								else
									strADAInput = L"204_DS_InvalidKey.wav";
							}
							/* uncomment this for multiple switching
							else
							if (nCurrADAStage == 2)
							{
								nValidADAInput = 1;
								
								if (GetKeyStr == L"2")
								{
									nCurrADAStage = 2;
									bEnteringAmtMode = TRUE;
									nEnteredAmt = 0;
									strEnteredAmt = L"";
								}
								else
								if (GetKeyStr == L"5")
									nCurrADAStage = 3;
								else
								if (GetKeyStr == L"8")
								{
									m_sUserSelection.nCST1Cnt = nCST1Cnt;
									m_sUserSelection.nCST2Cnt = nCST2Cnt;
									m_sUserSelection.nCST3Cnt = nCST3Cnt;
									m_sUserSelection.nCST4Cnt = nCST4Cnt;
									m_sUserSelection.nCST1Denom = nCST1Denom;
									m_sUserSelection.nCST2Denom = nCST2Denom;
									m_sUserSelection.nCST3Denom = nCST3Denom;
									m_sUserSelection.nCST4Denom = nCST4Denom;
									strSelectedAmt.Format(L"%010d00", nSelectedAmt);
									m_sUserSelection.strMoney.Format(L"%s", strSelectedAmt);
									return RES_OK;
								}
								else
									strADAInput = L"204_DS_InvalidKey.wav";
							}
							*/
							else
							if (nCurrADAStage == 3)
							{
								nValidADAInput = 8;
								if (GetKeyStr == L"1")
								{
									if (nCST1Denom > 0)
										if (nCST1Cnt > 0)
										{
											nCST1Cnt--;
											strADAInput = L"204_DS_Decreased.wav";
											strTemp.Format(L"%d.wav", nCST1Denom);
											strADAInput2 = strTemp;
										}
										else
											strADAInput = L"204_DS_MinReached.wav";
									else
										strADAInput = L"204_DS_InvalidKey.wav";
								}
								else
								if (GetKeyStr == L"3")
								{
									if (nCST1Denom > 0)
										if (nCST1Denom <= (m_pDevCmn->m_MoneyMaxOut - nCurrTotal))
										{
											nCST1Cnt++;
											strADAInput = L"204_DS_Increased.wav";
											strTemp.Format(L"%d.wav", nCST1Denom);
											strADAInput2 = strTemp;
										}
										else
											strADAInput = L"204_DS_MaxReached.wav";
									else
										strADAInput = L"204_DS_InvalidKey.wav";
								}
								else
								if (GetKeyStr == L"4")
								{
									if (nCST2Denom > 0)
										if (nCST2Cnt > 0)
										{
											nCST2Cnt--;
											strADAInput = L"204_DS_Decreased.wav";
											strTemp.Format(L"%d.wav", nCST2Denom);
											strADAInput2 = strTemp;
										}
										else
											strADAInput = L"204_DS_MinReached.wav";
									else
										strADAInput = L"204_DS_InvalidKey.wav";
								}
								else
								if (GetKeyStr == L"6")
								{
									if (nCST2Denom > 0)
										if (nCST2Denom <= (m_pDevCmn->m_MoneyMaxOut - nCurrTotal))
										{
											nCST2Cnt++;
											strADAInput = L"204_DS_Increased.wav";
											strTemp.Format(L"%d.wav", nCST2Denom);
											strADAInput2 = strTemp;
										}
										else
											strADAInput = L"204_DS_MaxReached.wav";
									else
										strADAInput = L"204_DS_InvalidKey.wav";
								}
								else
								if (GetKeyStr == L"7")
								{
									if (nCST3Denom > 0)
										if (nCST3Cnt > 0)
										{
											nCST3Cnt--;
											strADAInput = L"204_DS_Decreased.wav";
											strTemp.Format(L"%d.wav", nCST3Denom);
											strADAInput2 = strTemp;
										}
										else
											strADAInput = L"204_DS_MinReached.wav";
									else
										strADAInput = L"204_DS_InvalidKey.wav";
								}
								else
								if (GetKeyStr == L"9")
								{
									if (nCST3Denom > 0)
										if (nCST1Denom <= (m_pDevCmn->m_MoneyMaxOut - nCurrTotal))
										{
											nCST3Cnt++;
											strADAInput = L"204_DS_Increased.wav";
											strTemp.Format(L"%d.wav", nCST3Denom);
											strADAInput2 = strTemp;
										}
										else
											strADAInput = L"204_DS_MaxReached.wav";
									else
										strADAInput = L"204_DS_InvalidKey.wav";
								}
								/* uncomment this for multiple switching
								else
								if (GetKeyStr == L"2")
								{
									nCurrADAStage = 2;
									bEnteringAmtMode = TRUE;
									nEnteredAmt = 0;
									strEnteredAmt = L"";
								}
								*/
								else
								if (GetKeyStr == L"8")
								{
									if ( (nCST1Cnt > 0) || (nCST2Cnt > 0) || (nCST3Cnt > 0) || (nCST4Cnt > 0) )
									{
										m_sUserSelection.nCST1Cnt = nCST1Cnt;
										m_sUserSelection.nCST2Cnt = nCST2Cnt;
										m_sUserSelection.nCST3Cnt = nCST3Cnt;
										m_sUserSelection.nCST4Cnt = nCST4Cnt;
										m_sUserSelection.nCST1Denom = nCST1Denom;
										m_sUserSelection.nCST2Denom = nCST2Denom;
										m_sUserSelection.nCST3Denom = nCST3Denom;
										m_sUserSelection.nCST4Denom = nCST4Denom;
										strSelectedAmt.Format(L"%010d00", nSelectedAmt);
										m_sUserSelection.strMoney.Format(L"%s", strSelectedAmt);
										return RES_OK;
									}
									else
									{
										if( IsAdaTransaction())		
										{
										#if ( (US_VERSION)||(CA_VERSION) )		// [#RWC6-12] Denomination Selection Support
											m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
										#endif
										}
										NVDump('O', 'C', "00", L"P_NHNOR", L"NG_37");	// [#2024] NH KSK 2011.02.24
										return RES_USER_EXIT;
									}
								}
							}
							
						}
					}
					else if (GetKeyStr == S_CLEAR)										// Clear
					{
						strADAInput = L"ClearEntry.wav";
						nEnteredAmt = 0;
						strEnteredAmt = L"";
						nValidADAInput = 1;												// (1) Need to read entered key (US)
					}
					else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )	// Repeat or Volume Change
					{
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
						nValidADAInput = 2;												// (2) Repeat of Volume Change
					}
					else if (GetKeyStr == S_ENTER)
					{
						NHDEBUG(DBG_INFO, (_T("INPUT AMOUNT - VALUE [%s]\n"), strEnteredAmt));

						#if ( (US_VERSION)||(CA_VERSION) )		// [#RWC6-12] Denomination Selection Support
							// Other countries => read at "MainFrm.cpp"
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Enter.wav", TRUE);
						#endif	

						nValidADAInput = 3;												// (3) No Input or Zero Length
						strADAInput = L"Indispensable.wav";
						if (strEnteredAmt.GetLength() > 0)
						{
							nEnteredAmt = Asc2Int(strEnteredAmt);

							if (!m_pDevCmn->fbCDU_IsDispensible(nEnteredAmt))
							{
								strADAInput = L"Indispensable.wav";
								nValidADAInput = 4;										// (4) Not Dispensible
							}
							else if(nEnteredAmt > m_pDevCmn->m_MoneyMaxOut)
							{
								strADAInput = L"MaxAmt.wav";
								nValidADAInput = 5;										// (5) Over Maximum Amount
							}
							else if (nEnteredAmt < m_pDevCmn->m_MoneyMinOut)
							{
								strADAInput = L"LowAmt.wav";
								nValidADAInput = 6;										// (6) Less than Minimum Amount
							}
							else
							{
								nRes = P_NH_ADA_ConfirmAmt(nEnteredAmt);
								if (nRes == RES_USER_PREV)
									nValidADAInput = 7;									// (7) Try Again
								else
								if (nRes == RES_OK)
								{
									SplitString(m_pDevCmn->fstrCDU_GetDispenseOfCST(nEnteredAmt), L",", strTempArray);
									for (int i = 0; i < strTempArray.GetSize(); i++)
									{
										switch (i)
										{
											case 0:
												nCST1Cnt = Asc2Int(strTempArray[i], 4);
												break;
											case 1:
												nCST2Cnt = Asc2Int(strTempArray[i], 4);
												break;
											case 2:
												nCST3Cnt = Asc2Int(strTempArray[i], 4);
												break;
											case 3:	
												nCST4Cnt = Asc2Int(strTempArray[i], 4);
												break;
										}
									}

									// comment below for multiple switching
									m_sUserSelection.nCST1Cnt = nCST1Cnt;
									m_sUserSelection.nCST2Cnt = nCST2Cnt;
									m_sUserSelection.nCST3Cnt = nCST3Cnt;
									m_sUserSelection.nCST4Cnt = nCST4Cnt;
									m_sUserSelection.nCST1Denom = nCST1Denom;
									m_sUserSelection.nCST2Denom = nCST2Denom;
									m_sUserSelection.nCST3Denom = nCST3Denom;
									m_sUserSelection.nCST4Denom = nCST4Denom;
									strSelectedAmt.Format(L"%010d00", nEnteredAmt);
									m_sUserSelection.strMoney.Format(L"%s", strSelectedAmt);
									return RES_OK;
									// comment above for multiple switching

									// bEnteringAmtMode = FALSE; // uncomment this for multiple switching
								}
							}
						}

						if ( (nEnteredAmt > 0) && (nEnteredAmt != nPrevEnteredAmt) && (nEnteredAmt <= m_pDevCmn->m_MoneyMaxOut) && (nEnteredAmt >= m_pDevCmn->m_MoneyMinOut))
						{
							SplitString(m_pDevCmn->fstrCDU_GetDispenseOfCST(nEnteredAmt), L",", strTempArray);
							for (int i = 0; i < strTempArray.GetSize(); i++)
							{
								switch (i)
								{
									case 0:
										nCST1Cnt = Asc2Int(strTempArray[i], 4);
										break;
									case 1:
										nCST2Cnt = Asc2Int(strTempArray[i], 4);
										break;
									case 2:
										nCST3Cnt = Asc2Int(strTempArray[i], 4);
										break;
									case 3:	
										nCST4Cnt = Asc2Int(strTempArray[i], 4);
										break;
								}
							}
							nPrevEnteredAmt = nEnteredAmt;
							nSelectedAmt = nEnteredAmt;
							strSelectedAmt = strEnteredAmt;		
						}				
						nRetryCount--;
					}

					if( nValidADAInput == 1 )								// Read Entered keys (Numbers and Clear Key : US TTS Only)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance : Numbers and Clear Key \n"));
						#if ( (US_VERSION)||(CA_VERSION) )		// [#RWC6-12] Denomination Selection Support
							m_pAdaCtrl->fnExp_ResetAndAddPlay(strADAInput, FALSE);
						#endif
					}
					else if( (nValidADAInput>=2)&&(nValidADAInput<=8) )		// 2(Repeat of Volume Change),  3(zero amount), 4(not dispensible), 5(too big), 6(too small) 7 (TryAgain)
					{
						if( (nValidADAInput>=3)&&(nValidADAInput<=6) ) 
						{
							NHDEBUG(DBG_INFO, (L"ADA Voice Guidance : not dispensible Amounts \n"));
							m_pAdaCtrl->fnExp_ResetAndAddPlay(strADAInput, TRUE);
						}

						// [#2380] US Justin 2015.12.08 Adding VG for Cardtronics
						if(nRetryCount<=0)
						{
							#if ( (US_VERSION)||(CA_VERSION) )		// [#RWC6-12] Denomination Selection Support
							if(m_pDevCmn->fnSNS_GetEnhancedAudio())
								m_pAdaCtrl->fnExp_ResetAndAddPlay(L"110_3.wav", TRUE);		// Exceed maximum number of retrial
							#endif
							NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
							NVDump('O', 'C', "00", L"P_NHNOR", L"NG_46");	// [#2024] NH KSK 2011.02.24
							return RES_USER_EXIT;
						}
						// End of [#2380]


						if (nValidADAInput == 8)
						{
							#if ( (US_VERSION)||(CA_VERSION) )		// [#RWC6-12] Denomination Selection Support
							m_pAdaCtrl->fnExp_ResetAndAddPlay(strADAInput, TRUE);
							if (strADAInput2.GetLength() > 0)
								m_pAdaCtrl->fnExp_ResetAndAddPlay(strADAInput2, TRUE);
							#endif
						}

						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}			
				}
			}
		}
#endif

		Delay_Msg(50);

		// [#2375] US Justin 2015.11.03 US Justin US ADA
		#if ( (US_VERSION)||(CA_VERSION) )		// [#RWC6-12] Denomination Selection Support
			if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
				if( P_NH_ADA_NeedMoreTime() == RES_OK )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
				else
					return RES_USER_EXIT;
			}
		#endif
		// End of [#2375]

		if (nRetryCount <= 0)
		{
			NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_114");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}

		if (g_TimeCheck.IsElapsedTimes() == TRUE) // && (P_NH_NOR_NeedMoreTime() != RES_YES) ) // NeedMoreTime cancelled
		{
			NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
			return RES_USER_TIMEOUT;
		}
	}

	NHDEBUG(DBG_INFO, (_T("ABNORMAL PROCESSING\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_48");	// [#2024] NH KSK 2011.02.24
	return RES_USER_EXIT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_InputTRAmount()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 이체 금액을 입력 받는다.
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_InputTRAmount()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"InputTRAmount");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_InputTRAmount]\n"));

	int			nRetryCount = 3;
	int			nInputAmount = 0;
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// ADA Variable
	BIZ_RETURN	nRes;
	BOOL bStartToTimeout = FALSE;
	BOOL bInvalidRepeat = FALSE;
	CString		strInputAmount;
	CString		strADAInput;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// [#2375]

	while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_122");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2375]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();

				#if (US_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFile(401, L"401.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(401, L"ClearLocationKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(401, L"LocateCancelKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(401, L"LocationRepeatKey.wav");	
				#endif

				m_pAdaCtrl->fnExp_PlayScreenWave(401);
				strInputAmount = L"";
				bStartToTimeout = TRUE;
			}
			else
			{
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
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113201));

					// Currency Symbol
					m_pDevCmn->fnSCR_DisplayString(4, L"DOLLAR");
					//m_pDevCmn->fnSCR_DisplayString(5, L"INT");
					m_pDevCmn->fnSCR_DisplayString(5, L"INT_CENT");

					m_pDevCmn->fnSCR_DisplayString(10, L"1");	// min
					m_pDevCmn->fnSCR_DisplayString(11, L"8");	// max

					// auto run
					m_pDevCmn->fnSCR_DisplayString(12, L"off");

				}

				m_pDevCmn->fnSCR_DisplayScreen(113, KEYIN_TIME_OUT, PIN_MENU_MODE);
			}
			// End of [#2375]

			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
		}

#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			m_sUserSelection.strMoney = L"000000002000";
			return  RES_OK;
		}
#else

		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2375]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2375]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				// [#2375] US Justin Combine ADA and Screen Flow
				if( IsAdaTransaction())		
				{
					#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2375]

				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_49");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_50");	// [#2024] NH KSK 2011.02.24
				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr.GetLength() > 0)
			{
				// [#2375] US Justin 2015.10.28 Combine ADA and Screen Transaction flows
				if (!IsAdaTransaction())			// Screen Mode....
				{
					GetKeyStr.Replace(GetCurrencySymbol(), L"");
					GetKeyStr.TrimLeft();
					nInputAmount = Asc2Int(GetKeyStr);

					NHDEBUG(DBG_INFO, (_T("Input Amount to convert integer [%d]\n"), nInputAmount));

					if (nInputAmount > 0)
					{
						m_sUserSelection.strMoney.Format(L"%010d", nInputAmount);
						return  RES_OK;
					}
					else
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
				}
				else								// ADA MODE
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));

					int nValidADAInput = 0;				

					if ((GetKeyStr.GetLength() == 1) && (GetKeyStr != L"."))			// Numbers.... 0,1,2,3,4,5,6,7,8,9
					{
						strADAInput = GetKeyStr + L".wav";
						strInputAmount += GetKeyStr;
						nValidADAInput = 1;												// (1) Need to read entered key (US)
					}
					else if (GetKeyStr == S_CLEAR)										// Clear
					{
						strADAInput = L"ClearEntry.wav";
						strInputAmount = L"";
						nValidADAInput = 1;												// (1) Need to read entered key (US)
					}
					else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )	// Repeat or Volume Change
					{
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
						nValidADAInput = 2;												// (2) Repeat of Volume Change
					}
					else if (GetKeyStr == S_ENTER)
					{
						NHDEBUG(DBG_INFO, (_T("INPUT AMOUNT - VALUE [%s]\n"), strInputAmount));

						#if (US_VERSION)
							// Other countries => read at "MainFrm.cpp"
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Enter.wav", TRUE );
						#endif	

						nValidADAInput = 3;												// (3) No Input or Zero Length
						strADAInput = L"LowAmt.wav";
						if (strInputAmount.GetLength() > 8)
						{
							strADAInput = L"MaxAmt.wav";
							nValidADAInput = 5;											// (5) Over Maximum Amount
						}
						else if( (strInputAmount.GetLength()>0) && (strInputAmount.GetLength()<9) )
						{
							nInputAmount = Asc2Int(strInputAmount);
							if( nInputAmount <= 0)
							{
								strADAInput = L"LowAmt.wav";
								nValidADAInput = 6;										// (6) Value is ZERO
							}
							else
							{
								nRes = P_NH_ADA_ConfirmAmt(nInputAmount);
								if (nRes == RES_USER_PREV)
									nValidADAInput = 7;									// (7) Try Again
								else
									return nRes;
							}
						}
						nRetryCount--;
					}

					if( nValidADAInput == 1 )								// Read Entered keys (Numbers and Clear Key : US TTS Only)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance : Numbers and Clear Key \n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(strADAInput, FALSE);
						#endif
					}
					else if( (nValidADAInput>=2)&&(nValidADAInput<=7) )		// 2(Repeat of Volume Change),  3(zero amount), 5(too big), 6(too small) 7 (TryAgain)
					{
						if( (nValidADAInput>=3)&&(nValidADAInput<=6) ) 
						{
							NHDEBUG(DBG_INFO, (L"ADA Voice Guidance : not dispensible Amounts \n"));
							m_pAdaCtrl->fnExp_ResetAndAddPlay(strADAInput, TRUE);
						}

						// [#2380] US Justin 2015.12.08 Adding VG for Cardtronics
						if(nRetryCount<=0)
						{
							#if (US_VERSION)
							if(m_pDevCmn->fnSNS_GetEnhancedAudio())
								m_pAdaCtrl->fnExp_ResetAndAddPlay(L"110_3.wav", TRUE);		// Exceed maximum number of retrial
							#endif
							NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
							NVDump('O', 'C', "00", L"P_NHNOR", L"NG_51");	// [#2024] NH KSK 2011.02.24
							return RES_USER_EXIT;
						}
						// End of [#2380]

						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
				}
				// End of [#2375]
			}
		}
#endif

		Delay_Msg(50);
		// [#2375] US Justin 2015.11.03 US Justin US ADA
		#if(US_VERSION)
			if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
				if( P_NH_ADA_NeedMoreTime() == RES_OK )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
				else
					return RES_USER_EXIT;
			}
		#endif
		// End of [#2375]
	}

	if (nRetryCount <= 0)
	{
		NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
		// [#2375] US Justin 2015.10.28 Combine ADA and Screen Transaction flows
		if (!IsAdaTransaction())			// Screen Mode....
		{
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
			/*
			m_pDevCmn->fnSCR_DisplayPrevSet(122);
			m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
			m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112));
			m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
			m_pDevCmn->fnSCR_DisplayScreen(122);
			m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
			*/
		}
		// End of [#2375]
		NVDump('O', 'C', "00", L"P_NHNOR", L"NG_51");	// [#2024] NH KSK 2011.02.24
		return RES_USER_EXIT;
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_52");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_CashDispense()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Cash Dispense
-------------------------------------------------------------------*/
//BIZ_RETURN CTranCmn::P_NH_NOR_CashDispense()
BIZ_RETURN CTranCmn::P_NH_NOR_CashDispense(BOOL bDenominationSelect, BOOL showScreen)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"CashDispense");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_CashDispense]\n"));

	CString strMoney;
	DWORD			dwTickStart = 0, dwTickEnd = 0;

	///////////////////////////////////
	// DISPLAY SCREEN
	// [#2375] US Justin 2015.10.30 Combine ADA and Screen Flow
	if ( m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE && showScreen )
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(117);

		// Sub Title
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_117001));

		m_pDevCmn->fnSCR_DisplayScreen(117);
	}
	// End of [#2375]
	dwTickStart = GetTickCount();

	///////////////////////////////////
	// DISPENSE MONEY

	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 1);	// [#2058] NH KSK 2011.05.11 Dispense 전에 Reason For Reversal값 설정 추가

	// CDU FLICKER ON
	// [#2205] US KSK 2013.06.28
//	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
	if (!(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE"))	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_CDU, FLICKER_ON);
	else
		SetHaloLedControl(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_DISPENSING), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_DISPENSING), SKIP_MCULED);
	// end of [#2205]

	// [#GLDV-2890] US Kook 2021.05.21 Support VB Flickers on MX5400
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX5400")
	{
		// turn off CDU VB flicker (and EPP VB flicker should be turned off as well, so turn off them all)
		SetGuideLight(0x00, 0x00);		
	}

	// Dispense, CENT 제거.
	// [#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA
	if ( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DENOMINATION_SELECT_ENABLE) == ENABLE) && (TranCode = TC_WITHDRAWAL) && (bDenominationSelect == TRUE) ) // Denomination Selection is only available for normal withdrawal, not for JushCash, Popmoney, Etc
		m_pDevCmn->fnCDU_DispenseAndPresentCount(m_sUserSelection.nCST1Cnt, m_sUserSelection.nCST2Cnt, m_sUserSelection.nCST3Cnt, m_sUserSelection.nCST4Cnt, MID_DEVRSP_TIME); // DispenseCount does not work 20190313
		//m_pDevCmn->fnCDU_DispenseCount(m_sUserSelection.nCST1Cnt, m_sUserSelection.nCST2Cnt, m_sUserSelection.nCST3Cnt, m_sUserSelection.nCST4Cnt);
	else  // end of [#RWC6-12, #2584]
		m_pDevCmn->fnCDU_Dispense(Asc2Int(m_sUserSelection.strMoney.Left(10)));

	// Wait
	m_pDevCmn->fnAPL_CheckDeviceAction(DEV_CDU);
	m_pDevCmn->fnAPL_CheckDevice();			// [#7] KSK 2008. 03.14

	// TOTAL AMOUNT OF LAST DISPENSED NOTES
	strMoney = m_pDevCmn->fstrCDU_GetLastDispensedAmount();
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT, strMoney);	// [#169] [NH] KSK 2008.04.25

#if (MX_VERSION)
	// [#2147] MX KSK 2012.08.18 Multi Currency의 International인 경우 qbE를 재계산 한다 (고객 요청사항)
	if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
	{
		if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_INTERNATIONAL_TYPE)
		{
			int nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));
			int nRequestedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));

			if ((nDispensedAmount != 0) && (nDispensedAmount != nRequestedAmount))
			{
				// Partial인 경우에만 재계산 한다
				char chTemp[1024] = { 0, };
				WideToMulti(chTemp, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE), sizeof(chTemp));
				float fExchangeFee = (float)atof(chTemp);
				float fDispensedAmount = (float)(nDispensedAmount * 0.01);
				float fRequestedAmount = (float)(nRequestedAmount * 0.01);
				float fExchangePercent = fExchangeFee / fRequestedAmount;	// qbE의 percent값 얻어오기

				float fCalcPartialFee  = fDispensedAmount * fExchangePercent;
				float fCalcReversalFee = fExchangeFee - fCalcPartialFee;

				CString strTemp;
				strTemp.Format(L"%.2f", fCalcPartialFee);
				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE, strTemp);	// Partial인 경우 계산된 Fee를 재계산해서 저장한다.
				strTemp.Format(L"%.2f", fCalcReversalFee);
				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_REVERSAL_EXCHANGE_FEE, strTemp);	// Reversal시의 Fee를 계산해서 저장한다.
			}
		}
	}
	// end of [#2147]
#endif

	// Dispense Status
	{
		int	i = 0;
		int	nDispensed[4] = { 0, }, nRejected[4] = { 0, };
		int nSkewNote[4] = { 0, }, nGapNote[4] = { 0, }, nLongNote[4] = { 0, }, nShortNote[4] = { 0, }, nDoubleNote[4] = { 0, };
		CString strErrorCode, strtemp;

		
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

	#if (APP_EVENT_NOTICE)
	BIZ_EVENT_SendHost(_EVENTID_DISPENSED);		// [#2313] US Justin 2014.11.20 Notice Event
	#endif

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
			// Partial 처리
			NHDEBUG(DBG_INFO, (_T("SHOW ERROR SCREEN\n")));

			// CDU FLICKER OFF
			// [#2205] US KSK 2013.06.28
//			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
			if (!(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE"))	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_CDU, FLICKER_OFF);
			else
				SetHaloLedControl(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_TRANSACTION), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_TRANSACTION), SKIP_MCULED);
			// end of [#2205]

			dwTickEnd = GetTickCount();

			if ((dwTickEnd - dwTickStart) < 2000)
			{
				NHDEBUG(DBG_INFO, (_T("WAIT 2SEC...\n")));
				m_pDevCmn->fstrSCR_WaitTime(2);
			}

			CString strTemp;

			///////////////////////////////////
			// DISPLAY SCREEN
			// [#2375] US Justin 2015.10.30 Combine ADA and Screen Flow
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				strTemp.Format(L"%s\n", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122201));
				LIB_UserPopUpNotice(SCR_ICON_STOP, strTemp, INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
				/*
				m_pDevCmn->fnSCR_DisplayPrevSet(122);
				m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
				strTemp.Format(L"%s\n", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122201));
				m_pDevCmn->fnSCR_DisplayString(4, strTemp);
				m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
				m_pDevCmn->fnSCR_DisplayScreen(122);
				m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);
				*/
			}
			else
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay( L"001ATMError.wav", TRUE);
			}
			// End of [#2375]

			if (nDispensedAmount > 0)	// partial 처리
			{
				// CDU FLICKER ON
				// [#2205] US KSK 2013.06.28
//				if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
				if (!(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE"))	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
					m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_CDU, FLICKER_ON);
				else
					SetHaloLedControl(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_DISPENSING), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_DISPENSING), SKIP_MCULED);
				// end of [#2205]

				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_53");	// [#2024] NH KSK 2011.02.24
				return RES_DEV_CDU_ERR_PARTIAL;
			}

			NVDump('O', 'C', "00", L"P_NHNOR", L"NG_54");	// [#2024] NH KSK 2011.02.24
			return RES_DEV_CDU_ERR;		// full 처리
		}
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_SaveCWStatus()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 현금 방출에 대한 통계 갱신한다.
			    현금 방출 정보를 저널에 기록한다.
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_SaveCWStatus(BOOL bNonCash)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"SaveCWStatus");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_SaveCWStatus]\n"));

	int nRequestedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
	int nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

	if (nDispensedAmount >= nRequestedAmount)
	{
		// REVERSAL FLAG OFF SET(정상방출됨)
		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 0);	// [#12] NH PSC 2008.03.24 reason for reversal 값 초기화.

		// ERROR STACK
		if (nDispensedAmount > nRequestedAmount)
		{
			// [#2485] AU KSK 2017.06.08 고객 혼돈이 발생하므로 문구 제거
			// OTHER MESSAGE에 실제 방출 금액을 넣어줌
			//CString	strtemp;
			//strtemp.Format(L"%s%c[OVER DISPENSED AMOUNT : %s%s]", CDU_OVER_DISPENSE, UNIT_DELIMITER, GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nDispensedAmount)));
			//MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strtemp+MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG) );	// [#2292] JUSTIN 2014.10.07 Leave DCC INFO
			// end of [#2485]

			// REQUEST 금액을 DISPENSED 금액으로 엎어쓰고 정상처리함
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT, 
						MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));

			m_pDevCmn->fnAPL_StackError(L"C004F00", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003013), DEV_CDU);
		}
	}

	if (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) == REVERSAL_NONE)
	{
		m_pDevCmn->m_JNLMgr.Save(NORMAL_TRX);

		// [#2477] CA Justin 2017.04.10 Leave EMV Journal
		/*
		#if (MX_VERSION)	
		// [#2115] MX KSK 2012.02.05 
		if (m_pDevCmn->fnMCU_IsEmvEnable())
			m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
		// end of [#2115]
		#elif (US_VERSION || AU_VERSION)		// [#2353] AU KSK 2015.06.28 AU EMV Data 저장 기능 추가		
		// [#2294] US JUSTIN 2014.10.07 Leave EMV Data in US VERSION
		if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
			m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
		// End of [#2294]
		#endif		
		*/

		#if (MX_VERSION)	
			if (m_pDevCmn->fnMCU_IsEmvEnable())
		#else
			if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
		#endif		
				m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
		// End of [#2477]

	}

#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	//[#2000] 2010.10.17 SOOK 호주 사양 적용 (거래 FLOW )	// [#2069] NH KSK 2011.06.13
	if (nDispensedAmount >= nRequestedAmount) //정상 방출일 때만 Surcharge Sum함 
		m_pDevCmn->fnCDU_SumProc(SUM_OF_SURCHARGE);	
#else //end of [#2000]
	if (Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT)) > 0)	// Surcharge는 방출 금액이 있을 경우에만 Sum하도록 수정
	{
		// [#2185] US Justin 2013.05.14 Dual Host DCC
		//m_pDevCmn->fnCDU_SumProc(SUM_OF_SURCHARGE);									// Surcharge Amount Sum
		if(m_HostConfig == HC_DUALHOST)	m_pDevCmn->fnCDU_SumProc(SUM_OF_DHDCC_SURCHARGE);			// Dual Host Surcharge Amount Sum
		else				m_pDevCmn->fnCDU_SumProc(SUM_OF_SURCHARGE);					// Surcharge Amount Sum
		// End of [#2185]
	}
#endif

	if (bNonCash == TRUE)
		m_pDevCmn->fnCDU_SumProc(SUM_OF_NON_CASH_WITHDRAWAL);			// Sum Procedure(ATM)
	else
	{
		// [#2185] US Justin 2013.05.14 Dual Host DCC
		//m_pDevCmn->fnCDU_SumProc(SUM_OF_WITHDRAWAL);					// Sum Procedure(ATM)
		if(m_HostConfig == HC_DUALHOST)	m_pDevCmn->fnCDU_SumProc(SUM_OF_DHDCC);						// Sum Procedure(Dual Host)
		else				m_pDevCmn->fnCDU_SumProc(SUM_OF_WITHDRAWAL);				// Sum Procedure(ATM)
		// End of [#2185]
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_CashDispense()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Cash Dispense
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_TakeCash()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"TakeCash");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_TakeCash]\n"));

	///////////////////////////////////
	// DISPLAY SCREEN
	// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
	if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
	{
		m_pAdaCtrl->fnExp_StopAndResetWaveFile();
		m_pAdaCtrl->fnExp_AddWaveFile(1, L"Takemoney.wav");
		#if (US_VERSION)
			if		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1500SE")		// [#2380] US Justin 2015.12.09 Cardtronics Additional VG change
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"Dispenser_Location_1500.wav"); 	
			else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1800SE")		// [#2380] US Justin 2015.12.09 Cardtronics Additional VG change
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"Dispenser_Location_1800.wav");
			else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"Dispenser_Location_4000.wav");
			else
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"Dispenser_Location_Other.wav");
		#endif
		m_pAdaCtrl->fnExp_PlayScreenWave(1);
		m_pAdaCtrl->fnExp_WaitUntilStop();
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(118);

		// Setting Screen
		{
			int nRequestedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
			int nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

			// Sub Title
			if (nRequestedAmount == nDispensedAmount)
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_118001));
			else
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_118002));
		}

		m_pDevCmn->fnSCR_DisplayScreen(118);
	}
	// End of [#2375]
	
	///////////////////////////////////
	// WAIT FOR CUSTOMER
	// Enhanced Coupon이 없을 경우에만 속도 증가를 위해 Header Image를 먼저 Print 한다. // [#2219] 2013.09.04 Justin Digital Receipt, Change Variable Name
 	if ((LIB_EnhancedCouponDataProc() == FALSE) && (m_sUserSelection.nPrintReceipt == RCPT_PAPER))		
 	{
		if (GetConfigFuncPointer()->SupportsCamera())	// [#GLDV-2505] Support MX-2800%
		{
			// since MX-2800SE uses USB SPR, print header & context at once to improve speed.
		}
		else
		{
#if (US_VERSION)
#if (APP_B4U)
			if ((TranCode == TC_B4U) && (m_B4UJournal->DispenseResult > 0))	// partial or no dispense do not print header [#RWC6-16]
			{
				m_bPrintImage = FALSE;
			}
			else
#endif
#endif
			{
 				m_pDevCmn->fnSPR_PrintHeader();
				m_bPrintImage = FALSE;
			}
		}
		//}
 	}

	m_pDevCmn->fstrSCR_WaitTime(2);	// 너무 길어 2초로 변경

	// CDU FLICKER OFF
	// [#2205] US KSK 2013.06.28
//	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
	if (!(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE"))	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_CDU, FLICKER_OFF);
	else
		SetHaloLedControl(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_TRANSACTION), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_TRANSACTION), SKIP_MCULED);
	// end of [#2205]

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_Transaction()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_Transaction(BOOL bShowScreen)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"Transaction");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_Transaction]\n"));

	BIZ_RETURN	nRes;
	CString		strTemp;
	int			nEnableCount = 0;
	int			nRefreshTime = 0;
	DWORD		dwStartTime;

	if (bShowScreen == TRUE)
	{
		NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

		///////////////////////////////////
		// DISPLAY SCREEN

		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( IsAdaTransaction() )
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_DISABLE_MODE);

			if( m_pDevCmn->fnSNS_GetEnhancedAudio() )
			{
				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if (US_VERSION)
					if (TranCode == TC_REVERSAL)	m_pAdaCtrl->fnExp_AddWaveFile(1, L"010.wav");
					else							m_pAdaCtrl->fnExp_AddWaveFile(1, L"009.wav");				
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
				#else
					if (TranCode == TC_REVERSAL)	m_pAdaCtrl->fnExp_PlayScreenWave(10);
					else							m_pAdaCtrl->fnExp_PlayScreenWave(9);
				#endif
				m_pAdaCtrl->fnExp_WaitUntilStop();
			}
		}
		else
		{
			m_pDevCmn->fnSCR_DisplayPrevSet(116);

			// Setting Screen
			{
				// History
				for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
				{
					if (nHistory == 0)
						m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

					m_pDevCmn->fnSCR_DisplayHistory(nHistory+1, m_arHistory[nHistory]);
				}

				strTemp.Format(L"%s\n", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116001));

				switch(TranCode)
				{
					case TC_OPEN:					strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116101);	break;

					case TC_EXTENDED_EJUPLOAD:		// [#2076] NH KSK 2011.06.28
					case TC_EXTENDED_ADDSVC_CONFIG:	// [#2449] US Justin 2016.11.08
					case TC_EXTENDED_AID_UPDATE:	strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116108);	break;

					#if (!US_VERSION)
						case TC_REVERSAL:				strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116105);	break;	// 미국 특이 사항.
					#endif

					case TC_WITHDRAWAL:				strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116104);	break;
					case TC_INQUIRY:				strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116104);	break;
					case TC_TRANSFER:				strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116104);	break;

					case TC_HEALTHCHK:				strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116102);	break;
					case TC_DETAILHEALTHCHK:		strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116103);	break;

					case TC_TOTAL:					strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116106);	break;
					case TC_TRIALTOTAL:				strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116107);	break;
					case TC_GP_BUY_CARD:			strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116110);	break;	// [#2535] US Justin 2018.04.16
					case TC_B4U:					strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_172107);	break;	// [#2535] US Justin 2018.04.16
				}

				// Show Title
				m_pDevCmn->fnSCR_DisplayString(3, strTemp);

				// Under Guide (Please Wait)
				m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116002));

				// Tran. Advertisement.
				{
					int	i, nIndex;
					CString strCmd, strData;

					nEnableCount = 0;

					if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER ||
						TranCode == TC_DYNAMICFLOWL_1ST || TranCode == TC_DYNAMICFLOWL_2ND || TranCode == TC_PINCHANGE ||				// [#2150] US Justin 2012.09.27 Enable Advertisement on Dynamic Flow Host Connection.
						TranCode == TC_TDL_TIRSURCHARGE ||TranCode == TC_TDL_DCC_LOOKUP ||TranCode == TC_TDL_DCC_TRANSACTION )			// [#2292] US Justin 2014.09.30 Add TDL Option
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
									strData.Format(L"%s\\%d_%d\\TRANADV_%02d.jpg", ADVERTISEMENT_PATH, eFrontConfig.nWidth, eFrontConfig.nHeight, i+1);
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
			m_pDevCmn->fnSCR_DisplayScreen(116);
		}
		// End of [#2375]

		dwStartTime = GetTickCount();
	}

	///////////////////////////////////
	// CONNECT TO HOST

	#if (MX_VERSION)	// [#2137] MX KSK 2012.07.25
		if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
		{
			if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG) == MX_2ND_SEND)
				m_pDevCmn->fnAPL_SetProcCount('3');
		}
		else
		{
			m_pDevCmn->fnAPL_SetProcCount('3');
		}
	#else
		m_pDevCmn->fnAPL_SetProcCount('3');
	#endif				// end of [#2137]

	// Communication with Host
	while(1)
	{
		if ((nRes = BIZ_SendHost()) != RES_OK)
			break;
	
		if ((nRes = BIZ_RecvHost()) != RES_OK)
			break;

		// Host OK
		#if (MX_VERSION)	// [#2137] MX KSK 2012.07.25
			if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
			{
				if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG) == MX_2ND_SEND)
					m_pDevCmn->fnAPL_SetProcCount('5');
			}
			else
			{
				m_pDevCmn->fnAPL_SetProcCount('5');
			}
		#else
			m_pDevCmn->fnAPL_SetProcCount('5');
		#endif				// end of [#2137]

		if (nEnableCount > 0)
		{
			DWORD	dwDisplayTime = (GetTickCount() - dwStartTime);
			DWORD	dwTranAdvTime = (nEnableCount * nRefreshTime);

			if (dwDisplayTime < dwTranAdvTime)
			{
				m_pDevCmn->fstrSCR_WaitMiliTime(dwTranAdvTime - dwDisplayTime);
			}
		}
		return RES_OK;
	}

	if ((nRes != RES_OK) && (bShowScreen == TRUE) &&
		(m_pDevCmn->TranStatus == TRAN_TRAN || m_pDevCmn->TranStatus == TRAN_REVERSAL))		// KSK 2010.06.18 Reversal시에도 Error Msg 표시하도록 수정
	{
		NHDEBUG(DBG_INFO, (_T("SHOW ERROR SCREEN\n")));

		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( m_pDevCmn->fnSNS_GetEnhancedAudio() )
		{
			m_pAdaCtrl->fnExp_ResetAndAddPlay(L"001ATMError.wav", TRUE);

			// [#2006] NH KSK 2010.11.29 Bug Fix (Host Denial인 경우에는 Denial Description을 Play하도록 함 - WINCE5.0에 적용된 항목임)
			if (m_pDevCmn->fstrAPL_GetErrorCode().Left(3) == "DA0")
			{
				int nDiffScrWave = 0;
				CString strHostDenyMsg = m_pDevCmn->fstrAPL_GetErrorCode();

				// Host Deny Code 추출. DA0XXX0 -> XXX is Host error code
				nDiffScrWave = Asc2Int(strHostDenyMsg.Mid(3, 3));

				if (nDiffScrWave != 0)
				{
					// STD3의 ADA Host Deny 파일은 8XXX로 구성됨	// STD1,2의 ADA Host Deny 파일은 7XX0로 구성됨. 
					if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)	nDiffScrWave += 8000;
					else																			nDiffScrWave += 7000;

					m_pAdaCtrl->fnExp_StopAndResetWaveFile();
					#if (US_VERSION)
						strTemp.Format(L"%d.wav", nDiffScrWave);
						if( !m_pAdaCtrl->fnExp_TTS_IsWaveDataExist(strTemp) )
							strTemp = L"8999.wav";

						NHDEBUG(DBG_INFO, (_T("Error Wave Data = [%s]\n"), strTemp));
						m_pAdaCtrl->fnExp_AddWaveFile(1, strTemp);
						m_pAdaCtrl->fnExp_PlayScreenWave(1);
					#else
						if (!m_pAdaCtrl->fnExp_PlayScreenWave(nDiffScrWave))
						{
							nDiffScrWave = 8999;
							m_pAdaCtrl->fnExp_PlayScreenWave(nDiffScrWave);
						}
					#endif
					m_pAdaCtrl->fnExp_WaitUntilStop();
				}
			}
		}
		else
		{

			///////////////////////////////////
			// DISPLAY SCREEN	
			if( m_DualBalance.m_bDualBalance!=TRUE )			// [#2318] US Justin 2015.01.19 Not showing Error for Piggyback
			{
				strTemp.Format(L"%s\n\n", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122201));
				strTemp += m_pDevCmn->fstrAPL_GetErrorMessage();
				LIB_UserPopUpNotice(SCR_ICON_STOP, strTemp, INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
			}
			//#endif							// Remove by [#2351]
			// [#2288]
		}
		NVDump('O', 'C', "00", L"P_NHTRN", L"NG_126");
		// End of [#2375]
	}
	return nRes;	
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_Reversal()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_Reversal()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_Reversal]\n"));

	int		nReversalFlag = 0;
	CString	strTemp;	// KSK 2012.03.01 Code Sonar 대책

	if (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) == REVERSAL_NONE)
		return RES_OK;

	NVDump('O', 'C', "00", L"P_NHNOR", L"Reversal");

	// reversal flag가 2, 3 이면 configuration조건을 초기화한다.
	// reversal flag - 0 : no reversal
	// reversal flag - 1 : normal reversal
	// reversal flag - 2 : configuration only, host MAC result is all space("         ") or host MAC result error(조회, 이체거래)
	// reversal flag - 3 : configuration and reversal, host MAC result error(출금거래)
	nReversalFlag = MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG);

	if (nReversalFlag == REVERSAL_TRAN)
	{
		NHDEBUG(DBG_INFO, (L"REVERSAL FLAG IS REVERSAL_TRAN\n"));

		m_pDevCmn->TranResult = FALSE;
		m_pDevCmn->TranStatus = TRAN_REVERSAL;
		TranCode = TC_REVERSAL;

		// Communication with Host
		BIZ_RETURN nRes;
		// [#2292] US Justin 2014.10.03 Add STD3 TDL DCC
		//nRes = P_NH_NOR_Transaction(TRUE);
		if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE)&&(m_STD3_TDL_Data.m_bProceedDCC==TRUE) )
			nRes = P_NH_NOR_TDL_Connection(TC_TDL_DCC_REVERSAL);
		else
			nRes = P_NH_NOR_Transaction(TRUE);
		// End of [#2292]

		if ((nRes == RES_HOST_DENIED) || (nRes == RES_OK))
		{
			// OK or NG Value를 받았으면 Reversal Flag 를 끈다.
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 0);

			// Sum Procedure(Reversal)
			// [#2185] US Justin 2013.05.14 Dual Host DCC
			//m_pDevCmn->fnCDU_SumProc(SUM_OF_REVERSAL);
			if(m_HostConfig == HC_DUALHOST)	
				m_pDevCmn->fnCDU_SumProc(SUM_OF_DHDCC_REVERSAL);
			else
			{
				// [#2445] US Justin 2016.09.28 Exclude Just.Cash Reversal 
				// [#2446] US Justin 2016.09.29 Exclude Paypal Reversal 
				// [#2405] US Justin 2016.03.22 Exclude Pin4 Reversal 
				/*
				//m_pDevCmn->fnCDU_SumProc(SUM_OF_REVERSAL);	
				if (WITHDRAWAL_PIN4!=MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE)) 
					m_pDevCmn->fnCDU_SumProc(SUM_OF_REVERSAL);	
				// End of [#2405]
				*/
				int nWithdrawalType = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE);
				if( (WITHDRAWAL_PIN4!=nWithdrawalType) && (WITHDRAWAL_JUSTCASH!=nWithdrawalType) && (WITHDRAWAL_PAYPAL!=nWithdrawalType)) 
					m_pDevCmn->fnCDU_SumProc(SUM_OF_REVERSAL);	
				// End of [#2445]
			}
			// End of [#2185]
		}

		NHDEBUG(DBG_INFO, (L"P_NH_NOR_Transaction return [%d]\n", nRes));
	}
	else if (nReversalFlag == REVERSAL_CONF)
	{
		NHDEBUG(DBG_INFO, (L"REVERSAL FLAG IS REVERSAL_CONF\n"));

		m_pDevCmn->HostOpenRetryTime = 0;
		m_pDevCmn->HostOpenFlag = FALSE;

		// [#2375] US Justin 2015.10.29 Combine ADA and Screen Flow
		BOOL bShowScreen = TRUE;
		if( m_pDevCmn->fnSNS_GetEnhancedAudio() )
			bShowScreen = FALSE;
		// End of [#2375]

		if (BIZ_HostConfigProc(bShowScreen) == TRUE)		// [#2375] US Justin Use Parameter
		{
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
		}
	}
	else if (nReversalFlag == REVERSAL_CONF_TRAN)
	{
		NHDEBUG(DBG_INFO, (L"REVERSAL FLAG IS REVERSAL_CONF_TRAN\n"));

		m_pDevCmn->HostOpenRetryTime = 0;
		m_pDevCmn->HostOpenFlag = FALSE;

		// [#2375] US Justin 2015.10.29 Combine ADA and Screen Flow
		BOOL bShowScreen = TRUE;
		if( m_pDevCmn->fnSNS_GetEnhancedAudio() )
			bShowScreen = FALSE;
		// End of [#2375]

		if (BIZ_HostConfigProc(bShowScreen) == TRUE)		// [#2375] US Justin Use Parameter
		{
			m_pDevCmn->TranResult = FALSE;
			m_pDevCmn->TranStatus = TRAN_REVERSAL;
			TranCode = TC_REVERSAL;

			// Communication with Host
			BIZ_RETURN nRes;
			nRes = P_NH_NOR_Transaction(TRUE);
			if ((nRes == RES_HOST_DENIED) || (nRes == RES_OK))
			{
				// OK or NG Value를 받았으면 Reversal Flag 를 끈다.
				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 0);

				// Sum Procedure(Reversal)
				m_pDevCmn->fnCDU_SumProc(SUM_OF_REVERSAL);
			}
		}
	}
					
	// Reversal 결과가 DA0 장애시 정상 처리함.
	if (m_pDevCmn->fstrNET_GetErrorCode().Left(3).CompareNoCase(L"DA0") == 0)
	{
		NHDEBUG(DBG_INFO, (L"REVERSAL RESPONSE [%s]\n", m_pDevCmn->fstrNET_GetErrorCode()));

		// 만일 에러 코드가 다를시에는 JNL에 메시지 남김
		if (m_pDevCmn->fstrAPL_GetErrorCode().Left(7) != m_pDevCmn->fstrNET_GetErrorCode().Left(7))	
		{
			strTemp.Format(L"%1.1s%c%5.5s(%2.2s)%c%s", 	MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT), UNIT_DELIMITER,	
					m_pDevCmn->fstrNET_GetErrorCode().Left(5),	m_pDevCmn->fstrNET_GetErrorCode().Right(2),	UNIT_DELIMITER,	m_pDevCmn->fstrNET_GetErrorMsg());
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strTemp + MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG) );	// [#2292] JUSTIN 2014.10.07 Leave DCC INFO
			// end of KSK 2012.03.01

			SetErrSum(m_pDevCmn->fstrNET_GetErrorCode(), DEV_NET);	// [#419] [NH] KSK 2008.9.18	
			NVDump('O', 'C', "00", L"P_NHNOR", L"NG_55");	// [#2024] NH KSK 2011.02.24
		}

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
	}
	
	// REVERSAL SUCCESS
	if ((MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) == REVERSAL_NONE) && (nReversalFlag != REVERSAL_CONF))	// KSK 2010.06.18 Bug Fix (Mac Error시 처리 Bug)
	{
		NHDEBUG(DBG_INFO, (L"REVERSAL TRANSACTION SUCCESSFUL\n"));

		m_pDevCmn->m_JNLMgr.Save(REVERSAL_TRX);

		// [#2477] CA Justin 2017.04.10 Leave EMV Journal
		/*
		#if (MX_VERSION)	
			// [#2115] MX KSK 2012.02.05 
			if (m_pDevCmn->fnMCU_IsEmvEnable())
				m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
			// end of [#2115]
		#elif (US_VERSION || AU_VERSION)	// [#2353] AU KSK 2015.06.28 AU EMV Data 저장 기능 추가	
			// [#2294] US JUSTIN 2014.10.07 Leave EMV Data in US VERSION
			if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
				m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
			// End of [#2294]
		#endif		
		*/
		#if (MX_VERSION)	
			if (m_pDevCmn->fnMCU_IsEmvEnable())
		#else 
			if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
		#endif		
				m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
		// End of [#2477]

		return RES_OK;
	}

	NHDEBUG(DBG_INFO, (L"REVERSAL TRANSACTION FAILED\n"));

	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_56");	// [#2024] NH KSK 2011.02.24

	return RES_NG;	
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_ContiueReversal()
 RETURN TYPE  : - RES_YES : Stop Reversal
 PARAMETER    : - RES_USER_EXIT : Continue Reversal
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_ContiueReversal()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"ContinueRevers.");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_ContiueReversal]\n"));

	// [#2375] US Justin 2015.10.29 Combine ADA and Screen Flow
	if( m_pDevCmn->fnSNS_GetEnhancedAudio() )
		return RES_USER_EXIT;
	// End of [#2375]

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	g_TimeCheck.SetTargetTimeAfterSec(10);
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
				// ICON
				m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_QUESTION);

				// Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_107201));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_107202));
			}

			m_pDevCmn->fnSCR_DisplayScreen(107, 0, PIN_MENU_MODE);
			bShowScreen = FALSE;
			g_TimeCheck.SetTargetTimeAfterSec(10);
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_57");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_YES)
			{
				return RES_YES;
			}
			else if (GetKeyStr == S_NO)
			{
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_59");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_60");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_PrintReceipt()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_PrintReceipt()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"PrintReceipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_PrintReceipt]\n"));

	BIZ_RETURN	nRes;

	///////////////////////////////////
	// DISPLAY SCREEN
	// [#2375] US Justin 2015.10.29 Combine ADA and Screen Transactions
	if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(119);

		// Setting Screen
		{
			// Sub Title
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119001));
		}

		// [#2327] NH KSK 2015.01.28
		// NH2600 / MX2600SE인 경우 Receipt Flicker가 존재하지 않아 SW적으로 위치 표시
		#if(US_VERSION || CA_VERSION || MX_VERSION)			// [#2338] US Justin 2015.03.31 Enable HALO2 on Canadian and Mexican Versions
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
				m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
			else
				m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
		#endif
		// end of [#2327]

		m_pDevCmn->fnSCR_DisplayScreen(119);
	}
	// End of [#2375]

	if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
	{
		// [#2375] US Justin 2015.10.29 Combine ADA and Screen
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
		{
			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			#if (US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_NA.wav");
			#else
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"001ATMError.wav");
			#endif
			m_pAdaCtrl->fnExp_PlayScreenWave(1);
			m_pAdaCtrl->fnExp_WaitUntilStop();
		}
		else
		{
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119201), INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
			/*
			m_pDevCmn->fnSCR_DisplayPrevSet(122);
			m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
			m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119201));
			m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
			m_pDevCmn->fnSCR_DisplayScreen(122);
			m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);
			*/
		}
		// End of [#2375]
		NVDump('O', 'C', "00", L"P_NHNOR", L"NG_61");	// [#2024] NH KSK 2011.02.24
		return RES_DEV_SPR_ERR;
	}

	///////////////////////////////////
	// PRINT RECEIPT

	#if (MX_VERSION) // [#2115] MX KSK 2012.02.05 페소 방출은 선택된 언어로 인자, USD방출은 SPANISH로 인자.
		if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
			m_pDevCmn->fnSCR_SetCurrentLangMode(SPN_MODE);
	#endif			// end of [#2115]

	// [#2324] AU Kook 2016.08.09 Force ENGLISH receipt in case of SPANISH/FRENCH in AUS.
	// if SetCurrentLangMode(ENG_MODE) is used in ASIAN LANGAUGE, asian texts at 'Printing Receipt' screen
	// will be shown like 'ㅁㅁㅁㅁ' because font of ENG_MODE doesn't have asian characters.
	// to avoid this, ASIAN LANGUAGES own receipt texts in ENGLISH in DAT file, instead of changing Language Mode.
#if (AU_VERSION) 
	nSavedLanguage = m_pDevCmn->fnSCR_GetCurrentLangMode();
	switch (nSavedLanguage)
	{
	case SPN_MODE:
	case FRN_MODE:
		m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);	break;
	default:
		break;
	}
#endif
	// end of [#2324]

	// [#RWC6-16] Bitload 4 U. A new cryptocurrency feature for CE 6.0 ATMs
	/*if (TranCode == TC_B4U)
	{	
		CString strPrintData;
		LIB_GetDisplayData(strPrintData);
		m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT, TRUE, TRUE);
		nRes = RES_OK;
	}
	else
	{*/
	// end of [[#RWC6-16]
 	nRes = P_NH_NOR_TransactionPrint();
	//}

#if (MX_VERSION) // [#2115] MX KSK 2012.02.05 SPN Print 완료 후 원래 언어로 복귀
	if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
		m_pDevCmn->fnSCR_SetCurrentLangMode(nSavedLanguage);
#endif			// end of [#2115]

	 // [#2324] AU Kook 2016.08.09 Force Receipt Language to ENGLISH
#if (AU_VERSION)
	m_pDevCmn->fnSCR_SetCurrentLangMode(nSavedLanguage);
#endif
	// end of [#2324]

	if (nRes != RES_OK)
	{
		// [#2375] US Justin 2015.10.29 Combine ADA and Screen
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
		{
			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			#if (US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_NA.wav");
			#else
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"001ATMError.wav");
			#endif
			m_pAdaCtrl->fnExp_PlayScreenWave(1);
			m_pAdaCtrl->fnExp_WaitUntilStop();
		}
		else
		{
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119201), INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
			/*
			m_pDevCmn->fnSCR_DisplayPrevSet(122);
			m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
			m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119201));
			m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
			m_pDevCmn->fnSCR_DisplayScreen(122);
			m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);
			*/
		}
		// End of [#2375]
	}
	else
	{
		// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
		if (!(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE"))	
			m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_ON);
	}

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_TakeReceipt()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_TakeReceipt()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"TakeReceipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_TakeReceipt]\n"));

	///////////////////////////////////
	// DISPLAY SCREEN
	// [#2375] US Justin 2015.10.30 Combine ADA and Screen Transaction Flow
	// [#2380] US Justin 2015.12.09 Cardtronics Additional Voice Guidance change
	/*
	if (m_pDevCmn->fnSNS_GetEnhancedAudio())
	{
		m_pAdaCtrl->fnExp_StopAndResetWaveFile();
		m_pAdaCtrl->fnExp_AddWaveFile(1, L"Takereceipt.wav");
		#if (US_VERSION)
			if		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1500SE")		// [#2380] US Justin 2015.12.09 Cardtronics Additional VG change
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_Location_1500.wav"); 	
			else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1800SE")
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_Location_1800.wav");
			else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_Location_4000.wav");
			else
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_Location_Other.wav");
		#endif
		m_pAdaCtrl->fnExp_PlayScreenWave(1);
		m_pAdaCtrl->fnExp_WaitUntilStop();
	}
	else
	*/
	if (!m_pDevCmn->fnSNS_GetEnhancedAudio())		// End of [#2380]
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(120);

		// Setting Screen
		{
			// Sub Title
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_120001));
		}

		// NH2600 / MX2600SE인 경우 Receipt Flicker가 존재하지 않아 SW적으로 위치 표시
		// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
		#if(US_VERSION || CA_VERSION || MX_VERSION)			
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")	
				m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
			else
				m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
		#endif
		// end of [#2327]

		m_pDevCmn->fnSCR_DisplayScreen(120);

		///////////////////////////////////
		// WAIT FOR CUSTOMER
		
		//m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);
		m_pDevCmn->fstrSCR_WaitTime(2);		// 너무 길어 2초로 변경

		// SPR FLICKER OFF
		// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
		if (!(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE"))	
			m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_OFF);
		// end of [#2205]
	}
	// End of [#2375]
	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_DisplayOnReceipt()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_DisplayOnReceipt()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"DisplayReceipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_DisplayOnReceipt]\n"));

	///////////////////////////////////
	// DISPLAY SCREEN

	// [#2375] US Justin 2015.10.30 Combine ADA and Screen Transaction Flow
	if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
		return RES_OK;
	// End of [#2375]

	m_pDevCmn->fnSCR_DisplayPrevSet(121);
	// Setting Screen
	{
		/*
		m_pDevCmn->fnSCR_DisplayString( 1, L"         1         2         3         4");
		m_pDevCmn->fnSCR_DisplayString( 2, L"1234567890123456789012345678901234567890");
		m_pDevCmn->fnSCR_DisplayString( 3, L"3   567890123    89012345678901234567890");
		m_pDevCmn->fnSCR_DisplayString( 4, L"4   5678901234567890123456    1234567890");
		m_pDevCmn->fnSCR_DisplayString( 5, L"5   5678     4567    2345678901234567890");
		m_pDevCmn->fnSCR_DisplayString( 6, L"6   5678901234567890      7890     67890");
		m_pDevCmn->fnSCR_DisplayString( 7, L"7   5678901    6789012345678901234    90");
		m_pDevCmn->fnSCR_DisplayString( 8, L"8   56789012345678     45678901234567890");
		m_pDevCmn->fnSCR_DisplayString( 9, L"9   5678     45678901234567     34567890");
		m_pDevCmn->fnSCR_DisplayString(10, L"10  567890123      0123456789     567890");
		m_pDevCmn->fnSCR_DisplayString(11, L"11  567     345678        789    4567890");
		m_pDevCmn->fnSCR_DisplayString(12, L"12  567890123456789012345678901234567890");
		*/

		CString strPrintData;
		CStringArray strTempArray;
		
#if (MX_VERSION) // [#2115] MX KSK 2012.02.05 페소 방출은 선택된 언어로 인자, USD방출은 SPANISH로 인자.
		if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
			m_pDevCmn->fnSCR_SetCurrentLangMode(SPN_MODE);
#endif			// end of [#2115]

		// [#2324] AU Kook 2016.08.09 Force Receipt Language to ENGLISH
#if (AU_VERSION) 
		nSavedLanguage = m_pDevCmn->fnSCR_GetCurrentLangMode();
		m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);
#endif
		// end of [#2324]

 		LIB_GetDisplayData(strPrintData);

#if (MX_VERSION) // [#2115] MX KSK 2012.02.05 원래 Language로 복귀
		if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
			m_pDevCmn->fnSCR_SetCurrentLangMode(nSavedLanguage);
#endif			// end of [#2115]

		// [#2324] AU Kook 2016.08.09 Force Receipt Language to ENGLISH
#if (AU_VERSION)
		m_pDevCmn->fnSCR_SetCurrentLangMode(nSavedLanguage);
#endif
		// end of [#2324]


		SplitString(strPrintData, FIELD_DELIMITER, strTempArray);

		for (int i = 0; i < strTempArray.GetSize(); i++)
			m_pDevCmn->fnSCR_DisplayString((i+1), strTempArray[i]);
	}
	m_pDevCmn->fnSCR_DisplayScreen(121, NORMAL_SCR_TIMEOUT, PIN_MENU_MODE);

	///////////////////////////////////
	// WAIT FOR CUSTOMER
#ifdef APP_AGING_MODE
	m_pDevCmn->fstrSCR_GetKeyString(AGING_KEYIN_TIME);
#else
	m_pDevCmn->fstrSCR_GetKeyString(NORMAL_SCR_TIMEOUT);
#endif

	return RES_OK;
}

BIZ_RETURN	CTranCmn::P_NH_NOR_DisplayOnReceipt_B4U()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"DisplayReceipt_B4U");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_DisplayOnReceipt_B4U]\n"));

	///////////////////////////////////
	// DISPLAY SCREEN

	m_pDevCmn->fnSCR_DisplayPrevSet(172);
	
	// Setting Screen

	//CString strPrintData;
	//CStringArray strTempArray;
	

	//LIB_GetDisplayData(strPrintData);
	// 15, 16, 17, 18, 19 Disclaimer 2
	/*AddPrintData(strPrintData, L"%-75.75s",	L"Your transaction will be credited subject to verification, collection and");
	AddPrintData(strPrintData, L"%-75.75s",	L"the Rules and Regulations for B4U Financial and as otherwise provided by");
	AddPrintData(strPrintData, L"%-75.75s",	L"law. Update to your account balance is controlled by your wallet and the");
	AddPrintData(strPrintData, L"%-75.75s",	L"blockchain and may require up 24 hours. Please retain this receipt until");
	AddPrintData(strPrintData, L"%-75.75s",	L"you confirm receipt of funds.");*/

	CString strTemp=L"Your transaction will be credited subject to verification, collection and";

	m_pDevCmn->fnSCR_DisplayString(1, strTemp);
	m_pDevCmn->fnSCR_DisplayString(2, strTemp);
	m_pDevCmn->fnSCR_DisplayString(3, strTemp);
	m_pDevCmn->fnSCR_DisplayString(4, strTemp);
	m_pDevCmn->fnSCR_DisplayString(5, strTemp);
	m_pDevCmn->fnSCR_DisplayString(6, strTemp);
	m_pDevCmn->fnSCR_DisplayString(7, strTemp);
	m_pDevCmn->fnSCR_DisplayString(8, strTemp);
	m_pDevCmn->fnSCR_DisplayString(9, strTemp);
	m_pDevCmn->fnSCR_DisplayString(10, strTemp);


	/*SplitString(strPrintData, FIELD_DELIMITER, strTempArray);

	for (int i = 0; i < strTempArray.GetSize(); i++)
		m_pDevCmn->fnSCR_DisplayString((i+1), strTempArray[i]);*/

	m_pDevCmn->fnSCR_DisplayScreen(172, NORMAL_SCR_TIMEOUT, PIN_MENU_MODE);


	///////////////////////////////////
	// WAIT FOR CUSTOMER
#ifdef APP_AGING_MODE
	m_pDevCmn->fstrSCR_GetKeyString(AGING_KEYIN_TIME);
#else
	m_pDevCmn->fstrSCR_GetKeyString(NORMAL_SCR_TIMEOUT);
#endif

	return RES_OK;
}


// [#2219] 2013.09.04 NH Justin Digital Receipt
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_NH_NOR_DisplayQRCode()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_DisplayQRCode()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"DisplayQRCode");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_DisplayQRCode]\n"));

	// [#2375] US Justin 2015.10.30 Combine ADA and Screen Transaction Flow
	if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
		return RES_OK;
	// End of [#2375]

	CString GetKeyStr;
	BOOL	bShowScreen = TRUE;
	int		nScreenTimeout = NORMAL_SCR_TIMEOUT * 2;	// default 60 sec

#if (AU_VERSION)
	nScreenTimeout = NORMAL_SCR_TIMEOUT;	// 고객 요청으로 인해 Timeout 변경 60 sec -> 30sec
#endif

	while (TRUE)
	{
		if (bShowScreen == TRUE)
		{
			///////////////////////////////////
			// DISPLAY SCREEN

			bShowScreen = FALSE;
			m_pDevCmn->fnSCR_DisplayPrevSet(137);

			P_NH_NOR_TransactionPrint();	// 함수 내에 QRCODE 처리 부분이 있음

			// [#2382] US Justin Make Function
			m_pDevCmn->fnAPL_DisplayQRCodeOnScreen(QR_RECEIPT_IMAGEFILE);

			/*
			CString strData, strCmd;
			strData.Format(L"%d", 1);
			m_pDevCmn->fnSCR_SetDisplayData(L"APTranAdv", strData);

			int nRefreshTime = MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_TRAN_ADV_DISP_TIME);
			strData.Format(L"%d", nRefreshTime);
			m_pDevCmn->fnSCR_SetDisplayData(L"APTranAdvTime", strData);

			strCmd.Format(L"APTranAdv1");
			m_pDevCmn->fnSCR_SetDisplayData(strCmd, _T("\\ATM\\SCREEN\\AP_QRCode.BMP"));
			*/

			//m_pDevCmn->fnSCR_DisplayImage(1, TRUE);		 // [#2529] NH Justin 2018.02.21 Add 2800 SE

			m_pDevCmn->fnSCR_DisplayScreen(137, nScreenTimeout, PIN_MENU_MODE);
		}
		//

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (GetKeyStr == S_EXIT)
				return RES_OK;
			else if (GetKeyStr == S_TIMEOVER)
			{
				#if (AU_VERSION)	// [#2375] US Justin 2015.11.04 
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME) == 1)	// OP의 Need More Time 설정시에만 적용
					{
						if (P_NH_NOR_NeedMoreTime() != RES_YES)
							return RES_OK;
						else
							bShowScreen = TRUE;
					}
					else
					{
						return RES_OK;
					}
				#else
					return RES_OK;
				#endif
			}
		}
		Delay_Msg(50);
	}

	// 거래 종료이므로 정상 처리
	return RES_OK;
}
// End of [#2219]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_DisplayCancel()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_DisplayCancel(BIZ_RETURN nReason)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"DisplayCancel");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_DisplayCancel] nReason(%d)\n", nReason));

	CString strTemp;

	// PRE-DIALING CLOSE..
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
		MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE) == ENABLE)
		m_pDevCmn->fnNET_PreDialCancelByUser();

	///////////////////////////////////
	// DISPLAY SCREEN

	// [#2375] US Justin 2015.10.30 Combine ADA and Screen Transaction Flow
	if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
	{
		m_pAdaCtrl->fnExp_StopAndResetWaveFile();

		#if (US_VERSION)
			m_pAdaCtrl->fnExp_AddWaveFile(1, L"Transaction_NG.wav");
			m_pAdaCtrl->fnExp_PlayScreenWave(1);
		#else
			if (nReason == RES_USER_TIMEOUT)
			{
				#if !(AU_VERSION) // [#2047] AU KJW 2011.04.19 호주에서는 We are sorry가 빠짐.
					m_pAdaCtrl->fnExp_AddWaveFile(1, L"Sorry.wav");
				#endif
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"TimeOver.wav");
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
			}
			else
			{
				m_pAdaCtrl->fnExp_PlayScreenWave(4);
			}
		#endif
		m_pAdaCtrl->fnExp_WaitUntilStop();
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(122);
		// Setting Screen
		{
			switch (nReason)
			{
				// [#2150] US Justin 2012.10.04 Add Pin Change Error
				case RES_PINCHANGE_SAMEASORIGINAL:	strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122401);	break;
				case RES_PINCHANGE_DIFFERENTPIN:	strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122402);	break;
				// End of [#2150]
				//case RES_DAMAGED_ICCARD:			strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122403);	break;		// [#2159] MX Justin 2012.10.26 Cancel transaction for damaged IC card
				case RES_USER_TIMEOUT:				strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122002);	break;
				case RES_EMV_FALLBACK_DISABLED:		strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122403);	break;		// [#2554] NH Justin 2018.06.05 Change Fallback Disabled Error Message
				default:							strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122001);	break;
			}
			m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
			m_pDevCmn->fnSCR_DisplayString(4, strTemp);
			m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
		}
		m_pDevCmn->fnSCR_DisplayScreen(122);

		///////////////////////////////////
		// WAIT FOR CUSTOMER
		m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
	}
	// End of [#2375]

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_DisplayExit()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_DisplayExit()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"DisplayExit");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_DisplayExit]\n"));

	///////////////////////////////////
	// DISPLAY SCREEN

	// [#2375] US Justin 2015.10.30 Combine ADA and Screen Transaction Flow
	if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
		m_pAdaCtrl->fnExp_ResetAndAddPlay(L"TRCompleted.wav", TRUE);
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(123);

		// Setting Screen
		{
			// Exit
			if (m_bShowNotice == FALSE)
			{
				#if (AU_VERSION)
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_123001));		// "Thank you for using our service"
				#else
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_123002));		// "Thank you"
				#endif
				// End of [#2431] 
			}
			else
			{
				CString strMsg;
				strMsg = m_sNoticeInfo.strLine1;
				strMsg += L"\n";
				strMsg += m_sNoticeInfo.strLine2;
				strMsg += L"\n";
				strMsg += m_sNoticeInfo.strLine3;

				m_pDevCmn->fnSCR_DisplayString(5, strMsg);
			}
		}
		m_pDevCmn->fnSCR_DisplayScreen(123);

		///////////////////////////////////
		// WAIT FOR CUSTOMER
		//m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
		//m_pDevCmn->fstrSCR_WaitTime(1);		// 너무 길어 2초로
		m_pDevCmn->fstrSCR_WaitMiliTime(500);
	}
	// Endof [#2375]

	#if (APP_EVENT_NOTICE)
	BIZ_EVENT_SendHost(_EVENTID_FINISH_TRAN);		// [#2313] US Justin 2014.11.20 Notice Event
	BIZ_EVENT_SendHost(_EVENTID_TRAN_RESULT);		// [#2313] US Justin 2014.11.20 Notice Event
	#endif

	CString strDate;
	strDate.Format(L"%2s%2s%4s", GetDate().Mid(4,2), GetDate().Right(2), GetDate().Left(4));
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_LOCALTRANDATE, strDate);
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_LOCALTRANTIME, GetTime());

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_NH_NOR_SelectEnhancedCoupon()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Enhanced Coupon Print 여부 표시
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_SelectEnhancedCoupon()
{
	CString GetKeyStr;
	BOOL	bShowScreen = TRUE;

	// [#2375] US Justin 2015.10.29 Combine ADA and Screen Flow
	if( m_pDevCmn->fnSNS_GetEnhancedAudio() )
		return RES_OK;
	// End of [#2375]

	// [#2445] US Justin Just.Cash
	// [#2446] US Justin Paypal
	// [#2405] US Justin HalCash Online
	/*
	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE) == WITHDRAWAL_PIN4 )
		return RES_OK;
	// End of [#2405]
	*/
	int nWithdrawalType = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE);
	if( (WITHDRAWAL_PIN4==nWithdrawalType) || (WITHDRAWAL_JUSTCASH==nWithdrawalType) || (WITHDRAWAL_PAYPAL==nWithdrawalType) ) 
		return RES_OK;
	// End of [#2445]

	// Enhanced Coupon 조건이 충족하지 않을 경우 Screen Display 하지 않음
	if (LIB_EnhancedCouponDataProc() == FALSE)
		return RES_OK;

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(107);

			// Setting Screen
			{
				// ICON
				m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_QUESTION);

				// Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_107301));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_107302));

				// [#2555] NH Justin 2018.06.07 Preview Enhanced Coupon
				CString strPreview = _T("");
				for( int i=0; i<6; i++)
				{
					if (m_strarrEnhancedCouponData[i].GetSize() > 0)
					{
						for(int j=0; j<m_strarrEnhancedCouponData[i].GetSize(); j++)
						{
							if(strPreview.GetLength()>0)
								strPreview += _T("\n");
							strPreview += m_strarrEnhancedCouponData[i][j];
						}
						break;
					}
				}

				if(strPreview.GetLength()>0)
				{
					m_pDevCmn->fnSCR_DisplayString(4, L" ");		// Show Background Receipt Image 
					m_pDevCmn->fnSCR_DisplayString(5, strPreview);	// Coupon Contents
				}
				// End of [#2555]
			}

			m_pDevCmn->fnSCR_DisplayScreen(107, KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}


		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_NO || GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT || GetKeyStr == S_TIMEOVER)
			{
				LIB_EnhancedCouponDataProc(TRUE);
				return RES_OK;
			}
			else if (GetKeyStr == S_YES)
			{
				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	LIB_EnhancedCouponDataProc(TRUE);	// Max Timeout시 Enhanced Coupon Data 삭제
	return RES_OK;	// 화면에서 Event가 안올경우 사양 협의 필요
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_AnotherTransaction()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_NOR_AnotherTransaction()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"AnotherTran");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_AnotherTransaction]\n"));

	// [#2375] US Justin 2015.10.29 Combine ADA and Screen Flow
	if( m_pDevCmn->fnSNS_GetEnhancedAudio() )
		return RES_USER_EXIT;
	// End of [#2375]

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	m_arHistory.RemoveAll();
	switch (m_pDevCmn->fnSCR_GetCurrentLangMode())
	{
	case ENG_MODE:	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_ENGLISH));		break;
	case SPN_MODE:	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SPANISH));		break;
	case FRN_MODE:	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_FRENCH));		break;
	case CHN_MODE:	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_CHINESE));		break;
	case KOR_MODE:	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_KOREAN));		break;
	case JPN_MODE:	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_JAPANESE));	break;
	}

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
				// ICON
				m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_QUESTION);

				// Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_107101));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_107102));
			}

			m_pDevCmn->fnSCR_DisplayScreen(107, KEYIN_TIME_OUT, PIN_MENU_MODE);
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
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_70");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_71");	// [#2024] NH KSK 2011.02.24
				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_YES)
			{
				m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
				return RES_YES;
			}
			else if (GetKeyStr == S_NO)
			{
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_72");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_73");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

// [#2340] US Justin 2015.04.17 Combine IC Processing Function.
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_NH_NOR_EMV_IC_Processing()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Need More Time
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_EMV_IC_Processing(BOOL bADA)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"NOR_EMV_IC_Pro");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_EMV_IC_Processing]\n"));

	BOOL	bRemoveCard = FALSE;		// [#2517] US Justin 2017.11.29 EMV Fallback Enable/Diable Option

	// nRes : RES_OK, RES_EMV_FALLBACK, RES_EMV_TERMINATE, 
	BIZ_RETURN nRes = P_EMV_ICProcessing();

	if (nRes == RES_EMV_FALLBACK)
	{
		// [#2517] US Justin 2017.11.29 EMV Fallback Enable/Diable Option
		BOOL bContinueFallback = TRUE;

		#if (US_VERSION)
		if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_ENABLE_DISABLE) == EMV_FALLBACK_DISABLE )
			bContinueFallback = FALSE;
		#endif
		// End of [#2517]

		if(bContinueFallback==TRUE)		// [#2517] US Justin 2017.11.29 EMV Fallback Enable/Diable Option
		{
			// [#2375] US Justin 2015.11.02 Continue Fallback Transaction without Asking
			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			#if(US_VERSION || CA_VERSION)
				return nRes = P_EMV_ContinueFallBackProc();
			#else
				if(bADA)		
					return  P_EMV_ADA_FallBackProc();
				else
					return P_EMV_FallBackProc();
			#endif
			// End of [#2375]
		}
		else
			bRemoveCard = TRUE;
	}
	// [#2517] US Justin 2017.11.29 EMV Fallback Enable/Diable Option
	/*
	else if (nRes == RES_EMV_TERMINATE)
	{
		if (m_pDevCmn->fnMCU_GetMaterialInfo() == 0)
		{
			if(bADA)
				P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003), FALSE, TRUE);
			else
			{
				if(m_pDevCmn->m_bDisplayDeclined == TRUE)
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127005), FALSE, TRUE);
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003), FALSE, TRUE);
			}
		}
		else
		{
			if(bADA)
				P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
			else
			{
				if (m_pDevCmn->m_bDisplayDeclined == TRUE)
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127005));
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
			}
		}
		return RES_EMV_TERMINATE;
	}
	*/
	else if (nRes == RES_EMV_TERMINATE)
		bRemoveCard = TRUE;

	// Remove Card and return Terminate....
	if(bRemoveCard == TRUE)
	{
		if (m_pDevCmn->fnMCU_GetMaterialInfo() == ST_NOT_DETECT)
		{
			if(bADA)
				P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003), FALSE, TRUE);
			else
			{
				if(m_pDevCmn->m_bDisplayDeclined == TRUE)
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127005), FALSE, TRUE);
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003), FALSE, TRUE);
			}
		}
		else
		{
			if(bADA)
				P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
			else
			{
				if (m_pDevCmn->m_bDisplayDeclined == TRUE)
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127005));
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
			}
		}
		return RES_EMV_TERMINATE;
	}
	// End of [#2517]

	return RES_OK;
}
// End of [#2340]

//#if (AU_VERSION || MX_VERSION)	// [#2375] US JUSTIN Add Country restriction.
#if (AU_VERSION || MX_VERSION || US_VERSION)	// [#2375] US JUSTIN Add Country restriction.
//[#2000] SOOK 2010.10.16 호주향 개발 (거래 FLOW)
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_NH_NOR_NeedMoreTime()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Need More Time
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_NeedMoreTime()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"NeedMoreTime");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_NeedMoreTime]\n"));

	CString GetKeyStr;
	BOOL	bShowScreen = TRUE;

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
				// ICON
				m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_INFO);

				// Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_108001));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_108002));
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
			return RES_NO;
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
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_YES)
			{
				return RES_YES;
			}
			else if (GetKeyStr == S_NO)
			{
				return RES_NO;
			}
		}
#endif

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}
//end of [#2000]
#endif

// [#2396] US Justin 2016.02.10
BIZ_RETURN	CTranCmn::P_NH_SelectCardlessTransaction()
{
	NVDump('O', 'C', "00", L"P_NH_Cdls", L"Seled cardless");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_SelectCardlessTransaction()]\n"));

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// [#2413] US Justin 2016.04.08 Implement VG
	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// ADA Variable
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CStringArray arrVG;
	BIZ_RETURN nTimeOutCheck;

	int nNumEnabled, nEnabledServices;
	GetEnabledCardlessServices(&nNumEnabled, &nEnabledServices);

	// [#RWC6-68] PAI Bitcoin button
	#if (APP_CUSTOM_PAI)
		if( nNumEnabled<1 )
	#else
	if( nNumEnabled<=1 )
	#endif
		return RES_USER_EXIT;
	// end of [#RWC6-68]

	CString	strValueOfFdk[MENU_SELECT_SIZE] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };	// [#2557] NH Justin 2018.06.21 GivePay Enhancement


	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// End of [#2413]

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_78");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2413]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();

				if( nEnabledServices & CDLS_SVC_PIN4 )
				{
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"For.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"Pin4Tr.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"Press1.wav");
				}

				if( nEnabledServices & CDLS_SVC_POPMONEY )
				{
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"For.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"PopmoneyTr.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"Press2.wav");
				}

				// [#2446] US Justin 2016.09.30
				if( nEnabledServices & CDLS_SVC_PAYPALCCA )
				{
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"For.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"PayPalTr.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"Press3.wav");
				}
				// End of [#2446]

				// [#RWC6-68] PAI Bitcoin button
				#if (!APP_CUSTOM_PAI)
				// [#2445] US Justin 2016.09.28 Just.Cash
				if( nEnabledServices & CDLS_SVC_JUSTCASH )
				{
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"For.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"JustCashTr.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"Press4.wav");
				}

				if( nEnabledServices & CDLS_SVC_LIBERTYX )
				{
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"For.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"LibertyXTr.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"Press5.wav");
				}
				#else
				if( nEnabledServices & CDLS_SVC_LIBERTYX )
				{
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"For.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"LibertyXTr.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"Press4.wav");
				}
				#endif
				// end of [#RWC6-68]

				m_pAdaCtrl->fnExp_AddWaveFile(153, L"LocateCancelKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(153, L"LocationRepeatKey.wav");				
				m_pAdaCtrl->fnExp_PlayScreenWave(153);
				bStartToTimeout = TRUE;
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(153);

				//  Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_153001));

				//  Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_153002));

				for (int i = 1; i < MENU_SELECT_SIZE; i++)
					m_pDevCmn->fnSCR_DisplayString(10+i, L"");


				int nLeftFDK = 0;			// Text Mode Function Keys (F1, F3, F5, F7) // 10s for image, 20s for text and image, 30s for text with default button
				// F1, Text Only
				// [#RWC6-68] PAI Bitcoin button
				#if (!APP_CUSTOM_PAI)
					// F3, B4U
					#if(APP_B4U)
						if( nEnabledServices & CDLS_SVC_B4U )
						{
							m_pDevCmn->fnSCR_DisplayString( (10 + nLeftFDK*2 + 1), B4U_LOGO_FILE );
							m_pDevCmn->fnSCR_DisplayString( (20 + nLeftFDK*2 + 1), L"  ");
							strValueOfFdk[ nLeftFDK*2 ] = L"B4U";
							nLeftFDK ++;
						}
					#endif

					// LibertyX [#RWC6-59] US William 2019.10.14 LibertyX
					// F5, LibertyX
					#if(APP_LIBERTYX)
						if( nEnabledServices & CDLS_SVC_LIBERTYX )
						{
							m_pDevCmn->fnSCR_DisplayString( (10 + nLeftFDK*2 + 1), LIBERTYX_LOGO_FILE );
							m_pDevCmn->fnSCR_DisplayString( (20 + nLeftFDK*2 + 1), L"  ");
							strValueOfFdk[ nLeftFDK*2] = L"LIBERTYX";
							nLeftFDK ++;
						}
					#endif

					#if (APP_DIGITALMINT)
						if ( nEnabledServices & CDLS_SVC_DIGITALMINT )
						{
							m_pDevCmn->fnSCR_DisplayString( (10 + nLeftFDK*2 + 1), DIGITALMINT_LOGO_FILE );
							m_pDevCmn->fnSCR_DisplayString( (20 + nLeftFDK*2 + 1), L"  ");
							strValueOfFdk[ nLeftFDK*2] = L"DIGITALMINT";
							nLeftFDK ++;
						}
					#endif
				#endif
				// end of [#RWC6-68]

				// F7, (available)	// use this for new Cardless or make this screen as page-type if exceeds the max # of items

				int nRightFDK = 0;			// Image Mode Function Keys (F2, F4, F6, F8)

				// [#RWC6-68] PAI Bitcoin button
				#if (!APP_CUSTOM_PAI)
					// F2, Just Cash
				#if (APP_JUST_CASH)
				if( nEnabledServices & CDLS_SVC_JUSTCASH )
				{
					m_pDevCmn->fnSCR_DisplayString( (10 + nRightFDK*2 + 2), m_JustCashData.GetJustCashFileName(JUSTCASH_FILE_LOGO_BUTTON) );
					m_pDevCmn->fnSCR_DisplayString( (20 + nRightFDK*2 + 2), L"  ");
					strValueOfFdk[ nRightFDK*2 + 1] = L"JUSTCASH";
					nRightFDK ++;
				}
				#endif
				#endif
				// end of [#RWC6-68]

				// F4, Paypal CCA
				#if (APP_PAYDIANT_CCA)
				if( nEnabledServices & CDLS_SVC_PAYPALCCA )
				{
					m_pDevCmn->fnSCR_DisplayString( (10 + nRightFDK*2 + 2), m_PayPalCCA.GetPaypalCCAFileName(PAYPAL_FILE_LOGO_BUTTON) );
					m_pDevCmn->fnSCR_DisplayString( (20 + nRightFDK*2 + 2), L"  ");
					strValueOfFdk[ nRightFDK*2 + 1] = L"PAYDIANT";
					nRightFDK ++;
				}
				#endif
				// End of [#2446]

				// F6, PIN4 
				if( nEnabledServices & CDLS_SVC_PIN4 )
				{
					m_pDevCmn->fnSCR_DisplayString( (10 + nRightFDK*2 + 2), m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_BUTTON) );
					m_pDevCmn->fnSCR_DisplayString( (20 + nRightFDK*2 + 2), L"  ");
					strValueOfFdk[ nRightFDK*2 + 1] = L"PIN4";
					nRightFDK ++;
				}

				// F8, POPMONEY
				if( nEnabledServices & CDLS_SVC_POPMONEY )
				{
					m_pDevCmn->fnSCR_DisplayString( (10 + nRightFDK*2 + 2), POPMONEY_LOGO_FILE );
					m_pDevCmn->fnSCR_DisplayString( (20 + nRightFDK*2 + 2), L"  ");
					strValueOfFdk[ nRightFDK*2 + 1] = L"POPMONEY";
					nRightFDK ++;
				}
				// End of [#2557]

				m_pDevCmn->fnSCR_DisplayScreen(153, KEYIN_TIME_OUT, PIN_MENU_MODE);
			}
			// End of [#2413]
			bShowScreen = FALSE;
		}

		// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2413]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2413]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
				if( IsAdaTransaction())		
				{
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2413]
				NHDEBUG(DBG_INFO, (_T("PRESS Cancel EXIT\n")));
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				return RES_USER_TIMEOUT;
			}
			else if( (GetKeyStr==L"F1") || (GetKeyStr==L"F2") || (GetKeyStr==L"F3") || (GetKeyStr==L"F4") ||
					 (GetKeyStr==L"F5") || (GetKeyStr==L"F6") || (GetKeyStr==L"F7") || (GetKeyStr==L"F8") )
			{
				int nFdkId = Asc2Int( GetKeyStr.Right(1) ) - 1;
				if( strValueOfFdk[nFdkId] == L"PAYDIANT" )		return RES_CARDLESS_PAYPAL;
				else if( strValueOfFdk[nFdkId] == L"PIN4" )			return RES_CARDLESS_PIN4;
				else if( strValueOfFdk[nFdkId] == L"POPMONEY" )		return RES_CARDLESS_POPMONEY;
				// [#RWC6-68] PAI Bitcoin button
				#if (!APP_CUSTOM_PAI)
					else if( strValueOfFdk[nFdkId] == L"JUSTCASH" )		return RES_CARDLESS_JUSTCASH;
					else if( strValueOfFdk[nFdkId] == L"B4U" )			return RES_CARDLESS_B4U; // [#RWC6-16] Bitload4U
					else if( strValueOfFdk[nFdkId] == L"LIBERTYX" )		return RES_CARDLESS_LIBERTYX;
					else if( strValueOfFdk[nFdkId] == L"DIGITALMINT" )  return RES_CARDLESS_DIGITALMINT;
				#endif
				// end of [#RWC6-68]
			}
			// End of [#2557]
			else															// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney	
			{
				if( IsAdaTransaction() )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));
					int nValidADAInput = 0;
					arrVG.RemoveAll();

					if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
					{
						nValidADAInput = 1;					// (2) Repeat or Volume Control
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
					}
					else if( (GetKeyStr == L"1") && (nEnabledServices & CDLS_SVC_PIN4) )
					{
						arrVG.Add(L"1.wav");				arrVG.Add(L"Pin4Tr.wav");
						nValidADAInput = 2;					
					}
					else if( (GetKeyStr == L"2") && (nEnabledServices & CDLS_SVC_POPMONEY) )
					{
						arrVG.Add(L"2.wav");				arrVG.Add(L"PopmoneyTr.wav");
						nValidADAInput = 3;					
					}
					// [#2446] US Justin 2016.09.30 Paypal CCA
					else if( (GetKeyStr == L"3") && (nEnabledServices & CDLS_SVC_PAYPALCCA) )
					{
						arrVG.Add(L"3.wav");				arrVG.Add(L"PayPalTr.wav");
						nValidADAInput = 4;					
					}
					// End of [#2446]
					// [#RWC6-68] PAI Bitcoin button
					#if (!APP_CUSTOM_PAI)
					// [#2445] US Justin 2016.09.28 Just.Cash
					else if( (GetKeyStr == L"4") && (nEnabledServices & CDLS_SVC_JUSTCASH) )
					{
						arrVG.Add(L"4.wav");				arrVG.Add(L"JustCashTr.wav");
						nValidADAInput = 5;					
					}
					else if( (GetKeyStr == L"5") && (nEnabledServices & CDLS_SVC_LIBERTYX) )
					{
						arrVG.Add(L"5.wav");				arrVG.Add(L"LibertyXTr.wav");
						nValidADAInput = 6;					
					}
					#else
					else if( (GetKeyStr == L"4") && (nEnabledServices & CDLS_SVC_LIBERTYX) )
					{
						arrVG.Add(L"4.wav");				arrVG.Add(L"LibertyXTr.wav");
						nValidADAInput = 5;					
					}
					#endif
					// end of [#RWC6-68]
										
					if( nValidADAInput == 0 )				// Invalid
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
							bInvalidRepeat = TRUE;
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						#endif
					}
					else if( nValidADAInput == 1)			// Repeat or Volume Control
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
					else if( (nValidADAInput>=2)&&(nValidADAInput<=6) )			// Valid Input
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(arrVG.GetAt(0), TRUE, arrVG.GetAt(1));
						#endif

						if(nValidADAInput==2)		return RES_CARDLESS_PIN4;
						else if(nValidADAInput==3)	return RES_CARDLESS_POPMONEY;
						else if(nValidADAInput==4)	return RES_CARDLESS_PAYPAL;			// [#2446] US justin 2016.09.29
						// [#RWC6-68] PAI Bitcoin button
						#if (!APP_CUSTOM_PAI)
						else if(nValidADAInput==5)	return RES_CARDLESS_JUSTCASH;		// [#2445] US Justin 2016.09.28
						else if(nValidADAInput==6)	return RES_CARDLESS_LIBERTYX;
						#else
						else if(nValidADAInput==5)	return RES_CARDLESS_LIBERTYX;
						#endif
					}
				}
			}
			// End of [#2413]
		}
		Delay_Msg(50);

		// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
		nTimeOutCheck = P_NH_ADA_Check_TimeOUT();
		if(nTimeOutCheck==RES_OK)				bShowScreen = TRUE;
		else if(nTimeOutCheck==RES_USER_EXIT)	return RES_USER_EXIT;
		// End of [#2413]
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_52");
	return RES_USER_TIMEOUT;
}
// End of [#2396]

// [#RWC6-68] PAI Bitcoin button
BIZ_RETURN	CTranCmn::P_NH_SelectBitcoinTransaction()
{
	NVDump('O', 'C', "00", L"P_NH_Cdls", L"Seled cardless");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_SelectCardlessTransaction()]\n"));

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// [#2413] US Justin 2016.04.08 Implement VG
	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// ADA Variable
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CStringArray arrVG;
	BIZ_RETURN nTimeOutCheck;

	int nNumBCEnabled, nEnabledBitcoin;
	GetEnabledBitcoinServices(&nNumBCEnabled, &nEnabledBitcoin);
	if( nNumBCEnabled<1 )
		return RES_USER_EXIT;

	CString	strValueOfFdk[MENU_SELECT_SIZE] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };	// [#2557] NH Justin 2018.06.21 GivePay Enhancement


	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// End of [#2413]

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_78");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2413]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();

				// no ADA for B4U yet
				/*
				if( nEnabledBitcoin & CDLS_BC_B4U )
				{
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"For.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"B4UTr.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"Press6.wav");
				}
				*/

				// no ADA for LibertyX yet
				/*
				if( nEnabledBitcoin & CDLS_BC_LIBERTYX )
				{
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"For.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"LibertyXTr.wav");	
					m_pAdaCtrl->fnExp_AddWaveFile(153, L"Press7.wav");
				}
				*/

				m_pAdaCtrl->fnExp_AddWaveFile(153, L"LocateCancelKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(153, L"LocationRepeatKey.wav");				
				m_pAdaCtrl->fnExp_PlayScreenWave(153);
				bStartToTimeout = TRUE;
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(153);

				// Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_153001));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_153003));

				// BUY
				m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_153004));
				
				// SELL
				m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_153005));

				for (int i = 1; i < MENU_SELECT_SIZE; i++)
					m_pDevCmn->fnSCR_DisplayString(10+i, L"");

				int nLeftFDK = 0;			// 10s for image, 20s for text and image, 30s for text with default button

				// F1, LibertyX (Buy)
				#if (APP_LIBERTYX)
				if( nEnabledBitcoin & CDLS_BC_LIBERTYX && m_LXConfig.BuyBitcoinEnabled )
				{
					m_pDevCmn->fnSCR_DisplayString( (10 + nLeftFDK*2 + 1), LIBERTYX_LOGO_FILE );
					m_pDevCmn->fnSCR_DisplayString( (20 + nLeftFDK*2 + 1), L"  ");
					strValueOfFdk[ nLeftFDK*2 ] = L"LIBERTYX_BUY";
					nLeftFDK ++;
				}
				#endif
			
				// F3, (available)

				// F5, (available)

				// F7, (available)

				int nRightFDK = 0;			// 10s for image, 20s for text and image, 30s for text with default button

				// F2, (available)

				// F4, B4U
				#if (APP_B4U)
				if( nEnabledBitcoin & CDLS_BC_B4U )
				{
					m_pDevCmn->fnSCR_DisplayString( (10 + nRightFDK*2 + 2), B4U_LOGO_FILE );
					m_pDevCmn->fnSCR_DisplayString( (20 + nRightFDK*2 + 2), L"  ");
					strValueOfFdk[ nRightFDK*2 + 1] = L"B4U";
					nRightFDK ++;
				}
				#endif

				// F6, LibertyX (Sell)
				#if (APP_LIBERTYX)
				if( nEnabledBitcoin & CDLS_BC_LIBERTYX && m_LXConfig.SellBitcoinEnabled )
				{
					m_pDevCmn->fnSCR_DisplayString( (10 + nRightFDK*2 + 2), LIBERTYX_LOGO_FILE );
					m_pDevCmn->fnSCR_DisplayString( (20 + nRightFDK*2 + 2), L"  ");
					strValueOfFdk[ nRightFDK*2 + 1] = L"LIBERTYX_SELL";
					nRightFDK ++;
				}
				#endif

				// F8, (available)

				m_pDevCmn->fnSCR_DisplayScreen(153, KEYIN_TIME_OUT, PIN_MENU_MODE);
			}
			// End of [#2413]
			bShowScreen = FALSE;
		}

		// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2413]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2413]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
				if( IsAdaTransaction())		
				{
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2413]
				NHDEBUG(DBG_INFO, (_T("PRESS Cancel EXIT\n")));
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				return RES_USER_TIMEOUT;
			}
			else if ((GetKeyStr == L"F1") || (GetKeyStr == L"F2") || (GetKeyStr == L"F3") || (GetKeyStr == L"F4") ||
					 (GetKeyStr == L"F5") || (GetKeyStr == L"F6") || (GetKeyStr == L"F7") || (GetKeyStr == L"F8"))
			{
				int nFdkId = Asc2Int(GetKeyStr.Right(1)) - 1;

				if		(strValueOfFdk[nFdkId] == L"B4U")			return RES_BITCOIN_B4U; // [#RWC6-16] Bitload4U
				else if (strValueOfFdk[nFdkId] == L"LIBERTYX_SELL")	return RES_BITCOIN_LIBERTYX_SELL;
				else if (strValueOfFdk[nFdkId] == L"LIBERTYX_BUY")	return RES_BITCOIN_LIBERTYX_BUY;
			}
			// End of [#2557]
			else															// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney	
			{
				if( IsAdaTransaction() )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));
					int nValidADAInput = 0;
					arrVG.RemoveAll();

					if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
					{
						nValidADAInput = 1;					// (2) Repeat or Volume Control
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
					}
					// End of [#2446]
					
					// no B4U ADA yet
					/*
					else if( (GetKeyStr == L"6") && (nEnabledBitcoin & CDLS_BC_B4U) )
					{
						//
						arrVG.Add(L"6.wav");				arrVG.Add(L"B4UTr.wav");
						nValidADAInput = 6;					
					}
					*/
					// no LibertyX ADA yet
					/*
					else if( (GetKeyStr == L"7") && (nEnabledBitcoin & CDLS_BC_LIBERTYX) )
					{
						//
						arrVG.Add(L"7.wav");				arrVG.Add(L"B4UTr.wav");
						nValidADAInput = 7;					
					}
					*/

					// End of [#2445]

					if (nValidADAInput == 0)		// Invalid
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
						bInvalidRepeat = TRUE;
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
#endif
					}
					else if (nValidADAInput == 1)	// Repeat or Volume Control
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
					// else if( (nValidADAInput>=2)&&(nValidADAInput<=5) )			// Valid Input
					// {
					// 	NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
					// 	#if (US_VERSION)
					// 		m_pAdaCtrl->fnExp_ResetAndAddPlay(arrVG.GetAt(0), TRUE, arrVG.GetAt(1));
					// 	#endif

					// 	else if(nValidADAInput==6)	return RES_BITCOIN_B4U;	// no B4U ADA yet
					// 	else if(nValidADAInput==7)	return RES_BITCOIN_B4U;	// no LibertyX ADA yet
					// }
				}
			}
			// End of [#2413]
		}
		Delay_Msg(50);

		// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
		nTimeOutCheck = P_NH_ADA_Check_TimeOUT();
		if(nTimeOutCheck==RES_OK)				bShowScreen = TRUE;
		else if(nTimeOutCheck==RES_USER_EXIT)	return RES_USER_EXIT;
		// End of [#2413]
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_52");
	return RES_USER_TIMEOUT;
}
// end of [#RWC6-68]

#if (APP_POPMONEY) //  (APP_CUSTOM_PAI) [#2471] US Justin 2017.02.01 Enable Popmoney to all customers.
// [#2350] US Justin 2015.06.18 Add POP MOney
BIZ_RETURN	CTranCmn::P_NH_POPMoney_EnterAccessCode()
{
	NVDump('O', 'C', "00", L"P_POPMNY", L"EnterAccessCode");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_POPMoney_EnterAccessCode]\n"));

	CString			GetKeyStr;
	int				nRetryCount = 3;
	BOOL			bShowScreen = TRUE;

	// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
	// ADA Variable
	BIZ_RETURN nTimeOutCheck;
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CString	strADAVGInput;
	int		nValidADAInput = 0;				
	CString	strUserADAInputString;
	int		nUserInputValidation;

	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// End of [#2413]

	while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_78");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2413]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if (US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(147, L"147.wav"); 
				#endif
				m_pAdaCtrl->fnExp_AddWaveFile(147, L"LocateCancelKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(147, L"LocationRepeatKey.wav");				
				m_pAdaCtrl->fnExp_PlayScreenWave(147);
				strUserADAInputString = L"";
				bStartToTimeout = TRUE;
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(147);
				// Setting Screen
				{
					// History
					for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
					{
						if (nHistory == 0)
							m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

						m_pDevCmn->fnSCR_DisplayHistory(nHistory+1, m_arHistory[nHistory]);
					}

					// Popmoney Logo
					m_pDevCmn->fnSCR_DisplayString(2, POPMONEY_LOGO_FILE);

					// Sub Title
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_147001));
				}

				m_pDevCmn->fnSCR_DisplayScreen(147, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
			}
			// End of [#2413]
			bShowScreen = FALSE;
		}

		// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2413]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2413]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				// [#2413] US Justin 2016.04.08 Implement VG for Pin4 and Popmoney
				if( IsAdaTransaction())		
				{
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2413]
				NHDEBUG(DBG_INFO, (_T("PRESS Cancel EXIT\n")));
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr.GetLength() > 0)
			{
				// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
				nUserInputValidation = 0;
				if (!IsAdaTransaction())			// Screen Mode....
				{
					strUserADAInputString = GetKeyStr;
					nUserInputValidation = 1;		// Perform Validation
				}
				else								// ADA MODE
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));

					nValidADAInput = 0;				
					if ((GetKeyStr.GetLength() == 1) && (GetKeyStr != L"."))			// Numbers.... 0,1,2,3,4,5,6,7,8,9
					{
						strADAVGInput = GetKeyStr + L".wav";
						if( strUserADAInputString.GetLength() <= 10 )
							strUserADAInputString += GetKeyStr;
						nValidADAInput = 1;												// (1) Need to read entered key (US)
					}
					else if (GetKeyStr == S_CLEAR)										// Clear
					{
						strADAVGInput = L"ClearReEntry.wav";
						strUserADAInputString = L"";
						nValidADAInput = 1;												// (1) Need to read entered key (US)
					}
					else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )	// Repeat or Volume Change
					{
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
						nValidADAInput = 2;												// (2) Repeat of Volume Change
					}
					else if (GetKeyStr == S_ENTER)
					{
						NHDEBUG(DBG_INFO, (_T("ENTERED CODE [%s]\n"), strUserADAInputString));
						nUserInputValidation = 1;										// Perform Code Validation

						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Enter.wav", TRUE );		// Other countries => read at "MainFrm.cpp"
						#endif	
					}

					if( nValidADAInput == 1 )								// Read Entered keys (Numbers and Clear Key : US TTS Only)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance : Numbers and Clear Key \n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(strADAVGInput, FALSE);
						#endif
					}
					else if( nValidADAInput ==2 )							// 2(Repeat or Volume Change)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
				}

				if( nUserInputValidation == 1)
				{
					NHDEBUG(DBG_INFO, (L"Validating User Input[%s]\n", strUserADAInputString));
					if(	strUserADAInputString.GetLength() == 11)
					{
						CString sVirtualAccount = _T("");
						int nBinID = Asc2Int(strUserADAInputString.Left(1));
						switch (nBinID)
						{
							case 0:		sVirtualAccount = _T("506984");	break;
							case 1:		sVirtualAccount = _T("506985");	break;
							case 2:		sVirtualAccount = _T("506986");	break;
							case 3:		sVirtualAccount = _T("506987");	break;
							case 4:		sVirtualAccount = _T("506988");	break;
							case 5:		sVirtualAccount = _T("506989");	break;
							case 6:		sVirtualAccount = _T("506990");	break;
							case 7:		sVirtualAccount = _T("506991");	break;
							case 8:		sVirtualAccount = _T("506992");	break;
							case 9:		sVirtualAccount = _T("506993");	break;
						}
						sVirtualAccount += strUserADAInputString.Mid(1);
						NHDEBUG(DBG_INFO, (L"Virtual Account = [%s]\n", sVirtualAccount));
						if(sVirtualAccount.GetLength() == 16)
						{
							char tmpData[40];
							memset(tmpData, 0x00, sizeof(tmpData));
							WideToMulti(tmpData, sVirtualAccount, 40);
							if(IsValidMod10Data(tmpData,16))
							{
								m_sCardData.strAccountNo	= sVirtualAccount.Mid(3, 12);		// Card PAN for EPP
								m_sCardData.strBankID		= sVirtualAccount.Right(4);			// Bank ID
								m_sCardData.strISO2Data		= sVirtualAccount += L"=4912";		// Full Track2 Data....
								m_sCardData.strShowNumber.Format(L"************%s", m_sCardData.strBankID);
								// Save for reversal
								MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKCODE,	m_sCardData.strBankID);
								MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKACCOUNT,	m_sCardData.strAccountNo);	
								MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA,	m_sCardData.strShowNumber);
								MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2,		m_sCardData.strISO2Data);
								NHDEBUG(DBG_INFO, (L"MOD CHECK OK\n"));
								return RES_OK;
							}
							NHDEBUG(DBG_INFO, (L"MOD CHECK FAIL\n"));
						}
						else
							NHDEBUG(DBG_INFO, (L"length of Virtual account length is not 16\n"));
					}
					else
						NHDEBUG(DBG_INFO, (L"length of entered code is not 11\n"));

					nRetryCount--;
					if(nRetryCount>0)
					{
						if (!IsAdaTransaction())			// Screen Mode....
							LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_147101), INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
						else
						{
							#if (US_VERSION)
								m_pAdaCtrl->fnExp_ResetAndAddPlay(L"InvalidAccessCode.wav", TRUE);
							#endif
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						}
						bShowScreen = TRUE;
					}
				}
				// End of [#2413]
			}
		}
		Delay_Msg(50);

		// [#2413] US Justin 2016.04.13 VG for POPMoney and Pin4
		nTimeOutCheck = P_NH_ADA_Check_TimeOUT();
		if(nTimeOutCheck==RES_OK)				bShowScreen = TRUE;
		else if(nTimeOutCheck==RES_USER_EXIT)	return RES_USER_EXIT;
		// End of [#2413]
	}

	if (nRetryCount <= 0)
	{
		// [#2413] US Justin 2016.0413
		if (!IsAdaTransaction())			// Screen Mode....
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);
		// End of [#2413]

		NVDump('O', 'C', "00", L"P_NHNOR", L"NG_51");	// [#2024] NH KSK 2011.02.24
		return RES_USER_EXIT;
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}
// End of [#2350]
#endif

#if (APP_PIN4_CASHPICKUP) // [#2471] US Justin 2017.02.01 Enable Popmoney to all customers.
// [#2396] US Justin 2016.02.10 PIN4 Transaction
BIZ_RETURN	CTranCmn::P_NH_Pin4_Enter_PhoneNumber()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"_Pin4_Enter_Phone");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_Pin4_Enter_PhoneNumber]\n"));

	int			nRetryCount = 3;
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
	// ADA Variable
	BIZ_RETURN nTimeOutCheck;
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CString	strADAVGInput;
	int		nValidADAInput = 0;				
	CString	strUserADAInputString;
	int		nUserInputValidation;

	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// End of [#2413]

	while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_78");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2413]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if (US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(154, L"154.wav"); 
				#endif
				m_pAdaCtrl->fnExp_AddWaveFile(154, L"LocateCancelKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(154, L"LocationRepeatKey.wav");				
				m_pAdaCtrl->fnExp_PlayScreenWave(154);
				strUserADAInputString = L"";
				bStartToTimeout = TRUE;
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(154);

				// Setting Screen
				{
					// Pin4 Logo
					m_pDevCmn->fnSCR_DisplayString(20, m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_TOPCONER));

					// Title
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_154001));

					// Guide under input Box
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_154002));

					// Press Enter when Finished
					m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_154003));

					// BUTTON(F8) : International Numbers
					m_pDevCmn->fnSCR_DisplayString(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_154004));
				}

				m_pDevCmn->fnSCR_DisplayScreen(154, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
			}
			bShowScreen = FALSE;
		}

		// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2413]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2413]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
				if( IsAdaTransaction())		
				{
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2413]
				NHDEBUG(DBG_INFO, (_T("PRESS Cancel EXIT\n")));
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == L"F8")				// International Phone
			{
				BIZ_RETURN	nRes;

				// International Phone.. Country Code
				if ((nRes = P_NH_Pin4_Enter_International_PhoneNumber(TRUE)) != RES_OK)		// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
					return nRes;

				// International Phone.. Phone Number
				if ((nRes = P_NH_Pin4_Enter_International_PhoneNumber(FALSE)) != RES_OK)	// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
					return nRes;

				return RES_OK;
			}
			else 
			{
				// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
				nUserInputValidation = 0;
				if (!IsAdaTransaction())			// Screen Mode....
				{
					strUserADAInputString = GetKeyStr;
					nUserInputValidation = 1;		// Perform Validation
				}
				else								// ADA MODE
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));

					nValidADAInput = 0;				
					if ((GetKeyStr.GetLength() == 1) && (GetKeyStr != L"."))			// Numbers.... 0,1,2,3,4,5,6,7,8,9
					{
						strADAVGInput = GetKeyStr + L".wav";
						if( strUserADAInputString.GetLength() < 10 )
							strUserADAInputString += GetKeyStr;
						nValidADAInput = 1;												// (1) Need to read entered key (US)
					}
					else if (GetKeyStr == S_CLEAR)										// Clear
					{
						strADAVGInput = L"ClearReEntry.wav";
						strUserADAInputString = L"";
						nValidADAInput = 1;												// (1) Need to read entered key (US)
					}
					else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )	// Repeat or Volume Change
					{
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
						nValidADAInput = 2;												// (2) Repeat of Volume Change
					}
					else if (GetKeyStr == S_ENTER)
					{
						NHDEBUG(DBG_INFO, (_T("ENTERED CODE [%s]\n"), strUserADAInputString));
						nUserInputValidation = 1;										// Perform Code Validation

						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Enter.wav", TRUE );		// Other countries => read at "MainFrm.cpp"
						#endif	
					}

					if( nValidADAInput == 1 )								// Read Entered keys (Numbers and Clear Key : US TTS Only)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance : Numbers and Clear Key \n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(strADAVGInput, FALSE);
						#endif
					}
					else if( nValidADAInput ==2 )							// 2(Repeat or Volume Change)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
				}

				if( nUserInputValidation == 1)
				{
					NHDEBUG(DBG_INFO, (L"Validating User Input[%s]\n", strUserADAInputString));
					if(	strUserADAInputString.GetLength() == 10)
					{
						m_Pin4.m_strUSPhoneNumber = strUserADAInputString.Left(10);
						NHDEBUG(DBG_INFO, (_T("  Pin4 US Phone Number - [%s]\n"), m_Pin4.m_strUSPhoneNumber));
						return RES_OK;
					}
					else if( IsAdaTransaction() && (strUserADAInputString==L"1") )
					{
						NHDEBUG(DBG_INFO, (L"ADA MODE International Phone number input\n"));
						BIZ_RETURN	nRes;

						// International Phone.. Country Code
						if ((nRes = P_NH_Pin4_Enter_International_PhoneNumber(TRUE)) != RES_OK)		// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
							return nRes;

						// International Phone.. Phone Number
						if ((nRes = P_NH_Pin4_Enter_International_PhoneNumber(FALSE)) != RES_OK)	// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
							return nRes;

						return RES_OK;

					}
					else
						NHDEBUG(DBG_INFO, (L"length of entered string is not 10 or 1\n"));

					nRetryCount--;
					if(nRetryCount>0)
					{
						if (!IsAdaTransaction())			// Screen Mode....
							LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_154005), INFO_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
						else
						{
							#if (US_VERSION)
								m_pAdaCtrl->fnExp_ResetAndAddPlay(L"InvalidUSphoneNumber.wav", TRUE);
							#endif
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						}
						bShowScreen = TRUE;
					}
				}
				// End of [#2413]
			}
		}
		Delay_Msg(50);

		// [#2413] US Justin 2016.04.13 VG for POPMoney and Pin4
		nTimeOutCheck = P_NH_ADA_Check_TimeOUT();
		if(nTimeOutCheck==RES_OK)				bShowScreen = TRUE;
		else if(nTimeOutCheck==RES_USER_EXIT)	return RES_USER_EXIT;
		// End of [#2413]
	}

	if (nRetryCount <= 0)
	{
		// [#2413] US Justin 2016.0413
		if (!IsAdaTransaction())			// Screen Mode....
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);
		// End of [#2413]
		NVDump('O', 'C', "00", L"P_NHNOR", L"NG_51");	// [#2024] NH KSK 2011.02.24
		return RES_USER_EXIT;
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_52");
	return RES_USER_TIMEOUT;
}

BIZ_RETURN	CTranCmn::P_NH_Pin4_Enter_International_PhoneNumber(BOOL bCountryCode)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"_Pin4_Enter_Int_PhoneNumber");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_Pin4_Enter_International_PhoneNumber(%d)]\n", bCountryCode));

	int			nRetryCount = 3;
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
	// ADA Variable
	BIZ_RETURN nTimeOutCheck;
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CString	strADAVGInput;
	int		nValidADAInput = 0;				
	CString	strUserADAInputString;
	int		nUserInputValidation;

	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// End of [#2413]

	while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
	{
		// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_78");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2413]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if (US_VERSION)
					if(bCountryCode)	m_pAdaCtrl->fnExp_AddWaveFile(155, L"155_1.wav"); 
					else				m_pAdaCtrl->fnExp_AddWaveFile(155, L"155_2.wav"); 
				#endif
				m_pAdaCtrl->fnExp_AddWaveFile(155, L"LocateCancelKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(155, L"LocationRepeatKey.wav");				
				m_pAdaCtrl->fnExp_PlayScreenWave(155);
				strUserADAInputString = L"";
				bStartToTimeout = TRUE;
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(155);

				// Setting Screen
				{
					// Pin4 Logo
					m_pDevCmn->fnSCR_DisplayString(20, m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_TOPCONER));

					// Title
					if(bCountryCode)
						m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_155001));
					else
						m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_155101));

					// Press Enter when Finished
					m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_155002));
				}

				m_pDevCmn->fnSCR_DisplayScreen(155, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
			}
			// End of [#2413]
			bShowScreen = FALSE;
		}

		// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2413]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2413]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
				if( IsAdaTransaction())		
				{
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2413]

				NHDEBUG(DBG_INFO, (_T("PRESS Cancel EXIT\n")));
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				return RES_USER_TIMEOUT;
			}
			else 
			{
				// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
				nUserInputValidation = 0;
				if (!IsAdaTransaction())			// Screen Mode....
				{
					strUserADAInputString = GetKeyStr;
					nUserInputValidation = 1;		// Perform Validation
				}
				else								// ADA MODE
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));

					nValidADAInput = 0;				
					if ((GetKeyStr.GetLength() == 1) && (GetKeyStr != L"."))			// Numbers.... 0,1,2,3,4,5,6,7,8,9
					{
						strADAVGInput = GetKeyStr + L".wav";
						if( strUserADAInputString.GetLength() <= 10 )
							strUserADAInputString += GetKeyStr;
						nValidADAInput = 1;												// (1) Need to read entered key (US)
					}
					else if (GetKeyStr == S_CLEAR)										// Clear
					{
						strADAVGInput = L"ClearReEntry.wav";
						strUserADAInputString = L"";
						nValidADAInput = 1;												// (1) Need to read entered key (US)
					}
					else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )	// Repeat or Volume Change
					{
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
						nValidADAInput = 2;												// (2) Repeat of Volume Change
					}
					else if (GetKeyStr == S_ENTER)
					{
						NHDEBUG(DBG_INFO, (_T("ENTERED CODE [%s]\n"), strUserADAInputString));
						nUserInputValidation = 1;										// Perform Code Validation

						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Enter.wav", TRUE );		// Other countries => read at "MainFrm.cpp"
						#endif	
					}

					if( nValidADAInput == 1 )								// Read Entered keys (Numbers and Clear Key : US TTS Only)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance : Numbers and Clear Key \n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(strADAVGInput, FALSE);
						#endif
					}
					else if( nValidADAInput ==2 )							// 2(Repeat or Volume Change)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
				}

				if( nUserInputValidation == 1)
				{
					NHDEBUG(DBG_INFO, (L"Validating User Input[%s]\n", strUserADAInputString));
					if(	strUserADAInputString.GetLength() > 0)
					{
						if(bCountryCode)	m_Pin4.m_strIntPhoneCountryCode = strUserADAInputString;
						else				m_Pin4.m_strIntPhoneNumber = strUserADAInputString;
						NHDEBUG(DBG_INFO, (_T("  Pin4 International Phone Number Input - [%s]\n"), strUserADAInputString));
						return RES_OK;
					}
					else
						NHDEBUG(DBG_INFO, (L"length of entered string is equal or less than 0\n"));

					nRetryCount--;
					if(nRetryCount>0)
					{
						if (!IsAdaTransaction())			// Screen Mode....
						{
							if(bCountryCode)	LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_155201), INFO_SCR_TIMEOUT);
							else 				LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_155202), INFO_SCR_TIMEOUT);
						}
						else
						{
							#if (US_VERSION)
								m_pAdaCtrl->fnExp_ResetAndAddPlay(L"InvalidInternationalPhoneNumber.wav", TRUE);
							#endif
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						}
						bShowScreen = TRUE;
					}
				}
				// End of [#2413]
			}
		}
		Delay_Msg(50);
		// [#2413] US Justin 2016.04.13 VG for POPMoney and Pin4
		nTimeOutCheck = P_NH_ADA_Check_TimeOUT();
		if(nTimeOutCheck==RES_OK)				bShowScreen = TRUE;
		else if(nTimeOutCheck==RES_USER_EXIT)	return RES_USER_EXIT;
		// End of [#2413]
	}

	if (nRetryCount <= 0)
	{
		NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
		// [#2413] US Justin 2016.0413
		if (!IsAdaTransaction())			// Screen Mode....
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);
		// End of [#2413]
		NVDump('O', 'C', "00", L"P_NHNOR", L"NG_51");
		return RES_USER_EXIT;
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_52");
	return RES_USER_TIMEOUT;
}

BIZ_RETURN	CTranCmn::P_NH_Pin4_Enter_Pin4_Code(BOOL bSecretCode)
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"_Pin4_Pin4_Code");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_Pin4_Enter_Pin4_Code(%d)]\n", bSecretCode));

	int			nRetryCount = 3;
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
	// ADA Variable
	BIZ_RETURN nTimeOutCheck;
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CString	strADAVGInput;
	int		nValidADAInput = 0;				
	CString	strUserADAInputString;
	int		nUserInputValidation;

	//g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// End of [#2413]

	while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
	{
		// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_78");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2413]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if (US_VERSION)
					if(bSecretCode)		m_pAdaCtrl->fnExp_AddWaveFile(156, L"156_1.wav"); 
					else				m_pAdaCtrl->fnExp_AddWaveFile(156, L"156_2.wav"); 
				#endif
				m_pAdaCtrl->fnExp_AddWaveFile(156, L"LocateCancelKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(156, L"LocationRepeatKey.wav");				
				m_pAdaCtrl->fnExp_PlayScreenWave(156);
				strUserADAInputString = L"";
				bStartToTimeout = TRUE;
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(156);

				// Setting Screen
				{
					// Pin4 Logo
					m_pDevCmn->fnSCR_DisplayString(20, m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_TOPCONER));

					// Title
					if( bSecretCode)
						m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_156101));
					else
						m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_156001));

					// Guide under input Box
					if( bSecretCode)
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_156102));
					else
						m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_156002));

					// Press Enter when Finished
					m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_156003));

					// Order Number Prefix
					// [#2498] US Justin 2017.08.17
					if( !bSecretCode)
						m_pDevCmn->fnSCR_DisplayString(6, "MC");
					// End of [#2498]

				}

				m_pDevCmn->fnSCR_DisplayScreen(156, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
			}
			// End of [#2413]
			bShowScreen = FALSE;
		}

		// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2413]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2413]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NHDEBUG(DBG_INFO, (_T("PRESS Cancel EXIT\n")));
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				return RES_USER_TIMEOUT;
			}
			else 
			{
				// [#2413] US Justin 2016.04.13 Implement VG for Pin4 and Popmoney
				nUserInputValidation = 0;
				if (!IsAdaTransaction())			// Screen Mode....
				{
					strUserADAInputString = GetKeyStr;
					nUserInputValidation = 1;		// Perform Validation
				}
				else								// ADA MODE
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));

					nValidADAInput = 0;				
					if ((GetKeyStr.GetLength() == 1) && (GetKeyStr != L"."))			// Numbers.... 0,1,2,3,4,5,6,7,8,9
					{
						strADAVGInput = GetKeyStr + L".wav";
						if( strUserADAInputString.GetLength() < 4 )
							strUserADAInputString += GetKeyStr;
						nValidADAInput = 1;												// (1) Need to read entered key (US)
					}
					else if (GetKeyStr == S_CLEAR)										// Clear
					{
						strADAVGInput = L"ClearReEntry.wav";
						strUserADAInputString = L"";
						nValidADAInput = 1;												// (1) Need to read entered key (US)
					}
					else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )	// Repeat or Volume Change
					{
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
						nValidADAInput = 2;												// (2) Repeat of Volume Change
					}
					else if (GetKeyStr == S_ENTER)
					{
						NHDEBUG(DBG_INFO, (_T("ENTERED CODE [%s]\n"), strUserADAInputString));
						nUserInputValidation = 1;										// Perform Code Validation

						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Enter.wav", TRUE );		// Other countries => read at "MainFrm.cpp"
						#endif	
					}

					if( nValidADAInput == 1 )								// Read Entered keys (Numbers and Clear Key : US TTS Only)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance : Numbers and Clear Key \n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(strADAVGInput, FALSE);
						#endif
					}
					else if( nValidADAInput ==2 )							// 2(Repeat or Volume Change)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
				}

				if( nUserInputValidation == 1)
				{
					NHDEBUG(DBG_INFO, (L"Validating User Input[%s]\n", strUserADAInputString));
					if(	strUserADAInputString.GetLength() == 4)
					{
						if( bSecretCode)	m_Pin4.m_strSecretPin = strUserADAInputString;
						else				m_Pin4.m_strPin4Code  = strUserADAInputString;
						NHDEBUG(DBG_INFO, (_T("  Entered Pin4 Code - [%s]\n"), strUserADAInputString));
						return RES_OK;
					}
					else
						NHDEBUG(DBG_INFO, (L"length of entered string is not 4\n"));

					nRetryCount--;
					if(nRetryCount>0)
					{
						if (!IsAdaTransaction())			// Screen Mode....
							LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_156201), INFO_SCR_TIMEOUT);
						else
						{
							#if (US_VERSION)
								m_pAdaCtrl->fnExp_ResetAndAddPlay(L"InvalidCode.wav", TRUE);
							#endif
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						}
						bShowScreen = TRUE;
					}
				}
				// End of [#2413]
			}
		}
		Delay_Msg(50);
		// [#2413] US Justin 2016.04.13 VG for POPMoney and Pin4
		nTimeOutCheck = P_NH_ADA_Check_TimeOUT();
		if(nTimeOutCheck==RES_OK)				bShowScreen = TRUE;
		else if(nTimeOutCheck==RES_USER_EXIT)	return RES_USER_EXIT;
		// End of [#2413]
	}

	if (nRetryCount <= 0)
	{
		NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
		// [#2413] US Justin 2016.0413
		if (!IsAdaTransaction())			// Screen Mode....
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);
		// End of [#2413]
		NVDump('O', 'C', "00", L"P_NHNOR", L"NG_51");
		return RES_USER_EXIT;
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_52");
	return RES_USER_TIMEOUT;
}

// [#2515] US Justin 2017.11.14 Pin4 Prestaging
BIZ_RETURN	CTranCmn::P_NH_Pin4_Auth_PIN4()
{
	NVDump('O', 'C', "00", L"P_PIN4", L"P4_AUTH");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_Pin4_Auth_PIN4]\n"));

	#if(!APP_PRESTAGIN_PIN4)
	// No need to Display screen for Prestaging Mode....
	// DISPLAY SCREEN or Play VG
	if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_DISABLE_MODE);
		m_pAdaCtrl->fnExp_StopAndResetWaveFile();
		#if (US_VERSION)
			m_pAdaCtrl->fnExp_AddWaveFile(125, L"125.wav");
		#endif
		m_pAdaCtrl->fnExp_PlayScreenWave(125);	//"Please Wait a moment" 125.wav
		m_pAdaCtrl->fnExp_WaitUntilStop();
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(161);
		//	m_pDevCmn->fnSCR_DisplayImage(1, FALSE);													// Hide Exit Button	// [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
		m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161001));		// Please Wait
		m_pDevCmn->fnSCR_DisplayString(2, L"" );														// Hide Cancel F8
		m_pDevCmn->fnSCR_DisplayString(3, L"" );														// NO Background Image
		m_pDevCmn->fnSCR_DisplayScreen(161);
	}
	#endif
	 
	m_Pin4.m_nTransactionStep = PIN4_TRAN_AUTH_PIN4;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if(nRes == RES_OK)	
	{
		NHDEBUG(DBG_CALL, (L"PIN4 AUTH OK\n"));
		
		// Assign received PAN for ATM Processor Authorization
		#if(APP_PRESTAGIN_PIN4)
			m_sCardData.strAccountNo	= m_Pin4.m_strVCN.Mid(3, 12);		// Card PAN for EPP
			m_sCardData.strBankID		= m_Pin4.m_strVCN.Right(4);			// Bank ID
			m_sCardData.strISO2Data		= m_Pin4.m_strVCN;					// Full Track2 Data....
			m_sCardData.strISO2Data		+= L"=";	
			// [#2568] US Justin 2018.07.30 PIN4 : Sending YYMM data to ATM processor 
			/*
			m_sCardData.strISO2Data		+= m_Pin4.m_strExpirationMMYY.Right(2);	// YY
			m_sCardData.strISO2Data		+= m_Pin4.m_strExpirationMMYY.Left(2);	// MM
			*/
			m_sCardData.strISO2Data		+= m_Pin4.m_strExpirationYYMM.Left(4);	// Spec Changed MMYY => YYMM (7/31/18)
			// End of [#2568]
			m_sCardData.strShowNumber.Format(L"************%s", m_sCardData.strBankID);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKCODE,	m_sCardData.strBankID);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKACCOUNT,	m_sCardData.strAccountNo);	
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA,	m_sCardData.strShowNumber);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2,		m_sCardData.strISO2Data);
		#endif
	}
	else
	{
		NHDEBUG(DBG_CALL, (L"FAIL TO PIN4 AUTH\n"));

		// [#2576] US Justin 2018.09.07  Display Error Description
		if(m_Pin4.m_strErrorDesc.GetLength() > 0)
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_Pin4.m_strErrorCode + _T(", ") + m_Pin4.m_strErrorDesc, INFO_SCR_TIMEOUT);	
		// End of [#2576]

	}
	return nRes;
}

BIZ_RETURN	CTranCmn::P_NH_Pin4_Reversal_PIN4()
{
	NVDump('O', 'C', "00", L"P_PIN4", L"P4_REV");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_Pin4_Reversal_PIN4]\n"));

	// DISPLAY SCREEN or Play VG
	if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_DISABLE_MODE);
		m_pAdaCtrl->fnExp_StopAndResetWaveFile();
		#if (US_VERSION)
			m_pAdaCtrl->fnExp_AddWaveFile(125, L"125.wav");
		#endif
		m_pAdaCtrl->fnExp_PlayScreenWave(125);	//"Please Wait a moment" 125.wav
		m_pAdaCtrl->fnExp_WaitUntilStop();
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(161);
		//m_pDevCmn->fnSCR_DisplayImage(1, FALSE);														// Hide Exit Button		// [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
		m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161001));		// Please Wait
		m_pDevCmn->fnSCR_DisplayString(2, L"" );														// Hide Cancel F8
		m_pDevCmn->fnSCR_DisplayString(3, L"" );														// NO Background Image
		m_pDevCmn->fnSCR_DisplayScreen(161);
	}
	 
	m_Pin4.m_nTransactionStep = PIN4_TRAN_REVERSAL_PIN4;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if(nRes == RES_OK)	NHDEBUG(DBG_CALL, (L"PIN4 Reversal OK\n"));
	else				NHDEBUG(DBG_CALL, (L"FAIL TO PIN4 Reversal\n"));
	return nRes;
}

#if(APP_PRESTAGIN_PIN4)
BIZ_RETURN	CTranCmn::P_NH_Pin4_GetToken()
{
	NVDump('O', 'C', "00", L"P_PIN4", L"GETTOKEN");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_Pin4_GetToken]\n"));

	// DISPLAY SCREEN or Play VG
	if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_DISABLE_MODE);
		m_pAdaCtrl->fnExp_StopAndResetWaveFile();
		#if (US_VERSION)
			m_pAdaCtrl->fnExp_AddWaveFile(125, L"125.wav");
		#endif
		m_pAdaCtrl->fnExp_PlayScreenWave(125);	//"Please Wait a moment" 125.wav
		m_pAdaCtrl->fnExp_WaitUntilStop();
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(161);
		//m_pDevCmn->fnSCR_DisplayImage(1, FALSE);														// Hide Exit Button		// [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
		m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161001));		// Please Wait
		m_pDevCmn->fnSCR_DisplayString(2, L"" );														// Hide Cancel F8
		m_pDevCmn->fnSCR_DisplayString(3, L"" );														// NO Background Image
		m_pDevCmn->fnSCR_DisplayScreen(161);
	}	 

	m_Pin4.m_nTransactionStep = PIN4_TRAN_GETTOKEN;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);
	if(nRes == RES_OK)	
	{
		if( (m_Pin4.m_nLenPrivateKey <=0)	|| (m_Pin4.m_pPrivateKey == NULL) || (m_Pin4.m_nLenSymmetricKey <=0) || (m_Pin4.m_pSymmetricKey == NULL) )
		{
			NHDEBUG(DBG_CALL, (L"ERROR : GetToken OK, But PRIVATE KEY or SYMM KEY IS NULL\n"));
			nRes = RES_NG;
		}
		else
			NHDEBUG(DBG_CALL, (L"GetToken OK\n"));
	}
	else
	{
		NHDEBUG(DBG_CALL, (L"Fail to get Token\n"));

		// [#2576] US Justin 2018.09.17  Display Error Description
		if(m_Pin4.m_strErrorDesc.GetLength() > 0)
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_Pin4.m_strErrorCode + _T(", ") + m_Pin4.m_strErrorDesc, INFO_SCR_TIMEOUT);	
		// End of [#2576]
	}
	return nRes;
}

BIZ_RETURN	CTranCmn::P_NH_Pin4_ReEnter_SecretCode()
{
	NVDump('O', 'C', "00", L"P_PIN4", L"ReEnterCode");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_Pin4_ReEnter_SecretCode]\n"));

	CString			GetKeyStr;
	DWORD			dwTickStart = 0, dwTickEnd = 0;
	BOOL			bShowScreen = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)					// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// ADA Variable
	BOOL	bStartToTimeout = FALSE;
	int		nInputCount;
	if(IsAdaTransaction())
		m_bAdaPasswordMode = TRUE;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if( (IsAdaTransaction()) && (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE) )
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			m_bAdaPasswordMode = FALSE;
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_75");
			return RES_USER_EXIT;
		}

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, KEYIN_TIME_OUT, PIN_PASSWORD_MODE, L"", PIN_PASSWORD_MIN, PIN_PASSWORD_MAX, PIN_PASSWORD_AUTO_TRUE, PIN_PASSWORD_TERM);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if (US_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFile(156, L"156_4.wav"); 
				#endif
				m_pAdaCtrl->fnExp_AddWaveFile(156, L"LocateCancelKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(156, L"LocationRepeatKey.wav");				
				m_pAdaCtrl->fnExp_PlayScreenWave(156);
				bStartToTimeout = TRUE;
				nInputCount = 0;
			}
			else
			{			
				m_pDevCmn->fnSCR_DisplayPrevSet(103);
				// Setting Screen
				{
					// Title
					//m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_103001));

					// Sub Title
					CString strRenterCode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_156401) + m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_156003); 
					m_pDevCmn->fnSCR_DisplayString(3, strRenterCode);

					// Guide
					m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_156102));
				}

				dwTickStart = GetTickCount();
				NHDEBUG(DBG_INFO, (L"CARD ACCOUNT NO : [%s]\n", m_sCardData.strAccountNo));
				m_pDevCmn->fnSCR_DisplayScreen(103, KEYIN_TIME_OUT, PIN_PASSWORD_MODE, L"", PIN_PASSWORD_MIN, PIN_PASSWORD_MAX, PIN_PASSWORD_AUTO_TRUE, PIN_PASSWORD_TERM);

			}
			bShowScreen = FALSE;
		}

		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE))
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE = [%s]\n"), GetKeyStr));

			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}

			dwTickEnd = GetTickCount();			

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NHDEBUG(DBG_INFO, (L"ENTER PIN : CANCEL or EXIT Pressed\n"));
				if( IsAdaTransaction())		
				{
					m_bAdaPasswordMode = FALSE;			
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay("Cancel.wav", TRUE);
					#endif
				}
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_5");
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_6");
				m_bAdaPasswordMode = FALSE;	
				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == L"LOAD_OK")		// Non ADA Response
			{
				NHDEBUG(DBG_INFO, (_T("PASSWORD PIN ENABLE..\n")));
				m_pDevCmn->fnSCR_DisplayScreen(0, 0, PIN_PASSWORD_MODE, L"", PIN_PASSWORD_MIN, PIN_PASSWORD_MAX, PIN_PASSWORD_AUTO_TRUE, PIN_PASSWORD_TERM);	
			}
			else if (GetKeyStr == L"PIN")			// Non ADA Response
			{
				NHDEBUG(DBG_INFO, (L"ENTER PIN : PIN returned\n"));				
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);								// Read Complete�� �ö����� ���
				return RES_OK;
			}
			else
			{
				if( IsAdaTransaction())		
				{
					if (GetKeyStr == S_CLEAR)
					{
						NHDEBUG(DBG_INFO, (L"ENTER PIN : CLEAR Pressed\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"103_1.wav", FALSE);
						#endif
						nInputCount = 0;
					}
					else if (GetKeyStr == S_ENTER)
					{
						NHDEBUG(DBG_INFO, (L"ENTER PIN : ENTER Pressed\n"));
						m_bAdaPasswordMode = FALSE;
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"ENTER.wav", TRUE);
						#endif
						m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);
						return RES_OK;
					}
					else if (GetKeyStr.GetLength() == 1)
					{
						NHDEBUG(DBG_INFO, (L"ENTER PIN : 1 Digit Pressed\n"));

						#if (US_VERSION)
							m_pAdaCtrl->fnExp_StopPlay();
						#endif
						nInputCount++;
						if (nInputCount == 12)
							m_pDevCmn->fnSCR_SetByPassData(S_ENTER);
					}
				}
			}
		}
		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_7");
	m_bAdaPasswordMode = FALSE;	
	return RES_USER_TIMEOUT;
}

BIZ_RETURN	CTranCmn::P_NH_Pin4_Auth_Processor()
{
	NVDump('O', 'C', "00", L"P_PIN4", L"AUTH_PROC");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_Pin4_Auth_Processor]\n"));

	// Reset Result... Filled when ATM makes PIN4 Authorization
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE, L"");
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME, L"");
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSSETTLEDATE, L"");
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM, L"");
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID, L"");

	m_Pin4.m_nTransactionStep = PIN4_TRAN_AUTH_PROC;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(TRUE);
	if(nRes == RES_OK)	NHDEBUG(DBG_CALL, (L"PROCESSOR AUTH OK\n"));
	else				NHDEBUG(DBG_CALL, (L"PROCESSOR AUTH  FAIL\n"));
	return nRes;
}
#endif
// End of [#2515]

#endif
// End of [#2396]

#if (APP_JUST_CASH)
BIZ_RETURN CTranCmn::P_NH_JUSTCASH_RegisterService()
{
	NVDump('O', 'C', "00", L"JUSTCSH", L"REG_SVC");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_JUSTCASH_RegisterService()]\n"));

	TranCode = TC_JUSTCASH;
	m_JustCashData.m_nTransactionStep = JUSTCASH_SETUP_LOGIN;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if(nRes == RES_OK)	NHDEBUG(DBG_CALL, (L"LOGIN OK\n"));
	else				NHDEBUG(DBG_CALL, (L"Fail to LOGIN\n"));
	return nRes;
}
// End of [#2448]
#endif
// End of [#2445]

// [#2446] US Justin 2016.09.30 Paypal CCA
#if (APP_PAYDIANT_CCA)
BIZ_RETURN	CTranCmn::P_NH_PaypalCCA_GetToken()
{
	NVDump('O', 'C', "00", L"P_CCA_TOKEN", L"Paypal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_PaypalCCA_GetToken]\n"));

	// DISPLAY SCREEN
	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	//m_pDevCmn->fnSCR_DisplayImage(1, FALSE);														// Hide Exit Button		// [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
	m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161001));		// Please Wait
	m_pDevCmn->fnSCR_DisplayString(2, L"" );														// Hide Cancel F8
	m_pDevCmn->fnSCR_DisplayString(3, L"" );														// NO Background Image
	m_pDevCmn->fnSCR_DisplayScreen(161);

	m_PayPalCCA.m_nTransactionStep = PAYPAL_TRAN_GETTOKEN;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);
	if(nRes == RES_OK)	NHDEBUG(DBG_CALL, (L"GetToken OK\n"));
	else				NHDEBUG(DBG_CALL, (L"Fail to get Token\n"));
	return nRes;
}

// [#2523] NH Justin 2017.12.21 Paypal - Add Cancel State before retrieve customer info.
BIZ_RETURN	CTranCmn::P_NH_PaypalCCA_WaitUserCancel()
{
	NVDump('O', 'C', "00", L"P_CCA_USERCNL", L"Paypal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_PaypalCCA_WaitUserCancel]\n"));

	// Delete Existing QR Image
	DeleteFile(QR_RECEIPT_IMAGEFILE);		

	// Create QR Image
	MakeQRCodeImageFile(m_PayPalCCA.m_strAccessToken, QR_RECEIPT_IMAGEFILE);
	if (!IsExistFile(QR_RECEIPT_IMAGEFILE))
	{
		NHDEBUG(DBG_CALL, (L"ERROR on Finding QR Image file [%s] RETURN RES_NG\n", QR_RECEIPT_IMAGEFILE));
		return RES_NG;
	}

	NHDEBUG(DBG_CALL, (L"Found QR Image file [%s]\n", QR_RECEIPT_IMAGEFILE));

	CString GetKeyStr;
	BOOL	bShowScreen = TRUE;
	int		nScreenTimeout = 8;	// Wait 8 seconds

	while (TRUE)
	{
		if (bShowScreen == TRUE)
		{
			///////////////////////////////////
			// DISPLAY SCREEN
			bShowScreen = FALSE;	
			m_pDevCmn->fnSCR_DisplayPrevSet(162);
			m_pDevCmn->fnAPL_DisplayQRCodeOnScreen(QR_RECEIPT_IMAGEFILE);
			// [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
			//m_pDevCmn->fnSCR_DisplayImage(1, TRUE);								// Show Exit Button
			m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_EXIT));			
			// End of [#2529]
			m_pDevCmn->fnSCR_DisplayScreen(162, nScreenTimeout, PIN_MENU_MODE);
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			if (GetKeyStr == S_TIMEOVER)		// TIME OUT : User does not press Exit Button
				return RES_OK;
			else								// User Pressed Exit Button
			{
				// [#2526] US Justin 2018.01.12 Paypal - Continue Transaction if User Cancel (Release token) Failure
				// Transaction Cancelled Screen
				/*
				m_pDevCmn->fnSCR_DisplayPrevSet(122);
				m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
				m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122001));
				m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
				m_pDevCmn->fnSCR_DisplayScreen(122);

				// RELEASE TOKEN
				NHDEBUG(DBG_CALL, (L"User Cancel ==> Release Token\n"));
				m_PayPalCCA.m_nTransactionStep = PAYPAL_TRAN_RELEASETOKEN;
				P_NH_NOR_Transaction(FALSE);
				return RES_USER_EXIT;
				*/

				// Please Wait
				m_pDevCmn->fnSCR_DisplayPrevSet(161);
				//m_pDevCmn->fnSCR_DisplayImage(1, FALSE);														// Hide Exit Button		// [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
				m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161002));
				m_pDevCmn->fnSCR_DisplayString(2, L"" );														// Hide Cancel F8
				m_pDevCmn->fnSCR_DisplayString(3, L"" );														// NO Background Image
				m_pDevCmn->fnSCR_DisplayScreen(161);

				// RELEASE TOKEN
				NHDEBUG(DBG_CALL, (L"User Cancel ==> Release Token\n"));
				m_PayPalCCA.m_nTransactionStep = PAYPAL_TRAN_RELEASETOKEN;
				BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);
				if( nRes == RES_OK )
					return RES_USER_EXIT;

				// IF ATM FAILS TO RELEASE TOKEN, CONTINUE TRANSACTION
				m_PayPalCCA.m_bUserCancel = TRUE;
				return RES_OK;
				// End of [#2526]
			}
		}
		Delay_Msg(50);
	}
	return RES_OK;
}
// End of [#2523]

BIZ_RETURN	CTranCmn::P_NH_PaypalCCA_ShowQR_GetCustomerInfo()
{
	NVDump('O', 'C', "00", L"P_CCA_CUSINFO", L"Paypal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_PaypalCCA_ShowQR_GetCustomerInfo]\n"));

	// [#2523] NH Justin 2017.12.21 Paypal - Add Cancel State before retrieve customer info.
	/*
	// QR Code Generation => Move to the previous state
	CString strParam = _T("");
	strParam.Format(L"\"%s\" %s", m_PayPalCCA.m_strAccessToken, QR_RECEIPT_IMAGEFILE);
	
	// Delete Existing QR Image
	DeleteFile(QR_RECEIPT_IMAGEFILE);		

	// Execute QREncode Generator
	SHELLEXECUTEINFO ShExecInfo = {0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = QRCODE_GENERATOR;	
	ShExecInfo.lpParameters = strParam;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL; 
	ShellExecuteEx(&ShExecInfo);

	// [#2503] US Justin
	//WaitForSingleObject(ShExecInfo.hProcess,INFINITE);
	WaitForSingleObject(ShExecInfo.hProcess, 5 * 1000);	// Wait 5 seconds.

	// Find Created QR IMAGE
	WIN32_FIND_DATA FileData;
	HANDLE h = INVALID_HANDLE_VALUE;
	if( (h = FindFirstFile( QR_RECEIPT_IMAGEFILE, &FileData )) == INVALID_HANDLE_VALUE  )
	{
		NHDEBUG(DBG_CALL, (L"ERROR on Finding QR Image file [%s] RETURN RES_NG\n", QR_RECEIPT_IMAGEFILE));
		return RES_NG;
	}	
	FindClose(h);
	NHDEBUG(DBG_CALL, (L"Found QR Image file [%s]\n", QR_RECEIPT_IMAGEFILE));
	// End of [#2503]
	*/
	// End of [#2523]

	// [#2526] US Justin 2018.01.12 Paypal - Continue Transaction if User Cancel (Release token) Failure
	/*
	m_pDevCmn->fnSCR_DisplayPrevSet(162);
	m_pDevCmn->fnAPL_DisplayQRCodeOnScreen(QR_RECEIPT_IMAGEFILE);
	m_pDevCmn->fnSCR_DisplayImage(1, FALSE);								// Hide Exit Button
	m_pDevCmn->fnSCR_DisplayScreen(162);
	*/
	if(m_PayPalCCA.m_bUserCancel != TRUE)
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(162);
		m_pDevCmn->fnAPL_DisplayQRCodeOnScreen(QR_RECEIPT_IMAGEFILE);
		//m_pDevCmn->fnSCR_DisplayImage(1, FALSE);							// Hide Exit Button		// [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
		m_pDevCmn->fnSCR_DisplayScreen(162);
	}
	// End of [#2526]

	#ifdef APP_LOCAL_MODE
		Delay_Msg(5000);		//  Displaying QR CODE and Wait (Delay for DEMO MODE)
	#endif

	m_PayPalCCA.m_nTransactionStep = PAYPAL_TRAN_GETCUSTOMERINFO;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);
	if(nRes == RES_OK)	
	{
		NHDEBUG(DBG_CALL, (L"Get Customer Info OK \n"));
		return RES_OK;
	}

	// FAIL TO GET CUSTOMER INFO => RELEASE TOKEN
	// DISPLAY WAIT SCREEN
	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	//m_pDevCmn->fnSCR_DisplayImage(1, FALSE);														// Hide Exit Button		// [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
	m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161001));		// Please Wait
	m_pDevCmn->fnSCR_DisplayString(2, L"" );														// Hide Cancel F8
	m_pDevCmn->fnSCR_DisplayString(3, L"" );														// NO Background Image
	m_pDevCmn->fnSCR_DisplayScreen(161);

	NHDEBUG(DBG_CALL, (L"Fail to get customer info ==> Release Token\n"));
	m_PayPalCCA.m_nTransactionStep = PAYPAL_TRAN_RELEASETOKEN;
	nRes = P_NH_NOR_Transaction(FALSE);
	return RES_NG;
}

BIZ_RETURN	CTranCmn::P_NH_PaypalCCA_GetCashTicket()
{
	NVDump('O', 'C', "00", L"P_CCA_CSHTKT", L"Paypal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_PaypalCCA_GetCashTicket]\n"));

	// DISPLAY SCREEN
	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	//m_pDevCmn->fnSCR_DisplayImage(1, FALSE);														// Hide Exit Button
	m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161002));
	m_pDevCmn->fnSCR_DisplayString(2, L"" );														// Hide Cancel F8
	m_pDevCmn->fnSCR_DisplayString(3, L"" );														// NO Background Image
	m_pDevCmn->fnSCR_DisplayScreen(161);

	m_PayPalCCA.m_nTransactionStep = PAYPAL_TRAN_GETWITHDRAWALTICKET;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);
	if(nRes == RES_OK)
	{
		NHDEBUG(DBG_CALL, (L"Get Cash Ticket OK \n"));
		int nInputAmountFull   = Dollar2Cent(m_PayPalCCA.m_strReqAmount);
		int nInputAmountDollar = (int) (0.01*nInputAmountFull + 0.0001);
		
		// [#2466] US Justin 2017.01.27 Paydiant Certification
		/*
		if( (nInputAmountFull == (nInputAmountDollar*100)) && (m_pDevCmn->fbCDU_IsDispensible(nInputAmountDollar) ) )
		{
			m_sUserSelection.strMoney.Format(L"%010d00", nInputAmountDollar);
			m_sCardData.strAccountNo	= m_PayPalCCA.m_strCW_CardNum.Mid(3, 12);		// Card PAN for EPP
			m_sCardData.strBankID		= m_PayPalCCA.m_strCW_CardNum.Right(4);			// Bank ID
			m_sCardData.strISO2Data		= m_PayPalCCA.m_strCW_CardNum;					// Full Track2 Data....
			m_sCardData.strISO2Data		+= L"=";	
			m_sCardData.strISO2Data		+= m_PayPalCCA.m_strCW_CardExpDate.Right(2);	// YY
			m_sCardData.strISO2Data		+= m_PayPalCCA.m_strCW_CardExpDate.Left(2);		// MM
			m_sCardData.strShowNumber.Format(L"************%s", m_sCardData.strBankID);

			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKCODE,	m_sCardData.strBankID);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKACCOUNT,	m_sCardData.strAccountNo);	
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA,	m_sCardData.strShowNumber);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2,		m_sCardData.strISO2Data);
			return RES_OK;
		}
		*/

		BOOL bValidAmount = FALSE;	
		CString strUserNotice = L"";
		if( (nInputAmountFull == (nInputAmountDollar*100)) && (m_pDevCmn->fbCDU_IsDispensible(nInputAmountDollar) ) )
		{
			m_pDevCmn->m_nTranDispLimit = Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT));
			m_pDevCmn->m_MoneyMaxOut = m_pDevCmn->fnCDU_CalcMoneyMaxOut();
			if( m_pDevCmn->m_MoneyMaxOut >= nInputAmountDollar )
			{
				m_sUserSelection.strMoney.Format(L"%010d00", nInputAmountDollar);
				m_sCardData.strAccountNo	= m_PayPalCCA.m_strCW_CardNum.Mid(3, 12);		// Card PAN for EPP
				m_sCardData.strBankID		= m_PayPalCCA.m_strCW_CardNum.Right(4);			// Bank ID
				m_sCardData.strISO2Data		= m_PayPalCCA.m_strCW_CardNum;					// Full Track2 Data....
				m_sCardData.strISO2Data		+= L"=";	
				m_sCardData.strISO2Data		+= m_PayPalCCA.m_strCW_CardExpDate.Right(2);	// YY
				m_sCardData.strISO2Data		+= m_PayPalCCA.m_strCW_CardExpDate.Left(2);		// MM
				m_sCardData.strShowNumber.Format(L"************%s", m_sCardData.strBankID);

				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKCODE,	m_sCardData.strBankID);
				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKACCOUNT,	m_sCardData.strAccountNo);	
				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA,	m_sCardData.strShowNumber);
				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2,		m_sCardData.strISO2Data);
				return RES_OK;
			}
			else
				strUserNotice = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113108);		// We are sorry. Over max amount. Transaction has been cancelled.
		}
		else
			strUserNotice = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113110);			// We are sorry.Indispensable amount. Transaction has been cancelled.

		LIB_UserPopUpNotice(SCR_ICON_STOP, strUserNotice, ABORT_SCR_TIMEOUT);				// User Notice
		m_PayPalCCA.UpdateExternalStatusCode(L"IA", L"--", L"--");							// IA : Invalid Amount
		// End of [#2466]

		// Not Dispensible... Cash Shortage or Denomination Errors
		NHDEBUG(DBG_CALL, (L"Paypal Received Amt[%s] is not Dispensible\n", m_PayPalCCA.m_strReqAmount ));
		m_PayPalCCA.m_nTransactionStep = PAYPAL_TRAN_CANCEL_WITHTICKET;
		nRes = P_NH_NOR_Transaction(FALSE);
		return RES_NG;
	}

	// FAIL TO GET CASH TICKET => CANCEL TRANSACTION
	NHDEBUG(DBG_CALL, (L"Fail to get Cash Ticket\n"));
	m_PayPalCCA.m_nTransactionStep = PAYPAL_TRAN_CANCEL_CUSINFO;
	nRes = P_NH_NOR_Transaction(FALSE);
	return RES_NG;
}

BIZ_RETURN	CTranCmn::P_NH_PaypalCCA_UpdateSurcharge()
{
	NVDump('O', 'C', "00", L"P_CCA_UDT_SUR", L"Paypal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_PaypalCCA_UpdateSurcharge]\n"));

	// DISPLAY SCREEN
	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	//m_pDevCmn->fnSCR_DisplayImage(1, FALSE);														// Hide Exit Button
	m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161002));
	m_pDevCmn->fnSCR_DisplayString(2, L"" );														// Hide Cancel F8
	m_pDevCmn->fnSCR_DisplayString(3, L"" );														// NO Background Image
	m_pDevCmn->fnSCR_DisplayScreen(161);

	m_PayPalCCA.m_nTransactionStep = PAYPAL_TRAN_UPDATESURCHARGE;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if(nRes == RES_OK)	
	{
		NHDEBUG(DBG_CALL, (L"Update Surcharge OK \n"));
		return RES_OK;
	}

	// FAIL TO UPDATE SURCHARGE => CANCEL TRANSACTION
	NHDEBUG(DBG_CALL, (L"Fail to Update Surcharge\n"));
	if( m_PayPalCCA.m_strErrorCode != L"DP92004" )							// Skip Cancelling Transaction if a Customer refused surcharge
	{
		NHDEBUG(DBG_CALL, (L"Cancel Transaction \n"));
		m_PayPalCCA.UpdateExternalStatusCode(L"TC", L"--", L"--");			// [#2466] US Justin 2017.01.27, TC:Transaction Cancelled
		m_PayPalCCA.m_nTransactionStep = PAYPAL_TRAN_CANCEL_WITHTICKET;
		nRes = P_NH_NOR_Transaction(FALSE);
	}
	return RES_NG;
}

BIZ_RETURN	CTranCmn::P_NH_PaypalCCA_Processor_Auth()
{
	NVDump('O', 'C', "00", L"P_CCA_ATM_PROC", L"Paypal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_PaypalCCA_Processor_Auth]\n"));

	// DISPLAY SCREEN
	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	//m_pDevCmn->fnSCR_DisplayImage(1, FALSE);														// Hide Exit Button		// [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
	m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161001));
	m_pDevCmn->fnSCR_DisplayString(2, L"" );														// Hide Cancel F8
	m_pDevCmn->fnSCR_DisplayString(3, L"" );														// NO Background Image
	m_pDevCmn->fnSCR_DisplayScreen(161);

	m_PayPalCCA.m_nTransactionStep = PAYPAL_TRAN_HOSTAPPROVAL;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if(nRes == RES_OK)	NHDEBUG(DBG_CALL, (L"Processor Authorization OK \n"));
	else				NHDEBUG(DBG_CALL, (L"Fail to Authorize ATM Processor\n"));
	return nRes;
}

BIZ_RETURN	CTranCmn::P_NH_PaypalCCA_UpdateTransaction(int nUpdateType, int nDispResult, BIZ_RETURN nHostResult)
{
	NVDump('O', 'C', "00", L"P_CCA_UPT__TRN", L"Paypal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_PaypalCCA_UpdateTransactio(Type:%d, nAmount=%d)n]\n", nUpdateType, nDispResult));

	// DISPLAY SCREEN
	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	//m_pDevCmn->fnSCR_DisplayImage(1, FALSE);														// Hide Exit Button		// [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
	m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161001));
	m_pDevCmn->fnSCR_DisplayString(2, L"" );														// Hide Cancel F8
	m_pDevCmn->fnSCR_DisplayString(3, L"" );														// NO Background Image
	m_pDevCmn->fnSCR_DisplayScreen(161);

	// Update Dispense Amount
	m_PayPalCCA.m_strTranDispensed.Format(L"%0.2f", 0.01 * nDispResult);

	// Update Day Total
	if( nDispResult > 0 )	
		m_PayPalCCA.RecodeDispenseResult(nDispResult);

	// Update Error Code
	CString strErrorCode = L"";
	if( Dollar2Cent(m_PayPalCCA.m_strTranDispensed) >= Dollar2Cent(m_PayPalCCA.m_strReqAmount) )
		m_PayPalCCA.UpdateExternalStatusCode(L"00", L"00", L"Authorized Transation" );
	else
	{
		if( nUpdateType==PAYPAL_UPDATETRAN_DISPENSED )		strErrorCode = L"DE";												// Dispense Error
		else if(nHostResult==RES_HOST_DENIED)				strErrorCode.Format(L"%2.2s", m_sSTD1_TranResp.ResponseCode_2);		// Host Denial
		else												strErrorCode = L"CE";												// Communication Error
		m_PayPalCCA.UpdateExternalStatusCode(strErrorCode, m_pDevCmn->fstrAPL_GetErrorCode(), m_pDevCmn->fstrAPL_GetErrorMessage() );
	}
	m_PayPalCCA.m_nTransactionStep = PAYPAL_TRAN_UPDATETRAN;

	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);
	if(nRes == RES_OK)	NHDEBUG(DBG_CALL, (L"Update Transaction result OK \n"));
	else				NHDEBUG(DBG_CALL, (L"Fail to Update Transaction\n"));
	return nRes;
}
#endif
// End of [#2446]

// [#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
#if (APP_GPAY_GIFTCARD_PURCHASE) 

// [#2574] US Justin GivePay Enhancement3
BIZ_RETURN CTranCmn::P_NH_GivePay_DownloadData(int nDownloadType)
{
	CString strTemp;
	strTemp.Format(L"DownData[%d]", nDownloadType);

	NVDump('O', 'C', "00", L"GivePay", strTemp);
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_DownloadData(%d)]\n", nDownloadType));

	int nTempTranStatus = m_pDevCmn->TranStatus;
	int nTempTranCode = TranCode;
	int nSavedLangMode = m_pDevCmn->fnSCR_GetCurrentLangMode();

	if(nDownloadType != GPAY_DOWNLOAD_TRANINFO)				// Beginning of GivePay Transaction - Parameters are set 
	{
		m_pDevCmn->TranStatus = TRAN_TRAN;
		TranCode = TC_GP_BUY_CARD;
		m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);
		m_GivePayData.ResetGPTransactionData();
	}

	// Please Wait Screen
	if(nDownloadType != GPAY_DOWNLOAD_OP_ENROLL)
		P_NH_GivePay_DisplayWait();
	
	BIZ_RETURN nRes = RES_NG;
	while(1)
	{
		if( (nDownloadType == GPAY_DOWNLOAD_OP_ENROLL) || (nDownloadType == GPAY_DOWNLOAD_CMN_IMAGE) || (nDownloadType == GPAY_DOWNLOAD_ALL) || (m_GivePayData.m_bTempDG911) ) // [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix
		{
			if( (nRes = P_NH_GivePay_DownloadTranInfo(GPAY_TRAN_ENROLLMENT)) != RES_OK )
				break;
			
			if(nDownloadType == GPAY_DOWNLOAD_OP_ENROLL)
				break;
		}

		if ((nRes = P_NH_GivePay_DownloadTranInfo(GPAY_TRAN_GETTOKEN)) != RES_OK)	
			break;

		// Get Parameters - Fees, Gift Card Category, greetings, wireless carriers
		BOOL bDownloadData = TRUE;
		if( (nDownloadType == GPAY_DOWNLOAD_TRANINFO) && (m_GivePayData.IsGPParameterOld() == FALSE) )
			bDownloadData = FALSE;

		NHDEBUG(DBG_CALL, (L"GivePay Parameter. NEED TO DOWNLOAD = [%d]\n", bDownloadData));
		if(bDownloadData)	
		{
			if ((nRes = P_NH_GivePay_DownloadTranInfo(GPAY_TRAN_DOWNLOAD_FEES)) != RES_OK)
				break;
			if ((nRes = P_NH_GivePay_DownloadTranInfo(GPAY_TRAN_DOWNLOAD_CATEGORY)) != RES_OK)
				break;
			if ((nRes = P_NH_GivePay_DownloadTranInfo(GPAY_TRAN_DOWNLOAD_GREETINGS)) != RES_OK)
				break;
			if ((nRes = P_NH_GivePay_DownloadTranInfo(GPAY_TRAN_DOWNLOAD_WIRELESS)) != RES_OK)
				break;

			m_GivePayData.UpdateGPParameterExpire(86400);		// Renew daily (24*60*60) Sec
		}

		// Download Images
		if( (nDownloadType == GPAY_DOWNLOAD_CMN_IMAGE) || (nDownloadType == GPAY_DOWNLOAD_ALL) )
		{
			P_NH_GivePay_DownloadImages(FALSE, GPAY_ID_CATEGORY);
			P_NH_GivePay_DownloadSubCategoryAndCardImges(FALSE, nDownloadType);
			P_NH_GivePay_DownloadImages(FALSE, GPAY_ID_GREETINGS);
			P_NH_GivePay_DownloadImages(FALSE, GPAY_ID_WIRELESS);
		}
		break;
	}

	if( nDownloadType != GPAY_DOWNLOAD_TRANINFO )
	{
		m_pDevCmn->TranStatus = nTempTranStatus;
		TranCode = nTempTranCode;
		m_pDevCmn->fnSCR_SetCurrentLangMode(nSavedLangMode);
		m_GivePayData.ResetGPTransactionData();
	}

	if( (nDownloadType == GPAY_DOWNLOAD_CMN_IMAGE) || (nDownloadType == GPAY_DOWNLOAD_ALL) )
		m_GivePayData.UpdateGPNextDailyUpdateTime();

	if(nRes == RES_OK)
		NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_DownloadData() RETURN RES_OK]\n"));
	else
	{
		if( (nDownloadType == GPAY_DOWNLOAD_TRANINFO) && (m_GivePayData.m_strErrorDesc.GetLength() > 0) && (!m_GivePayData.m_bTempDG911) ) // [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_GivePayData.m_strErrorDesc, INFO_SCR_TIMEOUT);	

		NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_DownloadData() RETURN NOT ERROR]\n"));
	}
	return nRes;
}

void CTranCmn::P_NH_GivePay_DisplayWait()
{
	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	m_pDevCmn->fnSCR_DisplayString(1, L"" );														// Hide Please Wait
	m_pDevCmn->fnSCR_DisplayString(2, L"" );														// Hide Cancel F8
	m_pDevCmn->fnSCR_DisplayString(3, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_WAIT) );			// Background Image
	m_pDevCmn->fnSCR_DisplayScreen(161);
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_DownloadTranInfo(int nDataType)
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_DlTraninfo");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_DownloadTranInfo(%d)]\n", nDataType ));

	if(nDataType == GPAY_TRAN_GETTOKEN)		// Use Existing Token if it is not expiredn
	{
		if(	!m_GivePayData.m_strGPToken.IsEmpty() && !m_GivePayData.m_strGPTokenType.IsEmpty() && !m_GivePayData.m_strGPTokenExpire.IsEmpty() )
		{
			CString sTE = m_GivePayData.m_strGPTokenExpire;
			if(sTE.GetLength() == 14)
			{
				CTime CurTime = CTime::GetCurrentTime();
				CTime ExpTime = CTime( Asc2Int(sTE.Left(4)), Asc2Int(sTE.Mid(4,2)), Asc2Int(sTE.Mid(6,2)), Asc2Int(sTE.Mid(8,2)), Asc2Int(sTE.Mid(10,2)), Asc2Int(sTE.Mid(12,2)) );
				if( ExpTime > CurTime)
				{
					NHDEBUG(DBG_CALL, (L"Token is Valid. Expire at [%s]\n", sTE));
					return RES_OK;
				}
			}
		}
		m_GivePayData.m_strGPToken = m_GivePayData.m_strGPTokenType = m_GivePayData.m_strGPTokenExpire = _T("");
	}

	m_GivePayData.m_nTransactionStep = nDataType;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if(nRes == RES_OK)	NHDEBUG(DBG_CALL, (L"GP_DlTraninfo [%d] OK\n", nDataType ));
	else				NHDEBUG(DBG_CALL, (L"GP_DlTraninfo [%d] FAILURE\n", nDataType ));
	return nRes;
}
// End of [#2574]

BIZ_RETURN	CTranCmn::P_NH_GivePay_DownloadImages(BOOL bShowScreen, int nImageType)
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_DN_IMages");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_DownloadImages(%d)]\n", nImageType));

	int i;
	BIZ_RETURN nRes;
	CStringArray arrImageList;
	arrImageList.RemoveAll();

	CStringArray* parrStr = NULL;
	if(     nImageType==GPAY_ID_CATEGORY)		parrStr = &m_GivePayData.m_arrCategory;
	else if(nImageType==GPAY_ID_SUBCATEGORY)	parrStr = &m_GivePayData.m_arrSubCategory;
	else if(nImageType==GPAY_ID_CARDS)			parrStr = &m_GivePayData.m_arrCards;
	else if(nImageType==GPAY_ID_GREETINGS)		parrStr = &m_GivePayData.m_arrGreetings;
	else if(nImageType==GPAY_ID_WIRELESS)		parrStr = &m_GivePayData.m_arrWireless;
	
	CString strOneItem, strTemp;
	for(i=0; i<parrStr->GetSize(); i++)
	{
		strOneItem = parrStr->GetAt(i);

		// Pack SUB ITEMS
		if(nImageType==GPAY_ID_CATEGORY)
		{
			strTemp = GetJSONNodeValue(strOneItem, L"children" );
			if(strTemp.GetLength() > 10)
				strOneItem.Replace(strTemp, L"_CATEGORY_CHILDREN_");
		}

		strTemp = GetJSONNodeValue(strOneItem, L"imageUri" );
		if( strTemp.GetLength() > 10 ) // Minimum Length....
		{
			arrImageList.Add(strTemp);		
			NHDEBUG(DBG_CALL, (L"  ADD Dowload List : imageUri [%d] - [%s]\n", arrImageList.GetSize(), strTemp));
		}
	}

	BOOL bDisplayed = FALSE;
	if(arrImageList.GetSize()>0)
	{
		m_GivePayData.m_nTransactionStep = GPAY_TRAN_DOWNLOAD_IMAGES;

		for(i=0; i<arrImageList.GetSize(); i++)
		{
			m_GivePayData.m_sImageURL = arrImageList.GetAt(i);
			if( m_GivePayData.IsImageFileExist(m_GivePayData.m_sImageURL) != TRUE)
			{
				// DISPLAY SCREEN
				if( (bShowScreen) && (bDisplayed==FALSE) )
				{
					P_NH_GivePay_DisplayWait();
					bDisplayed = TRUE;
				}

				nRes = P_NH_NOR_Transaction(FALSE);
			
				if( nRes == RES_OK )		NHDEBUG(DBG_CALL, (L"Get Image [%s] OK\n", m_GivePayData.m_sImageURL ));
				else						NHDEBUG(DBG_CALL, (L"Get Image [%s] FAILURE\n", m_GivePayData.m_sImageURL ));
			}
			else
				NHDEBUG(DBG_CALL, (L"  Image [%s] is stored in Local. SKIP DOWNLOADING\n", m_GivePayData.m_sImageURL ));
		}
	}
	return RES_OK;
}

// [#2574] US Justin GivePay Enhancement3 - Downloading
BIZ_RETURN	CTranCmn::P_NH_GivePay_DownloadSubCategoryAndCardImges(BOOL bShowScreen, int nDownloadType)
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_DN_SubCard");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_DownloadSubCategoryAndCardImges(%d)]\n", nDownloadType));

	// DISPLAY SCREEN
	if(bShowScreen)
		P_NH_GivePay_DisplayWait();

	// [#J002] US JUSTIN
	if(nDownloadType == GPAY_DOWNLOAD_ALL)
		m_GivePayData.m_arrFullCardList.RemoveAll();
	// End of [#J002]

	CStringArray* parrCategory = &m_GivePayData.m_arrCategory;
	for(int i=0; i<parrCategory->GetSize(); i++)
	{
		CString strOneItem = parrCategory->GetAt(i);

		// Check Sub Category
		CString strChildren = GetJSONNodeValue(strOneItem, L"children" );
		if(strChildren.GetLength() > 10)		// Has Sub Category
		{
			NHDEBUG(DBG_CALL, (L"Category[%d/%d] has Sub Category\n", i+1, parrCategory->GetSize() ));
			m_GivePayData.ParseAndFillGPSubCategory(i);
			P_NH_GivePay_DownloadImages(FALSE, GPAY_ID_SUBCATEGORY);

			if(nDownloadType == GPAY_DOWNLOAD_ALL)
			{
				CStringArray* parrSubCategory = &m_GivePayData.m_arrSubCategory;
				for(int j=0; j<parrSubCategory->GetSize(); j++)
				{
					CString sOneSubCat = parrSubCategory->GetAt(j);

					m_GivePayData.m_nGPCategoryId = Asc2Int(GetJSONNodeValue(sOneSubCat, L"id") );					
					NHDEBUG(DBG_CALL, (L"Downloading card of sub Category[%d/%d], Id = [%d]\n", j+1, parrSubCategory->GetSize(), m_GivePayData.m_nGPCategoryId));

					if( P_NH_GivePay_GetCards(FALSE) == RES_OK)
					{
						m_GivePayData.CollectCardInformation(m_GivePayData.m_nGPCategoryId);
						P_NH_GivePay_DownloadImages(FALSE, GPAY_ID_CARDS);
					}
				}
			}
		}
		else
		{
			NHDEBUG(DBG_CALL, (L"Category[%d/%d] does not have Sub Category\n", i+1, parrCategory->GetSize()));
			if(nDownloadType == GPAY_DOWNLOAD_ALL)
			{
				m_GivePayData.m_nGPCategoryId = Asc2Int(GetJSONNodeValue(strOneItem, L"id") );
				NHDEBUG(DBG_CALL, (L"Downloading card of Category[%d/%d], Id = [%d]\n", i+1, parrCategory->GetSize(), m_GivePayData.m_nGPCategoryId));
				if( P_NH_GivePay_GetCards(FALSE) == RES_OK)
				{
					m_GivePayData.CollectCardInformation(m_GivePayData.m_nGPCategoryId);
					P_NH_GivePay_DownloadImages(FALSE, GPAY_ID_CARDS);
				}
			}
		}
	}
	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_GivePay_NeedMoreTime()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_NeedMoreTime()]\n"));

	BIZ_RETURN nRes = RES_USER_TIMEOUT;
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
			
			m_pDevCmn->fnSCR_DisplayPrevSet(165);

			// Setting Screen
			{
				// SCREEN OVERLAY
				m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_NEEDTIME));

				// Clear Image File Name
				for(int i=0; i<(MENU_SELECT_SIZE-2); i++)
					m_pDevCmn->fnSCR_DisplayString(i + 21, L"" );

				// F7 : No
				m_pDevCmn->fnSCR_DisplayString(17, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165908) );

				// F8 : Yes
				m_pDevCmn->fnSCR_DisplayString(18, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165909) );

			}
			m_pDevCmn->fnSCR_DisplayScreen(165, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT )
				return RES_USER_EXIT;
			else if (GetKeyStr == S_TIMEOVER)
				return RES_USER_TIMEOUT;
			else if (GetKeyStr==L"F7")
				return RES_USER_EXIT;
			else if (GetKeyStr==L"F8")
				return RES_OK;
		}
		Delay_Msg(50);
	}
	return nRes;
}
// End of [#2547]

BIZ_RETURN	CTranCmn::P_NH_GivePay_SelectServices(BOOL bIsPiggyBack)
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_Service");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_SelectServices()]\n"));

	// Initialize Parameters
	m_GivePayData.m_nGPServiceType = GPAY_SERVICE_NONE;

	BIZ_RETURN nRes = RES_USER_TIMEOUT;
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
			
			m_pDevCmn->fnSCR_DisplayPrevSet(165);

			// Setting Screen
			{
				// SCREEN OVERLAY
				if(bIsPiggyBack)	m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_UPSELL));
				else				m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_MAIN));

				// Clear Image File Name
				for(int i=0; i<(MENU_SELECT_SIZE-2); i++)
					m_pDevCmn->fnSCR_DisplayString(i + 21, L"" );

				// [#2574] US Justin GivePay Enhancement3
				if(bIsPiggyBack)
				{
					m_pDevCmn->fnSCR_DisplayString(19, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165001) );		// Give a Gift (F6, Orange Button)
					m_pDevCmn->fnSCR_DisplayString(20, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165901) );		// SKIP (F7, Gray Button)
				}
				else
				{
					m_pDevCmn->fnSCR_DisplayString(15, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165902) );		// MAIN MENU (F5, Gray Button)
					m_pDevCmn->fnSCR_DisplayString(17, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165001) );		// Give a Gift (F7, Orange Button)
				}
				// End of [#2547]

				// F8 : Pay for wireless
				m_pDevCmn->fnSCR_DisplayString(18, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165002) );

			}
			m_pDevCmn->fnSCR_DisplayScreen(165, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT )
				return RES_USER_EXIT;
			else if (GetKeyStr == S_TIMEOVER)
			{
				// [#2574] US Justin GivePay Enhancement3
				//return RES_USER_TIMEOUT;
				if(P_NH_GivePay_NeedMoreTime() == RES_OK)
					bShowScreen = TRUE;
				else
					return RES_USER_TIMEOUT;
				// End of [#2574]
			}
			// [#2574] US Justin GivePay Enhancement3
			else if( (GetKeyStr==L"F5") || ((GetKeyStr==L"F7")&&(bIsPiggyBack==TRUE)) ) 
				return RES_USER_PREV;
			else if( (GetKeyStr==L"F6") || ((GetKeyStr==L"F7")&&(bIsPiggyBack!=TRUE) ) )
			{
				m_GivePayData.m_nGPServiceType = GPAY_SERVICE_GIFTCARD;
				return RES_OK;
			}
			// End of [#2574]
			else if (GetKeyStr==L"F8")
			{
				m_GivePayData.m_nGPServiceType = GPAY_SERVICE_WIRELESS;
				return RES_OK;
			}
		}
		Delay_Msg(50);
	}
	return nRes;
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_SelectCategory()
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_Cat.");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_SelectCategory()]\n"));

	P_NH_GivePay_DownloadImages(TRUE, GPAY_ID_CATEGORY);

	// Initialize Parameters
	m_GivePayData.m_nGPCategoryId = 0;
	int nPageNum = 0;	
	int nNextPage, nIdxOfFdk;
	BIZ_RETURN nRes = RES_USER_TIMEOUT;
	CString	strValueOfFdk[MENU_SELECT_SIZE] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };
	CString	strImageFile[MENU_SELECT_SIZE] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
			
			m_pDevCmn->fnSCR_DisplayPrevSet(165);

			// Setting Screen
			{
				// SCREEN OVERLAY
				m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_GC_CATEGORY));

				// Initialize FDK and Image File
				for(nIdxOfFdk=0; nIdxOfFdk<MENU_SELECT_SIZE; nIdxOfFdk++)
				{
					strValueOfFdk[nIdxOfFdk] = L"";
					strImageFile[nIdxOfFdk] = L"";
				}

				// Fill Button Text (UP TO 6 BUTTONs) : Sub(1, 1:exist, 0:no) + ArrayID(2) + ItemID(6)  + Name
				if(m_GivePayData.FillGPButtonItems( GPAY_ID_CATEGORY, nPageNum, strValueOfFdk, strImageFile, &nNextPage) < 0)
				{
					NHDEBUG(DBG_CALL, (L"Fill Category FAILURE\n"));
					return RES_USER_EXIT;
				}

				// Fee Notice (Extra line below Sub Title: APValue 6)
				// [#J006] US Justin Add Fee notice 
				CString strFeeNotice = L""; 
				if(Dollar2Cent(m_GivePayData.m_strCardFee) > 0)
				{
					strFeeNotice = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165006);
					CString sFeeValue; 
					sFeeValue.Format(L"%s %s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(Dollar2Cent(m_GivePayData.m_strCardFee) ) ) );
					strFeeNotice.Replace(L"XXXXX", sFeeValue);
					m_pDevCmn->fnSCR_DisplayString(6, strFeeNotice );
				}
				// [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6
				//else
					// strFeeNotice = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165007); 
					//m_pDevCmn->fnSCR_DisplayString(6, strFeeNotice );
				// end of [#RWC6-1, #2583]
				// End [#J006]

				// Assign Image File Name (APValue 21 - 26)
				for(nIdxOfFdk=0; nIdxOfFdk<(MENU_SELECT_SIZE-2); nIdxOfFdk++)
				{
					NHDEBUG(DBG_INFO, (L"Assign Image [F%d] = [%s]\n", (nIdxOfFdk+1), strImageFile[nIdxOfFdk] ));
					m_pDevCmn->fnSCR_DisplayString(nIdxOfFdk + 21, strImageFile[nIdxOfFdk] );
				}

				// F7
				strValueOfFdk[6].Format( L"000999999%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165903) );		// Back
				
				// F8
				if( nNextPage>0 )
					strValueOfFdk[7].Format( L"000999998%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165904) );	// More Choices

				// Fill Button (APValue 11 - 18)
				for(nIdxOfFdk=0; nIdxOfFdk<MENU_SELECT_SIZE; nIdxOfFdk++)
				{
					if(strValueOfFdk[nIdxOfFdk].GetLength()>9)
						m_pDevCmn->fnSCR_DisplayString(nIdxOfFdk + 11, strValueOfFdk[nIdxOfFdk].Mid(9) );
				}
			}
			m_pDevCmn->fnSCR_DisplayScreen(165, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT )
				return RES_USER_EXIT;
			else if (GetKeyStr == S_TIMEOVER)
			{
				// [#2574] US Justin GivePay Enhancement3
				//return RES_USER_TIMEOUT;
				if(P_NH_GivePay_NeedMoreTime() == RES_OK)
					bShowScreen = TRUE;
				else
					return RES_USER_TIMEOUT;
				// End of [#2574]
			}
			else if( (GetKeyStr==L"F1") || (GetKeyStr==L"F2") || (GetKeyStr==L"F3") || (GetKeyStr==L"F4") || 
					 (GetKeyStr==L"F5") || (GetKeyStr==L"F6") || (GetKeyStr==L"F7") || (GetKeyStr==L"F8") )
			{
				nIdxOfFdk = Asc2Int(GetKeyStr.Right(1) ) - 1;
				CString strSelected = strValueOfFdk[nIdxOfFdk];

				if(strSelected.GetLength() > 9)
				{
					CString strSelCardID = strSelected.Mid(3,6);
					if(strSelCardID == L"999999")					// Back
					{
						if(nPageNum>0 )
						{
							nPageNum--;
							bShowScreen = TRUE;
						}
						else
							return RES_USER_PREV;
					}
					else if(strSelCardID == L"999998")				// More Choices
					{
						nPageNum++;
						bShowScreen = TRUE;
					}
					else
					{
						if( Asc2Int(strSelected.Left(1)) == 0 )		// No Sub Catetory
						{
							m_GivePayData.m_nGPCategoryId = Asc2Int(strSelCardID );
							NHDEBUG(DBG_INFO, (_T("No Sub Category, Selected Giftcard Category ID = [%d]\n"), m_GivePayData.m_nGPCategoryId ));
							return RES_GIVEPAY_FINALSELECT;
						}
						else
						{
							if( m_GivePayData.ParseAndFillGPSubCategory(Asc2Int(strSelected.Mid(1,2))) )
							{
								NHDEBUG(DBG_INFO, (_T("Has Sub Category. Category ID = [%d]\n"), Asc2Int(strSelected.Mid(1,2)) ));
								return RES_OK;
							}
							else
								return RES_USER_EXIT;
						}
					}
				}
				else
					return RES_USER_EXIT;
			}
		}
		Delay_Msg(50);
	}
	return nRes;
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_SelectSubCategory()
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_SubCat");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_SelectSubCategory()]\n"));

	P_NH_GivePay_DownloadImages(TRUE, GPAY_ID_SUBCATEGORY);

	// Initialize Parameters
	m_GivePayData.m_nGPCategoryId = 0;
	int nPageNum = 0;
	int nNextPage, nIdxOfFdk;
	BIZ_RETURN nRes = RES_USER_TIMEOUT;
	CString	strValueOfFdk[MENU_SELECT_SIZE] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };
	CString	strImageFile[MENU_SELECT_SIZE] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
			
			m_pDevCmn->fnSCR_DisplayPrevSet(165);

			// Setting Screen
			{
				// SCREEN OVERLAY
				m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_GC_SUB_CATEGORY));

				// Initialize FDK and Image File
				for(nIdxOfFdk=0; nIdxOfFdk<MENU_SELECT_SIZE; nIdxOfFdk++)
				{
					strValueOfFdk[nIdxOfFdk] = L"";
					strImageFile[nIdxOfFdk] = L"";
				}

				// Fill Button Text (UP TO 6 BUTTONs) : Sub(1, 1:exist, 0:no) + ArrayID(2) + ItemID(6)  + Name
				if(m_GivePayData.FillGPButtonItems( GPAY_ID_SUBCATEGORY, nPageNum, strValueOfFdk, strImageFile, &nNextPage) < 0)
				{
					NHDEBUG(DBG_CALL, (L"Fill sub category FAILURE\n"));
					return RES_USER_EXIT;
				}

				// Fee Notice (Extra line below Sub Title: APValue 6)
				// [#J006] US Justin Add Fee notice 
				CString strFeeNotice = L""; 
				if(Dollar2Cent(m_GivePayData.m_strCardFee) > 0)
				{
					strFeeNotice = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165006);
					CString sFeeValue; 
					sFeeValue.Format(L"%s %s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(Dollar2Cent(m_GivePayData.m_strCardFee) ) ) );
					strFeeNotice.Replace(L"XXXXX", sFeeValue);
					m_pDevCmn->fnSCR_DisplayString(6, strFeeNotice );
				}
				// [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6
				//else
					// strFeeNotice = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165007); 
					//m_pDevCmn->fnSCR_DisplayString(6, strFeeNotice );
				// end of [#RWC6-1, #2583]
				// End [#J006]

				// Assign Image File Name (APValue 21 - 26)
				for(nIdxOfFdk=0; nIdxOfFdk<(MENU_SELECT_SIZE-2); nIdxOfFdk++)
				{
					NHDEBUG(DBG_INFO, (L"Assign Image [F%d] = [%s]\n", (nIdxOfFdk+1), strImageFile[nIdxOfFdk] ));
					m_pDevCmn->fnSCR_DisplayString(nIdxOfFdk + 21, strImageFile[nIdxOfFdk] );
				}

				// F7
				strValueOfFdk[6].Format( L"000999999%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165903) );		// Back
				
				// F8
				if( nNextPage>0 )
					strValueOfFdk[7].Format( L"000999998%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165904) );	// More Choices

				// Fill Button (APValue 11 - 18)
				for(nIdxOfFdk=0; nIdxOfFdk<MENU_SELECT_SIZE; nIdxOfFdk++)
				{
					if(strValueOfFdk[nIdxOfFdk].GetLength()>9)
						m_pDevCmn->fnSCR_DisplayString(nIdxOfFdk + 11, strValueOfFdk[nIdxOfFdk].Mid(9) );
				}
			}

			m_pDevCmn->fnSCR_DisplayScreen(165, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT )
				return RES_USER_EXIT;
			else if (GetKeyStr == S_TIMEOVER)
			{
				// [#2574] US Justin GivePay Enhancement3
				//return RES_USER_TIMEOUT;
				if(P_NH_GivePay_NeedMoreTime() == RES_OK)
					bShowScreen = TRUE;
				else
					return RES_USER_TIMEOUT;
				// End of [#2574]
			}
			else if( (GetKeyStr==L"F1") || (GetKeyStr==L"F2") || (GetKeyStr==L"F3") || (GetKeyStr==L"F4") || 
					 (GetKeyStr==L"F5") || (GetKeyStr==L"F6") || (GetKeyStr==L"F7") || (GetKeyStr==L"F8") )
			{
				nIdxOfFdk = Asc2Int(GetKeyStr.Right(1) ) - 1;
				CString strSelected = strValueOfFdk[nIdxOfFdk];

				if(strSelected.GetLength() > 9)
				{
					CString strSelCardID = strSelected.Mid(3,6);
					if(strSelCardID == L"999999")					// Back
					{
						if(nPageNum>0 )
						{
							nPageNum--;
							bShowScreen = TRUE;
						}
						else
							return RES_USER_PREV;
					}
					else if(strSelCardID == L"999998")				// More Choices
					{
						nPageNum++;
						bShowScreen = TRUE;
					}
					else
					{
						m_GivePayData.m_nGPCategoryId = Asc2Int(strSelCardID );
						NHDEBUG(DBG_INFO, (_T("Selected Giftcard Category ID = [%d]\n"), m_GivePayData.m_nGPCategoryId ));
						return RES_OK;
					}
				}
				else
					return RES_USER_EXIT;
			}
		}
		Delay_Msg(50);
	}
	return nRes;
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_GetCards(BOOL bShowScreen)
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_Card");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_GetCards()]\n"));

	// [#J002] US Justin
	int nFilledCard = m_GivePayData.GetSelectedCardInformation(m_GivePayData.m_nGPCategoryId);
	if( nFilledCard > 0)
	{
		NHDEBUG(DBG_CALL, (L"[%d] Card Images are filled... Return without downloading Card Information\n", nFilledCard));
		return RES_OK;
	}
	// End of [#J002]

	// DISPLAY SCREEN
	if(bShowScreen)
		P_NH_GivePay_DisplayWait();

	m_GivePayData.m_arrCards.RemoveAll();
	m_GivePayData.m_nTransactionStep = GPAY_TRAN_DOWNLOAD_CARDS;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if( nRes == RES_OK )	
		NHDEBUG(DBG_CALL, (L"Get Cards OK\n"));
	else
	{
		NHDEBUG(DBG_CALL, (L"Get Cards FAILURE\n"));
		// Display Error Description
		if(bShowScreen)
		{
			if(m_GivePayData.m_strErrorDesc.GetLength() > 0)
				LIB_UserPopUpNotice(SCR_ICON_STOP, m_GivePayData.m_strErrorDesc, INFO_SCR_TIMEOUT);	
		}
	}
	return nRes;
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_SelectCard()
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_Card");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_SelectCard()]\n"));

	P_NH_GivePay_DownloadImages(TRUE, GPAY_ID_CARDS);

	// Initialize Parameters
	m_GivePayData.m_strSelectedCard = _T("");
	int nPageNum = 0;	
	int nNextPage, nIdxOfFdk;
	BIZ_RETURN nRes = RES_USER_TIMEOUT;
	CString	strValueOfFdk[MENU_SELECT_SIZE] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };
	CString	strImageFile[MENU_SELECT_SIZE] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
			
			m_pDevCmn->fnSCR_DisplayPrevSet(165);

			// Setting Screen
			{
				// SCREEN OVERLAY
				m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_GC_CARD_SELECTION));

				// Initialize FDK and Image File
				for(nIdxOfFdk=0; nIdxOfFdk<MENU_SELECT_SIZE; nIdxOfFdk++)
				{
					strValueOfFdk[nIdxOfFdk] = L"";
					strImageFile[nIdxOfFdk] = L"";
				}

				// Fill Button Text (UP TO 6 BUTTONs) : Sub(1, 1:exist, 0:no) + ArrayID(2) + ItemID(6)  + Name
				if(m_GivePayData.FillGPButtonItems( GPAY_ID_CARDS, nPageNum, strValueOfFdk, strImageFile, &nNextPage) < 0)
				{
					NHDEBUG(DBG_CALL, (L"Fill Card FAILURE\n"));
					return RES_USER_EXIT;
				}			

				// Fee Notice (Extra line below Sub Title: APValue 6)
				// [#2574] US Justin Add Fee notice 
				CString strFeeNotice = L""; 
				if(Dollar2Cent(m_GivePayData.m_strCardFee) > 0)
				{
					strFeeNotice = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165006);
					CString sFeeValue; 
					sFeeValue.Format(L"%s %s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(Dollar2Cent(m_GivePayData.m_strCardFee) ) ) );
					strFeeNotice.Replace(L"XXXXX", sFeeValue);
					m_pDevCmn->fnSCR_DisplayString(6, strFeeNotice );
				}
				// [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6
				// else
				//	strFeeNotice = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165007);
				// end of [#RWC6-1, #2583]
				// End [#2574]

				// Assign Image File Name (APValue 21 - 26)
				for(nIdxOfFdk=0; nIdxOfFdk<(MENU_SELECT_SIZE-2); nIdxOfFdk++)
				{
					NHDEBUG(DBG_INFO, (L"Assign Image [F%d] = [%s]\n", (nIdxOfFdk+1), strImageFile[nIdxOfFdk] ));
					m_pDevCmn->fnSCR_DisplayString(nIdxOfFdk + 21, strImageFile[nIdxOfFdk] );
				}

				// F7
				strValueOfFdk[6].Format( L"000999999%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165903) );		// Back
				
				// F8
				if( nNextPage>0 )
					strValueOfFdk[7].Format( L"000999998%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165904) );	// More Choices

				// Fill Button (APValue 11 - 18)
				for(nIdxOfFdk=0; nIdxOfFdk<MENU_SELECT_SIZE; nIdxOfFdk++)
				{
					if(strValueOfFdk[nIdxOfFdk].GetLength()>9)
						m_pDevCmn->fnSCR_DisplayString(nIdxOfFdk + 11, strValueOfFdk[nIdxOfFdk].Mid(9) );
				}
			}
			m_pDevCmn->fnSCR_DisplayScreen(165, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				return RES_USER_EXIT;
			else if (GetKeyStr == S_TIMEOVER)
			{
				// [#2574] US Justin GivePay Enhancement3
				//return RES_USER_TIMEOUT;
				if(P_NH_GivePay_NeedMoreTime() == RES_OK)
					bShowScreen = TRUE;
				else
					return RES_USER_TIMEOUT;
				// End of [#2574]
			}
			else if( (GetKeyStr==L"F1") || (GetKeyStr==L"F2") || (GetKeyStr==L"F3") || (GetKeyStr==L"F4") || 
					 (GetKeyStr==L"F5") || (GetKeyStr==L"F6") || (GetKeyStr==L"F7") || (GetKeyStr==L"F8") )
			{
				nIdxOfFdk = Asc2Int(GetKeyStr.Right(1) ) - 1;
				CString strSelected = strValueOfFdk[nIdxOfFdk];

				if(strSelected.GetLength() > 9)
				{
					CString strSelCardID = strSelected.Mid(3,6);
					if(strSelCardID == L"999999")					// Back
					{
						if(nPageNum>0 )
						{
							nPageNum--;
							bShowScreen = TRUE;
						}
						else
							return RES_USER_PREV;
					}
					else if(strSelCardID == L"999998")				// More Choices
					{
						nPageNum++;
						bShowScreen = TRUE;
					}
					else
					{
						m_GivePayData.m_strSelectedCard = m_GivePayData.m_arrCards.GetAt(Asc2Int(strSelected.Mid(1,2)));
						NHDEBUG(DBG_INFO, (_T("Selected Card ID = [%s]\n"), strSelected.Mid(3,6) ));
						return RES_OK;

					}
				}
				else
					return RES_USER_EXIT;
			}
		}
		Delay_Msg(50);
	}
	return nRes;
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_EnterAmount()
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_Amt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_EnterAmount]\n"));

	// Initialize Parameters
	int	nIdxOfFdk;
	CString strTemp;
	m_GivePayData.m_strGPTranAmount = _T("");
	BIZ_RETURN nRes = RES_USER_TIMEOUT;
	CString	strValueOfFdk[MENU_SELECT_SIZE] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };

	// [#2574] US Justin 2018.08.29 GivePay Enhancement3

	CStringArray arrAmountRange;
	int nAmtInputMethod = -1;			// 0:selection, 1:Entering

	BOOL bExistFixedAmts = m_GivePayData.FillGPCardFixedAmount(strValueOfFdk, 6);
	BOOL bExistRngedAmts = m_GivePayData.FillGPCardMinMaxAmount(&arrAmountRange);

	if(		 (bExistFixedAmts==TRUE)  && (bExistRngedAmts==FALSE) )
		nAmtInputMethod = 0;
	else if( (bExistFixedAmts==FALSE) && (bExistRngedAmts==TRUE) )
		nAmtInputMethod = 1;
	else if( (bExistFixedAmts==TRUE)  && (bExistRngedAmts==TRUE) )
	{
		m_GivePayData.CombineMultiRanges(&arrAmountRange, strValueOfFdk, 6);
		nAmtInputMethod = 1;
	}

	if( nAmtInputMethod == -1 )
	{
		NHDEBUG(DBG_CALL, (L"ERROR ON finding Available Amounts\n"));
		return RES_USER_EXIT;
	}
	else if( nAmtInputMethod == 0 )
	{
		CString		GetKeyStr, strTemp;
		BOOL		bShowScreen = TRUE;
		long nScrTimeOut = GetTransactionScreenTimeOut();
		if(nScrTimeOut==0)
			return RES_USER_EXIT;

		NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
		g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

		while (g_TimeCheck.IsElapsedTimes() == FALSE)
		{
			///////////////////////////////////
			// DISPLAY SCREEN
			if (bShowScreen == TRUE)
			{
				NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
				
				m_pDevCmn->fnSCR_DisplayPrevSet(165);

				// Setting Screen
				{
					// SCREEN OVERLAY
					if(m_GivePayData.m_nGPServiceType == GPAY_SERVICE_GIFTCARD) 
						m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_GC_AMOUNT));
					else
						m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_WL_AMOUNT));

					// Clear Image File Name
					for(nIdxOfFdk=0; nIdxOfFdk<(MENU_SELECT_SIZE-2); nIdxOfFdk++)
						m_pDevCmn->fnSCR_DisplayString(nIdxOfFdk + 21, L"" );

					// F7
					strValueOfFdk[6].Format( L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165903) );		// Back

					// Fill Button (APValue 11 - 18)
					for(nIdxOfFdk=0; nIdxOfFdk<MENU_SELECT_SIZE; nIdxOfFdk++)
					{
						if(strValueOfFdk[nIdxOfFdk].GetLength()>1)
							m_pDevCmn->fnSCR_DisplayString(nIdxOfFdk + 11, strValueOfFdk[nIdxOfFdk]);
					}
				}
				m_pDevCmn->fnSCR_DisplayScreen(165, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
				bShowScreen = FALSE;
			}

			///////////////////////////////////
			// GET KEY STRING
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT )
					return RES_USER_EXIT;
				else if (GetKeyStr == S_TIMEOVER)
				{
					// [#2574] US Justin GivePay Enhancement3
					//return RES_USER_TIMEOUT;
					if(P_NH_GivePay_NeedMoreTime() == RES_OK)
						bShowScreen = TRUE;
					else
						return RES_USER_TIMEOUT;
					// End of [#2574]
				}
				else if( (GetKeyStr==L"F1") || (GetKeyStr==L"F2") || (GetKeyStr==L"F3") || (GetKeyStr==L"F4") || 
						 (GetKeyStr==L"F5") || (GetKeyStr==L"F6") || (GetKeyStr==L"F7") || (GetKeyStr==L"F8") )
				{
					nIdxOfFdk = Asc2Int(GetKeyStr.Right(1) ) - 1;
					if(strValueOfFdk[nIdxOfFdk].GetLength() > 1)
					{
						strTemp = strValueOfFdk[nIdxOfFdk].Left(1);
						if(strTemp == GetCurrencySymbol())
						{
							m_GivePayData.m_strGPTranAmount.Format(L"%d", Dollar2Cent(strValueOfFdk[nIdxOfFdk].Mid(1)));
							NHDEBUG(DBG_INFO, (_T("Selected Amount = [%s]\n"), m_GivePayData.m_strGPTranAmount ));
							return RES_OK;
						}
						else
							return RES_USER_PREV;
					}
				}
			}
			Delay_Msg(50);
		}
		return nRes;
	}
	else if( nAmtInputMethod == 1 )	// Cards does not have fixed amounts => Free Entering...
	{
		// [#2574] US Justin GivePay Enhancement3
		/*
		int nMinInput, nMaxInput;	// Cents
		if( !m_GivePayData.GetGPCardMinMaxAmount( nMinInput, nMaxInput) )
		{
			NHDEBUG(DBG_CALL, (L"ERROR ON finding Minimum and Maximum Amount\n"));
			return RES_USER_EXIT;
		}
		*/

		int			nRetryCount = 3;
		int			nInputAmount = 0;
		CString		GetKeyStr, strTemp;
		BOOL		bShowScreen = TRUE;

		long nScrTimeOut = GetTransactionScreenTimeOut();
		if(nScrTimeOut==0)							// ADA Mode....Jack removed.
			return RES_USER_EXIT;

		NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
		g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

		while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
		{		
			///////////////////////////////////
			// DISPLAY SCREEN
			if (bShowScreen == TRUE)
			{
				NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));			
				m_pDevCmn->fnSCR_DisplayPrevSet(166);
				// Setting Screen
				{
					// SCREEN OVERLAY
					if(m_GivePayData.m_nGPServiceType == GPAY_SERVICE_GIFTCARD) 
						m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_GC_AMOUNT));
					else
						m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_WL_AMOUNT));

					// Guide 
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_166001) );

					// Min & Max
					// [#2574] US Justin GivePay Enhancement3
					//strTemp.Format(L"%s %0.2f - %s %0.2f", GetCurrencySymbol(), (0.01*nMinInput+0.0001), GetCurrencySymbol(), (0.01*nMaxInput+0.0001) );
					strTemp = m_GivePayData.GetAvailableAmountRange(&arrAmountRange);
					m_pDevCmn->fnSCR_DisplayString(4, strTemp );

					// Card Image
					strTemp = m_GivePayData.GetGPImageFileName(GPAY_IMG_ICON_SELECTED_CARD);
					NHDEBUG(DBG_INFO, ( L"Assign Selected Card Image [%s]\n", strTemp ));
					m_pDevCmn->fnSCR_DisplayString(5, strTemp);
		
					// F7
					m_pDevCmn->fnSCR_DisplayString(17, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165903) );		// Back

					// Input Parameters
					m_pDevCmn->fnSCR_DisplayString(20, L"DOLLAR");		// Symbol
					m_pDevCmn->fnSCR_DisplayString(21, L"INT_CENT");	// Amount Type
					m_pDevCmn->fnSCR_DisplayString(22, L"1");			// min
					m_pDevCmn->fnSCR_DisplayString(23, L"6");			// max
					m_pDevCmn->fnSCR_DisplayString(24, L"off");			// auto run
				}
				m_pDevCmn->fnSCR_DisplayScreen(166, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
				bShowScreen = FALSE;
			}

			///////////////////////////////////
			// GET KEY STRING
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
					return RES_USER_EXIT;
				else if (GetKeyStr == S_TIMEOVER)
				{
					// [#2574] US Justin GivePay Enhancement3
					//return RES_USER_TIMEOUT;
					if(P_NH_GivePay_NeedMoreTime() == RES_OK)
						bShowScreen = TRUE;
					else
						return RES_USER_TIMEOUT;
					// End of [#2574]
				}
				else if (GetKeyStr == L"F7")
					return RES_USER_PREV;
				else
				{
					CString sEnterVal = GetKeyStr;						
					sEnterVal.Replace(GetCurrencySymbol(), L"");
					sEnterVal.TrimLeft();
					nInputAmount = Asc2Int(sEnterVal);

					NHDEBUG(DBG_INFO, (_T("Input Amount converted integer = [%d]\n"), nInputAmount));

					// [#2574] US Justin GivePay Enhancement3
					/*
					if( (nInputAmount>=nMinInput) && (nInputAmount<=nMaxInput) )
					{
						m_GivePayData.m_strGPTranAmount.Format(L"%d", nInputAmount);
						NHDEBUG(DBG_INFO, (_T("Selected Amount = [%s]\n"), m_GivePayData.m_strGPTranAmount ));
						return  RES_OK;
					}

					if( nInputAmount < nMinInput )
						LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113105), INFO_SCR_TIMEOUT);
					else if( nInputAmount > nMaxInput )
						LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113104), INFO_SCR_TIMEOUT);
					*/

					if(m_GivePayData.IsAmountIncludedInRange(&arrAmountRange, nInputAmount) )
					{
						m_GivePayData.m_strGPTranAmount.Format(L"%d", nInputAmount);
						NHDEBUG(DBG_INFO, (_T("Selected Amount = [%s]\n"), m_GivePayData.m_strGPTranAmount ));
						return  RES_OK;
					}
					LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_166902), INFO_SCR_TIMEOUT);
					// End of [#2574]

					nRetryCount--;
					bShowScreen = TRUE;
				}
			}
			Delay_Msg(50);
		}

		if (nRetryCount <= 0)
			return RES_USER_EXIT;

		return RES_USER_TIMEOUT;
	}
	return RES_USER_EXIT;
	// End of [#2574]
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_SelectGreeting()
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_Greet");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_SelectGreeting()]\n"));

	P_NH_GivePay_DownloadImages(TRUE, GPAY_ID_GREETINGS);

	// Initialize Parameters
	m_GivePayData.m_strSelectedGreeting = _T("");
	m_GivePayData.m_strStg_GifAmt = _T("");
	int nPageNum = 0;	
	int nNextPage, nIdxOfFdk;
	BIZ_RETURN nRes = RES_USER_TIMEOUT;
	CString	strValueOfFdk[MENU_SELECT_SIZE] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };
	CString	strImageFile[MENU_SELECT_SIZE] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
			
			m_pDevCmn->fnSCR_DisplayPrevSet(165);

			// Setting Screen
			{
				// SCREEN OVERLAY
				m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_GC_DIGITAL_GREET));

				// Initialize FDK and Image File
				for(nIdxOfFdk=0; nIdxOfFdk<MENU_SELECT_SIZE; nIdxOfFdk++)
				{
					strValueOfFdk[nIdxOfFdk] = L"";
					strImageFile[nIdxOfFdk] = L"";
				}

				// Fill Button Text (UP TO 5 BUTTONs) : Sub(1, 1:exist, 0:no) + ArrayID(2) + ItemID(6)  + Name
				if(m_GivePayData.FillGPButtonItems( GPAY_ID_GREETINGS, nPageNum, strValueOfFdk, strImageFile, &nNextPage, 5) < 0)
				{
					NHDEBUG(DBG_CALL, (L"Fill Greeting FAILURE\n"));
					return RES_USER_EXIT;
				}

				// Sub Title - Greeting fee notice
				int nGreetingFee = Dollar2Cent(m_GivePayData.m_strGreetingFee);
				CString strGreetingNotice;
				if(nGreetingFee>0)
				{
					strGreetingNotice = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165004);
					CString sGreetValue;
					sGreetValue.Format(L"%s %s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(Dollar2Cent(m_GivePayData.m_strGreetingFee) ) ) );
					strGreetingNotice.Replace(L"XXXXX", sGreetValue);
				}
				else
					strGreetingNotice = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165005);
				m_pDevCmn->fnSCR_DisplayString(4, strGreetingNotice);	


				// Assign Image File Name
				for(nIdxOfFdk=0; nIdxOfFdk<(MENU_SELECT_SIZE-2); nIdxOfFdk++)
				{
					NHDEBUG(DBG_INFO, (L"Assign Image [F%d] = [%s]\n", (nIdxOfFdk+1), strImageFile[nIdxOfFdk] ));
					m_pDevCmn->fnSCR_DisplayString(nIdxOfFdk + 21, strImageFile[nIdxOfFdk] );
				}

				// F6
				if( nNextPage>0 )
					strValueOfFdk[5].Format( L"000999998%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165904) );	// More Choices

				// F7
				strValueOfFdk[6].Format( L"000999999%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165903) );		// Back
				
				// F8
				strValueOfFdk[7].Format( L"000999997%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165901) );		// Skip

				// Fill Button
				for(nIdxOfFdk=0; nIdxOfFdk<MENU_SELECT_SIZE; nIdxOfFdk++)
				{
					if(strValueOfFdk[nIdxOfFdk].GetLength()>9)
						m_pDevCmn->fnSCR_DisplayString(nIdxOfFdk + 11, strValueOfFdk[nIdxOfFdk].Mid(9) );
				}
			}
			m_pDevCmn->fnSCR_DisplayScreen(165, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				return RES_USER_EXIT;
			else if (GetKeyStr == S_TIMEOVER)
			{
				// [#2574] US Justin GivePay Enhancement3
				//return RES_USER_TIMEOUT;
				if(P_NH_GivePay_NeedMoreTime() == RES_OK)
					bShowScreen = TRUE;
				else
					return RES_USER_TIMEOUT;
				// End of [#2574]
			}
			else if( (GetKeyStr==L"F1") || (GetKeyStr==L"F2") || (GetKeyStr==L"F3") || (GetKeyStr==L"F4") || 
					 (GetKeyStr==L"F5") || (GetKeyStr==L"F6") || (GetKeyStr==L"F7") || (GetKeyStr==L"F8") )
			{
				nIdxOfFdk = Asc2Int(GetKeyStr.Right(1) ) - 1;
				CString strSelected = strValueOfFdk[nIdxOfFdk];

				if(strSelected.GetLength() > 9)
				{
					CString strSelCardID = strSelected.Mid(3,6);
					if(strSelCardID == L"999999")					// Back
					{
						if(nPageNum>0 )
						{
							nPageNum--;
							bShowScreen = TRUE;
						}
						else
							return RES_USER_PREV;
					}
					else if(strSelCardID == L"999998")				// More Choices
					{
						nPageNum++;
						bShowScreen = TRUE;
					}
					else if(strSelCardID == L"999997")				// Skip
					{
						m_GivePayData.m_strSelectedGreeting = L"";
						NHDEBUG(DBG_INFO, (_T("Custoner rejected Greetings\n") ) );
						return RES_OK;
					}
					else
					{
						m_GivePayData.m_strSelectedGreeting = m_GivePayData.m_arrGreetings.GetAt(Asc2Int(strSelected.Mid(1,2)));
						m_GivePayData.m_strStg_GifAmt.Format(L"%0.2f", 0.01*Dollar2Cent(m_GivePayData.m_strGreetingFee) );
						NHDEBUG(DBG_INFO, (_T("Selected Greeting ID = [%3]\n"), strSelected.Mid(3,6)));
						return RES_OK;

					}
				}
				else
					return RES_USER_EXIT;
			}
		}
		Delay_Msg(50);
	}
	return nRes;
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_EnterPhoneNumber(int nType)
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_Phone");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_EnterPhoneNumber(%d)]\n", nType));

	if(nType == GPAY_PHONETYPE_SENDER )		m_GivePayData.m_strGPPhoneNumSnd = _T("");
	else									m_GivePayData.m_strGPPhoneNumRcv = _T("");

	BIZ_RETURN nRes = RES_USER_TIMEOUT;
	int			nRetryCount = 3;
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(167);

			// Setting Screen
			{
				// SCREEN OVERLAY
				if(nType == GPAY_PHONETYPE_SENDER )	
					m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_GC_SENDER_PHONE));
				else if(nType == GPAY_PHONETYPE_RECEIPIENT )	
					m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_GC_RECIPENT_PHONE));
				else if(nType == GPAY_PHONETYPE_WIRELESS )	
					m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_WL_PHONE));

				// press enter when finished 
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_154003) );
	
				// F7
				m_pDevCmn->fnSCR_DisplayString(17, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165903) );		// Back
			}
			m_pDevCmn->fnSCR_DisplayScreen(167, GPAY_KEYIN_TIME_OUT, PIN_AMOUNT_FTN_MODE);
			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				return RES_USER_EXIT;
			else if (GetKeyStr == S_TIMEOVER)
			{
				// [#2574] US Justin GivePay Enhancement3
				//return RES_USER_TIMEOUT;
				if(P_NH_GivePay_NeedMoreTime() == RES_OK)
					bShowScreen = TRUE;
				else
					return RES_USER_TIMEOUT;
				// End of [#2574]
			}
			else if (GetKeyStr == L"F7")				// Back
				return RES_USER_PREV;
			else 
			{						
				NHDEBUG(DBG_INFO, (L"Validating User Input[%s]\n", GetKeyStr));
				if(	GetKeyStr.GetLength() == 10)
				{
					if(nType == GPAY_PHONETYPE_SENDER )		m_GivePayData.m_strGPPhoneNumSnd = GetKeyStr.Left(10);
					else									m_GivePayData.m_strGPPhoneNumRcv = GetKeyStr.Left(10);
					NHDEBUG(DBG_INFO, (L"Phone Number Entered. type=[%d]:[%s]\n", nType, GetKeyStr.Left(10) ));
					return RES_OK;
				}
				else
					NHDEBUG(DBG_INFO, (L"length of entered string is not 10 \n"));

				nRetryCount--;
				if(nRetryCount>0)
				{
					LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_154005), INFO_SCR_TIMEOUT);
					bShowScreen = TRUE;
				}				
			}
		}
		Delay_Msg(50);
	}

	if (nRetryCount <= 0)
	{
		LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);
		return RES_USER_EXIT;
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_ConfirmSMS()
{
	// Initialize Parameters
	BIZ_RETURN nRes = RES_USER_TIMEOUT;

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
			
			m_pDevCmn->fnSCR_DisplayPrevSet(165);

			// Setting Screen
			{
				// SCREEN OVERLAY
				if(m_GivePayData.m_nGPServiceType == GPAY_SERVICE_GIFTCARD) 
					m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_GC_SMS_FEE_CONFIRM));
				else
					m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_WL_SMS_FEE_CONFIRM));

				// Clear Image File Name
				for(int i=0; i<(MENU_SELECT_SIZE-2); i++)
					m_pDevCmn->fnSCR_DisplayString(i + 21, L"" );

				// F7
				m_pDevCmn->fnSCR_DisplayString(17, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165903) );		// Back

				// F8
				m_pDevCmn->fnSCR_DisplayString(18, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165905) );		// Authorize Text
			}
			m_pDevCmn->fnSCR_DisplayScreen(165, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT )
				return RES_USER_EXIT;
			else if (GetKeyStr == S_TIMEOVER)
			{
				// [#2574] US Justin GivePay Enhancement3
				//return RES_USER_TIMEOUT;
				if(P_NH_GivePay_NeedMoreTime() == RES_OK)
					bShowScreen = TRUE;
				else
					return RES_USER_TIMEOUT;
				// End of [#2574]
			}
			else if(GetKeyStr == L"F7")
				return RES_USER_PREV;
			else if(GetKeyStr == L"F8")
				return RES_OK;
		}
		Delay_Msg(50);
	}
	return nRes;
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_SelectWireless()
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_Cat.");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_SelectCategory()]\n"));

	P_NH_GivePay_DownloadImages(TRUE, GPAY_ID_WIRELESS);

	// Initialize Parameters
	m_GivePayData.m_strSelectedCard = L"";
	int nPageNum = 0;	
	int nNextPage, nIdxOfFdk;
	BIZ_RETURN nRes = RES_USER_TIMEOUT;
	CString	strValueOfFdk[MENU_SELECT_SIZE] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };
	CString	strImageFile[MENU_SELECT_SIZE] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
			
			m_pDevCmn->fnSCR_DisplayPrevSet(165);

			// Setting Screen
			{
				// SCREEN OVERLAY
				m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_WL_SELECTION));

				// Initialize FDK and Image File
				for(nIdxOfFdk=0; nIdxOfFdk<MENU_SELECT_SIZE; nIdxOfFdk++)
				{
					strValueOfFdk[nIdxOfFdk] = L"";
					strImageFile[nIdxOfFdk] = L"";
				}

				// Fill Button Text (UP TO 5 BUTTONs) : Sub(1, 1:exist, 0:no) + ArrayID(2) + ItemID(6)  + Name
				if(m_GivePayData.FillGPButtonItems( GPAY_ID_WIRELESS, nPageNum, strValueOfFdk, strImageFile, &nNextPage) < 0)
				{
					NHDEBUG(DBG_CALL, (L"Fill Wireless FAILURE\n"));
					return RES_USER_EXIT;
				}

				// Fee Notice (Extra Line below Sub Title : APValue 6)
				// [#2574] US Justin Add Fee notice 
				CString strFeeNotice = L""; 
				if(Dollar2Cent(m_GivePayData.m_strWirelessFee) > 0)
				{
					strFeeNotice = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165006);
					CString sFeeValue; 
					sFeeValue.Format(L"%s %s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(Dollar2Cent(m_GivePayData.m_strWirelessFee) ) ) );
					strFeeNotice.Replace(L"XXXXX", sFeeValue);
				}
				else
					strFeeNotice = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165007);
				m_pDevCmn->fnSCR_DisplayString(6, strFeeNotice );
				// End [#2574]
				
				// Assign Image File Name
				for(nIdxOfFdk=0; nIdxOfFdk<(MENU_SELECT_SIZE-2); nIdxOfFdk++)
				{
					NHDEBUG(DBG_INFO, (L"Assign Image [F%d] = [%s]\n", (nIdxOfFdk+1), strImageFile[nIdxOfFdk] ));
					m_pDevCmn->fnSCR_DisplayString(nIdxOfFdk + 21, strImageFile[nIdxOfFdk] );
				}

				// F7
				strValueOfFdk[6].Format( L"000999999%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165903) );		// Back
				
				// F8
				if( nNextPage>0 )
					strValueOfFdk[7].Format( L"000999998%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165904) );	// More Choices

				// Fill Button
				for(nIdxOfFdk=0; nIdxOfFdk<MENU_SELECT_SIZE; nIdxOfFdk++)
				{
					if(strValueOfFdk[nIdxOfFdk].GetLength()>9)
						m_pDevCmn->fnSCR_DisplayString(nIdxOfFdk + 11, strValueOfFdk[nIdxOfFdk].Mid(9) );
				}
			}
			m_pDevCmn->fnSCR_DisplayScreen(165, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				return RES_USER_EXIT;
			else if (GetKeyStr == S_TIMEOVER)
			{
				// [#2574] US Justin GivePay Enhancement3
				//return RES_USER_TIMEOUT;
				if(P_NH_GivePay_NeedMoreTime() == RES_OK)
					bShowScreen = TRUE;
				else
					return RES_USER_TIMEOUT;
				// End of [#2574]
			}
			else if( (GetKeyStr==L"F1") || (GetKeyStr==L"F2") || (GetKeyStr==L"F3") || (GetKeyStr==L"F4") || 
					 (GetKeyStr==L"F5") || (GetKeyStr==L"F6") || (GetKeyStr==L"F7") || (GetKeyStr==L"F8") )
			{
				nIdxOfFdk = Asc2Int(GetKeyStr.Right(1) ) - 1;
				CString strSelected = strValueOfFdk[nIdxOfFdk];

				if(strSelected.GetLength() > 9)
				{
					CString strSelCardID = strSelected.Mid(3,6);
					if(strSelCardID == L"999999")					// Back
					{
						if(nPageNum>0 )
						{
							nPageNum--;
							bShowScreen = TRUE;
						}
						else
							return RES_USER_PREV;
					}
					else if(strSelCardID == L"999998")				// More Choices
					{
						nPageNum++;
						bShowScreen = TRUE;
					}
					else
					{
						m_GivePayData.m_strSelectedCard = m_GivePayData.m_arrWireless.GetAt(Asc2Int(strSelected.Mid(1,2)));
						NHDEBUG(DBG_INFO, (_T("Selected Card ID = [%s]\n"), strSelected.Mid(3,6) ));
						return RES_OK;		
					}
				}
				else
					return RES_USER_EXIT;
			}
		}
		Delay_Msg(50);
	}
	return nRes;
}

BIZ_RETURN CTranCmn::P_NH_GivePay_Staging(BOOL bShowScreen)
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_Staging");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_Staging()]\n"));

	// Assign CardHolder Name
	if(	m_strHolderName.GetLength() > 0)		// LastName, FirstName
	{
		CStringArray	arrName;
		SplitString(m_strHolderName, ",", arrName);
		if(arrName.GetSize()<=1)
			m_GivePayData.m_strGPHolderFName =  m_strHolderName;
		else
		{
			m_GivePayData.m_strGPHolderFName = arrName.GetAt(1);
			m_GivePayData.m_strGPHolderLName = arrName.GetAt(0);
		}
	}
	NHDEBUG(DBG_CALL, (L"   Card Holder FName = [%s], LName=[%s]\n", m_GivePayData.m_strGPHolderFName, m_GivePayData.m_strGPHolderLName));

	// DISPLAY SCREEN
	if(bShowScreen)
		P_NH_GivePay_DisplayWait();

	m_GivePayData.m_nTransactionStep = GPAY_TRAN_PURCHASECARD_STAGE;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if( nRes == RES_OK )	
		NHDEBUG(DBG_CALL, (L"Staging OK\n"));
	else
	{
		NHDEBUG(DBG_CALL, (L"Staging FAILURE\n"));
		// Display Error Description
		if(m_GivePayData.m_strErrorDesc.GetLength() > 0)
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_GivePayData.m_strErrorDesc, INFO_SCR_TIMEOUT);	
	}
	return nRes;
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_PurchaseSummary()
{
	NVDump('O', 'C', "00", L"GivePay", L"");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::()]\n"));
	BIZ_RETURN nRes = RES_USER_TIMEOUT;

	CString		GetKeyStr, strTemp, strTemp2;
	BOOL		bShowScreen = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	BOOL bIsTranAccepted = FALSE;

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
			
			m_pDevCmn->fnSCR_DisplayPrevSet(168);	

			// Setting Screen
			{
				// SCREEN OVERLAY
				m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_GC_ORDER_CONFIRM));

				// Selected Card (Image)
				strTemp = m_GivePayData.GetGPImageFileName(GPAY_IMG_ICON_SELECTED_CARD);
				NHDEBUG(DBG_INFO, ( L"Assign Selected Card Image [%s]\n", strTemp ));
				m_pDevCmn->fnSCR_DisplayString(3, strTemp);

				// Selected Greeting (Image)
				strTemp = m_GivePayData.GetGPImageFileName(GPAY_IMG_ICON_SELECTED_GREET);
				NHDEBUG(DBG_INFO, ( L"Assign Greeting Image [%s]\n", strTemp ));
				m_pDevCmn->fnSCR_DisplayString(4, strTemp);

				// Send From / Paying to mobile Number (AP Value 21)
				if(m_GivePayData.m_nGPServiceType == GPAY_SERVICE_GIFTCARD) 
				{
					strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_168001);			// Sending From: 
					if(m_GivePayData.m_strStg_Snd_FullName.GetLength() > 0)	strTemp += m_GivePayData.m_strStg_Snd_FullName;
					else													strTemp += m_GivePayData.m_strStg_Snd_Phone;		//m_strGPPhoneNumSnd;
				}
				else
					strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_168003);			// Paying to mobile number
				NHDEBUG(DBG_INFO, ( L"Assign Send From or Paying to Mobile Number = [%s]\n", strTemp ));
				m_pDevCmn->fnSCR_DisplayString(21, strTemp);

				// Send To / recipient number (AP Value 22)
				if(m_GivePayData.m_nGPServiceType == GPAY_SERVICE_GIFTCARD) 
				{
					strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_168002);			// Sending to: 
					if(m_GivePayData.m_strStg_Rcv_FullName.GetLength() > 0)	strTemp += m_GivePayData.m_strStg_Rcv_FullName;
					else													strTemp += m_GivePayData.m_strStg_Rcv_Phone;
				}
				else
					strTemp = m_GivePayData.m_strStg_Rcv_Phone;									//  mobile number
				NHDEBUG(DBG_INFO, ( L"Assign Send To or Receipient Number = [%s]\n", strTemp ));
				m_pDevCmn->fnSCR_DisplayString(22, strTemp);

				// Card Name (AP Value 23)
				NHDEBUG(DBG_INFO, ( L"Assign Card Number = [%s]\n", m_GivePayData.m_strStg_ProductName ));
				m_pDevCmn->fnSCR_DisplayString(23, m_GivePayData.m_strStg_ProductName);

				// Include $2 Convenience Fee (AP Value 24)
				strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_168004);				// Includes $XXXXX Convenience Fee
				strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent( Int2Asc(Dollar2Cent(m_GivePayData.m_strStg_FeeAmt)) ) );
				strTemp.Replace( L"XXXXX",  strTemp2);
				NHDEBUG(DBG_INFO, ( L"Assign Convenience Fee = [%s]\n", strTemp ));
				m_pDevCmn->fnSCR_DisplayString(24, strTemp);

				// Sub Total (APValue 25)
				strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc( Dollar2Cent(m_GivePayData.m_strStg_SubTotal) - Dollar2Cent(m_GivePayData.m_strStg_GifAmt) ) ) );
				NHDEBUG(DBG_INFO, ( L"Assign Sub Total = [%s]\n", strTemp ));
				m_pDevCmn->fnSCR_DisplayString(25, strTemp);

				int nMaxAdditionalFeeNum = 0;
				int nPresentLocation = 0;
				if(m_GivePayData.m_nGPServiceType == GPAY_SERVICE_GIFTCARD) 
				{
					// Greeting
					if(	m_GivePayData.m_strSelectedGreeting.GetLength() > 0 )
					{										
						m_pDevCmn->fnSCR_DisplayImage(1, TRUE);						// Show Separator

						// Personal Greeting - "HAPPY Holidays"
						CString strGreetingName =  m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_168005) + GetJSONNodeValue(m_GivePayData.m_strSelectedGreeting, L"defaultMessage");
						if( Dollar2Cent(m_GivePayData.m_strStg_GifAmt) <= 0)
						{
							m_pDevCmn->fnSCR_DisplayString(26, strGreetingName );
							NHDEBUG(DBG_INFO, ( L"Gif Amount = [0], Show Greeting Name[%s], (One line) only withouth other information\n", strGreetingName ));
						}
						else
						{
							// Greeting Name and its Value
							m_pDevCmn->fnSCR_DisplayString(27, strGreetingName );

							strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent( Int2Asc( Dollar2Cent(m_GivePayData.m_strStg_GifAmt) ) ) );
							NHDEBUG(DBG_INFO, ( L"Assign 1st Line : Greeting Name =[%s], Amount = [%s]\n", strGreetingName, strTemp ));
							m_pDevCmn->fnSCR_DisplayString(29, strTemp);
							
							// Sales Tax
							if( (Dollar2Cent(m_GivePayData.m_strStg_TaxAmt) > 0) && (m_GivePayData.m_arrStg_ExtraFee.GetSize()>0) )
							{
								nMaxAdditionalFeeNum = 1;
								nPresentLocation = 1;
							}
						}
					}
					else
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);					// Hide Separator
				}
				else
				{
					if(m_GivePayData.m_arrStg_ExtraFee.GetSize()>0)
					{
						m_pDevCmn->fnSCR_DisplayImage(1, TRUE);						// Show Separator
						nMaxAdditionalFeeNum = 2;
					}
					else
						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);					// Hide Separator
				}

				// Sales Tax or Extra Fees
				if(nMaxAdditionalFeeNum>0)
				{
					CString strFeeText, strFeeLabel, strFeeAmount;
					int nMaxItemNum = m_GivePayData.m_arrStg_ExtraFee.GetSize();
					if(nMaxItemNum>nMaxAdditionalFeeNum) 
						nMaxItemNum = nMaxAdditionalFeeNum;

					for(int i=0; i<nMaxItemNum; ++i)
					{
						strFeeText = m_GivePayData.m_arrStg_ExtraFee.GetAt(i);
						strFeeLabel = GetJSONNodeValue(strFeeText, L"label") ;
						strFeeAmount = GetJSONNodeValue(strFeeText, L"amount");
						strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent( Int2Asc( Dollar2Cent(strFeeAmount) ) ) );

						m_pDevCmn->fnSCR_DisplayString(27+i+nPresentLocation, strFeeLabel);
						m_pDevCmn->fnSCR_DisplayString(29+i+nPresentLocation, strTemp);
						NHDEBUG(DBG_INFO, ( L"Extra Fee[%d] (%s) => Label=[%s], Amount=[%s]\n", i, strFeeText, strFeeLabel, strTemp ));
					}
				}

				// Total Amount (Grand Total) Debited (AP Value 31)
				strTemp.Format(L"%s %s%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_168006), GetCurrencySymbol(), MakeMoneyCent( Int2Asc( Dollar2Cent(m_GivePayData.m_strStg_GrandTotal))));
				NHDEBUG(DBG_INFO, ( L"Assign Total Amount = [%s]\n", strTemp ));
				m_pDevCmn->fnSCR_DisplayString(31, strTemp);

				// F5 View Terms
				if(GetJSONNodeValue(m_GivePayData.m_strSelectedCard, L"textModeTermsAndConditions").GetLength()>10)
					m_pDevCmn->fnSCR_DisplayString(15, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165906) );	

				// F7 Exit
				m_pDevCmn->fnSCR_DisplayString(17, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_EXIT) );	
					
				// F8 Confirm
				m_pDevCmn->fnSCR_DisplayString(18, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165907) );
			}
			m_pDevCmn->fnSCR_DisplayScreen(168, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				return RES_USER_EXIT;
			else if (GetKeyStr == S_TIMEOVER)
			{
				// [#2574] US Justin GivePay Enhancement3
				//return RES_USER_TIMEOUT;
				if(P_NH_GivePay_NeedMoreTime() == RES_OK)
					bShowScreen = TRUE;
				else
					return RES_USER_TIMEOUT;
				// End of [#2574]
			}
			else if (GetKeyStr == L"F5")					// F5, View Terms
				return RES_GIVEPAY_VIEWTNC;
			else if (GetKeyStr == L"F7")					// F7, Exit
				return RES_USER_EXIT;
			else if (GetKeyStr == L"F8")					// F8, Confirm
				return RES_OK;
		}
		Delay_Msg(50);
	}
	return nRes;
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_ViewTerms()
{
	// Initialize Parameters
	BIZ_RETURN nRes = RES_USER_TIMEOUT;

	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
			
			m_pDevCmn->fnSCR_DisplayPrevSet(165);

			// Setting Screen
			{
				// SCREEN OVERLAY
				m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_TAC));				// [#2574] US Justin GivePay Enhancement3

				// Title
				//m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165003) );		// Terms of Use	// [#2574] US Justin GivePay Enhancement3
				
				// Terms : Multi Line
				CString strTC = GetJSONNodeValue(m_GivePayData.m_strSelectedCard, L"textModeTermsAndConditions");
				strTC.Replace(L"\\r\\n", L"\n");
				m_pDevCmn->fnSCR_DisplayString(5, strTC );	

				// Clear Image File Name
				for(int i=0; i<(MENU_SELECT_SIZE-2); i++)
					m_pDevCmn->fnSCR_DisplayString(i + 21, L"" );

				// F7 Exit
				m_pDevCmn->fnSCR_DisplayString(17, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_EXIT) );	

				// F8 Confirm
				m_pDevCmn->fnSCR_DisplayString(18, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_165907) );
			}
			m_pDevCmn->fnSCR_DisplayScreen(165, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT )
				return RES_USER_EXIT;
			else if (GetKeyStr == S_TIMEOVER)
			{
				// [#2574] US Justin GivePay Enhancement3
				//return RES_USER_TIMEOUT;
				if(P_NH_GivePay_NeedMoreTime() == RES_OK)
					bShowScreen = TRUE;
				else
					return RES_USER_TIMEOUT;
				// End of [#2574]
			}
			else if(GetKeyStr == L"F7")
				return RES_USER_EXIT;
			else if(GetKeyStr == L"F8")
				return RES_OK;
		}
		Delay_Msg(50);
	}
	return nRes;
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_EnterZipCode()
{
	NVDump('O', 'C', "00", L"GivePay", L"GP_Category");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_EnterZipCode()]\n"));
	m_GivePayData.m_strGPZipCode = _T("");

	CString			GetKeyStr;
	int				nRetryCount = 3;
	BOOL			bShowScreen = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
	{

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(169);
			// Setting Screen
			{
				// SCREEN OVERLAY
				m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_ZIPCODE));

				// press enter when finished 
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_154003) );

				// F7 Exit
				// m_pDevCmn->fnSCR_DisplayString(17, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_EXIT) );	// [#2574] US Justin GivePay Enhancement : Remove Exit

				// Input Parameters
				m_pDevCmn->fnSCR_DisplayString(21, L"STRING");		// Input Type
				m_pDevCmn->fnSCR_DisplayString(22, L"1");			// min
				m_pDevCmn->fnSCR_DisplayString(23, L"5");			// max
				m_pDevCmn->fnSCR_DisplayString(24, L"off");			// auto run
			}
			m_pDevCmn->fnSCR_DisplayScreen(169, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));	

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT || GetKeyStr == L"F7")
				return RES_USER_EXIT;
			else if (GetKeyStr == S_TIMEOVER)
			{
				// [#2574] US Justin GivePay Enhancement3
				//return RES_USER_TIMEOUT;
				if(P_NH_GivePay_NeedMoreTime() == RES_OK)
					bShowScreen = TRUE;
				else
					return RES_USER_TIMEOUT;
				// End of [#2574]
			}
			else if (GetKeyStr.GetLength() > 0)
			{
				NHDEBUG(DBG_INFO, (L"Validating User Input[%s]\n", GetKeyStr));
				if(	GetKeyStr.GetLength() == 5)
				{
					m_GivePayData.m_strGPZipCode = GetKeyStr;	
					return RES_OK;
				}
				else
					NHDEBUG(DBG_INFO, (L"length of entered code is not 5\n"));

				nRetryCount--;
				if(nRetryCount>0)
				{
					LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_166901), INFO_SCR_TIMEOUT);
					bShowScreen = TRUE;
				}				
			}
		}
		Delay_Msg(50);
	}

	if (nRetryCount <= 0)
	{
		LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);
		return RES_USER_EXIT;
	}
	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	return RES_USER_TIMEOUT;
}

BIZ_RETURN	CTranCmn::P_NH_GivePay_Purchase_Auth(BOOL bShowScreen)
{
	NVDump('O', 'C', "00", L"GivePay", L"GcardPcs_Auth");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_Purchase_Auth()]\n"));

	// Assign Card Number, and Expiration Date
	CString strTrack2Data = m_sCardData.strISO2Data;
	int nLoc =  strTrack2Data.Find( L"=");
	if(nLoc<= 0)
		return RES_HOST_SEND_ERR;

	CString strExpDate = strTrack2Data.Mid(nLoc+1);
	if(strExpDate.GetLength()<4)
		return RES_HOST_SEND_ERR;

	// DISPLAY SCREEN
	if(bShowScreen)
		P_NH_GivePay_DisplayWait();

	m_GivePayData.m_strGPPan = strTrack2Data.Left(nLoc);
	m_GivePayData.m_strGPCardExp = strExpDate;
	NHDEBUG(DBG_CALL, (L"   PAN = [%s], ExpDate = [%s]\n", m_GivePayData.m_strGPPan, m_GivePayData.m_strGPCardExp));

	//m_sUserSelection.strMoney.Format(L"%012d",  Asc2Int(m_GivePayData.m_strGPTranAmount) );

	m_GivePayData.m_nTransactionStep = GPAY_TRAN_PURCHASECARD_APPROVAL;
	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if(nRes == RES_OK)	
		NHDEBUG(DBG_CALL, (L"Purchase Approval OK\n"));
	else				
	{
		NHDEBUG(DBG_CALL, (L"Purchase Approval FAILURE\n"));
		LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);
	}
	return nRes;
}

BIZ_RETURN CTranCmn::P_NH_GivePay_PrintReceipt(BOOL bSuccess)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_GivePay_PrintReceipt]\n"));
	CString	strTemp, strPrintData ;
	strPrintData = L"";
	int i;

	if(bSuccess)
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(165);
		m_pDevCmn->fnSCR_DisplayString(2, m_GivePayData.GetGPImageFileName(GPAY_IMG_OL_THANK));

		// Clear Image File Name
		for(i=0; i<(MENU_SELECT_SIZE-2); i++)
			m_pDevCmn->fnSCR_DisplayString(i + 21, L"" );

		m_pDevCmn->fnSCR_DisplayScreen(165);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(119);
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119001));
		#if(US_VERSION || CA_VERSION || MX_VERSION)	
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
				m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
			else
				m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
		#endif
		m_pDevCmn->fnSCR_DisplayScreen(119);
	}

	// Space;
	AddPrintData(strPrintData, L" ");

	// ADDRESS 1,2,3,PHONE NUMBER PRINT
	{
		for(i=0; i<4; i++)
		{
			strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1 + i);
			strTemp.TrimLeft();

			if (strTemp.GetLength() > 0)
				AddPrintData(strPrintData, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1 + i));
		}

		// HEADER 1,2 PRINT
		for(i=0; i<2; i++)
		{
			strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER1 + i);
			strTemp.TrimLeft();

			if (strTemp.GetLength() > 0)
				AddPrintData(strPrintData, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER1 + i));
		}
	}

	// Terminal Number, Date and Time, Card information, Order ID
	{
		// TERMINAL NUMBER
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));

		// Date and Time 
		// [#J006] Print Date and Time

		// Current ATM Time
		//char			gDate[9]	= "";
		//char			gTime[7]	= "";
		//GetDateTime(gDate, gTime);
		//AddPrintData(strPrintData, L"%-17.17s = %s %2.2S:%2.2S", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_004), GetCmnLocalDate(&gDate[4], &gDate[6], &gDate[0]), &gTime[0], &gTime[2]);
		
		// Received "creationTime" FORMAT:2018-04-06T18:20:54.51409 Available only approved
		if(m_GivePayData.m_strAuth_TrDateTime.GetLength()>=19)
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_004), m_GivePayData.m_strAuth_TrDateTime.Left(19));
		// End of [#J006]

		// CARD NUMBER
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_006), m_sCardData.strShowNumber);

		// Order ID
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", L"Order ID", m_GivePayData.m_strStg_OrderId);
	}

	AddPrintData(strPrintData, L"----------------------------------------");

	CString strPurchaseSummary = m_GivePayData.MakeGPTransactionSummary(bSuccess, TRUE, SLIP_MAX_COL);
	CStringArray arrSummary;
	SplitString(strPurchaseSummary, SCR_RES_DELIMITER, arrSummary);
	for(i=0; i<arrSummary.GetSize(); i++)
		AddPrintData(strPrintData, arrSummary.GetAt(i) );

	if (m_pDevCmn->fnSPR_PrintReceipt(TRUE, strPrintData, K_1_WAIT, TRUE, FALSE) != TRUE)		// Print Header Image, Data, Wait, Cut, No BCD
		return RES_NG;

	return RES_OK;
}
// End of [#2496]

BIZ_RETURN	CTranCmn::P_NH_GivePay_ScreenReceipt(BOOL bSuccess)
{
	NVDump('O', 'C', "00", L"GivePay", L"");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::()]\n"));
	BIZ_RETURN nRes = RES_USER_TIMEOUT;

	///////////////////////////////////
	// DISPLAY SCREEN

	if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
		return RES_OK;

	m_pDevCmn->fnSCR_DisplayPrevSet(121);

	// Setting Screen
	{
		// Support up to 13 lines
		CString strPurchaseSummary = m_GivePayData.MakeGPTransactionSummary(bSuccess, FALSE, SLIP_MAX_COL);
		CStringArray arrSummary;
		SplitString(strPurchaseSummary, SCR_RES_DELIMITER, arrSummary);
		int nNumMaxLine = arrSummary.GetSize();
		if(nNumMaxLine>13)	
			nNumMaxLine = 13;

		for(int i=0; i<nNumMaxLine; i++)
			m_pDevCmn->fnSCR_DisplayString(i+1, arrSummary.GetAt(i));
	}
	m_pDevCmn->fnSCR_DisplayScreen(121, GPAY_KEYIN_TIME_OUT, PIN_MENU_MODE);
	m_pDevCmn->fstrSCR_GetKeyString(GPAY_KEYIN_TIME_OUT);
	return RES_OK;
}

#endif
// End of [#2513]