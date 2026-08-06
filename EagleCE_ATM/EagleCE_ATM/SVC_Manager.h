#pragma once

#include <list>
#include <mmsystem.h>

#include "SVC_Define.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleDataManager.h"

#include "../../EagleCE_Framework/EagleCE_Framework/CircularQueue.h"
#include "../../EagleCE_Device/EagleCE_Device/DEV_Manager.h"


/** **********************************************************
* 서비스 관리자
************************************************************/
class CSVC_Manager : public CWnd
{

public:
	// 인스턴스
	static CSVC_Manager* m_pInstance;

	// 생성자
	CSVC_Manager(void);

	// 소멸자
	~CSVC_Manager(void);

public:
	// SVC_Manager.cpp

	// 인스턴스 생성
	static void CreateInstance();

	// 인스턴스 취득
	static CSVC_Manager* GetInstance();

	// 인스턴스 해제
	static void ReleaseInstance();


	// 초기화
	BOOL Initialize();

	// 종결화
	BOOL Finalize();

	// 통지 윈도우 설정
	void SetNotifyWnd(CWnd* pNotifyWnd);


	// 서비스 모드 취득 (IN SERVICE / OUT OF SERVICE)
	int GetServiceMode(void);

	// 서비스 모드 설정 (IN SERVICE / OUT OF SERVICE)
	void SetServiceMode(int nServiceMode);

	// 계원 권한 취득 (ADMINISTRATOR / MASTER / REPLENISHER)
	int GetOperatorMode(void);

	// 계원 권한 설정 (ADMINISTRATOR / MASTER / REPLENISHER)
	void SetOperatorMode(int nOperatorMode);

	// 서비스 시작
	static UINT RunService(LPVOID pParam);

	// Software Monitoring Thread
	static UINT SWMonitoring(LPVOID pParam);

	// ATM 상태 Monitoring Thread
	static UINT ATMAtstus_Monitoring(LPVOID pParam);

	// 서비스 실행
	void RunService(void);

	// 서비스 실행 - Remote Access 모드 실행
	BOOL RunService_RemoteAccess(BYTE byCmd);

	// 서비스 체크
	int CheckService(void);

	// 서비스 체크 - OUT OF SERVICE
	void SetOutOfService();

	// 서비스 체크 - IN SERVICE
	void SetInService(void);

	void SetOperatorService(void);

	void SetRemoteAccessService(void);

	// 연결
	BOOL Proc_Connect(void);

	// 해제
	BOOL Proc_Disconnect(void);

	// 초기화
	BOOL Proc_Device_Initialize(void);


	// 서비스 종료
	void StopService(void);

	// DIO - Front Door 열림
	BOOL Is_DIO_Front_Door_Open();

	// DIO - Safe Door 열림
	BOOL Is_DIO_Safe_Door_Open();

	// DIO - Upper Frame 열림
	BOOL Is_DIO_Upper_Frame_Open();

	// DIO - Ear Jack 삽입
	BOOL Is_DIO_Ear_Jack_Insert();

	// SVC_Manager_Client.cpp

	// 고객용 커맨드 - 고객 변경
	void Client_ChangeCustom(void);

	// 고객용 커맨드 - 카드 투입
	void Client_InsertCard(void);

	// 고객용 커맨드 - EMV 카드 읽기
	void CSVC_Manager::Client_EMV_CardRead(void);

	// 고객용 커맨드 - EMV AID LIST 작성
	void CSVC_Manager::Client_EMV_MakeAppLists(void);

	// 고객용 커맨드 - EMV AID 선택
	void CSVC_Manager::Client_EMV_FinalSelect(void);

	// 고객용 커맨드 - EMV Multi AID 선택
	void CSVC_Manager::Client_EMV_MultiAppSelect(void);

	// 고객용 커맨드 - EMV FAllback
	void CSVC_Manager::Client_EMV_FallBack(void);


	// 고객용 커맨드 - 카드 제거
	void Client_RemoveCard(void);

	// 고객용 커맨드 - 카드 읽기
	void Client_ReadCard(void);

	// 고객용 커맨드 - 카드 읽기
	void Client_SelectLanguage(void);

	// 고객용 커맨드 - PIN 읽기
	void Client_ReadPin(void);

	// 고객용 커맨드 - 거래 선택
	void Client_Select_Transaction(void);

	// 고객용 커맨드 - 계좌 선택
	void Client_Select_Accounts(void);

	// 고객용 커맨드 - 이체 계좌 선택
	void Client_Select_TransferAccounts(void);

	// 고객용 커맨드 - 금액 선택
	void Client_Select_Amount(void);

	// 고객용 커맨드 - 금액 확인 (ADA)
	void Client_ADA_ConfirmAmount(void);

	// 고객용 커멘드 - 수수료 표시
	void Client_Confirm_Surcharge(void);

	// 고객용 커맨드 - 출금 금액 입력
	void Client_Enter_WithdrawalAmount(void);

	// 고객용 커맨드 - 이체 계좌 선택
	void Client_Enter_TransferAmount(void);

	// 고객용 커멘드 - 영수증 선택
	void Client_Select_Receipt(void);

	// 고객용 커멘드 - 영수증 불가 화면
	void Client_Confirm_ReceiptError(void);

	// 고객용 커멘드 - 화면 영수증 확인
	void Client_Confirm_ScreenReceipt(void);

	// 고객용 커멘드 - HOST 송/수신
	void Client_Host_Processing(void);

	// 고객용 커멘드 - EMV Remove Card
	void Client_EMV_RemoveCard(void);

	// 고객용 커멘드 - 현금 방출
	void Client_Cash_Dispensing(void);

	// 고객용 커맨드 - 영수증 출력
	void Client_PrintReceipt(void);

	// 고객용 커맨드 - 영수증 화면 출력
	void Client_ScreenReceipt(void);

	// 고객용 커맨드 - Reversal 처리
	void Client_ReversalProcessing(void);

	// AMS 커맨드 - Accept OK
	void AMS_AcceptOK();

	// 서비스 실행 - 고객용 모드 실행
	BOOL RunService_Client(BYTE byCmd);

	// 고객용 커맨드 - 고객 변경
	BOOL Proc_Client_ChangeCustom(void);

	// 고객용 커맨드 - 카드 투입
	BOOL Proc_Client_InsertCard(void);

	// 고객용 커맨드 - EMV 카드 Read
	BOOL Proc_Client_EMV_CardRead(void);

	// 고객용 커맨드 - EMV Fallback
	BOOL Proc_Client_EMV_FallBack(void);

	// 고객용 커맨드 - EMV AID LIST 작성
	BOOL Proc_Client_EMV_MakeAIDList(void);

	// 고객용 커맨드 - Multi AID List 처리
	BOOL Proc_Client_EMV_MultiAppSelect(void);

	// 고객용 커맨드 - EMV AID LIST 작성
	BOOL Proc_Client_EMV_FromFinalAppToReadApp(void);

	// 고객용 커맨드 - 카드 제거
	BOOL Proc_Client_RemoveCard(void);

	// 고객용 커맨드 - 카드 읽기
	BOOL Proc_Client_ReadCard(void);

	// 고객용 커맨드 - 언어 선택
	BOOL Proc_Client_SelectLanguage(void);

	// 고객용 커맨드 - PIN 읽기
	BOOL Proc_Client_ReadPin(void);

	// 고객용 커맨드 - 거래 선택
	BOOL Proc_Client_SelectTransaction(void);

	// 고객용 커맨드 - 계좌 선택
	BOOL Proc_Client_SelectAccounts(void);

	// 고객용 커맨드 - 이체계좌 선택
	BOOL Proc_Client_SelectTransferAccounts(void);

	// 고객용 커맨드 - 출금 금액 선택
	BOOL Proc_Client_SelectAmount(void);

	// 고객용 커맨드 - 출금 금액 입력 
	BOOL Proc_Client_EnterAmount(void);

	// 고객용 커맨드 - ADA 금액 확인 
	BOOL Proc_Client_ADA_ConfirmAmount();

	// 고객용 커맨드 - 수수료 확인
	BOOL Proc_Client_ConfirmSurcharge(void);

	// 고객용 커맨드 - 이체 금액 입력
	BOOL Proc_Client_EnterTransferAmount(void);

	// 고객용 커맨드 - 명세표 선택
	BOOL Proc_Client_SelectReceipt(void);

	// 고객용 커맨드 - 명세표 불가 확인
	BOOL Proc_Client_Confirm_ReceiptError(void);

	// 고객용 커맨드 - 화면 명세표 확인
	BOOL Proc_Client_Confirm_ScreenReceipt(void);

	// 고객용 커맨드 - HOST 송/수신
	BOOL Proc_Client_HostProcessing(void);

	// 고객용 커맨드 - 현금 방출
	BOOL Proc_Client_CashDispensing(void);

	// 고객용 커맨드 - Reversal
	BOOL Proc_Client_ReversalProcessing(void);

	// 고객용 커맨드 - 영수증 출력
	BOOL Proc_Client_PrintReceipt(void);

	// 고객용 커맨드 - 화면 영수증 표시
	BOOL Proc_Client_ScreenReceipt(void);

	// 고객용 커맨드 - Power Off Reversal Processing
	void Proc_Client_PowerOffReversalProcessing(void);

	// 고객용 커맨드 - EMV - Remove Card
	BOOL Proc_Client_EMV_Remove_Card();

	// 고객용 커맨드 - Surcharge 부과 여부 확인 후 Next Step 결정
	void Proc_Client_NextStep_CheckSurcharge(void);

	// 고객용 커맨드 - Printer 상태 확인 후 Next Step 결정
	void Proc_Client_NextStep_CheckPrinterStatus(void);

	// 고객용 커맨드 - Printer 상태 확인 후 Next Step 결정
	void Proc_Client_NextStep_CheckSelectReceipt(void);

	// 고객용 커맨드 - Host 정상 송/수신 후 Next Step 결정
	void Proc_Client_NextStep_CheckCompleteHostProcessing(void);

	void Client_ADA_Guide_InvalidKey();

	void Client_ADA_Guide_VolumeUp();

	void Client_ADA_Guide_VolumeDown();

	void Client_ADA_Guide_Repeat();

	void Client_ADA_Guide_Cancel();

	void Client_ADA_Guide_Clear();

	void Client_ADA_Guide_Enter();

	void Client_ADA_Guide_BalanceInfo(CString strBalance, CString strAvailBalance);


	/*	Screen Display Test	*/

	// Voice Guidance Screen
	void Client_Screen_Initialize();

	// Voice Guidance Screen
	void Client_Screen_VoiceGuidance();

	// Insert Card
	void Client_Screen_InsertCard(BOOL bADAMode = FALSE);

	// EMV Processing
	void Client_Screen_EMVProcessing(BOOL bADAMode = FALSE);

	// Transaction Cancel
	void Client_Screen_TransactionCancel(BOOL bADAMode = FALSE);

	// EMV Fallback
	void Client_Screen_EMV_Fallback(BOOL bADAMode = FALSE);

	// Card Read Error
	void Client_Screen_CardReadError(BOOL bADAMode = FALSE);

	// EMV Select Multi AID
	void Client_Screen_EMV_SelectMultiAID(BOOL bADAMode = FALSE);

	// EMV Failed Selected AID
	void Client_Screen_ErrorNotice(CString strErrorMsg, CString strSubMsg = _T(""));

	// Select Language
	void Client_Screen_SelectLanguage(BOOL bADAMode = FALSE);

	// Enter PIN
	void Client_Screen_EnterPIN(BOOL bADAMode = FALSE);

	// Select Transaction
	CString Client_Screen_SelectTransaction(BOOL bADAMode = FALSE);

	// Select Account
	CString Client_Screen_SelectAccount(BOOL bADAMode = FALSE);

	// Select Transfer Account
	CString Client_Screen_SelectTransferAccount(BOOL bADAMode = FALSE);

	// Select Amount
	CString Client_Screen_SelectAmount(BOOL bADAMode = FALSE);

	// Enter Withdrawal Amount
	void Client_Screen_EnterWithdrawalAmount(BOOL bADAMode = FALSE);

	// Enter Withdrawal Amount
	CString Client_Screen_ConfirmAmount(BOOL bADAMode = FALSE);

	// Fee Notification
	CString Client_Screen_FeeNotification(BOOL bADAMode = FALSE);

	// Enter Transfer Amount
	void Client_Screen_EnterTransferAmount(BOOL bADAMode = FALSE);

	// Select Receipt
	CString Client_Screen_SelectReceipt(BOOL bADAMode = FALSE);

	// Confirm Receipt Error
	CString Client_Screen_ConfirmReceiptError(BOOL bADAMode = FALSE);

	// Confirm Screen Receipt
	CString Client_Screen_ConfirmScreenReceipt(BOOL bADAMode = FALSE);

	// Host Processing
	void Client_Screen_Processing(int nTransType = 0, BOOL bADAMode = FALSE);

	// EMV Remove Card
	void Client_Screen_EMV_RemoveCard(BOOL bADAMode = FALSE);

	// Cash Dispensing
	void Client_Screen_CashDispensing(BOOL bADAMode = FALSE);

	// Get Cash
	void Client_Screen_GetCash(BOOL bADAMode = FALSE);

	// Print Receipt
	void Client_Screen_PrintReceipt(BOOL bADAMode = FALSE);

	// Screen Receipt
	void Client_Screen_DisplayScreenReceipt(BOOL bADAMode = FALSE);

	// Transaction Complete
	void Client_Screen_TransactionComplete(BOOL bADAMode = FALSE);

	void AMS_Screen_RemoteProgress();


	// for Cryptera EPP
	BOOL CrypteraEPP_EnterClearTextMode();

	void CrypteraEPP_StopClearTextMode();

	// SVC_Manager_Operator.cpp

	// 계원용 커맨드 - Operator Initialzie
	void Operator_Initialize(void);

	// 계원용 커맨드 - 종료
	void Operator_Exit(void);


	// 서비스 실행 - 계원용 모드 실행
	BOOL RunService_Operator(BYTE byCmd);

	// 계원용 커맨드 - OP Main
	void Proc_Operator_Main(void);

	// 계원용 커맨드 - OP Exit
	void Proc_Operator_Exit(void);

	//계원용 커맨드 - Option
	void Proc_Operator_Option_Main();

	//계원용 커맨드 - Option - Transaction 
	void Proc_Operator_Option_Transaction();

	//계원용 커맨드 - Option - Amount Settings 
	void Proc_Operator_Option_AmountSettings();

	//계원용 커맨드 - Option - Configure General 
	void Proc_Operator_Option_ConfigureGeneral();

	//계원용 커맨드 - Option - Block ISO 
	void Proc_Operator_Option_BlockISO();

	//계원용 커맨드 - Option - AD Screen 
	void Proc_Operator_Option_AD_Screen();

	//계원용 커맨드 - Option - AD1 Preview
	void Proc_Operator_Option_AD1_Preview();
	
	//계원용 커맨드 - Option - AD2 Preview
	void Proc_Operator_Option_AD2_Preview();

	//계원용 커맨드 - Option - AD3 Preview
	void Proc_Operator_Option_AD3_Preview();

	//계원용 커맨드 - Option - AD4 Preview
	void Proc_Operator_Option_AD4_Preview();

	//계원용 커맨드 - Option - AD5 Preview
	void Proc_Operator_Option_AD5_Preview();

	//계원용 커맨드 - Option - AD6 Preview
	void Proc_Operator_Option_AD6_Preview();
	
	//계원용 커맨드 - Option - Message
	void Proc_Operator_Option_Message();

	//계원용 커맨드 - Option - Marketing Message
	void Proc_Operator_Option_MarketingMsg();

	//계원용 커맨드 - Option - Location Message
	void Proc_Operator_Option_LocationMsg();

	//계원용 커맨드 - Option - Attract Message
	void Proc_Operator_Option_AttractMsg();

	//계원용 커맨드 - Option - Farewell Message
	void Proc_Operator_Option_FarewellMsg();

	// 계원용 커맨드 - System
	void Proc_Operator_System_Main();

	// 계원용 커맨드 - Date Time
	void Proc_Operator_DateTime();

	// 계원용 커맨드 - Password
	void Proc_Operator_Password();

	// 계원용 커맨드 - Reboote
	void Proc_Operator_Reboot();

	// 계원용 커맨드 - Upload
	void Proc_Operator_Upload();

	// 계원용 커맨드 - BackUp
	void Proc_Operator_BackUp();

	// 계원용 커맨드 - BackUp
	void Proc_Operator_BackUpCaptureImage();

	void Proc_Operator_Device_Main();					///< 계원용 커맨드 - Device Main

	void Proc_Operator_Device_Camera();					///< 계원용 커맨드 - Device - Camera

	void Proc_Operator_Device_Camera_Capture();			///< 계원용 커맨드 - Device - Camera - Capture

	void Proc_Operator_Device_CardReader();				///< 계원용 커맨드 - Device - CardReader

//	void Proc_Operator_Device_CardReader_Reset();		///< 계원용 커맨드 - Device - CardReader - Reset

//	BOOL Proc_Operator_Device_CardReader_ICTest();		///< 계원용 커맨드 - Device - CardReader - IC Test

//	BOOL Proc_Operator_Device_CardReader_ScanTest();	///< 계원용 커맨드 - Device - CardReader - Scan Test

	BOOL Proc_Operator_Device_CardReader_InsertCard(BOOL bICCard);							///< CardReader - Insert Card

//	BOOL Proc_Operator_Device_CardReader_RemoveCard(BOOL bICCard, BOOL &bRemoveSuccess);	///< CardReader - Remove Card	
	BOOL Proc_Operator_Device_CardReader_RemoveCard(BOOL bICCard);	///< CardReader - Remove Card	

//	BOOL Proc_Operator_Device_CardReader_ReadData(BOOL bICCard, BOOL &bReadSuccess);		///< CardReader - ReadData
	BOOL Proc_Operator_Device_CardReader_ReadData(BOOL bICCard);							///< CardReader - ReadData

	BOOL Proc_Operator_Device_CardReader_LatchOption();										///< CardReader - Latch Option

	void Proc_Operator_Device_Sound();					///< 계원용 커맨드 - Device - Sound

	void Proc_Operator_Device_CashDispenser();			///< 계원용 커맨드 - Device - Cash Dispenser

	void Proc_Operator_Device_CashDispenser_Reset();			///< Cash Dispenser - TestDispense

	void Proc_Operator_Device_CashDispenser_TestDispense();		///< Cash Dispenser - TestDispense

	void Proc_Operator_Device_CashDispenser_CbxSetting();		///< Cash Dispenser - Cassette Setting

	void Proc_Operator_Device_CashDispenser_SetBill();			///< Cash Dispenser - Set Bill Parameters

	void Proc_Operator_Device_CashDispenser_LearnBill();		///< Cash Dispenser - Learn Bill Parameters

	void Proc_Operator_Device_SIU();					///< 계원용 커맨드 - Device - SIU

	void Proc_Operator_Device_PINPAD();					///< 계원용 커맨드 - Device - PinPad

	void Proc_Operator_Device_Printer();				///< 계원용 커맨드 - Device - Printer

	void Proc_Operator_Device_Printer_Reset();					///< Printer - Reset

	void Proc_Operator_Device_Printer_TestPrint();				///< Printer - Test Print

	void Proc_Operator_Device_TCPIP();					///< 계원용 커맨드 - Device - TCP/IP

	void Proc_Operator_Device_Modem();					///< 계원용 커맨드 - Device - Modem


	CString Operator_CDR_GetLatchOption();				///< CDR의 LatchOption 정보 가져옴.

	//BOOL ExecuteLatchOption(BOOL bLatchOn);			///< LatchOption 수행

	//BOOL IsUnSuccessReply(CString strErrorCode);		///< 준정상코드인지 판단 -> Device로 함수 변경


	// 계원용 커맨드 - Report Main
	void Proc_Operator_Report_Main();

	// SW Version 취득
	CString Get_SW_VersionInfo();

	// 계원용 커맨드 - Error Code
	void Proc_Operator_ErrorCode();

	// Print Parameter function
	BOOL Print_ATM_Parameters();


	// 계원용 커맨드 - Task Main
	void Proc_Operator_Task_Main();

	// 계원용 커맨드 - Add Quantity in Cassette
	void Proc_Operator_AddQTYinCST();

	// 계원용 커맨드 - Schedule Day Close
	void Proc_Operator_ScheduleDayClose();

	// 계원용 커맨드 - Host Main
	void Proc_Operator_Host_Main();

	// 계원용 커맨드 - NetworkSettings
	void Proc_Operator_NetworkSettings();

	// 계원용 커맨드 - Modem Settings
	void Proc_Operator_ModemSettings();

	// 계원용 커맨드 - IP Settings
	void Proc_Operator_IPSettings();

	// 계원용 커맨드 - Key Management
	void Proc_Operator_KeyManagement();

	// 계원용 커맨드 - Master Keys Management
	void Proc_Operator_MasterKeysManagement();

	// 계원용 커맨드 - PINPAD Activation
	void Proc_Operator_PNPAD_Activation();

	// 계원용 커맨드 - Inject Master Key
	void Proc_Operator_InjectMasterKey();

	// 계원용 커맨드 - Master Key KCV Info.
	void Proc_Operator_MasterKey_KCV_Info();

	// 계원용 커맨드 - Configure Connection
	void Proc_Operator_ConfigureConnection();

	// 계원용 커맨드 - Protocol Option
	void Proc_Operator_ProtocolOption();

	// 계원용 커맨드 - Host Phone Number
	void Proc_Operator_HostPhoneNumber();

	// 계원용 커맨드 - Host IP Address
	void Proc_Operator_HostIPAddress();

	// 계원용 커맨드 - Terminal Monitoring
	void Proc_Operator_TerminalMonitoring();

	// 계원용 커맨드 - EMV AID List
	void Proc_Operator_EMV_AIDList();
	
	// 계원용 커맨드 - Configure AMS (Modem)
	void Proc_Operator_ConfigureAMS_Modem();
	
	// 계원용 커맨드 - Configure AMS (TCP/IP)
	void Proc_Operator_ConfigureAMS_TCPIP();

	// 계원용 커맨드 - Journal
	void Proc_Operator_Journal_Main();

	// 계원용 커맨드 - View Journal
	void Proc_Operator_View_Journal();

	// 계원용 커맨드 - View Journal
	void Proc_Operator_View_Journal_Detail();

	// 계원용 커맨드 - Journal - Print Last X
	void Proc_Operator_Journal_PrintLastX(void);

	CString Get_JournalData(FLSEJCOMMINFO CommInfo, FLSEJINFO JnlInfo, CString strDelimiter = SCR_LINEFEED_DELEMITER, BOOL bMinimumData = FALSE);

	BOOL Operator_Input_Text(CString strTitle, CString strInText, CString &strOutText, int nMaxInNumber=40, CString strInputDefaultMode=_T("NUMBER"));		///< OP Text 입력방식

	BOOL Operator_Input_Number(CString strTitleText, CString strInText, CString &strOutText, int nMaxInNumber=20, CString strInputType=NORMAL_TYPE, CString strGuideText = _T(""));		///< OP Numeric 입력방식

	BOOL Operator_Input_HexaDecimal(CString strTitle, CString strInText, CString &strOutText, int nMaxInNumber=40);											///< OP Hexa 입력방식

	BOOL Operator_Confirm_Screen(CString strTitleText, CString strGuideText = _T(""), CString strImportantGuideText = _T(""), BOOL bUseSound = FALSE, BOOL bNormalForTimeout = FALSE);			///< OP Confirm 화면


	BOOL Operator_Processing_Screen(CString strProcessingText, BOOL bResultScreen=FALSE, CString strErrorCode=_T("000000"), BOOL bResultDisplay=TRUE);									///< OP 수행 Guide 화면

	void Operator_DisplayScreen(CString strScrNum, BOOL bKeyEnable = TRUE, CString strScreenInfo = _T(""), CString strButtonInfo = _T(""), CString strEditStateInfo = _T(""), BOOL FocusKeyActive = FALSE);						///< OP 화면 표시

	void Operator_UpdateScreen(CString strScreenInfo = _T(""), CString strButtonInfo = _T(""), BOOL FocusKeyActive = FALSE);																	///< OP 화면 갱신

	void Operator_UserPasswordChange_Screen(CString strCurrentUser);

//protected:
	DECLARE_MESSAGE_MAP()

public:

	// DIO 통지
	LRESULT OnDioNotify(WPARAM wParam, LPARAM lParam);

	// EPP 통지 - 미사용
	//LRESULT OnEppNotify(WPARAM wParam, LPARAM lParam);

	// PRT 통지
	LRESULT OnPrtNotify(WPARAM wParam, LPARAM lParam);

	// USBMEM 통지
	LRESULT OnUsbMemNotify(WPARAM wParam, LPARAM lParam);

	// AMS 통지
	LRESULT OnAMSNotify(WPARAM wParam, LPARAM lParam);

	// Cryptera EPP 통지
	int PressKeyHandler(unsigned short usParam, byte *lpByte, unsigned short usDataLength);
	int StatusEventHandler(unsigned short usParam, BYTE *lpByte, unsigned short usDataLength);


public:
	// 디바이스 변경
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);


public:
	// 통지 윈도우
	CWnd* m_pNotifyWnd;

	// 서비스 락
	CCriticalSection m_ServiceLock;

	// 서비스 스레드 핸들
	HANDLE m_hServiceThread;

	// SW Monitor 스레드 핸들
	HANDLE m_hSWMonitorThread;

	// ATM 상태 Monitor 스레드 핸들
	HANDLE m_hATMStatus_MonitorThread;

	// 서비스 종료
	BOOL m_bStopService;

	// 서비스 취소
	//BOOL m_bCancelService;

	// Supervisor Password Key Check
	CString m_strKeyValue;

	// 커맨드 큐
	CircularQueue<CHAR> m_CmdQueue;

	// EPP 메시지 버퍼
	CircularQueue<CHAR> m_EppMsgBuff;


	// 날짜 변경 후 처음
	BOOL m_First_After_Day_Changed;

	// 현재 날짜 시간
	SYSTEMTIME m_CurrentDatetime;


	// CDM - Connect
	BOOL m_CDM_Connect;

	BOOL m_CDM_CommError;

	// CDR - Connect
	BOOL m_CDR_Connect;

	// DIO - Connect
	BOOL m_DIO_Connect;

	// EJL - Connect
	BOOL m_EJL_Connect;

	// EPP - Connect
	BOOL m_EPP_Connect;

	// HOST - Connect
	BOOL m_HOST_Connect;

	// PRT - Connect
	BOOL m_PRT_Connect;


	// DIO - Front Door 열림
	BOOL m_DIO_Front_Door_Open;

	// DIO - Top Door 열림
	BOOL m_DIO_Safe_Door_Open;

	// DIO - Upper Frame 열림
	BOOL m_DIO_Upper_Frame_Open;

	// DIO - Ear Jack 삽입
	BOOL m_DIO_Ear_Jack_Insert;

	BOOL m_bIsEPPTimeout;

	int	m_nInjectKeySequence;

	CString m_strMasterKey_KCV;

	CString m_strPINKey_KCV;

	// Password관련 변수
	CString strMasterPassword;

	CString strAdminPassword;

	CString strReplenishPassword;

	BOOL	m_bDetectedSensorPoll_PowerOn;	// 최초 Comport Open 후 1번 수행 여부 확인을 위해

	int		m_nEPP_Enable_Recovery;

public:
	// 서비스 모드
	int m_nServiceMode;

	// 계원용 권한 설정
	int m_nOperatorMode;

	// 명세서 출력 화면 사용 여부 설정
	BOOL m_bUseReceipt;

	// 계원용 커맨드 - Device - Printer - Use Receipt
	void Operator_Device_Printer_SetUseReceipt(BOOL bUse);

	// 계원용 커맨드 - Device - Printer - Use Receipt
	BOOL Operator_Device_Printer_GetUseReceipt(void);


	// In service, Out of Service 상태 표시
	BOOL	m_bOutOfService;

	BOOL	m_bAMSStatusSend;

	BOOL	m_bAMSUploadJNLSend;

	BOOL	m_bNotifyStatusToAMS_PowerOn;			// Power On 또는 AMS Status Send Enable시 최초 한번 전송하기 위한 Flag

	BOOL	m_bExecDownloadKeysToHost_PowerOn;		// Power On시 최초 한번 전송하기 위한 Flag

protected:
	// Screen Interface를 위한 변수
	CString m_strScrOutName;
	CString m_strScrOutData;

	/** **********************************************************
	*	@brief		OP 화면 제어를 위한 변수
	************************************************************/
	CString 	m_strCurrentScrNum;
	CString		m_strNextScrNum;
	
	CString		m_strSave_OP_ScreenInfo;
	CString		m_strSave_OP_ButtonInfo;
	int			m_nCurrentFocusIndex;

	/** **********************************************************
	*	@brief		OP 저널 제어를 위한 변수
	************************************************************/
	int		m_nSearchJNLNo;
	int		m_nLastXJNLCnt;
	int		m_nJNLNo_Detail;

	/** **********************************************************
	*	@brief		KEY 주입 관련 명령 제어를 위한 변수
	************************************************************/
	int		m_nActiveCmd;
	
	/** **********************************************************
	*	@brief		CDM에서 사용하는 변수
	************************************************************/
	BOOL m_bInitReset;	///< DEVICE - CASH DISPENSER/CARD READER 메뉴로 접근했을 경우에만 RESET 동작을 수행하도록 하기 위함.

	CString m_strSelectTouch;
};

