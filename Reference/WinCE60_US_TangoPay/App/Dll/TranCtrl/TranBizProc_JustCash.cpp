#include "stdafx.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"

#include ".\Common\NHDbgApi.h"

#if (APP_JUST_CASH)

extern CTimeCheck g_TimeCheck;

//
// Private Functions
//
BOOL CTranCmn::H_NH_JC_IsJustCashAvailable()
{
	return m_JustCashData.m_bJustCashRegistered == TRUE &&
		MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) != NETWORK_DIALUP &&
		m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL;
}

BOOL CTranCmn::H_NH_JC_IsJustCashCardless()
{
	return H_NH_JC_IsJustCashAvailable() &&
		MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE) & JUSTCASH_SERVICE_CARDLESS;
}

BOOL CTranCmn::H_NH_JC_IsJustCashBitcoin()
{
	return H_NH_JC_IsJustCashAvailable() &&
		MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE) & JUSTCASH_SERVICE_DIGITAL_CURRENCY &&
		(H_NH_JC_IsAtmModeAvailable() || H_NH_JC_IsSidecarModeAvailable());
}

BOOL CTranCmn::H_NH_JC_IsAtmModeAvailable()
{
	return m_JustCashData.IsAtmConfigured();
}

BOOL CTranCmn::H_NH_JC_IsSidecarModeAvailable()
{
#ifdef UNDER_CE
	BOOL bAvailable = false;
#else
	BOOL bAvailable = m_JustCashData.IsSidecarConfigured();
#endif

	// Ensure JC is configured for side car and side is enabled
	if ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE) && m_JustCashData.IsSidecarConfigured())
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

bool CTranCmn::H_NH_JC_StartCashIn()
{
	bool bRes = false;

	m_pDevCmn->fnBNA_StartCashIn();

	// Check the result of cash in
	if (m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BNA))
	{
		// Start accepting cash
		bRes = H_NH_JC_StartAcceptCash();
	}
	else
	{
		NHDEBUG(DBG_CALL, (L"Failed to start cash in \n"));
		int deviceStatus = m_pDevCmn->fnBNA_GetDeviceStatus();
		NHDEBUG(DBG_CALL, (L"H_NH_JC_StartAcceptCash - BNA Status: [%d]\n", deviceStatus));
		CString errorCode = m_pDevCmn->fstrBNA_GetErrorCode();
		NHDEBUG(DBG_CALL, (L"H_NH_JC_StartAcceptCash - BNA error code: [%s]\n", errorCode));
	}

	return bRes;
}

bool CTranCmn::H_NH_JC_StartAcceptCash()
{
	bool bRes = false;

	int result = m_pDevCmn->fnBNA_AcceptCash(KEYIN_TIME_OUT * 1000, KEYIN_TIME_OUT * 1000);

	if (result == 0)
	{
		NHDEBUG(DBG_CALL, (L"Started AcceptCash Cash In \n"));
		bRes = true;
	}
	else
	{
		NHDEBUG(DBG_CALL, (L"Failed AcceptCash Cash. Result: [%d] \n", result));
		m_pDevCmn->fnBNA_CancelAccept();
		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BNA);
	}

	return bRes;
}

bool CTranCmn::H_NH_JC_StoreCash()
{
	bool bRes = false;

	m_pDevCmn->fnBNA_StoreCash();
	if (m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BNA))
	{
		bRes = true;
		// JCTODO: Log & Journal
	}
	else
	{
		// JCTODO: Log & Journal
	}

	return bRes;
}

bool CTranCmn::H_NH_JC_ReturnCash()
{
	bool bRes = false;

	m_pDevCmn->fnBNA_RollbackCash(K_5_WAIT * 1000);
	if (m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BNA))
	{
		bRes = true;
		// JCTODO: Log & Journal
	}
	else
	{
		// JCTODO: Log & Journal
	}

	return bRes;
}

//
// Public Functions
//

BIZ_RETURN CTranCmn::P_NH_JC_ChooseFlow(JCFlowSelection *selection)
{
	*selection = JCF_DISABLED;

	CString GetKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(500);
		m_pDevCmn->fnSCR_DisplayScreen(500, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "-1", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(500, L"500.wav");

			m_pAdaCtrl->fnExp_PlayScreenWave(500);

			replay = FALSE;
		}

		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (IsAdaTransaction() && GetKeyStr.GetLength() > 6)
			{
				GetKeyStr = GetKeyStr.Mid(6);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));

				if (GetKeyStr == L"." || GetKeyStr == L"00" || GetKeyStr == L"000") // Repeat or Volume Change
				{
					m_pAdaCtrl->fnExp_StopPlay();

					// Handle volume change repeats
					if (GetKeyStr == L".")
						m_pAdaCtrl->fnExp_SetVolumeDown();
					else if (GetKeyStr == L"00")
						m_pAdaCtrl->fnExp_SetVolumeUp();

					// Repeat after a volume change
					replay = TRUE;

					continue;
				}
				else if (GetKeyStr == L"1")
				{
					GetKeyStr = L"BITCOIN";
				}
				else if (GetKeyStr == L"3")
				{
					GetKeyStr = L"CARDLESS";
				}
			}

			if (GetKeyStr == L"BITCOIN")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"500_BitcoinSelected.wav", TRUE);
				}

				*selection = JCF_BITCOIN;

				return RES_OK;
			}
			else if (GetKeyStr == L"CARDLESS")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"500_CardlessSelected.wav", TRUE);
				}

				*selection = JCF_CARDLESS;

				return RES_OK;
			}
			else if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "-1", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "-1", L"NG_1", GetKeyStr);

				return RES_NG;
			}
			else if (IsAdaTransaction())
			{
				replay = TRUE;

				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));
	NVDump('F', 'C', "-1", L"NG_2", L"CANCEL");

	return RES_NG;
}

BIZ_RETURN CTranCmn::P_NH_JC_ChoosePayment(JCPaymentSelection *selection)
{
	*selection = JCP_NONE;

	CString GetKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (!IsAdaTransaction())
	{
		CString selectedCoin;
		selectedCoin.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_163101));

		m_pDevCmn->fnSCR_DisplayPrevSet(501);
		m_pDevCmn->fnSCR_DisplayFormat(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_501000), selectedCoin);
		m_pDevCmn->fnSCR_DisplayScreen(501, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "-1", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(501, L"501.wav");

			m_pAdaCtrl->fnExp_PlayScreenWave(501);

			replay = FALSE;
		}

		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (IsAdaTransaction() && GetKeyStr.GetLength() > 6)
			{
				GetKeyStr = GetKeyStr.Mid(6);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));

				if (GetKeyStr == L"." || GetKeyStr == L"00" || GetKeyStr == L"000") // Repeat or Volume Change
				{
					m_pAdaCtrl->fnExp_StopPlay();

					// Handle volume change repeats
					if (GetKeyStr == L".")
						m_pAdaCtrl->fnExp_SetVolumeDown();
					else if (GetKeyStr == L"00")
						m_pAdaCtrl->fnExp_SetVolumeUp();

					// Repeat after a volume change
					replay = TRUE;

					continue;
				}
				else if (GetKeyStr == L"1")
				{
					GetKeyStr = L"DEBIT";
				}
				else if (GetKeyStr == L"3")
				{
					GetKeyStr = L"CASH";
				}
			}

			if (GetKeyStr == L"DEBIT")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"501_DebitSelected.wav", TRUE);
				}

				*selection = JCP_DEBIT;

				return RES_OK;
			}
			else if (GetKeyStr == L"CASH")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"501_CashSelected.wav", TRUE);
				}

				*selection = JCP_CASH;

				return RES_OK;
			}
			else if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "-1", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "-1", L"NG_1", GetKeyStr);

				return RES_NG;
			}
			else if (IsAdaTransaction())
			{
				replay = TRUE;

				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));
	NVDump('F', 'C', "-1", L"NG_2", L"CANCEL");

	return RES_NG;
}

BIZ_RETURN CTranCmn::P_NH_JC_EnterCustomerPhoneNumber(CString &phoneNumber)
{
	NHDBG((L"[CTranCmn::P_NH_JC_EnterCustomerPhoneNumber]\n"));
	NVDump('O', 'C', "-1", L"Start", L"Phone");

	CString GetKeyStr, AdaKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(505);
		m_pDevCmn->fnSCR_DisplayScreen(505, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "-1", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(505, L"505.wav");

			if (!AdaKeyStr.IsEmpty())
			{
				CString strTemp;
				strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"505_Entry.wav"), AdaKeyStr);
				m_pAdaCtrl->fnExp_AddWaveFile(505, strTemp);
			}

			m_pAdaCtrl->fnExp_PlayScreenWave(505);

			replay = FALSE;
		}

		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (IsAdaTransaction() && GetKeyStr.GetLength() > 6)
			{
				GetKeyStr = GetKeyStr.Mid(6);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));

				if (GetKeyStr == L"." || GetKeyStr == L"00" || GetKeyStr == L"000") // Repeat or Volume Change
				{
					m_pAdaCtrl->fnExp_StopPlay();

					// Handle volume change repeats
					if (GetKeyStr == L".")
						m_pAdaCtrl->fnExp_SetVolumeDown();
					else if (GetKeyStr == L"00")
						m_pAdaCtrl->fnExp_SetVolumeUp();

					// Repeat after a volume change
					replay = TRUE;

					continue;
				}
			}

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "-1", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "-1", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_ENTER)
			{
				phoneNumber = AdaKeyStr;
				NVDump('O', 'C', "-1", L"Phone", phoneNumber);

				return RES_OK;
			}
			else if (GetKeyStr == S_CLEAR)
			{
				AdaKeyStr = L"";
			}
			else
			{
				if (IsAdaTransaction())
				{
					int numChk = Asc2Int(GetKeyStr);

					if (numChk >= 0 && numChk <= 9)
					{
						AdaKeyStr += GetKeyStr;

						CString strTemp;
						strTemp.Format(L"%s.wav", GetKeyStr);
						m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
					}
					else
					{
						replay = TRUE;

						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					}
				}
				else
				{
					phoneNumber = GetKeyStr;
					NVDump('O', 'C', "-1", L"Phone", phoneNumber);

					return RES_OK;
				}
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_JC_EnterSmsCode(CString &code)
{
	NHDBG((L"[CTranCmn::P_NH_JC_EnterSmsCode]\n"));
	NVDump('O', 'C', "-1", L"Start", L"SmsCode");

	CString GetKeyStr, AdaKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(506);
		m_pDevCmn->fnSCR_DisplayScreen(506, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "-1", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(506, L"506.wav");

			if (!AdaKeyStr.IsEmpty())
			{
				CString strTemp;
				strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"506_Entry.wav"), AdaKeyStr);
				m_pAdaCtrl->fnExp_AddWaveFile(506, strTemp);
			}

			m_pAdaCtrl->fnExp_PlayScreenWave(506);

			replay = FALSE;
		}

		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (IsAdaTransaction() && GetKeyStr.GetLength() > 6)
			{
				GetKeyStr = GetKeyStr.Mid(6);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));

				if (GetKeyStr == L"." || GetKeyStr == L"00" || GetKeyStr == L"000") // Repeat or Volume Change
				{
					m_pAdaCtrl->fnExp_StopPlay();

					// Handle volume change repeats
					if (GetKeyStr == L".")
						m_pAdaCtrl->fnExp_SetVolumeDown();
					else if (GetKeyStr == L"00")
						m_pAdaCtrl->fnExp_SetVolumeUp();

					// Repeat after a volume change
					replay = TRUE;

					continue;
				}
			}

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "-1", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "-1", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_ENTER)
			{
				code = AdaKeyStr;
				NVDump('O', 'C', "-1", L"code", code);

				return RES_OK;
			}
			else if (GetKeyStr == S_CLEAR)
			{
				AdaKeyStr = L"";
			}
			else
			{
				if (IsAdaTransaction())
				{
					int numChk = Asc2Int(GetKeyStr);

					if (numChk >= 0 && numChk <= 9)
					{
						AdaKeyStr += GetKeyStr;

						CString strTemp;
						strTemp.Format(L"%s.wav", GetKeyStr);
						m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
					}
					else
					{
						replay = TRUE;

						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					}
				}
				else
				{
					code = GetKeyStr;
					NVDump('O', 'C', "-1", L"code", code);

					return RES_OK;
				}
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_JC_SelectCoin()
{
	NVDump('O', 'C', "00", L"JUSTCSH", L"Dgt_Cur_Sel");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_JUSTCASH_DigitalCur_SelectType()]\n"));

	CString	GetKeyStr, strTemp;
	BOOL	bShowScreen = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(163); // USE General Selection

			// Title => APValue2
			strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_163001));
			m_pDevCmn->fnSCR_DisplayString(2, strTemp);

			// Sub Title => APValue3
			strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_163002));
			m_pDevCmn->fnSCR_DisplayString(3, strTemp);

			// F2 => BitCoin
			strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_163101));
			m_pDevCmn->fnSCR_DisplayString(12, strTemp);

			// F4 => Ethereum
			// Disable Ethereum 2017.10.03 - The printed Big QR Image can't be recognized by a phone
			// strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_163102));
			// m_pDevCmn->fnSCR_DisplayString(14, strTemp);

			// F8 EXIT
			m_pDevCmn->fnSCR_DisplayString(18, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_EXIT)); // [#2539] US Justin 2018.03.26

			m_pDevCmn->fnSCR_DisplayScreen(163, KEYIN_TIME_OUT, PIN_MENU_MODE);

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
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT || GetKeyStr == L"F8") // [#2539] US Justin 2018.03.26
			{
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_14");
				break;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_15");
				break;
			}
			else if (GetKeyStr == L"F2") // F2, BitCoin
			{
				m_JustCashData.m_strCurrencyType = JUSTCASH_CURRENCY_BITCOIN;

				return RES_OK;
			}
			else if (GetKeyStr == L"F4") // F4, Ethereum
			{
				m_JustCashData.m_strCurrencyType = JUSTCASH_CURRENCY_ETHEREUM;

				return RES_OK;
			}
		}
#endif

		Delay_Msg(50);
	}

	return RES_NG;
}

BIZ_RETURN CTranCmn::P_NH_JC_Purchase()
{
	NVDump('O', 'C', "00", L"JUSTCSH", L"Dgt_Cur_Prchs");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_JUSTCASH_DigitalCur_Purchase()]\n"));

	// DISPLAY SCREEN
	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	// m_pDevCmn->fnSCR_DisplayImage(1, FALSE);													// Hide Exit Button // [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
	m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161001));	// Please Wait
	m_pDevCmn->fnSCR_DisplayString(2, L"");														// Hide Cancel F8
	m_pDevCmn->fnSCR_DisplayString(3, L"");														// NO Background Image
	m_pDevCmn->fnSCR_DisplayScreen(161);

	m_JustCashData.m_strTranAmount.Format(L"%d", Asc2Int(m_sUserSelection.strMoney));
	m_JustCashData.m_nTransactionStep = JUSTCASH_DIGITAL_CUR_PURCHASE;

	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if (nRes == RES_OK)
	{
		NHDEBUG(DBG_CALL, (L"Dgt_Cur_PURCHASE OK\n"));
	}
	else
	{
		NHDEBUG(DBG_CALL, (L"Dgt_Cur_PURCHASE FAILURE\n"));
		LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);
	}

	return nRes;
}

BIZ_RETURN CTranCmn::P_NH_JC_ApproveTransaction()
{
	NVDump('O', 'C', "00", L"JUSTCSH", L"Dgt_Cur_UserApprv");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_JUSTCASH_DigitalCur_ApproveTransaction()]\n"));

	CString	GetKeyStr, strTemp;
	BOOL	bShowScreen = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	BOOL bIsTranAccepted = FALSE;

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(135);		// USE DCC OFFERING SCREEN

			m_pDevCmn->fnSCR_DisplayImage(1, TRUE);		// Small Black Box
			m_pDevCmn->fnSCR_DisplayImage(2, FALSE);	// Large Black Box
			m_pDevCmn->fnSCR_DisplayImage(3, TRUE);		// Icon
			m_pDevCmn->fnSCR_DisplayImage(4, TRUE);		// Line

			// Title => APValue2 (Bitcoin Purchase Summary)
			if (m_JustCashData.m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_CODE)
			{
				strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135111));
			}
			else
			{
				strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135101));
			}

			// [#2503] US Justin 2017.08.06 Add Ethereum
			if (m_JustCashData.m_strCurrencyType == JUSTCASH_CURRENCY_ETHEREUM)
				strTemp.Replace(L"Bitcoin", L"Ethereum");
			// End of [#2503]

			m_pDevCmn->fnSCR_DisplayString(2, strTemp);

			// [#2511] US Justin 2017.10.17 Change Bitcoin screen text
			/*
			// Price => APValue3, APValue4
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135102));
			strTemp.Format(L"$ %s", m_JustCashData.m_strDCPrice);
			m_pDevCmn->fnSCR_DisplayString(4, strTemp);

			// BTC Quantity => APValue7, APValue8
			m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135103));
			strTemp.Format(L"%s", m_JustCashData.m_strDCQuantity);
			m_pDevCmn->fnSCR_DisplayString(8, strTemp);

			// BTC Whole Unit Prince => APValue11, APValue12
			m_pDevCmn->fnSCR_DisplayString(11, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135104));
			strTemp.Format(L"$ %s", m_JustCashData.m_strDCUnitPrice);
			m_pDevCmn->fnSCR_DisplayString(12, strTemp);
			*/

			// Price per Bitcoin => APValue3, APValue4
			strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135102));

			if (m_JustCashData.m_strCurrencyType == JUSTCASH_CURRENCY_ETHEREUM)
				strTemp.Replace(L"Bitcoin", L"Ethereum");

			m_pDevCmn->fnSCR_DisplayString(3, strTemp);
			strTemp.Format(L"$ %s", m_JustCashData.m_strDCUnitPrice);
			m_pDevCmn->fnSCR_DisplayString(4, strTemp);

			if (m_JustCashData.m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_CODE)
			{
				// Minimum Amount => APValue7, APValue8
				m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135112));
				strTemp.Format(L"$ %s", m_JustCashData.m_strMinAmount);
				m_pDevCmn->fnSCR_DisplayString(8, strTemp);

				// Maximum Amount => APValue11, APValue12
				m_pDevCmn->fnSCR_DisplayString(11, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135113));
				strTemp.Format(L"$ %s", m_JustCashData.m_strMaxAmount);
				m_pDevCmn->fnSCR_DisplayString(12, strTemp);
			}
			else
			{
				// Purchase Amount => APValue7, APValue8
				m_pDevCmn->fnSCR_DisplayString(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135103));
				strTemp.Format(L"$ %s", m_JustCashData.m_strDCPrice);
				m_pDevCmn->fnSCR_DisplayString(8, strTemp);

				// Purchased Quantity => APValue11, APValue12
				m_pDevCmn->fnSCR_DisplayString(11, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135104));
				strTemp.Format(L"%s", m_JustCashData.m_strDCQuantity);
				m_pDevCmn->fnSCR_DisplayString(12, strTemp);
			}
			// End of [#2511]

			if (m_JustCashData.m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_ADD)
			{
				// F7 Button => Decline
				m_pDevCmn->fnSCR_DisplayString(17, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135115));

				// F8 Button => Confirm
				m_pDevCmn->fnSCR_DisplayString(18, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135114));
			}
			else
			{
				// Please make selection => APValue13
				m_pDevCmn->fnSCR_DisplayString(13, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135105));

				// F7 Button => Decline
				m_pDevCmn->fnSCR_DisplayString(17, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135106));

				// F8 Button => Approve
				m_pDevCmn->fnSCR_DisplayString(18, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135107));
			}

			m_pDevCmn->fnSCR_DisplayScreen(135, KEYIN_TIME_OUT, PIN_MENU_MODE);

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
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_14");
				break;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_15");
				break;
			}
			else if (GetKeyStr == L"LOCALCURRENCY")	// F7, Decline
			{
				break;
			}
			else if (GetKeyStr == L"HOMECURRENCY")	// F8, Approve
			{
				bIsTranAccepted = TRUE;
				break;
			}
		}
#endif

		Delay_Msg(50);
	}

	if (bIsTranAccepted)
		return RES_OK;

	// LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);
	// P_NH_JUSTCASH_DigitalCur_Commit(0, FALSE);

	return RES_NG;
}

BIZ_RETURN CTranCmn::P_NH_JC_Processor_Auth()
{
	NVDump('O', 'C', "00", L"JUSTCSH", L"Dgt_Cur_HostAuth");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_JUSTCASH_DigitalCur_Processor_Auth()]\n"));

	// DISPLAY SCREEN
	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	// m_pDevCmn->fnSCR_DisplayImage(1, FALSE);													// Hide Exit Button // [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
	m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161001));	// Please Wait
	m_pDevCmn->fnSCR_DisplayString(2, L"");														// Hide Cancel F8
	m_pDevCmn->fnSCR_DisplayString(3, L"");														// NO Background Image
	m_pDevCmn->fnSCR_DisplayScreen(161);

	m_JustCashData.m_nTransactionStep = JUSTCASH_DIGITAL_CUR_HOSTAPPROVAL;

	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if (nRes == RES_OK)
		NHDEBUG(DBG_CALL, (L"NonCash HOST AUTH OK\n"));
	else
		NHDEBUG(DBG_CALL, (L"NonCash Host AUTH FAILURE\n"));

	return nRes;
}

BIZ_RETURN CTranCmn::P_NH_JC_PrintQRCode(bool debit)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_JUSTCASH_DigitalCur_PrintQRCode]\n"));

	CString strTemp, strTemp2, strPrintData;
	strPrintData = L"";
	int i;

	m_pDevCmn->fnSCR_DisplayPrevSet(119);

	if (debit || m_JustCashData.m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_CODE)
	{
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119002)); // [#RWC6-16] JustCash screen change and receipt change
	}
	else if (!debit && m_JustCashData.m_nTransactionStep != JUSTCASH_DIGITAL_CUR_CASH_CODE)
	{
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119003)); // [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash
	}

	// Display Arrow for HALO... (NO SPR Flicker)
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
		m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
	else
		m_pDevCmn->fnSCR_DisplayImage(1, FALSE);

	m_pDevCmn->fnSCR_DisplayScreen(119);

	// Space
	AddPrintData(strPrintData, L" ");

	// ADDRESS 1,2,3,PHONE NUMBER PRINT
	for (i = 0; i < 4; i++)
	{
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1 + i);
		strTemp.TrimLeft();

		if (strTemp.GetLength() > 0)
			AddPrintData(strPrintData, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1 + i));
	}

	// HEADER 1,2 PRINT
	for (i = 0; i < 2; i++)
	{
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER1 + i);
		strTemp.TrimLeft();

		if (strTemp.GetLength() > 0)
			AddPrintData(strPrintData, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER1 + i));
	}
	// End of ADDRESS 1,2,3,PHONE NUMBER PRINT

	// Terminal and Card information
	// TERMINAL NUMBER
	AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_001), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID));

	// SEQUENCE NUMBER
	AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_002), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SEQUENCENO));

	if (debit)
	{
		// AUTHORIZATION ID
		if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
			strTemp.Format(L"%-6.6s%-2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID));
		else // STANDARD1, 2, EPS는 Audit Num과 NetworkID 사이에 SPACE 추가
			strTemp.Format(L"%-6.6s %-2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID));

		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_003), strTemp);

		// CARD NUMBER
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_006), m_sCardData.strShowNumber);
	}
	// End of Terminal and Card information

	// Currency Type and QR Codes(Public and Private Keys)
	// [#2503] US Justin 2017.09.06
	// Assign Public and Private Keys of Digital Currencies
	CString strQRPublicKey, strQRPrivateKey, strPrintType, strQRString;
	strQRPublicKey = strQRPrivateKey = strPrintType = L"";

	if (m_JustCashData.m_strCurrencyType == JUSTCASH_CURRENCY_BITCOIN)
	{
		strQRPublicKey = m_JustCashData.m_strBTPublicKey;
		strQRPrivateKey = m_JustCashData.m_strBTPrivateKey;
		strPrintType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_027);
	}
	else if (m_JustCashData.m_strCurrencyType == JUSTCASH_CURRENCY_ETHEREUM)
	{
		strQRPublicKey = m_JustCashData.m_strETAddress;
		strQRPrivateKey = m_JustCashData.m_strETPassPhase;
		strPrintType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_028);
	}

	// Print Currency Type
	AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), strPrintType);

	// Space
	AddPrintData(strPrintData, L" ");

	// Print Public and Private Key with TEXT and QRCODE on a receipt
	// PUBLIC KEY, PRIVATE KEY
	if (debit || m_JustCashData.m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_CODE)
	{
		for (i = 0; i < 2; i++)
		{
			strPrintType = strQRString = L"";

			if (i == 0)			// Public Key
			{
				strQRString = strQRPublicKey;
				strPrintType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_046);
			}
			else if (i == 1)	// Private Key
			{
				strQRString = strQRPrivateKey;
				strPrintType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_047);
			}

			if (strQRString.GetLength() > 0)
			{
				strTemp = strTemp2 = L"";
				AddPrintData(strPrintData, L"%s :", strPrintType);

				if (strQRString.GetLength() > SLIP_MAX_COL)
				{
					strTemp = strQRString.Left(SLIP_MAX_COL);
					strTemp2 = strQRString.Mid(SLIP_MAX_COL);
				}
				else
				{
					strTemp = strQRString;
				}

				AddPrintData(strPrintData, L"%s", GetCenteredString(strTemp, SLIP_MAX_COL));

				if (strTemp2.GetLength() > 0)
					AddPrintData(strPrintData, L"%s", GetCenteredString(strTemp2, SLIP_MAX_COL));

				AddPrintData(strPrintData, L"<BARCODE_QR>%s</BARCODE_QR>", strQRString);
				AddPrintData(strPrintData, L" ");
			}
		}

		// Wallet - Ethereum ONLY
		if (m_JustCashData.m_strCurrencyType == JUSTCASH_CURRENCY_ETHEREUM)
		{
			AddPrintData(strPrintData, L"%s :", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_051));
			AddPrintData(strPrintData, L"<BARCODE_QR>**USE STORED QR IMAGE FILE**:%s</BARCODE_QR>", JUSTCASH_ETHEREUM_WALLETQRIMAGE);
			AddPrintData(strPrintData, L" ");
		}
	}
	// End of [#2503]

	// Unit Price
	AddPrintData(strPrintData, L"%20s : $%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_048), m_JustCashData.m_strDCUnitPrice);

	if (debit || m_JustCashData.m_nTransactionStep != JUSTCASH_DIGITAL_CUR_CASH_CODE)
	{
		// Quantity
		AddPrintData(strPrintData, L"%20s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_049), m_JustCashData.m_strDCQuantity);

		// Total
		AddPrintData(strPrintData, L"%20s : $ %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_050), m_JustCashData.m_strDCPrice);
	}
	// End of Currency Type and QR Codes(Public and Private Keys)

	// TAIL PRINT
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER3);
	strTemp.TrimLeft();

	if (strTemp.GetLength() > 0)
	{
		AddPrintData(strPrintData, L" ");
		AddPrintData(strPrintData, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER3));
	}

	AddPrintData(strPrintData, L" ");
	// 1234567890123456789012345678901234567890
	AddPrintData(strPrintData, L" For transaction support please contact");
	AddPrintData(strPrintData, L"           support@just.cash");
	// End of TAIL PRINT

	if (m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT, TRUE, TRUE) != TRUE) // NO Header Image, Data, Wait, Cut, Print BCD
		return RES_NG;

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_JC_Commit(int nResult, BOOL bShowScreen)
{
	NVDump('O', 'C', "00", L"JUSTCSH", L"Dgt_Cur_Cmmt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_JUSTCASH_DigitalCur_Commit(isCommit=%d, showScreen=%d)]\n", nResult, bShowScreen));

	if (bShowScreen)
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(161);
		// m_pDevCmn->fnSCR_DisplayImage(1, FALSE);													// Hide Exit Button // [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
		m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161001));	// Please Wait
		m_pDevCmn->fnSCR_DisplayString(2, L"");														// Hide Cancel F8
		m_pDevCmn->fnSCR_DisplayString(3, L"");														// NO Background Image
		m_pDevCmn->fnSCR_DisplayScreen(161);
	}

	m_JustCashData.m_strDCCommit.Format(L"%d", nResult);
	m_JustCashData.m_nTransactionStep = JUSTCASH_DIGITAL_CUR_COMMIT;

	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if (nRes == RES_OK)
	{
		NVDump('O', 'C', "00", L"JUSTCSH", L"Dgt_Cur_Cmmt_OK");
		NHDEBUG(DBG_CALL, (L"Dgt_Cur_COMMIT OK\n"));
	}
	else
	{
		NHDEBUG(DBG_CALL, (L"Dgt_Cur_COMMIT FAILURE\n"));
		NVDump('O', 'C', "00", L"JUSTCSH", L"Dgt_Cur_Cmmt_NG");
	}

	return nRes;
}

BIZ_RETURN CTranCmn::P_NH_JC_ConfirmWallet()
{
	NHDBG((L"[CTranCmn::P_NH_JC_ConfirmWallet]\n"));
	NVDump('O', 'C', "2J", L"Start", L"ConfirmWallet");

	CString GetKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (!IsAdaTransaction())
	{
		CString selectedCoin;
		selectedCoin.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_163101));

		m_pDevCmn->fnSCR_DisplayPrevSet(508);
		m_pDevCmn->fnSCR_DisplayFormat(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_508000), selectedCoin, selectedCoin);
		m_pDevCmn->fnSCR_DisplayScreen(508, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "20", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(508, L"508.wav");

			m_pAdaCtrl->fnExp_PlayScreenWave(508);

			replay = FALSE;
		}

		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (IsAdaTransaction() && GetKeyStr.GetLength() > 6)
			{
				GetKeyStr = GetKeyStr.Mid(6);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));

				if (GetKeyStr == L"." || GetKeyStr == L"00" || GetKeyStr == L"000") // Repeat or Volume Change
				{
					m_pAdaCtrl->fnExp_StopPlay();

					// Handle volume change repeats
					if (GetKeyStr == L".")
						m_pAdaCtrl->fnExp_SetVolumeDown();
					else if (GetKeyStr == L"00")
						m_pAdaCtrl->fnExp_SetVolumeUp();

					// Repeat after a volume change
					replay = TRUE;

					continue;
				}
				else if (GetKeyStr == L"1")
				{
					GetKeyStr = L"NEXT";
				}
				else if (GetKeyStr == L"3")
				{
					GetKeyStr = L"REPRINT";
				}
			}

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "2J", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "2J", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == L"NEXT")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"508_NextSelected.wav", TRUE);
				}

				return RES_OK;
			}
			else if (GetKeyStr == L"REPRINT")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"508_ReprintSelected.wav", TRUE);
				}

				// JCTODO: Reprint paper wallet
				return RES_USER_PREV;
			}
			else if (IsAdaTransaction())
			{
				replay = TRUE;

				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_JC_InsertCard()
{
	NVDump('O', 'C', "-1", L"", L"CARD");
	NHDBG((L"[CTranCmn::P_NH_JC_InsertCard]\n"));

	CString GetKeyStr;
	BOOL bShowScreen = TRUE;
	BOOL replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);
	m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_ON);

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(502);
		SetCardReadScreenValue(L"");
		m_pDevCmn->fnSCR_DisplayScreen(502, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "-1", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();

			// Begin Transaction
			// [#2380] US Justin 2015.12.21 Cardtronics Additional VG
			m_pAdaCtrl->fnExp_AddWaveFile(502, L"Please_Insert.wav");

			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
				m_pAdaCtrl->fnExp_AddWaveFile(502, L"Insertion_Direction_V.wav");
			else
				m_pAdaCtrl->fnExp_AddWaveFile(502, L"Insertion_Direction_H.wav");

			if (P_EMV_CheckTransMode() == RES_NOR_MS_TRANS)
				m_pAdaCtrl->fnExp_AddWaveFile(502, L"Smooth_remove.wav");
			else
				m_pAdaCtrl->fnExp_AddWaveFile(502, L"Donot_remove.wav");
			// End of [#2380]

			m_pAdaCtrl->fnExp_PlayScreenWave(502);

			replay = FALSE;
		}

		if ((DidMediaExist() == FALSE) && (LIB_MainMenuDeviceEvent() == DEV_MCU) && (m_pDevCmn->fnMCU_GetDeviceStatus() == NORMAL))
		{
			TurnOffCardReaderFlicker();

			BIZ_RETURN cardReadResult = VATReadCard(125, 127, false);
			if (cardReadResult != RES_OK)
			{
				NHERROR((L"Card read failed\r\n"));

				CString result;
				result.Format(L"%d", cardReadResult);
				NVDump('F', 'C', "-1", L"NG_0", result);
				return RES_NG;
			}

			return RES_OK;
		}
		else if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (IsAdaTransaction() && GetKeyStr.GetLength() > 6)
			{
				GetKeyStr = GetKeyStr.Mid(6);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));

				if (GetKeyStr == L"." || GetKeyStr == L"00" || GetKeyStr == L"000") // Repeat or Volume Change
				{
					m_pAdaCtrl->fnExp_StopPlay();

					// Handle volume change repeats
					if (GetKeyStr == L".")
						m_pAdaCtrl->fnExp_SetVolumeDown();
					else if (GetKeyStr == L"00")
						m_pAdaCtrl->fnExp_SetVolumeUp();

					// Repeat after a volume change
					replay = TRUE;

					continue;
				}
			}

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NHDEBUG(DBG_INFO, (_T("CANCEL OR EXIT BUTTON PRESSED\n")));
				TurnOffCardReaderFlicker(TRUE);
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				TurnOffCardReaderFlicker(TRUE);
				return RES_USER_TIMEOUT;
			}
			else if (IsAdaTransaction())
			{
				replay = TRUE;

				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
		}

		Delay_Msg(50);
	}

	NVDump('F', 'C', "-1", L"NG_2", GetKeyStr);

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_JC_EnterAmount()
{
	NVDump('O', 'C', "00", L"P_NHNOR", L"Dgt_Cur_Amt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_JUSTCASH_DigitalCur_EnterAmount]\n"));

	int		nRetryCount = 3;
	int		nInputAmount = 0;
	CString	GetKeyStr;
	BOOL	bShowScreen = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
	{
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(113);

			// History
			for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
			{
				if (nHistory == 0)
					m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

				m_pDevCmn->fnSCR_DisplayHistory(nHistory + 1, m_arHistory[nHistory]);
			}

			// Title
			m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113001));

			// Sub Title
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113700));

			m_pDevCmn->fnSCR_DisplayString(4, L"DOLLAR");	// Currency Symbol
			m_pDevCmn->fnSCR_DisplayString(5, L"INT_CENT");

			CString strTemp;
			m_pDevCmn->fnSCR_DisplayString(8, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_135112));
			strTemp.Format(L"$ %s", m_JustCashData.m_strMinAmount);
			m_pDevCmn->fnSCR_DisplayString(9, strTemp);

			m_pDevCmn->fnSCR_DisplayString(10, L"1");		// min
			m_pDevCmn->fnSCR_DisplayString(11, L"8");		// max
			m_pDevCmn->fnSCR_DisplayString(12, L"off");		// auto run

			m_pDevCmn->fnSCR_DisplayScreen(113, KEYIN_TIME_OUT, PIN_MENU_MODE);

			bShowScreen = FALSE;

#ifdef APP_AGING_MODE
			g_AgingCheck.SetTargetTimeAfterSec(AGING_KEYIN_TIME);
#endif
		}

#ifdef APP_AGING_MODE
		if (g_AgingCheck.IsElapsedTimes() == TRUE)
		{
			m_sUserSelection.strMoney = L"000000002000";

			return RES_OK;
		}
#else
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_49");

				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "00", L"P_NHNOR", L"NG_50");

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr.GetLength() > 0)
			{
				GetKeyStr.Replace(GetCurrencySymbol(), L"");
				GetKeyStr.TrimLeft();

				nInputAmount = Asc2Int(GetKeyStr);

				NHDEBUG(DBG_INFO, (_T("Input Amount to convert integer [%d]\n"), nInputAmount));

				if (nInputAmount > 0)
				{
					m_sUserSelection.strMoney.Format(L"%010d", nInputAmount);

					return RES_OK;
				}
				else
				{
					LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113105), INFO_SCR_TIMEOUT);

					nRetryCount--;
					bShowScreen = TRUE;
				}
			}
		}
#endif

		Delay_Msg(50);
	}

	if (nRetryCount <= 0)
	{
		NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
		LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);
		NVDump('O', 'C', "00", L"P_NHNOR", L"NG_51");

		return RES_USER_EXIT;
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHNOR", L"NG_52");

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_JC_CollectCash()
{
	NHDBG((L"[CTranCmn::P_NH_JC_CollectCash]\n"));
	NVDump('O', 'C', "2O", L"Start", L"Cash");

	CString GetKeyStr;
	BIZ_RETURN nRes = RES_NG;
	BOOL replay = TRUE;
	BOOL bShowCancel = TRUE;
	BOOL bUpdateScreen = FALSE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	int amountCollected = 0;
	int transactionAmount = Asc2Int(m_JustCashData.m_strDCPrice);
	int maxCashAllowed = Asc2Int(m_JustCashData.m_strMaxAmount);

	if (transactionAmount > 0)
	{
		bShowCancel = FALSE;
		transactionAmount /= 100;
	}

#ifdef UNDER_CE
	BOOL bShowInsertOptions = FALSE;
#else
	BOOL bShowInsertOptions = TRUE;
#endif

	// BitCoin
	CString selectedCoin;
	selectedCoin.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_163101));

	if (!IsAdaTransaction())
	{
		// Show screen
		m_pDevCmn->fnSCR_DisplayPrevSet(504);
		m_pDevCmn->fnSCR_DisplayFormat(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_390000), selectedCoin);
		m_pDevCmn->fnSCR_DisplayFormat(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_390001), CURRENCY_SYMBOL, (double)transactionAmount);
		m_pDevCmn->fnSCR_DisplayFormat(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_390002), CURRENCY_SYMBOL, (double)maxCashAllowed);
		m_pDevCmn->fnSCR_DisplayString(4, L"");

		// Buttons
		if (bShowCancel)
		{
			m_pDevCmn->fnSCR_DisplayImage(1, FALSE);			// F2, ENTER
			m_pDevCmn->fnSCR_DisplayImage(2, TRUE);				// F8, CANCEL
		}
		else
		{
			m_pDevCmn->fnSCR_DisplayImage(1, TRUE);				// F2, ENTER
			m_pDevCmn->fnSCR_DisplayImage(2, FALSE);			// F8, CANCEL
		}

		m_pDevCmn->fnSCR_DisplayImage(3, bShowInsertOptions);	// F1, Insert $1
		m_pDevCmn->fnSCR_DisplayImage(4, bShowInsertOptions);	// F3, Insert $5
		m_pDevCmn->fnSCR_DisplayImage(5, bShowInsertOptions);	// F5, Insert $10
		m_pDevCmn->fnSCR_DisplayImage(6, bShowInsertOptions);	// F7, Insert $20
		m_pDevCmn->fnSCR_DisplayImage(7, bShowInsertOptions);	// F4, Insert $50
		m_pDevCmn->fnSCR_DisplayImage(8, bShowInsertOptions);	// F6, Insert $100

		m_pDevCmn->fnSCR_DisplayScreen(504, K_NO_WAIT, PIN_MENU_MODE);
	}

	bool rollbackCashOnEnd = true;
	bool acceptCash = true;

	if (H_NH_JC_StartCashIn())
	{
		// BNA FLICKER ON
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_BNA, FLICKER_ON);

		while (acceptCash)
		{
			if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "-1", L"P_NHADA", L"NG0");

				m_pAdaCtrl->fnExp_StopPlay();

				return RES_NG;
			}

			if (IsAdaTransaction() && replay)
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();

				CString strTemp;
				strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"504.wav"), selectedCoin, CURRENCY_SYMBOL, (double)maxCashAllowed, CURRENCY_SYMBOL, (double)transactionAmount);
				m_pAdaCtrl->fnExp_AddWaveFile(504, strTemp);

				if (bShowCancel)
				{
					m_pAdaCtrl->fnExp_AddWaveFile(504, L"504_Cancel.wav");
				}
				else
				{
					m_pAdaCtrl->fnExp_AddWaveFile(504, L"504_Done.wav");
				}

				if (bShowInsertOptions)
				{
					m_pAdaCtrl->fnExp_AddWaveFile(504, L"504_InsertOptions.wav");
				}

				m_pAdaCtrl->fnExp_PlayScreenWave(504);

				replay = FALSE;
			}

			// Listen for KeyPad
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				CString GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString();
				NHDEBUG(DBG_CALL, (L"GetKeyString(): [%s]\n", GetKeyStr));

				if (IsAdaTransaction() && GetKeyStr.GetLength() > 6)
				{
					GetKeyStr = GetKeyStr.Mid(6);

					NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));

					if (GetKeyStr == L"." || GetKeyStr == L"00" || GetKeyStr == L"000") // Repeat or Volume Change
					{
						m_pAdaCtrl->fnExp_StopPlay();

						// Handle volume change repeats
						if (GetKeyStr == L".")
							m_pAdaCtrl->fnExp_SetVolumeDown();
						else if (GetKeyStr == L"00")
							m_pAdaCtrl->fnExp_SetVolumeUp();

						// Repeat after a volume change
						replay = TRUE;

						continue;
					}
					else if (GetKeyStr == L"1")
					{
						if (bShowCancel)
						{
							GetKeyStr = S_CANCEL;
						}
						else
						{
							GetKeyStr = L"DONE";
						}
					}
					else if (bShowInsertOptions)
					{
						if (GetKeyStr == L"2")
						{
							GetKeyStr = L"F1";
						}
						else if (GetKeyStr == L"3")
						{
							GetKeyStr = L"F3";
						}
						else if (GetKeyStr == L"5")
						{
							GetKeyStr = L"F5";
						}
						else if (GetKeyStr == L"7")
						{
							GetKeyStr = L"F7";
						}
						else if (GetKeyStr == L"4")
						{
							GetKeyStr = L"F4";
						}
						else if (GetKeyStr == L"6")
						{
							GetKeyStr = L"F6";
						}
					}
				}

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					if (IsAdaTransaction())
					{
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"504_CancelSelected.wav", TRUE);
					}

					g_sBizFlowInfo.nReasonforCancel = RES_USER_EXIT;
					nRes = RES_USER_EXIT;
					break;
				}
				else if (GetKeyStr == S_TIMEOVER)
				{
					g_sBizFlowInfo.nReasonforCancel = RES_USER_TIMEOUT;
					nRes = RES_USER_TIMEOUT;
					break;
				}
				else if (GetKeyStr == L"DONE")
				{
					if (IsAdaTransaction())
					{
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"504_DoneSelected.wav", TRUE);
					}

					nRes = RES_OK;
					break;
				}
				else if (bShowInsertOptions)
				{
					if (GetKeyStr == L"F1")
					{
						if (IsAdaTransaction())
						{
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"504_1Selected.wav", TRUE);
						}

						amountCollected = 1;
						bUpdateScreen = TRUE;
					}
					else if (GetKeyStr == L"F3")
					{
						if (IsAdaTransaction())
						{
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"504_5Selected.wav", TRUE);
						}

						amountCollected = 5;
						bUpdateScreen = TRUE;
					}
					else if (GetKeyStr == L"F5")
					{
						if (IsAdaTransaction())
						{
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"504_10Selected.wav", TRUE);
						}

						amountCollected = 10;
						bUpdateScreen = TRUE;
					}
					else if (GetKeyStr == L"F7")
					{
						if (IsAdaTransaction())
						{
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"504_20Selected.wav", TRUE);
						}

						amountCollected = 20;
						bUpdateScreen = TRUE;
					}
					else if (GetKeyStr == L"F4")
					{
						if (IsAdaTransaction())
						{
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"504_50Selected.wav", TRUE);
						}

						amountCollected = 50;
						bUpdateScreen = TRUE;
					}
					else if (GetKeyStr == L"F6")
					{
						if (IsAdaTransaction())
						{
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"504_100Selected.wav", TRUE);
						}

						amountCollected = 100;
						bUpdateScreen = TRUE;
					}
					else if (IsAdaTransaction())
					{
						replay = TRUE;

						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					}
				}
				else if (IsAdaTransaction())
				{
					replay = TRUE;

					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				}

				if (bUpdateScreen)
				{
					if (P_NH_JC_ConfirmDebit(amountCollected) == RES_OK)
					{
						// Sum the transaction total
						// Cash will retained at this point no matter what happens
						// with store procedure
						transactionAmount += amountCollected;

						// Update screen
						m_pDevCmn->fnSCR_DisplayFormat(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_390001), CURRENCY_SYMBOL, (double)transactionAmount);

						// Buttons
						bShowCancel = false;

						m_pDevCmn->fnSCR_DisplayImage(1, TRUE);		// F2, ENTER
						m_pDevCmn->fnSCR_DisplayImage(2, FALSE);	// F8, CANCEL

						m_pDevCmn->fnSCR_DisplayUpdate(504);
					}
				}
			}
			// Listen for cash acceptor
			else if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_BNA))
			{
				amountCollected = 0;
				NHDEBUG(DBG_CALL, (L"fnAPL_GetDeviceEvent(DEV_BNA): [%d]\n", L" "));

				m_pDevCmn->fnSCR_DisplayString(4, L" ");
				m_pDevCmn->fnSCR_DisplayUpdate(504);

				// Check device event
				if (m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BNA))
				{
					// Get the amount collected
					amountCollected = m_pDevCmn->fnBNA_GetLastCashInValue();

					// If amount is less than zero
					// could be item refused or timeout
					if (amountCollected > 0)
					{
						// Send payment to JC
						if (P_NH_JC_ConfirmDebit(amountCollected) == RES_OK)
						{
							// Sum the transaction total
							// Cash will retained at this point no matter what happens
							// with store procedure
							transactionAmount += amountCollected;

							// Update screen
							m_pDevCmn->fnSCR_DisplayFormat(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_390001), CURRENCY_SYMBOL, (double)transactionAmount);

							// Buttons
							bShowCancel = false;

							m_pDevCmn->fnSCR_DisplayImage(1, TRUE);		// F2, ENTER
							m_pDevCmn->fnSCR_DisplayImage(2, FALSE);	// F8, CANCEL

							m_pDevCmn->fnSCR_DisplayUpdate(504);

							// Store the cash
							if (H_NH_JC_StoreCash())
							{
								// Restart the accept process
								if (!H_NH_JC_StartCashIn())
								{
									// If we can't restart cash process end the procedure
									nRes = RES_NG;
									break;
								}
							}
							else
							{
								// Customer got credit for the money inserted
								// Don't try to return the money to them
								// at the end of the transaction
								nRes = RES_NG;
								rollbackCashOnEnd = false;
								break;
							}
						}
						else
						{
							// Denied by JC, return the cash
							H_NH_JC_ReturnCash();

							// Customer may have exceeded limits with bill. If they still have
							// bandwidth, they may insert additional smaller bills.
							if (transactionAmount < maxCashAllowed)
							{
								m_pDevCmn->fnSCR_DisplayFormat(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_390003), CURRENCY_SYMBOL, (double)(amountCollected + transactionAmount));
								m_pDevCmn->fnSCR_DisplayUpdate(504);

								if (!H_NH_JC_StartCashIn())
								{
									// If we can't restart cash process end the procedure
									nRes = RES_NG;
									break;
								}
							}
							else
							{
								nRes = RES_NG;
								break;
							}
						}
					}
					else
					{
						CString status = m_pDevCmn->fstrCMN_CheckDeviceAction(DEV_BNA, 5);

						// Determine if hte BNA had timeout
						if (status == "TIMEOUT")
						{
							nRes = RES_USER_TIMEOUT;
							break;
						}
						else
						{
							// Restart the accept process
							if (!H_NH_JC_StartAcceptCash())
							{
								// If we can't restart cash process end the procedure
								nRes = RES_NG;
								break;
							}
						}
					}
				}
				else
				{
					CString error = m_pDevCmn->fstrBNA_GetErrorCode();
					bool canContinue = false;

					if (error == "9770195")
					{
						m_pDevCmn->fnBNA_ClearErrorCode();

						if (H_NH_JC_StartAcceptCash())
						{
							NHDEBUG(DBG_CALL, (L"fnAPL_GetDeviceEvent(DEV_BNA,EVENT_IN): Restarted cash in after error \n"));
							canContinue = true;
						}
						else
						{
							NHDEBUG(DBG_CALL, (L"fnAPL_GetDeviceEvent(DEV_BNA,EVENT_IN): Failed to restarted cash in After Error \n"));
						}
					}

					if (!canContinue)
					{
						nRes = RES_NG;
						break;
					}
				}
			}
			else if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_BNA, EVENT_OUT))
			{
				NHDEBUG(DBG_CALL, (L"fnAPL_GetDeviceEvent(DEV_BNA,EVENT_OUT): [%d]\n", L" "));

				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BNA);
				m_pDevCmn->fnAPL_CheckDevice();

				CString error = m_pDevCmn->fstrBNA_GetErrorCode();
				NHDEBUG(DBG_CALL, (L"fnAPL_GetDeviceEvent(DEV_BNA,EVENT_OUT): [%s]\n", error));
			}

			Delay_Msg(50);
		}

		// BNA FLICKER OFF
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_BNA, FLICKER_OFF);
	}
	else
	{
		NHDEBUG(DBG_CALL, (L"Failed to start cash in \n"));
		nRes = RES_NG;
	}

	m_pDevCmn->fnBNA_CancelAccept();

	// Don't try to rollback cash if we had an error
	// storing the cash

	if (rollbackCashOnEnd)
	{
		CString stackerStatus = m_pDevCmn->fstrBNA_GetStackerStatus();

		if (stackerStatus == "NOTEMPTY" || stackerStatus == "FULL")
		{
			int amount = m_pDevCmn->fnBNA_GetLastCashInValue();
			CString errorCode = m_pDevCmn->fstrBNA_GetErrorCode();

			// JCTODO: Journaling
			m_JustCashData.RecodeDispenseResult(amount, JUSTCASH_SERVICE_DIGITAL_CURRENCY);
		}
		else
		{
			m_JustCashData.RecodeDispenseResult(0, JUSTCASH_SERVICE_DIGITAL_CURRENCY);
		}

		H_NH_JC_ReturnCash(); // to cancel StartCashIn
	}

	if (transactionAmount > 0)
	{
		m_JustCashData.m_strDCPrice.Format(L"%.2f", (float)transactionAmount);
		nRes = RES_OK;
	}

	return nRes;
}

BIZ_RETURN CTranCmn::P_NH_JC_ConfirmDebit(int amount)
{
	m_pDevCmn->fnBNA_GetCashUnitInformation(); // update cashin counts

	m_JustCashData.m_strAmount = Int2Asc(amount);
	m_JustCashData.m_strVaultBalance = Int2Asc(m_pDevCmn->fnBNA_GetTotalCount());
	m_JustCashData.m_nTransactionStep = JUSTCASH_DIGITAL_CUR_CASH_ADD;

	if (P_NH_NOR_Transaction(FALSE) != RES_OK)
	{
		P_NH_JC_Error();
		NVDump('O', 'C', "-1", L"", L"ERR_ADD");

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_JC_CompleteTransaction()
{
	m_pDevCmn->fnBNA_GetCashUnitInformation(); // update cashin counts

	m_JustCashData.m_strVaultBalance = Int2Asc(m_pDevCmn->fnBNA_GetTotalCount());
	m_JustCashData.m_nTransactionStep = JUSTCASH_DIGITAL_CUR_CASH_COMMIT;

	if (P_NH_NOR_Transaction(FALSE) != RES_OK)
	{
		P_NH_JC_Error();
		NVDump('O', 'C', "-1", L"", L"ERR_ADD");

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_JC_Error()
{
	CString message;

	if (m_JustCashData.m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_ADD)
	{
		message.Format(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_507000), m_JustCashData.m_strServerMessage);
	}
	else
	{
		message.Format(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_507001), m_JustCashData.m_strServerMessage);
	}

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(507);
		m_pDevCmn->fnSCR_DisplayString(2, message);
		m_pDevCmn->fnSCR_DisplayScreen(507);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

		m_pAdaCtrl->fnExp_StopAndResetWaveFile();
		m_pAdaCtrl->fnExp_AddWaveFile(507, message);

		m_pAdaCtrl->fnExp_PlayScreenWave(507);

		return RES_OK;
	}

	Delay_Msg(5000);

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_JC_TransactionData()
{
	// Increase Sequence Number
	m_pDevCmn->fnAPL_AddSerialNo(m_HostConfig);

	// Approved => Assign Value for Receipt
	CString strTemp, strTemp2;
	CTime ct = CTime::GetCurrentTime();
	strTemp2.Format(L"%04d", ct.GetYear());

	// Host Date, Settlement date
	strTemp.Format(L"%02d%02d%s", ct.GetMonth(), ct.GetDay(), strTemp2.Right(2));
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE, strTemp);
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSSETTLEDATE, strTemp);

	// Host Time
	strTemp.Format(L"%02d%02d%02d", ct.GetHour(), ct.GetMinute(), ct.GetSecond());
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME, strTemp);

	// Match with Standard1 format
	// MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM, m_JustCashData.m_strApprovalCode.Left(6));
	// MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID, m_JustCashData.m_strApprovalCode.Mid(6, 2));

	// Operation Code
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE, TRANTYPE_NONCASH_WITHDRAW);

	// Request Amount
	strTemp.Format(L"%d", Asc2Int(m_JustCashData.m_strDCPrice));
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT, strTemp);

	// Set as Successful Dispensing
	m_pDevCmn->fnAPL_SetProcCount('6');

	MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

	// Save JNL and Update Statistics
	P_NH_NOR_SaveCWStatus(FALSE);

	// Surcharge Amount
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT, L"0");
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEFLAG, L"0");

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_JC_ThankYou()
{
	CString message = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_123004);

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(503);
		m_pDevCmn->fnSCR_DisplayString(1, message);
		m_pDevCmn->fnSCR_DisplayScreen(503);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

		m_pAdaCtrl->fnExp_StopAndResetWaveFile();
		m_pAdaCtrl->fnExp_AddWaveFile(503, message);

		m_pAdaCtrl->fnExp_PlayScreenWave(503);

		return RES_OK;
	}

	Delay_Msg(2000);

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_JC_Cancel()
{
	m_pDevCmn->fnSCR_DisplayPrevSet(122);
	m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
	m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122001));
	m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
	m_pDevCmn->fnSCR_DisplayScreen(122);

	m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_JC_Enter_CashCode()
{
	NVDump('O', 'C', "00", L"JUSTCSH", L"JustCashCode");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_JUSTCASH_Enter_CashCode]\n"));

	CString	GetKeyStr;
	int		nRetryCount = 3;
	BOOL	bShowScreen = TRUE;

	// ADA Variable
	BIZ_RETURN	nTimeOutCheck;
	BOOL		bStartToTimeout =	FALSE;
	BOOL		bInvalidRepeat =	FALSE;
	CString		strADAVGInput;
	int			nValidADAInput =	0;
	CString		strUserADAInputString;
	int			nUserInputValidation;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
	{
		if (IsAdaTransaction())
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_78");

				m_pAdaCtrl->fnExp_StopPlay();

				return RES_USER_EXIT;
			}

			if ((bInvalidRepeat) && (m_pAdaCtrl->IsPlaying() == FALSE))
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}

		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			if (IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
				m_pAdaCtrl->fnExp_StopAndResetWaveFile();

#if (US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(160, L"160.wav");
#endif

				m_pAdaCtrl->fnExp_AddWaveFile(160, L"LocateCancelKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(160, L"LocationRepeatKey.wav");
				m_pAdaCtrl->fnExp_PlayScreenWave(160);

				strUserADAInputString = L"";
				bStartToTimeout = TRUE;
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(160);

				// History
				for (int nHistory = 0; nHistory < m_arHistory.GetCount(); nHistory++)
				{
					if (nHistory == 0)
						m_pDevCmn->fnSCR_DisplayHistory(m_arHistory.GetCount());

					m_pDevCmn->fnSCR_DisplayHistory(nHistory + 1, m_arHistory[nHistory]);
				}

				// Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_160001));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_160002));

				m_pDevCmn->fnSCR_DisplayScreen(160, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
			}

			bShowScreen = FALSE;
		}

		if ((IsAdaTransaction()) && (m_pAdaCtrl->IsPlaying() == FALSE) && (bStartToTimeout == TRUE))
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));

			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);

			bStartToTimeout = FALSE;
		}

		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			bInvalidRepeat = FALSE;

			if ((IsAdaTransaction()) && (GetKeyStr.GetLength() > 6))
			{
				GetKeyStr = GetKeyStr.Mid(6);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
#if (US_VERSION)
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
				}
#endif

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
				nUserInputValidation = 0;

				if (!IsAdaTransaction()) // Screen Mode....
				{
					strUserADAInputString = GetKeyStr;
					nUserInputValidation = 1; // Perform Validation
				}
				else // ADA MODE
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));

					nValidADAInput = 0;

					if ((GetKeyStr.GetLength() == 1) && (GetKeyStr != L".")) // Numbers.... 0,1,2,3,4,5,6,7,8,9
					{
						strADAVGInput = GetKeyStr + L".wav";

						if (strUserADAInputString.GetLength() <= 4) // Maximum 5 digits
							strUserADAInputString += GetKeyStr;

						nValidADAInput = 1; // (1) Need to read entered key (US)
					}
					else if (GetKeyStr == S_CLEAR) // Clear
					{
						strADAVGInput = L"ClearReEntry.wav";
						strUserADAInputString = L"";
						nValidADAInput = 1; // (1) Need to read entered key (US)
					}
					else if ((GetKeyStr == L".") || (GetKeyStr == L"00") || (GetKeyStr == L"000")) // Repeat or Volume Change
					{
						m_pAdaCtrl->fnExp_StopPlay();

						if (GetKeyStr == L".")
							m_pAdaCtrl->fnExp_SetVolumeDown();
						else if (GetKeyStr == L"00")
							m_pAdaCtrl->fnExp_SetVolumeUp();

						nValidADAInput = 2; // (2) Repeat of Volume Change
					}
					else if (GetKeyStr == S_ENTER)
					{
						NHDEBUG(DBG_INFO, (_T("ENTERED CODE [%s]\n"), strUserADAInputString));

						nUserInputValidation = 1; // Perform Code Validation

#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Enter.wav", TRUE); // Other countries => read at "MainFrm.cpp"
#endif
					}

					if (nValidADAInput == 1) // Read Entered keys (Numbers and Clear Key : US TTS Only)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance : Numbers and Clear Key \n"));

#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(strADAVGInput, FALSE);
#endif
					}
					else if (nValidADAInput == 2) // 2(Repeat or Volume Change)
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));

						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);

						bShowScreen = TRUE;
					}
				}

				if (nUserInputValidation == 1)
				{
					NHDEBUG(DBG_INFO, (L"Validating User Input[%s]\n", strUserADAInputString));

					if (strUserADAInputString.GetLength() == 5)
					{
						m_JustCashData.m_strCashCode = strUserADAInputString;

						NHDEBUG(DBG_INFO, (_T("  Entered Cash Code - [%s]\n"), strUserADAInputString));

						// Save Cash code for journal
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA, strUserADAInputString);

						return RES_OK;
					}
					else
					{
						NHDEBUG(DBG_INFO, (L"length of entered code is not 5\n"));
					}

					nRetryCount--;

					if (nRetryCount > 0)
					{
						if (!IsAdaTransaction()) // Screen Mode....
						{
							LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_160101), INFO_SCR_TIMEOUT);
						}
						else
						{
#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"InvalidCashCode.wav", TRUE);
#endif

							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						}

						bShowScreen = TRUE;
					}
				}
			}
		}

		Delay_Msg(50);

		nTimeOutCheck = P_NH_ADA_Check_TimeOUT();

		if (nTimeOutCheck == RES_OK)
			bShowScreen = TRUE;
		else if (nTimeOutCheck == RES_USER_EXIT)
			return RES_USER_EXIT;
	}

	if (nRetryCount <= 0)
	{
		if (!IsAdaTransaction())
			LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_113112), ABORT_SCR_TIMEOUT);

		NVDump('O', 'C', "00", L"P_NHNOR", L"NG_51");

		return RES_USER_EXIT;
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_JC_Submit_CashCode()
{
	NVDump('O', 'C', "00", L"JUSTCSH", L"SbmtCode");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_JUSTCASH_Submit_CashCode]\n"));

	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	// m_pDevCmn->fnSCR_DisplayImage(1, FALSE);													// Hide Exit Button // [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
	m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161001));	// Please Wait
	m_pDevCmn->fnSCR_DisplayString(2, L"");														// Hide Cancel F8
	m_pDevCmn->fnSCR_DisplayString(3, L"");														// NO Background Image
	m_pDevCmn->fnSCR_DisplayScreen(161);

	m_JustCashData.m_nTransactionStep = JUSTCASH_CARDLESS_TRAN_SUBMITCODE;

	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if (nRes == RES_OK)
	{
		NHDEBUG(DBG_CALL, (L"Submit Code OK\n"));

		int nInputAmountFull = Dollar2Cent(m_JustCashData.m_strTranAmount);
		int nInputAmountDollar = (int)(0.01 * nInputAmountFull + 0.0001);

		if ((nInputAmountFull != (nInputAmountDollar * 100)) || (!m_pDevCmn->fbCDU_IsDispensible(nInputAmountDollar)))
		{
			// Not Dispensible... Cash Shortage or Denomination Errors
			NHDEBUG(DBG_CALL, (L"Just Cash Received Amt[%s] is not Dispensible\n", m_JustCashData.m_strTranAmount));

			m_JustCashData.m_nTransactionStep = JUSTCASH_CARDLESS_TRAN_UPDATETRAN;
			nRes = P_NH_NOR_Transaction(FALSE);

			return RES_NG;
		}

		// Assign User Selection
		m_sUserSelection.strMoney.Format(L"%010d00", nInputAmountDollar);

		if (m_JustCashData.m_bIsOnUsTransaction == TRUE) // ON-US TRANSACTION
		{
			// Increase Sequence Number
			m_pDevCmn->fnAPL_AddSerialNo(m_HostConfig);

			// Assign Dummy Card Number
			m_sCardData.strAccountNo = L"123456789012";
			m_sCardData.strBankID = L"1234";
			m_sCardData.strISO2Data = L"1234567890123456=9912";
			m_sCardData.strShowNumber.Format(L"1-%s", m_JustCashData.m_strCashCode);

			// Approved => Assign Value for Receipt
			CString strTemp, strTemp2;
			CTime ct = CTime::GetCurrentTime();
			strTemp2.Format(L"%04d", ct.GetYear());

			// Host Date, Settlement date
			strTemp.Format(L"%02d%02d%s", ct.GetMonth(), ct.GetDay(), strTemp2.Right(2));
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE, strTemp);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSSETTLEDATE, strTemp);

			// Host Time
			strTemp.Format(L"%02d%02d%02d", ct.GetHour(), ct.GetMinute(), ct.GetSecond());
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME, strTemp);

			// Match with Standard1 format
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM, m_JustCashData.m_strApprovalCode.Left(6));
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID, m_JustCashData.m_strApprovalCode.Mid(6, 2));

			// Operation Code
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE, TRANTYPE_WITHDRAWAL);

			// Request Amount
			strTemp.Format(L"%d", Asc2Int(m_sUserSelection.strMoney));
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT, strTemp);

			// Surcharge Amount
			MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT, L"0");
			MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEFLAG, L"0");
		}
		else
		{
			// Parse Track2 data
			CStringArray strArrayTrack;
			SplitString(m_JustCashData.m_strTrackData, L"=", strArrayTrack);

			CString sAccountNum = strArrayTrack.GetAt(0);

			m_sCardData.strISO2Data = m_JustCashData.m_strTrackData;
			m_sCardData.strAccountNo = sAccountNum.Mid(3, 12);
			m_sCardData.strBankID = sAccountNum.Right(4);
			m_sCardData.strShowNumber.Format(L"0-%s", m_JustCashData.m_strCashCode);
		}

		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKCODE, m_sCardData.strBankID);
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKACCOUNT, m_sCardData.strAccountNo);
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA, m_sCardData.strShowNumber);
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2, m_sCardData.strISO2Data);

		return RES_OK;
	}

	NHDEBUG(DBG_CALL, (L"Fail to Get valid data for Submit Code\n"));

	return RES_NG;
}

BIZ_RETURN CTranCmn::P_NH_JC_Authorization()
{
	NVDump('O', 'C', "00", L"JUSTCSH", L"AUTH");

	if (m_JustCashData.m_bIsOnUsTransaction == TRUE) // On-US Transaction => Dispense without additional Authorization
		return RES_OK;

	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	// m_pDevCmn->fnSCR_DisplayImage(1, FALSE);													// Hide Exit Button // [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
	m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161001));	// Please Wait
	m_pDevCmn->fnSCR_DisplayString(2, L"");														// Hide Cancel F8
	m_pDevCmn->fnSCR_DisplayString(3, L"");														// NO Background Image
	m_pDevCmn->fnSCR_DisplayScreen(161);

	m_JustCashData.m_nTransactionStep = JUSTCASH_CARDLESS_TRAN_HOSTAPPROVAL;

	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if (nRes == RES_OK)
		NHDEBUG(DBG_CALL, (L"HOST AUTH OK\n"));
	else
		NHDEBUG(DBG_CALL, (L"FAIL TO HOST AUTH\n"));

	return nRes;
}

BIZ_RETURN CTranCmn::P_NH_JC_UpdateTransaction(int nDispResult)
{
	NVDump('O', 'C', "00", L"JUSTCSH", L"UPDT_TR");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_JUSTCASH_UpdateTransaction(nAmount=%d)]\n", nDispResult));

	m_JustCashData.m_strTranDispensed.Format(L"%0.2f", 0.01 * nDispResult);
	m_JustCashData.RecodeDispenseResult(nDispResult, JUSTCASH_SERVICE_CARDLESS);

	m_pDevCmn->fnSCR_DisplayPrevSet(161);
	// m_pDevCmn->fnSCR_DisplayImage(1, FALSE);													// Hide Exit Button // [#2529] NH Justin 2018.02.20 Change Name Value APSTATE1 => APVALUE2
	m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_161001));	// Please Wait
	m_pDevCmn->fnSCR_DisplayString(2, L"");														// Hide Cancel F8
	m_pDevCmn->fnSCR_DisplayString(3, L"");														// NO Background Image
	m_pDevCmn->fnSCR_DisplayScreen(161);

	m_JustCashData.m_nTransactionStep = JUSTCASH_CARDLESS_TRAN_UPDATETRAN;

	BIZ_RETURN nRes = P_NH_NOR_Transaction(FALSE);

	if (nRes == RES_OK)
		NHDEBUG(DBG_CALL, (L"Update Tran OK\n"));
	else
		NHDEBUG(DBG_CALL, (L"Fail to Update Tran\n"));

	return nRes;
}

#endif