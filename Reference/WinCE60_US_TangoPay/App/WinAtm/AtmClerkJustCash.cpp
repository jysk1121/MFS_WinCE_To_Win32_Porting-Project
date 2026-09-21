#include "stdafx.h"
#include "MainFrm.h"
#include "AtmClerkCtrl.h"
#include ".\Tran\CJustCashData.h"
#include ".\Common\NHDbgApi.h"

#if (APP_JUST_CASH)

CString CMainFrame::ClerkJustCashScreenDataMake(int screenNumber)
{
	switch (screenNumber)
	{
	case 761: return ClerkJustCashGetGeneralOptions();
	}

	return CString(L"");
}

CString CMainFrame::ClerkJustCashGetGeneralOptions()
{
	CString data(L"");

	data.Format(
		L"%s%s%s%s",
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_JUSTCASH_ENABLE_SVC_OPTION(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE))), SCR_CMD_DELIMITER,
		m_pTranCmn->m_JustCashData.GetTransactionMode(), SCR_CMD_DELIMITER);

	if (m_pTranCmn->m_JustCashData.m_bJustCashRegistered == TRUE)
		data += L"REGISTERED";
	else
		data += L"NOT REGISTERED";

	return data;
}

CString CMainFrame::ClerkJustCashEnableButtons(int screenNumber)
{
	switch (screenNumber)
	{
	case 761: return ClerkJustCashGetGeneralOptionButtons();
	}

	return CString(L"");
}

CString CMainFrame::ClerkJustCashGetGeneralOptionButtons()
{
	CString data;
	data.Append(L"on \ron \ron "); // F1-En/Disable, F2-Registering Service, F3-Transaction Mode

	return data;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkJustCashGeneralOptions()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 761 PROC - General options for JustCash
-------------------------------------------------------------------*/
void CMainFrame::ClerkJustCashGeneralOptions()
{
	while (TRUE)
	{
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
		if (m_OpInfo.strReturn == L"ENABLEJUSTCASH")
		{
			int nNewValue = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE) + 1;

			if (nNewValue > JUSTCASH_SERVICE_ALL)
				nNewValue = 0;

			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE, nNewValue);
			break;
		}
		// F2
		else if (m_OpInfo.strReturn == L"REGISTERJUSTCASH")
		{
			ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);

			if (m_pTranCmn->P_NH_JUSTCASH_RegisterService() == RES_OK)
				m_OpInfo.m_strMsg = T_OPERATE_OK;
			else
				m_OpInfo.m_strMsg = T_OPERATE_NG;

			ClerkNoticeToUser(m_OpInfo.m_strMsg);
			break;
		}
		// F3
		else if (m_OpInfo.strReturn == L"MODE")
		{
			ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);

			int mode = m_pTranCmn->m_JustCashData.m_transactionMode;
			mode++;

			bool justCashAvailable = m_pTranCmn->m_JustCashData.m_bJustCashAvailable && m_pTranCmn->m_JustCashData.m_bJustCashRegistered;
			bool allowSidecarMode = justCashAvailable && ClerkJustCashIsSidecarModeAvailable();

			if (mode == JCTXNMODE_ATM && !justCashAvailable)
			{
				mode++;
			}

			if (mode == JCTXNMODE_SIDECAR && !allowSidecarMode)
			{
				mode++;
			}

			if (mode == JCTXNMODE_BOTH && !allowSidecarMode)
			{
				mode++;
			}

			if (mode >= JCTXNMODE_MAX || mode < JCTXNMODE_ATM)
			{
				mode = JCTXNMODE_ATM;
			}

			m_pTranCmn->m_JustCashData.m_transactionMode = mode;

			if (m_pTranCmn->m_JustCashData.SaveSetting())
			{
				// Enablement changed
				NHDBG((L"Changed Just.Cash Mode: %d\r\n", m_pTranCmn->m_JustCashData.m_transactionMode));

				m_OpInfo.m_strMsg = T_OPERATE_OK;
			}
			else
			{
				m_OpInfo.m_strMsg = T_OPERATE_NG;
			}

			ClerkNoticeToUser(m_OpInfo.m_strMsg);
			break;
		}

		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 795; // Back to Other Transactions
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = OP_TIMEOUT_SCREEN_NUMBER;
			break;
		}
	}
}

bool CMainFrame::ClerkJustCashIsSidecarModeAvailable()
{
#ifdef UNDER_CE
	bool bAvailable = false;
#else
	bool bAvailable = true;
#endif

	// Ensure sidecar support is enabled
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
	{
		bAvailable = true;

		// Update device status
		m_pDevCmn->fnAPL_CheckDevice();

		// Check BNA
		bAvailable &= m_pDevCmn->fnBNA_GetAggregateDeviceStatus() == NORMAL;

		// JCTODO: Enable BCR check once BCR issues are resolved
		// Check BCR
		// bAvailable &= m_pDevCmn->fnBCR_GetDeviceStatus() == NORMAL;
	}

	return bAvailable;
}

#endif