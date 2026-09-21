/////////////////////////////////////////////////////////////////////////////
//	DevCmn.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

//#define NH_DEBUG

#define	DBG_CALL		1
#define DBG_INFO		1

#include ".\Common\CmnLib.h"
#include ".\Common\NHDbgApi.h"
#include ".\Common\ConstDef.h"

//#include ".\Common\MB2500DLL.h"			// [#573] NH AIREAT 2009.10.26 SW NVRAM
#include ".\Dev\DevDefine.h"
#include ".\Dll\nhmwi.h"
#include ".\Net\Network.h"
#include ".\Dev\DevCmn.h"
#include ".\Dll\NHReadiniFile.h"

#include ".\Common\ScreenDef.h"		// [#257] [MX] KSK 2008.6.5
#include ".\Scr\ScrCtrl.h"			// [#257] [MX] KSK 2008.6.10

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int	linetype = LINE_DIALUP;
int	flowtype = VISAII_FLOW;

/////////////////////////////////////////////////////////////////////////////
//	CDevCmn Construction/Destruction
/////////////////////////////////////////////////////////////////////////////
CDevCmn::CDevCmn() : m_eWaitObject(FALSE, FALSE)
{
	m_pNetWork	= NULL;

	m_pCurScrCtrl = NULL;

	for (int i = 0; i < SCR_LOCATION_MAX; i++)
		m_arScrCtrl[i] = NULL;

	m_pConfig = GetConfigFuncPointer();


	// [#GLDV-3005] US Kook 2022.04.14 execute SIUInit() to read side car door sensor. couldn't find specific reason it was limited to NH2600/MX5400
	// [#2205] US KSK 2013.06.28 NH2600인 경우에만 SIUInit을 수행함
	//if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600"
	//	|| MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX5400")		// [#GLDV-2890] US Kook 2021.05.21 MX5400 VB Flickers
	{
		SIUInit();
		SetGuideLight(0x00, 0x00);
	}
	// end of [#2205]

#if SUPPORT_CHANGE_PARAMETER_JNL 
	pCurrentNVRam2 = NULL;
	pCurrentNVRam2	= (LPNVRAM2_ADDR)GetSystemAddr(2, 4096);

	pCurrentNVRam3 = NULL;
	pCurrentNVRam3	= (LPNVRAM3_ADDR)GetSystemAddr(3, 4096);

	pCurrentNVRam6 = NULL;
	pCurrentNVRam6	= (LPNVRAM6_ADDR)GetSystemAddr(6, 4096);

	pCurrentNVRam8 = NULL;
	pCurrentNVRam8	= (LPNVRAM8_ADDR)GetSystemAddr(8, 4096);

#endif

	m_pTarScreenAsset = NULL;
	m_bExecuteResetMP = false;	// [#RWC6-94] NH Kook 2020.01.31 Easing MPW Clear
	
}

CDevCmn::~CDevCmn()
{

	// [#2205] US KSK 2013.06.28 NH2600인 경우에만 SIUDeInit을 수행함
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600"
		|| MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX5400"		// [#GLDV-2890] US Kook 2021.05.21 MX5400 VB Flickers
		|| MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)		// [#GLDV-3005] US Kook 2022.04.13 support side car
	{
		SIUDeInit();
	}
	// end of [#2205]

#if SUPPORT_CHANGE_PARAMETER_JNL 

	FreeSystemAddr((LPVOID)pCurrentNVRam2);
	FreeSystemAddr((LPVOID)pCurrentNVRam3);
	FreeSystemAddr((LPVOID)pCurrentNVRam6);
	FreeSystemAddr((LPVOID)pCurrentNVRam8);
	pCurrentNVRam2 = NULL;
	pCurrentNVRam3 = NULL;
	pCurrentNVRam6 = NULL;
	pCurrentNVRam8 = NULL;
#endif

	for (int i = 0; i < SCR_LOCATION_MAX; i++)
	{
		if (m_arScrCtrl[i] != NULL)
		{
			CScrCtrl::DestroyScreenEngine(m_arScrCtrl[i]);
			m_arScrCtrl[i] = NULL;
		}
	}

	if (m_pNetWork != NULL)
		delete m_pNetWork;

}


BEGIN_MESSAGE_MAP(CDevCmn, CWnd)
	//{{AFX_MSG_MAP(CDevCmn)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//	CDevCmn message handlers
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//	FUNCTION
/////////////////////////////////////////////////////////////////////////////
// Set TAR Asset
void CDevCmn::SetTarAsset(void* asset)
{
	m_pTarScreenAsset = static_cast<CAssetArchive*>(asset);
}

// Set Owner
void CDevCmn::SetOwner(CNHMWI* pMwi)
{
	m_pMwi		= pMwi;

#ifdef APP_LOCAL_MODE
	// LOCAL일 경우 MASTER PASSWORD ERROR를 방지하기 위해 '777777'으로 SETTING함
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MASTERPW, L"777777");
	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_HEALTHYFLAG, 0);
	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSFLAG, 0);
#endif

	// [#2220] NH KMK 2014.01.27 APP_SHOW_MODE 관련 로직 미사용하므로 주석처리
// #ifdef APP_SHOW_MODE
// 	CString strShowID = MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID);
// 	if (strShowID == L"00000000" || strShowID.GetLength() <= 0)
// 		MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID, L"NH201002");
// #endif
	// end of [#2220]

	//////////////////////////////////////////////////////////////////////////
	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_IDLE);
	///////////////////////////////////////////////////////////////////////////

	// [#127] KSK 2008.04.21
	// AP / RMS / MWI / VDM OS Version을 NVRAM에 Write한다.
	MemSetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_AP_VERSION,	AP_VERSION);
	// [#286] [NH] KSK 2008.6.12
	CString	strTemp, strRMSVersion;
	strTemp.Format(_T("%06x"), RMS_VERSION);
	strRMSVersion.Format(_T("V%2.2s.%2.2s.%2.2s"),strTemp.Left(2), strTemp.Mid(2,2), strTemp.Right(2));
	MemSetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_RMS_VERSION,	strRMSVersion);
	// end of [#286]

	// [#2321] NH KSK 2015.01.14 AP Revision 표시를 위해 MWI 버전 활용 (V06.03.xx -> xx AP Revision
//	MemSetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_MWI_VERSION,	MWI_VERSION);
	CStringArray strTempArray;
	strTemp.Format(L"%d,%d,%d,%d", AP_DETAIL_VERSION);							// AP DETAIL VERSION : x,x,x,x
	SplitString(strTemp, ",", strTempArray);
	int nAPRevision = Asc2Int(strTempArray.GetAt(strTempArray.GetSize()-1));		// 'x,x,x,x' 중 마지막 x가 revision

#if (AU_VERSION)	// [#2424] AU KSK 2016.05.12
	int nDomesticVerInfo = 0;
	nDomesticVerInfo = LoadAUDomesticBinFromFile(L"\\ATM\\DomesticBin.dat", m_arrAUDomesticBinList);
	strTemp.Format(L"%s.%01d%01d.%02d", MWI_VERSION, nDomesticVerInfo, 3, nAPRevision);		// "3" means MWI Real Version
	MemSetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_MWI_VERSION,	strTemp);
#else
	// end of [#2379]
	strTemp.Format(L"%s.%02d", MWI_VERSION, nAPRevision);
	MemSetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_MWI_VERSION,	strTemp);
#endif				// end of [#2424]

	MemSetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_VDM_VERSION,	_T("N/A")); //[#568] SOOK 2009.09.23 전국가 공통 적용으로 N/A로 변경
	MemSetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_OS_VERSION,	RegGetStr(L"SOFTWARE\\ATM", L"OSVersion"));
	// end of [#127]

	// [#465] [NH] KSK 2008.12.19 이전 Version 호환성을 위해 Convert한다.
	// update 정보 및 INI에서 읽어와서 Convert하는 부분에 대한 처리가 문제임
	// 1. ATMIP INFO
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_IPADDR, Convert_IpAddressInfo(MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_IPADDR)));
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_SUBNET, Convert_IpAddressInfo(MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_SUBNET)));
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_GATEWAY, Convert_IpAddressInfo(MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_GATEWAY)));
	// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
	//MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DNS, Convert_IpAddressInfo(MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DNS)));
	if (MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DNS).GetLength() <= 0)
		MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DNS,_T("8.8.8.8"));
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ATM_DNS_2ND).GetLength() <= 0)
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ATM_DNS_2ND,_T("8.8.4.4"));
	// end of [#RWC6-10]

	// [#486] [NH] KSK 2009.1.19 URL일 경우 IP가 사라지는 Bug Fix
	// 2. HOST IP INFO
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ISHOSTUSEURL) == 0)
	{
		MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1NAME, Convert_IpAddressInfo(MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1NAME)));
		MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2NAME, Convert_IpAddressInfo(MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2NAME)));
	}

	// 3. RMS IP INFO
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ISRMSUSEURL) == 0)
		MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSNAME, Convert_IpAddressInfo(MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSNAME)));
	// end of [#465]
	// end of [#486]

	Set_NetworkInformation();
	
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == 2)
	{
		AtmSetIpAddress((LPCTSTR)MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_IPADDR));
		AtmSetSubnetMask((LPCTSTR)MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_SUBNET));
		AtmSetGateWay((LPCTSTR)MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_GATEWAY));

		// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
		//AtmSetDNS((LPCTSTR)MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DNS));
		AtmSetDNS((LPCTSTR)MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DNS), (LPCTSTR)MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ATM_DNS_2ND));
		// end of [#RWC6-10

		AtmSetDHCP(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DHCP));
		
		#ifndef APP_LOCAL_MODE
				ReloadLanCard();
		#endif
	}

	// [#268] [NH] KSK 2008.6.9
	// FASTCASH 기존 VERSION과 호환성을 위해 Convert 작업을 함
	if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASHDUMMY) != (WORD) 0xFFFF)
	{
		// FastCash Convert 수행
		CStringArray	strArrayFastCash;
		int				i = 0;
		strTemp.Format(_T("%s"), GetOldFastCashValue());
		SplitString(strTemp, ",", strArrayFastCash);

		if (strArrayFastCash.GetSize() == CDU_CST_MAXFASTCASH)
		{
			for(i=0; i<CDU_CST_MAXFASTCASH; i++)
				MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH1 + i, Asc2Int(strArrayFastCash[i]));
		}
		else
		{
			// FASTCASH가 6개가 아닌경우 값이 비정상인 경우이므로
			// Default값으로 SET
			// [#437] [NH] KSK 2008.10.17
			MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH1,			FASTCASH1_DEFAULT);
			MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH2,			FASTCASH2_DEFAULT);
			MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH3,			FASTCASH3_DEFAULT);
			MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH4,			FASTCASH4_DEFAULT);
			MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH5,			FASTCASH5_DEFAULT);
			MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH6,			FASTCASH6_DEFAULT);
			// end of [#437]
		}
		MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASHDUMMY, 0xFFFF);
	}
	// end of [#268]

// [#323] [NH] KSK 2008.6.23 국가별 NVRAM 보완처리

// [#2468] AU KSK 2017.01.18 전국가 적용
//#if !(CA_VERSION || US_VERSION || MX_VERSION || AU_A_VERSION)	// [#2115] MX KSK 2012.02.05	// [#2434] AU Kook 2016.07.27 Support Multi Language
	// DEFAULT LANGUAGE - ENGLISH
//	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE, ENG_MODE);
//#endif
// end of [#2468]

#if !(US_VERSION || MX_VERSION)		// [#441] [US] KSK 2008.11.03	// [#2115] MX KSK 2012.02.05
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CONTINUETRANS_ENABLE, 0);		// CONTINUE TRANS AFTER PRE-BALANCE
#endif

#if !(US_VERSION || MX_VERSION)		// PRE-BALANCE	// [#2115] MX KSK 2012.02.05
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PREBALANCE, 0);
#endif

	////////////////////////////////////////////////////
	// [#2136] US PCS 2012.05.21
	// AP가 올라올 때 Terminal ID를 MD5로 수정한 값이 맞지 않으면 NOT_USED
	// 유상화한 고객에 대해서는 Table 모드로 시작한다.
#if !(US_VERSION || CA_VERSION || AU_VERSION)		// PERCENTAGE SURCHARGE		// [#2220] AU KMK 2014.02.04 AU 추가
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE, NOT_USED); // [#2136] US PCS 2012.05.30 
#else
	IsValidSurchargeTable();				// [#2389] US Justin Change Function Name (Prev:"IsValidCheckTerminalID();")
#endif
	//end of [#2136]

#if (US_VERSION || CA_VERSION || AU_VERSION)	// 2008. 11.04 KSK Bug Fix (Percent 값이 0일때 최소값으로 set하도록 수정)
	// [#2316] NH Justin 2014.12.17 Decimal point surcharge
	//if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_PERCENT) == 0)
	//	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_PERCENT, 1);
	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE) == 0 )
	{
		int nExist = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_PERCENT);		// Copy from exist value
		if( (nExist>0)&&(nExist<=99) )
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE, nExist*100);
		else
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE, 100);	// Minimum 1.00 %
	}
	// Do not reset the original value just in case of downgrade.
	// End of [#2316]

	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_SURCHARGE_WARNINGLOCATION, 1);		// [#2150] US Justin 2012.10.03 "% surcharge Warning" => Present "After" for US and Canada

#endif

// [#571] NH KSK 2009.10.05 Credit Option 공통 처리
#if (US_VERSION || CA_VERSION )
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING, 1);
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS, 1);
	// [#RWC6-70] PAI Needs Dual Balance for Credit
	#if (APP_CUSTOM_PAI)
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD, 1);
	#endif
	// end of [#RWC6-70]
#elif(AU_VERSION) // [#2031] NZ KJW 2011.03.16 //[#570] SOOK 2009.09.24 호주 Checking만 Default로	// [#2069] NH KSK 2011.06.13
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING, 1);
#else
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING, 1);
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS, 1);
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD, 1);
#endif

#if !(AU_VERSION)
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_INQUIRY_SURCHARGE, 0);
#endif
// end of [#323]

	// [#2468] AU KSK 2017.01.24
	// [#365] [NH] KSK 2008.7.15
	// Language 관련 국가별 En/Disable 설정
	//#if (AU_C_VERSION)		// [#571] NH KSK 2009.10.05 UK_VERSION 삭제
	//						// [#2069] NH KSK 2011.06.13
	//						// [#2434] AU Kook 2016.07.27 Support Multi Language (blocking AU -> AU_C)
	//	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ENGMODE,			1);
	//	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SPNMODE,			0);
	//	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FRNMODE,			0);
	//#endif
	// end of [#2468]

	// [#2479] CA Justin 2017.04.11 Support Spanish
	/*
	#if (CA_VERSION)	// KSK 2011.07.18 내부검사 지적사항 대응
		MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SPNMODE,			0);
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE) == SPN_MODE)
			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE, ENG_MODE);
	#elif (MX_VERSION)
		// [#2115] MX KSK 2012.02.05
		MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FRNMODE,			0);
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE) == FRN_MODE)
			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE, ENG_MODE);
		// end of [#2115]
	#endif				// end of KSK 2011.07.18
	*/

	#if (MX_VERSION)
		MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FRNMODE,			0);
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE) == FRN_MODE)
			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE, ENG_MODE);
	#endif
	// End of [#2479]

	// KSK 2010.02.02 Journal Available Field �߰�
	// [#2037] AU KSK 2011.03.31	
	// [#2457] US Justin 2016.12.09 Support Moniview Journal Uploading
	// [#2500] CA Justin 2017.08.22 Support Moniview Journal Uploading
#if (AU_VERSION || US_VERSION || CA_VERSION)	
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_AVAILABLE, 1);
#else
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_AVAILABLE, 0);
#endif

	// [#2166] NH KSK 2012.11.13 Supervisor Language Default Setting
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SUPERVISOR_LANGUAGE) == 0)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SUPERVISOR_LANGUAGE, ENG_MODE);
	// end of [#2166]

// [#2202] NH Justin 2013.06.20 Assign Default DCC Disclaimer
	if( (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER1).GetLength() == 0)&&
		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER2).GetLength() == 0)&&
		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER3).GetLength() == 0)&&
		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER4).GetLength() == 0)&&
		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER5).GetLength() == 0)&&
		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER6).GetLength() == 0) )
	{

		// [#2499] US Justin 2017.08.21 Open Dual Host DCC for All US Customer
		/*																					//1234567890123456789012345678901234567890  	
		#if (APP_CUSTOM_PAI)														
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER1, L"I have chosen not to use the MasterCard" );
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER2, L" currency conversion process and agree" );
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER3, L"  that I will have no recourse against" );
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER4, L"   MasterCard concerning the currency" );
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER5, L"     conversion or its disclosure." );
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER6, L"" );
		#else
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER1, L"I understand that MasterCard has a " );
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER2, L"currency conversion process and I" );
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER3, L"will have no recourse against" );
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER4, L"MasterCard with respect to any matter" );
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER5, L"related to the currency conversion or" );
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER6, L"disclosure thereof." );
		#endif
		*/
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER1, L"I have chosen not to use the MasterCard" );
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER2, L" currency conversion process and agree" );
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER3, L"  that I will have no recourse against" );
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER4, L"   MasterCard concerning the currency" );
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER5, L"     conversion or its disclosure." );
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER6, L"" );
		// End of [#2499]
	}
// End of [#2202]

// [#2360] NH KSK 2015.12.23 호주는 DISCLAIMER 미사용이므로 무조건 해당 값 초기화 하도록 로직 추가
#if (AU_VERSION)
	MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER1, L"" );
	MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER2, L"" );
	MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER3, L"" );
	MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER4, L"" );
	MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER5, L"" );
	MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER6, L"" );
#endif
// end of [#2360]

// [#2283] NH Justin. Initializing Hot Key String
#if (US_VERSION || CA_VERSION || MX_VERSION)
	if(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HOT_KEY_STRING).GetLength() == 0)
	{
		NHDEBUG(1, (_T("HOTKEY CHECK - ERROR (Hot key is not stored in NVRAM) => Restore Factory default\n") ) );
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HOT_KEY_STRING, DEFAULT_OPMODE_HOTKEY );
	}
#else
	MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HOT_KEY_STRING, DEFAULT_OPMODE_HOTKEY );
#endif
// End of [#2283]

	//[#2320] US Justin 2015.01.14 Support TLS, Change Version scheme : ABOVE => UPTO
	int nSSLVer = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSLVERSION);
	if ( nSSLVer != CONN_UPTO_SSL_V30
	  && nSSLVer != CONN_UPTO_TLS_V10
	  && nSSLVer != CONN_UPTO_TLS_V11 
	  && nSSLVer != CONN_UPTO_TLS_V12
	  && nSSLVer != CONN_UPTO_TLS_V13)
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSLVERSION,		CONN_UPTO_TLS_V13);			// Default UP_TO_TLSV1.3
	}
	// End of [#2320]

#if (US_VERSION)
	// [#RWC6-31] PAI Needs to Change Default Configuration Setting
	// uncomment below for #RWC6-31
	//[Default for PAI]
	//Terminal Status: Enabled
	//Reversal on Host Error: Enabled
	//Reason for Reversal: Enabled
	//Health Check: Enabled
	//Interval: 2 Hours
	//Check Balance: Enabled
	//Continue: Enabled
	//Dual Balance: Enabled
	//CCA: Enabled by default (Standard 1 Only)

	// [#RWC6-145] US William 2020.030.6 Disable reboot defaults
	//#if (APP_CUSTOM_PAI)
	//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TERMINAL_STATUS_FIELD_ENABLE, ENABLE);
	//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REVERSAL_AT_HOSTERROR, ENABLE);
	//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REASONFORREVERSAL_ENABLE, ENABLE);
	//	MemSetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_HSDELAYINT, 2);
	//	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_HEALTHYFLAG, ENABLE);
	//	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PREBALANCE, ENABLE);
	//	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CONTINUETRANS_ENABLE, ENABLE);
	//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE, ENABLE);
	//	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
	//		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE, ENABLE);
	//#endif
	//uncomment above for #RWC6-31 
	// end of [#RWC6-31]

	// SET Dual Host DCC SSL Version 
	nSSLVer = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION, CONN_UPTO_TLS_V13);
	if( (nSSLVer!=CONN_UPTO_TLS_V12)&&(nSSLVer!=CONN_UPTO_TLS_V13) )
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION, CONN_UPTO_TLS_V13);

	// Assign Default PAI ROUTER Parameters (After NVRAM Clear  OR   AP Update from old versions
	if( (MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST1NAME).GetLength() == 0)&&
		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST2NAME).GetLength() == 0) )
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_ENABLE, 0);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ISHOSTUSEURL, 0);

		#if (APP_CUSTOM_PAI)		
			// Assign Default PAI Parameters (After NVRAM Clear  OR   AP Update from old versions)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, 0);				// DISABLE DUAL HOST DCC
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST1NAME, L"192.168.1.90" );
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST2NAME, L"192.168.1.90" );
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST1PORTNO, 5306);
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST2PORTNO, 5306);

			if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE) &&
				(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE) == DYNAMICFLOW_DISABLE ) )
			{
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE, DYNAMICFLOW_DUALHOST );			// USE DUAL HOST => Disable Stabdard1 Dynamic Flow
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION, DYNAMICFLOW_1STCALL_ON);	// DF 1st Call ON (Dual Host USE 1st Call)
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC, 0);							// Disable STD1 DF DCC
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE, 0);					// DIsable PIN Change
			}
			else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
			{
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION, DYNAMICFLOW_1STCALL_ON);	// DF 1st Call ON (Dual Host USE 1st Call)
			}
			
		#else
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, 0);						// Disable DUAL HOST DCC
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST1NAME,   MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1NAME) );
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST2NAME,	 MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2NAME) );
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST1PORTNO, MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1PORTNO));
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST2PORTNO, MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2PORTNO));
		#endif
	// End of [#2499]
	}

#if (APP_CUSTOM_PAI)
	// Verify PAI settings are aligned with FIS DCC settings
	if ((MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE) &&
		(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE) == DYNAMICFLOW_ENABLE))
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE, ENABLE);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC, ENABLE); // Host det.
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION, DYNAMICFLOW_1STCALL_OFF); // 1st call disabled
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE, DYNAMICFLOW_DCC_TYPE_BOTH);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION, DCC_CUSTOMOPTION_PLANET);
	}
#endif
#else	// Non US VERSION => Disable US Services

	// [#2499] US Justin 2017.08.21 Disable Dual Host connection for other countries.
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE) == DYNAMICFLOW_DUALHOST)			// DOES NOT SUPPORT DUAL HOST......
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE, DYNAMICFLOW_DISABLE);			// Disable STD1 DF
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC, 0);							// Disable STD1 DF DCC
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE, 0);					// DIsable PIN Change
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION, DYNAMICFLOW_1STCALL_OFF);// DF 1st Call OFF

		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, 0);						// Disable Dual Host DCC
	}
	// End of [#2499]

	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, 0);			// Disable Dual Host DCC
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_ENABLE, 0);			// Disable POP Money

//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_CONTINUE_FALLBACK, 0);		// [#2208] US Justin 2013.07.08 Add Fallback Continue Option (US Only)	[#2324] NH KSK 2015.01.16
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE, 0);				// [#2396] US Justin 2016.02.10 PAI PIN4
#endif

	// [#2499] US Justin 2017.08.23
	// Validate Dual Host / Dynamic Flow condition
	if(   MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
	{
		// [#2242] US Justin 2013.12.17 DF1st call option
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE) == ENABLE)						// USE Standard1 Dynamic Flow
		{
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) != 0 )
				MemSetInt( _MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, 0);			

			if( (MemGetInt( _MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION) == DYNAMICFLOW_1STCALL_OFF) &&
				(MemGetInt( _MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE) == 2) )	// Host Determine
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE, 0);		
		}
		else if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE) == DYNAMICFLOW_DUALHOST)	// USE Dual Host DCC
		{
			// Disable Standard1 DCC Services
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC) != 0 )
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC, 0);	

			// DF 1st Call ON (Dual Host(Standard1) USE Dynamic Flow 1st Call)
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION ) != DYNAMICFLOW_1STCALL_ON)	
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION, DYNAMICFLOW_1STCALL_ON);

			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE) == 2)		// Host Determine
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE, 0);
		}
		else																								// Standard1 Dynamic Flow is Disabled	
		{
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) != 0 )
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, 0);	

			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE) == 2)		// Host Determine
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE, 0);

			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC) != 0)
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC, 0);
		}
		// End of [#2242]
	}
	else if(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
	{
		if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) == ENABLE) &&
			(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION) != DYNAMICFLOW_1STCALL_ON) )
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION, DYNAMICFLOW_1STCALL_ON);			// DF 1st Call ON (Dual Host(STD1 DF) USE 1st Call)

#if (APP_CUSTOM_CIBC_MX)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION, DCC_CUSTOMOPTION_BANSI);
#endif
	}
	// End of [#2499]

	// [#2471] NH Justin 2017.02.01 Enable Pin4 and Popmoney to all customers
#if (APP_POPMONEY)
	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_MAXAMT) == 0)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_MAXAMT, 300);		// Max Dispense Amount
#else
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_ENABLE, 0);
#endif

#if (APP_PIN4_CASHPICKUP)
	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_MAXAMT) == 0)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_MAXAMT, 300);			// Max Dispense Amount
#else
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE, 0);
#endif
	// End of [#2471]

#if (AU_VERSION)
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION, DYNAMICFLOW_1STCALL_OFF); // [#2450] AU KSK 2015.07.20 AU는 1st call 미지원
#endif

	/* EMV 사용 설정.
	// 1. Device.dat 파일에 MagTek로 설정되어 있어야 함.
	// 2. 소스에서 EMV_LEVEL2 define이 1 로 설정되어 있어야함.
	// 3. 1, 2를 만족하면 _MEM_VAR_OPT_EMV_AVAILABLE 필드가 1로 셋팅된다.(조건이 만족되면 자동으로 셋팅된다. OP에서 셋팅하지 않는다.)
	// 4. 3이 만족될때, OP에서 EMV Enable로 설정해야 함.(_MEM_VAR_OPT_EMV_ENABLE 가 1로 셋팅됨)
	// 위의 모든 조건이 만족될때 EMV 거래가 가능한다.
	*/
#if (EMV_LEVEL2)	// [#397] [NH] psc 2008.08.01 EMV 사용가능 설정.
//	if(GetDeviceType(L"MCU") == MCU_MAGTEK_DIP)	// [#25] NH PSC '1': DepType, '3': Magtek
	if(GetDeviceType(L"MCU") == MCU_MAGTEK_DIP || GetDeviceType(L"MCU") == MCU_SANKYO_DIP)	// [#25] NH PSC '1': DepType, '3': Magtek, '4' Sankyo Dip	[#2073] NH KSK 2011.06.24
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_AVAILABLE, 1);	// Magtek 일때, EMV Available 을 1로 셋팅한다.

		#if !(AU_A_VERSION)	// [#2458] AU KSK 2016.12.09 Support Sankyo Latch Option
		if (GetDeviceType(L"MCU") == MCU_SANKYO_DIP)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_LATCH_ENABLE, 0);	// Sankyo는 무조건 Latch 사용으로 설정됨 (Option 없음)
		#endif				// end of [#2458]
	}
	else
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_AVAILABLE, 0);				// EMV not Available
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_ENABLE, 0);				// EMV Disable
	}
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_TRANSACTIONOPTION, 1);		// EMV Option Default Set KSK 2009.2.6	[#2082] CA KSK 2011.07.02 MS FIRST OPTION 제거
#else
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_AVAILABLE, 0);				// EMV not Available
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_ENABLE, 0);				// EMV Disable
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_TRANSACTIONOPTION, 1);		// EMV Option Default Set(MS First) KSK 2009.2.6	[#2082] CA KSK 2011.07.02
#endif	// end of [#397]

	if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSRINGCNT) == 0)
		MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSRINGCNT, 1);

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_YEAR) == 0)
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_ENABLE,		0);
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TYPE,			AUTO_DAY_TOTAL);				// [#416] NZ AIREAT 2008.09.08
#else
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TYPE,			AUTO_TRIAL_DAY_TOTAL);			// [#416] NZ AIREAT 2008.09.08
#endif
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_YEAR,	2000);				// YEAR		// [#416] NZ AIREAT 2008.09.08
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_MONTH,	01);				// Month	// [#416] NZ AIREAT 2008.09.08
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_DAY,		01);				// Day		// [#416] NZ AIREAT 2008.09.08
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_HOUR,	22);				// Hour		// [#416] NZ AIREAT 2008.09.08
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_MIN,		00);				// Minute	// [#416] NZ AIREAT 2008.09.08
		MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DAYTOTAL_LASTDATE,			L"20000101");		// YYYYMMDD	// [#416] NZ AIREAT 2008.09.08
	}

	// [#468][NH]2008.12.19 AIREAT NZ 소스 머지
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSMODEMINITIALSTRING).GetLength() == 0)
	{
//		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSMODEMINITIALSTRING, L"AT&F&G9&H4");			// AIREAT 2009.04.22 RMS Default값 Set 추가

#if (AU_VERSION)	// [#2102] AU KSK 2011.12.15 호주 MoniView Initial String 변경 (Customers에서 검증 완료)
//		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSMODEMINITIALSTRING, L"AT&F&G17&H0+GCI=09");
//         2014년 4월 호주 출장 시 해당값 변경 후 정상동작 확인 완료 (Nextpayments, DC Payments)
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSMODEMINITIALSTRING, L"AT&F+GCI=09");
#else
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSMODEMINITIALSTRING, L"AT&F&G9&H4");			// AIREAT 2009.04.22 RMS Default값 Set 추가
#endif				// end of [#2102]
	}
	// end of [#468]

	// [#476] [NH] KSK 2009.1.5
// 	// NVRAM에 기종정보가 없을 경우 Default값으로 SET한다.
// 	MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND, m_pConfig->GetMachineType());	// 국가별 Default 기종을 Set한다.
	
	// [#2097] US PCS 2011.11.14 (미국인경우 NH1800SE에 한해서 기종 설정 메뉴가 있으므로 설정 안함)
	// [#2128] US PCS 2012.03.13 "MX5000SE 신기종 추가에 따라서 NH2700T를 제외한 모든 모델은 수동 입력해야 함."
	// [#2180] NH Justin 2013.03.12  Add 5000SE to CA,MX as well
	// [#2267] US KSK 2014.05.12
	// [#2315] Justin 2014.12.12 Add MX5200SE. AP will assign Machine type automatically for 2700T, 1500SE, and 5200SE
	// [#2383] US Justin 2015.12.18 Support 5300SE (5200SE and 5300 SE have the same configuration)
	// [#2518] US Kook 2018.01.20 Support MX-2800SE, Auto Detecting Wyvern.

#if (US_VERSION || CA_VERSION || MX_VERSION)	
	// [#2559] NH Justin 2018.06.16 Correct ATM Types when a main board is swapped
	/*
	if ( (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND).IsEmpty())
		|| m_pConfig->GetMachineType() == L"NH2700T"
		|| m_pConfig->GetMachineType() == L"NH1500SE"
		|| m_pConfig->GetMachineType() == L"MX2800SE")
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND, m_pConfig->GetMachineType());
	*/

	// [#2571] NH Justin 2018.08.03 Fix Machine Type Setting Problem - 1024*600 Type ATMs
	/*
	CString strUniqueATMType = CString(ATM_TYPE_UNIQUE);
	CString strMType = m_pConfig->GetMachineType();
	if ( MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND).IsEmpty() || (strUniqueATMType.Find(strMType)>= 0) )
	{
		NHDEBUG(DBG_CALL, (_T("Machine Type is Empty or Special Type [%s]\n"), strMType));
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND, strMType);
	}

	CString strStoredType = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND);
	if( (strUniqueATMType.Find(strStoredType) >= 0) && (strStoredType != strMType) )
	{
		NHDEBUG(DBG_CALL, (_T("Stored Machine Type [%s] is Special but Actual Machine Type[%s] is not.\n"), strStoredType, strMType));
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND, strMType);
	}
	*/

	CString strMType = m_pConfig->GetMachineType();
	NHDEBUG(DBG_CALL, (_T("Detected Machine Type = [%s]\n"), strMType));
	if ( MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND).IsEmpty() || IsUniqueATMType(strMType) )
	{
		NHDEBUG(DBG_CALL, (_T("Machine Type is Empty or Special Type [%s]\n"), strMType));
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND, strMType);
	}

	// Fix when a unique ATM Board is used on "normal" ATM.
	CString strStoredType = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND);
	NHDEBUG(DBG_CALL, (_T("Stored Machine Type = [%s]\n"), strStoredType));
	if( IsUniqueATMType(strStoredType) && (strStoredType != strMType) )
	{
		NHDEBUG(DBG_CALL, (_T("Stored Machine Type [%s] is Unique but Actual Machine Type [%s] is not.\n"), strStoredType, strMType));
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND, strMType);
	}
	// End of [#2571]
	// End of [#2559]
#else
	MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND, m_pConfig->GetMachineType());
#endif
	//end of [#2097]
	// end of [#476]

	// [#2327] NH KSK 2015.02.06 모델명 변경으로 인해 보안 코드 삽입
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600SE")
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND, L"MX2600SE");
	// end of [#2327]

// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 (호주 CUSTOMERS만 적용함)
#if !(AU_VERSION)	// [#2069] NH KSK 2011.06.13
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AWARD_COUPON1_ENABLE, 0);
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AWARD_COUPON2_ENABLE, 0);
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AWARD_COUPON3_ENABLE, 0);
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AWARD_COUPON4_ENABLE, 0);
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AWARD_COUPON5_ENABLE, 0);
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AWARD_COUPON6_ENABLE, 0);
#endif 
//end of [#546] 

	// [#2074] NH KSK 2011.07.09 보완 처리
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_TIMETHRESHOLD) == 0)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_TIMETHRESHOLD, 30);		// [#2074] NH KSK 2011.07.09 SP Default값 적용
	// end of [#2074]

// [#620] US KSK 2010.02.22
#if (US_VERSION || CA_VERSION || MX_VERSION)	// [#2115] MX KSK 2012.02.05
	if (MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_AVAILABLE) == 0)
	{
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_AVAILABLE, ENABLE);
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_OPTION,	0);		// not use
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_PRINT_START_TIME, 9);	// default 9시 ~ 18시
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_PRINT_END_TIME, 18);
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON2_OPTION,	0);		// not use
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON2_PRINT_START_TIME, 9);	// default 9시 ~ 18시
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON2_PRINT_END_TIME, 18);
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON3_OPTION,	0);		// not use
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON3_PRINT_START_TIME, 9);	// default 9시 ~ 18시
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON3_PRINT_END_TIME, 18);
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON4_OPTION,	0);		// not use
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON4_PRINT_START_TIME, 9);	// default 9시 ~ 18시
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON4_PRINT_END_TIME, 18);
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON5_OPTION,	0);		// not use
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON5_PRINT_START_TIME, 9);	// default 9시 ~ 18시
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON5_PRINT_END_TIME, 18);
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON6_OPTION,	0);		// not use
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON6_PRINT_START_TIME, 9);	// default 9시 ~ 18시
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON6_PRINT_END_TIME, 18);
	}
#else
	MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_AVAILABLE, 0);
	MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_ENABLE,	0);
	MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_CUT_OPTION,	0);
	MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_OPTION,	0);
	MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON2_OPTION,	0);
	MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON3_OPTION,	0);
	MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON4_OPTION,	0);
	MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON5_OPTION,	0);
	MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON6_OPTION,	0);
#endif
// end of [#620]

// [#522] US KSK 2009.05.19
//#if (US_VERSION || CA_VERSION || MX_VERSION)	// [#2115] MX KSK 2012.02.05
#if (US_VERSION || CA_VERSION )					// [#2137] MX PCS 2012.07.25 "MX 국가 사양 제외."
	LoadBinFromFile(ATM2_EXTENDED_BIN_FILE, m_arrExtendedBinList);
#endif

//	/////////////////////////////////////////////////////////////////////////
// 	//[#2130] MX PCS 2012.03.26 "MX DP 특이 사양 지원." // [#2137] MX PCS 2012.07.25 "MX 국가 사양 제외."
// #if (MX_VERSION)
// 	LoadBinFromFile(L"\\ATM\\DomesticBinList.dat", m_arrDomesticBinList);
// #endif
// 	//end of [#2130] // end of [#2137]
// 	/////////////////////////////////////////////////////////////////////////

	// [#558] CA KSK 2009.08.19
//#if (CA_VERSION)	KSK 2010.06.10 공통 지원으로 변경
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_STANDARD1OPTION_AVAILABLE, ENABLE);
//#else
//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_STANDARD1OPTION_AVAILABLE, 0);		// KSK 2010.02.23 보완 코드
//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TERMINAL_STATUS_FIELD_ENABLE, 0);	// [#603] NH KSK 2010.01.06
//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REVERSAL_AT_HOSTERROR, 0);			// [#603] NH KSK 2010.01.06
//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REASONFORREVERSAL_ENABLE, 0);		// KSK 2010.02.18
//#endif
	// end of [#558]

// [#613] AU_C KSK 2010.01.18
// [#2037] AU KSK 2011.03.31	
// [#2457] US Justin 2016.12.09 Support Moniview Journal Uploading
// [#2500] CA Justin 2017.08.22 Support Moniview Journal Uploading
#if (AU_VERSION || US_VERSION  || CA_VERSION)	
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_YEAR) == 0)
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_ENABLE,	0);			// Default : Disable
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_OPTION,	1);			// Default : Monthly
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_COUNT,	1);			// Default : 1
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_YEAR,		2000);		// Year
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_MONTH,	01);		// Month
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_DAY,		01);		// Day
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_NEXT_DAY, 01);		// Next Day 2010.02.01
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_HOUR,		22);		// Hour
	}
	#if (AU_VERSION)
	// [#2042] AU KSK 2011.04.02
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == 0)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION, COUNTRY_AUS);
	#endif

#endif
// end of [#613]

// [#2115] MX KSK 2012.02.05
#if !(MX_VERSION)
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MULTI_CURRENCY_ENABLE, 0);
//	MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EXCHANGE_RATE, L"");	// [#2137] MX KSK 2012.07.25
#endif
// end of [#2115]

	// [#2067] NZ KSK 2011.06.08 뉴질랜드인 경우 NEED MORE TIME 미 지원하도록 수정
#if (AU_VERSION)
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_NZ)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME, 0);
#endif
	// end of [#2067]

	// [#528] AU 2008.09.06 호주 뉴질랜드만 적용 됨.
#if !(AU_VERSION)	// [#2069] NH KSK 2011.06.13
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME, 0);
#endif
	// end of [#528]

	// WINCE 6.0
	if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_DEFAULT_TYPE) == 0)
		MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_DEFAULT_TYPE,	1);

	// Touch가 지원되지 않는 Type은 무조건 Disable 처리
	if (!(m_pConfig->GetOSVersion() == NH_OS_NH2700_L || m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T))		// [#11] NH KSK 2010.10.1	[#2064] NH KSK 2011.05.18
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TOUCHVIBRATION_ENABLE, 0);

	// TOUCH VIBRATION값 설정 (TOUCH만 지원)
	if (m_pConfig->GetOSVersion() == NH_OS_NH2700_L || m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T)	// [#11] NH KSK 2010.10.1	[#2064] NH KSK 2011.05.18
		SetTouchVibration(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TOUCHVIBRATION_ENABLE));

	// [#2074] NH KSK 2011.06.25 Sankyo Dip이 아닌 경우 Anti Skimming Sensor Disable 처리
	if (GetDeviceType(L"MCU") != MCU_SANKYO_DIP)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_ENABLE, 0);
	// end of [#2074]

	// [#2085] NH KSK 2011.07.22
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_AUTORECOVERYTIME) == 0)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_AUTORECOVERYTIME, 5);
	// end of [#2085]

	// [#2205] US KSK 2013.07.01
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_DEFAULT) == 0)
	{
		// Default 설정이 안되어져 있으므로 Default값 설정하도록 수정
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_INSERVICE, FADE_HALOLED_MODE);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_INSERVICE, HALOLED_COLOR_CYAN);	// Fade인 경우에는 의미 없음
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_TRANSACTION, ON_HALOLED_MODE);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_TRANSACTION, HALOLED_COLOR_GREEN);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_DISPENSING, FLICKING_HALOLED_MODE);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_DISPENSING, HALOLED_COLOR_GREEN);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_OUTOFSERVICE, ON_HALOLED_MODE);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_OUTOFSERVICE, HALOLED_COLOR_PINK);

		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_DEFAULT, 1);
	}
	// end of [#2205]

	// [#2234] NH KMK 2013.11.21 MoniView Timeout
	// 기본값이 없는 경우
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_CONNECT_TCPIP) == 0)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_CONNECT_TCPIP, TIMEOUT_RMS_CONNECT_TCPIP);		// 30s
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_RECEIVE_TCPIP) == 0)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_RECEIVE_TCPIP, TIMEOUT_RMS_RECEIVE_TCPIP);		// 10s

	// 허용범위(10~99) 이외의 값이 있는 경우
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_CONNECT_TCPIP) < 10)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_CONNECT_TCPIP, TIMEOUT_RMS_CONNECT_TCPIP);		// 30s
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_RECEIVE_TCPIP) < 10)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_RECEIVE_TCPIP, TIMEOUT_RMS_RECEIVE_TCPIP);		// 10s
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_CONNECT_TCPIP) > 99)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_CONNECT_TCPIP, TIMEOUT_RMS_CONNECT_TCPIP);		// 30s
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_RECEIVE_TCPIP) > 99)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_RECEIVE_TCPIP, TIMEOUT_RMS_RECEIVE_TCPIP);		// 10s
	// end of [#2234]

	// [#2292] US Justin 2014.09.17 Cardtronics Additional Setting
#if( APP_TDL_OPTION )
	if(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE).GetLength() == 0)
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE,	L"0000");

	#if !(US_VERSION)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_SURCHARGE, 0);									// [#2437] US Justin Cardtronic AllPoint Surcharge Option
	#endif

	// [#2426] Justin 2016.05.23 Add STD1 Cardtronics Option
	if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION) == DCC_CUSTOMOPTION_GENERAL)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION, DCC_CUSTOMOPTION_CARDTRONICS);
	// End of [#2426]

	// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE, DYNAMICFLOW_DCC_TYPE_BOTH);	// default = 0 for Cardtronics, always
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOST_DCC_TYPE, DUALHOST_DCC_TYPE_BOTH);			// default = 0 for Cardtronics, always
	// end of [#RWC6-2, #2585]
#else
	// [#RWC6-29] Add ALLPOINT for PAI
	#if (APP_CUSTOM_PAI)
		#if !(US_VERSION)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_SURCHARGE, 0);									// [#2437] US Justin Cardtronic AllPoint Surcharge Option
		#endif								
	#else
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_TIRSURCHARGE, 0);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DCC, 0);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_SURCHARGE, 0);
	#endif
		// To add credit card segmentation for US general version as well.
		if(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE).GetLength() == 0)
		{
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE,	L"0000");
		}
	// end of [#RWC6-29]
	
	// [#2426] Justin 2016.05.23 Add STD1 Cardtronics Option
	if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION) == DCC_CUSTOMOPTION_CARDTRONICS)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION, DCC_CUSTOMOPTION_GENERAL);
	// End of [#2426]

	// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
	if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE) != DYNAMICFLOW_DCC_TYPE_MASTERONLY) &&
		(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE) != DYNAMICFLOW_DCC_TYPE_VISAONLY) )
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE, DYNAMICFLOW_DCC_TYPE_BOTH);	// default = 0 for general, if not specified
	
	if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOST_DCC_TYPE) != DUALHOST_DCC_TYPE_MASTERONLY) &&
		(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOST_DCC_TYPE) != DUALHOST_DCC_TYPE_VISAONLY) )
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOST_DCC_TYPE, DUALHOST_DCC_TYPE_BOTH);			// default = 0 for general, if not specified
	// end of [#RWC6-2, #2585]
#endif
	// End of [#2292]

// [#2444] US Justin 2016.09.01 Enable Dual Balance to All Customers.
#if !(APP_DUALBALANCE )
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE, 0);									
#endif
// end of [#2444]

	// [#2309]  US Justin 2014.11.14 Balance Inquiry No Fee Notice
#if !(US_VERSION)
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_BALANCE_NOFEE_NOTICE, 0);
#endif
	// End of [#2309]

	// [#2382] US Justin 2015.12.14 Support Mobile APP QR CODE
#if !(MONIMOBILE_QRCODE)
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MOBILEAPP_QRCODE, 0);
#endif
	// End of [#2382]

	// [#2325] NH KSK 2015.01.28 해상도가 1024*600 / 1024 *768이 아닌 경우 ap / op dat file작업이 안되어 해당 값 disable처리
#if (US_VERSION)
	//#ifndef APP_LOCAL_MODE
	//// if (GetSystemMetrics(SM_CXSCREEN) != 1024)	// [#RWC6-73] Remove NFC (MSD) Support (at least in US)
	//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE, 0);
	//#endif
#else	// 그 외 국가는 현재 ap.dat file 작업이 안되어 disable 처리
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE, 0);
#endif
	// end of [#2325]

	// [#2481] US Justin 2017.04.21 Fallback for unknown AID
	//#if !(US_VERSION)
	//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_4_UNKNOWNAID, 0);		// Non US VERSION : Fallback for by Default
	//#endif
	// end of [#2481]

	// [#2490] NH Justin 2017.06.15 Standard1 Dynamic Flow MAC Option
	// if Default value is not assigned
	// Assign 2(DISABLE) => CA and US    /     1(ENABLE) => AU and others
	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_STD1_DYNAMICFLOW_MAC) == 0 ) 
	{
		#if (CA_VERSION || US_VERSION)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_STD1_DYNAMICFLOW_MAC, 2);
		#else
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_STD1_DYNAMICFLOW_MAC, 1);
		#endif
	}
	// End of [#2490]

	// [#2472] US Justin 2017.02.07 DCC Withdrawal Options
	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_OPTION_MAXWITHDRAWAL) == 0 )
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_OPTION_MAXWITHDRAWAL, Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT))/100 );
	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_OPTION_FASTCASH1) == 0 )
	{
		for(int i=0; i<6; i++)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_OPTION_FASTCASH1+i, MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH1+i) );
	}
	// End of [#2472]

	// Denomination Select only available for multiple CSTs, or force disabling it
	// [#RWC6-12]
	//if (fnCDU_GetNumberOfCST() <= 1)
	//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DENOMINATION_SELECT_ENABLE, 0);
	// end of [#RWC6-12]

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_ENABLE_DISABLE) == 0)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_UPSELL, GIVEPAY_APPEAR_BOTH);	// Default = 0 = BOTH	// [#RWC6-1]

	// [#RWC6-27]
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_STYLE) == 0)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_STYLE, DISPENSE_STYLE_LEAST);	// Default = 1 = Least,  2 = Same, 3  = Balanced, 10 = Most
	// end of [#RWC6-27]

#if (AU_A_VERSION)	// [#2459] AU KSK 2016.12.23 Default는 기존 Mix 사용으로 설정
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_MIX_OPTION) == 0)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_MIX_OPTION, CDM_MIX_MINIMUM_NUMBER_OF_BILLS);	
#else
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_MIX_OPTION, CDM_MIX_MINIMUM_NUMBER_OF_BILLS);	
#endif				// end of [#2459]

	// [#2518] US Kook 2018.01.14 Disable camera functions except for MX-2800SE.	// [#GLDV-2505] Support MX-2800T
	if (!m_pConfig->SupportsCamera())
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE, 0);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ANTI_SHLDR_SURF_ENABLE, 0);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_MEMORIZE_ENABLE, 0);
	}

	// Validate index of camera image.
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_IMAGE_INDEX) < 1
		|| MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_IMAGE_INDEX) > NH_CAM_FACE_IMAGE_CNT_MAX)
	{
		// set initial index to 1
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_IMAGE_INDEX, 1);
	}
	// end of [#2518]

	// [#2538] NH Justin 2018.03.09 MoniMobile Cash Balance Option	(Change default(0) as Enable(1), Disble(2) ... Support Field Unit)
	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MONIMOBILE_CASHBALANCE) == 0 )	// Field Update or NVRAM Cleared
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MONIMOBILE_CASHBALANCE, 1);		// CONVERT "0" => "1" Enable
	// End of [#2538]

	// Disable No-Cash In Service mode for Canada
#if (!US_VERSION && !MX_VERSION)
	 MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_INSERVICE_WHENCSTSEMPTY, DISABLE);
	 MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_INSERVICE_WHENCSTSSHORT, DISABLE);
#endif

	// [#419] [NH] KSK 2008.9.16
	// ErrStack Version이 없을 경우 Clear한다
	if (strcmp(GetErrVersion(), NVRAM_ERRSTACK_VERSION))
	{
		fnAPL_ClearError();				// Current Error Clear
		ClearErrSum();					// Error Summary Clear
	}
	// end of [#419]

	CheckRejectTotalVersion();		// [#534] NH AIREAT 2009.06.03

	// DEFAULT FOLDER CREATE
	// [#156] KSK 2008.04.22 OS Version에 따른 PATH 지정
	strTemp.Format(_T("%s\\JNL"), JNLBACKUP_SRC_PATH);
	CreateDirectory(strTemp, NULL);
	strTemp.Format(_T("%s\\LOG"), JNLBACKUP_SRC_PATH);
	CreateDirectory(strTemp, NULL);
	// end of [#156]
	CreateDirectory(ADVERTISEMENT_PATH, NULL);

	// [#478] NH AIREAT 2009.01.07 : 광고 해상도별 지원 - 하위 호환성을 위해 광고를 새로운 경로로 이동한다.
	// 기존의 UPDATE 방식을 사용할수 있으므로 무조건 실행되어야 한다.
	WIN32_FIND_DATA	FileData;
	HANDLE			hSearch = INVALID_HANDLE_VALUE;		// [#2022] NH KSK 2011.02.22
	CString			strSearchFile;
	CString			strDestPath;
	
	// create Directory
	strDestPath.Format(_T("%s\\%d_%d"), ADVERTISEMENT_PATH, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	CreateDirectory(strDestPath, NULL);

	// make file name for search.
	strSearchFile.Format(_T("%s\\Adv_??.jpg"), ADVERTISEMENT_PATH);
	
	hSearch = FindFirstFile(strSearchFile, &FileData);
	if (hSearch != INVALID_HANDLE_VALUE)
	{
		CString			strSrcFileName, strDestFileName;

		while(1)
		{
			// copy file
			strSrcFileName.Format(_T("%s\\%s"), ADVERTISEMENT_PATH, FileData.cFileName);
			strDestFileName.Format(_T("%s\\%s"), strDestPath, FileData.cFileName);	
			CopyFile(strSrcFileName, strDestFileName, FALSE);

			// [#2110] NH KSK 2011.12.22 File Delete시에 Read Only File을 삭제하지 못하는 Bug Fix (Read Only->Read/Write로 변경)
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
				SetFileAttributes(strSrcFileName, FILE_ATTRIBUTE_NORMAL);
			// end of [#2110]

			DeleteFile(strSrcFileName);
								
			if (!FindNextFile(hSearch, &FileData))
				break;
		}
		
		FindClose(hSearch);
	}
	// end of [#478]

	//[#616] AU_C SOOK 2010.02.05 AD BRAND NAME
#if (AU_VERSION)	// [#2040] AU KSK 2011.03.31
	CString strFilePath;

	// [#2362] AU KSK 2015.08.07 Fixed Bug NH2700T do not show Brand Name for resolution.
	if (m_pConfig->GetOSVersion() == NH_OS_NH2700_F_T || m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T)
		strFilePath.Format(_T("%s\\%d_%d"), ADVERTISEMENT_PATH, 1024, 600);
	else
		strFilePath.Format(_T("%s\\%d_%d"), ADVERTISEMENT_PATH, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	// end of [#2362]

	strSearchFile.Format(_T("%s\\*_ADInfo.dat"), strFilePath);

	hSearch = FindFirstFile(strSearchFile, &FileData);
	if ( hSearch != INVALID_HANDLE_VALUE)
	{
		CFile datFile;
		CFileException e;
		CString strFileName;
		strFileName.Format(_T("%s\\%s"), strFilePath, FileData.cFileName);

		if(datFile.Open(strFileName, CFile::modeRead, &e))
		{

			ULONGLONG  nSize = datFile.GetLength();
			BYTE *pBuffer = new BYTE[10+1];
			memset(pBuffer, 0, 10+1);
			if( datFile.Read(pBuffer, 10) > 0 )
				MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AD_BRAND_NAME ,CString(pBuffer));
			else
				MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AD_BRAND_NAME ,L"");
			delete pBuffer;
			datFile.Close();
		}
	}
	else
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AD_BRAND_NAME ,L"");

#endif
	//end of [#616]

	m_JNLMgr.Initialize(this);
	m_ErrorCodeInfo.Initialize(this);					// [#] NH AIREAT 2008.07.21 ERROR CODE VIEW
	if (m_pTarScreenAsset)
		m_MultiString.SetTarAsset(m_pTarScreenAsset);
	m_MultiString.Initialize();
	
	fnAPL_InitializeDS();

	// [#2226] NH Justin 2013.10.22 Assign default EMV Kernel Version and Check Enabled AID
	int nEMVKernelVer = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_KERNELVERSION);
#if (CA_VERSION)	//  Default EMV Kernel Version for Canada : Version 7
	if ( (nEMVKernelVer != EMV_KERNEL_V4) && (nEMVKernelVer != EMV_KERNEL_V5) && (nEMVKernelVer != EMV_KERNEL_V6) && (nEMVKernelVer != EMV_KERNEL_V7) ) // [#RWC6-50] US William 2019.09.05 Add Kernel v7
	{
		//MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_KERNELVERSION, EMV_KERNEL_V4);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_KERNELVERSION, EMV_KERNEL_V7);						// [#RWC6-226] US William 2019.09.05	 Default Kernel V7
	}
#else				// Default EMV Kernel Version for All other countries : Version 5
	// [#2435] NH KSK 2016.07.14 V5 or V6 Setting
	// Default EMV Kernel Version for All other countries : Version 5
	//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_KERNELVERSION, EMV_KERNEL_V5);
	if ( (nEMVKernelVer != EMV_KERNEL_V5) && (nEMVKernelVer != EMV_KERNEL_V6) && (nEMVKernelVer != EMV_KERNEL_V7) ) // [#RWC6-50] US William 2019.09.05 Add Kernel v7
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_KERNELVERSION, EMV_KERNEL_V5);
	// end of [#2435]
#endif
	nEMVKernelVer = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_KERNELVERSION);
	fnEMV_SetKernelVersion(nEMVKernelVer);

	// [#2342] US Justin 2015.05.05 AID Selection....
	//if(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_AIDENDISABLE).GetLength() == 0)
	//{
	//	#if (CA_VERSION)	
	//		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_AIDENDISABLE, L"11111111111111111111111111111111111111111111111111111111111" );
	//	#else
	//		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_AIDENDISABLE, L"01111111111111111111111111111111111111111111111111111111111" );
	//	#endif
	//}

	CString strNewEMVEnDisable = _T(""); 
	CString strAIDSeparator = CString(AID_FIELD_DELIMITER);
	TCHAR chEnable;

	// Check Old AID Enable/Disable Status and convert to new structure..........
	CString strEMVEnDisable = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_AIDENDISABLE);
	if(strEMVEnDisable.GetLength() == 59)	
	{
		NHDEBUG(DBG_CALL, (_T("OLD EMV En/Disable parameter was used before [%s] => Start Converting \n"), strEMVEnDisable));

		// Interac
		chEnable = strEMVEnDisable.GetAt(0);
		if( chEnable == '1')		strNewEMVEnDisable += _T("A0000002771010")	+ strAIDSeparator;

		// VISA
		chEnable = strEMVEnDisable.GetAt(1);
		if( chEnable == '1')		strNewEMVEnDisable += _T("A0000000031010")	+ strAIDSeparator;

		// VISA ELECTRON
		chEnable = strEMVEnDisable.GetAt(2);
		if( chEnable == '1')		strNewEMVEnDisable += _T("A0000000032010")	+ strAIDSeparator;

		// VISA PLUS
		chEnable = strEMVEnDisable.GetAt(3);
		if( chEnable == '1')		strNewEMVEnDisable += _T("A0000000038010")	+ strAIDSeparator;

		// MasterCard
		chEnable = strEMVEnDisable.GetAt(4);
		if( chEnable == '1')		strNewEMVEnDisable += _T("A0000000041010")	+ strAIDSeparator;

		// Maestro
		chEnable = strEMVEnDisable.GetAt(5);
		if( chEnable == '1')		strNewEMVEnDisable += _T("A0000000043060")	+ strAIDSeparator;

		// Cirrus
		chEnable = strEMVEnDisable.GetAt(6);
		if( chEnable == '1')		strNewEMVEnDisable += _T("A0000000046000")	+ strAIDSeparator;

		if( fnEMV_GetKernelVersion() == EMV_KERNEL_V4)		// Canada Only
		{
			// AMEX
			chEnable = strEMVEnDisable.GetAt(7);
			if( chEnable == '1')		strNewEMVEnDisable += _T("A00000002501")	+ strAIDSeparator;
		}
		else
		{
			// PULSE D-PAS
			chEnable = strEMVEnDisable.GetAt(7);
			if( chEnable == '1')		strNewEMVEnDisable += _T("A0000001523010")	+ strAIDSeparator;

			// AMEX
			chEnable = strEMVEnDisable.GetAt(8);
			if( chEnable == '1')		strNewEMVEnDisable += _T("A00000002501")	+ strAIDSeparator;

			#if (US_VERSION || CA_VERSION)												// [#2491] US Justin 2017.06.28 Change Default AID List)	
				// US MAESTRO
				chEnable = strEMVEnDisable.GetAt(9);
				if( chEnable == '1')		strNewEMVEnDisable += _T("A0000000042203")	+ strAIDSeparator;

				// US VISA
				chEnable = strEMVEnDisable.GetAt(10);
				if( chEnable == '1')		strNewEMVEnDisable += _T("A0000000980840")	+ strAIDSeparator;

				strNewEMVEnDisable += _T("A0000000651010")		+ strAIDSeparator;		// JCB
				strNewEMVEnDisable += _T("A000000333010101")	+ strAIDSeparator;		// UnionPay Debit
				strNewEMVEnDisable += _T("A000000333010102")	+ strAIDSeparator;		// UnionPay Credit
				strNewEMVEnDisable += _T("A000000333010103")	+ strAIDSeparator;		// UnionPay Quasi Credit
				strNewEMVEnDisable += _T("A000000333010108")	+ strAIDSeparator;		// US UnionPay
				strNewEMVEnDisable += _T("A0000001524010")		+ strAIDSeparator;		// US Discover
				strNewEMVEnDisable += _T("A0000006200620")		+ strAIDSeparator;		// US DNA

			#elif(MX_VERSION)
				// JCB
				chEnable = strEMVEnDisable.GetAt(9);
				if( chEnable == '1')		strNewEMVEnDisable += _T("A0000000651010")	+ strAIDSeparator;

				// UnionPay Debit
				chEnable = strEMVEnDisable.GetAt(10);
				if( chEnable == '1')		strNewEMVEnDisable += _T("A000000333010101") + strAIDSeparator;

				// UnionPay Credit
				chEnable = strEMVEnDisable.GetAt(11);
				if( chEnable == '1')		strNewEMVEnDisable += _T("A000000333010102") + strAIDSeparator;

				// UnionPay Quasi Credit
				chEnable = strEMVEnDisable.GetAt(12);
				if( chEnable == '1')		strNewEMVEnDisable += _T("A000000333010103") + strAIDSeparator;
			#endif
		}
		
		// Set Enabled AIDs
		NHDEBUG(DBG_CALL, (_T("Conversion Completed = [%s]\n"), strNewEMVEnDisable ));
		MemSetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID, strNewEMVEnDisable);

		// Clear OLD AID Enable Status After Convert
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_AIDENDISABLE, L"" );
	}
	else
		NHDEBUG(DBG_CALL, (_T("OLD EMV En/Disable Status is EMPTY => NVRAM CLEARED or Already Converted\n")));

	char	buf[512] = { 0, };
	char	szIndex[16]	= { 0, };
	int		nAIDCount;

	// Check AID En/Disable Status
	strEMVEnDisable = MemGetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID);
	

	////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	///   Assign Default Enabled AIDs 
	////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	if(strEMVEnDisable.GetLength() < 2)		// NVRAM CLEARED ===> Assign Default Enabled AIDs		
	{
		NHDEBUG(DBG_CALL, (_T("EMV En/Disable Status is EMPTY => NVRAM CLEARED or New Installation\n")));

		// Open AID POOL FILE
		CINIFile iniEMV(GET_EMV_AID_POOLFILENAME(fnEMV_GetKernelVersion()) );

		strNewEMVEnDisable = _T(""); 

		// Read Total number of AID from the pool file
		memset(buf, 0, sizeof(buf));
		iniEMV.GetPrivateProfileString("AIDCOUNT","TOTALCOUNT",buf, 512);
		strTemp.Format(L"%S",buf);
		nAIDCount = Asc2Int(strTemp);

		for(int nCnt=0; nCnt<nAIDCount; nCnt++)
		{
			// Session
			sprintf_s(szIndex, sizeof(szIndex), "TERMINAL%d", nCnt+1);

			// Get AID
			memset(buf, 0, sizeof(buf));
			iniEMV.GetPrivateProfileString(szIndex,"AID",buf, 512); 
			strTemp.Format(L"%S",buf);

			// Compare AID
			if(strTemp.GetLength() >= 10)
			{
				CString strAID10 = strTemp.Left(10);
				if( strAID10.CompareNoCase(L"A000000277") == 0 )			// Interac	
				{															//					====> CANADA ONLY
					#if (CA_VERSION)	
					strNewEMVEnDisable += strTemp + strAIDSeparator;
					#endif
				}
				else if( (strAID10.CompareNoCase(L"A000000003") == 0) ||	// VISA, VISA ELECTRON, VISA PLUS
						 (strAID10.CompareNoCase(L"A000000098") == 0) ||	// US VISA
						 (strAID10.CompareNoCase(L"A000000004") == 0) ||	// MasterCard, MAESTRO, CIRRUS, US MAESTRO
						 (strAID10.CompareNoCase(L"A000000025") == 0) )		// AMEX
				{															// 					====> ALL COUNRTIES
					strNewEMVEnDisable += strTemp + strAIDSeparator;
				}
				else if( (strAID10.CompareNoCase(L"A000000152") == 0) ||	// PULSE D-PAS(Discover), US Discover
						 (strAID10.CompareNoCase(L"A000000620") == 0)	)	// US DNA
				{															// 					====> US and Canada
					#if (US_VERSION || CA_VERSION)							// [#2491] US Justin 2017.06.28 Change Default AID List
					strNewEMVEnDisable += strTemp + strAIDSeparator;
					#endif
				}
				else if( (strAID10.CompareNoCase(L"A000000065") == 0) ||	// JCB
						 (strAID10.CompareNoCase(L"A000000333") == 0)	)	// UnionPay Debit, Credit, Quiasi Credit, US UnionPay
				{															//					====> US, Canada, AND MEXICO
					#if (US_VERSION || MX_VERSION || CA_VERSION)			// [#2491] US Justin 2017.06.28 Change Default AID List
					strNewEMVEnDisable += strTemp + strAIDSeparator;
					#endif

				}
				// Added 2016.08.26 KSK
				else if( (strAID10.CompareNoCase(L"A000000384") == 0) ||	// eftpos saving 10, cheque 20
						 (strAID10.CompareNoCase(L"A000000384") == 0)	)	// => AU Only
				{
					#if (AU_VERSION)
					strNewEMVEnDisable += strTemp + strAIDSeparator;
					#endif
				}
				// end of 2016.08.26
				else														// Unknown AID... => Add here later....
				{
					;
				}
			}
		}

		if( strNewEMVEnDisable.GetLength() < 2 )		// NO AID is Enabled.....
			NHDEBUG(DBG_CALL, (_T("NO AID is Enabled.. Let ATM check again on Next Reboot......\n")));
		else
		{
			NHDEBUG(DBG_CALL, (_T("Enabled AIDs = [%s]\n"), strNewEMVEnDisable));
			MemSetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID, strNewEMVEnDisable);
		}
	}

	// Load Common AID........................
#if (US_VERSION)
	// Load Common AID File
	CINIFile iniEMV( COMMONAID_LIST_INI_FILE );

	// Read a number of Common AID
	memset(buf, 0, sizeof(buf));
	iniEMV.GetPrivateProfileString("COMMONAID","COUNT",buf, 512);
	strTemp.Format(L"%S",buf);
	nAIDCount = Asc2Int(strTemp);

	for(int nCommonAID = 0; nCommonAID < nAIDCount; nCommonAID++)
	{
		sprintf_s(szIndex, sizeof(szIndex), "AID%d",nCommonAID+1);

		// Read Common AID
		memset(buf, 0, sizeof(buf));
		iniEMV.GetPrivateProfileString("COMMONAID", szIndex, buf, 40); 
		strTemp.Format(L"%S",buf);

		// Add to Common AID Buffer
		if(strTemp.GetLength() > 1) 
		{
			m_arrUSCommonAID.Add(strTemp);		
			NHDEBUG(DBG_CALL, (_T("  COMMON AID [%s] is Added  \n"), strTemp));
		}
	}
#endif
	// End of [#2342]
	// End of [#2226]

	//////////////////////////////////////////////////////////////
	// Load EMV Kernel and Initialize
	//////////////////////////////////////////////////////////////
	fnEMV_Initialize();	

#ifdef UNDER_CE
 	// [#2518] US Kook 2018.01.10 Support Camera
 	m_bCamInit = false;
 	m_bCamPreviewInProgress = false;
  	// end of [#2518]
#endif // UNDER_CE

	NVDump('O', 'E', "43", L"", L"SetOwner_E");

	return;
}

CString CDevCmn::GetSWVersionInfo()
{
NHDEBUG(1, (_T("***DevApl***CDevCmn::GetSWVersionInfo() \n")));

	CString strVersion;
	CString strTemp;

	//1.AP Version
	strVersion = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_AP_VERSION);	// [#127] KSK 2008.04.21
	strVersion += L"||";

	//2.OS Version
	strVersion += MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_OS_VERSION);	// [#127] KSK 2008.04.21
	strVersion += L"||";

	//3.RMS version
	strVersion += MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_RMS_VERSION);	// [#127] KSK 2008.04.21
	strVersion += L"||";

	// KSK 2010.03.25 MWI 버전 삭제 후 Boot Loader Version으로 대체
	//4. BootLoader Version
	strVersion += m_pConfig->GetBootLoaderVersion();
	strVersion += L"||";

	// [#2321] NH KSK 2015.01.14 MWI Version으로 변경 (AP Revision 관리를 위해)
	//5. Touch Panel
//	strVersion += m_pConfig->GetTouchPanelVersion();
//	strVersion += "||";
	strVersion += MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_MWI_VERSION);
	strVersion += L"||";
	// end of [#2321]

	// [#608] NH KSK 2010.01.12 VDM Version 추가
	//6. VDM CDU Version
	strVersion += GetFileReadVersion(VDM_CDU_FULL_PATH, TRUE);
	strVersion += L"||";

	//7. VDM SPR Version
	strVersion += GetFileReadVersion(VDM_SPR_FULL_PATH, TRUE);
	strVersion += L"||";

	//8. VDM MCU Version
	if(GetDeviceType(L"MCU") == MCU_MAGTEK_DIP)	// [#25] NH PSC '1': DepType, '3': Magtek, '4': Sankyo
		strVersion += GetFileReadVersion(VDM_MCU_MAGTEK_FULL_PATH, TRUE);
	else if (GetDeviceType(L"MCU") == MCU_SANKYO_DIP)	// [#2073] NH KSK 2011.06.24
		strVersion += GetFileReadVersion(VDM_MCU_SANKYO_FULL_PATH, TRUE);
	else
		strVersion += GetFileReadVersion(VDM_MCU_NHDIP_FULL_PATH, TRUE);
	strVersion += L"||";

	//9.CDU SP
	strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"CDM");
	strVersion += L"||";

	//10.SPR SP
	strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"SPR");
	strVersion += L"||";

	//11.MCU SP
	strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"IDC");
	strVersion += L"||";

	//12.PIN SP
	strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"PIN");
	strVersion += L"||";

	//13.SIU SP
	strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"SIU");
	strVersion += L"||";

	//14.CDU EP
	strTemp = MemGetVersion(_MEMKEY_EPVERSION, L"CDM");
	if (strTemp.GetLength() > 0)
	{
		if (MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_CDU_AUTHFLAG))		strTemp += L"B";	// [#2384] US JUSTIN 2015.12.23 Display CDUEP Binding Information.
		if (MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_CDU_SUPPORTAR))		strTemp += L"R";	// [#RWC6-82] NH Kook 2021.12.27 Support CDU Auto Recovery (Jam Clear)
	}
	strVersion += strTemp;
	strVersion += L"||";

	//15.SPR EP
	strVersion += MemGetVersion(_MEMKEY_EPVERSION, L"SPR");
	strVersion += L"||";

	//16.IDC EP
	strVersion += MemGetVersion(_MEMKEY_EPVERSION, L"IDC");
	strVersion += L"||";

	//17.PIN EP
	strVersion += MemGetVersion(_MEMKEY_EPVERSION, L"PIN");
	strVersion += L"||";

	//18.SIU EP
	strVersion += L"N/A";

	// [#2220] NH KMK 2013.10.04 EMV Kernel Version 추가
	strVersion += L"||";

	// 19. EMV Kernel Version (AP)
	if (fnMCU_IsEmvAvailable())			// EMV Available 시 current EMV Kernel Version 출력
	{
		switch (fnEMV_GetKernelVersion())
		{
		case EMV_KERNEL_V4:			// EMV Kernel V4
			strVersion += fstrSCR_GetStringFromTextID(T_EMV_KERNEL_VER4);	break;
		case EMV_KERNEL_V5:			// EMV Kernel V5.5
			strVersion += fstrSCR_GetStringFromTextID(T_EMV_KERNEL_VER5);	break;
		// [#2435] NH KSK 2016.07.14
		case EMV_KERNEL_V6:			// EMV Kernel V6.0
			strVersion += fstrSCR_GetStringFromTextID(T_EMV_KERNEL_VER6);	break;
		// end of [#2435]
		case EMV_KERNEL_V7:
			strVersion += fstrSCR_GetStringFromTextID(T_EMV_KERNEL_VER7);	break;
		default:						// 예외처리
			strVersion += L"N/A";		break;
		}
	}
	else		// EMV Unavailable 시 N/A 출력
	{
		strVersion += L"N/A";
	}
	// end of [#2220]

	// [#2259] NH KSK 2014.03.13 호환성을 위해 맨 뒤에 추가함
	// 20. KeyMgr Version
	strVersion += L"||";
	strVersion += GetFileReadVersion(VDM_KEYMGR_FULL_PATH, TRUE);
	// end of [#2259]

	// [#2325] NH KSK 2015.01.26 RFID Version Added
	// 21. RFID SP Version, 22. RFID EP Version	23. RFID VDM Dll
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
	{
		strVersion += L"||";
		strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"RFID");
		strVersion += L"||";
		strVersion += MemGetVersion(_MEMKEY_EPVERSION, L"RFID");
		strVersion += L"||";
		strVersion += GetFileReadVersion(VDM_RFID_VIVOPAYKII_FULL_PATH, TRUE);
	}
	else
	{
		strVersion += L"||";
		strVersion += L"N/A";
		strVersion += L"||";
		strVersion += L"N/A";
		strVersion += L"||";
		strVersion += L"N/A";
	}
	// end of [#2325]

	// [#GLDV-3005] US Kook 2022.01.03 Side Car
	// 24. BCR SP, 25. BCR EP, 26: BCR VDM
	// 27. BNA SP, 28. BNA EP, 29: BNA VDM
	// 30. COD SP, 31. COD EP, 31: COD VDM
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
	{
		strVersion += L"||";
		strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"BCR");
		strVersion += L"||";
		strVersion += MemGetVersion(_MEMKEY_EPVERSION, L"BCR");
		strVersion += L"||";
		strVersion += GetFileReadVersion(VDM_BCR_FULL_PATH, TRUE);

		strVersion += L"||";
		strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"BNA");
		strVersion += L"||";
		strVersion += MemGetVersion(_MEMKEY_EPVERSION, L"BNA");
		strVersion += L"||";
		strVersion += GetFileReadVersion(VDM_BNA_MEI_FULL_PATH, TRUE);

		//strVersion += L"||";
		//strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"COD");
		//strVersion += L"||";
		//strVersion += MemGetVersion(_MEMKEY_EPVERSION, L"COD");
		//strVersion += L"||";
		//strVersion += GetFileReadVersion(VDM_COD_FULL_PATH, TRUE);
		strVersion += L"||";
		strVersion += L"N/A";
		strVersion += L"||";
		strVersion += L"N/A";
		strVersion += L"||";
		strVersion += L"N/A";
	}
	else
	{
		strVersion += L"||";
		strVersion += L"N/A";
		strVersion += L"||";
		strVersion += L"N/A";
		strVersion += L"||";
		strVersion += L"N/A";

		strVersion += L"||";
		strVersion += L"N/A";
		strVersion += L"||";
		strVersion += L"N/A";
		strVersion += L"||";
		strVersion += L"N/A";

		strVersion += L"||";
		strVersion += L"N/A";
		strVersion += L"||";
		strVersion += L"N/A";
		strVersion += L"||";
		strVersion += L"N/A";
	}
	// end of [#GLDV-3005]

	return strVersion;
}

// [#105] KSK 2008.04.14
void CDevCmn::Set_NetworkInformation()
{
	int rmsClientLineType, rmsServerLineType;

#ifdef UNDER_CE
	if ((RegGetStr(L"SOFTWARE\\ATM", L"OSVersion")).CompareNoCase(L"V01.01.06") < 0)
		MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE, 1);
#else
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE, 2);
#endif

	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == 1)
	{
		linetype = LINE_DIALUP;

		// [#RWC6-67] US William 2019.10.21 MoniView TLS
		// Ensure the RMS line selection matches the selected option
		rmsClientLineType = LINE_DIALUP;
		rmsServerLineType = LINE_DIALUP;

		// [#105] KSK 2008.04.14 EOTCHECK MODE 값 변경
		// GENERAL_MODE -> EOT_REQUIRED ("1")
		// LYNK_MODE	-> EOT_OPTIONAL ("2")
		if ((MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == LYNK_MODE) ||
			(MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == EOT_OPTIONAL))
		{
			MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE, EOT_OPTIONAL);
		}
		else
		if (MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == NO_EOT_REQUIRED)
		{
			MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE, NO_EOT_REQUIRED);
		}
		else
		if (MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == NO_ENQ_REQUIRED)
		{
			MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE, NO_ENQ_REQUIRED);
		}
		else //[#556] SOOK 2009.08.12 NO ENQ/EOT REQUIRED 추가 
		if (MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == NO_ENQ_EOT_REQUIRED)
		{
			MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE, NO_ENQ_EOT_REQUIRED);
		} //end of [#556]
		else
		{
			MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE, EOT_REQUIRED);
		}
		flowtype = VISAII_FLOW;
		// end of [#105]
	}
	else
	{
		linetype = LINE_TLS;

		// [#RWC6-67] US William 2019.10.21 MoniView TLS
		// Ensure the RMS line selection matches the selected option
		// LineTLS also handles unencrypted traffic, but LineTCP is the only
		// server-mode "line"
		rmsClientLineType = LINE_TLS;
		rmsServerLineType = LINE_TCP;

		// [#105] KSK 2008.04.14 EOTCHECK MODE 값 변경
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE) == VISA_FRAMED_TCPIP)
		{
			if ((MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == LYNK_MODE) ||
				(MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == EOT_OPTIONAL))
			{
				MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE, EOT_OPTIONAL);
			}
			else
			if (MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == NO_EOT_REQUIRED)
			{
				MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE, NO_EOT_REQUIRED);
			}
			else
			if (MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == NO_ENQ_REQUIRED)
			{
				MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE, NO_ENQ_REQUIRED);
			}
			else //[#556] SOOK 2009.08.12 NO ENQ/EOT REQUIRED 추가 
			if (MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == NO_ENQ_EOT_REQUIRED)
			{
				MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE, NO_ENQ_EOT_REQUIRED);
			} //end of [#556]
			else
			{
				MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE, EOT_REQUIRED);
			}
			flowtype = VISAII_FLOW;
		}
		else
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE) == STANDARD_TCPIP)
		{
			flowtype = STANDARD_FLOW;
		}
		else
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE) == ACK_CONTROLLED_TCPIP)
		{
			flowtype = ACKCONTROLLED_FLOW;
		}
		else
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE) == VISA_FRAMED_WITHOUT_ENQ_TCPIP)
		{
			MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE, NO_ENQ_REQUIRED);
			flowtype = VISAII_FLOW;
		}
		// end of [#105]
	}	

	if (m_pNetWork != NULL)
	{
		delete m_pNetWork;
		m_pNetWork = NULL;
	}

	m_pNetWork = new CNetWork(linetype, rmsClientLineType, rmsServerLineType);
	m_pNetWork->SetInformation(flowtype);
}

// [#465] [NH] KSK 2008.12.22
CString	CDevCmn::Convert_IpAddressInfo(CString strParm_Ip)
{
	// ex: 150.001.001.002 -> 150.1.1.2로 변환하여 NVRAM에 Set함
	// 1. HOST IP(1, 2) Convert
	CStringArray	StrArrayCheckValid;
	CString			strReturnIp;
	int				param_value[4] = {};
	int				j = 0;
	char			chTempBuffer[256] = { 0, };	// [#486] [NH] KSK 2009.1.19
	
	StrArrayCheckValid.RemoveAll();
	strReturnIp.Empty();
	
	SplitString(strParm_Ip, L".", StrArrayCheckValid);
	
	if (StrArrayCheckValid.GetSize() == 4)
	{
		for(j=0; j<StrArrayCheckValid.GetSize(); j++)
		{
			// [#486] [NH] KSK 2009.1.19 IP일 때 Validation Check
			memset(chTempBuffer, 0, sizeof(chTempBuffer));
			WideToMulti(chTempBuffer, StrArrayCheckValid[j], sizeof(chTempBuffer));

			if (IsNum(chTempBuffer, StrArrayCheckValid[j].GetLength()) == TRUE)
			{
				param_value[j] = Asc2Int(StrArrayCheckValid[j]);
				if (j==0)	
					strReturnIp.Format(L"%d", param_value[j]);
				else
				{
					CString	temp;
					temp.Format(L".%d", param_value[j]);
					strReturnIp += temp;
				}
			}
			else
			{
				strReturnIp = strParm_Ip;
				break;
			}
			// end of [#486]
		}
	}
	else
		strReturnIp = strParm_Ip;

	return strReturnIp;
}
// end of [#465]

// [#25] NH PSC MagTek기능 추가관련 DeviceType 체크함수
int CDevCmn::GetDeviceType(CString strDeviceName)
{
	int ret = 1;	
	NHTRACE((_T("***DevCmn***CDevCmn::GetDeviceType(%s) \n"), strDeviceName));

	// [#2073] NH KSK 2011.06.24 Card Reader 설정기능에 의해 ini file이 아닌 NVRAM에서 Read하도록 수정
	if (strDeviceName == L"MCU")
	{
		// '1': NH Dip, '3': Magtek,	'4' : Sankyo Dip
		ret = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CARDREADER_SELECTION);	// 설정된 값이 없을 경우 0이 리턴됨

		// Set Default Card Reader 1:NH Dip, 3:Magtek, 4:Sankyo Dip
		if( (ret!= MCU_NH_DIP)&&(ret!=MCU_MAGTEK_DIP)&&(ret!=MCU_SANKYO_DIP) )		// Initial Installation or Incorrect Card reader Type
		{	
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CARDREADER_SELECTION, MCU_SANKYO_DIP);
		}
		NHTRACE((_T("***DevCmn***CDevCmn::GetDeviceType(%d) \n"), ret));
		// [#2393]	

		return ret;
	}
	// end of [#2073]
	// [#GLDV-2505] US Kook 2019.05.13 Support MX2800T
	else if (strDeviceName == L"SPL")
	{
		if (m_pConfig->GetOSVersion() == NH_OS_NH2700_F_T
			|| m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T
			|| m_pConfig->GetOSVersion() == NH_OS_MX2800_F_T)
		{
			return SPL_SET;
		}
		else
		{
			return NOT_SET;
		}
	}
	// end of [#GLDV-2505]
	// [#GLDV-3005] US Kook 2022.04.07 support side car
	else if (strDeviceName == L"BNA")
	{
		ret = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_BNATYPE);
		if ((ret != BNATYPE_MEI) && (ret != BNATYPE_JCM))
			ret = BNATYPE_MEI;

		NHDEBUG(1, (_T("bnaType: [%d]) \n"), ret));
	}
	// end of [#GLDV-3005]

	return ret;
}
// end of [#25]

/////////////////////////////////////////////////////////////////////////////
// Clear Error Code
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnDOR_ClearErrorCode()
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnDOR_ClearErrorCode() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_DOR))
		nReturn = m_pMwi->DorClearErrorCode();

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
//	DOOR FUNCTION(DOR) : DOR GET MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Get Error Code
/////////////////////////////////////////////////////////////////////////////
CString CDevCmn::fstrDOR_GetErrorCode()
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fstrDOR_GetErrorCode() \n")));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_DOR))
		strReturn = m_pMwi->DorGetErrorCode();

	return strReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get Device Status
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnDOR_GetDeviceStatus()
{
	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_DOR))
		nReturn = m_pMwi->DorGetDeviceStatus();

	if (nReturn != nSaveReturn)
	{
		nSaveReturn = nReturn;
	}
	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get Door Status
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnDOR_GetDoorStatus()
{
	// NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnDOR_GetDoorStatus() \n")));

	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_DOR))
		nReturn = m_pMwi->DorGetDoorStatus();

	if (nReturn != nSaveReturn)
	{
		nSaveReturn = nReturn;
	}
	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
//	DOOR FUNCTION(DOR) : DOR OPERATION MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Initialize
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnDOR_Initialize()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnDOR_Initialize() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_DOR))
		nReturn = m_pMwi->DorInitialize();

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Deinitialize
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnDOR_Deinitialize()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnDOR_Deinitialize() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_DOR))
		nReturn = m_pMwi->DorDeinitialize();

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
//	LIGHT FUNCTION(LGT) : LGT SET MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Clear Error Code
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnLGT_ClearErrorCode()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnLGT_ClearErrorCode() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_LGT))
		nReturn = m_pMwi->LgtClearErrorCode();

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Set Flicker(Index, Value)
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnLGT_SetFlicker(int nIndex, int nValue)
{
	//NHDEBUG(1, (_T("nIndex: (%d), nValue: (%d) \n"), nIndex, nValue));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_LGT))
		nReturn = m_pMwi->LgtSetFlicker(nIndex, nValue);

	return nReturn;
}


/////////////////////////////////////////////////////////////////////////////
//	LIGHT FUNCTION(LGT) : LGT GET MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Get Error Code
/////////////////////////////////////////////////////////////////////////////
CString CDevCmn::fstrLGT_GetErrorCode()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fstrLGT_GetErrorCode() \n")));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_LGT))
		strReturn = m_pMwi->LgtGetErrorCode();

	return strReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get Device Status
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnLGT_GetDeviceStatus()
{
	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_LGT))
		nReturn = m_pMwi->LgtGetDeviceStatus();

	if (nReturn != nSaveReturn)
	{
		nSaveReturn = nReturn;
	}
	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
//	LIGHT FUNCTION(LGT) : LGT OPERATION MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Initialize
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnLGT_Initialize()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnLGT_Initialize() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_LGT))
		nReturn = m_pMwi->LgtInitialize();

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Deinitialize
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnLGT_Deinitialize()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnLGT_Deinitialize() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_LGT))
		nReturn = m_pMwi->LgtDeinitialize();

	return nReturn;
}


/////////////////////////////////////////////////////////////////////////////
//	PIN FUNCTION(PIN) : PIN SET MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Clear Error Code
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_ClearErrorCode()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_ClearErrorCode() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinClearErrorCode();

	return nReturn;
}

// [#2057] NH KSK 2011.05.04 InitializeEDM() 함수 추가
/////////////////////////////////////////////////////////////////////////////
// Clear All Keys
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_ClearAllKeys()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_ClearAllKeys() \n")));

	int		nReturn = FALSE;

	nReturn = m_pMwi->PinInitializeEDM();

	return nReturn;
}
// end of [#2057]

/////////////////////////////////////////////////////////////////////////////
//	PIN FUNCTION(PIN) : PIN GET MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Get Error Code
/////////////////////////////////////////////////////////////////////////////
CString CDevCmn::fstrPIN_GetErrorCode()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fstrPIN_GetErrorCode() \n")));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_PIN))
		strReturn = m_pMwi->PinGetErrorCode();

	return strReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get Device Status
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_GetDeviceStatus()
{
	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinGetDeviceStatus();

	if (nReturn != nSaveReturn)
	{
		nSaveReturn = nReturn;
		m_strNVLog.Format(L"1:PINST(%d)", nReturn);
		NVDump('O', 'E', "1N", L"", m_strNVLog);
	}
	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get Pin Key Data
/////////////////////////////////////////////////////////////////////////////
CString CDevCmn::fstrPIN_GetPinKeyData()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fstrPIN_GetPinKeyData() \n")));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_PIN))
		strReturn = m_pMwi->PinGetPinKeyData();

	return strReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get Macing Data
/////////////////////////////////////////////////////////////////////////////
CString CDevCmn::fstrPIN_GetMacingData()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fstrPIN_GetMacingData() \n")));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_PIN))
		strReturn = m_pMwi->PinGetMacingData();

	return strReturn;
}

/////////////////////////////////////////////////////////////////////////////
//	PIN FUNCTION(PIN) : PIN OPERATION MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Initialize
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_Initialize()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_Initialize() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinInitialize();

#if (AU_VERSION)
	// [#2580] AU Kook 2019.07.08 Support EPP S/N Inquiry.
	CString strEppSerialNumberAscii;
	strEppSerialNumberAscii.Empty();		// default empty. ("")

	if (nReturn == 0)
	{
		if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) < 0)
		{
			// There is no EPP S/N when EPP has no RKT capability. (below V08.20.XX)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_AVAILABLE, 0);
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_ENABLE, 0);
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, 0);
		}
		else
		{
			// We can try to get EPP S/N when EPP has RKT capability. (V08.20.XX and above)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_AVAILABLE, ENABLE);
			if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_PCI30_EP_VERSION) >= 0)
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_RANDOMNUMBER_ENABLE, ENABLE);

			if (fnPIN_RKT_ExportRSAIssuerSignedItem(L"EPPID", L"") != T_OK)
			{
				NHDEBUG(DBG_INFO, (_T("***fnPIN_Initialize()*** fnPIN_RKT_ExportRSAIssuerSignedItem(EPP_ID) Fail\n")));
				NVDump('O', 'C', "76", L"", L"EPPID_FAIL");
			}
			else
			{
				// Get EPP Serial Number
				CString strEppSerialNumberHex = fnPIN_RKT_GetExportedKey();
				MakePack(strEppSerialNumberHex, strEppSerialNumberAscii);
				NHDEBUG(1, (_T("***fnPIN_Initialize()*** fnPIN_RKT_GetExportedKey() (%s) [HEX] / (%s) [ASCII] \n"), strEppSerialNumberHex, strEppSerialNumberAscii));
				NVDump('O', 'C', "76", L"", L"EPPID_OK");
				NVDump('O', 'C', "76", L"", strEppSerialNumberAscii);
			}
		}
	}

	MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_EPP_SERIAL_NUMBER, strEppSerialNumberAscii);
	// end of [#2580]
#endif

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Deinitialize
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_Deinitialize()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_Deinitialize() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinDeinitialize();

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Entry Enable(EnableMode, MinKeyCount, MaxKeyCount, AutoEnd, ActiveKeys, TerminatorKeys, CardData, Timeout)
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_EntryEnable(int nEnableMode, int nMinKeyCount, int nMaxKeyCount, int bAutoEnd, LPCTSTR szActiveKeys, LPCTSTR szTerminatorKeys, LPCTSTR szCardData, int nTimeout)
{
NHDEBUG(1, (_T("[nEnableMode(%d), nMinKeyCount(%d), nMaxKeyCount(%d), bAutoEnd(%d), szActiveKeys(%s), szTerminatorKeys(%s), szCardData(%s), nTimeout(%d)] \n"), 
		nEnableMode, 
		nMinKeyCount, 
		nMaxKeyCount, 
		bAutoEnd, 
		szActiveKeys, 
		szTerminatorKeys, 
		szCardData, 
		nTimeout));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinEntryEnable(nEnableMode, nMinKeyCount, nMaxKeyCount, bAutoEnd, szActiveKeys, szTerminatorKeys, szCardData, nTimeout);

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Entry Disable
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_EntryDisable()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_EntryDisable() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinEntryDisable();

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Load IV(Key Name) - 사용 안함
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_LoadIV(LPCTSTR szKeyName)
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_LoadIV() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinLoadIV(szKeyName);

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Load Key(Key Name, Key Value)
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_LoadKey(LPCTSTR szKeyName, LPCTSTR szKeyValue)
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_LoadKey(KeyName=[%s], KeyValue=[%s]) \n"), szKeyName, szKeyValue ));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinLoadKey(szKeyName, szKeyValue);

	m_strNVLog.Format(L"1:(%s)", szKeyName);
	NVDump('O', 'E', "1V", L"", m_strNVLog);

	return nReturn;
}

// [#554] KSK 2009.06.22
/////////////////////////////////////////////////////////////////////////////
// Load Key(Key Name, Key Value) - 512K
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_LoadEncryptedKey(LPCTSTR KeyUse, LPCTSTR szKeyName, LPCTSTR szKeyValue, LPCTSTR szEncKeyName)
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_LoadEncryptedKey(KeyUse=[%s], KeyName=[%s], KeyValue=[%s], EncKey=[%s]) \n"), KeyUse, szKeyName, szKeyValue, szEncKeyName));
	
	int		nReturn = FALSE;
	
	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinLoadEncryptedKey(KeyUse, szKeyName, szKeyValue, szEncKeyName);
	
	m_strNVLog.Format(L"1:(%s)", szKeyName);
	NVDump('O', 'E', "1V", L"", m_strNVLog);
	
	return nReturn;
}
// end of [#554]


/////////////////////////////////////////////////////////////////////////////
// Macing Data(Mac Data)
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_MacingData(LPCTSTR szMacData)
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_MacingData() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinMacingData(szMacData);

	return nReturn;
}

// [#554] NH KSK 2009.08.10 RKT 관련 함수 추가
/////////////////////////////////////////////////////////////////////////////
// fnPIN_RKT_ExportRSAIssuerSignedItem
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_RKT_ExportRSAIssuerSignedItem(LPCTSTR ExportItemType, LPCTSTR KeyName)
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_RKT_ExportRSAIssuerSignedItem() \n")));
	
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->RKT_ExportRSAIssuerSignedItem(ExportItemType, KeyName);

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// fnPIN_RKT_ImportRSAPublicKey
/////////////////////////////////////////////////////////////////////////////
// [#GLDV-2797] AU Kook 2020.05.07 Support Host PK Removal
//int	CDevCmn::fnPIN_RKT_ImportRSAPublicKey(LPCTSTR Recv_HSM_PublicKey, LPCTSTR Recv_HSM_Signature)
  int	CDevCmn::fnPIN_RKT_ImportRSAPublicKey(LPCTSTR Recv_HSM_PublicKey, LPCTSTR Recv_HSM_Signature, LPCTSTR Usage)
// end of [#GLDV-2797]
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_RKT_ImportRSAPublicKey() Usage: (%s)\n"), Usage));
	
	int		nReturn = FALSE;
	
	if (fnAPL_GetDefineDevice(DEV_PIN))
// [#GLDV-2797] AU Kook 2020.05.07 Support Host PK Removal
//		nReturn = m_pMwi->RKT_ImportRSAPublicKey(Recv_HSM_PublicKey, Recv_HSM_Signature);
		nReturn = m_pMwi->RKT_ImportRSAPublicKey(Recv_HSM_PublicKey, Recv_HSM_Signature, Usage);
// end of [#GLDV-2797]
	
	CString m_strNVLog;
	m_strNVLog.Format(_T("%s:%d"), _T("IRSAPK"), nReturn);
	NVDump('O', 'E', "1H", L"", m_strNVLog);

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// fnPIN_RKT_StartKeyExchange
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_RKT_StartKeyExchange()
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_RKT_StartKeyExchange() \n")));
	
	int		nReturn = FALSE;
	
	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->RKT_StartKeyExchange();
	
	return nReturn;
}

// [#2080] NH KSK 2011.07.13
int	CDevCmn::fnPIN_RKT_GetExtraCaps()
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_RKT_GetExtraCaps() \n")));

	int nReturn = 0;

	nReturn = m_pMwi->RKT_GetExtraCaps();

	return nReturn;
}
// end of [#2080]

/////////////////////////////////////////////////////////////////////////////
// fnPIN_RKT_ImportRSASignedDESKey
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnPIN_RKT_ImportRSASignedDESKey(int KeyType, LPCTSTR Recv_EPP_RSACrypt, LPCTSTR Recv_TMK_RSACrypt, LPCTSTR Recv_HSM_Signature)
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_RKT_ImportRSASignedDESKey() \n")));
	
	int		nReturn = FALSE;
	
	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->RKT_ImportRSASignedDESKey(KeyType, Recv_EPP_RSACrypt, Recv_TMK_RSACrypt, Recv_HSM_Signature);
	
	return nReturn;
}

CString	CDevCmn::fnPIN_RKT_GetExportedKey()
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_RKT_GetExportedKey() \n")));
	
	CString	strReturn;
	
	if (fnAPL_GetDefineDevice(DEV_PIN))
		strReturn = m_pMwi->RKT_GetExportedKey();
	
	return strReturn;
}

CString	CDevCmn::fnPIN_RKT_GetExportedSignedValue()
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_RKT_GetExportedSignedValue() \n")));
	
	CString	strReturn;
	
	if (fnAPL_GetDefineDevice(DEV_PIN))
		strReturn = m_pMwi->RKT_GetExportedSignedValue();
	
	return strReturn;
}

CString	CDevCmn::fnPIN_RKT_GetEPP_RandomNumber()
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_RKT_GetEPP_RandomNumber() \n")));
	
	CString	strReturn;
	
	if (fnAPL_GetDefineDevice(DEV_PIN))
		strReturn = m_pMwi->RKT_GetEPP_RandomNumber();	// [#2259] NH KSK 2014.03.13 Random Number로 함수 변경
	
	return strReturn;
}

CString	CDevCmn::fnPIN_RKT_GetEPP_KCV()
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_RKT_GetEPP_KCV() \n")));
	
	CString	strReturn;
	
	if (fnAPL_GetDefineDevice(DEV_PIN))
		strReturn = m_pMwi->RKT_GetEPP_KCV();
	
	return strReturn;
}
// end of [#554]

// [#2253] NH KSK 2014.02.21
CString	CDevCmn::fnPIN_Get_KCV_512K(CString KeyName)
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_Get_KCV_512K(%s) \n"), KeyName));
	
	CString strReturn;
	
	if (fnAPL_GetDefineDevice(DEV_PIN))
		strReturn = m_pMwi->PIN_GenerateKCV_512K(KeyName);
	
	return strReturn;
}
// end of [#2253]

// [#GLDV-2797] AU Kook 2020.04.28 Support Host PK Removal
BOOL CDevCmn::fnPIN_RKT_DeleteHostPK()
{
	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnPIN_RKT_DeleteHostPK() \n"), _T("")));

	int nReturn = FALSE;
	if (fnPIN_GetDeviceStatus() == NORMAL)
	{
		nReturn = fnPIN_RKT_ImportRSAPublicKey(L"0x00", L"0x00", L"ZERO");

		if (nReturn == WFS_SUCCESS
			|| fstrPIN_GetErrorCode() == L"HPKEMTY")
		{
			m_JNLMgr.Save(OPERATOR_ACTION, _T("Host PK is empty"));
			NVDump('O', 'E', "1H", L"", L"HPKREM_OK");
			return TRUE;
		}
	}

	CString m_strNVLog;
	m_strNVLog.Format(_T("%s:%d"), _T("HPKREM_NG"), nReturn);
	NVDump('O', 'E', "1H", L"", m_strNVLog);

	return FALSE;
}
// end of [#GLDV-2797]

// [#RWC6-151] US Kook 2020.07.20 TR34
int CDevCmn::fnPIN_TR34_GetCertificate(CString Certificate)
{
	NHDEBUG(1, (_T("Certificate: %s \n"), Certificate));

	int nReturn = -1;
	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinGetCertificate(Certificate);
	return nReturn;
}


int	CDevCmn::fnPIN_TR34_InitializeEDMEx(CString Indent, CString Key)
{
	NHDEBUG(1, (_T("Indent: %s, Key: %s\n"), Indent, Key));

	int nReturn = -1;
	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinInitializeEDMEx(Indent, Key);
	return nReturn;
}

int	CDevCmn::fnPIN_TR34_ImportKeyBlock(CString Key, CString EncKey, CString KeyBlock)
{
	NHDEBUG(1, (_T("Key: %s, EncKey: %s, KeyBlock: %s\n"), Key, EncKey, KeyBlock));

	CString encodedData = String2Hex(KeyBlock);

	int nReturn = -1;
	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinImportKeyBlock(Key, EncKey, encodedData);
	return nReturn;
}

int	CDevCmn::fnPIN_TR34_LoadCertificateEx(CString LoadOption, CString Signer, CString CertificateData)
{
	NHDEBUG(1, (_T("LoadOption: %s, Signer: %s, CertificateData: %s\n"), LoadOption, Signer, CertificateData));

	int nReturn = -1;
	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinLoadCertificateEx(LoadOption, Signer, CertificateData);
	return nReturn;
}

int	CDevCmn::fnPIN_TR34_ImportRSAEncipheredPKCS7KeyEx(CString RSAKeyIn, CString Key, CString Use, CString LoadOption)
{
	NHDEBUG(1, (_T("RSAKeyIn: %s, Key: %s, Use: %s, LoadOption: %s\n"), RSAKeyIn, Key, Use, LoadOption));

	int nReturn = -1;
	if (fnAPL_GetDefineDevice(DEV_PIN))
		nReturn = m_pMwi->PinImportRSAEncipheredPKCS7KeyEx(RSAKeyIn, Key, Use, LoadOption);
	return nReturn;
}

CString CDevCmn::fstrPIN_TR34_GetCertificateData()
{
	CString strCTepp = m_pMwi->PinGetCertificateData();
	//NHDEBUG(1, (_T("PinGetCertificateData: %s\n"), strCTepp));

	return strCTepp;
}

CString CDevCmn::fstrPIN_TR34_GetCertificateState()
{
	CString strCertState = m_pMwi->PinGetCertificateState();
	NHDEBUG(1, (_T("PinGetCertificateState: %s\n"), strCertState));

	return strCertState;
}

int CDevCmn::fnPIN_TR34_StartAuthenticateSync(CString MethodName, CString Parameters)
{
	return m_pMwi->PinStartAuthenticateSync(MethodName, Parameters);
}

CString CDevCmn::fstrPIN_TR34_GetAuthenticateDataToSign()
{
	CString strAuthDataToSign = m_pMwi->PinGetAuthenticateDataToSign();
	NHDEBUG(1, (_T("strAuthDataToSign: %s\n"), strAuthDataToSign));

	return strAuthDataToSign;
}

CString CDevCmn::fstrPIN_TR34_GetAuthenticateSigner()
{
	CString strAuthSigner = m_pMwi->PinGetAuthenticateSigner();
	NHDEBUG(1, (_T("strAuthSigner: %s\n"), strAuthSigner));

	return strAuthSigner;
}

void CDevCmn::fnPIN_TR34_SetAuthenticateSigner(CString Signer)
{
	NHDEBUG(1, (_T("AuthenticateSigner SET TO: %s\n"), Signer));
	m_pMwi->PinSetAuthenticateSigner(Signer);
}

void CDevCmn::fnPIN_TR34_SetAuthenticateSigKey(CString SigKey)
{
	NHDEBUG(1, (_T("AuthenticateSig Key SET TO: %s\n"), SigKey));
	m_pMwi->PinSetAuthenticateSigKey(SigKey);
}

void CDevCmn::fnPIN_TR34_SetAuthenticateSignedData(CString SignedData)
{
	NHDEBUG(1, (_T("AuthenticateSignedData SET TO: %s\n"), SignedData));
	m_pMwi->PinSetAuthenticateSignedData(SignedData);
}



/////////////////////////////////////////////////////////////////////////////
//	SENSOR FUNCTION(SNS) : SNS SET MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Clear Error Code
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnSNS_ClearErrorCode()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnSNS_ClearErrorCode() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_SNS))
		nReturn = m_pMwi->SnsClearErrorCode();

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Clear Operator Switch
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnSNS_ClearOperatorSwitch()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnSNS_ClearOperatorSwitch() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_SNS))
		nReturn = m_pMwi->SnsClearOperatorSwitch();

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
//	SENSOR FUNCTION(SNS) : SNS GET MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Get Error Code
/////////////////////////////////////////////////////////////////////////////
CString CDevCmn::fstrSNS_GetErrorCode()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fstrSNS_GetErrorCode() \n")));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_SNS))
		strReturn = m_pMwi->SnsGetErrorCode();

	return strReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get Device Status
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnSNS_GetDeviceStatus()
{
	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_SNS))
		nReturn = m_pMwi->SnsGetDeviceStatus();

	if (nReturn != nSaveReturn)
	{
		nSaveReturn = nReturn;
	}
	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get SENSOR Status
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnSNS_GetOperatorSwitch()
{
	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

//나중에 : SNS 중에서도 OperatorSwitch가 있는지 없는지 검사해야한다.
//	if (fnAPL_GetDefineDevice(DEV_SNS))
//		nReturn = m_pMwi->SnsGetOperatorSwitch();
//	else
		nReturn = m_nHotKeyStatus;

	if (nReturn != nSaveReturn)		nSaveReturn = nReturn;

	return nReturn;
}


/////////////////////////////////////////////////////////////////////////////
// Get SENSOR Enhanced Audio Status
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnSNS_GetEnhancedAudio()
{
	// [#2220] NH KMK 2014.01.27 APP_SHOW_MODE 관련 로직 미사용하므로 주석처리
// 	#ifdef APP_SHOW_MODE	// [#429] [NH] KSK 2008.9.26
// 		return FALSE;
// 	#endif					// end of [#429]
	// end of [#2220]

#ifdef APP_LOCAL_MODE		// Local Mode시 ADA 미지원
	return FALSE;
#endif

	int		nReturn = FALSE;

//	#if !US_VERSION
#if !(US_VERSION || AU_VERSION||CA_VERSION)		// [#2047] AU KSK 2011.04.13 (호주만 지원인지, 뉴질랜드도 지원해야 하는지 추가 문의 필요)
												// [#2077] CA PCS 2011.06.29 CANADA ADA지원
		return FALSE;
#endif

	if (fnAPL_GetDefineDevice(DEV_SNS))
		nReturn = m_pMwi->SnsGetEnhancedAudio();

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get SENSOR Proximity
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnSNS_GetProximity()
{
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_SNS))
		nReturn = m_pMwi->SnsGetProximity();

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
//	SENSOR FUNCTION(SNS) : SNS OPERATION MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Initialize
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnSNS_Initialize()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnSNS_Initialize() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_SNS))
	{
		nReturn = m_pMwi->SnsInitialize();
	}

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// De-initialize
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnSNS_Deinitialize()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnSNS_Deinitialize() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_SNS))
		nReturn = m_pMwi->SnsDeinitialize();

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Clear Event Device(Device Id)
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnCMN_ClearEventDevice(int nDevId)
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnCMN_ClearEventDevice() \n")));

	int		nReturn = FALSE;

	nReturn = m_pMwi->ClearEventDevice(nDevId);

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Clear Status Device(Device Id)
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnCMN_ClearStatusDevice(int nDevId)
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnCMN_ClearStatusDevice() \n")));

	int		nReturn = FALSE;

	nDevId = fnAPL_GetDefineDevice(nDevId);

	nReturn = m_pMwi->ClearStatusDevice(nDevId);

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
//	COMMON FUNCTION(CMN) : CMN GET MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Get Error Device(Device Id)
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnCMN_GetErrorDevice(int nDevId)
{

	int		nReturn = FALSE;

	nDevId = fnAPL_GetDefineDevice(nDevId);

	nReturn = m_pMwi->GetErrorDevice(nDevId);

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get Timeout Device(Device Id)
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnCMN_GetTimeoutDevice(int nDevId)
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnCMN_GetTimeoutDevice() \n")));

	int		nReturn = FALSE;

	nDevId = fnAPL_GetDefineDevice(nDevId);

	nReturn = m_pMwi->GetTimeoutDevice(nDevId);

NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnCMN_GetTimeoutDevice() Return(%d) \n"), nReturn));
	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get Event Device(Device Id)
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnCMN_GetEventDevice(int nDevId)
{
	int		nReturn = FALSE;

	nDevId = fnAPL_GetDefineDevice(nDevId);

	nReturn = m_pMwi->GetEventDevice(nDevId);

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get Status Device(Device Id)
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnCMN_GetStatusDevice(int nDevId)
{
	int		nReturn = FALSE;

	nDevId = fnAPL_GetDefineDevice(nDevId);

	nReturn = m_pMwi->GetStatusDevice(nDevId);

	return nReturn;
}

int	CDevCmn::fnCMN_GetActiveDevice()
{
	static int OldDeviceDefine = 0;

	if (OldDeviceDefine != DeviceDefine)
	{
		NHDEBUG(1, (_T("List of active devices\n")));

		if (fnAPL_GetDefineDevice(DEV_CDU) == DEV_CDU)			NHDEBUG(1, (_T(" - DEV_CDU\n")));
		if (fnAPL_GetDefineDevice(DEV_PIN) == DEV_PIN)			NHDEBUG(1, (_T(" - DEV_PIN\n")));
		if (fnAPL_GetDefineDevice(DEV_MCU) == DEV_MCU)			NHDEBUG(1, (_T(" - DEV_MCU\n")));
		if (fnAPL_GetDefineDevice(DEV_SPR) == DEV_SPR)			NHDEBUG(1, (_T(" - DEV_SPR\n")));
		if (fnAPL_GetDefineDevice(DEV_RFID) == DEV_RFID)		NHDEBUG(1, (_T(" - DEV_RFID\n")));
		if (fnAPL_GetDefineDevice(DEV_BCR) == DEV_BCR)			NHDEBUG(1, (_T(" - DEV_BCR\n")));
		if (fnAPL_GetDefineDevice(DEV_BNA) == DEV_BNA)			NHDEBUG(1, (_T(" - DEV_BNA\n")));

		OldDeviceDefine = DeviceDefine;
	}

	return DeviceDefine;
}

void CDevCmn::fnCMN_AddActiveDevice(int nDevId)
{
	NHDEBUG(1, (_T("Adding '%S' to active devices\n"), GET_DEVNAME_BY_ID(nDevId)));
	DeviceDefine |= nDevId;
	fnCMN_GetActiveDevice();
}

void CDevCmn::fnCMN_RemoveActiveDevice(int nDevId)
{
	NHDEBUG(1, (_T("Removing '%S' from active devices\n"), GET_DEVNAME_BY_ID(nDevId)));
	DeviceDefine &= ~nDevId;
	fnCMN_GetActiveDevice();
}


/////////////////////////////////////////////////////////////////////////////
//	COMMON FUNCTION(CMN) : CMN OPERATION MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Check Device Action(Device Id, Check Time)
/////////////////////////////////////////////////////////////////////////////
CString	CDevCmn::fstrCMN_CheckDeviceAction(int nDevId, int nCheckTime)
{
	CString			strReturn("");

	nDevId = fnAPL_GetDefineDevice(nDevId);

	strReturn = m_pMwi->CheckDeviceAction(nDevId, nCheckTime);

	if (strReturn != CHK_NORMAL)
	{
		//	NHDEBUG(1, (_T("***DevCmn***CDevCmn::fstrCMN_CheckDeviceAction() nDevId(%d) strReturn(%s) \n"), nDevId, strReturn));
		NHDEBUG(1, (_T("%S: [%s] \n"), GET_DEVNAME_BY_ID(nDevId), strReturn));
	}

	return strReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Scan Device Action(Device Id, Check Time, Event Kind)
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnCMN_ScanDeviceAction(int nDevId, int nScanTime, int nEventKind)
{
	int		nReturn = FALSE;

	nDevId = fnAPL_GetDefineDevice(nDevId);

	nReturn = m_pMwi->ScanDeviceAction(nDevId, nScanTime, nEventKind);

	return nReturn;
}


/////////////////////////////////////////////////////////////////////////////
//	CASH FUNCTION(CASH) : CASH SET MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Set Cash of Service Provider (Reject/Last Dispensed Cassette)
/////////////////////////////////////////////////////////////////////////////
//int	CDevCmn::fnCDU_SetCstInitialDispInfo(CString strField, int nTargetCst)
int	CDevCmn::fnCDU_SetCstInitialDispInfo(int nTargetCst)
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnCDU_SetCstInitialDispInfo() nTargetCst(%d) \n"), nTargetCst));

	int		nReturn = FALSE;

	if (nTargetCst == CDU_CST_ALL)
	{	
		// Dispensed Count(User Count)
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_DISPCASHCST1, 0);
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_DISPCASHCST2, 0);
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_DISPCASHCST3, 0);
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_DISPCASHCST4, 0);

		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE1, 0);
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE2, 0);
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE3, 0);
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE4, 0);
	}
	else if (nTargetCst == CDU_CST_1)
	{
		// Dispensed Count out of Cassette
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_DISPCASHCST1, 0);
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE1, 0);
	}
	else if (nTargetCst == CDU_CST_2)
	{
		// Dispensed Count out of Cassette
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_DISPCASHCST2, 0);
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE2, 0);
	}
	else if (nTargetCst == CDU_CST_3)
	{
		// Dispensed Count out of Cassette
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_DISPCASHCST3, 0);
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE3, 0);
	}
	else if (nTargetCst == CDU_CST_4)
	{
		// Dispensed Count out of Cassette
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_DISPCASHCST4, 0);
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE4, 0);
	}
	else if (nTargetCst == CDU_CST_REJECT)
	{
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE1, 0);
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE2, 0);
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE3, 0);
		MemSetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE4, 0);
	}

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
//	CASH FUNCTION(CASH) : CASH GET MODE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Get Amount Of CST
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnCDU_GetNumberOfAllCSTs()		// [#1] NH JSW 2008.02.28 Fast Cash 보완	[#507] [NH] KSK 2009.2.23 함수명 변경 및 Logic 변경
{
	int nTotalCstCount = 0;
	for (int i=0; i<CSTCnt; i++)
	{
		nTotalCstCount += fnCDU_GetNumberOfCash(CDU_CST_1 + i);
	}
	return nTotalCstCount;
}

/////////////////////////////////////////////////////////////////////////////
// Get Amount Of Cash(CST No)
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnCDU_GetAmountOfCash(int nCSTNo)
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fnCDU_GetAmountOfCash() nCSTNo(%d) \n"), nCSTNo));

	int		nAmount = 0;

	if (CSTCnt >= nCSTNo)
	{
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.06
		int	nCstLoc[CDU_MAX_CST_COUNT] = { FirstCSTLoc, SecondCSTLoc, ThirdCSTLoc, FourthCSTLoc };
		int nCstStatus[CDU_MAX_CST_COUNT] = { FirstCSTStatus, SecondCSTStatus, ThirdCSTStatus, FourthCSTStatus };
		int nCstValue[CDU_MAX_CST_COUNT] = { FirstCSTValue, SecondCSTValue, ThirdCSTValue, FourthCSTValue };

		if (nCSTNo <= 4)	// codesonar 지적사항 대책
		{
			if ((nCstLoc[nCSTNo-1] == NORMAL_POS) && 
				((nCstStatus[nCSTNo-1] == CST_NORMAL) || (nCstStatus[nCSTNo-1] == CST_NEAR)))
			{
				if (m_strCurrencyID == CURRENCY_TYPE)
				{
					if (nCstValue[nCSTNo-1] == CASH_DENOMINATION8 || nCstValue[nCSTNo-1] == CASH_DENOMINATION7 ||
						nCstValue[nCSTNo-1] == CASH_DENOMINATION6 || nCstValue[nCSTNo-1] == CASH_DENOMINATION5 ||
						nCstValue[nCSTNo-1] == CASH_DENOMINATION4 || nCstValue[nCSTNo-1] == CASH_DENOMINATION3 ||
						nCstValue[nCSTNo-1] == CASH_DENOMINATION2 || nCstValue[nCSTNo-1] == CASH_DENOMINATION1)
					{
						nAmount += fnCDU_GetNumberOfCash(nCSTNo) * nCstValue[nCSTNo-1];
					}
				}
				else
				{
					if (nCstValue[nCSTNo-1] == MULTI_CASH_DENOMINATION8 || nCstValue[nCSTNo-1] == MULTI_CASH_DENOMINATION7 ||
						nCstValue[nCSTNo-1] == MULTI_CASH_DENOMINATION6 || nCstValue[nCSTNo-1] == MULTI_CASH_DENOMINATION5 ||
						nCstValue[nCSTNo-1] == MULTI_CASH_DENOMINATION4 || nCstValue[nCSTNo-1] == MULTI_CASH_DENOMINATION3 ||
						nCstValue[nCSTNo-1] == MULTI_CASH_DENOMINATION2 || nCstValue[nCSTNo-1] == MULTI_CASH_DENOMINATION1)
					{
						nAmount += fnCDU_GetNumberOfCash(nCSTNo) * nCstValue[nCSTNo-1];
					}				
				}
			}
		}
#else
		// [#282] [NH] KSK 2008.6.11
		// 국가별 권종 지원으로 인해 DEFINE 변경
		if ((nCSTNo == CDU_CST_1) && 
				(FirstCSTLoc == NORMAL_POS) &&
					((FirstCSTStatus == CST_NORMAL) || (FirstCSTStatus == CST_NEAR)) &&
						(fnCDU_GetNumberOfCash(nCSTNo) > CDU_EMPTYCASHCNT))
		{
			if (FirstCSTValue == CASH_DENOMINATION8 || FirstCSTValue == CASH_DENOMINATION7 ||
				FirstCSTValue == CASH_DENOMINATION6 || FirstCSTValue == CASH_DENOMINATION5 ||
				FirstCSTValue == CASH_DENOMINATION4 || FirstCSTValue == CASH_DENOMINATION3 ||
				FirstCSTValue == CASH_DENOMINATION2 || FirstCSTValue == CASH_DENOMINATION1)
				nAmount += fnCDU_GetNumberOfCash(nCSTNo) * FirstCSTValue;
		}

		if ((nCSTNo == CDU_CST_2) &&
				(SecondCSTLoc == NORMAL_POS) &&
					((SecondCSTStatus == CST_NORMAL) || (SecondCSTStatus == CST_NEAR)) &&
						(fnCDU_GetNumberOfCash(nCSTNo) > CDU_EMPTYCASHCNT))
		{
			if (SecondCSTValue == CASH_DENOMINATION8 || SecondCSTValue == CASH_DENOMINATION7 ||
				SecondCSTValue == CASH_DENOMINATION6 || SecondCSTValue == CASH_DENOMINATION5 ||
				SecondCSTValue == CASH_DENOMINATION4 || SecondCSTValue == CASH_DENOMINATION3 ||
				SecondCSTValue == CASH_DENOMINATION2 || SecondCSTValue == CASH_DENOMINATION1)
				nAmount += fnCDU_GetNumberOfCash(nCSTNo) * SecondCSTValue;
		}

		if ((nCSTNo == CDU_CST_3) &&
				(ThirdCSTLoc == NORMAL_POS) &&
					((ThirdCSTStatus == CST_NORMAL) || (ThirdCSTStatus == CST_NEAR)) &&
						(fnCDU_GetNumberOfCash(nCSTNo) > CDU_EMPTYCASHCNT))
		{
			if (ThirdCSTValue == CASH_DENOMINATION8 || ThirdCSTValue == CASH_DENOMINATION7 ||
				ThirdCSTValue == CASH_DENOMINATION6 || ThirdCSTValue == CASH_DENOMINATION5 ||
				ThirdCSTValue == CASH_DENOMINATION4 || ThirdCSTValue == CASH_DENOMINATION3 ||
				ThirdCSTValue == CASH_DENOMINATION2 || ThirdCSTValue == CASH_DENOMINATION1)
				nAmount += fnCDU_GetNumberOfCash(nCSTNo) * ThirdCSTValue;
		}

		if ((nCSTNo == CDU_CST_4) &&
				(FourthCSTLoc == NORMAL_POS) &&
					((FourthCSTStatus == CST_NORMAL) || (FourthCSTStatus == CST_NEAR)) &&
						(fnCDU_GetNumberOfCash(nCSTNo) > CDU_EMPTYCASHCNT))
		{
			if (FourthCSTValue == CASH_DENOMINATION8 || FourthCSTValue == CASH_DENOMINATION7 ||
				FourthCSTValue == CASH_DENOMINATION6 || FourthCSTValue == CASH_DENOMINATION5 ||
				FourthCSTValue == CASH_DENOMINATION4 || FourthCSTValue == CASH_DENOMINATION3 ||
				FourthCSTValue == CASH_DENOMINATION2 || FourthCSTValue == CASH_DENOMINATION1)
				nAmount += fnCDU_GetNumberOfCash(nCSTNo) * FourthCSTValue;
		}
		// end of [#282]
#endif			// end of [#2115]
	}

	return nAmount;
}

/////////////////////////////////////////////////////////////////////////////
// Get Number Of Dispense Cash(CST No)
/////////////////////////////////////////////////////////////////////////////
int	CDevCmn::fnCDU_GetNumberOfDispenseCash(int nCSTNo)
{
	int		nReturn = 0;

	if (CSTCnt >= nCSTNo)								// 2004.08.23
	{
		if (nCSTNo == CDU_CST_1)
			nReturn = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_DISPCASHCST1);
		if (nCSTNo == CDU_CST_2)
			nReturn = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_DISPCASHCST2);
		if (nCSTNo == CDU_CST_3)
			nReturn = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_DISPCASHCST3);
		if (nCSTNo == CDU_CST_4)
			nReturn = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_DISPCASHCST4);
	}

	NHDEBUG(1, (_T(" CST%d: %d \n"), nCSTNo, nReturn));

	return nReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Get Last Dispensed Notes Amount (To Customer)
/////////////////////////////////////////////////////////////////////////////
CString	CDevCmn::fstrCDU_GetLastDispensedAmount()
{
NHDEBUG(1, (_T("***DevCmn***CDevCmn::fstrCDU_GetLastDispensedAmount()  \n")));

	CString	strReturn("0");

	strReturn = Int2Asc(fnCDU_GetLastDispensedAmount());

	if (strReturn == "0")
		return "0";
	else
		strReturn += "00";
	return strReturn;
}

//[#522] US KSK 2009.05.19 DevCtrl로 이동
/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: LoadBinFromFile()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 거래 가능 은행 ID List를 파일로 부터 로드한다.
-------------------------------------------------------------------*/
void CDevCmn::LoadBinFromFile(CString strFileName, CStringArray &strLoadedBinList)
{
	CFile datFile;
	CString strData;
	CString	strBIN;
	CFileException e;
	if(datFile.Open(strFileName, CFile::modeRead, &e))
	{
		strLoadedBinList.RemoveAll();

		UINT nSize = (UINT)datFile.GetLength();
		BYTE *pBuffer = new BYTE[nSize+1]();
		if( datFile.Read(pBuffer, nSize) == nSize )
		{
			strData = CString(pBuffer);
			strData.MakeUpper();

			int i, nStart = 0, nEnd = 0;
			CString strTmp;
			//for(i=1; i<=3000; i++)		// [#522] US KSK 2009.05.19 3000개까지 지원하도록 수정
			for(i=1; i<=10000; i++)			// [#2209] NH Justin 2013.07.11 Extended bin - support up to 10,000
			{
				strTmp.Format(L"BIN%d=", i);
				nStart = strData.Find(strTmp, nStart);
				if( nStart == -1 )		break;
				else
				{
					nStart += strTmp.GetLength();
					nEnd = strData.Find(L"\r\n", nStart);
					if( nEnd == -1 )	nEnd = strData.GetLength();

					// KSK 2009.06.08 BIN값이 NULL일 경우 Bug Fix
					strBIN = strData.Mid(nStart, nEnd - nStart);
					strBIN.TrimLeft();
					strBIN.TrimRight();
					if (strBIN.GetLength() > 0)
						strLoadedBinList.Add(strBIN);
					// end of KSK 2009.06.08

					nStart = nEnd + 2;
				}
			}
		}
		delete [] pBuffer;
		datFile.Close();
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: IsBinRestrictionList()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 주어진 Data에 거래 가능 BIN이 있는지 검사한다.
-------------------------------------------------------------------*/
BOOL CDevCmn::IsBinExist(CString strTrackData, CStringArray &strLoadedBinList)
{
	CString strBIN;
	int nCount = strLoadedBinList.GetSize();
	for(int i=0; i<nCount; i++)
	{
		strBIN = strLoadedBinList.GetAt(i);
		if( strBIN == strTrackData.Left(strBIN.GetLength()) )
		{
			NVDump('O', 'E', "43", L"", L"BIN_DETECTED");	// [#2379] AU KSK 2015.11.17 NVLog 추가
			return TRUE;
		}
	}
	return FALSE;
}
// end of [#522]

// [#2283] NH Justin Implement Customized Hot Key
BOOL CDevCmn::IsValidHotKey(CString strKeyString)
{
	NHDEBUG(1, (_T("HOTKEY CHECK - [%s]\n"), strKeyString));

	CString strNewHotKey = strKeyString;
	if(strNewHotKey.GetLength() < 11)		// Minimum Length "1,1,1,1,1,1" => length = 11
	{
		NHDEBUG(1, (_T("HOTKEY CHECK - ERROR (too short : [%d])\n"), strNewHotKey.GetLength() ) );
		return FALSE;
	}

	CString strRightOne = strNewHotKey.Right(1);
	CString strLeftOne = strNewHotKey.Left(1);
	if( (strRightOne == ",") || ( strLeftOne == ",") )		// Starting or Ending with "," is Invalid
	{
		NHDEBUG(1, (_T("HOTKEY CHECK - ERROR (Left or Right is Comma.)\n") ) );
		return FALSE;
	}

	CStringArray	strArrayHotKey;
	SplitString(strNewHotKey, ",", strArrayHotKey);
	if( strArrayHotKey.GetSize() != 6 )
	{
		NHDEBUG(1, (_T("HOTKEY CHECK - ERROR (not six digit : [%d])\n"), strArrayHotKey.GetSize() ) );
		return FALSE;
	}

	for(int i=0; i<6; i++)
	{
		CString strTmp = strArrayHotKey.GetAt(i);
		if(strTmp.GetLength() == 1)
		{
			char chKey = (char)strTmp.GetAt(0);
			if( (chKey < '0') || (chKey > '9') )
			{
				NHDEBUG(1, (_T("HOTKEY CHECK - ERROR (one digit is not number : [%c])\n"), chKey ) );
				return FALSE;
			}
		}
		else
		{
			if( (strTmp != "ENTER") && (strTmp != "CLEAR") && (strTmp != "CANCEL") )
			{
				NHDEBUG(1, (_T("HOTKEY CHECK - ERROR (Not Enter, clear, clear : [%s])\n"), strTmp ) );
				return FALSE;
			}
		}
	}
	return TRUE;
}


void CDevCmn::ReArrangeEnteredHotKey()
{
	if( m_strHotKey.GetLength() < 11 )
		return;

	CStringArray	strArrayHotKey;
	SplitString(m_strHotKey, ",", strArrayHotKey);
	if( strArrayHotKey.GetSize() < 7 )
		return;

	// Shift Left.......
	m_strHotKey =	strArrayHotKey.GetAt(1) + "," + strArrayHotKey.GetAt(2) + "," + strArrayHotKey.GetAt(3) + "," + 
					strArrayHotKey.GetAt(4) + "," + strArrayHotKey.GetAt(5) + "," + strArrayHotKey.GetAt(6) + ",";	
}
// End of [#2283]

// [#2379] AU KSK 2015.11.17
/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: LoadAUDomesticBinFromFile()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : IC거래가 아닌 FAllBack 거래용 ID List를 파일로 부터 로드한다.
-------------------------------------------------------------------*/
int CDevCmn::LoadAUDomesticBinFromFile(CString strFileName, CStringArray &strLoadedBinList)
{
	CFile datFile;
	CString strData, strBIN, strTotalCnt;
	int nAuDomesticBinTotalCnt = 0;
	CFileException e;

	CString strVersionInfo;			// [#2424] AU KSK 2016.05.12 Default Version is 0
	int nDomesticBinVersion = 0;	// [#2424] AU KSK 2016.05.12 Default Version is 0

	strLoadedBinList.RemoveAll();	// 추가 보완처리

	if(datFile.Open(strFileName, CFile::modeRead, &e))
	{
//		strLoadedBinList.RemoveAll();

		UINT nSize = (UINT)datFile.GetLength();
		BYTE *pBuffer = new BYTE[nSize+1]();
		if( datFile.Read(pBuffer, nSize) == nSize )
		{
			strData = CString(pBuffer);
			strData.MakeUpper();

			int nStart = 0, nEnd = 0;
			CString strTmp;

			// [#2424] AU KSK 2016.05.12 Version 정보 오류시 BIN Read Fail로 처리
			strTmp.Format(L"VERSION_INFO=");
			nStart = strData.Find(strTmp, nStart);

			if (nStart == -1)
			{
				// Version Information is missing 
				NVDump('F', 'E', "43", L"", L"VER_ERR");
				return 0;
			}

			nStart += strTmp.GetLength();
			nEnd = strData.Find(L"\r\n", nStart);

			if( nEnd == -1 )
			{
				// Line Feed Error
				NVDump('F', 'E', "43", L"", L"VER_ERR1");
				return 0;
			}

			strVersionInfo = strData.Mid(nStart, nEnd - nStart);
			nDomesticBinVersion = Asc2Int(strVersionInfo);

			if (nDomesticBinVersion <= 0 || nDomesticBinVersion > 9)
			{
				NVDump('F', 'E', "43", L"", L"VER_ERR2");
				return 0;
			}

			NVDump('F', 'E', "43", L"", L"VER_OK");
			nStart = 0;
			nEnd = 0;
			// end of [#2424]
			
			strTmp.Format(L"DOMESTIC_BIN_COUNT=");
			nStart = strData.Find(strTmp, nStart);

			if (nStart == -1)
			{
				// Total Count is missing 
				NVDump('F', 'E', "43", L"", L"TOTAL_ERR");
				return 0;	// [#2424] AU KSK 2016.05.12
			}
			else
			{
				nStart += strTmp.GetLength();
				nEnd = strData.Find(L"\r\n", nStart);

				if( nEnd == -1 )
				{
					// Line Feed Error
					NVDump('F', 'E', "43", L"", L"TOTAL_ERR1");
					return 0;	// [#2424] AU KSK 2016.05.12
				}

				strTotalCnt = strData.Mid(nStart, nEnd - nStart);
				nAuDomesticBinTotalCnt = Asc2Int(strTotalCnt);

				strTmp.Format(L"D_BIN CNT:%s", strTotalCnt);
				if (nAuDomesticBinTotalCnt <= 0)
				{
					// Total Count 0 or under
					NVDump('F', 'E', "43", L"", L"TOTAL_ERR2");
					return 0;	// [#2424] AU KSK 2016.05.12
				}

				if (nAuDomesticBinTotalCnt > 9999)
				{
					// Max Total Count Error
					NVDump('F', 'E', "43", L"", L"TOTAL_ERR3");
					return 0;	// [#2424] AU KSK 2016.05.12
				}

				strTmp.Format(L"D_BIN_CNT(%d)", nAuDomesticBinTotalCnt);
				NVDump('F', 'E', "43", L"", strTmp);

				// Normal Count
				for (int i=0; i<nAuDomesticBinTotalCnt; i++)
				{
					strTmp.Format(L"DOMESTIC_BIN_%04d=", i+1);
					nStart = strData.Find(strTmp, nStart);

					if( nStart == -1 )
					{
						NVDump('F', 'E', "43", L"", L"D_BIN_ERR");
						strLoadedBinList.RemoveAll();	// List를 모두 삭제
						return 0;	// [#2424] AU KSK 2016.05.12
					}
					else
					{
						nStart += strTmp.GetLength();
						nEnd = strData.Find(L"\r\n", nStart);
						if( nEnd == -1 )	nEnd = strData.GetLength();

						strBIN = strData.Mid(nStart, nEnd - nStart);
						strBIN.TrimLeft();
						strBIN.TrimRight();

						if (strBIN.GetLength() > 0)
							strLoadedBinList.Add(strBIN);

						nStart = nEnd + 2;
					}
				}
			}
		}
		delete [] pBuffer;
		datFile.Close();
	}
	return nDomesticBinVersion;	// [#2424] AU KSK 2016.05.12
}
// end of [#2379]

// [#2342] US Justin 2015.05.12 AID Selection
#define MAX_AIDLIST_CNT 60
BOOL CDevCmn::Create_EMV_Termdata_From_POOL()
{
	NHDEBUG(1, (_T("Create_EMV_Termdata_From_POOL()\n")) );

	CString strTemp;
	FILE	*Stream;

	DWORD	dwFileWriten = 0;
	char	buf[512] = { 0, };
	char	bufContents[512] = { 0, };
	char	szIndex[16]	= { 0, };
	int		i;

	char chAPVer[5] = {0,};	// [#2342] US Justin 2015.05.05 AID Selection

	//CString strAIDEnDisable = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_AIDENDISABLE);
	CString strAIDEnDisable = MemGetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID);		// [#2342] US Justin 2015.05.07 AID Selection

	// Open AID Pool
	CINIFile iniEMV(GET_EMV_AID_POOLFILENAME(fnEMV_GetKernelVersion()) );

	// Total number of listed AID
	iniEMV.GetPrivateProfileString("AIDCOUNT","TOTALCOUNT",buf, 512);
	strTemp.Format(L"%S",buf);
	int nListedTotalAIDList = Asc2Int(strTemp);

	// Delete Existing TERM_DATA File
	strTemp.Format(L"%S", GET_EMV_AID_WORKFILENAME(fnEMV_GetKernelVersion()) );
	DeleteFile(strTemp);

	// Create new TERM_DATA File
	if ( (Stream = _tfopen(strTemp, _T("wb+"))) == NULL)
	{
		NHDEBUG(1, (_T("***BIZ_EMV_SaveAIDList*** File Open Fail[%s]\n"), strTemp));
		return FALSE;
	}

	// [#2342] US Justin 2015.05.07 Save Total Count Last / Change making INI with enabled AID List
	// Write Each AID inforamtion
	int nEnabledAID = 0;
	for(i=0; i<__min(nListedTotalAIDList, MAX_AIDLIST_CNT); i++)
	{
		sprintf_s(szIndex, sizeof(szIndex), "TERMINAL%d", i+1);

		// Read AID
		memset(bufContents, 0, sizeof(bufContents));
		iniEMV.GetPrivateProfileString(szIndex,"AID",bufContents, 512); 
		strTemp.Format( L"%S", bufContents );
	
		// Validate AID .... Check Length for now....
		if( (strTemp.GetLength()>=10) && (strAIDEnDisable.Find(strTemp)>=0) )
		{
			NHDEBUG(1, (_T(" AID [%s] is Enabled . Store in INI FILE\n"), strTemp));

			// section : [TERMINAL1], [TERMINAL2], ...
			memset(buf, 0, sizeof(buf));
			sprintf_s(buf, sizeof(buf), "[TERMINAL%d]\r\n", nEnabledAID+1);
			dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);

			// application name : APPLICATIONNAME=VISA, APPLICATIONNAME=MASTER
			memset(bufContents, 0, sizeof(bufContents));	memset(buf, 0, sizeof(buf));
			iniEMV.GetPrivateProfileString(szIndex,"APPLICATIONNAME",bufContents, 512); 			
			sprintf_s(buf, sizeof(buf), "APPLICATIONNAME=%s\r\n", bufContents);
			dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);

			// application index : APPLICATION=00, APPLICATION=01
			memset(buf, 0, sizeof(buf));
			sprintf_s(buf, sizeof(buf), "APPLICATION=%02d\r\n", nEnabledAID);
			dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);

			// AID : AID=A0000000041010 
			memset(bufContents, 0, sizeof(bufContents));	memset(buf, 0, sizeof(buf));
			iniEMV.GetPrivateProfileString(szIndex,"AID",bufContents, 512); 
			sprintf_s(buf, sizeof(buf), "AID=%s\r\n", bufContents);
			dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);

			// Terminal Len : TERMINALLEN=120 (FIXED)
			memset(buf, 0, sizeof(buf));
			sprintf_s(buf, sizeof(buf), "TERMINALLEN=120\r\n");	// fixed 
			dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);

			// Terminal Data : TERMINALDATA=D9050000000000DA05FFFFFFFFFFD805FFFFFFFFFF......
			memset(bufContents, 0, sizeof(bufContents));	memset(buf, 0, sizeof(buf));
			iniEMV.GetPrivateProfileString(szIndex,"TERMINALDATA",bufContents, 512); 
			if( fnEMV_GetKernelVersion() == EMV_KERNEL_V4 )
			{
				// Kernel 4 TERMDATA includes all parameters.
				sprintf_s(buf, sizeof(buf), "TERMINALDATA=%s\r\n\r\n", bufContents);		
			}
			else
			{
				// Kernel 5 TERMDATA includes only TAC values. (D9050000000000DA05FFFFFFFFFFD805FFFFFFFFFF : 42 bytes)
				Get_AID_AP_VersionNumber(strTemp, chAPVer, sizeof(chAPVer)); 
				sprintf_s(buf, sizeof(buf), "TERMINALDATA=%42.42s%s%4.4s%s\r\n\r\n", bufContents, TAG_EMV_APVERSION, chAPVer,GET_EMV_DEFAULT_TERMINALDATA(fnEMV_GetKernelVersion()));
			}
			dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);
			nEnabledAID++;
		}
	}

	// Save Total number of Enabled AIDs.
	// section  [AIDCOUNT]
	dwFileWriten = fwrite("[AIDCOUNT]\r\n", sizeof(char), strlen("[AIDCOUNT]\r\n"), Stream);
	memset(buf, 0, sizeof(buf));
	sprintf_s(buf, sizeof(buf), "TOTALCOUNT=%d\r\n\r\n", nEnabledAID);
	dwFileWriten = fwrite(buf, sizeof(char), strlen(buf), Stream);
	NHDEBUG(1, (_T("Total Enabled AID = [%d]\n"), nEnabledAID));
	// End of [#2342]

	fflush(Stream);
	fclose(Stream);
	return TRUE;

}

void CDevCmn::Get_AID_AP_VersionNumber(CString strAID, char* pVersion, int lenVersion)
{
	if(strAID.GetLength()>=10)
	{
		CString strAID10 = strAID.Left(10);
		if(      strAID10.CompareNoCase(L"A000000003") == 0)		// VISA, VISA ELECTRON, VISA PLUS
			sprintf_s(pVersion, lenVersion, "008C");
		else if( strAID10.CompareNoCase(L"A000000098") == 0)		// US VISA
			sprintf_s(pVersion, lenVersion, "008C");
		else if( strAID10.CompareNoCase(L"A000000004") == 0)		// MasterCard, MAESTRO, CIRRUS, US MAESTRO
			sprintf_s(pVersion, lenVersion, "0002");
		else if( strAID10.CompareNoCase(L"A000000065") == 0)		// JCB
			sprintf_s(pVersion, lenVersion, "0200");
		else if( strAID10.CompareNoCase(L"A000000333") == 0)		// UnionPay Debit, Credit, Quiasi Credit, US UnionPay
			sprintf_s(pVersion, lenVersion, "0020");
		else if( strAID10.CompareNoCase(L"A000000277") == 0)		// Interac
			sprintf_s(pVersion, lenVersion, "0001");
		else if( strAID10.CompareNoCase(L"A000000152") == 0)		// PULSE D-PAS(Discover), US Discover
			sprintf_s(pVersion, lenVersion, "0001");
		else if( strAID10.CompareNoCase(L"A000000025") == 0)		// AMEX
			sprintf_s(pVersion, lenVersion, "0001");
		else if( strAID10.CompareNoCase(L"A000000620") == 0)		// US DNA
			sprintf_s(pVersion, lenVersion, "0001");
		// Added 2016.08.26
		else if( strAID10.CompareNoCase(L"A000000384") == 0)		// AU eftpos
			sprintf_s(pVersion, lenVersion, "0100");
		// end of 2016.08.26
		else														
			sprintf_s(pVersion,lenVersion,  "0001");
	}
	else
		sprintf_s(pVersion, lenVersion, "0001");
}
// End of [#2342]

/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: IsValidSurchargeTable()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Checking Table Surcharge...
-------------------------------------------------------------------*/
BOOL CDevCmn::IsValidSurchargeTable()											// [#2389] US Justin Change Function Name
{
	char		chBuffer[1024] = {};
	CINIFile	strLoadFile(SINFO_DESFILE);
	CString		strTemp;
	int			i = 0;
	CStringArray	strArrTemp;

	memset(m_nSurchTable, 0, sizeof(m_nSurchTable));

	// [#2264]NH Justin 2014.05.05 Remove Table Surcharge Restriction.
	/*
	CString		strTerminalIDFromNVRam;
	CString		strSrcChkSumResult, strDestChkSumResult;

	strArrTemp.RemoveAll();
	memset(chBuffer, 0, sizeof(chBuffer));

	if(strLoadFile.GetPrivateProfileString("INFO", "CSUM", chBuffer, sizeof(chBuffer)) == FALSE)
		goto ERR_INVALID;

	// Check Sum Found and Get Dest Check Sum
	strDestChkSumResult.Format(L"%S", chBuffer);
	strDestChkSumResult.MakeUpper();	// 대문자로 변경
	strDestChkSumResult.TrimRight();
	strDestChkSumResult.TrimLeft();

	// Get Terminal ID
	strTerminalIDFromNVRam = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTerminalIDFromNVRam.TrimRight();
	strTerminalIDFromNVRam.TrimLeft();

	// Get Source Check Sum
	strSrcChkSumResult = GetSurchTableLicense(strTerminalIDFromNVRam);
	strSrcChkSumResult.MakeUpper();

	// Compare CheckSumResult
	if(strSrcChkSumResult != strDestChkSumResult)
		goto ERR_INVALID;
	*/
	// End of [#2264]

	// Check Sum이 유요한 경우 Surcharge Table을 Get한다.
	strArrTemp.RemoveAll();

	if(strLoadFile.GetPrivateProfileString("INFO", "WAMOUNT", chBuffer, sizeof(chBuffer)) == FALSE)
		goto ERR_INVALID;

	// Get Withdrawal Amount Limit
	strTemp.Format(L"%S", chBuffer);
	SplitString(strTemp, L",", strArrTemp);

	// Table 등록이 10개가 안되어져 있을 경우 Valid하지 않도록 처리=> 사양 협의 필요 (일단 Invalid 처리)
	if (strArrTemp.GetSize() != 10)
		goto ERR_INVALID;

	// Value Validation Check (Numeric - Dollar)
	for(i=0; i<10; i++)
	{
		if (IsNum(strArrTemp[i]) != TRUE)
			goto ERR_INVALID;

		m_nSurchTable[0][i] = Asc2Int(strArrTemp[i]);	// Withdrawal Amount 입력
	}

	// Get Surcharge Table Value
	memset(chBuffer, 0, sizeof(chBuffer));
	strArrTemp.RemoveAll();

	if(strLoadFile.GetPrivateProfileString("INFO", "SURCHARGE", chBuffer, sizeof(chBuffer)) == FALSE)
		goto ERR_INVALID;

	strTemp.Format(L"%S", chBuffer);
	SplitString(strTemp, L",", strArrTemp);

	// Table 등록이 10개가 안되어져 있을 경우 Valid하지 않도록 처리=> 사양 협의 필요 (일단 Invalid 처리)
	if (strArrTemp.GetSize() != 10)
		goto ERR_INVALID;

	for(i=0; i<10; i++)
	{
		if (IsNum(strArrTemp[i]) != TRUE)
			goto ERR_INVALID;

		m_nSurchTable[1][i] = Asc2Int(strArrTemp[i]);	// Surcharge Amount 입력
	}

	// File 및 Value가 정상적으로 Check 되었으므로 Withdrawal 금액을 작은순으로 Sorting 후 리턴								
	// 3. Sort in ascending order
	int nWithAmtTemp, nSurchAmtTemp;

	for (i=0; i<10; i++)
	{
		for (int j=i+1; j<10; j++)
		{
			if (m_nSurchTable[0][i] > m_nSurchTable[0][j])
			{
				nWithAmtTemp  = m_nSurchTable[0][i];
				nSurchAmtTemp = m_nSurchTable[1][i];

				m_nSurchTable[0][i] = m_nSurchTable[0][j];
				m_nSurchTable[1][i] = m_nSurchTable[1][j];
				m_nSurchTable[0][j] = nWithAmtTemp;
				m_nSurchTable[1][j] = nSurchAmtTemp;
			}
		}
	}

	return TRUE;

ERR_INVALID:
	// Table Value Initialize
	memset(m_nSurchTable, 0, sizeof(m_nSurchTable));

	// 이전 MODE가 TABLE인데 Invalid인 경우에는 MODE 사용 안함으로 강제로 설정함 (사양 협의)
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) == TABLE_MODE)
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE,NOT_USED);

	return FALSE;
}

// [#2167] NH Justin 2012.11.14 Check whether Antiskimming is available or not
/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fbESU_IsAntiSkimmingAvailable()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Check Antiskimming is availability
-------------------------------------------------------------------*/
BOOL  CDevCmn::fbESU_IsAntiSkimmingAvailable()
{
	BOOL bEnable = FALSE;
	if (GetDeviceType(L"MCU") == MCU_SANKYO_DIP)
	{
		CString sSankyoEP = MemGetVersion(_MEMKEY_EPVERSION, L"IDC");
		if( sSankyoEP.GetLength()>3 )  // Version 3XXX-XXX : At least 4 character....
		{
			CString sVer1 = sSankyoEP.Left(1);
			if(sVer1.CompareNoCase(L"3") == 0)
					bEnable = TRUE;	
		}
	}
	return bEnable;
}
// End of [#2167]

// [#2458] AU KSK 2016.12.26
/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fbIDC_IsLatchOptionAvailable()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Check Latch Option is availability
-------------------------------------------------------------------*/
BOOL  CDevCmn::fbIDC_IsLatchOptionAvailable()
{
	BOOL bEnable = FALSE;

	if (GetDeviceType(L"MCU") == MCU_SANKYO_DIP)
	{
		CString sSankyoEP = MemGetVersion(_MEMKEY_EPVERSION, L"IDC");

		// Remove Space 
		sSankyoEP.TrimLeft();
		sSankyoEP.TrimRight();

		// Check Sankyo Card Reader Model
		CString sVer1, sModel;

		sModel = sSankyoEP.Left(7);	// compare 7 character
		if (sModel.CompareNoCase(L"2420-03") == 0)
		{
			// ICM300-R1372 Model
			sVer1 = sSankyoEP.Right(1);
			if (sVer1.CompareNoCase(L"D") >= 0)
				bEnable = TRUE;
		}
		else if (sModel.CompareNoCase(L"4707-02") == 0)
		{
			// ICM300-R0775 Model
			sVer1 = sSankyoEP.Right(1);
			if (sVer1.CompareNoCase(L"E") >= 0)
				bEnable = TRUE;
		}
		else if (sModel.CompareNoCase(L"3898-02") == 0)
		{
			// ICM30A-R1575 Model
			sVer1 = sSankyoEP.Right(1);
			if (sVer1.CompareNoCase(L"C") >= 0)
				bEnable = TRUE;
		}
		else
		{
			// 그 외의 Firmware 버전은 미지원으로 return함.
		}
	}
	else if (GetDeviceType(L"MCU") == MCU_MAGTEK_DIP)
	{
		bEnable = TRUE;
	}

	return bEnable;
}
// end of [#2458]

// [#2449] US Justin Make Function
void CDevCmn::Disable_CardReader_RFID()
{
	// [#2510] US Justin 2017.10.05 Card Disable with Wait=TRUE Flag FOR "HOST/RMS connection of Idle screen"
	//m_pDevCmn->fnMCU_CardEnDisable(DISABLE, FALSE);
	fnMCU_CardEnDisable(DISABLE, TRUE);
	// End of [#2510]

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
	{
		fnRFID_EntryDisable();
		fnAPL_CheckDeviceAction(DEV_RFID);
	}
}
// end of [#2449]

//[#610] SOOK 2009.12.21 Parameter Change Journal 저장 
#if SUPPORT_CHANGE_PARAMETER_JNL 
/*-------------------------------------------------------------------
 FUNCTION NAME: SetPreviousParameter()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Parameter 변경 전에 NVRAM 데이타 저장 
-------------------------------------------------------------------*/
void	CDevCmn::SetPreviousParameter()
{

NHDEBUG(1, (_T("***CDevCmn::SetPreviousParameter() : Start *** \n")));

	pPreviousNetworkInfo	= pCurrentNVRam2->Network;
	wPreviousSpeakerVolume = pCurrentNVRam2->Device.wVolumeLevel; 

	pPreviousATMInfo 		= pCurrentNVRam3->APPATMInfo;

	pPreviousAdvInfo 		= pCurrentNVRam6->APPAdvInfo;
	pPreviousTCPIPInfo 		= pCurrentNVRam6->APPTCPIPInfo;
	pPreviousOptionalSetting = pCurrentNVRam6->APPOptionalSetting;			

	pPreviousOptionalSetting2 = pCurrentNVRam8->APPOptionalSetting2;	
	
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	wPreviousTotalBinCount	= m_BINMgr.GetTotalBinCount();
	wPreviousAllowOnlyBinEnable	= m_BINMgr.GetAllowOnlyBinEnable();		
#endif

	bAllocSucceed = TRUE;
NHDEBUG(1, (_T("***CDevCmn::SetPreviousParameter():End *** \n")));

}

/*-------------------------------------------------------------------
 FUNCTION NAME: SaveChangeParameter()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 변경 완료 후 미리 저장해 놓은 값고 현재 값을 비교하여 저널에 저장함. 
				OP 기능 추가시마다 추가할 껏 
-------------------------------------------------------------------*/
void	CDevCmn::SaveChangeParameter(int nUser)
{
NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() Start ***\n")));

	NETWORKINFO		pCurrentNetworkInfo		= pCurrentNVRam2->Network;
	APPATMINFO		pCurrentATMInfo 		= pCurrentNVRam3->APPATMInfo;
	ADVINFO			pCurrentAdvInfo 		= pCurrentNVRam6->APPAdvInfo;
	TCPIPINFO		pCurrentTCPIPInfo 		= pCurrentNVRam6->APPTCPIPInfo;
	OPTINALSETTING2 pCurrentOptionalSetting2 = pCurrentNVRam8->APPOptionalSetting2;
	OPTINALSETTING	pCurrentOptionalSetting = pCurrentNVRam6->APPOptionalSetting;			

	CString strOldValue, strNewValue;
//	CStringArray strOldValueArray, strNewValueArray;
	
	if ( wPreviousSpeakerVolume   != pCurrentNVRam2->Device.wVolumeLevel)
	{
		strOldValue.Format(L"%d",wPreviousSpeakerVolume);
		strNewValue.Format(L"%d",pCurrentNVRam2->Device.wVolumeLevel);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SPEAKER VOLUME", strOldValue, strNewValue); 
	}

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	if ( wPreviousTotalBinCount	!= m_BINMgr.GetTotalBinCount())
	{
		strOldValue.Format(L"%d",wPreviousTotalBinCount);
		strNewValue.Format(L"%d", m_BINMgr.GetTotalBinCount());
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"TOTAL BIN COUNT", strOldValue, strNewValue); 
	}

	if (wPreviousAllowOnlyBinEnable	!= m_BINMgr.GetAllowOnlyBinEnable())
	{
		strOldValue = GET_ENDIS_STRING(wPreviousAllowOnlyBinEnable);
		strNewValue = GET_ENDIS_STRING(m_BINMgr.GetAllowOnlyBinEnable());
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"ALLOW ONLY LISTED BIN", strOldValue, strNewValue); 
	}
#endif

	//NETWORK INFO

	if ( memcmp(&pPreviousNetworkInfo, &pCurrentNetworkInfo, sizeof(NETWORKINFO)) != 0)
	{
		SaveNetworkInfoParameter(nUser, &pPreviousNetworkInfo, &pCurrentNetworkInfo);
	}

	//ATMINFO
	if ( memcmp(&pPreviousATMInfo, &pCurrentATMInfo , sizeof(APPATMINFO)) != 0)
	{
		SaveATMInfoParameter(nUser, &pPreviousATMInfo, &pCurrentATMInfo);
	}

	if ( memcmp(&pPreviousAdvInfo, &pCurrentAdvInfo, sizeof(ADVINFO)) != 0)
	{
		SaveAdvertiseInfoParameter(nUser, &pPreviousAdvInfo, &pCurrentAdvInfo);
	}
	
	if ( memcmp(&pPreviousTCPIPInfo, &pCurrentTCPIPInfo, sizeof(TCPIPINFO)) != 0)
	{
		SaveTCPIPInfoParameter(nUser, &pPreviousTCPIPInfo, &pCurrentTCPIPInfo);
	}
	//OPTIONAL SETTING
	if ( memcmp(&pPreviousOptionalSetting, &pCurrentOptionalSetting, sizeof(OPTINALSETTING)) != 0)
	{
		SaveOptionalSettingParameter(nUser, &pPreviousOptionalSetting, &pCurrentOptionalSetting);
	}

	if ( memcmp(&pPreviousOptionalSetting2, &pCurrentOptionalSetting2, sizeof(OPTINALSETTING2)) != 0)
	{
		SaveOptionalSetting2Parameter(nUser, &pPreviousOptionalSetting2, &pCurrentOptionalSetting2);
	}


NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() End ***\n")));

}
/*-------------------------------------------------------------------
 FUNCTION NAME: SaveNetworkInfoParameter()
 RETURN TYPE  : 
 PARAMETER    : NETWORKINFO 
 DESCRIPTION  : 변경 완료 후 미리 저장해 놓은 값고 현재 값을 비교하여 저널에 저장함. 
-------------------------------------------------------------------*/
void CDevCmn::SaveNetworkInfoParameter(int nUser, LPNETWORKINFO pPreviousNetworkInfo, LPNETWORKINFO pCurrentNetworkInfo)
{
NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() ** NETWORKINFO** Start\n")));

	CString strOldValue, strNewValue;
	CStringArray strOldValueArray, strNewValueArray;

	if ( pPreviousNetworkInfo->wStatusMonitorFlag != pCurrentNetworkInfo->wStatusMonitorFlag)
	{
		strOldValue = GET_ENDIS_STRING(pPreviousNetworkInfo->wStatusMonitorFlag);
		strNewValue = GET_ENDIS_STRING(pCurrentNetworkInfo->wStatusMonitorFlag);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"STATUS MONITORING", strOldValue, strNewValue); 
	}
	if ( pPreviousNetworkInfo->wCommHeaderFlag  != pCurrentNetworkInfo->wCommHeaderFlag)
	{
		strOldValue = GET_ENDIS_STRING(pPreviousNetworkInfo->wCommHeaderFlag);
		strNewValue = GET_ENDIS_STRING(pCurrentNetworkInfo->wCommHeaderFlag);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"COMMUNICATION HEADER", strOldValue, strNewValue); 
	}
	
	if ( pPreviousNetworkInfo->wHealthSendDelayInterval  != pCurrentNetworkInfo->wHealthSendDelayInterval)
	{
		strOldValue.Format(L"%d HOURS", pPreviousNetworkInfo->wHealthSendDelayInterval);
		strNewValue.Format(L"%d HOURS", pCurrentNetworkInfo->wHealthSendDelayInterval);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"HEALTH CHECK INTERVAL", strOldValue, strNewValue); 
	}

	if ( pPreviousNetworkInfo->wHealthEnableFlag    != pCurrentNetworkInfo->wHealthEnableFlag)
	{
		strOldValue = GET_ENDIS_STRING(pPreviousNetworkInfo->wHealthEnableFlag);
		strNewValue = GET_ENDIS_STRING(pCurrentNetworkInfo->wHealthEnableFlag);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"HEALTH CHECK", strOldValue, strNewValue); 
	}
	if ( pPreviousNetworkInfo->wRMSRingCount     != pCurrentNetworkInfo->wRMSRingCount)
	{
		strOldValue.Format(L"%d",pPreviousNetworkInfo->wRMSRingCount);
		strNewValue.Format(L"%d",pCurrentNetworkInfo->wRMSRingCount);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RMS RING COUNT", strOldValue, strNewValue); 
	}

	if ( pPreviousNetworkInfo->wRMSSendFlag     != pCurrentNetworkInfo->wRMSSendFlag)
	{
		strOldValue = GET_ENDIS_STRING(pPreviousNetworkInfo->wRMSSendFlag);
		strNewValue = GET_ENDIS_STRING(pCurrentNetworkInfo->wRMSSendFlag);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RMS", strOldValue, strNewValue); 
	}

	if ( pPreviousNetworkInfo->wRMSStatusSend      != pCurrentNetworkInfo->wRMSStatusSend)
	{
		strOldValue = GET_ENDIS_STRING(pPreviousNetworkInfo->wRMSStatusSend);
		strNewValue = GET_ENDIS_STRING(pCurrentNetworkInfo->wRMSStatusSend);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RMS STATUS SEND", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousNetworkInfo->chCommID , pCurrentNetworkInfo->chCommID, sizeof( pPreviousNetworkInfo->chCommID)))
	{
		strOldValue = CString(pPreviousNetworkInfo->chCommID);
		strNewValue = CString(pCurrentNetworkInfo->chCommID);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"COMMUNICATION ID", strOldValue, strNewValue); 
	}
	if ( memcmp(pPreviousNetworkInfo->chCommHeader , pCurrentNetworkInfo->chCommHeader, sizeof( pPreviousNetworkInfo->chCommHeader)))
	{
		strOldValue = CString(pPreviousNetworkInfo->chCommHeader);
		strNewValue = CString(pCurrentNetworkInfo->chCommHeader);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"COMMUNICATION HEADER", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousNetworkInfo->chEOTCheckMode  , pCurrentNetworkInfo->chEOTCheckMode, sizeof( pPreviousNetworkInfo->chEOTCheckMode)))
	{
//	3 GENERAL //	5 NO ENQ REQUIRED //	2 EOT OPTIONAL //	4 NO EOT REQUIRED //	6 NO ENQ/EOT REQUIRED
		CString szTemp = CString(pPreviousNetworkInfo->chEOTCheckMode);
		if ( szTemp == L"5" ) strOldValue = L"NO ENQ REQUIRED";
		else if ( szTemp == L"2") strOldValue = L"EOT OPTIONAL";
		else if ( szTemp == L"5") strOldValue = L"NO EOT REQUIRED";
		else if ( szTemp == L"6") strOldValue = L"NO ENQ/EOT REQUIRED";
		else  strOldValue = L"GENERAL";

		szTemp = CString(pCurrentNetworkInfo->chEOTCheckMode);
		if ( szTemp == L"5" ) strNewValue = L"NO ENQ REQUIRED";
		else if ( szTemp == L"2") strNewValue = L"EOT OPTIONAL";
		else if ( szTemp == L"5") strNewValue = L"NO EOT REQUIRED";
		else if ( szTemp == L"6") strNewValue = L"NO ENQ/EOT REQUIRED";
		else  strNewValue = L"GENERAL";
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"EOT/ENQ OPTION", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousNetworkInfo->chPhoneNumber1  , pCurrentNetworkInfo->chPhoneNumber1, sizeof( pPreviousNetworkInfo->chPhoneNumber1)))
	{
		strOldValue = CString(pPreviousNetworkInfo->chPhoneNumber1);
		strNewValue = CString(pCurrentNetworkInfo->chPhoneNumber1);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"HOST PHONE NUMBER 1", strOldValue, strNewValue); 
	}
	if ( memcmp(pPreviousNetworkInfo->chPhoneNumber2   , pCurrentNetworkInfo->chPhoneNumber2, sizeof( pPreviousNetworkInfo->chPhoneNumber2)))
	{
		strOldValue = CString(pPreviousNetworkInfo->chPhoneNumber2);
		strNewValue = CString(pCurrentNetworkInfo->chPhoneNumber2);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"HOST PHONE NUMBER 2", strOldValue, strNewValue); 
	}
	if ( memcmp(pPreviousNetworkInfo->chModemInitialStr , pCurrentNetworkInfo->chModemInitialStr, sizeof( pPreviousNetworkInfo->chModemInitialStr)))
	{
		strOldValue = CString(pPreviousNetworkInfo->chModemInitialStr);
		strNewValue = CString(pCurrentNetworkInfo->chModemInitialStr);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"HOST MODEM INITIAL STRING", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousNetworkInfo->chRMSPassword , pCurrentNetworkInfo->chRMSPassword, sizeof( pPreviousNetworkInfo->chRMSPassword)))
	{
		strOldValue = L"******";//CString(pPreviousNetworkInfo->chRMSPassword);
		strNewValue = L"******";//CString(pCurrentNetworkInfo->chRMSPassword;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RMS PASSWORD", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousNetworkInfo->chRMSPhoneNum1    , pCurrentNetworkInfo->chRMSPhoneNum1, sizeof( pPreviousNetworkInfo->chRMSPhoneNum1)))
	{
		strOldValue = CString(pPreviousNetworkInfo->chRMSPhoneNum1);
		strNewValue = CString(pCurrentNetworkInfo->chRMSPhoneNum1);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RMS PHONE NUMBER 1", strOldValue, strNewValue); 
	}
	if ( memcmp(pPreviousNetworkInfo->chRMSPhoneNum2    , pCurrentNetworkInfo->chRMSPhoneNum2, sizeof( pPreviousNetworkInfo->chRMSPhoneNum2)))
	{
		strOldValue = CString(pPreviousNetworkInfo->chRMSPhoneNum2);
		strNewValue = CString(pCurrentNetworkInfo->chRMSPhoneNum2);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RMS PHONE NUMBER 2", strOldValue, strNewValue); 
	}
	if ( strOldValueArray.GetSize() > 0) strOldValueArray.RemoveAll();
	if ( strNewValueArray.GetSize() > 0) strNewValueArray.RemoveAll();

NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() ** NETWORKINFO** End\n")));

}
/*-------------------------------------------------------------------
 FUNCTION NAME: SaveATMInfoParameter()
 RETURN TYPE  : 
 PARAMETER    : APPATMINFO 
 DESCRIPTION  : 변경 완료 후 미리 저장해 놓은 값고 현재 값을 비교하여 저널에 저장함. 
-------------------------------------------------------------------*/
void CDevCmn::SaveATMInfoParameter(int nUser, LPAPPATMINFO pPreviousATMInfo, LPAPPATMINFO pCurrentATMInfo)
{
NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() ** APPATMINFO** Start\n")));

	CString strOldValue, strNewValue;
	CStringArray strOldValueArray, strNewValueArray;

	//Language Mode 
	if ( pPreviousATMInfo->wENGMode != pCurrentATMInfo->wENGMode )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousATMInfo->wENGMode);
		strNewValue = GET_ENDIS_STRING(pCurrentATMInfo->wENGMode);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"LANGUAGES (ENGLISH)", strOldValue, strNewValue); 
	}

	if ( pPreviousATMInfo->wSPNMode != pCurrentATMInfo->wSPNMode )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousATMInfo->wSPNMode);
		strNewValue = GET_ENDIS_STRING(pCurrentATMInfo->wSPNMode);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"LANGUAGES (SPANISH)", strOldValue, strNewValue); 
	}

	if ( pPreviousATMInfo->wFRNMode  != pCurrentATMInfo->wFRNMode )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousATMInfo->wFRNMode);
		strNewValue = GET_ENDIS_STRING(pCurrentATMInfo->wFRNMode);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"LANGUAGES (FRENCH)", strOldValue, strNewValue); 
	}

	// [#2186] US KMK 2013.04.16 Asian Language Support
	if ( pPreviousATMInfo->wCHNMode  != pCurrentATMInfo->wCHNMode )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousATMInfo->wCHNMode);
		strNewValue = GET_ENDIS_STRING(pCurrentATMInfo->wCHNMode);

		m_JNLMgr.SaveChangeParameterJnl(nUser, L"LANGUAGES (CHINESE)", strOldValue, strNewValue); 
	}
	if ( pPreviousATMInfo->wKORMode  != pCurrentATMInfo->wKORMode )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousATMInfo->wKORMode);
		strNewValue = GET_ENDIS_STRING(pCurrentATMInfo->wKORMode);

		m_JNLMgr.SaveChangeParameterJnl(nUser, L"LANGUAGES (KOREAN)", strOldValue, strNewValue); 
	}
	if ( pPreviousATMInfo->wJAPMode  != pCurrentATMInfo->wJAPMode )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousATMInfo->wJAPMode);
		strNewValue = GET_ENDIS_STRING(pCurrentATMInfo->wJAPMode);

		m_JNLMgr.SaveChangeParameterJnl(nUser, L"LANGUAGES (JAPANESE)", strOldValue, strNewValue); 
	}
	// end of [#2186]

	//ISO Flag
	if ( pPreviousATMInfo->wISO1Flag != pCurrentATMInfo->wISO1Flag )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousATMInfo->wISO1Flag);
		strNewValue = GET_ENDIS_STRING(pCurrentATMInfo->wISO1Flag);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"CARD ISO 1", strOldValue, strNewValue); 
	}
	if ( pPreviousATMInfo->wISO2Flag != pCurrentATMInfo->wISO2Flag )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousATMInfo->wISO2Flag);
		strNewValue = GET_ENDIS_STRING(pCurrentATMInfo->wISO2Flag);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"CARD ISO 2", strOldValue, strNewValue); 
	}
	if ( pPreviousATMInfo->wISO3Flag != pCurrentATMInfo->wISO3Flag )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousATMInfo->wISO3Flag);
		strNewValue = GET_ENDIS_STRING(pCurrentATMInfo->wISO3Flag);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"CARD ISO 3", strOldValue, strNewValue); 
	}
	//Low Currency Check 
	if ( pPreviousATMInfo->wLowCurrencyCheck != pCurrentATMInfo->wLowCurrencyCheck )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousATMInfo->wLowCurrencyCheck);
		strNewValue = GET_ENDIS_STRING(pCurrentATMInfo->wLowCurrencyCheck);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"LOW CURRENCY CHECK", strOldValue, strNewValue); 
	}

	//Sucharge Enable 
	if ( pPreviousATMInfo->wSurchargeEnable  != pCurrentATMInfo->wSurchargeEnable )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousATMInfo->wSurchargeEnable);
		strNewValue = GET_ENDIS_STRING(pCurrentATMInfo->wSurchargeEnable);
	
#if ( AU_VERSION)
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"ATM OPERATOR FEE", strOldValue, strNewValue); 
#else
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SURCHARGE", strOldValue, strNewValue); 
#endif
	}

	//Select Receipt 
	if ( pPreviousATMInfo->wSelectReceipt  != pCurrentATMInfo->wSelectReceipt )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousATMInfo->wSelectReceipt);
		strNewValue = GET_ENDIS_STRING(pCurrentATMInfo->wSelectReceipt);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SELECT RECEIPT", strOldValue, strNewValue); 
	}

	//Surcharge Display는 OP/RMS로 변경 불가 

	//Serial NUmber
	if ( memcmp(pPreviousATMInfo->chATMSerialNo, pCurrentATMInfo->chATMSerialNo, sizeof( pCurrentATMInfo->chATMSerialNo)))
	{
		strOldValue = CString(pPreviousATMInfo->chATMSerialNo);
		strNewValue = CString(pCurrentATMInfo->chATMSerialNo);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SERIAL NUMBER", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousATMInfo->chMessageType, pCurrentATMInfo->chMessageType, sizeof( pCurrentATMInfo->chMessageType)))
	{
		CString strOldFormat = CString(pPreviousATMInfo->chMessageType);
		if ( strOldFormat == MSG_HYOSUNG_TYPE ) strOldValue = L"STANDARD1";
		else if (strOldFormat == MSG_CSP200_TYPE ) strOldValue = L"STANDARD2";
		else if (strOldFormat == MSG_TRITON_TYPE ) strOldValue = L"STANDARD3";
		else strOldValue = L"EPS";
		CString strNewFormat = CString(pCurrentATMInfo->chMessageType);
		if ( strNewFormat == MSG_HYOSUNG_TYPE ) strNewValue = L"STANDARD1";
		else if (strNewFormat == MSG_CSP200_TYPE ) strNewValue = L"STANDARD2";
		else if (strNewFormat == MSG_TRITON_TYPE ) strNewValue = L"STANDARD3";
		else strNewValue = L"EPS";
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"MESSAGE FORMAT", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousATMInfo->chDispenseLimit, pCurrentATMInfo->chDispenseLimit, sizeof( pCurrentATMInfo->chDispenseLimit)))
	{
		strOldValue = CString(pPreviousATMInfo->chDispenseLimit);
		strNewValue = CString(pCurrentATMInfo->chDispenseLimit);

		strOldValue = MakeMoneyCent(strOldValue);//strOldValue.Left( strOldValue.GetLength() -2);  //cent 까지 저장함. 
		strNewValue = MakeMoneyCent(strNewValue);//strNewValue.Left( strNewValue.GetLength() -2);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"DISPENSE LIMIT", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousATMInfo->chRoutingID, pCurrentATMInfo->chRoutingID, sizeof( pCurrentATMInfo->chRoutingID)))
	{
		strOldValue = CString(pPreviousATMInfo->chRoutingID);
		strNewValue = CString(pCurrentATMInfo->chRoutingID);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"ROUTING ID", strOldValue, strNewValue); 
	}
	if ( memcmp(pPreviousATMInfo->chTerminalID, pCurrentATMInfo->chTerminalID, sizeof( pCurrentATMInfo->chTerminalID)))
	{
		strOldValue = CString(pPreviousATMInfo->chTerminalID);
		strNewValue = CString(pCurrentATMInfo->chTerminalID);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"TERMINAL ID", strOldValue, strNewValue); 
	}
	//Fast Cash
	if ( (pPreviousATMInfo->dwFastCash[0] != pCurrentATMInfo->dwFastCash[0] )
		|| (pPreviousATMInfo->dwFastCash[1] != pCurrentATMInfo->dwFastCash[1] )
		|| (pPreviousATMInfo->dwFastCash[2] != pCurrentATMInfo->dwFastCash[2] )
		|| (pPreviousATMInfo->dwFastCash[3] != pCurrentATMInfo->dwFastCash[3] )
		|| (pPreviousATMInfo->dwFastCash[4] != pCurrentATMInfo->dwFastCash[4] )
		|| (pPreviousATMInfo->dwFastCash[5] != pCurrentATMInfo->dwFastCash[5] ) )
	{
		strOldValue.Format(L"%03d, %03d, %03d, %03d, %03d, %03d", 
			pPreviousATMInfo->dwFastCash[0], pPreviousATMInfo->dwFastCash[1], pPreviousATMInfo->dwFastCash[2],
			pPreviousATMInfo->dwFastCash[3], pPreviousATMInfo->dwFastCash[4], pPreviousATMInfo->dwFastCash[5]);

		strNewValue.Format(L"%03d, %03d, %03d, %03d, %03d, %03d", 
			pCurrentATMInfo->dwFastCash[0], pCurrentATMInfo->dwFastCash[1], pCurrentATMInfo->dwFastCash[2],
			pCurrentATMInfo->dwFastCash[3], pCurrentATMInfo->dwFastCash[4], pCurrentATMInfo->dwFastCash[5]);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"FAST CASH", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousATMInfo->chSurchargeAmount, pCurrentATMInfo->chSurchargeAmount, sizeof( pCurrentATMInfo->chSurchargeAmount)))
	{
		strOldValue = CString(pPreviousATMInfo->chSurchargeAmount);
		strNewValue = CString(pCurrentATMInfo->chSurchargeAmount);

		strOldValue = MakeMoneyCent(strOldValue);//strOldValue.Left( strOldValue.GetLength() -2);  //cent 까지 저장함. 
		strNewValue = MakeMoneyCent(strNewValue);//strNewValue.Left( strNewValue.GetLength() -2);
		
		
#if ( AU_VERSION)
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"WITHDRAWAL ATM FEE AMOUNT", strOldValue, strNewValue); 
#else
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SURCHARGE AMOUNT", strOldValue, strNewValue); 
#endif
	}

	if ( memcmp(pPreviousATMInfo->chSurchargeOwner, pCurrentATMInfo->chSurchargeOwner, sizeof( pCurrentATMInfo->chSurchargeOwner)))
	{
		strOldValue = CString(pPreviousATMInfo->chSurchargeOwner);
		strNewValue = CString(pCurrentATMInfo->chSurchargeOwner);
		
#if ( AU_VERSION)
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"ATM OPERATOR FEE OWNER", strOldValue, strNewValue); 
#else
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SURCHARGE OWNER", strOldValue, strNewValue); 
#endif
	}

	if ( memcmp(pPreviousATMInfo->chReceiptHeader1, pCurrentATMInfo->chReceiptHeader1, sizeof( pCurrentATMInfo->chReceiptHeader1))
		|| memcmp(pPreviousATMInfo->chReceiptHeader2, pCurrentATMInfo->chReceiptHeader2, sizeof( pCurrentATMInfo->chReceiptHeader2)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousATMInfo->chReceiptHeader1,sizeof( pPreviousATMInfo->chReceiptHeader1) ));
		strOldValueArray.Add(CString(pPreviousATMInfo->chReceiptHeader2,sizeof( pPreviousATMInfo->chReceiptHeader2)));
		strNewValueArray.Add(CString(pCurrentATMInfo->chReceiptHeader1,sizeof( pCurrentATMInfo->chReceiptHeader1)));
		strNewValueArray.Add(CString(pCurrentATMInfo->chReceiptHeader2,sizeof( pCurrentATMInfo->chReceiptHeader2)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RECEIPT HEADER 1,2", strOldValueArray, strNewValueArray); 
	}

	if ( memcmp(pPreviousATMInfo->chReceiptHeader3, pCurrentATMInfo->chReceiptHeader3, sizeof( pCurrentATMInfo->chReceiptHeader3)))
	{
		strOldValue = CString(pPreviousATMInfo->chReceiptHeader3);
		strNewValue = CString(pCurrentATMInfo->chReceiptHeader3);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RECEIPT TAIL", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousATMInfo->chReceiptAddress1, pCurrentATMInfo->chReceiptAddress1, sizeof( pCurrentATMInfo->chReceiptAddress1))
		|| memcmp(pPreviousATMInfo->chReceiptAddress2, pCurrentATMInfo->chReceiptAddress2, sizeof( pCurrentATMInfo->chReceiptAddress2))
		|| memcmp(pPreviousATMInfo->chReceiptAddress3, pCurrentATMInfo->chReceiptAddress3, sizeof( pCurrentATMInfo->chReceiptAddress3))
		|| memcmp(pPreviousATMInfo->chReceiptPhoneNo, pCurrentATMInfo->chReceiptPhoneNo, sizeof( pCurrentATMInfo->chReceiptPhoneNo)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousATMInfo->chReceiptAddress1, sizeof( pPreviousATMInfo->chReceiptAddress1) ));
		strOldValueArray.Add(CString(pPreviousATMInfo->chReceiptAddress2, sizeof( pPreviousATMInfo->chReceiptAddress2)));
		strOldValueArray.Add(CString(pPreviousATMInfo->chReceiptAddress3, sizeof( pPreviousATMInfo->chReceiptAddress3)));
		strOldValueArray.Add(CString(pPreviousATMInfo->chReceiptPhoneNo, sizeof( pPreviousATMInfo->chReceiptPhoneNo)));
		strNewValueArray.Add(CString(pCurrentATMInfo->chReceiptAddress1, sizeof( pCurrentATMInfo->chReceiptAddress1)));
		strNewValueArray.Add(CString(pCurrentATMInfo->chReceiptAddress2, sizeof( pCurrentATMInfo->chReceiptAddress2)));
		strNewValueArray.Add(CString(pCurrentATMInfo->chReceiptAddress3,sizeof( pCurrentATMInfo->chReceiptAddress3)));
		strNewValueArray.Add(CString(pCurrentATMInfo->chReceiptPhoneNo, sizeof( pCurrentATMInfo->chReceiptPhoneNo)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RECEIPT ADDRESS & PHONE NO", strOldValueArray, strNewValueArray); 
	}
	//[#2000] SOOK 2010.10.12 호주 사양 적용
	if ( memcmp(pPreviousATMInfo->chATMSerialNo, pCurrentATMInfo->chATMSerialNo, sizeof( pCurrentATMInfo->chATMSerialNo)))
	{
		strOldValue = CString(pPreviousATMInfo->chATMSerialNo);
		strNewValue = CString(pCurrentATMInfo->chATMSerialNo);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SERIAL NUMBER", strOldValue, strNewValue); 
	}//end of [#2000]
	if ( strOldValueArray.GetSize() > 0) strOldValueArray.RemoveAll();
	if ( strNewValueArray.GetSize() > 0) strNewValueArray.RemoveAll();

NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() ** APPATMINFO** End\n")));

}
/*-------------------------------------------------------------------
 FUNCTION NAME: SaveAdvertiseInfoParameter()
 RETURN TYPE  : 
 PARAMETER    : ADVINFO 
 DESCRIPTION  : 변경 완료 후 미리 저장해 놓은 값고 현재 값을 비교하여 저널에 저장함. 
-------------------------------------------------------------------*/
void	CDevCmn::SaveAdvertiseInfoParameter(int nUser, LPADVINFO pPreviousAdvInfo, LPADVINFO pCurrentAdvInfo)
{
NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() ** ADVINFO** Start\n")));
	
	CString strParameter;
	CString strOldValue, strNewValue;
	CStringArray strOldValueArray, strNewValueArray;

	for ( int i =0; i < 6; i++)
	{
		if ( pPreviousAdvInfo->Adv_EndisFlag[i] != pCurrentAdvInfo->Adv_EndisFlag[i] )
		{
			strParameter.Format(L"ADVERTISEMENT %d", i+1);

			strOldValue = GET_ENDIS_STRING(pPreviousAdvInfo->Adv_EndisFlag[i]);
			strNewValue = GET_ENDIS_STRING(pCurrentAdvInfo->Adv_EndisFlag[i]);
			
			m_JNLMgr.SaveChangeParameterJnl(nUser, strParameter, strOldValue, strNewValue); 
		}
	}

	for (int i =0; i < 6; i++)
	{
		if ( pPreviousAdvInfo->Coupon_EndisFlag[i] != pCurrentAdvInfo->Coupon_EndisFlag[i] )
		{
			strParameter.Format(L"COUPON %d", i+1);

			strOldValue = GET_ENDIS_STRING(pPreviousAdvInfo->Coupon_EndisFlag[i]);
			strNewValue = GET_ENDIS_STRING(pCurrentAdvInfo->Coupon_EndisFlag[i]);
			
			m_JNLMgr.SaveChangeParameterJnl(nUser, strParameter, strOldValue, strNewValue); 
		}
	}
	if ( pPreviousAdvInfo->Adv_GuideDisp_Time != pCurrentAdvInfo->Adv_GuideDisp_Time )
	{

		strOldValue.Format(L"%d SEC", pPreviousAdvInfo->Adv_GuideDisp_Time );
		strNewValue.Format(L"%d SEC", pCurrentAdvInfo->Adv_GuideDisp_Time );
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"ADVERTISEMENT TIMER", strOldValue, strNewValue); 
	}
	//광고 & 쿠폰 추가 

	//1
	if ( memcmp(pPreviousAdvInfo->Adv_Title1 , pCurrentAdvInfo->Adv_Title1, sizeof( pCurrentAdvInfo->Adv_Title1)))
	{
		strOldValue = CString(pPreviousAdvInfo->Adv_Title1);
		strNewValue = CString(pCurrentAdvInfo->Adv_Title1);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"ADVERTISEMENT 1 TITLE", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousAdvInfo->Coupon1_Text1 , pCurrentAdvInfo->Coupon1_Text1, sizeof( pCurrentAdvInfo->Coupon1_Text1))
		|| memcmp(pPreviousAdvInfo->Coupon1_Text2 , pCurrentAdvInfo->Coupon1_Text2, sizeof( pCurrentAdvInfo->Coupon1_Text2)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousAdvInfo->Coupon1_Text1, sizeof( pPreviousAdvInfo->Coupon1_Text1)));
		strOldValueArray.Add(CString(pPreviousAdvInfo->Coupon1_Text2, sizeof( pPreviousAdvInfo->Coupon1_Text2)));

		strNewValueArray.Add(CString(pCurrentAdvInfo->Coupon1_Text1, sizeof( pCurrentAdvInfo->Coupon1_Text1)));
		strNewValueArray.Add(CString(pCurrentAdvInfo->Coupon1_Text2, sizeof( pCurrentAdvInfo->Coupon1_Text2)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"COUPON 1 TEXT", strOldValueArray, strNewValueArray); 
	}

	//2
	if ( memcmp(pPreviousAdvInfo->Adv_Title2 , pCurrentAdvInfo->Adv_Title2, sizeof( pCurrentAdvInfo->Adv_Title2)))
	{
		strOldValue = CString(pPreviousAdvInfo->Adv_Title2);
		strNewValue = CString(pCurrentAdvInfo->Adv_Title2);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"ADVERTISEMENT 2 TITLE", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousAdvInfo->Coupon2_Text1 , pCurrentAdvInfo->Coupon2_Text1, sizeof( pCurrentAdvInfo->Coupon2_Text1))
		|| memcmp(pPreviousAdvInfo->Coupon2_Text2 , pCurrentAdvInfo->Coupon2_Text2, sizeof( pCurrentAdvInfo->Coupon2_Text2)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousAdvInfo->Coupon2_Text1, sizeof( pPreviousAdvInfo->Coupon2_Text1)));
		strOldValueArray.Add(CString(pPreviousAdvInfo->Coupon2_Text2, sizeof( pPreviousAdvInfo->Coupon2_Text2)));

		strNewValueArray.Add(CString(pCurrentAdvInfo->Coupon2_Text1, sizeof( pCurrentAdvInfo->Coupon2_Text1)));
		strNewValueArray.Add(CString(pCurrentAdvInfo->Coupon2_Text2,  sizeof(pCurrentAdvInfo->Coupon2_Text2)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"COUPON 2 TEXT", strOldValueArray, strNewValueArray); 
	}
	//3
	if ( memcmp(pPreviousAdvInfo->Adv_Title3 , pCurrentAdvInfo->Adv_Title3, sizeof( pCurrentAdvInfo->Adv_Title3)))
	{
		strOldValue = CString(pPreviousAdvInfo->Adv_Title3);
		strNewValue = CString(pCurrentAdvInfo->Adv_Title3);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"ADVERTISEMENT 3 TITLE", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousAdvInfo->Coupon3_Text1 , pCurrentAdvInfo->Coupon3_Text1, sizeof( pCurrentAdvInfo->Coupon3_Text1))
		|| memcmp(pPreviousAdvInfo->Coupon3_Text2 , pCurrentAdvInfo->Coupon3_Text2, sizeof( pCurrentAdvInfo->Coupon3_Text2)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousAdvInfo->Coupon3_Text1, sizeof( pPreviousAdvInfo->Coupon3_Text1)));
		strOldValueArray.Add(CString(pPreviousAdvInfo->Coupon3_Text2, sizeof( pPreviousAdvInfo->Coupon3_Text2)));

		strNewValueArray.Add(CString(pCurrentAdvInfo->Coupon3_Text1, sizeof( pCurrentAdvInfo->Coupon3_Text1)));
		strNewValueArray.Add(CString(pCurrentAdvInfo->Coupon3_Text2, sizeof( pCurrentAdvInfo->Coupon3_Text2)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"COUPON 3 TEXT", strOldValueArray, strNewValueArray); 
	}
	//4
	if ( memcmp(pPreviousAdvInfo->Adv_Title4 , pCurrentAdvInfo->Adv_Title4, sizeof( pCurrentAdvInfo->Adv_Title4)))
	{
		strOldValue = CString(pPreviousAdvInfo->Adv_Title4);
		strNewValue = CString(pCurrentAdvInfo->Adv_Title4);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"ADVERTISEMENT 4 TITLE", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousAdvInfo->Coupon4_Text1 , pCurrentAdvInfo->Coupon4_Text1, sizeof( pCurrentAdvInfo->Coupon4_Text1))
		|| memcmp(pPreviousAdvInfo->Coupon4_Text2 , pCurrentAdvInfo->Coupon4_Text2, sizeof( pCurrentAdvInfo->Coupon4_Text2)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousAdvInfo->Coupon4_Text1, sizeof( pPreviousAdvInfo->Coupon4_Text1)));
		strOldValueArray.Add(CString(pPreviousAdvInfo->Coupon4_Text2, sizeof( pPreviousAdvInfo->Coupon4_Text2)));

		strNewValueArray.Add(CString(pCurrentAdvInfo->Coupon4_Text1, sizeof( pCurrentAdvInfo->Coupon4_Text1)));
		strNewValueArray.Add(CString(pCurrentAdvInfo->Coupon4_Text2, sizeof( pCurrentAdvInfo->Coupon4_Text2)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"COUPON 4 TEXT", strOldValueArray, strNewValueArray); 
	}

	//5
	if ( memcmp(pPreviousAdvInfo->Adv_Title5 , pCurrentAdvInfo->Adv_Title5, sizeof( pCurrentAdvInfo->Adv_Title5)))
	{
		strOldValue = CString(pPreviousAdvInfo->Adv_Title5);
		strNewValue = CString(pCurrentAdvInfo->Adv_Title5);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"ADVERTISEMENT 5 TITLE", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousAdvInfo->Coupon5_Text1 , pCurrentAdvInfo->Coupon5_Text1, sizeof( pCurrentAdvInfo->Coupon5_Text1))
		|| memcmp(pPreviousAdvInfo->Coupon5_Text2 , pCurrentAdvInfo->Coupon5_Text2, sizeof( pCurrentAdvInfo->Coupon5_Text2)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousAdvInfo->Coupon5_Text1, sizeof( pCurrentAdvInfo->Coupon5_Text1)));
		strOldValueArray.Add(CString(pPreviousAdvInfo->Coupon5_Text2, sizeof( pCurrentAdvInfo->Coupon5_Text2)));

		strNewValueArray.Add(CString(pCurrentAdvInfo->Coupon5_Text1, sizeof( pCurrentAdvInfo->Coupon5_Text1)));
		strNewValueArray.Add(CString(pCurrentAdvInfo->Coupon5_Text2, sizeof( pCurrentAdvInfo->Coupon5_Text2)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"COUPON 5 TEXT", strOldValueArray, strNewValueArray); 
	}

	//6
	if ( memcmp(pPreviousAdvInfo->Adv_Title6 , pCurrentAdvInfo->Adv_Title6, sizeof( pCurrentAdvInfo->Adv_Title6)))
	{
		strOldValue = CString(pPreviousAdvInfo->Adv_Title6);
		strNewValue = CString(pCurrentAdvInfo->Adv_Title6);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"ADVERTISEMENT 6 TITLE", strOldValue, strNewValue); 
	}

	if ( memcmp(pPreviousAdvInfo->Coupon6_Text1 , pCurrentAdvInfo->Coupon6_Text1, sizeof( pCurrentAdvInfo->Coupon6_Text1))
		|| memcmp(pPreviousAdvInfo->Coupon6_Text2 , pCurrentAdvInfo->Coupon6_Text2, sizeof( pCurrentAdvInfo->Coupon6_Text2)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousAdvInfo->Coupon6_Text1, sizeof( pCurrentAdvInfo->Coupon6_Text1)));
		strOldValueArray.Add(CString(pPreviousAdvInfo->Coupon6_Text2, sizeof( pCurrentAdvInfo->Coupon6_Text2)));

		strNewValueArray.Add(CString(pCurrentAdvInfo->Coupon6_Text1, sizeof( pCurrentAdvInfo->Coupon6_Text1)));
		strNewValueArray.Add(CString(pCurrentAdvInfo->Coupon6_Text2, sizeof( pCurrentAdvInfo->Coupon6_Text2)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"COUPON 6 TEXT", strOldValueArray, strNewValueArray); 
	}

	if ( pPreviousAdvInfo->TranAdv_Disp_Time != pCurrentAdvInfo->TranAdv_Disp_Time )
	{

		strOldValue.Format(L"%d SEC", pPreviousAdvInfo->TranAdv_Disp_Time );
		strNewValue.Format(L"%d SEC", pCurrentAdvInfo->TranAdv_Disp_Time );
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"TRANSACTION AD TIMER", strOldValue, strNewValue); 
	}

	for (int i =0; i < 6; i++)
	{
		if ( pPreviousAdvInfo->TranAdv_EndisFlag[i] != pCurrentAdvInfo->TranAdv_EndisFlag[i] )
		{
			strParameter.Format(L"TRANSACTION AD %d", i+1);

			strOldValue = GET_ENDIS_STRING(pPreviousAdvInfo->TranAdv_EndisFlag[i]);
			strNewValue = GET_ENDIS_STRING(pCurrentAdvInfo->TranAdv_EndisFlag[i]);
			
			m_JNLMgr.SaveChangeParameterJnl(nUser, strParameter, strOldValue, strNewValue); 
		}
	}

	if ( pPreviousAdvInfo->ChangeBackFlag != pCurrentAdvInfo->ChangeBackFlag )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousAdvInfo->ChangeBackFlag);
		strNewValue = GET_ENDIS_STRING(pCurrentAdvInfo->ChangeBackFlag);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"CHANGING BACKGROUND", strOldValue, strNewValue); 
	}	

	if ( pPreviousAdvInfo->ChangeDefaultBack != pCurrentAdvInfo->ChangeDefaultBack )
	{

		strOldValue.Format(L"%d", pPreviousAdvInfo->ChangeDefaultBack );
		strNewValue.Format(L"%d", pCurrentAdvInfo->ChangeDefaultBack );
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"DEFAULT BACKGROUND", strOldValue, strNewValue); 
	}

	for (int i =0; i < 6; i++)
	{
		if ( pPreviousAdvInfo->ChangeBackTypeFlag[i] != pCurrentAdvInfo->ChangeBackTypeFlag[i] )
		{
			strParameter.Format(L"BACKGROUND SCREEN %d", i+1);

			strOldValue = GET_ENDIS_STRING(pPreviousAdvInfo->ChangeBackTypeFlag[i]);
			strNewValue = GET_ENDIS_STRING(pCurrentAdvInfo->ChangeBackTypeFlag[i]);
			
			m_JNLMgr.SaveChangeParameterJnl(nUser, strParameter, strOldValue, strNewValue); 
		}
	}

	if ( strOldValueArray.GetSize() > 0) strOldValueArray.RemoveAll();
	if ( strNewValueArray.GetSize() > 0) strNewValueArray.RemoveAll();

NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() ** ADVINFO** End\n")));

}
/*-------------------------------------------------------------------
 FUNCTION NAME: SaveTCPIPInfoParameter()
 RETURN TYPE  : 
 PARAMETER    : TCPIPINFO 
 DESCRIPTION  : 변경 완료 후 미리 저장해 놓은 값고 현재 값을 비교하여 저널에 저장함. 
-------------------------------------------------------------------*/
void CDevCmn::SaveTCPIPInfoParameter(int nUser, LPTCPIPINFO pPreviousTCPIPInfo, LPTCPIPINFO pCurrentTCPIPInfo)
{
NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() ** TCPIPINFO** Start\n")));

	CString strParameter;
	CString strOldValue, strNewValue;
	CStringArray strOldValueArray, strNewValueArray;

	if ( memcmp(pPreviousTCPIPInfo->ATM_IpAddr , pCurrentTCPIPInfo->ATM_IpAddr, sizeof( pCurrentTCPIPInfo->ATM_IpAddr)))
	{
		strOldValue = CString(pPreviousTCPIPInfo->ATM_IpAddr);
		strNewValue = CString(pCurrentTCPIPInfo->ATM_IpAddr);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"TERMINAL IP ADDRESS", strOldValue, strNewValue); 
	}
	if ( memcmp(pPreviousTCPIPInfo->ATM_Subnet , pCurrentTCPIPInfo->ATM_Subnet, sizeof( pCurrentTCPIPInfo->ATM_Subnet)))
	{
		strOldValue = CString(pPreviousTCPIPInfo->ATM_Subnet);
		strNewValue = CString(pCurrentTCPIPInfo->ATM_Subnet);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SUBNET MASK", strOldValue, strNewValue); 
	}
	if ( memcmp(pPreviousTCPIPInfo->ATM_Gateway , pCurrentTCPIPInfo->ATM_Gateway, sizeof( pCurrentTCPIPInfo->ATM_Gateway)))
	{
		strOldValue = CString(pPreviousTCPIPInfo->ATM_Gateway);
		strNewValue = CString(pCurrentTCPIPInfo->ATM_Gateway);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"GATEWAY", strOldValue, strNewValue); 
	}
	if ( memcmp(pPreviousTCPIPInfo->ATM_DNS , pCurrentTCPIPInfo->ATM_DNS, sizeof( pCurrentTCPIPInfo->ATM_DNS)))
	{
		strOldValue = CString(pPreviousTCPIPInfo->ATM_DNS);
		strNewValue = CString(pCurrentTCPIPInfo->ATM_DNS);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"DNS", strOldValue, strNewValue); 
	}

	if ( pPreviousTCPIPInfo->ATM_DHCP != pCurrentTCPIPInfo->ATM_DHCP )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousTCPIPInfo->ATM_DHCP);
		strNewValue = GET_ENDIS_STRING(pCurrentTCPIPInfo->ATM_DHCP);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"DHCP", strOldValue, strNewValue); 
	}	

	if ( pPreviousTCPIPInfo->IsHostUseURL != pCurrentTCPIPInfo->IsHostUseURL )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousTCPIPInfo->IsHostUseURL);
		strNewValue = GET_ENDIS_STRING(pCurrentTCPIPInfo->IsHostUseURL);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"HOST URL", strOldValue, strNewValue); 
	}	
	if ( memcmp(pPreviousTCPIPInfo->Host1Name , pCurrentTCPIPInfo->Host1Name, sizeof( pCurrentTCPIPInfo->Host1Name)))
	{
		strOldValue = CString(pPreviousTCPIPInfo->Host1Name);
		strNewValue = CString(pCurrentTCPIPInfo->Host1Name);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"HOST ADDRESS 1", strOldValue, strNewValue); 
	}

	if ( pPreviousTCPIPInfo->Host1PortNo != pCurrentTCPIPInfo->Host1PortNo )
	{
		strOldValue.Format(L"%d", pPreviousTCPIPInfo->Host1PortNo);
		strNewValue.Format(L"%d",pCurrentTCPIPInfo->Host1PortNo);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"HOST 1 PORT NO", strOldValue, strNewValue); 
	}
	
	if ( memcmp(pPreviousTCPIPInfo->Host2Name , pCurrentTCPIPInfo->Host2Name, sizeof( pCurrentTCPIPInfo->Host2Name)))
	{
		strOldValue = CString(pPreviousTCPIPInfo->Host2Name);
		strNewValue = CString(pCurrentTCPIPInfo->Host2Name);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"HOST ADDRESS 2", strOldValue, strNewValue); 
	}

	if ( pPreviousTCPIPInfo->Host2PortNo != pCurrentTCPIPInfo->Host2PortNo )
	{
		strOldValue.Format(L"%d", pPreviousTCPIPInfo->Host2PortNo);
		strNewValue.Format(L"%d",pCurrentTCPIPInfo->Host2PortNo);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"HOST 2 PORT NO", strOldValue, strNewValue); 
	}	

	//GET_DIALTCP_STRING
	if ( pPreviousTCPIPInfo->NetworkType  != pCurrentTCPIPInfo->NetworkType )
	{
		strOldValue = GET_DIALTCP_STRING(pPreviousTCPIPInfo->NetworkType);
		strNewValue = GET_DIALTCP_STRING(pCurrentTCPIPInfo->NetworkType);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"COMMUNICATION", strOldValue, strNewValue); 
	}
	
	if ( pPreviousTCPIPInfo->SSL_Enable   != pCurrentTCPIPInfo->SSL_Enable )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousTCPIPInfo->SSL_Enable);
		strNewValue = GET_ENDIS_STRING(pCurrentTCPIPInfo->SSL_Enable);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SSL", strOldValue, strNewValue); 
	}	
	if ( pPreviousTCPIPInfo->CRC_Enable   != pCurrentTCPIPInfo->CRC_Enable )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousTCPIPInfo->CRC_Enable);
		strNewValue = GET_ENDIS_STRING(pCurrentTCPIPInfo->CRC_Enable);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"CRC", strOldValue, strNewValue); 
	}	

		// [1-VISA_FRAMED], [2-STANDARD], [3-ACK_CONTROLED]
	if ( pPreviousTCPIPInfo->TCPIP_Type  != pCurrentTCPIPInfo->TCPIP_Type )
	{
		strOldValue = GET_TCPIP_TYPE_STRING(pPreviousTCPIPInfo->TCPIP_Type);
		strNewValue = GET_TCPIP_TYPE_STRING(pCurrentTCPIPInfo->TCPIP_Type);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"TCP/IP TYPE", strOldValue, strNewValue); 
	}

	if ( pPreviousTCPIPInfo->IsRMSUseURL != pCurrentTCPIPInfo->IsRMSUseURL )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousTCPIPInfo->IsRMSUseURL);
		strNewValue = GET_ENDIS_STRING(pCurrentTCPIPInfo->IsRMSUseURL);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RMS URL", strOldValue, strNewValue); 
	}	
	if ( memcmp(pPreviousTCPIPInfo->RMSName , pCurrentTCPIPInfo->RMSName, sizeof( pCurrentTCPIPInfo->RMSName)))
	{
		strOldValue = CString(pPreviousTCPIPInfo->RMSName);
		strNewValue = CString(pCurrentTCPIPInfo->RMSName);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RMS ADDRESS", strOldValue, strNewValue); 
	}

	if ( pPreviousTCPIPInfo->RMSPortNo != pCurrentTCPIPInfo->RMSPortNo )
	{
		strOldValue.Format(L"%d", pPreviousTCPIPInfo->RMSPortNo);
		strNewValue.Format(L"%d",pCurrentTCPIPInfo->RMSPortNo);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"ATM LISTENING PORT NO", strOldValue, strNewValue); 
	}

	if ( pPreviousTCPIPInfo->RMSSvrPortNo != pCurrentTCPIPInfo->RMSSvrPortNo )
	{
		strOldValue.Format(L"%d", pPreviousTCPIPInfo->RMSSvrPortNo);
		strNewValue.Format(L"%d",pCurrentTCPIPInfo->RMSSvrPortNo);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RMS LISTENING PORT NO", strOldValue, strNewValue); 
	}

	if ( pPreviousTCPIPInfo->RMSSendInterval  != pCurrentTCPIPInfo->RMSSendInterval )
	{

		strOldValue.Format(L"%d HOURS", pPreviousTCPIPInfo->RMSSendInterval );
		strNewValue.Format(L"%d HOURS", pCurrentTCPIPInfo->RMSSendInterval );
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RMS SEND INTERVAL", strOldValue, strNewValue); 
	}
	if ( strOldValueArray.GetSize() > 0) strOldValueArray.RemoveAll();
	if ( strNewValueArray.GetSize() > 0) strNewValueArray.RemoveAll();

NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() ** TCPIPINFO** End\n")));

}
/*-------------------------------------------------------------------
 FUNCTION NAME: SaveOptionalSettingParameter()
 RETURN TYPE  : 
 PARAMETER    : OPTINALSETTING 
 DESCRIPTION  : 변경 완료 후 미리 저장해 놓은 값고 현재 값을 비교하여 저널에 저장함. 
-------------------------------------------------------------------*/
void CDevCmn::SaveOptionalSettingParameter(int nUser, LPOPTIONALSETTING pPreviousOptionalSetting , LPOPTIONALSETTING pCurrentOptionalSetting )
{
NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() ** OPTINALSETTING** Start\n")));

	CString strParameter;
	CString strOldValue, strNewValue;
	CStringArray strOldValueArray, strNewValueArray;


	if ( pPreviousOptionalSetting->dwReversalRetryCount  != pCurrentOptionalSetting->dwReversalRetryCount )
	{
		strOldValue.Format(L"%d", pPreviousOptionalSetting->dwReversalRetryCount);
		strNewValue.Format(L"%d",pCurrentOptionalSetting->dwReversalRetryCount);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"REVERSAL RETRY COUNT", strOldValue, strNewValue); 
	}
	if ( pPreviousOptionalSetting->MOD10_Enable   != pCurrentOptionalSetting->MOD10_Enable )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting->MOD10_Enable);
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting->MOD10_Enable);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"MOD 10", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->dwAccountChecking    != pCurrentOptionalSetting->dwAccountChecking )
	{
#if (CA_VERSION)	// [#2069] NH KSK 2011.06.13
		strParameter = L"CHEQUING ACCOUNT";
#elif ( AU_VERSION)
		strParameter = L"CHEQUE ACCOUNT";
#else
		strParameter = L"CHECKING ACCOUNT";
#endif
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting->dwAccountChecking);
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting->dwAccountChecking);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, strParameter, strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->dwAccountSavings    != pCurrentOptionalSetting->dwAccountSavings )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting->dwAccountSavings);
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting->dwAccountSavings);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SAVINGS ACCOUNT", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->dwAccountCreditCard     != pCurrentOptionalSetting->dwAccountCreditCard )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting->dwAccountCreditCard);
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting->dwAccountCreditCard);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"CREDIT CARD ACCOUNT", strOldValue, strNewValue); 
	}	
	//Network User Address 생략 
	if ( memcmp(pPreviousOptionalSetting->chWelcomeMessage1 , pCurrentOptionalSetting->chWelcomeMessage1, sizeof( pCurrentOptionalSetting->chWelcomeMessage1))
		|| memcmp(pPreviousOptionalSetting->chWelcomeMessage2 , pCurrentOptionalSetting->chWelcomeMessage2, sizeof( pCurrentOptionalSetting->chWelcomeMessage2))
		|| memcmp(pPreviousOptionalSetting->chWelcomeMessage3 , pCurrentOptionalSetting->chWelcomeMessage3, sizeof( pCurrentOptionalSetting->chWelcomeMessage3)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();
		
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chWelcomeMessage1, sizeof( pPreviousOptionalSetting->chWelcomeMessage1)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chWelcomeMessage2, sizeof( pPreviousOptionalSetting->chWelcomeMessage2)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chWelcomeMessage3, sizeof( pPreviousOptionalSetting->chWelcomeMessage3)));

		strNewValueArray.Add(CString(pCurrentOptionalSetting->chWelcomeMessage1, sizeof( pCurrentOptionalSetting->chWelcomeMessage1)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting->chWelcomeMessage2, sizeof( pCurrentOptionalSetting->chWelcomeMessage2)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting->chWelcomeMessage3, sizeof( pCurrentOptionalSetting->chWelcomeMessage3)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"WELCOME MESSAGE", strOldValueArray, strNewValueArray); 
	}
	//Pre-Balance
	if ( pPreviousOptionalSetting->bPreBalance != pCurrentOptionalSetting->bPreBalance )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting->bPreBalance);
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting->bPreBalance);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"BALANCE AT START", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->bPreBalance != pCurrentOptionalSetting->bPreBalance )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting->bPreBalance);
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting->bPreBalance);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"BALANCE AT START", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->bPercentSurcharge != pCurrentOptionalSetting->bPercentSurcharge )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting->bPercentSurcharge);
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting->bPercentSurcharge);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SURCHARGE PERCENT", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->dPerSurchargeManner != pCurrentOptionalSetting->dPerSurchargeManner )
	{
		strOldValue = GET_SURMANNER_STRING(pPreviousOptionalSetting->dPerSurchargeManner);
		strNewValue = GET_SURMANNER_STRING(pCurrentOptionalSetting->dPerSurchargeManner);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SURCHARGE PERCENT", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->dSurchargePercent != pCurrentOptionalSetting->dSurchargePercent )
	{
		strOldValue.Format(L"%d (%)", pPreviousOptionalSetting->dSurchargePercent);
		strNewValue.Format(L"%d (%)", pCurrentOptionalSetting->dSurchargePercent);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"PERCENTAGE", strOldValue, strNewValue); 
	}	

	if ( pPreviousOptionalSetting->bContinueTrans != pCurrentOptionalSetting->bContinueTrans )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting->bContinueTrans);
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting->bContinueTrans);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"CONTINUE", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->dwWithdrawalSurcharge != pCurrentOptionalSetting->dwWithdrawalSurcharge )
	{
		strOldValue.Format(L"%d", pPreviousOptionalSetting->dwWithdrawalSurcharge) ;
		strNewValue.Format(L"%d", pCurrentOptionalSetting->dwWithdrawalSurcharge) ;

		strOldValue = MakeMoneyCent(strOldValue);
		strNewValue = MakeMoneyCent(strNewValue);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"WITHDRAWAL SURCHARGE AMOUNT", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->dwBalanceSurcharge != pCurrentOptionalSetting->dwBalanceSurcharge )
	{
		strOldValue.Format(L"%d", pPreviousOptionalSetting->dwBalanceSurcharge) ;
		strNewValue.Format(L"%d", pCurrentOptionalSetting->dwBalanceSurcharge) ;

		strOldValue = MakeMoneyCent(strOldValue);
		strNewValue = MakeMoneyCent(strNewValue);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"BALANCE SURCHARGE AMOUNT", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->dwPinChangeSurcharge != pCurrentOptionalSetting->dwPinChangeSurcharge )
	{
		strOldValue.Format(L"%d", pPreviousOptionalSetting->dwPinChangeSurcharge) ;
		strNewValue.Format(L"%d", pCurrentOptionalSetting->dwPinChangeSurcharge) ;

		strOldValue = MakeMoneyCent(strOldValue);
		strNewValue = MakeMoneyCent(strNewValue);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"PIN CHANGE SURCHARGE AMOUNT", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->dwDefaultLanguage != pCurrentOptionalSetting->dwDefaultLanguage )
	{
		strOldValue = GET_LANGUAGE_STRING(pPreviousOptionalSetting->dwDefaultLanguage);
		strNewValue = GET_LANGUAGE_STRING(pCurrentOptionalSetting->dwDefaultLanguage);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"DEFAULT LANGUAGE", strOldValue, strNewValue); 
	}	

	if ( memcmp(pPreviousOptionalSetting->chExitMessage1 , pCurrentOptionalSetting->chExitMessage1, sizeof( pCurrentOptionalSetting->chExitMessage1))
		|| memcmp(pPreviousOptionalSetting->chExitMessage2 , pCurrentOptionalSetting->chExitMessage2, sizeof( pCurrentOptionalSetting->chExitMessage2))
		|| memcmp(pPreviousOptionalSetting->chExitMessage3 , pCurrentOptionalSetting->chExitMessage3, sizeof( pCurrentOptionalSetting->chExitMessage3)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousOptionalSetting->chExitMessage1, sizeof( pPreviousOptionalSetting->chExitMessage1)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chExitMessage2, sizeof( pPreviousOptionalSetting->chExitMessage2)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chExitMessage3, sizeof( pPreviousOptionalSetting->chExitMessage3)));

		strNewValueArray.Add(CString(pCurrentOptionalSetting->chExitMessage1, sizeof( pCurrentOptionalSetting->chExitMessage1)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting->chExitMessage2, sizeof( pCurrentOptionalSetting->chExitMessage2)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting->chExitMessage3, sizeof( pCurrentOptionalSetting->chExitMessage3)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"EXIT MESSAGE", strOldValueArray, strNewValueArray); 
	}

	if ( memcmp(pPreviousOptionalSetting->chStoreMessage1 , pCurrentOptionalSetting->chStoreMessage1, sizeof( pCurrentOptionalSetting->chStoreMessage1))
		|| memcmp(pPreviousOptionalSetting->chStoreMessage2 , pCurrentOptionalSetting->chStoreMessage2, sizeof( pCurrentOptionalSetting->chStoreMessage2))
		|| memcmp(pPreviousOptionalSetting->chStoreMessage3 , pCurrentOptionalSetting->chStoreMessage3, sizeof( pCurrentOptionalSetting->chStoreMessage3))
		|| memcmp(pPreviousOptionalSetting->chStoreMessage4 , pCurrentOptionalSetting->chStoreMessage4, sizeof( pCurrentOptionalSetting->chStoreMessage4)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousOptionalSetting->chStoreMessage1,sizeof(pPreviousOptionalSetting->chStoreMessage1)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chStoreMessage2,sizeof(pPreviousOptionalSetting->chStoreMessage2)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chStoreMessage3,sizeof(pPreviousOptionalSetting->chStoreMessage3)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chStoreMessage4,sizeof(pPreviousOptionalSetting->chStoreMessage4)));

		strNewValueArray.Add(CString(pCurrentOptionalSetting->chStoreMessage1,sizeof(pPreviousOptionalSetting->chStoreMessage1)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting->chStoreMessage2,sizeof(pPreviousOptionalSetting->chStoreMessage2)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting->chStoreMessage3,sizeof(pPreviousOptionalSetting->chStoreMessage3)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting->chStoreMessage4,sizeof(pPreviousOptionalSetting->chStoreMessage4)));

		m_JNLMgr.SaveChangeParameterJnl(nUser, L"STORE MESSAGE", strOldValueArray, strNewValueArray); 
	}
	if ( memcmp(pPreviousOptionalSetting->chMarketingMessage1 , pCurrentOptionalSetting->chMarketingMessage1, sizeof( pCurrentOptionalSetting->chMarketingMessage1))
		|| memcmp(pPreviousOptionalSetting->chMarketingMessage2 , pCurrentOptionalSetting->chMarketingMessage2, sizeof( pCurrentOptionalSetting->chMarketingMessage2))
		|| memcmp(pPreviousOptionalSetting->chMarketingMessage3 , pCurrentOptionalSetting->chMarketingMessage3, sizeof( pCurrentOptionalSetting->chMarketingMessage3))
		|| memcmp(pPreviousOptionalSetting->chMarketingMessage4 , pCurrentOptionalSetting->chMarketingMessage4, sizeof( pCurrentOptionalSetting->chMarketingMessage4)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousOptionalSetting->chMarketingMessage1, sizeof( pCurrentOptionalSetting->chMarketingMessage1)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chMarketingMessage2, sizeof( pCurrentOptionalSetting->chMarketingMessage2)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chMarketingMessage3, sizeof( pCurrentOptionalSetting->chMarketingMessage3)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chMarketingMessage4, sizeof( pCurrentOptionalSetting->chMarketingMessage4)));

		strNewValueArray.Add(CString(pCurrentOptionalSetting->chMarketingMessage1, sizeof( pCurrentOptionalSetting->chMarketingMessage1)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting->chMarketingMessage2, sizeof( pCurrentOptionalSetting->chMarketingMessage2)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting->chMarketingMessage3, sizeof( pCurrentOptionalSetting->chMarketingMessage3)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting->chMarketingMessage4, sizeof( pCurrentOptionalSetting->chMarketingMessage4)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"MARKETING MESSAGE", strOldValueArray, strNewValueArray); 
	}
	if ( memcmp(pPreviousOptionalSetting->chProcessorMessage1 , pCurrentOptionalSetting->chProcessorMessage1, sizeof( pCurrentOptionalSetting->chProcessorMessage1))
		|| memcmp(pPreviousOptionalSetting->chProcessorMessage2 , pCurrentOptionalSetting->chProcessorMessage2, sizeof( pCurrentOptionalSetting->chProcessorMessage2))
		|| memcmp(pPreviousOptionalSetting->chProcessorMessage3 , pCurrentOptionalSetting->chProcessorMessage3, sizeof( pCurrentOptionalSetting->chProcessorMessage3))
		|| memcmp(pPreviousOptionalSetting->chProcessorMessage4 , pCurrentOptionalSetting->chProcessorMessage4, sizeof( pCurrentOptionalSetting->chProcessorMessage4)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousOptionalSetting->chProcessorMessage1, sizeof( pCurrentOptionalSetting->chProcessorMessage1)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chProcessorMessage2, sizeof( pCurrentOptionalSetting->chProcessorMessage2)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chProcessorMessage3, sizeof( pCurrentOptionalSetting->chProcessorMessage3)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting->chProcessorMessage4, sizeof( pCurrentOptionalSetting->chProcessorMessage4)));

		strNewValueArray.Add(CString(pCurrentOptionalSetting->chProcessorMessage1, sizeof( pCurrentOptionalSetting->chProcessorMessage1)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting->chProcessorMessage2, sizeof( pCurrentOptionalSetting->chProcessorMessage2)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting->chProcessorMessage3, sizeof( pCurrentOptionalSetting->chProcessorMessage3)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting->chProcessorMessage4, sizeof( pCurrentOptionalSetting->chProcessorMessage4)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"PROCESSOR MESSAGE", strOldValueArray, strNewValueArray); 
	}
	if ( pPreviousOptionalSetting->dwInquirySurchargeAmount != pCurrentOptionalSetting->dwInquirySurchargeAmount )
	{
		strOldValue.Format(L"%d", pPreviousOptionalSetting->dwInquirySurchargeAmount) ;
		strNewValue.Format(L"%d", pCurrentOptionalSetting->dwInquirySurchargeAmount) ;

		strOldValue = MakeMoneyCent(strOldValue);
		strNewValue = MakeMoneyCent(strNewValue);
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"INQUIRY ATM FEE AMOUNT", strOldValue, strNewValue); 
	}
	// Certification Key, EMV Available는 생략함 
	if ( pPreviousOptionalSetting->bEMVEnable != pCurrentOptionalSetting->bEMVEnable )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting->bEMVEnable) ;
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting->bEMVEnable) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"EMV", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->nSurchargeWarningLocation != pCurrentOptionalSetting->nSurchargeWarningLocation )
	{
		strOldValue = GET_SURCHARGELOCATION_STRING(pPreviousOptionalSetting->nSurchargeWarningLocation) ;
		strNewValue = GET_SURCHARGELOCATION_STRING(pCurrentOptionalSetting->nSurchargeWarningLocation) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SURCHARGE WARNING LOCATION", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->bEMVTransactionOption != pCurrentOptionalSetting->bEMVTransactionOption )
	{
		strOldValue = GET_EMV_TRAN_STRING(pPreviousOptionalSetting->bEMVTransactionOption) ;
		strNewValue = GET_EMV_TRAN_STRING(pCurrentOptionalSetting->bEMVTransactionOption) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"EMV TRANSACTION", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->dwAutoDayTotal != pCurrentOptionalSetting->dwAutoDayTotal )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting->dwAutoDayTotal) ;
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting->dwAutoDayTotal) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"AUTO DAY TOTAL", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting->dwAutoDayTotalType != pCurrentOptionalSetting->dwAutoDayTotalType )
	{
		strOldValue = GET_TOTALTYPE_STRING(pPreviousOptionalSetting->dwAutoDayTotalType) ;
		strNewValue = GET_TOTALTYPE_STRING(pCurrentOptionalSetting->dwAutoDayTotalType) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"AUTO DAY TOTAL TYPE", strOldValue, strNewValue); 
	}	

	if ( pPreviousOptionalSetting->dwAutoDayTotalTime_Hour != pCurrentOptionalSetting->dwAutoDayTotalTime_Hour 
		|| pPreviousOptionalSetting->dwAutoDayTotalTime_Min != pCurrentOptionalSetting->dwAutoDayTotalTime_Min)
	{
		strOldValue.Format(L"%02d:%02d", pPreviousOptionalSetting->dwAutoDayTotalTime_Hour,pPreviousOptionalSetting->dwAutoDayTotalTime_Min ) ;
		strNewValue.Format(L"%02d:%02d", pCurrentOptionalSetting->dwAutoDayTotalTime_Hour,pCurrentOptionalSetting->dwAutoDayTotalTime_Min ) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"AUTO DAY TOTAL TIME(HH:mm)", strOldValue, strNewValue); 
	}	
	if ( strOldValueArray.GetSize() > 0) strOldValueArray.RemoveAll();
	if ( strNewValueArray.GetSize() > 0) strNewValueArray.RemoveAll();
NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() ** OPTINALSETTING** End\n")));

}
/*-------------------------------------------------------------------
 FUNCTION NAME: SaveOptionalSettingParameter()
 RETURN TYPE  : 
 PARAMETER    : OPTINALSETTING2 
 DESCRIPTION  : 변경 완료 후 미리 저장해 놓은 값고 현재 값을 비교하여 저널에 저장함. 
-------------------------------------------------------------------*/
void CDevCmn::SaveOptionalSetting2Parameter(int nUser, LPOPTIONALSETTING2 pPreviousOptionalSetting2, LPOPTIONALSETTING2 pCurrentOptionalSetting2)
{
NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() ** OPTINALSETTING2** Start\n")));

	CString strParameter;
	CString strOldValue, strNewValue;
	CStringArray strOldValueArray, strNewValueArray;


	if ( memcmp(pPreviousOptionalSetting2->chSurchargeOwnerContact1 , pCurrentOptionalSetting2->chSurchargeOwnerContact1, sizeof( pCurrentOptionalSetting2->chSurchargeOwnerContact1))
		|| memcmp(pPreviousOptionalSetting2->chSurchargeOwnerContact2 , pCurrentOptionalSetting2->chSurchargeOwnerContact2, sizeof( pCurrentOptionalSetting2->chSurchargeOwnerContact2))
		|| memcmp(pPreviousOptionalSetting2->chSurchargeOwnerContact3 , pCurrentOptionalSetting2->chSurchargeOwnerContact3, sizeof( pCurrentOptionalSetting2->chSurchargeOwnerContact3)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousOptionalSetting2->chSurchargeOwnerContact1, sizeof( pCurrentOptionalSetting2->chSurchargeOwnerContact1)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting2->chSurchargeOwnerContact2, sizeof( pCurrentOptionalSetting2->chSurchargeOwnerContact2)));
		strOldValueArray.Add(CString(pPreviousOptionalSetting2->chSurchargeOwnerContact3, sizeof( pCurrentOptionalSetting2->chSurchargeOwnerContact3)));

		strNewValueArray.Add(CString(pCurrentOptionalSetting2->chSurchargeOwnerContact1,  sizeof( pCurrentOptionalSetting2->chSurchargeOwnerContact1)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting2->chSurchargeOwnerContact2,  sizeof( pCurrentOptionalSetting2->chSurchargeOwnerContact2)));
		strNewValueArray.Add(CString(pCurrentOptionalSetting2->chSurchargeOwnerContact3,  sizeof( pCurrentOptionalSetting2->chSurchargeOwnerContact3)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"CONTACT INFO", strOldValueArray, strNewValueArray); 
	}

	if ( memcmp(pPreviousOptionalSetting2->chRMSModemInitialString , pCurrentOptionalSetting2->chRMSModemInitialString, sizeof( pCurrentOptionalSetting2->chRMSModemInitialString)))
	{
		strOldValue = CString(pPreviousOptionalSetting2->chRMSModemInitialString);
		strNewValue = CString(pCurrentOptionalSetting2->chRMSModemInitialString);
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RMS MODEM INITIAL STRING", strOldValue, strNewValue); 
	}
	if ( memcmp(pPreviousOptionalSetting2->chMachineKind , pCurrentOptionalSetting2->chMachineKind, sizeof( pCurrentOptionalSetting2->chMachineKind)))
	{
		strOldValue = CString(pPreviousOptionalSetting2->chMachineKind);
		strNewValue = CString(pCurrentOptionalSetting2->chMachineKind);
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"MACHINE KIND", strOldValue, strNewValue); 
	}
	if ( pPreviousOptionalSetting2->wMultiCurrencyFlag != pCurrentOptionalSetting2->wMultiCurrencyFlag )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting2->wMultiCurrencyFlag) ;
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting2->wMultiCurrencyFlag) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"MULTI CURRENCY", strOldValue, strNewValue); 
	}	
	if ( memcmp(pPreviousOptionalSetting2->chExchangeRate , pCurrentOptionalSetting2->chExchangeRate, sizeof( pCurrentOptionalSetting2->chExchangeRate)))
	{
		strOldValue = CString(pPreviousOptionalSetting2->chExchangeRate);
		strNewValue = CString(pCurrentOptionalSetting2->chExchangeRate);
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"EXCHANGE RATE", strOldValue, strNewValue); 
	}
	//Currency ID skip
	if ( pPreviousOptionalSetting2->wPreDialFlag != pCurrentOptionalSetting2->wPreDialFlag )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting2->wPreDialFlag) ;
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting2->wPreDialFlag) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"PRE DIALING", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting2->wPreDialOption != pCurrentOptionalSetting2->wPreDialOption )
	{
		strOldValue = GET_PREDIAL_OPTION_STRING(pPreviousOptionalSetting2->wPreDialOption) ;
		strNewValue = GET_PREDIAL_OPTION_STRING(pCurrentOptionalSetting2->wPreDialOption) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"PRE DIALING", strOldValue, strNewValue); 
	}	

	if ( pPreviousOptionalSetting2->wNeedMoreTime != pCurrentOptionalSetting2->wNeedMoreTime )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting2->wNeedMoreTime) ;
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting2->wNeedMoreTime) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"NEED MORE TIME", strOldValue, strNewValue); 
	}	
	for ( int i = 0; i < 6; i++)
	{
		if ( pPreviousOptionalSetting2->wAwardCouponFlag[i] != pCurrentOptionalSetting2->wAwardCouponFlag[i] )
		{
			strParameter.Format(L"AWARD COUPON %d", i+1);
			strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting2->wAwardCouponFlag[i]) ;
			strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting2->wAwardCouponFlag[i]) ;
			
			m_JNLMgr.SaveChangeParameterJnl(nUser, strParameter, strOldValue, strNewValue); 
		}	
	}
	if ( pPreviousOptionalSetting2->wTerminalStatusFieldEnable != pCurrentOptionalSetting2->wTerminalStatusFieldEnable )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting2->wTerminalStatusFieldEnable) ;
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting2->wTerminalStatusFieldEnable) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"TERMINAL STATUS", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting2->wReversalAtHostError != pCurrentOptionalSetting2->wReversalAtHostError )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting2->wReversalAtHostError) ;
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting2->wReversalAtHostError) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"REVERSAL AT HOST ERROR", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting2->wPaperLowSensorEnable != pCurrentOptionalSetting2->wPaperLowSensorEnable )
	{
		strOldValue = GET_REVERSE_ENDIS_STRING(pPreviousOptionalSetting2->wPaperLowSensorEnable) ;
		strNewValue = GET_REVERSE_ENDIS_STRING(pCurrentOptionalSetting2->wPaperLowSensorEnable) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RECEIPT PAPER LOW SENSOR", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting2->wEPPFlickerOption != pCurrentOptionalSetting2->wEPPFlickerOption )
	{
		strOldValue = GET_EPP_OPTION_STRING(pPreviousOptionalSetting2->wEPPFlickerOption) ;
		strNewValue = GET_EPP_OPTION_STRING(pCurrentOptionalSetting2->wEPPFlickerOption) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"EPP FLICKER ON", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting2->wCSTSoundOption != pCurrentOptionalSetting2->wCSTSoundOption )
	{
		strOldValue = GET_CST_SOUND_OPTION_STRING(pPreviousOptionalSetting2->wCSTSoundOption) ;
		strNewValue = GET_CST_SOUND_OPTION_STRING(pCurrentOptionalSetting2->wCSTSoundOption) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"CST SOUND", strOldValue, strNewValue); 
	}	
	if ( pPreviousOptionalSetting2->wEmvLatchEnable != pCurrentOptionalSetting2->wEmvLatchEnable )
	{
		strOldValue = GET_REVERSE_ENDIS_STRING(pPreviousOptionalSetting2->wEmvLatchEnable) ;
		strNewValue = GET_REVERSE_ENDIS_STRING(pCurrentOptionalSetting2->wEmvLatchEnable) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"EMV LATCH", strOldValue, strNewValue); 
	}	

	if ( pPreviousOptionalSetting2->wReasonForReversal != pCurrentOptionalSetting2->wReasonForReversal )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting2->wReasonForReversal) ;
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting2->wReasonForReversal) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"REASON FOR REVERSAL", strOldValue, strNewValue); 
	}	

	if ( pPreviousOptionalSetting2->wScheduledJournalEnable != pCurrentOptionalSetting2->wScheduledJournalEnable )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting2->wScheduledJournalEnable) ;
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting2->wScheduledJournalEnable) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SCHEDULED JOURNAL UPLOAD", strOldValue, strNewValue); 
	}
	if ( pPreviousOptionalSetting2->wScheduledJournalOption != pCurrentOptionalSetting2->wScheduledJournalOption )
	{
		strOldValue = GET_SCH_JOURNAL_OPTION_STRING(pPreviousOptionalSetting2->wScheduledJournalOption) ;
		strNewValue = GET_SCH_JOURNAL_OPTION_STRING(pCurrentOptionalSetting2->wScheduledJournalOption) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SCHEDULED JOURNAL TYPE", strOldValue, strNewValue); 
	}

	if ( pPreviousOptionalSetting2->wScheduledJournalCount != pCurrentOptionalSetting2->wScheduledJournalCount )
	{
		strOldValue.Format(L"%d", pPreviousOptionalSetting2->wScheduledJournalCount) ;
		strNewValue.Format(L"%d", pCurrentOptionalSetting2->wScheduledJournalCount) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SCHEDULED JOURNAL COUNT", strOldValue, strNewValue); 
	}

	if ( pPreviousOptionalSetting2->wScheduledJournal_Day != pCurrentOptionalSetting2->wScheduledJournal_Day )
	{
		strOldValue.Format(L"%d", pPreviousOptionalSetting2->wScheduledJournal_Day) ;
		strNewValue.Format(L"%d", pCurrentOptionalSetting2->wScheduledJournal_Day) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SCHEDULED JOURNAL DAY", strOldValue, strNewValue); 
	}

	if ( pPreviousOptionalSetting2->wScheduledJournal_Hour != pCurrentOptionalSetting2->wScheduledJournal_Hour )
	{
		strOldValue.Format(L"%d", pPreviousOptionalSetting2->wScheduledJournal_Hour) ;
		strNewValue.Format(L"%d", pCurrentOptionalSetting2->wScheduledJournal_Hour) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SCHEDULED JOURNAL HOUR", strOldValue, strNewValue); 
	}

	// [#615] AU_C KSK 2010.01.21
	if ( pPreviousOptionalSetting2->wConfigurationOption != pCurrentOptionalSetting2->wConfigurationOption )
	{
		strOldValue = GET_REVERSE_ENDIS_STRING( pPreviousOptionalSetting2->wConfigurationOption) ;
		strNewValue = GET_REVERSE_ENDIS_STRING( pCurrentOptionalSetting2->wConfigurationOption) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"CONFIG DOWNLOAD AT START", strOldValue, strNewValue); 
	}
	// end of [#615]

	// [#616] AU_C SOOK 2010.02.05 Transaction AD
	if ( pPreviousOptionalSetting2->wTransactionAdver1Enable  != pCurrentOptionalSetting2->wTransactionAdver1Enable  )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting2->wTransactionAdver1Enable ) ;
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting2->wTransactionAdver1Enable ) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"TRANSACTION ADVERTISEMENT1", strOldValue, strNewValue); 
	}
	if ( pPreviousOptionalSetting2->wTransactionAdver2Enable  != pCurrentOptionalSetting2->wTransactionAdver2Enable  )
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting2->wTransactionAdver2Enable ) ;
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting2->wTransactionAdver2Enable ) ;
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"TRANSACTION ADVERTISEMENT2", strOldValue, strNewValue); 
	}
	//end of [#616]

	//SOOK 2010.09.07 
	if ( pPreviousOptionalSetting2->wBankNameFeePrint != pCurrentOptionalSetting2->wBankNameFeePrint)
	{
		strOldValue = 	GET_ENDIS_STRING(pPreviousOptionalSetting2->wBankNameFeePrint);
		strNewValue = 	GET_ENDIS_STRING(pCurrentOptionalSetting2->wBankNameFeePrint);
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"BANK NAME AND FEE PRINT", strOldValue, strNewValue);
	}

	if ( pPreviousOptionalSetting2->wNoticeServiceEnable != pCurrentOptionalSetting2->wNoticeServiceEnable)
	{
		strOldValue = 	GET_ENDIS_STRING(pPreviousOptionalSetting2->wNoticeServiceEnable);
		strNewValue = 	GET_ENDIS_STRING(pCurrentOptionalSetting2->wNoticeServiceEnable);
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"NOTICE SERVICE", strOldValue, strNewValue);
	}

	if ( memcmp(pPreviousOptionalSetting2->chNoticeServiceTitle , pCurrentOptionalSetting2->chNoticeServiceTitle, sizeof( pCurrentOptionalSetting2->chNoticeServiceTitle)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousOptionalSetting2->chNoticeServiceTitle, sizeof( pCurrentOptionalSetting2->chNoticeServiceTitle)));

		strNewValueArray.Add(CString(pCurrentOptionalSetting2->chNoticeServiceTitle,  sizeof( pCurrentOptionalSetting2->chNoticeServiceTitle)));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"NOTICE TITLE", strOldValueArray, strNewValueArray); 
	}

	//[3][40]배열로 잡아서 그냥 통으로 비교함 
	if ( memcmp(pPreviousOptionalSetting2->chNoticeServiceMsg , pCurrentOptionalSetting2->chNoticeServiceMsg, sizeof( pCurrentOptionalSetting2->chNoticeServiceMsg)))
	{
		strOldValueArray.RemoveAll();
		strNewValueArray.RemoveAll();

		strOldValueArray.Add(CString(pPreviousOptionalSetting2->chNoticeServiceMsg[0], sizeof( pCurrentOptionalSetting2->chNoticeServiceMsg)/3));
		strOldValueArray.Add(CString(pPreviousOptionalSetting2->chNoticeServiceMsg[1], sizeof( pCurrentOptionalSetting2->chNoticeServiceMsg)/3));
		strOldValueArray.Add(CString(pPreviousOptionalSetting2->chNoticeServiceMsg[2], sizeof( pCurrentOptionalSetting2->chNoticeServiceMsg)/3));

		strNewValueArray.Add(CString(pCurrentOptionalSetting2->chNoticeServiceMsg[0],  sizeof( pCurrentOptionalSetting2->chNoticeServiceMsg)/3));
		strNewValueArray.Add(CString(pCurrentOptionalSetting2->chNoticeServiceMsg[1],  sizeof( pCurrentOptionalSetting2->chNoticeServiceMsg)/3));
		strNewValueArray.Add(CString(pCurrentOptionalSetting2->chNoticeServiceMsg[2],  sizeof( pCurrentOptionalSetting2->chNoticeServiceMsg)/3));
		
		m_JNLMgr.SaveChangeParameterJnl(nUser, L"NOTICE MESSAGE", strOldValueArray, strNewValueArray); 
	}

	if ( pPreviousOptionalSetting2->wTouchVibrationEnable != pCurrentOptionalSetting2->wTouchVibrationEnable)
	{
		strOldValue = 	GET_ENDIS_STRING(pPreviousOptionalSetting2->wTouchVibrationEnable);
		strNewValue = 	GET_ENDIS_STRING(pCurrentOptionalSetting2->wTouchVibrationEnable);

		m_JNLMgr.SaveChangeParameterJnl(nUser, L"TOUCH VIBRATION", strOldValue, strNewValue);
	}

	if ( pPreviousOptionalSetting2->SSLVersion != pCurrentOptionalSetting2->SSLVersion)
	{
		// [#2310] NH KSK 2014.11.18 TLS Support
//		if (pPreviousOptionalSetting2->SSLVersion == SSL_VER_23)
//			strOldValue = _T("VERSION 23");
//		else
//			strOldValue = _T("VERSION 3");
//
//		if (pCurrentOptionalSetting2->SSLVersion == SSL_VER_23)
//			strNewValue = _T("VERSION 23");
//		else
//			strNewValue = _T("VERSION 3");
//
//		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SSL VERSION", strOldValue, strNewValue);

		// [#2320]NH Justin 2015.01.14 Support NON-"Version Negotiation" Processor
		/*
		//if (pPreviousOptionalSetting2->SSLVersion == SSL_VER3_AND_ABOVE)
		//	strOldValue = _T("SSL VER3 AND ABOVE");
		//else if (pPreviousOptionalSetting2->SSLVersion == TLS_VER10_AND_ABOVE)
		//	strOldValue = _T("TLS VER1.0 AND ABOVE");
		//else if (pPreviousOptionalSetting2->SSLVersion == TLS_VER11_AND_ABOVE)
		//	strOldValue = _T("TLS VER1.1 AND ABOVE");
		//else if (pPreviousOptionalSetting2->SSLVersion == TLS_VER12)
		//	strOldValue = _T("TLS VER1.2");
		//else
		//	strOldValue = _T("NOT DEFINED");

		//if (pCurrentOptionalSetting2->SSLVersion == SSL_VER3_AND_ABOVE)
		//	strNewValue = _T("SSL VER3 AND ABOVE");
		//else if (pCurrentOptionalSetting2->SSLVersion == TLS_VER10_AND_ABOVE)
		//	strNewValue = _T("TLS VER1.0 AND ABOVE");
		//else if (pCurrentOptionalSetting2->SSLVersion == TLS_VER11_AND_ABOVE)
		//	strNewValue = _T("TLS VER1.1 AND ABOVE");
		//else if (pCurrentOptionalSetting2->SSLVersion == TLS_VER12)
		//	strNewValue = _T("TLS VER1.2");
		//else
		//	strNewValue = _T("NOT DEFINED");
		*/

		if (pPreviousOptionalSetting2->SSLVersion == CONN_UPTO_SSL_V30)
			strOldValue = _T("SSL V3.0");
		else if (pPreviousOptionalSetting2->SSLVersion == CONN_UPTO_TLS_V10)
			strOldValue = _T("UP TO TLS V1.0");
		else if (pPreviousOptionalSetting2->SSLVersion == CONN_UPTO_TLS_V11)
			strOldValue = _T("UP TO TLS V1.1");
		else if (pPreviousOptionalSetting2->SSLVersion == CONN_UPTO_TLS_V12)
			strOldValue = _T("UP TO TLS V1.2");
		else
			strOldValue = _T("NOT DEFINED");

		if (pCurrentOptionalSetting2->SSLVersion == CONN_UPTO_SSL_V30)
			strNewValue = _T("SSL V3.0");
		else if (pCurrentOptionalSetting2->SSLVersion == CONN_UPTO_TLS_V10)
			strNewValue = _T("UP TO TLS V1.0");
		else if (pCurrentOptionalSetting2->SSLVersion == CONN_UPTO_TLS_V11)
			strNewValue = _T("UP TO TLS V1.1");
		else if (pCurrentOptionalSetting2->SSLVersion == CONN_UPTO_TLS_V12)
			strNewValue = _T("UP TO TLS V1.2");
		else
			strNewValue = _T("NOT DEFINED");
		// End of [#2320]

		m_JNLMgr.SaveChangeParameterJnl(nUser, L"SSL/TLS VERSION", strOldValue, strNewValue);
		// end of [#2310]
	}
	//end of SOOK 2010.09.07

	// [#2241] AU KMK 2014.02.27 Note Counting
	if ( pPreviousOptionalSetting2->dwCDUMNegativeCountEnable != pCurrentOptionalSetting2->dwCDUMNegativeCountEnable)
	{
		strOldValue = GET_ENDIS_STRING(pPreviousOptionalSetting2->dwCDUMNegativeCountEnable);
		strNewValue = GET_ENDIS_STRING(pCurrentOptionalSetting2->dwCDUMNegativeCountEnable);

		m_JNLMgr.SaveChangeParameterJnl(nUser, L"NEGATIVE COUNT", strOldValue, strNewValue); 
	}
	// end of [#2241]

	// [#2234] AU KMK 2014.02.27 MoniView Timeout
	if ( pPreviousOptionalSetting2->dwRMSTimeout_Connect_TcpIp != pCurrentOptionalSetting2->dwRMSTimeout_Connect_TcpIp)
	{
		strOldValue.Format(L"%d", pPreviousOptionalSetting2->dwRMSTimeout_Connect_TcpIp) ;
		strNewValue.Format(L"%d", pCurrentOptionalSetting2->dwRMSTimeout_Connect_TcpIp) ;

		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RMS CONNECT TIMEOUT", strOldValue, strNewValue); 
	}
	if ( pPreviousOptionalSetting2->dwRMSTimeout_Receive_TcpIp != pCurrentOptionalSetting2->dwRMSTimeout_Receive_TcpIp)
	{
		strOldValue.Format(L"%d", pPreviousOptionalSetting2->dwRMSTimeout_Receive_TcpIp) ;
		strNewValue.Format(L"%d", pCurrentOptionalSetting2->dwRMSTimeout_Receive_TcpIp) ;

		m_JNLMgr.SaveChangeParameterJnl(nUser, L"RMS RECEIVE TIMEOUT", strOldValue, strNewValue); 
	}
	// end of [#2234]


	if ( strOldValueArray.GetSize() > 0) strOldValueArray.RemoveAll();
	if ( strNewValueArray.GetSize() > 0) strNewValueArray.RemoveAll();
NHDEBUG(1, (_T("***CDevCmn::SaveChangeParameter() ** OPTINALSETTING2** End\n")));

}

#endif
