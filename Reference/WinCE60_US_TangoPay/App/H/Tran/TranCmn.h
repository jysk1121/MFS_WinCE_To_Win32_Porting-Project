//#if !defined(AFX_TRANCMN_H__94D1FADB_96A6_4E42_A156_4886674FB303__INCLUDED_)
//#define AFX_TRANCMN_H__94D1FADB_96A6_4E42_A156_4886674FB303__INCLUDED_

#ifndef __TRAN_CMN_H__
#define __TRAN_CMN_H__

#ifndef _TRANCTRL_OWNER_
#pragma comment(lib, "TranCtrl.lib")
#endif

//#if _MSC_VER > 1000
//#pragma once
//#endif // _MSC_VER > 1000
// TranCmn.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//#include ".\Common\NHDbgApi.h"
//#include "..\Common\Class.h"
#include ".\TranCmnHost.h"
//#include ".\TranCmnMenu.h"	// [#2075] NH KSK 2011.06.27
#include ".\TranBizDef.h"
#include ".\TranCmnDefine.h"

#include "..\Dev\DevCmn.h"
#include "..\Ada\AdaCtrl.h"

#include "..\Common\ScreenDef.h"

// Jefferson - Refactored TranDataClass into multiple classes
#include "COfferAcceptance.h"
#include "CPin4Data.h"
#include "CPopMoneyData.h"
#include "CJustCashData.h"
#include "CPaypalCCAData.h"
#include "CGivePayData.h"

#include ".\Tran\LibertyXService.h"
#include ".\Tran\DigitalMintService.h"
#include ".\Common\PAIConfigurationManager.h"
#include ".\Tran\CashDepotService.h"
#include ".\Tran\TangoPayService.h"		// [RWC6-676] SKKim 2024.03.28

#if(APP_B4U)
#include "B4UService.h"				// [#RWC6-16] Bitload4U
#endif
#include "TranBizHost_Hyosung_EJUP.h"
/////////////////////////////////////////////////////////////////////////////
// CTranCmn window

class AFX_EXT_CLASS CTranCmn// : public CWnd
{
#include "TranBizFlow.h"		// Common
#include "TranBizFlow_CA.h"		// CA Only
#include "TranBizFlow_AU_C.h"	// AU_C Only
#include "TranBizFlow_MX.h"		// MX Only		[#2115] MX KSK 2012.01.11
#include "TranBizFlow_LibertyX.h" // [#RWC6-59] US William 2019.10.09 LibertyX

#if (APP_JUST_CASH)
#	include "TranBizFlow_JustCash.h"
#endif

#if (APP_DIGITALMINT)
#	include "TranBizFlow_DigitalMint.h"

	void UpdateDigitalMintServices(CString apiUrl); // [#RWC6-399] US ryan.payton 2022.09.25 DigitalMint
#endif

#if (APP_CUSTOM_CASHDEPOT)
#	include "TranBizFlow_CashDepot.h"

	void UpdateCashDepotServices(CString apiUrl);
#endif

// [RWC6-676] Start SKKim 2024.03.028
#if (APP_TANGOPAY)
#	include "TranBizFlow_TangoPay.h"

	void UpdateTangoPayServices(CString apiURL);
#endif
// [RWC6-676] End SKKim 2024.03.28

#include "TranBizProc.h"		// Common
#include "TranBizProc_CA.h"		// CA Only
#include "TranBizProc_AU_C.h"	// AU_C Only
#include "TranBizProc_MX.h"		// MX Only		[#2115] MX KSK 2012.01.27
#include "TranBizLib.h"
#include "TranBizReceipt.h"		// Common
#include "TranBizEmv.h"			// EMV
#include "TranBizRKT.h"			// RKT	[#2075] NH KSK 2011.06.27
#include "TranBizTR34.h"		// TR34			[#RWC6-151] US Kook 2020.07.22 TR34

#include "TranBizHost.h"
#include "TranBizHost_Hyosung.h"
#include "TranBizHost_CSP200.h"
#include "TranBizHost_Triton.h"

#if(APP_B4U)
#include "TranBizFlow_B4U.h"
#endif
// end of [#2015]

#if (APP_ALPHI)
	#include "TranBizHost_ALPHI.h"     // [#2497] NH woooZ 2017.09.13  ALPHI server
#endif

/////////////////////////////////////////////////////////////////////////////
// Construction
public:
	CTranCmn();

/////////////////////////////////////////////////////////////////////////////
// Attributes
public:
/////////////////////////////////////////////////////////////////////////////
//	Builder Information Variable
/////////////////////////////////////////////////////////////////////////////
	CString	m_strNVLog;
/////////////////////////////////////////////////////////////////////////////
//	Initialize Information Variable
/////////////////////////////////////////////////////////////////////////////
	// [#2012] NH KJW 2011.02.08 WideToMulti용 변수 크기 및 동적할당으로 변경. 2K -> 500K
	//char	m_szTemp[2048];				// WideToMulti 용
	char	m_szTemp[NETBUF_SEND_SIZE];	// WideToMulti 용 [###2]
	//char*	m_szTemp;	// [###2]
	// end of [#2012]
/////////////////////////////////////////////////////////////////////////////
// [#68] UK HWANG 2008.05.15 EMV Level 2
//////////////////////////////////////////////////////////////////////////
//	MCU Status Check
//////////////////////////////////////////////////////////////////////////
	BOOL	m_bMcuExisted;
	// end of [#68]

	int		nSavedLanguage;				// [#2115] MX KSK 2012.01.28 USD 방출 모드시 명세표 출력 언어 판단 Flag
	//BOOL	bMX_MSTransFlag;			// [#2115] MX KSK 2012.01.28 International Card인 경우 FallBack 및 해당 Tag를 송신하지 않기 위한 Flag [#2291] Justin Disable

// Operations
public:

	/////////////////////////////////////////////////////////////////////////////
	//	Initialize Function
	/////////////////////////////////////////////////////////////////////////////
	void	SetOwner(CDevCmn *pDevCmn, CAdaCtrl	*pAdaCtrl);
	int		fnAPP_InitializeDS();		// Initialize DS
	int		fnAPP_SetEJournalData(int nInitial = FALSE);		// ELEC. JOURNAL DATA SETTING
	BOOL	SetAdditionalSetting();								// [#2461] NH Justin 2016.12.16 Partial Update

	BOOL  DidMediaExist();										// [#397] [NH] psc 2008.08.01 EMV(IC), MS 거래 함수 공통화
	inline void  SetMcuExisted(BOOL McuExisted) 
	{
		m_bMcuExisted = McuExisted;
	}


/////////////////////////////////////////////////////////////////////////////
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTranCmn)
	//}}AFX_VIRTUAL

private:
	CDevCmn						*m_pDevCmn;
	CAdaCtrl					*m_pAdaCtrl;
	JournalFormatterMap			*m_ejFormatters;

// Implementation
public:
	int GetMultiFunctionalSurchargeAmount(int TranCode);// [#546] SOOK 2009.08.05 다기능 BIN LIST 기능 추가함 
	virtual ~CTranCmn();
};

#endif
