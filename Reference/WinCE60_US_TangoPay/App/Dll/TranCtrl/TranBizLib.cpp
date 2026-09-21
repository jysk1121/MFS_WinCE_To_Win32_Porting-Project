#include "stdafx.h"
#include ".\Tran\TranCmn.h"

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
//	Implement
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: LIB_CheckCardRead()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : CARD 정보를 추출한다.
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::LIB_CheckCardRead(CString strCardData)
{
	NHDEBUG(DBG_CALL, (_T("[CTranCmn::LIB_CheckCardRead] strCardData(%d)\n"), strCardData.GetLength()));

	NHDEBUG(DBG_INFO, (L"BEFORE CARD READ\n"));
	NHDEBUG(DBG_INFO, (L"CARD DATA : [%s]\n", strCardData));
	NHDEBUG(DBG_INFO, (L"BANK CODE : [%s]\n", m_sCardData.strBankID));
	NHDEBUG(DBG_INFO, (L"ACCOUNT   : [%s]\n", m_sCardData.strAccountNo));
	NHDEBUG(DBG_INFO, (L"SHOW NUMB : [%s]\n", m_sCardData.strShowNumber));
	NHDEBUG(DBG_INFO, (L"ISO2 DATA : [%s]\n", m_sCardData.strISO2Data));

	CString strJNLCardData;		// [#2200] AU KMK 2013.06.11

	if (strCardData.GetLength() > 0)
	{
		int	nSearchIndex;

		CString strPackCardData;

		// [#2250] US Justin 2014.01.27 Parse Track1 and Track2 data
		//MakePack(strCardData, strPackCardData, strCardData.GetLength());
		int nLocComma = strCardData.Find(',');
		if(nLocComma<0)
		{
			// Track2 Only...
			MakePack(strCardData, strPackCardData, strCardData.GetLength());
		}
		else
		{
			// Track1 Data
			CString sTrack1Data = strCardData.Left(nLocComma);
			CString strPackedTrack1;
			MakePack(sTrack1Data, strPackedTrack1, sTrack1Data.GetLength());
			m_sCardData.strISO1Data = strPackedTrack1;

			// Track2 Data.
			CString sTrack2Data = strCardData.Mid(nLocComma+1);
			MakePack(sTrack2Data, strPackCardData, sTrack2Data.GetLength());
		}
		// End of [#2250]

		nSearchIndex = strPackCardData.Find('=');
		if (nSearchIndex >= 13)
		{
			// Save ISO2
			m_sCardData.strISO2Data = strPackCardData;
			// Card PAN for EPP
			m_sCardData.strAccountNo = strPackCardData.Mid(nSearchIndex - 13, 12);
			// Bank ID
			m_sCardData.strBankID  = strPackCardData.Mid(nSearchIndex - 4, 4);
			// [#2200] AU KMK 2013.06.11 PAN Data 보관
			strJNLCardData = strPackCardData.Mid(0, nSearchIndex);

		}
		else if (nSearchIndex != -1)
		{
			// Save ISO2
			m_sCardData.strISO2Data = strPackCardData;
			// Card PAN for EPP
			m_sCardData.strAccountNo = L"0";
			m_sCardData.strAccountNo += strPackCardData.Mid(1, 12);
			// Bank ID
			m_sCardData.strBankID  = strPackCardData.Mid(13, 4);
		}

		// strShowNumber for Receipt
		if (m_sCardData.strAccountNo.GetLength() > 0)
			m_sCardData.strShowNumber.Format(L"************%s", m_sCardData.strBankID);		// KSK 2010.08.25 19자리일 경우 Bug Fix

		NHDEBUG(DBG_INFO, (L"Track1 : [%s]\n", m_sCardData.strISO1Data));
		NHDEBUG(DBG_INFO, (L"Track2 : [%s]\n", m_sCardData.strISO2Data));
	}

	// Check ISO 2 Track
	if (m_sCardData.strISO2Data.GetLength() <= 0)
	{
		NHDEBUG(DBG_INFO, (L"DON'T READ ISO2 TRACK Data\n"));
		return RES_CARD_ERROR;
	}

	// Check the mandatory information.
	if (m_sCardData.strAccountNo.GetLength() <= 0)
	{
		NHDEBUG(DBG_INFO, (L"DON'T READ ACCOUNT INFO.\n"));
		return RES_CARD_ERROR;
	}

	//////////////////////////////////////////////////////////////////////////
	// 4. Check Validation

	// Check MOD10
	if(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MOD10_ENABLE) == ENABLE)
	{
		int index;
		char tmpData[128];		// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
		CString strIDCTrack2;

		strIDCTrack2 = m_sCardData.strISO2Data;
		index = strIDCTrack2.Find(L"=");

		memset(tmpData, 0x00, sizeof(tmpData));
		WideToMulti(tmpData, strIDCTrack2.Left(index), __min(index*2, 128));	// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책

		if(IsValidMod10Data(tmpData, index) == 0)
		{
			NHDEBUG(DBG_INFO, (L"MOD 10 ERROR\n"));
			return RES_CARD_MOD_10;
		}
	}

#if (AU_VERSION) //[#2000] SOOK 2010.10.13 호주 사양 적용 (거래 예외 처리)	// [#2069] NH KSK 2011.06.13
	WORD nBinCount = m_pDevCmn->m_BINMgr.MakeTransactionBinList(m_sCardData.strISO2Data);
		//DECLINE 카드 일 경우 거래 중단함  
		//ALLOW ONLY LISTED BIN ENABLE일 경우 BIN 이  없거나 
	DWORD dwValue = 0;

	if (  m_pDevCmn->m_BINMgr.GetTransactionBinProperties(ACTION_DECLINE_CARD, &dwValue)== TRUE
		|| ( m_pDevCmn->m_BINMgr.GetAllowOnlyBinEnable() == 1  && nBinCount == 0))
	{
		int nReason = 0; //Decline Card로 등록된 경우 
		if ( m_pDevCmn->m_BINMgr.GetAllowOnlyBinEnable() == 1 && nBinCount == 0)
		{
			nReason = 1; //BIN이 등록이 안 된경우 
		}

		CString strReason;
		strReason.Format(L"RESTRICT BIN(%6.6s), REASON=%s", m_sCardData.strISO2Data.Left(6),
			(nReason == 0 ? L"DECLINE CARD": L"ALLOWED ONLY LISTED BIN"));  //[#568] SOOK 2009.09.30 REASON 설명 추가 
		m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_READ_CARD, strReason);
		
		return RES_CARD_BIN_NG;
	}
#endif //end of [#2000]

	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKCODE,	m_sCardData.strBankID);
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKACCOUNT,	m_sCardData.strAccountNo);
#if (AU_VERSION)	// [#2042] AU KSK 2011.04.03
	// [#2200] AU KMK 2013.06.11 PAN Data 수정
	// TRANSCARDDATA 에는 Track2 데이터 중 FS('=') 직전까지의 모든 문자열을 저장한다 (최대 19자)
// 	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA,	m_sCardData.strAccountNo + m_sCardData.strBankID);
 	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA,	strJNLCardData);
	// end of [#2200]

#else
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA,	m_sCardData.strShowNumber);
#endif
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2,		m_sCardData.strISO2Data);

	NHDEBUG(DBG_INFO, (L"AFTER CARD READ\n"));
	NHDEBUG(DBG_INFO, (L"BANK CODE : [%s]\n", m_sCardData.strBankID));
	NHDEBUG(DBG_INFO, (L"ACCOUNT   : [%s]\n", m_sCardData.strAccountNo));
	NHDEBUG(DBG_INFO, (L"SHOW NUMB : [%s]\n", m_sCardData.strShowNumber));
	NHDEBUG(DBG_INFO, (L"ISO2 DATA : [%s]\n", m_sCardData.strISO2Data));

	return RES_OK;
}

// [#2517] US Justin 2017.11.29 EMV Fallback Enable/Diable Option
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: LIB_IsICCard()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Check Track2 Service code (2 or 6 means IC Card)
-------------------------------------------------------------------*/
BOOL CTranCmn::LIB_IsICCard(CString strTrack2)
{
	int nResult = strTrack2.Find(L"=");

	if (strTrack2.GetLength() <= nResult + 5)
		return FALSE;

	if( (nResult != -1) && (strTrack2.GetAt(nResult+5)=='2' || strTrack2.GetAt(nResult+5)=='6') )
		return TRUE;

	return FALSE;
}
// End of [#2517]

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: LIB_CheckSurchargeDisplay()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 수수료 화면 표시 여부를 결정한다.
-------------------------------------------------------------------*/
BOOL CTranCmn::LIB_CheckSurchargeDisplay()
{
	NHDEBUG(DBG_CALL, (_T("***TranBizLib***[CTranCmn::LIB_CheckSurchargeDisplay]\n")));

	CString		strISO2Buff;
	CString		strBINList;
	CString		strIDCBin;

	// 1. Surcharge Flag가 Off 이면 Display 없음.
	if (!MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE))
		return FALSE;

	// 2. BIN List Check 해당 고객 카드 정보가 일치하면 Surcharge 없음.
	int nCount = MemGetInt(_MEM_FLD_APP_BININFO, _MEM_VAR_APP_SETBINLIST);
	strISO2Buff = m_sCardData.strISO2Data;

	for (int i=0; i<nCount; i++)
	{
		strBINList = MemGetStr(_MEM_FLD_APP_BININFO, _MEM_VAR_APP_BINLIST+i);
		strBINList.Replace(L" ",L"");

		if (strISO2Buff.GetLength() >= strBINList.GetLength())
			strIDCBin = strISO2Buff.Left(strBINList.GetLength());

		if ((!strBINList.CompareNoCase(strIDCBin)) && (strBINList.GetLength() != 0))
			return FALSE;
	}

	// 3. Extended Bin List Check 해당 고객 카드 정보가 일치하면 Surcharge 없음.
	if (m_pDevCmn->IsBinExist(strISO2Buff, m_pDevCmn->m_arrExtendedBinList) == TRUE)
		return FALSE;

	return TRUE;
}
//#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
//[#2000] 2010.10.17 SOOK 호주 사양 적용 (거래 FLOW )
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: LIB_CheckSurchargeDisplayWithMultipleBin()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 수수료 화면 표시 여부를 결정한다.(MultipleBin)
-------------------------------------------------------------------*/
BOOL CTranCmn::LIB_CheckSurchargeDisplayWithMultipleBin()
{

#if (AU_VERSION)	// KSK 2012.08.22 타국가 컴파일 오류 수정
	SurchargeAmount = 0;
	DWORD dwValue = 0;
	
	//Surcharge MOde Disable을 경우에는 BIN LIst에 Propertiesr 가 있어도 무시함 
	if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE) == 0)
	{
		return FALSE;
	}
	else if ( m_pDevCmn->m_BINMgr.GetTransactionBinProperties(ACTION_BLOCK_SURCHARGE, &dwValue) == TRUE)
	{
		SurchargeAmount = 0;
	}
	else if ( TranCode == TC_WITHDRAWAL 
		&& m_pDevCmn->m_BINMgr.GetTransactionBinProperties(ACTION_WITHDRAWAL_SURCHARGE, &dwValue) == TRUE)
	{
		SurchargeAmount = dwValue;
	}
	else if ( TranCode == TC_INQUIRY 
		&& m_pDevCmn->m_BINMgr.GetTransactionBinProperties(ACTION_BALANCE_SURCHARGE, &dwValue) == TRUE)
	{
		SurchargeAmount = dwValue;
	}
	else if ( MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE)== 1 )
	{
		//if ( TranCode == TC_WITHDRAWAL )
		if ( (TranCode == TC_WITHDRAWAL) &&  (Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT)) > 0) )	// OP에 설정된 수수료 금액이 0보다 큰경우에만
		{
			// [#2220] AU KMK 2014.01.28 호주 Percent Surcharge
			// 타국가는 DisplaySurcharge() 함수에서 수수료를 계산하지만 호주는 이 함수에서 수행함
			switch (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE))
			{
			case PERCENT_MODE:
				{
					float fPercentSurcharge = (float)(Asc2Int(m_sUserSelection.strMoney) * 0.01) * (float)(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE) * 0.0001);	// [#2316] US Justin 2014.12.17 Support Decimal Percentage surcharge

					float fResultSurcharge = 0;
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_MANNER) == GREATER)
						fResultSurcharge = max(fPercentSurcharge, (float)(Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT))*0.01));
					else
						fResultSurcharge = min(fPercentSurcharge, (float)(Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT))*0.01));
					SurchargeAmount = (int)(fResultSurcharge*100 + 0.001);
					break;
				}
			case NOT_USED:
			default:
				{
					SurchargeAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT));
					break;
				}
			}
			// end of [#2220]
		}
		else if (TranCode == TC_INQUIRY )
		{
			SurchargeAmount = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_INQUIRY_SURCHARGE);
		}
	}
	
	if ( SurchargeAmount != 0 ) return TRUE;
#endif			// end of KSK 2012.08.22

	return FALSE;

}
//#endif
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: LIB_IsReceiptPrintCondition()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 명세표 출력 조건을 설정한다 (호주 특이 사양 )
-------------------------------------------------------------------*/
//BOOL CTranCmn::LIB_IsReceiptPrintCondition4AU()
BOOL CTranCmn::LIB_IsReceiptPrintCondition()			//[#2183] NH Justin 2013.04.03 Change Function Name
{
	
	NHDEBUG(DBG_CALL, (_T("***TranBizLib***[CTranCmn::LIB_IsReceiptPrintCondition]\n")));
	// [#2219] 2013.09.04 Justin Digital Receipt
	//BOOL bCondition = m_sUserSelection.bPrintReceipt;			
	BOOL bCondition = TRUE;

	// [#2235] 2013.11.30 KMK Digital Receipt 사용 시에도 명세표 출력하지 않아야 함
	//if( m_sUserSelection.nPrintReceipt == FALSE)
	if( (m_sUserSelection.nPrintReceipt == FALSE) || (m_sUserSelection.nPrintReceipt == RCPT_QRCODE))
		bCondition = FALSE;
	// end of [#2235]

	// End of [#2219]

	// 명세표 선택 안 한 경우에  출금이면서 HOST 장애 또는 HOST 정상이나 정상 방출 못한 경우 명세표 출력한다. 
	if ( m_sUserSelection.nPrintReceipt == FALSE)	// If NO RECEIPT....			// [#2219] 2013.09.04 Justin Digital Receipt, Change Varialble Name
	{
		if ( TranCode == TC_WITHDRAWAL )
		{
			if ( m_pDevCmn->TranResult == FALSE)
				bCondition = TRUE;
		}
		else if ( TranCode == TC_REVERSAL ) // Reversal 일 경우 
		{
			int nRequestedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
			int nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

			if ( nRequestedAmount > 0 && nRequestedAmount > nDispensedAmount)
				bCondition = TRUE;
		}
	}

// [#2183] NH Justin 2013.04.03  Receipt Option (No Transaction result on Screen)
#if (AU_VERSION)
	if ( bCondition == TRUE && (TranCode == TC_WITHDRAWAL ||  TranCode == TC_REVERSAL))
	{
		if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
			bCondition = FALSE;
	}
#endif
// End of [#2183]

	NHDEBUG(DBG_INFO, (L"LIB_IsReceiptPrintCondition %d\n", bCondition));
	
	return bCondition;
}
//end of [#2000]

int CTranCmn::LIB_DayTotal(int PrintDevice, BOOL bCut)	// [#2185] US Justin 2013.05.15 Dual Host DCC, Add Cut Option
{
	NHDEBUG(DBG_CALL, (_T("***TranBizLib***[CTranCmn::LIB_DayTotal]\n")));

	CString		strTmp1		= "";
	CString		strTmp2		= "";
	CTime		CurTime = CTime::GetCurrentTime();
	int			nValueTemp;
	CString		strTemp;

	int			ErrCancelNum = 0, ErrTranNum = 0;/*, ErrHostNum = 0;*/
	int			i = 0, j = 0;
	long		lTemp = 0;


	CString		strPrintData;
	CString		strJnlData;

	// HEADER
	//------------------------------------------------------------------------------------------------
	if (PrintDevice & DEV_SPR)
	{
//	[#2160] CA KMK 2012.11.27 French OP
		AddPrintData(strPrintData, L"========================================");
		switch(TranCode)
		{
		case TC_TOTAL:
			AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_DAY_TOTAL)); break;
		default:
			AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_TRIALDAY_TOTAL)); break;
		}
		AddPrintData(strPrintData, L"========================================");
		//	end of [#2160]
	}

	// START TIME
	//------------------------------------------------------------------------------------------------
	{
		strTemp = MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_TOTALSTARTTIME);
		strTmp1.Format(L"%s %2.2s:%2.2s:%2.2s",	GetCmnLocalDate(strTemp.Left(2), strTemp.Mid(2,2), strTemp.Mid(4,4)),
												strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
		AddPrintData(strPrintData, L"%-18.18s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_030), strTmp1);	// [#2160] CA KMK 2012.11.27 FrenchOP

		strTmp1.Format(L"%2.2s/%2.2s/%4.4s %2.2s:%2.2s:%2.2s", strTemp.Left(2), strTemp.Mid(2,2), strTemp.Mid(4,4),
															   strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
		strJnlData += strTmp1;
	}

	// TODAY
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"%-18.18s = %s %02.2d:%02.2d:%02.2d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_002), GetCmnLocalDate(CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear()), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());	// [#2160] CA KMK 2012.11.27 FrenchOP

		if ((TranCode == TC_TOTAL) || (TranCode == TC_TRIALTOTAL)) // [#2014] NH KJW 2011.02.23
		{
			strTemp.Format(L"^%02.2d/%02.2d/%04.4d %02.2d:%02.2d:%02.2d",
								CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(),
								CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
			strJnlData += strTemp;

			// [#2014] NH KJW 2011.02.23
			if (TranCode == TC_TOTAL)
			// end of [#2014]
			{
				strTemp.Format(L"%02d%02d%04d%02d%02d%02d",			
								CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(),
								CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
				MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_TOTALSTARTTIME, strTemp);

				if (m_pDevCmn->OpenKey == FALSE)
				{
					strTemp.Format(L"%04d%02d%02d", CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay());
					MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DAYTOTAL_LASTDATE, strTemp);
				}
			}
		}
	}

	// TERMINAL ID
	//------------------------------------------------------------------------------------------------
	{
		strTmp1 = MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID);
		AddPrintData(strPrintData, L"%-18.18s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), strTmp1);	// [#2160] CA KMK 2012.11.27 French OP
		strTemp.Format(L"^%15.15s", strTmp1);
		strJnlData += strTemp;
	}

	// HOST RESULT
	//------------------------------------------------------------------------------------------------
	{
		if ((TranCode == TC_TRIALTOTAL) || (TranCode == TC_TOTAL))
		{
			if (m_pDevCmn->TranResult)
				AddPrintData(strPrintData, L"%-18.18s = (%s)", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_083), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_084));		// [#2160] CA KMK 2012.11.27
			else
				AddPrintData(strPrintData, L"%-18.18s = (%s)", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_083), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_085));	// [#2160] CA KMK 2012.11.27
		}

		AddPrintData(strPrintData, L"........................................");
	}

	CString strTemp2;

	// HOST TOTAL
	//------------------------------------------------------------------------------------------------
	AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_009));	// [#2160] CA KMK 2012.11.27 FrenchOP

	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
	{
		// NHA 요청사항 Trial Day Total Fail시 명세표 인자 & Journal 저장 하도록 수정
		//  4. Count of Withdrawals (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD1_TotalResp.NoOfCashWiths_4);
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), nValueTemp);	// [#2160] CA KMK 2012.11.27 FrenchOP
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			AddPrintData(strPrintData, L"%-21s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010));	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		//  5. Count of Balance (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD1_TotalResp.NoOfInquiries_4);
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_011), nValueTemp);	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			AddPrintData(strPrintData, L"%-21s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_011));	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		//  6. Count of Transfer (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD1_TotalResp.NoOfTransfers_4);
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_012), nValueTemp);	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			AddPrintData(strPrintData, L"%-21s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_012));	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}


		//  7. Count of Non-Cash (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD1_TotalResp.NoOfNonCashWiths_4);
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}


		//  8. Amount of Dispense (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD1_TotalResp.TotalDispenseAmt_V);
			strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(m_sSTD1_TotalResp.TotalDispenseAmt_V));
			AddPrintData(strPrintData, L"%-21.21s = %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^%12.12d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			AddPrintData(strPrintData, L"%-21.21s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014));		// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}


		//  9. Amount of Non-Cash (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD1_TotalResp.TotalNonCashDispAmt_V);
			strTemp.Format(L"^%12.12d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		// 10. Amount of Surcharge (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			strTemp.Format(L"^%6.6d", Asc2Int(m_sSTD1_TotalResp.TotalSurchargeAmt_V));
			strJnlData += strTemp;
		}
		else
		{
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}
	}
	else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
	{
		//  4. Count of Withdrawals (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD3_TotalResp.m_strNumberOfWithdrawals_4);
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), nValueTemp);	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			AddPrintData(strPrintData, L"%-21s = ", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010));		// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		//  5. Count of Balance (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD3_TotalResp.m_strNumberOfInquiries_4);
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_011), nValueTemp);	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			AddPrintData(strPrintData, L"%-21s = ", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_011));		// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		//  6. Count of Transfer (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD3_TotalResp.m_strNumberOfTransfers_4);
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_012), nValueTemp);	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			AddPrintData(strPrintData, L"%-21s = ", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_012));		// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}
	
		//  7. Count of Non-Cash
		if (m_pDevCmn->TranResult == TRUE)
		{
			strTemp.Format(L"^%06.6d", 0);
			strJnlData += strTemp;
		}
		else
		{
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		//  8. Amount of Dispense (HOST)
		if (Asc2Int(m_sSTD3_TotalResp.m_strSettlement_Ext12) > 0)
			strTmp1 = m_sSTD3_TotalResp.m_strSettlement_Ext12;
		else
			strTmp1 = m_sSTD3_TotalResp.m_strSettlement_8;

		if (m_pDevCmn->TranResult == TRUE)
		{
			strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTmp1));
			AddPrintData(strPrintData, L"%-21.21s = %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^%12.12d", Asc2Int(strTmp1));
			strJnlData += strTemp;
		}
		else
		{
			AddPrintData(strPrintData, L"%-21.21s = ", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014));	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		//  9. Amount of Non-Cash (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			strTemp.Format(L"^%012.12d", 0);
			strJnlData += strTemp;
		}
		else
		{
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		// 10. AMount of Surcharge (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			strTemp.Format(L"^%06.6d", 0);
			strJnlData += strTemp;
		}
		else
		{
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}
	}
	else			// CSP200 or EPS
	{
		//  4. Count of Withdrawals (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD2_TotalResp.NoOfCashWiths_4);
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), nValueTemp);	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			AddPrintData(strPrintData, L"%-21s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010));		// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		//  5. Count of Balance (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD2_TotalResp.NoOfInquiries_4);
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_011), nValueTemp);	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			AddPrintData(strPrintData, L"%-21s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_011));	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		//  6. Count of Transfer (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD2_TotalResp.NoOfTransfers_4);
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_012), nValueTemp);	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			AddPrintData(strPrintData, L"%-21s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_012));	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		//  7. Count of Non-Cash (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD2_TotalResp.NoOfNonCashWiths_4);
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		//  8. Amount of Dispense (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD2_TotalResp.TotalDispenseAmt_V);
			strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(m_sSTD2_TotalResp.TotalDispenseAmt_V));
			AddPrintData(strPrintData, L"%-21.21s = %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^%12.12d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			AddPrintData(strPrintData, L"%-21.21s =", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014));		// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		//  9. Amount of Non-Cash (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			nValueTemp = Asc2Int(m_sSTD2_TotalResp.TotalNonCashDispAmt_V);
			strTemp.Format(L"^%12.12d", nValueTemp);
			strJnlData += strTemp;
		}
		else
		{
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}

		// 10. Amount of Surcharge (HOST)
		if (m_pDevCmn->TranResult == TRUE)
		{
			strTemp.Format(L"^%6.6d", Asc2Int(m_sSTD2_TotalResp.TotalSurchargeAmt_V));
			strJnlData += strTemp;
		}
		else
		{
			strTemp.Format(L"^");
			strJnlData += strTemp;
		}
	}

	// TERMINAL TOTAL
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_016));	// [#2160] CA KMK 2012.11.27

		// 11. Count of Withdrawal (ATM)
		nValueTemp = (int) MemGetTotal(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_COUNT, 0);
		AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), nValueTemp);	// [#2160] CA KMK 2012.11.27
		strTemp.Format(L"^%6.6d", nValueTemp);
		strJnlData += strTemp;

		// 12. Count of Inquiry (ATM)
		nValueTemp = (int) MemGetTotal(_MEM_FLD_INI_INQTOTALINFO, _MEM_VAR_SUM_INQ_COUNT, 0);
		AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_011), nValueTemp);	// [#2160] CA KMK 2012.11.27
		strTemp.Format(L"^%6.6d", nValueTemp);
		strJnlData += strTemp;

		// 13. Count of Transfer (ATM)
		nValueTemp = (int) MemGetTotal(_MEM_FLD_INI_TRANSTOTALINFO, _MEM_VAR_SUM_TRANS_COUNT, 0);
		AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_012), nValueTemp);	// [#2160] CA KMK 2012.11.27
		strTemp.Format(L"^%6.6d", nValueTemp);
		strJnlData += strTemp;

		// 14. Count of Dispensed Set (ATM)
		nValueTemp = m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_1) + m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_2) +
					 m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_3) + m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_4);
		strTemp.Format(L"^%6.6d", nValueTemp);
		strJnlData += strTemp;

		// 15. Count of Non-Cash (ATM)
		nValueTemp = (int) MemGetTotal(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_COUNT, 12);

		AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_017), nValueTemp);		// [#2160] CA KMK 2012.11.27
		strTemp.Format(L"^%6.6d", nValueTemp);
		strJnlData += strTemp;

		// 16. Count of Reversal (ATM)
		nValueTemp = (int) MemGetTotal(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_COUNT, 5);

		// [#2166] CA KMK 2012.11.30 의미없는 if문 제거
		AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_018), nValueTemp);		// [#2160] CA KMK 2012.11.27
		// end of [#2166]

		strTemp.Format(L"^%6.6d", nValueTemp);
		strJnlData += strTemp;

		// 17. Amount of Withdrawal (ATM)
		lTemp = (long) MemGetTotal(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_AMOUNT, 0);

#if !(MX_VERSION)	// [#2115] MX KSK 2012.03.06 MX는 Save시에 Cent 포함하므로 Get시에 *100을 하지 않는다.
		lTemp = lTemp * 100;
#endif				// end of [#2115]

		strTmp1 = Int2Asc(lTemp);
		strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTmp1));
		AddPrintData(strPrintData, L"%-22.22s= %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);	// [#2160] CA KMK 2012.11.27
		strTemp.Format(L"^%12.12ld", lTemp);
		strJnlData += strTemp;

		// 18. Amount of Non-Cash (ATM)
		lTemp = (long) MemGetTotal(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_AMOUNT, 12);
		lTemp = lTemp * 100;
		strTmp1 = Int2Asc(lTemp);
		strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTmp1));
		AddPrintData(strPrintData, L"%-21s = %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_019), strTemp2);	// [#2160] CA KMK 2012.11.27
		strTemp.Format(L"^%12.12ld", lTemp);
		strJnlData += strTemp;
		
		//19. Surcharge Amount (ATM)
		lTemp = (long) MemGetTotal(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_AMOUNT, 11);
		strTmp1 = Int2Asc(lTemp);
		strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTmp1));
#if (AU_VERSION)
		AddPrintData(strPrintData, L"%-22s= %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_086), strTemp2);		// [#2160] CA KMK 2012.11.27
#elif (MX_VERSION)
		AddPrintData(strPrintData, L"%-21s = %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_087), strTemp2);	// [#2160] CA KMK 2012.11.27
#else
		AddPrintData(strPrintData, L"%-21s = %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_020), strTemp2);	// [#2160] CA KMK 2012.11.27
#endif
		strTemp.Format(L"^%6.6ld", lTemp);
		strJnlData += strTemp;

		// 20. Amount of Reversal (ATM)		// 2014.01.07 KMK 'Amount of Reversal' 필드는 현재 표시/출력 미지원. 사용하지 않음.
		lTemp = (long) MemGetTotal(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_AMOUNT, 5);
		strTemp.Format(L"^%12.12ld", lTemp);
		strJnlData += strTemp;
	}

	// Print Host Message
	// 'p' field print add
	//------------------------------------------------------------------------------------------------
	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
	{
		for (i = 0; i < m_strTritonPrtMsgArray.GetSize() && i < 4; i ++)
			AddPrintData(strPrintData, m_strTritonPrtMsgArray[i]);
	}
	// [#2182] NH Justin 2013.03.22 Add STD1 Print Line
	else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
	{
		for (i = 0; i < m_strSTD1PrtMsgArray.GetSize() && i < 4; i ++)
			AddPrintData(strPrintData, m_strSTD1PrtMsgArray[i]);
	}
	// End of [#2182]

	// [#2405] US Justin 2016.03.21 HalCash(PIN4) Online
	CString strPin4JNL = L"";
#if(APP_PIN4_CASHPICKUP) //(APP_CUSTOM_PAI)				// [#2471] US Justin Enable Pin4 to all customers
	// PIN4 Day Total
	if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE)==ENABLE) && (m_Pin4.m_bPin4Available==TRUE) )
	{
		CString strPin4TotalData = m_Pin4.GetDaytotalJournalData();
		CStringArray	arrTemp;
		SplitString(strPin4TotalData, L"^", arrTemp);
		if(arrTemp.GetSize() == 3)
		{
			// Header
			AddPrintData(strPrintData, L" ");
			AddPrintData(strPrintData, L"========================================");
			if	(TranCode == TC_TOTAL )		AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_PIN4_DAY_TOTAL));
			else							AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_PIN4_TRIALDAY_TOTAL));	
			AddPrintData(strPrintData, L"========================================");

			// START Time
			strTemp = arrTemp.GetAt(0);
			strTmp1.Format(L"%s %2.2s:%2.2s:%2.2s", GetCmnLocalDate(strTemp.Mid(4,2), strTemp.Mid(6,2), strTemp.Left(4)),strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_030), strTmp1);
			strTmp1.Format(L"%2.2s/%2.2s/%4.4s %2.2s:%2.2s:%2.2s",	strTemp.Mid(4,2), strTemp.Mid(6,2), strTemp.Left(4),  strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			strPin4JNL += strTmp1;

			// TODAY (Journal Only)
			strTemp.Format(L"^%02.2d/%02.2d/%04.4d %02.2d:%02.2d:%02.2d", CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
			strPin4JNL += strTemp;

			// TERMINAL ID (Journal Only)
			strTemp.Format(L"^%15.15s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
			strPin4JNL += strTemp;

			AddPrintData(strPrintData, L"........................................");

			// Count of Withdrawal
			nValueTemp = (int) Asc2Int( arrTemp.GetAt(1) );
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), nValueTemp);
			strTemp.Format(L"^%6.6d", nValueTemp);
			strPin4JNL += strTemp;

			// Amount of Withdrawal
			lTemp = (long)  ( Asc2Int(arrTemp.GetAt(2) ) * 100) ;
			strTmp1 = Int2Asc(lTemp);
			strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTmp1));
			AddPrintData(strPrintData, L"%-22.22s= %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^%12.12ld", lTemp);
			strPin4JNL += strTemp;
		}
	}
#endif

	CString strPopmoneyJNL = L"";
#if(APP_POPMONEY) // [#2471] US Justin Enable popmoney to all customers
	// Popmoney Day Total
	// [#2442] US Justin 2016.08.25 PopMoney DayTotal
	if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_ENABLE)==ENABLE)
	{
		CString strPopmoneyTotalData = m_PopMoneyData.GetDaytotalJournalData();
		CStringArray	arrTemp;
		SplitString(strPopmoneyTotalData, L"^", arrTemp);
		if(arrTemp.GetSize() == 3)
		{
			// Header
			AddPrintData(strPrintData, L" ");
			AddPrintData(strPrintData, L"========================================");
			if	(TranCode == TC_TOTAL )		AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_POPMONEY_DAY_TOTAL));
			else							AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_POPMONEY_TRIALDAY_TOTAL));	
			AddPrintData(strPrintData, L"========================================");

			// START Time
			strTemp = arrTemp.GetAt(0);
			strTmp1.Format(L"%s %2.2s:%2.2s:%2.2s", GetCmnLocalDate(strTemp.Mid(4,2), strTemp.Mid(6,2), strTemp.Left(4)),strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_030), strTmp1);
			strTmp1.Format(L"%2.2s/%2.2s/%4.4s %2.2s:%2.2s:%2.2s",	strTemp.Mid(4,2), strTemp.Mid(6,2), strTemp.Left(4),  strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			strPopmoneyJNL += strTmp1;

			// TODAY (Journal Only)
			strTemp.Format(L"^%02.2d/%02.2d/%04.4d %02.2d:%02.2d:%02.2d", CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
			strPopmoneyJNL += strTemp;

			// TERMINAL ID (Journal Only)
			strTemp.Format(L"^%15.15s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
			strPopmoneyJNL += strTemp;

			AddPrintData(strPrintData, L"........................................");

			// Count of Withdrawal
			nValueTemp = (int) Asc2Int( arrTemp.GetAt(1) );
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), nValueTemp);
			strTemp.Format(L"^%6.6d", nValueTemp);
			strPopmoneyJNL += strTemp;

			// Amount of Withdrawal
			lTemp = (long)  ( Asc2Int(arrTemp.GetAt(2) ) * 100) ;
			strTmp1 = Int2Asc(lTemp);
			strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTmp1));
			AddPrintData(strPrintData, L"%-22.22s= %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);
			strTemp.Format(L"^%12.12ld", lTemp);
			strPopmoneyJNL += strTemp;
		}
	}
	// End of [#2442]

#endif
	// End of [#2405]

	// [#2445] US Justin 2016.09.22 Just.Cash
	// Just.Cash Day Total
	CString strJustCashJNL = L"";
	CString strJCDigitalCurJNL = L"";		// [#2496]
#if(APP_JUST_CASH)
	if (H_NH_JC_IsJustCashCardless())
	{
		CString strJustCashTotalData = m_JustCashData.GetDaytotalJournalData(JUSTCASH_SERVICE_CARDLESS);
		CStringArray	arrTemp;
		SplitString(strJustCashTotalData, L"^", arrTemp);
		if(arrTemp.GetSize() == 3)
		{
			// Header
			AddPrintData(strPrintData, L" ");
			AddPrintData(strPrintData, L"========================================");
			if	(TranCode == TC_TOTAL )		AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_JUSTCASH_DAY_TOTAL));
			else							AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_JUSTCASH_TRIALDAY_TOTAL));	
			AddPrintData(strPrintData, L"========================================");

			// START Time
			strTemp = arrTemp.GetAt(0);
			strTmp1.Format(L"%s %2.2s:%2.2s:%2.2s", GetCmnLocalDate(strTemp.Mid(4,2), strTemp.Mid(6,2), strTemp.Left(4)),strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_030), strTmp1);
			strTmp1.Format(L"%2.2s/%2.2s/%4.4s %2.2s:%2.2s:%2.2s",	strTemp.Mid(4,2), strTemp.Mid(6,2), strTemp.Left(4),  strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			strJustCashJNL += strTmp1;

			// TODAY (Journal Only)
			strTemp.Format(L"^%02.2d/%02.2d/%04.4d %02.2d:%02.2d:%02.2d", CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
			strJustCashJNL += strTemp;

			// TERMINAL ID (Journal Only)
			strTemp.Format(L"^%15.15s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
			strJustCashJNL += strTemp;

			AddPrintData(strPrintData, L"........................................");

			// Count of Withdrawal
			nValueTemp = (int) Asc2Int( arrTemp.GetAt(1) );
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), nValueTemp);
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJustCashJNL += strTemp;

			// Amount of Withdrawal
			lTemp = (long)  ( Asc2Int(arrTemp.GetAt(2) ) * 100) ;
			strTmp1 = Int2Asc(lTemp);
			strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTmp1));
			AddPrintData(strPrintData, L"%-22.22s= %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);
			strTemp.Format(L"^%12.12ld", lTemp);
			strJustCashJNL += strTemp;
		}
	}

	// [#2496] US Justin 2017.08.17
	if (H_NH_JC_IsJustCashBitcoin())
	{
		CString strJustCashTotalData = m_JustCashData.GetDaytotalJournalData(JUSTCASH_SERVICE_DIGITAL_CURRENCY);
		CStringArray	arrTemp;
		SplitString(strJustCashTotalData, L"^", arrTemp);
		if(arrTemp.GetSize() == 3)
		{
			// Header
			AddPrintData(strPrintData, L" ");
			AddPrintData(strPrintData, L"========================================");
			if	(TranCode == TC_TOTAL )		AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_DIGITALCUR_DAY_TOTAL));
			else							AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_DIGITALCUR_TRIALDAY_TOTAL));	
			AddPrintData(strPrintData, L"========================================");

			// START Time
			strTemp = arrTemp.GetAt(0);
			strTmp1.Format(L"%s %2.2s:%2.2s:%2.2s", GetCmnLocalDate(strTemp.Mid(4,2), strTemp.Mid(6,2), strTemp.Left(4)),strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_030), strTmp1);
			strTmp1.Format(L"%2.2s/%2.2s/%4.4s %2.2s:%2.2s:%2.2s",	strTemp.Mid(4,2), strTemp.Mid(6,2), strTemp.Left(4),  strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			strJCDigitalCurJNL += strTmp1;

			// TODAY (Journal Only)
			strTemp.Format(L"^%02.2d/%02.2d/%04.4d %02.2d:%02.2d:%02.2d", CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
			strJCDigitalCurJNL += strTemp;

			// TERMINAL ID (Journal Only)
			strTemp.Format(L"^%15.15s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
			strJCDigitalCurJNL += strTemp;

			AddPrintData(strPrintData, L"........................................");

			// Count of Withdrawal
			nValueTemp = (int) Asc2Int( arrTemp.GetAt(1) );
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), nValueTemp);
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJCDigitalCurJNL += strTemp;

			// Amount of Withdrawal
			lTemp = (long)  Asc2Int(arrTemp.GetAt(2)) ;
			strTmp1 = Int2Asc(lTemp);
			strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTmp1));
			AddPrintData(strPrintData, L"%-22.22s= %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);
			strTemp.Format(L"^%12.12ld", lTemp);
			strJCDigitalCurJNL += strTemp;
		}
	}
	// End of [#2496]
#endif
	// End of [#2445]

	// [#RWC6-16] Bitload4U
	// B4U CCA Day Total
	CString strB4UJNL = L"";
#if(APP_B4U)
	if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_B4U_ENABLED)==ENABLE)
	{
		m_B4UJournal = new B4UJournal;
		CString strB4UTotalData = m_B4U->GetB4UDaytotalJournalData(*m_B4UJournal);
		CStringArray	arrTemp;
		SplitString(strB4UTotalData, L"^", arrTemp);
		if(arrTemp.GetSize() == 3)
		{
			// Header
			AddPrintData(strPrintData, L" ");
			AddPrintData(strPrintData, L"========================================");
			if	(TranCode == TC_TOTAL )		AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_B4U_DAY_TOTAL));
			else							AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_B4U_TRIALDAY_TOTAL));	
			AddPrintData(strPrintData, L"========================================");

			// START Time
			strTemp = arrTemp.GetAt(0);
			strTmp1.Format(L"%s %2.2s:%2.2s:%2.2s", GetCmnLocalDate(strTemp.Mid(4,2), strTemp.Mid(6,2), strTemp.Left(4)),strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_030), strTmp1);
			strTmp1.Format(L"%2.2s/%2.2s/%4.4s %2.2s:%2.2s:%2.2s",	strTemp.Mid(4,2), strTemp.Mid(6,2), strTemp.Left(4),  strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			strB4UJNL += strTmp1;

			// TODAY (Journal Only)
			strTemp.Format(L"^%02.2d/%02.2d/%04.4d %02.2d:%02.2d:%02.2d", CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
			strB4UJNL += strTemp;

			// TERMINAL ID (Journal Only)
			strTemp.Format(L"^%15.15s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
			strB4UJNL += strTemp;

			AddPrintData(strPrintData, L"........................................");

			// Count of Withdrawal
			nValueTemp = (int) Asc2Int( arrTemp.GetAt(1) );
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), nValueTemp);
			strTemp.Format(L"^%6.6d", nValueTemp);
			strB4UJNL += strTemp;

			// Amount of Withdrawal
			lTemp = (long)  ( Asc2Int(arrTemp.GetAt(2) ) * 100) ;
			strTmp1 = Int2Asc(lTemp);
			strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTmp1));
			AddPrintData(strPrintData, L"%-22.22s= %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);
			strTemp.Format(L"^%12.12ld", lTemp);
			strB4UJNL += strTemp;
		}
		if (m_B4UJournal != NULL)
			delete m_B4UJournal;
		m_B4UJournal = NULL;
	}
#endif
	// End of [#RWC6-16]


	// [#2446] US Justin 2016.09.29 Paypal CCA
	// Paypal CCA Day Total
	CString strPayPalJNL = L"";
#if(APP_PAYDIANT_CCA)
	if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE)==ENABLE) && (m_PayPalCCA.m_bPaypalCCAAvailable==TRUE) && (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE))
	{
		CString strPapPalTotalData = m_PayPalCCA.GetDaytotalJournalData();
		CStringArray	arrTemp;
		SplitString(strPapPalTotalData, L"^", arrTemp);
		if(arrTemp.GetSize() == 3)
		{
			// Header
			AddPrintData(strPrintData, L" ");
			AddPrintData(strPrintData, L"========================================");
			if	(TranCode == TC_TOTAL )		AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_PAYPAL_DAY_TOTAL));
			else							AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_PAYPAL_TRIALDAY_TOTAL));	
			AddPrintData(strPrintData, L"========================================");

			// START Time
			strTemp = arrTemp.GetAt(0);
			strTmp1.Format(L"%s %2.2s:%2.2s:%2.2s", GetCmnLocalDate(strTemp.Mid(4,2), strTemp.Mid(6,2), strTemp.Left(4)),strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_030), strTmp1);
			strTmp1.Format(L"%2.2s/%2.2s/%4.4s %2.2s:%2.2s:%2.2s",	strTemp.Mid(4,2), strTemp.Mid(6,2), strTemp.Left(4),  strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			strPayPalJNL += strTmp1;

			// TODAY (Journal Only)
			strTemp.Format(L"^%02.2d/%02.2d/%04.4d %02.2d:%02.2d:%02.2d", CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
			strPayPalJNL += strTemp;

			// TERMINAL ID (Journal Only)
			strTemp.Format(L"^%15.15s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
			strPayPalJNL += strTemp;

			AddPrintData(strPrintData, L"........................................");

			// Count of Withdrawal
			nValueTemp = (int) Asc2Int( arrTemp.GetAt(1) );
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), nValueTemp);
			strTemp.Format(L"^%6.6d", nValueTemp);
			strPayPalJNL += strTemp;

			// Amount of Withdrawal
			lTemp = (long)  ( Asc2Int(arrTemp.GetAt(2) ) * 100) ;
			strTmp1 = Int2Asc(lTemp);
			strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTmp1));
			AddPrintData(strPrintData, L"%-22.22s= %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);
			strTemp.Format(L"^%12.12ld", lTemp);
			strPayPalJNL += strTemp;
		}
	}
#endif
	// End of [#2446]

	if (PrintDevice & DEV_JPR)
	{	
		int nRmsCommandMode = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND);

		if (TranCode == TC_TOTAL)
		{
			if (m_pDevCmn->TranResult == TRUE)			// Day Total은 Host 통신 성공시에만 Journal에 저장한다.
			{
				// ATM (Remote) Day Total
				if( nRmsCommandMode == RMSCMD_IDLE )	m_pDevCmn->m_JNLMgr.Save(DAY_TOTAL, strJnlData);
				else									m_pDevCmn->m_JNLMgr.Save(REMOTE_DAY_TOTAL, strJnlData);

				// Additional Day Total (Pin4)				
				if(strPin4JNL.GetLength() > 0)			m_pDevCmn->m_JNLMgr.Save(PIN4_DAY_TOTAL, strPin4JNL);					// [#2405] US Justin 2016.03.21 HalCash Online					
				if(strPopmoneyJNL.GetLength() > 0)		m_pDevCmn->m_JNLMgr.Save(POPMONEY_DAY_TOTAL, strPopmoneyJNL);			// [#2442] US Justin 2016.08.24 Popmoney Total
				if(strJustCashJNL.GetLength() > 0)		m_pDevCmn->m_JNLMgr.Save(JUSTCASH_DAY_TOTAL, strJustCashJNL);			// [#2445] US Justin 2016.09.22 Just.Cash Total
				if(strJCDigitalCurJNL.GetLength() > 0)	m_pDevCmn->m_JNLMgr.Save(DIGITALCUR_DAY_TOTAL, strJCDigitalCurJNL);		// [#2496] US Justin 2017.08.17 Digital Currency Total
				if(strPayPalJNL.GetLength() > 0)		m_pDevCmn->m_JNLMgr.Save(PAYPAL_DAY_TOTAL, strPayPalJNL);				// [#2446] US Justin 2016.09.29 Paypal CCA
			}
		}
		else if (TranCode == TC_TRIALTOTAL)				// Trial Day Total은 Host 통신과 상관없이 Journal에 저장한다.
		{
			// ATM (Remote) Trial Day Total
			if( nRmsCommandMode == RMSCMD_IDLE )		m_pDevCmn->m_JNLMgr.Save(TRIALDAY_TOTAL, strJnlData);
			else										m_pDevCmn->m_JNLMgr.Save(REMOTE_TRIALDAY_TOTAL, strJnlData);

			// Additional Trial Day Total (Pin4)
			if(strPin4JNL.GetLength() > 0)				m_pDevCmn->m_JNLMgr.Save(PIN4_TRIALDAY_TOTAL, strPin4JNL);				// [#2405] US Justin 2016.03.21 HalCash Online
			if(strPopmoneyJNL.GetLength() > 0)			m_pDevCmn->m_JNLMgr.Save(POPMONEY_TRIALDAY_TOTAL, strPopmoneyJNL);		// [#2442] US Justin 2016.08.24 Popmoney Total
			if(strJustCashJNL.GetLength() > 0)			m_pDevCmn->m_JNLMgr.Save(JUSTCASH_TRIALDAY_TOTAL, strJustCashJNL);		// [#2445] US Justin 2016.09.22 Just.Cash Total
			if(strJCDigitalCurJNL.GetLength() > 0)		m_pDevCmn->m_JNLMgr.Save(DIGITALCUR_TRIALDAY_TOTAL, strJCDigitalCurJNL);// [#2496] US Justin 2017.08.17 Digital Currency Total
			if(strPayPalJNL.GetLength() > 0)			m_pDevCmn->m_JNLMgr.Save(PAYPAL_TRIALDAY_TOTAL, strPayPalJNL);			// [#2446] US Justin 2016.09.29 Paypal CCA
		}
	}

	if (PrintDevice & DEV_SPR)
		m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT, bCut);		// [#2185] US Justin 2013.05.15 Dual Host DCC, Add Cut Option

	return T_OK;
}

// [#2185] US Justin 2013.05.15 Dual Host DCC
int CTranCmn::LIB_DayTotal_DHDCC(int PrintDevice)	
{
	NHDEBUG(DBG_CALL, (_T("***TranBizLib***[CTranCmn::LIB_DayTotal_DHDCC]\n")));

	int			nValueTemp, i = 0, j = 0;
	long		lTemp = 0;
	CString		strTemp, strTemp2, strTmp1, strTmp2;
	CString		strPrintData, strJnlData;

	// JOURNAL ONLY -------------------------------------------------------------------------------------------
	{
		// 1. START DATE
		strTemp = MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_TOTALSTARTTIME);
		strTmp1.Format(L"%s %2.2s:%2.2s:%2.2s",	GetCmnLocalDate(strTemp.Left(2), strTemp.Mid(2,2), strTemp.Mid(4,4)), strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
		strJnlData += strTmp1;

		// 2. TODAY
		CTime		CurTime = CTime::GetCurrentTime();
		strTemp.Format(L"^%02.2d/%02.2d/%04.4d %02.2d:%02.2d:%02.2d",CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
		strJnlData += strTemp;

		// 3. TERMINAL ID
		strTemp.Format(L"^%15.15s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
		strJnlData += strTemp;
	}

	// HEADER
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"========================================");
		if(TranCode == TC_TOTAL)	AddPrintData(strPrintData, L"DCC HOST DAY TOTAL");
		else						AddPrintData(strPrintData, L"DCC HOST TRIAL DAY TOTAL");
	}

	// HOST RESULT
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_009));
		if (m_pDevCmn->TranResult)
		{
			AddPrintData(strPrintData, L"%-18.18s = (%s)", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_083), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_084));
			//  4. Count of Withdrawals (HOST)
			nValueTemp = Asc2Int(m_sSTD1_TotalResp.NoOfCashWiths_4);
			AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), nValueTemp);	// [#2160] CA KMK 2012.11.27 FrenchOP
			strTemp.Format(L"^%6.6d", nValueTemp);
			strJnlData += strTemp;

			// 5. COUNT OF BALANCE
			strJnlData += L"^0"; 

			// 6. COUNT OF TRANSFER
			strJnlData += L"^0";

			// 7. COUNT OF CHECK CASH
			strJnlData += L"^0"; 

			//  8. Amount of Dispense (HOST)
			nValueTemp = Asc2Int(m_sSTD1_TotalResp.TotalDispenseAmt_V);
			strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(m_sSTD1_TotalResp.TotalDispenseAmt_V));
			AddPrintData(strPrintData, L"%-21.21s = %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);	// [#2160] CA KMK 2012.11.27
			strTemp.Format(L"^%12.12d", nValueTemp);
			strJnlData += strTemp;

			// 9. AMOUNT OF CHECK CASH
			strJnlData += L"^0"; 

			// 10. Amount of Surcharge (HOST)
			strTemp.Format(L"^%6.6d", Asc2Int(m_sSTD1_TotalResp.TotalSurchargeAmt_V));
			strJnlData += strTemp;
		}
		else
		{
			AddPrintData(strPrintData, L"%-18.18s = (%s)", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_083), m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_085));
			strJnlData += L"^0"; // 4. COUNT OF WITHDRAWAL
			strJnlData += L"^0"; // 5. COUNT OF BALANCE
			strJnlData += L"^0"; // 6. COUNT OF TRANSFER
			strJnlData += L"^0"; // 7. COUNT OF CHECK CASH
			strJnlData += L"^0"; // 8. AMOUNT OF DISPENSE
			strJnlData += L"^0"; // 9. AMOUNT OF CHECK CASH
			strJnlData += L"^0"; // 10. Amount of Surcharge (HOST)
		}
	}

	// TERMINAL TOTAL
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_016));

		// 11. Count of Withdrawal (ATM)
		nValueTemp = (int) MemGetTotal(_MEM_FLD_INI_DHDCCINFO, _MEM_VAR_SUM_DHDCC_COUNT, 0);
		AddPrintData(strPrintData, L"%-21s = %16d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_010), nValueTemp);
		strTemp.Format(L"^%6.6d", nValueTemp);
		strJnlData += strTemp;
	
		// 12. Count of Inquiry (ATM)
		strJnlData += L"^0"; 

		// 13. Count of Transfer (ATM)
		strJnlData += L"^0"; 	

		// 14. Count of Dispensed Set (ATM)
		strJnlData += L"^0"; 

		// 15. Count of Non-Cash (ATM)
		strJnlData += L"^0"; 

		// 16. Count of Reversal (ATM)
		nValueTemp = (int) MemGetTotal(_MEM_FLD_INI_DHDCCINFO, _MEM_VAR_SUM_DHDCC_COUNT, 2);
		strTemp.Format(L"^%6.6d", nValueTemp);
		strJnlData += strTemp;

		// 17. Amount of Withdrawal (ATM)
		lTemp = (long) MemGetTotal(_MEM_FLD_INI_DHDCCINFO, _MEM_VAR_SUM_DHDCC_AMOUNT, 0) * 100;
		strTmp1 = Int2Asc(lTemp);
		strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTmp1));
		AddPrintData(strPrintData, L"%-22.22s= %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_014), strTemp2);
		strTemp.Format(L"^%12.12ld", lTemp);
		strJnlData += strTemp;

		// 18. Amount of Non-Cash (ATM)
		strJnlData += L"^0"; 

		//19. Surcharge Amount (ATM)
		lTemp = (long) MemGetTotal(_MEM_FLD_INI_DHDCCINFO, _MEM_VAR_SUM_DHDCC_AMOUNT, 1);
		strTmp1 = Int2Asc(lTemp);
		strTemp2.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(strTmp1));
		AddPrintData(strPrintData, L"%-21s = %16.16s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_020), strTemp2);
		strTemp.Format(L"^%6.6ld", lTemp);
		strJnlData += strTemp;

		// 20. Amount of Reversal (ATM)
		strJnlData += L"^0"; 
	}

	for (i = 0; i < m_strSTD1PrtMsgArray.GetSize() && i < 4; i ++)
		AddPrintData(strPrintData, m_strSTD1PrtMsgArray[i]);
	
	if (PrintDevice & DEV_JPR)
	{
		if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND) == RMSCMD_IDLE)
		{
			if (TranCode == TC_TOTAL)
			{
				if (m_pDevCmn->TranResult == TRUE)	// Day Total은 Host 통신 성공시에만 Journal에 저장한다.
					m_pDevCmn->m_JNLMgr.Save(DAY_TOTAL, strJnlData);
			}
			else if (TranCode == TC_TRIALTOTAL)		// Trial Day Total은 Host 통신과 상관없이 Journal에 저장한다.
				m_pDevCmn->m_JNLMgr.Save(TRIALDAY_TOTAL, strJnlData);
		}
		else
		{
			if (TranCode == TC_TOTAL)
			{
				if (m_pDevCmn->TranResult == TRUE)	// Day Total은 Host 통신 성공시에만 Journal에 저장한다.
					m_pDevCmn->m_JNLMgr.Save(REMOTE_DAY_TOTAL, strJnlData);
			}
			else if (TranCode == TC_TRIALTOTAL)		// Trial Day Total은 Host 통신과 상관없이 Journal에 저장한다.
				m_pDevCmn->m_JNLMgr.Save(REMOTE_TRIALDAY_TOTAL, strJnlData);
		}
	}

	if (PrintDevice & DEV_SPR)
	{
		m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT);
	}
	return T_OK;
}
// End of [#2185]

typedef struct _CE_PRT_KINDCODE_DESC_
{
	WCHAR	szKindCode[3];
	WCHAR	szDescription[41];
} CE_PRT_KINDCODE_DESC;

CE_PRT_KINDCODE_DESC g_CESupportPRTKindCode[] =
{
	// [#2160] CA KMK 2012.11.27 French OP, 배열 재구성
	{ADDCASH_CST1,					T_ADDCASH_CST1},		/* ADD CASH #1 */		
	{ADDCASH_CST2,					T_ADDCASH_CST2},		/* ADD CASH #2 */		
	{ADDCASH_CST1_2,				T_ADDCASH_CST1_2},		/* ADD CASH #1,2 */		
	{ADDCASH_CST3,					T_ADDCASH_CST3},		/* ADD CASH #3 */		
	{ADDCASH_CST1_3,				T_ADDCASH_CST1_3},		/* ADD CASH #1,3 */		
	{ADDCASH_CST2_3,				T_ADDCASH_CST2_3},		/* ADD CASH #2,3 */		
	{ADDCASH_CST1_2_3,				T_ADDCASH_CST1_2_3},	/* ADD CASH #1,2,3 */
	{ADDCASH_CST4,					T_ADDCASH_CST4},		/* ADD CASH #4 */		
	{ADDCASH_CST1_4,				T_ADDCASH_CST1_4},		/* ADD CASH #1,4 */		
	{ADDCASH_CST2_4,				T_ADDCASH_CST2_4},		/* ADD CASH #2,4 */		
	{ADDCASH_CST1_2_4,				T_ADDCASH_CST1_2_4},	/* ADD CASH #1,2,4 */
	{ADDCASH_CST3_4,				T_ADDCASH_CST3_4},		/* ADD CASH #3,4 */		
	{ADDCASH_CST1_3_4,				T_ADDCASH_CST1_3_4},	/* ADD CASH #1,3,4 */
	{ADDCASH_CST2_3_4,				T_ADDCASH_CST2_3_4},	/* ADD CASH #2,3,4 */
	{ADDCASH_CST1_2_3_4,			T_ADDCASH_CST1_2_3_4},	/* ADD CASH #1,2,3,4 */

	{CHANGE_DENOM_CST1,				T_CHANGE_DENOM_CST1},		/* CHANGE DENOMINATION OF CASSETTE #1 */
	{CHANGE_DENOM_CST2,				T_CHANGE_DENOM_CST2},		/* CHANGE DENOMINATION OF CASSETTE #2 */
	{CHANGE_DENOM_CST1_2,			T_CHANGE_DENOM_CST1_2},		/* CHANGE DENOMINATION OF CASSETTE #1,2 */
	{CHANGE_DENOM_CST3,				T_CHANGE_DENOM_CST3},		/* CHANGE DENOMINATION OF CASSETTE #3 */
	{CHANGE_DENOM_CST1_3,			T_CHANGE_DENOM_CST1_3},		/* CHANGE DENOMINATION OF CASSETTE #1,3 */
	{CHANGE_DENOM_CST2_3,			T_CHANGE_DENOM_CST2_3},		/* CHANGE DENOMINATION OF CASSETTE #2,3 */
	{CHANGE_DENOM_CST1_2_3,			T_CHANGE_DENOM_CST1_2_3},	/* CHANGE DENOMINATION OF CASSETTE #1,2,3 */
	{CHANGE_DENOM_CST4,				T_CHANGE_DENOM_CST4},		/* CHANGE DENOMINATION OF CASSETTE #4 */
	{CHANGE_DENOM_CST1_4,			T_CHANGE_DENOM_CST1_4},		/* CHANGE DENOMINATION OF CASSETTE #1,4 */
	{CHANGE_DENOM_CST2_4,			T_CHANGE_DENOM_CST2_4},		/* CHANGE DENOMINATION OF CASSETTE #2,4 */
	{CHANGE_DENOM_CST1_2_4,			T_CHANGE_DENOM_CST1_2_4},	/* CHANGE DENOMINATION OF CASSETTE #1,2,4 */
	{CHANGE_DENOM_CST3_4,			T_CHANGE_DENOM_CST3_4},		/* CHANGE DENOMINATION OF CASSETTE #3,4 */	
	{CHANGE_DENOM_CST1_3_4,			T_CHANGE_DENOM_CST1_3_4},	/* CHANGE DENOMINATION OF CASSETTE #1,3,4 */
	{CHANGE_DENOM_CST2_3_4,			T_CHANGE_DENOM_CST2_3_4},	/* CHANGE DENOMINATION OF CASSETTE #1,3,4 */
	{CHANGE_DENOM_CST1_2_3_4,		T_CHANGE_DENOM_CST1_2_3_4},	/* CHANGE DENOMINATION OF CASSETTE #1,2,3,4 */
	// end of [#2160]
};

#define COUNT_CE_PRT_KINDCODE (sizeof(g_CESupportPRTKindCode)/sizeof(CE_PRT_KINDCODE_DESC))

int	CTranCmn::LIB_CashPrintResult(int nActUser)
{
	NHDEBUG(DBG_CALL, (_T("***TranBizLib***[CTranCmn::LIB_CashPrintResult]\n")));

	int			i = 0, j = 0;
	int			TempRejectCount1 = 0, TempRejectCount2 = 0, TempRejectCount3 = 0, TempRejectCount4 =0;
	int			TempJamCount1 = 0, TempJamCount2 = 0, TempJamCount3 = 0, TempJamCount4 =0;
	CString		strTemp, strTemp1, strEJNLData;
	CTime		CurTime = CTime::GetCurrentTime();
	CString		strPrintData;

	// HEADER
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"========================================");

		switch (TranCode)
		{
		case TC_CSTTOTAL:
			AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_CST_TOTAL));	// [#2160] CA KMK 2012.11.27
			strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CASSETTETOTAL_STARTTIME);
			break;
		case TC_TRIALCSTTOTAL:
			AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_TRIALCST_TOTAL));		// [#2160] CA KMK 2012.11.27
			strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CASSETTETOTAL_STARTTIME);
			break;
		case TC_SETDENO:
			for (i = 0; i < COUNT_CE_PRT_KINDCODE; i++)
			{
				if (g_CESupportPRTKindCode[i].szKindCode == m_pDevCmn->strJnlKindCode)
				{
					AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportPRTKindCode[i].szDescription));		// [#2160] CA KMK 2012.11.27
					break;
				}
			}
			break;
		}
		AddPrintData(strPrintData, L"========================================");
	}

	// 1. START TIME
	//------------------------------------------------------------------------------------------------
	{
		if (TranCode != TC_SETDENO)
		{
			if (strTemp.GetLength() >= 14)
			{
				strTemp1.Format(L"%s %2.2s:%2.2s:%2.2s",
					GetCmnLocalDate(strTemp.Left(2), strTemp.Mid(2,2), strTemp.Mid(4,4)),
					strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
				AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_030), strTemp1);	// [#2160] CA KMK 2012.11.27
				if (TranCode == TC_CSTTOTAL || TranCode == TC_TRIALCSTTOTAL)
				{
					strTemp1.Format(L"%2.2s/%2.2s/%4.4s %2.2s:%2.2s:%2.2s",
						strTemp.Left(2), strTemp.Mid(2,2), strTemp.Mid(4,4),
						strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
					strEJNLData += strTemp1;
				}
			}
			else
			{
				AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_030), strTemp);		// [#2160] CA KMK 2012.11.27
				if (TranCode == TC_CSTTOTAL || TranCode == TC_TRIALCSTTOTAL)
				{
					strTemp1.Format(L"%19.19s", strTemp);
					strEJNLData += strTemp1;
				}
			}
		}
	}

	// 2. TODAY
	//------------------------------------------------------------------------------------------------
	{
		strTemp.Format(L"%s %02.2d:%02.2d:%02.2d",
					GetCmnLocalDate(CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear()),
					CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
		AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_002), strTemp);		// [#2160] CA KMK 2012.11.27 FrenchOP

		if (TranCode == TC_CSTTOTAL)
		{
			strTemp.Format(L"%02d%02d%04d%02d%02d%02d",			
								CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(),
								CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());	
			MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CASSETTETOTAL_STARTTIME, strTemp);
		}
		if (TranCode == TC_CSTTOTAL || TranCode == TC_TRIALCSTTOTAL)
		{
			strTemp.Format(L"%02.2d/%02.2d/%04.4d %02.2d:%02.2d:%02.2d",
								CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(),
								CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
			strEJNLData += L"^";
			strEJNLData += strTemp;
		}
	}

	// 3. TERMINAL INFO
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));		// [#2160] CA KMK 2012.11.27
		strTemp.Format(L"^%15.15s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
		strEJNLData += strTemp;

		if(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_002), MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID));	// [#2160] CA KMK 2012.11.27
		else
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_003), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));		// [#2160] CA KMK 2012.11.27
	}

	// [#2241] AU KMK 2014.01.08 CST Total 명세표 수정 (호주 only)
#if (AU_VERSION)
	// 저널은 기존 사양 유지 (기존 소스에서 명세표 출력부만 새로 작성함)
	{
		// [#2241] AU KMK 2014.03.03 Note Counting Disable 시에 Reject Count가 0이 되는 현상 수정
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
		{
			// [#2270] AU KSK 2014.05.26 Reject 매수정보도 AP에서 관리하는 값을 표시하도록 추가 수정
			// NOTE COUNTING ENABLE 시에는 기존 로직 사용
			TempRejectCount1 = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_1)
				- m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1)
				- m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_1)
				- m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_1);

			if (TempRejectCount1 >= 0)
				TempRejectCount1 = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_1);
			else
				TempRejectCount1 += m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_1);
			if (TempRejectCount1 < 0)
				TempRejectCount1 = 0;

			TempJamCount1 = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_1)
				- m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1)
				- m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_1)
				- TempRejectCount1;
			if (TempJamCount1 < 0)
				TempJamCount1 = 0;
		}
		else
		{
			// NOTE COUNTING DISABLE 시, REJECT 매수 값을 그대로 사용함
			TempRejectCount1 = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_1);
			TempJamCount1 = 0;
		}
		// end of [#2241]

		if (TranCode == TC_CSTTOTAL || TranCode == TC_TRIALCSTTOTAL)
		{
			//  4. [CST1] Initial Notes
			// [#2270] AU KSK 2014.05.26 AP에서 Setting한 값을 저널 및 명세표 Print
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			{
				strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_1));
			}
			else
			{
				// [#2270] AU KSK 2014.05.26 AP에서 Setting한 값을 저널 및 명세표 Print
				strTemp.Format(L"^%6.6d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_INITIALCOUNT_AP));
				// end of [#2270]
			}
			// end of [#2270]
			strEJNLData += strTemp;

			//  5. [CST1] Dispensed Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_1));
			strEJNLData += strTemp;

			//  6. [CST1] Rejected Notes
			strTemp.Format(L"^%6.6d", TempRejectCount1);
			strEJNLData += strTemp;

			//  7. [CST1] Jam Notes
			strTemp.Format(L"^%6.6d", TempJamCount1);
			strEJNLData += strTemp;

			//  8. [CST1] Remaining Notes
			// [#2270] AU KSK 2014.05.26 AP에서 계산한 Remain Count값을 저장함 (-1로 저장하는 로직 추가 수정)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			{
				strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1));
			}
			else
			{
				// [#2270] AU KSK 2014.05.26 AP에서 계산한 Remain Count값을 저장함
				strTemp.Format(L"^%6.6d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP));
				// end of [#2270]
			}
			// end of [#2270]
			strEJNLData += strTemp;

			//  9. [CST1] Denomination
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_1));
			strEJNLData += strTemp;
		}

		// [#2241] AU KMK 2014.03.03 Note Counting Disable 시에 Reject Count가 0이 되는 현상 수정
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
		{
			// NOTE COUNTING ENABLE 시에는 기존 로직 사용
			TempRejectCount2 = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_2)
				- m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_2)
				- m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_2)
				- m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_2);

			if (TempRejectCount2 >= 0)
				TempRejectCount2 = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_2);
			else
				TempRejectCount2 += m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_2);
			if (TempRejectCount2 < 0)
				TempRejectCount2 = 0;

			TempJamCount2 = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_2)
				- m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_2)
				- m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_2)
				- TempRejectCount2;
			if (TempJamCount2 < 0)
				TempJamCount2 = 0;
		}
		else
		{
			// NOTE COUNTING DISABLE 시, REJECT 매수 값을 그대로 사용함
			TempRejectCount2 = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_2);
			TempJamCount2 = 0;
		}
		// end of [#2241]

		if (TranCode == TC_CSTTOTAL || TranCode == TC_TRIALCSTTOTAL)
		{
			// 10. [CST2] Initial Notes
			// [#2270] AU KSK 2014.05.26 AP에서 계산한 Remain Count값을 저장함 (-1로 저장하는 로직 추가 수정)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			{
				strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_2));
			}
			else
			{
				// [#2270] AU KSK 2014.05.26
				strTemp.Format(L"^%6.6d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST2_INITIALCOUNT_AP));
				// end of [#2270]
			}
			// end of [#2270]
			strEJNLData += strTemp;

			// 11. [CST2] Dispensed Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_2));
			strEJNLData += strTemp;

			// 12. [CST2] Rejected Notes
			strTemp.Format(L"^%6.6d", TempRejectCount2);
			strEJNLData += strTemp;

			// 13. [CST2] Jam Notes
			strTemp.Format(L"^%6.6d", TempJamCount2);
			strEJNLData += strTemp;

			// 14. [CST2] Remaining Notes
			// [#2270] AU KSK 2014.05.26 AP에서 계산한 Remain Count값을 저장함 (-1로 저장하는 로직 추가 수정)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			{
				strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_2));
			}
			else
			{
				// [#2270] AU KSK 2014.05.26
				strTemp.Format(L"^%6.6d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST2_CURRENTCOUNT_AP));
				// end of [#2270]
			}
			// end of [#2270]
			strEJNLData += strTemp;

			// 15. [CST2] Denomination
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_2));
			strEJNLData += strTemp;
		}

		// [#2241] AU KMK 2014.03.03 Note Counting Disable 시에 Reject Count가 0이 되는 현상 수정
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
		{
			// NOTE COUNTING ENABLE 시에는 기존 로직 사용
			TempRejectCount3 = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_3)
				- m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_3)
				- m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_3)
				- m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_3);

			if (TempRejectCount3 >= 0)
				TempRejectCount3 = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_3);
			else
				TempRejectCount3 += m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_3);
			if (TempRejectCount3 < 0)
				TempRejectCount3 = 0;

			TempJamCount3 = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_3)
				- m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_3)
				- m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_3)
				- TempRejectCount3;
			if (TempJamCount3 < 0)
				TempJamCount3 = 0;
		}
		else
		{
			// NOTE COUNTING DISABLE 시, REJECT 매수 값을 그대로 사용함
			TempRejectCount3 = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_3);
			TempJamCount3 = 0;
		}
		// end of [#2241]

		if (TranCode == TC_CSTTOTAL || TranCode == TC_TRIALCSTTOTAL)
		{
			// 16. [CST3] Initial Notes
			// [#2270] AU KSK 2014.05.26 AP에서 계산한 Remain Count값을 저장함 (-1로 저장하는 로직 추가 수정)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			{
				strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_3));
			}
			else
			{
				// [#2270] AU KSK 2014.05.26
				strTemp.Format(L"^%6.6d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST3_INITIALCOUNT_AP));
				// end of [#2270]
			}
			// end of [#2241]
			// end of [#2270]
			strEJNLData += strTemp;

			// 17. [CST3] Dispensed Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_3));
			strEJNLData += strTemp;

			// 18. [CST3] Rejected Notes
			strTemp.Format(L"^%6.6d", TempRejectCount3);
			strEJNLData += strTemp;

			// 19. [CST3] Jam Notes
			strTemp.Format(L"^%6.6d", TempJamCount3);
			strEJNLData += strTemp;

			// 20. [CST3] Remaining Notes
			// [#2270] AU KSK 2014.05.26 AP에서 계산한 Remain Count값을 저장함 (-1로 저장하는 로직 추가 수정)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			{
				strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_3));
			}
			else
			{
				strTemp.Format(L"^%6.6d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST3_CURRENTCOUNT_AP));
			}
			// end of [#2270]
			strEJNLData += strTemp;

			// 21. [CST3] Denomination
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_3));
			strEJNLData += strTemp;
		}

		// [#2241] AU KMK 2014.03.03 Note Counting Disable 시에 Reject Count가 0이 되는 현상 수정
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
		{
			// NOTE COUNTING ENABLE 시에는 기존 로직 사용
			TempRejectCount4 = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_4)
				- m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_4)
				- m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_4)
				- m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_4);

			if (TempRejectCount4 >= 0)
				TempRejectCount4 = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_4);
			else
				TempRejectCount4 += m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_4);
			if (TempRejectCount4 < 0)
				TempRejectCount4 = 0;

			TempJamCount4 = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_4)
				- m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_4)
				- m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_4)
				- TempRejectCount4;
			if (TempJamCount4 < 0)
				TempJamCount4 = 0;
		}
		else
		{
			// NOTE COUNTING DISABLE 시, REJECT 매수 값을 그대로 사용함
			TempRejectCount4 = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_4);
			TempJamCount4 = 0;
		}
		// end of [#2241]

		if (TranCode == TC_CSTTOTAL || TranCode == TC_TRIALCSTTOTAL)
		{
			// 22. [CST4] Initial Notes
			// [#2270] AU KSK 2014.05.26 AP에서 계산한 Remain Count값을 저장함 (-1로 저장하는 로직 추가 수정)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			{
				strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_4));
			}
			else
			{
				// [#2270] AU KSK 2014.05.26
				strTemp.Format(L"^%6.6d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST4_INITIALCOUNT_AP));
				// end of [#2270]
			}
			// end of [#2270]
			strEJNLData += strTemp;

			// 23. [CST4] Dispensed Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_4));
			strEJNLData += strTemp;

			// 24. [CST4] Rejected Notes
			strTemp.Format(L"^%6.6d", TempRejectCount4);
			strEJNLData += strTemp;

			// 25. [CST4] Jam Notes
			strTemp.Format(L"^%6.6d", TempJamCount4);
			strEJNLData += strTemp;

			// 26. [CST4] Remaining Notes
			// [#2270] AU KSK 2014.05.26 AP에서 계산한 Remain Count값을 저장함 (-1로 저장하는 로직 추가 수정)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			{
				strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_4));
			}
			else
			{
				strTemp.Format(L"^%6.6d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST4_CURRENTCOUNT_AP));
			}
			// end of [#2270]
			strEJNLData += strTemp;

			// 27. [CST4] Denomination
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_4));
			strEJNLData += strTemp;
		}

		if (TranCode == TC_CSTTOTAL || TranCode == TC_TRIALCSTTOTAL)
		{
			// 28. Non-Cash Value
			strTemp.Format(L"^%06.6d", 0);
			strEJNLData += strTemp;

			// 29. Non-Cash Type
			strTemp.Format(L"^%012.12d", 0);
			strEJNLData += strTemp;
		}
	}
	// 명세표 출력부 신규 작성
	int nTotalDispensedAmount = 0;	// total dispensed amount(all cassettes) (CASSETTE TOTAL)
	for (int nCurCstNumber=CDU_CST_1; nCurCstNumber<=CDU_MAX_CST_COUNT; nCurCstNumber++)
	{
		// Denomination이 0인 Cassette의 정보는 출력하지 않는다
		if (m_pDevCmn->fnCDU_GetValueOfCash(nCurCstNumber) != 0)
		{
			int nCstInitialCount, nCstInitialAmount;			// initial count/amount
			int nCstDispensedCount, nCstDispensedAmount;		// dispensed count/amount
			int nCstRemainingCount, nCstRemainingAmount;		// remaining count/amount
			int nCstRejectCount;								// reject count
			int nCstDenomination;								// denomination
			int nTempCstRejectCount;							// [#2464] AU KSK 2016.12.21

			// counts
			// [#2270] AU KSK 2014.05.26 Note Counting Option을 보고 AP에서 Get하도록 추가 수정
			nCstInitialCount = m_pDevCmn->fnCDU_GetNumberOfSetCash(nCurCstNumber);
			nCstDispensedCount = m_pDevCmn->fnCDU_GetNumberOfDispenseCash(nCurCstNumber);
			nCstRemainingCount = m_pDevCmn->fnCDU_GetNumberOfCash(nCurCstNumber);
			nCstRejectCount = m_pDevCmn->fnCDU_GetNumberOfReject(nCurCstNumber);

			// [#2463] AU KSK 2016.12.21 Reject 매수 보정처리
			nTempCstRejectCount = nCstInitialCount - nCstRemainingCount - nCstDispensedCount - nCstRejectCount;

			if (nTempCstRejectCount >= 0)
				nTempCstRejectCount = nCstRejectCount;
			else
				nTempCstRejectCount += nCstRejectCount;

			if (nTempCstRejectCount < 0)
				nTempCstRejectCount = 0;

			nCstRejectCount = nTempCstRejectCount;	// 최종으로 보정된 매수를 CST Reject 매수로 사용

			/*
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			{
				nCstInitialCount = m_pDevCmn->fnCDU_GetNumberOfSetCash(nCurCstNumber);
				nCstRemainingCount = m_pDevCmn->fnCDU_GetNumberOfCash(nCurCstNumber);
			}
			else
			{
				nCstInitialCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_INITIALCOUNT_AP + (nCurCstNumber-1));
				nCstRemainingCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP + (nCurCstNumber-1));		
			}
			*/
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) != 0)
			{
				nCstInitialCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_INITIALCOUNT_AP + (nCurCstNumber-1));
				nCstRemainingCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP + (nCurCstNumber-1));	
			}
			// end of [#2463]
			// end of [#2270]

			// denomination of this cassette
			nCstDenomination = m_pDevCmn->fnCDU_GetValueOfCash(nCurCstNumber);	

			// calculate each amounts
			nCstInitialAmount = nCstDenomination * nCstInitialCount;
			nCstDispensedAmount = nCstDenomination * nCstDispensedCount;
			nCstRemainingAmount = nCstDenomination * nCstRemainingCount;			
			nTotalDispensedAmount += nCstDispensedAmount;

			AddPrintData(strPrintData, L"----------------------------------------");
			strTemp.Format(L"DENOMINATION %s%d", GetCurrencySymbol(), nCstDenomination);
			AddPrintData(strPrintData, L"CASSETTE #%d %28.28s", nCurCstNumber, strTemp);
			AddPrintData(strPrintData, L"----------------------------------------");


			// INITIAL COUNT / INITIAL AMOUNT
			// [#2270] AU KSK 2014.05.26 AP에서 관리하는 값을 표시하도록 추가 수정
			// if the note counting is disabled, it won't be printed
			AddPrintData(strPrintData, L"INITIAL COUNT     = %20d", nCstInitialCount);
			strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nCstInitialAmount * 100)));
			AddPrintData(strPrintData, L"INITIAL AMOUNT    = %20.20s", strTemp);
			// end of [#2270]

			// DISPENSED COUNT / DISPENSED AMOUNT
			AddPrintData(strPrintData, L"DISPENSED COUNT   = %20d", nCstDispensedCount);
			strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nCstDispensedAmount * 100)));
			AddPrintData(strPrintData, L"DISPENSED AMOUNT  = %20.20s", strTemp);

			// REMAINING COUNT / REMAINING AMOUNT
			// [#2270] AU KSK 2014.05.26 AP에서 관리하는 값을 표시하도록 추가 수정
			AddPrintData(strPrintData, L"REMAINING COUNT   = %20d", nCstRemainingCount);
			strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nCstRemainingAmount * 100)));
			AddPrintData(strPrintData, L"REMAINING AMOUNT  = %20.20s", strTemp);
			// end of [#2270]

			// REJECT COUNT
			AddPrintData(strPrintData, L"........................................");
			AddPrintData(strPrintData, L"REJECTED COUNT    = %20d", nCstRejectCount);
		}

	}
	// TOTAL AMOUNT (ALL CASSETTES)
	//------------------------------------------------------------------------------------------------
	strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nTotalDispensedAmount * 100)));
// 	AddPrintData(strPrintData, FIELD_DELIMITER);
	AddPrintData(strPrintData, L" ");	// [#2270] AU KSK 2014.05.30 구분을 위해 1 LINE 여백 추가
	AddPrintData(strPrintData, L"TOTAL DISPENSED AMOUNT = %15.15s", strTemp);
#else
	// end of [#2241]

	// CASSETTE INFO
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"----------------------------------------");
		AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_004));	// [#2160] CA KMK 2012.11.27
		AddPrintData(strPrintData, L"----------------------------------------");
		AddPrintData(strPrintData, L"%-12.12s%-6.6s:  %4.4s,%4.4s,%4.4s,%4.4s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_023), GetCurrencySymbol(6,1),		// [#2160] CA KMK 2012.11.27
												Int2Asc(m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_1)),
												Int2Asc(m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_2)),
												Int2Asc(m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_3)),
												Int2Asc(m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_4)));

		AddPrintData(strPrintData, L"%-17.17s :  %4.4s,%4.4s,%4.4s,%4.4s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_024),	// [#2160] CA KMK 2012.11.27
												Int2Asc(m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_1)),
												Int2Asc(m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_2)),
												Int2Asc(m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_3)),
												Int2Asc(m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_4)));
		AddPrintData(strPrintData, L"........................................");
		AddPrintData(strPrintData, L"%-17.17s :  %4.4s,%4.4s,%4.4s,%4.4s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_025),	// [#2160] CA KMK 2012.11.27
												Int2Asc(m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1)),
												Int2Asc(m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_2)),
												Int2Asc(m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_3)),
												Int2Asc(m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_4)));

		TempRejectCount1 =  m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_1)		-
							m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1)			-	
							m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_1)	-
							m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_1);

		if (TempRejectCount1 >= 0)
			TempRejectCount1 = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_1);
		else
			TempRejectCount1 += m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_1);

		if (TempRejectCount1 < 0)
			TempRejectCount1 = 0;

		TempJamCount1 = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_1)		-
						m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1)			-	
						m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_1)	-
						TempRejectCount1;
		if (TempJamCount1 < 0)
			TempJamCount1 = 0;

		if (TranCode == TC_CSTTOTAL || TranCode == TC_TRIALCSTTOTAL)
		{
			//  4. [CST1] Initial Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_1));
			strEJNLData += strTemp;

			//  5. [CST1] Dispensed Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_1));
			strEJNLData += strTemp;

			//  6. [CST1] Rejected Notes
			strTemp.Format(L"^%6.6d", TempRejectCount1);
			strEJNLData += strTemp;

			//  7. [CST1] Jam Notes
			strTemp.Format(L"^%6.6d", TempJamCount1);
			strEJNLData += strTemp;

			//  8. [CST1] Remaining Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1));
			strEJNLData += strTemp;

			//  9. [CST1] Denomination
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_1));
			strEJNLData += strTemp;
		}

		TempRejectCount2 =  m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_2)		-
							m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_2)			-	
							m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_2)	-
							m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_2);

		if (TempRejectCount2 >= 0)
			TempRejectCount2 = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_2);
		else
			TempRejectCount2 += m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_2);
		if (TempRejectCount2 < 0)
			TempRejectCount2 = 0;

		TempJamCount2 = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_2)		-
						m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_2)			-	
						m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_2)	-
						TempRejectCount2;
		if (TempJamCount2 < 0)
			TempJamCount2 = 0;

		if (TranCode == TC_CSTTOTAL || TranCode == TC_TRIALCSTTOTAL)
		{
			// 10. [CST2] Initial Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_2));
			strEJNLData += strTemp;

			// 11. [CST2] Dispensed Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_2));
			strEJNLData += strTemp;

			// 12. [CST2] Rejected Notes
			strTemp.Format(L"^%6.6d", TempRejectCount2);
			strEJNLData += strTemp;

			// 13. [CST2] Jam Notes
			strTemp.Format(L"^%6.6d", TempJamCount2);
			strEJNLData += strTemp;

			// 14. [CST2] Remaining Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_2));
			strEJNLData += strTemp;

			// 15. [CST2] Denomination
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_2));
			strEJNLData += strTemp;
		}

		TempRejectCount3 =	m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_3)		-
							m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_3)			-	
							m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_3)	-
							m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_3);
		if (TempRejectCount3 >= 0)
			TempRejectCount3 = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_3);
		else
			TempRejectCount3 += m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_3);
		if (TempRejectCount3 < 0)
			TempRejectCount3 = 0;

		TempJamCount3 = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_3)		-
						m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_3)			-	
						m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_3)	-
						TempRejectCount3;
		if (TempJamCount3 < 0)
			TempJamCount3 = 0;

		if (TranCode == TC_CSTTOTAL || TranCode == TC_TRIALCSTTOTAL)
		{
			// 16. [CST3] Initial Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_3));
			strEJNLData += strTemp;

			// 17. [CST3] Dispensed Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_3));
			strEJNLData += strTemp;

			// 18. [CST3] Rejected Notes
			strTemp.Format(L"^%6.6d", TempRejectCount3);
			strEJNLData += strTemp;

			// 19. [CST3] Jam Notes
			strTemp.Format(L"^%6.6d", TempJamCount3);
			strEJNLData += strTemp;

			// 20. [CST3] Remaining Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_3));
			strEJNLData += strTemp;

			// 21. [CST3] Denomination
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_3));
			strEJNLData += strTemp;
		}

		TempRejectCount4 =	m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_4)		-
							m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_4)			-	
							m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_4)	-
							m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_4);
		if (TempRejectCount4 >= 0)
			TempRejectCount4 = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_4);
		else
			TempRejectCount4 += m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_4);
		if (TempRejectCount4 < 0)
			TempRejectCount4 = 0;

		AddPrintData(strPrintData, L"%-17.17s :  %4.4s,%4.4s,%4.4s,%4.4s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_088),	// [#2160] CA KMK 2012.11.27
												Int2Asc(TempRejectCount1), Int2Asc(TempRejectCount2), Int2Asc(TempRejectCount3), Int2Asc(TempRejectCount4));

		TempJamCount4 = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_4)		-
						m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_4)			-	
						m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_4)	-
						TempRejectCount4;
		if (TempJamCount4 < 0)
			TempJamCount4 = 0;

		if (TranCode == TC_CSTTOTAL || TranCode == TC_TRIALCSTTOTAL)
		{
			// 22. [CST4] Initial Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_4));
			strEJNLData += strTemp;

			// 23. [CST4] Dispensed Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_4));
			strEJNLData += strTemp;

			// 24. [CST4] Rejected Notes
			strTemp.Format(L"^%6.6d", TempRejectCount4);
			strEJNLData += strTemp;

			// 25. [CST4] Jam Notes
			strTemp.Format(L"^%6.6d", TempJamCount4);
			strEJNLData += strTemp;

			// 26. [CST4] Remaining Notes
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_4));
			strEJNLData += strTemp;

			// 27. [CST4] Denomination
			strTemp.Format(L"^%6.6d", m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_4));
			strEJNLData += strTemp;
		}

		AddPrintData(strPrintData, L"%-17.17s :  %4.4s,%4.4s,%4.4s,%4.4s",	m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_027),	// [#2160] CA KMK 2012.11.27
													Int2Asc(TempJamCount1), Int2Asc(TempJamCount2), Int2Asc(TempJamCount3), Int2Asc(TempJamCount4));

		AddPrintData(strPrintData, L"%-17.17s :  %4.4s,%4.4s,%4.4s,%4.4s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_028),	// [#2160] CA KMK 2012.11.27
													Int2Asc(m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_1)),
													Int2Asc(m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_2)),
													Int2Asc(m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_3)),
													Int2Asc(m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_4)));
		AddPrintData(strPrintData, L"........................................");

		// INITIAL AMOUNT
		int nAmt = (m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_1) * m_pDevCmn->FirstCSTValue)
				+	(m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_2) * m_pDevCmn->SecondCSTValue)
				+	(m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_3) * m_pDevCmn->ThirdCSTValue)
				+	(m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_4) * m_pDevCmn->FourthCSTValue);
		strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nAmt*100)));
		AddPrintData(strPrintData, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_029), strTemp);	// [#2160] CA KMK 2012.11.27

		// REMAINING AMOUNT
		int nAmt2 = (m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1) * m_pDevCmn->FirstCSTValue)
				+	(m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_2) * m_pDevCmn->SecondCSTValue)
				+	(m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_3) * m_pDevCmn->ThirdCSTValue)
				+	(m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_4) * m_pDevCmn->FourthCSTValue);
		strTemp.Format(L"%s%s", GetCurrencySymbol(),MakeMoneyCent(Int2Asc(nAmt2*100)));
		AddPrintData(strPrintData, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_030), strTemp);	// [#2160] CA KMK 2012.11.27

		if (TranCode == TC_CSTTOTAL || TranCode == TC_TRIALCSTTOTAL)
		{
			// 28. Non-Cash Value
			strTemp.Format(L"^%06.6d", 0);
			strEJNLData += strTemp;

			// 29. Non-Cash Type
			strTemp.Format(L"^%012.12d", 0);
			strEJNLData += strTemp;
		}
	}
	// [#2241] AU KMK 2014.01.08 CST Total 명세표 수정 (호주 only)
#endif
	// end of [#2241]

	if (TranCode == TC_CSTTOTAL)
	{
		if (nActUser == 1)
			m_pDevCmn->m_JNLMgr.Save(REMOTE_CST_TOTAL, strEJNLData);
		else
			m_pDevCmn->m_JNLMgr.Save(CST_TOTAL, strEJNLData);
	}
	else if (TranCode == TC_TRIALCSTTOTAL)
	{
		m_pDevCmn->m_JNLMgr.Save(TRIALCST_TOTAL, strEJNLData);
	}

	if (nActUser == 0 && m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)		// by OP
		m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT);

	return T_OK;
}

// [#2220] AU KMK 2014.02.06 호주 Set Denomination 명세표 함수 추가 (저널과 동일 형식)
#if (AU_VERSION)
int CTranCmn::LIB_DenominationPrintResult(int nArrOldDeno[])
{
	NHDEBUG(DBG_CALL, (_T("***TranBizLib***[CTranCmn::LIB_DenominationPrintResult]\n")));

	int			i = 0, j = 0;
	CString		strTemp, strTemp1;
	CTime		CurTime = CTime::GetCurrentTime();
	CString		strPrintData;

	// HEADER
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"========================================");
		for (i = 0; i < COUNT_CE_PRT_KINDCODE; i++)
		{
			if (g_CESupportPRTKindCode[i].szKindCode == m_pDevCmn->strJnlKindCode)
			{
				AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportPRTKindCode[i].szDescription));		// [#2160] CA KMK 2012.11.27
				break;
			}
		}
		AddPrintData(strPrintData, L"========================================");
	}

	// TODAY
	//------------------------------------------------------------------------------------------------
	{
		strTemp.Format(L"%s %02.2d:%02.2d:%02.2d",		// [#2] NH JSW 2008.03.03		
			GetCmnLocalDate(CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear()),
			CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
		AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_002), strTemp);		// [#2160] CA KMK 2012.11.27
	}

	// TERMINAL INFO
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));

		if(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_002), MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID));	// [#2160] CA KMK 2012.11.27
		else
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_003), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));		// [#2160] CA KMK 2012.11.27
	}

	// CASSETTE INFO
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"----------------------------------------");
		AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_004));	// [#2160] CA KMK 2012.11.27
		AddPrintData(strPrintData, L"----------------------------------------");

		// OLD DENOMINATION
		AddPrintData(strPrintData, L"%-13.13s%-6.6s: %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_031), GetCurrencySymbol(6,1),
			nArrOldDeno[0], nArrOldDeno[1], nArrOldDeno[2], nArrOldDeno[3]);

		// NEW DENOMINATION
		AddPrintData(strPrintData, L"%-13.13s%-6.6s: %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_032), GetCurrencySymbol(6,1),
			m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_1),
			m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_2),
			m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_3),
			m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_4));
	}

	m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT);

	return T_OK;
}
#endif	// #if (AU_VERSION)
// end of [#2220]


int CTranCmn::LIB_PrintADDCash(int PrintDevice)
{
	NHDEBUG(DBG_CALL, (_T("***TranBizLib***[CTranCmn::LIB_PrintADDCash]\n")));

	int			i = 0, j = 0;
	int			nRemainCST1, nRemainCST2, nRemainCST3, nRemainCST4;
	int			nAddCST1, nAddCST2, nAddCST3, nAddCST4;
	CString		strTemp, strTemp1;
	CTime		CurTime = CTime::GetCurrentTime();
	CString		strPrintData;

	// Get Add CST Count
	nAddCST1 = m_pDevCmn->nAdditionCST1;
	nAddCST2 = m_pDevCmn->nAdditionCST2;
	nAddCST3 = m_pDevCmn->nAdditionCST3;
	nAddCST4 = m_pDevCmn->nAdditionCST4;

	// Get Remain CST Count
	// [#2270] AU KSK 2014.05.26
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
	{
		nRemainCST1 = m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1) - nAddCST1;
		nRemainCST2 = m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_2) - nAddCST2;
		nRemainCST3 = m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_3) - nAddCST3;
		nRemainCST4 = m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_4) - nAddCST4;
	}
	else
	{
		nRemainCST1 = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP);
		nRemainCST2 = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST2_CURRENTCOUNT_AP);
		nRemainCST3 = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST3_CURRENTCOUNT_AP);
		nRemainCST4 = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST4_CURRENTCOUNT_AP);
	}
	// end of [#2270]

	// HEADER
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"========================================");
		for (i = 0; i < COUNT_CE_PRT_KINDCODE; i++)
		{
			if (g_CESupportPRTKindCode[i].szKindCode == m_pDevCmn->strJnlKindCode)
			{
				AddPrintData(strPrintData, L"%s", m_pDevCmn->fstrSCR_GetStringFromTextID(g_CESupportPRTKindCode[i].szDescription));		// [#2160] CA KMK 2012.11.27
				break;
			}
		}
		AddPrintData(strPrintData, L"========================================");
	}
	
	//  START TIME & TODAY
	//------------------------------------------------------------------------------------------------
	{
		strTemp = MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_LASTADDCASHTIME);
		if (strTemp.GetLength() >= 14)
		{
			strTemp1.Format(L"%s %2.2s:%2.2s:%2.2s",	// [#2] NH JSW 2008.03.03
				GetCmnLocalDate(strTemp.Left(2), strTemp.Mid(2,2), strTemp.Mid(4,4)),
				strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_030), strTemp1);		// [#2160] CA KMK 2012.11.27
		}
		else
		{
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_030), strTemp);		// [#2160] CA KMK 2012.11.27
		}

		strTemp.Format(L"%s %02.2d:%02.2d:%02.2d",		// [#2] NH JSW 2008.03.03		
					GetCmnLocalDate(CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear()),
					CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
		AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_002), strTemp);		// [#2160] CA KMK 2012.11.27

		strTemp.Format(L"%02d%02d%04d%02d%02d%02d",			
						CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(),
						CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());

		MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_LASTADDCASHTIME, strTemp);
	}

	// TERMINAL INFO
	//------------------------------------------------------------------------------------------------
	{
	
		AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));

		if(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_002), MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID));	// [#2160] CA KMK 2012.11.27
		else
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_003), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));		// [#2160] CA KMK 2012.11.27
	}

	// [#2247] AU KMK 2014.01.07 ADD CASH 명세표 보완
#if (AU_VERSION)
	int nCstTotalAmount = 0;					// total amount of all cassettes

	for (int nCurCstIndex=CDU_CST_1; nCurCstIndex<=CDU_MAX_CST_COUNT; nCurCstIndex++)
	{
		// Denomination이 0인 Cassette의 정보는 출력하지 않는다
		if (m_pDevCmn->fnCDU_GetValueOfCash(nCurCstIndex) != 0)
		{
			int nCstRemainingCount, nCstRemainingAmount;		// remaining count/amount
			int nCstAdditionalCount, nCstAdditionalAmount;		// additional count/amount
			int nCstCurrentCount, nCstCurrentAmount;			// current count/amount
			int nCstDenomination;								// denomination

			switch (nCurCstIndex)	{			// get count information of this cassette
				case CDU_CST_1:
					nCstRemainingCount = nRemainCST1;
					nCstAdditionalCount = nAddCST1;
					break;
				case CDU_CST_2:
					nCstRemainingCount = nRemainCST2;
					nCstAdditionalCount = nAddCST2;
					break;
				case CDU_CST_3:
					nCstRemainingCount = nRemainCST3;
					nCstAdditionalCount = nAddCST3;
					break;
				case CDU_CST_4:
					nCstRemainingCount = nRemainCST4;
					nCstAdditionalCount = nAddCST4;
					break;
			}
			nCstCurrentCount = nCstRemainingCount + nCstAdditionalCount;			// current count = remaining + additional

			nCstDenomination = m_pDevCmn->fnCDU_GetValueOfCash(nCurCstIndex);		// denomination of this cassette

			nCstRemainingAmount = nCstDenomination * nCstRemainingCount;			// calculate each amounts
			nCstAdditionalAmount= nCstDenomination * nCstAdditionalCount;
			nCstCurrentAmount	= nCstDenomination * nCstCurrentCount;			
			nCstTotalAmount		+= nCstCurrentAmount;

			AddPrintData(strPrintData, L"----------------------------------------");
			strTemp.Format(L"DENOMINATION %s%d", GetCurrencySymbol(), nCstDenomination);
			AddPrintData(strPrintData, L"CASSETTE #%d %28.28s", nCurCstIndex, strTemp);
			AddPrintData(strPrintData, L"----------------------------------------");

// 			// REMAINING COUNT / REMAINING AMOUNT
			AddPrintData(strPrintData, L"REMAINING COUNT   = %20d", nCstRemainingCount);
			strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nCstRemainingAmount * 100)));
			AddPrintData(strPrintData, L"REMAINING AMOUNT  = %20.20s", strTemp);

			// additional items will be printed when cash is added
			if (nCstAdditionalCount != 0)
			{
				// ADDITIONAL COUNT / ADDITIONAL AMOUNT
				// -> ADDITION COUNT/AMOUNT 로 변경
				AddPrintData(strPrintData, L"ADDITION COUNT    = %20d", nCstAdditionalCount);
				strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nCstAdditionalAmount * 100)));
				AddPrintData(strPrintData, L"ADDITION AMOUNT   = %20.20s", strTemp);
				AddPrintData(strPrintData, L"........................................");

				// CURRENT COUNT / CURRENT AMOUNT
				// -> SUM TOTAL COUNT/AMOUNT 로 변경
				AddPrintData(strPrintData, L"SUM TOTAL COUNT   = %20d", nCstCurrentCount);
				strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nCstCurrentAmount * 100)));
				AddPrintData(strPrintData, L"SUM TOTAL AMOUNT  = %20.20s", strTemp);
			}
		}
	}
	// TOTAL AMOUNT (ALL CASSETTES)
	//------------------------------------------------------------------------------------------------
	strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nCstTotalAmount * 100)));
	AddPrintData(strPrintData, FIELD_DELIMITER);
	AddPrintData(strPrintData, L"ENTIRE AMOUNT     = %20.20s", strTemp);

#else
	// end of [#2247]

	// CASSETTE INFO
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"----------------------------------------");
		AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_004));	// [#2160] CA KMK 2012.11.27
		AddPrintData(strPrintData, L"----------------------------------------");
		AddPrintData(strPrintData, L"%-12.12s%-6.6s:  %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_023), GetCurrencySymbol(6,1),		// [#2160] CA KMK 2012.11.27
												m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_1),
												m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_2),
												m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_3),
												m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_4));
		AddPrintData(strPrintData, L"........................................");
		AddPrintData(strPrintData, L"%-17.17s :  %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_005), nRemainCST1, nRemainCST2, nRemainCST3, nRemainCST4);	// [#2160] CA KMK 2012.11.27
		AddPrintData(strPrintData, L"%-17.17s :  %4d,%4d,%4d,%4d", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_006), nAddCST1, nAddCST2, nAddCST3, nAddCST4);				// [#2160] CA KMK 2012.11.27
	}

	// [#2247] AU KMK 2014.01.07 호주 명세표 보완
#endif
	// end of [#2247]

	// [Win32-fixes] US ryan.payton 2023.09.13 SPR doesn't work. Need to print to the operator screen.
	#ifdef UNDER_CE
		m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT);
	#endif

	return T_OK;
}

// [#GLDV-3005] US Kook 2022.03.28 Side Car
int	CTranCmn::LIB_PrintCashInTotal_Demo(int nActUser)
{
	NHDEBUG(DBG_CALL, (_T(" \n")));

	int			i = 0, j = 0;
	CString		strTemp, strTemp1, strEJNLData;
	CTime		CurTime = CTime::GetCurrentTime();
	CString		strPrintData;

	// HEADER
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"========================================");

		switch (TranCode)
		{
		case TC_CASHIN_CSTTOTAL:
			AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_CASHIN_TOTAL));
			break;
		case TC_TRIAL_CASHIN_CSTTOTAL:
			AddPrintData(strPrintData, m_pDevCmn->fstrSCR_GetStringFromTextID(T_JNL_TRIALCASHIN_TOTAL));
			break;
		}

		AddPrintData(strPrintData, L"========================================");
	}

	// 1. START TIME
	//------------------------------------------------------------------------------------------------
	{
		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_CASHINTOTAL_TIME);
		if (strTemp.GetLength() >= 14)
		{
			strTemp1.Format(L"%s %2.2s:%2.2s:%2.2s",
				GetCmnLocalDate(strTemp.Left(2), strTemp.Mid(2,2), strTemp.Mid(4,4)),
				strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_030), strTemp1);
			if (TranCode == TC_CASHIN_CSTTOTAL || TranCode == TC_TRIAL_CASHIN_CSTTOTAL)
			{
				strTemp1.Format(L"%2.2s/%2.2s/%4.4s %2.2s:%2.2s:%2.2s",
					strTemp.Left(2), strTemp.Mid(2,2), strTemp.Mid(4,4),
					strTemp.Mid(8,2), strTemp.Mid(10,2), strTemp.Mid(12,2));
				strEJNLData += strTemp1;
			}
		}
		else
		{
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_030), strTemp);
			if (TranCode == TC_CASHIN_CSTTOTAL || TranCode == TC_TRIAL_CASHIN_CSTTOTAL)
			{
				strTemp1.Format(L"%19.19s", strTemp);
				strEJNLData += strTemp1;
			}
		}
	}

	// 2. TODAY
	//------------------------------------------------------------------------------------------------
	{
		strTemp.Format(L"%s %02.2d:%02.2d:%02.2d",
			GetCmnLocalDate(CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear()),
			CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
		AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_CLERK_002), strTemp);

		if (TranCode == TC_CASHIN_CSTTOTAL)
		{
			strTemp.Format(L"%02d%02d%04d%02d%02d%02d",			
				CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(),
				CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());	
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_CASHINTOTAL_TIME, strTemp);
		}

		strTemp.Format(L"%02.2d/%02.2d/%04.4d %02.2d:%02.2d:%02.2d",
			CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetYear(),
			CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());

		strEJNLData += L"^";
		strEJNLData += strTemp;
	}

	// 3. TERMINAL INFO
	//------------------------------------------------------------------------------------------------
	{
		AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));

		strTemp.Format(L"^%15.15s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
		strEJNLData += strTemp;

		if(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_002), MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID));
		else
			AddPrintData(strPrintData, L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_003), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));
	}


	// 4. CASHIN INFO
	//------------------------------------------------------------------------------------------------
	// 1234567890123456789012345678901234567890
	// ----------------------------------------
	// DENOMINATION($)   :                COUNT
	// ----------------------------------------
	//                 1 :                    0
	//                 5 :                    0
	//                10 :                    0
	//                50 :                    0
	//               100 :                    0
	// ........................................
	// TOTAL AMOUNT      =                $0.00
	{
		AddPrintData(strPrintData, L"----------------------------------------");
		AddPrintData(strPrintData, L"%-12.12s%-6.6s: %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_023), GetCurrencySymbol(6, 1), m_pDevCmn->fstrSCR_GetStringFromTextID(T_COUNT));
		AddPrintData(strPrintData, L"----------------------------------------");

		strEJNLData += L"^";

		int nCashInTotalAmount = 0;
		for (int itemId=0; itemId<m_pDevCmn->m_nBnaNumberOfItem; itemId++)
		{
			if (m_pDevCmn->m_nBnaItemValue[itemId] == 0)
				continue;

			strTemp.Format(L"%d|%d,", m_pDevCmn->m_nBnaItemValue[itemId], m_pDevCmn->m_nBnaItemCount[itemId]);
			strEJNLData += strTemp;

			AddPrintData(strPrintData, L"%17d : %20d", m_pDevCmn->m_nBnaItemValue[itemId], m_pDevCmn->m_nBnaItemCount[itemId]);
			
			if (m_pDevCmn->m_nBnaItemValue[itemId] > 0)
				nCashInTotalAmount += (m_pDevCmn->m_nBnaItemValue[itemId] * m_pDevCmn->m_nBnaItemCount[itemId]);
		}
		
		AddPrintData(strPrintData, L"........................................");

		// TOTAL AMOUNT
		strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyCent(Int2Asc(nCashInTotalAmount*100)));
		AddPrintData(strPrintData, L"%-17.17s = %20.20s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_RECE_012), strTemp);

		strTemp.Format(L"^%d", nCashInTotalAmount);
		strEJNLData += strTemp;
	}

	if (TranCode == TC_CASHIN_CSTTOTAL)
	{
		//if (nActUser == 1)
		//	m_pDevCmn->m_JNLMgr.Save(REMOTE_CST_TOTAL, strEJNLData);
		//else
			m_pDevCmn->m_JNLMgr.Save(CASHIN_TOTAL, strEJNLData);
	}
	else if (TranCode == TC_TRIAL_CASHIN_CSTTOTAL)
	{
		m_pDevCmn->m_JNLMgr.Save(TRIALCASHIN_TOTAL, strEJNLData);
	}

	if (nActUser == 0 && m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)		// by OP
		m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT);

	return T_OK;
}
// end of [#GLDV-3005]


// keymode reference table
/*---------------------------------------------------------------------------------------------------------------------
keymode,									key1,			key2,			key3,			key4			key5
-----------------------------------------------------------------------------------------------------------------------
0:Non Unique Key, DES, Single Master Key	Working Key		-				-				-				-
1:Non Unique Key, DES, Dual Master Key		Working Key		C-Key			-				-				-
2:Unique Key, DES							Working Key		-				-				-				-
3:Non Unique Key, TDES						Working Key1	Working Key2	Working Key3	-				-
4:Unique Key, TDES							Working Key1	Working Key2	Working Key3	-				-
5:Non Unique Key, Macing					Working Key		-				-				Macing Key		-
6:Unique Key, Macing						Working Key		-				-				Macing Key		-
7:TDES, Macing								Working Key1	Working Key2	Working Key3	Macing Key		-
11:TDES, TMacing							Working Key1	Working Key2	Working Key3	Macing key1		Macing Key2
-----------------------------------------------------------------------------------------------------------------------
key load 순서: key5 -> key4 -> key3 -> key2 -> key1(사용하지 않는 key는 제외)
*/
//[#4] NH PSC 2008.03.10 MAC working load function 수정
//[#408] NH AIREAT 2008.08.26 - error 시 에러값을 return, 성공시 T_OK return.
//int CTranCmn::LIB_LoadWorkingKey()
int CTranCmn::LIB_LoadWorkingKey(int nMasterKeyName)		// [#2181] US KSK 2013.03.11
{
	NHDEBUG(DBG_CALL, (_T("***TranBizLib***[CTranCmn::LIB_LoadWorkingKey]\n")));

	int nResult = 0; // [#2027] NH KJW 2011.03.14 CodeSonar 지적사항 대책
	int nKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);

	// [#554] KSK 2009.08.11 512K시 LoadWorkingKey 방식변경으로 인해 수정
	// 512K인 경우 KEY MODE가 잘못된 경우 처리는 사양협의 필요함
	if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) >= 0)
	{
		//[#679] SOOK Working Download 실패 저널 오류 수정 2010.11.26
		// 512K Load Working Key
		if (nKeyMode == KEYMODE_NON_UNIQ_DDES ||
			nKeyMode == KEYMODE_NON_UNIQ_SDES_MACING)
		{
			// Key Mode 1, 5
			// [#2181] US KSK 2013.03.11 Single은 현재 사용하지 않으므로 Dual Working Key Loading 미 구현함
			nResult = m_pDevCmn->fnPIN_LoadEncryptedKey(LOAD_DESKEY_USE, SINGLEDESKEY_WORKINGKEY_NAME, MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1), SINGLEDESKEY_NAME);
		}
		else 
		if (nKeyMode == KEYMODE_NON_UNIQ_TDES ||
			nKeyMode == KEYMODE_TDES_MACING	||
			nKeyMode == KEYMODE_TDES_TMACING)
		{
			// Key Mode 3, 7, 11
			// [#2181] US KSK 2013.03.31
//			nResult = m_pDevCmn->fnPIN_LoadEncryptedKey(LOAD_DESKEY_USE, TRIPLEDESKEY_WORKINGKEY_NAME, MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1) + MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2), TRIPLEDESKEY_NAME);
			if (nMasterKeyName == MASTERKEY_ATM)		// [#2185] US Justin 2013.05.06 MasterKeyName Definition
			{
				nResult = m_pDevCmn->fnPIN_LoadEncryptedKey(LOAD_DESKEY_USE, TRIPLEDESKEY_WORKINGKEY_NAME, MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1) + MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2), TRIPLEDESKEY_NAME);
			}
			else if (nMasterKeyName == MASTERKEY_DUALHOST)
			{
				nResult = m_pDevCmn->fnPIN_LoadEncryptedKey(LOAD_DESKEY_USE, TRIPLEDESKEY_2ND_WORKINGKEY_NAME, MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1) + MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2), TRIPLEDESKEY_2ND_PIN_NAME);
			}
			else if (nMasterKeyName == MASTERKEY_LIBERTYX)
			{
				nResult = m_pDevCmn->fnPIN_LoadEncryptedKey(LOAD_DESKEY_USE, LIBERTYX_WORKINGKEY_NAME, MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1) + MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2), LIBERTYXKEY_NAME);
			}
			// end of [#2181]
		}
		else if (nKeyMode == KEYMODE_TR31 || nKeyMode == KEYMODE_TR31_MACING || nKeyMode == KEYMODE_TR31_TMACING)
		{
			if (nMasterKeyName == MASTERKEY_ATM)
			{
				nResult = m_pDevCmn->fnPIN_TR34_ImportKeyBlock(TR31_ATM_WORKINGKEY_NAME, TRIPLEDESKEY_NAME, MemGetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_TR31_WKB));
			}
			else if (nMasterKeyName == MASTERKEY_DUALHOST)
			{
				nResult = m_pDevCmn->fnPIN_TR34_ImportKeyBlock(TR31_DUALHOST_WORKINGKEY_NAME, TRIPLEDESKEY_2ND_PIN_NAME, MemGetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_TR31_WKB));
			}
			else if (nMasterKeyName == MASTERKEY_LIBERTYX)
			{
				nResult = m_pDevCmn->fnPIN_TR34_ImportKeyBlock(TR31_LIBERTYX_WORKINGKEY_NAME, LIBERTYXKEY_NAME, MemGetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_TR31_WKB));
			}
		}

		// MAC Load Working Key
		if (nKeyMode == KEYMODE_NON_UNIQ_SDES_MACING ||
			nKeyMode == KEYMODE_TDES_MACING)
		{
			// Key Mode 1
			nResult = m_pDevCmn->fnPIN_LoadEncryptedKey(LOAD_MACKEY_USE, SINGLEMACKEY_WORKINGKEY_NAME, MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY4), SINGLEMACKEY_NAME);
		}
		else if (nKeyMode == KEYMODE_TDES_TMACING)
		{
			// Key Mode 7, 11
			if (RegGetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"TMACOption") == 0)	// NZ , AU_A, AU_C 인 경우
				nResult = m_pDevCmn->fnPIN_LoadEncryptedKey(LOAD_MACKEY_USE, TRIPLEMACKEY_WORKINGKEY_NAME, MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY4) + MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_APP_DESKEY5), TRIPLEDESKEY_NAME);	// KeyMgr과 협의한 항목임
			else	// CA인 경우
				nResult = m_pDevCmn->fnPIN_LoadEncryptedKey(LOAD_MACKEY_USE, TRIPLEMACKEY_WORKINGKEY_NAME, MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY4) + MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_APP_DESKEY5), TRIPLEMACKEY_NAME);
		}
		else if (nKeyMode == KEYMODE_TR31_MACING || nKeyMode == KEYMODE_TR31_TMACING)
		{
			if (nMasterKeyName == MASTERKEY_ATM)		// [#2185] US Justin 2013.05.06 MasterKeyName Definition
			{
				nResult = m_pDevCmn->fnPIN_TR34_ImportKeyBlock(TR31_ATM_TMACWORKINGKEY_NAME, TRIPLEMACKEY_NAME, MemGetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_TR31_MKB));
			}
			else if (nMasterKeyName == MASTERKEY_DUALHOST)
			{
				nResult = m_pDevCmn->fnPIN_TR34_ImportKeyBlock(TR31_DUALHOST_TMACWORKINGKEYNAME, TRIPLEMACKEY_NAME, MemGetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_TR31_MKB));
			}
			else if (nMasterKeyName == MASTERKEY_LIBERTYX)
			{
				nResult = m_pDevCmn->fnPIN_TR34_ImportKeyBlock(TR31_LTX_TMACWORKINGKEY_NAME, TRIPLEMACKEY_NAME, MemGetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_TR31_MKB));
			}
		}

		if (nResult != 0)
		{
			m_pDevCmn->TranResult = FALSE;
			return nResult;
		}
		//end of [#679]
	}
	else
	{
		// Single DES Working Key Load(keymode: 0, 1, 2)
		// [#554] KSK 2009.08.10 m_KeyMode삭제
		if (nKeyMode == KEYMODE_NON_UNIQ_SDES || 
			nKeyMode == KEYMODE_NON_UNIQ_DDES || 
			nKeyMode == KEYMODE_UNIQ_SDES)
		{
			if(nKeyMode == KEYMODE_NON_UNIQ_DDES)		// keymode: 1
			{
				nResult = m_pDevCmn->fnPIN_LoadKey(L"K2", MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2));
				if (nResult != 0)
				{
					m_pDevCmn->TranResult = FALSE;
					return nResult;
				}
			}

			// keymode: 0, 1, 2
			nResult = m_pDevCmn->fnPIN_LoadKey(L"K1", MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1));
			if (nResult != 0)
			{
				m_pDevCmn->TranResult = FALSE;
				return nResult;
			}
			else
				m_pDevCmn->TranResult = TRUE;
		}
		else	// Triple DES Working Key Load(keymode: 3, 4)
		if (nKeyMode == KEYMODE_NON_UNIQ_TDES || nKeyMode == KEYMODE_UNIQ_TDES)
		{
			nResult = m_pDevCmn->fnPIN_LoadKey(L"K3", MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY3));
			if (nResult != 0)
			{
				m_pDevCmn->TranResult = FALSE;
				return nResult;
			}

			nResult = m_pDevCmn->fnPIN_LoadKey(L"K2", MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2));
			if (nResult != 0)
			{
				m_pDevCmn->TranResult = FALSE;
				return nResult;
			}

			nResult = m_pDevCmn->fnPIN_LoadKey(L"K1", MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1));
			if (nResult != 0)
			{
				m_pDevCmn->TranResult = FALSE;
				return nResult;
			}
		}
		else	// Macing key load(keymode: 5, 6, 7, 11)
		if (nKeyMode == KEYMODE_NON_UNIQ_SDES_MACING	|| 
			nKeyMode == KEYMODE_UNIQ_SDES_MACING		|| 
			nKeyMode == KEYMODE_TDES_MACING			|| 
			nKeyMode == KEYMODE_TDES_TMACING)
		{
			if(nKeyMode == KEYMODE_TDES_TMACING)		// load Macing key2(keymode: 11)
			{
				nResult = m_pDevCmn->fnPIN_LoadKey(L"K5", MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_APP_DESKEY5));
				if (nResult != 0)
				{
					m_pDevCmn->TranResult = FALSE;
					return nResult;
				}
			}
			
			// load Macing key1
			nResult = m_pDevCmn->fnPIN_LoadKey(L"K4", MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY4));
			if (nResult != 0)
			{
				m_pDevCmn->TranResult = FALSE;
				return nResult;
			}
			
			// TDES, TMacing working key3, working key2 load(keymode: 7, 11)
			if(nKeyMode == KEYMODE_TDES_MACING || nKeyMode == KEYMODE_TDES_TMACING)
			{
				nResult = m_pDevCmn->fnPIN_LoadKey(L"K3", MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY3));
				if (nResult != 0)
				{
					m_pDevCmn->TranResult = FALSE;
					return nResult;
				}
				
				nResult = m_pDevCmn->fnPIN_LoadKey(L"K2", MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2));
				if (nResult != 0)
				{
					m_pDevCmn->TranResult = FALSE;
					return nResult;
				}
			}
			
			// working key1 load
			nResult = m_pDevCmn->fnPIN_LoadKey(L"K1", MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1));
			if (nResult != 0)
			{
				m_pDevCmn->TranResult = FALSE;
				return nResult;
			}
		}
	}

	return T_OK;
}
// end of [#4]

int CTranCmn::LIB_MainMenuDeviceEvent(int CheckTime)
{
	// [#2325] NH KSK 2015.01.22 RFID Event Check하도록 추가
	//if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_MCU))
	//{
	//	if ((m_pDevCmn->DeviceTran & DEV_MCU) == m_pDevCmn->DeviceTran)
	//	{
	//		NHDEBUG(DBG_CALL, (_T("***TranBizLib***CTranCmn::LIB_MainMenuDeviceEvent() return T_OK (%d) \n"), m_pDevCmn->DeviceTran));
	//		return T_OK;
	//	}
	//}
	if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_MCU))
		return DEV_MCU;
	else if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_RFID))
		return DEV_RFID;
	// end of [#2325]
	// [#GLDV-3005] US Kook 2022.01.06 Support Side Car
	else if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_BCR))
		return DEV_BCR;
	// end of [#GLDV-3005]

	return T_TIMEOVER;
}

/*****************************************************************************
    # 8자리의 Ramdom한 문자열 생성 (1~9, A~F)
*****************************************************************************/
CString	CTranCmn::LIB_MakeRandomMac()
{
	NHDEBUG(DBG_CALL, (_T("***TranBizLib***[CTranCmn::LIB_MakeRandomMac]\n")));

    CString JArray;
    int HexDigit = 0;

    JArray.Empty();

    for (int i=0 ;  i < 8 ; i++) {
        HexDigit = rand()%16;
        if(HexDigit == 0) JArray       += '0';
        else if(HexDigit == 1) JArray  += '1';
        else if(HexDigit == 2) JArray  += '2';
        else if(HexDigit == 3) JArray  += '3';
        else if(HexDigit == 4) JArray  += '4';
        else if(HexDigit == 5) JArray  += '5';
        else if(HexDigit == 6) JArray  += '6';
        else if(HexDigit == 7) JArray  += '7';
        else if(HexDigit == 8) JArray  += '8';
        else if(HexDigit == 9) JArray  += '9';
        else if(HexDigit == 10) JArray += 'A';
        else if(HexDigit == 11) JArray += 'B';
        else if(HexDigit == 12) JArray += 'C';
        else if(HexDigit == 13) JArray += 'D';
        else if(HexDigit == 14) JArray += 'E';
        else if(HexDigit == 15) JArray += 'F';
    }

    return JArray;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: LIB_EnhancedCouponDataProc()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Enhanced Coupon 
-------------------------------------------------------------------*/
int CTranCmn::LIB_EnhancedCouponDataProc(int bClear, int nMode)
{
	int i=0;
	int j=0;
	int k=0;
	CString strTemp, strTemp2;

	// Initialize variable
	m_strarrEnhancedCouponData[0].RemoveAll();
	m_strarrEnhancedCouponData[1].RemoveAll();
	m_strarrEnhancedCouponData[2].RemoveAll();
	m_strarrEnhancedCouponData[3].RemoveAll();
	m_strarrEnhancedCouponData[4].RemoveAll();
	m_strarrEnhancedCouponData[5].RemoveAll();

	if (bClear == TRUE)
		return TRUE;

	// OP의 PRINT ALL SETUP시에는 OPTION에 상관없이 DATA를 SET한다.
	if (nMode == FALSE)
	{
		for(i=0; i<6; i++)
		{
			// Enhanced Coupon Data가 있는 경우
			for(j=15; j>=0; j--)
			{
				strTemp = MemGetStr(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_TEXT1 + (i*16) + j);
				strTemp.Replace(L" ", L"");

				if (strTemp.GetLength() > 0)
				{
					// 하단의 Space를 제거한 Data를 Insert한다.
					for(k=0; k<=j; k++)
					{
						m_strarrEnhancedCouponData[i].Add(MemGetStr(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_TEXT1 + (i*16) + k));
					}
					break;
				}
			}
		}

		return FALSE;	// OP에서는 return이 의미가 없음
	}

	// ADA Mode시에는 지원 안함 (Wave File 추가로 인해)
	if(m_pDevCmn->fnSNS_GetEnhancedAudio())
		return FALSE;

	// 화면 Display시에는 미 지원
	//if (m_sUserSelection.nPrintReceipt == FALSE)			// [#2219] 2013.09.04 Justin Digital Receipt,  change Variable Name
	if( (m_sUserSelection.nPrintReceipt == FALSE)||(m_sUserSelection.nPrintReceipt == RCPT_QRCODE) ) // [#2274] NH Justin 2014.06.04 Digital Receipt Bug Fix (Enhanced coupon is not available when Digital receipt is chosen)
		return FALSE;

	if (!m_pDevCmn->fnAPL_CheckError())
		return FALSE;	// ERROR가 있는 경우 return

	if ( (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE) != TRANTYPE_WITHDRAWAL)&&
		 (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE) != TRANTYPE_DCC) )				// [#2150] US Justin 2012.10.09 Print Coupon on DCC Transaction too.
		return FALSE;	// 출금거래가 아니면 return

	// Enhanced Coupon 미 지원 시 return
	if (MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_AVAILABLE) == 0 ||
		MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_ENABLE) == 0)
		return FALSE;

	// Check En/Disable Flag
	for(i=0; i<6; i++)
	{
		// Use Time 설정 시
		if (MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_OPTION + i) == 2)
		{
			// 현재 시간대에 Print할 Coupon인지 Check
			SYSTEMTIME	localTime;
			GetLocalTime(&localTime);

			// 설정 시간 Check
			// make string:HHMMSS -> int:HHMMSS
			int nStartTime = ((MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_PRINT_START_TIME + i) * 10000) + 01);
			int nEndTime = (MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_PRINT_END_TIME + i) * 10000);
			int nCurTime = (localTime.wHour*10000) + (localTime.wMinute * 100) + localTime.wSecond;

			// Start Hour 00 01 ~ End Hour 00 00 일 경우 Print
			if (nStartTime > nEndTime)
			{
				// Start Time이 End Time보다 큰 경우
				if (!((nCurTime >= nEndTime) && (nCurTime <= nStartTime)))
				{
					// Enhanced Coupon Data가 있는 경우
					for(j=15; j>=0; j--)
					{
						strTemp = MemGetStr(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_TEXT1 + (i*16) + j);
						strTemp.Replace(L" ", L"");

						if (strTemp.GetLength() > 0)
						{
							// 하단의 Space를 제거한 Data를 Insert한다.
							for(k=0; k<=j; k++)
							{
								m_strarrEnhancedCouponData[i].Add(MemGetStr(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_TEXT1 + (i*16) + k));
							}
							break;
						}
					}
				}
			}
			else
			{
				// Start Time이 End Time보다 작은 경우
				if ((nCurTime >= nStartTime) && (nCurTime <= nEndTime))
				{
					// Enhanced Coupon Data가 있는 경우
					for(j=15; j>=0; j--)
					{
						strTemp = MemGetStr(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_TEXT1 + (i*16) + j);
						strTemp.Replace(L" ", L"");

						if (strTemp.GetLength() > 0)
						{
							// 하단의 Space를 제거한 Data를 Insert한다.
							for(k=0; k<=j; k++)
							{
								m_strarrEnhancedCouponData[i].Add(MemGetStr(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_TEXT1 + (i*16) + k));
							}							
							break;
						}
					}
				}
			}
		}
		else if (MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_OPTION + i) == 1)	// Always Print시
		{
			// Enhanced Coupon Data가 있는 경우
			for(j=15; j>=0; j--)
			{
				strTemp = MemGetStr(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_TEXT1 + (i*16) + j);
				strTemp.Replace(L" ", L"");

				if (strTemp.GetLength() > 0)
				{
					// 하단의 Space를 제거한 Data를 Insert한다.
					for(k=0; k<=j; k++)
					{
						m_strarrEnhancedCouponData[i].Add(MemGetStr(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_TEXT1 + (i*16) + k));
					}
					break;
				}
			}
		}
	}

	for(i=0; i<6; i++)
	{
		if (m_strarrEnhancedCouponData[i].GetSize() > 0)
			return TRUE;
	}

	return FALSE;
}

// // [#RWC6-16] Bitload 4 U. A new cryptocurrency feature for CE 6.0 ATMs
void CTranCmn::LIB_UserPopUpNotice(CString sICON, CString strUserMsg, int nTimeOut, int nStyle, bool busesound)
{
	// PreSet
	m_pDevCmn->fnSCR_DisplayPrevSet(122);

	// Set an Icon
	if(sICON.GetLength() > 0 )
		m_pDevCmn->fnSCR_DisplayString(1, sICON);

	// Set User Message
	if(strUserMsg.GetLength() > 0)
		if (nStyle <= 0)	// font size: 14
			m_pDevCmn->fnSCR_DisplayString(4, strUserMsg);
		else if (nStyle == 1)	// font size: 12	// [#RWC6-16] Bitload 4 U. A new cryptocurrency feature for CE 6.0 ATMs
			m_pDevCmn->fnSCR_DisplayString(5, strUserMsg);

	// ???? => Not defined in data file (removed ????) 
	m_pDevCmn->fnSCR_DisplayImage(1, FALSE);

	// Display
	m_pDevCmn->fnSCR_DisplayScreen(122);

	if (busesound == true)
	{
#if defined(_WIN32_WCE)
		sndPlaySound(L"\\ATM\\dingdingding.wav", SND_ASYNC);	// [RWC6-651] Audio Beep to return card Start  12/19/2023
#endif
	}

	// Wait for TimeOut
	if( nTimeOut > 0 )
		m_pDevCmn->fstrSCR_WaitTime(nTimeOut);
}
// End of [#RWC6-16]

// [#2487] AU KSK 2017.06.08
#if (AU_VERSION)
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: LIB_CalculateSurchargeModeNReuslt()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Display Surcharge Value at Idle Screen (APCA regulation)
-------------------------------------------------------------------*/
int CTranCmn::LIB_CalculateSurchargeModeNReuslt(int &nSurcharge1, int &nSurcharge2)
{
	CString strTemp;

	BOOL bWithSurchageMode = FALSE;
	BOOL bStandardSurchageMode = FALSE;
	BOOL bPercentSurchargeMode = FALSE;
	BOOL bBinSurchargeMode	 = FALSE;

	int nSurchargeMode = NO_DISPLAY_SURCHARGE;
	int nTempStdWithSurcharge = 0;
	int nTempStdBalSurcharge = 0;

	try
	{
		// Check Standard Surcharge Mode
		bWithSurchageMode = MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE);

		if (bWithSurchageMode == ENABLE)
		{
			nTempStdWithSurcharge = Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT));
			nTempStdBalSurcharge = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_INQUIRY_SURCHARGE);
		}

		if ((nTempStdWithSurcharge > 0) || nTempStdBalSurcharge > 0)
			bStandardSurchageMode = TRUE;

		// Check PercentSurcharge Mode
		if ((bWithSurchageMode == ENABLE) && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) == PERCENT_MODE))
			bPercentSurchargeMode = TRUE;

		// Check Bin Surcharge Mode
		if (m_pDevCmn->m_BINMgr.GetTotalBinCount() > 0)
		{
			if (m_pDevCmn->m_BINMgr.IsExistRegisterSurcharge() == TRUE)
				bBinSurchargeMode = TRUE;
		}

		// Case by Case별로 Surcharge 계산
		if (((bStandardSurchageMode == TRUE) && (bPercentSurchargeMode == FALSE) && (bBinSurchargeMode == FALSE)) ||
			((bStandardSurchageMode == TRUE) && (bPercentSurchargeMode == TRUE) && (bBinSurchargeMode == FALSE)))
		{
			nSurcharge1 = nTempStdWithSurcharge;
			nSurcharge2 = nTempStdBalSurcharge;

			if ((bStandardSurchageMode == TRUE) && (bPercentSurchargeMode == TRUE) && (bBinSurchargeMode == FALSE))
			{
				NVDump('O', 'D', "76", L"", L"PER SURCHARGE");
				nSurchargeMode = PERCENTAGE_SURCHARGE;
			}
			else
			{
				NVDump('O', 'D', "76", L"", L"STD SURCHARGE");
				nSurchargeMode = STANDARD_SURCHARGE;	// case 1
			}
		}
		else if (((bPercentSurchargeMode == FALSE) && (bBinSurchargeMode == TRUE)) ||
			     ((bStandardSurchageMode == TRUE) && (bPercentSurchargeMode == TRUE) && (bBinSurchargeMode == TRUE)))
		{
			int nMinimumSurcharge = 0;
			int nMaximumSurcharge = 0;

			// 출금 수수료와 조회수수료값을 가지고 Min / Max 설정
			nMinimumSurcharge = __min(nTempStdWithSurcharge, nTempStdBalSurcharge);
			nMaximumSurcharge = __max(nTempStdWithSurcharge, nTempStdBalSurcharge);

			// BinList에서 Get한 Min값 중 더 낮은 값을 Minimum으로 설정
			nMinimumSurcharge = __min(nMinimumSurcharge, m_pDevCmn->m_BINMgr.GetMinimumSurcharge());

			// BinList에서 Get한 Max값 중 더 높은 값을 Maximum으로 설정
			nMaximumSurcharge = __max(nMaximumSurcharge, m_pDevCmn->m_BINMgr.GetMaximumSurcharge());

			nSurcharge1 = nMinimumSurcharge;
			nSurcharge2 = nMaximumSurcharge;

			strTemp.Format(L"MIN:%d, MAX:%d", nSurcharge1, nSurcharge2);
			NVDump('O', 'D', "76", L"", strTemp);

			if ((nMinimumSurcharge == 0) && (nMaximumSurcharge == 0))
			{
				NVDump('O', 'D', "76", L"", L"NO DISP SURCHARGE");
				nSurchargeMode = NO_DISPLAY_SURCHARGE;
			}
			else if ((bStandardSurchageMode == TRUE) && (bPercentSurchargeMode == TRUE) && (bBinSurchargeMode == TRUE))
			{
				NVDump('O', 'D', "76", L"", L"PER_BIN SURCHARGE");
				nSurchargeMode = PERCENTAGE_N_BINLISTED_SURCHARGE;	// case 3
			}
			else
			{
				NVDump('O', 'D', "76", L"", L"BIN SURCHARGE");
				nSurchargeMode = BINLISTED_SURCHARGE;				// case 2
			}
		}
		else
		{
			nSurcharge1 = 0;
			nSurcharge2 = 0;

			strTemp.Format(L"MIN:%d, MAX:%d", nSurcharge1, nSurcharge2);
			NVDump('O', 'D', "77", L"", strTemp);

			// 그 이외의 경우는 Surcharge 미 표시
			nSurchargeMode = NO_DISPLAY_SURCHARGE;
		}
	}
	catch (CException* e)
	{
		nSurcharge1 = 0;
		nSurcharge2 = 0;

		strTemp.Format(L"MIN:%d, MAX:%d", nSurcharge1, nSurcharge2);
		NVDump('O', 'D', "78", L"", strTemp);

		// Exception 발생시 Surcharge No Display
		nSurchargeMode = NO_DISPLAY_SURCHARGE;
	}

	return nSurchargeMode;
}
#endif
// end of [#2487]
