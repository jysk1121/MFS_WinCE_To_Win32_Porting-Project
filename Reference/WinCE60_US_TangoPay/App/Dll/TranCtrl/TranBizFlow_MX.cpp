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
 FUNCTION NAME: FLOW_Main_Proc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Business Flow에 따른 분기를 수행한다.
-------------------------------------------------------------------*/
void CTranCmn::BIZ_MX_StartFlowProc(FLOW_ID eFlowID)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::FLOW_Main_Proc] eFlowID(%d)\n", (int)eFlowID));

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
	m_pDevCmn->fnNET_RMSConnectClose();

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_FLICKER_OPTION) == 1)	// ONLY TRANSACTION 인 경우에만 FLICKER ON
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_PIN, FLICKER_ON);

	//////////////////////////////////////
	//	2. PROCESSING
	//////////////////////////////////////

	while(g_sBizFlowInfo.bProcessing == TRUE)
	{
		// ADA not support for Mexico

		g_sBizFlowInfo.nPrevFlowID = g_sBizFlowInfo.nCurFlowID;
		g_sBizFlowInfo.nCurFlowID = g_sBizFlowInfo.nNextFlowID;

		switch(g_sBizFlowInfo.nCurFlowID)
		{
		case FID_NH_NOR_CLEANUP:			eNextFlowID = F_NH_NOR_CleanUp();				break;

		// NORMAL TRANSACTION
		case FID_MX_NOR_COMMON:				eNextFlowID = F_MX_NOR_Common();				break;
		case FID_NH_NOR_PREBALANCE:			eNextFlowID = F_NH_NOR_PreBalance();			break;
		case FID_NH_NOR_SELECTTRAN:			eNextFlowID = F_NH_NOR_SelectTran();			break;

		///////////////////////////////////////////////////////////////////////////////////////////
		// [#2137] MX PCS 2012.07.24 "거래 Flow 구분."
		case FID_MX_NOR_WITHDRAWAL:			eNextFlowID = F_MX_NOR_Withdrawal();			break;
		case FID_MX_NOR_INQUIRY:			eNextFlowID = F_MX_NOR_Inquiry();				break;
		case FID_NH_NOR_PINCHANGE:			eNextFlowID = F_NH_NOR_PinChange();				break;
		// end of [#2137]
		///////////////////////////////////////////////////////////////////////////////////////////

		case FID_NH_NOR_REVERSAL:			eNextFlowID = F_NH_NOR_Reversal();				break;
		case FID_NH_NOR_RECEIPT:			eNextFlowID = F_NH_NOR_Receipt();				break;
		case FID_NH_NOR_CANCEL:				eNextFlowID = F_NH_NOR_Cancel();				break;
		case FID_NH_NOR_EXIT:				eNextFlowID = F_NH_NOR_Exit();					break;

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

			strTemp2.Format(L"[%02d/%02d %02d:%02d:%02d] COUNT(%4ld) TIME(%10d) | PA_A(%10ld) PA_D(%10ld) VA_A(%10ld) VA_D(%10ld)\n", 
								ti.wMonth, ti.wDay, ti.wHour, ti.wMinute, ti.wSecond,
								m_nTransactionCount, (dwAfterTime-dwBeforeTime), dwAfterPA, (m_dwInitPA-dwAfterPA), dwAfterVA, (m_dwInitVA-dwAfterVA));

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
 FUNCTION NAME: F_MX_NOR_Common()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_MX_NOR_Common()
{
	NVDump('O', 'C', "00", L"F_MXNOR", L"Common");
	NHDEBUG(DBG_CALL, ( L"[CTranCmn::F_MX_NOR_Common]\n"));

#ifdef UNDER_CE
	// [#2561] NH Justin 2018.07.11 Enable Camera Features on CA and MX Mode APs
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE) == ENABLE)
		m_pDevCmn->fnCAM_Initialize();
	// end of [#2561]
#endif

	BIZ_RETURN nRes = RES_INIT;

	// nRes : RES_EMV_MS_FIRST, RES_EMV_IC_FIRST, RES_NOR_MS_TRANS
	nRes = P_EMV_CheckTransMode();

	if (nRes != RES_EMV_IC_FIRST)
	{
		if( m_PreBalanceStatus != PRE_BAL_ANOTHER )	// [#2265] Pre Balance Bug fix
		{
			// nRes : RES_OK, RES_CARD_ERROR, RES_CARD_MOD_10
			if (P_NH_NOR_ReadCard() != RES_OK)
				return FID_NH_NOR_CANCEL;
		}
		m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);
	}
	//[#2265] NH Justin 2014.05.06 Bug Fix, Continue transaction for FallBack Transaction
	else
	{
		if( m_PreBalanceStatus == PRE_BAL_ANOTHER )		// Previous Transaction was FallBack Transaction.
		{
			m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);
			nRes = RES_NOR_MS_TRANS;
		}
	}
	// End of [#2265]
	while(1)
	{
		if (nRes == RES_NOR_MS_TRANS)
		{
			NHDEBUG(DBG_CALL, (L"[CTranCmn::F_MX_NOR_Common] [MS NORMAL TRANSACTION]\n"));

			// 1. Dynamic Flow 1st Call if enabled
			if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_HYOSUNG_TYPE) && (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE)==ENABLE) )
			{
				P_NH_NOR_DynamicFlowConnection(TC_DYNAMICFLOWL_1ST, _T("001"));
			}

			// 2. Select Language
			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			if ((nRes = P_NH_NOR_SelectLanguage()) != RES_OK)
				break;

			// 3. Enter Password
			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT 
			if ((nRes = P_NH_NOR_EnterPassword()) != RES_OK)
				break;

			// [#2115] MX KSK 2012.01.28 명세표 인자 언어 선택을 위해 설정함
			nSavedLanguage = m_pDevCmn->fnSCR_GetCurrentLangMode();

// [#2291] NH Justin 2014.08.27 Add Cardtronics TDL Option Enable Balance Inquiry 
#if (APP_TDL_OPTION)	
			// Pre-Balance 기능은 EMV 거래가 아닌 경우에만 지원하도록 함. => Enable Balance AT first even it is EMV Transaction (divert to Welcome Screen at the end)
			//if ((m_PreBalanceStatus == PRE_BAL_NONE) && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PREBALANCE) == ENABLE) &&
			//	(m_pDevCmn->fnMCU_IsEmvTransaction() == FALSE))
			if (  (m_PreBalanceStatus == PRE_BAL_NONE) && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PREBALANCE) == ENABLE) ) // [#2265] NH Justin 2014.05.06 Enable Balance Inquiry even it is EMV Transaction
			{
				return FID_NH_NOR_PREBALANCE;
			}
			else
			{
				return FID_NH_NOR_SELECTTRAN;
			}
#else
			return FID_NH_NOR_SELECTTRAN;
#endif
// End of [#2291]
		}
		else if (nRes == RES_EMV_IC_FIRST)
		{
			NHDEBUG(DBG_CALL, (L"[CTranCmn::F_MX_NOR_Common] [IC FIRST TRANSACTION]\n"));
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
			NVDump('O', 'C', "00", L"F_MXNOR", L"FALLBACK");        // RWC6-656 01/03/2024
			// 처리 후 Cancel 한다.
			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			// nRes = P_EMV_FallBackProc();
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_ENABLE_DISABLE) == 1)
			    nRes = RES_USER_EXIT;                               // RWC6-656 01/03/2024
			else
			    nRes = P_EMV_ContinueFallBackProc();                // RWC6-656 01/03/2024
			//  nRes = P_EMV_FallBackProc();

			if (nRes != RES_OK)
				break;

			// 1. Read Card
			// nRes : RES_OK, RES_CARD_ERROR, RES_CARD_MOD_10
			if (P_NH_NOR_ReadCard() != RES_OK)
				break;

			m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);

			// [#2285] MX Justin 2014.07.09 Remove Damaged Card Restriction
			nRes = RES_NOR_MS_TRANS;
			/*
			// [#2159] MX Justin 2012.19 Check Service Code...., if service code is 2 or 6 (IC Card), ATM will cancel the transaction
			// nRes = RES_NOR_MS_TRANS;
			CString strTmp = m_sCardData.strISO2Data;
			int nResult = strTmp.Find(L"=");
			if( (nResult==-1)||(strTmp.GetLength() <= nResult + 5) )						// Not possible.. because it passed "P_NH_NOR_ReadCard" already....
			{
				nRes = RES_CARD_ERROR;
				break;
			}
			if (strTmp.GetAt(nResult+5) == '2' || strTmp.GetAt(nResult+5) == '6')			// if this is EMV card... => Cancel Transaction. MX SPECIAL REQUEST.
			{
				nRes = RES_DAMAGED_ICCARD;
				break;
			}			
			nRes = RES_NOR_MS_TRANS;
			// End of [#2159]
			*/
			// End of [#2285]
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
 FUNCTION NAME: F_MX_NOR_Withdrawal()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_MX_NOR_Withdrawal()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_MX_USD_Withdrawal]\n"));

	BIZ_RETURN	nRes;
	
	while (1)
	{
		// 1. Receive Receipt
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

		// 2. Select Account
		if ((nRes = P_NH_NOR_SelectCWAccount()) != RES_OK)
			break;

		// 3. Input Amount
		if ((nRes = P_MX_NOR_InputCWAmount()) != RES_OK)
			break;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 4. Make EMV Data
		if(m_pDevCmn->fnMCU_IsEmvTransaction())		// IC 거래가 아닐때에는 return
		{
			NHDEBUG(DBG_CALL, (L"[*****m_pDevCmn->fnMCU_IsEmvTransaction*****]\n"));
			// nRes : RES_OK, RES_EMV_FALLBACK, RES_EMV_TERMINATE, 
			nRes = P_EMV_ICProcessing();

			if (nRes == RES_EMV_FALLBACK)
			{
				// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			//	nRes = P_EMV_FallBackProc();
				// nRes = P_EMV_ContinueFallBackProc();    // RWC6-656  01/02/2024

			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_ENABLE_DISABLE) == 1)
			    nRes = RES_USER_EXIT;                   // RWC6-656 02/03/2024
			else
			    nRes = P_EMV_ContinueFallBackProc();    // RWC6-656  01/02/2024
		//	    nRes = P_EMV_FallBackProc();            // RWC6-656  01/02/2024

				if (nRes != RES_OK)
					break;
			}
			else if (nRes == RES_EMV_TERMINATE)
			{
				// IC Error시 MCU 매체가 없는 경우 IC ERROR 문구 표시 후 거래 취소 하도록 수정 (내부검사 지적사항 대응)
				if (m_pDevCmn->fnMCU_GetMaterialInfo() == 0)	// 매체가 없는 경우 정상 return
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003), FALSE, TRUE);
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));

				break;
			}
		}

		// [#2149] US Justin 2012.09.21 User Final Confirmation before Host communication
		// 5. User Final Confirmation
		if ((nRes = P_MX_NOR_CustomerFinalConfirmation()) != RES_OK)
			break;
		// End of [#2149]

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 5. 1번째 전문 송수신.
		// [#2137] MX KSK 2012.07.17 USD 방출 모드인 경우 First Call 수행
		NHDEBUG(DBG_CALL, (L"[*****1st Communication*****]\n"));

		m_sSTD3_TranResp.m_strEMVTag = L"";				// [#2291] MX Justin 2014.09.02 (for MultiCurrency, Domestic Card)

		// 5-1. Show Surcharge before transaction for DF DCC
		if ( ((MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_HYOSUNG_TYPE)
			  &&(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE)==ENABLE)) ||
			  (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE) )
		{
			if ((nRes = P_NH_NOR_CalculateAndShowWithDrawalSurcharge()) != RES_OK)
			{	
				if( m_pDevCmn->fnMCU_IsEmvTransaction() )
					P_EMV_RemoveCard();
				return FID_NH_NOR_CANCEL;
			}
		}

		if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
			MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG, MX_1ST_SEND);

		// 6. Transaction
		if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE)&&(m_STD3_TDL_Data.m_bProceedDCC==TRUE) )
			nRes = P_NH_NOR_TDL_Connection(TC_TDL_DCC_TRANSACTION);
		else
			nRes = P_NH_NOR_Transaction(TRUE);

 		if (nRes != RES_OK)
 		{
			// HOST 송/수신 후에 Card Type 설정을 해야함
			P_MX_NOR_CheckCustomer();

			switch (nRes)
			{
			case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
			case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
			case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
			}

			// 통신 장애시 처리
			/////////////////////////////////////////////////////////////////////////////
			// EMV의 경우 둘 다 카드 제거 후 Reversal FLOW에서 REVERSAL 송신 여부 판단하도록 분기처리함.
			if (m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				nRes = P_EMV_RemoveCard();

				if (nRes == RES_USER_TIMEOUT)
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);
			}

			return FID_NH_NOR_REVERSAL;
		}

		// reason for reversal 값 초기화.
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 0);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 6. qbG에 따른 거래 분기 처리.
		if (P_MX_NOR_CheckCustomer() != RES_OK)
		{
			/////////////////////////////////////////////////////////////////////////////
			// EMV의 경우 둘 다 카드 제거 후 Reversal FLOW에서 REVERSAL 송신 여부 판단하도록 분기처리함.
			if (m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				nRes = P_EMV_RemoveCard();

				if (nRes == RES_USER_TIMEOUT)
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);
			}

			return FID_NH_NOR_REVERSAL;
		}
		
		// EMV 거래인 경우 Card 제거
		// [#2180] MX Justin 2013.03.08 EMV Transaction
		/*
		if (m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			nRes = P_EMV_RemoveCard();
			if (nRes == RES_USER_TIMEOUT)
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);
		}
		*/
		// End of [#2180]

		if ( !(((MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_HYOSUNG_TYPE)
			&&(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE)==ENABLE)) ||
			  ((MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE)
			&&(m_STD3_TDL_Data.m_bProceedDCC==TRUE))) )
		{
			//[#2291] MX Justin 2014.09.02
			if( (m_pDevCmn->m_strCurrencyID==MULTI_CURRENCY_TYPE)&&(MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE)==MX_DOMESTIC_TYPE)&&(m_pDevCmn->fnMCU_IsEmvTransaction()) )
			{
				int	nEMVResult  = EMV_TR_SUCCESS;
				CString strTmp = m_sSTD3_TranResp.m_strEMVTag;

				if( strTmp.GetLength() <= 0)
				{
					if (m_bOnlineProcessing == FALSE)
					{
						m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);
						if (m_pDevCmn->fnEMV_Trans_OnlineProcess(1, AAC) == EMV_RSLT_OK)
						{
							m_pDevCmn->m_bDisplayDeclined = TRUE;
							m_pDevCmn->fnEMV_Trans_Completion();
						}
						BIZ_EMV_MakeICDataforReversal();
						m_bOnlineProcessing = TRUE;
					}
					nEMVResult = EMV_ERROR_ONLINE_DATA;
				}
				else
					nEMVResult = BIZ_EMV_Anal_ICDataforTrans(strTmp);
						
				if ( nEMVResult != EMV_TR_SUCCESS )
				{
					if (nEMVResult == EMV_ERROR_ONLINE_DATA)
						m_pDevCmn->fnAPL_StackError(L"9730100", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003087), DEV_MCU);
					else
						m_pDevCmn->fnAPL_StackError(L"9730200", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003087), DEV_MCU);

					/* Setting Reason for reversal : ICC DECLINED */
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 7);
					nRes = P_EMV_RemoveCard();
					return FID_NH_NOR_REVERSAL;
				}
			}
			// End of [#2291]

			// [#2266] US Justin 2014.05.07 Display Exchange Rate (USD Dispensing Mode, Domestic Card used)
			if( (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)&&(MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_DOMESTIC_TYPE) )
			{
				if( P_MX_NOR_ExchangeRateForDomesticCard() != RES_OK)
				{
					if( m_pDevCmn->fnMCU_IsEmvTransaction() )
						P_EMV_RemoveCard();
					return FID_NH_NOR_REVERSAL;
				}
			}
			// End of [#2266]

			// Surcharge Display
			if(P_MX_NOR_SurchargeFeeNotice() != RES_OK)
			{
				// [#2180] MX Justin 2013.03.08 EMV Transaction
				if( m_pDevCmn->fnMCU_IsEmvTransaction() )
					P_EMV_RemoveCard();
				// End of [#2180]
				return FID_NH_NOR_REVERSAL;
			}

			if ((MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_DOMESTIC_TYPE) || (m_pDevCmn->m_strCurrencyID == CURRENCY_TYPE))
			{
				// BankName and Fee Display
				// [#2180] MX Justin 2013.03.08 EMV Transaction
				//if (P_MX_NOR_fnAPP_BankFeeDisplay() != RES_OK)
				//	return FID_NH_NOR_REVERSAL;
				if (P_MX_NOR_fnAPP_BankFeeDisplay() != RES_OK)
				{
					if( m_pDevCmn->fnMCU_IsEmvTransaction() )
						P_EMV_RemoveCard();
					return FID_NH_NOR_REVERSAL;
				}

				if (m_pDevCmn->fnMCU_IsEmvTransaction())
				{
					nRes = P_EMV_RemoveCard();
					if (nRes == RES_USER_TIMEOUT)
						MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);

					if (nRes != RES_OK)
					{
						P_NH_NOR_SaveCWStatus(FALSE);
						return FID_NH_NOR_REVERSAL;
					}
				}
				// End of [#2180]
			}
			else
			{
				if (P_MX_NOR_ExchangeFeeNotice() != RES_OK)
				{
					// [#2180] MX Justin 2013.03.08 EMV Transaction
					if( m_pDevCmn->fnMCU_IsEmvTransaction() )
						nRes = P_EMV_RemoveCard();
					// End of [#2180]
					return FID_NH_NOR_REVERSAL;
				}

				MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG, MX_2ND_SEND);

				// 6. Transaction
				nRes = P_NH_NOR_Transaction(TRUE);

				if (nRes != RES_OK)
				{
					// JNL Save.
					switch (nRes)
					{
					case RES_HOST_SEND_ERR:
						m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);

						if (m_pDevCmn->fnMCU_IsEmvEnable())
							m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);

						break;
					case RES_HOST_RECV_ERR: 
						m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);

						if (m_pDevCmn->fnMCU_IsEmvEnable())
							m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);

						break;
					case RES_HOST_DENIED:
						m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);

						if (m_pDevCmn->fnMCU_IsEmvEnable())
							m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);

						break;
					}

					// [#2180] MX Justin 2013.03.08 EMV Transaction
					if( m_pDevCmn->fnMCU_IsEmvTransaction() )
						nRes = P_EMV_RemoveCard();
					// End of [#2180]

					////////////////////////////////////////////////////////////////////////////
					// 2��° ���� �ۼ��ſ����� ������ Reversal�� �����Ѵ�.
					return FID_NH_NOR_REVERSAL;
				}

				// [#2180] MX Justin 2013.03.08 EMV Transaction
				if (m_pDevCmn->fnMCU_IsEmvTransaction())
				{
					nRes = P_EMV_RemoveCard();

					if (nRes == RES_USER_TIMEOUT)
						MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);
					
					if (nRes != RES_OK)
					{
						P_NH_NOR_SaveCWStatus(FALSE);
						return FID_NH_NOR_REVERSAL;
					}
				}
				// End of [#2180]
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////////////
		// 12. Cash Dispense
		if ((nRes = P_NH_NOR_CashDispense()) != RES_OK)
		{
			// 12-1. Tack Cash
			if (nRes == RES_DEV_CDU_ERR_PARTIAL)
				P_NH_NOR_TakeCash();

			// 12-2. Save JNL and Update Statistics	Full reversal이더라도 거래 Count 증가 사양 (미국)
			P_NH_NOR_SaveCWStatus(FALSE);

			return FID_NH_NOR_REVERSAL;
		}

		m_pDevCmn->fnAPL_SetProcCount('6');						// Dispense OK

		///////////////////////////////////////////////////////////////////////////////////////////////
		// 13. Save JNL and Update Statistics
		P_NH_NOR_SaveCWStatus(FALSE);

		///////////////////////////////////////////////////////////////////////////////////////////////
		// 14. Tack Cash
		P_NH_NOR_TakeCash();

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
		
		return FID_NH_NOR_REVERSAL;
	}
	// [#2282] 2014.06.19 Add missed "Remove Card" Notice
	//int nProcCount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT));
	//if ((nProcCount < 3) && (nRes == RES_USER_EXIT || nRes == RES_USER_TIMEOUT))
	if(m_pDevCmn->fnMCU_IsEmvTransaction())
		P_EMV_RemoveCard();
	// [#2282]

	g_sBizFlowInfo.nReasonforCancel = nRes;
	return FID_NH_NOR_CANCEL;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: F_MX_NOR_Inquiry()
RETURN TYPE  : Next Flow ID
PARAMETER    : -
DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_MX_NOR_Inquiry()
{
	NVDump('O', 'C', "00", L"F_MXNOR", L"Inquiry");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_MX_NOR_Inquiry]\n"));

	BIZ_RETURN	nRes;

	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_INQUIRY;

	while (1)
	{
		// 1. Receive Receipt
		if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
		{
			//  [#2183] NH Justin 2013.04.03 Receipt Option (No Transaction result on Screen)
			//if ((nRes = P_NH_NOR_ReceiveReceipt()) != RES_OK)
			//	break;
			// [#2219] 2013.09.04 Justin 2013.04.25 Digital Receipt
			/*
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN) == RECEIPT_ON_SCREEN_ENABLE )
			{
				if ((nRes = P_NH_NOR_ReceiveReceipt()) != RES_OK)
					break;
			}
			else
				m_sUserSelection.bPrintReceipt = TRUE;
			*/
			if ((nRes = P_NH_NOR_ReceiveReceipt()) != RES_OK)
				break;
			// End of [#2219]
			// End of [#2183]
		}
		else
		{
			if ((nRes = P_NH_NOR_ErrorReceipt()) != RES_OK)		
				break;
		}

		// 2. Select Account
		if ((nRes = P_NH_NOR_SelectBIAccount()) != RES_OK)
			break;
		
		// 3. Make EMV Data
		if(m_pDevCmn->fnMCU_IsEmvTransaction())		// IC 거래가 아닐때에는 return
		{
			// nRes : RES_OK, RES_EMV_FALLBACK, RES_EMV_TERMINATE, 
			nRes = P_EMV_ICProcessing();

			if (nRes == RES_EMV_FALLBACK)
			{
				// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
				// nRes = P_EMV_FallBackProc();
			    // nRes = P_EMV_ContinueFallBackProc();    // RWC6-656  01/02/2024
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_ENABLE_DISABLE) == 1)
					nRes = RES_USER_EXIT;                               // RWC6-656  01/03/2024
				else
					nRes = P_EMV_ContinueFallBackProc();                // RWC6-656  01/02/2024
		//	    nRes = P_EMV_FallBackProc();                            // RWC6-656  01/02/2024


				if (nRes != RES_OK)
					break;
			}
			else if (nRes == RES_EMV_TERMINATE)
			{
				// IC Error시 MCU 매체가 없는 경우 IC ERROR 문구 표시 후 거래 취소 하도록 수정 (내부검사 지적사항 대응)
				if (m_pDevCmn->fnMCU_GetMaterialInfo() == 0)	// 매체가 없는 경우 정상 return
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003), FALSE, TRUE);
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));

				break;
			}
		}

		// [#2149] US Justin 2012.09.21 User Final Confirmation before Host communication
		// 4. User Final Confirmation
		if ((nRes = P_MX_NOR_CustomerFinalConfirmation()) != RES_OK)
			break;
		// End of [#2149]

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 5. 1번째 전문 송수신.
		// [#2137] MX KSK 2012.07.17 USD 방출 모드인 경우 First Call 수행
		NHDEBUG(DBG_CALL, (L"[*****1st Communication*****]\n"));

		m_sSTD3_TranResp.m_strEMVTag = L"";				// [#2291] MX Justin 2014.09.02 (for MultiCurrency, Domestic Card)

		if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
			MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG, MX_1ST_SEND);

		// 6. Transaction
		nRes = P_NH_NOR_Transaction(TRUE);

		if (nRes != RES_OK)
		{
			// HOST 송/수신 후에 Card Type 설정을 해야함
			P_MX_NOR_CheckCustomer();

			// 통신 장애시 처리
			/////////////////////////////////////////////////////////////////////////////
			// EMV의 경우 둘 다 카드 제거 후 Reversal FLOW에서 REVERSAL 송신 여부 판단하도록 분기처리함.
			if (m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				nRes = P_EMV_RemoveCard();

				if (nRes == RES_USER_TIMEOUT)
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);
			}

			return FID_NH_NOR_REVERSAL;
		}

		// reason for reversal 값 초기화.
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 0);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 6. qbG에 따른 거래 분기 처리.
		if (P_MX_NOR_CheckCustomer() != RES_OK)
		{
			/////////////////////////////////////////////////////////////////////////////
			// EMV의 경우 둘 다 카드 제거 후 Reversal FLOW에서 REVERSAL 송신 여부 판단하도록 분기처리함.
			if (m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				nRes = P_EMV_RemoveCard();

				if (nRes == RES_USER_TIMEOUT)
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);
			}

			return FID_NH_NOR_REVERSAL;
		}

		//[#2291] MX Justin 2014.09.02
		if( (m_pDevCmn->m_strCurrencyID==MULTI_CURRENCY_TYPE)&&(MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE)==MX_DOMESTIC_TYPE)&&(m_pDevCmn->fnMCU_IsEmvTransaction()) )
		{
			int	nEMVResult  = EMV_TR_SUCCESS;
			CString strTmp = m_sSTD3_TranResp.m_strEMVTag;

			if( strTmp.GetLength() <= 0)
			{
				if (m_bOnlineProcessing == FALSE)
				{
					m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);
					if (m_pDevCmn->fnEMV_Trans_OnlineProcess(1, AAC) == EMV_RSLT_OK)
					{
						m_pDevCmn->m_bDisplayDeclined = TRUE;
						m_pDevCmn->fnEMV_Trans_Completion();
					}
					BIZ_EMV_MakeICDataforReversal();
					m_bOnlineProcessing = TRUE;
				}
				nEMVResult = EMV_ERROR_ONLINE_DATA;
			}
			else
				nEMVResult = BIZ_EMV_Anal_ICDataforTrans(strTmp);
					
			if ( nEMVResult != EMV_TR_SUCCESS )
			{
				if (nEMVResult == EMV_ERROR_ONLINE_DATA)
					m_pDevCmn->fnAPL_StackError(L"9730100", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003087), DEV_MCU);
				else
					m_pDevCmn->fnAPL_StackError(L"9730200", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003087), DEV_MCU);

				/* Setting Reason for reversal : ICC DECLINED */
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 7);
				nRes = P_EMV_RemoveCard();
				return FID_NH_NOR_REVERSAL;
			}
		}
		// End of [#2291]

		// [#2180] MX Justin 2013.03.08 EMV Transaction
		/*
		// EMV 거래인 경우 Card 제거 
		if (m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			nRes = P_EMV_RemoveCard();

			if (nRes == RES_USER_TIMEOUT)
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);
		}
		*/
		// End of [#2180]

		// Surcharge Display
		if(P_MX_NOR_SurchargeFeeNotice() != RES_OK)
		{
			// [#2180] MX Justin 2013.03.08 EMV Transaction
			if( m_pDevCmn->fnMCU_IsEmvTransaction() )
				P_EMV_RemoveCard();
			// End of [#2180]
			return FID_NH_NOR_REVERSAL;			
		}

		// BankName and Fee Display
		if ((MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_DOMESTIC_TYPE) || m_pDevCmn->m_strCurrencyID == CURRENCY_TYPE)	// Domestic 또는 Peso 모드인 경우에 표시
		{
			// [#2180] MX Justin 2013.03.08 EMV Transaction
			//if (P_MX_NOR_fnAPP_BankFeeDisplay() != RES_OK)
			//	return FID_NH_NOR_REVERSAL;
			if (P_MX_NOR_fnAPP_BankFeeDisplay() != RES_OK)
			{
				if( m_pDevCmn->fnMCU_IsEmvTransaction() )
					P_EMV_RemoveCard();
				return FID_NH_NOR_REVERSAL;
			}

			if (m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				nRes = P_EMV_RemoveCard();
				if (nRes == RES_USER_TIMEOUT)
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);

				if (nRes != RES_OK)
				{
					P_NH_NOR_SaveCWStatus(FALSE);
					return FID_NH_NOR_REVERSAL;
				}
			}
			// End of [#2180]
		}
		else
		{
			MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG, MX_2ND_SEND);

			// 6. Transaction
			nRes = P_NH_NOR_Transaction(TRUE);

			if (nRes != RES_OK)
			{
				// JNL Save.
				switch (nRes)
				{
				case RES_HOST_SEND_ERR:
					m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);

					if (m_pDevCmn->fnMCU_IsEmvEnable())
						m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);

					break;
				case RES_HOST_RECV_ERR: 
					m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);

					if (m_pDevCmn->fnMCU_IsEmvEnable())
						m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);

					break;
				case RES_HOST_DENIED:
					m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);

					if (m_pDevCmn->fnMCU_IsEmvEnable())
						m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);

					break;
				}

				// [#2180] MX Justin 2013.03.08 EMV Transaction
				if( m_pDevCmn->fnMCU_IsEmvTransaction() )
					nRes = P_EMV_RemoveCard();
				// End of [#2180]

				////////////////////////////////////////////////////////////////////////////
				// 2번째 전문 송수신에서는 무조건 Reversal을 수행한다.
				return FID_NH_NOR_REVERSAL;
			}
			// [#2180] MX Justin 2013.03.08 EMV Transaction
			if (m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				nRes = P_EMV_RemoveCard();

				if (nRes == RES_USER_TIMEOUT)
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 11);
				
				if (nRes != RES_OK)
				{
					P_NH_NOR_SaveCWStatus(FALSE);
					return FID_NH_NOR_REVERSAL;
				}
			}
			// End of [#2180]
		}		
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		// 4. SAVE E-JNL
		m_pDevCmn->m_JNLMgr.Save(NORMAL_TRX);

		if (m_pDevCmn->fnMCU_IsEmvEnable())
			m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);

		// 5. Update Statistics
		m_pDevCmn->fnCDU_SumProc(SUM_OF_INQUIRY);
		m_pDevCmn->fnCDU_SumProc(SUM_OF_SURCHARGE);

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

		return FID_NH_NOR_REVERSAL;
	}
	// [#2282] NH Justin Add missed "Remove Card" notice
	if(m_pDevCmn->fnMCU_IsEmvTransaction())
		P_EMV_RemoveCard();
	// End of [#2282]

	g_sBizFlowInfo.nReasonforCancel = nRes;
	return FID_NH_NOR_CANCEL;
}