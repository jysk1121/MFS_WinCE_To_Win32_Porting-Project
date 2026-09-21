/////////////////////////////////////////////////////////////////////////////
//	DevApl.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "shlwapi.h"

//#define NH_DEBUG

#include ".\Common\CmnLib.h"
#include ".\Common\NHDbgApi.h"
#include ".\Common\ConstDef.h"
#include ".\Common\LibertyXConfigurationManager.h"

#include ".\Dev\DevDefine.h"

#include ".\Net\Network.h"

#include ".\Dev\DevCmn.h"
//#include ".\Common\MB2500DLL.h"			// [#573] NH AIREAT 2009.10.26 SW NVRAM
#include ".\Dll\NHReadiniFile.h"

#include ".\Common\ScreenDef.h"		// [#257] [MX] KSK 2008.6.5
#include ".\Scr\ScrCtrl.h"			// [#257] [MX] KSK 2008.6.10

#include <openssl/rand.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// Initialize DS(Variable)
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_InitializeDS()
{
	NHDEBUG(1, (_T("***DevApl***CDevCmn::fnAPL_InitializeDS() \n")));

	// [RWC6-676] SKKim 2024.05.21 추후 Registry 설정부분은 한곳으로 통일하도록 보완 필요(InstallNextwareRegistries() 함수로)
//////////////////////////////////////////////////////////////////////////
//  Reg Setting
//////////////////////////////////////////////////////////////////////////

	// [#RWC6-106] NH Kook 2020.01.16 Fast boot
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\SPR", L"Initialize", 0);		// 4 seconds
// 	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"Initialize", 0);		// 16 seconds but we'll keep CDU initialization for ATM stability.
	// end of [#RWC6-106]

	//////////////////////////////////////////////////////////////////////
	// SPR
	//////////////////////////////////////////////////////////////////////
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\SPR", L"LowSensor", L"1");
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\SPR", L"MBSupport", 1);
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\SPR", L"TraceLevel", 0);

	//[#2371] US Justin Add 4000W full/Paritial Cut
	if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_4000W_RECEIPT_FULLCUT)==1)
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\SPR", L"PartialCutAsCut", 0);
	else
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\SPR", L"PartialCutAsCut", 1);
	// End of [#2371]

	// [#2295] NH Justin 2014.10.08 Download SPR EP
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\SPR", L"EPDownloadOption", 1);
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\SPR", L"EPDownloadRetryCount", 2);		// [#2573] US Justin 2018.08.09 Increase EP download retry count 1 => 2 (SPR, MCU)
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\SPR", L"EPDownloadDir", L"\\ATM\\");
	// End of [#2295]

	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\SPR", L"ColumnSet", 2);					// [#2298] US Justin 2014.10.24 Column Set, 1=36, 2=40, 3=48 columns

	// [#2244] AU KMK 2013.12.19 SPR Device Status HWERROR 유지옵션은 호주에서만 활성화
#if (AU_VERSION)
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\SPR", L"ResetOnRecovery", L"1");	
#else
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\SPR", L"ResetOnRecovery", L"0");	
#endif
	// end of [#2244]

	// [#2421] NH KSK 2016.04.29 SPR Nextware Trace Log NVRAM에 저장하도록 Registry 추가
	RegSetInt(L"SOFTWARE\\Nextware", L"EnableLogNvramUnitID", 0xF0000001);	
	// end of [#2421]

	//////////////////////////////////////////////////////////////////////
	// CDU
	//////////////////////////////////////////////////////////////////////
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\CashDispenser", L"ConvPCU2LCU", L"0");

	// KSK 2011.06.13 Registry값을 적용하도록 국가 공통 적용 (basedef.h별로 매수정보가 국가별로 나뉘어져 있음)
	// [#2098] US KSK 2011.11.28 WallMount인 경우에는 최대 방출매수 20매로 제한함
#if (US_VERSION||CA_VERSION)				// [#2187] CA Justin 2013.04.18 Add 4000W
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")	// SP Open전이므로, 기종으로 구분할 수 밖에 없음 (푸른기술 CDU인 경우 Registry를 안보면 필요없는 로직임)
		RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"MaxBills", Int2Asc(CST_W_MAXDISPENSE));
	else
		RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"MaxBills", Int2Asc(CST_MAXDISPENSE));//[#542] SOOK 2009.07.01 인증 중 변경 
#else
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"MaxBills", Int2Asc(CST_MAXDISPENSE));//[#542] SOOK 2009.07.01 인증 중 변경 
#endif
	// end of [#2098]

	// [#2386] NH KSK 2016.01.12 CDU CE부 인증 사용 여부 Registry Setting
#if (AU_VERSION)
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDU_BINDING, 0);
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"C3Support", 0);
#else
	// [#2392] US Justin Add CDU Binding OP Option
	//RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"C3Support", 1);
	int nCDUBOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDU_BINDING);
	if( nCDUBOption != 0 )		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"C3Support", 1);
	else						RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"C3Support", 0);
	// End of [#2392]
#endif
	// end of [#2386]

	// [#2287] NH Justin 2014.07.28 Enable CDU Sensor for all countries
/*	// [#2238] AU KMK 2013.12.09 CDU Sensor Log. 호주에서만 활성화
#if (AU_VERSION)
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"SensorLogLevel", L"1");
#else
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"SensorLogLevel", L"0");
#endif
	// end of [#2238]
*/
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"SensorLogLevel", L"1");
	// End of [#2287]

	// [#2270] AU KSK 2014.05.30
	// [#2241] AU KMK 2013.12.17 CDU-M 논리매수 차감 Option, 호주 외 타국가는 기본값 ENABLE
	// Note Counting 항목이 NVRAM과 레지스트리에 쓰는 값의 의미가 각각 다르므로 주의 필요
	// NVRAM		0 : ENABLE (NVRAM 초기값)
	//				1 : DISABLE
	// 레지스트리	0 : DISABLE
	//				1 : ENABLE
#if (AU_VERSION)
	#if (AU_A_VERSION)	// [#2323] AU KSK 2015.01.15 NH Equipments 요청사항
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
		RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"NoteCounting", L"1");
	else
		RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"NoteCounting", L"0");
	#else
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE, 0);
		RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"NoteCounting", L"1");
	#endif				// end of [#2323]
#else
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"NoteCounting", L"1");
#endif
	// end of [#2241]
	// end of [#2270]


	// 추후 Registry 설정 변경시 검증 필요함
	//	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\CDM", L"BillsMinimum", Int2Asc(CDU_NEARCASHCNT));
	//	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\CDM", L"RejectMaximum", Int2Asc(CDU_FULLREJECTCNT));

	// [#2095] NH KSK 2011.10.27 SHOW MODE인 경우에는 CDU SP를 비 암호화로 변환함 (보안 강화)
#ifdef	APP_LOCAL_MODE
	#ifdef CDU_DECRYPT_MODE
	DWORD dwTrue = ConvEncryMode2Value(FALSE);		// 비암호화 설정	 
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"SetEcryptionMode", dwTrue);
	#endif
#endif
	// end of [#2095]

	// [#2154] NH KSK 2012.10.09 CDU Missfeed Option 변경
	// 1. 동일 권종일 경우 다른 CST에서 Retry하도록 수정
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"MisfeedRetry", L"1");
	// 2. Missfeed 발생 시 카세트 축퇴 (INOP)
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"INOPRETRY", L"0");
	// end of [#2154]

	//////////////////////////////////////////////////////////////////////
	// MCU
	//////////////////////////////////////////////////////////////////////
	// [#23] NH PSC 로직 위치 변경
	// '1': DevType, '3': Magtek, '4' : Sankyo
	// MCU는 무조건 PORT1을 사용"COM1"하므로 AP에서 별도 설정 바꾸지 않음
	if(GetDeviceType(L"MCU") == MCU_MAGTEK_DIP)	// [#25] NH PSC 
	{
		// [#605] NH KSK 2010.01.06 EMV Latch Option 적용
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"LatchOption", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_LATCH_ENABLE));	// Latch 사용함
		// end of [#605]

		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"ParityBits", 2);
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"BaudRate", 38400);	// Magtek : 38400, DipType : 9600
		RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\DIPIDC", L"exec", L"\\ATM\\NHIDC4MTKWCE.exe");

		// [#2026] NH KJW 2011.02.28
		RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"EPDownloadDir", L"\\ATM\\");	// F/W bin 경로
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"EPDownloadRetryCount", 2);		// Retry 횟수		// [#2573] US Justin 2018.08.09 Increase EP download retry count 1 => 2 (SPR, MCU)
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"EPDownloadOption", 1);			// Download 수행 여부
		// end of [#2026]
	}
	else if(GetDeviceType(L"MCU") == MCU_SANKYO_DIP)		// [#2073] NH KSK 2011.06.24
	{
		// [#2458] AU KSK 2016.12.08 Latch Option 추가
		#if (AU_A_VERSION)
		// Sankyo Latch Option FW 지원됨
		// ICM300-3R1372 : 2420-03D (일반)
		// ICM300-3R0775 : 4707-02E (고주파 대응)
		// ICM30A-3R1372 : 3898-02C (AntiSkimming)
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"LatchOption", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_LATCH_ENABLE)); // EP VERSION CHECK??
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"StatusOKIfCardNotPresent", 1);			// [#2544] AU HYEJIN AHN 2018.04.18 LatchOption 미 사용 시, Chip 거래 중 카드 제거할 경우 Device 상태 기존 상태로 유지(옵션 처리)
		#else
		// Sankyo는 Latch Option이 없으므로 무조건 Latch Enable 해야함
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"LatchOption", 0);	// Latch 사용함
		#endif	
		// end of [#2458]

		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"ParityBits", 0);	// Sankyo에서는 ParitiBits 사용 안함
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"BaudRate", 38400);
		RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\DIPIDC", L"exec", L"\\ATM\\NHDIP4SANDIPCE.exe");

		// [#2074] Anti Skimming 사용 유무 설정
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_ENABLE) == 0)
			RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"ESUSupportOption", 0);
		else
		{
			// Anti Skimming Enable일 경우 SP사양때문에 AP에서 분기 처리해야함 (Shutter 제어)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_SHUTTER_ENABLE) == ENABLE)
				RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"ESUSupportOption", 2);
			else
				RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"ESUSupportOption", 1);
		}

		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"ESUTimeThreshold", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_TIMETHRESHOLD));	// 단위 sec
		// end of [#2074]		

		// EP Download 사양은 동일한지 문의 필요
		RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"EPDownloadDir", L"\\ATM\\");	// F/W bin 경로
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"EPDownloadRetryCount", 2);		// Retry 횟수		// [#2573] US Justin 2018.08.09 Increase EP download retry count 1 => 2 (SPR, MCU)
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"EPDownloadOption", 1);			// Download 수행 여부
	}										// end of [#2073]
	else
	{
		// 보완 처리
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"LatchOption", 1);	// Latch 사용 안함

		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"MachineType", 1);	// [#2004] NH KSK 2010.11.15 Ding.wav를 출력하기 위해 Registry 설정
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"ParityBits", 0);
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"BaudRate", 9600);	// Magtek : 38400, DipType : 9600
		RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\DIPIDC", L"exec", L"\\ATM\\NHDIP4WCE.exe");
	}

	//////////////////////////////////////////////////////////////////////
	// PIN
	//////////////////////////////////////////////////////////////////////
	// MAC 관련 Registry 설정
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"UseNDCMAC", 1);
	
	// RKT를 위한 Registry 설정 (SP연동시 필요함)
	RegSetStr(L"SOFTWARE\\Nextware\\PIN", L"NoKeyCheck", L"1");

	// [#2080] NH KSK 2011.07.13
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_RANDOMNUMBER_ENABLE) == ENABLE)
//		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"RKT_RandomNumber", 3);
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"RKT_RandomNumber", 1);		// [#2259] NH KSK 2014.03.13 SP에서 알아서 호환해줌
	else
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"RKT_RandomNumber", 0);
	// end of [#2080]

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_PROTOCOL) == RKT_PROTOCOL_TR34)
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"RKT_RandomNumberLength", 16 | 0x80);			// [#RWC6-151] To retrieve RTepp which is asn.1 formatted
	else
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"RKT_RandomNumberLength", 16);

	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"KEKType", 1);
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"KEKTypeMethod", 0);
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"EPDownloadOption", 1);
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"EPDownloadRetryCount", 2);

	//////////////////////////////////////////////////////////////////////
	// KEYMGR
	//////////////////////////////////////////////////////////////////////
	// KeyMgr TDES/TMAC(KeyMode: 11) 사양
	// TDES, TMAC 사용시(KeyMode: 11) DES Master Key와 MAC Master Key의 공유 방식 지정.
	// 1 : TDES Master Key와 TMAC Master Key를 공유하지 않고 별도의 Key를 사용하는 방식.(캐나다)
	// 0 : 1 이외의 값일경우나 Registry 항목이 없을경우, TDES 와 TMAC Master Key를 공유하는 방식사용.(뉴질랜드, 호주)
	// 현재 뉴질랜드와 호주는 TDES, TMAC Master Key를 공유, 캐나다는 TDES, TMAC Master Key를 공유하지 않고 별도의 Key 사용.
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"TMACOption", 0);	// [#333] NH PSC 2008.06.22 KeyMode 11 호주 TMAC 지원.
#else
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"TMACOption", 1);	// [#333] NH PSC 2008.06.22 KeyMode 11 캐나다 TMAC 지원.
#endif

	// KeyMgr Registry Option 설정 (512K 사용시 필요)

	// [#2214] US Justin 2013.08.01 Change PIN Key Name
	RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"SPINMKButtonName", L"");							// Disable Single PIN Key Button
	RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"TPINMKButtonName", TRIPLEDESKEY_2ND_PIN_NAME);	//"2ND_MASTERK3"
	RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"SDATAMKButtonName", L"");						// Disable Single Data Key Button [#2322] US Justin 2015.01.13
	RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"TDATAMKButtonName", TRIPLEDATAKEY_NAME);	//"3RD_MASTERK3"	// [#2322] US Justin 2015.01.13 Triple Master Key
	// End of [#2214]

	// AP에서 사용하는 KEY NAME 정의 (AP에서 값을 변경하면 KeyMgr에서 이 값을 참조하여 KEY를주입함)
	RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"SingleKeyName",	S_TMK_BUTTON);
	RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"TripleKeyName",	T_TMK_BUTTON);

	RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"MACKeyName",		S_MACMK_BUTTON);
	RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"MACKeyName3",	T_MACMK_BUTTON);

	RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"PINKeyName",		S_PINMK_BUTTON);
	RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"PINKeyName3",	T_PINMK_BUTTON);		// "2ND_MASTERK3"  // [#2181] US KSK 2013.03.11 PIN¿ë Dual Master Key ÁÖÀÔÀ» À§ÇØ º¯°æ

	RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"DATAKeyName",	S_DATAMK_BUTTON);
	RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"DATAKeyName3",	T_DATAMK_BUTTON);		// "3RD_MASTERK3"	// [#2322] US Justin 2015.01.13 Triple Master Key

	// LibertyX Key name if the PIN Pad FW version is greater or equal to 10.x
#if (APP_LIBERTYX)
	CLibertyXConfigurationManager::ReconfigureKeyManagerRegistrySettings();
#endif

	// PIN, Data KEY Option
	// [#2499] US Justin 2017.08.21 Open Dual Host DCC for All US Customer
	/*
	// [#2181] US KSK 2013.03.11 PIN용 Dual Master Key 주입을 위해 변경
	// 1: USE, 0: Not Use
	#if (APP_CUSTOM_PAI)	
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"IsShowPINMKButton", 1);		// PCI V1.0  512K Dual Master : Use PIN Master Key 
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"EnterKeyMenuType", 1);		// [#2275] US Justin PCI V3.0 Dual Master Key Input
		//RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"IsShowDATAMKButton", 1);	// [#2322] US Justin 2015.01.13
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"IsShowDATAMKButton", 0);		// [#2322] US Justin 2015.01.13
	#else
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"IsShowPINMKButton", 0);		
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"EnterKeyMenuType", 0);		// [#2275] US Justin PCI V3.0 Dual Master Key Input
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"IsShowDATAMKButton", 0);		// [#2322] US Justin 2015.01.13
	#endif						
	// end of [#2181]
	*/
	#if (US_VERSION)
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"IsShowPINMKButton", 1);		// PCI V1.0  512K Dual Master : Use PIN Master Key 
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"EnterKeyMenuType", 1);		// [#2275] US Justin PCI V3.0 Dual Master Key Input
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"IsShowDATAMKButton", 0);		// [#2322] US Justin 2015.01.13
	#else
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"IsShowPINMKButton", 0);		
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"EnterKeyMenuType", 0);		// [#2275] US Justin PCI V3.0 Dual Master Key Input
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"IsShowDATAMKButton", 0);		// [#2322] US Justin 2015.01.13
	#endif
	// End of [#2499]

	//////////////////////////////////////////////////////////////////////
	// SENSOR
	//////////////////////////////////////////////////////////////////////
	// PROXIMITY 지원 설정
	//RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\SIU\\CAPS", L"CAP_SEN_PROXIMITY", L"YES");		// 사용안함 : NO, 사용함 : YES
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\SIU\\CAPS", L"CAP_SEN_PROXIMITY", L"NO");			// 사용안함 : NO, 사용함 : YES [###2]
	// [#2156] US KSK 2012.10.17 DHCP 설정관련 추가 (Expire Retry)
	#define COUNT_DHCP_RETRY_MAX     0xffff
//     RegSetInt(L"Comm\\CS8900A1\\Parms\\TcpIp", L"DhcpMaxRetry", 0xffffffff);        // Retry 무한으로수행
//     RegSetInt(L"Comm\\CS8900A1\\Parms\\TcpIp", L"DhcpRetryDialogue", 0xffffffff); // Retry 무한으로수행
	RegSetInt(L"Comm\\CS8900A1\\Parms\\TcpIp", L"DhcpMaxRetry", COUNT_DHCP_RETRY_MAX);
	RegSetInt(L"Comm\\CS8900A1\\Parms\\TcpIp", L"DhcpRetryDialogue", COUNT_DHCP_RETRY_MAX+1);
	// [#RWC6-61] US William 2019.10.29 D1500 Recovery
	RegSetInt(L"Comm\\CS8900A1\\Parms\\TcpIp", L"AutoCfg", 0x0);	// Disable auto configuration, i.e. 169.254.x.x address assignment
	// Reload card after setting these parameters
	ReloadLanCard();
	// End of [#RWC6-61] 

	// end of [#2156]

	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	// BCR Service Provider
	// HUSER
	#ifdef UNDER_CE		// [RWC6-676] Win32 debug시에는 기존 설정 유지하도록 처리함
	RegSetInt(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\ScannerPrinter", L"port", 0x00001f4d, 1);
	RegSetStr(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\ScannerPrinter", L"Provider", L"NHBCRCE", 1);
	RegSetStr(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\ScannerPrinter", L"Type", L"PTR", 1);
	RegSetStr(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\ScannerPrinter", L"Class", L"PTR", 1);

	// LOCAL MACHINE
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHBCRCE", L"Exec", L"\\ATM\\NW_BCR_CE.exe");
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHBCRCE", L"Vendor_Name", L"Hyosung");
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHBCRCE", L"DllName", L"BSSpi30.dll");
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHBCRCE", L"Version", L"3.00");
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHBCRCE", L"Class", L"PTR");

	// SP Information Setting
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\BARCODE", L"EPDownloadOption", 1);
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\BARCODE", L"EPDownloadRetryCount", 2);
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\BARCODE", L"ComPort", 0x00000008);

	// BNA Service Provider
	// HUSER
	RegSetInt(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\CashAcceptor", L"port", 0x00001f4e, 1);
	RegSetStr(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\CashAcceptor", L"Provider", L"NHCIMCE", 1);
	RegSetStr(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\CashAcceptor", L"Type", L"CIM", 1);
	RegSetStr(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\CashAcceptor", L"Class", L"CIM", 1);

	// LOCAL MACHINE
	if (GetDeviceType(L"BNA") == BNATYPE_MEI)
		RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHCIMCE", L"Exec", L"\\ATM\\NW_CIM_CE.exe");
	else
		RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHCIMCE", L"Exec", L"\\ATM\\NW_CIM_CE_JCM.exe");

	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHCIMCE", L"Vendor_Name", L"Hyosung");
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHCIMCE", L"DllName", L"BSSpi30.dll");
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHCIMCE", L"Version", L"3.00");
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHCIMCE", L"Class", L"CIM");

	// SP Information Setting
	RegSetStr(L"SOFTWARE\\ATM\\DEVINFO\\MEIBILL", L"Port", L"COM6:");
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\BNA", L"BarcodeHandling", 1);
	// end of [#GLDV-3005]

	// [#2325] NH KSK 2015.01.21 RFID 지원을 위한 Registry 설정
	// Service Provider
	// HUSER
	RegSetInt(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\RFIDReader", L"port", 0x00001f4b, 1);
	RegSetStr(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\RFIDReader", L"Provider", L"VIVOTECHRF", 1);
	RegSetStr(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\RFIDReader", L"Type", L"IDC", 1);
	RegSetStr(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\RFIDReader", L"Class", L"IDC", 1);

	// LOCAL MACHINE
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\VIVOTECHRF", L"Exec", L"\\ATM\\NHIDC4RFCE.exe");
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\VIVOTECHRF", L"Vendor_Name", L"Nautilus Hyosung");
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\VIVOTECHRF", L"DllName", L"BSSpi30.dll");
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\VIVOTECHRF", L"Version", L"3.00");
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\VIVOTECHRF", L"Class", L"IDC");

	// SP Information Setting
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"RequestOnlinAuthorization", 1);
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"TTQ", 0x34004000);				// VISA Invalid 대응
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"UseMagStripeData", 0);
#if (US_VERSION)
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"CountryCode", 0x00000840);
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"CurrencyCode", 0x00000840);
#elif (CA_VERSION)
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"CountryCode", 0x00000124);
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"CurrencyCode", 0x00000124);
#elif (MX_VERSION)
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"CountryCode", 0x00000484);
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"CurrencyCode", 0x00000484);
#elif (AU_VERSION)
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
	{
		// AUS
		RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"CountryCode", 0x00000036);
		RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"CurrencyCode", 0x00000036);
	}
	else
	{
		// NZL
		RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"CountryCode", 0x00000554);
		RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"CurrencyCode", 0x00000554);
	}
#else
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"CountryCode", 0x00000840);
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"CurrencyCode", 0x00000840);
#endif
	// end of [#2325]

	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"TraceLevel", 0);
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"CardPollTime", 3);	// VDM때문에 3초로 수정
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"Port", 0x00000007);	// [#RWC6-342] NH Kook 2021.11.18 Now WINCE60/WEC7 both uses COM7 for RFID
	RegSetInt(L"SOFTWARE\\ATM\\DEVINFO\\RFD", L"UseInsertedRemovedEvent", 0);
	#endif


//////////////////////////////////////////////////////////////////////////

	AtmDoing			= FALSE;								// Atm Doing
	AtmStatus			= ATM_INIT;								// Atm Status 
	AtmStatusSave		= ATM_INIT;								// Atm Status Save 
	TranStatus			= 0;								// Transaction Status 
	TranResult			= FALSE;								// Transaction Result

	DeviceDefine		= DEV_NONE;								// Defined Device Set
	DeviceNoDevice		= DEV_MAIN;								// Device No Device Set
	DeviceDownDevice	= DEV_MAIN;								// Device Down Device Set
	DeviceStatus		= DEV_NONE;								// Device Status Set
	DeviceAutoOff		= DEV_NONE;								// Device Auto Off Set

#if DEV_AUTO_OFF_SPR
	DeviceAutoOff		|= DEV_SPR;								// SPR Auto Off Set
#endif
	
	DeviceAutoOff		|= DEV_RFID;							// RFID Auto Off Set // [#2325] NH KSK 2015.01.26
	DeviceAutoOff		|= DEV_BCR;								// BCR Auto Off			[#GLDV-3005] US Kook 2022.01.03
	DeviceAutoOff		|= DEV_BNA;								// BNA Auto Off			[#GLDV-3005] US Kook 2022.01.03

//	DeviceTimeout		= DEV_NONE;								// Device Timeout Set	[#590] NH KSK 2009.12.07
	DeviceEvent			= DEV_NONE;								// Device Event Set

	DeviceOperationFlag	= FALSE;								// Device Operation Flag
	
	OpenKey				= TRUE;									// Open Key

	CSTCnt				= 0;									// CST Count
	FirstCSTValue		= 0;									// First CST Value
	SecondCSTValue		= 0;									// Second CST Value
	ThirdCSTValue		= 0;									// Third CST Value
	FourthCSTValue		= 0;									// Fourth CST Value
	FirstCSTSetCash		= 0;									// First CST Set Cash
	SecondCSTSetCash	= 0;									// Second CST Set Cash
	ThirdCSTSetCash		= 0;									// Third CST Set Cash
	FourthCSTSetCash	= 0;									// Fourth CST Set Cash
	RejectCSTCnt		= 0;									// Reject Cassette Count
	FirstCSTCnt			= 0;									// First CST Count(ClerkInformation)
	SecondCSTCnt		= 0;									// Second CST Count(ClerkInformation)
	ThirdCSTCnt			= 0;									// Third CST Count(ClerkInformation)
	FourthCSTCnt		= 0;									// Fourth CST Count(ClerkInformation)

	m_strCurrencyID.Empty();									// [#146] KSK 2008.04.20

	CduChangeLoc		= NO_CHANGE;							// Cdu Change Location
	CduLoc				= NORMAL_POS;							// Cdu Location
	RejectCSTLoc		= NORMAL_POS;							// Reject CST Location
	FirstCSTLoc			= NORMAL_POS;							// First CST Location(Cd)
	SecondCSTLoc		= NORMAL_POS;							// Second CST Location(Cd)
	ThirdCSTLoc			= NORMAL_POS;							// Third CST Location(Cd)
	FourthCSTLoc		= NORMAL_POS;							// Fourth CST Location(Cd)
	CSTLoc				= NORMAL_POS;							// All CST Location(Cd)			// [#51] NH PSC 2008.04.03

	SlipStatus			= SLIP_NORMAL;							// Slip Status

	RejectCSTStatus		= CST_NORMAL;							// Reject CST Status
	FirstCSTStatus		= CST_NORMAL;							// First CST Status
	SecondCSTStatus		= CST_NORMAL;							// Second CST Status
	ThirdCSTStatus		= CST_NORMAL;							// Third CST Status
	FourthCSTStatus		= CST_NORMAL;							// Fourth CST Status

	WithAvail			= TRAN_WITH_NOT;						// Withdraw Avail 
	HostOpenFlag		= FALSE;								// Host Open Flag

	// DeviceDefine Create Section
	DeviceDefine |= DEV_SPR;
	DeviceDefine |= DEV_MCU;
	DeviceDefine |= DEV_CDU;

	// [#2325] NH KSK 2015.01.20
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
		DeviceDefine |= DEV_RFID;
	// end of [#2325]

	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
	{
		fnCMN_AddActiveDevice(DEV_BCR);
		fnCMN_AddActiveDevice(DEV_BNA);
	}
	// end of [#GLDV-3005]

	// DeviceDefine Create(Miscellaneous) Section
	DeviceDefine |= DEV_SCR;									// SCREEN
	DeviceDefine |= DEV_DOR;									// DOOR
	DeviceDefine |= DEV_LGT;									// LIGHT
	DeviceDefine |= DEV_PIN;									// PIN : 2004.04.08
	DeviceDefine |= DEV_SNS;									// SENSOR : 2004.04.14

	nIsEmvChipPowerOn = 0;										// Initialize

	strJnlKindCode.Empty();										// [#591] NH KSK 2009.12.07

	// default language is ENGLISH
	m_nCurrentLangMode = ENG_MODE;

	m_oletimeStart = COleDateTime::GetCurrentTime();

	// [#2085] NH KSK 2011.07.22
	m_AntiSkiimingRecoveryTime = CTime::GetCurrentTime();
	bAntiSkimmingFlag = FALSE;
	// end of [#2085]

	nPINErrorFlag = WORKINGKEY_INIT;							// [#2134] NH KSK 2012.05.07

	m_nRemoteStatusCmd = 0;										// [#2277] NH Justin 2014.06.13 Remote ATM Status

#if (AU_VERSION)
	m_bDomesticFallbackTrans = FALSE;							// [#2379] AU KSK 2015.11.17
#endif
	m_bShownMultiAIDSelection = FALSE;							// [#2440] US Justin 2016.08.10 KSK 초기값 설정 추가

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Load Device
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_LoadDevice()
{
	//fnSCR_DisplayScreen(991);							// [#573] NH AIREAT iTM

	fnSCR_Initialize();
	fnDOR_Initialize();									// First Create
	fnLGT_Initialize();
	fnPIN_Initialize();
	fnSNS_Initialize();
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Unload Device
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_UnLoadDevice()
{
NHDEBUG(1, (_T("***DevApl***CDevCmn::fnAPL_UnLoadDevice() \n")));

	fnCDU_Deinitialize();
	fnMCU_Deinitialize();

	// [#2325] NH KSK 2015.01.25 RFID added
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
		fnRFID_Deinitialize();
	// end of [#2325]

#ifdef UNDER_CE
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE) == ENABLE)
		fnCAM_Deinitialize();
#endif // UNDER_CE

	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
	{
		fnBCR_Deinitialize();
		fnBNA_Deinitialize();
	}
	// end of [#GLDV-3005]

	fnSPR_Deinitialize();

	fnAPL_CheckDeviceAction(DEV_MAIN);

	fnSNS_Deinitialize();										// Last Delete
	fnPIN_Deinitialize();
	fnLGT_Deinitialize();
	fnDOR_Deinitialize();

	Delay_Msg(500);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Get Define Device
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnAPL_GetDefineDevice(int nDevId)
{
	return (DeviceDefine & nDevId);
}

/////////////////////////////////////////////////////////////////////////////
// Get Yes Device
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnAPL_GetYesDevice(int nDevId)
{
	return (fnAPL_GetDefineDevice(nDevId) & (~(fnAPL_GetNoDevice(nDevId))));
}

/////////////////////////////////////////////////////////////////////////////
// Get No Device
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnAPL_GetNoDevice(int nDevId)
{
	return (fnAPL_GetDefineDevice(nDevId) & (DeviceNoDevice & nDevId));
}

/////////////////////////////////////////////////////////////////////////////
// Get Down Device
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnAPL_GetDownDevice(int nDevId)
{
	return (fnAPL_GetDefineDevice(nDevId) & (DeviceDownDevice & nDevId));
}

/////////////////////////////////////////////////////////////////////////////
// Get Avail Device
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnAPL_GetAvailDevice(int nDevId)
{
	static int nTempAvail = 0;

	nDevId = fnAPL_GetDefineDevice(nDevId);


	if (nTempAvail != (nDevId & (~(fnAPL_GetErrorDevice(nDevId)))))
	{
		nTempAvail = nDevId & (~(fnAPL_GetErrorDevice(nDevId)));
	}

	return (nTempAvail);
}

/////////////////////////////////////////////////////////////////////////////
// Get Avail Error Device
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnAPL_GetAvailErrorDevice(int nDevId)
{
	nDevId = fnAPL_GetDefineDevice(nDevId);

	nDevId &= ~DeviceAutoOff;
	return (fnAPL_GetErrorDevice(nDevId));
}

/////////////////////////////////////////////////////////////////////////////
// Get Error Device
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnAPL_GetErrorDevice(int nDevId)
{
	static int DeviceStatusSave = 0;

	nDevId = fnAPL_GetDefineDevice(nDevId);
				
	if (DeviceStatus != DeviceStatusSave)
	{
		DeviceStatusSave = DeviceStatus;
		m_strNVLog.Format(L"1:ErrorDvc(%x)", DeviceStatus);
		NVDump('F', 'E', "05", L"", m_strNVLog);

		NHDEBUG(1, (_T("ErrorDvc:%S \n"), GET_DEVNAME_BY_ID(nDevId)));
	}

	return (DeviceStatus & nDevId);
}

/////////////////////////////////////////////////////////////////////////////
// Get Down Error Device
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnAPL_GetDownErrorDevice(int nDevId)
{
	NHTRACE((_T("***DevApl***CDevCmn::fnAPL_GetDownErrorDevice() nDevId:(%x)\n"), nDevId));

	nDevId = fnAPL_GetDefineDevice(nDevId);

	return (fnAPL_GetDownDevice(nDevId) | fnAPL_GetErrorDevice(nDevId));
}

/////////////////////////////////////////////////////////////////////////////
// Check Device Status Changed
/////////////////////////////////////////////////////////////////////////////
BOOL CDevCmn::fBAPL_IsDeviceStatusChanged(int nDevId)
{
	static	int		nSPRDeviceStatus	= 0;
	static	int		nSPRHeadStatus		= 0;
	static	int		nSPRPaperStatus		= 0;

	static	int		nCDUDeviceStatus	= 0;
	static	int		nCDUCstLoc			= 0;

	static	int		nRFIDDeviceStatus	= 0;	// [#2325] NH KSK 2015.01.30
	static	int		nBCRDeviceStatus	= 0;	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	static	int		nBNADeviceStatus	= 0;	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car

	int		nReturn = FALSE;

	if (nDevId & DEV_SPR)
	{
		if (SlipStatus != nSPRPaperStatus)
		{
			nSPRPaperStatus = SlipStatus;
			nReturn = TRUE;
		}

		if ((DeviceDownDevice & DEV_SPR) != nSPRDeviceStatus)
		{
			nSPRDeviceStatus = (DeviceDownDevice & DEV_SPR);
			nReturn = TRUE;
		}
	}

	if (nDevId & DEV_CDU)
	{
		if ((DeviceDownDevice & DEV_CDU) != nCDUDeviceStatus)
		{
			nCDUDeviceStatus = (DeviceDownDevice & DEV_CDU);
			nReturn = TRUE;
		}
	}

	// [#2325] NH KSK 2015.01.30 RFID 축퇴 처리
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
	{
		if (nDevId & DEV_RFID)
		{
			if ((DeviceDownDevice & DEV_RFID) != nRFIDDeviceStatus)
			{
				nRFIDDeviceStatus = (DeviceDownDevice & DEV_RFID);
				nReturn = TRUE;
			}
		}
	}
	// end of [#2325]

	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
	{
		if (nDevId & DEV_BCR)
		{
			if ((DeviceDownDevice & DEV_BCR) != nBCRDeviceStatus)
			{
				nBCRDeviceStatus = (DeviceDownDevice & DEV_BCR);
				nReturn = TRUE;
			}
		}
		if (nDevId & DEV_BNA)
		{
			if ((DeviceDownDevice & DEV_BNA) != nBNADeviceStatus)
			{
				nBNADeviceStatus = (DeviceDownDevice & DEV_BNA);
				nReturn = TRUE;
			}
		}
	}

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get Auto Off Bank Device
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_GetAutoOffDevice(int nDevId)
{
	nDevId = fnAPL_GetDefineDevice(nDevId);

	return (DeviceAutoOff & nDevId);
}


/////////////////////////////////////////////////////////////////////////////
// Get Avail Transaction
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_GetAvailTrans()
{
	WithAvail	= fnAPL_GetAvailWithdraw();

	return WithAvail;
}

/////////////////////////////////////////////////////////////////////////////
// Get Avail Withdraw
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_GetAvailWithdraw()
{
	int	nTempWithAvail = TRAN_WITH_NOT;

	if ((fnAPL_GetAvailDevice(DEV_CDU))	&& (CduLoc == NORMAL_POS))
		nTempWithAvail = fnCDU_GetAvailWithdraw();

	return nTempWithAvail;
}

/////////////////////////////////////////////////////////////////////////////
// Set Sensor Information
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_SetSensorInfo(BOOL bUpdateCSTCnt)
{
	int			nEventDevice = DEV_NONE;						// 2004.07.05
	int			nStatusDevice = DEV_NONE;
	int			nResultDevice = DEV_NONE;
	int			nTempRejectCstCnt[4] = {0,};
																
	nEventDevice = fnCMN_GetEventDevice(DEV_MAIN | DEV_DOR);	// 2005.04.19
	nStatusDevice = fnCMN_GetStatusDevice(DEV_MAIN | DEV_DOR);
	nResultDevice = DeviceEvent | nEventDevice | nStatusDevice;	// 2004.08.23
	DeviceEvent = DEV_NONE;										// 2004.08.23

	if (nResultDevice & DEV_CDU)
	{
		if(bUpdateCSTCnt)
		{
			// [#393] [NH] KSK 2008.8.5
			// REJECT 매수 보정처리
			RejectCSTCnt = 0;
			for(int i=0; i<CSTCnt; i++)
			{
				// [#2270] AU KSK 2014.05.26 Note Counting Option이 Disable인 경우 매수가 2000매로 고정되어져 있으므로 보정처리는 하지 말아야함
				//                           추가 로직 수정함
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)	// Note Counting을 사용하는 경우
				{
					nTempRejectCstCnt[i] =  fnCDU_GetNumberOfSetCash(CDU_CST_1+i)		-
											fnCDU_GetNumberOfCash(CDU_CST_1+i)			-
											fnCDU_GetNumberOfDispenseCash(CDU_CST_1+i)	-
											fnCDU_GetNumberOfReject(CDU_CST_1+i);

					if (nTempRejectCstCnt[i] >= 0)
						nTempRejectCstCnt[i] = fnCDU_GetNumberOfReject(CDU_CST_1+i);
					else
						nTempRejectCstCnt[i] += fnCDU_GetNumberOfReject(CDU_CST_1+i);

					if (nTempRejectCstCnt[i] < 0)
						nTempRejectCstCnt[i] = 0;

					RejectCSTCnt += nTempRejectCstCnt[i];
				}
				else
				{
					// Note Counting을 사용하지 않는 경우 SP에서 올라온 정보를 그대로 표시
					RejectCSTCnt += fnCDU_GetNumberOfReject(CDU_CST_1+i);
				}
				// end of [#2270]
			}
			// end of [#393]

			FirstCSTCnt = fnCDU_GetNumberOfCash(CDU_CST_1);				// First CST Count(ClerkInformation)
			SecondCSTCnt = fnCDU_GetNumberOfCash(CDU_CST_2);			// Second CST Count(ClerkInformation)
			ThirdCSTCnt = fnCDU_GetNumberOfCash(CDU_CST_3);				// Third CST Count(ClerkInformation)
			FourthCSTCnt = fnCDU_GetNumberOfCash(CDU_CST_4);			// Fourth CST Count(ClerkInformation)
		}

		CduLoc			= fnCDU_GetPosition();

		if (CSTCnt >= CDU_CST_1)
			FirstCSTLoc		= (fnCDU_GetCSTStatus(CDU_CST_1) == CST_SET_NG)	? ABNORMAL_POS : NORMAL_POS;
		if (CSTCnt >= CDU_CST_2)
			SecondCSTLoc	= (fnCDU_GetCSTStatus(CDU_CST_2) == CST_SET_NG)	? ABNORMAL_POS : NORMAL_POS;
		if (CSTCnt >= CDU_CST_3)
			ThirdCSTLoc		= (fnCDU_GetCSTStatus(CDU_CST_3) == CST_SET_NG)	? ABNORMAL_POS : NORMAL_POS;
		if (CSTCnt >= CDU_CST_4)
			FourthCSTLoc	= (fnCDU_GetCSTStatus(CDU_CST_4) == CST_SET_NG)	? ABNORMAL_POS : NORMAL_POS;

		if (CSTCnt >= CDU_CST_1)
			FirstCSTStatus= (fnCDU_GetCSTStatus(CDU_CST_1) == CST_SET_NG)	? FirstCSTStatus : fnCDU_GetCSTStatus(CDU_CST_1);
		if (CSTCnt >= CDU_CST_2)
			SecondCSTStatus= (fnCDU_GetCSTStatus(CDU_CST_2) == CST_SET_NG)	? SecondCSTStatus : fnCDU_GetCSTStatus(CDU_CST_2);
		if (CSTCnt >= CDU_CST_3)
			ThirdCSTStatus= (fnCDU_GetCSTStatus(CDU_CST_3) == CST_SET_NG)	? ThirdCSTStatus : fnCDU_GetCSTStatus(CDU_CST_3);
		if (CSTCnt >= CDU_CST_4)
			FourthCSTStatus= (fnCDU_GetCSTStatus(CDU_CST_4) == CST_SET_NG)	? FourthCSTStatus : fnCDU_GetCSTStatus(CDU_CST_4);

		RejectCSTStatus	= (fnCDU_GetRejectCSTStatus() == CST_SET_NG)	? RejectCSTStatus : fnCDU_GetRejectCSTStatus();		// [#380] [NH] KSK 2008.7.21 RejectBin Set 안되는 Bug Fix

		fnCMN_ClearEventDevice(~DEV_CDU);
		fnCMN_ClearStatusDevice(~DEV_CDU);
	}

 	if (nResultDevice & DEV_SPR)
 	{
 		SlipStatus		= fnSPR_GetPaperStatus();
 
 		fnCMN_ClearEventDevice(~DEV_SPR);
 		fnCMN_ClearStatusDevice(~DEV_SPR);
 	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Add Serial No
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_AddSerialNo(HOST_CONFIG hostConfig, int nSerialLength)
{	
NHDEBUG(1, (_T("***DevApl***CDevCmn::fnAPL_AddSerialNo() \n")));

	if (hostConfig == HC_LIBERTYX)
	{
		DWORD sequence = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_LIBERTYX_SEQUENCE, 1);
		if (sequence >= 0xFFFF) 
		{
			sequence = 0;
		}

		sequence++;

		CString decimalSequence;
		decimalSequence.Format(L"%04d", sequence);
		NVDump('O', 'E', "12", L"Inc.LXSeq#", decimalSequence);

		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_LIBERTYX_SEQUENCE, sequence);
	}
	else 
	{
		CString strNum = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SEQUENCENO);	// [#371] [US] KSK 2008.7.16
		int nNum = Asc2Int(strNum);
		CString strAddNum = "";

		if(nNum >= 9999)
			nNum = 0;
		nNum++;
		strAddNum.Format(L"%04d", nNum);
		NVDump('O', 'E', "12", L"Inc.Seq#", strAddNum);							// [#2465] NH Justin 2017.01.12 Leave Increased Sequence Number in the NVRAM
		MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SEQUENCENO, strAddNum);	// [#371] [US] KSK 2008.7.16
	}

	return TRUE;
}

CString CDevCmn::fnAPL_GetSerialNo(HOST_CONFIG hostConfig) 
{
	if (hostConfig == HC_LIBERTYX)
	{
		DWORD sequence = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_LIBERTYX_SEQUENCE, 1);

		CString decimalSequence;
		decimalSequence.Format(L"%04d", sequence);

		return decimalSequence;
	}
	else 
	{
		CString strNum = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SEQUENCENO);
		int nNum = Asc2Int(strNum);

		CString strAddNum = "";
		strAddNum.Format(L"%04d", nNum);
		return strAddNum;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Set Process Count
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_SetProcCount(char chProcCount)
{
	//NHDEBUG(1, (_T("***DevApl***CDevCmn::fnAPL_SetProcCount() \n")));

	CString srtProCnt;

	srtProCnt.Format(L"%c", chProcCount);
	m_strNVLog.Format(L"1:ProcCnt(%c)", chProcCount);
	NVDump('O', 'E', "12", L"", m_strNVLog);
	NHDEBUG(1, (_T("%s \n"), m_strNVLog));

	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT, srtProCnt); // V01.02.25

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Set Health Check Timer Set
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_SetHealthCheckTimer()
{
NHDEBUG(1, (_T("***DevApl***CDevCmn::fnAPL_SetHealthCheckTimer() \n")));

	// Update Current Time
	fnAPL_GetHealthGapTime();

	// [#36] KSK 2008.03.18
	// HealthCheck 무한 Retry 되는 Bug Fix
	m_oletimeStart	= m_oletimeNow;
	m_DelayTime		= MemGetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_HSDELAYINT)*60*60;	// hour
	// end of [#36]

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Get Health Check Gap Time
/////////////////////////////////////////////////////////////////////////////
UINT CDevCmn::fnAPL_GetHealthGapTime()
{
	m_oletimeNow = COleDateTime::GetCurrentTime();
	m_oletimeGap = m_oletimeNow - m_oletimeStart;

	return (UINT)m_oletimeGap.GetTotalSeconds();
}

/////////////////////////////////////////////////////////////////////////////
// Clear Error
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_ClearError(int nClearOption)		// [#2492] NN Justin 2017.06.30 Add Parameter
{
	NHDEBUG(1, (_T("[%s] \n"), (nClearOption == CLEAR_ERROR_ALL) ? L"ALL" : L"NETWORK"));

	m_strNVLog.Format(L"1:CErr[%s])", (nClearOption == CLEAR_ERROR_ALL) ? L"ALL" : L"NET");  // GSCJSD-9322 To test
	NVDump('O', 'E', "00", L"", m_strNVLog);

	// [#2492] NN Justin 2017.06.30 Add Parameter
	// ClrCrtErrStack();
	BOOL bClearErrorStack = TRUE;

	m_strNVLog.Format(L"1:CErr[%d])", GetCrtErrStackCount());       // GSCJSD-9322 To test
	NVDump('O', 'E', "00", L"", m_strNVLog);

	if( (nClearOption == CLEAR_ERROR_NETWORK ) && (GetCrtErrStackCount()) )
	{
		if( fstrAPL_GetErrorCode().Left(1) !="D")
			bClearErrorStack = FALSE;
	}

	if(bClearErrorStack)
	{
		NVDump('O', 'E', "00", L"", L"ClErrStck");
		ClrCrtErrStack();
	}
	// End of [#2492]

	fnNET_ClearErrorCode();		// [#431] [NH] KSK 2008.10.02

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Check Error
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_CheckError()
{
	if (GetCrtErrStackCount())		return FALSE;
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Stack Error													
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_StackError(CString szErrorCode, CString szErrorString, int nDevId)
{
	NHDEBUG(1, (_T("***DevApl***CDevCmn::fnAPL_StackError() Code:[%s],String:[%s] \n"), szErrorCode, szErrorString));

	BOOL bErrSumStacked = FALSE;
	char szTemp2[128] = { 0, };

	szErrorCode += L"0000000";
	szErrorCode = szErrorCode.Left(7);
	szErrorCode.TrimLeft();

	m_strNVLog.Format(L"ECODE(%s)", szErrorCode);
	NVDump('O', 'E', "21", L"", m_strNVLog);

	if(szErrorCode.Left(1) == "D")
	{
		m_pNetWork->SetErrorCode(szErrorCode);
		SetErrSum(szErrorCode, nDevId);
		bErrSumStacked = TRUE;
		
		if (szErrorCode.Left(5) == "D0300" || szErrorCode.Left(5) == "D0301" ||
			szErrorCode.Left(5) == "D0302" || szErrorCode.Left(5) == "D0303" ||
			szErrorCode.Left(5) == "D0304" || szErrorCode.Left(5) == "D0305" ||
			szErrorCode.Left(5) == "D0306" || szErrorCode.Left(5) == "D0307")
			m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003075));			
		else if (szErrorCode.Left(5) == "D1500")
		{
			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == 1)	// Dialup
				m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003062));
			else
				m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003063));
		}
		else if (szErrorCode.Left(5) == "D1701")
			m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003067));
		else if (szErrorCode.Left(5) == "D1704")
			m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003068));
		else if (szErrorCode.Left(5) == "D1706")
			m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003069));
		else if (szErrorCode.Left(5) == "D1707")
			m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003071));
		else if (szErrorCode.Left(5) == "D1708")
			m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003072));
		else if (szErrorCode.Left(5) == "D1709")
			m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003073));
		else if (szErrorCode.Left(5) == "D1710")
			m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003074));
		else if (szErrorCode.Left(5) == "D1800")
			m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003064));
		else if (szErrorCode.Left(5) == "D1900")
			m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003065));
		else if (szErrorCode.Left(5) == "D2000")
			m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003066));
		else if (szErrorCode.Left(5) == "D2200")
			m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003070));
		// [#2093] NH PCS 2011.10.28 "SSL Connection Failure"
		else if (szErrorCode.Left(5) == "D1501")
			m_pNetWork->SetErrorMsg(fstrSCR_GetStringFromTextID(T_APMSG_003092)); 
		// end of [#2093] NH PCS 2011.10.28
		else
			m_pNetWork->SetErrorMsg(szErrorString);

		szErrorString = m_pNetWork->GetErrorMsg();		// [#361] NH PSC 2008.07.14
	}

	if (!fnAPL_CheckError())	return FALSE;
	
	if(szErrorCode == "0000000")
	{
		SetCrtErrStack("9999999", "INTERNAL CHECK !!!");
	}
	else
	{

		char szTemp[128] = { 0, };

		WideCharToMultiByte(CP_UTF8, 0, szErrorCode, -1, szTemp, sizeof(szTemp), NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, szErrorString, -1, szTemp2, sizeof(szTemp2), NULL, NULL);
		// end of [#2186]

		SetCrtErrStack(szTemp, szTemp2);
		// end of [#2012]
	}
	
	if (bErrSumStacked == FALSE)
		SetErrSum(szErrorCode, nDevId);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Get Current Error Code
/////////////////////////////////////////////////////////////////////////////
// [#419] [NH] KSK 2008.9.11
CString CDevCmn::fstrAPL_GetErrorProCount()
{
	CString strTemp;

	if (GetCrtErrStackCount())
		strTemp.Format(L"%1.1S", GetCrtErrProCount());

	return strTemp;
}
// end of [#419]

CString CDevCmn::fstrAPL_GetErrorCode(int nBracket)
{
	CString strTemp;
	CString	strTemp2;

	if (GetCrtErrStackCount())
	{
		if (nBracket == 0)
			strTemp.Format(L"%S", GetCrtErrCode());
		else
		{
			strTemp2.Format(L"%7.7S", GetCrtErrCode());
			strTemp.Format(L"%5.5s(%2.2s)", strTemp2.Left(5), strTemp2.Right(2));
		}
	}

	return strTemp;
}

/////////////////////////////////////////////////////////////////////////////
// Get Current Error Message
/////////////////////////////////////////////////////////////////////////////
CString CDevCmn::fstrAPL_GetErrorMessage()
{
	// [#419] [NH] KSK 2008.9.11
	CString strTemp;

	// [#2186] US KMK 2013.05.01 에러메시지가 유니코드일때 출력 문제 수정
// 	if (GetCrtErrStackCount())
// 		strTemp.Format(L"%S", GetCrtErrMessage());
	
	WCHAR* wchTmp = new TCHAR[128];

	if (GetCrtErrStackCount())
	{
		char* chTmp = GetCrtErrMessage();
		MultiByteToWideChar(CP_UTF8, 0, chTmp, -1, wchTmp, 128);
// 		MultiToWide(wchTmp, chTmp, 128);
		strTemp.Format(L"%s", wchTmp);
	}

	delete[] wchTmp;		// US KMK 2013.07.01 코드소나 지적대응 (메모리 누수)

	return strTemp;
	// end of [#419]
}

/////////////////////////////////////////////////////////////////////////////
// Check Host Open
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_CheckHostOpen()
{
	return HostOpenFlag;
}

/////////////////////////////////////////////////////////////////////////////
// Initialize Device
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_InitializeDevice(int nDevId, int CDUInitialType)
{
	NHDEBUG(1, (_T("***DevApl***CDevCmn::fnAPL_InitializeDevice() \n")));

	m_strNVLog.Format(L"1:INITDVC(%x,%d)", nDevId, CDUInitialType);
	NVDump('O', 'E', "25", L"", m_strNVLog);

	int		nCardEjectFlag = FALSE;								// 2005.09.11

	nDevId = fnAPL_GetDefineDevice(nDevId);
	DeviceEvent |= nDevId;										// 2004.08.23

/////////////////////////////////////////////////////////////////////////////
	if (nDevId & DEV_SPR)										// 2005.04.03
	{
		fnAPL_CheckDeviceAction(DEV_SPR);
	}

	if (nDevId & DEV_MCU)										// 2005.04.03
	{
		fnAPL_CheckDeviceAction(DEV_MCU);
	}

	if (nDevId & DEV_CDU)										// 2005.04.03
	{
		fnAPL_CheckDeviceAction(DEV_CDU);
	}

	if (nDevId & DEV_PIN)										// 2006.02.18
	{
		// 2019.07.17 need to deactivate "ReadData/Pin Mode" before CheckDeviceAction to avoid MWI TIMEOUT.
		fnPIN_EntryDisable();

		fnAPL_CheckDeviceAction(DEV_PIN);
	}

	// [#2325] NH KSK 2015.01.20
	if (nDevId & DEV_RFID)
	{
		fnAPL_CheckDeviceAction(DEV_RFID);
	}
	// end of [#2325]

	if (nDevId & DEV_BCR)			{	fnAPL_CheckDeviceAction(DEV_BCR);	}		// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	if (nDevId & DEV_BNA)			{	fnAPL_CheckDeviceAction(DEV_BNA);	}		// [#GLDV-3005] US Kook 2022.01.03 Support Side Car

//	if (nDevId & (DEV_MAIN|DEV_PIN))									// 2005.04.03	[#589] NH KSK 2009.12.07 PIN DEVICE 추가
	if (nDevId & (DEV_MAIN|DEV_PIN|DEV_RFID|DEV_BCR|DEV_BNA))			// [#2325] NH KSK 2015.01.20	// [#GLDV-3005]
	{
		fnAPL_ClearError();										// 2005.04.03
	}

/////////////////////////////////////////////////////////////////////////////
	if ((fnDOR_GetDeviceStatus() == NODEVICE)	||				// 2005.04.19
		(fnDOR_GetDeviceStatus() == DOWN))
	{
		fnDOR_ClearErrorCode();
		fnDOR_Initialize();
	}
	if ((fnLGT_GetDeviceStatus() == NODEVICE)	||
		(fnLGT_GetDeviceStatus() == DOWN))
	{
		fnLGT_ClearErrorCode();
		fnLGT_Initialize();
	}
	if ((fnPIN_GetDeviceStatus() == NODEVICE)	||
		(fnPIN_GetDeviceStatus() == DOWN))
	{
		fnPIN_ClearErrorCode();
		fnPIN_Initialize();
	}
	if ((fnSNS_GetDeviceStatus() == NODEVICE)	||
		(fnSNS_GetDeviceStatus() == DOWN))
	{
		fnSNS_ClearErrorCode();
		fnSNS_Initialize();
	}
	if ((fnSCR_GetDeviceStatus() == NODEVICE)	||
		(fnSCR_GetDeviceStatus() == DOWN))
	{
		fnSCR_ClearErrorCode();
		fnSCR_Initialize();
	}

/////////////////////////////////////////////////////////////////////////////
	if (nDevId & DEV_SPR)										// 2004.11.05
	{
		fnSPR_ClearErrorCode();
		fnSPR_Initialize();
	}

	if (nDevId & DEV_CDU)										// 2004.11.05
	{
		fnCDU_ClearErrorCode();
		fnCDU_Initialize(CDUInitialType);	// [#507] [NH] KSK 2009.2.23
	}

	if (nDevId & DEV_MCU)										// 2004.11.05
	{
		fnMCU_ClearErrorCode();
		fnMCU_Initialize();
	}

	// [#2325] NH KSK 2015.01.20
	if (nDevId & DEV_RFID)
	{
		fnRFID_ClearErrorCode();
		fnRFID_Initialize();
	}
	// end of [#2325]

	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	if (nDevId & DEV_BCR)
	{
		fnBCR_ClearErrorCode();
		fnBCR_Initialize();
	}

	if (nDevId & DEV_BNA)
	{
		fnBNA_ClearErrorCode();
		fnBNA_Initialize();
	}
	// end of [#GLDV-3005]

/////////////////////////////////////////////////////////////////////////////
	if (nDevId & DEV_SPR)
	{
		fnAPL_CheckDeviceAction(DEV_SPR);						// 2004.07.26
	}

	if (nDevId & DEV_MCU)
	{
		fnAPL_CheckDeviceAction(DEV_MCU);						// 2005.09.11
	}

	// [#2325] NH KSK 2015.01.20
	if (nDevId & DEV_RFID)
	{
		fnAPL_CheckDeviceAction(DEV_RFID);						// 2005.09.11
	}
	// end of [#2325]

	if (nDevId & DEV_CDU)										// 2004.06.30
	{
		fnAPL_CheckDeviceAction(DEV_CDU);						// 2004.07.26
		// [#24] KSK 2008.04.8 V01.02.19 Merge 누락 적용
		// 2007.11.26 V01.02.19 CST VALUE를 UPDATE하기 위해 CSTCnt를 0로 만든다.
		CSTCnt = 0;
		// end of [#24]
	}

	if (nDevId & DEV_BCR)			{	fnAPL_CheckDeviceAction(DEV_BCR);	}		// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	if (nDevId & DEV_BNA)			{	fnAPL_CheckDeviceAction(DEV_BNA);	}		// [#GLDV-3005] US Kook 2022.01.03 Support Side Car

	m_strNVLog.Format(L"2:INITDVC(%x)", nDevId);
	NVDump('O', 'E', "25", L"", m_strNVLog);

	fnAPL_CheckDeviceAction(nDevId);

	if (nDevId != DEV_NONE
		&& CDUInitialType != INIT_BY_OPENSESSION)
	{
		NHDEBUG(1, (_T(">>> Delay_Msg(2000) \n")));
		Delay_Msg(2000);				// [#116] KSK 2008.04.16 Device Reset시에만 Delay주도록 수정
	}

	fnAPL_CheckDevice();										// Check Device

	if (fnAPL_GetErrorDevice(nDevId))	return FALSE;
	else								return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Check Device
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_CheckDevice()
{
	/////////////////////////////////////////////////////////////////////////////
	// No Device Check
	if (fnDOR_GetDeviceStatus() == NODEVICE)					// 2004.11.05
		DeviceNoDevice |= DEV_DOR;
	else
		DeviceNoDevice &= ~DEV_DOR;

	if (fnLGT_GetDeviceStatus() == NODEVICE)					// 2005.03.30
		DeviceNoDevice |= DEV_LGT;
	else
		DeviceNoDevice &= ~DEV_LGT;

	if (fnPIN_GetDeviceStatus() == NODEVICE)
		DeviceNoDevice |= DEV_PIN;
	else
		DeviceNoDevice &= ~DEV_PIN;

	if (fnSNS_GetDeviceStatus() == NODEVICE)
		DeviceNoDevice |= DEV_SNS;
	else
		DeviceNoDevice &= ~DEV_SNS;

	if (fnSCR_GetDeviceStatus() == NODEVICE)
		DeviceNoDevice |= DEV_SCR;
	else
		DeviceNoDevice &= ~DEV_SCR;

	/////////////////////////////////////////////////////////////////////////////
	if (fnSPR_GetDeviceStatus() == NODEVICE)
		DeviceNoDevice |= DEV_SPR;
	else
		DeviceNoDevice &= ~DEV_SPR;

	if (fnMCU_GetDeviceStatus() == NODEVICE)
		DeviceNoDevice |= DEV_MCU;
	else
		DeviceNoDevice &= ~DEV_MCU;

	if (fnCDU_GetDeviceStatus() == NODEVICE)
		DeviceNoDevice |= DEV_CDU;
	else
		DeviceNoDevice &= ~DEV_CDU;

	// [#2325] NH KSK 2015.01.20
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
	{
		if (fnRFID_GetDeviceStatus() == NODEVICE)
			DeviceNoDevice |= DEV_RFID;
		else
			DeviceNoDevice &= ~DEV_RFID;
	}
	else
	{
		DeviceNoDevice &= ~DEV_RFID;
	}
	// end of [#2325]

	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
	{
		if (fnBCR_GetDeviceStatus() == NODEVICE)
			DeviceNoDevice |= DEV_BCR;
		else
			DeviceNoDevice &= ~DEV_BCR;

		if (fnBNA_GetDeviceStatus() == NODEVICE)
			DeviceNoDevice |= DEV_BNA;
		else
			DeviceNoDevice &= ~DEV_BNA;
	}
	else
	{
		DeviceNoDevice &= ~DEV_BCR;
		DeviceNoDevice &= ~DEV_BNA;
	}
	// end of [#GLDV-3005]

	/////////////////////////////////////////////////////////////////////////////
	// Down Device Check
	if (fnDOR_GetDeviceStatus() == DOWN)						// 2004.11.05
		DeviceDownDevice |= DEV_DOR;
	else
		DeviceDownDevice &= ~DEV_DOR;

	if (fnLGT_GetDeviceStatus() == DOWN)						// 2005.03.30
		;
	else
		DeviceDownDevice &= ~DEV_LGT;

	if (fnPIN_GetDeviceStatus() == DOWN)
		DeviceDownDevice |= DEV_PIN;
	else
		DeviceDownDevice &= ~DEV_PIN;

	if (fnSNS_GetDeviceStatus() == DOWN)
		DeviceDownDevice |= DEV_SNS;
	else
		DeviceDownDevice &= ~DEV_SNS;

	if (fnSCR_GetDeviceStatus() == DOWN)
		DeviceDownDevice |= DEV_SCR;
	else
		DeviceDownDevice &= ~DEV_SCR;

	/////////////////////////////////////////////////////////////////////////////
	if (fnSPR_GetDeviceStatus() == DOWN)
		DeviceDownDevice |= DEV_SPR;
	else
		DeviceDownDevice &= ~DEV_SPR;

	if (fnMCU_GetDeviceStatus() == DOWN)
		DeviceDownDevice |= DEV_MCU;
	else
		DeviceDownDevice &= ~DEV_MCU;

	if (fnCDU_GetDeviceStatus() == DOWN)
		DeviceDownDevice |= DEV_CDU;
	else
	{
		DeviceDownDevice &= ~DEV_CDU;
	}

	// [#2325] NH KSK 2015.01.20
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
	{
		if (fnRFID_GetDeviceStatus() == DOWN)
			DeviceDownDevice |= DEV_RFID;
		else
			DeviceDownDevice &= ~DEV_RFID;
	}
	else
	{
		DeviceDownDevice &= ~DEV_RFID;
	}
	// end of [#2325]


	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
	{
		if (fnBCR_GetDeviceStatus() == DOWN)
			DeviceDownDevice |= DEV_BCR;
		else
			DeviceDownDevice &= ~DEV_BCR;

		if (fnBNA_GetDeviceStatus() == DOWN)
			DeviceDownDevice |= DEV_BNA;
		else
			DeviceDownDevice &= ~DEV_BNA;
	}
	else
	{
		DeviceDownDevice &= ~DEV_BCR;
		DeviceDownDevice &= ~DEV_BNA;
	}

	/////////////////////////////////////////////////////////////////////////////
	//	fnAPL_CheckError();											// 2005.04.03	[#419] [NH] KSK 2008.9.11
	/////////////////////////////////////////////////////////////////////////////
	// CST Information Procedure
	if (fnCDU_GetDeviceStatus() != NODEVICE)					// 2004.04.19
	{
		if (!CSTCnt)											// CST Information Setting
		{
			fnAPL_CheckDeviceAction(DEV_CDU);					// 2004.08.13
			CSTCnt		= fnCDU_GetNumberOfCST();

			// [#146] KSK 2008.04.20 Add Currency ID
			// CURRENCY ID는 MWI에서 무조건 CST1에 대해서만 값을 RETURN해줌
			m_strCurrencyID = fnCDU_GetCurrencyID(CDU_CST_1);
			// end of [#146]

			if (m_strCurrencyID != ST_UNKNOWN)					// avoid setting to "UNKNOWN" which is occurred when NVRAM.DAT is deleted.
			{
				fnCDU_SetCurrencyID(m_strCurrencyID);	// [#514] [MX] KSK 2009.3.10
			}

			if (CSTCnt >= CDU_CST_1)							// 2004.08.23
			{
				FirstCSTValue	= fnCDU_GetValueOfCash(CDU_CST_1);
				FirstCSTCnt = fnCDU_GetNumberOfCash(CDU_CST_1);	// First CST Count(ClerkInformation)
			}
			if (CSTCnt >= CDU_CST_2)							// 2004.08.23
			{
				SecondCSTValue	= fnCDU_GetValueOfCash(CDU_CST_2);
				SecondCSTCnt = fnCDU_GetNumberOfCash(CDU_CST_2);	// Second CST Count(ClerkInformation)
			}
			if (CSTCnt >= CDU_CST_3)							// 2004.08.23
			{
				ThirdCSTValue	= fnCDU_GetValueOfCash(CDU_CST_3);
				ThirdCSTCnt = fnCDU_GetNumberOfCash(CDU_CST_3);	// Third CST Count(ClerkInformation)
			}
			if (CSTCnt >= CDU_CST_4)							// 2004.08.23
			{
				FourthCSTValue	= fnCDU_GetValueOfCash(CDU_CST_4);
				FourthCSTCnt = fnCDU_GetNumberOfCash(CDU_CST_4);	// Fourth CST Count(ClerkInformation)
			}
			
			fnAPL_CheckDeviceAction(DEV_CDU);					// 2004.11.05
		}
	}
	
	if (!CSTCnt)												// CST No Information : 2005.04.03
	{
		DeviceDownDevice |= DEV_CDU;
		DeviceStatus |= DEV_CDU;
//		if (fnAPL_CheckError())		// [#419] [NH] KSK 2008.9.11	KSK 2009.07.12 DIAG에서 AP전환시 CDU 단선시 97400 ERROR CODE가 SET 안되는 BUG FIX
//			fnAPL_StackError(_T("9799499"), fstrSCR_GetStringFromTextID(T_APMSG_003003), DEV_CDU);		//[#257] [MX] KSK 2008.6.5
	}

	DeviceDownDevice |= DeviceNoDevice;							// 2004.11.05

/////////////////////////////////////////////////////////////////////////////
	// Down Device Procedure(~DEV_MAIN)							// 2005.04.03
	if (DeviceDownDevice & DEV_DOR)
	{
		DeviceStatus |= DEV_DOR;
		if (fnAPL_CheckError())		// [#419] [NH] KSK 2008.9.11
			fnAPL_StackError(_T("9799901"), fstrSCR_GetStringFromTextID(T_APMSG_003004), DEV_DOR);		//[#257] [MX] KSK 2008.6.5
	}

	if (DeviceDownDevice & DEV_LGT)
	{
		DeviceStatus |= DEV_LGT;
		if (fnAPL_CheckError())		// [#419] [NH] KSK 2008.9.11
			fnAPL_StackError(_T("9799902"), fstrSCR_GetStringFromTextID(T_APMSG_003005), DEV_LGT);		//[#257] [MX] KSK 2008.6.5
	}

	if (DeviceDownDevice & DEV_PIN)
	{
		DeviceStatus |= DEV_PIN;
		if (fnAPL_CheckError())		// [#419] [NH] KSK 2008.9.11
		{
			// [#2580] AU Kook 2019.07.17 EPP Tamper Switch Removal Detection (9799904 -> E21FF05 / EX1FFxx / 9EA61)
			CString strPINErrCode = fstrPIN_GetErrorCode();

			if (strPINErrCode.Left(5) == L"E21FF"												// PCI 3.0
				|| strPINErrCode.Left(5) == L"EX1FF" || strPINErrCode.Left(5) == L"9EA61")		// PCI 2.0
			{
				fnAPL_StackError(strPINErrCode, fstrSCR_GetStringFromTextID(T_APMSG_003006), DEV_PIN);
			}
			else
			// end of [#2580]
				fnAPL_StackError(_T("9799904"), fstrSCR_GetStringFromTextID(T_APMSG_003006), DEV_PIN);		//[#257] [MX] KSK 2008.6.5
		}
	}

	if (DeviceDownDevice & DEV_SNS)
	{
		DeviceStatus |= DEV_SNS;
		if (fnAPL_CheckError())		// [#419] [NH] KSK 2008.9.11
			fnAPL_StackError(_T("9799905"), fstrSCR_GetStringFromTextID(T_APMSG_003007), DEV_SNS);		//[#257] [MX] KSK 2008.6.5
	}

	if (DeviceDownDevice & DEV_SCR)
	{
		DeviceStatus |= DEV_SCR;
		if (fnAPL_CheckError())		// [#419] [NH] KSK 2008.9.11
			fnAPL_StackError(_T("9799908"), fstrSCR_GetStringFromTextID(T_APMSG_003008), DEV_SCR);		//[#257] [MX] KSK 2008.6.5
	}

#if ! DEV_AUTO_OFF_SPR
	// [#254] NH AIREAT 2008.06.03 : SPR-AUTO OFF
	if (DeviceDownDevice & DEV_SPR)
	{
		DeviceStatus |= DEV_SPR;
		if (fnAPL_CheckError())		// [#419] [NH] KSK 2008.9.11
		{
			if (SlipStatus == SLIP_EMPTY_PAPER)
				fnAPL_StackError(_T("2001300"), fstrSCR_GetStringFromTextID(T_APMSG_003009), DEV_SPR);	// [#419] [NH] KSK 2008.9.16
			else
				fnAPL_StackError(_T("2001200"), fstrSCR_GetStringFromTextID(T_APMSG_003010), DEV_SPR);		// [#419] [NH] KSK 2008.9.16
		}
	}
#endif

	if (DeviceDownDevice & DEV_CDU)
	{
		DeviceStatus |= DEV_CDU;
		// 2007.11.27 V01.02.19
		// DENOMINATION 변경장애 971A6인 경우에는 ERROR 문구를 변경한다
		if (fnAPL_CheckError())		// [#419] [NH] KSK 2008.9.11
		{
			if (fstrCDU_GetErrorCode().Left(5) == "971A6")
				fnAPL_StackError(fstrCDU_GetErrorCode(), fstrSCR_GetStringFromTextID(T_APMSG_003011), DEV_CDU);	//[#257] [MX] KSK 2008.6.5
			else if (fstrCDU_GetErrorCode().Left(5) == "971A7")
				fnAPL_StackError(fstrCDU_GetErrorCode(), fstrSCR_GetStringFromTextID(T_APMSG_003012), DEV_CDU);	//[#257] [MX] KSK 2008.6.5
			else
				fnAPL_StackError(fstrCDU_GetErrorCode(), fstrSCR_GetStringFromTextID(T_APMSG_003013), DEV_CDU);	//[#257] [MX] KSK 2008.6.5
		}
	}

	// [#68] UK JSW 2008.05.16 Magtek MCU
	if (DeviceDownDevice & DEV_MCU)
	{
		DeviceStatus |= DEV_MCU;	// 2008.07.09 Magtek 단선시 복구안되는 현상 임시대책. DeviceStatus 가 0으로 중간에 바뀌는 문제 대응.
									// 추후에 Magtek과 Dip type을 구분하여 CheckDeviceAction 을 선택적으로 적용해야 할것 같음.
		if (fnAPL_CheckError())		// [#2023] [NH] KSK 2011.02.23
			fnAPL_StackError(fstrMCU_GetErrorCode(), fstrSCR_GetStringFromTextID(T_APMSG_003084), DEV_MCU);	// [#397] Magtek 단선시 Device Error event 체크 안되는 문제 수정.
	}

	// [#2325] NH KSK 2015.01.20 RFID는 기본적으로 축퇴지원하도록 하기 위해 추가 Coding 안함

	// [#GLDV-3005] US Kook 2022.01.03 Side Car
	// TODO: Should we go OOS when there are BCR/BNA errors?
	//if (DeviceDownDevice & DEV_BCR)
	//{
	//	DeviceStatus |= DEV_BCR;
	//	if (fnAPL_CheckError())
	//		fnAPL_StackError(fstrBCR_GetErrorCode(), /*fstrSCR_GetStringFromTextID(T_APMSG_003084)*/L"Side Car Error (BCR)", DEV_BCR);
	//}
	//if (DeviceDownDevice & DEV_BNA)
	//{
	//	DeviceStatus |= DEV_BNA;
	//	if (fnAPL_CheckError())
	//		fnAPL_StackError(fstrBNA_GetErrorCode(), /*fstrSCR_GetStringFromTextID(T_APMSG_003084)*/L"Side Car Error (BNA)", DEV_BNA);
	//}
	// end of [#GLDV-3005]

	// [#RWC6-88] Vault Door Error Code is Blank if ATM is Booted with Door Open
	if (fnDOR_GetDoorStatus() == DOOR_OPENED)
	{
		if (fnAPL_CheckError())
			fnAPL_StackError(_T("2000400"), fstrSCR_GetStringFromTextID(T_APMSG_003061), DEV_DOR);
	}
	// end of // [#RWC6-88]

	// [#GLDV-3005] US Kook 2022.03.22 Side Car Safe Door
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
	{
		if (IsSideCarDoorOpen())
		{
			if (fnAPL_CheckError())
				fnAPL_StackError(_T("2000600"), fstrSCR_GetStringFromTextID(T_APMSG_003061), DEV_DOR);
		}
	}
	// end of [#GLDV-3005]

	if (DeviceDownDevice)		return FALSE;
	else						return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Check Device Action
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_CheckDeviceAction(int nDevId, int nCheckTime)
{
	CString	strTempAction("");
	int		nTempErrorDevice = DEV_NONE;

	nDevId = fnAPL_GetDefineDevice(nDevId);

	strTempAction = fstrCMN_CheckDeviceAction(nDevId, nCheckTime);
	//NHDEBUG(1, (_T("***DevApl***CDevCmn::fnAPL_CheckDeviceAction() fstrCMN_CheckDeviceAction[%s], nDevId[%x] \n"), strTempAction, nDevId));
	if (strTempAction == CHK_NORMAL)
	{
		if (nDevId & DEV_SPR)			DeviceStatus &= ~DEV_SPR;
		if (nDevId & DEV_MCU)			DeviceStatus &= ~DEV_MCU;
		if (nDevId & DEV_CDU)			DeviceStatus &= ~DEV_CDU;
		if (nDevId & DEV_PIN)			DeviceStatus &= ~DEV_PIN;	// [#580] SOOK 2009.11.12 PIN 단선 뒤 복구하여도 PIN DEVICESTATUS 갱신 안 되는 문제 
		if (nDevId & DEV_RFID)			DeviceStatus &= ~DEV_RFID;	// [#2325] NH KSK 2015.01.20
		if (nDevId & DEV_BCR)			DeviceStatus &= ~DEV_BCR;	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
		if (nDevId & DEV_BNA)			DeviceStatus &= ~DEV_BNA;	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	}
	else														// CHK_TIMEOVER
	{															// CHK_ERROR
		nTempErrorDevice = fnCMN_GetErrorDevice(nDevId);
		// [#2325] NH KSK 2015.01.20 RFID Device는 축퇴이므로 Error Setting하지 않음
		DeviceStatus |= (nTempErrorDevice & DEV_MAIN);
//		DeviceStatus |= (nTempErrorDevice & (DEV_MAIN|DEV_RFID));
		// end of [#2325]

		// [#GLDV-3005] US Kook 2022.03.15 Support Side Car
#if !DEV_AUTO_OFF_SIDECAR
		DeviceStatus |= (nTempErrorDevice & DEV_BCR);
		DeviceStatus |= (nTempErrorDevice & DEV_BNA);
#endif
		// end of [#GLDV-3005]

		if (DeviceOperationFlag)								// 2005.05.26
		{
			DeviceOperationFlag = FALSE;
			NHDEBUG(1, (_T("***DevApl***CDevCmn::fnAPL_CheckDeviceAction() [%x], ErrorDevice[%x] \n"), DeviceOperationFlag, nTempErrorDevice));
			if (nTempErrorDevice & DEV_SPR)
				fnAPL_StackError(fstrSPR_GetErrorCode(), fstrSCR_GetStringFromTextID(T_APMSG_003010), DEV_SPR);	//[#257] [MX] KSK 2008.6.5
			if (nTempErrorDevice & DEV_MCU)
				fnAPL_StackError(fstrMCU_GetErrorCode(), fstrSCR_GetStringFromTextID(T_APMSG_003014), DEV_MCU);	//[#257] [MX] KSK 2008.6.5
			if (nTempErrorDevice & DEV_CDU)
				fnAPL_StackError(fstrCDU_GetErrorCode(), fstrSCR_GetStringFromTextID(T_APMSG_003013), DEV_CDU);	//[#257] [MX] KSK 2008.6.5

			// [#2325] NH KSK 2015.01.20 RFID 장애 Detect 검토 필요
//			if (nTempErrorDevice & DEV_RFID)
//				fnAPL_StackError(fstrRFID_GetErrorCode(), fstrSCR_GetStringFromTextID(T_APMSG_003093), DEV_RFID);
			// end of [#2325]

			// [#GLDV-3005] US Kook 2022.03.15 Support Side Car
#if !DEV_AUTO_OFF_SIDECAR
			if (nTempErrorDevice & DEV_BCR)
				fnAPL_StackError(fstrBCR_GetErrorCode(), L"BCR ERROR", DEV_BCR);	//[#257] [MX] KSK 2008.6.5
			if (nTempErrorDevice & DEV_BNA)
				fnAPL_StackError(fstrBNA_GetErrorCode(), L"BNA ERROR", DEV_BNA);	//[#257] [MX] KSK 2008.6.5
#endif
			// end of [#GLDV-3005]
		}
	}

	if (fnAPL_GetErrorDevice(nDevId))	return FALSE;
	else								return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Check Material
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_CheckMaterial(int nDevId)
{
	int		nTempReturn = TRUE;

	nDevId = fnAPL_GetAvailDevice(nDevId);

	if (nDevId & DEV_CDU)
	{
		if (fnCDU_GetMaterialInfo()) 
		{
			// V01.02.25
			// CDU 단선시 8216091이 잠시 Display되는 현상 수정
			if(fnCDU_GetDeviceStatus() == OFFLINE)
			{
				fnAPL_StackError(_T("4DN0000"), fstrSCR_GetStringFromTextID(T_APMSG_003015), DEV_CDU);	//[#257] [MX] KSK 2008.6.5
				DeviceStatus |= DEV_CDU;
				nTempReturn = FALSE;
			}
			else
			{
				// [#419] [NH] KSK 2008.9.11
				if (fnAPL_CheckError() || fstrAPL_GetErrorCode().Left(1) != "4")	// Error가 없거나 "4"장애가 아닌 경우 8216091을 set한다.
					fnAPL_StackError(_T("8216091"), fstrSCR_GetStringFromTextID(T_APMSG_003016), DEV_CDU);
				
				DeviceStatus |= DEV_CDU;
				nTempReturn = FALSE;
				// end of [#419]
			}
		}
	}

	return nTempReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Retract Material
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnAPL_RetractMaterial(int nDevId)
{
NHDEBUG(1, (_T("***DevApl***CDevCmn::fnAPL_RetractMaterial() \n")));

	int		CduRetractCnt = -1;									// 2005.04.19
	CStringArray strTempRetractArray;
	
	if (fnAPL_GetYesDevice(DEV_CDU))							// 2005.04.19
		CduRetractCnt = fnCDU_GetNumberOfCash(CDU_CST_REJECT);
	
	nDevId = fnAPL_GetAvailDevice(nDevId);
	
	if (nDevId & DEV_SPR)
	{
		if (fnSPR_GetMaterialInfo())
			fnSPR_Retract();
	}

	fnAPL_CheckDeviceAction(nDevId);

	if (nDevId & DEV_CDU)
	{
		fnAPL_CheckDeviceAction(DEV_CDU);
		Delay_Msg(500);										// Sensor Wait(500ms)
		fnAPL_CheckDeviceAction(DEV_CDU);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Display & Print Error
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_DisplayPrintError(int nInitFlag)
{
	static	CString	strSaveErrorCode;
	static	int		nSaveClerkErrorMsgNo = 0;
	
	int		nTempClerkErrorMsgNo = 0;
	CString	strTempCmpErrorString;
	CString	strTempErrorString;
	CString	tempErrCode;
	CString strTemp;	

	if (nInitFlag)			;
	else if (AtmStatus != AtmStatusSave)		return TRUE;

	switch (AtmStatus)
	{
		case ATM_ERROR: 										// Error Mode
			if (nInitFlag)				strSaveErrorCode = L"";

			tempErrCode = fstrAPL_GetErrorCode();	// [#419] [NH] KSK 2008.9.11

			// [#2079] NH KSK 2011.07.15
			if (fstrAPL_GetErrorCode().Left(5) == L"31095" || fstrAPL_GetErrorCode().Left(5) == L"31096")
			{
				if ((fnSCR_GetCurrentScreenNo() != 6) || (strSaveErrorCode.CompareNoCase(tempErrCode)))
				{
					// [#2085] NH KSK 2011.07.22 
					if (bAntiSkimmingFlag == FALSE)
						m_AntiSkiimingRecoveryTime = CTime::GetCurrentTime() + (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_AUTORECOVERYTIME) * 60);
					bAntiSkimmingFlag = TRUE;
					// end of [#2085]

					fnSCR_ShowScreen(6);

					fnAPL_EnDisableHotKey(ENABLE);
					MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT, fstrAPL_GetErrorProCount().Left(2));	// KSK 2012.03.01 Code Sonar 지적사항 대책
				}
			}
			//else if ((fnSCR_GetCurrentScreenNo() != 2) || (strSaveErrorCode.CompareNoCase(tempErrCode)))
			else if( ( (fnSCR_GetCurrentScreenNo()!=2)&&(fnSCR_GetCurrentScreenNo()!=7) ) || (strSaveErrorCode.CompareNoCase(tempErrCode)) )	// [#2382] US Justin 2015.12.15 Support Mobile App (MoniMobile)
			{
				// [#2382] US Justin 2015.12.14 Support Mobile App (MoniMobile)
				//fnSCR_DisplayPrevSet(2);
				int nScrNum = 2;
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MOBILEAPP_QRCODE) == ENABLE)	
					nScrNum = 7;			// Screen 7 : Out of Service Screen with QR Code.

				fnSCR_DisplayPrevSet(nScrNum);

				if (nScrNum == 7)
					fnAPL_DisplayQRCodeOnScreen(MOBILEAPP_QR_IMAGEERROR);
				// End of [#2382]				
				
				// Error Code and Description
				strTemp.Format(L"%s : %s-%s", fstrSCR_GetStringFromTextID(T_APMSG_002001),
											fstrAPL_GetErrorProCount(),
											fstrAPL_GetErrorCode(1));
				fnSCR_DisplayString(3, strTemp);

				// [#2339] NH Justin 2015.04.07 Prevent Font corruption on Out of Service Screen 
				//			#2186 => NOT WORKING : The language is cleared when ATM enters "ATM_ERROR MODE" by "m_pDevCmn->fnAPL_UnUsed(ATM_ERROR)"
				/*
				// [#2186] US KMK 2013.05.03 아시아 언어 사용 시, 기본언어와 거래언어가 다를 경우 error desciption 깨짐문제 발생
				// 아시아 언어일 경우 출력 안하도록 임시 조치
				// fnSCR_DisplayString(4, fstrAPL_GetErrorMessage());
				switch (fnSCR_GetCurrentLangMode())
				{
					case ENG_MODE:		case SPN_MODE:		case FRN_MODE:
						fnSCR_DisplayString(4, fstrAPL_GetErrorMessage());
					break;

					case CHN_MODE:		case KOR_MODE:		case JPN_MODE:
						//fnSCR_DisplayString(4, fstrAPL_GetErrorMessage());		// 아시아 언어일 경우 미출력
					break;
				}
				// end of [#2186]
				*/
				NHDEBUG(1, (_T("Previous Language Mode, Mode =[%d]\n"), m_nPrevLangMode));
		
				// [#2468] AU KSK 2017.01.18 전국가 적용
				// [#2434] AU Kook 2016.07.19 Support Multi Languages
				//#if (CA_VERSION || US_VERSION || MX_VERSION || AU_A_VERSION)
					if( m_nPrevLangMode==MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE) )
						fnSCR_DisplayString(4, fstrAPL_GetErrorMessage());
				//#else
				//	fnSCR_DisplayString(4, fstrAPL_GetErrorMessage());
				//#endif
				// End of [#2434]
				// end of [#2468]
				//fnSCR_DisplayScreen(2);		
				fnSCR_DisplayScreen(nScrNum);					// [#2382] US Justin 2015.12.14 Support Mobile App (MoniMobile)

				fnAPL_EnDisableHotKey(ENABLE);

				//MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT, GetSprintf("%2.2S", fstrAPL_GetErrorProCount()));	// [#419] [NH] KSK 2008.9.11
				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT, fstrAPL_GetErrorProCount().Left(2));	// KSK 2012.03.01 Code Sonar 지적사항 대책
			}
			// end of [#2079]
			strSaveErrorCode.Format(L"%s", fstrAPL_GetErrorCode());
			break;

		case ATM_CLERK:
			{
				//if (m_pConfig->GetOSVersion() == NH_OS_NH2700_F_T || m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T)	// [#2064] NH KSK 2011.05.18
				if (GetDeviceType(L"SPL") == SPL_SET)															// [#GLDV-2505] US Kook 2019.05.13
				{
					if (m_nHotKeyStatus == OPER_SUPERVISOR)
					{
						// OPERATOR 동작중임을 화면에 표시한다.
						fnSCR_ShowScreen(3);
					}
				}
			}
			break;

		case ATM_RMSACTIVE:	
			{
				//RMS 동작중임을 화면에 표시한다.
				fnSCR_ShowScreen(5);
			}
			break;
		default:
			break;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Reset Device
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_ResetDevice(int nDevId, int CDUInitialType)	// [#507] [NH] KSK 2009.2.23
{
	NHDEBUG(1, (_T("***DevApl***CDevCmn::fnAPL_ResetDevice() \n")));

	int		SprRetractCnt = -1;									// Default -1
	int		McuRetractCnt = -1;
	int		CduRetractCnt = -1;									// 2005.04.19
	CStringArray strTempRetractArray;

	if (fnAPL_GetYesDevice(DEV_CDU))							// 2005.04.19
		CduRetractCnt = fnCDU_GetNumberOfCash(CDU_CST_REJECT);

	nDevId = fnAPL_GetDefineDevice(nDevId);

	fnAPL_InitializeDevice(nDevId, CDUInitialType);

	if (nDevId & DEV_CDU
		&& CDUInitialType != INIT_BY_OPENSESSION)
	{
		fnAPL_CheckDeviceAction(DEV_CDU);
		Delay_Msg(2000);										// Sensor Wait(2000ms)
		fnAPL_CheckDeviceAction(DEV_CDU);
	}
	else
		fnAPL_CheckDeviceAction(nDevId);

	fnAPL_CheckMaterial();
	fnAPL_SetSensorInfo(TRUE);			// Update CST Information [#2116] NH KSK 2012.01.11

	if (fnAPL_GetAvailErrorDevice(nDevId))		return FALSE;
	else										return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Un Used
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_UnUsed(int nMode)
{
	NHDEBUG(1, (_T("mode: [%S] \n"), GETSTR_ATMMODE(nMode)));

	fnAPL_SetProcCount(' ');

// [#2468] AU KSK 2017.01.18 전국가 적용
//#if (CA_VERSION || US_VERSION || MX_VERSION || AU_A_VERSION)	// [#302] [MX] KSK 2008.6.17
//																// [#2115] MX KSK 2012.02.05
//																// [#2434] AU Kook 2016.07.28 Support Multi Languages
	fnSCR_SetCurrentLangMode(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE));
//#else
//	fnSCR_SetCurrentLangMode(ENG_MODE);
//#endif		// end of [#302]
// end of [#2468]

	if(fnMCU_IsEmvEnable())								// [#397] EMV option Enable 상태(IC 거래 상태)
		fnAPL_DeviceEnDisable(DEV_MCU, DISABLE, TRUE);	// [#397] wait flag를 true로 설정.
	else
		fnAPL_DeviceEnDisable(DEV_MCU, DISABLE, FALSE);

	// [#2325] NH KSK 2015.01.20
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
	{
		fnRFID_EntryDisable();
		fnAPL_CheckDeviceAction(DEV_RFID);
	}
	// end of [#2325]

	fnAPL_DeviceEnDisable(DEV_PIN, DISABLE, FALSE);

	// [#2205] US KSK 2013.07.02 내부검사 지적사항 대응 OUT OF SERVICE인 경우 MCU Flicker가 꺼지지 않는 Bug Fix (NH2600 Only)
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
	{
		SetHaloLedControl(HALOLED_COLOR_CYAN, OFF_HALOLED_MODE, OFF_MCULED);
	}
	// [#GLDV-2890]
	else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX5400")
	{
		// turn off EPP VB flicker (=> turn on CDU VB flicker only)
		if (nMode == ATM_CLERK)
			SetGuideLight(0x00, CDU_VB_FLICKER);
	}
	// end of [#GLDV-2890]
	else
	{
		// EPP LED GUIDE 깜박이는 현상 대응을 위해 수정
		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
		{
			if (nMode == ATM_CLERK || nMode == ATM_READY)
				fnAPL_DeviceSetFlicker(DEV_MCU|DEV_JPR|DEV_CDU, FLICKER_OFF);
			else
				fnAPL_DeviceSetFlicker(DEV_MAIN, FLICKER_OFF);
		}
		else
		{
			fnAPL_DeviceSetFlicker(DEV_MAIN, FLICKER_OFF);
		}
	}
	// end of [#2205]

	// [#GLDV-3005] 2022.01.18 Support Side Car
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
		fnAPL_DeviceSetFlicker(DEV_BCR | DEV_BNA, FLICKER_OFF);
	// end of [#GLDV-3005]

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Hot key EnDisable
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_EnDisableHotKey(int nEnDisable)
{
	NHDEBUG(1, (_T("Mode=[%d]\n"), nEnDisable));

	m_strHotKey = "";

	if(nEnDisable == ENABLE)
	{
		fnPIN_EntryEnable(PINMODE_NORMAL,						// nEnableMode
							0,									// nMinKeyCount
							0,									// nMaxKeyCount : 2004.11.05
							FALSE,								// bAutoEnd
							PIN_ALL_PAD,						// PIN_NUM_ALL_PAD,	// szActiveKeys
							L"",								// szTerminatorKeys
							L"",								// szCardData
							K_WAIT);							// nTimeout
	}
	else														//DISABLE
	{
		fnPIN_EntryDisable();
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Device EnDisable
/////////////////////////////////////////////////////////////////////////////
// [#397] [NH] psc 2008.08.01 nWaitFlag 를 3번째 파라미터로 바꿈.(4번째 이후부터는 default 파라미터가 주로 사용되지 때문에)
int CDevCmn::fnAPL_DeviceEnDisable(int nDevId, int nAction, int nWaitFlag, int nPinInputMode, LPCTSTR szCardData,  int nPinPassWordMin, int nPinPassWordMax, int nPinPassWordAuto, LPCTSTR szPinPassWordTerm, LPCTSTR szEnabledKey)
{
	NHDEBUG(1, (_T("[D:%S, A:%s, nPinInputMode(%d), szCardData(%s), pwMin(%d), pwMax(%d), pwAuto(%d), pwTerm(%s), nWaitFlag(%d), szEnabledKey(%s)] \n"), 
															GET_DEVNAME_BY_ID(nDevId),
															nAction == ENABLE ? L"EN" : L"DIS",
															nPinInputMode, 
															szCardData, 
															nPinPassWordMin, 
															nPinPassWordMax, 
															nPinPassWordAuto, 
															szPinPassWordTerm, 
															nWaitFlag,
															szEnabledKey));

	int		nTempDevId = DEV_NONE;

	nDevId = fnAPL_GetAvailDevice(nDevId);
	
	nTempDevId = nDevId & DEV_MAIN;
	fnAPL_CheckDeviceAction(nTempDevId);

	if (nDevId & DEV_MCU)
	{
		if(fnMCU_IsEmvTransaction())		// [#397] [NH] psc 2008.08.01 EMV(IC) 거래일 경우
		{
			if( nAction == ENABLE )
				fnMCU_ICEntryEnable(0);
			else
				fnMCU_ICEntryDisable();
		}
		else								// MS 거래일 경우
		{
			if (nAction == ENABLE)
				fnMCU_EntryEnable();
			else 
				fnMCU_EntryDisable();
		}
	}

	if (nDevId & DEV_PIN)
	{
		m_strHotKey = L"";
		if (nAction == ENABLE)
		{
			switch (nPinInputMode)
			{
				case PIN_DISABLE_MODE:
					fnPIN_EntryDisable();
					break;
				case PIN_MENU_MODE:
					{
					CString strActiveKey = CString(szEnabledKey);
					
					if(strActiveKey.GetLength() == 0)
						strActiveKey = _T("1,2,3,4,5,6,7,8,9,0,.,00,000,CANCEL,CLEAR,ENTER,F1,F2,F3,F4,F5,F6,F7,F8");

					fnPIN_EntryEnable(PINMODE_NORMAL,		// nEnableMode
										0,					// nMinKeyCount
										0,					// nMaxKeyCount : 2004.11.05
										FALSE,				// bAutoEnd
										strActiveKey,		// szActiveKeys
										L"",				// szTerminatorKeys
										L"",				// szCardData
										K_WAIT);			// nTimeout
					}
					break;					
				case PIN_PASSWORD_MODE:
					if (fnSNS_GetEnhancedAudio())
					{
						fnPIN_EntryEnable(PINMODE_EPP,			// nEnableMode
											nPinPassWordMin,	// nMinKeyCount
											nPinPassWordMax,	// nMaxKeyCount
											nPinPassWordAuto,	// bAutoEnd
											PIN_ADA2_PAD,		// szActiveKeys
											szPinPassWordTerm,	// szTerminatorKeys
											szCardData,			// szCardData
											K_WAIT);			// nTimeout
					}
					else
					{
						fnPIN_EntryEnable(PINMODE_EPP,			// nEnableMode
											nPinPassWordMin,	// nMinKeyCount
											nPinPassWordMax,	// nMaxKeyCount
											nPinPassWordAuto,	// bAutoEnd
											PIN_PASSWORD_PAD,	// szActiveKeys
											szPinPassWordTerm,	// szTerminatorKeys
											szCardData,			// szCardData
											K_WAIT);			// nTimeout
					}
					break;
				case PIN_AMOUNT_MODE:
					// ADA Jack Detected 2006.02.26
					if (fnSNS_GetEnhancedAudio())
					{
						fnPIN_EntryEnable(PINMODE_NORMAL,		// nEnableMode
											0,					// nMinKeyCount
											0,					// nMaxKeyCount : 2004.11.05
											FALSE,				// bAutoEnd
											PIN_ADA2_PAD,		// szActiveKeys
											L"",				// szTerminatorKeys
											L"",				// szCardData
											K_WAIT);			// nTimeout
					}
					else
					{
						fnPIN_EntryEnable(PINMODE_NORMAL,		// nEnableMode
											0,					// nMinKeyCount
											0,					// nMaxKeyCount : 2004.11.05
											FALSE,				// bAutoEnd
											PIN_AMOUNT_PAD,		// szActiveKeys
											L"",				// szTerminatorKeys
											L"",				// szCardData
											K_WAIT);			// nTimeout
					}
					break;
				// [#2530] US Justin 2018.01.24
				case PIN_AMOUNT_FTN_MODE:
					// ADA Jack Detected 2006.02.26
					if (fnSNS_GetEnhancedAudio())
					{
						fnPIN_EntryEnable(PINMODE_NORMAL,		// nEnableMode
											0,					// nMinKeyCount
											0,					// nMaxKeyCount : 2004.11.05
											FALSE,				// bAutoEnd
											PIN_ADA2_PAD,		// szActiveKeys
											L"",				// szTerminatorKeys
											L"",				// szCardData
											K_WAIT);			// nTimeout
					}
					else
					{
						fnPIN_EntryEnable(PINMODE_NORMAL,		// nEnableMode
											0,					// nMinKeyCount
											0,					// nMaxKeyCount : 2004.11.05
											FALSE,				// bAutoEnd
											PIN_AMOUNT_FTN_PAD,	// szActiveKeys
											L"",				// szTerminatorKeys
											L"",				// szCardData
											K_WAIT);			// nTimeout
					}
					break;
				// End of [#2530]
				case PIN_ALL_MODE:
					// ADA Jack Detected 2006.02.26
					if (fnSNS_GetEnhancedAudio())
					{
						fnPIN_EntryEnable(PINMODE_NORMAL,		// nEnableMode
											0,					// nMinKeyCount
											0,					// nMaxKeyCount : 2004.11.05
											FALSE,				// bAutoEnd
											PIN_ADA_PAD,		// szActiveKeys
											L"",					// szTerminatorKeys
											L"",					// szCardData
											K_WAIT);			// nTimeout
					}
					else
					{
						fnPIN_EntryEnable(PINMODE_NORMAL,		// nEnableMode
											0,					// nMinKeyCount
											0,					// nMaxKeyCount : 2004.11.05
											FALSE,				// bAutoEnd
											PIN_ALL_PAD,		// szActiveKeys
											L"",					// szTerminatorKeys
											L"",					// szCardData
											K_WAIT);			// nTimeout
					}
					break;

				default:
					break;
			}
		}
		else 
		{
			fnPIN_EntryDisable();
		}
	}

	if (nWaitFlag)
		return (fnAPL_CheckDeviceAction(nTempDevId));
	else 
		return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// Device Set Flicker
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_DeviceSetFlicker(int nDevId, int nAction)
{
	NHDEBUG(1, (_T("[0x%08x|%S] '%S' \n"), nDevId, GET_DEVNAME_BY_ID(nDevId), GET_FLICKER_STATUS(nAction)));

	//CString strTemp;
	//strTemp.Format(L"F:%S/%d", GET_DEVNAME_BY_ID(nDevId), nAction);
	//NVDump('F', 'E', "05", L"", strTemp);

	if (nDevId & DEV_SPR)
		fnLGT_SetFlicker(SPR_FLICKER, nAction);
	if (nDevId & DEV_MCU)
		fnLGT_SetFlicker(MCU_FLICKER, nAction);
	if (nDevId & DEV_CDU)
		fnLGT_SetFlicker(CDU_FLICKER, nAction);
	if (nDevId & DEV_PIN)
		fnLGT_SetFlicker(PIN_FLICKER, nAction);
	// [#GLDV-3005] US Kook 2022.01.17 Support Side Car
	if (nDevId & DEV_BCR)	// DEV_BCR for COD flicker, test purpose
		fnLGT_SetFlicker(SIDECAR_COD_FLICKER, nAction);
	if (nDevId & DEV_BNA)
		fnLGT_SetFlicker(SIDECAR_BNA_FLICKER, nAction);
	// end of [#GLDV-3005]

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Get Device Event
/////////////////////////////////////////////////////////////////////////////
int CDevCmn::fnAPL_GetDeviceEvent(int nDevId, int nEventKind)
{
	int TranDev = DEV_NONE;

	TranDev = fnCMN_ScanDeviceAction(nDevId, 0, nEventKind);
	if (nDevId & TranDev)
	{
//NHDEBUG(1, (_T("***DevApl***CDevCmn::fnAPL_GetDeviceEvent() nDevId(%d) TranDev(%d) \n"), nDevId, TranDev));
//		DeviceTran = TranDev; [#2325] NH KSK 2015.01.22 불필요 변수 삭제
		return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Slip Header Image Print
/////////////////////////////////////////////////////////////////////////////
#if 0
int CDevCmn::fnAPL_ShuSlipImagePrint(int nWaitFlag)
{
NHDEBUG(1, (_T("***DevApl***CDevCmn::fnAPL_ShuSlipImagePrint() \n")));

	fnAPL_CheckDeviceAction(DEV_SPR);
	fnSPR_Print(CMD_PRINT_HEADER_IMAGE);				// [#2373] US JUSTIN Change Variable Name

	if (nWaitFlag)
		return (fnAPL_CheckDeviceAction(DEV_SPR));
	else 
		return TRUE;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Clerk Information Make
/////////////////////////////////////////////////////////////////////////////
CString	CDevCmn::fstrAPL_ClerkInformationMake(int nIndex)
{

	CStringArray	strnTempArray;
	CString			strnTempClerkInformation("");
	CString			strTemp("");
	char			gDate[9]	= "";
	char			gTime[7]	= "";
	char			CrtDate[20]	= "";
	char			CrtTime[20]	= "";

	switch (nIndex)
	{
/////////////////////////////////////////////////////////////////////////////
		case 300:
			if (CSTCnt >= 1)
			{
				// [#2270] AU KSK 2014.05.26 Note Counting Disable일 경우 AP 정보를 return한다.
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
					strnTempClerkInformation.Format(L"%d", FirstCSTCnt);
				else
					strnTempClerkInformation.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP));
				// end of [#2270]
			}
			else					
			{
				strnTempClerkInformation.Format(L"N/A");
			}
			break;

		case 301:
			// 301:CST2
			if (CSTCnt >= 2)
			{
				// [#2270] AU KSK 2014.05.26 Note Counting Disable일 경우 AP 정보를 return한다.
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
					strnTempClerkInformation.Format(L"%d", SecondCSTCnt);
				else
					strnTempClerkInformation.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST2_CURRENTCOUNT_AP));
				// end of [#2270]
			}
			else
			{
				strnTempClerkInformation.Format(L"N/A");
			}
			break;

		case 302:
			// 302:CST3
			if (CSTCnt >= 3)
			{
				// [#2270] AU KSK 2014.05.26 Note Counting Disable일 경우 AP 정보를 return한다.
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
					strnTempClerkInformation.Format(L"%d", ThirdCSTCnt);
				else
					strnTempClerkInformation.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST3_CURRENTCOUNT_AP));
				// end of [#2270]
			}
			else
			{
				strnTempClerkInformation.Format(L"N/A");
			}
			break;

		case 303:
			// 303:CST4
			if (CSTCnt >= 4)
			{
				// [#2270] AU KSK 2014.05.26 Note Counting Disable일 경우 AP 정보를 return한다.
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
					strnTempClerkInformation.Format(L"%d", FourthCSTCnt);
				else
					strnTempClerkInformation.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST4_CURRENTCOUNT_AP));
				// end of [#2270]
			}
			else
			{
				strnTempClerkInformation.Format(L"N/A");
			}
			break;

/////////////////////////////////////////////////////////////////////////////

		case 800:	// SYSTEM SETUP
			// 800:RMS RING COUNT (사용안함)
			break;

		case 801:
			// 801:CLOCK SETUP
			GetDateTime(gDate, gTime);
			sprintf_s(CrtDate, sizeof(CrtDate), "%4.4s%2.2s%2.2s", &gDate[0], &gDate[4], &gDate[6]);
			sprintf_s(CrtTime, sizeof(CrtTime), "%2.2s%2.2s", &gTime[0], &gTime[2]);
			strnTempClerkInformation.Format(L"%S%S", CrtDate, CrtTime);
			break;

		case 803:
			// 803:LANGUAGEENDISABLE (ENGLISH)
			// [#2166] NH KSK 2012.12.17 Multi Language 지원
			strnTempClerkInformation.Format(L"%s", fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID(MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_ENGMODE))));
			// end of [#2166]
			break;

		case 804:
			// 804:LANGUAGEENDISABLE (SPANISH)
			// [#2166] NH KSK 2012.12.17
			strnTempClerkInformation.Format(L"%s", fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID(MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SPNMODE))));
			// end of [#2166]
			break;
			
		case 805:
			// 805:LANGUAGEENDISABLE (FRENCH)
			// [#2166] NH KSK 2012.12.17
			strnTempClerkInformation.Format(L"%s", fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID(MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_FRNMODE))));
			// end of [#2166]
			break;

		case 806:
			// 806:LANGUAGEENDISABLE (KOREAN)
			// [#2166] NH KSK 2012.12.17
			strnTempClerkInformation.Format(L"%s", fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID(MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_KORMODE))));
			// end of [#2166]
			break;

		case 807:
			// 807:LANGUAGEENDISABLE (JAPANESE)
			// [#2166] NH KSK 2012.12.17
			strnTempClerkInformation.Format(L"%s", fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID(MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_JAPMODE))));
			// end of [#2166]
			break;

		case 808:
			// 804:LANGUAGEENDISABLE (CHINESE)
			// [#2166] NH KSK 2012.12.17
			strnTempClerkInformation.Format(L"%s", fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID(MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_CHNMODE))));
			// end of [#2166]
			break;

/////////////////////////////////////////////////////////////////////////////
		case 1000:
			// 1000:DISPENSE LIMIT
			strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT);
			strnTempClerkInformation.Format(L"%d", Asc2Int(strTemp)/100);
			break;

		case 1001:
			// 1001:LOW CURRENCY CHECK
			// [#2166] NH KSK 2012.12.17
			strnTempClerkInformation.Format(L"%s", fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID(MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_LOWCRCYCHECK))));
			break;

		// [#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA
		case 1002: 
			// 1002:DENOMINATION SELECT
			strnTempClerkInformation.Format(L"%s", fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID(MemGetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DENOMINATION_SELECT_ENABLE))));
			break;
		// end of [#RWC6-12, #2584]

		// [#RWC6-27]
		case 1003: 
			// 1003:DISPENSE STYLE
			strnTempClerkInformation.Format(L"%s", GET_DISPENSE_STYLE_STRING(MemGetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DISPENSE_STYLE)));
			break;
		// end of [#RWC6-27]

/////////////////////////////////////////////////////////////////////////////
		// [#268] [NH] KSK 2008.6.9
		// FastCash 처리 LOGIC 수정 (string -> DWORD)
		case 1100:
		case 1101:
		case 1102:
		case 1103:
		case 1104:
		case 1105:
			{
				int fastCashIndex = nIndex - 1100 + _MEM_VAR_APP_FASTCASH1;
				int fastCashValue = MemGetInt(_MEM_FLD_APP_ATMINFO, fastCashIndex);
				if (fastCashValue == 0)
				{
					strnTempClerkInformation.Format(fstrSCR_GetStringFromTextID(T_DISABLE));
				}
				else
				{
					strnTempClerkInformation.Format(_T("%d"), MemGetInt(_MEM_FLD_APP_ATMINFO, fastCashIndex));
				}
				break;
			}
/////////////////////////////////////////////////////////////////////////////
		case 1200:
			// 1200:CST1(VVV)
			if (CSTCnt >= 1)
				strnTempClerkInformation.Format(L"%d", FirstCSTValue);
			else
				strnTempClerkInformation = "N/A";
			break;

		case 1201:
			// 1201:CST2(VVV)
			if (CSTCnt >= 2)
				strnTempClerkInformation.Format(L"%d", SecondCSTValue);
			else
				strnTempClerkInformation = "N/A";
			break;

		case 1202:
			// 1202:CST3(VVV)
			if (CSTCnt >= 3)
				strnTempClerkInformation.Format(L"%d", ThirdCSTValue);
			else
				strnTempClerkInformation = "N/A";
			break;
			
		case 1203:
			// 1203:CST4(VVV)
			if (CSTCnt >= 4)
				strnTempClerkInformation.Format(L"%d", FourthCSTValue);
			else
				strnTempClerkInformation = "N/A";
			break;
		default:
			break;
	}
	
	return strnTempClerkInformation;
}

// [#2150] US Justin 2012.10.04 Make common PIN Building function
int	CDevCmn::fnAPL_BuildPinBlockWithAccountNo(CString sAccountNo, int nMasterKeyName)	// [#2181] US Justin 2013.03.12 Add Mastey Key type Parameter
{
	int keymode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnAPL_BuildPinBlockWithAccountNo(%s,%d) \n"),sAccountNo, nMasterKeyName));
	if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) >= 0)
	{
		// Key Mode 1, 5
		if (keymode == KEYMODE_NON_UNIQ_DDES || keymode == KEYMODE_NON_UNIQ_SDES_MACING)
		{
			m_pMwi->PinBuildPinBlock(sAccountNo,  NULL, 15, _T("ISO0"), SINGLEDESKEY_WORKINGKEY_NAME, NULL);
		}
		else if (keymode == KEYMODE_NON_UNIQ_TDES || keymode == KEYMODE_TDES_MACING	|| keymode == KEYMODE_TDES_TMACING)
		{
			// [#2181] US Justin 2013.03.12 Add Mastey Key type Parameter
			if( nMasterKeyName == MASTERKEY_ATM)  // [#2185] US Justin 2013.05.06 MasterKeyName Definition
				m_pMwi->PinBuildPinBlock(sAccountNo,  NULL, 15, _T("ISO0"), TRIPLEDESKEY_WORKINGKEY_NAME, NULL);
			else if( nMasterKeyName == MASTERKEY_DUALHOST)
				m_pMwi->PinBuildPinBlock(sAccountNo,  NULL, 15, _T("ISO0"), TRIPLEDESKEY_2ND_WORKINGKEY_NAME, NULL);
			else if( nMasterKeyName == MASTERKEY_LIBERTYX) // [#RWC6-59] US William 2019.10.08 LibertyX
				m_pMwi->PinBuildPinBlock(sAccountNo,  NULL, 15, _T("ISO0"), LIBERTYX_WORKINGKEY_NAME, NULL);
			else // Default to DUAL host key 
				m_pMwi->PinBuildPinBlock(sAccountNo,  NULL, 15, _T("ISO0"), TRIPLEDESKEY_2ND_WORKINGKEY_NAME, NULL);

			// end of [#2181]
		}
		else if (keymode == KEYMODE_TR31 || keymode == KEYMODE_TR31_MACING || keymode == KEYMODE_TR31_TMACING)
		{
			if( nMasterKeyName == MASTERKEY_ATM)  // [#2185] US Justin 2013.05.06 MasterKeyName Definition
				m_pMwi->PinBuildPinBlock(sAccountNo,  NULL, 15, _T("ISO0"), TR31_ATM_WORKINGKEY_NAME, NULL);
			else if( nMasterKeyName == MASTERKEY_DUALHOST)
				m_pMwi->PinBuildPinBlock(sAccountNo,  NULL, 15, _T("ISO0"), TR31_DUALHOST_WORKINGKEY_NAME, NULL);
			else if( nMasterKeyName == MASTERKEY_LIBERTYX) // [#RWC6-59] US William 2019.10.08 LibertyX
				m_pMwi->PinBuildPinBlock(sAccountNo,  NULL, 15, _T("ISO0"), TR31_LIBERTYX_WORKINGKEY_NAME, NULL);
			else // Default to ATM host key 
				m_pMwi->PinBuildPinBlock(sAccountNo,  NULL, 15, _T("ISO0"), TR31_ATM_WORKINGKEY_NAME, NULL);
		}
	}
	else
	{
		m_pMwi->PinBuildPinBlock(sAccountNo,  NULL, 15, _T("ISO0"), NULL, NULL);
	}
	return TRUE;
}
// End of [#2150]

// [#2382] US Justin 2015.12.11 Generate Mobile APP QR Code
BOOL CDevCmn::fnAPL_MakeMobileAppQRCode(int nType)
{
	NHDEBUG(1, (L"Type (%d)\n", nType));
	const unsigned char encrypt_table[256] = 
	{
		0x39, 0x56, 0x5F, 0xD3, 0xBC, 0x67, 0xA7, 0xCF, 
		0x8B, 0xEC, 0x7A, 0x6F, 0xA1, 0x0D, 0x31, 0x3A, 
		0xB5, 0xB0, 0xED, 0x57, 0xA6, 0x48, 0x24, 0xFD, 
		0x1C, 0xED, 0x59, 0x2A, 0x6D, 0xB2, 0x2A, 0x98, 
		0xBD, 0x33, 0x90, 0xED, 0x6A, 0x6D, 0x63, 0x31, 
		0xA7, 0xD3, 0x1C, 0x86, 0x66, 0x9E, 0xDF, 0x13, 
		0x7C, 0x64, 0xB9, 0x42, 0xFF, 0xEC, 0x26, 0x2B, 
		0xF3, 0x41, 0xD6, 0x50, 0x26, 0x05, 0xB0, 0x8A, 
		0x59, 0x08, 0x18, 0x43, 0xA0, 0x19, 0x6D, 0xE6, 
		0x0C, 0x1C, 0xD6, 0x94, 0x86, 0x5D, 0x3F, 0x19, 
		0xFE, 0x24, 0x9E, 0x1E, 0xC6, 0x8A, 0x7B, 0x9F, 
		0x38, 0x8A, 0xAF, 0xA0, 0x9F, 0x5B, 0x6C, 0x1C, 
		0x55, 0xFD, 0x7D, 0x3F, 0x28, 0x13, 0xCF, 0xD5, 
		0x02, 0xF0, 0x31, 0x01, 0xCA, 0xF4, 0x58, 0x32, 
		0x85, 0x19, 0x26, 0x53, 0xC1, 0xC9, 0x2C, 0x42, 
		0x33, 0xF3, 0x6D, 0x84, 0x9F, 0x5D, 0x64, 0x37, 
		0xF8, 0x3C, 0x49, 0x47, 0xC9, 0x02, 0x91, 0xE5, 
		0xD3, 0x77, 0xB4, 0x35, 0xF7, 0x0B, 0x32, 0x47, 
		0x57, 0x6A, 0xD8, 0x49, 0xB9, 0x52, 0x3F, 0xFB, 
		0x2C, 0xA1, 0x97, 0x63, 0xED, 0xB3, 0xA2, 0xC1, 
		0x8B, 0xEA, 0x03, 0xC5, 0x4A, 0x8E, 0xB9, 0x00, 
		0xC5, 0xD8, 0xE7, 0x99, 0xD8, 0x49, 0xD6, 0x41, 
		0xBD, 0x40, 0x3C, 0x68, 0x75, 0xCC, 0xBF, 0xB1, 
		0x6A, 0xBF, 0xB5, 0xA4, 0x52, 0x04, 0x2E, 0xA4, 
		0x56, 0x32, 0x34, 0x20, 0x74, 0x60, 0x51, 0x0D, 
		0x22, 0x3C, 0x52, 0x93, 0x34, 0x57, 0x4B, 0x06, 
		0xFF, 0xC3, 0xDB, 0x19, 0xBF, 0xE0, 0xB1, 0x4E, 
		0x36, 0x73, 0x4F, 0xB1, 0x96, 0xF8, 0x0E, 0xC6, 
		0xA1, 0x3A, 0x63, 0xC0, 0x0F, 0x20, 0x60, 0xF3, 
		0x30, 0xCA, 0x7E, 0x8C, 0xD2, 0xDC, 0x99, 0x80, 
		0x65, 0x1A, 0x3C, 0xC2, 0x5D, 0x36, 0x1F, 0xB9, 
		0xD8, 0xE5, 0xEE, 0xF1, 0x81, 0x38, 0x4C, 0x11
	};

	CString strQRImgFile = MOBILEAPP_QR_IMAGEFILE;
	if(nType == MOBILEAPP_QR_TYPE_ERRORCODE)
		strQRImgFile = MOBILEAPP_QR_IMAGEERROR;			

	DeleteFile(strQRImgFile);						// Delete Existing File

	CString strQRContents, strTemp;
	int i;

	// HEADER "HYO" + Message Type (00:Register, 01:Error Code, 02:CashBalance)
	strQRContents.Format(L"HYO%02d", nType);

	// [#2489] US Justin Update MoniMobile Message Format - 2017.06.14
	// Message Version (Ex: "00")
	//strQRContents += L"00";
	if(nType == MOBILEAPP_QR_TYPE_REGISTER)
		strQRContents += L"02";					// [#2521] NH Justin 2017.12.11 "01" => "02" MoniMobile : Add value added function enable status 
	else if(nType == MOBILEAPP_QR_TYPE_ERRORCODE)
		strQRContents += L"01";
	else if(nType == MOBILEAPP_QR_TYPE_CASHBALANCE)
		strQRContents += L"00";
	// End of [#2489]

	// Terminal ID
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	if(strTemp.GetLength() <1)
		strTemp = L" ";								// Default 1 Space if Terminal ID is not set
	strTemp.Replace(L"\\", L"\\\\");
	strTemp.Replace(L"~", L"\\~");
	strQRContents += strTemp;

	// Separator
	strQRContents += L"~";

	if(nType == MOBILEAPP_QR_TYPE_REGISTER)
	{
		// Serial Number
		strQRContents += MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_ATMSERIALNO);
		strQRContents += L"~";

		// ATM Model
		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND);
		if(strTemp==L"NH2700L")	strTemp = "NH2700";
		strQRContents += strTemp;
		strQRContents += L"~";

		// Number of Cassette and CDU EP Version number.
		strTemp.Format(L"%02d%s~", fnCDU_GetNumberOfCST(), MemGetVersion(_MEMKEY_EPVERSION, L"CDM"));
		strQRContents += strTemp;

		// MCR Type(00:NHDIP, 01:Magtek, 02:Sankyo, 03:Motorized) and EP Version number.
		// [#2390] US Justin 2016.01.25 MoniMobile Add Device Type(MCR,EPP)
		//strTemp.Format(L"%02d%s~", 1, MemGetVersion(_MEMKEY_EPVERSION, L"IDC"));
		int nDevType = 0;												// NH DIP (Default)	
		if (GetDeviceType(L"MCU") == 3)				nDevType = 1;		// Magtek DIP
		else if (GetDeviceType(L"MCU") == 4)		nDevType = 2;		// Sankyo DIP
		strTemp.Format(L"%02d%s~", nDevType, MemGetVersion(_MEMKEY_EPVERSION, L"IDC"));
		// End of [#2390]
		strQRContents += strTemp;

		// PIN Pad Type (00:Old, 01:Visa, 02:PCI1.0, 03:PCI2.0, 04:PCI3.0) and EP version
		// [#2390] US Justin 2016.01.25 MoniMobile Add Device Type(MCR,EPP)
		//strTemp.Format(L"%02d%s~", 0, MemGetVersion(_MEMKEY_EPVERSION, L"PIN"));
		nDevType = 4;													// PCI 3.0 (Default)
		CString strEPPFWVer = MemGetVersion(_MEMKEY_EPVERSION, L"PIN");
		CString strEPPMajorVer = strEPPFWVer.Mid(1, 2);
		const CString pci2MajorVersion = CString(PIN_PCI20_EP_VERSION).Mid(1, 2);
		const CString pci3MajorVersion = CString(PIN_PCI30_EP_VERSION).Mid(1, 2);
		const CString pci5MajorVersion = CString(PIN_PCI50_EP_VERSION).Mid(1, 2);
		if( strEPPFWVer.CompareNoCase(PIN_VISA_EP_VERSION) < 0 )		nDevType = 0;	// Pre-Visa EPP
		else if( strEPPFWVer.CompareNoCase(PIN_PCI_EP_VERSION) < 0 )	nDevType = 1;	// Visa EPP
		else if( strEPPFWVer.CompareNoCase(PIN_512K_EP_VERSION) < 0 )	nDevType = 2;	// PCI V1.0
		else if( strEPPFWVer.CompareNoCase(PIN_PCI20_EP_VERSION) < 0 )	nDevType = 3;	// PCI V1.0 512K
		else if( strEPPMajorVer.CompareNoCase(pci2MajorVersion) == 0 )	nDevType = 4;	// PCI V2.0 
		else if( strEPPMajorVer.CompareNoCase(pci3MajorVersion) == 0 )	nDevType = 5;	// PCI V3.0
		else if( strEPPMajorVer.CompareNoCase(pci5MajorVersion) == 0 )	nDevType = 6;	// PCI V5.0
		else															nDevType = 7;	// Unknown
		strTemp.Format(L"%02d%s~", nDevType, MemGetVersion(_MEMKEY_EPVERSION, L"PIN"));
		// End of [#2390]
		strQRContents += strTemp;

		// Location 1 ~ 4
		strTemp = L"";
		for(i=0; i<4; i++)
		{
			strTemp += MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1 + i);
			strTemp += CString(FIELD_DELIMITER);		// Temporary Separator....
		}
		strTemp.Replace(L"\\", L"\\\\");
		strTemp.Replace(L"~", L"\\~");
		strTemp.Replace(CString(FIELD_DELIMITER), L"~");
		strQRContents += strTemp;

		// Application Version Number
		strQRContents += MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_AP_VERSION);
		strQRContents += L"~";

		// [#2489] US Justin 2017.06.14 Update MoniMobile Messgage 
		// Message Version "01"
		// Communication Type (01:Dial, 02:TCPIP)
		if( MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP )		nDevType = 1;		// Dial UP
		else																					nDevType = 2;		// TCPIP
		strTemp.Format(L"%02d~", nDevType);
		strQRContents += strTemp;
		// End of [#2489]

		// [#2521] NH Justin 2017.12.11 MoniMobile : Add value added function enable status
		// Message Version "02"
		// Value added services	8 digit
		//	DCC Single Host			Bit1	0x01
		//	DCC Multi-Host			Bit2	0x02
		//	Deprecated feature		Bit3	0x04
		//	Deprecated feature		Bit4	0x08
		//	Just Cash				Bit5	0x10
		//	Card Free Cash			Bit6	0x20
		//	Mastercard Cash Pick-Up	Bit7	0x40
		//	Dual Balance			Bit8	0x80

		BYTE bt1, bt2, bt3, bt4;
		bt1 = bt2 = bt3 = bt4 = 0;

		// Single Host DCC 
		if (MemGetInt( _MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC)==ENABLE)
			bt1 |= 0x01;

		// Dual Host DCC
		if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) == ENABLE )
			bt1 |= 0x02;
		
		// Just.Cash
		#if(APP_JUST_CASH)
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE)!=0 )
				bt1 |= 0x10;
		#endif

		// Paypal Cardless Cash
		#if(APP_PAYDIANT_CCA)
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE)==ENABLE )
				bt1 |= 0x20;
		#endif

		// Pin4 : Master Card Cash Pick-Up
		#if (APP_PIN4_CASHPICKUP)
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE)==ENABLE )
				bt1 |= 0x40;
		#endif

		// Dual Balance
		if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE)==ENABLE )
			bt1 |= 0x80;

		strTemp.Format(L"%02X%02X%02X%02X~", bt4, bt3, bt2, bt1);
		strQRContents += strTemp;
		// End of [#2521]	
	}
	else if(nType == MOBILEAPP_QR_TYPE_ERRORCODE)
	{
		// Current Error Code without Bracket and Separator
		strTemp.Format(L"%s~", fstrAPL_GetErrorCode(0) );
		strQRContents += strTemp;

		// Time of Error (14 bytes) + Separator
		strTemp.Format(L"%-8s%-6s~", GetDate(), GetTime() );
		strQRContents += strTemp;
		
		// Stacked Error Code (5 digits(frequency) + error code(maximum 7digits) )
		int nErrorCount = __min(GetErrSumTotalCount(), 10);				// Maximum 10 : because screen resolution
		for(i=0; i<nErrorCount; i++)
		{
			strTemp.Format(L"%05d%s~", GetErrSumStackCount(i), GetErrSumErrCode(i) );
			strQRContents += strTemp;
		}

		// [#2409] US Justin 2016.03.28 Add TimeStamp for FrequentErrorCodes for MobileApp
		// Message Version "01"
		// Start TimeStamp for Frequent Error Code --- Message Version "01" Addition (2017.06.14)
		if( nErrorCount > 0)
		{
			CString strErrSumDate = GetErrSumDate();
			strTemp.Format(L"%04s%04s%06s~", strErrSumDate.Right(4), strErrSumDate.Left(4), GetErrSumTime() );
			strQRContents += strTemp;
		}
		// End of [#2409]
	}
	else if(nType == MOBILEAPP_QR_TYPE_CASHBALANCE)
	{
		// Data Buffer for Actual Data
		BYTE szBuffer[256] = {};
		int nDataLen = 0;

		// Fill Index : Random Number 0 ~ 255
		BYTE nEncrypIndex = 0;

		// Encryption is so bad here anyway, the inability to generate a random index is probably just as secure
		RAND_bytes(&nEncrypIndex, 1);

		sprintf_s( (char*)(szBuffer + nDataLen), sizeof(szBuffer) - nDataLen, "%03d", nEncrypIndex);
		nDataLen += 3;

		// Number of Cassette
		sprintf_s( (char*)(szBuffer + nDataLen), sizeof(szBuffer) - nDataLen, "%02d", fnCDU_GetNumberOfCST());
		nDataLen += 2;

		// Fill Denomination and Bill Count of each cassette
		for(i=0; i< fnCDU_GetNumberOfCST(); i++)
		{
			sprintf_s( (char*)(szBuffer + nDataLen), sizeof(szBuffer) - nDataLen, "~%05d%05d", fnCDU_GetValueOfCash(CDU_CST_1+i), fnCDU_GetNumberOfCash(CDU_CST_1+i) );
			nDataLen += 11;
		}

		// Encrypt data
		int nEncId = nEncrypIndex;
		for(i=3; i<nDataLen; i++)
		{
			szBuffer[i] ^= encrypt_table[nEncId];
			nEncId++;
			if(nEncId>=256)
				nEncId = 0;
		}

		// Base 64 Encoding

		// Length of encoded data is (4 * (nChars / 3)) - nPaddingChars
		long encodeBufferLen = 4 * (nDataLen /3);
		// Buffer for Encoded String
		char *szData = new char[encodeBufferLen + 1]();
		int nDataSz = base64_encode((char*)szBuffer, nDataLen, (char*) szData, encodeBufferLen + 1);

		// Copy the encoded string
		WCHAR* wchTmp = new TCHAR[nDataSz + 1]();
		MultiToWide((LPWSTR) wchTmp, (LPCSTR) szData, nDataSz);

		strQRContents.Append(wchTmp);

		memset(wchTmp, 0, sizeof(WCHAR) * (nDataSz + 1));
		memset(szData, 0, encodeBufferLen + 1);
		delete [] wchTmp;
		delete [] szData;
	}
	else if(nType == MOBILEAPP_QR_TYPE_TERMINALID)
	{
		strQRContents = MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID);
		strQRContents.TrimLeft();
		strQRContents.TrimRight();
	}

	NHDEBUG(1, (L"QR Content = [%s]\n", strQRContents));

	MakeQRCodeImageFile(strQRContents, strQRImgFile);
	if (IsExistFile(strQRImgFile))
	{
		NHDEBUG(1, (L"Found QR Image file [%s] : RETURN TRUE\n", strQRImgFile));
		return TRUE;
	}

	NHDEBUG(1, (L"ERROR on Finding QR Image file [%s] : RETURN FALSE\n", strQRImgFile));
	return FALSE;
}

void CDevCmn::fnAPL_DisplayQRCodeOnScreen(CString strQRFileName)
{
	fnSCR_SetDisplayData(L"APTranAdv", L"1");				// Number of Advertisement
	fnSCR_SetDisplayData(L"APTranAdvTime", L"3600");		// Refresh Time (Second) => refresh every hour. (60*60)
	fnSCR_SetDisplayData(L"APTranAdv1", strQRFileName);		// First Image
}
// End of [#2382]