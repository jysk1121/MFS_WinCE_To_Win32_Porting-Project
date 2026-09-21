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
void CTranCmn::BIZ_StartFlowProc(FLOW_ID eFlowID, int nEventKind)	// [#2325] NH KSK 2015.01.22
{
	NHDEBUG(DBG_CALL, (L"eFlowID(%d), nEventKind(%d)\n", (int)eFlowID, nEventKind));

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
	m_nEventKind = nEventKind;	// [#2325] NH KSK 2015.01.22

	// [#2035] NH KSK 2011.03.22 거래 모드시 RMS Connection을 끊지 않는 Bug Fix
	// m_pDevCmn->fnNET_RMSConnectClose();			// [#2305] US Justin 2014.11.13 "DISABLE INITIAL CLOSING" --- Move RMSClose to reduce Unlatch Delay. 

	// [#2205] US KSK 2013.06.28
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
	{
		// 거래 중 Halo Led Setting
		SetHaloLedControl(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_TRANSACTION), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_TRANSACTION), SKIP_MCULED);
	}
	// [#GLDV-2890] US Kook 2021.05.21 Support VB Flickers on MX5400
	else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX5400")
	{
		// turn off EPP flicker (=> turn on CDU VB flicker only)
		SetGuideLight(0x00, CDU_VB_FLICKER);		
	}
	// end of [#GLDV-2890]
	else
	{
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_FLICKER_OPTION) == 1)	// ONLY TRANSACTION 인 경우에만 FLICKER ON
			m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_PIN, FLICKER_ON);

		// [#2333] NH KSK 2015.03.06 EPP LED GUIDE 제어
		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
		{
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_LED_GUIDE_OPTION) == 1)	// Only Idle인 경우EPP LED Guide Off 처리
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_OFF);
			else if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_LED_GUIDE_OPTION) == 2)	// Only Transaction인 경우에만 LED GUIDE ON
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_CONTINUE);
		}
		// end of [#2333]
	}
	// end of [#2205]
	

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
		case FID_NH_NOR_COMMON:				eNextFlowID = F_NH_NOR_Common();				break;
		case FID_NH_NOR_PREBALANCE:			eNextFlowID = F_NH_NOR_PreBalance();			break;
		case FID_NH_NOR_SELECTTRAN:			eNextFlowID = F_NH_NOR_SelectTran();			break;
		case FID_NH_NOR_WITHDRAWAL:			eNextFlowID = F_NH_NOR_Withdrawal();			break;
		case FID_NH_NOR_INQUIRY:			eNextFlowID = F_NH_NOR_Inquiry();				break;
		case FID_NH_NOR_TRANSFER:			eNextFlowID = F_NH_NOR_Transfer();				break;
		case FID_NH_NOR_PINCHANGE:			eNextFlowID = F_NH_NOR_PinChange();				break;		// [#2150] US Justin 2012.10.04 Dynamic Flow ... Add PIN Change
		case FID_NH_NOR_REVERSAL:			eNextFlowID = F_NH_NOR_Reversal();				break;
		case FID_NH_NOR_RECEIPT:			eNextFlowID = F_NH_NOR_Receipt();				break;
		case FID_NH_NOR_CANCEL:				eNextFlowID = F_NH_NOR_Cancel();				break;
		case FID_NH_NOR_EXIT:				eNextFlowID = F_NH_NOR_Exit();					break;
		
		// ADA	
		case FID_NH_ADA_COMMON:				eNextFlowID = F_NH_ADA_Common();				break;		

		// [#2375] US Justin Disable Merged ADA functions.
		//case FID_NH_ADA_WITHDRAWAL:		eNextFlowID = F_NH_ADA_Withdrawal();			break;
		//case FID_NH_ADA_INQUIRY:			eNextFlowID = F_NH_ADA_Inquiry();				break;
		//case FID_NH_ADA_TRANSFER:			eNextFlowID = F_NH_ADA_Transfer();				break;
		//case FID_NH_ADA_REVERSAL:			eNextFlowID = F_NH_ADA_Reversal();				break;
		//case FID_NH_ADA_RECEIPT:			eNextFlowID = F_NH_ADA_Receipt();				break;
		//case FID_NH_ADA_CANCEL:			eNextFlowID = F_NH_ADA_Cancel();				break;
		//case FID_NH_ADA_EXIT:				eNextFlowID = F_NH_ADA_Exit();					break;
		// End of [#2375]

		case FID_NH_NOR_READCARD:			eNextFlowID = F_NH_NOR_ReadCardOrRFID();		break;		// [#2351] US Justin
		case FID_NH_CARDLESS_FLOWS:			eNextFlowID = F_NH_CardlessTransaction();		break;		// [#2350] US Justin 2016.02.16
		case FID_NH_BITCOIN_SVC_FLOWS:		eNextFlowID = F_NH_BitcoinTransaction();		break;		// [#RWC6-68] PAI Bitcoin button

		// [#2471] US Justin 2017.02.01 Enable Popmoney and Pin4 to all Customers
		/*
		#if (APP_CUSTOM_PAI)
		case FID_NH_POPMONEY_FLOWS:			eNextFlowID = F_NH_POPMoney();					break;		// [#2350] US Justin 2015.07.17
		case FID_NH_PIN4_FLOWS:				eNextFlowID = F_NH_Pin4Transaction();			break;		// [#2396] US Justin 2016.02.16
		#endif
		*/
		#if (APP_PIN4_CASHPICKUP)
		case FID_NH_PIN4_FLOWS:				eNextFlowID = F_NH_Pin4Transaction();			break;		// [#2396] US Justin 2016.02.16
		#endif

		#if (APP_POPMONEY)
		case FID_NH_POPMONEY_FLOWS:			eNextFlowID = F_NH_POPMoney();					break;		// [#2350] US Justin 2015.07.17
		#endif
		// End of [#2471]

#if (APP_JUST_CASH)
		case FID_NH_JUSTCASH_START:					eNextFlowID = F_NH_JustCashStartFlow();					break;	// [#2445] US Justin 2016.09.22
		case FID_NH_JUSTCASH_GREET:					eNextFlowID = F_NH_JustCashGreetFlow();					break;
		case FID_NH_JUSTCASH_TRANSACTION:
		{
			if (_jcTransactionState.PaymentSelection == JCP_DEBIT)
			{
				eNextFlowID = F_NH_JustCashTransactionFlowAtm();
			}
			else if (_jcTransactionState.PaymentSelection == JCP_CASH)
			{
				eNextFlowID = F_NH_JustCashTransactionFlowSidecar();
			}
			else
			{
				eNextFlowID = F_NH_JustCashCleanupFlow();
			}

			break;
		}
		case FID_NH_JUSTCASH_CARDLESS_TRANSACTION:	eNextFlowID = F_NH_JustCashCardlessTransactionFlow();	break;
		case FID_NH_JUSTCASH_DEBIT:					eNextFlowID = F_NH_JustCashDebitFlow();					break;
		case FID_NH_JUSTCASH_ACCEPT_CASH:			eNextFlowID = F_NH_JustCashAcceptCashFlow(); 			break;
		case FID_NH_JUSTCASH_END_TRANSACTION:		eNextFlowID = F_NH_JustCashEndTransactionFlow();		break;
		case FID_NH_JUSTCASH_CANCEL_TRANSACTION:	eNextFlowID = F_NH_JustCashCancelTransactionFlow();		break;
		case FID_NH_JUSTCASH_CLEANUP:				eNextFlowID = F_NH_JustCashCleanupFlow();				break;
#endif

		#if (APP_PAYDIANT_CCA)
		case FID_NH_PAYPAL_FLOWS:			eNextFlowID = F_NH_PaypalCCATransaction();		break;		// [#2446] US Justin 2016.09.22
		#endif

		// [#RWC6-59] US William 2019.10.09 LibertyX
		#if(APP_LIBERTYX)
		case FID_NH_LIBERTYX_INIT:			eNextFlowID = F_NH_LibertyXInit();					break;
		case FID_NH_LIBERTYX_PURCHASE:		eNextFlowID = F_NH_LibertyXPurchase();				break;
		case FID_NH_LIBERTYX_DISPENSE:		eNextFlowID = F_NH_LibertyXDispense();				break;
		#endif
		// End of [#RWC6-59]// [#RWC6-16] Bitload4U
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

#if (APP_DIGITALMINT)
		case FID_NH_DIGITALMINT_START:				eNextFlowID = F_NH_DigitalMintStartFlow(); break;
		case FID_NH_DIGITALMINT_GREET:				eNextFlowID = F_NH_DigitalMintGreetFlow(); break;
		case FID_NH_DIGITALMINT_TRANSACTION:
		{
			if (_dmTransactionState.FlowSelection == DMF_ATM)
			{
				eNextFlowID = F_NH_DigitalMintTransactionFlowAtm();
			}
			else if (_dmTransactionState.FlowSelection == DMF_Sidecar)
			{
				eNextFlowID = F_NH_DigitalMintTransactionFlowSidecar();
			}
			else
			{
				eNextFlowID = F_NH_DigitalMintCleanupFlow();
			}

			break;
		}
		case FID_NH_DIGITALMINT_DEBIT:				eNextFlowID = F_NH_DigitalMintDebitFlow(); break;
		case FID_NH_DIGITALMINT_ACCEPT_CASH:		eNextFlowID = F_NH_DigitalMintCashAcceptFlow(); break;
		case FID_NH_DIGITALMINT_END_TRANSACTION:	eNextFlowID = F_NH_DigitalMintEndTransactionFlow(); break;
		case FID_NH_DIGITALMINT_CANCEL_TRANSACTION: eNextFlowID = F_NH_DigitalMintCancelTransactionFlow(); break;
		case FID_NH_DIGITALMINT_CLEANUP:			eNextFlowID = F_NH_DigitalMintCleanupFlow(); break;
#endif

#if (APP_CUSTOM_CASHDEPOT)
		case FID_NH_CASHDEPOT_START:				eNextFlowID = F_NH_CashDepotStartFlow(); break;
		case FID_NH_CASHDEPOT_GREET:				eNextFlowID = F_NH_CashDepotGreetFlow(); break;
		case FID_NH_CASHDEPOT_TRANSACTION:			eNextFlowID = F_NH_CashDepotTransactionFlow(); break;
		case FID_NH_CASHDEPOT_DEBIT:				eNextFlowID = F_NH_CashDepotDebitFlow(); break;
		case FID_NH_CASHDEPOT_END_TRANSACTION:		eNextFlowID = F_NH_CashDepotEndTransactionFlow(); break;
		case FID_NH_CASHDEPOT_CANCEL_TRANSACTION:	eNextFlowID = F_NH_CashDepotCancelTransactionFlow(); break;
		case FID_NH_CASHDEPOT_CLEANUP:				eNextFlowID = F_NH_CashDepotCleanupFlow(); break;
#endif
		// End of [#]		case FID_END_OF_FLOW:

#if (APP_TANGOPAY)	// [RWC6-676] Start SKKim 2024.04.09
		case FID_NH_TANGOPAY_START:					eNextFlowID = F_NH_TangoPay_StartFlow();		break;
		case FID_NH_TANGOPAY_PRESTAGING:			eNextFlowID = F_NH_TangoPay_PreStaingFlow();	break;
		case FID_NH_TANGOPAY_ACCEPTCASH:			eNextFlowID = F_NH_TangoPay_AcceptCash();		break;
		case FID_NH_TANGOPAY_END:					eNextFlowID = F_NH_TangoPay_EndFlow();			break;

		case FID_NH_TANGOPAY_CANCEL:				eNextFlowID = F_NH_TangoPay_CancelFlow();		break;
		case FID_NH_TANGOPAY_CLEANUP:				eNextFlowID = F_NH_TangoPay_CleanUpFlow();		break;
#endif				// [RWC6-676] End SKKim 2024.04.09

		default:
			NHDEBUG(DBG_INFO, (L"[CTranCmn::FLOW_Mani_Proc] Stop Processing !! nPrevFlowID(%d) nCurFlowID(%d) nNextFlowID(%d)\n", 
								g_sBizFlowInfo.nPrevFlowID, g_sBizFlowInfo.nCurFlowID, g_sBizFlowInfo.nNextFlowID));
			g_sBizFlowInfo.bProcessing = FALSE;
			break;
		}

		g_sBizFlowInfo.nNextFlowID = eNextFlowID;
	}

	// [#GLDV-2890] US Kook 2021.05.21 Support VB Flickers on MX5400
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX5400")
	{
		// turn off EPP flicker (=> turn on CDU VB flicker only)
		SetGuideLight(0x00, CDU_VB_FLICKER);		
	}
	// [#2205] US KSK 2013.06.28
	else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
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
		(eFlowID == FID_NH_CCW_FLOWS))	// [#2011] NH KJW 2010.01.09
	{
		m_nTransactionCount++;
	}

#ifdef MEM_LEAK_CHECK
	if ((eFlowID == FID_NH_ADA_COMMON) || 
		(eFlowID == FID_NH_NOR_COMMON) ||
		(eFlowID == FID_NH_CCW_FLOWS))	// [#2011] NH KJW 2010.01.09
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
		if (hFile != INVALID_HANDLE_VALUE)	// [#2022] NH KSK 2011.02.22
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
 FUNCTION NAME: F_NH_NOR_Common()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_NOR_Common()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"Common");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Common]\n"));

	HexaDump(NULL, 0, 0, FALSE, FILE_LOG_TYPE, TRUE);	// [#2452] NH KSK 2016.11.10	File Initialize
														// Justin ... Initialize file (Delete Log file) whenever transaction is initiated.
#ifdef UNDER_CE
	// [#2518] US Kook 2018.01.10 Support Camera
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE) == ENABLE)
		m_pDevCmn->fnCAM_Initialize();
	// end of [#2518]
#endif // UNDER_CE

	//[#2175] US Justin 2013.01.18 US EMV
	BIZ_RETURN nRes = RES_INIT;

	// [#2325] NH KSK 2015.01.22 RFID 지원을 위해 Check Mode 변경
	// nRes : RES_EMV_MS_FIRST, RES_EMV_IC_FIRST, RES_NOR_MS_TRANS
	//	nRes = P_EMV_CheckTransMode();
	if (m_nEventKind == DEV_RFID)
	{
		m_pDevCmn->nKindOfMedia = MEDIA_MS;	
		nRes = RES_NOR_MS_TRANS;			// 강제로 MS 거래로 진행
	}
	else
	{
		nRes = P_EMV_CheckTransMode();
	}
	// end of [#2325]

	if (nRes != RES_EMV_IC_FIRST)	// Magnetic Stripe Transaction
	{
		m_pDevCmn->fnNET_RMSConnectClose();		// [#2305] US Justin 2014.11.13 "MS TRANSACTION" --- Move RMSClose Timing to reduce Unlatch Delay. MS Mode => Works the same way as before

		// [#2325] NH KSK 2015.01.22 RFID Device Disable 처리
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
		{
			m_pDevCmn->fnRFID_EntryDisable();
			m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
		}
		// end of [#2325]

		if( m_PreBalanceStatus != PRE_BAL_ANOTHER )		// Check whether "2nd(Proceeding) Transaction " after "Balance At First"
		{												// If is not 2nd (Proceeding) Transaction => Read Magnetic Stripe Data (1st Transaction)
			// [#2325] NH KSK 2015.01.22
			// nRes : RES_OK, RES_CARD_ERROR, RES_CARD_MOD_10, RES_USER_EXIT(ADA_JACK_OUT)
			if (P_NH_NOR_ReadCard(m_nEventKind) != RES_OK)
				return FID_NH_NOR_CANCEL;
			// end of [#2325]
		}
		m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);
	}
	//[#2265] NH Justin 2014.05.06 Bug Fix, Continue transaction for FallBack Transaction
	else							// EMV Transaction....
	{
		// PRE_BAL_NONE, PRE_BAL_START, PRE_BAL_ANOTHER
		if( m_PreBalanceStatus == PRE_BAL_ANOTHER )		// Previous Transaction was FallBack Transaction. (<======"EMV Enabled" and "Performed PreBalance")
		{
			if(! m_pDevCmn->fnMCU_IsEmvTransaction() )	// [#2391] US Justin 2016.01.26		Support Multi Transactions for one EMV Latch
				m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);

			nRes = RES_NOR_MS_TRANS;
			// [#2325] NH KSK 2015.01.22 Pre Balance Enable시 RFID Enable 검토 필요
		}
		// [#2351] US Justin 2015.06.30 IC Card Read => Disable RFID
		else
		{
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
			{
				m_pDevCmn->fnRFID_EntryDisable();
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
			}
		}
		// End of [#2351]
	}
	// End of [#2265]
	// End of [#2175]

	while(1)
	{
		// [#2175] US Justin 2013.01.18 US EMV
		if (nRes == RES_NOR_MS_TRANS)
		{
			//BIZ_EVENT_SendHost(_EVENTID_START_TRAN);												// [#2313] US Justin 2014.11.20 Notice Event

			NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NOR_Common] [MS NORMAL TRANSACTION]\n"));
			m_STD3_TDL_Data.m_bProceedDCC = FALSE;													// [#2292] US Justin 2014.10.02 Initialize Parameter
			m_strHolderName = GetCardHolderName();

			// 1. Dynamid Flow 1st Call
			// [#2150] US Justin 2012.09.27 Add Dynamic Flow		// Call every Transaction including a consequent transaction after PREBALANCE.. the transaction might be different.
			// [#2185] US Justin 2013.05.03 PAI Dual Host
			if( ( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_HYOSUNG_TYPE)&&(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE)==ENABLE) ) ||
				( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE)==ENABLE)&&(MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION)>=0) )  )
			{
				if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE)==ENABLE)&&(MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION)>=0) ) 
					m_HostConfig = HC_DUALHOST;

				if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC)==ENABLE &&
					MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE)==ENABLE &&
					LIB_CheckSurchargeDisplay() == FALSE )
				{
					MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEDISPLAY, _T("0"));
				}

				P_NH_NOR_DynamicFlowConnection(TC_DYNAMICFLOWL_1ST, _T("001"));

				// [#2375] US Justin 2015.10.27 Combine ADA on Normal Transactin Flow
				if( (IsAdaTransaction())&&(m_pDevCmn->fnSNS_GetEnhancedAudio()==FALSE) )
					break;
				// End of [#2375]

				if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE)==ENABLE)&&(MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION)>=0)&&
					(m_sSTD1_DynamicFlowResp.R1_ConnectionResult == ST_OK)&&
					(m_sSTD1_DynamicFlowResp.nService_DCC == 1)  )
				{
					m_HostConfig = HC_DUALHOST;
					MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_DUALHOSTTAN, ON);		 // For Power Failure Reversal.....

					// [#2499] US Justin 2017.08.22 Change Background only for PAI
					#if(APP_CUSTOM_PAI)
					NH_SCR_CONFIG eFrontConfig = m_pDevCmn->m_pConfig->GetScreenConfig(SCR_FRONT);
					CString strBackName;

					if (eFrontConfig.eEngine == SCR_ENG_FLASH_LITE)
						strBackName.Format(L"%s\\%d_%d\\Touch\\Back_DCC.jpg", SCREEN_PATH_BACKS, eFrontConfig.nWidth, eFrontConfig.nHeight);
					else
						strBackName.Format(L"%s\\%d_%d\\Function\\Back_DCC.jpg", SCREEN_PATH_BACKS, eFrontConfig.nWidth, eFrontConfig.nHeight);
					m_pDevCmn->fnSCR_SetDisplayData(L"APBackImage", strBackName);
					m_nBackScreenNumber = -1;
					#endif
					// End of [#2499]
				}
				else
					m_HostConfig = HC_ATM;
			}
			// End of [#2185]
			// End of [#2150]

			// [#2472] US Justin 2017.02.07 Dcc Local withdrawal Amount Option Check
			CheckLocalDCCEligibility();
			// End of [#2472]

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
				// [#2391] US Justin 2016.01.25 US Justin Cardtronics Additional Request
				/*
				if ((nRes = P_NH_NOR_SelectLanguage()) != RES_OK)			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
					break;
				*/
				if( m_PreBalanceStatus != PRE_BAL_ANOTHER )
				{
					if ((nRes = P_NH_NOR_SelectLanguage()) != RES_OK)			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
						break;
				}
				// End of [#2388]
			}
			// End of [#2375]

			// 3. Enter Password
			// [#2391] US Justin 2016.01.26 Support EMV Multi Transaction
			/*
			if ((nRes = P_NH_NOR_EnterPassword()) != RES_OK)			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT 
				break;
			*/
			BOOL bNeedEnterPIN = TRUE;
			// Cardtronics US EMV Continue Transaction => SKIP PIN : Requested by Cardtronics 1/26/16. 
			// BUT require PIN for Magnetic Stripe Transaction.
			// [#2395] US Justin 2016.02.10 Cardtronics Additional Change Request : PIN Entry for both EMV and Mag. Transaction
			//		   Leave the following lines just in case they want it again later.
			// [#2398] US Justin 2016.02.17 Skip PIN ENTRY FOR EMV Transaction
			// 2020.08.10 Add for PAI, too to prevent EPP -402 errors.
			#if( US_VERSION && ( APP_TDL_OPTION || APP_CUSTOM_PAI ) )
				if( (m_PreBalanceStatus == PRE_BAL_ANOTHER) && (m_pDevCmn->fnMCU_IsEmvTransaction()) )
					bNeedEnterPIN = FALSE;
			#endif
			// End of [#2398]
			// End of [#2395]

			if( bNeedEnterPIN )
			{
				m_sUserSelection.strPassword.Empty();					// PRE_BAL_ANOTHER => Password was not cleared
				if ((nRes = P_NH_NOR_EnterPassword()) != RES_OK)		// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT 
					break;
			}
			// End of [#2391]

			// 4. Cardtronics TDL Surcharge Segmentation
			// [#2292] US Justin 2014.09.25 Add TDL Surcharge Segmentation
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
			// End of [#2292]

			// condition DIALUP & PREDIAL ENABLE
			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE) == ENABLE &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION) == 1)
				m_pDevCmn->fnNET_PreDialStart();

			// [#RWC6-505] US ryan.payton 2023.01.01 CashDepot implement remaining balance inquiry
#if (APP_CUSTOM_CASHDEPOT)
			if (m_CDService != NULL)
				P_NH_CD_GetConfigurations();
#endif
			// End of [#RWC6-505]

			// 5. Select Transaction or Pre-Balance Offer
			// [#2185] US Justin 2013.05.03 PAI Dual Host
			if( m_HostConfig == HC_DUALHOST )
			{
				m_pDevCmn->TranStatus = TRAN_TRAN;
				TranCode = TC_WITHDRAWAL;
				return FID_NH_NOR_WITHDRAWAL;
			}
			else
			{
				if ( (m_PreBalanceStatus == PRE_BAL_NONE) && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PREBALANCE) == ENABLE) &&	// [#2265] NH Justin 2014.05.06 Enable Balance Inquiry even it is EMV Transaction
					 (m_pDevCmn->fnSNS_GetEnhancedAudio()==FALSE) )																																				// [#2375] US Justin 2015.10.27 Combine ADA and Screen Transaction Flow
					return FID_NH_NOR_PREBALANCE;
				else
					return FID_NH_NOR_SELECTTRAN;
			}
			// End of [#2185]
		}
		else if (nRes == RES_EMV_IC_FIRST)
		{
			NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_NOR_Common] [IC FIRST TRANSACTION]\n"));
			//nRes : RES_MAKE_AID, RES_EMV_FALLBACK, RES_EMV_TERMINATE 
			nRes = P_EMV_ICCardRead();

			// [#2305] US Justin 2014.11.13 "NON FALLBACK" --- Move RMSClose Timing to reduce Unlatch Delay.
			// RMS Listen Port will be closed in "ContinueFallBack Function" for Fallback Case  => EMV transaction case Connection Close will be called twice (Not an issue)
			if (nRes != RES_EMV_FALLBACK)		
				m_pDevCmn->fnNET_RMSConnectClose();		
			// End of [#2305]
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
			// [#2280] US Justin 2014.06.17 USE COntinue Fallback as a Default
			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			// nRes = P_EMV_FallBackProc();
			nRes = P_EMV_ContinueFallBackProc();
			// End of [#2280]

			m_pDevCmn->fnNET_RMSConnectClose();				// [#J008] US Justin 2018.12.12 Bug Fix : RMS Connection for Fallback Transaction

			if (nRes != RES_OK)
			{
				//m_pDevCmn->fnNET_RMSConnectClose();		// [#2305] US Justin 2014.11.13 "FALLBACK CARD READ FAILURE" --- Move RMSClose Timing to reduce Unlatch Delay. 
															// [#J008] US Justin 2018.12.12 Bug Fix : RMS Connection for Fallback Transaction	
				break;
			}

			// 1. Read Card
			// nRes : RES_OK, RES_CARD_ERROR, RES_CARD_MOD_10
			if (P_NH_NOR_ReadCard() != RES_OK)
				break;

			// [#2517] US Justin 2017.11.29 EMV Fallback Enable/Diable Option
			#if (US_VERSION || MX_VERSION)   // RWC6-656 Fall back 01/02/2024
				if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_ENABLE_DISABLE) == EMV_FALLBACK_DISABLE) &&
					(LIB_IsICCard(m_sCardData.strISO2Data) == TRUE) &&
					(m_pDevCmn->m_bUnKnownAID_SkipPE != TRUE)	)				// Magnetic Stripe Transaction for Unknown IC Card => Continue
				{
					//nRes = RES_USER_EXIT;					// Displaying "Transaction Cancelled"
					nRes = RES_EMV_FALLBACK_DISABLED;		// [#2554] NH Justin 2018.06.05 Change Fallback Disabled Error Message
					break;
				}
			#endif
			// End of [#2517]

			m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);

			// [#2325] NH KSK 2015.01.22 이미 RFID Disable 처리된 상태이므로 처리 안함
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
		// Delay_Msg(50);	// [#2308] US KSK 2014.11.14 Latch / UnLatch 속도 개선을 위해 Delay 제거
		// End of [#2175]
	}

#ifdef UNDER_CE // [Win32-fixes] US ryan.payton 2022.12.13 Card gets stuck in a inserted state without being able to remove it
	// [#2175] US Justin 2013.01.18 US EMV
	if (nRes == RES_USER_EXIT || nRes == RES_USER_TIMEOUT || RES_EMV_FALLBACK_DISABLED)		// [#2554] NH Justin 2018.06.05 Change Fallback Disabled Error Message
		P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127005));
	else
		P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
	// End of [#2175]
#else
	// m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);
#endif

	g_sBizFlowInfo.nReasonforCancel = nRes;

	return FID_NH_NOR_CANCEL;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: F_NH_NOR_PreBalance()
RETURN TYPE  : Next Flow ID
PARAMETER    : -
DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_NOR_PreBalance()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"PreBalance");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_NOR_PreBalance]\n"));

	// [#2183] NH Justin 2013.04.04 Receipt Option, Printer Error//  PreBalance is available only for US and Mexican Mode....... 
	if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN) == RECEIPT_ON_SCREEN_DISABLE)&&
		(m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL) )
	{
		TranCode = TC_WITHDRAWAL;
		#if (MX_VERSION)
			return FID_MX_NOR_WITHDRAWAL;
		#else
			return FID_NH_NOR_WITHDRAWAL;
		#endif
	}
	// end of [#2183]

	BIZ_RETURN	nRes;

	while(1)
	{
		// 1. Pre-Balance
		//[#2292] US Justin 2014.10.09 Dual Balance
		//nRes = P_NH_NOR_StartPreBalance();
		//if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE)==ENABLE)&&(m_pDevCmn->fnMCU_IsEmvTransaction()!=TRUE) )	// [#2309]
		if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE)==ENABLE )													// [#2340] US Justin 2015.04.20 Cardtronics EMV Dual Balance
		{
			// [#RWC6-28] PAI Needs to Remove Dual Balance Question and Use Dual Balance Screen Directly When Enabled
			/*
			// [#2516] US Justin 2017.11.21 PAI Customization - STD3 version info / extra step for Pre Dual Balance inquiry)
			#if (APP_CUSTOM_PAI)
				nRes = P_NH_NOR_StartPreBalance(FALSE);
				if (nRes == RES_NO)
					return FID_NH_NOR_SELECTTRAN;
				else if(nRes != RES_YES)
					break;
			#endif
			// End of [#2516]
			*/
			// end of [#RWC6-28]

			nRes = P_NH_NOR_StartDualBalance(TRUE);			// [#2340] US Justin 2015.04.17 Add Parameter
		}
		else
			nRes = P_NH_NOR_StartPreBalance();
		// End of [#2292]

		if (nRes == RES_YES)
		{
			m_PreBalanceStatus = PRE_BAL_START;
			#if (MX_VERSION)			// [#2232] MX Justin 2013.11.14 Mexico Prebalance Bug Fix
				return FID_MX_NOR_INQUIRY;
			#else
				return FID_NH_NOR_INQUIRY;
			#endif			
		}
		else if (nRes == RES_NO)
			return FID_NH_NOR_SELECTTRAN;

		break;
	}

	g_sBizFlowInfo.nReasonforCancel = nRes;

	return FID_NH_NOR_CANCEL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_NOR_SelectTran()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_NOR_SelectTran()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"SelectTran");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_NOR_SelectTran]\n"));

	// [#2340] MX Justin... Enable Balance Inquiry for Mexico regardless of TDL OPtion => Disable [#2291]
	// [#2291] MX Justin 2014.08.27 Disable Selection in Mexico if TDL is not available.
	//#if( (MX_VERSION)&&(!APP_TDL_OPTION) )
	//	TranCode = TC_WITHDRAWAL;
	//	return FID_MX_NOR_WITHDRAWAL;
	//#endif
	// End of [#2291]
	// End of [#2340]

	// [#2375] US Justin 2015.10.27
	if( !IsAdaTransaction() )			// Disable Screen Receipt option for ADA Mode
	{
		// [#2183] NH Justin 2013.04.04 Receipt Option, Printer Error//  F_NH_NOR_SelectTran is available only for US and Mexican Mode....... 
		if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN) == RECEIPT_ON_SCREEN_DISABLE) &&
			(m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL) )
		{
			TranCode = TC_WITHDRAWAL;
			#if (MX_VERSION)
				return FID_MX_NOR_WITHDRAWAL;
			#else
				return FID_NH_NOR_WITHDRAWAL;
			#endif
		}
		// end of [#2183]
	}
	// End of [#2375]

	BIZ_RETURN	nRes;

	while(1)
	{
		// 1. Select Transaction
		if ((nRes = P_NH_NOR_SelectTransaction()) != RES_OK)
			break;

		// check Transaction Type
		if (TranCode == TC_WITHDRAWAL)
		{
			#if (MX_VERSION)	// [#2137] MX KSK 2012.07.25
				return FID_MX_NOR_WITHDRAWAL;
			#else
				return FID_NH_NOR_WITHDRAWAL;
			#endif				// end of [#2137]
		}
		else if (TranCode == TC_INQUIRY)
		{
			// [#2137] MX KSK 2012.07.25
			#if (MX_VERSION)	
				return FID_MX_NOR_INQUIRY;
			#elif (US_VERSION)
				// [#2375] US Justin 2015.10.27
				if( IsAdaTransaction() )			
				{
					nRes = P_NH_NOR_SelectBIAccount();
					if( nRes==RES_OK)
						return FID_NH_NOR_INQUIRY;
					else
						break;
				}
				else
				{
					// [#2309] US Justin Dual Balance 
					if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE)==ENABLE )	// [#2340] US Justin 2015.04.20 Enable Dual Balance for EMV.
					{
						nRes = P_NH_NOR_StartDualBalance(FALSE);
						if( nRes==RES_YES)
							return FID_NH_NOR_INQUIRY;
						else if (nRes!=RES_NO)	// RES_NO : Main Menu
							break;
					}
					else
					{
						nRes = P_NH_NOR_SelectBIAccount();
						if( nRes==RES_OK)
							return FID_NH_NOR_INQUIRY;
						else if (nRes!=RES_NO)	// RES_NO : Main Menu
							break;
					}
					// End of [#2309]
				}
				// End of [#2375]
			#else
				return FID_NH_NOR_INQUIRY;
			#endif		
			// end of [#2137]
		}
		else if (TranCode == TC_TRANSFER)
			return FID_NH_NOR_TRANSFER;
			
		// [#2150] US Justin 2012.10.04
		else if (TranCode == TC_PINCHANGE)
			return FID_NH_NOR_PINCHANGE;
		// End of [#2150]

		// [#2496] US Justin 2017.08.14
		else if (TranCode == TC_JUSTCASH_BITCOIN)
			return FID_NH_JUSTCASH_DEBIT;
		// End of [#2496]

		// [#2557] US Justin 2018.06.21
		#if (APP_GPAY_GIFTCARD_PURCHASE)
		else if (TranCode == TC_GP_BUY_CARD)
		{
			FLOW_ID	nFlowID = F_NH_GivePay_PurchaseGiftCard();	// Return FID_NH_NOR_EXIT(Success), FID_NH_NOR_CANCEL(Cancelled or  Denied), FID_NH_GPAY_RETURN2MAIN_FLOWS
			if(nFlowID != FID_NH_GPAY_RETURN2MAIN_FLOWS)		// Main Menu....	
				return nFlowID;
		}
		#endif
		// End of [#2557]
		else
		{
			break;
		}
	}

#if (MX_VERSION||US_VERSION)	// [#2115] MX KSK 2012.03.05 EMV 거래 중 취소하는 경우 카드 처리	// [#2175] US Justin 2013.01.18 Add US EMV
	if (m_pDevCmn->fnMCU_IsEmvTransaction())
		P_EMV_RemoveCard();
#endif				// end of [#2115]

	g_sBizFlowInfo.nReasonforCancel = nRes;
	
	return FID_NH_NOR_CANCEL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_NOR_Withdrawal()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_NOR_Withdrawal()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"Withdrawal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Withdrawal]\n"));

	BIZ_RETURN	nRes;

	BOOL bIsDenominationSelect = FALSE;

	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_WITHDRAWAL;

	while (1)
	{
		// 2. Receive Receipt
		if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
		{
			if ((nRes = P_NH_NOR_ReceiveReceipt()) != RES_OK)
				break;

#if (APP_CUSTOM_CASHDEPOT)
			_cdTransactionState.ReceiptSelected = true;
#endif
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

		if ((nRes = P_NH_NOR_InputCWAmount()) == RES_DENOMINATION_SELECT) // [#RWC6-12] Denomination Selection Support
		{
			bIsDenominationSelect = TRUE;
			if ((nRes = P_NH_NOR_DenominationSelect()) != RES_OK)
				break;
		}
		else if (nRes != RES_OK)
			break;

		// 5. Make EMV Data
		if(m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			if (P_NH_NOR_EMV_IC_Processing() != RES_OK)
				break;
		}
		// End [#2175]

		// [#RWC6-505] US ryan.payton 2023.01.01 CashDepot implement remaining balance inquiry
		// [RWC6-645  DYNAMICDCC  - RBI reference
#if (APP_CUSTOM_CASHDEPOT)&&(!APP_CUSTOM_DYNAMICDCC)
		if (m_CDService != NULL && _cdTransactionState.Configurations.RbiStatus)
			P_NH_CD_RemainingBalanceInquiry();
#endif
		// End of [#RWC6-505]

		// 7. Display Surcharge
		if ((nRes = P_NH_NOR_CalculateAndShowWithDrawalSurcharge()) != RES_OK)	// [#2496] US Justin 2017.08.18 Make a Function for all countries
			break;

		// 8. Transaction
		// [#2292] US Justin 2014.10.02 Add TDL DCC
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
				// [#2188] NH KSK 2013.05.22	EMV 거래 중 Error인 경우 카드 제거 후 Reversal 수행
				if (m_pDevCmn->m_bDisplayDeclined == TRUE)
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127005));
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
				// end of [#2188]
			}
			return FID_NH_NOR_REVERSAL;
		}

		// [#2535] US Justin 2017.04.16 GivePay PiggyBack Transaction
		#if (APP_GPAY_GIFTCARD_PURCHASE)
			if( m_GivePayData.CanMakeGPTransaction(TRUE) && (m_bAdaTransaction == FALSE) ) // No piggyback for ADA // [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6
			{
				// Return FID_NH_NOR_EXIT(Successful Transaction) or FID_NH_NOR_CANCEL(Transaction Cancelled or Denied)
				FLOW_ID	nFlowID = F_NH_GivePay_PurchaseGiftCard(TRUE);
			}
		#endif
		// End of [#2535]

		// [#RWC6-505] US ryan.payton 2023.05.25 CashDepot implement remaining balance inquiry
#if (APP_CUSTOM_CASHDEPOT)
		if (!_cdTransactionState.RbiSelected)
#endif
		{
			// [#2175] US Justin 2013.01.18 US EMV
			// 9.Complete EMV Transaction
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
			// [#2375] US Justin 2015.11.03 Add VG for Approved Transaction
			else 
			{
				if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Transaction_OK.wav", TRUE);
			}
			// End of [#2375]
			// End of [#2175]
		}
		// End of [#RWC6-505]

		// 10. Cash Dispense
		//if ((nRes = P_NH_NOR_CashDispense()) != RES_OK)	// [#RWC6-12]
		if ((nRes = P_NH_NOR_CashDispense(bIsDenominationSelect)) != RES_OK)
		{
			// 9. Tack Cash
			if (nRes == RES_DEV_CDU_ERR_PARTIAL)
				P_NH_NOR_TakeCash();

			// 8. Save JNL and Update Statistics	Full reversal이더라도 거래 Count 증가 사양 (미국)
			P_NH_NOR_SaveCWStatus(FALSE);

			return FID_NH_NOR_REVERSAL;
		}

		m_pDevCmn->fnAPL_SetProcCount('6');						// Dispense OK

		// 11. Save JNL and Update Statistics
		P_NH_NOR_SaveCWStatus(FALSE);

		// 12. Take Cash
		P_NH_NOR_TakeCash();

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

		return FID_NH_NOR_REVERSAL;
	}
	// [#2282] US Justin Add missed "remove Card"
	// [#2175] US Justin 2013.01.18 US EMV
	//int nProcCount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT));	
	//if ((nProcCount < 3) && (nRes == RES_USER_EXIT || nRes == RES_USER_TIMEOUT))
	if(m_pDevCmn->fnMCU_IsEmvTransaction())
		P_EMV_RemoveCard();
	// End of [#2175]
	// End of [#2282]
	g_sBizFlowInfo.nReasonforCancel = nRes;
	return FID_NH_NOR_CANCEL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_NOR_Inquiry()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_NOR_Inquiry()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"Inquiry");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Inquiry]\n"));
	
	BIZ_RETURN	nRes;

	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_INQUIRY;
	while (1)
	{
		// [#2292] US Justin 2014.10.10 Implement Dual Balance
		// Clear Dual Balance Result
		m_DualBalance.ClearTranData();

		///////////////////////////////////////////////////////////////////////////////////////
		//// DUAL BALANCE INQUIRY
		///////////////////////////////////////////////////////////////////////////////////////
		if( m_DualBalance.m_bDualBalance == TRUE )
		{
			// Assign legacy Variable
			if(m_DualBalance.m_nDisplaceDevice == _DUAL_BALANCE_DISPLAY_SCREEN)	m_sUserSelection.nPrintReceipt = FALSE;
			else																m_sUserSelection.nPrintReceipt = RCPT_PAPER;

			// Making the First Balance Inquiry from Checking 
			m_SourceAccount = S_CHECKING;

			// [#2340] US Justin 2015.04.17 Cardtronics "Dual Balance for EMV"
			if(m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				if (P_NH_NOR_EMV_IC_Processing() != RES_OK)
					break;
			}
			// End of [#2340]

			nRes = P_NH_NOR_Transaction(TRUE);

			if(nRes==RES_OK) // Successful Balance Inquiry for Checking Account // Store Data and Journal
			{
				// Transaction is Successful
				m_DualBalance.m_bTranResult = TRUE;
				m_DualBalance.m_bCheckingBalance = TRUE;
				// Store Transaction Result
				if( MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE )
				{
					m_DualBalance.m_strAuthorizationNumber.Format(L"%-6.6s %-2.2s", m_sSTD1_TranResp.SystemTraceAuditNo_6, m_sSTD1_TranResp.NetworkIDCode_2);
					m_DualBalance.m_strSequenceNumber			= m_sSTD1_TranResp.TranSequenceNo_4;
					m_DualBalance.m_strTranDate					= m_sSTD1_TranResp.LocalDate_8;
					m_DualBalance.m_strTranTime					= m_sSTD1_TranResp.LocalTime_6;
					m_DualBalance.m_strBusinessDate				= m_sSTD1_TranResp.SettlementDate_8;
					m_DualBalance.m_strCheckingLedgerBalance	= m_sSTD1_TranResp.AccountBalance_V;
					m_DualBalance.m_strCheckingAvailBalance		= m_sSTD1_TranResp.AvailableBalance_V;
				}
				else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
				{
					m_DualBalance.m_strAuthorizationNumber		= m_sSTD3_TranResp.m_strAuthorizationNum_8;
					m_DualBalance.m_strSequenceNumber			= m_sSTD3_TranResp.m_strSequenceNumber_4;
					m_DualBalance.m_strTranDate					= m_sSTD3_TranResp.m_strTransactionDate_6;
					m_DualBalance.m_strTranTime					= m_sSTD3_TranResp.m_strTransactionTime_6;
					m_DualBalance.m_strBusinessDate				= m_sSTD3_TranResp.m_strBusinessDate_6;
					m_DualBalance.m_strCheckingAvailBalance		= m_sSTD3_TranResp.m_strAvailableBalance_8;
					if( !m_sSTD3_TranResp.m_strAmount1_Ext12.IsEmpty() )
						m_DualBalance.m_strCheckingLedgerBalance	= m_sSTD3_TranResp.m_strAmount1_Ext12;
					else
						m_DualBalance.m_strCheckingLedgerBalance	= m_sSTD3_TranResp.m_strAmount1_8;
				}
				// Leave Journal
				m_pDevCmn->m_JNLMgr.Save(NORMAL_TRX);
				// Update Transaction Summary
				m_pDevCmn->fnCDU_SumProc(SUM_OF_INQUIRY);

				// [#2502] US Justin 2017.08.25 Leave EMV Journal for Inquiry
				#if (US_VERSION)
					if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
						m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
				#endif
				// End of [#2502]
			}
			else		// Transaction Failure for the first Balance Inquiry (CHECKING)
			{
				// Leave Journal
				switch (nRes)
				{
					case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
					case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
					case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
				}

				// Set error code for receipt
				if (!m_pDevCmn->fnAPL_CheckError())
				{
					m_DualBalance.m_strCheckingError.Format(L"%s-%s", m_pDevCmn->fstrAPL_GetErrorProCount(), m_pDevCmn->fstrAPL_GetErrorCode(1));
				}
			}

			// Store Card and PIN BLOCK...
			CString strHolderName = m_strHolderName;		// [#2447] US Justin Save Holder Name
			CString strTmpISO2 = m_sCardData.strISO2Data;
			CString strTmpBankID = m_sCardData.strBankID;
			CString strTmpAccountNo = m_sCardData.strAccountNo;
			CString strTmpShowNumber = m_sCardData.strShowNumber;
			CString strTmpPassword = m_sUserSelection.strPassword;
			int		nTmpPrintReceipt = m_sUserSelection.nPrintReceipt;

			// Initialize All Parameter 
			fnAPP_InitializeDS();

			// [#2340] US Justin 2015.04.20 Dual Balance per one EMV Latch
			BOOL bEMVDualProc = TRUE;
			////////////////////////////////////////////////////////////////////////////////////////////
			// EMV Process from "Final Selection for Dual Transaction per latch - Cardtronics.
			if(m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				// [#2391] US Justin 2016.01.26 Support Multi Transactions per Latch
				if( P_EMV_FinalSelection4MultiTransactionPerLatch() != RES_OK )
					bEMVDualProc = FALSE;			
				// End of [#2391]
			}
			// End of [#2340]

			// Dual Balance Parameters
			m_DualBalance.m_bDualBalance = TRUE;
			//if(m_sUserSelection.nPrintReceipt==RCPT_PAPER)	m_DualBalance.m_nDisplaceDevice = _DUAL_BALANCE_DISPLAY_PRINTER;
			if(nTmpPrintReceipt==RCPT_PAPER)	m_DualBalance.m_nDisplaceDevice = _DUAL_BALANCE_DISPLAY_PRINTER;
			else								m_DualBalance.m_nDisplaceDevice = _DUAL_BALANCE_DISPLAY_SCREEN;

			// Assign Card Data and PIN BLOCK
			m_strHolderName = strHolderName;			// [#2447] US Justin
			m_sCardData.strISO2Data = strTmpISO2;
			m_sCardData.strBankID = strTmpBankID;
			m_sCardData.strAccountNo = strTmpAccountNo;
			m_sCardData.strShowNumber = strTmpShowNumber;
			m_sUserSelection.strPassword = strTmpPassword;
			m_sUserSelection.nPrintReceipt = nTmpPrintReceipt;

			// Transactin Type
			m_pDevCmn->TranStatus = TRAN_TRAN;
			TranCode = TC_INQUIRY;
			m_SourceAccount = S_SAVINGS;

			// [#2340] US Justin 2015.04.17 Cardtronics "Dual Balance for EMV"
			if( (m_pDevCmn->fnMCU_IsEmvTransaction())&&(bEMVDualProc) )
			{
				if (P_NH_NOR_EMV_IC_Processing() != RES_OK)
					bEMVDualProc = FALSE;
			}
			// End of [#2340]

			// The second Balance Inquiry - Savings Account
			// [#2340] US Justin 2015.04.20 Dual Balance EMV Trandaction
			// nRes = P_NH_NOR_Transaction(FALSE);
			if( (m_pDevCmn->fnMCU_IsEmvTransaction())&&(!bEMVDualProc))		// Need to print receipt.
				nRes = RES_HOST_SEND_ERR;
			else
			{
				BOOL bShowScreen = FALSE;
				if(m_pDevCmn->fnMCU_IsEmvTransaction() )
					bShowScreen = TRUE;

				nRes = P_NH_NOR_Transaction(bShowScreen);
			}
			// End of [#2340]
		}
		else			//// Regular Balance INQUIRY
		{
			// 2. Receive Receipt
			// Receipt/Screen is selected a previous screen for Dual Balance....
			if( (IsAdaTransaction()) || (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE)!=ENABLE) )
			{
				if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
				{
#if (APP_CUSTOM_CASHDEPOT)
					if (!_cdTransactionState.ReceiptSelected)
#endif
					{
						if ((nRes = P_NH_NOR_ReceiveReceipt()) != RES_OK)
							break;
					}
				}
				else
				{
					if ((nRes = P_NH_NOR_ErrorReceipt()) != RES_OK)		
						break;
				}
			}
			// End of [#2387]

			//[#2292] Set Account above
			// 2. Select Account
			//if ((nRes = P_NH_NOR_SelectBIAccount()) != RES_OK)
			//	break;
			// End of [#2292]

			// [#2150] US Justin 2012.09.27 Implement Dynamic Flow.. Balance Inquiry Surcharge
			// 3. Display Balance Inqiury Fee....
			if( (m_sSTD1_DynamicFlowResp.R1_ConnectionResult==ST_OK) && 
				(m_sSTD1_DynamicFlowResp.R1_Surcharge_BalanceInquiry.GetLength() == 8)&&
				(Asc2Int(m_sSTD1_DynamicFlowResp.R1_Surcharge_BalanceInquiry)>0) )
			{
				if ((nRes = P_NH_NOR_DisplaySurcharge(SURCHARGE_DISP_BALANCEINQUIRY)) != RES_OK)
					break;
			}
			// End of [#2150]

			// [#2175] US Justin 2013.01.18 US EMV
			// 3. Make EMV Data
			if(m_pDevCmn->fnMCU_IsEmvTransaction())
			{				
				if (P_NH_NOR_EMV_IC_Processing() != RES_OK)
					break;
			}
			// End of [#2175]
		
			// condition DIALUP & PREDIAL ENABLE
			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE) == ENABLE &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION) == 2)
				m_pDevCmn->fnNET_PreDialStart();

			// 4. Transaction
			nRes = P_NH_NOR_Transaction(TRUE);
		}

		//if ((nRes = P_NH_NOR_Transaction(TRUE)) != RES_OK)
		if (nRes != RES_OK)
		// End of [#2292]
		{
			// JNL Save.
			switch (nRes)
			{
			case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
			case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
			case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
			}

			// Set error code for receipt for 2nd balance
			if (!m_pDevCmn->fnAPL_CheckError())
			{
				m_DualBalance.m_strSavingError.Format(L"%s-%s", m_pDevCmn->fstrAPL_GetErrorProCount(), m_pDevCmn->fstrAPL_GetErrorCode(1));
			}

			// [#2175] US Justin 2013.01.18 US EMV
			if (m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				// [#2401] US Justin 2016.03.08 Continue 2nd transaction even the 1st transaction is declined
				if( (m_DualBalance.m_bDualBalance!=TRUE) || (m_PreBalanceStatus!=PRE_BAL_START) || (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CONTINUETRANS_ENABLE)!=ENABLE) )
				{
					// [#2188] NH KSK 2013.05.22
					if (m_pDevCmn->m_bDisplayDeclined == TRUE)
						P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127005));
					else
						P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
					// end of [#2188]
				}
				// End of [#2401]
			}
			// End of [#2175]
			return FID_NH_NOR_REVERSAL;
		}

		// 5.Complete EMV Transaction
		// [#2175] US Justin 2013.01.18 US EMV
		if (m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			if( (m_PreBalanceStatus != PRE_BAL_START) || (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CONTINUETRANS_ENABLE) != ENABLE) )	// [#2391] US Justin 2016.01.26 Support Continue Transaction after EMV Transaction
				nRes = P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127004));
		}
		// [#2375] US Justin 2015.11.03 Add VG for Approved Transaction
		else 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Transaction_OK.wav", TRUE);
		}
		// End of [#2375]
		// End of [#2175]

		// 6. SAVE E-JNL
		m_pDevCmn->m_JNLMgr.Save(NORMAL_TRX);
	
		// 7. Update Statistics
		m_pDevCmn->fnCDU_SumProc(SUM_OF_INQUIRY);	

		// [#2502] US Justin 2017.08.25 Leave EMV Journal for Inquiry
		#if (US_VERSION)
			if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
				m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
		#endif
		// End of [#2502]

		// [#2292] US Justin 2014.10.10 Dual Balance
		if( m_DualBalance.m_bDualBalance == TRUE )
		{
			// Transaction is Successful
			m_DualBalance.m_bTranResult = TRUE;
			m_DualBalance.m_bSavingBalance = TRUE;
			// Store Transaction Result
			if( MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE )
			{
				m_DualBalance.m_strAuthorizationNumber.Format(L"%-6.6s %-2.2s", m_sSTD1_TranResp.SystemTraceAuditNo_6, m_sSTD1_TranResp.NetworkIDCode_2);
				m_DualBalance.m_strSequenceNumber			= m_sSTD1_TranResp.TranSequenceNo_4;
				m_DualBalance.m_strTranDate					= m_sSTD1_TranResp.LocalDate_8;
				m_DualBalance.m_strTranTime					= m_sSTD1_TranResp.LocalTime_6;
				m_DualBalance.m_strBusinessDate				= m_sSTD1_TranResp.SettlementDate_8;
				m_DualBalance.m_strSavingLedgerBalance	= m_sSTD1_TranResp.AccountBalance_V;
				m_DualBalance.m_strSavingAvailBalance		= m_sSTD1_TranResp.AvailableBalance_V;
			}
			else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
			{
				m_DualBalance.m_strAuthorizationNumber		= m_sSTD3_TranResp.m_strAuthorizationNum_8;
				m_DualBalance.m_strSequenceNumber			= m_sSTD3_TranResp.m_strSequenceNumber_4;
				m_DualBalance.m_strTranDate					= m_sSTD3_TranResp.m_strTransactionDate_6;
				m_DualBalance.m_strTranTime					= m_sSTD3_TranResp.m_strTransactionTime_6;
				m_DualBalance.m_strBusinessDate				= m_sSTD3_TranResp.m_strBusinessDate_6;
				m_DualBalance.m_strSavingAvailBalance		= m_sSTD3_TranResp.m_strAvailableBalance_8;
				if( !m_sSTD3_TranResp.m_strAmount1_Ext12.IsEmpty() )
					m_DualBalance.m_strSavingLedgerBalance	= m_sSTD3_TranResp.m_strAmount1_Ext12;
				else
					m_DualBalance.m_strSavingLedgerBalance	= m_sSTD3_TranResp.m_strAmount1_8;
			}
		}
		// End of [#2292]

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

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_NOR_Transfer()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_NOR_Transfer()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"Transfer");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Transfer]\n"));

	BIZ_RETURN	nRes;

	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_TRANSFER;

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
		if ((nRes = P_NH_NOR_SelectTRAccount()) != RES_OK)
			break;

		// condition DIALUP & PREDIAL ENABLE
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE) == ENABLE &&
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION) == 2)
			m_pDevCmn->fnNET_PreDialStart();

		// 3. Input Amount
		if ((nRes = P_NH_NOR_InputTRAmount()) != RES_OK)
			break;

		// [#2175] US Justin 2013.01.18 US EMV
		// 4. Make EMV Data
		if(m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			if (P_NH_NOR_EMV_IC_Processing() != RES_OK)
				break;
		}
		// End of [#2175]

		// 5. Transaction
		if ((nRes = P_NH_NOR_Transaction(TRUE)) != RES_OK)
		{
			if (m_pDevCmn->TranStatus == TRAN_TRAN)		// Transaction Error 인 경우에만 Journal Save함
			{
				// JNL Save.
				switch (nRes)
				{
				case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
				case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
				case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
				}
			}

			// [#2175] US Justin 2013.01.18 US EMV
			if (m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				// [#2188] NH KSK 2013.05.22
				if (m_pDevCmn->m_bDisplayDeclined == TRUE)
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127005));
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
				// end of [#2188]
			}
			// End of [#2175]

			return FID_NH_NOR_REVERSAL;
		}

		// [#2175] US Justin 2013.01.18 US EMV
		// 6.Complete EMV Transaction
		if (m_pDevCmn->fnMCU_IsEmvTransaction())
			nRes = P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127004));
		// [#2375] US Justin 2015.11.03 Add VG for Approved Transaction
		else 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Transaction_OK.wav", TRUE);
		}
		// End of [#2375]
		// End of [#2175]

		// 7. SAVE E-JNL
		m_pDevCmn->m_JNLMgr.Save(NORMAL_TRX);
	
		// 8. Update Statistics
		m_pDevCmn->fnCDU_SumProc(SUM_OF_TRANSFER);	

		// [#2502] US Justin 2017.08.25 Leave EMV Journal for Transfer
		#if (US_VERSION)
			if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
				m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
		#endif
		// End of [#2502]

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

// [#2150] US Justin 2012.10.04 Dynamic Flow Add PIn Change
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_NOR_PinChange()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_NOR_PinChange()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"PinChange");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_PinChange]\n"));

	BIZ_RETURN	nRes;

	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_PINCHANGE;

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
			if ((nRes = P_NH_NOR_ErrorReceipt(FALSE)) != RES_OK)			// No Digital Receipt (PIN CHANGE)
				break;
		}

		// Assign Default informatioin
		m_SourceAccount = S_CHECKING;

		// 2. Surcharge for Pin Changing
		if(	( m_sSTD1_DynamicFlowResp.R1_ConnectionResult==ST_OK )&&
			( m_sSTD1_DynamicFlowResp.R1_Surcharge_PinChange.GetLength() == 8)&&
			( Asc2Int(m_sSTD1_DynamicFlowResp.R1_Surcharge_PinChange) > 0) )
		{
			if ((nRes = P_NH_NOR_DisplaySurcharge(SURCHARGE_DISP_PINCHANGE)) != RES_OK)
				break;
		}				

		// Get Original Pin Block
		m_pDevCmn->fnAPL_DeviceEnDisable(DEV_PIN, DISABLE);
		Delay_Msg(50);
		m_pDevCmn->fnAPL_BuildPinBlockWithAccountNo(m_sCardData.strAccountNo);
		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);
		m_sUserSelection.strPassword.Format(L"%16.16s", m_pDevCmn->fstrPIN_GetPinKeyData());
		NHDEBUG(DBG_CALL, (L"[CTranCmn::Original PIN Block Generaged:%s]\n", m_sUserSelection.strPassword));

		// 3. New Pin - 1st
		if((nRes = P_NH_NOR_InputPinChangePassword(0)) != RES_OK)
			break;	

		// Get New PIN BLOCK
		CString sNewPINBlock1;
		m_pDevCmn->fnAPL_DeviceEnDisable(DEV_PIN, DISABLE);
		Delay_Msg(50);
		m_pDevCmn->fnAPL_BuildPinBlockWithAccountNo(m_sCardData.strAccountNo);
		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);
		sNewPINBlock1.Format(L"%16.16s", m_pDevCmn->fstrPIN_GetPinKeyData());
		NHDEBUG(DBG_CALL, (L"[CTranCmn::1st PIN Block Generaged:%s]\n", sNewPINBlock1));

		// 4. New Pin - 2nd
		if ((nRes = P_NH_NOR_InputPinChangePassword(1)) != RES_OK)
			break;

		// Get New PIN BLOCK
		CString sNewPINBlock2;
		m_pDevCmn->fnAPL_DeviceEnDisable(DEV_PIN, DISABLE);
		Delay_Msg(50);
		m_pDevCmn->fnAPL_BuildPinBlockWithAccountNo(m_sCardData.strAccountNo);
		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);
		sNewPINBlock2.Format(L"%16.16s", m_pDevCmn->fstrPIN_GetPinKeyData());
		NHDEBUG(DBG_CALL, (L"[CTranCmn::2nd PIN Block Generaged:%s]\n", sNewPINBlock2));

		// Checking generated Pin blocks (original, New1, New2) 
		if( sNewPINBlock1.CompareNoCase(sNewPINBlock2) == 0 )
		{
			if(sNewPINBlock1.CompareNoCase(m_sUserSelection.strPassword)==0)
			{
				nRes = RES_PINCHANGE_SAMEASORIGINAL;
				break;
			}
			else
				m_sUserSelection.strNewPassword.Format(L"%s", sNewPINBlock2);
		}
		else
		{
			nRes = RES_PINCHANGE_DIFFERENTPIN;
			break;
		}

		// 5. Transaction
		if ((nRes = P_NH_NOR_Transaction(TRUE)) != RES_OK)
		{
			if (m_pDevCmn->TranStatus == TRAN_TRAN)		// Transaction Error 인 경우에만 Journal Save함
			{
				// JNL Save.
				switch (nRes)
				{
				case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
				case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
				case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
				}
			}

			// [#2289] US Justin 2014.08.10 Disable Remove Card Notice (PIN Change is not available with EMV Transaction)
			// [#2282] NH Justin Add missed "Remove Card" notice 
			/*
			if (m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				if (m_pDevCmn->m_bDisplayDeclined == TRUE)
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127005));
				else
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));
			}
			*/
			// End of [#2282]
			// End of [#2289]

			return FID_NH_NOR_REVERSAL;
		}

		// [#2289] US Justin 2014.08.10 Disable Remove Card Notice (PIN Change is not available with EMV Transaction)
		// [#2282] NH Justin Add missed "Remove Card" notice
		/*
		if (m_pDevCmn->fnMCU_IsEmvTransaction())
			nRes = P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127004));
		*/
		// End of [#2282]
		// End of [#2289]

		// 6. SAVE E-JNL
		m_pDevCmn->m_JNLMgr.Save(NORMAL_TRX);
		
		// 7. Update Statistics
		//m_pDevCmn->fnCDU_SumProc(SUM_OF_TRANSFER);	

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

		return FID_NH_NOR_REVERSAL;
	}
	// [#2289] US Justin 2014.08.10 Disable Remove Card Notice (PIN Change is not available with EMV Transaction)
	// [#2282] NH Justin Add missed "Remove Card" notice
	/*
	if(m_pDevCmn->fnMCU_IsEmvTransaction())
		P_EMV_RemoveCard();
	*/
	// End of [#2282]
	// End of [#2289]

	g_sBizFlowInfo.nReasonforCancel = nRes;
	return FID_NH_NOR_CANCEL;
}
// End of [#2150]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_NOR_Reversal()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_NOR_Reversal()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"Reversal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Reversal]\n"));

	int		nReversalExeCount = 0;
	int		nReversalTryCount = 0;
	CString	strTemp;		// KSK 2012.03.01 Code Sonar 대책
	BOOL	bNotPrintReceipt = FALSE;	// [#2115] MX KSK 2012.03.06	// [#2362] AU KSK 2015.08.12 EMV 조회 거래 중 Reversal시 명세표 미 Print하도록 수정

#if (MX_VERSION)	// [#2137] MX KSK 2012.07.25
	if ((TranCode == TC_INQUIRY) && (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) != REVERSAL_NONE))
		bNotPrintReceipt = TRUE;

	if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG) == MX_1ST_SEND)
	{
		if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_INTERNATIONAL_TYPE)
		{
			bNotPrintReceipt = TRUE;
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
		}
	}
#endif				// end of [#2137]

#if (AU_VERSION)	// [#2362] AU KSK 2015.08.12 EMV 조회 거래 중 Reversal시 명세표 미 Print하도록 수정
	if ((TranCode == TC_INQUIRY) && (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) != REVERSAL_NONE))
		bNotPrintReceipt = TRUE;
#endif				// end of [#2362]

	if (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) != REVERSAL_NONE)
	{
		// Get Try Count
		nReversalTryCount = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REVERSAL_RETRY);

		while (1)
		{
			if (nReversalExeCount <= nReversalTryCount)
			{
				// Reversal
				if (P_NH_NOR_Reversal() == RES_OK)
					break;
			}
			else
			{
				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
				
				strTemp.Format(L"%1.1s%cD0002(00)%c%s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT), UNIT_DELIMITER, UNIT_DELIMITER, 
														m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003083));
				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strTemp+MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG) );	// [#2292] JUSTIN 2014.10.07 Leave DCC INFO

				SetErrSum(L"D000200", DEV_NET);			
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
				
				break;
			}

			// reversal count가 999인경우 무한 revesal을 하기 위해서 count를 증가시키지 않는다.
			if(nReversalExeCount < 999)
				nReversalExeCount++;

			if (m_bPowerOffReversal == TRUE)
			{
				if (P_NH_NOR_ContiueReversal() == RES_YES)
					break;
			}
			else
				Delay_Msg(5000);	// KSK 2010.01.26 3초로 수정
		}

		// [#397] NH PSC 2008.12.15 EMV 거래시 reversal 전문에 ISR, IS Id Tag 값 전송후에 Clear
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_SCRIPT_ID_TLV,		L"");	// issuer script ID
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_SCRIPT_RESULTS_TLV,	L"");	// issuer script results
	}

	// [#639] NH KSK 2010.05.03
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CONFIGURATION_ENABLE) == 1)
	{
		// Option이 Disable인 경우 Success로 변수 설정
		m_pDevCmn->HostOpenFlag = TRUE;
		m_pDevCmn->HostOpenRetryTime = 0;
	}
	// end of [#639]

#if (MX_VERSION || AU_VERSION)	// [#2115] MX KSK 2012.03.06	// [#2362] AU KSK 2015.08.12
	if (bNotPrintReceipt == TRUE)
	{
		g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;
		return FID_NH_NOR_CANCEL;
	}
#endif				// end of [#2115]

	return FID_NH_NOR_RECEIPT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_NOR_Receipt()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_NOR_Receipt()
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"Receipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Receipt] nPrintReceipt(%d)\n", m_sUserSelection.nPrintReceipt));

	BIZ_RETURN	nRes = RES_NG;

#ifdef UNDER_CE
	// [#2518] US Kook 2018.01.14
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE) == ENABLE)
	{
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_MEMORIZE_ENABLE) == ENABLE)
		{
			if (m_pDevCmn->fnCAM_CaptureFace(Asc2Int(m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig)), TIME_RECEIPT_PROCESSING) != T_OK)
			{
				// [#2548] NH Justin 2018.04.27 Leave IN SERVICE even though the Camera is OUT OF SERVICE
				//m_pDevCmn->fnAPL_StackError(m_pDevCmn->fstrCAM_GetErrorCode(), m_pDevCmn->fstrCAM_GetErrorCode(), DEV_CMR);	
				NVDump('F', 'C', "93", m_pDevCmn->fstrCAM_GetErrorCode(), _T("Camera Error"));
				// End of [#2548]
			}
		}

		m_pDevCmn->fnCAM_Deinitialize();
	}
#endif // UNDER_CE

	// [#2466] US Justin 2017.01.27 Paypal Certification Issue. Skip printing receipt for successful Transaction
	#if(APP_PAYDIANT_CCA) 
	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE) == WITHDRAWAL_PAYPAL )
	{
		int nRequestedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
		int nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));
		if ( nDispensedAmount >= nRequestedAmount )			// No receipt for Successful Mobile Transaction
			return FID_NH_NOR_EXIT;

		m_sUserSelection.nPrintReceipt = RCPT_PAPER;		// Paper Receipt
	}
	#endif
	// End of [#2466]

	// [#2183] NH Justin 2013.04.03 Receipt Option (No Transaction result on Screen) : Called by US, Canada, Mexican Mode. (AU Mode use it's own function)
	// [#2219] 2013.09.04 JUSTIN Digital Receipt
	//if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN) == RECEIPT_ON_SCREEN_DISABLE) &&
	//	(LIB_IsReceiptPrintCondition() == TRUE) )
	//	m_sUserSelection.bPrintReceipt = TRUE;
	if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN)==RECEIPT_ON_SCREEN_DISABLE)&&(m_sUserSelection.nPrintReceipt==FALSE) )
	{
		if(LIB_IsReceiptPrintCondition() == TRUE)
			m_sUserSelection.nPrintReceipt = RCPT_PAPER;
	}
	// End of [#2219]
	// End of [#2183]

	BOOL bPrintReceipt = FALSE;									// [#2380]	US Justin 2015.12.09 Cardtronics Additional Voice Guidance Change
	if (m_sUserSelection.nPrintReceipt == RCPT_PAPER)			// [#2219] 2013.09.04 Justin Digital Receipt, Change Variable Name
	{
		P_NH_NOR_SelectEnhancedCoupon();	// return은 의미 없음
		if ((nRes = P_NH_NOR_PrintReceipt()) == RES_OK)
		{
			P_NH_NOR_TakeReceipt();
			bPrintReceipt = TRUE;								// [#2380]	US Justin 2015.12.09 Cardtronics Additional Voice Guidance Change									
		}
	}
	else if (m_sUserSelection.nPrintReceipt == RCPT_QRCODE)		// [#2219] 2013.09.04 Justin Digital Receipt
	{
		nRes = P_NH_NOR_DisplayQRCode();
	}
	// End of [#2219]
	// [#RWC6-16] Bitload 4 U. A new cryptocurrency feature for CE 6.0 ATMs
	else if (m_sUserSelection.nPrintReceipt == RCPT_DISPLAY)
	{
			nRes = P_NH_NOR_DisplayOnReceipt();
	}
	// end of #RWC-16]


	// [#2183] NH Justin 2013.04.03 Receipt Option (No Transaction result on Screen) : Called by US, Canada, Mexican Mode. (AU Mode use it's own function)
	if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN) == RECEIPT_ON_SCREEN_ENABLE) 
	{
		if (nRes != RES_OK || m_sUserSelection.nPrintReceipt == FALSE)			// [#2219] 2013.09.04 Justin Digital Receipt, Change Variable Name
				nRes = P_NH_NOR_DisplayOnReceipt();
	}
	// End of [#2183]

	// [#2375] US Justin 2015.10.30 Combine ADA and Screen Transaction Flow
	#if(US_VERSION)
		P_NH_ADA_TransactionResult(bPrintReceipt);				// [#2380]	US Justin 2015.12.09 Cardtronics Additional Voice Guidance Change									
	#else
		P_NH_ADA_GuideBalanceAmount();
	#endif
	// End of [#2375]

	// [#2391] US Justin 2016.01.26 Support Continue Transacton after EMV Transaction
	// [#2265] US Justin 2014.05.06 Enable Balance At First for EMV Transaction (return to Welcome Screen)
	/*
	if ((m_pDevCmn->TranResult) &&
		(TranCode == TC_INQUIRY) &&
		(m_PreBalanceStatus == PRE_BAL_START) &&
		(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CONTINUETRANS_ENABLE) == ENABLE) &&
		(m_pDevCmn->fnMCU_IsEmvTransaction() == FALSE) )	// [#2265] US Justin 2014.05.06 Enable Balance At First for EMV Transaction (return to Welcome Screen)
	{
		if ((nRes = P_NH_NOR_AnotherTransaction()) == RES_YES)
		{
			m_PreBalanceStatus = PRE_BAL_ANOTHER;

			int	nSavedLangMode = m_pDevCmn->fnSCR_GetCurrentLangMode();		// [#2388] US Justin 2016.01.25 Cardtronics Additional Request

			// Initialize for transaction
			fnAPP_SetEJournalData(TRUE);
			fnAPP_InitializeDS();

			m_pDevCmn->fnSCR_SetCurrentLangMode(nSavedLangMode);			// [#2388] US Justin 2016.01.25 Cardtronics Additional Request

			// [#2265] MX Justin Balance At First Bug Fix
			//return FID_NH_NOR_COMMON;
			#if (MX_VERSION)
				return FID_MX_NOR_COMMON;
			#else
				return FID_NH_NOR_COMMON;
			#endif
			// End of [#2265]
		}
	}
	*/

	//if( (m_pDevCmn->TranResult) && (TranCode == TC_INQUIRY) && (m_PreBalanceStatus == PRE_BAL_START) && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CONTINUETRANS_ENABLE) == ENABLE) )
	if( (TranCode == TC_INQUIRY) && (m_PreBalanceStatus == PRE_BAL_START) && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CONTINUETRANS_ENABLE) == ENABLE) )	// [#2401] US Justin 2016.03.08 Continue 2nd transaction even the 1st transaction is declined
	{
		// Other countries(Mexico) : Continue EMV Transaction after Balance Inquiry only for Magnetic Stripe Tr.
		#if !(US_VERSION)
		if (m_pDevCmn->fnMCU_IsEmvTransaction() == FALSE) 
		#endif
		{
			
			// [#2401] US Justin 2016.03.08 Continue 2nd transaction even the 1st transaction is declined
			BOOL bContinueNextTr = TRUE;
			if( (m_pDevCmn->TranResult!=TRUE)&&(m_DualBalance.m_bDualBalance!=TRUE) )
				bContinueNextTr = FALSE;

			if(bContinueNextTr==TRUE)
			{
				// [#RWC6-85] Dual Balance Screen Changes - PAI
				#if (APP_CUSTOM_PAI)
					nRes = RES_YES;
				#else
					nRes = P_NH_NOR_AnotherTransaction();
				#endif
				// end of [#RWC6-85]

				if (nRes == RES_YES)
				{
					m_PreBalanceStatus = PRE_BAL_ANOTHER;

					int	nSavedLangMode = m_pDevCmn->fnSCR_GetCurrentLangMode();
					CString strTmpPassword = m_sUserSelection.strPassword;

					fnAPP_SetEJournalData(TRUE);
					fnAPP_InitializeDS();					
					
					////////////////////////////////////////////////////////////////////////////////////////////
					// EMV Process from "Final Selection for Dual Transaction per latch 
					if(m_pDevCmn->fnMCU_IsEmvTransaction())		// US ONLY
					{
						if( P_EMV_FinalSelection4MultiTransactionPerLatch() != RES_OK )
						{
							LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122302), ABORT_SCR_TIMEOUT);
							P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127007));	
							return FID_NH_NOR_EXIT;
						}
					}
					m_sUserSelection.strPassword = strTmpPassword;
					m_pDevCmn->fnSCR_SetCurrentLangMode(nSavedLangMode);

					// [#2502] US Justin 2017.08.24 Bug Fix - Clearing Communicatio Error after Pre-Dual Balance
					m_DualBalance.ClearTranData();
					m_pDevCmn->fnAPL_ClearError();
					// End of [#2502]

					#if (MX_VERSION)
						return FID_MX_NOR_COMMON;
					#else
						return FID_NH_NOR_COMMON;
					#endif
				}
				else
				{
					if(m_pDevCmn->fnMCU_IsEmvTransaction())		// US ONLY
						P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127007));
				}
			}
			// End of [#2401]
		}
	}
	// End of [#2391]

#if (APP_CUSTOM_CASHDEPOT)
	if (_cdTransactionState.RbiSelected)
	{
		_cdTransactionState.RbiSelected = false;

		if (m_pDevCmn->fnMCU_IsEmvTransaction()) // US ONLY
		{
			if (P_EMV_FinalSelection4MultiTransactionPerLatch() != RES_OK)
			{
				LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122302), ABORT_SCR_TIMEOUT);
				P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127007));
				return FID_NH_NOR_EXIT;
			}
		}

		return FID_NH_NOR_INQUIRY;
	}
	else
#endif
	{
		return FID_NH_NOR_EXIT;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_NOR_Cancel()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_NOR_Cancel()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Cancel]\n"));

	BIZ_RETURN	nRes = g_sBizFlowInfo.nReasonforCancel;

	//if ((nRes == RES_USER_TIMEOUT) || (nRes == RES_USER_EXIT))
	if( (nRes == RES_USER_TIMEOUT) || (nRes == RES_USER_EXIT) || 
		(nRes == RES_PINCHANGE_SAMEASORIGINAL) || (nRes == RES_PINCHANGE_DIFFERENTPIN) || 	// [#2150] US Justin 2012.10.04 Add Pin Change Error 
		(nRes == RES_EMV_FALLBACK_DISABLED) )												// [#2554] NH Justin 2018.06.05 Change Fallback Disabled Error Message
	//		(nRes == RES_DAMAGED_ICCARD)  )													// [#2159] MX Justin 2012.10.26 Cancel transaction for damaged IC card =>[#2285] NH Justin 2014.07.09 Remove Damaged Card Restriction
	{
		P_NH_NOR_DisplayCancel(nRes);
		P_NH_ADA_RemoveHeadPhone();						// [#2380] US Justin 2015.12.01 Add Remove Head Phone VG for Cancelled Transactions.

		#if (APP_EVENT_NOTICE)
			BIZ_EVENT_SendHost(_EVENTID_CANCEL_TRAN);	// [#2313] US Justin 2014.11.20 Notice Event
		#endif

#ifdef UNDER_CE
 		// [#2518] US Kook 2018.01.27
 		if (GetConfigFuncPointer()->SupportsCamera())
 		{
 			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_MEMORIZE_ENABLE) == ENABLE)
 			{
 				// remove camera image of cancelled transaction.
				int nCancelledIndex = ((Asc2Int(m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig)) % 9999) + 1);
				NHDEBUG(DBG_CALL, (L"Delete PIN ENTERED Journal Picture, Cancelled Index = [%d]\n", nCancelledIndex ));		// [#2552]
 				m_pDevCmn->fnCAM_DeleteCamImage(nCancelledIndex, TIME_PIN_ENTERED);
 			}
 		}
 		// end of [#2518]
#else // [Win32-fixes] US ryan.payton 2022.12.13 Card gets stuck in a inserted state without being able to remove it
		m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);
#endif // UNDER_CE
	}

	return FID_NH_NOR_CLEANUP;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_NOR_Exit()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_NOR_Exit()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Exit]\n"));

	P_NH_NOR_DisplayExit();
	P_NH_ADA_RemoveHeadPhone();

	return FID_NH_NOR_CLEANUP;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_NOR_CleanUp()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_NOR_CleanUp()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_NOR_CleanUp]\n"));

	m_PreBalanceStatus = PRE_BAL_NONE;
	m_bAdaPasswordMode = FALSE;
	m_bAdaTransaction = FALSE;
	//m_bAdaSuccessiveInput = FALSE;					// [#2117] US PCS 2012.01.12	// [#2427] Justin Remove
	m_HostConfig = HC_ATM;							// [#2185] US Justin 2013.05.03 PAI Dual Host
	m_pDevCmn->m_nDCCLocalAvail = DCC_PRECHECK_OFF;	// [#2472] US Justin 2017.02.07 DCC WithdrawalOption
	m_bDCCDeclinedByOperator = FALSE;				// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message

// [#RWC6-505] US ryan.payton 2023.01.01 CashDepot implement remaining balance inquiry
#if (APP_CUSTOM_CASHDEPOT)
//	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT, DISPENSELIMIT_DEFAULT); // [#RWC6-513] US ryan.payton 2023.01.09 CashDepot implement max withdrawal

	_cdTransactionState.Reset();
#endif
// End of [#RWC6-505]

	P_NH_NOR_CleanUp();

	return FID_END_OF_FLOW;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_ADA_Common()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID	CTranCmn::F_NH_ADA_Common()
{
	NVDump('O', 'C', "00", L"F_NHADA", L"Common");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_ADA_Common]\n"));

	BIZ_RETURN	nRes;

	m_pDevCmn->fnSCR_DisplayPrevSet(004);
	m_pDevCmn->fnSCR_DisplayScreen(004, 0, PIN_ALL_MODE);

	m_bAdaTransaction = TRUE;

	m_pDevCmn->fnNET_RMSConnectClose();		// [#2305] US Justin 2014.11.13 "ADA TRANSACTION" --- Move RMSClose Timing to reduce Unlatch Delay. ADA Mode => Works the same way as before

#if (US_VERSION)
	// [#2413] US Justin 2016.04.08 VG for Popmoney and Pin4
	int nChosenFtn;

	if ((nRes = P_NH_ADA_Welcome(&nChosenFtn)) != RES_OK)
		return FID_NH_ADA_EXIT;

	if (nChosenFtn == (int)FID_NH_CARDLESS_FLOWS)
		return FID_NH_CARDLESS_FLOWS;
	else if (nChosenFtn == (int)FID_NH_POPMONEY_FLOWS)
		return FID_NH_POPMONEY_FLOWS;
	else if (nChosenFtn == (int)FID_NH_PIN4_FLOWS)
		return FID_NH_PIN4_FLOWS;
	else if (nChosenFtn == (int)FID_NH_JUSTCASH_START)		// [#2445] US Justin 2016.09.27 Just.Cash
		return FID_NH_JUSTCASH_START;
	else if (nChosenFtn == (int)FID_NH_PAYPAL_FLOWS)		// [#2446] US Justin 2016.09.29 US Paypal
		return FID_NH_PAYPAL_FLOWS;
	else if (nChosenFtn == (int)FID_NH_LIBERTYX_INIT)
		return FID_NH_LIBERTYX_INIT;
	else if (nChosenFtn == (int)FID_NH_DIGITALMINT_START)	// [#RWC6-399] US ryan.payton 2022.10.03 DigitalMint
		return FID_NH_DIGITALMINT_START;
	else
	{
		// Support Card Reader Only under Voice Guidance Mode.... ====> NEED TO IMPLEMENT RFID LATER... (2015.10.26)
		m_nEventKind = DEV_MCU;
		return FID_NH_NOR_COMMON;
	}
	// End of [#2413]

#elif(CA_VERSION)				
	// CA use "F_CA_ADA_Common()" and does not effect. But all follwoing ADA function should be disabled
	if ((nRes = P_NH_ADA_Welcome()) != RES_OK)
		return FID_NH_ADA_EXIT;
										
	m_nEventKind = DEV_MCU;				
	return FID_NH_NOR_COMMON;

#else
	///////// AU : Continue
	///////// CA : F_CA_ADA_Common()

	if ((nRes = P_NH_ADA_Welcome()) != RES_OK)
		return FID_NH_ADA_EXIT;

	//2. Read Card (Magnetic Stripe) nRes : RES_EMV_IC_FIRST, RES_NOR_MS_TRANS
	nRes = P_EMV_CheckTransMode();

	if (nRes != RES_EMV_IC_FIRST)
	{
		// nRes : RES_OK, RES_CARD_ERROR, RES_CARD_MOD_10
		if (P_NH_ADA_ReadCard() != RES_OK)
			return FID_NH_NOR_CANCEL;

		m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);
	}

	while(TRUE)
	{
		/*-----------------------------------------MS NORMAL------------------------------------------*/
		if (nRes == RES_NOR_MS_TRANS)
		{
			NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_ADA_Common] [MS NORMAL TRANSACTION]\n"));

			// condition DIALUP & PREDIAL ENABLE
			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE) == ENABLE &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION) == 0)
				m_pDevCmn->fnNET_PreDialStart();

			// 3. Enter Password
			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			if ((nRes = P_NH_ADA_EnterPassword()) != RES_OK)
				break;

			// [#2311] US Justin 2014.11.18 Support TDL Dynamic Surcharge
			if ( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE) && 
				 (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_TIRSURCHARGE) == ENABLE) && 
				 (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DCC) != ENABLE ) )			// [#2365] US Justin 2015.07.27 Support DCC+ in ADA Mode (Surcharge)

			{
				if ( (nRes = P_NH_NOR_TDL_Connection(TC_TDL_TIRSURCHARGE, TRUE)) != RES_OK )
				{
					nRes = RES_USER_EXIT;
					break;
				}
			}
			// End of [#2311]

			// condition DIALUP & PREDIAL ENABLE
			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE) == ENABLE &&
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION) == 1)
				m_pDevCmn->fnNET_PreDialStart();

			// 4. Select Trnasaction	
			if ((nRes = P_NH_ADA_SelectTransaction()) != RES_OK)
				break;
 
			// check Transaction Type
			if (TranCode == TC_WITHDRAWAL)
				return FID_NH_ADA_WITHDRAWAL;
			else if (TranCode == TC_INQUIRY)
				return FID_NH_ADA_INQUIRY;
			else if (TranCode == TC_TRANSFER)
				return FID_NH_ADA_TRANSFER;
			break;
		}

		/*------------------------------------------EMV FIRST-------------------------------------------*/
		else if (nRes == RES_EMV_IC_FIRST)
		{
			NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_ADA_Common] [IC FIRST TRANSACTION]\n"));
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
			nRes = P_EMV_ADA_SelectMultiAIDList();
		}
		else if (nRes == RES_EMV_FALLBACK)
		{
			// [#2280] Use Continue Fallback as default
			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			//nRes = P_EMV_ADA_FallBackProc();
#if(US_VERSION || AU_VERSION)
			nRes = P_EMV_ContinueFallBackProc();
#else
			nRes = P_EMV_ADA_FallBackProc();
#endif
			// End of [#2280]

#if (AU_VERSION)	// KSK 2016.06.21
			if (nRes == RES_PROGRAM_ERROR)
				m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);	// Medea 갱신을 위해 Card Disable 처리함
#endif

			if (nRes != RES_OK)
				break;

			// 1. Read Card
			// nRes : RES_OK, RES_CARD_ERROR, RES_CARD_MOD_10
			if (P_NH_ADA_ReadCard() != RES_OK)
				break;

			m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);

			nRes = RES_NOR_MS_TRANS;
		}
		/*------------------------------------------OTHER BRANCH----------------------------------------*/
		else
		{
			// Break하는 조건 
			// 1: EMV_TERMINATE인 경우
			// 2: User Cancel인 경우
			// 3: Timeout인 경우
			// 4. MS FIRST [#2077] CA PCS 2011.07.03 MS FIRST를 사양에서 제거 할 예정이기 때문에.
			break;
		}

		Delay_Msg(50);
	}
	if (nRes == RES_USER_EXIT || nRes == RES_USER_TIMEOUT)
		P_EMV_RemoveCard();
	else
		P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));	

	// End of [#2175]

	return FID_NH_ADA_CANCEL;
#endif
}

// [#2375] US Justin 2015.10.28... Disable for US and CA VERSIONs... TO AVOID CONFUSION
#if !( US_VERSION || CA_VERSION)	
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_ADA_Reversal()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_ADA_Reversal()
{
	NVDump('O', 'C', "00", L"F_NHADA", L"Reversal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Reversal]\n"));

	int		nReversalExeCount = 0;
	int		nReversalTryCount = 0;
	CString	strTemp;	// KSK 2012.03.01 Code Sonar 지적사항 대책

	if (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) != REVERSAL_NONE)
	{
		// Get Try Count
		nReversalTryCount = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REVERSAL_RETRY);

		while (1)
		{
			if (nReversalExeCount <= nReversalTryCount)
			{
				// Reversal
				if (P_NH_ADA_Reversal() == RES_OK)
					break;
			}
			else
			{
				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

				strTemp.Format(L"%1.1s%cD0002(00)%c%s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT), UNIT_DELIMITER, UNIT_DELIMITER, 
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003083));
				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strTemp+MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG) );	// [#2292] JUSTIN 2014.10.07 Leave DCC INFO

				// end of KSK 2012.03.01
				
				SetErrSum(L"D000200", DEV_NET);
				
				m_pDevCmn->m_JNLMgr.Save(REVERSAL_TRX);

#if (AU_VERSION)	// [#2353] AU KSK 2015.06.28 AU EMV Data 저장 기능 추가	(추가적으로 ADA시 EMV Data 미저장 버그 적용)
				// [#2294] US JUSTIN 2014.10.07 Leave EMV Data in US VERSION
				if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
					m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
				// End of [#2294]
#endif

				break;
			}

			// reversal count가 999인경우 무한 revesal을 하기 위해서 count를 증가시키지 않는다.
			if(nReversalExeCount < 999)
				nReversalExeCount++;

			Delay_Msg(5000);
		}

		// [#397] NH PSC 2008.12.15 EMV 거래시 reversal 전문에 ISR, IS Id Tag 값 전송후에 Clear
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_SCRIPT_ID_TLV,		L"");	// issuer script ID
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_SCRIPT_RESULTS_TLV,	L"");	// issuer script results
	}
	// [#639] NH KSK 2010.05.03
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CONFIGURATION_ENABLE) == 1)
	{
		// Option이 Disable인 경우 Success로 변수 설정
		m_pDevCmn->HostOpenFlag = TRUE;
		m_pDevCmn->HostOpenRetryTime = 0;
	}
	// end of [#639]

	return FID_NH_ADA_RECEIPT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_ADA_Receipt()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_ADA_Receipt()
{
	NVDump('O', 'C', "00", L"F_NHADA", L"Receipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Receipt] SlipSelectFlag(%d)\n", m_sUserSelection.nPrintReceipt));

	BIZ_RETURN	nRes = RES_NG;

	if (m_sUserSelection.nPrintReceipt == RCPT_PAPER)		// [#2219] 2013.09.04 Justin Digital Receipt, Change Variable Name
	{
		if ((nRes = P_NH_ADA_PrintReceipt()) == RES_OK)
			P_NH_ADA_TakeReceipt();
	}

	P_NH_ADA_GuideBalanceAmount();

	return FID_NH_ADA_EXIT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_ADA_Cancel()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_ADA_Cancel()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_ADA_Cancel]\n"));

	BIZ_RETURN	nRes = g_sBizFlowInfo.nReasonforCancel;
	
	P_NH_ADA_DisplayCancel(nRes);

	return FID_NH_NOR_CLEANUP;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_ADA_Exit()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_ADA_Exit()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_ADA_Exit]\n"));

	P_NH_ADA_DisplayExit();

	return FID_NH_NOR_CLEANUP;
}
#endif

// [#2144] US Justin 2012.08.30		Additional Service => ATM FLOW
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: F_NH_NOR_ReadCardOrRFID()
 RETURN TYPE  : Next Flow ID
 PARAMETER    : -
 DESCRIPTION  : Start ATM transaction from 3rd party application : Read Card
-------------------------------------------------------------------*/
FLOW_ID CTranCmn::F_NH_NOR_ReadCardOrRFID()
{
	int nRtn = P_NH_NOR_ReadCardOrRFID();
	
	if (nRtn == RES_ATM_READCARD_NORMAL)			
		return FID_NH_NOR_COMMON;
	else		
		return FID_NH_NOR_CANCEL;					//RES_ATM_READCARD_CANCEL, RES_ATM_READCARD_TIMEOUT
}
// End of [#2144]

// [#2396] US Justin 2016.02.10 PAI Pin 4
FLOW_ID CTranCmn::F_NH_CardlessTransaction()
{
	m_pDevCmn->fnNET_RMSConnectClose(); // [#2405] US Justin HalCash Online

	int nRtn = P_NH_SelectCardlessTransaction();

	if		(nRtn == RES_CARDLESS_PIN4)			return FID_NH_PIN4_FLOWS;
	else if	(nRtn == RES_CARDLESS_POPMONEY)		return FID_NH_POPMONEY_FLOWS;
	else if (nRtn == RES_CARDLESS_PAYPAL)		return FID_NH_PAYPAL_FLOWS;		// [#2446]

#if (!APP_CUSTOM_PAI)
	else if (nRtn == RES_CARDLESS_JUSTCASH)		return FID_NH_JUSTCASH_START;	// [#2445] US Justin 2016.09.27 Just.Cash
	else if (nRtn == RES_CARDLESS_B4U)			return FID_NH_B4U_REQ_INFO_FLOWS;
	else if (nRtn == RES_CARDLESS_LIBERTYX)		return FID_NH_LIBERTYX_INIT;
	else if (nRtn == RES_CARDLESS_DIGITALMINT)	return FID_NH_DIGITALMINT_START;
#endif

	else										g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;

	return FID_NH_NOR_CANCEL;
}

FLOW_ID CTranCmn::F_NH_BitcoinTransaction()
{
	m_pDevCmn->fnNET_RMSConnectClose(); // [#2405] US Justin HalCash Online

	int nRtn = P_NH_SelectBitcoinTransaction();

	if		(nRtn == RES_BITCOIN_B4U)			return FID_NH_B4U_REQ_INFO_FLOWS;
	else if	(nRtn == RES_BITCOIN_LIBERTYX_BUY)	return FID_NH_LIBERTYX_INIT; // [#RWC6-59] US William 2019.10.09 LibertyX
	else if	(nRtn == RES_BITCOIN_LIBERTYX_SELL)	return FID_NH_LIBERTYX_INIT; // [#RWC6-224] US William 2020.10.16 LibertyX
	else										g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;

	return FID_NH_NOR_CANCEL;
}

#if (APP_POPMONEY) // (APP_CUSTOM_PAI)	[#2471] US Justin 2017.02.01 Enable Popmoney to All Customers.
// [#2350] US Justin 2015.06.17 POP Money
FLOW_ID CTranCmn::F_NH_POPMoney()
{
	NVDump('O', 'C', "00", L"F_NH", L"POPMoney");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_POPMoney]\n"));

	m_pDevCmn->fnNET_RMSConnectClose();		// [#2405] US Justin HalCash Online

	BIZ_RETURN	nRes;

	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_POPMONEY;

	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE, WITHDRAWAL_POPMONEY);
	m_pDevCmn->nKindOfMedia = MEDIA_MS;								// Proceed as Magnetic stripe Mode
	MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID,		L"");

	while (1)
	{
		// 1. Select Language
		if ((nRes = P_NH_NOR_SelectLanguage()) != RES_OK)			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			break;

		// 2. Enter Access Code
		if ((nRes = P_NH_POPMoney_EnterAccessCode()) != RES_OK)		// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			break;

		// 3. Enter Password
		if ((nRes = P_NH_NOR_EnterPassword()) != RES_OK)			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT 
			break;

		// 4. Receive Receipt
		if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
		{
			if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SELECTRECEIPT) == ENABLE)
			{
				nRes = P_NH_NOR_ReceiveReceiptWithoutQR();
				if(nRes == RES_YES)
					m_sUserSelection.nPrintReceipt = RCPT_PAPER;
				else if(nRes != RES_NO)		
					break;	
			}
			else
				m_sUserSelection.nPrintReceipt = RCPT_PAPER;
		}
		else
		{
			// Do not provide Digital Receipt	(POP MONEY)	
			if ((nRes = P_NH_NOR_ErrorReceipt(FALSE))!= RES_OK)		
				break;
		}

		// 5. Select Account
		if ((nRes = P_NH_NOR_SelectCWAccount()) != RES_OK)
			break;

		// 6. Input Amount
		if ((nRes = P_NH_NOR_InputCWAmount()) != RES_OK)
			break;

		// 7. Transaction
		nRes = P_NH_NOR_Transaction(TRUE);
		if( nRes != RES_OK )
		{
			// JNL Save.
			switch (nRes)
			{
			case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
			case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
			case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
			}
			return FID_NH_NOR_REVERSAL;
		}

		// 8. Cash Dispense
		// [#2442] US Justin 2016.08.25 PopMoney Day Total
		//if ((nRes = P_NH_NOR_CashDispense()) != RES_OK)
		nRes = P_NH_NOR_CashDispense();

		// 9. Update PopMoney Day Total
		int nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));
		m_PopMoneyData.RecodeDispenseResult(nDispensedAmount );

		// 10. Cash Dispense
		if ( nRes != RES_OK )	// End of [#2442]
		{
			// 11. Tack Cash
			if (nRes == RES_DEV_CDU_ERR_PARTIAL)
				P_NH_NOR_TakeCash();

			// 12. Save JNL and Update Statistics
			P_NH_NOR_SaveCWStatus(FALSE);

			return FID_NH_NOR_REVERSAL;
		}

		m_pDevCmn->fnAPL_SetProcCount('6');						// Dispense OK

		// 11. Save JNL and Update Statistics
		P_NH_NOR_SaveCWStatus(FALSE);

		// 12. Tack Cash
		P_NH_NOR_TakeCash();

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
		
		return FID_NH_NOR_REVERSAL;
	}
	g_sBizFlowInfo.nReasonforCancel = nRes;
	return FID_NH_NOR_CANCEL;
}
// End of [#2350]
#endif

#if (APP_PIN4_CASHPICKUP)		// [#2471] US Justin 2017.02.01 Enable Pin4 to All Customers.
FLOW_ID	CTranCmn::F_NH_Pin4Transaction()
{
	NVDump('O', 'C', "00", L"F_NH", L"Pin4");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_Pin4Transaction]\n"));

	m_pDevCmn->fnNET_RMSConnectClose();		// [#2405] US Justin HalCash Online

	BIZ_RETURN	nRes;

	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_PIN4;
	m_SourceAccount = S_CHECKING;									// [#2527] Set Default account : From Checking Account
	m_Pin4.ResetTransactionData();

	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE, WITHDRAWAL_PIN4);
	m_pDevCmn->nKindOfMedia = MEDIA_MS;								// Proceed as Magnetic stripe Mode
	MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID,		L"");

	while (1)
	{
		// 1. Select Language
		if ((nRes = P_NH_NOR_SelectLanguage()) != RES_OK)			// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			break;

		// 2. Receive Receipt
		if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
		{
			if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SELECTRECEIPT) == ENABLE)
			{
				nRes = P_NH_NOR_ReceiveReceiptWithoutQR();
				if(nRes == RES_YES)
					m_sUserSelection.nPrintReceipt = RCPT_PAPER;
				else if(nRes != RES_NO)		
					break;	
			}
			else
				m_sUserSelection.nPrintReceipt = RCPT_PAPER;
		}
		else
		{
			// Do not provide Digital Receipt
			if ((nRes = P_NH_NOR_ErrorReceipt(FALSE))!= RES_OK)		
				break;
		}

		// 3. Enter Phone Number US or International
		if ((nRes = P_NH_Pin4_Enter_PhoneNumber()) != RES_OK)		// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			break;

		// 4. Input Amount
		if ((nRes = P_NH_NOR_InputCWAmount(FALSE)) != RES_OK)
			break;

		// 5. Enter 4 digit Pin4 Code
		if ((nRes = P_NH_Pin4_Enter_Pin4_Code(FALSE)) != RES_OK)	// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			break;

		// 6. Enter 4 digit Secret Pin
		if ((nRes = P_NH_Pin4_Enter_Pin4_Code(TRUE)) != RES_OK)		// nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
			break;
		
		// 7. Set Flag : RMS Other Message(Phone number, Pin4 Code, Secret Code) and Reversal NONE Flag
		m_Pin4.SetJournalRMSOtherMsg();

		// [#2515] US Justin 2017.11.14 PIN4 Prestaging
		#if(APP_PRESTAGIN_PIN4)
			// 8. Get Token
			if ( (nRes = P_NH_Pin4_GetToken())!= RES_OK)	
			{
				nRes = RES_USER_EXIT;								// [#2527] Display Transaction Cancelled
				break;
			}
		#endif
		
		// 9. Auth with PIN4 Server		
		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);	// SET  No Reversal...Regular ATM Transaction : "REVERSAL_TRAN" is set in STD1, STD2, STD3 Host Response....
		nRes = P_NH_Pin4_Auth_PIN4();
		if( nRes != RES_OK )
		{
			NHDEBUG(DBG_CALL, (L" PIN4 AUTHENTICATION ERROR. nRes = [%d]\n", nRes));

			if(m_Pin4.m_nNeedReversal == 1)		// DENIAL CODE "011" => Reversal Mandatory
			{
				// Regular ATM Transaction : "REVERSAL_TRAN" is set in STD1, STD2, STD3 Host Response....
				//MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
				P_NH_Pin4_Reversal_PIN4();
				m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);
			}
			else
			{
				switch (nRes)
				{
					case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
					case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
					case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
					default:				m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
				}
			}
			return FID_NH_NOR_REVERSAL;
		}
		
		#if(APP_PRESTAGIN_PIN4)
			// 10, RE Enter 4 digit Secret Pin => Generate PIN Block
			nRes = P_NH_Pin4_ReEnter_SecretCode();
			if (nRes != RES_OK)				// RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT, 
			{
				P_NH_Pin4_Reversal_PIN4();
				m_Pin4.SetJournalRMSOtherMsg(true);	// Add User Cancel on Journal
				m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);
				break;
			}

			// 11. Auth with ATM Processor
			nRes = P_NH_Pin4_Auth_Processor();
			if(nRes != RES_OK )
			{
				P_NH_Pin4_Reversal_PIN4();
				switch (nRes)
				{
					case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
					case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
					case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
					default:				m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;	// [#RWC6-40] PAI MCCP Transaction Delay
				}
				return FID_NH_NOR_REVERSAL;
			}
		#endif

		// 9. Cash Dispense
		nRes = P_NH_NOR_CashDispense();

		// 10. Update HalCash Day Total
		int nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));
		m_Pin4.RecodeDispenseResult(nDispensedAmount );

		if (nRes != RES_OK)			// Dispensing Failure
		{
			// 11. Save JNL and Take Cash
			if (nRes == RES_DEV_CDU_ERR_PARTIAL)
			{
				NHDEBUG(DBG_CALL, (L"PIN4 - PARTIAL DISPENSE : AMT=[%d]\n", nDispensedAmount));
				// PARTIAL DISPENSING
					// : NO PIN4 REVERSAL (OLD and NEW Flow) =>  ATM HOST REVERSAL (PRESTAGING MODE ONLY)
				// [#2515] US Justin 2017.11.14 PIN4 Prestaging
				#if(APP_PRESTAGIN_PIN4)
					// NEW FLOW.. NO PIN4 Reversal => ATM Processor Reversal => Journal Save
					// MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);		==> Set in STD1,3 Host Response Method
				#else
					// OLD FLOW.. NO PIN4 Reversal.. Save Journal then EXIT
					m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);
				#endif
				// End of [#2515]
				P_NH_NOR_TakeCash();
			}
			else
			{
				NHDEBUG(DBG_CALL, (L"PIN4 - NO DISPENSE : AMT=[%d]\n", nDispensedAmount));
				// NO DIEPSNE...
					// : PIN4 Reversal => ATM HOST Reversal (Prestaging Mode Only)
				// [#2515] US Justin 2017.11.14 PIN4 Prestaging
				// Regular ATM Transaction : "REVERSAL_TRAN" is set in STD1, STD2, STD3 Host Response....
				//MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
				P_NH_Pin4_Reversal_PIN4();

				#if(APP_PRESTAGIN_PIN4)
					// NEW FLOW ATM Processor Reversal => Journal Save
					// MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);		==> Set in STD1,3 Host Response Method
				#else
					// OLD Flow. Save Reversl Journal and Terminate.
					m_pDevCmn->m_JNLMgr.Save(REVERSAL_TRX);
				#endif
				// End of [#2515]
			}

			//m_pDevCmn->fnCDU_SumProc(SUM_OF_WITHDRAWAL);

			// [#2515] US Justin 2017.11.14 PIN4 Prestaging
			#if(!APP_PRESTAGIN_PIN4)
				m_pDevCmn->fnCDU_SumProc(SUM_OF_WITHDRAWAL);
			#endif
			// End of [#2515]
			return FID_NH_NOR_REVERSAL;
		}
		m_pDevCmn->fnAPL_SetProcCount('6');						// Dispense OK

		// 11. Update Dispense Summary and Save JNL
		P_NH_NOR_SaveCWStatus(FALSE);

		// 12. Take Cash
		P_NH_NOR_TakeCash();
		return FID_NH_NOR_REVERSAL;
	}

	g_sBizFlowInfo.nReasonforCancel = nRes;
	return FID_NH_NOR_CANCEL;
}
#endif
// End of [#2396]

// [#2446] US Justin 2016.09.29 Paypal CCA
#if (APP_PAYDIANT_CCA)
FLOW_ID	CTranCmn::F_NH_PaypalCCATransaction()
{
	NVDump('O', 'C', "00", L"F_NH", L"PayPalCCA");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_PaypalCCATransaction]\n"));
	BIZ_RETURN	nRes;

	// Deny RMS Connection
	m_pDevCmn->fnNET_RMSConnectClose();	

	// Set Default Transaction Variables
	MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID,		L"");
	m_pDevCmn->nKindOfMedia = MEDIA_MS;								// Proceed as Magnetic stripe Mode
	m_sUserSelection.nPrintReceipt = FALSE;							// [#2466] US Justin 2017.01.27 Not printing by default
	m_SourceAccount = S_CHECKING;									// From Checking Account
	m_PayPalCCA.ResetTransactionData();

	// Assign Transaction Type
	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_PAYPALCCA;
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE, WITHDRAWAL_PAYPAL);

	while (1)
	{
		// Proceed with Default Language
		//m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);

		// 1. Get Token
		if ((nRes = P_NH_PaypalCCA_GetToken()) != RES_OK)
			break;

		// [#2523] NH Justin 2017.12.21 Paypal - Add Cancel State before retrieve customer info.
		if ((nRes = P_NH_PaypalCCA_WaitUserCancel()) != RES_OK)
			break;
		// End of [#2523]

		// 2. Show QR and Get Customer Info
		if ((nRes = P_NH_PaypalCCA_ShowQR_GetCustomerInfo()) != RES_OK)
			break;

		// 3. Get Cash Ticket
		if ((nRes = P_NH_PaypalCCA_GetCashTicket()) != RES_OK)
			break;

		// 4. Find Surcharge and Update to Paypay Server
		m_nDisplayedSurchargeAmount = 0;
		if (LIB_CheckSurchargeDisplay() == TRUE)
		{
			// Find Surcharge
			int nSurchargeOption = SURCHARGE_DISP_NORMAL;
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) == PERCENT_MODE )
				nSurchargeOption = SURCHARGE_DISP_PRECENT;
			else if( MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) == TABLE_MODE )
				nSurchargeOption = SURCHARGE_DISP_TABLE;

			m_nDisplayedSurchargeAmount = Asc2Int(P_NH_NOR_CalculateLocalSurcharge(nSurchargeOption));
		}
		if(m_nDisplayedSurchargeAmount>0)
		{
			m_PayPalCCA.UpdateSurcharge(m_nDisplayedSurchargeAmount, PAYPAL_SURCHARGE_TYPE_ATM);		// [#2522] US Justin 2017.12.18 Update Processor Surcharge
			MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEDISPLAY, 1);
			if ((nRes = P_NH_PaypalCCA_UpdateSurcharge()) != RES_OK)
				break;
		}

		// 5. ATM Processor Authorization
		nRes = P_NH_PaypalCCA_Processor_Auth();
		if( nRes != RES_OK )
		{
			// JNL Save.
			switch (nRes)
			{
				case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
				case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
				case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
			}

			// 6. Update Transaction to Paydiant Cloud
			P_NH_PaypalCCA_UpdateTransaction(PAYPAL_UPDATETRAN_DENIED, 0, nRes);
			return FID_NH_NOR_REVERSAL;
		}
		m_PayPalCCA.UpdateSurcharge( Asc2Int(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT) ), PAYPAL_SURCHARGE_TYPE_PROCESSOR );	// [#2522] US Justin 2017.12.18 Update Processor Surcharge
		
		// 6. Cash Dispense
		nRes = P_NH_NOR_CashDispense();

		// 7. Update Day Total
		int nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

		// 8. Update Transaction to Paydiant Cloud
		P_NH_PaypalCCA_UpdateTransaction(PAYPAL_UPDATETRAN_DISPENSED, nDispensedAmount);
		
		// 9. Dispense Error
		if (nRes != RES_OK)	
		{
			// 9-1. Tack Cash
			if (nRes == RES_DEV_CDU_ERR_PARTIAL)
				P_NH_NOR_TakeCash();

			// 9-2. Save JNL and Update Statistics
			P_NH_NOR_SaveCWStatus(FALSE);

			// 9-3. Reversal Using the ATM processor
			m_PayPalCCA.m_nTransactionStep = PAYPAL_TRAN_HOSTREVERSAL;
			return FID_NH_NOR_REVERSAL;
		}

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

		// 10. Set Proc Count (Dispense OK)
		m_pDevCmn->fnAPL_SetProcCount('6');

		// 11. Save JNL and Update Statistics
		P_NH_NOR_SaveCWStatus(FALSE);

		// 12. Take Cash
		P_NH_NOR_TakeCash();

		return FID_NH_NOR_REVERSAL;
	}

	g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;
	return FID_NH_NOR_CANCEL;
}
#endif
// End of [#2446]

// [#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
#if (APP_GPAY_GIFTCARD_PURCHASE)
FLOW_ID	CTranCmn::F_NH_GivePay_PurchaseGiftCard(BOOL bPiggyBack)
{
	NVDump('O', 'C', "00", L"F_NHNOR", L"GPAY_PURCHAECARD");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_GivePay_PurchaseGiftCard]\n"));
	BIZ_RETURN	nRes;

	// Backup Transaction Code and Language Mode
	int nTempTranCode = TranCode;
	int nSavedLangMode = m_pDevCmn->fnSCR_GetCurrentLangMode();

	m_pDevCmn->TranStatus = TRAN_TRAN;
	TranCode = TC_GP_BUY_CARD;

	BOOL bTryAuthrization = FALSE;
	m_GivePayData.ResetGPTransactionData();

	m_GivePayData.m_bTempDG911 = FALSE; // TODO need this to be in flow // [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix

	// Set Language(English or Spanish)
	if(nSavedLangMode != SPN_MODE )		
		m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);	
	else
	{
		m_GivePayData.m_sLanguageExtension = L"_es";
		m_pDevCmn->fnSCR_SetCurrentLangMode(SPN_MODE);	
	}

	int nTransactionStep = 1;
	int nCardCalledStep = 5;
	while (1)
	{
		while(1)
		{
			switch(nTransactionStep)
			{
				// 1. Select "Give a Card" or "Pay for Wireless"
				case 1:					
					nRes = P_NH_GivePay_SelectServices(bPiggyBack);					// nRes : RES_OK, RES_USER_PREV, RES_USER_EXIT, RES_USER_TIMEOUT
					if(nRes == RES_OK)												nTransactionStep = 2;		// Get Token and other parameter
					else if(nRes == RES_USER_PREV)									nTransactionStep = -2;		// Return to Main
					else															nTransactionStep = -1;		// Exit
					break;

				// 2. Get Transaction information
				case 2:			
					if( P_NH_GivePay_DownloadData(GPAY_DOWNLOAD_TRANINFO) != RES_OK)		// [#2574] US Justin GivePay Enhancement3	
					{
						// for test purpose only
						// [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix
						if (m_GivePayData.m_bTempDG911)
							nTransactionStep = 2;	
						else // end of [#RWC6-1, #2583] // TODO need this to be in flow
							nTransactionStep = -1;
					}
					else
					{
						if(m_GivePayData.m_nGPServiceType == GPAY_SERVICE_GIFTCARD) 		nTransactionStep = 5;
						else if(m_GivePayData.m_nGPServiceType == GPAY_SERVICE_WIRELESS)	nTransactionStep = 20;	
						else																nTransactionStep = -1;
					}
					break;

				// 5. Select Gift card Category
				case 5:
					nCardCalledStep = 5;
					nRes = P_NH_GivePay_SelectCategory();							// nRes : RES_GIVEPAY_FINALSELECT, RES_OK, RES_USER_PREV, RES_USER_EXIT, RES_USER_TIMEOUT
					if(nRes==RES_GIVEPAY_FINALSELECT)	nTransactionStep = 7;
					else if(nRes==RES_OK)				nTransactionStep = 6;
					else if(nRes==RES_USER_PREV)		nTransactionStep = 1;
					else								nTransactionStep = -1;
					break;

				// 6. Select Gift card Sub Category
				case 6:
					nCardCalledStep = 6;
					nRes = P_NH_GivePay_SelectSubCategory();						// nRes : RES_OK, RES_USER_PREV, RES_USER_EXIT, RES_USER_TIMEOUT 
					if(nRes==RES_OK)					nTransactionStep = 7;
					else if(nRes==RES_USER_PREV)		nTransactionStep = 5;
					else								nTransactionStep = -1;
					break;

				// 7. Get Gift Cards
				case 7:
					nRes = P_NH_GivePay_GetCards(TRUE);								// nRes : RES_OK,  Network Error
					if(nRes==RES_OK)					nTransactionStep = 8;
					else								nTransactionStep = -1;
					break;

				// 8. Select Card
				case 8:
					nRes = P_NH_GivePay_SelectCard();								// nRes : RES_OK, RES_USER_PREV, RES_USER_EXIT, RES_USER_TIMEOUT 
					if(nRes==RES_OK)					nTransactionStep = 9;
					else if(nRes==RES_USER_PREV)		nTransactionStep = nCardCalledStep;
					else								nTransactionStep = -1;
					break;

				// 9. Select/Enter Amount
				case 9:
					nRes = P_NH_GivePay_EnterAmount();								// nRes : RES_OK, RES_USER_PREV, RES_USER_EXIT, RES_USER_TIMEOUT 
					if(nRes==RES_OK)					nTransactionStep = 10;
					else if(nRes==RES_USER_PREV)		nTransactionStep = 8;
					else								nTransactionStep = -1;
					break;

				// 10. Select Card Greeting
				case 10:
					nRes = P_NH_GivePay_SelectGreeting();							// nRes : RES_OK, RES_USER_PREV, RES_USER_EXIT, RES_USER_TIMEOUT 
					if(nRes==RES_OK)					nTransactionStep = 11;
					else if(nRes==RES_USER_PREV)		nTransactionStep = 9;
					else								nTransactionStep = -1;
					break;

				// 11. Enter Sender Mobile Phone Name 
				case 11:
					nRes = P_NH_GivePay_EnterPhoneNumber(GPAY_PHONETYPE_SENDER);	// nRes : RES_OK, RES_USER_PREV, RES_USER_EXIT, RES_USER_TIMEOUT 
					if(nRes==RES_OK)					nTransactionStep = 12;
					else if(nRes==RES_USER_PREV)		nTransactionStep = 10;
					else								nTransactionStep = -1;
					break;
				
				// 12. Enter Recipient Mobile Phone Number
				case 12:
					nRes = P_NH_GivePay_EnterPhoneNumber(GPAY_PHONETYPE_RECEIPIENT);// nRes : RES_OK, RES_USER_PREV, RES_USER_EXIT, RES_USER_TIMEOUT 
					if(nRes==RES_OK)					nTransactionStep = 50;
					else if(nRes==RES_USER_PREV)		nTransactionStep = 11;
					else								nTransactionStep = -1;
					break;
				
				// WIRELESS
				// 20. Select Wireless
				case 20:
					nRes = P_NH_GivePay_SelectWireless();							// nRes : RES_OK, RES_USER_PREV, RES_USER_EXIT, RES_USER_TIMEOUT 
					if(nRes==RES_OK)					nTransactionStep = 21;
					else if(nRes==RES_USER_PREV)		nTransactionStep = 1;
					else								nTransactionStep = -1;
					break;

				// 21. Enter topup amount
				case 21:
					nRes = P_NH_GivePay_EnterAmount();								// nRes : RES_OK, RES_USER_PREV, RES_USER_EXIT, RES_USER_TIMEOUT 
					if(nRes==RES_OK)					nTransactionStep = 22;
					else if(nRes==RES_USER_PREV)		nTransactionStep = 20;
					else								nTransactionStep = -1;
					break;
					

				// 22. Enter Mobile Phone Number
				case 22:
					nRes = P_NH_GivePay_EnterPhoneNumber(GPAY_PHONETYPE_WIRELESS);	// nRes : RES_OK, RES_USER_PREV, RES_USER_EXIT, RES_USER_TIMEOUT 
					if(nRes==RES_OK)					nTransactionStep = 50;
					else if(nRes==RES_USER_PREV)		nTransactionStep = 21;
					else								nTransactionStep = -1;
					break;	

				// 50. Confirm SMS
				case 50:
					nRes = P_NH_GivePay_ConfirmSMS();								// nRes : RES_OK, RES_USER_PREV, RES_USER_EXIT, RES_USER_TIMEOUT 
					if(nRes==RES_OK)					nTransactionStep = 51;
					else if(nRes==RES_USER_PREV)
					{
						if(m_GivePayData.m_nGPServiceType == GPAY_SERVICE_GIFTCARD) 		
							nTransactionStep = 12;
						else
							nTransactionStep = 22;	
					}
					else								nTransactionStep = -1;
					break;

				// 51. Staging
				case 51:
					nRes = P_NH_GivePay_Staging(TRUE);								// nRes : RES_OK,  Network Error
					if(nRes==RES_OK)					nTransactionStep = 52;
					else								nTransactionStep = -1;
					break;

				// 52. Summary
				case 52:
					nRes = P_NH_GivePay_PurchaseSummary();							// nRes : RES_OK, RES_GIVEPAY_VIEWTNC, RES_USER_EXIT, RES_USER_TIMEOUT
					if(nRes==RES_OK)					nTransactionStep = 54;
					else if(nRes==RES_GIVEPAY_VIEWTNC)	nTransactionStep = 53;
					else								nTransactionStep = -1;
					break;

				// 53. View Terms
				case 53:
					nRes = P_NH_GivePay_ViewTerms();								//  nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT 
					if(nRes==RES_OK)					nTransactionStep = 52;
					else								nTransactionStep = -1;
					break;

				// 54. Zip Code
				case 54:
					nRes = P_NH_GivePay_EnterZipCode();								//  nRes : RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT 
					if(nRes==RES_OK)					nTransactionStep = 100;
					else								nTransactionStep = -1;
					break;

				default:
					nTransactionStep = -1;	// Exit (Cancel Transaction)
					break;
			}
			
			if( (nTransactionStep == -1) || (nTransactionStep == -2) || (nTransactionStep == 100) )
				break;
		}

		if( (nTransactionStep == -1) || (nTransactionStep == -2) )
			break;

		// Make EMV Data
		/////////////////////////////////////////////////////////////////////////
		// Phase 2 Development
		/*
		if(m_pDevCmn->fnMCU_IsEmvTransaction())
		{			
			if (P_NH_NOR_EMV_IC_Processing() != RES_OK)
				break;
		}
		*/

		// Authorization - Transaction
		nRes = P_NH_GivePay_Purchase_Auth(TRUE);
		bTryAuthrization = TRUE;
		if( nRes != RES_OK )
		{
			// Save Journal
			CString strJnlKnd = _T("");
			switch (nRes)
			{
				case RES_HOST_SEND_ERR:		strJnlKnd = COMMUNICATION_ERROR;	break;
				case RES_HOST_RECV_ERR:		strJnlKnd = TRX_ERROR;				break;
				case RES_HOST_DENIED:		strJnlKnd = TRX_NOT_APPROVED;		break;
			}
			if(!strJnlKnd.IsEmpty())
				m_pDevCmn->m_JNLMgr.Save(strJnlKnd, m_GivePayData.MakeGPTrJournalData(strJnlKnd));

			// GivePay Transaction Total
			m_GivePayData.RecodeDispenseResult(0, GPAY_SERVICE_CARD_PURCHASE);

			// Remove Card
			if( (bPiggyBack!=TRUE) && (m_pDevCmn->fnMCU_IsEmvTransaction()) )
				P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127005));	// Transaction ERROR

			// Print Receipt
			if( P_NH_GivePay_PrintReceipt(FALSE) != RES_OK )
				P_NH_GivePay_ScreenReceipt(FALSE);
			break;
		}

		// 2nd GenAC.... Phase 2 Development
		/*
		if( m_pDevCmn->fnMCU_IsEmvTransaction() )
		{
			// 2nd Gen AC 
			nRes = ....

			if( nRes != RES_OK )		// 2ndGenAC Failed 
			{
				// Reversal with GivePay
				GivePayEMVFailReversal();

				// Save Journal
				m_pDevCmn->m_JNLMgr.Save(TRX_ERROR, m_GivePayData.MakeGPTrJournalData(TRX_ERROR));

				// GivePay Transaction Total
				m_GivePayData.RecodeDispenseResult(0, GPAY_SERVICE_CARD_PURCHASE);

				// Remove Card
				if( bPiggyBack != TRUE )
					P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003));

				// Print Receipt
				if( P_NH_GivePay_PrintReceipt(FALSE) != RES_OK )
					P_NH_GivePay_ScreenReceipt(FALSE);
				break;
			}
		}
		*/

		// Save Journal
		m_pDevCmn->m_JNLMgr.Save(NORMAL_TRX, m_GivePayData.MakeGPTrJournalData(NORMAL_TRX));

		// GivePay Transaction Total
		m_GivePayData.RecodeDispenseResult(Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT)), GPAY_SERVICE_CARD_PURCHASE);

		// Remove Card
		if( m_pDevCmn->fnMCU_IsEmvTransaction() )
		{
			// Save EMV Data ... Phase2 Development	
			//m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);

			if( bPiggyBack!=TRUE)
				P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127004));
		}

		// Print Receipt
		if( P_NH_GivePay_PrintReceipt(TRUE) != RES_OK )
			P_NH_GivePay_ScreenReceipt(TRUE);
		break;
	}
	
	// Restore Transaction code and language mode
	TranCode = nTempTranCode;
	m_pDevCmn->fnSCR_SetCurrentLangMode(nSavedLangMode);
	m_GivePayData.ResetGPTransactionData();

	if(bPiggyBack != TRUE)
	{
		if( nTransactionStep == -2 )
			return FID_NH_GPAY_RETURN2MAIN_FLOWS;

		if(m_pDevCmn->fnMCU_IsEmvTransaction())
			P_EMV_RemoveCard();

		if(bTryAuthrization ==TRUE)
			return FID_NH_NOR_EXIT;								// Result was presented (Receipt or Screen)

		// Transaction stopped before Authorization
		g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;		// Display "Transactin Cancelled"
		return FID_NH_NOR_CANCEL;
	}

	if(nRes == RES_OK)		return FID_NH_NOR_EXIT;
	else					return FID_NH_NOR_CANCEL;
}

#endif
// End of [#2513]