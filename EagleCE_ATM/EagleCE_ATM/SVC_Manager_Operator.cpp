#include "stdafx.h"
#include "SVC_Manager.h"

#include "EagleSVCLib.h"

//#include "../../EagleCE_Device/EagleCE_Device/DEV_Manager.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/** **********************************************************
*	@brief		서비스 실행 - 계원용 모드
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::RunService_Operator(BYTE byCmd)
{
	LOG(Info, _T("RunService_Operator - Start"));

	// 계원용 커맨드 - Supervisor main
	if (EAGLE_CMD_OP_INITIALIZE == byCmd)
	{
		LOG(Info, _T("RunService_Operator - OP INITIALIZE CMD"));

		// 위치 변경 (Supervisor로 전환시에 Processing 화면 및 AMS Close하도록 로직 수정
		Client_Screen_Processing();

		// AMS ConnectionClose
		this->PostMessage(WM_AMS_NOTIFY, EAGLE_ATM_DEV_AMS_CLOSE_START);

		// 최초 Supervisor 진입시 Screen Number를 OP Main으로 설정
		m_strNextScrNum = OP_SCREEN_MAIN;

		// Supervisor 진입 전 Configure 정보 Save
		CDEV_Manager::GetInstance()->Save_PreviousTerminalInfo();

		while(TRUE)
		{
			// 변수 초기화 공용화
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			m_nCurrentFocusIndex = 0;

			LOG(Info, _T("Supervisor Screen Information : %s"), m_strNextScrNum);

			if (m_strNextScrNum == OP_SCREEN_MAIN)
			{
				//<		MAIN
				Proc_Operator_Main();
			}
			else if (m_strNextScrNum == OP_SCREEN_EXIT)
			{
				Proc_Operator_Exit();
			}
			else if (m_strNextScrNum == OP_SCREEN_SYSTEM)
			{
				///<	MAIN -> SYSTEM
				Proc_Operator_System_Main();
			}
			else if (m_strNextScrNum == OP_SCREEN_DATE_N_TIME)
			{
				//<		MAIN->SYSTEM->DATE/TIME
				Proc_Operator_DateTime();
			}
			else if (m_strNextScrNum == OP_SCREEN_PASSWORD)
			{
				//<		MAIN->SYSTEM->PASSWORD
				Proc_Operator_Password();
			}
			// V1.0.2.4 2018.07.02 - Reboot 화면 추가
			else if (m_strNextScrNum == OP_SCREEN_REBOOT)
			{
				//<		MAIN->SYSTEM->REBOOT
				Proc_Operator_Reboot();
			}
			else if (m_strNextScrNum == OP_SCREEN_UPLOAD)
			{
				//<		MAIN->SYSTEM->UPLOAD
				Proc_Operator_Upload();
			}
			else if (m_strNextScrNum == OP_SCREEN_BACKUP)
			{
				//<		MAIN->SYSTEM->BACKUP
				Proc_Operator_BackUp();
			}
			else if (m_strNextScrNum == OP_SCREEN_BACKUP_CAPTUREIMAGE)
			{
				//<		MAIN->SYSTEM->BACKUP->BACKUP DATE
				Proc_Operator_BackUpCaptureImage();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE)
			{
				//<		MAIN->DEVICE
				Proc_Operator_Device_Main();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_CDR)
			{
				//<		MAIN->DEVICE->CDR
				Proc_Operator_Device_CardReader();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_CDM)
			{
				//<		MAIN->DEVICE->CASH DISPENSER
				Proc_Operator_Device_CashDispenser();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_TESTDISPENSE)
			{
				//<		MAIN->DEVICE->CASH DIPENSER->TEST DISPENSE
				Proc_Operator_Device_CashDispenser_TestDispense();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_CASSETTESETTING)
			{
				//<		MAIN->DEVICE->CASH DIPENSER->CASSETTE SETTINGS
				Proc_Operator_Device_CashDispenser_CbxSetting();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_SETBILL)
			{
				//<		MAIN->DEVICE->CASH DIPENSER->SET BILL PARAMETERS
				Proc_Operator_Device_CashDispenser_SetBill();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_LEARNING)
			{
				//<		MAIN->DEVICE->CASH DIPENSER->LEARN BILL PARAMETERS
				Proc_Operator_Device_CashDispenser_LearnBill();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_PRINTER)
			{
				//<		MAIN->DEVICE->PRINTER
				Proc_Operator_Device_Printer();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_PINPAD)
			{
				//<		MAIN->DEVICE->PINPAD
				Proc_Operator_Device_PINPAD();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_SIU)
			{
				//<		MAIN->DEVICE->SIU
				Proc_Operator_Device_SIU();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_SOUND)
			{
				//<		MAIN->DEVICE->SOUND
				Proc_Operator_Device_Sound();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_TCPIP)
			{
				//<		MAIN->DEVICE->TCP/IP
				Proc_Operator_Device_TCPIP();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_MODEM)
			{
				//<		Proc_Operator_Device_Modem
				Proc_Operator_Device_Modem();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_CAMERA)
			{
				//<		MAIN->DEVICE->CAMERA
				Proc_Operator_Device_Camera();
			}
			else if (m_strNextScrNum == OP_SCREEN_DEVICE_CAMERA_CAPTURE)
			{
				//<		MAIN->DEVICE->CAMERA
				Proc_Operator_Device_Camera_Capture();
			}
			else if (m_strNextScrNum == OP_SCREEN_JOURNAL)
			{
				//<		MAIN->JOURNAL
				Proc_Operator_Journal_Main();
			}
			else if (m_strNextScrNum == OP_SCREEN_VIEW_JOURNAL)
			{
				//<		MAIN->JOURNAL->VIEW JOURNAL
				Proc_Operator_View_Journal();
			}
			else if (m_strNextScrNum == OP_SCREEN_VIEW_JOURNAL_DETAIL)
			{
				//<		MAIN->JOURNAL->VIEW JOURNAL->DETAIL
				Proc_Operator_View_Journal_Detail();
			}
			else if (m_strNextScrNum == OP_SCREEN_PRINT_LASTX_JOURNAL)
			{
				//<		MAIN->JOURNAL->PRINT LASTX
				Proc_Operator_Journal_PrintLastX();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION)
			{
				//<		MAIN->OPTION
				Proc_Operator_Option_Main();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_TRANSACTION )
			{
				//<		MAIN->OPTION->TRANSACTION
				Proc_Operator_Option_Transaction();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_AMOUNT_SETTINGS)
			{
				//<		MAIN->OPTION->TRANSACTION->AMOUNTSETTINGS
				Proc_Operator_Option_AmountSettings();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_CONFIGURE_GENERAL)
			{
				//<		MAIN->OPTION->TRANSACTION->CINFIGUREGENERAL
				Proc_Operator_Option_ConfigureGeneral();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_BLOCK_ISO)
			{
				//<		MAIN->OPTION->TRANSACTION->BLOCKISO
				Proc_Operator_Option_BlockISO();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_AD_SCREEN)
			{
				//<		MAIN->OPTION->AD SCREEN
				Proc_Operator_Option_AD_Screen();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_AD_1_PREVIEW)
			{
				//<		MAIN->OPTION->AD SCREEN->AD1 PREVIEW
				Proc_Operator_Option_AD1_Preview();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_AD_2_PREVIEW)
			{
				//<		MAIN->OPTION->AD SCREEN->AD2 PREVIEW
				Proc_Operator_Option_AD2_Preview();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_AD_3_PREVIEW)
			{
				//<		MAIN->OPTION->AD SCREEN->AD3 PREVIEW
				Proc_Operator_Option_AD3_Preview();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_AD_4_PREVIEW)
			{
				//<		MAIN->OPTION->AD SCREEN->AD4 PREVIEW
				Proc_Operator_Option_AD4_Preview();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_AD_5_PREVIEW)
			{
				//<		MAIN->OPTION->AD SCREEN->AD5 PREVIEW
				Proc_Operator_Option_AD5_Preview();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_AD_6_PREVIEW)
			{
				//<		MAIN->OPTION->AD SCREEN->AD6 PREVIEW
				Proc_Operator_Option_AD6_Preview();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_MESSAGE)
			{
				//<		MAIN->OPTION->MESSAGE
				Proc_Operator_Option_Message();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_MARKETING)
			{
				//<		MAIN->OPTION->MESSAGE->MARKETING
				Proc_Operator_Option_MarketingMsg();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_LOCATION)
			{
				//<		MAIN->OPTION->MESSAGE->LOCATION
				Proc_Operator_Option_LocationMsg();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_ATTRACT)
			{
				//<		MAIN->OPTION->MESSAGE->ATTRACT
				Proc_Operator_Option_AttractMsg();
			}
			else if (m_strNextScrNum == OP_SCREEN_OPTION_FAREWELL)
			{
				//<		MAIN->OPTION->MESSAGE->FAREWELL
				Proc_Operator_Option_FarewellMsg();
			}
			else if (m_strNextScrNum == OP_SCREEN_REPORT)
			{
				//<		MAIN->REPORT
				Proc_Operator_Report_Main();
			}
			else if (m_strNextScrNum == OP_SCREEN_ERRORCODE)
			{
				//<		MAIN->REPORT->ERROR CODE
				Proc_Operator_ErrorCode();
			}
			else if (m_strNextScrNum == OP_SCREEN_TASK)
			{
				//<		MAIN->TASK
				Proc_Operator_Task_Main();
			}
			else if (m_strNextScrNum == OP_SCREEN_ADD_QTY_IN_CASSETTE)
			{
				//<		MAIN->TASK->ADD QTY IN CASSETTE
				Proc_Operator_AddQTYinCST();
			}
			else if (m_strNextScrNum == OP_SCREEN_SCHEDULE_DAY_CLOSE)
			{
				//<		MAIN->TASK->SCHEDULE DAY CLOSE
				Proc_Operator_ScheduleDayClose();
			}
			else if (m_strNextScrNum == OP_SCREEN_HOST)
			{
				//<		MAIN->HOST
				Proc_Operator_Host_Main();
			}
			else if (m_strNextScrNum == OP_SCREEN_TERMINAL_NETWORK_SETTINGS)
			{
				//<		MAIN->HOST->TERMINAL NETWORK SETTINGS
				Proc_Operator_NetworkSettings();			
			}
			else if (m_strNextScrNum == OP_SCREEN_TERMINAL_MODEM_SETTINGS)
			{
				//<		MAIN->HOST->TERMINAL NETWORK SETTINGS -> MODEM SETTINGS
				Proc_Operator_ModemSettings();
			}
			else if (m_strNextScrNum == OP_SCREEN_TERMINAL_IP_SETTINGS)
			{
				//<		MAIN->HOST->TERMINAL NETWORK SETTINGS -> IP SETTINGS
				Proc_Operator_IPSettings();
			}
			else if (m_strNextScrNum == OP_SCREEN_KEY_MANAGEMENT)
			{
				//<		MAIN->HOST->KEY MANAGEMENT
				Proc_Operator_KeyManagement();
			}
			else if (m_strNextScrNum == OP_SCREEN_MASTERKEYS_MANAGEMENT)
			{
				//<		MAIN->HOST->KEY MANAGEMENT->MASTER KEYS MANAGEMENT
				Proc_Operator_MasterKeysManagement();
			}
			else if (m_strNextScrNum == OP_SCREEN_PINPAD_ACTIVATION)
			{
				//<		MAIN->HOST->KEY MANAGEMENT->MASTER KEYS MANAGEMENT->ACTIVATION
				Proc_Operator_PNPAD_Activation();
			}
			else if (m_strNextScrNum == OP_SCREEN_INJECT_MASTER_KEY)
			{
				//<		MAIN->HOST->KEY MANAGEMENT->MASTER KEYS MANAGEMENT->INJECT MASTER KEY
				Proc_Operator_InjectMasterKey();
			}
			else if (m_strNextScrNum == OP_SCREEN_MASTER_KEY_KCV_INFO)
			{
				//<		MAIN->HOST->KEY MANAGEMENT->MASTER KEYS MANAGEMENT->KCV INFO
				Proc_Operator_MasterKey_KCV_Info();
			}
			else if (m_strNextScrNum == OP_SCREEN_CONFIGURE_CONNECTION)
			{
				//<		MAIN->HOST->CONFIGURE CONNECTION
				Proc_Operator_ConfigureConnection();
			}
			else if (m_strNextScrNum == OP_SCREEN_PROTOCOL_OPTION)
			{
				//<		MAIN->HOST->CONFIGURE CONNECTION->PROTOCOLOPTION
				Proc_Operator_ProtocolOption();
			}
			else if (m_strNextScrNum == OP_SCREEN_HOST_PHONENUMBER)
			{
				//<		MAIN->HOST->CONFIGURE CONNECTION->HOSTPHONENUMBER
				Proc_Operator_HostPhoneNumber();
			}
			else if (m_strNextScrNum == OP_SCREEN_HOST_IP_ADDRESS)
			{
				//<		MAIN->HOST->CONFIGURE CONNECTION->HOSTPHONENUMBER
				Proc_Operator_HostIPAddress();
			}
			else if (m_strNextScrNum == OP_SCREEN_TERMINAL_MONITORING)
			{
				//<		MAIN->HOST->TERMINAL MONITORING
				Proc_Operator_TerminalMonitoring();
			}
			else if (m_strNextScrNum == OP_SCREEN_EMV_AID_LIST)
			{
				//<		MAIN->HOST->AID LIST
				Proc_Operator_EMV_AIDList();
			}
			else if (m_strNextScrNum == OP_SCREEN_AMS_CONFIGURE_TCPIP)
			{
				//<		MAIN->AMS
				Proc_Operator_ConfigureAMS_TCPIP();
			}
			else if (m_strNextScrNum == OP_SCREEN_AMS_CONFIGURE_MODEM)
			{
				//<		MAIN->AMS
				Proc_Operator_ConfigureAMS_Modem();
			}
			else
			{
				// next screen이 정의되어져 있지 않으면 임시로 OP Main으로 변경
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
		}

		// Supervisor가 종료될 때 Configure 정보 변경시 저널 저장
		CDEV_Manager::GetInstance()->SaveEJL_SetChangedTerminalInfo();
	}

	LOG(Info, _T("RunService_Operator - End"));

	return TRUE;
}


/** **********************************************************
*	@brief		계원용 커맨드 - Operator Initialize
*	@retval		없음
************************************************************/
void CSVC_Manager::Operator_Initialize(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_OP_INITIALIZE);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		계원용 커맨드 - 종료
*	@retval		없음
************************************************************/
void CSVC_Manager::Operator_Exit(void)
{
	LOG(Info, _T("Exit Operator Mode"));

	// 서비스 모드
	m_nServiceMode = 0;

	// 계원용 모드 초기화
	m_nOperatorMode = 0;
}


/*------------------------------------------------------------*/
// Process Function
/*------------------------------------------------------------*/

static CString strMonthName[12] = { _T("JAN"), _T("FEB"), _T("MAR"), _T("APR"), _T("MAY"), _T("JUN"),
								    _T("JUL"), _T("AUG"), _T("SEP"), _T("OCT"), _T("NOV"), _T("DEC") };
/** **********************************************************
*	@brief		계원용 커맨드 - OP Main
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Main(void)
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInfo;
	CString strTemp, strTemp2, strErrorDevice, strSaveErrorCode, strSaveErrorDesc;

	SYSTEMTIME	st_current = { 0 };
	
	ERRCODEINFO	ErrorcodeInfo;
	memset(&ErrorcodeInfo, 0, sizeof(ErrorcodeInfo));

	LOG(Info, _T("Operator Main - Start"));

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		CEagleSVCLib::GetInstance()->IsTerminal_Error();	// Setting Error를 자동으로 Clear하기 위함

		// Data가 변경되는 경우 화면을 다시 표시하기 위함.
		strScreenInform.Empty();
		strButtonInfo.Empty();

		// Set Information Data
		// 1. Terminal ID
		strScreenInform += CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID;
		strScreenInform += SCR_CMD_DELIMITER;

		// 2. Serial No
		strScreenInform += CEagleDataManager::GetInstance()->m_Config.m_Option.strMachine_Serial_Number;
		strScreenInform += SCR_CMD_DELIMITER;

		// 3. S/W Version	
#if (NETWORK_OFFLINE_MODE)
		strTemp.Format(_T("D%s"), CDEV_Manager::GetInstance()->m_strAP_Version);
#else
		strTemp.Format(_T("V%s"), CDEV_Manager::GetInstance()->m_strAP_Version);
#endif

		strScreenInform += strTemp;
		strScreenInform += SCR_CMD_DELIMITER;

		// 4. Communication
#if (NETWORK_OFFLINE_MODE)
		strScreenInform += _T("OFFLINE");
#else
		strScreenInform += CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType;
#endif
		strScreenInform += SCR_CMD_DELIMITER;

		// ATM Status
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
		{
			strTemp2.Empty();

			strErrorDevice = CEagleSVCLib::GetInstance()->Get_ErrorCode();

			if (strErrorDevice.Left(1) == _T("Z"))
			{
				// AP 장애는 상세 표시 그 외의 장애는 미 표시
				if (strSaveErrorCode != strErrorDevice)
				{
					strSaveErrorCode = strErrorDevice;

					// Search Error Description
					memset(&ErrorcodeInfo, 0, sizeof(ErrorcodeInfo));
					if (CDEV_Manager::GetInstance()->m_DEV_EJL.GetErrorCodeInfo(CEagleSVCLib::GetInstance()->Get_ErrorCode(), &ErrorcodeInfo) == TRUE)
					{
						strSaveErrorDesc = (CString)ErrorcodeInfo.err_desc;
					}
				}
			}
			else if (strErrorDevice.Left(1) == _T("C"))
			{
				strSaveErrorDesc = _T("CARD READER ERROR");
			}
			else if (strErrorDevice.Left(1) == _T("D"))
			{
				strSaveErrorDesc = _T("CASH DISPENSER ERROR");
			}
			else if (strErrorDevice.Left(1) == _T("P"))
			{
				strSaveErrorDesc = _T("RECEIPT PRINTER ERROR");
			}
			else if (strErrorDevice.Left(1) == _T("E"))
			{
				strSaveErrorDesc = _T("PINPAD ERROR");
			}
			else if (strErrorDevice.Left(1) == _T("J"))
			{
				strSaveErrorDesc = _T("ELECTRONIC JOURNAL ERROR");
			}
			else
			{
				strSaveErrorDesc.Empty();
			}

			if (strSaveErrorDesc.IsEmpty() == FALSE)
			{
				strTemp.Format(_T("ERROR = %s\n----------------------------------------\n%s"), CEagleSVCLib::GetInstance()->Get_ErrorCode(), strSaveErrorDesc);
			}
			else
			{
				strTemp.Format(_T("ERROR = %s"), strErrorDevice);
			}
		}
		else
		{
			strTemp = _T("NORMAL");
		}

		strScreenInform += strTemp;
		strScreenInform += SCR_CMD_DELIMITER;

		// Check System Time
		GetLocalTime(&st_current);

		strTemp.Format(_T("%s/%02d/%04d %02d:%02d"), strMonthName[st_current.wMonth - 1], st_current.wDay, st_current.wYear, st_current.wHour, st_current.wMinute);
		strScreenInform += strTemp;
		strScreenInform += SCR_CMD_DELIMITER;

		if (bShowScreen == TRUE)
		{
			// Set Button Info
			// 1. DEVICE / 2. SYSTEM / 3. JOURNAL / 4. TASK / 5. HOST / 6. AMS / 7. OPTION / 8. REPORT
			if (GetOperatorMode() == EAGLE_ATM_SVC_REPLENISH)
			{
				strButtonInfo += _T("on");	// V1.0.2.3 - Reset 버튼 활성화 하기 위함.
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("off");
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

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;

				strButtonInfo += _T("on");
				strButtonInfo += SCR_CMD_DELIMITER;
			}

			Operator_DisplayScreen(OP_SCREEN_MAIN, bKeyEnable, strScreenInform, strButtonInfo);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}
		else if (m_strSave_OP_ScreenInfo != strScreenInform)
		{
			// Device 상태 변경시 UI 갱신 
			Operator_UpdateScreen(strScreenInform);
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("User Select - [%s] [%s]"), m_strScrOutName, m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER || m_strScrOutData == S_EXIT)
			{
				if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
				{
					if (Operator_Confirm_Screen(_T("CONFIRMATION"), _T("DO YOU WANT TO EXIT SUPERVISOR SCREEN?"), _T("CURRENTLY ATM IS ERROR CONDITION"), TRUE, TRUE) == TRUE)
					{
						m_strNextScrNum = OP_SCREEN_EXIT;
						break;						
					}
				}
				else
				{
					// 정상적인 상황에서 CST가 하나라도 missing인 경우에는 warning 문구 표시
					BOOL	bCBXMissing = FALSE;
					DWORD	dwCBXStatus[MAX_CASSETTE + 1];		// Reject Bin / CST1 ~ 4

					memset(&dwCBXStatus, 0, sizeof(dwCBXStatus));
					strTemp.Empty();

					CDEV_Manager::GetInstance()->m_DEV_CDM.GetCbxStatus(dwCBXStatus);

					for(int i=0; i<CDEV_Manager::GetInstance()->m_DEV_CDM.m_nCbxCount; i++)
					{
						if (dwCBXStatus[i+1] == WFS_CDM_STATCUMISSING)
						{
							bCBXMissing = TRUE;
							strTemp.Format(_T("CBX #%d IS MISSING"), i+1);
							break;
						}
					}

					if (bCBXMissing == TRUE)
					{
						if (Operator_Confirm_Screen(_T("CONFIRM"), _T("DO YOU WANT TO EXIT SUPERVISOR SCREEN?"), strTemp, TRUE, TRUE) == TRUE)
						{
							m_strNextScrNum = OP_SCREEN_EXIT;
							break;
						}
					}
					else
					{
						// 정상인 경우는 Supervisor Exit 처리
						m_strNextScrNum = OP_SCREEN_EXIT;
						break;
					}

				}

				bShowScreen = TRUE;
			}

			// Next Action
			if (m_strScrOutData == _T("DEVICE"))
			{
				// LEFT 1
				m_strNextScrNum = OP_SCREEN_DEVICE;
				break;

			}
			else if (m_strScrOutData == _T("SYSTEM"))
			{
				// RIGHT 1
				m_strNextScrNum = OP_SCREEN_SYSTEM;
				break;

			}
			else if (m_strScrOutData == _T("JOURNAL"))
			{
				// LEFT 2
				m_strNextScrNum = OP_SCREEN_JOURNAL;
				break;
			}
			else if (m_strScrOutData == _T("TASK"))
			{
				// RIGHT 2
				m_strNextScrNum = OP_SCREEN_TASK;
				break;
			}
			else if (m_strScrOutData == _T("HOST"))
			{
				// LEFT 3
				m_strNextScrNum = OP_SCREEN_HOST;
				break;
			}
			else if (m_strScrOutData == _T("AMS"))
			{
				if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
					m_strNextScrNum = OP_SCREEN_AMS_CONFIGURE_TCPIP;
				else
					m_strNextScrNum = OP_SCREEN_AMS_CONFIGURE_MODEM;
				break;
			}
			else if (m_strScrOutData == _T("OPTION"))
			{
				// LEFT 4
			    m_strNextScrNum = OP_SCREEN_OPTION;
				break;
			}

			else if (m_strScrOutData == _T("REPORT"))
			{
				// RIGHT 4
				m_strNextScrNum = OP_SCREEN_REPORT;
				break;
			}
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);			// System Holding 관련 AP Version을 Memory에 Loading 후 사용시 해당 현상 해결됨.
	}

	LOG(Info, _T("Operator Main - End"));
}


/** **********************************************************
*	@brief		계원용 커맨드 - OP Exit
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Exit(void)
{
	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	LOG(Info, _T("Exit Supervisor Mode - Start"));

	// 화면은 거래쪽 Initialize Device쪽 화면으로 대체
	Client_Screen_Initialize();

	// CDM Initialize를 하지 않는 대신에 CBX 상태만 강제 Clear (이전 사양 유지하기 위해 - Supervisor Exit시 CBX 상태 Clear)
	//CDEV_Manager::GetInstance()->m_DEV_CDM.Clear_CBXStatus();

	// Network Error가 Clear되지 않는 현상이 발생할 수도 있으므로 Network 오류코드인 경우는 Error Clear하도록 보완 처리
	if (CEagleSVCLib::GetInstance()->Get_ErrorCode().Left(2) == _T("Z3"))
		CEagleSVCLib::GetInstance()->Clear_Error();

	// Error Device가 있는 경우 Device Reset 재 수행 (Error가 없는 경우는 CDU Reset 수행 여부 확인 필요)
	int nErrorDevice = CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice;

	if (nErrorDevice != 0)
	{
		// Error Device시에 CDU는 무조건 수행 -> 장애가 있는 Device만 초기화 하도록 변경 (정전기 최소화)
		nErrorDevice |= ERROR_DEVICE_CDM;

		if (CEagleSVCLib::GetInstance()->Recovery_ErrorDevice(nErrorDevice, TRUE) == TRUE)
		{
			LOG(Info, _T("Operator Exit - Recovery is succeed"));
		}
		else
		{
			LOG(Info, _T("Operator Exit - Recovery is failed"));
		}
	}
	else
	{
		// 정상이라 판단

		// CDU Initialize 수행 -> 정상인 경우 WDM 초기화 하지 않도록 수정
		CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStop();	// Polling Stop

		if(FALSE == CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_Initialize(TRUE))
		{
			CEagleSVCLib::GetInstance()->Set_ErrorCode(CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError(), ERROR_DEVICE_CDM);
			LOG(Error, _T("Operator exit Reset - CDM - Failed - Reset (%s)"), CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError());
		}

		CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStart();	// Polling Start
	}

	LOG(Info, _T("Exit Supervisor Mode - End"));

	// initialize Operator variable
	m_strCurrentScrNum.Empty();
	m_strNextScrNum.Empty();

	Operator_Exit();
}


/** *************************************************************
*	@brief	Text 입력 화면
*	@param	CString strTitle	입력화면의 Title로 사용할 문자열
*	@param	CString strInText	수정 전의 문자열
*	@param	CString &strOutText	수정 완료된 문자열
*	@param	CString strInputDefaultMode	입력화면에서 처음 사용될 입력모드(default mode)
*	@param	int nMaxInNumber	Max Input Length(default : 40)
*	@retval TRUE Save 버튼 또는 KeyPad의 Enter Key를 누름
*	@retval FALSE KeyPad의 Cancel Key를 누름
*****************************************************************/
BOOL CSVC_Manager::Operator_Input_Text(CString strTitle, CString strInText, CString &strOutText, int nMaxInNumber/*=40*/, CString strInputDefaultMode/* =_T("NUMBER") */)
{
	CString strCommand = _T("");
	CString strScrOutName = _T("");
	CString strScrOutData = _T("");
	int		nRespDataCount = 0;
	CStringArray strTempArray;
	CString strGuide = _T("");
	CString strMaxInChar = _T("");

	// Key Enable
	CrypteraEPP_EnterClearTextMode();

	// PrevSet
	CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), OP_SCREEN_INPUT_TEXT);

	// Title
	strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
	CSCR_Manager::GetInstance()->SetVariable(strCommand, strTitle);

	// Value Guide
	if(nMaxInNumber <= 40)
	{
		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 2);
		strGuide.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_LEN, nMaxInNumber);
		CSCR_Manager::GetInstance()->SetVariable(strCommand, strGuide);
	}

	// Set MaxInChar
	strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 3);
	strMaxInChar.Format(_T("%s%d"), _T("MaxInChar:"), nMaxInNumber);
	CSCR_Manager::GetInstance()->SetVariable(strCommand, strMaxInChar);

	// TableEditBox
	CSCR_Manager::GetInstance()->SetVariable(strCommand, strInText);

	// SetInputMode
	CSCR_Manager::GetInstance()->SetVariable(_T("APSetInputMode"), strInputDefaultMode);

	// Virtual KeyPad Button 
	CSCR_Manager::GetInstance()->SetVariable(_T("APSetFocus"), strInputDefaultMode);

	// Set Timeout
	CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
	CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(SUPERVISOR_SCREEN_TIMEOUT));

	// Display Screen
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), OP_SCREEN_INPUT_TEXT);


	///////////////////////////////////
	// GET KEY STRING
	while (TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(strScrOutName, strScrOutData);
			break;
		}

		// 대기
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	nRespDataCount = CUtil::ParsingStringToStringArray(strScrOutData, (CString)SCR_RES_DELIMITER, strTempArray);

	if (nRespDataCount >= 2)
	{
		if(strTempArray[0] == S_SAVE)
		{
			strOutText = strTempArray[1];
			strOutText.TrimLeft();
			strOutText.TrimRight();
			return TRUE;
		}
		else if (strTempArray[0] == S_TIMEOVER)
		{
			strOutText = S_TIMEOVER;
			return FALSE;
		}
	}
	else
	{
		strOutText = strInText;
		return FALSE;
	}

	return FALSE;
}

/** *************************************************************
*	@brief	Number 입력 화면
*	@param	CString strTitleText	입력화면의 Title로 사용할 문자열
*	@param	CString strInText		수정 전의 문자열
*	@param	CString &strOutText		수정 완료된 문자열
*	@retval TRUE					Save 버튼 또는 KeyPad의 Enter Key를 누름
*	@retval FALSE					KeyPad의 Cancel Key를 누름
*****************************************************************/
BOOL CSVC_Manager::Operator_Input_Number(CString strTitleText, CString strInText, CString &strOutText, int nMaxInNumber /*=20*/, CString strInputType /*=NORMAL_TYPE*/, CString strGuideText /**=_T("")*/)
{
	CString strCommand = _T("");
	CString strScrOutName = _T("");
	CString strScrOutData = _T("");
	CString strMaxInChar = _T("");
	CString strGuide = _T("");
	int		nResultCount = 0;
	CStringArray strTempArray;

	// Key Enable
	CrypteraEPP_EnterClearTextMode();

	// PrevSet
	CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), OP_SCREEN_INPUT_NUMBER);

	// Title
	strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
	CSCR_Manager::GetInstance()->SetVariable(strCommand, strTitleText);

	// Value Guide
	if(nMaxInNumber <= 20)
	{
		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 2);

		// 고객에게 좀 더 정확한 Guide가 필요한 경우 사용하도록 기능 개선
		if (strGuideText.IsEmpty())
			strGuide.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_LEN, nMaxInNumber);
		else
			strGuide = strGuideText;

		CSCR_Manager::GetInstance()->SetVariable(strCommand, strGuide);
	}

	// Current Value
	strCommand.Format(_T("%s%d"),  SCR_TEXTVALUE_CMD, 3);
	CSCR_Manager::GetInstance()->SetVariable(strCommand, strInText);

	// Set MaxInChar
	strMaxInChar.Format(_T("%s%d"), _T("MaxInChar:"), nMaxInNumber);
	CSCR_Manager::GetInstance()->SetVariable(strCommand, strMaxInChar);

	// Set Timeout
	CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
	CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(SUPERVISOR_SCREEN_TIMEOUT));

	// Set InputType
	CSCR_Manager::GetInstance()->SetVariable(_T("APSetInputType"), strInputType);

	// Display Screen
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), OP_SCREEN_INPUT_NUMBER);


	///////////////////////////////////
	// GET KEY STRING
	while (TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(strScrOutName, strScrOutData);
			break;
		}

		// 대기
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	nResultCount = CUtil::ParsingStringToStringArray(strScrOutData, (CString)SCR_RES_DELIMITER, strTempArray);

	if (nResultCount >= 2)
	{
		if(strTempArray[0] == S_SAVE)
		{
			strOutText = strTempArray[1];
			return TRUE;
		}
		else
		{
			strOutText = strInText;
			return FALSE;
		}
	}

	return FALSE;
}


/** *************************************************************
*	@brief	HexaDecimal 입력 화면
*	@param	CString strTitle	입력화면의 Title로 사용할 문자열
*	@param	CString strInText	수정 전의 문자열
*	@param	CString &strOutText	수정 완료된 문자열
*	@param	int nMaxInNumber	Max Input Length(default : 40)
*	@retval TRUE Save 버튼 또는 KeyPad의 Enter Key를 누름
*	@retval FALSE KeyPad의 Cancel Key를 누름
*****************************************************************/
BOOL CSVC_Manager::Operator_Input_HexaDecimal(CString strTitle, CString strInText, CString &strOutText, int nMaxInNumber/*=40*/)
{
	CString strCommand = _T("");
	CString strScrOutName = _T("");
	CString strScrOutData = _T("");
	int		nRespDataCount = 0;
	CStringArray strTempArray;
	CString strGuide = _T("");
	CString strMaxInChar = _T("");

	// Key Enable
	CrypteraEPP_EnterClearTextMode();

	// PrevSet
	CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), OP_SCREEN_INPUT_HEX);

	// Title
	strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
	CSCR_Manager::GetInstance()->SetVariable(strCommand, strTitle);

	// Value Guide
	if(nMaxInNumber <= 40)
	{
		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 2);
		strGuide.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_LEN, nMaxInNumber);
		CSCR_Manager::GetInstance()->SetVariable(strCommand, strGuide);
	}

	// Set MaxInChar
	strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 3);
	strMaxInChar.Format(_T("%s%d"), _T("MaxInChar:"), nMaxInNumber);
	CSCR_Manager::GetInstance()->SetVariable(strCommand, strMaxInChar);

	// TableEditBox
	CSCR_Manager::GetInstance()->SetVariable(strCommand, strInText);

	// SetInputMode
	CSCR_Manager::GetInstance()->SetVariable(_T("APSetInputMode"), HEXADECIMAL_MODE);

	// Set Timeout
	CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
	CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(SUPERVISOR_SCREEN_TIMEOUT));

	// Display Screen
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), OP_SCREEN_INPUT_HEX);


	///////////////////////////////////
	// GET KEY STRING
	while (TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(strScrOutName, strScrOutData);
			break;
		}

		// 대기
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	nRespDataCount = CUtil::ParsingStringToStringArray(strScrOutData, (CString)SCR_RES_DELIMITER, strTempArray);

	if (nRespDataCount >= 2)
	{
		if(strTempArray[0] == S_SAVE)
		{
			strOutText = strTempArray[1];
			strOutText.TrimLeft();
			strOutText.TrimRight();
			return TRUE;
		}
		else if (strTempArray[0] == S_TIMEOVER)
		{
			strOutText = S_TIMEOVER;
			return FALSE;
		}
	}
	else
	{
		strOutText = strInText;
		return FALSE;
	}

	return FALSE;
}


/** *************************************************************
*	@brief	Confirm 화면
*	@param	CString strTitleID	입력화면의 Title로 사용할 문자열
*	@param	CString strGuideText			Guide 문구
*	@param	CString strImportantGuideText	중요한 Guide 문구
*	@retval TRUE				Yes 선택 시
*	@retval FALSE				No 또는 Cancel 선택시
*****************************************************************/
BOOL CSVC_Manager::Operator_Confirm_Screen(CString strTitleText, CString strGuideText, CString strImportantGuideText/* = _T("")*/, BOOL bUseSound/* = FALSE*/, BOOL bNormalForTimeout/* = FALSE*/)
{
	CString strTitle, strCommand, strTemp;
	CString strScrOutName;
	CString strScrOutData;
	int		nSoundInterval = 0;

	// Key Enable
	CrypteraEPP_EnterClearTextMode();

	// PrevSet
	CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), OP_SCREEN_CONFIRM);

	// Title
	strTitle = strTitleText;
	strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
	CSCR_Manager::GetInstance()->SetVariable(strCommand, strTitle);

	// Important Guide Text
	if (!strImportantGuideText.IsEmpty())
	{
		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 2);
		CSCR_Manager::GetInstance()->SetVariable(strCommand, strImportantGuideText);
	}

	// Guide Text
	strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 3);	// Guide 문구가 없을 경우에는 "Do you want to Continue?" 문구 표시
	if (strGuideText.IsEmpty())
		CSCR_Manager::GetInstance()->SetVariable(strCommand, CSCR_Manager::GetInstance()->GetAPTextIDString(OP_GUIDE_COMMON_CONFIRM));
	else
		CSCR_Manager::GetInstance()->SetVariable(strCommand, strGuideText);

	// Set Timeout
	CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
	CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(SUPERVISOR_SCREEN_TIMEOUT));

	// Display Screen
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), OP_SCREEN_CONFIRM);


	///////////////////////////////////
	// GET KEY STRING
	while (TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(strScrOutName, strScrOutData);
			break;
		}

		// 대기
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);

		if (bUseSound == TRUE)
		{
			nSoundInterval++;

			if (nSoundInterval > 5)	// 500ms마다 sound play
			{
				PlaySound(CUtil::GetKeyPressWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);
				nSoundInterval = 0;
			}
		}
	}

	if(strScrOutData == S_YES)
	{
		return TRUE;
	}

	if (bNormalForTimeout == TRUE)
	{
		// Timeout 발생시 정상 처리 로직 추가
		if(strScrOutData == S_TIMEOVER)
		{
			return TRUE;
		}
	}

	return FALSE;
}


/** *************************************************************
*	@brief	Processing 화면
*	@param	CString strProcessingText	동작중인지, 동작완료인지에 사용할 문자열
*	@param	BOOL bResultScreen			동작중인지, 동작완료인지 여부
*	@param	CString strErrorCode		동작완료 이후 장애코드
*	@param	BOOL bResultDisplay			"RESULT" 문구 display여부와 Blink 여부
*	@retval TRUE	동작 결과 화면에서 Enter Key 또는 Timeout
*	@retval FALSE	동작중 화면
*****************************************************************/
BOOL CSVC_Manager::Operator_Processing_Screen(CString strProcessingText, BOOL bResultScreen/*=FALSE*/, CString strErrorCode/*=_T("000000")*/, BOOL bResultDisplay/*=TRUE*/)
{
	CString strCommand = _T("");
	CString strResultCode = _T("");

	// Key Disable
	//CrypteraEPP_StopClearTextMode();

	// PrevSet
	CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), OP_SCREEN_PROGRESS);

	if(TRUE == bResultScreen)
	{
		m_strScrOutName.Empty();
		m_strScrOutData.Empty();

		// Key Enable
		CrypteraEPP_EnterClearTextMode();

		// Set Timeout
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(SUPERVISOR_INFO_SCREEN_TIMEOUT));

		// Result
		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
		if(TRUE == bResultDisplay)
			strResultCode.Format(_T("%s%s"), _T("RESULT : "), strErrorCode);
		else
			strResultCode.Format(_T("%s"), strErrorCode);
		CSCR_Manager::GetInstance()->SetVariable(strCommand, strResultCode);

		// Press Enter Key
		strCommand.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, 1);
		CSCR_Manager::GetInstance()->SetVariable(strCommand, _T("on"));

		// Set Blink
		CSCR_Manager::GetInstance()->SetVariable(_T("APSetBlinkMode"), _T("0"));
	}
	else
	{
		// Message
		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
		CSCR_Manager::GetInstance()->SetVariable(strCommand, strProcessingText);

		// Press Enter Key
		strCommand.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, 1);
		CSCR_Manager::GetInstance()->SetVariable(strCommand, _T("off"));

		// Set Blink
		if(TRUE == bResultDisplay)
			CSCR_Manager::GetInstance()->SetVariable(_T("APSetBlinkMode"), _T("1"));
		else
			CSCR_Manager::GetInstance()->SetVariable(_T("APSetBlinkMode"), _T("0"));
	}

	// Display Screen
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), OP_SCREEN_PROGRESS);

	if(TRUE == bResultScreen)
	{
		///////////////////////////////////
		// GET KEY STRING
		while (TRUE)
		{
			CheckService();

			if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
			{
				CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

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


/** *************************************************************
*	@brief	OP 화면 표시
*	@param	CString	strScrNum 표시할 화면 번호
*	@param	CString	strScreenInfo 표시할 화면 정보
*	@param	CString	strButtonInfo 표시할 버튼 정보
*	@retval 없음
*****************************************************************/
void CSVC_Manager::Operator_DisplayScreen(CString strScrNum, BOOL bKeyEnable, CString strScreenInfo, CString strButtonInfo, CString strEditStateInfo, BOOL FocusKeyActive)
{
	CString strCmd, strValue;
	CString strTemp;

	if (bKeyEnable == TRUE)
		CrypteraEPP_EnterClearTextMode();

	// 1. Prepare screen
	CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), strScrNum);

	// 2. Get Screen Information
	if (strScreenInfo.IsEmpty() == FALSE)
	{
		strCmd.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
		strTemp = strScreenInfo;
		CSCR_Manager::GetInstance()->SetVariable(strCmd, strTemp);		
	}

	// 3. Get Button Information
	if (strButtonInfo.IsEmpty() == FALSE)
	{
		strCmd.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, 1);
		strTemp = strButtonInfo;
		CSCR_Manager::GetInstance()->SetVariable(strCmd, strTemp);
	}

	// 4. Get Edit Control Information
	if (strEditStateInfo.IsEmpty() == FALSE)
	{
		strCmd.Format(_T("%s%d"), SCR_EDIT_CTRL_CMD, 1);
		strTemp = strEditStateInfo;
		CSCR_Manager::GetInstance()->SetVariable(strCmd, strTemp);
	}

	// 5. Check Focus
	if (m_nCurrentFocusIndex > 0)
	{
		strCmd = SCR_SET_FOCUS_VALUE;
		strValue.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, m_nCurrentFocusIndex);
		CSCR_Manager::GetInstance()->SetVariable(strCmd, strValue);

		if (FocusKeyActive == FALSE)
			CSCR_Manager::GetInstance()->SetVariable(_T("APKeyDisable"), strValue);
	}

	// 임시적으로 Active Key 화면에서는 10분 대기 처리
	if (strScrNum == OP_SCREEN_PINPAD_ACTIVATION)
	{
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(SUPERVISOR_PINPAD_ACTIVE_TIMEOUT));
	}
	else
	{
		CSCR_Manager::GetInstance()->SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		CSCR_Manager::GetInstance()->SetVariable(_T("APMaxTime"), CUtil::IntToString(SUPERVISOR_SCREEN_TIMEOUT));
	}

	// 3.Display Screen
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), strScrNum);

	// Save Screen Information
	m_strCurrentScrNum = strScrNum;
	m_strSave_OP_ScreenInfo = strScreenInfo;
	m_strSave_OP_ButtonInfo = strButtonInfo;
}


/** *************************************************************
*	@brief	OP 화면 표시
*	@param	CString	strScrNum 표시할 화면 번호
*	@param	CString	strScreenInfo 표시할 화면 정보
*	@param	CString	strButtonInfo 표시할 버튼 정보
*	@retval 없음
*****************************************************************/
void CSVC_Manager::Operator_UpdateScreen(CString strScreenInfo, CString strButtonInfo, BOOL FocusKeyActive)
{
	CString strCmd, strValue;

	// 2. Get Screen Information
	{
		if (strScreenInfo.IsEmpty() == FALSE)
		{
			if (m_strSave_OP_ScreenInfo != strScreenInfo)
			{
				strCmd.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
				CSCR_Manager::GetInstance()->SetVariable(strCmd, strScreenInfo);
			}
		}

		if (strButtonInfo.IsEmpty() == FALSE)
		{
			if (m_strSave_OP_ButtonInfo != strButtonInfo)
			{
				strCmd.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, 1);
				CSCR_Manager::GetInstance()->SetVariable(strCmd, strButtonInfo);
			}
		}

		// 3. Check Focus
		if (m_nCurrentFocusIndex > 0)
		{
			strCmd = SCR_SET_FOCUS_VALUE;
			strValue.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, m_nCurrentFocusIndex);
			CSCR_Manager::GetInstance()->SetVariable(strCmd, strValue);

			if (FocusKeyActive == FALSE)
				CSCR_Manager::GetInstance()->SetVariable(_T("APKeyDisable"), strValue);
		}

	}

	// Screen Update
	CSCR_Manager::GetInstance()->SetVariable(_T("APUpdate"), _T("SCREEN"));

	// Save Screen Information
	m_strSave_OP_ScreenInfo = strScreenInfo;
	m_strSave_OP_ButtonInfo = strButtonInfo;
}