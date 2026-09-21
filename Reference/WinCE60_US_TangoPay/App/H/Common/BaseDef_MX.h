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
#ifndef APP_DEF
#  define APP_CUSTOM_CIBC_MX				1	// B06.XX.XX"		// CIBC Mexico TDL version
#endif
#define PIN_CHANGE						1	//					PIN Change Transaction

// ----------------------------------------------------------------------------
///////////////////////// Disabled OR Unavailable Options /////////////////////
//////                           DO NOT ENABLE !!!!!                    ///////
// ----------------------------------------------------------------------------
#define MONIMOBILE_QRCODE			0	// MoniMobile Option
#define	APP_STD1_ENHANCED_V2		1	// [#2177] Standard 1 Enhanced Version 2
#define APP_CUSTOM_PAI				0	// [#2182] US PAI ONLY (Dual Host DCC)
#define	APP_EVENT_NOTICE			0	// [#2313] US Event Notice
	#define EVENT_NOTICE_TRANONLY	0	// [#2313] US Event Notice SUB MENU
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
//////////////////////////// Versions /////////////////////////////////////////
// ----------------------------------------------------------------------------
// RWC6  "V" instead "B"   12212023
#if(APP_CUSTOM_CIBC_MX)
#  define AP_TYPE		_T("B")
#  define APCODE		"B\0"
#  define APP_TDL_OPTION		1
#else
#  define AP_TYPE		_T("V")
#  define APCODE		"V\0"
#  define APP_TDL_OPTION        1 
#endif

#define TERRCODE "MX\0"

#define NO_SECURE_TLS_MODE
// ----------------------------------------------------------------------------
// MEMORY CHECK TEST - DEBUG
// ----------------------------------------------------------------------------
//#define MEM_LEAK_CHECK			// Memory leak check
//#define NEXTWARE_BSTRACE_ON	1	// BSTrace On

// ----------------------------------------------------------------------------
// SCREEN VIEW - DEBUG
// ----------------------------------------------------------------------------
//#define AE_SCREEN_DEGISN

// ----------------------------------------------------------------------------
// COUNTRY DEFINE (하나만 1로 설정) 2008.03.03 JSW
// ----------------------------------------------------------------------------
#define	US_VERSION		0	// 미국
#define AU_VERSION		0	// 호주
//	#define AU_A_VERSION	0	// 호주 - ATM Plus
//	#define AU_C_VERSION	0	// 호주 - Customers
#define MX_VERSION		1	// 멕시코
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
// [#285] [MX] KSK 2008.6.12
// MEXICO 권종을 지원하기 위해 TCHAR Type을 String Type으로 전환
//#define CURRENCY_SYMBOL	(TCHAR)0x24			// 국가별 화폐단위 표시(미국 달러): L'$'
#define CURRENCY_SYMBOL			_T("MXN")		// 국가별 화폐단위 표시(MEXICO 달러)
#define MULTI_CURRENCY_SYMBOL	_T("USD")		// 국가별 화폐단위 표시(US 달러)	// [#514] [MX] KSK 2009.3.4
#define SCREEN_CURRENCY			_T("$")			// KSK 2009.06.18 인증 불구합 대응
//#define	MEXICO_PESOS	_T("pesos")

// ----------------------------------------------------------------------------
// [#240] NH 2008.05.28 국가별 Checking Account 표현
// ----------------------------------------------------------------------------
//#define	S_ACCOUNT_CHECKING _T("CHEQUE")		// 영국
//#define	S_ACCOUNT_CHECKING _T("CHEQUING")	// 캐나다
#define	S_ACCOUNT_CHECKING _T("CHECKING")		// 미국
// end of [#240]

// ----------------------------------------------------------------------------
// [#50] NH PSC 2008.03.26 국가별 화폐단위 for MWI
// ----------------------------------------------------------------------------
//#define CURRENCY_TYPE	_T("USD")			// 국가별 화폐단위 (미국 달러):		'$'
//#define CURRENCY_TYPE	_T("GBP")			// 국가별 화폐단위 (영국 파운드):	'￡'
//#define CURRENCY_TYPE	_T("CAD")			// 국가별 화폐단위 (캐나다 달러):	'$'
//#define CURRENCY_TYPE	_T("NZD")			// 국가별 화폐단위 (뉴질랜드 달러):	'$'
//#define CURRENCY_TYPE	_T("AUD")			// 국가별 화폐단위 (호주 달러):		'$'
#define CURRENCY_TYPE		_T("MXN")		// 국가별 화폐단위 (멕시코 ):	'MX$'
#define MULTI_CURRENCY_TYPE	_T("USD")		// 국가별 화페단위 (미국달러):	'US$'	// [#514] [MX] KSK 2009.3.4

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

#define WINCE_RMS_5000_MACHINECODE				0x3D	// 5000 CE - MX
#define WINCE_RMS_5300_MACHINECODE				0x18	// 5300 CE - MX	기기 나간적 없음 (RMS와 Code와 미리 Define하였음)
#define WINCE_RMS_1800_MACHINECODE				0x0A	// 1800 CE - MX 기기 나간적 없음 (RMS와 Code와 미리 Define하였음)

#define WINCE_RMS_2700_MACHINECODE_FRONT		0x48	// NH2700CE FRONT //[#2130] MX PCS 2012.04.02 "RMS 2700기종 추가."
#define WINCE_RMS_1800SE_MACHINECODE_FRONT		0x59	// NH1800SE FRONT	// [#11] NH KSK 2010.09.27 REAR는 의미가 없으므로 1800SE로 수정
#define WINCE_RMS_MX4000W_MACHINECODE			0x5E	// [#2097] US PCS 2011.11.15 "WallMount"
#define WINCE_RMS_MX5000SE_MACHINECODE			0x61	// [#2097] US PCS 2011.11.15 "MX5000SE" - 추가 정의 필요
#define WINCE_RMS_HALO2600_MACHINECODE			0x00	// [#2222] US Justin 2013.10.03 HALO is not available at Mexico
#define WINCE_RMS_NH1500SE_MACHINECODE			0x00	// [#2267] US KSK 2014.05.12
#define WINCE_RMS_MX5200SE_MACHINECODE_FRONT	0x68	// [#2303] US KSK 2014.11.07		// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
#define WINCE_RMS_HALO2_MACHINECODE				0x6E	// [#2332] NH Justin 2015.02.10 Add Halo2 Machine code (110)
#define WINCE_RMS_MX5300SE_MACHINECODE			0x72	// [#2383] NH Justin 2015.12.18 Add 5300 SE Machine Code (114)
#define WINCE_RMS_MX2800SE_MACHINECODE			0x77	// [#2529] NH Justin 2018.01.23 Add 2800 SE Machine Code (119)
#define WINCE_RMS_MX2800T_MACHINECODE			0x7B	// [#GLDV-2505] US Kook 2019.05.22  Add 2800 T Machine Code (123 for MX)

#define WINCE7_RMS_COMMON_MACHINECODE			0x7F	// All WinCE 7.0 machines

// ----------------------------------------------------------------------------
// VERSION INFORMATION
// ----------------------------------------------------------------------------
#if (_WIN32_WCE < 0x600)

	/*------------------------------------------------
					WINDOWS CE 50
	------------------------------------------------*/
	


#else // (_WIN32_WCE < 0x600)

	/*------------------------------------------------
					WINDOWS CE 60
	------------------------------------------------*/

	//#define AP_VERSION		_T("V06.00.00")		// 초기 WinCE 60 버전 (WINCE5.0 V01.04.07 기준으로 작업함)
	//#define AP_VERSION		_T("V06.00.01")		// [#2130] MX PCS 2012.04.02 "CardTronics 2차 개발 추가."
	//#define AP_VERSION		_T("V06.00.02")		// [#2137] MX KMK 2012.06.26 "Prosa향 USD 방출 모드 지원"
	//#define AP_VERSION		_T("V06.00.03")		// [#2149] MX Justin 2012.09.21 Customer request, Exchange Fee => display as an Amount
	//#define AP_VERSION		_T("V06.00.04")		// USD Mode International EMV Transaction Bug Fix
	//#define AP_VERSION		_T("V06.00.06")		// [#2183] NH Justin 2013.04.05 Receipt Option
	//#define AP_VERSION		_T("V06.00.07")		// [#2266] NH Justin PCI V3.0
	//#define AP_VERSION		_T("V06.00.08")		// [#2279] NH Justin Upload EMV data to Moniview
	//#define AP_VERSION		_T("V06.00.09")		// Enhancements
	//#define AP_VERSION	AP_TYPE _T("06.00.10")	// Card Reader 31002 Error Fix, Not sending F0001 on Add Cash HeartBeat after Cassette Total
	//#define AP_VERSION	AP_TYPE _T("06.00.11")	// SPR EP Downloading, Printer Column Set
	//#define AP_VERSION	AP_TYPE _T("06.00.12")	// SSL/TLS Version : ABOVE => UPTO, Modify Screen Text(Adding USD) for USD Dispensing ATMs
	//#define AP_VERSION	AP_TYPE _T("06.00.13")	// Remote AID Selection
	//#define AP_VERSION	AP_TYPE _T("06.00.14")	// TCPIP Connect Bug fix, Enhance SSL/TLS Certificat 
	//#define AP_VERSION	AP_TYPE _T("06.00.15")	// TC for NON EMV DATA Case
	//#define AP_VERSION	AP_TYPE	_T("06.00.16")	// CDU Encrpytion
	//#define AP_VERSION	AP_TYPE	_T("06.00.17")	// Support Remote Multiple EMV En/Disable
	//#define AP_VERSION	AP_TYPE	_T("06.00.18")	// Supporting MX5300SE, EMV Kernel V6.0
	//#define AP_VERSION	AP_TYPE	_T("06.00.19")	// New EMV Card Reading Animation
	//#define AP_VERSION	AP_TYPE	_T("06.00.20")	// [#2518] US Kook 2017.12.21 Support MX-2800SE
	//#define AP_VERSION	AP_TYPE	_T("06.00.21")	// [#GLDV-2529] Addressing WINCE vulnerability. (RMS buffer overflow, XFS DLL External Socket Listening)
													// [#RWC6-73] Remove US NFC
													// [#RWC6-107,106, 136, 109, 104, 96, 1] Performance Improvements
													// [#RWC6-94] Master PW reset on NVRAM Broken screen
													// [#RWC6-51] Remove VATs
													// [#RWC6-56] Hashed Passwords
													// [#RWC6-121] Remote Updates
													// [#RWC6-108] Memory Management fixes (Resource Management)
													// [#RWC6-120] MoniMobile PCI 5.0 Codes
													// [#RWC6-133] Add DHCP options to INI file
													// [#RWC6-53] Remove MoniAir
													// [#RWC6-81] Multiple Bills per Test dispense in Diagnostics
													// [#RWC6-119] JustCash MoniView registration JOB
													// [#RWC6-140] B4U MoniView edit 
													// [#RWC6-125] change Date/time by processor STD1 't' and 'ua'
													// [#RWC6-143] PAI Dual-Balance screens are wrong for some screen sizes 
/*
Task,RWC6-218,44153,Update OpenSSL to 1.1.1g,William.Ward,Critical,2020-09-01 10:35 AM,2020-11-06 3:27 PM,Matt.Langdon,Resolved
Bug,RWC6-227,45121,Replace new (6.0.0.03) BSPin30 with old version (6.0.0.02) in MasterFiles,Matt.Langdon,Critical,2020-10-14 9:30 AM,2020-11-10 7:03 AM,Matt.Langdon,Closed
Improvement,RWC6-216,43943,Make LibertyX have separate sequence numbers,William.Ward,Major,2020-08-21 9:12 AM,2020-11-05 8:28 AM,Matt.Langdon,Closed
Bug,RWC6-219,44337,Quick Configuration not changing master password,Matt.Langdon,Major,2020-09-10 11:36 AM,2020-11-05 8:28 AM,Matt.Langdon,Closed
New Feature,RWC6-224,45080,LibertyX Cash Out Transaction,William.Ward,Major,2020-10-12 11:59 AM,2020-11-09 7:38 AM,Matt.Langdon,Closed
Improvement,RWC6-226,45105,Add EMV kernel v7 to Canada AP,Matt.Langdon,Major,2020-10-13 12:01 PM,2020-11-10 8:59 AM,Matt.Langdon,Closed
Task,RWC6-229,45260,Support for bcrypt,William.Ward,Major,2020-10-20 9:52 AM,2020-11-10 9:18 AM,Matt.Langdon,Closed
Improvement,RWC6-213,43726,Add dispense style to INI,William.Ward,Minor,2020-08-11 3:57 PM,2020-11-10 9:49 AM,Matt.Langdon,Closed
Improvement,RWC6-214,43728,Add dispense style to MoniView setup data,William.Ward,Minor,2020-08-11 3:57 PM,2020-09-01 1:26 PM,Matt.Langdon,Resolved
New Feature,RWC6-217,44081,Add Asian Language Pack to Mexico Release,Matt.Langdon,Minor,2020-08-27 2:33 PM,2020-11-05 8:36 AM,Matt.Langdon,Resolved
Improvement,RWC6-223,44681,Add account selection to DF inquiry,William.Ward,Minor,2020-09-23 8:43 AM,2020-11-06 8:05 AM,Matt.Langdon,Closed
Bug,RWC6-228,45126,General DCC offering screen accept-decline buttons issue,Matt.Langdon,Minor,2020-10-14 2:19 PM,2020-11-06 12:09 PM,Matt.Langdon,Closed
Bug,RWC6-233,45774,B4U Registration Fail Causing WinATM,Matt.Langdon,Minor,2020-11-06 3:41 PM,2020-11-09 10:59 AM,William.Ward,Submitted
*/
	//#define	AP_VERSION	AP_TYPE	_T("06.02.04")
	//#define AP_RC_VERSION "06.02.04\0"


/*
Bug,RWC6-221,44482,2800T specific zip 1024*600 OP screen asset missing,Matt.Langdon,Major,2020-09-16 1:06 PM,2020-12-11 3:49 PM,Matt.Langdon,In QA Review
Bug,RWC6-222,44680,5 second delay to ReadData on EPP with 8.0.0 EP,William.Ward,Major,2020-09-23 8:42 AM,2020-12-11 3:23 PM,Matt.Langdon,Resolved
Task,RWC6-238,46698,Add ATM TID to LibertyX messages,William.Ward,Major,2020-12-09 2:22 PM,2020-12-10 9:02 AM,Matt.Langdon,Resolved
Bug,RWC6-234,46283,Cardtronics Canada MC DCC Offering Screen Button Issue,Matt.Langdon,Minor,2020-11-24 9:10 AM,2020-12-10 7:21 AM,Matt.Langdon,Resolved
Bug,RWC6-237,46692,Incorrect SEQ# is printed on LTX receipt,William.Ward,Minor,2020-12-09 11:59 AM,2020-12-11 3:23 PM,Matt.Langdon,Resolved
Bug,RWC6-241,46768,Read thread crashes on Visa Framed when server doesn't disconnect after txn,William.Ward,Critical,2020-12-11 2:55 PM,2020-12-11 2:56 PM,Matt.Langdon,Resolved
Improvement,RWC6-232,45767,Remove GivePay,William.Ward,Minor,2020-11-06 10:54 AM,2020-12-08 3:15 PM,Matt.Langdon,Resolved
*/
	//#define	AP_VERSION	AP_TYPE	_T("06.02.06")

													//Mexico TDL - Prosa	RWC6-253
													//Bansi DCC Customizations	RWC6-245
													//LibertyX last4 does not include the actual last 4 of the card	RWC6-256
													//Add LibertyX Username to RMS setup information	RWC6-257
													//LTX Dispense Journal data is mis-aligned in MoniView	RWC6-258
													//LibertyX performs a health check when terminal is booting	RWC6-261
													//Add Dual Balance options to the settings INI	RWC6-263
													//MCCP decryption of symmetric key fails	RWC6-271
													//Missing TCP Connection Read Timeouts	RWC6-272
													//Reboot before remote update	RWC6-274
													//Updates to PlanetPayment DCC Screen/Receipt	RWC6-276

	//#define	AP_VERSION	AP_TYPE	_T("06.02.07")

													// Support dual-language receipts	RWC6-290
													// Display "no withdrawal" warning instead of going out of service when ATM is out of cash	RWC6-259
													// Fix Cash Totals MoniMobile QR code buffer issues	RWC6-298
													// Update.exe needs power failure on slow network connections	RWC6-303
	//#define	AP_VERSION	AP_TYPE	_T("06.02.08")

													// PIN Change Transaction!	RWC6-300
													// Update to OpenSSL 1.1.1l	RWC6-311
													// Correct LibertyX Journal display on error transaction	RWC6-313
													// [MPCE2] Asian Fonts not showing on WEC7	RWC6-266
													// [MPCE2] Rearrange folder structure for WEC7 patches	RWC6-267
													// [MPCE2] Random halts when CAdaCtrl() is being created after LVMOD is activated on WEC7	RWC6-268
													// Alpha-numeric entry screen has a very slow response rate (WinCE 7)	RWC6-269
													// Add MoniView WinCE7 MoniView machine codes	RWC6-270
													// Integrate remote software download into OS version of update.exe	RWC6-189
													// [MPCE2] New board development with WEC7	RWC6-225
													// USB SPR driver crashes	RWC6-264
													// Add ability to use multiple certificates in Update Repository	RWC6-248
													// Add LibertyX/B4U Journals to EJUP Spec and Job	RWC6-297
													// NVRAM backup and Journal Backup do not work	RWC6-320
													// [ADA] Press the 8 key for Cardless Transactions not voicing	RWC6-321
													// [MPCE2/AU] ADACtrl plays WAV file improperly on WEC7	RWC6-273
													// ATM freezes when exiting Admin functions.	RWC6-332
													// Receipt shows "Error Code" header for Savings Account balance inquiry	RWC6-328
													// [DEBUG] Serial output constantly printing "ErrCode_PIN(9792829)"	RWC6-322
													// Allow taking screenshots	RWC6-335

	//#define	AP_VERSION	AP_TYPE	_T("06.02.09")
													// CIBC Screen Updates RWC6-331
	//#define	AP_VERSION	AP_TYPE	_T("06.02.09")
														// Credit Card Segmentation	RWC6-341
														// Enable MoniView control of Fast Cash Options	RWC6-344
														// Reduce memory limit to 90% for screen issues	RWC6-345
														// Update EMV Kernel to 7.x.2.6	RWC6-354
														// Modify factory defaults	RWC6-355
														// Consider MWI third decimal component when calculating automatic remote update decisioning	RWC6-353
														// CDU Initialization - Jam Clear	RWC6-82
														// Update MCU EPs	RWC6-357
														// Update CDU EP to V0x8057	RWC6-358
														// Add support for blank DF68 TDL tags for Bansi	RWC6-295
														// Add support for variable length tag on STD3 TID - Bansi MX	RWC6-301
														// Respect IVA Taxation tags in TDL messages	RWC6-306
														// CIBC screen updates	RWC6-331
														// Test: Prosa needs reduced-strength ciphers	RWC6-346
														// Move DCC screen/receipt customizations to DAT files	RWC6-360
														// CST counts are cut off in OP main screen	RWC6-307
														// Quick Configuration when not TID Input	RWC6-359
														// Support USB camera on 5300SE	RWC6-371
														// CIBC Changes to MX DCC Screens	RWC6-372
	//#define	AP_VERSION	AP_TYPE	_T("06.02.10")

														// Add CDU Binding status and enablement to MoniView Setup Data	RWC6-388
														// Add Model Name to MoniView Setup Date	RWC6-382
														// Add Local Surcharge DCC option to MoniView Setup Data	RWC6-384
//	#define	AP_VERSION	AP_TYPE	_T("06.03.02.01")
//	#define AP_RC_VERSION "06.03.02.01\0"

// #define	AP_VERSION	AP_TYPE	_T("06.03.02.02")
// #define AP_RC_VERSION "06.03.02.02\0" 

// #define	AP_VERSION	AP_TYPE	_T("06.03.02.03")
// #define AP_RC_VERSION "06.03.02.03\0" 

// #define	AP_VERSION	AP_TYPE	_T("06.03.02.04")
//  #define AP_RC_VERSION "06.03.02.04\0"
														// Cash Depot - Requesting that the card reader guide light to be on when card is returned. RWC6-650
														// Cash Depot - Requesting that the an audio beep is played when card is returned. RWC6-651
														// Cash Depot - The maximum dispense limit is being set to a default value instead of host configured value.RWC6-652
														// Cash Depot - DCC is being denied by the host when the admin fee is enabled. RWC6-659

 // #define AP_VERSION	AP_TYPE	_T("06.03.02.05")
 // #define AP_RC_VERSION "06.03.02.05\0"

 #define AP_VERSION	AP_TYPE	_T("06.03.03")
 #define AP_RC_VERSION "06.03.03\0"

	//#define AP_DETAIL_VERSION	6,0,0,0				// 초기 WinCE 60 버전
	//#define AP_DETAIL_VERSION	6,0,1,0				// 1. RMS MS NH2700기종 추가.
													// 2. Domestic/International BIN 추가.
													// 3. Bansi/Banco 이미지 추가.
	//#define AP_DETAIL_VERSION	6,0,2,0				// 1.멕시코 USD 방출 모드 지원 및 거래 플로우 변경
	//#define AP_DETAIL_VERSION	6,0,2,1				// [#2137] MX PCS 2012.08.02 "132번 화면 문구 수정."
	//#define AP_DETAIL_VERSION	6,0,2,1				// [#2139] MX KMK 2012.08.07 USD방출모드 조회 거래 / International Card인 경우 저널에 qbE,qbF가 표시되는 Bug Fix
	//#define AP_DETAIL_VERSION	6,0,2,1				// [#2140] MX KMK 2012.08.07 Peso 방출 모드에서 출금 거래시에 명세표에 Total Amount값 인자
	//#define AP_DETAIL_VERSION	6,0,2,1				// [#2141] International Card로 거래시 명세표 및 저널에 qbA~D 인자되는 버그 수정
	//#define AP_DETAIL_VERSION	6,0,2,1				// [#2142] Prosa 사양 변경으로 인한 수정 (필드값, 명세표 등)
	//#define AP_DETAIL_VERSION	6,0,2,2				// WINCE5.0 사양 변경한 내용 적용
	//#define AP_DETAIL_VERSION	6,0,2,3				// 최종 소스 머지 적용 (공통 장애대책 내용 적용 포함 PIN ERROR)
	//#define AP_DETAIL_VERSION	6,0,3,0				// [#2149] MX Justin Display Exchange fee as an amount, User Final confirmation.
	//#define AP_DETAIL_VERSION	6,0,3,1				// [#2152] MX Justin Saving Journal Bug Fix (Remove extra Seperator(^)) 
	//#define AP_DETAIL_VERSION	6,0,3,2				// [#2153] NH KSK CDU Missfeed Registry값 변경
	//#define AP_DETAIL_VERSION	6,0,3,3				// [#2159] MX Justin Mexico Sales requests 1.Rejecting NON IC Card transaction (Service code = 2 or 6) 2.Showing surcharge screen even received surcharge = 0  (Regulation)
	//#define AP_DETAIL_VERSION	6,0,3,4				// [#2159] MX Justin Displaying Damaged IC card cancel message
	//#define AP_DETAIL_VERSION	6,0,3,5				// [#2164] MX Justin Change screen texts.
	//#define AP_DETAIL_VERSION	6,0,3,6				// [#2167] NH Justin Anti Skimming option Add....
	//#define AP_DETAIL_VERSION	6,0,3,7				// [#2156] NH KSK Kill DHCP POP UP, DHCP retrial....
	//#define AP_DETAIL_VERSION	6,0,3,8				// [#2168] MX Justin Remove "EMV restriction for International Card"
	//#define AP_DETAIL_VERSION	6,0,3,9				// [#2170] MX Justin Change EPP Layout Same as Canada
	//#define AP_DETAIL_VERSION	6,0,4,0				// [#2180] NH Justin Mexico International EMV Bug Fix, Store Currency ID (EMV), 
													// [#2180] Add 5000SE
	//#define AP_DETAIL_VERSION	6,0,4,1				// Release version.
	//#define AP_DETAIL_VERSION	6,0,5,0				// [#2183] NH Justin Receipt Option (No Transaction result on Screen)
													// [#2198] US Justin Remove Sankyo => NH EMV
													// [#2198] US Justin US EMV Response Code "85" fix 
													// [#2209] Receipt on Screen Option - "Print All setup"
													// [#2210] MX Justin 
													//		1. Print address on top of journal
													//		2. Add Exchange Fee Format (USD En/Disable)
	//#define AP_DETAIL_VERSION	6,0,5,1				// [#2213] US Justin Add Supervisor Language
													// [#2218] US Justin Change EMV Transaction Code (BI:30, TR:40)
	//#define AP_DETAIL_VERSION	6,0,5,2				// [#2219] Justin Merge Digital Receipt 
	//#define AP_DETAIL_VERSION	6,0,5,3				// [#2226] Justin EMV Kernel 5
	//#define AP_DETAIL_VERSION	6,0,6,0				// NH Justin Sankyo SP 06.00.08 / CDU SP 06,.06.11.01
													// [#2232] Justin Prebalance Bug Fix
	//#define AP_DETAIL_VERSION	6,0,7,0				// [#2265],[#2266] and PCI V3.0 NH Justin 
	//#define AP_DETAIL_VERSION	6,0,8,0				// [#2279] Justin Uploading EMV Journal data to Moniview, Add PESO (USD Dispensing, Domestic Card)
	//#define AP_DETAIL_VERSION	6,0,9,0				// [#2282] Justin Add Missed "Remove Card Notice"
													// [#2285] 1. Clear MasterKey when Denomination is changed.
													//	       2. Remove "Mexico Damaged Card Restriction" - [#2159]
	//#define AP_DETAIL_VERSION	6,0,10,0			// [#2287] NH Justin Enable CDUM Log Backup
													// [#2290] Ignoring "31002" Error on Unlatching command  => NO CODE Change (SP Changed)
													// [#2290] Not Sending F0001 (STD3:384) error code on Add Cash Heartbeat after Cassette Total.
													// [#2291] Disable Balance Inquiry if TDL is not enabled.
													// [#2291] Handling "085" BI response for STD3 if TDL is enabled.
	//#define AP_DETAIL_VERSION	6,0,11,0			// [#2295] SPR EP Download [#2298] Print Column Set, Support early versions of printer
													// [#2302] US Justin Move IC Processing after "Fallback" check
													// [#2304] US Justin Clearing Master Password only when NVRAM is Broken
													// [#2304] Display EMV AID Name on PIN Entry Screen
													// [#2306] KSK Add MWI Log
													// [#2307] KSK Remove time delay after Image Priting
													// FIX Card reader EMV 99999 ERROR => SP Change
													// [#2312] Support MX5200, Support TLS V1.2
	//#define AP_DETAIL_VERSION	6,0,12,0			// [#2320] WorldPay SSLV3.0 Issue : SSL/TLS Option : "ABOVE" => "UP TO"
													// [#2332] Modify Screen Text (Adding USD) for USD Dispensing ATMs
													// [#2336] Modify 9F1A CountryCode
	//#define AP_DETAIL_VERSION	6,0,13,0			// [#2342] AID Selection / Remote AID Selection 
													// [#2346] NH Justin Digital Receipt - Adding Address and Country Code in Digital Receipt.
	//#define AP_DETAIL_VERSION	6,0,14,0			// [#2356] NH KMK TCPIP QUEUE Bug Fix 
													// [#2363] NH KMK SSL/TLS Enahnce Certificate Check Logic 
													// [#2369] US Justin Country and Currency = MXN (484) for both USD and PESO mode
	//#define AP_DETAIL_VERSION	6,0,15,0			// [#2372] NH Justin TC for NON EMV Data Case
	//#define AP_DETAIL_VERSION	6,0,16,0			// [#2383] US Justin Supporting MX5300SE
													// [#2384] US Justin Supporting CDU Binding
													// [#2386] KSK Selective CDU Binding Option => Binding Icon
													// [#2392] US Justin Adding CDU Binding Option (Default : Disable)
													// [#2394] NH KSK 2016.02.05 OS Background Image 변경시 WinAtm Exception 발생하는 오류 수정
													// [#2397] KSK SPR 97922 Error => Reboot
													// [#2399] MX Justin Prosa's Request, USD Machine => Currency Code :840
													//	       Supporting $0 Surcharge/Exchange Fee for USD Machine
	//#define AP_DETAIL_VERSION	6,0,17,0			// [#2411] US Justin 2016.04.01 Socket Error Timeout Bug Fix
													// [#2412] US Justin 2016.04.05 Support Additional Host Certification
													// [#2423] US Justin 2016.05.05 En/Disable EMV for multiple ATMs
	//#define AP_DETAIL_VERSION	6,0,18,0			// [#2431] US Justin 2016.06.13 Change "Thank you for using our services" => "Thank You"
													// [#2435] Supporting EMV Kernel V6.0
													// [#2438] Change Transaction Category for Kernel V6.0 and higher versions (9F53 :  'R' (52) => 'Z'(5A) )
													// [#2440] NH Justin 2016.08.10 Showing AID Selection even though one AID Left after removing Blocked AID 
	//#define AP_DETAIL_VERSION	6,0,19,0			// [#2451] US Justin 2016.11.04 New EMV Card reading Animation
													// [#2453] NH Justin NHS2.8 Status 
													// [#2454] NH Justin EJ UPLOAD BUG FIX (Transaction Audit/Network ID)
													// [#2456] NH Justin 2016.12.07 Change Enagled AID on PRINT ALL SETUP
													// [#2461] NH Justin 2016.12.15 Partial Update throught Moniview
													// [#2492] NH Justin 2017.06.30 Fixing "Out of Service without Error Code"
													// [#2504] NH Justin 2017.09.11 Add Test Dispense on ADD CASH SCREEN													
													// [#2508] NH Justin 2017.09.27 Upload Master Key checksums to Moniview
													// [#2507] NH Justin 2017.09.26 Add Test Button on RMS Send Screen
													// [#2508] NH Justin 2017.09.27 Upload Master Key checksums to Moniview
													// [#2519] NH Justin 2017.12.08 Leave Fallback Indicator in the journal
													// [#2521] NH Justin 2017.12.11 MoniMobile : Add value added function enable status
	//#define AP_DETAIL_VERSION	6,0,20,0			// [#2529] NH Justin 2018.01.23 Support 2800SE
													// [#2537] NH Justin 2018.03.06 Combine US,CA,MX AP data
													// [#2538] NH Justin 2018.03.09 Adding MoniMobile Cash Balance Option
													// [#2561] NH Justin 2018.07.11 Enable Camera Features on CA and MX Mode APs
													// [#2562] NH Justin 2018.07.12 Reduce PAUSE INTERVAL Between PRINT and CUT
	//#define AP_DETAIL_VERSION	6,0,21,0			// [#GLDV-2529] Addressing WINCE vulnerability. (RMS buffer overflow, XFS DLL External Socket Listening)
													// [#RWC6-4] US William 2019.09.24 D1706
													// [#RWC6-23] US Wiliam 2019.09.24 GivePay Ads 7+8
													// [#RWC6-26] US William 2019.09.24 CA Certs
													// [#RWC6-44] US Brandon 2019.09.24 GivePay QR
													// [#RWC6-50] US William 2019.09.24 EMV Kernel V7
													// [#RWC6-3] US Brandon 2019.09.24 Just.Cash Copy Edits
													// [#RWC6-7] US William 2019.09.24 PAI Vault Door Journal
													// [#RWC6-24] US Brandon 2019.09.24 CCA feature in STD1 Only
	//#define AP_DETAIL_VERSION	6,0,22,0			// [#] US William 2019.09.24 MCCP Changes for PAI (STD1/3 msg flags + download on boot)
	//#define	AP_DETAIL_VERSION	6,2,2,0				// [#RWC6-73] Remove US NFC
													// [#RWC6-107,106, 136, 109, 104, 96, 1] Performance Improvements
													// [#RWC6-94] Master PW reset on NVRAM Broken screen
													// [#RWC6-51] Remove VATs
													// [#RWC6-56] Hashed Passwords
													// [#RWC6-121] Remote Updates
													// [#RWC6-108] Memory Management fixes (Resource Management)
													// [#RWC6-120] MoniMobile PCI 5.0 Codes
													// [#RWC6-133] Add DHCP options to INI file
													// [#RWC6-53] Remove MoniAir
													// [#RWC6-81] Multiple Bills per Test dispense in Diagnostics
													// [#RWC6-119] JustCash MoniView registration JOB
													// [#RWC6-140] B4U MoniView edit 
													// [#RWC6-125] change Date/time by processor STD1 't' and 'ua'
													// [#RWC6-143] PAI Dual-Balance screens are wrong for some screen sizes 

/*
Task,RWC6-218,44153,Update OpenSSL to 1.1.1g,William.Ward,Critical,2020-09-01 10:35 AM,2020-11-06 3:27 PM,Matt.Langdon,Resolved
Bug,RWC6-227,45121,Replace new (6.0.0.03) BSPin30 with old version (6.0.0.02) in MasterFiles,Matt.Langdon,Critical,2020-10-14 9:30 AM,2020-11-10 7:03 AM,Matt.Langdon,Closed
Improvement,RWC6-216,43943,Make LibertyX have separate sequence numbers,William.Ward,Major,2020-08-21 9:12 AM,2020-11-05 8:28 AM,Matt.Langdon,Closed
Bug,RWC6-219,44337,Quick Configuration not changing master password,Matt.Langdon,Major,2020-09-10 11:36 AM,2020-11-05 8:28 AM,Matt.Langdon,Closed
New Feature,RWC6-224,45080,LibertyX Cash Out Transaction,William.Ward,Major,2020-10-12 11:59 AM,2020-11-09 7:38 AM,Matt.Langdon,Closed
Improvement,RWC6-226,45105,Add EMV kernel v7 to Canada AP,Matt.Langdon,Major,2020-10-13 12:01 PM,2020-11-10 8:59 AM,Matt.Langdon,Closed
Task,RWC6-229,45260,Support for bcrypt,William.Ward,Major,2020-10-20 9:52 AM,2020-11-10 9:18 AM,Matt.Langdon,Closed
Improvement,RWC6-213,43726,Add dispense style to INI,William.Ward,Minor,2020-08-11 3:57 PM,2020-11-10 9:49 AM,Matt.Langdon,Closed
Improvement,RWC6-214,43728,Add dispense style to MoniView setup data,William.Ward,Minor,2020-08-11 3:57 PM,2020-09-01 1:26 PM,Matt.Langdon,Resolved
New Feature,RWC6-217,44081,Add Asian Language Pack to Mexico Release,Matt.Langdon,Minor,2020-08-27 2:33 PM,2020-11-05 8:36 AM,Matt.Langdon,Resolved
Improvement,RWC6-223,44681,Add account selection to DF inquiry,William.Ward,Minor,2020-09-23 8:43 AM,2020-11-06 8:05 AM,Matt.Langdon,Closed
Bug,RWC6-228,45126,General DCC offering screen accept-decline buttons issue,Matt.Langdon,Minor,2020-10-14 2:19 PM,2020-11-06 12:09 PM,Matt.Langdon,Closed
Bug,RWC6-233,45774,B4U Registration Fail Causing WinATM,Matt.Langdon,Minor,2020-11-06 3:41 PM,2020-11-09 10:59 AM,William.Ward,Submitted
*/
	//#define	AP_DETAIL_VERSION	6,2,4,0

/*
Bug,RWC6-221,44482,2800T specific zip 1024*600 OP screen asset missing,Matt.Langdon,Major,2020-09-16 1:06 PM,2020-12-11 3:49 PM,Matt.Langdon,In QA Review
Bug,RWC6-222,44680,5 second delay to ReadData on EPP with 8.0.0 EP,William.Ward,Major,2020-09-23 8:42 AM,2020-12-11 3:23 PM,Matt.Langdon,Resolved
Task,RWC6-238,46698,Add ATM TID to LibertyX messages,William.Ward,Major,2020-12-09 2:22 PM,2020-12-10 9:02 AM,Matt.Langdon,Resolved
Bug,RWC6-234,46283,Cardtronics Canada MC DCC Offering Screen Button Issue,Matt.Langdon,Minor,2020-11-24 9:10 AM,2020-12-10 7:21 AM,Matt.Langdon,Resolved
Bug,RWC6-237,46692,Incorrect SEQ# is printed on LTX receipt,William.Ward,Minor,2020-12-09 11:59 AM,2020-12-11 3:23 PM,Matt.Langdon,Resolved
Bug,RWC6-241,46768,Read thread crashes on Visa Framed when server doesn't disconnect after txn,William.Ward,Critical,2020-12-11 2:55 PM,2020-12-11 2:56 PM,Matt.Langdon,Resolved
Improvement,RWC6-232,45767,Remove GivePay,William.Ward,Minor,2020-11-06 10:54 AM,2020-12-08 3:15 PM,Matt.Langdon,Resolved
*/
	//#define	AP_DETAIL_VERSION	6,2,5,2


													// [RWC6-235] - CATM VISA DCC receipt verbiage
													// [RWC6-247] - EJUpload 'J' FID not initiating EJUP transaction
													// [RWC6-245] - Bansi DCC Customizations
													// [RWC6-242] - Add DF to Mexico Version
													// [RWC6-243] - LibertyX Voice Guidance
													// [RWC6-246] - Downgrade TLS ciphers for MoniView
													// [RWC6-236] - Add LibertyX KCV to PrintAllSetup
													// [RWC6-239] - CATM - French VISA DCC Receipt Message
													// [RWC6-240] - CATM - MasterCard DCC Partial Reversal Missing Fields
													// [RWC6-244] - Add Btye2.b1 for DCC DF enablement



	//#define		AP_DETAIL_VERSION	6,2,6,0	
													// [#RWC6-282] US William 2021.05.03 Update to EPP EP v15.0.3 and SP 6.4.15
	// #define	AP_DETAIL_VERSION	6,2,6,5
													// [@RWC6-289] US William 2021.06.02 EPP "ding" 10 minute fix
	//#define	AP_DETAIL_VERSION	6,2,6,6 
													//Mexico TDL - Prosa	RWC6-253
													//Bansi DCC Customizations	RWC6-245
													//LibertyX last4 does not include the actual last 4 of the card	RWC6-256
													//Add LibertyX Username to RMS setup information	RWC6-257
													//LTX Dispense Journal data is mis-aligned in MoniView	RWC6-258
													//LibertyX performs a health check when terminal is booting	RWC6-261
													//Add Dual Balance options to the settings INI	RWC6-263
													//MCCP decryption of symmetric key fails	RWC6-271
													//Missing TCP Connection Read Timeouts	RWC6-272
													//Reboot before remote update	RWC6-274
													//Updates to PlanetPayment DCC Screen/Receipt	RWC6-276
	//#define	AP_DETAIL_VERSION 6,2,7,0
													//EPP EP v15.00.03/SP v6.4.15 - RWC6-281
	//#define	AP_DETAIL_VERSION 6,2,7,1

													// [#RWC6-284] US William 2021.05.04 LibertyX AuthCode Journal Length Exceeds MoniView journal field length
													// [#RWC6-286] HQ Kook 2021.05.13 Prevent Canada from crashing after NVRAM
	// #define	AP_DETAIL_VERSION 6,2,7,2

													// [@RWC6-288] US William 2021.06.02 EPP "ding" 10 minute fix
	//#define	AP_DETAIL_VERSION 6,2,7,3

													// Support dual-language receipts	RWC6-290
													// Display "no withdrawal" warning instead of going out of service when ATM is out of cash	RWC6-259
													// Fix Cash Totals MoniMobile QR code buffer issues	RWC6-298
													// Update.exe needs power failure on slow network connections	RWC6-303
	//#define	AP_DETAIL_VERSION 6,2,8,0
													// PIN Change Transaction!	RWC6-300
													// Update to OpenSSL 1.1.1l	RWC6-311
													// Correct LibertyX Journal display on error transaction	RWC6-313
													// [MPCE2] Asian Fonts not showing on WEC7	RWC6-266
													// [MPCE2] Rearrange folder structure for WEC7 patches	RWC6-267
													// [MPCE2] Random halts when CAdaCtrl() is being created after LVMOD is activated on WEC7	RWC6-268
													// Alpha-numeric entry screen has a very slow response rate (WinCE 7)	RWC6-269
													// Add MoniView WinCE7 MoniView machine codes	RWC6-270
													// Integrate remote software download into OS version of update.exe	RWC6-189
													// [MPCE2] New board development with WEC7	RWC6-225
													// USB SPR driver crashes	RWC6-264
													// Add ability to use multiple certificates in Update Repository	RWC6-248
													// Add LibertyX/B4U Journals to EJUP Spec and Job	RWC6-297
													// NVRAM backup and Journal Backup do not work	RWC6-320
													// [ADA] Press the 8 key for Cardless Transactions not voicing	RWC6-321
													// [MPCE2/AU] ADACtrl plays WAV file improperly on WEC7	RWC6-273
													// ATM freezes when exiting Admin functions.	RWC6-332
													// Receipt shows "Error Code" header for Savings Account balance inquiry	RWC6-328
													// [DEBUG] Serial output constantly printing "ErrCode_PIN(9792829)"	RWC6-322
													// Allow taking screenshots	RWC6-335
	//#define		AP_DETAIL_VERSION 6,2,9,0
													// CIBC Screen Updates RWC6-331
	//#define		AP_DETAIL_VERSION 6,2,9,1
													// Credit Card Segmentation	RWC6-341
													// Enable MoniView control of Fast Cash Options	RWC6-344
													// Reduce memory limit to 90% for screen issues	RWC6-345
													// Update EMV Kernel to 7.x.2.6	RWC6-354
													// Modify factory defaults	RWC6-355
													// Consider MWI third decimal component when calculating automatic remote update decisioning	RWC6-353
													// CDU Initialization - Jam Clear	RWC6-82
													// Update MCU EPs	RWC6-357
													// Update CDU EP to V0x8057	RWC6-358
													// Add support for blank DF68 TDL tags for Bansi	RWC6-295
													// Add support for variable length tag on STD3 TID - Bansi MX	RWC6-301
													// Respect IVA Taxation tags in TDL messages	RWC6-306
													// CIBC screen updates	RWC6-331
													// Test: Prosa needs reduced-strength ciphers	RWC6-346
													// Move DCC screen/receipt customizations to DAT files	RWC6-360
													// CST counts are cut off in OP main screen	RWC6-307
													// Quick Configuration when not TID Input	RWC6-359
	//#define		AP_DETAIL_VERSION 6,2,10,0
													// Change "Surcharge" to "Access Fee" for general DCC	RWC6-367
	// #define		AP_DETAIL_VERSION 6,2,10,1
													// Non-English VISA DCC Mark-Up value missing	RWC6-369
	//#define		AP_DETAIL_VERSION 6,2,10,2
													// Support USB camera on 5300SE	RWC6-371
													// CIBC Changes to MX DCC Screens	RWC6-372
	//#define		AP_DETAIL_VERSION 6,2,10,3
													// CIBC changes to MX DCC v4	RWC6-373
	//#define		AP_DETAIL_VERSION 6,2,10,4
													// Tax is not added to total amount in TDL Transaction	RWC6-376
	//#define		AP_DETAIL_VERSION 6,2,10,5
													// TDL Host declines are not written to journal and have the wrong error code on the receipt	RWC6-383
	//#define		AP_DETAIL_VERSION 6,2,10,7
													// Add CDU Binding status and enablement to MoniView Setup Data	RWC6-388
													// Add Model Name to MoniView Setup Date	RWC6-382
													// Add Local Surcharge DCC option to MoniView Setup Data	RWC6-384
	//#define		AP_DETAIL_VERSION 6,2,11,0
	//#define		AP_DETAIL_VERSION 6,3,0,0
													//RWC6-293 TR31 for Standard 3
													//RWC6-356 [PAI] Disable dual host DCC by default
													//RWC6-380 SPR Paper Status Switches Between NORMAL and LOW_END
													//RWC6-409 SPR Paper Receipt surcharge length increase
													//RWC6-504 [Cash Depot] Add admin fees from API call
													//RWC6-505 [Cash Depot] Implement Remaining Balance Inquiry (RBI)
													//RWC6-508 [CA] Canada cannot be built
													//RWC6-513 [Cash Depot] Add max withdrawal from API call
													//RWC6-514 [Cash Depot] Add surcharge from API call
													//RWC6-515 Supervisor hangs on exit
													//RWC6-516 [LibertyX] Disable LIbertyX by default
													//RWC6-519 [MX] Mexico cannot be built
													// Fixed CustomerNVRAMValue not being parsed correctly
//	#define			AP_DETAIL_VERSION 6,3,1,0
													//RWC6-152 Disable ability to disable TLS certificate validation (US,MX,CA only)
													//RWC6-377 MCCP Sym Key Reset
													//RWC6-379 Add EPP Version to TSM
													//RWC6-506 [CashDepot] Implement cryptocurrency flows
													//RWC6-509 [DigitalMint] Online status not reporting accurately
													//RWC6-510 [DigitalMint] Reset Settings should only reset active mode's settings
													//RWC6-512 [PAI] Hyosung Software Enhancement Proposal
													//RWC6-534 [PAI] RMS - Add side car management
													//RWC6-545 [CA] AP Version not updating
													//RWC6-545 [MX] AP Version not updating
													//RWC6-546 [PAI] Improvments from Enhancement Request testing
													//RWC6-547 [PAI] CDS does not load settings correctly
													//RWC6-548 [Cash Depot] Improvments from phase 1 testing
													//RWC6-549 DCC Changes needed for MC and Visa
													//RWC6-551 [PAI] Improvments from Enhancement Request Testing
													//RWC6-552 [Cash Depot] Support 5200 models
													//RWC6-553 [PAI] RMS - Add Surcharge Segmentation enablement and value setting
													//RWC6-554 [PAI] RMS - Add Single-Host DCC enablement and value setting
													//RWC6-555 DCC - Mexico Updates....
													//RWC6-561 [Cash Depot] Remove crypto button from home screen
													//RWC6-562 [Cash Depot] Default API endpoint
//	#define			AP_DETAIL_VERSION 6,3,2,0
													//RWC6-568 Revert TLS requirement. (RWC6-152)
													//RWC6-573 Revert accidental tmacing reversal
													//RCW6-582 ADA Font Size issue
													//RWC6-583 Extra character in message for Cardtronics.
													//RWC6-589 Unable to build Cardtronics for Canada

//	#define			AP_DETAIL_VERSION 6,3,3,0
													//RWC6-655 US-CA DCC built as ‘V’ instead ‘D’
	#define			AP_DETAIL_VERSION 6,3,4,0
													//RWC6-659 Cash Depot - DCC is being denied by the host when the admin fee is enabled.
													//RWC6-669 Cash Depot: Enable Update of Network Fields using Moniview

#endif //(_WIN32_WCE < 0x600)

//#define RMS_VERSION		0x010318				// 1. Mexico 초기 RMS 버전
//#define RMS_VERSION		0x010325				// [#2320] Justin NEW SSL/TLS VERSION MANAGEMENT (ABOVE => UP TO)
//#define RMS_VERSION		0x010326				// [#2342] NH Justin 2015.05.11 AID SELECTION
//#define RMS_VERSION		0x010328				// [#2423] US Justin 2016.05.05 Support En/Disable EMV through individual command
//#define RMS_VERSION		0x010329				// [#2435] NH Justin Add Kernel version info in Setup, MV will check this version for allowing Kernel version change
//#define RMS_VERSION		0x010330				// [#2442] Enable Digital Donation (US ONLY)
//#define RMS_VERSION		0x010331				// [#2461] Support Partial Update, [#2475] US Justin Add Host Certificate En/Disable
//#define RMS_VERSION		0x010332				// [#2508] NH Justin 2017.09.27 Upload Master Key checksums to Moniview
//#define RMS_VERSION		0x010335				// [#2538] NH Justin 2018.03.09 Adding MoniMobile Cash Balance Option, Camera Option
//#define RMS_VERSION		0x010336				// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Optioin
//#define RMS_VERSION		0x010337				// AU Release
//#define RMS_VERSION		0x010338				// [#2574] US Justin GivePay Options
//#define RMS_VERSION		0x010339				// 2nd DNS
//#define RMS_VERSION		0x010340				// [#RWC6-29] PAI Add ALLPOIN
//#define RMS_VERSION		0x010341				// [#RWC6-67] US William 2019.10.24 RMS TLS
//#define RMS_VERSION		0x010342				// [#GLDV-2681] AU Kook 2019.11.06 Print Total Amount
//#define RMS_VERSION		0x010343				// [#RWC6-119] US William 2020.01.16 Add JustCash RMS changes / [#RWC6-56] US William 2020.02.07 Hashed Passwords
//#define RMS_VERSION		0x010344				// [HQ] Add RMS timeout options
//#define RMS_VERSION		0x010345				// [#RWC6-169] US William 2020.04.28 Add PAI Credit Segmentation RMS
//#define RMS_VERSION		0x010346				// [#RWC6-178] US William 2020.05.22 Add std3 comm header enablement to RMS setup data / // [#RWC6-168] US William 2020.06.09 Anti-skimming 2B RMS setup field
//#define RMS_VERSION		0x010347				// [#RWC6-207] US William 2020.08.05 Add LibertyX enablement to setup data
//#define RMS_VERSION		0x010348				// [#RWC6-214] US William 2020.09.01 Add CDU Dispense Style				
//#define RMS_VERSION		0x010349				// [#RWC6-224] US William 2020.10.21 LibertyX Dispense			
//#define RMS_VERSION		0x010350				// [#RWC6-257] US William 2021.02.25 Add LibertyX Username to setup data
//#define RMS_VERSION		0x010351				// [#GLDV-3109] AU Kook 2022.01.25 Support Model Name, CDU Binding, DCC Local Surcharge
#define RMS_VERSION		0x010353				// [#RWC6-533] US ryan.payton 2023.05.26 [PAI] RMS MCCP Key Reset


//#define MWI_VERSION			_T("V06.03.00")
#define MWI_VERSION				_T("V06.03")		// [#2321] NH KSK 2015.01.14 AP Revision 처리를 위해 Version을 2개만 Set함


#define	NVRAM_ERRSTACK_VERSION	"V010000"			// [#419] [NH] KSK 2008.9.16
#define NVRAM_REJTOTAL_VERSION	"V010100"			// [#534] [NH] AIREAT 2009.06.04


//#define JNLDB_VERSION			(0x56010000)		// [#] NH AIREAT 2008.10.29
#define JNLDB_VERSION			(0x56010001)		// [#462] [NH] KSK 2008.12.08 이전 Journal File을 삭제하지 못하는 Bug Fix


// ----------------------------------------------------------------------------
// 축퇴디바이스 설정 2008.04.03 PSC
// ----------------------------------------------------------------------------
#define DEV_AUTO_OFF_CDU	0				// CDU Auto-Off
#define DEV_AUTO_OFF_SPR	1				// SPR Auto-Off

// ----------------------------------------------------------------------------
// Support Software TTS								[#2368] US KSK 2015.08.31 Support Software TTS
// ----------------------------------------------------------------------------
#define SUPPORT_SOFTWARE_TTS			0

// ----------------------------------------------------------------------------
// ADA PATH DEFINE									[#157] KSK 2008.04.22
// ----------------------------------------------------------------------------
#define	ADA_WAVE_FILE_PATH		_T("\\ATM\\Wave\\")
#define	ADA_WAVE_DAT_FILE		_T("WaveLoad.dat")		//[#2077] CA PCS 2011.07.02
#define ADA_TTS_FILE_PATH		_T("\\ATM\\TTS\\")		// [#2368] US KSK 2015.09.01
#define	ADA_TTS_DAT_FILE		_T("VoiceText.dat")		// [#2368] US KSK 2015.09.01
#define ADA_TTS_DB_FILE_PATH	"\\ATM\\TTS_DB\\EN_US"	// [#2368] US KSK 2015.09.01

// ----------------------------------------------------------------------------
//	Cash Condition Amount Define
// ----------------------------------------------------------------------------
#define CDU_MCASH_MAXOUT		8000					// MAX OUT : $5000  // RWC6-648 Dispense Limit 8000
#define CDU_MCASH_MINOUT		50						// MIN OUT : $50
#define CDU_MULTI_MCASH_MAXOUT	4000					// MAX OUT : $4000	[#514] [MX] KSK 2009.3.10
#define CDU_MULTI_MCASH_MINOUT	1						// MIN OUT : $1		[#514] [MX] KSK 2009.3.10
#define CDU_CST_MAXFASTCASH		6						// MAXIMUM FAST CASH COUNT
#define	CDU_MAX_CST_COUNT		4						// [#281] [NH] JSW 2008.6.10

#define CST_MAXDISPENSE			40						// MAXIMUM DISPENSE COUNT
#define CST_W_MAXDISPENSE		20						// [#2098] US KSK 2011.11.28
#define CDU_NEARCASHCNT			100						// NEAR CASH COUNT	
#define CDU_FULLREJECTCNT		200						// FULL REJECT COUNT

// ----------------------------------------------------------------------------
//	MEXICO AVAILABLE DENOMINATION						[#282] [NH] KSK 2008.6.11 국가별 권종 DEFINE 추가 및 MWI 적용
// ----------------------------------------------------------------------------
#define CASH_DENOMINATION1		20						// $20 pesos
#define CASH_DENOMINATION2		50						// $50 pesos
#define CASH_DENOMINATION3		100						// $100 pesos
#define CASH_DENOMINATION4		200						// $200 pesos
#define	CASH_DENOMINATION5		500						// $500 pesos
#define	CASH_DENOMINATION6		0						// $0 pesos
#define	CASH_DENOMINATION7		0						// $0 pesos
#define	CASH_DENOMINATION8		0						// $0 pesos
// end of [#282]

// [#514] [MX] KSK 2009.3.5 Multi Currency 지원을 위해 Denomination 정보 추가(미국권종)
#define MULTI_CASH_DENOMINATION1	1					// $1
#define MULTI_CASH_DENOMINATION2	5					// $5
#define MULTI_CASH_DENOMINATION3	10					// $10
#define MULTI_CASH_DENOMINATION4	20					// $20
#define	MULTI_CASH_DENOMINATION5	50					// $50
#define	MULTI_CASH_DENOMINATION6	100					// $100
#define	MULTI_CASH_DENOMINATION7	0					// $0
#define	MULTI_CASH_DENOMINATION8	0					// $0
// end of [#514]

// ----------------------------------------------------------------------------
// SCREEN TIMEOUT [#236] AIREAT 2008.05.28
// ----------------------------------------------------------------------------
#define NORMAL_SCR_TIMEOUT			30					// AP 화면 타임아웃.
#define INFO_SCR_TIMEOUT			5					// 경고시 안내문구 타임아웃.
#define ABORT_SCR_TIMEOUT			3					// 최소시 안내문구 타임아웃.

// ----------------------------------------------------------------------------
// NVRAM Default Value.
// ----------------------------------------------------------------------------
#define TERMINAL_ID_DEFAULT			L"00000000"			// [#398] [NH] KSK 2008.8.6
#define	HOSTPHONE1_DEFAULT			L"0"				// [#426] [NH] KSK 2008.9.24
#define	HOSTPHONE2_DEFAULT			L"0"				// [#426] [NH] KSK 2008.9.24
#define	MASTERPW_DEFAULT			L"1:0:$2$gYTcUUhRepZGnhPO$Il8D1xfPAt1/gQ7gfrJHh2XfMluv0a1Lu4Rp85xP51GgzUtXnEfibAgiXHOHKyPDXPDFJLH6rgd8YitS5AYnAA==:0"			// [#419] [NH] KSK 2008.9.18
#define	SERVICEPW_DEFALUT			L"1:2:$2$4a9nICkgVr6GbXaq$1Di8tbKZD9zKy7AZAQ4JK+qnhXBcaq1cwsXOzIEiWVOtc+bTYS1Hy3wqAFpcl+tcQzPH9sQpeJBjLS/K2PfgAA==:0"			// [#419] [NH] KSK 2008.9.18
#define	OPERATORPW_DEFALUT			L"1:1:$2$cWZZLZXEyKMU2IsV$nQbceOXgKpxNv7gbglMQyqTXGuIlgQy6xCFFqUDryNMCpMEJc9trS9Sb4X4eSYEBwUOJY0jrBgjlvq3LwN+AAA==:0"			// [#419] [NH] KSK 2008.9.18

#define	DISPENSELIMIT_DEFAULT		L"500000"
#define	FASTCASH1_DEFAULT			100
#define	FASTCASH2_DEFAULT			400
#define	FASTCASH3_DEFAULT			200
#define	FASTCASH4_DEFAULT			500
#define	FASTCASH5_DEFAULT			300
#define	FASTCASH6_DEFAULT			1000
#define	SURCHAREAMOUNT_DEFAULT		L"00006960"			// KSK 2010.06.25	// KSK 2010.11.01 Default값 변경
#define	SURCHARGEOWNER_DEFAULT		L"OWNER"

#define	RECEIPT_HEADER1_DEFAULT		L""
#define	RECEIPT_HEADER2_DEFAULT		L""
#define	RECEIPT_HEADER3_DEFAULT		L""

#define ADV_GUIDEDISP_TIME_DEFAULT	5
#define	REVERSAL_RETRY_DEFAULT		2

// ----------------------------------------------------------------------------
// EMV and other options
// ----------------------------------------------------------------------------
#define	EMV_LEVEL2						1			// [#2115] EMV Level2 지원
#define	EMV_TEST_MODE					0			// [#2115] NH KSK 2011.10.24 EMV 거래 시 최종 1거래를 FILE에 저장하도록 기능 추가
#define SUPPORT_CHANGE_PARAMETER_JNL	0			// [#610] SOOK 2009.12.21 Configuration Change Journal 저장 

// ----------------------------------------------------------------------------
// OP TIMEOUT시 전환 화면 DEFINE
// ----------------------------------------------------------------------------
#define OP_TIMEOUT_SCREEN_NUMBER		899

// ----------------------------------------------------------------------------
// OP TIMEOUT시 전환 화면 DEFINE					// [#540] HWANG 2008.07.01
// ----------------------------------------------------------------------------
#define DIAG_TIMEOUT_SCREEN_NUMBER		829			

#endif // _BASE_DEF_H_
