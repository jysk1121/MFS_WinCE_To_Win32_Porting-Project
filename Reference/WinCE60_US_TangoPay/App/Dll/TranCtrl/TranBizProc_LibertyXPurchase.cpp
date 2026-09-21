#include "stdafx.h"
#include ".\Scr\ScrCtrl.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"

#include ".\Common\NHDbgApi.h"
#include ".\Tran\LibertyXTypes.h"

#if (APP_LIBERTYX)

extern CTimeCheck	g_TimeCheck;


BIZ_RETURN CTranCmn::P_NH_LTXP_OrderID(CString &orderId)
{
	NVDump('O', 'C', "21", L"", L"ODERID");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_LibertyXOrderID]\n"));

	CString			GetKeyStr;

	// Show screen
	m_pDevCmn->fnSCR_DisplayPrevSet(350);
	m_pDevCmn->fnSCR_DisplayScreen(350, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);

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
				NVDump('O', 'C', "21", L"NG_0", GetKeyStr);

				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));

				NVDump('F', 'C', "21", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else
			{
				orderId = GetKeyStr;
				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	NVDump('O', 'C', "21", L"NG_3", L"CANCEL");

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_OrderID_ADA(CString &orderId)
{
	NVDump('O', 'C', "21", L"", L"ODERIDADA");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_LibertyXOrderID_ADA]\n"));

	CString	GetKeyStr;
	CString input;
	BOOL	invalidRepeat = FALSE;
	BOOL	replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if (nScrTimeOut == 0)	
	{   // ADA Mode....Jack removed.
		return RES_NG;
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "21", L"NG_2", L"");
			m_pAdaCtrl->fnExp_StopPlay();
			return RES_NG;
		}

		if (invalidRepeat && m_pAdaCtrl->IsPlaying() == FALSE )
		{
			invalidRepeat = FALSE;
			replay = TRUE;
		}

		if (replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(350, L"EnterLTXToken.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(350, L"ClearTokeKey.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(350, L"LocateCancelKey.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(350, L"LocateEnterKey.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(350, L"LocationRepeatKey.wav");	
			m_pAdaCtrl->fnExp_PlayScreenWave(350);

			replay = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "21", L"NG_0", GetKeyStr);

				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));

				NVDump('F', 'C', "21", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_CLEAR)
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Clear.wav", TRUE);
				input = L"";
			}
			else if (GetKeyStr == L"." || GetKeyStr == L"00" || GetKeyStr == L"000")  // Repeat or Volume Change
			{
				m_pAdaCtrl->fnExp_StopPlay();

				// Handle volume change repeats
				if(GetKeyStr == L".")
					m_pAdaCtrl->fnExp_SetVolumeDown();
				else if(GetKeyStr == L"00")
					m_pAdaCtrl->fnExp_SetVolumeUp();

				// Repeat after a volume change
				replay = TRUE;
			}
			else if (GetKeyStr == S_ENTER)
			{
				if (input.GetLength() <= 4) 
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay("362_TokenTooShort.wav", TRUE);
				}
				else if (input.GetLength() > 12)
				{
					m_pAdaCtrl->fnExp_ResetAndAddPlay("362_TokenTooLong.wav", TRUE);
				}
				else 
				{
					orderId = input;
					NVDump('O', 'C', "21", L"TOK", orderId);
					return RES_OK;
				}

				replay = TRUE;
				input = L"";
			}
			else if (GetKeyStr.GetLength() == 1)
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay(GetKeyStr + L".wav", TRUE);
				input += GetKeyStr;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	return RES_USER_TIMEOUT;
}


BIZ_RETURN CTranCmn::P_NH_LTXP_Amount(double &amount)
{
	NVDump('O', 'C', "22", L"", L"AMT");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_LibertyXAmount]\n"));

	CString			GetKeyStr;

	// Show screen
	m_pDevCmn->fnSCR_DisplayPrevSet(351);
	// Input Parameters
	m_pDevCmn->fnSCR_DisplayString(20, L"DOLLAR");		// Symbol
	m_pDevCmn->fnSCR_DisplayString(21, L"INT_CENT");	// Amount Type
	m_pDevCmn->fnSCR_DisplayString(22, L"1");			// min
	m_pDevCmn->fnSCR_DisplayString(23, L"6");			// max
	m_pDevCmn->fnSCR_DisplayString(24, L"off");			// auto run
	m_pDevCmn->fnSCR_DisplayScreen(351, KEYIN_TIME_OUT, PIN_AMOUNT_MODE);

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
				NVDump('O', 'C', "22", L"NG_0", GetKeyStr);

				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));

				NVDump('F', 'C', "22", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else
			{
				CString sEnterVal(GetKeyStr);
				sEnterVal.Replace(GetCurrencySymbol(), L"");
				sEnterVal.Trim();

				// Set the amount for the debit transaction
				m_sUserSelection.strMoney = sEnterVal;

				// Set the amount for the LTX transaction
				amount = (double) Asc2Int(sEnterVal) / 100.0;

				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	NVDump('F', 'C', "22", L"NG_2", L"CANCEL");

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_Amount_ADA(double &amount)
{
	NVDump('O', 'C', "22", L"", L"AMTADA");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::F_NH_LibertyXAmount_ADA]\n"));

	CString	GetKeyStr;
	CString input;
	BOOL	replay = TRUE;

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "22", L"P_NHADA", L"NG_122");
			m_pAdaCtrl->fnExp_StopPlay();
			return RES_USER_EXIT;
		}

		if (replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();

#if (US_VERSION)
			m_pAdaCtrl->fnExp_AddWaveFile(351, L"351.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(351, L"LocateEnterKey.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(351, L"ClearLocationKey.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(351, L"LocateCancelKey.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(351, L"LocationRepeatKey.wav");	
#endif

			m_pAdaCtrl->fnExp_PlayScreenWave(351);
			replay = FALSE;
			input = L"";
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "22", L"NG_0", GetKeyStr);
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);

				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));

				NVDump('F', 'C', "22", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == S_CLEAR)
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Clear.wav", TRUE);
				input = L"";
			}
			else if (GetKeyStr == S_ENTER)
			{
				input.Trim();
				int intAmount = Asc2Int(input);
				// Set the amount for the debit transaction
				m_sUserSelection.strMoney = input;
				// Set the amount for the LTX transaction
				amount = (double) intAmount;

				if (P_NH_ADA_ConfirmAmt(intAmount) == RES_USER_PREV) 
				{
					replay = TRUE;
					input = L"";
					continue;
				}

				return RES_OK;
			}
			else if (GetKeyStr == L"." || GetKeyStr == L"00" || GetKeyStr == L"000")  // Repeat or Volume Change
			{
				m_pAdaCtrl->fnExp_StopPlay();

				// Handle volume change repeats
				if(GetKeyStr == L".")
					m_pAdaCtrl->fnExp_SetVolumeDown();
				else if(GetKeyStr == L"00")
					m_pAdaCtrl->fnExp_SetVolumeUp();

				// Repeat after a volume change
				replay = TRUE;
			}
			else if (GetKeyStr.GetLength() == 1)
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay(GetKeyStr + L".wav", TRUE);
				input += GetKeyStr;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	NVDump('F', 'C', "22", L"NG_2", L"CANCEL");

	return RES_USER_TIMEOUT;
}

BIZ_RETURN	CTranCmn::P_NH_LTXP_InsertCard()
{
	NVDump('O', 'C', "23", L"", L"CARD");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXD_InsertCard]\n"));

	CString			GetKeyStr;
	BOOL			bShowScreen = TRUE;

	m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);
	m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_ON);

	m_pDevCmn->fnSCR_DisplayPrevSet(352);
	m_pDevCmn->fnSCR_DisplayScreen(352, KEYIN_TIME_OUT, PIN_MENU_MODE);	

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// GET CARD EVENT
		if ((DidMediaExist() == FALSE) && (LIB_MainMenuDeviceEvent() == DEV_MCU) && (m_pDevCmn->fnMCU_GetDeviceStatus() == NORMAL))	
		{
			TurnOffCardReaderFlicker();

			BIZ_RETURN cardReadResult = VATReadCard();
			if (cardReadResult != RES_OK)
			{
				NHERROR((L"Card read failed\r\n"));

				CString result;
				result.Format(L"%d", cardReadResult);
				NVDump('F', 'C', "23", L"NG_0", result);
				return RES_NG;
			}

			return RES_OK;
		}
		else if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)			// GET KEY STRING
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NHDEBUG(DBG_INFO, (_T("CANCEL OR EXIT BUTTON PRESSED\n")));
				TurnOffCardReaderFlicker(TRUE);		// [#2388] US Justin
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				TurnOffCardReaderFlicker(TRUE);		// [#2388] US Justin
				return RES_USER_TIMEOUT;
			}
		}

		Delay_Msg(50);
	}

	NVDump('F', 'C', "23", L"NG_2", GetKeyStr);

	return RES_USER_TIMEOUT;
}

BIZ_RETURN	CTranCmn::P_NH_LTXP_InsertCard_ADA()
{
	NVDump('O', 'C', "23", L"", L"CARDADA");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXD_InsertCard_ADA]\n"));

	CString	GetKeyStr;
	BOOL	replay = TRUE;

	m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if (nScrTimeOut == 0)	
	{   // ADA Mode....Jack removed.
		return RES_NG;
	}	

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "23", L"NG_2", L"");
			m_pAdaCtrl->fnExp_StopPlay();
			return RES_NG;
		}

		if (replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			
			m_pAdaCtrl->fnExp_AddWaveFile(352, L"Familiar_ATM.wav");
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")			m_pAdaCtrl->fnExp_AddWaveFile(352, L"Insertion_Direction_V.wav");
			else																						m_pAdaCtrl->fnExp_AddWaveFile(352, L"Insertion_Direction_H.wav");
			if (P_EMV_CheckTransMode() == RES_NOR_MS_TRANS)		m_pAdaCtrl->fnExp_AddWaveFile(352, L"Smooth_remove.wav");
			else												m_pAdaCtrl->fnExp_AddWaveFile(352, L"Donot_remove.wav");

			// Card Location
			if		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1500SE")		// [#2380] US Justin 2015.12.09 Cardtronics Additional VG change
				m_pAdaCtrl->fnExp_AddWaveFile(352, L"Card_Location_1500.wav"); 	
			else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1800SE")
				m_pAdaCtrl->fnExp_AddWaveFile(352, L"Card_Location_1800.wav");
			else
				m_pAdaCtrl->fnExp_AddWaveFile(352, L"Card_Location_Other.wav");

			// Card Insertion
			// [#2380] US Justin 2015.12.21 Cardtronics Additional VG
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")			
			{
				m_pAdaCtrl->fnExp_AddWaveFile(352, L"Insertion_Vertical.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(352, L"Insertion_Direction_V.wav");
			}
			else
			{
				m_pAdaCtrl->fnExp_AddWaveFile(352, L"Insertion_Horizontal.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(352, L"Insertion_Direction_H.wav");
			}
			if(P_EMV_CheckTransMode() == RES_EMV_IC_FIRST)		
				m_pAdaCtrl->fnExp_AddWaveFile(352, L"Donot_remove_yourCard.wav");

			m_pAdaCtrl->fnExp_PlayScreenWave(352);

			replay = FALSE;
		}

		///////////////////////////////////
		// GET CARD EVENT
		if ((DidMediaExist() == FALSE) && (LIB_MainMenuDeviceEvent() == DEV_MCU) && (m_pDevCmn->fnMCU_GetDeviceStatus() == NORMAL))	
		{
			TurnOffCardReaderFlicker();
			m_pAdaCtrl->fnExp_StopPlay();

			BIZ_RETURN cardReadResult = VATReadCard();
			if (cardReadResult != RES_OK)
			{
				NHERROR((L"Card read failed\r\n"));

				CString result;
				result.Format(L"%d", cardReadResult);
				NVDump('F', 'C', "23", L"NG_0", result);
				return RES_NG;
			}

			// This has to be done to set the amount on the chip?
			if(m_pDevCmn->fnMCU_IsEmvTransaction())
			{
				cardReadResult = P_NH_NOR_EMV_IC_Processing();
				if (cardReadResult != RES_OK)
				{
					CString result;
					result.Format(L"%d", cardReadResult);
					NVDump('F', 'C', "23", L"NG_1", result);
					return RES_NG;
				}
			}

			return RES_OK;
		}
		else if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)			// GET KEY STRING
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "23", L"NG_0", GetKeyStr);

				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);

				return RES_NG;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));

				NVDump('F', 'C', "23", L"NG_1", GetKeyStr);

				return RES_USER_TIMEOUT;
			}
			else if (GetKeyStr == L"." || GetKeyStr == L"00" || GetKeyStr == L"000")  // Repeat or Volume Change
			{
				m_pAdaCtrl->fnExp_StopPlay();

				// Handle volume change repeats
				if(GetKeyStr == L".")
					m_pAdaCtrl->fnExp_SetVolumeDown();
				else if(GetKeyStr == L"00")
					m_pAdaCtrl->fnExp_SetVolumeUp();

				// Repeat after a volume change
				replay = TRUE;
			}
		}

		Delay_Msg(50);
	}

	NVDump('F', 'C', "23", L"NG_2", GetKeyStr);

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_TransactionSelection()
{
	NVDump('O', 'C', "2E", L"", L"SEL");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_TransactionSelection]\n"));

	CString keyString;

	m_pDevCmn->fnSCR_DisplayPrevSet(359);
	m_pDevCmn->fnSCR_DisplayScreen(359, KEYIN_TIME_OUT, PIN_MENU_MODE);

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			keyString = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (keyString == S_CANCEL || keyString == S_EXIT)
			{
				NVDump('O', 'C', "2E", L"NG_0", keyString);

				return RES_USER_EXIT;
			}
			else if (keyString == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));

				NVDump('F', 'C', "2E", L"NG_1", keyString);

				return RES_USER_TIMEOUT;
			}
			else if (keyString == "BTC")
			{
				return RES_LIBERTYX_BTC;
			}
			else if (keyString == "BILLPAY")
			{
				return RES_LIBERTYX_BILLPAY;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	NVDump('F', 'C', "2E", L"NG_2", L"CANCEL");

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_TransactionSelection_ADA()
{
	NVDump('O', 'C', "2E", L"", L"SELADA");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_TransactionSelection_ADA]\n"));

	CString keyString;
	BOOL replay = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if (nScrTimeOut == 0)	
	{   // ADA Mode....Jack removed.
		return RES_NG;
	}

	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "2E", L"P_NHADA", L"NG_0");
			m_pAdaCtrl->fnExp_StopPlay();
			return RES_USER_EXIT;
		}

		if (replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			m_pAdaCtrl->fnExp_AddWaveFile(359, L"359.wav");
			m_pAdaCtrl->fnExp_PlayScreenWave(359);
			replay = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			keyString = m_pDevCmn->fstrSCR_GetKeyString(1);
			keyString = keyString.Mid(6);

			if (keyString == S_CANCEL || keyString == S_EXIT)
			{
				NVDump('O', 'C', "2E", L"NG_0", keyString);
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);

				return RES_USER_EXIT;
			}
			else if (keyString == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));

				NVDump('F', 'C', "2E", L"NG_1", keyString);

				return RES_USER_TIMEOUT;
			}
			else if (keyString == L"." || keyString == L"00" || keyString == L"000")  // Repeat or Volume Change
			{
				m_pAdaCtrl->fnExp_StopPlay();

				// Handle volume change repeats
				if(keyString == L".")
					m_pAdaCtrl->fnExp_SetVolumeDown();
				else if(keyString == L"00")
					m_pAdaCtrl->fnExp_SetVolumeUp();

				// Repeat after a volume change
				replay = TRUE;
			}
			else if (keyString == "1")
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"359_1.wav", TRUE);
				return RES_LIBERTYX_BILLPAY;
			}
			else if (keyString == "2")
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"359_2.wav", TRUE);
				return RES_LIBERTYX_BTC;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	NVDump('F', 'C', "2E", L"NG_2", L"CANCEL");

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_Confirm()
{
	NVDump('O', 'C', "2F", L"", L"CFM");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_Confirm]\n"));

	CString keyString;
	m_pDevCmn->fnSCR_DisplayPrevSet(361);
	m_pDevCmn->fnSCR_DisplayScreen(361, KEYIN_TIME_OUT, PIN_MENU_MODE);

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			keyString = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (keyString == S_TIMEOVER || keyString == S_CANCEL)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));

				NVDump('F', 'C', "2F", L"NG_1", keyString); // TODO

				return RES_USER_EXIT;
			}
			else if (keyString == S_ENTER)
			{
				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	return RES_USER_EXIT;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_Confirm_ADA()
{
	NVDump('O', 'C', "2F", L"", L"CFMADA");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_Confirm_ADA]\n"));
	return PlayLibertyXADAMessage(361);
}

BIZ_RETURN CTranCmn::P_NH_LTXP_Authorize(LXAuthorization auth, LXAuthorizationResponse &response)
{
	NVDump('O', 'C', "30", L"", L"AUTH");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_Authorize_ADA]\n"));

	if (IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
		m_pAdaCtrl->fnExp_AddWaveFile(354, L"354.wav");
		m_pAdaCtrl->fnExp_PlayScreenWave(354);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(354);
		m_pDevCmn->fnSCR_DisplayScreen(354);
	}
		
	//
	// Authorize the LX transaction
	//

	bool authorizeCallResult = m_LXService->AuthorizePayment(auth, response);
	if (!authorizeCallResult || response.HasError())
	{
		NHERROR((L"LTX authorization failed!\r\n"));
		NVDump('F', 'C', "30", L"NG_0", L"");

		if (response.HasError())
		{
			NHDEBUG(DBG_CALL, (L"LTX authorization failed with error: %s; code: %s\r\n", response.Message, response.Status));
			NVDump('F', 'C', "30", L"NG_1", response.Message);
		}

		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_Debit(CString &errorCode, CString &errorMessage)
{
	NVDump('O', 'C', "31", L"", L"DEBIT");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_Debit]\n"));

	TranCode = TC_WITHDRAWAL;
	
	if (this->m_LXConfig.HostDepModeEnabled)
	{
		m_HostConfig = HC_LTX_HOST_DEP_MODE;
	}
	else
	{
		m_HostConfig = HC_LIBERTYX;
	}

	if (!IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(354);
		m_pDevCmn->fnSCR_DisplayScreen(354);
	}
	

	//
	// Do host processor transaction
	//

	BIZ_RETURN debitHostResult;
	if ((debitHostResult = P_NH_NOR_Transaction(IsAdaTransaction())) != RES_OK) // Use the standard ADA stuffs
	{
		errorCode = m_pDevCmn->fstrAPL_GetErrorCode();
		errorMessage = m_pDevCmn->fstrAPL_GetErrorMessage();

		NHERROR((L"LTX debit TXN failed with error code: %s; message: %s\r\n", errorCode, errorMessage));
		NVDump('F', 'C', "31", L"NG_3", errorCode);

		// Save the journal error
		switch (debitHostResult)
		{
		case RES_HOST_SEND_ERR:	m_pDevCmn->m_JNLMgr.Save(COMMUNICATION_ERROR);	break;
		case RES_HOST_RECV_ERR: m_pDevCmn->m_JNLMgr.Save(TRX_ERROR);			break;
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

BIZ_RETURN CTranCmn::P_NH_LTXP_Cancel(LXCancel cancel, LXCancelResponse &response, bool reverseAtmDebit)
{
	NVDump('O', 'C', "32", L"", L"CANCEL");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_Cancel]\n"));

	if (IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
		m_pAdaCtrl->fnExp_AddWaveFile(354, L"354.wav");
		m_pAdaCtrl->fnExp_PlayScreenWave(354);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(354);
		m_pDevCmn->fnSCR_DisplayScreen(354);
	}

	if (m_LXService->CancelPayment(cancel, response))
	{
		NHDEBUG(DBG_INFO, (L"TXN cancellation completed\r\n"));
		NVDump('F', 'C', "32", L"NG_4", L"");

		if (reverseAtmDebit)
		{
			TranCode = TC_REVERSAL;

			if (this->m_LXConfig.HostDepModeEnabled)
			{
				m_HostConfig = HC_LTX_HOST_DEP_MODE;
			}
			else
			{
				m_HostConfig = HC_LIBERTYX;
			}

			BIZ_RETURN reversalStatus = ReverseTransaction();

			if (reversalStatus != RES_OK)
			{
				NHERROR((L"Reversal attempts failed for LTX\r\n"));
				NVDump('F', 'C', "32", L"NG_5", m_pDevCmn->fstrAPL_GetErrorCode());
			}
		}
	}
	else
	{
		NHERROR((L"LTX transaction cancellation failed\r\n"));
		NVDump('F', 'C', "32", L"NG_6", L"");
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_Execute(LXExecute execute, LXExecuteResponse &executeResponse)
{
	NVDump('O', 'C', "33", L"", L"EXEC");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_Execute]\n"));

	if (IsAdaTransaction())
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
		m_pAdaCtrl->fnExp_AddWaveFile(354, L"354.wav");
		m_pAdaCtrl->fnExp_PlayScreenWave(354);
	}
	else
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(354);
		m_pDevCmn->fnSCR_DisplayScreen(354);
	}

	//
	// Execute the LX transaction
	//

	bool executeCallResult = this->m_LXService->ExecutePayment(execute, executeResponse);
	if (!executeCallResult || executeResponse.HasError())
	{
		NHERROR((L"LTX Execution failed for payment: %s!!\r\n", execute.PaymentId));
		NVDump('F', 'C', "33", L"NG_9", L"");

		if (executeResponse.HasError())
		{
			NHDEBUG(DBG_CALL, (L"Execute failed with error message: %s; type: %s\r\n", executeResponse.Message, executeResponse.ErrorType));
			NVDump('F', 'C', "33", L"NG_10", executeResponse.Message);
			NVDump('F', 'C', "33", L"NG_10", executeResponse.ErrorType);
		}

		return RES_NG;
	}

	// Dispense (bitcoin)
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT, m_sUserSelection.strMoney);

	// No reversal on boot!!!
	MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

	NVDump('O', 'C', "33", L"", L"SUCCESS");

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_Success()
{
	NVDump('O', 'C', "34", L"", L"SUCC");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_Success]\n"));

	m_pDevCmn->fnSCR_DisplayPrevSet(355);
	m_pDevCmn->fnSCR_DisplayScreen(355);	

	Delay_Msg(5000); // Show screen for 5 seconds

	// Remove the EMV card, if present
	if (m_pDevCmn->fnMCU_IsEmvTransaction())
	{
		P_EMV_RemoveCard();	
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_Success_ADA()
{
	NVDump('O', 'C', "34", L"", L"SUCCADA");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_Success_ADA]\n"));

	// Remove the EMV card, if present
	if (m_pDevCmn->fnMCU_IsEmvTransaction())
	{
		P_EMV_RemoveCard();	
	}

	m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
	m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
	m_pAdaCtrl->fnExp_AddWaveFile(355, L"355.wav");
	m_pAdaCtrl->fnExp_PlayScreenWave(355);
	m_pAdaCtrl->fnExp_WaitUntilStop();

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_Receipt()
{
	NVDump('O', 'C', "35", L"", L"RCPT");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_Receipt]\n"));

	CString receiptData = this->m_LXService->GetReceiptData();

	if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
	{
		// Print standard TXN receipt, but don't cut the tape
		if (!P_NH_NOR_TransactionPrint(FALSE) && !m_pDevCmn->fnSPR_PrintReceipt(false, receiptData, K_1_WAIT))
		{
			NHERROR((L"Printer failed to print LTX receipt\r\n"));
			return RES_NG;
		}

		return RES_OK;
	}
	else
	{
		// Do digital receipt
		return RES_NG;
	}
}

BIZ_RETURN CTranCmn::P_NH_LTXP_Receipt_ADA()
{
	NVDump('O', 'C', "36", L"", L"RCPTADA");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_Receipt_ADA]\n"));

	CString receiptData = this->m_LXService->GetReceiptData();

	if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
	{
		// Print standard TXN receipt, but don't cut the tape
		if (!P_NH_NOR_TransactionPrint(FALSE) && !m_pDevCmn->fnSPR_PrintReceipt(false, receiptData, K_1_WAIT))
		{
			NHERROR((L"Printer failed to print LTX receipt\r\n"));
			return RES_NG;
		}

		return RES_OK;
	}
	else
	{
		// Do digital receipt
		return RES_NG;
	}

	m_pAdaCtrl->fnExp_StopAndResetWaveFile();
	m_pAdaCtrl->fnExp_AddWaveFile(1, L"Takereceipt.wav");
	if		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1500SE")
		m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_Location_1500.wav"); 	
	else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1800SE")
		m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_Location_1800.wav");
	else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
		m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_Location_4000.wav");
	else
		m_pAdaCtrl->fnExp_AddWaveFile(1, L"Receipt_Location_Other.wav");
	m_pAdaCtrl->fnExp_PlayScreenWave(1);
	m_pAdaCtrl->fnExp_WaitUntilStop();
}

BIZ_RETURN CTranCmn::P_NH_LTXP_DigitalReceipt()
{
	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_AuthFail(CString errorMessage)
{
	NVDump('O', 'C', "37", L"", L"AUTHX");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_AuthFail]\n"));

	m_pDevCmn->fnSCR_DisplayPrevSet(356);
	m_pDevCmn->fnSCR_DisplayString(2, errorMessage);
	m_pDevCmn->fnSCR_DisplayScreen(356);	

	Delay_Msg(5000); // Show screen for 5 seconds

	// Remove the EMV card, if present
	if (m_pDevCmn->fnMCU_IsEmvTransaction())
	{
		P_EMV_RemoveCard();	
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_AuthFail_ADA(CString errorMessage)
{
	NVDump('O', 'C', "37", L"", L"AXADA");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_AuthFail_ADA]\n"));

	// Remove the EMV card, if present
	if (m_pDevCmn->fnMCU_IsEmvTransaction())
	{
		P_EMV_RemoveCard();
	}

	return PlayLibertyXADAMessage(356);
}


BIZ_RETURN CTranCmn::P_NH_LTXP_ExecFail(CString errorMessage)
{
	NVDump('O', 'C', "38", L"", L"EXECX");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_ExecFail]\n"));

	// Remove the EMV card, if present
	if (m_pDevCmn->fnMCU_IsEmvTransaction())
	{
		P_EMV_RemoveCard();	
	}

	m_pDevCmn->fnSCR_DisplayPrevSet(356);
	m_pDevCmn->fnSCR_DisplayString(2, errorMessage);
	m_pDevCmn->fnSCR_DisplayScreen(356);	

	if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
	{
		if (!P_NH_NOR_TransactionPrint())
		{
			NHERROR((L"Printer failed to print LTX receipt\r\n"));
			return RES_NG;
		}
	}
	else
	{
		// Do digital receipt
		return RES_NG;
	}

	Delay_Msg(5000); // Show screen for 5 seconds

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_ExecFail_ADA(CString errorMessage)
{
	NVDump('O', 'C', "38", L"", L"EXADA");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_ExecFail_ADA]\n"));

	BIZ_RETURN res = RES_OK;

	// Remove the EMV card, if present
	if (m_pDevCmn->fnMCU_IsEmvTransaction())
	{
		P_EMV_RemoveCard();	
	}

	res = PlayLibertyXADAMessage(356);
	if (res != RES_OK)
	{
		return RES_NG;
	}

	if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
	{
		if (!P_NH_NOR_TransactionPrint())
		{
			NHERROR((L"Printer failed to print LTX receipt\r\n"));
			return RES_NG;
		}
	}
	else
	{
		// Do digital receipt
		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_DebitFail(CString errorCode, CString errorMessage)
{
	NVDump('O', 'C', "39", L"", L"DEBITX");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_DebitFail]\n"));

	// Remove the EMV card, if present
	if (m_pDevCmn->fnMCU_IsEmvTransaction())
	{
		P_EMV_RemoveCard();	
	}

	m_pDevCmn->fnSCR_DisplayPrevSet(357);
	m_pDevCmn->fnSCR_DisplayScreen(357);	

	if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
	{
		if (!P_NH_NOR_TransactionPrint())
		{
			NHERROR((L"Printer failed to print LTX receipt\r\n"));
			return RES_NG;
		}
	}
	else
	{
		// Do digital receipt
		return RES_NG;
	}

	Delay_Msg(5000); // Show screen for 5 seconds

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_DebitFail_ADA(CString errorCode, CString errorMessage)
{
	NVDump('O', 'C', "39", L"", L"DXADA");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_DebitFail_ADA]\n"));

	BIZ_RETURN res = RES_OK;

	// Remove the EMV card, if present
	if (m_pDevCmn->fnMCU_IsEmvTransaction())
	{
		P_EMV_RemoveCard();	
	}

	res = PlayLibertyXADAMessage(357);
	if (res != RES_OK)
	{
		return RES_NG;
	}

	if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
	{
		if (!P_NH_NOR_TransactionPrint())
		{
			NHERROR((L"Printer failed to print LTX receipt\r\n"));
			return RES_NG;
		}
	}
	else
	{
		// Do digital receipt
		return RES_NG;
	}

	return RES_OK;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_BillPay()
{
	NVDump('O', 'C', "3A", L"", L"BILL");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_BillPay]\n"));

	CString keyString;

	//
	// Display bill pay notice
	//

	m_pDevCmn->fnSCR_DisplayPrevSet(360);
	m_pDevCmn->fnSCR_DisplayScreen(360, KEYIN_TIME_OUT, PIN_MENU_MODE);
	Delay_Msg(100);

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			keyString = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (keyString == L"DONE")
			{
				NVDump('O', 'C', "3A", L"NG_0", keyString); // TODO

				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	NVDump('F', 'C', "3A", L"NG_2", L"CANCEL"); // TODO

	return RES_USER_TIMEOUT;
}

BIZ_RETURN CTranCmn::P_NH_LTXP_BillPay_ADA()
{
	NVDump('O', 'C', "3A", L"", L"BILLADA");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_LTXP_BillPay_ADA]\n"));

	return PlayLibertyXADAMessage(360);
}

#endif // APP_LIBERTYX