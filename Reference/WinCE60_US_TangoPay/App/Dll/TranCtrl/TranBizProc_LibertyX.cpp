#include "stdafx.h"
#include ".\Scr\ScrCtrl.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"

#include ".\Common\NHDbgApi.h"
#include ".\Tran\LibertyXTypes.h"

#if (APP_LIBERTYX)

#define AUTHORIZATION_RETRIES	1 // Max of 2 attempts
#define EXECUTE_RETRIES			1 // Max of 3 attempts
#define CANCEL_RETRIES			4 // Max of 5 attempts

extern CTimeCheck	g_TimeCheck;

BIZ_RETURN CTranCmn::PlayLibertyXADAMessage(int screenNumber)
{
	CString keyString;
	CString wavfile;
	BOOL replay = TRUE;

	wavfile.Format(L"%d.wav", screenNumber);

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
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_1");
			m_pAdaCtrl->fnExp_StopPlay();
			return RES_USER_EXIT;
		}

		if (replay)
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			m_pAdaCtrl->fnExp_AddWaveFile(screenNumber, wavfile);
			m_pAdaCtrl->fnExp_PlayScreenWave(screenNumber);
			replay = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			keyString = m_pDevCmn->fstrSCR_GetKeyString(1);
			keyString = keyString.Mid(6);

			if (keyString == S_ENTER)
			{
				NVDump('O', 'C', "22", L"NG_0", keyString); // TODO

				return RES_OK;
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
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));

	NVDump('F', 'C', "22", L"NG_2", L"CANCEL"); // TODO

	return RES_USER_TIMEOUT;
}

/**
 * Returns the number of LibertyX-specific ads, which should be placed into rotation
 * [#RWC6-280] LibertyX Default Ads
 */
int CTranCmn::GetNumberOfLibertyXAds()
{
	if( m_LXIsConfigured && m_LXConfig.IsEnabled() )
	{
		return 2;
	}

	// Return no ads if LibertyX is disabled
	return 0;
}
#endif