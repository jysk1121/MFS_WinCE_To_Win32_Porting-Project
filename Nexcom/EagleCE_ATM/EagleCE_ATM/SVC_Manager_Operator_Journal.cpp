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
*	@brief		Journal Main
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Journal_Main()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strButtonInfo, strTemp;

	m_nSearchJNLNo = 0;
	m_nLastXJNLCnt = 0;

	m_nJNLNo_Detail = 0;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			strButtonInfo.Empty();

			// Master 계정만 Clear Journal 가능하도록 함
			if (GetOperatorMode() == EAGLE_ATM_SVC_MASTER)
				strButtonInfo = _T("on");
			else
				strButtonInfo = _T("off");
			strButtonInfo += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_JOURNAL, bKeyEnable, _T(""), strButtonInfo);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Journal_Main - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER || m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("PRINTLASTX"))
			{
				// Print 전에 Printer가 장애인 경우에는 에러 처리
				// Print 전에 Printer가 장애인 경우에는 에러 처리
				if (CEagleSVCLib::GetInstance()->IsPTR_Available() == TRUE)
				{
					if (Operator_Input_Number(_T("PRINT LAST X"), _T(""), m_strScrOutData, 6) == TRUE)
					{
						m_strScrOutData.TrimLeft();
						m_strScrOutData.TrimRight();

						if (CUtil::IsNumeric(m_strScrOutData) == TRUE)
						{
							int nValue = CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastJournalNo();

							if (nValue <= 0)
							{
								LOG(Error,_T("GetLastJournalNo - Error : %s"), CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastError());

								Operator_Processing_Screen(_T("NO JOURNAL DATA"));
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);							
							}
							else
							{
								m_nLastXJNLCnt = CUtil::StringToInt(m_strScrOutData);

								if (m_nLastXJNLCnt > 0)
								{
									m_strNextScrNum = OP_SCREEN_PRINT_LASTX_JOURNAL;
									break;
								}
								else
								{
									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
							}
						}
						else
						{
							Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
							CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
						}
					}
				}
				else
				{
					Operator_Processing_Screen(_T("PRINT ERROR"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SEARCHJOURNAL"))
			{
				if (Operator_Input_Number(_T("SEARCH JOURNAL"), _T(""), m_strScrOutData, 6) == TRUE)
				{
					m_strScrOutData.TrimLeft();
					m_strScrOutData.TrimRight();

					if (CUtil::IsNumeric(m_strScrOutData) == TRUE)
					{
						int nValue = CUtil::StringToInt(m_strScrOutData);

						if (nValue < 1 || nValue > 1000000)
						{
							Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
							CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
						}
						else
						{
							// 찾으려는 Journal이 최종 기록된 Journal보다 큰 경우 Invalid 처리
							if (nValue <= CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastJournalNo())
							{
								// Search journal number setting 후 View Journal로 이동
								m_nSearchJNLNo = nValue;
								m_nJNLNo_Detail = 0;
								m_strNextScrNum = OP_SCREEN_VIEW_JOURNAL;
								break;
							}
							else
							{
								Operator_Processing_Screen(_T("CAN NOT FIND JOURNAL DATA"));
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}
					}
					else
					{
						Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					}
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("CLEARJOURNAL"))
			{
				if (Operator_Confirm_Screen(_T("CLEAR JOURNAL"), _T(""), _T("ALL JOURNAL WILL BE CLEARED")) == TRUE)
				{
					Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

					// EJL Close 후 File Copy 수행
					CDEV_Manager::GetInstance()->m_DEV_EJL.Close();
					m_EJL_Connect = FALSE;

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					CString strSource, strDestination;

					// File copy to default DB
					strTemp.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, EJL_DB_FILE_NAME);
					::DeleteFile(strTemp);

					strSource.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, EJL_DB_FILE_NAME);
					strDestination.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, EJL_DB_FILE_NAME);

					BOOL bResult = ::CopyFile(strSource, strDestination, FALSE);
					LOG(Info, _T("Default journal DB is copied, result (%d)"), bResult);

					// Camera Image도 모두 제거하도록 로직 추가
					CString strCaptureFilePath;

					strCaptureFilePath.Format(_T("%s%s"), CUtil::GetAppPath(), CAPTURE_IMAGE_FILE_PATH);
					CUtil::DeleteFileInDirectory(strCaptureFilePath, _T("*.*"));

					// Capture Image File Index 초기화
					CEagleDataManager::GetInstance()->m_Config.m_System.strCaptureImageFile_Index = _T("0");
					CEagleDataManager::GetInstance()->m_Config.SaveSystemData(_T("backup_captureimage_index"), _T("0"));

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					if (FALSE == CDEV_Manager::GetInstance()->m_DEV_EJL.Open(GetSafeHwnd()))
					{
						LOG(Error, _T("EJL Open Failed"));

						// 에러 코드 설정
						CEagleSVCLib::GetInstance()->Set_ErrorCode(CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastError());	
						m_EJL_Connect = FALSE;

						Operator_Processing_Screen(_T("FAILED"));
					}
					else
					{
						m_EJL_Connect = TRUE;

						LOG(Error, _T("EJL Open Success"));

						// EXECUTE CLEAR JOURNAL
						CString strTemp;
						strTemp = _T("EXECUTE CLEAR JOURNAL");
						CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

						Operator_Processing_Screen(_T("SUCCESS"));
					}

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("VIEWJOURNAL"))
			{
				m_strNextScrNum = OP_SCREEN_VIEW_JOURNAL;
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


/** *************************************************************
*	@brief		View Journal
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_View_Journal()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strTemp2;

	int		nMaxFocusIndex = 32;
	int		i = 0, nCurrentPage = 1, nRemainCnt = 0, nTotalPage = 100000, nTotalJNLNo = 0;
	int		nStartJNLNo = 0, nEndJNLNo = 0, nExistJNLCnt = 0, nTemp = 0;
	BOOL	bResult = FALSE;

	nTotalJNLNo = CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastJournalNo();

	if (nTotalJNLNo <= 0)
	{
		// ERROR 표시 후 이전 화면으로 표시
		Operator_Processing_Screen(_T("NO JOURNAL DATA"));
		CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

		m_strNextScrNum = OP_SCREEN_JOURNAL;
		return;
	}

	FLSEJCOMMINFO* pCommInfo = new FLSEJCOMMINFO[MAX_PAGE_COUNT];
	memset(pCommInfo, 0x00, sizeof(FLSEJCOMMINFO) * MAX_PAGE_COUNT);

	LOG(Info, _T("View Journal - GetLastJournalNo is - %d %d"), nTotalJNLNo, m_nJNLNo_Detail);

	// Load Last Count
	if (m_nSearchJNLNo != 0 || m_nJNLNo_Detail != 0)	// Search Journal 또는 Detail 수행 후 View Journal 수행시
	{
		// Search Journal을 통해 온 경우에 대한 처리 추가
		if (m_nSearchJNLNo != 0)
		{
			nCurrentPage = m_nSearchJNLNo / MAX_PAGE_COUNT;
			nRemainCnt = m_nSearchJNLNo % MAX_PAGE_COUNT;
		}
		else
		{
			nCurrentPage = m_nJNLNo_Detail / MAX_PAGE_COUNT;
			nRemainCnt = m_nJNLNo_Detail % MAX_PAGE_COUNT;
		}

		if (nRemainCnt != 0)
			nCurrentPage++;

		LOG(Info, _T("Current Page : %d"), nCurrentPage);

		nStartJNLNo = ((nCurrentPage-1) * MAX_PAGE_COUNT) + 1;
		nEndJNLNo = nStartJNLNo + 9;

		if (nRemainCnt <= 0)
			m_nCurrentFocusIndex = 32;
		else
			m_nCurrentFocusIndex = ((nRemainCnt-1) * 3) + 5;

		LOG(Info, _T("Current Focus : %d"), m_nCurrentFocusIndex);
	}
	else
	{
		// 마지막 Journal Number를 정상적으로 조회했을 경우
		nCurrentPage = nTotalJNLNo / MAX_PAGE_COUNT;
		nRemainCnt = nTotalJNLNo % MAX_PAGE_COUNT;

		if (nRemainCnt != 0)
			nCurrentPage++;

		LOG(Info, _T("Current Page : %d"), nCurrentPage);

		nStartJNLNo = ((nCurrentPage-1) * MAX_PAGE_COUNT) + 1;
		nEndJNLNo = nStartJNLNo + 9;
		m_nCurrentFocusIndex = 5;
		LOG(Info, _T("Current Focus : %d"), m_nCurrentFocusIndex);
	}

	// Search Journal
	strTemp.Format(_T("%d"), nStartJNLNo);
	strTemp2.Format(_T("%d"), nEndJNLNo);

	LOG(Info, _T("Search From (%s), To (%s)"), strTemp, strTemp2);

	if (FALSE == CDEV_Manager::GetInstance()->m_DEV_EJL.GetSearchJnlDataList(JNL_NO, strTemp, strTemp2, pCommInfo, MAX_PAGE_COUNT, nExistJNLCnt))
	{
		LOG(Error, _T("GetSearchJnlDataList - Error - Failed to get last journal data list - Error Code : ") + CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastError());

		Operator_Processing_Screen(_T("CAN NOT FIND JOURNAL DATA"));
		CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

		// 저널이 있으나 프로그램 오류로 인해 Journal Data를 찾지 못하는 현상이므로 return 처리
		m_strNextScrNum = OP_SCREEN_JOURNAL;
		delete[] pCommInfo;

		return;
	}

	LOG(Info, _T("Existed Journal Data Count : %d"), nExistJNLCnt);

	if (nExistJNLCnt <= 0)
	{
		Operator_Processing_Screen(_T("CAN NOT FIND JOURNAL DATA"));
		CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

		// 저널이 있으나 프로그램 오류로 인해 Journal Data를 찾지 못하는 현상이므로 return 처리
		m_strNextScrNum = OP_SCREEN_JOURNAL;
		delete[] pCommInfo;

		return;
	}

	if (nExistJNLCnt > MAX_PAGE_COUNT)
		nExistJNLCnt = MAX_PAGE_COUNT;

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();	
			strButtonInform.Empty();

			// 1. Total Count
			strTemp.Format(_T("%d"), nTotalJNLNo);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// 2. Page Information
			strTemp.Format(_T("%d / %d"), nCurrentPage, nTotalPage);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;


			for(i=0; i<MAX_PAGE_COUNT; i++)
			{
				int nIndex = (nExistJNLCnt-1)-i;

				if (nIndex >= 0)
				{
					//LOG(Info, _T("Journal Code = (%d)"), pCommInfo[nIndex].jnl_cd);

					// Journal No
					strTemp.Format(_T("%06d"), pCommInfo[nIndex].jnl_no);
					strScreenInform += strTemp;
					strScreenInform += SCR_CMD_DELIMITER;

					//LOG(Info, _T("Journal No = (%d)"), pCommInfo[nIndex].jnl_no);

					// Date / Time
					strTemp.Format(_T("%S %S"), pCommInfo[nIndex].log_date, pCommInfo[nIndex].log_time);
					strScreenInform += strTemp;
					strScreenInform += SCR_CMD_DELIMITER;

					// Transaction Code
					switch(pCommInfo[nIndex].jnl_cd)
					{
					case transaction:
						{
							strTemp = _T("- TRANSACTION -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case add_bill:
						{
							strTemp = _T("- ADD QTY. IN CBX -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case cst_close:
						{
							strTemp = _T("- CBX CLOSE -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case day_close:
						{
							strTemp = _T("- DAY CLOSE -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case denom_info:
						{
							strTemp = _T("- SET DENOM. -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case value_change:
						{
							strTemp = _T("- CHANGE VALUE -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case text_field:
						{
							strTemp = _T("- INFORMATION -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case withdrawal:
						{
							strTemp = _T("- WITHDRAWAL -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case balance:
						{
							strTemp = _T("- INQUIRY -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case transfer:
						{
							strTemp = _T("- TRANSFER -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case power_on:
						{
							strTemp = _T("- POWER ON -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case in_service:
						{
							strTemp = _T("- IN SERVICE -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case out_of_service:
						{
							strTemp = _T("- OUT OF SERVICE -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case in_supervisor:
						{
							strTemp = _T("- IN SUPERVISOR -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case reboot:
						{
							strTemp = _T("- REBOOT -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					case test_dispense:
						{
							strTemp = _T("- TEST DISPENSE -");
							strScreenInform += strTemp;
							strScreenInform += SCR_CMD_DELIMITER;
						}
						break;

					default:
						strScreenInform += _T("NOT DEFINED");
						strScreenInform += SCR_CMD_DELIMITER;
						break;
					}
				}
				else
				{
					strScreenInform += _T("");
					strScreenInform += SCR_CMD_DELIMITER;

					strScreenInform += _T("");
					strScreenInform += SCR_CMD_DELIMITER;

					strScreenInform += _T("");
					strScreenInform += SCR_CMD_DELIMITER;
				}
			}

			// Button
			if (nCurrentPage <= 1)
				strButtonInform += _T("off");
			else
				strButtonInform += _T("on");
			strButtonInform += SCR_CMD_DELIMITER;

			nTemp = nTotalJNLNo / MAX_PAGE_COUNT;
			if (nTotalJNLNo % MAX_PAGE_COUNT)
				nTemp++;

			if (nCurrentPage >= nTemp)
				strButtonInform += _T("off");
			else
				strButtonInform += _T("on");
			strButtonInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_VIEW_JOURNAL, bKeyEnable, strScreenInform, strButtonInform);	
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_View_Journal - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_JOURNAL;	
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				int nTempFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(2));

				if (nTempFocusIndex > (((nExistJNLCnt-1)*3) + 5))
				{
					Operator_Processing_Screen(OP_GUIDE_TOUCH_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bShowScreen = TRUE;
					continue;
				}

				m_nCurrentFocusIndex = nTempFocusIndex;

				Operator_UpdateScreen();

				m_strScrOutData = _T("DETAIL");
			}

			// Next Action
			if (m_strScrOutData == _T("PREVPAGE"))
			{
				nCurrentPage--;

				if (nCurrentPage < 1)
					nCurrentPage = 1;

				// search start and end
				nStartJNLNo = ((nCurrentPage-1) * MAX_PAGE_COUNT) + 1;
				nEndJNLNo = nStartJNLNo + 9;

				// Search Journal
				strTemp.Format(_T("%d"), nStartJNLNo);
				strTemp2.Format(_T("%d"), nEndJNLNo);

				LOG(Info, _T("Prev Page Search From (%s), To (%s)"), strTemp, strTemp2);

				// Reload Journal Data
				memset(pCommInfo, 0x00, sizeof(FLSEJCOMMINFO) * MAX_PAGE_COUNT);
				nExistJNLCnt = 0;

				if (FALSE == CDEV_Manager::GetInstance()->m_DEV_EJL.GetSearchJnlDataList(JNL_NO, strTemp, strTemp2, pCommInfo, MAX_PAGE_COUNT, nExistJNLCnt))
				{
					LOG(Error, _T("PrevPage GetSearchJnlDataList - Error - Failed to get last journal data list - Error Code : ") + CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastError());

					Operator_Processing_Screen(_T("CAN NOT FIND JOURNAL DATA"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					// 저널이 있으나 프로그램 오류로 인해 Journal Data를 찾지 못하는 현상이므로 return 처리
					m_strNextScrNum = OP_SCREEN_JOURNAL;
					delete[] pCommInfo;

					return;
				}

				LOG(Info, _T("Existed Journal Data Count : %d"), nExistJNLCnt);

				if (nExistJNLCnt <= 0)
				{
					Operator_Processing_Screen(_T("CAN NOT FIND JOURNAL DATA"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}
				else
				{
					if (nExistJNLCnt > MAX_PAGE_COUNT)
						nExistJNLCnt = MAX_PAGE_COUNT;

					m_nCurrentFocusIndex = 5; // 초기화
				}

				bShowScreen = TRUE;

			}
			else if (m_strScrOutData == _T("NEXTPAGE"))
			{
				nCurrentPage++;

				if (nCurrentPage >= nTotalPage)
					nCurrentPage = nTotalPage;

				// search start and end
				nStartJNLNo = ((nCurrentPage-1) * MAX_PAGE_COUNT) + 1;
				nEndJNLNo = nStartJNLNo + 9;

				if (nStartJNLNo > nTotalJNLNo)	// 저널이 없을 경우 현재 화면 유지
				{
					nCurrentPage--;
				}
				else
				{
					// Search Journal
					strTemp.Format(_T("%d"), nStartJNLNo);
					strTemp2.Format(_T("%d"), nEndJNLNo);

					LOG(Info, _T("Next Page Search From (%s), To (%s)"), strTemp, strTemp2);

					// Reload Journal Data
					memset(pCommInfo, 0x00, sizeof(FLSEJCOMMINFO) * MAX_PAGE_COUNT);
					nExistJNLCnt = 0;
					if (FALSE == CDEV_Manager::GetInstance()->m_DEV_EJL.GetSearchJnlDataList(JNL_NO, strTemp, strTemp2, pCommInfo, MAX_PAGE_COUNT, nExistJNLCnt))
					{
						LOG(Error, _T("PrevPage GetSearchJnlDataList - Error - Failed to get last journal data list - Error Code : ") + CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastError());

						Operator_Processing_Screen(_T("CAN NOT FIND JOURNAL DATA"));
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

						// 저널이 있으나 프로그램 오류로 인해 Journal Data를 찾지 못하는 현상이므로 return 처리
						m_strNextScrNum = OP_SCREEN_JOURNAL;
						delete[] pCommInfo;

						return;
					}

					LOG(Info, _T("Existed Journal Data Count : %d"), nExistJNLCnt);

					if (nExistJNLCnt <= 0)
					{
						Operator_Processing_Screen(_T("CAN NOT FIND JOURNAL DATA"));
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					}
					else
					{
						if (nExistJNLCnt > MAX_PAGE_COUNT)
							nExistJNLCnt = MAX_PAGE_COUNT;

						m_nCurrentFocusIndex = 5; // 초기화
					}
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;
				m_nCurrentFocusIndex--;
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 5)
				{
					m_nCurrentFocusIndex = ((nExistJNLCnt-1)*3) + 5;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_nCurrentFocusIndex++;
				m_nCurrentFocusIndex++;
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > (((nExistJNLCnt-1)*3) + 5))
					m_nCurrentFocusIndex = 5;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("DETAIL"))
			{
				m_nJNLNo_Detail = nStartJNLNo + ((m_nCurrentFocusIndex/3) - 1);
				m_nSearchJNLNo = 0;
				m_strNextScrNum = OP_SCREEN_VIEW_JOURNAL_DETAIL;
				break;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}

	delete[] pCommInfo;
}


#define		MAX_PAGE_VIEW_LINE		20
/** *************************************************************
*	@brief		View Journal
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_View_Journal_Detail()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strTemp2, strTemp3, strPrintData, strTransType, strCAMImgPath;
	int		i = 0, nPageLineCount = 0, nTotalPage = 0, nCurrentPage = 1;
	CStringArray	strArrayTemp;

	FLSEJCOMMINFO CommInfo;
	memset(&CommInfo, 0x00, sizeof(CommInfo));

	FLSEJINFO	JnlInfo;
	memset(&JnlInfo, 0x00, sizeof(JnlInfo));

	int nCSTCnt = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

	LOG(Info, _T("Detail Journal Count (%d)"), m_nJNLNo_Detail);

	if (CDEV_Manager::GetInstance()->m_DEV_EJL.GetDetailJnlData(m_nJNLNo_Detail, &CommInfo, &JnlInfo) == FALSE)
	{
		LOG(Info, _T("Detail - Error - GetDetailJnlData is - %d (%s)"), m_nJNLNo_Detail, CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastError());

		// ERROR 표시 후 이전 화면으로 표시
		Operator_Processing_Screen(_T("CAN NOT FOUND JOURNAL DATA"));
		CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

		m_strNextScrNum = OP_SCREEN_VIEW_JOURNAL;
		return;
	}

	strCAMImgPath.Format(_T("%s%s"), CUtil::GetAppPath(), CAPTURE_IMAGE_FILE_PATH);

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();	
			strButtonInform.Empty();
			strPrintData.Empty();
			strArrayTemp.RemoveAll();

			strPrintData += Get_JournalData(CommInfo, JnlInfo);
			CUtil::ParsingStringToStringArray(strPrintData, SCR_LINEFEED_DELEMITER, strArrayTemp);

			nPageLineCount = strArrayTemp.GetCount();

			nTotalPage = nPageLineCount / MAX_PAGE_VIEW_LINE;

			if (nPageLineCount % MAX_PAGE_VIEW_LINE)
				nTotalPage++;

			// 1. Page Information
			strTemp.Format(_T("%d / %d"), nCurrentPage, nTotalPage);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;


			// Screen Data
			int i = (nCurrentPage-1)*MAX_PAGE_VIEW_LINE;
			int nDisplayLine = 0;

			if (nPageLineCount > (i + MAX_PAGE_VIEW_LINE))
				nDisplayLine = (i + MAX_PAGE_VIEW_LINE);
			else
				nDisplayLine = nPageLineCount;

			for(; i<nDisplayLine; i++)
			{
				strScreenInform += strArrayTemp[i];
				strScreenInform += SCR_LINEFEED_DELEMITER;
			}

			// 3. Camera Image File Read 후 해당 Data를 Screen에 전달 (Null일 경우 해당 Control이 비활성화 처리됨)
			strScreenInform += SCR_CMD_DELIMITER;

			if (CommInfo.jnl_cd == withdrawal || CommInfo.jnl_cd == balance || CommInfo.jnl_cd == transfer || CommInfo.jnl_cd == transaction)
			{
				strTemp.Empty();
				strTemp = JnlInfo.tran_info.cam_img_filename;

				LOG(Info, _T("CAM IMAGE FILE : %s"), strTemp);

				if (strTemp.IsEmpty() == FALSE)
				{
					strTemp2.Format(_T("%s\\%s"), strCAMImgPath, strTemp);
					// Image Data가 있는 경우 송부
					strScreenInform += strTemp2;
				}
			}
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_VIEW_JOURNAL_DETAIL, bKeyEnable, strScreenInform, strButtonInform);	
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_View_Journal_Detail - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_VIEW_JOURNAL;	
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("PRINT"))
			{
				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

				if (CEagleSVCLib::GetInstance()->PrintEJL_DetailData(strPrintData) == TRUE)
					Operator_Processing_Screen(_T("SUCCESS"));
				else
					Operator_Processing_Screen(_T("FAILED"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("PREV"))
			{
				nCurrentPage--;
				if (nCurrentPage <= 1)
					nCurrentPage = 1;

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("NEXT"))
			{
				nCurrentPage++;
				if (nCurrentPage >= nTotalPage)
					nCurrentPage = nTotalPage;

				bShowScreen = TRUE;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** **********************************************************
*	@brief		계원용 커맨드 - Journal - Print Last X
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Journal_PrintLastX(void)
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strTemp, strData;
	CStringA szTemp;
	CStringArray	strarrayJNLData;
	int		nCurrentJNLNo = 0, nExistJNLCount = 0;
	int     i = 0, nOrgX = 40;

	FLSEJCOMMINFO CommInfo;
	FLSEJINFO	JnlInfo;
	
	memset(&CommInfo, 0x00, sizeof(CommInfo));
	memset(&JnlInfo, 0x00, sizeof(JnlInfo));

	 nExistJNLCount = CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastJournalNo();

	 // 입력한 journal count보다 실제 저널이 적게 있는 경우 있는 저널만 Print.
	 if (m_nLastXJNLCnt > nExistJNLCount)
		 m_nLastXJNLCnt = nExistJNLCount;

	 nCurrentJNLNo = nExistJNLCount;

	 // 프린터 램프 ON
	 CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_FLICKING);

	 // PTR Quiry Status Stop
	 CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();

	if (CEagleSVCLib::GetInstance()->Set_PTR_Config() == FALSE)
	{
		LOG(Error, _T("Set PTR Config is failed"));
	}

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			Operator_DisplayScreen(OP_SCREEN_PRINT_LASTX_JOURNAL, bKeyEnable);

			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// Print Last X Journal Data
		if (CDEV_Manager::GetInstance()->m_DEV_EJL.GetDetailJnlData(nCurrentJNLNo, &CommInfo, &JnlInfo) == FALSE)
		{
			// Journal Get Fail시 Fail 표시 후 이전 화면으로 표시
			LOG(Error, _T("GetDetailJnlData is failed - Journal No : %d, Error (%s)"), m_nLastXJNLCnt, CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastError());
			m_strNextScrNum = OP_SCREEN_JOURNAL;
			break;
		}

		// 
		strData.Empty();
		strarrayJNLData.RemoveAll();

		strData = Get_JournalData(CommInfo, JnlInfo, SCR_LINEFEED_DELEMITER, TRUE);

		CUtil::ParsingStringToStringArray(strData, SCR_LINEFEED_DELEMITER, strarrayJNLData);

		for(i=0; i<strarrayJNLData.GetCount(); i++)
		{
			szTemp.Format("%S", strarrayJNLData[i]);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Error, _T("PRT - %S"), szTemp);
		}

		m_nLastXJNLCnt--;
		nCurrentJNLNo--;

		if (m_nLastXJNLCnt <= 0)
		{
			// Print가 다 끝났으므로 Cutting 후 return 처리
			// Cutting 후에 이전 화면으로 전환
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_CutPaper(EAGLE_PRT_TYPE_USB, 1, 0);

			m_strNextScrNum = OP_SCREEN_JOURNAL;
			break;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Journal_PrintLastX - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_EXIT)
			{
				Operator_Processing_Screen(_T("CANCELING. PLEASE WAIT"));

				// Cutting 후에 이전 화면으로 전환
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_CutPaper(EAGLE_PRT_TYPE_USB, 1, 0);

				m_strNextScrNum = OP_SCREEN_JOURNAL;
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

	// PTR Quiry Status Stop
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();

	// 프린터 램프 ON
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_OFF);
}


CString CSVC_Manager::Get_JournalData(FLSEJCOMMINFO CommInfo, FLSEJINFO JnlInfo, CString strDelimiter, BOOL bMinimumData)
{
	CString			strTemp, strTemp2, strTemp3, strTransType, strReturnValue;
	int				nCSTCnt = 0, nValueLen = 0, i = 0, j = 0;

	nCSTCnt = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

	// 2. Common Info
	// [V1.0.2.10] 2021.02.22 LEH - ViewDetail 화면에서 초 정보가 짤림으로 인해 수정
//	strTemp.Format(_T("JNL NO:%06d DATE:%S TIME:%S"), CommInfo.jnl_no, CommInfo.log_date, CommInfo.log_time);
	strTemp.Format(_T("JNL NO:%06d DATE:%S %S"), CommInfo.jnl_no, CommInfo.log_date, CommInfo.log_time);
	strReturnValue += strTemp;
	strReturnValue += strDelimiter;

	switch(CommInfo.jnl_cd)
	{
	case transaction:
	case withdrawal:
	case balance:
	case transfer:
		{
			strTemp = _T("*** TRANSACTION ***");
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// Terminal ID
			strTemp.Format(_T("TERMINAL ID = %S"), JnlInfo.tran_info.term_id);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// Sequence No
			strTemp.Format(_T("SEQUENCE NO. = %S"), JnlInfo.tran_info.tran_seq_no);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// CARD DATA
			strTemp.Format(_T("CARD NO. = %S"), JnlInfo.tran_info.card_tr2);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// AUTHORIZATION NO.
			strTemp.Format(_T("AUTHORIZATION NO. = %S"), JnlInfo.tran_info.auth_no);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// DATE / TIME
			strTemp.Format(_T("DATE / TIME = %S %S"), JnlInfo.tran_info.tran_date, JnlInfo.tran_info.tran_time);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// BUSINESS DATE
			strTemp.Format(_T("BUSINESS DATE = %S"), JnlInfo.tran_info.business_date);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// TRANSACTION
			strTransType.Format(_T("%S"), JnlInfo.tran_info.tran_cd);
			strTemp.Format(_T("TRANSACTION = %s"), strTransType);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// ACCOUNT
			strTemp.Format(_T("ACCOUNT = %S"), JnlInfo.tran_info.account_type);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			int nRequestedAmt = 0;
			int nDispensedAmt = 0;
			int nSurchargeAmt = 0;

			if (strTransType == S_WITHDRAWAL)
			{
				// REQUESTED
				strTemp2.Format(_T("%S%S"), JnlInfo.tran_info.req_amt, "00");	// Cent 추가
				nRequestedAmt = CUtil::StringToInt(strTemp2);
				strTemp.Format(_T("REQUESTED = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				// DISPENSED
				strTemp2.Format(_T("%S%S"), JnlInfo.tran_info.disp_amt, "00");	// Cent 추가
				nDispensedAmt = CUtil::StringToInt(strTemp2);
				strTemp.Format(_T("DISPENSED = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.tran_info.surc_amt);
				nSurchargeAmt = CUtil::StringToInt(strTemp2);

				if (nDispensedAmt > 0 && nSurchargeAmt > 0)	// 미국 사양
				{
					// ATM FEE
					strTemp.Format(_T("ATM FEE = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
					strReturnValue += strTemp;
					strReturnValue += strDelimiter;
				}
			}
			else if (strTransType == S_BALANCEINQUIRY)
			{
				// 조회시 수수료 처리 추가시 처리 추가 필요
			}
			else if (strTransType == S_TRANSFER)
			{
				strTemp2.Format(_T("%S"), JnlInfo.tran_info.req_amt);	// Cent 추가
				strTemp.Format(_T("TRANSFER AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}

			// Balance 정보는 저널에 무조건 표시
			//if (nDispensedAmt >= nRequestedAmt)
			{
				// BALANCE
				strTemp2.Format(_T("%S"), JnlInfo.tran_info.bal_amt);
				if (CUtil::StringToInt(strTemp2) > 0)
				{
					strTemp3.Format(_T("%s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2));
					if (strTemp3.GetAt(0) == '-' )
					{
						// '-' 기호 처리
						strTemp.Format(_T("BALANCE = - %s%s"), CURRENCY_SYMBOL, strTemp3.Mid(1));
					}
					else
					{
						strTemp.Format(_T("BALANCE = %s%s"), CURRENCY_SYMBOL, strTemp3);
					}

					strReturnValue += strTemp;
					strReturnValue += strDelimiter;
				}

				// AVALIABLE BALANCE
				strTemp2.Format(_T("%S"), JnlInfo.tran_info.avail_bal_amt);
				if (CUtil::StringToInt(strTemp2) > 0)
				{
					strTemp3.Format(_T("%s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2));
					if (strTemp3.GetAt(0) == '-' )
					{
						// '-' 기호 처리
						strTemp.Format(_T("AVAILABLE BALANCE = - %s%s"), CURRENCY_SYMBOL, strTemp3.Mid(1));
					}
					else
					{
						strTemp.Format(_T("AVAILABLE BALANCE = %s%s"), CURRENCY_SYMBOL, strTemp3);
					}

					strReturnValue += strTemp;
					strReturnValue += strDelimiter;
				}
			}

			// 출금거래에 한해 DISPENSED / REJECTED 매수 저장
			if(strTransType == S_WITHDRAWAL)
			{
				// DISPENSED COUNT / REJECTED COUNT 표시
				strTemp2.Format(_T("%S"), JnlInfo.tran_info.disp_cnt);
				if (strTemp2.GetLength() > 0)
					strTemp.Format(_T("DISPENSED COUNT = %s"), strTemp2);
				else
					strTemp.Format(_T("DISPENSED COUNT = %s"), _T("00, 00, 00, 00"));

				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.tran_info.reject_cnt);
				if (strTemp2.GetLength() > 0)
					strTemp.Format(_T("REJECTED COUNT  = %s"), strTemp2);
				else
					strTemp.Format(_T("REJECTED COUNT  = %s"), _T("00, 00, 00, 00"));

				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}

			// Error Code가 존재하는 경우 Error Code부터 먼저 Display
			// ERROR
			strTemp2.Format(_T("%6.6S"), JnlInfo.tran_info.err_code_1);
			strTemp2.TrimLeft();
			strTemp2.TrimRight();

			if (strTemp2.IsEmpty() == FALSE)
			{
				// ERROR
				strTemp.Format(_T("ERROR = %s"), strTemp2);
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}

			strTemp2.Format(_T("%6.6S"), JnlInfo.tran_info.err_code_2);
			strTemp2.TrimLeft();
			strTemp2.TrimRight();
			if (strTemp2.IsEmpty() == FALSE)
			{
				// ERROR2
				strTemp.Format(_T("ERROR2 = %s"), strTemp2);
				strReturnValue += strTemp;
			}

			// Power Off Reversal시에만 사용 -> err_code_1로 변경함
			//strTemp2.Format(_T("%6.6S"), JnlInfo.tran_info.err_code_3);
			//strTemp2.TrimLeft();
			//strTemp2.TrimRight();
			//if (strTemp2.IsEmpty() == FALSE)
			//{
			//	// ERROR3
			//	strTemp.Format(_T("ERROR3 = %s"), strTemp2);
			//	strReturnValue += strTemp;
			//}

			// EMV Data Check
			strTemp = JnlInfo.tran_info.emv_data;
			strTemp.TrimRight();

			if (strTemp.GetLength() > 0)
			{
				if (strTemp.Left(1) == JNL_ERRMSG_FIELD_EMV_DATA)
				{
					if (bMinimumData == TRUE)
					{
						// Print Last X시에는 EMV 상세 데이터 미 인자
						strReturnValue += _T(" ** EMV TRANSACTION **");
						strReturnValue += strDelimiter;
					}
					else
					{
						CStringArray	strArrayEMVData;
						CString			strEMVData;

						// EMV Data가 존재하면 구분을 위해 Line을 하나 띄움
						strReturnValue += strDelimiter;

						// EMV DATA Title 추가
						strReturnValue += _T(" ** EMV DATA **");
						strReturnValue += strDelimiter;

						strTemp = strTemp.Mid(1);		// EMV FIELD CMD 제거

						// Parsing FS
						CUtil::ParsingStringToStringArray(strTemp, (CString)FS, strArrayEMVData);

						for(i=0; i<strArrayEMVData.GetCount(); i++)
						{
							strEMVData = strArrayEMVData.GetAt(i);

							// Format - Tag / Length / Value
							if (strEMVData.GetLength() > 2)		// 최소 Tag가 존재하는지 Check
							{
								strTemp = strEMVData.Left(2);

								if (strTemp.Right(1) == _T("F"))	// Tag의 2번째가 "F"인 경우는 4BYTE Tag임
								{
									// 4BYTE TAG이므로 Tag Length 정보 제거 후 Value 값 얻어오기 (Tag : 4 / Length : 2)
									strTemp2.Format(_T("%4s = %s"), strEMVData.Left(4), strEMVData.Right(strEMVData.GetLength()-6));
								}
								else
								{
									// 2BYTE TAG
									strTemp2.Format(_T("%4s = %s"), strEMVData.Left(2), strEMVData.Right(strEMVData.GetLength()-4));
								}

								strReturnValue += strTemp2;
								strReturnValue += strDelimiter;
							}
						}
					}
				}
				else if (strTemp.Left(1) == JNL_ERRMSG_FIELD_FALLBACK)
				{
					strReturnValue += _T("FALLBACK TRANSACTION");
					strReturnValue += strDelimiter;
				}
			}
		}
		break;

	case add_bill:
		{
			// Title
			strTemp = _T("*** ADD QUANTITY IN CBX ***");
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// Terminal ID
			strTemp.Format(_T("TERMINAL ID = %S"), JnlInfo.add_bill_info.term_id);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// DATE / TIME
			strTemp.Format(_T("DATE / TIME = %S %S"), JnlInfo.add_bill_info.start_date, JnlInfo.add_bill_info.start_time);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// Added CBXCount
			strTemp.Format(_T("%S"),  JnlInfo.add_bill_info.cst_count);

			int nJnlCBXCount = CUtil::StringToInt(strTemp);

			if (nJnlCBXCount > 0)
				nCSTCnt = nJnlCBXCount;

			if (nCSTCnt >= 1)
			{
				strTemp.Format(_T("  ** CBX #1 **"));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.add_bill_info.add_bill_cst1);
				strTemp.Format(_T("    ADDED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.add_bill_info.remain_cst1);
				strTemp.Format(_T("    CURRENT NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}

			if (nCSTCnt >= 2)
			{
				strTemp.Format(_T("  ** CBX #2 **"));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.add_bill_info.add_bill_cst2);
				strTemp.Format(_T("    ADDED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.add_bill_info.remain_cst2);
				strTemp.Format(_T("    CURRENT NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}

			if (nCSTCnt >= 3)
			{
				strTemp.Format(_T("  ** CBX #3 **"));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.add_bill_info.add_bill_cst3);
				strTemp.Format(_T("    ADDED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.add_bill_info.remain_cst3);
				strTemp.Format(_T("    CURRENT NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}

			if (nCSTCnt >= 4)
			{
				strTemp.Format(_T("  ** CBX #4 **"));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.add_bill_info.add_bill_cst4);
				strTemp.Format(_T("    ADDED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.add_bill_info.remain_cst4);
				strTemp.Format(_T("    CURRENT NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}
		}
		break;

	case cst_close:
		{
			// Title
			strTemp.Format(_T("%S"), JnlInfo.cst_close_info.cst_close_type);
			if ( strTemp == _T("0"))
				strTemp = _T("*** TRIAL CBX CLOSE ***");
			else
				strTemp = _T("*** CBX CLOSE ***");
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// Terminal ID
			strTemp.Format(_T("TERMINAL ID = %S"), JnlInfo.cst_close_info.term_id);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// DATE / TIME
			strTemp.Format(_T("DATE / TIME = %S %S"), JnlInfo.cst_close_info.start_date, JnlInfo.cst_close_info.start_time);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// Added CBXCount
			strTemp.Format(_T("%S"),  JnlInfo.cst_close_info.cst_count);

			int nJnlCBXCount = CUtil::StringToInt(strTemp);

			if (nJnlCBXCount > 0)
				nCSTCnt = nJnlCBXCount;

			if (nCSTCnt >= 1)
			{
				strTemp.Format(_T("  ** CBX #1 **"));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.load_bill_cst1);
				strTemp.Format(_T("    LOADED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.disp_bill_cst1);
				strTemp.Format(_T("    DISPENSED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.rej_bill_cst1);
				strTemp.Format(_T("    REJECTED COUNT = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.rem_bill_cst1);
				strTemp.Format(_T("    CURRENT NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.load_amt_cst1);
				strTemp.Format(_T("    LOADED AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.disp_amt_cst1);
				strTemp.Format(_T("    DISPENSED AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.rem_amt_cst1);
				strTemp.Format(_T("    CURRENT AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.denom_cst1);
				strTemp.Format(_T("    VALUE OF EACH NOTE = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}

			if (nCSTCnt >= 2)
			{
				strTemp.Format(_T("  ** CBX #2 **"));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.load_bill_cst2);
				strTemp.Format(_T("    LOADED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.disp_bill_cst2);
				strTemp.Format(_T("    DISPENSED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.rej_bill_cst2);
				strTemp.Format(_T("    REJECTED COUNT = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.rem_bill_cst2);
				strTemp.Format(_T("    CURRENT NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.load_amt_cst2);
				strTemp.Format(_T("    LOADED AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.disp_amt_cst2);
				strTemp.Format(_T("    DISPENSED AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.rem_amt_cst2);
				strTemp.Format(_T("    CURRENT AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.denom_cst2);
				strTemp.Format(_T("    VALUE OF EACH NOTE = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}

			if (nCSTCnt >= 3)
			{
				strTemp.Format(_T("  ** CBX #3 **"));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.load_bill_cst3);
				strTemp.Format(_T("    LOADED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.disp_bill_cst3);
				strTemp.Format(_T("    DISPENSED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.rej_bill_cst3);
				strTemp.Format(_T("    REJECTED COUNT = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.rem_bill_cst3);
				strTemp.Format(_T("    CURRENT NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.load_amt_cst3);
				strTemp.Format(_T("    LOADED AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.disp_amt_cst3);
				strTemp.Format(_T("    DISPENSED AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.rem_amt_cst3);
				strTemp.Format(_T("    CURRENT AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.denom_cst3);
				strTemp.Format(_T("    VALUE OF EACH NOTE = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}

			if (nCSTCnt >= 4)
			{
				strTemp.Format(_T("  ** CBX #4 **"));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.load_bill_cst4);
				strTemp.Format(_T("    LOADED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.disp_bill_cst4);
				strTemp.Format(_T("    DISPENSED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.rej_bill_cst4);
				strTemp.Format(_T("    REJECTED COUNT = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.rem_bill_cst4);
				strTemp.Format(_T("    CURRENT NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.load_amt_cst4);
				strTemp.Format(_T("    LOADED AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.disp_amt_cst4);
				strTemp.Format(_T("    DISPENSED AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.rem_amt_cst4);
				strTemp.Format(_T("    CURRENT AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.denom_cst4);
				strTemp.Format(_T("    VALUE OF EACH NOTE = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}
		}
		break;

	case day_close:
		{
			// Title
			strTemp.Format(_T("%S"), JnlInfo.day_close_info.day_close_type);
			if ( strTemp == _T("0"))
				strTemp = _T("*** TRIAL DAY CLOSE ***");
			else
				strTemp = _T("*** DAY CLOSE ***");
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// Terminal ID
			strTemp.Format(_T("TERMINAL ID = %S"), JnlInfo.day_close_info.term_id);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// DATE / TIME
			strTemp.Format(_T("DATE / TIME = %S %S"), JnlInfo.day_close_info.start_date, JnlInfo.day_close_info.start_time);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			strTemp.Format(_T("%S"), JnlInfo.day_close_info.day_close_Result);
			if (strTemp == _T("1"))	// Success
			{
				strTemp.Format(_T("  ** HOST **"));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.day_close_info.host_with_cnt);
				strTemp.Format(_T("    WITHDRAWAL COUNT = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.day_close_info.host_bal_cnt);
				strTemp.Format(_T("    BALANCE COUNT = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.day_close_info.host_trans_cnt);
				strTemp.Format(_T("    TRANSFER COUNT = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.day_close_info.total_host_amt);
				strTemp.Format(_T("    SETTLEMENT AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

			}
			else
			{
				strTemp.Format(_T("  ** HOST **"));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp.Format(_T("    FAILED TO GET DATA FROM HOST"));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}

			// Line Feed
			strTemp.Format(_T(""));
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			strTemp.Format(_T("  ** TERMINAL **"));
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			strTemp2.Format(_T("%S"), JnlInfo.day_close_info.term_with_cnt);
			strTemp.Format(_T("    WITHDRAWAL COUNT = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			strTemp2.Format(_T("%S"), JnlInfo.day_close_info.term_bal_cnt);
			strTemp.Format(_T("    BALANCE COUNT = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			strTemp2.Format(_T("%S"), JnlInfo.day_close_info.term_trans_cnt);
			strTemp.Format(_T("    TRANSFER COUNT = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			strTemp2.Format(_T("%S"), JnlInfo.day_close_info.total_term_amt);
			strTemp.Format(_T("    SETTLEMENT AMOUNT = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2));
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;
		}
		break;

	case denom_info:
		{
			// Title
			strTemp = _T("*** SET DENOMINATION ***");
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// Terminal ID
			strTemp.Format(_T("TERMINAL ID = %S"), JnlInfo.denomination_info.term_id);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// DATE / TIME
			strTemp.Format(_T("DATE / TIME = %S %S"), JnlInfo.denomination_info.start_date, JnlInfo.denomination_info.start_time);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// Added CBXCount
			strTemp.Format(_T("%S"),  JnlInfo.denomination_info.cst_count);

			int nJnlCBXCount = CUtil::StringToInt(strTemp);

			if (nJnlCBXCount > 0)
				nCSTCnt = nJnlCBXCount;

			int nOldDenom = 0, nNewDenom = 0;

			if (nCSTCnt >= 1)
			{
				strTemp.Format(_T("%S"), JnlInfo.denomination_info.olddenom_cst1);
				nOldDenom = CUtil::StringToInt(strTemp);

				strTemp2.Format(_T("%S"), JnlInfo.denomination_info.newdenom_cst1);
				nNewDenom = CUtil::StringToInt(strTemp2);

				if (nOldDenom != nNewDenom)
				{
					strTemp3.Format(_T("  CBX #1 = %s%d -> %s%d"), CURRENCY_SYMBOL, nOldDenom, CURRENCY_SYMBOL, nNewDenom);
					strReturnValue += strTemp3;
					strReturnValue += strDelimiter;
				}
				else
				{
					strTemp3.Format(_T("  CBX #1 = %s%d (NO CHANGE)"), CURRENCY_SYMBOL, nOldDenom);
					strReturnValue += strTemp3;
					strReturnValue += strDelimiter;
				}
			}

			if (nCSTCnt >= 2)
			{
				strTemp.Format(_T("%S"), JnlInfo.denomination_info.olddenom_cst2);
				nOldDenom = CUtil::StringToInt(strTemp);

				strTemp2.Format(_T("%S"), JnlInfo.denomination_info.newdenom_cst2);
				nNewDenom = CUtil::StringToInt(strTemp2);

				if (nOldDenom != nNewDenom)
				{
					strTemp3.Format(_T("  CBX #2 = %s%d -> %s%d"), CURRENCY_SYMBOL, nOldDenom, CURRENCY_SYMBOL, nNewDenom);
					strReturnValue += strTemp3;
					strReturnValue += strDelimiter;
				}
				else
				{
					strTemp3.Format(_T("  CBX #2 = %s%d (NO CHANGE)"), CURRENCY_SYMBOL, nOldDenom);
					strReturnValue += strTemp3;
					strReturnValue += strDelimiter;
				}
			}

			if (nCSTCnt >= 3)
			{
				strTemp.Format(_T("%S"), JnlInfo.denomination_info.olddenom_cst3);
				nOldDenom = CUtil::StringToInt(strTemp);

				strTemp2.Format(_T("%S"), JnlInfo.denomination_info.newdenom_cst3);
				nNewDenom = CUtil::StringToInt(strTemp2);

				if (nOldDenom != nNewDenom)
				{
					strTemp3.Format(_T("  CBX #3 = %s%d -> %s%d"), CURRENCY_SYMBOL, nOldDenom, CURRENCY_SYMBOL, nNewDenom);
					strReturnValue += strTemp3;
					strReturnValue += strDelimiter;
				}
				else
				{
					strTemp3.Format(_T("  CBX #3 = %s%d (NO CHANGE)"), CURRENCY_SYMBOL, nOldDenom);
					strReturnValue += strTemp3;
					strReturnValue += strDelimiter;
				}
			}

			if (nCSTCnt >= 4)
			{
				strTemp.Format(_T("%S"), JnlInfo.denomination_info.olddenom_cst4);
				nOldDenom = CUtil::StringToInt(strTemp);

				strTemp2.Format(_T("%S"), JnlInfo.denomination_info.newdenom_cst4);
				nNewDenom = CUtil::StringToInt(strTemp2);

				if (nOldDenom != nNewDenom)
				{
					strTemp3.Format(_T("  CBX #4 = %s%d -> %s%d"), CURRENCY_SYMBOL, nOldDenom, CURRENCY_SYMBOL, nNewDenom);
					strReturnValue += strTemp3;
					strReturnValue += strDelimiter;
				}
				else
				{
					strTemp3.Format(_T("  CBX #4 = %s%d (NO CHANGE)"), CURRENCY_SYMBOL, nOldDenom);
					strReturnValue += strTemp3;
					strReturnValue += strDelimiter;
				}
			}
		}
		break;

	case value_change:
		{
			// Title
			strTemp = _T("*** CHANGE VALUE ***");
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// Terminal ID
			strTemp.Format(_T("TERMINAL ID = %S"), JnlInfo.denomination_info.term_id);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// OLD VALUE
			strTemp.Format(_T("[OLD] %S"), JnlInfo.change_info.old_value);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// NEW VALUE
			strTemp.Format(_T("[NEW] %S"), JnlInfo.change_info.new_value);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;
		}
		break;

	case text_field:
	case power_on:
	case in_service:
	case out_of_service:
	case in_supervisor:
	case reboot:
		{
			// Title
			strTemp = _T("*** INFORMATION ***");
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			strTemp.Format(_T("%S"), JnlInfo.text_info.text_value);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;
		}
		break;

	case test_dispense:
		{
			// Title
			strTemp = _T("*** TEST DISPENSE ***");
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// Terminal ID
			strTemp.Format(_T("TERMINAL ID = %S"), JnlInfo.test_dispense_info.term_id);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// DATE / TIME
			strTemp.Format(_T("DATE / TIME = %S %S"), JnlInfo.test_dispense_info.start_date, JnlInfo.test_dispense_info.start_time);
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// Test Dispense Result
			CString strTestDispResult;
			strTestDispResult.Format(_T("%S"), JnlInfo.test_dispense_info.test_disp_result);

			if (strTestDispResult == _T("0"))
				strTemp.Format(_T("RESULT = NG"));
			else
				strTemp.Format(_T("RESULT = OK"));
			strReturnValue += strTemp;
			strReturnValue += strDelimiter;

			// Added CBXCount
			strTemp.Format(_T("%S"),  JnlInfo.test_dispense_info.cst_count);

			int nJnlCBXCount = CUtil::StringToInt(strTemp);

			if (nJnlCBXCount > 0)
				nCSTCnt = nJnlCBXCount;

			for(i=0; i<nCSTCnt; i++)
			{
				strTemp.Empty();
				//if(0 == i)
				//	strTemp.Format(_T("  ** CBX #1 **"));
				//else if(1 == i)
				//	strTemp.Format(_T("  ** CBX #2 **"));
				//else if(2 == i)
				//	strTemp.Format(_T("  ** CBX #3 **"));
				//else if(3 == i)
				//	strTemp.Format(_T("  ** CBX #4 **"));
				strTemp.Format(_T("  ** CBX #%d **"), i+1);
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.test_dispense_info.test_req_bill_cst[i]);
				strTemp.Format(_T("    REQUESTED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.test_dispense_info.test_pickup_bill_cst[i]);
				strTemp.Format(_T("    PICKED UP NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.test_dispense_info.test_rej_bill_cst[i]);
				strTemp.Format(_T("    REJECTED NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.test_dispense_info.rem_bill_cst[i]);
				strTemp.Format(_T("    CURRENT NUMBER OF BILLS = %s"), CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;

				strTemp2.Format(_T("%S"), JnlInfo.test_dispense_info.denom_cst[i]);
				strTemp.Format(_T("    VALUE OF EACH NOTE = %s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp2, FALSE));
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}

			// ErrorCode
			if (strTestDispResult == _T("0"))
			{
				strTemp.Format(_T("ERROR = %S"), JnlInfo.test_dispense_info.err_code);
				strReturnValue += strTemp;
				strReturnValue += strDelimiter;
			}
		}
		break;

	default:
		break;
	}

	return strReturnValue;
}

