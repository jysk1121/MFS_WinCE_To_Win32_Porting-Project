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

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_CA_WelcomeProc()
RETURN TYPE  : TRUE : Business Flow를 처리 했다.
FALSE : Business Flow를 처리하지 않았다.
PARAMETER    : 
DESCRIPTION  : Business Flow의 시작을 판단한다.
-------------------------------------------------------------------*/
BOOL CTranCmn::BIZ_CA_WelcomeProc(BOOL bForceShowing)
{
	if ((m_pDevCmn->fnSCR_GetCurrentScreenNo() != 101) || (bForceShowing == TRUE))
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
			tmp.TrimRight(); //2010.02.16 공백 제거하고 표시하게 수정함

			if (tmp.GetLength() > 0)	// Welcome Message가 1개라도 있는 경우에만 유효함 KSK 2010.03.08
				bExistWelComeMsg = TRUE;

			strWelcome += tmp + L"\n";
		}

		// Setting Screen 
		{
			SetBackImageOnWelcomeScreenLayout();		// [#2187] CA Justin 2013.04.17 Use Function
			
			m_pDevCmn->fnSCR_DisplayPrevSet(101);		// KSK 2010.07.02 Change Background시 Check Cashing Button이 잠시 표시되는 Bug Fix

			// [#2375] US Justin USE Function
			// 1. APValue
			if(bExistWelComeMsg!=TRUE)	
				strWelcome = L"";
			SetCardReadScreenValue(strWelcome);		// Enable Device(MCR, MCR Flicker, RFID) in the routine

			// 2. Setting Additional Function button
			SetAdditionalFunctionButton();			// [#2445] US Justin Make Function

			// [#2187] CA Justin 2013.04.17 Replace with function
			////////////////////////////////////
			// 3. Setting Advertisement
			SetAdvertisementOnWelcomeScreenLayout();

			////////////////////////////////////
			// 4. Setting Weather
			SetWeatherOnWelcomeScreenLayout();

			////////////////////////////////////
			// 5. Setting Notice
			SetNoticeOnWelcomeScreenLayout();
			// End of [#2187]

			////////////////////////////////////
			// 6. ETC
			m_pDevCmn->fnSCR_SetDisplayData(L"APProximity", L"off");

			// 7. TID QR CODE	[#J007]
			SetTidQRCodeOnWelcomeScreenLayout();
		}

		m_pDevCmn->fnSCR_DisplayScreen(101);
		m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);

		// [#2205] US KSK 2013.06.28
		// [#604] NH KSK 2010.01.05 DEV_MCU는 fnAPD_CardEnDisable에서 ON하므로 DEV_MCU는 삭제함
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
			}
			// end of [#2333]
		}
		// end of [#604]
		// end of [#2205]

#ifdef APP_AGING_MODE
		g_AgingCheck.SetTargetTimeAfterSec(5);
#endif

		return FALSE;
	}

#ifdef APP_AGING_MODE
	if (g_AgingCheck.IsElapsedTimes() == TRUE)
	{
		// [#2222] CA Justin 2013.10.03 Add HALO LED Control
//		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);
		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
			m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);
		else
			SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);
		// end of [#2222]

		BIZ_CA_StartFlowProc(FID_NH_NOR_COMMON);
		return TRUE;
	}
#else
	if ((m_pDevCmn->fnSNS_GetEnhancedAudio() == TRUE) && ((DidMediaExist() == FALSE)))
	{
		TurnOffCardReaderFlicker(TRUE);

		BIZ_CA_StartFlowProc(FID_NH_ADA_COMMON);

		return TRUE;
		// END[#2077]
	}
//	else if ((DidMediaExist() == FALSE) && (LIB_MainMenuDeviceEvent() == T_OK) && (m_pDevCmn->fnMCU_GetDeviceStatus() == NORMAL))
	else if ((DidMediaExist() == FALSE) && (LIB_MainMenuDeviceEvent() == DEV_MCU) && (m_pDevCmn->fnMCU_GetDeviceStatus() == NORMAL))	// [#2325] NH KSK 2015.01.22 LIB_MainMenuDeviceEvent return 변경으로 인해 비교문 변경
	{
		TurnOffCardReaderFlicker();

		BIZ_CA_StartFlowProc(FID_NH_NOR_COMMON);

		return TRUE;
	}
	// [#RWC6-162] Add B4U to Canada Release
	else if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 5, EVENT_IN) == DEV_SCR)
	{
		CString strKeyString = m_pDevCmn->fstrSCR_GetKeyString(1);

		NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), strKeyString));

		// [#2350] US Justin 2015.06.17 3rd Party Application
		FLOW_ID	eChosenFlow = FID_END_OF_FLOW;
		if (strKeyString == L"F8")
		{			
			// [#2413] US Justin 2016.04.08 VG for Popmoney and Pin4
			int nNumEnabled, nEnabledServices;
			GetEnabledCardlessServices(&nNumEnabled, &nEnabledServices);
			if( (nNumEnabled==1) && (nEnabledServices==CDLS_SVC_B4U) )
			{
				eChosenFlow = FID_NH_B4U_REQ_INFO_FLOWS; // [#RWC6-16] Bitload4U
			}
		// end of [#RWC6-68]
		}

		if( eChosenFlow != FID_END_OF_FLOW)
		{
			// Turn off MCU Flicker
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);
			else
				SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);

			// Disable MCU and RFID
			m_pDevCmn->fnAPL_DeviceEnDisable(DEV_MCU, DISABLE, TRUE);
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
			{
				m_pDevCmn->fnRFID_EntryDisable();
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
			}

			BIZ_CA_StartFlowProc(eChosenFlow);
			return TRUE;
		}
		// End of [#2350]
	}
	// end of [#RWC6-162]
#endif

	return FALSE;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_CA_NOR_SelectTransaction()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Select Transaction
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_CA_NOR_SelectTransaction()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_CA_NOR_SelectTransaction]\n"));

	m_arHistory.Add(m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_WITHDRAWAL));

	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_WITHDRAWAL;

	return RES_OK;
}
