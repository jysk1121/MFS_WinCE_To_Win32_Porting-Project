#include "stdafx.h"
#include "SVC_Manager.h"

#include "EagleSVCLib.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/** **********************************************************
*	@brief		AMS Modem Main
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_ConfigureAMS_Modem()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	//int nMaxFocusIndex = 3;
	int nMaxFocusIndex = 5;
	CString strTemp, strRMSEnable, strRMSStatusEnable, strRMSPhoneNumber, strRMSPassword, strUploadJournalEnable, strUploadJournalCount;
	int nTempFocusIndex = 0;

	LOG(Info, _T("AMS Modem - Start"));

	strRMSEnable = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg;
	strRMSStatusEnable = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg;
	strRMSPhoneNumber = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_primary_num;
	strRMSPhoneNumber.TrimLeft();
	strRMSPhoneNumber.TrimRight();
	strRMSPassword = CEagleDataManager::GetInstance()->m_Config.m_System.strRms_Password;

	strUploadJournalEnable = CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_flg;
	strUploadJournalCount.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_count));

	m_nCurrentFocusIndex = 1;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// Screen Data
			strScreenInform = GET_ENDISABLE_STRING(strRMSEnable);
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += GET_ENDISABLE_STRING(strRMSStatusEnable);
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strRMSPhoneNumber;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += GET_ENDISABLE_STRING(strUploadJournalEnable);
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strUploadJournalCount;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_AMS_CONFIGURE_MODEM, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_ConfigureAMS_Modem - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER || m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				nTempFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));

				if( ((3 == nTempFocusIndex) || (4 == nTempFocusIndex) || (5 == nTempFocusIndex)) && (S_DISABLE == strRMSStatusEnable) )
				{
					Operator_Processing_Screen(OP_GUIDE_TOUCH_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bShowScreen = TRUE;
					continue;
				}

				if( (5 == nTempFocusIndex) && (S_DISABLE == strUploadJournalEnable) )
				{
					Operator_Processing_Screen(OP_GUIDE_TOUCH_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bShowScreen = TRUE;
					continue;
				}

				m_nCurrentFocusIndex = nTempFocusIndex;
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 1)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				if (m_nCurrentFocusIndex > 2)
				{
					if (strRMSStatusEnable == S_DISABLE)
					{
						m_nCurrentFocusIndex = 2;
					}
					else
					{
						if (m_nCurrentFocusIndex == nMaxFocusIndex)
						{
							if (strUploadJournalEnable == S_DISABLE)
								m_nCurrentFocusIndex--;
						}
					}
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 1;

				if (m_nCurrentFocusIndex > 2)
				{
					if (strRMSStatusEnable == S_DISABLE)
					{
						m_nCurrentFocusIndex = 1;
					}
					else
					{
						if (m_nCurrentFocusIndex == nMaxFocusIndex)
						{
							if (strUploadJournalEnable == S_DISABLE)
								m_nCurrentFocusIndex = 1;
						}
					}
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// AMS Activate
					{
						if (strRMSEnable == S_ENABLE)
							strRMSEnable = S_DISABLE;
						else
							strRMSEnable = S_ENABLE;

						bShowScreen = TRUE;
					}
					break;

				case 2:	// AMS Status Send
					{
						if (strRMSStatusEnable == S_ENABLE)
						{
							strRMSStatusEnable = S_DISABLE;

							// AMS Status가 Disable인 경우에는 Upload Journal도 Disable처리가 되어야 함
							strUploadJournalEnable = S_DISABLE;
						}
						else
						{
							strRMSStatusEnable = S_ENABLE;
						}

						bShowScreen = TRUE;
					}
					break;
				case 3:	// AMS Phone Number
					{
						if (Operator_Input_Text(_T("AMS PHONE NUMBER"), strRMSPhoneNumber, m_strScrOutData) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();
							strRMSPhoneNumber = m_strScrOutData;
						}

						bShowScreen = TRUE;
					}
					break;

				case 4:	// Upload Journal
					{
						if (strUploadJournalEnable == S_ENABLE)
							strUploadJournalEnable = S_DISABLE;
						else
							strUploadJournalEnable = S_ENABLE;

						bShowScreen = TRUE;
					}
					break;

				case 5:	// Upload Journal /count
					{
						if (Operator_Input_Number(_T("JOURNAL COUNT"), strUploadJournalCount, m_strScrOutData, 3) == TRUE)	// Range 1~ 999
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();

							if (CUtil::IsNumeric(m_strScrOutData) == TRUE)
							{
								int nCount = CUtil::StringToInt(m_strScrOutData);

								if (nCount > 0)
								{
									strUploadJournalCount.Format(_T("%d"), nCount);
								}
								else
								{
									// Invalid Data Notice
									LOG(Info, _T("Upload Journal Data is Invalid (%d)"), nCount);

									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
							}
							else
							{
								// Invalid Data Notice
								LOG(Info, _T("Upload Journal Data is Invalid (%s)"), m_strScrOutData);

								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}

						bShowScreen = TRUE;
					}
					break;

				default:
					break;
				}

			}
			else if (m_strScrOutData == _T("SET"))
			{
				CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg = strRMSEnable;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg = strRMSStatusEnable;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_primary_num = strRMSPhoneNumber;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_flg = strUploadJournalEnable;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_count = strUploadJournalCount;

				CEagleDataManager::GetInstance()->m_Config.SaveAMSData(_T("rms_en"), CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg);
				CEagleDataManager::GetInstance()->m_Config.SaveAMSData(_T("rms_status_en"), CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg);
				CEagleDataManager::GetInstance()->m_Config.SaveAMSData(_T("primary_num"), CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_primary_num);
				CEagleDataManager::GetInstance()->m_Config.SaveAMSData(_T("schedule_jnl_en"), CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_flg);
				CEagleDataManager::GetInstance()->m_Config.SaveAMSData(_T("schedule_jnl_cnt"), CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_count);

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				// Result 표시 후 화면 다시 표시
				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("PASSWORD"))
			{
				Operator_UserPasswordChange_Screen(_T("AMS PASSWORD"));
				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}
}


/** **********************************************************
*	@brief		AMS TCPIP Main
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_ConfigureAMS_TCPIP()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
//	int nMaxFocusIndex = 6;
	int nMaxFocusIndex = 7;
	CString strTemp, strRMSEnable, strRMSStatusEnable, strRMSIPAddress, strRMSPort, strUploadJournalEnable, strUploadJournalCount;
	int nTempFocusIndex = 0;
	CString strType;

	strRMSEnable = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg;
	strRMSStatusEnable = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg;
	strRMSIPAddress = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_IP_Address;
	strRMSIPAddress.TrimLeft();
	strRMSIPAddress.TrimRight();
	strRMSPort.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Port));

	strUploadJournalEnable = CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_flg;
	strUploadJournalCount.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_count));

	m_nCurrentFocusIndex = 1;

	if(strRMSIPAddress.GetAt(0) >= '0' && strRMSIPAddress.GetAt(0) <= '9')
		strType = _T("0");	// IP TYPE
	else
		strType = _T("1");

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// Screen Data
			// 1. AMS Enable
			strScreenInform = GET_ENDISABLE_STRING(strRMSEnable);
			strScreenInform += SCR_CMD_DELIMITER;

			// 2. AMS Status Enable
			strScreenInform += GET_ENDISABLE_STRING(strRMSStatusEnable);
			strScreenInform += SCR_CMD_DELIMITER;

			// 3. Type
			if (strType == _T("0"))
				strScreenInform += _T("IP");
			else
				strScreenInform += _T("URL");
			strScreenInform += SCR_CMD_DELIMITER;

			// 4. AMS IP
			strScreenInform += strRMSIPAddress;
			strScreenInform += SCR_CMD_DELIMITER;

			// 5. AMS Port
			strScreenInform += strRMSPort;
			strScreenInform += SCR_CMD_DELIMITER;

			// 6. JNL Upload Enable
			strScreenInform += GET_ENDISABLE_STRING(strUploadJournalEnable);
			strScreenInform += SCR_CMD_DELIMITER;

			// 7. JNL Count
			strScreenInform += strUploadJournalCount;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_AMS_CONFIGURE_TCPIP, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_ConfigureAMS_TCPIP - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER || m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				nTempFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));

				if( ((3 == nTempFocusIndex) || (4 == nTempFocusIndex) || (5 == nTempFocusIndex) || (6 == nTempFocusIndex) || (7 == nTempFocusIndex)) && (S_DISABLE == strRMSStatusEnable) )
				{
					Operator_Processing_Screen(OP_GUIDE_TOUCH_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bShowScreen = TRUE;
					continue;
				}

			//	if( (6 == nTempFocusIndex) && (S_DISABLE == strUploadJournalEnable) )
				if( (7 == nTempFocusIndex) && (S_DISABLE == strUploadJournalEnable) )
				{
					Operator_Processing_Screen(OP_GUIDE_TOUCH_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bShowScreen = TRUE;
					continue;
				}

				m_nCurrentFocusIndex = nTempFocusIndex;
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 1)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				if (m_nCurrentFocusIndex > 2)
				{
					if (strRMSStatusEnable == S_DISABLE)
					{
						m_nCurrentFocusIndex = 2;
					}
					else
					{
						if (m_nCurrentFocusIndex == nMaxFocusIndex)
						{
							if (strUploadJournalEnable == S_DISABLE)
								m_nCurrentFocusIndex--;
						}
					}
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 1;

				if (m_nCurrentFocusIndex > 2)
				{
					if (strRMSStatusEnable == S_DISABLE)
					{
						m_nCurrentFocusIndex = 1;
					}
					else
					{
						if (m_nCurrentFocusIndex == nMaxFocusIndex)
						{
							if (strUploadJournalEnable == S_DISABLE)
								m_nCurrentFocusIndex = 1;
						}
					}
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// AMS Activate
					{
						if (strRMSEnable == S_ENABLE)
							strRMSEnable = S_DISABLE;
						else
							strRMSEnable = S_ENABLE;

						bShowScreen = TRUE;
					}
					break;

				case 2:	// AMS Status Send
					{
						if (strRMSStatusEnable == S_ENABLE)
						{
							strRMSStatusEnable = S_DISABLE;

							// AMS Status가 Disable인 경우에는 Upload Journal도 Disable처리가 되어야 함
							strUploadJournalEnable = S_DISABLE;
						}
						else
						{
							strRMSStatusEnable = S_ENABLE;
						}

						bShowScreen = TRUE;
					}
					break;
				case 3:	// Type
					{
						if (strType == _T("0"))
							strType = _T("1");
						else
							strType = _T("0");

						bShowScreen = TRUE;
					}
					break;
				case 4:	// AMS IP Address
					{
						if (strType == _T("0"))
						{
							if (Operator_Input_Number(_T("AMS IP ADDRESS"), strRMSIPAddress, m_strScrOutData, 15) == TRUE)
							{
								m_strScrOutData.TrimLeft();
								m_strScrOutData.TrimRight();
								strRMSIPAddress = m_strScrOutData;
							}
						}
						else
						{
							if (Operator_Input_Text(_T("AMS URL ADDRESS"), strRMSIPAddress, m_strScrOutData) == TRUE)
							{
								m_strScrOutData.TrimLeft();
								m_strScrOutData.TrimRight();
								strRMSIPAddress = m_strScrOutData;
							}
						}

						bShowScreen = TRUE;
					}
					break;

				case 5:	// AMS Port
					{
						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 65535);

						if (Operator_Input_Number(_T("AMS PORT"), strRMSPort, m_strScrOutData, 5, NORMAL_TYPE, strGuideText) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();

							if (CUtil::IsNumeric(m_strScrOutData) == TRUE)
							{
								int nPort = CUtil::StringToInt(m_strScrOutData);

								if (nPort < 0 || nPort > 65535)
								{
									// Invalid Data Notice
									LOG(Info, _T("AMS Port Data is Invalid (%d)"), nPort);

									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
								else
								{
									strRMSPort.Format(_T("%d"), nPort);
								}
							}
							else
							{
								// Invalid Data Notice
								LOG(Info, _T("AMS Port Data is Invalid (%s)"), m_strScrOutData);

								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}

						bShowScreen = TRUE;
					}
					break;

				case 6:	// Upload Journal
					{
						if (strUploadJournalEnable == S_ENABLE)
							strUploadJournalEnable = S_DISABLE;
						else
							strUploadJournalEnable = S_ENABLE;

						bShowScreen = TRUE;
					}
					break;

				case 7:	// Upload Journal /count
					{
						if (Operator_Input_Number(_T("JOURNAL COUNT"), strUploadJournalCount, m_strScrOutData, 3) == TRUE)	// Range 1~ 999
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();

							if (CUtil::IsNumeric(m_strScrOutData) == TRUE)
							{
								int nCount = CUtil::StringToInt(m_strScrOutData);

								if (nCount > 0)
								{
									strUploadJournalCount.Format(_T("%d"), nCount);
								}
								else
								{
									// Invalid Data Notice
									LOG(Info, _T("Upload Journal Data is Invalid (%d)"), nCount);

									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
							}
							else
							{
								// Invalid Data Notice
								LOG(Info, _T("Upload Journal Data is Invalid (%s)"), m_strScrOutData);

								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}

						bShowScreen = TRUE;
					}
					break;

				default:
					break;
				}

			}
			else if (m_strScrOutData == _T("SET"))
			{
				LOG(Info, _T("AMS TCP/IP - Set Data"));

				CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg = strRMSEnable;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg = strRMSStatusEnable;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_IP_Address = strRMSIPAddress;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Port = strRMSPort;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_flg = strUploadJournalEnable;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_count = strUploadJournalCount;

				CEagleDataManager::GetInstance()->m_Config.SaveAMSData(_T("rms_en"), CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg);
				CEagleDataManager::GetInstance()->m_Config.SaveAMSData(_T("rms_status_en"), CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg);
				CEagleDataManager::GetInstance()->m_Config.SaveAMSData(_T("rms_ip_address"), CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_IP_Address);
				CEagleDataManager::GetInstance()->m_Config.SaveAMSData(_T("rms_port"), CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Port);
				CEagleDataManager::GetInstance()->m_Config.SaveAMSData(_T("schedule_jnl_en"), CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_flg);
				CEagleDataManager::GetInstance()->m_Config.SaveAMSData(_T("schedule_jnl_cnt"), CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_count);


				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				// Result 표시 후 화면 다시 표시
				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("PASSWORD"))
			{
				Operator_UserPasswordChange_Screen(_T("AMS PASSWORD"));

				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}
}
