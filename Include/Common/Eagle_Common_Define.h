#ifndef _EAGLE_COMMON_DEF_H_
#define _EAGLE_COMMON_DEF_H_

/** **********************************************************
*	@brief	RETAILMSG / DEBUGMSG compatibility for Win32 desktop builds.
*
*	These macros are provided by the WinCE SDK (<dbgapi.h>) and do not exist
*	on the desktop. The WinCE definition is kept untouched; for Win32 the same
*	call form is mapped onto OutputDebugString so existing call sites compile
*	and still emit their text to the debugger output window.
************************************************************/
#if !defined(_WIN32_WCE) && !defined(RC_INVOKED) && defined(__cplusplus)

#include <stdarg.h>
#include <tchar.h>

inline void Eagle_Win32DebugPrintf(LPCTSTR pszFormat, ...)
{
	if (pszFormat == NULL)
		return;

	TCHAR szBuffer[1024] = { 0, };

	va_list args;
	va_start(args, pszFormat);
	_vsntprintf_s(szBuffer, _countof(szBuffer), _TRUNCATE, pszFormat, args);
	va_end(args);

	::OutputDebugString(szBuffer);
}

#ifndef RETAILMSG
#define RETAILMSG(cond, printf_exp)	((void)((cond) ? (Eagle_Win32DebugPrintf printf_exp), 0 : 0))
#endif

#ifndef DEBUGMSG
#define DEBUGMSG(cond, printf_exp)	RETAILMSG(cond, printf_exp)
#endif


// ---------------------------------------------------------------------------
// GetProcAddress() takes a WIDE string on WinCE but only an ANSI string on
// Win32 desktop. The existing call sites all pass _T("Name"), so provide a
// wide overload for the desktop build instead of touching every call site.
// ---------------------------------------------------------------------------
inline FARPROC Eagle_GetProcAddressW(HMODULE hModule, LPCWSTR pszProcName)
{
	if (hModule == NULL || pszProcName == NULL)
		return NULL;

	char szName[256] = { 0, };

	if (0 == ::WideCharToMultiByte(CP_ACP, 0, pszProcName, -1, szName, sizeof(szName), NULL, NULL))
		return NULL;

	return ::GetProcAddress(hModule, szName);
}

inline FARPROC GetProcAddress(HMODULE hModule, LPCWSTR pszProcName)
{
	return Eagle_GetProcAddressW(hModule, pszProcName);
}
#endif // !_WIN32_WCE

/** **********************************************************
*	@brief	WINCE DEBUG PORT ACTIVE 여부
************************************************************/
#define FACTORY_TEST				0		
#define NETWORK_OFFLINE_MODE		0	// NETWORK OFFLINE MODE 여부
#define ACTIVE_DEBUG				0
#define COMM_DEBUG_LEVEL			0

#define SEND_RECV_DEBUG_AMS			0	// 송/수신 전문 debug

#define MOUSE_RBUTTON_ENABLE		0	// 마우스 우측버튼 누를 경우 Screen Hide 여부

#if (NETWORK_OFFLINE_MODE)
	#define SHOW_OFFLINE_TEXT			1		// For Demo : Not Show Text
	#define EXEC_RECOVERY_PROC			0		// Device Error 발생시 Recovery 진행 여부
#else
	#define SHOW_OFFLINE_TEXT			0
	#define EXEC_RECOVERY_PROC			1		// Device Error 발생시 Recovery 진행 여부
#endif

/** **********************************************************
*	@brief	WINCE DEBUG PORT ACTIVE 여부
************************************************************/
#define CS130_MACHINE_TYPE		_T("CS130")
#define MF200_MACHINE_TYPE		_T("MF200")

/** **********************************************************
*	@brief	WINCE AP VERSION (History 관리를 위해서만 사용, 실제 버전은 Resource를 변경해야함)
************************************************************/
//#define AP_VERSION	1,0,1,1		// Factory Version

//#define AP_VERSION	1,0,1,2		// 1. CDU 장애시 Recovery가 수행되지 않는 현상 수정
									// 2. Track2에서 '=' 이후 Data가 없는 경우 SW 비정상 종료현상 수정
									// 3. CDM 모터 과부하 장애 "6C"인 경우 Recovery 미수행하도록 추가
									// 4. 장애 발생시 Journal이 2번 저장되는 현상 수정
									// 5. System Default 수행시 Log도 Clear하도록 추가
									// 6. 방출 중 Power Off시 Flag 설정 로직 추가

//#define AP_VERSION	1,0,1,3		// 1. SNBC RTQuiry 실패 현상 수정 (통신장애 검지 부분도 추가)
									// 2. Set Denomination 수행시 명세표 Print하도록 수정
									// 3. 거래 중 Power Off시에 Protocol 변수 File Write 누락으로 인해 Power On시마다 Reversal 되는 현상 수정
									// 4. Host Delcine시에 Fallback시에도 2nd Generate AC가 수행되는 현상 수정
									// 5. ATM Status Send to AMS 기능
									// 6. Dispense하고 Reset 실패시 Sensor Log (.log / .csv) 생성하도록 추가
									// 7. DEVICE -> TEST CONNECTION에서 TLS가 Select 표현이 안되는 Bug Fix
									// 8. FACTORY MODE
                                    //    - Dispense / Reset시 무조건 Sensor Log (.log / .csv) 생성및 Log BackUp하도록 추가 (Log Index는 999999까지로 변경)
									//    - WDM 통신 로그 추가
									// 9. Image Print 비활성화 처리 - 대신 Transaction Record Text 문구로 대체
									// 10. AP 기동시 Log Backup시에 Log Index가 초기화 되는 현상 수정
									// 11. OS upgrade 기능 추가


//#define AP_VERSION	1,0,2,0		// 1. AP 기동시 CDM 통신 장애시 Cbx Count값이 갱신 안되는 Bug Fix
									// 2. UID 정보로 CE부 인증 처리(Option) -> OS V01.00.07 적용 필수
									// 3. 새벽 5시에 Daily Reboot하도록 로직 추가
									// 4. OS upgrade 기능 임시 제거 (실패시 Main B/D 복구 불가 현상 대책 필요

//#define AP_VERSION	1,0,2,1		// 1. In Service에서 AMS로부터 Command 수신시 Status를 Send하지 못하는 Bug Fix - 미발행

//#define AP_VERSION	1,0,2,2		// 1. Software Upadte가 정상적으로 동작하지 않는 Bug Fix
									// 2. OS Update시에 실패나는 현상 수정
									// 3. AMS로 Journal Upload 후 Count 갱신하도록 수정
									// 4. Screen File명 구조 변경
									// 5. System 날짜 변경시 Reboot Time 갱신하도록 로직 보완
									// 6. Software Update시에 Device 종료하고 수행하도록 로직 보완 (EPP는 제외)
									// 7. TCP/IP Test시 Test Network Flag 초기화 누락 Bug Fix

//#define AP_VERSION	1,0,2,3		// 1. Hardware Reboot 지원
									// 2. System Default 수행시 저널 저장하는 로직 Bug Fix
									// 3. Touch Screen 좌표값 -26 처리 제거 (Bug Fix)
									// 4. OFFLINE에서 ADA 미지원
									// 5. EJL Structure 변경 부분 적용 (Buffer Size 조정) -> EJL Dll 적용 필수
									// 6. 방출 중 장애로 인해 Out of Service 중 Software Reboot시 저널이 2번 저장되는 Bug Fix (거래 정보가 초기화 되지 않는 현상 대책)
									// 7. Fallback 거래시 간헐적으로 Latch가 Off되지 않는 Bug Fix (Initialize Command 추가)
									// 8. Log File 관리 개수를 100 -> 500으로 변경
									// 9. Reboot / Schedule Auto Day Total 관련 시간 계산 로직 Bug Fix
									//10. 비밀번호 Encyption해서 저장하도록 변경
									//11. WDM Error Recovery 안하는 조건 추가 (Error Code 추가, Firmware팀에서 전달 받은 에러코드 적용)
									//12. AMS Log Upload / Software Download시 Base64 로직 추가
									//13. AMS Status 정보에 Terminal Information / OS Verison / Machine Type 추가
									//14. OP - DEVICE - CASH DISPENSER - SET BILL PARAMETERS 메뉴 추가
									//15. Replenishment 권한일 경우, CDR의 Reset, CDM의 Reset/TestDispense, PTR의 Reset 메뉴 활성화 하도록 수정
									//16. Mix Algorithm 수정 - 요청한 매수가 max count(50매)를 넘으면 장애 처리 추가("D100M2")
									//17. AMS Modem 통신시, Timeout을 10초 -> 30초로 변경
									//18. CDM의 센서로그의 헤더정보 추가로 인한 수정(56개 -> 64개)
									//19. Configuration 전문의 Working 정보 송신 로직 수정 (PinKey 존재시 [2로 송신하도록 수정)
									//20. AMS Setup Write시 Rebooting하지 않도록 로직 수정
									//21. LCD Panel Issue 대응 (Hardware Reset시 20sec delay) - OS V01.00.10 적용 필수 (WDM Firmware 05버전도 적용해야함)
									// ==> Demo Software로 공장 Release (1대만 적용)

//#define AP_VERSION	1,0,2,4		//1.  Animation GIF 파일 가능하도록 추가([GIFBOX] script 추가)
									//2.  Print parameters에서 FAST CASH 항목 오타 수정
									//3.  해상도 1280*800을 기준으로 좌표 설정하도록 Screen Engine 수정
									//4.  상부 Door Sensor값 반대로 인지하도록 로직 수정 필요
									//5.  OP - SYSTEM - REBOOT TIME 설정 메뉴 추가(Screen No. 686)
									//6.  OP - SYSTEM - DATE/TIME 화면(Screen No. 681)에서 HOUR, MINUTE => HOUR(00 ~ 23), MINUTE(00 ~ 59)로 수정
									//7.  상부 Door Switch Sensor Reverse 처리
									//8.  Stuck EPP 처리 추가 (Key 감시 Time 10sec로 협의됨) - "Z50004" Error Code 추가
									//9.  최종 OS 적용 (RGB rotation display issue 대응) V01.00.11
									//10. Offline 모드에서는 Lan Card Reload 하지 않도록 수정
									//11. AP 기동시 Safe Door가 열려 있는 경우 다른 Application이 실행할 수 있도록 Interface 추가
									//12. Offline 모드로 내부 발행(매체에 WDM V01.05 포함함)

//#define AP_VERSION	1,0,2,5		//1.  MUB쪽 Buffer Overflow 관련 로직 개선
									//2. AMS 통계를 위해 Reject 및 Remain Count 저널에 저장하도록 로직 추가
									//3. AMS 전송 날짜 및 금액 정보 Format 변경
									//4. Offline Mode에서 비밀번호 입력 일원화 (Read Pin명령으로 통일)
									//5. Screen 폴더 구조 변경 (Customizing쪽으로 분리)
									//6. Print Parameter 위치 변경
									//7. AMS Configuration / Status Common Field 추가
									//8. Print Parameter에 AD 정보 추가
									//9. MF400(P3 Economic 지원) (1024*768)
									//10. Camera 기능 Device 메뉴에 추가
									//11. AMS 원격 Command 추가 (CST Total / Change ATM Date & Time)
									//12. Idle에서 광고 Enable시 간헐적으로 AP가 Holding되는 현상 대책(광고 표시 및 wDM Dll 로직 수정)
									//13. Print중 장애 발생시 Reset 로직 추가
									//14.Transaction DB에 EMV / DCC 칼럼 추가
									//15. System 부하를 줄이기 위해 WDM Polling Time 수정 (1초 -> 2초)
									//16. P3 E Demo용으로 발행

//#define AP_VERSION	1,0,2,6		//1.  Activation 관련 개선
                                    //    PreActivated ->Acticated 전환시 Operator ID 입력 부분 제거 (Confirm으로 대체)
									//    NonCE값을 8자리만 보여주도록 변경 (실제 8자리만 입력해서 Activation Code를 생성함)
									//2. AMS로 Status Send 후 통신 오류시 Accept를 하지 못하는 Bug Fix
									//3. WDM이 ATM Mode가 아닌 경우 Software가 비정상 종료되는 Bug Fix
									//4. MF400 Demo로 Release함 (UI 추가 변경)

//#define AP_VERSION	1,0,2,7		//1. MF400 Demo 대응으로 인해 Release함 (UMT Corporation -> UMT로 문구 및 Image 변경)

//#define AP_VERSION	1,0,2,8		//1. MF400 System 인증용으로 사용 (Aging 기능 추가)

//#define AP_VERSION	1,0,2,9			//1. ADA 거래 중 Remove Card 화면이 표시되는 Bug Fix - Test Version
									//2. Config File Name 및 불필요한 변수 정리
									//3. EMV Data 저장 및 Parsing 로직 변경 -> 기존 EJ Clear 필요
									//4. 명세표 Surcharge Onwer 위치 변경 및 Null인 경우 미인자하도록 로직 수정
									//5. 권한 재정의 / Admin인 경우 Master Key 입력 권한 제거
									//6. 모델명 변경(MF400->MF200)
									//7. Camera Capture시 Image 상하가 반전되어 저장되는 Bug Fix
									//8. EPP Active시에 USER ID 항목 제외 (내부적으로 "12345678"로 Fix 처리 (고객 입력 최소화)
									//9. CBX Setting은 Admin은 불가능하도록 권한 변경
									//10. AP 기동시 최초 한번 Configuration을 수행하도록 로직 변경
									//12. CRC 에러 발생시 Error Code가 잘못 Setting되는 Bug Fix
									//11. 각종 Bug Fix
									//12. Power Off시 Sum 로직 누락 추가
									//13. DIO 미사용으로 인해 해당 소스 제거
//#define AP_VERSION	1,0,2,10		//1. CBX #3#4 Low Sensor Detected Bug Fix
									//2. Set Bill Max값 수정
									//3. DIO 송신 전문 구성 위치 이동(DEV_MUB.cpp -> DEV_DIO.cpp)
									//4. TestDispense 저널 추가
									//5. Config Backup / Upload시에 기종별로 가능하도록 추가
									//6. Mix 실패시 이전 방출정보가 올라오는 Bug Fix
#define AP_VERSION	001,001,001,001	//1. 버전 체계 재정의로 인해 버전 변경

/** **********************************************************
*	@brief	MAIN BOARD	TYPE
************************************************************/
//#define	MAINBOARD_TYPE				1			// 0 - 진영콘택, 1 - Nexcom -> 미사용


/** **********************************************************
*	@brief	SCREEN DESIGN 영역 확인용
************************************************************/
//#define		AE_SCREEN_DEGISN

/** **********************************************************
*	@brief	상부 Door Reverse 처리
************************************************************/
#define		FRONT_DOOR_REVERSE		1

/** **********************************************************
*	@brief	거래 명세표 Print시에 Image Print 인자 여부
************************************************************/
//#define ENABLE_RECEIPT_HEADER_IMAGE_PRINT	1

/** **********************************************************
*	@brief	AUTO AGING MODE
************************************************************/
#define AUTO_AGING_TRAN_WITHDRAWAL	0		// 현재 미사용

/** **********************************************************
*	@brief	DEVICE EMULATION MODE 여부
************************************************************/
#ifdef _WIN32_WCE
#define	EMULATION_CDM_DEVICE		0	// Port8
#define EMULATION_EPP_DEVICE		0	// USB로 연결 (USB TO Serial)
#define EMULATION_PTR_DEVICE		0	// Port 7 사용
#define EMULATION_SIU_DEVICE		0	// Multi Sserial Port로 연결
#define EMULATION_CAM_DEVICE		0	// USB로 연결
#define EMULATION_CDR_DEVICE		0	// Port 6 사용
#define EMULATION_MUB_DEVICE		0	// Port 4 사용
#define EMULATION_EJL_DEVICE		0	// MFS_EJL_WEC7.dll
#else
// Win32 desktop: no physical devices are attached, so every device class
// runs its already-existing EMULATION_xxx_DEVICE simulation code path.
#define	EMULATION_CDM_DEVICE		1
#define EMULATION_EPP_DEVICE		1
#define EMULATION_PTR_DEVICE		1
#define EMULATION_SIU_DEVICE		1
#define EMULATION_CAM_DEVICE		1
#define EMULATION_CDR_DEVICE		1
#define EMULATION_MUB_DEVICE		1
// EJL is NOT emulated on Win32: MFS_EJL_WEC7 is built for Win32 too, using the
// SQLite amalgamation, so the real electronic journal is used on both platforms.
#define EMULATION_EJL_DEVICE		0
#endif // _WIN32_WCE


/** **********************************************************
*	@brief	CAMEARA Device 사용 여부
************************************************************/
#define USE_CAMERA_DEVICE			1

/** **********************************************************
*	@brief	NETWORK TIME 및 STATE DEFINE
************************************************************/
#define		CONN_TIMEOUT			60
#define		CONN_TIMEOUT_TCPIP		20

#define		ENQ_TIMEOUT				35
#define		ACK_TIMEOUT				30
#define		DATA_TIMEOUT			60
#define		DATA_TIMEOUT_STD		120
#define		EOT_TIMEOUT_REQUIRED	60
#define		EOT_TIMEOUT_OPTIONAL	10

#define		DATA_STX				0x02
#define		DATA_ETX				0x03
#define		DATA_EOT				0x04
#define		DATA_ENQ				0x05
#define		DATA_ACK				0x06
#define		DATA_NAK				0x15

// Network state Define
#define		INIT_STS				0
#define		CONN_STS				1
#define		ENQIN_STS				2		// Only use VISA FRAMED
#define		ACKIN_STS				3		// use VISA FRAMED & ACK CONTROLLED
#define		HIGHLENIN_STS			4		// use STANDARD / ACK CONTROLLED
#define		LOWLENIN_STS			5		// use STANDARD / ACK CONTROLLED
#define		STXIN_STS				6		// Only use VISA FRAMED
#define		ETXIN_STS				7		// Only use VISA FRAMED
#define		EOTWAIT_STS				8		// use VISA FRAMED & ACK CONTROLLED

/** **********************************************************
*	@brief	NETWORK RESULT CODE DEFINE
************************************************************/
#define		SUCCESS					0
#define		CONNECT_FAIL			1
#define		RECV_FAIL_ENQ			2
#define		RECV_FAIL_ACK			3
#define		RECV_FAIL_DATA			4
#define		RECV_FAIL_EOT			5
#define		COMM_ERROR				6


/** **********************************************************
*	@brief	Define Field Delimiter
************************************************************/
#define		FIELD_DELIMITER		((char)0x1C)
#define		GROUP_SEPARATOR		((char)0x1D)
#define		RECORD_SEPARATOR	((char)0x1E)
#define		UNIT_DELIMITER		((char)0x1F)
#define		FIELD_SPACE			((char)0x20)


/** **********************************************************
*	@brief	Define error msg sub field
************************************************************/
#define		JNL_ERRMSG_FIELD_EMV_DATA		_T("E")
#define		JNL_ERRMSG_FIELD_FALLBACK		_T("F")


/** **********************************************************
*	@brief	Define 국가 통화 기호
**************************************************************/
#define CURRENCY_SYMBOL				_T("$")


/** ***********************************************************
*	@brief	ERROR DataBase <-> AP Delimiter
**************************************************************/
#define ERROR_DB_LINEFEED_DELEMITER	_T("^")

/** ***********************************************************
*	@brief	AP 내부적으로 사용하는 Delimeter
**************************************************************/
#define AP_DELEMITER				_T("|")


/** ***********************************************************
*	@brief	SCR <-> AP Delimiter
**************************************************************/
#define SCR_CMD_DELIMITER			_T("\r")
#define SCR_RES_DELIMITER			((char)0x1F)
#define SCR_LINEFEED_DELEMITER		_T("\n")

#define S_ENTER						_T("ENTER")
#define S_TIMEOVER					_T("TIMEOVER")
#define S_EXIT						_T("EXIT")
#define S_CLEAR						_T("CLEAR")
#define S_CANCEL					_T("CANCEL")
#define S_YES						_T("YES")
#define S_NO						_T("NO")
#define S_SAVE						_T("SAVE")

/** **********************************************************
*	@brief Advertisement Define.
************************************************************/
#define SUPPORT_ADVERTISE			6


/** **********************************************************
*	@brief Define Screen Resolution
************************************************************/
#define CS130_WIDTH_RESOLUTION		(1280)
#define MF200_WIDTH_RESOLUTION		(1024)


/** **********************************************************
*	@brief Define Screen Number
************************************************************/
#define MAX_SCREEN_COUNT								999		// 지원 가능 Screen 개수
#define CLIENT_SCREEN_INITIALIZE					_T("001")
#define CLIENT_SCREEN_OUTOFSERVICE					_T("002")
#define CLIENT_SCREEN_INSERTCARD					_T("003")
#define CLIENT_SCREEN_CARDERROR						_T("004")
#define CLIENT_SCREEN_ENTERPASSWORD					_T("005")
#define CLIENT_SCREEN_TRANSACTION_CANCEL			_T("006")
#define CLIENT_SCREEN_SELECT_TRANSACTION			_T("007")
#define CLIENT_SCREEN_SELECT_ACCOUNT				_T("008")
#define CLIENT_SCREEN_SELECT_TRANSFER_ACCOUNT		_T("009")
#define CLIENT_SCREEN_SELECT_AMOUNT					_T("010")
#define CLIENT_SCREEN_ERROR_NOTICE					_T("011")
#define CLIENT_SCREEN_CONFIRM_SURCHARGE				_T("012")
#define CLIENT_SCREEN_SELECT_RECEIPT				_T("013")
#define CLIENT_SCREEN_HOSTPROCESSING				_T("014")
#define CLIENT_SCREEN_CASHDISPENSING				_T("015")
#define CLIENT_SCREEN_GET_CASH						_T("016")
#define CLIENT_SCREEN_PRINTING_RECEIPT				_T("017")
#define CLIENT_SCREEN_SCREEN_RECEIPT				_T("018")
#define CLIENT_SCREEN_EXIT_MESSAGE					_T("019")
#define CLIENT_SCREEN_CONFIRM_SCREENRECEIPT			_T("020")
#define CLIENT_SCREEN_ENTER_TRANSFERAMOUNT			_T("021")
#define CLIENT_SCREEN_SELECT_LANGUAGE				_T("022")
#define CLIENT_SCREEN_ENTER_AMOUNT					_T("023")
#define CLIENT_SCREEN_CONFIRM_RECEIPTERROR			_T("024")
#define CLIENT_SCREEN_EMVPROCESSING					_T("025")
#define CLIENT_SCREEN_EMVFALLBACK					_T("026")
#define CLIENT_SCREEN_EMVREMOVECARD					_T("027")
#define CLIENT_SCREEN_EMVMULTIAPPSELECT				_T("028")

#define CLIENT_SCREEN_AMSPROGRESS_SCR_DISPLAY		_T("090")

#define CLIENT_SCREEN_ADA_SCR_DISPLAY				_T("100")

#define CLIENT_ADV_SCREEN_1							450
#define CLIENT_ADV_SCREEN_2							451
#define CLIENT_ADV_SCREEN_3							452
#define CLIENT_ADV_SCREEN_4							453
#define CLIENT_ADV_SCREEN_5							454
#define CLIENT_ADV_SCREEN_6							455

#define OP_SCREEN_MAIN								_T("500")

#define OP_SCREEN_DEVICE							_T("501")
	#define OP_SCREEN_DEVICE_CAMERA						_T("502")
	#define OP_SCREEN_DEVICE_CDR						_T("503")
	#define OP_SCREEN_DEVICE_CDR_INSERT					_T("504")
	#define OP_SCREEN_DEVICE_CDR_REMOVE					_T("505")
	#define OP_SCREEN_DEVICE_CDR_RESULT					_T("506")
	#define OP_SCREEN_DEVICE_SOUND						_T("507")
	#define OP_SCREEN_DEVICE_CDM						_T("508")
	#define OP_SCREEN_DEVICE_TESTDISPENSE				_T("509")
	#define OP_SCREEN_DEVICE_CASSETTESETTING			_T("510")
	#define OP_SCREEN_DEVICE_LEARNING					_T("511")
	#define OP_SCREEN_DEVICE_SIU						_T("512")
	#define OP_SCREEN_DEVICE_PINPAD						_T("513")
	#define OP_SCREEN_DEVICE_PRINTER					_T("514")
	#define OP_SCREEN_DEVICE_TCPIP						_T("515")
	#define OP_SCREEN_DEVICE_MODEM						_T("516")
	#define OP_SCREEN_DEVICE_SETBILL					_T("517")
	#define OP_SCREEN_DEVICE_CAMERA_CAPTURE				_T("518")

#define OP_SCREEN_JOURNAL							_T("550")
	#define OP_SCREEN_VIEW_JOURNAL						_T("551")
		#define OP_SCREEN_VIEW_JOURNAL_DETAIL				_T("552")
	#define OP_SCREEN_PRINT_LASTX_JOURNAL				_T("554")



#define OP_SCREEN_HOST								_T("560")
	#define OP_SCREEN_TERMINAL_NETWORK_SETTINGS			_T("561")
		#define OP_SCREEN_TERMINAL_MODEM_SETTINGS			_T("562")
		#define OP_SCREEN_TERMINAL_IP_SETTINGS				_T("563")

	#define OP_SCREEN_KEY_MANAGEMENT					_T("564")
		#define OP_SCREEN_MASTERKEYS_MANAGEMENT				_T("565")
		#define OP_SCREEN_PINPAD_ACTIVATION					_T("566")
		#define OP_SCREEN_INJECT_MASTER_KEY					_T("567")
		#define OP_SCREEN_MASTER_KEY_KCV_INFO				_T("568")

	#define OP_SCREEN_EMV_AID_LIST						_T("570")

	#define OP_SCREEN_CONFIGURE_CONNECTION				_T("580")
		#define OP_SCREEN_PROTOCOL_OPTION					_T("581")
		#define OP_SCREEN_HOST_PHONENUMBER					_T("582")
		#define OP_SCREEN_HOST_IP_ADDRESS					_T("583")
		#define OP_SCREEN_TERMINAL_MONITORING				_T("584")

#define OP_SCREEN_OPTION							_T("600")
	#define OP_SCREEN_OPTION_TRANSACTION                _T("601")
		#define OP_SCREEN_OPTION_AMOUNT_SETTINGS		   _T("602")     
		#define OP_SCREEN_OPTION_BLOCK_ISO					_T("603")
		#define OP_SCREEN_OPTION_CONFIGURE_GENERAL			_T("604")

	#define OP_SCREEN_OPTION_AD_SCREEN                  _T("606")
		#define OP_SCREEN_OPTION_AD_1_PREVIEW				_T("607")
		#define OP_SCREEN_OPTION_AD_2_PREVIEW				_T("608")
		#define OP_SCREEN_OPTION_AD_3_PREVIEW				_T("609")
		#define OP_SCREEN_OPTION_AD_4_PREVIEW				_T("610")
		#define OP_SCREEN_OPTION_AD_5_PREVIEW				_T("611")
		#define OP_SCREEN_OPTION_AD_6_PREVIEW				_T("612")
	
	#define OP_SCREEN_OPTION_MESSAGE                    _T("613")
		#define OP_SCREEN_OPTION_MARKETING					_T("614")
		#define OP_SCREEN_OPTION_LOCATION					_T("615")
		#define OP_SCREEN_OPTION_ATTRACT					_T("616")
		#define OP_SCREEN_OPTION_FAREWELL					_T("617")

#define OP_SCREEN_TASK								_T("650")
	#define OP_SCREEN_ADD_QTY_IN_CASSETTE				_T("651")
	#define OP_SCREEN_SCHEDULE_DAY_CLOSE				_T("652")

#define OP_SCREEN_AMS_CONFIGURE_MODEM				_T("660")
#define OP_SCREEN_AMS_CONFIGURE_TCPIP				_T("661")

#define OP_SCREEN_REPORT							_T("670")
	#define		OP_SCREEN_ERRORCODE							_T("671")


#define OP_SCREEN_SYSTEM							_T("680")
	#define OP_SCREEN_DATE_N_TIME						_T("681")
	#define	OP_SCREEN_UPLOAD							_T("682")
	#define OP_SCREEN_BACKUP							_T("683")
	#define OP_SCREEN_PASSWORD							_T("684")
		#define OP_SCREEN_USER_CHANGE_PASSWORD			_T("685")
	#define OP_SCREEN_REBOOT							_T("686")
	#define OP_SCREEN_BACKUP_CAPTUREIMAGE				_T("687")


#define OP_SCREEN_INPUT_TEXT						_T("900")
#define OP_SCREEN_INPUT_NUMBER						_T("901")
#define OP_SCREEN_PROGRESS							_T("902")
#define OP_SCREEN_CONFIRM							_T("904")
#define OP_SCREEN_INPUT_HEX							_T("905")


#define OP_SCREEN_EXIT								_T("999")	// 해당 Screen은 001로 대체

/** **********************************************************
*	@brief Define Screen Timeout
************************************************************/
#define CLIENT_SCREEN_TIMEOUT					30
#define CLIENT_SCREENRECEIPT_TIMEOUT			10
#define SUPERVISOR_SCREEN_TIMEOUT				120			// OP Timeout 변경 1min -> 2min
#define SUPERVISOR_INFO_SCREEN_TIMEOUT			30			// 장애코드 화면, Data View 화면 Timeout 변경
#define SUPERVISOR_PINPAD_ACTIVE_TIMEOUT		600			// 10 min

/** **********************************************************
*	@brief Define Transaction Type
************************************************************/
#define	S_WITHDRAWAL			_T("WITHDRAWAL")
#define S_BALANCEINQUIRY		_T("INQUIRY")
#define S_TRANSFER				_T("TRANSFER")

/** **********************************************************
*	@brief Define Account Type
************************************************************/
#define	S_CHECKING			_T("CHECKING")
#define S_SAVINGS			_T("SAVINGS")
#define S_CREDIT			_T("CREDIT")


/** **********************************************************
*	@brief Get Text String List
************************************************************/
#define		SCR_TEXTVALUE_CMD		_T("APValue")			// 문자열 설정
#define		SCR_CTRLONOFF_CMD		_T("APState")			// 버튼 ON-OFF값 설정
#define		SCR_SET_FOCUS_VALUE		_T("APSetFocus")
#define		SCR_BLINK_CMD			_T("BLINKING")
#define		SCR_EDIT_CTRL_CMD		_T("APEditState")


#define		GETTEXT_001				_T("APTextID:GETTEXT_001")
#define		GETTEXT_002				_T("APTextID:GETTEXT_002")
#define		GETTEXT_003				_T("APTextID:GETTEXT_003")
#define		GETTEXT_004				_T("APTextID:GETTEXT_004")
#define		GETTEXT_005				_T("APTextID:GETTEXT_005")
#define		GETTEXT_0051			_T("APTextID:GETTEXT_0051")
#define		GETTEXT_0052			_T("APTextID:GETTEXT_0052")
#define		GETTEXT_006				_T("APTextID:GETTEXT_006")
#define		GETTEXT_007				_T("APTextID:GETTEXT_007")
#define		GETTEXT_008				_T("APTextID:GETTEXT_008")
#define		GETTEXT_009				_T("APTextID:GETTEXT_009")
#define		GETTEXT_010				_T("APTextID:GETTEXT_010")
#define		GETTEXT_011				_T("APTextID:GETTEXT_011")
#define		GETTEXT_012				_T("APTextID:GETTEXT_012")
#define		GETTEXT_013				_T("APTextID:GETTEXT_013")
#define		GETTEXT_014				_T("APTextID:GETTEXT_014")
#define		GETTEXT_015				_T("APTextID:GETTEXT_015")
#define		GETTEXT_016				_T("APTextID:GETTEXT_016")
#define		GETTEXT_017				_T("APTextID:GETTEXT_017")
//#define		GETTEXT_018				_T("APTextID:GETTEXT_018")	// MF300 Only Use
//#define		GETTEXT_019				_T("APTextID:GETTEXT_019")	// MF300 Only Use
#define		GETTEXT_020				_T("APTextID:GETTEXT_020")


// V1.0.2.4 2018.07.11 - Screen Engine 구조 변경에 따른 수정
#define		OP_GUIDE_COMMON_WAIT		_T("PLEASE WAIT A MINUTE...")
#define		OP_GUIDE_COMMON_CONFIRM		_T("DO YOU WANT TO CONTINUE?")
#define		OP_GUIDE_COMMON_INVALID		_T("INVALID VALUE")
#define		OP_GUIDE_TOUCH_INVALID		_T("CAN NOT BE SELECTED.")
#define		OP_GUIDE_MAX_INPUT_LEN		_T("MAX INPUT LENGTH :")
#define		OP_GUIDE_MAX_INPUT_VAL		_T("MAX INPUT VALUE :")
///////////////////////////////////////////////////////////////////////


/** **********************************************************
*	@brief Define Key Mapping
************************************************************/
#define		KEYMAP_1				'1'
#define		KEYMAP_2				'2'
#define		KEYMAP_3				'3'
#define		KEYMAP_4				'4'
#define		KEYMAP_5				'5'
#define		KEYMAP_6				'6'
#define		KEYMAP_7				'7'
#define		KEYMAP_8				'8'
#define		KEYMAP_9				'9'
#define		KEYMAP_0				'0'
#define		KEYMAP_LEFT				'A'
#define		KEYMAP_RIGHT			'B'
#define		KEYMAP_BLANK			'C'
#define		KEYMAP_ENTER			'D'
#define		KEYMAP_CLEAR			'E'
#define		KEYMAP_CANCEL			'F'

#define		KEYMAP_ADA_1			"A1"
#define		KEYMAP_ADA_2			"A2"
#define		KEYMAP_ADA_3			"A3"
#define		KEYMAP_ADA_4			"A4"
#define		KEYMAP_ADA_5			"A5"
#define		KEYMAP_ADA_6			"A6"
#define		KEYMAP_ADA_7			"A7"
#define		KEYMAP_ADA_8			"A8"
#define		KEYMAP_ADA_9			"A9"
#define		KEYMAP_ADA_0			"A0"
#define		KEYMAP_ADA_LEFT			"AA"
#define		KEYMAP_ADA_RIGHT		"AB"
#define		KEYMAP_ADA_BLANK		"AC"

#define		KEYMAP_FDK_L1			'K'
#define		KEYMAP_FDK_L2			'L'
#define		KEYMAP_FDK_L3			'M'
#define		KEYMAP_FDK_L4			'N'
#define		KEYMAP_FDK_R1			'G'
#define		KEYMAP_FDK_R2			'H'
#define		KEYMAP_FDK_R3			'I'
#define		KEYMAP_FDK_R4			'J'

/** **********************************************************
*	@brief Define cfg file path & name
************************************************************/
#define		EAGLE_ATM_CONFIG_PATH			_T("\\Config")
#define		EAGLE_ATM_CONFIG_DEFAULT_PATH	_T("\\Config\\Default")
#define		EAGLE_ATM_AD_PATH				_T("\\AD")
#define		ADS_CFG_PATH_NAME						_T("ADS.CFG")
#define		BINLIST_CFG_PATH_NAME					_T("BINLIST.CFG")
#define		CBXINFO_CFG_PATH_NAME					_T("CBXINFO.CFG")
#define		DEVICE_CFG_PATH_NAME					_T("DEVICE.CFG")
#define		DISPENSE_CFG_PATH_NAME					_T("DISPENSE.CFG")
#define		EAGLE_ATM_EMV_CONFIG_PATH_NAME			_T("EMV_CONFIG.CFG")
#define		EAGLE_ATM_EMV_US_COMM_AID_PATH_NAME		_T("EMV_US_COMM_AID.CFG")
#define		HOST_CFG_PATH_NAME						_T("HOST.CFG")
#define		LASTTRANSINFO_CFG_PATH_NAME				_T("LASTTRANSINFO.CFG")
#define		MESSAGE_CFG_PATH_NAME					_T("MESSAGE.CFG")
#define		OPTION_CFG_PATH_NAME					_T("OPTION.CFG")
#define		SYSTEM_CFG_PATH_NAME					_T("SYSTEM.CFG")
#define		TOTAL_CFG_PATH_NAME						_T("TOTALS.CFG")
#define		BACKUP_CONFIGFILE_NAME					_T("BACKUPCONFIG.DAT")

#define		EJL_DB_FILE_NAME						_T("MFS_EJL.db")
#define		ERRORCODE_DB_FILE_NAME					_T("MFS_ERR.db")


/** **********************************************************
*	@brief Define etc application Path & name
************************************************************/
#define		EXE_TERMINATE_EAGLECE					_T("\\Terminate_EagleCE.exe")
#define		EXE_SW_MONITOR							_T("\\SW_Monitor.exe")

#define		EXE_SWUPDATE_EAGLECE					_T("\\USB\\SoftwarePackage\\SoftwareUpdate.exe")
#define		PATH_UPLOAD_CONFIG						_T("\\USB\\SoftwarePackage\\CONFIG.zip")
#define		PATH_UPLOAD_ADVERTISEMENT				_T("\\USB\\SoftwarePackage\\ADV.zip")
#define		EXE_SOFTWARE_UPDATE_FROM_AMS_PATH		_T("\\SoftwarePackage\\SoftwareUpdate.exe")

#define		USB_CS130_OS_NK_FILE_NAME				_T("\\USB\\SoftwarePackage\\OS\\1280_800\\NK.nb0")
#define		USB_MF200_OS_NK_FILE_NAME				_T("\\USB\\SoftwarePackage\\OS\\1024_768\\NK.nb0")

#define		PATH_RECOVERY_APPLICATION				_T("\\USB\\Recovery\\RecoverySoftware.exe")

//#define		PATH_CHANGELCD_ROTATION_APP				_T("\\Windows\\Rotation.exe")

#define		PATH_USB								_T("\\USB")
#define		PATH_LOG								_T("\\Trace")
#define		PATH_TEMP								_T("\\Temp")
//#define		PATH_TEMP_AMS_LOG						_T("\\TempTrace")

#define		UPDATE_POOL								_T("\\SoftwarePackage")
#define		OS_UPDATE_POOL							_T("\\SoftwarePackage\\OS")

#define		RECEIPT_LOGO_IMAGE						_T("\\Receipt\\PRINT_LOGO.bmp")

#define		CDM_LOG_FILE_NAME_LOG					_T("CDM_SensorLog.log")
#define		CDM_LOG_FILE_NAME_CSV					_T("CDM_SensorLog.csv")
#define		CDM_LOG_FILE_SEARCH_NAME				_T("CDM_SensorLog*.*")

// Make Screen Files Name
#define		SCREEN_OP_DESIGN_FILE					_T("\\Supervisor\\Supervisor.dat")
#define		SCREEN_CLIENT_ADV_CTRL					_T("\\Client_AdvScreen.dat")
#define		TEXT_CLIENT_HOST_ERROR 					_T("\\HostErrorText.dat")
#define		TEXT_CLIENT_RECEIPTPRINT				_T("\\ReceiptPrintText.dat")
#define		SCREEN_CLIENT_CTRL_FILE					_T("\\ScreenControl.dat")

// Make SSL Files Name
#define		TERMINAL_CERTIFICATE_FILE					_T("\\terminal-certificate.pem")
#define		TERMINAL_PRIVATEKEY_FILE					_T("\\terminal-privatekey.pem")

#define		AMS_CERTIFICATE_FILE						_T("\\ams-certificate.pem")

// [PCI-SSF Fix] 표준 Host(Triton) 연결용 Root CA - GetAppPath()(BIN 폴더) 기준 상대경로
#define		HOST_ROOTCA_CERTIFICATE_FILE				_T("\\rootCA.pem")

#define		SOFTWARE_UPDATE_TYPE_NAME				_T("\\SoftwarePackage\\SOFTWARE_TYPE.txt")
#define		ATM_STATUS_MONITORING_FILE				_T("\\ATM_Status.txt")

#define		AD_TITLE_FILE							_T("\\AD_Title.txt")


/** **********************************************************
*	@brief Define OS version information Registry path
************************************************************/
#define		REGISTRY_OS_INFO_KEY				_T("SYSTEM\\NEXCOM\\ImageInformation")
#define		REGISTRY_BOOTLOADER_VALUE_NAME		_T("EbootVer")
#define		REGISTRY_KERNEL_VALUE_NAME			_T("NkVer")


/** **********************************************************
*	@brief Define Sound Volume Range
************************************************************/
#define	MAX_VOLUME_LEVEL		10
#define	MIN_VOLUME_LEVEL		0

/** **********************************************************
*	@brief Define Wave File List
************************************************************/
#define WAVE_FILE_PATH						_T("\\Wave\\")

#define KEY_PRESSED_WAVE_FILE				_T("KeySound.wav")
#define NOTICE_WAVE_FILE					_T("NoticeSound.wav")
#define BILLION_WAVE_FILE					_T("Billion.wav")
#define MILLION_WAVE_FILE					_T("Million.wav")
#define THOUSAND_WAVE_FILE					_T("Thousand.wav")
#define HUNDRED_WAVE_FILE					_T("Hundred.wav")
#define MINUS_WAVE_FILE						_T("Minus.wav")
#define DOLLAR_WAVE_FILE					_T("Dollar.wav")
#define DOLLARS_WAVE_FILE					_T("Dollars.wav")
#define CENT_WAVE_FILE						_T("Cent.wav")
#define CENTS_WAVE_FILE						_T("Cents.wav")

// Insert Card
#define INTRODUCE_WAVE_FILE					_T("Introduce.wav")
#define LOCATION_EPP_WAVE_FILE				_T("Location_EPP.wav")
#define INTRO_VOLUME_WAVE_FILE				_T("Intro_Volume.wav")
#define LOCATION_CARD_WAVE_FILE				_T("Location_Card.wav")
#define LOCATION_PRINTER_WAVE_FILE			_T("Location_Printer.wav")
#define LOCATION_DISPENSER_WAVE_FILE		_T("Location_Dispenser.wav")
#define MS_TRANSACTION_WAVE_FILE			_T("MS_Transaction.wav")
#define EMV_TRANSACTION_WAVE_FILE			_T("EMV_Transaction.wav")
#define EMV_REMOVECARDFORTRANS_WAVE_FILE	_T("EMV_RemoveCardforTrans.wav")
#define EMV_FALLBACK_TRANSACTION_WAVE_FILE	_T("EMV_Fallback_Transaction.wav")
#define EMV_SELECT_BRANDNAME_WAVE_FILE		_T("EMV_SelectBrandName.wav")

#define BRANDNAME_VISA_WAVE_FILE			_T("VISA.wav")
#define BRANDNAME_VISAELECTRON_WAVE_FILE	_T("VISAELECTRON.wav")
#define BRANDNAME_VISAPLUS_WAVE_FILE		_T("VISAPLUS.wav")
#define BRANDNAME_USVISA_WAVE_FILE			_T("USVISA.wav")
#define BRANDNAME_MASTERCARD_WAVE_FILE		_T("MASTERCARD.wav")
#define BRANDNAME_MAESTRO_WAVE_FILE			_T("MAESTRO.wav")
#define BRANDNAME_CIRRUS_WAVE_FILE			_T("CIRRUS.wav")
#define BRANDNAME_USMAESTRO_WAVE_FILE		_T("USMAESTRO.wav")
#define BRANDNAME_PLUSEDPAS_WAVE_FILE		_T("PULSEDPAS.wav")
#define BRANDNAME_USDISCOVER_WAVE_FILE		_T("USDISCOVER.wav")
#define BRANDNAME_AMEX_WAVE_FILE			_T("AMEX.wav")
#define BRANDNAME_JCB_WAVE_FILE				_T("JCB.wav")
#define BRANDNAME_UPDEBIT_WAVE_FILE			_T("UNIONPAYDEBIT.wav")
#define BRANDNAME_UPCREDIT_WAVE_FILE		_T("UNIONPAYCREDIT.wav")
#define BRANDNAME_USUP_WAVE_FILE			_T("USUNIONPAY.wav")
#define BRANDNAME_USDNA_WAVE_FILE			_T("USDNA.wav")
#define BRANDNAME_UNKNOWN_WAVE_FILE			_T("UNKNOWNNAME.wav")

// Enter Pin
#define ENTERPIN_WAVE_FILE					_T("EnterPin.wav")

// Select Transaction
#define SELECT_TRANSACTION_WAVE_FILE		_T("SelectTransaction.wav")


// Select Account
#define SELECT_ACCOUNT_WAVE_FILE			_T("SelectAccount.wav")


// Select Amount
#define SELECT_AMOUNT_WAVE_FILE				_T("selectAmount.wav")

// Enter Amount
#define	ENTER_AMOUNT_WAVE_FILE				_T("EnterAmount.wav")
#define MULTIPLE_AMOUNT_WAVE_FILE			_T("MultipleAmount.wav")
#define MAX_AMOUNT_WAVE_FILE				_T("MaxAmount.wav")

// Confirm Amount
#define CONFIRM_AMOUNT_WAVE_FILE			_T("ConfirmAmount.wav")

// Fee Notice
#define FEE_NOTICE_1_WAVE_FILE				_T("FeeNoticeGuide1.wav")
#define FEE_NOTICE_2_WAVE_FILE				_T("FeeNoticeGuide2.wav")
#define FEE_NOTICE_3_WAVE_FILE				_T("FeeNoticeGuide3.wav")

// Select Receipt
#define SELECT_RECEIPT_WAVE_FILE			_T("SelectReceipt.wav")

// Confirm Receipt Error
#define CONFIRM_RECEIPTERROR_WAVE_FILE		_T("ReceiptNotAvailable.wav")

// Confirm Screen Receipt
#define CONFIRM_SCREEN_RECEIPT_WAVE_FILE	_T("SelectScreenReceipt.wav")

// Transaction Processing
#define TRANSACTION_PROCESSING_WVE_FILE		_T("TransactionProcessing.wav")
#define REVERSAL_PROCESSING_WAVE_FILE		_T("ReversalProcessing.wav")

// Enter Transfer Amount
#define ENTER_TRANSFER_AMOUNT_WAVE_FILE		_T("EntTransferAmount.wav")

// Dispensing
#define DISPENSING_WAVE_FILE				_T("Dispensing.wav")
#define DISPENSING_ERROR_WAVE_FILE			_T("DispensingError.wav")

// Take Cash
#define TAKECASH_WAVE_FILE					_T("TakeCash.wav")
#define LOCATION_CASH_WAVE_FILE				_T("LocationCash.wav")

// Printing Receipt
#define	PRINTINGRECEIPT_WAVE_FILE			_T("PrintingReceipt.wav")
#define BALANCE_INFO_WAVE_FILE				_T("GuideBalance.wav")
#define AVAILABLE_BALANCE_INFO_WAVE_FILE	_T("GuideAvailableBalance.wav")
#define PRINTINGERROR_WAVE_FILE				_T("PrintingError.wav")

// Thank you
#define THANKYOU_WAVE_FILE					_T("Thankyou.wav")
#define REMOVEEARPHONE_WAVE_FILE			_T("RemoveEarphone.wav")

// Car Error
#define CARDERROR_WAVE_FILE					_T("CardError.wav")
#define EMV_CARDERROR_WAVE_FILE				_T("EMV_CardError.wav")

// Error
#define INVALIDAMOUNT_WAVE_FILE				_T("InvalidAmount.wav")
#define DECLINED_WAVE_FILE					_T("Decline.wav")
#define APPROVED_WAVE_FILE					_T("Approve.wav")


// Transaction Cancel
#define TRANSCANCEL_WAVE_FILE				_T("TransactionCancel.wav")

// Press 1
#define PRESS_1_WAVE_FILE					_T("Press_1.wav")
#define PRESS_2_WAVE_FILE					_T("Press_2.wav")
#define PRESS_3_WAVE_FILE					_T("Press_3.wav")
#define PRESS_4_WAVE_FILE					_T("Press_4.wav")
#define PRESS_5_WAVE_FILE					_T("Press_5.wav")
#define PRESS_6_WAVE_FILE					_T("Press_6.wav")
#define PRESS_7_WAVE_FILE					_T("Press_7.wav")
#define PRESS_8_WAVE_FILE					_T("Press_8.wav")
#define WITHDRAWAL_WAVE_FILE				_T("Withdrawal.wav")
#define INQUIRY_WAVE_FILE					_T("Inquiry.wav")
#define TRANSFER_WAVE_FILE					_T("Transfer.wav")
#define CHECKING_WAVE_FILE					_T("Checking.wav")
#define SAVINGS_WAVE_FILE					_T("Savings.wav")
#define CREDIT_WAVE_FILE					_T("Credit.wav")
#define TRANSFER_WAVE_FILE					_T("Transfer.wav")
#define CHECKINGTOSAVINGS_WAVE_FILE			_T("CheckingToSavings.wav")
#define SAVINGSTOCHECKING_WAVE_FILE			_T("SavingsToChecking.wav")
#define CREDITTOCHECKING_WAVE_FILE			_T("CreditToChecking.wav")
#define OTHER_WAVE_FILE						_T("Other.wav")
#define CONTINUE_TRANSACTION_WAVE_FILE		_T("ContinueTransaction.wav")
#define YES_WAVE_FILE						_T("Yes.wav")
#define NO_WAVE_FILE						_T("No.wav")
#define INVALIDKEY_WAVE_FILE				_T("InvalidKey.wav")
#define CANCEL_GUIDE_WAVE_FILE				_T("Cancel_Guide.wav")
#define SELECTEDKEY_WAVE_FILE				_T("SelectedItem.wav")
#define ENTEREDKEY_WAVE_FILE				_T("Entered.wav")
#define VOLUMEUP_WAVE_FILE					_T("VolumeUp.wav")
#define VOLUMEDOWN_WAVE_FILE				_T("VolumeDown.wav")
#define REPEAT_WAVE_FILE					_T("Repeat.wav")
#define	CANCEL_WAVE_FILE					_T("Cancel.wav")
#define	CLEAR_WAVE_FILE						_T("Clear.wav")
#define	ENTER_WAVE_FILE						_T("Enter.wav")
#define EMV_REMOVECARD_WAVE_FILE			_T("EMV_RemoveCard.wav")


/** **********************************************************
*	@brief Define ATM Default Setting Value
************************************************************/
#define DEFAULT_TERMINAL_ID				_T("")
#define DEFAULT_HOSTIP					_T("")
#define DEFAULT_PHONENUMBER				_T("")
#define DEFAULT_TERMINAL_IP				_T("0.0.0.0")
#define DEFAULT_MASTER_PW				_T("111111")
#define DEFAULT_ADMIN_PW				_T("222222")
#define DEFAULT_REPLENISHMENT_PW		_T("333333")
#define AMS_LISTEN_PORT					_T("11859")


/** **********************************************************
*	@brief Define BIN LIST
************************************************************/
#define MAX_BIN_COUNT			100		///< Max Bin 100개 지원


/** **********************************************************
*	@brief Define Input Mode List
************************************************************/
#define UPPERCASE_MODE		_T("UPPERCASE")
#define LOWERCASE_MODE		_T("LOWERCASE")
#define SPECIALCHAR_MODE	_T("SPECIALCHAR")
#define NUMBER_MODE			_T("NUMBER")
#define HEXADECIMAL_MODE	_T("HEXADECIMAL")

/** **********************************************************
*	@brief Define Numeric Input Type List
************************************************************/
#define NORMAL_TYPE		_T("0")	// default
#define DOLLAR_TYPE		_T("1")
#define CENT_TYPE		_T("2")
#define PERCENT_TYPE	_T("3")

/** **********************************************************
*	@brief Define Device Kind
************************************************************/
#define DEVICE_CDR	_T("C")
#define DEVICE_CDM	_T("D")
#define DEVICE_EPP	_T("E")
#define DEVICE_EJL	_T("J")
#define DEVICE_PTR	_T("P")
#define DEVICE_DIO	_T("S")

/** **********************************************************
*	@brief Define CDM Option
************************************************************/
#define	SUPPORT_NOTE_COUNTING		1

#define MAX_CST_COUNT				4
#define MAX_CST_NUMBEROFBILLS		2200

#define REJECT_KIND					9		// long, double, skew, near, more, thin, short, width, testdisp)

#define MAX_NOTE_COUNT				50

#define BILL_INFO_MIN_SIZE			40
#define BILL_INFO_MAX_SIZE			79

#define BILL_INFO_MIN_THICKNESS		30
#define BILL_INFO_MAX_THICKNESS		119

#define MAX_CASSETTE	6					// 2021.03.02 LEH - DEV_CDM_Define.h로부터 위치 이동

/** **********************************************************
*	@brief CDM Return Value
************************************************************/
#define CDM_FAILED_TO_SEND_COMMAND		-1
#define CDM_FAILED_TO_RECEIVE_ACK		-2
#define CDM_FAILED_TO_SEND_ENQ			-3
#define CDM_FAILED_TO_RECEIVE_REPLY		-4
#define CDM_FAILED_TO_SEND_ACK			-5

#define WFS_NOTDISPENSABLE				-6			// 방출시 Mix Fail 발생시 error return 추가
#define WFS_HARDWARE_ERROR				-14


/** **********************************************************
*	@brief Define electronic journal
************************************************************/
#define	MAX_JOURNAL_COUNT			999999


/** **********************************************************
*	@brief Define Key Name
************************************************************/
#define		MASTER_KEYNAME		_T("MASTERKEY")
#define		WORKING_KEYNAME		_T("PINKEY")

#define		STUCKKEY_PERIOD		_T("STUCKKEY_PERIOD")
#define		STUCK_TIME_VALUE	_T("000A")				// 10 sec


/** **********************************************************
*	@brief Define EMV Data
************************************************************/
#define VISA_AID					_T("A0000000031010")
#define VISAELECTRONIC_AID			_T("A0000000032010")
#define VISAPLUS_AID				_T("A0000000038010")
#define USVISA_AID					_T("A0000000980840")
#define MASTERCARD_AID				_T("A0000000041010")
#define MAESTRO_AID					_T("A0000000043060")
#define CIRRUS_AID					_T("A0000000046000")
#define USMAESTRO_AID				_T("A0000000042203")
#define DPAS_AID					_T("A0000001523010")
#define DISCOVER_AID				_T("A0000001524010")
#define AMEX_AID					_T("A00000002501")
#define JCB_AID						_T("A0000000651010")
#define UPDEBIT_AID					_T("A000000333010101")
#define UPCREDIT_AID				_T("A000000333010103")
#define USUP_AID					_T("A000000333010108")
#define DNA_AID						_T("A0000006200620")


#define SUPPORT_EMV						1
#define ACTIVE_EMV_KERNEL_DEBUG			1
#define VISA_TAC_VER_VALUE				_T("D9050010000000DA05DC4004F800D805DC4000A8009F09020096")
#define US_VISA_TAC_VER_VALUE			_T("D9050000000000DA05FFFFFFFFFFD805FFFFFFFFFF9F09020096")
#define MASTER_TAC_VER_VALUE			_T("D9050000980000DA05FC78048000D805FC780480009F09020002")
#define D_PAS_DISCOVERY_TAC_VER_VALUE	_T("D9050010000000DA05FFFFFFFFFFD805FFFFFFFFFF9F09020001")
#define AMEX_TAC_VER_VALUE				_T("D9050000000000DA05FFFFFFFFFFD805FFFFFFFFFF9F09020001")
#define JCB_TAC_VER_VALUE				_T("D9050010000000DA05FC60ACF800D805FC6024A8009F09020200")
#define UP_TAC_VER_VALUE				_T("D9050010000000DA05D84004F800D805D84000A8009F09020020")
#define US_DNA_TAC_VER_VALUE			_T("D9050000000000DA05FC50BCF800D805FC50ACA0009F09020001")
#define OTHER_TAC_VER_VALUE				_T("D9050000000000DA05FFFFFFFFFFD805FFFFFFFFFF9F09020001")

#define TERMINAL_EMV_CONFIG				_T("9F1A0208405F2A0208409F1B04000000009F33036040009F40058F80B050019F3501149F3901059F53015A")

/** **********************************************************
*	@brief Define EMV
************************************************************/
#define MAX_AID_COUNT			60		///< Max AID 60개 지원


/** **********************************************************
*	@brief Update 종류
************************************************************/
#define UPDATE_KIND_SOFTWARE	0
#define UPDATE_KIND_OS			1
#define UPDATE_KIND_BY_AMS		2

#define UPDATE_KIND_SOFTWARE_STRING		_T("0")
#define UPDATE_KIND_OS_STRING			_T("1")
#define UPDATE_KIND_BY_AMS_STRING		_T("2")


/** **********************************************************
*	@brief Registry Path 정보
************************************************************/
#define	ETHERNET_RESIGTRY_PATH			_T("Comm\\ENET1\\Parms\\Tcpip")	
#define ETHERNET_ADAPTER_NAME			_T("ENET1")							
#define SOFTWARE_REGISTRY_PATH			_T("SOFTWARE\\ATM")
#define UPDATE_PROC_REG_PATH			_T("SOFTWARE\\ATM\\UPDATE_PROC")
#define CHECK_ETHERNET_INFO_REG_PATH	_T("SOFTWARE\\ATM\\CHECK_ETHERNET")
#define UNIQUE_KEY_PATH					_T("SYSTEM\\NEXCOM\\ImageInformation")
#define LCD_ROTATION_INFO_PATH			_T("SYSTEM\\NEXCOM\\ImageInformation")

#define CRYPTERA_USB_DRV_REG_PATH			_T("Drivers\\USB\\ClientDrivers\\USBCDC_Template_Default")
#define CRYPTERA_USB_DRV_COMPORT_NAME		_T("Index")
#define CRYPTERA_USB_DRV_COMPORT_VALUE		0
#define CRYPTERA_USB_BUF_IN_REG_NAME		_T("BufferCountIn")
#define CRYPTERA_USB_BUF_IN_REG_VALUE		8					// Default 8
#define CRYPTERA_USB_BUF_OUT_REG_NAME		_T("BufferCountOut")
#define CRYPTERA_USB_BUF_OUT_REG_VALUE		8					// Default 8
#define CRYPTERA_USB_CTRL_IN_BUF_REG_NAME	_T("NumberOfControlInBuffers")
#define CRYPTERA_USB_CTRL_IN_BUF_REG_VALUE	4					// Default 4


/** **********************************************************
*	@brief Max Log File 갯수
************************************************************/
#define MAX_LOG_FILE_COUNT		500

/** **********************************************************
*	@brief M / A / R / AMS Password Encryption Key Value
************************************************************/
#define PASSWORD_KEY_32		_T("958273484971AE4B5729103048569423")

/** **********************************************************
*	@brief Log File 관리 변경을 위해 Version 생성
************************************************************/
#define LOG_MGR_VERSION	_T("V01.00.01")

/** **********************************************************
*	@brief AMS Agent 관련 정보
************************************************************/
//#define AMS_AGENT_VERSION	_T("V01.00.02.00")
#define AMS_AGENT_VERSION	_T("V01.00.03.00")

/** **********************************************************
*	@brief Camera 관련 정보
************************************************************/
#define CAPTURE_IMAGE_FILE_PATH				_T("\\CaptureImage")

/** **********************************************************
*	@brief	Runtime folders that live next to the executable.
*
*	On WinCE the literals above ("\\Trace", "\\Temp", "\\CaptureImage")
*	are rooted at the device root, which is also where the application runs
*	from. On Win32 the very same literal points at the ROOT OF THE DRIVE
*	(e.g. D:\\Trace), not at the folder holding the .exe.
*
*	Use the _ABS macros wherever the path is passed to an API directly.
*	On WinCE they expand to the original literal, so behaviour is unchanged.
************************************************************/
#ifdef _WIN32_WCE

#define		PATH_LOG_ABS					PATH_LOG
#define		PATH_TEMP_ABS					PATH_TEMP
#define		CAPTURE_IMAGE_FILE_PATH_ABS		CAPTURE_IMAGE_FILE_PATH

// The ATM LCD IS the screen, so the real metrics are always correct here.
#define		EAGLE_SCREEN_CX					GetSystemMetrics(SM_CXSCREEN)
#define		EAGLE_SCREEN_CY					GetSystemMetrics(SM_CYSCREEN)

#elif !defined(RC_INVOKED) && defined(__cplusplus)

// Resolves pszSub against the directory of the running executable.
// Deliberately self-contained (no CUtil dependency) so this header stays
// includable from anywhere.
inline CString Eagle_AppPath(LPCTSTR pszSub)
{
	TCHAR szModule[MAX_PATH] = { 0, };

	if (0 == ::GetModuleFileName(NULL, szModule, _countof(szModule)))
		return CString(pszSub);

	szModule[_countof(szModule) - 1] = _T('\0');

	CString strPath(szModule);

	int nPos = strPath.ReverseFind(_T('\\'));

	if (nPos < 0)
		return CString(pszSub);

	return strPath.Left(nPos) + pszSub;
}

#define		PATH_LOG_ABS					Eagle_AppPath(PATH_LOG)
#define		PATH_TEMP_ABS					Eagle_AppPath(PATH_TEMP)
#define		CAPTURE_IMAGE_FILE_PATH_ABS		Eagle_AppPath(CAPTURE_IMAGE_FILE_PATH)

/** **********************************************************
*	@brief	Simulated ATM screen resolution (Win32 only).
*
*	GetSystemMetrics(SM_CXSCREEN/SM_CYSCREEN) reports the developer PC's real
*	monitor resolution, which never matches a "Screen\\<W>_<H>" asset folder.
*	Read the resolution to simulate from DeviceSim.ini instead, so it can be
*	switched (e.g. between the 1024x768 and 1280x800 asset sets already on
*	disk) without a rebuild - same policy as every other simulator setting.
************************************************************/
#include "../../EagleCE_Framework/EagleCE_Framework/IniFile.h"

inline int Eagle_SimScreenCX()
{
	CIniFile ini(Eagle_AppPath(_T("\\DeviceSim.ini")));
	return _ttoi(ini.ReadString(_T("Screen"), _T("Width"), _T("1024")));
}

inline int Eagle_SimScreenCY()
{
	CIniFile ini(Eagle_AppPath(_T("\\DeviceSim.ini")));
	return _ttoi(ini.ReadString(_T("Screen"), _T("Height"), _T("768")));
}

#define		EAGLE_SCREEN_CX					Eagle_SimScreenCX()
#define		EAGLE_SCREEN_CY					Eagle_SimScreenCY()

#endif	// _WIN32_WCE


#define CLIENT_CAPTURE_TEMP_IMAGE				_T("Client_CaptureImage_Temp.jpg")
#define CLIENT_CAPTURE_IMAGE					_T("Client_CaptureImage.jpg")

#define SUPERVIOSR_CAPTURE_TEMP_IMAGE_NAME		_T("Supervisor_CaptureImage_Temp.jpg")
#define SUPERVISOR_CAPTURE_IMAGE_NAME			_T("Supervisor_CaptureImage.jpg")

#define MAX_CAPTURE_IMAGE_FILE_COUNT		10000		// 만 개의 Camera Image만 저장

/** **********************************************************
*	@brief CDR 관련 정보
************************************************************/
#define CDR_ENCRYPT_DEBUG		0
#define CDR_ENCRYPT_MODE		1
#define CDR_ENCRYPT_SUPPORT_VERSION	_T("5790-01B")

/** **********************************************************
*	@brief HOST 관련 정보
************************************************************/
#define PING_TEST_MAX_COUNT		4


/** **********************************************************
*	@brief Resource Format 정보
************************************************************/
#define RESOURCE_FORMAT_TXT		_T("TXT")
#define RESOURCE_FORMAT_SSL		_T("SSL")
#define RESOURCE_FORMAT_CFG		_T("CFG")


/** **********************************************************
*	@brief Config Encryption Key Value
************************************************************/
#define CONFIG_KEY_32		_T("7582B3484971AE4B57291D30485F9423")


#endif	// _EAGLE_COMMON_DEF_H_