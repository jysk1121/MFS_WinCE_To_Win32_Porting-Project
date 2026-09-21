#include "stdafx.h"
#include "SVC_Manager.h"

#include "EagleSVCLib.h"
#include "EagleTritonMsg.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/** **********************************************************
*	@brief		Task Main
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Task_Main()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInfo;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();
			strButtonInfo.Empty();

			// Set Button Info
			// 1. DAY CLOSE / 2. RESET SEQUENCE NO. / 3. CBX Close / 4. Add QTY. / 5. Trial Day Close / 6. Schedule Day Close
			if (GetOperatorMode() == EAGLE_ATM_SVC_REPLENISH)
			{
				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;
			}
			else
			{
				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;
			}

			Operator_DisplayScreen(OP_SCREEN_TASK, bKeyEnable, strScreenInform, strButtonInfo);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Task_Main - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER || m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("DAYCLOSE"))
			{
				if (Operator_Confirm_Screen(_T("DAY CLOSE")) == TRUE)
				{
					Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

					// DAY CLOSE 수행
					CEagleTritonMsg::GetInstance()->Triton_MakeHostMsg(EAGLE_TRAN_CODE_HOST_TOTALS);

					if (CEagleSVCLib::GetInstance()->SendRecvHost(EAGLE_TRAN_CODE_HOST_TOTALS) == SUCCESS)
					{
						// 성공시에만 명세표 Print
						if (CEagleSVCLib::GetInstance()->SaveEJL_N_Print_DayClose(EAGLE_TRAN_CODE_HOST_TOTALS, TRUE, CEagleSVCLib::GetInstance()->IsPTR_Available()) == TRUE)
							Operator_Processing_Screen(_T("SUCCESS"));
						else
							Operator_Processing_Screen(_T("SUCCESS(PRINT ERROR)"));
					}
					else
					{
						// Day Close Fail시에 Information Journal을 남기도록 보완
						CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, _T("FAILED DAY CLOSE"));
						Operator_Processing_Screen(_T("FAILED"));
					}

					// Day Total중 Host 통신 장애로 인한 장애는 자동 Clear 처리
					if (CEagleSVCLib::GetInstance()->Get_ErrorCode().Left(2) == _T("Z3"))
						CEagleSVCLib::GetInstance()->Clear_Error();

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("TRIALDAYCLOSE"))
			{
				if (Operator_Confirm_Screen(_T("TRIAL DAY CLOSE")) == TRUE)
				{
					Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

					// Trial Day Close 수행
					CEagleTritonMsg::GetInstance()->Triton_MakeHostMsg(EAGLE_TRAN_CODE_GET_HOST_TOTALS);

					if (CEagleSVCLib::GetInstance()->SendRecvHost(EAGLE_TRAN_CODE_GET_HOST_TOTALS) == SUCCESS)
					{
						// 명세표 Print
						if (CEagleSVCLib::GetInstance()->SaveEJL_N_Print_DayClose(EAGLE_TRAN_CODE_GET_HOST_TOTALS, TRUE, CEagleSVCLib::GetInstance()->IsPTR_Available()) == TRUE)
							Operator_Processing_Screen(_T("SUCCESS"));
						else
							Operator_Processing_Screen(_T("SUCCESS(PRINT ERROR)"));
					}
					else
					{
						if (CEagleSVCLib::GetInstance()->SaveEJL_N_Print_DayClose(EAGLE_TRAN_CODE_GET_HOST_TOTALS, FALSE, CEagleSVCLib::GetInstance()->IsPTR_Available()) == TRUE)
							Operator_Processing_Screen(_T("FAILED"));
						else
							Operator_Processing_Screen(_T("FAILED(PRINT ERROR)"));
					}

					// Trial Day Total중 Host 통신 장애로 인한 장애는 자동 Clear 처리
					if (CEagleSVCLib::GetInstance()->Get_ErrorCode().Left(2) == _T("Z3"))
						CEagleSVCLib::GetInstance()->Clear_Error();

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("RESETSEQUENCENO"))
			{
				if (Operator_Confirm_Screen(_T("RESET SEQUENCE NO.")) == TRUE)
				{
					CEagleDataManager::GetInstance()->m_Config.m_System.strSequence_Number = _T("0000");
					CEagleDataManager::GetInstance()->m_Config.SaveSystem();

					Operator_Processing_Screen(_T("SUCCESS"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("CBXCLOSE"))
			{
				if (Operator_Confirm_Screen(_T("CBX CLOSE")) == TRUE)
				{
					BOOL bResult = FALSE;

					Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

					// 명세표 Print
					bResult = CEagleSVCLib::GetInstance()->SaveEJL_N_Print_CBXClose(EAGLE_TRAN_CODE_CBX_TOTALS);

					// 수행 성공 후 Terminal Monitoring이 Enable일 경우 Host로 Configuration message 전송 (매수 변동 발생 notice)
					if (CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring == S_ENABLE)
					{
						if (m_bExecDownloadKeysToHost_PowerOn == TRUE)	// Power On시 Configuration을 수행한 후에 진행을 하도록 로직 추가
						{
							Operator_Processing_Screen(_T("HEARTBEATS SENDING..."));

							// Configuration 수행 (성공/실패는 Check 안함)
							CEagleTritonMsg::GetInstance()->Triton_MakeHostMsg(EAGLE_TRAN_CODE_DOWNLOAD_CONFIG);
							CEagleSVCLib::GetInstance()->SendRecvHost(EAGLE_TRAN_CODE_DOWNLOAD_CONFIG);

							// Host 통신 장애로 인한 장애는 자동 Clear 처리
							if (CEagleSVCLib::GetInstance()->Get_ErrorCode().Left(2) == _T("Z3"))
								CEagleSVCLib::GetInstance()->Clear_Error();
						}
					}
					
					if (bResult == TRUE)
					{
						Operator_Processing_Screen(_T("SUCCESS"));
					}
					else
					{
						Operator_Processing_Screen(_T("SUCCESS(PRINT ERROR)"));
					}

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("TRIALCBXCLOSE"))
			{
				if (Operator_Confirm_Screen(_T("TRIAL CBX CLOSE")) == TRUE)
				{
					BOOL bResult = FALSE;

					Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

					// 명세표 Print
					bResult = CEagleSVCLib::GetInstance()->SaveEJL_N_Print_CBXClose(EAGLE_TRAN_CODE_TRIAL_CBX_TOTALS);

					if (bResult == TRUE)
						Operator_Processing_Screen(_T("SUCCESS"));
					else
						Operator_Processing_Screen(_T("SUCCESS(PRINT ERROR)"));

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("ADDCBX"))
			{
				m_strNextScrNum = OP_SCREEN_ADD_QTY_IN_CASSETTE;
				break;
			}
			else if (m_strScrOutData == _T("SCHEDULEDAYCLOSE"))
			{
				// RIGHT 2
				m_strNextScrNum = OP_SCREEN_SCHEDULE_DAY_CLOSE;
				break;
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
*	@brief		Add Cash
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_AddQTYinCST()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;

	CString strCmd;
	CString strScreenInform;
	CString strTemp;
	CString strValue[MAX_CST_COUNT];
	CString strPrevCBXStatus, strCurCBXStatus;
	CString strTitle;

	CStringArray	strTempArray;

	int		nMaxFocusIndex = 8, i = 0, nCSTCnt = 0, nTemp = 0;
	int		nCurrentCBXCnt[MAX_CST_COUNT];
	int		nAddedCBXCount[MAX_CST_COUNT];
	int		nTempFocusIndex = 0;

	// 카세트 상태 취득
	DWORD nCbxStatus[MAX_CASSETTE+1];
	memset(nCbxStatus, 0, sizeof(nCbxStatus));

	memset(nCurrentCBXCnt, 0, sizeof(nCurrentCBXCnt));
	memset(nAddedCBXCount, 0, sizeof(nAddedCBXCount));

	// Focus Setting
	m_nCurrentFocusIndex = 5;

	nCSTCnt = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);
	nMaxFocusIndex = nCSTCnt + 4;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		// CBX Status 추가 (0은 Reject Bin임)
		strCurCBXStatus.Empty();
		CDEV_Manager::GetInstance()->m_DEV_CDM.GetCbxStatus(nCbxStatus);

		for(i=1; i<MAX_CST_COUNT + 1; i++)
		{
			if (i > nCSTCnt)
			{
				strCurCBXStatus += _T("N/A");
			}
			else
			{
				if (nCbxStatus[i] == WFS_CDM_STATCUMISSING)
					strCurCBXStatus += _T("MISSING");
				else if (nCbxStatus[i] == WFS_CDM_STATCULOW)
					strCurCBXStatus += _T("LOW");
				else if (nCbxStatus[i] == WFS_CDM_STATCUOK)
					strCurCBXStatus += _T("NORMAL");
				else if (nCbxStatus[i] == WFS_CDM_STATCUEMPTY)
					strCurCBXStatus += _T("EMPTY");
				else
					strCurCBXStatus += _T("UNKNOWN");
			}

			strCurCBXStatus += SCR_CMD_DELIMITER;
		}

		if (strPrevCBXStatus != strCurCBXStatus)
		{
			strPrevCBXStatus = strCurCBXStatus;
			bShowScreen = TRUE;
		}

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();
			strTempArray.RemoveAll();

			// CBX #1 ~ #4
			for(i=0; i<MAX_CST_COUNT; i++)
			{
				if (i < nCSTCnt)
				{
					nCurrentCBXCnt[i] = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);
					strTemp.Format(_T("%s"), CUtil::ConvertFromValueToAmountwithCent(CUtil::IntToString(nCurrentCBXCnt[i]), FALSE));
				}
				else
				{
					strTemp = _T("N/A");
				}

				strScreenInform += strTemp;
				strScreenInform += SCR_CMD_DELIMITER;
			}

			// Intput Value 5 ~ 8
			for(i=0; i<MAX_CST_COUNT; i++)
			{
				if (i < nCSTCnt)
				{
					strScreenInform += strValue[i];
					strScreenInform += SCR_CMD_DELIMITER;
				}
				else
				{
					strScreenInform += _T("N/A");
					strScreenInform += SCR_CMD_DELIMITER;
				}
			}

			// CBX Status 추가
			strScreenInform += strCurCBXStatus;

			Operator_DisplayScreen(OP_SCREEN_ADD_QTY_IN_CASSETTE, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_AddQTYinCST - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_TASK;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				nTempFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));

				if(nTempFocusIndex > nMaxFocusIndex)
					continue;

				m_nCurrentFocusIndex = nTempFocusIndex;
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 5)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 5;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 5:
				case 6:
				case 7:
				case 8:
					{
						strTitle.Format(_T("CBX #%d"), m_nCurrentFocusIndex - 4);

						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, MAX_CST_NUMBEROFBILLS);

						if (Operator_Input_Number(strTitle, strValue[m_nCurrentFocusIndex-5], m_strScrOutData, 4, NORMAL_TYPE, strGuideText) == TRUE)						
						{
							nTemp = CUtil::StringToInt(m_strScrOutData);

							if ((nCurrentCBXCnt[m_nCurrentFocusIndex-5] + nTemp) <= MAX_CST_NUMBEROFBILLS)
							{
								strValue[m_nCurrentFocusIndex-5] = CUtil::IntToString(nTemp);
							}
							else
							{
								// Invalid Data Notice
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
			else
			{
				CUtil::ParsingStringToStringArray(m_strScrOutData, (CString)SCR_RES_DELIMITER, strTempArray);

				if (strTempArray.GetCount() >= 5)
				{
					// 입력한 값이 모두 0인 경우 Invalid 처리
					BOOL bInvalid = TRUE;

					for(i=0; i<MAX_CST_COUNT; i++)
					{
						if (CUtil::StringToInt(strTempArray[i+1]) > 0)
						{
							bInvalid = FALSE;
							break;
						}
					}

					if (bInvalid == FALSE)
					{
						if (strTempArray[0] == _T("SET"))
						{
							Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

							// ADD CASH 수행
							for(i=0; i<nCSTCnt; i++)
							{
								// Added Current Count
								nAddedCBXCount[i] = CUtil::StringToInt(strTempArray[i+1]);
								nTemp = (CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]) + nAddedCBXCount[i]);
								CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i] = CUtil::IntToString(nTemp);

								// Added load Count
								nTemp = (CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Load_Count[i]) + nAddedCBXCount[i]);
								CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Load_Count[i] = CUtil::IntToString(nTemp);
							}

							CEagleDataManager::GetInstance()->m_Config.SaveCbxInfo();

							// EJL save and Print
							if (CEagleSVCLib::GetInstance()->SaveEJL_N_Print_CBXAddQTY(nAddedCBXCount, CEagleSVCLib::GetInstance()->IsPTR_Available()) == TRUE)
							{
								if (CEagleSVCLib::GetInstance()->IsPTR_Available() == TRUE)
									Operator_Processing_Screen(_T("SUCCESS"));
								else
									Operator_Processing_Screen(_T("SUCCESS(PRINT ERROR)"));
							}
							else
							{
								Operator_Processing_Screen(_T("SUCCESS(PRINT ERROR)"));
							}
							CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

							// 수행 성공 후 Terminal Monitoring이 Enable일 경우 Host로 Configuration message 전송 (매수 변동 발생 notice)
							if (CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring == S_ENABLE)
							{
								if (m_bExecDownloadKeysToHost_PowerOn == TRUE)	// Power On시 Configuration을 수행한 후에 진행을 하도록 로직 추가
								{
									Operator_Processing_Screen(_T("HEARTBEATS SENDING..."));

									// Configuration 수행 (성공/실패는 Check 안함)
									CEagleTritonMsg::GetInstance()->Triton_MakeHostMsg(EAGLE_TRAN_CODE_DOWNLOAD_CONFIG);
									CEagleSVCLib::GetInstance()->SendRecvHost(EAGLE_TRAN_CODE_DOWNLOAD_CONFIG);

									// Host 통신 장애로 인한 장애는 자동 Clear 처리
									if (CEagleSVCLib::GetInstance()->Get_ErrorCode().Left(2) == _T("Z3"))
										CEagleSVCLib::GetInstance()->Clear_Error();
								}
							}

							// Result 표시 후 화면 다시 표시
							m_nCurrentFocusIndex = 5;

							for(i=0; i<MAX_CST_COUNT; i++)
							{
								strValue[i].Empty();
								nAddedCBXCount[i] = 0;	// 초기화
							}
						}
						else
						{
							// Invalid Data Notice
							Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
							CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
						}
					}
					else
					{
						// Invalid Data Notice
						Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					}
				}
				else
				{
					// Invalid Data Notice
					Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				m_nCurrentFocusIndex = 5;
				bShowScreen = TRUE;
			}
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}
}


/** **********************************************************
*	@brief		Schedule Day close관련 시간 및 사용여부 설정
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_ScheduleDayClose()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	int		nMaxFocusIndex = 3;
	CString strTemp, strScheduleClose, strScheduleHour, strSchedulMinutes;

	// Focus Setting
	m_nCurrentFocusIndex = 1;

	strScheduleClose = CEagleDataManager::GetInstance()->m_Config.m_Option.strScheduleDayClose;
	strScheduleHour.Format(_T("%02d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime.Left(2)));
	strSchedulMinutes.Format(_T("%02d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime.Right(2)));

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// 1. Schedule Day Close
			strScreenInform = GET_ENDISABLE_STRING(strScheduleClose);
			strScreenInform += SCR_CMD_DELIMITER;

			// 2. Day Close Hour
			strScreenInform += strScheduleHour;
			strScreenInform += SCR_CMD_DELIMITER;

			// 3. Day Close Minutes
			strScreenInform += strSchedulMinutes;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_SCHEDULE_DAY_CLOSE, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_ScheduleDayClose - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_TASK;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 1)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 1;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// Enable or Disable
					{
						if (strScheduleClose == S_ENABLE)
							strScheduleClose = S_DISABLE;
						else
							strScheduleClose = S_ENABLE;

						bShowScreen = TRUE;
					}
					break;

				case 2:	// Hour
					{
						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 23);

						if (Operator_Input_Number(_T("HOUR"), strScheduleHour, m_strScrOutData, 2, NORMAL_TYPE, strGuideText) == TRUE)
						{
							if (m_strScrOutData.IsEmpty() == FALSE)
							{
								int nHour = CUtil::StringToInt(m_strScrOutData);

								if ( nHour >= 0 && nHour <= 23)
								{
									strScheduleHour.Format(_T("%02d"), nHour);
								}
								else
								{
									// Invalid Data Notice
									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
							}
							else
							{
								// Invalid Data Notice
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}

						bShowScreen = TRUE;
					}
					break;
				case 3:	// Minutes
					{
						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 59);

						if (Operator_Input_Number(_T("MINUTE"), strSchedulMinutes, m_strScrOutData, 2, NORMAL_TYPE, strGuideText) == TRUE)
						{
							if (m_strScrOutData.IsEmpty() == FALSE)
							{
								int nMinute = CUtil::StringToInt(m_strScrOutData);

								if (nMinute >= 0 && nMinute <= 59)
								{
									strSchedulMinutes.Format(_T("%02d"), nMinute);
								}
								else
								{
									// Invalid Data Notice
									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
							}
							else
							{
								// Invalid Data Notice
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
				// 입력한 값을 바탕으로 시간을 설정한다.
				CEagleDataManager::GetInstance()->m_Config.m_Option.strScheduleDayClose = strScheduleClose;
				CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime.Format(_T("%s%s"), strScheduleHour, strSchedulMinutes);

				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("schedule_dayclose_en"), CEagleDataManager::GetInstance()->m_Config.m_Option.strScheduleDayClose);
				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("dayclose_time"), CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime);

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				// Result 표시 후 화면 다시 표시
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

