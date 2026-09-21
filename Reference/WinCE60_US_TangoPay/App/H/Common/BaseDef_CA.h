#ifndef _BASE_DEF_H_
#define _BASE_DEF_H_

// ----------------------------------------------------------------------------
/////////////////////////           AP MODE DEFINE        /////////////////////
// ----------------------------------------------------------------------------
//#define	APP_LOCAL_MODE			// [#15] NH KGS 2008.03.10 Online-Mode로 동작하도록 수정
//#define	CDU_DECRYPT_MODE		// [#2095] NH KSK 2011.10.27 전시회용 MODE CDU 비 암호화 모드
//#define	APP_AGING_MODE			// AGING 용
// 	#define AGING_DIAL				// AGING OPTION (3개 중 1개만 활성화)
// 	#define AGING_TCPIP				// AGING OPTION (3개 중 1개만 활성화)


#define TERRCODE "CA\0"
#define NO_SECURE_TLS_MODE
// ---------------------------------------------------------------------------
//////////////////////////// Available Options ////////////////////////////////
// ----------------------------------------------------------------------------

#ifndef APP_DEF
#  define APP_TDL_OPTION			0	// C06.XX.XX"	// [#2291] CARDTRONICS ONLY(TDL OPTION)
#endif

#if (APP_CUSTOM_CARDTRONICS)
#  define APP_TDL_OPTION 			1
#else
#  define APP_TDL_OPTION 			0
#endif


// ----------------------------------------------------------------------------
////////////////// Additional Options -- No version number change  ////////////
#define APP_DCC_WITHOPTION			0	// [#2472] NH Justin 2017.02.07 DCC Withdrawal Option
#define APP_B4U						1	// [RWC6-162] Add B4U to Canada Release
#define PIN_CHANGE					0	// PIN Change Transaction


// ----------------------------------------------------------------------------
///////////////////////// Disabled OR Unavailable Options /////////////////////
//////                           DO NOT ENABLE !!!!!                    ///////
// ----------------------------------------------------------------------------
#define APP_CUSTOM_PAI				0	// [#2182] US PAI ONLY (Dual Host DCC)
#define	APP_EVENT_NOTICE			0	// [#2313] US Event Notice
	#define EVENT_NOTICE_TRANONLY	0	// [#2313] US Event Notice SUB MENU
#define APP_DUALBALANCE				0	// [#2444] US Justin 2016.10.12 DUal Balance Option
#define APP_JUST_CASH				0	// [#2445] US Justin 2016.09.21 Just.Cash Cardless Transaction
#define APP_PAYDIANT_CCA			0	// [#2446] US Justin 2016.09.29 Paydiant (Paypal) CCA(Cardless Cash Access)
#define APP_PIN4_CASHPICKUP			0	// [#2471] US Justin 2017.02.01 Pin4
	#define	APP_PRESTAGIN_PIN4		0	// [#2515] US Justin 2017.11.09 Mastercard phase2 development
#define APP_POPMONEY				0	// [#2471] US Justin 2017.02.01 Popmoney
#define APP_GPAY_GIFTCARD_PURCHASE	0	// [#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
#define APP_CUSTOM_CASHDEPOT		0	// [#J007] Cash Depot (QR CODE on Idle Screen)
#define APP_LIBERTYX				0   //[#RWC6-59] US William 2019.10.03 LibertyX 

// ----------------------------------------------------------------------------
/////////////////////////          Enabled Options        /////////////////////
//////                           DO NOT DISABLE !!!!!                   ///////
// ----------------------------------------------------------------------------
#define	APP_STD1_ENHANCED_V2		1	// [#2177] Standard1 Enhanced Version 2 (ENABLED)
#define MONIMOBILE_QRCODE			1	// Enabling MoniMobile Option		[#2410] CA Justin Enable MoniMobile

// ----------------------------------------------------------------------------
//////////////////////////// Versions /////////////////////////////////////////
// ----------------------------------------------------------------------------
// [#2291] Available Versions in Canada 
//	V06.XX.XX		: Generic NH Version
//	T96.XX.XX		: DEMO Version
//  B06.XX.XX       : CASH DEPOT  // RWC6-645 DYNAMICDCC
//  D06.XX.XX       : DYNAMICDCC  // RWC6-645 DYNAMICDCC  

#ifdef	APP_LOCAL_MODE
	#define AP_TYPE		_T("T")
	#define APCODE		"T\0"
#else
	#if(APP_CUSTOM_DYNAMICDCC)  // RWC6-645 DYNAMICDCC RWC6-655 Build V instead D
		#define AP_TYPE		_T("V")
		#define APCODE		"V\0"
 	#elif(APP_TDL_OPTION)
		#define AP_TYPE		_T("C")
		#define APCODE		"C\0"
	#else
		#define AP_TYPE		_T("V")
		#define APCODE		"V\0"
	#endif
#endif

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
//#define AU_A_VERSION	0	// 호주 - ATM Plus
//#define AU_C_VERSION	0	// 호주 - Customers
#define MX_VERSION		0	// 멕시코
#define CA_VERSION		1	// 캐나다
// ----------------------------------------------------------------------------
// DATE FORMAT DEFINE (하나만 1로 설정) 2008.03.03 JSW
// ----------------------------------------------------------------------------
#define	DATE_MMDDYYYY	1
#define DATE_DDMMYYYY	0
#define DATE_YYYYMMDD	0

// ----------------------------------------------------------------------------
// [#50] NH PSC 2008.03.26 국가별 화폐단위 표시
// ----------------------------------------------------------------------------
//#define CURRENCY_SYMBOL	(TCHAR)0xA3		// 국가별 화폐단위 표시(영국 파운드): L'￡'
#define CURRENCY_SYMBOL	(TCHAR)0x24			// 국가별 화폐단위 표시(미국 달러): L'$'

// ----------------------------------------------------------------------------
// [#240] NH 2008.05.28 국가별 Checking Account 표현
// ----------------------------------------------------------------------------
//#define	S_ACCOUNT_CHECKING _T("CHEQUE")		// 영국
#define	S_ACCOUNT_CHECKING _T("CHEQUING")	// 캐나다
//#define	S_ACCOUNT_CHECKING _T("CHECKING")		// 미국
// end of [#240]

// ----------------------------------------------------------------------------
// [#50] NH PSC 2008.03.26 국가별 화폐단위 for MWI
// ----------------------------------------------------------------------------
//#define CURRENCY_TYPE	_T("USD")			// 국가별 화폐단위 (미국 달러):		'$'
//#define CURRENCY_TYPE	_T("GBP")			// 국가별 화폐단위 (영국 파운드):	'￡'
#define CURRENCY_TYPE	_T("CAD")			// 국가별 화폐단위 (캐나다 달러):	'$'
//#define CURRENCY_TYPE	_T("NZD")			// 국가별 화폐단위 (뉴질랜드 달러):	'$'
//#define CURRENCY_TYPE	_T("AUD")			// 국가별 화폐단위 (호주 달러):		'$'
//#define CURRENCY_TYPE	_T("MXN")			// 국가별 화폐단위 (멕시코 페소):	'$'
#define MULTI_CURRENCY_TYPE	_T("USD")		// 국가별 화페단위 (미국달러):	'US$'	// [#514] [MX] KSK 2009.3.4

// ----------------------------------------------------------------------------
// 기종구분 정보 2008.03.27 PSC
// ----------------------------------------------------------------------------
#if (_WIN32_WCE < 0x600)
	#define WINCE_MACHINETYPE	_T("5000CE")	// 5000CE
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

#define WINCE_RMS_5000_MACHINECODE				0x32	// 5000 CE - CA	
#define WINCE_RMS_5300_MACHINECODE				0x42	// 5300 CE - CA 기기 나간적 없음 (RMS와 Code와 미리 Define하였음)
#define WINCE_RMS_1800_MACHINECODE				0x07	// 1800 CE - CA 기기 나간적 없음 (RMS와 Code와 미리 Define하였음)

#define WINCE_RMS_2700_MACHINECODE_FRONT		0x45	// NH2700CE FRONT
#define WINCE_RMS_1800SE_MACHINECODE_FRONT		0x59	// NH1800SE FRONT	// [#11] NH KSK 2010.09.27 REAR는 의미가 없으므로 1800SE로 수정
#define WINCE_RMS_MX4000W_MACHINECODE			0x5D	// [#2097] US PCS 2011.11.15 "WallMount"
#define WINCE_RMS_MX5000SE_MACHINECODE			0x60	// [#2097] US PCS 2011.11.15 "MX5000SE" - 추가 정의 필요
#define WINCE_RMS_HALO2600_MACHINECODE			0x63	// [#2222] CA Justin 2013.10.03 Add HALO
#define WINCE_RMS_NH1500SE_MACHINECODE			0x65	// [#2267] US KSK 2014.05.12
#define WINCE_RMS_MX5200SE_MACHINECODE_FRONT	0x67	// [#2303] US KSK 2014.11.07		// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
#define WINCE_RMS_HALO2_MACHINECODE				0x6D	// [#2332] NH Justin 2015.02.10 Add Halo2 Machine code (109)
#define WINCE_RMS_MX5300SE_MACHINECODE			0x71	// [#2383] NH Justin 2015.12.18 Add 5300 SE Machine Code (113)
#define WINCE_RMS_MX2800SE_MACHINECODE			0x76	// [#2529] NH Justin 2018.01.23 Add 2800 SE Machine Code (118)
#define WINCE_RMS_MX2800T_MACHINECODE			0x7A	// [#GLDV-2505] US Kook 2019.05.22  Add 2800 T Machine Code (122 for CA)

#define WINCE7_RMS_COMMON_MACHINECODE			0x7E	// All WinCE 7.0 machines

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

	//#define AP_VERSION		_T("V06.00.02")			// 캐나다 EMV 초기 인증 버전
	//#define AP_VERSION		_T("V06.00.04")			// CDU SP 암호화 적용
	//#define AP_VERSION		_T("V06.00.05")			// 캐나다 정식 Release 버전 (NH2700)
	//#define AP_VERSION		_T("V06.00.06")			// 캐나다 EMV new kernel 인증 버전
	//#define AP_VERSION		_T("V06.00.07")			// 캐나다 NH1800SE 정식 발행 버전
	//#define AP_VERSION		_T("V06.00.08")			// 캐나다 NH1800SE 정식 발행 버전
	//#define AP_VERSION		_T("V06.00.08")			// 캐나다 NH1800SE 정식 발행 버전
	//#define AP_VERSION		_T("V06.00.09")			// 품질 안정화 판
	//#define AP_VERSION		_T("V06.00.10")			// 태남CDU 연동 버전
	//#define AP_VERSION		_T("T06.00.12.00")		// SSL 비정상 종료 Bug Fix (공장 TEST용)
	//#define AP_VERSION		_T("T06.00.12.01")		// SSL 비정상 종료 Bug Fix - shutdown 제거(NHA TEST용)
	//#define AP_VERSION		_T("T06.00.12.02")		// WatchDoc 추가 (공장용)
	//#define AP_VERSION		_T("V06.00.12")			// 내부검사 Version
	//#define AP_VERSION		_T("V06.00.13")			// NH2700T 신규 기종 지원
	//#define AP_VERSION		_T("V06.00.14")			// 
	//#define AP_VERSION		_T("T06.00.14")			// 
	//#define AP_VERSION		_T("V06.00.14")			// 
	//#define AP_VERSION		_T("V06.00.15")			// KeyMgr Bug로 인해 Version 올림 (AP 변경 없음)
	//#define AP_VERSION		_T("V06.00.16")			// IDLE에서 Sankyo SP 통신장애 대책 (수정된 SP 적용)
	//#define AP_VERSION		_T("V06.00.17")			// MCU / ESU F/W 변경에 따른 매체작업
	//#define AP_VERSION		_T("V06.00.18")			// Middleware 적용 및 MWI 보완
	//#define AP_VERSION		_T("V06.00.19")			// 1. 장애대책 : VDM SPR Dll 적용
	//#define AP_VERSION		_T("V06.00.20")			// 1. OP, 명세표, 저널 French 지원 [#2160], CDU Retrial Logic Added
	//#define AP_VERSION		_T("V06.01.00")			// Enhanced T.H V2 - DCC...
	//#define AP_VERSION		_T("V06.01.02")			// Enhancement - Digital Receipt / Add HALO (LED)
	//#define AP_VERSION		_T("V06.01.03")			// Enhancement - Digital Receipt / Add HALO (LED)
	//#define AP_VERSION		_T("V06.01.04")			// [#2259]~[#2260] NH KSK 2014.03.13 PCI3.0 대응, CDU TYPE 추가
	//#define AP_VERSION		_T("V06.01.05")			// [#2262],[#2263] Justin 2014.05.05 AU Code Merge, Digital receipt Option
	//#define AP_VERSION		_T("V06.01.06")			// Support 1500SE, Security Enhancement
	//#define AP_VERSION	AP_TYPE	_T("06.01.07")		// Card Reader 31002 Error Fix, Not sending F0001 on Add Cash HeartBeat after Cassette Total
	//#define AP_VERSION	AP_TYPE	_T("06.01.08")		// SPR EP Downloading, Printer Column Set
	//#define AP_VERSION	AP_TYPE	_T("06.01.09")		// SSL/TLS Version : ABOVE => UPTO, Percentage Surcharge for DCC, CIBC Option Background
	//#define AP_VERSION	AP_TYPE	_T("06.01.10")		// Support Asian Language Pack
	//#define AP_VERSION	AP_TYPE	_T("06.01.11")		// Support Percentage and Amount Surcharge
	//#define AP_VERSION	AP_TYPE	_T("06.01.12")		// Fix Freezing, TC for non EMV Data Case
	//#define AP_VERSION	AP_TYPE	_T("06.01.13")		// Fixing 4000W Space issue (Below Image) 
	//#define AP_VERSION	AP_TYPE	_T("06.01.14")		// CDU Encrpytion / Canadian Local Loan / Combine ADA and Screen Transacton Flows
	//#define AP_VERSION	AP_TYPE	_T("06.01.15")		// Increase Dispense Limit :40=>50  / Support MX5300SE, Local Loan Test Versions
	//#define AP_VERSION	AP_TYPE	_T("06.01.16")		// Increase Dispense Limit :40=>50  / Support MX5300SE / Local Loan
	//#define AP_VERSION	AP_TYPE	_T("06.01.17")		// Support EMV Kernel V6.0
	//#define AP_VERSION	AP_TYPE	_T("06.01.18")		// New EMV card Reading Animation
	//#define AP_VERSION	AP_TYPE	_T("06.01.19")		// Print TSI, TVR, and TC on an receipt, EMV Journal
	//#define AP_VERSION	AP_TYPE	_T("06.01.20")		// Blank OP Menu Bug Fix (AU BillMix)
	//#define AP_VERSION	AP_TYPE	_T("06.01.21")		// [#2518] US Kook 2017.12.21 Support MX-2800SE
	//#define AP_VERSION	AP_TYPE	_T("06.01.22")
	//#define AP_VERSION	AP_TYPE	_T("06.01.23")			// 2018.08.22
	//#define AP_VERSION	AP_TYPE	_T("96.01.24")		// [#RWC6-22] EMV ADVT v7.0 Test 		
	//#define AP_VERSION	AP_TYPE	_T("06.01.24")		// [#RWC6-9] Visa DCC
	//#define AP_VERSION	AP_TYPE	_T("06.01.25")		// [#GLDV-2505] US Kook 2019.05.13 Support MX-2800T
														// [#GLDV-2529] Addressing WINCE vulnerability. (RMS buffer overflow, XFS DLL External Socket Listening)
	//#define AP_VERSION	AP_TYPE	_T("06.01.25")			// [#RWC6-12] Denomination Selection Support
														// [#RWC6-71] Bitload4U enhancement
														// [#RWC6-68] PAI Bitcoin button
														// [#RWC6-63] Terminal IP screen freezes
														// [#RWC6-34] RMS TLS
														// [#RWC6-61] D1500 recovery (AutoCfg disable)
														// [#RWC6-55] TLS refactor
														// [#RWC6-59] LibertyX
														// [#RWC6-85] Dual Balance Screen Changes - PAI
														// [#RWC6-60] Remove sensitive data in NVRAM after TXN
														// [#RWC6-27] Configurable Dispense Methods
														// [#RWC6-29] Allpoint Button
														// [#RWC6-30] 2800SE text outlines
														// [#RWC6-89] Remove delay in ReloadLanCard()
														// [#RWC6-20] PCI 5.0 EPP in NHA
														// [#RWC6-37/48] Disk Cleanup
														// [#RWC6-70] Credit Dual Balance Options
														// [#RWC6-80] Disable fast cash options
														// [#RWC6-83] PIA Default Scheduled reboot
														// [#RWC6-88] Vault door error code blank on boot fix
														// [#RWC6-91] B4U and LTX status monitoring fields in STD1 
														// [#RWC6-72] US William 2019.12.09 PAI INI file
														// [#RWC6-115] B4U: API Changes
														// [#RWC6-116] B4U Registration QR
	//#define AP_VERSION	AP_TYPE	_T("06.01.27")		// [#RWC6-118] Remove JustCash BTC Purchase from Cardless
	//#define AP_VERSION	AP_TYPE	_T("06.01.28")		// [#RWC6-129] LTX STD3 Fixes
														// [#RWC6-131] 1500 Installer Issue
														// [#RWC6-128] B4U fixes
														// [#RWC6-130] Welcome screen copy positioning
	//#define	AP_VERSION	AP_TYPE	_T("06.02.00")		// [#RWC6-73] Remove US NFC
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
	//#define	AP_VERSION	AP_TYPE	_T("06.02.01")		// [#RWC6-154] Fix for PAS buffer overrun 
														// [$RWC6-155] Remove GivePayConfig.xml from normal build
	//#define	AP_VERSION	AP_TYPE	_T("06.02.01.01")	// [#RWC6-156] Fix static IP PAS Crash
														// [#RWC6-123] Transaction Language selection is always english bug
														// [#RWC6-135] PAI credit additional surcharge
														// [#RWC6-158] JustCash QR receipt bug fix
														// [#RWC6-126] Increase anti-skimming timeout
														// [#RWC6-141] Change default EMV type on CA
														// [#RWC6-161] Update EPP EP/SP to prevent comm issues during CPU resets
														// [#RWC6-163] Allow LTX to use dual host keys if dual host is disabled on EP versions < V10
														// [#RWC6-164] Fix LTX welcome screen copy
														// [#RWC6-165] Fix LTX receipt copy
														// [#RWC6-132] Health Check after B4U Transaction
	//#define	AP_VERSION	AP_TYPE	_T("06.02.01.02")		
														// [#RWC6-162] Add B4U to Canada Release	
														// [#RWC6-117] Update JSON parsing/serialization library
														// [#RWC6-127] Add disk utilization to RMS setup message
														// [#RWC6-132] Add optional health check after B4U transaction
														// [#RWC6-134] Add support for RMS remote update job
														// [#RWC6-170] Add setup data for remote update configuration
														// [#RWC6-149] Display remote update config on PAS
														// [#RWC6-169] PAI: Credit segmentation with RMS
														// [#RWC6-162] Enable B4U in Canada
														// [#RWC6-166] Don't go out of service if GivePay is down
														// [#RWC6-167] LibertyX API changes  
	//#define	AP_VERSION	AP_TYPE	_T("06.02.02")
														// [#RWC6-209] RLS over TLS fix
														// [#RWC6-142] Checge HTTP to use non-blocking IO
														// [#RWC6-168] Add anti-skimming 2B field for RMS
														// [#RWC6-178] Add MoniView setup for STD3 comm header
														// [#RWC6-181] Add HTTP basic auth for software repo config
														// [#RWC6-188] Log journal entry for B4U
														// [#RWC6-190] Update.exe failure on VS2008
														// [#RWC6-192] LTX flow modifications
														// [#RWC6-195] EPP EP <= 8.0 fails for txn continue (remove PAI second PIN entry)
														// [#RWC6-196] EMV kernel V7 update for EMV continue txn 
														// [#RWC6-197] CATM VG for DCC error codes
														// [#RWC6-198] CATM dual balance VG
														// [#RWC6-199] CATM Visa DCC offering screen button issue
														// [#RWC6-201] VG surcharge acceptance "7 to accept" 
														// [#RWC6-203] CATM exchange rate not showing for Visa DCC
														// [#RWC6-205] EPP EP update V15.00.01.07
														// [#RWC6-210] Receipt not ejecting on STD1 DCC
														// [#RWC6-191] Update STD3 tsm fields (B4U, libertyx, credit segmentation)
														// [#RWC6-204] Cardtronics EMV TDL Lookup error handling
														// [#RWC6-206] B4U receipt incorrect surcharge
														// [#RWC6-207] LibertyX MoniView setup data
														// [#RWC6-208] LibertyX MoniView password change job
														// [#RWC6-212] Add last 6 hash for CoinSquare purchase route
														// [#RWC6-184] F000B ECode re-wording 

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
	//#define	AP_VERSION	AP_TYPE	_T("06.02.05")


														// [#RWC6-235] - CATM VISA DCC receipt verbiage
														// [#RWC6-247] - EJUpload 'J' FID not initiating EJUP transaction
														// [#RWC6-245] - Bansi DCC Customizations
														// [#RWC6-242] - Add DF to Mexico Version
														// [#RWC6-243] - LibertyX Voice Guidance
														// [#RWC6-246] - Downgrade TLS ciphers for MoniView
														// [#RWC6-236] - Add LibertyX KCV to PrintAllSetup
														// [#RWC6-239] - CATM - French VISA DCC Receipt Message
														// [#RWC6-240] - CATM - MasterCard DCC Partial Reversal Missing Fields
														// [#RWC6-244] - Add Btye2.b1 for DCC DF enablement
														// [#RWC6-251] - MCCP client auth error
														// [#RWC6-252] - LTX PAS translation error
	// #define	AP_VERSION	AP_TYPE	_T("06.02.06")


														// Fix Bitload4U Canadian Dollar glitch	RWC6-249
														// Add CATM root certificate	RWC6-277
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

//  #define	AP_VERSION	AP_TYPE	_T("06.03.02.02") 
//  #define AP_RC_VERSION "06.03.02.02\0"

//  #define	AP_VERSION	AP_TYPE	_T("06.03.02.03") 
//  #define AP_RC_VERSION "06.03.02.03\0"

//  #define	AP_VERSION	AP_TYPE	_T("06.03.02.04") 
//  #define AP_RC_VERSION "06.03.02.04\0"
														// Cash Depot - Requesting that the card reader guide light to be on when card is returned. RWC6-650
														// Cash Depot - Requesting that the an audio beep is played when card is returned. RWC6-651
														// Cash Depot - The maximum dispense limit is being set to a default value instead of host configured value.RWC6-652
														// Cash Depot - DCC is being denied by the host when the admin fee is enabled. RWC6-659

//  #define AP_VERSION	AP_TYPE	_T("06.03.02.05") 
//  #define AP_RC_VERSION "06.03.02.05\0"

//  #define AP_VERSION	AP_TYPE	_T("06.03.03") 
//  #define AP_RC_VERSION "06.03.03\0"

#define	AP_VERSION	AP_TYPE	_T("06.03.04") 
#define AP_RC_VERSION "06.03.04\0"


	//#define AP_DETAIL_VERSION	6,0,1,0				// SHOW 기기 인증 버전 (1차).
	//#define AP_DETAIL_VERSION	6,0,1,1				// SHOW 기기 인증 버전 (2차).
	//#define AP_DETAIL_VERSION	6,0,1,2				// SHOW 기기 인증 버전 (3차) - Reversal / Configuration 대책..
	//#define AP_DETAIL_VERSION	6,0,1,3				// NHA 요구 사항
	//#define AP_DETAIL_VERSION	6,0,2,0				// 미국 V6.0.1.10 최종작업 적용
	//#define AP_DETAIL_VERSION	6,0,2,1				// 1.OP CHECKING 문구 수정 -> CHEQUING
													// 2. After Amount일 때 Surcharge 가 두번 표시되는 Bug Fix
													// 3. Language Select에서 잘못 되는 Bug Fix
	//#define AP_DETAIL_VERSION	6,0,4,0				// CDU SP 암호화 적용
	//#define AP_DETAIL_VERSION	6,0,5,0				// 미국 머지 (날씨, Notice, 거래 중 광고 기능 추가 등)
	//#define AP_DETAIL_VERSION	6,0,6,0				// EMV New Kernel 적용
	//#define AP_DETAIL_VERSION	6,0,6,1				// Reversal 전문에 9F1A가 송신 안되는 Bug Fix
													// IC 거래 중 고객이 Card 미수취 시 APPROVED가 출력되는 Bug Fix
	//#define AP_DETAIL_VERSION	6,0,6,2				// Reversal시 Host에서 받은 Local Date & Time이 잘못 전송하는 Bug Fix
	//#define AP_DETAIL_VERSION	6,0,6,3				// Get Processing Option에서 "6985" 수신 시 FallBack하도록 수정 (VISA Issue 대응)
	//#define AP_DETAIL_VERSION	6,0,6,4				// Card Data 19 Byte를 처리하지 못하는 Bug Fix
	//#define AP_DETAIL_VERSION	6,0,6,5				// 8A Tag값이 3030 ~ 3939 범위 밖일 경우 처리 오류
													// KeyPad 배열 위치 변경 (E->C->C, C->C->E)
	//#define AP_DETAIL_VERSION	6,0,6,6				// 8A Tag값이 안 올 경우 30 35로 처리하도록 추가 수정
	//#define AP_DETAIL_VERSION	6,0,6,7				// 1. 2nd Generate AC 수행 중 CID가 바뀌지 않을 경우 "n" Field Code 추가 "08"
													// 2. VISA Test Case 22 Non ASCII 지원으로 인해 Kernel 수정 적용
													// 3. Host에서 89 Tag 존재 시 무시하도록 수정
													// 4. KSPR1VE 장애 대책 SP 적용
	//#define AP_DETAIL_VERSION	6,0,7,1				// 1. 2nd Generate AC 수행 중 CID가 바뀌지 않을 경우 "n" Field Code 추가 "08"
													// 2. 1800SE 기종 추가
	//#define AP_DETAIL_VERSION	6,0,7,2				// 1. 프랑스어 문구 수정
													// 2. RMS STATUS URL 입력시 15로 제한되는 Bug Fix
//	#define AP_DETAIL_VERSION	6,0,8,0				// 1. STANDARD3에서 EMV 거래 중 MAC ERROR가 발생하는 Bug Fix
													// 2. IDLE 상태에서 Health Check 중 통신 장애로 인해 Out Of Service로 가는 Bug Fix
													// 3. Weather 값이 "-"일 경우 잘못 표시되는 Bug Fix
													// 4. MB2500에서 NVRAM 동기화 부분 추가
													// 5. STANDARD1에서 Host Denial Description 표기 오류 수정
													// 6. 송신 Data가 512 byte 초과 시 Mac Error가 나는 Bug Fix
													// 7. CRC Enable인 상태에서 수신 Data에 "ub"값이 포함되는 경우 CRC Error가 나는 Bug Fix (MAC도 동일하게 수정)
													// 8. NHDIP에서 Card Read시 "DING" 소리 나도록 수정
													// 9. NH2700에서 VDM 진입 시 AP 비정상 종료 현상 수정 (stack size 변경 64KB -> 128KB)
													// 10. 조회 거래 시 이전 출금 금액이 송신되는 Bug Fix
	//#define AP_DETAIL_VERSION	6,0,8,1				// 1. CDU SP 변경 (V06.04.04)
	//#define AP_DETAIL_VERSION	6,0,8,2				// 1. CDU SP 변경 (V06.04.04)
													// 2. PIN SP 변경 (V06.04.04)
													// 3. Middleware 변경 (6.0.0.1)
													// 4. XFS 변경 (6.0.0.2)
	//#define AP_DETAIL_VERSION	6,0,8,3				// 1. CDU SP 변경 2CST시 비정상 종료 추가 대책 (V06.04.04) - 버전은 동일
	//#define AP_DETAIL_VERSION	6,0,9,0				// 1. Code Sonar 지적사항 대책
													// 2. Magtek MCU 장애시 NVRam에 Log를 지속적으로 Logging 하는 Bug Fix
	//#define AP_DETAIL_VERSION	6,0,10,0			// 1. TAENAM CDU GBM10CH VDM Sensor 추가
													// 2. Magtek IDC F/W Download 기능 추가
	//#define AP_DETAIL_VERSION	6,0,12,0			// 1. SSL 비정상 종료 Bug Fix (공장전달)
	//#define AP_DETAIL_VERSION	6,0,12,1			// 1. SSL 비정상 종료 Bug Fix - shutdown 제거(NHA전달)
	//#define AP_DETAIL_VERSION	6,0,12,2			// 1. WatchDoc 추가 (공장용))
	//#define AP_DETAIL_VERSION	6,0,12,0			// 1. 내부검사 Version
	//#define AP_DETAIL_VERSION	6,0,13,0			// 1. NH2700T 적용
	//#define AP_DETAIL_VERSION	6,0,13,1			// 1. Journal Print시 Cancel이 안먹히는 Bug Fix
													// 2. 광고 File Delete가 되지 않는 Bug Fix
													// 3. OP Notice 문구 짤림 수정
	//#define AP_DETAIL_VERSION	6,0,14,0			// 1. Journal Print시 Cancel이 안먹히는 Bug Fix
	//#define AP_DETAIL_VERSION	6,0,14,1			// 1. SANKYO VDM 진입 시 비정상 종료되는 현상 임시 대책판
	//#define AP_DETAIL_VERSION	6,0,14,2			// 1. IC 거래 시 BIN 등록된 CARD Surcharge를 감지하지 못하는 Bug Fix
	//#define AP_DETAIL_VERSION	6,0,15,0			// 1. EPP EP V08.20.00적용 시 KeyMgr에서 Master Key 주입 실패 Bug Fix
	//#define AP_DETAIL_VERSION	6,0,15,1			// 1. EMV 모드, Sankyo Device 에서 IDLE 상태에서 Power Off 송신 시 Media 상태가 없는 경우 장애나는 문제 수정
	//#define AP_DETAIL_VERSION	6,0,16,0			// IDLE에서 Sankyo SP 통신장애 대책 (수정된 SP 적용)
													// 고객 요청사항에 의해 Out of Service에 영어 / 불어 동시 표시
	//#define AP_DETAIL_VERSION	6,0,17,0			// MCU / ESU F/W 변경에 따른 매체작업
	//#define AP_DETAIL_VERSION	6,0,18,0			// [#2133] ~ [#2135]
	//#define AP_DETAIL_VERSION	6,0,19,0			// 1. 장애대책 : VDM SPR Dll 적용
	//#define AP_DETAIL_VERSION	6,0,20,0			// 1. OP, 명세표, 저널 French 지원 [#2160], CDU Retrial Logid / Kill DHCP Pop up
	//#define AP_DETAIL_VERSION	6,0,20,1			// 1. 내부검사 장애 대응
	//#define AP_DETAIL_VERSION	6,0,20,2			// 1. KeyMgr 내부검사 버전 적용
													// 2. Key Index가 Invalid한 경우 Buffer Overflow 나는 Bug Fix
	//#define AP_DETAIL_VERSION	6,0,20,3			// 1. SSL Connect시 POP UP 발생관련 보완 처리
													// 2. EMV Enable일 경우 TCP Connect Retry (3->1) 변경
													// 3. 프랑스 번역 문구 중 Device 약어 관련 영어로 유지하도록 수정
													// 4. RKT로 Mac Master Key 주입 오류 수정
													// 5. [#2176] Config Request Initiator 처리 로직 오류 수정
	//#define AP_DETAIL_VERSION	6,0,20,4			// 1. 2차 내부검사 지적사항 추가 대응
													// 2. Operator Action 저장시 MultiLine 처리시 Bug Fix
	//#define AP_DETAIL_VERSION	6,0,20,5			// 1. 고객 1차 검수 결과 반영 (문구 변경)
													// 2. 개행문자(\n)의 length 오류 수정 (저널 File Write 옵션 변경 w+ -> wb)
	//#define AP_DETAIL_VERSION	6,1,0,0				// 1. STD1 Enhanced V2 - DCC
	//#define AP_DETAIL_VERSION	6,1,0,1				// [#2180] NH Justin 2013.2.8 Change "Target" and "Base" currency (SCREEN and Receipt)
													//         NH Justin Store Currency ID (EMV)
													// Add 5000SE
	//#define AP_DETAIL_VERSION	6,1,0,2				// [#2182] NH Justin 2013.3.21 Implement NHD1.6
	//#define AP_DETAIL_VERSION	6,1,0,3				// [#2187] CA Justin 2013.04.15 Modify Screen Text
	//#define AP_DETAIL_VERSION	6,1,0,4				// [#2192] CA Justin 2013.05.02 Currency conversion Logic (CIBC)
	//#define AP_DETAIL_VERSION	6,1,0,5				// [#2183] NH Justin Receipt Option (No Transaction result on Screen)
													// [#2198] NH Justin Remove Sankyo => NH EMV
													// [#2199] NH Justin Add 4000W VG, Repeat on Welcome Screen.
													// [#2185] NH Justin 2013.06.05 Using Local Surcharge... (SPEC V1.61)
													// [#2201] NH Justin VG mode surcharge Bug Fix
	//#define AP_DETAIL_VERSION	6,1,0,6				// [#2202] NH Justin 
													//		1. Print Exchange Rate and converted amount for Partial Dispense
													//		2. Add Default DCC Disclaimer (3 Lines => 6 Lines) and assign default disclaimer
													//		3. Receipt Text Changes : Surcharge => Service Fee(CIBC ONLY), Account Charges => Account Charge
													// [#2203] NH Justin Bug fix : Remove Voice Guidance for Terminal Fee when the dispensed Amount = 0
	//#define AP_DETAIL_VERSION	6,1,0,7				// [#2204] NH Justin DCC screen text changes
													// 1. "Service fee" => "fee" on fee notice (removing one of [#2192] changes)
													// 2. "Currency Conversion" => "Cash withdrawals"
													// 3. Change Exchange rate for CIBC (1JPY = 0.0107CAD)													
													// [#2206] Add RMS Setup information (RMS VER01.03.20)
													//		1. Standard1 Dynamic Flow (En/Disable,PinChange,DCC,Disclaimer)
													//		2. Privacy receipt option
													//		3. DCC Option (General <=> CIBC)
	//#define AP_DETAIL_VERSION	6,1,0,8				// [#2208] Justin Add Enhanced Standard1 Option in the "Print All setup"
													// [#2209] Justin 
													//		1. Pin Change Option text - Print all setup (HOST DETERMINE => HOST DET.)
													//		2. Receipt on Screen Option - "Print All setup"
													//		3. Extended bin list - Support up to 10,000
													//	[#2215]	Implement Canada DCC pilot Test requests 
													//		1.DCC Offering French Text Change
													//		2.Declined IC Transaction => Remove "Chip Error"
													//		3.Bug fix : Displaying error for 1 digit DCC amount  (Bug : 1 => $0.10)
	//#define AP_DETAIL_VERSION	6,1,1,0				// [#2219] Justin Merge Digital Receipt
													// [#2221] Justin Update CDU SP (V06.06.07, F/W Download), Add CDU F/W (V0x.80.11) 
													// [#2222]	1. Justin 2013.10.03 Add HALO LED Control
													//			2. VCDU F/W Up (V22)
	//#define AP_DETAIL_VERSION	6,1,2,0				// [#2226] Justin EMV Kernel5, AID Selection
	//#define AP_DETAIL_VERSION	6,1,2,1				// NH Justin Sankyo SP 06.00.08 / CDU SP 06,.06.11.01
	//#define AP_DETAIL_VERSION	6,1,3,0				// DCC offer Green Button
													// [#2242] NH Justin DF 1st call option
													// [#2243] NH Justin Bug fix on Canadian 4000W sensor page
	//#define AP_DETAIL_VERSION	6,1,3,2				// [#2248] NH Justin Transaction Cancel Bug Fix for Surcharge Free ADA Mode
	//#define AP_DETAIL_VERSION	6,1,3,3				// [#2252] NH Justin Long AID Patch
	//#define AP_DETAIL_VERSION	6,1,4,0				// [#2259]~[#2260] NH KSK 2014.03.13 PCI3.0 대응, CDU TYPE 추가													
	//#define AP_DETAIL_VERSION	6,1,5,0				// [#2262] NH Justin AU Code Merge
													// [#2263] CA Justin Digital Receipt Option (2700, Canada Only)
													// [#2264] NH Justin Table Surcharge
	//#define AP_DETAIL_VERSION	6,1,6,0				// [#2276] ~ [#2280] NH Justin Support Remote Password/Status Management
													// [#2282] Add Missed "Remove Card" routine
													// [#2283] Changing DCC UI (Receipt:WITHDRAWAL), Support Customized Hot Key
													// Justin Support 1500SE 
	//#define AP_DETAIL_VERSION	6,1,6,1				// [#2286] NH Justin Add Country compile option for "negative counting option on RMS downloading"
	//#define AP_DETAIL_VERSION	6,1,7,0				// [#2287]	NH Justin Enable CDUM Log Backup
													// [#2290] Ignoring "31002" Error on Unlatching command  => NO CODE Change (SP Changed)
													// [#2290] Not Sending F0001 (STD3:384) error code on Add Cash Heartbeat after Cassette Total.
	//#define AP_DETAIL_VERSION	6,1,8,0				// [#2295] SPR EP Download [#2298] Print Column Set, SPR SP change => Support old 1500POS Printers
													// [#2302] US Justin Move IC Processing after "Fallback" check
													// [#2304] US Justin Clearing Master Password only when NVRAM is Broken
													// [#2304] Display EMV AID Name on PIN Entry Screen
													// [#2306] KSK Add MWI Log
													// [#2307] KSK Remove time delay after Image Priting
													// FIX Card reader EMV 99999 ERROR => SP Change
													// [#2312] Support MX5200, Support TLS V1.2
													// [#2316] Support Decimal Point Surcharge
	//#define AP_DETAIL_VERSION	6,1,9,0				// [#2320] WorldPay SSLV3.0 Issue : SSL/TLS Option : "ABOVE" => "UP TO"
													// [#2330] Support Percentage (local) surcharge for DCC transaction	
													// [#2331] CIBC DCC Offer Background change
													// [#2335] Percentage Surcharge Notice
	//#define AP_DETAIL_VERSION	6,1,10,0			// [#2339] Support Asian Language Pack
													// [#2342] AID Selection / Remote AID Selection 
													// [#2346] NH Justin Digital Receipt - Adding Address and Country Code in Digital Receipt.
													// [#2349] NH Justin DCC Local Surcharge
	//#define AP_DETAIL_VERSION	6,1,11,0			// [#2354] NH Justin Support Percentage and Amount Surcharge Notice
													// [#2355] NH Justin EJ Up Bug Fix ( Out of service error code )
													// [#2360] NH Justin Add DCC Disclaimer
													// [#2356] NH KMK TCPIP QUEUE Bug Fix 												
													// [#2363] NH KMK SSL/TLS Enahnce Certificate Check Logic 
													// [#2364] US Justin Remote update fast cash, msg format, eot option
													// [#2365] US Justin Enable TDL DCC
	//#define AP_DETAIL_VERSION	6,1,11,1			// [#2366] KMK Print All Setup Bug Fix - STD1 Enhance Option, Host Port #2
	//#define AP_DETAIL_VERSION	6,1,12,0			// [#2370] KSK 2015.09.09 Fixed bug after reinstalled AP or NVRAM Clear, failed AP boot up
													// [#2371] US Justin 4000W partial / full cut
													// [#2372] NH Justin TC for NON EMV Data Case
													// [#2373] US Justin 2015.09.22 Support Barcode Printing
	//#define AP_DETAIL_VERSION	6,1,13,0			// Change Receipt Image Media / Fix 4000W Image Printing Issue (Space below image)
	//#define AP_DETAIL_VERSION	6,1,14,0			// Combine ADA and Screen Transaction Flows
													// [#2383] US Justin Supporting MX5300SE
													// [#2384] US Justin Supporting CDU Binding
													// [#2385] US Justin Supporting Canadian Local Loan
													// [#2386] KSK Selective CDU Binding Option => Binding Icon
													// [#2392] US Justin Adding CDU Binding Option (Default : Disable)
													// [#2394] NH KSK 2016.02.05 OS Background Image 변경시 WinAtm Exception 발생하는 오류 수정
													// [#2397] KSK SPR 97922 Error => Reboot
	//#define AP_DETAIL_VERSION	6,1,14,1			// [#2400] CA Justin 2016.03.02 Add Company Logo on Local Loan Screen
	//#define AP_DETAIL_VERSION	6,1,14,2			// [#2402] CA Justin 2016.03.11 Move Loan Dispense result to ATM2 area to prevent to be erased when AP udate
	//#define AP_DETAIL_VERSION	6,1,14,3			// [#2404] CA Justin 2016.03.14 Disable Local Loan and Release Canadian Version to support Printer Error 
	//#define AP_DETAIL_VERSION	6,1,15,0			// [#2407] CA Justin Increase Maximum notes/dispense : 40 => 50
	//#define AP_DETAIL_VERSION	6,1,15,1			// [#2410] CA Justin Enable MoniMobile on Canadian Version
													// [#2411] US Justin 2016.04.01 Socket Error Timeout Bug Fix
													// [#2412] US Justin 2016.04.05 Support Additional Host Certification
													// [#2419] US Justin 2016.04.20 Assign Other transaction OP buttons dynamically
													// [#2423] US Justin 2016.05.05 En/Disable EMV for multiple ATMs
													// [#2428] US Justin 2016.05.23 Local Loan Improvement - Health Check After Local Loan / Deleting Employee / Employee Close
	//#define AP_DETAIL_VERSION	6,1,16,0			// [#2431] US Justin 2016.06.13 Change "Thank you for using our services" => "Thank You"
													// [#2432] US Justin 2016.06.15 Enable MoniMobile when NVRAM Cleared, Add MoniMobile in Print All Setup
	//#define AP_DETAIL_VERSION	6,1,17,0			// [#2435] Supporting EMV Kernel V6.0
													// [#2438] Change Transaction Category for Kernel V6.0 and higher versions (9F53 :  'R' (52) => 'Z'(5A) )
													// [#2439] CA Justin Enhancements on Local Cash Disbursement
													// [#2440] NH Justin 2016.08.10 Showing AID Selection even though one AID Left after removing Blocked AID 
	//#define AP_DETAIL_VERSION	6,1,18,0			// [#2451] US Justin 2016.11.04 New EMV Card reading Animation
													// [#2453] NH Justin NHS2.8 Status 
													// [#2454] NH Justin EJ UPLOAD BUG FIX (Transaction Audit/Network ID)
													// [#2456] NH Justin 2016.12.07 Change Enagled AID on PRINT ALL SETUP
													// [#2460] NH Justin 2016.12.14 Leave Dispense Result in Journals
													// [#2461] NH Justin 2016.12.15 Partial Update throught Moniview
													// [#2465] Enable Standard1 'w' field
													// [#2473] NH Justin 2017.02.08 Enable Status Monitoring for Cardtronics
	//#define AP_DETAIL_VERSION	6,1,19,0			// [#2477] CA Justin 2017.04.10 Leave EMV Journal
													// [#2478] NH Justin 2017.04.10 Print TSI, TVR, and TC on a receipt.
													// [#2479] CA Justin 2017.04.11 Support Spanish
													// [#2480] CA Justin 2017.04.12 Change default Kernel version number V4 => V6
	//#define AP_DETAIL_VERSION	6,1,19,1			// CHANGE VERSION (V06.01.19.0 was provided to TNS for certification)
													// [#2490] NH Justin 2017.06.14 Standard1 Dynamic Flow MAC Option
													// [#2491] US Justin 2017.06.28 Change Default AID List
													// [#2492] NH Justin 2017.06.30 Fixing "Out of Service without Error Code"
	//#define AP_DETAIL_VERSION	6,1,20,0			// [#2495] NH Justin 2017.08.03 Fixing Blank OP Menu(BillMix) Bug Fix (2700) - FIX OP Data ONLY
													// [#2500] CA Justin 2017.08.22 Moniview Journal Uploading
													// [#2504] NH Justin 2017.09.11 Add Test Dispense on ADD CASH SCREEN	
													// [#2506] CA Justin 2017.09.21 Add Local Employee Trial Close Out
													// [#2507] NH Justin 2017.09.26 Add Test Button on RMS Send Screen
													// [#2508] NH Justin 2017.09.27 Upload Master Key checksums to Moniview
													// [#2512] NH Justin 2017.10.24 Voice Guidance Bug Fix (AU, CA)
													// [#2514] US Justin 2017.11.09 Change Screen Text of CIBC DCC Offering
													// [#2519] NH Justin 2017.12.08 Leave Fallback Indicator in the journal
													// [#2520] NH Justin 2017.12.11 1500SE Remote Add Cash Bug Fix
													// [#2521] NH Justin 2017.12.11 MoniMobile : Add value added function enable status
	//#define AP_DETAIL_VERSION	6,1,21,0			// [#2529] NH Justin 2018.01.23 Support 2800SE
													// [#2534] NH Justin 2018.03.05 Change Default Network Setting, MODEM => TCPIP
													// [#2537] NH Justin 2018.03.06 Combine US,CA,MX AP data													
													// [#2538] NH Justin 2018.03.09 Adding MoniMobile Cash Balance Option
													// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
													// [#2546] NH Justin 2018.04.19 Supporting Moniview Setup Downloading through OP Menu (one stop staging)
													// [#2551] NH Justin 2018.05.24 Add 2800SE Machine Type in Standard1 Status Monitoring Field
													// [#2552] NH Justin 2018.05.29 2800 Image Bug Fix 1.OP Journal Image Vie(Showing previously viewed Image if Tr. images are missing) 2. Remove Cancelled PIN Entered Image
													// [#2555] NH Justin 2018.06.07 Adjust Extended Standard1 OP Setting Screen, Preview Enhanced Coupon
													// [#2556] NH Justin 2018.06.11 Test Printing Coupon (OP Menu)
													// [#2558] NH Justin 2018.06.12 Add Scheduled Reboot Option / Send RMS Status after exiting OP (US/CA/MX)
													// [#2559] NH Justin 2018.06.18 Rebooting on Downloading Setup or reboot command 
													//		Retry Status Send if "send" failed / Correct Unique ATM Types when a main board is swapped
													// [#2561] NH Justin 2018.07.11 Enable Camera Features on CA and MX Mode APs
													// [#2562] NH Justin 2018.07.12 Reduce PAUSE INTERVAL Between PRINT and CUT
	//#define AP_DETAIL_VERSION	6,1,22,0			// [#2571] CA Justin 2018.08.03 ATM Type Setting Error for 1024*600 Type ATMs (V06.01.21 Only)
													// [#2572] CA Justin 2018.08.07 Fixing Journal Cutting Issue (View Journal => PRINT THIS) (V06.01.21 Only)
	//#define AP_DETAIL_VERSION	6,1,23,0			// [#J008] US Justin 2018.12.12 Bug Fix : RMS Connection for Fallback Transaction
													// [#J005] US Justin 2018.11.17 Adding Comm Types (MODEM/TCPIP) in Standard1 Status Monitoring
	//#define AP_DETAIL_VERSION	96,1,24,0			// [#RWC6-22] EMV ADVT v7.0 Test 
	//#define AP_DETAIL_VERSION	6,1,24,0			// [#RWC6-9] Visa DCC
	//#define AP_DETAIL_VERSION	6,1,25,0			// [#GLDV-2505] US Kook 2019.05.13 Support MX-2800T
													// [#GLDV-2529] Addressing WINCE vulnerability. (RMS buffer overflow, XFS DLL External Socket Listening)
	//#define AP_DETAIL_VERSION	6,1,25,1			// [#RWC6-12] Denomination Selection Support
	//#define	AP_DETAIL_VERSION	6,1,27,0			// [#RWC6-60] Remove sensitive data in NVRAM after TXN
													// [#RWC6-27] Configurable Dispense Methods
													// [#RWC6-29] Allpoint Button
													// [#RWC6-30] 2800SE text outlines
													// [#RWC6-89] Remove delay in ReloadLanCard()
													// [#RWC6-20] PCI 5.0 EPP in NHA
													// [#RWC6-37/48] Disk Cleanup
													// [#RWC6-70] Credit Dual Balance Options
													// [#RWC6-80] Disable fast cash options
													// [#RWC6-83] PIA Default Scheduled reboot
													// [#RWC6-88] Vault door error code blank on boot fix
													// [#RWC6-91] B4U and LTX status monitoring fields in STD1 
													// [#RWC6-72] US William 2019.12.09 PAI INI file
													// [#RWC6-115] B4U: API Changes
													// [#RWC6-116] B4U Registration QR
													// [#RWC6-118] Remove JustCash BTC Purchase from Cardless
	//#define	AP_DETAIL_VERSION	6,1,28,0		// [#RWC6-129] LTX STD3 Fixes
													// [#RWC6-131] 1500 Installer Issue
													// [#RWC6-128] B4U fixes
													// [#RWC6-130] Welcome screen copy positioning
	//#define	AP_DETAIL_VERSION	6,2,0,0			// [#RWC6-73] Remove US NFC
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
	//#define	AP_DETAIL_VERSION	6,2,1,0			// [#RWC6-154] Fix for PAS buffer overrun 
													// [$RWC6-155] Remove GivePayConfig.xml from normal build
	//#define	AP_DETAIL_VERSION	6,2,1,1			// [#RWC6-156] fIX CRASH ON STATIC ip pas
													// [#RWC6-123] Transaction Language selection is always english bug
													// [#RWC6-135] PAI credit additional surcharge
													// [#RWC6-158] JustCash QR receipt bug fix
													// [#RWC6-126] Increase anti-skimming timeout
													// [#RWC6-141] Change default EMV type on CA
													// [#RWC6-161] Update EPP EP/SP to prevent comm issues during CPU resets
													// [#RWC6-163] Allow LTX to use dual host keys if dual host is disabled on EP versions < V10
													// [#RWC6-164] Fix LTX welcome screen copy
													// [#RWC6-165] Fix LTX receipt copy
													// [#RWC6-132] Health Check after B4U Transaction
	//#define	AP_DETAIL_VERSION	6,2,1,2				
													// [#RWC6-162] Add B4U to Canada Release	
													// [#RWC6-132] Health Check after B4U Transaction
													// [#RWC6-117] Update JSON parsing/serialization library
													// [#RWC6-127] Add disk utilization to RMS setup message
													// [#RWC6-132] Add optional health check after B4U transaction
													// [#RWC6-134] Add support for RMS remote update job
													// [#RWC6-170] Add setup data for remote update configuration
													// [#RWC6-149] Display remote update config on PAS
													// [#RWC6-169] PAI: Credit segmentation with RMS
													// [#RWC6-162] Enable B4U in Canada
													// [#RWC6-166] Don't go out of service if GivePay is down
	//#define	AP_DETAIL_VERSION	6,2,2,0			// [#RWC6-167] LibertyX API changes  

													// [#RWC6-209] RLS over TLS fix
													// [#RWC6-142] Checge HTTP to use non-blocking IO
													// [#RWC6-168] Add anti-skimming 2B field for RMS
													// [#RWC6-178] Add MoniView setup for STD3 comm header
													// [#RWC6-181] Add HTTP basic auth for software repo config
													// [#RWC6-188] Log journal entry for B4U
													// [#RWC6-190] Update.exe failure on VS2008
													// [#RWC6-192] LTX flow modifications
													// [#RWC6-195] EPP EP <= 8.0 fails for txn continue (remove PAI second PIN entry)
													// [#RWC6-196] EMV kernel V7 update for EMV continue txn 
													// [#RWC6-197] CATM VG for DCC error codes
													// [#RWC6-198] CATM dual balance VG
													// [#RWC6-199] CATM Visa DCC offering screen button issue
													// [#RWC6-201] VG surcharge acceptance "7 to accept" 
													// [#RWC6-203] CATM exchange rate not showing for Visa DCC
													// [#RWC6-205] EPP EP update V15.00.01.07
													// [#RWC6-210] Receipt not ejecting on STD1 DCC
													// [#RWC6-191] Update STD3 tsm fields (B4U, libertyx, credit segmentation)
													// [#RWC6-204] Cardtronics EMV TDL Lookup error handling
													// [#RWC6-206] B4U receipt incorrect surcharge
													// [#RWC6-207] LibertyX MoniView setup data
													// [#RWC6-208] LibertyX MoniView password change job
													// [#RWC6-212] Add last 6 hash for CoinSquare purchase route
	//#define	AP_DETAIL_VERSION	6,2,3,0			// [#RWC6-184] F000B ECode re-wording 
	
													// [#RWC6-216] Separate LibertyX sequence numbers
													// [#RWC6-213] Dispense style INI
													// [#RWC6-214] Dispense style MoniView
													// [#RWC6-226] Default EMV Kernel V7

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
	//#define	AP_DETAIL_VERSION	6,2,5,4

													// [#RWC6-235] - CATM VISA DCC receipt verbiage
													// [#RWC6-247] - EJUpload 'J' FID not initiating EJUP transaction
													// [#RWC6-245] - Bansi DCC Customizations
													// [#RWC6-242] - Add DF to Mexico Version
													// [#RWC6-243] - LibertyX Voice Guidance
													// [#RWC6-246] - Downgrade TLS ciphers for MoniView
													// [#RWC6-236] - Add LibertyX KCV to PrintAllSetup
													// [#RWC6-239] - CATM - French VISA DCC Receipt Message
													// [#RWC6-240] - CATM - MasterCard DCC Partial Reversal Missing Fields
													// [#RWC6-244] - Add Btye2.b1 for DCC DF enablement
													// [#RWC6-251] - MCCP client auth error
													// [#RWC6-252] - LTX PAS translation error
	//#define	AP_DETAIL_VERSION	6,2,6,0
													// [#RWC6-251] - MCCP client auth error
													// [#RWC6-252] - LTX PAS translation error
	//#define	AP_DETAIL_VERSION	6,2,6,4
													// [#RWC6-282] US William 2021.05.03 Update to EPP EP v15.0.3 and SP 6.4.15
	//#define	AP_DETAIL_VERSION	6,2,6,5
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
													// [#RWC6-282] US William 2021.05.03 Update to EPP EP v15.0.3 and SP 6.4.15
	//#define	AP_DETAIL_VERSION 6,2,7,1
													// [#RWC6-286] HQ Kook 2021.05.13 Prevent Canada from crashing after NVRAM
	// #define	AP_DETAIL_VERSION 6,2,7,2

													// [@RWC6-288] US William 2021.06.02 EPP "ding" 10 minute fix
	// #define	AP_DETAIL_VERSION 6,2,7,3

													// Fix Bitload4U Canadian Dollar glitch	RWC6-249
													// Add CATM root certificate	RWC6-277
													// Support dual-language receipts	RWC6-290
													// Display "no withdrawal" warning instead of going out of service when ATM is out of cash	RWC6-259
													// Fix Cash Totals MoniMobile QR code buffer issues	RWC6-298
													// Update.exe needs power failure on slow network connections	RWC6-303
	// #define	AP_DETAIL_VERSION 6,2,8,0

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


//#define RMS_VERSION		0x010300				// [#286] [NH] KSK 2008.6.12
//#define RMS_VERSION		0x010307				// [#286] [NH] KSK 2008.6.12
//#define RMS_VERSION		0x010309
//#define RMS_VERSION		0x010312				// [#558] KSK 2009.08.17
//#define RMS_VERSION		0x010318				// [#558] KSK 2009.08.17
//#define RMS_VERSION		0x010319				// [#2222] Justin 2013.10.03 RMS Agent Version Up Dynamic Flow, OP language
//#define RMS_VERSION		0x010320				// [#2242] Justin 2013.10.03 Dynamic Flow 1st Call Option
//#define RMS_VERSION		0x010321				// [#2276] Justin 2014.06.09 Surcharge Table and Password Change
//#define RMS_VERSION		0x010322				// [#2282] Justin Support Customized Hot Key
//#define RMS_VERSION		0x010324				// [#2316] Justin Support Decimal Point Surcharge
//#define RMS_VERSION		0x010325				// [#2320] Justin NEW SSL/TLS VERSION MANAGEMENT (ABOVE => UP TO)
//#define RMS_VERSION		0x010326				// [#2342] NH Justin 2015.05.11 AID SELECTION
//#define RMS_VERSION		0x010327				// [#2359] NH Justin 2015.07.15 Add Moniview Setup (Percentage Surcharge Format)
//#define RMS_VERSION		0x010328				// [#2410] NH Justin Support MoniMobile En/Disable,  Support En/Disable EMV through individual command
//#define RMS_VERSION		0x010329				// [#2435] NH Justin Add Kernel version info in Setup, MV will check this version for allowing Kernel version change
//#define RMS_VERSION		0x010330				// [#2442] Enable Digital Donation (US ONLY)
//#define RMS_VERSION		0x010331				// [#2461] Support Partial Update, [#2475] US Justin Add Host Certificate En/Disable
//#define RMS_VERSION		0x010332				// [#2508] NH Justin 2017.09.27 Upload Master Key checksums to Moniview
//#define RMS_VERSION		0x010335				// [#2538] NH Justin 2018.03.09 Adding MoniMobile Cash Balance Option
//#define RMS_VERSION		0x010343				// [#RWC6-119] US William 2020.01.16 Add JustCash RMS changes / [#RWC6-56] US William 2020.02.07 Hashed Passwords
//#define RMS_VERSION		0x010345				// [#RWC6-134] US William 2020.03.24 Add RMS support for remote updates
//#define RMS_VERSION		0x010347				// [#RWC6-207] US William 2020.08.05 Add LibertyX enablement to setup data
//#define RMS_VERSION		0x010348				// [#RWC6-214] US William 2020.09.01 Add CDU Dispense Style				
//#define RMS_VERSION		0x010349				// [#RWC6-224] US William 2020.10.21 LibertyX Dispense			
//#define RMS_VERSION		0x010350				// [#RWC6-257] US William 2021.02.25 Add LibertyX Username to setup data
//#define RMS_VERSION		0x010351				// [#GLDV-3109] AU Kook 2022.01.25 Support Model Name, CDU Binding, DCC Local Surcharge
#define RMS_VERSION		0x010353				// [#RWC6-533] US ryan.payton 2023.05.26 [PAI] RMS MCCP Key Reset

//#define MWI_VERSION		_T("V06.03.00")
#define MWI_VERSION			_T("V06.03")				// [#2321] NH KSK 2015.01.14 AP Revision 처리를 위해 Version을 2개만 Set함


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
// Support Software TTS									[#2368] US KSK 2015.08.31 Support Software TTS
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

// ----------------------------------------------------------------------------
//	Cash Condition Amount Define					[#273] [NH] KSK 2008.6.9 Basedef.h로 이동
// ----------------------------------------------------------------------------
//#define CDU_MCASH_MAXOUT	800					// MAX OUT : $800
//#define CDU_MCASH_MAXOUT	4000				// MAX OUT : $4000	미국과 동일한 Dispense Limit값으로 변경
#define CDU_MCASH_MAXOUT	5000				// MAX OUT : $5000				// [#2407] CA Justin 2016.03.22 Increase Maximum Dispensible Count for all US Versions (Prev : $4000)

#define CDU_MCASH_MINOUT	5					// MIN OUT : $5
#define CDU_CST_MAXFASTCASH	6					// MAXIMUM FAST CASH COUNT
#define	CDU_MAX_CST_COUNT	4					// [#281] [NH] JSW 2008.6.10

//#define CST_MAXDISPENSE	40					// MAXIMUM DISPENSE COUNT
#define CST_MAXDISPENSE		50					// MAXIMUM DISPENSE COUNT		// [#2407] CA Justin 2016.03.22 Increase Maximum Dispensible Count for all US Versions (Prev : 40)
#define CST_W_MAXDISPENSE	20					// [#2098] US KSK 2011.11.28
#define CDU_NEARCASHCNT		100					// NEAR CASH COUNT	
#define CDU_FULLREJECTCNT	200					// FULL REJECT COUNT

// ----------------------------------------------------------------------------
//	USA AVAILABLE DENOMINATION					[#282] [NH] KSK 2008.6.11 국가별 권종 DEFINE 추가 및 MWI 적용
// ----------------------------------------------------------------------------
#define CASH_DENOMINATION1	5					// $5
#define CASH_DENOMINATION2	10					// $10
#define CASH_DENOMINATION3	20					// $20
#define CASH_DENOMINATION4	50					// $50
#define	CASH_DENOMINATION5	100					// $100
#define	CASH_DENOMINATION6	0					// $0
#define	CASH_DENOMINATION7	0					// $0
#define	CASH_DENOMINATION8	0					// $0
// end of [#282]

// ----------------------------------------------------------------------------
// SCREEN TIMEOUT [#236] AIREAT 2008.05.28
// ----------------------------------------------------------------------------
#define NORMAL_SCR_TIMEOUT		30		// OP 화면 타임아웃.
#define INFO_SCR_TIMEOUT		5		// 경고시 안내문구 타임아웃.
#define ABORT_SCR_TIMEOUT		3		// 최소시 안내문구 타임아웃.

// ----------------------------------------------------------------------------
// NVRAM Default Value.
// ----------------------------------------------------------------------------
#define TERMINAL_ID_DEFAULT			L"00000000"	// [#398] [NH] KSK 2008.8.6
#define	HOSTPHONE1_DEFAULT			L"0"		// [#426] [NH] KSK 2008.9.24
#define	HOSTPHONE2_DEFAULT			L"0"		// [#426] [NH] KSK 2008.9.24
#define	MASTERPW_DEFAULT			L"1:0:$2$gYTcUUhRepZGnhPO$Il8D1xfPAt1/gQ7gfrJHh2XfMluv0a1Lu4Rp85xP51GgzUtXnEfibAgiXHOHKyPDXPDFJLH6rgd8YitS5AYnAA==:0"			// [#419] [NH] KSK 2008.9.18
#define	SERVICEPW_DEFALUT			L"1:2:$2$4a9nICkgVr6GbXaq$1Di8tbKZD9zKy7AZAQ4JK+qnhXBcaq1cwsXOzIEiWVOtc+bTYS1Hy3wqAFpcl+tcQzPH9sQpeJBjLS/K2PfgAA==:0"			// [#419] [NH] KSK 2008.9.18
#define	OPERATORPW_DEFALUT			L"1:1:$2$cWZZLZXEyKMU2IsV$nQbceOXgKpxNv7gbglMQyqTXGuIlgQy6xCFFqUDryNMCpMEJc9trS9Sb4X4eSYEBwUOJY0jrBgjlvq3LwN+AAA==:0"			// [#419] [NH] KSK 2008.9.18

#define	DISPENSELIMIT_DEFAULT		L"040000"	// POS 2008.8월 Default Limit값 변경 (200->400)
#define	FASTCASH1_DEFAULT			20
#define	FASTCASH2_DEFAULT			40
#define	FASTCASH3_DEFAULT			60
#define	FASTCASH4_DEFAULT			80
#define	FASTCASH5_DEFAULT			100
#define	FASTCASH6_DEFAULT			200
#define	SURCHAREAMOUNT_DEFAULT		L"00000150"
#define	SURCHARGEOWNER_DEFAULT		L"OWNER"

#define	RECEIPT_HEADER1_DEFAULT		L""
#define	RECEIPT_HEADER2_DEFAULT		L""
#define	RECEIPT_HEADER3_DEFAULT		L""

#define ADV_GUIDEDISP_TIME_DEFAULT	5
#define	REVERSAL_RETRY_DEFAULT		2

// ----------------------------------------------------------------------------
// EMV and other options
// ----------------------------------------------------------------------------
#define	EMV_LEVEL2						1			// EMV Level2 지원	[#559] CA KSK 2009.08.19
#define	EMV_TEST_MODE					0			// [#2115] NH KSK 2011.10.24 EMV 거래 시 최종 1거래를 FILE에 저장하도록 기능 추가	
#define SUPPORT_CHANGE_PARAMETER_JNL	0			//[#610] SOOK 2009.12.21 Configuration Change Journal 저장 

// ----------------------------------------------------------------------------
// OP TIMEOUT시 전환 화면 DEFINE
// ----------------------------------------------------------------------------
#define OP_TIMEOUT_SCREEN_NUMBER		899

// ----------------------------------------------------------------------------
// OP TIMEOUT시 전환 화면 DEFINE					// [#540] HWANG 2008.07.01
// ----------------------------------------------------------------------------
#define DIAG_TIMEOUT_SCREEN_NUMBER		829			

#endif // _BASE_DEF_H_
