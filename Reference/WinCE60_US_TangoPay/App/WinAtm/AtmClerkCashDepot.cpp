#include "stdafx.h"
#include "MainFrm.h"
#include "AtmClerkCtrl.h"
#include ".\Common\NHDbgApi.h"

#if (APP_CUSTOM_CASHDEPOT)

CString CMainFrame::ClerkCashDepotScreenDataMake(int screenNumber)
{
	switch (screenNumber)
	{
	case 1030: return ClerkCashDepotGetGeneralOptions();
	}

	return CString(L"");
}

CString CMainFrame::ClerkCashDepotGetGeneralOptions()
{
	CString data(L"");

	data.Format(L"%s%s%s",
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID(MemGetInt(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_ISHOSTUSEURL))), SCR_CMD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_HOSTNAME));

	return data;
}

CString CMainFrame::ClerkCashDepotEnableButtons(int screenNumber)
{
	switch (screenNumber)
	{
	case 1030: return ClerkCashDepotGetGeneralOptionButtons();
	}

	return CString(L"");
}

CString CMainFrame::ClerkCashDepotGetGeneralOptionButtons()
{
	CString data;
	data.Append(L"on \ron "); // F1-URL EN/DISABLE, F2-ADDRESS

	return data;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkCashDepotGeneralOptions()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 1030 PROC - General options for CashDepot
-------------------------------------------------------------------*/
void CMainFrame::ClerkCashDepotGeneralOptions()
{
	while (TRUE)
	{
		///////////////////////////////////
		// WAIT FOR USER INPUT
		ClerkWaitScreenInput_byCha();
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

		NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

		//-----------------------------------
		// PROCESS KEY STRING
		//-----------------------------------
		// F1
		if (m_OpInfo.strReturn == L"HOSTURLSETUP")
		{
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_ISHOSTUSEURL) == ENABLE)
				MemSetInt(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_ISHOSTUSEURL, 0);
			else
				MemSetInt(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_ISHOSTUSEURL, ENABLE);
		}
		// F2
		else if (m_OpInfo.strReturn == L"HOSTADDRESS")
		{
			BOOL bRes;
			CString strValue;
			strValue = MemGetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_HOSTNAME);

			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_ISHOSTUSEURL) == 1)
				bRes = ClerkInputAlpha(L"1030F2T", strValue, 1, 40, m_OpInfo.strReturn);
			else
				bRes = ClerkInputNumber(L"1030F2T", L"", strValue, EDIT_NORMAL, 15, m_OpInfo.strReturn);

			if (bRes == TRUE)
			{
				CString strTemp;

				strTemp = m_OpInfo.strReturn;
				strTemp.TrimLeft();
				strTemp.TrimRight();

				if (strTemp.GetLength() <= 0)
				{
					MemSetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_HOSTNAME, L"");
					m_OpInfo.m_strMsg = T_OPERATE_OK;
				}
				else
				{
					// URL이 아닌 경우에만 Check한다.
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_ISHOSTUSEURL) == 0)
					{
						if (CheckValidate_IPValue(strTemp) == TRUE)
						{
							MemSetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_HOSTNAME, m_pDevCmn->Convert_IpAddressInfo(strTemp));
							m_OpInfo.m_strMsg = T_OPERATE_OK;
						}
						else
						{
							m_OpInfo.m_strMsg = T_INVALID_DATA;
						}
					}
					else
					{
						MemSetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_HOSTNAME, strTemp);
						m_OpInfo.m_strMsg = T_OPERATE_OK;
					}
				}

				m_pTranCmn->UpdateCashDepotServices(MemGetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_HOSTNAME));
				ClerkNoticeToUser(m_OpInfo.m_strMsg);
			}
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

#endif