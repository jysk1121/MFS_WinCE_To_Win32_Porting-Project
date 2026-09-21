#include "StdAfx.h"
#include ".\Dev\JnlMgr.h"
#include ".\Dev\Devcmn.h"
#include "..\H\Tran\TranCmnDefine.h"	// [#387] [NH] KSK 2008.7.28

typedef struct _CE_JNL_KINDCODE_DESC_
{
	WCHAR	szKindCode[3];
	WCHAR	szDescription[64];	// [#2160] CA KMK 2012.11.14 French OP, 배열 구조 재조정
} CE_JNL_KINDCODE_DESC;

CE_JNL_KINDCODE_DESC g_CESupportEJKindCode[] =
{
	// [#2160] CA KMK 2012.11.14 French OP, 다국어 지원을 위한 배열 재구성
	{ADDCASH_CST1,						T_ADDCASH_CST1},		/* ADD CASH #1 */
	{ADDCASH_CST2,						T_ADDCASH_CST2},		/* ADD CASH #2 */
	{ADDCASH_CST1_2,					T_ADDCASH_CST1_2},		/* ADD CASH #1,2 */
	{ADDCASH_CST3,						T_ADDCASH_CST3},		/* ADD CASH #3 */
	{ADDCASH_CST1_3,					T_ADDCASH_CST1_3},		/* ADD CASH #1,3 */
	{ADDCASH_CST2_3,					T_ADDCASH_CST2_3},		/* ADD CASH #2,3 */
	{ADDCASH_CST1_2_3,					T_ADDCASH_CST1_2_3},	/* ADD CASH #1,2,3 */
	{ADDCASH_CST4,						T_ADDCASH_CST4},		/* ADD CASH #4 */
	{ADDCASH_CST1_4,					T_ADDCASH_CST1_4},		/* ADD CASH #1,4 */
	{ADDCASH_CST2_4,					T_ADDCASH_CST2_4},		/* ADD CASH #2,4 */
	{ADDCASH_CST1_2_4,					T_ADDCASH_CST1_2_4},	/* ADD CASH #1,2,4 */
	{ADDCASH_CST3_4,					T_ADDCASH_CST3_4},		/* ADD CASH #3,4 */
	{ADDCASH_CST1_3_4,					T_ADDCASH_CST1_3_4},	/* ADD CASH #1,3,4 */
	{ADDCASH_CST2_3_4,					T_ADDCASH_CST2_3_4},	/* ADD CASH #2,3,4 */
	{ADDCASH_CST1_2_3_4,				T_ADDCASH_CST1_2_3_4},	/* ADD CASH #1,2,3,4 */

	{REMOTE_ADDCASH_CST1,				T_REMOTE_ADDCASH_CST1},			/* REMOTE ADD CASH #1 */
	{REMOTE_ADDCASH_CST2,				T_REMOTE_ADDCASH_CST2},			/* REMOTE ADD CASH #2 */
	{REMOTE_ADDCASH_CST1_2,				T_REMOTE_ADDCASH_CST1_2},		/* REMOTE ADD CASH #1,2 */
	{REMOTE_ADDCASH_CST3,				T_REMOTE_ADDCASH_CST3},			/* REMOTE ADD CASH #3 */
	{REMOTE_ADDCASH_CST1_3,				T_REMOTE_ADDCASH_CST1_3},		/* REMOTE ADD CASH #1,3 */
	{REMOTE_ADDCASH_CST2_3,				T_REMOTE_ADDCASH_CST2_3},		/* REMOTE ADD CASH #2,3 */
	{REMOTE_ADDCASH_CST1_2_3,			T_REMOTE_ADDCASH_CST1_2_3},		/* REMOTE ADD CASH #1,2,3 */
	{REMOTE_ADDCASH_CST4,				T_REMOTE_ADDCASH_CST4},			/* REMOTE ADD CASH #4 */
	{REMOTE_ADDCASH_CST1_4,				T_REMOTE_ADDCASH_CST1_4},		/* REMOTE ADD CASH #1,4 */
	{REMOTE_ADDCASH_CST2_4,				T_REMOTE_ADDCASH_CST2_4},		/* REMOTE ADD CASH #2,4 */
	{REMOTE_ADDCASH_CST1_2_4,			T_REMOTE_ADDCASH_CST1_2_4},		/* REMOTE ADD CASH #1,2,4 */
	{REMOTE_ADDCASH_CST3_4,				T_REMOTE_ADDCASH_CST3_4},		/* REMOTE ADD CASH #3,4 */
	{REMOTE_ADDCASH_CST1_3_4,			T_REMOTE_ADDCASH_CST1_3_4},		/* REMOTE ADD CASH #1,3,4 */
	{REMOTE_ADDCASH_CST2_3_4,			T_REMOTE_ADDCASH_CST2_3_4},		/* REMOTE ADD CASH #2,3,4 */
	{REMOTE_ADDCASH_CST1_2_3_4,			T_REMOTE_ADDCASH_CST1_2_3_4},	/* REMOTE ADD CASH #1,2,3,4 */

	{DAY_TOTAL,							T_JNL_DAY_TOTAL},				/* DAY TOTAL */
	{REMOTE_DAY_TOTAL,					T_JNL_REMOTE_DAY_TOTAL},		/* REMOTE DAY TOTAL */
	{CST_TOTAL,							T_JNL_CST_TOTAL},				/* CASSETTE TOTAL */
	{REMOTE_CST_TOTAL,					T_JNL_REMOTE_CST_TOTAL},		/* REMOTE CASSETTE TOTAL */
	{TRIALDAY_TOTAL,					T_JNL_TRIALDAY_TOTAL},			/* TRIAL DAY TOTAL */
	{REMOTE_TRIALDAY_TOTAL,				T_JNL_REMOTE_TRIALDAY_TOTAL},	/* REMOTE TRIAL DAY TOTAL */
	{TRIALCST_TOTAL,					T_JNL_TRIALCST_TOTAL},			/* TRIAL CASSETTE TOTAL */
	
	// [#GLDV-3005] US Kook 2022.04.14 support side car
	{CASHIN_TOTAL,						T_JNL_CASHIN_TOTAL},			/* CASHIN TOTAL */
	{TRIALCASHIN_TOTAL,					T_JNL_TRIALCASHIN_TOTAL},		/* TRIAL CASHIN TOTAL */
	// end of [#GLDV-3005]

	{CHANGE_DENOM_CST1,					T_CHANGE_DENOM_CST1},		/* CHANGE DENOMINATION OF CASSETTE #1 */
	{CHANGE_DENOM_CST2,					T_CHANGE_DENOM_CST2},		/* CHANGE DENOMINATION OF CASSETTE #2 */
	{CHANGE_DENOM_CST1_2,				T_CHANGE_DENOM_CST1_2},		/* CHANGE DENOMINATION OF CASSETTE #1,2 */
	{CHANGE_DENOM_CST3,					T_CHANGE_DENOM_CST3},		/* CHANGE DENOMINATION OF CASSETTE #3 */
	{CHANGE_DENOM_CST1_3,				T_CHANGE_DENOM_CST1_3},		/* CHANGE DENOMINATION OF CASSETTE #1,3 */
	{CHANGE_DENOM_CST2_3,				T_CHANGE_DENOM_CST2_3},		/* CHANGE DENOMINATION OF CASSETTE #2,3 */
	{CHANGE_DENOM_CST1_2_3,				T_CHANGE_DENOM_CST1_2_3},	/* CHANGE DENOMINATION OF CASSETTE #1,2,3 */
	{CHANGE_DENOM_CST4,					T_CHANGE_DENOM_CST4},		/* CHANGE DENOMINATION OF CASSETTE #4 */
	{CHANGE_DENOM_CST1_4,				T_CHANGE_DENOM_CST1_4},		/* CHANGE DENOMINATION OF CASSETTE #1,4 */
	{CHANGE_DENOM_CST2_4,				T_CHANGE_DENOM_CST2_4},		/* CHANGE DENOMINATION OF CASSETTE #2,4 */
	{CHANGE_DENOM_CST1_2_4,				T_CHANGE_DENOM_CST1_2_4},	/* CHANGE DENOMINATION OF CASSETTE #1,2,4 */
	{CHANGE_DENOM_CST3_4,				T_CHANGE_DENOM_CST3_4},		/* CHANGE DENOMINATION OF CASSETTE #3,4 */
	{CHANGE_DENOM_CST1_3_4,				T_CHANGE_DENOM_CST1_3_4},	/* CHANGE DENOMINATION OF CASSETTE #1,3,4 */
	{CHANGE_DENOM_CST2_3_4,				T_CHANGE_DENOM_CST2_3_4},	/* CHANGE DENOMINATION OF CASSETTE #1,3,4 */
	{CHANGE_DENOM_CST1_2_3_4,			T_CHANGE_DENOM_CST1_2_3_4},	/* CHANGE DENOMINATION OF CASSETTE #1,2,3,4 */

	{CHANGE_PROCESSOR,					T_CHANGE_PROCESSOR},		/* CHANGE MESSAGE FORMAT */

	{CANCEL_AT_READ_CARD,				T_CANCEL_AT_READ_CARD},			/* CANCEL AT CARD READING */
	{CANCEL_AT_SELECT_LANGUAGE,			T_CANCEL_AT_SELECT_LANGUAGE},	/* CANCEL AT LANGUAGE SELECTION */
	{CANCEL_AT_INPUT_PIN,				T_CANCEL_AT_INPUT_PIN},			/* CANCEL AT PIN INPUT */
	{CANCEL_AT_SELECT_TRAN,				T_CANCEL_AT_SELECT_TRAN},		/* CANCEL AT TRANSACTION SELECTION */
	{CANCEL_AT_CONFIRM_FEE,				T_CANCEL_AT_CONFIRM_FEE},		/* CANCEL AT SURCHARGE CONFIRMATION */
	{CANCEL_AT_RECV_RECEIPT,			T_CANCEL_AT_RECV_RECEIPT},		/* CANCEL AT RECEIPT PRINTING SELECTION */
	{CANCEL_AT_RECEIPT_ERROR,			T_CANCEL_AT_RECEIPT_ERROR},		/* CANCEL AT CONTINUATION ON PRINT ERROR */
	{CANCEL_AT_RECEIPT_SCREEN,			T_CANCEL_AT_RECEIPT_SCREEN},	/* CANCEL AT RECEIPT DISPLAY ON SCRREN */
	{CANCEL_AT_FROM_ACCOUNT,			T_CANCEL_AT_FROM_ACCOUNT},		/* CANCEL AT FROM-ACCOUNT SELECTION */
	{CANCEL_AT_TO_ACCOUNT,				T_CANCEL_AT_TO_ACCOUNT},		/* CANCEL AT TO-ACCOUNT SELECTION */
	{CANCEL_AT_FASTCASH,				T_CANCEL_AT_FASTCASH},			/* CANCEL AT FASTCASH SELECTION */
	{CANCEL_AT_OTHER_AMOUNT,			T_CANCEL_AT_OTHER_AMOUNT},		/* CANCEL AT OTHER AMOUNT INPUT */
	{CANCEL_AT_TRANSFER_AMOUNT,			T_CANCEL_AT_TRANSFER_AMOUNT},	/* CANCEL AT TRANSFER AMOUNT INPUT */

	{OPERATOR_ACTION,					T_OPERATOR_ACTION},			/* EVENT */
	{CHANGE_EXCHANGE_RATE,				T_CHANGE_EXCHANGE_RATE},	/* EVENT */

	{CHANGE_MASTER_PWD,					T_CHANGE_MASTER_PWD},		/* CHANGE MASTER PASSWORD */
	{CHANGE_OPERATOR_PWD,				T_CHANGE_OPERATOR_PWD}	,	/* CHANGE OPERATOR PASSWORD */
	{CHANGE_SUPERVISOR_PWD,				T_CHANGE_SUPERVISOR_PWD},	/* CHANGE SERVICE PASSWORD */

	{POWER_ON_SYSTEM,					T_POWER_ON_SYSTEM},		/* SYSTEM POWER ON */
	{POWER_OFF_SYSTEM,					T_POWER_OFF_SYSTEM},	/* SYSTEM POWER OFF */		// [#2536]
	{CHANGE_SERVICE_MODE,				T_CHANGE_SERVICE_MODE},	/* CHANGE INTO SERVICE MODE */
	{CHANGE_ERROR_MODE,					T_CHANGE_ERROR_MODE},	/* CHANGE INTO ERROR MODE */
	{CHANGE_OP_MODE,					T_CHANGE_OP_MODE},		/* CHANGE INTO OPERATOR MODE */
	{CHANGE_OP_RESET,					T_CHANGE_OP_RESET},		/* RESET OPERATOR */
	{REBOOT_SYSTEM,						T_REBOOT_SYSTEM},		/* SYSTEM REBOOT */
	{CHANGE_DIAG_MODE,					T_CHANGE_DIAG_MODE},	/* CHANGE INTO DIAGNOSTICS MODE */

	{NORMAL_TRX,						T_NORMAL_TRX},			/* NORMAL TRANSACTION */
	{TRX_ERROR,							T_TRX_ERROR},			/* TRANSACTION ERROR */
	{REVERSAL_TRX,						T_REVERSAL_TRX},		/* REVERSAL TRANSACTION */
	{TRX_NOT_APPROVED,					T_TRX_NOT_APPROVED},	/* TRANSACTION IS NOT APPROVED */
	{COMMUNICATION_ERROR,				T_COMMUNICATION_ERROR},	/* COMMUNICATION ERROR */

	{MONIVIEW_ACTION,					T_MONIVIEW_ACTION},		/* EVENT */
	{HOST_ACTION,			    		T_HOST_ACTION},			/* EVENT */
	{CHANGE_PARAMETER, 	    			T_CHANGE_PARAMETER},	/* CHANGE PARAMETER */
	{TRX_EMV_DATA, 	    				T_TRX_EMV_DATA},		/* EMV DATA        */

	// [#2405] US Justin 2016.03.21 HalCash Online
	{PIN4_DAY_TOTAL,					T_JNL_PIN4_DAY_TOTAL},			/* CASH PICK-UP DAY TOTAL */	
	{PIN4_TRIALDAY_TOTAL,				T_JNL_PIN4_TRIALDAY_TOTAL},		/* Trial CASH PICK-UP  DAY TOTAL */
	// End of [#2405]

	// [#2442] US Justin 2016.08.24 PopMone (Trial) Day Total
	{POPMONEY_DAY_TOTAL,				T_JNL_POPMONEY_DAY_TOTAL},			/* Popmoney DAY TOTAL */
	{POPMONEY_TRIALDAY_TOTAL,			T_JNL_POPMONEY_TRIALDAY_TOTAL},		/* Popmoney Trial DAY TOTAL */
	// End of [#2442]

	// [#2445] US Justin 2016.09.26 Just. Cash
	{JUSTCASH_DAY_TOTAL,				T_JNL_JUSTCASH_DAY_TOTAL},			/* Just.Cash DAY TOTAL */
	{JUSTCASH_TRIALDAY_TOTAL,			T_JNL_JUSTCASH_TRIALDAY_TOTAL},		/* Trial Just.Cash  DAY TOTAL */
	// End of [#2445]

	// [#2496] US Justin 2017.08.17 Just. Cash
	{DIGITALCUR_DAY_TOTAL,				T_JNL_DIGITALCUR_DAY_TOTAL},		/* Digital Currency DAY TOTAL */
	{DIGITALCUR_TRIALDAY_TOTAL,			T_JNL_DIGITALCUR_TRIALDAY_TOTAL},	/* Trial Digital Currency  DAY TOTAL */
	// End of [#2496]

	// [#2446] US Justin 2016.09.29 Paypal CCA
	{PAYPAL_DAY_TOTAL,					T_JNL_PAYPAL_DAY_TOTAL},			/* Paypal CCA DAY TOTAL */
	{PAYPAL_TRIALDAY_TOTAL,				T_JNL_PAYPAL_TRIALDAY_TOTAL},		/* Trial Paypal CCA  DAY TOTAL */
	// End of [#2446]

	{B4U_DAY_TOTAL,						T_JNL_B4U_DAY_TOTAL},				/* B4U DAY TOTAL */	
	{B4U_TRIALDAY_TOTAL,				T_JNL_B4U_TRIALDAY_TOTAL},			/* Trial B4U DAY TOTAL */

	// [#RWC6-188] B4U Log journal entry for B4U for any transaction result after the amount entry screen
	{B4U_TXN,							T_JNL_B4U_TXN},						/* B4U TRANSACTION */  
	{B4U_TXN_ERROR_FAIL,				T_JNL_B4U_TXN_ERROR_FAIL},			/* B4U TRANSACTION ERROR - FAIL */
	{B4U_TXN_ERROR_CANCEL,				T_JNL_B4U_TXN_ERROR_CANCEL},		/* B4U TRANSACTION ERROR - CANCEL */
	{B4U_TXN_ERROR_TIMEOUT,				T_JNL_B4U_TXN_ERROR_TIMEOUT},		/* B4U TRANSACTION ERROR - TIMEOUT */
	// End of [#RWC6-188] 

	{LIBERTYX_TXN,						T_LTX_TXN_NORMAL},					/* LibertyX Transaction */
	{LIBERTYX_TXN_ERROR,				T_LTX_TXN_ERROR},					/* LibertyX Error Transaction */
	{LIBERTYX_TXN_DISPENSE,				T_LTX_TXN_DISPENSE},				/* LibertyX Dispense */

	// [#RWC6-399] US ryan.payton 2022.08.02 DigitalMint
	{DIGITALMINT_TXN,					T_DM_TXN_NORMAL},					/* DigitalMint Transaction */  
	{DIGITALMINT_TXN_ERROR,				T_DM_TXN_ERROR},					/* DigitalMint Error Transaction */
	// End of [#RWC6-188]

	// [RWC6-676] SKKim 2024.05.09 TangoPay
	{TANGOPAY_TXN,						T_TP_TXN_NORMAL},					/* TangoPay Transaction */  
	{TANGOPAY_TXN_ERROR,				T_TP_TXN_ERROR}						/* TangoPay Error Transaction */
	// [RWC6-676] End SKKim 2024.05.09

};

#define COUNT_CE_EJNL_KINDCODE (sizeof(g_CESupportEJKindCode)/sizeof(CE_JNL_KINDCODE_DESC))

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: CJnlMgr()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CJnlMgr::CJnlMgr()
{
	m_bInit = FALSE;
	m_nJNLStatus = NORMAL;	// [#462] [NH] KSK 2008.12.09
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ~CJnlMgr()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자.
-------------------------------------------------------------------*/
CJnlMgr::~CJnlMgr()
{
	Deinitialize();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: Initialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CJnlMgr::Initialize(CWnd *pDevCmn)
{
	m_pDevCmn = (CDevCmn*)pDevCmn;

	m_JnlDB.Initialize();

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: Deinitialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CJnlMgr::Deinitialize()
{
	m_JnlDB.Deinitialize();

	if (m_pDevCmn)
		m_pDevCmn = NULL;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ClearJNL()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CJnlMgr::ClearJNL()
{
	return m_JnlDB.Clear();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: Save()
 RETURN TYPE  : 
 PARAMETER    : strKindCode -	EJNL의 Kind Code..
				strJNLData  -   EJNL의 JNL Data..
								만약 JNL Data가 없다면 Kind Code가 데이터로 저장된다.
 DESCRIPTION  : JNL을 저장한다.
-------------------------------------------------------------------*/
BOOL CJnlMgr::Save(CString strKindCode, CString strEJNLData/*=L""*/)
{
	int i;
	CString strTemp, strTemp2;

	///////////////////////////////
	// 1. check!!  valid kindcode.
	for (i = 0; i < COUNT_CE_EJNL_KINDCODE; i++)
	{
		if (g_CESupportEJKindCode[i].szKindCode == strKindCode)
			break;
	}

	if (i >= COUNT_CE_EJNL_KINDCODE)
		return FALSE;

	// Make JNL DataU
	if (strEJNLData.GetLength() == 0)
	{
		// Make Transaction JNL Data
		if (strKindCode == NORMAL_TRX || strKindCode == TRX_ERROR || strKindCode == REVERSAL_TRX ||
			strKindCode == TRX_NOT_APPROVED || strKindCode == COMMUNICATION_ERROR || strKindCode == CHANGE_ERROR_MODE )
		{
			int nStrLen;
			int nWithdrawalType = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE);			// [#2415] US Justin
			CString strTransactionType = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE);				// [#2415] US Justin

			// 01. TERMINAL NUMBER -- 15 or 8 자리
			strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);	// [#371] [NH] KSK 2008.7.31
			strTemp.TrimLeft();
			strEJNLData.Format(L"%s", strTemp);
			
			// 02. SEQUENCE NUMBER
			strTemp.Format(L"^%4.4s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SEQUENCENO));
			strEJNLData += strTemp;
			
			// 03. TRANSACTION TYPE		
			//strTemp.Format(L"^%2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE));
			if(		 nWithdrawalType == WITHDRAWAL_POPMONEY )			strTemp.Format(L"^%2.2s", TRANTYPE_POPMONEY);			// [#2350] US Justin 2015.06.19 Add POP Money
			else if( nWithdrawalType == WITHDRAWAL_PIN4 )				strTemp.Format(L"^%2.2s", TRANTYPE_PIN4);				// [#2405] US Justin 2016.03.21 HalCash Online
			else if( nWithdrawalType == WITHDRAWAL_JUSTCASH )			strTemp.Format(L"^%2.2s", TRANTYPE_JUSTCASH);			// [#2445] US Justin 2016.09.28 Just.Cash
			else if( nWithdrawalType == WITHDRAWAL_PAYPAL )				strTemp.Format(L"^%2.2s", TRANTYPE_PAYPAL);				// [#2446] US Justin 2016.09.29 Paypal
			else														strTemp.Format(L"^%2.2s", strTransactionType );
			strEJNLData += strTemp;

			// 04. TRANSACTION FROM ACCOUNT
			// [#2150] US Justin 2012.10.04		No From Account for Pin Change Transaction
			// [#2405] US Justin 2016.04.18		No From Account for HalCash Online
			// [#2415] US Justin 2016.04.18		No From WalPay
			// [#2445] US Justin 2016.09.28		No From Just.Cash
			if( (strTransactionType == TRANTYPE_PINCHANGE)	||		
				(nWithdrawalType == WITHDRAWAL_PIN4) ||	
				(nWithdrawalType == WITHDRAWAL_JUSTCASH) )		
				strTemp = L"^--";
			else
				strTemp.Format(L"^%2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSFROMACCOUNT));
			strEJNLData += strTemp;
			
			// 05. TRANSACTION TO ACCOUNT	
			if(strTransactionType == TRANTYPE_TRANSFER)		strTemp.Format(L"^%2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTOACCOUNT));
			else											strTemp = L"^--";
			strEJNLData += strTemp;
			
			// 06. TRANSACTION BANK CODE
			// [#2405] US Justin 2016.03.21 HalCash Online
			// [#2445] US Justin 2016.09.28 Just.Cash
			if( 
#if(!APP_PRESTAGIN_PIN4)		// [#2515] US Justin 2017.11.20 Leave Account information for Phase 2(Prestaging) development
				(nWithdrawalType == WITHDRAWAL_PIN4) ||	
#endif
				(nWithdrawalType == WITHDRAWAL_JUSTCASH)  )
				strTemp = L"^----";
			else
				strTemp.Format(L"^%4.4s",MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKCODE));
			strEJNLData += strTemp;
			
			// 07. TRANSACTION BANK ACCOUNT -- 모두 '*'로 채움.		
			// [#2405] US Justin 2016.03.21 HalCash Online		
			// [#2445] US Justin 2016.09.28 Just.Cash
			if( 
#if(!APP_PRESTAGIN_PIN4)		// [#2515] US Justin 2017.11.20 Leave Account information for Phase 2(Prestaging) development
				(nWithdrawalType == WITHDRAWAL_PIN4 ) || 
#endif
				(nWithdrawalType == WITHDRAWAL_JUSTCASH) )		
				strEJNLData += L"^------------";
			else
			{
				strTemp2 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKACCOUNT);
				strTemp = CString('*', strTemp2.GetLength());
				strEJNLData += L"^";
				strEJNLData += strTemp;
			}
	
	//[#598] SOOK 2009.12.14 호주 SPS 인증 대응 
#if (AU_VERSION) // KSK 2014.05.15 Host에서 DATE 미수신시 Local Time 표시하도록 호주 사양 통일
			// 08. TRANSACTION HOST DATE
			strTemp.Format(L"^%8.8s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE));
			if ( strTemp != L"^01012000")// NVRAM Default
			{	
				strEJNLData += strTemp;
			
				// 09. TRANSACTION HOST TIME
				strTemp.Format(L"^%6.6s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME));
				strEJNLData += strTemp;
			}
			else
			{
				// Host로부터 날짜를 안받았을 경우 처리 Local Time 저장 한다. 
				SYSTEMTIME localTime;
				::GetLocalTime(&localTime);

				CString strDateTime;
				strDateTime.Format(_T("%2.2d%2.2d%4.4d"), localTime.wMonth, localTime.wDay, localTime.wYear);
				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE, strDateTime);
				strEJNLData += L"^";
				strEJNLData += strDateTime;

				strDateTime.Format(_T("%2.2d%2.2d%2.2d"), localTime.wHour, localTime.wMinute, localTime.wSecond);
				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME, strDateTime);
				strEJNLData += L"^";
				strEJNLData += strDateTime;
			}

#else
			// 08. TRANSACTION HOST DATE
			strTemp.Format(L"^%8.8s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE));
			strEJNLData += strTemp;
			
			// 09. TRANSACTION HOST TIME
			strTemp.Format(L"^%6.6s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME));
			strEJNLData += strTemp;
#endif
			// 10. TRANSACTION AVAIL BALANCE
			{
				// AIREAT 2008.05.09
				// HOST에서 받은 값을 저널에 남기기로 사양 변경.		
				// [#2150] US Justin 2012.10.04 Add Pin Change Transaction .. Journal Account....
				// [#2405] US Justin 2016.03.21 HalCash Online
				// [#2445] US Justin 2016.09.28 Just.Cash
				if( (strTransactionType == TRANTYPE_PINCHANGE) ||
					(nWithdrawalType == WITHDRAWAL_PIN4)	 ||	(nWithdrawalType == WITHDRAWAL_JUSTCASH)	)
					strTemp = L"^--------";
				else
				{
					// [#2460] BUG (Negative Amount, Bigger than 8 digit. Ex "-00007500" => Bug:"$-7.50". Correct:"$-$75.00")
					//strTemp.Format(L"^%8.8s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSAVAILBALANCE));
					int nAvailAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSAVAILBALANCE));
					if(nAvailAmt<0)	strTemp.Format(L"^-%07ld", (int)(-1*nAvailAmt));
					else			strTemp.Format(L"^%08ld",  nAvailAmt);
					// End of [#2460]
				}
				strEJNLData += strTemp;
			}
			
			// 11. TRANSACTION RETRIVAL NUMBER
			strTemp.Format(L"^%12.12s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRETRIEVALNUM));
			strEJNLData += strTemp;
			
			// 12. TRANSACTION AUDIT NUMBER
			strTemp.Format(L"^%6.6s%2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM),
												MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID));
			strEJNLData += strTemp;
			
			// 13. TRANSACTION ROUTING OR COMMMUNICATION ID
			if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
				strTemp2.Format(L"^%8.8s", MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID));		// STANDARD 3
			else
				strTemp2.Format(L"^%6.6s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));					// STANDARD 1,2
			// [#2115] MX KSK 2012.02.04 Journal Size를 최소화 하기 위해 SPACE 제거
			strTemp2.TrimLeft();
			strTemp2.TrimRight();
			strTemp.Format(L"%s", strTemp2);												// [#2152] US Justin 2012.10.05 Bug Fix... Remove Extra '^'... => Journal Shifted.
			strEJNLData += strTemp;
			// end of [#2115]
			
			// 14. TRANSACTION SETTLE DATE
			strTemp.Format(L"^%8.8s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSSETTLEDATE));
			strEJNLData += strTemp;
			
			// 15. TRANSACTION SURCHARGE
			// [#445] [NH] KSK 2008.11.06
			// 출금거래 중 Full Reversal시 Surcharge가 RMS로 송신되는 Bug Fix			
			// Balance or Transfer 거래시 Host에서 Surcharge가 왔을 경우 처리는 추후 사양 협의 후 대책 필요
			// [#2405] US Justin 2016.03.21 HalCash Online
			// [#2445] US Justin 2016.09.28 Just.Cash
			if( 
#if(!APP_PRESTAGIN_PIN4)		// [#2515] US Justin 2017.11.20 Leave Account information for Phase 2(Prestaging) development
				(nWithdrawalType == WITHDRAWAL_PIN4) || 
#endif
				(nWithdrawalType == WITHDRAWAL_JUSTCASH)  )
				strTemp.Format(L"^%8.8s", L"000");
			// End of [#2405]
			else
			{

#if (AU_VERSION)
				if (strKindCode != NORMAL_TRX)		// Balance Inquiry일 경우에는 TA / TC Journal이 두개 남는 경우가 발생함 (이때 Surcharge처리가 달라짐, 처리방안 필요)
#elif (MX_VERSION)
				if (Asc2Int(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT)) <= 0)		// [#2115] MX KSK 2012.02.04 장애와 상관없이 Host에서 받은 Surcharge or Local Surcharge를 Journal에 저장하도록 사양 변경
#else
				if ((strKindCode == REVERSAL_TRX &&
					Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT)) <= 0) ||
					(strKindCode == TRX_ERROR) || (strKindCode == TRX_NOT_APPROVED) || (strKindCode == COMMUNICATION_ERROR) || (strKindCode == CHANGE_ERROR_MODE))
#endif
				{
					strTemp.Format(L"^%8.8s", L"0");
				}
				else
				{
					// [#2137] MX KSK 2012.7.25
//					strTemp.Format(L"^%8.8s", MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT));
					int nSurcharge = Asc2Int(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT));
					strTemp.Format(L"^%d", nSurcharge);
					// end of [#2137]
				}
				// end of [#445]
			}
			strEJNLData += strTemp;
			
			// 16. TRANSACTION REQUEST AMOUNT
			strTemp.Format(L"^%8.8s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
			strEJNLData += strTemp;
			
			// 17. TRANSACTION DISPENSE AMOUNT
			strTemp.Format(L"^%8.8s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));
			strEJNLData += strTemp;
			
			// 18. TRANSACTION REMAIN AMOUNT
			{
				// AIREAT 2008.05.09
				// HOST에서 받은 값을 저널에 남기기로 사양 변경.
				// [#2014] NH KJW 2011.02.23
				// [#2150] US Justin 2012.10.04 Add Pin Change Transaction .. Journal Account....
				// [#2405] US Justin 2016.03.21 HalCash Online
				// [#2445] US Justin 2016.09.28 Just.Cash
				if( (strTransactionType == TRANTYPE_PINCHANGE) || 
					(nWithdrawalType == WITHDRAWAL_PIN4 )	 || (nWithdrawalType == WITHDRAWAL_JUSTCASH ) )													
					strTemp.Format(L"^%12.12s", L"000");
				else
					strTemp.Format(L"^%12.12s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT));
				strEJNLData += strTemp;
			}
			
			// 19. TRANSACTION PROC COUNT
			// [#419] [NH] KSK 2008.9.11
			if (!m_pDevCmn->fnAPL_CheckError())
				strTemp.Format(L"^ %s", m_pDevCmn->fstrAPL_GetErrorProCount());
			else
				strTemp.Format(L"^%2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT));
			// end of [#419]
			strEJNLData += strTemp;
			
			// 20. TRANSACTION RESULT
			if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT))
				strTemp = L"^ TRUE";
			else 
				strTemp = L"^FALSE";
			strEJNLData += strTemp;
			
			// 21. TRANSACTION ERROR CODE
			// [#419] [NH] KSK 2008.9.11
			if (!m_pDevCmn->fnAPL_CheckError())
				strTemp.Format(L"^%7.7s", m_pDevCmn->fstrAPL_GetErrorCode());
			else
				strTemp.Format(L"^0000000");
			// end of [#419]
			strEJNLData += strTemp;

			// 22. TRANSACTION CARD DATA
			if(nWithdrawalType == WITHDRAWAL_JUSTCASH)
			{
				strTemp.Format(L"^%s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA) );
			}
			// End of [#2445]
			else
			{				
				//[#598] SOOK 2009.12.14 호주 SPS 인증 대응 //호주 특이 사양 제거함 
				// 2010.01.13 호주 Customers Receipt에는 ************1234, 저널에는 123456******1234로 표시함 
#if (AU_C_VERSION)	// [#2062] AU KSK 2011.05.16 AU_A 고객 SPS에서는 미국과 동일한 사양으로 저널에 남기도록 요청함 (Customers만 하기 사양대로 적용함)

				// [#2200] AU KMK 2013.06.10 PAN Data 저널 저장 형식 변경
				// 16자 이상의 PAN : PAN Data 앞 6자 + ****** + PAN Data 뒤 4자
				// 15자 이하의 PAN : PAN Data 앞 (PAN길이-10자, 최소 0자) + ****** + PAN Data 뒤 4자
				// 호주 기준, _MEM_VAR_APP_TRANSCARDDATA에는 PAN DATA가 담겨있음 (Track2중 '=' 앞부분)
 				strTemp2 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA);
				// 16자 이상일 때 좌측 6글자, 우측 4글자를 가져와서 저널에 저장
				if (strTemp2.GetLength() >= 16)
					strTemp.Format(L"^%s******%s", strTemp2.Left(6), strTemp2.Right(4));
				// 15자 이하일 때 좌측 (길이-10)글자, 우측 4글자를 가져와서 저널에 저장. (길이-10)이 음수일 때는 공란으로 나옴.
				else
					strTemp.Format(L"^%s******%s", strTemp2.Left(strTemp2.GetLength()-10), strTemp2.Right(4));
				// end of [#2200]

#else
				strTemp2 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA);
				// Max 16자리 = 12(*)+4(카드번호)
				nStrLen =  __min(strTemp2.GetLength()-4, 12);
				if (nStrLen <= 0)
					nStrLen = 0;
				
				strTemp = L"^";
				strTemp += CString('*', nStrLen);
				strTemp += strTemp2.Right(4);
#endif
				//end of [#598]
			}
			strEJNLData += strTemp;
			
			// 23. TRANSACTION NON-CASH VALUE
			strTemp.Format(L"^%4.4s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSNONCASHVALUE));
			strEJNLData += strTemp;
			
			// 24. TRANSACTION NON-CASH TYPE
			strTemp.Format(L"^%12.12s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSNONCASHTYPE));
			strEJNLData += strTemp;
			
			// 25. TRANSACTION OTHER MESSAGE
			// [#2460] US Justin 2016.12.14 Combine Other Message for Dispense Result)
			/*
#if (AU_VERSION || MX_VERSION)		// [#519] [NH] KSK 2009.3.10	// [#2069] NH KSK 2011.06.13	// [#2115] MX KSK 2012.02.04
			// Journal 저장시 ErrorString을 제거하도록 수정
			CStringArray	strTempArray;
			CString			strTemp3;

			strTemp2 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG);
			SplitString(strTemp2, UNIT_DELIMITER, strTempArray);

			if (strTempArray.GetSize() >= 2)
			{
				if (strTempArray[0] != CDU_OVER_DISPENSE)
					strTemp2.Format(L"%s%c%s", strTempArray[0], UNIT_DELIMITER, strTempArray[1]);
			}

			if (strTransactionType == TRANTYPE_WITHDRAWAL)
			{
				#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
				if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_DISP_COUNT).GetLength())
				{
					// [#2137] MX KSK 2012.07.25
					// Journal이 Full나는 현상 대책을 위해 필요없는 인자 값 삭제
					strTemp3.Format(L"%c%s%s%s%s%s%s%s%s%s", UNIT_DELIMITER,
						RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_CURRENCYID, MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID), 
						RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_DISPENSED_COUNT, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_DISP_COUNT),
						RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_REJECTED_COUNT, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_REJ_COUNT));
					// end of [#2137]
				}
				else
				{
					// [#2137] MX KSK 2012.07.25
					strTemp3.Format(L"%c%s%s%s", UNIT_DELIMITER,
						RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_CURRENCYID, MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID));
					// end of [#2137]

				}
				#else
				if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_DISP_COUNT).GetLength())
				{
					strTemp3.Format(L"%c%s%s%s%s%s%s", UNIT_DELIMITER,
													   RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_DISPENSED_COUNT, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_DISP_COUNT),
													   RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_REJECTED_COUNT, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_REJ_COUNT));
				}
				#endif				// end of [#2115]
			}
			else if (strTransactionType == TRANTYPE_INQUIRY)
			{
				// [#2137] MX KSK 2012.07.27
				#if (MX_VERSION)
				strTemp3.Format(L"%c%s%s%s", UNIT_DELIMITER,
									RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_CURRENCYID, MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID));				
				#endif
				// end of [#2137]
			}

			strTemp.Format(L"^%s%s", strTemp2, strTemp3);

			#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04 BANK NAME & BANK FEE 추가
			if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_BANK_NAME).IsEmpty())
			{
				// BANK NAME ADD
				strTemp2.Empty();
				strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_BANKNAME, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_BANK_NAME));
				strTemp += strTemp2;
			}

			if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE).IsEmpty())
			{
				strTemp2 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE);
				if (Asc2Float(strTemp2) != 0)
				{
					// LOYALTY FEE ADD
					strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_LOYALTY_FEE, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE));
					strTemp += strTemp2;
				}
			}

			if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE).IsEmpty())
			{
				strTemp2 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE);
				if (Asc2Float(strTemp2) != 0)
				{
					// LINE USAGE FEE ADD
					strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_LINEUSAGE_FEE, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));
					strTemp += strTemp2;
				}
			}

			if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX).IsEmpty())
			{
				strTemp2 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX);
				if (Asc2Float(strTemp2) != 0)
				{
					// IVA TAX ADD
					strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_IVA_TAX, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX));
					strTemp += strTemp2;
				}
			}

			// Returned Account Number값 수신시에만 OtherMsg에 추가함.
			if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_RETURNED_ACCNT_NO).IsEmpty())
			{
				// Returned Account Number ADD
				strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_RETURNED_ACCNT_NO, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_RETURNED_ACCNT_NO));
				strTemp += strTemp2;
			}

			// [#2137] MX KMK 2012.07.19  EXCHANGE FEE ADDED
			if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE).IsEmpty())
			{
				strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_EXCHANGE_FEE, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE));
				strTemp += strTemp2;
			}

 			if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TOTAL_FEE).IsEmpty())	// [#2142] MX KMK 2012.08.16
			{
				strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_TOTAL_FEE, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TOTAL_FEE));	// [#2142] MX KMK 2012.08.16
				strTemp += strTemp2;
			}


			if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE).IsEmpty())
			{
				strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_EXCHANGERATE, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE));
				strTemp += strTemp2;
			}

			if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) != 0)
			{
				// Customer Type
				strTemp2.Format(L"%s%s%d", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_CUSTOMERTYPE, MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE));
				strTemp += strTemp2;
			}
			// end of [#2137]

			#endif	// end of [#2115]
#else
			strTemp.Format(_T("^%s"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG));	// [#86] KSK 2008.04.10
#endif	// end of [#519]

#if (AU_VERSION)	// [#2379] AU KSK 2015.11.17
			if (m_pDevCmn->m_bDomesticFallbackTrans == TRUE)
			{
				// Domestic 거래를 Journal에 저장 
				strTemp2.Format(L"%c%s%s%s", UNIT_DELIMITER, RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_AU_DOMESTICBIN_FALLBACK, L"DOMESTIC BIN - FALLBACK");
				strTemp += strTemp2;
			}
#endif		// end of [#2379]
			*/

			// Journal Data
			strTemp = L"^";

			// Currency ID (Mexico Only)
			#if (MX_VERSION)
				strTemp2.Format(L"%s%s%s",	RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_CURRENCYID, MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID));				
				strTemp += strTemp2;
			#endif

			// Add Dispensed and Reject Count
			if( strTransactionType == TRANTYPE_WITHDRAWAL || strTransactionType == TRANTYPE_DCC ||		
				strTransactionType == TRANTYPE_POPMONEY   || strTransactionType == TRANTYPE_PIN4 ||
				strTransactionType == TRANTYPE_JUSTCASH || strTransactionType == TRANTYPE_PAYPAL  )								
			{
				if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_DISP_COUNT).GetLength())
				{
					strTemp2.Format(L"%s%s%s%s%s%s",
						RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_DISPENSED_COUNT, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_DISP_COUNT),
						RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_REJECTED_COUNT, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_REJ_COUNT) );

					if(strTemp.GetLength() > 1)			strTemp += CString(UNIT_DELIMITER);
					strTemp += strTemp2;
				}
			}

			// Mexico Specific Data
			#if (MX_VERSION)	
				CString strTemp3 = L"";

				// BANK NAME ADD
				if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_BANK_NAME).IsEmpty())
				{
					strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_BANKNAME, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_BANK_NAME));
					strTemp3 += strTemp2;
				}
				// LOYALTY FEE
				if (Asc2Float(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE)) != 0)
				{					
					strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_LOYALTY_FEE, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE));
					strTemp3 += strTemp2;
				}
				// LINE USAGE FEE
				if (Asc2Float(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE)) != 0)
				{						
					strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_LINEUSAGE_FEE, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));
					strTemp3 += strTemp2;
				}
				// IVA TAX ADD
				if (Asc2Float(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX)) != 0)
				{
					strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_IVA_TAX, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX));
					strTemp3 += strTemp2;
				}
				// Returned Account Number
				if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_RETURNED_ACCNT_NO).IsEmpty())
				{
					strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_RETURNED_ACCNT_NO, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_RETURNED_ACCNT_NO));
					strTemp3 += strTemp2;
				}
				//  EXCHANGE FEE 
				if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE).IsEmpty())
				{
					strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_EXCHANGE_FEE, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE));
					strTemp3 += strTemp2;
				}
				// Total Fee
 				if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TOTAL_FEE).IsEmpty())
				{
					strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_TOTAL_FEE, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TOTAL_FEE));
					strTemp3 += strTemp2;
				}
				// Exchange Rate
				if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE).IsEmpty())
				{
					strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_EXCHANGERATE, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE));
					strTemp3 += strTemp2;
				}
				// Customer Type	
				if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) != 0)
				{					
					strTemp2.Format(L"%s%s%d", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_CUSTOMERTYPE, MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE));
					strTemp3 += strTemp2;
				}

				if( strTemp3.GetLength() > 0)
				{
					if(strTemp.GetLength() > 1)			strTemp += CString(UNIT_DELIMITER);
					strTemp += strTemp3;
				}
			#endif

			// AU Specific Data
			#if (AU_VERSION)
				if (m_pDevCmn->m_bDomesticFallbackTrans == TRUE)		// Domestic 거래를 Journal에
				{					
					strTemp2.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_AU_DOMESTICBIN_FALLBACK, L"DOMESTIC BIN - FALLBACK");
					if(strTemp.GetLength() > 1)			strTemp += CString(UNIT_DELIMITER);
					strTemp += strTemp2;
				}
			#endif

			// Add Existing Other Message (Remove Error Text)
			CStringArray	strOtherMsgArray;
			SplitString(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG), UNIT_DELIMITER, strOtherMsgArray);
			for( i=0; i<strOtherMsgArray.GetSize(); i++)
			{
				strTemp2 = L"";
				if (strOtherMsgArray[i].GetLength())
				{
					if (strOtherMsgArray[i].Left(1) == RMS_OTHERMSG_DELIMITER)
						strTemp2 = strOtherMsgArray[i];
					else if (strOtherMsgArray[i].Left(1) == CDU_OVER_DISPENSE)	// CDU Over Dispense  (2 UNITs)
					{					
						if (strOtherMsgArray.GetSize() >= i+2)
						{
							strTemp2.Format(L"%s%c%s", strOtherMsgArray[i], UNIT_DELIMITER, strOtherMsgArray[i+1]);
							i++;
						}
					}
					else														// Error Code (3 UNITs - PROC Count/Code/Error Text) => Remove Error Text (2 Units)
					{					
						if (strOtherMsgArray.GetSize() >= i+3)
						{
							strTemp2.Format(L"%s%c%s", strOtherMsgArray[i], UNIT_DELIMITER, strOtherMsgArray[i+1]);
							i += 2;
						}
					}
				}

				if( strTemp2.GetLength() > 0 )
				{
					if(strTemp.GetLength() > 1)			strTemp += CString(UNIT_DELIMITER);
					strTemp += strTemp2;
				}
			}
			// End of [#2460]
			strEJNLData += strTemp;
			
			// 26. SURCHARGE DISPLAY TO CUTOMER
			// [#2014] NH KJW 2011.02.23
			// [#2405] US Justin 2016.03.21 HalCash Online	
			// [#2445] US Justin 2016.09.28 Just.Cash
			if( 
#if(!APP_PRESTAGIN_PIN4)		// [#2515] US Justin 2017.11.20 Leave Account information for Phase 2(Prestaging) development
				(nWithdrawalType == WITHDRAWAL_PIN4) || 
#endif
				(nWithdrawalType == WITHDRAWAL_JUSTCASH) )													
				strTemp = L"^0";
			else
				strTemp.Format(L"^%1.1d", MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEDISPLAY));
			strEJNLData += strTemp;
		}

		// Change Password JNL Data
		else if (strKindCode == CHANGE_MASTER_PWD || strKindCode == CHANGE_OPERATOR_PWD || strKindCode == CHANGE_SUPERVISOR_PWD)
		{
			strEJNLData.Format(_T("********^********"));
		}
		else if (strKindCode == TRX_EMV_DATA)
		{
			// [#2115] MX KSK 2012.02.04
			// 01. TERMINAL NUMBER -- 15 or 8 자리
			strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
			strTemp.TrimLeft();
			strTemp.TrimRight();
			strEJNLData.Format(L"%s", strTemp);

			// 02. SEQUENCE NUMBER
			strTemp.Format(L"^%4.4s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SEQUENCENO));
			strEJNLData += strTemp;

			// 03. TRANSACTION AUDIT NUMBER
			strTemp.Format(L"^%6.6s%2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID));
			strEJNLData += strTemp;

			int		nLen = 0;
			unsigned char szTemp[1024] = {0,};
			CString	strValue, strValue2;
			CString strTagLenValue;

			strEJNLData += L"^";

			// 04. Application ID
			strValue = MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID);
			strValue2 = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_EMV_APPLICATION_NAME);		// [#2252] US Justin 2014.02.04 Long AID Bug Fix
			//if (strValue.GetLength() > 0)
			if( (strValue.GetLength() > 0) && (strValue2.GetLength() > 0) )					// [#2252] US Justin 2014.02.04 Long AID Bug Fix
			{
				//strTemp = strValue.Right(20);
				strTemp = strValue;			strTemp.TrimLeft();								// [#2252] US Justin 2014.02.04 Long AID Bug Fix
				strTemp.TrimRight();
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%s", APPLICATION_ID, strTemp.GetLength(), strTemp);
				strEJNLData += strTagLenValue;

				//strTemp = strValue.Left(18);
				strTemp = strValue2;		strTemp.TrimLeft();								// [#2252] US Justin 2014.02.04 Long AID Bug Fix
				strTemp.TrimRight();
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%s", APPLICATION_LABEL, strTemp.GetLength(), strTemp);
				strEJNLData += strTagLenValue;
			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", APPLICATION_ID, 0, 0);
				strEJNLData += strTagLenValue;
				strTagLenValue.Format(L"%s%02d%02d", APPLICATION_LABEL, 0, 0);
				strEJNLData += strTagLenValue;
			}

			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();

			// 06. AC (9F26)
			// [#706] MX KSK 2011.10.19 Host에서 전송한 AC값을 Journal에 저장 (고객 요청사항)
			strValue = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AC_REQ_9F26_VALUE);

			if( strValue.GetLength() > 0)
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%s", APP_CRYPTOGRAM_TAG_9F26, strValue.GetLength(), strValue);
			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", APP_CRYPTOGRAM_TAG_9F26, 0, 0);
			}								
			// end of [#706]

			strEJNLData += strTagLenValue;

			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();

			// 07. ARPC (91)
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			{
				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_IsuAuthData, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					if( nLen > 0)
					{
						/* Tag + Length + Value */
						strValue.Format(L"%s", strValue.Left(16));	// KSK 2011.10.19 Host에서 내려온 91값 중에 앞에 8BYTE만 저널에 저장 (고객 요청사항)
						strTagLenValue.Format(L"%s%02d%s", JNL_IAD, strValue.GetLength(), strValue);
					}
					else
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%02d", JNL_IAD, 0, 0);
					}
				}
				else
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%02d", JNL_IAD, 0, 0);
				}
			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", JNL_IAD, 0, 0);
			}

			strEJNLData += strTagLenValue;

			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();

			// 08. Service Code
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			{
				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_ServiceCod, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					if( nLen > 0)
					{
						/* Tag + Length + Value */
						strValue.Format(L"%d", Asc2Int(strValue));	// KSK 2011.10.19 service code는 3자리이나 kernel에서 0xxx로 올려주어 앞에 0 제거 로직 추가
						strTagLenValue.Format(L"%s%02d%s", SERVICE_CODE, strValue.GetLength(), strValue);
					}
					else
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%02d", SERVICE_CODE, 0, 0);
					}
				}
				else
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%02d", SERVICE_CODE, 0, 0);
				}
			}
			else
			{
				// MS에 있는 Service Code 저장 (고객 요청 사항 R5)
				strTemp = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2);
				int nResult = strTemp.Find(L"=");

				if (nResult != -1)
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%s", SERVICE_CODE, 3, strTemp.Mid(nResult+5, 3));
				}
				else
				{
					strTagLenValue.Format(L"%s%02d%02d", SERVICE_CODE, 0, 0);
				}
			}

			strEJNLData += strTagLenValue;

			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();

			// 09. Terminal Capability
			//[#2294] US Justin 2014.10.07 EMV data
#if (MX_VERSION)
			// EMV사양에는 TERMINAL CAPABILITY이나 PROSA에서는 TRITON과 동일한 POS ENTRY값을 TERMINAL CAPABILITY로 저장하도록 요청함 (R2로 수정함)
			// 현재 AP는 EMV 거래인 경우에만 해당 값을 저장하므로 무조건 "5"로 저장하도록 코딩함 (IC가 아닌 경우에는 "2"로 저장해야함)
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)
				strTagLenValue.Format(L"%s%02d%d", TERMINAL_CAPABILITIES_TAG_9F33, 1, 5);
			else
				strTagLenValue.Format(L"%s%02d%d", TERMINAL_CAPABILITIES_TAG_9F33, 1, 2);
#else
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// No data for Fallback Transaction
			{
				if( m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TerminalCapa, &nLen, szTemp) == EMV_RSLT_OK )
				{
					strValue = MakeUnPack(szTemp, nLen);
					if( (nLen >0)&&(nLen<=3) )
						strTagLenValue.Format(L"%s%02d%s", TERMINAL_CAPABILITIES_TAG_9F33, strValue.GetLength(), strValue);
					else
						strTagLenValue.Format(L"%s%02d%02d", TERMINAL_CAPABILITIES_TAG_9F33, 0, 0);
				}
				else
					strTagLenValue.Format(L"%s%02d%02d", TERMINAL_CAPABILITIES_TAG_9F33, 0, 0);
			}
			else
				strTagLenValue.Format(L"%s%02d%02d", TERMINAL_CAPABILITIES_TAG_9F33, 0, 0);
#endif
			// End of [#2294]
			strEJNLData += strTagLenValue;

			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();

			// 10. Pos Entry Mode
			//[#2294] US Justin 2014.10.07 EMV data
#if (MX_VERSION)
			// EMV사양에는 POS ENTRY값이 9F39이나, TRITON 사양서에 있는 값을 저장하도록 PROSA에서 요청함 (R2)
			// POS ENTRY값이 "05"인 경우 "90"으로 값을 설정하며 마지막 1BYTE는 무조건 "1"로 설정 ("901)
			// IC 거래인 경우에만 저장하므로 "901"로 FIX함
			// STANDARD1인 경우에는 다르게 처리할 필요가 있음
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)
				strTagLenValue.Format(L"%s%02d%s", POS_ENTRY_MODE_TAG_9F39, 3, L"051");
			else
				strTagLenValue.Format(L"%s%02d%s", POS_ENTRY_MODE_TAG_9F39, 3, L"901");
#else
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// No data for Fallback Transaction
			{
				if( m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_POSEntryMode, &nLen, szTemp) == EMV_RSLT_OK )
				{
					strValue = MakeUnPack(szTemp, nLen);
					if( (nLen >0)&&(nLen<=1) )
						strTagLenValue.Format(L"%s%02d%s", POS_ENTRY_MODE_TAG_9F39, strValue.GetLength(), strValue);
					else
						strTagLenValue.Format(L"%s%02d%02d", POS_ENTRY_MODE_TAG_9F39, 0, 0);
				}
				else
					strTagLenValue.Format(L"%s%02d%02d", POS_ENTRY_MODE_TAG_9F39, 0, 0);
			}
			else
				strTagLenValue.Format(L"%s%02d%02d", POS_ENTRY_MODE_TAG_9F39, 0, 0);
#endif
			// End of [#2294]
			strEJNLData += strTagLenValue;

			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();

			// 11. TVR (95)
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			{
				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TVR, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					if( nLen > 0)
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%s", JNL_TVR, strValue.GetLength(), strValue);
					}
					else
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%02d", JNL_TVR, 0, 0);
					}
				}
				else
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%02d", JNL_TVR, 0, 0);
				}
			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", JNL_TVR, 0, 0);
			}

			strEJNLData += strTagLenValue;

			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();

			// 12. IAC
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			{
				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_IACDenial, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					if( nLen > 0)
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%s", ISSUER_ACTION_CODE, strValue.GetLength(), strValue);
					}
					else
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%02d", ISSUER_ACTION_CODE, 0, 0);
					}
				}
				else
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%02d", ISSUER_ACTION_CODE, 0, 0);
				}
			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", ISSUER_ACTION_CODE, 0, 0);
			}

			strEJNLData += strTagLenValue;
			// end of [#2115]

			// [#2353] AU KSK 2015.06.28 Added EMV Extra Data (DC Payments requirement)
			#if AU_VERSION
			//////////////////////////////////////////////////////////////////////////
			// 13. Application Primary Account Number Sequence Number (5F34) (Len : 1Byte)
			// Journal View Line 부족으로 인해 제거
			//nLen = 0;
			//memset(szTemp, 0, sizeof(szTemp));
			//strValue.Empty();
			//strTagLenValue.Empty();
			//if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			//{
			//	if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_PANSeqNo, &nLen, szTemp) == EMV_RSLT_OK)
			//	{
			//		strValue = MakeUnPack(szTemp, nLen);

			//		/* Application Primary Account Number Sequence Number Max Len 1 */
			//		if( nLen > 0)
			//		{
			//			/* Tag + Length + Value */
			//			strTagLenValue.Format(L"%s%02d%s", APPLICATION_PAN_SEQ_TAG_5F34, strValue.GetLength(), strValue);
			//		}
			//		else
			//		{
			//			/* Tag + Length + Value */
			//			strTagLenValue.Format(L"%s%02d%02d", APPLICATION_PAN_SEQ_TAG_5F34, 0, 0);
			//		}
			//	}
			//	else
			//	{
			//		/* Tag + Length + Value */
			//		strTagLenValue.Format(L"%s%02d%02d", APPLICATION_PAN_SEQ_TAG_5F34, 0, 0);
			//	}
			//}
			//else
			//{
			//	/* Tag + Length + Value */
			//	strTagLenValue.Format(L"%s%02d%02d", APPLICATION_PAN_SEQ_TAG_5F34, 0, 0);
			//}
			//strEJNLData += strTagLenValue;

			//////////////////////////////////////////////////////////////////////////
			// 14. Application Interchange Profile (82) (Len : 2Byte)
			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			{
				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_AIP, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					/* Application Interchange Profile Max Len 2*/
					if( nLen > 0)
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%s", JNL_AIP_8200, strValue.GetLength(), strValue);
					}
					else
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%02d", JNL_AIP_8200, 0, 0);
					}
				}
				else
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%02d", JNL_AIP_8200, 0, 0);
				}
			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", JNL_AIP_8200, 0, 0);
			}
			strEJNLData += strTagLenValue;	

			//////////////////////////////////////////////////////////////////////////
			// 15. Authorisation Response Code (8A) (Len : 2Byte)
			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			{
				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_AuthRespCod, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					/* Application Interchange Profile Max Len 2*/
					if( nLen > 0)
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%s", JNL_ARC_8A00, strValue.GetLength(), strValue);
					}
					else
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%02d", JNL_ARC_8A00, 0, 0);
					}
				}
				else
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%02d", JNL_ARC_8A00, 0, 0);
				}

			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", JNL_ARC_8A00, 0, 0);
			}
			strEJNLData += strTagLenValue;	

			//////////////////////////////////////////////////////////////////////////
			// 16. Transaction Type (9C)
			//nLen = 0;
			//memset(szTemp, 0, sizeof(szTemp));
			//strValue.Empty();
			//strTagLenValue.Empty();
			//if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			//{
			//	if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TrType, &nLen, szTemp) == EMV_RSLT_OK)
			//	{
			//		strValue = MakeUnPack(szTemp, nLen);

			//		/* Transaction Type Max Len 1 */
			//		if( nLen > 0)
			//		{
			//			/* Tag + Length + Value */
			//			strTagLenValue.Format(L"%s%02X%s", JNL_TRANSTYPE_9C00, strValue.GetLength(), strValue);
			//		}
			//		else
			//		{
			//			/* Tag + Length + Value */
			//			strTagLenValue.Format(L"%s%02d%02d", JNL_TRANSTYPE_9C00, 0, 0);
			//		}
			//	}
			//	else
			//	{
			//		/* Tag + Length + Value */
			//		strTagLenValue.Format(L"%s%02d%02d", JNL_TRANSTYPE_9C00, 0, 0);
			//	}
			//}
			//else
			//{
			//	/* Tag + Length + Value */
			//	strTagLenValue.Format(L"%s%02d%02d", JNL_TRANSTYPE_9C00, 0, 0);
			//}
			//strEJNLData += strTagLenValue;

			//////////////////////////////////////////////////////////////////////////
			// 17. Transaction Status Information (9B)
			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			{
				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TSI, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					/* Transaction Status Information Max Len 2 */
					if( nLen > 0 )
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%s", JNL_TSI_9B00, strValue.GetLength(), strValue);
					}
					else
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%02d", JNL_TSI_9B00, 0, 0);
					}
				}
				else
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%02d", JNL_TSI_9B00, 0, 0);
				}
			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", JNL_TSI_9B00, 0, 0);
			}
			strEJNLData += strTagLenValue;

			//////////////////////////////////////////////////////////////////////////
			// 18. Issuer Application Data (9F10) (Len : 32Byte)
			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			{
				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_IsuAppData, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					// Data가 너무 크므로 앞 16byte만(Unpack시 32byte) 저장하는 것으로 임시 대응
					strTemp = strValue.Left(32);

					/* Issuer Application Data Max Len 32*/
					if( nLen > 0)
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%s", ISSUER_APP_DATA_TAG_9F10, strTemp.GetLength(), strTemp);
					}
					else
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%02d", ISSUER_APP_DATA_TAG_9F10, 0, 0);
					}
				}
				else
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%02d", ISSUER_APP_DATA_TAG_9F10, 0, 0);
				}
			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", ISSUER_APP_DATA_TAG_9F10, 0, 0);
			}
			strEJNLData += strTagLenValue;

			//////////////////////////////////////////////////////////////////////////
			// 19. Cryptogram Information Data  (9F27) (Len : 1Byte)
			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			{
				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_CryptInfData, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					/* Cryptogram Information Data Max Len 1 */
					if( nLen > 0 )
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%s", CRYPTOGRAM_INFO_DATA_TAG_9F27, strValue.GetLength(), strValue); 
					}
					else
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%02d", CRYPTOGRAM_INFO_DATA_TAG_9F27, 0, 0); 
					}
				}
				else
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%02d", CRYPTOGRAM_INFO_DATA_TAG_9F27, 0, 0); 
				}
			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", CRYPTOGRAM_INFO_DATA_TAG_9F27, 0, 0); 
			}
			strEJNLData += strTagLenValue;

			//////////////////////////////////////////////////////////////////////////
			// 20. Application Transaction Counter (9F36) (Len : 2Byte)
			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			{
				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_ATC, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					/* Application Transaction Counter Len 2*/
					if( nLen > 0 )
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%s", APP_TRANSACTION_COUNTER_TAG_9F36, strValue.GetLength(), strValue);
					}
					else
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%02d", APP_TRANSACTION_COUNTER_TAG_9F36, 0, 0);
					}
				}
				else
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%02d", APP_TRANSACTION_COUNTER_TAG_9F36, 0, 0);
				}
			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", APP_TRANSACTION_COUNTER_TAG_9F36, 0, 0);
			}
			strEJNLData += strTagLenValue;

			//////////////////////////////////////////////////////////////////////////
			// 21. Unpredictable Number (9F37) (Len : 4Byte)
			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			{
				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_UnpredictNo, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					/* Unpredictable Number Max Len 4 */
					if( nLen > 0 )
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%s", RANDOM_NUMBER_TAG_9F37, strValue.GetLength(), strValue); 
					}
					else
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%02d", RANDOM_NUMBER_TAG_9F37, 0, 0);
					}
				}
				else
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%02d", RANDOM_NUMBER_TAG_9F37, 0, 0);
				}
			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", RANDOM_NUMBER_TAG_9F37, 0, 0);
			}
			strEJNLData += strTagLenValue;

			//////////////////////////////////////////////////////////////////////////
			// 22. Terminal Currency Code (5F2A) (Len : 2Byte)
			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			{
				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TrCurCod, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					/* Terminal Currency Code Max Len 2 */
					if( nLen > 0 )
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%s", TRANSACTION_CURRENCY_CODE_TAG_5F2A, strValue.GetLength(), strValue);
					}
					else
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%02d", TRANSACTION_CURRENCY_CODE_TAG_5F2A, 0, 0);
					}
				}
				else
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%02d", TRANSACTION_CURRENCY_CODE_TAG_5F2A, 0, 0);
				}
			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", TRANSACTION_CURRENCY_CODE_TAG_5F2A, 0, 0);
			}
			strEJNLData += strTagLenValue;

			//////////////////////////////////////////////////////////////////////////
			// 23. Terminal Country Code (9F1A) (Len : 2Byte)
			nLen = 0;
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTagLenValue.Empty();
			if (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength() > 0)	// KSK 2011.11.14 Fallback일 경우 data 표시 안함
			{
				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TCountryCod, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					/* Terminal Country Code Max Len 2 */
					if( nLen > 0 )
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%s", TERMINAL_COUNTRY_CODE_TAG_9F1A, strValue.GetLength(), strValue);
					}
					else
					{
						/* Tag + Length + Value */
						strTagLenValue.Format(L"%s%02d%02d", TERMINAL_COUNTRY_CODE_TAG_9F1A, 0, 0);
					}
				}
				else
				{
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02d%02d", TERMINAL_COUNTRY_CODE_TAG_9F1A, 0, 0);
				}
			}
			else
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02d%02d", TERMINAL_COUNTRY_CODE_TAG_9F1A, 0, 0);
			}
			strEJNLData += strTagLenValue;
			#endif
			// end of [#2353]
		}
		// Make Default JNL Data
		else
		{
			strEJNLData = strKindCode;
		}
	}

	// [#462] [NH] KSK 2008.12.09
	// Save Journal
//	return m_JnlDB.Write(strKindCode, strEJNLData);
	BOOL bResult = m_JnlDB.Write(strKindCode, strEJNLData);
	if ( bResult == FALSE)
		m_nJNLStatus = DOWN;

	return bResult;
	// end of [#462]
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: SearchIndex()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : JNL Index를 JNL을 검색한다.
-------------------------------------------------------------------*/
CString	CJnlMgr::SearchIndex(UINT32 nJnlIndex)
{
	return m_JnlDB.Search(nJnlIndex);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: MoveFirst()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : First JNL으로 이동한다.
-------------------------------------------------------------------*/
CString CJnlMgr::MoveFirst()
{
	return m_JnlDB.MoveFirst();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: MoveLast()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Last JNL으로 이동한다.
-------------------------------------------------------------------*/
CString CJnlMgr::MoveLast()
{
	return m_JnlDB.MoveLast();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: MoveNext()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 정해진 크기만큼 떨어진 다음 JNL으로 이동한다.
-------------------------------------------------------------------*/
CString CJnlMgr::MoveNext(UINT32 nSize/* =1 */)
{
	return m_JnlDB.MoveNext(nSize);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: MovePrev()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 정해진 크기만큼 떨어진 이전 JNL으로 이동한다.
-------------------------------------------------------------------*/
CString CJnlMgr::MovePrev(UINT32 nSize/* =1 */)
{
	return m_JnlDB.MovePrev(nSize);
}

// [#508] AIREAT 2009.02.27 : Search by date Method 추가
/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: SearchByDate()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 정해진 기간에 해당하는 모든 저널의 인덱스를 가져온다.
-------------------------------------------------------------------*/
BOOL CJnlMgr::SearchByDate(CTime StartDate, CTime EndDate, CStringArray &Indexs)
{
	return m_JnlDB.SearchByDate(StartDate, EndDate, Indexs);
}
// end of [#508]

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: GetFirstIndex()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Journal의 처음 Index를 조회한다.
-------------------------------------------------------------------*/
INT32 CJnlMgr::GetFirstIndex()
{
	return m_JnlDB.GetFirstIndex();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: GetLastIndex()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Journal의 마지막 Index를 조회한다.
-------------------------------------------------------------------*/
INT32 CJnlMgr::GetLastIndex()
{
	return m_JnlDB.GetLastIndex();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: GetCurrentIndex()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Current Position의 Journal Index를 조회한다.
-------------------------------------------------------------------*/
INT32 CJnlMgr::GetCurrentIndex()
{
	return m_JnlDB.GetCurrentIndex();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: GetTotalCount()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Journal의 총 개수를 조회한다.
-------------------------------------------------------------------*/
INT32 CJnlMgr::GetTotalCount()
{
	return m_JnlDB.GetTotalCount();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: GetUploadLastIndex()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Upload된 마지막 Journal Index를 조회한다.
-------------------------------------------------------------------*/
INT32 CJnlMgr::GetUploadLastIndex(UP_TYPE Type)
{
	return m_JnlDB.GetUploadLastIndex(Type);
}

// [#2076] NH KSK 2011.06.29
/*-------------------------------------------------------------------
CLASS    NAME: CJnlMgr
FUNCTION NAME: SetUploadedIndex()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Upload 성공 후 성공한 Index를 설정한다.
-------------------------------------------------------------------*/
INT32 CJnlMgr::SetUploadedIndex(UP_TYPE Type, INT32 nJnlIndex)
{
	return m_JnlDB.SetUploadedIndex(Type, nJnlIndex);
}
// end of [#2076]

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: GetUploadRemainCount()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Upload할 Journal Count를 조회한다.
-------------------------------------------------------------------*/
INT32 CJnlMgr::GetUploadRemainCount(UP_TYPE Type)
{
	return m_JnlDB.GetUploadRemainCount(Type);
}

// [#462] [NH] KSK 2008.12.09
/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: GetJNLStatus()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Journal Write 가능 여부를 조회한다.
-------------------------------------------------------------------*/
INT32 CJnlMgr::GetJNLStatus()
{
	return m_nJNLStatus;
}
// end of [#462]

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: UploadMoveNext()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Upload할 다음 Journal를 가져온다.
-------------------------------------------------------------------*/
CString CJnlMgr::UploadMoveNext(UP_TYPE Type)
{
	return m_JnlDB.UploadMoveNext(Type);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: BackupJNLData()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 모든 저널 데이터를 주어진 Path에 Copy 한다.
-------------------------------------------------------------------*/
BOOL CJnlMgr::BackupJNLData(CString strDestPath)
{
	return m_JnlDB.BackupJNLData(strDestPath);
}

// [#508] AIREAT 2009.02.27 : Save JNL Data Method 추가
/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: SaveJNLData()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Indexs에 해당하는 저널을 주어진 파일에 저장한다.
-------------------------------------------------------------------*/
BOOL CJnlMgr::SaveJNLData(CString strFileName, CStringArray &Indexs)
{
	return m_JnlDB.SaveJNLData(strFileName, Indexs);
}
// end of [#508]

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: UploadMoveNext()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : JNL Data를 SPR Print Data로 변환한다.
-------------------------------------------------------------------*/
CString	CJnlMgr::ConvertPrintData(int PrintDevice, CString strJnlData)
{
	int		DescKindCodeIndex;
	CString	strPrintData = L"", strTemp;
	CStringArray	arJnlField;
	CTime	CurTime = CTime::GetCurrentTime();

	SplitString(strJnlData,"^", arJnlField);

	// check Device
	if (PrintDevice != DEV_SPR && PrintDevice != DEV_SCR)
		return strPrintData;

	// check size.
	if (arJnlField.GetSize() <= 9)
		return strPrintData;

	// check!!  valid kindcode.
	arJnlField[1].TrimLeft();
	for (DescKindCodeIndex = 0; DescKindCodeIndex < COUNT_CE_EJNL_KINDCODE; DescKindCodeIndex++)
	{
		if (g_CESupportEJKindCode[DescKindCodeIndex].szKindCode == arJnlField[1])
			break;
	}

	if (DescKindCodeIndex >= COUNT_CE_EJNL_KINDCODE)
		return strPrintData;

	strPrintData = L"========================================^";

	//////////////////////////////////
	// Transaction JNL
	if (arJnlField[1] == NORMAL_TRX || arJnlField[1] == TRX_ERROR || arJnlField[1] == REVERSAL_TRX ||
		arJnlField[1] == TRX_NOT_APPROVED || arJnlField[1] == COMMUNICATION_ERROR)
	{
		//[#2210] MX Justin 2013.07.12 Add Address on top of Transaction Journal
#if(MX_VERSION)
		// ADDRESS 1,2,3,PHONE NUMBER PRINT 
		//------------------------------------------------------------------------------------------------
		for(int i=0; i<4; i++)
		{
			strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1 + i);
			strTemp.TrimLeft();

			if (strTemp.GetLength() > 0)
			{
				strTemp.Format(L"%s^", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1 + i) );
				strPrintData += strTemp;
			}
		}
		//------------------------------------------------------------------------------------------------
#endif
		// end of [#2210]

		strPrintData += ConvertTranToPrint(DescKindCodeIndex, arJnlField);
	}
	//////////////////////////////////
	// ERROR JNL
	else 
	if (arJnlField[1] == CHANGE_ERROR_MODE)
	{
		strPrintData += ConvertErrorToPrint(DescKindCodeIndex, arJnlField);
	}
	//////////////////////////////////
	// Add Cash JNL
	else 
 	if (arJnlField[1] == ADDCASH_CST1 || arJnlField[1] == ADDCASH_CST2 || arJnlField[1] == ADDCASH_CST1_2 ||
 		arJnlField[1] == ADDCASH_CST3 || arJnlField[1] == ADDCASH_CST1_3 || arJnlField[1] == ADDCASH_CST2_3 ||			// [#332] NH AIREAT 2008.06.24
 		arJnlField[1] == ADDCASH_CST1_2_3 || arJnlField[1] == ADDCASH_CST4 || arJnlField[1] == ADDCASH_CST1_4 ||
 		arJnlField[1] == ADDCASH_CST2_4 || arJnlField[1] == ADDCASH_CST1_2_4 || arJnlField[1] == ADDCASH_CST3_4 ||
 		arJnlField[1] == ADDCASH_CST1_3_4 || arJnlField[1] == ADDCASH_CST2_3_4 || arJnlField[1] == ADDCASH_CST1_2_3_4 ||
 		arJnlField[1] == REMOTE_ADDCASH_CST1 || arJnlField[1] == REMOTE_ADDCASH_CST2 || arJnlField[1] == REMOTE_ADDCASH_CST1_2 ||
 		arJnlField[1] == REMOTE_ADDCASH_CST3 || arJnlField[1] == REMOTE_ADDCASH_CST1_3 || arJnlField[1] == REMOTE_ADDCASH_CST2_3 ||			// [#332] NH AIREAT 2008.06.24
 		arJnlField[1] == REMOTE_ADDCASH_CST1_2_3 || arJnlField[1] == REMOTE_ADDCASH_CST4 || arJnlField[1] == REMOTE_ADDCASH_CST1_4 ||
 		arJnlField[1] == REMOTE_ADDCASH_CST2_4 || arJnlField[1] == REMOTE_ADDCASH_CST1_2_4 || arJnlField[1] == REMOTE_ADDCASH_CST3_4 ||
 		arJnlField[1] == REMOTE_ADDCASH_CST1_3_4 || arJnlField[1] == REMOTE_ADDCASH_CST2_3_4 || arJnlField[1] == REMOTE_ADDCASH_CST1_2_3_4)	// [#530] NH KSK 2009.06.09
	{
		strPrintData += ConvertAddCashToPrint(DescKindCodeIndex, arJnlField);
	}
	//////////////////////////////////
	// Denomination JNL
	else
 	if (arJnlField[1] == CHANGE_DENOM_CST1 || arJnlField[1] == CHANGE_DENOM_CST2 || arJnlField[1] == CHANGE_DENOM_CST1_2 ||
 		arJnlField[1] == CHANGE_DENOM_CST3 || arJnlField[1] == CHANGE_DENOM_CST1_3 || arJnlField[1] == CHANGE_DENOM_CST2_3 ||			// [#332] NH AIREAT 2008.06.24
 		arJnlField[1] == CHANGE_DENOM_CST1_2_3 || arJnlField[1] == CHANGE_DENOM_CST4 || arJnlField[1] == CHANGE_DENOM_CST1_4 ||
 		arJnlField[1] == CHANGE_DENOM_CST2_4 || arJnlField[1] == CHANGE_DENOM_CST1_2_4 || arJnlField[1] == CHANGE_DENOM_CST3_4 ||
 		arJnlField[1] == CHANGE_DENOM_CST1_3_4 || arJnlField[1] == CHANGE_DENOM_CST2_3_4 || arJnlField[1] == CHANGE_DENOM_CST1_2_3_4)																	// [#332] NH AIREAT 2008.06.24
	{
		strPrintData += ConvertDenoToPrint(DescKindCodeIndex, arJnlField);
	}
	//////////////////////////////////
	// Day Total, Trial Day Total
	else
	if (arJnlField[1] == DAY_TOTAL || arJnlField[1] == TRIALDAY_TOTAL ||
		arJnlField[1] == REMOTE_DAY_TOTAL || arJnlField[1] == REMOTE_TRIALDAY_TOTAL)	// [#532] NH KSK 2009.06.09
	{
		strPrintData += ConvertDayTotalToPrint(DescKindCodeIndex, arJnlField);
	}
	//////////////////////////////////
	// CST Total, Trial CST Total
	else if (arJnlField[1] == CST_TOTAL || arJnlField[1] == TRIALCST_TOTAL || arJnlField[1] == REMOTE_CST_TOTAL)		// [#531] NH KSK 2009.06.09
	{
		strPrintData += ConvertCstTotalToPrint(DescKindCodeIndex, arJnlField);
	}
	// [#GLDV-3005] US Kook 2022.04.14 support side car
	else if (arJnlField[1] == CASHIN_TOTAL || arJnlField[1] == TRIALCASHIN_TOTAL)
	{
		strPrintData += ConvertCashInTotalToPrint_Demo(DescKindCodeIndex, arJnlField);
	}
	// end of [#GLDV-3005]
	//////////////////////////////////
	// [#2442] US Justin 2016.08.25 Brief Day Total: Pin4, Popmoney, just.cash
	// added B4U
	else if ( arJnlField[1] == PIN4_DAY_TOTAL		|| arJnlField[1] == PIN4_TRIALDAY_TOTAL		||
			  arJnlField[1] == POPMONEY_DAY_TOTAL	|| arJnlField[1] == POPMONEY_TRIALDAY_TOTAL	||
			  arJnlField[1] == JUSTCASH_DAY_TOTAL	|| arJnlField[1] == JUSTCASH_TRIALDAY_TOTAL	||			// [#2445]
			  arJnlField[1] == PAYPAL_DAY_TOTAL		|| arJnlField[1] == PAYPAL_TRIALDAY_TOTAL	||			// [#2446]
			  arJnlField[1] == B4U_DAY_TOTAL		|| arJnlField[1] == B4U_TRIALDAY_TOTAL		||
			  arJnlField[1] == DIGITALCUR_DAY_TOTAL	|| arJnlField[1] == DIGITALCUR_TRIALDAY_TOTAL )
	{
		strPrintData += ConvertBriefDayTotalToPrint(DescKindCodeIndex, arJnlField);
	}

	//////////////////////////////////
	// Processor JNL
	else if (arJnlField[1] == CHANGE_PROCESSOR)
	{
		strPrintData += ConvertCommMsgToPrint(DescKindCodeIndex, arJnlField);
	}
	//////////////////////////////////
	// In to Operator Mode
	else if (arJnlField[1] == CHANGE_OP_MODE)
	{
		strPrintData += ConvertOperatorToPrint(DescKindCodeIndex, arJnlField);
	}
	//////////////////////////////////
	// Change Password
	else if (arJnlField[1] == CHANGE_MASTER_PWD || arJnlField[1] == CHANGE_OPERATOR_PWD ||arJnlField[1] == CHANGE_SUPERVISOR_PWD)
	{
		strPrintData += ConvertPasswordToPrint(DescKindCodeIndex, arJnlField);
	}
	//////////////////////////////////
	// Operator Action
	else if (arJnlField[1] == OPERATOR_ACTION || arJnlField[1] == MONIVIEW_ACTION || arJnlField[1] == HOST_ACTION) //[#610] SOOK 2010.01.18 HOST ACTION,MONIVIEW ACTION 추가  
	{
		strPrintData += ConvertOperatorAction(DescKindCodeIndex, arJnlField);
	}
	//////////////////////////////////
	// CHANGE EXCHANGE RATE
	else if (arJnlField[1] == CHANGE_EXCHANGE_RATE)	// KSK 2009.3.30
	{
		strPrintData += ConvertChangeExchangeRate(DescKindCodeIndex, arJnlField);
	}
	//////////////////////////////////
	// USER CANCEL
	else if (arJnlField[1] == CANCEL_AT_READ_CARD || arJnlField[1] == CANCEL_AT_SELECT_LANGUAGE || arJnlField[1] == CANCEL_AT_INPUT_PIN ||
		arJnlField[1] == CANCEL_AT_SELECT_TRAN || arJnlField[1] == CANCEL_AT_CONFIRM_FEE || arJnlField[1] == CANCEL_AT_RECV_RECEIPT ||
		arJnlField[1] == CANCEL_AT_RECEIPT_ERROR || arJnlField[1] == CANCEL_AT_RECEIPT_SCREEN || arJnlField[1] == CANCEL_AT_FROM_ACCOUNT ||
		arJnlField[1] == CANCEL_AT_TO_ACCOUNT || arJnlField[1] == CANCEL_AT_FASTCASH || arJnlField[1] == CANCEL_AT_OTHER_AMOUNT ||
		arJnlField[1] == CANCEL_AT_TRANSFER_AMOUNT)
	{
		strPrintData += ConvertUserCancelToPrint(DescKindCodeIndex, arJnlField);
	}
	else if ( arJnlField[1] == CHANGE_PARAMETER)
	{
		strPrintData += ConvertChangeParameterToPrint(DescKindCodeIndex, arJnlField);
	}
	else if ( arJnlField[1] == TRX_EMV_DATA)
	{
		strPrintData = ConvertTrxEMVDataToPrint(DescKindCodeIndex, arJnlField);			// [#2115] MX KSK 2012.02.04 EMV DATA 추가
	}
	else if ( arJnlField[1] == LIBERTYX_TXN || arJnlField[1] == LIBERTYX_TXN_ERROR )
	{
		strPrintData += ConvertLibertyXToPrint(DescKindCodeIndex, arJnlField);			// [#RWC6-59] US William 2019.10.28 LibertyX
	}
	else if ( arJnlField[1] == LIBERTYX_TXN_DISPENSE )
	{
		strPrintData += ConvertLibertyXDispenseToPrint(DescKindCodeIndex, arJnlField);	// [#RWC6-224] US William 2020.10.14 LibertyX Dispense
	}
	else if ( arJnlField[1] == B4U_TXN || arJnlField[1] == B4U_TXN_ERROR_FAIL ||
			  arJnlField[1] == B4U_TXN_ERROR_CANCEL || arJnlField[1] == B4U_TXN_ERROR_TIMEOUT )
	{
		strPrintData += ConvertB4UToPrint(DescKindCodeIndex, arJnlField);				// [#RWC6-188] B4U Log journal entry
	}
	else if (arJnlField[1] == DIGITALMINT_TXN || arJnlField[1] == DIGITALMINT_TXN_ERROR)
	{
		strPrintData += ConvertDigitalMintToPrint(DescKindCodeIndex, arJnlField);		// [#RWC6-399] US ryan.payton 2022.08.02 DigitalMint
	}
	else if (arJnlField[1] == TANGOPAY_TXN || arJnlField[1] == TANGOPAY_TXN_ERROR)
	{
		strPrintData += ConvertTangoPayToPrint(DescKindCodeIndex, arJnlField);			// [RWC6-676] SKKim 2024.05.09
	}

	//////////////////////////////////
	// Default JNL
	else
	{
		strPrintData += ConvertDefaultToPrint(DescKindCodeIndex, arJnlField);
	}

	strPrintData.Replace('^', FIELD_DELIMITER);
	strPrintData.Replace(PROXY_JNL_DELIMITER, '^');//[#610] SOOK 2010.01.27
	return strPrintData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertAddCashToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : ADD CASH JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertAddCashToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int	nRemainCST1 = 0, nRemainCST2 = 0, nRemainCST3 = 0, nRemainCST4 = 0;
	int nAddCST1 = 0, nAddCST2 = 0, nAddCST3 = 0, nAddCST4 = 0;
	int nJnlField, nJnlIndex;
	
	nJnlIndex = Asc2Int(arJnlField[2]);

	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;
	
	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;
	
	// [#332] NH AIREAT 2008.06.24

	nJnlField = arJnlField.GetSize();
	
	// Get CST1 Value
	if (nJnlField >= 11)
	{
		nRemainCST1 = Asc2Int(arJnlField[9]);
		nAddCST1 = Asc2Int(arJnlField[10]);
	}

	// Get CST2 Value
	if (nJnlField >= 13)
	{
		nRemainCST2 = Asc2Int(arJnlField[11]);
		nAddCST2 = Asc2Int(arJnlField[12]);
	}
	
	// Get CST3 Value
	if (nJnlField >= 15)
	{
		nRemainCST3 = Asc2Int(arJnlField[13]);
		nAddCST3 = Asc2Int(arJnlField[14]);
	}
	
	// Get CST4 Value
	if (nJnlField >= 17)
	{
		nRemainCST4 = Asc2Int(arJnlField[15]);
		nAddCST4 = Asc2Int(arJnlField[16]);
	}

	// end of [#332]

	// KSK 2009.2.19 잘못된 정보를 보여주므로 Line에서 삭제함 (Journal Local Time이 있으므로)
	//  5 Line
//	strTemp.Format(L"^START TIME(LAST)  = %s %2.2s:%2.2s:%2.2s", 
//						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
//						arJnlField[6], arJnlField[7], arJnlField[8]);
//	strPrintData += strTemp;
	// end of KSK 2009.2.19
	
	//  6 Line
	strTemp.Format(L"^%-18.18s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	//  7 Line
	if (MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
		strTemp.Format(L"^%-18.18s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_002), MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID));	// [#2160] CA KMK 2012.11.12 FrenchOP
	else
		strTemp.Format(L"^%-18.18s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_003), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));	// [#2160] CA KMK 2012.11.12 FrenchOP
	
	strPrintData += strTemp;
	
	//  8 Line
	strTemp.Format(L"^----------------------------------------");
	strPrintData += strTemp;
	
	//  9 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_004));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	// 10 Line
	strTemp.Format(L"^----------------------------------------");
	strPrintData += strTemp;
	
	// 11 Line
	strTemp.Format(L"^%-18.18s:  %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_005), nRemainCST1, nRemainCST2, nRemainCST3, nRemainCST4);	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	// 12 Line
	strTemp.Format(L"^%-18.18s:  %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_006), nAddCST1, nAddCST2, nAddCST3, nAddCST4);	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;	
	
	return strPrintData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertDayTotalToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : DAY TOTAL JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertDayTotalToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	
	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;
	
	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;
	
	// exception
	if (arJnlField.GetSize() < 29)
		return strPrintData;

	//  5 Line
	strTemp.Format(L"^%-18.18s=  %s %s",
						m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_007), // [#2160] CA KMK 2012.11.12 FrenchOP
						GetCmnLocalDate(arJnlField[9].Left(2), arJnlField[9].Mid(3,2), arJnlField[9].Mid(6,4)),
						arJnlField[9].Right(8));
	strPrintData += strTemp;
	
	//  6 Line
	strTemp.Format(L"^%-18.18s=  %s %s", 
						m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_008), // [#2160] CA KMK 2012.11.12 FrenchOP
						GetCmnLocalDate(arJnlField[10].Left(2), arJnlField[10].Mid(3,2), arJnlField[10].Mid(6,4)),
						arJnlField[10].Right(8));
	strPrintData += strTemp;
	
	//  7 Line
	// Journal Format중에 ADD CASH / DENOMINATION에 Terminal ID가 없어서 현재 TERMINAL ID로 SET하도록 함
	strTemp.Format(L"^%-18.18s= %-20.20s",  m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));		// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	//  8 Line
	strTemp = L"^";
	strPrintData += strTemp;
	
	//  9 Line
	strTemp.Format(L"^%s",  m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_009));			// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	// 10 Line
	// NHA 요청사항으로 Trial Day Total Fail시 처리 추가
	if (arJnlField[12].IsEmpty() == FALSE)
		strTemp.Format(L"^%-21.21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), Asc2Int(arJnlField[12]));	// [#2160] CA KMK 2012.11.12 FrenchOP
	else
		strTemp.Format(L"^%-21.21s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010));			// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	// 10 Line
	if (arJnlField[13].IsEmpty() == FALSE)
		strTemp.Format(L"^%-21.21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_011), Asc2Int(arJnlField[13]));	// [#2160] CA KMK 2012.11.12 FrenchOP
	else
		strTemp.Format(L"^%-21.21s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_011));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	// 11 Line
	if (arJnlField[14].IsEmpty() == FALSE)
		strTemp.Format(L"^%-21.21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_012), Asc2Int(arJnlField[14]));	// [#2160] CA KMK 2012.11.12 FrenchOP
	else
		strTemp.Format(L"^%-21.21s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_012));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	CString strTemp2;

	// 12 Line
	strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[16]));
	if (arJnlField[16].IsEmpty() == FALSE)
		strTemp.Format(L"^%-21.21s = %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);	// [#2160] CA KMK 2012.11.12 FrenchOP
	else
		strTemp.Format(L"^%-21.21s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	// 13 Line
	strTemp = L"^";
	strPrintData += strTemp;
	
	// 14 Line
	strTemp.Format(L"^%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_016));
	strPrintData += strTemp;
	
	// 15 Line		
	strTemp.Format(L"^%-21.21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), Asc2Int(arJnlField[19]));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	// 16 Line
	strTemp.Format(L"^%-21.21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_011), Asc2Int(arJnlField[20]));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	// 17 Line
	strTemp.Format(L"^%-21.21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_012), Asc2Int(arJnlField[21]));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	// 18 Line
	strTemp.Format(L"^%-21.21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_017), Asc2Int(arJnlField[23]));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	// 19 Line
	strTemp.Format(L"^%-21.21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_018), Asc2Int(arJnlField[24]));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	// 20 Line
	strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[25]));
	strTemp.Format(L"^%-21.21s = %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	// 21 Line
	strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[26]));
	strTemp.Format(L"^%-21.21s = %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_019), strTemp2);	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	// 22 Line
#if (AU_VERSION)
	strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[27]));
	strTemp.Format(L"^%-22s= %16.16s",  L"TOTAL ATM OPERATOR FEE", strTemp2);	// [#484] [NH] KSK 2009.1.14
#elif (MX_VERSION)
	// [#2115] MX KSK 2012.02.04
//	strTemp2.Format(L"%s%s", CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[27])); 
	strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[27])); // [#2147] MX KSK 2012.08.23 사양 변경에 따라 권종 정보를 사용하도록 수정
	strTemp.Format(L"^%-21s = %16.16s", L"AMOUNT OF ATM FEE", strTemp2 );
	// end of [#2115]
#else
	strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[27]));
	strTemp.Format(L"^%-21.21s = %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_020), strTemp2);	// [#2160] CA KMK 2012.11.12 FrenchOP
#endif
	strPrintData += strTemp;
	
	return strPrintData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertCstTotalToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : CST TOTAL JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertCstTotalToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nAmount, nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	
	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;
	
	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;
	
	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;
	
	// exception
	if (arJnlField.GetSize() < 38)
		return strPrintData;

	//  5 Line
	strTemp.Format(L"^%-18.18s= %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_007),	// [#2160] CA KMK 2012.11.12 FrenchOP
						GetCmnLocalDate(arJnlField[9].Left(2), arJnlField[9].Mid(3,2), arJnlField[9].Mid(6,4)),
						arJnlField[9].Right(8));
	strPrintData += strTemp;
	
	//  6 Line
	strTemp.Format(L"^%-18.18s= %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_008),	// [#2160] CA KMK 2012.11.12 FrenchOP
						GetCmnLocalDate(arJnlField[10].Left(2), arJnlField[10].Mid(3,2), arJnlField[10].Mid(6,4)),
						arJnlField[10].Right(8));
	strPrintData += strTemp;
	
	//  7 Line
	strTemp.Format(L"^%-18.18s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	//  8 Line
	if (MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
		strTemp.Format(L"^%-18.18s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_002), MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID));	// [#2160] CA KMK 2012.11.12 FrenchOP
	else
		strTemp.Format(L"^%-18.18s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_003), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));	// [#2160] CA KMK 2012.11.12 FrenchOP
	
	strPrintData += strTemp;
	
	//  9 Line
	strTemp.Format(L"^----------------------------------------");
	strPrintData += strTemp;
	
	// 10 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_004));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	// 11 Line
	strTemp.Format(L"^----------------------------------------");
	strPrintData += strTemp;

	// 12 Line	(DENOMINATION)
	strTemp.Format(L"^%-12.12s%s:  %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_023), GetCurrencySymbol(6,1), Asc2Int(arJnlField[17]), Asc2Int(arJnlField[23]),		// [#2160] CA KMK 2012.11.12 FrenchOP
				   Asc2Int(arJnlField[29]), Asc2Int(arJnlField[35]));
	strPrintData += strTemp;
	

	// [#2270] AU KSK 2014.05.26 AP에서 관리하는 매수정보를 표시하도록 추가 수정하여 매수정보 표시하도록 재수정
	// [#2241] AU KMK 2014.01.07 CDUM 논리매수 차감 Option 비활성화 시(값: -1) INITIAL COUNT, REMAINING COUNT를 저널에 출력하지 않는다
//	if (Asc2Int(arJnlField[12]) == -1)	// CST1 initial count가 -1인 경우 (CST2~CST4도 -1이므로 확인 필요 없음)
//	{
//		// initial/remaining count 미출력
//	}
//	else
//	{
	// 13 Line	(INITIAL COUNT)
	strTemp.Format(L"^%-18.18s:  %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_024), Asc2Int(arJnlField[12]), Asc2Int(arJnlField[18]), 
		Asc2Int(arJnlField[24]), Asc2Int(arJnlField[30]));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	// 14 Line	(REMAINING COUNT)
	strTemp.Format(L"^%-18.18s:  %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_025), Asc2Int(arJnlField[16]), Asc2Int(arJnlField[22]), 
		Asc2Int(arJnlField[28]), Asc2Int(arJnlField[34]));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
//	}
	// end of [#2241]
	// end of [#2270]
		
	// 15 Line	'REJECTED'
	// [#468] NZ AIREAT 2008.12.19
	strTemp.Format(L"^%-18.18s:  %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_026), Asc2Int(arJnlField[14]), Asc2Int(arJnlField[20]), 
															 Asc2Int(arJnlField[26]), Asc2Int(arJnlField[32]));	// [#2160] CA KMK 2012.11.12 FrenchOP
	// end of [#468]
	strPrintData += strTemp;

	// 16 Line	'JAMMED'
	// [#2247] AU KMK 2014.01.08 CDUM 논리매수 차감 Option 비활성화 시 'JAMMED' 항목 출력 안함
	if (Asc2Int(arJnlField[12]) == -1)	// 논리매수 차감 Option 비활성화 = CST1 initial count가 -1
	{
		// JAMMED 미출력
	}
	else
	{
	// end of [#2247]
		strTemp.Format(L"^%-18.18s:  %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_027), Asc2Int(arJnlField[15]), Asc2Int(arJnlField[21]), 
			Asc2Int(arJnlField[27]), Asc2Int(arJnlField[33]));	// [#2160] CA KMK 2012.11.12 FrenchOP
		strPrintData += strTemp;

	// [#2247]
	}
	// end of [#2247]
	
	// 17 Line	'DISPENSED'
	strTemp.Format(L"^%-18.18s:  %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_028), Asc2Int(arJnlField[13]), Asc2Int(arJnlField[19]), 
															 Asc2Int(arJnlField[25]), Asc2Int(arJnlField[31]));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	// 18 Line
	strTemp.Format(L"^----------------------------------------");
	strPrintData += strTemp;

	// [#2270] AU KSK 2014.05.26 AP에서 관리하는 매수정보를 표시하도록 추가 수정하여 매수정보 표시하도록 재수정
	// [#2241] AU KMK 2014.01.07 CDUM 논리매수 차감 Option 비활성화 시 INITIAL AMOUNT, REMAINING AMOUNT를 저널에 출력하지 않는다
//	if (Asc2Int(arJnlField[12]) == -1)	// CST1 initial count가 -1인 경우 (CST2~CST4도 -1이므로 확인 필요 없음)
//	{
//		// initial/remaining amount 미출력
//	}
//	else
//	{
	// 19 Line	'INITIAL AMOUNT'
	nAmount = (Asc2Int(arJnlField[17]) * Asc2Int(arJnlField[12])) + (Asc2Int(arJnlField[23]) * Asc2Int(arJnlField[18])) +
			  (Asc2Int(arJnlField[29]) * Asc2Int(arJnlField[24])) + (Asc2Int(arJnlField[35]) * Asc2Int(arJnlField[30]));
	nAmount *= 100;
	strTemp.Format(L"%d", nAmount);

	CString strTemp2;
	strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTemp));
	strTemp.Format(L"^%-18.18s= %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_029), strTemp2);	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;


	// 20 Line	'REMAINING AMOUNT'
	nAmount = (Asc2Int(arJnlField[17]) * Asc2Int(arJnlField[16])) + (Asc2Int(arJnlField[23]) * Asc2Int(arJnlField[22])) +
			  (Asc2Int(arJnlField[29]) * Asc2Int(arJnlField[28])) + (Asc2Int(arJnlField[35]) * Asc2Int(arJnlField[34]));
	nAmount *= 100;
	strTemp.Format(L"%d", nAmount);

	strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTemp));
	strTemp.Format(L"^%-18.18s= %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_030), strTemp2);	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
//	}
	// end of [#2241]
	// end of [#2270]
	
	return strPrintData;
}

// [#GLDV-3005] US Kook 2022.04.14 support side car
/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertCashInTotalToPrint_Demo()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : CASHIN TOTAL JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertCashInTotalToPrint_Demo(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nAmount, nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	
	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;
	
	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription));
	strPrintData += strTemp;
	
	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;
	
	// exception
	if (arJnlField.GetSize() < 14)
		return strPrintData;

	//  5 Line
	strTemp.Format(L"^%-18.18s= %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_007),
						GetCmnLocalDate(arJnlField[9].Left(2), arJnlField[9].Mid(3,2), arJnlField[9].Mid(6,4)),
						arJnlField[9].Right(8));
	strPrintData += strTemp;
	
	//  6 Line
	strTemp.Format(L"^%-18.18s= %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_008),
						GetCmnLocalDate(arJnlField[10].Left(2), arJnlField[10].Mid(3,2), arJnlField[10].Mid(6,4)),
						arJnlField[10].Right(8));
	strPrintData += strTemp;
	
	//  7 Line
	arJnlField[11].TrimLeft();
	arJnlField[11].TrimRight();
	strTemp.Format(L"^%-18.18s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), arJnlField[11]);
	strPrintData += strTemp;
	
	//  8 Line
	if (MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
		strTemp.Format(L"^%-18.18s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_002), MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID));
	else
		strTemp.Format(L"^%-18.18s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_003), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));
	strPrintData += strTemp;
	
	//  9 Line
	strTemp.Format(L"^----------------------------------------");
	strPrintData += strTemp;
	
	// 10 Line
	strTemp.Format(L"^%-12.12s%-6.6s: %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_023), GetCurrencySymbol(6, 1), m_pDevCmn->fstrSCR_GetStringFromTextID(T_COUNT));
	strPrintData += strTemp;
	
	// TODO: value/count, value/count, value/count, ...
	// 5|0, 10|1, 20|0, 50|0,...,
	arJnlField[12].TrimRight(_T(","));
	CStringArray strCashInValueCount;
	SplitString(arJnlField[12], ',', strCashInValueCount);

	for (int i=0; i<strCashInValueCount.GetSize(); i++)
	{
		CStringArray strValueCount;
		SplitString(strCashInValueCount.GetAt(i), '|', strValueCount);

		if (strValueCount.GetSize() == 2)
		{
			strTemp.Format(L"^%17s : %20s", strValueCount.GetAt(0), strValueCount.GetAt(1));
			strPrintData += strTemp;
		}
	}

	// 11 Line
	strTemp.Format(L"^----------------------------------------");
	strPrintData += strTemp;

	// 19 Line	'TOTAL AMOUNT'
	nAmount = Asc2Int(arJnlField[13]) * 100;
	strTemp.Format(L"%d", nAmount);

	CString strTemp2;
	strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTemp));
	strTemp.Format(L"^%-18.18s= %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_012), strTemp2);
	strPrintData += strTemp;
	
	return strPrintData;
}

// end of [#GLDV-3005]


// [#2385] US Justin 2015.01.12 Canadian Local Loan
CString CJnlMgr::ConvertBriefDayTotalToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp, strTemp2, strTemp3;
	int		nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	
	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription));
	strPrintData += strTemp;
	
	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;
	
	// exception
	if (arJnlField.GetSize() < 14)
		return strPrintData;

	//  5 Line (Start Time)
	strTemp.Format(L"^%-18.18s=  %s %s",
						m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_007),
						GetCmnLocalDate(arJnlField[9].Left(2), arJnlField[9].Mid(3,2), arJnlField[9].Mid(6,4)),
						arJnlField[9].Right(8));
	strPrintData += strTemp;
	
	//  6 Line (End Time)
	strTemp.Format(L"^%-18.18s=  %s %s", 
						m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_008),
						GetCmnLocalDate(arJnlField[10].Left(2), arJnlField[10].Mid(3,2), arJnlField[10].Mid(6,4)),
						arJnlField[10].Right(8));
	strPrintData += strTemp;
	
	//  7 Line (Terminal ID)
	arJnlField[11].TrimLeft();
	arJnlField[11].TrimRight();
	strTemp.Format(L"^%-18.18s= %-20.20s",  m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001),  arJnlField[11]);
	strPrintData += strTemp;
	
	//  8 Line
	strTemp = L"^";
	strPrintData += strTemp;

	//  9 Line (Count of withdrawal)
	// [#2442] US Justin 2016.08.25 Add Day Total for PopMoney
	// strTemp.Format(L"^%-21.21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), Asc2Int(arJnlField[12]));
	strTemp3 = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010);		// COUNT OF WITHDRAWAL
	strTemp.Format(L"^%-21.21s = %16d", strTemp3, Asc2Int(arJnlField[12]));
	// End of [#2442]
	strPrintData += strTemp;
	
	// 10 Line (Amount of Dispense)
	strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[13]));
	// [#2442] US Justin 2016.08.25 Add Day Total for PopMoney
	//strTemp.Format(L"^%-21.21s = %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);
	strTemp3 = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014);		// AMOUNT OF DISPENSE
	strTemp.Format(L"^%-21.21s = %16.16s", strTemp3, strTemp2);
	// End of [#2442]
	strPrintData += strTemp;

	return strPrintData;	
}

CString CJnlMgr::ConvertBriefCstTotalToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	
	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;
	
	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription));
	strPrintData += strTemp;
	
	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;
	
	// exception
	if (arJnlField.GetSize() < 16)
		return strPrintData;

	//  5 Line (Start Time)
	strTemp.Format(L"^%-18.18s= %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_007),
						GetCmnLocalDate(arJnlField[9].Left(2), arJnlField[9].Mid(3,2), arJnlField[9].Mid(6,4)),
						arJnlField[9].Right(8));
	strPrintData += strTemp;
	
	//  6 Line (End Time)
	strTemp.Format(L"^%-18.18s= %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_008),
						GetCmnLocalDate(arJnlField[10].Left(2), arJnlField[10].Mid(3,2), arJnlField[10].Mid(6,4)),
						arJnlField[10].Right(8));
	strPrintData += strTemp;
	
	//  7 Line (Terminal ID)
	arJnlField[11].TrimLeft();
	arJnlField[11].TrimRight();
	strTemp.Format(L"^%-18.18s= %-20.20s",  m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001),  arJnlField[11]);
	strPrintData += strTemp;
	
	//  9 Line
	strTemp.Format(L"^----------------------------------------");
	strPrintData += strTemp;
	
	// 10 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_004));
	strPrintData += strTemp;
	
	// 11 Line
	strTemp.Format(L"^----------------------------------------");
	strPrintData += strTemp;

	// 12 Line	'DISPENSED'
	strTemp.Format(L"^%-18.18s:  %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_028), Asc2Int(arJnlField[12]), Asc2Int(arJnlField[13]), 
															 Asc2Int(arJnlField[14]), Asc2Int(arJnlField[15]));
	strPrintData += strTemp;
	
	// 13 Line
	strTemp.Format(L"^----------------------------------------");
	strPrintData += strTemp;
	
	return strPrintData;
}
// End of [#2385]

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertDenoToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : CHANGE DENOMINATION JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertDenoToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int	nOldDenoCST1 = 0, nOldDenoCST2 = 0, nOldDenoCST3 = 0, nOldDenoCST4 = 0;
	int nNewDenoCST1 = 0, nNewDenoCST2 = 0, nNewDenoCST3 = 0, nNewDenoCST4 = 0;
	int nJnlField, nJnlIndex;
	
	nJnlIndex = Asc2Int(arJnlField[2]);

	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;
	
	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	// [#332] NH AIREAT 2008.06.24

	nJnlField = arJnlField.GetSize();
		
	// Get CST1 Value
	if (nJnlField >= 11)
	{
		nOldDenoCST1 = Asc2Int(arJnlField[9]);
		nNewDenoCST1 = Asc2Int(arJnlField[10]);
	}
	
	// Get CST2 Value
	if (nJnlField >= 13)
	{
		nOldDenoCST2 = Asc2Int(arJnlField[11]);
		nNewDenoCST2 = Asc2Int(arJnlField[12]);
	}
	
	// Get CST3 Value
	if (nJnlField >= 15)
	{
		nOldDenoCST3 = Asc2Int(arJnlField[13]);
		nNewDenoCST3 = Asc2Int(arJnlField[14]);
	}
	
	// Get CST4 Value
	if (nJnlField >= 17)
	{
		nOldDenoCST4 = Asc2Int(arJnlField[15]);	// [#578] KSK 2009.11.09 Bug Fix
		nNewDenoCST4 = Asc2Int(arJnlField[16]);
	}
	// end of [#332]
	
	// KSK 2009.2.19 잘못된 정보를 보여주므로 Line에서 삭제함 (Journal Local Time이 있으므로)
	//  5 Line
//	strTemp.Format(L"^START TIME(LAST)   = %s %2.2s:%2.2s:%2.2s", 
//						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
//						arJnlField[6], arJnlField[7], arJnlField[8]);
//	strPrintData += strTemp;
	// end of KSK 2009.2.19
	
	//  6 Line
	strTemp.Format(L"^%-19.19s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	//  7 Line
	if (MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
		strTemp.Format(L"^%-19.19s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_002), MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID));	// [#2160] CA KMK 2012.11.12 FrenchOP
	else
		strTemp.Format(L"^%-19.19s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_003), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));	// [#2160] CA KMK 2012.11.12 FrenchOP
	
	strPrintData += strTemp;
	
	//  8 Line
	strTemp.Format(L"^----------------------------------------");
	strPrintData += strTemp;
	
	//  9 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_004));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	// 10 Line
	strTemp.Format(L"^----------------------------------------");
	strPrintData += strTemp;

	// 11 Line
	strTemp.Format(L"^%-13.13s%-6.6s: %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_031), GetCurrencySymbol(6,1), nOldDenoCST1, nOldDenoCST2, nOldDenoCST3, nOldDenoCST4);	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	// 12 Line
	strTemp.Format(L"^%-13.13s%-6.6s: %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_032), GetCurrencySymbol(6,1), nNewDenoCST1, nNewDenoCST2, nNewDenoCST3, nNewDenoCST4);	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	return strPrintData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertCommMsgToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : COMMUNICATION MESSAGE JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertCommMsgToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	
	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;
	
	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	// exception
	if (arJnlField.GetSize() < 11)
		return strPrintData;
	
	//  5 Line
	strTemp.Format(L"^%-20.20s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	//  6 Line
	strTemp.Format(L"^%-20.20s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_033), m_pDevCmn->fstrSCR_GetStringFromTextID(AP_TEXT_ID + arJnlField[9].Trim()));	// [#2160] CA KMK 2012.12.21 FrenchOP
	strPrintData += strTemp;
	
	//  7 Line
	strTemp.Format(L"^%-20.20s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_034), m_pDevCmn->fstrSCR_GetStringFromTextID(AP_TEXT_ID + arJnlField[10].Trim()));	// [#2160] CA KMK 2012.12.21 FrenchOP
	strPrintData += strTemp;
	
	return strPrintData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertTranToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : TRANSACTION JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertTranToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp, strTemp2;
	int		nSeqIndex, nJnlIndex;
	int		nSurchargeAmount, nRequestAmount, nDispensAmount;

	nJnlIndex = Asc2Int(arJnlField[2]);
	nSeqIndex = Asc2Int(arJnlField[10]);

	nSurchargeAmount = Asc2Int(arJnlField[23]);
	nRequestAmount   = Asc2Int(arJnlField[24]);
	nDispensAmount   = Asc2Int(arJnlField[25]);

#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
	if (arJnlField[1] == TRX_ERROR || arJnlField[1] == REVERSAL_TRX || 
		arJnlField[1] == TRX_NOT_APPROVED || arJnlField[1] == COMMUNICATION_ERROR)
		nSurchargeAmount = 0;
#endif

	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS T#:XXXX J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s  T#:%04.4d J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nSeqIndex, nJnlIndex);
	strPrintData += strTemp;

	//  2 Line
#if !(MX_VERSION)	// [#2115] MX KSK 2012.02.04 이중장애 때문에 Line 부족으로 인해 여백 줄임
	strTemp.Format(L"^");
	strPrintData += strTemp;
#endif
	
	//  3 Line
	strTemp.Format(L"^%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;
	
	//  4 Line
#if !(MX_VERSION)	// [#2115] MX KSK 2012.02.04 이중장애 때문에 Line 부족으로 인해 여백 줄임
	strTemp.Format(L"^");
	strPrintData += strTemp;
#endif

	// exception
	if (arJnlField.GetSize() < 35)
		return strPrintData;

	//  5 Line => Terminal #
	// Journal Format중에 ADD CASH / DENOMINATION에 Terminal ID가 없어서 현재 TERMINAL ID로 SET하도록 함
	strTemp.Format(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	//  6 Line => Sequence #
	strTemp.Format(L"^%-17.17s = %04d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_035), nSeqIndex);	// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	// 7 Line => Auth #
	if (arJnlField[11] != TRANTYPE_B4U)		// no Auth # for B4U
	{
		if (Asc2Int(arJnlField[20]) != 0)
			strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_036), arJnlField[20]);	// [#2160] CA KMK 2012.11.12 FrenchOP
		else
			strTemp.Format(L"^%-17.17s = ", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_036));	// [#2160] CA KMK 2012.11.12 FrenchOP
		strPrintData += strTemp;
	}

	//  8 Line => Transaction Date and Time
	// 값이 잘못 되어 있으면 Print 하지 않는다.
	arJnlField[16].TrimLeft();
	arJnlField[17].TrimLeft();
	if ((arJnlField[16].GetLength() >= 6) && (arJnlField[17].GetLength() == 6) && (arJnlField[16].Right(2) != L"00"))
	{
		strTemp.Format(L"^%-17.17s = %s %2.2s:%2.2s:%2.2s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_037),	// [#2160] CA KMK 2012.11.12 FrenchOP
							GetCmnLocalDate(arJnlField[16].Left(2), arJnlField[16].Mid(2,2), arJnlField[16].Right(2)),
							arJnlField[17].Left(2), arJnlField[17].Mid(2,2), arJnlField[17].Right(2));
	}
	else
	{
		strTemp.Format(L"^%-17.17s = ", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_037));	// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	strPrintData += strTemp;

	//  9 Line => Business Date
	arJnlField[22].TrimLeft();
	if ((arJnlField[22].GetLength() >= 6) && (arJnlField[22].Right(2) != L"00"))
	{
		strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_038),	// [#2160] CA KMK 2012.11.12 FrenchOP
							GetCmnLocalDate(arJnlField[22].Left(2), arJnlField[22].Mid(2,2), arJnlField[22].Right(2)));
		strPrintData += strTemp;
	}
#if (AU_VERSION)
	// 호주 FDI는 Business Date를 사용하지 않음. '000000'으로 내려줌. --> 인자 하지 않음.
	else if (arJnlField[22].Left(6) == L"000000" || arJnlField[22].Left(8) == L"01012000")
	{
		strTemp = L"";
	}
#endif
	else
	{
		strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_038));	// [#2160] CA KMK 2012.11.12 FrenchOP
		strPrintData += strTemp;
	}

	// 10 Line => Card Number
	if (arJnlField[11] == TRANTYPE_POPMONEY)																		// [#2350] US Justin 2015.06.19 Add POP Money
		strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_094), arJnlField[30]);
	else if (arJnlField[11] == TRANTYPE_JUSTCASH)																		// [#2445] US Justin 2016.09.28 Add just.cash
		strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_118), arJnlField[30]);	
	else 
		strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_040), arJnlField[30]);
	strPrintData += strTemp;

	// 11 Line => Transaction Type
	if (arJnlField[11] == TRANTYPE_WITHDRAWAL)
	{
		strTemp.Format(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_041), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_042));	// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	else if (arJnlField[11] == TRANTYPE_INQUIRY)	
	{
#if (AU_VERSION) //[#0542] SOOK 2009.06.23 CUSTOMERS 요청으로 Inquiry -> Enquiry로 변경 함 [#2041] AU KSK 2011.03.31
		strTemp.Format(L"^%-17.17s = %-20.20s", L"TRANSACTION", L"BALANCE ENQUIRY");
#else
		strTemp.Format(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_041), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_043));
#endif
	}
	else if (arJnlField[11] == TRANTYPE_TRANSFER)
	{
		strTemp.Format(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_041), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_044));	// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	else if (arJnlField[11] == TRANTYPE_NONCASH_WITHDRAW)
	{
		strTemp.Format(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_041), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_046));	// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	// [#2150] US Justin 2012.10.08 Add "Pin Change" and DCC on Journal Print
	else if (arJnlField[11] == TRANTYPE_PINCHANGE)
	{
		strTemp.Format(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_041), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_047));	// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	else if (arJnlField[11] == TRANTYPE_DCC)
	{
		strTemp.Format(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_041), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_048));	// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	// End of [#2150]
	// [#2405] US Justin 2016.03.21 Hal Cash Online
	else if (arJnlField[11] == TRANTYPE_PIN4)
	{
		strTemp.Format(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_041), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_099));
	}
	// End of [#2405]
	// [#2445] US Justin 2016.09.28 Just.Cash
	else if (arJnlField[11] == TRANTYPE_JUSTCASH)
	{
		strTemp.Format(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_041), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_117));
	}
	// End of [#2445]
	// [#2446] US Justin 2016.09.29 Paypal CCA
	else if (arJnlField[11] == TRANTYPE_PAYPAL)
	{
		strTemp.Format(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_041), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_119));
	}
	// End of [#2446]
	// [#2535] US Justin 2018.04.24 GivePay
	else if (arJnlField[11] == TRANTYPE_GIVEPAY)
	{
		strTemp.Format(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_041), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_121));
	}
	// End of [#2535]
	else
	{
		strTemp.Format(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_041), L"--");
	}
	strPrintData += strTemp;

	// 12 Line Account
	// [#2150] US Justin 2012.10.08 Not Printing account for "Pin Change"
	// [#2405] Not Printing account for  HalCash
	// [#2445] Not Printing account for  just.cash
	// [#2535] Not Printing account for  GivePay
	if( (arJnlField[11] != TRANTYPE_PINCHANGE) && 
		(arJnlField[11] != TRANTYPE_PIN4) && 
		(arJnlField[11] != TRANTYPE_JUSTCASH)  && 
		(arJnlField[11] != TRANTYPE_GIVEPAY) )
	{
		CString strFromAcount;
		CString strToAcount;

		// from account
		// [#240] NH 2008.05.28 국가별 Checking Account 표현
		if (arJnlField[12] == L"CA")		strFromAcount = S_CHECKING;
		else if (arJnlField[12] == L"SA")	strFromAcount = S_SAVINGS;
		else if (arJnlField[12] == L"CR")	strFromAcount = L"CREDIT";
		else								strFromAcount = L"--";

		// to account
		if (arJnlField[13] == L"CA")		strToAcount = S_CHECKING;
		else if (arJnlField[13] == L"SA")	strToAcount = S_SAVINGS;
		else if (arJnlField[13] == L"CR")	strToAcount = L"CREDIT";
		// end of [#240]
		else								strToAcount = L"--";

		if (arJnlField[11] == TRANTYPE_TRANSFER)
		{
			strTemp.Format(L"^%-17.17s = %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_050), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_049), strFromAcount);	// [#2160] CA KMK 2012.11.12 FrenchOP
			strPrintData += strTemp;
			strTemp.Format(L"^%-19.19s %s %s", L"", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_051), strToAcount);	// [#2160] CA KMK 2012.11.12 FrenchOP
			strPrintData += strTemp;
		}
		else
		{
			strTemp.Format(L"^%-17.17s = %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_050), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_049), strFromAcount);	// [#2160] CA KMK 2012.11.12 FrenchOP
			strPrintData += strTemp;
		}
	}

	// 13 Line
	strTemp.Format(L"^----------------------------------------");
	strPrintData += strTemp;

	// Other Message FORMAT
	// Sepated by "UNIT_DELIMITER (1F)"
		// CDU_OVER_DISPENSE(O) + 1F + "CDU OVER DISPENSE TEXT" follows next	(2 UNITs)
		// PROC COUNT + 1F + ERROR CODE + 1F + ERROR TEXT						(3 UNITs)
		// ~(RMS_OTHERMSG_DELIMITER) + "RMS OTHER MSG"(Indicator(1byte:"A"B,C,D,E,....) + Value)
	
	int		nCustomerType = 0;																																			// Mexico
	CString strOtherErrorMsg, strOtherErrorCode, strOtherProcCount, strDispensedCount, strRejectedCount, strGeneralComment;												// COMMON
	CString	strJNLCurrencyID, strJNLExchangeRate, strBankName, strLoyaltyFee, strLineUsageFee, strIVATax, strReturnedAccountNumber, strExchangeFee, strTotalFee;		// Mexico
	CString strDomesticBinFallBack;																																		// AU
	CString strDCCERate, strDCCCurrency, strDCCConvtedAmt, strDCCConvtedScg, strDCCOfferAccept, strDCCQuoteType;														// US, CA
	CString strPin4PhoneNumber, strPin4Pin4Code, strPin4SecretCode;																						// US	

	strOtherErrorMsg = strOtherErrorCode = strOtherProcCount = strDispensedCount = strRejectedCount = strGeneralComment = L"";	
	strJNLCurrencyID = strJNLExchangeRate = strBankName = strLoyaltyFee = strLineUsageFee = strIVATax = strReturnedAccountNumber = strExchangeFee = strTotalFee = L"";
	strDomesticBinFallBack = strDCCERate = strDCCCurrency = strDCCConvtedAmt = strDCCConvtedScg = strDCCOfferAccept = strDCCQuoteType = L"";
	strPin4PhoneNumber = strPin4Pin4Code = strPin4SecretCode = L"";

	if (arJnlField[33].GetLength())
	{
		CStringArray	strOtherMsgArray;
		SplitString(arJnlField[33], UNIT_DELIMITER, strOtherMsgArray);

		for(int i=0; i<strOtherMsgArray.GetSize(); i++)
		{
			if (strOtherMsgArray[i].GetLength())
			{
				if (strOtherMsgArray[i].Left(1) == RMS_OTHERMSG_DELIMITER)
				{
					CStringArray	strExtendInfoArray;
					SplitString(strOtherMsgArray[i], RMS_OTHERMSG_DELIMITER, strExtendInfoArray);

					for (int j=0; j<strExtendInfoArray.GetSize(); j++)
					{
						if (strExtendInfoArray[j].GetLength() > 0)
						{
							CString strOtherMsgValue = strExtendInfoArray[j].Mid(1);

							if		(strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_CURRENCYID)				strJNLCurrencyID			= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_EXCHANGERATE)			strJNLExchangeRate			= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_DISPENSED_COUNT)			strDispensedCount			= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_REJECTED_COUNT)			strRejectedCount			= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_BANKNAME)				strBankName					= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_LOYALTY_FEE)				strLoyaltyFee				= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_LINEUSAGE_FEE)			strLineUsageFee				= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_IVA_TAX)					strIVATax					= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_RETURNED_ACCNT_NO)		strReturnedAccountNumber	= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_EXCHANGE_FEE)			strExchangeFee				= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_TOTAL_FEE)				strTotalFee					= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_CUSTOMERTYPE)			nCustomerType				= Asc2Int(strOtherMsgValue);
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_AU_DOMESTICBIN_FALLBACK)	strDomesticBinFallBack		= strOtherMsgValue;	// [#2379] AU KSK 2015.11.17
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_DCC_EXCHANGERATE)		strDCCERate					= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_DCC_CURRENCY)			strDCCCurrency				= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_DCC_CONVERTEDAMT)		strDCCConvtedAmt			= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_DCC_CONVERTEDSCG)		strDCCConvtedScg			= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_DCC_OFFERACCEPT)			strDCCOfferAccept			= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_DCC_QUOTETYPE)			strDCCQuoteType				= strOtherMsgValue;
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_PIN4_PHONENUMBER)		strPin4PhoneNumber			= strOtherMsgValue;	
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_PIN4_PIN4CODE)			strPin4Pin4Code				= strOtherMsgValue;	
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_PIN4_SECRETCODE)			strPin4SecretCode			= strOtherMsgValue;	
							else if (strExtendInfoArray[j].Left(1) == RMS_OTHERMSG_FID_GENERAL_COMMENT)			strGeneralComment			= strOtherMsgValue;	// [#2496] US Justin							
						}
					}
					// end of [#2115]
				}
				else if (strOtherMsgArray[i].Left(1) == CDU_OVER_DISPENSE)		// CDU Over Dispense  (2 UNITs)
				{					
					if (strOtherMsgArray.GetSize() >= i+2)
					{
						strOtherErrorMsg = strOtherMsgArray[i+1];
						i++;
					}
				}
				else															// Error Code ( 3 UNITs)										
				{					
					if (strOtherMsgArray.GetSize() >= i+2)
					{
						strOtherProcCount = strOtherMsgArray[i];
						strOtherErrorCode = strOtherMsgArray[i+1];
						i++;
					}
				}
			}
		}
	}
	// End of [#2460]

	// Line 14, 15 : Dispensed and Requested Amounts
	// [#2150] US Justin 2012.10.09 Add DCC
	// [#2350] US Justin 2015.06.19 Add POP Money
	// [#2405] US Justin 2016.03.21 Add HalCash
	// [#2445] US Justin 2016.09.28 Add Just.Cash
	// [#2446] US Justin 2016.09.29 Add Paypal CCA
	// [#2496] US Justin 2017.08.17 Add Just.Cash Bit Coin
	// Added B4U
	if( arJnlField[11] == TRANTYPE_WITHDRAWAL	|| arJnlField[11] == TRANTYPE_DCC ||		
		arJnlField[11] == TRANTYPE_POPMONEY		|| arJnlField[11] == TRANTYPE_PIN4 ||
		arJnlField[11] == TRANTYPE_JUSTCASH		|| arJnlField[11] == TRANTYPE_PAYPAL ||
		arJnlField[11] == TRANTYPE_NONCASH_WITHDRAW )								
	{
		// 14 Line Dispensed Amount
		strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[25]));
		strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_052), strTemp2);	// [#2160] CA KMK 2012.11.12 FrenchOP
		strPrintData += strTemp;

		// 15 Line Requested Amount
		strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[24]));
		strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_053), strTemp2);	// [#2160] CA KMK 2012.11.12 FrenchOP
		strPrintData += strTemp;

		#if (MX_VERSION)	// [#2115] [MX] KSK 2012.02.04
			if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
			{
				if (!strJNLExchangeRate.IsEmpty())
				{
					if (nCustomerType == MX_DOMESTIC_TYPE)
					{
						// 16 Line
						strTemp2.Format(L"%s%s", CURRENCY_SYMBOL, strJNLExchangeRate);
						strTemp.Format(  L"^EXCHANGE RATE     = %20.20s", strTemp2 );
						strPrintData += strTemp;
					}
				}
			}
		#endif				// end of [#2115]

		// [#2460] NH Justin 2016.12.14 Add Dispense result (Remove Country restriction)		
		// #if (AU_VERSION || MX_VERSION)		// [#2115] MX KSK 2012.02.04	
		if (!strDispensedCount.IsEmpty())
		{
			strTemp.Format(L"^%-17.17s = %20.20s", L"DISPENSED COUNT", strDispensedCount);
			strPrintData += strTemp;
		}
		if (!strRejectedCount.IsEmpty())
		{
			strTemp.Format(L"^%-17.17s = %20.20s", L"REJECTED COUNT", strRejectedCount);
			strPrintData += strTemp;
		}
		// #endif
	}
	else if (arJnlField[11] == TRANTYPE_TRANSFER)
	{
		// 14 Line

		// 15 Line
		strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[24]));
		strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_054), strTemp2);	// [#2160] CA KMK 2012.11.12 FrenchOP
		strPrintData += strTemp;
	}
	#if (US_VERSION || CA_VERSION)
	// [#2535] US Justin 2018.04.24 GivePay
	else if (arJnlField[11] == TRANTYPE_GIVEPAY)
	{
		// 14 Line

		// 15 Line Requested Amount
		strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[24]));
		strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_053), strTemp2);
		strPrintData += strTemp;
	}
	// End of [#2535]
	#endif	
	// End of [#2289]

	arJnlField[27].TrimLeft();
	arJnlField[28].TrimLeft();
#if (MX_VERSION)
	// [#2147] KSK 2012.08.17 WINCE5.0과 로직 동일하게 수정
	if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
	{
		if (nCustomerType == MX_INTERNATIONAL_TYPE)
			nSurchargeAmount = Asc2Int(strLineUsageFee);
	}
	// end of [#2147]

	if (nSurchargeAmount != 0)	// [#2115] MX KSK 2012.02.04 장애가 발생하더라도 Host에서 받은 Surcharge or Local Surcharge를 Journal에 저장함 (사양 협의 완료 2010.03.31)

#elif (US_VERSION)
	BOOL bPrintSurcharge = FALSE;
	if( (nSurchargeAmount != 0)&&(arJnlField[28]==L"TRUE")&&(arJnlField[27]!=L"3") )
	{
		if( (arJnlField[11]==TRANTYPE_INQUIRY)||(arJnlField[11]==TRANTYPE_PINCHANGE) )
			bPrintSurcharge = TRUE;
		else if(nDispensAmount != 0)
			bPrintSurcharge = TRUE;
	}

	if(bPrintSurcharge)

#else
	if ((nSurchargeAmount != 0) && (nDispensAmount != 0) && 
		(arJnlField[28] == L"TRUE") && (arJnlField[27] != L"3"))
	// End of [#2150]

#endif
	{
#if (AU_VERSION)
		// 16 Line
		strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[23]));
		strTemp.Format(L"^%-17.17s = %20.20s", L"ATM OPERATOR FEE", strTemp2);
#elif (MX_VERSION)
		// [#2147] MX KSK 2012.08.17 위에서 계산한 Surcharge Amount값을 출력하도록 수정
		// [#2137] MX KSK 2012.07.25
		if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
		{
			if (nCustomerType == MX_DOMESTIC_TYPE)
				strTemp2.Format(L"%s%s", CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nSurchargeAmount)));
			else
				strTemp2.Format(L"%s%s", MULTI_CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nSurchargeAmount)));
		}
		else
		{
			strTemp2.Format(L"%s%s", CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nSurchargeAmount)));
		}
		strTemp.Format(L"^ATM FEE           = %20.20s", strTemp2 );
		// end of [#2137]
		// end of [#2147]
#else
		// 16 Line
		strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[23]));
		strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_055), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
#endif
		strPrintData += strTemp;
	}
#if (AU_VERSION)
	else if ((arJnlField[11] == TRANTYPE_INQUIRY) && (nSurchargeAmount != 0))
	{
		// 16 Line
		strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[23]));
		strTemp.Format(L"^%-17.17s = %20.20s", L"ATM OPERATOR FEE", strTemp2);
		strPrintData += strTemp;
	}
#endif

#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
	// [#2137] MX KSK 2012.07.25
	if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
	{
		if (nCustomerType == MX_DOMESTIC_TYPE)
		{
			if (!strIVATax.IsEmpty())
			{
				strTemp2.Format(L"%s%s", CURRENCY_SYMBOL,strIVATax);
				strTemp.Format(L"^TAX               = %20.20s", strTemp2);
				strPrintData += strTemp;
			}
		}
		else
		{
			// International Card는 인자안함
		}
	}
	else
	{
		if (!strIVATax.IsEmpty())
		{
			strTemp2.Format(L"%s%s", CURRENCY_SYMBOL,strIVATax);
			strTemp.Format(L"^TAX               = %20.20s", strTemp2);
			strPrintData += strTemp;
		}
	}

	// Exchange Fee 추가
	if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
	{
		if ( !strExchangeFee.IsEmpty() && (arJnlField[11] == TRANTYPE_WITHDRAWAL) )		// [#2139] MX KMK 2012.08.07
		{
			if (nCustomerType == MX_INTERNATIONAL_TYPE)
			{
				strTemp2.Format(L"%s%s", MULTI_CURRENCY_SYMBOL, strExchangeFee);
				strTemp.Format(L"^EXCHANGE FEE      = %20.20s", strTemp2);
				strPrintData += strTemp;
			}
			else
			{
				// Domestic Card 미인자
			}
		}
	}
	else
	{
		// Peso에서는 미인자
	}
#endif				
	// end of [#2115]

	arJnlField[29].TrimLeft();

	// Other Messages
	// [#2292] US Justin 2014.10.07
#if (US_VERSION || CA_VERSION || AU_VERSION)	// [#2362] AU KSK 2015.07.20
	if (arJnlField[11] == TRANTYPE_DCC)
	{
		if(!strDCCERate.IsEmpty())
		{
			strTemp.Format(L"^%-17.17s = %20.20s", L"DCC EXCHANGE RATE", strDCCERate);
			strPrintData += strTemp;
		}

		if(!strDCCCurrency.IsEmpty())
		{
			strTemp.Format(L"^%-17.17s = %20.20s", L"DCC CURRENCY", strDCCCurrency);
			strPrintData += strTemp;
		}

		if(!strDCCConvtedAmt.IsEmpty())
		{
			strTemp.Format(L"^%-17.17s = %20.20s", L"DCC CONVERTED AMT", strDCCConvtedAmt);
			strPrintData += strTemp;
		}

		if(!strDCCConvtedScg.IsEmpty())
		{
#if (AU_VERSION)	// [#2362] AU KSK 2015.07.20
			strTemp.Format(L"^%-17.17s = %20.20s", L"DCC CONVERTED FEE", strDCCConvtedScg);
#else
			strTemp.Format(L"^%-17.17s = %20.20s", L"DCC CONVERTED SCG", strDCCConvtedScg);
#endif				// end of [#2362]
			strPrintData += strTemp;
		}

		if(!strDCCOfferAccept.IsEmpty())
		{
			strTemp.Format(L"^%-17.17s = %20.20s", L"DCC OFFER ACCEPT", strDCCOfferAccept);
			strPrintData += strTemp;
		}

		if(!strDCCQuoteType.IsEmpty())
		{
			strTemp.Format(L"^%-17.17s = %20.20s", L"DCC QUOTE TYPE", strDCCQuoteType);
			strPrintData += strTemp;
		}
	}
#endif
	// End of [#2292]

	// [#2405] US Justin 2016.03.21 HalCash Online
#if (APP_PIN4_CASHPICKUP) // (APP_CUSTOM_PAI)	// [#2471] NH Justin 2017.02.01 Open Pin4 to all US Customers
	if(arJnlField[11] == TRANTYPE_PIN4)
	{
		if(!strPin4PhoneNumber.IsEmpty())
		{
			strTemp.Format(L"^%-17.17s = %20.20s", L"PHONE NUMBER", strPin4PhoneNumber);
			strPrintData += strTemp;
		}
		if(!strPin4Pin4Code.IsEmpty())
		{
			strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_120), strPin4Pin4Code);			// [#2470] US Justin 2017.02.01 Change Name. Pin4 => MC Cash Pickup
			strPrintData += strTemp;
		}
		if(!strPin4SecretCode.IsEmpty())
		{
			strTemp.Format(L"^%-17.17s = %20.20s", L"SECRET CDODE", strPin4SecretCode);
			strPrintData += strTemp;
		}
	}
#endif
	// End of [#2292]

	// [#2496] US Justin 2017.08.16
	if( !strGeneralComment.IsEmpty() )
	{
		strTemp.Format(L"^%s", strGeneralComment);
		strPrintData += strTemp;
	}
	// End of [#2496]

	// LEDGER / AVAIL BALANCE
	// Transaction 성공 유무와 관계없이 BALANCE 정보 인자. - 정보 저장 및 확인 용.
	// [#2150] US Justin 2012.10.09 Add DCC
	// [#2445] US Justin 2016.09.28 Add Just.Cash
	// [#2446] US Justin 2016.09.29 Add Paypal CCA
	// [#2496] US Justin 2017.08.17 Add Just.Cash Bitcoin
	if (arJnlField[11] == TRANTYPE_WITHDRAWAL || arJnlField[11] == TRANTYPE_INQUIRY  || arJnlField[11] == TRANTYPE_TRANSFER || 
		arJnlField[11] == TRANTYPE_DCC		  || arJnlField[11] == TRANTYPE_JUSTCASH || arJnlField[11] == TRANTYPE_PAYPAL ||
		arJnlField[11] == TRANTYPE_NONCASH_WITHDRAW )
	{
		int	nLedgerBalance, nAvailableBalance;
		
		arJnlField[26].TrimLeft();
		arJnlField[18].TrimLeft();

		nLedgerBalance = Asc2Int(arJnlField[26]);
		nAvailableBalance = Asc2Int(arJnlField[18]);
		
		//  Account BALANCE is SPACE
		if (arJnlField[26].GetLength() == 0)
		{
			// --> SPACE
			if (arJnlField[18].GetLength() == 0)
			{
				// [#346] [NH] KSK 2008.6.26	EPS PROTOCOL 누락됨
				if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) != MSG_TRITON_TYPE)
				{
					// 18 Line
					strTemp.Format(L"^%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_056));		// [#2160] CA KMK 2012.11.12 FrenchOP
					strPrintData += strTemp;
				}
				// end of [#346]
			}
			// --> 0
			else if (nAvailableBalance == 0)
			{
				if (arJnlField[11] == TRANTYPE_INQUIRY)
				{
					// 18 Line
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
					// [#2137] MX KSK 2012.07.25
					if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
					{
						// USD Dispense Mode
						if (nCustomerType == MX_DOMESTIC_TYPE)
							strTemp2.Format(L"%s0.00",  CURRENCY_SYMBOL);
						else
							strTemp2.Format(L"%s0.00",  MULTI_CURRENCY_SYMBOL);
					}
					else
					{
						// Pesos Dispense Mode
						strTemp2.Format(L"%s0.00",  CURRENCY_SYMBOL);
					}
					// end of [#2137]
#else
					strTemp2.Format(L"%s0.00", GetCurrencySymbol());
#endif				// end of [#2115]
					strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_057), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
					strPrintData += strTemp;
				}
			}
			// --> +, -
			else
			{
				// 18 Line
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
				// [#2137] MX KSK 2012.07.25
				if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
				{
					// USD Dispense Mode
					if (nCustomerType == MX_DOMESTIC_TYPE)
						strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[18]));
					else
						strTemp2.Format(L"%s%s",  MULTI_CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[18]));
				}
				else
				{
					// Peso Dispense Mode
					strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[18]));
				}
				// end of [#2137]
#else
				strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[18]));
#endif				// end of [#2115]
				strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_057), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
				strPrintData += strTemp;
			}
		}
		// LEDGER BALANCE is 0
		else if (nLedgerBalance == 0)
		{
			// --> SPACE
			if (arJnlField[18].GetLength() == 0)
			{
				if (arJnlField[11] == TRANTYPE_INQUIRY)
				{
					// 18 Line
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
					// [#2137] MX KSK 2012.07.25
					if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
					{
						if (nCustomerType == MX_DOMESTIC_TYPE)
							strTemp2.Format(L"%s0.00",  CURRENCY_SYMBOL);
						else
							strTemp2.Format(L"%s0.00",  MULTI_CURRENCY_SYMBOL);
					}
					else
					{
						strTemp2.Format(L"%s0.00",  CURRENCY_SYMBOL);
					}
					// end of [#2137]
#else
					strTemp2.Format(L"%s0.00", GetCurrencySymbol());
#endif				// end of [#2115]

#if (AU_VERSION)
					strTemp.Format(L"^%-17.17s = %20.20s", L"BALANCE", strTemp2);
#else
					strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_058), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
#endif
					strPrintData += strTemp;
				}
			}
			// --> 0
			else if (nAvailableBalance == 0)
			{
				if (arJnlField[11] == TRANTYPE_INQUIRY)
				{
					// 18 Line
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
					// [#2137] MX KSK 2012.07.25
					if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
					{
						if (nCustomerType == MX_DOMESTIC_TYPE)
							strTemp2.Format(L"%s0.00",  CURRENCY_SYMBOL);
						else
							strTemp2.Format(L"%s0.00",  MULTI_CURRENCY_SYMBOL);
					}
					else
					{
						strTemp2.Format(L"%s0.00",  CURRENCY_SYMBOL);
					}
					// end of [#2137]
#else
					strTemp2.Format(L"%s0.00", GetCurrencySymbol());
#endif				// end of [#2115]

#if (AU_VERSION)
					strTemp.Format(L"^%-17.17s = %20.20s", L"BALANCE", strTemp2);
#else
					strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_058), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
#endif
					strPrintData += strTemp;

					// 19 Line
#if (MX_VERSION)
					if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
					{
						if (nCustomerType == MX_DOMESTIC_TYPE)
							strTemp2.Format(L"%s0.00",  CURRENCY_SYMBOL);
						else
							strTemp2.Format(L"%s0.00",  MULTI_CURRENCY_SYMBOL);
					}
					else
					{
						strTemp2.Format(L"%s0.00",  CURRENCY_SYMBOL);
					}
#else
					strTemp2.Format(L"%s0.00",  GetCurrencySymbol());
#endif
					strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_057), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
					strPrintData += strTemp;
				}
			}
			// --> +, -
			else
			{
				if (arJnlField[11] == TRANTYPE_INQUIRY)
				{
					// 18 Line
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
					// [#2137] MX KSK 2012.07.25
					if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
					{
						if (nCustomerType == MX_DOMESTIC_TYPE)
							strTemp2.Format(L"%s0.00",  CURRENCY_SYMBOL);
						else
							strTemp2.Format(L"%s0.00",  MULTI_CURRENCY_SYMBOL);
					}
					else
					{
						strTemp2.Format(L"%s0.00",  CURRENCY_SYMBOL);
					}
					// end of [#2137]
#else
					strTemp2.Format(L"%s0.00", GetCurrencySymbol());
#endif				// end of [#2115]

#if (AU_VERSION)
					strTemp.Format(L"^%-17.17s = %20.20s", L"BALANCE", strTemp2);
#else
					strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_058), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
#endif
					strPrintData += strTemp;

					// 19 Line
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
					// [#2137] MX KSK 2012.07.25
					if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
					{
						if (nCustomerType == MX_DOMESTIC_TYPE)
							strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[18]));
						else
							strTemp2.Format(L"%s%s",  MULTI_CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[18]));
					}
					else
					{
						strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[18]));
					}
					// end of [#2137]
#else
					strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[18]));
#endif				// end of [#2115]
					strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_057), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
					strPrintData += strTemp;
				}
				else
				{
					// 18 Line
//					strTemp = L"^";
//					strPrintData += strTemp;

					// 19 Line
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
					// [#2137] MX KSK 2012.07.25
					if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
					{
						if (nCustomerType == MX_DOMESTIC_TYPE)
							strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[18]));
						else
							strTemp2.Format(L"%s%s",  MULTI_CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[18]));
					}
					else
					{
						strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[18]));
					}
					// end of [#2137]
#else
					strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[18]));
#endif				// end of [#2115]
					strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_057), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
					strPrintData += strTemp;
				}
			}
		}
		// LEDGER BALANCE isn't 0
		else
		{
			// --> SPACE
			if (arJnlField[18].GetLength() == 0)
			{
				// 18 Line
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
				// [#2137] MX KSK 2012.07.25
				if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
				{
					if (nCustomerType == MX_DOMESTIC_TYPE)
						strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[26]));
					else
						strTemp2.Format(L"%s%s",  MULTI_CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[26]));
				}
				else
				{
					strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[26]));
				}
				// end of [#2137]
#else
				strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[26]));
#endif				// end of [#2115]

#if (AU_VERSION)
				strTemp.Format(L"^%-17.17s = %20.20s", L"BALANCE", strTemp2);
#else
				strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_058), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
#endif
				strPrintData += strTemp;
			}
			// --> 0
			else if (nAvailableBalance == 0)
			{
				if (arJnlField[11] == TRANTYPE_INQUIRY)
				{
					// 18 Line
#if (MX_VERSION)
					// [#2137] MX KSK 2012.07.25
					if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
					{
						if (nCustomerType == MX_DOMESTIC_TYPE)
							strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[26]));
						else
							strTemp2.Format(L"%s%s",  MULTI_CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[26]));
					}
					else
					{
						strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[26]));
					}
					// end of [#2137]
#else
					strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[26]));
#endif

#if (AU_VERSION)
					strTemp.Format(L"^%-17.17s = %20.20s", L"BALANCE", strTemp2);
#else
					strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_058), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
#endif
					strPrintData += strTemp;

					// 19 Line
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
					// [#2137] MX KSK 2012.07.25
					if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
					{
						if (nCustomerType == MX_DOMESTIC_TYPE)
							strTemp2.Format(L"%s0.00",  CURRENCY_SYMBOL);
						else
							strTemp2.Format(L"%s0.00",  MULTI_CURRENCY_SYMBOL);
					}
					else
					{
						strTemp2.Format(L"%s0.00",  CURRENCY_SYMBOL);
					}
					// end of [#2137]
#else
					strTemp2.Format(L"%s0.00", GetCurrencySymbol());
#endif				// end of [#2115]
					strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_057), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
					strPrintData += strTemp;
				}
				else
				{
					// 18 Line
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
					// [#2137] MX KSK 2012.07.25
					if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
					{
						if (nCustomerType == MX_DOMESTIC_TYPE)
							strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[26]));
						else
							strTemp2.Format(L"%s%s",  MULTI_CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[26]));
					}
					else
					{
						strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[26]));
					}
					// end of [#2137]
#else
					strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[26]));
#endif				// end of [#2115]

#if (AU_VERSION)
					strTemp.Format(L"^%-17.17s = %20.20s", L"BALANCE", strTemp2);
#else
					strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_058), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
#endif
					strPrintData += strTemp;
				}
			}
			// --> +, -
			else
			{
				// 18 Line
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
				// [#2137] MX KSK 2012.07.25
				if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
				{
					if (nCustomerType == MX_DOMESTIC_TYPE)
						strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[26]));
					else
						strTemp2.Format(L"%s%s",  MULTI_CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[26]));
				}
				else
				{
					strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[26]));
				}
				// end of [#2137]
#else
				strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[26]));
#endif				// end of [#2115]

#if (AU_VERSION)
				strTemp.Format(L"^%-17.17s = %20.20s", L"BALANCE", strTemp2);
#else
				strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_058), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
#endif
				strPrintData += strTemp;

				// 19 Line
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
				// [#2137] MX KSK 2012.07.25
				if (strJNLCurrencyID == MULTI_CURRENCY_TYPE)
				{
					if (nCustomerType == MX_DOMESTIC_TYPE)
						strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[18]));
					else
						strTemp2.Format(L"%s%s",  MULTI_CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[18]));
				}
				else
				{
					strTemp2.Format(L"%s%s",  CURRENCY_SYMBOL, MakeMoneyCent(arJnlField[18]));
				}
				// end of [#2137]
#else
				strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[18]));
#endif				// end of [#2115]
				strTemp.Format(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_057), strTemp2);		// [#2160] CA KMK 2012.11.12 FrenchOP
				strPrintData += strTemp;
			}
		}
	}

	// surcharge display.
#if MX_VERSION		// [#2115] MX KSK 2012.02.04
	if (nSurchargeAmount != 0)
#else
	if ((nDispensAmount != 0) && (nSurchargeAmount != 0))
#endif				// end of [#2115]
	{
#if (AU_VERSION)
		// 20 Line
		if (arJnlField[11] != TRANTYPE_DCC)	// [#2362] AU KSK 2015.07.20
		{
			// Line 부족으로 인해 DCC인 경우에는 하기 2개 Line 미 표시 및 미 인자
			strTemp.Format(L"^ATM OPERATOR FEE PAID TO:");
			strPrintData += strTemp;
			strTemp.Format(L"^%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER));
			strPrintData += strTemp;
		}
		// end of [#2362]
#elif (MX_VERSION)	// Journal View Line 부족으로 인해 Fee 정보는 View 및 Display 안함.
		// 20 Line
		// [#2115] MX KSK 2012.02.04
//		strTemp.Format(L"^ATM FEE PAID TO %s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER));
//		strPrintData += strTemp;
		// end of [#2115]
#else

		// 20, 21 Line
		// [#2535] US Justin 2018.04.24 GivePay
		if (arJnlField[11] != TRANTYPE_GIVEPAY)			// Not Printing Surcharge Owner for GivePay
		{
			strTemp.Format(L"^%s:", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_082));		// [#2160] CA KMK 2012.11.12 FrenchOP
			strPrintData += strTemp;
			strTemp.Format(L"^%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER));
			strPrintData += strTemp;
		}
		// End of [#2535]
#endif
	}

#if (AU_VERSION)
	else if ((arJnlField[11] == TRANTYPE_INQUIRY) && (nSurchargeAmount != 0))
	{
		// 20 Line
		strTemp.Format(L"^ATM OPERATOR FEE PAID TO:");
		strPrintData += strTemp;
		strTemp.Format(L"^%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER));
		strPrintData += strTemp;
	}
#endif

#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04

	// [#2141] MX KMK 2012.08.08 BANK NAME, LOYALTY FEE, LINE USAGE FEE는 특정 조건하에 인자된다
	if ( ((strJNLCurrencyID == MULTI_CURRENCY_TYPE) && (nCustomerType == MX_DOMESTIC_TYPE)) // USD 방출 & Domestic Card
		|| (strJNLCurrencyID != MULTI_CURRENCY_TYPE) )	// 혹은 PESO 방출일 때 인자함
	{
		if (!strBankName.IsEmpty())
		{
			strTemp.Format(L"^%-17.17s = %20.20s", L"BANK NAME", strBankName);
			strPrintData += strTemp;
		}

		if (!strLoyaltyFee.IsEmpty())
		{
			strTemp2.Format(L"%s%s", CURRENCY_SYMBOL,strLoyaltyFee);
			strTemp.Format(L"^%-17.17s = %20.20s", L"LOYALTY FEE", strTemp2);
			strPrintData += strTemp;
		}

		if (!strLineUsageFee.IsEmpty())
		{
			strTemp2.Format(L"%s%s", CURRENCY_SYMBOL,strLineUsageFee);
			strTemp.Format(L"^%-17.17s = %20.20s", L"LINE USAGE FEE", strTemp2);
			strPrintData += strTemp;
		}
	}
	// end of [#2141]

	if (!strReturnedAccountNumber.IsEmpty())
	{
		strTemp.Format(L"^%-17.17s = %20.20s", L"ACCOUNT NUMBER", strReturnedAccountNumber);
		strPrintData += strTemp;
	}
#endif	// end of [#2115]

	// 21 Line ERROR CODE
	arJnlField[29].TrimLeft();
	if (arJnlField[29].GetLength() > 0 && arJnlField[29] != L"0000000")
	{
		// [#331] NH AIREAT 2008.06.24 : 에러 코드 형식 일치.
		strTemp.Format(L"^%-11.11s = %1.1s-%5.5s(%2.2s)", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_059), 		// [#2160] CA KMK 2012.11.12 FrenchOP
												arJnlField[27].Right(1),
												arJnlField[29].Left(5), 
												arJnlField[29].Right(2));
		// end of [#331]
		strPrintData += strTemp;
	}

	// [#2292] US Justin 2014.10.07 Other Journal
	// Other Message로 올수 있는 Format 종류
	// 위에서 미리 Parsing을 하므로 이쪽에서는 그냥 Set만 하도록 함
	// 22 Line
	if (!strOtherErrorCode.IsEmpty())
	{
		strTemp.Format(L"^%-11.11s = %1.1s-%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_059), strOtherProcCount, strOtherErrorCode);
		strPrintData += strTemp;
	}
	else if (!strOtherErrorMsg.IsEmpty())
	{
		strTemp.Format(L"^%s", strOtherErrorMsg);
		strPrintData += strTemp;
	}
	// End of [#2292]

#if (AU_VERSION)	// [#2379] AU KSK 2015.11.17
	if (!strDomesticBinFallBack.IsEmpty())
	{
		strTemp.Format(L"^%s", strDomesticBinFallBack);
		strPrintData += strTemp;
	}
#endif				// end of [#2379]

	return strPrintData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertErrorToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : ERROR JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertErrorToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nSeqIndex, nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	nSeqIndex = Asc2Int(arJnlField[10]);
	
	if (nSeqIndex > 0)
	{
		//  1 Line
		// [ MM/DD/YYYY HH:MM:SS T#:XXXX J#:XXXX ]
		strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s  T#:%04.4d J#:%04.4d ]", 
							GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
							arJnlField[6], arJnlField[7], arJnlField[8], nSeqIndex, nJnlIndex);
		strPrintData += strTemp;
	}
	else
	{
		//  1 Line
		// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
		strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
							GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
							arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
		strPrintData += strTemp;
	}
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;
	
	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	// exception
	if (arJnlField.GetSize() < 35)
		return strPrintData;
	
	//  5 Line
	strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));		// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	if (nSeqIndex > 0)
	{
		//  5 Line
		strTemp.Format(L"^%-17.17s = %04d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_035), nSeqIndex);		// [#2160] CA KMK 2012.11.12 FrenchOP
		strPrintData += strTemp;
	}
	else
	{
		//  5 Line
		strTemp.Format(L"^%-17.17s = ", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_035));		// [#2160] CA KMK 2012.11.12 FrenchOP
		strPrintData += strTemp;
	}

	//  6 Line
	// [#331] NH AIREAT 2008.06.24 : 에러 코드 형식 일치.
	strTemp.Format(L"^%-17.17s = %1.1s-%5.5s(%2.2s)", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_059),		// [#2160] CA KMK 2012.11.12 FrenchOP
											arJnlField[27].Right(1),
											arJnlField[29].Left(5), 
											arJnlField[29].Right(2));
	// end of [#331]
	strPrintData += strTemp;

	if (arJnlField[33].GetLength())	
	{
		CStringArray	strOtherMsgArray;
		SplitString(arJnlField[33], UNIT_DELIMITER, strOtherMsgArray);
		
		for(int i=0; i<strOtherMsgArray.GetSize(); i++)
		{
			if (strOtherMsgArray[i].GetLength())
			{
				if( (strOtherMsgArray[i].Left(1)!=RMS_OTHERMSG_DELIMITER)&&(strOtherMsgArray[i].Left(1)!= CDU_OVER_DISPENSE) ) // General Error
				{
					if (strOtherMsgArray.GetSize() >= i+2)
					{
						strTemp.Format(L"^%-17.17s = %1.1s-%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_059), strOtherMsgArray[i], strOtherMsgArray[i+1]);
						strPrintData += strTemp;
					}
					i++;
				}
				else if( strOtherMsgArray[i].Left(1)== CDU_OVER_DISPENSE )
				{
					if (strOtherMsgArray.GetSize() >= i+2)
					{
						strTemp.Format(L"^%s", strOtherMsgArray[i+1]);
						strPrintData += strTemp;
					}
					i+=1;
				}
			}
		}
	}
	// End of [#2460]

	return strPrintData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertOperatorToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Default JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertOperatorToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	
	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;
	
	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;
	
	// exception
	if (arJnlField.GetSize() < 10)
		return strPrintData;

	//  5 Line"master"
	// [#2160] CA KMK 2012.12.21 FrenchOP
	strTemp.Format(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_060), m_pDevCmn->fstrSCR_GetStringFromTextID(AP_TEXT_ID + arJnlField[9].Trim()));
	// end of [#2160]

	strPrintData += strTemp;

	return strPrintData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertPasswordToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Default JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertPasswordToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	
	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;
	
	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;
	
	//  5 Line
	strTemp.Format(L"^%-20.20s=  ********", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_061));		// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	//  6 Line
	strTemp.Format(L"^%-20.20s=  ********", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_062));		// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	return strPrintData;
}

//[#408] AU AIREAT 2008.08.26
/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertOperatorAction()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Operator Action JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertOperatorAction(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	
	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;

	// [#2178] NH KSK 2013.01.30 MultiLine 처리 로직 Bug로 인해 수정
	//  4 Line
//	strTemp.Format(L"^%-40.40s", arJnlField[9]);
//	strPrintData += strTemp;

	// multistring에서 file read시 ID 값은 모두 MakeUpper 시킨 후 저장했기 때문에
	// 거기서 원하는 데이터를 가져오려면 그에 해당하는 ID를 MakeUpper 시킨 후에 던져줘야 함
	CString IDValue = arJnlField[9].MakeUpper();
	CString jnlParameterFromTextID = m_pDevCmn->fstrSCR_GetStringFromTextID(AP_TEXT_ID + IDValue);

	if (jnlParameterFromTextID.GetLength() > 0)		// 저널 문구를 DAT에서 제대로 불러왔을 경우
	{
		strTemp.Format(L"^%-40.40s", jnlParameterFromTextID);
		strPrintData += strTemp;
	}
	else	// DAT에서 못 불러왔을 경우 
	{
		// DAT에서 찾을 수 없는 문구는 있는 그대로 표시한다.
		// [#2326] US Kook 2015.08.27 support multiple lines for 'OA' type journal.
// 		strTemp.Format(L"^%-40.40s", arJnlField[9]);
// 		strPrintData += strTemp;
		CStringArray strArray;
		strTemp.Format(L"%s", arJnlField[9]);
		SplitString(strTemp, UNIT_DELIMITER, strArray);

		for ( int i = 0; i < strArray.GetSize(); i++)
		{
			strTemp.Format(L"^%-40.40s", strArray[i]);
			strPrintData += strTemp;
		}
		// end of [#2326]
	}
	// end of [#2178]

	return strPrintData;
}
//end of [#408]


// KSK 2009.3.30
/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertChangeExchangeRate()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Change Exchange Rate JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertChangeExchangeRate(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	
	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;

	//  4 Line
	strTemp.Format(L"^%-40.40s", arJnlField[9]);
	strPrintData += strTemp;

	return strPrintData;
}
//end of KSK 2009.3.30

// [#529] AU AIREAT 2009.06.02
/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertUserCancelToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : User Cancel JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertUserCancelToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	
	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;

	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	// [#2178] NH KSK 2013.01.30
	// [#2160] CA KMK 2012.12.10 French OP
	// multistring에서 file read시 ID 값은 모두 MakeUpper 시킨 후 저장했기 때문에
	// 거기서 원하는 데이터를 가져오려면 그에 해당하는 ID를 MakeUpper 시킨 후에 던져줘야 함
	CString IDValue = arJnlField[9].MakeUpper();
	CString jnlParameterFromTextID = m_pDevCmn->fstrSCR_GetStringFromTextID(AP_TEXT_ID + IDValue);

	if (jnlParameterFromTextID.GetLength() > 0)		// 저널 문구를 DAT에서 제대로 불러왔을 경우
	{
		strTemp.Format(L"^%-40.40s", jnlParameterFromTextID);
		strPrintData += strTemp;
	}
	else	// DAT에서 못 불러왔을 경우 
	{
		// \n로 분할하기 전의 문구들은 현재 DAT에 
		// 그런 경우 해당 문구를 그대로 사용한다
		strTemp.Format(L"^%-40.40s", arJnlField[9].Left(40));
		strPrintData += strTemp;

		if (arJnlField[9].GetLength() > 40)
		{
			strTemp.Format(L"^%-40.40s", arJnlField[9].Mid(40, 40));
			strPrintData += strTemp;
		}
	}
	// end of [#2160]	end of [#2178]

	return strPrintData;
}
// end of [#529]

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertLibertyXToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : LibertyX TXN -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertLibertyXToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData;
	int	nJnlIndex = Asc2Int(arJnlField[2]);

	// 1. Local Date Time
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strPrintData.AppendFormat(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
		GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
		arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);

	//  2.
	strPrintData.Append(L"^");

	//  3.
	strPrintData.AppendFormat(L"^%40.40s^", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription));

	// 9-22 Fields
	int lineLength = 0;
	
	for (int i = 9; i <= 21; i++)
	{
		CString textIdIndex;
		textIdIndex.Format(L"%02d", i);
		
		CString line;
		line.AppendFormat(L"%s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_LTX + textIdIndex), arJnlField[i]);

		// Make a new line, if necessary. 31 is the max line length, but we need to account for the field separator, "; ".
		if (lineLength + line.GetLength() >= 29)
		{
			strPrintData.Append(L"^");
			lineLength = 0;
		}
		else if (lineLength > 0)
		{
			// Add a separator if there is data pending to be added
			line = L"; " + line;
		}

		strPrintData += line;
		lineLength += line.GetLength();
	}

	strPrintData.Append(L"^");

	return strPrintData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertLibertyXDispenseToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : LibertyX Dispense TXN -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertLibertyXDispenseToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData;
	int	nJnlIndex = Asc2Int(arJnlField[2]);

	// 1. Local Date Time
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strPrintData.AppendFormat(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
		GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
		arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);

	//  2.
	strPrintData.Append(L"^");

	//  3.
	strPrintData.AppendFormat(L"^%40.40s^", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription));

	// 9-22 Fields
	int lineLength = 0;

	for (int i = 9; i <= 29; i++)
	{
		CString textIdIndex;
		textIdIndex.Format(L"%02d", i);

		CString line;
		line.AppendFormat(L"%s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_LTX_DISPENSE + textIdIndex), arJnlField[i]);

		// Make a new line, if necessary. 31 is the max line length, but we need to account for the field separator, "; ".
		if (lineLength + line.GetLength() >= 29)
		{
			strPrintData.Append(L"^");
			lineLength = 0;
		}
		else if (lineLength > 0)
		{
			// Add a separator if there is data pending to be added
			line = L"; " + line;
		}

		strPrintData += line;
		lineLength += line.GetLength();
	}

	strPrintData.Append(L"^");

	return strPrintData;
}

// [#RWC6-188] B4U Log journal entry
/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertB4UToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : B4U TXN -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertB4UToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData, strTemp;
	int	nJnlIndex = Asc2Int(arJnlField[2]);
	int nSeqNum = Asc2Int(arJnlField[10]);

	// 1. Local Date Time
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strPrintData.AppendFormat(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
		GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
		arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);

	// 2. Line
	strPrintData.Append(L"^");

	// 3. Title
	strPrintData.AppendFormat(L"^%40.40s^", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription));

	// 4. Line
	strPrintData.Append(L"^");

	// 5. Terminal ID
	strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
	
	// 6. Sequence #
	strPrintData.AppendFormat(L"^%-17.17s = %04d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_035), nSeqNum);

	// 7. Requested Amount
	strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[24]));
	strPrintData.AppendFormat(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_053), strTemp);

	// 8. Dispensed Amount
	strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[25]));
	strPrintData.AppendFormat(L"^%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_052), strTemp);
	
	if (arJnlField[1].TrimLeft() == B4U_TXN)
	{
		// 9. BtcCharged
		strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_CHARGED), arJnlField[22]);

		// 19. TransactionHash	
		int nHashLen = arJnlField[23].GetLength();
		for (int i = 0; i <= nHashLen / 20; i++)
		{	
			if (i == (nHashLen / 20))
			{
				if (i == 0)
				{
					strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", 
						m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_TXN_HASH),
						arJnlField[23].Mid(i * 20, nHashLen % 20));
				}
				else
				{
					strPrintData.AppendFormat(L"^%-17.17s   %-20.20s",
						L" ",
						arJnlField[23].Mid(i * 20, nHashLen % 20));
				}			
			}
			else
			{
				if (i == 0)
				{
					strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", 
						m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_TXN_HASH),
						arJnlField[23].Mid(i * 20, 20));
				}
				else
				{
					strPrintData.AppendFormat(L"^%-17.17s   %-20.20s",
						L" ",
						arJnlField[23].Mid(i * 20, 20));
				}
			}
		}	
	}
	
	strPrintData.Append(L"^");

	return strPrintData;
}
// End of [#RWC6-188]

// [#RWC6-399] US ryan.payton 2022.08.02 DigitalMint
/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertDigitalMintToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : DigitalMint TXN -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertDigitalMintToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData, strTemp;
	int	nJnlIndex = Asc2Int(arJnlField[2]);

	// Local Date Time
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strPrintData.AppendFormat(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
		GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
		arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);

	// Title
	strPrintData.AppendFormat(L"^^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription));

	// Terminal ID
	strPrintData.AppendFormat(L"^^%-17.17s = %-20.20s^", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), arJnlField[9]);

	// Session ID, Transaction ID, Account ID, API URL, Selected Coin
	for (int i = 10; i <= 14; i++)
	{
		strTemp.Format(L"%02d", i);
		
		CString fieldLabel;
		fieldLabel.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_DM + strTemp), arJnlField[i]);

		CString separator = L"=";

		int fieldLength = arJnlField[i].GetLength();
		for (int j = 0; j <= (fieldLength - 1) / 20; j++)
        {
			int numChars = (j == (fieldLength / 20)) ? fieldLength % 20 : 20;
            strPrintData.AppendFormat(L"^%-17.17s %s %-20.20s", fieldLabel, separator, arJnlField[i].Mid(j * 20, numChars));

			fieldLabel = L" ";
			separator = L" ";
		}

		strPrintData.Append(L"^");
	}

	// Transaction Mode
	if (arJnlField[15] == L"1")
	{
		strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_DM_MODE_DEBIT));
	}
	else if (arJnlField[15] == L"2")
	{
		strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_DM_MODE_CASH));
	}
	else
	{
		strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_DM_MODE_UNKNOWN));
	}

	strPrintData.AppendFormat(L"^%-17.17s = %-20.20s^", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_DM_15), strTemp);

	// Transaction Amount
	strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(arJnlField[16]));
	strPrintData.AppendFormat(L"^%-17.17s = %-20.20s^", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_053), strTemp);
	
	// Error Code
	if (arJnlField[1] == DIGITALMINT_TXN_ERROR)
	{
		strPrintData.AppendFormat(L"^%-17.17s = %-20.20s^", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_059), arJnlField[17]);
	}

	return strPrintData;
}
// End of [#RWC6-399]


// [RWC6-676] Start SKKim 2024.05.09
/*-------------------------------------------------------------------
CLASS    NAME: CJnlMgr
FUNCTION NAME: ConvertTangoPayToPrint()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : DigitalMint TXN -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertTangoPayToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData, strTemp, strData;
	int	nJnlIndex = Asc2Int(arJnlField[2]);

	// Local Date Time
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strPrintData.AppendFormat(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
		GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
		arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);

	// Title
	strPrintData.AppendFormat(L"^^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription));

	// Terminal ID
	strPrintData.AppendFormat(L"^^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), arJnlField[9]);

	int nDataIndex = 0;

	// From Partner ID to Reference Number
	for (nDataIndex=10; nDataIndex<14; nDataIndex++)
	{
		strTemp.Format(L"%02d", nDataIndex);
		strData = arJnlField[nDataIndex];
		strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTemp), strData);

	}

	// 14. Total Amount (Server Received with '.' Cent)
	strTemp.Format(L"%02d", nDataIndex);
	strData.Format(L"%s%s", arJnlField[16], MakeMoneyCent(arJnlField[nDataIndex]));
	strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTemp), strData);

	// 15. Deposited Amount (with Cent)
	nDataIndex++;
	strTemp.Format(L"%02d", nDataIndex);
	strData.Format(L"%s%s", arJnlField[16], MakeMoneyCent(arJnlField[nDataIndex]));
	strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTemp), strData);

	// 16. Send Currency
	nDataIndex++;

	// 17. Send Amount with cent
	nDataIndex++;
	strTemp.Format(L"%02d", nDataIndex);
	strData.Format(L"%s%s", arJnlField[16], MakeMoneyCent(arJnlField[nDataIndex]));
	strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTemp), strData);

	// 18. Send Fees with cent
	nDataIndex++;
	strTemp.Format(L"%02d", nDataIndex);
	strData.Format(L"%s%s", arJnlField[16], MakeMoneyCent(arJnlField[nDataIndex]));
	strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTemp), strData);

	// 19. Send Taxes with cent [RWC6-676] SKKim 2024.05.29
	nDataIndex++;
	strTemp.Format(L"%02d", nDataIndex);
	strData.Format(L"%s%s", arJnlField[16], MakeMoneyCent(arJnlField[nDataIndex]));
	strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTemp), strData);

	// 20. Receive Amount (without cent)
	nDataIndex++;
	strTemp.Format(L"%02d", nDataIndex);
	strData.Format(L"%s%s", arJnlField[20], MakeMoneyAmount(arJnlField[nDataIndex]));
	strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTemp), strData);

	// 21. Receive Currency
	nDataIndex++;

	// 22. Receive Country
	nDataIndex++;
	strTemp.Format(L"%02d", nDataIndex);
	strData.Format(L"%s", arJnlField[nDataIndex]);
	strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTemp), strData);

	// 23. Receive Name
	nDataIndex++;
	strTemp.Format(L"%02d", nDataIndex);
	strData.Format(L"%s", arJnlField[nDataIndex]);
	strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTemp), strData);

	// 24. Result
	nDataIndex++;
	strTemp.Format(L"%02d", nDataIndex);
	strData = arJnlField[nDataIndex];
	strPrintData.AppendFormat(L"^%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTemp), strData);

	return strPrintData;
}
// [RWC6-676] End SKKim 2024.05.09


/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertDefaultToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Default JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertDefaultToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	
	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;
	
	return strPrintData;
}

//[#586] SOOK 2009.12.08 iTM MoniView 연동 부분 추가 
/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: GetJnlImageFileName()
 RETURN TYPE  : CString
 PARAMETER    : 
 DESCRIPTION  : 해당 Stack No, Year, Month, Day에 해당하는 저널 파일 이름을 얻어온다 (사용안함)
-------------------------------------------------------------------*/
CString CJnlMgr::GetJnlImageFileName(int StackNo, int StackYear, int StackMonth, int StackDay)
{
	CString FullFileName;
	FullFileName.Format(_T("%s\\Image-%04d-%04d%02d%02d-1.jpg"),
		JNL_IMAGE_FILE_PATH, StackNo, StackYear, StackMonth, StackDay ); //CE는 Image 1 컷만 저장하므로 Sequence는 1로 고정함)

	return FullFileName;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: DoesJnlImageExist()
 RETURN TYPE  : BOOL (FALSE/TRUE)
 PARAMETER    : 
 DESCRIPTION  : 해당 Stack No, Year, Month, Day에 Still Image 존재 여부 확인 (사용안함)
-------------------------------------------------------------------*/
BOOL CJnlMgr::DoesJnlImageExist(int StackNo, int StackYear, int StackMonth, int StackDay)
{
	CCeFileFind finder;
	BOOL bWorking = FALSE;

	CString FullFileName = GetJnlImageFileName(StackNo, StackYear, StackMonth, StackDay);
	
	bWorking = finder.FindFile(FullFileName);
	
	if (bWorking == FALSE)
	{
		return FALSE;
	}
	
	finder.Close();

	return TRUE;
}
//end of [#586];
//[#610] SOOK 2009.12.21 Parameter Change Journal 저장 
/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: SaveChangeParameterJnl()
 RETURN TYPE  : BOOL (FALSE: 저장하지 않고  Retur함 
 PARAMETER    : 
	1. USER
	2. 변경하고자 하는 Parameter
	3. Old Value
	4. New Value
 DESCRIPTION  : Old Value와 New Value 값을 받아 변경사항이 있으면 저장함 
-------------------------------------------------------------------*/
BOOL  CJnlMgr::SaveChangeParameterJnl(int User, CString strParameter, CString oldValue, CString newValue)
{
	CString strJnlData = L"";

	strParameter.TrimRight();
	if ( strParameter.GetLength() == 0 ) return FALSE;

	strJnlData.Format(L"%d", User);
	strJnlData += L"^";

	strJnlData += strParameter;
	CString strJnlSep;								//[#610] SOOK 2010.01.27
	strJnlSep.Format(L"%c", PROXY_JNL_DELIMITER);	//[#610] SOOK 2010.01.27

	oldValue.TrimRight();  //우측에 공백만 추가 될 경우 String Compare로는 다른 문자열로 인식하나 
	oldValue.Replace('^', PROXY_JNL_DELIMITER);	//[#610] SOOK 2010.01.27
	newValue.TrimRight();  //화면 표시는 동일하게 표시되므로 공백을 제거하라 저장한다.  
	newValue.Replace('^', PROXY_JNL_DELIMITER);	//[#610] SOOK 2010.01.27

	if ( oldValue == newValue )
		return FALSE;

	strJnlData += L"^";
	strJnlData += oldValue;
	strJnlData += L"^";
	strJnlData += newValue;

	Save(CHANGE_PARAMETER, strJnlData);
	
	return TRUE;
}

BOOL  CJnlMgr::SaveChangeParameterJnl(int User, CString strParameter, CStringArray &oldValue, CStringArray &newValue)
{
	CString strJnlData = L"";
	CString strTempOld = L"", strTempNew=L"";
	CString strTemp; 

	strParameter.TrimRight();
	if ( strParameter.GetLength() == 0 || oldValue.GetSize() ==0 ) return FALSE;

	//[#621]SOOK 2010.02.22 RMS에서 공백 채워서 내려왔을 경우도 저장 되므로 예외 처리함. 
	int nArrayIndex = oldValue.GetSize();
	BOOL bChange = FALSE;
	for ( int i = 0; i < nArrayIndex; i++)
	{
		CString strTempOld, strTempNew;
		strTempOld = oldValue.GetAt(i);
		strTempNew = newValue.GetAt(i);
		strTempOld.TrimRight();
		strTempNew.TrimRight();

		if ( strTempOld != strTempNew )
			bChange = TRUE;
	}
	
	if ( bChange == FALSE) return FALSE;
	//end of [#621] 2010.02.22

	strJnlData.Format(L"%d", User);
	strJnlData += L"^";

	strJnlData += strParameter;


	strJnlData += L"^";
	CString strJnlSep;								//[#610] SOOK 2010.01.27
	strJnlSep.Format(L"%c", PROXY_JNL_DELIMITER);	//[#610] SOOK 2010.01.27

	
	for (int i = 0; i < nArrayIndex; i++)
	{
		strTemp = oldValue.GetAt(i);
		strTemp.TrimRight();//우측에 공백만 추가 될 경우 String Compare로는 다른 문자열로 인식하나 화면 표시는 동일하게 표시되므로 공백을 제거하라 저장한다
		strTemp.Replace('^', PROXY_JNL_DELIMITER); //[#610] SOOK 2010.01.27 
		strJnlData.Format(L"%s%s", strJnlData, strTemp);

		if ( i < (nArrayIndex -1))
			strJnlData.Format(L"%s%c", strJnlData, UNIT_DELIMITER);
	}

	strJnlData.Format(L"%s^", strJnlData);

	nArrayIndex = newValue.GetSize();
	for (int i = 0; i < nArrayIndex; i++)
	{
		strTemp = newValue.GetAt(i);
		strTemp.TrimRight();//우측에 공백만 추가 될 경우 String Compare로는 다른 문자열로 인식하나 화면 표시는 동일하게 표시되므로 공백을 제거하여 저장함 
		strTemp.Replace('^', PROXY_JNL_DELIMITER); //[#610] SOOK 2010.01.27 
		strJnlData.Format(L"%s%s", strJnlData, strTemp);

		if ( i < (nArrayIndex -1))
			strJnlData.Format(L"%s%c", strJnlData, UNIT_DELIMITER);
	}

	Save(CHANGE_PARAMETER, strJnlData);
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlMgr
 FUNCTION NAME: ConvertChangeParameterToPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : CHNAGE PARAMETER JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertChangeParameterToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nJnlIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	
	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
						GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
						arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;
	
	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP//[#638]
	strPrintData += strTemp;
	
	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	// exception
	if (arJnlField.GetSize() < 12)
		return strPrintData;
	
	//  5 Line
	CString strUserName = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_063);		// [#2160] CA KMK 2012.11.12 FrenchOP
	if ( arJnlField[9] == "1")
		strUserName = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_064);		// [#2160] CA KMK 2012.11.12 FrenchOP
	else if ( arJnlField[9] == "2")
		strUserName = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_065);		// [#2160] CA KMK 2012.11.12 FrenchOP
	else if ( arJnlField[9] == "3")
		strUserName = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_066);		// [#2160] CA KMK 2012.11.12 FrenchOP
	else if ( arJnlField[9] == "4")
		strUserName = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_067);		// [#2160] CA KMK 2012.11.12 FrenchOP
	else if ( arJnlField[9] == "5")
		strUserName = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_068);		// [#2160] CA KMK 2012.11.12 FrenchOP
	strTemp.Format(L"^%-10.10s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_069), strUserName);		// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	//  6 Line
	strTemp.Format(L"^%-10.10s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_070), arJnlField[10]);		// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;
	
	if ( arJnlField[11].Find(UNIT_DELIMITER) == -1) //한줄로 된 경우 
	{
		//  7 Line
		if ( arJnlField[11].GetLength() > 33 || arJnlField[12].GetLength()> 33) //명세표 출력 범위 넘어 갈 때. 
		{
			strTemp.Format(L"^%-4.4s: ^%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_071), arJnlField[11]);		// [#2160] CA KMK 2012.11.12 FrenchOP
			strPrintData += strTemp;
			
			//  8 Line
			strTemp.Format(L"^%-4.4s: ^%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_072), arJnlField[12]);		// [#2160] CA KMK 2012.11.12 FrenchOP
			strPrintData += strTemp;
		}
		else
		{
			strTemp.Format(L"^%-4.4s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_071), arJnlField[11]);		// [#2160] CA KMK 2012.11.12 FrenchOP
			strPrintData += strTemp;
			
			//  8 Line
			strTemp.Format(L"^%-4.4s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_072), arJnlField[12]);		// [#2160] CA KMK 2012.11.12 FrenchOP
			strPrintData += strTemp;
		}
	}
	else
	{
		CStringArray		strArray;
		//  7 Line
		strTemp.Format(L"^%-4.4s: ",  m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_071));		// [#2160] CA KMK 2012.11.12 FrenchOP
		strPrintData += strTemp;

		strTemp.Format(L"%s", arJnlField[11]);
		SplitString(strTemp, UNIT_DELIMITER, strArray);

		for ( int i = 0; i < strArray.GetSize(); i++)
		{
			strPrintData += L"^";
			strPrintData += strArray[i];
		}
		
		//  8 Line
		strTemp.Format(L"^%-4.4s : ", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_072));		// [#2160] CA KMK 2012.11.12 FrenchOP
		strPrintData += strTemp;

		strTemp.Format(L"%s", arJnlField[12]);
		SplitString(strTemp, UNIT_DELIMITER, strArray);

		for ( int i = 0; i < strArray.GetSize(); i++)
		{
			strPrintData += L"^";
			strPrintData += strArray[i];
		}

	}

	return strPrintData;
}

//#endif
//end of [#610]

// [#2115] MX KSK 2012.02.04
/*-------------------------------------------------------------------
CLASS    NAME: CJnlMgr
FUNCTION NAME: ConvertTrxEMVDataToPrint()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : TRANSACKTION EMV DATA JNL -> PRINT DATA
-------------------------------------------------------------------*/
CString CJnlMgr::ConvertTrxEMVDataToPrint(int DescKindCodeIndex, CStringArray &arJnlField)
{
	CString	strPrintData = L"", strTemp;
	int		nJnlIndex, nSeqIndex;

	nJnlIndex = Asc2Int(arJnlField[2]);
	nSeqIndex = Asc2Int(arJnlField[10]);

	//  1 Line
	// [ MM/DD/YYYY HH:MM:SS         J#:XXXX ]
	strTemp.Format(L"[ %s %2.2s:%2.2s:%2.2s          J#:%04.4d ]", 
		GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]),
		arJnlField[6], arJnlField[7], arJnlField[8], nJnlIndex);
	strPrintData += strTemp;

	//  2 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	//  3 Line
	strTemp.Format(L"^%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportEJKindCode[DescKindCodeIndex].szDescription)); // [#2160] CA KMK 2012.11.14 FrenchOP
	strPrintData += strTemp;

	//  4 Line
	strTemp.Format(L"^");
	strPrintData += strTemp;

	// exception
	if (arJnlField.GetSize() < 12)
		return strPrintData;

	//  5 Line
	// Journal Format중에 ADD CASH / DENOMINATION에 Terminal ID가 없어서 현재 TERMINAL ID로 SET하도록 함
	strTemp.Format(L"^%-18.18s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));		// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	//  6 Line
	strTemp.Format(L"^%-18.18s= %04d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_035), nSeqIndex);		// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	//	7 Line
	strTemp.Format(L"^%-18.18s= %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_036), arJnlField[11]);		// [#2160] CA KMK 2012.11.12 FrenchOP
	strPrintData += strTemp;

	CString strTagLengthValue;
	strTagLengthValue = arJnlField[12];

	CString strTag, strValue;
	int		nLen = 0;

	//	8 Line (4F00) Application ID
	strTag = strTagLengthValue.Left(4);
	if (strTag == APPLICATION_ID)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_073), strValue);		// [#2160] CA KMK 2012.11.12 FrenchOP
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_073));		// [#2160] CA KMK 2012.11.12 FrenchOP
		}
	}
	else
	{
		strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_073));		// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	strPrintData += strTemp;

	//	9 Line (5000) Application Label
	strTag = strTagLengthValue.Left(4);
	if (strTag == APPLICATION_LABEL)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_074), strValue);		// [#2160] CA KMK 2012.11.12 FrenchOP
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_074));		// [#2160] CA KMK 2012.11.12 FrenchOP
		}
	}
	else
	{
		strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_074));		// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	strPrintData += strTemp;

	//	10 Line (9F26) ARQC
	strTag = strTagLengthValue.Left(4);
	if (strTag == APP_CRYPTOGRAM_TAG_9F26)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_075), strValue);		// [#2160] CA KMK 2012.11.12 FrenchOP
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_075));		// [#2160] CA KMK 2012.11.12 FrenchOP
		}
	}
	else
	{
		strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_075));		// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	strPrintData += strTemp;

	//	11 Line (91) ARPC
	strTag = strTagLengthValue.Left(4);
	if (strTag == JNL_IAD)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_076), strValue);		// [#2160] CA KMK 2012.11.12 FrenchOP
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_076));		// [#2160] CA KMK 2012.11.12 FrenchOP
		}
	}
	else
	{
		strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_076));		// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	strPrintData += strTemp;

	//	12 Line (5F30) Service Code
	strTag = strTagLengthValue.Left(4);
	if (strTag == SERVICE_CODE)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_077), strValue);		// [#2160] CA KMK 2012.11.12 FrenchOP
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_077));		// [#2160] CA KMK 2012.11.12 FrenchOP
		}
	}
	else
	{
		strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_077));		// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	strPrintData += strTemp;

	//	13 Line (9F33) Terminal Capability
	strTag = strTagLengthValue.Left(4);
	if (strTag == TERMINAL_CAPABILITIES_TAG_9F33)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_078), strValue);		// [#2160] CA KMK 2012.11.12 FrenchOP
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_078));		// [#2160] CA KMK 2012.11.12 FrenchOP
		}
	}
	else
	{
		strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_078));		// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	strPrintData += strTemp;

	//	14 Line (9F39) POS Entry Mode
	strTag = strTagLengthValue.Left(4);
	if (strTag == POS_ENTRY_MODE_TAG_9F39)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_079), strValue);		// [#2160] CA KMK 2012.11.12 FrenchOP
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_079));		// [#2160] CA KMK 2012.11.12 FrenchOP
		}
	}
	else
	{
		strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_079));		// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	strPrintData += strTemp;

	// 15 Line (9500) TVR
	strTag = strTagLengthValue.Left(4);
	if (strTag == JNL_TVR)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_080), strValue);		// [#2160] CA KMK 2012.11.12 FrenchOP
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_080));		// [#2160] CA KMK 2012.11.12 FrenchOP
		}
	}
	else
	{
		strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_080));		// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	strPrintData += strTemp;

	//	16 Line (9F0E) IAC
	strTag = strTagLengthValue.Left(4);
	if (strTag == ISSUER_ACTION_CODE)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_081), strValue);	// [#2160] CA KMK 2012.11.12 FrenchOP
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_081));		// [#2160] CA KMK 2012.11.12 FrenchOP
		}
	}
	else
	{
		strTemp.Format(L"^%-17.17s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_081));			// [#2160] CA KMK 2012.11.12 FrenchOP
	}
	strPrintData += strTemp;

// [#2353] AU KSK 2015.06.28 고객 요청에 의해 EMV Extra Data 추가 (DCPayments요청사항 대응)
#if (AU_VERSION)
	//	17 Line (5F34) PAN SEQ.NO
	//strTag = strTagLengthValue.Left(4);
	//if (strTag == APPLICATION_PAN_SEQ_TAG_5F34)
	//{
	//	strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
	//	nLen = Asc2Int(strTagLengthValue.Left(2));
	//	strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

	//	if (nLen > 0)
	//	{
	//		strValue = strTagLengthValue.Left(nLen);
	//		strTemp.Format(L"^%-4.4s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_104), strValue);
	//		strTagLengthValue = strTagLengthValue.Mid(nLen);
	//	}
	//	else
	//	{
	//		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
	//		strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_104));
	//	}
	//}
	//else
	//{
	//	strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_104));
	//}
	//strPrintData += strTemp;	

	//	18 Line (8200) AIP
	strTag = strTagLengthValue.Left(4);
	if (strTag == JNL_AIP_8200)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-4.4s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_105), strValue);
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_105));
		}
	}
	else
	{
		strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_105));
	}
	strPrintData += strTemp;

	//	19 Line (8A00) ARC
	strTag = strTagLengthValue.Left(4);
	if (strTag == JNL_ARC_8A00)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-4.4s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_106), strValue);
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_106));
		}
	}
	else
	{
		strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_106));
	}
	strPrintData += strTemp;

	//	20 Line (9C00) TRANS. TYPE
	//strTag = strTagLengthValue.Left(4);
	//if (strTag == JNL_TRANSTYPE_9C00)
	//{
	//	strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
	//	nLen = Asc2Int(strTagLengthValue.Left(2));
	//	strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

	//	if (nLen > 0)
	//	{
	//		strValue = strTagLengthValue.Left(nLen);
	//		strTemp.Format(L"^%-4.4s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_107), strValue);
	//		strTagLengthValue = strTagLengthValue.Mid(nLen);
	//	}
	//	else
	//	{
	//		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
	//		strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_107));
	//	}
	//}
	//else
	//{
	//	strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_107));
	//}
	//strPrintData += strTemp;

	//	21 Line (9B00) TSI
	strTag = strTagLengthValue.Left(4);
	if (strTag == JNL_TSI_9B00)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-4.4s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_108), strValue);
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_108));
		}
	}
	else
	{
		strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_108));
	}
	strPrintData += strTemp;

	//	22 Line (9F10) IAD
	strTag = strTagLengthValue.Left(4);
	if (strTag == ISSUER_APP_DATA_TAG_9F10)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-4.4s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_109), strValue);
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_109));
		}
	}
	else
	{
		strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_109));
	}
	strPrintData += strTemp;

	//	23 Line (9F27) CID
	strTag = strTagLengthValue.Left(4);
	if (strTag == CRYPTOGRAM_INFO_DATA_TAG_9F27)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-4.4s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_110), strValue);
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_110));
		}
	}
	else
	{
		strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_110));
	}
	strPrintData += strTemp;

	//	24 Line (9F36) ATC
	strTag = strTagLengthValue.Left(4);
	if (strTag == APP_TRANSACTION_COUNTER_TAG_9F36)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-4.4s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_111), strValue);
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_111));
		}
	}
	else
	{
		strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_111));
	}
	strPrintData += strTemp;

	//	25 Line (9F37) UNPREDICTABLE NO.
	strTag = strTagLengthValue.Left(4);
	if (strTag == RANDOM_NUMBER_TAG_9F37)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-4.4s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_112), strValue);
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_112));
		}
	}
	else
	{
		strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_112));
	}
	strPrintData += strTemp;

	//	26 Line (5F2A) TRANSACTION CURRENCY CODE
	strTag = strTagLengthValue.Left(4);
	if (strTag == TRANSACTION_CURRENCY_CODE_TAG_5F2A)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-4.4s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_113), strValue);
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_113));
		}
	}
	else
	{
		strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_113));
	}
	strPrintData += strTemp;

	//	27 Line (9F1A) TERMINAL COUNTRY CODE
	strTag = strTagLengthValue.Left(4);
	if (strTag == TERMINAL_COUNTRY_CODE_TAG_9F1A)
	{
		strTagLengthValue = strTagLengthValue.Mid(4);	// 앞 4 Byte 제거
		nLen = Asc2Int(strTagLengthValue.Left(2));
		strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거

		if (nLen > 0)
		{
			strValue = strTagLengthValue.Left(nLen);
			strTemp.Format(L"^%-4.4s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_114), strValue);
			strTagLengthValue = strTagLengthValue.Mid(nLen);
		}
		else
		{
			strTagLengthValue = strTagLengthValue.Mid(2);	// 앞 2 Byte 제거
			strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_114));
		}
	}
	else
	{
		strTemp.Format(L"^%-4.4s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_114));
	}
	strPrintData += strTemp;
#endif
// end of [#2353]

	return strPrintData;
}
// end of [#2115]
