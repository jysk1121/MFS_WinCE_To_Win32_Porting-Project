#include "stdafx.h"
#include ".\Tran\TranCmn.h"

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

//------------------------------------------------------------------
//	Implement
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: FLOW_Mani_Proc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Business Flow에 따른 분기를 수행한다.
-------------------------------------------------------------------*/
void CTranCmn::BIZ_CA_StartFlowProc(FLOW_ID eFlowID)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::FLOW_Mani_Proc] eFlowID(%d)\n", (int)eFlowID));

	FLOW_ID			eNextFlowID = FID_END_OF_FLOW;

#ifdef MEM_LEAK_CHECK
	DWORD	dwBeforePA=0, dwAfterPA=0;
	DWORD	dwBeforeVA=0, dwAfterVA=0;
	DWORD	dwBeforeTime=0, dwAfterTime=0;

	// Before check.
	if ((eFlowID == FID_NH_ADA_COMMON) || 
		(eFlowID == FID_NH_NOR_COMMON) ||
		(eFlowID == FID_NH_CCW_FLOWS))
	{
		MEMORYSTATUS memStatus;	
		memStatus.dwLength = sizeof(MEMORYSTATUS);
		GlobalMemoryStatus(&memStatus);

		dwBeforePA = memStatus.dwAvailPhys;
		dwBeforeVA = memStatus.dwAvailVirtual;
		dwBeforeTime = GetTickCount();
	}
#endif

	//////////////////////////////////////
	//	0. INIT VARIABLE
	//////////////////////////////////////
	g_sBizFlowInfo.bProcessing = TRUE;
	g_sBizFlowInfo.nPrevFlowID = 0;
	g_sBizFlowInfo.nCurFlowID = 0;
	g_sBizFlowInfo.nNextFlowID = eFlowID;
	g_sBizFlowInfo.nReasonforCancel = RES_OK;

	//////////////////////////////////////
	//	1. BEFORE PROCESSING
	//////////////////////////////////////
	m_PreBalanceStatus = PRE_BAL_NONE;
	m_bStartTransaction = TRUE;
	m_bPrintImage = TRUE;

	// [#2035] NH KSK 2011.03.22 거래 모드시 RMS Connection을 끊지 않는 Bug Fix
	//m_pDevCmn->fnNET_RMSConnectClose();		// [#2365] US Justin 2015.07.27 Use Continue Fall back as default in CA

	// [#2205] US KSK 2013.06.28
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
	{
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_FLICKER_OPTION) == 1)	// ONLY TRANSACTION 인 경우에만 FLICKER ON
			m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_PIN, FLICKER_ON);

		// [#2333] NH KSK 2015.03.06 EPP LED GUIDE 제어
		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
		{
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_LED_GUIDE_OPTION) == 1)	// Only Idle인 경우 LED GUIDE OFF
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_OFF);			
			else if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_LED_GUIDE_OPTION) == 2)	// Only Transaction인 경우에만 LED GUIDE ON
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_CONTINUE);
		}
		// end of [#2333]
	}
	// [#2222] CA Justin 2013.10.03 Add HALO LED Control
	else
	{
		SetHaloLedControl(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_TRANSACTION), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_TRANSACTION), SKIP_MCULED);
	}
	// End of[#2222]
	// end of [#2205]

	//////////////////////////////////////
	//	2. PROCESSING
	//////////////////////////////////////

	while(g_sBizFlowInfo.bProcessing == TRUE)
	{
		//Default ADA language is "ENG."
		//g_sBizFlowInfo.nAdaTypeID= ADA_ENG_MODE;		//[#2077] [CA] PCS 2011.06.23

		g_sBizFlowInfo.nPrevFlowID = g_sBizFlowInfo.nCurFlowID;
		g_sBizFlowInfo.nCurFlowID = g_sBizFlowInfo.nNextFlowID;

		switch(g_sBizFlowInfo.nCurFlowID)
		{
		case FID_NH_NOR_CLEANUP:			eNextFlowID = F_NH_NOR_CleanUp();				break;

		// NORMAL TRANSACTION
		case FID_NH_NOR_COMMON:				eNextFlowID = F_CA_NOR_Common();				break;
		case FID_NH_NOR_SELECTTRAN:			eNextFlowID = F_CA_NOR_SelectTran();			break;
		case FID_NH_NOR_WITHDRAWAL:			eNextFlowID = F_CA_NOR_Withdrawal();			break;
		case FID_NH_NOR_PINCHANGE:			eNextFlowID = F_NH_NOR_PinChange();				break;
		case FID_NH_NOR_REVERSAL:			eNextFlowID = F_NH_NOR_Reversal();				break;
		case FID_NH_NOR_RECEIPT:			eNextFlowID = F_NH_NOR_Receipt();				break;
		case FID_NH_NOR_CANCEL:				eNextFlowID = F_NH_NOR_Cancel();				break;
		case FID_NH_NOR_EXIT:				eNextFlowID = F_NH_NOR_Exit();					break;

		// ADA TRANSACTION
		case FID_NH_ADA_COMMON:				eNextFlowID = F_CA_ADA_Common();				break;

		// [#2375] US Justin Disable ADA functions.
		// [#2077] CA PCS 2011.06.23		ADA에서는 FID_NH_NOR_SELECTTRAN이 없고 출금 거래만 존재한다.
		//case FID_NH_ADA_WITHDRAWAL:		eNextFlowID = F_CA_ADA_Withdrawal();			break;
		//case FID_NH_ADA_REVERSAL:			eNextFlowID = F_NH_ADA_Reversal();				break;
		//case FID_NH_ADA_RECEIPT:			eNextFlowID = F_NH_ADA_Receipt();				break;
		//case FID_NH_ADA_CANCEL:			eNextFlowID = F_NH_ADA_Cancel();				break;
		//case FID_NH_ADA_EXIT:				eNextFlowID = F_NH_ADA_Exit();					break;
		//END [#2077]
		// End of [#2375]

		// [RWC6-162] Add B4U to Canada Release	
		#if(APP_B4U)
		// B4U
		case FID_NH_B4U_REQ_INFO_FLOWS:		eNextFlowID = F_NH_B4U_RequestInfo();	break;
		case FID_NH_B4U_QR_FLOWS:			eNextFlowID = F_NH_B4U_QR();			break;
		case FID_NH_B4U_FAIL_FLOWS:			eNextFlowID = F_NH_B4U_Failed();		break;
		case FID_NH_B4U_CANCEL_FLOWS:		eNextFlowID = F_NH_B4U_Cancelled();		break;
		case FID_NH_B4U_TIMEOUT_FLOWS:		eNextFlowID = F_NH_B4U_Timeout();		break;
		case FID_NH_B4U_AUTH_FLOWS:			eNextFlowID = F_NH_B4U_Authorization();	break;
		case FID_NH_B4U_SUCCESS_FLOWS:		eNextFlowID = F_NH_B4U_Success();		break;
		case FID_NH_B4U_FINALIZE_FLOWS:		eNextFlowID = F_NH_B4U_Finalize();		break;
		#endif
		// end of [RWC6-162]

		case FID_END_OF_FLOW:
		default:
			NHDEBUG(DBG_INFO, (L"[CTranCmn::FLOW_Mani_Proc] Stop Processing !! nPrevFlowID(%d) nCurFlowID(%d) nNextFlowID(%d)\n", 
				g_sBizFlowInfo.nPrevFlowID, g_sBizFlowInfo.nCurFlowID, g_sBizFlowInfo.nNextFlowID));
			g_sBizFlowInfo.bProcessing = FALSE;
			break;

		}

		g_sBizFlowInfo.nNextFlowID = eNextFlowID;
	}

	// [#2205] US KSK 2013.06.28
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
	{
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_FLICKER_OPTION) == 1)
			m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_PIN, FLICKER_OFF);

		// [#2333] NH KSK 2015.03.06 EPP LED GUIDE 제어
		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
		{
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_LED_GUIDE_OPTION) == 2)	// Only Transaction인 경우에만 LED GUIDE ON
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_OFF);
		}
		// end of [#2333]
	}
	// end of [#2205]

	m_bStartTransaction = FALSE;


	if ((eFlowID == FID_NH_ADA_COMMON) || 
		(eFlowID == FID_NH_NOR_COMMON) ||
		(eFlowID == FID_NH_CCW_FLOWS))
	{
		m_nTransactionCount++;
	}

#ifdef MEM_LEAK_CHECK
	if ((eFlowID == FID_NH_ADA_COMMON) || 
		(eFlowID == FID_NH_NOR_COMMON) ||
		(eFlowID == FID_NH_CCW_FLOWS))
	{
		MEMORYSTATUS memStatus;	
		memStatus.dwLength = sizeof(MEMORYSTATUS);
		GlobalMemoryStatus(&memStatus);

		dwAfterPA = memStatus.dwAvailPhys;
		dwAfterVA = memStatus.dwAvailVirtual;
		dwAfterTime = GetTickCount();


		HANDLE hFile = CreateFile (L"\\ATM\\MEM_HIS.DAT",				// Open NVRAM.TXT.
									GENERIC_WRITE|GENERIC_READ,          // Open for writing
									0,                      // Do not share
									NULL,                   // No security
									OPEN_ALWAYS,          // Open or create
									FILE_ATTRIBUTE_NORMAL,  // Normal file
									NULL);                  // No template fil
		if (hFile != NULL)
		{
			CString strTemp2;

			SYSTEMTIME	ti;
			GetLocalTime(&ti);

			strTemp2.Format(L"[%02d/%02d %02d:%02d:%02d] COUNT(%4ld) TIME(%10d) | PA_A(%10ld) VA_A(%10ld)\n", 
								ti.wMonth, ti.wDay, ti.wHour, ti.wMinute, ti.wSecond,
								m_nTransactionCount, (dwAfterTime-dwBeforeTime), dwAfterPA, dwAfterVA);

			SetFilePointer(hFile, 0, NULL, FILE_END);

			int		nWriteByte = 0;
			char	arTempBuf[512];

			if (WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)strTemp2, -1, 
									(LPSTR)&arTempBuf[0], 512,
									NULL, NULL) != 0)
			{
				DWORD dwWritten;
				WriteFile(hFile, arTempBuf, strTemp2.GetLength(), &dwWritten, NULL);
			}

			CloseHandle(hFile);
			RETAILMSG(1, (L"%s", strTemp2));
			m_pDevCmn->fnSCR_SetDisplayData(L"APDebugMsg", strTemp2);
		}		
	}
#endif
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_CA_NOR_Common()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_CA_NOR_Common()
{
	NVDump('O', 'C', "00", L"F_CANOR", L"Common");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_CA_NOR_Common]\n"));

#ifdef UNDER_CE
	// [#2561] NH Justin 2018.07.11 Enable Camera Features on CA and MX Mode APs
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE) == ENABLE)
		m_pDevCmn->fnCAM_Initialize();
	// end of [#2561]
#endif // UNDER_CE

	BIZ_RETURN nRes = RES_INIT;

	// nRes : RES_EMV_MS_FIRST, RES_EMV_IC_FIRST, RES_NOR_MS_TRANS
	nRes = P_EMV_CheckTransMode();

	if (nRes != RES_EMV_IC_FIRST)
	{
		m_pDevCmn->fnNET_RMSConnectClose();							// [#2365] US Justin 2015.07.27 

		// nRes : RES_OK, RES_CARD_ERROR, RES_CARD_MOD_10
		if (P_NH_NOR_ReadCard() != RES_OK)
			return FID_NH_NOR_CANCEL;

		m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);
	}
	
	while(1)
	{
		if (nRes == RES_NOR_MS_TRANS)
		{
			NHDEBUG(DBG_CALL, (L"[CTranCmn::F_CA_NOR_Common] [MS NORMAL TRANSACTION]\n"));

			// [#2150] US Justin 2012.09.27 Add Dynamic Flow
			// Call every Transaction including a consequent transaction after PREBALANCE.. the transaction might be different.
			if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)&&
				(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE) == ENABLE) )
				P_NH_NOR_DynamicFlowConnection(TC_DYNAMICFLOWL_1ST, _T("001"));
			// End of [#2150]

			// [#2472] US Justin 2017.02.07 Dcc Local withdrawal Amount Option Check
			CheckLocalDCCEligibility();
			// End of [#2472]

			// [#2375] US Justin 2015.11.05 Combine ADA on Normal Transactin Flow
			if( (IsAdaTransaction())&&(m_pDevCmn->fnSNS_GetEnhancedAudio()==FALSE) )
				break;
			// End of [#2375]

			// condition DIALUP & PREDIAL ENABLE
			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE) == ENABLE &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION) == 0)
				m_pDevCmn->fnNET_PreDialStart();

			// 2. Select Language (Screen Mode Only)
			// [#2375] US Justin 2015.10.27 Combine ADA on Normal Transactin Flow
			//if ((nRes = P_NH_NOR_SelectLanguage()) != RES_OK)			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			//	break;
			if( IsAdaTransaction() )
			{
				if(m_pDevCmn->fnSNS_GetEnhancedAudio()==FALSE)
					break;
			}
			else
			{
				if ((nRes = P_NH_NOR_SelectLanguage()) != RES_OK)		// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
					break;
			}
			// End of [#2375]


			// 1. Select Language

			// 3. Enter Password
			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT 
			if ((nRes = P_NH_NOR_EnterPassword()) != RES_OK)
				break;

			// 4. Cardtronics TDL Surcharge Segmentation
			// [#2365] US Justin 2015.07.24 Add TDL Surcharge Segmentation
			if ( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE) && 
				 (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_TIRSURCHARGE) == ENABLE) && 
				 (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DCC) != ENABLE ) )
			{
				if ( (nRes = P_NH_NOR_TDL_Connection(TC_TDL_TIRSURCHARGE)) != RES_OK )
				{
					nRes = RES_USER_EXIT;
					break;
				}
			}
			// End of [#2365]

			// condition DIALUP & PREDIAL ENABLE
			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE) == ENABLE &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION) == 1)
				m_pDevCmn->fnNET_PreDialStart();

			return FID_NH_NOR_SELECTTRAN;
		}
		else if (nRes == RES_EMV_IC_FIRST)
		{
			NHDEBUG(DBG_CALL, (L"[CTranCmn::F_CA_NOR_Common] [IC FIRST TRANSACTION]\n"));
			//nRes : RES_MAKE_AID, RES_EMV_FALLBACK, RES_EMV_TERMINATE 
			nRes = P_EMV_ICCardRead();

			// [#2365] US Justin 2015.07.27 Implement Continue Fallback on CA
			// RMS Listen Port will be closed in "ContinueFallBack Function" for Fallback Case  => EMV transaction case Connection Close will be called twice (Not an issue)
			if (nRes != RES_EMV_FALLBACK)		
				m_pDevCmn->fnNET_RMSConnectClose();		
			// End of [#2365]
		}
		else if (nRes == RES_MAKE_AID)
		{
			// nRes : RES_MULTI_AID_SELECT, RES_MAKE_AID, RES_NOR_MS_TRANS, RES_EMV_TERMINATE, RES_EMV_FALLBACK
			nRes = P_EMV_MakeAIDList();
		}
		else if (nRes == RES_MULTI_AID_SELECT)
		{
			// nRes : RES_MAKE_AID, RES_NOR_MS_TRANS, RES_EMV_TERMINATE, RES_EMV_FALLBACK, RES_USER_EXIT
			nRes = P_EMV_SelectMultiAIDList();
		}
		else if (nRes == RES_EMV_FALLBACK)
		{
			// [#2365] CA Justin 2015.07.27 Use Continue Fallback
			// 처리 후 Cancel 한다.
			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			/*
			nRes = P_EMV_FallBackProc();
			if (nRes != RES_OK)
				break;
			*/

			nRes = P_EMV_ContinueFallBackProc();

			m_pDevCmn->fnNET_RMSConnectClose();				// [#J008] US Justin 2018.12.12 Bug Fix : RMS Connection for Fallback Transaction

			if (nRes != RES_OK)
			{
				//m_pDevCmn->fnNET_RMSConnectClose();		// [#J008] US Justin 2018.12.12 Bug Fix : RMS Connection for Fallback Transaction
				break;
			}
			// End of [#2365]

			// 1. Read Card
			// nRes : RES_OK, RES_CARD_ERROR, RES_CARD_MOD_10
			if (P_NH_NOR_ReadCard() != RES_OK)
				break;

			m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);
			
			nRes = RES_NOR_MS_TRANS;
		}
		else
		{
			// Break하는 조건 
			// 1: EMV_TERMINATE인 경우
			// 2: RES_USER_EXIT인 경우
			// 3: RES_USER_TIMEOUT인 경우
			break;
		}

		Delay_Msg(50);
	}

	if (nRes == RES_USER_EXIT || nRes == RES_USER_TIMEOUT)
		P_EMV_RemoveCard();
	else
		P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));

	g_sBizFlowInfo.nReasonforCancel = nRes;

	return FID_NH_NOR_CANCEL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_CA_NOR_SelectTran()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_CA_NOR_SelectTran()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_CA_NOR_SelectTran]\n"));

	// 1. Select Transaction
	P_CA_NOR_SelectTransaction();

	return FID_NH_NOR_WITHDRAWAL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_CA_NOR_Withdrawal()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_CA_NOR_Withdrawal()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"Withdrawal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_CA_NOR_Withdrawal]\n"));

	BIZ_RETURN	nRes;

	BOOL bIsDenominationSelect = FALSE;		// [#RWC6-12] Denomination Selection Support

	while (1)
	{
		// 2. Receive Receipt
		if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
		{
			if ((nRes = P_NH_NOR_ReceiveReceipt()) != RES_OK)
				break;
		}
		else
		{
			if ((nRes = P_NH_NOR_ErrorReceipt()) != RES_OK)		
				break;
		}

		// 3. Select Account
		if ((nRes = P_NH_NOR_SelectCWAccount()) != RES_OK)
			break;

		// condition DIALUP & PREDIAL ENABLE
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE) == ENABLE &&
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION) == 2)
			m_pDevCmn->fnNET_PreDialStart();

		// 4. Input Amount
		//if ((nRes = P_NH_NOR_InputCWAmount()) != RES_OK)
		//	break;
		// [#RWC6-12] Denomination Selection Support
		if ((nRes = P_NH_NOR_InputCWAmount()) == RES_DENOMINATION_SELECT) 
		{
			NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_InputCWAmount()] Result=[%d]\n", nRes));
			bIsDenominationSelect = TRUE;
			if ((nRes = P_NH_NOR_DenominationSelect()) != RES_OK)
			{
				NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_DenominationSelect()] Result=[%d]\n", nRes));
				break;
			}
		}
		else if (nRes != RES_OK)
		{
			NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_InputCWAmount()] Result=[%d]\n", nRes));
			break;
		}
		// end of [#RWC6-12]

		// 5. Make EMV Data
		if(m_pDevCmn->fnMCU_IsEmvTransaction())	
		{
			if (P_NH_NOR_EMV_IC_Processing() != RES_OK)
				break;
		}
		
		// 6. Display Surcharge
		if ((nRes = P_NH_NOR_CalculateAndShowWithDrawalSurcharge()) != RES_OK)							// [#2496] US Justin 2017.08.18 Make a Function for all countries						
			break;

		/*
		// [#2375] US Justin 2015.11.05 Combine ADA and Screen Transaction Flow
		BOOL bADAVGMode = FALSE;
		if( IsAdaTransaction() )
			bADAVGMode = TRUE;
		// End of [#2375]

		BOOL bIsDCC2ndCall = FALSE;			// DCC Inquiry Call or CW Dynamic Surchage Inquiry call?	// [#2182] NH Justin 2013.03.21 Implement NHD1.6
		BOOL bDFSurcharge = FALSE;			// Using a Surcharge which was downloaded by Dnyamic Flow.
		BOOL bTerminate = FALSE;														// [#2175] US Justin 2013.01.18 US EMV
		if( (m_sSTD1_DynamicFlowResp.nService_DCC )&&(MemGetInt( _MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC)==ENABLE))
		{
#ifdef APP_LOCAL_MODE
//[#2180] US Justin 2013.02.06 DCC Demo
			m_sSTD1_DynamicFlowResp.R2_ConnectionResult = ST_OK;
			#if (US_VERSION)
				m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency = _T("USD/2/MXN/2");
			#elif(CA_VERSION)
				m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency = _T("CAD/2/MXN/2");
			#endif	
			float fERate = 13.7271;
			m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate.Format(L"%0.2f", fERate);
			m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount.Format(L"%012d", (int)(fERate*(Asc2Int(m_sUserSelection.strMoney)+200)) );
			m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge.Format(L"%08d", (int)(fERate*200) );
			m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC = _T("00000200");
			bIsDCC2ndCall = TRUE;
// End of [#2180]
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
				// [#2182] NH Justin 2013.03.21 Implement NHD1.6
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

						// [#2330] US Justin 2015.02.10 Percentage DCC Surcharge
						P_NH_NOR_DFDCC_Convert2HomeCurrency();
						// End of [#2330]
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
				// End of [#2182]
			}
			// End of [#2242]
#endif
		}
		/////////////////////////////// Checking Dynamic Surcharge....NON DCC Cashwithdrawal.........
		else if( m_sSTD1_DynamicFlowResp.R1_ConnectionResult==ST_OK )
		{
			if( m_sSTD1_DynamicFlowResp.R1_Surcharge_Withdrawal.GetLength() == 8 )
			{
				if( m_sSTD1_DynamicFlowResp.R1_Surcharge_Withdrawal == L"        ")		// All Spaces on the 1st call => Need to call again.
					P_NH_NOR_DynamicFlowConnection(TC_DYNAMICFLOWL_2ND, _T("010"));
				else if( Asc2Int(m_sSTD1_DynamicFlowResp.R1_Surcharge_Withdrawal) > 0 )	// Surcharge from 1st Call
				{
					bDFSurcharge = TRUE;
					if ((nRes = P_NH_NOR_DisplaySurcharge(SURCHARGE_DISP_DYNAMIC1ST)) != RES_OK)		// Dynamic Surcharge..... fc of the first call.....
						bTerminate = TRUE;
				}
				else
					bDFSurcharge = TRUE;		// Surcharge = 0, Not showing....
			}
			// else => ATM did not get fc field from the first call (DCC disabled) => Normal Surcharge Flow (Existing....)
		}
		///////////////////////////////// Standard3 TDL DCC ////////////////////////////////////////
		// [#2365] US Justin 2015.07.24 TDL Surcharge Segmentation or DCC
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

					// [#2375] Justin 2015.11.05 Disable DCC for VG mode.... No Wave file available (English and French)
					///////////////////////////////////////////////////////////////////////////////
					///////////////////////////////////////////////////////////////////////////////
					if(bADAVGMode)
						m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCEligible = L"0";		// Set here otherwise ATM will skip "surcharge notice"
					///////////////////////////////////////////////////////////////////////////////
					///////////////////////////////////////////////////////////////////////////////
					// End of [#2375]

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
		// End of [#2365]

		// Check Standard1 2nd Dynamic Flow result....
		if( (bTerminate==FALSE)&&(m_sSTD1_DynamicFlowResp.R2_ConnectionResult==ST_OK) )
		{
			// Standard 1 Enhanced Suscharge Segmentation
			//if( m_sSTD1_DynamicFlowResp.R2_Surcharge_Withdrawal.GetLength() == 8 )		// Normal Withdrawal....(fc Exist) => Not a DCC transaction.
			if( (!bIsDCC2ndCall)&&(m_sSTD1_DynamicFlowResp.R2_Surcharge_Withdrawal.GetLength() == 8) )	// Normal Withdrawal : Get a response for "010" request	// [#2182] NH Justin 2013.03.21 Implement NHD1.6
			{
				bDFSurcharge = TRUE;
				if( Asc2Int(m_sSTD1_DynamicFlowResp.R2_Surcharge_Withdrawal) > 0 )		// Surcharge from 2nd Call
				{
					if ((nRes = P_NH_NOR_DisplaySurcharge(SURCHARGE_DISP_DYNAMIC2ND)) != RES_OK)		// Dynamic Surcharge..... fc of the second call.....
						bTerminate = TRUE;
				}
				// else => ZERO Surcharge ..  Not displaying
			}
			// Standard 1 Enhanced DCC
			//else if(	(MemGetInt( _MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC)==ENABLE)&&	
			else if(	(bIsDCC2ndCall)&&	// DCC : Get a response for "011" request	// [#2182] NH Justin 2013.03.21 Implement NHD1.6
			//			(m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC.GetLength() == 8)&&										// [#2182] NH Justin 2013.03.21 Implement NHD1.6
						(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount.GetLength() == 12)&&
						(m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.GetLength() == 11)&&
						(m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate.GetLength() > 0 )&&
						(Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount) > 0)			)	// DCC Available
			{
				// Checking Currency ID and converted Transaction Amount
				bDFSurcharge = TRUE;
				if( Asc2Int(m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC) > 0 )	
				{
					if ((nRes = P_NH_NOR_DisplaySurcharge(SURCHARGE_DISP_DYNAMICDCC)) != RES_OK)		// DCC FEE
						bTerminate = TRUE;
				}
				// else => ZERO Surcharge ..  Not displaying

				if( bTerminate==FALSE)
				{
					// [#2375] Justin 2015.11.05 Disable DCC for VG mode.... No Wave file available(English and French)
					//nRes = P_NH_NOR_DisplayDCCOption();			// Withdrawal Option (Normal WD / DCC WD ?)
					///////////////////////////////////////////////////////////////////////////////
					///////////////////////////////////////////////////////////////////////////////
					if(bADAVGMode)
						nRes = RES_DCC_USE_LOCALCURRENCY;			// ADA ==> LOCAL CURRENCY ONLY
					else
						nRes = P_NH_NOR_DisplayDCCOption();			// Withdrawal Option (Normal WD / DCC WD ?)
					///////////////////////////////////////////////////////////////////////////////
					///////////////////////////////////////////////////////////////////////////////
					// End of [#2375]

					if(nRes==RES_DCC_USE_LOCALCURRENCY)				// Withdrawal without DCC conversion
						m_sSTD1_DynamicFlowResp.nDCCTransaction = 2;								// [#2426] Justin Change DCC Type 0:non DCC, 1:DCC Tr, 2:DCC Denial
					else if(nRes==RES_DCC_USE_HOMECURRENCY)			// Withdrawal with DCC conversion
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

		// [#2365] US Justin 2015.07.25 Add TDL DCC
		if( (bTerminate==FALSE)&&(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE)&&(m_STD3_TDL_Data.m_bProceedDCC==TRUE) )
		{
			nRes = P_NH_NOR_DisplayDCCOption(DCC_CUSTOMOPTION_CARDTRONICS);			// Withdrawal Option (Normal WD / DCC WD ?)
			if(nRes==RES_DCC_USE_LOCALCURRENCY)										// Withdrawal without DCC conversion
				m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_OfferAcceptance = L"0"; 
			else if(nRes==RES_DCC_USE_HOMECURRENCY)									// Withdrawal with DCC conversion
				m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_OfferAcceptance = L"1";
			else
				bTerminate = TRUE;
		}
		// End of [#2365]

		if (bTerminate==TRUE)
		{
			if (P_EMV_CheckTransMode() == RES_EMV_IC_FIRST)
				P_EMV_RemoveCard();
			break;
		}
		// End of [#2173]

		// [#2365] US Justin 2015.07.25 Add DCC+ in Canada
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
		// End of [#2365]
		*/
		// End of [#2496]

		// 7. Transaction
		// [#2292] US Justin 2014.10.02 Add TDL DCC
		//if ((nRes = P_NH_NOR_Transaction(TRUE)) != RES_OK)
		if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE)&&(m_STD3_TDL_Data.m_bProceedDCC==TRUE) )
			nRes = P_NH_NOR_TDL_Connection(TC_TDL_DCC_TRANSACTION);
		else
			nRes = P_NH_NOR_Transaction(TRUE);

		if( nRes != RES_OK )
		// End of [#2292]
		{
			// JNL Save.
			switch (nRes)
			{
			case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
			case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
			case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
			}

			if (m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				// EMV 거래 중 Error인 경우
				// 카드 제거 후 Reversal 수행
				// [#2215] NH Justin 2013.08.05 Screen Message change for EMV Denial Transaction : CHIP ERROR => Transaction Error
				//P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
				//if( (nRes==RES_HOST_SEND_ERR)||(nRes==RES_HOST_RECV_ERR)||(nRes==RES_HOST_DENIED) )		// [#2365] US Justin 2015.07.24 Add DCC+ on Canadian AP
				if (m_pDevCmn->m_bDisplayDeclined == TRUE)
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127005));
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
				// End of [#2215]
			}
			return FID_NH_NOR_REVERSAL;
		}

		// 8. Complete EMV Transaction
		if (m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			nRes = P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127004));

			if (nRes == RES_USER_TIMEOUT)
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);

			if (nRes != RES_OK)
			{
				// 8. Save JNL and Update Statistics	Full reversal이더라도 거래 Count 증가 사양 (미국)
				P_NH_NOR_SaveCWStatus(FALSE);

				return FID_NH_NOR_REVERSAL;
			}
		}

		// 9. Cash Dispense
		if ((nRes = P_NH_NOR_CashDispense()) != RES_OK)
		{
			// 9. Tack Cash
			if (nRes == RES_DEV_CDU_ERR_PARTIAL)
				P_NH_NOR_TakeCash();

			// 8. Save JNL and Update Statistics	Full reversal이더라도 거래 Count 증가 사양 (미국)
			P_NH_NOR_SaveCWStatus(FALSE);

			return FID_NH_NOR_REVERSAL;
		}

		m_pDevCmn->fnAPL_SetProcCount('6');						// Dispense OK

		// 10. Save JNL and Update Statistics
		P_NH_NOR_SaveCWStatus(FALSE);

		// 11. Tack Cash
		P_NH_NOR_TakeCash();

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
		
		return FID_NH_NOR_REVERSAL;
	}
	// [#2282] NH Justin Add Missed "remove Card"
	//int nProcCount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT));
	//if ((nProcCount < 3) && (nRes == RES_USER_EXIT || nRes == RES_USER_TIMEOUT))	
	if(m_pDevCmn->fnMCU_IsEmvTransaction())
		P_EMV_RemoveCard();
	// End of[#2282]

	g_sBizFlowInfo.nReasonforCancel = nRes;
	return FID_NH_NOR_CANCEL;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: F_CA_ADA_Common()
RETURN TYPE  : Next Flow ID
PARAMETER    : -
DESCRIPTION  : CANADA ADA 실행시 가장 먼저 실행되는 FLOW.
-------------------------------------------------------------------*/
//[#2077] CA PCS 2011.06.23
FLOW_ID CTranCmn::F_CA_ADA_Common()
{
	NVDump('O', 'C', "00", L"F_CANOR", L"Common");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_CA_NOR_Common]\n"));

	BIZ_RETURN nRes = RES_INIT;

	//-------------------STATE1-----------------------------------------------------------------
	//ADA로 거래가 넘어오면 처음에 화면을 뿌려주게 된다.
	m_pDevCmn->fnSCR_DisplayPrevSet(004);				  // [#2077] CA PCS 2011. 06.26 언어선택 후 화면도 동일함.
	m_pDevCmn->fnSCR_DisplayScreen(004, 0, PIN_ALL_MODE); // [#2077] CA PCS 2011. 06.26 타임아웃을 0으로 주어서 AP에서 시간 통제						  
	m_bAdaTransaction = TRUE;

	m_pDevCmn->fnNET_RMSConnectClose();		// [#2365] US Justin 2015.07.27 Implement Continue Fallback 

	//[#2077] CA PCS 2011.06.27 
	// 1. Select Language
	// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
	if ((nRes = P_CA_ADA_Language()) != RES_OK)
		return FID_NH_ADA_EXIT;

	//2. Welcome Broadcasting
	if ((nRes = P_NH_ADA_Welcome()) != RES_OK)
		return FID_NH_ADA_EXIT;

	// [#2375] NH Justin 2015.11.05 Combine ADA and Screen Flow.
	return FID_NH_NOR_COMMON;
}//END [#2077]