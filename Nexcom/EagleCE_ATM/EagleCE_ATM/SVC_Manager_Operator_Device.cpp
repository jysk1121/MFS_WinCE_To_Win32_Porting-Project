#include "stdafx.h"
#include "SVC_Manager.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"
#include "../../EagleCE_ATM/EagleCE_ATM/EagleSVCLib.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define LAMP_COUNT		7
#define SENSOR_COUNT	3


/** *********************************************************
*	@brief		Device Main
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_Main()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform = _T("");
	CString strNetworkType = _T("");
	CString strButtonInfo;

	BOOL bExistCameraDriver = FALSE;	// V1.0.2.4 2018.07.27 - 카메라 드라이버 유무 판단

	m_bInitReset = FALSE;

	LOG(Info, _T("Device Main() ErrorCode:[%s], ErrorDevice:[0x%08x]"), CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode, CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice);

	strNetworkType = CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType;

	// CAMERA 연결
	// 카메라 드라이버 유무에 따라 Camera Open 시도를 결정한다.
	// 카메라 드라이버가 있을 경우, OP - Device 메뉴에 카메라 메뉴 활성화
	// 카메라 드라이버가 없을 경우, OP - Device 메뉴에 카메라 메뉴 비활성화
	if(TRUE == CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsExistCameraDriver())
	{
		bExistCameraDriver = TRUE;
		LOG(Info, _T("Device Main - CAMERA Driver is exist."));
	}
	else
	{
		bExistCameraDriver = FALSE;
		LOG(Info, _T("Device Main - CAMERA Driver is not exist."));
	}

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();
			strButtonInfo.Empty();

			// Set Network Information
			strScreenInform += strNetworkType;
			strScreenInform += SCR_CMD_DELIMITER;

			// Set Button Info
			// 1. Card Reader / 2. SIU / 3. Cash Dispenser / 4. Sound / 5. Printer / 6. PINPAD / 7. Camera
			// 하위 메뉴에서 권한별 Button Disable 처리
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

			// V1.0.2.4 2018.07.27 - Camera 추가
			if(TRUE == bExistCameraDriver)	// 카메라 드라이버가 있을 경우에는 카메라 메뉴 활성화
			{
				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;
			}
			else
			{
				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;
			}
			/////////////////////////////////////

			Operator_DisplayScreen(OP_SCREEN_DEVICE, bKeyEnable, strScreenInform, strButtonInfo);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Device_Main - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER || m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			// Next Action
			if(m_strScrOutData == _T("CARDREADER"))
			{
				// LEFT 1
				m_bInitReset = TRUE;
				m_strNextScrNum = OP_SCREEN_DEVICE_CDR;
				break;
			}
			else if(m_strScrOutData == _T("CASHDISPENSER"))
			{
				// LEFT 2
				m_bInitReset = TRUE;
				m_strNextScrNum = OP_SCREEN_DEVICE_CDM;
				break;
			}
			else if(m_strScrOutData == _T("PRINTER"))
			{
				// LEFT 3
				m_bInitReset = TRUE;
				m_strNextScrNum = OP_SCREEN_DEVICE_PRINTER;
				break;
			}
			else if(m_strScrOutData == _T("PINPAD"))
			{
				// LEFT 4
				m_strNextScrNum = OP_SCREEN_DEVICE_PINPAD;
				break;
			}
			else if(m_strScrOutData == _T("SIU"))
			{
				// RIGHT 1
				m_strNextScrNum = OP_SCREEN_DEVICE_SIU;
				break;
			}
			else if(m_strScrOutData == _T("SOUND"))
			{
				// RIGHT 2
				m_strNextScrNum = OP_SCREEN_DEVICE_SOUND;
				break;
			}
			else if(m_strScrOutData == _T("NETWORK"))
			{
				// RIGHT 3
				if(_T("MODEM") == strNetworkType)
					m_strNextScrNum = OP_SCREEN_DEVICE_MODEM;
				else
					m_strNextScrNum = OP_SCREEN_DEVICE_TCPIP;
				break;
			}
			else if(m_strScrOutData == _T("CAMERA"))
			{
				// RIGHT 4
				m_strNextScrNum = OP_SCREEN_DEVICE_CAMERA;
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

/** *********************************************************
*	@brief		Camera
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_Camera()
{
	BOOL bResult = FALSE;
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strErrorCode;
	CString strScreenInform;
	CString strUseCamera;
	BOOL bSetCameraPos = FALSE;
	BOOL bCameraRunGraph = FALSE;
	RECT rc = { 0, 0, 0, 0};

	m_nCurrentFocusIndex = 1;

	// Camera를 테스트하기 위한 용도이므로 이미 Open되어져 있다면 Close후에 다시 Open하도록 로직 수정
	Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

	// Camera Test를 위한 용도이므로 여기서는 무조건 Camera를 Open해서 사용
	if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsOpened() == FALSE)
	{
		// Camera Open 수행
 		if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CAMERA.Open())
		{
			// Camera Error 표시 후 이전 Menu로 Return 처리
			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CAMERA.GetLastError();
			LOG(Error, _T("CAMERA - Open is failed. Error Code : ") + strErrorCode);

			Operator_Processing_Screen(_T(""), TRUE, strErrorCode);

			// 진입 장애시 이전 화면으로 전환
			m_strNextScrNum = OP_SCREEN_DEVICE;
			return;
		}
	}
	else
	{
		// 이미 Open되어져 있으면 Close후에 다시 Open
		CDEV_Manager::GetInstance()->m_DEV_CAMERA.Close();

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);

		if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CAMERA.Open())
		{
			// Camera Error 표시 후 이전 Menu로 Return 처리
			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CAMERA.GetLastError();
			LOG(Error, _T("CAMERA - Open is failed. Error Code : ") + strErrorCode);

			Operator_Processing_Screen(_T(""), TRUE, strErrorCode);

			// 진입 장애시 이전 화면으로 전환
			m_strNextScrNum = OP_SCREEN_DEVICE;
			return;
		}
	}

	// CAMERA Open후 바로 RunGraph 전 1초 Delay
	CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);

	if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.GetCameraSetting() == TRUE)
		strUseCamera = _T("ENABLE");
	else
		strUseCamera = _T("DISABLE");

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			// Camera 설정값 표시
			strScreenInform.Empty();
			strScreenInform = strUseCamera;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_DEVICE_CAMERA, bKeyEnable, strScreenInform);

			bShowScreen = FALSE;
			bKeyEnable = FALSE;

			if (bCameraRunGraph == FALSE)
			{
				bCameraRunGraph = CDEV_Manager::GetInstance()->m_DEV_CAMERA.RunGraph();
				CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);

				if(FALSE == bCameraRunGraph)
				{
					strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CAMERA.GetLastError();
					LOG(Error, _T("CAMERA - RunGraph is failed. Error Code : ") + strErrorCode);

					Operator_Processing_Screen(_T(""), TRUE, strErrorCode);

					// 진입 장애시 이전 화면으로 전환
					m_strNextScrNum = OP_SCREEN_DEVICE;
					break;
				}
			}

			if(FALSE == bSetCameraPos)
			{
				//RECT rc = {363, 290, 554, 337};
				rc = CSCR_Manager::GetInstance()->GetPosValue4Camera();

				// Pen 표시를 위해 -2 조정
				rc.left += 2;
				rc.top += 2;
				rc.right -= 2;
				rc.bottom -= 2;

				CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_500MS);	// 화면 갱신을 먼저하기 위해 500ms Delay 추가
				CDEV_Manager::GetInstance()->m_DEV_CAMERA.SetVideoWindosPos(&rc);
				bSetCameraPos = TRUE;
			}
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Device_Camera - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_DEVICE;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			if (m_strScrOutData == _T("CAPTURE"))
			{
				Proc_Operator_Device_Camera_Capture();
				bShowScreen = TRUE;
				bCameraRunGraph = FALSE;
				bSetCameraPos = FALSE;
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				if (strUseCamera == _T("ENABLE"))
					strUseCamera = _T("DISABLE");
				else
					strUseCamera = _T("ENABLE");

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SET"))
			{
				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

				// "WAIT 화면 표시를 위해 Camera Position 변경 (TOP MOST이기 때문)
				memset(&rc, 0, sizeof(rc));
				CDEV_Manager::GetInstance()->m_DEV_CAMERA.SetVideoWindosPos(&rc);
				bSetCameraPos = FALSE;

				if (strUseCamera == _T("ENABLE"))
					CDEV_Manager::GetInstance()->m_DEV_CAMERA.SetUseCamera(TRUE);
				else
					CDEV_Manager::GetInstance()->m_DEV_CAMERA.SetUseCamera(FALSE);

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

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

	if (bCameraRunGraph == TRUE)
	{		
		CDEV_Manager::GetInstance()->m_DEV_CAMERA.StopGraph(TRUE);
	}

	// default window
	memset(&rc, 0, sizeof(rc));
	CDEV_Manager::GetInstance()->m_DEV_CAMERA.SetVideoWindosPos(&rc);

	// Camera 설정이 Disable이면 Camera Close 처리 (TRUE이면 Open 상태 유지)
	if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.GetCameraSetting() == FALSE)
	{
		if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsOpened() == TRUE)
			CDEV_Manager::GetInstance()->m_DEV_CAMERA.Close();
	}
}


/** *************************************************************
*	@brief		Camera Capture
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Device_Camera_Capture()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp;

	BOOL bResult = FALSE;
	CString strImageFilePath;
	CString strErrorCode;
	int i=0;

	RECT rc = { 0, 0, 0, 0};

	// Image Capture 직전, 파일 지움. - 거래 연동 하면서 Image Folder 통일 시킴
	CUtil::DeleteFileInDirectory(CAPTURE_IMAGE_FILE_PATH_ABS, _T("*.*"));

	// Processing Message Display
	Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

	CDEV_Manager::GetInstance()->m_DEV_CAMERA.SetVideoWindosPos(&rc);

	//bResult = CDEV_Manager::GetInstance()->m_DEV_CAMERA.StillCapture(FALSE, FALSE);
	CDEV_Manager::GetInstance()->m_DEV_CAMERA.StillCapture(FALSE, TRUE);	// Camera Holding Recovery를 위해 Async로 방식 변경

	// 10초간 완료 여부 확인. (Camera Timeout이 5초이므로 2배 대기)
	int nCount = 0;
	BOOL bNeedToRecovery = FALSE;

	while(TRUE)
	{
		if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsCameraProcessing() == FALSE)
			break;

		if (nCount > 100)
		{
			// Timeout Need To Recovery
			bNeedToRecovery = TRUE;
			break;
		}

		nCount++;
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CAMERA.GetLastError();

	if ((strErrorCode.IsEmpty() == TRUE) && (bNeedToRecovery == FALSE))
		bResult = TRUE;

	if (bResult == FALSE)
	{
		if (bNeedToRecovery == FALSE)
			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CAMERA.GetLastError();
		else
			strErrorCode = _T("CAMERA ERROR\nATM WILL BE REBOOT FOR RECOVERY");

		LOG(Error, _T("CAMERA - StillCapture is failed. Error Code : ") + strErrorCode);

		Operator_Processing_Screen(_T(""), TRUE, strErrorCode);

		if (bNeedToRecovery == TRUE)
		{
			LOG(Info, _T("Camera is Processing - Recovery Start - System Reboot"));

			CString strTemp;
			strTemp = _T("[RECOVERY] SYSTEM REBOOT");
			CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(reboot, strTemp);

			CDEV_Manager::GetInstance()->System_Reboot(TRUE);
		}

		m_strNextScrNum = OP_SCREEN_DEVICE;
	}

	strImageFilePath.Format(_T("%s\\%s"), CAPTURE_IMAGE_FILE_PATH_ABS, SUPERVISOR_CAPTURE_IMAGE_NAME);

	if(TRUE == bResult)
	{
		while(TRUE) 
		{ 
			CheckService();

			if (bShowScreen == TRUE)
			{  
				strScreenInform.Empty();
				strButtonInform.Empty();
				m_strScrOutName.Empty();
				m_strScrOutData.Empty();

				strScreenInform += strImageFilePath;
				strScreenInform += SCR_CMD_DELIMITER;

				Operator_DisplayScreen(OP_SCREEN_DEVICE_CAMERA_CAPTURE, bKeyEnable, strScreenInform);

				bShowScreen = FALSE;
				bKeyEnable = FALSE;
			}

			if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
			{
				CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

				LOG(Info, _T("User Select - [%s] [%s]"), m_strScrOutName, m_strScrOutData);

				// Common Action
				if (m_strScrOutData == S_TIMEOVER || m_strScrOutData == S_ENTER)
				{
					m_strNextScrNum = OP_SCREEN_DEVICE_CAMERA;
					break;
				}

			}

			Sleep(EAGLE_SLEEP_INTERVAL);
		}
	}

	CUtil::DeleteFileInDirectory(CAPTURE_IMAGE_FILE_PATH_ABS, _T("*.*"));
}


/** *********************************************************
*	@brief		CardReader Main
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_CardReader()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	BOOL bExit = FALSE;
	BOOL bResult = TRUE;
	CString strScreenInform = _T("");
	CString strErrorCode = _T("");
	CString strButtonInfo;

	// CardReader 화면 진입 전, 기기 정상인지 확인
	if (m_bInitReset == TRUE)
	{
		m_bInitReset = FALSE;

		strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();
		if(FALSE == strErrorCode.IsEmpty())
		{
			LOG(Error, _T("CDR - Device is error. Error Code : ") + strErrorCode);

			CEagleSVCLib::GetInstance()->Set_DeviceErrorCode(DEVICE_CDR, strErrorCode);

			Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
		}
	}

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();
			strButtonInfo.Empty();

			Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

			strScreenInform = Operator_CDR_GetLatchOption();

			// V1.0.2.3 - Replenishment 권한일 경우, CDR의 Reset, CDM의 Reset/TestDispense, PTR의 Reset 메뉴 활성화 하도록 수정
			// Set Button Info
			// 1. Reset / 2. IC Card Test / 3. Latch Option / 4. Scan Card Test
			if (GetOperatorMode() == EAGLE_ATM_SVC_REPLENISH)
			{
				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
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
			}

			Operator_DisplayScreen(OP_SCREEN_DEVICE_CDR, bKeyEnable, strScreenInform, strButtonInfo);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Device_CardReader - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_DEVICE;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("RESET"))
			{
				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

				bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.Initialize();

				if(TRUE == bResult)
				{
					CDEV_Manager::GetInstance()->m_DEV_CDR.GetVersionInfo();

					CEagleSVCLib::GetInstance()->Clear_DeviceError(DEVICE_CDR);

					Operator_Processing_Screen(_T("SUCCESS"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}
				else
				{
					strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();
					LOG(Error, _T("CDR - Initialize is failed. Error Code : ") + strErrorCode);

					CEagleSVCLib::GetInstance()->Set_DeviceErrorCode(DEVICE_CDR, strErrorCode);

					Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
				}

			}
			else if (m_strScrOutData == _T("ICTEST"))
			{
				if (Proc_Operator_Device_CardReader_InsertCard(TRUE) == TRUE)
				{
					Proc_Operator_Device_CardReader_ReadData(TRUE);
					Proc_Operator_Device_CardReader_RemoveCard(TRUE);
				}
			}
			else if (m_strScrOutData == _T("SCANTEST"))
			{
				if (Proc_Operator_Device_CardReader_InsertCard(FALSE) == TRUE)
				{
					if (Proc_Operator_Device_CardReader_RemoveCard(FALSE) == TRUE)
					{
						Proc_Operator_Device_CardReader_ReadData(FALSE);

						strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();

						if(CDR_CERTIFICATION_ERROR == strErrorCode)
						{
							m_strNextScrNum = OP_SCREEN_DEVICE;
							break;
						}
					}
				}
			}
			else if (m_strScrOutData == _T("LATCHOPTION"))
			{
				Proc_Operator_Device_CardReader_LatchOption();
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			bShowScreen = TRUE;
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}
}


/** *********************************************************
*	@brief		CDR Insert Card
*	@param	BOOL bICCard		IC/Scan Card Test 여부
*	@retval TRUE	성공 TRUE / 실패 FALSE
************************************************************/
BOOL CSVC_Manager::Proc_Operator_Device_CardReader_InsertCard(BOOL bICCard)
{
	BOOL bResult = FALSE;
	CString strErrorCode = _T("");
	CString strCommand = _T("");
	CString strTitle = _T("");

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	// 2. LED ON
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_FLICKING);
	CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(TRUE);

	// 1. Screen Setting
	// Prepare screen
	CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), OP_SCREEN_DEVICE_CDR_INSERT);

	if(TRUE == bICCard)
		strTitle = _T("IC CARD TEST");
	else
		strTitle = _T("SCAN CARD TEST");

	// Get Screen Information
	strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
	CSCR_Manager::GetInstance()->SetVariable(strCommand, strTitle);

	// Set Timeout
	CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
	CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(SUPERVISOR_SCREEN_TIMEOUT));

	// Display Screen
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), OP_SCREEN_DEVICE_CDR_INSERT);

	///////////////////////////////////
	// GET KEY STRING
	while (TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		// 3. Insert Card
		bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.InsertCard();

		if (bResult == TRUE)
			break;

		// 준정상일 경우, CardReader Main 화면으로 돌아간다.
		if (CDEV_Manager::GetInstance()->m_DEV_CDR.IsUnSuccessReply() == FALSE)
		{
			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();

			LOG(Error,_T("CDR - InsertCard is failed. Error Code : ") + strErrorCode);

			// 2. LED OFF
			CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
			CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(FALSE);

			Operator_Processing_Screen(_T(""), TRUE, strErrorCode);

			return FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("User Select - [%s] [%s]"), m_strScrOutName, m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER || m_strScrOutData == S_EXIT)
			{
				// 2. LED OFF
				CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
				CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(FALSE);

				return FALSE;
			}
		}

		// 대기
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// 2. LED OFF
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
	CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(FALSE);

	return TRUE;
}

/** *********************************************************
*	@brief		CDR Remove Card
*	@param	BOOL bICCard		IC/Scan Card Test 여부
*	@retval TRUE	성공 TRUE / 실패 FALSE
************************************************************/
BOOL CSVC_Manager::Proc_Operator_Device_CardReader_RemoveCard(BOOL bICCard)
{
	BOOL bResult = FALSE;
	CString strErrorCode = _T("");
	CString strCommand = _T("");
	CString strTitle = _T("");
	DWORD dwReplyType = 0;
	DWORD dwReturnValue = 0;
	DWORD dwStartTime = ::GetTickCount();

	// 2. LED ON
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_FLICKING);
	CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(TRUE);

	// 1. Screen Setting

	// Prepare screen
	CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), OP_SCREEN_DEVICE_CDR_REMOVE);

	if(TRUE == bICCard)
		strTitle = _T("IC CARD TEST");
	else
		strTitle = _T("SCAN CARD TEST");

	// Get Screen Information
	strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
	CSCR_Manager::GetInstance()->SetVariable(strCommand, strTitle);

	// Set Timeout
	CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
	CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(SUPERVISOR_SCREEN_TIMEOUT));

	// Display Screen
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), OP_SCREEN_DEVICE_CDR_REMOVE);

	///////////////////////////////////
	// GET KEY STRING
	while (TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if(TRUE == bICCard)
		{
			// Initialize 명령을 수행해야 Latch가 풀림.
			bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.Initialize();

			if(FALSE == bResult)
			{
				// 카드 위치 정보 조회 후, 기기내 카드 미존재시 화면 전환하도록 수정
				if(CARD_EMPTY == CDEV_Manager::GetInstance()->m_DEV_CDR.GetMediaStatus())
					break;

				strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();
				LOG(Error,_T("CDR - Initialize is failed. Error Code : ") + strErrorCode);

				// 2. LED OFF
				CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
				CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(FALSE);

				Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
				return FALSE;
			}

			if(CARD_EMPTY == CDEV_Manager::GetInstance()->m_DEV_CDR.GetMediaStatus())
				break;
		}
		else
		{
			// 2. Remove Card
			bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.RemoveCard();

			if (bResult == TRUE)
				break;

			if (CDEV_Manager::GetInstance()->m_DEV_CDR.IsUnSuccessReply() == FALSE)
			{
				strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();

				LOG(Error,_T("CDR - RemoveCard is failed. Error Code : ") + strErrorCode);

				// 2. LED OFF
				CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
				CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(FALSE);

				Operator_Processing_Screen(_T(""), TRUE, strErrorCode);

				return FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("User Select - [%s] [%s]"), m_strScrOutName, m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER || m_strScrOutData == S_EXIT)
			{
				// 2. LED OFF
				CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
				CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(FALSE);

				return FALSE;
			}
		}
		
		// 대기
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// 2. LED OFF
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
	CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(FALSE);

	return TRUE;
}

/** *********************************************************
*	@brief		CDR ReadData
*	@param	BOOL bICCard		IC/Scan Card Test 여부
*	@param	BOOL &bReadSuccess	Read 성공 유무
*	@retval TRUE	화면 종료
*	@retval FALSE	동작 진행
************************************************************/
BOOL CSVC_Manager::Proc_Operator_Device_CardReader_ReadData(BOOL bICCard)
{
	BOOL bResult = FALSE;
	int nRetISO1 = 0;
	int nRetISO2 = 0;
	int nRetISO3 = 0;
	CString strCommand = _T("");
	CString strErrorCode = _T("");
	CString strTitle = _T("");

	CString strISO1 = _T("");
	CString strISO2 = _T("");
	CString strISO3 = _T("");
	CString strICData = _T("");
	CString strICDataTemp = _T("");
	BYTE ATR[MAX_DATA_ARRAY_SIZE] = {0,};
	DWORD dwDataSize = 0;
	CString strReadData = _T("");
	CString strTempData;
	int nLineWidth = 50;

	// 2. ReadData
	if(TRUE == bICCard)
	{
		bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.ICCPowerOn(ATR, dwDataSize);
		if(FALSE == bResult)
		{
			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();
			LOG(Error,_T("CDR - InsertCard is failed. Error Code : ") + strErrorCode);

			Operator_Processing_Screen(_T(""), TRUE, strErrorCode);

			return TRUE;
		}

		// Read 성공했지만, size가 0일 경우 메시지 display
		if(0 == dwDataSize)
		{
			strICData = _T("ATR DATA MISSING");
		}
		else
		{
			for(int i=0; i<(int)dwDataSize; i++)
			{
				strICDataTemp.Format(_T("%02X "), ATR[i]);
				strICData += strICDataTemp;
			}
		}
		strReadData.Format(_T("Data => %s"), strICData);
	}
	else
	{
		if(TRUE == CDEV_Manager::GetInstance()->m_DEV_CDR.GetSupportEncryption())
		{
			// MS data 암/복호화를 위한 키 교환
			BOOL bEncrypResult = FALSE;
			bEncrypResult = CDEV_Manager::GetInstance()->m_DEV_CDR.GenerateKey4MagneticData();
			if(FALSE == bEncrypResult)
			{
				CString strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();

				LOG(Error, _T("CDR - GenerateKey4MagneticData() is failed - Error Code : ") + strErrorCode);

				// 암호화 관련 에러이기 때문에, 에러코드를 설정해서 시스템을 멈추게 함.
				CEagleSVCLib::GetInstance()->Set_DeviceErrorCode(DEVICE_CDR, strErrorCode);
				Operator_Processing_Screen(_T(""), TRUE, strErrorCode);

				return TRUE;
			}
			else
			{
				LOG(Info, _T("CDR - GenerateKey4MagneticData() is success"));
			}
		}

		// Read ISO1
		nRetISO1 = CDEV_Manager::GetInstance()->m_DEV_CDR.ReadCardData_ISO1(&strISO1);
		if (0 == nRetISO1)
		{
			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();
			LOG(Error, _T("CDR - IOS1 read failed. Error Code : ") + strErrorCode);

			strISO1 = _T("TRACK1 READ FAILED.");

			if( (CARDDATA_ERROR != strErrorCode) && (strErrorCode.IsEmpty() == FALSE) )
			{
				Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
				return TRUE;
			}
		}
		else
		{
			strTempData.Empty();
			if(strISO1.GetLength() > nLineWidth)
				strTempData.Format(_T("%s\n%s"), strISO1.Left(nLineWidth), strISO1.Mid(nLineWidth));
			else
				strTempData = strISO1;

			strISO1 = strTempData;
		}

		// Read ISO2
		nRetISO2 = CDEV_Manager::GetInstance()->m_DEV_CDR.ReadCardData_ISO2(&strISO2);
		if (0 == nRetISO2)
		{
			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();
			LOG(Error, _T("CDR - IOS2 read failed. Error Code : ") + strErrorCode);

			strISO2 = _T("TRACK2 READ FAILED.");

			if( (CARDDATA_ERROR != strErrorCode) && (strErrorCode.IsEmpty() == FALSE) )
			{
				Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
				return TRUE;
			}
		}
		else
		{
			strTempData.Empty();
			if(strISO2.GetLength() > nLineWidth)
				strTempData.Format(_T("%s\n%s"), strISO2.Left(nLineWidth), strISO2.Mid(nLineWidth));
			else
				strTempData = strISO2;

			strISO2 = strTempData;
		}

		// Read ISO3
		nRetISO3 = CDEV_Manager::GetInstance()->m_DEV_CDR.ReadCardData_ISO3(&strISO3);
		if (0 == nRetISO3)
		{
			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();
			LOG(Error, _T("CDR - IOS3 read failed. Error Code : ") + strErrorCode);

			strISO3 = _T("TRACK3 READ FAILED.");

			if( (CARDDATA_ERROR != strErrorCode) && (strErrorCode.IsEmpty() == FALSE) )
			{
				Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
				return TRUE;
			}
		}
		else
		{
			strTempData.Empty();
			if(strISO3.GetLength() > nLineWidth)
				strTempData.Format(_T("%s\n%s"), strISO3.Left(nLineWidth), strISO3.Mid(nLineWidth));
			else
				strTempData = strISO3;

			strISO3 = strTempData;
		}

		strReadData.Format(_T("TRACK1 => \n%s\n\nTRACK2 => \n%s\n\nTRACK3 => \n%s"), strISO1, strISO2, strISO3);
	}

	// 3. Clear Buffer
	bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.ClearBuffer();
	if (FALSE == bResult)
	{
		strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();
		LOG(Error, _T("CDR - ClearBuffer is failed. Error Code : ") + strErrorCode);

		Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
		return TRUE;
	}

	// 5. Display Screen
	if(TRUE == bResult)
	{
		// PrevSet
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), OP_SCREEN_DEVICE_CDR_RESULT);

		if(TRUE == bICCard)
			strTitle = _T("IC CARD TEST");
		else
			strTitle = _T("SCAN CARD TEST");

		// Get Screen Information
		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
		CSCR_Manager::GetInstance()->SetVariable(strCommand, strTitle);

		// 4. Set ReadData
		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 2);
		CSCR_Manager::GetInstance()->SetVariable(strCommand, strReadData);

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(SUPERVISOR_INFO_SCREEN_TIMEOUT));

		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), OP_SCREEN_DEVICE_CDR_RESULT);
		
		///////////////////////////////////
		// GET KEY STRING
		while (TRUE)
		{
			CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

			if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
			{
				CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

				LOG(Info, _T("User Select - [%s] [%s]"), m_strScrOutName, m_strScrOutData);
				
				if( (m_strScrOutData == S_ENTER) || (m_strScrOutData == S_TIMEOVER) )
				{
					return TRUE;
				}
			}

			// 대기
			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
		}
	}

	return FALSE;
}

/** *********************************************************
*	@brief		CDR LatchOption 가져오기
*	@retval ON	Latch ON
*	@retval OFF	Latch OFF
************************************************************/
CString CSVC_Manager::Operator_CDR_GetLatchOption()
{
	BOOL bResult = FALSE;
	BYTE byRecvBuff[MAX_DATA_ARRAY_SIZE] = {0,};
	CString strLatchOption = _T("");
	CString strErrorCode = _T("");

	bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.ReadLatch(byRecvBuff);

	if(FALSE == bResult)
	{
		strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();
		LOG(Error, _T("CDR - ReadLatch is failed. Error Code : ") + strErrorCode);

		strLatchOption = _T("UNKNOWN");
	}
	else
	{
		if(0x30 == byRecvBuff[0])
			strLatchOption = _T("OFF");
		else
			strLatchOption = _T("ON");
	}

	return strLatchOption;
}

/** *********************************************************
*	@brief		CDR LatchOption
*	@retval TRUE	화면 종료
*	@retval FALSE	동작 진행
************************************************************/
BOOL CSVC_Manager::Proc_Operator_Device_CardReader_LatchOption()
{
	BOOL bResult = FALSE;
	BYTE byRecvBuff[MAX_DATA_ARRAY_SIZE] = {0,};
	CString strErrorCode = _T("");

	Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

	// 1. 현재의 Latch 정보 read
	bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.ReadLatch(byRecvBuff);

	if (FALSE == bResult)
	{
		strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();
		LOG(Error, _T("CDR - ReadLatch is failed_2. Error Code : ") + strErrorCode);

		Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
		return TRUE;
	}

	// 2. Latch 정보 Set(ON일 경우 OFF로 Set, OFF일 경우 ON으로 Set)
	if(0x30 == byRecvBuff[0])
		bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.SetLatch(TRUE);
	else
		bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.SetLatch(FALSE);
		
	if (FALSE == bResult)
	{
		strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();
		LOG(Error, _T("CDR - ExecuteLatchOption is failed. Error Code : ") + strErrorCode);

		Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
		return TRUE;
	}
	
	return FALSE;
}

/** *********************************************************
*	@brief		준정상 장애코드인지 판단하는 함수
*	@param	CString strErrorCode		ErrorCode
*	@retval TRUE	준정상 장애
*	@retval FALSE	이상 장애
************************************************************/
//BOOL CSVC_Manager::IsUnSuccessReply(CString strErrorCode)
//{
//	CString strCode = strErrorCode.Right(2);
//
//	if( (strCode == _T("07")) || (strCode == _T("08")) || (strCode == _T("09")) || (strCode == _T("00")) || (strCode.IsEmpty() == TRUE))
//	{
//		CDEV_Manager::GetInstance()->m_DEV_CDR.ClearErrorCode();
//		return TRUE;
//	}
//	else
//		return FALSE;
//}

/** *********************************************************
*	@brief		Sound Menu Screen
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_Sound()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = FALSE;
	CString strButtonInform = _T("");
	int nVolumeLevel = 0;
	CString strSoundInfo = _T("");
	int i = 0;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strButtonInform.Empty();

			// 현재의 volumn level을 가져옴.
			nVolumeLevel = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strSound_Volume);
			
			for(i=0; i<nVolumeLevel; i++)
			{
				strButtonInform += _T("on");
				strButtonInform += SCR_CMD_DELIMITER;
			}

			for(i=nVolumeLevel; i<10; i++)
			{
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
			}

			Operator_DisplayScreen(OP_SCREEN_DEVICE_SOUND, bKeyEnable, _T(""), strButtonInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Device_Sound - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_DEVICE;
				break;
			}

			// Next Action
			if ( (m_strScrOutData == _T("UP")) || (m_strScrOutData == _T("DOWN")) )
			{
				if(m_strScrOutData == _T("UP"))
					CDEV_Manager::GetInstance()->SetVolumeUp();
				else
					CDEV_Manager::GetInstance()->SetVolumeDown();

				PlaySound(CUtil::GetKeyPressWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);

				nVolumeLevel = CDEV_Manager::GetInstance()->GetVolume();
				CEagleDataManager::GetInstance()->m_Config.m_Option.strSound_Volume = CUtil::IntToString(nVolumeLevel);
				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("sound_val"), CEagleDataManager::GetInstance()->m_Config.m_Option.strSound_Volume);

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

/** *********************************************************
*	@brief		CashDispenser Main
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_CashDispenser()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	BOOL bResult = TRUE;
	CString strErrorCode = _T("");
	CString strButtonInfo;

	// Cash Dispenser 화면 진입 전, 기기 정상인지 확인
	if(TRUE == m_bInitReset)
	{
		m_bInitReset = FALSE;

		Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

		// 진입시 Delay로 인해 reset 명령 제거 (하기 Status로 정상 유무를 판단함
		CDMSTATUS stResult;
		memset(&stResult, 0, sizeof(CDMSTATUS));

		// 현재 CDM이 장애 상태라면, CDM_Status()는 FALSE를 리턴한다.
		bResult = CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_Status(&stResult);

		// 장애가 나더라도 CDU는 TEST MENU로 전환한다. (CBX Denomination 기능 사용을 위해)
		if(FALSE == bResult)
		{
			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError();
			LOG(Error, _T("CDM - CDM_Status is error. Error Code : ") + strErrorCode);

			CEagleSVCLib::GetInstance()->Set_DeviceErrorCode(DEVICE_CDM, strErrorCode);

			Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
		}
	}

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strButtonInfo.Empty();

			// V1.0.2.3 - Replenishment 권한일 경우, CDR의 Reset, CDM의 Reset/TestDispense, PTR의 Reset 메뉴 활성화 하도록 수정
			// V1.0.2.10 - Admin은 CBX Setting 불가하도록 권한 변경
			// Set Button Info
			// 1. Cassette Settings / 2. Set Bill / 3. Reset / 4. Learn Bill / 5. TestDispense
			if (GetOperatorMode() == EAGLE_ATM_SVC_REPLENISH)
			{
				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;
			}
			else if (GetOperatorMode() == EAGLE_ATM_SVC_ADMIN)
			{
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
			}

			Operator_DisplayScreen(OP_SCREEN_DEVICE_CDM, bKeyEnable, _T(""), strButtonInfo);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Device_CashDispenser - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_DEVICE;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("CASSETTESET"))
			{
				// LEFT 2
				m_strNextScrNum = OP_SCREEN_DEVICE_CASSETTESETTING;
				break;
			}
			else if (m_strScrOutData == _T("SETBILL"))		// Size/Thickness 설정 화면
			{
				// LEFT 3
				m_strNextScrNum = OP_SCREEN_DEVICE_SETBILL;
				break;
			}
			else if (m_strScrOutData == _T("LEARNBILL"))
			{
				// LEFT 4
				m_strNextScrNum = OP_SCREEN_DEVICE_LEARNING;
				break;
			}
			else if (m_strScrOutData == _T("RESET"))
			{
				// RIGHT 3
				Proc_Operator_Device_CashDispenser_Reset();
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("TESTDISPENSE"))
			{
				// RIGHT 4
				m_strNextScrNum = OP_SCREEN_DEVICE_TESTDISPENSE;
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

/** *********************************************************
*	@brief		CashDispenser - Reset
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_CashDispenser_Reset()
{
	BOOL bResult = FALSE;
	CString strErrorCode = _T("");

	Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

	CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStop();	// Polling Stop

	bResult = CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_Initialize(TRUE);

	CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStart();	// Polling Start

	if(TRUE == bResult)
	{
		CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_CfgStatus();	// Version Read를 위해 추가로 Call

		CEagleSVCLib::GetInstance()->Clear_DeviceError(DEVICE_CDM);

		Operator_Processing_Screen(_T("SUCCESS"));
		CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
	}
	else
	{
		strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError();
		LOG(Error, _T("CDM - CDM_Reset is failed. Error Code : ") + strErrorCode);

		CEagleSVCLib::GetInstance()->Set_DeviceErrorCode(DEVICE_CDM, strErrorCode);

		Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
	}
}

/** *********************************************************
*	@brief		CashDispenser - TestDispense
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_CashDispenser_TestDispense()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	BOOL bReject = FALSE;
	CString strScreenInform = _T("");
	CString strErrorCode = _T("");
	CString strCountInfo = _T("");
	CString strOutString = _T("");
	int nCbxCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);
	int nMaxFocusIndex = nCbxCount;
	int i=0;
	int nRemainCount = 0;
	int nRejectCount = 0;
	int nTotalRejectCount = 0, nTempTotal = 0;
	int nResult = CDM_FAILED_TO_SEND_COMMAND;
	int nTempFocusIndex = 0;
	int nTotalRemainCount = 0, nTotalRequestCount = 0;
	
	CDM_COUNT_INFO cstCountInfo;
	CDMCBXITEM cbxItem;
	CDMMULTIDISPENSE dispResult;
	CDMLASTDISPENSE lastDispense;

	ZeroMemory(&cstCountInfo, sizeof(CDM_COUNT_INFO));
	ZeroMemory(&cbxItem, sizeof(CDMCBXITEM));
	ZeroMemory(&dispResult, sizeof(CDMMULTIDISPENSE));
	ZeroMemory(&lastDispense, sizeof(CDMLASTDISPENSE));

	LOG(Info, _T("CDM - TestDispense - CbxCount : [%d]"), nCbxCount);

	m_nCurrentFocusIndex = 1;

	for(i=0; i<nCbxCount; i++)
		cstCountInfo.byReqCount[i] = 1;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			for(i=0; i<MAX_CST_COUNT; i++)
			{
				// ReqCount
				if(i+1 > nCbxCount)
					strCountInfo = _T("N/A");
				else
					strCountInfo.Format(_T("%d"), cstCountInfo.byReqCount[i]);
				strScreenInform += strCountInfo;
				strScreenInform += SCR_CMD_DELIMITER;
			}
			
			for(i=0; i<MAX_CST_COUNT; i++)
			{
				// Pickup Count
				if(i+1 > nCbxCount)
					strCountInfo = _T("N/A");
				else
					strCountInfo.Format(_T("%d"), cstCountInfo.byPickupCount[i]);
				strScreenInform += strCountInfo;
				strScreenInform += SCR_CMD_DELIMITER;
			}

			for(i=0; i<MAX_CST_COUNT; i++)
			{
				// Rejected Count
				if(i+1 > nCbxCount)
					strCountInfo = _T("N/A");
				else
					strCountInfo.Format(_T("%d"), cstCountInfo.byRejectedCount[i]);
				strScreenInform += strCountInfo;
				strScreenInform += SCR_CMD_DELIMITER;
			}

			Operator_DisplayScreen(OP_SCREEN_DEVICE_TESTDISPENSE, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Device_CashDispenser_TestDispense - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_DEVICE_CDM;
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
			else if (m_strScrOutData == _T("TESTDISPENSE"))
			{
				ZeroMemory(cstCountInfo.byPickupCount, sizeof(cstCountInfo.byPickupCount));
				ZeroMemory(cstCountInfo.byRejectedCount, sizeof(cstCountInfo.byRejectedCount));
				ZeroMemory(&lastDispense, sizeof(CDMLASTDISPENSE));
				nTotalRejectCount = 0;
				bReject = FALSE;
				nTotalRemainCount = 0;
				nTotalRequestCount = 0;

				for(i=0; i<nCbxCount; i++)
				{
					nTotalRemainCount += CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);
					nTotalRequestCount += cstCountInfo.byReqCount[i];
				}

				// 잔여 매수가 0일 경우 처리
				if(0 == nTotalRemainCount)
				{
					LOG (Info, _T("REMAIN BILL COUNT IS ZERO. (%d)"), nTotalRemainCount);

					Operator_Processing_Screen(_T("REMAIN BILL COUNT IS ZERO."));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bShowScreen = TRUE;
					continue;
				}

				// 요청 매수가 0일 경우 처리
				if(0 == nTotalRequestCount)
				{
					LOG (Info, _T("REQUEST BILL COUNT IS ZERO. (%d)"), nTotalRequestCount);

					Operator_Processing_Screen(_T("REQUEST BILL COUNT IS ZERO."));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bShowScreen = TRUE;
					continue;
				}

				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

				LOG (Info, _T("Execute Test Dispense - Start"));

				for(i=0; i<nCbxCount; i++)
				{
					nRemainCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);
					if(cstCountInfo.byReqCount[i] <= nRemainCount)
						cbxItem.count[i] = cstCountInfo.byReqCount[i];
					else
						cbxItem.count[i] = nRemainCount;
				}

				nResult = CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_TestDispense(&cbxItem, &dispResult);

				if(NO_ERROR != nResult)
				{
					strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError();
					LOG(Error, _T("CDM - CDM_TestDispense is failed. Error Code : ") + strErrorCode);

					Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
				}
				else
				{				
					Operator_Processing_Screen(_T("SUCCESS"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				memcpy(cstCountInfo.byRejectedCount, dispResult.count, MAX_CASSETTE);

				for(i=0; i<nCbxCount; i++)
				{
					if(dispResult.count[i] > 0)
						CEagleSVCLib::GetInstance()->UpdateRejectInfo(i+1, dispResult.count[i]);
				}
			
				// FW에서는 Dispense 명령을 받았을 때, LastDispense를 초기화 하기 때문에,
				// Dispense 명령의 CMD전송 실패시에는 LastDispense 명령을 수행하지 않는다.(이전 data가 올라올 수 있으므로)
				if(CDM_FAILED_TO_SEND_COMMAND != nResult)
					CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_LastDispense(&lastDispense);
	
				for(i=0; i<MAX_CASSETTE; i++)
				{
					LOG(Info, _T("CDM - TestDispense - CBX #%d => dispResult.count : [%d]"), i+1, dispResult.count[i]);

					cstCountInfo.byPickupCount[i] = lastDispense.last_pick_count[i];
					cstCountInfo.byRejectedCount[i] += lastDispense.last_divert_count[i];

					LOG(Info, _T("CDM - TestDispense - CBX #%d => last_pick_count : [%d], last_divert_count : [%d]"), i+1, lastDispense.last_pick_count[i], lastDispense.last_divert_count[i]);

					if(lastDispense.last_divert_count[i] > 0)
						bReject = TRUE;
				}

				if(TRUE == bReject)
					CEagleSVCLib::GetInstance()->UpdateRejectInfo();
				
				for(i=0; i<MAX_CFG_CBX_COUNT; i++)
				{

#if (SUPPORT_NOTE_COUNTING)
					nRemainCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]) - (int)cstCountInfo.byPickupCount[i];
					CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i] = CUtil::IntToString(nRemainCount);
#endif

					nRejectCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[i]) + (int)cstCountInfo.byRejectedCount[i];
					CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[i] = CUtil::IntToString(nRejectCount);

					nTotalRejectCount += (int)cstCountInfo.byRejectedCount[i];
				}
				nTempTotal = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strRBN_Item_Count) + nTotalRejectCount;
				CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strRBN_Item_Count = CUtil::IntToString(nTempTotal); 

				CEagleDataManager::GetInstance()->m_Config.SaveCbxInfo();

				// 결과를 Journal에 저장한다.(추가) (REQ / PICKUP / REJECT)
				//CString strEJL_Result;

				//strEJL_Result.Format(_T("TEST DISP.(%d,%d,%d,%d/%d,%d,%d,%d/%d,%d,%d,%d)"), cstCountInfo.byReqCount[0], cstCountInfo.byReqCount[1], cstCountInfo.byReqCount[2], cstCountInfo.byReqCount[3],
				//	lastDispense.last_pick_count[0], lastDispense.last_pick_count[1], lastDispense.last_pick_count[2], lastDispense.last_pick_count[3],
				//	cstCountInfo.byRejectedCount[0], cstCountInfo.byRejectedCount[1], cstCountInfo.byRejectedCount[2], cstCountInfo.byRejectedCount[3]);
				//CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strEJL_Result);
				
				// [V1.0.2.10] 2021.03.02 LEH - TestDispense 결과 저널저장 수정
				CEagleSVCLib::GetInstance()->SaveEJL_TestDispense(nResult, cstCountInfo.byReqCount, cstCountInfo.byPickupCount, cstCountInfo.byRejectedCount, strErrorCode);

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// CBX #1
					{
						strOutString.Empty();
						strCountInfo.Format(_T("%d"), cstCountInfo.byReqCount[0]);

						if (Operator_Input_Number(_T("CBX #1"), strCountInfo, strOutString, 1) == TRUE)
							cstCountInfo.byReqCount[0] = CUtil::StringToInt(strOutString);
						
						bShowScreen = TRUE;
					}
					break;

				case 2:	// CBX #2
					{
						strOutString.Empty();
						strCountInfo.Format(_T("%d"), cstCountInfo.byReqCount[1]);

						if (Operator_Input_Number(_T("CBX #2"), strCountInfo, strOutString, 1) == TRUE)
							cstCountInfo.byReqCount[1] = CUtil::StringToInt(strOutString);
						
						bShowScreen = TRUE;
					}
					break;

				case 3:	// CBX #3
					{
						strOutString.Empty();
						strCountInfo.Format(_T("%d"), cstCountInfo.byReqCount[2]);

						if (Operator_Input_Number(_T("CBX #3"), strCountInfo, strOutString, 1) == TRUE)
							cstCountInfo.byReqCount[2] = CUtil::StringToInt(strOutString);

						bShowScreen = TRUE;
					}
					break;

				case 4:	// CBX #4
					{
						strOutString.Empty();
						strCountInfo.Format(_T("%d"), cstCountInfo.byReqCount[3]);

						if (Operator_Input_Number(_T("CBX #4"), strCountInfo, strOutString, 1) == TRUE)
							cstCountInfo.byReqCount[3] = CUtil::StringToInt(strOutString);

						bShowScreen = TRUE;
					}
					break;

				default:
					break;
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
}

/** *********************************************************
*	@brief		CashDispenser - Cassette Setting
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_CashDispenser_CbxSetting()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;

	CString strScreenInform;
	CString strOutString;
	CString strDenom[MAX_CST_COUNT];
	CString strCbxValue, strPrevCBXStatus, strCurCBXStatus;


	int nCbxCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);
	int nMaxFocusIndex = nCbxCount;
	int i = 0;
	int nTempFocusIndex = 0;

	int nOldDenom[MAX_CST_COUNT], nNewDenom[MAX_CST_COUNT];

	// 카세트 상태 취득
	DWORD nCbxStatus[MAX_CASSETTE+1];
	memset(nCbxStatus, 0, sizeof(nCbxStatus));

	memset(nOldDenom, 0, sizeof(nOldDenom));
	memset(nNewDenom, 0, sizeof(nNewDenom));

	LOG(Info, _T("CDM - CbxSetting - CbxCount : [%d]"), nCbxCount);
	
	m_nCurrentFocusIndex = 1;

	for(i=0; i<nCbxCount; i++)
		strDenom[i].Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i]));

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		// CBX Status 추가 (0은 Reject Bin임)
		strCurCBXStatus.Empty();
		CDEV_Manager::GetInstance()->m_DEV_CDM.GetCbxStatus(nCbxStatus);

		for(i=1; i<MAX_CST_COUNT + 1; i++)
		{
			if (i > nCbxCount)
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

			for(i=0; i<MAX_CST_COUNT; i++)
			{
				// Value
				if(i+1 > nCbxCount)
					strCbxValue = _T("N/A");
				else
					strCbxValue.Format(_T("%s%s"), CURRENCY_SYMBOL, strDenom[i]);
				strScreenInform += strCbxValue;
				strScreenInform += SCR_CMD_DELIMITER;
			}

			// CBX Status 추가
			strScreenInform += strCurCBXStatus;
			
			Operator_DisplayScreen(OP_SCREEN_DEVICE_CASSETTESETTING, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Device_CashDispenser_CbxSetting - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_DEVICE_CDM;
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
			else if (m_strScrOutData == _T("SET"))
			{
				if (Operator_Confirm_Screen(_T("CASSETTE SETTINGS"), OP_GUIDE_COMMON_CONFIRM, _T("THE COUNT OF ALL CASSETTES WILL BE CLEARED.")) == TRUE)
				{
					Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

					BOOL bResult = FALSE;
					bResult = CEagleSVCLib::GetInstance()->SaveEJL_N_Print_CBXClose(EAGLE_TRAN_CODE_CBX_TOTALS);

					if ( bResult == TRUE)
					{
						LOG(Info, _T("CBX Total is cleared, print success"));
					}
					else
					{
						LOG(Info, _T("CBX Total is cleared, print failed"));
					}

					memset(nOldDenom, 0, sizeof(nOldDenom));
					memset(nNewDenom, 0, sizeof(nNewDenom));

					for(i=0; i<nCbxCount; i++)
					{
						nOldDenom[i] = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i]);
						nNewDenom[i] = CUtil::StringToInt(strDenom[i]);
						CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i] = strDenom[i];
					}
					CEagleDataManager::GetInstance()->m_Config.SaveCbxInfo();

					// [EJL]
					CEagleSVCLib::GetInstance()->SaveEJL_SetDenomination(nOldDenom, nNewDenom);

					if (CEagleSVCLib::GetInstance()->IsPTR_Available() == TRUE)
						bResult = CEagleSVCLib::GetInstance()->Print_SetDenomination(nOldDenom, nNewDenom);
					else
						bResult = FALSE;

					if ( bResult == TRUE)
					{
						LOG(Info, _T("SetDenomination print success"));
					}
					else
					{
						LOG(Info, _T("SetDenomination print failed"));
					}

					for (i=0; i<__min(nCbxCount, 4); i++)
					{
						LOG(Info, _T("CDM - CbxSetting - CBX #%d => Denomination : [%s]"), i+1, strDenom[i]);
					}

					if (bResult == TRUE)
						Operator_Processing_Screen(_T("SUCCESS"));
					else
						Operator_Processing_Screen(_T("SUCCESS(PRINT ERROR)"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}
				else
				{
					// "SET" 수행후, Confirm 화면에서 "NO"를 선택할 경우를 위해 원래의 권종 표시
					for(i=0; i<nCbxCount; i++)
						strDenom[i].Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i]));
				}

				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// CBX #1
					{
						strOutString.Empty();

						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 100);

						if (Operator_Input_Number(_T("CBX #1"), strDenom[0], strOutString, 3, DOLLAR_TYPE, strGuideText) == TRUE)
						{
							int nValue = CUtil::StringToInt(strOutString);

							if ( (1 == nValue) || (2 == nValue) || (5 == nValue) || (10 == nValue) || (20 == nValue) || (50 == nValue) || (100 == nValue) )
							{
								strDenom[0] = CUtil::IntToString(nValue);
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

				case 2:	// CBX #2
					{
						strOutString.Empty();

						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 100);

						if (Operator_Input_Number(_T("CBX #2"), strDenom[1], strOutString, 3, DOLLAR_TYPE, strGuideText) == TRUE)
						{
							int nValue = CUtil::StringToInt(strOutString);

							if ( (1 == nValue) || (2 == nValue) || (5 == nValue) || (10 == nValue) || (20 == nValue) || (50 == nValue) || (100 == nValue) )
							{
								strDenom[1] = CUtil::IntToString(nValue);
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

				case 3:	// CBX #3
					{
						strOutString.Empty();

						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 100);

						if (Operator_Input_Number(_T("CBX #3"), strDenom[2], strOutString, 3, DOLLAR_TYPE, strGuideText) == TRUE)
						{
							int nValue = CUtil::StringToInt(strOutString);

							if ( (1 == nValue) || (2 == nValue) || (5 == nValue) || (10 == nValue) || (20 == nValue) || (50 == nValue) || (100 == nValue) )
							{
								strDenom[2] = CUtil::IntToString(nValue);
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

				case 4:	// CBX #4
					{
						strOutString.Empty();

						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 100);

						if (Operator_Input_Number(_T("CBX #4"), strDenom[3], strOutString, 3, DOLLAR_TYPE, strGuideText) == TRUE)
						{
							int nValue = CUtil::StringToInt(strOutString);

							if ( (1 == nValue) || (2 == nValue) || (5 == nValue) || (10 == nValue) || (20 == nValue) || (50 == nValue) || (100 == nValue) )
							{
								strDenom[3] = CUtil::IntToString(nValue);
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

				default:
					break;
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
}

/** *********************************************************
*	@brief		CashDispenser - SetBill
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_CashDispenser_SetBill()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	BOOL bInitialize = TRUE;
	BOOL bInitResult = TRUE;
	BOOL bResult = FALSE;
	CString strScreenInform = _T("");
	CString strErrorCode = _T("");
	CString strOutString = _T("");
	CString strTemp = _T("");
	int nCbxNumber = 0;
	int nSize = 0;
	int nThickness = 0;
	int nMaxFocusIndex = 3;
	int i = 0;
	int nCbxCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

	CDM_BILL_INFO billInfo;
	CDM_BILL_INFO orgBillInfo;

	ZeroMemory(&billInfo, sizeof(CDM_BILL_INFO));
	m_strSave_OP_ScreenInfo.Empty();
	m_strSave_OP_ButtonInfo.Empty();

	LOG(Info, _T("CDM - SetBill - CbxCount : [%d]"), nCbxCount);

	m_nCurrentFocusIndex = 1;
	nCbxNumber = 1;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			// Set Bill 화면 진입 전, 지폐 정보 취득
			if(TRUE == bInitialize)
			{

				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

				ZeroMemory(&billInfo, sizeof(CDM_BILL_INFO));

				bInitResult = CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_GetBillInfo(nCbxCount, billInfo);
				nSize = (int)billInfo.bySize[nCbxNumber-1];
				nThickness = (int)billInfo.byThickness[nCbxNumber-1];
				bInitialize = FALSE;
			}

			if(FALSE == bInitResult)
			{
				strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError();
				LOG(Error, _T("CDM - CDM_GetBillInfo is failed. Error Code : ") + strErrorCode);

				Operator_Processing_Screen(_T(""), TRUE, strErrorCode);

				m_strNextScrNum = OP_SCREEN_DEVICE_CDM;
				break;
			}
			else
			{
				m_strScrOutName.Empty();
				m_strScrOutData.Empty();
				strScreenInform.Empty();

				// 1. CBX Number
				strScreenInform += CUtil::IntToString(nCbxNumber);
				strScreenInform += SCR_CMD_DELIMITER;

				// 2. Size for Set
				strScreenInform += CUtil::IntToString(nSize);
				strScreenInform += SCR_CMD_DELIMITER;

				// 3. Thickness for Set
				strScreenInform += CUtil::IntToString(nThickness);
				strScreenInform += SCR_CMD_DELIMITER;

				for(i=0; i<MAX_CST_COUNT; i++)
				{
					// Size
					if(i+1 > nCbxCount)
						strScreenInform += _T("N/A");
					else
						strScreenInform += CUtil::IntToString((int)billInfo.bySize[i]);
					strScreenInform += SCR_CMD_DELIMITER;
				}

				for(i=0; i<MAX_CST_COUNT; i++)
				{
					// Thickness
					if(i+1 > nCbxCount)
						strScreenInform += _T("N/A");
					else
						strScreenInform += CUtil::IntToString((int)billInfo.byThickness[i]);
					strScreenInform += SCR_CMD_DELIMITER;
				}

				Operator_DisplayScreen(OP_SCREEN_DEVICE_SETBILL, bKeyEnable, strScreenInform);
				bShowScreen = FALSE;
				bKeyEnable = FALSE;	// 최초 한번만 Key Enable 처리
			}
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Device_CashDispenser_SetBill - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_DEVICE_CDM;
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
			else if (m_strScrOutData == _T("STARTSET"))
			{
				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

				memcpy(&orgBillInfo, &billInfo, sizeof(CDM_BILL_INFO));

				billInfo.bySize[nCbxNumber-1] = (BYTE)nSize;
				billInfo.byThickness[nCbxNumber-1] = (BYTE)nThickness;

				bResult = CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_SetBillInfo(nCbxNumber-1, billInfo);
				if(FALSE == bResult)
				{
					strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError();
					LOG(Error, _T("CDM - CDM_SetBillInfo is failed. Error Code : ") + strErrorCode);

					Operator_Processing_Screen(_T(""), TRUE, strErrorCode);

					memcpy(&billInfo, &orgBillInfo, sizeof(CDM_BILL_INFO));
				}
				else
				{
					LOG(Info, _T("CDM - SetBill - CBX #%d => size : [%d], thickness : [%d]"), nCbxNumber, nSize, nThickness);

					Operator_Processing_Screen(_T("SUCCESS"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bInitialize = TRUE;
				}

				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// CBX Number
					{
						nCbxNumber++;

						if(nCbxNumber > nCbxCount)
							nCbxNumber = 1;

						nSize = (int)billInfo.bySize[nCbxNumber-1];
						nThickness = (int)billInfo.byThickness[nCbxNumber-1];

						bShowScreen = TRUE;
					}
					break;

				case 2:	// Size for Set
					{
						strOutString.Empty();
						strTemp = CUtil::IntToString(nSize);

						CString strGuideText;
						strGuideText.Format(_T("VALID RANGE : %d - %d"), BILL_INFO_MIN_SIZE, BILL_INFO_MAX_SIZE);

						if (Operator_Input_Number(_T("SIZE"), strTemp, strOutString, 2, NORMAL_TYPE, strGuideText) == TRUE)
						{
							int nValue = CUtil::StringToInt(strOutString);

							if( (nValue < BILL_INFO_MIN_SIZE) || (nValue > BILL_INFO_MAX_SIZE) )
							{
								// Invalid Notice
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
							else	// range : 40 ~ 80
							{
								nSize = nValue;
							}
						}

						bShowScreen = TRUE;
					}
					break;

				case 3:	// Thickness for Set
					{
						strOutString.Empty();
						strTemp = CUtil::IntToString(nThickness);

						CString strGuideText;
						strGuideText.Format(_T("VALID RANGE : %d - %d"), BILL_INFO_MIN_THICKNESS, BILL_INFO_MAX_THICKNESS);

						if (Operator_Input_Number(_T("THICKNESS"), strTemp, strOutString, 3, NORMAL_TYPE, strGuideText) == TRUE)
						{
							int nValue = CUtil::StringToInt(strOutString);

							if( (nValue < BILL_INFO_MIN_THICKNESS) || (nValue > BILL_INFO_MAX_THICKNESS) )
							{
								// Invalid Notice
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
							else	// range 30 ~ : 120
							{
								nThickness = nValue;
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
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}
}

/** *********************************************************
*	@brief		CashDispenser - LearnBill
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_CashDispenser_LearnBill()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	BOOL bInitialize = TRUE;
	BOOL bInitResult = TRUE;
	BOOL bResult = FALSE;
	CString strScreenInform = _T("");
	CString strErrorCode = _T("");
	CString strOutString = _T("");
	CString strTemp = _T("");
	int nCbxNumber = 0;
	int nLearnCount = 0;
	int nMaxFocusIndex = 2;
	int i = 0;
	int nCbxCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

	CDM_BILL_INFO billInfo;
	CDM_BILL_INFO orgBillInfo;
	CDMLEARN cdmLearn;

	ZeroMemory(&billInfo, sizeof(CDM_BILL_INFO));
	ZeroMemory(&cdmLearn, sizeof(CDMLEARN));
	m_strSave_OP_ScreenInfo.Empty();
	m_strSave_OP_ButtonInfo.Empty();

	LOG(Info, _T("CDM - LearnBill - CbxCount : [%d]"), nCbxCount);

	m_nCurrentFocusIndex = 1;
	nCbxNumber = 1;
	nLearnCount = 5;	// default : 5매

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			// Learn Bill 화면 진입 전, 지폐 정보 취득
			if(TRUE == bInitialize)
			{
				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

				ZeroMemory(&billInfo, sizeof(CDM_BILL_INFO));

				bInitResult = CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_GetBillInfo(nCbxCount, billInfo);
				bInitialize = FALSE;
			}

			if(FALSE == bInitResult)
			{
				strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError();
				LOG(Error, _T("CDM - CDM_GetBillInfo is failed. Error Code : ") + strErrorCode);

				Operator_Processing_Screen(_T(""), TRUE, strErrorCode);

				m_strNextScrNum = OP_SCREEN_DEVICE_CDM;
				break;
			}
			else
			{
				m_strScrOutName.Empty();
				m_strScrOutData.Empty();
				strScreenInform.Empty();

				// 1. CBX Number
				strScreenInform += CUtil::IntToString(nCbxNumber);
				strScreenInform += SCR_CMD_DELIMITER;

				// 2. Counter for learn
				strScreenInform += CUtil::IntToString(nLearnCount);
				strScreenInform += SCR_CMD_DELIMITER;

				for(i=0; i<MAX_CST_COUNT; i++)
				{
					// Size
					if(i+1 > nCbxCount)
						strScreenInform += _T("N/A");
					else
						strScreenInform += CUtil::IntToString((int)billInfo.bySize[i]);
					strScreenInform += SCR_CMD_DELIMITER;
				}

				for(i=0; i<MAX_CST_COUNT; i++)
				{
					// Thickness
					if(i+1 > nCbxCount)
						strScreenInform += _T("N/A");
					else
						strScreenInform += CUtil::IntToString((int)billInfo.byThickness[i]);
					strScreenInform += SCR_CMD_DELIMITER;
				}

				Operator_DisplayScreen(OP_SCREEN_DEVICE_LEARNING, bKeyEnable, strScreenInform);
				bShowScreen = FALSE;
				bKeyEnable = FALSE;	// 최초 한번만 Key Enable 처리
			}
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Device_CashDispenser_LearnBill - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_DEVICE_CDM;
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
			else if (m_strScrOutData == _T("STARTLEARN"))
			{
				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

				memcpy(&orgBillInfo, &billInfo, sizeof(CDM_BILL_INFO));

				bResult = CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_LearnBill(nCbxNumber-1, nLearnCount, &cdmLearn);
				if(FALSE == bResult)
				{
					strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError();
					LOG(Error, _T("CDM - CDM_LearnBill is failed. Error Code : ") + strErrorCode);

					Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
				}
				else
				{
					billInfo.bySize[nCbxNumber-1] = cdmLearn.size_avg;
					billInfo.byThickness[nCbxNumber-1] = cdmLearn.thickness_avg;
					
					bResult = CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_SetBillInfo(nCbxNumber-1, billInfo);
					if(FALSE == bResult)
					{
						strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError();
						LOG(Error, _T("CDM - CDM_SetBillInfo is failed. Error Code : ") + strErrorCode);

						Operator_Processing_Screen(_T(""), TRUE, strErrorCode);

						memcpy(&billInfo, &orgBillInfo, sizeof(CDM_BILL_INFO));
					}
					else
					{
						LOG(Info, _T("CDM - LearnBill - CBX #%d => size_avg : [%d], thickness_avg : [%d]"), nCbxNumber, (int)cdmLearn.size_avg, (int)cdmLearn.thickness_avg);

						Operator_Processing_Screen(_T("SUCCESS"));
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

						bInitialize = TRUE;
					}
				}

				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// CBX Number
					{
						nCbxNumber++;

						if(nCbxNumber > nCbxCount)
							nCbxNumber = 1;
					
						bShowScreen = TRUE;
					}
					break;

				case 2:	// Counter for learn
					{
						strOutString.Empty();
						strTemp = CUtil::IntToString(nLearnCount);
						if (Operator_Input_Number(_T("COUNTER"), strTemp, strOutString, 2) == TRUE)
							nLearnCount = CUtil::StringToInt(strOutString);

						bShowScreen = TRUE;
					}
					break;

				default:
					break;
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
}


/** *********************************************************
*	@brief		SIU
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_SIU()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform = _T("");
	CString strLamp[LAMP_COUNT];
	int i = 0;

	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_ALL, DIO_CMD_OFF);

	for(i=0; i<LAMP_COUNT; i++)
	{
		strLamp[i] = _T("OFF");
	}

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		strScreenInform.Empty();

		// 1. Printer on/off
		strScreenInform += strLamp[0];
		strScreenInform += SCR_CMD_DELIMITER;

		// 2. Card Reader on/off
		strScreenInform += strLamp[1];
		strScreenInform += SCR_CMD_DELIMITER;

		// 3. Dispenser on/off
		strScreenInform += strLamp[2];
		strScreenInform += SCR_CMD_DELIMITER;

		// 4. PINAD on/off
		strScreenInform += strLamp[3];
		strScreenInform += SCR_CMD_DELIMITER;

		// 5. TOP ROUND
		strScreenInform += strLamp[4];
		strScreenInform += SCR_CMD_DELIMITER;

		// 6. PINPAD ROUND
		strScreenInform += strLamp[5];
		strScreenInform += SCR_CMD_DELIMITER;

		// 7. BOTTOM ROUND
		strScreenInform += strLamp[5];
		strScreenInform += SCR_CMD_DELIMITER;

		// 8. Front Door open/close
		//strScreenInform += strSensor[0];
		if (Is_DIO_Front_Door_Open() == TRUE)
			strScreenInform += _T("OPEN");
		else
			strScreenInform += _T("CLOSE");
		strScreenInform += SCR_CMD_DELIMITER;

		// 9. Safe Door open/close
		//strScreenInform += strSensor[1];
		if (Is_DIO_Safe_Door_Open() == TRUE)
			strScreenInform += _T("OPEN");
		else
			strScreenInform += _T("CLOSE");
		strScreenInform += SCR_CMD_DELIMITER;

		// 10. Ear Jack detect/not detect
		//if (Is_DIO_Ear_Jack_Insert() == TRUE)
		if ( CDEV_Manager::GetInstance()->m_DEV_DIO.m_bAudioJackInserted == TRUE)	// OFFLINE에서도 동작하게 하기 위해 변수를 직접 참조하도록 변경
			strScreenInform += _T("DETECT");
		else
			strScreenInform += _T("NOT DETECT");
		strScreenInform += SCR_CMD_DELIMITER;

		if (bShowScreen == TRUE || (m_strSave_OP_ScreenInfo != strScreenInform))
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			Operator_DisplayScreen(OP_SCREEN_DEVICE_SIU, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Device_SIU - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_DEVICE;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("LAMPFLICK"))
			{
				CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_ALL, DIO_CMD_FLICKING);

				for(i=0; i<LAMP_COUNT; i++)
					strLamp[i] = _T("FLICKING");

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("LAMPON"))
			{
				CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_ALL, DIO_CMD_ON);

				for(i=0; i<LAMP_COUNT; i++)
						strLamp[i] = _T("ON");

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("LAMPOFF"))
			{
				CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_ALL, DIO_CMD_OFF);

				for(i=0; i<LAMP_COUNT; i++)
					strLamp[i] = _T("OFF");

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

	// EPP를 제외한 모든 LED OFF (EPP는 항상 ON 상태임)
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_ALL, DIO_CMD_OFF);
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed((DIO_4 | DIO_5 | DIO_6 | DIO_7), DIO_CMD_ON);

}


/** *********************************************************
*	@brief		PinPad
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_PINPAD()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;

	LOG(Info, _T("Proc_Operator_Device_PINPAD - Start"));

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			Operator_DisplayScreen(OP_SCREEN_DEVICE_PINPAD, bKeyEnable);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_DEVICE;
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

	LOG(Info, _T("Proc_Operator_Device_PINPAD - End"));
}

/** *********************************************************
*	@brief		Printer
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_Printer()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	BOOL bResult = TRUE;
	CString strScreenInform = _T("");
	CString strErrorCode = _T("");
	CString strButtonInfo;

	// Printer도 현재 상태가 장애인지 아닌지 표시하고 전환하도록 로직 처리

	if (m_bInitReset == TRUE)
	{
		m_bInitReset = FALSE;

		Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

		if (CEagleSVCLib::GetInstance()->IsPTR_Available() == FALSE)
		{
			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_PRT.GetLastError();
			LOG(Error, _T("PRT - PRT is error. Error Code : ") + strErrorCode);

			Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
		}
		else
		{
			// Printer 상태 정상
			LOG(Info, _T("Printer is Normal"));
		}
	}

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			//>>>
			// 추후에 기기 초기화 동작 추가 될 경우 Reset 로직 넣어야 함.
			//<<<

			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			Operator_DisplayScreen(OP_SCREEN_DEVICE_PRINTER, bKeyEnable);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Device_Printer - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_DEVICE;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("RESET"))
			{
				// LEFT 4
				Proc_Operator_Device_Printer_Reset();
				bShowScreen = TRUE;
			}
			else 
			if (m_strScrOutData == _T("TESTPRINT"))
			{
				// RIGHT 4
				Proc_Operator_Device_Printer_TestPrint();
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

/** *********************************************************
*	@brief		Printer - Reset
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_Printer_Reset()
{
	int nResult = 0;
	CString strErrorCode = _T("");

	LOG(Info, _T("Execute Proc_Operator_Device_Printer_Reset"));

	// Reset 후 Reset Print 인자
	Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

	// PTR Quiry Status Stop
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();

	nResult = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_Reset();

	if (nResult == EAGLE_PRT_SUCCESS)
	{
		// RESET 성공 후 Version Read하도록 추가
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetVersionInfo();

		// Reset 성공 후 PTR 인자가 가능한 경우에만 인자
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();

		if (CEagleSVCLib::GetInstance()->IsPTR_Available(TRUE) == TRUE)
		{
			// PTR Quiry Status Stop
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();

			int nOrgX = 40;

			if ( CEagleSVCLib::GetInstance()->Set_PTR_Config() == TRUE)
			{
				int i = 0;

				for(i=0; i<8; i++)
				{
					CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", 40, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
					CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
				}

				nResult = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)("           Reset print OK"), nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				for(i=0; i<9; i++)
				{
					CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", 40, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
					CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
				}

				nResult = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_CutPaper(EAGLE_PRT_TYPE_USB, 1, 0);

				// PTR Quiry Status Start
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				if(EAGLE_PRT_NORMAL == CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetStatus())
				{
					CEagleSVCLib::GetInstance()->Clear_DeviceError(DEVICE_PTR);

					// Operator Successful
					Operator_Processing_Screen(_T("SUCCESS"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					return;
				}
				else
				{
					LOG(Info, _T("PTR_CutPaper is failed"));
				}
			}
			else
			{
				LOG(Info, _T("PTR set config is failed"));
			}
		}
		else
		{
			// 정상 처리
			// PTR Quiry Status Start
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();

			CEagleSVCLib::GetInstance()->Clear_DeviceError(DEVICE_PTR);

			// Operator Successful
			Operator_Processing_Screen(_T("SUCCESS"));
			CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
			return;
		}
	}
	else
	{
		LOG(Info, _T("PTR reset is failed"));
	}

	// PTR Quiry Status Start
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();	// Polling Start 로직 누락 추가

	strErrorCode = CDEV_Manager::GetInstance()->m_DEV_PRT.GetLastError();
	LOG(Error, _T("PTR - Reset print is failed. Error Code : ") + strErrorCode);

	//CEagleSVCLib::GetInstance()->Set_DeviceErrorCode(DEVICE_PTR, strErrorCode);

	Operator_Processing_Screen(_T(""), TRUE, strErrorCode);

}


/** *********************************************************
*	@brief		Printer - TestPrint
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_Printer_TestPrint()
{
	BOOL bResult = FALSE;
	CString strErrorCode = _T("");
	CStringA	szTemp = "";
	int		nPrintError = ERROR_SUCCESS;

	LOG(Info, _T("Execute Proc_Operator_Device_Printer_TestPrint"));

	int nResult = 0;

	Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

	if (CEagleSVCLib::GetInstance()->IsPTR_Available() == TRUE)
	{
		// PTR Quiry Status Stop
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();

		int i = 0;
		int nOrgX = 40;

		for(i=0; i<3; i++)		// 앞에 3 Line 처리
		{
			szTemp.Format("");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		}

//#if(ENABLE_RECEIPT_HEADER_IMAGE_PRINT)
		szTemp.Format("============================================");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

		// Print Logo Image
		nPrintError = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_PrintBitmapFlash(EAGLE_PRT_TYPE_USB, 1, nOrgX, EAGLE_PRT_BITMAP_PRINT_NORMAL);

		if (EAGLE_PRT_SUCCESS != nPrintError)
		{
			LOG(Info, _T("PRT - PTR_S_PrintBitmapFlash Failed - %d"), nPrintError);
		}

		szTemp.Format("============================================");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

		szTemp.Format("");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
//#else
//		for(i=0; i<8; i++)
//		{
//			szTemp.Format("");
//			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
//			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
//		}
//#endif

		szTemp.Format("============================================");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		
		szTemp.Empty();
		for(i=0; i<0x20; i++)
		{
			szTemp += (char)(0x20 + i);
		}

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

		szTemp.Empty();
		for(i=0x20; i<0x40; i++)
		{
			szTemp += (char)(0x20 + i);
		}

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

		szTemp.Empty();
		for(i=0x40; i<0x5F; i++)
		{
			szTemp += (char)(0x20 + i);
		}

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

		// 한번 더 Print
		szTemp.Empty();
		for(i=0; i<0x20; i++)
		{
			szTemp += (char)(0x20 + i);
		}

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

		szTemp.Empty();
		for(i=0x20; i<0x40; i++)
		{
			szTemp += (char)(0x20 + i);
		}

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

		szTemp.Empty();
		for(i=0x40; i<0x5F; i++)
		{
			szTemp += (char)(0x20 + i);
		}

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

		szTemp.Format("============================================");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

		for(i=0; i<3; i++)	// 마지막 3 Line 처리
		{
			szTemp.Format("");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_3000MS);

		if(CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetStatus() != EAGLE_PRT_NORMAL)
		{
			LOG(Error, _T("Test Print is failed"));

			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_PRT.GetLastError();
			LOG(Error, _T("PTR - PTR_TestPrint is failed. Error Code : ") + strErrorCode);

			Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
		}
		else
		{
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_CutPaper(EAGLE_PRT_TYPE_USB, 1, 0);

			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_3000MS);

			if(EAGLE_PRT_NORMAL == CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetStatus())
			{
				CEagleSVCLib::GetInstance()->Clear_DeviceError(DEVICE_PTR);

				// Operator Successful
				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
			}
			else
			{
				LOG(Error, _T("PTR_CutPaper is failed"));

				strErrorCode = CDEV_Manager::GetInstance()->m_DEV_PRT.GetLastError();
				LOG(Error, _T("PTR - PTR_TestPrint is failed. Error Code : ") + strErrorCode);

				Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
			}
		}

		// PTR Quiry Status Start
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();
	}
	else
	{
		strErrorCode = CDEV_Manager::GetInstance()->m_DEV_PRT.GetLastError();
		LOG(Error, _T("PTR - Test Print print is failed. Error Code : ") + strErrorCode);

		//CEagleSVCLib::GetInstance()->Set_DeviceErrorCode(DEVICE_PTR, strErrorCode);

		Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
	}
}

/** *********************************************************
*	@brief		Device - TCP/IP
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_TCPIP()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform = _T("");
	CString strErrorCode = _T("");
	CString strOutString = _T("");
	CString strHostIP = _T("");
	CString strPortNum = _T("");
	CString strUseTLS = _T("");
	CString strType = _T("");
	int nMaxFocusIndex = 4;
	int nResult = 0;

	CString strPingResultHeader = _T("");
	CString strPingResultContent = _T("");
	CString strPingResultContent2 = _T("");
	CString strPingResultTotal = _T("");
	CString strIP4Ping;
	int nRtt = 0;
	UCHAR cTtl = 0;
	ICMP_ECHO_REPLY icmpEchoReply;
	BOOL bPingTest = FALSE;

	m_nCurrentFocusIndex = 1;

	strType = _T("0");	// IP TYPE
	strHostIP = CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip;
	strPortNum = CEagleDataManager::GetInstance()->m_Config.m_Host.strPort;
	strUseTLS = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			if (strType == _T("0"))
				strScreenInform += _T("IP");
			else
				strScreenInform += _T("URL");
			strScreenInform += SCR_CMD_DELIMITER;

			// 2. HostIP
			strScreenInform += strHostIP;
			strScreenInform += SCR_CMD_DELIMITER;

			// 3. Port Number
			strScreenInform += strPortNum;
			strScreenInform += SCR_CMD_DELIMITER;

			// 4. TLS 유무
			strScreenInform += GET_ENDISABLE_STRING(strUseTLS);
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_DEVICE_TCPIP, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Device_TCPIP - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_DEVICE;
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
			else if (m_strScrOutData == _T("TESTPING"))
			{
				strOutString.Empty();
				strPingResultHeader.Empty();
				strPingResultContent.Empty();
				strPingResultContent2.Empty();
				strPingResultTotal.Empty();
				nRtt = 0;
				bPingTest = FALSE;

				memset(&icmpEchoReply, 0x00, sizeof(ICMP_ECHO_REPLY));

				if (strType == _T("0"))
				{
					if (Operator_Input_Number(_T("IP"), strHostIP, strOutString, 15) == TRUE)
					{
						strOutString.TrimLeft();
						strOutString.TrimRight();
						strIP4Ping = strOutString;
						bPingTest = TRUE;
					}
				}
				else
				{
					if (Operator_Input_Text(_T("ADDRESS"), strHostIP, strOutString) == TRUE)
					{
						strOutString.TrimLeft();
						strOutString.TrimRight();
						strIP4Ping = strOutString;
						bPingTest = TRUE;
					}
				}

				// URL로 PingTest도 가능하도록 지원
			//	if (Operator_Input_Number(_T("IP"), strHostIP, strOutString, 15) == TRUE)
				if(TRUE == bPingTest)
				{
					//strOutString.TrimLeft();
					//strOutString.TrimRight();
					//strIP4Ping = strOutString;

					// initial message
					strPingResultHeader.Format(_T("PING %s ..."), strIP4Ping);
					CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_500MS);

					for(int i=0; i<PING_TEST_MAX_COUNT; i++)
					{
						nResult = CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Sync_PingTest(strIP4Ping, icmpEchoReply);
						LOG(Info, _T("Result : [%d], icmpEchoReply.Status : [%d]"), nResult, icmpEchoReply.Status);

						if(SUCCESS == nResult)
						{
							nRtt = icmpEchoReply.RoundTripTime;
							cTtl = icmpEchoReply.Options.Ttl;

							if(0 == nRtt)
								strPingResultContent.Format(_T("REPLY FROM %s: TIME<1ms TTL=%d\n"), strIP4Ping, cTtl);
							else
								strPingResultContent.Format(_T("REPLY FROM %s: TIME=%dms TTL=%d\n"), strIP4Ping, nRtt, cTtl);
						}
						else
						{
							strPingResultContent.Format(_T("TRANSMIT FAILED.\n"));
						}

						strPingResultContent2 += strPingResultContent;

						strPingResultTotal.Format(_T("%s\n%s"), strPingResultHeader, strPingResultContent2);

						Operator_Processing_Screen(strPingResultTotal, FALSE, _T(""), FALSE);

						CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_500MS);
					}

					Operator_Processing_Screen(_T(""), TRUE, strPingResultTotal, FALSE);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("TESTCONNECT"))
			{
				// CHECK INPUT DATA
				if ((strHostIP.IsEmpty() == TRUE) || (CUtil::StringToInt(strPortNum) <= 0))
				{
					Operator_Processing_Screen(_T("INPUT ADDRESS AND PORT NUMBER"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}
				else
				{
					Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

					nResult = CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Sync_ConnectTest(strHostIP, strPortNum, strUseTLS);

					if(0 != nResult)
					{
						strErrorCode = ERROR_NET_CONNECT_FAIL;
						LOG(Error, _T("TCP/IP Connect is failed (%s)"), strErrorCode);

						Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
					}
					else
					{
						Operator_Processing_Screen(_T("SUCCESS"));
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					}
				}

				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:
					{
						if (strType == _T("0"))
							strType = _T("1");
						else
							strType = _T("0");

						bShowScreen = TRUE;
					}
					break;

				case 2:	// HostIP
					{
						if (strType == _T("0"))
						{
							if (Operator_Input_Number(_T("IP"), strHostIP, m_strScrOutData, 15) == TRUE)
							{
								m_strScrOutData.TrimLeft();
								m_strScrOutData.TrimRight();
								strHostIP = m_strScrOutData;
							}
						}
						else
						{
							if (Operator_Input_Text(_T("ADDRESS"), strHostIP, m_strScrOutData) == TRUE)
							{
								m_strScrOutData.TrimLeft();
								m_strScrOutData.TrimRight();
								strHostIP = m_strScrOutData;
							}
						}

						bShowScreen = TRUE;
					}
					break;

				case 3:	// Port Number
					{
						CString strGuideText, strTemp;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 65535);

						if (CUtil::StringToInt(strPortNum) <= 0)
							strTemp.Empty();
						else
							strTemp = strPortNum;

						if (Operator_Input_Number(_T("PORT"), strTemp, m_strScrOutData, 5, NORMAL_TYPE, strGuideText) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();
							int nPort = CUtil::StringToInt(m_strScrOutData);

							if (nPort < 0 || nPort > 65535)	// 0도 입력이 가능하도록 로직 수정
							{
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
							else
							{
								strPortNum = m_strScrOutData;
							}
						}

						bShowScreen = TRUE;
					}
					break;

				case 4:	// Use TLS
					{
						if (strUseTLS == S_ENABLE)
							strUseTLS = S_DISABLE;
						else
							strUseTLS = S_ENABLE;
					
						bShowScreen = TRUE;
					}
					break;

				default:
					break;
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
}

/** *********************************************************
*	@brief		Device - Modem
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Device_Modem()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform = _T("");
	CString strErrorCode = _T("");
	CString strOutString = _T("");
	CString strPhoneNum = _T("");
	int nResult = 0;

	strPhoneNum = CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number;
	strPhoneNum.TrimLeft();
	strPhoneNum.TrimRight();

	m_nCurrentFocusIndex = 1;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// 1. Phone Number
			strScreenInform += strPhoneNum;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_DEVICE_MODEM, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Device_Modem - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_DEVICE;
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
			if (m_strScrOutData == _T("TESTCONNECT"))
			{
				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

				nResult = CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Sync_ConnectTest(strPhoneNum, _T("0"), _T("0"));
				if(0 != nResult)
				{
					strErrorCode = ERROR_NET_CONNECT_FAIL;
					LOG(Error, _T("Modem Connect is failed (%s)"), strErrorCode);

					Operator_Processing_Screen(_T(""), TRUE, strErrorCode);
				}
				else
				{
					Operator_Processing_Screen(_T("SUCCESS"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				strOutString.Empty();
				if (Operator_Input_Number(_T("PHONE NUMBER"), strPhoneNum, strOutString) == TRUE)
				{
					strOutString.TrimLeft();
					strOutString.TrimRight();
					strPhoneNum = strOutString;
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
