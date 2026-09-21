#include "stdafx.h"
#include ".\Tran\TranCmn.h"
#include ".\LibUpdate\UpdateRepoConfigurationManager.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

//------------------------------------------------------------------
//	Define Symbol
//------------------------------------------------------------------


//------------------------------------------------------------------
//	Macros
//------------------------------------------------------------------
#define DUALPRINT(printData, dualReceiptLang, ...) \
{ \
	AddPrintData(printData, __VA_ARGS__); \
	if ( (dualReceiptLang > 0) && (dualReceiptLang != m_pDevCmn->fnSCR_GetCurrentLangMode()) ) \
	{ \
		int currentLang = m_pDevCmn->fnSCR_GetCurrentLangMode(); \
		m_pDevCmn->fnSCR_SetCurrentLangMode(dualReceiptLang); \
		AddPrintData(printData, __VA_ARGS__); \
		m_pDevCmn->fnSCR_SetCurrentLangMode(currentLang); \
	} \
}

#define CHANGE_LANG()

//------------------------------------------------------------------
//	Implement
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_NOR_TransactionPrint()
 RETURN TYPE  :
 PARAMETER    :
 DESCRIPTION  :
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_NOR_TransactionPrint(BOOL bCut)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_NOR_TransactionPrint]\n"));

	int		i;
	int		nDualReceiptLang = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUAL_RECEIPT);
	CString		strTemp, strTemp2;
	CString		strTranType;
	CString		strPrintData;
	BOOL		bPrintTransData = FALSE;
	BOOL		bIsDualReceiptEnabled = nDualReceiptLang > 0 ? TRUE : FALSE;
	BOOL		bIsDualReceiptSameAsCurrent = nDualReceiptLang == m_pDevCmn->m_nCurrentLangMode ? TRUE : FALSE;

	// [#2219] 2013.09.04 US KMK 2013.05.13 Digital Receipt 사용 시 명세표를 영문으로 출력하도록 함
	// (현재 QRCODE용 명세표 축약 로직이 영문 문구만 지원하므로)
	int			nSavedLangMode;
	if (m_sUserSelection.nPrintReceipt == RCPT_QRCODE)
	{
		nSavedLangMode = m_pDevCmn->fnSCR_GetCurrentLangMode();	// 현재 언어 정보 저장
		m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);
	}
	// end of [#2219]

	strTranType.Format(L"%2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE));
	int nWithdrawalType = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE);

	strPrintData = L"";
	AddPrintData(strPrintData, L" ");

	BOOL bLineFeed = FALSE;	// AU KSK 2016.06.20 AU

#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
	NHDEBUG(DBG_INFO, (L"[PRINT]-[STORE MESSAGE]\n"));
	// STORE MESSAGE -- 값이 모두 SPACE인 경우 PRINT하지 않음
	//------------------------------------------------------------------------------------------------
	for (i = 0; i < 4; i++)
	{
		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_STORE_MESSAGE1 + i);
		strTemp.TrimLeft();

		if (strTemp.GetLength() > 0)
		{
			AddPrintData(strPrintData, MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_STORE_MESSAGE1 + i));
			bLineFeed = TRUE;	// AU KSK 2016.06.20 AU
		}
	}
	//------------------------------------------------------------------------------------------------

	NHDEBUG(DBG_INFO, (L"[PRINT]-[PROCESSOR MESSAGE]\n"));
	// PROCESSOR MESSAGE -- 값이 모두 SPACE인 경우 PRINT하지 않음
	//------------------------------------------------------------------------------------------------
	for (i = 0; i < 4; i++)
	{
		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PROCESSOR_MESSAGE1 + i);
		strTemp.TrimLeft();

		if (strTemp.GetLength() > 0)
		{
			AddPrintData(strPrintData, MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PROCESSOR_MESSAGE1 + i));
			bLineFeed = TRUE;	// AU KSK 2016.06.20 AU
		}
	}
	//------------------------------------------------------------------------------------------------
#endif

	NHDEBUG(DBG_INFO, (L"[PRINT]-[ADDRESS 1,2,3,PHONE NUMBER]\n"));
	// ADDRESS 1,2,3,PHONE NUMBER PRINT -- 값이 모두 SPACE인 경우 PRINT하지 않음
	{
		//------------------------------------------------------------------------------------------------
		for (i = 0; i < 4; i++)
		{
			strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1 + i);
			strTemp.TrimLeft();

			if (strTemp.GetLength() > 0)
			{
				AddPrintData(strPrintData, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1 + i));
				bLineFeed = TRUE;	// AU KSK 2016.06.20 AU
			}
		}
		//------------------------------------------------------------------------------------------------


		NHDEBUG(DBG_INFO, (L"[PRINT]-[HEADER 1,2 PRINT]\n"));
		// HEADER 1,2 PRINT
		//------------------------------------------------------------------------------------------------	
		for (i = 0; i < 2; i++)
		{
			strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER1 + i);
			strTemp.TrimLeft();

			if (strTemp.GetLength() > 0)
			{
				AddPrintData(strPrintData, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER1 + i));
				bLineFeed = TRUE;	// AU KSK 2016.06.20 AU
			}
		}
		//------------------------------------------------------------------------------------------------
	}

#if (AU_VERSION)	// AU KSK 2016.06.20 AU
	if (bLineFeed == TRUE)
		AddPrintData(strPrintData, L" ");
#endif			// end of KSK 2016.06.20

	// BODY
	// 
	//------------------------------------------------------------------------------------------------
	{
		// KSK 2010.04.19 명세표 표준화 적용
		// Title 왼쪽 정렬(18 Colume) / Value 우측 정렬(21 Colume)
		NHDEBUG(DBG_INFO, (L"[PRINT]-[TERMINAL NUMBER]\n"));
		// TERMINAL NUMBER

		CString terminalId;
#if (US_VERSION && APP_LIBERTYX)
		if (m_HostConfig == HC_LIBERTYX)
		{
			terminalId = m_LXConfig.LocationID;
		}
		else
#endif
		{
			terminalId = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
		}

		DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_001), terminalId);

		NHDEBUG(DBG_INFO, (L"[PRINT]-[SEQUENCE NUMBER]\n"));
		// SEQUENCE NUMBER
		// [#2292] US Justin 2014.10.10 Dual Balance : PRINT SEQUENCE NUMBER
		//AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_002),
		//										    		MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SEQUENCENO));

		// [#2502] US Justin 2017.08.24 Bug Fix - Clearing Communicatio Error after Pre-Dual Balance
		//if( (m_DualBalance.m_bTranResult==TRUE)&&(m_DualBalance.m_bSavingBalance!=TRUE) )
		if ((m_DualBalance.m_bDualBalance == TRUE) && (m_DualBalance.m_bTranResult == TRUE) && (m_DualBalance.m_bSavingBalance != TRUE))			// Use Stored Data (Dual BI Success but the second BI was Fail)
		{
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_002), m_DualBalance.m_strSequenceNumber);
		}
		else
		{
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_002), m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig));
		}
		// End of [#2292]

#if (APP_JUST_CASH)
		if (_jcTransactionState.PaymentSelection != JCP_CASH)
#endif
		{
			// AUTHORIZATION ID
			if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
				strTemp.Format(L"%-6.6s%-2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID));
			else // STANDARD1, 2, EPS는 Audit Num과 NetworkID 사이에 SPACE 추가
				strTemp.Format(L"%-6.6s %-2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID));

			DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_003), strTemp);
		}

		NHDEBUG(DBG_INFO, (L"[PRINT]-[DATE TIME]\n"));
		// DATE & TIME
		{
#ifdef APP_LOCAL_MODE
			SYSTEMTIME localTime;
			::GetLocalTime(&localTime);
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %s %2.2d:%2.2d:%2.2d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_004), GetCmnLocalDate(localTime.wMonth, localTime.wDay, localTime.wYear), localTime.wHour, localTime.wMinute, localTime.wSecond);
#else
			// [#2292] US Justin 2014.10.10 Dual Balance
			// [#2502] US Justin 2017.08.24 Bug Fix - Clearing Communicatio Error after Pre-Dual Balance
			//if( (m_DualBalance.m_bTranResult==TRUE)&&(m_DualBalance.m_bSavingBalance!=TRUE) )
			if ((m_DualBalance.m_bDualBalance == TRUE) && (m_DualBalance.m_bTranResult == TRUE) && (m_DualBalance.m_bSavingBalance != TRUE))			// Use Stored Data (Dual BI Success but the second BI was Fail)
				strTemp = m_DualBalance.m_strTranDate;
			else
				strTemp.Format(_T("%8.8s"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE));
			// End of [#2292]
			strTemp.Replace(_T(" "), _T(""));		// STD3인 경우 6byte이므로 SPACE 제거

			if (strTemp != L"01012000")	// NVRAM의 Default값이 01012000임
			{
				// [#2292] US Justin 2014.10.10 Dual Balance : PRINT TRANSACTION DATE AND TIME
				// [#2502] US Justin 2017.08.24 Bug Fix - Clearing Communicatio Error after Pre-Dual Balance
				//if( (m_DualBalance.m_bTranResult==TRUE)&&(m_DualBalance.m_bSavingBalance!=TRUE) )
				if ((m_DualBalance.m_bDualBalance == TRUE) && (m_DualBalance.m_bTranResult == TRUE) && (m_DualBalance.m_bSavingBalance != TRUE))			// Use Stored Data (Dual BI Success but the second BI was Fail)
					strTemp2 = m_DualBalance.m_strTranTime;
				else
					strTemp2.Format(_T("%6.6s"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME));
				// End of [#2292]

				DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %s %2.2s:%2.2s:%2.2s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_004), GetCmnLocalDate(strTemp.Left(2), strTemp.Mid(2, 2), strTemp.Right(2)),
					strTemp2.Left(2), strTemp2.Mid(2, 2), strTemp2.Right(2));
			}
			else
			{
				// Host로부터 날짜를 안받았을 경우 처리
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = ", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_004));
			}
#endif
		}

		NHDEBUG(DBG_INFO, (L"[PRINT]-[BUSINESS DATE]\n"));
		// BUSINESS DATE
#ifdef APP_LOCAL_MODE
		SYSTEMTIME localTime;
		::GetLocalTime(&localTime);
		DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_005), GetCmnLocalDate(localTime.wMonth, localTime.wDay, localTime.wYear));
#else

		strTemp.Format(_T("%8.8s"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSSETTLEDATE));
		strTemp.Replace(_T(" "), _T(""));		// STD3인 경우 6byte이므로 SPACE 제거

#if (AU_VERSION)
// 호주 FDI는 Business Date를 사용하지 않음. '000000'으로 내려줌. --> 인자 하지 않음.
		if (strTemp == L"000000" || strTemp == L"01012000")
		{
		}
		else
#endif

			if (strTemp != L"01012000")	// NVRAM의 Default값이 01012000임
			{
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_005), GetCmnLocalDate(strTemp.Left(2), strTemp.Mid(2, 2), strTemp.Right(2)));
			}
			else
			{
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = ", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_005));
			}
#endif

		NHDEBUG(DBG_INFO, (L"[PRINT]-[CARD NUMBER]\n"));
		// CARD NUMBER

		if (nWithdrawalType == WITHDRAWAL_POPMONEY)		// POP Money : ACCESS CODE
		{
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_037));
		}
		else if (nWithdrawalType == WITHDRAWAL_PIN4)
		{
			// [#2560] US Justin 2018.07.09 MasterCard Cash Pickup - Print Virtual Card Number on receipts
#if(APP_PRESTAGIN_PIN4)
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_006), m_sCardData.strShowNumber);
#endif
			// End of [#2560]

			strTemp = "Phone Number";
			if (m_Pin4.m_strUSPhoneNumber.GetLength() == 10)
			{
				m_sCardData.strShowNumber.Format(L"1-%s", m_Pin4.m_strUSPhoneNumber);
			}
			else
			{
				m_sCardData.strShowNumber.Format(L"%s-%s", m_Pin4.m_strIntPhoneCountryCode, m_Pin4.m_strIntPhoneNumber);
			}

			AddPrintData(strPrintData, L"%-17.17s = %-20.20s", strTemp, m_sCardData.strShowNumber);
			if (bIsDualReceiptEnabled && !bIsDualReceiptSameAsCurrent)
			{
				AddPrintData(strPrintData, L"%-17.17s = %-20.20s", strTemp, m_sCardData.strShowNumber);
			}
		}
		// End of [#2396]
		// [#2445] US Justin 2016.09.28 Just.Cash
#if (APP_JUST_CASH)
		else if (nWithdrawalType == WITHDRAWAL_JUSTCASH)
		{
			strTemp = "CASH CODE";
			m_sCardData.strShowNumber.Format(L"%s", m_JustCashData.m_strCashCode);
			AddPrintData(strPrintData, L"%-17.17s = %-20.20s", strTemp, m_sCardData.strShowNumber);
			if (bIsDualReceiptEnabled && !bIsDualReceiptSameAsCurrent)
			{
				AddPrintData(strPrintData, L"%-17.17s = %-20.20s", strTemp, m_sCardData.strShowNumber);
			}
		}
#endif
		// End of [#2445]
		else if (nWithdrawalType != WITHDRAWAL_DIGITAL_CUR_CASH_JC)
		{
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_006), m_sCardData.strShowNumber);
		}
		// End of [#2350]

		// APPILICATION ID		
		if (m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			//[#2252] US Justin 2014.02.07 Long AID
			//AddPrintData(strPrintData, MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID));
			CString strAPPID = MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID);
			CString strAPPName = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_EMV_APPLICATION_NAME);
			if ((strAPPName.GetLength() > 18) || (strAPPID.GetLength() > 20))
			{
				CString sAllSpace = L"                                        ";
				// AID
				strTemp.Format(_T("AID%s%s"), sAllSpace.Left(40 - 3 - strAPPID.GetLength()), strAPPID);
				DUALPRINT(strPrintData, nDualReceiptLang, strTemp)

				// APP Name
				strTemp.Format(_T("%s%s"), sAllSpace.Left(40 - strAPPName.GetLength()), strAPPName);
				DUALPRINT(strPrintData, nDualReceiptLang, strTemp)
			}
			else
			{
				strTemp.Format(_T("%-18.18s= %-20.20s"), strAPPName, strAPPID);
				DUALPRINT(strPrintData, nDualReceiptLang, strTemp)
			}
			// End of [#2252]
		}

		NHDEBUG(DBG_INFO, (L"[PRINT]-[TRANSACTION TYPE]\n"));
		// TRANSACTION TYPE
		{
			// TRANSACTION
			if (m_DualBalance.m_bDualBalance == TRUE)
			{
				DUALPRINT(strPrintData, nDualReceiptLang, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_016));
			}
			else
			{
				if (strTranType == TRANTYPE_WITHDRAWAL)
				{
					// [#2350] US Justin 2015.06.19 Add POP Money
					//strTemp2 = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_001);
					if (nWithdrawalType == WITHDRAWAL_POPMONEY)
					{
						DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_017));	// CDLS4 Withdrawal
					}
					else if (nWithdrawalType == WITHDRAWAL_PIN4)										// [#2396] US Justin 2016.02.17 Pin4	HalCash Withdrawal
					{
						DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_019));
					}
					else if (nWithdrawalType == WITHDRAWAL_JUSTCASH)									// [#2445] US Justin 2016.09.28 Just.Cash
					{
						DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_025));
					}
					else
					{
						DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_001));	// Withdrawal
					}
					// End of [#2350] 
				}
				else if (strTranType == TRANTYPE_INQUIRY)
				{
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_002));
				}
				else if (strTranType == TRANTYPE_TRANSFER)
				{
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_003));
				}
				else if (strTranType == TRANTYPE_PINCHANGE)											// [#2150] US Justin 2012.10.04 Add Pin Change, DCC Transaction	
				{
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_014));
				}
				else if (strTranType == TRANTYPE_DCC)
				{
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_001));	// [#2283] Justin "Currency Conversion" => "Withdrawal" (MasterCard requirement)
				}
				else if (strTranType == TRANTYPE_PAYPAL)
				{
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_026));	// [#2446] US Justion 2016.09.30 Paypal CCA
				}
				// [#2496] US Justin 2017.08.17 Just.Cash Bitcoin
				else if (strTranType == TRANTYPE_NONCASH_WITHDRAW)
				{
					strTemp2 = L"";
#if (APP_JUST_CASH)
					if (nWithdrawalType == WITHDRAWAL_DIGITAL_CUR_JC || nWithdrawalType == WITHDRAWAL_DIGITAL_CUR_CASH_JC)
					{
						if (m_JustCashData.m_strCurrencyType == JUSTCASH_CURRENCY_BITCOIN)
						{
							DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_027));
						}
						else if (m_JustCashData.m_strCurrencyType == JUSTCASH_CURRENCY_ETHEREUM)
						{
							DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_028));
						}
						else
						{
							DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), L"");
						}
					}
#endif
				}
				// End of [#2496]
				else
				{
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), L"");
				}
			}

			// [#2292] US Justin 2014.10.10 Dual Balance : PRINT TRANSACTION TYPE => DUAL
			// End of [#2292]
		}

		NHDEBUG(DBG_INFO, (L"[PRINT]-[ACCOUNT]\n"));
		// ACCOUNT
		//------------------------------------------------------------------------------------------------
		if ((strTranType != TRANTYPE_PINCHANGE)			&&			// [#2150] US Justin 2012.10.04. PIN Change => not print Account information
			(nWithdrawalType != WITHDRAWAL_PIN4)		&&			// [#2396] US Justin 2016.02.17 Remove Account Type for Pin4
			(nWithdrawalType != WITHDRAWAL_JUSTCASH)	&&			// [#2445] US Justin 2016.09.28 Remove Account Type for Just.Cash
			(nWithdrawalType != WITHDRAWAL_DIGITAL_CUR_CASH_JC))	// [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash
		{
			CString strFrom, strTo, strFromDual, strToDual;

			if (bIsDualReceiptEnabled && !bIsDualReceiptSameAsCurrent)
			{
				int currentLang = m_pDevCmn->fnSCR_GetCurrentLangMode();
				m_pDevCmn->fnSCR_SetCurrentLangMode(nDualReceiptLang);

				// Source Account Information
				if (m_SourceAccount == S_CHECKING)
				{
					strFromDual = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_007);
				}
				else if (m_SourceAccount == S_SAVINGS)
				{
					strFromDual = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_008);
				}
				else if (m_SourceAccount == S_CREDITCARD)
				{
					strFromDual = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_009);
				}

				// Dest Account Information
				if (m_DestAccount == S_CHECKING)
				{
					strToDual = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_007);
				}
				else if (m_DestAccount == S_SAVINGS)
				{
					strToDual = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_008);
				}
				else if (m_DestAccount == S_CREDITCARD)
				{
					strToDual = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_009);
				}

				m_pDevCmn->fnSCR_SetCurrentLangMode(currentLang);
			}

			// Source Account Information
			if (m_SourceAccount == S_CHECKING)
			{
				strFrom = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_007);
			}
			else if (m_SourceAccount == S_SAVINGS)
			{
				strFrom = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_008);
			}
			else if (m_SourceAccount == S_CREDITCARD)
			{
				strFrom = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_009);
			}

			// Dest Account Information
			if (m_DestAccount == S_CHECKING)
			{
				strTo = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_007);
			}
			else if (m_DestAccount == S_SAVINGS)
			{
				strTo = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_008);
			}
			else if (m_DestAccount == S_CREDITCARD)
			{
				strTo = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_009);
			}

			if (strTranType == TRANTYPE_TRANSFER)
			{
				// Transfer 거래인 경우에는 From / To를 2Line으로 표시
				strTemp2.Format(L"%-s %-s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_005), strFrom);
				AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_008), strTemp2);

				strTemp2.Format(L"%-s %-s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_006), strTo);
				AddPrintData(strPrintData, L"%-20.20s%-20.20s", L"", strTemp2);

				if (bIsDualReceiptEnabled && !bIsDualReceiptSameAsCurrent)
				{
					int currentLang = m_pDevCmn->fnSCR_GetCurrentLangMode();
					m_pDevCmn->fnSCR_SetCurrentLangMode(nDualReceiptLang);

					strTemp2.Format(L"%-s %-s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_005), strFromDual);
					AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_008), strTemp2);

					strTemp2.Format(L"%-s %-s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_006), strToDual);
					AddPrintData(strPrintData, L"%-20.20s%-20.20s", L"", strTemp2);

					m_pDevCmn->fnSCR_SetCurrentLangMode(currentLang);
				}
			}
			// [#2150] US Justin 2012.10.09 Add DCC //[#2289] Add Charity	// [#2446] US Justin Add Paypal	[#2496] US Justin Add Bitcoin
			else if (strTranType == TRANTYPE_WITHDRAWAL || strTranType == TRANTYPE_INQUIRY || strTranType == TRANTYPE_DCC ||
				strTranType == TRANTYPE_PAYPAL || strTranType == TRANTYPE_NONCASH_WITHDRAW)
			{
				// [#2266] US Justin Remove "FROM" - Source Account.
				if (m_DualBalance.m_bDualBalance != TRUE)			// [#2292] US Justin 2014.10.10 Dual Balance : NOT PRINTING ACCOUNT
				{
					strTemp2.Format(L"%s", strFrom);
					AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_008), strTemp2);
					if (bIsDualReceiptEnabled && !bIsDualReceiptSameAsCurrent)
					{
						int currentLang = m_pDevCmn->fnSCR_GetCurrentLangMode();
						m_pDevCmn->fnSCR_SetCurrentLangMode(nDualReceiptLang);

						strTemp2.Format(L"%s", strFromDual);
						AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_008), strTemp2);

						m_pDevCmn->fnSCR_SetCurrentLangMode(currentLang);
					}
				}
			}
			else
			{
				strTemp2 = L"";
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_008), strTemp2);
			}
		}
		//#endif   // [#2351]
		// end of [#2288]

	}
	//------------------------------------------------------------------------------------------------

	NHDEBUG(DBG_INFO, (L"[PRINT]-[AMOUNT, BALANCE]\n"));
	// AMOUNT, BALANCE
	//------------------------------------------------------------------------------------------------
	{
		if (strTranType != TRANTYPE_PINCHANGE)				// [#2150] US Justin 2012.10.08 Prevent Double line of Pin change....
			AddPrintData(strPrintData, L"----------------------------------------");

		P_NH_NOR_AmountAndBalancePrintProc(strPrintData, 0, nDualReceiptLang);
	}
	//------------------------------------------------------------------------------------------------

	// Bank Name and Fee
	//------------------------------------------------------------------------------------------------
#if (MX_VERSION)	
	// [#2115] MX KSK 2012.02.04
	{
		P_MX_NOR_BankNameandFeePrintProc(strPrintData);

		// [#2147] MX KSK 2012.08.18 외부에서 Print하도록 이동
		// 2. ACCOUNT NUMBER
		//------------------------------------------------------------------------------------------------
		if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_RETURNED_ACCNT_NO).IsEmpty())
		{
			strTemp = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_RETURNED_ACCNT_NO);
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_027), strTemp);
		}
		//------------------------------------------------------------------------------------------------
		// end of [#2142]


		// 3. EMV TAG PRINT (ARQC, ARPC)
		if (m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			// 1. ARQC Value
			strTemp = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AC_REQ_9F26_VALUE);
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_028), strTemp);

			// 2. ARPC Value - Host에서 내려온 91값 중에 앞에 8BYTE만 Print (고객 요청사항)
			int nLen = 0;
			unsigned char szTemp[1024] = { 0, };

			strTemp.Empty();
			if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_IsuAuthData, &nLen, szTemp) == EMV_RSLT_OK)
			{
				strTemp = MakeUnPack(szTemp, nLen);
				strTemp.Format(L"%s", strTemp.Left(16));
			}
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_029), strTemp);
		}
		// end of [#2147]
	}
	// end of [#2115]
#elif(CA_VERSION || US_VERSION)				
	// [#2478] NH JUstin 2017.04.10 Print TSI, TVR, and TC on a receipt
	{
		//if (m_pDevCmn->fnMCU_IsEmvTransaction())
		if ((m_pDevCmn->fnMCU_IsEmvTransaction()) && (m_DualBalance.m_bDualBalance != TRUE))	// [#2502] US Justin 2017.08.24 Bug Fix - Clearing Communicatio Error after Pre-Dual Balance
		{
			CString strValue;
			int nLen = 0;
			unsigned char szTemp[1024] = { 0, };

			// TSI
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTemp.Empty();
			if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TSI, &nLen, szTemp) == EMV_RSLT_OK)
			{
				strValue = MakeUnPack(szTemp, nLen);
				if (nLen > 0)
					strTemp.Format(L"%s", strValue);
			}
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_042), strTemp);

			// TC or AAC : TNS (Interac) Request 2017.04.11 - Application Cryptogram(9F26) of the 2nd GenAC
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTemp.Empty();
			if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_AC, &nLen, szTemp) == EMV_RSLT_OK)
			{
				strValue = MakeUnPack(szTemp, nLen);
				if (nLen > 0)
					strTemp.Format(L"%s", strValue);
			}

			if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"97301" || m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"97302")			// AAC
			{
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_044), strTemp);
			}
			else
			{
				// TC
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_043), strTemp);
			}
			// DEBUG... Stored CryptoGram
			// AddPrintData(strPrintData, L"%-17.17s = %20.20s", L"1GenAC Cryptogram", MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AC_REQ_9F26_VALUE) );

			// TVR
			memset(szTemp, 0, sizeof(szTemp));
			strValue.Empty();
			strTemp.Empty();
			if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TVR, &nLen, szTemp) == EMV_RSLT_OK)
			{
				strValue = MakeUnPack(szTemp, nLen);
				if (nLen > 0)
					strTemp.Format(L"%s", strValue);
			}
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_045), strTemp);
		}
	}
	// End of [#2478]
#endif


#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
	NHDEBUG(DBG_INFO, (L"[PRINT]-[MARKETING MESSAGE]\n"));
	// MARKETING MESSAGE -- 값이 모두 SPACE인 경우 PRINT하지 않음
	//------------------------------------------------------------------------------------------------
	AddPrintData(strPrintData, L"");	// [#2362] AU KSK 2015.08.03 DC Payments 요청에 의해 SPACE 추가 (NH1800CE와 통일)
	for (i = 0; i < 4; i++)
	{
		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MARKETING_MESSAGE1 + i);
		strTemp.TrimLeft();

		if (strTemp.GetLength() > 0)
			AddPrintData(strPrintData, MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MARKETING_MESSAGE1 + i));
	}
	//------------------------------------------------------------------------------------------------
#endif

	NHDEBUG(DBG_INFO, (L"[PRINT]-[Print Host Message]\n"));
	// Print Host Message
	{
		if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
		{
			for (i = 0; i < m_strTritonPrtMsgArray.GetSize() && i < 4; i++)
				AddPrintData(strPrintData, L"%-40.40s", m_strTritonPrtMsgArray[i]);
		}
		// [#2182] NH Justin 2013.02.22 Add STD1 Print line
		else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
		{
			for (i = 0; i < m_strSTD1PrtMsgArray.GetSize() && i < 4; i++)
				AddPrintData(strPrintData, L"%-40.40s", m_strSTD1PrtMsgArray[i]);
		}
	}

	NHDEBUG(DBG_INFO, (L"[PRINT]-[Print Host Result and Error Message]\n"));
	// Print Host Result and Error Message
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"----------------------------------------");
		if (m_DualBalance.m_bDualBalance != TRUE)			// [#2292] US Justin 2014.10.10 Dual Balance : PRINT TRANSACTION RESULT
		{
			if (m_pDevCmn->fstrAPL_GetErrorCode().Left(1) == "D")
			{
				DUALPRINT(strPrintData, nDualReceiptLang, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_011));
			}
			// KSK 2011.03.11 Bug Fix	// [#2150] US Justin 2012.10.09 Add DCC		// [#2496] US Justin 2017.08.18 Add Bitcoin
			else if (strTranType == TRANTYPE_WITHDRAWAL || strTranType == TRANTYPE_DCC || (strTranType == TRANTYPE_NONCASH_WITHDRAW && nWithdrawalType != WITHDRAWAL_DIGITAL_CUR_CASH_JC))
			{
				int nRequestedAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
				int nDispensedAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

				if (nDispensedAmt <= 0)						// Full Reversal
				{
					// [2405] US Justin 2016.03.21 HalCash Online
					// AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_013));
					if (nWithdrawalType == WITHDRAWAL_PIN4)
					{
						DUALPRINT(strPrintData, nDualReceiptLang, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_020));
						DUALPRINT(strPrintData, nDualReceiptLang, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_021));
					}
					else
					{
						DUALPRINT(strPrintData, nDualReceiptLang, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_013));							// Full Reversal
					}
					// End of [#2405]
				}
				else if (nRequestedAmt > nDispensedAmt)		// Partial Reversal
				{
					// [2405] US Justin 2016.03.21 HalCash Online
					//AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_012));
					if (nWithdrawalType == WITHDRAWAL_PIN4)
					{
						DUALPRINT(strPrintData, nDualReceiptLang, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_022));
						DUALPRINT(strPrintData, nDualReceiptLang, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_023));
					}
					else
					{
						DUALPRINT(strPrintData, nDualReceiptLang, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_012));
					}
				}
				else
				{
					DUALPRINT(strPrintData, nDualReceiptLang, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_010));
				}
			}
			else
			{
				DUALPRINT(strPrintData, nDualReceiptLang, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_010));
			}

			if (!m_pDevCmn->fnAPL_CheckError())	// ERROR 존재
			{
				DUALPRINT(strPrintData, nDualReceiptLang, L"%s = %s-%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_018), m_pDevCmn->fstrAPL_GetErrorProCount(), m_pDevCmn->fstrAPL_GetErrorCode(1));

				// [#2190] US KSK 2013.05.01 Asian Language인경우에는 Device 미지원으로 인해 인자 안함
				if (m_pDevCmn->fnSCR_GetCurrentLangMode() == ENG_MODE || m_pDevCmn->fnSCR_GetCurrentLangMode() == SPN_MODE || m_pDevCmn->fnSCR_GetCurrentLangMode() == FRN_MODE)
					AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrAPL_GetErrorMessage());

				// end of [#2190]

				// Reversal 다중 장애 처리
				strTemp = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG);
				if (strTemp.IsEmpty() == FALSE)
				{
					CStringArray strMultiError;

					//if (strTemp.Left(1) != CDU_OVER_DISPENSE)
					if ((strTemp.Left(1) != CDU_OVER_DISPENSE) && (strTemp.GetAt(0) != UNIT_DELIMITER))  // [#2292] JUSTIN 2014.10.07 Leave DCC INFO
					{
						// [#2292] US Justin 2014.10.07 Remove DCC INFO
						int nLocRD = strTemp.Find(RMS_OTHERMSG_DELIMITER);
						if (nLocRD >= 0)
							strTemp = strTemp.Left(nLocRD);
						// End of [#2292]

						int nCount = SplitString(strTemp, UNIT_DELIMITER, strMultiError);

						// Error Code =  (Proc Count) - (Error Code)
						if (nCount >= 2)
						{
							DUALPRINT(strPrintData, nDualReceiptLang, L"%s = %1.1s-%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_018), strMultiError[0], strMultiError[1]);
						}

						// [#2190] US KSK 2013.05.01 Asian Language인경우에는 Device 미지원으로 인해 인자 안함
//						if (nCount >= 3)
//							AddPrintData(strPrintData, L"%s", strMultiError[2]);
						if (m_pDevCmn->fnSCR_GetCurrentLangMode() == ENG_MODE || m_pDevCmn->fnSCR_GetCurrentLangMode() == SPN_MODE || m_pDevCmn->fnSCR_GetCurrentLangMode() == FRN_MODE)
						{
							if (nCount >= 3) { ; }
							//AddPrintData(strPrintData, L"%s", strMultiError[2]);
						}
						// end of [#2190]
					}
				}
			}
		}
	}
	//------------------------------------------------------------------------------------------------

	NHDEBUG(DBG_INFO, (L"[PRINT]-[TAIL PRINT]\n"));
	// TAIL PRINT
	{
		//------------------------------------------------------------------------------------------------
		{
			strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER3);
			strTemp.TrimLeft();

			if (strTemp.GetLength() > 0)
			{
				AddPrintData(strPrintData, L" ");
				AddPrintData(strPrintData, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER3));
			}
		}
		//------------------------------------------------------------------------------------------------
	}

	NHDEBUG(DBG_INFO, (L"[PRINT]-[JNL, SEQUENCE NUMBER PRINT]\n"));
	// JNL, SEQUENCE NUMBER PRINT
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L" ");
		strTemp.Format(L"JNL.#: %4.4d", m_pDevCmn->m_JNLMgr.GetLastIndex());

		if (m_DualBalance.m_bDualBalance == TRUE)
			AddPrintData(strPrintData, L"%s", strTemp);
		else if (m_HostConfig == HC_DUALHOST)
			AddPrintData(strPrintData, L"SEQ.#: %4.4s %28.28s", m_sSTD1_TranResp.TranSequenceNo_4, strTemp);
		else
		{
			AddPrintData(strPrintData, L"SEQ.#: %4.4s %28.28s", m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig), strTemp);
		}
	}
	//------------------------------------------------------------------------------------------------

	// DCC Disclaimer.
	if (strTranType == TRANTYPE_DCC)
	{
		NHDEBUG(DBG_INFO, (L"[PRINT]-[DCC Disclaimer]\n"));
		int dccCustomer = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION);

		// [#RWC6-549] DCC Generalization - Visa check was different than during transaction, copied from TranBizProc.
		//bool isVisaDcc = m_strVG_MarkupRate.GetLength() > 0;

		// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
		BOOL bIsVisaDCC = FALSE;

		if (((m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCDisclaimerScheme == L"1") &&
			 (m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCMarkupRate.GetLength() > 0)) ||
			(m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRateMarkUp.GetLength() > 0) ||
			(m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRateMarkUp.GetLength() > 0))
			bIsVisaDCC = TRUE;
		// End of [#RWC6-2, #2585]
		// End of [#RWC6-549]

//		bIsVisaDCC = FALSE;   // RWC6-607 To test only , Set MC

		if (((m_sSTD1_DynamicFlowResp.nDCCTransaction == 2) && (m_bDCCDeclinedByOperator == FALSE)) ||
			((m_STD3_TDL_Data.m_bProceedDCC == TRUE) && (m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_OfferAcceptance == L"0")))
		{
			strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PRT_DCC_DECLINE, dccCustomer, bIsVisaDCC);
			strTemp = CenterLines(strTemp, 40, L"\\n");
			strTemp.Replace(L"\\n", L"\x1c");

			AddPrintData(strPrintData, L" ");
			AddPrintData(strPrintData, strTemp);
		}
		else
		{
			if (bIsVisaDCC)
				strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PRT_DCC_DISCLAIMER, dccCustomer, bIsVisaDCC);
			else
				strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PRT_DCC_DISCLAIMER_MC, dccCustomer, bIsVisaDCC);

			strTemp = CenterLines(strTemp, 40, L"\\n");
			strTemp.Replace(L"\\n", L"\x1c");

			AddPrintData(strPrintData, L" ");
			AddPrintData(strPrintData, strTemp);

			if (bIsDualReceiptEnabled && !bIsDualReceiptSameAsCurrent)
			{
				if (bIsVisaDCC)
					strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PRT_DCC_DISCLAIMER, dccCustomer, bIsVisaDCC);
				else
					strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PRT_DCC_DISCLAIMER_MC, dccCustomer, bIsVisaDCC);

				strTemp = CenterLines(strTemp, 40, L"\\n");
				strTemp.Replace(L"\\n", L"\x1c");

				AddPrintData(strPrintData, L" ");
				AddPrintData(strPrintData, strTemp);
			}
		}
	}

	NHDEBUG(DBG_INFO, (L"[PRINT]-[COUPON]\n"));
	// COUPON PRINT 기능 추가
	{
		//------------------------------------------------------------------------------------------------
#if ( AU_VERSION)	// [#2069] NH KSK 2011.06.13
		if ((TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY) && (m_pDevCmn->fnAPL_CheckError()))
		{

			SYSTEMTIME	localTime;
			CString		strDateTime;

			::GetLocalTime(&localTime);
			strDateTime.Format(L"%s  %2.2d:%2.2d:%2.2d", GetCmnLocalDate(localTime.wMonth, localTime.wDay, localTime.wYear),
				localTime.wHour, localTime.wMinute, localTime.wSecond);
			DWORD dwValue = 0;
			BOOL bSupportAwardCouponBin = m_pDevCmn->m_BINMgr.GetTransactionBinProperties(ACTION_AWARD_COUPON, &dwValue);

			for (i = 0; i < 6; i++)
			{
				if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_ENDISFLAG + i) == ENABLE)
				{
					//AWARD COUPON이 셋팅 안 되어 있거나,AWARD COUPON 셋팅되어 있고 해당 BIN이 AWARD COUPON과 연관된 BIN일 경우 
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AWARD_COUPON1_ENABLE + i) != ENABLE
						|| (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AWARD_COUPON1_ENABLE + i) == ENABLE && bSupportAwardCouponBin == TRUE))
					{

						AddPrintData(strPrintData, L" ");
						AddPrintData(strPrintData, L" ");

						DUALPRINT(strPrintData, nDualReceiptLang, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_019));

						AddPrintData(strPrintData, strDateTime);

						DUALPRINT(strPrintData, nDualReceiptLang, L"%s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_001), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID));

						DUALPRINT(strPrintData, nDualReceiptLang, L"%s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_002), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SEQUENCENO)

							AddPrintData(strPrintData, L" ");
						AddPrintData(strPrintData, L"***************************************");
						AddPrintData(strPrintData, MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_TEXT1 + (i * 3)));
						AddPrintData(strPrintData, MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_TEXT2 + (i * 3)));
						AddPrintData(strPrintData, L"***************************************");
					}
				}
			}
		}
#else
		if ((TranCode == TC_WITHDRAWAL) && (m_pDevCmn->fnAPL_CheckError()))
		{
			SYSTEMTIME	localTime;
			CString		strDateTime;

			::GetLocalTime(&localTime);
			strDateTime.Format(L"%s  %2.2d:%2.2d:%2.2d", GetCmnLocalDate(localTime.wMonth, localTime.wDay, localTime.wYear),
				localTime.wHour, localTime.wMinute, localTime.wSecond);

			int bReceiptCutFlag = MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_CUT_OPTION);

			if (MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_ENABLE) == ENABLE)			// Enhanced Coupon
			{
				for (i = 0; i < 6; i++)
				{
					if (m_strarrEnhancedCouponData[i].GetSize() > 0)
					{
						if (bReceiptCutFlag == ENABLE)
						{
							// Cut 후에 Enhanced Coupon Data를 Print한다.
							NHDEBUG(DBG_INFO, (L"[PRINT]-[fnSPR_PrintReceipt]\n"));
							if (m_pDevCmn->fnSPR_PrintReceipt(TRUE, strPrintData, K_1_WAIT, TRUE, TRUE) != TRUE)		// Cut, Print BCD
								return RES_NG;		// NORMAL TRANSACTION ERROR 시 ERROR return

							strPrintData.Empty();
							bPrintTransData = TRUE;
						}

						// Cut Option이 AFTER RECEIPT인 경우에는 최초 Coupon에는 CUT HERE을 Print하지 않음
						if (bReceiptCutFlag == FALSE)
						{
							//AddPrintData(strPrintData, L" ");											// [#2373] US Justin Remove Empty Line
							AddPrintData(strPrintData, L" ");
							DUALPRINT(strPrintData, nDualReceiptLang, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_019));
						}

						bReceiptCutFlag = FALSE;	// Transaction Receipt만 Cut 하는 Option임

						AddPrintData(strPrintData, strDateTime);

						DUALPRINT(strPrintData, nDualReceiptLang, L"%s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_001), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID));

						DUALPRINT(strPrintData, nDualReceiptLang, L"%s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_002), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SEQUENCENO));

						AddPrintData(strPrintData, L" ");
						//AddPrintData(strPrintData, L"***************************************");		// [#2373] US Justin Remove Empty Line

						int j = 0;
						for (j = 0; j < m_strarrEnhancedCouponData[i].GetSize(); j++)
							AddPrintData(strPrintData, L"%s", m_strarrEnhancedCouponData[i][j]);

						//AddPrintData(strPrintData, L"***************************************");		// [#2373] US Justin Remove Empty Line
					}
				}
			}
			else		// Basic Coupon
			{
				for (i = 0; i < 6; i++)
				{
					if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_ENDISFLAG + i) == ENABLE)
					{
						AddPrintData(strPrintData, L" ");
						AddPrintData(strPrintData, L" ");

						DUALPRINT(strPrintData, nDualReceiptLang, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_019));

						AddPrintData(strPrintData, strDateTime);

						DUALPRINT(strPrintData, nDualReceiptLang, L"%s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_001), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID));

						DUALPRINT(strPrintData, nDualReceiptLang, L"%s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_002), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SEQUENCENO));

						AddPrintData(strPrintData, L" ");
						AddPrintData(strPrintData, L"***************************************");
						AddPrintData(strPrintData, MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_TEXT1 + (i * 3)));
						AddPrintData(strPrintData, MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_TEXT2 + (i * 3)));
						AddPrintData(strPrintData, L"***************************************");
					}
				}
			}
		}
#endif
	}
	//------------------------------------------------------------------------------------------------


	NHDEBUG(DBG_INFO, (L"[PRINT]-[fnSPR_PrintReceipt]\n"));

#if (AU_VERSION)
	// [#2235] AU KMK 2013.11.30 Digital Receipt 사용 시 명세표 예외처리 수행 안함
	if (m_sUserSelection.nPrintReceipt != RCPT_QRCODE)
		// end of [2235]
	{
		if (m_pDevCmn->fnSPR_PrintReceipt(m_bPrintImage, strPrintData, K_1_WAIT) != TRUE)
		{
			// [#2054] AU KSK 2011.05.04 조회거래에서 명세표 장애 시 End Sensor에 용지가 없으면 정상처리, 있으면 Reversal 처리 하도록 수정
			if (m_pDevCmn->fnSPR_GetPaperStatus() == SLIP_EMPTY_PAPER)
				return RES_OK;
			else
				return RES_NG;
			// end of [#2054]
		}
	}
	// [#2235] AU KMK 2013.11.30 호주 QRCODE 생성 로직 허용
#else
	if ((strPrintData.GetLength() > 0) && (m_sUserSelection.nPrintReceipt != RCPT_QRCODE))
	{
		if (bPrintTransData == FALSE)
		{
			// Transaction Data Print
			if (m_pDevCmn->fnSPR_PrintReceipt(m_bPrintImage, strPrintData, K_1_WAIT, bCut, TRUE) != TRUE)				// Cut,  Print BCD
				return RES_NG;
		}
		else
		{
			// Enhanced Coupon Print (Error와 상관없이 정상 처리)
			m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT, TRUE, TRUE);									// Cut,  Print BCD
		}
	}
	// [#2235] AU KMK 2013.11.30 호주 QRCODE 생성 로직 허용
	// #endif
	// end of [#2235]
#endif
// end of [#2235]

	// [#2219] 2013.09.04 NH Justin Digital Receipt
	//-----------------------------------------------------------------------------------------------
	if (m_sUserSelection.nPrintReceipt == RCPT_QRCODE)
		m_pDevCmn->fnSCR_SetCurrentLangMode(nSavedLangMode);

	if ((strPrintData.GetLength() > 0) && (m_sUserSelection.nPrintReceipt == RCPT_QRCODE))
	{
		// QREncode Generator Parameter
		//CString strFileQREncoder = _T("\\ATM\\QREncode.exe");
		//CString strFileOutputBMP = _T("\\ATM\\SCREEN\\AP_QRCode.BMP");
		CString strLightenedPrintData, strTempLine;
		CString strFieldDelimiter = _T("");
		CString strQRDelimiter = _T(";");
		strFieldDelimiter.Format(L"%c", FIELD_DELIMITER);

		// QR Code Format Version
		strLightenedPrintData = _T("V=1");			// initial version
		strLightenedPrintData += strQRDelimiter;

		// [#2346] NH Justin 2015.05.26 Add Country Code and Address on "Digital Receipt"
		// Country Code
		{
#if (AU_VERSION)
			strLightenedPrintData += _T("CC=AU");
#elif (CA_VERSION)
			strLightenedPrintData += _T("CC=CA");
#elif (MX_VERSION)
			strLightenedPrintData += _T("CC=MX");
#else
			strLightenedPrintData += _T("CC=US");
#endif
		}

		// Address
		for (i = 0; i < 4; i++)
		{
			strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1 + i);
			strTemp.Replace(L";", L";;");		// ';' is used for field separator, so change it to ';;'.
			strTemp2 = strTemp;
			strTemp.TrimLeft();
			strTemp2.TrimRight();
			if ((strTemp.GetLength() > 0) && (strTemp2.GetLength() > 0))
			{
				strTempLine.Format(L"%sA%d=%s", strQRDelimiter, i + 1, strTemp2);
				strLightenedPrintData += strTempLine;
			}
		}
		// End of [#2346]

		// Transaction Result
		CStringArray strTempArrayOne, strTempArrayTwo;
		SplitString(strPrintData, strFieldDelimiter, strTempArrayOne);
		for (int i = 0; i < strTempArrayOne.GetSize(); i++)
		{
			CString sOnePrintLine = strTempArrayOne.GetAt(i);
			sOnePrintLine.Trim();
			strTempLine = _T("");
			strTempArrayTwo.RemoveAll();
			SplitString(sOnePrintLine, _T("="), strTempArrayTwo);

			if (strTempArrayTwo.GetSize() == 2)		// "=" Exists.
			{
				CString strStr1 = strTempArrayTwo.GetAt(0);
				CString strStr2 = strTempArrayTwo.GetAt(1);
				strStr1.Trim();
				strStr2.Trim();

				if (strStr1.CompareNoCase(_T("TERMINAL  #")) == 0)			strTempLine = _T("TN");
				else if (strStr1.CompareNoCase(_T("SEQUENCE  #")) == 0)			strTempLine = _T("SN");
				else if (strStr1.CompareNoCase(_T("AUTH.     #")) == 0)			strTempLine = _T("AN");
				else if (strStr1.CompareNoCase(_T("DATE & TIME")) == 0)			strTempLine = _T("DT");
				else if (strStr1.CompareNoCase(_T("BUSINESS DATE")) == 0)		strTempLine = _T("BD");
				else if (strStr1.CompareNoCase(_T("CARD NUMBER")) == 0)
				{
					strTempLine = _T("CN");
					strStr2 = strStr2.Right(4);
				}
				else if (strStr1.CompareNoCase(_T("TRANSACTION")) == 0)
				{
					strTempLine = _T("TT");
					if (strTranType == TRANTYPE_WITHDRAWAL)			strStr2 = _T("W");
					else if (strTranType == TRANTYPE_INQUIRY)		strStr2 = _T("B");
					else if (strTranType == TRANTYPE_TRANSFER)		strStr2 = _T("T");
					//else if (strTranType == TRANTYPE_DCC)			strStr2 = _T("C");
					else if (strTranType == TRANTYPE_DCC)			strStr2 = _T("W");		// [#2283] Justin "Currency Conversion" => "Withdrawal" (MasterCard requirement)
				}
				else if (strStr1.CompareNoCase(_T("ACCOUNT")) == 0)
				{
					strTempLine = _T("AC");
					if (m_SourceAccount == S_CHECKING)				strStr2 = _T("C");
					else if (m_SourceAccount == S_SAVINGS)			strStr2 = _T("S");
					else if (m_SourceAccount == S_CREDITCARD)		strStr2 = _T("R");
					if (strTranType == TRANTYPE_TRANSFER)
					{
						if (m_DestAccount == S_CHECKING)			strStr2 += _T("C");
						else if (m_DestAccount == S_SAVINGS)		strStr2 += _T("S");
						else if (m_DestAccount == S_CREDITCARD)		strStr2 += _T("R");
						i++;
					}
				}
				else if (strStr1.CompareNoCase(_T("DISPENSED AMOUNT")) == 0)	strTempLine = _T("DA");
				else if (strStr1.CompareNoCase(_T("WITHDRAWAL")) == 0)			strTempLine = _T("WA");		// DCC CIBC Option (Same as Dispensed Amount)
				else if (strStr1.CompareNoCase(_T("REQUESTED AMOUNT")) == 0)	strTempLine = _T("RA");
				else if (strStr1.CompareNoCase(_T("ATM FEE")) == 0)				strTempLine = _T("SA");		// SURCHARGE AMOUNT -> ATM FEE
				else if (strStr1.CompareNoCase(_T("ATM OPERATOR FEE")) == 0)	strTempLine = _T("SA");		// KSK 2015.07.15 AU인 경우 문구 mismatch로 Digital Receipt에 수수료 Field가 표시되지 않는 Bug Fix
				else if (strStr1.CompareNoCase(_T("SERVICE FEE")) == 0)			strTempLine = _T("SF");		// DCC CIBC Option (Same as Surcharge Amount)
				else if (strStr1.CompareNoCase(_T("TOTAL AMOUNT")) == 0)		strTempLine = _T("TA");
				else if (strStr1.CompareNoCase(_T("ACCOUNT CHARGE")) == 0)		strTempLine = _T("AH");		// DCC Account Charge
				else if (strStr1.CompareNoCase(_T("LEDGER BALANCE")) == 0)		strTempLine = _T("LB");
				else if (strStr1.CompareNoCase(_T("BALANCE")) == 0)				strTempLine = _T("LB");		// KSK 2015.07.15 AU인 경우 문구 mismatch로 Digital Receipt에 Balance Field가 표시되지 않는 Bug Fix
				else if (strStr1.CompareNoCase(_T("AVAILABLE BALANCE")) == 0)	strTempLine = _T("AB");
				else if (strStr1.CompareNoCase(_T("TRANSFER AMOUNT")) == 0)		strTempLine = _T("FA");

				if ((strTempLine.GetLength() == 2) && (strStr2.GetLength() > 0))
					strTempLine += _T("=") + strStr2;
			}
			else if (strTempArrayTwo.GetSize() == 1)
			{
				CString strStr = strTempArrayTwo.GetAt(0);
				CString strStr2 = _T("");

				if (strStr.CompareNoCase(_T("ATM FEE PAID TO:")) == 0 || strStr.CompareNoCase(_T("ATM OPERATOR FEE PAID TO:")) == 0)	// SURCHARGE PAID TO: -> ATM FEE PAID TO:
				{
					strTempLine.Format(_T("SP=%s"), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER));
					i++;
				}
				else if (strStr.CompareNoCase(_T("APPROVED")) == 0)				strTempLine = _T("TR=A");
				else if (strStr.CompareNoCase(_T("DECLINED")) == 0)				strTempLine = _T("TR=D");
				else if (strStr.CompareNoCase(_T("PARTIAL REVERSAL")) == 0)		strTempLine = _T("TR=P");
				else if (strStr.CompareNoCase(_T("FULL REVERSAL")) == 0)		strTempLine = _T("TR=F");
				// Check Journal Number
				else
				{
					CString strFirstThreeChar = strStr.Left(3);
					if (strFirstThreeChar.CompareNoCase(_T("SEQ")) == 0)
						strTempLine.Format(_T("JN=%s"), strStr.Right(4));

					// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message
					CString strFirstSixteenChar = strStr.Left(16); // trying to differenciate from "'EXCHANGE RATE MA'RK-UP", it is possible exchange rate string length can be longer than 21
					if (strFirstSixteenChar.Trim().CompareNoCase(_T("EXCHANGE RATE")) == 0)
					{
						strStr2.Format(_T("%s"), strStr.Right(strStr.GetLength() - 13));
						strStr2.Trim();
						strTempLine = _T("ER=") + strStr2;  // DCC Exchange Rate 
					}
					else if (strFirstSixteenChar.CompareNoCase(_T("EXCHANGE RATE MA")) == 0)
					{
						strStr2.Format(_T("%s"), strStr.Right(strStr.GetLength() - 21));
						strStr2.Trim();
						strTempLine = _T("EM=") + strStr2;  // DCC Exchange Rate Mark-up
					}
					// end of [#RWC6-2, #2585]
				}
			}

			if (strTempLine.GetLength() > 0)
			{
				strLightenedPrintData += strQRDelimiter;
				strLightenedPrintData += strTempLine;
			}
		}

		// [#2360] US Justin 2015.07.15 Disable Disclaimer on a receipt
		/*
		// DCC Disclaimer ID
		if( m_sSTD1_DynamicFlowResp.nDCCTransaction==1 )
		{
			strLightenedPrintData += strQRDelimiter;
			#if (APP_CUSTOM_PAI)
				strLightenedPrintData += _T("DD=2");
			#else
				strLightenedPrintData += _T("DD=1");
			#endif
		}
		*/
		// End of [#2360]

		CString strParam = _T("");
		strParam.Format(L"\"%s\" %s", strLightenedPrintData, QR_RECEIPT_IMAGEFILE);

		// Delete Existing QR Image
		DeleteFile(QR_RECEIPT_IMAGEFILE);

		MakeQRCodeImageFile(strLightenedPrintData, QR_RECEIPT_IMAGEFILE);
	}
	//-----------------------------------------------------------------------------------------------
	// End of [#2219]

	return RES_OK;
}

void CTranCmn::LIB_GetDisplayData(CString& strPrintData)
{
	CString		strTemp, strTemp2, strTemp3;
	CString		strTranType;

	int nRequestedAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
	int nDispensedAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));
	int nSurchargeAmt = Asc2Int(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT));

	strTranType.Format(L"%2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE));
	int nWithdrawalType = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE);

#if (MX_VERSION)	// [#2115] MX KSK 2012.02.02: IVA TAX 정수로 변경
	int	nIVATAXSurchargeAmt = 0;
	int nLenIVATAX = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX).GetLength(); // IVA TAX

	CString strIVATAX = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX);
	strIVATAX.Remove('.');
	nIVATAXSurchargeAmt = Asc2Int(strIVATAX);
#endif				// end of [#2115]

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	if (strTranType == TRANTYPE_WITHDRAWAL && nRequestedAmt > nDispensedAmt)	nSurchargeAmt = 0;
	else if (strTranType == TRANTYPE_INQUIRY && !m_pDevCmn->fnAPL_CheckError())	nSurchargeAmt = 0;
#endif
	// end of [#2031]

	// ACCOUNT
	// [#2150] US Justin 2012.10.09 Add DCC // [#2446] Justin Add Paypal  // [#2496] US Justin Add Bitcoin
	if (strTranType == TRANTYPE_INQUIRY || strTranType == TRANTYPE_TRANSFER || strTranType == TRANTYPE_WITHDRAWAL ||
		strTranType == TRANTYPE_DCC || strTranType == TRANTYPE_PAYPAL ||
		strTranType == TRANTYPE_NONCASH_WITHDRAW)
	{
		CString strFrom, strTo;

		if (m_SourceAccount == S_CHECKING)
			strFrom = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_007);
		else if (m_SourceAccount == S_SAVINGS)
			strFrom = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_008);
		else if (m_SourceAccount == S_CREDITCARD)
			strFrom = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_009);

		// Dest Account Information
		if (m_DestAccount == S_CHECKING)
			strTo = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_007);
		else if (m_DestAccount == S_SAVINGS)
			strTo = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_008);
		else if (m_DestAccount == S_CREDITCARD)
			strTo = m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_009);

		if (strTranType == TRANTYPE_TRANSFER)
		{
			strTemp2.Format(L"%s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_005), strFrom);
			AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_008), strTemp2);

			strTemp2.Format(L"%s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_006), strTo);
			AddPrintData(strPrintData, L"%-20.20s%-20.20s", L"", strTemp2);
		}
		// [#2150] US Justin 2012.10.09 Add DCC 
		// [#2446] US Justin Add Paypal 
		// [#2496] US Justin Add Bitcoin
		else if (strTranType == TRANTYPE_WITHDRAWAL || strTranType == TRANTYPE_INQUIRY || strTranType == TRANTYPE_DCC ||
			strTranType == TRANTYPE_PAYPAL || strTranType == TRANTYPE_NONCASH_WITHDRAW)
		{
			// [#2292] US Justin 2014.10.10 Dual Balance : DISPLAY TRANSACTION TYPE => DUAL
			if (m_DualBalance.m_bDualBalance == TRUE)
				AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_016));
			else
			{
				// [#2405] US Justin 2016.03.21 Removing Account info for PIN4 Transaction
				if ((nWithdrawalType != WITHDRAWAL_PIN4) && (nWithdrawalType != WITHDRAWAL_JUSTCASH))
				{
					strTemp2.Format(L"%s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_005), strFrom);
					AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_008), strTemp2);
				}
			}
		}
		else
		{
			strTemp2 = L"";
			AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_008), strTemp2);
		}
	}

	// [#RWC6-16] Bitload 4 U. A new cryptocurrency feature for CE 6.0 ATMs
#if (US_VERSION)
#if (APP_B4U)
	if (strTranType == TRANTYPE_B4U)
	{
		if (m_sUserSelection.nPrintReceipt == RCPT_PAPER)	//  MAX CHAR: 40
		{
			// TRANSACTION =
			AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_FINANCIAL));

			// DATE = 
			AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_004), m_B4USession->ReqDateTime);

			// TERMINAL ID =
			AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TML), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID));

			// BTC EXCHANGE TRANSACTION INFO
			AddPrintData(strPrintData, L"---------------------------------------");
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_TXN_INFO));
			AddPrintData(strPrintData, L"---------------------------------------");

			// BTC ADDRESS = 
			strTemp.Format(L"%s", m_B4USession->BitcoinAddr);
			AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_ADDRESS), strTemp.Left(20));
			int nLen = strTemp.GetLength();
			int nLine = nLen / 20;
			int nRemain = nLen % 20;
			for (int i = 1; i < nLine + 1; i++)
			{
				if (i + 1 > nLine)
				{
					if (nRemain > 0)
					{
						AddPrintData(strPrintData, L"%-20.20s%-20.20s", L" ", strTemp.Mid(i * 20, nRemain));
					}
				}
				else
				{
					AddPrintData(strPrintData, L"%-20.20s%s%-20.20s", L" ", strTemp.Mid(i * 20, 20));
				}
			}
			AddPrintData(strPrintData, L"%-40.40s", L"");

			// TRANSACTION ID = 
			strTemp.Format(L"%s", m_B4USession->EncTransactionId);
			AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_TXN_ID), strTemp.Left(20));
			nLen = strTemp.GetLength();
			nLine = nLen / 20;
			nRemain = nLen % 20;
			for (int i = 1; i < nLine + 1; i++)
			{
				if (i + 1 > nLine)
				{
					if (nRemain > 0)
					{
						AddPrintData(strPrintData, L"%-20.20s%-20.20s", L" ", strTemp.Mid(i * 20, nRemain));
					}
				}
				else
				{
					AddPrintData(strPrintData, L"%-20.20s%s%-20.20s", L" ", strTemp.Mid(i * 20, 20));
				}
			}
			AddPrintData(strPrintData, L"%-40.40s", L"");

			// SEQUENCE # = 
			AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_035), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SEQUENCENO));

			// AMOUNT REQUESTED =
			strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT)));
			AddPrintData(strPrintData, L"%17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_010), strTemp);

			// AMOUNT DISPENSED =
			strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT)));
			AddPrintData(strPrintData, L"%17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_009), strTemp);

			// BTC CHARGED =
			strTemp.Format(L"%s(BTC)", m_B4USession->BitcoinAmountDisplay);
			AddPrintData(strPrintData, L"%17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_CHARGED), m_B4USession->BitcoinAmountDisplay);

			// FEE CHARGED =
			strTemp.Format(L"%s%s", GetCurrencySymbol(), m_B4UIni->FeeDisplay);
			AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_FEE_CHARGED), strTemp);

			// BTC RATE = 
			//strTemp.Format(L"1(BTC):$%s(USD)", m_B4USessionStatus->Usdvalue);	// not sure if this is BTC rate, need to contact B4U for this
			//AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_RATE), strTemp);

			// TRANSACTION HASH = 
			strTemp.Format(L"%s", m_B4USessionStatus->TransactionHash);
			AddPrintData(strPrintData, L"%-17.17s = %-20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_TXN_HASH), strTemp.Left(20));
			nLen = strTemp.GetLength();
			nLine = nLen / 20;
			nRemain = nLen % 20;
			for (int i = 1; i < nLine + 1; i++)
			{
				if (i + 1 > nLine)
				{
					if (nRemain > 0)
					{
						AddPrintData(strPrintData, L"%-20.20s%-20.20s", L" ", strTemp.Mid(i * 20, nRemain));
					}
				}
				else
				{
					AddPrintData(strPrintData, L"%-20.20s%s%-20.20s", L" ", strTemp.Mid(i * 20, 20));
				}
			}

			// TRANSACTION IS APPROVAED
			AddPrintData(strPrintData, L"%-40.40s", L"");
			AddPrintData(strPrintData, L"%-40.40s", L"");
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_TXN_APPROVED));
			AddPrintData(strPrintData, L"%-40.40s", L"");

			// Disclaimer 1
			AddPrintData(strPrintData, L"%-40.40s", L"");
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_1_01));
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_1_02));
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_1_03));
			AddPrintData(strPrintData, L"%-40.40s", L"");

			// Disclaimer 2
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_01));
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_02));
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_03));
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_04));
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_05));
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_06));
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_07));
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_08));
			AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_DISC_2_09));
			AddPrintData(strPrintData, L"%-40.40s", L"");
		}
		else	//  MAX CHAR: 75
		{
			// 1 TRANSACTION =
			AddPrintData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_007), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_FINANCIAL));

			// 2 DATE = 
			AddPrintData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_004), m_B4USession->ReqDateTime);

			// 3 TERMINAL ID =
			AddPrintData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TML),
				MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID));

			//// BTC EXCHANGE TRANSACTION INFO
			//AddPrintData(strPrintData, L"---------------------------------------");
			//AddPrintData(strPrintData, L"%-75.75s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_TXN_INFO));
			//AddPrintData(strPrintData, L"---------------------------------------");

			// 4 BTC ADDRESS = 
			strTemp.Format(L"%s", m_B4USession->BitcoinAddr);
			AddPrintData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_ADDRESS), strTemp);

			// 5 TRANSACTION ID = 
			strTemp.Format(L"%s", m_B4USession->EncTransactionId);
			AddPrintData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_TXN_ID), strTemp);

			// 6 SEQUENCE # = 
			AddPrintData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_035), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SEQUENCENO));

			// 7 AMOUNT REQUESTED =
			strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT)));
			AddPrintData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_010), strTemp);

			// 8 AMOUNT DISPENSED =
			strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT)));
			AddPrintData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_009), strTemp);

			// 9 BTC CHARGED =
			strTemp.Format(L"%s(BTC)", m_B4USession->BitcoinAmountDisplay);
			AddPrintData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_CHARGED), m_B4USession->BitcoinAmountDisplay);

			// 10 FEE CHARGED =
			strTemp.Format(L"%s%s", GetCurrencySymbol(), m_B4UIni->FeeDisplay);
			AddPrintData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_FEE_CHARGED), strTemp);

			// BTC RATE = 
			//strTemp.Format(L"1(BTC):$%s(USD)", m_B4USessionStatus->Usdvalue);	// not sure if this is BTC rate, need to contact B4U for this
			//AddPrintData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_RATE), strTemp);

			// 11 TRANSACTION HASH = 
			strTemp.Format(L"%s", m_B4USessionStatus->TransactionHash);
			AddPrintData(strPrintData, L"%-22.22s = %-50.50s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_BTC_TXN_HASH), strTemp);

			// 12 TRANSACTION IS APPROVAED
			AddPrintData(strPrintData, L"%-75.75s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_TXN_APPROVED));

			// 13, 14 Disclaimer 1
			AddPrintData(strPrintData, L"%-75.75s", L"Contact support@b4ufinancial.com if you didn?�t receive money owed for this");
			AddPrintData(strPrintData, L"%-75.75s", L"with the picture of this receipt.");

			// 15, 16, 17, 18, 19 Disclaimer 2
			AddPrintData(strPrintData, L"%-75.75s", L"Your transaction will be credited subject to verification, collection and");
			AddPrintData(strPrintData, L"%-75.75s", L"the Rules and Regulations for B4U Financial and as otherwise provided by");
			AddPrintData(strPrintData, L"%-75.75s", L"law. Update to your account balance is controlled by your wallet and the");
			AddPrintData(strPrintData, L"%-75.75s", L"blockchain and may require up 24 hours. Please retain this receipt until");
			AddPrintData(strPrintData, L"%-75.75s", L"you confirm receipt of funds.");
		}
	}
#endif
#endif
	// end of [[#RWC6-16]

	// Amount and Balance
	// [#2150] US Justin 2012.10.04 Add Pin Change Transaction
	if ((strTranType != TRANTYPE_PINCHANGE) && (strTranType != TRANTYPE_B4U))
	{
		P_NH_NOR_AmountAndBalancePrintProc(strPrintData, TRUE);
	}
	// End of [#2150]

	// Bank Name and Fee
	//------------------------------------------------------------------------------------------------
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
	{
		P_MX_NOR_BankNameandFeePrintProc(strPrintData);

		// [#2147] MX KSK 2012.08.18 외부에서 Print하도록 이동
		// 2. ACCOUNT NUMBER
		//------------------------------------------------------------------------------------------------
		if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_RETURNED_ACCNT_NO).IsEmpty())
		{
			strTemp = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_RETURNED_ACCNT_NO);
			AddPrintData(strPrintData, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_027), strTemp);
		}
		//------------------------------------------------------------------------------------------------
		// end of [#2142]


		// 3. EMV TAG PRINT (ARQC, ARPC)
		if (m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			// 1. ARQC Value
			strTemp = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AC_REQ_9F26_VALUE);

			AddPrintData(strPrintData, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_028), strTemp);

			// 2. ARPC Value - Host에서 내려온 91값 중에 앞에 8BYTE만 Print (고객 요청사항)
			int nLen = 0;
			unsigned char szTemp[1024] = { 0, };

			strTemp.Empty();
			if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_IsuAuthData, &nLen, szTemp) == EMV_RSLT_OK)
			{
				strTemp = MakeUnPack(szTemp, nLen);
				strTemp.Format(L"%s", strTemp.Left(16));
			}

			AddPrintData(strPrintData, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_029), strTemp);
		}
		// end of [#2147]
	}
#endif				// end of [#2115]

	NHDEBUG(DBG_INFO, (L"[PRINT]-[Print Host Result and Error Message]\n"));
	// Print Host Result and Error Message
	//------------------------------------------------------------------------------------------------
	// [#11] NH KSK 2010.10.01 Bug Fix
	if (m_DualBalance.m_bDualBalance != TRUE)			// [#2292] US Justin 2014.10.10 Dual Balance : PRINT TRANSACTION RESULT
	{
		CString strTemp;
		{
			if (m_pDevCmn->fstrAPL_GetErrorCode().Left(1) == "D")
				AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_011));
			// KSK 2011.03.11 Bug Fix	// [#2150] US Justin 2012.10.09 Add DCC		// [#2496] US Justin 2017.08.18 Add Bitcoin
			else if (strTranType == TRANTYPE_WITHDRAWAL || strTranType == TRANTYPE_DCC || strTranType == TRANTYPE_NONCASH_WITHDRAW)
			{
				int nRequestedAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
				int nDispensedAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

				if (nDispensedAmt <= 0)						// Full Reversal
				{
					// [2405] US Justin 2016.03.21 HalCash Online
					// AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_013));
					if (nWithdrawalType == WITHDRAWAL_PIN4)
					{
						AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_020));
						AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_021));
					}
					else
						AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_013));
					// End of [#2405]
				}
				else if (nRequestedAmt > nDispensedAmt)		// Partial Reversal
				{
					// [2405] US Justin 2016.03.21 HalCash Online
					// AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_012));
					if (nWithdrawalType == WITHDRAWAL_PIN4)
					{
						AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_022));
						AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_023));
					}
					else
						AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_012));
					// End of [#2405]
				}
				else
					AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_010));	// �??�에???�상?�로 간주?�다.
			}
			else
			{
				AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_010));	// �??�에???�상?�로 간주?�다.
			}

			if (!m_pDevCmn->fnAPL_CheckError())	// ERROR 존재
			{
				AddPrintData(strPrintData, L"%s = %s-%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_018),
					m_pDevCmn->fstrAPL_GetErrorProCount(),
					m_pDevCmn->fstrAPL_GetErrorCode(1));

				// [#2190] US KSK 2013.05.01 Asian Language인경우에는 Device 미지원으로 인해 인자 안함
				if (m_pDevCmn->fnSCR_GetCurrentLangMode() == ENG_MODE || m_pDevCmn->fnSCR_GetCurrentLangMode() == SPN_MODE || m_pDevCmn->fnSCR_GetCurrentLangMode() == FRN_MODE)
					AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrAPL_GetErrorMessage());
				// end of [#2190]

				// Reversal 다중 장애 처리
				strTemp = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG);

				if (strTemp.IsEmpty() == FALSE)
				{
					CStringArray strMultiError;

					//if (strTemp.Left(1) != CDU_OVER_DISPENSE)
					if ((strTemp.Left(1) != CDU_OVER_DISPENSE) && (strTemp.GetAt(0) != UNIT_DELIMITER))  // [#2292] JUSTIN 2014.10.07 Leave DCC INFO
					{
						// [#2292] US Justin 2014.10.07 Remove DCC INFO
						int nLocRD = strTemp.Find(RMS_OTHERMSG_DELIMITER);
						if (nLocRD >= 0)
							strTemp = strTemp.Left(nLocRD);
						// End of [#2292]

						int nCount = SplitString(strTemp, UNIT_DELIMITER, strMultiError);

						// Error Code =  (Proc Count) - (Error Code)
						if (nCount >= 2)
							AddPrintData(strPrintData, L"%s = %1.1s-%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_018), strMultiError[0], strMultiError[1]);

						// [#2190] US KSK 2013.05.01 Asian Language인경우에는 Device 미지원으로 인해 인자 안함
						if (m_pDevCmn->fnSCR_GetCurrentLangMode() == ENG_MODE || m_pDevCmn->fnSCR_GetCurrentLangMode() == SPN_MODE || m_pDevCmn->fnSCR_GetCurrentLangMode() == FRN_MODE)
						{
							if (nCount >= 3)
								AddPrintData(strPrintData, L"%s", strMultiError[2]);
						}
						// end of [#2190]
					}
				}
			}
		}
	}
	// end of [#11]
}

// BALANCE 처리를 위해 함수 생성
// return값은 증가된 index값이다
// balance 처리
// 0x80 -> STANDARD1, 2 EPS에서는 BALANCE UNAVAILABLE로 인자
// 0x01 -> LEDGER BALANCE 인자
// 0x10 -> AVAILABLE BALANCE 인자
void CTranCmn::P_NH_NOR_AmountAndBalancePrintProc(CString& strPrintData, int nDispScreen, int nDualReceiptLang)	// [#2147] MX KSK 2012.08.18
{
	CString	strLedgerBalance;
	CString	strAvailableBalance;
	CString	strTemp, strTemp2, strTemp3;
	CString strTranType;
	BYTE	BalProc = 0;

	int nRequestedAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
	int nDispensedAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));
	int nSurchargeAmt = Asc2Int(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT));
	int nTempCnt = 0;
	int nTempAmt = 0;
	int nTempDenom = 0;

	// Exchange Rate Mark-up
	bool isVisaDcc = ((m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCDisclaimerScheme == L"1") &&
		(m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCMarkupRate.GetLength() > 0))
		|| (m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRateMarkUp.GetLength() > 0)
		|| (m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRateMarkUp.GetLength() > 0);

	strTranType.Format(L"%2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE));
	int nWithdrawalType = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE);

	int nCustomerOption = -1;
	if (strTranType == TRANTYPE_DCC)
	{
		nCustomerOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION);
	}

// [#2115] MX KSK 2012.02.02: IVA TAX 정수로 변경
#if (MX_VERSION)
	int	nIVATAXSurchargeAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX));
	int nLenIVATAX = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX).GetLength(); // IVA TAX
#endif
// end of [#2115]

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	if (strTranType == TRANTYPE_WITHDRAWAL && nRequestedAmt > nDispensedAmt)	nSurchargeAmt = 0;
	else if (strTranType == TRANTYPE_INQUIRY && !m_pDevCmn->fnAPL_CheckError())	nSurchargeAmt = 0;
#endif
	// end of [#2031]

	// 명세표 표준화 적용 KSK 2010.04.19 (Title 왼쪽 정렬 / Value 우측 정렬)
	// DISPENSED AMOUNT
	//------------------------------------------------------------------------------------------------
	// [#2150] US Justin 2012.10.09 Add DCC
	// [#2446] US Justin Add Paypal
	// [#2496] US Justin Add Bit Coin (Do not print Dispensed Amount for Bit Coin)
	if (strTranType == TRANTYPE_WITHDRAWAL || strTranType == TRANTYPE_DCC || strTranType == TRANTYPE_PAYPAL ||
		(strTranType == TRANTYPE_NONCASH_WITHDRAW && nWithdrawalType != WITHDRAWAL_DIGITAL_CUR_JC && nWithdrawalType != WITHDRAWAL_DIGITAL_CUR_CASH_JC))
	{
		// [#2072] AU KSK 2011.06.17 명세표와 화면 View에만 'NZ$' 표시 (뉴질랜드 Option인 경우)
#if (AU_VERSION)
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
			strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nDispensedAmt)));
		else
			strTemp.Format(L"%s%s", NZ_CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nDispensedAmt)));

		DUALPRINT(strPrintData, nDualReceiptLang, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_009), strTemp);
#else
		if (strTranType == TRANTYPE_DCC)
		{
			if (nCustomerOption != DCC_CUSTOMOPTION_DEFAULT)
			{
				strTemp.Format(L"%s %s", MakeMoneyCent(Int2Asc(nDispensedAmt)), m_pDevCmn->m_strCurrencyID);
			}
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_856, nCustomerOption, isVisaDcc), strTemp);
#else
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_056, nCustomerOption, isVisaDcc), strTemp);
#endif
		}
		else
		{
			strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nDispensedAmt)));
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_009), strTemp);
		}
	}
#endif

	//------------------------------------------------------------------------------------------------

	/* // comment below until sale team approves
	// [#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA
	// DENOMINATION COUNT
	//------------------------------------------------------------------------------------------------
	if ( (strTranType == TRANTYPE_WITHDRAWAL) && (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DENOMINATION_SELECT_ENABLE) == ENABLE) )
	{
		if ( (m_sUserSelection.nCST1Denom > 0) && (m_sUserSelection.nCST1Cnt > 0) )
		{
			nTempDenom = m_sUserSelection.nCST1Denom;
			nTempCnt = m_pDevCmn->fnCDU_GetLastDispensedCount(1);
			nTempAmt = nTempDenom * nTempCnt;
			strTemp.Format(L" (%s%s   x  %s",  GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nTempDenom * 100)), Int2Asc(nTempCnt));
			strTemp2.Format(L"%s%s",  GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nTempAmt * 100)));
			AddPrintData(strPrintData, L"%-18.18s=%10.10s)    ", strTemp, strTemp2);
		}
		if ( (m_sUserSelection.nCST2Denom > 0) && (m_sUserSelection.nCST2Cnt > 0) )
		{
			nTempDenom = m_sUserSelection.nCST2Denom;
			nTempCnt = m_pDevCmn->fnCDU_GetLastDispensedCount(2);
			nTempAmt = nTempDenom * nTempCnt;
			strTemp.Format(L" (%s%s   x  %s",  GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nTempDenom * 100)), Int2Asc(nTempCnt));
			strTemp2.Format(L"%s%s",  GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nTempAmt * 100)));
			AddPrintData(strPrintData, L"%-18.18s=%10.10s)    ", strTemp, strTemp2);
		}
		if ( (m_sUserSelection.nCST3Denom > 0) && (m_sUserSelection.nCST3Cnt > 0) )
		{
			nTempDenom = m_sUserSelection.nCST3Denom;
			nTempCnt = m_pDevCmn->fnCDU_GetLastDispensedCount(3);
			nTempAmt = nTempDenom * nTempCnt;
			strTemp.Format(L" (%s%s   x  %s",  GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nTempDenom * 100)), Int2Asc(nTempCnt));
			strTemp2.Format(L"%s%s",  GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nTempAmt * 100)));
			AddPrintData(strPrintData, L"%-18.18s=%10.10s)    ", strTemp, strTemp2);
		}
		if ( (m_sUserSelection.nCST4Denom > 0) && (m_sUserSelection.nCST4Cnt > 0) )
		{
			nTempDenom = m_sUserSelection.nCST4Denom;
			nTempCnt = m_pDevCmn->fnCDU_GetLastDispensedCount(4);
			nTempAmt = nTempDenom * nTempCnt;
			strTemp.Format(L" (%s%s   x  %s",  GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nTempDenom * 100)), Int2Asc(nTempCnt));
			strTemp2.Format(L"%s%s",  GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nTempAmt * 100)));
			AddPrintData(strPrintData, L"%-18.18s=%10.10s)    ", strTemp, strTemp2);
		}
	}
	//------------------------------------------------------------------------------------------------
	// end of [#RWC6-12, #2584]
	*/ // comment above until sale team approves

	// REQUEST AMOUNT
	//------------------------------------------------------------------------------------------------
	// [#2150] US Justin 2012.10.09 Add DCC
	// [#2446] US Jusitn Add Paypal
	// [#2496] US Justin Add BITCOIN
	if (strTranType == TRANTYPE_WITHDRAWAL || strTranType == TRANTYPE_DCC || strTranType == TRANTYPE_PAYPAL ||
		strTranType == TRANTYPE_NONCASH_WITHDRAW)
	{
		// [#2072] AU KSK 2011.06.17 명세표와 화면 View에만 'NZ$' 표시 (뉴질랜드 Option인 경우)
#if (AU_VERSION)
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
			strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nRequestedAmt)));
		else
			strTemp.Format(L"%s%s", NZ_CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nRequestedAmt)));
#else
		if (nCustomerOption != DCC_CUSTOMOPTION_DEFAULT)
		{
			strTemp.Format(L"%s %s", MakeMoneyCent(Int2Asc(nRequestedAmt)),  m_pDevCmn->m_strCurrencyID);
		}
		else
		{
			strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nRequestedAmt)));
		}
#endif	// end of [#2072]

		DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_010), strTemp);

	}
	else if (strTranType == TRANTYPE_TRANSFER)
	{
		strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nRequestedAmt)));
		DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_011), strTemp);
	}
	//------------------------------------------------------------------------------------------------

	// Mexico Exchange Rate
	//------------------------------------------------------------------------------------------------
#if (MX_VERSION)		// [#2115] [MX] KSK 2012.02.04
	// [#2137] MX KSK 2012.07.25
	if ((strTranType == TRANTYPE_WITHDRAWAL) && (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE))
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//[#2137] MX PCS 2012.07.06
		if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_DOMESTIC_TYPE)
		{
			if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE).GetLength() > 0)
			{
				//13. EXCHANGE_RATE
				if (strTranType == TRANTYPE_DCC)
					strTemp.Format( _T("%s %s"), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE), GetCurrencySymbol(0, 0, MX_DOMESTIC_TYPE));
				else
					strTemp.Format(_T("%s%s"), CURRENCY_SYMBOL, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE));

				DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_022), strTemp);
			}
		}
		else
		{
			// International은 출력 안함
		}
	}
	// end of [#2137]
#endif					// end of [#2115]
	//------------------------------------------------------------------------------------------------

	// SURCHARGE AMOUNT
	//------------------------------------------------------------------------------------------------
	// [#2150] US Justin 2012.10.09 Add DCC
	// [#2446] US Justin Add Paypal
	if (strTranType == TRANTYPE_WITHDRAWAL || strTranType == TRANTYPE_DCC || strTranType == TRANTYPE_PAYPAL)
	{
#if (MX_VERSION) // [#2115] SURCHARGE 가 안오더라도 IVA TAX 인자해야함
		if (nDispensedAmt != 0)				// [#2147] MX KSK 2012.08.18 또다시 사양 변경으로 인해 Partial인 경우에도 Surcharge 인자하도록 변경
#else
		if ((nDispensedAmt != 0) && (nSurchargeAmt != 0))
#endif			// end of [#215]
		{
			// [#2072] AU KSK 2011.06.17 명세표와 화면 View에만 'NZ$' 표시 (뉴질랜드 Option인 경우)
#if (AU_VERSION)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
				strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nSurchargeAmt)));
			else
				strTemp.Format(L"%s%s", NZ_CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nSurchargeAmt)));
#elif (MX_VERSION)
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			// [#2147] MX KSK 2012.08.09 USD 방출모드의 International인 경우에는 qbB를 Surcharge Amount로 출력함 (고객 요청사항)
			if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
			{
				if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_INTERNATIONAL_TYPE)
					nSurchargeAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));
			}

			// [#2137] MX PCS 2012.07.06
			if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
			{
				if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_DOMESTIC_TYPE)
				{
					if (strTranType == TRANTYPE_DCC)
						strTemp.Format(L"%s %s", MakeMoneyCent(Int2Asc(nSurchargeAmt)), GetCurrencySymbol(0, 0, MX_DOMESTIC_TYPE));
					else
						strTemp.Format(L"%s%s", CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nSurchargeAmt)));
				}
				else	// INTL CARD
				{
					// [#2142] MX KMK 2012.08.10 
					if (strTranType == TRANTYPE_DCC)
						strTemp.Format(L"%s %s", MakeMoneyCent(Int2Asc(nSurchargeAmt)), GetCurrencySymbol(0, 0, MX_INTERNATIONAL_TYPE));
					else
						strTemp.Format(L"%s%s", MULTI_CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nSurchargeAmt)));	// [#2147] MX KSK 2012.08.18
				}
			}
			else
			{
				if (strTranType == TRANTYPE_DCC)
					strTemp.Format(L"%s %s", MakeMoneyCent(Int2Asc(nSurchargeAmt)), GetCurrencySymbol(0, 0, MX_DOMESTIC_TYPE));
				else
					strTemp.Format(L"%s%s", CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nSurchargeAmt)));
			}
			//end of [#2137]
			// end of [#2147]
			/////////////////////////////////////////////////////////////////////////////////////////////////////
#else
			if (nCustomerOption != DCC_CUSTOMOPTION_DEFAULT)
			{
				strTemp.Format(L"%s %s", MakeMoneyCent(Int2Asc(nSurchargeAmt)), m_pDevCmn->m_strCurrencyID);
			}
			else
			{
				strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nSurchargeAmt)));
			}

#endif	// end of [#2072]

#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_821, nCustomerOption, isVisaDcc), strTemp);
#else
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_021, nCustomerOption, isVisaDcc), strTemp);
#endif

#if (MX_VERSION)
			// [#2137] IVA TAX Print
			if (nLenIVATAX != 0)
			{
				if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
				{
					if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_DOMESTIC_TYPE)
					{
						if (strTranType == TRANTYPE_DCC)
							strTemp.Format(L"%s %s", GetCurrencySymbol(0, 0, MX_DOMESTIC_TYPE), MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX));
						else
							strTemp.Format(L"%s%s", CURRENCY_SYMBOL, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX));

						DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_023, nCustomerOption, isVisaDcc), strTemp);
					}
					else
					{
						// International은 미인자
					}
				}
				else
				{
					if (strTranType == TRANTYPE_DCC)
					{
						strTemp.Format(L"%s %s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX), GetCurrencySymbol(0, 0, MX_DOMESTIC_TYPE));
					}
					else
					{
						strTemp.Format(L"%s%s", CURRENCY_SYMBOL, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX));
					}

					DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_023, nCustomerOption, isVisaDcc), strTemp);
				}
			}
			// end of [#2137]

			// [#2137] Exchange Fee
			if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
			{
				// 출금이면서 International인 경우에만 인자
				if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_INTERNATIONAL_TYPE)
				{
					if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE).GetLength())
					{
						if (strTranType == TRANTYPE_DCC)
							strTemp.Format(L"%s %s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE), GetCurrencySymbol(0, 0, MX_INTERNATIONAL_TYPE));
						else
							strTemp.Format(L"%s%s", MULTI_CURRENCY_SYMBOL, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE));

						DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_030, nCustomerOption, isVisaDcc), strTemp)
					}
				}
				else
				{
					// Domestic은 미인자
				}
			}
			else
			{
				// Peso Dispense시에는 미인자
			}
			// end of [#2137]
			// end of [#2115]
#endif
		}
	}
#if( AU_VERSION || MX_VERSION)		// [#2115] MX KSK 2012.02.04
	else if (strTranType == TRANTYPE_INQUIRY)
	{
#if (MX_VERSION)
		// [#2115] MX KSK 2012.02.04 조회 거래인 경우 통신장애가 아닌 경우에만 인자
		if (m_pDevCmn->fstrAPL_GetErrorCode().Left(1) != "D")
		{
			// [#2147] MX KSK 2012.08.09 USD 방출모드의 International인 경우에는 qbB를 Surcharge Amount로 출력함 (고객 요청사항)
			if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
			{
				if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_INTERNATIONAL_TYPE)
					nSurchargeAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));
			}
			// end of [#2147]

			if (nSurchargeAmt != 0)
			{
				if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
				{
					if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_DOMESTIC_TYPE)
						strTemp.Format(L"%s%s", CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nSurchargeAmt)));
					else
						strTemp.Format(L"%s%s", MULTI_CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nSurchargeAmt)));
				}
				else
				{
					strTemp.Format(L"%s%s", CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nSurchargeAmt)));
				}
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_021), strTemp);
			}

			if (nLenIVATAX != 0)
			{
				if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
				{
					if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_DOMESTIC_TYPE)
						strTemp.Format(_T("%s%s"), CURRENCY_SYMBOL, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX));
					else
						strTemp.Format(_T("%s%s"), MULTI_CURRENCY_SYMBOL, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX));
				}
				else
				{
					strTemp.Format(_T("%s%s"), CURRENCY_SYMBOL, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX));
				}

				DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_023), strTemp);
			}
			// end of [#2137]
		}
		// end of [#2115]
#else
		if (nSurchargeAmt != 0)
		{
			// [#2072] AU KSK 2011.06.17 명세표와 화면 View에만 'NZ$' 표시 (뉴질랜드 Option인 경우)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
				strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nSurchargeAmt)));
			else
				strTemp.Format(L"%s%s", NZ_CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nSurchargeAmt)));
			// end of [#2072]

			DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_021), strTemp);
		}
#endif
	}
	// [#2150] US Justin 2012.10.09 Add Printing "Surcharge Amount for Balance inquiry and Pin Change
#elif (US_VERSION)
	else if ((nSurchargeAmt > 0) && ((strTranType == TRANTYPE_INQUIRY) || (strTranType == TRANTYPE_PINCHANGE)))
	{
		// [#2298] 2014.10.23 Bug Fix (Printing ATM fee for Denied Balance Inquiry)
		/*
		if (strTranType == TRANTYPE_PINCHANGE)
			AddPrintData(strPrintData, L"----------------------------------------");
		strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nSurchargeAmt)));
		if (m_pDevCmn->fnSCR_GetCurrentLangMode() == FRN_MODE)
			AddPrintData(strPrintData, L"%-18.18s= %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_021), strTemp);
		else
			AddPrintData(strPrintData, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_021), strTemp);
		*/
		if (m_pDevCmn->fstrAPL_GetErrorCode().IsEmpty())
		{
			if (strTranType == TRANTYPE_PINCHANGE)
				AddPrintData(strPrintData, L"----------------------------------------");
			strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nSurchargeAmt)));
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_821), strTemp);
#else
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_021), strTemp);
#endif
		}
		// End of [#2298]
	}
#endif
	// End of [#2150]
	//------------------------------------------------------------------------------------------------

	// TOTAL AMOUNT
#if !(AU_A_VERSION)	// [#2293] AU KSK 2014.09.17 removed total amount filed (requested by Customer)	// [#2362] DC Payments 요청사항
	//------------------------------------------------------------------------------------------------
	// [#2150] US Justin 2012.10.09 Add DCC
	// [#2446] US Justin Add Paypal
	if (strTranType == TRANTYPE_WITHDRAWAL || strTranType == TRANTYPE_DCC || strTranType == TRANTYPE_PAYPAL)
	{
#if (MX_VERSION)
		if (nDispensedAmt != 0)	// [#2115] MX KSK 2012.02.04 surcharge가 없더라도 IVA TAX값 더해주어야 함 [#2147] MX KSK 2012.08.18
#else
		if ((nDispensedAmt != 0) && (nSurchargeAmt != 0))
#endif
		{
			int nTemp = 0;

			// [#2137] MX KSK 2012.07.25
			nTemp = nDispensedAmt + nSurchargeAmt;

			if (nCustomerOption != DCC_CUSTOMOPTION_DEFAULT)
			{
				strTemp.Format(L"%s %s", MakeMoneyCent(Int2Asc(nTemp)), m_pDevCmn->m_strCurrencyID);
			}
			else
			{
				strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nTemp)));
			}
			//			#endif				// end of [#2072]		end of [#2293]

#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04 Multi Currency인 경우에만 Total Amount 인자
// [#2147] MX KSK 2012.08.18 사양 변경으로 인해 Partial인 경우 % Base로 Sum하도록 수정
			if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
			{
				// [#2142] MX KMK 2012.08.10 [USD MODE] Total Amount 값 변경
				int nTotalAmount = 0;
				if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_DOMESTIC_TYPE)	// DOMESTIC CARD
				{
					// Domestic인 경우 환율로 곱해서 인자함
					// Dispensed Amount* Exchange Rate + Amount2(actual surcharge) + qbC(TAX)
					char	chTemp[1024] = { 0, };
					WideToMulti(chTemp, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE), sizeof(chTemp));
					float fDispenseAmt = (float)(nDispensedAmt * 0.01);
					float fExchangeRate = (float)atof(chTemp);
					float fTax = (float)(nIVATAXSurchargeAmt * 0.01);
					float fSurchargeAmt = (float)(nSurchargeAmt * 0.01);
					float fTotalAmt = (fDispenseAmt * fExchangeRate) + fSurchargeAmt + fTax;
					CString strTemp2;
					strTemp2.Format(L"%.2f", fTotalAmt);

					if (strTranType == TRANTYPE_DCC)
						strTemp.Format(L"%s %s", strTemp2, GetCurrencySymbol(0, 0, MX_DOMESTIC_TYPE));
					else
						strTemp.Format(L"%s%s", CURRENCY_SYMBOL, strTemp2);
				}
				else	// INTERNATIONAL CARD
				{	// Dispensed Amount + qbF(Total Fee)
					// qbF대신 qbB(Surcharge Amount + TAX) + qbE(International Transfer Fee) 사용

					// [#2147] MX KSK 2012.08.18
					// Partial인 경우에는 % base로 Exchange Fee를 부과하여 명세표에 인자하며 Reversal시에도 실제 부과된 qbE값을 송신함
					// 이전 Flow에서 Partial인 경우 Exchange Fee값을 계산하였으므로 여기서는 그 값을 그대로 사용함
					// end of [#2147]

					int qbB = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));	// 이때의 qbB는 Surcharge Amount + TAX
					int qbE = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE));
					nTotalAmount = nDispensedAmt + qbB + qbE;

					if (strTranType == TRANTYPE_DCC)
						strTemp.Format(L"%s %s", MakeMoneyCent(Int2Asc(nTotalAmount)), GetCurrencySymbol(0, 0, MX_INTERNATIONAL_TYPE));
					else
						strTemp.Format(L"%s%s", MULTI_CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nTotalAmount)));
				}

				if (strTranType == TRANTYPE_DCC)
				{
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_058, nCustomerOption, isVisaDcc), strTemp);
				}
				else
				{
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_012), strTemp);
				}
				// end of [#2142]
			}
			else
			{
				// [#2147] MX KSK 2012.08.18
				// [#2142] MX KMK 2012.08.10 PESO 방출 모드에서 출금 거래시에 명세표에 Total Amount값 인자
				nTemp = nTemp + nIVATAXSurchargeAmt;		// Dispensed + Surcharge + TAX

				if (strTranType == TRANTYPE_DCC)
				{
					strTemp.Format(L"%s %s", MakeMoneyCent(Int2Asc(nTemp)), GetCurrencySymbol(0, 0, MX_DOMESTIC_TYPE));
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_058, nCustomerOption, isVisaDcc), strTemp);
				}
				else
				{
					strTemp.Format(L"%s%s", CURRENCY_SYMBOL, MakeMoneyCent(Int2Asc(nTemp)));
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_012), strTemp);
				}
				// end of [#2142]
				// end of [#2147]
			}
			// end of [#2137]
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
			#else
			if (nCustomerOption != DCC_CUSTOMOPTION_DEFAULT)
			{
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_058, nCustomerOption, isVisaDcc), strTemp);
			}
			else
			{
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_012), strTemp);
			}
#endif				// end of [#2115]
		}
	}
#endif	// end of [#2293]
	//------------------------------------------------------------------------------------------------

	// [#2150] US Justin 2012.10.02 Add DCC Receipt
	// [#2202] NH Justin 2013.06.20 Print Exchange Rate and converted amount for partial dispense case as well.
	if ((nDispensedAmt > 0) &&
		((m_sSTD1_DynamicFlowResp.nDCCTransaction == 1) ||
			((m_STD3_TDL_Data.m_bProceedDCC == TRUE) && (m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_OfferAcceptance == L"1"))))
	{
		CString sAllSpaces = _T("                                        ");

		// Exchange Rate
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
		strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_831));
#else
		strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_031));
#endif

		if (m_STD3_TDL_Data.m_bProceedDCC == TRUE)
		{
			// TDL message
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
			strTemp2.Format(L"1.00 %s=%s %s", m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferHomeCurrencyISO,
				m_STD3_TDL_Data.m_DCC_Lookup.m_strDisplayedExchangeRate, MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID));
#else
			strTemp2.Format(L"1 %s=%s %s", m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferHomeCurrencyISO,
				m_STD3_TDL_Data.m_DCC_Lookup.m_strDisplayedExchangeRate, MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID));
#endif

		}
		else
		{
			// DF message
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
			strTemp2.Format(L"1.00 %s=%s %s", m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Left(3),
					m_sSTD1_DynamicFlowResp.R2_DisplayedExchangeRate, m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Mid(6, 3));
#else
			strTemp2.Format(L"1 %s=%s %s", m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Left(3),
					m_sSTD1_DynamicFlowResp.R2_DisplayedExchangeRate, m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Mid(6, 3));
#endif

		}
		// End of [#2292]

		AddPrintData(strPrintData, L"%s %s%s", strTemp, sAllSpaces.Left(40 - strTemp.GetLength() - strTemp2.GetLength() - 1), strTemp2);

		// [#GSCJSD-5967] US ryan.payton 2023.02.01 Fixed Cardtronics being out of Visa DCC compliance
		//if (isVisaDcc)
		//{
		CString markup;

		if (m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCDisclaimerScheme == L"1" && m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCMarkupRate.GetLength() > 0)
			markup = m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCMarkupRate;
		else if (m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRateMarkUp.GetLength() > 0)
			markup = m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRateMarkUp;
		else if (m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRateMarkUp.GetLength() > 0)
			markup = m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRateMarkUp;

		if (!markup.IsEmpty())
		{
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
			// Mark-up label 11162023 mark up printer
			//strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_052, nCustomerOption, isVisaDcc));

			// Mark-up value
			strTemp2.Format(L"%s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_852, nCustomerOption, isVisaDcc),
				                      m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_053, nCustomerOption, isVisaDcc));
			strTemp2.Replace(L"XXXXX", markup);
            strTemp = L""; 
			AddPrintData(strPrintData, L"%s %s%s", strTemp, sAllSpaces.Left(40 - strTemp.GetLength() - strTemp2.GetLength() - 1), strTemp2);
#else
			// Mark-up label 11162023 mark up printer
			strTemp.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_052, nCustomerOption, isVisaDcc));

			// Mark-up value
			strTemp2.Format(L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_053, nCustomerOption, isVisaDcc));
			strTemp2.Replace(L"XXXXX", markup);

			AddPrintData(strPrintData, L"%s %s%s", strTemp, sAllSpaces.Left(40 - strTemp.GetLength() - strTemp2.GetLength() - 1), strTemp2);
#endif

		}
		//}
		// End of [#GSCJSD-5967]

		// Home Currency Charge

		BOOL bPrintLocalAmount = TRUE;
		CString sTotalHome;
		if (m_STD3_TDL_Data.m_bProceedDCC == TRUE)
		{
			if (nRequestedAmt == nDispensedAmt)
			{
				strTemp2.Format(L"%s %s", m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_PresentedAmount, m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferHomeCurrencyISO);		// [#2348] US Justin 2015.06.02 fix decimal point 
			}
			else
			{
				char chTemp[1024] = { 0, };
				WideToMulti(chTemp, m_STD3_TDL_Data.m_DCC_Lookup.m_strDisplayedExchangeRate, sizeof(chTemp));
				double fERate = (float)atof(chTemp);
				int amount = nDispensedAmt + nSurchargeAmt;

#if (MX_VERSION)
				if (m_pDevCmn->m_strCurrencyID == CURRENCY_TYPE)
				{
					amount += nIVATAXSurchargeAmt;
				}
#endif

				sTotalHome.Format(L"%d", (int)(1.0 * amount / fERate));

				strTemp2.Format(L"%0.2f %s", (0.01 * Asc2Int(sTotalHome)), m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferHomeCurrencyISO);
			}
		}
		else
		{
			if ((nCustomerOption == DCC_CUSTOMOPTION_CARDTRONICS) && (nRequestedAmt != nDispensedAmt))
			{
				bPrintLocalAmount = FALSE;
			}

			// Calculate Total converted Amount
			if (nRequestedAmt == nDispensedAmt)
			{
				int nTotalHCValue = Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount) + Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge);
				sTotalHome.Format(L"%d", nTotalHCValue);
			}
			else			// Need to calculate dispensed total amount 
			{
				char chTemp[1024] = { 0, };
				WideToMulti(chTemp, m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate, sizeof(chTemp));
				double fERate = (float)atof(chTemp);
				int nBCDigit = Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Mid(4, 1));
				int nTCDigit = Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Right(1));
				sTotalHome.Format(L"%d", (int)(fERate * (nDispensedAmt + nSurchargeAmt) * pow(10.0, (nTCDigit - nBCDigit)) + 0.5));
			}

			int nTotalHCLength = Asc2Int(m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Right(1));
			if (nTotalHCLength <= 0)
			{
				strTemp2.Format(L"%s ", sTotalHome);
			}
			else
			{
				if (sTotalHome.GetLength() < (nTotalHCLength + 1))
				{
					CString strZeroTemp = _T("00000000000000000000");
					sTotalHome = strZeroTemp.Left(nTotalHCLength + 1 - sTotalHome.GetLength()) + sTotalHome;
				}
				strTemp2.Format(L"%s.%s ", sTotalHome.Left(sTotalHome.GetLength() - nTotalHCLength), sTotalHome.Right(nTotalHCLength));
			}

			strTemp2 += m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency.Mid(6, 3);
		}

		if (bPrintLocalAmount == TRUE)
		{
#if (APP_CUSTOM_CASHDEPOT)||(APP_CUSTOM_DYNAMICDCC)  // RWC6-DYNAMICDCC
			strTemp.Format(L"%s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_832, nCustomerOption, isVisaDcc));
#else
			strTemp.Format(L"%s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_032, nCustomerOption, isVisaDcc));
#endif
			if (strTemp.GetLength() + strTemp2.GetLength() > 40)
			{
				// "TOTAL TRANSACTION AMOUNT =     "
				// "                         $20.00"
				AddPrintData(strPrintData, L"%s\x1c%40.40s", strTemp, strTemp2);
			}
			else 
			{
				// "TOTAL AMOUNT =           $20.00"
				AddPrintData(strPrintData, L"%s%s%s", strTemp, sAllSpaces.Left(40 - strTemp.GetLength() - strTemp2.GetLength()), strTemp2);
			}
		}
	}

	// LEDGER BALANCE / AVAILABLE BALANCE 처리 (17 ~ 18)
	// [#2292] US Justin 2014.10.10 DUAL BALANCE : PRINT BALANCES
	if (m_DualBalance.m_bDualBalance == TRUE)
	{
		// BALANCE FOR CHECKING ACCOUNT
		AddPrintData(strPrintData, L" ");

		DUALPRINT(strPrintData, nDualReceiptLang, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_007));

		if (m_DualBalance.m_bCheckingBalance)
		{
			m_DualBalance.m_strCheckingLedgerBalance.Replace(_T(" "), _T(""));
			m_DualBalance.m_strCheckingAvailBalance.Replace(_T(" "), _T(""));
			// Ledger Balance
			if ((!m_DualBalance.m_strCheckingLedgerBalance.IsEmpty()) && (Asc2Int(m_DualBalance.m_strCheckingLedgerBalance) != 0))
			{
				strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(m_DualBalance.m_strCheckingLedgerBalance));
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_014), strTemp);
			}
			// Available Balance
			if ((!m_DualBalance.m_strCheckingAvailBalance.IsEmpty()) && (Asc2Int(m_DualBalance.m_strCheckingAvailBalance) != 0))
			{
				strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(m_DualBalance.m_strCheckingAvailBalance));
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_015), strTemp);
			}
		}
		else
		{
			// Balance unavailable
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_013));

			// Error code if provided
			if (!m_DualBalance.m_strCheckingError.IsEmpty())
			{
				DUALPRINT(strPrintData, nDualReceiptLang, L"%s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_018), m_DualBalance.m_strCheckingError);
			}
		}

		// BALANCE FOR SAVINGS ACCOUNT
		AddPrintData(strPrintData, L" ");

		DUALPRINT(strPrintData, nDualReceiptLang, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_DATA_008));

		if (m_DualBalance.m_bSavingBalance)
		{
			m_DualBalance.m_strSavingLedgerBalance.Replace(_T(" "), _T(""));
			m_DualBalance.m_strSavingAvailBalance.Replace(_T(" "), _T(""));
			// Ledger Balance
			if ((!m_DualBalance.m_strSavingLedgerBalance.IsEmpty()) && (Asc2Int(m_DualBalance.m_strSavingLedgerBalance) != 0))
			{
				strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(m_DualBalance.m_strSavingLedgerBalance));
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_014), strTemp);
			}
			// Available Balance
			if ((!m_DualBalance.m_strSavingAvailBalance.IsEmpty()) && (Asc2Int(m_DualBalance.m_strSavingAvailBalance) != 0))
			{
				strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(m_DualBalance.m_strSavingAvailBalance));
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_015), strTemp);
			}
		}
		else
		{
			// Balance unavailable
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_013));
			// Error code if provided
			if (!m_DualBalance.m_strSavingError.IsEmpty())
			{
				DUALPRINT(strPrintData, nDualReceiptLang, L"%s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_018), m_DualBalance.m_strSavingError);
			}
		}
	}
	// End of [#2292]
	// [#2350] US Justin 2015.06.19 POP Money => Not printing balance.... 
	// [#2405] US Justin 2016.03.21 HalCash => Not Printing Balance
	// [#2445] US Justin 2016.09.28 Just.Cash => Not Printing Balance (Print Balance for Bit Coin)
	else if ((nWithdrawalType != WITHDRAWAL_POPMONEY) &&
		(nWithdrawalType != WITHDRAWAL_PIN4) && (nWithdrawalType != WITHDRAWAL_JUSTCASH))
	{
		// missfeed 장애 or 통신장애시 balance는 인자하지 않는다.
		//------------------------------------------------------------------------------------------------
		//if (strTranType == TRANTYPE_WITHDRAWAL && nRequestedAmt != nDispensedAmt)		;
		// [#2150] US Justin 2012.10.09 Add DCC
		// [#2446] US Justin Add Paypal
		// [#2496] US Justin Add Bitcoin
		BOOL bPrintBalance = TRUE;

		if ((strTranType == TRANTYPE_WITHDRAWAL || strTranType == TRANTYPE_DCC || strTranType == TRANTYPE_PAYPAL || strTranType == TRANTYPE_NONCASH_WITHDRAW) &&
			(nRequestedAmt != nDispensedAmt))
			bPrintBalance = FALSE;
		else if (m_pDevCmn->fstrAPL_GetErrorCode().Left(1) == "D")
			bPrintBalance = FALSE;
		else if (strTranType == TRANTYPE_PINCHANGE)
			bPrintBalance = FALSE;									// [#2150] US Justin 2012.10.05 No balance for Pin Change

		if (bPrintBalance)
		{
			strLedgerBalance = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT);
			strAvailableBalance = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSAVAILBALANCE);

			strLedgerBalance.Replace(_T(" "), _T(""));
			strAvailableBalance.Replace(_T(" "), _T(""));

			if (strLedgerBalance.IsEmpty())
			{
				// ledger balance is space
				if (strAvailableBalance.IsEmpty())
				{
					// Balance unavailable
					BalProc = 0x80;
				}
				else if (Asc2Int(strAvailableBalance) == 0)
				{
					if (TranCode == TC_INQUIRY)	BalProc = 0x10;	// available balance is 0 (print)
					else						BalProc = 0x00;	// available balance is 0 (not print)
				}
				else
				{
					BalProc = 0x10;
				}
			}
			else if (Asc2Int(strLedgerBalance) == 0)
			{
				// ledger balance is 0
				if (strAvailableBalance.IsEmpty())
				{
					if (TranCode == TC_INQUIRY) BalProc = 0x01;
					else						BalProc = 0x00;
				}
				else if (Asc2Int(strAvailableBalance) == 0)
				{
					if (TranCode == TC_INQUIRY)	BalProc = 0x11;
					else						BalProc = 0x00;
				}
				else
				{
					if (TranCode == TC_INQUIRY)	BalProc = 0x11;
					else						BalProc = 0x10;
				}
			}
			else
			{
				// ledger balance is +/- value
				if (strAvailableBalance.IsEmpty())
				{
					BalProc = 0x01;
				}
				else if (Asc2Int(strAvailableBalance) == 0)
				{
					if (TranCode == TC_INQUIRY)	BalProc = 0x11;
					else						BalProc = 0x01;
				}
				else
				{
					BalProc = 0x11;
				}
			}

			if (BalProc & 0x80)
			{
				if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) != MSG_TRITON_TYPE)
				{
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04 
					// [#2137] MX KSK 2012.07.25
					// CARDTRONICS는 출금 거래 시 Balance 정보 인자 안함 (PROSA인 경우에만 인자함
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_013));
					// end of [#2137]
#else
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_013));
#endif				// end of [#2115]
				}
			}
			else
			{
				if (BalProc & 0x01)
				{
					// [#2072] AU KSK 2011.06.17 명세표와 화면 View에만 'NZ$' 표시 (뉴질랜드 Option인 경우)
#if (AU_VERSION)
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
						strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strLedgerBalance));
					else
						strTemp.Format(L"%s%s", NZ_CURRENCY_SYMBOL, MakeMoneyCent(strLedgerBalance));
#elif (MX_VERSION)
// [#2137] MX KSK 2012.07.25
					if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
					{
						// USD Dispense Mode
						if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_DOMESTIC_TYPE)
						{
							if (strTranType == TRANTYPE_DCC)
								strTemp.Format(L"%s %s", MakeMoneyCent(strLedgerBalance), GetCurrencySymbol(0, 0, MX_DOMESTIC_TYPE));
							else
								strTemp.Format(L"%s%s", CURRENCY_SYMBOL, MakeMoneyCent(strLedgerBalance));
						}
						else
						{
							if (strTranType == TRANTYPE_DCC)
								strTemp.Format(L"%s %s", MakeMoneyCent(strLedgerBalance), GetCurrencySymbol(0, 0, MX_INTERNATIONAL_TYPE));
							else
								strTemp.Format(L"%s%s", MULTI_CURRENCY_SYMBOL, MakeMoneyCent(strLedgerBalance));
						}
					}
					else
					{
						if (strTranType == TRANTYPE_DCC)
							strTemp.Format(L"%s %s", MakeMoneyCent(strLedgerBalance), GetCurrencySymbol(0, 0, MX_DOMESTIC_TYPE));
						else
							strTemp.Format(L"%s%s", CURRENCY_SYMBOL, MakeMoneyCent(strLedgerBalance));
					}
					// end of [#2137]
#else
					strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strLedgerBalance));
#endif	// end of [#2072]

#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
					// [#2137] MX KSK 2012.07.25 DP 특이사항 삭제
					// CARDTRONICS는 출금 거래 시 Balance 정보 인자 안함 (PROSA인 경우에만 인자함
	//				if ((strTranType == TRANTYPE_INQUIRY) || (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MX_DP_SELECTION) == MX_PROSA))
	//					AddPrintData(strPrintData, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_014), strTemp);
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_014), strTemp);
					// end of [#2137]
#else
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_014), strTemp);
#endif
				}

				if (BalProc & 0x10)
				{
					// [#2072] AU KSK 2011.06.17 명세표와 화면 View에만 'NZ$' 표시 (뉴질랜드 Option인 경우)
#if (AU_VERSION)
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
						strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strAvailableBalance));
					else
						strTemp.Format(L"%s%s", NZ_CURRENCY_SYMBOL, MakeMoneyCent(strAvailableBalance));
#elif (MX_VERSION)
// [#2137] MX KSK 2012.07.25
					if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
					{
						// USD Dispense Mode
						if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_DOMESTIC_TYPE)
						{
							if (strTranType == TRANTYPE_DCC)
								strTemp.Format(L"%s %s", MakeMoneyCent(strAvailableBalance), GetCurrencySymbol(0, 0, MX_DOMESTIC_TYPE));
							else
								strTemp.Format(L"%s%s", CURRENCY_SYMBOL, MakeMoneyCent(strAvailableBalance));
						}
						else
						{
							if (strTranType == TRANTYPE_DCC)
								strTemp.Format(L"%s %s", MakeMoneyCent(strAvailableBalance), GetCurrencySymbol(0, 0, MX_INTERNATIONAL_TYPE));
							else
								strTemp.Format(L"%s%s", MULTI_CURRENCY_SYMBOL, MakeMoneyCent(strAvailableBalance));
						}
					}
					else
					{
						if (strTranType == TRANTYPE_DCC)
							strTemp.Format(L"%s %s", MakeMoneyCent(strAvailableBalance), GetCurrencySymbol(0, 0, MX_DOMESTIC_TYPE));
						else
							strTemp.Format(L"%s%s", CURRENCY_SYMBOL, MakeMoneyCent(strAvailableBalance));
					}
					// end of [#2137]
#else
					strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strAvailableBalance));
#endif	// end of [#2072]

#if (MX_VERSION)	// [#2115] MX KSK 2012.02.04
					// [#2137] MX KSK 2012.07.25 DP 특이사항 제거
					// CARDTRONICS는 출금 거래 시 Balance 정보 인자 안함 (PROSA인 경우에만 인자함)
	//				if ((strTranType == TRANTYPE_INQUIRY) || (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MX_DP_SELECTION) == MX_PROSA))
	//					AddPrintData(strPrintData, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_015), strTemp);
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_015), strTemp);
					// end of [#2137]
#else
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_015), strTemp);
#endif				// end of [#2115]
				}
			}
		}
	}
	//------------------------------------------------------------------------------------------------

	// 19. Surcharge Payed To   -- SURCHARGE PAID TO
	//------------------------------------------------------------------------------------------------
#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
	if (nSurchargeAmt != 0)
	{
		NHDEBUG(DBG_INFO, (L"[PRINT]-[Surcharge Amount and Message(%d)]\n", nSurchargeAmt));
		DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_016));

		//		AddPrintData(strPrintData, L":%-39.39s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER));
		AddPrintData(strPrintData, L" : %-37.37s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER));	// [#2326] AU KSK 2015.08.09 DC Payments 요청에 의해 NH1800CE와 동일하게 Format 수정
		CString strMsg[3];

		for (int i = 0; i < 3; i++)
		{
			strMsg[i] = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT1 + i);
			strMsg[i].TrimRight();
		}

		if (strMsg[0].GetLength() > 0 || strMsg[1].GetLength() > 0 || strMsg[2].GetLength() > 0)
		{
			AddPrintData(strPrintData, L"%-40.40s", MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT1));

			AddPrintData(strPrintData, L"%-40.40s", MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT2));

			AddPrintData(strPrintData, L"%-40.40s", MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT3));
		}
		// end of KSK 2011.02.10
	}
#elif (MX_VERSION)
	// [#2115] MX KSK 2012.02.04
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER);

	if (strTranType == TRANTYPE_WITHDRAWAL)
	{
		if ((nDispensedAmt != 0) && (nSurchargeAmt != 0))	// [#2147] MX KSK 2012.08.18
		{
#if (MX_VERSION)	// [#2147] MX KSK 2012.08.18
			if (nDispScreen == FALSE)	// 화면 Line 문제로 인해 화면에는 표시 안함
			{
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_016));
				
				AddPrintData(strPrintData, L"%-40.40s", strTemp);
			}
#else
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_016));
			
			AddPrintData(strPrintData, L"%-40.40s", strTemp);
#endif				// end of [#2147]
		}
	}
	else if (strTranType == TRANTYPE_INQUIRY)
	{
		// [#2142] MX KMK 2012.08.16 5.0소스 반영
// 		if ((nSurchargeAmt != 0) && (m_pDevCmn->fstrAPL_GetErrorCode().Left(1) != "D"))
		if (m_pDevCmn->fstrAPL_GetErrorCode().Left(1) != "D")
		{
			// USD 방출모드의 International인 경우에는 qbB를 Surcharge Amount로 출력함 (고객 요청사항)
			if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
			{
				if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_INTERNATIONAL_TYPE)
					nSurchargeAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));
			}

			if (nSurchargeAmt != 0)
			{
				if (nDispScreen == FALSE)	// [#2147] MX KSK 2012.08.18
				{
					DUALPRINT(strPrintData, nDualReceiptLang, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_016));
					AddPrintData(strPrintData, L"%-40.40s", strTemp);
				}
			}
		}
		// end of [#2142]
	}
	// end of [#2115]
#else
	// [#2150] US Justin 2012.10.09 Add DCC
	// [#2446] US Justin Add Paypal
	if (strTranType == TRANTYPE_WITHDRAWAL || strTranType == TRANTYPE_INQUIRY || strTranType == TRANTYPE_DCC || strTranType == TRANTYPE_PAYPAL)
	{
		if ((nDispensedAmt != 0) && (nSurchargeAmt != 0))
		{
			DUALPRINT(strPrintData, nDualReceiptLang, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_016));

			AddPrintData(strPrintData, L"%-40.40s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER));
		}
	}
#endif
	//------------------------------------------------------------------------------------------------
	}

// [#2115] MX KSK 2012.02.04
void CTranCmn::P_MX_NOR_BankNameandFeePrintProc(CString& strPrintData, int nDualReceiptLang)
{
	CString	strTranType, strTemp;
	int		nLenLoyaltyFee, nLenLineUsageFee, nLenBankName, nDispensedAmount, nRequestedAmount;	// [#2137] MX KSK 2012.07.27

	nRequestedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
	nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));
	nLenLoyaltyFee = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE).GetLength();
	nLenLineUsageFee = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE).GetLength();
	nLenBankName = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_BANK_NAME).GetLength();
	strTranType.Format(L"%2.2s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE));

	// [#2147] MX KSK 2012.08.18 USD 방출시 International Card인 경우 BankName 및 Fee Print하지 않도록 수정
	if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
	{
		if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_INTERNATIONAL_TYPE)
			return;
	}
	// end of [#2147]

	// BankName and Fee Print 만족 조건
	// 1. Bank Name Fee Option이 Enable이면서 통신장애가 아닌 경우
	//  1) 출금거래에서 방출 금액이 있는 경우
	//  2) 조회 거래인 경우
	//  2012.08.08 +) PESO 방출 or (USD 방출 & Domestic Card 거래)인 경우
	if (((m_pDevCmn->fstrAPL_GetErrorCode().Left(1) != "D") && (ENABLE == MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_BANK_NAME_FEE_PRINT))))
	{
		if ((strTranType == TRANTYPE_WITHDRAWAL && nDispensedAmount != 0) || (strTranType == TRANTYPE_INQUIRY))	// [#2147] MX KSK 2012.08.18
//		if ((strTranType == TRANTYPE_WITHDRAWAL && nRequestedAmount == nDispensedAmount) || (strTranType == TRANTYPE_INQUIRY))
		{
			// 1. BANK NAME 인자
			if (nLenBankName != 0)
			{
				strTemp = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_BANK_NAME);
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_024), strTemp);
			}

			// [#2137] MX KMK 2012.07.19   LOYALTY FEE, LINE FEE 출력 부분
			// 2. LOYALTY FEE 인자		
			if (nLenLoyaltyFee != 0)
			{
				strTemp.Format(L"%s%s", CURRENCY_SYMBOL, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE));
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_025), strTemp);
			}

			// 3. LINE FEE 인자
			if (nLenLineUsageFee != 0)
			{
				strTemp.Format(L"%s%s", CURRENCY_SYMBOL, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));
				DUALPRINT(strPrintData, nDualReceiptLang, L"%-20.20s = %17.17s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_026), strTemp);
			}
		}
	}

	// [#2147] MX KSK 2012.08.18 외부에서 Print하도록 이동
// 	// 2. ACCOUNT NUMBER는 무조건 PRINT하도록 수정
// 	//------------------------------------------------------------------------------------------------
// 	if (!MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_RETURNED_ACCNT_NO).IsEmpty())
// 	{
// 		strTemp = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_RETURNED_ACCNT_NO);
// 		AddPrintData(strPrintData, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_027), strTemp);
// 	}
// 	//------------------------------------------------------------------------------------------------
// 	// end of [#2142]
// 
// 
// 	// 3. EMV TAG PRINT (ARQC, ARPC)
// 	if (m_pDevCmn->fnMCU_IsEmvTransaction())
// 	{
// 		// 1. ARQC Value
// 		strTemp = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AC_REQ_9F26_VALUE);
// 
// 		AddPrintData(strPrintData, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_028), strTemp);
// 
// 		// 2. ARPC Value - Host에서 내려온 91값 중에 앞에 8BYTE만 Print (고객 요청사항)
// 		int nLen = 0;
// 		unsigned char szTemp[1024] = { 0, };
// 
// 		strTemp.Empty();
// 		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_IsuAuthData, &nLen, szTemp) == EMV_RSLT_OK)
// 		{
// 			strTemp = MakeUnPack(szTemp, nLen);
// 			strTemp.Format(L"%s", strTemp.Left(16));
// 		}
// 
// 		AddPrintData(strPrintData, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_029), strTemp);
// 	}
	// end of [#2147]
}
// end of [#2115]

BOOL CTranCmn::LIB_AllSetupPrint()
{
	CString		strTemp, strTemp2;
	CString		strPrintData;
	CString		strDateTime;

	SYSTEMTIME	localTime;

	::GetLocalTime(&localTime);
	strDateTime.Format(L"%s %2.2d:%2.2d:%2.2d", GetCmnLocalDate(localTime.wMonth, localTime.wDay, localTime.wYear),
		localTime.wHour, localTime.wMinute, localTime.wSecond);

	// HEADER
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"%40.40s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PRT_ALLSETUP));

		AddPrintData(strPrintData, L"========================================");
		AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TODAY), strDateTime);
		AddPrintData(strPrintData, L"----------------------------------------");
		AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MACHINEKIND),
			MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND));

		LIB_GetSetupHeaderPrintData(strPrintData);
	}
	//------------------------------------------------------------------------------------------------
	// 1. VERSION INFORMATION
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L" ");
		AddPrintData(strPrintData, L"........................................");
		AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_1_VER_INFO));
		AddPrintData(strPrintData, L" ");

		LIB_GetVersionInformationPrintData(strPrintData);
	}
	//------------------------------------------------------------------------------------------------
	// 2. SYSTEM SETUP
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L" ");
		AddPrintData(strPrintData, L"........................................");
		AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_2_SYSTEM_SETUP));
		AddPrintData(strPrintData, L" ");

		LIB_GetSystemSetupPrintData(strPrintData);
	}

	//------------------------------------------------------------------------------------------------
	// 3. CUSTOMER SETUP
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L" ");
		AddPrintData(strPrintData, L"........................................");
		AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_3_CUST_SETUP));
		AddPrintData(strPrintData, L" ");

		LIB_GetCustomerSetupPrintData(strPrintData);
	}

	//------------------------------------------------------------------------------------------------
	// 4. TRANSACTION SETUP
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L" ");
		AddPrintData(strPrintData, L"........................................");
		AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_4_TRANS_SETUP));
		AddPrintData(strPrintData, L" ");

		LIB_GetTransactionSetupPrintData(strPrintData);
	}

	//------------------------------------------------------------------------------------------------
	// 5. HOST SETUP
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L" ");
		AddPrintData(strPrintData, L"........................................");
		AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_5_HOST_SETUP));
		AddPrintData(strPrintData, L" ");

		LIB_GetHostSetupPrintData(strPrintData);
	}
	//------------------------------------------------------------------------------------------------

	return m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT);
}

// [#2326] US Kook 2015.04.07
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: LIB_GetVersionInformationPrintData()
RETURN TYPE  :
PARAMETER    :
DESCRIPTION  :
-------------------------------------------------------------------*/
void CTranCmn::LIB_GetVersionInformationPrintData(CString& strPrintData)
{
	CStringArray strTempArray;

	SplitString(m_pDevCmn->GetSWVersionInfo(), "||", strTempArray);
	// 1234567890123456789012345678901234567890
	// --AP   VERSION----------VDM  VERSION----
	// AP  : V06.01.15    CDU : V06.06.07.00
	// OS  : V06.01.17    SPR : V06.04.13.00
	// RMS : V01.03.25    MCU : V06.00.04.00
	// B-L : V06.22.15    KMR : V06.00.00.16
	// MWI : V06.03.00    RFID: V06.00.04.00
	// EMV KERNEL : V5.5
	// 
	// 1234567890123456789012345678901234567890
	AddPrintData(strPrintData, L"--%s----------%s----",
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_003),
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_009));
	AddPrintData(strPrintData, L"%-3.3s : %-12.12s %-3.3s : %-14.14s",
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_AP), strTempArray[0],
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_010), strTempArray[5]);
	AddPrintData(strPrintData, L"%-3.3s : %-12.12s %-3.3s : %-14.14s",
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_OS), strTempArray[1],
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_011), strTempArray[6]);
	AddPrintData(strPrintData, L"%-3.3s : %-12.12s %-3.3s : %-14.14s",
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RMS), strTempArray[2],
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_012), strTempArray[7]);
	// (optional) KMR VDM
	if (strTempArray.GetSize() > 22)
	{
		AddPrintData(strPrintData, L"%-3.3s : %-12.12s %-3.3s : %-14.14s",
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B_L), strTempArray[3],
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_039), strTempArray[19]);
		// (optional) RFID VDM
		if (GetSystemMetrics(SM_CXSCREEN) == 1024)
		{
			AddPrintData(strPrintData, L"%-3.3s : %-12.12s %-4.4s: %-14.14s",
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MWI), strTempArray[4],
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_040), (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE) ? strTempArray[22] : L"N/A");
		}
		else
		{
			AddPrintData(strPrintData, L"%-3.3s : %-12.12s",
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MWI), strTempArray[4]);
		}
	}
	else
	{
		AddPrintData(strPrintData, L"%-3.3s : %-12.12s",
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B_L), strTempArray[3]);
		AddPrintData(strPrintData, L"%-3.3s : %-12.12s",
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MWI), strTempArray[4]);
	}

	// [#GLDV-3005] US Kook 2022.04.07 support side car
	// (optional) SIDECAR VDM
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE && strTempArray.GetSize() > 29)
	{
		AddPrintData(strPrintData, L"%-3.3s : %-13.13s %-4.4s: %-14.14s",
			L"EMV", (m_pDevCmn->fnMCU_IsEmvAvailable() == TRUE) ? strTempArray[18] : L"N/A", L"BCR", strTempArray[25]);

		AddPrintData(strPrintData, L"                    %-4.4s: %-14.14s",
			L"BNA", strTempArray[28]);
	}
	// end of [#GLDV-3005]
	else
	{
		AddPrintData(strPrintData, L"%s : %-9.9s",
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_038), (m_pDevCmn->fnMCU_IsEmvAvailable() == TRUE) ? strTempArray[18] : L"N/A");
	}

	AddPrintData(strPrintData, L" ");

	// 1234567890123456789012345678901234567890
	// --SP   VERSION----------EP   VERSION----
	// CDU : V06.06.14    CDU : CO3H1CV098017BR 
	// SPR : V06.04.18    SPR : SPR12
	// MCU : V06.00.12    MCU : 2420-03A
	// PIN : V06.04.07    PIN : V08.21.01
	// SIU : V06.03.01    SIU : N/A
	// RFID: V06.00.01    RFID: ...
	// ANTI SKIMMING EP : N/A
	// ========================================
	// 1234567890123456789012345678901234567890
	AddPrintData(strPrintData, L"--%s----------%s----",
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_013),
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_016));
	AddPrintData(strPrintData, L"%-3.3s : %-12.12s %-3.3s : %-14.14s",
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_010), strTempArray[8],
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_010), strTempArray[13]);
	AddPrintData(strPrintData, L"%-3.3s : %-12.12s %-3.3s : %-14.14s",
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_011), strTempArray[9],
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_011), strTempArray[14]);
	AddPrintData(strPrintData, L"%-3.3s : %-12.12s %-3.3s : %-14.14s",
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_012), strTempArray[10],
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_012), strTempArray[15]);
	AddPrintData(strPrintData, L"%-3.3s : %-12.12s %-3.3s : %-14.14s",
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_014), strTempArray[11],
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_014), strTempArray[16]);
	AddPrintData(strPrintData, L"%-3.3s : %-12.12s %-3.3s : %-14.14s",
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_015), strTempArray[12],
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_015), strTempArray[17]);
	
	// (optional) RFID SP/EP
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE && strTempArray.GetSize() > 22)
	{
		AddPrintData(strPrintData,	L"%-4.4s: %-13.13s %-4.4s: %-14.14s",
									m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_040), strTempArray[20],
									m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_040), strTempArray[21]);
	}

	// [#GLDV-3005] US Kook 2022.04.07 support side car
	// (optional) SIDECAR SP/EP
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE && strTempArray.GetSize() > 29)
	{
		AddPrintData(strPrintData,	L"%-4.4s: %-13.13s %-4.4s: %-14.14s",
									L"BCR", strTempArray[23],
									L"BCR", strTempArray[24]);

		AddPrintData(strPrintData,	L"%-4.4s: %-13.13s %-4.4s: %-14.14s",
									L"BNA", strTempArray[26],
									L"BNA", strTempArray[27]);
	}
	// end of [#GLDV-3005]

	AddPrintData(strPrintData, L"%-16.16s : %-10.10s",
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ANTISKIM_EP),
		RegGetStr(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"ESUVer").GetLength() > 0 ? RegGetStr(L"SOFTWARE\\ATM\\DevInfo\\IDC", L"ESUVer") : L"N/A");
}
// end of [#2326]


// [#2324] Receipt Optimizing with refactoring

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: LIB_GetSetupHeaderPrintData()
RETURN TYPE  :
PARAMETER    :
DESCRIPTION  : HEADER of Print All Setup
-------------------------------------------------------------------*/
void CTranCmn::LIB_GetSetupHeaderPrintData(CString& strPrintData)
{
	// COUNTRY
	CString strCountry = _T("");
#if (US_VERSION)			
	strCountry = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_USA);
#elif (AU_VERSION)
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
		strCountry = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_AUSTRALIA);
	else
		strCountry = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_NEWZEALAND);
#elif (CA_VERSION)
	strCountry = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CANADA);
#elif (MX_VERSION) 		// [#2115] MX KSK 2012.02.05	// [#2160] CA KMK 2012.11.21
	strCountry = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MEXICO);
#endif
	AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_COUNTRY), strCountry);

	// HOST PROCESSOR
	CString strProcessor = _T("");
	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
		strProcessor = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_STD1);
	else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_CSP200_TYPE)
		strProcessor = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_STD2);
	else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
		strProcessor = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_STD3);
	else
		strProcessor = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_EPS);
	AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_PROC), strProcessor);

	// NETWORK TYPE
	CString strNetType = _T("");
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP)
		strNetType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DIALUP);
	else
		strNetType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TCPIP);
	AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_NETWORKTYPE), strNetType);

	// TCP/IP TYPE DEFINED
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_TCPIP)
	{
		CString strTcpType = _T("");
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE) == VISA_FRAMED_TCPIP)
			strTcpType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_VISA_FRAMED);
		else if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE) == STANDARD_TCPIP)
			strTcpType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_STD);
		else
			strTcpType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ACK_CTRLD);

		AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TCPIP_TYPE_DEF),
			strTcpType);
	}

	// VISA FRAMED OPTION
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP
		|| ((MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_TCPIP)
			&& MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE) == VISA_FRAMED_TCPIP))
	{
		CString strVisaFramedOption = _T("");
		CString strEotCheckMode = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE);

		if (strEotCheckMode == EOT_OPTIONAL)
			strVisaFramedOption = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_EOT_OPTIONAL);
		else if (strEotCheckMode == NO_EOT_REQUIRED)
			strVisaFramedOption = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_NO_ENQ_EOT_REQ);
		else if (strEotCheckMode == NO_ENQ_REQUIRED)
			strVisaFramedOption = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_NO_ENQ_REQ);
		else if (strEotCheckMode == NO_ENQ_EOT_REQUIRED)
			strVisaFramedOption = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_NO_ENQ_EOT_REQ);
		else
			strVisaFramedOption = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_GENERAL);

		AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_VISA_FRAMED_OPT),
			strVisaFramedOption);
	}

	// SSL
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_TCPIP)
	{
		CString strSSLEnable = m_pDevCmn->fstrSCR_GetStringFromTextID(
			GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_SSLENABLE)));
		AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SSL), strSSLEnable);

		// SSL VERSION
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_SSLENABLE) == ENABLE)
		{
			// [#2310] NH KSK 2014.11.18 TLS Support
			// [#2320] US Justin 2015.01.14 Support NON-"Version Negotiation" Processor
			int nSSLVer = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSLVERSION);
			CString strSSLVer = _T("");

			if (nSSLVer == CONN_UPTO_SSL_V30)
				strSSLVer = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONN_UPTO_SSL_V30);
			else if (nSSLVer == CONN_UPTO_TLS_V10)
				strSSLVer = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONN_UPTO_TLS_V10);
			else if (nSSLVer == CONN_UPTO_TLS_V11)
				strSSLVer = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONN_UPTO_TLS_V11);
			else if (nSSLVer == CONN_UPTO_TLS_V12)
				strSSLVer = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONN_UPTO_TLS_V12);
			else if (nSSLVer == CONN_UPTO_TLS_V13)
				strSSLVer = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONN_UPTO_TLS_V13);
			else
				strSSLVer = m_pDevCmn->fstrSCR_GetStringFromTextID(T_NOT_DEFINED);

			AddPrintData(strPrintData, L"  %-17.17s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SSL_VER),
				strSSLVer);
		}

		// SSL CERTIFICATE
#if !(AU_VERSION)	// KSK 2015.01.14 AU는 Cert 미지원
		//[#2093] NH PCS 2011.10.25 "SSL Cert. Print"
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_SSLENABLE) == ENABLE)
		{
			CString strSSLCertEnable = _T("");
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSL_CERTIFY) == ENABLE)
				strSSLCertEnable = m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENABLE_RCPT);
			else
				strSSLCertEnable = m_pDevCmn->fstrSCR_GetStringFromTextID(T_DISABLE_RCPT);

			AddPrintData(strPrintData, L"  %-17.17s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SSL_CERT),
				strSSLCertEnable);
		}
		//end of [#2093] NH PCS 2011.10.25
#endif				// end of KSK 2015.01.14

	}
}


void CTranCmn::LIB_GetHaloLedPrintData(CString& strPrintData, int pRequiredLedState)
{
	int nHaloLedMode = 0;
	int nHaloLedColor = 0;
	CString strHaloLedMode = _T("");

	switch (pRequiredLedState)
	{
	case 1:		// IN SERVICE
		strHaloLedMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_INSERVICE);
		nHaloLedMode = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_INSERVICE);
		nHaloLedColor = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_INSERVICE);
		break;
	case 2:		// TRANSACTION
		strHaloLedMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TRANSACTION);
		nHaloLedMode = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_TRANSACTION);
		nHaloLedColor = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_TRANSACTION);
		break;
	case 3:		// DISPENSING
		strHaloLedMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DISPENSING);
		nHaloLedMode = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_DISPENSING);
		nHaloLedColor = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_DISPENSING);
		break;
	case 4:		// OUT OF SERVICE
		strHaloLedMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_OUTOFSERVICE);
		nHaloLedMode = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_OUTOFSERVICE);
		nHaloLedColor = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_OUTOFSERVICE);
		break;

	default:
		break;
	}

	switch (nHaloLedMode)
	{
	case OFF_HALOLED_MODE:
	case FADE_HALOLED_MODE:
		// MODE
		AddPrintData(strPrintData, L"%-16s : %s", strHaloLedMode,
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_HALO_LEDMODE_APTEXTID(nHaloLedMode)));
		break;

	case FLICKING_HALOLED_MODE:
	case ON_HALOLED_MODE:
		// MODE, COLOR
		AddPrintData(strPrintData, L"%-16s : %s, %s", strHaloLedMode,
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_HALO_LEDMODE_APTEXTID(nHaloLedMode)),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_HALO_COLOR_APTEXTID(nHaloLedColor)));
		break;

	default:
		break;
	}
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: LIB_GetSystemSetupPrintData()
RETURN TYPE  :
PARAMETER    :
DESCRIPTION  : '2. SYSTEM SETUP' of Print All Setup
-------------------------------------------------------------------*/
void CTranCmn::LIB_GetSystemSetupPrintData(CString& strPrintData)
{
	CString strTemp, strTemp2;

	// CURRENT BILL COUNT
	CString strCountCst[5] = { _T("N/A"), _T("N/A"), _T("N/A"), _T("N/A"), _T("N/A") };
	for (int nCstIdx = 0; nCstIdx <= m_pDevCmn->fnCDU_GetNumberOfCST(); nCstIdx++)
	{
		strCountCst[nCstIdx].Format(_T("%4d"), m_pDevCmn->fnCDU_GetNumberOfCash(nCstIdx));
	}

	AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringByTextID(_T("CURRENT BILL COUNT")));
	AddPrintData(strPrintData, L"  REJ.    CST1    CST2    CST3    CST4");
	AddPrintData(strPrintData, L"  %4s    %4s    %4s    %4s    %4s",
		strCountCst[0], strCountCst[1], strCountCst[2], strCountCst[3], strCountCst[4]);

	// [#RWC6-82] NH Kook 2021.12.27 Support CDU Auto Recovery (Jam Clear)
	AddPrintData(strPrintData, L"%-27.27s:%-12.12s", L"CDU AUTO RECOVERY", (MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_CDU_SUPPORTAR) ? L"SUPPORT" : L"NOT SUPPORT"));
	// end of [#RWC6-82]

	// SERIAL NUMBER
	AddPrintData(strPrintData, L"%-22.22s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SN),
		MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSERIALNO));

	// SPEAKER VOLUME
	AddPrintData(strPrintData, L"%-22.22s : %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SPK_VOL),
		MemGetInt(_MEM_FLD_INI_DEVINFO, _MEM_VAR_DEVINFO_VOLUMELEVEL));

	// MODEM INITIAL STRING
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP)
	{
		AddPrintData(strPrintData, L"%s :", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_MODEM_INITSTR));
		AddPrintData(strPrintData, L"%-40.40s", MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_MODEMINITSTR));
		AddPrintData(strPrintData, L"%s :", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RMS_MODEM_INITSTR));
		AddPrintData(strPrintData, L"%-40.40s", MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSMODEMINITIALSTRING));
	}

	// TERMINAL IP INFORMATION
	//   DHCP                 : DISABLE
	//   IP ADDRESS           : 123.123.123.123
	//   GATEWAY              : 123.123.123.123
	//   SUBNET MASK          : 123.123.123.123
	//   DNS SERVER           : 123.123.123.123
	//   MAC ADDRESS          : XX:XX:XX:XX:XX:XX
	//   MASQUE DE SOUS-RESEAU: xxx.xxx.xxx.xxx (FRN)
	else
	{
		AddPrintData(strPrintData, _T("%-40.40s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"TERMINAL IP INFORMATION"));

		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DHCP) == ENABLE)
		{
			AddPrintData(strPrintData, _T("  %-16.16s: %-15.15s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"DHCP"),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENABLE_RCPT));
			AtmGetDhcpIpAddress(strTemp);
			AddPrintData(strPrintData, _T("  %-16.16s: %-20.20s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"IP ADDRESS"), strTemp);
			AtmGetDhcpGateWay(strTemp);
			AddPrintData(strPrintData, _T("  %-16.16s: %-20.20s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"GATEWAY"), strTemp);
			AtmGetDhcpSubnetMask(strTemp);
			AddPrintData(strPrintData, _T("  %-16.16s: %-20.20s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"SUBNET MASK"), strTemp);
			// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
			//AtmGetDhcpDNS(strTemp);
			//AddPrintData(strPrintData, _T("  %-21.21s: %-15.15s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"DNS SERVER"), strTemp);
			AtmGetDhcpDNS(strTemp, strTemp2);
			AddPrintData(strPrintData, _T("  %-16.16s: %-20.20s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"DNS SERVER"), strTemp);
			// [#RWC6-54] US William 2019.09.13 Add MAC Address to PAS
			AddPrintData(strPrintData, _T("  %-16.16s: %-20.20s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"MAC ADDRESS"), m_pDevCmn->m_pNetWork->GetMACAddress());
			//AddPrintData(strPrintData, _T("  %-21.21s: %-15.15s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"DNS SERVER (2ND)"), strTemp2);
			// end of [#RWC6-10]
		}
		else
		{
			AddPrintData(strPrintData, _T("  %-16.16s: %-20.20s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"DHCP"),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_DISABLE_RCPT));
			AddPrintData(strPrintData, _T("  %-16.16s: %-20.20s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"IP ADDRESS"),
				MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_IPADDR));
			AddPrintData(strPrintData, _T("  %-16.16s: %-20.20s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"GATEWAY"),
				MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_GATEWAY));
			AddPrintData(strPrintData, _T("  %-16.16s: %-20.20s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"SUBNET MASK"),
				MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_SUBNET));
			AddPrintData(strPrintData, _T("  %-16.16s: %-20.20s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"DNS SERVER"),
				MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DNS));
			// [#RWC6-54] US William 2019.09.13 Add MAC Address to PAS
			AddPrintData(strPrintData, _T("  %-16.16s: %-20.20s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"MAC ADDRESS"), m_pDevCmn->m_pNetWork->GetMACAddress());
			// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
			//AddPrintData(strPrintData, _T("  %-21.21s: %-15.15s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"DNS SERVER (2ND)"),
			//	MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ATM_DNS_2ND));
			// end of [#RWC6-10]
		}
		// end of [#2028]
	}

	// [#2468] AU KSK 2017.01.18 전국가 적용
	// LANGUAGES
//#if (AU_C_VERSION)
//	// DC Payments (AU_C) doesn't support LANGUAGES
//#else
	strTemp.Empty();				strTemp2.Empty();
	for (int i = 0; i < LANGUAGE_MODE_MAX; i++)
	{

		if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ENGMODE + i) == ENABLE)
			strTemp.Format(L"%s%s,", strTemp, GET_LANGUAGE_STRING_SHORT(i + 1));
		else
			strTemp2.Format(L"%s%s,", strTemp2, GET_LANGUAGE_STRING_SHORT(i + 1));
	}

	// remove last ',' character. ( #1, #2, => #1, #2 )
	if (strTemp.IsEmpty())			strTemp.Format(_T("(%s)"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("none")));
	else							strTemp.Delete(strTemp.GetLength() - 1, 1);
	if (strTemp2.IsEmpty())			strTemp2.Format(_T("(%s)"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("none")));
	else							strTemp2.Delete(strTemp2.GetLength() - 1, 1);

	AddPrintData(strPrintData, _T("%-40.40s"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("LANGUAGES")));
	AddPrintData(strPrintData, _T("  %-12.12s : %-23.23s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENABLE_RCPT), strTemp);
	AddPrintData(strPrintData, _T("  %-12.12s : %-23.23s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_DISABLE_RCPT), strTemp2);

	// SUPERVISOR LANGUAGE
#if (US_VERSION || CA_VERSION)		// SUPERVISOR LANGAUGE is visible in US/CA
	AddPrintData(strPrintData, _T("  %-12.12s : %-3.3s  %12.12s : %-3.3s"),
		m_pDevCmn->fstrSCR_GetStringByTextID(_T("DEFAULT")),
		GET_LANGUAGE_STRING_SHORT(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE)),
		m_pDevCmn->fstrSCR_GetStringByTextID(_T("SUPERVISOR")),
		GET_LANGUAGE_STRING_SHORT(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SUPERVISOR_LANGUAGE)));
#else
	AddPrintData(strPrintData, _T("  %-12.12s : %-3.3s"),
		m_pDevCmn->fstrSCR_GetStringByTextID(_T("DEFAULT")),
		GET_LANGUAGE_STRING_SHORT(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE)));
#endif
	//#endif	// end of (AU_C_VERSION)
		// end of [#2468]

		// CARD READER
		// [#2084] NH KSK 2011.07.15 CARD TYPE 추가
	CString strCardReader = _T("");

	if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_NH_DIP)
		strCardReader = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_NH_DIP);
	else if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_MAGTEK_DIP)
		strCardReader = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MAGTEK_DIP);
	else if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_SANKYO_DIP)
		strCardReader = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SANKYO_DIP);
	else
		strCardReader = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_UNKNOWN);

	AddPrintData(strPrintData, L"%-25.25s : %-12.12s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CARDREADER_TYPE),
		strCardReader);

	// SANKYO MCU with ANTI SKIMMING
	if (m_pDevCmn->GetDeviceType(L"MCU") == 4)
	{
		// [#2167] NH Justin 2012.11.14 Determine AntiSkimming availability with Sankyo MCR EP Firmware
		if (m_pDevCmn->fbESU_IsAntiSkimmingAvailable())
		{
			// ANTI SKIMMING
			AddPrintData(strPrintData, L"  %-23.23s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ANTISKIM),
				m_pDevCmn->fstrSCR_GetStringFromTextID(
					GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_ENABLE))
				));
			// SHUTTER
			AddPrintData(strPrintData, L"  %-23.23s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SHUTTER),
				m_pDevCmn->fstrSCR_GetStringFromTextID(
					GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_SHUTTER_ENABLE))
				));
			// TIME THRESHOLD
			AddPrintData(strPrintData, L"  %-23.23s : %d %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TIME_THRES), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_TIMETHRESHOLD),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SEC));
		}
		// End of [#2167]
	}
	// end of [#2084]

//------------------------------------------------------------------------------------------------

	// [#2205] US KSK 2013.06.28
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
	{
		// [#2434] AU Kook 2016.07.22 Receipt Optimizing with refactoring
		// Halo Led Setup 정보 추가
		// 1. IN SERVICE
		LIB_GetHaloLedPrintData(strPrintData, 1);		// IN SERVICE

		// 2. TRANSACTION
		LIB_GetHaloLedPrintData(strPrintData, 2);		// TRANSACTION

		// 3. DISPENSING
		LIB_GetHaloLedPrintData(strPrintData, 3);		// DISPENSING

		// 4. OUT OF SERVICE
		LIB_GetHaloLedPrintData(strPrintData, 4);		// OUT OF SERVICE
		// end of [#2434]
	}
	// end  of [#2205]

	// [#2432] US Justin 2016.06.15 Add MoniMobile in Print All Setup 
#if (US_VERSION || CA_VERSION)
	AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MONIMOBILEQR),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MOBILEAPP_QRCODE))));

	// [#2538] NH Justin 2018.03.09 MoniMobile Cash Balance Option
	AddPrintData(strPrintData, L"  %-23.23s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_QRBALANCE),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_MONIMOBILE_CASHBALANCE(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MONIMOBILE_CASHBALANCE))));
	// End of [#2538]
#endif
	// end of [#2432]

	// [#2518] US Kook 2018.01.23 Support MX-2800SE, Camara
	if (GetConfigFuncPointer()->SupportsCamera())	// [#GLDV-2505] Supoprt MX-2800T
	{
		//AddPrintData(strPrintData, _T("%-40.40s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CAMERA));

		AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CAMERA),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE))));

		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE) == ENABLE)					// [#2548] NH Justin 2018.04.27 Change Camera Option Name : MoniVision, Live-Feed, Journal Pic
		{
			AddPrintData(strPrintData, L"  %-23.23s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ANTI_SHLDR_SURF),
				m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
					MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ANTI_SHLDR_SURF_ENABLE))));

			AddPrintData(strPrintData, L"  %-23.23s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_FACE_MEMORIZE),
				m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
					MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_MEMORIZE_ENABLE))));

			{
#ifdef UNDER_CE
				LONGLONG nFreeSpaceATM2 = m_pDevCmn->fnCAM_GetFreeSpaceInfo() / UNIT_KILOBYTE;

				CString strAvailableATM2Space;
				strAvailableATM2Space.Format(_T("%lli"), nFreeSpaceATM2);
				strAvailableATM2Space = MakeMoneyCent(strAvailableATM2Space + _T("00"));
				strAvailableATM2Space.Replace(_T(".00"), _T(""));

				AddPrintData(strPrintData, L"  %-23.23s : %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_FREESPACE),
					strAvailableATM2Space,
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_KILOBYTE)
				);
#endif
			}
		}
	}
	// end of [#2518]

	// [#GLDV-3005] US Kook 2022.04.07 support side car
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
	{
		AddPrintData(strPrintData, L"%-25.25s : %s", L"SIDE CAR",
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT))));

		AddPrintData(strPrintData, L"  %-23.23s : %s", L"BNA TYPE",
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_BNATYPE) == BNATYPE_MEI ? L"MEI" : L"JCM");
	}
	// end of [#GLDV-3005]

	// [#2558] NH Justin 2018.06.14 Add Scheduled Reboot Optioin
	{
		int nRebootOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_OPTION);
		AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SCHEDULED_REBOOT),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_REBOOT_OPTION_APTEXTID(nRebootOption)));

		if (nRebootOption == REBOOT_OPTION_ONTIME)
		{
			int nRebootTime = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_TIME);
			AddPrintData(strPrintData, L"  %-23.23s : %02d:%02d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_REBOOT_TIME),
				(int)(0.01 * nRebootTime), (nRebootTime % 100));
		}
		else if (nRebootOption == REBOOT_OPTION_INTERVAL)
		{
			AddPrintData(strPrintData, L"  %-23.23s : %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_REBOOT_INTERVAL),
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_INTERVAL));
		}
	}
	// End of [#2558]

	// [#RWC6-149] Support Remote Updates on PAS
	AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RU_SCHEDULED),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_ENABLE))));
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_ENABLE) == ENABLE)
	{
		int dow = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_DOW);
		CString dowString = m_pDevCmn->fstrSCR_GetStringFromTextID(GET_STRING_FOR_DOW(dow));
		AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RU_DOW), dowString);

		CString config = L"N/A";
		CUpdateRepoConfigurationManager manager;
		UpdateSourceInfo	pUpdateConfiguration;
		CONFIGERR err = manager.GetConfiguration(&pUpdateConfiguration);
		if (err == ERR_OK)
		{
			int nConfigLen = 0;
			AddPrintData(strPrintData, L"%-25.25s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RU_CURRCONFIG));

			config.Format(L"%s", pUpdateConfiguration.RepositoryBaseURL);
			nConfigLen = config.GetLength();
			if (nConfigLen <= 25)
			{
				AddPrintData(strPrintData, L"  %-10.10s : %-25.25s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RU_HOST), config);
			}
			else if (nConfigLen > 25)
			{
				AddPrintData(strPrintData, L"  %-10.10s : %-25.25s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RU_HOST), config.Mid(0, 25));
				AddPrintData(strPrintData, L"               %-25.25s", config.Mid(25));
			}

			config.Format(L"");
			for (int i = 0; i < pUpdateConfiguration.Hives.GetCount(); i++)
			{
				config.AppendFormat(L"%s ", pUpdateConfiguration.Hives.GetAt(i));
			}
			nConfigLen = config.GetLength();
			if (nConfigLen <= 25)
			{
				AddPrintData(strPrintData, L"  %-10.10s : %-25.25s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RU_HIVES), config);
			}
			else if (nConfigLen > 25)
			{
				AddPrintData(strPrintData, L"  %-10.10s : %-25.25s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RU_HIVES), config.Mid(0, 25));
				AddPrintData(strPrintData, L"               %-25.25s", config.Mid(25));
			}

			config.Format(L"%d", pUpdateConfiguration.MinimumSeverity);
			AddPrintData(strPrintData, L"  %-10.10s : %-25.25s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RU_SEVERITY), config);
		}
		else
		{
			AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RU_CURRCONFIG), config);
		}
	}
	// end of [#RWC6-149]
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: LIB_GetCustomerSetupPrintData()
RETURN TYPE  :
PARAMETER    :
DESCRIPTION  : '3. CUSTOMER SETUP'
-------------------------------------------------------------------*/
void CTranCmn::LIB_GetCustomerSetupPrintData(CString& strPrintData)
{
	CString strTemp, strTemp2;

	// Welcome Message
	AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_WC_MSG));
	strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WELCOME_MESSAGE1);
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WELCOME_MESSAGE2).GetLength() > 0)
		strTemp += FIELD_DELIMITER + MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WELCOME_MESSAGE2);
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WELCOME_MESSAGE3).GetLength() > 0)
		strTemp += FIELD_DELIMITER + MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WELCOME_MESSAGE3);

	if (strTemp.GetLength() <= 2)	// FIELD_DELIMITER만 2개 있는 경우 (메시지가 공란인 경우)
		strTemp.Format(L"  (%s)", m_pDevCmn->fstrSCR_GetStringByTextID("none"));
	AddPrintData(strPrintData, strTemp);


#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	// [#2031] NZ KJW 2011.03.16
	// [#2000] SOOK 2010.10.12 호주 사양 적용 (Print All Setup)	

// EXIT MESSAGE (3 lines)
	AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_EXIT_MSG));
	strTemp.Format(L"%s%c%s%c%s", MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EXIT_MESSAGE1), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EXIT_MESSAGE2), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EXIT_MESSAGE3));
	if (strTemp.GetLength() <= 2)	// FIELD_DELIMITER만 2개 있는 경우 (메시지가 공란인 경우)
		strTemp.Format(L"  (%s)", m_pDevCmn->fstrSCR_GetStringByTextID("none"));
	AddPrintData(strPrintData, strTemp);


	// MARKETING MESSAGE
	AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MARKETING_MSG));
	strTemp.Format(L"%s%c%s%c%s%c%s", MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MARKETING_MESSAGE1), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MARKETING_MESSAGE2), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MARKETING_MESSAGE3), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MARKETING_MESSAGE4));
	if (strTemp.GetLength() <= 3)	// FIELD_DELIMITER만 3개 있는 경우 (메시지가 공란인 경우)
		strTemp.Format(L"  (%s)", m_pDevCmn->fstrSCR_GetStringByTextID("none"));
	AddPrintData(strPrintData, strTemp);


	// STORE MESSAGE
	AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_STORE_MSG));
	strTemp.Format(L"%s%c%s%c%s%c%s", MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_STORE_MESSAGE1), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_STORE_MESSAGE2), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_STORE_MESSAGE3), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_STORE_MESSAGE4));
	if (strTemp.GetLength() <= 3)	// FIELD_DELIMITER만 3개 있는 경우 (메시지가 공란인 경우)
		strTemp.Format(L"  (%s)", m_pDevCmn->fstrSCR_GetStringByTextID("none"));
	AddPrintData(strPrintData, strTemp);


	// PROCESSOR MESSAGE
	AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PROC_MSG));
	strTemp.Format(L"%s%c%s%c%s%c%s", MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PROCESSOR_MESSAGE1), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PROCESSOR_MESSAGE2), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PROCESSOR_MESSAGE3), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PROCESSOR_MESSAGE4));
	if (strTemp.GetLength() <= 3)	// FIELD_DELIMITER만 3개 있는 경우 (메시지가 공란인 경우)
		strTemp.Format(L"  (%s)", m_pDevCmn->fstrSCR_GetStringByTextID("none"));
	AddPrintData(strPrintData, strTemp);

#endif  //end of [#2000]


	// RECEIPT HEADER
	AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RCPT_HEADER));
	strTemp.Format(L"%s%c%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER1), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER2));
	if (strTemp.GetLength() <= 1)	// FIELD_DELIMITER만 1개 있는 경우 (메시지가 공란인 경우)
		strTemp.Format(L"  (%s)", m_pDevCmn->fstrSCR_GetStringByTextID("none"));
	AddPrintData(strPrintData, strTemp);


	// RECEIPT TAIL
	AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RCPT_TAIL));
	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER3).GetLength() > 0)
		AddPrintData(strPrintData, L"%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER3));
	else
		AddPrintData(strPrintData, L"  (%s)", m_pDevCmn->fstrSCR_GetStringByTextID("none"));


	// RECEIPT ADDRESS & PHONE NUMBER
	AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RCPT_ADDR_PHONE_NUM));
	strTemp.Format(L"%s%c%s%c%s%c%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS2), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS3), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_PHONENO));
	if (strTemp.GetLength() <= 3)	// FIELD_DELIMITER만 3개 있는 경우 (메시지가 공란인 경우)
		strTemp.Format(L"  (%s)", m_pDevCmn->fstrSCR_GetStringByTextID("none"));
	AddPrintData(strPrintData, strTemp);


#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	// ATM OPERATOR FEE
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ATM_OPER_FEE), m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE))));
	// ATM OPERATOR FEE OWNER
	AddPrintData(strPrintData, L"  %-25.25s :", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ATM_OPER_FEE_OWNER));
	AddPrintData(strPrintData, L"%-40.40s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER));
	// ATM OPERATOR FEE CONTACT INFO
	AddPrintData(strPrintData, L"  %-38.38s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ATM_OPER_FEE_CINFO));
	strTemp.Format(L"%s%c%s%c%s",
		MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT1), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT2), FIELD_DELIMITER,
		MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT3));

	// KSK 2017.01.25 Contact Info가 모두 Space인 경우 none 처리 보완
//	if (strTemp.GetLength() <= 2)	// FIELD_DELIMITER만 2개 있는 경우 (메시지가 공란인 경우)
//		strTemp.Format(L"  (%s)", m_pDevCmn->fstrSCR_GetStringByTextID("none"));

	strTemp2 = strTemp;
	strTemp2.Replace(L" ", L"");
	if (strTemp2.GetLength() <= 2)
		strTemp.Format(L"  (%s)", m_pDevCmn->fstrSCR_GetStringByTextID("none"));
	// end of KSK 2017.01.25

	AddPrintData(strPrintData, strTemp);

	// WITHDDRAWAL AMOUNT
	AddPrintData(strPrintData, L"  %-25.25s : %s%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_WITHDRAW_AMOUNT), GetCurrencySymbol(), MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT)));
	// BALANCE AMOUNT
	strTemp.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_INQUIRY_SURCHARGE));
	AddPrintData(strPrintData, L"  %-25.25s : %s%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_BALAMT), GetCurrencySymbol(), MakeMoneyCent(strTemp));

#else //end of [#2000]

	// SURCHARGE OWNER
	AddPrintData(strPrintData, L"%-25.25s : %-12.12s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SCHGOWNER),
		MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER));

	// SURCHARGE MODE
#if !(MX_VERSION)	// [#2115] MX KSK 2012.02.05
	AddPrintData(strPrintData, L"%-25.25s : %-12.12s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SCHGMODE),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE))));
#endif				// end of [#2115]

	// SURCHARGE AMOUNT
#if !(MX_VERSION)	// [#2115] MX KSK 2012.02.05
	AddPrintData(strPrintData, L"%-25.25s : %s%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SCHGAMT),
		GetCurrencySymbol(), MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT)));
#endif				// end of [#2115]

	// BANK NAME & FEE PRINT
#if (MX_VERSION)	// [#2137] MX PCS 2012.07.25 " MX의 경우 Host에서 매 거래 마다 수수료가 나옴."
	AddPrintData(strPrintData, L"%-22.22s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_BANKNAME_FEEPRINT),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_BANK_NAME_FEE_PRINT))));
#endif				// end of [#2137]
#endif

#if (US_VERSION || CA_VERSION || AU_VERSION)	// [#2115] MX KSK 2012.02.05
	// [#2220] AU KMK 2014.01.10 호주 Percent Surcharge Mode 추가지원
// [#2434] AU Kook 2016.07.22 Receipt Optimizing with refactoring
// ADDITIONAL SURCHARGE MODE
	CString strAddSurchargeMode = _T("");
	switch (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE))
	{
	case PERCENT_MODE:
		strAddSurchargeMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PCT);		break;
	case TABLE_MODE:
		strAddSurchargeMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TABLE);		break;
	default:
		strAddSurchargeMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_NOT_USED);	break;
	}

	if (strAddSurchargeMode.GetLength() < 11)
		strAddSurchargeMode = " " + strAddSurchargeMode;		// alignment
	AddPrintData(strPrintData, L"%-27.27s :%-11.11s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ADD_SCHG_MODE),
		strAddSurchargeMode);

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) == PERCENT_MODE)
	{
		// PERCENT MANNER
		CString strPercentManner = _T("");
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_MANNER) == GREATER)
			strPercentManner = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_GREATER);
		else
			strPercentManner = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LESSER);
		AddPrintData(strPrintData, L"  %-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PCT_MNR),
			strPercentManner);

		// PERCENT AMOUNT
		// Percent 값은 항상 찍어줘야 한다.		// [#2316] NH Justin 2014.12.17 Decimal Surcharge
		AddPrintData(strPrintData, L"  %-25.25s : %0.2f %%", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PCT_AMT),
			0.01 * MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE));

		// SURCHARGE NOTICE	-> PERCENTAGE NOTICE
		// [#2354] US Justin 2015.07.01 Surcharge Notice : Amount, Percentage, Amount and Percentage
		CString strSurchargeNotice = m_pDevCmn->fstrSCR_GetStringFromTextID(
			GET_SURCHARGENOTICE_OPTION(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PERCENT_SURCHARGE_NOTICE)));
		if (strSurchargeNotice.GetLength() < 11)
			strSurchargeNotice = " " + strSurchargeNotice;		// alignment
		AddPrintData(strPrintData, L"  %-25.25s :%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PRECENTNOTICE),
			strSurchargeNotice);
		// End of [#2354]
	}
	// end of [#2434]
#endif

	// [#2309] US Justin 2014.11.17 Balance No fee Notice
#if (US_VERSION)
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_BALANCEFEE_NOTICE),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_BALANCE_NOFEE_NOTICE))));
#endif
	// End of [#2309]

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	// BIN LIST TOTAL COUNT
	AddPrintData(strPrintData, L"%-27.27s : %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_BINLIST_TOTCNT),
		m_pDevCmn->m_BINMgr.GetTotalBinCount());
#elif (US_VERSION || CA_VERSION)		// [#2115] MX KSK 2012.02.05
	// BIN TOTAL COUNT
	int	nCount = MemGetInt(_MEM_FLD_APP_BININFO, _MEM_VAR_APP_SETBINLIST);
	AddPrintData(strPrintData, L"%-27.27s : %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_BIN_TOTCNT), nCount);

	// EXTENDED BIN TOTAL COUNT
	for (int i = 0; i < nCount; i++)
	{
		strTemp2 = MemGetStr(_MEM_FLD_APP_BININFO, _MEM_VAR_APP_BINLIST + i);
		strTemp2.TrimLeft();

		if (strTemp2.GetLength() > 0)
			AddPrintData(strPrintData, L" #(%02d) : %s", i + 1, strTemp2);
	}
	AddPrintData(strPrintData, L"%-27.27s : %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_EXTDBIN_TOTCNT),
		m_pDevCmn->m_arrExtendedBinList.GetSize());
#endif

	// MOD10
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MOD10),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MOD10_ENABLE))));

#if (EMV_LEVEL2)
	// EMV
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_EMV),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(m_pDevCmn->fnMCU_IsEmvEnable())));

	if (m_pDevCmn->fnMCU_IsEmvEnable())
	{
		// LATCH OPTION
		AddPrintData(strPrintData, L"  %-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LATCHOPT),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_REVERSE_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_LATCH_ENABLE))));
		// [#2280] NH Justin Make ContinueFallBack as Default
		/*
		// [#2209] NH Justin 2013.07.11 Add Fall back continue Option - US ONLY
		#if (US_VERSION)
		AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONTINUEFALLBACK),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_CONTINUE_FALLBACK))));
		#endif
		// End of [#2209]
		*/
		// End of [#2280]

		// [#2342] US Justin 2015.05.08 Print Enabled AID using NVRAM and US COMMON AID
		// [#2456] NH Justin 2016.12.07 Change Enagled AID on PRINT ALL SETUP
		/*
		AddPrintData(strPrintData, L"  %-25.25s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_AIDLIST_CNT));
		strTemp = MemGetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID);
		CStringArray strEnabledAIDArray;
		SplitString( strTemp, AID_FIELD_DELIMITER, strEnabledAIDArray);
		int nEnabledAIDNum = 0;
		for (int nEnAID=0; nEnAID<strEnabledAIDArray.GetSize(); nEnAID++)
		{
			strTemp = strEnabledAIDArray.GetAt(nEnAID);
			if( strTemp.GetLength()>=10 )
			{
				nEnabledAIDNum++;
				AddPrintData(strPrintData, L"    #(%03d) : %s", nEnabledAIDNum, strTemp);
			}
		}
		*/

		//#if !(AU_C_VERSION)	// [#2425] AU KSK 2016.12.26 EMV Language Option 누락분 적용
#if (US_VERSION || AU_VERSION)	// [#2469] AU KSK 2017.01.18 US와 AU만 지원함
		AddPrintData(strPrintData, L"  %-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_EMV_LAN_SELECT),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_LANGUAGESELECTION))));
#endif				// end of [#2425]

		CString strAIDEnDisable = MemGetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID);
		CStringArray strEnabledAIDArray;
		SplitString(strAIDEnDisable, AID_FIELD_DELIMITER, strEnabledAIDArray);
		int nNumEnabledAID = strEnabledAIDArray.GetSize();
		if ((nNumEnabledAID > 0) && (strEnabledAIDArray.GetAt(nNumEnabledAID - 1).GetLength() <= 10))
			nNumEnabledAID--;
		AddPrintData(strPrintData, L"  %s (%d)", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_AIDLIST_CNT), nNumEnabledAID);

		char buf[256] = { 0, };
		char szIndex[16] = { 0, };
		CINIFile iniEMV(GET_EMV_AID_POOLFILENAME(m_pDevCmn->fnEMV_GetKernelVersion()));

		// Get Total AID NUmber
		iniEMV.GetPrivateProfileString("AIDCOUNT", "TOTALCOUNT", buf, 256);
		strTemp.Format(L"%S", buf);
		int nTotalAIDList = Asc2Int(strTemp);

		for (int nEnAID = 0; nEnAID < nTotalAIDList; nEnAID++)
		{
			sprintf(szIndex, "TERMINAL%d", nEnAID + 1);
			memset(buf, 0, sizeof(buf));
			strTemp2.Empty();
			if (iniEMV.GetPrivateProfileString(szIndex, "AID", buf, 256))						// AID
				strTemp2.Format(L"%S", buf);

			if ((strTemp2.GetLength() >= 10) && (strAIDEnDisable.Find(strTemp2) >= 0))			// Enabled.
			{
				memset(buf, 0, sizeof(buf));
				strTemp.Empty();
				if (iniEMV.GetPrivateProfileString(szIndex, "APPLICATIONNAME", buf, 256))		// APPLICATION NAME
					strTemp.Format(L"%S", buf);
				AddPrintData(strPrintData, L"   %s (%s)", strTemp2, strTemp);
			}
		}
		// End of [#2456]

#if (US_VERSION)
	// USE COMMON AID
		AddPrintData(strPrintData, L"  %-23.23s   : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_COMMON_AID),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_REVERSE_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_USE_USCOMMONAID))));

		// [#2549] US Justin 2018.05.08 US Territory	
		AddPrintData(strPrintData, L"  %-23.23s   : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_US_TERRITORY),
			GET_US_TERRITORY_OPTION_STRING(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_US_TERRITORY_OPTION)));

		// [#2481] US Justin 2017.04.21 Fallback Option for Unknown AID
		// Fallback Option for Unknown AID
		// [#2484] US Justin 2017.06.12
		//if(m_pDevCmn->fnEMV_GetKernelVersion() >= EMV_KERNEL_V6)			// [#2564] US Justin 2018.07.17 Enable Fallback OP Options for EMV KERNEL 5.5		
		{
			AddPrintData(strPrintData, L"  %-23.23s   : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_FALLBACK4UNKNOWNAID),
				m_pDevCmn->fstrSCR_GetStringFromTextID(GET_FB4UNKNOWNAID_APTEXTID(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_4_UNKNOWNAID))));
			// [#2517] US Justin 2017.11.29 EMV Fallback Enable/Diable Option
			AddPrintData(strPrintData, L"  %-23.23s   : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_FALLBACK_ENDISABLE),
				m_pDevCmn->fstrSCR_GetStringFromTextID(GET_REVERSE_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_ENABLE_DISABLE))));
			// End of [#2517]
		}
		// End of [#2481]
#endif
	// End of [#2342]

	}			// end of 'fnMCU_IsEmvEnable()'

#endif			// end of '(EMV_LEVEL2)'

	// SELECT RECEIPT
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SEL_RCPT),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SELECTRECEIPT))));

	// [#2209] NH Justin 2013.07.11 Add Receipt on Screen - PRINT ALL SETUP
#if !(AU_VERSION)
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RECEIPTONSCREEN),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_REVERSE_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN))));
#endif
	// End of [#2209]

	// DUAL RECEIPT
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DUAL_RCPT),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_DUAL_RECEIPT_APTEXTID(
		MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUAL_RECEIPT))));

	// [#2434] AU Kook 2016.07.26 Receipt Optimizing with refactoring
	// REVERSAL RETRY COUNT
	AddPrintData(strPrintData, L"%-27.27s : %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_REV_RETRYCNT),
		MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REVERSAL_RETRY));
	// end of [#2434]

#if (US_VERSION || MX_VERSION)	// [#2115] MX KSK 2012.02.05
	// [#2291] MX Justin 2014.08.27 Remove Balance Inquiry Info if TDL is disabled in Mexico AP
	//BOOL bPrintBalanceInfo = TRUE;
	/*
	// [#2160] CA KMK 2012.11.21
	AddPrintData(strPrintData, L"%-18.18s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_BAL_ATSTART), m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PREBALANCE))));
	AddPrintData(strPrintData, L"%-26.26s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONTAFTER_PREBAL), m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CONTINUETRANS_ENABLE))));
	// end of [#2160]
	*/

	// [#2340] US JUstin 2015.04.20 Enable Pre Balance for Mexico Regardless of TDL OPTION
	//#if( (MX_VERSION)&&(!APP_TDL_OPTION) )
	//	bPrintBalanceInfo = FALSE;
	//#endif
	// End of [#2340]

	//if(bPrintBalanceInfo)
	{
		AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_BAL_ATSTART),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PREBALANCE))));
		AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONTAFTER_PREBAL),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CONTINUETRANS_ENABLE))));
	}
	// End of [#2291]
#endif

// [#2444] US Justin 2016.09.02 Enable Dual Balance to All Customers.
#if (APP_DUALBALANCE)				// [#2467] 2017.01.24 Bug Fix.. Add "APP_TDL_OPTION". Bug:Skipping Dual Balance Option on Cardtronics version. 
									// [#2482] 2017.04.28 US Justin Make one Option for Dual Balance to avoid confusion
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DUALBALANCE),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE))));
#endif
	// End of [#2444]

	// [#2292] US Justin 2014.10.10 Print All Setup for TDL OPTION 
#if(APP_TDL_OPTION)			
	// [#2444] US Justin 2016.09.02 Enable Dual Balance to All Customers

	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TDL_DYNAMICSURCHARGE),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_TIRSURCHARGE))));
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TDL_DCC),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DCC))));
	AddPrintData(strPrintData, L"%-27.27s : %s%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TDL_ADDITIONALCRFEE),
		GetCurrencySymbol(),
		MakeMoneyCent(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE)));

#if (US_VERSION)		// [#2437] US Justin
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TDL_ALLPOINTSURCHARGE),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_SURCHARGE))));
#endif
	// End of [#2292]
#elif(APP_CUSTOM_PAI)	// [#RWC6-29] PAI Add ALLPOINT
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TDL_ALLPOINTSURCHARGE),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_SURCHARGE))));
#elif(US_VERSION && !APP_TDL_OPTION)
	// To add credit card segmentation for US general version as well.
	AddPrintData(strPrintData, L"%-27.27s : %s%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TDL_ADDITIONALCRFEE),
		GetCurrencySymbol(),
		MakeMoneyCent(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE)));
#endif

	// ACCOUNT
#if !(MX_VERSION)	// [#2115] MX KSK 2012.02.05
	CString strFormat = _T("");
	strFormat.Format(L"%%-14.14s %%-10.10s   : %%s");
	AddPrintData(strPrintData, strFormat, S_CHECKING, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_008),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING))));
	AddPrintData(strPrintData, strFormat, S_SAVINGS, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_008),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS))));
	AddPrintData(strPrintData, strFormat, L"CREDIT", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_008),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD))));
#endif				// end of [#2115]

	// Standard1 Enhanced Option
#if (APP_STD1_ENHANCED_V2)	
	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
	{
		// [#2434] AU Kook 2016.07.28 Receipt Optimizing with refactoring
		// DYNAMIC FLOW
		AddPrintData(strPrintData, L"%-27.27s : %s",
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DYNAMICFLOW),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_STD1_DYNAMICFLOW_OPTION_APTEXTID(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE))));

		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE) == ENABLE)
		{
			// DF 1ST CALL
			// [#2242] US Justin 2013.12.18 Dynamic Flow 1st Call Option
#if !(AU_VERSION)	// KSK 2017.01.06 AU 1st Call 미지원
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION) == DYNAMICFLOW_1STCALL_OFF)
				strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_DISABLE_RCPT);
			else
				strTemp = m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENABLE_RCPT);
			AddPrintData(strPrintData, L"  %-25.25s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DF1STCALL),
				strTemp);
#endif
			// End of [#2242]

			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC) == ENABLE)
			{
				// CURRENCY CONVERSION
				AddPrintData(strPrintData, L"  %-25.25s: %s",
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CURRENCYCONVERSION),
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENABLE_RCPT));

#if !(AU_VERSION)	// [#2362] AU KSK 2015.07.20 AU 미지원
				// CONVERSION OPTION
				AddPrintData(strPrintData, L"    %-23.23s: %s",
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DCCOPTION),
					GET_DCC_CUST_OPTION_STRING(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION)));

				// DCC LOCAL SURCHARGE
				// [#2349] US Justin 2015.06.05 DCC LOOKUP Local Surcharge
				AddPrintData(strPrintData, L"    %-23.23s: %s",
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DCCLOCALSURCHARGE),
					m_pDevCmn->fstrSCR_GetStringFromTextID(
						GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE))));
				// End of [#2349]
#endif		// end of [#2362]

// DYNAMIC FLOW DCC TYPE
// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
				AddPrintData(strPrintData, L"    %-23.23s: %s",
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DYNAMICFLOW_DCC_TYPE),
					GET_DYNAMICFLOW_DCC_TYPE_STRING(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE)));

				// end of [#RWC6-2, #2585]
			}
			else
				AddPrintData(strPrintData, L"  %-25.25s: %s",
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CURRENCYCONVERSION),
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_DISABLE_RCPT));

			// PIN CHANGE
			// [#2499] US Justin 2017.08.22 Disable PIN Chane UNTIL "EMV PIN Change" is implemented.
			//#if !(AU_VERSION)	// [#2362] AU KSK 2015.07.20 AU 미지원
			//CString strPinChangeType = _T("");
			//if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE) == 0)
			//	strPinChangeType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_DISABLE_RCPT);
			//else if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE) == 1)
			//	strPinChangeType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENABLE_RCPT);
			//else
			//	strPinChangeType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_HOST_DETERMINE);
			//AddPrintData(strPrintData, L"  %-25.25s : %s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PINCHANGE), strPinChangeType);	
			// End of [#2499]

			// end of [#2434]
			// [#2490] NH Justin 2017.06.14 Standard1 Dynamic Flow MAC Option
#if (CA_VERSION || US_VERSION)
			{
				int nEPPKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);
				if (nEPPKeyMode == KEYMODE_NON_UNIQ_SDES_MACING || nEPPKeyMode == KEYMODE_UNIQ_SDES_MACING ||
					nEPPKeyMode == KEYMODE_TDES_MACING || nEPPKeyMode == KEYMODE_TDES_TMACING)
				{
					AddPrintData(strPrintData, L"  %-25.25s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_STD1_DF_MAC),
						m_pDevCmn->fstrSCR_GetStringFromTextID(GET_STD1_DF_MAC_APTEXTID(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_STD1_DYNAMICFLOW_MAC))));
				}
			}
#endif
			// End of [#2490]
		}
	}
#endif

	// Dual Host DCC - Host Information
#if (US_VERSION)
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) == ENABLE)
	{
		AddPrintData(strPrintData, L"------ DUAL HOST DCC ------------------");

		// EN/DISABLE
		AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CURRENCYCONVERSION),
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ENABLE));

		// DUAL HOST DCC TYPE
		// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
		AddPrintData(strPrintData, L"    %-23.23s: %s",
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DUALHOST_DCC_TYPE),
			GET_DUALHOST_DCC_TYPE_STRING(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOST_DCC_TYPE)));
		// end of [#RWC6-2, #2585]

		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ISHOSTUSEURL) != 0)
		{
			AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_USE_URL),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ENABLE));
			AddPrintData(strPrintData, L"%-9.9s %-15.15s :", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_1),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_URL_NAME));
			AddPrintData(strPrintData, L"%s", MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST1NAME));
			AddPrintData(strPrintData, L"%-9.9s %-15.15s :", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_2),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_URL_NAME));
			AddPrintData(strPrintData, L"%s", MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST2NAME));
		}
		else
		{
			AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_USE_URL),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DISABLE));
			AddPrintData(strPrintData, L"%-9.9s %-15.15s :", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_1),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_IP_ADDR));
			AddPrintData(strPrintData, L"%s", MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST1NAME));
			AddPrintData(strPrintData, L"%-9.9s %-15.15s :", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_2),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_IP_ADDR));
			AddPrintData(strPrintData, L"%s", MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST2NAME));
		}
		AddPrintData(strPrintData, L"%-9.9s %-15.15s : %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_1),
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PORTNO),
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST1PORTNO));
		AddPrintData(strPrintData, L"%-9.9s %-15.15s : %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_2),
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PORTNO),
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST2PORTNO));

		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_ENABLE) != 0)
		{
			AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SSL),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ENABLE));

			int nSSLVer = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION);
			if (nSSLVer == CONN_UPTO_SSL_V30)	AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SSL_VER), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONN_UPTO_SSL_V30));
			else if (nSSLVer == CONN_UPTO_TLS_V10)	AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SSL_VER), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONN_UPTO_TLS_V10));
			else if (nSSLVer == CONN_UPTO_TLS_V11)	AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SSL_VER), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONN_UPTO_TLS_V11));
			else if (nSSLVer == CONN_UPTO_TLS_V12)	AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SSL_VER), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONN_UPTO_TLS_V12));
			else if (nSSLVer == CONN_UPTO_TLS_V13)	AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SSL_VER), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONN_UPTO_TLS_V13));
			else									AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SSL_VER), L"NOT DEFINED");
		}
		else
			AddPrintData(strPrintData, L"%-19.19s: %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SSL),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DISABLE));

		AddPrintData(strPrintData, L"----------------------------------------");
	}
#endif
	// End of [#2499]

#if (APP_POPMONEY)							//  [#2471] US Justin 2017.02.01 Enable popmoney to all customers.
	// Popmoney Setting (En/Disable)
	AddPrintData(strPrintData, L"%-27.27s : %s", L"Popmoney", m_pDevCmn->fstrSCR_GetStringFromTextID(
		GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_ENABLE))));
#endif

#if (APP_PIN4_CASHPICKUP)					//  [#2471] US Justin 2017.02.01 Enable Pin4 to all customers.
	// [#2405] US Justin 2016.03.21 HalCash Online
	// HalCash Setting (En/Disable) 
	AddPrintData(strPrintData, L"%-27.27s : %s", L"MasterCard Cash Pick-Up", m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE))));
	//if(  (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE)== ENABLE)&&(m_Pin4.m_bPin4Available==TRUE) ) // [#RWC6-11] MasterCard Cash PickUp "CommID" on PrintAllSetup
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE) == ENABLE) // [#RWC6-11] MasterCard Cash PickUp "CommID" on PrintAllSetup
	{
		// [#2554] US Justin 2018.06.06 Adjust MC Cash Pick-Up Setup
		/*
		AddPrintData(strPrintData, L" %-13.13s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOSTURL), m_Pin4.m_strHostIP);
		AddPrintData(strPrintData, L" %-13.13s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOSTPORT), m_Pin4.m_strHostPort);
		AddPrintData(strPrintData, L" %-13.13s : %s", L"SSL/TLS", m_Pin4.m_strHostSSL);
		AddPrintData(strPrintData, L" %-13.13s : %s", L"commId",  m_Pin4.m_strCommId);
		AddPrintData(strPrintData, L" %-13.13s : $%d", L"MAX WITHDRWAL",  Asc2Int(m_Pin4.m_strMaxDispenseLimit) );		// [#2433] US Justin 2016.06.23 Add Dispense Limit.
		AddPrintData(strPrintData, L" %-13.13s", L"Process");
		AddPrintData(strPrintData, L"    %s", m_Pin4.m_strHostProcess);
		*/
#if(APP_PRESTAGIN_PIN4)
		AddPrintData(strPrintData, L" %s (AUTHN) :", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOSTURL));
		AddPrintData(strPrintData, L"   %s", m_Pin4.m_strAuthHostIP);
		AddPrintData(strPrintData, L" %s (AUTHN): %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOSTPORT), m_Pin4.m_strAuthHostPort);
#endif
		AddPrintData(strPrintData, L" %s (AUTHZ) :", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOSTURL));
		AddPrintData(strPrintData, L"   %s", m_Pin4.m_strHostIP);
		AddPrintData(strPrintData, L" %s (AUTHZ): %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOSTPORT), m_Pin4.m_strHostPort);
		AddPrintData(strPrintData, L" %-13.13s : %s", L"commId", m_Pin4.m_strCommId);
		AddPrintData(strPrintData, L" %-13.13s : $%d", L"MAX WITHDRWAL", Asc2Int(m_Pin4.m_strMaxDispenseLimit));
		// End of [#2554]

		//if(m_Pin4.m_strHostSSLCert.GetLength()>1)
		//{
		//	AddPrintData(strPrintData, L" %-13.13s", L"Host Cert.");
		//	AddPrintData(strPrintData, L"    %s", m_Pin4.m_strHostSSLCert);
		//}
	}
	// End of [#2405]
#endif

	// Paydiant CCA
	// [#2446] US Justin 2016.10.11 Add Paypal CCA
#if (APP_PAYDIANT_CCA)
	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)	// [#RWC6-24] CCA feature in STD1 Only. Remove from STD3
		AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PAYPAL_CCA),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE))));
#endif
	// End of [#2446]

	// Just.Cash ALTPAY
	// [#2448] US Justin
#if (APP_JUST_CASH)
	if (m_JustCashData.m_bJustCashAvailable == TRUE)
	{
		AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_JUSTCASH),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_JUSTCASH_ENABLE_SVC_OPTION(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE))));
	}
#endif
	// End of [#2448]

	// [#RWC6-16] Bitload4U
#if (APP_B4U)
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_B4U_ENABLED) == ENABLE)
	{
		AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_B4U_FINANCIAL),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_B4U_ENABLE_SVC_OPTION(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_B4U_ENABLED))));
	}
#endif
	// End of [#RWC6-16]

// [#J006] 
#if (APP_GPAY_GIFTCARD_PURCHASE)
	AddPrintData(strPrintData, L"%-27.27s : %s", L"GivePay", m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_ENABLE_DISABLE))));
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_ENABLE_DISABLE))
		AddPrintData(strPrintData, L"  %-17.17s : %s", L"UP_SELL", m_pDevCmn->fstrSCR_GetStringFromTextID(GET_GIVEPAY_APPEAR_OPTION(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_UPSELL))));
#endif
	// End of [J#006]

#if !(MX_VERSION)	// [#2115] MX KSK 2012.02.05
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP)	// Dialup
	{
		// PRE DIALING
		AddPrintData(strPrintData, L"%-22.22s : %-12.12s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PREDIAL),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE))));

		// [#2434] AU Kook 2016.07.26 Receipt Optimizing with refactoring
		CString strPredialType = _T("");
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION) == 0)
			strPredialType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_AFTER_CARD);
		else if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION) == 1)
			strPredialType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_AFTER_PIN);
		else
			strPredialType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_AFTER_ACCT);

		// TYPE
		AddPrintData(strPrintData, L"  %-20.20s : %-15.15s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PREDIAL_TYPE),
			strPredialType);
		// end of [#2434]
	}
#endif				// end of [#2115]

	// [#528] AU AIREAT 2009.06.02 NEED MORE TIME
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	// NEED MORE TIME
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_NEEDMORETIME),
		m_pDevCmn->fstrSCR_GetStringFromTextID(
			GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME))));
#endif

	// RECEIPT PAPER LOW SENSOR
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RCPT_PAPER_LOW_SENS),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_REVERSE_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAPER_LOW_SENSOR_ENABLE))));

	// EPP FLICKER OPTION
	// [#2205] US KSK 2013.06.28
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
	{
		CString strEppFlickerOption = m_pDevCmn->fstrSCR_GetStringFromTextID(GET_EPP_OPTION_APTEXTID(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_FLICKER_OPTION)));

		if (strEppFlickerOption.GetLength() <= 10)
			AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_EPP_FLKR_OPT),
				strEppFlickerOption);
		else
		{
			AddPrintData(strPrintData, L"%-27.27s : ", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_EPP_FLKR_OPT));
			AddPrintData(strPrintData, L"%s", strEppFlickerOption);
		}
	}
	// end of [#2205]

	// CST SOUND OPTION
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CST_SND_OPT),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_CST_SOUND_OPTION_APTEXTID(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST_SOUND_OPTION))));

	// [#2241] AU KMK 2014.01.16 CDU 논리매수 차감 Option
#if (AU_VERSION)
#if (AU_A_VERSION)	// [#2323] AU KSK 2015.01.15
	AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringByTextID(_T("NEGATIVE COUNT")),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE))));
#endif				// end of [#2323]
#endif
	// end of [#2241]

	// [#11] NH KSK 2010.09.27
	// Notice
	NH_OS_VERSION	eOSVersion = m_pDevCmn->m_pConfig->GetOSVersion();		// [#11] NH KSK 2010.09.27
	//if (eOSVersion != NH_OS_NH1800SE_B)
	if (!(eOSVersion == NH_OS_NH1800SE_B || eOSVersion == NH_OS_NH1500SE))	// [#2267] US KSK 2014.05.08
	{
		AddPrintData(strPrintData, L"%-27.27s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_NTC),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_ENABLE))));

		// NOTICE가 ENABLE일 경우에만 해당 내용을 출력한다
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_ENABLE) == ENABLE)
		{
			// [#2434] AU Kook 2016.07.26 Receipt Optimizing with refactoring
			// TITLE
			AddPrintData(strPrintData, L"  %-7.7s:%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_NTC_TITLE),
				MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_TITLE));
			// end of [#2434]

			// MESSAGE
			AddPrintData(strPrintData, L"  %-7.7s:", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_NTC_MSG));
			for (int i = 0; i < 3; i++)
				AddPrintData(strPrintData, L"%s", MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_MSG1 + i));
		}
	}
	// end of [#11]

	// 1234567890123456789012345678901234567890
	// BACKGROUND SCREEN
	//   ENABLED            : #1,#2,#3,#4,#5,#6
	// 	 DISABLED           : (none)
	// 	 DEFAULT            : #4
	//	 CHANGE BACKGROUND  : ENABLE
	strTemp.Empty();				strTemp2.Empty();
	for (int i = 0; i < 6; i++)
	{
		if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_ENDISFLAG_TYPE1 + i) == ENABLE)	strTemp.Format(L"%s#%d,", strTemp, i + 1);
		else																			strTemp2.Format(L"%s#%d,", strTemp2, i + 1);
	}

	// remove last ',' character. ( #1, #2, => #1, #2 )
	if (strTemp.IsEmpty())			strTemp.Format(_T("(%s)"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("none")));
	else							strTemp.Delete(strTemp.GetLength() - 1, 1);
	if (strTemp2.IsEmpty())			strTemp2.Format(_T("(%s)"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("none")));
	else							strTemp2.Delete(strTemp2.GetLength() - 1, 1);

	AddPrintData(strPrintData, _T("%-40.40s"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("BACKGROUND SCREEN")));
	AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENABLE_RCPT), strTemp);
	AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_DISABLE_RCPT), strTemp2);
	AddPrintData(strPrintData, _T("  %-18.18s : #%d"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("DEFAULT")),
		MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_DEFAULT_TYPE));
	AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CHG_BGR),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_CHANGE_ENDISFLAG))));

	// end of [#2326]

	// [#2326] US Kook 2015.04.07 Support MoniAir, 명세표 사용률 개선
	// 1234567890123456789012345678901234567890
	// WELCOME ADVERTISEMENT
	//   ENABLED            : #1,#2,#3,#4,#5,#6
	//   DISABLED           : (none)
	//	 REFRESH TIME       : 5 sec
	strTemp.Empty();				strTemp2.Empty();
	for (int i = 0; i < 6; i++)
	{
		if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_ENDISFLAG1 + i) == ENABLE)	strTemp.Format(L"%s#%d,", strTemp, i + 1);
		else																	strTemp2.Format(L"%s#%d,", strTemp2, i + 1);
	}

	// remove last ',' character. ( #1, #2, => #1, #2 )
	if (strTemp.IsEmpty())			strTemp.Format(_T("(%s)"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("none")));
	else							strTemp.Delete(strTemp.GetLength() - 1, 1);
	if (strTemp2.IsEmpty())			strTemp2.Format(_T("(%s)"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("none")));
	else							strTemp2.Delete(strTemp2.GetLength() - 1, 1);

	AddPrintData(strPrintData, _T("%-40.40s"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("WELCOME ADVERTISEMENT")));
	AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENABLE_RCPT), strTemp);
	AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_DISABLE_RCPT), strTemp2);
	AddPrintData(strPrintData, _T("  %-18.18s : %d %s"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("REFRESH TIME")),
		MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_GUIDEDISP_TIME),
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SEC));
	// end of [#2326]


	// [#2326] US Kook 2015.04.07 Support MoniAir, 명세표 사용률 개선
	// 1234567890123456789012345678901234567890
	// TRANSACTION ADVERTISEMENT
	//   ENABLED            : #1,#2,#3,#4,#5,#6
	//   DISABLED           : (none)
	//	 REFRESH TIME       : 5 sec
	strTemp.Empty();				strTemp2.Empty();
	for (int i = 0; i < 6; i++)
	{
		if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_TRAN_ADV_ENDISFLAG1 + i) == ENABLE)	strTemp.Format(L"%s#%d,", strTemp, i + 1);
		else																			strTemp2.Format(L"%s#%d,", strTemp2, i + 1);
	}

	// remove last ',' character. ( #1, #2, => #1, #2 )
	if (strTemp.IsEmpty())			strTemp.Format(_T("(%s)"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("none")));
	else							strTemp.Delete(strTemp.GetLength() - 1, 1);
	if (strTemp2.IsEmpty())			strTemp2.Format(_T("(%s)"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("none")));
	else							strTemp2.Delete(strTemp2.GetLength() - 1, 1);

	AddPrintData(strPrintData, _T("%-40.40s"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("TRANSACTION ADVERTISEMENT")));
	AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENABLE_RCPT), strTemp);
	AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_DISABLE_RCPT), strTemp2);
	AddPrintData(strPrintData, _T("  %-18.18s : %d %s"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("REFRESH TIME")),
		MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_TRAN_ADV_DISP_TIME),
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SEC));
	// end of [#2326]

#if (AU_VERSION)	// [#2041] AU KSK 2011.03.31
	// BRAND NAME, 미입력시 "(none)" 출력
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AD_BRAND_NAME).GetLength() > 0)
		AddPrintData(strPrintData, L"%-20.20s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_BRDNAME),
			MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AD_BRAND_NAME));		// [#2160] CA KMK 2012.11.21 
	else
		AddPrintData(strPrintData, L"%-20.20s :  (%s)", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_BRDNAME),
			m_pDevCmn->fstrSCR_GetStringByTextID("none"));

#endif

#if (US_VERSION || CA_VERSION || MX_VERSION)	// [#620] US KSK 2010.02.24	
	// [#2115] MX KSK 2012.02.05

// [#2326] US Kook 2015.04.07 Support MoniAir, 명세표 사용률 개선
// 1234567890123456789012345678901234567890
// COUPON
//   COUPON TYPE        : BASIC COUPON
//   COUPON TYPE        : ENHANCED COUPON
	AddPrintData(strPrintData, _T("%-40.40s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CPN));
	AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CPNTYPE),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENHANCED_COUPON_TYPE_APTEXTID(MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_ENABLE))));

	if (MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_ENABLE) == 0)		// ENHANCED COUPON DISABLE 시
	{
		// 1234567890123456789012345678901234567890
		// 	 ENABLE             : #1,#2,#3,#4,#5,#6
		//	 DISABLE            : (none)
		//   COUPON    #1 MESSAGE
		//     (none)
		strTemp.Empty();				strTemp2.Empty();
		for (int i = 0; i < 6; i++)
		{
			if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_ENDISFLAG + i) == ENABLE)	strTemp.Format(L"%s#%d,", strTemp, i + 1);
			else																		strTemp2.Format(L"%s#%d,", strTemp2, i + 1);
		}

		// remove last ',' character. ( #1, #2, => #1, #2 )
		if (strTemp.IsEmpty())			strTemp.Format(_T("(%s)"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("none")));
		else							strTemp.Delete(strTemp.GetLength() - 1, 1);
		if (strTemp2.IsEmpty())			strTemp2.Format(_T("(%s)"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("none")));
		else							strTemp2.Delete(strTemp2.GetLength() - 1, 1);

		AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENABLE), strTemp);
		AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_DISABLE), strTemp2);

		for (int i = 0; i < 6; i++)
		{
			// [#2220] NH KMK 2014.02.25 명세표 사용률 개선
			// COUPON DISABLE 시 해당 COUPON MESSAGE 출력안함. COUPON MESSAGE 공란 시 (none) 출력함.
			if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_ENDISFLAG + i) == 1)		// ENABLE일 때
			{
				AddPrintData(strPrintData, L"  %-10.10s#%d %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CPN), (i + 1), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MSG));
				strTemp.Format(L"%s%c%s", MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_TEXT1 + (i * 3)), FIELD_DELIMITER,
					MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_TEXT2 + (i * 3)));
				if (strTemp.GetLength() <= 1)	// FIELD_DELIMITER만 1개 있는 경우 (메시지가 공란인 경우)
					strTemp.Format(L"  (%s)", m_pDevCmn->fstrSCR_GetStringByTextID("none"));

				AddPrintData(strPrintData, strTemp);
			}
			// end of [#2220]
		}
	}
	else
	{
		// Enhanced Coupon 추가
		// 1234567890123456789012345678901234567890
		//   COUPON #1 OPTION        : NOT USE
		//   COUPON #1 OPTION        : PAS UTILISER
		//   COUPON START TIME       : 9
		//   COUPON #1 HEURE DE DEBUT: 9

		LIB_EnhancedCouponDataProc(FALSE, FALSE);

		for (int i = 0; i < 6; i++)
		{
			AddPrintData(strPrintData, L"  %-7.7s#%d %-14.14s: %-12.12s",
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CPN), i + 1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_OPT),
				m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENHANCED_COUPON_OPTION_APTEXTID(
					MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_OPTION + i))));

			// [#2220] NH KMK 2014.02.25 ENHANCED COUPON 명세표 사용률 개선
			// 0: NOT USE	: TIME, MESSAGE 출력 안함
			// 1: ALWAYS	: TIME 출력 안함
			// 2: TIME		: 모두 출력 함
			switch (MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_OPTION + i))
			{
			case 2:	// TIME		: TIME 항목 출력
				AddPrintData(strPrintData, L"  %-7.7s#%d %-14.14s: %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CPN),
					i + 1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_STARTTIME),
					MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_PRINT_START_TIME + i));
				AddPrintData(strPrintData, L"  %-7.7s#%d %-14.14s: %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CPN),
					i + 1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ENDTIME),
					MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_PRINT_END_TIME + i));
				// 곧이어 MESSAGE 항목 출력 (no break)

			case 1:	// ALWAYS	: MESSAGE 항목 출력
				AddPrintData(strPrintData, L"  %-7.7s#%d %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CPN),
					i + 1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MSG));
				if (m_strarrEnhancedCouponData[i].GetSize() > 0)
				{
					AddPrintData(strPrintData, L"****************************************");
					for (int j = 0; j < m_strarrEnhancedCouponData[i].GetSize(); j++)
					{
						AddPrintData(strPrintData, L"%s", m_strarrEnhancedCouponData[i][j]);
					}
					AddPrintData(strPrintData, L"****************************************");
				}
				else
				{
					AddPrintData(strPrintData, L"  (%s)", m_pDevCmn->fstrSCR_GetStringByTextID("none"));
				}

			case 0:	// NOT USE	: 미출력
			default:
				// 쿠폰 관련 내용 출력하지 않음
				break;
			}
			// end of [#2220]
		}
	}
#else
	// [#2434] AU Kook 2016.07.26 Receipt Optimizing with refactoring
	// 1234567890123456789012345678901234567890
	// COUPON
	//   ENABLE             : #1,#2,#3,#4,#5,#6
	//   DISABLE            : (none)
	//   AWARD ON BIN       : #1,#2,#3,#4,#5,#6
	//   COUPON #1 MESSAGE
	// COUPON
	AddPrintData(strPrintData, _T("%-40.40s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CPN));

	// ENABLE, DISABLE, AWARD ON BIN
	strTemp.Empty();				// enabled COUPON
	strTemp2.Empty();				// disabled COUPON
	CString strAwardCpn = _T("");	// award COUPON
	for (int i = 0; i < 6; i++)
	{
		if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_ENDISFLAG + i) == ENABLE)
			strTemp.Format(L"%s#%d,", strTemp, i + 1);
		else
			strTemp2.Format(L"%s#%d,", strTemp2, i + 1);

		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AWARD_COUPON1_ENABLE + i) == ENABLE)
			strAwardCpn.Format(L"%s#%d,", strAwardCpn, i + 1);
	}

	// remove last ',' character. ( #1, #2, => #1, #2 )
	if (strTemp.IsEmpty())			strTemp.Format(_T("(%s)"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("none")));
	else							strTemp.Delete(strTemp.GetLength() - 1, 1);
	if (strTemp2.IsEmpty())			strTemp2.Format(_T("(%s)"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("none")));
	else							strTemp2.Delete(strTemp2.GetLength() - 1, 1);
	if (strAwardCpn.IsEmpty())		strAwardCpn.Format(_T("(%s)"), m_pDevCmn->fstrSCR_GetStringByTextID(_T("none")));
	else							strAwardCpn.Delete(strAwardCpn.GetLength() - 1, 1);

	AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENABLE), strTemp);
	AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_DISABLE), strTemp2);
	AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"),
		m_pDevCmn->fstrSCR_GetStringByTextID(_T("AWARD ON BIN")), strAwardCpn);

	// COUPON #n MESSAGE
	for (int i = 0; i < 6; i++)
	{
		if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_ENDISFLAG + i) == 1)		// ENABLE일 때
		{
			AddPrintData(strPrintData, L"  %-8.8s #%d %s",
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CPN), (i + 1),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MSG));

			strTemp.Format(L"%s%c%s",
				MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_TEXT1 + (i * 3)), FIELD_DELIMITER,
				MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_TEXT2 + (i * 3)));
			if (strTemp.GetLength() <= 1)	// FIELD_DELIMITER만 1개 있는 경우 (메시지가 공란인 경우)
				strTemp.Format(L"  (%s)", m_pDevCmn->fstrSCR_GetStringByTextID("none"));

			AddPrintData(strPrintData, strTemp);
		}
	}
	// end of [#2434]
#endif

	// Digital Receipt Option 추가
#if (AU_VERSION)
	AddPrintData(strPrintData, L"%-20.20s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DIGITAL_RECEIPT),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DIGITAL_RECEIPT_OPTION))));
#elif (CA_VERSION)
	AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DIGITAL_RECEIPT),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_REVERSE_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DIGITAL_RECEIPT_OPTION))));
#endif

#if (AU_A_VERSION)	// [#2459] AU KSK 2016.12.23
	// Dispense Mix Option 추가
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_MIX_OPTION) == CDM_MIX_MAXIMUM_NUMBER_OF_BILLS)
		strTemp.Format(L"%-20.20s : %s", m_pDevCmn->fstrSCR_GetStringByTextID(_T("DISPENSE MIX OPTION")), m_pDevCmn->fstrSCR_GetStringByTextID(_T("LOW TO HIGH")));
	else
		strTemp.Format(L"%-20.20s : %s", m_pDevCmn->fstrSCR_GetStringByTextID(_T("DISPENSE MIX OPTION")), m_pDevCmn->fstrSCR_GetStringByTextID(_T("HIGH TO LOW")));
	AddPrintData(strPrintData, strTemp);
#endif				// end of [#2459]

#if (APP_LIBERTYX)

	LXConfiguration config;
	CLibertyXConfigurationManager* manager = new CLibertyXConfigurationManager();

	if (manager->GetConfiguration(config) && config.IsEnabled())
	{
		AddPrintData(strPrintData, _T("%-40.40s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LIBERTYX));
		AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LTX_LOC), config.LocationID);
		AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LTX_RID), config.RoutingID);
		AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LTX_H1), config.Host1Url);
		AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LTX_H2), config.Host2Url);
		AddPrintData(strPrintData, _T("  %-18.18s : %d"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LTX_HP1), config.Host1Port);
		AddPrintData(strPrintData, _T("  %-18.18s : %d"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LTX_HP2), config.Host2Port);
		AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LTX_TLS), config.TlsEnabled ? L"Enabled" : L"Disabled");
		AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LTX_PROTO), config.GetHostProtocolName());
		AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LTX_ENV), config.GetHostEnvelopeName());
		AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LTX_URL), config.APIUrl);
		AddPrintData(strPrintData, _T("  %-18.18s : %-17.17s"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LTX_USER), config.Username);
	}

	delete manager;
	manager = NULL;

#endif

// [RWC6-676] Start SKKim 2024.05.20
#if (APP_TANGOPAY)
	AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TANGONET),
												 m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(m_TangoPayConfig.bEnable)));
	if (m_TangoPayConfig.bEnable == true)
	{
		AddPrintData(strPrintData, L"%-12.12s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TANGONET_IDEN_URL), m_TangoPayConfig.strIdentifyServerURL.Left(25));
		AddPrintData(strPrintData, L"%-12.12s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TANGONET_TRANS_URL), m_TangoPayConfig.strTransactionServerURL.Left(25));

		AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TANGONET_PARTNERID), m_TangoPayConfig.strPartnerID);
		
		if (m_TangoPayConfig.strApiKey.GetLength() > 0)
			AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TANGONET_APIKEY), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TANGONET_EXIST));
		else
			AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TANGONET_APIKEY), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TANGONET_NOT_EXIST));

		if (m_TangoPayConfig.strPublicKey.GetLength() > 0)
			AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TANGONET_PUBLICKEY), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TANGONET_EXIST));
		else
			AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TANGONET_PUBLICKEY), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TANGONET_NOT_EXIST));
	}

#endif
// [RWC6-676] End SKKim 2024.05.20

}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: LIB_GetTransactionSetupPrintData()
RETURN TYPE  :
PARAMETER    :
DESCRIPTION  : '4. TRANSACTION SETUP'
-------------------------------------------------------------------*/
void CTranCmn::LIB_GetTransactionSetupPrintData(CString& strPrintData)
{
	CString strTemp, strTemp2;

	// DISPENSE LIMIT
	AddPrintData(strPrintData, L"%-25.25s : %s%s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DISPENSE_LIMIT),
		GetCurrencySymbol(), MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT)));

	// CURRENCY ID
	AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CURRENCY_ID),
		m_pDevCmn->m_strCurrencyID);

	// [#2434] AU Kook 2016.07.26 Receipt Optimizing with refactoring
	// 1234567890123456789012345678901234567890
	// DENOMINATION ($)
	//   CST1    CST2    CST3    CST4
	//	   20      50     100     N/A
	CString strDenomCst[5] = { _T("N/A"), _T("N/A"), _T("N/A"), _T("N/A"), _T("N/A") };
	for (int nCstIdx = 1; nCstIdx <= m_pDevCmn->fnCDU_GetNumberOfCST(); nCstIdx++)	// skip 0 which is REJECT CST
	{
		strDenomCst[nCstIdx].Format(_T("%4d"), m_pDevCmn->fnCDU_GetValueOfCash(nCstIdx));
	}

	AddPrintData(strPrintData, L"%s (%s)",
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DENOM), GetCurrencySymbol());
	AddPrintData(strPrintData, L"  CST1    CST2    CST3    CST4");
	AddPrintData(strPrintData, L"  %4s    %4s    %4s    %4s    %4s",
		strDenomCst[1], strDenomCst[2], strDenomCst[3], strDenomCst[4]);
	// end of [#2434]

	// FAST CASH
	strTemp2.Empty();	// [#2220] AU KMK 2014.01.17 strTemp2 초기화 (위에서 사용한 값이 남아 있을수도 있으므로)
	AddPrintData(strPrintData, L"%s (%s)", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_FASTCASH), GetCurrencySymbol());
	for (int i = 0; i < CDU_CST_MAXFASTCASH; i++)
	{
		if (strTemp2.IsEmpty())
			strTemp.Format(_T(" : %d"), MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH1 + i));
		else
			strTemp.Format(_T(", %d"), MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH1 + i));
		strTemp2 += strTemp;
	}
	AddPrintData(strPrintData, L"%s", strTemp2);

	// LOW CURRENCY CHECK
	if (m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LOW_CURRENCY_CHK).GetLength() > 25) {
		AddPrintData(strPrintData, L"%-35.35s :", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LOW_CURRENCY_CHK));
		AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_LOWCRCYCHECK))));
	}
	else {
		AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_LOW_CURRENCY_CHK),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_LOWCRCYCHECK))));
	}

	// DENOMINATION SELECT
	// [#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA
	if (m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DENOMINATION_SELECT).GetLength() > 25) {
		AddPrintData(strPrintData, L"%-35.35s :", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DENOMINATION_SELECT));
		AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DENOMINATION_SELECT_ENABLE))));
	}
	else {
		AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DENOMINATION_SELECT),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DENOMINATION_SELECT_ENABLE))));
	}
	// end of [#RWC6-12, #2584]

	// [#2472] US Justin 2017.02.09
	// DCC Withdrawal Option
#if(APP_DCC_WITHOPTION)
	AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DCC_WITH_OPTION),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_DCC_WITHOPTION_APTEXTID(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_OPTION_ELIGIBLE))));

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_OPTION_ELIGIBLE) != DCC_WITHDRAWAL_SETTING_NONE)
	{
		// DCC Dispense Limit
		AddPrintData(strPrintData, L"  DCC %-19.19s : %s%d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DISPENSE_LIMIT),
			GetCurrencySymbol(), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_OPTION_MAXWITHDRAWAL));

		// DCC Fast Cash
		strTemp2.Empty();
		AddPrintData(strPrintData, L"  DCC %s (%s)", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_FASTCASH), GetCurrencySymbol());
		for (int i = 0; i < CDU_CST_MAXFASTCASH; i++)
		{
			if (strTemp2.IsEmpty())		strTemp.Format(_T("     : %d"), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_OPTION_FASTCASH1 + i));
			else						strTemp.Format(_T(", %d"), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_OPTION_FASTCASH1 + i));
			strTemp2 += strTemp;
		}
		AddPrintData(strPrintData, L"%s", strTemp2);
	}
#endif
	// End of [#2472]
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: LIB_GetHostSetupPrintData()
RETURN TYPE  :
PARAMETER    :
DESCRIPTION  : '5. HOST SETUP'
-------------------------------------------------------------------*/
void CTranCmn::LIB_GetHostSetupPrintData(CString& strPrintData)
{
	CString strTemp;

	// TERMINAL #
	AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TML),
		MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID));

	// STANDARD3 OPTION
	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
	{
		AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringByTextID(_T("STANDARD3 OPTION")));

		// STATUS MONITORING
		AddPrintData(strPrintData, L"  %-23.23s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_STA_MON),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_STATUSMONITORINGENABLE))));

		// COMMUNICATION HEADER
		AddPrintData(strPrintData, L"  %-23.23s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_COMM_HEADER),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE))));

		// COMMUNICATION ID
		AddPrintData(strPrintData, L"  %-23.23s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_COMM_ID),
			MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID));

		// CRC
		AddPrintData(strPrintData, L"  %-23.23s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CRC),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_CRCENABLE))));
	}
	else		// in case of STANDARD1, STANDARD2
	{
		// ROUTING ID
		AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ROUTING_ID),
			MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));
	}

	// STANDARD1 OPTION
	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
	{
		AddPrintData(strPrintData, L"%-40.40s", m_pDevCmn->fstrSCR_GetStringByTextID(_T("STANDARD1 OPTION")));

		// TERMINAL STATUS
		AddPrintData(strPrintData, L"  %-23.23s : %-12.12s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TML_STA),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TERMINAL_STATUS_FIELD_ENABLE))));

		// REVERSAL AT HOST ERROR
		AddPrintData(strPrintData, L"  %-23.23s : %-12.12s",
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_REV_AT_HOST_ERR),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REVERSAL_AT_HOSTERROR))));	// KSK V06.00.20 내부검사 지적사항 대응

	// REASON FOR REVERSAL
		AddPrintData(strPrintData, L"  %-23.23s : %-12.12s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RSN_FOR_REV),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REASONFORREVERSAL_ENABLE))));

		// [#2084] NH KSK 2011.07.15 EJUPLOAD 추가
		AddPrintData(strPrintData, L"  %-23.23s : %-12.12s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_EJ_UL),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EJUPLOAD_ENABLE))));
		// end of [#2084]
	}

	// AUTO DAY TOTAL
	AddPrintData(strPrintData, L"%-22.22s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_AUTO_DAYTOTAL),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_ENABLE))));

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_ENABLE))
	{
		// [#2434] AU Kook 2016.07.26 Receipt Optimizing with refactoring
		// DAY TOTAL TYPE
		CString strAutoDayTotalType = _T("");
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TYPE) == AUTO_DAY_TOTAL)
			strAutoDayTotalType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DAYTOTAL);
		else
			strAutoDayTotalType = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TRIALDAYTOTAL);
		if (strAutoDayTotalType.GetLength() < 16)		// alignment
			strAutoDayTotalType = " " + strAutoDayTotalType;
		AddPrintData(strPrintData, L"  %-20.20s :%-16.16s",
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_AUTO_DAYTOTAL_TYPE), strAutoDayTotalType);

		// DAY TOTAL TIME
		AddPrintData(strPrintData, L"  %-20.20s : %02d:%02d",
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_AUTO_DAYTOTAL_TIME),
			MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_HOUR),
			MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_MIN));
		// end of [#2434]
	}

	// HOST INFORMATION
	AddPrintData(strPrintData, _T("%-40.40s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"HOST INFORMATION"));

	// DIALUP
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP)
	{
		// HOST 1ST PHONE #
		AddPrintData(strPrintData, L"  %-20.20s : %-15.15s",
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_1ST_PHONE),
			MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1));

		// HOST 2ND PHONE #
		AddPrintData(strPrintData, L"  %-20.20s : %-15.15s",
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_2ND_PHONE),
			MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE2));
	}
	else	// TCPIP
	{
		// 1234567890123456789012345678901234567890
		//   USE URL              : ENABLE         
		//   HOST[1]  ADDRESS     :
		// www.abcdefghijklmnopqrstuvwxyz123456.com
		//   HOST[1]  PORT        : 12345
		//   HOST[2]  ADDRESS     : 123.123.123.123
		//   HOST[2]  PORT        : 34567
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ISHOSTUSEURL) == ENABLE)
		{
			AddPrintData(strPrintData, _T("  %-20.20s : %-15.15s"),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_USE_URL),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_ENABLE_RCPT));
		}
		else
		{
			AddPrintData(strPrintData, _T("  %-20.20s : %-15.15s"),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_USE_URL),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_DISABLE_RCPT));
		}

		// HOST[1] ADDRESS
		if (MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1NAME).GetLength() > 15)
		{
			AddPrintData(strPrintData, _T("  %-8.8s %-11.11s :%c%-40.40s"),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_1),
				m_pDevCmn->fstrSCR_GetStringByTextID(_T("ADDRESS")), FIELD_DELIMITER,
				MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1NAME));
		}
		else
		{
			AddPrintData(strPrintData, _T("  %-8.8s %-11.11s : %-15.15s"),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_1),
				m_pDevCmn->fstrSCR_GetStringByTextID(_T("ADDRESS")),
				MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1NAME));
		}

		// HOST[1] PORT NO
		AddPrintData(strPrintData, _T("  %-8.8s %-11.11s : %d"),
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_1), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PORTNO),
			MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1PORTNO));

		// HOST[2] ADDRESS
		if (MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2NAME).GetLength() > 15)
		{
			AddPrintData(strPrintData, _T("  %-8.8s %-11.11s : %c%-40.40s"),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_2),
				m_pDevCmn->fstrSCR_GetStringByTextID(_T("ADDRESS")), FIELD_DELIMITER,
				MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2NAME));
		}
		else
		{
			AddPrintData(strPrintData, _T("  %-8.8s %-11.11s : %-15.15s"),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_2),
				m_pDevCmn->fstrSCR_GetStringByTextID(_T("ADDRESS")),
				MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2NAME));
		}

		// HOST[2] PORT NO
		AddPrintData(strPrintData, _T("  %-8.8s %-11.11s : %d"),
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOST_2), m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PORTNO),
			MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2PORTNO));
		// end of [#2326]
	}

	// 1234567890123456789012345678901234567890
	// HEALTH CHECK MESSAGE   : ENABLE
	//   INTERVAL             : 2 HOUR
	// HEALTH CHECK MESSAGE
	AddPrintData(strPrintData, L"%-22.22s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HEALTH_CHK_MSG),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_HEALTHYFLAG))));	// KSK V06.00.20 내부검사 지적사항 대응

	// INTERVAL
	AddPrintData(strPrintData, L"  %-20.20s : %d %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HEALTH_CHK_INTERVAL),
		MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_HSDELAYINT),
		m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOUR));

	// CONFIG DOWNLOAD AT START
	AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_CONF_DL_ATSTART),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_REVERSE_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CONFIGURATION_ENABLE))));

	// RMS (remote monitoring system)		// [#2326] US Kook 2015.10.15 Support MoniAir, 명세표 절약
	// 1234567890123456789012345678901234567890
	// RMS CONNECTION         : ENABLE
	//   RING COUNT           : 1					(DIAL-UP)
	//   ATM LISTENING PORT   : 5555				(TCP/IP)

	//   RMS STATUS SEND      : ENABLE
	//     SEND INTERVAL      : 4 HOUR				(TCP/IP)

	//     1ST PHONE #   : 12345678901234567890		(DIAL-UP)
	//     1E TELEPHONE #: 12345678901234567890		(DIAL-UP)
	//     2ND PHONE #   : 12345678901234567890		(DIAL-UP)

	//     RMS USE URL        : ENABLE				(TCP/IP)
	//     RMS ADDRESS        : 123.123.123.123		(TCP/IP)
	// www.abcdefghijklmnopqrstuvwxyz123456.com		(TCP/IP)
	//     RMS PORT           : 9999				(TCP/IP)
	//     RMS TLS            : ENABLE              (TCP/IP)

	//   CONNECT TIMEOUT(TCPIP) : 30 sec			// AU Only
	//   RECEIVE TIMEOUT(TCPIP) : 30 sec			// AU Only

	// RMS CONNECTION
	AddPrintData(strPrintData, L"%-23.23s: %-15.15s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RMS_CONN),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSFLAG))));

	// 2016.08.09 show RMS things only if RMS is enabled
	if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSFLAG))
	{
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP)		// Dial-Up
		{
			// RING COUNT
			AddPrintData(strPrintData, L"  %-21.21s: %d",
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RMS_RING_CNT),
				MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSRINGCNT));
		}
		else
		{
			// ATM LISTENING PORT
			AddPrintData(strPrintData, L"  %-21.21s: %d",
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_ATM_LISTEN_PORTNO),
				MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSSVRPORTNO));
		}

		// RMS STATUS SEND
		AddPrintData(strPrintData, L"  %-21.21s: %-15.15s",
			m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RMS_STATUS_SEND),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSSTATUSSEND))));

		// show RMS STATUS SEND things only if STATUS SEND is enabled
		if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSSTATUSSEND))
		{
			// SEND INTERVAL
			AddPrintData(strPrintData, L"    %-19.19s: %d %s",
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RMS_SEND_INTERVAL),
				MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSSENDINTERVAL),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_HOUR));

			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP)
			{
				// 1ST PHONE # : 
				AddPrintData(strPrintData, L"    %-13.13s: %s",
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RMS_1ST_PHONE),
					MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPHONENO1));

				// 2ND PHONE # : 
				AddPrintData(strPrintData, L"    %-13.13s: %s",
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RMS_2ND_PHONE),
					MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPHONENO2));
			}
			else	// TCP/IP
			{
				// [#2559] NH Justin 2018.06.19 Retry Status Send if "send" failed
				// COMM ERROR Retrial
#if( US_VERSION || CA_VERSION || MX_VERSION )
				AddPrintData(strPrintData, L"    %-19.19s: %d %s",
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RMS_SEND_RETRY),
					MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSSEND_RETRY),
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MINUTES));
#endif
				// End of [#2559]

				// RMS USE URL
				AddPrintData(strPrintData, L"    %-19.19s: %-15.15s",
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RMS_USE_URL),
					m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
						MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ISRMSUSEURL))));

				// RMS ADDRESS
				if (MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSNAME).GetLength() > 15)
				{
					AddPrintData(strPrintData, L"    %-19.19s:%c%-40.40s",
						m_pDevCmn->fstrSCR_GetStringByTextID(L"RMS ADDRESS"),
						FIELD_DELIMITER,
						MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSNAME));
				}
				else
				{
					AddPrintData(strPrintData, L"    %-19.19s: %-15.15s",
						m_pDevCmn->fstrSCR_GetStringByTextID(L"RMS ADDRESS"),
						MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSNAME));
				}

				bool tlsEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSTLSENABLE) == ENABLE;
				int rmsPort = tlsEnable ? MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSTLSPORT)
					: MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSPORTNO);

				// RMS PORT
				AddPrintData(strPrintData, L"    %-19.19s: %d",
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RMS_LISTEN_PORTNO), rmsPort);

				// RMS TLS
				AddPrintData(strPrintData, L"    %-19.19s: %-15.15s",
					m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_PRT_RMS_TLS),
					m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(tlsEnable)));

			}
		}

		// [#2234] NH KMK 2014.01.16 MoniView Timeout 명세표 출력 (RMS On & TCP/IP인 경우)
#if (AU_VERSION)
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_TCPIP)
		{
			// CONNECT TIMEOUT(TCP/IP)
			AddPrintData(strPrintData, L"  CONNECT TIMEOUT(TCPIP)  : %d %s",
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_CONNECT_TCPIP),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SEC));

			// RECEIVE TIMEOUT(TCP/IP)
			AddPrintData(strPrintData, L"  RECEIVE TIMEOUT(TCPIP)  : %d %s",
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_RECEIVE_TCPIP),
				m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SEC));
		}
#endif
		// end of [#2234]
	}

	// [#2037] AU KSK 2011.03.31	
	// [#2457] US Justin 2016.12.09 Support Moniview Journal Uploading	
	// [#2500] CA Justin 2017.08.22 Support Moniview Journal Uploading
#if (AU_VERSION  || US_VERSION  || CA_VERSION)	
	// SCHEDULED JOURNAL UPLOAD
	AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SCHED_JNL_UL),
		m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
			MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_ENABLE))));

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_ENABLE))
	{
		// [#2434] AU Kook 2016.07.26 Receipt Optimizing with refactoring
		AddPrintData(strPrintData, L"  %-23.23s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_JNL_UL_TYPE),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_SCH_JOURNAL_OPTION_APTEXTID(
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_OPTION))));

		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_OPTION) == 0)
		{
			AddPrintData(strPrintData, L"  %-23.23s : %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_JNL_UL_CNT),
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_COUNT));
		}
		else if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_OPTION) == 1)
		{
			AddPrintData(strPrintData, L"  %-23.23s : %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_JNL_UL_DAY),
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_DAY));
			AddPrintData(strPrintData, L"  %-23.23s : %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_JNL_UL_HOUR),
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_HOUR));
		}
		else
		{
			AddPrintData(strPrintData, L"  %-23.23s : %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_JNL_UL_HOUR),
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_HOUR));
		}
		// end of [#2434]
	}
#endif

	// [#2084] NH KSK 2011.07.15 RKT 추가
	// 1234567890123456789012345678901234567890
	// RKT                       : ENABLE
	//   RANDOM NUMBER           : ENABLE
	if ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_AVAILABLE) == ENABLE)
		&& (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE))
	{
		// RKT
		AddPrintData(strPrintData, L"%-25.25s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RKT),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_ENABLE))));

		// RANDOM NUMBER
		AddPrintData(strPrintData, L"  %-23.23s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_RKT_RAND_NUM),
			m_pDevCmn->fstrSCR_GetStringFromTextID(GET_ENDIS_APTEXTID_RCPT(
				MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_RANDOMNUMBER_ENABLE))));
	}
	// end of [#2084]

	// [#2434] AU Kook 2016.07.26 Receipt Optimizing with refactoring
	// KEY INFORMATION		// 2016.08.09
	AddPrintData(strPrintData, _T("%-40.40s"), m_pDevCmn->fstrSCR_GetStringByTextID(L"KEY INFORMATION"));

	CString strCurKeyMode = _T("");
	switch (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE))
	{
	case KEYMODE_NON_UNIQ_SDES:
		strCurKeyMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_SNGL_KEY_DES);	break;
	case KEYMODE_NON_UNIQ_DDES:
		strCurKeyMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DUAL_MK_KEY);		break;
	case KEYMODE_UNIQ_SDES:
		strCurKeyMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_UNIQ_KEY);
		strCurKeyMode += ", ";
		strCurKeyMode += m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DES);			break;
	case KEYMODE_NON_UNIQ_TDES:
		strCurKeyMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TDES);			break;
	case KEYMODE_UNIQ_TDES:
		strCurKeyMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_UNIQ_KEY);
		strCurKeyMode += ", ";
		strCurKeyMode += m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TDES);			break;
	case KEYMODE_NON_UNIQ_SDES_MACING:
		strCurKeyMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_DES);
		strCurKeyMode += ", ";
		strCurKeyMode += m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MAC);			break;
	case KEYMODE_UNIQ_SDES_MACING:
		strCurKeyMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_UNIQ_KEY);
		strCurKeyMode += ", ";
		strCurKeyMode += m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MAC);			break;
	case KEYMODE_TDES_MACING:
		strCurKeyMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TDES);
		strCurKeyMode += ", ";
		strCurKeyMode += m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MAC);			break;
	case KEYMODE_TDES_TMACING:
		strCurKeyMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TDES);
		strCurKeyMode += ", ";
		strCurKeyMode += m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_TMAC);			break;
	default:
		strCurKeyMode = m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_UNKNOWN);			break;
	}

	// KEY MODE
	AddPrintData(strPrintData, L"  %-14.14s : %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_KEYMODE), strCurKeyMode);
	// end of [#2434]

	if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) >= 0)
	{
		BOOL bUsePinKey = FALSE, bUseMacKey = FALSE, bUseTriplePinKey = FALSE, bUseTripleMacKey = FALSE;

		if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES
			|| MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_DDES
			|| MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING)
		{
			bUsePinKey = TRUE;
		}
		else if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_TDES
			|| MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING
			|| MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)
		{
			bUsePinKey = TRUE;
			bUseTriplePinKey = TRUE;
		}

		if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING
			|| MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING
			|| MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING)
		{
			bUseMacKey = TRUE;
		}
		else if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)
		{
#if !(AU_VERSION)	// [#2069] NH KSK 2011.06.13
			bUseMacKey = TRUE;
			bUseTripleMacKey = TRUE;
#endif
		}

		if (bUsePinKey)
		{
			if (bUseTriplePinKey)
			{
				// [#2434] AU Kook 2016.07.26 Receipt Optimizing with refactoring
				// 1234567890123456789012345678901234567890
				//   MASTER KEY CHECKSUM(MASTERK3)  : ## ##
				AddPrintData(strPrintData, _T("  %-30.30s: %s"),
					m_pDevCmn->fstrSCR_GetStringByTextID(_T("MASTER KEY CHECKSUM")) + L"(" + TRIPLEDESKEY_NAME + L")",
					MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0));
				// end of [#2434]
			}
			else
			{
				// [#2434] AU Kook 2016.07.26 Receipt Optimizing with refactoring
				// 1234567890123456789012345678901234567890
				//   MASTER KEY CHECKSUM(MASTERK)   : ## ##
				AddPrintData(strPrintData, _T("  %-30.30s: %s"),
					m_pDevCmn->fstrSCR_GetStringByTextID(_T("MASTER KEY CHECKSUM")) + L"(" + SINGLEDESKEY_NAME + L")",
					MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0));
				// end of [#2434]
			}

		}
		if (bUseMacKey)
		{
			if (bUseTripleMacKey)
			{
				// [#2434] AU Kook 2016.07.26 Receipt Optimizing with refactoring
				// 1234567890123456789012345678901234567890
				//   MAC KEY CHECKSUM(MACK3)        : ## ##
				AddPrintData(strPrintData, _T("  %-30.30s: %s"),
					m_pDevCmn->fstrSCR_GetStringByTextID(_T("MAC KEY CHECKSUM")) + L"(" + TRIPLEMACKEY_NAME + L")",
					MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM15));
				// end of [#2434]
			}
			else
			{
				// [#2434] AU Kook 2016.07.26 Receipt Optimizing with refactoring
				// 1234567890123456789012345678901234567890
				//   MAC KEY CHECKSUM(MACK)         : ## ##
				AddPrintData(strPrintData, _T("  %-30.30s: %s"),
					m_pDevCmn->fstrSCR_GetStringByTextID(_T("MAC KEY CHECKSUM")) + L"(" + SINGLEMACKEY_NAME + L")",
					MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM15));
				// end of [#2434]
			}
		}

		// [#2319] US Justin 2014
		strTemp = MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM1);
		if (strTemp.GetLength() >= 4)
			AddPrintData(strPrintData, L"  %-28.28s : %s", TRIPLEDESKEY_2ND_PIN_NAME, strTemp);
		// End of [#2319]

#if(APP_LIBERTYX)
		strTemp = MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM2);
		if (strTemp.GetLength() >= 4)
			AddPrintData(strPrintData, L"  %-28.28s : %s", LIBERTYXKEY_NAME, strTemp);
#endif
	}
	else
	{
		// [#2434] AU Kook 2016.07.26 Receipt Optimizing with refactoring
		// 1234567890123456789012345678901234567890
		//   MASTER KEY INDEX              : ##
		//   KEY CHECKSUM
		//     MASTER KEY      (#01)       : ## ##
		//     CLE PRINCIPALE  (#02)       : ## ##
		// MASTER KEY INDEX
		AddPrintData(strPrintData, L"  %-30.30s : %d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_RECE_MK_INDEX),
			MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX));

		// KEY CHECKSUM
		AddPrintData(strPrintData, L"  %-38.38s", m_pDevCmn->fstrSCR_GetStringByTextID(_T("KEY CHECKSUM")));

		for (int i = 0; i < 16; i++)
		{
			// MASTER KEY #(01) : (value)
			AddPrintData(strPrintData, L"    %-15.15s (#%02d)       : %s",
				m_pDevCmn->fstrSCR_GetStringByTextID(_T("MASTER KEY")), (i + 1),
				MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0 + i));
		}
		// end of [#2434]
	}
}

// end of [#2324]


#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: LIB_ALLBINPrint()
 RETURN TYPE  :
 PARAMETER    :
 DESCRIPTION  :
-------------------------------------------------------------------*/
BOOL CTranCmn::LIB_ALLBINPrint()
{
	CString		strPrintData;
	CString		strBinData;
	CStringArray strTempArray;

	CString		strTerminal = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);


	WORD wPage = m_pDevCmn->m_BINMgr.GetTotalPage();

	for (int i = 0; i <= wPage; i++)
	{
		if (i == 0)
			strBinData = m_pDevCmn->m_BINMgr.MakeHeaderPrintData(strTerminal);
		else
			strBinData = m_pDevCmn->m_BINMgr.MakeBinPrintData(i);

		SplitString(strBinData, "||", strTempArray);
		for (int nline = 0; nline < strTempArray.GetSize(); nline++)
		{

			if (strTempArray[nline].GetLength() > 0)
			{
				AddPrintData(strPrintData, L"%s", strTempArray[nline]);
			}

		}
	}
	return m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT);

}
#endif
