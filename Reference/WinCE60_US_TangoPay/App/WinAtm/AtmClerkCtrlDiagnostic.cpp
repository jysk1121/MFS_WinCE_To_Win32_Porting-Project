/////////////////////////////////////////////////////////////////////////////
// AtmClerkCtrlDiagnostic.cpp : Implementation of the CWinAtmCtrl ActiveX Control class.
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MainFrm.h"
#include "AtmClerkCtrl.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

struct PARAM_OPR_SP
{
	int				nDvcType;
	int				nOprType;
	CDevCmn*		pDevCmn;
};

struct PARAM_OPR_VDM
{
	int				nDvcType;
	int				nOprType;
	CNHCtrlThread*	pDvcDiag;
	HANDLE			hCompleteEvent;	
	int				nResult;
};

static UINT ThreadSpOperator(__in LPVOID lpParameter)
{
	PARAM_OPR_SP* param = (PARAM_OPR_SP*)lpParameter;
	NHDEBUG(DBG_CALL, (L"[ThreadSpOperator] [%x] Thread Executing [%s]\n", param->nDvcType, (param->nOprType == ON) ? _T("Initialize()") : _T("Deinitialize()")));


	switch (param->nDvcType)
	{
	case DEV_SPR:
		if (param->nOprType == ON)		return param->pDevCmn->fnSPR_Initialize();//fnAPL_InitializeDevice(param->nDvcType);
		else							return param->pDevCmn->fnSPR_Deinitialize();

		break;

	case DEV_MCU:
		if (param->nOprType == ON)		return param->pDevCmn->fnMCU_Initialize();//fnAPL_InitializeDevice(param->nDvcType);
		else							return param->pDevCmn->fnMCU_Deinitialize();

		break;

	case DEV_CDU:
		if (param->nOprType == ON)		return param->pDevCmn->fnCDU_Initialize(0);//fnAPL_InitializeDevice(param->nDvcType);
		else							return param->pDevCmn->fnCDU_Deinitialize();

		break;

	case DEV_RFID:
		if (param->nOprType == ON)		return param->pDevCmn->fnRFID_Initialize();
		else							return param->pDevCmn->fnRFID_Deinitialize();

		break;

	case DEV_BCR:
		if (param->nOprType == ON)		return param->pDevCmn->fnBCR_Initialize();
		else							return param->pDevCmn->fnBCR_Deinitialize();

		break;

	case DEV_BNA:
		if (param->nOprType == ON)		return param->pDevCmn->fnBNA_Initialize();
		else							return param->pDevCmn->fnBNA_Deinitialize();

		break;

	default:
		break;
	}

	return 0;
}

static UINT ThreadVdmOperator(__in LPVOID lpParameter)
{
	PARAM_OPR_VDM* param = (PARAM_OPR_VDM*)lpParameter;
	NHDEBUG(DBG_CALL, (L"[ThreadVdmOperator] [%S] Thread Executing [%s]\n", GET_DEVNAME_BY_ID(param->nDvcType), (param->nOprType == ON) ? _T("Open()") : _T("Close()")));

	// return error (-1) when event handle is not prepared
	if (param->hCompleteEvent == NULL)
	{
		NHDEBUG(DBG_CALL, (L"[ThreadVdmOperator] [%S] hCompleteEvent is NULL! \n", GET_DEVNAME_BY_ID(param->nDvcType)));

		param->nResult = -1;
		return -1;
	}

	// return error (-2) when device pointer is not prepared
	if (param->pDvcDiag == NULL)
	{
		NHDEBUG(DBG_CALL, (L"[ThreadVdmOperator] [%S] pDvcDiag is NULL! \n", GET_DEVNAME_BY_ID(param->nDvcType)));

		param->nResult = -2;
		return -2;
	}

	int bResult = -1;
	switch (param->nDvcType)
	{
	case DEV_SPR:
		if (param->nOprType == ON)		bResult = ((CDiagSpr*)param->pDvcDiag)->Open();
		else							bResult = ((CDiagSpr*)param->pDvcDiag)->Close();		
		break;

	case DEV_MCU:
		if (param->nOprType == ON)		bResult = ((CDiagCard*)param->pDvcDiag)->Open();
		else							bResult = ((CDiagCard*)param->pDvcDiag)->Close();
		break;

	case DEV_CDU:
		if (param->nOprType == ON)		bResult = ((CDiagCdu*)param->pDvcDiag)->Open();
		else							bResult = ((CDiagCdu*)param->pDvcDiag)->Close();
		break;

	case DEV_RFID:
		if (param->nOprType == ON)		bResult = ((CDiagRfid*)param->pDvcDiag)->Open();
		else							bResult = ((CDiagRfid*)param->pDvcDiag)->Close();
		break;

	case DEV_BCR:
		if (param->nOprType == ON)		bResult = ((CDiagBcr*)param->pDvcDiag)->Open();
		else							bResult = ((CDiagBcr*)param->pDvcDiag)->Close();
		break;

	case DEV_BNA:
		if (param->nOprType == ON)		bResult = ((CDiagBna*)param->pDvcDiag)->Open();
		else							bResult = ((CDiagBna*)param->pDvcDiag)->Close();
		break;

	default:
		NHDEBUG(DBG_CALL, (L"[ThreadVdmOperator] UNKNOWN DEVICE!\n"));
		break;
	}

	// 'SUCCESS (0)' is only guaranteed when it is 'Open()'.
	// 'Close()' doesn't guarantee return of 'SUCCESS (0)' since some device returns it as 'IO_COMPLETED (1)'... 
	NHDEBUG(DBG_CALL, (L"[ThreadVdmOperator] [%S] %s Result:[%d] \n", GET_DEVNAME_BY_ID(param->nDvcType), (param->nOprType == ON) ? _T("Open()") : _T("Close()"), bResult));

	param->nResult = bResult;
	SetEvent(param->hCompleteEvent);

	return bResult;
}

int CMainFrame::ClerkDiagnosticsDeviceHandler(int pOnOff)
{
	CString strTemp;

	NHDEBUG(1, (_T("[VDM] %s DEVICES USING MULTI-THREAD ... \n"), (pOnOff == ON) ? _T("OPENING") : _T("CLOSING")));
	ClerkNoticeToUser(T_PROCESS_WAIT);

	if (pOnOff == ON)
	{
		m_bDiagSPROpen = FALSE;
		m_bDiagMCUOpen = FALSE;
		m_bDiagCDUOpen = FALSE;

		// optional devices
		m_bDiagRFIDOpen = FALSE;
		m_bDiagBCROpen = FALSE;
		m_bDiagBNAOpen = FALSE;
	}

	CWinThread *pThVdmOp[VDM_MULTI_COUNT] = {0};
	PARAM_OPR_VDM paramVdm[VDM_MULTI_COUNT];

	// begin threads
	for (int nDvcIdx=0; nDvcIdx<VDM_MULTI_COUNT; nDvcIdx++)
	{
		// skip optional devices
		if (GET_DVCTYPE(nDvcIdx) == DEV_RFID && (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) != ENABLE)
			|| GET_DVCTYPE(nDvcIdx) == DEV_BCR && (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) != ENABLE)
			|| GET_DVCTYPE(nDvcIdx) == DEV_BNA && (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) != ENABLE))
		{
			NHDEBUG(1, (_T("[VDM] [%S] Skipping unconfigured devices ... \n"), GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx)) ));
			continue;
		}

		NHDEBUG(1, (_T("[VDM] [%S] Thread Begins ... \n"), GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx)) ));
		paramVdm[nDvcIdx].nOprType = pOnOff;
		paramVdm[nDvcIdx].nDvcType = GET_DVCTYPE(nDvcIdx);
		paramVdm[nDvcIdx].pDvcDiag = GET_DVCDIAG(nDvcIdx);
		paramVdm[nDvcIdx].hCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		paramVdm[nDvcIdx].nResult = -1;

		pThVdmOp[nDvcIdx] = AfxBeginThread(ThreadVdmOperator, &paramVdm[nDvcIdx]);

		if (pThVdmOp[nDvcIdx] == NULL)
		{
			NHDEBUG(1, (_T("[VDM] [%S] Failed to make thread! \n"), GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx)) ));
			strTemp.Format(L"%S_TH_FAIL(%d)", GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx)), GetLastError());
			NVDump('F', 'A', "11", L"", strTemp);
		}
	}

	// wait their work completed, and close handles
	for (int nDvcIdx=0; nDvcIdx<VDM_MULTI_COUNT; nDvcIdx++)
	{
		// skip optional devices
		if (GET_DVCTYPE(nDvcIdx) == DEV_RFID && (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) != ENABLE)
			|| GET_DVCTYPE(nDvcIdx) == DEV_BCR && (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) != ENABLE)
			|| GET_DVCTYPE(nDvcIdx) == DEV_BNA && (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) != ENABLE))
		{
			NHDEBUG(1, (_T("[VDM] [%S] Skipping unconfigured device ... \n"), GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx)) ));
			continue;
		}

		if (pThVdmOp[nDvcIdx] == NULL)
		{
			NHDEBUG(1, (_T("[VDM] [%S] Thread not found! \n"), GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx)) ));

			// Devices could be opened even thread creation was failed. ("TEST" button will try to open the device again.)
			// We should close devices clearly regardless of thread status. If not, WinATM will crash when SP opens.
			if (pOnOff == OFF)
			{
				NHDEBUG(1, (_T("[VDM] [%S] Close it manually. \n"), GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx)) ));
				switch (GET_DVCTYPE(nDvcIdx))
				{
				case DEV_SPR:	((CDiagSpr*)	GET_DVCDIAG(nDvcIdx))->Close();		break;
				case DEV_MCU:	((CDiagCard*)	GET_DVCDIAG(nDvcIdx))->Close();		break;
				case DEV_CDU:	((CDiagCdu*)	GET_DVCDIAG(nDvcIdx))->Close();		break;
				case DEV_RFID:	((CDiagRfid*)	GET_DVCDIAG(nDvcIdx))->Close();		break;
				case DEV_BCR:	((CDiagBcr*)	GET_DVCDIAG(nDvcIdx))->Close();		break;
				case DEV_BNA:	((CDiagBna*)	GET_DVCDIAG(nDvcIdx))->Close();		break;
				default:
					break;
				}
			}
			continue;
		}

		DWORD dwWaitResult = WaitForSingleObject(paramVdm[nDvcIdx].hCompleteEvent, 120 * 1000);		// 4000W SPR consumes 44 seconds when it is not connected. set timeout 3x of it.

		NHDEBUG(1, (_T("[VDM] [%S] WaitForSingleObject : %d\n"), GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx)), dwWaitResult));
		strTemp.Format(L"%S_%s_WR:%d", GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx)), (pOnOff == ON) ? _T("OPEN") : _T("CLOSE"), dwWaitResult);
		NVDump('F', 'A', "11", L"", strTemp);

		switch (dwWaitResult) 
		{
		// Thread object was signaled
		case WAIT_OBJECT_0:
			NHDEBUG(1, (_T("[VDM] [%S] Thread ended.\n"), GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx))));
			if (paramVdm[nDvcIdx].nResult == WFS_SUCCESS							// 'Successfully opened' returns '0'
				|| (paramVdm[nDvcIdx].nResult == IO_COMPLETED && pOnOff == OFF) )	// 'Successfully closed' returns '0' or '1'
			{
				NHDEBUG(1, (_T("[VDM] [%S] Result : SUCCESS \n"), GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx))));

				if (GET_DVCTYPE(nDvcIdx) == DEV_SPR)		m_bDiagSPROpen = (pOnOff == ON) ? TRUE : FALSE;
				else if (GET_DVCTYPE(nDvcIdx) == DEV_MCU)	m_bDiagMCUOpen = (pOnOff == ON) ? TRUE : FALSE;
				else if (GET_DVCTYPE(nDvcIdx) == DEV_CDU)	m_bDiagCDUOpen = (pOnOff == ON) ? TRUE : FALSE;
				else if (GET_DVCTYPE(nDvcIdx) == DEV_RFID)	m_bDiagRFIDOpen = (pOnOff == ON) ? TRUE : FALSE;
				else if (GET_DVCTYPE(nDvcIdx) == DEV_BCR)	m_bDiagBCROpen = (pOnOff == ON) ? TRUE : FALSE;
				else if (GET_DVCTYPE(nDvcIdx) == DEV_BNA)	m_bDiagBNAOpen = (pOnOff == ON) ? TRUE : FALSE;
			}
			else
			{
				NHDEBUG(1, (_T("[VDM] [%S] Result : FAILURE \n"), GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx))));

				strTemp.Format(L"%S_%s_NG", GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx)), (pOnOff == ON) ? _T("OPEN") : _T("CLOSE"));
				NVDump('F', 'A', "11", L"", strTemp);
			}
			break;

		// An error occurred
		default:				
			NHDEBUG(1, (_T("[VDM] [%S] WaitForSingleObject failed (%d)\n"), GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx)), GetLastError()));

			strTemp.Format(L"%S_%s_NG(%d)", GET_DEVNAME_BY_ID(GET_DVCTYPE(nDvcIdx)), (pOnOff == ON) ? _T("OPEN") : _T("CLOSE"), GetLastError());
			NVDump('F', 'A', "11", L"", strTemp);
			break;
		}

		if (pThVdmOp[nDvcIdx] != NULL)
			CloseHandle(pThVdmOp[nDvcIdx]);
	}

	NHDEBUG(1, (_T("[VDM] %s DEVICES USING MULTI-THREAD ... DONE! \n"), (pOnOff == ON) ? _T("OPENING") : _T("CLOSING")));

	if (pOnOff == OFF)
	{
		delete m_pDiagSpr;	m_pDiagSpr = NULL;
		delete m_pDiagMcu;	m_pDiagMcu = NULL;
		delete m_pDiagCdu;	m_pDiagCdu = NULL;

		// optional devices;
		delete m_pDiagRfid;	m_pDiagRfid = NULL;
		delete m_pDiagBcr;	m_pDiagBcr = NULL;
		delete m_pDiagBna;	m_pDiagBna = NULL;
	}

	return 0;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDiagonsticsMain()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 821 PROC
-------------------------------------------------------------------*/
int	CMainFrame::ClerkDiagonsticsMain()
{
NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkDiagonsticsMain() \n")));

	int nErrorCode = 0;
	int nDeviceCmd;

	BOOL	bWait = TRUE;
	BOOL	bCancelSPR = FALSE;
	BOOL	bCancelCDU = FALSE;
	BOOL	bCancelAGING = FALSE;
	BOOL	bCancelIntial = FALSE;
	CString strDeviceError;

	ULONG	ulRequestNotes[4];
	ULONG	ulDispensedNotes[4];
	ULONG	ulTotalRejectedNotes;

	m_OpInfo.nProcLoofCnt = 0;
	m_OpInfo.nMsgClearCount = 0;
	m_OpInfo.nMoveProcReason = MOVE_PROC_SELECT;

	// [#585] NH KSK 2009.12.03
	m_OpInfo.m_strnetworkDestination = MemGetStr(_MEM_FLD_TCPIPINFO,_MEM_VAR_TCPIP_HOST1NAME);
	m_OpInfo.m_strPort	= Int2Asc(MemGetInt(_MEM_FLD_TCPIPINFO,_MEM_VAR_TCPIP_HOST1PORTNO));
	m_OpInfo.m_nSSLOption = MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_SSLENABLE);
	m_OpInfo.m_nSSLVersion = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSLVERSION);
	// end of [#585]

	while(TRUE)
	{
		// Clerk 화면 Display, Update 한다. 
		// Screen Input 발생 시 리턴된다.
		ClerkWaitScreenInput(TRUE);

		// Get Event String
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();
		SplitString(m_OpInfo.strReturn, SCR_RES_DELIMITER, m_OpInfo.strTempArray);

		NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame:: PRESSED KEY [%s],[%d] \n"), m_OpInfo.strReturn, m_OpInfo.strTempArray.GetSize()));

		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
			break;
		}
		else
		{
			/* Device Initialization */
			if (m_OpInfo.strReturn == L"INITIALIZE")			// F1
			{
				/* ENABLE PIN & FUNCTION KEY */
				m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);

				// Clear Device
				m_pDiagCdu->ClearDeviceError();
				m_pDiagSpr->ClearDeviceError();
				m_pDiagMcu->ClearDeviceError();

				// [#2325] NH KSK 2015.01.25
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)		// [#2325] NH KMK 2015.02.12 RFID DISABLE 시 AP Popup 대책
					m_pDiagRfid->ClearDeviceError();
				// end of [#2325]

				// [#GLDV-3005] US Kook 2022.07.18 Side Car
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
				{
					if (m_pDiagBcr != NULL)
						m_pDiagBcr->ClearError();
					if (m_pDiagBna != NULL)
						m_pDiagBna->ClearError();
				}
				// end of [#GLDV-3005]

				// Clear Processing Count
				m_OpInfo.nProcessingCount = 0;

				while(m_OpInfo.nProcessingCount < m_OpInfo.nTestCount || (m_OpInfo.nTestCount == -1))
				{
					m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_INIT_SPR);	// [#2160] CA KMK 2012.11.16 French OP
					ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
//					Delay_Msg(SCR_OPERATION_WAIT);

					/******************/
					/* Initialize SPR */
					/******************/
					// [#600] NH KSK 2009.12.16 단선시 복구 가능하도록 수정
					if( m_bDiagSPROpen )
						nErrorCode = m_pDiagSpr->Initialize();
					else
					{
						nErrorCode = m_pDiagSpr->Open();

						if(nErrorCode == SPR_SUCCESS)
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Open() : SUCCESS \n")));
							m_bDiagSPROpen = TRUE;
							nErrorCode = m_pDiagSpr->Initialize();
						}
						else
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Open() : FAIL \n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_SPR);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							NVDump('O', 'A', "11", L"", L"OPEN SPR_NG1" );		// [#2024] NH KSK 2011.02.24
							break;
						}
					}
					// end of [#600]

					if(nErrorCode == SPR_SUCCESS)
					{
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Initialize() : SUCCESS \n")));

						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_INIT_SPR);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
					}
					else /* SPR ERROR */
					{
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Initialize() : FAIL \n")));

						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_SPR);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
						NVDump('O', 'A', "11", L"", L"INIT SPR_NG" );		// [#2024] NH KSK 2011.02.24
						break;
					}

					//////////////////////
					/* CHECK INPUT WAIT */
					//////////////////////
					Delay_Msg(200);

					if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
					{
						CString strName, strData;
						// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
						strData = m_pDevCmn->fstrSCR_GetKeyString();
						if( (strData == S_EXIT) || (strData == S_CANCEL) )
						{
							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_INIT);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							bCancelIntial = TRUE;
						}
						// end of [#573]

						if (bCancelIntial)
							break;
					}

					/******************/
					/* Initialize MCU */
					/******************/
					m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_INIT_MCU);	// [#2160] CA KMK 2012.11.16 French OP
					ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);

					// [#600] NH KSK 2009.12.16 단선시 복구 가능하도록 수정
					if( m_bDiagMCUOpen == TRUE )
						nErrorCode = m_pDiagMcu->Initialize();
					else
					{
						nErrorCode = m_pDiagMcu->Open();

						if(nErrorCode == MCU_SUCCESS)
						{
							m_bDiagMCUOpen = TRUE;
							nErrorCode = m_pDiagMcu->Initialize();
						}
						else
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagMcu->Initialize() : FAIL \n")));
							m_bDiagMCUOpen = FALSE;		// [#2229] NH KMK 2013.11.06

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_MCU);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							NVDump('O', 'A', "11", L"", L"OPEN MCU_NG1" );		// [#2024] NH KSK 2011.02.24
							break;
						}
					}
					// end of [#600]

					if(nErrorCode == MCU_SUCCESS)
					{
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagMcu->Initialize() : SUCCESS \n")));

						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_INIT_MCU);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
					}
					else /* MCU ERROR */
					{
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagMcu->Initialize() : FAIL \n")));
						m_bDiagMCUOpen = FALSE;		// [#2229] NH KMK 2013.11.06

						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_MCU);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
						NVDump('O', 'A', "11", L"", L"INIT MCU_NG1" );		// [#2024] NH KSK 2011.02.24
						break;
					}

					//////////////////////
					/* CHECK INPUT WAIT */
					//////////////////////
					Delay_Msg(200);

					if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
					{
						CString strName, strData;
						// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
						strData = m_pDevCmn->fstrSCR_GetKeyString();
						if( (strData == S_EXIT) || (strData == S_CANCEL) )
						{
							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_INIT);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							bCancelIntial = TRUE;
						}
						// end of [#573]

						if (bCancelIntial)
							break;
					}

					// [#2325] NH KSK 2015.01.30
					/******************/
					/* Initialize RFID */
					/******************/
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
					{
						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_INIT_RFID);
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);

						// Open()시 단선도 Check하므로 Initialize Button시 Open을 시도하도록 함
						{
							nErrorCode = m_pDiagRfid->Open();

							if(nErrorCode == RFID_SUCCESS)
							{
								m_bDiagRFIDOpen = TRUE;
							}
							else
							{
								NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagRfid->Open() : FAIL \n")));
								m_bDiagRFIDOpen = FALSE;

								m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_RFID);
								ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
								Delay_Msg(SCR_OPERATION_WAIT);
								NVDump('O', 'A', "11", L"", L"OPEN RFID NG1" );
								break;
							}
						}

						if(nErrorCode == RFID_SUCCESS)
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagRfid->Open() : SUCCESS \n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_INIT_RFID);
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
						}
						else /* RFID ERROR */
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagRfid->Open() : FAIL \n")));
							m_bDiagRFIDOpen = FALSE;

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_RFID);
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							NVDump('O', 'A', "11", L"", L"INIT RFID_NG1" );
							break;
						}

						//////////////////////
						/* CHECK INPUT WAIT */
						//////////////////////
						Delay_Msg(200);

						if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
						{
							CString strName, strData;
							// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
							strData = m_pDevCmn->fstrSCR_GetKeyString();
							if( (strData == S_EXIT) || (strData == S_CANCEL) )
							{
								m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_INIT);	// [#2160] CA KMK 2012.11.16 French OP
								ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
								Delay_Msg(SCR_OPERATION_WAIT);
								bCancelIntial = TRUE;
							}
							// end of [#573]

							if (bCancelIntial)
								break;
						}
					}
					// end of [#2325]


					// [#GLDV-3005] US Kook 2022.07.18 Side Car
					/***********************/
					/* Initialize Side Car */
					/***********************/
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
					{
						///////////////////
						// BARCODE READER
						///////////////////
						m_OpInfo.strTempClerkMessage = L"INITIALIZE BCR";
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);

						if( m_bDiagBCROpen == TRUE )
							nErrorCode = m_pDiagBcr->Enable();
						else
						{
							nErrorCode = m_pDiagBcr->Open();

							if (nErrorCode == WFS_SUCCESS)
							{
								m_bDiagBCROpen = TRUE;
							}
							else
							{
								NHDEBUG(1, (_T("m_pDiagBcr->Open() : FAIL \n")));
								m_bDiagBCROpen = FALSE;

								m_OpInfo.strTempClerkMessage = L"FAIL TO INITIALIZE BCR";
								ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
								Delay_Msg(SCR_OPERATION_WAIT);
								NVDump('O', 'A', "11", L"", L"OPEN BCR NG1" );
								break;
							}
						}

						if (nErrorCode == WFS_SUCCESS)
						{
							NHDEBUG(1, (_T("m_pDiagBcr->Open() : SUCCESS \n")));

							m_OpInfo.strTempClerkMessage = L"SUCCESS TO INITIALIZE BCR";
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							nErrorCode = m_pDiagBcr->Disable();
						}
						else
						{
							NHDEBUG(1, (_T("m_pDiagBcr->Open() : FAILED \n")));
							m_bDiagBCROpen = FALSE;

							m_OpInfo.strTempClerkMessage = L"FAIL TO INITIALIZE BCR";
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							NVDump('O', 'A', "11", L"", L"INIT BCR NG1" );
							break;
						}

						//////////////////////
						/* CHECK INPUT WAIT */
						//////////////////////
						Delay_Msg(200);

						if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
						{
							CString strName, strData;
							// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
							strData = m_pDevCmn->fstrSCR_GetKeyString();
							if( (strData == S_EXIT) || (strData == S_CANCEL) )
							{
								m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_INIT);	// [#2160] CA KMK 2012.11.16 French OP
								ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
								Delay_Msg(SCR_OPERATION_WAIT);
								bCancelIntial = TRUE;
							}
							// end of [#573]

							if (bCancelIntial)
								break;
						}

						///////////////////
						// BILL ACCEPTOR
						///////////////////
						m_OpInfo.strTempClerkMessage = L"INITIALIZE BNA";
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);


						if( m_bDiagBNAOpen == TRUE )
							nErrorCode = m_pDiagBna->Initialize();
						else
						{
							nErrorCode = m_pDiagBna->Open();

							if (nErrorCode == WFS_SUCCESS)
							{
								m_bDiagBNAOpen = TRUE;
							}
							else
							{
								NHDEBUG(1, (_T("m_pDiagBna->Open() : FAIL \n")));
								m_bDiagBNAOpen = FALSE;

								m_OpInfo.strTempClerkMessage = L"FAIL TO INITIALIZE BNA";
								ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
								Delay_Msg(SCR_OPERATION_WAIT);
								NVDump('O', 'A', "11", L"", L"OPEN BNA NG1" );
								break;
							}
						}

						if (nErrorCode == WFS_SUCCESS)
						{
							NHDEBUG(1, (_T("m_pDiagBna->Open() : SUCCESS \n")));

							m_OpInfo.strTempClerkMessage = L"SUCCESS TO INITIALIZE BNA";
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
						}
						else
						{
							NHDEBUG(1, (_T("m_pDiagBna->Open() : FAILED \n")));
							m_bDiagBNAOpen = FALSE;

							m_OpInfo.strTempClerkMessage = L"FAIL TO INITIALIZE BNA";
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							NVDump('O', 'A', "11", L"", L"INIT BNA NG1" );
							break;
						}

						//////////////////////
						/* CHECK INPUT WAIT */
						//////////////////////
						Delay_Msg(200);

						if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
						{
							CString strName, strData;
							// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
							strData = m_pDevCmn->fstrSCR_GetKeyString();
							if( (strData == S_EXIT) || (strData == S_CANCEL) )
							{
								m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_INIT);	// [#2160] CA KMK 2012.11.16 French OP
								ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
								Delay_Msg(SCR_OPERATION_WAIT);
								bCancelIntial = TRUE;
							}
							// end of [#573]

							if (bCancelIntial)
								break;
						}
					}
					// end of [#GLDV-3005]


					/******************/
					/* Initialize CDU */
					/******************/
					m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_INIT_CDU);	// [#2160] CA KMK 2012.11.16 French OP
					ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);

					// [#600] NH KSK 2009.12.16 단선시 복구 가능하도록 수정
					if ( m_bDiagCDUOpen )
						nErrorCode = m_pDiagCdu->Initialize();
					else
					{
						nErrorCode = m_pDiagCdu->Open();

						if(nErrorCode == CDU_SUCCESS)
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagCdu->Open() : SUCCESS\n")));
							m_bDiagCDUOpen = TRUE;
							nErrorCode = m_pDiagCdu->Initialize();
						}
						else
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagCdu->Open() : FAIL\n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_CDU);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							NVDump('O', 'A', "11", L"", L"OPEN CDU_NG1" );		// [#2024] NH KSK 2011.02.24
							break;
						}
					}
					// end of [#600]

					if(nErrorCode == CDU_SUCCESS)
					{
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagCdu->Initialize() : SUCCESS\n")));

						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_INIT_CDU);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
					}
					else /* CDU ERROR */
					{ 
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagCdu->Initialize() : FAIL[%s] \n"), strDeviceError));

						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_CDU);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
						NVDump('O', 'A', "11", L"", L"INIT CDU_NG1" );		// [#2024] NH KSK 2011.02.24
						break;
					}

					//////////////////////
					/* CHECK INPUT WAIT */
					//////////////////////
					Delay_Msg(200);

					if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
					{
						CString strName, strData;
						// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
						strData = m_pDevCmn->fstrSCR_GetKeyString();
						if( (strData == S_EXIT) || (strData == S_CANCEL) )
						{
							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_INIT);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							bCancelIntial = TRUE;
						}
						// end of [#573]

						if (bCancelIntial)
							break;
					}

					/********************/
					/* Initialize MODEM */
					/********************/
					// [#585] NH KSK 2009.12.03
					if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP)
					{
						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_TEST_MODEM);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);

						if(m_pDevCmn->fnNET_ModemTest(m_OpInfo.strTestPhoneNumber, MODEM_HOST_DEV_TEST) == ERROR_SUCCESS)	// [#2016] NH KSK 2011.02.09
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::fnNET_ModemTest() : SUCCESS \n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_MODEM_TEST);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
						}
						else
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::fnNET_ModemTest() : FAIL \n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_MODEM_TEST);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							NVDump('O', 'A', "11", L"", L"TEST_MD_NG1");		// [#2024] NH KSK 2011.02.24
							break;
						}
					}
					// end of [#585]

					m_OpInfo.nProcessingCount++;

					if( m_OpInfo.nProcessingCount == m_OpInfo.nTestCount)
						break;
				}
			}
			else if (m_OpInfo.strReturn == L"CARDSCAN")			// F2
			{
				if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_MAGTEK_DIP || m_pDevCmn->GetDeviceType(L"MCU") == MCU_SANKYO_DIP)	// [#2078] NH KSK 2011.07.15
					m_OpInfo.nProcNextScrNum = 856;
				else
					m_OpInfo.nProcNextScrNum = 822;
				break;
			}
			else if (m_OpInfo.strReturn == L"SELECTTESTCOUNT")
			{
				m_pDevCmn->fnSCR_SetDisplayData("APSetFocus", "APValue2");
			}

			else if (m_OpInfo.strTempArray[0] == L"TESTCOUNT")
			{
				// KSK 2009.07.28 TEST COUNT가 NULL일 경우 보완 처리
				if (m_OpInfo.strTempArray[1].GetLength() > 0)
				{
					if (IsNum(m_OpInfo.strTempArray[1]) == TRUE)
					{
						if(Asc2Int(m_OpInfo.strTempArray[1]) == 0)
						{
							m_OpInfo.nTestCount = -1;
						}
						else
						{
							m_OpInfo.nTestCount = Asc2Int(m_OpInfo.strTempArray[1]);

							// Set focus to bill count input
							m_pDevCmn->fnSCR_SetDisplayData("APSetFocus", "APValue6");
						}
					}
					else
					{
						m_OpInfo.strTempClerkMessage = T_INVALID_DATA;
					}
				}
				else
				{
					m_OpInfo.strTempClerkMessage = T_INVALID_DATA;
				}
			}
			else if (m_OpInfo.strTempArray[0] == L"NOTECOUNT")
			{
				// KSK 2009.07.28 TEST COUNT가 NULL일 경우 보완 처리
				if (m_OpInfo.strTempArray[1].GetLength() > 0)
				{
					if (IsNum(m_OpInfo.strTempArray[1]) == TRUE)
					{
						if(Asc2Int(m_OpInfo.strTempArray[1]) == 0)
						{
							m_OpInfo.nNoteCount = 1;
						}
						else
						{
							m_OpInfo.nNoteCount = Asc2Int(m_OpInfo.strTempArray[1]);
						}
					}
					else
					{
						m_OpInfo.strTempClerkMessage = T_INVALID_DATA;
					}
				}
				else
				{
					m_OpInfo.strTempClerkMessage = T_INVALID_DATA;
				}
			}
			else if (m_OpInfo.strReturn == L"PRINTER")			// F3
			{
				/* ENABLE PIN & FUNCTION KEY */
				m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);

				// Clear Device
				m_pDiagCdu->ClearDeviceError();
				m_pDiagSpr->ClearDeviceError();
				m_pDiagMcu->ClearDeviceError();

				// [#2325] NH KSK 2015.01.25 RFID Added
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)		// [#2325] NH KMK 2015.02.12 RFID DISABLE 시 AP Popup 대책
					m_pDiagRfid->ClearDeviceError();
				// end of [#2325]

				// [#GLDV-3005] US Kook 2022.07.18 Side Car
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
				{
					if (m_pDiagBcr != NULL)
						m_pDiagBcr->ClearError();
					if (m_pDiagBna != NULL)
						m_pDiagBna->ClearError();
				}
				// end of [#GLDV-3005]

				// Clear Processing Count
				m_OpInfo.nProcessingCount = 0;

				while( m_OpInfo.nProcessingCount < m_OpInfo.nTestCount || (m_OpInfo.nTestCount == -1))
				{
					/* TEST PRINT SPR */
					m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_TEST_PRINTING);	// [#2160] CA KMK 2012.11.16 French OP
					ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
					Delay_Msg(SCR_OPERATION_WAIT);

					// [#600] NH KSK 2009.12.16 SPR 단선 시 복구하도록 수정
					if ( m_bDiagSPROpen )
					{
						nErrorCode = m_pDiagSpr->Test_Print(bWait, &nDeviceCmd);
					}
					else
					{
						nErrorCode = m_pDiagSpr->Open();
						if(nErrorCode == SPR_SUCCESS)
						{
							m_bDiagSPROpen = TRUE;
							nErrorCode = m_pDiagSpr->Test_Print(bWait, &nDeviceCmd);
						}
						else
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Open() : FAIL \n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_PRINT);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							NVDump('O', 'A', "11", L"", L"OPEN_SPR_NG2");		// [#2024] NH KSK 2011.02.24
							break;
						}
					}
					// end of [#600]

					if(nErrorCode == SPR_SUCCESS)
					{
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Test_Print() : SUCCESS \n")));

						nErrorCode = m_pDiagSpr->Cutting();

						if(nErrorCode == SPR_SUCCESS)
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Cut() : SUCCESS \n")));

							m_OpInfo.nProcessingCount++;
							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_PRINT);	// [#2160] CA KMK 2012.11.16 French OP;
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);

							if( m_OpInfo.nProcessingCount == m_OpInfo.nTestCount)
								break;
						}
						else
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Cut() : FAIL \n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_PRINT);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							NVDump('O', 'A', "11", L"", L"TEST_PT_NG1");		// [#2024] NH KSK 2011.02.24
							break;
						}
					}
					else
					{
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Test_Print() : FAIL \n")));

						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_PRINT);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
						break;
					}

					//////////////////////
					/* CHECK INPUT WAIT */
					//////////////////////
					Delay_Msg(200);
					if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
					{
						CString strName, strData;
						// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
						strData = m_pDevCmn->fstrSCR_GetKeyString();
						if( (strData == S_EXIT) || (strData == S_CANCEL) )
						{
							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_PRINTER);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							bCancelSPR = TRUE;
						}
						// end of [#573]

						if (bCancelSPR)
							break;
					}
				}
			}
			else if (m_OpInfo.strReturn == L"ANTISKIMMING")		// F4 [#2078] NH KSK 2011.07.11 ANTI SKIMMING 지원 
			{
				m_OpInfo.nProcNextScrNum = 746;
				break;
			}
			else if (m_OpInfo.strReturn == L"DISPENSER")		// F5
			{
				/* ENABLE PIN & FUNCTION KEY */
				m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);

				// Clear Device
				m_pDiagCdu->ClearDeviceError();
				m_pDiagSpr->ClearDeviceError();
				m_pDiagMcu->ClearDeviceError();

				// [#2325] NH KSK 2015.01.25 RFID Added
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)		// [#2325] NH KMK 2015.02.12 RFID DISABLE 시 AP Popup 대책
					m_pDiagRfid->ClearDeviceError();
				// end of [#2325]

				// [#GLDV-3005] US Kook 2022.07.18 Side Car
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
				{
					if (m_pDiagBcr != NULL)
						m_pDiagBcr->ClearError();
					if (m_pDiagBna != NULL)
						m_pDiagBna->ClearError();
				}
				// end of [#GLDV-3005]

				// Clear Processing Count
				m_OpInfo.nProcessingCount = 0;

				while( m_OpInfo.nProcessingCount < m_OpInfo.nTestCount || (m_OpInfo.nTestCount == -1))
				{
					memset(ulDispensedNotes, 0, sizeof(ulDispensedNotes));
					memset(ulRequestNotes, 0, sizeof(ulRequestNotes));
					ulTotalRejectedNotes = 0;

					/* CDU DISPENSE */
					m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_DISPENSING);	// [#2160] CA KMK 2012.11.16 French OP
					ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
					Delay_Msg(SCR_OPERATION_WAIT);

					// Set dispense note counts
					for (int i = 0; i < sizeof(ulRequestNotes) / sizeof(ulRequestNotes[0]); i++)
					{
						ulRequestNotes[i] = m_OpInfo.nNoteCount;
					}

					// [#600] NH KSK 2009.12.16 CDU 단선시 복구하도록 수정
					if (m_bDiagCDUOpen == TRUE)
					{
						nErrorCode =  m_pDiagCdu->Test_Dispense(ulDispensedNotes, &ulTotalRejectedNotes, ulRequestNotes);
					}
					else
					{
						nErrorCode = m_pDiagCdu->Open();
						if (nErrorCode == CDU_SUCCESS)
						{
							m_bDiagCDUOpen = TRUE;
							nErrorCode =  m_pDiagCdu->Test_Dispense(ulDispensedNotes, &ulTotalRejectedNotes, ulRequestNotes);
						}
						else
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagCdu->Open() : FAIL[%s] \n"), strDeviceError));
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							NVDump('O', 'A', "11", L"", L"OPEN_CDU_NG2");		// [#2024] NH KSK 2011.02.24
							break;							
						}
					}
					// end of [#600]

					if(nErrorCode == CDU_SUCCESS)
					{
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagCdu->Test_Dispense() : SUCCESS [%s] \n"), strDeviceError));

						m_OpInfo.nProcessingCount++;

						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_DISPENSING);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);

						if( m_OpInfo.nProcessingCount == m_OpInfo.nTestCount)
							break;
					}
					else
					{
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagCdu->Test_Dispense() : FAIL[%s] \n"), strDeviceError));

						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_DISPENSING);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
						NVDump('O', 'A', "11", L"", L"TEST_DP_NG1");		// [#2024] NH KSK 2011.02.24
						break;
					}

					//////////////////////
					/* CHECK INPUT WAIT */
					//////////////////////
					Delay_Msg(200);
					if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
					{
						CString strName, strData;
						// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
						strData = m_pDevCmn->fstrSCR_GetKeyString();
						if( (strData == S_EXIT) || (strData == S_CANCEL) )
						{
							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_DISPENSER);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							bCancelCDU = TRUE;
						}
						// end of [#573]

						if (bCancelCDU)
							break;
					}
				}
			}
			else if (m_OpInfo.strReturn == L"AUXILIARYUNIT")			// F6
			{
				m_OpInfo.nProcNextScrNum = 827;
				break;
			}
			
//			else if (m_OpInfo.strReturn == L"MODEM")			// F7
			else if (m_OpInfo.strReturn == L"NETWORKTEST")
			{
				if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP)
					m_OpInfo.nProcNextScrNum = 823;
				else
				{
					m_OpInfo.nProcNextScrNum = 872;
				}
				break;
			}
			
			else if (m_OpInfo.strReturn == L"AGING")			// F8
			{
				/* ENABLE PIN & FUNCTION KEY */
				m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);

				// Clear Device
				m_pDiagCdu->ClearDeviceError();
				m_pDiagSpr->ClearDeviceError();
				m_pDiagMcu->ClearDeviceError();

				// [#2325] NH KSK 2015.01.25 RFID Added
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)		// [#2325] NH KMK 2015.02.12 RFID DISABLE 시 AP Popup 대책
					m_pDiagRfid->ClearDeviceError();
				// end of [#2325]

				// [#GLDV-3005] US Kook 2022.07.18 Side Car
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
				{
					if (m_pDiagBcr != NULL)
						m_pDiagBcr->ClearError();
					if (m_pDiagBna != NULL)
						m_pDiagBna->ClearError();
				}
				// end of [#GLDV-3005]

				// Clear Processing Count
				m_OpInfo.nProcessingCount = 0;

				// KSK 2009.07.28 AGING일 경우에는 UNLIMIT으로 자동 설정되어야만 한다.
				m_OpInfo.nTestCount = -1;

				while( m_OpInfo.nProcessingCount < m_OpInfo.nTestCount || (m_OpInfo.nTestCount == -1))
				{
					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					/* TEST PRINT SPR */
					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_TEST_PRINTING);	// [#2160] CA KMK 2012.11.16 French OP
					ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);

					// [#600] NH KSK 2009.12.16 SPR 단선 시 복구하도록 수정
					if ( m_bDiagSPROpen )
					{
						nErrorCode = m_pDiagSpr->Test_Print(bWait, &nDeviceCmd);
					}
					else
					{
						nErrorCode = m_pDiagSpr->Open();
						if(nErrorCode == SPR_SUCCESS)
						{
							m_bDiagSPROpen = TRUE;
							nErrorCode = m_pDiagSpr->Test_Print(bWait, &nDeviceCmd);
						}
						else
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Open() : FAIL \n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_PRINT);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							break;
						}
					}
					// end of [#600]

					if(nErrorCode == SPR_SUCCESS)
					{
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Test_Print() : SUCCESS \n")));

						nErrorCode = m_pDiagSpr->Cutting();

						if(nErrorCode == SPR_SUCCESS)
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Cut() : SUCCESS \n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_PRINT);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
						}
						else
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Cut() : FAIL \n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_PRINT);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							break;
						}

						/* CHECK INPUT WAIT */
						if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
						{
							CString strName, strData;
							// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
							strData = m_pDevCmn->fstrSCR_GetKeyString();
							if( (strData == S_EXIT) || (strData == S_CANCEL) )
							{
								m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_AGING_TEST);	// [#2160] CA KMK 2012.11.16 French OP
								ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
								Delay_Msg(SCR_OPERATION_WAIT);
								bCancelAGING = TRUE;
							}
							// end of [#573]

							if (bCancelAGING)
								break;
						}
					}
					else
					{
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagSpr->Test_Print() : FAIL \n")));

						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_PRINT);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
						break;
					}

					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					/* TEST MCU */
					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_INIT_MCU);	// [#2160] CA KMK 2012.11.16 French OP
					ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);

					// [#600] NH KSK 2009.12.16 단선시 복구 가능하도록 수정
					if( m_bDiagMCUOpen == TRUE )
						nErrorCode = m_pDiagMcu->Initialize();
					else
					{
						nErrorCode = m_pDiagMcu->Open();

						if(nErrorCode == MCU_SUCCESS)
						{
							m_bDiagMCUOpen = TRUE;
							nErrorCode = m_pDiagMcu->Initialize();
						}
						else
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagMcu->Initialize() : FAIL \n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_MCU);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							break;
						}
					}
					// end of [#600]

					if(nErrorCode == MCU_SUCCESS)
					{
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagMcu->Initialize() : FAIL \n")));

						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_INIT_MCU);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
					}
					else /* MCU ERROR */
					{
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagMcu->Initialize() : FAIL \n")));

						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_MCU);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
						break;
					}

					//////////////////////
					/* CHECK CANCLE     */
					//////////////////////
					Delay_Msg(200);
					if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
					{
						CString strName, strData;
						// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
						strData = m_pDevCmn->fstrSCR_GetKeyString();
						if( (strData == S_EXIT) || (strData == S_CANCEL) )
						{
							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_AGING_TEST);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							bCancelAGING = TRUE;
						}
						// end of [#573]

						if (bCancelAGING)
							break;
					}

					// [#2325] NH KSK 2015.01.25 RFID Added
					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					/* TEST RFID */
					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
					{
						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_INIT_RFID);
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);

						if( m_bDiagRFIDOpen != TRUE )
						{
							nErrorCode = m_pDiagRfid->Open();

							if(nErrorCode == RFID_SUCCESS)
							{
								m_bDiagRFIDOpen = TRUE;
							}
							else
							{
								NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagRfid->Initialize() : FAIL \n")));

								m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_RFID);
								ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
								Delay_Msg(SCR_OPERATION_WAIT);
								break;
							}
						}

						if(nErrorCode == RFID_SUCCESS)
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagRfid->Initialize() : FAIL \n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_INIT_RFID);
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
						}
						else /* RFID ERROR */
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagRfid->Initialize() : FAIL \n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_RFID);
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							break;
						}

						//////////////////////
						/* CHECK CANCLE     */
						//////////////////////
						Delay_Msg(200);
						if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
						{
							CString strName, strData;
							strData = m_pDevCmn->fstrSCR_GetKeyString();
							if( (strData == S_EXIT) || (strData == S_CANCEL) )
							{
								m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_AGING_TEST);
								ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
								Delay_Msg(SCR_OPERATION_WAIT);
								bCancelAGING = TRUE;
							}

							if (bCancelAGING)
								break;
						}
					}
					// end of [#2325]


					// [#GLDV-3005] US Kook 2022.07.18 Side Car
					/***********************/
					/* Initialize Side Car */
					/***********************/
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
					{
						///////////////////
						// BARCODE READER
						///////////////////
						m_OpInfo.strTempClerkMessage = L"INITIALIZE BCR";
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);

						if( m_bDiagBCROpen == TRUE )
							nErrorCode = m_pDiagBcr->Enable();
						else
						{
							nErrorCode = m_pDiagBcr->Open();

							if (nErrorCode == WFS_SUCCESS)
							{
								m_bDiagBCROpen = TRUE;
							}
							else
							{
								NHDEBUG(1, (_T("m_pDiagBcr->Open() : FAIL \n")));
								m_bDiagBCROpen = FALSE;

								m_OpInfo.strTempClerkMessage = L"FAIL TO INITIALIZE BCR";
								ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
								Delay_Msg(SCR_OPERATION_WAIT);
								NVDump('O', 'A', "11", L"", L"OPEN BCR NG1" );
								break;
							}
						}

						if (nErrorCode == WFS_SUCCESS)
						{
							NHDEBUG(1, (_T("m_pDiagBcr->Open() : SUCCESS \n")));

							m_OpInfo.strTempClerkMessage = L"SUCCESS TO INITIALIZE BCR";
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							nErrorCode = m_pDiagBcr->Disable();
						}
						else
						{
							NHDEBUG(1, (_T("m_pDiagBcr->Open() : FAILED \n")));
							m_bDiagBCROpen = FALSE;

							m_OpInfo.strTempClerkMessage = L"FAIL TO INITIALIZE BCR";
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							NVDump('O', 'A', "11", L"", L"INIT BCR NG1" );
							break;
						}

						//////////////////////
						/* CHECK INPUT WAIT */
						//////////////////////
						Delay_Msg(200);

						if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
						{
							CString strName, strData;
							// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
							strData = m_pDevCmn->fstrSCR_GetKeyString();
							if( (strData == S_EXIT) || (strData == S_CANCEL) )
							{
								m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_INIT);	// [#2160] CA KMK 2012.11.16 French OP
								ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
								Delay_Msg(SCR_OPERATION_WAIT);
								bCancelIntial = TRUE;
							}
							// end of [#573]

							if (bCancelIntial)
								break;
						}

						///////////////////
						// BILL ACCEPTOR
						///////////////////
						m_OpInfo.strTempClerkMessage = L"INITIALIZE BNA";
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);


						if( m_bDiagBNAOpen == TRUE )
							nErrorCode = m_pDiagBna->Initialize();
						else
						{
							nErrorCode = m_pDiagBna->Open();

							if (nErrorCode == WFS_SUCCESS)
							{
								m_bDiagBNAOpen = TRUE;
							}
							else
							{
								NHDEBUG(1, (_T("m_pDiagBna->Open() : FAIL \n")));
								m_bDiagBNAOpen = FALSE;

								m_OpInfo.strTempClerkMessage = L"FAIL TO INITIALIZE BNA";
								ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
								Delay_Msg(SCR_OPERATION_WAIT);
								NVDump('O', 'A', "11", L"", L"OPEN BNA NG1" );
								break;
							}
						}

						if (nErrorCode == WFS_SUCCESS)
						{
							NHDEBUG(1, (_T("m_pDiagBna->Open() : SUCCESS \n")));

							m_OpInfo.strTempClerkMessage = L"SUCCESS TO INITIALIZE BNA";
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
						}
						else
						{
							NHDEBUG(1, (_T("m_pDiagBna->Open() : FAILED \n")));
							m_bDiagBNAOpen = FALSE;

							m_OpInfo.strTempClerkMessage = L"FAIL TO INITIALIZE BNA";
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							NVDump('O', 'A', "11", L"", L"INIT BNA NG1" );
							break;
						}

						//////////////////////
						/* CHECK INPUT WAIT */
						//////////////////////
						Delay_Msg(200);

						if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
						{
							CString strName, strData;
							// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
							strData = m_pDevCmn->fstrSCR_GetKeyString();
							if( (strData == S_EXIT) || (strData == S_CANCEL) )
							{
								m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_INIT);	// [#2160] CA KMK 2012.11.16 French OP
								ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
								Delay_Msg(SCR_OPERATION_WAIT);
								bCancelIntial = TRUE;
							}
							// end of [#573]

							if (bCancelIntial)
								break;
						}
					}
					// end of [#GLDV-3005]

					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					/* TEST CDU */
					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_INIT_CDU);	// [#2160] CA KMK 2012.11.16 French OP
					ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);

					// [#600] NH KSK 2009.12.16 단선시 복구 가능하도록 수정
					if ( m_bDiagCDUOpen )
						nErrorCode = m_pDiagCdu->Initialize();
					else
					{
						nErrorCode = m_pDiagCdu->Open();

						if(nErrorCode == CDU_SUCCESS)
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagCdu->Open() : SUCCESS\n")));
							m_bDiagCDUOpen = TRUE;
							nErrorCode = m_pDiagCdu->Initialize();
						}
						else
						{
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagCdu->Open() : FAIL\n")));

							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_CDU);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							break;
						}
					}
					// end of [#600]

					if(nErrorCode == CDU_SUCCESS)
					{
						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_INIT_CDU);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagCdu->Initialize() : SUCCESS\n")));
					}
					else /* CDU ERROR */
					{ 
						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT_CDU);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
						break;
						NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagCdu->Initialize() : FAIL[%s] \n"), strDeviceError));
					}

					//////////////////////
					/* CHECK CANCLE     */
					//////////////////////					
					Delay_Msg(200);

					if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
					{
						CString strName, strData;
						// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
						strData = m_pDevCmn->fstrSCR_GetKeyString();
						if( (strData == S_EXIT) || (strData == S_CANCEL) )
						{
							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_AGING_TEST);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							bCancelAGING = TRUE;
						}
						// end of [#573]

						if (bCancelAGING)
							break;
					}

					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					/* TEST MODEM */
					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_TEST_MODEM);	// [#2160] CA KMK 2012.11.16 French OP
					ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);

					if(m_pDevCmn->fnNET_ModemTest(m_OpInfo.strTestPhoneNumber, MODEM_HOST_DEV_TEST) == ERROR_SUCCESS)	// [#2016] NH KSK 2011.02.09
					{
						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_MODEM_TEST);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
					}
					else
					{
						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_MODEM_TEST);	// [#2160] CA KMK 2012.11.16 French OP
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
						Delay_Msg(SCR_OPERATION_WAIT);
						break;
					}

					//////////////////////
					/* CHECK CANCLE     */
					//////////////////////					
					Delay_Msg(200);

					if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
					{
						CString strName, strData;
						// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
						strData = m_pDevCmn->fstrSCR_GetKeyString();
						if( (strData == S_EXIT) || (strData == S_CANCEL) )
						{
							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_CANCEL_AGING_TEST);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
							Delay_Msg(SCR_OPERATION_WAIT);
							bCancelAGING = TRUE;
						}
						// end of [#573]

						if (bCancelAGING)
							break;
					}

					m_OpInfo.nProcessingCount++;

					if( m_OpInfo.nProcessingCount == m_OpInfo.nTestCount)
						break;

					Delay_Msg(200);
				}
			}
		}

		m_pDevCmn->fnSCR_ClearUserInputData();			// [#573] NH AIREAT 2009.10.06 iTM 소스 작업

		m_OpInfo.nMsgClearCount = SCR_OPMSGCYCLE;	// about 2 sec.
		// 데이터 업데이트
		ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);
	} // end of while

	NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkDiagonsticsMain() return \n")));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDiagnosticsCardScan()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 822 PROC
-------------------------------------------------------------------*/
int	CMainFrame::ClerkDiagnosticsCardScan()
{
NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsCardScan() \n")));
	CARD_TRACK_DATA TrackData;
//	CString	strISO1Track;
//	CString	strISO2Track;
//	CString strISO3Track;
	int nReturn = 0;
	int nDeviceCmd = 0;
	BOOL bReadCmd  = FALSE;
	BOOL bWait	   = FALSE;
	BOOL bScreenDisplay = TRUE;

	m_OpInfo.nProcLoofCnt = 0;
	m_OpInfo.nMsgClearCount = 0;
	m_OpInfo.nMoveProcReason = MOVE_PROC_SELECT;
	
	m_OpInfo.strISO1Track.Empty();
	m_OpInfo.strISO2Track.Empty();
	m_OpInfo.strISO3Track.Empty();
	m_OpInfo.nISO1Length = 0;
	m_OpInfo.nISO2Length = 0;
	m_OpInfo.nISO3Length = 0;

	while(TRUE)
	{
		while(TRUE)
		{
			// OP MSG Clear
			m_OpInfo.strTempClerkMessage = _T(" ");

			// Display Screen
			if ((m_OpInfo.nCurScrNum != m_OpInfo.nProcNextScrNum) || (bScreenDisplay == TRUE))
			{
				bScreenDisplay = FALSE;
				ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout, FALSE, PIN_AMOUNT_MODE);
			}
			else
			{
				ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
			}

			if( bReadCmd == FALSE )
			{
NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::m_pDiagMcu->Read_TimeOut() Start !!!!\n")));
				bWait = FALSE;
				m_pDiagMcu->Read_TimeOut(&TrackData, MCU_READ_CHECK_TIME, bWait, &nDeviceCmd);
				bReadCmd = TRUE;
			}
			else
			{
				nReturn = m_pDiagMcu->CompleteAction(nDeviceCmd, MCU_IO_COMPLETED_CHECK_TIME);

NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::m_pDiagMcu->CompleteAction() nReturn[%d]\n"), nReturn));	
			
				if ( nReturn == MCU_SUCCESS )
				{
					CString strTmp;
					CString strTmp2;

					m_pDiagMcu->GetTrackData(&TrackData);

					m_OpInfo.nISO1Length = TrackData.Iso1_Length;
					m_OpInfo.nISO2Length = TrackData.Iso2_Length;
					m_OpInfo.nISO3Length = TrackData.Iso3_Length;

					if ( TrackData.Iso1_Length >= 55 )
					{
						strTmp.Format(_T("%S"), TrackData.Iso1_Data);
						m_OpInfo.strISO1Track.Format(_T("%s\n%s"),strTmp.Left(50), strTmp.Mid(50));
						
					}
					else
						m_OpInfo.strISO1Track.Format(_T("%S"), TrackData.Iso1_Data);

					if ( TrackData.Iso2_Length >= 55 )
					{
						strTmp.Format(_T("%S"), TrackData.Iso2_Data);
						m_OpInfo.strISO2Track.Format(_T("%s\n%s"),strTmp.Left(50), strTmp.Mid(50));						
					}
					else
						m_OpInfo.strISO2Track.Format(_T("%S"), TrackData.Iso2_Data);

					if ( TrackData.Iso3_Length >= 55 )
					{
						strTmp.Format(_T("%S"), TrackData.Iso3_Data);
						m_OpInfo.strISO3Track.Format(_T("%s\n%s"),strTmp.Left(50), strTmp.Mid(50));
						
					}
					else
						m_OpInfo.strISO3Track.Format(_T("%S"), TrackData.Iso3_Data);

					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsCardScan() : TrackData.Iso1_Length[%d], TrackData.Iso2_Length[%d]\n"),TrackData.Iso1_Length, TrackData.Iso2_Length));
					bReadCmd = FALSE;

					// KSK 2009.08.04 Track Data가 없을 경우 FAIL 처리 추가
					if (m_OpInfo.strISO1Track.GetLength() > 0 || m_OpInfo.strISO2Track.GetLength() > 0 || m_OpInfo.strISO3Track.GetLength() > 0)
					{
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);
						bScreenDisplay = TRUE;	// [#2343] KSK 2015.05.20 화면 TimeOut 갱신		
					}
					else
					{
						ClerkNoticeToUser(m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_READ), TRUE);
						bScreenDisplay = TRUE;
					}
					// end of KSK 2009.08.04

					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsCardScan() : MCU_IO_COMPLETED \n")));
				}
				else if ( nReturn == MCU_IO_TIME_OUT )
				{
					bReadCmd = FALSE;
					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsCardScan() : MCU_IO_TIME_OUT \n")));
				}
				else if ( nReturn == IO_PROCESSING )
				{
					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsCardScan() : MCU_IO_COMPLETED_CHECK_TIME TIME OUT \n")));
				}
				else if ( nReturn == -1 )
				{
					m_OpInfo.strISO1Track.Empty();
					m_OpInfo.strISO2Track.Empty();
					m_OpInfo.strISO3Track.Empty();
					m_OpInfo.nISO1Length = 0;
					m_OpInfo.nISO2Length = 0;
					m_OpInfo.nISO3Length = 0;

					ClerkNoticeToUser(m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_READ), TRUE);
					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsCardScan() : MCU_IO_FAIL [%d]\n"), nReturn));
					bReadCmd = FALSE;
					bScreenDisplay = TRUE;
				}

				// check input
				if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
				{
					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsCardScan() KEY PRESSED \n")));
					break;
				}
			}
			Delay_Msg(500);	// KSK 2009.08.07 500ms으로 Delay 조정
		}

		// Get Event String
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();
		SplitString(m_OpInfo.strReturn, SCR_RES_DELIMITER, m_OpInfo.strTempArray);

		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			NHDEBUG(DBG_INFO, (_T("S_CANCEL || S_EXIT : nReturn[%d]  bReadCmd[%d] \n"), nReturn, bReadCmd));	

			//while( nReturn != MCU_IO_TIME_OUT && bReadCmd == TRUE)
			while( nReturn == IO_PROCESSING && bReadCmd == TRUE)		// [#2343] KSK 2015.05.20 화면 TimeOut 갱신		
			{
				nReturn = m_pDiagMcu->CompleteAction(nDeviceCmd, MCU_IO_COMPLETED_CHECK_TIME);
				if( nReturn == MCU_IO_TIME_OUT )
				{
					NHDEBUG(DBG_INFO, (_T("if( nReturn == MCU_IO_TIME_OUT ) nReturn[%d]\n"), nReturn));
					m_pDiagMcu->CancleIO();
					break;
				}
			}

			NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsCardScan()  S_EXIT : nReturn[%d]\n"), nReturn));	

			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 821;
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			//while( nReturn != MCU_IO_TIME_OUT)
			while( nReturn == IO_PROCESSING && bReadCmd == TRUE)		// [#2343] KSK 2015.05.20 화면 TimeOut 갱신		
			{
				nReturn = m_pDiagMcu->CompleteAction(nDeviceCmd, MCU_IO_COMPLETED_CHECK_TIME);
				
				if( nReturn == MCU_IO_TIME_OUT )
				{
					nReturn = m_pDiagMcu->CompleteAction(nDeviceCmd, MCU_IO_COMPLETED_CHECK_TIME);
					break;
				}
			}
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
			break;
		}

		m_OpInfo.nMsgClearCount = SCR_OPMSGCYCLE;	// about 2 sec.
		// 데이터 업데이트
		ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);

	} // end of while

	NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsCardScan() return \n")));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDiagnosticsModem()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 823 PROC
-------------------------------------------------------------------*/
int	CMainFrame::ClerkDiagnosticsModem()
{
	NHDEBUG(DBG_CALL, (L"START\n"));

	while(TRUE)
	{
		NHDEBUG(DBG_INFO, (L"WAIT FOR USER INPUT\n"));

		///////////////////////////////////
		// WAIT FOR USER INPUT
		ClerkWaitScreenInput_byCha();
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();
		SplitString(m_OpInfo.strReturn, SCR_RES_DELIMITER, m_OpInfo.strTempArray);

		NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

		//-----------------------------------
		// PROCESS KEY STRING
		//-----------------------------------
		if (m_OpInfo.strTempArray.GetSize() >= 2 && m_OpInfo.strTempArray[0]==L"TESTPHONENUMBER")
		{
			CString strTemp;
			strTemp = m_OpInfo.strTempArray[1];

			m_OpInfo.m_strMsg = T_INVALID_DATA;

			if (strTemp.Find(L".") == -1)
			{
				if (m_OpInfo.strTempArray[1].IsEmpty() == FALSE)
				{
					ClerkNoticeToUser(T_PROCESS_WAIT);

//					if(m_pDevCmn->fnNET_ModemTest(m_OpInfo.strTempArray[1]))
					if(m_pDevCmn->fnNET_ModemTest(m_OpInfo.strTempArray[1], MODEM_HOST_LINE_TEST))	// [#2016] NH KSK 2011.02.09
						m_OpInfo.m_strMsg = T_OPERATE_OK;
					else
						m_OpInfo.m_strMsg = T_OPERATE_NG;
				}
			}			

			ClerkNoticeToUser(m_OpInfo.m_strMsg);
		}

		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 821;
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
			break;
		}

	} // end of while

	NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsModem() return \n")));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDiagnosticsSensor()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 824 PROC
-------------------------------------------------------------------*/
void	CMainFrame::ClerkDiagnosticsSensor()
{
	NHDEBUG(DBG_CALL, (L"START\n"));

	BOOL	bShowScreen = TRUE;

	while(TRUE)
	{
		///////////////////////////////////
		// DISPLAY OR UPDATE SCREEN
		if (bShowScreen == TRUE)
		{
			ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
			m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
			bShowScreen = FALSE;
			m_OpInfo.nProcLoofCnt = 0;
		}
		else if (m_OpInfo.nProcLoofCnt >= SCR_UPDATECYCLE)
		{
			ClerkCheckDeviceStatus();
			ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
			m_OpInfo.nProcLoofCnt = 0;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
			m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

			NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

			//-----------------------------------
			// PROCESS KEY STRING
			//-----------------------------------
			if (m_OpInfo.strReturn == L"CDUSENSOR")
			{
				CString strCDUEPVersion = MemGetVersion(_MEMKEY_EPVERSION, L"CDM");

				if (strCDUEPVersion.Left(3) == L_CDU_EP_VERSION)
				{
					m_OpInfo.nProcNextScrNum = 871;
				}
				else if (strCDUEPVersion.Left(3) == GBM10_EP_VERSION)	// [#2025] NH KJW 2011.02.28
				{
					m_OpInfo.nProcNextScrNum = 896;
				}														// end of [#2025]
#if (US_VERSION || CA_VERSION)	// [#2243] CA Justin 2013.12.17 Bug fix  on Canadian 4000W
				else if (strCDUEPVersion.Left(3) == W_CDU_EP_VERSION)	// [#2097] US PCS 2011.11.21
				{
					m_OpInfo.nProcNextScrNum = 876;
				}														// end of [#2097]
#endif
				else
				{
					// [#11] NH KSK 2010.09.20 후면기 미 지원
					// NH KSK 2012.03.20 후면기 지원으로 수정함.
//					NH_OS_VERSION	eOSVersion = m_pDevCmn->m_pConfig->GetOSVersion();
//					if (eOSVersion != NH_OS_NH1800SE_B)					// [#11] NH KSK 2010.09.14 1800SE 기종 Weather Service 미 지원					
					{
						if (((strCDUEPVersion.GetAt(5) & 0xF0) == 0x40))	// 후면기 Type은 0x4x 인경우 후면기임 
							m_OpInfo.nProcNextScrNum = 875;
						else
							m_OpInfo.nProcNextScrNum = 825;
					}
//					else
//						m_OpInfo.nProcNextScrNum = 825;
					// end of [#11]
				}		
				break;
			}
			else if (m_OpInfo.strReturn == L"SPRSENSOR")
			{
				//[#2097] US PCS 2011.11.21 WallMount Typed ATM
#if (US_VERSION || CA_VERSION)	// [#2243] CA Justin 2013.12.17 Bug fix  on Canadian 4000W
				if (MemGetVersion(_MEMKEY_EPVERSION, L"SPR").Left(3) == SPR_HWASUNG_EP_VERSION)
					m_OpInfo.nProcNextScrNum = 877;
				else
					m_OpInfo.nProcNextScrNum = 826;
#else
				m_OpInfo.nProcNextScrNum = 826;
#endif
				break;//end of [#2097]			
			}

			//-----------------------------------
			// COMMON PROCESS
			//-----------------------------------
			if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
				m_OpInfo.nProcNextScrNum = 827;
				break;
			}
			else if (m_OpInfo.strReturn == S_TIMEOVER)
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
				m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
				break;
			}
		}

		Delay_Msg(PROC_LOOFDELAY);
		m_OpInfo.nProcLoofCnt++;
	} // end of while

	NHDEBUG(DBG_CALL, (L"END\n"));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDiagnosticsCDUSensorView()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 825 PROC
-------------------------------------------------------------------*/
void	CMainFrame::ClerkDiagnosticsCDUSensorView()
{
	NHDEBUG(DBG_CALL, (L"START\n"));

	BOOL bShowScreen = TRUE;

	while(TRUE)
	{
		///////////////////////////////////
		// DISPLAY OR UPDATE SCREEN
		if (bShowScreen == TRUE)
		{
			ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
			m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
			bShowScreen = FALSE;
			m_OpInfo.nProcLoofCnt = 0;
		}
		else if (m_OpInfo.nProcLoofCnt >= SCR_UPDATECYCLE)
		{
			ClerkCheckDeviceStatus();
			ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
			m_OpInfo.nProcLoofCnt = 0;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
			m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

			NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

			//-----------------------------------
			// PROCESS KEY STRING
			//-----------------------------------
			// [#RWC6-82] US Kook 2022.04.22 cdu ar auto dimming
			if (m_OpInfo.strReturn == _T("AUTODIMMING"))
			{
				ClerkNoticeToUser(T_PROCESS_WAIT);

				int nRet = m_pDiagCdu->GetAutoAdjustSensor();

				if (nRet == CDU_SUCCESS)
				{
					ClerkNoticeToUser(T_OPERATE_OK);
				}
				else
				{
					ClerkNoticeToUser(T_DEVICE_ERR);
				}

				break;
			}

			// end of [#RWC6-82]

			//-----------------------------------
			// COMMON PROCESS
			//-----------------------------------
			if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
				m_OpInfo.nProcNextScrNum = 824;
				break;
			}
			else if (m_OpInfo.strReturn == S_TIMEOVER)
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
				m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
				break;
			}
		}

		Delay_Msg(PROC_LOOFDELAY);
		m_OpInfo.nProcLoofCnt++;
	}

	NHDEBUG(DBG_CALL, (L"END\n"));
}

//[#2097] US PCS 2011.11.21 "WallMount Typed CDU"
#if (US_VERSION || CA_VERSION)	// [#2243] CA Justin 2013.12.17 Bug fix  on Canadian 4000W
/*-------------------------------------------------------------------
CLASS    NAME: CMainFrame
FUNCTION NAME: ClerkDiagnosticsWCDUSensorView()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : SCR 876 PROC
-------------------------------------------------------------------*/
void CMainFrame::ClerkDiagnosticsWCDUSensorView()
{
	NHDEBUG(DBG_CALL, (L"START\n"));

	BOOL bShowScreen = TRUE;

	while(TRUE)
	{
		///////////////////////////////////
		// DISPLAY OR UPDATE SCREEN
		if (bShowScreen == TRUE)
		{
			ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
			m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
			bShowScreen = FALSE;
			m_OpInfo.nProcLoofCnt = 0;
		}
		else if (m_OpInfo.nProcLoofCnt >= SCR_UPDATECYCLE)
		{
			ClerkCheckDeviceStatus();
			ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
			m_OpInfo.nProcLoofCnt = 0;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
			m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

			NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

			//-----------------------------------
			// PROCESS KEY STRING
			//-----------------------------------

			//-----------------------------------
			// COMMON PROCESS
			//-----------------------------------
			if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
				m_OpInfo.nProcNextScrNum = 824;
				break;
			}
			else if (m_OpInfo.strReturn == S_TIMEOVER)
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
				m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
				break;
			}
		}

		Delay_Msg(PROC_LOOFDELAY);
		m_OpInfo.nProcLoofCnt++;
	}

	NHDEBUG(DBG_CALL, (L"END\n"));
}
#endif
// end of [#2097] 


// [#576] NH KSK 2009.11.06
/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDiagnostics_LCDUSensorView()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 871 PROC
-------------------------------------------------------------------*/
void	CMainFrame::ClerkDiagnostics_LCDUSensorView()
{
	NHDEBUG(DBG_CALL, (L"START\n"));

	BOOL bShowScreen = TRUE;

	while(TRUE)
	{
		///////////////////////////////////
		// DISPLAY OR UPDATE SCREEN
		if (bShowScreen == TRUE)
		{
			ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
			m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
			bShowScreen = FALSE;
			m_OpInfo.nProcLoofCnt = 0;
		}
		else if (m_OpInfo.nProcLoofCnt >= SCR_UPDATECYCLE)
		{
			ClerkCheckDeviceStatus();
			ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
			m_OpInfo.nProcLoofCnt = 0;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
			m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

			NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

			//-----------------------------------
			// PROCESS KEY STRING
			//-----------------------------------

			//-----------------------------------
			// COMMON PROCESS
			//-----------------------------------
			if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
				m_OpInfo.nProcNextScrNum = 824;
				break;
			}
			else if (m_OpInfo.strReturn == S_TIMEOVER)
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
				m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
				break;
			}
		}

		Delay_Msg(PROC_LOOFDELAY);
		m_OpInfo.nProcLoofCnt++;
	}

	NHDEBUG(DBG_CALL, (L"END\n"));
}

/*-------------------------------------------------------------------
CLASS    NAME: CMainFrame
FUNCTION NAME: ClerkDiagnostics_CDURSensorView()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : SCR 875 PROC
-------------------------------------------------------------------*/
void	CMainFrame::ClerkDiagnostics_CDURSensorView()
{
	NHDEBUG(DBG_CALL, (L"START\n"));

	BOOL bShowScreen = TRUE;

	while(TRUE)
	{
		///////////////////////////////////
		// DISPLAY OR UPDATE SCREEN
		if (bShowScreen == TRUE)
		{
			ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
			m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
			bShowScreen = FALSE;
			m_OpInfo.nProcLoofCnt = 0;
		}
		else if (m_OpInfo.nProcLoofCnt >= SCR_UPDATECYCLE)
		{
			ClerkCheckDeviceStatus();
			ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
			m_OpInfo.nProcLoofCnt = 0;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
			m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

			NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

			//-----------------------------------
			// PROCESS KEY STRING
			//-----------------------------------

			//-----------------------------------
			// COMMON PROCESS
			//-----------------------------------
			if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
				m_OpInfo.nProcNextScrNum = 824;
				break;
			}
			else if (m_OpInfo.strReturn == S_TIMEOVER)
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
				m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
				break;
			}
		}

		Delay_Msg(PROC_LOOFDELAY);
		m_OpInfo.nProcLoofCnt++;
	}

	NHDEBUG(DBG_CALL, (L"END\n"));
}

/*-------------------------------------------------------------------
CLASS    NAME: CMainFrame
FUNCTION NAME: ClerkDiagnostics_GBM10CHSensorView()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : SCR 896 PROC
-------------------------------------------------------------------*/
void	CMainFrame::ClerkDiagnostics_GBM10CHSensorView()
{
	NHDEBUG(DBG_CALL, (L"START\n"));

	BOOL bShowScreen = TRUE;

	while(TRUE)
	{
		///////////////////////////////////
		// DISPLAY OR UPDATE SCREEN
		if (bShowScreen == TRUE)
		{
			ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
			m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
			bShowScreen = FALSE;
			m_OpInfo.nProcLoofCnt = 0;
		}
		else if (m_OpInfo.nProcLoofCnt >= SCR_UPDATECYCLE)
		{
			ClerkCheckDeviceStatus();
			ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
			m_OpInfo.nProcLoofCnt = 0;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
			m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

			NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

			//-----------------------------------
			// PROCESS KEY STRING
			//-----------------------------------

			//-----------------------------------
			// COMMON PROCESS
			//-----------------------------------
			if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
				m_OpInfo.nProcNextScrNum = 824;
				break;
			}
			else if (m_OpInfo.strReturn == S_TIMEOVER)
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
				m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
				break;
			}
		}

		Delay_Msg(PROC_LOOFDELAY);
		m_OpInfo.nProcLoofCnt++;
	}

	NHDEBUG(DBG_CALL, (L"END\n"));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDiagnosticsSPRSensorView()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 826 PROC
-------------------------------------------------------------------*/
int	CMainFrame::ClerkDiagnosticsSPRSensorView()
{
	NHDEBUG(DBG_CALL, (L"START\n"));
	
	// [#2420] NH KSK 2016.04.29 Sensor Viewer시 200ms으로 화면 갱신 로직을 Sensor값 변경시에만 갱신하도록 수정
	BOOL bShowScreen = TRUE;

	while(TRUE)
	{
		///////////////////////////////////
		// DISPLAY OR UPDATE SCREEN
		if (bShowScreen == TRUE)
		{
			ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
			m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
			bShowScreen = FALSE;
			m_OpInfo.nProcLoofCnt = 0;
		}
		else if (m_OpInfo.nProcLoofCnt >= SCR_UPDATECYCLE)
		{
			ClerkCheckDeviceStatus();
			ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
			m_OpInfo.nProcLoofCnt = 0;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
			m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

			NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

			//-----------------------------------
			// PROCESS KEY STRING
			//-----------------------------------

			//-----------------------------------
			// COMMON PROCESS
			//-----------------------------------
			if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
				m_OpInfo.nProcNextScrNum = 824;
				break;
			}
			else if (m_OpInfo.strReturn == S_TIMEOVER)
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
				m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
				break;
			}
		}

		Delay_Msg(PROC_LOOFDELAY);
		m_OpInfo.nProcLoofCnt++;
	}

	/*
	m_OpInfo.nProcLoofCnt = 0;
	m_OpInfo.nMsgClearCount = 0;
	m_OpInfo.nMoveProcReason = MOVE_PROC_SELECT;					// [#236] NZ AIREAT 2008.05.27 OP Screen Timeout
	m_OpInfo.strTestPhoneNumber.Empty();
	
	while(TRUE)
	{
		m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);			// [#126] NH AIREAT 2008.4.18 OP Process 진행 중 모든 Key Disable 처리

		// start to check timeout
		// [#236] NZ AIREAT 2008.06.04
		if (m_OpInfo.nCurScrNum == m_OpInfo.nProcNextScrNum)
		{
			m_pDevCmn->fnSCR_SetDisplayData(L"APTimeoutMode", L"TimeoutLast");		// NH PSC 2008.07.07 소스리뷰결과 공통적용.
			m_pDevCmn->fnSCR_SetDisplayData(L"APMaxTime", Int2Asc(m_OpInfo.nTimeout));
		}
		// end of [#236]

		while(TRUE)
		{
			// 화면을 표시 or Update 한다.
			if (m_OpInfo.nCurScrNum != m_OpInfo.nProcNextScrNum)
			{
				ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
			}
			else
				ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);
			
			// 화면 입력을 기다린다.
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
				break;
			
			Delay_Msg(200);
		}

		// Stop to check timeout
		m_pDevCmn->fnSCR_SetDisplayData(L"APTimeoutMode", L"TimeoutStop");				// [#236] NZ AIREAT 2008.06.04

		m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);			// [#126] NH AIREAT 2008.4.18 OP Process 진행 중 모든 Key Disable 처리
		
		// Get Event String
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();
		SplitString(m_OpInfo.strReturn, SCR_RES_DELIMITER, m_OpInfo.strTempArray);
		
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 824;
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = OP_TIMEOUT_SCREEN_NUMBER;
			break;
		}
		else
			m_OpInfo.strTempClerkMessage = T_INVALID_DATA;
		
		
		m_OpInfo.nMsgClearCount = SCR_OPMSGCYCLE;	// about 2 sec.
		// 데이터 업데이트
		ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);
		
	} // end of while
	*/
	// end of [#2420]

	NHDEBUG(DBG_CALL, (L"END\n"));
	
	return TRUE;
}

#if (US_VERSION || CA_VERSION)	// [#2243] CA Justin 2013.12.17 Bug fix  on Canadian 4000W
/*-------------------------------------------------------------------
CLASS    NAME: CMainFrame
FUNCTION NAME: ClerkDiagnosticsWSPRSensorView()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : SCR 877 PROC
-------------------------------------------------------------------*/
int	CMainFrame::ClerkDiagnosticsWSPRSensorView()
{
	NHDEBUG(DBG_CALL, (L"START\n"));
	
	// [#2420] NH KSK 2016.04.29 Sensor Viewer시 200ms으로 화면 갱신 로직을 Sensor값 변경시에만 갱신하도록 수정
	BOOL bShowScreen = TRUE;

	while(TRUE)
	{
		///////////////////////////////////
		// DISPLAY OR UPDATE SCREEN
		if (bShowScreen == TRUE)
		{
			ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
			m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
			bShowScreen = FALSE;
			m_OpInfo.nProcLoofCnt = 0;
		}
		else if (m_OpInfo.nProcLoofCnt >= SCR_UPDATECYCLE)
		{
			ClerkCheckDeviceStatus();
			ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
			m_OpInfo.nProcLoofCnt = 0;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
			m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

			NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

			//-----------------------------------
			// PROCESS KEY STRING
			//-----------------------------------

			//-----------------------------------
			// COMMON PROCESS
			//-----------------------------------
			if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
				m_OpInfo.nProcNextScrNum = 824;
				break;
			}
			else if (m_OpInfo.strReturn == S_TIMEOVER)
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
				m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
				break;
			}
		}

		Delay_Msg(PROC_LOOFDELAY);
		m_OpInfo.nProcLoofCnt++;
	}

	/*
	m_OpInfo.nProcLoofCnt = 0;
	m_OpInfo.nMsgClearCount = 0;
	m_OpInfo.nMoveProcReason = MOVE_PROC_SELECT;					
	m_OpInfo.strTestPhoneNumber.Empty();

	while(TRUE)
	{
		m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);			

		// start to check timeout
		if (m_OpInfo.nCurScrNum == m_OpInfo.nProcNextScrNum)
		{
			m_pDevCmn->fnSCR_SetDisplayData(L"APTimeoutMode", L"TimeoutLast");		
			m_pDevCmn->fnSCR_SetDisplayData(L"APMaxTime", Int2Asc(m_OpInfo.nTimeout));
		}
		// end of [#236]

		while(TRUE)
		{
			// 화면을 표시 or Update 한다.
			if (m_OpInfo.nCurScrNum != m_OpInfo.nProcNextScrNum)
			{
				ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
			}
			else
				ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);

			// 화면 입력을 기다린다.
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
				break;

			Delay_Msg(200);
		}

		// Stop to check timeout
		m_pDevCmn->fnSCR_SetDisplayData(L"APTimeoutMode", L"TimeoutStop");				

		m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);			

		// Get Event String
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();
		SplitString(m_OpInfo.strReturn, SCR_RES_DELIMITER, m_OpInfo.strTempArray);

		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 824;
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = OP_TIMEOUT_SCREEN_NUMBER;
			break;
		}
		else
			m_OpInfo.strTempClerkMessage = T_INVALID_DATA;


		m_OpInfo.nMsgClearCount = SCR_OPMSGCYCLE;	// about 2 sec.
		// 데이터 업데이트
		ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);

	} // end of while
	*/
	// end of [#2420]

	NHDEBUG(DBG_CALL, (L"END\n"));

	return TRUE;
}

#endif
/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDiagnosticsAuxiliaryUnit()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 827 PROC
-------------------------------------------------------------------*/
int	CMainFrame::ClerkDiagnosticsAuxiliaryUnit()
{
	NHDEBUG(DBG_CALL, (L"START\n"));

	while(TRUE)
	{
		NHDEBUG(DBG_INFO, (L"WAIT FOR USER INPUT\n"));

		///////////////////////////////////
		// WAIT FOR USER INPUT
		ClerkWaitScreenInput_byCha();
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();
		SplitString(m_OpInfo.strReturn, SCR_RES_DELIMITER, m_OpInfo.strTempArray);

		NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

		//-----------------------------------
		// PROCESS KEY STRING
		//-----------------------------------
		if (m_OpInfo.strReturn == L"SENSOR")
		{
			m_OpInfo.nProcNextScrNum = 824;
			break;
		}
		else if (m_OpInfo.strReturn == L"FLICKER")
		{
			// [#2205] US KSK 2013.06.28
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
				m_OpInfo.nProcNextScrNum = 888;
			else
				m_OpInfo.nProcNextScrNum = 828;
			// end of [#2205]
			break;			
		}
		else if (m_OpInfo.strReturn == L"RFIDSCAN")
		{
			// [#2325] NH KSK 2015.01.25
			m_OpInfo.nProcNextScrNum = 751;
			// end of [#2325]
			break;			
		}
		// [#2518] US Kook 2017.12.21 Support MX-2800SE
		else if (m_OpInfo.strReturn == L"CAMERA")
		{
			m_OpInfo.nProcNextScrNum = 765;
			break;
		}
		// end of [#2518]
		// [#GLDV-3005] US Kook 2021.10.07 Support Side Car
		else if (m_OpInfo.strReturn == L"BARCODEREADER")
		{
			m_OpInfo.nProcNextScrNum = 770;
			break;
		}
		else if (m_OpInfo.strReturn == L"BILLACCEPTOR")
		{
			m_OpInfo.nProcNextScrNum = 771;
			break;
		}
		// end of [#GLDV-3005]

		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 821;
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
			break;
		}		
	} // end of while
	
NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsAuxiliaryUnit() return \n")));
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDiagnosticsFlicker()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 828 PROC
-------------------------------------------------------------------*/
int	CMainFrame::ClerkDiagnosticsFlicker()
{
//#define FLICKER_CDU		0x01
#define FLICKER_CDU		(0x01 | CDU_VB_FLICKER)		// [#GLDV-2890] US Kook 2021.05.17 Support CDU VB Flicker
#define FLICKER_EPP		(0x02 | EPP_VB_FLICKER)		// [#GLDV-2890] US Kook 2021.05.21 Support EPP VB Flicker
#define FLICKER_SPR		0x04
#define FLICKER_MCU		0x08
#define FLICKER_SIDECAR (SIDECAR_BNA_FLICKER | SIDECAR_COD_FLICKER)
#define FLICKER_ALL		(FLICKER_CDU | FLICKER_EPP | FLICKER_SPR | FLICKER_MCU | FLICKER_SIDECAR)

	BOOL bSIUInit;

NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsFlicker() \n")));
	
	m_OpInfo.nProcLoofCnt = 0;
	m_OpInfo.nMsgClearCount = 0;
	m_OpInfo.nMoveProcReason = MOVE_PROC_SELECT;

	m_OpInfo.strAllFlicker = L"OFF";
	m_OpInfo.strEPPFlicker = L"OFF";
	m_OpInfo.strCDUFlicker = L"OFF";
	m_OpInfo.strSPRFlicker = L"OFF";
	m_OpInfo.strMCUFlicker = L"OFF";
	m_OpInfo.strSideCarFlicker = L"OFF";

	m_OpInfo.FlickingDevice	= 0x00; //FLICKER_CDU | FLICKER_EPP | FLICKER_SPR| FLICKER_MCU;
	m_OpInfo.OnOffDevice	= 0x00; //FLICKER_CDU | FLICKER_EPP | FLICKER_SPR| FLICKER_MCU;
	
	for(int i =0 ; i < 5 ; i++)
	{
		bSIUInit = SIUInit();
		if( bSIUInit )
			break;
		Delay_Msg(500);
	}

	SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);

	while(TRUE)
	{
		NHDEBUG(DBG_INFO, (L"WAIT FOR USER INPUT\n"));

		///////////////////////////////////
		// WAIT FOR USER INPUT
		ClerkWaitScreenInput_byCha();
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();
		SplitString(m_OpInfo.strReturn, SCR_RES_DELIMITER, m_OpInfo.strTempArray);

		NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

		//-----------------------------------
		// PROCESS KEY STRING
		//-----------------------------------
		if (m_OpInfo.strReturn == L"ALLFLICKER")
		{
			if (m_OpInfo.strAllFlicker == L"OFF")
			{
				m_pDevCmn->fnAPL_DeviceSetFlicker(m_OpInfo.FlickingDevice, FLICKER_OFF);

				m_OpInfo.strAllFlicker = L"ON";

				m_OpInfo.strEPPFlicker = L"ON";
				m_OpInfo.strCDUFlicker = L"ON";
				m_OpInfo.strSPRFlicker = L"ON";
				m_OpInfo.strMCUFlicker = L"ON";
				m_OpInfo.strSideCarFlicker = L"ON";		// [#GLDV-3005]

				m_OpInfo.OnOffDevice = FLICKER_ALL;
				m_OpInfo.FlickingDevice = 0x00;

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}
			else if (m_OpInfo.strAllFlicker == L"ON")
			{
				m_OpInfo.strAllFlicker = L"FLICKING";

				m_OpInfo.strEPPFlicker = L"FLICKING";
				m_OpInfo.strCDUFlicker = L"FLICKING";
				m_OpInfo.strSPRFlicker = L"FLICKING";
				m_OpInfo.strMCUFlicker = L"FLICKING";
				m_OpInfo.strSideCarFlicker = L"FLICKING";		// [#GLDV-3005]

				m_OpInfo.OnOffDevice = 0x00;
				m_OpInfo.FlickingDevice = FLICKER_ALL;
				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}
			else if (m_OpInfo.strAllFlicker == L"FLICKING")
			{
				m_OpInfo.strAllFlicker = L"OFF";

				m_OpInfo.strEPPFlicker = L"OFF";
				m_OpInfo.strCDUFlicker = L"OFF";
				m_OpInfo.strSPRFlicker = L"OFF";
				m_OpInfo.strMCUFlicker = L"OFF";
				m_OpInfo.strSideCarFlicker = L"OFF";			// [#GLDV-3005]

				m_OpInfo.OnOffDevice	= 0x00;
				m_OpInfo.FlickingDevice = 0x00;
				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}
		}
		else if (m_OpInfo.strReturn == L"EPPFLICKER")
		{
			if (m_OpInfo.strEPPFlicker == L"OFF")
			{
				m_OpInfo.strEPPFlicker = L"ON";
				m_OpInfo.OnOffDevice |= FLICKER_EPP;

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}
			else if (m_OpInfo.strEPPFlicker == L"ON")
			{
				m_OpInfo.strEPPFlicker = L"FLICKING";
				m_OpInfo.FlickingDevice |= FLICKER_EPP;
				m_OpInfo.OnOffDevice = m_OpInfo.OnOffDevice & ~(FLICKER_EPP);

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}
			else if (m_OpInfo.strEPPFlicker == L"FLICKING")
			{
				m_OpInfo.strEPPFlicker = L"OFF";
				m_OpInfo.OnOffDevice	= m_OpInfo.OnOffDevice & ~(FLICKER_EPP);
				m_OpInfo.FlickingDevice = m_OpInfo.FlickingDevice & ~(FLICKER_EPP);

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}
		}
		else if (m_OpInfo.strReturn == L"CDUFLICKER")
		{
			if (m_OpInfo.strCDUFlicker == L"OFF")
			{
				m_OpInfo.strCDUFlicker = L"ON";
				m_OpInfo.OnOffDevice |= FLICKER_CDU;

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}
			else if (m_OpInfo.strCDUFlicker == L"ON")
			{
				m_OpInfo.strCDUFlicker = L"FLICKING";
				m_OpInfo.FlickingDevice |= FLICKER_CDU;
				m_OpInfo.OnOffDevice = m_OpInfo.OnOffDevice & ~(FLICKER_CDU);

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
//				SetGuideLight(m_OpInfo.FlickingDevice, 0x00);
			}
			else if (m_OpInfo.strCDUFlicker == L"FLICKING")
			{
				m_OpInfo.strCDUFlicker = L"OFF";
				m_OpInfo.OnOffDevice	= m_OpInfo.OnOffDevice & ~(FLICKER_CDU);
				m_OpInfo.FlickingDevice = m_OpInfo.FlickingDevice & ~(FLICKER_CDU);

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}				
		}
//		else if (m_OpInfo.strReturn == L"SPRFLICKER")
		else if (m_OpInfo.strReturn == L"SPRFLICKER" || m_OpInfo.strReturn == L"EPPLEDGUIDE")	// [#2334] NH KSK 2015.03.06 EPP2 -> EPP GUIDE LED 명칭 변경
		{
			if (m_OpInfo.strSPRFlicker == L"OFF")
			{
				m_OpInfo.strSPRFlicker = L"ON";
				m_OpInfo.OnOffDevice |= FLICKER_SPR;

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}
			else if (m_OpInfo.strSPRFlicker == L"ON")
			{
				m_OpInfo.strSPRFlicker = L"FLICKING";
				m_OpInfo.FlickingDevice |= FLICKER_SPR;
				m_OpInfo.OnOffDevice = m_OpInfo.OnOffDevice & ~(FLICKER_SPR);

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}
			else if (m_OpInfo.strSPRFlicker == L"FLICKING")
			{
				m_OpInfo.strSPRFlicker = L"OFF";
				m_OpInfo.OnOffDevice	= m_OpInfo.OnOffDevice & ~(FLICKER_SPR);
				m_OpInfo.FlickingDevice = m_OpInfo.FlickingDevice & ~(FLICKER_SPR);

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}				
		}
		else if (m_OpInfo.strReturn == L"MCUFLICKER")
		{
			if (m_OpInfo.strMCUFlicker == L"OFF")
			{
				m_OpInfo.strMCUFlicker = L"ON";
				m_OpInfo.OnOffDevice |= FLICKER_MCU;

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}
			else if (m_OpInfo.strMCUFlicker == L"ON")
			{
				m_OpInfo.strMCUFlicker = L"FLICKING";
				m_OpInfo.FlickingDevice |= FLICKER_MCU;
				m_OpInfo.OnOffDevice = m_OpInfo.OnOffDevice & ~(FLICKER_MCU);

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}
			else if (m_OpInfo.strMCUFlicker == L"FLICKING")
			{
				m_OpInfo.strMCUFlicker = L"OFF";
				m_OpInfo.OnOffDevice	= m_OpInfo.OnOffDevice & ~(FLICKER_MCU);
				m_OpInfo.FlickingDevice = m_OpInfo.FlickingDevice & ~(FLICKER_MCU);

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}				
		}
		// [#GLDV-3005] US Kook 2022.01.18 Support Side Car
		else if (m_OpInfo.strReturn == L"SIDECARFLICKER")
		{
			if (m_OpInfo.strSideCarFlicker == L"OFF")
			{
				m_OpInfo.strSideCarFlicker = L"ON";
				m_OpInfo.OnOffDevice |= FLICKER_SIDECAR;

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}
			else if (m_OpInfo.strSideCarFlicker == L"ON")
			{
				m_OpInfo.strSideCarFlicker = L"FLICKING";
				m_OpInfo.FlickingDevice |= FLICKER_SIDECAR;
				m_OpInfo.OnOffDevice = m_OpInfo.OnOffDevice & ~(FLICKER_SIDECAR);

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}
			else if (m_OpInfo.strSideCarFlicker == L"FLICKING")
			{
				m_OpInfo.strSideCarFlicker = L"OFF";
				m_OpInfo.OnOffDevice	= m_OpInfo.OnOffDevice & ~(FLICKER_SIDECAR);
				m_OpInfo.FlickingDevice = m_OpInfo.FlickingDevice & ~(FLICKER_SIDECAR);

				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			}				
		}
		// end of [#GLDV-3005]
		// 2022.05.03
		else if (m_OpInfo.strReturn == L"AGING_FLICKER")
		{
			CString strTemp;

			/* ENABLE PIN & FUNCTION KEY */
			m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);

			// Clear Processing Count
			m_OpInfo.nProcessingCount = 0;

			// KSK 2009.07.28 AGING일 경우에는 UNLIMIT으로 자동 설정되어야만 한다.
			m_OpInfo.nTestCount = 10000;

			while( m_OpInfo.nProcessingCount < m_OpInfo.nTestCount || (m_OpInfo.nTestCount == -1))
			{
				////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				/* FLICKER ON */
				////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				m_OpInfo.OnOffDevice = FLICKER_ALL;
				m_OpInfo.FlickingDevice = 0x00;
				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);

				strTemp.Format(L"TURNING ON FLICKERS...\n(%d / %d)\n\nPRESS CANCEL TO EXIT", m_OpInfo.nProcessingCount, m_OpInfo.nTestCount);
				ClerkNoticeToUser(strTemp, TRUE, 3);
				Delay_Msg(200);		// allowing key press

				//////////////////////
				/* CHECK CANCLE     */
				//////////////////////					
				if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
				{
					CString strName, strData;
					strData = m_pDevCmn->fstrSCR_GetKeyString();
					if( (strData == S_EXIT) || (strData == S_CANCEL) )
					{
						ClerkNoticeToUser(T_OPERATE_OK);
						Delay_Msg(SCR_OPERATION_WAIT);
						break;
					}
				}

				////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				/* FLICKER OFF */
				////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				m_OpInfo.OnOffDevice	= 0x00;
				m_OpInfo.FlickingDevice = 0x00;
				SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);

				strTemp.Format(L"TURNING OFF FLICKERS...\n(%d / %d)\n\nPRESS CANCEL TO EXIT", m_OpInfo.nProcessingCount, m_OpInfo.nTestCount);
				ClerkNoticeToUser(strTemp, TRUE, 2);
				Delay_Msg(200);		// allowing key press

				//////////////////////
				/* CHECK CANCLE     */
				//////////////////////					
				if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
				{
					CString strName, strData;
					strData = m_pDevCmn->fstrSCR_GetKeyString();
					if( (strData == S_EXIT) || (strData == S_CANCEL) )
					{
						ClerkNoticeToUser(T_OPERATE_OK);
						Delay_Msg(SCR_OPERATION_WAIT);
						break;
					}
				}

				m_OpInfo.nProcessingCount++;

				if( m_OpInfo.nProcessingCount == m_OpInfo.nTestCount)
					break;

				Delay_Msg(200);
			}
		}

		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
				m_OpInfo.OnOffDevice	= FLICKER_SPR;	// EPP LED GUIDE ON 처리
			else
				m_OpInfo.OnOffDevice	= 0x00;

			m_OpInfo.FlickingDevice = 0x00;
			SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			SIUDeInit();

			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 827;
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
				m_OpInfo.OnOffDevice	= FLICKER_SPR;	// EPP LED GUIDE ON 처리
			else
				m_OpInfo.OnOffDevice	= 0x00;

			m_OpInfo.FlickingDevice = 0x00;
			SetGuideLight(m_OpInfo.FlickingDevice, m_OpInfo.OnOffDevice);
			SIUDeInit();

			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
			break;
		}		
	} // end of while
	
NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsFlicker() return \n")));

	// Clear Processing Count
	m_OpInfo.nProcessingCount = 0;

	return TRUE;
}

// [#2205] US KSK 2013.06.28
/*-------------------------------------------------------------------
CLASS    NAME: CMainFrame
FUNCTION NAME: ClerkHaloLedNMCUFlickerProc()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : SCR 888 PROC
-------------------------------------------------------------------*/
int	CMainFrame::ClerkHaloLedNMCUFlickerProc()
{
	NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkHaloLedNMCUFlickerProc() \n")));

	m_OpInfo.nProcLoofCnt = 0;
	m_OpInfo.nMsgClearCount = 0;
	m_OpInfo.nMoveProcReason = MOVE_PROC_SELECT;

	m_OpInfo.nHaloLedMode    = OFF_HALOLED_MODE;
	m_OpInfo.nHaloLedColor   = HALOLED_COLOR_CYAN;
	m_OpInfo.strMCUFlicker	 = L"OFF";

	SetHaloLedControl(HALOLED_COLOR_CYAN, OFF_HALOLED_MODE, OFF_MCULED);

	while(TRUE)
	{
		NHDEBUG(DBG_INFO, (L"WAIT FOR USER INPUT\n"));

		///////////////////////////////////
		// WAIT FOR USER INPUT
		ClerkWaitScreenInput_byCha();
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();
		SplitString(m_OpInfo.strReturn, SCR_RES_DELIMITER, m_OpInfo.strTempArray);

		NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

		//-----------------------------------
		// PROCESS KEY STRING
		//-----------------------------------
		if (m_OpInfo.strReturn == L"LEDMODE")
		{
			switch(m_OpInfo.nHaloLedMode)
			{
				case OFF_HALOLED_MODE:
					m_OpInfo.nHaloLedMode = ON_HALOLED_MODE;
					break;
				case ON_HALOLED_MODE:
					m_OpInfo.nHaloLedMode = FLICKING_HALOLED_MODE;
					break;
				case FLICKING_HALOLED_MODE:
					m_OpInfo.nHaloLedMode = FADE_HALOLED_MODE;
					break;
				default:
					m_OpInfo.nHaloLedMode = OFF_HALOLED_MODE;
					break;
			}
			SetHaloLedControl(SKIP_HALOLED_COLOR, m_OpInfo.nHaloLedMode, SKIP_MCULED);
		}
		if (m_OpInfo.strReturn == L"LEDCOLOR")
		{
			switch(m_OpInfo.nHaloLedColor)
			{
				case HALOLED_COLOR_CYAN:
					m_OpInfo.nHaloLedColor = HALOLED_COLOR_BLUE;
					break;
				case HALOLED_COLOR_BLUE:
					m_OpInfo.nHaloLedColor = HALOLED_COLOR_YELLOWGREEN;
					break;
				case HALOLED_COLOR_YELLOWGREEN:
					m_OpInfo.nHaloLedColor = HALOLED_COLOR_WHITE;
					break;
				case HALOLED_COLOR_WHITE:
					m_OpInfo.nHaloLedColor = HALOLED_COLOR_GREEN;
					break;
				case HALOLED_COLOR_GREEN:
					m_OpInfo.nHaloLedColor = HALOLED_COLOR_PINK;
					break;
				default:
					m_OpInfo.nHaloLedColor = HALOLED_COLOR_CYAN;
					break;
			}
			SetHaloLedControl(m_OpInfo.nHaloLedColor, SKIP_HALOLED_MODE, SKIP_MCULED);

			// Delay Check
			if (m_OpInfo.nHaloLedMode == FLICKING_HALOLED_MODE)
				Delay_Msg(500);
		}
		else if (m_OpInfo.strReturn == L"MCUFLICKER")
		{
			if (m_OpInfo.strMCUFlicker == L"OFF")
			{
				m_OpInfo.strMCUFlicker = L"ON";
				SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, ON_MCULED);
			}
			else if (m_OpInfo.strMCUFlicker == L"ON")
			{
				m_OpInfo.strMCUFlicker = L"FLICKING";
				SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, FLICKER_MCULED);
			}
			else if (m_OpInfo.strMCUFlicker == L"FLICKING")
			{
				m_OpInfo.strMCUFlicker = L"OFF";
				SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);
			}
			else
			{
				// 비정상적인 조건에서는 무조건 OFF로 처리
				m_OpInfo.strMCUFlicker = L"OFF";
				SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);
			}
		}

		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 827;
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
			break;
		}		
	} // end of while

	m_OpInfo.nHaloLedMode	 = OFF_HALOLED_MODE;
	m_OpInfo.nHaloLedColor	 = HALOLED_COLOR_CYAN;
	m_OpInfo.strMCUFlicker	 = L"OFF";

	SetHaloLedControl(HALOLED_COLOR_CYAN, OFF_HALOLED_MODE, OFF_MCULED);

	NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkHaloLedNMCUFlickerProc() return \n")));

	return TRUE;
}
// end of [#2205]

// [#2518] US Kook 2017.12.21 Support MX-2800SE
/*-------------------------------------------------------------------
CLASS    NAME: CMainFrame
FUNCTION NAME: ClerkDiagnosticsCamera()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : SCR 765 PROC
-------------------------------------------------------------------*/
int CMainFrame::ClerkDiagnosticsCamera()
{
	NHDEBUG(DBG_CALL, (L"START\n"));

#ifdef UNDER_CE
	// Turn on camera preview.
	// Currently 448 x 336 is maximum available resolution. (due to memory limit?)
 	long width = NH_CAM_RESOLUTION_WIDTH + ((8*4) * 4);			// 448 (must be multiple of 8)
 	long height = NH_CAM_RESOLUTION_HEIGHT + ((8*3) * 4);		// 336 (must be multiple of 8)
 
	m_pDevCmn->fnCAM_Initialize();
   	m_pDevCmn->fnCAM_StartAntiShoulderSurfing((1024 - width) / 2, (768 - height) / 2, width, height);

	while(TRUE)
	{
		NHDEBUG(DBG_INFO, (L"WAIT FOR USER INPUT\n"));

		///////////////////////////////////
		// WAIT FOR USER INPUT
		ClerkWaitScreenInput_byCha();
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

		NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

		//-----------------------------------
		// PROCESS KEY STRING
		//-----------------------------------

		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
 			m_OpInfo.nProcNextScrNum = 827;
			break;
		}
 		else if (m_OpInfo.strReturn == S_TIMEOVER)
 		{
 			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
 			m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
 			break;
 		}
	} // end of while

	// turn off PREVIEW
	m_pDevCmn->fnCAM_StopAntiShoulderSurfing();
	m_pDevCmn->fnCAM_Deinitialize();

	NHDEBUG(DBG_CALL, (L"END\n"));
#endif

	return TRUE;
}
// end of [#2518]

// [#GLDV-3005] US Kook 2021.10.07 Support Side Car
/*-------------------------------------------------------------------
CLASS    NAME: CMainFrame
FUNCTION NAME: ClerkDiagnosticsBarcodeReader()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : SCR 770 PROC
-------------------------------------------------------------------*/
int CMainFrame::ClerkDiagnosticsBarcodeReader()
{
	NHDEBUG(DBG_CALL, (_T("ClerkDiagnosticsBarcodeReader() \n")));
	int nReturn = 0;
	int nDeviceCmd = 0;
	BOOL bCommandExecuted = FALSE;
	BOOL bScreenDisplay = TRUE;
	BOOL bShowScreen = TRUE;

	m_VdmInfo.strBarcodeData.Empty();

	while(TRUE)
	{
		///////////////////////////////////
		// DISPLAY OR UPDATE SCREEN
		if (bShowScreen == TRUE)
		{
			ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
			m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
			bShowScreen = FALSE;
			m_OpInfo.nProcLoofCnt = 0;
		}
		else if (m_OpInfo.nProcLoofCnt >= (SCR_UPDATECYCLE / 2))
		{
			if (m_pDiagBcr->GetData2() == TRUE)
			{
				if (m_pDiagBcr->GetBarcodeData().GetLength() > 0)
					m_VdmInfo.strBarcodeData = m_pDiagBcr->GetBarcodeData();
			}

			ClerkCheckDeviceStatus();
			ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
			m_OpInfo.nProcLoofCnt = 0;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
			m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

			NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));
			//-----------------------------------
			// PROCESS KEY STRING
			//-----------------------------------
			if (m_OpInfo.strReturn == L"ENABLE")
			{
				m_VdmInfo.strBarcodeData.Empty();

				int nRet = WFS_ERR_HARDWARE_ERROR;
				if (m_bDiagBCROpen == TRUE)
				{
					m_pDiagBcr->ClearError();
					nRet = m_pDiagBcr->Enable();

					NHDEBUG(1, (_T("m_pDiagBcr->Enable() : %s \n"), (nRet == TRUE) ? L"OK" : L"NG"));
					ClerkNoticeToUser((nRet == TRUE) ? T_OPERATE_OK : T_OPERATE_NG);

					if (nRet != TRUE)
					{
						m_VdmInfo.strBarcodeData = m_pDiagBcr->GetErrorCode();
					}
				}
				else
				{
					ClerkNoticeToUser(T_DEVICE_ERR);
				}
			}
			else if (m_OpInfo.strReturn == L"DISABLE")
			{
				m_VdmInfo.strBarcodeData.Empty();

				int nRet = WFS_ERR_HARDWARE_ERROR;
				if (m_bDiagBCROpen == TRUE)
				{
					m_pDiagBcr->ClearError();
					nRet = m_pDiagBcr->Disable();

					NHDEBUG(1, (_T("m_pDiagBcr->Disable() : %s \n"), (nRet == TRUE) ? L"OK" : L"NG"));
					ClerkNoticeToUser((nRet == TRUE) ? T_OPERATE_OK : T_OPERATE_NG);

					if (nRet != TRUE)
					{
						m_VdmInfo.strBarcodeData = m_pDiagBcr->GetErrorCode();
					}
				}
				else
				{
					ClerkNoticeToUser(T_DEVICE_ERR);
				}
			}

			//-----------------------------------
			// COMMON PROCESS
			//-----------------------------------
			if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
				m_OpInfo.nProcNextScrNum = 827;
				break;
			}
			else if (m_OpInfo.strReturn == S_TIMEOVER)
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
				m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
				break;
			}

			bShowScreen = TRUE;
		}

		Delay_Msg(PROC_LOOFDELAY);
		m_OpInfo.nProcLoofCnt++;
	} // end of while

	if (m_bDiagBCROpen == TRUE)
	{
		int nRet = m_pDiagBcr->Disable();
		NHDEBUG(1, (_T("m_pDiagBcr->Disable() : %s \n"), (nRet == TRUE) ? L"OK" : L"NG"));
	}

	NHDEBUG(DBG_CALL, (_T("ClerkDiagnosticsBarcodeReader() return \n")));

	return TRUE;
}


/*-------------------------------------------------------------------
CLASS    NAME: CMainFrame
FUNCTION NAME: ClerkDiagnosticsBillAcceptor()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : SCR 771 PROC
-------------------------------------------------------------------*/
int CMainFrame::ClerkDiagnosticsBillAcceptor()
{
	NHDEBUG(DBG_CALL, (L"START\n"));

	m_VdmInfo.nAcceptValue = 0;
	m_VdmInfo.nAcceptCount = 0;
	m_VdmInfo.nRejectCount = 0;
	m_VdmInfo.strBarcodeData.Empty();
	m_VdmInfo.bBarcodeAccepted = FALSE;
	m_VdmInfo.bBillAccepted = FALSE;

	BOOL	bShowScreen = TRUE;

	// JCM requires Initialize() before Accept2() (do lazy initialization for faster VDM entrance)
	if (m_bDiagBNAOpen == TRUE)
	{
		m_pDiagBna->Initialize();
	}

	while(TRUE)
	{
		///////////////////////////////////
		// DISPLAY OR UPDATE SCREEN
		if (bShowScreen == TRUE)
		{
			ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
			m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
			bShowScreen = FALSE;
			m_OpInfo.nProcLoofCnt = 0;
		}
		else if (m_OpInfo.nProcLoofCnt >= SCR_UPDATECYCLE)
		{
			ClerkCheckDeviceStatus();
			ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
			m_OpInfo.nProcLoofCnt = 0;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
			m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

			NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));
			//-----------------------------------
			// PROCESS KEY STRING
			//-----------------------------------
			if (m_OpInfo.strReturn == L"INITIALIZE")
			{
				int nRet = WFS_ERR_HARDWARE_ERROR;

				if (m_bDiagBNAOpen == TRUE)
				{
					// clear variables
					m_VdmInfo.bBillAccepted = FALSE;
					m_VdmInfo.nAcceptValue = 0;
					m_VdmInfo.nAcceptCount = 0;
					m_VdmInfo.nRejectCount = 0;

					ClerkNoticeToUser(T_PROCESS_WAIT);
					m_pDiagBna->ClearError();
					nRet = m_pDiagBna->Initialize();
				}

				if (nRet == WFS_SUCCESS)
				{
					NHDEBUG(1, (_T("m_pDiagBna->Initialize() : SUCCESS \n")));
					ClerkNoticeToUser(T_OPERATE_OK);
				}
				else
				{
					ClerkNoticeToUser(T_DEVICE_ERR);
				}
			}
			else if (m_OpInfo.strReturn == L"ACCEPTBILL")
			{
				int nRet = WFS_ERR_HARDWARE_ERROR;

				if (m_bDiagBNAOpen == TRUE)
				{
					// clear variables
					m_VdmInfo.bBillAccepted = FALSE;
					m_VdmInfo.nAcceptValue = 0;
					m_VdmInfo.nAcceptCount = 0;
					m_VdmInfo.nRejectCount = 0;

					ClerkNoticeToUser(_T("BRING A NOTE TO BE ACCEPTED"));

					nRet = m_pDiagBna->Accept2();
					if(nRet == WFS_SUCCESS)
					{
						m_VdmInfo.bBillAccepted = TRUE;
						m_VdmInfo.nAcceptValue = m_pDiagBna->GetAcceptValue();
						m_VdmInfo.nAcceptCount = m_pDiagBna->GetAcceptCount();
						m_VdmInfo.nRejectCount = m_pDiagBna->GetRejectCount();

						NHDEBUG(DBG_INFO, (_T("AcceptedValue: [%d] AcceptedCount: [%d] RejectedCount: [%d]\n"), m_VdmInfo.nAcceptValue, m_VdmInfo.nAcceptCount, m_VdmInfo.nRejectCount));

						if (m_VdmInfo.nAcceptCount == 0 && m_VdmInfo.nRejectCount > 0)
						{
							NHDEBUG(DBG_INFO, (_T("Failed to accept a note!\n")));

							m_VdmInfo.bBillAccepted = FALSE;
							nRet = WFS_ERR_CANCELED;
						}
					}
				}

				if (nRet == WFS_SUCCESS)
				{
					NHDEBUG(1, (_T("m_pDiagBna->Accept2() : SUCCESS \n")));
					ClerkNoticeToUser(T_OPERATE_OK);
				}
				else
				{
					ClerkNoticeToUser(T_DEVICE_ERR);
				}
			}
			else if (m_OpInfo.strReturn == L"CANCELBILL")
			{
				int nRet = WFS_ERR_HARDWARE_ERROR;

				if (m_bDiagBNAOpen == TRUE)
				{
					ClerkNoticeToUser(T_PROCESS_WAIT);
					m_pDiagBna->ClearError();

					// eject if there is an accepted note
					if(m_bDiagBNAOpen == TRUE)
					{
						if (m_pDiagBna->Eject() == WFS_SUCCESS)
							NHDEBUG(1, (_T("m_pDiagBna->Eject() : SUCCESS \n")));
						else
							NHDEBUG(1, (_T("m_pDiagBna->Eject() : FAILURE \n")));
					}

					nRet = m_pDiagBna->CancelAccept2();
				}

				if (nRet == WFS_SUCCESS)
				{
					NHDEBUG(1, (_T("m_pDiagBna->CancelAccept2() : SUCCESS \n")));
					m_VdmInfo.bBillAccepted = FALSE;
					ClerkNoticeToUser(T_OPERATE_OK);
				}
				else
				{
					ClerkNoticeToUser(T_DEVICE_ERR);
				}

				// clear variables
				m_VdmInfo.bBillAccepted = FALSE;
				m_VdmInfo.nAcceptValue = 0;
				m_VdmInfo.nAcceptCount = 0;
				m_VdmInfo.nRejectCount = 0;
			}
			else if (m_OpInfo.strReturn == L"STACK")
			{
				int nRet = WFS_ERR_HARDWARE_ERROR;

				if (m_bDiagBNAOpen == TRUE)
				{
					ClerkNoticeToUser(T_PROCESS_WAIT);
					nRet = m_pDiagBna->Stack();
				}

				if(nRet == WFS_SUCCESS)
				{
					NHDEBUG(1, (_T("m_pDiagBna->Stack() : SUCCESS \n")));
					
					CString strTemp;
					strTemp.Format(L"BnaStk(%d,%d)", m_VdmInfo.nAcceptValue, m_VdmInfo.nAcceptCount);
					NVDump('O', 'A', "11", L"", strTemp);

					ClerkNoticeToUser(T_OPERATE_OK);
				}
				else
				{
					ClerkNoticeToUser(T_DEVICE_ERR);
				}

				m_VdmInfo.bBillAccepted = FALSE;
			}
			else if (m_OpInfo.strReturn == L"EJECT")
			{
				int nRet = WFS_ERR_HARDWARE_ERROR;

				if(m_bDiagBNAOpen == TRUE)
				{
					ClerkNoticeToUser(T_PROCESS_WAIT);
					nRet = m_pDiagBna->Eject();
				}

				if(nRet == WFS_SUCCESS)
				{
					NHDEBUG(1, (_T("m_pDiagBna->Eject() : SUCCESS \n")));
					ClerkNoticeToUser(T_OPERATE_OK);
				}
				else
				{
					ClerkNoticeToUser(T_DEVICE_ERR);
				}

				m_VdmInfo.bBillAccepted = FALSE;
			}
			else if (m_OpInfo.strReturn == L"ACCEPTBARCODE")
			{
				int nRet = WFS_ERR_HARDWARE_ERROR;

				if(m_bDiagBNAOpen == TRUE)
				{
					ClerkNoticeToUser(T_PROCESS_WAIT);

					nRet = m_pDiagBna->Accept3();
					if (nRet == WFS_SUCCESS)
					{
						m_VdmInfo.bBarcodeAccepted = TRUE;
						NHDEBUG(1, (_T("m_pDiagBna->Accept3() : SUCCESS \n")));
						ClerkNoticeToUser(T_OPERATE_OK);

						m_pDiagBna->QueryBarcodeData();
						m_VdmInfo.strBarcodeData = m_pDiagBna->GetBarcodeData();
					}
					else
					{
						m_VdmInfo.bBarcodeAccepted = FALSE;
						ClerkNoticeToUser(T_DEVICE_ERR);
					}
				}
			}
			else if (m_OpInfo.strReturn == L"CANCELBARCODE")
			{
				int nRet = WFS_ERR_HARDWARE_ERROR;

				if (m_bDiagBNAOpen == TRUE)
				{
					ClerkNoticeToUser(T_PROCESS_WAIT);
					m_pDiagBna->ClearError();
					nRet = m_pDiagBna->CancelAccept3();
				}

				if (nRet == WFS_SUCCESS)
				{
					NHDEBUG(1, (_T("m_pDiagBna->CancelAccept3() : SUCCESS \n")));
					m_VdmInfo.bBarcodeAccepted = FALSE;
					m_VdmInfo.strBarcodeData.Empty();
					m_pDiagBna->Eject();
					ClerkNoticeToUser(T_OPERATE_OK);
				}
				else
				{
					ClerkNoticeToUser(T_DEVICE_ERR);
				}
			}

			//-----------------------------------
			// COMMON PROCESS
			//-----------------------------------
			if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
				m_OpInfo.nProcNextScrNum = 827;
				break;
			}
			else if (m_OpInfo.strReturn == S_TIMEOVER)
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
				m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
				break;
			}

			bShowScreen = TRUE;
		}

		Delay_Msg(PROC_LOOFDELAY);
		m_OpInfo.nProcLoofCnt++;

	} // end of while

	NHDEBUG(DBG_CALL, (L"END\n"));

	return TRUE;
}
// end of [#GLDV-3005]


/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDiagnosticsExitScreen()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 829 PROC
-------------------------------------------------------------------*/
int	CMainFrame::ClerkDiagnosticsExitScreen()
{
NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsExitScreen() \n")));
	// VDM, CDU SETUP, KEYMGR를 빠져 나올 경우 Confirm 화면 표시하도록 수정
	m_OpInfo.nProcLoofCnt = 0;
	m_OpInfo.nMsgClearCount = 0;

	while(TRUE)
	{
		// Clerk 화면 Display, Update 한다. 
		// Screen Input 발생 시 리턴된다.
		ClerkWaitScreenInput(TRUE);
		
		// Get Event String
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();
		SplitString(m_OpInfo.strReturn, SCR_RES_DELIMITER, m_OpInfo.strTempArray);
		
		if ((m_OpInfo.strReturn == L"YES") || (m_OpInfo.strReturn == S_TIMEOVER))
		{
			if (m_OpInfo.nProcPrevScrNum == 845)
			{			
				/*************************************/
				/* Close CDU & Set Exit Message		 */
				/*************************************/

				NVDump('O', 'A', "11", L"", L"CDU_CLOSE");		// [#2024] NH KSK 2011.02.24
		
NHDEBUG(DIAG_TEST, (_T("[CMainFrame::ClerkDiagnosticsExitScreen] CLOSING CDU PORT(845)+\n")));
				ClerkNoticeToUser(m_pDevCmn->fstrSCR_GetStringFromTextID(T_CLOSING_CDU));	// [#2160] CA KMK 2012.11.16 French OP
				if(m_pDiagCdu != NULL)
				{
					m_pDiagCdu->Close();
					/* Wait to close device's serial port 1 sec. */
					Delay_Msg(1000);
					
					m_bDiagCDUOpen = FALSE;
					delete m_pDiagCdu;
					m_pDiagCdu = NULL;
					
				}

				NVDump('O', 'A', "11", L"", L"CDU_CLOSE_OK");		// [#2024] NH KSK 2011.02.24
				
				NVDump('O', 'A', "11", L"", L"CDU_INIT");		// [#2024] NH KSK 2011.02.24
				/*************************************/
				/* INITIALIZE CDU & Set Exit Message */
				/*************************************/
NHDEBUG(DIAG_TEST, (_T("[CMainFrame::ClerkDiagnosticsExitScreen] INITIALIZING CDU(845)+\n")));
				ClerkNoticeToUser(m_pDevCmn->fstrSCR_GetStringFromTextID(T_INITIALIZING_CDU));	// [#2160] CA KMK 2012.11.16 French OP
				m_pDevCmn->fnAPL_ResetDevice(DEV_CDU);
				
				if (m_OpInfo.nMoveProcReason == MOVE_PROC_TIMEOUT)
					m_OpInfo.nProcNextScrNum = OP_TIMEOUT_SCREEN_NUMBER;
				else
					m_OpInfo.nProcNextScrNum = 844;
				NVDump('O', 'A', "11", L"", L"CDU_INIT_OK");		// [#2024] NH KSK 2011.02.24
			}
			else
			{
				// Close devices using multi-thread
				ClerkDiagnosticsDeviceHandler(OFF);

				/*************************************/
				/* INITIALIZE SPR & MCU & CDU & RFID Set Exit Message */
				/*************************************/
				// Device 장애 시 장애가 Clear되는 Bug Fix
				NVDump('O', 'A', "11", L"", L"DEV_INIT");

				// [#2220] AU KMK 2014.02.26 호주용 문구 추가 (INITIALIZING -> INITIALISING)
// 				ClerkNoticeToUser(m_pDevCmn->fstrSCR_GetStringFromTextID(T_INITIALIZING_DEVICES));	// [#2160] CA KMK 2012.11.16 French OP
				ClerkNoticeToUser(m_pDevCmn->fstrSCR_GetStringByTextID(_T("INITIALIZING DEVICES")));
				// end of [#2220]

				// [#GLDV-3005] US Kook 2022.01.11 Side Car
				/*
				// [#2325] NH KSK 2015.01.20
//				m_pDevCmn->fnAPL_InitializeDevice(DEV_SPR | DEV_MCU | DEV_CDU);
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
					m_pDevCmn->fnAPL_InitializeDevice(DEV_SPR | DEV_MCU | DEV_CDU | DEV_RFID);
				else
					m_pDevCmn->fnAPL_InitializeDevice(DEV_SPR | DEV_MCU | DEV_CDU);
				// end of [#2325]
				*/



				m_pDevCmn->fnAPL_InitializeDevice(m_pDevCmn->fnCMN_GetActiveDevice());
				// end of [#GLDV-3005]
				

				NVDump('O', 'A', "11", L"", L"DEV_INIT_END");

				NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsExitScreen() return \n")));

				if (m_OpInfo.nMoveProcReason == MOVE_PROC_TIMEOUT)
				{
					m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;//이렇게 한 이유라도??
					m_OpInfo.nProcNextScrNum = OP_TIMEOUT_SCREEN_NUMBER;
				}
				else
				{
					m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
					m_OpInfo.nProcNextScrNum = 901;
				}
			}
			
			m_pDevCmn->CSTCnt = 0;
			m_pDevCmn->fnAPL_CheckDevice();
			m_pDevCmn->fnAPL_SetSensorInfo(TRUE);	// REJECT COUNT REFRESH
			break;
		}
		else
		{
			m_OpInfo.nProcNextScrNum = m_OpInfo.nProcPrevScrNum;
			break;
		}

		m_OpInfo.nMsgClearCount = SCR_OPMSGCYCLE;	// about 2 sec.
		// 데이터 업데이트
		ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);
	}
	
	return TRUE;
}

int	CMainFrame::ClerkDiagnosticsKeyMatrix()
{
	return TRUE;
}

/*-------------------------------------------------------------------
CLASS    NAME: CMainFrame
FUNCTION NAME: ClerkDiagnosticsMagtekCard()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : SCR 856 PROC
-------------------------------------------------------------------*/
int	CMainFrame::ClerkDiagnosticsMagtekCard()
{
	NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsMagtekCard() \n")));

	CARD_TRACK_DATA TrackData;
	CString			strTmp;
	int				nResult = 0;
	int				nDeviceCmd = 0;			// [#581] NH KSK 2009.12.02
	BOOL			bReadCmd  = FALSE;		// [#581] NH KSK 2009.12.02
	BOOL			bUserCancel = FALSE;	// [#581] NH KSK 2009.12.02

	m_OpInfo.nProcLoofCnt = 0;
	m_OpInfo.nMsgClearCount = 0;
	m_OpInfo.nMoveProcReason = MOVE_PROC_SELECT;

	m_OpInfo.strISO1Track.Empty();
	m_OpInfo.strISO2Track.Empty();
	m_OpInfo.strISO3Track.Empty();
	m_OpInfo.strATRData.Empty();
	m_OpInfo.nISO1Length = 0;
	m_OpInfo.nISO2Length = 0;
	m_OpInfo.nISO3Length = 0;

	while(TRUE)
	{
		// Clerk 화면 Display, Update 한다. 
		// Screen Input 발생 시 리턴된다.
		ClerkWaitScreenInput(TRUE);

		// Get Event String
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();
		SplitString(m_OpInfo.strReturn, SCR_RES_DELIMITER, m_OpInfo.strTempArray);

		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 821;
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
			break;
		}
		else
		{
			m_OpInfo.strISO1Track.Empty();
			m_OpInfo.strISO2Track.Empty();
			m_OpInfo.strISO3Track.Empty();
			m_OpInfo.strATRData.Empty();
			m_OpInfo.nISO1Length = 0;
			m_OpInfo.nISO2Length = 0;
			m_OpInfo.nISO3Length = 0;
			TrackData.Iso1_Length = 0;	// KSK 2009.9.9 Codesonar 지적사항 대책
			TrackData.Iso2_Length = 0;	// KSK 2009.9.9 Codesonar 지적사항 대책
			TrackData.Iso3_Length = 0;	// KSK 2009.9.9 Codesonar 지적사항 대책

			nResult = m_pDiagMcu->Initialize();

			if( nResult == MCU_SUCCESS)
			{
				if (m_OpInfo.strReturn == L"ICTEST")
				{
					// LATCH
					// [#2078] NH KSK 2011.07.15 SANKYO는 Latch 제어 없음
					if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_MAGTEK_DIP)
						nResult = m_pDiagMcu->ICCardLatch();
					else
						nResult = MCU_SUCCESS;
					// end of [#2078]

					if (nResult == MCU_SUCCESS)
					{
						// [#2078] NH KSK 2011.07.15
						if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_MAGTEK_DIP)
						{
							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_LATCH);	// [#2160] CA KMK 2012.11.16 French OP
							ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);
							Delay_Msg(SCR_OPERATION_WAIT);
						}
						// end of [#2078]

						// POWER ON
						BYTE	chATRData[1024] = { 0, };
						WORD	wATRLen = 0;
						CString sTemp;
						nResult = m_pDiagMcu->ICCardPowerOn(chATRData, &wATRLen);

						for(int i=0; i < (WORD)wATRLen ; i++)
						{
							sTemp.Format(_T("%2.2X "), chATRData[i]);
							m_OpInfo.strATRData += sTemp;
						}

						if (nResult == MCU_SUCCESS)
							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_POWER_ON);	// [#2160] CA KMK 2012.11.16 French OP
						else
							m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_POWER_ON);	// [#2160] CA KMK 2012.11.16 French OP

						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);
						Delay_Msg(SCR_OPERATION_WAIT);

						// UNLATCH
						// [#2078] NH KSK 2011.07.15
						if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_MAGTEK_DIP)
						{
							nResult = m_pDiagMcu->ICCardUnLatch();

							if (nResult == MCU_SUCCESS)
								m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_UNLATCH);	// [#2160] CA KMK 2012.11.16 French OP
							else
								m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_UNLATCH);	// [#2160] CA KMK 2012.11.16 French OP
						}
						else
						{
							// SANKYO인 경우는 UnLatch를 안하므로 Power Off를 Call해야함
							nResult = m_pDiagMcu->ICCardPowerOff();

							if (nResult == MCU_SUCCESS)
								m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_POWER_OFF);	// [#2160] CA KMK 2012.11.16 French OP
							else
								m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_POWER_OFF);	// [#2160] CA KMK 2012.11.16 French OP
						}

						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);
						Delay_Msg(SCR_OPERATION_WAIT);
					}
					else
						m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_LATCH);	// [#2160] CA KMK 2012.11.16 French OP
				}
				else if (m_OpInfo.strReturn == L"CARDSCAN")
				{
					// [#581] NH KSK 2009.12.02
					m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_REMOVE_CARD);	// [#2160] CA KMK 2012.11.16 French OP
					ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);

					m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);	// [#126] OP Process 진행 중 Key Disable 예외 처리

					m_pDevCmn->fnSCR_SetDisplayData(L"APTimeoutMode", L"TimeoutLast");		// NH PSC 2008.07.07 소스리뷰결과 공통적용.
					m_pDevCmn->fnSCR_SetDisplayData(L"APMaxTime", Int2Asc(m_OpInfo.nTimeout));

					while (TRUE)
					{
						if (bReadCmd == FALSE)
						{
							nResult = m_pDiagMcu->ICCardAccept(MCU_TIMEOUT, FALSE, &nDeviceCmd);	// 3초 설정
							bReadCmd = TRUE;
						}
						else
						{
							nResult = m_pDiagMcu->CompleteAction(nDeviceCmd, MCU_IO_COMPLETED_CHECK_TIME);

							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagMcu->CompleteAction() nReturn[%d]\n"), nResult));	

							if (nResult == 1)
							{
								// Next Process
								// [#2078] NH KSK 2011.07.15
								if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_MAGTEK_DIP)
									nResult = m_pDiagMcu->ICCardRemove();
								else
									nResult = 1;
								// end of [#2078]

								if (nResult == 1)
								{
									// Next Process
									m_pDiagMcu->ICCardRead(&TrackData);

									m_OpInfo.nISO1Length = TrackData.Iso1_Length;
									m_OpInfo.nISO2Length = TrackData.Iso2_Length;
									m_OpInfo.nISO3Length = TrackData.Iso3_Length;

									if ( TrackData.Iso1_Length >= 55 )
									{
										strTmp.Format(_T("%S"), TrackData.Iso1_Data);
										m_OpInfo.strISO1Track.Format(_T("%s\n%s"),strTmp.Left(50), strTmp.Mid(50));

									}
									else
										m_OpInfo.strISO1Track.Format(_T("%S"), TrackData.Iso1_Data);

									if ( TrackData.Iso2_Length >= 55 )
									{
										strTmp.Format(_T("%S"), TrackData.Iso2_Data);
										m_OpInfo.strISO2Track.Format(_T("%s\n%s"),strTmp.Left(50), strTmp.Mid(50));

									}
									else
										m_OpInfo.strISO2Track.Format(_T("%S"), TrackData.Iso2_Data);

									if ( TrackData.Iso3_Length >= 55 )
									{
										strTmp.Format(_T("%S"), TrackData.Iso3_Data);
										m_OpInfo.strISO3Track.Format(_T("%s\n%s"),strTmp.Left(50), strTmp.Mid(50));

									}
									else
										m_OpInfo.strISO3Track.Format(_T("%S"), TrackData.Iso3_Data);

									NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsMagtekCard() : TrackData.Iso1_Length[%d], TrackData.Iso2_Length[%d]\n"),TrackData.Iso1_Length, TrackData.Iso2_Length));

									// KSK 2009.08.04 Track Data가 없을 경우 FAIL 처리 추가
									if (m_OpInfo.strISO1Track.GetLength() > 0 || m_OpInfo.strISO2Track.GetLength() > 0 || m_OpInfo.strISO3Track.GetLength() > 0)
										m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_SUCCESS_READ);	// [#2160] CA KMK 2012.11.16 French OP
									else
										m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_READ);	// [#2160] CA KMK 2012.11.16 French OP
									// end of KSK 2009.08.04							
								}
								else
								{
									m_OpInfo.strTempClerkMessage.Format(L"ERROR [%s]", m_pDiagMcu->GetErrorString());
								}
								bReadCmd = FALSE;
								break;
							}
							// [#2078] NH KSK 2011.07.15 
							else if (nResult != IO_PROCESSING)
							{
								bReadCmd = FALSE;
							}
							// end of [#2078]
						}

						// check input
						if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
						{
							// Get Event String
							m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();
							SplitString(m_OpInfo.strReturn, SCR_RES_DELIMITER, m_OpInfo.strTempArray);

							if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
							{
								if (bReadCmd == TRUE)
								{
									nResult = m_pDiagMcu->CompleteAction(nDeviceCmd, MCU_IO_COMPLETED_CHECK_TIME);

									while( nResult == IO_PROCESSING)
									{
										nResult = m_pDiagMcu->CompleteAction(nDeviceCmd, MCU_IO_COMPLETED_CHECK_TIME);
										NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsCardScan() Wait Timeout nResult(%d)\n"), nResult));
									}
									bReadCmd = FALSE;
								}

								m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
								m_OpInfo.nProcNextScrNum = 821;
								NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsCardScan() KEY PRESSED \n")));
								bUserCancel = TRUE;
								break;
							}
							else if (m_OpInfo.strReturn == S_TIMEOVER)
							{
								if (bReadCmd == TRUE)
								{
									nResult = m_pDiagMcu->CompleteAction(nDeviceCmd, MCU_IO_COMPLETED_CHECK_TIME);

									while( nResult == IO_PROCESSING)
									{
										nResult = m_pDiagMcu->CompleteAction(nDeviceCmd, MCU_IO_COMPLETED_CHECK_TIME);
										NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsCardScan() Wait Timeout nResult(%d)\n"), nResult));
									}
									bReadCmd = FALSE;
								}

								m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
								m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
								bUserCancel = TRUE;
								break;
							}
						}

						if (bReadCmd == TRUE)
							Delay_Msg(500);	// KSK 2009.08.07 500ms으로 Delay 조정
					}
				}
			}
			else
			{
				m_OpInfo.strTempClerkMessage = m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_INIT);	// [#2160] CA KMK 2012.11.16 French OP
			}
		}

		// 데이터 업데이트
		if (bUserCancel != TRUE)	// KSK 2011.04.02 USER CANCEL시 잠시 이전 DATA가 표시되는 Bug Fix
			ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);

		if (bUserCancel == TRUE)
			break;
		// end of [#581]
	} // end of while

	NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsMagtekCard() return \n")));

	return TRUE;
}

// end of [#540]


// [#2325] NH KSK 2015.01.28
/*-------------------------------------------------------------------
CLASS    NAME: CMainFrame
FUNCTION NAME: ClerkDiagnosticsRFIDScan()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : SCR 751 PROC
-------------------------------------------------------------------*/
int	CMainFrame::ClerkDiagnosticsRFIDCardScan()
{
	NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsRFIDCardScan() \n")));
	int nReturn = 0;
	int nDeviceCmd = 0;
	BOOL bReadCmd = FALSE;
	BOOL bScreenDisplay = TRUE;

	m_OpInfo.nProcLoofCnt = 0;
	m_OpInfo.nMsgClearCount = 0;
	m_OpInfo.nMoveProcReason = MOVE_PROC_SELECT;

	m_OpInfo.strISO1Track.Empty();
	m_OpInfo.strISO2Track.Empty();
	m_OpInfo.strISO3Track.Empty();
	m_OpInfo.nISO1Length = 0;
	m_OpInfo.nISO2Length = 0;
	m_OpInfo.nISO3Length = 0;

	RFVDMCARDDATA	m_RFIDTrackData[5];

	memset(m_RFIDTrackData, 0, sizeof(m_RFIDTrackData));

	while(TRUE)
	{
		while(TRUE)
		{
			// OP MSG Clear
			m_OpInfo.strTempClerkMessage = _T(" ");

			// Display Screen
			if ((m_OpInfo.nCurScrNum != m_OpInfo.nProcNextScrNum) || (bScreenDisplay == TRUE))
			{
				bScreenDisplay = FALSE;
				ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout, FALSE, PIN_AMOUNT_MODE);
			}
			else
			{
				ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
			}

			if( bReadCmd == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::m_pDiagRfid->ReadData2() Start !!!!\n")));
				memset(m_RFIDTrackData, 0, sizeof(m_RFIDTrackData));

				// 0x01 : Track1, 0x02 : Track2
				// 0x08 : ATR
				m_pDiagRfid->ReadRawData(RFID_READ_CHECK_TIME, FALSE, &nDeviceCmd);
				bReadCmd = TRUE;
			}
			else
			{
				nReturn = m_pDiagRfid->CompleteAction(nDeviceCmd, RFID_IO_COMPLETED_CHECK_TIME);

				NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::m_pDiagRfid->CompleteAction nReturn[%d]\n"), nReturn));	

				if ( nReturn == RFID_SUCCESS )
				{
					CString strTmp;
					CString strTmp2;

					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsRFIDCardScan() RFID READ SUCCESS\n")));

					m_pDiagRfid->RFID_GetTrackData(m_RFIDTrackData);

					m_OpInfo.nISO1Length = m_RFIDTrackData[0].ulDataLength;
					m_OpInfo.nISO2Length = m_RFIDTrackData[1].ulDataLength;

					if ( m_OpInfo.nISO1Length >= 55 )
					{
						strTmp.Format(_T("%S"), m_RFIDTrackData[0].byData);
						m_OpInfo.strISO1Track.Format(_T("%s\n%s"),strTmp.Left(50), strTmp.Mid(50));

					}
					else
					{
						// show hex-typed ATR, instead of Track data.
// 						m_OpInfo.strISO1Track.Format(_T("%S"), m_RFIDTrackData[0].byData);
						m_OpInfo.strISO1Track.Empty();
						for( int nIdx =0; nIdx < m_OpInfo.nISO1Length; nIdx++ )
						{
							int n = m_RFIDTrackData[0].byData[nIdx];
							CString csTemp;
							csTemp.Format( _T("%02x "), n );
							m_OpInfo.strISO1Track += csTemp.MakeUpper();
						}
					}

					if ( m_OpInfo.nISO2Length >= 55 )
					{
						strTmp.Format(_T("%S"), m_RFIDTrackData[1].byData);
						m_OpInfo.strISO2Track.Format(_T("%s\n%s"),strTmp.Left(50), strTmp.Mid(50));

					}
					else
					{
						m_OpInfo.strISO2Track.Format(_T("%S"), m_RFIDTrackData[1].byData);
					}

					// RFID는 TRACK3 미지원함

					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsRFIDCardScan() : TrackData.Iso1_Length[%d], TrackData.Iso2_Length[%d]\n"),m_OpInfo.nISO1Length, m_OpInfo.nISO2Length));

					bReadCmd = FALSE;

					// KSK 2009.08.04 Track Data가 없을 경우 FAIL 처리 추가
					if (m_OpInfo.strISO1Track.GetLength() > 0 || m_OpInfo.strISO2Track.GetLength() > 0)
					{
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);
					}
					else
					{
						ClerkNoticeToUser(m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_READ), TRUE);
						bScreenDisplay = TRUE;
					}
					// end of KSK 2009.08.04

					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsRFIDCardScan() : RFID_IO_COMPLETED \n")));
				}
				else if ( nReturn == IO_PROCESSING )
				{
//					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsRFIDCardScan() : RFID_IO_PROCESSING \n")));
				}
				else if (nReturn == RFID_ERROR)
				{
					// 단선시 무응답 코드이므로 ERROR 표시 후 이전 화면으로 전환 한다.
					ClerkNoticeToUser(m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_READ), TRUE);

					NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsRFIDCardScan() return \n")));
					m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
					m_OpInfo.nProcNextScrNum = 827;
					return TRUE;
				}
				else if ( nReturn == RFID_FAIL )
				{
					m_OpInfo.strISO1Track.Empty();
					m_OpInfo.strISO2Track.Empty();
					m_OpInfo.strISO3Track.Empty();
					m_OpInfo.nISO1Length = 0;
					m_OpInfo.nISO2Length = 0;
					m_OpInfo.nISO3Length = 0;

					ClerkNoticeToUser(m_pDevCmn->fstrSCR_GetStringFromTextID(T_FAIL_READ), TRUE);
					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsRFIDCardScan() : RFID_IO_FAIL [%d]\n"), nReturn));
					bReadCmd = FALSE;
					bScreenDisplay = TRUE;
				}
				else
				{
					// 알려지지 않은 Command Return시 다시 Read Command 날리도록 함
					bReadCmd = FALSE;
					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsRFIDCardScan() : else \n")));
				}

				// check input
				if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
				{
					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsRFIDCardScan() KEY PRESSED \n")));
					break;
				}
			}

			Delay_Msg(50);
		}

		// Get Event String
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();
		SplitString(m_OpInfo.strReturn, SCR_RES_DELIMITER, m_OpInfo.strTempArray);

		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL) || (m_OpInfo.strReturn == S_TIMEOVER))
		{
			while(bReadCmd == TRUE)
			{
				// Command가 끝날때까지 대기
				nReturn = m_pDiagRfid->CompleteAction(nDeviceCmd, RFID_IO_COMPLETED_CHECK_TIME);

				if (nReturn  != IO_PROCESSING)
				{
					NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsRFIDCardScan()  S_EXIT : nReturn[%d]\n"), nReturn));	
					bReadCmd = FALSE;
					break;
				}
				Delay_Msg(50);
			}

			NHDEBUG(DBG_INFO, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsRFIDCardScan()  S_EXIT : nReturn[%d]\n"), nReturn));	

			if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
				m_OpInfo.nProcNextScrNum = 827;
			}
			else
			{
				m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
				m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
			}
			break;
		}

		m_OpInfo.nMsgClearCount = SCR_OPMSGCYCLE;	// about 2 sec.
		// 데이터 업데이트
		ClerkScreenUpdate(m_OpInfo.nProcNextScrNum, TRUE);

	} // end of while

	NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnosticsRFIDCardScan() return \n")));

	return TRUE;
}
// end of [#2325]


/*-------------------------------------------------------------------
CLASS    NAME: CMainFrame
FUNCTION NAME: ClerkDiagnostics_TCPIPProc()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : SCR 872 PROC
-------------------------------------------------------------------*/
void	CMainFrame::ClerkDiagnostics_TCPIPProc()
{
	NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnostics_TCPIPProc() \n")));
	CString strTemp;

	while(TRUE)
	{
		NHDEBUG(DBG_INFO, (L"WAIT FOR USER INPUT\n"));

		///////////////////////////////////
		// WAIT FOR USER INPUT
		ClerkWaitScreenInput_byCha();
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();
		SplitString(m_OpInfo.strReturn, SCR_RES_DELIMITER, m_OpInfo.strTempArray);

		NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

		//-----------------------------------
		// PROCESS KEY STRING
		//-----------------------------------
		// F1
		if (m_OpInfo.strReturn == L"DIAG_HOSTADDRESS")
		{
			CString strVal;
			strVal.Format(L"%s", m_OpInfo.m_strnetworkDestination);
			if (ClerkInputAlpha(L"872F1T", strVal, 1, 40, strTemp) == TRUE)
			{
				strTemp.TrimLeft();
				strTemp.TrimRight();

				if (strTemp.GetLength() > 0)
				{
					m_OpInfo.m_strnetworkDestination = strTemp;
					ClerkNoticeToUser(T_OPERATE_OK);
				}
				else
				{
					ClerkNoticeToUser(T_INVALID_DATA);
				}
			}
		}
		// [#2358] US Kook 2015.07.14 support 'PING' function.
		// F2
		if (m_OpInfo.strReturn == L"DIAG_PING")
		{
			const int PING_MAX_COUNT	= 4;

#ifdef MEM_LEAK_CHECK
			DWORD	dwBeforePA=0, dwAfterPA=0;
			DWORD	dwBeforeVA=0, dwAfterVA=0;
			DWORD	dwBeforeTime=0, dwAfterTime=0;
			DWORD	dwInitPA=0, dwInitVA=0;

			if (dwInitPA == 0 || dwInitVA == 0)
			{
				MEMORYSTATUS memStatus;
				memStatus.dwLength = sizeof(MEMORYSTATUS);
				GlobalMemoryStatus(&memStatus);

				dwInitPA = memStatus.dwAvailPhys;
				dwInitVA = memStatus.dwAvailVirtual;
			}
#endif

			if (m_OpInfo.m_strnetworkDestination.GetLength() > 0)
			{
				CString strPingResultHeader = _T("");
				CString strPingResultContent = _T("");
				CString strPingResultTotal = _T("");
				int nPingCount = 0;
				int nPingSuccessCount = 0;
				int nPingAccmlRTT = 0;
				float fLossPercentage = 0;
				int nRtt = 0;
				int nAverageRTT = 0;

				// initial message
				strPingResultHeader.Format(L"Ping %s ...", m_OpInfo.m_strnetworkDestination);
				strPingResultContent.Format(L"%d %s, %d %s\n%.f%% %s, %dms %s",
					nPingCount, m_pDevCmn->fstrSCR_GetStringByTextID( _T("transmitted") ),
					nPingSuccessCount, m_pDevCmn->fstrSCR_GetStringByTextID( _T("received") ),
					fLossPercentage, m_pDevCmn->fstrSCR_GetStringByTextID( _T("loss") ),
					nAverageRTT, m_pDevCmn->fstrSCR_GetStringByTextID( _T("average") ));
				strPingResultTotal.Format(L"%s\n\n%s\n\n\n", strPingResultHeader, strPingResultContent);
				ClerkNoticeToUser(strPingResultTotal, FALSE);

				for ( ;; )
				{
#ifdef MEM_LEAK_CHECK
					MEMORYSTATUS memStatus;	
					memStatus.dwLength = sizeof(MEMORYSTATUS);
					GlobalMemoryStatus(&memStatus);

					dwBeforePA = memStatus.dwAvailPhys;
					dwBeforeVA = memStatus.dwAvailVirtual;
					dwBeforeTime = GetTickCount();
#endif
					// return : -1	- error in address 
					//			 0	- ping FALSE
					//			 1	- ping TRUE 
					if (m_pDevCmn->fnNET_PingTest(m_OpInfo.m_strnetworkDestination, &nRtt) == 1)
					{
						// PING function returns SUCCESS sometimes even it has failed.
						// so, determine it by positive valid RTT values. out-of-ranged RTT means failure.
						if (nRtt >= 0
							&& nRtt <= 20000)		// set valid RTT range : 0 ~ 20000 ms
						{
							nPingSuccessCount++;
							nPingAccmlRTT += nRtt;
						}
					}

					nPingCount++;
					
					fLossPercentage = (1 - ((float)nPingSuccessCount / nPingCount)) * 100;
					if (nPingSuccessCount > 0)
						nAverageRTT = nPingAccmlRTT / nPingSuccessCount;

					strPingResultContent.Format(L"%d %s, %d %s\n%.f%% %s, %dms %s",
						nPingCount, m_pDevCmn->fstrSCR_GetStringByTextID( _T("transmitted") ),
						nPingSuccessCount, m_pDevCmn->fstrSCR_GetStringByTextID( _T("received") ),
						fLossPercentage, m_pDevCmn->fstrSCR_GetStringByTextID( _T("loss") ),
						nAverageRTT, m_pDevCmn->fstrSCR_GetStringByTextID( _T("average") ));
					strPingResultTotal.Format(L"%s\n\n%s\n\n\n", strPingResultHeader, strPingResultContent);
					ClerkNoticeToUser(strPingResultTotal, FALSE); 

					if (nPingCount < PING_MAX_COUNT)
						Delay_Msg(500);
					else
						break;

#ifdef MEM_LEAK_CHECK
// 					MEMORYSTATUS memStatus;	
					memStatus.dwLength = sizeof(MEMORYSTATUS);
					GlobalMemoryStatus(&memStatus);

					dwAfterPA = memStatus.dwAvailPhys;
					dwAfterVA = memStatus.dwAvailVirtual;
					dwAfterTime = GetTickCount();

					HANDLE hFile = CreateFile (L"\\ATM\\MEM_HIS_PING.DAT",				// Open NVRAM.TXT.
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

						strTemp2.Format(L"[%02d/%02d %02d:%02d:%02d] COUNT(%4ld) TIME(%10d) | PA_A(%10ld) PA_D(%10ld) VA_A(%10ld) VA_D(%10ld)\n", 
							ti.wMonth, ti.wDay, ti.wHour, ti.wMinute, ti.wSecond,
							nPingCount, (dwAfterTime-dwBeforeTime), dwAfterPA, (dwInitPA-dwAfterPA), dwAfterVA, (dwInitVA-dwAfterVA));

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
						ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);
					}	
#endif
				}

				strPingResultHeader.Format(L"%s %s!", strPingResultHeader, m_pDevCmn->fstrSCR_GetStringByTextID( _T("completed") ));
				strPingResultTotal.Format(L"%s\n\n%s\n\n%s", strPingResultHeader, strPingResultContent,
					m_pDevCmn->fstrSCR_GetStringByTextID( _T("PRESS CANCEL TO EXIT") ));
				ClerkNoticeToUser(strPingResultTotal, FALSE);

				m_OpInfo.nProcLoofCnt = 0;
				m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
				while(TRUE)
				{
					m_OpInfo.nProcLoofCnt++;

					// exit if any key is entered
					if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
						break;

					Delay_Msg(200);

					// show 'DO YOU WANT TO EXIT?' screen after 5 minutes
					if (m_OpInfo.nProcLoofCnt > 1500)		// 200ms * 1500 = 300s
					{
						m_OpInfo.strReturn = S_TIMEOVER;
						break;
					}
				}
				m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
			}
			else
			{
				ClerkNoticeToUser(T_INVALID_DATA);
			}
		}
		// end of [#2358]
		// F4
		else if (m_OpInfo.strReturn == L"DIAG_CONNECT")
		{
			if (m_OpInfo.m_strnetworkDestination.GetLength() > 0)
			{
				ClerkNoticeToUser(m_pDevCmn->fstrSCR_GetStringFromTextID(T_CONNECTING));	// [#2160] CA KMK 2012.11.16 French OP

				// [#2016] NH KSK 2011.02.09
				BOOL bResult = FALSE;
				if (m_OpInfo.m_nSSLOption == 0)
				{
					// SSL Disable
					bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(TCPIP_HOST_LINE_TEST));
				}
				else
				{
					// SSL Enable
					// [#2310] NH KSK 2014.11.18
					//if (m_OpInfo.m_nSSLVersion == SSL_VER_23)
					//{
					//	if (m_OpInfo.m_nSSLCert == ENABLE) 
					//		bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(SSL_VER_23_USE_CERTIFY_LINE_TEST));
					//	else
					//		bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(TCPIP_HOST_SSL_VER23_LINE_TEST));
					//}
					//else
					//{
					//	if (m_OpInfo.m_nSSLCert == ENABLE) 
					//		bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(SSL_VER_3_USE_CERTIFY_LINE_TEST));
					//	else
					//		bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(TCPIP_HOST_SSL_VER3_LINE_TEST));
					//}

					// [#2320] US Justin 2015.01.14 Support NON-"Version Negotiation" Processor
					//if (m_OpInfo.m_nSSLVersion == SSL_VER3_AND_ABOVE)
					//{
					//	if (m_OpInfo.m_nSSLCert == ENABLE) 
					//		bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(SSL_VER3_AND_ABOVE_USE_CERTIFY_LINE_TEST));
					//	else
					//		bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(SSL_VER3_AND_ABOVE_LINE_TEST));
					//}
					//else if (m_OpInfo.m_nSSLVersion == TLS_VER10_AND_ABOVE)
					//{
					//	if (m_OpInfo.m_nSSLCert == ENABLE) 
					//		bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(TLS_VER10_AND_ABOVE_USE_CERTIFY_LINE_TEST));
					//	else
					//		bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(TLS_VER10_AND_ABOVE_LINE_TEST));
					//}
					//else if (m_OpInfo.m_nSSLVersion == TLS_VER11_AND_ABOVE)
					//{
					//	if (m_OpInfo.m_nSSLCert == ENABLE) 
					//		bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(TLS_VER11_AND_ABOVE_USE_CERTIFY_LINE_TEST));
					//	else
					//		bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(TLS_VER11_AND_ABOVE_LINE_TEST));
					//}
					//else if (m_OpInfo.m_nSSLVersion == TLS_VER12)
					//{
					//	if (m_OpInfo.m_nSSLCert == ENABLE) 
					//		bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(TLS_VER12_USE_CERTIFY_LINE_TEST));
					//	else
					//		bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(TLS_VER12_LINE_TEST));
					//}

					int nTestMode = TCPIP_HOST_LINE_TEST;
					if (m_OpInfo.m_nSSLVersion == CONN_UPTO_SSL_V30)
					{
						if (m_OpInfo.m_nSSLCert == ENABLE)		nTestMode = CONN_UPTO_SSL_V30_USE_CERT_LINE_TEST;
						else									nTestMode = CONN_UPTO_SSL_V30_LINE_TEST;
					}
					else if (m_OpInfo.m_nSSLVersion == CONN_UPTO_TLS_V10)
					{
						if (m_OpInfo.m_nSSLCert == ENABLE)		nTestMode = CONN_UPTO_TLS_V10_USE_CERT_LINE_TEST;
						else									nTestMode = CONN_UPTO_TLS_V10_LINE_TEST;
					}
					else if (m_OpInfo.m_nSSLVersion == CONN_UPTO_TLS_V11)
					{
						if (m_OpInfo.m_nSSLCert == ENABLE)		nTestMode = CONN_UPTO_TLS_V11_USE_CERT_LINE_TEST;
						else									nTestMode = CONN_UPTO_TLS_V11_LINE_TEST;
					}
					else if (m_OpInfo.m_nSSLVersion == CONN_UPTO_TLS_V12)
					{
						if (m_OpInfo.m_nSSLCert == ENABLE)		nTestMode = CONN_UPTO_TLS_V12_USE_CERT_LINE_TEST;
						else									nTestMode = CONN_UPTO_TLS_V12_LINE_TEST;
					}
					else if (m_OpInfo.m_nSSLVersion == CONN_UPTO_TLS_V13)
					{
						if (m_OpInfo.m_nSSLCert == ENABLE)		nTestMode = CONN_UPTO_TLS_V13_USE_CERT_LINE_TEST;
						else									nTestMode = CONN_UPTO_TLS_V13_LINE_TEST;
					}
					if (nTestMode != TCPIP_HOST_LINE_TEST)
						bResult = m_pDevCmn->fnNET_NetworkTest(m_OpInfo.m_strnetworkDestination, m_OpInfo.m_strPort, Int2Asc(nTestMode) );
					// End of [#2320]
					// end of [#2310]
				}

				if (bResult == TRUE)
					ClerkNoticeToUser(T_OPERATE_OK);
				else
					ClerkNoticeToUser(T_OPERATE_NG);
				// end of [#2016]
			}
			else
			{
				ClerkNoticeToUser(T_INVALID_DATA);
			}
		}
		// F3
		else if (m_OpInfo.strReturn == L"DIAG_HOSTPORT")
		{
			if (ClerkInputNumber(L"872F3T", L"", m_OpInfo.m_strPort, EDIT_NORMAL, 5, strTemp) == TRUE)
			{
				if (IsNum(strTemp) == TRUE)
				{
					if ((Asc2Int(strTemp) > 0) && (Asc2Int(strTemp) < 65536))
					{
						m_OpInfo.m_strPort = strTemp;
						ClerkNoticeToUser(T_OPERATE_OK);
					}
					else
						ClerkNoticeToUser(T_INVALID_DATA);
				}
				else
					ClerkNoticeToUser(T_INVALID_DATA);
			}
		}
		// F7
		else if (m_OpInfo.strReturn == L"SSLVERSION")
		{
			// [#2310] NH KSK 2014.11.18
//			if (m_OpInfo.m_nSSLVersion == SSL_VER_23)
//				m_OpInfo.m_nSSLVersion = SSL_VER_3;
//			else
//				m_OpInfo.m_nSSLVersion = SSL_VER_23;

			// [#2320] US Justin 2015.01.14 Support NON-"Version Negotiation" Processor
			//if (m_OpInfo.m_nSSLVersion == SSL_VER3_AND_ABOVE)
			//	m_OpInfo.m_nSSLVersion = TLS_VER10_AND_ABOVE;
			//else if (m_OpInfo.m_nSSLVersion == TLS_VER10_AND_ABOVE)
			//	m_OpInfo.m_nSSLVersion = TLS_VER11_AND_ABOVE;
			//else if (m_OpInfo.m_nSSLVersion == TLS_VER11_AND_ABOVE)
			//	m_OpInfo.m_nSSLVersion = TLS_VER12;
			//else
			//	m_OpInfo.m_nSSLVersion = SSL_VER3_AND_ABOVE;

			if (m_OpInfo.m_nSSLVersion == CONN_UPTO_TLS_V10)
				m_OpInfo.m_nSSLVersion = CONN_UPTO_TLS_V11;
			else if (m_OpInfo.m_nSSLVersion == CONN_UPTO_TLS_V11)
				m_OpInfo.m_nSSLVersion = CONN_UPTO_TLS_V12;
			else if (m_OpInfo.m_nSSLVersion == CONN_UPTO_TLS_V12)
				m_OpInfo.m_nSSLVersion = CONN_UPTO_TLS_V13;
			else
				m_OpInfo.m_nSSLVersion = CONN_UPTO_TLS_V13;
			// End of [#2320]
			// end of [#2310]
		}
		// [#2166] NH KSK 2012.12.17
		// F5
		else if(m_OpInfo.strReturn == L"DIAG_SSLOPTION")
		{
			if (m_OpInfo.m_nSSLOption == ENABLE)
				m_OpInfo.m_nSSLOption = 0;
			else
				m_OpInfo.m_nSSLOption = ENABLE;
		}
		// F8
		else if (m_OpInfo.strReturn == L"DIAG_SSLCERTIFY")
		{
			if (m_OpInfo.m_nSSLCert == ENABLE)
				m_OpInfo.m_nSSLCert = 0;
			else
				m_OpInfo.m_nSSLCert = ENABLE;
		}
		// end of [#2166]

		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;				// [#236] NZ AIREAT 2008.05.27 OP Screen Timeout
			m_OpInfo.nProcNextScrNum = 821;
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;			// [#236] NZ AIREAT 2008.05.27 OP Screen Timeout
			m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
			break;
		}

	} // end of while

	NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::ClerkDiagnostics_TCPIPProc() return \n")));

	return;
}

// [#2078] NH KSK 2011.07.15
/*-------------------------------------------------------------------
CLASS    NAME: CMainFrame
FUNCTION NAME: ClerkDiagAntiSkimmingTestProc()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : SCR 746 PROC ANTI SKIMMING TEST
-------------------------------------------------------------------*/
void CMainFrame::ClerkDiagAntiSkimmingTestProc()
{
	NHDEBUG(DBG_CALL, (L"START\n"));

	m_OpInfo.nAntiSkimmingTimeThreshold = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_TIMETHRESHOLD);

	// 수행 전 Initialize를 해야만 값을 얻어올 수 있다.
	m_pDiagMcu->Initialize();

	while(TRUE)
	{
		NHDEBUG(DBG_INFO, (L"WAIT FOR USER INPUT\n"));

		///////////////////////////////////
		// WAIT FOR USER INPUT
//		ClerkWaitScreenInput_byCha(TRUE);
		ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
		m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);

		while(TRUE)
		{
			///////////////////////////////////
			// DISPLAY SCREEN
			ClerkScreenUpdate(m_OpInfo.nProcNextScrNum);

			///////////////////////////////////
			// GET KEY STRING
			// 화면 입력을 기다린다.
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, SCR_INPUTWAIT, EVENT_IN) == DEV_SCR)
				break;

			m_OpInfo.nProcLoofCnt++;	
			Delay_Msg(2000);	// SP에서 2초로 Delay 주기를 원하여 2초로 사양 정함
		}

		// Stop to check timeout
		m_pDevCmn->fnSCR_SetDisplayData(L"APTimeoutMode", L"TimeoutStop");

		m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);

		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

		NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

		//-----------------------------------
		// PROCESS KEY STRING
		//-----------------------------------
		if (m_OpInfo.strReturn == L"INITIALIZE")
		{
			int nRet = MCU_FAIL;
			
			// Anti Skimming Initialize
			if(m_bDiagMCUOpen == TRUE)
			{
				ClerkNoticeToUser(T_PROCESS_WAIT);
				m_pDiagMcu->ClearDeviceError();
				nRet = m_pDiagMcu->Initialize();
			}

			if(nRet == MCU_SUCCESS)
			{
				NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagMcu->Initialize() : SUCCESS \n")));
				ClerkNoticeToUser(T_OPERATE_OK);
			}
			else /* Skimming ERROR */
			{
				ClerkNoticeToUser(T_DEVICE_ERR);
			}
		}
		else if (m_OpInfo.strReturn == L"TIMETHRESHOLD")
		{
			// INPUT TIME THRESHOLD
			CString strTimer;
			int		nTimer = 0;
			strTimer.Format(L"%d", m_OpInfo.nAntiSkimmingTimeThreshold);

			if (ClerkInputNumber(L"746F2", L"745F4S", strTimer, EDIT_NORMAL, 4, m_OpInfo.strReturn) == TRUE)
			{
				nTimer = Asc2Int(m_OpInfo.strReturn);
//				if (nTimer < 1 || nTimer > 9999)
				if (nTimer < 0 || nTimer > 9999)	// KSK 2011.07.30 0으로 설정 시 Metal Check 안하도록 하는 사양 추가
				{
					ClerkNoticeToUser(T_INVALID_DATA);
				}
				else
				{
					int nRet = MCU_FAIL;

					// Anti Skimming Initialize
					if( m_bDiagMCUOpen == TRUE)
					{
						ClerkNoticeToUser(T_PROCESS_WAIT);

						nRet = m_pDiagMcu->AntiSkimmingSetup(nTimer);
						if (nRet == MCU_SUCCESS)
						{
							m_OpInfo.nAntiSkimmingTimeThreshold = nTimer;
							NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::m_pDiagMcu->AntiSkimmingSetup() : SUCCESS \n")));
							ClerkNoticeToUser(T_OPERATE_OK);
						}
						else
						{
							ClerkNoticeToUser(T_DEVICE_ERR);
						}
					}
					else
					{
						ClerkNoticeToUser(T_DEVICE_ERR);
					}
				}
			}
		}

		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 821;
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
			break;
		}

	} // end of while

	NHDEBUG(DBG_CALL, (L"END\n"));
}
// end of [#2078]


// [#2078] NH KSK 2011.07.14
void CMainFrame::ClerkEnterDiagnostics()
{
	m_pDevCmn->m_JNLMgr.Save(CHANGE_DIAG_MODE);		// [#3] NH AIREAT 2008.3.10

	m_OpInfo.nTestCount			= 1;
	m_OpInfo.nProcessingCount	= 0;

	// Kill SP Processes
	ClerkNoticeToUser(T_PROCESS_WAIT);
	KillProcByGoma(L"NW_KSPR.EXE NHDIP4WCE.EXE NHIDC4MTKWCE.EXE NHDIP4SANDIPCE.EXE NHIDC4RFCE.EXE NHCDM.EXE NW_BCR_CE.EXE NW_CIM_CE.EXE NW_CIM_CE_JCM.EXE");

#ifdef DEV_SIMULATION
	KillProcByGoma(L"DEVCDM30.exe DEVIDC30.EXE DEVSIU30.EXE");
#endif

	NHDEBUG(1, (_T("DEINIT SPR %s ...\n"), _T(" ")));
	NVDump('O', 'A', "11", L"", L"DEINIT SPR" );		// [#2024] NH KSK 2011.02.24

	// [#97] US KGS 2008.04.08 VDM 진입시 Device 종료 안내 문구 표시
	// 순서 : SPR -> MCU - > CDU
	//ClerkNoticeToUser(T_TERMINATING_SPR);
	m_pDevCmn->fnSPR_Deinitialize();

	NHDEBUG(1, (_T("DEINIT MCU %s ...\n"), _T(" ")));
	NVDump('O', 'A', "11", L"", L"DEINIT MCU" );		// [#2024] NH KSK 2011.02.24

	//ClerkNoticeToUser(T_TERMINATING_MCU);
	m_pDevCmn->fnMCU_Deinitialize();

	// [#2325] NH KSK 2015.01.25 RFID added
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
	{
		NVDump('O', 'A', "11", L"", L"DEINIT RFID" );		// [#2024] NH KSK 2011.02.24
		//ClerkNoticeToUser(m_pDevCmn->fstrSCR_GetStringFromTextID(T_CLOSING_RFID));
		m_pDevCmn->fnRFID_Deinitialize();
	}
	// end of [#2325]

	// [#GLDV-3005] US Kook 2022.01.11 Side Car
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
	{
		NVDump('O', 'A', "11", L"", L"DEINIT SIDECAR" );
		m_pDevCmn->fnBCR_Deinitialize();
		m_pDevCmn->fnBNA_Deinitialize();
	}
	// end of [#GLDV-3005]

	NHDEBUG(1, (_T("DEINIT CDU %s ...\n"), _T(" ")));
	NVDump('O', 'A', "11", L"", L"DEINIT CDU" );		// [#2024] NH KSK 2011.02.24

	//ClerkNoticeToUser(T_TERMINATING_CDU);
	m_pDevCmn->fnCDU_Deinitialize();

	// [#GLDV-3005] US Kook 2022.01.11 Side Car
	/*
	// [#2325] NH KSK 2015.01.25 RFID added
	//	m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MAIN);
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MAIN|DEV_RFID);
	else
		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MAIN);
	// end of [#2325]
	*/
	m_pDevCmn->fnAPL_CheckDeviceAction(m_pDevCmn->fnCMN_GetActiveDevice());
	// end of [#GLDV-3005]

	int nErrorCode = 0;

	if( m_pDiagSpr == NULL )
		m_pDiagSpr			= (CDiagSpr*)  new CDiagSpr;					
	if( m_pDiagMcu == NULL )
		m_pDiagMcu			= (CDiagCard*) new CDiagCard(m_pDevCmn->GetDeviceType(L"MCU"));
	if( m_pDiagCdu == NULL )
		m_pDiagCdu			= (CDiagCdu*)  new CDiagCdu;

	// [#2325] NH KSK 2015.01.25 RFID Added
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
	{
		if (m_pDiagRfid == NULL)
			m_pDiagRfid			= (CDiagRfid*) new CDiagRfid(1);	// 현재 VivoPay만 지원하여 Fix함
	}
	// end of [#2325]

	// [#GLDV-3005] US Kook 2021.10.22 Support Side Car
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
	{
		NHDEBUG(1, (_T("Checking m_pDiagBcr %s ...\n"), _T(" ")));
		if (m_pDiagBcr == NULL)
		{
			NHDEBUG(1, (_T("Creating m_pDiagBcr %s ...\n"), _T(" ")));
			m_pDiagBcr = new CDiagBcr();
		}

		NHDEBUG(1, (_T("Checking m_pDiagBna %s ...\n"), _T(" ")));
		if (m_pDiagBna == NULL)
		{
			NHDEBUG(1, (_T("Creating m_pDiagBna %s ...\n"), _T(" ")));
			m_pDiagBna = new CDiagBna(m_pDevCmn->GetDeviceType(L"BNA"));
		}
	}
	// end of [#GLDV-3005]

	// Open devices using multi-thread
	ClerkDiagnosticsDeviceHandler(ON);



}
// end of [#2078]



