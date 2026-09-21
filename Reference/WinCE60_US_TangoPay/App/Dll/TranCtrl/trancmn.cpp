/////////////////////////////////////////////////////////////////////////////
// TranCmn.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <time.h>

#include ".\Tran\TranCmn.h"
#include ".\..\..\H\Common\Markup.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

/////////////////////////////////////////////////////////////////////////////
// CTranCmn
/////////////////////////////////////////////////////////////////////////////
CTranCmn::CTranCmn()
{
	m_nBackScreenNumber = 0;
	m_nBackIndex = -1;
	m_nEnableCount = 0;
	for (int jj =0; jj < 6; jj++)
		m_nBackList[jj] = -1;

	m_arHistory.RemoveAll();
	m_pDevCmn = NULL;
	m_pAdaCtrl = NULL;
	m_bAdaPasswordMode = FALSE;
	m_bAdaTransaction = FALSE;
	//m_bAdaSuccessiveInput = FALSE;			//[#2117] US PCS 2012.01.12		// [#2427] Justin Remove
	m_HostConfig = HC_ATM;						// [#2185] US Justin 2013.05.03 PAI Dual Host

#if (US_VERSION || MX_VERSION)					// [#2115] MX KSK 2012.02.05
	m_PreBalanceStatus = PRE_BAL_NONE;
#endif

	m_bPowerOffReversal = FALSE;
	m_bStartTransaction = FALSE;

	nTerminalRemainAIDCnt = 0;
	nHostRemainBlock = 0;
	nCalcRecvTotalAIDCnt = 0;

	// Clear
	for (int i = 0; i < 2; i++)
	{
		m_sWeatherInfo[i].nYear = 0;
		m_sWeatherInfo[i].nMonth = 0;
		m_sWeatherInfo[i].nDay = 0;
		m_sWeatherInfo[i].strDayOfWeek = L"";
		m_sWeatherInfo[i].nWeatherCode = -1;
		m_sWeatherInfo[i].strTemperatureType = L"";
		m_sWeatherInfo[i].nHighestTemp = 0;
		m_sWeatherInfo[i].nLowestTemp = 0;
	}


	m_sNoticeInfo.strTitle = L"";
	m_sNoticeInfo.strLine1 = L"";
	m_sNoticeInfo.strLine2 = L"";
	m_sNoticeInfo.strLine3 = L"";

	m_nTransactionCount = 0;

	// KSK 2010.08.29 Default ARC값 추가 ("30 35")
	memset(m_szDefaultResponseCd, 0, sizeof(m_szDefaultResponseCd));
	m_szDefaultResponseCd[0] = 0x30;
	m_szDefaultResponseCd[1] = 0x35;

	nSavedLanguage = ENG_MODE;	// [#2115] MX KSK 2012.01.28 KSK 2013.05.02 MX 누락분 적용
	//bMX_MSTransFlag = FALSE;	// [#2115] MX KSK 2012.01.28 KSK 2013.05.02 MX 누락분 적용		//[#2291] Justin Disable

	//m_szTemp = NULL;	// [###2]
	//m_szTemp = new char[ NETBUF_RECV_SIZE ];	// [#2012] NH KJW 2011.02.08 WideToMulti용 변수 크기 및 동적할당으로 변경. 2K -> 500K

	// [#2012] NH KJW 2011.01.19 버퍼를 멤버에서 힙으로 이동
	m_arSendBuffer = NULL;	// [###2]
	m_arSendBuffer = new BYTE[ NETBUF_SEND_SIZE ];
	m_arRecvBuffer = NULL;	// [###2]
	m_arRecvBuffer = new BYTE[ NETBUF_RECV_SIZE ];
	// end of [#2012]

#if (APP_LIBERTYX)
	m_LXService = NULL;
	m_LXHttp = NULL;
	m_LXConfigManager = NULL;
#endif

#if (APP_DIGITALMINT)
	m_DMService = NULL;
	m_DMHttp = NULL;
	m_DMConfigManager = NULL;
#endif

#if (APP_CUSTOM_PAI)
	m_PAIConfigManager = NULL;
#endif

#if (APP_CUSTOM_CASHDEPOT)
	m_CDService = NULL;
	m_CDHttp = NULL;
#endif

	// [RWC6-676] Start SKKim 2024.03.28
#if (APP_TANGOPAY)
	m_pTangoPayConfigManager = NULL;
	m_pTangoPayHttp = NULL;
	m_pTangoPayService = NULL;
#endif
	// [RWC6-676] End SKKim 2024.03.28

	// Construct journal formatters
	m_ejFormatters = new JournalFormatterMap();
	m_ejFormatters->SetAt(NORMAL_TRX, &CreateTransactionJournalString);
	m_ejFormatters->SetAt(TRX_ERROR, &CreateTransactionJournalString);
	m_ejFormatters->SetAt(REVERSAL_TRX, &CreateTransactionJournalString);
	m_ejFormatters->SetAt(TRX_NOT_APPROVED, &CreateTransactionJournalString);
	m_ejFormatters->SetAt(COMMUNICATION_ERROR, &CreateTransactionJournalString);

	m_ejFormatters->SetAt(CHANGE_ERROR_MODE, &CreateErrorModeJournalString);

	m_ejFormatters->SetAt(ADDCASH_CST1, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST2, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST1_2, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST3, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST1_3, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST2_3, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST1_2_3, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST1_4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST2_4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST1_2_4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST3_4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST1_3_4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST2_3_4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(ADDCASH_CST1_2_3_4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST1, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST2, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST1_2, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST3, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST1_3, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST2_3, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST1_2_3, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST1_4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST2_4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST1_2_4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST3_4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST1_3_4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST2_3_4, &CreateAddCashJournalString);
	m_ejFormatters->SetAt(REMOTE_ADDCASH_CST1_2_3_4, &CreateAddCashJournalString);

	m_ejFormatters->SetAt(CHANGE_DENOM_CST1, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST2, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST1_2, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST3, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST1_3, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST2_3, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST1_2_3, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST4, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST1_4, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST2_4, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST1_2_4, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST3_4, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST1_3_4, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST2_3_4, &CreateChangeDenomJournalString);
	m_ejFormatters->SetAt(CHANGE_DENOM_CST1_2_3_4, &CreateChangeDenomJournalString);

	m_ejFormatters->SetAt(DAY_TOTAL, &CreateDayTotalJournalString);
	m_ejFormatters->SetAt(TRIALDAY_TOTAL, &CreateDayTotalJournalString);
	m_ejFormatters->SetAt(REMOTE_DAY_TOTAL, &CreateDayTotalJournalString);
	m_ejFormatters->SetAt(REMOTE_TRIALDAY_TOTAL, &CreateDayTotalJournalString);

	m_ejFormatters->SetAt(CST_TOTAL, &CreateCassetteTotalJournalString);
	m_ejFormatters->SetAt(TRIALCST_TOTAL, &CreateCassetteTotalJournalString);
	m_ejFormatters->SetAt(REMOTE_CST_TOTAL, &CreateCassetteTotalJournalString);

	m_ejFormatters->SetAt(REMOTE_CST_TOTAL, &CreateVatDayTotalJournalString);
	m_ejFormatters->SetAt(PIN4_TRIALDAY_TOTAL, &CreateVatDayTotalJournalString);
	m_ejFormatters->SetAt(POPMONEY_DAY_TOTAL, &CreateVatDayTotalJournalString);
	m_ejFormatters->SetAt(POPMONEY_TRIALDAY_TOTAL, &CreateVatDayTotalJournalString);
	m_ejFormatters->SetAt(JUSTCASH_DAY_TOTAL, &CreateVatDayTotalJournalString);
	m_ejFormatters->SetAt(JUSTCASH_TRIALDAY_TOTAL, &CreateVatDayTotalJournalString);
	m_ejFormatters->SetAt(PAYPAL_DAY_TOTAL, &CreateVatDayTotalJournalString);
	m_ejFormatters->SetAt(PAYPAL_TRIALDAY_TOTAL, &CreateVatDayTotalJournalString);
	m_ejFormatters->SetAt(DIGITALCUR_DAY_TOTAL, &CreateVatDayTotalJournalString);
	m_ejFormatters->SetAt(DIGITALCUR_TRIALDAY_TOTAL, &CreateVatDayTotalJournalString);
	m_ejFormatters->SetAt(B4U_DAY_TOTAL, &CreateVatDayTotalJournalString);
	m_ejFormatters->SetAt(B4U_TRIALDAY_TOTAL, &CreateVatDayTotalJournalString);

	m_ejFormatters->SetAt(OPERATOR_ACTION, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CHANGE_EXCHANGE_RATE, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CANCEL_AT_READ_CARD, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CANCEL_AT_SELECT_LANGUAGE, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CANCEL_AT_INPUT_PIN, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CANCEL_AT_SELECT_TRAN, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CANCEL_AT_CONFIRM_FEE, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CANCEL_AT_RECV_RECEIPT, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CANCEL_AT_RECEIPT_ERROR, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CANCEL_AT_RECEIPT_SCREEN, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CANCEL_AT_FROM_ACCOUNT, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CANCEL_AT_TO_ACCOUNT, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CANCEL_AT_FASTCASH, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CANCEL_AT_OTHER_AMOUNT, &CreateBasicTextJournalString);
	m_ejFormatters->SetAt(CANCEL_AT_TRANSFER_AMOUNT, &CreateBasicTextJournalString);

	m_ejFormatters->SetAt(CHANGE_PROCESSOR, &CreateChangeProcessorJournalString);

	m_ejFormatters->SetAt(CHANGE_MASTER_PWD, &CreateOperatorActionJournalString);
	m_ejFormatters->SetAt(CHANGE_OPERATOR_PWD, &CreateOperatorActionJournalString);
	m_ejFormatters->SetAt(CHANGE_SUPERVISOR_PWD, &CreateOperatorActionJournalString);
	m_ejFormatters->SetAt(POWER_ON_SYSTEM, &CreateOperatorActionJournalString);
	m_ejFormatters->SetAt(CHANGE_SERVICE_MODE, &CreateOperatorActionJournalString);
	m_ejFormatters->SetAt(CHANGE_OP_MODE, &CreateOperatorActionJournalString);
	m_ejFormatters->SetAt(CHANGE_OP_RESET, &CreateOperatorActionJournalString);
	m_ejFormatters->SetAt(REBOOT_SYSTEM, &CreateOperatorActionJournalString);
	m_ejFormatters->SetAt(CHANGE_DIAG_MODE, &CreateOperatorActionJournalString);

	m_ejFormatters->SetAt(LIBERTYX_TXN, &CreateLibertyXPurchaseJournalString);
	m_ejFormatters->SetAt(LIBERTYX_TXN_ERROR, &CreateLibertyXPurchaseJournalString);
	m_ejFormatters->SetAt(LIBERTYX_TXN_DISPENSE, &CreateLibertyXSellJournalString);
	m_ejFormatters->SetAt(B4U_TXN, &CreateBitload4UJournalString);
	m_ejFormatters->SetAt(DIGITALMINT_TXN, &CreateDigitalMintJournalString);
	m_ejFormatters->SetAt(DIGITALMINT_TXN_ERROR, &CreateDigitalMintJournalString);

#if (APP_TANGOPAY)	// [RWC6-676] Start SKKim 2024.04.05
	if (m_pTangoPayConfigManager == NULL)
	{
		m_pTangoPayConfigManager = new CTangoPayConfigurationManager();
		m_pTangoPayConfigManager->CheckConfigData();
	}

	m_pTangoPayConfigManager->GetConfiguration(m_TangoPayConfig);

	// [RWC6-676] test code wince (추후 real 배포시에는 identify / transaction server setting 필요 -> 주소가 안바뀐다고하여 입력 UI는 개발하지 않음)
	// dev URL 정보
	m_TangoPayConfig.strIdentifyServerURL = L"identity.dev.tangopay.com";
	m_TangoPayConfig.nIdentifyPort = 0;
	m_TangoPayConfig.strTransactionServerURL = L"transaction.dev.tangopay.com";
	m_TangoPayConfig.nTransactionPort = 0;

	// UAT URL 정보
	//m_TangoPayConfig.strIdentifyServerURL = L"identity.uat.tangopay.com";
	//m_TangoPayConfig.nIdentifyPort = 0;
	//m_TangoPayConfig.strTransactionServerURL = L"transaction.uat.tangopay.com";
	//m_TangoPayConfig.nTransactionPort = 0;

	#ifdef UNDER_CE
	// dev 환경설정 정보
	//MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID, L"D0000012");
	MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID, L"D0000011");
	//m_TangoPayConfig.strApiKey = L"395a2ea2-aa1a-412e-bc71-62921098301d";
	m_TangoPayConfig.strApiKey = L"dce9b0a9-d77c-41e3-bb7a-33b05edfd373";
	m_TangoPayConfig.strPartnerID = L"2";

	// UAT 환경설정 정보
	//MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID, L"U0000007");
	//m_TangoPayConfig.strPartnerID = L"2";
	//m_TangoPayConfig.strApiKey = L"fcd26e92-8b78-4580-a1de-9e76d7c1c80c";
	#else
	// dev 환경설정 정보
	MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID, L"D0000011");
	m_TangoPayConfig.strPartnerID = L"2";
	m_TangoPayConfig.strApiKey = L"dce9b0a9-d77c-41e3-bb7a-33b05edfd373";

	// UAT 환경설정 정보
	//m_TangoPayConfig.strPartnerID = L"2";
	//MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID, L"U0000004");
	//m_TangoPayConfig.strApiKey = L"3a4fe385-2476-4976-9345-7c1b362af6b3";
	#endif

	m_TangoPayConfig.strDownloadAppInfo = L"https://pay.dev.tangopay.com";	// [RWC6-676] SKKim 2024.05.29 Download App 정보 write처리

	m_pTangoPayConfigManager->SaveConfiguration(m_TangoPayConfig);

	CString enbinUrl;
	enbinUrl.Format(_T("%s:%d"), m_TangoPayConfig.strIdentifyServerURL, m_TangoPayConfig.nIdentifyPort);
	UpdateTangoPayServices(enbinUrl);
#endif

}

CTranCmn::~CTranCmn()
{
	// [#] NH KJW 2011.01.13
	if( m_arSendBuffer != NULL)
		delete [] m_arSendBuffer;
	if( m_arRecvBuffer != NULL)
		delete [] m_arRecvBuffer;
	// end of [#2011]

	//if( m_szTemp )	// [#2012] NH KJW 2011.02.08 WideToMulti용 변수 크기 및 동적할당으로 변경. 2K -> 500K // [###2]
	//	delete [] m_szTemp;

	// [#RWC6-59] US William 2019.10.08 LibertyX
#if (APP_LIBERTYX)
	if (m_LXService != NULL) delete m_LXService;
	m_LXService = NULL;

	if (m_LXHttp != NULL) delete m_LXHttp;
	m_LXHttp = NULL;

	if (m_LXConfigManager != NULL) delete m_LXConfigManager;
	m_LXConfigManager = NULL;
#endif
	// End of [#RWC6-59]

#if (APP_DIGITALMINT)
	if (m_DMService != NULL) delete m_DMService;
	m_DMService = NULL;

	if (m_DMHttp != NULL) delete m_DMHttp;
	m_DMHttp = NULL;

	if (m_DMConfigManager != NULL) delete m_DMConfigManager;
	m_DMConfigManager = NULL;
#endif

#if (APP_CUSTOM_PAI)
	if (m_PAIConfigManager != NULL) delete m_PAIConfigManager;
	m_PAIConfigManager = NULL;
#endif

#if (APP_CUSTOM_CASHDEPOT)
	if (m_CDService != NULL) delete m_CDService;
	m_CDService = NULL;

	if (m_CDHttp != NULL) delete m_CDHttp;
	m_CDHttp = NULL;
#endif

#if (APP_TANGOPAY)	// [RWC6-676] Start SKKim 2024.04.05
	if (m_pTangoPayConfigManager != NULL)	delete m_pTangoPayConfigManager;
	m_pTangoPayConfigManager = NULL;

	if (m_pTangoPayHttp != NULL)	delete m_pTangoPayHttp;
	m_pTangoPayHttp = NULL;

	if (m_pTangoPayService != NULL)	delete m_pTangoPayService;
	m_pTangoPayService = NULL;
#endif				// [RWC6-676] End SKKim 2024.04.05

	delete m_ejFormatters;
}

/////////////////////////////////////////////////////////////////////////////
// CTranCmn message handlers
/////////////////////////////////////////////////////////////////////////////
// Set Owner
//void CTranCmn::SetOwner(CMainFrame *pOwner)
void CTranCmn::SetOwner(CDevCmn *pDevCmn, CAdaCtrl	*pAdaCtrl)
{
//#include ".\Common\ClassSetOwner.h"

	m_pDevCmn = pDevCmn;
	m_pAdaCtrl = pAdaCtrl;

/////////////////////////////////////////////////////////////////////////////
//	Version & Operation Information Setting
/////////////////////////////////////////////////////////////////////////////

#ifdef	APP_LOCAL_MODE
	//m_pScrCtrl->SetDemoMode(TRUE);
	m_pDevCmn->GetCurrentScreen()->SetDemoMode(TRUE);
#endif

/////////////////////////////////////////////////////////////////////////////
//	Library Variable
/////////////////////////////////////////////////////////////////////////////
	
	srand(GetTickCount());

//EMV	if(m_pDevCmn->fnMCU_IsEmvEnable())			// [#397] [NH] psc 2008.08.01 EMV(IC) 거래일경우
//EMV	{
//EMV		fnAPP_MakeDefaultAIDProc();				// [#68] UK HWANG 2008.04.02 EMV Level2
//EMV	}

	return;
}

/////////////////////////////////////////////////////////////////////////////
//	Initialize Function
/////////////////////////////////////////////////////////////////////////////
// Initialize DS
int CTranCmn::fnAPP_InitializeDS()
{
	NHDEBUG(1, (_T("***TranCmn***CTranCmn::fnAPP_InitializeDS() \n")));

	/////////////////////////////////////////////////////////////////////////////
	//	Transaction Result
	m_pDevCmn->TranResult = FALSE;								// Host Result

	/////////////////////////////////////////////////////////////////////////////
	//	Menu Variable
	TranCode = TC_NULL;											// Transaction Code
	m_bAdaPasswordMode = FALSE;
	m_bAdaTransaction = FALSE;
	//m_bAdaSuccessiveInput = FALSE;			// [#2117] US PCS 2012.01.12		// [#2427] Justin Remove

	// [#2375] US Justin 2015.10.30 US ADA
	m_strVG_ExchangeRate.Empty();
	m_strVG_HomeCurrencyName.Empty();
	m_strVG_ConvertedAmt.Empty();
	// end of [#2375]

	m_strVG_MarkupRate.Empty();					// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message

	/////////////////////////////////////////////////////////////////////////////
	//	Initialize Enhanced Coupon Variable [#620] US KSK 2010.02.24
	m_strarrEnhancedCouponData[0].RemoveAll();
	m_strarrEnhancedCouponData[1].RemoveAll();
	m_strarrEnhancedCouponData[2].RemoveAll();
	m_strarrEnhancedCouponData[3].RemoveAll();
	m_strarrEnhancedCouponData[4].RemoveAll();
	m_strarrEnhancedCouponData[5].RemoveAll();
	
	/////////////////////////////////////////////////////////////////////////////
	//	Media Variable
	// 조회 후 연속거래시 Clear하지 말아야 할 Buffer Check
#if (US_VERSION || MX_VERSION)	// [#2115] MX KSK 2012.02.05
	if (m_PreBalanceStatus == PRE_BAL_NONE)
#endif
	{
		m_strHolderName = L"";			// [#2447] US Justin
		m_sCardData.strISO1Data = L"";
		m_sCardData.strISO2Data = L"";
		m_sCardData.strISO3Data = L"";

		m_sCardData.strBankID = L"";
		m_sCardData.strAccountNo = L"";
		m_sCardData.strShowNumber = L"";

		m_pDevCmn->m_nDCCLocalAvail = DCC_PRECHECK_OFF;	// [#2472] US Justin 2017.02.07 DCC Withdrawal Option
	}
	m_DualBalance.ClearSetting();		// [#2292] US Justin 2014.10.9 Dual Balance 

	// [#2009] NH KSK 2010.12.6 Amount가 초기화 되지 않는 Bug Fix
	/////////////////////////////////////////////////////////////////////////////
	//	Amount and Password Variable
	m_sUserSelection.nPrintReceipt = FALSE;
	m_sUserSelection.strMoney.Empty();
	m_sUserSelection.strPassword.Empty();
	m_sUserSelection.strNewPassword.Empty();		// [#2150] US Justin 2012.10.01 Implement PIN Change....
	// end of [#2009]

	/////////////////////////////////////////////////////////////////////////////
	//	Terminate Variable
	//#if NZ_VERSION	// [#314] [NH] 2008.6.19	// [#2069] NH KSK 2011.06.13
	//	UserCancelPrintFlag = FALSE;				// User Cancel Print Flag
	//#endif	// end of [#314]

	/////////////////////////////////////////////////////////////////////////////
	//	Initialize Account Information
	m_SourceAccount.Empty();
	m_DestAccount.Empty();
	SurchargeAmount = 0;							// [#2000] SOOK 2010.10.16 호주향 개발 (거래 FLOW)
	nSavedLanguage = ENG_MODE;						// [#2115] MX KSK 2012.01.28
	m_nDisplayedSurchargeAmount = 0;				// [#2150] US Justin 2012.09.28
	m_sUsedSurchargePercentage = L"";				// [#2335] US Justin 2015.03.12	Implement Percent Surcharge Notice

	/////////////////////////////////////////////////////////////////////////////
	//	History Record
	m_arHistory.RemoveAll();

	/////////////////////////////////////////////////////////////////////////////
	//	ADDITIONAL SURCHARGE TABLE MODE - [#2136] US/CA PCS 2012.05.22
	m_pDevCmn->m_strTableSurcharge.Empty();

	/////////////////////////////////////////////////////////////////////////////
	//	EMV Variable
	m_bOnlineProcessing = FALSE;
	m_pDevCmn->m_bDisplayDeclined = FALSE;		// [#2188] NH KSK 2013.05.22

	//if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
	if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE) || (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE)==ENABLE) )	// [#2242] NH JUstin.
	{	
		// Common Request Header
		m_sSTD1_CommReqHeader.RecordFormat_1.Empty();			// Record Format (H)
		m_sSTD1_CommReqHeader.ApplicationType_1.Empty();		// Application Type (= '0')
		m_sSTD1_CommReqHeader.MessageDelimiter_1.Empty();		// MessageDelimiter (= '.')
		m_sSTD1_CommReqHeader.BankID_6.Empty();					// Bank ID
		m_sSTD1_CommReqHeader.TerminalID_8.Empty();				// Terminal ID

		// Common Response Header
		m_sSTD1_CommRespHeader.RecordFormat_1.Empty();			// Record Format (H)
		m_sSTD1_CommRespHeader.ApplicationType_1.Empty();		// Application Type (= '0')
		m_sSTD1_CommRespHeader.MessageDelimiter_1.Empty();		// MessageDelimiter (= '.')
		m_sSTD1_CommRespHeader.BankID_6.Empty();				// Bank ID (6)
		m_sSTD1_CommRespHeader.TerminalID_8.Empty();			// Terminal ID

		// Config Request
		m_sSTD1_ConfigReq.RequestType_2.Empty();				// Request Type
		m_sSTD1_ConfigReq.KeyMode_1.Empty();					// Key Mode (0 ~ 11)
		m_sSTD1_ConfigReq.UKSerialNoA_8.Empty();				// Unique Serial Number A
		m_sSTD1_ConfigReq.UKSerialNoB_8.Empty();				// Unique Serial Number B
		m_sSTD1_ConfigReq.SubRequestType_2.Empty();				// --

		// Config Response
		m_sSTD1_ConfigResp.ResponseType_2.Empty();				// Response Type
		m_sSTD1_ConfigResp.LocalDate_8.Empty();					// Local Date
		m_sSTD1_ConfigResp.LocalTime_6.Empty();					// Local Time
		m_sSTD1_ConfigResp.HealthMsgTimerVal_6.Empty();			// Healthy Message Timer Value
		m_sSTD1_ConfigResp.WorkingKey_V.Empty();				// Working Key
		m_sSTD1_ConfigResp.SurchargeAmount_V.Empty();			// Surcharge Amount	 (ddcc)		** Variable
		m_sSTD1_ConfigResp.ConfigInitFlag_1.Empty();			// Configuration Initiator Flag
		m_sSTD1_ConfigResp.WorkingKey2_V.Empty();				// 2번째 암호화키
		m_sSTD1_ConfigResp.WorkingKey3_V.Empty();				// 3번째 암호화키
		m_sSTD1_ConfigResp.WorkingKey4_V.Empty();				// 4번째 암호화키
		m_sSTD1_ConfigResp.WorkingKey5_V.Empty();				// 5번째 암호화키 [#565] NH KSK 2009.8.20
		m_sSTD1_ConfigResp.WorkingKey6_V.Empty();				// 6번째 암호화키 [#565] NH KSK 2009.8.20 TDES, TMAC Option2인 경우 - 사용안함
		m_sSTD1_ConfigResp.WorkingKey7_V.Empty();				// 7번째 암호화키 [#565] NH KSK 2009.8.20 TDES, TMAC Option2인 경우 - 사용안함
		m_sSTD1_ConfigResp.SubResponseType_2.Empty();			// [#560] NH KSK 2009.8.20 EXTENDED CONFIGURATION 지원을 위해 추가

		// Extend Config Request
		m_sSTD1_ExtendConfigReq.RequestType_2.Empty();			// Request Type
		m_sSTD1_ExtendConfigReq.SubRequestType_2.Empty();		// [#560] NH KSK 2009.8.20

		// Extend Config Response
		m_sSTD1_ExtendConfigResp.ResponseType_2.Empty();		// Response Type
		m_sSTD1_ExtendConfigResp.SubResponseType_2.Empty();		// [#560] NH KSK 2009.8.20 EXTENDED CONFIGURATION 지원을 위해 추가

		// Transaction Request
		m_sSTD1_TranReq.RequestType_2.Empty();					// Request Type
		m_sSTD1_TranReq.RandomMac_8.Empty();					// Random Mac				*2
		m_sSTD1_TranReq.OperationCode_2.Empty();				// Operation Code
		m_sSTD1_TranReq.SourceAccount_2.Empty();				// Source Account
		m_sSTD1_TranReq.DestinationAccount_2.Empty();			// Destination Account
		m_sSTD1_TranReq.TransactionSequenceNo_4.Empty();		// Transaction Sequence Number
		m_sSTD1_TranReq.Track1Data_79.Empty();					// ISO Track 1 Data
		m_sSTD1_TranReq.Track2Data_40.Empty();					// ISO Track 2 Data
		m_sSTD1_TranReq.Track3Data_106.Empty();					// ISO Track 3 Data
		m_sSTD1_TranReq.PINBuffer_16.Empty();					// PIN Buffer 16
		m_sSTD1_TranReq.TransactionAmount_V.Empty();			// Variable Size	"ddddddcc"
		m_sSTD1_TranReq.SurchargeAmount_V.Empty();				// Surcharge Amount	"ddcc"
		m_sSTD1_TranReq.SurchargeEnableFlag_1.Empty();			// Surcharge Enable Flag
		m_sSTD1_TranReq.AuthrizationNumber_9.Empty();			// Authorization Number [#369] [US] KSK 2008.7.17
		m_sSTD1_TranReq.PINNumber_6.Empty();					// PIN Number [#369] [US] KSK 2008.7.17
		m_sSTD1_TranReq.MACData_8.Empty();						// MAC Data					*2

		// Transaction Response
		m_sSTD1_TranResp.ResponseType_2.Empty();				// Response Type
		m_sSTD1_TranResp.RandomMac_8.Empty();					// Random Mac				*2
		m_sSTD1_TranResp.TranSequenceNo_4.Empty();				// Transaction Sequence Number
		m_sSTD1_TranResp.ResponseCode_2.Empty();				// Response Code
		m_sSTD1_TranResp.LocalDate_8.Empty();					// Local Transaction Date
		m_sSTD1_TranResp.LocalTime_6.Empty();					// Local Transaction Time
		m_sSTD1_TranResp.RetrievalRefNo_12.Empty();				// Retrieval Reference Number
		m_sSTD1_TranResp.SystemTraceAuditNo_6.Empty();			// System Trace Audit Number
		m_sSTD1_TranResp.NetworkIDCode_2.Empty();				// Network ID Code
		m_sSTD1_TranResp.SettlementDate_8.Empty();				// Settlement Date
		m_sSTD1_TranResp.AccountBalance_V.Empty();				// Account Balance,	 (ddddddcc)	** Variable
		m_sSTD1_TranResp.AvailableBalance_V.Empty();			// Available Balance,(ddddddcc)	** Variable
		m_sSTD1_TranResp.SurchargeAmount_V.Empty();				// Surcharge Amount	 (ddcc)		** Variable
		m_sSTD1_TranResp.AuthResponseText_V.Empty();			// Authorization Response Text	** Variable
		m_sSTD1_TranResp.ConfigRequestInitiator_2.Empty();		// Configuration Request Initiator
		m_sSTD1_TranResp.MACData_8.Empty();						// MAC Data

		// Reversal Request
		m_sSTD1_ReversalReq.RequestType_2.Empty();				// Request Type
		m_sSTD1_ReversalReq.RandomMac_8.Empty();				// Random Mac				*2
		m_sSTD1_ReversalReq.LocalTranDate_8.Empty();			// Local Transaction Date
		m_sSTD1_ReversalReq.LocalTranTime_6.Empty();			// Local Transaction Time
		m_sSTD1_ReversalReq.RetrievalRefNo_12.Empty();			// Retrieval Reference Number
		m_sSTD1_ReversalReq.RequestedDollarAmt_V.Empty();		// Requested Dollar Amount		Variable
		m_sSTD1_ReversalReq.DispensedDollarAmt_V.Empty();		// Dispensed Dollar Amount		Variable
		m_sSTD1_ReversalReq.SurchargeAmount_4.Empty();			// Surcharge Amount	"ddcc"
		m_sSTD1_ReversalReq.SurchargeEnableFlag_1.Empty();		// Surcharge Enable Flag
		m_sSTD1_ReversalReq.MACData_8.Empty();					// MAC Data					*2

		// Reversal Response
		m_sSTD1_ReversalResp.ResponseType_2.Empty();			// Response Type
		m_sSTD1_ReversalResp.RandomMac_8.Empty();				// Random Mac				*2
		m_sSTD1_ReversalResp.ConfigRequestInitiator_2.Empty();	// Configuration Request Initiator
		m_sSTD1_ReversalResp.MACData_8.Empty();					// MAC Data

		// Total Request
		m_sSTD1_TotalReq.RequestType_2.Empty();					// Request Type
		m_sSTD1_TotalReq.ResetHostTotalFlag_1.Empty();			// Reset Host Total Flag

		// Total Response
		m_sSTD1_TotalResp.ResponseType_2.Empty();				// Response Type = '87'
		m_sSTD1_TotalResp.NoOfCashWiths_4.Empty();				// No of Cash Withdrawals
		m_sSTD1_TotalResp.NoOfTransfers_4.Empty();				// No of Transfers
		m_sSTD1_TotalResp.NoOfInquiries_4.Empty();				// No of Inquiries
		m_sSTD1_TotalResp.NoOfNonCashWiths_4.Empty();			// No of None Cash Withdrawals
		m_sSTD1_TotalResp.TotalDispenseAmt_V.Empty();			// Total Dispensed Amount
		m_sSTD1_TotalResp.TotalNonCashDispAmt_V.Empty();		// Total Non-Cash Dispensed Amount
		m_sSTD1_TotalResp.TotalSurchargeAmt_V.Empty();			// Total Surcharge Amount
		m_sSTD1_TotalResp.ConfigRequestInitiator_2.Empty();		// Configuration Initiator

		// Health Check Request
		m_sSTD1_HealthReq.RequestType_2.Empty();				// Request Type
		m_sSTD1_HealthReq.LocalDate_8.Empty();					// Local Transaction Date
		m_sSTD1_HealthReq.LocalTime_6.Empty();					// Local Transaction Time
		m_sSTD1_HealthReq.BillCount1_V.Empty();					// Bill Count 1
		m_sSTD1_HealthReq.BillCount2_V.Empty();					// Bill Count 2
		m_sSTD1_HealthReq.ModeType_1.Empty();					// Mode Type
		m_sSTD1_HealthReq.ErrorCode_V.Empty();					// Error Code
		m_sSTD1_HealthReq.NewJournalCount_V.Empty();			// New Journal Count
		m_sSTD1_HealthReq.ProgramVersion.Empty();				// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CDUVersion.Empty();					// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.RPUVersion.Empty();					// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.EPPVersion.Empty();					// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.TopDoorOpen.Empty();					// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.VaultDoorOpen.Empty();				// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.DispenserStatus.Empty();				// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.ReceiptPrinterStatus.Empty();			// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.ReceiptPrinterPaperStatus.Empty();	// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CardReaderStatus.Empty();				// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTDenomination[0].Empty();			// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTDenomination[1].Empty();			// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTDenomination[2].Empty();			// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTDenomination[3].Empty();			// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTLoaded[0].Empty();					// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTLoaded[1].Empty();					// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTLoaded[2].Empty();					// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTLoaded[3].Empty();					// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTDispensed[0].Empty();				// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTDispensed[1].Empty();				// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTDispensed[2].Empty();				// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTDispensed[3].Empty();				// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTRejectEvents[0].Empty();			// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTRejectEvents[1].Empty();			// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTRejectEvents[2].Empty();			// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTRejectEvents[3].Empty();			// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTBillCout[0].Empty();				// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTBillCout[1].Empty();				// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTBillCout[2].Empty();				// Extended HealthCheck Message (Tranax Format임)
		m_sSTD1_HealthReq.CSTBillCout[3].Empty();				// Extended HealthCheck Message (Tranax Format임)

		// Health Check Response
		m_sSTD1_HealthResp.ResponseType_2.Empty();				// Response Type = '90'
		m_sSTD1_HealthResp.ConfigRequestInitiator_2.Empty();	// Configuration Request Initiator
	}

	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE) // [#2242] NH JUstin.
	{
		// Common Header
		m_sSTD3_CommHeader.m_strCommsHeaderID_8.Empty();
		m_sSTD3_CommHeader.m_strTerminalID_2.Empty();			// "td" fixed data
		m_sSTD3_CommHeader.m_strSoftwareVersionNo_2.Empty();
		m_sSTD3_CommHeader.m_strEncryptionModeFlag_1.Empty();
		m_sSTD3_CommHeader.m_strInformationHeader_7.Empty();	// Unused Must be space filled
		m_sSTD3_CommHeader.m_strTerminalID_15.Empty();

		// Status Field
		m_sSTD3_StatusField.m_strProgramVersionNo_10.Empty();
		m_sSTD3_StatusField.m_strTableVersionNo_10.Empty();
		m_sSTD3_StatusField.m_strFirmwareVersionNo_10.Empty();
		m_sSTD3_StatusField.m_strAlarm_ChestDoorOpen_1.Empty();
		m_sSTD3_StatusField.m_strAlarm_TopDoorOpen_1.Empty();
		m_sSTD3_StatusField.m_strAlarm_SupervisonActive_1.Empty();
		m_sSTD3_StatusField.m_strReceiptPrinter_PaperStatus_1.Empty();
		m_sSTD3_StatusField.m_strReceiptPrinter_RibbonStatus_1.Empty();
		m_sSTD3_StatusField.m_strJournalPrinter_PaperStatus_1.Empty();
		m_sSTD3_StatusField.m_strJournalPrinter_RibbonStatus_1.Empty();
		m_sSTD3_StatusField.m_strNoteStatus_Dispenser_1.Empty();
		m_sSTD3_StatusField.m_strReceiptPrinter_1.Empty();
		m_sSTD3_StatusField.m_strJournalPrinter_1.Empty();
		m_sSTD3_StatusField.m_strDispenser_1.Empty();
		m_sSTD3_StatusField.m_strCommunicationsSystem_1.Empty();
		m_sSTD3_StatusField.m_strCardReader_1.Empty();
		m_sSTD3_StatusField.m_strCardsRetained_3.Empty();
		m_sSTD3_StatusField.m_strElectronicsSystem_2.Empty();
		m_sSTD3_StatusField.m_strCurrentErrorCode_3.Empty();
		m_sSTD3_StatusField.m_strCommunicatioinsFailures_3.Empty();
		m_sSTD3_StatusField.m_strCassetteA_Denomination_3.Empty();
		m_sSTD3_StatusField.m_strCassetteA_NotesLoaded_4.Empty();
		m_sSTD3_StatusField.m_strCassetteA_NotesDispensed_4.Empty();
		m_sSTD3_StatusField.m_strCassetteA_RejectEvents_3.Empty();
		m_sSTD3_StatusField.m_strCassetteB_Denomination_3.Empty();
		m_sSTD3_StatusField.m_strCassetteB_NotesLoaded_4.Empty();
		m_sSTD3_StatusField.m_strCassetteB_NotesDispensed_4.Empty();
		m_sSTD3_StatusField.m_strCassetteB_RejectEvents_3.Empty();
		m_sSTD3_StatusField.m_strCassetteC_Denomination_3.Empty();
		m_sSTD3_StatusField.m_strCassetteC_NotesLoaded_4.Empty();
		m_sSTD3_StatusField.m_strCassetteC_NotesDispensed_4.Empty();
		m_sSTD3_StatusField.m_strCassetteC_RejectEvents_3.Empty();
		m_sSTD3_StatusField.m_strCassetteD_Denomination_3.Empty();
		m_sSTD3_StatusField.m_strCassetteD_NotesLoaded_4.Empty();
		m_sSTD3_StatusField.m_strCassetteD_NotesDispensed_4.Empty();
		m_sSTD3_StatusField.m_strCassetteD_RejectEvents_3.Empty();
		m_sSTD3_StatusField.m_strTotalNotesPurged_3.Empty();

		// Config Request
		m_sSTD3_ConfigReq.m_strTransactionCode_2.Empty();
		m_sSTD3_ConfigReq.m_strStatusMonitoringField_X.Empty();
		m_sSTD3_ConfigReq.m_strMiscellaneousX_V.Empty();

		// Config Response
		m_sSTD3_ConfigResp.m_strInformation_Header_12.Empty();
		m_sSTD3_ConfigResp.m_strTerminalID_15.Empty();
		m_sSTD3_ConfigResp.m_strTransactionCode_2.Empty();
		m_sSTD3_ConfigResp.m_strFieldIDCode1_1.Empty();
		m_sSTD3_ConfigResp.m_strEncryptedPINKey_16.Empty();
		m_sSTD3_ConfigResp.m_strFieldIDCode2_1.Empty();
		m_sSTD3_ConfigResp.m_strEncryptedPINKey2_16.Empty();
		m_sSTD3_ConfigResp.m_strFieldIDCode3_1.Empty();
		m_sSTD3_ConfigResp.m_strSurchargeAmount_8.Empty();
		m_sSTD3_ConfigResp.m_strFieldIDCode4_1.Empty();
		m_sSTD3_ConfigResp.m_strEncryptedPINKey4_16.Empty();
		m_sSTD3_ConfigResp.m_strFieldIDCode5_1.Empty();
		m_sSTD3_ConfigResp.m_strEncryptedPINKey5_16.Empty();

		// Transaction Request
		m_sSTD3_TranReq.m_strTransactionCode_2.Empty();
		m_sSTD3_TranReq.m_strSequenceNumber_4.Empty();
		m_sSTD3_TranReq.m_strTrack2_37.Empty();
		m_sSTD3_TranReq.m_strAmount1_8.Empty();					// Accept.Money
		m_sSTD3_TranReq.m_strAmount2_8.Empty();					// ATMINFO.INI [TRANINFO]
		m_sSTD3_TranReq.m_strPINBlock_16.Empty();				// Accept.PassWord
		m_sSTD3_TranReq.m_strMiscellaneous1.Empty();
		m_sSTD3_TranReq.m_strMiscellaneous2.Empty();
		m_sSTD3_TranReq.m_strStatusMonintoring.Empty();
		m_sSTD3_TranReq.m_strMiscellaneous3.Empty();

		// Transaction Response
		m_sSTD3_TranResp.m_strInformation_Header_12.Empty();
		m_sSTD3_TranResp.m_strMulti_Block_Indicator_1.Empty();
		m_sSTD3_TranResp.m_strTerminalID_15.Empty();
		m_sSTD3_TranResp.m_strTransactionCode_2.Empty();
		m_sSTD3_TranResp.m_strSequenceNumber_4.Empty();
		m_sSTD3_TranResp.m_strResponseCode_3.Empty();
		m_sSTD3_TranResp.m_strAuthorizationNum_8.Empty();
		m_sSTD3_TranResp.m_strTransactionDate_6.Empty();
		m_sSTD3_TranResp.m_strTransactionTime_6.Empty();
		m_sSTD3_TranResp.m_strBusinessDate_6.Empty();
		m_sSTD3_TranResp.m_strAmount1_8.Empty();
		m_sSTD3_TranResp.m_strAmount2_8.Empty();
		m_sSTD3_TranResp.m_strAvailableBalance_8.Empty();		// CITI07-087 KJK_086 2007.04.30
		m_sSTD3_TranResp.m_strAmount1_Ext12.Empty();			// CITI07-087 KJK_087 2007.04.30

		// Reversal Request
		m_sSTD3_ReversalReq.m_strTransactionCode_2.Empty();
		m_sSTD3_ReversalReq.m_strSequenceNumber_4.Empty();
		m_sSTD3_ReversalReq.m_strTrack2_37.Empty();
		m_sSTD3_ReversalReq.m_strAmount1_8.Empty();
		m_sSTD3_ReversalReq.m_strAmount2_8.Empty();
		m_sSTD3_ReversalReq.m_strAmount3_8.Empty();
		m_sSTD3_ReversalReq.m_strStatusMonintoring.Empty();
		m_sSTD3_ReversalReq.m_strMiscellaneousX_V.Empty();

		// Reversal Response
		m_sSTD3_ReversalResp.m_strInformation_Header_12.Empty();
		m_sSTD3_ReversalResp.m_strTerminalID_15.Empty();
		m_sSTD3_ReversalResp.m_strTransactionCode_2.Empty();
		m_sSTD3_ReversalResp.m_strSequenceNumber_4.Empty();
		m_sSTD3_ReversalResp.m_strResponseCode_3.Empty();	

		// Host Total Request
		m_sSTD3_TotalReq.m_strTransactionCode_2.Empty();
		m_sSTD3_TotalReq.m_strStatusMonintoring.Empty();
		m_sSTD3_TotalReq.m_strTotalWithdrawals_4.Empty();
		m_sSTD3_TotalReq.m_strTotalInquiries_4.Empty();
		m_sSTD3_TotalReq.m_strTotalTransfers_4.Empty();
		m_sSTD3_TotalReq.m_strSettlement_12.Empty();
		m_sSTD3_TotalReq.m_strMiscellaneousX_V.Empty();

		// Host Total Response
		m_sSTD3_TotalResp.m_strInformation_Header_12.Empty();
		m_sSTD3_TotalResp.m_strTerminalID_15.Empty();
		m_sSTD3_TotalResp.m_strTransactionCode_2.Empty();
		m_sSTD3_TotalResp.m_strBusinessDate_6.Empty();
		m_sSTD3_TotalResp.m_strNumberOfWithdrawals_4.Empty();
		m_sSTD3_TotalResp.m_strNumberOfInquiries_4.Empty();
		m_sSTD3_TotalResp.m_strNumberOfTransfers_4.Empty();
		m_sSTD3_TotalResp.m_strSettlement_8.Empty();
		m_sSTD3_TotalResp.m_strSettlement_Ext12.Empty();		// CITI07-087 KJK_087 2007.04.30

		// [#2292] US Justin Add Cardtronics TDL Option
		m_STD3_TDL_Data.ClearAllData();
		// End of [#2292]
	}

	if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_CSP200_TYPE) ||
		(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_EPS_TYPE ) )// [#2242] NH JUstin.
	{
		// Common Request Header
		m_sSTD2_CommReqHeader.RecordFormat_1.Empty();			// Record Format (I or 5)
		m_sSTD2_CommReqHeader.ApplicationType_1.Empty();		// Application Type (= '0')
		m_sSTD2_CommReqHeader.MessageDelimiter_1.Empty();		// MessageDelimiter (= '.')
		m_sSTD2_CommReqHeader.BankID_6.Empty();					// Bank ID
		m_sSTD2_CommReqHeader.TerminalID_8.Empty();				// Terminal ID

		// Common Response Header
		m_sSTD2_CommRespHeader.RecordFormat_1.Empty();			// Record Format (I or 5)
		m_sSTD2_CommRespHeader.ApplicationType_1.Empty();		// Application Type (= '0')
		m_sSTD2_CommRespHeader.MessageDelimiter_1.Empty();		// MessageDelimiter (= '.')
		m_sSTD2_CommRespHeader.BankID_6.Empty();				// Bank ID (6)
		m_sSTD2_CommRespHeader.TerminalID_8.Empty();			// Terminal ID

		// Config Request
		m_sSTD2_ConfigReq.RequestType_2.Empty();				// Request Type
		m_sSTD2_ConfigReq.TimeVariantNo_8.Empty();				// Time Variant				*2
		m_sSTD2_ConfigReq.KeyMode_1.Empty();					// Key Mode (0 ~ 11)
		m_sSTD2_ConfigReq.UKSerialNoA_8.Empty();				// Unique Serial Number A
		m_sSTD2_ConfigReq.UKSerialNoB_8.Empty();				// Unique Serial Number B
		m_sSTD2_ConfigReq.SubRequestType_2.Empty();				// [#560] NH KSK 2009.8.20
		m_sSTD2_ConfigReq.MACData_8.Empty();					// MAC Data

		// Config Response
		m_sSTD2_ConfigResp.ResponseType_2.Empty();				// Response Type
		m_sSTD2_ConfigResp.TimeVariantNo_8.Empty();				// Time Variant				*2
		m_sSTD2_ConfigResp.LocalDate_8.Empty();					// Local Date
		m_sSTD2_ConfigResp.LocalTime_6.Empty();					// Local Time
		m_sSTD2_ConfigResp.HealthMsgTimerVal_6.Empty();			// Healthy Message Timer Value
		m_sSTD2_ConfigResp.WorkingKey_V.Empty();				// Working Key
		m_sSTD2_ConfigResp.SurchargeAmount_V.Empty();			// Surcharge Amount	 (ddcc)		** Variable
		m_sSTD2_ConfigResp.BINListReqFlag_1.Empty();			// BIN List Request Flag
		m_sSTD2_ConfigResp.WorkingKey2_V.Empty();				// 2번째 암호화키
		m_sSTD2_ConfigResp.WorkingKey3_V.Empty();				// 3번째 암호화키
		m_sSTD2_ConfigResp.WorkingKey4_V.Empty();				// 4번째 암호화키
		m_sSTD2_ConfigResp.WorkingKey5_V.Empty();				// 5번째 암호화키 [#565] NH KSK 2009.8.20
		m_sSTD2_ConfigResp.MACData_8.Empty();					// MAC Data

		// Transaction Request
		m_sSTD2_TranReq.RequestType_2.Empty();					// Request Type
		m_sSTD2_TranReq.TimeVariantNo_8.Empty();				// Time Variant				*2
		m_sSTD2_TranReq.OperationCode_2.Empty();				// Operation Code
		m_sSTD2_TranReq.SourceAccount_2.Empty();				// Source Account
		m_sSTD2_TranReq.DestinationAccount_2.Empty();			// Destination Account
		m_sSTD2_TranReq.TransactionSequenceNo_4.Empty();		// Transaction Sequence Number
		m_sSTD2_TranReq.Track1Data_79.Empty();					// ISO Track 1 Data
		m_sSTD2_TranReq.Track2Data_40.Empty();					// ISO Track 2 Data
		m_sSTD2_TranReq.Track3Data_106.Empty();					// ISO Track 3 Data
		m_sSTD2_TranReq.PINBuffer_16.Empty();					// PIN Buffer 16
		m_sSTD2_TranReq.TransactionAmount_V.Empty();			// Variable Size	"ddddddcc"
		m_sSTD2_TranReq.SurchargeAmount_V.Empty();				// Surcharge Amount	"ddcc"
		m_sSTD2_TranReq.SurchargeEnableFlag_1.Empty();			// Surcharge Enable Flag
		m_sSTD2_TranReq.AuthrizationNumber_9.Empty();			// Authorization Number
		m_sSTD2_TranReq.PINNumber_6.Empty();					// PIN Number
		m_sSTD2_TranReq.MACData_8.Empty();						// MAC Data					*2

		// Transaction Response
		m_sSTD2_TranResp.ResponseType_2.Empty();				// Response Type
		m_sSTD2_TranResp.TimeVariantNo_8.Empty();				// Time Variant				*2
		m_sSTD2_TranResp.TranSequenceNo_4.Empty();				// Transaction Sequence Number
		m_sSTD2_TranResp.ResponseCode_2.Empty();				// Response Code
		m_sSTD2_TranResp.LocalDate_8.Empty();					// Local Transaction Date
		m_sSTD2_TranResp.LocalTime_6.Empty();					// Local Transaction Time
		m_sSTD2_TranResp.RetrievalRefNo_12.Empty();				// Retrieval Reference Number
		m_sSTD2_TranResp.SystemTraceAuditNo_6.Empty();			// System Trace Audit Number
		m_sSTD2_TranResp.NetworkIDCode_2.Empty();				// Network ID Code
		m_sSTD2_TranResp.SettlementDate_8.Empty();				// Settlement Date
		m_sSTD2_TranResp.AccountBalance_V.Empty();				// Account Balance,	 (ddddddcc)	** Variable
		m_sSTD2_TranResp.AvailableBalance_V.Empty();			// Available Balance,(ddddddcc)	** Variable
		m_sSTD2_TranResp.SurchargeAmount_V.Empty();				// Surcharge Amount	 (ddcc)		** Variable
		m_sSTD2_TranResp.AuthResponseText_V.Empty();			// Authorization Response Text	** Variable
		m_sSTD2_TranResp.ConfigRequestInitiator_2.Empty();		// Configuration Request Initiator
		m_sSTD2_TranResp.MACData_8.Empty();						// MAC Data

		// Reversal Request
		m_sSTD2_ReversalReq.RequestType_2.Empty();				// Request Type
		m_sSTD2_ReversalReq.TimeVariantNo_8.Empty();			// Time Variant				*2
		m_sSTD2_ReversalReq.LocalTranDate_8.Empty();			// Local Transaction Date
		m_sSTD2_ReversalReq.LocalTranTime_6.Empty();			// Local Transaction Time
		m_sSTD2_ReversalReq.RetrievalRefNo_12.Empty();			// Retrieval Reference Number
		m_sSTD2_ReversalReq.RequestedDollarAmt_V.Empty();		// Requested Dollar Amount		Variable
		m_sSTD2_ReversalReq.DispensedDollarAmt_V.Empty();		// Dispensed Dollar Amount		Variable
		m_sSTD2_ReversalReq.SurchargeAmount_4.Empty();			// Surcharge Amount	"ddcc"
		m_sSTD2_ReversalReq.SurchargeEnableFlag_1.Empty();		// Surcharge Enable Flag
		m_sSTD2_ReversalReq.MACData_8.Empty();					// MAC Data					*2

		// Reversal Response
		m_sSTD2_ReversalResp.ResponseType_2.Empty();			// Response Type
		m_sSTD2_ReversalResp.TimeVariant_8.Empty();				// Time Variant				*2
		m_sSTD2_ReversalResp.ConfigRequestInitiator_2.Empty();	// Configuration Request Initiator
		m_sSTD2_ReversalResp.MACData_8.Empty();					// MAC Data

		// Total Request
		m_sSTD2_TotalReq.RequestType_2.Empty();					// Request Type
		m_sSTD2_TotalReq.TimeVariantNo_8.Empty();				// Time Variant				*2
		m_sSTD2_TotalReq.ResetHostTotalFlag_1.Empty();			// Reset Host Total Flag
		m_sSTD2_TotalReq.MACData_8.Empty();						// MAC Data					*2

		// Total Response
		m_sSTD2_TotalResp.ResponseType_2.Empty();				// Response Type = '87'
		m_sSTD2_TotalResp.TimeVariantNo_8.Empty();				// Time Variant				*2
		m_sSTD2_TotalResp.NoOfCashWiths_4.Empty();				// No of Cash Withdrawals
		m_sSTD2_TotalResp.NoOfTransfers_4.Empty();				// No of Transfers
		m_sSTD2_TotalResp.NoOfInquiries_4.Empty();				// No of Inquiries
		m_sSTD2_TotalResp.NoOfNonCashWiths_4.Empty();			// No of None Cash Withdrawals
		m_sSTD2_TotalResp.TotalDispenseAmt_V.Empty();			// Total Dispensed Amount
		m_sSTD2_TotalResp.TotalNonCashDispAmt_V.Empty();		// Total Non-Cash Dispensed Amount
		m_sSTD2_TotalResp.TotalSurchargeAmt_V.Empty();			// Total Surcharge Amount
		m_sSTD2_TotalResp.ConfigRequestInitiator_2.Empty();		// Configuration Initiator
		m_sSTD2_TotalResp.MACData_8.Empty();					// MAC Data					*2

		// Health Check Request
		m_sSTD2_HealthReq.RequestType_2.Empty();				// Request Type
		m_sSTD2_HealthReq.TimeVariantNo_8.Empty();				// Time Variant				*2
		m_sSTD2_HealthReq.LocalDate_8.Empty();					// Local Transaction Date
		m_sSTD2_HealthReq.LocalTime_6.Empty();					// Local Transaction Time
		m_sSTD2_HealthReq.TerminalStatus_4.Empty();				// Terminal Status
		m_sSTD2_HealthReq.MACData_8.Empty();					// MAC Data					*2

		// Health Check Response
		m_sSTD2_HealthResp.ResponseType_2.Empty();				// Response Type = '90'
		m_sSTD2_HealthResp.TimeVariantNo_8.Empty();				// Time Variant				*2
		m_sSTD2_HealthResp.ConfigRequestInitiator_2.Empty();	// Configuration Request Initiator
		m_sSTD2_HealthResp.MACData_8.Empty();					// MAC Data					*2

		// Detail Health Check Request
		m_sSTD2_DetailHealthReq.RequestType_2.Empty();			// Request Type
		m_sSTD2_DetailHealthReq.TimeVariantNo_8.Empty();		// Time Variant				*2
		m_sSTD2_DetailHealthReq.LocalDate_8.Empty();			// Local Transaction Date
		m_sSTD2_DetailHealthReq.LocalTime_6.Empty();			// Local Transaction Time
		m_sSTD2_DetailHealthReq.TerminalStatus_16.Empty();		// Terminal Status
		m_sSTD2_DetailHealthReq.ApplicationVersion_7.Empty();	// Application Version #
		m_sSTD2_DetailHealthReq.ApplicationCRC_4.Empty();		// Application CRC #
		m_sSTD2_DetailHealthReq.MACData_8.Empty();				// MAC Data					*2

		// Detail Health Check Response
		m_sSTD2_DetailHealthResp.ResponseType_2.Empty();			// Response Type = '90'
		m_sSTD2_DetailHealthResp.TimeVariantNo_8.Empty();			// Time Variant				*2
		m_sSTD2_DetailHealthResp.ConfigRequestInitiator_2.Empty();	// Configuration Request Initiator
		m_sSTD2_DetailHealthResp.MACData_8.Empty();					// MAC Data					*2
	}

	// [#2185] 2013.05.08 Dual Host -- Initialize DCC option at any cases
	{
		// [#2227] US Justin 2013.10.30 Dual Host DCC - Parameter Initialize.
		m_sSTD1_DynamicFlowReq.TransactionType.Empty();
		m_sSTD1_DynamicFlowReq.RandomMac_8.Empty();						// 2015.09.98
		m_sSTD1_DynamicFlowReq.R1_TerminalCurrency.Empty();	// at
		m_sSTD1_DynamicFlowReq.R1_Track2Data.Empty();			// cb
		m_sSTD1_DynamicFlowReq.R1_AID.Empty();					// cd	// [#2182] NH Justin 2013.03.21 Implement NHD1.6
		m_sSTD1_DynamicFlowReq.R2_Surcharge_Withdrawal.Empty();	// fc
		m_sSTD1_DynamicFlowReq.R2_TransactionAmount.Empty();	// ta
		m_sSTD1_DynamicFlowReq.R2_CurrencyCode.Empty();		// tc

		m_sSTD1_DynamicFlowResp.RandomMac_8.Empty();					// 2015.09.08
		m_sSTD1_DynamicFlowResp.PartnerID.Empty();						// i
		m_sSTD1_DynamicFlowResp.R1_TransactionCapacity.Empty();			// ac
		m_sSTD1_DynamicFlowResp.R1_Surcharge_Withdrawal.Empty();		// fc
		m_sSTD1_DynamicFlowResp.R1_Surcharge_BalanceInquiry.Empty();	// fi
		m_sSTD1_DynamicFlowResp.R1_Surcharge_PinChange.Empty();			// fp
		m_sSTD1_DynamicFlowResp.R1_Surcharge_MPT.Empty();				// ft
		m_sSTD1_DynamicFlowResp.R1_ConnectionResult.Empty();
		m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRate.Empty();			// er		
		m_sSTD1_DynamicFlowResp.R1_DCC_BaseTargetCurrency.Empty();		// ec
		m_sSTD1_DynamicFlowResp.R1_Surcharge_DCC.Empty();				// fd
		// End of [#2227]
		m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRateMarkUp.Empty();		// em	[#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1 message

		m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Empty();		// ec	
		m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate.Empty();			// er
		m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount.Empty();		// et
		m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge.Empty();		// ef	
		m_sSTD1_DynamicFlowResp.R2_Surcharge_Withdrawal.Empty();		// fc	
		m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC.Empty();				// fd	
		m_sSTD1_DynamicFlowResp.R2_Surcharge_MPT.Empty();				// ft	
		m_sSTD1_DynamicFlowResp.R2_ConnectionResult.Empty();
		m_sSTD1_DynamicFlowResp.nService_PINChange = 0;
		m_sSTD1_DynamicFlowResp.nService_DCC = 0;
		m_sSTD1_DynamicFlowResp.nService_MPT = 0;
		m_sSTD1_DynamicFlowResp.nDCCTransaction = 0;
		m_sSTD1_DynamicFlowResp.R2_DisplayedExchangeRate.Empty();		// [#2292] US Justin 2014.10.06 
		m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRateMarkUp.Empty();		// em	[#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1 message
	}
	// End of [#2185]

	// [#2185] US Justin 2013.05.14 Dual Host DCC
	//#if (APP_CUSTOM_PAI)		// [#2499] US Justin 2017.08.21 Enable Dual Host DCC for all customers.
	if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_DUALHOSTTAN) == ON)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_DUALHOSTTAN, OFF);
	//#endif
	// End of [#2185]

	// [#RWC6-59] US William 2019.10.08 LibertyX
#if(APP_LIBERTYX)
	if (m_LXConfigManager == NULL)
	{
		m_LXConfigManager = new CLibertyXConfigurationManager();
	}

	CString oldUrl = m_LXConfig.APIUrl;
	if (m_LXConfigManager->GetConfiguration(m_LXConfig))
	{
		m_LXIsConfigured = true;

		// This service takes a bit to initialize, so only do it if necessary.
		// New http service if one doesn't exist, or if the url changed
		if (m_LXHttp == NULL)
		{
			m_LXAuth = new CHTTPBasicAuthenticator(m_LXConfig.Username, m_LXConfig.Password);
			m_LXHttp = new HTTP(m_LXConfig.APIUrl, m_LXAuth);
		}
		else if (oldUrl != m_LXConfig.APIUrl)
		{
			NHDBG((L"LTX URL has changed\r\n"));

			delete m_LXHttp;
			delete m_LXAuth;

			m_LXAuth = new CHTTPBasicAuthenticator(m_LXConfig.Username, m_LXConfig.Password);
			m_LXHttp = new HTTP(m_LXConfig.APIUrl, m_LXAuth);
		}

		if (m_LXService != NULL)
		{
			delete m_LXService;
			m_LXService = NULL;
		}

		m_LXService = new CLibertyXService(m_LXConfig, m_LXHttp);
	}

#endif
	// End of [#RWC6-59]

#if (APP_DIGITALMINT)
	{
		if (m_DMConfigManager == NULL)
		{
			m_DMConfigManager = new CDigitalMintConfigurationManager();
		}

		UpdateDigitalMintServices(m_DMConfigManager->APIUrl(m_DMConfig));
	}
#endif

#if (APP_CUSTOM_PAI)
	{
		if (m_PAIConfigManager == NULL)
		{
			m_PAIConfigManager = new CPAIConfigurationManager();
		}

		if (m_PAIConfigManager->GetConfiguration(m_PAIConfig))
		{
			m_PAIIsConfigured = true;
		}
#endif

#if (APP_CUSTOM_CASHDEPOT)
	{
		UpdateCashDepotServices(MemGetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_HOSTNAME));
	}
#endif

	// [RWC6-676] Start SKKim 2024.03.28
#if (APP_TANGOPAY)
	{
		Load_TangoPayDefaultTransData();
	}
#endif

	return T_OK;
}

#if (APP_DIGITALMINT)
void CTranCmn::UpdateDigitalMintServices(CString apiUrl)
{
	CString oldUrl;

	if (m_DMHttp != NULL)
	{
		oldUrl = m_DMHttp->GetHostname();
	}

	if (m_DMConfigManager->GetConfiguration(m_DMConfig))
	{
		m_DMIsConfigured = true;

		if (apiUrl.IsEmpty())
		{
			apiUrl = m_DMConfigManager->APIUrl(m_DMConfig);
		}

		// This service takes a bit to initialize, so only do it if necessary.
		// New http service if one doesn't exist, or if the url changed
		if (m_DMHttp == NULL && apiUrl != L"")
		{
			m_DMHttp = new HTTP(apiUrl);
		}
		else if (oldUrl != apiUrl)
		{
			NHDBG((L"DM URL has changed\r\n"));

			delete m_DMHttp;
			m_DMHttp = new HTTP(apiUrl);
		}

		if (m_DMService != NULL)
		{
			delete m_DMService;
			m_DMService = NULL;
		}

		m_DMService = new CDigitalMintService(m_DMConfig, *m_DMConfigManager, m_DMHttp);
	}
}
#endif

#if (APP_CUSTOM_CASHDEPOT)
void CTranCmn::UpdateCashDepotServices(CString apiUrl)
{
	CString oldUrl;

	if (m_CDHttp != NULL)
	{
		oldUrl = m_CDHttp->GetHostname();
	}

	// New http service if one doesn't exist
	// This service takes a bit to initialize, so only do it if necessary.
	if (m_CDHttp == NULL && apiUrl != L"")
	{
		m_CDHttp = new HTTP(apiUrl);
	}
	else if (oldUrl != apiUrl)
	{
		NHDBG((L"CD URL has changed\r\n"));

		delete m_CDHttp;
		m_CDHttp = new HTTP(apiUrl);
	}

	if (m_CDService != NULL)
	{
		delete m_CDService;
		m_CDService = NULL;
	}

	if (m_CDHttp != NULL)
	{
		m_CDService = new CCashDepotService(m_CDHttp);
	}
}
#endif


// [RWC6-676] Start SKKim 2024.03.28
#if (APP_TANGOPAY)
void CTranCmn::UpdateTangoPayServices(CString apiURL)
{
	if (m_pTangoPayHttp != NULL)
	{
		delete m_pTangoPayHttp;
		m_pTangoPayHttp = NULL;
	}

	if (m_pTangoPayHttp == NULL)
	{
		m_pTangoPayHttp = new HTTP(apiURL);
	}

	if (m_pTangoPayService != NULL)
	{
		delete m_pTangoPayService;
		m_pTangoPayService = NULL;
	}

	if (m_pTangoPayService == NULL)
	{
		m_pTangoPayService = new TangoPayService(m_pTangoPayHttp);
	}
}
#endif
// [RWC6-676] End SKKim 2024.03.28

// Initialize Serial Transaction DS
int CTranCmn::fnAPP_SetEJournalData(int nInitial)
{
	NHDEBUG(1, (_T("***TranCmn***CTranCmn::fnAPP_SetEJournalData() \n")));

	CString strTemp = "";

	if (nInitial)
	{
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE				, L"--");			// 2
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSFROMACCOUNT		, L"--");			// 2
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTOACCOUNT		, L"--");			// 2

		// [#292] [MX] KSK 2008.6.12
		// Card 관련 Data는 Clear하지 않는다.
#if (US_VERSION || MX_VERSION)	// [#2115] MX KSK 2012.02.05
		if (m_PreBalanceStatus == PRE_BAL_NONE)
#endif
		{
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKCODE		, L"0000");			// 4
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKACCOUNT	, L"000000000000");	// 12
		}

		// end of [#292]
		// AIREAT_2007-10-11오전 8:36:14
		// DATE & TIME Default Value 00000000000 -> 2000.01.01 00:00:01
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE			, L"01012000");		// 8 // 2000.01.01 - change default value
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME			, L"000001");		// 6 // 00:00:01 - change default value
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSAVAILBALANCE		, L"        ");		// 2008-01-21 V01.02.25 초기값 변경(POS)
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRETRIEVALNUM		, L"000000000000");	// 12
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM	, L"      ");		// 6
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID		, L"  ");			// 2
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSSETTLEDATE		, L"01012000");		// 8 // 2000.01.01 - change default value
		// end of AIREAT_2007-10-11오전 8:36:14
		MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT			, L"00000000");		// 8 [#169] [NH] 2008.04.25
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT		, L"00000000");		// 8
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT		, L"00000000");		// 8
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT		, L"            ");		// 12  2008-01-21 V01.02.25 초기값 변경(POS)
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT		, L"00");			// 2
		MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT			, FALSE);			// WORD
		// [#292] [MX] KSK 2008.6.12
		// Card 관련 Data는 Clear하지 않는다.
#if (US_VERSION || MX_VERSION)	// [#2115] MX KSK 2012.02.05
		if (m_PreBalanceStatus == PRE_BAL_NONE)
#endif
		{
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA		, L"0000000000000000"); // 16
		}	// end of [#292]

		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSNONCASHVALUE		, L"0000");			// 4
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSNONCASHTYPE		, L"000000000000");	// 12
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG			, L"");	// [#86] KSK 2008.04.10
// 		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CALC_REQ_MXDOLLAR		, L"00000000");	// [#519] [NH] KSK 2009.3.10 // [#2137] MX PCS 2012.07.17
// 		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CALC_DISP_MXDOLLAR	, L"00000000");	// [#519] [NH] KSK 2009.3.10 // [#2137] MX PCS 2012.07.17
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_DISP_COUNT		, L"");	// KSK 2009.3.30
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_REJ_COUNT		, L"");	// KSK 2009.3.30

		// [#292] [MX] KSK 2008.6.12
		// Card 관련 Data는 Clear하지 않는다.
#if (US_VERSION || MX_VERSION)	// [#2115] MX KSK 2012.02.05
		if (m_PreBalanceStatus == PRE_BAL_NONE)
#endif
		{
			// KSK 2012.03.01 Code Sonar 대책
//			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK1			, GetSprintf("%79.79S", L" "));	// 79
//			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2			, GetSprintf("%40.40S", L" "));	// 40
//			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK3			, GetSprintf("%106.106S", L" "));// 106
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK1			, L"");
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2			, L"");
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK3			, L"");
			// end of KSK 2012.03.01
			MemSetStr(_MEM_FLD_EMVINFO,		 _MEM_VAR_EMV_APPLICATION_ID		, L"");	// [#2502] US Justin 2017.08.24 Bug Fix - Clearing Communicatio Error after Pre-Dual Balance
			MemSetStr(_MEM_FLD_EMVREVERSAL,  _MEM_VAR_EMV_APPLICATION_NAME		, L"");	// [#2502] US Justin
		}

		MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG		,MX_NOT_SEND);	// [#2137] MX PCS 2012.07.23

		// end of [#292]
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL	, 0);	// [#12] NH PSC 2008.03.24 host reson for reversal 값 초기기화(standard#3 만 사용)
		MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_APP_ISSUERFEE		, L"000000000");	// [#66] NH PSC 2008.04.07 'k' field 처리

		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_TRANCODE	, TC_NULL);		// [#343] [MX] KSK 2008.6.26

		MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEDISPLAY		, 0);	// [#390] [NH] KSK 2008.8.1

		// [#397] [NH] PSC 2008.08.12 EMV reversal data clear, ISR과 IS Id는 다음거래 전문전송시까지 유지해야 하므로 Clear하지 않음.
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_CRYPTOGRAM_INFO_DATA_TLV,	L"");	// cryptogram information data
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TERMINAL_VERIFY_RESULT_TLV,L"");	// Terminal Verification Result
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_CRYPTOGRAM_TLV,		L"");	// application cryptogram
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AMOUNT_AUTHORIZED_TLV,		L"");	// amount authorized
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AMOUNT_OTHER_TLV,			L"");	// amount other
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APPLICATION_ID_TLV,		L"");	// application ID(AID)
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_INTERCHANGE_PRO_TLV,	L"");	// Application Interchange Profile
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_TRAN_COUNTER_TLV,		L"");	// Application Transaction Counter
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TERMINAL_COUNTRY_CODE_TLV,	L"");	// terminal country code
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRANSACTION_CURRENCY_CODE_TLV,	L"");	// transaction currency code
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRANSACTION_DATE_TLV,		L"");	// transaction date
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRANSACTION_TYPE_TLV,		L"");	// Transaction Type
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_RANDOM_NUMBER_TLV,			L"");	// unpredictable number
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_APP_DATA_TLV,		L"");	// Issuer Application Data
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_IFD_SERIAL_NUMBER_TLV,		L"");	// interface device(IFD) serial number
		// [#559] [CA] KSK 2009.8.19
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_SCRIPT_ID_TLV,		L"");	// issuer script ID
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_SCRIPT_RESULTS_TLV,	L"");	// issuer script results
		// end of [#559]
		// KSK 2009.8.31
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_EXPIRED_DATE_TLV,				L"");
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_ACCOUNT_NUMBER_TLV,			L"");
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_ACCOUNT_NUMBER_SEQ_NUMBER_TLV, L"");
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_POS_ENTRY_MODE_TLV,				L"");
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TERMINAL_CAPABILITIES_TLV,			L"");
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TERMINAL_TYPE_TLV,					L"");
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRACK2_EQU_DATA_TLV,				L"");
		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRANSACTION_SEQ_COUNTER_TLV,		L"");
		// end of KSK 2009.8.31

		MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AC_REQ_9F26_VALUE,					L"");	// [#2115] MX KSK 2012.01.28

		// [#2015] MX KSK 2012.02.02
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_BANK_NAME,					L"");
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE,					L"");
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE,				L"");
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX,						L"");
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_RETURNED_ACCNT_NO,			L"");
		
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE,					L"");				// [#2137] MX PCS 2012.07.02
// 		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_TOTAL_AMOUNT,		L"");				// [#2137] MX PCS 2012.07.02
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TOTAL_FEE,					L"");				// [#2142] MX KMK 2012.08.16
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE,				L"");				// [#2137] MX PCS 2012.07.23
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_REVERSAL_EXCHANGE_FEE,		L"");				// [###1] MX KSK 2012.08.18
		MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE			,MX_NOT_SET);				// [#2137] MX PCS 2012.07.23
		MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG	,MX_NOT_SEND);				// [#2137] MX PCS 2012.07.23

		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE, WITHDRAWAL_ATM);			// [#2350] US Justin 2015.06.19
		
		m_pDevCmn->fnEMV_Trans_Initial();			// [#2188-1]
		// end of [#2015]
	}

	return T_OK;
}

// [#2461] NH Justin 2016.12.16 Partial Update
BOOL CTranCmn::SetAdditionalSetting()
{
	m_OfferAcceptance.ReadAcceptanceHistory();	// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
	// [#2405] US Justin 2016.03.21 HalCash Online
	#if (APP_PIN4_CASHPICKUP)	// (APP_CUSTOM_PAI) [#2471] US Justin 2017.02.01 Enable Pin4 to all customers
		m_Pin4.ReadSettingAndDispenseHistory();

		// [#2521] NH Justin 2017.12.11 MoniMobile : Add value added function enable status
		if (m_Pin4.m_bPin4Available != TRUE)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE, 0);
		// End of [#2521]
	#endif

	#if (APP_POPMONEY)			// (APP_CUSTOM_PAI) [#2471] US Justin 2017.02.01 Enable Popmoney to all customers
		m_PopMoneyData.ReadDispenseHistory();
	#endif
	// End of [#2405]

	// [#2445] US Justin 2016.09.28
	#if (APP_JUST_CASH)
		m_JustCashData.ReadSettingAndDispenseHistory();

		// [#2521] NH Justin 2017.12.11 MoniMobile : Add value added function enable status
		if (m_JustCashData.m_bJustCashAvailable != TRUE)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE, 0);
		// End of [#2521]
	#endif
	// End of [#2445]

	// [#2446] US Justin 29 Paypal CCA
	#if(APP_PAYDIANT_CCA)
		m_PayPalCCA.ReadSettingAndDispenseHistory();
	#endif
	// End of [#2446]

	// [#2513] US Justin 2017.11.03 GivePay Giftcard Purchase
	#if (APP_GPAY_GIFTCARD_PURCHASE)
		m_GivePayData.ReadSettingAndDispenseHistory();
	#endif
	// End of [#2513]

	// Check Update XML FILE
	CString strPath;
	strPath.Format(L"%s\\%s", ATM_DATA_PATH, ADDITIONAL_UPDATE_FILE);
	if(!IsExistFile(strPath))
	{
		NHDEBUG(DBG_INFO, (L"UPDATE XML FILE (%s) DOES NOT EXIST\n", strPath));
		return TRUE;
	}

	// Read Additional Setting
	CFile cf;
	if( cf.Open( strPath, CFile::modeRead | CFile::typeText) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		DeleteFile(strPath);
		return FALSE;
	}
	int nFileLen = (int)cf.GetLength();
	unsigned char* pBuffer = new unsigned char[nFileLen + 2];				// Allocate buffer for binary file data
	if( pBuffer == NULL )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO CREATE NEW BUFFER(%d)]\n", nFileLen + 2));
		cf.Close();
		DeleteFile(strPath);
		return FALSE;
	}
	nFileLen = cf.Read( pBuffer, nFileLen );
	cf.Close();

	// Delete file to avoid multiple upgrades...
	DeleteFile(strPath);	

	// Assign to XML String Buffer
	pBuffer[nFileLen] = '\0';
	pBuffer[nFileLen+1] = '\0';												// in case 2-byte encoded
	CString strXML(_T(""));
	if ( pBuffer[0] == 0xFF && pBuffer[1] == 0xFE )							// Windows Unicode file is detected if starts with FEFF	
	{
		strXML = (LPCWSTR)(&pBuffer[2]);
		NHDEBUG(DBG_CALL, (L"[File starts with hex FFFE, assumed to be wide char format.]\n"));
	}
	else
		strXML = (LPCSTR)pBuffer;
	delete [] pBuffer;
	if ( strXML.GetLength() < nFileLen / 2 - 20 )							// If it is too short, assume it got truncated due to non-text content	
	{
		NHDEBUG(DBG_INFO, (L"[Error converting file to string (may contain binary data)]\n"));
		return FALSE;
	}
	NHDEBUG(DBG_INFO, (L"XML DATA =\n%s\n", strXML ));

	// Read XML
	CMarkup xml;
	if( xml.SetDoc( strXML ) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO PARSE XML]\n]\n"));
		return FALSE;
	}
	while( xml.FindChildElem() )
	{
		CString strConfigApp = xml.GetChildTagName();
		if( strConfigApp.GetLength()>0)
		{
			NHDEBUG(DBG_INFO, (L" Config App = [%s]\n", strConfigApp ));
			if( xml.IntoElem() )
			{
				BOOL bUpdateParameter = FALSE;
				while( xml.FindChildElem() )
				{
					CString strName = xml.GetChildTagName();
					if( strName.IsEmpty() == TRUE )
						continue;

					CString strChildData = xml.GetChildData();
					NHDEBUG(DBG_INFO, (L" XML ChildNode [%s]/[%s] = [%s]\n", strConfigApp, strName, strChildData ));

					if(strConfigApp == L"ATMSETUP")
					{
						;	// Add ATM Partial Update
					}

					#if (APP_PIN4_CASHPICKUP)	// (APP_CUSTOM_PAI) [#2471] US Justin 2017.02.01 Enable Pin4 to all customers
					else if( (strConfigApp == L"HalCash_Pin4") && (m_Pin4.m_bPin4Available == TRUE) )
					{
						bUpdateParameter = TRUE;
						if(		 strName.CompareNoCase(L"HostIP") == 0 )				m_Pin4.m_strHostIP = strChildData;
						else if( strName.CompareNoCase(L"HostPort") == 0 )				m_Pin4.m_strHostPort = strChildData;
						else if( strName.CompareNoCase(L"MaxDispenseLimit") == 0 )		m_Pin4.m_strMaxDispenseLimit = strChildData;
					}
					#endif
				}

				if( xml.OutOfElem() == FALSE )
				{
					NHDEBUG(DBG_INFO, (L"[FAIL OUT OF <%s>] => RETURN FALSE \n", strConfigApp));
					return FALSE;
				}

				// Save Updated Setting
				if(bUpdateParameter)
				{
					BOOL bSaveResult = FALSE;

					if(strConfigApp == L"ATMSETUP")
					{
						;	// Add ATM Partial Update
					}

					#if (APP_PIN4_CASHPICKUP)	// (APP_CUSTOM_PAI) [#2471] US Justin 2017.02.01 Enable Pin4 to all customers
					else if( (strConfigApp == L"HalCash_Pin4") && (m_Pin4.m_bPin4Available == TRUE) )
						bSaveResult = m_Pin4.SavePin4Setting();
					#endif

					NHDEBUG(DBG_INFO, (L"UPDATE PARAMETER FOR [%] = [%d]\n", strConfigApp, bSaveResult));
				}
			}
			else
				NHDEBUG(DBG_INFO, (L"[FAIL TO INTO <%s>] => CONTINE to NEXT ELEMENT \n", strConfigApp));
		}
	}
	return TRUE;
}
// End of [#2461]

// [#68] UK HWANG 2008.05.15 EMV Level , inline 함수 -> 일반함수로 변경.
BOOL CTranCmn::DidMediaExist()		// [#397] [NH] psc 2008.08.01 EMV(IC), MS 거래 함수 공통화
{
	if(!m_pDevCmn->fnMCU_IsEmvEnable())	return FALSE;			// MS mode
	else								return m_bMcuExisted;	// IC mode
}
// [#546] SOOK 2009.08.05 다기능 BIN LIST 기능 추가함 
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: GetMultiFunctionalSurchargeAmount()
 RETURN TYPE  : Surcharge 금액 
 PARAMETER    : TranCode
 DESCRIPTION  : USER Define Surcharge가 있는지 검사해서 BIN에 맞는 Surchage를 송신하도록 함 
-------------------------------------------------------------------*/
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
int CTranCmn::GetMultiFunctionalSurchargeAmount(int TranCode)
{
	int nSurchargeAmt = 0;

	DWORD dwValue = 0;

	//Surcharge MOde Disable을 경우에는 BIN LIst에 Propertiesr 가 있어도 무시함 
	if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE) == 0)
	{
		return 0;
	}
	else if ( m_pDevCmn->m_BINMgr.GetTransactionBinProperties(ACTION_BLOCK_SURCHARGE, &dwValue) == TRUE)
	{
		nSurchargeAmt = 0;
	}
	else if ( TranCode == TC_WITHDRAWAL 
		&& m_pDevCmn->m_BINMgr.GetTransactionBinProperties(ACTION_WITHDRAWAL_SURCHARGE, &dwValue) == TRUE)
	{
		nSurchargeAmt = dwValue;
	}
#if(AU_VERSION)
	else if ( TranCode == TC_INQUIRY 
		&& m_pDevCmn->m_BINMgr.GetTransactionBinProperties(ACTION_BALANCE_SURCHARGE, &dwValue) == TRUE)
	{
		nSurchargeAmt = dwValue;
	}
#endif
	else if ( MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE)== 1 )
	{
		if ( TranCode == TC_WITHDRAWAL )
		{
			nSurchargeAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT));
		}
#if(AU_VERSION)
		else if (TranCode == TC_INQUIRY )
		{
			nSurchargeAmt = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_INQUIRY_SURCHARGE);
		}
#endif
	}
	return nSurchargeAmt;
}
#endif
//end of [#546]
