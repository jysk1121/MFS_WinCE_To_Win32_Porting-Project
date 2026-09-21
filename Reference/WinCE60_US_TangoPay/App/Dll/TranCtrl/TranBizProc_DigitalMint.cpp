#include "stdafx.h"
#include ".\Scr\ScrCtrl.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"

#include ".\Common\NHDbgApi.h"
#include ".\Tran\DigitalMintTypes.h"

#if (APP_DIGITALMINT)

#define MAX_COINS		3
#define DM_UNKNOWNERROR	m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:DMDefaultError")

// APP_LOCAL_MODE data
#define	TRANSACTION_HASH	L"1bfa0939-4c7c-4a57-afb9-bf5ba2420516";
#define	TIMESTAMP			L"2022-08-31T18:37:01.51101Z";
#define	ACCOUNT_HASH		L"e2f96167-e7b8-47ab-a962-bc8a12111793";
#define	SESSION_HASH		L"fa1c7b65-c0b4-477c-a41e-af529b2df7a8";
#define	ACCOUNT_NAME		L"customer first name customer last name";
#define	PRICE				22681.32;
#define	EXPIRY				L"2022-08-31T18:52:01.51101Z";
int		DAILY_LIMIT =		5000;
CString	WithdrawalAddress =	L"";

extern CTimeCheck g_TimeCheck;

BIZ_RETURN CTranCmn::P_NH_DM_ChooseFlow(DMFlowSelection *selection)
{
	*selection = DMF_ATM;
	H_NH_DM_SetTransactionMode(DMTXNMODE_ATM);

	CString GetKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(373);
		m_pDevCmn->fnSCR_DisplayScreen(373, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(373, L"373.wav");

			m_pAdaCtrl->fnExp_PlayScreenWave(373);

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
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"373_DebitSelected.wav", TRUE);
				}

				*selection = DMF_ATM;
				H_NH_DM_SetTransactionMode(DMTXNMODE_ATM);

				return RES_OK;
			}
			else if (GetKeyStr == L"CASH")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"373_CashSelected.wav", TRUE);
				}

				*selection = DMF_Sidecar;
				H_NH_DM_SetTransactionMode(DMTXNMODE_SIDECAR);

				return RES_OK;
			}
			else if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "1T", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "1T", L"NG_1", GetKeyStr);

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
	NVDump('F', 'C', "1T", L"NG_2", L"CANCEL");

	return RES_NG;
}

BIZ_RETURN CTranCmn::P_NH_DM_EnterCustomerPhoneNumber(CString &phoneNumber, bool retry)
{
	NHDBG((L"[CTranCmn::P_NH_DM_EnterCustomerPhoneNumber]\n"));
	NVDump('O', 'C', "1U", L"Start", L"Phone");

	CString GetKeyStr, HelpMsg, AdaKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (retry)
	{
		HelpMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_370000);
	}

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(370);
		m_pDevCmn->fnSCR_DisplayString(1, HelpMsg);
		m_pDevCmn->fnSCR_DisplayScreen(370, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();

			if (!HelpMsg.IsEmpty())
			{
				m_pAdaCtrl->fnExp_AddWaveFile(370, HelpMsg);
			}

			m_pAdaCtrl->fnExp_AddWaveFile(370, L"370.wav");

			if (!AdaKeyStr.IsEmpty())
			{
				CString strTemp;
				strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"370_Entry.wav"), AdaKeyStr);
				m_pAdaCtrl->fnExp_AddWaveFile(370, strTemp);
			}

			m_pAdaCtrl->fnExp_PlayScreenWave(370);

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
				NVDump('O', 'C', "1U", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "1U", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_ENTER)
			{
				phoneNumber = AdaKeyStr;
				NVDump('O', 'C', "1U", L"Phone", phoneNumber);

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
					NVDump('O', 'C', "1U", L"Phone", phoneNumber);

					return RES_OK;
				}
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_DM_EnterPin(DMPinMode mode, CString &pin)
{
	NHDBG((L"[CTranCmn::P_NH_DM_EnterPin]\n"));
	NVDump('O', 'C', "1V", L"Pin", L"Enter");

	CString GetKeyStr, HelpMsg, AdaKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (mode == DMPM_RetryPin)
	{
		HelpMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_371000);
	}

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(371);
		m_pDevCmn->fnSCR_DisplayString(1, HelpMsg);
		m_pDevCmn->fnSCR_DisplayScreen(371, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();

			if (!HelpMsg.IsEmpty())
			{
				m_pAdaCtrl->fnExp_AddWaveFile(371, HelpMsg);
			}

			m_pAdaCtrl->fnExp_AddWaveFile(371, L"371.wav");

			if (!AdaKeyStr.IsEmpty())
			{
				CString strTemp;
				strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"371_Entry.wav"), AdaKeyStr);
				m_pAdaCtrl->fnExp_AddWaveFile(371, strTemp);
			}

			m_pAdaCtrl->fnExp_PlayScreenWave(371);

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
					GetKeyStr = L"RESETPIN";
				}
			}

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "1V", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "1V", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_ENTER)
			{
				pin = AdaKeyStr;
				NVDump('O', 'C', "1V", L"PIN", pin);

				return RES_OK;
			}
			else if (GetKeyStr == L"RESETPIN")
			{
				pin = GetKeyStr;
				NVDump('O', 'C', "1V", L"PIN", pin);

				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"371_ResetPin.wav", TRUE);
				}

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
					pin = GetKeyStr;
					NVDump('O', 'C', "1V", L"PIN", pin);

					return RES_OK;
				}
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_DM_CreatePin(CString &pin)
{
	NHDBG((L"[CTranCmn::P_NH_DM_CreatePin]\n"));
	NVDump('O', 'C', "1W", L"Pin", L"Create");

	CString GetKeyStr, AdaKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(383);
		m_pDevCmn->fnSCR_DisplayScreen(383, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(383, L"383.wav");

			if (!AdaKeyStr.IsEmpty())
			{
				CString strTemp;
				strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"383_Entry.wav"), AdaKeyStr);
				m_pAdaCtrl->fnExp_AddWaveFile(383, strTemp);
			}

			m_pAdaCtrl->fnExp_PlayScreenWave(383);

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
				NVDump('O', 'C', "1W", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "1W", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_ENTER)
			{
				pin = AdaKeyStr;
				NVDump('O', 'C', "1W", L"PIN", pin);

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
					pin = GetKeyStr;
					NVDump('O', 'C', "1W", L"PIN", pin);

					return RES_OK;
				}
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_DM_EnterSmsCode(CString &code)
{
	NHDBG((L"[CTranCmn::P_NH_DM_EnterSmsCode]\n"));
	NVDump('O', 'C', "1X", L"Start", L"SmsCode");

	CString GetKeyStr, AdaKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(379);
		m_pDevCmn->fnSCR_DisplayScreen(379, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(379, L"379.wav");

			if (!AdaKeyStr.IsEmpty())
			{
				CString strTemp;
				strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"379_Entry.wav"), AdaKeyStr);
				m_pAdaCtrl->fnExp_AddWaveFile(379, strTemp);
			}

			m_pAdaCtrl->fnExp_PlayScreenWave(379);

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
				NVDump('O', 'C', "1X", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "1X", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_ENTER)
			{
				code = AdaKeyStr;
				NVDump('O', 'C', "1X", L"code", code);

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
					NVDump('O', 'C', "1X", L"code", code);

					return RES_OK;
				}
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_DM_ScanID(CString &rawData, bool retry)
{
	BIZ_RETURN result = RES_USER_TIMEOUT;
	NHDBG((L"[CTranCmn::P_NH_DM_ScanID]\n"));
	NVDump('O', 'C', "1Y", L"Start", L"ScanID");

	CString bcrData, GetKeyStr, HelpMsg;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (retry)
	{
		HelpMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_381000);
	}

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(381);
		m_pDevCmn->fnSCR_DisplayString(1, HelpMsg);
		m_pDevCmn->fnSCR_DisplayScreen(381, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
	}

	m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_BCR, FLICKER_ON);

	if (m_pDevCmn->fnBCR_SendRawData(FALSE))
	{
		g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

		while (g_TimeCheck.IsElapsedTimes() == FALSE)
		{
			if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

				m_pAdaCtrl->fnExp_StopPlay();

				return RES_NG;
			}

			if (IsAdaTransaction() && replay)
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();

				if (!HelpMsg.IsEmpty())
				{
					m_pAdaCtrl->fnExp_AddWaveFile(381, HelpMsg);
				}

				m_pAdaCtrl->fnExp_AddWaveFile(381, L"381.wav");

				m_pAdaCtrl->fnExp_PlayScreenWave(381);

				replay = FALSE;
			}

			if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_BCR))
			{
				// Check device with reduced time
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BCR, MIN_DEVRSP_TIME);
				m_pDevCmn->fnAPL_CheckDevice();
				CString errorCode = m_pDevCmn->fstrBCR_GetErrorCode();
				bcrData = m_pDevCmn->fstrBCR_GetBarcodeData();

				if (errorCode == L"0000000" && !bcrData.IsEmpty())
				{
					rawData = bcrData;
					result = RES_OK;

					if (IsAdaTransaction())
					{
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"381_Scan.wav", FALSE);
					}

					break;
				}
				else
				{
					NHDEBUG(DBG_INFO, (_T("Failed to scan id. Device error %s\n"), errorCode));
					NVDump('F', 'C', "1Y", L"Failed to scan id", L"ScanId");

					// 9792001 -> Timeover error
					if (errorCode == L"9792001")
					{
						m_pDevCmn->fnBCR_ClearErrorCode();
					}

					result = RES_USER_TIMEOUT;
					break;
				}
			}
			else if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_BCR, EVENT_OUT))
			{
				NHDEBUG(DBG_INFO, (_T("Failed to scan wallet - fnAPL_GetDeviceEvent(DEV_BCR, EVENT_OUT)\n")));
				NVDump('F', 'C', "1Y", L"Failed to scan id", L"ScanId");

				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BCR, MIN_DEVRSP_TIME);
				m_pDevCmn->fnAPL_CheckDevice();

				result = RES_NG;
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
					result = RES_USER_EXIT;
					break;
				}
				else if (GetKeyStr == S_TIMEOVER)
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					result = RES_USER_TIMEOUT;
					break;
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

		if (result != RES_OK)
		{
			m_pDevCmn->fnBCR_CancelAccept();
		}
	}
	else
	{
		// DMTODO: Specific Error for BCR
		NVDump('F', 'C', "1Y", L"Scan BCR Error", L"ScanId");
		result = RES_NG;
	}

#ifndef UNDER_CE
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();

			if (!HelpMsg.IsEmpty())
			{
				m_pAdaCtrl->fnExp_AddWaveFile(381, HelpMsg);
			}

			m_pAdaCtrl->fnExp_AddWaveFile(381, L"381.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(381, L"381_Win32.wav");

			m_pAdaCtrl->fnExp_PlayScreenWave(380);

			replay = FALSE;
		}

		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
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
				else if (GetKeyStr == L"1")
				{
					GetKeyStr = L"ENTER";
				}
			}

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NHDEBUG(DBG_INFO, (_T("CANCEL OR EXIT BUTTON PRESSED\n")));
				result = RES_USER_EXIT;
				break;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				result = RES_USER_TIMEOUT;
				break;
			}
			else if (GetKeyStr == S_ENTER)
			{
				rawData = L"@\n\u001e\rANSI 636004080002DL00410266ZN03070017DLDAQ123456789123\nDCSDOE\nDDEN\nDACJANE\nDDFN\nDAD\nDDGN\nDCAC\nDCBNONE\nDCDNONE\nDBD08152015\nDBB08151987\nDBA08152020\nDBC2\nDAU070 in\nDAYBRO\nDAG1100 NEW BERN AVENUE\nDAIRALEIGH\nDAJNC\nDAK276970001\nDCF0123456789\nDCGUSA\nDAZBRO\nDCLU  \nDCK000012345678NCSVTL01\nDDB10242014\nDDK1\nDDL1\nZNZNADUP\nZNB\nZNC0\n";
				result = RES_OK;

				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"381_EnterSelected.wav", FALSE);
				}

				break;
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
#endif

	m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_BCR, FLICKER_OFF);

	return result;
}

BIZ_RETURN CTranCmn::P_NH_DM_PhotoID()
{
	NHDBG((L"[CTranCmn::P_NH_DM_PhotoID]\n"));
	NVDump('O', 'C', "1Z", L"Start", L"PhotoID");

	CString GetKeyStr;

	// Show screen
	m_pDevCmn->fnSCR_DisplayPrevSet(382);
	m_pDevCmn->fnSCR_DisplayScreen(382, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "1Z", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "1Z", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_DM_PleaseWait()
{
	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(374);
		m_pDevCmn->fnSCR_DisplayScreen(374);
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_DM_SendTOS()
{
	if (!m_DMService->SendTOS(_dmTransactionState.AccountId))
	{
		return RES_NG;
	}

	if (IsAdaTransaction())
	{
		m_pAdaCtrl->fnExp_ResetAndAddPlay(L"TOSSent.wav", TRUE);
	}

	P_NH_DM_Info(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122600));

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_DM_SelectCoin(DMProducts &products)
{
	NHDBG((L"[CTranCmn::P_NH_DM_SelectCoin]\n"));
	NVDump('O', 'C', "2G", L"Start", L"Coin");

	CString GetKeyStr;
	int numCoins = 0;
	DMProduct product;
	POSITION pos = NULL;
	BOOL replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	CString dailyLimit, monthlyLimit, perLimit, maxLimit;

	dailyLimit.Format(L"%c%.2f", CURRENCY_SYMBOL, (double)_dmTransactionState.AccountLimits.DailyTransactionLimit);
	monthlyLimit.Format(L"%c%.2f", CURRENCY_SYMBOL, (double)_dmTransactionState.AccountLimits.MonthlyTransactionLimit);
	perLimit.Format(L"%c%.2f", CURRENCY_SYMBOL, (double)_dmTransactionState.AccountLimits.PerTransactionLimit);
	maxLimit.Format(L"%c%.2f", CURRENCY_SYMBOL, (double)H_NH_DM_GetTransactionLimit());

	CString coinStr;
	CStringList coins;

	if (!IsAdaTransaction())
	{
		// Show screen
		m_pDevCmn->fnSCR_DisplayPrevSet(372);

		/*
		 * 4 - Coin Button 1
		 * 5 - Coin Button 2
		 * 6 - Coin Button 3
		 * 7 - Daily Limit
		 * 8 - Monthly Limit
		 * 9 - Transaction Limit
		 * 10 - Max Limit
		 * 11 - Coin 1 Name + ':'
		 * 12 - Coin 2 Name + ':'
		 * 13 - Coin 3 Name + ':'
		 * 14 - Coin 1 conversion rate
		 * 15 - Coin 2 conversion rate
		 * 16 - Coin 3 conversion rate
		 */

		// Limits
		m_pDevCmn->fnSCR_DisplayString(7, dailyLimit);
		m_pDevCmn->fnSCR_DisplayString(8, monthlyLimit);
		m_pDevCmn->fnSCR_DisplayString(9, perLimit);
		m_pDevCmn->fnSCR_DisplayString(10, maxLimit);

		// Coins
		pos = products.GetHeadPosition();

		while (pos != NULL)
		{
			product = products.GetNext(pos);

			m_pDevCmn->fnSCR_DisplayString(4 + numCoins, product.Symbol);
			m_pDevCmn->fnSCR_DisplayFormat(11 + numCoins, L"%s:", product.Symbol);

			double markup = product.Markup * 100.00;
			m_pDevCmn->fnSCR_DisplayFormat(14 + numCoins, L"%.2f%%", markup);

			numCoins++;

			if (numCoins > MAX_COINS)
			{
				// Only supports MAX_COINS coins
				break;
			}
		}

		m_pDevCmn->fnSCR_DisplayScreen(372, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}
	else
	{
		// Coins
		pos = products.GetHeadPosition();

		while (pos != NULL)
		{
			product = products.GetNext(pos);

			double markup = product.Markup * 100.00;

			CString coin;

			for (int i = 0; i < product.Symbol.GetLength(); i++)
			{
				TCHAR currChar = product.Symbol.GetAt(i);
				coin += CString(currChar) + " ";
			}

			CString strTemp;
			strTemp.Format(L"Press %d for %s at %.2f%%. ", 1 + numCoins, coin, markup);

			coinStr += strTemp;
			coins.AddTail(coin);

			numCoins++;

			if (numCoins > MAX_COINS)
			{
				// Only supports MAX_COINS coins
				break;
			}
		}
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();

			CString strTemp;
			strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"372.wav"), dailyLimit, monthlyLimit, perLimit, maxLimit);
			m_pAdaCtrl->fnExp_AddWaveFile(372, strTemp);

			m_pAdaCtrl->fnExp_AddWaveFile(372, coinStr);
			m_pAdaCtrl->fnExp_PlayScreenWave(372);

			replay = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
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
					GetKeyStr = L"F2";
				}
				else if (GetKeyStr == L"2")
				{
					GetKeyStr = L"F4";
				}
				else if (GetKeyStr == L"3")
				{
					GetKeyStr = L"F6";
				}
			}

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "2G", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "2G", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == L"F2" && products.GetCount() > 0)
			{
				if (IsAdaTransaction())
				{
					CString strTemp;
					POSITION coin = coins.FindIndex(0);
					strTemp.Format(L"1. %s", coins.GetAt(coin));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
				}

				pos = products.FindIndex(0);
				_dmTransactionState.SelectedCoin = products.GetAt(pos);

				return RES_OK;
			}
			else if (GetKeyStr == L"F4" && products.GetCount() > 1)
			{
				if (IsAdaTransaction())
				{
					CString strTemp;
					POSITION coin = coins.FindIndex(1);
					strTemp.Format(L"2. %s", coins.GetAt(coin));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
				}

				pos = products.FindIndex(1);
				_dmTransactionState.SelectedCoin = products.GetAt(pos);

				return RES_OK;
			}
			else if (GetKeyStr == L"F6" && products.GetCount() > 2)
			{
				if (IsAdaTransaction())
				{
					CString strTemp;
					POSITION coin = coins.FindIndex(2);
					strTemp.Format(L"3. %s", coins.GetAt(coin));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
				}

				pos = products.FindIndex(2);
				_dmTransactionState.SelectedCoin = products.GetAt(pos);

				return RES_OK;
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

BIZ_RETURN CTranCmn::P_NH_DM_ConfirmRates()
{
	NHDBG((L"[CTranCmn::P_NH_DM_ConfirmRates]\n"));
	NVDump('O', 'C', "2H", L"Start", L"");

	CString GetKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	DMProduct product = _dmTransactionState.SelectedCoin;
	CString selectedCoin = product.Description;

	CString day;
	day.Format(L"%c%.2f", CURRENCY_SYMBOL, (double)_dmTransactionState.AccountLimits.DailyTransactionLimit);

	CString per;
	per.Format(L"%c%.2f", CURRENCY_SYMBOL, (double)H_NH_DM_GetTransactionLimit());

	CString max;
	max.Format(L"%c%.2f", CURRENCY_SYMBOL, (double)_dmTransactionState.AccountLimits.MaximumCashAllowed);

	double buy = Asc2Float(_dmTransactionState.Quote.Buy);
	buy /= 100;

	double markup = product.Markup * 100.00;

	CString exchangeRate, exchangeFee;

	exchangeRate.Format(L"%c%.2f", CURRENCY_SYMBOL, buy);
	exchangeFee.Format(L"%.2f%%", markup);

	if (!IsAdaTransaction())
	{
		// Show screen
		m_pDevCmn->fnSCR_DisplayPrevSet(384);
		m_pDevCmn->fnSCR_DisplayFormat(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_384000), selectedCoin);
		m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_384001));
		m_pDevCmn->fnSCR_DisplayString(3, _dmTransactionState.WalletAddress);
		m_pDevCmn->fnSCR_DisplayFormat(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_384002), selectedCoin);
		m_pDevCmn->fnSCR_DisplayString(5, exchangeRate);
		m_pDevCmn->fnSCR_DisplayFormat(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_384003), selectedCoin);
		m_pDevCmn->fnSCR_DisplayString(7, exchangeFee);
		m_pDevCmn->fnSCR_DisplayFormat(8, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_384004), max, day, per);
		m_pDevCmn->fnSCR_DisplayFormat(9, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_384005), m_pDevCmn->fstrSCR_GetStringFromTextID(L"APTextID:Confirm"));
		m_pDevCmn->fnSCR_DisplayScreen(384, K_60_WAIT, PIN_MENU_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();

			CString strTemp;
			strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"384.wav"), selectedCoin, _dmTransactionState.WalletAddress, selectedCoin, exchangeRate, selectedCoin, exchangeFee, max, day, per);
			m_pAdaCtrl->fnExp_AddWaveFile(384, strTemp);

			m_pAdaCtrl->fnExp_PlayScreenWave(384);

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
					GetKeyStr = L"CONFIRM";
				}
				else if (GetKeyStr == L"3")
				{
					GetKeyStr = L"TOS";
				}
			}

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "2H", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "2H", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == L"TOS")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"384_TOSSelected.wav", TRUE);
					replay = TRUE;
				}

				// Show TOS screen
				P_NH_DM_SendTOS();

				if (!IsAdaTransaction())
				{
					// Show Confirm Rates screen
					m_pDevCmn->fnSCR_DisplayPrevSet(384);
					m_pDevCmn->fnSCR_DisplayFormat(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_384000), selectedCoin);
					m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_384001));
					m_pDevCmn->fnSCR_DisplayString(3, _dmTransactionState.WalletAddress);
					m_pDevCmn->fnSCR_DisplayFormat(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_384002), selectedCoin);
					m_pDevCmn->fnSCR_DisplayFormat(5, L"%c%.2f", CURRENCY_SYMBOL, buy);
					m_pDevCmn->fnSCR_DisplayFormat(6, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_384003), selectedCoin);
					m_pDevCmn->fnSCR_DisplayFormat(7, L"%.2f%%", markup);
					m_pDevCmn->fnSCR_DisplayFormat(8, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_384004), max, day, per);
					m_pDevCmn->fnSCR_DisplayFormat(9, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_384005), m_pDevCmn->fstrSCR_GetStringFromTextID(L"APTextID:Confirm"));
					m_pDevCmn->fnSCR_DisplayScreen(384, K_60_WAIT, PIN_MENU_MODE);
				}

				g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
				continue;
			}
			else if (GetKeyStr == L"CONFIRM")
			{
				NVDump('O', 'C', "2H", L"OK", GetKeyStr);

				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"384_ConfirmSelected.wav", TRUE);
				}

				return RES_OK;
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

BIZ_RETURN CTranCmn::P_NH_DM_EnterTransactionAmount(int *purchaseAmount)
{
	NHDBG((L"[CTranCmn::P_NH_DM_EnterTransactionAmount]\n"));
	NVDump('O', 'C', "2I", L"Start", L"Amount");

	CString GetKeyStr, AdaKeyStr;
	BOOL	replay = TRUE;

	*purchaseAmount = 0;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	CString tempStr = Int2Asc(_dmTransactionState.AccountLimits.MaximumCashAllowed);
	int tempStrLen = tempStr.GetLength();
	CString sMax = Int2Asc(tempStrLen);

	CString dailyLimit, transLimit;

	dailyLimit.Format(L"%c%.2f", CURRENCY_SYMBOL, (double)_dmTransactionState.AccountLimits.DailyTransactionLimit);
	transLimit.Format(L"%c%.2f", CURRENCY_SYMBOL, (double)H_NH_DM_GetTransactionLimit());

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(375);
		m_pDevCmn->fnSCR_DisplayString(1, dailyLimit);
		m_pDevCmn->fnSCR_DisplayString(2, transLimit);
		m_pDevCmn->fnSCR_DisplayString(4, L"DOLLAR");
		m_pDevCmn->fnSCR_DisplayString(5, L"INT_DOLLAR");
		m_pDevCmn->fnSCR_DisplayString(10, L"1"); // min
		m_pDevCmn->fnSCR_DisplayString(11, sMax); // max
		m_pDevCmn->fnSCR_DisplayString(12, L"off");
		m_pDevCmn->fnSCR_DisplayScreen(375, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();

			CString strTemp;
			strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"375.wav"), dailyLimit, transLimit);
			m_pAdaCtrl->fnExp_AddWaveFile(375, strTemp);

			m_pAdaCtrl->fnExp_PlayScreenWave(375);

			replay = FALSE;
		}

		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDBG((_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

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
				NVDump('O', 'C', "2I", L"P_NHNOR", L"NG_44"); // [#2024] NH KSK 2011.02.24

				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump('O', 'C', "2I", L"P_NHNOR", L"NG_45"); // [#2024] NH KSK 2011.02.24

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_ENTER)
			{
				CString sEnterVal(AdaKeyStr);
				sEnterVal.Replace(GetCurrencySymbol(), L"");
				sEnterVal.Trim();

				// Set the amount for the debit transaction
				*purchaseAmount = Asc2Int(sEnterVal);

				int atmAmount = *purchaseAmount * 100.00;
				m_sUserSelection.strMoney = Int2Asc(atmAmount);

				if (*purchaseAmount > _dmTransactionState.AccountLimits.MaximumCashAllowed)
				{
					return RES_USER_EXIT;
				}

				NHDBG((_T("Input Amount to convert integer [%d]\n"), *purchaseAmount));

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
				else if (GetKeyStr.GetLength() > 0)
				{
					CString sEnterVal(GetKeyStr);
					sEnterVal.Replace(GetCurrencySymbol(), L"");
					sEnterVal.Trim();

					// Set the amount for the debit transaction
					*purchaseAmount = Asc2Int(sEnterVal);

					int atmAmount = *purchaseAmount * 100.00;
					m_sUserSelection.strMoney = Int2Asc(atmAmount);

					if (*purchaseAmount > _dmTransactionState.AccountLimits.MaximumCashAllowed)
					{
						return RES_USER_EXIT;
					}

					NHDBG((_T("Input Amount to convert integer [%d]\n"), *purchaseAmount));

					return RES_OK;
				}
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("ABNORMAL PROCESSING\n")));
	NVDump('O', 'C', "2I", L"P_NHNOR", L"NG_48"); // [#2024] NH KSK 2011.02.24

	return RES_USER_EXIT;
}

BIZ_RETURN CTranCmn::P_NH_DM_ConfirmWallet()
{
	NHDBG((L"[CTranCmn::P_NH_DM_ConfirmWallet]\n"));
	NVDump('O', 'C', "2J", L"Start", L"ConfirmWallet");

	CString GetKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(376);
		m_pDevCmn->fnSCR_DisplayString(1, m_DMConfig.DefaultCoin);
		m_pDevCmn->fnSCR_DisplayString(2, _dmTransactionState.WalletAddress);
		m_pDevCmn->fnSCR_DisplayScreen(376, KEYIN_TIME_OUT, PIN_MENU_MODE);
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

			CString strTemp;
			strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"376.wav"), _dmTransactionState.WalletAddress, _dmTransactionState.WalletAddress);
			m_pAdaCtrl->fnExp_AddWaveFile(376, strTemp);

			m_pAdaCtrl->fnExp_PlayScreenWave(376);

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
					GetKeyStr = L"YES";
				}
				else if (GetKeyStr == L"3")
				{
					GetKeyStr = L"NO";
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
			else if (GetKeyStr == L"YES")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"376_YesSelected.wav", TRUE);
				}

				return RES_OK;
			}
			else if (GetKeyStr == L"NO")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"376_NoSelected.wav", TRUE);
				}

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

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_DM_ScanWallet(CString &walletAddress, bool retry)
{
	BIZ_RETURN result = RES_USER_TIMEOUT;
	NHDBG((L"[CTranCmn::P_NH_DM_ScanWallet]\n"));
	NVDump('O', 'C', "2K", L"Start", L"ScanWallet");

	CString bcrData, GetKeyStr, HelpMsg;
	BOOL replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (retry)
	{
		HelpMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_380000);
	}

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(380);
		m_pDevCmn->fnSCR_DisplayString(1, HelpMsg);
		m_pDevCmn->fnSCR_DisplayScreen(380, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}

	m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_BCR, FLICKER_ON);

	if (m_pDevCmn->fnBCR_SendRawData(FALSE))
	{
		g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

		while (g_TimeCheck.IsElapsedTimes() == FALSE)
		{
			if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

				m_pAdaCtrl->fnExp_StopPlay();

				return RES_NG;
			}

			if (IsAdaTransaction() && replay)
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();

				if (!HelpMsg.IsEmpty())
				{
					m_pAdaCtrl->fnExp_AddWaveFile(380, HelpMsg);
				}

				m_pAdaCtrl->fnExp_AddWaveFile(380, L"380.wav");

				m_pAdaCtrl->fnExp_PlayScreenWave(380);

				replay = FALSE;
			}

			if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_BCR))
			{
				// Check device with reduced time
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BCR, MIN_DEVRSP_TIME);
				m_pDevCmn->fnAPL_CheckDevice();

				CString errorCode = m_pDevCmn->fstrBCR_GetErrorCode();
				bcrData = m_pDevCmn->fstrBCR_GetBarcodeData();

				if (errorCode == L"0000000" && !bcrData.IsEmpty())
				{
					walletAddress = bcrData;
					result = RES_OK;

					if (IsAdaTransaction())
					{
						CString strTemp;
						strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"380_Scan.wav"), walletAddress);
						m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, FALSE);
					}

					break;
				}
				else
				{
					NHDEBUG(DBG_INFO, (_T("Failed to scan wallet. Device error %s\n"), errorCode));
					NVDump('F', 'C', "2K", L"Failed to scan wallet", L"ScanWallet");

					// 9792001 -> Timeover error
					if (errorCode == L"9792001")
					{
						m_pDevCmn->fnBCR_ClearErrorCode();
					}

					result = RES_USER_TIMEOUT;
					break;
				}
			}
			else if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_BCR, EVENT_OUT))
			{
				NHDEBUG(DBG_INFO, (_T("Failed to scan wallet - fnAPL_GetDeviceEvent(DEV_BCR, EVENT_OUT)\n")));
				NVDump('F', 'C', "2K", L"Failed to scan wallet", L"ScanWallet");

				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BCR, MIN_DEVRSP_TIME);
				m_pDevCmn->fnAPL_CheckDevice();

				result = RES_NG;
			}
			else if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
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

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					NHDEBUG(DBG_INFO, (_T("CANCEL OR EXIT BUTTON PRESSED\n")));
					result = RES_USER_EXIT;
					break;
				}
				else if (GetKeyStr == S_TIMEOVER)
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					result = RES_USER_TIMEOUT;
					break;
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

		if (result != RES_OK)
		{
			m_pDevCmn->fnBCR_CancelAccept();
		}
	}
	else
	{
		// DMTODO: Specific Error for BCR
		NVDump('F', 'C', "2K", L"Scan BCR Error", L"ScanWallet");
		result = RES_NG;
	}

#ifndef UNDER_CE
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();

			if (!HelpMsg.IsEmpty())
			{
				m_pAdaCtrl->fnExp_AddWaveFile(380, HelpMsg);
			}

			m_pAdaCtrl->fnExp_AddWaveFile(380, L"380.wav");

			CString strTemp;
			strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"380_Win32.wav"), L"35PNqL8dxpY9tG3DBTToxUNBLjE9JiUA35");
			m_pAdaCtrl->fnExp_AddWaveFile(380, strTemp);

			m_pAdaCtrl->fnExp_PlayScreenWave(380);

			replay = FALSE;
		}

		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
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
				else if (GetKeyStr == L"1")
				{
					GetKeyStr = L"ENTER";
				}
			}

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NHDEBUG(DBG_INFO, (_T("CANCEL OR EXIT BUTTON PRESSED\n")));
				result = RES_USER_EXIT;
				break;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				result = RES_USER_TIMEOUT;
				break;
			}
			else if (GetKeyStr == S_ENTER)
			{
				walletAddress = L"35PNqL8dxpY9tG3DBTToxUNBLjE9JiUA35";
				result = RES_OK;

				if (IsAdaTransaction())
				{
					CString strTemp;
					strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"380_EnterSelected.wav"), walletAddress);
					m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, FALSE);
				}

				break;
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
#endif

	m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_BCR, FLICKER_OFF);

	return result;
}

BIZ_RETURN CTranCmn::P_NH_DM_CancelTranscation(CString &responseMessage)
{
	responseMessage = DM_UNKNOWNERROR;

	if (!_dmTransactionState.TransactionId.IsEmpty())
	{
		P_NH_DM_PleaseWait();

		DMCancelTransactionRequest request;
		request.CancelCode = _dmTransactionState.CancelCode;

		DMTransactionResponse response;

#ifdef APP_LOCAL_MODE
		response.Code = 0;
		response.State = 420;
		response.Message = L"Transaction has been canceled.";
		response.Status = L"";
		response.TransactionHash = TRANSACTION_HASH;
		response.TransactionID = L"";
		response.Timestamp = TIMESTAMP;
		response.AccountHash = ACCOUNT_HASH;
		response.SessionHash = SESSION_HASH;
		response.AccountName = ACCOUNT_NAME;
		response.DefaultWallet = L"";
		response.WithdrawalAddress = WithdrawalAddress;
		response.Price = PRICE;
		response.Amount = 0;
		response.Product = _dmTransactionState.SelectedCoin.Symbol;
		response.Expiry = EXPIRY;
#else
		if (!m_DMService->CancelTransaction(_dmTransactionState.TransactionId, request, response))
		{
			NHERROR((L"Cancel transaction failed\r\n"));

			return RES_NG;
		}
#endif

		responseMessage = response.Message;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_DM_Error(CString message)
{
	if (message.IsEmpty())
	{
		if (_dmTransactionState.CancelCode == DMCC_CustomerEnrollmentIssue)
		{
			message = m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:DMEnrollError");
		}
		else
		{
			message = DM_UNKNOWNERROR;
		}
	}

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(377);
		m_pDevCmn->fnSCR_DisplayString(2, message);
		m_pDevCmn->fnSCR_DisplayScreen(377);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

		m_pAdaCtrl->fnExp_StopAndResetWaveFile();
		m_pAdaCtrl->fnExp_AddWaveFile(377, message);

		m_pAdaCtrl->fnExp_PlayScreenWave(377);

		return RES_OK;
	}

	Delay_Msg(5000);

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_DM_ThankYou()
{
	CString message = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_123003);

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(386);
		m_pDevCmn->fnSCR_DisplayString(1, message);
		m_pDevCmn->fnSCR_DisplayScreen(386);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

		m_pAdaCtrl->fnExp_StopAndResetWaveFile();
		m_pAdaCtrl->fnExp_AddWaveFile(386, message);

		m_pAdaCtrl->fnExp_PlayScreenWave(386);

		return RES_OK;
	}

	Delay_Msg(2000);

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_DM_Cancel()
{
	CString message = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122001);

	if (_dmTransactionState.CancelCode == DMCC_CustomerChangedMind)
	{
		return P_NH_DM_Info(message);
	}

	m_pDevCmn->fnSCR_DisplayPrevSet(122);
	m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
	m_pDevCmn->fnSCR_DisplayString(4, message);
	m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
	m_pDevCmn->fnSCR_DisplayScreen(122);

	m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_DM_Info(CString message)
{
	if (message.IsEmpty())
	{
		return RES_NG;
	}

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(122);
		m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_INFO);
		m_pDevCmn->fnSCR_DisplayString(4, message);
		m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
		m_pDevCmn->fnSCR_DisplayScreen(122);

		m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_DM_InsertCard()
{
	NVDump('O', 'C', "2L", L"", L"CARD");
	NHDBG((L"[CTranCmn::P_NH_DM_InsertCard]\n"));

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
		m_pDevCmn->fnSCR_DisplayPrevSet(378);
		SetCardReadScreenValue(L"");
		m_pDevCmn->fnSCR_DisplayScreen(378, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

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
			m_pAdaCtrl->fnExp_AddWaveFile(378, L"Please_Insert.wav");

			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
				m_pAdaCtrl->fnExp_AddWaveFile(378, L"Insertion_Direction_V.wav");
			else
				m_pAdaCtrl->fnExp_AddWaveFile(378, L"Insertion_Direction_H.wav");

			if (P_EMV_CheckTransMode() == RES_NOR_MS_TRANS)
				m_pAdaCtrl->fnExp_AddWaveFile(378, L"Smooth_remove.wav");
			else
				m_pAdaCtrl->fnExp_AddWaveFile(378, L"Donot_remove.wav");
			// End of [#2380]

			m_pAdaCtrl->fnExp_PlayScreenWave(378);

			replay = FALSE;
		}

		if ((DidMediaExist() == FALSE) && (LIB_MainMenuDeviceEvent() == DEV_MCU) && (m_pDevCmn->fnMCU_GetDeviceStatus() == NORMAL))
		{
			TurnOffCardReaderFlicker();

			BIZ_RETURN cardReadResult = VATReadCard(388, 389, false);
			if (cardReadResult != RES_OK)
			{
				NHERROR((L"Card read failed\r\n"));

				CString result;
				result.Format(L"%d", cardReadResult);
				NVDump('F', 'C', "2L", L"NG_0", result);
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

	NVDump('F', 'C', "2L", L"NG_2", GetKeyStr);

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_DM_DebitAccount(CString &errorCode, CString &errorMessage)
{
	P_NH_DM_PleaseWait();

	NVDump('O', 'C', "2M", L"", L"DEBIT");
	NHDBG((L"[CTranCmn::P_NH_DM_DebitAccount]\n"));

	TranCode = TC_DIGITALMINT;
	m_SourceAccount = S_CHECKING;
	m_DestAccount = S_CHECKING;

	// ATM Debit Transaction
	BIZ_RETURN debitHostResult;

	if (debitHostResult = P_NH_NOR_Transaction(FALSE))
	{
		errorCode = m_pDevCmn->fstrAPL_GetErrorCode();
		errorMessage = m_pDevCmn->fstrAPL_GetErrorMessage();

		NHERROR((L"DigitalMint debit TXN failed with error code: %s; message: %s\r\n", errorCode, errorMessage));
		NVDump('F', 'C', "2M", L"NG_3", errorCode);

		// Save the journal error
		switch (debitHostResult)
		{
		case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
		case RES_HOST_RECV_ERR:	m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
		case RES_HOST_DENIED:	m_pDevCmn->m_JNLMgr.Save(TRX_NOT_APPROVED);		break;
		}

		// Display the ATM error to the customer
		CString userMessage;
		userMessage.Format(L"%s\n\n", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122201));
		userMessage += errorMessage;
		LIB_UserPopUpNotice(SCR_ICON_STOP, userMessage, INFO_SCR_TIMEOUT);

		return RES_NG;
	}

	return RES_OK;
}

//
// Private functions
//

CTranCmn::DMGreetProcID CTranCmn::P_NH_DM_Greet(CString phoneNumber, CString &errorMessage)
{
	DMGreeting greet;
	greet.PhoneNumber = phoneNumber;

	P_NH_DM_PleaseWait();

	DMGreetingResponse response;

#ifdef APP_LOCAL_MODE
	DMTRANSACTIONMODE transactionMode = m_DMService->GetTransactionMode();

	if (phoneNumber == L"7777777777")
	{
		response.Code = 406;
		response.Message = L"customer is unable to transact, please have them contact customer support";
		response.AccountHash = L"";
		response.SessionHash = L"";
	}
	else if (phoneNumber == L"8888888888")
	{
		if (transactionMode == DMTXNMODE_ATM)
		{
			response.Code = 228;
			response.Message = L"Please complete mobile enrollment.";
			response.AccountHash = L"";
			response.SessionHash = L"";
		}
		else if (transactionMode == DMTXNMODE_SIDECAR)
		{
			response.Code = 220;
			response.Message = L"Enter the confirmation code text from the customer.";
			response.AccountHash = L"";
			response.SessionHash = L"";
		}
	}
	else if (phoneNumber == L"9999999999")
	{
		if (transactionMode == DMTXNMODE_ATM)
		{
			response.Code = 228;
			response.Message = L"Please complete mobile enrollment.";
			response.AccountHash = L"";
			response.SessionHash = L"";
		}
		else if (transactionMode == DMTXNMODE_SIDECAR)
		{
			response.Code = 222;
			response.Message = L"Please confirm customer's phone number to create a PIN";
			response.AccountHash = L"";
			response.SessionHash = L"";
		}
	}
	else
	{
		response.Code = 224;
		response.Message = L"Please enter your PIN.";
		response.AccountHash = L"";
		response.SessionHash = L"";
	}
#else
	if (!m_DMService->Greet(greet, response))
	{
		NHERROR((L"Greet failed\r\n"));

		return DMGP_Fail;
	}
#endif

	if (response.HasError())
	{
		errorMessage = response.Message;
	}

	switch (response.Code)
	{
	case DMR_OK:
		return DMGP_Complete;
	case DMR_GreetingOK:
		_dmTransactionState.CancelCode = DMCC_CustomerEnrollmentIssue;

#ifndef APP_LOCAL_MODE
		_dmTransactionState.LastResponseCode = static_cast<DMResponseType>(response.Code);
#endif

		return DMGP_ConfirmCode;
	case DMR_ConfirmationCodeRequired:
		_dmTransactionState.CancelCode = DMCC_CustomerEnrollmentIssue;

#ifndef APP_LOCAL_MODE
		_dmTransactionState.LastResponseCode = static_cast<DMResponseType>(response.Code);
#endif

		_dmTransactionState.UsePutForCode = true;

		return DMGP_ConfirmCode;
	case DMR_PINRequired:
		return DMGP_PIN;
	case DMR_WebEnrollmentRequired:
		_dmTransactionState.CancelCode = DMCC_CustomerEnrollmentIssue;
		errorMessage = m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:DMWebEnrollError");

		return DMGP_Fail;
	case DMR_CustomerRestricted:
		return DMGP_Pending;
	default:
		return DMGP_Fail;
	}
}

CTranCmn::DMGreetProcID CTranCmn::P_NH_DM_GreetPin(CString phoneNumber, CString PIN, CString &sessionId, CString &accountId, CString &errorMessage)
{
	DMGreeting greet;
	greet.PhoneNumber = phoneNumber;
	greet.Pin = PIN;

	P_NH_DM_PleaseWait();

	DMGreetingResponse response;

#ifdef APP_LOCAL_MODE
	response.Code = 200;
	response.Message = L"";
	response.AccountHash = ACCOUNT_HASH;
	response.SessionHash = SESSION_HASH;
#else
	if (!m_DMService->Greet(greet, response))
	{
		NHERROR((L"Greet failed\r\n"));

		return DMGP_Fail;
	}
#endif

	if (response.HasError())
	{
		errorMessage.Format(m_pDevCmn->fstrSCR_GetStringFromTextID("APTextID:DMInvalidPin"), response.Message);
	}

	sessionId = response.SessionHash;
	accountId = response.AccountHash;

	switch (response.Code)
	{
	case DMR_OK:
		return DMGP_Complete;
	case DMR_Unauthorize:
		return DMGP_PINRetry;
	default:
		return DMGP_Fail;
	}
}

CTranCmn::DMGreetProcID CTranCmn::P_NH_DM_GreetReconfirm(CString phoneNumber, CString &sessionId, CString &accountId, CString &errorMessage)
{
	DMGreeting greet;
	greet.PhoneNumber = phoneNumber;

	P_NH_DM_PleaseWait();

	DMGreetingResponse response;

#ifdef APP_LOCAL_MODE
	response.Code = 220;
	response.Message = L"Enter the confirmation code text from the customer.";
	response.AccountHash = L"";
	response.SessionHash = L"";
#else
	if (!m_DMService->GreetReconfirm(greet, response))
	{
		NHERROR((L"Greet reconfirm failed\r\n"));

		return DMGP_Fail;
	}
#endif

	if (response.HasError())
	{
		errorMessage = response.Message;
	}

	switch (response.Code)
	{
	case DMR_GreetingOK:
		_dmTransactionState.LastResponseCode = static_cast<DMResponseType>(response.Code);
		_dmTransactionState.UsePutForCode = true;

		return DMGP_ConfirmCode;
	default:
		return DMGP_Fail;
	}
}

CTranCmn::DMGreetProcID CTranCmn::P_NH_DM_GreetResetPin(CString phoneNumber, CString PIN, CString &sessionId, CString &accountId, CString &errorMessage)
{
	DMGreeting greet;
	greet.PhoneNumber = phoneNumber;
	greet.Pin = PIN;

	P_NH_DM_PleaseWait();

	DMGreetingResponse response;

#ifdef APP_LOCAL_MODE
	response.Code = 200;
	response.Message = L"";
	response.AccountHash = ACCOUNT_HASH;
	response.SessionHash = SESSION_HASH;
#else
	if (!m_DMService->GreetReset(greet, response))
	{
		NHERROR((L"Greet reset failed\r\n"));

		return DMGP_Fail;
	}
#endif

	if (response.HasError())
	{
		errorMessage = response.Message;
	}

	sessionId = response.SessionHash;
	accountId = response.AccountHash;

	switch (response.Code)
	{
	case DMR_OK:
		return DMGP_Complete;
	default:
		return DMGP_Fail;
	}
}

CTranCmn::DMGreetProcID CTranCmn::P_NH_DM_GreetNewPin(CString phoneNumber, CString PIN, CString &sessionId, CString &accountId, CString &errorMessage)
{
	DMGreeting greet;
	greet.PhoneNumber = phoneNumber;
	greet.Pin = PIN;

	P_NH_DM_PleaseWait();

	DMGreetingResponse response;

#ifdef APP_LOCAL_MODE
	response.Code = 200;
	response.Message = L"";
	response.AccountHash = ACCOUNT_HASH;
	response.SessionHash = SESSION_HASH;

	// DMTODO: We don't have these but DM is returning them:
	// response.SessionToken = L"e0a184f9-4836-49cb-97a8-8e2d7ef23ba8";
#else
	if (!m_DMService->Greet(greet, response))
	{
		NHERROR((L"Greet failed\r\n"));

		return DMGP_Fail;
	}
#endif

	if (response.HasError())
	{
		errorMessage = response.Message;
	}

	sessionId = response.SessionHash;
	accountId = response.AccountHash;

	switch (response.Code)
	{
	case DMR_OK:
		return DMGP_Complete;
	case DMR_Unauthorize:
		return DMGP_ConfirmCode;
	default:
		return DMGP_Fail;
	}
}

CTranCmn::DMGreetProcID CTranCmn::P_NH_DM_GreetEnroll(DMEnrollmentRequest enroll, CString &sessionId, CString &accountId, CString &errorMessage)
{
	P_NH_DM_PleaseWait();

	DMEnrollmentResponse response;

#ifdef APP_LOCAL_MODE
	response.Code = 200;
	response.Message = L"";
	response.AccountHash = ACCOUNT_HASH;
	response.SessionHash = SESSION_HASH;
#else
	if (!m_DMService->GreetEnroll(enroll, response))
	{
		NHERROR((L"Greet failed\r\n"));

		return DMGP_Fail;
	}
#endif

	if (response.HasError())
	{
		errorMessage = response.Message;
	}

	sessionId = response.SessionHash;
	accountId = response.AccountHash;

	switch (response.Code)
	{
	case DMR_OK:
		return DMGP_Complete;
	case DMR_AccountPendingApproval:
		return DMGP_Pending;
	default:
		return DMGP_Fail;
	}
}

BIZ_RETURN CTranCmn::P_NH_DM_EnrollPending()
{
	NHDBG((L"[CTranCmn::P_NH_DM_EnrollPending]\n"));
	NVDump('O', 'C', "2N", L"Pending", L"Show Screen");

	CString GetKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(391);
		m_pDevCmn->fnSCR_DisplayScreen(391, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(391, L"391.wav");

			m_pAdaCtrl->fnExp_PlayScreenWave(391);

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
					GetKeyStr = L"DONE";
				}
			}

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "2N", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "2N", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == L"DONE")
			{
				NVDump('O', 'C', "2N", L"Pending", GetKeyStr);

				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"391_DoneSelected.wav", TRUE);
				}

				return RES_OK;
			}
			else
			{
				if (IsAdaTransaction())
				{
					replay = TRUE;

					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				}
				else
				{
					NVDump('O', 'C', "2N", L"Pending", GetKeyStr);

					return RES_OK;
				}
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}

CTranCmn::DMGreetProcID CTranCmn::P_NH_DM_GreetCode(CString phoneNumber, CString code, CString &sessionId, CString &accountId, CString &errorMessage)
{
	DMGreeting greet;
	greet.PhoneNumber = phoneNumber;
	greet.ConfirmCode = code;

	P_NH_DM_PleaseWait();

	DMGreetingResponse response;

#ifdef APP_LOCAL_MODE
	if (code == L"8888")
	{
		if (_dmTransactionState.LastResponseCode == DMR_ConfirmationCodeRequired)
		{
			response.Code = 223;
			response.Message = L"Please create a PIN.";
			response.AccountHash = L"";
			response.SessionHash = L"";
		}
		else if (_dmTransactionState.LastResponseCode == DMR_GreetingOK)
		{
			response.Code = 224;
			response.Message = L"Please enter your PIN.";
			response.AccountHash = L"";
			response.SessionHash = L"";
		}
		else
		{
			response.Code = 200;
			response.Message = L"Phone confirmation OK, please enroll new customer.";
			response.AccountHash = L"";
			response.SessionHash = L"";
		}
	}
	else
	{
		response.Code = 401;
		response.Message = L"Invalid confirmation code for phone number.";
		response.AccountHash = L"";
		response.SessionHash = L"";
	}
#else
	if (!m_DMService->GreetCode(greet, response, _dmTransactionState.UsePutForCode))
	{
		NHERROR((L"Greet failed\r\n"));

		return DMGP_Fail;
	}
#endif

	if (response.HasError())
	{
		errorMessage = response.Message;

		return DMGP_Fail;
	}

	sessionId = response.SessionHash;
	accountId = response.AccountHash;

	switch (response.Code)
	{
	// DMTODO: Remove DMR_None, not receiving a response code from a successful Confirm Code route
	case DMR_None:
	case DMR_OK:
		return DMGP_ScanID;
	case DMR_CreatePIN:
		return DMGP_NewPIN;
	case DMR_PINRequired:
		return DMGP_ResetPIN;
	case DMR_CustomerNotApproved:
		_dmTransactionState.CancelCode = DMCC_CustomerEnrollmentIssue;

		return DMGP_Fail;
	default:
		return DMGP_Fail;
	}
}

BIZ_RETURN CTranCmn::P_NH_DM_GetAccountLimits(CString &errorMessage)
{
	P_NH_DM_PleaseWait();

#ifdef APP_LOCAL_MODE
	_dmTransactionState.AccountLimits.Code = 0;
	_dmTransactionState.AccountLimits.Message = L"";
	_dmTransactionState.AccountLimits.Status = L"";
	_dmTransactionState.AccountLimits.NextDayCanPurchase = L"08/31/22 12:16 PM";
	_dmTransactionState.AccountLimits.DailyTransactionLimit = 5000;
	_dmTransactionState.AccountLimits.DailyVolume = 0;
	_dmTransactionState.AccountLimits.MonthlyTransactionLimit = 20000;
	_dmTransactionState.AccountLimits.MonthlyVolume = 38;
	_dmTransactionState.AccountLimits.After14DaysCanPurchase = 19962;
	_dmTransactionState.AccountLimits.PerTransactionLimit = 2500;
	_dmTransactionState.AccountLimits.MaximumCashAllowed = DAILY_LIMIT;
	_dmTransactionState.AccountLimits.RemainingToHitMonthlyLimit = 19962;
	_dmTransactionState.AccountLimits.RemainingToHitDailyLimit = 5000;
	_dmTransactionState.AccountLimits.After7DaysCanPurchase = 19962;
	_dmTransactionState.AccountLimits.NextDayCanPurchaseAmount = 5000;
	_dmTransactionState.AccountLimits.MonthlyLimitExceeded = false;
	_dmTransactionState.AccountLimits.DailyLimitExceeded = false;
#else
	if (!m_DMService->GetAccountLimits(_dmTransactionState.AccountId, _dmTransactionState.AccountLimits))
	{
		return RES_NG;
	}
#endif

	if (_dmTransactionState.AccountLimits.HasError())
	{
		errorMessage = _dmTransactionState.AccountLimits.Message;

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_DM_GetProducts(DMProductsResponse &products, CString &errorMessage)
{
	P_NH_DM_PleaseWait();

#ifdef APP_LOCAL_MODE
	products.Code = 0;
	products.Message = L"";
	products.Status = L"";

	DMProduct p1;
	p1.Hash = L"f104606c-c16b-4e7c-abd3-e0b692585a31";
	p1.Symbol = L"TBTCUSD";
	p1.Live = false;
	p1.Description = L"Testnet Bitcoin/US Dollar";
	p1.Markup = 0.12;
	p1.Buy = true;
	p1.Sell = false;
	products.Products.AddTail(p1);

	DMProduct p2;
	p2.Hash = L"2096ddbe-7627-403a-a33e-94435a0e43ff";
	p2.Symbol = L"LTCUSD";
	p2.Live = true;
	p2.Description = L"Litecoin/US Dollar";
	p2.Markup = 0.12;
	p2.Buy = true;
	p2.Sell = false;
	products.Products.AddTail(p2);

	DMProduct p3;
	p3.Hash = L"7841e260-114e-4b0e-8fec-2344c039e8e1";
	p3.Symbol = L"BTCUSD";
	p3.Live = true;
	p3.Description = L"Bitcoin/US Dollar";
	p3.Markup = 0.12;
	p3.Buy = true;
	p3.Sell = false;
	products.Products.AddTail(p3);
#else
	if (!m_DMService->GetProducts(products))
	{
		return RES_NG;
	}
#endif

	if (products.HasError())
	{
		errorMessage = products.Message;

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_DM_IntiateTransaction(DMInitiateTransactionRequest &request, DMTransactionResponse &transaction, CString &errorMessage)
{
	P_NH_DM_PleaseWait();

#ifdef APP_LOCAL_MODE
	transaction.Code = 0;
	transaction.Message = L"Please scan or confirm the customer's wallet address.";
	transaction.Status = L"";
	transaction.State = 200;
	transaction.TransactionHash = TRANSACTION_HASH;
	transaction.TransactionID = L"";
	transaction.Timestamp = TIMESTAMP;
	transaction.AccountHash = ACCOUNT_HASH;
	transaction.SessionHash = SESSION_HASH;
	transaction.AccountName = ACCOUNT_NAME;
	transaction.DefaultWallet = L"";
	transaction.WithdrawalAddress = WithdrawalAddress;
	transaction.Price = PRICE;
	transaction.Amount = 0;
	transaction.Product = _dmTransactionState.SelectedCoin.Symbol;
	transaction.Expiry = EXPIRY;
#else
	if (!m_DMService->InitiateTransaction(request, transaction))
	{
		return RES_NG;
	}
#endif

	if (transaction.HasError())
	{
		errorMessage = transaction.Message;

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_DM_SetWalletAddress(CString &errorMessage)
{
	P_NH_DM_PleaseWait();

	DMScanWalletRequest request;
	DMTransactionResponse transaction;
	request.WithdrawalAddress = _dmTransactionState.WalletAddress;

#ifdef APP_LOCAL_MODE
	transaction.Code = 0;
	transaction.State = 210;
	transaction.Message = L"Please collect cash.";
	transaction.Status = L"";
	transaction.TransactionHash = TRANSACTION_HASH;
	transaction.TransactionID = L"";
	transaction.Timestamp = TIMESTAMP;
	transaction.AccountHash = ACCOUNT_HASH;
	transaction.SessionHash = SESSION_HASH;
	transaction.AccountName = ACCOUNT_NAME;
	transaction.DefaultWallet = L"";
	transaction.WithdrawalAddress = WithdrawalAddress;
	transaction.Price = PRICE;
	transaction.Amount = 0;
	transaction.Product = _dmTransactionState.SelectedCoin.Symbol;
	transaction.Expiry = EXPIRY;
#else
	if (!m_DMService->SetWalletAddress(_dmTransactionState.TransactionId, request, transaction))
	{
		return RES_NG;
	}
#endif

	if (transaction.HasError())
	{
		errorMessage = transaction.Message;

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_DM_GetQuote(CString &errorMessage)
{
	P_NH_DM_PleaseWait();

#ifdef APP_LOCAL_MODE
	_dmTransactionState.Quote.Code = 0;
	_dmTransactionState.Quote.Message = L"";
	_dmTransactionState.Quote.Status = L"";
	_dmTransactionState.Quote.Buy = L"22673.78";
	_dmTransactionState.Quote.Sell = L"15712.02";
	_dmTransactionState.Quote.Hash = L"96a31a90-9f3e-44aa-8cc8-8f3729a78ed9";
	_dmTransactionState.Quote.MaxQuantity = 0;
	_dmTransactionState.Quote.Expiry = EXPIRY;
	_dmTransactionState.Quote.Bid = L"19838.41";
	_dmTransactionState.Quote.Ask = L"20044.01";
#else
	if (!m_DMService->GetQuote(_dmTransactionState.SelectedCoin.Symbol, _dmTransactionState.AccountId, _dmTransactionState.Quote))
	{
		return RES_NG;
	}
#endif

	if (_dmTransactionState.Quote.HasError())
	{
		errorMessage = _dmTransactionState.Quote.Message;

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_DM_ConfirmDebit(int amount, CString &errorMessage, bool showPleaseWait)
{
	if (showPleaseWait)
	{
		P_NH_DM_PleaseWait();
	}

	DMTransactionResponse response;
	DMCollectPaymentRequest collect;
	collect.Execute = false;
	collect.Receipt = true;
	collect.PaymentCollected.Format(L"%d", amount);

#ifdef APP_LOCAL_MODE
	response.Code = 0;
	response.State = 210;
	response.Message = L"Please collect more cash or complete the transaction.";
	response.Status = L"";
	response.TransactionHash = TRANSACTION_HASH;
	response.TransactionID = L"";
	response.Timestamp = TIMESTAMP;
	response.AccountHash = ACCOUNT_HASH;
	response.SessionHash = SESSION_HASH;
	response.AccountName = ACCOUNT_NAME;
	response.DefaultWallet = L"";
	response.WithdrawalAddress = WithdrawalAddress;
	response.Price = PRICE;
	response.Amount = _dmTransactionState.TransactionAmount;
	response.Product = _dmTransactionState.SelectedCoin.Symbol;
	response.Expiry = EXPIRY;
#else
	if (!m_DMService->CollectPayment(_dmTransactionState.TransactionId, collect, response))
	{
		return RES_NG;
	}
#endif

	if (response.HasError())
	{
		errorMessage = response.Message;

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_DM_CollectCash(int &transactionAmount, CString &errorMessage)
{
	NHDBG((L"[CTranCmn::P_NH_CollectCash]\n"));
	NVDump('O', 'C', "2O", L"Start", L"Cash");

	CString GetKeyStr;
	BIZ_RETURN nRes = RES_NG;
	BOOL replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	int maxCashAllowed = H_NH_DM_GetTransactionLimit();

#ifdef UNDER_CE
	int amountCollected = 0;
	BOOL bShowCancel = true;
#else
	int amountCollected = 1;
	BOOL bShowCancel = false;

	// Send payment to DM
	if (P_NH_DM_ConfirmDebit(amountCollected, errorMessage, false) == RES_OK)
	{
		// Sum the transaction total
		transactionAmount += amountCollected;
	}
	else
	{
		nRes = RES_NG;
	}
#endif

	if (!IsAdaTransaction())
	{
		// Show screen
		m_pDevCmn->fnSCR_DisplayPrevSet(390);
		m_pDevCmn->fnSCR_DisplayFormat(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_390000), _dmTransactionState.SelectedCoin.Description);
		m_pDevCmn->fnSCR_DisplayFormat(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_390001), CURRENCY_SYMBOL, (double)transactionAmount);
		m_pDevCmn->fnSCR_DisplayFormat(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_390002), CURRENCY_SYMBOL, (double)maxCashAllowed);
		m_pDevCmn->fnSCR_DisplayString(4, L"");

		// Buttons
		if (bShowCancel)
		{
			m_pDevCmn->fnSCR_DisplayImage(1, FALSE);	// F2, ENTER
			m_pDevCmn->fnSCR_DisplayImage(2, TRUE);		// F8, CANCEL
		}
		else
		{
			m_pDevCmn->fnSCR_DisplayImage(1, TRUE);		// F2, ENTER
			m_pDevCmn->fnSCR_DisplayImage(2, FALSE);	// F8, CANCEL
		}

		m_pDevCmn->fnSCR_DisplayScreen(390, K_NO_WAIT, PIN_MENU_MODE);
	}

	bool rollbackCashOnEnd = true;
	bool acceptCash = true;

	if (H_NH_DM_StartCashIn())
	{
		// BNA FLICKER ON
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_BNA, FLICKER_ON);

		while (acceptCash)
		{
			if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

				m_pAdaCtrl->fnExp_StopPlay();

				return RES_NG;
			}

			if (IsAdaTransaction() && replay)
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();

				CString strTemp;
				strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"390.wav"), _dmTransactionState.SelectedCoin.Description, CURRENCY_SYMBOL, (double)maxCashAllowed, CURRENCY_SYMBOL, (double)transactionAmount);
				m_pAdaCtrl->fnExp_AddWaveFile(390, strTemp);

				if (bShowCancel)
				{
					m_pAdaCtrl->fnExp_AddWaveFile(390, L"390_Cancel.wav");
				}
				else
				{
					m_pAdaCtrl->fnExp_AddWaveFile(390, L"390_Done.wav");
				}

				m_pAdaCtrl->fnExp_PlayScreenWave(390);

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
				}

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					if (IsAdaTransaction())
					{
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"390_CancelSelected.wav", TRUE);
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
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"390_DoneSelected.wav", TRUE);
					}

					nRes = RES_OK;
					break;
				}
				else if (IsAdaTransaction())
				{
					replay = TRUE;

					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				}
			}
			// Listen for cash acceptor
			else if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_BNA))
			{
				amountCollected = 0;
				NHDEBUG(DBG_CALL, (L"fnAPL_GetDeviceEvent(DEV_BNA): [%d]\n", L" "));

				m_pDevCmn->fnSCR_DisplayString(4, L" ");
				m_pDevCmn->fnSCR_DisplayUpdate(390);

				// Check device event
				if (m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BNA))
				{
					// Get the amount collected
					amountCollected = m_pDevCmn->fnBNA_GetLastCashInValue();

					// If amount is less than zero
					// could be item refused or timeout
					if (amountCollected > 0)
					{
						// Send payment to DM
						if (P_NH_DM_ConfirmDebit(amountCollected, errorMessage, false) == RES_OK)
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

							m_pDevCmn->fnSCR_DisplayUpdate(390);

							// Store the cash
							if (H_NH_DM_StoreCash())
							{
								// Restart the accept process
								if (!H_NH_DM_StartCashIn())
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
							// Denied by DM, return the cash
							H_NH_DM_ReturnCash();

							// Customer may have exceeded limits with bill. If they still have
							// bandwidth, they may insert additional smaller bills.
							if (transactionAmount < H_NH_DM_GetTransactionLimit())
							{
								m_pDevCmn->fnSCR_DisplayFormat(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_390003), CURRENCY_SYMBOL, (double)(amountCollected + transactionAmount));
								m_pDevCmn->fnSCR_DisplayUpdate(390);

								if (!H_NH_DM_StartCashIn())
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
							if (!H_NH_DM_StartAcceptCash())
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

						if (H_NH_DM_StartAcceptCash())
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

			if (!this->m_DMService->WriteErrorJournalEntry(
					&m_pDevCmn->m_JNLMgr,
					_dmTransactionState.SessionId,
					_dmTransactionState.TransactionId,
					_dmTransactionState.AccountId,
					_dmTransactionState.SelectedCoin.Symbol,
					amount,
					errorCode))
			{
				NHERROR((L"Failed to write DigitalMint journal entry\r\n"));
				NVDump('F', 'C', "2O", L"NG_0", L"");
			}
		}

		H_NH_DM_ReturnCash(); // to cancel StartCashIn
	}

	if (transactionAmount > 0)
	{
		nRes = RES_OK;
	}

	return nRes;
}

BIZ_RETURN CTranCmn::P_NH_DM_TransactionSummary()
{
	NHDBG((L"[CTranCmn::P_NH_DM_TransactionSummary]\n"));
	NVDump('O', 'C', "2P", L"Start", L"");

	CString GetKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	DMProduct product = _dmTransactionState.SelectedCoin;
	CString selectedCoin = product.Description;

	CString cashStr;
	DMTRANSACTIONMODE transactionMode = m_DMService->GetTransactionMode();

	if (transactionMode == DMTXNMODE_SIDECAR)
		cashStr = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_385000);
	else
		cashStr = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_385001);

	double amount = _dmTransactionState.TransactionAmount;

	CString cashCollected;
	cashCollected.Format(L"%c%.2f", CURRENCY_SYMBOL, amount);

	double markup = product.Markup * 100.00;

	CString exchangeFee;
	exchangeFee.Format(L"%.2f%%", markup);

	double buy = Asc2Float(_dmTransactionState.Quote.Buy);
	buy /= 100;

	double fee = amount * product.Markup;
	double applied = amount - fee;
	double purchased = applied / buy;

	CString appliedStr;
	appliedStr.Format(L"%c%.2f", CURRENCY_SYMBOL, applied);

	CString purchasedStr;
	purchasedStr.Format(L"%f", purchased);

	if (!IsAdaTransaction())
	{
		// Show screen
		m_pDevCmn->fnSCR_DisplayPrevSet(385);
		m_pDevCmn->fnSCR_DisplayFormat(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_385002), selectedCoin);
		m_pDevCmn->fnSCR_DisplayFormat(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_385003), cashStr);
		m_pDevCmn->fnSCR_DisplayString(3, cashCollected);
		m_pDevCmn->fnSCR_DisplayFormat(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_385004), selectedCoin);
		m_pDevCmn->fnSCR_DisplayString(5, exchangeFee);
		m_pDevCmn->fnSCR_DisplayString(6, appliedStr);
		m_pDevCmn->fnSCR_DisplayFormat(7, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_385005), selectedCoin);
		m_pDevCmn->fnSCR_DisplayString(8, purchasedStr);
		m_pDevCmn->fnSCR_DisplayScreen(385, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (IsAdaTransaction() && m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "1T", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();

			CString strTemp;
			strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"385.wav"), selectedCoin, cashStr, cashCollected, selectedCoin, exchangeFee, appliedStr, selectedCoin, purchasedStr);
			m_pAdaCtrl->fnExp_AddWaveFile(385, strTemp);

			m_pAdaCtrl->fnExp_PlayScreenWave(385);

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
					GetKeyStr = L"CONFIRM";
				}
			}

			if (IsAdaTransaction())
			{
				if (GetKeyStr == L"CONFIRM")
				{
					if (IsAdaTransaction())
					{
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"385_ConfirmSelected.wav", TRUE);
					}

					return RES_OK;
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
				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_DM_CompleteTransaction(CString &errorMessage)
{
	P_NH_DM_PleaseWait();

	DMCompleteTransactionRequest request;
	request.Execute = true;
	request.Receipt = true;

	DMTransactionResponse response;

	if (!this->m_DMService->WriteJournalEntry(
			&m_pDevCmn->m_JNLMgr,
			_dmTransactionState.SessionId,
			_dmTransactionState.TransactionId,
			_dmTransactionState.AccountId,
			_dmTransactionState.SelectedCoin.Symbol,
			_dmTransactionState.TransactionAmount))
	{
		NHERROR((L"Failed to write DigitalMint journal entry\r\n"));
		NVDump('F', 'C', "2Q", L"NG_0", L"");
	}

#ifdef APP_LOCAL_MODE
	response.Code = 0;
	response.State = 300;
	response.Message = L"Transaction completed.";
	response.Status = L"";
	response.TransactionHash = TRANSACTION_HASH;
	response.TransactionID = L"";
	response.Timestamp = TIMESTAMP;
	response.AccountHash = ACCOUNT_HASH;
	response.SessionHash = SESSION_HASH;
	response.AccountName = ACCOUNT_NAME;
	response.DefaultWallet = L"";
	response.WithdrawalAddress = WithdrawalAddress;
	response.Price = PRICE;
	response.Amount = _dmTransactionState.TransactionAmount;
	response.Product = _dmTransactionState.SelectedCoin.Symbol;
	response.Expiry = EXPIRY;

	DAILY_LIMIT -= _dmTransactionState.TransactionAmount;

	if (DAILY_LIMIT <= 0)
	{
		DAILY_LIMIT = 5000;
	}
#else
	if (!m_DMService->CompleteTransaction(_dmTransactionState.TransactionId, request, response))
	{
		return RES_NG;
	}
#endif

	if (response.HasError())
	{
		_dmTransactionState.CancelCode = DMCC_Other;
		errorMessage = response.Message;

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::H_NH_DM_SetTransactionMode(DMTRANSACTIONMODE transactionMode)
{
	m_DMService->SetTransactionMode(transactionMode);

#ifdef APP_LOCAL_MODE
	if (transactionMode == DMTXNMODE_ATM)
	{
		WithdrawalAddress = L"35PNqL8dxpY9tG3DBTToxUNBLjE9JiUA35";
	}
#endif

	return RES_OK;
}

BIZ_RETURN CTranCmn::H_NH_DM_ParseID(DMEnrollmentRequest &enroll, CString phoneNumber, CString code, CString rawData)
{
	int pos = 0;
	CString field, data;
	BIZ_RETURN nRes = RES_OK;

	data = rawData;
	enroll.CountryCode = L"US";
	enroll.DocumentScanData = rawData;
	enroll.PhoneNumber = phoneNumber;
	enroll.ConfirmCode = code;

	if (!data.IsEmpty() && data.GetLength() > 4)
	{
		// Remove header special fiels (@,l/f,r/s,c/r)
		data = data.Mid(3);

		// Tokenize on \r to separate the header correctly
		CString token = data.Tokenize(L"\r\n", pos);

		while (!token.IsEmpty())
		{
			if (token.GetLength() > 3)
			{
				/*
					Look at the header to fields: (Special characters are lost in the tokenization)
					Header Sample: ANSI 636004080002DL00410266ZN03070017DLDAQ123456789123
						 |	Header	  | |Sub hdr| |Data elem |
					ANSI 636004080002DL 00410266  ZN03070017DL DAQ123456789123
				*/

				field = token.Mid(0, 3);

				NHDEBUG(DBG_CALL, (L"H_NH_DM_ParseID Parse Field: [%s]\n", field));

				if (field.CompareNoCase(L"ANS") == 0) // header
				{
					if (token.GetLength() > 27)
					{
						// Strip off the header and sub header information
						// Before: ANSI 636004080002DL00410266ZN03070017DLDAQ123456789123
						// After: ZN03070017DLDAQ123456789123
						token = token.Mid(27, token.GetLength());

						if (!token.IsEmpty())
						{
							// Determine if we have jurisdiction-specific meta data
							if (token.GetAt(0) == 'Z')
							{
								// Safety check
								if (token.GetLength() > 12)
								{
									// Remove jurisdiction-specific meta data
									// Before: ZN03070017DLDAQ123456789123
									// After: DAQ123456789123
									token = token.Mid(12, token.GetLength());
								}
							}

							// If we don't have jurisdiction-specific meta data
							// then we will have to assume the data looks like the
							// other fields (e.g. DAQ123456789123)

							if (token.GetLength() > 3)
							{
								// Get the field at the end of the header
								field = token.Mid(0, 3);
							}
						}
					}
				}

				if (field.CompareNoCase(L"DAC") == 0)
				{
					enroll.FirstName = token.Mid(3);
					NHDEBUG(DBG_CALL, (L"H_NH_DM_ParseID Parse First Name: [%s]\n", enroll.FirstName));
				}
				else if (field.CompareNoCase(L"DCS") == 0)
				{
					enroll.LastName = token.Mid(3);
					NHDEBUG(DBG_CALL, (L"H_NH_DM_ParseID Parsed Last Name: [%s]\n", enroll.LastName));
				}
				else if (field.CompareNoCase(L"DAG") == 0)
				{
					enroll.Street = token.Mid(3);
					NHDEBUG(DBG_CALL, (L"H_NH_DM_ParseID Parsed Street: [%s]\n", enroll.Street));
				}
				else if (field.CompareNoCase(L"DAI") == 0)
				{
					enroll.City = token.Mid(3);
					NHDEBUG(DBG_CALL, (L"H_NH_DM_ParseID Parsed City: [%s]\n", enroll.City));
				}
				else if (field.CompareNoCase(L"DAJ") == 0)
				{
					enroll.StateCode = token.Mid(3);
					NHDEBUG(DBG_CALL, (L"H_NH_DM_ParseID Parsed State: [%s]\n", enroll.StateCode));
				}
				else if (field.CompareNoCase(L"DAQ") == 0)
				{
					enroll.DocumentSerial = token.Mid(3);
					NHDEBUG(DBG_CALL, (L"H_NH_DM_ParseID Parsed DocumentSerial: [%s]\n", enroll.DocumentSerial));
				}
				else if (field.CompareNoCase(L"DBB") == 0)
				{
					CString temp = token.Mid(3);

					if (temp.GetLength() == 8)
					{
						enroll.DOB.Format(L"%s/%s/%s", temp.Mid(0, 2), temp.Mid(2, 2), temp.Mid(4, 4));
						NHDEBUG(DBG_CALL, (L"H_NH_DM_ParseID Parsed DOB: [%s]\n", enroll.DOB));
					}
				}
				else if (field.CompareNoCase(L"DBC") == 0)
				{
					CString gender = token.Mid(3);

					if (gender.CompareNoCase(L"1") == 0)
					{
						gender = L"m";
					}
					else if (gender.CompareNoCase(L"2") == 0)
					{
						gender = L"f";
					}
					else if (gender.CompareNoCase(L"9"))
					{
						gender = L"o";
					}

					enroll.Gender = gender;
					NHDEBUG(DBG_CALL, (L"H_NH_DM_ParseID Parsed Gender: [%s]\n", enroll.Gender));
				}
				else if (field.CompareNoCase(L"DBD") == 0)
				{
					CString temp = token.Mid(3);

					if (temp.GetLength() == 8)
					{
						enroll.DocumentIssue.Format(L"%s/%s/%s", temp.Mid(0, 2), temp.Mid(2, 2), temp.Mid(4, 4));
						NHDEBUG(DBG_CALL, (L"H_NH_DM_ParseID Parsed DocumentIssue: [%s]\n", enroll.DocumentIssue));
					}
				}
				else if (field.CompareNoCase(L"DBA") == 0)
				{
					CString temp = token.Mid(3);

					if (temp.GetLength() == 8)
					{
						enroll.DocumentExpiry.Format(L"%s/%s/%s", temp.Mid(0, 2), temp.Mid(2, 2), temp.Mid(4, 4));
						NHDEBUG(DBG_CALL, (L"H_NH_DM_ParseID Parsed DocumentExpiry: [%s]\n", enroll.DocumentExpiry));
					}
				}
			}

			token = rawData.Tokenize(L"\n", pos);
		}
	}
	else
	{
		nRes = RES_NG;
	}

	if (nRes == RES_OK && enroll.DOB.IsEmpty())
	{
		NHERROR((L"Failed to parse DOB. Did we scan the correct barcode?\r\n"));
		nRes = RES_NG;
	}

	return nRes;
}

BIZ_RETURN CTranCmn::P_NH_DM_ReturnCard()
{
#if UNDER_CE
	// Remove the EMV card, if present
	if (m_pDevCmn->fnMCU_IsEmvTransaction())
	{
		P_EMV_RemoveCard(L"", FALSE, FALSE, 389);
	}
#endif

	return RES_OK;
}

CTime CTranCmn::H_NH_DM_ParseDate(CString dateTime)
{
	// DigitalMint Format:
	// 2022-08-23T16:43:13.899612Z
	int tIndex = dateTime.Find(L"T");
	int mIndex = dateTime.Find(L".", tIndex);
	int zIndex = dateTime.Find(L"Z", mIndex);

	CString date = dateTime.Mid(0, tIndex);

	tIndex++;
	int tIndexEnd = mIndex - tIndex;
	CString time = dateTime.Mid(tIndex, tIndexEnd);

	mIndex++;
	int mIndexEnd = zIndex - mIndex;
	CString milliseconds = dateTime.Mid(mIndex, mIndexEnd);

	date.Replace('-', '/');

	// Acceptable Formats:
	// "25 January 1996"
	// "8:30:00"
	// "20:30:00"
	// "January 25, 1996 8:30:00"
	// "8:30:00 Jan. 25, 1996"
	// "1/25/1996 8:30:00" // always specify the full year, even in a 'short date' format
	CString COleDateTimeFormat;
	COleDateTimeFormat.Format(L"%s %s", date, time);

	COleDateTime dt;
	dt.ParseDateTime(COleDateTimeFormat);

	SYSTEMTIME st;

	CTime t;

	if (dt.GetAsSystemTime(st))
	{
		t = CTime(st);
	}

	return t;
}

CString CTranCmn::H_NH_DM_ParseDate(CString dateTime, CString format)
{
	CTime t = H_NH_DM_ParseDate(dateTime);
	return t.Format(format);
}

bool CTranCmn::H_NH_DM_StartCashIn()
{
	bool bRes = false;

	m_pDevCmn->fnBNA_StartCashIn();

	// Check the result of cash in
	if (m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BNA))
	{
		// Start accepting cash
		bRes = H_NH_DM_StartAcceptCash();
	}
	else
	{
		NHDBG((L"Failed to start cash in \n"));
		int deviceStatus = m_pDevCmn->fnBNA_GetDeviceStatus();
		NHDBG((L"H_NH_DM_StartAcceptCash - BNA Status: [%d]\n", deviceStatus));
		CString positionstatus = m_pDevCmn->fstrBNA_GetPositionStatus();
		NHDBG((L"H_NH_DM_StartAcceptCash - BNA Position Status: [%d]\n", positionstatus));
		CString errorCode = m_pDevCmn->fstrBNA_GetErrorCode();
		NHDBG((L"H_NH_DM_StartAcceptCash - BNA error code: [%s]\n", errorCode));
	}

	return bRes;
}

bool CTranCmn::H_NH_DM_StoreCash()
{
	bool bRes = false;

	m_pDevCmn->fnBNA_StoreCash();
	if (m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BNA))
	{
		bRes = true;
		// DMTODO: Log & Journal
	}
	else
	{
		// DMTODO: Log & Journal
	}

	return bRes;
}

bool CTranCmn::H_NH_DM_ReturnCash()
{
	bool bRes = false;

	m_pDevCmn->fnBNA_RollbackCash(K_5_WAIT * 1000);
	if (m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BNA))
	{
		bRes = true;
		// DMTODO: Log & Journal
	}
	else
	{
		// DMTODO: Log & Journal
	}

	return bRes;
}

bool CTranCmn::H_NH_DM_StartAcceptCash()
{
	bool bRes = false;

	int result = 0;

// [RWC6-676] Start SKKim 2024.05.29 Accept In Timeout은 화면 Timeout으로 대체 (화면 Timeout이 ADA_SCREEN_TIMEOUT이므로 Cash In은 24시간 처리)
#if (APP_TANGOPAY)
	result = m_pDevCmn->fnBNA_AcceptCash((ADA_VOICE_MAX_TIMEOUT * 1000), KEYIN_TIME_OUT * 1000);
#else
	result = m_pDevCmn->fnBNA_AcceptCash(KEYIN_TIME_OUT * 1000, KEYIN_TIME_OUT * 1000);
#endif
// [RWC6-676] End SKKim 2024.05.29

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

int CTranCmn::H_NH_DM_GetTransactionLimit()
{
	int maxAllowed = 0;

	if (!_dmTransactionState.AccountLimits.MonthlyLimitExceeded && !_dmTransactionState.AccountLimits.DailyLimitExceeded)
	{
		maxAllowed = min(_dmTransactionState.AccountLimits.MaximumCashAllowed, _dmTransactionState.AccountLimits.PerTransactionLimit);
	}

	return maxAllowed;
}

#endif