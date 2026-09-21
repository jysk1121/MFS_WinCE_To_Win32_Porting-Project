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
 FUNCTION NAME: BIZ_STD2_SendHost()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD2_SendHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD2_SendHost]\n"));
	NVDump('O', 'H', "20", L"", L"(M)AP_SEND1");

	////////////////////////////////////////////////////////////////////////////
	//	Transaction Result Initialize
	m_pDevCmn->TranResult = FALSE;								// Host Result
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, FALSE);

	// [#12] NH PSC 2008.03.24 reason for reversal 초기값을 0으로 셋팅한다.
	if(TranCode != TC_REVERSAL)		// [#68] UK JSW 2008.05.16 수정
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 2);

/////////////////////////////////////////////////////////////////////////////
	if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER)
		m_pDevCmn->fnAPL_AddSerialNo(m_HostConfig);							// Add Serial No
	
	BIZ_STD2_MakeHostMsg();						// Send Host Make

	// [#2220] NH KMK 2014.01.27 APP_SHOW_MODE 관련 로직 미사용하므로 주석처리
// #ifdef APP_SHOW_MODE	// Local Mode에서 거래속도 향상을 위해 수정
// 		//Delay_Msg(5000);
// 		m_pDevCmn->fstrSCR_WaitTime(2);
// #endif
	// end of [#2220]

#ifdef APP_LOCAL_MODE
		m_pDevCmn->fstrSCR_WaitTime(1);
		m_pDevCmn->TranResult = TRUE;
		MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);
		return RES_OK;
#endif

	// [#2135] NH KSK 2012.05.07 Pin Block or Mac Result가 SPACE시에 거래 취소 후 저널 저장하도록 수정
	// PIN Block(16)이 SPACE 이거나 Mac Result(8)가 SPACE인 경우 거래 취소
	if (m_sUserSelection.strPassword.Left(16) == L"                " || m_strMacSendResult.Left(8) == L"        ")
	{
		// PIN 장애 발생 및 PIN SP 재기동 하도록 Flag 설정
		if (m_pDevCmn->nPINErrorFlag == WORKINGKEY_INIT)
			m_pDevCmn->nPINErrorFlag = PIN_MAC_BLANK_ERROR;

		m_pDevCmn->fnAPL_StackError(_T("9799904"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003006), DEV_PIN);
		return RES_HOST_SEND_ERR;
	}
	// end of [#2135]

	if (BIZ_SendData() != RES_OK)
	{
		// EOT 미수신시 Configuration / Inquery / Transfer에서는 정상처리함
		if (TranCode == TC_OPEN || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER)	// [#560] NH KSK 2009.8.20
		{
			if (m_pDevCmn->fstrNET_GetErrorCode().Left(5) == L"D2200")	
				return RES_OK;	// KSK 2009.8.24
		}

		if (m_pDevCmn->TranStatus == TRAN_TRAN || m_pDevCmn->TranStatus == TRAN_REVERSAL)
		{
			if ((TranCode == TC_WITHDRAWAL || TranCode == TC_REVERSAL) && 
				(m_pDevCmn->fstrNET_GetErrorCode().Left(5) == L"D2200"))
			{
				if (LIB_IsReversalCondition())				// [#439] NH AIREAT 2008.10.21
				{
					if (BIZ_RecvData() == RES_OK)
						BIZ_STD2_AnalHostData();
				
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
				}
			}

			m_pDevCmn->fnAPL_StackError(m_pDevCmn->fstrNET_GetErrorCode(), m_pDevCmn->fstrNET_GetErrorMsg(), DEV_NET);	// [#419] [NH] KSK 2008.9.16

			return RES_HOST_SEND_ERR;
		}
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD2_MakeHostMsg()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD2_MakeHostMsg()
{
	NHDEBUG(DBG_CALL, (L"***TranHostProc***CTranCmn::BIZ_STD2_MakeHostMsg() \n"));

	////////////////////////////////////////////
	// 1. Make Message
	BIZ_STD2_MakeHeader();			// Send Host Make Header
	BIZ_STD2_MakeBody();			// Send Host Make Message

	////////////////////////////////////////////
	// 2. Macing
	// [#4] NH PSC 2008.03.10 Mac result 계산결과 전문에 추가(CSP200은 모든 전문에 MACing 하는데, 지금 빠져있음.)
	// keymode: 5, 6, 7, 11 - 현재 standard 2는 TMAC 없음.
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	if(	MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)	// [#565] NH KSK 2009.8.20 TDES, TMAC 지원
	{
		if (TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_WITHDRAWAL || TranCode == TC_REVERSAL)
		{
			CString		strMacData;
			MakeUnPack(m_strSendData, strMacData);
			m_pDevCmn->fnPIN_MacingData(strMacData);

			// [#2135] NH KSK 2012.05.07 Mac Result가 SPACE인 경우 거래 취소 하도록 수정
//			strMacResult.Format(L"%8.8s", m_pDevCmn->fstrPIN_GetMacingData());	// KSK 2009.8.21 Mac Data가 없을 경우 Space로 송신하도록 수정
//			m_strSendData += FIELD_DELIMITER;
//			m_strSendData += strMacResult.Left(8);		// 앞에서 8자리만 짤라서 보낸다.

			m_strMacSendResult.Format(L"%8.8s", m_pDevCmn->fstrPIN_GetMacingData());
			m_strSendData += FIELD_DELIMITER;
			m_strSendData += m_strMacSendResult.Left(8);		// 앞에서 8자리만 짤라서 보낸다.
			// end of [#2135]
		}
	}
	// end of [#4]

	////////////////////////////////////////////
	// 3. Make Buffer
	memset(m_szTemp, NULL, sizeof(m_szTemp));
	WideToMulti(m_szTemp, m_strSendData, sizeof(m_szTemp));

	////////////////////////////////////////////
	// Initialize Send Buffer
	m_nSendLength = 0;												// Send Length
	//memset(m_arSendBuffer, 0, sizeof(m_arSendBuffer));				// Send Buffer
	memset(m_arSendBuffer, 0, NETBUF_SEND_SIZE);

	memcpy(m_arSendBuffer, m_szTemp, m_strSendData.GetLength());
	m_nSendLength = m_strSendData.GetLength();
	
	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD2_MakeHeader()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD2_MakeHeader()
{
	NHDEBUG(DBG_CALL, (L"***TranHostProc***CTranCmn::BIZ_STD2_MakeHeader() \n"));

	// 1. Record Format Set
	// Record Format = '5' Format Option 1, 'I' Format Option 0
	// V01.02.30 ADD EPS REVIEW10 [#89] KSK 2008.04.14
	if (!MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_MSGTYPE).CompareNoCase(MSG_CSP200_TYPE))
		m_sSTD2_CommReqHeader.RecordFormat_1 = L"I";
	else
		m_sSTD2_CommReqHeader.RecordFormat_1 = L"5";

	// 2. Application Type Set
	m_sSTD2_CommReqHeader.ApplicationType_1 = L"0";
	// 3. Message Delimiter Set
	m_sSTD2_CommReqHeader.MessageDelimiter_1 = L".";
	// 4. Authorization Host ID Set
	m_sSTD2_CommReqHeader.BankID_6.Format(L"%6.6s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));
	// 5. Terminal ID Set
	m_sSTD2_CommReqHeader.TerminalID_8.Format(L"%8.8s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));

	// Record Format = '5' Format Option 1(FS Missing), 'I' Format Option 0
	if (!MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_MSGTYPE).CompareNoCase(MSG_EPS_TYPE))
	{
		m_strSendData = m_sSTD2_CommReqHeader.RecordFormat_1		+
						m_sSTD2_CommReqHeader.ApplicationType_1		+
						m_sSTD2_CommReqHeader.MessageDelimiter_1	+
						m_sSTD2_CommReqHeader.BankID_6				+
						m_sSTD2_CommReqHeader.TerminalID_8;
	}
	else
	{
		m_strSendData = m_sSTD2_CommReqHeader.RecordFormat_1		+
						m_sSTD2_CommReqHeader.ApplicationType_1		+
						m_sSTD2_CommReqHeader.MessageDelimiter_1	+
						m_sSTD2_CommReqHeader.BankID_6;
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += m_sSTD2_CommReqHeader.TerminalID_8;
	}
	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD2_MakeBody()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD2_MakeBody()
{
	switch(TranCode)
	{
		case TC_INQUIRY:
		case TC_TRANSFER:
		case TC_WITHDRAWAL:
			BIZ_STD2_MakeTransMsg();
			break;

		case TC_REVERSAL:
			BIZ_STD2_MakeReversalMsg();
			break;

		case TC_TOTAL:
		case TC_TRIALTOTAL:
			BIZ_STD2_MakeTotalMsg();
			break;

		case TC_OPEN:
			BIZ_STD2_MakeConfigMsg();
			break;

		case TC_HEALTHCHK:
			BIZ_STD2_MakeHealthCheckMsg();
			break;

		case TC_DETAILHEALTHCHK:
			BIZ_STD2_MakeDetailHealthCheckMsg();
			break;
	}
	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD2_MakeTransMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD2_MakeTransMsg()
{
	// Request Type
	m_sSTD2_TranReq.RequestType_2 = L"85";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_TranReq.RequestType_2;

	// 8. Time Variant Number Set (If Mac used)
	// [#4] NH PSC 2008.03.10 random Mac 추가. keymode: 5, 6, 7, 11
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)		// [#565] NH KSK 2009.8.20 TDES, TMAC 지원
	{
		m_sSTD2_TranReq.TimeVariantNo_8.Format(L"%s", LIB_MakeRandomMac());
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += m_sSTD2_TranReq.TimeVariantNo_8;
	}
	// end of [#4]

	// 10. Operation Code
	switch (TranCode)
	{
	case TC_WITHDRAWAL:
		m_sSTD2_TranReq.OperationCode_2 = TRANTYPE_WITHDRAWAL;
		break;
	case TC_INQUIRY:
		m_sSTD2_TranReq.OperationCode_2 = TRANTYPE_INQUIRY;
		break;
	case TC_TRANSFER:
		m_sSTD2_TranReq.OperationCode_2 = TRANTYPE_TRANSFER;
		break;
	default:
		break;
	}
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_TranReq.OperationCode_2;
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE, m_sSTD2_TranReq.OperationCode_2);

	// 11. Source Account
	if (m_SourceAccount == S_CHECKING)
		m_sSTD2_TranReq.SourceAccount_2 = L"CA";
	else
	if (m_SourceAccount == S_SAVINGS)
		m_sSTD2_TranReq.SourceAccount_2 = L"SA";
	else
	if (m_SourceAccount == S_CREDITCARD)
		m_sSTD2_TranReq.SourceAccount_2 = L"CR";

	m_strSendData += m_sSTD2_TranReq.SourceAccount_2;
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSFROMACCOUNT, m_sSTD2_TranReq.SourceAccount_2);

	// 12. Destination Account
	// Default
	m_sSTD2_TranReq.DestinationAccount_2 = m_sSTD2_TranReq.SourceAccount_2;	// 2006.03.10 PJH AT 5/3rd

	if (TranCode == TC_TRANSFER)
	{
		if (m_DestAccount == S_CHECKING)
			m_sSTD2_TranReq.DestinationAccount_2 = L"CA";
		else
		if (m_DestAccount == S_SAVINGS)
			m_sSTD2_TranReq.DestinationAccount_2 = L"SA";
		else
		if (m_DestAccount == S_CREDITCARD)
			m_sSTD2_TranReq.DestinationAccount_2 = L"CR";
	}
	m_strSendData += m_sSTD2_TranReq.DestinationAccount_2;
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTOACCOUNT, m_sSTD2_TranReq.DestinationAccount_2);

	// 13. Transaction Sequence Number
	m_sSTD2_TranReq.TransactionSequenceNo_4.Format(L"%4.4s", m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig));	// [#371] [US] KSK 2008.7.16 ATMINFO쪽 값을 참조하도록 수정
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_TranReq.TransactionSequenceNo_4;

	// 14. ISO CARD 1
	// [#2250] US Justin 2014.01.27 Enable Track1 => disable sending data to keep backward comatibility
	//if (m_sCardData.strISO1Data.GetLength() > 0)
	//	m_sSTD2_TranReq.Track1Data_79.Format(L"%%%s?", m_sCardData.strISO1Data.Left(77));
	// End of [#2250]
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_TranReq.Track1Data_79;
	MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_ISO1DATA, m_sSTD2_TranReq.Track1Data_79);

	// 15. ISO CARD 2
	if (m_sCardData.strISO2Data.GetLength() > 0)
	{
		if (m_sCardData.strISO2Data.GetLength() <= 37)
			m_sSTD2_TranReq.Track2Data_40.Format(L";%s?", m_sCardData.strISO2Data);
		else
			m_sSTD2_TranReq.Track2Data_40.Format(L";%38.38s?", m_sCardData.strISO2Data);
	}
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_TranReq.Track2Data_40;
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO2DATA, m_sSTD2_TranReq.Track2Data_40);

	// 16. ISO CARD 3
	if (m_sCardData.strISO3Data.GetLength() > 0)
		m_sSTD2_TranReq.Track3Data_106.Format(L";%s?", m_sCardData.strISO3Data.Left(106));
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_TranReq.Track3Data_106;
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO3DATA, m_sSTD2_TranReq.Track3Data_106);

	/************************************************************************/
	/* Make PIN Block                                                       */
	/************************************************************************/
	if (m_sUserSelection.strPassword.IsEmpty())		// [#11] NH KSK 2010.10.5	[#2021] US KSK 2011.02.14
	{
		/* Make PIN Block */
		m_pDevCmn->fnAPL_BuildPinBlockWithAccountNo(m_sCardData.strAccountNo);
		// End of [#2150]

		/* Check Build Pin Block Command */
		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);

		// [#2135] NH KSK 2012.05.07 Pin Block Result가 SPACE인 경우 거래 취소 하도록 수정
		/* Get PIN Block */
		m_sUserSelection.strPassword.Format(L"%16.16s", m_pDevCmn->fstrPIN_GetPinKeyData());
		// end of [#2135]
	}

	// 17. PIN Buffer
	m_sSTD2_TranReq.PINBuffer_16.Format(L"%16.16s", m_sUserSelection.strPassword.Left(16));
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_TranReq.PINBuffer_16;

	// 18. Transaction Amount
	m_sSTD2_TranReq.TransactionAmount_V.Format(L"%d", Asc2Int(m_sUserSelection.strMoney));
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_TranReq.TransactionAmount_V;
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT,	m_sSTD2_TranReq.TransactionAmount_V);	// [#169] [NH] KSK 2008.04.25

	// [#390] [NH] KSK 2008.8.1 RBS LYNK 특이사항
	// [#92] KSK 2008.04.18 add Percent surcharge
	// 19. Surcharge Amount
	// Surcharge Flag is set (Surcharge On)

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	m_sSTD2_TranReq.SurchargeAmount_V.Format(_T("%d"), SurchargeAmount);
#else
	// [#2150] US Justin 2012.10.02 Put Displayed Surcharge Amount
	/*
	if (TranCode == TC_WITHDRAWAL)
	{
		// [#417] [NH] KSK 2008.9.9 Surcharge Disable일 경우 Default값을 송신하도록 수정
		if ((MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEDISPLAY) == ENABLE) &&
			(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) == PERCENT_MODE)) //[#2136] US/CA PCS 2012.05.31
			// end of [#417]
		{
			int		nReqAmount			= 0;
			int		nPercent			= 0;
			int		nPercentSurcharge	= 0;
			int		nOrigineSurcharge	= 0;

			nReqAmount			= Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT))/100;	// [#169] [NH] KSK 2008.04.25
			nPercent			= MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_PERCENT);
			nOrigineSurcharge	= Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT));
			nPercentSurcharge	= nReqAmount * nPercent;

			if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_MANNER) == GREATER)
			{
				// GREATER SURCHARGE
				if (nPercentSurcharge >= nOrigineSurcharge)
					m_sSTD2_TranReq.SurchargeAmount_V.Format(_T("%d"), nPercentSurcharge);
				else
					m_sSTD2_TranReq.SurchargeAmount_V.Format(L"%d", nOrigineSurcharge);
			}
			else
			{
				// LESSER SURCHARGE
				if (nPercentSurcharge >= nOrigineSurcharge)
					m_sSTD2_TranReq.SurchargeAmount_V.Format(_T("%d"), nOrigineSurcharge);
				else
					m_sSTD2_TranReq.SurchargeAmount_V.Format(L"%d", nPercentSurcharge);
			}
		}
		else
			m_sSTD2_TranReq.SurchargeAmount_V.Format(L"%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT));
	}
	else
		m_sSTD2_TranReq.SurchargeAmount_V.Format(L"%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT));
	// end of [#92]
	// end of [#390]
	*/
	if(MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEDISPLAY) == ENABLE)
		m_sSTD2_TranReq.SurchargeAmount_V.Format(L"%d", m_nDisplayedSurchargeAmount);
	else
		m_sSTD2_TranReq.SurchargeAmount_V.Format(L"%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT));
	// End of [#2150]
#endif

	// save percent value
	m_sSTD2_TranReq.SurchargeAmount_V.TrimLeft(L" ");
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_TranReq.SurchargeAmount_V;
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT, m_sSTD2_TranReq.SurchargeAmount_V);					// [#304] NH AIREAT 08.06.15 - Reversal Surcharge 버그 수정.

	// 20. Surcharge Flag : Surcharge 화면을 뿌릴지 말지 결정하는 변수
	if (MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEDISPLAY))
		m_sSTD2_TranReq.SurchargeEnableFlag_1 = L"1";
	else
		m_sSTD2_TranReq.SurchargeEnableFlag_1 = L"0";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_TranReq.SurchargeEnableFlag_1;
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEFLAG, m_sSTD2_TranReq.SurchargeEnableFlag_1);

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD2_MakeReversalMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CTranCmn::BIZ_STD2_MakeReversalMsg()
{
	// Request Type
	m_sSTD2_ReversalReq.RequestType_2 = L"86";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_ReversalReq.RequestType_2;

	// [#4] NH PSC 2008.03.10 random Mac 추가. keymode: 5, 6, 7, 11
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)	// [#565] NH KSK 2009.8.20 TDES, TMAC 지원
	{
		m_sSTD2_ReversalReq.TimeVariantNo_8.Format(L"%s", LIB_MakeRandomMac());
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += m_sSTD2_ReversalReq.TimeVariantNo_8;
	}
	// end of [#4]
	// 9. Local Transaction Date
	m_sSTD2_ReversalReq.LocalTranDate_8 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE); // 2008-01-16 V01.02.25 SRC-19
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_ReversalReq.LocalTranDate_8;

	// 10. Local Transaction Time
	m_sSTD2_ReversalReq.LocalTranTime_6 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME); // 2008-01-16 V01.02.25 SRC-19
	m_strSendData += m_sSTD2_ReversalReq.LocalTranTime_6;

	// 11. Retrieval Reference Number
	m_sSTD2_ReversalReq.RetrievalRefNo_12 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRETRIEVALNUM);
	m_strSendData += m_sSTD2_ReversalReq.RetrievalRefNo_12;

	// 12. Requested Dollar Amount
	m_sSTD2_ReversalReq.RequestedDollarAmt_V = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT);	// [#169] [NH] KSK 2008.04.25
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_ReversalReq.RequestedDollarAmt_V;

	// 13. Dispensed Dollar Amount
	m_sSTD2_ReversalReq.DispensedDollarAmt_V = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT);	// [#169] [NH] KSK 2008.04.25
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_ReversalReq.DispensedDollarAmt_V;

	// 14. Surcharge Amount
	m_sSTD2_ReversalReq.SurchargeAmount_4 = MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT);
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_ReversalReq.SurchargeAmount_4;

	// 15. Surcharge Flag
	m_sSTD2_ReversalReq.SurchargeEnableFlag_1 = MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEFLAG);
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_ReversalReq.SurchargeEnableFlag_1;

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD2_MakeTotalMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD2_MakeTotalMsg()
{
	m_sSTD2_TotalReq.RequestType_2 = L"87";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_TotalReq.RequestType_2;

	// [#4] NH PSC 2008.03.10 random Mac 추가. keymode: 5, 6, 7
	if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING)
	{
		m_sSTD2_TotalReq.TimeVariantNo_8.Format(L"%s", LIB_MakeRandomMac());
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += m_sSTD2_TotalReq.TimeVariantNo_8;
	}
	// end of [#4]

	// 9. Reset Host Totals Flag
	m_sSTD2_TotalReq.ResetHostTotalFlag_1 = L"1";
	if (TranCode == TC_TRIALTOTAL)
		m_sSTD2_TotalReq.ResetHostTotalFlag_1 = L"0";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_TotalReq.ResetHostTotalFlag_1;

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD2_MakeConfigMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CTranCmn::BIZ_STD2_MakeConfigMsg()
{
	m_sSTD2_ConfigReq.RequestType_2 = L"88";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_ConfigReq.RequestType_2;

	// V01.02.30 ADD EPS REVIEW10 [#89] KSK 2008.04.14
	if (!MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_MSGTYPE).CompareNoCase(MSG_EPS_TYPE))
	{
		if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) != 0)
		{
			m_sSTD2_ConfigReq.KeyMode_1.Format(L"%d", MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE));
			m_strSendData += FIELD_DELIMITER;
			m_strSendData += m_sSTD2_ConfigReq.KeyMode_1;
		}
	}
	// end of V01.02.30 end of [#89]
	// 8. Mac Random Time Variant Data
	// [#4] NH PSC 2008.03.10 random Mac 추가. keymode: 5, 6, 7
	if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING)
	{
		m_sSTD2_ConfigReq.TimeVariantNo_8.Format(L"%s", LIB_MakeRandomMac());
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += m_sSTD2_ConfigReq.TimeVariantNo_8;
	}
	// end of [#4]

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD2_MakeHealthCheckMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CTranCmn::BIZ_STD2_MakeHealthCheckMsg()
{
	if (MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_MSGTYPE) == MSG_EPS_TYPE)
		m_sSTD2_HealthReq.RequestType_2 = L"H0";			// EPS
	else 
		m_sSTD2_HealthReq.RequestType_2 = L"90";			//CSP200

	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_HealthReq.RequestType_2;

	// 8. Mac Random Time Variant Data
	// [#4] NH PSC 2008.03.10 random Mac 추가. keymode: 5, 6, 7
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING)
	{
		m_sSTD2_HealthReq.TimeVariantNo_8.Format(L"%s", LIB_MakeRandomMac());
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += m_sSTD2_HealthReq.TimeVariantNo_8;
	}
	// end of [#4]

	// 10. Set Date
	m_sSTD2_HealthReq.LocalDate_8.Format(L"%2s%2s%4s", GetDate().Mid(4,2), GetDate().Right(2), GetDate().Left(4));
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_HealthReq.LocalDate_8;

	// 11. Set Time
	m_sSTD2_HealthReq.LocalTime_6 = GetTime();
	m_strSendData+= m_sSTD2_HealthReq.LocalTime_6;

	// 13-1. Terminal Status Set (Terminal Mode)
	if (m_pDevCmn->AtmStatus == ATM_ERROR)
		m_sSTD2_HealthReq.TerminalStatus_4 = "2";
	else
	if (m_pDevCmn->fnDOR_GetDoorStatus() == DOOR_OPENED)	// [#483] [NH] KSK 2009.2.6
		m_sSTD2_HealthReq.TerminalStatus_4 = "1";
	else
		m_sSTD2_HealthReq.TerminalStatus_4 = "0";

	int CstNoteStatus = m_pDevCmn->fnCDU_GetAllCSTStatus();	// [#194] KSK 2008.5.16
	
	if (m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_CDU)	|| 
							CstNoteStatus == CST_EMPTY	|| 
							CstNoteStatus == CST_NEAR	|| 
							CstNoteStatus == CST_SET_NG)
		m_sSTD2_HealthReq.TerminalStatus_4 += L"1";				// OUT OF SERVICE (fault)
	else if (m_pDevCmn->fnCDU_GetRecycleBoxStatus() == CST_NEAR)	// 전체 Cassette가 low인지를 Check한다.
		m_sSTD2_HealthReq.TerminalStatus_4 += L"2";				// NEED ATTENTION (supplies)
	else
		m_sSTD2_HealthReq.TerminalStatus_4 += L"0";				// OK

	// 13-3. Terminal Status Set (Receipt Printer)
	if (m_pDevCmn->fnAPL_GetDownDevice(DEV_SPR))
	{
		if (m_pDevCmn->fnSPR_GetPaperStatus() == SLIP_EMPTY_PAPER)
			m_sSTD2_HealthReq.TerminalStatus_4 += "2";
		else
			m_sSTD2_HealthReq.TerminalStatus_4 += "1";
	}
	else	// Receipt Printer Empty Set
	if (m_pDevCmn->fnSPR_GetPaperStatus() == SLIP_EMPTY_PAPER)
		m_sSTD2_HealthReq.TerminalStatus_4 += "2";
	else
		m_sSTD2_HealthReq.TerminalStatus_4 += "0";

	// 13-4. Terminal Status Set (Journal Printer)
	// Journal Printer가 Not Set이면 무조건 정상으로 처리함.
	m_sSTD2_HealthReq.TerminalStatus_4 += "0";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_HealthReq.TerminalStatus_4;

	// 15. Surcharge Amount Set
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT);

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD2_MakeDetailHealthCheckMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CTranCmn::BIZ_STD2_MakeDetailHealthCheckMsg()
{
	m_sSTD2_DetailHealthReq.RequestType_2 = L"61";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_DetailHealthReq.RequestType_2;

	// 10. Set Date
	m_sSTD2_DetailHealthReq.LocalDate_8.Format(L"%2s%2s%4s", GetDate().Mid(4,2), GetDate().Right(2), GetDate().Left(4));
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_DetailHealthReq.LocalDate_8;

	// 11. Set Time
	m_sSTD2_DetailHealthReq.LocalTime_6 = GetTime();
	m_strSendData+= m_sSTD2_DetailHealthReq.LocalTime_6;

	// 13-1. Terminal Status Set (Terminal Mode)
	if (m_pDevCmn->AtmStatus == ATM_ERROR)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 = "2";
	else
	if (m_pDevCmn->fnDOR_GetDoorStatus() == DOOR_OPENED)	// [#483] [NH] KSK 2009.2.6
		m_sSTD2_DetailHealthReq.TerminalStatus_16 = "1";
	else
		m_sSTD2_DetailHealthReq.TerminalStatus_16 = "0";

	int CstNoteStatus = m_pDevCmn->fnCDU_GetAllCSTStatus();	// [#194] KSK 2008.5.16
	
	if (m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_CDU)	|| 
							CstNoteStatus == CST_EMPTY	|| 
							CstNoteStatus == CST_NEAR	|| 
							CstNoteStatus == CST_SET_NG)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += L"1";				// OUT OF SERVICE (fault)
	else 
	if (m_pDevCmn->fnCDU_GetRecycleBoxStatus() == CST_NEAR)	// 전체 Cassette가 low인지를 Check한다.
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += L"2";				// NEED ATTENTION
	else
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += L"0";				// OK

	// 13-3. Terminal Status Set (Receipt Printer)
	if (m_pDevCmn->fnAPL_GetDownDevice(DEV_SPR))
	{
		if (m_pDevCmn->fnSPR_GetPaperStatus() == SLIP_EMPTY_PAPER)
			m_sSTD2_DetailHealthReq.TerminalStatus_16 += "2";
		else
			m_sSTD2_DetailHealthReq.TerminalStatus_16 += "1";
	}	// Receipt Printer Empty Set
	else
	if (m_pDevCmn->fnSPR_GetPaperStatus() == SLIP_EMPTY_PAPER)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "2";
	else
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "0";

	// 13-4. Terminal Status Set (Journal Printer)
	// Journal Printer가 Not Set이면 무조건 정상으로 처리함.
	m_sSTD2_DetailHealthReq.TerminalStatus_16 += "0";

	// 13-5 1st Cassette Status
	if (m_pDevCmn->fnCDU_GetNumberOfCST() == 0)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "9";
	else
	if (m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_1) == CST_SET_NG)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "1";
	else
	if (m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_1) == CST_NEAR)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "2";
	else
	if (m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_1) == CST_EMPTY)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "3";
	else
	if ((m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_1) == CST_NORMAL) ||
		(m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_1) == CST_FULL))
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "0";
	else
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "9";

	// 13-6 2nd Cassette Status
	if (m_pDevCmn->fnCDU_GetNumberOfCST() < 2)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "9";
	else
	if (m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_2) == CST_SET_NG)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "1";
	else
	if (m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_2) == CST_NEAR)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "2";
	else
	if (m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_2) == CST_EMPTY)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "3";
	else
	if ((m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_2) == CST_NORMAL) ||
		(m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_2) == CST_FULL))
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "0";
	else
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "9";

	// 13-7 3rd Cassette Status
	if (m_pDevCmn->fnCDU_GetNumberOfCST() < 3)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "9";
	else
	if (m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_3) == CST_SET_NG)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "1";
	else
	if (m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_3) == CST_NEAR)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "2";
	else
	if (m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_3) == CST_EMPTY)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "3";
	else
	if ((m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_3) == CST_NORMAL) ||
		(m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_3) == CST_FULL))
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "0";
	else
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "9";

	// 13-8 4th Cassette Status
	if (m_pDevCmn->fnCDU_GetNumberOfCST() < CDU_MAX_CST_COUNT)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "9";
	else
	if (m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_4) == CST_SET_NG)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "1";
	else
	if (m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_4) == CST_NEAR)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "2";
	else
	if (m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_4) == CST_EMPTY)
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "3";
	else
	if ((m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_4) == CST_NORMAL) ||
		(m_pDevCmn->fnCDU_GetCSTStatus(CDU_CST_4) == CST_FULL))
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "0";
	else
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "9";

	// 13-9 Elec. Journal Enabled or not
	m_sSTD2_DetailHealthReq.TerminalStatus_16 += "1";

	// 13-10 Elec. Journal Status
	m_sSTD2_DetailHealthReq.TerminalStatus_16 += "0";

	// 13-11 
	if (m_pDevCmn->fnAPL_GetDownDevice(DEV_PIN))
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "1";
	else
		m_sSTD2_DetailHealthReq.TerminalStatus_16 += "0";

	// 13-12 Status of sensor
	m_sSTD2_DetailHealthReq.TerminalStatus_16 += "0";	// Normal

	// 13-13 MMD Status
	m_sSTD2_DetailHealthReq.TerminalStatus_16 += "0";	// Normal

	// 13-14-16 Reserved
	m_sSTD2_DetailHealthReq.TerminalStatus_16 += "000";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_DetailHealthReq.TerminalStatus_16;

	// 15. Application Version
	CString strVersion = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_AP_VERSION);	// [#127] KSK 2008.04.21
	strVersion.Replace(L".", L"");
	m_sSTD2_DetailHealthReq.ApplicationVersion_7 = strVersion.Left(7);
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_DetailHealthReq.ApplicationVersion_7;

	// 17. Application CRC
	m_sSTD2_DetailHealthReq.ApplicationCRC_4 = "0000";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD2_DetailHealthReq.ApplicationCRC_4;

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD2_RecvHost()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD2_RecvHost()
{
	NHDEBUG(1, (_T("[CTranCmn::BIZ_STD2_RecvHost]\n")));

	//////////////////////////////////////////
	// 1. Recv Data
	if (BIZ_RecvData() != RES_OK)
	{
		// 여기에 들어올수 있는 조건은 Data가 0일때 뿐이다.
		// 들어오면 잘못된 Logic임
		// ErrorCode는 추후 협의 필요
		m_pDevCmn->fnAPL_StackError(m_pDevCmn->fstrNET_GetErrorCode(), m_pDevCmn->fstrNET_GetErrorMsg(), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		return RES_HOST_RECV_ERR;
	}

	//////////////////////////////////////////
	// 2. Analyze to Recv Data
	{
		BIZ_RETURN	nRes;
		nRes = BIZ_STD2_AnalHostData();		// Recv Anal	[#492] [NH] KSK 2009.2.5

		if (nRes != RES_OK)
			return nRes;
	}

	//////////////////////////////////////////
	// 3. Check Error

#ifdef APP_LOCAL_MODE		// LOCAL Mode 시에 에러체크 없음.
	m_pDevCmn->TranResult = TRUE;
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	// KSK 2008.10.16 Local Mode시 Reversal 처리 추가
	if (LIB_IsReversalCondition())				// [#439] NH AIREAT 2008.10.21
		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

	return RES_OK;
#endif

	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	INIT_STS);			// [#358] [NH] KSK 2008.7.10

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD2_AnalHostData()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD2_AnalHostData()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD2_AnalHostData]\n"));

	int		Cnt = 0;
	CString	strTemp;

	BIZ_RETURN nRes = RES_HOST_RECV_ERR;		// Default는 HOST ERROR로 SET함

#ifdef APP_LOCAL_MODE
	switch (TranCode)
	{
		case TC_OPEN:
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_CONFIG));
			break;
		case TC_HEALTHCHK:
		case TC_DETAILHEALTHCHK:
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_HEALTH));
			break;
		case TC_TOTAL:
		case TC_TRIALTOTAL:
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_TOTAL));
			break;
		case TC_REVERSAL:
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_REVERSAL));
			break;
		case TC_WITHDRAWAL:
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_WITHDRAWAL));
			break;
		case TC_INQUIRY:
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_INQUIRY));
			break;
		case TC_TRANSFER:
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_TRANSFER));
			break;
	}
	m_strArrRecvData.RemoveAll();
	SplitString(m_strRecvData, ".", m_strArrRecvData);

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		if(m_strArrRecvData[Cnt].GetLength() >= 2)
		{
			m_sSTD2_CommRespHeader.RecordFormat_1		= m_strArrRecvData[Cnt].Mid(0,1);
			m_sSTD2_CommRespHeader.ApplicationType_1		= m_strArrRecvData[Cnt].Mid(1,1);
		}
	}
	m_sSTD2_CommRespHeader.MessageDelimiter_1	= ".";
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD2_CommRespHeader.BankID_6 = m_strArrRecvData[Cnt];
#else

	// [#4] NH PSC 2008.03.10 Mac result 체크.
	// keymode: 5, 6, 7 and 출금, 조회, 이체, 취소거래일때만.
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	if( (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	||
		 MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		||
		 MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
		 MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING) &&			// [#565] NH KSK 2009.8.20
		(TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_REVERSAL) )
	{
		strTemp.Format(L"%S", m_arRecvBuffer);
		memset(m_szTemp, NULL, sizeof(m_szTemp));
		WideToMulti(m_szTemp, strTemp, strTemp.GetLength()-9);	// MAC field의 길이를 제외한다.
		
		m_pDevCmn->fnPIN_MacingData(MakeUnPack(m_szTemp, strTemp.GetLength()-9));	// MAC field의 길이를 제외한다.
		CString PinMacingData = m_pDevCmn->fstrPIN_GetMacingData();
		
		if (PinMacingData.Left(8).CompareNoCase(strTemp.Right(8)) != 0)
		{
			// [#397] NH PSC 2008.12.15 reversal 이 없지만, 명시적으로 reason for reversal flag를 셋팅한다.
			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 4);				// [#12] NH PSC 2008.03.24 reson for reversal 값 셋팅.

			m_pDevCmn->fnAPL_StackError(_T("D00C2"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003032), DEV_NET);	// [#419] [NH] KSK 2008.9.16

			return RES_HOST_RECV_ERR;
		}
	}
	// end of [#4]

	m_strRecvData.Format(L"%S", m_arRecvBuffer);
	m_strArrRecvData.RemoveAll();
	SplitString(m_strRecvData, FIELD_DELIMITER, m_strArrRecvData);
	
	if(m_strArrRecvData[Cnt].GetLength() >= 1)
		m_sSTD2_CommRespHeader.RecordFormat_1		= m_strArrRecvData[Cnt].Mid(0,1);

	if(m_strArrRecvData[Cnt].GetLength() >= 2)
		m_sSTD2_CommRespHeader.ApplicationType_1	= m_strArrRecvData[Cnt].Mid(1,1);

	if(m_strArrRecvData[Cnt].GetLength() >= 3)
		m_sSTD2_CommRespHeader.MessageDelimiter_1	= m_strArrRecvData[Cnt].Mid(2,1);

	if(m_strArrRecvData[Cnt].GetLength() >= 8)
		m_sSTD2_CommRespHeader.BankID_6				= m_strArrRecvData[Cnt].Mid(3,6);

#endif

	// V01.02.30 ADD EPS REVIEW10 [#89] KSK 2008.04.14
	// EPS를 제외한 경우에는 모두 FS가 있음
	if (MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_MSGTYPE).CompareNoCase(MSG_EPS_TYPE))
		Cnt++;
	// end of V01.02.30 end of [#89]

	// Record Format = '5' Format Option 1, 'I' Format Option 0
	// V01.02.30 ADD EPS REVIEW10 [#89] KSK 2008.04.14
	if (!MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_MSGTYPE).CompareNoCase(MSG_EPS_TYPE))
		m_sSTD2_CommRespHeader.TerminalID_8 = m_strArrRecvData[Cnt].Right(8);
	else
	{
		if (m_strArrRecvData.GetSize() > Cnt)
			m_sSTD2_CommRespHeader.TerminalID_8 = m_strArrRecvData[Cnt].Left(8);
	}
	// end of V01.02.30 end of [#89]
	Cnt++;

	switch (TranCode)
	{
		case TC_OPEN:
			nRes = BIZ_STD2_AnalConfigMsg(Cnt);
			break;

		case TC_WITHDRAWAL:
		case TC_INQUIRY:
		case TC_TRANSFER:
			nRes = BIZ_STD2_AnalTranMsg(Cnt);
			break;

		case TC_REVERSAL:
			nRes = BIZ_STD2_AnalReversalMsg(Cnt);
			break;

		case TC_TOTAL:
		case TC_TRIALTOTAL:
			nRes = BIZ_STD2_AnalTotalMsg(Cnt);
			break;

		case TC_HEALTHCHK:
			nRes = BIZ_STD2_AnalHealthCheckMsg(Cnt);
			break;

		case TC_DETAILHEALTHCHK:
			nRes = BIZ_STD2_AnalDetailHealthCheckMsg(Cnt);
			break;
			
		default:
			break;
	}
	
	return nRes;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD2_AnalCommHeader()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD2_AnalCommHeader()
{
	// [#318] [NH] KSK 2008.6.19
	if (m_sSTD2_CommReqHeader.RecordFormat_1 != m_sSTD2_CommRespHeader.RecordFormat_1)
	{
		// ERROR CHECK 1
		m_pDevCmn->fnAPL_StackError(_T("D009400"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003017), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD2_CommReqHeader.BankID_6 != m_sSTD2_CommRespHeader.BankID_6)
	{
		// ERROR CHECK 2
		m_pDevCmn->fnAPL_StackError(L"D009500", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003018), DEV_NET);		// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD2_CommReqHeader.TerminalID_8 != m_sSTD2_CommRespHeader.TerminalID_8)
	{
		// ERROR CHECK 3
		m_pDevCmn->fnAPL_StackError(L"D009600", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003019), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD2_AnalConfigMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD2_AnalConfigMsg(int nIndex)
{
	int Cnt = nIndex;

	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD2_ConfigResp.ResponseType_2 = m_strArrRecvData[Cnt];
	Cnt++;

	// [#4] NH PSC 2008.03.10 Macing enabled 일때, randomMac
	// keymode: 5, 6, 7
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	if (!MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE).CompareNoCase(MSG_CSP200_TYPE) &&
		(MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING || 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING))
	{
		m_sSTD2_ConfigResp.TimeVariantNo_8 = m_strArrRecvData[Cnt];		// 실제적으로 사용 안함
		Cnt++;
	}
	// end of [#4]

	if (m_strArrRecvData.GetSize() > nIndex)
	{
		if(m_strArrRecvData[nIndex].GetLength() >= 8)
		{
			m_sSTD2_ConfigResp.LocalDate_8 = m_strArrRecvData[Cnt].Mid(0,8);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE, m_sSTD2_ConfigResp.LocalDate_8); // 2008-01-16 V01.02.25 SRC-19
		}

		if(m_strArrRecvData[Cnt].GetLength() >= 14)
		{
			m_sSTD2_ConfigResp.LocalTime_6 = m_strArrRecvData[Cnt].Mid(8,6);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME, m_sSTD2_ConfigResp.LocalTime_6); // 2008-01-16 V01.02.25 SRC-19
		}				
	}
	Cnt++;

	// DDHHMM, Healthy Check Message Maximum value to pass
	if (m_strArrRecvData.GetSize() > Cnt)
	{
		m_sSTD2_ConfigResp.HealthMsgTimerVal_6 = m_strArrRecvData[Cnt];
	}
	Cnt++;

	// V01.02.30 WORKING PROC for EPS REVIEW10 [#89] KSK 2008.04.14
	if (!MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_MSGTYPE).CompareNoCase(MSG_EPS_TYPE))
	{
		if (m_strArrRecvData.GetSize() > Cnt)
		{
			// [#554] KSK 2009.08.10 m_KeyMode삭제
			// EPS인 경우 T-DES만 사용함 (MAC 사용시 추가작업 필요함)
			switch (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE))
			{
			case KEYMODE_NON_UNIQ_SDES:
			case KEYMODE_UNIQ_SDES:
				{
					m_sSTD2_ConfigResp.WorkingKey_V = m_strArrRecvData[Cnt];
					MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1, m_sSTD2_ConfigResp.WorkingKey_V);
				}
				break;

			case KEYMODE_NON_UNIQ_TDES:
				{
					// Working Key가 1개만 들어오더라도 WorkingKey 2에 copy하여 사용함.
					if(m_strArrRecvData[Cnt].GetLength() >= 16)
					{
						m_sSTD2_ConfigResp.WorkingKey_V   = m_strArrRecvData[Cnt].Mid(0,16);
						m_sSTD2_ConfigResp.WorkingKey3_V	= m_strArrRecvData[Cnt].Mid(0,16);

						// Working Key값이 2개일 경우 Working Key 2 값을 Copy해서 사용
						if (m_strArrRecvData[Cnt].GetLength() >= 32)
							m_sSTD2_ConfigResp.WorkingKey2_V	= m_strArrRecvData[Cnt].Mid(16,16);
					}
					MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1, m_sSTD2_ConfigResp.WorkingKey_V);
					MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2, m_sSTD2_ConfigResp.WorkingKey2_V);						
					MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY3, m_sSTD2_ConfigResp.WorkingKey3_V);
				}
				break;
			default:
				break;
			}
		}
	}
	else
	{
		if (m_strArrRecvData.GetSize() > Cnt)
		{
			// [#554] KSK 2009.08.10 m_KeyMode삭제
			switch (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE))
			{
				// keymode: 0, 1, 2, 5, 6
				case KEYMODE_NON_UNIQ_SDES:
				case KEYMODE_NON_UNIQ_DDES:
				case KEYMODE_UNIQ_SDES:
				case KEYMODE_NON_UNIQ_SDES_MACING:
				case KEYMODE_UNIQ_SDES_MACING:
					{
						m_sSTD2_ConfigResp.WorkingKey_V = m_strArrRecvData[Cnt];
						MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1, m_sSTD2_ConfigResp.WorkingKey_V);
						if (m_strArrRecvData.GetSize() > (Cnt+3))
						{
							if(MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_DDES			|| 
								MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
								MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING)
							{
								if(MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_DDES)
								{
									m_sSTD2_ConfigResp.WorkingKey2_V = m_strArrRecvData[Cnt+3];
									MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2, m_sSTD2_ConfigResp.WorkingKey2_V);
								}
								else
								{
									m_sSTD2_ConfigResp.WorkingKey4_V = m_strArrRecvData[Cnt+3];
									MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY4, m_sSTD2_ConfigResp.WorkingKey4_V);
								}
							}
						}
					}
					break;

				// keymode: 3, 4
				case KEYMODE_NON_UNIQ_TDES:
				case KEYMODE_UNIQ_TDES:
					{
						if(m_strArrRecvData[Cnt].GetLength() >= 16)
						{
							m_sSTD2_ConfigResp.WorkingKey_V   = m_strArrRecvData[Cnt].Mid(0,16);
							// Working Key값이 2개일 경우 Working Key 2 값을 Copy해서 사용
							if (m_strArrRecvData[Cnt].GetLength() >= 32)
								m_sSTD2_ConfigResp.WorkingKey2_V	= m_strArrRecvData[Cnt].Mid(16,16);
							// Working Key 값이 한개 내려올 경우 Working Key 1 값을 2에 Copy하여 사용
							else
								m_sSTD2_ConfigResp.WorkingKey2_V	= m_strArrRecvData[Cnt].Mid(0,16);
						}
						MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1, m_sSTD2_ConfigResp.WorkingKey_V);
						MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2, m_sSTD2_ConfigResp.WorkingKey2_V);
						// end of KSK_220

						if(m_strArrRecvData[Cnt].GetLength() >= 16)
							m_sSTD2_ConfigResp.WorkingKey3_V	= m_strArrRecvData[Cnt].Mid(0,16);

						MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY3, m_sSTD2_ConfigResp.WorkingKey3_V);
					}
					break;

				// keymode: 7
				case KEYMODE_TDES_MACING:
					{
						m_sSTD2_ConfigResp.WorkingKey_V = m_strArrRecvData[Cnt];
						MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1, m_sSTD2_ConfigResp.WorkingKey_V);
						if (m_strArrRecvData.GetSize() > (Cnt+3))
						{
							m_sSTD2_ConfigResp.WorkingKey2_V = m_strArrRecvData[Cnt+3];
							MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2, m_sSTD2_ConfigResp.WorkingKey2_V);
						}
						if (m_strArrRecvData.GetSize() > (Cnt+4))
						{
							m_sSTD2_ConfigResp.WorkingKey3_V = m_strArrRecvData[Cnt+4];
							MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY3, m_sSTD2_ConfigResp.WorkingKey3_V);
						}
						if (m_strArrRecvData.GetSize() > (Cnt+5))
						{
							m_sSTD2_ConfigResp.WorkingKey4_V = m_strArrRecvData[Cnt+5];
							MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY4, m_sSTD2_ConfigResp.WorkingKey4_V);
						}
					}
					break;

				default:
					break;
			}
		}
		// end of [#554]
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// [#2316] US Justin 2014.12.17 Not update Surcharge Amount if Percentage is used
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) != PERCENT_MODE)
		{
			m_sSTD2_ConfigResp.SurchargeAmount_V = m_strArrRecvData[Cnt];

			// [#242] NH KSK 2008.5.28 Surcharge Set Bug Fix
			// Surcharge는 길이가 variable이므로 길이check는 하지 않는다.
			if (!m_sSTD2_ConfigResp.SurchargeAmount_V.IsEmpty())
			{
				// KSK 2008.11.18 Surcharge Bug Fix
				if (Asc2Int(m_sSTD2_ConfigResp.SurchargeAmount_V) != 0)
				{
					MemSetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEENABLE, 1);
					MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHAREAMT, m_sSTD2_ConfigResp.SurchargeAmount_V);
				}
				else
				{
					MemSetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEENABLE, 0);
					MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHAREAMT, L"000");
				}
			}
			// end of [#242]
		}
		// End of [#2316] 
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		m_sSTD2_ConfigResp.BINListReqFlag_1 = m_strArrRecvData[Cnt];
		BIZ_STD2_SetConfigRequestInitiator(m_sSTD2_ConfigResp.BINListReqFlag_1);	// [#560] NH KSK 2009.8.20 공통 적용
	}
	Cnt++;

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	// Error Check
	if ((BIZ_STD2_AnalCommHeader()) != RES_OK)
		return RES_HOST_RECV_ERR;
		

	if (m_sSTD2_ConfigReq.RequestType_2 != m_sSTD2_ConfigResp.ResponseType_2)
	{
		m_pDevCmn->fnAPL_StackError(L"D009A00", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003020), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;
	}

	int nKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);

	if (m_sSTD2_ConfigResp.WorkingKey_V.GetLength() != 16)		// Working Key 1
	{
		m_pDevCmn->fnAPL_StackError(L"D009900", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003021), DEV_NET);		// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD2_ConfigResp.WorkingKey2_V.GetLength() != 16)	// Working Key 2
	{
		// Key Mode 1, 3, 4, 7, 11
		if (nKeyMode == KEYMODE_NON_UNIQ_DDES || nKeyMode == KEYMODE_NON_UNIQ_TDES || nKeyMode == KEYMODE_UNIQ_TDES ||
			nKeyMode == KEYMODE_TDES_MACING || nKeyMode == KEYMODE_TDES_TMACING)
		{
			m_pDevCmn->fnAPL_StackError(L"D009900", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003022), DEV_NET);	// [#419] [NH] KSK 2008.9.16	
			return RES_HOST_RECV_ERR;
		}
	}

	if (m_sSTD2_ConfigResp.WorkingKey3_V.GetLength() != 16)	// Working Key 3
	{
		// Key Mode 3, 4, 7, 11
		if (nKeyMode == KEYMODE_NON_UNIQ_TDES || nKeyMode == KEYMODE_UNIQ_TDES || nKeyMode == KEYMODE_TDES_MACING || nKeyMode == 	KEYMODE_TDES_TMACING)
		{
			m_pDevCmn->fnAPL_StackError(L"D009900", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003023), DEV_NET);	// [#419] [NH] KSK 2008.9.16	
			return RES_HOST_RECV_ERR;
		}
	}

	if (m_sSTD2_ConfigResp.WorkingKey4_V.GetLength() != 16)	// Mac Working Key 1 (Working Key 4)
	{
		// Key Mode 5, 6, 7, 11
		if (nKeyMode == KEYMODE_NON_UNIQ_SDES_MACING || nKeyMode == KEYMODE_UNIQ_SDES_MACING || nKeyMode == KEYMODE_TDES_MACING || nKeyMode == KEYMODE_TDES_TMACING)
		{
			m_pDevCmn->fnAPL_StackError(L"D009900", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003024), DEV_NET);	// [#419] [NH] KSK 2008.9.16	
			return RES_HOST_RECV_ERR;
		}
	}

	if (m_sSTD2_ConfigResp.WorkingKey5_V.GetLength() != 16)	// Mac Working Key 2 (Working Key 5)
	{
		// Key Mode 11
		if (nKeyMode == KEYMODE_TDES_TMACING)
		{
			m_pDevCmn->fnAPL_StackError(L"D009900", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003090), DEV_NET);	// [#419] [NH] KSK 2008.9.16
			return RES_HOST_RECV_ERR;
		}
	}

	m_pDevCmn->TranResult = TRUE;	// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD2_AnalTranMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD2_AnalTranMsg(int nIndex)
{
	int Cnt = nIndex;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// Response Type
		m_sSTD2_TranResp.ResponseType_2 = m_strArrRecvData[Cnt];
	}
	Cnt++;

	if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)	// [#565] NH KSK 2009.8.20
	{
		// MAC 사용 시,
		// keymode: 5, 6, 7, 11
		m_sSTD2_TranResp.TimeVariantNo_8 = m_strArrRecvData[Cnt];		// 실제적으로 사용은 안함
		Cnt++;
	}

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// Transaction Sequence Number
		m_sSTD2_TranResp.TranSequenceNo_4 = m_strArrRecvData[Cnt];
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// Response Code
		m_sSTD2_TranResp.ResponseCode_2 = m_strArrRecvData[Cnt];
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		if(m_strArrRecvData[Cnt].GetLength() >= 8)
		{
			m_sSTD2_TranResp.LocalDate_8 = m_strArrRecvData[Cnt].Mid(0,8);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE, m_sSTD2_TranResp.LocalDate_8);
		}
		if(m_strArrRecvData[Cnt].GetLength() >= 14)
		{
			m_sSTD2_TranResp.LocalTime_6 = m_strArrRecvData[Cnt].Mid(8,6);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME, m_sSTD2_TranResp.LocalTime_6);
		}

		// Retrieval Reference Number
		if(m_strArrRecvData[Cnt].GetLength() >= 26)
		{
			m_sSTD2_TranResp.RetrievalRefNo_12 = m_strArrRecvData[Cnt].Mid(14,12);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRETRIEVALNUM, m_sSTD2_TranResp.RetrievalRefNo_12);
		}
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// 2008-01-21 V01.02.25
		// 위와 같이 항목별로 Check하도록 수정
		// System Trace Audit Number
		if (m_strArrRecvData[Cnt].GetLength() >= 6)
		{
			// System Audit No
			m_sSTD2_TranResp.SystemTraceAuditNo_6 = m_strArrRecvData[Cnt].Mid(0,6);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM, m_sSTD2_TranResp.SystemTraceAuditNo_6);
		}

		if (m_strArrRecvData[Cnt].GetLength() >= 8)
		{
			// Network ID Code
			m_sSTD2_TranResp.NetworkIDCode_2 = m_strArrRecvData[Cnt].Mid(6,2);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID, m_sSTD2_TranResp.NetworkIDCode_2);
		}

		if (m_strArrRecvData[Cnt].GetLength() >= 16)
		{
			// Settlement Date
			m_sSTD2_TranResp.SettlementDate_8 = m_strArrRecvData[Cnt].Mid(8,8);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSSETTLEDATE, m_sSTD2_TranResp.SettlementDate_8);
		}
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// [#477] [NH] KSK 2009.1.6 Ledger Balance값이 12BYTE이상인 경우 ATM에서 처리하지 못하므로 인자하지 않는다.
		// Account Balance
		CString	strtemp;
		strtemp = m_strArrRecvData[Cnt];

		// KSK 2009.2.19 Bug Fix
		strtemp.TrimLeft();
		strtemp.TrimRight();

		memset(m_szTemp, 0, sizeof(m_szTemp));
		WideToMulti(m_szTemp, strtemp, sizeof(m_szTemp));

		if (strtemp.GetLength() > 0 && strtemp.GetLength() <= 12)		// 12자리 이하인 경우
		{
			if (m_szTemp[0] != '-')
			{
				if (IsNum(m_szTemp, strtemp.GetLength()) == TRUE)			// Numeric인 경우만 처리
				{
					m_sSTD2_TranResp.AccountBalance_V = strtemp;
					MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT, m_sSTD2_TranResp.AccountBalance_V);
				}
			}
			else
			{
				if (IsNum(&m_szTemp[1], __max(strtemp.GetLength()-1, 1)) == TRUE)			// 마이너스 처리
				{
					m_sSTD2_TranResp.AccountBalance_V = strtemp;
					MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT, m_sSTD2_TranResp.AccountBalance_V);
				}
			}
		}
		// end of [#477]
		// end of KSK 2009.2.19
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// [#477] [NH] KSK 2009.1.6 Available Balance값이 10BYTE이상인 경우 ATM에서 처리하지 못하므로 인자하지 않는다.
		// Available Balance
		CString	strtemp;
		strtemp = m_strArrRecvData[Cnt];

		// KSK 2009.2.19 Bug Fix
		strtemp.TrimLeft();
		strtemp.TrimRight();

		memset(m_szTemp, 0, sizeof(m_szTemp));
		WideToMulti(m_szTemp, strtemp, sizeof(m_szTemp));

		if (strtemp.GetLength() > 0 && strtemp.GetLength() <= 10)	// 10자리 이하인 경우
		{
			if (m_szTemp[0] != '-')
			{
				if(IsNum(m_szTemp, strtemp.GetLength()) == TRUE)		// Numeric인 경우만 처리
				{
					m_sSTD2_TranResp.AvailableBalance_V = strtemp;
					MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSAVAILBALANCE, m_sSTD2_TranResp.AvailableBalance_V);
				}
			}
			else
			{
				if(IsNum(&m_szTemp[1], __max(strtemp.GetLength()-1, 1)) == TRUE)		// 마이너스 값 처리
				{
					m_sSTD2_TranResp.AvailableBalance_V = strtemp;
					MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSAVAILBALANCE, m_sSTD2_TranResp.AvailableBalance_V);
				}
			}
		}
		// end of [#477]
		// end of KSK 2009.2.19
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// Surcharge Amount
		m_sSTD2_TranResp.SurchargeAmount_V = m_strArrRecvData[Cnt];
		m_sSTD2_TranResp.SurchargeAmount_V.TrimLeft();
		m_sSTD2_TranResp.SurchargeAmount_V.TrimRight();
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// Authorization Response Text
		m_sSTD2_TranResp.AuthResponseText_V = m_strArrRecvData[Cnt];
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// Configuration Request Initiator
		m_sSTD2_TranResp.ConfigRequestInitiator_2 = m_strArrRecvData[Cnt];
		if (m_sSTD2_TranResp.ConfigRequestInitiator_2 != "00")
			BIZ_STD2_SetConfigRequestInitiator(m_sSTD2_TranResp.ConfigRequestInitiator_2);
	}
	Cnt++;

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	// Error Check
	if ((BIZ_STD2_AnalCommHeader()) != RES_OK)
		return RES_HOST_RECV_ERR;

	if (m_sSTD2_TranReq.RequestType_2 != m_sSTD2_TranResp.ResponseType_2)
	{
		// ERROR CHECK 4
		m_pDevCmn->fnAPL_StackError(L"D009A00", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003020), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD2_TranReq.TransactionSequenceNo_4 != m_sSTD2_TranResp.TranSequenceNo_4)
	{
		// ERROR CHECK 5
		m_pDevCmn->fnAPL_StackError(_T("D009300"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003025), DEV_NET);		// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD2_TranResp.LocalDate_8.GetLength() != 8)
	{
		// ERROR CHECK 7
		m_pDevCmn->fnAPL_StackError(_T("D009E00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003026), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD2_TranResp.LocalTime_6.GetLength() != 6)
	{
		// ERROR CHECK 8
		m_pDevCmn->fnAPL_StackError(_T("D009E00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003027), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD2_TranResp.RetrievalRefNo_12.GetLength() != 12)
	{
		// ERROR CHECK 9
		m_pDevCmn->fnAPL_StackError(_T("D009E00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003028), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD2_TranResp.SystemTraceAuditNo_6.GetLength() != 6)
	{
		// ERROR CHECK 10
		m_pDevCmn->fnAPL_StackError(_T("D009F00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003029), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD2_TranResp.NetworkIDCode_2.GetLength() != 2)
	{
		// ERROR CHECK 11
		m_pDevCmn->fnAPL_StackError(_T("D009F00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003030), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD2_TranResp.SettlementDate_8.GetLength() != 8)
	{
		// ERROR CHECK 12
		m_pDevCmn->fnAPL_StackError(_T("D009F00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003031), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD2_TranResp.ResponseCode_2.GetLength() != 2)
	{
		m_pDevCmn->fnAPL_StackError(_T("D009D00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003020), DEV_NET);		// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (!(m_sSTD2_TranResp.ResponseCode_2 == L"00" || m_sSTD2_TranResp.ResponseCode_2 == L"85"))	// "00" or "85"가 아닌 경우 모두 Denial 처리
	{
		CString strHostError, strHostErrorMsg;
		strHostError.Format(L"DA0%-2.2s00", m_sSTD2_TranResp.ResponseCode_2);
		
		if (m_sSTD2_TranResp.AuthResponseText_V.GetLength() > 0)
		{
			// Host에서 Error Message가 있는 경우 Message를 그대로 화면 Display한다.
			strHostErrorMsg = m_sSTD2_TranResp.AuthResponseText_V;
			m_pDevCmn->fnAPL_StackError(strHostError, strHostErrorMsg, DEV_NET);	// [#419] [NH] KSK 2008.9.16
			return RES_HOST_DENIED;
		}

		int nResCodeIndex = -1;	
		strHostErrorMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003078);		// [#257] [MX] KSK 2008.6.5	

		nResCodeIndex = BIZ_STD2_CheckResponseCode(m_sSTD2_TranResp.ResponseCode_2);

		if (nResCodeIndex != -1)
		{
			CString	strTemp;
			strTemp.Format(_T("%s%02d"), T_APMSG_003100, nResCodeIndex);
			strHostErrorMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(strTemp);
		}


		m_pDevCmn->fnAPL_StackError(strHostError, strHostErrorMsg, DEV_NET);	// [#419] [NH] KSK 2008.9.16
		return RES_HOST_DENIED;
	}

	if (LIB_IsReversalCondition())
	{
		// REVERSAL FLAG SET
		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
	}

	// Surcharge Field는 Error Check 이후에 Set한다.
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT, m_sSTD2_TranResp.SurchargeAmount_V);	// [#169] [NH] 2008.04.25

	// EMV는 일단 제외함.
	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD2_AnalReversalMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD2_AnalReversalMsg(int nIndex)
{
	int Cnt = nIndex;

	// Response Type
	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD2_ReversalResp.ResponseType_2 = m_strArrRecvData[Cnt];
	Cnt++;

	// MAC 사용 시,
	// [#4] NH PSC 2008.03.10 Macing
	// keymdoe: 5, 6, 7
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)	// [#565] NH KSK 2009.8.20
	{
		m_sSTD2_ReversalResp.TimeVariant_8 = m_strArrRecvData[Cnt];	// 실제적으로 사용은 안함
		Cnt++;
	}
	// end of [#4]

	// TOTAL Config. Request Initiator
	if (m_strArrRecvData.GetSize() > Cnt)
	{
		m_sSTD2_ReversalResp.ConfigRequestInitiator_2 = m_strArrRecvData[Cnt];
		if (m_sSTD2_ReversalResp.ConfigRequestInitiator_2 != "00")
			BIZ_STD2_SetConfigRequestInitiator(m_sSTD2_ReversalResp.ConfigRequestInitiator_2);
	}

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	// Error Check
	if ((BIZ_STD2_AnalCommHeader()) != RES_OK)
		return RES_HOST_RECV_ERR;

	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD2_AnalTotalMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD2_AnalTotalMsg(int nIndex)
{
	int Cnt = nIndex;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// 2.Response Type
		m_sSTD2_TotalResp.ResponseType_2 = m_strArrRecvData[Cnt];
	}
	Cnt++;

	// 3.MAC 사용 시,
	// [#4] NH PSC 2008.03.10 Macing
	// keymode: 5, 6, 7
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	if (!MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE).CompareNoCase(MSG_CSP200_TYPE) &&
		(MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING || 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING))
	{
		m_sSTD2_TotalResp.TimeVariantNo_8 = m_strArrRecvData[Cnt];		// 실제적으로 사용은 안함
		Cnt++;
	}
	// end of [#4]

	// 4.NUMBER OF TRANSACTIONS
	if (m_strArrRecvData.GetSize() > Cnt)
	{
		if(m_strArrRecvData[Cnt].GetLength() >= 16)
		{
			// 4-1.NUMBER OF CASH WITHDRAWAL
			m_sSTD2_TotalResp.NoOfCashWiths_4 = m_strArrRecvData[Cnt].Left(4);
			// 4-2.NUMBER OF TRANSFER
			m_sSTD2_TotalResp.NoOfTransfers_4 = m_strArrRecvData[Cnt].Mid(4,4);
			// 4-3.NUMBER OF INQUIRY
			m_sSTD2_TotalResp.NoOfInquiries_4 = m_strArrRecvData[Cnt].Mid(8,4);
			// 4-4.NUMBER OF NON-CASH WITHDRAWAL
			m_sSTD2_TotalResp.NoOfNonCashWiths_4 = m_strArrRecvData[Cnt].Mid(12,4);
		}
	}
	Cnt++;

	// 5.TOTAL CASH DISPENSED AMOUNT
	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD2_TotalResp.TotalDispenseAmt_V = m_strArrRecvData[Cnt];
	Cnt++;

	// 6.TOTAL NON-CASH DISPENSED AMOUNT
	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD2_TotalResp.TotalNonCashDispAmt_V = m_strArrRecvData[Cnt];
	Cnt++;

	// 7.TOTAL SURCHARGE AMOUNT
	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD2_TotalResp.TotalSurchargeAmt_V = m_strArrRecvData[Cnt];
	Cnt++;

	// 8.TOTAL Config. Request Initiator
	if (m_strArrRecvData.GetSize() > Cnt)
	{
		m_sSTD2_TotalResp.ConfigRequestInitiator_2 = m_strArrRecvData[Cnt];
		if (m_sSTD2_TotalResp.ConfigRequestInitiator_2 != "00")
			BIZ_STD2_SetConfigRequestInitiator(m_sSTD2_TotalResp.ConfigRequestInitiator_2);
	}

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	// Error Check
	if ((BIZ_STD2_AnalCommHeader()) != RES_OK)
		return RES_HOST_RECV_ERR;

	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD2_AnalHealthCheckMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD2_AnalHealthCheckMsg(int nIndex)
{
	int Cnt = nIndex;

	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD2_HealthResp.ResponseType_2 = m_strArrRecvData[Cnt];
	Cnt++;

	// MAC 사용 시,
	// [#4] NH PSC 2008.03.10 Macing
	// keymdoe: 5, 6, 7
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	if (!MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE).CompareNoCase(MSG_CSP200_TYPE) &&
		(MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING			|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING				|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING))
	{
		m_sSTD2_HealthResp.TimeVariantNo_8 = m_strArrRecvData[Cnt];		// 실제적으로 사용은 안함
		Cnt++;
	}
	// end of [#4]

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		m_sSTD2_HealthResp.ConfigRequestInitiator_2 = m_strArrRecvData[Cnt];
		if (m_sSTD2_HealthResp.ConfigRequestInitiator_2 != "00")
			BIZ_STD2_SetConfigRequestInitiator(m_sSTD2_HealthResp.ConfigRequestInitiator_2);
	}

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	// Error Check
	if ((BIZ_STD2_AnalCommHeader()) != RES_OK)
		return RES_HOST_RECV_ERR;

	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD2_AnalDetailHealthCheckMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD2_AnalDetailHealthCheckMsg(int nIndex)
{
	int Cnt = nIndex;


	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD2_DetailHealthResp.ResponseType_2 = m_strArrRecvData[Cnt];
	Cnt++;

	// MAC 사용 시,
	// [#4] NH PSC 2008.03.10 Macing
	// keymdoe: 5, 6, 7
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	if (!MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE).CompareNoCase(MSG_CSP200_TYPE) &&
		(MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING || 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING))
	{
		m_sSTD2_DetailHealthResp.TimeVariantNo_8 = m_strArrRecvData[Cnt];		// 실제적으로 사용은 안함
		Cnt++;
	}
	// end of [#4]

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		m_sSTD2_DetailHealthResp.ConfigRequestInitiator_2 = m_strArrRecvData[Cnt];
		if (m_sSTD2_DetailHealthResp.ConfigRequestInitiator_2 != "00")
			BIZ_STD2_SetConfigRequestInitiator(m_sSTD2_DetailHealthResp.ConfigRequestInitiator_2);
	}

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	// Error Check
	if ((BIZ_STD2_AnalCommHeader()) != RES_OK)
		return RES_HOST_RECV_ERR;

	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD2_CheckResponseCode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD2_CheckResponseCode(CString strResponseCode)
{
	int nCodeIndex = 0;

	for( nCodeIndex =0; nCodeIndex < sizeof(STD2_RSP_TBL) / sizeof(_RSP_TBL); nCodeIndex++)
	{
		if( (LPCSTR)STD2_RSP_TBL[nCodeIndex].CODE == strResponseCode )
			return nCodeIndex;
	}
	return -1;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD2_SetConfigRequestInitiator()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD2_SetConfigRequestInitiator(CString strCmd)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD2_SetConfigRequestInitiator]\n"));

	if (!strCmd.CompareNoCase(L"00"));
	else if (!strCmd.CompareNoCase(L"01"))		// 재개국 요청 시, 재 개국함.	(Type 88)
	{
		m_pDevCmn->HostConfigCmd = TC_RECONFIGURATION;	// [#144] KSK 2008.04.20
	}
	else if (!strCmd.CompareNoCase(L"15"))		// Detailed Status Request				(Type 61)
	{
		m_pDevCmn->HostConfigCmd = TC_DETAILSTATUS;
		MemSetInt(_MEM_FLD_APP_HOSTCMD, _MEM_VAR_APP_HC15DetailStatus, 1);
	}

	return TRUE;
}