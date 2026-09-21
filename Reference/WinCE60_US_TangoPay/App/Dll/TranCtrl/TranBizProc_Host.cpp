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

//------------------------------------------------------------------
//	Global Variable
//------------------------------------------------------------------
extern CTimeCheck	g_TimeCheck;

//------------------------------------------------------------------
//	Implement
//------------------------------------------------------------------
//BOOL CTranCmn::BIZ_HostConfigProc(BOOL bShowScreen)				//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
BOOL CTranCmn::BIZ_HostConfigProc(BOOL bShowScreen, int nConfigOption, BOOL clearNetworkErrors )//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
{
	NVDump('O', 'C', "00", L"HOST", L"HostConfig");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_HostConfigProc]\n"));

	//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
	if ( nConfigOption == 1)// Disable인 경우 Configuration 수행 안함
	{
		m_pDevCmn->HostOpenRetryTime = 0;
		m_pDevCmn->HostOpenFlag = TRUE;
		return TRUE;
	}
	//end of [#2000] 

	int	nReversalFlag;

	nReversalFlag = MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG);

	if ((nReversalFlag != REVERSAL_CONF) &&
		(nReversalFlag != REVERSAL_CONF_TRAN))
	{
		fnAPP_InitializeDS();
	}

	m_pDevCmn->TranResult = FALSE;
	m_pDevCmn->TranStatus = TRAN_OPEN;
	TranCode = TC_OPEN;

	// Communication with Host
	P_NH_NOR_Transaction(bShowScreen);
	
	// Check Result
	if (m_pDevCmn->TranResult == TRUE)
	{
		//if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) != MSG_TRITON_TYPE)
		if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) != MSG_TRITON_TYPE) || ( m_HostConfig == HC_DUALHOST ) ) // [#2185] US Justin 2013.05.08 Dual Host DCC
		{
			int nWorkingRes;
			CString strOperatorAction;

			// [#2185] US Justin 2013.05.08 Dual Host DCC
			//nWorkingRes = LIB_LoadWorkingKey();
			int nMKeyName = MASTERKEY_ATM;
			if (m_HostConfig == HC_DUALHOST)		nMKeyName = MASTERKEY_DUALHOST;
			else if (m_HostConfig == HC_LIBERTYX)	nMKeyName = MASTERKEY_LIBERTYX;

			nWorkingRes = LIB_LoadWorkingKey(nMKeyName);
			// End of [#2185]

			//[#2010] SOOK 2011.01.14 WORKING KEY LOAD 실패 시 버그 FIX
			if (nWorkingRes == T_OK)
			{
				strOperatorAction.Format(L"Download Working Key - Success");

				if (m_pDevCmn->nPINErrorFlag == WORKINGKEY_INIT)
					m_pDevCmn->nPINErrorFlag = WORKINGKEY_DOWNLOAD_SUCCESS;	// [#2010] SOOK 2011.01.14 WORKING KEY LOAD 실패 시 버그 FIX	[#2134] NH KSK 2012.05.07
			}
			else
			{
				strOperatorAction.Format(L"Download Working Key - Fail(%d)", nWorkingRes);

				if (m_pDevCmn->nPINErrorFlag == WORKINGKEY_INIT)
					m_pDevCmn->nPINErrorFlag = WORKINGKEY_DOWNLOAD_FAIL;	// [#2134] NH KSK 2012.05.07
			}
			m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, strOperatorAction);
			//end of [#2010]
		}

// KSK 2010.06.18 Configuration 성공 시에는 error clear하지 않도록 수정
// 사유 : Mac Error 발생 시 Configuration 시도 시 Transaction중의 Mac Error Code가 Clear되는 Bug를 수정하기 위함
//		P_NH_NOR_CleanUp();

		// KSK 2010.06.18 Bug fix Configuration 성공 시 HostOpen Flag True로 설정
		m_pDevCmn->HostOpenRetryTime = 0;
		m_pDevCmn->HostOpenFlag = TRUE;

		return TRUE;
	}

	P_NH_NOR_DisplayError();

	NVDump('F', 'C', "79", L"", L"1:CONFIG-NG");

	P_NH_NOR_CleanUp(clearNetworkErrors);

	return FALSE;
}

BOOL CTranCmn::BIZ_HostConfigExtendedProc(int nTranStatus, int nTracCode)
{
	NVDump('O', 'C', "00", L"HOST", L"HostConfigExtended");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_HostConfigExtendedProc]\n"));
	
	fnAPP_InitializeDS();

	// [#2449] US Justin 2016.11.08
	#if (APP_PAYDIANT_CCA)
	if( nTracCode == TC_EXTENDED_ADDSVC_CONFIG)
		m_PayPalCCA.ResetDownloadCredential();
	#endif
	// End of [#2449]

	// Setting Transaction Type
	m_pDevCmn->TranResult = FALSE;
	m_pDevCmn->TranStatus = nTranStatus;
	TranCode = nTracCode;
	
	// Communication with Host
	P_NH_NOR_Transaction(TRUE);

	P_NH_NOR_CleanUp();

	if (m_pDevCmn->TranResult == FALSE)
		return FALSE;
	
	// [#2449] US Justin 2016.11.08
	#if (APP_PAYDIANT_CCA)
	if( nTracCode == TC_EXTENDED_ADDSVC_CONFIG)
	{
		if( m_PayPalCCA.AssignPaydiantCredential(UPDATE_CREDENTIAL_COMPLETE, L"") == TRUE )
		{
			// Enable the setting <=== If a host uses the extened configuration request initiator....
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE, ENABLE);
		}
	}
	#endif
	// End of [#2449]

	return TRUE;
}

// [#RWC6-57] US William 2019.09.18 PAI MCCP Key presence
/**
 * Performs an MCCP test transaction, causing the symmetric key to install
 */
BOOL CTranCmn::BIZ_MCCPSymmetricKeyConfig()
{
	NHDEBUG(DBG_CALL, (L"Downloading MCCP symmetric key\r\n"));
	// Set test transaction data
	m_Pin4.SetTestTransactionSettings();

	// Setting Transaction Type
	TranCode = TC_PIN4;

	// Communication with Host
	BIZ_RETURN configResult = P_NH_NOR_Transaction(TRUE);

	P_NH_NOR_CleanUp();

	if (configResult != RES_OK)
	{
		NHERROR((L"MCCP: Symmetric key upload failed\r\n"));
		NVDump('F', '4', "01", L"", L"MCCP_Download_Fail");
	}

	return true;
}
// end of [#RWC6-57]

BOOL CTranCmn::BIZ_AddCashProc(int nActuser)
{
	NVDump('O', 'C', "00", L"HOST", L"AddCash");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_AddCash]\n"));

	CString 	strTemp, strInitialCntData, strCashCntData;

	// ADD CASH
	//---------------------------------------------------------------
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)	// [#2270] AU KSK 2014.05.26 Note Counting Enable인 경우만 수행
	{
		strInitialCntData	= Int2Asc(m_pDevCmn->RejectCSTCnt);
		strCashCntData		= strInitialCntData;

		if (m_pDevCmn->CSTCnt >= CDU_CST_1)
		{
			strInitialCntData	+= "," + Int2Asc(m_pDevCmn->FirstCSTInitCash);
			strCashCntData		+= "," + Int2Asc(m_pDevCmn->FirstCSTSetCash);
		}
		if (m_pDevCmn->CSTCnt >= CDU_CST_2)
		{
			strInitialCntData	+= "," + Int2Asc(m_pDevCmn->SecondCSTInitCash);
			strCashCntData		+= "," + Int2Asc(m_pDevCmn->SecondCSTSetCash);
		}
		if (m_pDevCmn->CSTCnt >= CDU_CST_3)
		{
			strInitialCntData	+= "," + Int2Asc(m_pDevCmn->ThirdCSTInitCash);
			strCashCntData		+= "," + Int2Asc(m_pDevCmn->ThirdCSTSetCash);
		}
		if (m_pDevCmn->CSTCnt >= CDU_CST_4)
		{
			strInitialCntData	+= "," + Int2Asc(m_pDevCmn->FourthCSTInitCash);
			strCashCntData		+= "," + Int2Asc(m_pDevCmn->FourthCSTSetCash);
		}

		// [Win32-fixes] US ryan.payton 2023.09.13 Need a proper way to add cash to CDU for Win32
		#ifdef UNDER_CE
			m_pDevCmn->fnCDU_AddNumberOfCash(strInitialCntData, strCashCntData);
		#endif

		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_CDU);
	}
	//---------------------------------------------------------------

	if (nActuser == 0 && m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
		LIB_PrintADDCash(DEV_SPR);			// by OP

	return TRUE;
}

BOOL CTranCmn::BIZ_DayTotalProc(BOOL bShow, BOOL bReal)
{
	NVDump('O', 'C', "00", L"HOST", L"DayTotal");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_DayTotalProc]\n"));

	fnAPP_InitializeDS();

	// Setting Transaction Type
	if (bReal == TRUE)
	{
		m_pDevCmn->TranResult = FALSE;
		m_pDevCmn->TranStatus = TRAN_TOTAL;
		TranCode = TC_TOTAL;
	}
	else
	{
		m_pDevCmn->TranResult = FALSE;
		m_pDevCmn->TranStatus = TRAN_TRIALTOTAL;
		TranCode = TC_TRIALTOTAL;
	}

	// Communication with Host
	P_NH_NOR_Transaction(bShow);
	P_NH_NOR_CleanUp();

	// NHA 요청사항 Trial Day Total Fail 시 명세표 인자 & Journal 저장 하도록 수정
	//if (m_pDevCmn->TranResult == FALSE)
	//	return FALSE;
	if ((bReal == TRUE) && (m_pDevCmn->TranResult == FALSE))
		return FALSE;

	// [#2185] US Justin 2013.05.14 Dual Host DCC
	BOOL bCutPaper = TRUE;
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) == ENABLE)
		bCutPaper = FALSE;
	// End of [#2185]

	// Print Result
	if (m_pDevCmn->OpenKey == TRUE)
		LIB_DayTotal(DEV_JPR);							// by REMOTE	(ATM & Additional Totals)
	else		
		LIB_DayTotal((DEV_SPR | DEV_JPR), bCutPaper);	// by OP		(ATM & Additional Totals)	// [#2185] US Justin 2013.05.14 Add Cut Option

	// [#2185] US Justin 2013.05.14 Dual Host DCC
	//#if (APP_CUSTOM_PAI)		// [#2499] US Justin 2017.08.21 Enable Dual Host DCC for all customers.
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) == ENABLE)
	{
		NVDump('O', 'C', "00", L"HOST", L"DCCDayTotal");
		NHDEBUG(DBG_CALL, (L"[CTranCmn::DCC_DayTotalProc]\n"));

		fnAPP_InitializeDS();

		m_HostConfig = HC_DUALHOST;
		m_pDevCmn->TranResult = FALSE;
		if (bReal == TRUE)
		{
			m_pDevCmn->TranStatus = TRAN_TOTAL;
			TranCode = TC_TOTAL;
		}
		else
		{
			m_pDevCmn->TranStatus = TRAN_TRIALTOTAL;
			TranCode = TC_TRIALTOTAL;
		}
		P_NH_NOR_Transaction(bShow);
		P_NH_NOR_CleanUp();

		if (m_pDevCmn->OpenKey == TRUE)
			LIB_DayTotal_DHDCC(DEV_JPR);			// by REMOTE	(Dual Host DCC)
		else		
			LIB_DayTotal_DHDCC((DEV_SPR | DEV_JPR));// by OP		(Dual Host DCC)
	}
	//#endif
	// End of [#2185]

	// update status
	if (bReal == TRUE)
	{
		m_pDevCmn->fnCDU_SumClear();
		
		//#if (APP_CUSTOM_PAI)			// [#2499] US Justin 2017.08.21 Enable Dual Host DCC for all customers.
			m_pDevCmn->fnCDU_SumClear(2);			// [#2185] US Justin 2013.05.14 Dual Host DCC
		//#endif

		#if (APP_PIN4_CASHPICKUP)					//  [#2471] US Justin 2017.02.01 Enable Pin4 to all customers.
			if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE)==ENABLE) && (m_Pin4.m_bPin4Available==TRUE) )			// [#2405] US Justin 2016.03.21	HalCash Online
				m_Pin4.ResetTotal();
		#endif

		#if (APP_POPMONEY)							//  [#2471] US Justin 2017.02.01 Enable Popmoney to all customers.
			if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_ENABLE)==ENABLE)											// [#2442] US Justin 2016.08.25 Popmoney Day Total
				m_PopMoneyData.ResetTotal();
		#endif

		// [#2445] US Justin 2016.09.27 Just.Cash
		#if (APP_JUST_CASH)
			if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE)) && (m_JustCashData.m_bJustCashRegistered==TRUE) )
				m_JustCashData.ResetTotal( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE) );
		#endif
		// End of [#2445]

		#if (APP_B4U)
			if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_B4U_ENABLED)==ENABLE)
				m_B4U->ResetTotal();
		#endif

		// [#2446] US Justin 2016.09.30 Paypal CCA
		#if (APP_PAYDIANT_CCA)
			if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE)==ENABLE) && (m_PayPalCCA.m_bPaypalCCAAvailable==TRUE) && (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE))
				m_PayPalCCA.ResetTotal();
		#endif
		// End of [#2446]
	}
	return TRUE;
}

#if (APP_DIGITALMINT)
BOOL CTranCmn::H_NH_DM_IsAtmModeAvailable(DMConfiguration config)
{
	return config.IsAtmConfigured();
}

BOOL CTranCmn::H_NH_DM_IsSidecarModeAvailable(DMConfiguration config)
{
#ifdef UNDER_CE
	bool bAvailable = false;
#else
	bool bAvailable = config.IsSidecarConfigured();
#endif

	// Ensure DM is configured for side car && side is enabled
	if ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE) && config.IsSidecarConfigured())
	{
		bAvailable = true;

		// Update device status
		m_pDevCmn->fnAPL_CheckDevice();

		// Check BNA
		bAvailable &= m_pDevCmn->fnBNA_GetAggregateDeviceStatus() == NORMAL;

		// Check BCR
		bAvailable &= m_pDevCmn->fnBCR_GetDeviceStatus() == NORMAL;
	}

	return bAvailable;
}

BIZ_RETURN CTranCmn::H_NH_DM_ReportAtmState(DMConfiguration config, bool forceUpdate)
{
	BIZ_RETURN nRes = RES_OK;
	bool bSendUpdate = forceUpdate;

	if (config.IsAtmConfigured())
	{
		m_dmAtmStateMap.HasCameraState = false;
		m_dmAtmStateMap.HasBillState = false;
		m_dmAtmStateMap.HasScannerState = false;
		m_dmAtmStateMap.HasOnline = true;

		// DMTODO: Check ATM functionality, PinPad, etc.
		bool bOnlineStatus = true;

		m_dmAtmStateMap.Online = bOnlineStatus;

		if (!bSendUpdate)
		{
			// Time will be zero if we have never sent a message
			if (m_dmLastAtmUpdate == 0)
			{
				bSendUpdate = true;
			}
			else
			{
				COleDateTime timeNow = COleDateTime::GetCurrentTime();
				COleDateTimeSpan timeDiff = timeNow - m_dmLastAtmUpdate;

				// Has max time elapsed?
				if ((UINT)timeDiff.GetTotalSeconds() >= SIDECAR_MAX_UPDATE_DURATION_SECONDS)
				{
					bSendUpdate = true;
				}
			}
		}

		if (bSendUpdate)
		{
			DMKioskStateRequest request;
			request.StateMap = m_dmAtmStateMap;

			if (!m_DMService->ReportKioskState(request, DMTXNMODE_ATM))
			{
				nRes = RES_NG;
			}
			else
			{
				// Update our communication time
				m_dmLastAtmUpdate = COleDateTime::GetCurrentTime();
			}
		}
	}

	return nRes;
}

BIZ_RETURN CTranCmn::H_NH_DM_ReportSidecarState(DMConfiguration config, bool forceUpdate)
{
	BIZ_RETURN nRes = RES_OK;
	bool bSendUpdate = forceUpdate;

#ifdef UNDER_CE
	bool isSidecarSupported = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE;
#else
	bool isSidecarSupported = true;
#endif

	if (isSidecarSupported && config.IsSidecarConfigured())
	{
		m_dmSidecarStateMap.HasCameraState = false;
		m_dmSidecarStateMap.HasBillState = true;
		m_dmSidecarStateMap.HasScannerState = true;
		m_dmSidecarStateMap.HasOnline = true;

#ifdef UNCER_CE
		// Check BNA
		bool bBnaStatus = m_pDevCmn->fnBNA_GetAggregateDeviceStatus() == NORMAL;

		// Check BCR
		bool bBcrStatus = m_pDevCmn->fnBCR_GetDeviceStatus() == NORMAL;
#else
		bool bBnaStatus = true;
		bool bBcrStatus = true;
#endif

		// Check ATM Status
		bool bOnlineStatus = bBnaStatus && bBcrStatus;

		// Check for state changes
		if (m_dmSidecarStateMap.BillState != bBnaStatus || forceUpdate)
		{
			m_dmSidecarStateMap.BillState = bBnaStatus;
			bSendUpdate = true;
		}

		if (m_dmSidecarStateMap.ScannerState != bBcrStatus || forceUpdate)
		{
			m_dmSidecarStateMap.ScannerState = bBcrStatus;
			bSendUpdate = true;
		}

		if (m_dmSidecarStateMap.Online != bOnlineStatus || forceUpdate)
		{
			m_dmSidecarStateMap.Online = bOnlineStatus;
			bSendUpdate = true;
		}

		if (!bSendUpdate)
		{
			// Time will be zero if we have never sent a message
			if (m_dmLastSidecarUpdate == 0)
			{
				bSendUpdate = true;
			}
			else
			{
				COleDateTime timeNow = COleDateTime::GetCurrentTime();
				COleDateTimeSpan timeDiff = timeNow - m_dmLastSidecarUpdate;

				// Has max time elapsed?
				if ((UINT)timeDiff.GetTotalSeconds() >= SIDECAR_MAX_UPDATE_DURATION_SECONDS)
				{
					bSendUpdate = true;
				}
			}
		}

		if (bSendUpdate)
		{
			DMKioskStateRequest request;
			request.StateMap = m_dmSidecarStateMap;

			if (!m_DMService->ReportKioskState(request, DMTXNMODE_SIDECAR))
			{
				nRes = RES_NG;
			}
			else
			{
				// Update our communication time
				m_dmLastSidecarUpdate = COleDateTime::GetCurrentTime();
			}
		}
	}

	return nRes;
}

BIZ_RETURN CTranCmn::H_NH_DM_ReportKioskEvent(DMConfiguration config, DMKioskEventCode eventCode)
{
	BIZ_RETURN nRes = RES_OK;

#ifdef UNDER_CE
	bool isSidecarSupported = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE;
#else
	bool isSidecarSupported = true;
#endif

	if (isSidecarSupported && config.IsSidecarConfigured())
	{
		DMKioskEventRequest request;
		request.Event = eventCode;

		if (!m_DMService->ReportKioskEvent(request, DMTXNMODE_SIDECAR))
		{
			nRes = RES_NG;
		}
	}

	return nRes;
}
#endif

BOOL CTranCmn::BIZ_CassetteTotalProc(BOOL bReal, int nActUser)
{
	NVDump('O', 'C', "00", L"HOST", L"CST Total");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_CassetteTotalProc] bReal(%d)\n", bReal));

	fnAPP_InitializeDS();

	// Setting Transaction Type
	if (bReal == TRUE)
	{
		m_pDevCmn->TranResult = FALSE;
		m_pDevCmn->TranStatus = TRAN_CSTTOTAL;
		TranCode = TC_CSTTOTAL;
	}
	else
	{
		m_pDevCmn->TranResult = FALSE;
		m_pDevCmn->TranStatus = TRAN_TRIALCSTTOTAL;
		TranCode = TC_TRIALCSTTOTAL;
	}

	m_pDevCmn->TranResult = TRUE;

	// Print Result
	LIB_CashPrintResult(nActUser);

	if (bReal == TRUE)
	{
		CString strCashCntData	= L"0";	// Reject 매수 초기화

		if (m_pDevCmn->CSTCnt >= CDU_CST_1)
			strCashCntData		+= L",0";
		if (m_pDevCmn->CSTCnt >= CDU_CST_2)
			strCashCntData		+= L",0";
		if (m_pDevCmn->CSTCnt >= CDU_CST_3)
			strCashCntData		+= L",0";
		if (m_pDevCmn->CSTCnt >= CDU_CST_4)
			strCashCntData		+= L",0";

		m_pDevCmn->fnCDU_SetNumberOfCash(strCashCntData);
		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_CDU);

		m_pDevCmn->fnCDU_ClearDispenseInfo();

		// [#2270] AU KSK 2014.05.26
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 1)
		{
			// Note counting이 Disable시에는 AP에서 관리하는 Data를 Clear함
			for(int i=0; i<CDU_MAX_CST_COUNT; i++)
			{
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_INITIALCOUNT_AP + i, 0);
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP + i, 0);
			}
		}
		// end of [#2270]
	}
	return TRUE;
}

// [#GLDV-3005] US Kook 2022.03.28 Support Side Car
BOOL CTranCmn::BIZ_CashInCassetteTotalProc_Demo(BOOL bReal, int nActUser)
{
	NVDump('O', 'C', "00", L"HOST", L"CashInTotal");
	NHDEBUG(DBG_CALL, (L" bReal(%d)\n", bReal));

	fnAPP_InitializeDS();

	// Setting Transaction Type
	if (bReal == TRUE)
	{
		m_pDevCmn->TranResult = FALSE;
		m_pDevCmn->TranStatus = TRAN_CASHIN_CSTTOTAL;
		TranCode = TC_CASHIN_CSTTOTAL;
	}
	else
	{
		m_pDevCmn->TranResult = FALSE;
		m_pDevCmn->TranStatus = TRAN_TRIAL_CASHIN_CSTTOTAL;
		TranCode = TC_TRIAL_CASHIN_CSTTOTAL;
	}

	m_pDevCmn->TranResult = TRUE;

	m_pDevCmn->fnBNA_GetCashUnitInformation();	// update cashin counts
	LIB_PrintCashInTotal_Demo(nActUser);		// Print Result

	if (bReal == TRUE)
	{
#if (APP_JUST_CASH)
		// Notify Just.Cash with total and that total was reset
		TranCode = TC_JUSTCASH;
		m_JustCashData.m_strVaultBalance = Int2Asc(m_pDevCmn->fnBNA_GetTotalCount());
		m_JustCashData.m_nTransactionStep = JUSTCASH_DIGITAL_CUR_VAULT_BALANCE;

		P_NH_NOR_Transaction(FALSE);
#endif

#if (APP_DIGITALMINT)
		H_NH_DM_ReportKioskEvent(m_DMConfig, DMEC_CashCollection);
#endif

		m_pDevCmn->fnBNA_ClearCashInInfo();			// Clear Cash In Count
		m_pDevCmn->fnBNA_GetCashUnitInformation();	// update cashin counts
	}

	return TRUE;
}


// end of [#GLDV-3005]

BOOL CTranCmn::BIZ_DenominationProc()
{
	NVDump('O', 'C', "00", L"HOST", L"Denomination");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_DenominationProc]\n"));

	CStringArray	strTempArray;
	CString			strTemp;
	CString			strTemp2;
	int				nArrOldDeno[4];		// [#2220] AU KMK 2014.02.07 Set Denomination 명세표

	fnAPP_InitializeDS();

	// Setting Transaction Type
	m_pDevCmn->TranResult = FALSE;
	m_pDevCmn->TranStatus = TRAN_SETDENO;
	TranCode = TC_SETDENO;

	m_pDevCmn->FirstCSTValue = m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_1);
	m_pDevCmn->SecondCSTValue = m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_2);
	m_pDevCmn->ThirdCSTValue = m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_3);
	m_pDevCmn->FourthCSTValue = m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_4);

	// [#2220] AU KMK 2014.02.07 Set Denomination 명세표
	nArrOldDeno[0] = m_pDevCmn->FirstCSTValue;
	nArrOldDeno[1] = m_pDevCmn->SecondCSTValue;
	nArrOldDeno[2] = m_pDevCmn->ThirdCSTValue;
	nArrOldDeno[3] = m_pDevCmn->FourthCSTValue;
	// end of [#2220]

	m_pDevCmn->FirstCSTSetCash = 0;
	m_pDevCmn->SecondCSTSetCash = 0;
	m_pDevCmn->ThirdCSTSetCash = 0;
	m_pDevCmn->FourthCSTSetCash = 0;

	// DENOMINATION 장애시에도 가능하도록 수정
	if (m_pDevCmn->fnCDU_SetValueOfCash(strCSTValues) == WFS_SUCCESS)
		m_pDevCmn->TranResult = TRUE;
	else
		m_pDevCmn->TranResult = FALSE;

	strCSTValues = "";

	m_pDevCmn->FirstCSTValue	= m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_1);
	m_pDevCmn->SecondCSTValue	= m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_2);
	m_pDevCmn->ThirdCSTValue	= m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_3);
	m_pDevCmn->FourthCSTValue	= m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_4);

	// [#146] KSK 2008.04.20 Add Currency ID
	// CURRENCY ID는 MWI에서 무조건 CST1에 대해서만 값을 RETURN해줌
	m_pDevCmn->m_strCurrencyID = m_pDevCmn->fnCDU_GetCurrencyID(CDU_CST_1);
	m_pDevCmn->fnCDU_SetCurrencyID(m_pDevCmn->m_strCurrencyID);
	m_pDevCmn->fnAPL_CheckDeviceAction(DEV_CDU);

	// Error시 Print 안함.
	if (m_pDevCmn->TranResult)
// [#2220] AU KMK 2014.02.06 호주는 Set Denomination 명세표 포맷을 저널과 동일하게 가져감
#if (AU_VERSION)
		LIB_DenominationPrintResult(nArrOldDeno);
#else
// end of [#2220]
		LIB_CashPrintResult(0);
// [#2220]
#endif
// end of [#2220]
	return TRUE;
}

BOOL CTranCmn::BIZ_HealthCheckProc(BOOL bShowScreen)
{
	NVDump('O', 'C', "00", L"HOST", L"HealthCheck");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_NOR_Healthcheck]\n"));
	
	fnAPP_InitializeDS();

	// Setting Transaction Type
	if (MemGetInt(_MEM_FLD_APP_HOSTCMD, _MEM_VAR_APP_HC15DetailStatus) == 1)
	{
		m_pDevCmn->TranResult = FALSE;
		m_pDevCmn->TranStatus = TRAN_DETAILHEALTHCHK;
		TranCode = TC_DETAILHEALTHCHK;
	}
	else
	{
		m_pDevCmn->TranResult = FALSE;
		m_pDevCmn->TranStatus = TRAN_HEALTHCHK;
		TranCode = TC_HEALTHCHK;
	}

	MemSetInt(_MEM_FLD_APP_HOSTCMD, _MEM_VAR_APP_HC15DetailStatus, 0);		// Bug Fix KSK 2009.06.19
//	m_pDevCmn->fnAPL_SetHealthCheckTimer();	// [#583] NH KSK 2009.11.24 Sec로 변경됨에 따라 Refresh Time 설정 위치 수정 (송신 후)

	// Communication with Host
	P_NH_NOR_Transaction(bShowScreen);

	P_NH_NOR_CleanUp();

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME	: CTranCmn
 FUNCTION NAME	: LIB_IsReversalCondition()
 RETURN TYPE	: TRUE  : Reversal을 수행한다.
 				  FALSE : Reversal을 수행하지 않는다.
 PARAMETER		: 
 DESCRIPTION	: 현재 Transaction이 Reversal 조건시 Reversal을 해야 하는지
 				  판단하는 함수 이다.
-------------------------------------------------------------------*/
BOOL CTranCmn::LIB_IsReversalCondition()
{
	BOOL bRet = FALSE;
	// [#2350] US Justin 2015.06.19 Add POP Money
	// [#2445] US Justin 2016.09.27 Add JustCash
	// [#2446] US Justin 2016.09.30 Add Paypal CCA
	// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
	// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
	if (TranCode == TC_WITHDRAWAL		|| TranCode == TC_POPMONEY	||
		TranCode == TC_JUSTCASH			|| TranCode == TC_PAYPALCCA	||
		TranCode == TC_JUSTCASH_BITCOIN	|| TranCode == TC_PIN4		||
		TranCode == TC_DIGITALMINT)
		bRet = TRUE;

#if (AU_VERSION || MX_VERSION)
	else if (TranCode == TC_INQUIRY)
		bRet = TRUE;
#endif

	return bRet;
}