#include "stdafx.h"
#include "SVC_Manager.h"

#include "EagleTritonMsg.h"
#include "EagleSVCLib.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Initialize Screen
void CSVC_Manager::Client_Screen_Initialize()
{
	// Display Screen
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_INITIALIZE);
}


// Voice Guidance
void CSVC_Manager::Client_Screen_VoiceGuidance()
{
	// Display Screen
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_ADA_SCR_DISPLAY);
}


// Insert Card
void CSVC_Manager::Client_Screen_InsertCard(BOOL bADAMode)
{
	CString strCommand, strTextValue;
	CString strTemp, strWelcomeMsg;
	BOOL	bIsWelcomeMsg = FALSE;
	BOOL	bActiveAdvertisement = FALSE;
	CString	strAdvEnableFlags;
	int		nAdv_IntervalTime = 0;

	LOG(Info, _T("Client_Screen_InsertCard"));

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Key Disable
		CSVC_Manager::GetInstance()->CrypteraEPP_StopClearTextMode();

		// Set Advertisement
		for(int i=0; i< SUPPORT_ADVERTISE; i++)
		{
			strAdvEnableFlags += CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Screen_en[i];

			if (CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Screen_en[i] == S_ENABLE)
				bActiveAdvertisement = TRUE;
		}

		nAdv_IntervalTime = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Interval_time);

		CSCR_Manager::GetInstance()->SetAdvertisement(bActiveAdvertisement, nAdv_IntervalTime, strAdvEnableFlags);


		for (int i=0; i<3; i++)
		{
			strTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strAttraction_message[i];

			// Center Display를 위해 좌/우측 Space 제거
			strTemp.TrimLeft();
			strTemp.TrimRight();

			if (strTemp.GetLength() > 0)
				bIsWelcomeMsg = TRUE;

			strWelcomeMsg += strTemp + SCR_LINEFEED_DELEMITER;
		}

		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_INSERTCARD);

		// Added Data
		if (bIsWelcomeMsg == TRUE)
		{
			strTextValue = strWelcomeMsg;
		}
		else
		{
			if(EAGLE_SCREEN_CX == CS130_WIDTH_RESOLUTION)	// CS130일 경우에만 하도록 변경 (화면 Script 차이)
				strTextValue = _T("\n\n");	// 하단에 표시하기 위해 2 line 추가
		}

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.IsEMV_Enable() == TRUE)
		{
			if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_CardExist == TRUE)
			{
				// Card가 있는 경우 고객이 제거할 수 있도록 Notice warning 추가
				PlaySound(CUtil::GetNoticeWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);
				strTextValue += CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_0052);
			}
			else
			{
				strTextValue += CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_0051);
			}
		}
		else
		{
			strTextValue += CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_005);
		}


		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
		CSCR_Manager::GetInstance()->SetVariable(strCommand, strTextValue);

		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_INSERTCARD);

		// Notice Sound 추가
		PlaySound(CUtil::GetNoticeWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);
	}
	else
	{
		CSCR_Manager::GetInstance()->SetAdvertisement(FALSE);	// ADA인 경우 Advertisement False 처리 추가

		// Key Enable
		Client_Screen_VoiceGuidance();

		GetInstance()->CrypteraEPP_EnterClearTextMode();

		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_CardExist == FALSE)
		{
			CDEV_Manager::GetInstance()->AddWaveFile(INTRODUCE_WAVE_FILE);

			CDEV_Manager::GetInstance()->AddWaveFile(LOCATION_EPP_WAVE_FILE);

			CDEV_Manager::GetInstance()->AddWaveFile(INTRO_VOLUME_WAVE_FILE);

			CDEV_Manager::GetInstance()->AddWaveFile(LOCATION_CARD_WAVE_FILE);

			CDEV_Manager::GetInstance()->AddWaveFile(LOCATION_PRINTER_WAVE_FILE);

			CDEV_Manager::GetInstance()->AddWaveFile(LOCATION_DISPENSER_WAVE_FILE);

			if (CDEV_Manager::GetInstance()->m_DEV_CDR.IsEMV_Enable() == TRUE)
			{
				CDEV_Manager::GetInstance()->AddWaveFile(EMV_TRANSACTION_WAVE_FILE);
			}
			else
			{
				CDEV_Manager::GetInstance()->AddWaveFile(MS_TRANSACTION_WAVE_FILE);
			}
		}
		else
		{
			CDEV_Manager::GetInstance()->AddWaveFile(EMV_REMOVECARDFORTRANS_WAVE_FILE);
		}

		CDEV_Manager::GetInstance()->PlayWaveFile();
	}
}

void CSVC_Manager::Client_Screen_EMVProcessing(BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_EMVProcessing"));

	//bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if(bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_EMVPROCESSING);

		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_EMVPROCESSING);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(TRANSACTION_PROCESSING_WVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);	// wave file play가 끝날때까지 wait
	}
}


// Transaction Cancel
void CSVC_Manager::Client_Screen_TransactionCancel(BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_TransactionCancel"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_TRANSACTION_CANCEL);

		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_TRANSACTION_CANCEL);
		CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		// Play Transaction cancel
		CDEV_Manager::GetInstance()->ResetWaveFile();
		CDEV_Manager::GetInstance()->AddWaveFile(TRANSCANCEL_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
	}

	// 고객 변경
	Client_ChangeCustom();
}


// EMV Fallback
void CSVC_Manager::Client_Screen_EMV_Fallback(BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_EMV_Fallback"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_EMVFALLBACK);

		// Added Data

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));

		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_EMVFALLBACK);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(EMV_FALLBACK_TRANSACTION_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();
	}
}


void CSVC_Manager::Client_Screen_CardReadError(BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_CardReadError"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Card Read Error시 Sound 추가
		PlaySound(CUtil::GetKeyPressWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);

		// 카드 에러 화면 표시
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_CARDERROR);

		// Added Data

		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_CARDERROR);
		CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		// Play Card Error
		CDEV_Manager::GetInstance()->ResetWaveFile();
		CDEV_Manager::GetInstance()->AddWaveFile(CARDERROR_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
	}

	Client_ChangeCustom();
}


// Select Multi AID
void CSVC_Manager::Client_Screen_EMV_SelectMultiAID(BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_EMV_SelectMultiAID"));

	CString strCmd, strValue, strTemp, strLabel, strAID;

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_EMVMULTIAPPSELECT);

		// Added Data
		for(int i=0; i<6; i++)
		{
			if (i < CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList)
			{
				if (CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].IsrCodeTableIndex == 1)
				{
					strLabel.Format(_T("%S"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].PreferredName);
					strLabel.TrimRight();

					if (strLabel.IsEmpty())
					{
						// Preffered Name이 없을 경우 Label 표시
						strLabel.Format(_T("%S"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].AppLabel);
						strLabel.TrimRight();
					}
				}
				else
				{
					strLabel.Format(_T("%S"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].AppLabel);
					strLabel.TrimRight();
				}

				strValue += strLabel;
				strValue += SCR_CMD_DELIMITER;
			}
		}

		strCmd.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
		CSCR_Manager::GetInstance()->SetVariable(strCmd, strValue);


		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));


		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_EMVMULTIAPPSELECT);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(EMV_SELECT_BRANDNAME_WAVE_FILE);

		int nAIDCount = CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList;

		if (nAIDCount > 0)
		{
			strTemp = CDEV_Manager::GetInstance()->m_DEV_CDR.Get_BrandNameForADA(0);
			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
			CDEV_Manager::GetInstance()->AddWaveFile(strTemp);
		}

		if (nAIDCount > 1)
		{
			strTemp = CDEV_Manager::GetInstance()->m_DEV_CDR.Get_BrandNameForADA(1);
			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
			CDEV_Manager::GetInstance()->AddWaveFile(strTemp);
		}

		if (nAIDCount > 2)
		{
			strTemp = CDEV_Manager::GetInstance()->m_DEV_CDR.Get_BrandNameForADA(2);
			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_3_WAVE_FILE);
			CDEV_Manager::GetInstance()->AddWaveFile(strTemp);
		}

		if (nAIDCount > 3)
		{
			strTemp = CDEV_Manager::GetInstance()->m_DEV_CDR.Get_BrandNameForADA(3);
			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_4_WAVE_FILE);
			CDEV_Manager::GetInstance()->AddWaveFile(strTemp);
		}

		if (nAIDCount > 4)
		{
			strTemp = CDEV_Manager::GetInstance()->m_DEV_CDR.Get_BrandNameForADA(4);
			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_5_WAVE_FILE);
			CDEV_Manager::GetInstance()->AddWaveFile(strTemp);
		}

		if (nAIDCount > 5)
		{
			strTemp = CDEV_Manager::GetInstance()->m_DEV_CDR.Get_BrandNameForADA(5);
			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_6_WAVE_FILE);
			CDEV_Manager::GetInstance()->AddWaveFile(strTemp);
		}

		CDEV_Manager::GetInstance()->AddWaveFile(INTRO_VOLUME_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CANCEL_GUIDE_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();
	}
}


// Error Notice
void CSVC_Manager::Client_Screen_ErrorNotice(CString strErrorMsg,  CString strSubMsg)
{
	BOOL bADAMode = Is_DIO_Ear_Jack_Insert();
	CString strTemp;

	LOG(Info, _T("Client_Screen_ErrorNotice"));

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		CString strCommand, strTextValue;

		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_ERROR_NOTICE);

		// Added Data
		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
		strTextValue = CSCR_Manager::GetInstance()->GetAPTextIDString(strErrorMsg);

		// Sub Message 존재시
		if (strSubMsg.IsEmpty() == FALSE)
		{
			strTextValue += SCR_LINEFEED_DELEMITER;
			strTextValue += strSubMsg;
		}

		CSCR_Manager::GetInstance()->SetVariable(strCommand, strTextValue);

		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_ERROR_NOTICE);
		
		CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

	}
	else
	{
		Client_Screen_VoiceGuidance();

		// 해당 문구에 맞는 wave file play
		if (strErrorMsg == GETTEXT_001)
		{
			CDEV_Manager::GetInstance()->AddWaveFile(INVALIDAMOUNT_WAVE_FILE);
			CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
		}
		else if (strErrorMsg == GETTEXT_004)
		{
			CDEV_Manager::GetInstance()->AddWaveFile(DECLINED_WAVE_FILE);

			// Host Response Code에 대해서만 wave play
			if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_WITHDRAWAL || 
				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_BALANCEINQUIRY ||
				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_TRANSFER)
			{
				// 거래 전문에 대해서만 wave guide (Reversal error에 대한 Guide는 추가 보완 필요)
				if (CEagleTritonMsg::GetInstance()->m_sTriton_TranResp.m_strResponseCode_3.IsEmpty() == FALSE)
				{
					if (CEagleTritonMsg::GetInstance()->m_sTriton_TranResp.m_strResponseCode_3 != _T("000"))
					{
						strTemp.Format(_T("Decline_%3.3s.wav"), CEagleTritonMsg::GetInstance()->m_sTriton_TranResp.m_strResponseCode_3);
						CDEV_Manager::GetInstance()->AddWaveFile(strTemp);	
					}
				}
			}

			CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
		}
		else if (strErrorMsg == GETTEXT_016)
		{
			CDEV_Manager::GetInstance()->AddWaveFile(EMV_CARDERROR_WAVE_FILE);
			CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
		}
		else if (strErrorMsg == GETTEXT_011)
		{
			CDEV_Manager::GetInstance()->AddWaveFile(DISPENSING_ERROR_WAVE_FILE);
			CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
		}
		else if (strErrorMsg == GETTEXT_017)
		{
			CDEV_Manager::GetInstance()->AddWaveFile(PRINTINGERROR_WAVE_FILE);
			CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);			
		}
		else if (strErrorMsg == GETTEXT_020)
		{
			// Wave File이 없어 Transaction is declined로 처리 - 추후 wave file 제작 필요
			CDEV_Manager::GetInstance()->AddWaveFile(DECLINED_WAVE_FILE);
			CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
		}
	}
}


// Select Language
void CSVC_Manager::Client_Screen_SelectLanguage(BOOL bADAMode)
{
	CString strCommand, strTextValue;

	LOG(Info, _T("Client_Screen_SelectLanguage"));

	// Screen Data Make
	CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_SELECT_LANGUAGE);

	// Added Data
	for(int i=0; i<6; i++)
	{
		strCommand.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, i+1);

		if (CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[i]))
			strTextValue = _T("on");
		else
			strTextValue = _T("off");

		CSCR_Manager::GetInstance()->SetVariable(strCommand, strTextValue);			
	}

	// Set Timeout
	CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
	CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));


	// Display Screen
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_SELECT_LANGUAGE);
}

// Enter PIN
void CSVC_Manager::Client_Screen_EnterPIN(BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_EnterPIN"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_ENTERPASSWORD);

		// Added Data

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));


		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_ENTERPASSWORD);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(ENTERPIN_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();
	}
}

// Select Transaction
CString CSVC_Manager::Client_Screen_SelectTransaction(BOOL bADAMode)
{
	CString strCmd;
	CString strTemp, strButtonValue;
	CString strADAActiveInfo;
	int		nIndex = 1;

	LOG(Info, _T("Client_Screen_SelectTransaction"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_SELECT_TRANSACTION);

		// Added Data
		if (CEagleDataManager::GetInstance()->m_Config.m_Option.strbalance_Enable == S_DISABLE)
			strButtonValue += _T("off");
		else
			strButtonValue += _T("on");

		strButtonValue += SCR_CMD_DELIMITER;

		if (CEagleDataManager::GetInstance()->m_Config.m_Option.strtransfer_Enable == S_DISABLE)
			strButtonValue += _T("off");
		else
			strButtonValue += _T("on");

		strButtonValue += SCR_CMD_DELIMITER;

		strCmd.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, 1); 
		CSCR_Manager::GetInstance()->SetVariable(strCmd, strButtonValue);

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));

		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_SELECT_TRANSACTION);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(SELECT_TRANSACTION_WAVE_FILE);

		// 1. Withdrawal
		strADAActiveInfo.Empty();
		strTemp.Format(_T("%d%c%s%c"), nIndex, UNIT_DELIMITER, S_WITHDRAWAL, UNIT_DELIMITER);
		strADAActiveInfo += strTemp;

		CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(WITHDRAWAL_WAVE_FILE);

		// 2. Balance
		if (CEagleDataManager::GetInstance()->m_Config.m_Option.strbalance_Enable == S_ENABLE)
		{
			nIndex++;
			strTemp.Format(_T("%d%c%s%c"),  nIndex, UNIT_DELIMITER, S_BALANCEINQUIRY, UNIT_DELIMITER);
			strADAActiveInfo += strTemp;

			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
			CDEV_Manager::GetInstance()->AddWaveFile(INQUIRY_WAVE_FILE);
		}

		if (CEagleDataManager::GetInstance()->m_Config.m_Option.strtransfer_Enable == S_ENABLE)
		{
			nIndex++;
			strTemp.Format(_T("%d%c%s"), nIndex, UNIT_DELIMITER, S_TRANSFER);
			strADAActiveInfo += strTemp;

			if (nIndex == 2)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
			else
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_3_WAVE_FILE);
			CDEV_Manager::GetInstance()->AddWaveFile(TRANSFER_WAVE_FILE);
		}

		CDEV_Manager::GetInstance()->AddWaveFile(INTRO_VOLUME_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CANCEL_GUIDE_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();

	}

	return strADAActiveInfo;
}


// Select Account
CString CSVC_Manager::Client_Screen_SelectAccount(BOOL bADAMode)
{
	CString strCmd, strButtonValue;
	CString strADAActiveInfo, strTemp;
	int		nIndex = 1;

	LOG(Info, _T("Client_Screen_SelectAccount"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_SELECT_ACCOUNT);

		// Added Data
		if (CEagleDataManager::GetInstance()->m_Config.m_Option.strSaving_account == S_DISABLE)
			strButtonValue += _T("off");
		else
			strButtonValue += _T("on");
		strButtonValue += SCR_CMD_DELIMITER;

		if (CEagleDataManager::GetInstance()->m_Config.m_Option.strCredit_account == S_DISABLE)
			strButtonValue += _T("off");
		else
			strButtonValue += _T("on");
		strButtonValue += SCR_CMD_DELIMITER;


		strCmd.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, 1);
		CSCR_Manager::GetInstance()->SetVariable(strCmd, strButtonValue);

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));


		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_SELECT_ACCOUNT);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(SELECT_ACCOUNT_WAVE_FILE);

		strADAActiveInfo.Empty();
		strTemp.Format(_T("%d%c%s%c"), nIndex, UNIT_DELIMITER, S_CHECKING, UNIT_DELIMITER);
		strADAActiveInfo += strTemp;

		CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CHECKING_WAVE_FILE);

		if (CEagleDataManager::GetInstance()->m_Config.m_Option.strSaving_account == S_ENABLE)
		{
			nIndex++;
			strTemp.Format(_T("%d%c%s%c"), nIndex, UNIT_DELIMITER, S_SAVINGS, UNIT_DELIMITER);
			strADAActiveInfo += strTemp;

			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
			CDEV_Manager::GetInstance()->AddWaveFile(SAVINGS_WAVE_FILE);
		}

		if (CEagleDataManager::GetInstance()->m_Config.m_Option.strCredit_account == S_ENABLE)
		{
			nIndex++;
			strTemp.Format(_T("%d%c%s"), nIndex, UNIT_DELIMITER, S_CREDIT);
			strADAActiveInfo += strTemp;

			if (nIndex == 2)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
			else
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_3_WAVE_FILE);
			CDEV_Manager::GetInstance()->AddWaveFile(CREDIT_WAVE_FILE);
		}

		CDEV_Manager::GetInstance()->AddWaveFile(INTRO_VOLUME_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CANCEL_GUIDE_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();
	}

	return strADAActiveInfo;
}


// Select Transfer Account
CString CSVC_Manager::Client_Screen_SelectTransferAccount(BOOL bADAMode)
{
	CString strCmd, strButtonValue;
	CString strADAActiveInfo, strTemp;
	int		nIndex = 1;

	LOG(Info, _T("Client_Screen_SelectTransferAccount"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_SELECT_TRANSFER_ACCOUNT);

		// Added Data

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));


		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_SELECT_TRANSFER_ACCOUNT);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(SELECT_ACCOUNT_WAVE_FILE);

		strADAActiveInfo.Empty();
		strTemp.Format(_T("%d%c%s%c"), nIndex, UNIT_DELIMITER, _T("CHECKINGTOSAVINGS"), UNIT_DELIMITER);
		strADAActiveInfo += strTemp;

		CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CHECKINGTOSAVINGS_WAVE_FILE);

		nIndex++;

		strTemp.Format(_T("%d%c%s%c"), nIndex, UNIT_DELIMITER, _T("SAVINGSTOCHECKING"), UNIT_DELIMITER);
		strADAActiveInfo += strTemp;

		CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(SAVINGSTOCHECKING_WAVE_FILE);

		nIndex++;

		strTemp.Format(_T("%d%c%s"), nIndex, UNIT_DELIMITER, _T("CREDITTOCHECKING"));
		strADAActiveInfo += strTemp;

		CDEV_Manager::GetInstance()->AddWaveFile(PRESS_3_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CREDITTOCHECKING_WAVE_FILE);

		CDEV_Manager::GetInstance()->AddWaveFile(INTRO_VOLUME_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CANCEL_GUIDE_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();
	}

	return strADAActiveInfo;
}


// Select Amount
CString CSVC_Manager::Client_Screen_SelectAmount(BOOL bADAMode)
{
	CString strCmd, strValue, strTemp;
	int		nAmount;
	CString strADAActiveInfo;
	int		nIndex = 1;

	LOG(Info, _T("Client_Screen_SelectAmount"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_SELECT_AMOUNT);

		// Added Data
		for(int i=0; i<6; i++)
		{
			strTemp.Empty();
			nAmount = 0;

			nAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[i]);

			if (nAmount > 0)
			{		
				// 방출 가능 여부 추가 확인 후 표시 여부
				if (CDEV_Manager::GetInstance()->m_DEV_CDM.IsDispenable(nAmount) == TRUE)
				{
					// 1회 최대 방출 금액도 추가로 Check하도록 함
					if (nAmount <= CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strMax_withdrawal_amount))
					{
						if (i > 2)
							strTemp.Format(_T("%s%s%d"), SPACE_2, CURRENCY_SYMBOL, nAmount);
						else
							strTemp.Format(_T("%s%d%s"), CURRENCY_SYMBOL, nAmount, SPACE_2);
					}
				}
			}

			strValue += strTemp;
			strValue += SCR_CMD_DELIMITER;
		}

		strCmd.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
		CSCR_Manager::GetInstance()->SetVariable(strCmd, strValue);


		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));


		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_SELECT_AMOUNT);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(SELECT_AMOUNT_WAVE_FILE);

		strADAActiveInfo.Empty();

		nAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[0]);

		if (CDEV_Manager::GetInstance()->m_DEV_CDM.IsDispenable(nAmount) == TRUE)
		{
			strTemp.Format(_T("%d%c%d%c"), nIndex, UNIT_DELIMITER, nAmount, UNIT_DELIMITER);
			strADAActiveInfo += strTemp;

			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
			CDEV_Manager::GetInstance()->AddAmountWaveFiles(CUtil::IntToString(nAmount));

			nIndex++;
		}

		nAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[1]);

		if (CDEV_Manager::GetInstance()->m_DEV_CDM.IsDispenable(nAmount) == TRUE)
		{
			strTemp.Format(_T("%d%c%d%c"), nIndex, UNIT_DELIMITER, nAmount, UNIT_DELIMITER);
			strADAActiveInfo += strTemp;

			if (nIndex == 1)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
			else if (nIndex == 2)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);

			CDEV_Manager::GetInstance()->AddAmountWaveFiles(CUtil::IntToString(nAmount));

			nIndex++;
		}

		nAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[2]);

		if (CDEV_Manager::GetInstance()->m_DEV_CDM.IsDispenable(nAmount) == TRUE)
		{
			strTemp.Format(_T("%d%c%d%c"), nIndex, UNIT_DELIMITER, nAmount, UNIT_DELIMITER);
			strADAActiveInfo += strTemp;

			if (nIndex == 1)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
			else if (nIndex == 2)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
			else if (nIndex == 3)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_3_WAVE_FILE);

			CDEV_Manager::GetInstance()->AddAmountWaveFiles(CUtil::IntToString(nAmount));

			nIndex++;
		}

		nAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[3]);

		if (CDEV_Manager::GetInstance()->m_DEV_CDM.IsDispenable(nAmount) == TRUE)
		{
			strTemp.Format(_T("%d%c%d%c"), nIndex, UNIT_DELIMITER, nAmount, UNIT_DELIMITER);
			strADAActiveInfo += strTemp;

			if (nIndex == 1)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
			else if (nIndex == 2)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
			else if (nIndex == 3)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_3_WAVE_FILE);
			else if (nIndex == 4)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_4_WAVE_FILE);

			CDEV_Manager::GetInstance()->AddAmountWaveFiles(CUtil::IntToString(nAmount));

			nIndex++;
		}

		nAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[4]);

		if (CDEV_Manager::GetInstance()->m_DEV_CDM.IsDispenable(nAmount) == TRUE)
		{
			strTemp.Format(_T("%d%c%d%c"), nIndex, UNIT_DELIMITER, nAmount, UNIT_DELIMITER);
			strADAActiveInfo += strTemp;

			if (nIndex == 1)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
			else if (nIndex == 2)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
			else if (nIndex == 3)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_3_WAVE_FILE);
			else if (nIndex == 4)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_4_WAVE_FILE);
			else if (nIndex == 5)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_5_WAVE_FILE);

			CDEV_Manager::GetInstance()->AddAmountWaveFiles(CUtil::IntToString(nAmount));

			nIndex++;
		}

		nAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[5]);

		if (CDEV_Manager::GetInstance()->m_DEV_CDM.IsDispenable(nAmount) == TRUE)
		{
			strTemp.Format(_T("%d%c%d%c"), nIndex, UNIT_DELIMITER, nAmount, UNIT_DELIMITER);
			strADAActiveInfo += strTemp;

			if (nIndex == 1)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
			else if (nIndex == 2)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
			else if (nIndex == 3)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_3_WAVE_FILE);
			else if (nIndex == 4)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_4_WAVE_FILE);
			else if (nIndex == 5)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_5_WAVE_FILE);
			else if (nIndex == 6)
				CDEV_Manager::GetInstance()->AddWaveFile(PRESS_6_WAVE_FILE);

			CDEV_Manager::GetInstance()->AddAmountWaveFiles(CUtil::IntToString(nAmount));

			nIndex++;
		}

		// Other
		strTemp.Format(_T("%d%c%s"), nIndex, UNIT_DELIMITER, _T("OTHERAMOUNT"));
		strADAActiveInfo += strTemp;

		if (nIndex == 1)
			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
		else if (nIndex == 2)
			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
		else if (nIndex == 3)
			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_3_WAVE_FILE);
		else if (nIndex == 4)
			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_4_WAVE_FILE);
		else if (nIndex == 5)
			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_5_WAVE_FILE);
		else if (nIndex == 6)
			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_6_WAVE_FILE);
		else if (nIndex == 7)
			CDEV_Manager::GetInstance()->AddWaveFile(PRESS_7_WAVE_FILE);

		CDEV_Manager::GetInstance()->AddWaveFile(OTHER_WAVE_FILE);

		CDEV_Manager::GetInstance()->AddWaveFile(INTRO_VOLUME_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CANCEL_GUIDE_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();
	}

	return strADAActiveInfo;
}


// Enter Withdrawal Amount
void CSVC_Manager::Client_Screen_EnterWithdrawalAmount(BOOL bADAMode)
{
	CString strCmd, strValue, strTemp;
	CString strAmount;
	int i = 0, j = 0, nScrValueIndex = 1;

	LOG(Info, _T("Client_Screen_EnterWithdrawalAmount"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	int nCbxCount = CDEV_Manager::GetInstance()->m_DEV_CDM.m_nCbxCount;

	// 카세트 권종 취득
	int nCbxDenom[4];
	memset(nCbxDenom, 0, sizeof(nCbxDenom));

	for (i=0; i<__min(nCbxCount, 4); i++)
	{
		nCbxDenom[i] = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i]);
	}

	// 카세트 잔류 취득
	int nCbxRemain[4];
	memset(nCbxRemain, 0, sizeof(nCbxRemain));

	for (i=0; i<__min(nCbxCount, 4); i++)
	{
		nCbxRemain[i] = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);
	}

	// 카세트 상태 취득
	DWORD nCbxStatus[MAX_CASSETTE+1];
	memset(nCbxStatus, 0, sizeof(nCbxStatus));

	CDEV_Manager::GetInstance()->m_DEV_CDM.GetCbxStatus(nCbxStatus);

	// 방출 가능한 최소 권종 찾기
	BOOL bFirstCBX = FALSE;
	int nMinDenom = 0;

	// 가능한 최소 권종 찾기
	for(i=0; i<__min(nCbxCount, 4); i++)
	{
		if ((nCbxDenom[i] > 0) && (nCbxRemain[i] > 0) && (nCbxStatus[i+1] == WFS_CDM_STATCULOW || nCbxStatus[i+1] == WFS_CDM_STATCUOK))
		{
			if (bFirstCBX == FALSE)
				nMinDenom = nCbxDenom[i];

			// 더 작은 값으로 갱신
			if (nMinDenom > nCbxDenom[i])
				nMinDenom = nCbxDenom[i];

			bFirstCBX = TRUE;
		}
	}

	// Max Amount 값 구하기
	// Mix를 돌려서 방출이 불가능할 경우 최대 방출 가능한 매수를 다시 설정함.
	CString strMaxAmount;
	int nMaxAmount = 0;

	strMaxAmount = CEagleDataManager::GetInstance()->m_Config.m_Option.strMax_withdrawal_amount;
	nMaxAmount = CUtil::StringToInt(strMaxAmount);

	if (CDEV_Manager::GetInstance()->m_DEV_CDM.IsDispenable(nMaxAmount) == FALSE)
	{
		// 설정된 금액 방출이 불가능하므로 다시 산정함
		LOG(Info, _T("Max Amount i not dispensible (%d) - Try to recalculator - Start"), nMaxAmount);

		// 설정된 금액 방출이 불가능하므로 다시 산정함
		CDMMIXRESULT TempMixResult;
		UINT nRemainAmount = 0;

		memset(&TempMixResult, 0, sizeof(TempMixResult));
		CDEV_Manager::GetInstance()->m_DEV_CDM.ExecuteMixAlgorithm4MaxAvailAmount(nMaxAmount, &TempMixResult, nRemainAmount, TRUE);

		LOG(Info, _T("Calculated result : Max Amount : %d, Remain Amount : %d"), nMaxAmount, nRemainAmount);

		if (nRemainAmount > 0)
			nMaxAmount = nMaxAmount - nRemainAmount;

		LOG(Info, _T("Calculated result : Available Max Amount : %d"), nMaxAmount);
	}

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_ENTER_AMOUNT);

		// Add Data
		// Multiple of Amount	APValue1
		BOOL bFoundAvaiCST = FALSE;
		BOOL bFoundSameDenom = FALSE;
		int nTempDenom[4];
		memset(nTempDenom, 0, sizeof(nTempDenom));

		for(i=0; i<__min(nCbxCount, 4); i++)
		{
			strTemp.Empty();
			bFoundSameDenom = FALSE;

			if ((nCbxDenom[i] > 0) && (nCbxRemain[i] > 0) && (nCbxStatus[i+1] == WFS_CDM_STATCULOW || nCbxStatus[i+1] == WFS_CDM_STATCUOK))
			{
				// 동일 권종인 경우는 1번만 표시하도록 함 (LCU개념)
				for(int j=0; j<4; j++)
				{
					if (nCbxDenom[i] == nTempDenom[j])
					{
						LOG(Info, _T("Same Denomination Info : %d %d"), nTempDenom[j], nCbxDenom[i]);
						bFoundSameDenom = TRUE;
						break;
					}
				}

				if (bFoundSameDenom == TRUE)
					continue;

				nTempDenom[i] = nCbxDenom[i];

				if (bFoundAvaiCST == FALSE)			// first CST
					strTemp.Format(_T("%s%s%d"), CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_006), CURRENCY_SYMBOL, nCbxDenom[i]);
				else
					strTemp.Format(_T(", %s%d"), CURRENCY_SYMBOL, nCbxDenom[i]);

				strValue += strTemp;
				bFoundAvaiCST = TRUE;
			}
		}

		strCmd.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, nScrValueIndex++);
		CSCR_Manager::GetInstance()->SetVariable(strCmd, strValue);

		strCmd.Empty();
		strValue.Empty();

		// Max Amount	APValue2
		strCmd.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, nScrValueIndex++);
		strValue.Format(_T("%s %s%s"), CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_002), CURRENCY_SYMBOL, CUtil::IntToString(nMaxAmount));
		CSCR_Manager::GetInstance()->SetVariable(strCmd, strValue);

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));


		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_ENTER_AMOUNT);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(ENTER_AMOUNT_WAVE_FILE);

		//// 최소 권종 1종류만 Guide
		// Multiple minimum Denomination
		CDEV_Manager::GetInstance()->AddWaveFile(MULTIPLE_AMOUNT_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddAmountWaveFiles(CUtil::IntToString(nMinDenom));

		// Max Amount
		CDEV_Manager::GetInstance()->AddWaveFile(MAX_AMOUNT_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddAmountWaveFiles(CUtil::IntToString(nMaxAmount));

		CDEV_Manager::GetInstance()->AddWaveFile(INTRO_VOLUME_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CANCEL_GUIDE_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();
	}
}


// Confirm Amount for ADA
CString CSVC_Manager::Client_Screen_ConfirmAmount(BOOL bADAMode)
{
	CString strCmd, strButtonValue;
	CString strADAActiveInfo, strTemp;
	int		nIndex = 1;

	LOG(Info, _T("Client_Screen_ConfirmAmount"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// ADA 모드만 지원
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(CONFIRM_AMOUNT_WAVE_FILE);

		if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_WITHDRAWAL)
		{
			// Cent 제외한 금액
			strTemp = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strRequestAmount;
		}
		else
		{
			// Cent 포함한 금액
			strTemp = CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransferAmount);
		}

		CDEV_Manager::GetInstance()->AddAmountWaveFiles(strTemp);
		CDEV_Manager::GetInstance()->AddWaveFile(CONTINUE_TRANSACTION_WAVE_FILE);

		strADAActiveInfo.Empty();
		strTemp.Format(_T("%d%c%s%c"), nIndex, UNIT_DELIMITER, S_YES, UNIT_DELIMITER);
		strADAActiveInfo += strTemp;

		CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(YES_WAVE_FILE);

		nIndex++;

		strTemp.Format(_T("%d%c%s%c"), nIndex, UNIT_DELIMITER, S_NO, UNIT_DELIMITER);
		strADAActiveInfo += strTemp;

		CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(NO_WAVE_FILE);

		CDEV_Manager::GetInstance()->AddWaveFile(INTRO_VOLUME_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CANCEL_GUIDE_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();
	}

	return strADAActiveInfo;
}


// Fee Notification
CString CSVC_Manager::Client_Screen_FeeNotification(BOOL bADAMode)
{
	CString strCommand, strTextValue;
	CString strTemp, strTemp2;

	CString strADAActiveInfo;
	int		nIndex = 1;

	LOG(Info, _T("Client_Screen_FeeNotification Screen"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_type == _T("0"))
	{
		// amount base

		// Surcharge amount
		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount.Format(_T("%03d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount));
	}
	else
	{
		// Get Selected Amount
		int nSelectedAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strRequestAmount + _T("00"));	// cent 포함해서 계산
		int nWithdrawalSurcharge = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount);
		int nWithdrawalSurchargePercent = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_percentage);

		float fSurcharge =  (float)(0.01 * nWithdrawalSurcharge); 
		float fPercentSurcharge = (float)(nSelectedAmount * 0.01) * (float)(nWithdrawalSurchargePercent * 0.0001);
		float fResultSurcharge = 0;

		// percentage base
		if (CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawal_PercentSurcharge_Option == _T("0"))
		{
			// calculate surcharge amount
			if( fPercentSurcharge < fSurcharge)
				fResultSurcharge = fPercentSurcharge;
			else
				fResultSurcharge = fSurcharge;
		}
		else
		{
			// calculate surcharge amount
			if( fPercentSurcharge < fSurcharge)
				fResultSurcharge = fSurcharge;
			else
				fResultSurcharge = fPercentSurcharge;
		}

		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount.Format(_T("%03d"), (int)(fResultSurcharge*100 + 0.001));
	}

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_CONFIRM_SURCHARGE);

		// Added Data
		strTemp.Format(CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_003));

		// Surcharge owner
		strTemp.Replace(_T("XXX_1"), CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_owner);

		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);

		strTemp2.Format(_T("%s %s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount));
		strTemp.Replace(_T("XXX_2"), strTemp2);
		strTextValue = strTemp;

		CSCR_Manager::GetInstance()->SetVariable(strCommand, strTextValue);

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));

		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_CONFIRM_SURCHARGE);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(FEE_NOTICE_1_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(FEE_NOTICE_2_WAVE_FILE);

		// Amount
		strTemp = CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount);
		CDEV_Manager::GetInstance()->AddAmountWaveFiles(strTemp);

		CDEV_Manager::GetInstance()->AddWaveFile(FEE_NOTICE_3_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CONTINUE_TRANSACTION_WAVE_FILE);

		CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(YES_WAVE_FILE);

		strTemp.Format(_T("%d%c%s%c"), nIndex, UNIT_DELIMITER, S_YES, UNIT_DELIMITER);
		strADAActiveInfo += strTemp;
		nIndex++;

		CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(NO_WAVE_FILE);

		strTemp.Format(_T("%d%c%s"), nIndex, UNIT_DELIMITER, S_EXIT);
		strADAActiveInfo += strTemp;

		CDEV_Manager::GetInstance()->AddWaveFile(INTRO_VOLUME_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CANCEL_GUIDE_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();
	}

	return strADAActiveInfo;
}


void CSVC_Manager::Client_Screen_EnterTransferAmount(BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_EnterTransferAmount Screen"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_ENTER_TRANSFERAMOUNT);

		// Add Data


		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));


		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_ENTER_TRANSFERAMOUNT);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(ENTER_TRANSFER_AMOUNT_WAVE_FILE);

		CDEV_Manager::GetInstance()->AddWaveFile(INTRO_VOLUME_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CANCEL_GUIDE_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();
	}
}


// Select Receipt
CString CSVC_Manager::Client_Screen_SelectReceipt(BOOL bADAMode)
{
	CString strADAActiveInfo, strTemp;
	int		nIndex = 1;

	LOG(Info, _T("Client_Screen_SelectReceipt Screen"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_SELECT_RECEIPT);

		// Added Data

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));


		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_SELECT_RECEIPT);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(SELECT_RECEIPT_WAVE_FILE);

		CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(YES_WAVE_FILE);

		strTemp.Format(_T("%d%c%s%c"), nIndex, UNIT_DELIMITER, S_YES, UNIT_DELIMITER);
		strADAActiveInfo += strTemp;
		nIndex++;

		CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(NO_WAVE_FILE);

		strTemp.Format(_T("%d%c%s"), nIndex, UNIT_DELIMITER, S_NO);
		strADAActiveInfo += strTemp;

		CDEV_Manager::GetInstance()->AddWaveFile(INTRO_VOLUME_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CANCEL_GUIDE_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();
	}

	return strADAActiveInfo;
}



// Confirm Receipt Error
CString CSVC_Manager::Client_Screen_ConfirmReceiptError(BOOL bADAMode)
{
	CString strADAActiveInfo, strTemp;
	int		nIndex = 1;

	LOG(Info, _T("Client_Screen_ConfirmReceiptError Screen"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_CONFIRM_RECEIPTERROR);

		// Added Data

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));


		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_CONFIRM_RECEIPTERROR);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(CONFIRM_RECEIPTERROR_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CONTINUE_TRANSACTION_WAVE_FILE);

		CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(YES_WAVE_FILE);

		strTemp.Format(_T("%d%c%s%c"), nIndex, UNIT_DELIMITER, S_YES, UNIT_DELIMITER);
		strADAActiveInfo += strTemp;
		nIndex++;

		CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(NO_WAVE_FILE);

		strTemp.Format(_T("%d%c%s"), nIndex, UNIT_DELIMITER, S_EXIT);
		strADAActiveInfo += strTemp;

		CDEV_Manager::GetInstance()->AddWaveFile(INTRO_VOLUME_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(CANCEL_GUIDE_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();
	}

	return strADAActiveInfo;
}


// Confirm Screen Receipt
CString CSVC_Manager::Client_Screen_ConfirmScreenReceipt(BOOL bADAMode)
{
	CString strADAActiveInfo, strTemp;
	int		nIndex = 1;

	LOG(Info, _T("Client_Screen_ConfirmReceiptError Screen"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		CString strCmd, strValue;

		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_CONFIRM_SCREENRECEIPT);

		// Added Data
		strCmd.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, 1);
		if (CEagleSVCLib::GetInstance()->IsPTR_Available() == FALSE)
		{
			strValue.Format(_T("%s"),_T("on"));
		}
		else
		{
			strValue.Format(_T("%s"),_T("off"));
		}

		CSCR_Manager::GetInstance()->SetVariable(strCmd, strValue);	

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));

		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_CONFIRM_SCREENRECEIPT);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		//CDEV_Manager::GetInstance()->AddWaveFile(CONFIRM_SCREEN_RECEIPT_WAVE_FILE);
		//CDEV_Manager::GetInstance()->AddWaveFile(CONTINUE_TRANSACTION_WAVE_FILE);

		//CDEV_Manager::GetInstance()->AddWaveFile(PRESS_1_WAVE_FILE);
		//CDEV_Manager::GetInstance()->AddWaveFile(YES_WAVE_FILE);

		//strTemp.Format(_T("%d%c%s%c"), nIndex, UNIT_DELIMITER, S_YES, UNIT_DELIMITER);
		//strADAActiveInfo += strTemp;
		//nIndex++;

		//CDEV_Manager::GetInstance()->AddWaveFile(PRESS_2_WAVE_FILE);
		//CDEV_Manager::GetInstance()->AddWaveFile(NO_WAVE_FILE);

		//strTemp.Format(_T("%d%c%s"), nIndex, UNIT_DELIMITER, S_EXIT);
		//strADAActiveInfo += strTemp;

		//CDEV_Manager::GetInstance()->AddWaveFile(INTRO_VOLUME_WAVE_FILE);
		//CDEV_Manager::GetInstance()->AddWaveFile(CANCEL_GUIDE_WAVE_FILE);
		//CDEV_Manager::GetInstance()->PlayWaveFile();
	}

	return strADAActiveInfo;
}


// Host Processing
void CSVC_Manager::Client_Screen_Processing(int nTransType, BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_Processing Screen"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_HOSTPROCESSING);

		// Added Data (Transaction Advertisement)


		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_HOSTPROCESSING);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		if (nTransType == 0)
			CDEV_Manager::GetInstance()->AddWaveFile(TRANSACTION_PROCESSING_WVE_FILE);
		else
			CDEV_Manager::GetInstance()->AddWaveFile(REVERSAL_PROCESSING_WAVE_FILE);

		CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
	}
}

// EMV Remove Card
void CSVC_Manager::Client_Screen_EMV_RemoveCard(BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_EMV_RemoveCard Screen"));

	CString strMsg;
	CString strCommand, strTextValue;

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_nProcessCount > 0)
		{
			// Transaction Result 확인 후 Message Add
			if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
				strMsg = CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_004);
			else
				strMsg = CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_013);
		}
		else
		{
			if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMVDeclined == TRUE)
				strMsg = CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_004);
		}

		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_EMVREMOVECARD);

		// Added Data
		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
		if (strMsg.IsEmpty())
			strTextValue = CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_012);
		else
			strTextValue.Format(_T("%s\n%s"), strMsg, CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_012));

		CSCR_Manager::GetInstance()->SetVariable(strCommand, strTextValue);

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREEN_TIMEOUT));

		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_EMVREMOVECARD);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		if (CEagleDataManager::GetInstance()->m_Client_Info.m_nProcessCount > 0)
		{
			if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
			{
				// Host Decline은 이전에 하나, Card쪽 에러인 경우에는 여기서 wave를 표시함
				if (CEagleTritonMsg::GetInstance()->m_sTriton_TranResp.m_strResponseCode_3 == _T("000"))
					CDEV_Manager::GetInstance()->AddWaveFile(DECLINED_WAVE_FILE);
			}
			else
			{
				CDEV_Manager::GetInstance()->AddWaveFile(APPROVED_WAVE_FILE);
			}
		}
		else
		{
			if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMVDeclined == TRUE)
				CDEV_Manager::GetInstance()->AddWaveFile(DECLINED_WAVE_FILE);
		}

		CDEV_Manager::GetInstance()->AddWaveFile(EMV_REMOVECARD_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile();
	}
}


// Cash Dispensing
void CSVC_Manager::Client_Screen_CashDispensing(BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_CashDispensing Screen"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_CASHDISPENSING);

		// Added Data (Transaction Advertisement)


		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_CASHDISPENSING);		
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(DISPENSING_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
	}
}


// Get Cash
void CSVC_Manager::Client_Screen_GetCash(BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_GetCash Screen"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_GET_CASH);

		// Added Data (Transaction Advertisement)

		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_GET_CASH);

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_3000MS);		// Get Cash Flicker 처리를 위해 2초 -> 3초로 Timing 변경
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(TAKECASH_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddWaveFile(LOCATION_CASH_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
	}
}


// Print Receipt
void CSVC_Manager::Client_Screen_PrintReceipt(BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_PrintReceipt Screen"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_PRINTING_RECEIPT);

		// Added Data

		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_PRINTING_RECEIPT);
	}
	else
	{
		Client_Screen_VoiceGuidance();

		CDEV_Manager::GetInstance()->AddWaveFile(PRINTINGRECEIPT_WAVE_FILE);
		CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
	}
}


// Display Screen Receipt
void CSVC_Manager::Client_Screen_DisplayScreenReceipt(BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_DisplayScreenReceipt Screen"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		CString strCommand, strTextValue, strCtrlButtonValue;
		CString strItemText[3];
		CString strTemp, strTemp2;
		int		nItemCount = 0;

		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_SCREEN_RECEIPT);

		// Added Data
		CString strTransType = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType;

		// 1. Transfer Amount
		if (strTransType == S_TRANSFER)
		{
			// 이체 금액
			strItemText[nItemCount] = CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_008);
			strItemText[nItemCount] += SCR_LINEFEED_DELEMITER;
			strTemp.Format(_T("%s %s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransferAmount));
			strItemText[nItemCount] += strTemp;
			nItemCount++;
		}

		// 2. 잔액
		strTemp2 = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strBalance;
		//if ((CUtil::StringToInt(strTemp2) > 0) && (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE))
		if (CUtil::StringToInt(strTemp2) > 0)		// 명세표 Print중 장애시 화면 표시를 위해 로직 수정 (이전 장애시 처리는 상위국에서 처리)
		{
			strTemp.Empty();

			// 잔액
			strItemText[nItemCount] = CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_009);
			strItemText[nItemCount] += SCR_LINEFEED_DELEMITER;
			strTemp.Format(_T("%s %s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
			strItemText[nItemCount] += strTemp;
			nItemCount++;
		}

		// 3. 가능 잔액
		strTemp2 = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strAvailableBalance;
		//if ((CUtil::StringToInt(strTemp2) > 0) && (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE))
		if (CUtil::StringToInt(strTemp2) > 0)		// 명세표 Print중 장애시 화면 표시를 위해 로직 수정 (이전 장애시 처리는 상위국에서 처리)
		{
			strTemp.Empty();

			strItemText[nItemCount] = CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_010);
			strItemText[nItemCount] += SCR_LINEFEED_DELEMITER;
			strTemp.Format(_T("%s %s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
			strItemText[nItemCount] += strTemp;
			nItemCount++;
		}

		// Screen Script 변경에 따른 코드 변경 (APState 제거됨)
		switch(nItemCount)
		{
		case 1:
			strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
			strTextValue = strItemText[0];
			CSCR_Manager::GetInstance()->SetVariable(strCommand, strTextValue);

			//strCommand.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, 1);

			//strCtrlButtonValue = _T("on");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("on");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("off");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("off");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("off");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("off");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//CSCR_Manager::GetInstance()->SetVariable(strCommand, strCtrlButtonValue);
			break;

		case 2:
			strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 2);
			strTextValue = strItemText[0];
			strTextValue += SCR_CMD_DELIMITER;
			strTextValue += strItemText[1];
			strTextValue += SCR_CMD_DELIMITER;
			CSCR_Manager::GetInstance()->SetVariable(strCommand, strTextValue);

			//strCommand.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, 1);

			//strCtrlButtonValue = _T("off");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("off");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("on");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("on");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("off");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("off");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//CSCR_Manager::GetInstance()->SetVariable(strCommand, strCtrlButtonValue);
			break;

		case 3:
			strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 4);
			strTextValue = strItemText[0];
			strTextValue += SCR_CMD_DELIMITER;
			strTextValue += strItemText[1];
			strTextValue += SCR_CMD_DELIMITER;
			strTextValue += strItemText[2];
			strTextValue += SCR_CMD_DELIMITER;
			CSCR_Manager::GetInstance()->SetVariable(strCommand, strTextValue);

			//strCommand.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, 1);

			//strCtrlButtonValue = _T("off");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("off");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("off");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("off");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("on");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//strCtrlButtonValue += _T("on");
			//strCtrlButtonValue += SCR_CMD_DELIMITER;

			//CSCR_Manager::GetInstance()->SetVariable(strCommand, strCtrlButtonValue);
			break;

		default:
			{
				LOG(Error, _T("Screen receipt item count error (%d)"), nItemCount);

				// Count가 잘못된 경우 빈 Screen이 나오도록 보완 처리
				//strCommand.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, 1);

				//strCtrlButtonValue = _T("off");
				//strCtrlButtonValue += SCR_CMD_DELIMITER;

				//strCtrlButtonValue += _T("off");
				//strCtrlButtonValue += SCR_CMD_DELIMITER;

				//strCtrlButtonValue += _T("off");
				//strCtrlButtonValue += SCR_CMD_DELIMITER;

				//strCtrlButtonValue += _T("off");
				//strCtrlButtonValue += SCR_CMD_DELIMITER;

				//strCtrlButtonValue += _T("off");
				//strCtrlButtonValue += SCR_CMD_DELIMITER;

				//strCtrlButtonValue += _T("off");
				//strCtrlButtonValue += SCR_CMD_DELIMITER;

				//CSCR_Manager::GetInstance()->SetVariable(strCommand, strCtrlButtonValue);
			}
			break;
		}

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(CLIENT_SCREENRECEIPT_TIMEOUT));


		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_SCREEN_RECEIPT);
	}
	else
	{
		Client_Screen_VoiceGuidance();
	}
}


void CSVC_Manager::Client_Screen_TransactionComplete(BOOL bADAMode)
{
	LOG(Info, _T("Client_Screen_TransactionComplete Screen"));

	bADAMode = Is_DIO_Ear_Jack_Insert();

	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	if (bADAMode == FALSE)
	{
		// Exit Message 화면 표시
		BOOL	bIsExitMessage = FALSE;
		CString strTemp, strMsg;

		for(int i=0; i<3; i++)
		{
			strTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strFarewell_message[i];
			strTemp.TrimLeft();
			strTemp.TrimRight();

			if (strTemp.GetLength() > 0)
				bIsExitMessage = TRUE;

			if (strMsg.IsEmpty() == FALSE)	// 2번째 line부터 nulti line 처리
				strMsg += _T("\n");

			strMsg += strTemp;
		}

		if (bIsExitMessage)
		{
			LOG(Info, _T("Display Exit Message Screen"));

			CString strCommand, strTextValue;

			// Screen Data Make
			CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_EXIT_MESSAGE);

			// Added Data
			strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
			strTextValue = strMsg;
			CSCR_Manager::GetInstance()->SetVariable(strCommand, strTextValue);

			// Display Screen
			CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_EXIT_MESSAGE);

			CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
		}
	}
	else
	{
		// Thanks for using our ATM. Please remove your earphone.
		// 정상인 경우에만 Guide 문구 처리
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE)
			Client_ADA_Guide_BalanceInfo(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strBalance, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strAvailableBalance);
	}

	// 고객 변경
	Client_ChangeCustom();
}


void CSVC_Manager::AMS_Screen_RemoteProgress()
{
	LOG(Info, _T("AMS_Screen_RemoteProgress Screen"));

	// Screen Data Make
	CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_AMSPROGRESS_SCR_DISPLAY);

	// Display Screen
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_AMSPROGRESS_SCR_DISPLAY);
}

