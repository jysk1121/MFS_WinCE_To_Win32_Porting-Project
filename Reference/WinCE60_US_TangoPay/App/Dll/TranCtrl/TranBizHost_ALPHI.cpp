#include "stdafx.h"
#include ".\Tran\TranCmn.h"
// [#2497] NH woooZ 2017.09.13  ALPHI server
//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"


#define	DBG_CALL		1
#define DBG_INFO		1
#define SPLIT_DUMP		16		// 2018.01.25 HJAHN NVDump Message 최대 길이


#if (APP_ALPHI)

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_ALPHI_SendHost()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_ALPHI_SendHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_ALPHI_SendHost]\n"));

#ifdef APP_LOCAL_MODE
	m_pDevCmn->fstrSCR_WaitTime(1);
	return RES_OK;
#endif

	if (m_pDevCmn->TranStatus != TRAN_ATS_ALPHI)
	{
		WriteAlphiLogToFile(L"[CTranCmn::TranStatus is not ALPHI]");
		NHDEBUG(DBG_CALL, (L"[CTranCmn::TranStatus is not ALPHI]\n"));
		return RES_NG;
	}

	// TDES KEY CHECK
	CString strAlphiKey = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_TDES_KEY);
	strAlphiKey.Trim();
	if ( strAlphiKey.GetLength() < (ALPHI_MAC_STRING_LENGTH * 2) )		// key length should be 16 * 2 = 32
	{
		WriteAlphiLogToFile(L"[CTranCmn::INVALID ALPHI KEY]: " + strAlphiKey);
		return RES_NG;
	}

	BIZ_RETURN nRet = RES_OK;
	
	switch(m_TranCodeAlphi)
	{
		case TC_ATS_REDEMPTION:
			BIZ_REDEMPTION_MakeTransMsg();
			break;
		case TC_ATS_REVERSAL:
			BIZ_REVERSAL_MakeTransMsg();
			break;
		case TC_ATS_ENQUIRY:
			BIZ_ENQUIRY_MakeTransMsg();
			break;
		case TC_ATS_LOOPTEST:
			BIZ_LOOPTEST_MakeTransMsg();
			break;
		default:
			nRet = RES_NG;
			break;
	}

	if(nRet != RES_OK)
		return nRet;

	m_nSendLength = m_strSendData.GetLength();

	// 2018.01.25 HJAHN ALPHI Send 전문 CLog.txt로 출력(Func. 78)
	m_strNVLog.Format(L"ALPHI_Send:%d", m_nSendLength);
	NVDump('O', 'C', "78", L"", m_strNVLog );
	/* Track2 need to be masked
	int index = 0;

	while( index < m_strSendData.GetLength() ) 
	{
		m_strNVLog.Format(L"%s", m_strSendData.Mid(index,SPLIT_DUMP));
		NVDump('O', 'C', "78", L"", m_strNVLog );
		index = index+SPLIT_DUMP;	
	}
	*/
	BYTE* hexBytes = HexToBytes(m_strSendData, m_nSendLength);
	m_nSendLength = m_nSendLength/2;
	
	// Initialize Send Buffer
	memset(m_arSendBuffer, 0, sizeof(m_arSendBuffer));				// Send Buffer
	memcpy(m_arSendBuffer, hexBytes, m_nSendLength);
	
	// ADD MAC
	BYTE* mac = HexToBytes(Bit128_MAC(m_strSendData),ALPHI_MAC_STRING_LENGTH);
	memcpy(&m_arSendBuffer[m_nSendLength], mac, ALPHI_MAC_HEX_LENGTH);
	m_nSendLength += ALPHI_MAC_HEX_LENGTH;

#if ALPHI_DEBUG
	WriteAlphiLogToFile(L"m_strSendData:" + m_strSendData);
	WriteAlphiLogToFile(L"m_nSendLength:" + Int2Asc(m_nSendLength));
	CString temp("m_arSendBuffer=");
	for(int i=0 ; i < m_nSendLength ; i++)
	{
		temp.Format(_T("%s [0x%2.2X]"), temp, m_arSendBuffer[i]);
	}
	WriteAlphiLogToFile(temp);
#endif

	NHDEBUG(1, (_T("***TranBizHost_ALPHI***CTranCmn::BIZ_ALPHI_SendHost() m_strSendData[%s] \n"), m_strSendData));

	if (BIZ_ALPHI_SendData() != RES_OK)
	{
		WriteAlphiLogToFile(L"BIZ_ALPHI_SendData error");

		if (m_TranCodeAlphi == TC_ATS_REDEMPTION)
		{
			/* 아래 필요한지 체크
			CString strErrCode5 = m_pDevCmn->fstrNET_GetErrorCode().Left(5);
			if( strErrCode5 == L"D1704" ||	// NO ACK to REQ
				strErrCode5 == L"D1706" ||	// NO RESP
				strErrCode5 == L"D1707" ||	// LRC MISMATCH
				strErrCode5 == L"D2200"		// NO EOT
				)
			{
				if (LIB_IsReversalCondition())
				{
					// REVERSAL FLAG SET
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
				}
			}
			*/
		}
		else if(m_TranCodeAlphi == TC_ATS_REVERSAL)
		{
		}
		else if(m_TranCodeAlphi == TC_ATS_ENQUIRY)
		{
		}
		else if(m_TranCodeAlphi == TC_ATS_LOOPTEST)
		{
		}
		else
		{
			NHDEBUG(DBG_CALL, (L"[CTranCmn::m_TranCodeAlphi is not about ALPHI]\n"));
		}
		m_pDevCmn->fnAPL_StackError(m_pDevCmn->fstrNET_GetErrorCode(), m_pDevCmn->fstrNET_GetErrorMsg(), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		nRet = RES_HOST_SEND_ERR;
	}

	return nRet;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_ALPHI_SendData()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_ALPHI_SendData()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_ALPHI_SendData] Length(%d), Data(%c%c)\n", m_nSendLength, m_arSendBuffer[0], m_arSendBuffer[1]));

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	return m_pDevCmn->fnNET_SendData(m_arSendBuffer, m_nSendLength, K_300_WAIT, TRANHOST_ALPHI); 
}


/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_ALPHI_RecvHost()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_ALPHI_RecvHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_ALPHI_RecvHost]\n"));
	if (BIZ_ALPHI_RecvData() != RES_OK)
	{
		// [#2541] 2018.03.19 AU HJAHN
		m_pDevCmn->fnAPL_StackError(m_pDevCmn->fstrNET_GetErrorCode(), m_pDevCmn->fstrNET_GetErrorMsg(), DEV_NET);	
		// end of [#2541] 
		return RES_HOST_RECV_ERR;
	}
	BIZ_RETURN	nRes = BIZ_ALPHI_AnalHostData();
	return nRes;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_ALPHI_RecvData()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_ALPHI_RecvData()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_ALPHI_RecvData]\n"));

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	m_nRecvLength = NETBUF_RECV_SIZE;
	memset( m_arRecvBuffer, 0x00, NETBUF_RECV_SIZE);

	m_pDevCmn->fnNET_RecvData( m_arRecvBuffer, &m_nRecvLength, K_180_WAIT, TRANHOST_ALPHI);

	if (!m_nRecvLength)
	{
		m_nRecvLength = 0;
		memset(m_arRecvBuffer, 0, NETBUF_RECV_SIZE);

		return T_RECVERROR;
	}
	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit3_ProcessingCode()
RETURN TYPE  : 
PARAMETER    : redemption 거래이면 TRUE
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit3_ProcessingCode(BOOL pRedemption)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit3_ProcessingCode ]\n"));
	//Bit3 Processing Code n 6
	if(pRedemption == TRUE)
		m_Reversal_Bit3_Req = "170000"; // Redemption
	else
		m_Reversal_Bit3_Req = "380000";

	WriteAlphiLogToFile(L"Bit3_ProcessingCode:" + m_Reversal_Bit3_Req );

	m_strSendData += m_Reversal_Bit3_Req;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit4_SurchargeAmount()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit4_SurchargeAmount()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit4_SurchargeAmount ]\n"));
	//Bit4 Surcharge Amount,transaction n 12
	m_Reversal_Bit4_Req.Format(L"%012d",SurchargeAmount);

	WriteAlphiLogToFile(L"Bit4_SurchargeAmount:" + m_Reversal_Bit4_Req );

	m_strSendData += m_Reversal_Bit4_Req;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit7_TransmissionDateAndTime()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit7_TransmissionDateAndTime(WORD pYear,WORD pMonth,WORD pDay,WORD pHour,WORD pMin,WORD pSec)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit7_TransmissionDateAndTime ]\n"));
	//Bit7 Transmission date and time n12 YYMMDDHHmmss
	CString temp("");

	temp.Format(L"%s%2.2d%2.2d%2.2d%2.2d%2.2d", 
		Int2Asc(pYear).Right(2), pMonth, pDay,
		pHour, pMin, pSec);

	WriteAlphiLogToFile(L"Bit7_TransmissionDateAndTime:" + temp);

	m_strSendData += temp;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit11_SystemTraceAuditNumber()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit11_SystemTraceAuditNumber(BOOL pRedemption)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit11_SystemTraceAuditNumber ]\n"));
	//Bit11 System trace audit number n6
	CString temp("");
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_MSG_AUDITNUMBER) == 0)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_MSG_AUDITNUMBER,1); // default value
	int auditNumber = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_MSG_AUDITNUMBER);
	auditNumber++;
	if(auditNumber >= 1000000)
		auditNumber = 1;
	temp.Format(L"%06d",auditNumber);
	if(pRedemption == TRUE)
		m_Reversal_Bit11_Req = temp;
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_MSG_AUDITNUMBER,auditNumber);

	WriteAlphiLogToFile(L"Bit11_SystemTraceAuditNumber:" + temp);

	m_strSendData += temp;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit12_LocalTime()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit12_LocalTime(WORD pHour,WORD pMin,WORD pSec)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit12_LocalTime ]\n"));
	//Bit12 Time,local transaction n6 HHmmss
	m_Reversal_Bit12_Req.Format(L"%2.2d%2.2d%2.2d",
		pHour, pMin, pSec);

	WriteAlphiLogToFile(L"Bit12_LocalTime:" + m_Reversal_Bit12_Req);

	m_strSendData += m_Reversal_Bit12_Req;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit13_LocalDate()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit13_LocalDate(WORD pYear,WORD pMonth,WORD pDay)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit13_LocalDate ]\n"));
	//Bit13 Date,local transaction n6 YYMMDD
	m_Reversal_Bit13_Req.Format(L"%s%2.2d%2.2d", 
		Int2Asc(pYear).Right(2), pMonth, pDay);

	WriteAlphiLogToFile(L"Bit13_LocalDate:" + m_Reversal_Bit13_Req);

	m_strSendData += m_Reversal_Bit13_Req;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit22_PointOfService()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit22_PointOfService()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit22_PointOfService ]\n"));
	//Bit22 Point of service entry mode n3 (fix 홀수는 왼쪽 0으로 padding)
	// EMV 거래면 0920 EMV 인지 어떻게 알지 Review
	if (m_pDevCmn->fnMCU_IsEmvTransaction() == TRUE)
		m_Reversal_Bit22_Req = "0920";
	else
		m_Reversal_Bit22_Req = "0900";

	WriteAlphiLogToFile(L"Bit22_PointOfService:" + m_Reversal_Bit22_Req );

	m_strSendData += m_Reversal_Bit22_Req;  // magnetic stripe

}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit32_AcquiringInstitutionIdentificationCode()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit32_AcquiringInstitutionIdentificationCode()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit32_AcquiringInstitutionIdentificationCode ]\n"));
	//Bit32 Acquiring institution identification code n LL 11 (가변 홀수는 오른쪽 F로 padding)
	m_Reversal_Bit32_Req = GetVariableLengthFields(2,MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_ID),FALSE);
	WriteAlphiLogToFile(L"Bit32_AcquiringInstitutionIdentificationCode:" + m_Reversal_Bit32_Req);

	m_strSendData += m_Reversal_Bit32_Req;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit35_Optional_Track2Data()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit35_Optional_Track2Data()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit35_Optional_Track2Data ]\n"));
	//Bit35 Track 2 data z LL 37 ( Review start,end LRC 제거확인)
	m_Reversal_Bit35_Req = GetVariableLengthFields(2,m_strLoyaltyCardTrack2,FALSE);

	WriteAlphiLogToFile(L"Bit35_Optional_Track2Data:" + m_Reversal_Bit35_Req);

	if (m_Reversal_Bit35_Req.Find('=') != -1)
	{
		WriteAlphiLogToFile(L"('=' replaced to 'D') Bit35_Optional_Track2Data:" + m_Reversal_Bit35_Req);
		m_Reversal_Bit35_Req.Replace('=', 'D');
	}

	m_strSendData += m_Reversal_Bit35_Req;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit41_CardAcceptorTerminalIdentification()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit41_CardAcceptorTerminalIdentification()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit41_CardAcceptorTerminalIdentification ]\n"));
	//Bit41 Card acceptor terminal identification ans 8
	CString temp("");	
	temp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_TERMINAL_ID);
	MakeUnPack(temp,m_Reversal_Bit41_Req);
	WriteAlphiLogToFile(L"Bit41_CardAcceptorTerminalIdentification:" + m_Reversal_Bit41_Req);

	m_strSendData += m_Reversal_Bit41_Req;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit42_CardAcceptorIdentificationCode()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit42_CardAcceptorIdentificationCode()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit42_CardAcceptorIdentificationCode ]\n"));
	//Bit42 Card acceptor identification code ans 15
	CString temp("");
	temp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_OPERATOR_ID);
	MakeUnPack(temp,m_Reversal_Bit42_Req);
	WriteAlphiLogToFile(L"Bit42_CardAcceptorIdentificationCode:" + m_Reversal_Bit42_Req);

	m_strSendData += m_Reversal_Bit42_Req;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit45_Optional_Track1Data()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit45_Optional_Track1Data()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit45_Optional_Track1Data ]\n"));
	//Bit45 Track 1 data z LL 37 ( Review start,end LRC 제거확인)
	m_Reversal_Bit45_Req = GetVariableLengthFields(2,m_strLoyaltyCardTrack2,TRUE);
	WriteAlphiLogToFile(L"Bit45_Optional_Track1Data:" + m_Reversal_Bit45_Req);

	m_strSendData += m_Reversal_Bit45_Req;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit48_Optional_AdditionalData()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit48_Optional_AdditionalData()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit48_Optional_AdditionalData ]\n"));
	//Bit48 Additional data,private ans LLL 999 optional

	m_Reversal_Bit48_Req = GetVariableLengthFields(3,MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_EXTRA_INFORMATION),TRUE);
	WriteAlphiLogToFile(L"Bit48_Optional_AdditionalData:" + m_Reversal_Bit48_Req);

	m_strSendData += m_Reversal_Bit48_Req;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit49_CurrencyCode()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit49_CurrencyCode()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit49_CurrencyCode ]\n"));
	//Bit49 Currency Code,transaction n3
	// AU 0036, NZ 0554, US 0840
	m_Reversal_Bit49_Req = "0" + MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_CURRENCY_CODE);
	WriteAlphiLogToFile(L"Bit49_CurrencyCode:" + m_Reversal_Bit49_Req);

	m_strSendData += m_Reversal_Bit49_Req;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit62_ReservedForPrivateUse()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit62_ReservedForPrivateUse()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit62_ReservedForPrivateUse ]\n"));
	//Bit62 sequence number + "|" + terminal id
	CString temp("");
	temp.Format(L"%4.4s|%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SEQUENCENO),MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID));
	m_Reversal_Bit62_Req = GetVariableLengthFields(3,temp,TRUE);
	WriteAlphiLogToFile(L"Bit62_ReservedForPrivateUse:" + m_Reversal_Bit62_Req);

	m_strSendData += m_Reversal_Bit62_Req;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit70_NetworkManagementCode()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit70_NetworkManagementCode()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit70_NetworkManagementCode ]\n"));
	//Bit70 Network management information code n3
	WriteAlphiLogToFile(L"Bit70_NetworkManagementCode:0301");

	m_strSendData += "0301";
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit90_OriginalDataElements()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit90_OriginalDataElements()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit90_OriginalDataElements ]\n"));

	//Bit90 Original system trace audit number + bit37+bit38
	CString bit90("");
	bit90 +=  m_Reversal_Bit11_Req;
	if(m_ALPHI_ResponseRedemption.Bit37.IsEmpty())
		bit90 += "000000000000";
	else
	{
		CString temp("");
		MakePack(m_ALPHI_ResponseRedemption.Bit37,temp);
		bit90 += temp;
	}
	if(m_ALPHI_ResponseRedemption.Bit38.IsEmpty())
		bit90 += "000000";
	else
	{
		CString temp("");
		MakePack(m_ALPHI_ResponseRedemption.Bit38,temp);
		bit90 += temp;
	}
	bit90 += "000000000000000000"; // reserved for Bit90

	WriteAlphiLogToFile(L"Bit90_OriginalDataElements:"+bit90);

	m_strSendData += bit90;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit101_File_Name()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit101_File_Name()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit101_File_Name ]\n"));
	//Bit101 File Name ans LL 99
	m_strSendData += "303530312E3037"; // version number 01.07 고정? 가변? review

	WriteAlphiLogToFile(L"Bit101_File_Name:303530312E3037");
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit104_TransactionDescription()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::Bit104_TransactionDescription()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit104_TransactionDescription ]\n"));
	//Bit104 Transaction description ans LLL 100
	CString temp("");
	if(TranCode == TC_INQUIRY)
		temp = "3030";
	else if(TranCode == TC_WITHDRAWAL)
		temp = "3031";
	else if(TranCode == TC_TRANSFER)
		temp = "3032";
	else
		NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit104_TransactionDescription not defined Trancode]\n"));
	m_Reversal_Bit104_Req = "303032" + temp; // length is 2

	WriteAlphiLogToFile(L"Bit104_TransactionDescription:" + m_Reversal_Bit104_Req);

	m_strSendData += m_Reversal_Bit104_Req; 
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: Bit128_MAC()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
CString CTranCmn::Bit128_MAC(CString pData)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::Bit128_MAC ]\n"));
	//Bit128 Message authentication code b64
	BYTE	MAC[ALPHI_MAC_HEX_LENGTH]		= {0, };

	//left,right key 를 cstring으로부터 변환
	CString temp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_TDES_KEY);
	temp.Trim();
	if ( temp.GetLength() < (ALPHI_MAC_STRING_LENGTH * 2) )
	{
		temp.Format(_T("%032s"), temp);

		WriteAlphiLogToFile(L"UNREACHABLE CODE !");
		WriteAlphiLogToFile(L"INVALID ALPHI KEY: " + MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_TDES_KEY));
		WriteAlphiLogToFile(L"IT HAS BEEN PADDED WITH ZERO: " + temp);
	}

	int length = ALPHI_MAC_STRING_LENGTH;
	BYTE* leftKey = HexToBytes(temp.Left(ALPHI_MAC_STRING_LENGTH), length);
	BYTE* rightKey = HexToBytes(temp.Right(ALPHI_MAC_STRING_LENGTH), length);

	length = pData.GetLength();
	BYTE* cSource = HexToBytes(pData, length);

	MakeTripleMACBlock(cSource, (BYTE*)MAC, leftKey, rightKey, leftKey, length/2);

	CString strMAC("");
	for(int i=0 ; i < ALPHI_MAC_HEX_LENGTH ; i++)
	{
		strMAC.Format(_T("%s%2.2x"),strMAC,MAC[i]);
	}

#if ALPHI_DEBUG
	CString tempLog("MAC=");
	for(int i=0 ; i < ALPHI_MAC_HEX_LENGTH ; i++)
	{
		tempLog.Format(_T("%s %d=[0x%2.2x]"), tempLog,i, MAC[i]);
	}
	WriteAlphiLogToFile(tempLog);
	WriteAlphiLogToFile(L"Bit128_MAC:" + strMAC);
#endif

	return strMAC;
}

BYTE CTranCmn::HexToByte(CString Ps_Hex)
{
	BYTE Rb_Byte = (Ps_Hex[0] >= '0' && Ps_Hex[0] <= '9' ?
		(Ps_Hex[0] & 0x0f) << 4 : (0x0a + tolower(Ps_Hex[0]) - 'a') << 4)
		| (Ps_Hex[1] >= '0' && Ps_Hex[1] <= '9' ?
		Ps_Hex[1] & 0x0f : 0x0a + tolower(Ps_Hex[1]) - 'a');
	return Rb_Byte;
}

BYTE* CTranCmn::HexToBytes(CString Ps_Str, int Ri_Length)
{
	BYTE *Rb_Bytes = new BYTE[Ri_Length/2];
	int j = 0;
	for (int i = 0; i < Ri_Length/2; i++)
	{
		Rb_Bytes[i] = HexToByte(Ps_Str.Mid(j,2));
		j += 2;
	}
	return Rb_Bytes;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_REDEMPTION_MakeTransMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_REDEMPTION_MakeTransMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_REDEMPTION_MakeTransMsg ]\n"));
	WriteAlphiLogToFile(L"BIZ_REDEMPTION_MakeTransMsg-------------->");

	if (TranCode != TC_INQUIRY && TranCode != TC_WITHDRAWAL && TranCode != TC_TRANSFER)
		return RES_NG;

	ClearBit();
	PrepareNewMessage("0200");
	m_bGotResponseForRedeem = FALSE;

	BOOL bSendTrack2(FALSE);
	BOOL bSendTrack1(FALSE);
	bSendTrack2 = SendTrack2Data();
	bSendTrack1 = !bSendTrack2;
	BOOL bSendAdditonal = ExistAdditionalData();

	// Primary Bitmap
	// 1011 0010 secondary exist,3,4,7		B2
	// 0011 1000 11,12,13	38
	// 0000 0100 22			04
	// 0000 0001 32			01
	// 0010 0000 35			20
	CString bitmap5("");
	if(bSendTrack2)
		bitmap5 = "20";
	else
		bitmap5 = "00";

	// 1100 1001 41,42,45,48	C1
	CString bitmap6("");
	bitmap6 = GetPrimaryBitmap6(bSendTrack1);

	// 1000 0000 49			80
	// 0000 0100 62			00  // [#2497] NH woooZ 2018.02.14 Bit62 추가
	m_strSendData += "B2380401"+bitmap5+bitmap6+"8004"; // Primary bitmap // [#2497] NH woooZ 2018.02.14 Bit62 추가

	// Secondary Bitmap
	// 0000 0000			00
	// 0000 0000			00
	// 0000 0000			00
	// 0000 0000			00
	// 0000 1001 101,104	09
	// 0000 0000			00
	// 0000 0000			00
	// 0000 0001 128		01
	m_strSendData += "0000000009000001"; // Secondary bitmap

	SYSTEMTIME localTime;
	::GetLocalTime(&localTime);

	Bit3_ProcessingCode(TRUE);

	Bit4_SurchargeAmount();

	Bit7_TransmissionDateAndTime(localTime.wYear,localTime.wMonth,localTime.wDay,localTime.wHour,localTime.wMinute,localTime.wSecond);

	Bit11_SystemTraceAuditNumber(TRUE);

	Bit12_LocalTime(localTime.wHour,localTime.wMinute,localTime.wSecond);

	Bit13_LocalDate(localTime.wYear,localTime.wMonth,localTime.wDay);

	Bit22_PointOfService();

	Bit32_AcquiringInstitutionIdentificationCode();

	if(bSendTrack2)	
		Bit35_Optional_Track2Data();

	Bit41_CardAcceptorTerminalIdentification();

	Bit42_CardAcceptorIdentificationCode();

	if(bSendTrack1)
		Bit45_Optional_Track1Data(); //option

	if(bSendAdditonal)
		Bit48_Optional_AdditionalData();  //option

	Bit49_CurrencyCode();

	Bit62_ReservedForPrivateUse();

	Bit101_File_Name();

	Bit104_TransactionDescription();

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_REVERSAL_MakeTransMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_REVERSAL_MakeTransMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_REVERSAL_MakeTransMsg ]\n"));
	WriteAlphiLogToFile(L"BIZ_REVERSAL_MakeTransMsg-------------->");

	PrepareNewMessage("0400");

	// Primary bitmap
	// 1011 0010 Secondary Exist,3,4,7		B2
	// 0011 1000 11,12,13	38
	// 0000 0100 22			04
	// 0000 0001 32			01
	// 0010 0000 35			20
	CString bitmap5("");
	if(m_Reversal_Bit35_Req.IsEmpty())
		bitmap5 = "00";
	else
		bitmap5 = "20";

	BOOL bSentTrack1(FALSE);
	if(!m_Reversal_Bit45_Req.IsEmpty())
		bSentTrack1 = TRUE;
	CString bitmap6("");
	bitmap6 = GetPrimaryBitmap6(bSentTrack1);

	// 1000 0000 49			80
	// 0001 0100 60			16 // [#2497] NH woooZ 2018.02.14 Bit62 추가
	m_strSendData += "B2380401"+ bitmap5 + bitmap6 + "8014"; // Primary bitmap // [#2497] NH woooZ 2018.02.14 Bit62 추가

	// Secondary bitmap
	// 0000 0000			00
	// 0000 0000			00
	// 0000 0000			00
	// 0100 0000			40
	// 0000 1001 101,104	09
	// 0000 0000			00
	// 0000 0000			00
	// 0000 0001 128		01
	m_strSendData += "0000004009000001"; // Secondary bitmap

	SYSTEMTIME localTime;
	::GetLocalTime(&localTime);

	m_strSendData += m_Reversal_Bit3_Req;
	m_strSendData += m_Reversal_Bit4_Req;
	Bit7_TransmissionDateAndTime(localTime.wYear,localTime.wMonth,localTime.wDay,localTime.wHour,localTime.wMinute,localTime.wSecond);
	Bit11_SystemTraceAuditNumber(FALSE);
	m_strSendData += m_Reversal_Bit12_Req;
	m_strSendData += m_Reversal_Bit13_Req;
	m_strSendData += m_Reversal_Bit22_Req;
	m_strSendData += m_Reversal_Bit32_Req;
	if(!m_Reversal_Bit35_Req.IsEmpty())
		m_strSendData += m_Reversal_Bit35_Req;

	m_strSendData += m_Reversal_Bit41_Req;
	m_strSendData += m_Reversal_Bit42_Req;
	if(!m_Reversal_Bit45_Req.IsEmpty())
		m_strSendData += m_Reversal_Bit45_Req;
	if(!m_Reversal_Bit48_Req.IsEmpty())
		m_strSendData += m_Reversal_Bit48_Req;
	m_strSendData += m_Reversal_Bit49_Req;

	//Bit60 Message reason code n 4
	// if redemption got response 4007 else 4021
	if(m_bGotResponseForRedeem == TRUE)
		m_strSendData += "4007";
	else
		m_strSendData += "4021";

	m_strSendData += m_Reversal_Bit62_Req;

	Bit90_OriginalDataElements();

	Bit101_File_Name();

	m_strSendData +=m_Reversal_Bit104_Req;

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_ENQUIRY_MakeTransMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_ENQUIRY_MakeTransMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_ENQUIRY_MakeTransMsg ]\n"));
	WriteAlphiLogToFile(L"BIZ_ENQUIRY_MakeTransMsg-------------->");

	ClearBit();
	PrepareNewMessage("0200");

	BOOL bSendTrack2(FALSE);
	BOOL bSendTrack1(FALSE);
	bSendTrack2 = SendTrack2Data();
	bSendTrack1 = !bSendTrack2;
	BOOL bSendAdditonal = ExistAdditionalData();
	
	// Primary bitmap
	// 1010 0010 secondary exist,3,7		A2
	// 0010 0000 11			20
	// 0000 0100 22			04
	// 0000 0001 32			01
	// 0010 0000 35			20
	CString bitmap5("");
	if(bSendTrack2)
		bitmap5 = "20";
	else
		bitmap5 = "00";

	// 1100 1001 41,42,45,48	C1
	CString bitmap6("");
	bitmap6 = GetPrimaryBitmap6(bSendTrack1);

	// 0000 0000			00
	// 0000 0000			00
	m_strSendData += "A2200401"+bitmap5+bitmap6+"0000"; // Primary bitmap

	// Secondary bitmap
	// 0000 0000			00
	// 0000 0000			00
	// 0000 0000			00
	// 0000 0000			00
	// 0000 1000 101		08
	// 0000 0000			00
	// 0000 0000			00
	// 0000 0001 128		01
	m_strSendData += "0000000008000001"; // Secondary bitmap

	SYSTEMTIME localTime;
	::GetLocalTime(&localTime);

	Bit3_ProcessingCode(FALSE);

	Bit7_TransmissionDateAndTime(localTime.wYear,localTime.wMonth,localTime.wDay,localTime.wHour,localTime.wMinute,localTime.wSecond);

	Bit11_SystemTraceAuditNumber(FALSE);

	Bit22_PointOfService();

	Bit32_AcquiringInstitutionIdentificationCode();

	if(bSendTrack2)
		Bit35_Optional_Track2Data(); //option

	Bit41_CardAcceptorTerminalIdentification();

	Bit42_CardAcceptorIdentificationCode();

	if(bSendTrack1)
		Bit45_Optional_Track1Data(); //option

	if(bSendAdditonal)
		Bit48_Optional_AdditionalData();  //option

	Bit101_File_Name();

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_LOOPTEST_MakeTransMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_LOOPTEST_MakeTransMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_LOOPTEST_MakeTransMsg ]\n"));
	WriteAlphiLogToFile(L"BIZ_LOOPTEST_MakeTransMsg-------------->");

	ClearBit();
	PrepareNewMessage("0800");

	// Primary bitmap
	// 1000 0010 secondary exist,7		82
	// 0010 0000 11			20
	// 0000 0000 			00
	// 0000 0000 			00
	// 0000 0000 			00
	// 1100 0000 41,42      C0
	// 0000 0000			00
	// 0000 0000			00
	m_strSendData += "8220000000C00000"; // Primary bitmap

	// Secondary bitmap
	// 0000 0100 70			04
	// 0000 0000			00
	// 0000 0000 			00
	// 0000 0000			00
	// 0000 1000 101		08
	// 0000 0000			00
	// 0000 0000			00
	// 0000 0001 128		01
	m_strSendData += "0400000008000001"; // Secondary bitmap

	SYSTEMTIME localTime;
	::GetLocalTime(&localTime);

	Bit7_TransmissionDateAndTime(localTime.wYear,localTime.wMonth,localTime.wDay,localTime.wHour,localTime.wMinute,localTime.wSecond);

	Bit11_SystemTraceAuditNumber(FALSE);

	Bit41_CardAcceptorTerminalIdentification();

	Bit42_CardAcceptorIdentificationCode();

	Bit70_NetworkManagementCode();
	
	Bit101_File_Name();

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_ALPHI_AnalHostData()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_ALPHI_AnalHostData()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_ALPHI_AnalHostData ]\n"));

	BIZ_RETURN nRes = RES_HOST_RECV_ERR;

	m_strRecvData.Empty();
	
#if ALPHI_DEBUG
	WriteAlphiLogToFile(L"BIZ_ALPHI_AnalHostData<--------------");
	WriteAlphiLogToFile(L"m_nRecvLength:"+Int2Asc(m_nRecvLength));
#endif

	for(int i=0 ; i < m_nRecvLength ; i++)
	{
		m_strRecvData.Format(L"%s%2.2x", m_strRecvData,m_arRecvBuffer[i]);  // check data length 4 in front of message
	}

#if ALPHI_DEBUG
	WriteAlphiLogToFile(L"m_strRecvData:"+m_strRecvData);
#endif

	// 2018.01.25 HJAHN ALPHI Recv 전문 CLog.txt로 출력(Func. 79)
	m_strNVLog.Format(L"ALPHI_Recv:%d", m_strRecvData.GetLength());
	NVDump('O', 'C', "79", L"", m_strNVLog );
	/* Track2 need to be masked.
	int index = 0;
	while( index < m_strRecvData.GetLength() ) 
	{
		m_strNVLog.Format(L"%s", m_strRecvData.Mid(index,SPLIT_DUMP));
		NVDump('O', 'C', "79", L"", m_strNVLog );
		index= index+SPLIT_DUMP;	
	}
	*/
	// check MAC value first
	if(!VerifyMAC(m_strRecvData))
		return RES_HOST_RECV_ERR;
	if(!VerifyLength())
		return RES_HOST_RECV_ERR;


	switch (m_TranCodeAlphi)
	{
		case TC_ATS_REDEMPTION:
			m_bGotResponseForRedeem = TRUE;
			nRes = BIZ_ALPHI_AnalRedemption();

			if (nRes == RES_OK)
			{
				m_bLoyaltyPointRedeemed = TRUE;
			}
			else
			{
				nRes = RES_HOST_DENIED;
			}
			break;
		case TC_ATS_ENQUIRY:
			nRes = BIZ_ALPHI_AnalEnquiry();
			if (nRes == RES_OK)
			{
			}
			else
			{
				nRes = RES_HOST_DENIED;
			}
			break;

		case TC_ATS_REVERSAL:
			nRes = BIZ_ALPHI_AnalReversal();
			if (nRes == RES_OK)
			{
			}
			else
			{
				nRes = RES_HOST_DENIED;
			}
			break;
		case TC_ATS_LOOPTEST:
			nRes = BIZ_ALPHI_AnalLoopTest();
			if (nRes == RES_OK)
			{
			}
			else
			{
				nRes = RES_HOST_DENIED;
			}
			break;

		default:
			break;
	}

	if(nRes == RES_OK)
	{
		//m_pDevCmn->TranResult = TRUE;						
		//MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);
	}
	
	return nRes;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_ALPHI_AnalEnquiry()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_ALPHI_AnalEnquiry()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_ALPHI_AnalEnquiry ]\n"));

	int offset = 0;
	CString MTI = m_strRecvData.Mid(offset,4);
	offset += 4;

	if(MTI != "0210")
		return RES_NG;

	CString primaryBitmap = m_strRecvData.Mid(offset,16);
	offset += 16;

	CString secondaryBitmap = m_strRecvData.Mid(offset,16);
	offset += 16;

	if(OptionExistBitmap(primaryBitmap.Mid(8,2),0x20)) // 0010 0000
		m_ALPHI_ResponseEnquiry.Bit35_Exist = TRUE;
	else
		m_ALPHI_ResponseEnquiry.Bit35_Exist = FALSE;

	//1100 1001 41,42,45,48
	if(OptionExistBitmap(primaryBitmap.Mid(10,2),0x08))
		m_ALPHI_ResponseEnquiry.Bit45_Exist = TRUE;
	else
		m_ALPHI_ResponseEnquiry.Bit45_Exist = FALSE;

	if(OptionExistBitmap(primaryBitmap.Mid(10,2),0x01))
		m_ALPHI_ResponseEnquiry.Bit48_Exist = TRUE;
	else
		m_ALPHI_ResponseEnquiry.Bit48_Exist = FALSE;

	//0010 0000
	if(OptionExistBitmap(primaryBitmap.Mid(14,2),0x20))
		m_ALPHI_ResponseEnquiry.Bit59_Exist = TRUE;
	else
		m_ALPHI_ResponseEnquiry.Bit59_Exist = FALSE;

	m_ALPHI_ResponseEnquiry.Bit3 = m_strRecvData.Mid(offset,6);
	offset += 6;
	m_ALPHI_ResponseEnquiry.Bit7 = m_strRecvData.Mid(offset,12);
	offset += 12;
	m_ALPHI_ResponseEnquiry.Bit11 = m_strRecvData.Mid(offset,6);
	offset += 6;
	m_ALPHI_ResponseEnquiry.Bit22 = m_strRecvData.Mid(offset+1,3); // n3 으로 앞의 0은 padding
	offset += 4;

	offset = ParseDataOfVariantLength(FALSE,2,m_ALPHI_ResponseEnquiry.Bit32,offset);

	if(m_ALPHI_ResponseEnquiry.Bit35_Exist)
	{
		offset = ParseDataOfVariantLength(FALSE,2,m_ALPHI_ResponseEnquiry.Bit35,offset);
	}

	// Bit37 alphanumeric 12
	m_ALPHI_ResponseEnquiry.Bit37 = m_strRecvData.Mid(offset,24); // 30 31 32 이렇게 데이터 구성됨 *2배
	offset += 24;

	// Bit39 alphanumeric 2
	m_ALPHI_ResponseEnquiry.Bit39 = m_strRecvData.Mid(offset,4); // 30 31 32 이렇게 데이터 구성됨 *2배
	offset += 4;

	// Bit41 ans 8
	m_ALPHI_ResponseEnquiry.Bit41 = m_strRecvData.Mid(offset,16); 
	offset += 16;

	// Bit42 ans 15
	m_ALPHI_ResponseEnquiry.Bit42 = m_strRecvData.Mid(offset,30); 
	offset += 30;

	if(m_ALPHI_ResponseEnquiry.Bit45_Exist)
	{
		offset = ParseDataOfVariantLength(TRUE,2,m_ALPHI_ResponseEnquiry.Bit45,offset);
	}
	if(m_ALPHI_ResponseEnquiry.Bit48_Exist)
	{
		offset = ParseDataOfVariantLength(TRUE,3,m_ALPHI_ResponseEnquiry.Bit48,offset);
	}
	if(m_ALPHI_ResponseEnquiry.Bit59_Exist)
	{
		m_ALPHI_ResponseEnquiry.Bit59 = m_strRecvData.Mid(offset,12); 
		offset += 12;
	}
	offset = ParseDataOfVariantLength(TRUE,2,m_ALPHI_ResponseEnquiry.Bit101,offset);

	m_ALPHI_ResponseEnquiry.Bit128 = m_strRecvData.Mid(offset,16);

#if ALPHI_DEBUG
	CString analized("");
	analized += L"primaryBitmap:"+primaryBitmap;
	analized += L",secondaryBitmap:"+secondaryBitmap;
	analized += L",Bit3:"+m_ALPHI_ResponseEnquiry.Bit3;
	analized += L",Bit7:"+m_ALPHI_ResponseEnquiry.Bit7;
	analized += L",Bit11:"+m_ALPHI_ResponseEnquiry.Bit11;
	analized += L",Bit22:"+m_ALPHI_ResponseEnquiry.Bit22;
	analized += L",Bit35:"+m_ALPHI_ResponseEnquiry.Bit35;
	analized += L",Bit37:"+m_ALPHI_ResponseEnquiry.Bit37;
	analized += L",Bit39:"+m_ALPHI_ResponseEnquiry.Bit39;
	analized += L",Bit41:"+m_ALPHI_ResponseEnquiry.Bit41;
	analized += L",Bit42:"+m_ALPHI_ResponseEnquiry.Bit42;
	analized += L",Bit45:"+m_ALPHI_ResponseEnquiry.Bit45;
	analized += L",Bit48:"+m_ALPHI_ResponseEnquiry.Bit48;
	analized += L",Bit59:"+m_ALPHI_ResponseEnquiry.Bit59;
	analized += L",Bit101:"+m_ALPHI_ResponseEnquiry.Bit101;
	analized += L",Bit128:"+m_ALPHI_ResponseEnquiry.Bit128;
	WriteAlphiLogToFile(analized);
#endif

	// check response code
	if(m_ALPHI_ResponseEnquiry.Bit39 == "3030")
		return RES_OK;
	else
	{
		// [#2541] AU HJAHN 2018.03.19 ALPHI Detailed error messages
		// If declined, save journal
		CString strReason;
		CString temp, strtemp;

		MakePack(m_ALPHI_ResponseEnquiry.Bit39, temp , m_ALPHI_ResponseEnquiry.Bit39.GetLength());		
		strtemp.Format(L"BIT39 : %s\n",temp);
		strReason=strtemp;

		MakePack(m_ALPHI_ResponseEnquiry.Bit48, temp , m_ALPHI_ResponseEnquiry.Bit48.GetLength());		
		strtemp.Format(L"BIT48 : %s",temp);
		strReason+=strtemp;

		m_pDevCmn->m_JNLMgr.Save(ALPHI_TRX_ERROR, strReason);
		return RES_NG;
	}
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_ALPHI_AnalLoopTest()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_ALPHI_AnalLoopTest()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_ALPHI_AnalLoopTest ]\n"));

	int offset = 0;
	CString MTI = m_strRecvData.Mid(offset,4);
	offset += 4;

	if(MTI != "0810")
		return RES_NG;

	CString primaryBitmap = m_strRecvData.Mid(offset,16);
	offset += 16;
	
	CString secondaryBitmap = m_strRecvData.Mid(offset,16);
	offset += 16;
	if(OptionExistBitmap(primaryBitmap.Mid(10,2),0x01))
		m_ALPHI_ResponseLoopTest.Bit48_Exist = TRUE;
	else
		m_ALPHI_ResponseLoopTest.Bit48_Exist = FALSE;

	m_ALPHI_ResponseLoopTest.Bit7 = m_strRecvData.Mid(offset,12);
	offset += 12;
	m_ALPHI_ResponseLoopTest.Bit11 = m_strRecvData.Mid(offset,6);
	offset += 6;
	// Bit37 alphanumeric 12
	m_ALPHI_ResponseLoopTest.Bit37 = m_strRecvData.Mid(offset,24); // 30 31 32 이렇게 데이터 구성됨 *2배
	offset += 24;
	// Bit39 alphanumeric 2
	m_ALPHI_ResponseLoopTest.Bit39 = m_strRecvData.Mid(offset,4); // 30 31 32 이렇게 데이터 구성됨 *2배
	offset += 4;
	// Bit41 ans 8
	m_ALPHI_ResponseLoopTest.Bit41 = m_strRecvData.Mid(offset,16); 
	offset += 16;
	// Bit42 ans 15
	m_ALPHI_ResponseLoopTest.Bit42 = m_strRecvData.Mid(offset,30); 
	offset += 30;
	if(m_ALPHI_ResponseLoopTest.Bit48_Exist)
	{
		offset = ParseDataOfVariantLength(TRUE,3,m_ALPHI_ResponseLoopTest.Bit48,offset);
	}
	// Bit70 n 3
	m_ALPHI_ResponseLoopTest.Bit70 = m_strRecvData.Mid(offset+1,3); 
	offset += 4;
	offset = ParseDataOfVariantLength(TRUE,2,m_ALPHI_ResponseLoopTest.Bit101,offset);

	m_ALPHI_ResponseLoopTest.Bit128 = m_strRecvData.Mid(offset,16);

#if ALPHI_DEBUG
	CString analized("");
	analized += L"primaryBitmap:"+primaryBitmap;
	analized += L",secondaryBitmap:"+secondaryBitmap;
	analized += L",Bit7:"+m_ALPHI_ResponseLoopTest.Bit7;
	analized += L",Bit11:"+m_ALPHI_ResponseLoopTest.Bit11;
	analized += L",Bit37:"+m_ALPHI_ResponseLoopTest.Bit37;
	analized += L",Bit39:"+m_ALPHI_ResponseLoopTest.Bit39;
	analized += L",Bit41:"+m_ALPHI_ResponseLoopTest.Bit41;
	analized += L",Bit42:"+m_ALPHI_ResponseLoopTest.Bit42;
	analized += L",Bit48:"+m_ALPHI_ResponseLoopTest.Bit48;
	analized += L",Bit70:"+m_ALPHI_ResponseLoopTest.Bit70;
	analized += L",Bit101:"+m_ALPHI_ResponseLoopTest.Bit101;
	analized += L",Bit128:"+m_ALPHI_ResponseLoopTest.Bit128;
	WriteAlphiLogToFile(analized);
#endif

	// check response code
	if(m_ALPHI_ResponseLoopTest.Bit39 == "3030")
		return RES_OK;
	else
		return RES_NG;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_ALPHI_AnalReversal()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_ALPHI_AnalReversal()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_ALPHI_AnalReversal ]\n"));

	int offset = 0;
	CString MTI = m_strRecvData.Mid(offset,4);
	offset += 4;

	if(MTI != "0410")
		return RES_NG;
	CString primaryBitmap = m_strRecvData.Mid(offset,16);
	offset += 16;

	CString secondaryBitmap = m_strRecvData.Mid(offset,16);
	offset += 16;

	if(OptionExistBitmap(primaryBitmap.Mid(0,2),0x10)) // 0001 0000
		m_ALPHI_ResponseReversal.Bit4_Exist = TRUE;
	else
		m_ALPHI_ResponseReversal.Bit4_Exist = FALSE;

	if(OptionExistBitmap(primaryBitmap.Mid(8,2),0x20)) // 0010 0000
		m_ALPHI_ResponseReversal.Bit35_Exist = TRUE;
	else
		m_ALPHI_ResponseReversal.Bit35_Exist = FALSE;

	if(OptionExistBitmap(primaryBitmap.Mid(10,2),0x08))
		m_ALPHI_ResponseReversal.Bit45_Exist = TRUE;
	else
		m_ALPHI_ResponseReversal.Bit45_Exist = FALSE;

	if(OptionExistBitmap(primaryBitmap.Mid(10,2),0x01)) // 0000 0001
		m_ALPHI_ResponseReversal.Bit48_Exist = TRUE;
	else
		m_ALPHI_ResponseReversal.Bit48_Exist = FALSE;

	m_ALPHI_ResponseReversal.Bit3 = m_strRecvData.Mid(offset,6);
	offset += 6;
	if(m_ALPHI_ResponseReversal.Bit4_Exist == TRUE)
	{
		m_ALPHI_ResponseReversal.Bit4 = m_strRecvData.Mid(offset,12);
		offset += 12;
	}
	m_ALPHI_ResponseReversal.Bit7 = m_strRecvData.Mid(offset,12);
	offset += 12;

	m_ALPHI_ResponseReversal.Bit11 = m_strRecvData.Mid(offset,6);
	offset += 6;

	m_ALPHI_ResponseReversal.Bit12 = m_strRecvData.Mid(offset,6);
	offset += 6;

	m_ALPHI_ResponseReversal.Bit13 = m_strRecvData.Mid(offset,6);
	offset += 6;

	m_ALPHI_ResponseReversal.Bit22 = m_strRecvData.Mid(offset+1,3); // n3 으로 앞의 0은 padding
	offset += 4;

	offset = ParseDataOfVariantLength(FALSE,2,m_ALPHI_ResponseReversal.Bit32,offset);

	if(m_ALPHI_ResponseReversal.Bit35_Exist == TRUE)
		offset = ParseDataOfVariantLength(FALSE,2,m_ALPHI_ResponseReversal.Bit35,offset);

	// Bit37 alphanumeric 12
	m_ALPHI_ResponseReversal.Bit37 = m_strRecvData.Mid(offset,24); // 30 31 32 이렇게 데이터 구성됨 *2배
	offset += 24; 

	m_ALPHI_ResponseReversal.Bit39 = m_strRecvData.Mid(offset,4);
	offset += 4;

	m_ALPHI_ResponseReversal.Bit41 = m_strRecvData.Mid(offset,16); 
	offset += 16;

	m_ALPHI_ResponseReversal.Bit42 = m_strRecvData.Mid(offset,30); 
	offset += 30;

	if(m_ALPHI_ResponseReversal.Bit45_Exist)
	{
		offset = ParseDataOfVariantLength(TRUE,2,m_ALPHI_ResponseReversal.Bit45,offset);
	}

	if(m_ALPHI_ResponseReversal.Bit48_Exist == TRUE)
	{
		offset = ParseDataOfVariantLength(TRUE,3,m_ALPHI_ResponseReversal.Bit48,offset);
	}
	
	//Bit49
	m_ALPHI_ResponseReversal.Bit49 = m_strRecvData.Mid(offset+1,3); 
	offset += 4;

	//Bit49
	m_ALPHI_ResponseReversal.Bit60 = m_strRecvData.Mid(offset,4); 
	offset += 4;

	//Bit49
	m_ALPHI_ResponseReversal.Bit90 = m_strRecvData.Mid(offset,42); 
	offset += 42;

	offset = ParseDataOfVariantLength(TRUE,2,m_ALPHI_ResponseReversal.Bit101,offset);

	m_ALPHI_ResponseReversal.Bit128 = m_strRecvData.Mid(offset,16);

#if ALPHI_DEBUG
	CString analized("");
	analized += L"primaryBitmap:"+primaryBitmap;
	analized += L",secondaryBitmap:"+secondaryBitmap;
	analized += L",Bit3:"+m_ALPHI_ResponseReversal.Bit3;
	analized += L",Bit4:"+m_ALPHI_ResponseReversal.Bit4;
	analized += L",Bit7:"+m_ALPHI_ResponseReversal.Bit7;
	analized += L",Bit11:"+m_ALPHI_ResponseReversal.Bit11;
	analized += L",Bit12:"+m_ALPHI_ResponseReversal.Bit12;
	analized += L",Bit13:"+m_ALPHI_ResponseReversal.Bit13;
	analized += L",Bit22:"+m_ALPHI_ResponseReversal.Bit22;
	analized += L",Bit32:"+m_ALPHI_ResponseReversal.Bit32;
	analized += L",Bit35:"+m_ALPHI_ResponseReversal.Bit35;
	analized += L",Bit37:"+m_ALPHI_ResponseReversal.Bit37;
	analized += L",Bit39:"+m_ALPHI_ResponseReversal.Bit39;
	analized += L",Bit41:"+m_ALPHI_ResponseReversal.Bit41;
	analized += L",Bit42:"+m_ALPHI_ResponseReversal.Bit42;
	analized += L",Bit45:"+m_ALPHI_ResponseReversal.Bit45;
	analized += L",Bit48:"+m_ALPHI_ResponseReversal.Bit48;
	analized += L",Bit49:"+m_ALPHI_ResponseReversal.Bit49;
	analized += L",Bit60:"+m_ALPHI_ResponseReversal.Bit60;
	analized += L",Bit90:"+m_ALPHI_ResponseReversal.Bit90;
	analized += L",Bit101:"+m_ALPHI_ResponseReversal.Bit101;
	analized += L",Bit128:"+m_ALPHI_ResponseReversal.Bit128;
	WriteAlphiLogToFile(analized);
#endif

	// check response code
	if(m_ALPHI_ResponseReversal.Bit39 == "3030")
		return RES_OK;
	else
		return RES_NG;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_ALPHI_AnalRedemption()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_ALPHI_AnalRedemption()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_ALPHI_AnalRedemption ]\n"));

	int offset = 0;
	CString MTI = m_strRecvData.Mid(offset,4);
	offset += 4;

	if(MTI != "0210")
		return RES_NG;
	CString primaryBitmap = m_strRecvData.Mid(offset,16);
	offset += 16;
	
	CString secondaryBitmap = m_strRecvData.Mid(offset,16);
	offset += 16;

	if(OptionExistBitmap(primaryBitmap.Mid(0,2),0x10)) // 0001 0000
		m_ALPHI_ResponseRedemption.Bit4_Exist = TRUE;
	else
		m_ALPHI_ResponseRedemption.Bit4_Exist = FALSE;

	if(OptionExistBitmap(primaryBitmap.Mid(8,2),0x20)) // 0010 0000
		m_ALPHI_ResponseRedemption.Bit35_Exist = TRUE;
	else
		m_ALPHI_ResponseRedemption.Bit35_Exist = FALSE;

	if(OptionExistBitmap(primaryBitmap.Mid(8,2),0x04))	// 0000 0100  // [#2497] NH woooZ 2018.01.22  Bitmap parsing 오류
		m_ALPHI_ResponseRedemption.Bit38_Exist = TRUE;
	else
		m_ALPHI_ResponseRedemption.Bit38_Exist = FALSE;

	if(OptionExistBitmap(primaryBitmap.Mid(10,2),0x08))
		m_ALPHI_ResponseRedemption.Bit45_Exist = TRUE;
	else
		m_ALPHI_ResponseRedemption.Bit45_Exist = FALSE;

	if(OptionExistBitmap(primaryBitmap.Mid(10,2),0x01)) // 0000 0001
		m_ALPHI_ResponseRedemption.Bit48_Exist = TRUE;
	else
		m_ALPHI_ResponseRedemption.Bit48_Exist = FALSE;

	if(OptionExistBitmap(primaryBitmap.Mid(14,2),0x20)) // 0010 0000
		m_ALPHI_ResponseRedemption.Bit59_Exist = TRUE;
	else
		m_ALPHI_ResponseRedemption.Bit59_Exist = FALSE;

	m_ALPHI_ResponseRedemption.Bit3 = m_strRecvData.Mid(offset,6);
	offset += 6;
	if(m_ALPHI_ResponseRedemption.Bit4_Exist == TRUE)
	{
		m_ALPHI_ResponseRedemption.Bit4 = m_strRecvData.Mid(offset,12);
		offset += 12;
	}
	m_ALPHI_ResponseRedemption.Bit7 = m_strRecvData.Mid(offset,12);
	offset += 12;

	m_ALPHI_ResponseRedemption.Bit11 = m_strRecvData.Mid(offset,6);
	offset += 6;

	m_ALPHI_ResponseRedemption.Bit12 = m_strRecvData.Mid(offset,6);
	offset += 6;

	m_ALPHI_ResponseRedemption.Bit13 = m_strRecvData.Mid(offset,6);
	offset += 6;

	m_ALPHI_ResponseRedemption.Bit22 = m_strRecvData.Mid(offset+1,3); // n3 으로 앞의 0은 padding
	offset += 4;

	offset = ParseDataOfVariantLength(FALSE,2,m_ALPHI_ResponseRedemption.Bit32,offset);
	
	if(m_ALPHI_ResponseRedemption.Bit35_Exist == TRUE)
		offset = ParseDataOfVariantLength(FALSE,2,m_ALPHI_ResponseRedemption.Bit35,offset);

	// Bit37 alphanumeric 12
	m_ALPHI_ResponseRedemption.Bit37 = m_strRecvData.Mid(offset,24); // 30 31 32 이렇게 데이터 구성됨 *2배
	offset += 24;

	if(m_ALPHI_ResponseRedemption.Bit38_Exist == TRUE)
	{
		m_ALPHI_ResponseRedemption.Bit38 = m_strRecvData.Mid(offset,12);
		offset += 12;
	}

	m_ALPHI_ResponseRedemption.Bit39 = m_strRecvData.Mid(offset,4); 
	offset += 4; 

	m_ALPHI_ResponseRedemption.Bit41 = m_strRecvData.Mid(offset,16); 
	offset += 16;

	m_ALPHI_ResponseRedemption.Bit42 = m_strRecvData.Mid(offset,30); 
	offset += 30;

	if(m_ALPHI_ResponseRedemption.Bit45_Exist)
	{
		offset = ParseDataOfVariantLength(TRUE,2,m_ALPHI_ResponseRedemption.Bit45,offset);
	}

	if(m_ALPHI_ResponseRedemption.Bit48_Exist == TRUE)
	{
		offset = ParseDataOfVariantLength(TRUE,3,m_ALPHI_ResponseRedemption.Bit48,offset);
	}

	if(m_ALPHI_ResponseRedemption.Bit59_Exist == TRUE)
	{
		m_ALPHI_ResponseRedemption.Bit59 = m_strRecvData.Mid(offset,12); 
		offset += 12;

		m_strAvailableLoyaltyPoints = m_ALPHI_ResponseRedemption.Bit59;
	}

	offset = ParseDataOfVariantLength(TRUE,2,m_ALPHI_ResponseRedemption.Bit101,offset);

	m_ALPHI_ResponseRedemption.Bit128 = m_strRecvData.Mid(offset,16);
	
#if ALPHI_DEBUG
	CString analized("");
	analized += L"primaryBitmap:"+primaryBitmap;
	analized += L",secondaryBitmap:"+secondaryBitmap;
	analized += L",Bit3:"+m_ALPHI_ResponseRedemption.Bit3;
	analized += L",Bit4:"+m_ALPHI_ResponseRedemption.Bit4;
	analized += L",Bit7:"+m_ALPHI_ResponseRedemption.Bit7;
	analized += L",Bit11:"+m_ALPHI_ResponseRedemption.Bit11;
	analized += L",Bit12:"+m_ALPHI_ResponseRedemption.Bit12;
	analized += L",Bit13:"+m_ALPHI_ResponseRedemption.Bit13;
	analized += L",Bit22:"+m_ALPHI_ResponseRedemption.Bit22;
	analized += L",Bit32:"+m_ALPHI_ResponseRedemption.Bit32;
	analized += L",Bit35:"+m_ALPHI_ResponseRedemption.Bit35;
	analized += L",Bit37:"+m_ALPHI_ResponseRedemption.Bit37;
	analized += L",Bit38:"+m_ALPHI_ResponseRedemption.Bit38;
	analized += L",Bit39:"+m_ALPHI_ResponseRedemption.Bit39;
	analized += L",Bit41:"+m_ALPHI_ResponseRedemption.Bit41;
	analized += L",Bit42:"+m_ALPHI_ResponseRedemption.Bit42;
	analized += L",Bit45:"+m_ALPHI_ResponseRedemption.Bit45;
	analized += L",Bit48:"+m_ALPHI_ResponseRedemption.Bit48;
	analized += L",Bit59:"+m_ALPHI_ResponseRedemption.Bit59;
	analized += L",Bit101:"+m_ALPHI_ResponseRedemption.Bit101;
	analized += L",Bit128:"+m_ALPHI_ResponseRedemption.Bit128;
	WriteAlphiLogToFile(analized);
#endif

	// check response code
	if(m_ALPHI_ResponseRedemption.Bit39 == "3030")
		return RES_OK;
	else
	{
		// [#2541] AU HJAHN 2018.03.19 ALPHI Detailed error messages
		// If declined, save journal
		CString strReason;
		CString temp, strtemp;

		MakePack(m_ALPHI_ResponseRedemption.Bit39, temp , m_ALPHI_ResponseRedemption.Bit39.GetLength());		
		strtemp.Format(L"BIT39 : %s\n",temp);
		strReason=strtemp;
		
		MakePack(m_ALPHI_ResponseRedemption.Bit48, temp , m_ALPHI_ResponseRedemption.Bit48.GetLength());		
		strtemp.Format(L"BIT48 : %s",temp);
		strReason+=strtemp;

		m_pDevCmn->m_JNLMgr.Save(ALPHI_TRX_ERROR, strReason);
		return RES_NG;
	}
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: OptionExistBitmap()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CTranCmn::OptionExistBitmap(CString pBitmap,BYTE pPosition)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::OptionExistBitmap ]\n"));
	// HEX string "3C" -> 0x3C
	BYTE nValue = HexToByte(pBitmap);
	if( (nValue & pPosition) == pPosition)
		return TRUE;
	else
		return FALSE;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: ParseDataOfVariantLength()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::ParseDataOfVariantLength(BOOL pUnpacked,INT pCountOfLL,CString &pKey , INT pOffset)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::ParseDataOfVariantLength ]\n"));
	if(pUnpacked == TRUE) // data was unpacked : 31323334353637383930
	{
		pCountOfLL = pCountOfLL *2;
		CString temp("");
		MakePack(m_strRecvData.Mid(pOffset,pCountOfLL),temp);
		int lengthOfLL = Asc2Int(temp) * 2;
		pKey = m_strRecvData.Mid(pOffset,pCountOfLL +lengthOfLL);
		pOffset += pCountOfLL + lengthOfLL;
	}
	else // data was not unpacked : 1234567890
	{
		int lengthOfLL = Asc2Int(m_strRecvData.Mid(pOffset,pCountOfLL));
		pKey = m_strRecvData.Mid(pOffset,pCountOfLL + lengthOfLL);
		pOffset += pCountOfLL + lengthOfLL;
		if(lengthOfLL %2 != 0)
			pOffset++; // add 1 for padding length
	}
	return pOffset;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: GetVariableLengthFields()
RETURN TYPE  : 
PARAMETER    : pNeedUnpack is true then data should be 313233 from 123
DESCRIPTION  : pNeedUnpack is false then data should be 123F from 123 with padding if length is odd
-------------------------------------------------------------------*/
CString CTranCmn::GetVariableLengthFields(int pLL,CString pValue,BOOL pNeedUnpack)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::GetVariableLengthFields ]\n"));
	CString data("");
	if(pLL == 2)
		data.Format(L"%02d",pValue.GetLength()); // length LL is 2 LLL is 4
	else if(pLL == 3)
		data.Format(L"%03d",pValue.GetLength());
	data += pValue;
	if(pNeedUnpack == TRUE)
	{
		CString unPack("");
		MakeUnPack(data,unPack);
		data = unPack;
	}
	else
	{
		if(data.GetLength() % 2 != 0)
			data += "F"; // 길이가 홀수이면 padding F
	}
	return data;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: PrepareNewMessage()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::PrepareNewMessage(CString pMTI)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::PrepareNewMessage ]\n"));
	m_strSendData.Empty();
	m_nSendLength = 0;
	m_strSendData += pMTI; // Message Type Indicator
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: GetPrimaryBitmap6()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
CString CTranCmn::GetPrimaryBitmap6(BOOL pSendTrack1)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::GetPrimaryBitmap6 ]\n"));
	CString bitmap6("");
	BOOL bSendAdditonal = ExistAdditionalData();

	if(bSendAdditonal && pSendTrack1)	// 1100 1001
		bitmap6 = "C9";
	else if(bSendAdditonal)			// 1100 0001
		bitmap6 = "C1";
	else if(pSendTrack1)
		bitmap6 = "C8";								// 1100 1000
	else
		bitmap6 = "C0";								// 1100 0000
	return bitmap6;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: ExistAdditionalData()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CTranCmn::ExistAdditionalData()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::ExistAdditionalData ]\n"));
	if(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_EXTRA_INFORMATION).GetLength()>0)
		return TRUE;
	else
		return FALSE;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: SendTrack2Data()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CTranCmn::SendTrack2Data()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::SendTrack2Data ]\n"));
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_CARDDATA_TO_BE_USED) == 0)
		return TRUE;
	else
		return FALSE;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: ClearBit()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::ClearBit()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::ClearBit ]\n"));
	m_Reversal_Bit3_Req.Empty();
	m_Reversal_Bit4_Req.Empty();
	m_Reversal_Bit11_Req.Empty();
	m_Reversal_Bit12_Req.Empty();
	m_Reversal_Bit13_Req.Empty();
	m_Reversal_Bit22_Req.Empty();
	m_Reversal_Bit32_Req.Empty();
	m_Reversal_Bit35_Req.Empty();
	m_Reversal_Bit41_Req.Empty();
	m_Reversal_Bit42_Req.Empty();
	m_Reversal_Bit45_Req.Empty();
	m_Reversal_Bit48_Req.Empty();
	m_Reversal_Bit49_Req.Empty();
	m_Reversal_Bit62_Req.Empty();
	m_Reversal_Bit104_Req.Empty();
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: VerifyMAC()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CTranCmn::VerifyMAC(CString pData)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::VerifyMAC ]\n"));

	CString mac = pData.Right(16);
	CString calculatedMAC = Bit128_MAC(pData.Left(pData.GetLength()-16));

	WriteAlphiLogToFile(L"MAC from server:"+mac);
	WriteAlphiLogToFile(L"MAC from calculated:"+calculatedMAC);

	if(mac == calculatedMAC)
		return true;
	else
		return false;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: VerifyLength()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CTranCmn::VerifyLength()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::VerifyLength ]\n"));

	if((m_nRecvLength * 2) == m_strRecvData.GetLength())
		return TRUE;
	else
		return FALSE;
}

#endif
