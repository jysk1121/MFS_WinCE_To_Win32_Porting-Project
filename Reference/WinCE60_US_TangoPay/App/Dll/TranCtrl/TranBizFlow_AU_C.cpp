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
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: FLOW_Mani_Proc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Business Flow에 따른 분기를 수행한다.
-------------------------------------------------------------------*/
void CTranCmn::BIZ_AU_C_StartFlowProc(FLOW_ID eFlowID, int nEventKind)
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
	m_sUserSelection.bScreenDisplay = FALSE;	//[#2000] 2010.10.17 SOOK 호주 사양 적용 (거래 FLOW )
	m_nEventKind = nEventKind;		// [#2325] NH KSK 2015.01.22

	// [#2035] NH KSK 2011.03.22 거래 모드시 RMS Connection을 끊지 않는 Bug Fix
	m_pDevCmn->fnNET_RMSConnectClose();

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_FLICKER_OPTION) == 1)	// ONLY TRANSACTION 인 경우에만 FLICKER ON
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_PIN, FLICKER_ON);

	//////////////////////////////////////
	//	2. PROCESSING
	//////////////////////////////////////

	while(g_sBizFlowInfo.bProcessing == TRUE)
	{
		g_sBizFlowInfo.nPrevFlowID = g_sBizFlowInfo.nCurFlowID;
		g_sBizFlowInfo.nCurFlowID = g_sBizFlowInfo.nNextFlowID;

		switch(g_sBizFlowInfo.nCurFlowID)
		{
		case FID_NH_NOR_CLEANUP:			eNextFlowID = F_NH_NOR_CleanUp();				break;

		// NORMAL TRANSACTION
		case FID_NH_NOR_COMMON:				eNextFlowID = F_AU_C_NOR_Common();				break;
		case FID_NH_NOR_SELECTTRAN:			eNextFlowID = F_AU_C_NOR_SelectTran();			break;
		case FID_NH_NOR_WITHDRAWAL:			eNextFlowID = F_AU_C_NOR_Withdrawal();			break;
		case FID_NH_NOR_INQUIRY:			eNextFlowID = F_AU_C_NOR_Inquiry();				break;
		case FID_NH_NOR_REVERSAL:			eNextFlowID = F_NH_NOR_Reversal();				break;
		case FID_NH_NOR_RECEIPT:			eNextFlowID = F_AU_C_NOR_Receipt();				break;
		case FID_NH_NOR_CANCEL:				eNextFlowID = F_NH_NOR_Cancel();				break;
		case FID_NH_NOR_EXIT:				eNextFlowID = F_AU_C_NOR_Exit();				break;

		// [#2047] AU KSK 2011.04.13
		case FID_NH_ADA_COMMON:				eNextFlowID = F_NH_ADA_Common();				break;

		// [#2375] US Justin 
		#if (AU_VERSION)
		case FID_NH_ADA_WITHDRAWAL:			eNextFlowID = F_AU_ADA_Withdrawal();			break;
		case FID_NH_ADA_INQUIRY:			eNextFlowID = F_AU_ADA_Inquiry();				break;
		case FID_NH_ADA_REVERSAL:			eNextFlowID = F_NH_ADA_Reversal();				break;
		case FID_NH_ADA_RECEIPT:			eNextFlowID = F_NH_ADA_Receipt();				break;
		case FID_NH_ADA_CANCEL:				eNextFlowID = F_NH_ADA_Cancel();				break;
		case FID_NH_ADA_EXIT:				eNextFlowID = F_NH_ADA_Exit();					break;
		#endif
		// end of [#2047]

		case FID_END_OF_FLOW:
		default:
			NHDEBUG(DBG_INFO, (L"[CTranCmn::FLOW_Mani_Proc] Stop Processing !! nPrevFlowID(%d) nCurFlowID(%d) nNextFlowID(%d)\n", 
								g_sBizFlowInfo.nPrevFlowID, g_sBizFlowInfo.nCurFlowID, g_sBizFlowInfo.nNextFlowID));
			g_sBizFlowInfo.bProcessing = FALSE;
			break;
		}

		g_sBizFlowInfo.nNextFlowID = eNextFlowID;
	}

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_FLICKER_OPTION) == 1)
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_PIN, FLICKER_OFF);

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
 FUNCTION NAME: F_AU_C_NOR_Common()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_AU_C_NOR_Common()
{
	NVDump('O', 'C', "00", L"F_AUNOR", L"Common");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_AU_NOR_Common]\n"));

	// [#2220] AU KMK 2013.09.11 AU EMV
	BIZ_RETURN	nRes = RES_INIT;

	// IC/MS 거래 유무를 판단한다. (RES_EMV_MS_FIRST, RES_EMV_IC_FIRST, RES_NOR_MS_TRANS)
	// RES_EMV_MS_FIRST는 미사용
	nRes = P_EMV_CheckTransMode();

	if (nRes != RES_EMV_IC_FIRST)
	{
		// nRes : RES_OK, RES_CARD_ERROR, RES_CARD_MOD_10
		if (P_NH_NOR_ReadCard() != RES_OK)
			return FID_NH_NOR_CANCEL;

		m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);
	}

	while(1)
	{
		if (nRes == RES_NOR_MS_TRANS)
		{
			// 1. Dynamid Flow 1st Call
			// [#2362] AU KSK 2015.07.20 Add Dynamic Flow		// Call every Transaction including a consequent transaction after PREBALANCE.. the transaction might be different.
			if( ( MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_HYOSUNG_TYPE)&&(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE)==ENABLE) )
			{
				if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
				{
					// TTS가 지원될때까지 DCC는 미지원
					P_NH_NOR_DynamicFlowConnection(TC_DYNAMICFLOWL_1ST, _T("001"));
				}
			}
			// End of [#2362]

			NHDEBUG(DBG_CALL, (L"[CTranCmn::F_CA_NOR_Common] [MS NORMAL TRANSACTION]\n"));

			// condition DIALUP & PREDIAL ENABLE (option 0 - AFTER CARD)
			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE) == ENABLE &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION) == 0)
				m_pDevCmn->fnNET_PreDialStart();


			// [#2434] AU Kook 2016.07.27 Support Multilanguage for Transaction.
			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			//#if (AU_A_VERSION)	// [#2468] AU KSK 2017.01.18
			if ((nRes = P_NH_NOR_SelectLanguage()) != RES_OK)
				break;
			//#endif
			// end of [#2434]

			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT 
			if ((nRes = P_AU_C_NOR_EnterPassword()) != RES_OK)
				break;

			// condition DIALUP & PREDIAL ENABLE (option 1 - AFTER PIN)
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
			// [#2324] NH KSK 2015.01.16 Fallback US continue fallback 화면 적용
			// 처리 후 Cancel 한다.
			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
//			nRes = P_EMV_FallBackProc();
			nRes = P_EMV_ContinueFallBackProc();
			// end of [#2324]

			if (nRes == RES_PROGRAM_ERROR)
				m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);	// KSK 2016.06.28 Medea 갱신을 위해 Card Disable 처리함

			if (nRes != RES_OK)
				break;

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
	// end of [#2220]

}
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_AU_C_NOR_SelectTran()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_AU_C_NOR_SelectTran()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"SelectTran");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_NOR_SelectTran]\n"));

	BIZ_RETURN	nRes;

	while(1)
	{
		// 1. Select Transaction
		if ((nRes = P_AU_C_NOR_SelectTransaction()) != RES_OK)
			break;

		// check Transaction Type
		if (TranCode == TC_WITHDRAWAL)
			return FID_NH_NOR_WITHDRAWAL;
		else if (TranCode == TC_INQUIRY)
			return FID_NH_NOR_INQUIRY;
//		else if (TranCode == TC_TRANSFER)
//			return FID_NH_NOR_TRANSFER;

		break;
	}

	// [#2220] AU KMK 2013.10.31 EMV 거래 중 거래선택 취소 시 카드 제거 처리
	if (m_pDevCmn->fnMCU_IsEmvTransaction())
		P_EMV_RemoveCard();
	// end of [#2220]

	g_sBizFlowInfo.nReasonforCancel = nRes;
	
	return FID_NH_NOR_CANCEL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_AU_C_NOR_Withdrawal()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_AU_C_NOR_Withdrawal()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"Withdrawal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Withdrawal]\n"));

	BIZ_RETURN	nRes;

	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_WITHDRAWAL;

	while (1)
	{
		// 1. Select Account
		if ((nRes = P_NH_NOR_SelectCWAccount()) != RES_OK)
			break;

		// condition DIALUP & PREDIAL ENABLE
		// PREDIAL #3 (AFTER ACCOUNT - option 2)		// [#2220]
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE) == ENABLE &&
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION) == 2)
			m_pDevCmn->fnNET_PreDialStart();

		// 2. Input Amount
		if ((nRes = P_AU_C_NOR_InputCWAmount()) != RES_OK)
			break;

		// 3. Receive Receipt
		if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
		{
			if ((nRes = P_NH_NOR_ReceiveReceipt()) != RES_OK)
				break;
		}
		else
		{
			// [#2220] AU KMK 2014.02.14 Error Receipt QR Code 적용
			if ((nRes = P_NH_NOR_ErrorReceipt()) != RES_OK)
				break;
			// end of [#2220]
		}

		// [#2220] AU KMK 2013.10.17 호주 EMV
		// 4. Make EMV Data
		if (m_pDevCmn->fnMCU_IsEmvTransaction() == TRUE)
		{
			// [#2362] AU KSK 2015.07.20 미국에서 생성한 P_NH_NOR_EMV_IC_Processing 함수로 대체
			if (P_NH_NOR_EMV_IC_Processing() != RES_OK)
				break;
			// end of [#2362]
		}
		// end of [#2220]

		// [#2362] AU KSK 2015.07.20 AU DCC Added
		BOOL bIsDCC2ndCall = FALSE;			// DCC Inquiry Call or CW Dynamic Surchage Inquiry call?	// [#2182] NH Justin 2013.03.21 Implement NHD1.6
		BOOL bDFSurcharge = FALSE;			// Using a Surcharge which was downloaded by Dnyamic Flow.
		BOOL bTerminate = FALSE;			// [#2175] US Justin 2013.01.18 US EMV
		int	 nWithdrawal_Surcharge = 0;		// [#2362] AU KSK 2015.07.20

		if( ( (m_sSTD1_DynamicFlowResp.nService_DCC )&&(MemGetInt( _MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC)==ENABLE) ) )
		{
#ifdef APP_LOCAL_MODE
			//[#2182] US Justin 2013.02.06 DCC Demo
			m_sSTD1_DynamicFlowResp.R2_ConnectionResult = ST_OK;
			m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency = _T("AUD/2/USD/2");
			float fERate = 1.20;
			m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate.Format(L"%0.2f", fERate);
			m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount.Format(L"%012d", (int)(fERate*(Asc2Int(m_sUserSelection.strMoney))) );
			m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge.Format(L"%08d", (int)(fERate*200) );
			m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC = _T("00000200");
			bIsDCC2ndCall = TRUE;
			// End of [#2182]
#else
			// [#2242] NH Justin 2013.12.17 DF 1st call option
			P_NH_NOR_DynamicFlowConnection(TC_DYNAMICFLOWL_2ND, _T("012"));

			if( (m_sSTD1_DynamicFlowResp.R2_ConnectionResult == ST_OK)&&
				(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount.GetLength() == 12)&&
				(m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.GetLength() == 11)&&
				(m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate.GetLength() > 0 )&&
				(Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount) > 0)	)
			{
				bIsDCC2ndCall = TRUE;
			}
#endif
		}

		// Check Standard1 2nd Dynamic Flow result....
		if( (bTerminate==FALSE) && (m_sSTD1_DynamicFlowResp.R2_ConnectionResult == ST_OK) )	// [#2175] US Justin 2013.01.18 US EMV
		{
			// Standard 1 Enhanced Surcharge Segmentation
			if( (!bIsDCC2ndCall)&&(m_sSTD1_DynamicFlowResp.R2_Surcharge_Withdrawal.GetLength() == 8) )	// Normal Withdrawal : Get a response for "010" request	// [#2182] NH Justin 2013.03.21 Implement NHD1.6
			{
				bDFSurcharge = TRUE;

				nWithdrawal_Surcharge = Asc2Int(m_sSTD1_DynamicFlowResp.R2_Surcharge_Withdrawal);

				// 수수료 부과시는 HOST에서 수신받은 출금 수수료 적용
				if (nWithdrawal_Surcharge > 0)
				{
					if ((nRes = P_AU_C_NOR_DisplaySurcharge(nWithdrawal_Surcharge)) != RES_OK)
						bTerminate = TRUE;
				}
				// else => ZERO Surcharge ..  Not displaying
			}
			// Standard 1 Enhanced DCC
			else if(	(bIsDCC2ndCall)&&	// DCC : Get a response for "011" request	// [#2182] NH Justin 2013.03.21 Implement NHD1.6 // [#2185] US Justin 2013.06.05 Dual Host Bug Fix
				(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount.GetLength() == 12)&&
				(m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.GetLength() == 11)&&
				(m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate.GetLength() > 0 )&&
				(Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount) > 0)			)	// DCC Available
			{
				// Checking Currency ID and converted Transaction Amount
				bDFSurcharge = TRUE;

				// DCC를 수신 받는 경우에는 무조건 DCC Surcharge 적용
				nWithdrawal_Surcharge = Asc2Int(m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC);
				if (nWithdrawal_Surcharge > 0)
				{
					// 수수료 부과시는 HOST에서 수신받은 출금 수수료 적용
					if ((nRes = P_AU_C_NOR_DisplaySurcharge(nWithdrawal_Surcharge)) != RES_OK)
						bTerminate = TRUE;
				}
				// else => ZERO Surcharge ..  Not displaying

				if( bTerminate==FALSE)																// [#2175] US Justin 2013.01.18 US EMV
				{
					nRes = P_NH_NOR_DisplayDCCOption();			// Withdrawal Option (Normal WD / DCC WD ?)
					if(nRes==RES_DCC_USE_LOCALCURRENCY)			// Withdrawal without DCC conversion
						m_sSTD1_DynamicFlowResp.nDCCTransaction = 0;
					else if(nRes==RES_DCC_USE_HOMECURRENCY)		// Withdrawal with DCC conversion
						m_sSTD1_DynamicFlowResp.nDCCTransaction = 1;
					else
						bTerminate = TRUE;															// [#2175] US Justin 2013.01.18 US EMV
				}
			}
			// else  => Normal Surcharge Flow (Existing....)
		}

		// Dynamic Surcharge was not presented... USE Existing Surcharge....
		if( (bTerminate==FALSE)&&(bDFSurcharge==FALSE) )											// [#2175] US Justin 2013.01.18 US EMV
		{
			if (LIB_CheckSurchargeDisplayWithMultipleBin() == TRUE)
			{
				if ((nRes = P_AU_C_NOR_DisplaySurcharge()) != RES_OK)
					bTerminate = TRUE;
			}
		}

		// [#2175] US Justin 2013.01.18 US EMV
		if (bTerminate==TRUE)
		{
			if (P_EMV_CheckTransMode() == RES_EMV_IC_FIRST)
				P_EMV_RemoveCard();
			break;
		}
		// End of [#2175]
		// End of [#2150]

		// [#2292] US Justin 2014.10.07 Save DCC Journal Info
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

		NHDEBUG(DBG_CALL, (L"Other Message for DCC = [%s]\n", strTemp2));
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strTemp2);		
		// End of [#2292]

		// 6. Transaction
		if ((nRes = P_NH_NOR_Transaction(TRUE)) != RES_OK)
		{
			// JNL Save.
			switch (nRes)
			{
			case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
			case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
			case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
			}

			// [#2462] AU KSK 2016.12.16 HOST DECLINED시 REVERSAL을 수행하지 않으므로 IC DATA 저장이 안됨 (보완처리)
			if (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) == REVERSAL_NONE)
			{
				if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
					m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
			}
			// end of [#2462]

			// [#2200] AU KMK 2013.10.18 EMV 거래 중 Error 시 Reversal 전 카드 제거
			if (m_pDevCmn->fnMCU_IsEmvTransaction() == TRUE)	
				P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
			// end of [#2220]

			return FID_NH_NOR_REVERSAL;
		}

		// [#2200] AU KMK 2013.10.17 호주 EMV. EMV 거래 시 출금 전 Card Remove 수행
		if (m_pDevCmn->fnMCU_IsEmvTransaction() == TRUE)
		{
			nRes = P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127004));

			if (nRes == RES_USER_TIMEOUT)
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);

			if (nRes != RES_OK)
				return FID_NH_NOR_REVERSAL;
		}
		// end of [#2220]

		// 6. Cash Dispense
		if ((nRes = P_AU_C_NOR_CashDispense()) != RES_OK)
		{
			// 7. Take Cash
			if (nRes == RES_DEV_CDU_ERR_PARTIAL)
			{
				P_NH_NOR_TakeCash();
				// Partial 일 경우 거래 Count만 증가시킴 
				m_pDevCmn->fnCDU_SumProc(SUM_OF_WITHDRAWAL);					// Sum Procedure(ATM)
			}

			return FID_NH_NOR_REVERSAL;
		}

		m_pDevCmn->fnAPL_SetProcCount('6');						// Dispense OK

		// 8. Save JNL and Update Statistics
		P_NH_NOR_SaveCWStatus(FALSE);

		// 9. Tack Cash
		P_NH_NOR_TakeCash();

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
		
		return FID_NH_NOR_REVERSAL;
	}

	// [#2220] AU KMK 2013.10.18 호주 EMV
	int nProcCount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT));
	if ((nProcCount < 3) && (nRes == RES_USER_EXIT || nRes == RES_USER_TIMEOUT))
		P_EMV_RemoveCard();
	// end of [#2220]


	g_sBizFlowInfo.nReasonforCancel = nRes;
	return FID_NH_NOR_CANCEL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_AU_C_NOR_Inquiry()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_AU_C_NOR_Inquiry()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"Inquiry");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Inquiry]\n"));
	
	BIZ_RETURN	nRes;

	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_INQUIRY;

	while (1)
	{

		// 1. Select Account
		if ((nRes = P_NH_NOR_SelectBIAccount()) != RES_OK)
			break;
	
		// condition DIALUP & PREDIAL ENABLE
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE) == ENABLE &&
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION) == 2)
			m_pDevCmn->fnNET_PreDialStart();

		// 2. Receive Receipt
		if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
		{
			if ((nRes = P_NH_NOR_ReceiveReceipt()) != RES_OK)
				break;
		}
		//2.1 명세표 안 받는다고 선택하거나 명세표 에러일 경우 화면에 출력하겠다 확인 함 
		// [#2220] AU KMK 2014.02.14 Error Receipt QR Code 적용
		if ((m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL) && (m_sUserSelection.nPrintReceipt == FALSE))
		{
			if ((nRes = P_AU_C_NOR_ErrorReceipt()) != RES_OK)	// P_AU_C_NOR_ErrorReceipt 함수는 현재 Balance details on .. 문구 표시에 사용
				break;
		}
		else if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
		{
			if ((nRes = P_NH_NOR_ErrorReceipt()) != RES_OK)
				break;
		}
		// end of [#2220]

		// 3. Make EMV Data
		if (m_pDevCmn->fnMCU_IsEmvTransaction() == TRUE)
		{
			nRes = P_EMV_ICProcessing();

			if (nRes == RES_EMV_FALLBACK)
			{
				nRes = P_EMV_FallBackProc();

				if (nRes != RES_OK)
					break;
			}
			else if (nRes == RES_EMV_TERMINATE)
			{
				if (m_pDevCmn->fnMCU_GetMaterialInfo() == 0)
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003), FALSE, TRUE);
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));

				break;
			}
		}
		// end of [#2220]		

		// 4. Display Surcharge
		if (LIB_CheckSurchargeDisplayWithMultipleBin() == TRUE)
		{
			if ((nRes = P_AU_C_NOR_DisplaySurcharge()) != RES_OK)
			{
				// [#2200] AU KMK 2013.10.17 호주 EMV. IC 거래 중 예외 발생 시 카드 제거 표시
				if (m_pDevCmn->fnMCU_IsEmvTransaction() == TRUE)
					P_EMV_RemoveCard();
				// end of [#2200]

				break;
			}
		}

		// 5. Transaction
		if ((nRes = P_NH_NOR_Transaction(TRUE)) != RES_OK)
		{
			// JNL Save.
			switch (nRes)
			{
			case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
			case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
			case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
			}

			// [#2462] AU KSK 2016.12.16 HOST DECLINED시 REVERSAL을 수행하지 않으므로 IC DATA 저장이 안됨 (보완처리)
			if (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) == REVERSAL_NONE)
			{
				if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
					m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
			}
			// end of [#2462]

			// [#2200] AU KMK 2013.10.18 EMV 거래 중 Error 시 Reversal 전 카드 제거
			if (m_pDevCmn->fnMCU_IsEmvTransaction() == TRUE)
				P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
			// end of [#2220]

			return FID_NH_NOR_REVERSAL;
		}

		// 6. Complete EMV Transaction
		if (m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			// [#2362] AU KSK 2015.08.11 카드 미 제거시 Reversal 하도록 로직 수정
			nRes = P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127004));

			if (nRes == RES_USER_TIMEOUT)
			{
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);
				return FID_NH_NOR_REVERSAL;
			}
			// end of [#2362]
		}

		// 7. SAVE E-JNL
		m_pDevCmn->m_JNLMgr.Save(NORMAL_TRX);

		// [#2353] AU KSK 2015.06.28 EMV DATA 저장 로직 추가
		if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
			m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
		// end of [#2353]
	
		// 8. Update Statistics 
		// 조회의 경우 명세표 출력 완료가 거래 정상 종료이므로 여기 Sum을 하면 안됨 
//		m_pDevCmn->fnCDU_SumProc(SUM_OF_INQUIRY);	
//		m_pDevCmn->fnCDU_SumProc(SUM_OF_SURCHARGE);	

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE); // 우선 여기서 Reversal Flag를 꺼야 리버설 안 함 

		return FID_NH_NOR_REVERSAL;
	}

	// [#2220] AU KMK 2013.10.18 호주 EMV
	int nProcCount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT));
	if ((nProcCount < 3) && (nRes == RES_USER_EXIT || nRes == RES_USER_TIMEOUT))
		P_EMV_RemoveCard();
	// end of [#2220]

	g_sBizFlowInfo.nReasonforCancel = nRes;
	return FID_NH_NOR_CANCEL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_AU_C_NOR_Receipt()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_AU_C_NOR_Receipt()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"Receipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_AU_C_NOR_Receipt] nPrintReceipt(%d)\n", m_sUserSelection.nPrintReceipt));		// [#2219] 2013.09.04 Justin Digital Receipt, Changing Variable Name

	BIZ_RETURN	nRes = RES_NG;

	BOOL	bResersal = FALSE;
	// 출금이면서 HOST 장애 또는 HOST 정상이나 CDU 장애일 경우 명세표 출력한다. 
	//if (LIB_IsReceiptPrintCondition4AU() == TRUE)
	if (LIB_IsReceiptPrintCondition() == TRUE)		//[#2183] NH Justin 2013.04.03 Change Function Name,
	{
		m_sUserSelection.nPrintReceipt = TRUE;		// [#2219] 2013.09.04 Justin Digital Receipt, Changing Variable Name			
		
		if ((nRes = P_NH_NOR_PrintReceipt()) == RES_OK)
			P_NH_NOR_TakeReceipt();
		else			
		{
			// 조회거래 시 명세표 장애인 경우 Reversal 송신한다. 
			if ( (TranCode == TC_INQUIRY) && (m_pDevCmn->TranResult == TRUE))
			{
				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
				// 명세표 Print 실패로 인해 Reversal시 전문 사양에 Reason Define 안되어 있으므로 현재 "0"로 송부함
				F_NH_NOR_Reversal();
				bResersal = TRUE;
			}
		}
	}
	// [#2235] 2013.11.29 AU KMK Digital Receipt
	else if (m_sUserSelection.nPrintReceipt == RCPT_QRCODE)
	{
		nRes = P_NH_NOR_DisplayQRCode();
	}
	// end of [#2235]

	// 사용자가 화면에 출력하겠다고 선택한 경우만 화면에 출력함 
	if ((m_sUserSelection.nPrintReceipt == FALSE) && (m_sUserSelection.bScreenDisplay == TRUE))			// [#2219] 2013.09.04 Justin Digital Receipt, Changing Variable Name
		P_NH_NOR_DisplayOnReceipt();

	// 조회 거래 일 경우 명세표 출력 완료해야만 거래 Count와 Surcharge Sum 한다. 
	if ( (TranCode == TC_INQUIRY) && (m_pDevCmn->TranResult == TRUE) && (bResersal == FALSE))
	{
		m_pDevCmn->fnCDU_SumProc(SUM_OF_INQUIRY);	
		m_pDevCmn->fnCDU_SumProc(SUM_OF_SURCHARGE);	

	}
	MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

	return FID_NH_NOR_EXIT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_AU_C_NOR_Exit()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_AU_C_NOR_Exit()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Exit]\n"));

	P_AU_C_NOR_DisplayExit();

	return FID_NH_NOR_CLEANUP;
}

// [#2375] US Justin 2015.10.28... Specify country... TO AVOID CONFUSION
#if (AU_VERSION)
// [#2047] AU KSK 2011.04.13
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: F_AU_ADA_Withdrawal()
RETURN TYPE  : Next Flow ID
PARAMETER    : -
DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID	CTranCmn::F_AU_ADA_Withdrawal()
{
	NVDump('O', 'C', "00", L"F_AUADA", L"Withdrawal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_AU_Withdrawal]\n"));

	BIZ_RETURN	nRes;

	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_WITHDRAWAL;

	while (1)
	{
		// 1. Select Account
		if ((nRes = P_NH_ADA_SelectCWAccount()) != RES_OK)
			break;

		// 2. Input Amount
		if ((nRes = P_NH_ADA_InputCWAmount()) != RES_OK)
			break;

		// 3. Receive Receipt
		if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
		{
			if ((nRes = P_NH_ADA_ReceiveReceipt()) != RES_OK)
				break;
		}
		else
		{
			if ((nRes = P_AU_ADA_ErrorReceipt()) != RES_OK)		
				break;
		}

		// [#2220] AU KMK 2013.10.22 호주 EMV
		// 4. Make EMV Data
		if(m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			nRes = P_EMV_ICProcessing();

			if (nRes == RES_EMV_FALLBACK)
			{
				if ( (nRes = P_EMV_ADA_FallBackProc()) != RES_OK)
					break;
			}
			else if (nRes == RES_EMV_TERMINATE)
			{
				if (m_pDevCmn->fnMCU_GetMaterialInfo() == 0)	// 매체가 없는 경우 정상 return
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003), FALSE, TRUE);
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));

				break;
			}
		}
		// end of [#2220]

		// 5. Display Surcharge
		if (LIB_CheckSurchargeDisplayWithMultipleBin() == TRUE )
		{
			if ((nRes = P_AU_ADA_DisplaySurcharge()) != RES_OK)
			{
				// [#2200] AU KMK 2013.10.17 호주 EMV. IC 거래 중 예외 발생 시 카드 제거 표시
				if (m_pDevCmn->fnMCU_IsEmvTransaction() == TRUE)
					P_EMV_RemoveCard();
				// end of [#2200]

				break;
			}
			
		}

		// 6. Transaction
		if ((nRes = P_NH_ADA_Transaction()) != RES_OK)
		{
			// JNL Save.
			switch (nRes)
			{
			case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
			case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
			case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
			}

			// [#2200] AU KMK 2013.10.18 EMV 거래 중 Error 시 Reversal 전 카드 제거
			if (m_pDevCmn->fnMCU_IsEmvTransaction() == TRUE)	
				P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
			// end of [#2220]

			return FID_NH_ADA_REVERSAL;
		}

		// [#2200] AU KMK 2013.10.17 호주 EMV. EMV 거래 시 출금 전 Card Remove 수행
		// 7. Remove IC Card
		if (m_pDevCmn->fnMCU_IsEmvTransaction() == TRUE)
		{
			nRes = P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127004));

			if (nRes == RES_USER_TIMEOUT)
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);

			if (nRes != RES_OK)
				return FID_NH_NOR_REVERSAL;
		}
		// end of [#2220]

		// 8. Cash Dispense
		if ((nRes = P_NH_ADA_CashDispense()) != RES_OK)
		{
			// 9. Take Cash
			if (nRes == RES_DEV_CDU_ERR_PARTIAL)
				P_NH_ADA_TakeCash();

			// 10. Save JNL and Update Statistics
			P_NH_NOR_SaveCWStatus(FALSE);

			return FID_NH_ADA_REVERSAL;
		}

		m_pDevCmn->fnAPL_SetProcCount('6');						// Dispense OK

		// 9. Save JNL and Update Statistics
		P_NH_NOR_SaveCWStatus(FALSE);

		// 10. Take Cash
		P_NH_ADA_TakeCash();

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

		return FID_NH_ADA_REVERSAL;
	}

	// [#2220] AU KMK 2013.10.18 호주 EMV
	int nProcCount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT));
	if ((nProcCount < 3) && (nRes == RES_USER_EXIT || nRes == RES_USER_TIMEOUT))
		P_EMV_RemoveCard();
	// end of [#2220]

	g_sBizFlowInfo.nReasonforCancel = nRes;
	return FID_NH_ADA_CANCEL;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: F_AU_ADA_Inquiry()
RETURN TYPE  : Next Flow ID
PARAMETER    : -
DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID	CTranCmn::F_AU_ADA_Inquiry()
{
	NVDump('O', 'C', "00", L"F_AUADA", L"Inquiry");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_AU_ADA_Inquiry]\n"));

	BIZ_RETURN	nRes;

	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_INQUIRY;

	while (1)
	{
		// 1. Select Account
		if ((nRes = P_NH_ADA_SelectBIAccount()) != RES_OK)
			break;

		// 2. Receive Receipt
		if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
		{
			if ((nRes = P_NH_ADA_ReceiveReceipt()) != RES_OK)
				break;
		}
		else
		{
			if ((nRes = P_AU_ADA_ErrorReceipt()) != RES_OK)		
				break;
		}

		// [#2220] AU KMK 2013.10.22 호주 EMV
		// 3. Make EMV Data
		if(m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			nRes = P_EMV_ICProcessing();

			if (nRes == RES_EMV_FALLBACK)
			{
				if ( (nRes = P_EMV_ADA_FallBackProc()) != RES_OK)
					break;
			}
			else if (nRes == RES_EMV_TERMINATE)
			{
				if (m_pDevCmn->fnMCU_GetMaterialInfo() == 0)	// 매체가 없는 경우 정상 return
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003), FALSE, TRUE);
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));

				break;
			}
		}
		// end of [#2220]

		// 4. Display Surcharge
		if (LIB_CheckSurchargeDisplayWithMultipleBin() == TRUE )
		{
			if ((nRes = P_AU_ADA_DisplaySurcharge()) != RES_OK)
			{
				// [#2200] AU KMK 2013.10.17 호주 EMV. IC 거래 중 예외 발생 시 카드 제거 표시
				if (m_pDevCmn->fnMCU_IsEmvTransaction() == TRUE)
					P_EMV_RemoveCard();
				// end of [#2200]

				break;
			}
		}

		// 5. Transaction
		if ((nRes = P_NH_ADA_Transaction()) != RES_OK)
		{
			// JNL Save.
			switch (nRes)
			{
			case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
			case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
			case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
			}

			// [#2200] AU KMK 2013.10.18 EMV 거래 중 Error 시 Reversal 전 카드 제거
			if (m_pDevCmn->fnMCU_IsEmvTransaction() == TRUE)	
				P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
			// end of [#2220]

			return FID_NH_ADA_REVERSAL;
		}

		// [#2200] AU KMK 2013.10.17 호주 EMV. EMV 거래 시 출금 전 Card Remove 수행
		// 6. Remove IC Card
		if (m_pDevCmn->fnMCU_IsEmvTransaction() == TRUE)
		{
			nRes = P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127004));

			if (nRes == RES_USER_TIMEOUT)
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);

			if (nRes != RES_OK)
				return FID_NH_NOR_REVERSAL;
		}
		// end of [#2220]

		// 7. SAVE E-JNL
		m_pDevCmn->m_JNLMgr.Save(NORMAL_TRX);

		// [#2353] AU KSK 2015.06.28 EMV DATA 저장 로직 추가
		if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
			m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
		// end of [#2353]

		// 8. Update Statistics
		m_pDevCmn->fnCDU_SumProc(SUM_OF_INQUIRY);

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

		return FID_NH_ADA_REVERSAL;
	}

	// [#2220] AU KMK 2013.10.18 호주 EMV
	int nProcCount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT));
	if ((nProcCount < 3) && (nRes == RES_USER_EXIT || nRes == RES_USER_TIMEOUT))
		P_EMV_RemoveCard();
	// end of [#2220]

	g_sBizFlowInfo.nReasonforCancel = nRes;
	return FID_NH_ADA_CANCEL;
}
// end of [#2047]
#endif