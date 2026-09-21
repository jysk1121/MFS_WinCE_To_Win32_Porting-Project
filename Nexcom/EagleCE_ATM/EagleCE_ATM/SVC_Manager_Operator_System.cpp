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
*	@brief		System Main
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_System_Main()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInfo;

	m_nCurrentFocusIndex = 1;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();
			strButtonInfo.Empty();

			strScreenInform = CEagleDataManager::GetInstance()->m_Config.m_Option.strMachine_Serial_Number;

			// Set Button Info
			// 1. DATE & TIME / 2. SYSTEM DEFAULT / 3. SERIAL NO. / 4. PASSWORD / 5. UPLOAD / 6. REBOOT / 7. BACKUP
			if (GetOperatorMode() == EAGLE_ATM_SVC_REPLENISH)
			{
				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");				// 자기 자신의 password 변경이 가능해야 하므로 off -> on으로 수정
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;
			}
			else
			{
				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				// Admin 권한은 System 초기화 불가능하도록 로직 수정
				if (GetOperatorMode() == EAGLE_ATM_SVC_MASTER)
					strButtonInfo += _T("on");
				else
					strButtonInfo += _T("off");
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

			Operator_DisplayScreen(OP_SCREEN_SYSTEM, bKeyEnable, strScreenInform, strButtonInfo);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_System_Main - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER || m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			CString strTouchName = _T("SERIALNO_TOUCH");
			if (strTouchName.CompareNoCase(m_strScrOutData.Left(strTouchName.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SERIALNO");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("DATE"))
			{
				// LEFT 1
				m_strNextScrNum = OP_SCREEN_DATE_N_TIME;
				break;

			}
			else if (m_strScrOutData == _T("SYSTEMDEFAULT"))
			{
				if (Operator_Confirm_Screen(_T("SYSTEM DEFAULT"), _T(""), _T("THE MACHINE WILL BE INITIALIZED TO FACTORY DEFAULT STATE\nAND SYSTEM WILL BE REBOOTED AUTOMATICALLY")) == TRUE)
				{
					Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

					// File Copy from Config\Default  to Config folder
					BOOL bResult = FALSE;
					CString strSource, strDestination, strTemp;

					// EJL도 초기화 해야하므로 EJL Close 후 File Copy 수행
					CDEV_Manager::GetInstance()->m_DEV_EJL.Close();
					m_EJL_Connect = FALSE;

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					strSource.Format(_T("%s%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH);
					strDestination.Format(_T("%s%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH);

					bResult = CUtil::CopyFileInDirectory(strSource, _T("*.*"), strDestination);
					LOG(Info, _T("Configure default is executed, result (%d)"), bResult);

					strTemp.Format(_T("%s%s"), CUtil::GetAppPath(), PATH_LOG);
					CUtil::DeleteFileInDirectory(strTemp, _T("*.*"));

					// Camera Image도 모두 제거하도록 로직 추가
					CString strCaptureFilePath;

					strCaptureFilePath.Format(_T("%s%s"), CUtil::GetAppPath(), CAPTURE_IMAGE_FILE_PATH);
					CUtil::DeleteFileInDirectory(strCaptureFilePath, _T("*.*"));

					// Capture Image File Index 초기화
					CEagleDataManager::GetInstance()->m_Config.m_System.strCaptureImageFile_Index = _T("0");
					CEagleDataManager::GetInstance()->m_Config.SaveSystemData(_T("backup_captureimage_index"), _T("0"));

					// Delay 2sec
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					// EJL 연결
					if (TRUE == CDEV_Manager::GetInstance()->m_DEV_EJL.Open(GetSafeHwnd()))
					{
						LOG(Info, _T("EJL Open Success!"));
						m_EJL_Connect = TRUE;
					}

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					// EXECUTE SYSTEM DEFAULT
					strTemp = _T("EXECUTE SYSTEM DEFAULT");
					CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

					// Success 후 System 재부팅
					strTemp = _T("SYSTEM REBOOT");
					CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(reboot, strTemp);

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					CDEV_Manager::GetInstance()->m_DEV_EJL.Close();
					m_EJL_Connect = FALSE;

					// Network Registry 초기화(Default DHCP) 수행
					CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP = S_ENABLE;

					// Set Network
					CDEV_Manager::GetInstance()->Set_Network_Config(TRUE);

					Operator_Processing_Screen(_T("SUCCESS"));

					// System Reboot 수행
					CDEV_Manager::GetInstance()->System_Reboot();
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SERIALNO"))
			{
				if (Operator_Input_Text(_T("SERIAL NO."), strScreenInform, m_strScrOutData, 15, NUMBER_MODE) == TRUE)
				{
					m_strScrOutData.TrimLeft();
					m_strScrOutData.TrimRight();
					CEagleDataManager::GetInstance()->m_Config.m_Option.strMachine_Serial_Number = m_strScrOutData;
					CEagleDataManager::GetInstance()->m_Config.SaveOption();

					Operator_Processing_Screen(_T("SUCCESS"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("PASSWORD"))
			{
				// LEFT 3
				m_strNextScrNum = OP_SCREEN_PASSWORD;
				break;				
			}
			else if (m_strScrOutData == _T("REBOOT"))
			{
				// LEFT 4
				m_strNextScrNum = OP_SCREEN_REBOOT;	// V1.0.2.4 2018.07.02 - 화면 추가
				break;
			}
			else if (m_strScrOutData == _T("UPLOAD"))
			{
				// RIGHT 3
				m_strNextScrNum = OP_SCREEN_UPLOAD;
				break;

			}
			else if (m_strScrOutData == _T("BACKUP"))
			{
				// RIGHT 4
				m_strNextScrNum = OP_SCREEN_BACKUP;
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
*	@brief		Date Time 설정
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_DateTime()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	BOOL bSystemTimeUpdate = TRUE;
	CString strCmd;
	CString strValue;
	CString strScreenInform;
	CString strTemp;
	int		nMaxFocusIndex = 5;
	SYSTEMTIME SystemTime;

	// Focus Setting
	m_nCurrentFocusIndex = 1;

	SYSTEMTIME	BeforeTi;
	GetLocalTime(&BeforeTi);	// Setting 전의 시간 정보


	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			if (bSystemTimeUpdate == TRUE)
			{
				GetLocalTime(&SystemTime);
				bSystemTimeUpdate = FALSE;
			}

			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// 1. Year
			strTemp.Format(_T("%04d"), SystemTime.wYear);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// 2. Month
			strTemp.Format(_T("%02d"), SystemTime.wMonth);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// 3. Day
			strTemp.Format(_T("%02d"), SystemTime.wDay);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// 4. Hour
			strTemp.Format(_T("%02d"), SystemTime.wHour);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// 5. Minute
			strTemp.Format(_T("%02d"), SystemTime.wMinute);
			strScreenInform += strTemp;

			Operator_DisplayScreen(OP_SCREEN_DATE_N_TIME, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_DateTime - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_SYSTEM;
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
				m_strScrOutData.Empty();

				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// Year
					{
						strTemp.Format(_T("%04d"), SystemTime.wYear);

						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 2037);

						if (Operator_Input_Number(_T("YEAR"), strTemp, m_strScrOutData, 4, NORMAL_TYPE, strGuideText) == TRUE)
						{
							int nYear = CUtil::StringToInt(m_strScrOutData);

							//if (nYear >= 2000 && nYear <= 2099)
							if (nYear >= 1999 && nYear <= (2038-1))		// Main B/D 초기 시간값이 1999년부터 시작되면 2038년 1월 23일까지밖에 지원 안되므로 2037년까지만 입력 가능하도록 수정
							{
								SystemTime.wYear = nYear;
							}
							else
							{
								// Invalid Notice
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}
						
						bShowScreen = TRUE;
					}
					break;

				case 2:	// Month
					{
						strTemp.Format(_T("%02d"), SystemTime.wMonth);

						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 12);

						if (Operator_Input_Number(_T("MONTH"), strTemp, m_strScrOutData, 2, NORMAL_TYPE, strGuideText) == TRUE)
						{
							int nMonth = CUtil::StringToInt(m_strScrOutData);

							if (nMonth >= 1 && nMonth <=12)
							{
								SystemTime.wMonth = nMonth;
							}
							else
							{
								// Invalid Notice
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}

						bShowScreen = TRUE;
					}
					break;
				case 3:	// Day
					{
						strTemp.Format(_T("%02d"), SystemTime.wDay);

						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 31);

						if (Operator_Input_Number(_T("DAY"), strTemp, m_strScrOutData, 2, NORMAL_TYPE, strGuideText) == TRUE)
						{
							int nDay = CUtil::StringToInt(m_strScrOutData);

							COleDateTime CheckDate;

							CheckDate = COleDateTime::GetCurrentTime();

							CheckDate.SetDate(SystemTime.wYear, SystemTime.wMonth, nDay);
							
							if (CheckDate.GetStatus() == COleDateTime::valid)
							{
								SystemTime.wDay = nDay;
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

				case 4:	// Hour
					{
						strTemp.Format(_T("%02d"), SystemTime.wHour);

						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 23);

						if (Operator_Input_Number(_T("HOUR"), strTemp, m_strScrOutData, 2, NORMAL_TYPE, strGuideText) == TRUE)
						{
							int nHour = CUtil::StringToInt(m_strScrOutData);

							// V1.0.2.4 2018.07.03 - NULL Check 추가
							if( (m_strScrOutData.IsEmpty() == TRUE) || (nHour < 0) || (nHour > 23) )
							{
								// Invalid Data Notice
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
							else
							{
								SystemTime.wHour = nHour;
							}
						}

						bShowScreen = TRUE;
					}
					break;
				case 5:	// Minute
					{
						strTemp.Format(_T("%02d"), SystemTime.wMinute);

						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 59);

						if (Operator_Input_Number(_T("MINUTE"), strTemp, m_strScrOutData, 2, NORMAL_TYPE, strGuideText) == TRUE)
						{
							int nMinute = CUtil::StringToInt(m_strScrOutData);

							// V1.0.2.4 2018.07.03 - NULL Check 추가
							if( (m_strScrOutData.IsEmpty() == TRUE) || (nMinute < 0) || (nMinute > 59) )
							{
								// Invalid Data Notice
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
							else
							{
								SystemTime.wMinute = nMinute;
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
				// 설정한 날짜 Validation Check 후 진행 필요
				COleDateTime CheckDate;

				CheckDate = COleDateTime::GetCurrentTime();

				CheckDate.SetDate(SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay);

				if (CheckDate.GetStatus() == COleDateTime::valid)
				{
					// 입력한 값을 바탕으로 시간을 설정한다.
					SetLocalTime(&SystemTime);

					if (BeforeTi.wYear != SystemTime.wYear || BeforeTi.wMonth != SystemTime.wMonth || BeforeTi.wDay != SystemTime.wDay)
					{
						if (CEagleDataManager::GetInstance()->m_Config.m_Option.strScheduleDayClose == S_ENABLE)
						{
							// Day Total schedule Time refresh
							CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseYear = _T("2000");	// 2000년으로 setting해서 바로 send

							// Save File
							CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("dayclose_year"), CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseYear );

							LOG(Info, _T("Set Schedule Day Close Year 2000, because system time is changed by supervisor"));
						}

						CEagleSVCLib::GetInstance()->Set_Reboot_Time();	// 날짜가 변경되면 ReBoot Time도 변경하도록 보완 처리
					}

					// 날짜 및 시간 변경시 Journal 저장 로직 추가
					CString strOldValue, strNewValue;
					strOldValue.Format(_T("CHANGE TIME = %02d/%02d/%04d %02d:%02d"), BeforeTi.wMonth, BeforeTi.wDay, BeforeTi.wYear, BeforeTi.wHour, BeforeTi.wMinute);
					strNewValue.Format(_T("CHANGE TIME = %02d/%02d/%04d %02d:%02d"), SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear, SystemTime.wHour, SystemTime.wMinute);
					CDEV_Manager::GetInstance()->SaveEJL_SetChangeValue(strOldValue, strNewValue);

					Operator_Processing_Screen(_T("SUCCESS"));
				}
				else
				{
					Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
				}

				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				// Result 표시 후 화면 다시 표시
				m_nCurrentFocusIndex = 1;
				bSystemTimeUpdate = TRUE;
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
*	@brief		Upload 메뉴
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Upload()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strCmd, strValue;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			// 임시적으로 CONFIG는 비활성화 처리
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			Operator_DisplayScreen(OP_SCREEN_UPLOAD, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Upload - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_SYSTEM;
				break;
			}

			if ( (m_strScrOutData == _T("SOFTWARE")) || (m_strScrOutData == _T("OS")) )
			{
				if (Operator_Confirm_Screen(m_strScrOutData, _T(""), _T("PLEASE INSERT USB MEMORY STICK IN THE ATM")) == TRUE)
				{
					// USB가 꼽혔는지 확인
					if (CUtil::IsExistFile(PATH_USB) == TRUE)
					{
						// Execute Software Update
						// Check USB Memory
						// USB memory가 없을 경우 Invalid Notice 후 Upload 화면으로 전환
						if (CUtil::IsExistFile(EXE_SWUPDATE_EAGLECE) == TRUE)
						{
							if (m_strScrOutData == _T("OS"))
							{
								// 해상도에 맞는 OS를 설치하도록 로직 보완
								CString strOSFile;
								
								if (EAGLE_SCREEN_CX == CS130_WIDTH_RESOLUTION)
									strOSFile = USB_CS130_OS_NK_FILE_NAME;
								else
									strOSFile = USB_MF200_OS_NK_FILE_NAME;

								// OS File이 존재하는지 확인
								if (CUtil::IsExistFile(strOSFile) == TRUE)
								{
									CUtil::SetSWUpdateType(UPDATE_KIND_OS_STRING);		// OS Update 하도록 세팅

									// Setup 정보 변경사항 적용
									CDEV_Manager::GetInstance()->SaveEJL_SetChangedTerminalInfo();

									strTemp = _T("EXECUTE OS UPDATE");
									CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

									strTemp = _T("SYSTEM REBOOT");
									CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(reboot, strTemp);

									Operator_Processing_Screen(_T("AFTER SYSTEM REBOOT\nOS UPDATE WILL BE EXECUTED.\nPLEASE DO NOT REMOVE USB MEMORY STICK!"));
									CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_5000MS);

									CDEV_Manager::GetInstance()->System_Reboot();
								}
								else
								{
									Operator_Processing_Screen(_T("CAN NOT FIND OS FILE"));
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);								
								}
							}
							else
							{
								Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

								// Device Deinitialize 수행
								CEagleSVCLib::GetInstance()->Device_DeInitialize();

								CUtil::SetSWUpdateType(UPDATE_KIND_SOFTWARE_STRING);	// Software Update 하도록 세팅

								strTemp = _T("EXECUTE SOFTWARE UPDATE");
								CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

								strTemp = _T("SYSTEM REBOOT");
								CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(reboot, strTemp);

								// Execute Terminate_EagleCE
								{
									STARTUPINFO si;
									ZeroMemory (&si, sizeof(si));
									si.cb = sizeof (si);
									si.wShowWindow = SW_HIDE;
									PROCESS_INFORMATION pi;
									ZeroMemory (&pi, sizeof(pi));

									CString strPath;
									strPath.Format(_T("%s%s"), CUtil::GetAppPath(), EXE_TERMINATE_EAGLECE);

									CreateProcess(strPath, NULL,NULL,NULL,NULL,FALSE,0,NULL,&si, &pi);
								}

								CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_5MIN);
							}
						}
						else
						{
							Operator_Processing_Screen(_T("CAN NOT FIND FILE"));
							CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
						}
					}
					else
					{
						Operator_Processing_Screen(_T("CAN NOT FIND USB MEMORY STICK"));
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					}
				}
				
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("CONFIG"))
			{
				if (Operator_Confirm_Screen(m_strScrOutData, _T(""), _T("PLEASE INSERT USB MEMORY STICK IN THE ATM\nSYSTEM WILL BE REBOOTED AUTOMATICALLY")) == TRUE)
				{
					CString strSrcPath;
					CString strSrcFileName;
					CString strDestPath;
					BOOL	bResult = FALSE;

					// USB가 꼽혔는지 확인
					if (CUtil::IsExistFile(PATH_USB) == TRUE)
					{
						strDestPath.Format(_T("%s%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH);

						// 기종별 Configuration 정보를 다르게 저장이 가능하도록 수정
						strSrcPath.Format(_T("%s%s"), PATH_USB, EAGLE_ATM_CONFIG_PATH);

						if (EAGLE_SCREEN_CX == CS130_WIDTH_RESOLUTION)
							strSrcPath += _T("\\1280_800");
						else
							strSrcPath += _T("\\1024_768");

						if (CUtil::IsExistFile(strSrcPath) == TRUE)
						{
							Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);
							
							// Log Index를 기억했다가 Upload 후 다시 원래 Index로 Overwrite하도록 로직 수정
							CString strLogIndex;
							strLogIndex = CEagleDataManager::GetInstance()->m_Config.m_System.strBackUpLogIndex;

							// Sequence Number Index를 기억했다가 Upload 후 다시 원래 Index로 Overwrite하도록 로직 수정
							CString strSeqNo;
							strSeqNo = 	CEagleDataManager::GetInstance()->m_Config.m_System.strSequence_Number;

							// Capture Image File Index를 기억했다가 Upload 후 다시 원래 Index로 Overwrite하도록 로직 수정
							CString strCaptureImageIndex;
							strCaptureImageIndex = CEagleDataManager::GetInstance()->m_Config.m_System.strCaptureImageFile_Index;

							bResult = CEagleSVCLib::GetInstance()->Upload_ConfigFile(strSrcPath, strDestPath);

							// Upload 후 Log Index를 원래대로 복구
							CEagleDataManager::GetInstance()->m_Config.m_System.strBackUpLogIndex = strLogIndex;
							CEagleDataManager::GetInstance()->m_Config.SaveSystemData(_T("backup_log_index"), CEagleDataManager::GetInstance()->m_Config.m_System.strBackUpLogIndex);
							CEagleDataManager::GetInstance()->m_Config.m_System.strSequence_Number = strSeqNo;
							CEagleDataManager::GetInstance()->m_Config.SaveSystemData(_T("sequence_no"), CEagleDataManager::GetInstance()->m_Config.m_System.strSequence_Number);
							CEagleDataManager::GetInstance()->m_Config.m_System.strCaptureImageFile_Index = strCaptureImageIndex;
							CEagleDataManager::GetInstance()->m_Config.SaveSystemData(_T("backup_captureimage_index"), CEagleDataManager::GetInstance()->m_Config.m_System.strCaptureImageFile_Index);

							if (bResult == TRUE)
							{
								// Setup 정보 변경사항 적용
								CDEV_Manager::GetInstance()->SaveEJL_SetChangedTerminalInfo();

								strTemp = _T("EXECUTE UPLOAD CONFIG");
								CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

								// Success 후 System 재부팅
								Operator_Processing_Screen(_T("SUCCESS"));
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

								strTemp = _T("SYSTEM REBOOT");
								CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(reboot, strTemp);

								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

								// System Reboot 수행
								CDEV_Manager::GetInstance()->System_Reboot();
							}
							else
							{
								// 실패
								Operator_Processing_Screen(_T("FAILED"));
							}
						}
						else
						{
							// File Not Found
							Operator_Processing_Screen(_T("CAN NOT FIND FILE"));
						}
					}
					else
					{
						Operator_Processing_Screen(_T("CAN NOT FIND USB MEMORY STICK"));
					}

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("ADVERTISEMENT"))
			{
				if (Operator_Confirm_Screen(m_strScrOutData, _T(""), _T("PLEASE INSERT USB MEMORY STICK IN THE ATM")) == TRUE)
				{
					// USB가 꼽혔는지 확인
					if (CUtil::IsExistFile(PATH_USB) == TRUE)
					{
						CString strSrcPath;
						CString strDestPath;
						CString strDestFileName;
						BOOL	bResult = FALSE;

						// Execute Upload AD Files
						strDestPath.Format(_T("%s%s\\%d_%d"), PATH_USB, EAGLE_ATM_AD_PATH, EAGLE_SCREEN_CX, EAGLE_SCREEN_CY);
						strDestFileName = _T("*.jpg");
						strSrcPath.Format(_T("%s%s\\%d_%d"), CUtil::GetAppPath(), EAGLE_ATM_AD_PATH, EAGLE_SCREEN_CX, EAGLE_SCREEN_CY);

						if (CUtil::IsExistFile(strDestPath) == TRUE)
						{
							Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

							// Copy 전에 기존 ADV 제거 후 Copy하도록 로직 보완
							CUtil::DeleteFileInDirectory(strSrcPath, _T("*.*"));

							// Folder Create
							::CreateDirectory(strSrcPath, NULL);

							bResult = CUtil::CopyFileInDirectory(strDestPath, strDestFileName, strSrcPath);

							if (bResult == TRUE)
							{
								// 성공
								Operator_Processing_Screen(_T("SUCCESS"));
							}
							else
							{
								// 실패
								Operator_Processing_Screen(_T("FAILED"));
							}

							CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

							CSCR_Manager::GetInstance()->LoadAdvertisement();	// AD Upload 후 ADV를 다시 Load하도록 로직 추가
						}
						else
						{
							Operator_Processing_Screen(_T("CAN NOT FIND FILE"));
							CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
						}
					}
					else
					{
						Operator_Processing_Screen(_T("CAN NOT FIND USB MEMORY STICK"));
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					}
				}

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
*	@brief		BackUp
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_BackUp()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strCmd, strValue;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();
			strButtonInform.Empty();

			if (GetOperatorMode() == EAGLE_ATM_SVC_REPLENISH)
			{
				// Backup Config
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;

				// Backup Journal
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;

				// Backup Capture Image
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
			}
			else
			{
				strButtonInform += _T("on");
				strButtonInform += SCR_CMD_DELIMITER;

				strButtonInform += _T("on");
				strButtonInform += SCR_CMD_DELIMITER;

				//Camera가 장착된 기기에서만 해당 Menu가 표시되도록 함
				if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsExistCameraDriver() == TRUE)
					strButtonInform += _T("on");
				else
					strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
			}

			Operator_DisplayScreen(OP_SCREEN_BACKUP, bKeyEnable, strScreenInform, strButtonInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_BackUp - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_SYSTEM;
				break;
			}

			if (m_strScrOutData == _T("BACKUPCONFIG"))
			{
				if (Operator_Confirm_Screen(_T("BACKUP CONFIG"), _T(""), _T("PLEASE INSERT USB MEMORY STICK IN THE ATM")) == TRUE)
				{
					WIN32_FIND_DATA	FileData;
					HANDLE			hSearch = INVALID_HANDLE_VALUE;
					CString			strDestPath, strSerialNo;
					BOOL			bResult = FALSE;

					hSearch = FindFirstFile(PATH_USB, &FileData);
					if (hSearch != INVALID_HANDLE_VALUE)
					{
						FindClose(hSearch);

						Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

						// Execute Backup Config Files
						strDestPath.Format(_T("%s\\%s"), PATH_USB, EAGLE_ATM_CONFIG_PATH);

						// 기종별로 Back을 다르게 할 수 있도록 수정
						CreateDirectory(strDestPath, NULL);

						if (EAGLE_SCREEN_CX == CS130_WIDTH_RESOLUTION)
							strDestPath += _T("\\1280_800");
						else
							strDestPath += _T("\\1024_768");

						// Encryption 후 Backup하는 로직 보완 필요
						bResult = CEagleSVCLib::GetInstance()->Backup_ConfigFile(strDestPath);

						if (bResult == TRUE)
						{
							Operator_Processing_Screen(_T("SUCCESS"));
						}
						else
						{
							Operator_Processing_Screen(_T("FAILED"));
						}
					}
					else
					{
						Operator_Processing_Screen(_T("CAN NOT FIND USB MEMORY STICK"));
					}

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("BACKUPCAPTUREIMAGE"))
			{
				// File Size가 큰 관계로 하루치만 Backup하도록 로직 개선 (화면 추가 - 687
				m_strNextScrNum = OP_SCREEN_BACKUP_CAPTUREIMAGE;
				break;
			}
			else if (m_strScrOutData == _T("BACKUPJOURNAL"))
			{
				if (Operator_Confirm_Screen(_T("BACKUP JOURNAL"), _T(""), _T("PLEASE INSERT USB MEMORY STICK IN THE ATM")) == TRUE)
				{
					Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

					// File Backup시에는 CDM / PRT Status Polling을 Stop 후 진행하도록 로직 개선
					CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStop();
					CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();

					// Execute Backup Journal
					WIN32_FIND_DATA	FileData;
					HANDLE			hSearch = INVALID_HANDLE_VALUE;
					CString			strDestPath, strTerminalID;

					hSearch = FindFirstFile(PATH_USB, &FileData);
					if (hSearch != INVALID_HANDLE_VALUE)
					{
						FindClose(hSearch);

						// Execute Backup Journal DB File.
						strTerminalID = CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID;
						strTerminalID.TrimLeft();
						strTerminalID.TrimRight();

						// Terminal ID가 Null인 경우 Default로 Folder를 생성하도록 로직 수정
						if (strTerminalID.IsEmpty())
							strTerminalID = _T("DEFAULT");

						strDestPath.Format(_T("%s\\%s"), PATH_USB, strTerminalID);
						CreateDirectory(strDestPath, NULL);

						strDestPath += _T("\\BACKUPJNL");
						CreateDirectory(strDestPath, NULL);

						CDEV_Manager::GetInstance()->m_DEV_EJL.Close();
						CSVC_Manager::GetInstance()->m_EJL_Connect = FALSE;

						if (CEagleSVCLib::GetInstance()->Backup_JnlFile(strDestPath) == TRUE)
						{
							Operator_Processing_Screen(_T("SUCCESS"));
						}
						else
						{
							Operator_Processing_Screen(_T("FAILED"));
						}

						// EJL 연결
						if (TRUE == CDEV_Manager::GetInstance()->m_DEV_EJL.Open(GetSafeHwnd()))
						{
							LOG(Info, _T("EJL Open Success!"));
							m_EJL_Connect = TRUE;
						}

						CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStart();
						CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();
					}
					else
					{
						Operator_Processing_Screen(_T("CAN NOT FIND USB MEMORY STICK"));
					}

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("BACKUPLOG"))
			{
				if (Operator_Confirm_Screen(_T("BACKUP LOG"), _T(""), _T("PLEASE INSERT USB MEMORY STICK IN THE ATM")) == TRUE)
				{
					WIN32_FIND_DATA	FileData;
					HANDLE			hSearch = INVALID_HANDLE_VALUE;
					CString			strDestPath, strTerminaID;

					hSearch = FindFirstFile(PATH_USB, &FileData);
					if (hSearch != INVALID_HANDLE_VALUE)
					{
						FindClose(hSearch);

						// Backup 후 Log를 지울지 여부 Confirm 화면
						BOOL bDeleteLog = FALSE;
						if (Operator_Confirm_Screen(_T("DELETE LOG"), _T("DO YOU WANT TO DELETE WHOLE LOG FILES\nAFTER BACK UP TO USB MEMORY STICK?")) == TRUE)
						{
							bDeleteLog = TRUE;
						}

						Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

						CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStop();
						CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();

						// Execute Backup log Files
						strTerminaID = CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID;
						strTerminaID.TrimLeft();
						strTerminaID.TrimRight();

						// Terminal ID가 Null인 경우 Default로 Folder를 생성하도록 로직 수정
						if (strTerminaID.IsEmpty())
							strTerminaID = _T("DEFAULT");

						strDestPath.Format(_T("%s\\%s"), PATH_USB, strTerminaID);
						CreateDirectory(strDestPath, NULL);

						strDestPath += _T("\\BACKUPLOG");
						CreateDirectory(strDestPath, NULL);

						if (CEagleSVCLib::GetInstance()->Backup_LogFile(strDestPath) == TRUE)
						{
							if (bDeleteLog == TRUE)
							{
								strTemp.Format(_T("%s%s"), CUtil::GetAppPath(), PATH_LOG);
								CUtil::DeleteFileInDirectory(strTemp, _T("*.*"));

								CEagleDataManager::GetInstance()->m_Config.m_System.strBackUpLogIndex = _T("0");
								CEagleDataManager::GetInstance()->m_Config.SaveSystemData(_T("backup_log_index"), CEagleDataManager::GetInstance()->m_Config.m_System.strBackUpLogIndex);

								// Backup Log 삭제 후 저널에 해당 내역 저장
								strTemp = _T("CLEAR LOG FILES");
								CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

								Operator_Processing_Screen(_T("SUCCESS\nTHE LOG FILES HAS BEEN DELETED"));
							}
							else
							{
								Operator_Processing_Screen(_T("SUCCESS"));
							}
						}
						else
						{
							Operator_Processing_Screen(_T("FAILED"));
						}

						CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStart();
						CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();
					}
					else
					{
						Operator_Processing_Screen(_T("CAN NOT FIND USB MEMORY STICK"));
					}

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}
				
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
*	@brief		Capture Image Backup 날짜 설정
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_BackUpCaptureImage()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;

	CString strCmd;
	CString strValue;
	CString strScreenInform;
	CString strTemp;
	int		nMaxFocusIndex = 3;

	// Focus Setting
	m_nCurrentFocusIndex = 1;

	SYSTEMTIME SystemTime;
	GetLocalTime(&SystemTime);	// 현재 시간 정보

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// 1. Year
			strTemp.Format(_T("%04d"), SystemTime.wYear);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// 2. Month
			strTemp.Format(_T("%02d"), SystemTime.wMonth);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// 3. Day
			strTemp.Format(_T("%02d"), SystemTime.wDay);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_BACKUP_CAPTUREIMAGE, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_BackUpCaptureImage - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_BACKUP;
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
				m_strScrOutData.Empty();

				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// Year
					{
						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 2037);

						strTemp.Format(_T("%04d"), SystemTime.wYear);
						if (Operator_Input_Number(_T("YEAR"), strTemp, m_strScrOutData, 4, NORMAL_TYPE, strGuideText) == TRUE)
						{
							int nYear = CUtil::StringToInt(m_strScrOutData);

							if (nYear >= 1999 && nYear <= (2038-1))		// Main B/D 초기 시간 범위가 1999년 ~ 2038 1월까지임 (2037년까지만 입력 가능하게 처리함)
							{
								SystemTime.wYear = nYear;
							}
							else
							{
								// Invalid Notice
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}

						bShowScreen = TRUE;
					}
					break;

				case 2:	// Month
					{
						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 12);

						strTemp.Format(_T("%02d"), SystemTime.wMonth);
						if (Operator_Input_Number(_T("MONTH"), strTemp, m_strScrOutData, 2, NORMAL_TYPE, strGuideText) == TRUE)
						{
							int nMonth = CUtil::StringToInt(m_strScrOutData);

							if (nMonth >= 1 && nMonth <=12)
							{
								SystemTime.wMonth = nMonth;
							}
							else
							{
								// Invalid Notice
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}

						bShowScreen = TRUE;
					}
					break;
				case 3:	// Day
					{
						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 31);

						strTemp.Format(_T("%02d"), SystemTime.wDay);
						if (Operator_Input_Number(_T("DAY"), strTemp, m_strScrOutData, 2, NORMAL_TYPE, strGuideText) == TRUE)
						{
							int nDay = CUtil::StringToInt(m_strScrOutData);

							COleDateTime CheckDate;

							CheckDate = COleDateTime::GetCurrentTime();

							CheckDate.SetDate(SystemTime.wYear, SystemTime.wMonth, nDay);

							if (CheckDate.GetStatus() == COleDateTime::valid)
							{
								SystemTime.wDay = nDay;
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
			else if (m_strScrOutData == _T("BACKUPIMAGE"))
			{
				// 최종 선택된 validation check
				COleDateTime CheckDate;

				CheckDate = COleDateTime::GetCurrentTime();

				CheckDate.SetDate(SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay);

				if (CheckDate.GetStatus() == COleDateTime::valid)
				{
					if (Operator_Confirm_Screen(_T("BACKUP CAPTURE IMAGE"), _T(""), _T("PLEASE INSERT USB MEMORY STICK IN THE ATM")) == TRUE)
					{
						WIN32_FIND_DATA	FileData;
						HANDLE			hSearch = INVALID_HANDLE_VALUE;
						CString			strDestPath, strTerminaID;

						hSearch = FindFirstFile(PATH_USB, &FileData);

						if (hSearch != INVALID_HANDLE_VALUE)
						{
							FindClose(hSearch);

							Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

							CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStop();
							CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();

							CString			strDestPath, strTerminalID;

							strTerminalID = CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID;
							strTerminalID.TrimLeft();
							strTerminalID.TrimRight();

							// Terminal ID가 Null인 경우 Default로 Folder를 생성하도록 로직 수정
							if (strTerminalID.IsEmpty())
								strTerminalID = _T("DEFAULT");

							strDestPath.Format(_T("%s\\%s"), PATH_USB, strTerminalID);
							CreateDirectory(strDestPath, NULL);

							strDestPath += _T("\\BACKUPIMG");
							CreateDirectory(strDestPath, NULL);

							LOG(Info, _T("Processing Backup Capture Image - Start (%04d%02d%02d)"), SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay);

							CString strBackupDate;

							strBackupDate.Format(_T("%04d%02d%02d"), SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay);

							// Capture Image File도 압축해서 Backup하도록 함
							if (CEagleSVCLib::GetInstance()->Backup_CaptureImageFile(strDestPath, strBackupDate) == TRUE)
							{
								Operator_Processing_Screen(_T("SUCCESS"));
								LOG(Info, _T("Processing Backup Capture Image Success - End"));
							}
							else
							{
								Operator_Processing_Screen(_T("CAN NOT FIND FILES"));
								LOG(Info, _T("Processing Backup Capture Image Failed - End"));
							}

							CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStart();
							CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();
						}
						else
						{
							Operator_Processing_Screen(_T("CAN NOT FIND USB MEMORY STICK"));
							LOG(Info, _T("Processing Backup Capture Image - Can not find USB memory stick"));
						}
					}
				}
				else
				{
					Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);

					// 현재 시간 정보 갱신
					GetLocalTime(&SystemTime);	// 현재 시간 정보
				}
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



/** **********************************************************
*	@brief		Password 변경 화면
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Password()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInfo;
	CString strOutString;

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
			if (GetOperatorMode() == EAGLE_ATM_SVC_MASTER)
			{
				// 모든 Password 변경 가능
				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

			}
			else if (GetServiceMode() == EAGLE_ATM_SVC_ADMIN)
			{
				// Master를 제외한 Password 변경 가능
				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;
			}
			else if (GetServiceMode() == EAGLE_ATM_SVC_REPLENISH)
			{
				// 자기 자신 Password만 변경 가능
				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;
			}

			Operator_DisplayScreen(OP_SCREEN_PASSWORD, bKeyEnable, strScreenInform, strButtonInfo);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Password - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_SYSTEM;
				break;
			}

			// Next Action
			Operator_UserPasswordChange_Screen(m_strScrOutData);
			bShowScreen = TRUE;
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}
}


/** **********************************************************
*	@brief		Reboot 화면
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Reboot()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strTemp;
	int	nMaxFocusIndex = 2;
	int nHour = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime.Left(2));
	int nMinute = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime.Right(2));

	// Focus Setting
	m_nCurrentFocusIndex = 1;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// 1. Hour
			strTemp.Format(_T("%02d"), nHour);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// 2. Minute
			strTemp.Format(_T("%02d"), nMinute);
			strScreenInform += strTemp;

			Operator_DisplayScreen(OP_SCREEN_REBOOT, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Reboot - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_SYSTEM;
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
				m_strScrOutData.Empty();

				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// Hour
					{
						strTemp.Format(_T("%02d"), nHour);

						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 23);

						if (Operator_Input_Number(_T("HOUR"), strTemp, m_strScrOutData, 2, NORMAL_TYPE, strGuideText) == TRUE)
						{
							int nValue = CUtil::StringToInt(m_strScrOutData);

							if( (m_strScrOutData.IsEmpty() == TRUE) || (nValue < 0) || (nValue > 23) )
							{
								// Invalid Data Notice
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						//	else if ( nValue >= 0 && nValue <= 23)
							else
							{
								nHour = nValue;
							}
						}

						bShowScreen = TRUE;
					}
					break;
				case 2:	// Minute
					{
						strTemp.Format(_T("%02d"), nMinute);

						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 59);

						if (Operator_Input_Number(_T("MINUTE"), strTemp, m_strScrOutData, 2, NORMAL_TYPE, strGuideText) == TRUE)
						{
							int nValue = CUtil::StringToInt(m_strScrOutData);

							if( (m_strScrOutData.IsEmpty() == TRUE) || (nValue < 0) || (nValue > 59) )
							{
								// Invalid Data Notice
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						//	if (nValue >= 0 && nValue <= 59)
							else
							{
								nMinute = nValue;
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
				CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime = CUtil::IntToStringEx(nHour, 2) + CUtil::IntToStringEx(nMinute, 2);
				
				// Save File
				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("reboot_newtime"), CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime);

				CEagleSVCLib::GetInstance()->Set_Reboot_Time();
								
				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				// Result 표시 후 화면 다시 표시
				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("STARTREBOOT"))
			{
				if (Operator_Confirm_Screen(_T("REBOOT")) == TRUE)
				{
					Operator_Processing_Screen(CSCR_Manager::GetInstance()->GetAPTextIDString(OP_GUIDE_COMMON_WAIT));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					// Setup 정보 변경사항 적용
					CDEV_Manager::GetInstance()->SaveEJL_SetChangedTerminalInfo();

					CString strTemp;
					strTemp = _T("SYSTEM REBOOT");
					CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(reboot, strTemp);

					// System Reboot 수행
					CDEV_Manager::GetInstance()->System_Reboot();
				}
				
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
*	@brief		Password 변경 화면
*	@retval		없음
************************************************************/
void CSVC_Manager::Operator_UserPasswordChange_Screen(CString strCurrentUser)
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform, strTemp;
	CString strCurrentPW, strNewPassword, strVerifyPassword;
	CStringArray	strarryValue;

	m_nCurrentFocusIndex = 2;

	if (strCurrentUser == _T("ADMINISTRATOR"))
		strCurrentPW = CEagleDataManager::GetInstance()->m_Config.m_System.strAdministrator_password;
	else if (strCurrentUser == _T("MASTER"))
		strCurrentPW = CEagleDataManager::GetInstance()->m_Config.m_System.strMaster_password;
	else if (strCurrentUser == _T("REPLENISHMENT"))
		strCurrentPW = CEagleDataManager::GetInstance()->m_Config.m_System.strReplenishment_password;
	else if (strCurrentUser == _T("AMS PASSWORD"))
		strCurrentPW = CEagleDataManager::GetInstance()->m_Config.m_System.strRms_Password;
	else
		return;

	LOG(Info, _T("Operator_UserPasswordChange_Screen - start"));

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strNewPassword.Empty();
			strVerifyPassword.Empty();
			strarryValue.RemoveAll();

			strScreenInform = strCurrentUser;

			Operator_DisplayScreen(OP_SCREEN_USER_CHANGE_PASSWORD, bKeyEnable, strScreenInform, _T(""), _T(""), TRUE);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER || m_strScrOutData == S_EXIT)
			{
				if (strCurrentUser == _T("AMS PASSWORD"))
				{
					if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
						m_strNextScrNum = OP_SCREEN_AMS_CONFIGURE_TCPIP;
					else
						m_strNextScrNum = OP_SCREEN_AMS_CONFIGURE_MODEM;
				}
				else
				{
					m_strNextScrNum = OP_SCREEN_PASSWORD;
				}

				break;
			}

			// Next Action
			if (m_strScrOutName == _T("CURRENT"))
			{
				CUtil::ParsingStringToStringArray(m_strScrOutData, (CString)SCR_RES_DELIMITER, strarryValue);

				if (strarryValue.GetCount() == 2)
				{
					if (strCurrentPW == strarryValue[1])
					{
						m_nCurrentFocusIndex = 3;

						Operator_UpdateScreen(_T(""), _T(""), TRUE);
					}
					else
					{
						m_nCurrentFocusIndex = 2;
						Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
						bShowScreen = TRUE;
					}
				}
				else
				{
					m_nCurrentFocusIndex = 2;
					Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					bShowScreen = TRUE;
				}

			}
			else if (m_strScrOutName == _T("NEW"))
			{
				CUtil::ParsingStringToStringArray(m_strScrOutData, (CString)SCR_RES_DELIMITER, strarryValue);

				if (strarryValue.GetCount() == 2)
				{
					BOOL	bInvaild = FALSE;

					// Password가 숫자만 입력되었는지 확인 - 숫자만 입력 가능
					if (CUtil::IsNumeric(strarryValue[1]) == TRUE)
					{
						if (strCurrentUser == _T("ADMINISTRATOR"))
						{
							if (strarryValue[1] == CEagleDataManager::GetInstance()->m_Config.m_System.strMaster_password ||
								strarryValue[1] == CEagleDataManager::GetInstance()->m_Config.m_System.strReplenishment_password)
							{
								bInvaild = TRUE;
							}
						}
						else if (strCurrentUser == _T("MASTER"))
						{
							if (strarryValue[1] == CEagleDataManager::GetInstance()->m_Config.m_System.strAdministrator_password || 
								strarryValue[1] == CEagleDataManager::GetInstance()->m_Config.m_System.strReplenishment_password)
							{
								bInvaild = TRUE;
							}
						}
						else if (strCurrentUser == _T("REPLENISHMENT"))
						{
							if (strarryValue[1] == CEagleDataManager::GetInstance()->m_Config.m_System.strAdministrator_password || 
								strarryValue[1] == CEagleDataManager::GetInstance()->m_Config.m_System.strMaster_password)
							{
								bInvaild = TRUE;
							}
						}
						else if (strCurrentUser == _T("AMS PASSWORD"))
						{
							;
						}
					}
					else
					{
						bInvaild = TRUE;
					}

					if (bInvaild == TRUE)
					{
						m_nCurrentFocusIndex = 2;
						Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
						bShowScreen = TRUE;
					}
					else
					{
						m_nCurrentFocusIndex = 4;
						strNewPassword = strarryValue[1];
						Operator_UpdateScreen(_T(""), _T(""), TRUE);
						strarryValue.RemoveAll();
					}
				}
				else
				{
					m_nCurrentFocusIndex = 2;
					Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					bShowScreen = TRUE;
				}
			}
			else if (m_strScrOutName == _T("VERIFY"))
			{
				CUtil::ParsingStringToStringArray(m_strScrOutData, (CString)SCR_RES_DELIMITER, strarryValue);

				if (strarryValue.GetCount() == 2)
				{
					strVerifyPassword = strarryValue[1];

					if (strNewPassword == strVerifyPassword)
					{
						if (strCurrentUser == _T("ADMINISTRATOR"))
						{
							CEagleDataManager::GetInstance()->m_Config.m_System.strAdministrator_password = strNewPassword;
						}
						else if (strCurrentUser == _T("MASTER"))
						{
							CEagleDataManager::GetInstance()->m_Config.m_System.strMaster_password = strNewPassword;
						}
						else if (strCurrentUser == _T("REPLENISHMENT"))
						{
							CEagleDataManager::GetInstance()->m_Config.m_System.strReplenishment_password = strNewPassword;
						}
						else if (strCurrentUser == _T("AMS PASSWORD"))
						{
							CEagleDataManager::GetInstance()->m_Config.m_System.strRms_Password = strNewPassword;
						}

						CEagleDataManager::GetInstance()->m_Config.SaveSystem();

						Operator_Processing_Screen(_T("PASSWORD IS CHANGED"));
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
						
						if (strCurrentUser == _T("AMS PASSWORD"))
						{
							if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
								m_strNextScrNum = OP_SCREEN_AMS_CONFIGURE_TCPIP;
							else
								m_strNextScrNum = OP_SCREEN_AMS_CONFIGURE_MODEM;
						}
						else
						{
							m_strNextScrNum = OP_SCREEN_PASSWORD;
						}
						break;
					}
					else
					{
						m_nCurrentFocusIndex = 2;
						Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
						bShowScreen = TRUE;
					}
				}
				else
				{
					m_nCurrentFocusIndex = 2;
					Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					bShowScreen = TRUE;
				}
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

	LOG(Info, _T("Operator_UserPasswordChange_Screen - End"));
}