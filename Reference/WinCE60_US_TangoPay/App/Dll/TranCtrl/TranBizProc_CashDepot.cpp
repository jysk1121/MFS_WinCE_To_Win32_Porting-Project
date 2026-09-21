#include "stdafx.h"
#include ".\Scr\ScrCtrl.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"

#include ".\Common\NHDbgApi.h"
#include ".\Tran\CashDepotTypes.h"

#if (APP_CUSTOM_CASHDEPOT)

extern CTimeCheck g_TimeCheck;

//
// Private Functions
//

//
// Standard Flows
//

BIZ_RETURN CTranCmn::P_NH_CD_GetConfigurations()
{
	NHDBG((L"[CTranCmn::P_NH_CD_GetConfigurations]\n"));
	NVDump('O', 'C', "-1", L"Start", L"Configs");

	BIZ_RETURN nRes;
	CDConfigurations configs;
	CDConfigurationsResponse response;

	// Terminal Id
	CString terminalId = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	terminalId.Trim();

	_cdTransactionState.TerminalId = terminalId;

	CString strTmp = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2);
	configs.Bin = strTmp.Left(12);
	configs.Timestamp = m_CDService->GetTimestamp();

	P_NH_CD_PleaseWait();

	if (m_CDService->GetConfigurations(configs, _cdTransactionState.TerminalId, response))
	{
		NHDBG((_T("GetConfigurations Success")));

		nRes = RES_OK;
	}
	else
	{
		NHDBG((_T("GetConfigurations Error")));

		nRes = RES_NG;
	}

	_cdTransactionState.Configurations = response;

	if (_cdTransactionState.Configurations.MaxWithdrawalAmount)
		MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT, Int2Asc(_cdTransactionState.Configurations.MaxWithdrawalAmount * 100));

	return nRes;
}

BIZ_RETURN CTranCmn::P_NH_CD_RemainingBalanceInquiry()
{
	NHDBG((L"[CTranCmn::P_NH_CD_RemainingBalanceInquiry]\n"));
	NVDump('O', 'C', "-1", L"Start", L"RBI");

	CString	GetKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(399);

		// ICON
		m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_INFO);

		// Title
		m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_399001));

		// Sub Title
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_399002));

		// Disclaimer
		m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_399003));

		m_pDevCmn->fnSCR_DisplayScreen(399, KEYIN_TIME_OUT, PIN_MENU_MODE);
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

			CString strTemp;
			strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"399.wav"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_399002), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_399003));
			m_pAdaCtrl->fnExp_AddWaveFile(399, strTemp);

			m_pAdaCtrl->fnExp_PlayScreenWave(399);

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
				NVDump('O', 'C', "-1", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "-1", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == L"YES")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"399_YesSelected.wav", TRUE);
				}

				_cdTransactionState.RbiSelected = true;

				return RES_OK;
			}
			else if (GetKeyStr == L"NO")
			{
				if (IsAdaTransaction())
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"399_NoSelected.wav", TRUE);
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

BIZ_RETURN CTranCmn::P_NH_CD_PleaseWait()
{
	m_pDevCmn->fnSCR_DisplayPrevSet(116);

	// Show Title
	m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116001));

	// Under Guide (Please Wait)
	m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116002));

	m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
	m_pDevCmn->fnSCR_DisplayImage(2, FALSE);
	m_pDevCmn->fnSCR_DisplayImage(3, FALSE);

	m_pDevCmn->fnSCR_DisplayScreen(116);

	return RES_OK;
}

//
/// Cryptocurrency Flows
//

#define MAX_PROVIDERS			3
#define MAX_CRYPTO_CURRENCIES	6
#define CD_UNKNOWNERROR			m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:CDDefaultError")

CString CTranCmn::GenerateUUIDv4()
{
	const int pLen = 16;

	CString uuid;
	CString parts[pLen] = {"a", "b", "c", "d", "e", "f", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};

	int min = 0, max = 5;

	// Providing a seed value
	srand(GetTickCount());

	for (int i = min; i < max; i++)
	{
		int lst = max - 1;
		int len = i == min ? 8 : i == lst ? 12 : 4;

		for (int f = 0; f < len; f++)
		{
			// Get a random number
			int random = rand() % pLen;
			uuid += parts[random];
		}

		if (i < lst)
			uuid += "-";
	}

	return uuid;
}

BIZ_RETURN CTranCmn::P_NH_CD_ChooseProvider(CDProviders &providers)
{
	CString		GetKeyStr;
	int			numProviders = 0;
	CDProvider	provider;
	POSITION	pos = NULL;
	BOOL		replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	CString providerStr;
	CStringList providerCodes;

	if (!IsAdaTransaction())
	{
		// Show screen
		m_pDevCmn->fnSCR_DisplayPrevSet(400);

		/*
		 * 1 - Provider 1
		 * 2 - Provider 2
		 * 3 - Provider 3
		 */

		// Providers
		pos = providers.GetHeadPosition();

		while (pos != NULL)
		{
			provider = providers.GetNext(pos);

			m_pDevCmn->fnSCR_DisplayString(1 + numProviders, provider.Provider);

			numProviders++;

			if (numProviders > MAX_PROVIDERS)
			{
				// Only supports MAX_PROVIDERS providers
				break;
			}
		}

		m_pDevCmn->fnSCR_DisplayScreen(400, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}
	else
	{
		// Providers
		pos = providers.GetHeadPosition();

		while (pos != NULL)
		{
			provider = providers.GetNext(pos);

			CString strTemp;
			strTemp.Format(L"Press %d for %s. ", 1 + numProviders, provider.Provider);

			providerStr += strTemp;
			providerCodes.AddTail(provider.ProviderCode);

			numProviders++;

			if (numProviders > MAX_PROVIDERS)
			{
				// Only supports MAX_PROVIDERS providers
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
			NVDump('O', 'C', "-1", L"P_NHADA", L"NG0");

			m_pAdaCtrl->fnExp_StopPlay();

			return RES_NG;
		}

		if (IsAdaTransaction() && replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(400, providerStr);

			m_pAdaCtrl->fnExp_PlayScreenWave(400);

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
				NVDump('O', 'C', "-1", L"NG_0", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));
				NVDump('F', 'C', "-1", L"NG_1", GetKeyStr);

				return RES_NG;
			}
			else if (GetKeyStr == L"F2" && providers.GetCount() > 0)
			{
				if (IsAdaTransaction())
				{
					CString strTemp;
					POSITION provider = providerCodes.FindIndex(0);
					strTemp.Format(L"1. %s", providerCodes.GetAt(provider));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
				}

				pos = providers.FindIndex(0);
				_cdCryptoState.SelectedProvider = providers.GetAt(pos);

				return RES_OK;
			}
			else if (GetKeyStr == L"F4" && providers.GetCount() > 1)
			{
				if (IsAdaTransaction())
				{
					CString strTemp;
					POSITION provider = providerCodes.FindIndex(1);
					strTemp.Format(L"2. %s", providerCodes.GetAt(provider));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
				}

				pos = providers.FindIndex(1);
				_cdCryptoState.SelectedProvider = providers.GetAt(pos);

				return RES_OK;
			}
			else if (GetKeyStr == L"F6" && providers.GetCount() > 2)
			{
				if (IsAdaTransaction())
				{
					CString strTemp;
					POSITION provider = providerCodes.FindIndex(2);
					strTemp.Format(L"3. %s", providerCodes.GetAt(provider));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
				}

				pos = providers.FindIndex(2);
				_cdCryptoState.SelectedProvider = providers.GetAt(pos);

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
	NVDump('F', 'C', "-1", L"NG_2", L"CANCEL");

	return RES_NG;
}

BIZ_RETURN CTranCmn::P_NH_CD_EnterCustomerPhoneNumber(CString &phoneNumber, bool retry)
{
	NHDBG((L"[CTranCmn::P_NH_CD_EnterCustomerPhoneNumber]\n"));
	NVDump('O', 'C', "-1", L"Start", L"Phone");

	CString	GetKeyStr, HelpMsg, AdaKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (retry)
	{
		HelpMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_402000);
	}

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(402);
		m_pDevCmn->fnSCR_DisplayString(1, HelpMsg);
		m_pDevCmn->fnSCR_DisplayScreen(402, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
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

			if (!HelpMsg.IsEmpty())
			{
				m_pAdaCtrl->fnExp_AddWaveFile(402, HelpMsg);
			}

			m_pAdaCtrl->fnExp_AddWaveFile(402, L"402_CD.wav");

			if (!AdaKeyStr.IsEmpty())
			{
				CString strTemp;
				strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"402_Entry.wav"), AdaKeyStr);
				m_pAdaCtrl->fnExp_AddWaveFile(402, strTemp);
			}

			m_pAdaCtrl->fnExp_PlayScreenWave(402);

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

BIZ_RETURN CTranCmn::P_NH_CD_EnterSmsCode(CString &code)
{
	NHDBG((L"[CTranCmn::P_NH_CD_EnterSmsCode]\n"));
	NVDump('O', 'C', "-1", L"Start", L"SmsCode");

	CString	GetKeyStr, AdaKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(403);
		m_pDevCmn->fnSCR_DisplayScreen(403, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
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
			m_pAdaCtrl->fnExp_AddWaveFile(403, L"403_CD.wav");

			if (!AdaKeyStr.IsEmpty())
			{
				CString strTemp;
				strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"403_Entry.wav"), AdaKeyStr);
				m_pAdaCtrl->fnExp_AddWaveFile(403, strTemp);
			}

			m_pAdaCtrl->fnExp_PlayScreenWave(403);

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

BIZ_RETURN CTranCmn::P_NH_CD_TermsOfService()
{
	int			tosPos = 0;
	CString		GetKeyStr, TermsOfService;
	BOOL		replay = TRUE;
	CStringList	tosList;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	if (!IsAdaTransaction())
	{
		// Show screen
		m_pDevCmn->fnSCR_DisplayPrevSet(405);

		tosList.AddTail("Coinsource Terms of Service\n\nLast updated: April 29, 2019\n\nThis User Agreement (\"Agreement\") is a contract between you and Clark, Sharp, and Reynolds, LLC d/b/a Coinsource, a Limited Liability Company incorporated in Texas (\"Coinsource,\" \"we\", \"us\" or \"our\") and applies to your use of the Coinsource website, Bitcoin Transaction Machines(BTM) and all services described herein (collectively, the \"Coinsource Services\")\n\n");
		tosList.AddTail("Introduction\n\nBy signing up to use a Coinsource Account, you agree to comply with and be legally bound by this Agreement. If you do not agree to any of the terms in this Agreement, or any subsequent modification to this Agreement, you may not use any of the Coinsource Services and must cancel your Coinsource Account, free of charge, as set forth below.\n\nWe may amend this Agreement by posting on the Coinsource Site or emailing to you a revised Agreement. ");
		tosList.AddTail("Any revised Agreement shall be effective immediately upon your express approval, or one month after the date of posting, except where the changes are required by law or on the instruction of a competent governmental agency. By continuing to use the Coinsource Services once the revised Agreement is effective, you agree to be bound by the Agreement. We may (a) modify or discontinue any portion of the Coinsource Services, and (b) limit or terminate your access to the Coinsource Services, at any time, without notice to you. ");
		tosList.AddTail("You agree that Coinsource shall not be liable to you or any third party for any modification or termination of the Coinsource Services, or limitations or termination of your access to the Coinsource Services, except to the extent otherwise expressly set forth herein.\n\nTo be eligible to use the Coinsource Services, you must be at least 18 years old. By accessing or using the Coinsource Services you represent and warrant that you are 18 or older.\n\n");
		tosList.AddTail("Please note the following risks of using Coinsource Services:\n\n* Every Bitcoin Transaction is confirmed by the Bitcoin network. The confirmation takes a period of time, and will be unconfirmed during that period of time (usually less than one hour, but up to one day or more). A Bitcoin Transaction is not complete while it is being confirmed. Bitcoin associated with transactions that are in a pending state will be designated accordingly."); // \n\n
		tosList.AddTail("* The risk of loss in trading or holding bitcoin can be substantial. Therefore, you should carefully consider whether trading or holding Bitcoin is suitable for you in light of your financial condition. The price or value of Bitcoin can change rapidly, decrease, and potentially even fall to zero. ");
		tosList.AddTail("If you use a Coinsource product which allows you and/or third parties to access and hold bitcoin private keys, you acknowledge that Coinsource is not responsible for safeguarding such keys and that Coinsource is not responsible for any loss of Bitcoin resulting from theft, loss, or mishandling of bitcoin private keys outside its control.\n\n1. Basic Coinsource Services."); // \n\n
		tosList.AddTail("Your Coinsource Account encompasses the following Coinsource Services, among others:\n\nOne Bitcoin wallet that allows users to store Bitcoin private keys, and to track, transfer, and manage their Bitcoin and account information and through which Coinsource can facilitate Bitcoin Transactions pursuant to instructions submitted by a user or authorized entity to Coinsource (the \"Bitcoin Wallet\");A Bitcoin conversion service through which users can buy Bitcoin from, and sell Bitcoin to, Coinsource (the \"Conversion Service\"); and each of these Coinsource Services is described in detail below."); // \n\n
		tosList.AddTail("2. Creating a Coinsource Account.\n\n2.1. Registration of Coinsource Account. In order to use any of the Coinsource Services, you must first register at a BTM or online by providing your name, phone number, identity document (if required) and affirming your acceptance of this Agreement. Upon successful completion of the registration process, Coinsource will establish your Coinsource Account. Coinsource may, in our sole discretion, refuse to allow you to establish a Coinsource Account."); // \n\n
		tosList.AddTail("2.2. Identity Verification. In order to use certain features of the Coinsource Services, including certain transfers of Bitcoin and/or currency, you may be required to provide Coinsource with certain personal information, including, but not limited to, your name, address, telephone number, e-mail address, date of birth, taxpayer identification number, and government identification number. Coinsource may also require you to answer certain questions or take actions in order to verify your identity, provide the Coinsource Services to you, or comply with applicable law. ");
		tosList.AddTail("In submitting this or any other personal information as may be required, you verify that the information is accurate and authentic, and you agree to update Coinsource if any information changes. You hereby authorize Coinsource to, directly or through third parties, make reasonably necessary inquiries to verify your identity and/or to protect against fraud, including to query identity information contained in public reports and to take actions we deem necessary. You further authorize any and all third parties to which such inquiries may be directed to fully respond to such inquiries."); // \n\n
		tosList.AddTail("2.3. Consent to Collection of Biometric Data. In opening an account with Coinsource, your face geometry will be collected and stored by Coinsource for the purpose of verifying your identity. Your face geometry data will not be disclosed by Coinsource without your consent unless the disclosure is required by law or by subpoena. Your face geometry data will be permanently deleted from Coinsource's systems no later than three (3) years after your last transaction with Coinsource. A copy of Coinsource's Biometric Information Security Policy is available upon request and is posted within Coinsource's Privacy Policy at www.coinsource.net. ");
		tosList.AddTail("By agreeing to Coinsource's Terms of Service, you agree to Coinsource's collection, use, and storage of your face geometry for the above defined purpose.\n\n3. Bitcoin Transactions.\n\n3.1. Transactions with Coinsource. When buying or selling Bitcoin through the Conversion Service, you are buying from, or selling to, Coinsource directly. Coinsource does not act as an intermediary or marketplace between other buyers and sellers of Bitcoin. ");
		tosList.AddTail("The Conversion Service is subject to the Coinsource \"Conversion Rate.\" \"Conversion Rate\" means the liquidity adjusted price of a given Bitcoin amount in terms of local currency as quoted to you on the Coinsource Site at the time you begin a Conversion transaction. The Conversion Rate is stated either as a \"Buy Price,\" the price in terms of local currency at which you purchase Bitcoin from Coinsource, or as a \"Sell Price,\" the price in terms of local currency at which you sell Bitcoin to Coinsource. For the purpose of any transaction which incorporates the Conversion Rate, you agree, as a condition of using any Coinsource Services, to accept the Conversion Rate as the sole conversion metric."); // \n\n
		tosList.AddTail("3.1.1. Bitcoin Purchase Transactions. After successfully completing the Verification Procedures, you may purchase Bitcoin from Coinsource, subject to applicable limits. If your Bitcoin purchase is marked as complete or pending in your Bitcoin Wallet, you cannot cancel, reverse, or change the order. In connection with any purchase of Bitcoin from Coinsource, Coinsource will use good faith efforts to fulfill such purchase order at the applicable Buy Price Conversion Rate quoted on the Coinsource Site at the time that you place such order. However, from time to time, it may be necessary for Coinsource to delay fulfillment of a purchase order until such time as we are able to execute the transaction. ");
		tosList.AddTail("In such cases, we will notify you through the Coinsource Site prior to the completion of your purchase order that the amount of Bitcoin that you will receive will be determined based on the applicable Buy Price Conversion Rate quoted on the Coinsource Site at the subsequent point at which Coinsource, with your further approval, may execute your transaction.\n\n3.1.2. Bitcoin Sale Transactions. After successfully completing the Verification Procedures, you may sell Bitcoin to Coinsource, subject to the above-referenced limits. ");
		tosList.AddTail("Upon any sale of Bitcoin to Coinsource, all settlements or payments by Coinsource for such transaction shall be paid in local currency (e.g., U.S. Dollars) at the applicable Sell Price Conversion Rate quoted on the Coinsource Site at the time that you approved the transaction.\n\n3.2. Third Party Transactions. As part of the services offered in connection with your Coinsource Account, Coinsource can help you send bitcoin to third parties pursuant to the instructions you provide at the BTM. Coinsource processes Bitcoin Transactions according to the instructions received from its users and we do not guarantee the identity of any user, receiver, requestee or other party. ");
		tosList.AddTail("You should verify all transaction information prior to submitting instructions to Coinsource as the relevant Bitcoin Transaction may not be cancelled or reversed once initiated.\n\n3.2.1. Validity of Third Party Addresses. If you initiate a Bitcoin Transaction by entering the recipient's Bitcoin address, Coinsource will confirm that the Bitcoin address is valid; however, Coinsource cannot confirm that the Bitcoin address belongs to the intended recipient.\n\n3.2.2. Bitcoin Storage & Transmission Delays. Coinsource does not hold any of private keys for its users. ");
		tosList.AddTail("As such, you accept the risk that a Bitcoin Transaction facilitated by Coinsource may be delayed and you agree not to hold Coinsource responsible for any damages or injury arising out of or related to such delay.\n\n3.2.3. Third Party Disputes. Coinsource has no control over, or liability for, the delivery, quality, safety, legality or any other aspect of any goods or services that you may purchase or sell to or from a third party (including other users of Coinsource Services). Coinsource is not responsible for ensuring that a buyer or a seller you are dealing with will actually complete the transaction or is authorized to do so. ");
		tosList.AddTail("If you experience a problem with any goods or services purchased from, or sold to, a third party in connection with bitcoin transferred using the Coinsource Services, or if you have a dispute with such third party, you must handle it directly with that third party. Due to the nature of Bitcoin, any Bitcoin Transaction conducted using the Coinsource Services is final regardless of any defects in or non-delivery of any goods or services purchased in connection with such Bitcoin Transaction, or any other aspect of your transaction or relationship with the relevant third party. ");
		tosList.AddTail("If you believe a third party has behaved in a fraudulent, misleading, or inappropriate manner, or if you cannot adequately resolve a dispute with a third party, you may notify Coinsource Support at support@coinsource.net so that we may consider whether or not further action is necessary.\n\n3.3. Refunds. Bitcoin transactions are irreversible. Therefore, Coinsource is not responsible for issuing refunds regardless of whether the customer was the vicitim of fraud, mistake or loss of private key."); // \n\n
		tosList.AddTail("3.4. Taxes. It is your sole responsibility to determine whether, and to what extent, any taxes apply to any transactions associated with your receipt or transfer of bitcoin, and/or to the Bitcoin Transactions you conduct through the Coinsource Services, and to withhold, collect, report and remit the correct amounts of taxes to the appropriate tax authorities.\n\n3.5. Limits. Coinsource imposes limits on the value of Bitcoin that you may buy, sell, or trade in a day. Coinsource reserves the right to change posted limits as we deem necessary or appropriate. Coinsource does not guarantee the availability of its services. If you purchase Bitcoin from Coinsource, we do not guarantee that you may sell your Bitcoin to Coinsource. ");
		tosList.AddTail("If you wish to raise your limits beyond the posted amounts, submit a request to compliance@coinsource.net. We may require you to submit additional information about yourself or your business, provide records, and arrange for meetings with Coinsource staff (the \"Extended Due Diligence Process\"). Coinsource reserves the right to charge your costs and fees associated with Extended Due Diligence, provided that we notify you in advance of any such charges accruing. In our sole discretion, we may refuse to raise your limits or we may lower your limits at a subsequent time even if you have completed the Extended Due Diligence Process."); // \n\n
		tosList.AddTail("3.6. Conversion Fee. Each Conversion Service transaction is subject to a fee (the \"Conversion Fee\") charged by Coinsource which consists of: (a) a fee for the conversion of local currency (e.g., U.S. Dollars) for Bitcoin (or vice versa) which is calculated as a percentage of the local currency to be converted, plus (b) either of: (i) a fixed transfer fee which may be applied to cover applicable bank fees, if any, or (ii) a convenience fee. Coinsource will not process a conversion if a fixed transfer fee exceeds the value of your transaction. The applicable Conversion Fee will be displayed to you on the Coinsource Site prior to your confirmation of a Conversion Service transaction."); // \n\n
		tosList.AddTail("3.7. Payments.\n\n3.7.1. You agree to pay for any Bitcoin purchase made upon confirmation of an order, regardless of changes in the applicable Buy Price Conversion Rate prior to completion of the purchase transaction, with such payment to be made in good funds in the amount calculated in the manner specified herein."); // \n\n
		tosList.AddTail("3.7.2. Prior to the completion of any transaction, we reserve the right to refuse to cancel any proposed purchases or sales of Bitcoin or transfers to or from your Wallet(s) in our sole discretion, including but not limited to instances where Coinsource suspects the transaction involves (or has a high risk of involvement in) money laundering, terrorist financing, fraud, or any other type of financial crime, in response to a subpoena, court order, or other government order, or if Coinsource suspects the transaction relates to Prohibited Use or a Prohibited Business as set forth below. Coinsource will cancel the transaction before execution and we are under no obligation to allow you to reinitiate a purchase or sale order at the same price or on the same terms as the cancelled transaction. ");
		tosList.AddTail("This paragraph does not apply to completed transactions.\n\n3.7.3. Once a transaction is completed, the transaction is permanent and irreversible. You cannot withdraw from a transaction to buy or sell Bitcoin once you have agreed to the terms of a transaction with Coinsource, even if the price of Bitcoin fluctuates. Coinsource has no control over the market price for Bitcoin.\n\nGeneral Use, Prohibited Use, and Termination."); // \n\n
		tosList.AddTail("4.1. General Use.\n\n4.1.1 Limited License. We grant you a limited, nonexclusive, nontransferable license, subject to the terms of this Agreement, to access and use the Coinsource Site, and the content, materials, information and functionality available in connection therewith (collectively, the \"Content\") solely for informational, transactional, or other approved purposes as permitted by Coinsource from time to time. Any other use of the Coinsource Site or Content is expressly prohibited. All other rights in the Coinsource Site or Content are reserved by us and our licensors. ");
		tosList.AddTail("We reserve all rights in the Coinsource Site and Content and you agree that this Agreement does not grant you any rights in or licenses to the Coinsource Site or the Content, except for this express, limited license. You will not otherwise copy, transmit, distribute, sell, resell, license, de-compile, reverse engineer, disassemble, modify, publish, participate in the transfer or sale of, create derivative works from, perform, display, incorporate into another website, or in any other way exploit any of the Content or any other part of the Coinsource Site or any derivative works thereof, in whole or in part for commercial or non-commercial purposes. ");
		tosList.AddTail("Without limiting the foregoing, you will not frame or display the Coinsource Site or Content (or any portion thereof) as part of any other website or any other work of authorship without our prior written permission. If you violate any portion of this Agreement, your permission to access and use the Coinsource Services may be terminated pursuant to this Agreement. In addition, we reserve the right to all remedies available at law and in equity for any such violation. \"Coinsource.net\", \"Coinsource\", and all logos related to the Coinsource Services or displayed on the Coinsource Site are either trademarks or registered marks of Coinsource or its licensors. You may not copy, imitate or use them without Coinsource's prior written consent."); // \n\n
		tosList.AddTail("4.1.2. Website Accuracy. We intend to provide accurate and timely information on the Coinsource Site. However, the Coinsource Site may not always be accurate, complete or current and may also include technical inaccuracies or typographical errors. In an effort to continue to provide you with as complete and accurate information as possible, information may be changed or updated from time to time without notice, including without limitation information regarding our policies, products and services. Accordingly, you should verify all information before relying on it, and all decisions based on information contained on the Coinsource Site are your sole responsibility and we shall have no liability for such decisions."); // \n\n
		tosList.AddTail("4.1.3. Third-Party Materials. From time to time, the Coinsource Site may contain references or links to third-party materials and third-party applications which are not controlled by us. Such information, links, and third-party applications are provided as a convenience to you. Such links should not be considered endorsements and such reference does not imply our recommendation, approval, affiliation, or sponsorship of that respective property, product, service, or process. You acknowledge and agree that we are not responsible for any aspect of the information, content, or services contained in any third-party materials or on any third-party sites accessible or linked to the Coinsource Site."); // \n\n
		tosList.AddTail("4.1.4. Third-Party Applications. If, to the extent permitted by Coinsource from time to time, you grant express permission to a third party to access or connect to your Coinsource Account, either through the third party's product or service or through the Coinsource Site, you acknowledge that granting permission to a third party to take specific actions on your behalf does not relieve you of any of your responsibilities under this Agreement. You are fully responsible for all acts or omissions of any third party using your Coinsource Account credentials. Further, you acknowledge and agree that you will not hold Coinsource responsible for, and will indemnify Coinsource from, any liability arising out of or related to any act or omission of any third party using your Coinsource Account credentials. ");
		tosList.AddTail("You may change or remove permissions granted by you to third parties with respect to your Coinsource Account at any time through the Account Settings (Integrations) page on the Coinsource Site.\n\n4.1.6. Customer Accounts and Assets. You hereby certify to us that any funds used by you in connection with the Coinsource Services are either owned by you or that you are validly authorized to transfer such funds. In particular, you acknowledge that Coinsource may not be a qualified custodian under applicable law, and represent that your use of the Coinsource Services is in compliance with any applicable requirements governing the maintenance and use of fiduciary accounts and custodial assets."); // \n\n
		tosList.AddTail("4.2. Prohibited Use. In connection with your use of the Coinsource Services, and your interactions with other users, and third parties you agree and represent you will not engage in any Prohibited Business or Prohibited Use defined herein. We reserve the right at all times to monitor, review, retain and/or disclose any information as necessary to satisfy any applicable law, regulation, legal process or governmental request. We reserve the right to cancel and/or suspend your Coinsource Account immediately and without notice if we determine, in our sole discretion, that your Account is associated with Prohibited Use and/or a Prohibited Business."); // \n\n
		tosList.AddTail("4.3. Export Controls & Sanctions. The supply of Bitcoin and the Coinsource Services through the Coinsource Site is subject to United States laws and regulations. By acquiring any such items through the Coinsource Site, you represent and warrant that your acquisition comports with and your use of the item will comport with those requirements. Without limiting the foregoing, you may not acquire Bitcoin or any of the Coinsource Services through the Coinsource Site if: ");
		tosList.AddTail("(1) you are in, under the control of, or a national or resident of Cuba, Iran, North Korea, Sudan, or Syria or any other country subject to United States embargo, UN sanctions, HM Treasury's financial sanctions regime, or if you are on the U.S. Treasury Department's Specially Designated Nationals List or the U.S. Commerce Department's Denied Persons List, Unverified List, Entity List HM Treasury's financial sanctions regime; or ");
		tosList.AddTail("(2) you intend to supply the acquired Bitcoin or Coinsource Services to Cuba, Iran, North Korea, Sudan or Syria or any other country subject to United States embargo or HM Treasury's financial sanctions regime (or a national or resident of one of these countries), or to a person on the Specially Designated Nationals List, Denied Persons List, Unverified List, Entity List, or HM Treasury's financial sanctions regime.\n\n4.4. Suspension, Termination, and Cancellation. Coinsource may: (a) suspend, restrict, or terminate your access to any or all of the Coinsource Services, and/or (b) deactivate or cancel your Coinsource Account if:"); // \n\n
		tosList.AddTail("We are so required by a valid subpoena, court order, or order of a government authority;We have reason to believe you are using your Coinsource Account in connection with Prohibited Business or Prohibited Use;We are notified that your Coinsource Account is subject of or to any pending litigation, investigation, or government proceeding and/or we perceive a heightened risk of legal or regulatory non-compliance associated with your Account activity;Our Service Partners are unable to support your use; orWe have reason to believe that you are circumventing Coinsource's controls, including, but not limited to, opening multiple Coinsource Accounts or abusing promotions which Coinsource may offer from time to time. ");
		tosList.AddTail("You will not be charged for canceling your Coinsource Account and will only be required to pay for those Coinsource Services used that are subject to charges. If any transaction is in a pending status at the time your Coinsource Account is cancelled or suspended, such transaction may be cancelled and/or refunded as appropriate. You may not cancel your Coinsource Account to evade an investigation or avoid paying any amounts otherwise due to Coinsource. Upon cancellation of your Coinsource Account, you authorize Coinsource to cancel or suspend pending Conversion Service transactions and hold the funds associated with such transactions until Coinsource is certain that funding reversal windows are complete. ");
		tosList.AddTail("In the event that you or Coinsource terminates this Agreement or your access to the Coinsource Services, or deactivates or cancels your Coinsource Account, you will remain liable for all amounts due hereunder. In the event that a technical problem causes system outage or Account errors, Coinsource may temporarily suspend access to your Account until the problem is resolved.\n\nOur Relationship with You.\n\n5.1. Relationship of the Parties. Coinsource is an independent contractor for all purposes. ");
		tosList.AddTail("Nothing in this Agreement shall be deemed or is intended to be deemed, nor shall it cause, you and Coinsource to be treated as partners, joint ventures, or otherwise as joint associates for profit, or either you or Coinsource to be treated as the agent of the other.\n\n5.2. Service Providers. From time to time, Coinsource may engage third parties to assist Coinsource in providing certain aspects of the Coinsource Services (each, a \"Service Provider\"). Service Providers may include, but are not limited to, Coinsource's banking partners and technology or engineering service providers."); // \n\n
		tosList.AddTail("5.3. Your Privacy. Protecting your privacy is very important to Coinsource. Please review our Privacy Policy, which is hereby incorporated by reference into this Agreement, in order to better understand our commitment to maintaining your privacy, as well as our use and disclosure of your information.\n\n5.4. Privacy of Others; Marketing. In the unlikely scenario that you obtain information about another user through the Coinsource Services, you must keep the information confidential. ");
		tosList.AddTail("You may not disclose or distribute a user's information to a third party or use the information except as reasonably necessary to effectuate a transaction and other functions reasonably incidental thereto such as support, reconciliation and accounting unless you receive the user's express consent to do so.\n\n5.5. Password Security and Keeping Your Contact Information Current. You are responsible for maintaining adequate security and control of any IDs, passwords, hints, personal identification numbers (PINs), API keys or any other codes that you use to access the Coinsource Services. ");
		tosList.AddTail("Any loss or compromise of the foregoing information and/or your personal information may result in unauthorized access to your Coinsource Account and the loss or theft of any bitcoin and/or funds held in your Coinsource Account and any associated accounts. You are responsible for keeping your email address and telephone number up to date in your Account Profile in order to receive any notices or alerts that we may send you. We assume no responsibility for any loss due to compromise of your sensitive information or failure to follow or act on any notices or alerts that we may send to you. In the event you believe your Coinsource Account information has been compromised, contact Coinsource Support immediately at support@coinsource.net."); // \n\n
		tosList.AddTail("5.6. Consent to Electronic Records. By using the Coinsource Services, you agree that Coinsource may provide you with any notices or other communications about your Coinsource Account and the Coinsource Services electronically: (a) via email (in each case to the address that you provide), SMS message, or telephone call (in each case to the phone number that you provide), or (b) by posting to the Coinsource Site. For notices made by email, the date of receipt will be deemed the date on which such notice is transmitted. If you do not wish to receive notices or other communications electronically, we will be unable to support your Coinsource Account and your Coinsource Account will be cancelled."); // \n\n
		tosList.AddTail("5.7. Notices to Coinsource. We prefer receiving notices to Coinsource electronically through our support system at support@coinsource.net.\n\n6. Customer Feedback, Queries, and Complaints\n\n6.1. Contact Coinsource. If you have any feedback, questions, or complaints, contact us via our Customer Support email address at support@coinsource.net. Please provide your name, address, and any other information we may need to identify you, your Coinsource Account, and the transaction on which you have feedback, questions, or complaints."); // \n\n
		tosList.AddTail("6.2. Complaints. In the event of a complaint, please set out the cause of your complaint, how you would like us to resolve the complaint and any other information you believe to be relevant. We will acknowledge your complaint within 24 hours of its receipt by our Customer Support email at support@coinsource.net. A Customer Complaints officer (\"Officer\") will review your complaint. The Officer will consider your complaint without prejudice based on the information you have provided and any information provided by Coinsource. ");
		tosList.AddTail("Within two weeks of our receipt of your complaint the Officer will address your complaint by sending you an e-mail (\"Resolution Notice\") in which the Officer will: (i) offer to resolve your complaint in the way your requested; (ii) make a determination rejecting your complaint and set out the reasons for the rejection; or (iii) offer to resolve your complaint with an alternative solution. Upon receipt of the Resolution Notice, you will have 20 business days to accept or reject the offer or determination, which you must do by following the instructions set out in the Resolution Notice. Failure to respond to a Resolution Notice will be deemed to be a withdrawal of the complaint."); // \n\n
		tosList.AddTail("6.3. Appeals. In the event that you reject an offer or determination (\"Rejection\"), please set out the reasons for the Rejection and include all additional information that you believe is pertinent to your complaint. Any Rejection will be treated as an application to appeal the offer or determination set out in the Resolution Notice. Our Customer Appeals Committee (\"Appeals Committee\") will impartially consider your complaint (including any additional information provided). We will acknowledge your Appeal within 24 hours of its receipt if you contact us in the prescribed electronic way, and within ten business days if you contact us in any other prescribed way. ");
		tosList.AddTail("Within four weeks of our receipt of the Rejection the Appeals Committee will address your complaint by sending you an e-mail (\"Final Notice\") in which the Appeals Committee will: (i) uphold the Resolution Notice; or (ii) reject the Resolution Notice. Notwithstanding its decision regarding the Resolution Notice, the Appeals Committee may also make a new offer to resolve the complaint. Upon receipt of the Final Notice, you will have 20 business days to accept or reject the offer or determination, which you must do by following the instructions set out in the Final Notice.Failure to respond to a Final Notice will be deemed to be a withdrawal of the complaint. ");
		tosList.AddTail("For consumers only: if you fail to respond to a Final Notice and file a claim in a competent court, your failure may be used as evidence of your unwillingness to settle the issue and/or the vexatious nature of the complaint.\n\n6.4. Offers. Any offer made under this Section 6 will only become binding on Coinsource if you accept the offer by following the instructions provided by Coinsource within the stated timeframe. Any offer under this Section 6 will not constitute any admission by Coinsource of any wrongdoing or liability regarding the subject matter of the complaint. ");
		tosList.AddTail("Any acceptance of an offer by you will constitute an acceptance that the complaint is resolved and an undertaking that you will not file a claim in any competent court against Coinsource regarding the subject matter of the complaint.\n\n6.5. Claims. This provision only applies to consumers. You agree to use the complaints procedure of this Section 6 before filing any claim in a competent court. Failure to comply with this provision may be used as evidence of your unwillingness to settle the issue and/or the vexatious nature of the complaint."); // \n\n
		tosList.AddTail("7. General Provisions.\n\n7.1. Limitations of Liability. (A) IN NO EVENT SHALL COINSOURCE, ITS AFFILIATES AND SERVICE PROVIDERS, OR ANY OF THEIR RESPECTIVE OFFICERS, DIRECTORS, AGENTS, JOINT VENTURERS, EMPLOYEES OR REPRESENTATIVES, BE LIABLE FOR LOST PROFITS OR ANY SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION WITH THE COINSOURCE SITE, THE COINSOURCE SERVICES, OR THIS AGREEMENT (HOWEVER ARISING, INCLUDING NEGLIGENCE)."); // \n\n
		tosList.AddTail("(B) FOR CONSUMERS ONLY: WHERE APPLICABLE CONSUMER PROTECTION LAW LIMITS THE EFFECTIVENESS OF SECTION 7.1(A) WITH RESPECT TO CONSUMERS, SECTION 7.1(A) ONLY APPLIES TO THE MAXIMUM EXTENT AS PERMITTED BY LAW.\n\n7.2. Computer Viruses. We shall not bear any liability, whatsoever, for any damage or interruptions caused by any computer viruses, spyware, scareware, Trojan horses, worms or other malware that may affect your computer or other equipment, or any phishing, spoofing or other attack. We advise the regular use of a reputable and readily available virus screening and prevention software. ");
		tosList.AddTail("You should also be aware that SMS and email services are vulnerable to spoofing and phishing attacks and should use care in reviewing messages purporting to originate from Coinsource. Always log into your Coinsource Account through the Coinsource Site to review any transactions or required actions if you have any uncertainty regarding the authenticity of any communication or notice."); // \n\n
		tosList.AddTail("7.3. Release of Coinsource from Disputes with Other Users. If you have a dispute with one or more users of the Coinsource services, to the extent permitted by law, you release Coinsource, its affiliates and service providers, and each of their respective officers, directors, agents, joint venturers, employees and representatives from any and all claims, demands and damages (actual and consequential) of every kind and nature arising out of or in any way connected with such disputes. In addition, to the extent permitted by law, in entering into this release you expressly waive any protections, that would otherwise limit the coverage of this release to include only those claims which you may know or suspect to exist in your favor at the time of agreeing to this release."); // \n\n
		tosList.AddTail("7.4. No Warranty. (A) THE COINSOURCE SERVICES ARE PROVIDED \"AS IS\" AND \"AS AVAILABLE\" BASIS WITHOUT ANY REPRESENTATION OR WARRANTY, WHETHER EXPRESS, IMPLIED, OR STATUTORY. COINSOURCE SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT. COINSOURCE DOES NOT MAKE ANY REPRESENTATIONS OR WARRANTIES THAT ACCESS TO ANY PART OF THE COINSOURCE SERVICES, OR ANY OF THE MATERIALS CONTAINED THEREIN, WILL BE CONTINUOUS, UNINTERRUPTED, TIMELY, ERROR-FREE OR SECURE. ");
		tosList.AddTail("OPERATION OF THE COINSOURCE SITE MAY BE INTERFERED WITH BY NUMEROUS FACTORS OUTSIDE OF OUR CONTROL. WE ARE NOT RESPONSIBLE FOR THE BITCOIN MARKET, AND WE MAKE NO REPRESENTATIONS OR WARRANTIES CONCERNING THE REAL OR PERCEIVED VALUE OF BITCOIN AS DENOMINATED IN ANY QUOTED CURRENCY. ALTHOUGH WE MAY PROVIDE HISTORICAL AND/OR REAL-TIME DATA REGARDING THE PRICE OF BITCOIN, WE MAKE NO REPRESENTATIONS REGARDING THE QUALITY, SUITABILITY, TRUTH, USEFULNESS, ACCURACY, OR COMPLETENESS OF SUCH DATA, AND YOU SHOULD NOT RELY ON SUCH DATA FOR ANY REASON WHATSOEVER. ");
		tosList.AddTail("YOU UNDERSTAND AND AGREE THAT THE VALUE OF BITCOIN CAN BE VOLATILE, AND WE ARE NOT IN ANY WAY RESPONSIBLE OR LIABLE FOR ANY LOSSES YOU MAY INCUR BY HOLDING OR TRADING BITCOIN, EVEN IF THE COINSOURCE SERVICES ARE DELAYED, SUSPENDED, OR INTERRUPTED FOR ANY REASON. FURTHER, COINSOURCE MAKES NO REPRESENTATIONS OR WARRANTIES AS TO THE QUALITY, SUITABILITY, TRUTH, USEFULNESS, ACCURACY, OR COMPLETENESS OF THE COINSOURCE SERVICES OR ANY MATERIALS CONTAINED THEREIN. ");
		tosList.AddTail("COINSOURCE WILL MAKE REASONABLE EFFORTS TO ENSURE THAT REQUESTS FOR BUYS AND SELLS ARE PROCESSED IN A TIMELY MANNER BUT COINSOURCE MAKES NO REPRESENTATIONS OR WARRANTIES REGARDING THE AMOUNT OF TIME NEEDED TO COMPLETE PROCESSING BECAUSE THE COINSOURCE SERVICES ARE DEPENDENT UPON MANY FACTORS OUTSIDE OF OUR CONTROL, SUCH AS DELAYS IN THE BITCOIN BLOCKCHAIN OR INTERNATIONAL MAIL SERVICE.\n\n(B) FOR CONSUMERS ONLY: ");
		tosList.AddTail("WHERE APPLICABLE CONSUMER PROTECTION LAW LIMITS THE EFFECTIVENESS OF SECTION 7.5(A) WITH RESPECT TO CONSUMERS, SECTION 7.5(A) ONLY APPLIES TO THE MAXIMUM EXTENT AS PERMITTED BY LAW.\n\n7.5. Indemnification. You agree to indemnify and hold Coinsource, its affiliates and Service Providers, and each of their respective officers, directors, agents, joint venturers, employees and representatives, harmless from any claim or demand (including attorneys' fees and any fines, fees or penalties imposed by any regulatory authority) arising out of or related to (i) your breach of this Agreement, (ii) your use of Coinsource Services, or (iii) your violation of any law, rule or regulation, or the rights of any third party."); // \n\n
		tosList.AddTail("7.6. Entire Agreement. This Agreement sets forth the entire understanding and agreement between you and Coinsource as to the subject matter hereof, and supersedes any and all prior discussions, agreements and understandings of any kind and every nature between and among you and Coinsource.\n\n7.7. Assignment. This Agreement, and any rights and licenses granted hereunder, may not be transferred or assigned by you, but may be assigned by Coinsource without restriction, including without limitation to any of its affiliates or subsidiaries, or to any successor in interest of any business associated with the Coinsource Services.");
		tosList.AddTail("Any attempted transfer or assignment in violation hereof shall be null and void. Subject to the foregoing, this Agreement will bind and inure to the benefit of the parties, their successors and permitted assigns. In the event that Coinsource is acquired by or merged with a third-party entity, we reserve the right, in any of these circumstances, to transfer or assign the information we have collected from you as part of such merger, acquisition, sale, or other change of control."); // \n\n
		tosList.AddTail("7.8. Severability. If any provision of this Agreement shall be determined to be unenforceable under any rule, law or regulation of any governmental agency, such provision will be interpreted to accomplish the objectives of the provision to the greatest extent possible under any applicable law and the validity or enforceability of any other provision of this Agreement shall not be affected."); // \n\n
		tosList.AddTail("7.9. Survival. All provisions of this Agreement which by their nature extend beyond the expiration or termination of this Agreement, including, without limitation, sections pertaining to suspension or termination, Coinsource Account cancellation, debts owed to Coinsource, general use of the Coinsource Site, disputes with Coinsource, and general provisions, shall survive the termination or expiration of this Agreement."); // \n\n
		tosList.AddTail("7.10. Governing Law and Jurisdiction. This Agreement will be governed by Texas law.\n\n7.11. Force Majeure. We shall not be liable for delays, failure in performance or interruption of service which result directly or indirectly from any cause or condition beyond our reasonable control, including but not limited to, any delay or failure due to any act of God, act of civil or military authorities, act of terrorists, civil disturbance, war, strike or other labor dispute, fire, interruption in telecommunications or Internet services or network provider services, failure of equipment and/or software, other catastrophe or any other occurrence which is beyond our reasonable control and shall not affect the validity and enforceability of any remaining provisions."); // \n\n
		tosList.AddTail("7.12. Section Headings. Section headings in this Agreement are for convenience only, and shall not govern the meaning or interpretation of any provision of this Agreement.\n\nAPPENDIX 1: PROHIBITED BUSINESSES AND PROHIBITED USE\n\nThe following categories of businesses, business practices, and sale items are barred from Coinsource Services (\"Prohibited Businesses\"). By opening a Coinsource Account, you confirm that you will not use Coinsource Services to accept payments in connection with the following businesses, activities, practices, or items:"); // \n\n
		tosList.AddTail("Operating as an unlicensed money transmitter, money service, payment service provider, e-money, or any other financial services business which requires licensure, including but not limited to exchanges of virtual currencies, sales of money orders or traveler's checks, and escrow servicesCounterfeit products or any product or service that infringes upon the copyright, trademark, or trade secrets of any third partyStolen goodsNarcotics, controlled substances, prescription and pharmaceutical services, drug paraphernalia, or any substances designed ");
		tosList.AddTail("to mimic illegal drugsGamblingSports forecasting or odds makingProstitution or illegal escort servicesViolent acts towards self or others, or activities or items that encourage, promote, facilitate or instruct others regarding the sameFunding any of the items included on this Prohibited Businesses listExtortion, blackmail, or efforts to induce unearned paymentsUnlicensed sale of firearms and certain weaponsEngaging in deceptive marketing practiceAny business that violates any law, statute, ordinance or regulationYou may not use your Coinsource Account to engage in the following categories of activity (\"Prohibited Use\")."); // \n\n
		tosList.AddTail("By opening a Coinsource Account, you confirm that you will not use your Account to do any of the following:\n\nViolate or assist any party in violating any law, statute, ordinance, regulation, or any rule of any self-regulatory or similar organization of which you are or are required to be a member (for example, those laws, rules, or regulations governing financial services, controlled substances, or consumer protections)Partake in a transaction which involves the proceeds of any unlawful activity;Partake in any transaction involving online gambling;Defraud or attempt to defraud Coinsource or other Coinsource users;");
		tosList.AddTail("Infringe upon Coinsource's or any third party's copyright, patent, trademark, or intellectual property rights;Provide false, inaccurate, or misleading information;Take any action that imposes an unreasonable or disproportionately large load on our infrastructure, or detrimentally interfere with, intercept, or expropriate any system, data, or information;Interfere with another individual's or entity's access to or use of any of the Coinsource Services;Defame, abuse, harass, stalk, threaten, or otherwise violate or infringe the legal rights (such as, but not limited to, rights of privacy, publicity and intellectual property) of others;");
		tosList.AddTail("Publish, distribute, or disseminate any unlawful material or information;Transmit or upload any material to the Coinsource Site that contains viruses, Trojan horses, worms, or any other harmful or deleterious programs;Harvest or otherwise collect information from the Coinsource Site about others, including without limitation email addresses, without proper consent;Act as a payment intermediary or aggregator or otherwise resell any of the Coinsource Services, unless expressly authorized by Coinsource in writing;Transfer any rights granted to you under this Agreement;");
		tosList.AddTail("Use the Coinsource Account information of another party to access or use the Coinsource Site, except in the case of specific Merchants and/or applications which are specifically authorized by a user to access such user's Coinsource Account and information;Otherwise attempt to gain unauthorized access to the Coinsource Site, other Coinsource Accounts, computer systems or networks connected to the Coinsource Site, through password mining or any other means; orEngage in transactions involving items that infringe or violate any copyright, trademark, right of publicity or privacy or any other proprietary right under the law.");
		tosList.AddTail("APPENDIX 2: VERIFICATION PROCEDURES AND LIMITS\n\nCoinsource uses multi-level systems and procedures to collect and verify information about you in order to protect Coinsource and the community from fraudulent users, and to keep appropriate records of Coinsource's customers. Your daily or weekly Conversion limits, Trading limits, and limits on transactions from a linked account are based on the identifying information and/or proof of identity you provide to Coinsource.\n\nUsers in Approved Countries who wish to buy and sell Bitcoin using Coinsource's Conversion Service, at minimum, must:"); // \n\n
		tosList.AddTail("Establish a Coinsource Account by providing your name, phone number, and accepting the Coinsource User TermsIn addition, users in Approved Countries who wish to use Coinsource's Buy and Sell functionality, at minimum, must:Submit a copy of your government-issued identification (in certain instances)Notwithstanding these minimum verification procedures for the referenced Coinsource Services, Coinsource may require you to provide or verify additional information, or to wait some amount of time after completion of a transaction, before permitting you to use any Coinsource Services and/or before permitting you to engage in transactions beyond certain volume limits. ");
		tosList.AddTail("You may determine the volume limits associated with your level of identity verification by visiting your account's Limits page.\n\nIf Coinsource cannot successfully verify your name, address, date of birth, and/or identification are authentic and accurate, certain Coinsource Services may not be available to you. From time to time, Coinsource may impose limits on the balance you may store in your Wallet.\n\nYou may contact support@coinsource.net to request larger daily limits. ");
		tosList.AddTail("Coinsource will require you to submit to Extended Due Diligence. Additional fees and costs may apply, and Coinsource does not guarantee that we will raise your limits.\n\nAPPENDIX 3: E-SIGN DISCLOSURE AND CONSENT\n\nThis policy describes how Coinsource delivers communications to you electronically.\n\nElectronic Delivery of Communications"); // \n\n
		tosList.AddTail("You agree to receive electronically all communications, agreements, documents, notices and disclosures (collectively, \"Communications\") that we provide in connection with your Coinsource Account and your use of Coinsource Services. Communications include:\n\nTerms of use and policies you agree to (e.g., the Coinsource User Agreement and Privacy Policy), including updates to these agreements or policies;Account details, transaction receipts, confirmations, and any other Account or transaction information;Legal, regulatory, and tax disclosures or statements we may be required to make available to you;");
		tosList.AddTail("Responses to claims or customer support inquiries filed in connection with your Account; andOccasional advertisements regarding discounts and other promotional material.We will provide these Communications to you by posting them on the Coinsource website, emailing them to you at the primary email address listed in your Coinsource profile, communicating to you via instant chat, and/or through other electronic communication such as text message or mobile push notification."); // \n\n
		tosList.AddTail("Hardware and Software Requirements\n\nIn order to access and retain electronic Communications, you will need the following computer hardware and software:\n\nA device with an Internet connection;A current web browser that includes 128-bit encryption (e.g. Internet Explorer version 9.0 and above, Firefox version 3.6 and above, Chrome version 31.0 and above, or Safari 7.0 and above) with cookies enabled;");
		tosList.AddTail("A valid email address (your primary email address on file with Coinsource); andSufficient storage space to save past Communications or an installed printer to print them.How to Withdraw Your Consent\n\nYou may withdraw your consent to receive Communications electronically by e-mailing us at support@coinsource.net, or by selecting the opt-out option via text message.\n\n");
		tosList.AddTail("Updating your Information\n\nIt is your responsibility to provide us with a true, accurate and complete e-mail address and your contact information, and to keep such information up to date. You understand and agree that if Coinsource sends you an electronic Communication but you do not receive it because your primary email address on file is incorrect, out of date, blocked by your service provider, or you are otherwise unable to receive electronic Communications, Coinsource will be deemed to have provided the Communication to you.\n\n");
		tosList.AddTail("You may update your information by logging into your account and visiting settings or by contacting our support team via e-mail at support@coinsource.net.\n\nCoinsource SMS Communications\n\nBy submitting your phone number in a Coinsource web form, you consent to receive one or more automated texts at the phone number from which you texted or at the phone number you entered in the form.");
		tosList.AddTail("We will not be liable for any delays in the receipt of any SMS messages as delivery is subject to effective transmission from your mobile service operator. SMS MESSAGE SERVICES ARE PROVIDED ON AN \"AS IS\" BASIS, AND WE MAKE NO WARRANTY, EXPRESS OR IMPLIED, AND ALL WARRANTIES, INCLUDING IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR ANY PARTICULAR PURPOSE, ARE HEREBY EXPRESSLY DISCLAIMED.Data obtained from you in connection with this SMS service may include your cell phone number, your carrier's name, and the date, time and content of your messages, as well as other information that you provide. ");
		tosList.AddTail("We may use this information to contact you and to provide the services you request from us.By subscribing, you consent to receive two-factor authentication texts to confirm your identity at our ATM network and transaction receiptsBy subscribing or otherwise using the service, you acknowledge and agree that we will have the right to change and/or terminate the service at any time, with or without cause and/or advance notice.To cancel your SMS subscriptions, text STOP to 768723 in reply to a text message you receive. You may receive a subsequent message confirming your opt-out request.For additional help, text HELP to 768723 in reply to a text message you receive.");
		tosList.AddTail("Message and Data Rates May ApplyT-Mobile® is not liable for delayed or undelivered messages.United States Participating Carriers Include AT&T, T-Mobile®, Verizon Wireless, Sprint, Boost, U.S. Cellular®, MetroPCS®, InterOp, Cellcom, C Spire Wireless, Cricket, Virgin Mobile and others.Our privacy policy is at https://coinsource.net/privacy-policy/Customer Support: support@coinsource.net\n\nAPPENDIX 4: NEW YORK SUPPLEMENT\n\nIf you are engaging in a transaction in the State of New York, pursuant to 23 NYCRR 200:19, please be aware of the following material risks involved in Virtual Currency generally, including at a minimum, the following:\n\n");
		tosList.AddTail("Virtual Currency is not legal tender, is not backed by the government, and accounts and value balances are not subject to Federal Deposit Insurance Corporation or Securities Investor Protection Corporation protectionsLegislative and regulatory changes or actions at the state, federal, or international level may adversely affect the use, transfer, exchange, and value of Virtual Currency;Transactions in Virtual Currency may be irreversible, and, accordingly, losses due to fraudulent or accidental transactions may not be recoverable;Some Virtual Currency transactions shall be deemed to be made when recorded on a public ledger, which is not necessarily the date or time that the customer initiates the transaction;");
		tosList.AddTail("The value of Virtual Currency may be derived from the continued willingness of market participants to exchange Fiat Currency for Virtual Currency, which may result in the potential for permanent and total loss of value of a particular Virtual Currency should the market for that Virtual Currency disappear;There is no assurance that a Person who accepts a Virtual Currency as payment today will continue to do so in the future;The volatility and unpredictability of the price of Virtual Currency relative to Fiat Currency may result in significant loss over a short period of time;The nature of Virtual Currency may lead to an increased risk of fraud or cyber attack;");
		tosList.AddTail("The nature of Virtual Currency means that any technological difficulties experienced by the Licensee may prevent the access or use of a customer's Virtual Currency; andAny bond or trust account maintained by the Licensee for the benefit of its customers may not be sufficient to cover all losses incurred by the customer.Pursuant to 23 NYCRR 200:19(b), please take notice of the following risks involved in Virtual Currency:\n\nYou are solely liability for unauthorized Virtual Currency transactions, no different than you are responsible for unauthorized cash transactions or unauthorized Fiat Currency transactions. ");
		tosList.AddTail("You should protect your Virtual Currency transactions and your digital wallet with the same care and concern as your cash transactions and your traditional wallet.Coinsource does not offer or provide preauthorized Virtual Currency transfers;The Privacy Policy addresses those circumstances under which Coinsource may disclose information concerning the customer's account to third parties;Coinsource does not act as a custodian or otherwise hold Virtual Currency, claims or any other assets on behalf of the customers. All transactions are recorded on the virtual currency blockchain at www.blockchain.info. ");
		tosList.AddTail("The customer should preserve all records of transactions provided by Coinsource, and does not have the right to receive periodic account statements and valuations from Coinsource.You have the right to receive a receipt, trade ticket, or other evidence of a transaction. Coinsource may provide such evidence electronically or in writing.");

		POSITION pos = tosList.FindIndex(tosPos);
		TermsOfService = tosList.GetAt(pos);

		m_pDevCmn->fnSCR_DisplayString(1, TermsOfService);
		m_pDevCmn->fnSCR_DisplayImage(1, "off");
		m_pDevCmn->fnSCR_DisplayScreen(405, KEYIN_TIME_OUT, PIN_MENU_MODE);
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
			m_pAdaCtrl->fnExp_AddWaveFile(405, L"405.wav");

			m_pAdaCtrl->fnExp_PlayScreenWave(405);

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
				// F6, I Accept
				else if (GetKeyStr == L"1")
				{
					GetKeyStr = L"ACCEPT";
				}
				// F8, I do not accept
				else if (GetKeyStr == L"2")
				{
					GetKeyStr = L"NOACCEPT";
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

				return RES_NG;
			}
			// F5, Prev
			else if (GetKeyStr == L"PREV")
			{
				if (tosPos > 0)
				{
					tosPos--;
					POSITION pos = tosList.FindIndex(tosPos);
					TermsOfService = tosList.GetAt(pos);

					m_pDevCmn->fnSCR_DisplayImage(1, "off");
				}

				m_pDevCmn->fnSCR_DisplayString(1, TermsOfService);
				m_pDevCmn->fnSCR_DisplayUpdate(405);
			}
			// F6, I Accept
			else if (GetKeyStr == L"ACCEPT")
			{
				if (IsAdaTransaction())
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"405_AcceptSelected.wav", TRUE);

				return RES_OK;
			}
			// F7, Next
			else if (GetKeyStr == L"NEXT")
			{
				int tosCount = tosList.GetCount() - 1;

				if (tosPos < tosCount)
				{
					tosPos++;
					POSITION pos = tosList.FindIndex(tosPos);
					TermsOfService = tosList.GetAt(pos);
				}
				
				if (tosPos == tosCount)
					m_pDevCmn->fnSCR_DisplayImage(1, "on ");

				m_pDevCmn->fnSCR_DisplayString(1, TermsOfService);
				m_pDevCmn->fnSCR_DisplayUpdate(405);
			}
			// F8, I do not accept
			else if (GetKeyStr == L"NOACCEPT")
			{
				if (IsAdaTransaction())
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"405_AcceptNotSelected.wav", TRUE);

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

BIZ_RETURN CTranCmn::P_NH_CD_ChooseCryptoCurrency(CDCryptoCurrencies &cryptoCurrencies)
{
	CString				GetKeyStr;
	int					numCryptoCurrencies = 0;
	CDCryptoCurrency	cryptoCurrency;
	POSITION			pos = NULL;
	BOOL				replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	CString cryptoCurrencyStr;
	CStringList cryptoCurrencyValues;

	if (!IsAdaTransaction())
	{
		// Show screen
		m_pDevCmn->fnSCR_DisplayPrevSet(406);

		/*
		 * 1 - Cryptocurrency 1
		 * 2 - Cryptocurrency 2
		 * 3 - Cryptocurrency 3
		 */

		// Cryptocurrencies
		pos = cryptoCurrencies.GetHeadPosition();

		while (pos != NULL)
		{
			cryptoCurrency = cryptoCurrencies.GetNext(pos);

			m_pDevCmn->fnSCR_DisplayString(1 + numCryptoCurrencies, cryptoCurrency.DisplayName);

			numCryptoCurrencies++;

			if (numCryptoCurrencies > MAX_CRYPTO_CURRENCIES)
			{
				// Only supports MAX_CRYPTO_CURRENCIES cryptocurrencies
				break;
			}
		}

		CString strTemp;
		strTemp.Format(L"$%d", _cdCryptoState.DailyLimit);
		m_pDevCmn->fnSCR_DisplayString(7, strTemp);

		m_pDevCmn->fnSCR_DisplayScreen(406, KEYIN_TIME_OUT, PIN_MENU_MODE);
	}
	else
	{
		// Cryptocurrencies
		pos = cryptoCurrencies.GetHeadPosition();

		cryptoCurrencyStr = "Please select one of the currencies available. ";

		while (pos != NULL)
		{
			cryptoCurrency = cryptoCurrencies.GetNext(pos);

			CString strTemp;
			strTemp.Format(L"Press %d for %s. ", 1 + numCryptoCurrencies, cryptoCurrency.DisplayName);

			cryptoCurrencyStr += strTemp;
			cryptoCurrencyValues.AddTail(cryptoCurrency.DisplayName);

			numCryptoCurrencies++;

			if (numCryptoCurrencies > MAX_CRYPTO_CURRENCIES)
			{
				// Only supports MAX_CRYPTO_CURRENCIES cryptocurrencies
				break;
			}
		}

		CString strTemp;
		strTemp.Format(L"Your remaining daily limit is $%d.", _cdCryptoState.DailyLimit);
		cryptoCurrencyStr += strTemp;
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
			m_pAdaCtrl->fnExp_AddWaveFile(406, cryptoCurrencyStr);

			m_pAdaCtrl->fnExp_PlayScreenWave(406);

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
					GetKeyStr = L"F1";
				}
				else if (GetKeyStr == L"3")
				{
					GetKeyStr = L"F2";
				}
				else if (GetKeyStr == L"4")
				{
					GetKeyStr = L"F3";
				}
				else if (GetKeyStr == L"6")
				{
					GetKeyStr = L"F4";
				}
				else if (GetKeyStr == L"7")
				{
					GetKeyStr = L"F5";
				}
				else if (GetKeyStr == L"9")
				{
					GetKeyStr = L"F6";
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

				return RES_NG;
			}
			else if (GetKeyStr == L"F1" && cryptoCurrencies.GetCount() > 0)
			{
				if (IsAdaTransaction())
				{
					CString strTemp;
					POSITION pos = cryptoCurrencyValues.FindIndex(0);
					strTemp.Format(L"1. %s", cryptoCurrencyValues.GetAt(pos));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
				}

				pos = cryptoCurrencies.FindIndex(0);
				CDCryptoCurrency cur = cryptoCurrencies.GetAt(pos);
				_cdCryptoState.SelectedCurrency = cur.Currency;

				return RES_OK;
			}
			else if (GetKeyStr == L"F2" && cryptoCurrencies.GetCount() > 1)
			{
				if (IsAdaTransaction())
				{
					CString strTemp;
					POSITION pos = cryptoCurrencyValues.FindIndex(1);
					strTemp.Format(L"2. %s", cryptoCurrencyValues.GetAt(pos));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
				}

				pos = cryptoCurrencies.FindIndex(1);
				CDCryptoCurrency cur = cryptoCurrencies.GetAt(pos);
				_cdCryptoState.SelectedCurrency = cur.Currency;

				return RES_OK;
			}
			else if (GetKeyStr == L"F3" && cryptoCurrencies.GetCount() > 2)
			{
				if (IsAdaTransaction())
				{
					CString strTemp;
					POSITION pos = cryptoCurrencyValues.FindIndex(2);
					strTemp.Format(L"3. %s", cryptoCurrencyValues.GetAt(pos));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
				}

				pos = cryptoCurrencies.FindIndex(2);
				CDCryptoCurrency cur = cryptoCurrencies.GetAt(pos);
				_cdCryptoState.SelectedCurrency = cur.Currency;

				return RES_OK;
			}
			else if (GetKeyStr == L"F4" && cryptoCurrencies.GetCount() > 3)
			{
				if (IsAdaTransaction())
				{
					CString strTemp;
					POSITION pos = cryptoCurrencyValues.FindIndex(3);
					strTemp.Format(L"4. %s", cryptoCurrencyValues.GetAt(pos));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
				}

				pos = cryptoCurrencies.FindIndex(3);
				CDCryptoCurrency cur = cryptoCurrencies.GetAt(pos);
				_cdCryptoState.SelectedCurrency = cur.Currency;

				return RES_OK;
			}
			else if (GetKeyStr == L"F5" && cryptoCurrencies.GetCount() > 4)
			{
				if (IsAdaTransaction())
				{
					CString strTemp;
					POSITION pos = cryptoCurrencyValues.FindIndex(4);
					strTemp.Format(L"5. %s", cryptoCurrencyValues.GetAt(pos));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
				}

				pos = cryptoCurrencies.FindIndex(4);
				CDCryptoCurrency cur = cryptoCurrencies.GetAt(pos);
				_cdCryptoState.SelectedCurrency = cur.Currency;

				return RES_OK;
			}
			else if (GetKeyStr == L"F6" && cryptoCurrencies.GetCount() > 5)
			{
				if (IsAdaTransaction())
				{
					CString strTemp;
					POSITION pos = cryptoCurrencyValues.FindIndex(5);
					strTemp.Format(L"6. %s", cryptoCurrencyValues.GetAt(pos));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(strTemp, TRUE);
				}

				pos = cryptoCurrencies.FindIndex(5);
				CDCryptoCurrency cur = cryptoCurrencies.GetAt(pos);
				_cdCryptoState.SelectedCurrency = cur.Currency;

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
	NVDump('F', 'C', "-1", L"NG_2", L"CANCEL");

	return RES_NG;
}

BIZ_RETURN CTranCmn::P_NH_CD_ConfirmManualWalletAddress(CString &errorMessage)
{
	CString	GetKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	CString message = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_404000);

	if (!IsAdaTransaction())
	{
		// Show screen
		m_pDevCmn->fnSCR_DisplayPrevSet(404);
		m_pDevCmn->fnSCR_DisplayString(1, message);

		m_pDevCmn->fnSCR_DisplayScreen(404, KEYIN_TIME_OUT, PIN_MENU_MODE);
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

			CString strTemp;
			strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"404.wav"), message);
			m_pAdaCtrl->fnExp_AddWaveFile(404, strTemp);

			m_pAdaCtrl->fnExp_PlayScreenWave(404);

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
					GetKeyStr = L"CONTINUE";
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

				return RES_NG;
			}
			else if (GetKeyStr == L"CONTINUE")
			{
				if (IsAdaTransaction())
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"404_ContinueSelected.wav", TRUE);

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
	NVDump('F', 'C', "-1", L"NG_2", L"CANCEL");

	return RES_NG;
}

BIZ_RETURN CTranCmn::P_NH_CD_EnterTransactionAmount(int *transactionAmount)
{
	NHDBG((L"[CTranCmn::P_NH_CD_EnterTransactionAmount]\n"));
	NVDump('O', 'C', "2I", L"Start", L"Amount");

	CString GetKeyStr, AdaKeyStr;
	BOOL	replay = TRUE;

	*transactionAmount = 0;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	CString tempStr = Int2Asc(_cdCryptoState.DailyLimit);
	int tempStrLen = tempStr.GetLength();
	CString sMax = Int2Asc(tempStrLen);

	CString dailyLimit;
	dailyLimit.Format(L"%c%.2f", CURRENCY_SYMBOL, (double)_cdCryptoState.DailyLimit);

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(407);
		m_pDevCmn->fnSCR_DisplayString(1, dailyLimit);
		m_pDevCmn->fnSCR_DisplayString(4, L"DOLLAR");
		m_pDevCmn->fnSCR_DisplayString(5, L"INT_DOLLAR");
		m_pDevCmn->fnSCR_DisplayString(10, L"1"); // min
		m_pDevCmn->fnSCR_DisplayString(11, sMax); // max
		m_pDevCmn->fnSCR_DisplayString(12, L"off");
		m_pDevCmn->fnSCR_DisplayScreen(407, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);
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
			strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"407.wav"), dailyLimit);
			m_pAdaCtrl->fnExp_AddWaveFile(407, strTemp);

			m_pAdaCtrl->fnExp_PlayScreenWave(407);

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
				*transactionAmount = Asc2Int(sEnterVal);

				int atmAmount = *transactionAmount * 100.00;
				m_sUserSelection.strMoney = Int2Asc(atmAmount);

				if (*transactionAmount > _cdCryptoState.DailyLimit)
				{
					return RES_USER_EXIT;
				}

				NHDBG((_T("Input Amount to convert integer [%d]\n"), *transactionAmount));

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
					*transactionAmount = Asc2Int(sEnterVal);

					int atmAmount = *transactionAmount * 100.00;
					m_sUserSelection.strMoney = Int2Asc(atmAmount);

					if (*transactionAmount > _cdCryptoState.DailyLimit)
					{
						return RES_USER_EXIT;
					}

					NHDBG((_T("Input Amount to convert integer [%d]\n"), *transactionAmount));

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

BIZ_RETURN CTranCmn::P_NH_CD_InsertCard()
{
	NVDump('O', 'C', "2L", L"", L"CARD");
	NHDBG((L"[CTranCmn::P_NH_CD_InsertCard]\n"));

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
		m_pDevCmn->fnSCR_DisplayPrevSet(409);
		SetCardReadScreenValue(L"");
		m_pDevCmn->fnSCR_DisplayScreen(409, KEYIN_TIME_OUT, PIN_MENU_MODE);
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
			m_pAdaCtrl->fnExp_AddWaveFile(409, L"Please_Insert.wav");

			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
				m_pAdaCtrl->fnExp_AddWaveFile(409, L"Insertion_Direction_V.wav");
			else
				m_pAdaCtrl->fnExp_AddWaveFile(409, L"Insertion_Direction_H.wav");

			if (P_EMV_CheckTransMode() == RES_NOR_MS_TRANS)
				m_pAdaCtrl->fnExp_AddWaveFile(409, L"Smooth_remove.wav");
			else
				m_pAdaCtrl->fnExp_AddWaveFile(409, L"Donot_remove.wav");
			// End of [#2380]

			m_pAdaCtrl->fnExp_PlayScreenWave(409);

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

BIZ_RETURN CTranCmn::P_NH_CD_DebitAccount(CString &errorCode, CString &errorMessage)
{
	P_NH_CD_PleaseWait();

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

		NHERROR((L"Cash Depot debit TXN failed with error code: %s; message: %s\r\n", errorCode, errorMessage));
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

BIZ_RETURN CTranCmn::P_NH_CD_TransactionSummary()
{
	NHDBG((L"[CTranCmn::P_NH_CD_TransactionSummary]\n"));
	NVDump('O', 'C', "2P", L"Start", L"");

	CString GetKeyStr;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();

	if (nScrTimeOut == 0) // ADA Mode....Jack removed.
		return RES_NG;

	double amount = _cdCryptoState.TransactionAmount;
	double purchased = amount / _cdCryptoState.Price;

	CString cName = _cdCryptoState.CoinName;
	cName.MakeUpper();

	if (!IsAdaTransaction())
	{
		// Show screen
		m_pDevCmn->fnSCR_DisplayPrevSet(408);

		// 1 BITCOIN = $30,279.41
		CString strTmp;
		strTmp.Format(L"1 %s = %c%.2f", cName, CURRENCY_SYMBOL, _cdCryptoState.Price);
		m_pDevCmn->fnSCR_DisplayString(1, strTmp);

		// $1000
		strTmp.Format(L"%c%.2f", CURRENCY_SYMBOL, amount);
		m_pDevCmn->fnSCR_DisplayString(2, strTmp);

		m_pDevCmn->fnSCR_DisplayFormat(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_408000), cName, purchased, _cdCryptoState.CoinType);
		m_pDevCmn->fnSCR_DisplayString(4, _cdCryptoState.WalletAddress);
		m_pDevCmn->fnSCR_DisplayScreen(408, KEYIN_TIME_OUT, PIN_MENU_MODE);
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
			strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"408.wav"), cName, CURRENCY_SYMBOL, _cdCryptoState.Price, CURRENCY_SYMBOL, amount, cName, purchased, _cdCryptoState.CoinType, _cdCryptoState.WalletAddress);
			m_pAdaCtrl->fnExp_AddWaveFile(408, strTemp);

			m_pAdaCtrl->fnExp_PlayScreenWave(408);

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
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"408_ConfirmSelected.wav", TRUE);
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

BIZ_RETURN CTranCmn::P_NH_CD_Error(CString message)
{
	if (message.IsEmpty())
		message = CD_UNKNOWNERROR;

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(404);
		m_pDevCmn->fnSCR_DisplayString(1, message);
		m_pDevCmn->fnSCR_DisplayScreen(404);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

		m_pAdaCtrl->fnExp_StopAndResetWaveFile();

		CString strTemp;
		strTemp.Format(m_pAdaCtrl->fnExp_TTS_GetWaveData(L"404.wav"), message);
		m_pAdaCtrl->fnExp_AddWaveFile(404, strTemp);

		m_pAdaCtrl->fnExp_PlayScreenWave(404);

		return RES_OK;
	}

	Delay_Msg(5000);

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_Cancel()
{
	CString message = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122001);

	m_pDevCmn->fnSCR_DisplayPrevSet(122);
	m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
	m_pDevCmn->fnSCR_DisplayString(4, message);
	m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
	m_pDevCmn->fnSCR_DisplayScreen(122);

	m_pDevCmn->fstrSCR_WaitTime(INFO_SCR_TIMEOUT);

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_ReturnCard()
{
#if UNDER_CE
	// Remove the EMV card, if present
	if (m_pDevCmn->fnMCU_IsEmvTransaction())
	{
		P_EMV_RemoveCard();
	}
#endif

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_PrintReceipt()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_CD_PrintReceipt]\n"));

	CString strTemp, strTemp2, strPrintData = L"";

	m_pDevCmn->fnSCR_DisplayPrevSet(119);
	m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119003));

	// Display Arrow for HALO... (NO SPR Flicker)
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
		m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
	else
		m_pDevCmn->fnSCR_DisplayImage(1, FALSE);

	m_pDevCmn->fnSCR_DisplayScreen(119);

	// Space
	AddPrintData(strPrintData, L" ");

	// Header
	strTemp.Format(L"%s RECEIPT", GetCenteredString(_cdCryptoState.SelectedProvider.ProviderCode.MakeUpper(), SLIP_MAX_COL));
	AddPrintData(strPrintData, strTemp);

	// Timestamp
	AddPrintData(strPrintData, _cdCryptoState.Receipt.Timestamp);

	// Space
	AddPrintData(strPrintData, L" ");

	// ADDRESS 1,2,3,PHONE NUMBER PRINT
	for (int i = 0; i < 4; i++)
	{
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1 + i);
		strTemp.TrimLeft();

		if (strTemp.GetLength() > 0)
			AddPrintData(strPrintData, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1 + i));
	}

	// HEADER 1,2 PRINT
	for (int i = 0; i < 2; i++)
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

	// AUTHORIZATION ID
	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
		strTemp.Format(L"%-6.6s%-2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID));
	else // STANDARD1, 2, EPS는 Audit Num과 NetworkID 사이에 SPACE 추가
		strTemp.Format(L"%-6.6s %-2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID));

	AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_003), strTemp);

	// CARD NUMBER
	AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_006), m_sCardData.strShowNumber);
	// End of Terminal and Card information

	// TRANSACTION ID
	CString transactionId = _cdCryptoState.Receipt.Id;

	int maxLen = SLIP_MAX_COL / 2;

	if (transactionId.GetLength() > maxLen)
	{
		strTemp = transactionId.Left(maxLen);
		strTemp2 = transactionId.Mid(maxLen);
	}
	else
	{
		strTemp = transactionId;
	}

	AddPrintData(strPrintData, L"%-17.17s = %-20.20s", L"Transaction ID", strTemp);

	if (strTemp2.GetLength() > 0)
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", L"", strTemp2);

	AddPrintData(strPrintData, L"%-17.17s = %-20.20s", L"Cash", _cdCryptoState.Receipt.Total);
	AddPrintData(strPrintData, L"%-17.17s = %-20.20s", _cdCryptoState.CoinName, _cdCryptoState.Receipt.Rates.Crypto);

	// WALLET ADDRESS
	CString strQRString = _cdCryptoState.WalletAddress;

	if (strQRString.GetLength() > maxLen)
	{
		strTemp = strQRString.Left(maxLen);
		strTemp2 = strQRString.Mid(maxLen);
	}
	else
	{
		strTemp = strQRString;
	}

	AddPrintData(strPrintData, L"%-17.17s = %-20.20s", L"Address", strTemp);

	if (strTemp2.GetLength() > 0)
		AddPrintData(strPrintData, L"%-17.17s = %-20.20s", L"", strTemp2);

	// Space
	AddPrintData(strPrintData, L" ");

	AddPrintData(strPrintData, L"<BARCODE_QR>%s</BARCODE_QR>", strQRString);
	
	// Space
	AddPrintData(strPrintData, L" ");

	// TAIL PRINT
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER3);
	strTemp.TrimLeft();

	if (strTemp.GetLength() > 0)
	{
		AddPrintData(strPrintData, L" ");
		AddPrintData(strPrintData, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER3));
	}

	AddPrintData(strPrintData, L" ");
	AddPrintData(strPrintData, L" For transaction support please contact");
	AddPrintData(strPrintData, L"           support@coinsource.net");
	AddPrintData(strPrintData, L" ");
	AddPrintData(strPrintData, L"                        All Sales Final");
	AddPrintData(strPrintData, L"       Terms of Service and Delivery at");
	AddPrintData(strPrintData, L"                         coinsource.net");
	// End of TAIL PRINT

	if (m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT, TRUE, TRUE) != TRUE) // NO Header Image, Data, Wait, Cut, Print BCD
		return RES_NG;

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_GetProviders(CDProvidersResponse &providers, CString &errorMessage)
{
	P_NH_CD_PleaseWait();

	if (!m_CDService->GetProviders(providers))
	{
		return RES_NG;
	}

	if (providers.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_GetProviders] Error: %s\n", providers.GetError()));

		errorMessage = providers.GetMessage();

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_GetClientToken(CDClientTokenResponse &clientToken, CString &errorMessage)
{
	CDClientTokenRequest request;
	request.ServiceProvider = _cdCryptoState.SelectedProvider.ProviderCode;

	P_NH_CD_PleaseWait();

	if (!m_CDService->GetClientToken(request, clientToken))
	{
		return RES_NG;
	}

	if (clientToken.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_GetClientToken] Error: %s\n", clientToken.GetError()));

		errorMessage = clientToken.GetMessage();

		return RES_NG;
	}

	return RES_OK;
}

CTranCmn::CDGreetProcID CTranCmn::P_NH_CD_SendCode(CString phoneNumber, CString &errorMessage)
{
	CDSendCodeRequest request;
	request.Token = _cdCryptoState.AccessToken;
	request.TerminalId = _cdCryptoState.TerminalId;
	request.ServiceProvider = _cdCryptoState.SelectedProvider.ProviderCode;
	request.Phone = phoneNumber;

	CDSendCodeResponse response;

	P_NH_CD_PleaseWait();
	
	if (!m_CDService->SendCode(request, GenerateUUIDv4(), response))
	{
		NHERROR((L"Greet failed\r\n"));

		return CDGP_Fail;
	}

	if (response.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_SendCode] Error: %s\n", response.GetError()));

		errorMessage = response.GetMessage();
	}

	switch (response.StatusCode)
	{
		case CDR_OK:
			return CDGP_ConfirmCode;
		default:
			return CDGP_Fail;
	}
}

CTranCmn::CDGreetProcID CTranCmn::P_NH_CD_VerifyCode(CString phoneNumber, CString code, CDVerifyCodeResponse &verifyCode, CString &errorMessage)
{
	CDVerifyCodeRequest request;
	request.Token = _cdCryptoState.AccessToken;
	request.TerminalId = _cdCryptoState.TerminalId;
	request.ServiceProvider = _cdCryptoState.SelectedProvider.ProviderCode;
	request.Phone = phoneNumber;
	request.Code = code;

	P_NH_CD_PleaseWait();

	if (!m_CDService->VerifyCode(request, GenerateUUIDv4(), verifyCode))
	{
		NHERROR((L"Greet failed\r\n"));

		return CDGP_Fail;
	}

	if (verifyCode.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_VerifyCode] Error: %s\n", verifyCode.GetError()));

		errorMessage = verifyCode.GetMessage();

		return CDGP_Fail;
	}

	switch (verifyCode.StatusCode)
	{
		case CDR_OK:
			return CDGP_Complete;
		default:
			return CDGP_Fail;
	}
}

BIZ_RETURN CTranCmn::P_NH_CD_Disclosure(CString &errorMessage, CDDisclosureResponse &disclosure)
{
	CDDisclosureRequest request;
	request.Token = _cdCryptoState.AccessToken;
	request.TerminalId = _cdCryptoState.TerminalId;
	request.ServiceProvider = _cdCryptoState.SelectedProvider.ProviderCode;

	if (!m_CDService->Disclosure(request, disclosure))
	{
		return RES_NG;
	}

	if (disclosure.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_Disclosure] Error: %s\n", disclosure.GetError()));

		errorMessage = disclosure.GetMessage();

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_DisclosureAccept(CString &errorMessage, CDDisclosureResponse &disclosure, CDDisclosureAcceptResponse &disclosureAccept)
{
	CDDisclosureAcceptRequest request;
	request.Token = _cdCryptoState.AccessToken;
	request.TerminalId = _cdCryptoState.TerminalId;
	request.ServiceProvider = _cdCryptoState.SelectedProvider.ProviderCode;
	request.UserToken = _cdCryptoState.UserToken;
	request.CustomerId = _cdCryptoState.CustomerId;

	int count = disclosure.Disclosures.GetCount();

	for (size_t i = 0; i < count; i++)
	{
		POSITION pos = disclosure.Disclosures.FindIndex(i);
		CDDisclosure dis = disclosure.Disclosures.GetAt(pos);
		request.Disclosures.AddTail(dis);
	}

	P_NH_CD_PleaseWait();

	if (!m_CDService->DisclosureAccept(request, disclosureAccept))
	{
		return RES_NG;
	}

	if (disclosureAccept.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_DisclosureAccept] Error: %s\n", disclosureAccept.GetError()));

		errorMessage = disclosureAccept.GetMessage();

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_DailyLimit(CString &errorMessage, CDDailyLimitResponse &dailyLimit)
{
	CDDailyLimitRequest request;
	request.Token = _cdCryptoState.AccessToken;
	request.ServiceProvider = _cdCryptoState.SelectedProvider.ProviderCode;
	request.DisclosureToken = _cdCryptoState.DisclosureToken;

	if (!m_CDService->DailyLimit(request, GenerateUUIDv4(), dailyLimit))
	{
		return RES_NG;
	}

	if (dailyLimit.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_DailyLimit] Error: %s\n", dailyLimit.GetError()));

		errorMessage = dailyLimit.GetMessage();

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_CryptoCurrency(CString &errorMessage, CDCryptoCurrencyResponse &cryptoCurrency)
{
	CDCryptoCurrencyRequest request;
	request.Token = _cdCryptoState.AccessToken;
	request.TerminalId = _cdCryptoState.TerminalId;
	request.ServiceProvider = _cdCryptoState.SelectedProvider.ProviderCode;
	request.DisclosureToken = _cdCryptoState.DisclosureToken;

	if (!m_CDService->CryptoCurrency(request, GenerateUUIDv4(), cryptoCurrency, _cdCryptoState.Currency))
	{
		return RES_NG;
	}

	if (cryptoCurrency.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_CryptoCurrency] Error: %s\n", cryptoCurrency.GetError()));

		errorMessage = cryptoCurrency.GetMessage();

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_CryptoCurrencyPrice(CString &errorMessage, CDCryptoCurrencyPriceResponse &cryptoCurrencyPrice)
{
	CDCryptoCurrencyPriceRequest request;
	request.Token = _cdCryptoState.AccessToken;
	request.TerminalId = _cdCryptoState.TerminalId;
	request.ServiceProvider = _cdCryptoState.SelectedProvider.ProviderCode;
	request.DisclosureToken = _cdCryptoState.DisclosureToken;

	P_NH_CD_PleaseWait();

	if (!m_CDService->CryptoCurrencyPrice(request, GenerateUUIDv4(), cryptoCurrencyPrice, _cdCryptoState.SelectedCurrency))
	{
		return RES_NG;
	}

	if (cryptoCurrencyPrice.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_CryptoCurrencyPrice] Error: %s\n", cryptoCurrencyPrice.GetError()));

		errorMessage = cryptoCurrencyPrice.GetMessage();

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_SendManualWalletLink(CString &errorMessage)
{
	CDSendManualWalletLinkRequest request;
	request.Token = _cdCryptoState.AccessToken;
	request.ServiceProvider = _cdCryptoState.SelectedProvider.ProviderCode;
	request.DisclosureToken = _cdCryptoState.DisclosureToken;
	request.CoinType = _cdCryptoState.CoinType;

	CDSendManualWalletLinkResponse response;

	if (!m_CDService->SendManualWalletLink(request, GenerateUUIDv4(), response))
	{
		return RES_NG;
	}

	if (response.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_SendManualWalletLink] Error: %s\n", response.GetError()));

		errorMessage = response.GetMessage();

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_GetManualWalletAddress(CString &errorMessage, CDGetManualWalletAddressResponse &getManualWalletAddress)
{
	CDGetManualWalletAddressRequest request;
	request.Token = _cdCryptoState.AccessToken;
	request.DisclosureToken = _cdCryptoState.DisclosureToken;

	P_NH_CD_PleaseWait();

	if (!m_CDService->GetManualWalletAddress(request, GenerateUUIDv4(), getManualWalletAddress))
	{
		return RES_NG;
	}

	if (getManualWalletAddress.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_GetClientToken] P_NH_CD_GetManualWalletAddress: %s\n", getManualWalletAddress.GetError()));

		errorMessage = getManualWalletAddress.GetMessage();

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_CheckManualWalletAddress(CString &errorMessage, CDCheckManualWalletAddressResponse &checkManualWalletAddress)
{
	CDCheckManualWalletAddressRequest request;
	request.Token = _cdCryptoState.AccessToken;
	request.ServiceProvider = _cdCryptoState.SelectedProvider.ProviderCode;
	request.DisclosureToken = _cdCryptoState.DisclosureToken;
	request.CoinType = _cdCryptoState.CoinType;
	request.WalletAddress =_cdCryptoState.WalletAddress;

	if (!m_CDService->CheckManualWalletAddress(request, GenerateUUIDv4(), checkManualWalletAddress))
	{
		return RES_NG;
	}

	if (checkManualWalletAddress.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_GetClientToken] P_NH_CD_CheckManualWalletAddress: %s\n", checkManualWalletAddress.GetError()));

		errorMessage = checkManualWalletAddress.GetMessage();

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_Buy(CString &errorMessage, CDBuyResponse &buy)
{
	CDBuyRequest request;
	request.Token = _cdCryptoState.AccessToken;
	request.TerminalId = _cdCryptoState.TerminalId;
	request.ServiceProvider = _cdCryptoState.SelectedProvider.ProviderCode;
	request.DisclosureToken = _cdCryptoState.DisclosureToken;
	request.CoinType = _cdCryptoState.CoinType;
	request.WalletAddress = _cdCryptoState.WalletAddress;
	request.QuoteId = _cdCryptoState.QuoteId;
	request.CashTotal = Int2Asc(_cdCryptoState.TransactionAmount);
	request.Phone = L"";
	request.Code = L"";

	if (!m_CDService->Buy(request, GenerateUUIDv4(), buy))
	{
		return RES_NG;
	}

	if (buy.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_Buy] Error: %s\n", buy.GetError()));

		errorMessage = buy.GetMessage();

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_CD_BuyComplete(CString &errorMessage, CDBills &bills, CDBuyCompleteResponse &buyComplete)
{
	CDBuyCompleteRequest request;
	request.Token = _cdCryptoState.AccessToken;
	request.TerminalId = _cdCryptoState.TerminalId;
	request.ServiceProvider = _cdCryptoState.SelectedProvider.ProviderCode;
	request.DisclosureToken = _cdCryptoState.DisclosureToken;
	request.WalletAddress = _cdCryptoState.WalletAddress;
	request.WalletTransactionId = _cdCryptoState.WalletTransactionId;
	request.QuoteId = _cdCryptoState.QuoteId;
	request.CashTotal = Int2Asc(_cdCryptoState.TransactionAmount);
	request.Code = L"";

	int count = bills.GetCount();

	for (size_t i = 0; i < count; i++)
	{
		POSITION pos = bills.FindIndex(i);
		CDBill bill = bills.GetAt(pos);
		request.Bills.AddTail(bill);
	}

	P_NH_CD_PleaseWait();

	if (!m_CDService->BuyComplete(request, GenerateUUIDv4(), buyComplete))
	{
		return RES_NG;
	}

	if (buyComplete.HasError())
	{
		NHDBG((L"[CTranCmn::P_NH_CD_BuyComplete] Error: %s\n", buyComplete.GetError()));

		errorMessage = buyComplete.GetMessage();

		return RES_NG;
	}

	return RES_OK;
}

#endif