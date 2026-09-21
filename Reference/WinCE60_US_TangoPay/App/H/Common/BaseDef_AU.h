#ifndef _BASE_DEF_H_
#define _BASE_DEF_H_

// ----------------------------------------------------------------------------
/////////////////////////           AP MODE DEFINE        /////////////////////
// ----------------------------------------------------------------------------
//#define	APP_LOCAL_MODE		// [#15] NH KGS 2008.03.10 Online-Mode로 동작하도록 수정
//#define	CDU_DECRYPT_MODE	// [#2095] NH KSK 2011.10.27 전시회용 MODE CDU 비 암호화 모드
//#define	APP_AGING_MODE		// AGING 용
// 	#define AGING_DIAL			// AGING OPTION (3개 중 1개만 활성화)
// 	#define AGING_TCPIP			// AGING OPTION (3개 중 1개만 활성화)

// ----------------------------------------------------------------------------
//////////////////////////// Available Options ////////////////////////////////
// ----------------------------------------------------------------------------
#define	APP_STD1_ENHANCED_V2		1	// [#2177] ALL Standard 1 Enhanced Version 2

/////////////////////// TLS Restriction ////////////////////////////////
#define NO_SECURE_TLS_MODE

// ----------------------------------------------------------------------------
///////////////////////// Disabled OR Unavailable Options /////////////////////
//////                           DO NOT ENABLE !!!!!                    ///////
// ----------------------------------------------------------------------------
#define MONIMOBILE_QRCODE			0	// MoniMobile Option
#define APP_TDL_OPTION				0	// [#2291] US/CA CARDTRONICS ONLY(TDL OPTION)
#define APP_CUSTOM_PAI				0	// [#2182] US PAI ONLY (Dual Host DCC)
#define	APP_EVENT_NOTICE			0	// [#2313] US Event Notice
	#define EVENT_NOTICE_TRANONLY	0	// [#2313] US Event Notice SUB MENU
#define APP_ALPHI					1		// [#2497] AU Kook 2017.09.20 ATS ALPHI
	#define ALPHI_DEBUG				1		// [#2497] NH woooZ 2017.09.20  ALPHI server
#define APP_DUALBALANCE				0	// [#2444] US Justin 2016.10.12 DUal Balance Option
#define APP_JUST_CASH				0	// [#2445] US Justin 2016.09.21 Just.Cash Cardless Transaction
#define APP_PAYDIANT_CCA			0	// [#2446] US Justin 2016.09.29 Paydiant (Paypal) CCA(Cardless Cash Access)
#define APP_PIN4_CASHPICKUP			0	// [#2471] US Justin 2017.02.01 Pin4
	#define	APP_PRESTAGIN_PIN4		0	// [#2515] US Justin 2017.11.09 Mastercard phase2 development
#define APP_POPMONEY				0	// [#2471] US Justin 2017.02.01 Popmoney
#define APP_DCC_WITHOPTION			0	// [#2472] NH Justin 2017.02.07 DCC Withdrawal Option
#define APP_GPAY_GIFTCARD_PURCHASE	0	// [#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
#define APP_CUSTOM_CASHDEPOT		0	// [#J007] Cash Depot (QR CODE on Idle Screen)

// ----------------------------------------------------------------------------
// MEMORY CHECK TEST - DEBUG
// ----------------------------------------------------------------------------
//#define MEM_LEAK_CHECK		// Memory leak check
//#define NEXTWARE_BSTRACE_ON	1	// BSTrace On

// ----------------------------------------------------------------------------
// SCREEN VIEW - DEBUG
// ----------------------------------------------------------------------------
//#define AE_SCREEN_DEGISN

// ----------------------------------------------------------------------------
// COUNTRY DEFINE (하나만 1로 설정) 2008.03.03 JSW
// ----------------------------------------------------------------------------
#define	US_VERSION		0	// 미국
#define AU_VERSION		1	// 호주
	#define AU_A_VERSION	1	// 호주 - ATM Plus	//PostBuildCmd.bat에서 ATM_DEALER를 AU_A로 수정해야함.
	#define AU_C_VERSION	0	// 호주 - Customers	//PostBuildCmd.bat에서 ATM_DEALER를 AU_C로 수정해야함.
//#define MX_VERSION		0	// 멕시코
#define CA_VERSION		0	// 캐나다

// ----------------------------------------------------------------------------
// DATE FORMAT DEFINE (하나만 1로 설정) 2008.03.03 JSW
// ----------------------------------------------------------------------------
#define	DATE_MMDDYYYY	0
#define DATE_DDMMYYYY	1
#define DATE_YYYYMMDD	0

// ----------------------------------------------------------------------------
// [#50] NH PSC 2008.03.26 국가별 화폐단위 표시
// ----------------------------------------------------------------------------
//#define CURRENCY_SYMBOL	(TCHAR)0xA3		// 국가별 화폐단위 표시(영국 파운드): L'￡'
#define CURRENCY_SYMBOL	(TCHAR)0x24			// 국가별 화폐단위 표시(미국 달러): L'$'
#define NZ_CURRENCY_SYMBOL	L"NZ$"			// 국가별 화폐단위 표시(뉴질랜드 달러): L'NZ$'	[#2072] AU KSK 2011.06.17

// ----------------------------------------------------------------------------
// [#240] NH 2008.05.28 국가별 Checking Account 표현
// ----------------------------------------------------------------------------
#define	S_ACCOUNT_CHECKING _T("CHEQUE")			// 영국, 호주
//#define	S_ACCOUNT_CHECKING _T("CHEQUING")	// 캐나다, 뉴질랜드
//#define	S_ACCOUNT_CHECKING _T("CHECKING")	// 미국
// end of [#240]

// ----------------------------------------------------------------------------
// [#50] NH PSC 2008.03.26 국가별 화폐단위 for MWI
// ----------------------------------------------------------------------------
//#define CURRENCY_TYPE	_T("USD")			// 국가별 화폐단위 (미국 달러):		'$'
//#define CURRENCY_TYPE	_T("GBP")			// 국가별 화폐단위 (영국 파운드):	'￡'
//#define CURRENCY_TYPE	_T("CAD")			// 국가별 화폐단위 (캐나다 달러):	'$'
//#define CURRENCY_TYPE	_T("NZD")			// 국가별 화폐단위 (뉴질랜드 달러):	'$'
#define CURRENCY_TYPE	_T("AUD")			// 국가별 화폐단위 (호주 달러):		'$'
//#define CURRENCY_TYPE	_T("MXN")			// 국가별 화폐단위 (멕시코 페소):	'$'

// [#2042] AU KSK 2011.04.02
#define CURRENCY_TYPE_NZ	_T("NZD")			// 국가별 화폐단위 (뉴질랜드 달러):	'$'
// end of [#2042]
#define MULTI_CURRENCY_TYPE	_T("USD")		// 국가별 화페단위 (미국달러):	'US$'	// [#514] [MX] KSK 2009.3.4

// ----------------------------------------------------------------------------
// 기종구분 정보 2008.03.27 PSC
// ----------------------------------------------------------------------------
#if (_WIN32_WCE < 0x600)
	#define WINCE_MACHINETYPE	_T("1800CE")	// 1800CE
#endif

// ----------------------------------------------------------------------------
// RMS 기종 구분 코드 2008.04.25 AIREAT
// ----------------------------------------------------------------------------
// [#481] [NH] KSK 2009.1.9
//#define WINCE_RMS_MACHINECODE	0x08		//   5000 CE - US
//#define WINCE_RMS_MACHINECODE	0x32		//   5000 CE - CA
//#define WINCE_RMS_MACHINECODE	0x39		// NH1800 CE - UK
//#define WINCE_RMS_MACHINECODE	0x3A		// NH1800 CE - NZ
//#define WINCE_RMS_MACHINECODE	0x3C		// NH1800 CE - AU
//#define WINCE_RMS_MACHINECODE	0x3D		// NH1800 CE - MX

#define WINCE_RMS_5000_MACHINECODE				0x00	// 5000 CE - AU 기기 나간적 없음
#define WINCE_RMS_5300_MACHINECODE				0x00	// 5300 CE - AU 기기 나간적 없음
#define WINCE_RMS_1800_MACHINECODE				0x3C	// 1800 CE - AU

#define WINCE_RMS_2700_MACHINECODE_FRONT		0x47	// NH2700CE FRONT
//#define WINCE_RMS_2700_MACHINECODE_REAR		0x47	// NH2700CE REAR
#define WINCE_RMS_1800SE_MACHINECODE_FRONT		0x00	// NH1800SE FRONT - AU 기기 나간적 없음 (향후도 없음)
#define WINCE_RMS_MX4000W_MACHINECODE			0x5C	// [#2097] US PCS 2011.11.15 "WallMount"
#define WINCE_RMS_MX5000SE_MACHINECODE			0x00	// [#2097] US PCS 2011.11.15 "MX5000SE" - 추가 정의 필요
#define WINCE_RMS_HALO2600_MACHINECODE			0x62	// [#2222] US Justin 2013.10.03 Add HALO
#define WINCE_RMS_NH1500SE_MACHINECODE			0x00	// [#2267] US KSK 2014.05.12
#define WINCE_RMS_MX5200SE_MACHINECODE_FRONT	0x69	// [#2303] US KSK 2014.11.07		// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
#define WINCE_RMS_HALO2_MACHINECODE				0x6F	// [#2332] NH Justin 2015.02.10 Add Halo2 Machine code (111)
#define WINCE_RMS_MX5300SE_MACHINECODE			0x73	// [#2383] NH Justin 2015.12.18 Add 5300 SE Machine Code (115)
#define WINCE_RMS_MX2800SE_MACHINECODE			0x78	// [#2529] NH Justin 2018.01.23 Add 2800 SE Machine Code (120)
#define WINCE_RMS_MX2800T_MACHINECODE			0x7C	// [#GLDV-2505] US Kook 2019.05.22  Add 2800 T Machine Code (124 for AU)

#define WINCE7_RMS_COMMON_MACHINECODE			0x80	// All WinCE 7.0 machines

// ----------------------------------------------------------------------------
// 축퇴디바이스 설정 2008.04.03 PSC
// ----------------------------------------------------------------------------
//#define DEV_AUTO_OFF_CDU	0				// CDU Auto-Off
#if AU_C_VERSION
	#define DEV_AUTO_OFF_CDU	0				// CDU Auto-Off	[#2254] AU KSK 2014.02.24
#else
	#define DEV_AUTO_OFF_CDU	1				// CDU Auto-Off	[#2254] AU KSK 2014.02.24
#endif

#define DEV_AUTO_OFF_SPR	1				// SPR Auto-Off

// ----------------------------------------------------------------------------
// VERSION INFORMATION
// ----------------------------------------------------------------------------
	/*------------------------------------------------
					WINDOWS CE 60
	------------------------------------------------*/

/*	// [#2212] KMK 2013.07.15 소스 통합(V06.00.02) 이후 버전 관리 일원화 (AP_VERSION)
	#if (AU_C_VERSION)
		//#define AP_VERSION		_T("V06.00.00")		// Customers 호주 APCA 초기 인증 버전
		//#define AP_VERSION		_T("V06.00.01")		// Voice Guidance 기능 추가 (내부 UAT 인증 완료)
		//#define AP_VERSION		_T("V06.00.02")		// 뉴질랜드 인증 버전
	#else
		//#define AP_VERSION		_T("V06.00.00")			// ATMPLUS(CASHPLUS) 호주 인증 버전
		//#define AP_VERSION		_T("V06.00.01")			// DFL(CASHPLUS) 호주 인증 버전
		#define AP_VERSION		_T("V06.00.02")			// DFL(CASHPLUS) 뉴질랜드 인증 버전
	#endif
*/
	//#define AP_VERSION			_T("V06.00.03")		// [#2200] AU KMK 2013.06.10 PAN Data 수정
													// [#2133][#2134][#2135] NH KSK 2012.05.07 Working Key Fail 보완 처리 (Middleware 적용)
													// [#2211] AU KMK 2013.07.15 NH2700T 지원
	//#define AP_VERSION			_T("V06.00.04")		// [#2220] AU KMK 2013.10.07 호주 EMV 거래 지원
	//#define AP_VERSION			_T("T06.00.04")		// [#2270] AU KSK 2014.05.26
	//#define AP_VERSION				_T("V06.00.01")		// [#2270] AU KSK 2014.06.11 영업 및 고객 요청으로 FDI 인증 버전 V06.00.01 R10으로 버전 변경
	//#define AP_VERSION				_T("V06.00.02")		// 호주 V06.00.01 R12 내용을 뉴질랜드 발행을 위해 버전만 변경
	//#define AP_VERSION				_T("V06.00.01")		// 호주 V06.00.01 R13 TLS Support
	#if (AU_A_VERSION)
//	#define AP_VERSION				_T("V06.11.00")		// 호주 EMV 인증을 위해 Main Version Change 2015.01.15
//	#define AP_VERSION				_T("V06.11.01")		// [#2362] DCC 거래 추가
		//#if (APP_STD1_ENHANCED_V2)
		//	#define AP_VERSION				_T("V06.11.01")		// [#2362] EMV + DCC 거래 추가
		//#else
		//	#define AP_VERSION				_T("V06.11.00")		// EMV Support
//	#define AP_VERSION				_T("V06.11.00")		// EMV + DCC Support

//	#define AP_VERSION				_T("V86.11.00")		// [#2494] AU Kook 2017.08.01, ATS Project - Gaming Expo Demo
	#define AP_VERSION				_T("V06.11.00")		// [#2497] AU Kook 2017.08.24, ATS Project - Official (V06.11.00 R18)
														// [#2525] AU HJHAN 2018.01.10 Print Option 추가 (TOTAL AMOUNT En/Disable ) - AU ONLY
														// [#2497] woooZ 2018.01.25 ALPHI SSL CERT. option 추가 - AU ONLY
														// [#2532] AU Kook 2018.02.09 Note Counting Disabled 시 Trial Cassette Total 명세표에서 Reject Count가 0으로 출력되는 현상 수정

		//#endif
	#else
//	#define AP_VERSION				_T("V06.01.00")		// 호주 EMV 인증을 위해 Main Version Change 2015.01.15
	#define AP_VERSION				_T("V06.01.01")		// [#2362] DCC 거래 추가
	#endif

/*	// KMK 2013.07.15 소스 통합(V06.00.02) 이후 버전 관리 일원화 (AP_DETAIL_VERSION)
//	#define AP_DETAIL_VERSION	6,0,0,0				// 호주 인증 버전 
//	#define AP_DETAIL_VERSION	6,0,0,1				// ub, ^ 필드 버그 Working download 저널 버그 수정 2010.11.29 
//	#define AP_DETAIL_VERSION	6,0,0,2				// 1. SSL Version 접속 오류 수정
//													// 2. PCI2.0에서 KeyMode 강제 설정 오류 수정
//													// 3. PinBlock 생성 위치 변경 (SP : V06.04.04 필히 적용해야함)
	#if (AU_C_VERSION)
	//#define AP_DETAIL_VERSION	6,0,0,0				// 1. APCA 최초 인증 버전
	//#define AP_DETAIL_VERSION	6,0,1,0				// 1. Voice Guidance 기능 추가
													// 2. CDU SETUP에 뉴질랜드 국가 추가
													// 3. NVRAM Backup/Restore시 Multi Bin 영역 추가
	//#define AP_DETAIL_VERSION	6,0,1,1				// 1. Voice Guidance 추가 수정 (Customers 요청사항)
	//#define AP_DETAIL_VERSION	6,0,1,2				// 1. Keymgr의 EPPCommunication.dll 버그로 인해 재 build
	//#define AP_DETAIL_VERSION	6,0,1,3				// 1. Reset Master Password 기능 추가
	//#define AP_DETAIL_VERSION	6,0,1,4				// 1. Reset Master Password 기능 추가 (512K일때 Clear 안됨)
													// 2. Reset Master Password 기능이 Function에서 안먹는 Bug Fix
													// 3. Surcharge Screen에서 Fast Key "8" - Continue가 되도록 추가
													// 4. Reason For Reversal의 값이 "01"로 올라가지 않는 Bug Fix
	//#define AP_DETAIL_VERSION	6,0,2,0				// 1. MultiBin 사라지는 Bug Fix(MB2500Dll 적용) 및 SPS 특이사항 적용
													// 2. KeyMgr 정식 발행 버전 적용
													// 3. Modem Initial String Default값 변경
													// 4. Denomination에서 2CST 이상 선택 시 권종 표시 오류 수정
													// 5. NH2700 L에 space 제거 (OP MAIN에서 NEW ZEALAND 설정 시 글자 짤림 대책)
													// 6. NVRAM Clear시 비정상 종료되는 Bug Fix (MB2500.dll bug 수정됨)
													// 7. 뉴질랜드 (Need More Time 기능 제거 - Paymark 인증 항목)
	//#define AP_DETAIL_VERSION	6,0,2,1				// 1. 뉴질랜드 (수수료 확인 화면 문구 변경 - Paymark 인증 항목)
													// 2. 뉴질랜드 (권종 표시 변경 -> 'NZ$' - Paymark 인증 항목)

	#else
	//#define AP_DETAIL_VERSION	6,0,0,0				// 1. ATMPLUS AU SPS/NZ ETSL 인증버전 (초기버전)
													// 1. u! Balance Inquiry Surcharge 추가
	//#define AP_DETAIL_VERSION	6,0,0,1				// 1. S/W update시에 MB2500 / Cmnlib file delete / copy fail나는 현상 수정
	//#define AP_DETAIL_VERSION	6,0,0,2				// 1. 호주 Customers 6.0.1.4 내용 모두 적용
													// 2. Cash Image 공용으로 변경
													// 3. OP Main에 Receipt 오타 Image 변경
	//#define AP_DETAIL_VERSION	6,0,1,0				// 1. SPS 특이사항 적용 (EJ에 Card Number xxxxxxxxxxxx****)로 남기도록 수정
													// 2. Multi Bin 사라지는 Bug 수정을 위해 MB2500Dll 변경
													// 3. KeyMgr 정식 Version 적용
													// 4. Modem Initial String Default값 변경
													// 5. Denomination에서 2CST 이상 선택 시 권종 표시 오류 수정
													// 6. NH2700 L에 space 제거 (OP MAIN에서 NEW ZEALAND 설정 시 글자 짤림 대책)
													// 7. NVRAM Clear시 비정상 종료되는 Bug Fix (MB2500.dll bug 수정됨)
	//#define AP_DETAIL_VERSION	6,0,2,0				// 1. 뉴질랜드 (Need More Time 기능 제거 - Paymark 인증 항목)
	#define AP_DETAIL_VERSION	6,0,2,1				// 1. 뉴질랜드 (수수료 확인 화면 문구 변경 - Paymark 인증 항목)
													// 2. 뉴질랜드 (권종 표시 변경 -> 'NZ$' - Paymark 인증 항목)
	#endif
*/
	//#define AP_DETAIL_VERSION	6,0,3,0		// 1. [#2200] PAN Data 저널 저장 오류 수정
											// 2. [#2133] Cancel Read Data 장애 시 MWI 보완 처리
											// 3. [#2134] Working Key Fail시 PIN SP 재기동 처리
											// 4. [#2135] PIN or MAC 값이 SPACE인 경우 거래 취소 처리
	//#define AP_DETAIL_VERSION	6,0,3,1		// 1. PAN Data 끝 1자리 누락 현상 수정
	//#define AP_DETAIL_VERSION	6,0,3,2		// 1. [#2211] 호주 NH2700T 지원 (Next Payments)
	//#define AP_DETAIL_VERSION	6,0,3,3		// 1. 호주 NH2700T MoniView에서 등록 안되는 현상 수정
	//#define AP_DETAIL_VERSION	6,0,4,0		// 1. [#2220] 호주 EMV 거래 지원
											// 2. [#2234] MoniView Timeout 기본값 수정 및 변경 지원
											// 3. [#2238] CDU Sensor Log
											// 4. [#2239] MoniView Log Backup (\CMD=LOGBACKUP)
											// 5. [#2244] SPR Reset
											// 6. [#2241] CDU-M 논리매수 차감 Option 추가
											// 7. [#2247] 명세표 형식 일부 수정 (Add Cash, Cst Total 등에서 Amount 표시)
//	#define AP_DETAIL_VERSION	6,0,4,1		// 1. 내부검사 지적사항 대응
//	#define AP_DETAIL_VERSION	6,0,4,2		// 1. TEST 버전
//	#define AP_DETAIL_VERSION	6,0,4,3		// 1. TEST 버전
//	#define AP_DETAIL_VERSION	6,0,4,4		// 1. TEST 버전
//	#define AP_DETAIL_VERSION	6,0,1,10	// [#2270] AU KSK 2014.06.11 영업 및 고객 요청으로 FDI 인증 버전 V06.00.01 R10으로 버전 변경
//	#define AP_DETAIL_VERSION	6,0,1,11	// [#2270] AU KSK 2014.06.11 내부검사중 Reject다발시 잔여매수 갱신 안되는 Bug Fix
//	#define AP_DETAIL_VERSION	6,0,1,12	// [#2293] AU KSK 2014.09.17 고객 요청사항 대응
//	#define AP_DETAIL_VERSION	6,0,2,5		// 호주 V06.00.01 R12 내용을 뉴질랜드 발행을 위해 버전만 변경
//	#define AP_DETAIL_VERSION	6,0,1,13	// 호주 V06.00.01 R13 TLS Support
//	#define AP_DETAIL_VERSION	6,0,1,14	// TTW기종에서 OP Timeout시 Inservice 전환하도록 수정(고객 요청사항)
											// Digital Receipt Option 처리
											// MoniView Agent Version Degrade (예전 버전의 Register 등록 오류 대응을 위해)
//	#define AP_DETAIL_VERSION	6,0,1,15	// [#2353] Queue 처리 문제로 인해 SSL 통신 중 간헐적 Holding 현상 수정
											// [#2354] VDM에 카세트별 Reject / Dispense 매수 표시하도록 수정
//	#define AP_DETAIL_VERSION	6,1,0,0		// 호주 V06.01.00 R0 호주 EMV 지원

	#if (AU_A_VERSION)
//	#define AP_DETAIL_VERSION	6,11,0,0	// 호주 V06.11.00 R0 호주 EMV 지원
//	#define AP_DETAIL_VERSION	6,11,0,1	// 호주 V06.11.00 R1 NH2700T Op Timeout시 In Service 처리, Decimal Surcharge 지원, Digital Receipt Option 처리, SSL Option 처리 변경 (ABOVE -> UP TO)
//	#define AP_DETAIL_VERSION	6,11,0,6	// 호주 V06.11.06 DCPayments V06.01.00 R6 적용
//	#define AP_DETAIL_VERSION	6,11,0,7	// 호주 V06.11.07 DCPayments V06.01.00 R7 적용
//	#define AP_DETAIL_VERSION	6,11,0,8	// Fallback 거래시 Card를 먼저 제거하는 경우에 대한 처리 보완
//	#define AP_DETAIL_VERSION	6,11,0,9	// MWI Bug Fix
											// [#2434] AU Kook 2016.07.18	Support Multi Languages,
											//								Speed Up AP Loading, Revise Print All Setup.
//	#define AP_DETAIL_VERSION	6,11,0,10	// [#2434] 내부검사 지적사항 수정
//	#define AP_DETAIL_VERSION	6,11,0,11	// 2016.08.26 eftpos AID 추가
//	#define AP_DETAIL_VERSION	6,11,0,12	// 2016.09.29 New Kernel V6.0 적용
//	#define AP_DETAIL_VERSION	6,11,0,13	// [#2462] AU 2016.12.16 HOST Declined시 IC Data 저널 저장 로직 보완
		//#if (APP_STD1_ENHANCED_V2)
			//#define AP_DETAIL_VERSION	6,11,1,14	// [#2463] CST Total시 Reject 매수 보완처리 누락 Bug Fix
													// [#2458] Sanyko Dip Card Reader Latch Option 기능 추가
													// [#2425] US Justin 2016.05.23 EMV Language Selection
													// Change RMSAgent Version (V010329)
													// Trunk Merge
													// [#2459] AU KSK 2016.12.26 Dispense Mix 기능 추가
			//#define AP_DETAIL_VERSION	6,11,1,15	// 
		//#else
			//#define AP_DETAIL_VERSION	6,11,0,14	// [#2463] CST Total시 Reject 매수 보완처리 누락 Bug Fix
													// [#2458] Sanyko Dip Card Reader Latch Option 기능 추가
													// [#2425] US Justin 2016.05.23 EMV Language Selection
													// Change RMSAgent Version (V010329)
													// Trunk Merge
													// [#2459] AU KSK 2016.12.26 Dispense Mix 기능 추가
			//#define AP_DETAIL_VERSION	6,11,0,15	// [#2463] CST Total시 Reject 매수 보완처리 누락 Bug Fix (미발행)
		//#endif
// 	#define AP_DETAIL_VERSION	6,11,0,15	// 1. CST Total시 Reject 매수 보완처리 누락 Bug Fix
 											// 2. 기존 VDM Main Menu 화면 적용
 											// 3. Over Dispense 문구 제거
 											// 4. Digital Receipt Option이 Disable일 때 해당 Menu가 표시되는 현상 수정
 											// 5. APCA 규정(수수료 문구 표시) 적용
//	#define AP_DETAIL_VERSION	6,11,0,16	// [#2493] AU Kook 2017.07.26, MoniView - Last Transaction Time Update 미갱신 현상 대책

//	#define AP_DETAIL_VERSION	86,11,0,16	// [#2494] AU Kook 2017.08.01 ATS Project - Gaming Expo Demo
//	#define AP_DETAIL_VERSION	6,11,0,17	// [#2501] AU Kook 2017.08.23 고객 요청으로 DCC 거래 시 Total Amount 인자하도록 수정
											//                            (추후 인자 En/Disable 옵션화 예정)
// 	#define AP_DETAIL_VERSION	6,11,0,18	// [#2497] AU Kook 2017.08.24 ATS Project - Official
//	#define AP_DETAIL_VERSION	6,11,0,19	// [#2497] AU Kook 2017.12.18 ATS ALPHI Version Up
											// [#2525] AU HJAHN 2018.01.10 Print Option 추가 (TOTAL AMOUNT En/Disable ) - AU ONLY
//	#define AP_DETAIL_VERSION	6,11,0,20   // [#2497] woooZ 2018.01.25 ALPHI SSL CERT. option 추가 - AU ONLY
//	#define AP_DETAIL_VERSION	6,11,0,21	// [#2532] AU Kook 2018.02.09 Note Counting Disabled 시 Trial Cassette Total 명세표에서 Reject Count가 0으로 출력되는 현상 수정
//	#define AP_DETAIL_VERSION	6,11,0,22   // ALPHI BIT 62 test version
//	#define AP_DETAIL_VERSION	6,11,0,23   // [#2544] AU HJAHN 2018.04.09
											// 1. Change Popup text and icon when IC card is removed before transaction is completed.
											// 2. Change button name (OP 741)
											// 3. Fix bug when setting NH EMV ( Sankyo ) 
// 	#define AP_DETAIL_VERSION	6,11,0,24   // [#2579] AU Kook 2018.10.10 Support ALPHI SSL Configuration via MoniView
//	#define AP_DETAIL_VERSION	6,11,0,25   // [#2550] AU HJAHN 2018.08.03 IC 거래 중 카드 제거시, 재시도 알림 팝업 표시, StatusOKIfCardNotPresent 값 변경
											// [#2567] AU HJAHN 2018.07.25 CLOG 파일 저장 실패로 인한 파일 저장 방식 변경( CLog 0kb 오류 )
											// [#2570] AU HJAHN 2018.08.17 NEXTWARE Log ON/OFF 방식 변경 
											// [#2575] AU HJAHN 2018.09.07 codesonar 지적 사항 수정
	#define AP_DETAIL_VERSION	6,11,0,26   // [#2580] AU Kook 2018.11.06 RMS TEST (SILENT SEND with LAST 5 JOURNALS)
											// [#2581] AU Kook 2018.12.28 Requests from Next Payments.
											//		- Add a new journal for the case of cancellation of DCC transaction.
											//		- Support AUD 5 / NZD 5. (CurrencyInfo4CE.INI needs to be updated)
											//		- Allow SSL Cert. Option. (developed item from US version)
	#else
//	#define AP_DETAIL_VERSION	6,1,0,1		// 호주 V06.01.00 R1 호주 EMV 지원 (AID List에서 Cancel시 POPUP 대책, 재컴파일함)
//	#define AP_DETAIL_VERSION	6,1,0,2		// 호주 V06.01.00 R2 NH2700T Op Timeout시 In Service 처리, Decimal Surcharge 지원, Digital Receipt Option 처리, SSL Option 처리 변경 (ABOVE -> UP TO)
											// Remote AID 기능 / EMV Data 추가 Journal 저장 / Brand Name 지원 / QR Receipt Option 처리
//	#define AP_DETAIL_VERSION	6,1,0,3		// Network 통신 Queue Bug Fix 적용
											// 고객 요청에 의해 AU_C는 CDU 축퇴 모드 미지원하도록 수정
//	#define AP_DETAIL_VERSION	6,1,0,4		// 1. 명세표에 Total Amount 값 복원 (NH1800CE와 동일하게 Format 수정)
											// 2. NH2700TTW에서 Brand Name 미표시 수정
											// 3. PrintAllSetup에 Digital Receipt Option 추가
											// 4. Surcharge Owner Center 표시 (NH1800CE와 동일하게 Format 수정)
											// 5. Operator Fee Owner 와 Marketing Message 사이에 빈 Line 1개 추가 (NH1800CE와 동일하게 Format 수정)
											// 6. RMS Write Setup Fail시 Rebooting 하지 않도록 변경
											// 7. Balance Inquiry EMV 거래 중 카드 미 수취시 정상 처리되는 Bug Fix
											// 8. Power Off시 Reversal이 되지 않는 Bug Fix
//	#define AP_DETAIL_VERSION	6,1,0,5		// 1. Assorted Field의 Withdrawal / Balance Inquiry Surcharge값을 Setting하지 못하는 Bug Fix
//	#define AP_DETAIL_VERSION	6,1,0,6		// 1. Hyosung Message인 경우 Surcharge 관련 Field V06.01.xx(DCC)에 적용된 항목 추가 적용
											// 2. Power Off 처리 관련 V06.01.xx(DCC)에 적용된 항목 추가 적용
											// 3. Domestic Bin 기능 추가
//	#define AP_DETAIL_VERSION	6,1,0,7		// 1. IC Bug Fix
//	#define AP_DETAIL_VERSION	6,1,0,8		// 1. IC Bug Fix IC 거래시 JNL CARD DATA 저장 오류 수정
//	#define AP_DETAIL_VERSION	6,1,1,0		// DCC 지원
//	#define AP_DETAIL_VERSION	6,1,1,1		// [#2362] AU KSK 2015.08.03 DCC 인증 중 Bug Fix
//	#define AP_DETAIL_VERSION	6,1,1,2		// 1. 명세표에 Total Amount 값 복원 (NH1800CE와 동일하게 Format 수정)
											// 2. NH2700TTW에서 Brand Name 미표시 수정
											// 3. Surcharge Owner Center 표시 (NH1800CE와 동일하게 Format 수정)
											// 4. Operator Fee Owner 와 Marketing Message 사이에 빈 Line 1개 추가 (NH1800CE와 동일하게 Format 수정)
											// 5. RMS Write Setup Fail시 Rebooting 하지 않도록 변경
											// 6. TEST DISPENSE시에 매수 정보 표시 추가
											// 7. Balance Inquiry EMV 거래 중 카드 미 수취시 정상 처리되는 Bug Fix
											// 8. Power Off시 Reversal이 되지 않는 Bug Fix
											// 9. NHS "fc" / "fi" 설정 관련 Bug Fix
											// 10. HOST 송/수신 Message에 Random Mac Field 누락 Bug Fix
//	#define AP_DETAIL_VERSION	6,1,1,3		// 1. DF Request시 Random Mac Field 위치 오류 수정										
//	#define AP_DETAIL_VERSION	6,1,1,4		// 1. 미주에서 발생한 간헐적 AP Loading Fail 현상 적용
											// 2. DCPayments 요청에 의해 DCC Information Layout 변경 (고객별 Customizing 필요) 
//	#define AP_DETAIL_VERSION	6,1,1,5		// 1. AU_C_VERSION STANDARD1에서 TERMINAL ID를 15자리 입력받도록 수정 (DCPayments요청사항)
//	#define AP_DETAIL_VERSION	6,1,1,6		// 1. 명세표 Format을 NH1800CE와 동일하게 맞춤
											// 2. Domestic Bin Version 추가
//	#define AP_DETAIL_VERSION	6,1,1,7		// DCC, EOT/ENQ, MESSAGE FORMAT값을 MoniView에서 Setting 가능하도록 수정
//	#define AP_DETAIL_VERSION	6,1,0,9		// 1. DomesticBin File 관리를 위해 MWI 가운데를 Revision으로 사용하도록 변경
											// 2. Receipt Header가 있을 경우 NH1800CE와 동일하게 1 Line Space 처리
//	#define AP_DETAIL_VERSION	6,1,0,10	// 1. DomesticBin File이 Read Only일 때 Copy File이 fail 나는 Bug Fix
											//    . File 속성 변경시 Delay 100ms 추가함
//	#define AP_DETAIL_VERSION	6,1,0,11	// 1. DomesticBin File이 Read Only일 때 Copy File이 fail 나는 Bug Fix (추가수정) - 6.1.0.11에서 종료, 이후부터 V6.1.1.x로 발행
//	#define AP_DETAIL_VERSION	6,1,1,8		// 1. RKT Random Number 처리 보완
											// 2. 'w' field 추가
											// 3. V06.11.00.R11 (eftpos AID 추가) 항목 적용 (Print All Setup 제외)
//	#define AP_DETAIL_VERSION	6,1,1,9		// 1. New Kernel V6 적용
	#define AP_DETAIL_VERSION	6,1,1,10	// [#2468][#2469] AU KSK 2017.01.18 Added Multi Language, EMV Language Selection.
	#endif


//#define RMS_VERSION		0x010317
//#define RMS_VERSION		0x010318			// [#558] KSK 2009.08.17
//#define RMS_VERSION		0x010318			// [#558] KSK 2009.08.17
												// [#2220] AU KMK 2014.03.01 RMS 호환성 유지를 위해 버전을 010318로 유지 (추후 MoniView 업데이트 시 0x010321 버전으로 변경 예정)
//#define RMS_VERSION		0x010319			// [#2206] NH Justin 2013.06.27 Add RMS Setup info - Dynamic Flow, Dual Host DCC
//#define RMS_VERSION		0x010320			// Add Justin First Call Option
//#define RMS_VERSION		0x010321			// [#2255] NH KSK 2014.02.26 Add Note Counting Option, 현재 MoniView에서는 0x010321 버전 지원 안함 (업데이트 필요) [ #2293]
//#define RMS_VERSION		0x010325			// [#2320] Justin NEW SSL/TLS VERSION MANAGEMENT (ABOVE => UP TO)
//#define RMS_VERSION			0x010326			// [#2342] NH Justin 2015.05.11 AID SELECTION
//#define RMS_VERSION		0x010327			// [#2359] NH Justin 2015.07.15 Add Moniview Setup (Percentage Surcharge Format)
//#define RMS_VERSION		0x010328			// [#2410] NH Justin Support MoniMobile En/Disable,  Support En/Disable EMV through individual command
//#define RMS_VERSION		0x010329			// [#2435] NH Justin Add Kernel version info in Setup, MV will check this version for allowing Kernel version change
//#define RMS_VERSION		0x010333					// [#2497] AU Kook 2017.10.20 ATS ALPHI
//#define RMS_VERSION		0x010337					// [#2579] AU Kook 2018.10.12 ATS ALPHI SSL Options
//#define RMS_VERSION		0x010340					// [#2580] AU Kook 2019.02.13 NextPayments Requests (Silent Send)
														// [#2580] AU Kook 2019.07.08 NextPayments Requests (RKT Serial Number)
//#define RMS_VERSION		0x010342					// [#GLDV-2681] AU Kook 2019.11.06 Print Total Amount
//#define RMS_VERSION		0x010343					// [#RWC6-119] US William 2020.01.16 Add JustCash RMS changes / [#RWC6-56] US William 2020.02.07 Hashed Passwords
#define RMS_VERSION		0x010344					// [#GLDV-2792] AU Kook 2020.03.13 RMS Timeout configuration via MoniView


//#define MWI_VERSION		_T("V06.03.00")
//#define MWI_VERSION		_T("V06.03")				// [#2321] NH KSK 2015.01.14 AP Revision 처리를 위해 Version을 2개만 Set함
#define MWI_VERSION		_T("V06")				// [#2321] NH KSK 2015.01.14 AP Revision 처리를 위해 Version을 2개만 Set함

#define	NVRAM_ERRSTACK_VERSION	"V010000"			// [#419] [NH] KSK 2008.9.16
#define NVRAM_REJTOTAL_VERSION	"V010100"			// [#534] [NH] AIREAT 2009.06.04

//#define JNLDB_VERSION			(0x56010000)		// [#] NH AIREAT 2008.10.29
#define JNLDB_VERSION			(0x56010001)		// [#462] [NH] KSK 2008.12.08 이전 Journal File을 삭제하지 못하는 Bug Fix


// ----------------------------------------------------------------------------
// Support Software TTS					[#2368] US KSK 2015.08.31 Support Software TTS
// ----------------------------------------------------------------------------
#define SUPPORT_SOFTWARE_TTS			0

// ----------------------------------------------------------------------------
// ADA PATH DEFINE	[#157] KSK 2008.04.22
// ----------------------------------------------------------------------------
#define	ADA_WAVE_FILE_PATH		_T("\\ATM\\Wave\\")
#define	ADA_WAVE_DAT_FILE		_T("WaveLoad.dat")		//[#2077] CA PCS 2011.07.02
#define ADA_TTS_FILE_PATH		_T("\\ATM\\TTS\\")		// [#2368] US KSK 2015.09.01
#define	ADA_TTS_DAT_FILE		_T("VoiceText.dat")		// [#2368] US KSK 2015.09.01
#define ADA_TTS_DB_FILE_PATH	"\\ATM\\TTS_DB\\EN_US"	// [#2368] US KSK 2015.09.01

//-----------------------------------------------------------------------------
// ALPHI SSL CERT Path. [#2497] AU Kook ATS ALPHI
//-----------------------------------------------------------------------------
#define SSL_CERT_SRCFILE_ALPHI	"\\USB\\rootcert_alphi.pem"
#define SSL_CERT_DESFILE_ALPHI	"\\ATM\\rootcert_alphi.pem"
#define	ALPHI_LOG_FILE			_T("\\ATM2\\LOG\\ALPHI.LOG")				// [#2497] AU Kook 2018.02.22 ALPHI

// ----------------------------------------------------------------------------
//	Cash Condition Amount Define					[#273] [NH] KSK 2008.6.9 Basedef.h로 이동
// ----------------------------------------------------------------------------
#define CDU_MCASH_MAXOUT	5000					// MAX OUT : $999 -> $2000 // AIREAT 2009.06.05  ->[#0542]2500 SOOK 2009.07.01	[#2038] AU KSK 2011.03.31
#define CDU_MCASH_MINOUT	1						// MIN OUT : $1
#define CDU_CST_MAXFASTCASH	6						// MAXIMUM FAST CASH COUNT
#define	CDU_MAX_CST_COUNT	4						// [#281] [NH] JSW 2008.6.10

#define CST_MAXDISPENSE		50						// MAXIMUM DISPENSE COUNT //[#0542]SOOK 2009.07.01
#define CST_W_MAXDISPENSE	20						// [#2098] US KSK 2011.11.28
#define CDU_NEARCASHCNT		100						// NEAR CASH COUNT	
#define CDU_FULLREJECTCNT	200						// FULL REJECT COUNT

// ----------------------------------------------------------------------------
//	AU AVAILABLE DENOMINATION					[#282] [NH] KSK 2008.6.11 국가별 권종 DEFINE 추가 및 MWI 적용
// ----------------------------------------------------------------------------
#define CASH_DENOMINATION1	10					// $10
#define CASH_DENOMINATION2	20					// $20
#define CASH_DENOMINATION3	50					// $50
#define CASH_DENOMINATION4	100					// $100
#define	CASH_DENOMINATION5	5					// not use	// [#2581] AU Kook 2018.12.28 Support AUD $5
#define	CASH_DENOMINATION6	0					// not use
#define	CASH_DENOMINATION7	0					// not use
#define	CASH_DENOMINATION8	0					// not use
// end of [#282]

// ----------------------------------------------------------------------------
// SCREEN TIMEOUT [#236] AIREAT 2008.05.28
// ----------------------------------------------------------------------------
#define NORMAL_SCR_TIMEOUT		30		// OP 화면 타임아웃.
#define INFO_SCR_TIMEOUT		5		// 경고시 안내문구 타임아웃.
#define ABORT_SCR_TIMEOUT		3		// 최소시 안내문구 타임아웃.	// AIREAT 2009.06.03 : 5->3 sec

// ----------------------------------------------------------------------------
// NVRAM Default Value.
// ----------------------------------------------------------------------------
#define TERMINAL_ID_DEFAULT			L"00000000"	// [#398] [NH] KSK 2008.8.6
#define	HOSTPHONE1_DEFAULT			L"0"		// [#426] [NH] KSK 2008.9.24
#define	HOSTPHONE2_DEFAULT			L"0"		// [#426] [NH] KSK 2008.9.24
#define	MASTERPW_DEFAULT			L"1:0:$0$r4pukaCWjfac8vjA$FmJ6L1SpHm9o6X3hITSPCbRTDnJJT7N0iX/KpbsDIUco2phcuDUBjrVmH2xYz0n14tXaYqyJeB3ApAu4CkhuNA==:0"			// [#419] [NH] KSK 2008.9.18
#define	SERVICEPW_DEFALUT			L"1:2:$0$0FPjibtA0VJmHIQU$vNW3WslZw3q9i2kI32ZZ7nQNCa4meSe4nKUunG/IVMDgLl63/bxOjcZUfdKzjsqEISH2k0MM6LEHjqrskKetHg==:0"			// [#419] [NH] KSK 2008.9.18
#define	OPERATORPW_DEFALUT			L"1:1:$0$rbYkJwIPRzc6yw8O$yUn1euUeQS61z28dnHypbFAVvviK10U64Ya06tczM48KXxNTmndL7giNqfe7Kw02JiCzwSzDz1Crdy+C3o1pTA==:0"			// [#419] [NH] KSK 2008.9.18

#define	DISPENSELIMIT_DEFAULT		L"020000" //[#542] AU_C SOOK 2009.07.01 CUSTOMERS Default 값 적용	// [#2038] AU KSK 2011.03.31
#define	FASTCASH1_DEFAULT			20
#define	FASTCASH2_DEFAULT			40
#define	FASTCASH3_DEFAULT			60
#define	FASTCASH4_DEFAULT			80
#define	FASTCASH5_DEFAULT			100
#define	FASTCASH6_DEFAULT			200
#define	SURCHAREAMOUNT_DEFAULT		L"00000200" //[#542] AU_C SOOK 2009.07.01 CUSTOMERS Default 값 적용	[#2038] AU KSK 2011.03.31
#define	SURCHARGEOWNER_DEFAULT		L"OWNER"

#define	RECEIPT_HEADER1_DEFAULT		L""
#define	RECEIPT_HEADER2_DEFAULT		L""
#define	RECEIPT_HEADER3_DEFAULT		L""

#define ADV_GUIDEDISP_TIME_DEFAULT	5
#define	REVERSAL_RETRY_DEFAULT		3 //[#542] AU_C SOOK 2009.07.01 CUSTOMERS Default 값 적용	[#2038] AU KSK 2011.03.31

// ----------------------------------------------------------------------------
// EMV and other options
// ----------------------------------------------------------------------------
#define	EMV_LEVEL2					1	// EMV Level2 지원
#define	EMV_TEST_MODE					0			// [#2115] NH KSK 2011.10.24 EMV 거래 시 최종 1거래를 FILE에 저장하도록 기능 추가
#define SUPPORT_CHANGE_PARAMETER_JNL	1			//[#610] SOOK 2009.12.21 Configuration Change Journal 저장 

// ----------------------------------------------------------------------------
// OP TIMEOUT시 전환 화면 DEFINE,  2분 Timeout 발생시 OP 비밀번호 대기 화면. 비밀 번호 대기 화면에서 1분 Timeout 발생시 거래 대기 화면
// ----------------------------------------------------------------------------
#define OP_TIMEOUT_SCREEN_NUMBER		899

// ----------------------------------------------------------------------------
// OP TIMEOUT시 전환 화면 DEFINE					// [#540] HWANG 2008.07.01
// ----------------------------------------------------------------------------
#define DIAG_TIMEOUT_SCREEN_NUMBER		829			

#define RESEVED_AREA_VERSION	(0x78010000)        // [#546] SOOK 2009.08.06 Reserved Area 버전 추가 

#endif // _BASE_DEF_H_
