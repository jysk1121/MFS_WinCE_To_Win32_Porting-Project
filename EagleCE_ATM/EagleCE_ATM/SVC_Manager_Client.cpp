#include "stdafx.h"
#include "SVC_Manager.h"

#include "EagleTritonMsg.h"
#include "EagleSVCLib.h"
#include "EagleCE_ATMDlg.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/IniFile.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"
//#include "../../Template/TimeCheck.h"
#include "TimeCheck.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CTimeCheck	g_TimeOutCheck;

/** **********************************************************
*	@brief		서비스 실행 - 고객용 모드
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::RunService_Client(BYTE byCmd)
{
	// 고객용 커맨드 - 고객 변경
	if (EAGLE_CMD_CL_CHANGE_CUSTOM == byCmd)
	{
		LOG(Info, _T("RunService - Client - ChangeCustom - Start"));

		if (FALSE == Proc_Client_ChangeCustom())
		{
			LOG(Error, _T("RunService - Client - ChangeCustom - False"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - ChangeCustom - End"));
	}

	// 고객용 커맨드 - 카드 투입
	else if (EAGLE_CMD_CL_INSERT_CARD == byCmd)
	{
		LOG(Info, _T("RunService - Client - Insert Card - Start"));

		Proc_Client_InsertCard();

		// Advertisement DeActivation
		CSCR_Manager::GetInstance()->SetAdvertisement(FALSE);

		LOG(Info, _T("RunService - Client - Insert Card - End"));
	}

	else if (EAGLE_CMD_CL_EMV_CARD_READ == byCmd)
	{
		LOG(Info, _T("RunService - Client - EMV Card Read - Start"));

		Proc_Client_EMV_CardRead();

		LOG(Info, _T("RunService - Client - EMV Card Read - End"));
	}

	else if (EAGLE_CMD_CL_EMV_MAKEAPPLISTS == byCmd)
	{
		LOG(Info, _T("RunService - Client - Proc_Client_EMV_MakeAIDList - Start"));

		Proc_Client_EMV_MakeAIDList();

		LOG(Info, _T("RunService - Client - Proc_Client_EMV_MakeAIDList - End"));
	}

	else if (EAGLE_CMD_CL_EMV_FINALSELECT == byCmd)
	{
		LOG(Info, _T("RunService - Client - Proc_Client_EMV_FromFinalAppToReadApp - Start"));

		Proc_Client_EMV_FromFinalAppToReadApp();

		LOG(Info, _T("RunService - Client - Proc_Client_EMV_FromFinalAppToReadApp - End"));
	}

	else if (EAGLE_CMD_CL_EMV_MULTIAPPSELECT == byCmd)
	{
		LOG(Info, _T("RunService - Client - Proc_Client_EMV_MultiAppSelect - Start"));

		Proc_Client_EMV_MultiAppSelect();

		LOG(Info, _T("RunService - Client - Proc_Client_EMV_MultiAppSelect - End"));
	}

	else if (EAGLE_CMD_CL_EMV_FALLBACK == byCmd)
	{
		LOG(Info, _T("RunService - Client - EMV FallBack - Start"));

		Proc_Client_EMV_FallBack();

		LOG(Info, _T("RunService - Client - EMV FallBack - End"));
	}

	// 고객용 커맨드 - 카드 제거
	else if (EAGLE_CMD_CL_REMOVE_CARD == byCmd)
	{
		LOG(Info, _T("RunService - Client - Remove Card - Start"));

		if (FALSE == Proc_Client_RemoveCard())
		{
			LOG(Error, _T("RunService - Client - Remove Card - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Remove Card - End"));
	}

	// 고객용 커맨드 - 카드 읽기
	else if (EAGLE_CMD_CL_READ_CARD == byCmd)
	{
		LOG(Info, _T("RunService - Client - Read Card - Start"));

		if (FALSE == Proc_Client_ReadCard())
		{
			//CEagleDataManager::GetInstance()->m_UnitOperation_Info.m_nError_CDR++;

			LOG(Error, _T("RunService - Client - Read Card - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Read Card - End"));
	}

	else if (EAGLE_CMD_CL_SELECT_LANGUAGE == byCmd)
	{
		LOG(Info, _T("RunService - Client - Select Language - Start"));

		if (FALSE == Proc_Client_SelectLanguage())
		{
			LOG(Error, _T("RunService - Client - Select Language - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Select Language - End"));
	}

	// 고객용 커맨드 - 핀 읽기
	else if (EAGLE_CMD_CL_READ_PIN == byCmd)
	{
		LOG(Info, _T("RunService - Client - Read Pin - Start"));

		if (FALSE == Proc_Client_ReadPin())
		{
			LOG(Error, _T("RunService - Client - Read Pin - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Read Pin - End"));
	}

	// 고객용 커맨드 - 거래선택
	else if (EAGLE_CMD_CL_SELECT_TRANSACTION == byCmd)
	{
		LOG(Info, _T("RunService - Client - Select Transaction - Start"));

		if (FALSE == Proc_Client_SelectTransaction())
		{
			LOG(Error, _T("RunService - Client - Select Transaction - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Select Transaction - End"));
	}

	// 고객용 커맨드 - 계좌 선택
	else if (EAGLE_CMD_CL_SELECT_ACCOUNT == byCmd)
	{
		LOG(Info, _T("RunService - Client - Select Accounts - Start"));

		if (FALSE == Proc_Client_SelectAccounts())
		{
			LOG(Error, _T("RunService - Client - Select Accounts - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Select Accounts - End"));
	}

	// 고객용 커맨드 - 이체계좌 선택
	else if (EAGLE_CMD_CL_SELECT_TRANSFERACCOUNT == byCmd)
	{
		LOG(Info, _T("RunService - Client - Select Transfer Accounts - Start"));

		if (FALSE == Proc_Client_SelectTransferAccounts())
		{
			LOG(Error, _T("RunService - Client - Select Transfer Accounts - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Select Transfer Accounts - End"));
	}

	// 고객용 커맨드 - 금액 선택
	else if (EAGLE_CMD_CL_SELECT_AMOUNT == byCmd)
	{
		LOG(Info, _T("RunService - Client - Select Amount - Start"));

		if (FALSE == Proc_Client_SelectAmount())
		{
			LOG(Error, _T("RunService - Client - Select Amount - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Select Amount - End"));
	}

	else if (EAGLE_CMD_CL_ADA_CONFIRMAMOUNT == byCmd)
	{
		LOG(Info, _T("RunService - Client - Proc_Client_ADA_ConfirmAmount - Start"));

		if (FALSE == Proc_Client_ADA_ConfirmAmount())
		{
			LOG(Error, _T("RunService - Client - Proc_Client_ADA_ConfirmAmount - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Proc_Client_ADA_ConfirmAmount - End"));
	}

	// 고객용 커맨드 - 금액 입력
	else if (EAGLE_CMD_CL_ENTER_WITHDRAWALAMOUNT == byCmd)
	{
		LOG(Info, _T("RunService - Client - Enter Other Amount - Start"));

		if (FALSE == Proc_Client_EnterAmount())
		{
			LOG(Error, _T("RunService - Client - Select Amount - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Enter Other Amount - End"));
	}

	// 고객용 커맨드 - 수수료 확인
	else if(EAGLE_CMD_CL_CONFIRM_SURCHARGE == byCmd)
	{
		LOG(Info, _T("RunService - Client - Confirm Surcharge - Start"));

		if (FALSE == Proc_Client_ConfirmSurcharge())
		{
			LOG(Error, _T("RunService - Client - Confirm Surcharge - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Confirm Surcharge - End"));
	}

	// 고객용 커맨드 - 이체 금액 입력
	else if (EAGLE_CMD_CL_ENTER_TRANSFERAMOUNT == byCmd)
	{
		LOG(Info, _T("RunService - Client - Enter transfer amount - Start"));

		if (FALSE == Proc_Client_EnterTransferAmount())
		{
			LOG(Error, _T("RunService - Client - Enter transfer amount - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Enter transfer amount - End"));		
	}

	// 고객용 커맨드 - 명세표 선택
	else if(EAGLE_CMD_CL_SELECT_RECEIPT == byCmd)
	{
		LOG(Info, _T("RunService - Client - Select Receipt - Start"));

		if (FALSE == Proc_Client_SelectReceipt())
		{
			LOG(Error, _T("RunService - Client - Select Receipt - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Select Receipt - End"));
	}

	else if (EAGLE_CMD_CL_CONFIRM_RECEIPTERROR == byCmd)
	{
		LOG(Info, _T("RunService - Client - Select Receipt - Start"));

		if (FALSE == Proc_Client_Confirm_ReceiptError())
		{
			LOG(Error, _T("RunService - Client - Select Receipt - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Select Receipt - End"));
	}

	else if (EAGLE_CMD_CL_CONFIRM_RECEIPTSCREEN == byCmd)
	{
		LOG(Info, _T("RunService - Client - Confirm Receipt Error - Start"));

		if (FALSE == Proc_Client_Confirm_ScreenReceipt())
		{
			LOG(Error, _T("RunService - Client - Confirm Receipt Error - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Confirm Receipt Error - End"));
	}

	// 고객용 커맨드 - HOST 송/수신
	else if (EAGLE_CMD_CL_HOST_PROCESSING == byCmd)
	{
		LOG(Info, _T("RunService - Client - Host Processing - Start"));

		if (FALSE == Proc_Client_HostProcessing())
		{
			LOG(Error, _T("RunService - Client - Host Processing - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Host Processing - End"));
	}

	else if (EAGLE_CMD_CL_EMV_REMOVECARD == byCmd)
	{
		LOG(Info, _T("RunService - Client - EMV Remove Card - Start"));
	
		if (Proc_Client_EMV_Remove_Card() == TRUE)
		{
			if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
			{
				// Processing Error 후이므로 Reversal 또는 Print Receipt 처리
				if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal == ENABLE_REVERSAL)
				{
					// Reversal Process
					Client_ReversalProcessing();
				}
				else
				{
					// Host 송/수신 중 장애 발생시 명세표 선택 여부에 상관없이 무조건 명세표 발행
					Client_PrintReceipt();
				}
			}
			else
			{
				// 정상 제거 되었으므로 Process Complete 처리
				Proc_Client_NextStep_CheckCompleteHostProcessing();
			}
		}
		else
		{
			// Timeout시의 처리

			// 에러가 없을 경우에 Reversal 조건 Check
			if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE)
			{
				// Reversal 조건이 아닌 경우에는 정상으로 처리 (현재 미국은 출금만 Reversal을 하므로 조회나 이체는 Card 미 제거시 정상 처리함.
				//CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_EMV_IC_TIMEOUT, ERROR_DEVICE_NET);

				if (CEagleTritonMsg::GetInstance()->Triton_IsReversalCondition())
				{
					CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_EMV_IC_TIMEOUT, ERROR_DEVICE_NET);

					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_ICC_CARD_NOT_REMOVED;

					// Card 미수취로 인해 Error Notice 표시 후 진행하도록 로직 보완
					Client_Screen_ErrorNotice(GETTEXT_020);
				}
			}

			if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal == ENABLE_REVERSAL)
			{
				// Reversal Process
				Client_ReversalProcessing();
			}
			else
			{
				// Host 송/수신 중 장애 발생시 명세표 선택 여부에 상관없이 무조건 명세표 발행
				Client_PrintReceipt();
			}
		}
	}

	// 고객용 커맨드 - 현금 방출
	else if (EAGLE_CMD_CL_CASH_DISPENSING == byCmd)
	{
		LOG(Info, _T("RunService - Client - Cash dispensing - Start"));

		if (FALSE == Proc_Client_CashDispensing())
		{
			LOG(Error, _T("RunService - Client - Cash dispensing - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Cash dispensing - End"));
	}

	// 고객용 커맨드 - 현금 방출
	else if (EAGLE_CMD_CL_REVERSAL == byCmd)
	{
		LOG(Info, _T("RunService - Client - Reversal Processing - Start"));

		if (FALSE == Proc_Client_ReversalProcessing())
		{
			LOG(Error, _T("RunService - Client - Reversal Processing - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Reversal Processing - End"));
	}

	// 고객용 커맨드 - 영수증 프린트
	else if (EAGLE_CMD_CL_PRINT_RECEIPT == byCmd)
	{
		LOG(Info, _T("RunService - Client - Print Receipt - Start"));

		if (FALSE == Proc_Client_PrintReceipt())
		{
			LOG(Error, _T("RunService - Client - Print Receipt - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Print Receipt - End"));
	}

	// 고객용 커맨드 - 화면 영수증 표시
	else if (EAGLE_CMD_CL_SCREEN_RECEIPT == byCmd)
	{
		LOG(Info, _T("RunService - Client - Screen Receipt - Start"));

		if (FALSE == Proc_Client_ScreenReceipt())
		{
			LOG(Error, _T("RunService - Client - Screen Receipt - Error"));

			return FALSE;
		}

		LOG(Info, _T("RunService - Client - Screen Receipt - End"));
	}

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 고객 변경
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_ChangeCustom(void)
{
	// PRE-DIALING CLOSE..
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("MODEM") &&
		CEagleDataManager::GetInstance()->m_Config.m_Host.strPre_dial == S_ENABLE)
		CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Sync_PreDialCancelbyUser();

	// 거래 종료 후 항상 이 곳으로 전환되므로 여기서 Transaction Journal을 저장함
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_nProcessCount > 0)
	{
		if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_bSaveEJLforTransRecord == TRUE)
		{
			// 저널을 저장한 후 Flag Off 처리 (한번만 저널에 저장하도록 로직 수정)
			CEagleSVCLib::GetInstance()->SaveEJL_TransactionInfo();

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_bSaveEJLforTransRecord = FALSE;
			CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfoData(_T("IsSaveEJLTransRecord"), _T("0"));
		}
	}

	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsTransaction == TRUE)
	{
		// EMV 거래시 Card 존재 시 Remove Card Display
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction == TRUE)
		{
			// HOST 송/수신 이후에는 Remove Card가 표시되지 않도록 조건 처리
			if (CEagleDataManager::GetInstance()->m_Client_Info.m_nProcessCount == 0)
			{
				LOG(Info, _T("Waiting Remove Card"));
				Proc_Client_EMV_Remove_Card();
			}
		}

		if (Is_DIO_Ear_Jack_Insert() == TRUE)
		{
			// Play Remove Earphone
			CDEV_Manager::GetInstance()->ResetWaveFile();
			CDEV_Manager::GetInstance()->AddWaveFile(THANKYOU_WAVE_FILE);
			CDEV_Manager::GetInstance()->AddWaveFile(REMOVEEARPHONE_WAVE_FILE);
			CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
		}

#if (FACTORY_TEST)
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE)
		{
			CString strPath;
			strPath.Format(_T("%s%s"), CUtil::GetAppPath(), PATH_LOG);
			CEagleSVCLib::GetInstance()->Add_Backup_LogFile(strPath);
		}
#endif
	}

	// 거래 진행 중 Flag 초기화
	CEagleDataManager::GetInstance()->m_Client_Info.m_bIsTransaction = FALSE;

	// 거래 정보 초기화
	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.Clear();		// Memory Data 초기화
	CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfo();			// File Data 초기화

	// Camera Current Capture Image 삭제
	if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsOpened() == TRUE)
	{
		LOG(Info, _T("Delete Current Camera Image"));

		CUtil::DeleteFileInDirectory(CAPTURE_IMAGE_FILE_PATH_ABS, _T("*.*"));

		// 카메라가 계속 동작중이면 Holding으로 간주하여 Recovery 수행하도록 로직 추가
		if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsCameraProcessing() == TRUE)
		{
			LOG(Info, _T("Camera Processing is not completed - waiting 5 sec"));

			// 비밀번호 입력 후 바로 Cancel하는 경우 동작 완료가 안 될 수 있으므로 5초동안 감시하도록 로직 보완
			BOOL bResult = TRUE;
			DWORD	max_timeout = GetTickCount() + (5 * 1000);

			while(GetTickCount() < max_timeout)
			{
				if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsCameraProcessing() == FALSE)
				{
					bResult = FALSE;
					break;
				}

				CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
			}
			
			if (bResult == TRUE)
			{
				CString strTemp;
				strTemp = _T("[RECOVERY] SYSTEM REBOOT");
				CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(reboot, strTemp);

				LOG(Error, _T("Camera is Processing - Recovery Start - System Reboot"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);

				CDEV_Manager::GetInstance()->System_Reboot(TRUE);
			}
			else
			{
				LOG(Info, _T("Camera is Processing Completed - Skip"));				
			}
		}
	}

	// Triton Message 전문 정보 초기화
	CEagleTritonMsg::GetInstance()->InitializeVariable();
	CEagleTritonMsg::GetInstance()->m_strarrPrintTextData.RemoveAll();	// 위치 변경

	// EMV 관련 정보 초기화
#if (SUPPORT_EMV)
	CDEV_Manager::GetInstance()->m_DEV_CDR.Init_EMVKernel_Varialbe();
#endif

	// Volume Set - Default
	CEagleDataManager::GetInstance()->m_Client_Info.m_nSoundVolume = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strSound_Volume);
	CDEV_Manager::GetInstance()->SetVolume(CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strSound_Volume));

	// Language Set - Default English
	CSCR_Manager::GetInstance()->SetScreenLocale(EAGLE_LANGUAGE_ENGLISH);

	// Error Device가 어떤 Type인지 Logging
	LOG(Info, _T("ChangeCustom - Error Information : %08X"), CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice);

	// 이전 거래에서 장애가 발생했을 경우 Ouf of Service로 전환하도록 로직 추가
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
	{
		// 하기 2가지 조건으로 Error Clear하도록 함
		// 1. ErrorDevice가 존재하지 않을 경우
		// 2. Network Error만 존재하는 경우
		BOOL bClearError = FALSE;
		int nTempErrInfo = CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice;

		if (nTempErrInfo == 0)	// 통신 에러 + Reversal 에러인경우 NET Error Device가 Clear됨.
			bClearError = TRUE;
		else if (nTempErrInfo == ERROR_DEVICE_NET)
		{
			bClearError = TRUE;
		}

		//if (CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice == ERROR_DEVICE_NET)
		if (bClearError == TRUE)
		{
			LOG(Info, _T("Clear error code - reason : clear condition"));
			CEagleSVCLib::GetInstance()->Clear_Error();
		}
		else
		{
			// Out of Service로 전환
			SetOutOfService();

			// 에러가 초기화 될때까지 거래 진입 못하도록 return 처리
			return FALSE;
		}
	}

	// 거래 정보 초기화
	CEagleDataManager::GetInstance()->m_Client_Info.Clear();	// Client_Info는 Memory에만 저장됨 (File 관리 안함)

	// WDM이 Reject이 발생한 경우 Log BackUp 후 In Service로 전환하도록 로직 수정
	if (CDEV_Manager::GetInstance()->m_DEV_CDM.m_bNeedToRejectLog == TRUE)
	{
		CString strPath;
		strPath.Format(_T("%s%s"), CUtil::GetAppPath(), PATH_LOG);
		CEagleSVCLib::GetInstance()->Add_Backup_LogFile(strPath, _T("REJECTION"));
	}
	CDEV_Manager::GetInstance()->m_DEV_CDM.m_bNeedToRejectLog = FALSE;

	Client_InsertCard();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 카드 투입
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_InsertCard(void)
{
	BOOL bShowScreen = TRUE;
	BOOL bOld_Ear_Jack_Mode = FALSE;
	CString strTemp;
	int nReturn = 0;

	int nMemoryCheckCnt = 0;

	m_bAMSUploadJNLSend = TRUE;

	// In Service이므로 LED 제어
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_4|DIO_5|DIO_6|DIO_7, DIO_CMD_ON);

	// 카드 리더
	// FLICKER ON / LAMP FLICKING
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_FLICKING);
	CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(TRUE);
	
	// EMV가 On인 경우 Card 상태 조회
	if (CDEV_Manager::GetInstance()->m_DEV_CDR.IsEMV_Enable() == TRUE)
	{
		if (CDEV_Manager::GetInstance()->m_DEV_CDR.GetMediaStatus() & CARD_REAR_DETECT)
			CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_CardExist = TRUE;
	}

	// 카세트 정보 LOG 저장
	for(int i=0; i<CDEV_Manager::GetInstance()->m_DEV_CDM.m_nCbxCount; i++)
	{
		LOG(Info, _T("CST[%d] REMAIN COUNT : %s, DENOMINATION : %s"), i+1, CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i], CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i]);
	}

	// Receipt 정보 LOG 저장
	LOG(Info, _T("PTR STATUS : %d, MIDEA STATUS : %d"), CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetStatus(), CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetPaperStatus());


	try
	{
		// 카드 투입 대기(unlimited)
		while (TRUE)
		{
			nReturn = CheckService();		// EPP Timeout 시 재명령 처리

			if (GetServiceMode() != EAGLE_ATM_SVC_IN_SERVICE)
			{
				CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
				CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(FALSE);
				CDEV_Manager::GetInstance()->m_DEV_CDR.CancelInsertCard();

				// 이어폰이 연결된 경우 Sound 설정이 변경되므로 원래의 Volume Level로 설정 - 보류
				//CDEV_Manager::GetInstance()->SetVolume(CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strSound_Volume));

				return FALSE;
			}

			if (bShowScreen == TRUE)
			{
				bOld_Ear_Jack_Mode = Is_DIO_Ear_Jack_Insert();

				if (bOld_Ear_Jack_Mode == TRUE)
				{
					// AMS Connection Close
					LOG(Info, _T("EarPhone is Detected - Transaction Doing Flag Set"));

					this->PostMessage(WM_AMS_NOTIFY, EAGLE_ATM_DEV_AMS_CLOSE_START);
					CEagleDataManager::GetInstance()->m_Client_Info.m_bIsTransaction = TRUE;		// 거래 시작 Flag Set
				}
				else
				{
					LOG(Info, _T("EarPhone Removed - Transaction Doing Flag Cleared"));
					CEagleDataManager::GetInstance()->m_Client_Info.m_bIsTransaction = FALSE;
				}

				// 카드 투입 시작 메시지 통지
				Client_Screen_InsertCard(bOld_Ear_Jack_Mode);

				bShowScreen = FALSE;
			}

			// Device Event Check
			if (nReturn == 1)	// Health Check or Schedule Day Close 후 화면 갱신
			{
				bShowScreen = TRUE;
				continue;
			}

			if (bOld_Ear_Jack_Mode != Is_DIO_Ear_Jack_Insert())
			{
				// 초기화면에서 EarJack 삽입이나 제거시 Default Volume 설정하도록 수정
				CDEV_Manager::GetInstance()->SetVolume(CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strSound_Volume));

				// Ear Jack 변경시 화면 다시 갱신
				bShowScreen = TRUE;
				continue;
			}

			if (bOld_Ear_Jack_Mode == TRUE)
			{
				if (CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE)
				{
					// Auto Repeat
					bShowScreen = TRUE;
					continue;
				}
			}

			// Key Check
			if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
			{
				m_strScrOutName.Empty();
				m_strScrOutData.Empty();

				CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

				if (m_strScrOutData == _T("UP"))
				{
					// MAX인 경우는 Volume Up처리가 안되도록 보완 처리 (Wave File 보완 후 처리 필요하여 보류)
					//if (CDEV_Manager::GetInstance()->GetVolume() < MAX_VOLUME_LEVEL)
					{
						Client_ADA_Guide_VolumeUp();
						bShowScreen = TRUE;
						continue;
					}
				}
				else if (m_strScrOutData == _T("DOWN"))
				{
					// MIN인 경우는 Volume Down처리가 안되도록 보완 처리 (Wave File 보완 후 처리 필요하여 보류)
					//if (CDEV_Manager::GetInstance()->GetVolume() > MIN_VOLUME_LEVEL)
					{
						Client_ADA_Guide_VolumeDown();
						bShowScreen = TRUE;
						continue;
					}
				}
				else if (m_strScrOutData == _T("REPLY"))
				{
					Client_ADA_Guide_Repeat();
					bShowScreen = TRUE;
					continue;
				}
				else
				{
					Client_ADA_Guide_InvalidKey();
					bShowScreen = TRUE;
					continue;
				}
			}

			if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_CardExist == TRUE)
			{
				if (!(CDEV_Manager::GetInstance()->m_DEV_CDR.GetMediaStatus() & CARD_REAR_DETECT))	// Rear Detect가 아닌 경우 화면 갱신
				{
					CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_CardExist = FALSE;
					bShowScreen = TRUE;
					continue;
				}
			}
			else
			{
				nMemoryCheckCnt++;

				if (nMemoryCheckCnt > 3600)	// 1시간마다 메모리 Check 로직 추가
				{
					nMemoryCheckCnt = 0;

					CDEV_Manager::GetInstance()->CheckMemoryStatus();
				}

				// 카드 삽입 대기 (500ms wait)
				if (TRUE == CDEV_Manager::GetInstance()->m_DEV_CDR.InsertCard())
				{
					LOG(Info, _T("Insert Card - Card Detected"));
					break;
				}
				else
				{

					// 준정상 error code는 무시
					if (CDEV_Manager::GetInstance()->m_DEV_CDR.IsUnSuccessReply() == TRUE)
					{
						CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
						continue;
					}
					else
					{
						LOG(Error, _T("InsertCard Error (%s)"), strTemp);
						CEagleSVCLib::GetInstance()->Set_ErrorCode(strTemp, ERROR_DEVICE_IDC);

						CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
						CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(FALSE);

						return FALSE;
					}
				}
			}

			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
		}
	}
	catch (CException* e)
	{
		e->Delete();

		LOG(Error, _T(" Client InsertCard Exception"));

		CDEV_Manager::GetInstance()->CheckMemoryStatus();
	}

	CEagleDataManager::GetInstance()->m_Client_Info.m_bIsTransaction = TRUE;		// 거래 시작 Flag Set

	// 카드 리더 램프 OFF
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
	CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(FALSE);

	// AMS ConnectionClose
	this->PostMessage(WM_AMS_NOTIFY, EAGLE_ATM_DEV_AMS_CLOSE_START);

	// 카드 투입 완료 메시지 통지
	if (CDEV_Manager::GetInstance()->m_DEV_CDR.IsEMV_Enable() == TRUE)
	{
		// EMV Card Read
		Client_EMV_CardRead();
	}
	else
	{
		// 카드 제거
		GetInstance()->Client_RemoveCard();
	}

	// 거래 정상 시작시에도 Memory Check하도록 함
	CDEV_Manager::GetInstance()->CheckMemoryStatus();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 카드 제거
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_EMV_CardRead(void)
{
	BYTE byATRData[64] = { 0, };
	DWORD dwATRLen = 0;
	int	 nRetryPowerOn_Cnt = 0;
	BOOL bResult_PowerOn = FALSE;

	BOOL bADAMode = Is_DIO_Ear_Jack_Insert();

	// Card 위치로 인해 Power On이 실패하는 경우가 발생하여 3회 Retry하도록 로직 보완
	while(nRetryPowerOn_Cnt < 3)
	{
		memset(byATRData, 0, sizeof(byATRData));
		dwATRLen = 0;

		bResult_PowerOn = CDEV_Manager::GetInstance()->m_DEV_CDR.ICCPowerOn(byATRData, dwATRLen);
		
		if (bResult_PowerOn == FALSE)
			nRetryPowerOn_Cnt++;
		else
			break;

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_200MS);
	}

	CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction = TRUE;

	// Chip Power On
	if (bResult_PowerOn == FALSE)
	{
		LOG(Error, _T("final result - IC Power Failed"));

		// Power On시 Card가 제거된 경우 거래 취소 처리 로직 추가
		if(CARD_EMPTY == CDEV_Manager::GetInstance()->m_DEV_CDR.GetMediaStatus())
		{
			// 거래 취소 처리
			Client_Screen_TransactionCancel();
		}
		else
		{
			// IC Power On Fail로 인한 FallBack 처리
			Client_EMV_FallBack();
		}

		return TRUE;
	}

	//
	if (dwATRLen <= 0)
	{
		// ATR Read Fail - FallBack  Transaction
		LOG(Info, _T("ATR Error - Data Length  (%d)"), dwATRLen);
		Client_EMV_FallBack();

		return TRUE;
	}

	// ATR Data Exist
	LOG(Info, _T("IC Power Success (%s)"), CUtil::ConvertHexToString(byATRData, dwATRLen));

	if(TRUE == CDEV_Manager::GetInstance()->m_DEV_CDR.GetSupportEncryption())
	{
		// IC Read를 위해 암호화 Key를 CDR로 전달해야 함.
		BOOL bEncrypResult = CDEV_Manager::GetInstance()->m_DEV_CDR.GenerateKey4APDUData();
		if(FALSE == bEncrypResult)
		{
			CString strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();

			LOG(Error, _T("GenerateKey4APDUData - CDR - Error Code : ") + strErrorCode);

			// 에러 코드 설정
			CEagleSVCLib::GetInstance()->Set_ErrorCode(strErrorCode, ERROR_DEVICE_IDC);

			Client_Screen_TransactionCancel();

			return TRUE;
		}
		else
		{
			LOG(Info, _T("GenerateKey4APDUData - CDR - Succeeded"));
		}
		//////////////////////////////////////////////////////////////
	}

	// 화면 전환 후 Select Application 명령 수행
	CrypteraEPP_StopClearTextMode();

	Client_Screen_EMVProcessing(bADAMode);

	int nResult = -1;

	nResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_SelectApplication();

	if (bADAMode == TRUE)
	{
		// Ear Jack이 꼽힌 상태에서 빠진 경우 거래 취소 처리
		if (Is_DIO_Ear_Jack_Insert() == FALSE)
		{
			Client_Screen_TransactionCancel();
			return TRUE;
		}
	}

	if ( nResult != EMV_RSLT_OK)
	{
		if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
		{
			if (nResult == EMV_ERR_NOT_SUPPORT)
			{
				CDEV_Manager::GetInstance()->m_DEV_CDR.m_bUnknownAID = TRUE;
				LOG(Error, _T("SelectApplication - reason unknown AID"));
			}

			LOG(Error, _T("SelectApplication Error. (%d)"), nResult);
			Client_EMV_FallBack();
		}
		else
		{
			// 거래 취소 처리
			Client_Screen_TransactionCancel();
		}

		return TRUE;
	}

	Client_EMV_MakeAppLists();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 카드 제거
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_EMV_FallBack(void)
{
	CString strErrorCode;

	BOOL	bShowScreen = TRUE;
	int		nInitRetry = 0;

	CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction = FALSE;

	// ADA를 위한 무한 Timeout Set
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;

	// Card 미 존재시 거래 취소 처리
	if ((CDEV_Manager::GetInstance()->m_DEV_CDR.GetMediaStatus() & CARD_REAR_DETECT) == 0x00)
	{
		Client_Screen_TransactionCancel();

		return TRUE;
	}

	// 카드 리더
	// FLICKER ON
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_FLICKING);
	CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(TRUE);

	// EMV Fallback 거래 Notice Sound 추가
	if (bADAMode == FALSE)
	{
		PlaySound(CUtil::GetNoticeWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);
		CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_SoundNotice = TRUE;
	}

	// Card Remove를 위해 Initialize Command 추가 (Latch Off가 안되는 현상이 발생함)
	CDEV_Manager::GetInstance()->m_DEV_CDR.Initialize();

	// 카드 제거
	while(TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// Key Enable 처리
			CrypteraEPP_EnterClearTextMode();

			// FALLBACK 표시 메시지 통지
			Client_Screen_EMV_Fallback();

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
				bADA_StartToTimeout = TRUE;
			}

			bShowScreen = FALSE;
		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_EMV_FallBack - Ear Jack FALSE or Timeout - transaction cancel"));
				
				// EarJack이 빠질 경우 또는 Timeout시 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("EMV Fallback - ADA Timeout Set"));
				
				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
				bADA_StartToTimeout = FALSE;
			}
		}

		// Voice Guidance Check
		// Key Check
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == _T("UP"))
			{
				Client_ADA_Guide_VolumeUp();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("DOWN"))
			{
				Client_ADA_Guide_VolumeDown();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("REPLY"))
			{
				Client_ADA_Guide_Repeat();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				// 일반 화면에서의 Timeout 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}
			else
			{
				// ADA Mode에서만 Key Invalid 처리
				if (bADAMode == TRUE)
				{
					Client_ADA_Guide_InvalidKey();
					bShowScreen = TRUE;
					continue;
				}
				else
				{
					bShowScreen = TRUE;		// 잘못된 Key가 전달된 경우 화면 갱신
				}
			}
		}

		if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CDR.RemoveCard())
		{
			// 준정상 error code는 무시
			if (CDEV_Manager::GetInstance()->m_DEV_CDR.IsUnSuccessReply() == TRUE)
			{
				continue;
			}
			else
			{
				strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();

				if (strErrorCode.Right(2) == _T("10"))
				{
					// Latch Fail인 경우 CDR 초기화 Retry 로직 추가
					CDEV_Manager::GetInstance()->m_DEV_CDR.Initialize();

					LOG(Info, _T("Latch Retry Result : %s"), CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError());
					nInitRetry++;

					if (nInitRetry < 3)
						continue;
				}

				LOG(Error, _T("[EMV FALLBACK] Remove Card - Failed to card - Error Code : ") + strErrorCode);

				// 카드 리더 FLICKER OFF
				CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
				CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(FALSE);

				Client_Screen_CardReadError();

				return FALSE;
			}
		}

		// 카드 읽기
		Client_ReadCard();

		break;
	}

	// 카드 리더
	// FLICKER ON
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
	CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(FALSE);

	return TRUE;
}


/** **********************************************************
*	@brief		AID List를 추출하는 함수
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_EMV_MakeAIDList(void)
{
	int nResult = EMV_ERR_TERMINATE;
	
	while(TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		nResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_MakeAppLists();

		if (nResult != EMV_RSLT_OK)
		{
			LOG(Error, _T("EMV_MakeAppLists Error (%d)"), nResult);

			Client_Screen_TransactionCancel();
			return TRUE;
		}

		LOG(Info, _T("AID Count (%d) after EMV_MakeAppLists"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList);

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_US_Common_AID_List.US_Comm_AidUse == TRUE)
		{
			BOOL bRemoveAID = FALSE;
			int nCandidateList = CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList;

			if ( nCandidateList > 1)
			{
				LOG(Info, _T("Processing US Common AID - Start (%d)"), nCandidateList);

				int i, k, nBgPt;
				CString strTemp1, strTemp2;

				// Buffer for Country code and IIN
				// 7 bytes for each AID : Country Code[2], IIN[3], List Number[1], Remove ot not[1]
				unsigned char *pCompareCode = new unsigned char [(7*nCandidateList) + 1];
				memset(pCompareCode, NULL, (7*nCandidateList) + 1);

				// Assign Country code and IIN to buffer
				for (i=0; i< nCandidateList; i++)
				{
					LOG(Info, _T("==== AID [%d]===="), i);
					LOG(Info, _T("IsrCntryCode_Alpha2  = [%c%c]"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].IsrCntryCode_Alpha2[0], CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].IsrCntryCode_Alpha2[1]);
					LOG(Info, _T("IsrIdNo              = [%c%c%c]"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].IsrIdNo[0], CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].IsrIdNo[1], CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].IsrIdNo[2]);
					LOG(Info, _T("AID                  = [%s]"), CUtil::ConvertHexToString(CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].AidName, CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].AidLen));
					LOG(Info, _T("AID Name             = [%S]"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].AppLabel);
					LOG(Info, _T("AID Preferred Name   = [%S]"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].PreferredName);

					nBgPt = 7*i;				
					memcpy( &pCompareCode[nBgPt], CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].IsrCntryCode_Alpha2, 2);	// Copy Country Code (2 bytes)
					memcpy( &pCompareCode[nBgPt + 2], CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[i].IsrIdNo, 3);			// Copy IIN (3 bytes)
					pCompareCode[nBgPt + 5] = (unsigned char) i;																				// AID Order

					// Country Code => Make Capital for comparision (us => US)
					if( pCompareCode[nBgPt]   == 'u')		pCompareCode[nBgPt] = 'U';
					if( pCompareCode[nBgPt+1] == 's')		pCompareCode[nBgPt+1] = 'S';
				}

				// Sorting Buffer : "Country code + IIN"
				BOOL bChanged;
				unsigned char pBufTemp[6] = { 0, };
				unsigned long nCntryCode1, nCntryCode2, nIIN1, nIIN2;

				for(i=0; i<(nCandidateList-1); i++)
				{
					bChanged = FALSE;

					for(k=0; k<(nCandidateList-1); k++)
					{
						nCntryCode1 = pCompareCode[(k+0)*7]*256 + pCompareCode[(k+0)*7+1];
						nCntryCode2 = pCompareCode[(k+1)*7]*256 + pCompareCode[(k+1)*7+1];
						nIIN1 = pCompareCode[(k+0)*7+2]*255*255 + pCompareCode[(k+0)*7+3]*255 + pCompareCode[(k+0)*7+4];
						nIIN2 = pCompareCode[(k+1)*7+2]*255*255 + pCompareCode[(k+1)*7+3]*255 + pCompareCode[(k+1)*7+4];

						if( (nCntryCode1>nCntryCode2) || ( (nCntryCode1==nCntryCode2)&&(nIIN1>nIIN2) ) )
						{
							memcpy( pBufTemp,               &pCompareCode[k*7], 6 );
							memcpy( &pCompareCode[k*7],     &pCompareCode[(k+1)*7], 6 );
							memcpy( &pCompareCode[(k+1)*7], pBufTemp, 6 );
							bChanged = TRUE;
						}
					}
					if(bChanged==FALSE)
						break;
				}

				// Find the first and last AIDs which has "US" country code.....
				int nStartOrder = -1;
				int nEndOrder	= -1;
				BOOL bFoundStart = FALSE;
				for(i=0; i<nCandidateList; i++)
				{
					nBgPt = 7*i;
					nIIN1 = pCompareCode[nBgPt+2]*255*255 + pCompareCode[nBgPt+3]*255 + pCompareCode[nBgPt+4];
					if(nIIN1>0)			// Is "0x00 0x00 0x00" AN INSTITUTE ???????
					{
						if( (pCompareCode[nBgPt]=='U')&&(pCompareCode[nBgPt+1]=='S') )
						{
							if(bFoundStart!=TRUE)
							{
								bFoundStart = TRUE;
								nStartOrder = i;
							}
						}
						else
						{
							if(bFoundStart==TRUE)
							{
								nEndOrder = i-1;
								break;
							}
						}
					}
				}
				if( (bFoundStart==TRUE)&&(nStartOrder>=0)&&(nEndOrder<0) )
					nEndOrder = nCandidateList -1;

				// Mark Common AID (Set Flag)
				int nTargetAIDOrder;
				if(	(nStartOrder>=0) && (nEndOrder>nStartOrder) )
				{
					int nWorkingOrder = nStartOrder;
					int nSubStart, nSubEnd, nNumCommonAID;
					BOOL bCheckAll = FALSE;

					do
					{
						// Finding one IIN Sub group
						nSubStart = nWorkingOrder;
						nSubEnd = -1;
						nIIN1 = pCompareCode[nSubStart*7+2]*255*255 + pCompareCode[nSubStart*7+3]*255 + pCompareCode[nSubStart*7+4];
						for(k=(nSubStart+1); k<=nEndOrder; k++)
						{
							nIIN2 = pCompareCode[k*7+2]*255*255 + pCompareCode[k*7+3]*255 + pCompareCode[k*7+4];
							if (nIIN1 != nIIN2 )
							{
								nSubEnd = k-1;
								nWorkingOrder = k;
								break;
							}
						}
						if(nSubEnd<0)
						{
							nSubEnd = nEndOrder;
							nWorkingOrder = nEndOrder+1;
						}

						// if one IIN sub group has multiple AIDs.......
						if( nSubEnd > nSubStart)
						{
							// Compare AID with common AID... It if is one of common AID, set a flag
							nNumCommonAID = 0;
							for(i=nSubStart; i<=nSubEnd; i++)
							{
								// Target AID
								nTargetAIDOrder = pCompareCode[i*7+5];		
								CString strAID;
								strAID.Format(L"%s", CUtil::ConvertHexToString(CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[nTargetAIDOrder].AidName, CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[nTargetAIDOrder].AidLen) );
								for(k=0; k<CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_US_Common_AID_List.AidCount; k++)				// Check All US Common AID and Mark 1
								{
									strTemp1 = CUtil::ConvertHexToString(CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_US_Common_AID_List.US_Comm_Aid[k].Aid, CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_US_Common_AID_List.US_Comm_Aid[k].AidLen);					// US Common AID
									if(strAID.GetLength() >= strTemp1.GetLength() )
									{
										strTemp2 = strAID.Left(strTemp1.GetLength());	// Shortened Target AID (Same Length of Common AID)
										if(strTemp2.CompareNoCase(strTemp1)==0)			// Matching with Common AID
										{
											LOG(Info, _T("US COMMON AID Detected (%s)"), strAID)
											nNumCommonAID++;
											pCompareCode[i*7+6] = 1;					// This is Common AID....
											break;
										}
									}
								}
							}

							// Mark remove AID depends on OPTION (USE COMMON AID or USE INTERNATIONAL AID)
							if( (nNumCommonAID>0) && (nNumCommonAID<(nSubEnd-nSubStart+1)) )	// Common AIDs are mixed with other AIDs...
							{
								for(i=nSubStart; i<=nSubEnd; i++)
								{
									if(pCompareCode[i*7+6]==1)					// This is Common AID
									{
										pCompareCode[i*7+6] = 0;				// Do not Remove
									}
									else										// Other AIDs
									{
										pCompareCode[i*7+6] = 1;			// Remove this
									}
								}
							}
							else									// All Common AID or All Other AIDs
							{
								for(i=nSubStart; i<=nSubEnd; i++)
									pCompareCode[i*7+6] = 0;		// Do not remove All
							}
						}
						if(nWorkingOrder>=nEndOrder)
							bCheckAll = TRUE;
					} while (bCheckAll == FALSE);
				}

				// Remove marked AIDs
				int nOrgCandidateList = nCandidateList;
				for (i=0; i< nOrgCandidateList; i++)
				{	
					if( pCompareCode[i*7+6] == 1) // Marked AID..
					{
						nTargetAIDOrder = pCompareCode[i*7+5];
						LOG(Info, _T("[Proc_Client_EMV_MakeAIDList] Remove AID for US Common AID (%d), (%s)"), nTargetAIDOrder, CUtil::ConvertHexToString(CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[nTargetAIDOrder].AidName, CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[nTargetAIDOrder].AidLen));
						CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_RemoveAppList(nTargetAIDOrder);

						LOG(Info, _T("Removed AID Count (%d)"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList);

						bRemoveAID = TRUE;
					}
				}
				delete [] pCompareCode;
			}

			if (CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList <= 0)
			{
				// Terminate
				Client_Screen_TransactionCancel();

				return TRUE;
			}

			if (bRemoveAID == TRUE)
			{
				LOG(Info, _T("AID Removed - Retry Make AID List"));
				CUtil::Sleep_Wait(100);

				continue;
			}
			else
			{
				LOG(Info, _T("AID is not removed"));
			}
		}	// End of US Common AID Process

		break;
	}

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList < 1)
	{
		// 거래 취소 처리
		LOG(Error, _T("Transaction Cancelled - m_nCandidateList is (%d)"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList);

		Client_Screen_TransactionCancel();
		return TRUE;		
	}

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList > 1)
	{
		// Multi Select AID 화면으로 전환
		LOG(Info, _T("Proc_Client_EMV_MakeAIDList - AID List is  (%d)"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList);

		Client_EMV_MultiAppSelect();
		return TRUE;
	}

	LOG(Info, _T("Proc_Client_EMV_MakeAIDList AID Count is 1"));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.CardHolderConfirm == 0x00)
	{
		if (CDEV_Manager::GetInstance()->m_DEV_CDR.m_bShowAIDList == FALSE)
		{
			LOG(Info, _T("Proc_Client_EMV_MakeAIDList Auto Final Selection Start"));

			CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_Set_SelectAID_Index(0);
			Client_EMV_FinalSelect();
		}
		else
		{
			// Display AID List
			LOG(Info, _T("Proc_Client_EMV_MakeAIDList - AID List is  (%d)"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList);
			Client_EMV_MultiAppSelect();
		}
	}
	else
	{
		// Display AID List
		LOG(Info, _T("Proc_Client_EMV_MakeAIDList - AID List is  (%d)"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList);
		Client_EMV_MultiAppSelect();
	}

	return TRUE;
}


/** **********************************************************
*	@brief		Multi AID List 처리
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_EMV_MultiAppSelect(void)
{
	int nResult = EMV_ERR_TERMINATE;

	BOOL	bShowScreen = TRUE;
	BOOL	bNoticeSound = TRUE;
	int		nSelectedAID = -1;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	CDEV_Manager::GetInstance()->m_DEV_CDR.m_bShowAIDList = TRUE;

	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;

	while(TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// Key Enable 처리
			CrypteraEPP_EnterClearTextMode();

			Client_Screen_EMV_SelectMultiAID(bADAMode);

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);
				bADA_StartToTimeout = TRUE;
			}

			if (bNoticeSound == TRUE)
			{
				// EMV Fallback 거래 Notice Sound 추가
				if (bADAMode == FALSE)
					PlaySound(CUtil::GetNoticeWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);

				bNoticeSound = FALSE;
			}


			bShowScreen = FALSE;
		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_EMV_MultiAppSelect - Ear Jack FALSE or Timeout - transaction cancel"));

				// EarJack이 빠질 경우 또는 Timeout시 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("Proc_Client_EMV_MultiAppSelect - ADA Timeout Set"));

				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);
				bADA_StartToTimeout = FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == _T("UP"))
			{
				Client_ADA_Guide_VolumeUp();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("DOWN"))
			{
				Client_ADA_Guide_VolumeDown();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("REPLY"))
			{
				Client_ADA_Guide_Repeat();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				// 일반 화면에서의 Timeout 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}
			else
			{
				if (CUtil::IsNumeric(m_strScrOutData.Right(1)) == TRUE)
				{
					nSelectedAID = CUtil::StringToInt(m_strScrOutData.Right(1)) -1;

					LOG(Info, _T("[Multi App Selection] Selected AID is (%d)"), nSelectedAID);

					if (nSelectedAID > CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList)
					{
						if (bADAMode == FALSE)
						{
							// 거래 종료 처리
							LOG(Error, _T("[Multi App Selection] Selected AID Index Error (%d)"), nSelectedAID);

							Client_Screen_TransactionCancel();
							return TRUE;
						}
						else
						{
							Client_ADA_Guide_InvalidKey();
							bShowScreen = TRUE;
							continue;
						}
					}
					else
					{
						if (bADAMode == TRUE)
						{
							// Selected card type
							CString strTemp;

							CDEV_Manager::GetInstance()->ResetWaveFile();
							strTemp = CDEV_Manager::GetInstance()->m_DEV_CDR.Get_BrandNameForADA(nSelectedAID);

							CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);
							CDEV_Manager::GetInstance()->AddWaveFile(strTemp);
							CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
						}
						break;
					}
				}
				else
				{
					// ADA Mode에서만 Key Invalid 처리
					if (bADAMode == TRUE)
					{
						Client_ADA_Guide_InvalidKey();
						bShowScreen = TRUE;
						continue;
					}
					else
					{
						bShowScreen = TRUE;
					}
				}
			}
		}	

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Final App Selection 수행
	CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_Set_SelectAID_Index(nSelectedAID);

	Client_EMV_FinalSelect();

	return TRUE;
}


/** **********************************************************
*	@brief		AID List를 추출하는 함수
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_EMV_FromFinalAppToReadApp(void)
{
	int nResult = EMV_ERR_TERMINATE;

	nResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_FinalApplication();

	if (nResult != EMV_RSLT_OK)
	{
		if (nResult == EMV_ERR_NOT_ACCEPT)
		{
			if (CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList > 1)
			{
				CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_RemoveAppList(CDEV_Manager::GetInstance()->m_DEV_CDR.m_nSelectedAID);
				LOG(Info, _T("Proc_Client_EMV_FromFinalAppToReadApp - EMV_RemoveAppList (%d)"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_nSelectedAID);

				CDEV_Manager::GetInstance()->m_DEV_CDR.m_bShowAIDList = TRUE;
				LOG(Info, _T("Proc_Client_EMV_FromFinalAppToReadApp - m_bShowAIDList is TRUE"));

				Client_Screen_ErrorNotice(GETTEXT_016);

				Client_EMV_MakeAppLists();
			}
			else
			{
				LOG(Info, _T("Proc_Client_EMV_FromFinalAppToReadApp - Fallback (%d)"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList);

				Client_EMV_FallBack();
			}
		}
		else if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
		{
			LOG(Info, _T("EMV_FinalApplication Error. nResult  (%d)"), nResult);

			Client_EMV_FallBack();
		}
		else
		{
			// 거래 취소 처리
			LOG(Error, _T("Transaction Cancelled - EMV_FinalApplication is (%d)"), nResult);

			Client_Screen_TransactionCancel();
		}
		
		return TRUE;
	}

	// Set AID Default Config
	CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_StoreValFromRecord();

	// Execute Get Processing Option
	nResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_InitApplication();

	if (nResult != EMV_RSLT_OK)
	{
		if (nResult == EMV_ERR_NOT_ACCEPT)
		{
			if (CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList > 1)
			{
				CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_RemoveAppList(CDEV_Manager::GetInstance()->m_DEV_CDR.m_nSelectedAID);
				LOG(Info, _T("Proc_Client_EMV_FromFinalAppToReadApp - EMV_RemoveAppList (%d)"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_nSelectedAID);

				CDEV_Manager::GetInstance()->m_DEV_CDR.m_bShowAIDList = TRUE;
				LOG(Info, _T("Proc_Client_EMV_FromFinalAppToReadApp - m_bShowAIDList is TRUE"));

				Client_Screen_ErrorNotice(GETTEXT_016);
				Client_EMV_MakeAppLists();
			}
			else
			{
				LOG(Info, _T("Proc_Client_EMV_FromFinalAppToReadApp - Fallback (%d)"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_nCandidateList);
				Client_EMV_FallBack();
			}
		}
		else if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
		{
			LOG(Info, _T("Proc_Client_EMV_FromFinalAppToReadApp - Init - Fallback (%d)"), nResult);
			Client_EMV_FallBack();
		}
		else
		{
			LOG(Info, _T("Proc_Client_EMV_FromFinalAppToReadApp - Init - Terminate (%d)"), nResult);
			Client_Screen_TransactionCancel();
		}

		return TRUE;
	}

	// Multi AID Selection 화면에서 선택시 화면 갱신
	if (CDEV_Manager::GetInstance()->m_DEV_CDR.m_bShowAIDList == TRUE)
	{
		CrypteraEPP_StopClearTextMode();
		Client_Screen_EMVProcessing(Is_DIO_Ear_Jack_Insert());
	}

	// Execute Read Data
	nResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_ReadAppData();

	if (nResult != EMV_RSLT_OK)
	{
		if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
		{
			LOG(Info, _T("Proc_Client_EMV_FromFinalAppToReadApp - ReadAppData - Fallback (%d)"), nResult);
			Client_EMV_FallBack();
		}
		else
		{
			LOG(Info, _T("Proc_Client_EMV_FromFinalAppToReadApp - ReadAppData - Terminate (%d)"), nResult);
			Client_Screen_TransactionCancel();
		}

		return TRUE;
	}

	// AID Select Complete, save AID and label
	int nLen = 0;
	CString strValue;
	unsigned char	szTemp[1024];

	memset(szTemp, 0x0, sizeof(szTemp));
	
	// Check Issuer Code Index 
	if (CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[CDEV_Manager::GetInstance()->m_DEV_CDR.m_nSelectedAID].IsrCodeTableIndex == 1)
	{
		nLen = CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[CDEV_Manager::GetInstance()->m_DEV_CDR.m_nSelectedAID].PreferredLen;

		if (nLen > 0)
		{
			memcpy(szTemp, CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[CDEV_Manager::GetInstance()->m_DEV_CDR.m_nSelectedAID].PreferredName, nLen);
		}
		else
		{
			nLen = CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[CDEV_Manager::GetInstance()->m_DEV_CDR.m_nSelectedAID].AppLabelLen;
			memcpy(szTemp, CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[CDEV_Manager::GetInstance()->m_DEV_CDR.m_nSelectedAID].AppLabel, nLen);
		}
	}
	else
	{
		nLen = CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[CDEV_Manager::GetInstance()->m_DEV_CDR.m_nSelectedAID].AppLabelLen;
		memcpy(szTemp, CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[CDEV_Manager::GetInstance()->m_DEV_CDR.m_nSelectedAID].AppLabel, nLen);
	}

	strValue.Format(_T("%S"), szTemp);
	strValue.TrimRight();

	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_str_AID_Label = strValue;

	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	nLen = CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[CDEV_Manager::GetInstance()->m_DEV_CDR.m_nSelectedAID].AidLen;
	memcpy(szTemp, CDEV_Manager::GetInstance()->m_DEV_CDR.m_sIccAppInform.IccAppDesc[CDEV_Manager::GetInstance()->m_DEV_CDR.m_nSelectedAID].AidName, nLen);

	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);
		strValue.TrimRight();

		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_str_AID = strValue;
	}

	LOG(Info, _T("Selected AID : %s, AID Label : %s"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_str_AID, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_str_AID_Label);

	CString strIDCTrack2;
	nLen = 0;
	memset(szTemp, 0, sizeof(szTemp));

	/* Get 2 Track data from IC */
	CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue((unsigned char*)EMVTag_Track2EquData, &nLen, szTemp);
	strIDCTrack2 = CUtil::ConvertHexToString(szTemp, nLen);
	LOG(Info, _T("Get Track2 Information"));

	if (strIDCTrack2.GetLength() > 0)
	{
		int	nSearchIndex;
		CString strPackCardData;

		// remove padding data 'F'	for EMV PSC 2008.10.14; standard1 EMV 전문에 'F'가 padding되면 37 bytes를 넘어가기 때문에 
		// padding data 'F'를 삭제함
		nSearchIndex = strIDCTrack2.FindOneOf(_T("F"));

		if(nSearchIndex != -1)
			strIDCTrack2.Replace(_T("F"), _T(""));

		/* Replace 'D' to '=' in the Track 2 data from the IC */
		/* Upper & Lower Case Check!! "D" or "d"*/
		nSearchIndex = strIDCTrack2.FindOneOf(_T("D"));

		if(nSearchIndex != -1)
			strIDCTrack2.Replace(_T("D"), _T("="));

		if (nSearchIndex >= 13)
		{
			// Card PAN for EPP
			CEagleDataManager::GetInstance()->m_Client_Info.m_strAccountNo = strIDCTrack2.Mid(nSearchIndex - 13, 12);

			// Save ISO2
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTrack2Data = strIDCTrack2;

			// 명세표 및 저널에 사용할 카드 데이터 설정
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strShowCardNumber.Format(_T("************%s"), strIDCTrack2.Mid(nSearchIndex - 4, 4));

		}
		else
		{
			LOG(Error, _T("Read Card - track2 length error %d"), nSearchIndex);

			// 에러 메시지 통지
			Client_Screen_TransactionCancel();

			return TRUE;
		}
	}

	// Check ISO 2 Track
	if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTrack2Data.GetLength() <= 0)
	{
		LOG(Error, _T("IC Track2 length error"));

		// 에러 메시지 통지
		Client_Screen_TransactionCancel();

		return TRUE;
	}

	// Check the mandatory information.
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_strAccountNo.GetLength() <= 0)
	{
		LOG(Error, _T("IC Account No length error"));

		// 에러 메시지 통지
		Client_Screen_TransactionCancel();

		return TRUE;
	}

	// BIN CHECK
	CString strRegistedBin;

	for(int i=0; i<MAX_BIN_COUNT; i++)
	{
		strRegistedBin = CEagleDataManager::GetInstance()->m_Config.m_Iso.strBLOCK_ISO[i];

		strRegistedBin.TrimLeft();
		strRegistedBin.TrimRight();

		if (strRegistedBin.IsEmpty() == FALSE)
		{
			if (strRegistedBin == CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTrack2Data.Left(strRegistedBin.GetLength()))
			{
				LOG(Info, _T("Block Bin is Detected : BIN Number : %s"), strRegistedBin);
				CEagleDataManager::GetInstance()->m_Client_Info.m_bIsBlockBin = TRUE;
				break;
			}
		}		
	}

	Client_SelectLanguage();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 카드 제거
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_RemoveCard(void)
{

	// 카드 제거
	while(TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CDR.RemoveCard())
		{
			// 준정상 error code는 무시
			if (CDEV_Manager::GetInstance()->m_DEV_CDR.IsUnSuccessReply() == TRUE)
			{
				continue;
			}
			else
			{
				LOG(Error, _T("Remove Card - Failed to card - Error Code : ") + CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError());

				// 에러 메시지 통지
				Client_Screen_CardReadError();
				return FALSE;
			}
		}

		Client_ReadCard();

		break;
	}

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 카드 읽기
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_ReadCard(void)
{
	UINT nErrorCode = 0;

	// 카드 번호
	CString strCardNumber;

	// Read ISO
	CString strISO1, strISO2, strISO3;

	if(TRUE == CDEV_Manager::GetInstance()->m_DEV_CDR.GetSupportEncryption())
	{
		// MS data 암/복호화를 위한 키 교환
		BOOL bEncrypResult = FALSE;
		bEncrypResult = CDEV_Manager::GetInstance()->m_DEV_CDR.GenerateKey4MagneticData();
		if(FALSE == bEncrypResult)
		{
			CString strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();

			LOG(Error, _T("Read Card - GenerateKey4MagneticData() is failed - Error Code : ") + strErrorCode);

			// 에러 코드 설정
			CEagleSVCLib::GetInstance()->Set_ErrorCode(strErrorCode, ERROR_DEVICE_IDC);

			Client_Screen_CardReadError();

			return FALSE;
		}
		else
		{
			LOG(Info, _T("Read Card - GenerateKey4MagneticData() is success"));
		}
	}

	//CDEV_Manager::GetInstance()->m_DEV_CDR.ReadCardData_ISO1(&strISO1);

	if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CDR.ReadCardData_ISO2(&strISO2))
	{
		Client_Screen_CardReadError();

		return FALSE;
	}

	//CDEV_Manager::GetInstance()->m_DEV_CDR.ReadCardData_ISO3(&strISO3);

	int nSearchIndex = strISO2.Find('=');

	if (nSearchIndex == -1)
	{
		// Not Found '=' data
		LOG(Error, _T("Read Card - not found '=' data"));

		Client_Screen_CardReadError();

		return FALSE;
	}

	if (nSearchIndex >= 13)
	{
		// Card PAN for EPP
		CEagleDataManager::GetInstance()->m_Client_Info.m_strAccountNo = strISO2.Mid(nSearchIndex - 13, 12);

		// 명세표 및 저널에 사용할 카드 데이터 설정
		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strShowCardNumber.Format(_T("************%s"), strISO2.Mid(nSearchIndex - 4, 4));
		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTrack2Data = strISO2;

		LOG(Info, _T("RunService - Client Read Card - Card Data - ") + CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strShowCardNumber);
	}
	else
	{
		LOG(Error, _T("Read Card - track2 length error %d"), nSearchIndex);

		Client_Screen_CardReadError();

		return FALSE;
	}


	// Clear Buffer
	if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CDR.ClearBuffer())
	{
		CString strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();

		LOG(Error, _T("Read Card - Failed to read card - Clear buffer - Error Code : ") + strErrorCode);

		Client_Screen_CardReadError();

		return FALSE;
	}

	// BIN CHECK
	CString strRegistedBin;

	for(int i=0; i<MAX_BIN_COUNT; i++)
	{
		strRegistedBin = CEagleDataManager::GetInstance()->m_Config.m_Iso.strBLOCK_ISO[i];

		strRegistedBin.TrimLeft();
		strRegistedBin.TrimRight();
		if (strRegistedBin.IsEmpty() == FALSE)
		{
			if (strRegistedBin == strISO2.Left(strRegistedBin.GetLength()))
			{
				LOG(Info, _T("Block Bin is Detected : BIN Number : %s"), strRegistedBin);
				CEagleDataManager::GetInstance()->m_Client_Info.m_bIsBlockBin = TRUE;
				break;
			}
		}		
	}

	Client_SelectLanguage();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 언어 선택
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_SelectLanguage(void)
{
	BOOL bShowScreen = TRUE;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	// Language가 2개 이상일 경우 언어 선택 화면으로 전환
	int nSuppoertedLanguage = 0;

	for(int i=0; i<6; i++)
	{
		if (CUtil::StringToBOOL(CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[i]))
			nSuppoertedLanguage++;
	}

	// ADA Mode인 경우는 무조건 영어로 진행
	if (Is_DIO_Ear_Jack_Insert() == TRUE)
		nSuppoertedLanguage = 1;

	if (nSuppoertedLanguage <= 1)
	{
		Client_ReadPin();

		return TRUE;
	}

	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction == TRUE)
	{
		if (Is_DIO_Ear_Jack_Insert() == FALSE)
		{
			PlaySound(CUtil::GetNoticeWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);
			CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_SoundNotice = TRUE;
		}
	}
	else
	{
		CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_SoundNotice = TRUE;
	}

	while (TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// 언어선택 화면은 ADA 미지원

			// 화면 표시 후 Key Enable
			CrypteraEPP_EnterClearTextMode();

			// 거래 시작 메시지 통지
			Client_Screen_SelectLanguage();

			bShowScreen = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);
			break;
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	if (m_strScrOutData == S_CANCEL || m_strScrOutData == S_EXIT)
	{
		Client_Screen_TransactionCancel();

		return TRUE;
	}
	else if (m_strScrOutData == S_TIMEOVER)
	{
		Client_Screen_TransactionCancel();

		return TRUE;
	}
	else if (m_strScrOutData == _T("ENGLISH"))
	{
		CSCR_Manager::GetInstance()->SetScreenLocale(EAGLE_LANGUAGE_ENGLISH);
	}
	else if (m_strScrOutData == _T("SPANISH"))
	{
		CSCR_Manager::GetInstance()->SetScreenLocale(EAGLE_LANGUAGE_SPANISH);
	}
	else if (m_strScrOutData == _T("FRENCH"))
	{
		CSCR_Manager::GetInstance()->SetScreenLocale(EAGLE_LANGUAGE_FRENCH);
	}

	Client_ReadPin();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - PIN 읽기
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_ReadPin(void)
{
	int nEPPState = 0;
	BOOL bShowScreen = TRUE;

	// ADA를 위한 무한 Timeout Set
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();


//#if (NETWORK_OFFLINE_MODE)
//	CrypteraEPP_EnterClearTextMode();	// Offline에서는 일반 Text Mode로만 운영되도록 함.(For supporting Demo)
//#else
	// EPP STATE CHECK [$$$]
	//nEPPState = CDEV_Manager::GetInstance()->m_DEV_CREPP.GetDeviceStatus();

	//if (GET_EPP_ACTIVATE__STRING(nEPPState) != _T("ACTIVATED"))
	//{
	//	LOG(Error, _T("EPP STATE IS NOT ACTIVATED (%d"), nEPPState);
	//	Client_Screen_TransactionCancel();

	//	return TRUE;
	//}

	// EPP State가 Active가 아닌 경우에도 Start Entry가 성공됨.
	if (CDEV_Manager::GetInstance()->m_DEV_CREPP.StartEntryReadPin() == FALSE)
	{
		Client_Screen_TransactionCancel();

		return TRUE;
	}
//#endif

	// 화면 처리 후  Camera Capture를 수행하도록 함
	if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsOpened() == TRUE)
	{
		// 1. Capture Image File Name 생성
		CDEV_Manager::GetInstance()->m_DEV_CAMERA.StillCapture(TRUE, TRUE);
	}

	if (Is_DIO_Ear_Jack_Insert() == FALSE)
	{
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_SoundNotice == FALSE)
		{
			PlaySound(CUtil::GetNoticeWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);
			CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_SoundNotice = TRUE;
		}
	}

	while (TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			Client_Screen_EnterPIN(bADAMode);

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
				bADA_StartToTimeout = TRUE;
			}

			bShowScreen = FALSE;
		}

		// Stuck EPP 처리 추가
		if (CDEV_Manager::GetInstance()->m_DEV_CREPP.m_bEPPStuckState == TRUE)
		{
			// 거래 취소 처리
			Client_Screen_TransactionCancel();
			break;
		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_ReadPin - Ear Jack FALSE or Timeout - transaction cancel"));

				// ADA 도중 EarJack을 빼는 경우 거래 취소 처리
				Client_Screen_TransactionCancel();
				break;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("EMV Proc_Client_ReadPin - ADA Timeout Set"));

				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
				bADA_StartToTimeout = FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == S_EXIT || m_strScrOutData == S_CANCEL)
			{
				if (bADAMode == TRUE)
					Client_ADA_Guide_Cancel();

				Client_Screen_TransactionCancel();
				break;
			}
			else if (m_strScrOutData == S_TIMEOVER)
			{
				Client_Screen_TransactionCancel();
				break;
			}
			else if (m_strScrOutData == S_ENTER)
			{
				if (bADAMode == TRUE)
					Client_ADA_Guide_Enter();

				Client_Select_Transaction();
				break;

				// PinBlock은 Host 전문 송신 부분에서 취득 (EMV 거래 지원을 위해)
			}
			else if (m_strScrOutData == S_CLEAR)
			{
				if (bADAMode == TRUE)
					Client_ADA_Guide_Clear();

				bShowScreen = TRUE;
			}
			else
			{
				Client_Screen_TransactionCancel();
				break;
			}
		}

		// 대기
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	CrypteraEPP_StopClearTextMode();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 거래 선택
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_SelectTransaction(void)
{
	BOOL	bShowScreen = TRUE;

	// ADA를 위한 무한 Timeout Set
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;
	CString strADA_ActiveKeyList, strTemp;
	CStringArray strArray_ADA_ActiveKey;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	if ((CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("MODEM")) && (CEagleDataManager::GetInstance()->m_Config.m_Host.strPre_dial == S_ENABLE))
		CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Async_PreDialStart(CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number);

	// Check Transaction Option - ADA가 아닌 경우에만 Auto Selection 하도록 함
	if (bADAMode == FALSE)
	{
		if ((CEagleDataManager::GetInstance()->m_Config.m_Option.strbalance_Enable == S_DISABLE) && (CEagleDataManager::GetInstance()->m_Config.m_Option.strtransfer_Enable == S_DISABLE))
		{
			LOG(Info, _T("Auto selected withdrawal transaction"));

			// 자동 출금 거래 선택 후 Next로 이동	
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType = S_WITHDRAWAL;

			// 계좌 선택
			Client_Select_Accounts();

			return TRUE;
		}
	}

	while (TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// Key Enable
			CrypteraEPP_EnterClearTextMode();

			// 거래 시작 메시지 통지
			strADA_ActiveKeyList = Client_Screen_SelectTransaction(bADAMode);

			strArray_ADA_ActiveKey.RemoveAll();
			CUtil::ParsingStringToStringArray(strADA_ActiveKeyList, (CString)UNIT_DELIMITER, strArray_ADA_ActiveKey);

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
				bADA_StartToTimeout = TRUE;
			}

			bShowScreen = FALSE;

		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_SelectTransaction - Ear Jack FALSE or Timeout - transaction cancel"));

				// EarJack이 빠질 경우 또는 Timeout시 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("Proc_Client_SelectTransaction - ADA Timeout Set"));

				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
				bADA_StartToTimeout = FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == _T("UP"))
			{
				Client_ADA_Guide_VolumeUp();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("DOWN"))
			{
				Client_ADA_Guide_VolumeDown();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("REPLY"))
			{
				Client_ADA_Guide_Repeat();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				// 일반 화면에서의 Timeout 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}
			else
			{
				// ADA Mode에서만 Key Invalid 처리
				if (bADAMode == TRUE)
				{
					BOOL bFoundKey = FALSE;
					// Vaild Key Check
					for(int i=0; i<strArray_ADA_ActiveKey.GetCount(); i++)
					{
						if (m_strScrOutData == strArray_ADA_ActiveKey.GetAt(i))
						{
							CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType = strArray_ADA_ActiveKey.GetAt(i+1);
							bFoundKey = TRUE;

							// selected wave file play
							CDEV_Manager::GetInstance()->ResetWaveFile();
							CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);

							m_strScrOutData = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType;
							if (m_strScrOutData == S_WITHDRAWAL)
								CDEV_Manager::GetInstance()->AddWaveFile(WITHDRAWAL_WAVE_FILE);
							else if (m_strScrOutData == S_BALANCEINQUIRY)
								CDEV_Manager::GetInstance()->AddWaveFile(INQUIRY_WAVE_FILE);
							else if (m_strScrOutData == S_TRANSFER)
								CDEV_Manager::GetInstance()->AddWaveFile(TRANSFER_WAVE_FILE);

							CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
							break;
						}
					}

					if (bFoundKey == FALSE)
					{
						Client_ADA_Guide_InvalidKey();
						bShowScreen = TRUE;
						continue;
					}
				}

				// 일반 Mode에서는 정상 처리
				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType = m_strScrOutData;
				break;
			}
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	LOG(Info, _T("Selected Transaction : %s"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType);

	if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_TRANSFER)
	{
		// 이체 계좌 선택
		Client_Select_TransferAccounts();
	}
	else
	{
		// 계좌 선택
		Client_Select_Accounts();
	}

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 계좌 선택
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_SelectAccounts(void)
{
	BOOL bShowScreen = TRUE;

	// ADA를 위한 무한 Timeout Set
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;
	CString strADA_ActiveKeyList, strTemp;
	CStringArray strArray_ADA_ActiveKey;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	// Account Option - ADA 거래가 아닌 경우에만 Auto Selection 하도록 함
	if (bADAMode == FALSE)
	{
		if ((CEagleDataManager::GetInstance()->m_Config.m_Option.strSaving_account == S_DISABLE) && (CEagleDataManager::GetInstance()->m_Config.m_Option.strCredit_account == S_DISABLE))
		{
			LOG(Info, _T("Auto Selected Account - Checking"));

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strFromAccountType = S_CHECKING;

			if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_WITHDRAWAL)
			{
				// 금액 선택
				Client_Select_Amount();
			}
			else
			{
				Proc_Client_NextStep_CheckPrinterStatus();
			}

			return TRUE;
		}
	}

	while (TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// Key Enable
			CrypteraEPP_EnterClearTextMode();

			// 거래 시작 메시지 통지
			strADA_ActiveKeyList = Client_Screen_SelectAccount(bADAMode);

			strArray_ADA_ActiveKey.RemoveAll();
			CUtil::ParsingStringToStringArray(strADA_ActiveKeyList, (CString)UNIT_DELIMITER, strArray_ADA_ActiveKey);

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
				bADA_StartToTimeout = TRUE;
			}

			bShowScreen = FALSE;
		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_SelectAccounts - Ear Jack FALSE or Timeout - transaction cancel"));

				// EarJack이 빠질 경우 또는 Timeout시 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("Proc_Client_SelectAccounts - ADA Timeout Set"));

				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
				bADA_StartToTimeout = FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == _T("UP"))
			{
				Client_ADA_Guide_VolumeUp();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("DOWN"))
			{
				Client_ADA_Guide_VolumeDown();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("REPLY"))
			{
				Client_ADA_Guide_Repeat();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				// 일반 화면에서의 Timeout 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}
			else
			{
				// ADA Mode에서만 Key Invalid 처리
				if (bADAMode == TRUE)
				{
					BOOL bFoundKey = FALSE;

					// Vaild Key Check
					for(int i=0; i<strArray_ADA_ActiveKey.GetCount(); i++)
					{
						if (m_strScrOutData == strArray_ADA_ActiveKey.GetAt(i))
						{
							bFoundKey = TRUE;

							m_strScrOutData = strArray_ADA_ActiveKey.GetAt(i+1);

							// selected wave file play
							CDEV_Manager::GetInstance()->ResetWaveFile();
							CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);

							if (m_strScrOutData == S_CHECKING)
								CDEV_Manager::GetInstance()->AddWaveFile(CHECKING_WAVE_FILE);
							else if (m_strScrOutData == S_SAVINGS)
								CDEV_Manager::GetInstance()->AddWaveFile(SAVINGS_WAVE_FILE);
							else if (m_strScrOutData == S_CREDIT)
								CDEV_Manager::GetInstance()->AddWaveFile(CREDIT_WAVE_FILE);

							CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
							break;
						}
					}

					if (bFoundKey == FALSE)
					{
						Client_ADA_Guide_InvalidKey();
						bShowScreen = TRUE;
						continue;
					}
				}

				// 일반 Mode에서는 정상 처리
				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strFromAccountType = m_strScrOutData;
				break;
			}
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_WITHDRAWAL)
	{
		// 금액 선택
		Client_Select_Amount();
	}
	else
	{
		Proc_Client_NextStep_CheckPrinterStatus();
	}

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 이체 계좌 선택
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_SelectTransferAccounts(void)
{
	BOOL bShowScreen = TRUE;

	// ADA를 위한 무한 Timeout Set
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;
	CString strADA_ActiveKeyList, strTemp;
	CStringArray strArray_ADA_ActiveKey;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	while (TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// Key Enable
			CrypteraEPP_EnterClearTextMode();

			// 거래 시작 메시지 통지
			strADA_ActiveKeyList = Client_Screen_SelectTransferAccount();

			strArray_ADA_ActiveKey.RemoveAll();
			CUtil::ParsingStringToStringArray(strADA_ActiveKeyList, (CString)UNIT_DELIMITER, strArray_ADA_ActiveKey);

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
				bADA_StartToTimeout = TRUE;
			}

			bShowScreen = FALSE;
		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_SelectTransferAccounts - Ear Jack FALSE or Timeout - transaction cancel"));

				// EarJack이 빠질 경우 또는 Timeout시 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("Proc_Client_SelectTransferAccounts - ADA Timeout Set"));

				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
				bADA_StartToTimeout = FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == _T("UP"))
			{
				Client_ADA_Guide_VolumeUp();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("DOWN"))
			{
				Client_ADA_Guide_VolumeDown();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("REPLY"))
			{
				Client_ADA_Guide_Repeat();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				// 일반 화면에서의 Timeout 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}
			else
			{
				// ADA Mode에서만 Key Invalid 처리
				if (bADAMode == TRUE)
				{
					BOOL bFoundKey = FALSE;

					// Vaild Key Check
					for(int i=0; i<strArray_ADA_ActiveKey.GetCount(); i++)
					{
						if (m_strScrOutData == strArray_ADA_ActiveKey.GetAt(i))
						{
							bFoundKey = TRUE;

							m_strScrOutData = strArray_ADA_ActiveKey.GetAt(i+1);

							// selected wave file play
							CDEV_Manager::GetInstance()->ResetWaveFile();
							CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);

							if (m_strScrOutData == _T("CHECKINGTOSAVINGS"))
								CDEV_Manager::GetInstance()->AddWaveFile(CHECKINGTOSAVINGS_WAVE_FILE);
							else if (m_strScrOutData == _T("SAVINGSTOCHECKING"))
								CDEV_Manager::GetInstance()->AddWaveFile(SAVINGSTOCHECKING_WAVE_FILE);
							else if (m_strScrOutData == _T("CREDITTOCHECKING"))
								CDEV_Manager::GetInstance()->AddWaveFile(CREDITTOCHECKING_WAVE_FILE);

							CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
							break;
						}
					}

					if (bFoundKey == FALSE)
					{
						Client_ADA_Guide_InvalidKey();
						bShowScreen = TRUE;
						continue;
					}
				}

				if (m_strScrOutData == _T("CHECKINGTOSAVINGS"))
				{
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strFromAccountType = S_CHECKING;
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strToAccountType = S_SAVINGS;
				}
				else if (m_strScrOutData == _T("SAVINGSTOCHECKING"))
				{
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strFromAccountType = S_SAVINGS;
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strToAccountType = S_CHECKING;
				}
				else if (m_strScrOutData == _T("CREDITTOCHECKING"))
				{
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strFromAccountType = S_CREDIT;
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strToAccountType = S_CHECKING;
				}
				else
				{
					// 예외 처리
					LOG(Error, _T("Select Transfer Account is invalid"));
					Client_Screen_TransactionCancel();

					return TRUE;
				}
				break;
			}
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	Client_Enter_TransferAmount();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 출금 금액 선택
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_SelectAmount()
{
	BOOL bShowScreen = TRUE;

	// ADA를 위한 무한 Timeout Set
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;
	CString strADA_ActiveKeyList, strTemp;
	CStringArray strArray_ADA_ActiveKey;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	while (TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// Key Enable
			CrypteraEPP_EnterClearTextMode();

			// 거래 시작 메시지 통지
			strADA_ActiveKeyList = Client_Screen_SelectAmount();

			strArray_ADA_ActiveKey.RemoveAll();
			CUtil::ParsingStringToStringArray(strADA_ActiveKeyList, (CString)UNIT_DELIMITER, strArray_ADA_ActiveKey);

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
				bADA_StartToTimeout = TRUE;
			}

			bShowScreen = FALSE;
		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_SelectAmount - Ear Jack FALSE or Timeout - transaction cancel"));

				// EarJack이 빠질 경우 또는 Timeout시 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("Proc_Client_SelectAmount - ADA Timeout Set"));

				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
				bADA_StartToTimeout = FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == _T("UP"))
			{
				Client_ADA_Guide_VolumeUp();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("DOWN"))
			{
				Client_ADA_Guide_VolumeDown();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("REPLY"))
			{
				Client_ADA_Guide_Repeat();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				// 일반 화면에서의 Timeout 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}
			else if (m_strScrOutData == S_CLEAR)
			{
				if (bADAMode == TRUE)
				{
					Client_ADA_Guide_InvalidKey();
					bShowScreen = TRUE;
					continue;
				}
			}
			else
			{
				if (bADAMode == TRUE)
				{
					BOOL bFoundKey = FALSE;


					LOG(Info, _T("Return Key Value : %s"), m_strScrOutData);

					// Valid Key Check
					for(int i=0; i<strArray_ADA_ActiveKey.GetCount(); i++)
					{
						if (m_strScrOutData == strArray_ADA_ActiveKey.GetAt(i))
						{
							bFoundKey = TRUE;

							m_strScrOutData.Empty();
							m_strScrOutData = strArray_ADA_ActiveKey.GetAt(i+1);

							LOG(Info, _T("Selected Amount : %s"), m_strScrOutData);

							// selected wave file play
							CDEV_Manager::GetInstance()->ResetWaveFile();
							CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);

							if (m_strScrOutData == _T("OTHERAMOUNT"))
								CDEV_Manager::GetInstance()->AddWaveFile(OTHER_WAVE_FILE);
							else
								CDEV_Manager::GetInstance()->AddAmountWaveFiles(m_strScrOutData);

							CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
							break;
						}
					}

					if (bFoundKey == FALSE)
					{
						Client_ADA_Guide_InvalidKey();
						bShowScreen = TRUE;
						continue;
					}
				}

				if (m_strScrOutData == _T("OTHERAMOUNT"))
				{
					// 거래 금액 선택 메시지 통지
					Client_Enter_WithdrawalAmount();
					return TRUE;
				}
				else
				{
					int nRequestAmt = 0;

					if (bADAMode == TRUE)
					{
						nRequestAmt = CUtil::StringToInt(m_strScrOutData);			
					}
					else
					{
						int nSelectedKey = (CUtil::StringToInt(m_strScrOutData.Right(1)) - 1);

						if (nSelectedKey < 0 || nSelectedKey > 5)
						{
							// Index Error
							Client_Screen_TransactionCancel();

							return TRUE;		
						}

						nRequestAmt = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[nSelectedKey]);
					}

					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strRequestAmount.Format(_T("%d"), nRequestAmt);
				}

				break;
			}

		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	if (bADAMode == TRUE)
		Client_ADA_ConfirmAmount();
	else
		Proc_Client_NextStep_CheckSurcharge();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 출금 금액 입력
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_EnterAmount()
{
	CString strADA_EnterAmount;

	BOOL bShowScreen = TRUE;

	// ADA를 위한 무한 Timeout Set
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;
	CString strADA_ActiveKeyList, strTemp;
	CStringArray strArray_ADA_ActiveKey;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	while (TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// Key Enable
			CrypteraEPP_EnterClearTextMode();

			// 거래 금액 선택 메시지 통지
			Client_Screen_EnterWithdrawalAmount();

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
				bADA_StartToTimeout = TRUE;
			}

			strADA_EnterAmount.Empty();
			bShowScreen = FALSE;
		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_EnterAmount - Ear Jack FALSE or Timeout - transaction cancel"));

				// EarJack이 빠질 경우 또는 Timeout시 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("Proc_Client_EnterAmount - ADA Timeout Set"));

				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
				bADA_StartToTimeout = FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == _T("UP"))
			{
				Client_ADA_Guide_VolumeUp();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("DOWN"))
			{
				Client_ADA_Guide_VolumeDown();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("REPLY"))
			{
				Client_ADA_Guide_Repeat();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				if ((bADAMode == TRUE) && (m_strScrOutData == S_EXIT))
					Client_ADA_Guide_Cancel();

				// 일반 화면에서의 Timeout 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}
			else if (m_strScrOutData == S_CLEAR)
			{
				if (bADAMode == TRUE)
				{
					Client_ADA_Guide_Clear();
				}

				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutName == _T("ENTERAMOUNT") || m_strScrOutData == S_ENTER)
			{
				int nInputAmount = 0;

				if (bADAMode == TRUE)
				{
					nInputAmount = CUtil::StringToInt(strADA_EnterAmount);

					Client_ADA_Guide_Enter();
				}
				else
				{
					nInputAmount = CUtil::StringToInt(m_strScrOutData);
				}

				LOG(Info, _T("Entered Amount is : %d"), nInputAmount);

				if ( nInputAmount > 0)
				{
					// 1회 최대 출금 금액보다 큰 경우에는 Error 처리
					if (nInputAmount <= CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strMax_withdrawal_amount))
					{
						// 방출 가능한지 Check 로직 추가 필요
						if (CDEV_Manager::GetInstance()->m_DEV_CDM.IsDispenable(CUtil::StringToInt(CUtil::IntToString(nInputAmount))) == TRUE)
						{
							CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strRequestAmount.Format(_T("%d"), nInputAmount);
							break;
						}
						else
						{
							Client_Screen_ErrorNotice(GETTEXT_001);

							// 금액 입력
							Client_Enter_WithdrawalAmount();

							return TRUE;
						}
					}
					else
					{
						Client_Screen_ErrorNotice(GETTEXT_001);

						// 금액 입력
						Client_Enter_WithdrawalAmount();
						return TRUE;
					}
				}
				else
				{
					Client_Screen_ErrorNotice(GETTEXT_001);

					// 금액 입력
					Client_Enter_WithdrawalAmount();
					return TRUE;
				}
			}
			else
			{
				if (bADAMode == TRUE)
				{
					if (strADA_EnterAmount.GetLength() < 4)	// 4자리 이상 입력시 4자리까지만 입력 처리
					{
						strADA_EnterAmount += m_strScrOutData;

						CDEV_Manager::GetInstance()->ResetWaveFile();
						CDEV_Manager::GetInstance()->AddNumberWaveFiles(CUtil::StringToInt(m_strScrOutData));
						CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
					}

					continue;
				}
				else
				{
					Client_Screen_TransactionCancel();

					return TRUE;
				}
			}

			break;
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	if (bADAMode == TRUE)
		Client_ADA_ConfirmAmount();
	else
		Proc_Client_NextStep_CheckSurcharge();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 출금 금액 입력
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_ADA_ConfirmAmount()
{
	CString strSelectedValue;

	BOOL bShowScreen = TRUE;

	// ADA를 위한 무한 Timeout Set
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;
	CString strADA_ActiveKeyList, strTemp;
	CStringArray strArray_ADA_ActiveKey;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	while (TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// Key Enable
			CrypteraEPP_EnterClearTextMode();

			// 거래 금액 선택 메시지 통지
			strADA_ActiveKeyList = Client_Screen_ConfirmAmount();

			strArray_ADA_ActiveKey.RemoveAll();
			CUtil::ParsingStringToStringArray(strADA_ActiveKeyList, (CString)UNIT_DELIMITER, strArray_ADA_ActiveKey);

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
				bADA_StartToTimeout = TRUE;
			}

			bShowScreen = FALSE;
		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_EnterAmount - Ear Jack FALSE or Timeout - transaction cancel"));

				// EarJack이 빠질 경우 또는 Timeout시 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("Proc_Client_EnterAmount - ADA Timeout Set"));

				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
				bADA_StartToTimeout = FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == _T("UP"))
			{
				Client_ADA_Guide_VolumeUp();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("DOWN"))
			{
				Client_ADA_Guide_VolumeDown();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("REPLY"))
			{
				Client_ADA_Guide_Repeat();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				// 일반 화면에서의 Timeout 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}
			else
			{
				if (bADAMode == TRUE)
				{
					BOOL bFoundKey = FALSE;


					LOG(Info, _T("Return Key Value : %s"), m_strScrOutData);

					// Valid Key Check
					for(int i=0; i<strArray_ADA_ActiveKey.GetCount(); i++)
					{
						if (m_strScrOutData == strArray_ADA_ActiveKey.GetAt(i))
						{
							bFoundKey = TRUE;

							m_strScrOutData.Empty();
							strSelectedValue = strArray_ADA_ActiveKey.GetAt(i+1);

							// selected wave file play
							CDEV_Manager::GetInstance()->ResetWaveFile();
							CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);

							if (strSelectedValue == S_YES)
								CDEV_Manager::GetInstance()->AddWaveFile(YES_WAVE_FILE);
							else
								CDEV_Manager::GetInstance()->AddWaveFile(NO_WAVE_FILE);

							CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
							break;
						}
					}

					if (bFoundKey == FALSE)
					{
						Client_ADA_Guide_InvalidKey();
						bShowScreen = TRUE;
						continue;
					}
				}
				else
				{
					Client_Screen_TransactionCancel();

					return TRUE;
				}
			}

			break;
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_WITHDRAWAL)	// 출금 거래시
	{
		if (strSelectedValue == S_YES)
			Proc_Client_NextStep_CheckSurcharge();
		else
			Client_Select_Amount();		// 금액 다시 입력
	}
	else	// 이체 거래시
	{
		if (strSelectedValue == S_YES)
			Proc_Client_NextStep_CheckPrinterStatus();
		else
			Client_Enter_TransferAmount();
	}

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 수수료 확인
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_ConfirmSurcharge()
{
	BOOL bShowScreen = TRUE;

	// ADA를 위한 무한 Timeout Set
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;
	CString strADA_ActiveKeyList, strTemp;
	CStringArray strArray_ADA_ActiveKey;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	while (TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// Key Enable
			CrypteraEPP_EnterClearTextMode();

			// 수수료 확인 메시지 통지
			strADA_ActiveKeyList = Client_Screen_FeeNotification();

			strArray_ADA_ActiveKey.RemoveAll();
			CUtil::ParsingStringToStringArray(strADA_ActiveKeyList, (CString)UNIT_DELIMITER, strArray_ADA_ActiveKey);

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
				bADA_StartToTimeout = TRUE;
			}

			bShowScreen = FALSE;
		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_ConfirmSurcharge - Ear Jack FALSE or Timeout - transaction cancel"));

				// EarJack이 빠질 경우 또는 Timeout시 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("Proc_Client_ConfirmSurcharge - ADA Timeout Set"));

				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
				bADA_StartToTimeout = FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == _T("UP"))
			{
				Client_ADA_Guide_VolumeUp();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("DOWN"))
			{
				Client_ADA_Guide_VolumeDown();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("REPLY"))
			{
				Client_ADA_Guide_Repeat();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				// 일반 화면에서의 Timeout 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}
			else
			{
				if (bADAMode == TRUE)
				{
					BOOL bFoundKey = FALSE;

					// Vaild Key Check
					for(int i=0; i<strArray_ADA_ActiveKey.GetCount(); i++)
					{
						if (m_strScrOutData == strArray_ADA_ActiveKey.GetAt(i))
						{
							bFoundKey = TRUE;

							m_strScrOutData = strArray_ADA_ActiveKey.GetAt(i+1);

							// selected wave file play
							CDEV_Manager::GetInstance()->ResetWaveFile();
							CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);

							if (m_strScrOutData == S_YES)
								CDEV_Manager::GetInstance()->AddWaveFile(YES_WAVE_FILE);
							else 
								CDEV_Manager::GetInstance()->AddWaveFile(NO_WAVE_FILE);

							CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);

							if (m_strScrOutData == S_EXIT)
							{
								Client_Screen_TransactionCancel();
								return TRUE;
							}

							break;
						}
					}

					if (bFoundKey == FALSE)
					{
						Client_ADA_Guide_InvalidKey();
						bShowScreen = TRUE;
						continue;
					}					
				}

				break;
			}

		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// 수수료 확인 완료 메시지 통지
	Proc_Client_NextStep_CheckPrinterStatus();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 이체 금액 입력
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_EnterTransferAmount()
{
	CString strADA_EnterTransferAmount;

	BOOL bShowScreen = TRUE;

	// ADA를 위한 무한 Timeout Set
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;
	CString strADA_ActiveKeyList, strTemp;
	CStringArray strArray_ADA_ActiveKey;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	while (TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// Key Enable
			CrypteraEPP_EnterClearTextMode();

			// Transfer Amount 입력 화면
			Client_Screen_EnterTransferAmount();

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
				bADA_StartToTimeout = TRUE;
			}

			strADA_EnterTransferAmount.Empty();
			bShowScreen = FALSE;
		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_EnterTransferAmount - Ear Jack FALSE or Timeout - transaction cancel"));

				// EarJack이 빠질 경우 또는 Timeout시 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("Proc_Client_EnterTransferAmount - ADA Timeout Set"));

				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
				bADA_StartToTimeout = FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == _T("UP"))
			{
				Client_ADA_Guide_VolumeUp();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("DOWN"))
			{
				Client_ADA_Guide_VolumeDown();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("REPLY"))
			{
				Client_ADA_Guide_Repeat();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				if ((bADAMode == TRUE) && (m_strScrOutData == S_EXIT))
					Client_ADA_Guide_Cancel();

				// 일반 화면에서의 Timeout 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}
			else if (m_strScrOutData == S_CLEAR)
			{
				if (bADAMode == TRUE)
				{
					Client_ADA_Guide_Clear();
				}

				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutName == _T("ENTERTRANSFERAMOUNT") || m_strScrOutData == S_ENTER)
			{
				int nInputAmount = 0;

				if (bADAMode == TRUE)
				{
					nInputAmount = CUtil::StringToInt(strADA_EnterTransferAmount);

					Client_ADA_Guide_Enter();
				}
				else
				{
					nInputAmount = CUtil::StringToInt(m_strScrOutData);
				}

				LOG(Info, _T("Entered Transfer Amount is : %d"), nInputAmount);

				if ( nInputAmount > 0)
				{
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransferAmount.Format(_T("%d"), nInputAmount);
					break;
				}
				else
				{
					LOG(Error, _T("Enter transfer amount error (%d)"), nInputAmount);

					// Show Error Notice
					Client_Screen_ErrorNotice(GETTEXT_001);

					// Enter Transfer Amount 다시 표시
					Client_Enter_TransferAmount();
					return TRUE;
				}
			}
			else
			{
				if (bADAMode == TRUE)
				{
					if (strADA_EnterTransferAmount.GetLength() < 8)
					{
						strADA_EnterTransferAmount += m_strScrOutData;

						CDEV_Manager::GetInstance()->ResetWaveFile();
						CDEV_Manager::GetInstance()->AddNumberWaveFiles(CUtil::StringToInt(m_strScrOutData));
						CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
					}

					continue;
				}
				else
				{
					Client_Screen_TransactionCancel();

					return TRUE;
				}
			}

			break;
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	if (bADAMode == TRUE)
		Client_ADA_ConfirmAmount();
	else
		Proc_Client_NextStep_CheckPrinterStatus();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 명세표 선택
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_SelectReceipt()
{
	BOOL bShowScreen = TRUE;

	// ADA를 위한 무한 Timeout Set
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;
	CString strADA_ActiveKeyList, strTemp;
	CStringArray strArray_ADA_ActiveKey;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	while (TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// Key Enable
			CrypteraEPP_EnterClearTextMode();

			// 거래 시작 메시지 통지
			strADA_ActiveKeyList = Client_Screen_SelectReceipt();

			strArray_ADA_ActiveKey.RemoveAll();
			CUtil::ParsingStringToStringArray(strADA_ActiveKeyList, (CString)UNIT_DELIMITER, strArray_ADA_ActiveKey);

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
				bADA_StartToTimeout = TRUE;
			}

			bShowScreen = FALSE;
		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_SelectReceipt - Ear Jack FALSE or Timeout - transaction cancel"));

				// EarJack이 빠질 경우 또는 Timeout시 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("Proc_Client_SelectReceipt - ADA Timeout Set"));

				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
				bADA_StartToTimeout = FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == _T("UP"))
			{
				Client_ADA_Guide_VolumeUp();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("DOWN"))
			{
				Client_ADA_Guide_VolumeDown();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("REPLY"))
			{
				Client_ADA_Guide_Repeat();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				// 일반 화면에서의 Timeout 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}
			else
			{
				if (bADAMode == TRUE)
				{
					BOOL bFoundKey = FALSE;

					// Valid Key Check
					for(int i=0; i<strArray_ADA_ActiveKey.GetCount(); i++)
					{
						if (m_strScrOutData == strArray_ADA_ActiveKey.GetAt(i))
						{
							bFoundKey = TRUE;

							m_strScrOutData = strArray_ADA_ActiveKey.GetAt(i+1);

							// selected wave file play
							CDEV_Manager::GetInstance()->ResetWaveFile();
							CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);

							if (m_strScrOutData == S_YES)
								CDEV_Manager::GetInstance()->AddWaveFile(YES_WAVE_FILE);
							else
								CDEV_Manager::GetInstance()->AddWaveFile(NO_WAVE_FILE);

							CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
							break;
						}
					}

					if (bFoundKey == FALSE)
					{
						Client_ADA_Guide_InvalidKey();
						bShowScreen = TRUE;
						continue;
					}					
				}

				if (m_strScrOutData == S_YES)
				{
					// 명세표 선택 Flag Set
					CEagleDataManager::GetInstance()->m_Client_Info.m_bIsPrintReceipt = TRUE;
				}
				else if (m_strScrOutData == S_NO)
				{
					// 명세표 선택 Flag Set
					CEagleDataManager::GetInstance()->m_Client_Info.m_bIsPrintReceipt = FALSE;
				}

				break;
			}
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// 명세표 선택 완료 메시지 통지
	Proc_Client_NextStep_CheckSelectReceipt();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 명세표 불가 확인
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_Confirm_ReceiptError()
{
	BOOL bShowScreen = TRUE;

	// ADA를 위한 무한 Timeout Set
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;
	CString strADA_ActiveKeyList, strTemp;
	CStringArray strArray_ADA_ActiveKey;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	while (TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// Key Enable
			CrypteraEPP_EnterClearTextMode();

			// 거래 시작 메시지 통지
			strADA_ActiveKeyList = Client_Screen_ConfirmReceiptError();

			strArray_ADA_ActiveKey.RemoveAll();
			CUtil::ParsingStringToStringArray(strADA_ActiveKeyList, (CString)UNIT_DELIMITER, strArray_ADA_ActiveKey);

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
				bADA_StartToTimeout = TRUE;
			}

			bShowScreen = FALSE;
		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_Confirm_ReceiptError - Ear Jack FALSE or Timeout - transaction cancel"));

				// EarJack이 빠질 경우 또는 Timeout시 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("Proc_Client_Confirm_ReceiptError - ADA Timeout Set"));

				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
				bADA_StartToTimeout = FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == _T("UP"))
			{
				Client_ADA_Guide_VolumeUp();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("DOWN"))
			{
				Client_ADA_Guide_VolumeDown();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("REPLY"))
			{
				Client_ADA_Guide_Repeat();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				// 일반 화면에서의 Timeout 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}
			else
			{
				if (bADAMode == TRUE)
				{
					BOOL bFoundKey = FALSE;

					// Valid Key Check
					for(int i=0; i<strArray_ADA_ActiveKey.GetCount(); i++)
					{
						if (m_strScrOutData == strArray_ADA_ActiveKey.GetAt(i))
						{
							bFoundKey = TRUE;

							m_strScrOutData = strArray_ADA_ActiveKey.GetAt(i+1);

							// selected wave file play
							CDEV_Manager::GetInstance()->ResetWaveFile();
							CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);

							if (m_strScrOutData == S_YES)
								CDEV_Manager::GetInstance()->AddWaveFile(YES_WAVE_FILE);
							else
								CDEV_Manager::GetInstance()->AddWaveFile(NO_WAVE_FILE);

							CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
							break;
						}
					}

					if (bFoundKey == FALSE)
					{
						Client_ADA_Guide_InvalidKey();
						bShowScreen = TRUE;
						continue;
					}
				}

				if (m_strScrOutData == S_EXIT)
				{
					Client_Screen_TransactionCancel();
					return TRUE;
				}

				CEagleDataManager::GetInstance()->m_Client_Info.m_bIsPrintReceipt = FALSE;
				break;
			}
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// 화면 명세표 확인 완료 메시지 통지
	Proc_Client_NextStep_CheckSelectReceipt();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 화면 명세표 확인
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_Confirm_ScreenReceipt()
{
	BOOL bShowScreen = TRUE;

	// ADA를 위한 무한 Timeout Set
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();
	BOOL	bADA_StartToTimeout = FALSE;
	CString strADA_ActiveKeyList, strTemp;
	CStringArray strArray_ADA_ActiveKey;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	if (bADAMode == TRUE)
	{
		// ADA인 경우에는 화면에 Screen 정보를 표시하지 않으므로 Skip 처리
		Client_Host_Processing();
		return TRUE;
	}

	while (TRUE)
	{
		CheckService();		// EPP Timeout 시 재명령 처리

		if (bShowScreen == TRUE)
		{
			// Key Enable
			CrypteraEPP_EnterClearTextMode();

			// 거래 시작 메시지 통지
			strADA_ActiveKeyList = Client_Screen_ConfirmScreenReceipt();

			strArray_ADA_ActiveKey.RemoveAll();
			CUtil::ParsingStringToStringArray(strADA_ActiveKeyList, (CString)UNIT_DELIMITER, strArray_ADA_ActiveKey);

			if (bADAMode == TRUE)
			{
				g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
				bADA_StartToTimeout = TRUE;
			}

			bShowScreen = FALSE;
		}

		if (bADAMode == TRUE)
		{
			if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
			{
				LOG (Error, _T("Proc_Client_Confirm_ScreenReceipt - Ear Jack FALSE or Timeout - transaction cancel"));

				// EarJack이 빠질 경우 또는 Timeout시 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}

			if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
			{
				LOG(Info, _T("Proc_Client_Confirm_ScreenReceipt - ADA Timeout Set"));

				g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
				bADA_StartToTimeout = FALSE;
			}
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			if (m_strScrOutData == _T("UP"))
			{
				Client_ADA_Guide_VolumeUp();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("DOWN"))
			{
				Client_ADA_Guide_VolumeDown();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == _T("REPLY"))
			{
				Client_ADA_Guide_Repeat();
				bShowScreen = TRUE;
				continue;
			}
			else if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				// 일반 화면에서의 Timeout 처리
				Client_Screen_TransactionCancel();
				return TRUE;
			}
			else
			{
				if (bADAMode == TRUE)
				{
					BOOL bFoundKey = FALSE;

					// Valid Key Check
					for(int i=0; i<strArray_ADA_ActiveKey.GetCount(); i++)
					{
						if (m_strScrOutData == strArray_ADA_ActiveKey.GetAt(i))
						{
							bFoundKey = TRUE;

							m_strScrOutData = strArray_ADA_ActiveKey.GetAt(i+1);

							// selected wave file play
							CDEV_Manager::GetInstance()->ResetWaveFile();
							CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);

							if (m_strScrOutData == S_YES)
								CDEV_Manager::GetInstance()->AddWaveFile(YES_WAVE_FILE);
							else
								CDEV_Manager::GetInstance()->AddWaveFile(NO_WAVE_FILE);

							CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
							break;
						}
					}

					if (bFoundKey == FALSE)
					{
						Client_ADA_Guide_InvalidKey();
						bShowScreen = TRUE;
						continue;
					}					
				}

				if (m_strScrOutData == S_EXIT)
				{
					Client_Screen_TransactionCancel();
					return TRUE;
				}

				break;
			}
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	Client_Host_Processing();

	return TRUE;
}

/** **********************************************************
*	@brief		고객용 커맨드 - HOST 송/수신
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_HostProcessing()
{
	int nResult = 0, nTransType = 0;
	CString strTransType, strTemp;
	int nTempSequenceNumber = 0;

	int	nEMVResult = EMV_ERR_TERMINATE;

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	// Key Disable
	CrypteraEPP_StopClearTextMode();

	// HOST 송/수신 메시지 통지
	Client_Screen_Processing();

	// 송신 전 Save
	// Save Last Transaction Information
	CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfo();

	// EMV Processing Start
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction == TRUE)
	{
		CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_StoreTransValues();

		nEMVResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_OfflineDataAuth();

		if (nEMVResult != EMV_RSLT_OK)
		{
			CEagleDataManager::GetInstance()->m_Client_Info.m_bEMVDeclined = TRUE;
			Client_Screen_TransactionCancel();

			return TRUE;
		}

		nEMVResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_ProcessRestrict();

		if (nEMVResult != EMV_RSLT_OK)
		{
			if (nEMVResult == EMV_ERR_NOT_SUPPORT || nEMVResult == EMV_ERR_CARD_SW)
			{
				Client_EMV_FallBack();
			}
			else
			{
				CEagleDataManager::GetInstance()->m_Client_Info.m_bEMVDeclined = TRUE;
				Client_Screen_TransactionCancel();
			}

			return TRUE;
		}

		nEMVResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_CardholderVerify();

		if (nEMVResult != EMV_RSLT_OK)
		{
			if (nEMVResult == EMV_ERR_NOT_SUPPORT || nEMVResult == EMV_ERR_CARD_SW)
			{
				Client_EMV_FallBack();
			}
			else
			{
				CEagleDataManager::GetInstance()->m_Client_Info.m_bEMVDeclined = TRUE;
				Client_Screen_TransactionCancel();
			}

			return TRUE;
		}

		nEMVResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TerminalRiskMgmt();

		if (nEMVResult != EMV_RSLT_OK)
		{
			if (nEMVResult == EMV_ERR_NOT_SUPPORT || nEMVResult == EMV_ERR_CARD_SW)
			{
				Client_EMV_FallBack();
			}
			else
			{
				CEagleDataManager::GetInstance()->m_Client_Info.m_bEMVDeclined = TRUE;
				Client_Screen_TransactionCancel();
			}

			return TRUE;
		}

		CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TerminalActionAnalysis();

		unsigned char byOnline = 0;
		nEMVResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_CardActionAnalysis(&byOnline);

		if (nEMVResult != EMV_RSLT_OK)
		{
			if (nEMVResult == EMV_ERR_NOT_SUPPORT || nEMVResult == EMV_ERR_CARD_SW)
			{
				Client_EMV_FallBack();
			}
			else
			{
				CEagleDataManager::GetInstance()->m_Client_Info.m_bEMVDeclined = TRUE;
				Client_Screen_TransactionCancel();
			}

			return TRUE;
		}

		if (byOnline != (unsigned char) ONLINE_TRANS)
		{
			CEagleDataManager::GetInstance()->m_Client_Info.m_bEMVDeclined = TRUE;

			CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_Completion();
			Client_Screen_TransactionCancel();

			return TRUE;
		}
	}

	// HOST 송 / 수신 처리
	CEagleDataManager::GetInstance()->m_Client_Info.m_nProcessCount = 1;		// Host 송/수신 후부터는 저널 저장을 위해 Count Set

	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_bSaveEJLforTransRecord = TRUE;
	CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfoData(_T("IsSaveEJLTransRecord"), _T("1"));

	// Convert Transaction Type from string to int
	strTransType = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType;

	if (strTransType == S_WITHDRAWAL)
		nTransType = EAGLE_TRAN_CODE_WITHDRAWAL;
	else if (strTransType == S_TRANSFER)
		nTransType = EAGLE_TRAN_CODE_TRANSFER;
	else if (strTransType == S_BALANCEINQUIRY)
		nTransType = EAGLE_TRAN_CODE_BALANCE;

	// Add Sequence Number
	nTempSequenceNumber = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_System.strSequence_Number);

	if (nTempSequenceNumber >= 9999)
		nTempSequenceNumber = 0;
	nTempSequenceNumber++;

	CEagleDataManager::GetInstance()->m_Config.m_System.strSequence_Number.Format(_T("%04d"), nTempSequenceNumber);

	// Sequence Number가 File 저장이 안되는 Bug Fix
	CEagleDataManager::GetInstance()->m_Config.SaveSystemData(_T("sequence_no"), CEagleDataManager::GetInstance()->m_Config.m_System.strSequence_Number);

	// Save file of transaction request message
	CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfo();

#if (NETWORK_OFFLINE_MODE)

	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strAutorizationNumber = _T("12345678");

	SYSTEMTIME	st;
	GetLocalTime(&st);

	strTemp.Format(_T("%04d"), st.wYear);
	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransactionDate.Format(_T("%02d%02d%02s"), st.wMonth, st.wDay, strTemp.Right(2));
	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransactionTime.Format(_T("%02d%02d%02d"), st.wHour, st.wMinute, st.wSecond);
	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strBusinessDate = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransactionDate;
	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strBalance = _T("777777");

	// EMV일 경우에는 Card 제거 후 방출 수행 아닌 경우 방출 수행
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction == TRUE)
	{
		Client_EMV_RemoveCard();
	}
	else
	{
		Proc_Client_NextStep_CheckCompleteHostProcessing();
	}
#else

	CEagleTritonMsg::GetInstance()->Triton_MakeHostMsg(nTransType);
	nResult = CEagleSVCLib::GetInstance()->SendRecvHost(nTransType);

	if (nResult == SUCCESS)
	{
		// Host로부터 정상 수신 후 Card 제거 전 Power Off 대비를 위해 Reversal Flag Setting
		if (CEagleTritonMsg::GetInstance()->Triton_IsReversalCondition())
		{
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_ICC_CARD_NOT_REMOVED;
		}

		LOG(Info, _T("Host Processing Success"));

		// Save file of transaction response message
		//CEagleDataManager::GetInstance()->m_Config.Save();

		// EMV일 경우에는 Card 제거 후 방출 수행 아닌 경우 방출 수행
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction == TRUE)
		{
			Client_EMV_RemoveCard();
		}
		else
		{
			Proc_Client_NextStep_CheckCompleteHostProcessing();
		}
	}
	else
	{
		if (nResult > RECV_FAIL_ENQ)
		{
			//if (nResult == RECV_FAIL_EOT)	// Send_Recv_Host에서 처리하므로 무의미한 코드 주석 처리
				//CEagleTritonMsg::GetInstance()->Triton_AnalHostData(nTransType);

			if (CEagleTritonMsg::GetInstance()->Triton_IsReversalCondition())
			{
				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;

				if (nResult == RECV_FAIL_EOT)
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_LOST_EOT_ERROR;
				else
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_PROTOCOL_ERROR;
			}
		}

		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction == TRUE)
		{
			if (CEagleDataManager::GetInstance()->m_Client_Info.m_bExec2ndGAC == FALSE)
			{
				// Invalid Tag인 경우 Default 처리
				CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_StoreValue(EMVTag_AuthRespCod, 2, (unsigned char*)"05");

				LOG(Error, _T("[EMV] Decline OnlineProcess"));

				nResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_OnlineProcess(AAC);

				if (nResult == EMV_RSLT_OK)
					CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_Completion();

				CEagleTritonMsg::GetInstance()->Triton_Save_EMV_Data();

				CEagleDataManager::GetInstance()->m_Client_Info.m_bExec2ndGAC = TRUE;
			}
		}

		// 명세표가 정상인 경우에만 Receipt 출력하도록 수정
		if (CEagleSVCLib::GetInstance()->IsPTR_Available() == TRUE)
			CEagleDataManager::GetInstance()->m_Client_Info.m_bIsPrintReceipt = TRUE;	// 장애인 경우 Receipt 무조건 출력

		// HOST 송/수신 에러 메시지 통지 (통신 장애)
		LOG(Info, _T("Host Processing Error"));

		// Save file of transaction response message
		//CEagleDataManager::GetInstance()->m_Config.Save();

		// Show Error Notice
		Client_Screen_ErrorNotice(GETTEXT_004, CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorMsg);

		// EMV일 경우에는 Card 제거 후 Reversal 수행
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction == TRUE)
		{
			Client_EMV_RemoveCard();
		}
		else
		{
			if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal == ENABLE_REVERSAL)
			{
				// Reversal Process
				Client_ReversalProcessing();
			}
			else
			{
				// Host 송/수신 중 장애 발생시 명세표 선택 여부에 상관없이 무조건 명세표 발행
				// 이전부터 명세표가 장애 상태인 경우에 대한 처리 보완 필요 [$$$]
				Client_PrintReceipt();
			}
		}
	}

#endif

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 현금 방출
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_CashDispensing()
{
	int i=0;
	CString strTemp;
	BOOL bReject = FALSE;

	Client_Screen_CashDispensing();

	// 출금 데이터
	CDMMULTIDISPENSE	cdmmultidispense;
	memset(&cdmmultidispense, 0, sizeof(cdmmultidispense));

	// Request Amount
	UINT nRequestedAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strRequestAmount);
	UINT nDispensedAmount = 0;

	LOG(Info, _T("Requested Amount : %d"), nRequestedAmount);

	int nResult = CDM_FAILED_TO_SEND_COMMAND;

	// 방출 전 Power Off를 고려하여 Reversal Flag를 설정한다. (Reason은 Incorrect Dispense)
	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_INCORRECT_DISPENSE;
	CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfo();

	// 출금 Mix 적용
	nResult = CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_DispenseByAmount(nRequestedAmount, &cdmmultidispense, &nDispensedAmount);

	if(NO_ERROR != nResult)
	{
		LOG(Error, _T("Withdrawal - Failed to withdrawal"));

		int nIndex = 0;
		CString strTemp;

		nIndex = (int)(cdmmultidispense.error_cd);

		LOG(Error, _T("CDM ERROR = %d"), nIndex);
		strTemp = CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError();
		CEagleSVCLib::GetInstance()->Set_ErrorCode(strTemp, ERROR_DEVICE_CDM);

		// Get Error Info
		LOG(Error, _T("CDM ERROR = %s"), strTemp);

		// WDM 방출중 장애 발생시에는 명세표를 무조건 Print하도록 로직 보완
		// 명세표 선택 Flag Set
		CEagleDataManager::GetInstance()->m_Client_Info.m_bIsPrintReceipt = TRUE;
	}

	LOG(Info, _T("CDM Result Information : error code - 0x%02X, reject code - 0x%02X"), cdmmultidispense.error_cd, cdmmultidispense.reject_cd);
	LOG(Info, _T("CDM Result Information : Count - 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X"), cdmmultidispense.count[0], cdmmultidispense.count[1], cdmmultidispense.count[2], 
																									 cdmmultidispense.count[3], cdmmultidispense.count[4], cdmmultidispense.count[5]);
	// Get Sensor Info
	LOG(Info, _T("CDM Sensor : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X"), cdmmultidispense.sensor[0], cdmmultidispense.sensor[1], cdmmultidispense.sensor[2], cdmmultidispense.sensor[3],
																						  cdmmultidispense.sensor[4], cdmmultidispense.sensor[5], cdmmultidispense.sensor[6], cdmmultidispense.sensor[7]);
#if (EMULATION_CDM_DEVICE)
	nDispensedAmount = nRequestedAmount;
#endif

	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount = CUtil::IntToString(nDispensedAmount);
	CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfo();

	LOG(Info, _T("CDM Dispensed Amount = %s"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount);

	// Log Dispensed count
	LOG(Info, _T("Withdrawal - Dispensed: CST1=%d, CST2=%d, CST3=%d, CST4=%d"), cdmmultidispense.count[0], cdmmultidispense.count[1], cdmmultidispense.count[2], cdmmultidispense.count[3]);

	// Get Last Dispense Info
	CDMLASTDISPENSE last_dispense_info;
	memset(&last_dispense_info, 0x00, sizeof(CDMLASTDISPENSE));

	// FW에서는 Dispense 명령을 받았을 때, LastDispense를 초기화 하기 때문에,
	// Dispense 명령의 CMD전송 실패시에는 LastDispense 명령을 수행하지 않는다.(이전 data가 올라올 수 있으므로)
	// Mix 실패인 경우도 추가 (2021.03.08)
	//if(CDM_FAILED_TO_SEND_COMMAND != nResult)
	if (!(CDM_FAILED_TO_SEND_COMMAND == nResult || WFS_NOTDISPENSABLE == nResult))
		CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_LastDispense(&last_dispense_info);

	LOG(Info, _T("LastDispense: cmd=0x%02X, cbx=%d, errcode=0x%02X"), last_dispense_info.last_cmd, last_dispense_info.last_cbx, last_dispense_info.last_error_cd);
	LOG(Info, _T("LastDispense: Disp[0]=%d, Rjt[0]=%d, Pick[0]=%d"), last_dispense_info.last_dispense_count[0], last_dispense_info.last_divert_count[0], last_dispense_info.last_pick_count[0]);
	LOG(Info, _T("LastDispense: Disp[1]=%d, Rjt[1]=%d, Pick[1]=%d"), last_dispense_info.last_dispense_count[1], last_dispense_info.last_divert_count[1], last_dispense_info.last_pick_count[1]);
	LOG(Info, _T("LastDispense: Disp[2]=%d, Rjt[2]=%d, Pick[2]=%d"), last_dispense_info.last_dispense_count[2], last_dispense_info.last_divert_count[2], last_dispense_info.last_pick_count[2]);
	LOG(Info, _T("LastDispense: Disp[3]=%d, Rjt[3]=%d, Pick[3]=%d"), last_dispense_info.last_dispense_count[3], last_dispense_info.last_divert_count[3], last_dispense_info.last_pick_count[3]);

	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispensedCount.Format(_T("%02d %02d %02d %02d"), last_dispense_info.last_dispense_count[0], last_dispense_info.last_dispense_count[1], last_dispense_info.last_dispense_count[2], last_dispense_info.last_dispense_count[3]);
	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strRejectedCount.Format(_T("%02d %02d %02d %02d"), last_dispense_info.last_divert_count[0], last_dispense_info.last_divert_count[1], last_dispense_info.last_divert_count[2], last_dispense_info.last_divert_count[3]);

	for(i=0; i<MAX_CASSETTE; i++)
	{
		if(last_dispense_info.last_divert_count[i] > 0)
		{
			bReject = TRUE;
			break;
		}
	}
	
	if(TRUE == bReject)
		CEagleSVCLib::GetInstance()->UpdateRejectInfo();

	if (nDispensedAmount > 0)
	{
		// Sum Withdrawal Transaction
		CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.SumData(S_WITHDRAWAL, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount);
		CEagleDataManager::GetInstance()->m_Config.SaveTotalInfo();		// File 저장 로직 누락 보완
	}

	if (nDispensedAmount >= nRequestedAmount)
	{
		// 정상 방출이므로 Reversal Flag 초기화
		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = 0;
		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal.Empty();
		CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfo();

		// Sum Withdrawal Transaction - 로직 공용화
		//CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.SumData(S_WITHDRAWAL, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount);
		//CEagleDataManager::GetInstance()->m_Config.SaveTotalInfo();		// File 저장 로직 누락 보완

		if (CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount) > 0)
		{
			// 현금 램프 ON
			CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_3, DIO_CMD_FLICKING);

			// Get Cash 화면 표시
			Client_Screen_GetCash();

			// 현금 램프 OFF
			CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_3, DIO_CMD_OFF);
		}

		// 명세표 Print 또는 View
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsPrintReceipt == TRUE)
		{
			// Print Receipt
			Client_PrintReceipt();
		}
		else
		{
			// 거래 종료 화면
			Client_Screen_TransactionComplete();
		}
	}
	else
	{
		// 로직 공용화
		//if (nDispensedAmount > 0)
		//{
		//	// Partial인 경우에도 거래 Count 및 금액 증가
		//	CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.SumData(S_WITHDRAWAL, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount);
		//	CEagleDataManager::GetInstance()->m_Config.SaveTotalInfo();		// File 저장 로직 누락 보완
		//}

		CEagleSVCLib::GetInstance()->Set_ErrorCode(CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError(), ERROR_DEVICE_CDM);

		// Dispensing중 Error 발생 Notice 표시
		Client_Screen_ErrorNotice(GETTEXT_011);

		if (nDispensedAmount > 0)
		{
			// 현금 램프 ON
			CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_3, DIO_CMD_FLICKING);

			// Get Cash 화면 표시
			Client_Screen_GetCash();

			// 현금 램프 OFF
			CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_3, DIO_CMD_OFF);
		}

		// Reversal Process
		Client_ReversalProcessing();
	}

	// WDM이 정상이더라도 Reject이 발생하는 경우 WDM Log를 저장하도록 로직 보완
	if ((bReject == TRUE) && (nResult == NO_ERROR))
	{
		LOG(Info, _T("WDM Reject is occurred - try to get WDM Log"));

		if (CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_GetDispenseLog() == TRUE)
		{
			CDEV_Manager::GetInstance()->m_DEV_CDM.m_bNeedToRejectLog = TRUE;
			LOG(Info, _T("WDM Reject is occurred - backup flag set for WDM Log"));
		}
	}


	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - Reversal Processing
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_ReversalProcessing()
{
	int nResult = 0;
	CString strTemp;

#if (NETWORK_OFFLINE_MODE)
	// HOST 송 / 수신 정상 처리

	// 출금 중 장애이므로 명세표 무조건 Print
	CEagleDataManager::GetInstance()->m_Client_Info.m_bIsPrintReceipt = TRUE;	// 강제로 명세표 선택으로 Flag Set

#else
	if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal == ENABLE_REVERSAL)
	{
		Client_Screen_Processing(1);

		CEagleTritonMsg::GetInstance()->Triton_MakeHostMsg(EAGLE_TRAN_CODE_REVERSAL);

		nResult = CEagleSVCLib::GetInstance()->SendRecvHost(EAGLE_TRAN_CODE_REVERSAL);

		if (nResult != SUCCESS)
		{
			LOG(Error, _T("Reversal Processing is failed"));
			CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetReversalError = TRUE;

			// Reversal 실패+ WDM 방출 실패시 Recovery에서 WDM Error Code가 제거되는 현상 방지를 위해 Network Error는 제거하도록 로직 보완
			CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice &= ~(ERROR_DEVICE_NET);
			LOG(Info, _T("Remove Network Error due to reversal failed"));
		}
		else
		{
			LOG(Error, _T("Reversal Processing is successful"));
		}
	}
#endif

	// 명세표 Print 또는 View
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsPrintReceipt == TRUE)
	{
		// Print Receipt
		CSVC_Manager::GetInstance()->Client_PrintReceipt();
	}
	else
	{
		// 거래 종료 화면
		Client_Screen_TransactionComplete();
	}

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 영수증 출력
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_PrintReceipt(void)
{

	// 영수증 출력 시작 메시지 통지
	Client_Screen_PrintReceipt();

	// 영수증 출력 장애 여부 확인
	BOOL bIsPrintError = FALSE;

	// 영수증 장애 코드
	int			 nPrintError = ERROR_SUCCESS;
	int			nOrgX = 40;

	CStringA	szTemp;

	CString		strTemp, strTemp2, strTemp3;
	CString		strTransType;

	// Print 상태가 Error인 경우 FALSE로 Return 처리
	if (CEagleSVCLib::GetInstance()->IsPTR_Available() == FALSE)
	{
		Client_Screen_ErrorNotice(GETTEXT_017);

		LOG(Error, _T("PrintReceipt - Receipt is not Available"));

		Client_Screen_TransactionComplete();

		return TRUE;
	}

	// 프린터 램프 ON
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_FLICKING);

	// PTR Quiry Status Stop
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();

	strTransType = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType;

	{
		if (CEagleSVCLib::GetInstance()->Set_PTR_Config() == FALSE)
		{
			LOG(Error, _T("Set PTR Config is failed"));
			bIsPrintError = TRUE;
		}

//#if(ENABLE_RECEIPT_HEADER_IMAGE_PRINT)
		// Print Logo Image
		nPrintError = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_PrintBitmapFlash(EAGLE_PRT_TYPE_USB, 1, nOrgX, EAGLE_PRT_BITMAP_PRINT_NORMAL);
		
		if (EAGLE_PRT_SUCCESS != nPrintError)
		{
			bIsPrintError = TRUE;
			LOG(Info, _T("PRT - PTR_S_PrintBitmapFlash Failed - %d"), nPrintError);
		}
//#else
//		// TEXT로 TRANSACTION RECORD 문구 출력
//		//             123456789012345678901234567890123456789012345
//		szTemp.Format("*********************************************");
//		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
//		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
//		LOG(Info, _T("PRT - %S"), szTemp);
//
//		//             123456789012345678901234567890123456789012345
//		szTemp.Format("             TRANSACTION RECORD              ");
//		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
//		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
//		LOG(Error, _T("PRT - %S"), szTemp);
//
//		//             123456789012345678901234567890123456789012345
//		szTemp.Format("*********************************************");
//		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
//		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
//		LOG(Info, _T("PRT - %S"), szTemp);
//
//#endif

		int i=0;

		szTemp.Format("============================================");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);

		// Location Message
		BOOL bExistMsg = FALSE;

		for(i=0; i<4; i++)
		{
			strTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strLocation_message[i];
			strTemp.TrimLeft();

			if (strTemp.GetLength() > 0)
			{
				//szTemp.Format("%S", CEagleDataManager::GetInstance()->m_Config.m_Message.strLocation_message[i]);
				szTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strLocation_message[i];
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

				szTemp.Replace("%", "%%");
				LOG(Info, _T("PRT - %S"), szTemp);

				bExistMsg = TRUE;
			}
		}

		if (bExistMsg == TRUE)
		{
			szTemp.Format("--------------------------------------------");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}

		// Terminal ID
		szTemp.Format("%-17.17S = %S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("TERMINAL ID")),  CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);


		// SEQUENCE NO
		szTemp.Format("%-17.17S = %S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("SEQUENCE NO.")), CEagleDataManager::GetInstance()->m_Config.m_System.strSequence_Number);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);		
		LOG(Info, _T("PRT - %S"), szTemp);

		// CARD NUMBER
		szTemp.Format("%-17.17S = %S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("CARD NO.")), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strShowCardNumber);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		// AID NAME & AID
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction == TRUE)
		{
			CString strAID, strAIDLabel;
			strAID = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_str_AID;
			strAIDLabel = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_str_AID_Label;

			if (strAID.GetLength() > 17 || strAIDLabel.GetLength() > 20)
			{
				szTemp.Format("AID = %S", strAID);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);		
				LOG(Info, _T("PRT - %S"), szTemp);

				szTemp.Format("%-6.6S%S", _T(""), strAIDLabel);
			}
			else
			{
				szTemp.Format("%-17.17S = %S", strAID, strAIDLabel);
			}

			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);		
			LOG(Info, _T("PRT - %S"), szTemp);
		}

		// AUTH NO
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strAutorizationNumber;
		if (strTemp.IsEmpty() == FALSE)
		{
			szTemp.Format("%-17.17S = %S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("AUTH NO")), strTemp);
		}
		else
		{
			szTemp.Format("%-17.17S =", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("AUTH NO")));
		}
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);


		// DATE / TIME - HOST 수신 Data
		// 날짜 표기 형식 : MM/DD/YYYY
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransactionDate;		// Host 수신 Format은 MMDDYY
		strTemp2 = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransactionTime;		// Host 수신 Format은 HHMMSS

		if (strTemp.IsEmpty() == FALSE)
		{
			szTemp.Format("%-17.17S = %S/%S/%S %S:%S:%S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("DATE / TIME")), 
														  strTemp.Left(2), strTemp.Mid(2,2), _T("20") + strTemp.Right(2), strTemp2.Left(2),strTemp2.Mid(2,2), strTemp2.Right(2));
		}
		else
		{
			szTemp.Format("%-17.17S = ", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("DATE / TIME")));
		}

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);


		// BUSINESS DATE
		// 날짜 표기 형식 : MM/DD/YYYY
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strBusinessDate;		// Host 수신 Format은 MMDDYY
		if (strTemp.IsEmpty() == FALSE)
		{
			szTemp.Format("%-17.17S = %S/%S/%S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("BUSINESS DATE")), 
												 strTemp.Left(2), strTemp.Mid(2,2), _T("20") + strTemp.Right(2));
		}
		else
		{
			szTemp.Format("%-17.17S = ", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("BUSINESS DATE")));
		}

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);


		// ACCOUNT
		if (strTransType == S_TRANSFER)
		{
			szTemp.Format("%-17.17S = %S %S %S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("ACCOUNT")),  
												 CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strFromAccountType,
												 CSCR_Manager::GetInstance()->GetAPTextIDString(_T("TO")),
												 CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strToAccountType);
		}
		else
		{
			szTemp.Format("%-17.17S = %S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("ACCOUNT")), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strFromAccountType);
		}
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);


		// TRANSACTION	[DCC]
		szTemp.Format("%-17.17S = %S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("TRANSACTION")), CSCR_Manager::GetInstance()->GetAPTextIDString(strTransType));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		szTemp.Format("--------------------------------------------");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
		
		if (strTransType == S_WITHDRAWAL)
		{
			// 요청 금액
			szTemp.Format("%-17.17S = %S%S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("REQUESTED")), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strRequestAmount + _T("00")));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);


			// 출금 금액
			szTemp.Format("%-17.17S = %S%S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("DISPENSED")), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount + _T("00")));
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}
		else if (strTransType == S_TRANSFER)
		{
			// 이체 금액
			if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE)
			{
				szTemp.Format("%-17.17S = %S%S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("TRANSFER")), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransferAmount));
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
				LOG(Info, _T("PRT - %S"), szTemp);
			}
		}

		// 수수료 (Partial Dispense인 경우에도 수수료 부과 - USA)
		int nFeeAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount);
		int nDispensedAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount);

		if (nFeeAmount > 0)
		{
			if (strTransType == S_WITHDRAWAL)
			{
				if (nDispensedAmount > 0)
				{
					szTemp.Format("%-17.17S = %S%S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("ATM FEE")), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount));
					CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
					CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
					LOG(Info, _T("PRT - %S"), szTemp);

					// ATM FEE Paid To는 Balance 및으로 이동
					//szTemp.Format("%-S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("ATM FEE PAID TO")));
					//CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
					//CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
					//LOG(Info, _T("PRT - %S"), szTemp);

					//szTemp.Format("%-S", CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_owner);
					//CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
					//CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
					//LOG(Info, _T("PRT - %S"), szTemp);

				}
			}
			//else if (strTransType == S_BALANCEINQUIRY)	// USA는 수수료 미부과하여 임시 주석 처리
			//{
			//	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE)	// 조회시에는 장애가 없는 경우에만 수수료 부과
			//	{
			//		szTemp.Format("%-17.17S = %S%S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("ATM FEE")), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount));
			//		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			//		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			//		LOG(Info, _T("PRT - %S"), szTemp);
			//	}
			//}
		}

		// 잔액
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strBalance;
		if ((strTemp.GetLength() > 0) && (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE))
		{
			// 가능 잔액
			if (strTemp.GetAt(0) == '-')
			{
				strTemp = strTemp.Mid(1);
				szTemp.Format("%-17.17S = - %S%S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("BALANCE")), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp));
			}
			else
			{
				szTemp.Format("%-17.17S = %S%S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("BALANCE")), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp));
			}

			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}

		// 가능 잔액
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strAvailableBalance;
		if ((strTemp.GetLength() > 0) && (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE))
		{
			if (strTemp.GetAt(0) == '-')
			{
				strTemp = strTemp.Mid(1);
				szTemp.Format("%-17.17S = - %S%S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("AVAILABLE BALANCE")), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp));
			}
			else
			{
				szTemp.Format("%-17.17S = %S%S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("AVAILABLE BALANCE")), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp));
			}
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);			LOG(Info, _T("PRT - %S"), szTemp);
		}

		// ATM FEE PAID TO
		if (strTransType == S_WITHDRAWAL)
		{
			if ((nFeeAmount > 0) && (nDispensedAmount > 0))
			{
				// Check Surcharge Owner Data exist
				strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_owner;
				strTemp.TrimLeft();

				if (strTemp.GetLength() > 0)
				{
					szTemp.Format("%-S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("ATM FEE PAID TO")));
					CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
					CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
					LOG(Info, _T("PRT - %S"), szTemp);

					szTemp.Format("%-S", CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_owner);
					CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
					CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
					LOG(Info, _T("PRT - %S"), szTemp);
				}
			}
		}

		// Check Marketing exist
		bExistMsg = FALSE;
		for(i=0; i<4; i++)
		{
			strTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strMarketing_message[i];
			strTemp.TrimLeft();

			if (strTemp.GetLength() > 0)
			{
				bExistMsg = TRUE;
				break;
			}
		}

		if (bExistMsg == FALSE)
		{
			// Check Print Text Data exist
			if (CEagleTritonMsg::GetInstance()->m_strarrPrintTextData.GetSize() > 0)
				bExistMsg = TRUE;
		}

		if (bExistMsg == TRUE)
		{
			szTemp.Format("--------------------------------------------");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}

		// Marketing Message
		for(i=0; i<4; i++)
		{
			strTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strMarketing_message[i];
			strTemp.TrimLeft();

			if (strTemp.GetLength() > 0)
			{
				// % Print를 위한 처리 - Bug Fix
				//szTemp.Format("%S", CEagleDataManager::GetInstance()->m_Config.m_Message.strMarketing_message[i]);
				szTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strMarketing_message[i];
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

				szTemp.Replace("%", "%%");
				LOG(Info, _T("PRT - %S"), szTemp);
			}
		}

		// Print Text Field ('p')
		for(i=0; i<CEagleTritonMsg::GetInstance()->m_strarrPrintTextData.GetSize(); i++)
		{
			strTemp = CEagleTritonMsg::GetInstance()->m_strarrPrintTextData[i];
			strTemp.TrimLeft();

			if (strTemp.GetLength() > 0)
			{
				szTemp.Format("%S", CEagleTritonMsg::GetInstance()->m_strarrPrintTextData[i]);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

				szTemp.Replace("%", "%%");
				LOG(Info, _T("PRT - %S"), szTemp);
			}

		}

		// 에러
		if (TRUE == CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError)
		{
			szTemp.Format("%S = %S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("ERROR")), CEagleSVCLib::GetInstance()->Get_ErrorCode());
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);

			// Error Message가 있는 경우에는 Error Message 출력
			if (CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorMsg.IsEmpty() == FALSE)
			{
				szTemp.Format("%S", CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorMsg);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
				LOG(Info, _T("PRT - %S"), szTemp);
			}
		}

		// Reversal 에러
		if (TRUE == CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetReversalError)
		{
			szTemp.Format("%S = %S", CSCR_Manager::GetInstance()->GetAPTextIDString(_T("ERROR")), ERROR_REVERSAL_FAILED);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}

		szTemp.Format("============================================");
		nPrintError = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		if (nPrintError != EAGLE_PRT_SUCCESS)
		{
			bIsPrintError = TRUE;
			LOG(Error, _T("PTR_S_Textout (%d)"), nPrintError);
		}

		// JOURNAL NO
		strTemp.Format(_T("JNL NO. : %d"), CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastJournalNo() + 1);
		szTemp.Format("%40.40S", strTemp);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		nPrintError = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_CutPaper(EAGLE_PRT_TYPE_USB, 1, 0);

		if (nPrintError != EAGLE_PRT_SUCCESS)
		{
			bIsPrintError = TRUE;
			LOG(Error, _T("PTR_CutPaper (%d)"), nPrintError);
		}
	}

	// PTR Quiry Status Start
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();

	// Print command가 Async라 3초 Delay 처리
	if (CEagleSVCLib::GetInstance()->IsPTR_Available(TRUE) == FALSE)
	{
		bIsPrintError = TRUE;
	}
	else
	{
		// SNBC에서 제공한 API에서 에러가 발생해도 Status가 정상이면 정상 처리
		bIsPrintError = FALSE;
	}

	// 프린터 램프 OFF
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_OFF);

	// 화면 표시 여부 Flag 처리
	BOOL bScreenReceipt = FALSE;

	if ((strTransType == S_BALANCEINQUIRY) || (strTransType == S_TRANSFER))
	{
		// 조회나 이체 거래 중 명세표 장애시 Screen Receipt 처리
		if ((bIsPrintError == TRUE) && (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE))
		{
			// 명세표 장애이고 Host Error가 없을 경우에 한해 Balance 정보를 화면에 표시함.
			bScreenReceipt = TRUE;
		}
	}

	if (bIsPrintError == TRUE)
	{
		// 추후 조회 거래시 수수료 부과시에는 Reversal 처리 추가 필요
		Client_Screen_ErrorNotice(GETTEXT_017);

		// 거래 중 Print Error시 Recovery를 위해 Error Set
		CEagleSVCLib::GetInstance()->Set_ErrorCode(_T("P00001"), ERROR_DEVICE_PTR);

		LOG(Error, _T("Print Error during transaction printing - %08X"), CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice);

	}

	// Print중 Error 발생시 조회와 이체거래에 대해서는 화면 Display하도록 함
	if (bScreenReceipt == TRUE)
	{
		// Screen Receipt
		CSVC_Manager::GetInstance()->Client_ScreenReceipt();
	}
	else
	{
		Client_Screen_TransactionComplete();
	}

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 화면 영수증 표시
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_ScreenReceipt(void)
{
	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	if (Is_DIO_Ear_Jack_Insert() == FALSE)
	{
		// Key Enable
		CrypteraEPP_EnterClearTextMode();

		// 화면 영수증 표시 메시지 통지
		Client_Screen_DisplayScreenReceipt();

		while (TRUE)
		{
			CheckService();		// EPP Timeout 시 재명령 처리

			if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
			{
				CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);
				break;
			}

			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
		}

		// TIMEOVER 또는 ENTER 입력 후 무조건 COMPLETE로 이동하므로 Key 처리 안함
	}

	// 거래 종료 처리
	Client_Screen_TransactionComplete();

	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - Power off Reversal Processing
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Client_PowerOffReversalProcessing()
{
	int nRetryCnt = 0;
	int nResult = 0;

#if (NETWORK_OFFLINE_MODE)
	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = 0;

	// Save Last Transaction Information
	CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfo();

	// Save Journal
	CEagleSVCLib::GetInstance()->SaveEJL_TransactionInfo();

	return;
#endif

	if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal == ENABLE_REVERSAL)
	{

		LOG(Info, _T("Execute Power Off Reversal"));

		CEagleDataManager::GetInstance()->m_Client_Info.m_bPowerOffFlag = TRUE;

		Client_Screen_Processing(1);

		CEagleTritonMsg::GetInstance()->Triton_MakeHostMsg(EAGLE_TRAN_CODE_REVERSAL);

		nResult = CEagleSVCLib::GetInstance()->SendRecvHost(EAGLE_TRAN_CODE_REVERSAL);

		if (nResult != SUCCESS)
		{
			LOG(Error, _T("Power off Reversal Processing is failed"));
			CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetReversalError = TRUE;
		}
		else
		{
			LOG(Error, _T("Power off Reversal Processing is successful"));
		}

		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = 0;

		// Save Last Transaction Information
		CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfo();

		// Save Journal
		CEagleSVCLib::GetInstance()->SaveEJL_TransactionInfo();

	}
}


/** **********************************************************
*	@brief		고객용 커맨드 - EMV - Remove Card
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Client_EMV_Remove_Card()
{
	BOOL bShowScreen = TRUE;

	BOOL	bADA_StartToTimeout = FALSE;
	BOOL	bADAMode = Is_DIO_Ear_Jack_Insert();

	m_strScrOutName.Empty();
	m_strScrOutData.Empty();

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.GetMediaStatus() & CARD_REAR_DETECT)
	{
		// IC Power Off 처리
		CDEV_Manager::GetInstance()->m_DEV_CDR.Initialize();

		// 카드 리더
		// FLICKER ON
		CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_FLICKING);

		if (bADAMode == FALSE)
			PlaySound(CUtil::GetNoticeWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);

		// Win32 simulator: stands in for the customer taking the card back.
		CDEV_Manager::GetInstance()->m_DEV_CDR.ShowRemoveCardWait(TRUE);

		while (TRUE)
		{
			CheckService();		// EPP Timeout 시 재명령 처리

			if (bShowScreen == TRUE)
			{
				Client_Screen_EMV_RemoveCard(bADAMode);

				if (bADAMode == TRUE)
				{
					g_TimeOutCheck.SetTargetTimeAfterSec(ADA_TIMEOUT_INFINITE);	// 우선 무한으로 설정
					bADA_StartToTimeout = TRUE;
				}

				bShowScreen = FALSE;
			}

			if (bADAMode == TRUE)
			{
				if((Is_DIO_Ear_Jack_Insert() == FALSE) || (g_TimeOutCheck.IsElapsedTimes() == TRUE))
				{
					LOG (Error, _T("Proc_Client_EMV_Remove_Card - Ear Jack FALSE or Timeout - return"));

					CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
					CDEV_Manager::GetInstance()->m_DEV_CDR.ShowRemoveCardWait(FALSE);

					return FALSE;
				}

				if ((CDEV_Manager::GetInstance()->IsPlayingWaveFile() == FALSE) && (bADA_StartToTimeout == TRUE))
				{
					LOG(Info, _T("Proc_Client_EMV_Remove_Card - ADA Timeout Set"));

					g_TimeOutCheck.SetTargetTimeAfterSec(CLIENT_SCREEN_TIMEOUT);	// ADA Timeout 설정
					bADA_StartToTimeout = FALSE;
				}
			}

			if ((CDEV_Manager::GetInstance()->m_DEV_CDR.GetMediaStatus() & CARD_REAR_DETECT) == 0x00)
			{
				// Card 제거 감지
				CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
				CDEV_Manager::GetInstance()->m_DEV_CDR.ShowRemoveCardWait(FALSE);

				break;
			}

			if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
			{
				CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

				// TIMEOUT만 Check함
				CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
				CDEV_Manager::GetInstance()->m_DEV_CDR.ShowRemoveCardWait(FALSE);

				return FALSE;
			}

			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
		}
	}

	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_2, DIO_CMD_OFF);
	return TRUE;
}


/** **********************************************************
*	@brief		고객용 커맨드 - 수수료 부과 여부 확인
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Client_NextStep_CheckSurcharge()
{
	int nFeeAmount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount);

	if (CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_type == _T("0"))
	{
		if ((nFeeAmount == 0) || (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsBlockBin == TRUE))
		{
			// 수수료 미 부과
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount = _T("0");

			Proc_Client_NextStep_CheckPrinterStatus();
		}
		else
		{
			// 수수료 확인
			Client_Confirm_Surcharge();
		}
	}
	else
	{
		if (((CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawal_PercentSurcharge_Option == _T("0")) && (nFeeAmount == 0)) ||
			(CEagleDataManager::GetInstance()->m_Client_Info.m_bIsBlockBin == TRUE))
		{
			// 수수료 미 부과
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount = _T("0");

			// 출금 거래 중 명세표 장애시 분기 처리
			Proc_Client_NextStep_CheckPrinterStatus();
		}
		else
		{
			// 수수료 확인
			Client_Confirm_Surcharge();
		}
	}
}


/** **********************************************************
*	@brief		고객용 커맨드 - 수수료 부과 여부 확인
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Client_NextStep_CheckPrinterStatus()
{
	// 출금 거래 중 명세표 장애시 분기 처리 (현재 Device에서 처리가 안되서 임시 주석 처리)
	if (CEagleSVCLib::GetInstance()->IsPTR_Available() == FALSE)
	{
		Client_Confirm_ReceiptError();				
	}
	else
	{
		Client_Select_Receipt();
	}
}


/** **********************************************************
*	@brief		고객용 커맨드 - 명세표 선택 후 거래별로 추가 처리 확인
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Client_NextStep_CheckSelectReceipt()
{
	CString strTransType = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType;

	if (strTransType == S_BALANCEINQUIRY || strTransType == S_TRANSFER)	// 조회와 이체는 화면에 거래 내역 표시
	{
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsPrintReceipt == FALSE)
		{
			// 화면 영수증 확인
			Client_Confirm_ScreenReceipt();
		}
		else
		{
			// HOST 송/수신
			Client_Host_Processing();
		}
	}
	else
	{
		// HOST 송/수신
		Client_Host_Processing();
	}
}


/** **********************************************************
*	@brief		고객용 커맨드 - 명세표 선택 후 거래별로 추가 처리 확인
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Client_NextStep_CheckCompleteHostProcessing()
{
	CString strTransType = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType;

	if (strTransType == S_WITHDRAWAL)
	{
		// 현금 방출
		Client_Cash_Dispensing();
	}
	else
	{
		// 조회나 이체시에 거래 Count 증가
		CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.SumData(strTransType, _T("0"));
		CEagleDataManager::GetInstance()->m_Config.SaveTotalInfo();		// File 저장 로직 누락 보완

		// 명세표 Print 또는 View
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsPrintReceipt == TRUE)
		{
			// Print Receipt
			Client_PrintReceipt();
		}
		else
		{
			// 조회 이체시에는 화면에 표시
			// Screen Receipt
			CSVC_Manager::GetInstance()->Client_ScreenReceipt();
		}
	}
}


/** **********************************************************
*	@brief		고객용 커맨드 - ADA 거래 중 고객이 Key를 잘못 눌렀을 경우 Wave File Guide 처리
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ADA_Guide_InvalidKey()
{
	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	CDEV_Manager::GetInstance()->AddWaveFile(INVALIDKEY_WAVE_FILE);
	CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);

	//CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);		// 1초 Delay
}


/** **********************************************************
*	@brief		고객용 커맨드 - ADA 거래 중 Volume up guide
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ADA_Guide_VolumeUp()
{
	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	//CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);
	CDEV_Manager::GetInstance()->AddWaveFile(VOLUMEUP_WAVE_FILE);
	CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);

	CDEV_Manager::GetInstance()->SetVolumeUp();

	//CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);		// 1초 Delay
}


/** **********************************************************
*	@brief		고객용 커맨드 - ADA 거래 중 Volume Down guide
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ADA_Guide_VolumeDown()
{
	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	//CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);
	CDEV_Manager::GetInstance()->AddWaveFile(VOLUMEDOWN_WAVE_FILE);
	CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);

	CDEV_Manager::GetInstance()->SetVolumeDown();

	//CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);		// 1초 Delay
}


/** **********************************************************
*	@brief		고객용 커맨드 - ADA 거래 중 repeat guide
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ADA_Guide_Repeat()
{
	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();

	//CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);
	CDEV_Manager::GetInstance()->AddWaveFile(REPEAT_WAVE_FILE);
	CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);

	//CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);		// 1초 Delay
}


/** **********************************************************
*	@brief		고객용 커맨드 - ADA 거래 중 Cancel Key 입력 Guide
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ADA_Guide_Cancel()
{
	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();
	//CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);
	CDEV_Manager::GetInstance()->AddWaveFile(CANCEL_WAVE_FILE);
	CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
}


/** **********************************************************
*	@brief		고객용 커맨드 - ADA 거래 중 Clear Key 입력 Guide
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ADA_Guide_Clear()
{
	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();
	//CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);
	CDEV_Manager::GetInstance()->AddWaveFile(CLEAR_WAVE_FILE);
	CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
}


/** **********************************************************
*	@brief		고객용 커맨드 - ADA 거래 중 Enter Key 입력 Guide
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ADA_Guide_Enter()
{
	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();
	//CDEV_Manager::GetInstance()->AddWaveFile(SELECTEDKEY_WAVE_FILE);
	CDEV_Manager::GetInstance()->AddWaveFile(ENTER_WAVE_FILE);
	CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
}


/** **********************************************************
*	@brief		고객용 커맨드 - ADA 거래 중 Enter Key 입력 Guide
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ADA_Guide_BalanceInfo(CString strBalance, CString strAvailBalance)
{
	// Stop Wave File
	CDEV_Manager::GetInstance()->ResetWaveFile();
	
	if (CUtil::StringToInt(strBalance) > 0)
	{
		CDEV_Manager::GetInstance()->AddWaveFile(BALANCE_INFO_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddAmountWaveFiles(CUtil::ConvertFromValueToAmountwithCent(strBalance));
	}

	if (CUtil::StringToInt(strAvailBalance) > 0)
	{
		CDEV_Manager::GetInstance()->AddWaveFile(AVAILABLE_BALANCE_INFO_WAVE_FILE);
		CDEV_Manager::GetInstance()->AddAmountWaveFiles(CUtil::ConvertFromValueToAmountwithCent(strAvailBalance));
	}
	
	CDEV_Manager::GetInstance()->PlayWaveFile(TRUE);
}
