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
 FUNCTION NAME: BIZ_STD1_SendHost()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD1_SendHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_SendHost]\n"));
	NVDump('O', 'H', "20", L"", L"(M)AP_SEND1");

	////////////////////////////////////////////////////////////////////////////
	//	Transaction Result Initialize
	m_pDevCmn->TranResult = FALSE;								// Host Result
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, FALSE);

	// [#12] NH PSC 2008.03.24 reson for reversal 초기값을 0으로 셋팅한다.	// [#397] NH PSC 2008.12.10 Standard1 protocol도 EMV 거래시 'n' 필드 추가
	// 1: Incorrect dispense, 2: Protocol error, 3: Lost eot, 4: Mac error, 
	// 5: Customer cancellation, 6: VAS challenge/response error, 7: ICC declined transaction
	if(TranCode != TC_REVERSAL)		// [#68] UK JSW 2008.05.16 수정
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 2);

	// [#2150] US Justin 2012.10.05 Add Pin Change....
	// [#2350] US Justin 2015.06.19 Add POP Money
	// [#2446] US Justin 2016.10.07 Add Paypal CCA
	// [#2448] US Justin 2016.11.21 Add Just.Cash
	// [#2496] US Justin 2017.08.14 Add Just.Cash Bitcoin
	if (TranCode == TC_WITHDRAWAL	|| TranCode == TC_INQUIRY			|| TranCode == TC_TRANSFER	||
		TranCode == TC_PINCHANGE	|| TranCode == TC_POPMONEY			|| TranCode == TC_PAYPALCCA	||
		TranCode == TC_JUSTCASH		|| TranCode == TC_JUSTCASH_BITCOIN	|| TranCode == TC_B4U		||
		TranCode == TC_DIGITALMINT) // [#2515] PIN4 Prestaging.. => DO NOT Increase Serial Number because it is increased on prestaging.
	{
		m_pDevCmn->fnAPL_AddSerialNo(m_HostConfig); // Add Serial No
	}

	BIZ_STD1_MakeHostMsg();						// Send Host Make

	// [#2182] NH Justin 2013.03.22		Add Standard1 Print Line and xg(DCC disclaimer)
	m_strSTD1PrtMsgArray.RemoveAll();			

	// [#2360] NH KSK 2015.12.23
//	if( TranCode != TC_REVERSAL)
//		m_strSTD1DCCMsgArray.RemoveAll();
	// end of [#2360]
	// End of [#2182]

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
		// [#2446] US Justin 2016.10.07 Paypay CCA => Skip Checking PinBlock
		BOOL bCheckPinBlock = TRUE;
		#if (APP_PAYDIANT_CCA)	
		if( (TranCode==TC_PAYPALCCA) || ( (TranCode==TC_REVERSAL)&&(WITHDRAWAL_PAYPAL==MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE)) ) ) 
			bCheckPinBlock = FALSE;
		#endif
		// End of [#2446]

		if(bCheckPinBlock)			
		{
			// PIN 장애 발생 및 PIN SP 재기동 하도록 Flag 설정
			if (m_pDevCmn->nPINErrorFlag == WORKINGKEY_INIT)
				m_pDevCmn->nPINErrorFlag = PIN_MAC_BLANK_ERROR;

			m_pDevCmn->fnAPL_StackError(_T("9799904"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003006), DEV_PIN);
			return RES_HOST_SEND_ERR;
		}
	}
	// end of [#2135]	

	if (BIZ_SendData() != RES_OK)
	{
		// EOT 미수신시 Configuration / Inquiry / Transfer / Extended Configuration 에서는 정상처리함
		// [#560] NH KSK 2009.8.20	[#2076] NH KSK 2011.06.28	// [#2449] Add Download Additional service configuration
		if (TranCode == TC_OPEN || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_EXTENDED_AID_UPDATE || 
			TranCode == TC_EXTENDED_EJUPLOAD || TranCode == TC_EXTENDED_ADDSVC_CONFIG)	
		{
			if (m_pDevCmn->fstrNET_GetErrorCode().Left(5) == L"D2200")	
				return RES_OK;	// KSK 2009.8.24
		}

		// KSK 2010.03.13 위치 변경 EMV 거래 중 D2200시에 정상 처리해야함
		//////////////////////////////////////////////////////////////////////////
		// EMV Default Approval Process because of Host Send/Receive Error
		// Withdrawal and Inquiry Transacion
		//////////////////////////////////////////////////////////////////////////
		if(m_pDevCmn->fnMCU_IsEmvTransaction())		// [#397] [NH] psc 2008.08.01 for EMV transaction
		{
			if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_JUSTCASH_BITCOIN || TranCode == TC_DIGITALMINT)
			{
				if (m_bOnlineProcessing == FALSE)
				{
					// KSK 2010.08.29 통신 장애인 경우 Default 처리 추가
					m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

					if (m_pDevCmn->fnEMV_Trans_OnlineProcess(0, AAC) == EMV_RSLT_OK)
					{
						m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
						m_pDevCmn->fnEMV_Trans_Completion();
					}

					// KSK 2010.09.03 CID가 안바뀔 경우 reason for reversal code "08"로 설정
					if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
					{
						int nLen = 0;
						CString strValue;
						unsigned char	szTemp[1024] = {0,};

						if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_CryptInfData, &nLen, szTemp) == EMV_RSLT_OK)
						{
							strValue = MakeUnPack(szTemp, nLen);

							// CID가 ARQC (0x80)인 경우 Reason for Reversal을 "08"로 설정
							if (strValue == L"80")
							{
								if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
									MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 8);
							}
						}
					}
					// end of KSK 2010.09.03

					BIZ_EMV_MakeICDataforReversal();	// KSK 2009.8.31 송신 에러시에 

					m_bOnlineProcessing = TRUE;
				}
			}
		}

		if (m_pDevCmn->TranStatus == TRAN_TRAN || m_pDevCmn->TranStatus == TRAN_REVERSAL)
		{
			if ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REVERSAL_AT_HOSTERROR) == ENABLE) &&
				(m_pDevCmn->fstrNET_GetErrorCode().Left(5) == L"D1704" || m_pDevCmn->fstrNET_GetErrorCode().Left(5) == L"D1706"))
			{
				// [#563] CA KSK 2009.8.20 STANDARD1에서 D1704 or D1706시에도 Option에 따라 Reversal을 하도록 수정
				// 단, Assorted Field를 사용해야함.
				if (LIB_IsReversalCondition())				// [#439] NH AIREAT 2008.10.21
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
				// end of [#563]
			}
			// [#2350] US Justin 2015.06.19 Add POP Money
			// [#2446] US Justin 2016.10.07 Add Paypal CCA
			// [#2448] US Justin 2016.11.21 Add Just.Cash
			// [#2496] US Justin 2017.08.15 Add Just.Cash BitCoin
			// [#2515] US Justin 2017.11.20 Add Pin4 Prestaging
			else if ((TranCode == TC_WITHDRAWAL	|| TranCode == TC_POPMONEY			||
					  TranCode == TC_REVERSAL	|| TranCode == TC_PAYPALCCA			||
					  TranCode == TC_JUSTCASH	|| TranCode == TC_JUSTCASH_BITCOIN	||
					  TranCode == TC_PIN4		|| TranCode == TC_B4U				||
					  TranCode == TC_DIGITALMINT) &&
					 (m_pDevCmn->fstrNET_GetErrorCode().Left(5) == L"D2200"))
			{
				// [#99] KSK 2008.04.11 STANDARD1은 RESPONSE를 받아야만 REVERSAL을 할 수 있다
				if (LIB_IsReversalCondition())				// [#439] NH AIREAT 2008.10.21
				{
					if (BIZ_RecvData() == RES_OK)
					{
						BIZ_STD1_AnalHostData();
					}
				
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 3);
				}
			}

			m_pDevCmn->fnAPL_StackError(m_pDevCmn->fstrNET_GetErrorCode(), m_pDevCmn->fstrNET_GetErrorMsg(), DEV_NET);	// [#419] [NH] KSK 2008.9.16
			return RES_HOST_SEND_ERR;
		}
		// [#2362] NH KSK 2015.07.24 DCC일 경우 Send Fail시 OK로 return되는 Bug Fix
		#if (AU_VERSION)
		else if (m_pDevCmn->TranStatus == TRAN_DYNAMICFLOWL_1ST || m_pDevCmn->TranStatus == TRAN_DYNAMICFLOWL_2ND)
		{
			return RES_HOST_SEND_ERR;
		}
		#endif
		// end of [#2362]
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD1_MakeHostMsg()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD1_MakeHostMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTrnCmn::BIZ_STD1_MakeHostMsg]\n"));

	////////////////////////////////////////////
	// 1. Make Message
#if (APP_LIBERTYX)
	if (m_HostConfig == HC_LIBERTYX)	// [#RWC6-59] US William 2019.10.08 LibertyX
	{
		BIZ_STD1_MakeHeader_LibertyX();	
	}
	else
#endif
	{
		// Send Host Make Header
		BIZ_STD1_MakeHeader();
	}

	// Send Host Make Message
	BIZ_STD1_MakeBody();

	////////////////////////////////////////////
	// 2. Macing
	// [#4] NH PSC 2008.03.10 Mac result 계산결과 전문에 추가
	// keymode: 5, 6, 7, 11 - 현재 standard 2는 TMAC 없음.
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	int nEPPKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);
	if( nEPPKeyMode == KEYMODE_NON_UNIQ_SDES_MACING	|| nEPPKeyMode == KEYMODE_UNIQ_SDES_MACING	||
		nEPPKeyMode == KEYMODE_TDES_MACING			|| nEPPKeyMode == KEYMODE_TDES_TMACING		||
		nEPPKeyMode == KEYMODE_TR31_MACING			|| nEPPKeyMode == KEYMODE_TR31_TMACING)
	{
		BOOL bCheckMAC = FALSE;

		if (TranCode == TC_WITHDRAWAL		|| TranCode == TC_INQUIRY	|| TranCode == TC_TRANSFER	|| TranCode == TC_REVERSAL	||
			TranCode == TC_PINCHANGE		|| TranCode == TC_POPMONEY	|| TranCode == TC_PAYPALCCA	|| TranCode == TC_JUSTCASH	||
			TranCode == TC_JUSTCASH_BITCOIN	|| TranCode == TC_PIN4		|| TranCode == TC_DIGITALMINT)
			bCheckMAC = TRUE;
		else if (TranCode == TC_DYNAMICFLOWL_1ST || TranCode == TC_DYNAMICFLOWL_2ND)
		{
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_STD1_DYNAMICFLOW_MAC) == ENABLE ) 
				bCheckMAC = TRUE;
		}

		if(bCheckMAC==TRUE)		// End of [#2490]
		{
			CString		strMacData;
			MakeUnPack(m_strSendData, strMacData);
			m_pDevCmn->fnPIN_MacingData(strMacData);

			// [#2135] NH KSK 2012.05.07 Mac Result가 SPACE인 경우 PIN SP 재기동 하도록 보완
			m_strMacSendResult.Format(L"%8.8s", m_pDevCmn->fstrPIN_GetMacingData());
			m_strSendData += FIELD_DELIMITER;
			m_strSendData += m_strMacSendResult.Left(8);		// 앞에서 8자리만 짤라서 보낸다.
			// end of [#2135]

			NVDump('O', 'H', "20", L"", L"MAC_MAKE");	// [#2362] AU KSK 2015.08.03
		}
	}
	// end of [#4]

	////////////////////////////////////////////
	// 3. Make Buffer
	memset(m_szTemp, NULL, sizeof(m_szTemp));	// [###2]
	WideToMulti(m_szTemp, m_strSendData, sizeof(m_szTemp));

	////////////////////////////////////////////
	// Initialize Send Buffer
	m_nSendLength = 0;												// Send Length
	//memset(m_arSendBuffer, 0, sizeof(m_arSendBuffer));				// Send Buffer
	memset(m_arSendBuffer, 0, NETBUF_SEND_SIZE);

	memcpy(m_arSendBuffer, m_szTemp, m_strSendData.GetLength());
	m_nSendLength = m_strSendData.GetLength();

	NHDEBUG(1, (_T("***TranBizHost_Hyosung***CTranCmn::BIZ_STD1_MakeHostMsg() m_strSendData[%s] \n"), m_strSendData));
	
	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD1_MakeHeader()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD1_MakeHeader()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_MakeHeader]\n"));

	// 1. Record Format Set
	m_sSTD1_CommReqHeader.RecordFormat_1 = L"H";

	// 2. Application Type Set
	m_sSTD1_CommReqHeader.ApplicationType_1 = L"0";
	// 3. Message Delimiter Set
	m_sSTD1_CommReqHeader.MessageDelimiter_1 = L".";
	// 4. Authorization Host ID Set
	m_sSTD1_CommReqHeader.BankID_6.Format(L"%6.6s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));

	// 5. Terminal ID Set
// [#2217] US Justin 2013.08.13 Convert TID : ATM TID (STD3, 15 characters) => Dual Host TID(STD1, 8 characters)
	// m_sSTD1_CommReqHeader.TerminalID_8.Format(L"%8.8s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
	//#if (APP_CUSTOM_PAI)		// [#2499] US Justin 2017.08.21 Enable Dual Host DCC for all customers.
	if( m_HostConfig == HC_DUALHOST )
	{
		CString strConvertedTID = L"";
		CString strTemp1 = MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID);
		strTemp1.TrimRight();
		if( strTemp1.GetLength() > 8 )
		{
			CString strTemp2 = strTemp1.Left(3);			
			if( strTemp2.CompareNoCase(L"HYO")==0 )		strConvertedTID = strTemp1.Mid(3,8);
			else										strConvertedTID = strTemp1.Left(8);
		}
		else
			strConvertedTID = strTemp1;
		strConvertedTID += L"         ";
		m_sSTD1_CommReqHeader.TerminalID_8.Format(L"%8.8s", strConvertedTID.Left(8) );
	}
	else
		m_sSTD1_CommReqHeader.TerminalID_8.Format(L"%8.8s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
	// [#2499] US Justin 2017.08.21 Enable Dual Host DCC for all customers.
	/*
	#else
		m_sSTD1_CommReqHeader.TerminalID_8.Format(L"%8.8s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
	#endif
	*/
	// End of [#2499]

// End of [#2217]

	m_strSendData = m_sSTD1_CommReqHeader.RecordFormat_1		+
					m_sSTD1_CommReqHeader.ApplicationType_1		+
					m_sSTD1_CommReqHeader.MessageDelimiter_1	+
					m_sSTD1_CommReqHeader.BankID_6;
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_CommReqHeader.TerminalID_8;
	return RES_OK;
}

// [#RWC6-59] US William 2019.10.08 LibertyX
#if(APP_LIBERTYX)
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD1_MakeHeader_LibertyX()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Sets parameters on the STD1 message for LibertyX Withdrawals
-------------------------------------------------------------------*/
int	CTranCmn::BIZ_STD1_MakeHeader_LibertyX()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_MakeHeader_LibertyX]\n"));

	// 1. Record Format Set
	m_sSTD1_CommReqHeader.RecordFormat_1 = L"H";
	// 2. Application Type Set
	m_sSTD1_CommReqHeader.ApplicationType_1 = L"0";
	// 3. Message Delimiter Set
	m_sSTD1_CommReqHeader.MessageDelimiter_1 = L".";
	// 4. Authorization Host ID Set
	m_sSTD1_CommReqHeader.BankID_6.Format(L"%-6.6s", m_LXConfig.RoutingID);

	// 5. Terminal ID Set
	m_sSTD1_CommReqHeader.TerminalID_8.Format(L"%-8.8s", m_LXConfig.LocationID);

	m_strSendData = m_sSTD1_CommReqHeader.RecordFormat_1		+
					m_sSTD1_CommReqHeader.ApplicationType_1		+
					m_sSTD1_CommReqHeader.MessageDelimiter_1	+
					m_sSTD1_CommReqHeader.BankID_6;
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_CommReqHeader.TerminalID_8;
	return RES_OK;
}
#endif
// End of [#RWC6-59]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD1_MakeBody()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD1_MakeBody()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_MakeBody]\n"));

	switch(TranCode)
	{
		case TC_INQUIRY:
		case TC_TRANSFER:
		case TC_WITHDRAWAL:
		case TC_PINCHANGE:				// [#2150] US Justin 2012.10.04 Add Pin change transaction
		case TC_POPMONEY:				// [#2350] US Justin 2015.06.19 Add POP Money
		case TC_PAYPALCCA:				// [#2446] US Justin 2016.10.07 Add Paypal CCA
		case TC_JUSTCASH:				// [#2448] US Justin 2016.11.21 Add Just.Cash
		case TC_JUSTCASH_BITCOIN:		// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
		case TC_PIN4:					// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
		case TC_DIGITALMINT:
			BIZ_STD1_MakeTransMsg();
			break;

		case TC_REVERSAL:
			BIZ_STD1_MakeReversalMsg();
			break;

		case TC_TOTAL:
		case TC_TRIALTOTAL:
			BIZ_STD1_MakeTotalMsg();
			break;

		case TC_OPEN:
			BIZ_STD1_MakeConfigMsg();
			break;

		case TC_EXTENDED_EJUPLOAD:		// [#2076] NH KSK 2011.06.28
		case TC_EXTENDED_AID_UPDATE:
		case TC_EXTENDED_ADDSVC_CONFIG:	// [#2449] US Justin 2016.11.08
			BIZ_STD1_MakeExtendConfigMsg(TranCode);
			break;

		case TC_HEALTHCHK:
			BIZ_STD1_MakeHealthCheckMsg();
			break;

		// [#2150] US Justin 2012.09.27 Add Dynamic Flow
		case TC_DYNAMICFLOWL_1ST:
		case TC_DYNAMICFLOWL_2ND:
			BIZ_STD1_MakeDynamicFlowMsg();
			break;
		// End of [#2150]

		default:
			break;
	}
	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_MakeTransMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD1_MakeTransMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_MakeTransMsg]\n"));

	// Request Type
	m_sSTD1_TranReq.RequestType_2 = L"85";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_TranReq.RequestType_2;

	// 8. Time Variant Number Set (If Mac used)
	// [#4] NH PSC 2008.03.10 random Mac 추가. keymode: 5, 6, 7, 11
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TR31_MACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TR31_TMACING)		// [#565] NH KSK 2009.8.20 TDES, TMAC 지원
	{
		m_sSTD1_TranReq.RandomMac_8.Format(L"%s", LIB_MakeRandomMac());
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += m_sSTD1_TranReq.RandomMac_8;
	}
	// end of [#4]

	// 10. Operation Code
	switch (TranCode)
	{
		case TC_WITHDRAWAL:
		case TC_POPMONEY:			// [#2350] US Justin 2015.06.19 Add POP Money
		case TC_JUSTCASH:			// [#2448] US Justin 2016.11.21 Add Just.Cash
		case TC_PIN4:				// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
			// [#2150] US Justin 2012.10.09 Add DCC
			if (m_sSTD1_DynamicFlowResp.nDCCTransaction == 1)
				m_sSTD1_TranReq.OperationCode_2 = TRANTYPE_DCC;
			else
				m_sSTD1_TranReq.OperationCode_2 = TRANTYPE_WITHDRAWAL;
			// End of [#2150]
			break;
		case TC_JUSTCASH_BITCOIN:	// [#2496] US Justin 2017.08.15 Add NonCash Withdraw - Bitcoin
		case TC_DIGITALMINT:
			m_sSTD1_TranReq.OperationCode_2 = TRANTYPE_NONCASH_WITHDRAW;
			break;
		case TC_PAYPALCCA:			// [#2446] US Justin 2016.10.07 Add Paypal
			m_sSTD1_TranReq.OperationCode_2 = TRANTYPE_PAYPAL;
			break;
		case TC_INQUIRY:
			m_sSTD1_TranReq.OperationCode_2 = TRANTYPE_INQUIRY;
			break;
		case TC_TRANSFER:
			m_sSTD1_TranReq.OperationCode_2 = TRANTYPE_TRANSFER;
			break;
		case TC_PINCHANGE:			// [#2150] US Justin 2012.10.04 Add Pin Change Transaction
			m_sSTD1_TranReq.OperationCode_2 = TRANTYPE_PINCHANGE;
			break;
		default:
			break;
	}

	// [#RWC6-59] US William 2019.10.08 LibertyX
	// LibertyX host does not support cash withdrawal on it's host
	if ( (m_HostConfig == HC_LIBERTYX || m_HostConfig == HC_LTX_HOST_DEP_MODE) && 
		TranCode == TC_WITHDRAWAL )
	{
		m_sSTD1_TranReq.OperationCode_2 = TRANTYPE_NONCASH_WITHDRAW;
	}

	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_TranReq.OperationCode_2;
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE, m_sSTD1_TranReq.OperationCode_2);

	// 11. Source Account
	if (m_SourceAccount == S_CHECKING)
		m_sSTD1_TranReq.SourceAccount_2 = L"CA";
	else if (m_SourceAccount == S_SAVINGS)
		m_sSTD1_TranReq.SourceAccount_2 = L"SA";
	else if (m_SourceAccount == S_CREDITCARD)
		m_sSTD1_TranReq.SourceAccount_2 = L"CR";

	// [#RWC6-59] US William 2019.10.08 LibertyX
	// #RWC6-59:TODO Allow for LX account selection
	if ( m_HostConfig == HC_LIBERTYX || m_HostConfig == HC_LTX_HOST_DEP_MODE )
	{
		// LibertyX TXN must come from checking accounts
		m_sSTD1_TranReq.SourceAccount_2 = L"CA";
	}

	m_strSendData += m_sSTD1_TranReq.SourceAccount_2;
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSFROMACCOUNT, m_sSTD1_TranReq.SourceAccount_2);

	// 12. Destination Account
	// Default
	m_sSTD1_TranReq.DestinationAccount_2 = m_sSTD1_TranReq.SourceAccount_2;	// 2006.03.10 PJH AT 5/3rd

	if (TranCode == TC_TRANSFER)
	{
		if (m_DestAccount == S_CHECKING)
			m_sSTD1_TranReq.DestinationAccount_2 = L"CA";
		else
		if (m_DestAccount == S_SAVINGS)
			m_sSTD1_TranReq.DestinationAccount_2 = L"SA";
		else
		if (m_DestAccount == S_CREDITCARD)
			m_sSTD1_TranReq.DestinationAccount_2 = L"CR";
	}
	m_strSendData += m_sSTD1_TranReq.DestinationAccount_2;
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTOACCOUNT, m_sSTD1_TranReq.DestinationAccount_2);

	// 13. Transaction Sequence Number
	m_sSTD1_TranReq.TransactionSequenceNo_4.Format(L"%4.4s", m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig));
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_TranReq.TransactionSequenceNo_4;

	// 14. ISO CARD 1
	// [#2250] US Justin 2014.01.27 Enable Track1 => disable sending data to keep backward comatibility
	//if (m_sCardData.strISO1Data.GetLength() > 0)
	//	m_sSTD1_TranReq.Track1Data_79.Format(L"%%%s?", m_sCardData.strISO1Data.Left(77));
	// End of [#2250]
	MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_ISO1DATA, m_sSTD1_TranReq.Track1Data_79);
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_TranReq.Track1Data_79;

	// 15. ISO CARD 2
	if (m_sCardData.strISO2Data.GetLength() > 0)
	{
		if (m_sCardData.strISO2Data.GetLength() <= 37)
			m_sSTD1_TranReq.Track2Data_40.Format(L";%s?", m_sCardData.strISO2Data);
		else
			m_sSTD1_TranReq.Track2Data_40.Format(L";%38.38s?", m_sCardData.strISO2Data);
	}

	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO2DATA, m_sSTD1_TranReq.Track2Data_40);
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_TranReq.Track2Data_40;

	// 16. ISO CARD 3
	if (m_sCardData.strISO3Data.GetLength() > 0)
		m_sSTD1_TranReq.Track3Data_106.Format(L";%s?", m_sCardData.strISO3Data.Left(106));
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO3DATA, m_sSTD1_TranReq.Track3Data_106);
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_TranReq.Track3Data_106;

	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_MakeTransMsg] m_sUserSelection.strPassword [%s] \n", m_sUserSelection.strPassword));
	/************************************************************************/
	/* Make PIN Block                                                       */
	/************************************************************************/
	if (TranCode == TC_PAYPALCCA)
	{
		m_sUserSelection.strPassword = CString(' ', 16);		// ALL SPACEs...	
	}
	else if ( m_sUserSelection.strPassword.IsEmpty() )
	{
		NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_MakeTransMsg] m_sUserSelection.strPassword 1 [%s] \n", m_sUserSelection.strPassword));

		// [#RWC6-59] US William 2019.10.08 LibertyX
		// Select Master Key
		int nMKeyName = MASTERKEY_ATM;
		if (m_HostConfig == HC_DUALHOST)	
		{
			nMKeyName = MASTERKEY_DUALHOST;
		}
		else if (m_HostConfig == HC_LIBERTYX)
		{
			nMKeyName = MASTERKEY_LIBERTYX;
		}

		m_pDevCmn->fnAPL_BuildPinBlockWithAccountNo(m_sCardData.strAccountNo, nMKeyName);
		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);
		m_sUserSelection.strPassword.Format(L"%16.16s", m_pDevCmn->fstrPIN_GetPinKeyData());
		NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_MakeTransMsg] m_sUserSelection.strPassword 2 [%s] \n", m_sUserSelection.strPassword));
	}
	// End of [#2483]

	// 17. PIN Buffer
	m_sSTD1_TranReq.PINBuffer_16.Format(L"%16.16s", m_sUserSelection.strPassword.Left(16));
	m_strSendData += FIELD_DELIMITER;
	// [#2449] US Justin 2016.11.08 Not sending PIN Block for Paypay Cash Withdrawal (Request from FIS, 2016.11.08)
	//m_strSendData += m_sSTD1_TranReq.PINBuffer_16;
	if (TranCode != TC_PAYPALCCA)
		m_strSendData += m_sSTD1_TranReq.PINBuffer_16;
	// End of [#2449]

	// 18. Transaction Amount
	m_sSTD1_TranReq.TransactionAmount_V.Format(L"%d", Asc2Int(m_sUserSelection.strMoney));
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_TranReq.TransactionAmount_V;
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT,	m_sSTD1_TranReq.TransactionAmount_V);	// [#169] [NH] KSK 2008.04.25

	// [#390] [NH] KSK 2008.8.1 RBS LYNK 특이사항
	// [#92] KSK 2008.04.18 add Percent surcharge
	// 19. Surcharge Amount

	// [#2150] US Justin 2012.10.02 Displayed Surcharge Amount
	if(MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEDISPLAY) == ENABLE)
	{
#if (APP_CUSTOM_CASHDEPOT)													// RWC6-659 Admin Fee handling
		CString sSurchargeAmount = L"";
		if (m_sSTD1_DynamicFlowResp.nDCCTransaction == 1)	
		{
		//	int tmpInt = _cdTransactionState.Configurations.Surcharges * 100;    
		//  sSurchargeAmount.Format(L"%d", tmpInt);
		//	m_sSTD1_TranReq.SurchargeAmount_V.Format(sSurchargeAmount);						// RWC6-659 DCC Accepted

			int tmpInt1 = _wtoi(m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC);
			CString strValue;
			strValue.Format(L"%d", tmpInt1);

			m_sSTD1_TranReq.SurchargeAmount_V = strValue;    // RWC6-659 test Admin Fee Handling
		//	m_sSTD1_TranReq.SurchargeAmount_V = m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC;    // RWC6-659 test Admin Fee Handling
		}
		else
			m_sSTD1_TranReq.SurchargeAmount_V.Format(L"%d", m_nDisplayedSurchargeAmount);	// RWC6-659 DCC Declined (Includes Admin Fee)									
#else
		m_sSTD1_TranReq.SurchargeAmount_V.Format(L"%d", m_nDisplayedSurchargeAmount);
#endif

	}
	else
	{
		#if (AU_VERSION)	// [#2362] AU KSK 2015.07.20 AU인 경우 Balance Surcharge가 존재하므로 거래에 따라 Default로 송부함
		if (TranCode == TC_WITHDRAWAL)
		{
			// OP에 Setting된 Surcharge 송신
			m_sSTD1_TranReq.SurchargeAmount_V.Format(L"%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT));
		}
		else
		{
			m_sSTD1_TranReq.SurchargeAmount_V.Format(L"%s", MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_INQUIRY_SURCHARGE));
		}
		#else
		m_sSTD1_TranReq.SurchargeAmount_V.Format(L"%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT));
		#endif				// end of [#2362]
	}
	// End of [#2150]

	// save percent value
	m_sSTD1_TranReq.SurchargeAmount_V.TrimLeft(L" ");
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT, m_sSTD1_TranReq.SurchargeAmount_V);					// [#304] NH AIREAT 08.06.15 - Reversal Surcharge 버그 수정.
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_TranReq.SurchargeAmount_V;

	// 20. Surcharge Flag : Surcharge 화면을 뿌릴지 말지 결정하는 변수
	if (MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEDISPLAY))
		m_sSTD1_TranReq.SurchargeEnableFlag_1 = L"1";
	else
		m_sSTD1_TranReq.SurchargeEnableFlag_1 = L"0";
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEFLAG, m_sSTD1_TranReq.SurchargeEnableFlag_1);
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_TranReq.SurchargeEnableFlag_1;

	BIZ_STD1_Make_AssortedField();

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_MakeReversalMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CTranCmn::BIZ_STD1_MakeReversalMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_MakeReversalMsg]\n"));

	// Request Type
	m_sSTD1_ReversalReq.RequestType_2 = L"86";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_ReversalReq.RequestType_2;

	// [#4] NH PSC 2008.03.10 random Mac 추가. keymode: 5, 6, 7, 11
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)	// [#565] NH KSK 2009.8.20 TDES, TMAC 지원
	{
		m_sSTD1_ReversalReq.RandomMac_8.Format(L"%s", LIB_MakeRandomMac());
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += m_sSTD1_ReversalReq.RandomMac_8;
	}
	// end of [#4]

	// KSK 2009.12.21 Host Date and Time Default값 Set (Response 미 수신시 Reversal시 전송함)			
	if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE) == L"01012000" &&
		MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME) == L"000001")
	{
		SYSTEMTIME localTime;
		::GetLocalTime(&localTime);

		CString strTemp;

		strTemp.Format(L"%2.2d%2.2d%4.4d", localTime.wMonth, localTime.wDay, localTime.wYear);	// MMDDYYYY
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE, strTemp);

		strTemp.Format(L"%2.2d%2.2d%2.2d", localTime.wHour, localTime.wMinute, localTime.wSecond);	// HHMMSS
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME, strTemp);
	}
	// end of KSK 2009.12.21

	// 9. Local Transaction Date
	m_sSTD1_ReversalReq.LocalTranDate_8 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE); // 2008-01-16 V01.02.25 SRC-19
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_ReversalReq.LocalTranDate_8;

	// 10. Local Transaction Time
	m_sSTD1_ReversalReq.LocalTranTime_6 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME); // 2008-01-16 V01.02.25 SRC-19
	m_strSendData += m_sSTD1_ReversalReq.LocalTranTime_6;

	// 11. Retrieval Reference Number
	m_sSTD1_ReversalReq.RetrievalRefNo_12 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRETRIEVALNUM);
	m_strSendData += m_sSTD1_ReversalReq.RetrievalRefNo_12;

	// 12. Requested Dollar Amount
	m_sSTD1_ReversalReq.RequestedDollarAmt_V = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT);	// [#169] [NH] KSK 2008.04.25
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_ReversalReq.RequestedDollarAmt_V;

	// 13. Dispensed Dollar Amount
	m_sSTD1_ReversalReq.DispensedDollarAmt_V = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT);	// [#169] [NH] KSK 2008.04.25
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_ReversalReq.DispensedDollarAmt_V;

	// 14. Surcharge Amount
	m_sSTD1_ReversalReq.SurchargeAmount_4 = MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT);
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_ReversalReq.SurchargeAmount_4;

	// 15. Surcharge Flag
	m_sSTD1_ReversalReq.SurchargeEnableFlag_1 = MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEFLAG);
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_ReversalReq.SurchargeEnableFlag_1;

	BIZ_STD1_Make_AssortedField();
	
	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_MakeTotalMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD1_MakeTotalMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_MakeTotalMsg]\n"));

	m_sSTD1_TotalReq.RequestType_2 = L"87";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_TotalReq.RequestType_2;

	// 9. Reset Host Totals Flag
	m_sSTD1_TotalReq.ResetHostTotalFlag_1 = L"1";
	if (TranCode == TC_TRIALTOTAL)
		m_sSTD1_TotalReq.ResetHostTotalFlag_1 = L"0";

	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_TotalReq.ResetHostTotalFlag_1;

	BIZ_STD1_Make_AssortedField();					// Make Assorted Field

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_MakeConfigMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CTranCmn::BIZ_STD1_MakeConfigMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_MakeConfigMsg]\n"));
	int keyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);

	m_sSTD1_ConfigReq.RequestType_2 = L"88";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_ConfigReq.RequestType_2;

	// Set STD1 Key Mode value
	switch (keyMode)
	{
	case KEYMODE_NON_UNIQ_SDES:
		// If Key mode is 0, The Nautilus Hyosung ATM does not send Key mode field.
		m_sSTD1_ConfigReq.KeyMode_1.Format(L"");
		break;
	case KEYMODE_TDES_TMACING:
		m_sSTD1_ConfigReq.KeyMode_1.Format(L";");
		break;
	case KEYMODE_TR31:
		m_sSTD1_ConfigReq.KeyMode_1.Format(L"=");
		break;
	case KEYMODE_TR31_MACING:
		m_sSTD1_ConfigReq.KeyMode_1.Format(L">");
		break;
	case KEYMODE_TR31_TMACING:
		m_sSTD1_ConfigReq.KeyMode_1.Format(L"?");
		break;

	default: // Values below 0x0A
		m_sSTD1_ConfigReq.KeyMode_1.Format(L"%d", keyMode);
		break;
	}

	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_ConfigReq.KeyMode_1;

	if (keyMode == KEYMODE_UNIQ_SDES		||
		keyMode == KEYMODE_UNIQ_TDES		||
		keyMode == KEYMODE_UNIQ_SDES_MACING)
	{
		m_sSTD1_ConfigReq.UKSerialNoA_8.Format(L"%S", "00000000");
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += m_sSTD1_ConfigReq.UKSerialNoA_8;

		m_sSTD1_ConfigReq.UKSerialNoB_8.Format(L"%S", "00000000");
		m_strSendData += m_sSTD1_ConfigReq.UKSerialNoB_8;
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_MakeExtendConfigMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CTranCmn::BIZ_STD1_MakeExtendConfigMsg(int nSubType)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_MakeExtendConfigMsg]\n"));
	
	// Request Type
	m_sSTD1_ExtendConfigReq.RequestType_2 = L"NH";
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_ExtendConfigReq.RequestType_2;

	// Sub Request Type
	if (nSubType == TC_EXTENDED_AID_UPDATE)				// [#2076] NH KSK 2011.06.28
		m_sSTD1_ExtendConfigReq.SubRequestType_2 = L"82";
	else if (nSubType == TC_EXTENDED_ADDSVC_CONFIG)		// [#2449] US Justin 2016.11.08
		m_sSTD1_ExtendConfigReq.SubRequestType_2 = L"87";
	else
		m_sSTD1_ExtendConfigReq.SubRequestType_2 = L"86";

	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_ExtendConfigReq.SubRequestType_2;		// AID EXCHANGE

	// Assorted Field
	BIZ_STD1_Make_AssortedField();					// Make Assorted Field	[#560] CA KSK 2009.8.20

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_MakeHealthCheckMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CTranCmn::BIZ_STD1_MakeHealthCheckMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_MakeHealthCheckMsg]\n"));

	m_sSTD1_HealthReq.RequestType_2 = L"H0";		// HYOSUNG

	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_HealthReq.RequestType_2;

	// 9. Set Date
	m_sSTD1_HealthReq.LocalDate_8.Format(L"%2s%2s%4s", GetDate().Mid(4,2), GetDate().Right(2), GetDate().Left(4));
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_HealthReq.LocalDate_8;

	// 10. Set Time
	m_sSTD1_HealthReq.LocalTime_6 = GetTime();
	m_strSendData += m_sSTD1_HealthReq.LocalTime_6;

	m_strSendData += FIELD_DELIMITER;
	
	// Hyosung Format
	// 12. Bill Count 1
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TERMINAL_STATUS_FIELD_ENABLE) == 0)
	{
		m_sSTD1_HealthReq.BillCount1_V.Format(L"%d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1));
		m_strSendData += m_sSTD1_HealthReq.BillCount1_V;
	}

	m_strSendData += FIELD_DELIMITER;
	
	// 13. Bill Count 2
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TERMINAL_STATUS_FIELD_ENABLE) == 0)
	{
		m_sSTD1_HealthReq.BillCount2_V.Format(L"%d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_2));
		m_strSendData += m_sSTD1_HealthReq.BillCount2_V;
	}

	m_strSendData += FIELD_DELIMITER;	
	
	// 16. Mode Type
	// STANDARD1, STANDARD2에서 장애인 경우 OUT OF SERVICE로 송신하도록 수정
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TERMINAL_STATUS_FIELD_ENABLE) == 0)
	{
		if (m_pDevCmn->AtmStatus != ATM_CUSTOM)
			m_sSTD1_HealthReq.ModeType_1 = L"O";
		else
			m_sSTD1_HealthReq.ModeType_1 = L"I";
		m_strSendData += m_sSTD1_HealthReq.ModeType_1;
	}

	m_strSendData += FIELD_DELIMITER;
				
	// 18. Error Code
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TERMINAL_STATUS_FIELD_ENABLE) == 0)
	{
		m_sSTD1_HealthReq.ErrorCode_V.Format(L"%s", m_pDevCmn->fstrAPL_GetErrorCode());
		m_strSendData += m_sSTD1_HealthReq.ErrorCode_V;
	}

	m_strSendData += FIELD_DELIMITER;
				
	// 20. New Journal Count
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TERMINAL_STATUS_FIELD_ENABLE) == 0)
	{
		m_sSTD1_HealthReq.NewJournalCount_V.Format(L"%d", m_pDevCmn->m_JNLMgr.GetLastIndex());		// [#111] NH AIREAT 2008.04.15 JNL Number 수정.
		m_strSendData += m_sSTD1_HealthReq.NewJournalCount_V;
	}
	
	BIZ_STD1_Make_AssortedField();					// Make Assorted Field	[#561] CA KSK 2009.8.20

	return RES_OK;
}

// [#2150] US Justin 2012.09.27 Add Dynamic Flow
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_MakeDynamicFlowMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CTranCmn::BIZ_STD1_MakeDynamicFlowMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_MakeDynamicFlowMsg]\n"));
	
	// Request Type
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += L"DF";

	// [#2490] NH Justin 2017.06.14 Standard1 Dynamic Flow MAC Option
	/*
	// [#2362] Mac을 사용하는 경우에는 Random Mac을 사용하도록 추가 (미국/캐나다는 Mac 미사용이므로 국가 Define 필요)
#if (AU_VERSION)
	if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)		// [#565] NH KSK 2009.8.20 TDES, TMAC 지원
	{
		m_sSTD1_DynamicFlowReq.RandomMac_8.Format(L"%s", LIB_MakeRandomMac());
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += m_sSTD1_DynamicFlowReq.RandomMac_8;
	}
#endif
	// end of [#2362]
	*/
	int nEPPKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);
	if( nEPPKeyMode == KEYMODE_NON_UNIQ_SDES_MACING	|| nEPPKeyMode == KEYMODE_UNIQ_SDES_MACING	||
		nEPPKeyMode == KEYMODE_TDES_MACING			|| nEPPKeyMode == KEYMODE_TDES_TMACING		)
	{
		if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_STD1_DYNAMICFLOW_MAC) == ENABLE ) 
		{
			m_sSTD1_DynamicFlowReq.RandomMac_8.Format(L"%s", LIB_MakeRandomMac());
			m_strSendData += FIELD_DELIMITER;
			m_strSendData += m_sSTD1_DynamicFlowReq.RandomMac_8;
		}
	}	
	// End of [#2490]

	// Transaction Type
	m_strSendData += FIELD_DELIMITER;
	m_strSendData += m_sSTD1_DynamicFlowReq.TransactionType;

	if(TranCode == TC_DYNAMICFLOWL_1ST)
	{
		// at : Terminal currency
		int nCSTCnt = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_NUMBEROFCST);
		CString sCurID = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID);
		m_sSTD1_DynamicFlowReq.R1_TerminalCurrency.Format(L"%d%s", nCSTCnt, sCurID);
		for(int i=2;i<=4; i++)
		{
			m_sSTD1_DynamicFlowReq.R1_TerminalCurrency += L",";
			if(i<=nCSTCnt)		m_sSTD1_DynamicFlowReq.R1_TerminalCurrency += sCurID;
			else				m_sSTD1_DynamicFlowReq.R1_TerminalCurrency += L"   ";
		}
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += L"at" + m_sSTD1_DynamicFlowReq.R1_TerminalCurrency;

		// cb : Track 2 data
		if (m_sCardData.strISO2Data.GetLength() <= 37)
			m_sSTD1_DynamicFlowReq.R1_Track2Data.Format(L";%s?", m_sCardData.strISO2Data);
		else
			m_sSTD1_DynamicFlowReq.R1_Track2Data.Format(L";%38.38s?", m_sCardData.strISO2Data);
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += L"cb" + m_sSTD1_DynamicFlowReq.R1_Track2Data;

		// [#2182] NH Justin 2013.03.21 Implement NHD1.6 
		// cd : AID		 
		if( (m_pDevCmn->fnMCU_IsEmvTransaction()) && (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
		{
			CString sAID = MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID);
			//int nLoc = sAID.Find(L"=");
			//if(nLoc>0)
			{
				//m_sSTD1_DynamicFlowReq.R1_AID = sAID.Mid(nLoc+1);
				m_sSTD1_DynamicFlowReq.R1_AID = sAID;				// [#2245] Justin 2014.02.04
				m_sSTD1_DynamicFlowReq.R1_AID.TrimLeft();
				m_sSTD1_DynamicFlowReq.R1_AID.TrimRight();
				if( m_sSTD1_DynamicFlowReq.R1_AID.GetLength() > 0)
				{
					m_strSendData += FIELD_DELIMITER;
					m_strSendData += L"cd" + m_sSTD1_DynamicFlowReq.R1_AID;
				}
			}
		}
		// End of [#2182]
	}
	else
	{
		// [#2242] NH Justin 2013.12.19 NHD DF 1st call Option
		if( m_sSTD1_DynamicFlowReq.R1_Track2Data.GetLength() > 3)
		{
			// [#2182] NH Justin 2013.03.21 Implement NHD1.6 
			// cb : Track 2 data
			m_strSendData += FIELD_DELIMITER;
			m_strSendData += L"cb" + m_sSTD1_DynamicFlowReq.R1_Track2Data;

			// cd : AID	
			if( m_sSTD1_DynamicFlowReq.R1_AID.GetLength() > 0)
			{
				m_strSendData += FIELD_DELIMITER;
				m_strSendData += L"cd" + m_sSTD1_DynamicFlowReq.R1_AID;
			}
			// End of [#2182]
		}
		else
		{
			// cb
			if (m_sCardData.strISO2Data.GetLength() <= 37)
				m_sSTD1_DynamicFlowReq.R1_Track2Data.Format(L";%s?", m_sCardData.strISO2Data);
			else
				m_sSTD1_DynamicFlowReq.R1_Track2Data.Format(L";%38.38s?", m_sCardData.strISO2Data);
			m_strSendData += FIELD_DELIMITER;
			m_strSendData += L"cb" + m_sSTD1_DynamicFlowReq.R1_Track2Data;
			
			// cd
			if( (m_pDevCmn->fnMCU_IsEmvTransaction()) && (MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
			{
				CString sAID = MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID);
				//int nLoc = sAID.Find(L"=");
				//if(nLoc>0)
				{
					//m_sSTD1_DynamicFlowReq.R1_AID = sAID.Mid(nLoc+1);
					m_sSTD1_DynamicFlowReq.R1_AID = sAID;				// [#2245] Justin 2014.02.04
					m_sSTD1_DynamicFlowReq.R1_AID.TrimLeft();
					m_sSTD1_DynamicFlowReq.R1_AID.TrimRight();
					if( m_sSTD1_DynamicFlowReq.R1_AID.GetLength() > 0)
					{
						m_strSendData += FIELD_DELIMITER;
						m_strSendData += L"cd" + m_sSTD1_DynamicFlowReq.R1_AID;
					}
				}
			}
		}
		// End of [#2242]

		// ta : Transaction Amount
		m_sSTD1_DynamicFlowReq.R2_TransactionAmount.Format(L"%08d", Asc2Int(m_sUserSelection.strMoney));
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += L"ta" + m_sSTD1_DynamicFlowReq.R2_TransactionAmount;

		// tc : Crrency Code
		m_sSTD1_DynamicFlowReq.R2_CurrencyCode.Format(L"%s", MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID) );
		m_strSendData += FIELD_DELIMITER;
		m_strSendData += L"tc" + m_sSTD1_DynamicFlowReq.R2_CurrencyCode;

		// tt : Account Type	// [#RWC6-223] US William 2020-10-20 Add account type to DF 2nd request
		CString accountCode;
		if (m_SourceAccount == S_CREDITCARD)
		{
			accountCode = L"R";
		}
		else if (m_SourceAccount == S_SAVINGS)
		{
			accountCode = L"S";
		}
		else if (m_SourceAccount == S_CHECKING)
		{
			accountCode = L"C";
		}

		if (!accountCode.IsEmpty())
		{
			m_sSTD1_DynamicFlowReq.R2_AccountType = accountCode;
			m_strSendData += FIELD_DELIMITER;
			m_strSendData += L"tt" + m_sSTD1_DynamicFlowReq.R2_AccountType;
		}

	}
	return RES_OK;
}
// End of [#2150]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD1_Make_AssortedField()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD1_Make_AssortedField()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_Make_AssortedField]\n"));

	int nAssortedIndex = 0, i = 0;
	BYTE chAssortedCode = 0;
	UINT nAssorted_FID2 = 0, nTmp = 0;
	CString strTmp, strTmp2;

	// LibertyX will not send assorted fields
	// [#RWC6-59] US William 2019.10.08 LibertyX
	if (m_HostConfig == HC_LIBERTYX)
	{
		return T_ERROR;
	}

	switch (TranCode)
	{
		case TC_INQUIRY:
		case TC_TRANSFER:
		case TC_WITHDRAWAL:				// Transaction Message
		case TC_PINCHANGE:				// [#2150] US Justin 2012.10.01 Add Pin Change
		case TC_POPMONEY:				// [#2350] US Justin 2015.06.19 Add POP Money
		case TC_PAYPALCCA:				// [#2446] US Justin 2016.10.07 Paypay CCA
		case TC_JUSTCASH:				// [#2448] US Justin 2016.11.21 Add Just.Cash
		case TC_JUSTCASH_BITCOIN:		// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
		case TC_PIN4:					// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
		case TC_DIGITALMINT:
			nAssortedIndex = ASSORTED_REQUEST_TRAN;
			break;
		case TC_REVERSAL:				// Reversal Message
			nAssortedIndex = ASSORTED_REQUEST_REVERSAL;
			break;
		case TC_TOTAL:
		case TC_TRIALTOTAL:
			nAssortedIndex = ASSORTED_REQUEST_TOTAL;
			break;
		case TC_HEALTHCHK:
			nAssortedIndex = ASSORTED_REQUEST_HEALTHCHECK;
			break;
		case TC_EXTENDED_EJUPLOAD:		// [#2076] NH KSK 2011.06.28
		// [#560] NH KSK 2009.8.20
		case TC_EXTENDED_AID_UPDATE:
		case TC_EXTENDED_ADDSVC_CONFIG:	// [#2449] US Justin 2016.11.08
			nAssortedIndex = ASSORTED_REQUEST_EXTCONFIG;
			break;
		// End of [#560]
		default:
			return T_ERROR;
	}

	for( i = 0; chAssorted_Requests[nAssortedIndex][i] != 0; i++)
	{
		chAssortedCode = (BYTE)chAssorted_Requests[nAssortedIndex][i];
		switch( chAssortedCode)
		{
			case 'y':
				break;

			case 'u': // Two character FIDs
				nAssorted_FID2 = chAssorted_Requests[nAssortedIndex][i];
				nAssorted_FID2 &= 0xFFFFFF00;

				// 'd' Encoded EMV data block ASCII characters. Variable length.
				if ( (nAssorted_FID2 & FID2_d) == FID2_d )
				{
					if(m_pDevCmn->fnMCU_IsEmvTransaction())
					{
						BIZ_EMV_MakeICDataforTrans(MSG_HYOSUNG_TYPE);		// [#2237] US Justin 2013.12.05 Dual Host DCC EMV field separator Bug fix
					}
					else
					{
						// MS or FallBack 거래
						if (m_pDevCmn->fnMCU_IsEmvEnable())
						{
							// [#2116] NH KSK 2012.03.05 Power Off Reversal시 Track2 Data 감지가 안되어 NVRAM에서 Read하도록 수정함
							// Check IC Card Information
							//int nResult = m_sCardData.strISO2Data.Find(L"=");	// "="이 없는 경우는 위쪽에서 return함
							// Service Code가 "2" or "6"인 경우 POS Entry Mode 전송하도록 함
							//if (m_sCardData.strISO2Data.GetAt(nResult+5) == '2' || m_sCardData.strISO2Data.GetAt(nResult+5) == '6')			// EMV CARD CHECK
							//{
							//	strTmp.Format(L"ud9F390190");
							//	m_strSendData += FIELD_DELIMITER; 
							//	m_strSendData += strTmp;
							//}

							strTmp = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2);
							int nResult = strTmp.Find(L"=");

							// [#2137] MX KSK 2012.07.27
							if (strTmp.GetLength() <= nResult + 5)
								break;
							// end of [#2137]

							// Service Code가 "2" or "6"인 경우 POS Entry Mode 전송하도록 함
							if (nResult != -1)
							{
								// EMV CARD CHECK
								//if (strTmp.GetAt(nResult+5) == '2' || strTmp.GetAt(nResult+5) == '6')			
								//if( (m_pDevCmn->m_bUnKnownAID_SkipPE==FALSE) && (strTmp.GetAt(nResult+5) == '2' || strTmp.GetAt(nResult+5) == '6') )		// [#2436] US Justin 2016.07.22
								if( (m_pDevCmn->m_bUnKnownAID_SkipPE==FALSE) && LIB_IsICCard(strTmp) )		// [#2517]
								{
									strTmp.Format(L"ud9F390190");
									m_strSendData += FIELD_DELIMITER; 
									m_strSendData += strTmp;
									
									// [#2519] NH Justin 2017.12.08 Leave Fallback Indicator in the journal
									CString strOtherMsg = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG);
									if(strOtherMsg.Find(L"F.BACK") < 0)
									{
										if(strOtherMsg.GetLength()>0)
											strOtherMsg += CString(UNIT_DELIMITER);

										CString sNewOtherMsg = L"";
										sNewOtherMsg.Format(L"%s%sF.BACK", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_GENERAL_COMMENT);
										strOtherMsg += sNewOtherMsg;
										MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strOtherMsg );
									}
									// End of [#2519]
								}
							}
							// end of [#2116]
						}
					}
				}

				if ( (nAssorted_FID2 & FID2_h) == FID2_h )
				{
					strTmp.Empty();
					strTmp = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_SCRIPT_RESULTS_TLV);

					if( strTmp.GetLength() != 0 )
					{
						m_strSendData += FIELD_DELIMITER;
						m_strSendData += L"uh" + strTmp;
					}
					strTmp.Empty();
				}
				break;
			
			case 'n':
				// 1: Incorrect dispense, 2: Protocol error, 3: Lost eot, 4: Mac error, 
				// 5: Customer cancellation, 6: VAS challenge/response error, 7: ICC declined transaction
				// 11: Customer did not remove card from cardreader	
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REASONFORREVERSAL_ENABLE) == ENABLE)
				{
					if(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) >= 1 && 
						MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 11)
					{
						strTmp.Format(L"n%02d", MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL));
						m_strSendData += FIELD_DELIMITER;
						m_strSendData += strTmp;
					}
				}
				break;

			// Reversal at Host Error Option이 ENABLE && 시에만 송부한다.
			case 'r':
				if ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REVERSAL_AT_HOSTERROR) == ENABLE) &&
					(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRETRIEVALNUM) == L"000000000000"))	// 2009.10.21 retrieval number가 default인 경우 'r' field를 send하도록 수정
				{
					strTmp.Format(L"r%4.4s", m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig));
					m_strSendData += FIELD_DELIMITER;
					m_strSendData += strTmp;
				}
				break;
			// end of [#563]

			case 's':
				//if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TERMINAL_STATUS_FIELD_ENABLE) == ENABLE)
				if(  (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TERMINAL_STATUS_FIELD_ENABLE) == ENABLE) || (m_HostConfig == HC_DUALHOST) )	// [#2185] US Justin 2013.05.07 Dual Host DCC
				{
					strTmp.Format(L"s%s", BIZ_STD1_Make_TerminalStatusField());
					m_strSendData += FIELD_DELIMITER;
					m_strSendData += strTmp;
				}
				break;

			case 'e':	// KSK 2010.03.13 여기부터 해야함.
				nAssorted_FID2 = chAssorted_Requests[nAssortedIndex][i];
				nAssorted_FID2 &= 0xFFFFFF00;

				// 'et' Current AID List of the terminal.
				if ((nAssorted_FID2 & FID2_t) && (TranCode == TC_EXTENDED_AID_UPDATE))
				{
					if (nTerminalRemainAIDCnt <= 0)
						break;

					int nTempRemainCount = nTerminalRemainAIDCnt;
					CString strTempAIDData;

					NHDEBUG(DBG_CALL, (_T("***TranCmn***CTranCmn::Assorted Terminal Remain Count [%d]\n"), nTempRemainCount));

					// Check Send Total Count
					int i=0;
					for(i=0; i< __min(nTempRemainCount, MAX_AIDLIST_SENDCNT); i++)
					{
						strTmp.Format(L"%S", m_Standard1_Terminal_AIDList.chAID[m_Standard1_Terminal_AIDList.nTotalAIDCount - nTempRemainCount + i]);
						strTempAIDData += strTmp;
						// [#2329] NH Justin 2015.01.26 Bug Fix on Uploading Journal
						//m_strSendData  += AID_FIELD_DELIMITER;
						strTempAIDData  += AID_FIELD_DELIMITER;
						// End of [#2329]

						NHDEBUG(1, (_T("***TranCmn***CTranCmn::Assorted AID [%s]\n"), strTempAIDData));

						nTerminalRemainAIDCnt--;
					}

					if (nTerminalRemainAIDCnt <= 0)
						nTerminalRemainAIDCnt = 0;

					strTmp.Format(L"et%03d%02d%02d", m_Standard1_Terminal_AIDList.nTotalAIDCount, i,  nTempRemainCount / (MAX_AIDLIST_SENDCNT+1));

					m_strSendData += FIELD_DELIMITER;
					m_strSendData += strTmp + strTempAIDData;
					NHDEBUG(DBG_CALL, (_T("***TranCmn***CTranCmn::Total Send Data [%s]\n"), m_strSendData));
				}
				
				// [#2076] NH KSK 2011.06.28
				// 'ej' Journal Upload data field.
				//if ((nAssorted_FID2 & FID2_j) == FID2_j)
				if( ((nAssorted_FID2 & FID2_j) == FID2_j) && (TranCode != TC_EXTENDED_AID_UPDATE) && (TranCode != TC_EXTENDED_ADDSVC_CONFIG) )		// [#2449] US Justin
				{
					NHDEBUG(DBG_CALL, (_T("***TranCmn***CTranCmn::Assorted Fid 2 Value [j]\n")));

					// EJ Upload 구현 (Size가 1000Byte 미만일때까지 EJ를 Upload한다.
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EJUPLOAD_ENABLE) == ENABLE)
					{
						CString strJnlData;

						// 실패 시 Journal Index를 roll back 하기 위해 송신 전 Journal Count를 저장한다.
						// Intialize variable
						m_nUploadLastJnlCount = 0;

						m_nUploadLastJnlCount = m_pDevCmn->m_JNLMgr.GetUploadLastIndex(UP_HOST);
						int nRemainCount = m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_HOST);

						NHDEBUG(DBG_CALL, (_T("***TranCmn***CTranCmn::Remain Journal Count (%d)\n"), m_nUploadLastJnlCount));

						// 한번에 보내는 MAX JOURNAL COUNT는 8로 사양 변경 필요 (근거 : MAX JOURNAL DATA SIZE 125 BYTE * 8 = 1000 BYTE)
						for(int i=0; i<8; i++)
						{
							strJnlData = m_pDevCmn->m_JNLMgr.UploadMoveNext(UP_HOST);

							NHDEBUG(DBG_CALL, (_T("***TranCmn***CTranCmn::Remain Journal Data (%s)\n"), strJnlData));

							if (strJnlData.GetLength() <= 0)
								break;

							strTmp2.Format(L"%s%c", BIZ_STD1_Make_EJUploadData(strJnlData), GROUP_SEPERATOR);
							strTmp += strTmp2;							
						}

						if (strTmp.GetLength() > 0)
						{
							strTmp2.Format(L"ej1%05d%c", nRemainCount, GROUP_SEPERATOR);
							m_strSendData += FIELD_DELIMITER;
							m_strSendData += strTmp2;
							m_strSendData += strTmp;
						}
					}
				}
				// end of [#2076]

				break;

			// [#2150] US Justin 2012.10.01 Add 'x' fields - Dynamic Flow
			case 'x':
				nAssorted_FID2 = chAssorted_Requests[nAssortedIndex][i];
				nAssorted_FID2 &= 0xFFFFFF00;
				if( ((nAssorted_FID2 & FID2_c) == FID2_c)&&(m_sSTD1_DynamicFlowResp.nDCCTransaction==1) ) 		// DCC : Base and Target Currency, Copy from DCC response ec
				{
					strTmp.Format(L"%s", m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency);
					if(strTmp.GetLength()>0)
					{
						m_strSendData += FIELD_DELIMITER;
						m_strSendData += L"xc" + strTmp;
						NHDEBUG(DBG_CALL, (_T("***TranCmn***CTranCmn::Assorted field [xc:%s]\n"),strTmp));
					}
				}
				if( ((nAssorted_FID2 & FID2_f) == FID2_f)&&(m_sSTD1_DynamicFlowResp.nDCCTransaction==1) )		// DCC : Converted Surcharge Amount, Copy from DCC response ef
				{
					strTmp.Format(L"%s", m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge);
					if(strTmp.GetLength()>0)
					{
						m_strSendData += FIELD_DELIMITER;
						m_strSendData += L"xf" + strTmp;
						NHDEBUG(DBG_CALL, (_T("***TranCmn***CTranCmn::Assorted field [xf:%s]\n"),strTmp));
					}
				}
				if( ((nAssorted_FID2 & FID2_r) == FID2_r)&&(m_sSTD1_DynamicFlowResp.nDCCTransaction==1) )		// DCC : Exchange Rate, Copy from DCC response er
				{
					strTmp.Format(L"%s", m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate);
					if(strTmp.GetLength()>0)
					{
						m_strSendData += FIELD_DELIMITER;
						m_strSendData += L"xr" + strTmp;
						NHDEBUG(DBG_CALL, (_T("***TranCmn***CTranCmn::Assorted field [xr:%s]\n"),strTmp));
					}
				}
				if( ((nAssorted_FID2 & FID2_t) == FID2_t)&&(m_sSTD1_DynamicFlowResp.nDCCTransaction==1) )		// DCC : Converted Target Amount, Copy from DCC response et
				{
					strTmp.Format(L"%s", m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount);
					if(strTmp.GetLength()>0)
					{
						m_strSendData += FIELD_DELIMITER;
						m_strSendData += L"xt" + strTmp;
						NHDEBUG(DBG_CALL, (_T("***TranCmn***CTranCmn::Assorted field [xt:%s]\n"),strTmp));
					}
				}
				if( ((nAssorted_FID2 & FID2_x) == FID2_x)&&(TranCode == TC_PINCHANGE) )						// Pin Change
				{
					strTmp.Format(L"%s", m_sUserSelection.strNewPassword);
					if(strTmp.GetLength()>0)
					{
						m_strSendData += FIELD_DELIMITER;
						m_strSendData += L"xx" + strTmp;
						NHDEBUG(DBG_CALL, (_T("***TranCmn***CTranCmn::Assorted field [xx:%s]\n"),strTmp));
					}
				}
				// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1 message 
				if( ((nAssorted_FID2 & FID2_m) == FID2_m)&&(m_sSTD1_DynamicFlowResp.nDCCTransaction==1) )		// DCC : Exchange Rate Mark-Up, Copy from DCC response
				{
					strTmp.Format(L"%s", m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRateMarkUp);
					if(strTmp.GetLength()>0)
					{
						m_strSendData += FIELD_DELIMITER;
						m_strSendData += L"xm" + strTmp;
						NHDEBUG(DBG_CALL, (_T("***TranCmn***CTranCmn::Assorted field [xm:%s]\n"),strTmp));
					}
				}
				// end of [#RWC6-2, #2585]
				break;
			// End of [#2150]

			// [#2449] US Justin 2016.11.08 Add 'a' fields - Mobile Cash Access
			case 'a':
				if(TranCode == TC_EXTENDED_ADDSVC_CONFIG)
				{
					nAssorted_FID2 = chAssorted_Requests[nAssortedIndex][i];
					nAssorted_FID2 &= 0xFFFFFF00;
					if( (nAssorted_FID2 & FID2_e) == FID2_e )
					{
						m_strSendData += FIELD_DELIMITER;
						m_strSendData += L"ae1";
						NHDEBUG(DBG_CALL, (_T("***TranCmn***CTranCmn::Assorted field [ae:1]\n")));
					}
				}
				break;
			// End of [#2449]

			default:
				break;
		}
	}
	
	return T_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD1_Make_TerminalStatusField()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CString CTranCmn::BIZ_STD1_Make_TerminalStatusField()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_Make_TerminalStatusField]\n"));

	CString strResult;
	CString strTemp;

	// 1. Terminal Platform (1) [P, C, X (C - Window CE Platform)]
	strResult = L"C";		

	// 2. Program Version Number (8) [XX.YY.ZZ]
	strTemp.Format(L"%8.8s", MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_AP_VERSION).Right(8));
	// [#2453] US Justin 2016.11.29 NHS2.8 US AP Type
	//strResult += strTemp;
	#if (US_VERSION || CA_VERSION || MX_VERSION)
		strResult+= MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_AP_VERSION).Left(1) + strTemp.Right(7);
	#else
		strResult += strTemp;
	#endif
	// End of [#2453]

	// 3. Mode Type (1) ["I", "O", "F"]
	if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSTATUS) == ATM_CLERK)
		strResult += L"F";	// Supervisor Active
	else 
	if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSTATUS) == ATM_INIT		|| 
	    MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSTATUS) == ATM_READY	||
		MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSTATUS) == ATM_ERROR)
		strResult += L"O";	// Out of Service
	else
		strResult += L"I";	// In Service

	// 4. Current Error Code (7)
	strTemp.Format(L"%7.7s", m_pDevCmn->fstrAPL_GetErrorCode());
	strResult += strTemp;

	// 5. Chest Door Open (1) ["0", "1", "X"]
	if (m_pDevCmn->fnDOR_GetDoorStatus() == DOOR_OPENED)	// [#483] [NH] KSK 2009.2.6
		strResult += L"0";
	else
		strResult += L"1";
	// 6. Top Door Open (1) ["0", "1", " ", "X"] - Not Support
	strResult += L" ";

	// 7. Card Reader Type (1) ["0"(SWIPE), "1"(DIP), "2"(MotorDriven), "3"(DIP EMV), "4"(MotorDriven EMV), "X"(Not Installed)]
	// [#2453] US Justin 2016.11.29 NHS2.8
	//strResult += L"1";
	if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_NH_DIP)				strResult += L"1";		// NH DIP
	else															strResult += L"3";		// Sankyo or Magtek
	// End of [#2453]

	// 8. Card Reader Status (1) ["0", "1", " ", "X"]
	if (m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_MCU))
		strResult += L"1";
	else
		strResult += L"0";
	
	// 9. Cards Retained (3)  000-999, <space> Not supported
	strTemp.Format(L"%3.3s", L"");
	strResult += strTemp;

	// 10. PIN Pad Type (1) ["0", "1", "2", "3", "X"] - 512K 사양추가 필요 ("0","1"사용안함)
	if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_VISA_EP_VERSION) <= 0)
		strResult += L"2";	// PrePCI EPP (VISA EPP)
	else if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_PCI_EP_VERSION) <= 0)
		strResult += L"3";	// PCI EPP
	else
		strResult += L"3";	// Default Value	(source review후 default는 PCI로 결정함)

	// 11. PIN Pad Status (1) ["0", "1", " ", "X"]
	if (m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_PIN))
		strResult += L"1";
	else
		strResult += L"0";

	// 12. Receipt Printer Type (1) ["0", "1", "X"]
	strResult += L"0";

	// 13. Receipt Printer Status (1) ["0", "1" " ", "X"]
	if (m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_SPR))
		strResult += L"1";
	else
		strResult += L"0";

	// 14. Receipt Printer Paper Status (1) ["0", "1", "2", " ", "X"]
	if( m_pDevCmn->fnSPR_GetPaperStatus() == SLIP_NORMAL )
		strResult += L"0";
	else if(m_pDevCmn->fnSPR_GetPaperStatus() == SLIP_LOW_END)
		strResult += L"1";
	else if(m_pDevCmn->fnSPR_GetPaperStatus() == SLIP_EMPTY_PAPER)
		strResult += L"2";
	else
		strResult += L"0";
	
	// 15. Journal Printer Type (1) ["0", "1", "2", "X"]
	strResult += L"1";

	// 16. Journal Printer Status (1) ["0", "1", "2", "3", " ", "X"]
	strResult += L"0";

	// 17. Journal Printer Paper Status (1) ["0", "1", "2", " ", "X"]
	strResult += L"0";

	// 18. New Journal Count (4) ["0000" ~ "9999"]
	strTemp.Format(L"%04d", m_pDevCmn->m_JNLMgr.GetLastIndex());
	strResult += strTemp;

	// 19. Cash Dispense Type (1) ["0", "1", "2", "X"]
	strResult += L"0";

	// 20. Cash Dispense Status (1) ["0", "1", " ", "X"]
	BOOL	bCheckLowStatus = FALSE;
	int		nNoteCount = m_pDevCmn->fnCDU_GetNumberOfAllCSTs();
	int		CstNoteStatus = m_pDevCmn->fnCDU_GetAllCSTStatus();

	if ((m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_CDU))		||	// CDU 장애 시
							(CstNoteStatus == CST_SET_NG)	||	// CST 탈착 시 (20001)
							(CstNoteStatus == CST_EMPTY)	||	// 논리적인 금액이 없을 경우 (F0001)
							(CstNoteStatus == CST_NEAR))		// Low Sensor Check Enable시 (20002)
		strResult += L"1";		// out of service
	else
		strResult += L"0";		// ok

	// 21. Cash Dispenser Note Status (1) ["0", "1", "2", " "]
	if ( nNoteCount <= 0)	// 전체 매수가 0인 경우 (음수값과 양수값 합이 0인 경우에는 문제 소지 있음) - KSK 2009.3.2
	{
		strResult += L"2";	// OUT OF SERVICE
	}
	else
	{
		if ((m_pDevCmn->CSTCnt > 0) && (m_pDevCmn->CSTCnt <= CDU_MAX_CST_COUNT))
		{
			for(int i=0; i<m_pDevCmn->CSTCnt; i++)	// 논리적인 매수가 LOW인 경우 Check
			{
				if ((m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1 + i) > CDU_NEARCASHCNT) && (MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_LOWSENSOR + i) == 1))
				{
					// 논리적인 매수 100매 초과 && low sensor 미감지시 normal
					bCheckLowStatus = FALSE;
					break;
				}
				else if ((m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1 + i) <= CDU_NEARCASHCNT) || (MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_LOWSENSOR + i) == 0))
				{
					// 논리적인 매수 100매 이하 or low sensor 감지시 low
					bCheckLowStatus = TRUE;
				}
			}
			
			if (bCheckLowStatus == TRUE)	// 전체 Cassette가 LOW이면
				strResult += L"1";	// LOW
			else
				strResult += L"0";	// OK
		}
		else
			strResult += L"2";	// OUT OF SERVICE	CSTCNT값이 0 or 4보다 큰 경우 OUT으로 return
	}

	// 22. 1st Cassette Denomination	(3)
	// 23. 1st Cassette Notes Count		(4)
	// 24. 1st Cassette Notes Loaded	(4)
	// 25. 1st Cassette Notes Dispensed (4)
	// 26. 1st Cassette Notes Rejected	(3)
	CString Denomination3	= L"000";
	CString NotesCount4		= L"0000";	
	CString LoadedCount4	= L"0000";
	CString DispensedCount4	= L"0000";
	CString RejectedEvent3	= L"000";

	int nNumberofCst = m_pDevCmn->fnCDU_GetNumberOfCST();
	int nDenomination = 0;
	int nDivider = 0;

	// Cassette A 
	if( nNumberofCst >= 1)
	{
		nDenomination = m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_1);
		// DENOMINATION REPORT 오류 수정
		// 권종이 $10인 경우 1K로 보내줘야함 ($25인 경우에는 사양적으로 문제소지 있음)
		////////////////////////////////////////////////////////////////////////////////////
		if( (nDenomination%10) == 0 )
		{
			nDivider = nDenomination / 10;
			Denomination3.Format(L"%02dK", nDivider);
		}
		else
		{
			nDivider = nDenomination * 100;		// [#555] NH KSK 2009.08.11 $1 -> 100, $5 -> 500으로 변경하도록 사양 변경 (NHA협의사항)
			Denomination3.Format(L"%03d", nDivider);
		}

		// [#2270] AU KSK 2014.05.26
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
		{
			NotesCount4.Format(L"%4.4d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1));
			LoadedCount4.Format(L"%4.4d", m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_1));
		}
		else
		{
			// -차감이 될 수 있으므로 -일경우 Host로는 0으로 전송한다.
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP) < 0)
				NotesCount4 = L"0000";
			else
				NotesCount4.Format(L"%4.4d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP));

			LoadedCount4.Format(L"%4.4d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_INITIALCOUNT_AP));
		}
		// end of [#2270]

		DispensedCount4.Format(L"%4.4d",m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_1));
		RejectedEvent3.Format(L"%3.3d",m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_1));
	}
	// [#564] NH KSK 2009.8.20 보완 처리
	strResult += Denomination3.Right(3)	+ NotesCount4.Right(4) + LoadedCount4.Right(4) +
				 DispensedCount4.Right(4) + RejectedEvent3.Right(3);
	// end of [#564]

	// 27. 2st Cassette Denomination	(3)
	// 28. 2st Cassette Notes Count		(4)
	// 29. 2st Cassette Notes Loaded	(4)
	// 30. 2st Cassette Notes Dispensed (4)
	// 31. 2st Cassette Notes Rejected	(3)
	Denomination3	= L"000";
	NotesCount4		= L"0000";	
	LoadedCount4	= L"0000";
	DispensedCount4	= L"0000";
	RejectedEvent3	= L"000";
	
	// Cassette B
	if( nNumberofCst >= 2)
	{
		nDenomination = m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_2);

		if( (nDenomination%10) == 0 )
		{
			nDivider = nDenomination / 10;
			Denomination3.Format(L"%02dK", nDivider);
		}
		else
		{
			nDivider = nDenomination * 100;		// [#555] NH KSK 2009.08.11 $1 -> 100, $5 -> 500으로 변경하도록 사양 변경 (NHA협의사항)
			Denomination3.Format(L"%03d", nDivider);
		}

		// [#2270] AU KSK 2014.05.26
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
		{
			NotesCount4.Format(L"%4.4d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_2));
			LoadedCount4.Format(L"%4.4d", m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_2));
		}
		else
		{
			// -차감이 될 수 있으므로 -일경우 Host로는 0으로 전송한다.
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST2_CURRENTCOUNT_AP) < 0)
				NotesCount4 = L"0000";
			else
				NotesCount4.Format(L"%4.4d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST2_CURRENTCOUNT_AP));

			LoadedCount4.Format(L"%4.4d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST2_INITIALCOUNT_AP));
		}
		// end of [#2270]

		DispensedCount4.Format(L"%4.4d",m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_2)) ;
		RejectedEvent3.Format(L"%3.3d",m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_2));
	}
	// [#564] NH KSK 2009.8.20 보완 처리
	strResult += Denomination3.Right(3)	+ NotesCount4.Right(4) + LoadedCount4.Right(4) +
				 DispensedCount4.Right(4) + RejectedEvent3.Right(3);
	// end of [#564]

	// 32. 3st Cassette Denomination	(3)
	// 33. 3st Cassette Notes Count		(4)
	// 34. 3st Cassette Notes Loaded	(4)
	// 35. 3st Cassette Notes Dispensed (4)
	// 36. 3st Cassette Notes Rejected	(3)
	Denomination3	= L"000";
	NotesCount4		= L"0000";	
	LoadedCount4	= L"0000";
	DispensedCount4	= L"0000";
	RejectedEvent3	= L"000";

	// Cassette C
	if( nNumberofCst >= 3)
	{
		nDenomination = m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_3);

		if( (nDenomination%10) == 0 )
		{
			nDivider = nDenomination / 10;
			Denomination3.Format(L"%02dK", nDivider);
		}
		else
		{
			nDivider = nDenomination * 100;		// [#555] NH KSK 2009.08.11 $1 -> 100, $5 -> 500으로 변경하도록 사양 변경 (NHA협의사항)
			Denomination3.Format(L"%03d", nDivider);
		}

		// [#2270] AU KSK 2014.05.26
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
		{
			NotesCount4.Format(L"%4.4d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_3));		
			LoadedCount4.Format(L"%4.4d", m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_3));
		}
		else
		{
			// -차감이 될 수 있으므로 -일경우 Host로는 0으로 전송한다.
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST3_CURRENTCOUNT_AP) < 0)
				NotesCount4 = L"0000";
			else
				NotesCount4.Format(L"%4.4d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST3_CURRENTCOUNT_AP));

			LoadedCount4.Format(L"%4.4d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST3_INITIALCOUNT_AP));
		}
		// end of [#2270]

		DispensedCount4.Format(L"%4.4d", m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_3)) ;
		RejectedEvent3.Format(L"%3.3d",m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_3));
	}
	// [#564] NH KSK 2009.8.20 보완 처리
	strResult += Denomination3.Right(3)	+ NotesCount4.Right(4) + LoadedCount4.Right(4) +
				 DispensedCount4.Right(4) + RejectedEvent3.Right(3);
	// end of [#564]

	// 37. 4st Cassette Denomination	(3)
	// 38. 4st Cassette Notes Count		(4)
	// 39. 4st Cassette Notes Loaded	(4)
	// 40. 4st Cassette Notes Dispensed (4)
	// 41. 4st Cassette Notes Rejected	(3)
	Denomination3	= L"000";
	NotesCount4		= L"0000";	
	LoadedCount4	= L"0000";
	DispensedCount4	= L"0000";
	RejectedEvent3	= L"000";

	// Cassette D
	if( nNumberofCst >= 4)
	{
		nDenomination = m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_4);

		if( (nDenomination%10) == 0 )
		{
			nDivider = nDenomination / 10;
			Denomination3.Format(L"%02dK", nDivider);
		}
		else
		{
			nDivider = nDenomination * 100;		// [#555] NH KSK 2009.08.11 $1 -> 100, $5 -> 500으로 변경하도록 사양 변경 (NHA협의사항)	
			Denomination3.Format(L"%03d", nDivider);
		}

		// [#2270] AU KSK 2014.05.26
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
		{
			NotesCount4.Format(L"%4.4d", m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_4));			
			LoadedCount4.Format(L"%4.4d", m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_4));
		}
		else
		{
			// -차감이 될 수 있으므로 -일경우 Host로는 0으로 전송한다.
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST4_CURRENTCOUNT_AP) < 0)
				NotesCount4 = L"0000";
			else
				NotesCount4.Format(L"%4.4d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST4_CURRENTCOUNT_AP));

			LoadedCount4.Format(L"%4.4d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST4_INITIALCOUNT_AP));
		}
		// end of [#2270]

		DispensedCount4.Format(L"%4.4d", m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_4));
		RejectedEvent3.Format(L"%3.3d", m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_4));
	}
	// [#564] NH KSK 2009.8.20 보완 처리
	strResult += Denomination3.Right(3)	+ NotesCount4.Right(4) + LoadedCount4.Right(4) +
				 DispensedCount4.Right(4) + RejectedEvent3.Right(3);

	// WINCE not support
	// 42. 1st Depository Type (1) ["0", "1", "2", "X"]
	strResult += L"X";
	// 43. 1st Depository Status (1) ["0", "1", " ", "X"]
	strResult += L"X";
	// 44. 1st Depository Note Status (1) ["0", "1", "2", " "]
	strResult += L" ";
	// 45. 2st Depository Type (1) ["0", "1", "2", "X"]
	strResult += L"X";
	// 46. 2st Depository Status (1) ["0", "1", " ", "X"]
	strResult += L"X";
	// 47. 2st Depository Note Status (1) ["0", "1", "2", " "]
	strResult += L" ";
	// 48. 3st Depository Type (1) ["0", "1", "2", "X"]
	strResult += L"X";
	// 49. 3st Depository Status (1) ["0", "1", " ", "X"]
	strResult += L"X";
	// 50. 3st Depository Note Status (1) ["0", "1", "2", " "]
	strResult += L" ";
	
	// [#2453] US Justin 2016.11.29 NHS2.8
	#if !(AU_VERSION)		// KSK 2017.01.06 AU는 전문 변경을 하면 인증에 문제가 되므로 일단 하기 Spec 미적용
		// 51. ATM Type (2 bytes)
		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND);
		if (GetConfigFuncPointer()->GetCEVersion() == WINCE_7)
		{
			if		(strTemp == L"MX2600SE")	strResult += L"20";
			else if	(strTemp == L"MX4000W")		strResult += L"23";
			else if	(strTemp == L"MX5200SE")	strResult += L"24";
			else if	(strTemp == L"MX5300SE")	strResult += L"25";
			else if	(strTemp == L"MX2800SE")	strResult += L"21";
			else if (strTemp == L"MX2800T")		strResult += L"22";
			else if (strTemp == L"MX5400")		strResult += L"26";
			else if (strTemp == L"NH1500SE")	strResult += L"15";
			else if (strTemp == L"NH1800SE")	strResult += L"16";
			else if (strTemp == L"NH2600SE")	strResult += L"17";
			else if (strTemp == L"NH2700")		strResult += L"18";
			else if (strTemp == L"NH2700T")		strResult += L"18";
			else if (strTemp == L"MX5000SE")	strResult += L"19";
			else								strResult += L"20";		// Default HALO II
		}
		else
		{
			if		(strTemp == L"NH1500SE")	strResult += L"04";
			else if	(strTemp == L"NH1800SE")	strResult += L"05";
			else if	(strTemp == L"NH2600")		strResult += L"06";
			else if	(strTemp == L"MX2600SE")	strResult += L"07";
			else if	(strTemp == L"NH2700T")		strResult += L"08";
			else if	(strTemp == L"MX4000W")		strResult += L"09";
			else if	(strTemp == L"MX5000SE")	strResult += L"10";
			else if	(strTemp == L"MX5200SE")	strResult += L"11";
			else if	(strTemp == L"MX5300SE")	strResult += L"12";
			else if	(strTemp == L"MX2800SE")	strResult += L"13";		// [#2551] NH Justin 2018.05.24 Add MX2800SE in Status Monitoring Field
			else if (strTemp == L"MX2800T")		strResult += L"14";		// [#RWC6-91] US William 2019.12.05 Add LTX+B4U Status Monitoring Fields
			else								strResult += L"08";		// Default 2700
		}


		// 52. OS Version (8 bytes)
		strTemp.Format(L"%8.8s", MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_OS_VERSION).Right(8) );
		strResult += strTemp.Left(8);

		// 53. Transaction Capability (10 bytes)
		BYTE bt1, bt2, bt3, bt4, bt5;
		bt1 = bt2 = bt3 = bt4 = bt5 = 0;
		// EMV ---------------------------------------------------------------------------
		if (P_EMV_CheckTransMode() == RES_EMV_IC_FIRST)		
			bt1 |= 0x80;
		// DCC ---------------------------------------------------------------------------
		if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) == ENABLE) || (MemGetInt( _MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC)==ENABLE) )
			bt1 |= 0x40;
		// PopMoney ---------------------------------------------------------------------------
		if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_ENABLE)==ENABLE)
			bt1 |= 0x20;
		// Pin4 ---------------------------------------------------------------------------
		if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE)==ENABLE) && (m_Pin4.m_bPin4Available==TRUE) )
			bt1 |= 0x10;
		// Pin Change ---------------------------------------------------------------------------
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE) != 0)
			bt1 |= 0x08;
		// Mobile Phone Topup ---------------------------------------------------------------------------
			// 0x04 => SKIP
		
#if (APP_CUSTOM_PAI)
		// PAI PIN4 Symmertic Key Flag -------------------------------------------------------------
		// [#RWC6-57] US William 2019.09.18 PAI MCCP Key presence
		if( m_Pin4.m_bPin4Available==TRUE
			&& MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE) == ENABLE
			&& m_Pin4.HaveSymmetricKey() )
			bt1 |= 0x02;
#endif
		// Paypal CCA ---------------------------------------------------------------------------
		#if(APP_PAYDIANT_CCA)
			if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE)==ENABLE) && (m_PayPalCCA.m_bPaypalCCAAvailable==TRUE) )
				bt1 |= 0x01;
		#endif

		// [#2554] NH Justin 2018.06.06 Gipay and Dual Balance Status
		// GivePay Status ---------------------------------------------------------------------------
		#if (APP_GPAY_GIFTCARD_PURCHASE)
			if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_ENABLE_DISABLE) == ENABLE) && 	(m_GivePayData.m_bGivePayAvailable == TRUE) )
				bt2 |= 0x20;
		#endif
		// Dual Balance Status ---------------------------------------------------------------------------
		#if (APP_DUALBALANCE)
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE)==ENABLE )
				bt2 |= 0x10;
		#endif
		// End of [#2554]

		// [#RWC6-91] US William 2019.12.05 Add LTX+B4U Status Monitoring Fields
		// LibertyX Buy Bitcoin+Available ---------------------------------------------------------------------------
		#if (APP_LIBERTYX)
			if (m_LXIsConfigured && m_LXConfig.BuyBitcoinEnabled)
			{
				bt2 |= 0x08;
			}
		#endif

		// Bitload4U Enabled ---------------------------------------------------------------------------
		#if (APP_B4U)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_B4U_ENABLED)==ENABLE)
			{
				bt2 |= 0x04;
			}
		#endif
		// End of [#RWC6-91]

		// US Jefferson 2020.06.16 Add bit if credit surcharge is non-zero, only for PAI
		// To add credit card segmentation for US general version as well.
		#if (US_VERSION && !APP_TDL_OPTION)
			if (Asc2Int(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE)) != 0) 
			{
				bt2 |= 0x02;
			}
		#endif

		// [#RWC6-244] US William 2021.01.12 PAI DF Byte
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC) == ENABLE) 
		{
			bt2 |= 0x01;
		}

		// [#RWC6-296] US William 2021.07.21 Add LTX Cash Out Status Monitoring Fields
		// LibertyX Cash Out + Available ---------------------------------------------------------------------------
#if (APP_LIBERTYX)
		if (m_LXIsConfigured && m_LXConfig.SellBitcoinEnabled)
		{
			bt3 |= 0x80;
		}
#endif

		strTemp.Format(L"%02X%02X%02X%02X%02X", bt1, bt2, bt3, bt4, bt5);
		strResult += strTemp;
		// End of [#2453]

		// 54. Communication Type (1byte)		// [#J005]Adding Comm Types (MODEM/TCPIP) in Standard1 Status Monitoring		
		if(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP) 	strResult += L"0";
		else																				strResult += L"1";	
		// End of [#J005]

		CString strCThsm = m_pDevCmn->fstrPIN_TR34_GetCertificateData(); // RWC6-379 TR34 Bind
		NVDump('O', 'A', "02", L"", L"1:T34GetCertificateData()");		 // RWC6-379 To test only

		if (strCThsm.GetLength() > 0)
		{
			NHDEBUG(DBG_INFO, (_T("EPP IS BOUND! \n")));
			strResult += L"1";
		}
		else
		{
			NHDEBUG(DBG_INFO, (_T("EPP IS NOT BOUND YET! \n")));
			strResult += L"0";
		}

		strTemp = MemGetVersion(_MEMKEY_EPVERSION, L"PIN").Left(3);
		NVDump('O', 'A', "02", L"", L"1:GetEPPEPVersion()"); // RWC6-379 To test only

		if (strTemp == L"V03" || strTemp == L"V05" || strTemp == L"V07" || strTemp == L"V08")
		{
			strResult += L"1";
		}
		else if (strTemp == L"V10")
		{
			strResult += L"2";
		}
		else if (strTemp == L"V11" || strTemp == L"V12")
		{
			strResult += L"3";
		}
		else if (strTemp == L"V14" || strTemp == L"V15")
		{
			strResult += L"4";
		}
		else
		{
			strResult += L"0";
		}

		// 55. Reserved
		strTemp.Format(L"%17.17s", L"");			// [#2453] US Justin 2016.11.29 NHS2.8
	#else
		strTemp.Format(L"%40.40s", L"");
	#endif
	
	strResult += strTemp;

	return strResult;
}

// [#2076] NH KSK 2011.06.29
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_Make_EJUploadData()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : HOST로 전송할 EJUPLOAD Data를 편집하는 함수
-------------------------------------------------------------------*/
CString	CTranCmn::BIZ_STD1_Make_EJUploadData(CString strJnlData)
{
	CStringArray	arJnlField;
	CString			strResult, strTemp, strTemp2;
	int				nTemp = 0;

	NHDEBUG(DBG_CALL, (_T("***TranCmn*** BIZ_STD1_Make_EJUploadData() Journal Data [%s]\n"), strJnlData));

	SplitString(strJnlData,"^", arJnlField);

	if (arJnlField.GetSize() <= 1)
		return L""; // Data Error

	arJnlField[1].TrimLeft();
	arJnlField[1].TrimRight();

	void* formatter;
	if (m_ejFormatters->Lookup(arJnlField[1], formatter))
	{
		strResult = (JournalFormatter(formatter))(arJnlField);
	}
	else
	{
		// Unknown journals can be Operator actions
		strResult = CreateOperatorActionJournalString(arJnlField);
	}

	return strResult;
}
// end of [#2076]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD1_RecvHost()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD1_RecvHost()
{
	NHDEBUG(1, (_T("[CTranCmn::BIZ_STD1_RecvHost]\n")));

	//////////////////////////////////////////
	// 1. Recv Data
	if (BIZ_RecvData() != RES_OK)
	{
		if(m_pDevCmn->fnMCU_IsEmvTransaction())					// [#397] [NH] psc 2008.08.01 for EMV transaction
		{
			// [#2496] US Justin 2017.08.15 Add Bitcoin
			// [#RWC6-59] US William 2019.10.08 LibertyX
			if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_JUSTCASH_BITCOIN || TranCode == TC_DIGITALMINT)
			{
				if (m_bOnlineProcessing == FALSE)
				{
					// KSK 2010.08.29 통신 장애인 경우 Default 처리 추가
					m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

					if (m_pDevCmn->fnEMV_Trans_OnlineProcess(0, AAC) == EMV_RSLT_OK)
					{
						m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
						m_pDevCmn->fnEMV_Trans_Completion();
					}

					// KSK 2010.09.03 CID가 안바뀔 경우 reason for reversal code "08"로 설정
					if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
					{
						int nLen = 0;
						CString strValue;
						unsigned char	szTemp[1024] = {0,};

						if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_CryptInfData, &nLen, szTemp) == EMV_RSLT_OK)
						{
							strValue = MakeUnPack(szTemp, nLen);

							// CID가 ARQC (0x80)인 경우 Reason for Reversal을 "08"로 설정
							if (strValue == L"80")
							{
								if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
									MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 8);
							}
						}
					}
					// end of KSK 2010.09.03

					BIZ_EMV_MakeICDataforReversal();	// KSK 2009.8.31 송신 에러시에 

					m_bOnlineProcessing = TRUE;
				}
			}
		}

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
		nRes = BIZ_STD1_AnalHostData();											// Recv Anal	[#492] [NH] KSK 2009.2.5

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
 FUNCTION NAME: BIZ_STD1_AnalHostData()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD1_AnalHostData()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_AnalHostData]\n"));

	int		Cnt = 0;
	char	chMsg[200] = { 0, };
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
		case TC_JUSTCASH_BITCOIN:	// [#2496] US Justin 2017.08.15 Add Bitcoin
		case TC_PIN4:				// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
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
			m_sSTD1_CommRespHeader.RecordFormat_1		= m_strArrRecvData[Cnt].Mid(0,1);
			m_sSTD1_CommRespHeader.ApplicationType_1		= m_strArrRecvData[Cnt].Mid(1,1);
		}
	}
	m_sSTD1_CommRespHeader.MessageDelimiter_1	= ".";
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD1_CommRespHeader.BankID_6 = m_strArrRecvData[Cnt];
#else

	// [#4] NH PSC 2008.03.10 Mac result 체크.
	// keymode: 5, 6, 7 and 출금, 조회, 이체, 취소거래일때만.
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	// [#2150] US Justin 2012.10.05 Add Pin Change Transaction 
	// [#2289] US Justin 2014.08.13 Add Donation 
	// [#2350] US Justin 2015.06.19 Add POP Money
	// [#2415] US Justin 2016.04.15 Add WalPay
	// [#2446] US Justin 2016.10.07 Add Paypal
	// [#2448] US Justin 2016.11.21 Add Just.Cash
	// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
	// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
//	if( (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	||
//		 MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		||
//		 MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
//		 MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING) &&			// [#565] NH KSK 2009.8.20
//		(TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY  || TranCode == TC_TRANSFER || TranCode == TC_CHECKCASHING || TranCode == TC_REVERSAL || 
//		 TranCode == TC_PINCHANGE  || TranCode == TC_DONATION || TranCode == TC_POPMONEY || TranCode == TC_WALPAY		|| TranCode == TC_PAYPALCCA||
//		 TranCode == TC_JUSTCASH) ) 

	int nEPPKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);
	if( nEPPKeyMode== KEYMODE_NON_UNIQ_SDES_MACING	|| nEPPKeyMode == KEYMODE_UNIQ_SDES_MACING	||
		nEPPKeyMode == KEYMODE_TDES_MACING			|| nEPPKeyMode == KEYMODE_TDES_TMACING		||
		nEPPKeyMode == KEYMODE_TR31_MACING			|| nEPPKeyMode == KEYMODE_TR31_TMACING		)
	{
		// [#2490] NH Justin 2017.06.15 Standard1 Dynamic Flow MAC Option
		/*
		#if (AU_VERSION)		// KSK 2017.01.25 AU인 경우 1st / 2nd Call시에도 MAC 사용함. 이에 가독성을 위해 로직 변경
		if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY  || TranCode == TC_TRANSFER || TranCode == TC_CHECKCASHING || TranCode == TC_REVERSAL || 
			TranCode == TC_PINCHANGE  || TranCode == TC_DONATION || TranCode == TC_POPMONEY || TranCode == TC_WALPAY		|| TranCode == TC_PAYPALCCA||
			TranCode == TC_JUSTCASH   || TranCode == TC_DYNAMICFLOWL_1ST || TranCode == TC_DYNAMICFLOWL_2ND) 
		#else
		if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY  || TranCode == TC_TRANSFER || TranCode == TC_CHECKCASHING || TranCode == TC_REVERSAL || 
		    TranCode == TC_PINCHANGE  || TranCode == TC_DONATION || TranCode == TC_POPMONEY || TranCode == TC_WALPAY		|| TranCode == TC_PAYPALCCA||
		    TranCode == TC_JUSTCASH)
		#endif					// end of KSK 2017.01.25
		*/

		BOOL bCheckMAC = FALSE;

		if (TranCode == TC_WITHDRAWAL		|| TranCode == TC_INQUIRY	|| TranCode == TC_TRANSFER	|| TranCode == TC_REVERSAL	||
			TranCode == TC_PINCHANGE		|| TranCode == TC_POPMONEY	|| TranCode == TC_PAYPALCCA	|| TranCode == TC_JUSTCASH	||
			TranCode == TC_JUSTCASH_BITCOIN	|| TranCode == TC_PIN4		|| TranCode == TC_DIGITALMINT)
		{
			bCheckMAC = TRUE;
		}
		else if (TranCode == TC_DYNAMICFLOWL_1ST || TranCode == TC_DYNAMICFLOWL_2ND)
		{
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_STD1_DYNAMICFLOW_MAC) == ENABLE)
				bCheckMAC = TRUE;
		}

		if(bCheckMAC==TRUE)		// End of [#2490]
		{
			strTemp.Format(L"%S", m_arRecvBuffer);
			memset(m_szTemp, NULL, sizeof(m_szTemp));
			WideToMulti(m_szTemp, strTemp, strTemp.GetLength()-9);	// MAC field의 길이를 제외한다.

			m_pDevCmn->fnPIN_MacingData(MakeUnPack(m_szTemp, strTemp.GetLength()-9));	// MAC field의 길이를 제외한다.
			CString PinMacingData = m_pDevCmn->fstrPIN_GetMacingData();

			if (PinMacingData.Left(8).CompareNoCase(strTemp.Right(8)) != 0)
			{
				NVDump('F', 'H', "20", L"", L"MAC_ERROR");	// [#2362] AU KSK 2015.08.03

				if(m_pDevCmn->fnMCU_IsEmvTransaction())		// [#397] [NH] psc 2008.08.01 for EMV transaction
				{
					// [#2496] US Justin 2017.08.15 Add Bitcoin
					// [#RWC6-59] US William 2019.10.08 LibertyX
					if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_JUSTCASH_BITCOIN || TranCode == TC_DIGITALMINT)
					{
						if (m_bOnlineProcessing == FALSE)
						{
							// KSK 2010.08.29 MAC ERROR 경우 Default 처리 추가
							m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

							if (m_pDevCmn->fnEMV_Trans_OnlineProcess(0, AAC) == EMV_RSLT_OK)
							{
								m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
								m_pDevCmn->fnEMV_Trans_Completion();
							}

							// KSK 2010.09.03 CID가 안바뀔 경우 reason for reversal code "08"로 설정
							if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
							{
								int nLen = 0;
								CString strValue;
								unsigned char	szTemp[1024] = {0,};

								if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_CryptInfData, &nLen, szTemp) == EMV_RSLT_OK)
								{
									strValue = MakeUnPack(szTemp, nLen);

									// CID가 ARQC (0x80)인 경우 Reason for Reversal을 "08"로 설정
									if (strValue == L"80")
									{
										if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
											MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 8);
									}
								}
							}
							// end of KSK 2010.09.03

							BIZ_EMV_MakeICDataforReversal();	// KSK 2009.8.31 송신 에러시에 

							m_bOnlineProcessing = TRUE;
						}
					}
				}

				if (LIB_IsReversalCondition())
				{
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_CONF_TRAN);	// reversal flag가 3이면 configuration 후 reversal을 한다.
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 4);				// [#12] NH PSC 2008.03.24 reson for reversal 값 셋팅.
				}
				else if(TranCode == TC_REVERSAL)
				{
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_CONF_TRAN);	// reversal flag가 3이면 configuration 후 reversal을 한다.
				}
				else if(TranCode == TC_TRANSFER || TranCode == TC_INQUIRY)
				{
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_CONF);	// reversal flag가 2이면 configuration 만 한다.
				}
				m_pDevCmn->fnAPL_StackError(_T("D00C2"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003032), DEV_NET);	// [#419] [NH] KSK 2008.9.16

				return RES_HOST_RECV_ERR;
			}
		}
	}
	// end of [#4]

	m_strRecvData.Format(L"%S", m_arRecvBuffer);
	m_strArrRecvData.RemoveAll();
	SplitString(m_strRecvData, FIELD_DELIMITER, m_strArrRecvData);
	
	if(m_strArrRecvData[Cnt].GetLength() >= 1)
		m_sSTD1_CommRespHeader.RecordFormat_1		= m_strArrRecvData[Cnt].Mid(0,1);

	if(m_strArrRecvData[Cnt].GetLength() >= 2)
		m_sSTD1_CommRespHeader.ApplicationType_1	= m_strArrRecvData[Cnt].Mid(1,1);

	if(m_strArrRecvData[Cnt].GetLength() >= 3)
		m_sSTD1_CommRespHeader.MessageDelimiter_1	= m_strArrRecvData[Cnt].Mid(2,1);

	if(m_strArrRecvData[Cnt].GetLength() >= 8)
		m_sSTD1_CommRespHeader.BankID_6				= m_strArrRecvData[Cnt].Mid(3,6);

#endif

	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD1_CommRespHeader.TerminalID_8 = m_strArrRecvData[Cnt].Left(8);

	Cnt++;

	switch (TranCode)
	{
		case TC_OPEN:
			nRes = BIZ_STD1_AnalConfigMsg(Cnt);
			break;
		case TC_EXTENDED_EJUPLOAD:		// [#2076] NH KSK 2011.06.28
		case TC_EXTENDED_AID_UPDATE:
		case TC_EXTENDED_ADDSVC_CONFIG:	// [#2449] US Justin 2016.11.08
			nRes = BIZ_STD1_AnalExtendConfigMsg(Cnt);
			break;
		case TC_WITHDRAWAL:
		case TC_INQUIRY:
		case TC_TRANSFER:
		case TC_PINCHANGE:				// [#2150] US Justin 2012.10.05 Add Pin Change Transaction
		case TC_POPMONEY:				// [#2350] US Justin 2015.06.19 Add POP Money
		case TC_PAYPALCCA:				// [#2446] US Justin 2016.10.07 Paypal
		case TC_JUSTCASH:				// [#2448] US Justin 2016.11.21 Add Just.Cash
		case TC_JUSTCASH_BITCOIN:		// [#2496] US Justin 2017.08.15 Add Just.Cash BitCoin
		case TC_PIN4:					// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
		case TC_DIGITALMINT:
			nRes = BIZ_STD1_AnalTranMsg(Cnt);
			break;
		case TC_REVERSAL:
			nRes = BIZ_STD1_AnalReversalMsg(Cnt);
			break;
		case TC_TOTAL:
		case TC_TRIALTOTAL:
			nRes = BIZ_STD1_AnalTotalMsg(Cnt);
			break;
		case TC_HEALTHCHK:
			nRes = BIZ_STD1_AnalHealthCheckMsg(Cnt);
			break;
		case TC_DYNAMICFLOWL_1ST:
		case TC_DYNAMICFLOWL_2ND:
			nRes = BIZ_STD1_AnalDynamicFlowMsg(Cnt);
			break;
		default:
			break;
	}

	return nRes;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_AnalCommHeader()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD1_AnalCommHeader()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_AnalCommHeader]\n"));

	// [#318] [NH] KSK 2008.6.19
	if (m_sSTD1_CommReqHeader.RecordFormat_1 != m_sSTD1_CommRespHeader.RecordFormat_1)
	{
		// ERROR CHECK 1
		m_pDevCmn->fnAPL_StackError(_T("D009400"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003017), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD1_CommReqHeader.BankID_6 != m_sSTD1_CommRespHeader.BankID_6)
	{
		// ERROR CHECK 2
		m_pDevCmn->fnAPL_StackError(L"D009500", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003018), DEV_NET);		// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD1_CommReqHeader.TerminalID_8 != m_sSTD1_CommRespHeader.TerminalID_8)
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
FUNCTION NAME: BIZ_STD1_AnalConfigMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD1_AnalConfigMsg(int nIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_AnalConfigMsg]\n"));

	int Cnt = nIndex;

	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD1_ConfigResp.ResponseType_2 = m_strArrRecvData[Cnt];
	Cnt++;

	if (m_strArrRecvData.GetSize() > nIndex)
	{
		if(m_strArrRecvData[nIndex].GetLength() >= 8)
		{
			m_sSTD1_ConfigResp.LocalDate_8 = m_strArrRecvData[Cnt].Mid(0,8);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE, m_sSTD1_ConfigResp.LocalDate_8); // 2008-01-16 V01.02.25 SRC-19
		}

		if(m_strArrRecvData[Cnt].GetLength() >= 14)
		{
			m_sSTD1_ConfigResp.LocalTime_6 = m_strArrRecvData[Cnt].Mid(8,6);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME, m_sSTD1_ConfigResp.LocalTime_6); // 2008-01-16 V01.02.25 SRC-19
		}				
	}
	Cnt++;

	// DDHHMM, Healthy Check Message Maximum value to pass
	if (m_strArrRecvData.GetSize() > Cnt)
	{
		m_sSTD1_ConfigResp.HealthMsgTimerVal_6 = m_strArrRecvData[Cnt];
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		m_sSTD1_ConfigResp.WorkingKey_V = m_strArrRecvData[Cnt];

		// [#RWC6-150] US William 2021.03.29 TR-31
		if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TR31			|| 
			MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TR31_MACING	||
			MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TR31_TMACING)
		{
			// TR31 uses the Key1 slot, so we need to handle this as an exceptional case because the length of the
			// key is different
			MemSetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_TR31_WKB, m_sSTD1_ConfigResp.WorkingKey_V);
		}
		else
		{
			MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1, m_sSTD1_ConfigResp.WorkingKey_V);

			if(MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_TDES || 
				MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_TDES)
			{
				MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY3, m_sSTD1_ConfigResp.WorkingKey_V);
				m_sSTD1_ConfigResp.WorkingKey3_V = m_sSTD1_ConfigResp.WorkingKey_V;
			}
		}
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		m_sSTD1_ConfigResp.SurchargeAmount_V = m_strArrRecvData[Cnt];

		// [#242] NH KSK 2008.5.28 Surcharge Set Bug Fix
		// Surcharge는 길이가 variable이므로 길이check는 하지 않는다.
		if (!m_sSTD1_ConfigResp.SurchargeAmount_V.IsEmpty())
		{
#if (US_VERSION)
			//[#2092] US PCS 2011.10.25 "RBSLynk Surcharge Option WinCE5.0 US V01.04.09 개발내용"
			//1. STD1에서 
			//2. Percent Surcharge가 Enable로 설정되어 있고.
			//3. PercentSurcharge Ignore가 Enable로 설정되어 있으면, TC_OPEN에서 호스트로부터 받는 수수료 무시.
			if ((MemGetInt(_MEM_FLD_OPTIONALSETTING,_MEM_VAR_OPT_PERSURCHARGE_ENABLE)==ENABLE)&&
				(MemGetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_PERSURCHARGE_IGNORE))==ENABLE)
			{
				//Ignore Host Surcharge.
			}
			else
			{
				if( m_HostConfig == HC_ATM )		// [#2185] US Justin 2013.05.08 Dual Host DCC - Do not update Surcharge amount for Dual Host response
				{
					// [#2316] US Justin 2014.12.17 Not update Surcharge Amount if Percentage is used
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) != PERCENT_MODE)
					{
						if (Asc2Int(m_sSTD1_ConfigResp.SurchargeAmount_V) != 0)
						{
							MemSetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEENABLE, 1);
							MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHAREAMT, m_sSTD1_ConfigResp.SurchargeAmount_V);
						}
						else
						{
							MemSetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEENABLE, 0);
							MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHAREAMT, L"000");
						}
					}
					// End of [#2316]
				}
			}
#else
			// [#2316] US Justin 2014.12.17 Not update Surcharge Amount if Percentage is used
			#if (CA_VERSION)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) != PERCENT_MODE)
			#endif
			{

				// KSK 2008.11.18 Surcharge Bug Fix
				if (Asc2Int(m_sSTD1_ConfigResp.SurchargeAmount_V) != 0)
				{
					MemSetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEENABLE, 1);
					MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHAREAMT, m_sSTD1_ConfigResp.SurchargeAmount_V);
				}
				else
				{
					MemSetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEENABLE, 0);
					MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHAREAMT, L"000");
				}
			}
#endif
		//end of [#2092] US PCS 2011.10.25
		}
		// end of [#242]
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		m_sSTD1_ConfigResp.ConfigInitFlag_1 = m_strArrRecvData[Cnt];
		BIZ_STD1_SetConfigRequestInitiator(m_sSTD1_ConfigResp.ConfigInitFlag_1);	// [#560] NH KSK 2009.8.20 공통 적용
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		switch (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE))
		{
			// keymode: 1, 3, 4, 7
			case KEYMODE_NON_UNIQ_DDES:
			case KEYMODE_NON_UNIQ_TDES:
			case KEYMODE_UNIQ_TDES:
			case KEYMODE_TDES_MACING:
			case KEYMODE_TDES_TMACING:		// [#565] NH KSK 2009.8.20
				{
					m_sSTD1_ConfigResp.WorkingKey2_V = m_strArrRecvData[Cnt];
					MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2, m_sSTD1_ConfigResp.WorkingKey2_V);
				}
				break;

			// keymode: 5, 6
			case KEYMODE_NON_UNIQ_SDES_MACING:
			case KEYMODE_UNIQ_SDES_MACING:
				{
					m_sSTD1_ConfigResp.WorkingKey4_V = m_strArrRecvData[Cnt];
					MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY4, m_sSTD1_ConfigResp.WorkingKey4_V);
				}
				break;

			case KEYMODE_TR31_MACING:
			case KEYMODE_TR31_TMACING:
				{
					m_sSTD1_ConfigResp.WorkingKey2_V = m_strArrRecvData[Cnt];
					MemSetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_TR31_MKB, m_sSTD1_ConfigResp.WorkingKey2_V);
				}
				break;

			default:
				break;
		}
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		switch (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE))
		{
			// keymode: 3, 4, 7
			case KEYMODE_NON_UNIQ_TDES:
			case KEYMODE_UNIQ_TDES:
			case KEYMODE_TDES_MACING:
			case KEYMODE_TDES_TMACING:			// [#565] NH KSK 2009.8.20
				{
					m_sSTD1_ConfigResp.WorkingKey3_V = m_strArrRecvData[Cnt].Left(16);			// KSK 2008.12.18 metavante 인증 불구합 대책 (맨 끝에 ETX가 오는 것 제거 보완처리)
					MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY3, m_sSTD1_ConfigResp.WorkingKey3_V);
				}
				break;

			default:
				break;
		}
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		switch (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE))
		{
			// keymode: 7
			case KEYMODE_TDES_MACING:
			case KEYMODE_TDES_TMACING:		// [#565] NH KSK 2009.8.20
				{
					m_sSTD1_ConfigResp.WorkingKey4_V = m_strArrRecvData[Cnt];
					MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY4, m_sSTD1_ConfigResp.WorkingKey4_V);
				}
				break;

			default:
				break;
		}
	}
	// end of [#554]
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		switch (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE))
		{
			// keymode: 11
			case KEYMODE_TDES_TMACING:		// [#565] NH KSK 2009.8.20
				{
					m_sSTD1_ConfigResp.WorkingKey5_V = m_strArrRecvData[Cnt];
					MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_APP_DESKEY5, m_sSTD1_ConfigResp.WorkingKey5_V);
				}
				break;

			default:
				break;
		}
	}

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	// Error Check
	if ((BIZ_STD1_AnalCommHeader()) != RES_OK)
		return RES_HOST_RECV_ERR;
		

	if (m_sSTD1_ConfigReq.RequestType_2 != m_sSTD1_ConfigResp.ResponseType_2)
	{
		m_pDevCmn->fnAPL_StackError(L"D009A00", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003020), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;
	}

	int nKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);

	if (nKeyMode == KEYMODE_TR31 || nKeyMode == KEYMODE_TR31_MACING || nKeyMode == KEYMODE_TR31_TMACING)
	{
		if (m_sSTD1_ConfigResp.WorkingKey_V.IsEmpty())
		{
			m_pDevCmn->fnAPL_StackError(L"D009900", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003021), DEV_NET);		// [#419] [NH] KSK 2008.9.16
			return RES_HOST_RECV_ERR;
		}
	}
	else if (m_sSTD1_ConfigResp.WorkingKey_V.GetLength() != 16)		// Working Key 1
	{
		m_pDevCmn->fnAPL_StackError(L"D009900", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003021), DEV_NET);		// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;
	}

	// Key Mode 1, 3, 4, 7, 11
	if (nKeyMode == KEYMODE_NON_UNIQ_DDES || nKeyMode == KEYMODE_NON_UNIQ_TDES || nKeyMode == KEYMODE_UNIQ_TDES ||
		nKeyMode == KEYMODE_TDES_MACING || nKeyMode == KEYMODE_TDES_TMACING)
	{
		if (m_sSTD1_ConfigResp.WorkingKey2_V.GetLength() != 16)	// Working Key 2
		{
			m_pDevCmn->fnAPL_StackError(L"D009900", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003022), DEV_NET);	// [#419] [NH] KSK 2008.9.16	
			return RES_HOST_RECV_ERR;
		}
	}
	else if (nKeyMode == KEYMODE_TR31_MACING || nKeyMode == KEYMODE_TR31_TMACING)
	{
		if (m_sSTD1_ConfigResp.WorkingKey2_V.IsEmpty())
		{
			m_pDevCmn->fnAPL_StackError(L"D009900", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003022), DEV_NET);	// [#419] [NH] KSK 2008.9.16	
			return RES_HOST_RECV_ERR;
		}
	}

	if (m_sSTD1_ConfigResp.WorkingKey3_V.GetLength() != 16)	// Working Key 3
	{
		// Key Mode 3, 4, 7, 11
		if (nKeyMode == KEYMODE_NON_UNIQ_TDES || nKeyMode == KEYMODE_UNIQ_TDES || nKeyMode == KEYMODE_TDES_MACING || nKeyMode == 	KEYMODE_TDES_TMACING)
		{
			m_pDevCmn->fnAPL_StackError(L"D009900", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003023), DEV_NET);	// [#419] [NH] KSK 2008.9.16	
			return RES_HOST_RECV_ERR;
		}
	}

	if (m_sSTD1_ConfigResp.WorkingKey4_V.GetLength() != 16)	// Mac Working Key 1 (Working Key 4)
	{
		// Key Mode 5, 6, 7, 11
		if (nKeyMode == KEYMODE_NON_UNIQ_SDES_MACING || nKeyMode == KEYMODE_UNIQ_SDES_MACING || nKeyMode == KEYMODE_TDES_MACING || nKeyMode == KEYMODE_TDES_TMACING)
		{
			m_pDevCmn->fnAPL_StackError(L"D009900", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003024), DEV_NET);	// [#419] [NH] KSK 2008.9.16	
			return RES_HOST_RECV_ERR;
		}
	}

	if (m_sSTD1_ConfigResp.WorkingKey5_V.GetLength() != 16)	// Mac Working Key 2 (Working Key 5)
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
FUNCTION NAME: BIZ_STD1_AnalExtendConfigMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD1_AnalExtendConfigMsg(int nIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_AnalExtendConfigMsg]\n"));

	int Cnt = nIndex;

	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD1_ExtendConfigResp.ResponseType_2 = m_strArrRecvData[Cnt];
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD1_ExtendConfigResp.SubResponseType_2 = m_strArrRecvData[Cnt];
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		BIZ_STD1_AnalAssortedField(Cnt);
	}

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	// Error Check
	if ((BIZ_STD1_AnalCommHeader()) != RES_OK)
		return RES_HOST_RECV_ERR;

	if (m_sSTD1_ExtendConfigReq.RequestType_2 != m_sSTD1_ExtendConfigResp.ResponseType_2)
	{
		// ERROR CHECK 4
		m_pDevCmn->fnAPL_StackError(L"D009A00", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003020), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;
	}

	// 아래 Error Check를 할 경우에는 Error Code를 등록해야함. (협의 필요)
//	if (m_sSTD1_ExtendConfigReq.SubRequestType_2 != m_sSTD1_ExtendConfigResp.SubResponseType_2)	// AID EXCHANGE
//	{
//		// ERROR CHECK 4
//		m_pDevCmn->fnAPL_StackError(L"D00AA00", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003091), DEV_NET);	// ERROR CODE 등록해야함. (e-partner)
//		return RES_HOST_RECV_ERR;
//	}

	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_AnalTranMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD1_AnalTranMsg(int nIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_AnalTranMsg]\n"));

	int Cnt = nIndex;
	int nEMVResult = 0;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// Response Type
		m_sSTD1_TranResp.ResponseType_2 = m_strArrRecvData[Cnt];
	}
	Cnt++;

	if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TR31_MACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TR31_TMACING)	// [#565] NH KSK 2009.8.20
	{
		// MAC 사용 시,
		// keymode: 5, 6, 7, 11
		m_sSTD1_TranResp.RandomMac_8 = m_strArrRecvData[Cnt];		// 실제적으로 사용은 안함
		Cnt++;
	}

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// Transaction Sequence Number
		m_sSTD1_TranResp.TranSequenceNo_4 = m_strArrRecvData[Cnt];
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// Response Code
		m_sSTD1_TranResp.ResponseCode_2 = m_strArrRecvData[Cnt];
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		if(m_strArrRecvData[Cnt].GetLength() >= 8)
		{
			m_sSTD1_TranResp.LocalDate_8 = m_strArrRecvData[Cnt].Mid(0,8);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE, m_sSTD1_TranResp.LocalDate_8);
		}
		if(m_strArrRecvData[Cnt].GetLength() >= 14)
		{
			m_sSTD1_TranResp.LocalTime_6 = m_strArrRecvData[Cnt].Mid(8,6);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME, m_sSTD1_TranResp.LocalTime_6);
		}

		// Retrieval Reference Number
		if(m_strArrRecvData[Cnt].GetLength() >= 26)
		{
			m_sSTD1_TranResp.RetrievalRefNo_12 = m_strArrRecvData[Cnt].Mid(14,12);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRETRIEVALNUM, m_sSTD1_TranResp.RetrievalRefNo_12);
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
			m_sSTD1_TranResp.SystemTraceAuditNo_6 = m_strArrRecvData[Cnt].Mid(0,6);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM, m_sSTD1_TranResp.SystemTraceAuditNo_6);
		}

		if (m_strArrRecvData[Cnt].GetLength() >= 8)
		{
			// Network ID Code
			m_sSTD1_TranResp.NetworkIDCode_2 = m_strArrRecvData[Cnt].Mid(6,2);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID, m_sSTD1_TranResp.NetworkIDCode_2);
		}

		if (m_strArrRecvData[Cnt].GetLength() >= 16)
		{
			// Settlement Date
			m_sSTD1_TranResp.SettlementDate_8 = m_strArrRecvData[Cnt].Mid(8,8);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSSETTLEDATE, m_sSTD1_TranResp.SettlementDate_8);
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
					m_sSTD1_TranResp.AccountBalance_V = strtemp;
					MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT, m_sSTD1_TranResp.AccountBalance_V);
				}
			}
			else
			{
				if (IsNum(&m_szTemp[1], __max(strtemp.GetLength()-1, 1)) == TRUE)			// 마이너스 처리
				{
					m_sSTD1_TranResp.AccountBalance_V = strtemp;
					MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT, m_sSTD1_TranResp.AccountBalance_V);
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
					m_sSTD1_TranResp.AvailableBalance_V = strtemp;
					MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSAVAILBALANCE, m_sSTD1_TranResp.AvailableBalance_V);
				}
			}
			else
			{
				if(IsNum(&m_szTemp[1], __max(strtemp.GetLength()-1, 1)) == TRUE)		// 마이너스 값 처리
				{
					m_sSTD1_TranResp.AvailableBalance_V = strtemp;
					MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSAVAILBALANCE, m_sSTD1_TranResp.AvailableBalance_V);
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
		m_sSTD1_TranResp.SurchargeAmount_V = m_strArrRecvData[Cnt];
		m_sSTD1_TranResp.SurchargeAmount_V.TrimLeft();
		m_sSTD1_TranResp.SurchargeAmount_V.TrimRight();
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// Authorization Response Text
		m_sSTD1_TranResp.AuthResponseText_V = m_strArrRecvData[Cnt];
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// Configuration Request Initiator
		m_sSTD1_TranResp.ConfigRequestInitiator_2 = m_strArrRecvData[Cnt];
		// [#2176] NH KSK 2013.01.25 Host에서 수신 받은 값을 Setting 하도록 수정
//		if (m_sSTD1_TranResp.ConfigRequestInitiator_2 != "00")
//			BIZ_STD1_SetConfigRequestInitiator(m_sSTD1_TranResp.ConfigRequestInitiator_2);
		BIZ_STD1_SetConfigRequestInitiator(m_sSTD1_TranResp.ConfigRequestInitiator_2);
		// end of [#2176]
	}
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		nEMVResult = BIZ_STD1_AnalAssortedField(Cnt);
	}

	if(m_pDevCmn->fnMCU_IsEmvTransaction())		// [#397] [NH] psc 2008.08.01 for EMV transaction
	{
		// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
		// [#RWC6-59] US William 2019.10.08 LibertyX
		if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_JUSTCASH_BITCOIN || TranCode == TC_DIGITALMINT)
		{
			if(m_strRecvData.Find(L"ud") == -1 || m_strArrRecvData.GetSize() <= Cnt)		// ud field가 없거나 다음 data가 없는 경우 error 처리
			{
				// [#2372] US Justin 2015.09.11 TC for NON EMV Transaction
				/*
				if (m_bOnlineProcessing == FALSE)
				{
					// KSK 2010.08.29 ud 미 수신인 경우 Default 처리 추가
					m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

					if (m_pDevCmn->fnEMV_Trans_OnlineProcess(0, AAC) == EMV_RSLT_OK)
					{
						m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
						m_pDevCmn->fnEMV_Trans_Completion();
					}

					// KSK 2010.09.03 CID가 안바뀔 경우 reason for reversal code "08"로 설정
					if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
					{
						int nLen = 0;
						CString strValue;
						unsigned char	szTemp[1024] = {0,};

						if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_CryptInfData, &nLen, szTemp) == EMV_RSLT_OK)
						{
							strValue = MakeUnPack(szTemp, nLen);

							// CID가 ARQC (0x80)인 경우 Reason for Reversal을 "08"로 설정
							if (strValue == L"80")
							{
								if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
									MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 8);
							}
						}
					}
					// end of KSK 2010.09.03

					BIZ_EMV_MakeICDataforReversal();	// KSK 2009.8.31 송신 에러시에 

					m_bOnlineProcessing = TRUE;
				}
				nEMVResult = EMV_ERROR_ONLINE_DATA;
				*/
				if (m_bOnlineProcessing == FALSE)
				{
					CString strEMVTag4NonEMVDataCase = _T("8A023030910A00000000000000000000");			// MTIP 06 / 05 / 01 Test Case (8A:3030, 91:00000000000000000000)
					//CString strEMVTag4NonEMVDataCase = _T("8A023030"); // [#RWC6-22] EMV ADVT v7.0 test purpose only
					nEMVResult = BIZ_EMV_Anal_ICDataforTrans(strEMVTag4NonEMVDataCase);
				}
				// End of [#2372]
			}
		}
	}

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	// Error Check
	if ((BIZ_STD1_AnalCommHeader()) != RES_OK)
		return RES_HOST_RECV_ERR;

	if (m_sSTD1_TranReq.RequestType_2 != m_sSTD1_TranResp.ResponseType_2)
	{
		// ERROR CHECK 4
		m_pDevCmn->fnAPL_StackError(L"D009A00", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003020), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD1_TranReq.TransactionSequenceNo_4 != m_sSTD1_TranResp.TranSequenceNo_4)
	{
		// ERROR CHECK 5
		m_pDevCmn->fnAPL_StackError(_T("D009300"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003025), DEV_NET);		// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD1_TranResp.LocalDate_8.GetLength() != 8)
	{
		// ERROR CHECK 7
		m_pDevCmn->fnAPL_StackError(_T("D009E00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003026), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD1_TranResp.LocalTime_6.GetLength() != 6)
	{
		// ERROR CHECK 8
		m_pDevCmn->fnAPL_StackError(_T("D009E00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003027), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD1_TranResp.RetrievalRefNo_12.GetLength() != 12)
	{
		// ERROR CHECK 9
		m_pDevCmn->fnAPL_StackError(_T("D009E00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003028), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD1_TranResp.SystemTraceAuditNo_6.GetLength() != 6)
	{
		// ERROR CHECK 10
		m_pDevCmn->fnAPL_StackError(_T("D009F00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003029), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD1_TranResp.NetworkIDCode_2.GetLength() != 2)
	{
		// ERROR CHECK 11
		m_pDevCmn->fnAPL_StackError(_T("D009F00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003030), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD1_TranResp.SettlementDate_8.GetLength() != 8)
	{
		// ERROR CHECK 12
		m_pDevCmn->fnAPL_StackError(_T("D009F00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003031), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (m_sSTD1_TranResp.ResponseCode_2.GetLength() != 2)
	{
		m_pDevCmn->fnAPL_StackError(_T("D009D00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003020), DEV_NET);		// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if (!(m_sSTD1_TranResp.ResponseCode_2 == L"00" || m_sSTD1_TranResp.ResponseCode_2 == L"85"))	// "00" or "85"가 아닌 경우 모두 Denial 처리
	{
		CString strHostError, strHostErrorMsg;
		strHostError.Format(L"DA0%-2.2s00", m_sSTD1_TranResp.ResponseCode_2);
		
		if (m_sSTD1_TranResp.AuthResponseText_V.GetLength() > 0)
		{
			// Host에서 Error Message가 있는 경우 Message를 그대로 화면 Display한다.
			strHostErrorMsg = m_sSTD1_TranResp.AuthResponseText_V;
			m_pDevCmn->fnAPL_StackError(strHostError, strHostErrorMsg, DEV_NET);	// [#419] [NH] KSK 2008.9.16
			return RES_HOST_DENIED;
		}

		int nResCodeIndex = -1;	
		strHostErrorMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003078);		// [#257] [MX] KSK 2008.6.5	

		nResCodeIndex = BIZ_STD1_CheckResponseCode(m_sSTD1_TranResp.ResponseCode_2);

		if (nResCodeIndex != -1)
		{
			CString	strTemp;
			strTemp.Format(_T("%s%02d"), T_APMSG_003300, nResCodeIndex);
			strHostErrorMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(strTemp);
			// End of [#2351]
			// End of [#2288]
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
	// [#RWC6-59] US William 2019.10.08 LibertyX
	// William - Only set surcharge field if it is a primary host TXN, i.e. LibertyX has a different surcharge,
	// and it should not effect the standard surcharge of the terminal.
	if (m_HostConfig == HC_ATM)
	{
		MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT, m_sSTD1_TranResp.SurchargeAmount_V);	// [#169] [NH] 2008.04.25
	}

	// EMV ERROR시에 Cancel Proc을 태우도록 한다 (Error Set도 해야함)
	if (m_pDevCmn->fnMCU_IsEmvTransaction() && (nEMVResult != EMV_TR_SUCCESS))
	{
		if (nEMVResult == EMV_ERROR_DECLINED)
		{
			/* EMV ERROR CODE STACK */ /* 9730100 */
			// EMV On-line Script result is reversal 
			m_pDevCmn->fnAPL_StackError(L"9730100", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003087), DEV_MCU);	// KSK 2010.04.05 "IC CARD ERROR"로 문구 통일
		}
		else
		{
			/* EMV ERROR CODE STACK */ /* 9730200 */
			// EMV On-line Script result is reversal
			m_pDevCmn->fnAPL_StackError(L"9730200", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003087), DEV_MCU);	// KSK 2010.04.05 "IC CARD ERROR"로 문구 통일
		}

		/* Setting Reason for reversal : ICC DECLINED */
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 7);	// 캐나다 OpenSolution과 협의함 (2 -> 7 수정)

		return RES_EMV_ERROR_REVERSAL;
	}

	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);


	// [#2580] AU Kook 2019.06.27 Update ATM time when transaction is approved.
	// m_sSTD1_TranResp.LocalDate_8 : date recevied ("mmddyyyy")
	// m_sSTD1_TranResp.LocalTime_6 : time received ("hhmmss")
#if (AU_A_VERSION)
	CString szYear = m_sSTD1_TranResp.LocalDate_8.Mid(4, 4);
	CString szMonth = m_sSTD1_TranResp.LocalDate_8.Mid(0, 2);
	CString szDay = m_sSTD1_TranResp.LocalDate_8.Mid(2, 2);
	CString szHour = m_sSTD1_TranResp.LocalTime_6.Mid(0, 2);
	CString szMinute = m_sSTD1_TranResp.LocalTime_6.Mid(2, 2);
	CString szSecond = m_sSTD1_TranResp.LocalTime_6.Mid(4, 2);

	SYSTEMTIME	ti;
	GetLocalTime(&ti);
	
	// [GLDV-2858] AU Kook 2021.01.19 Exclude specific days from ATM Time Update feature.
	// 'cDT:202101190101' (current DateTime)
	CString strCurDateTime;
	strCurDateTime.Format(_T("cDT:%04d%02d%02d%02d%02d"), ti.wYear, ti.wMonth, ti.wDay, ti.wHour, ti.wMinute);
	NVDump('F', 'C', "00", L"HOST", strCurDateTime);

	// Don't update time when ATM local time is 12/31 or 1/1. Switch could give wrong date value at that time. (Year+1)
	if ( (ti.wMonth == 12 && ti.wDay == 31)		// 12/31
		|| (ti.wMonth == 1 && ti.wDay == 1) )	// 1/1
	{
		NVDump('F', 'C', "00", L"HOST", L"DT_not_upd");
	}
	else
	// end of [GLDV-2858]
	{
		ti.wYear   = Asc2Int(szYear);
		ti.wMonth  = Asc2Int(szMonth);
		ti.wDay    = Asc2Int(szDay);
		ti.wHour   = Asc2Int(szHour);
		ti.wMinute = Asc2Int(szMinute);
		ti.wSecond = Asc2Int(szSecond);
		SetLocalTime(&ti);

		// get/set localtime again to avoid daylight saving correction. (1-hour shifting)
		// change date from 1 May to 1 Jun.

		// needs some delay to apply time value immediately again.
		Delay_Msg(500);

		GetLocalTime(&ti);
		ti.wHour   = Asc2Int(szHour);
		ti.wMinute = Asc2Int(szMinute);
		ti.wSecond = Asc2Int(szSecond);
		SetLocalTime(&ti);

		// 'uDT:202101190101' (updated DateTime)
		CString szDateTime;
		szDateTime.Format(_T("uDT:%s%s%s%s%s"), szYear, szMonth, szDay, szHour, szMinute);
		NVDump('F', 'C', "00", L"HOST", szDateTime);
	}
#endif
	// end of [#2580]

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_AnalReversalMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD1_AnalReversalMsg(int nIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_AnalReversalMsg] nIndex(%d)\n", nIndex));

	int Cnt = nIndex;

	// Response Type
	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD1_ReversalResp.ResponseType_2 = m_strArrRecvData[Cnt];
	Cnt++;

	if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)	// [#565] NH KSK 2009.8.20
	{
		// MAC 사용 시,
		// keymdoe: 5, 6, 7, 11
		m_sSTD1_ReversalResp.RandomMac_8 = m_strArrRecvData[Cnt];	// 실제적으로 사용은 안함
		Cnt++;
	}
	// end of [#4]

	// TOTAL Config. Request Initiator
	if (m_strArrRecvData.GetSize() > Cnt)
	{
		NHDEBUG(DBG_INFO, (L"Data[%s], Count[%d]\n", m_strArrRecvData[Cnt], Cnt));
		m_sSTD1_ReversalResp.ConfigRequestInitiator_2 = m_strArrRecvData[Cnt];
		// [#2176] NH KSK 2013.01.25
//		if (m_sSTD1_ReversalResp.ConfigRequestInitiator_2 != "00")
//			BIZ_STD1_SetConfigRequestInitiator(m_sSTD1_ReversalResp.ConfigRequestInitiator_2);
		BIZ_STD1_SetConfigRequestInitiator(m_sSTD1_ReversalResp.ConfigRequestInitiator_2);
		// end of [#2176]
	}


#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	// Error Check
	if ((BIZ_STD1_AnalCommHeader()) != RES_OK)
	{
		NHDEBUG(DBG_INFO, (L"[RES_HOST_RECV_ERR]\n"));
		return RES_HOST_RECV_ERR;
	}

	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	NHDEBUG(DBG_INFO, (L"[RES_OK]\n"));

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_AnalTotalMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD1_AnalTotalMsg(int nIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_AnalTotalMsg]\n"));

	int Cnt = nIndex;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		// 2.Response Type
		m_sSTD1_TotalResp.ResponseType_2 = m_strArrRecvData[Cnt];
	}
	Cnt++;

	// 4.NUMBER OF TRANSACTIONS
	if (m_strArrRecvData.GetSize() > Cnt)
	{
		if(m_strArrRecvData[Cnt].GetLength() >= 16)
		{
			// 4-1.NUMBER OF CASH WITHDRAWAL
			m_sSTD1_TotalResp.NoOfCashWiths_4 = m_strArrRecvData[Cnt].Left(4);
			// 4-2.NUMBER OF TRANSFER
			m_sSTD1_TotalResp.NoOfTransfers_4 = m_strArrRecvData[Cnt].Mid(4,4);
			// 4-3.NUMBER OF INQUIRY
			m_sSTD1_TotalResp.NoOfInquiries_4 = m_strArrRecvData[Cnt].Mid(8,4);
			// 4-4.NUMBER OF NON-CASH WITHDRAWAL
			m_sSTD1_TotalResp.NoOfNonCashWiths_4 = m_strArrRecvData[Cnt].Mid(12,4);
		}
	}
	Cnt++;

	// 5.TOTAL CASH DISPENSED AMOUNT
	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD1_TotalResp.TotalDispenseAmt_V = m_strArrRecvData[Cnt];
	Cnt++;

	// 6.TOTAL NON-CASH DISPENSED AMOUNT
	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD1_TotalResp.TotalNonCashDispAmt_V = m_strArrRecvData[Cnt];
	Cnt++;

	// 7.TOTAL SURCHARGE AMOUNT
	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD1_TotalResp.TotalSurchargeAmt_V = m_strArrRecvData[Cnt];
	Cnt++;

	// 8.TOTAL Config. Request Initiator
	if (m_strArrRecvData.GetSize() > Cnt)
	{
		m_sSTD1_TotalResp.ConfigRequestInitiator_2 = m_strArrRecvData[Cnt];
		// [#2176] NH KSK 2013.01.25
//		if (m_sSTD1_TotalResp.ConfigRequestInitiator_2 != "00")
//			BIZ_STD1_SetConfigRequestInitiator(m_sSTD1_TotalResp.ConfigRequestInitiator_2);
		BIZ_STD1_SetConfigRequestInitiator(m_sSTD1_TotalResp.ConfigRequestInitiator_2);
		// end of [#2176]
	}

	// [#410] [US] KSK 2008.8.29
	if (m_strArrRecvData.GetSize() > Cnt)
	{
		BIZ_STD1_AnalAssortedField(Cnt);
	}

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	// Error Check
	if ((BIZ_STD1_AnalCommHeader()) != RES_OK)
		return RES_HOST_RECV_ERR;

	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_AnalHealthCheckMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD1_AnalHealthCheckMsg(int nIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_AnalHealthCheckMsg]\n"));

	int Cnt = nIndex;

	if (m_strArrRecvData.GetSize() > Cnt)
		m_sSTD1_HealthResp.ResponseType_2 = m_strArrRecvData[Cnt];
	Cnt++;

	if (m_strArrRecvData.GetSize() > Cnt)
	{
		m_sSTD1_HealthResp.ConfigRequestInitiator_2 = m_strArrRecvData[Cnt];
		// [#2176] NH KSK 2013.01.25
//		if (m_sSTD1_HealthResp.ConfigRequestInitiator_2 != "00")
//			BIZ_STD1_SetConfigRequestInitiator(m_sSTD1_HealthResp.ConfigRequestInitiator_2);
		BIZ_STD1_SetConfigRequestInitiator(m_sSTD1_HealthResp.ConfigRequestInitiator_2);
		// end of [#2176]
	}

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	// Error Check
	if ((BIZ_STD1_AnalCommHeader()) != RES_OK)
		return RES_HOST_RECV_ERR;

	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	return RES_OK;
}

// [#2150] US Justin 2012.09.27	Add Dynamic Flow
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD1_AnalDynamicFlowMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex, int nSubType
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD1_AnalDynamicFlowMsg(int nIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_AnalDynamicFlowMsg]\n"));

	int Cnt = nIndex;
	CString sTemp;

#ifdef APP_LOCAL_MODE
	m_pDevCmn->TranResult = TRUE;
	return RES_OK;
#endif

	// Check Header
//	if ((BIZ_STD1_AnalCommHeader()) != RES_OK)
//		return RES_HOST_RECV_ERR;

	// Check Response Type
	if (m_strArrRecvData.GetSize() > Cnt)
	{
		sTemp = m_strArrRecvData[Cnt++];
		if( sTemp.Compare(L"DF") != 0 )
			return RES_HOST_RECV_ERR;
	}
	else	
		return	RES_HOST_RECV_ERR;

	// [#2490] NH Justin 2017.06.15 Standard1 Dynamic Flow MAC Option
	// Random MAC Check.....SKIP.......
	/*
	#if (AU_VERSION)	// [###] AU KSK 2015.12.21 DCC시에 사양서와 동일하게 MAC 사용하도록 함
		if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
			MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
			MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
			MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)
		{
			// MAC 사용 시,
			// keymode: 5, 6, 7, 11
			m_sSTD1_DynamicFlowResp.RandomMac_8 = m_strArrRecvData[Cnt++];		// 실제적으로 사용은 안함
		}
	#endif		// end of [###]
	*/
	int nEPPKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);
	if( nEPPKeyMode == KEYMODE_NON_UNIQ_SDES_MACING	|| nEPPKeyMode == KEYMODE_UNIQ_SDES_MACING	||
		nEPPKeyMode == KEYMODE_TDES_MACING			|| nEPPKeyMode == KEYMODE_TDES_TMACING		)
	{
		if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_STD1_DYNAMICFLOW_MAC) == ENABLE ) 
			m_sSTD1_DynamicFlowResp.RandomMac_8 = m_strArrRecvData[Cnt++];		// 실제적으로 사용은 안함
	}
	// End of [#2490]

	// Check Response Code
	if (m_strArrRecvData.GetSize() > Cnt)
	{
		sTemp = m_strArrRecvData[Cnt++];
		if( sTemp.Compare(L"00") != 0 )
			return RES_HOST_RECV_ERR;
	}
	else
		return	RES_HOST_RECV_ERR;
	
	// Check Associate field
	CString sFirst1, sFirst2;
	if (m_strArrRecvData.GetSize() > Cnt)
	{
		do
		{
			sTemp = m_strArrRecvData[Cnt++];

			if(sTemp.GetLength()>2)
			{
				sFirst1 = sTemp.Left(1);
				sFirst2 = sTemp.Left(2);

				// i : Patner ID
				if( (sFirst1==L"i") && (sTemp.GetLength() == 9) )
				{
					m_sSTD1_DynamicFlowResp.PartnerID = sTemp.Right(8);
					MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_PARTNERID, m_sSTD1_DynamicFlowResp.PartnerID);
					NHDEBUG(DBG_CALL, (L"Partner ID = [%s]\n",m_sSTD1_DynamicFlowResp.PartnerID));
				}

				// ac : Customer or Terminal Capacity, 2bytes, 1st Msg only
				else if( (sFirst2==L"ac") && (sTemp.GetLength()==4) && (TranCode==TC_DYNAMICFLOWL_1ST) )
				{
					m_sSTD1_DynamicFlowResp.R1_TransactionCapacity = sTemp.Mid(2);
					BYTE nValue = Asc2Int(m_sSTD1_DynamicFlowResp.R1_TransactionCapacity);
					//Asc2Byte( (void*)(LPCTSTR)m_sSTD1_DynamicFlowResp.R1_TransactionCapacity, &nValue);
					NHDEBUG(DBG_CALL, (L"Received AC:[%s], Asc:[%02X]\n",m_sSTD1_DynamicFlowResp.R1_TransactionCapacity, nValue) );

					if( nValue & 0x01 )	m_sSTD1_DynamicFlowResp.nService_MPT = 1;
					if( nValue & 0x02 )	m_sSTD1_DynamicFlowResp.nService_PINChange = 1;
					if( nValue & 0x04 )	m_sSTD1_DynamicFlowResp.nService_DCC = 1;
					NHDEBUG(DBG_CALL, (L"MPT=[%d], PIN Change=[%d], DCC=[%d]\n", m_sSTD1_DynamicFlowResp.nService_MPT,
						m_sSTD1_DynamicFlowResp.nService_PINChange, m_sSTD1_DynamicFlowResp.nService_DCC));
				}
				// fi : Surcharge for Balance Inquiry, 8 Bytes, EX. 00000100, 1st Msg Only
				else if( (sFirst2==L"fi") && (sTemp.GetLength()==10) && (TranCode==TC_DYNAMICFLOWL_1ST) )
				{
					m_sSTD1_DynamicFlowResp.R1_Surcharge_BalanceInquiry = sTemp.Mid(2);
					NHDEBUG(DBG_CALL, (L"1st, Surcharge_BalanceInquiry = [%s]\n", m_sSTD1_DynamicFlowResp.R1_Surcharge_BalanceInquiry));
				}
				// fp : Surcharge for Pin Change, 8 Bytes, EX. 00000100, 1st Msg Only
				else if( (sFirst2==L"fp") && (sTemp.GetLength()==10) && (TranCode==TC_DYNAMICFLOWL_1ST) )
				{
					m_sSTD1_DynamicFlowResp.R1_Surcharge_PinChange = sTemp.Mid(2);
					NHDEBUG(DBG_CALL, (L"1st, Surcharge_PinChange = [%s]\n", m_sSTD1_DynamicFlowResp.R1_Surcharge_PinChange));
				}

				// [#2182] NH Justin 2013.03.21 Implementing NHD1.6
				// ec : Base and Target Currency Code, 11 bytes Ex. USD/2/KRW/0, 1st or 2nd Msg
				else if( (sFirst2==L"ec") && (sTemp.GetLength()==13) )
				{
					if	   (TranCode==TC_DYNAMICFLOWL_1ST)		m_sSTD1_DynamicFlowResp.R1_DCC_BaseTargetCurrency = sTemp.Mid(2);
					else if(TranCode==TC_DYNAMICFLOWL_2ND)		m_sSTD1_DynamicFlowResp.R2_DCC_BaseTargetCurrency = sTemp.Mid(2);
					NHDEBUG(DBG_CALL, (L"DCC_BaseTargetCurrency(%d) = [%s]\n",TranCode, sTemp.Mid(2) ));
				}
				// er : Exchange Rate, Variable Ex. 12.45567, 1st or 2nd Msg
				else if( (sFirst2==L"er") && (sTemp.GetLength()>2) )
				{
					if	   (TranCode==TC_DYNAMICFLOWL_1ST)		m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRate = sTemp.Mid(2);
					else if(TranCode==TC_DYNAMICFLOWL_2ND)		m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRate = sTemp.Mid(2);
					NHDEBUG(DBG_CALL, (L"DCC_ExchangeRate(%d) = [%s]\n",TranCode, sTemp.Mid(2) ));
				}
				// fd : Surcharge for DCC, 8 Bytes, EX. 00000100, 1st or 2nd Msg
				else if( (sFirst2==L"fd") && (sTemp.GetLength()==10) )
				{
					if     (TranCode==TC_DYNAMICFLOWL_1ST) 		m_sSTD1_DynamicFlowResp.R1_Surcharge_DCC = sTemp.Mid(2);
					else if(TranCode==TC_DYNAMICFLOWL_2ND)		m_sSTD1_DynamicFlowResp.R2_Surcharge_DCC = sTemp.Mid(2);
					NHDEBUG(DBG_CALL, (L"Surcharge_DCC(%d) = [%s]\n",TranCode, sTemp.Mid(2) ));
				}
				// End of  [#2182]

				// et : Converted Transaction Amount, 12 bytes Ex. 000000001234, 2nd Msg Only
				else if( (sFirst2==L"et") && (sTemp.GetLength()==14) && (TranCode==TC_DYNAMICFLOWL_2ND) )
				{
					m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount = sTemp.Mid(2);
					NHDEBUG(DBG_CALL, (L"2nd, DCC_ConvertedTrAmount = [%s]\n", m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedTrAmount));
				}
				// ef : Converted Surcharge Amount, 12 bytes Ex. 000000000234, 2nd Msg Only
				else if( (sFirst2==L"ef") && (sTemp.GetLength()==14) && (TranCode==TC_DYNAMICFLOWL_2ND) )
				{
					m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge = sTemp.Mid(2);
					NHDEBUG(DBG_CALL, (L"2nd, DCC_ConvertedSurcharge = [%s]\n", m_sSTD1_DynamicFlowResp.R2_DCC_ConvertedSurcharge));
				}

				// fc : Surcharge for Cash Withdrawal, 8 Bytes, EX. 00000100 or 8 spaces, 1st and 2nd Msg
				else if( (sFirst2==L"fc") && (sTemp.GetLength()==10) )
				{
					NHDEBUG(DBG_CALL, (L"Surcharge_CashWIthdrawal = [%s]\n", sTemp.Mid(2)));
					if(TranCode==TC_DYNAMICFLOWL_1ST)	m_sSTD1_DynamicFlowResp.R1_Surcharge_Withdrawal = sTemp.Mid(2);
					else								m_sSTD1_DynamicFlowResp.R2_Surcharge_Withdrawal = sTemp.Mid(2);
				}
				// ft : Surcharge for MPT, 8 Bytes, EX. 00000100 or 8 spaces, 1st and 2nd Msg
				else if( (sFirst2==L"ft") && (sTemp.GetLength()==10) )
				{
					NHDEBUG(DBG_CALL, (L"Surcharge_MPT = [%s]\n", sTemp.Mid(2)));
					if(TranCode==TC_DYNAMICFLOWL_1ST)	m_sSTD1_DynamicFlowResp.R1_Surcharge_MPT = sTemp.Mid(2);
					else								m_sSTD1_DynamicFlowResp.R2_Surcharge_MPT = sTemp.Mid(2);
				}
				// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1 message 
				// em : Exchange Rate Mark-Up, Variable Ex. 3.33, 1st or 2nd Msg
				else if( (sFirst2==L"em") && (sTemp.GetLength()>2) )
				{
					if	   (TranCode==TC_DYNAMICFLOWL_1ST)
						m_sSTD1_DynamicFlowResp.R1_DCC_ExchangeRateMarkUp = sTemp.Mid(2);
					else if(TranCode==TC_DYNAMICFLOWL_2ND)
						m_sSTD1_DynamicFlowResp.R2_DCC_ExchangeRateMarkUp = sTemp.Mid(2);
					NHDEBUG(DBG_CALL, (L"DCC_ExchangeRateMarkUp(%d) = [%s]\n",TranCode, sTemp.Mid(2) ));
				}
				// end of [#RWC6-2, #2585]
			}
		} while (m_strArrRecvData.GetSize() > Cnt);
	}
	else
		return	RES_HOST_RECV_ERR;

	m_pDevCmn->TranResult = TRUE;						// Host Ok
	return RES_OK;
}
// End of [#2150]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD1_AnalAssortedField()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD1_AnalAssortedField(int nCurrentIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_AnalAssortedField]\n"));

	int nSearchIdx = 0;
	int nAssortedIndex = 0, i = 0;
	UINT nAssorted_FID2 = 0, nTmp = 0;
	BYTE	chAssortedField = 0;
	CString strAssortedField;
	CString	strTemp;
	CString strTmp, strTmp2;				// [#2182] NH Justin 2013.03.22 

	// [#2185] US Justin 2013.05.07 Implement Assorted Fields for "Download Working Key"
	bool bReceivedFirstWorkingKey = false;
	bool bReceivedSecondWorkingKey = false;
	bool bReceivedThirdWorkingKey = false;
	bool bReceivedMACFirstWorkingKey = false;
	bool bReceivedMACSecondWorkingKey = false;
	// End of [#2185]

	bool receivedDate = false;
	bool receivedTime = false;
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);

	UINT	nFID2 = 0;
	int		nEMVResult = EMV_TR_SUCCESS;	// [#492] [NH] KSK 2009.2.5

	switch (TranCode)
	{
		case TC_INQUIRY:
		case TC_TRANSFER:
		case TC_WITHDRAWAL:				// Transaction Message
		case TC_POPMONEY:				// [#2350] US Justin 2015.06.19 Add POP Money
		case TC_PAYPALCCA:				// [#2446] US Justin 2016.10.07 Paypal
		case TC_JUSTCASH:				// [#2448] US Justin 2016.11.21 Add Just.Cash
		case TC_JUSTCASH_BITCOIN:		// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
		case TC_PIN4:					// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
		case TC_DIGITALMINT:
			nAssortedIndex = ASSORTED_RESPONSE_TRAN;
			break;
		case TC_TOTAL:
		case TC_TRIALTOTAL:
			nAssortedIndex = ASSORTED_RESPONSE_TOTAL;
			break;
		case TC_EXTENDED_EJUPLOAD:		// [#2076] NH KSK 2011.06.28
		case TC_EXTENDED_AID_UPDATE:	// [#560] NH KSK 2009.8.20
		case TC_EXTENDED_ADDSVC_CONFIG:	// [#2449] US Justin 2016.11.08
			nAssortedIndex = ASSORTED_RESPONSE_EXTCONFIG;
			break;
		default:
			return T_ERROR;
	}

#if SUPPORT_CHANGE_PARAMETER_JNL		//[#610] SOOK 2010.01.14 Configuration 저널 저장 
	m_pDevCmn->SetPreviousParameter();	//STD1에는 HOST에서 내려온 것 들을 저장하는 부분이 없음 //나중에 생기면 추가 
#endif		//end of [#610]

	for (nSearchIdx = nCurrentIndex; nSearchIdx < m_strArrRecvData.GetSize(); nSearchIdx++,i = 0 )
	{
		strAssortedField = m_strArrRecvData[nSearchIdx];
		
		if( strAssortedField == "")
			continue;

		chAssortedField = (BYTE)strAssortedField.GetAt(0);

		while(chAssorted_Response[nAssortedIndex][i] != 0)
		{
			// search current table
			if( chAssortedField == (BYTE)chAssorted_Response[nAssortedIndex][i])
				break;

			i++;
			
			// search next table
			if( chAssorted_Response[nAssortedIndex][i] == 0)
				chAssortedField = 0x00;
		}

		switch( chAssortedField ) 
		{
			case 'y':
				break;

			// [#397] [NH] psc 2008.08.01 EMV 거래 관련 'u'필드 분석
			case 'u': // Two character FIDs
				// Make the second FID
				if(strAssortedField.GetLength() < 2)				
					break;
				
				chAssortedField = (BYTE)strAssortedField.GetAt(1);
				nAssorted_FID2 = chAssorted_Response[nAssortedIndex][i] & 0xFFFFFF00;	// [#103] NH JSW 2008.04.12
				if( (BIZ_STD1_GetFID(chAssortedField) &  nAssorted_FID2) != BIZ_STD1_GetFID(chAssortedField))
					break;

				// [#RWC6-125] US William 2020.02.20 Add support for 'ua' and 't' FIDs
				if (chAssortedField == 'a') // Processor date change
				{
					// Date String "MMDDYY"
					strTemp.Format(L"%s", strAssortedField.Mid(2, 6));

					if (strTemp.GetLength() != 6)
					{
						NHDBG((L"Date format not correct length\r\n"));
						break;
					}

					int month = Asc2Int(strTemp.Mid(0, 2));
					int date = Asc2Int(strTemp.Mid(2, 2));
					int year = Asc2Int(strTemp.Mid(4, 2)) + 2000;
					
					bool invalid = 
						(month <= 0 || month > 12) ||
						(date <= 0 || date > 31) ||
						(year < 2020 || year > 2099);
					if (invalid)
					{
						NHDBG((L"Date string is invalid: %s\r\n", strTemp));
						break;
					}

					// Set date
					systemTime.wDay = date;
					systemTime.wMonth = month;
					systemTime.wYear = year;
					
					receivedDate = true;
					NHDBG((L"Received new system date\r\n"));

					break;
				}

				// [#559] CA KSK 2009.08.19
					// 'd' Encoded EMV data block ASCII characters. Variable length.
				if( m_pDevCmn->fnMCU_IsEmvTransaction() && chAssortedField == 'd' )
				{
					strTemp.Format(L"%s",strAssortedField.Mid(2));
							
					if( strTemp.GetLength() <= 0)
					{
						if (m_bOnlineProcessing == FALSE)
						{
							// "ud" Field만 온 경우 Error를 내도록 한다. KSK 2009.4.17
							// KSK 2010.08.29 Default 처리 추가
							m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

							if (m_pDevCmn->fnEMV_Trans_OnlineProcess(1, AAC) == EMV_RSLT_OK)
							{
								m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
								m_pDevCmn->fnEMV_Trans_Completion();
							}

							// KSK 2010.09.03 CID가 안바뀔 경우 reason for reversal code "08"로 설정
							if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
							{
								int nLen = 0;
								CString strValue;
								unsigned char	szTemp[1024] = {0,};

								if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_CryptInfData, &nLen, szTemp) == EMV_RSLT_OK)
								{
									strValue = MakeUnPack(szTemp, nLen);

									// CID가 ARQC (0x80)인 경우 Reason for Reversal을 "08"로 설정
									if (strValue == L"80")
									{
										if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value ?? ??? ?? ?? ??
											MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 8);
									}
								}
							}
							// end of KSK 2010.09.03

							BIZ_EMV_MakeICDataforReversal();

							m_bOnlineProcessing = TRUE;
						}
						nEMVResult = EMV_ERROR_ONLINE_DATA;
						break;
					}
					
					nEMVResult = BIZ_EMV_Anal_ICDataforTrans(strTemp);	// [#492] [NH] KSK 2009.2.5
					NHDEBUG(DBG_CALL, (_T("[CTranCmn::BIZ_STD1_AnalAssortedField] LIB_EMV_Anal_ICDataforTrans nRet(%d0\n"), nEMVResult));
				}
				// end of [#559]

				break;
			// end of [#397]

			// [#560] NH KSK 2009.8.20
			case 'e': // Extended Configuration fields. Two character FIDs
				if(strAssortedField.GetLength() < 2)
				{
					NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Standard1_ANAL_AssortedField() e Length Error \n")));	
					break;
				}

				chAssortedField = (BYTE)strAssortedField.GetAt(1);
				nAssorted_FID2 = chAssorted_Response[nAssortedIndex][i] & 0xFFFFFF00;	// [#103] NH JSW 2008.04.12
				if( (BIZ_STD1_GetFID(chAssortedField) &  nAssorted_FID2) != BIZ_STD1_GetFID(chAssortedField))
					break;

				// [#559] CA KSK 2009.08.19
				// 'ef' Continue or Stop flag of Extended Configuration Request.
				if (chAssortedField == 'f' )
				{
					strTemp.Format(L"%s",strAssortedField.Mid(2));

					if (strTemp.GetLength() == 1)	// 1BYTE인 경우에만 유효
					{
						if (strTemp == L"0")
							nTerminalRemainAIDCnt = 0;	// 더이상 Host로 Terminal AID List를 전송하지 않는다.
					}
					NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Standard1_ANAL_AssortedField() AID Flag ef Value(%s\n"), strTemp));
				}
				// 'es' AID List to be Saved in the terminal.
				else if (chAssortedField == 's')
				{
					int	nCurrentAIDCount = 0;
					BOOL bInvalidCheck = FALSE;
					CStringArray	strarrayAIDList;
					CStringArray	strarrayAID_TAC;

					strTemp.Format(L"%s",strAssortedField.Mid(2));

					NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Standard1_ANAL_AssortedField() AID LIST (%s\n"), strTemp));

					if (strTemp.GetLength() >= 7)
					{
						nCurrentAIDCount = Asc2Int(strTemp.Mid(3,2));
						strTemp.Format(L"%s",strAssortedField.Mid(9));

						NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Standard1_ANAL_AssortedField() AID LIST (%s\n"), strTemp));

						if (strTemp.GetLength() > 0)
						{
							SplitString(strTemp, AID_FIELD_DELIMITER, strarrayAIDList);

							NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Standard1_ANAL_AssortedField() CURRENT AID COUNT (%d) CALC AID COUNT(%d)\n"), nCurrentAIDCount, strarrayAIDList.GetSize()));

							if (nCurrentAIDCount == strarrayAIDList.GetSize())
							{
								for(int i=0; i<strarrayAIDList.GetSize(); i++)
								{
									NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Standard1_ANAL_AssortedField() AID LIST (%s\n"), strarrayAIDList[i]));

									if (strarrayAIDList[i].GetLength() > 0)
									{
										SplitString(strarrayAIDList[i], AID_FIELD_DELIMITER2, strarrayAID_TAC);

										if (strarrayAID_TAC.GetSize() == 2)
										{
											NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Standard1_ANAL_AssortedField() AID LIST (%s\n"), strarrayAID_TAC[0]));
											NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Standard1_ANAL_AssortedField() AID LIST (%s\n"), strarrayAID_TAC[1]));
											// Length Check
											if (strarrayAID_TAC[0].GetLength() > 0 && strarrayAID_TAC[1].GetLength() == 30)
											{
												WideToMulti(m_Standard1_Host_AIDList.chAID[nCalcRecvTotalAIDCnt], strarrayAID_TAC[0], sizeof(m_Standard1_Host_AIDList.chAID[nCalcRecvTotalAIDCnt]));
												WideToMulti(m_Standard1_Host_AIDList.chTAC_Denial[nCalcRecvTotalAIDCnt], strarrayAID_TAC[1].Left(10), sizeof(m_Standard1_Host_AIDList.chTAC_Denial[nCalcRecvTotalAIDCnt]));
												WideToMulti(m_Standard1_Host_AIDList.chTAC_Online[nCalcRecvTotalAIDCnt], strarrayAID_TAC[1].Mid(10, 10), sizeof(m_Standard1_Host_AIDList.chTAC_Online[nCalcRecvTotalAIDCnt]));
												WideToMulti(m_Standard1_Host_AIDList.chTAC_Default[nCalcRecvTotalAIDCnt], strarrayAID_TAC[1].Right(10), sizeof(m_Standard1_Host_AIDList.chTAC_Default[nCalcRecvTotalAIDCnt]));

												nCalcRecvTotalAIDCnt++;
												nTerminalRemainAIDCnt = 0;	// 더이상 Host로 Terminal AID List를 전송하지 않는다. (POS와 사양 통일) KSK 2009.11.24
											}
											else
											{
												bInvalidCheck = TRUE;
												break;
											}
										}
										else
										{
											bInvalidCheck = TRUE;
											break;
										}
									}
									else
									{
										bInvalidCheck = TRUE;
										break;
									}
								}
							}
							else
								bInvalidCheck = TRUE;
						}
						else
							bInvalidCheck = TRUE;
					}
					else
						bInvalidCheck = TRUE;

					if (bInvalidCheck == FALSE)
					{
						// AID NORMAL RECEIVED
						// AID TOTAL COUNT
						m_Standard1_Host_AIDList.nTotalAIDCount = Asc2Int(strAssortedField.Mid(2,3));
						nHostRemainBlock = Asc2Int(strAssortedField.Mid(7,2));
						NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Standard1_ANAL_AssortedField() SUCCESS!! AID LIST COUNT(%d) (%d)\n"), m_Standard1_Host_AIDList.nTotalAIDCount, nHostRemainBlock));
					}
					else
					{
						// 값이 잘못될 경우 AID Update를 하지 않기 위해서 Total Count 및 RemianBlock을 초기화 시킨다.
						m_Standard1_Host_AIDList.nTotalAIDCount = 0;
						nHostRemainBlock = 0;
						nTerminalRemainAIDCnt = 0;
						NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Standard1_ANAL_AssortedField() FAIL!! AID LIST COUNT(%d) (%d)\n"), m_Standard1_Host_AIDList.nTotalAIDCount,nHostRemainBlock));
					}
				}
				// 'ej' Journal Upload data field. 
				else if (chAssortedField == 'j')
				{
					// [#2076] NH KSK 2011.06.29
					strTemp.Format(L"%s",strAssortedField.Mid(2));

					if (strTemp.GetLength() > 0)
					{
						if (strTemp.GetAt(0) == '0')	// EJ Status Fail
						{
							if (m_nUploadLastJnlCount > 0)	// Upload한 Journal이 존재하나 송신 Fail이므로 이전 Index로 설정한다.
								m_pDevCmn->m_JNLMgr.SetUploadedIndex(UP_HOST, m_nUploadLastJnlCount);
						}
					}
					// end of [#2076]
				}
				break;

			// [#2076] NH KSK 2011.06.29
			case 'j':
				{
					//if (TranCode != TC_EXTENDED_AID_UPDATE)
					if( (TranCode != TC_EXTENDED_AID_UPDATE) && (TranCode != TC_EXTENDED_ADDSVC_CONFIG) )	// [#2449] US Justin
					{
						strAssortedField = strAssortedField.Right(1);

						if (strAssortedField.GetAt(0) == '0')
							MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EJUPLOAD_ENABLE, 0);
						else if (strAssortedField.GetAt(0) == '1')
							MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EJUPLOAD_ENABLE, ENABLE);
					}
				}
				break;
			// end of [#2076]

			// [#2182] NH Justin 2013.03.22 Add Print line
			case 'p':
				// Printing Data Set
				{
					int nCnt = 0;
					CString strCnt = "";
					if (strAssortedField.GetLength() < 2)	break;

					// 1. Get Line Count
					strCnt.Format(L"%s",strAssortedField.Mid(1,1));
					nCnt = Asc2Int(strCnt);

					if (nCnt < 1 || nCnt > 4)				break;	

					// 2. Get Message Data
					strTmp.Format(L"%s",strAssortedField.Mid(2));

					CString			strTmp2;
					CStringArray	strTmpArray;

					// 3. Separator
					strTmp2.Format(L"%x", 0x0d);
					SplitString(strTmp, (char)0x0d, strTmpArray);

					if (!strTmpArray.GetSize())				break;

					if (nCnt > strTmpArray.GetSize())
						nCnt = strTmpArray.GetSize();

					for (int i = 0; i < nCnt; i++)
					{
						m_strSTD1PrtMsgArray.Add(strTmpArray[i]);
					}
					break;
				}
				break;
			// end of [#2182]

			// [#2443] AU KSK 2016.08.31 support 'w' Field
			case 'w':
				#if (AU_VERSION || US_VERSION || CA_VERSION)	// [#2465] US/CA Justin 2017.01.12 Enable Standard1 'w'
				strTemp.Format(L"%s",strAssortedField.Mid(1));

				if (strTemp.GetLength() != 4)
				{
					NVDump('F', 'C', "00", L"HOST", L"w len error");
					break;
				}

				if (IsNum(strTemp) == FALSE)
				{
					NVDump('F', 'C', "00", L"HOST", L"w isnum error");
					break;
				}

				if ((Asc2Int(strTemp) < 1) || (Asc2Int(strTemp) > 9999))
				{
					NVDump('F', 'C', "00", L"HOST", L"w len range err");
					break;				
				}

				MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_DISPENSELIMIT, (strTemp + _T("00")));
				#endif
				break;
			// end of [#2443]

			// [#2360] NH KSK 2015.12.23 DCC Disclaimer 미사용으로 사양 변경되어 주석 처리
			/*
			case 'x':// Two character FIDs
				if(strAssortedField.GetLength() < 3)
				{
					NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Standard1_ANAL_AssortedField() x Length Error \n")));	
					break;
				}

				chAssortedField = (BYTE)strAssortedField.GetAt(1);
				nAssorted_FID2 = chAssorted_Response[nAssortedIndex][i] & 0xFFFFFF00;
				if( (BIZ_STD1_GetFID(chAssortedField) &  nAssorted_FID2) != BIZ_STD1_GetFID(chAssortedField))
					break;

				if (chAssortedField == 'g' )				// DCC Disclaimer
				{
					int nCnt = 0;
					CString strCnt = "";
					if (strAssortedField.GetLength() < 3)	break;

					// 1. Get Line Count
					strCnt.Format(L"%s",strAssortedField.Mid(2,1));
					nCnt = Asc2Int(strCnt);

					if (nCnt < 1 || nCnt > 9)				break;	

					// 2. Get Message Data
					strTmp.Format(L"%s",strAssortedField.Mid(3));

					CString			strTmp2;
					CStringArray	strTmpArray;

					// 3. Separator
					strTmp2.Format(L"%x", 0x0d);
					SplitString(strTmp, (char)0x0d, strTmpArray);

					if (!strTmpArray.GetSize())				break;

					if (nCnt > strTmpArray.GetSize())
						nCnt = strTmpArray.GetSize();

					for (int i = 0; i < nCnt; i++)
					{
						m_strSTD1DCCMsgArray.Add(strTmpArray[i]);
					}
					break;
				}
				break;
			*/
			// end of [#2360]

			// [#2185] US Justin 2013.05.07 Implement Assorted Fields for "Download Working Key"
			case '~':		// Working Key1
				strTmp = strAssortedField.Mid(1);
				if(strTmp.GetLength() != 16)
					break;
				MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1, (LPCTSTR)strTmp);
				bReceivedFirstWorkingKey = true;
				break;
			case '{':		// Working Key2
				strTmp = strAssortedField.Mid(1);
				if(strTmp.GetLength() != 16)
					break;
				MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2, (LPCTSTR)strTmp);
				bReceivedSecondWorkingKey = true;
				break;
			case '(':		// Working Key3
				strTmp = strAssortedField.Mid(1);
				if(strTmp.GetLength() != 16)
					break;
				MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY3, (LPCTSTR)strTmp);
				bReceivedThirdWorkingKey = true;
				break;
			case '}':		// MAC Working Key1
				{
					int nCurKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);
					if( (nCurKeyMode==KEYMODE_NON_UNIQ_SDES_MACING)||(nCurKeyMode==KEYMODE_UNIQ_SDES_MACING)||(nCurKeyMode==KEYMODE_TDES_MACING)||(nCurKeyMode==KEYMODE_TDES_TMACING) )
					{
						strTmp = strAssortedField.Mid(1);
						if(strTmp.GetLength() != 16)
							break;
						MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY4, (LPCTSTR)strTmp);
						bReceivedMACFirstWorkingKey = true;
					}
				}
				break;
			case 'm':		// MAC Working Key2
				if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)
				{
					strTmp = strAssortedField.Mid(1);
					if(strTmp.GetLength() != 16)
						break;
					MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_APP_DESKEY5, (LPCTSTR)strTmp);
					bReceivedMACSecondWorkingKey = true;
				}
				break;
			// End of [#2185]

			// [#2362] AU KSK 2015.08.17 Withdrawal / Balance fee setting 추가
			case 'f':
#if (AU_VERSION)
				if(strAssortedField.GetLength() < 3)
				{
					NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Standard1_ANAL_AssortedField() f Length Error \n")));	
					break;
				}

				chAssortedField = (BYTE)strAssortedField.GetAt(1);
				nAssorted_FID2 = chAssorted_Response[nAssortedIndex][i] & 0xFFFFFF00;
				if( (BIZ_STD1_GetFID(chAssortedField) &  nAssorted_FID2) != BIZ_STD1_GetFID(chAssortedField))
					break;

				if (chAssortedField == 'c')	// Withdrawal Surcharge Amount
				{
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) == PERCENT_MODE)
						break;

					strTemp.Format(L"%s",strAssortedField.Mid(2));

					if (strTemp.GetLength() != 8)
						break;

					if (IsNum(strTemp) == FALSE)		// KSK 2010.04.09 Numeric이 아닌 경우 모두 Invalid 처리
						break;

					if (Asc2Int(strTemp) != 0)
					{
						MemSetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEENABLE, 1);
						MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHAREAMT, strTemp);
					}
					else
					{
						MemSetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEENABLE, 0);
						MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHAREAMT, L"000");
					}
				}
				else if (chAssortedField == 'i')	// Balance Surcharge Amount
				{
					strTemp.Format(L"%s",strAssortedField.Mid(2));

					if (strTemp.GetLength() != 8)
						break;

					if (IsNum(strTemp) == FALSE)		// KSK 2010.04.09 Numeric이 아닌 경우 모두 Invalid 처리
						break;

					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_INQUIRY_SURCHARGE, Asc2Int(strTemp));
				}
#endif
				break;
			// end of [#2362]

			// [#2449] US Justin 2016.11.08 Mobile Cash Access
			case 'a':// Two character FIDs
				if (TranCode == TC_EXTENDED_ADDSVC_CONFIG)
				{
					if(strAssortedField.GetLength() < 3)
					{
						NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Standard1_ANAL_AssortedField() x Length Error \n")));	
						break;
					}

					chAssortedField = (BYTE)strAssortedField.GetAt(1);
					nAssorted_FID2 = chAssorted_Response[nAssortedIndex][i] & 0xFFFFFF00;
					if( (BIZ_STD1_GetFID(chAssortedField) &  nAssorted_FID2) != BIZ_STD1_GetFID(chAssortedField))
						break;

					strTemp.Format(L"%s",strAssortedField.Mid(2));

					#if (APP_PAYDIANT_CCA)
					if (chAssortedField == 'e' )				// Enable/Disable Service
					{
						if( strTemp == L"0")
							MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE, 0);
					}
					else if (chAssortedField == 'i' )			// ATM ID
						m_PayPalCCA.AssignPaydiantCredential(UPDATE_CREDENTIAL_ATMID, strTemp);
					else if (chAssortedField == 'k' )			// ATM Key
						m_PayPalCCA.AssignPaydiantCredential(UPDATE_CREDENTIAL_KEY, strTemp);
					else if (chAssortedField == 'l' )			// Location ID
						m_PayPalCCA.AssignPaydiantCredential(UPDATE_CREDENTIAL_LOCATIONID, strTemp);
					else if (chAssortedField == 'p' )			// Provider ID
						m_PayPalCCA.AssignPaydiantCredential(UPDATE_CREDENTIAL_PROVIDERID, strTemp);
					else if (chAssortedField == 'u' )			// Web Server URL
						m_PayPalCCA.AssignPaydiantCredential(UPDATE_CREDENTIAL_HOSTURL, strTemp);
					#endif
				}
				break;
			// End of [#2449]

			// [#RWC6-125] US William 2020.02.20 Add support for 'ua' and 't' FIDs
			case 't': // Processor time change
				{		
					// Time String "HHMMSS"
					strTemp.Format(L"%s", strAssortedField.Mid(1, 6));

					if (strTemp.GetLength() != 6)
					{
						NHDBG((L"Time format not correct length\r\n"));
						break;
					}

					int hour = Asc2Int(strTemp.Mid(0, 2));
					int minute = Asc2Int(strTemp.Mid(2, 2));
					int second = Asc2Int(strTemp.Mid(4, 2));
					
					bool invalid = 
						(hour < 0 || hour >= 24) ||
						(minute < 0 || minute >= 60) ||
						(second < 0 || second >= 60);
					if (invalid)
					{
						NHDBG((L"Time string is invalid: %s\r\n", strTemp));
						break;
					}

					// Set date
					systemTime.wHour = hour;
					systemTime.wMinute = minute;
					systemTime.wSecond = second;

					receivedTime = true;
					NHDBG((L"Received new system time\r\n"));

					break;
				}


			default:
				break;
		}
	}
#if SUPPORT_CHANGE_PARAMETER_JNL		//[#610] SOOK 2010.01.14 Configuration 저널 저장 
		m_pDevCmn->SaveChangeParameter(CHANGE_USER_HOST);	//STD1에는 HOST에서 내려온 것 들을 저장하는 부분이 없음 //나중에 생기면 추가 
#endif		//end of [#610]

	// [#RWC6-125] US William 2020.02.20 Add support for 'ua' and 't' FIDs
	if (receivedDate && receivedTime)
	{
		if (!SetLocalTime(&systemTime))
		{
			NHERROR((L"Unable to set system local time\r\n"));
		}
		else
		{
#if SUPPORT_CHANGE_PARAMETER_JNL
			m_pDevCmn->m_JNLMgr.Save(HOST_ACTION, L"ATM Date & Time Changed");
#endif
		}
	}

	// [#2185] US Justin 2013.05.07 Implement Assorted Fields for "Download Working Key"
	if( bReceivedFirstWorkingKey || bReceivedSecondWorkingKey || bReceivedThirdWorkingKey || bReceivedMACFirstWorkingKey || bReceivedMACSecondWorkingKey )
	{
		bool bDownloadKeys = false;	
		int nCurKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);

		// Check PIN Working Key
		if( bReceivedFirstWorkingKey == true)
		{
			if( (nCurKeyMode==KEYMODE_NON_UNIQ_TDES)||(nCurKeyMode==KEYMODE_UNIQ_TDES)||(nCurKeyMode==KEYMODE_TDES_MACING)||(nCurKeyMode==KEYMODE_TDES_TMACING) )
			{
				if( bReceivedSecondWorkingKey == true )
				{
					bDownloadKeys = true;
					if( bReceivedThirdWorkingKey != true)
					{
						strTmp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1);
						MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY3, (LPCTSTR)strTmp);
					}
				}
			}
			else
				bDownloadKeys = true;
		}

		if( bDownloadKeys == true )
		{
			int nWorkingRes;
			CString strOperatorAction;
			int nMKeyName = MASTERKEY_ATM;
			if (m_HostConfig == HC_DUALHOST)		nMKeyName = MASTERKEY_DUALHOST;
			else if (m_HostConfig == HC_LIBERTYX)	nMKeyName = MASTERKEY_LIBERTYX;
			nWorkingRes = LIB_LoadWorkingKey(nMKeyName);

			if (nWorkingRes == T_OK)
				strOperatorAction.Format(L"Download Working Key(%d) - Success", nMKeyName);
			else
				strOperatorAction.Format(L"Download Working Key(%d) - Fail(%d)", nMKeyName, nWorkingRes);
			m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, strOperatorAction);
		}
	}
	// End of [#2185]
	return nEMVResult;	// [#492] [NH] KSK 2009.2.5
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD1_CheckResponseCode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD1_CheckResponseCode(CString strResponseCode)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_CheckResponseCode]\n"));

	int nCodeIndex = 0;

//	for( nCodeIndex =0; nCodeIndex < sizeof(STD2_RSP_TBL) / sizeof(_RSP_TBL); nCodeIndex++)
	for( nCodeIndex =0; nCodeIndex < sizeof(STD1_RSP_TBL) / sizeof(_RSP_TBL); nCodeIndex++)	// KSK 2010.11.25 Response Code Bug Fix
	{
//		if( (LPCSTR)STD2_RSP_TBL[nCodeIndex].CODE == strResponseCode )
		if( (LPCSTR)STD1_RSP_TBL[nCodeIndex].CODE == strResponseCode )		// KSK 2010.11.25 Response Code Bug Fix
			return nCodeIndex;
	}
	return -1;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD1_SetConfigRequestInitiator()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD1_SetConfigRequestInitiator(CString strCmd)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_SetConfigRequestInitiator]\n"));

	if (!strCmd.CompareNoCase(L"00"))
	{
		// [#2176] Host에서 ConfigRequestInitiator Flag 초기화
		m_pDevCmn->HostConfigCmd = 0;
		// end of [#2176]
	}
	else if (!strCmd.CompareNoCase(L"01"))		// 재개국 요청 시, 재 개국함.	(Type 88)
	{
		m_pDevCmn->HostConfigCmd = TC_RECONFIGURATION;	// [#144] KSK 2008.04.20
	}
	else if (!strCmd.CompareNoCase(L"80") || !strCmd.CompareNoCase(L"4"))		// [#2075] NH KSK 2011.06.27
	{
		m_pDevCmn->HostConfigCmd = TC_EXTENDED_RKT_TMK;
	}
	else if (!strCmd.CompareNoCase(L"81") || !strCmd.CompareNoCase(L"5"))
	{
		m_pDevCmn->HostConfigCmd = TC_EXTENDED_RKT_MMK;
	}																			// end of [#2075]
	else if (!strCmd.CompareNoCase(L"82") || !strCmd.CompareNoCase(L"2"))	// [#560] NH KSK 2009.8.20
	{
		m_pDevCmn->HostConfigCmd = TC_EXTENDED_AID_UPDATE;
	}
	else if (!strCmd.CompareNoCase(L"86") || !strCmd.CompareNoCase(L"6"))	// [#2076] NH KSK 2011.06.28
	{
		m_pDevCmn->HostConfigCmd = TC_EXTENDED_EJUPLOAD;
	}
	else if (!strCmd.CompareNoCase(L"87") || !strCmd.CompareNoCase(L"7"))	// [#2449] US Justin 2016.11.08
	{
		#if (APP_PAYDIANT_CCA)
			m_pDevCmn->HostConfigCmd = TC_EXTENDED_ADDSVC_CONFIG;
		#else
			m_pDevCmn->HostConfigCmd = 0;
		#endif
	}
	else if (!strCmd.CompareNoCase(L"90") || !strCmd.CompareNoCase(L"8"))
	{
		m_pDevCmn->HostConfigCmd = TC_TR34_LOAD;
	}
	else if (!strCmd.CompareNoCase(L"9"))
	{
		m_pDevCmn->HostConfigCmd = TC_TR34_UNBIND;
	}
	else
	{
		// [#2176] NH KSK 2013.01.25 Define되지 않은 값이 오는 경우는 무시처리해야하므로 초기화 처리
		m_pDevCmn->HostConfigCmd = 0;
		// end of [#2176]
	}
	// end of [#2076]

	return RES_OK;
}

// [#103] NH 2008.04.23 PSC
UINT CTranCmn::BIZ_STD1_GetFID(char fid2)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD1_GetFID]\n"));

	switch(fid2)
	{
	case 'a':		return FID2_a;
	case 'b':		return FID2_b;
	case 'c':		return FID2_c;
	case 'd':		return FID2_d;
	case 'e':		return FID2_e;
	case 'f':		return FID2_f;
	case 'g':		return FID2_g;
	case 'h':		return FID2_h;
	case 'i':		return FID2_i;
	case 'j':		return FID2_j;
	case 'k':		return FID2_k;
	case 'l':		return FID2_l;
	case 'm':		return FID2_m;
	case 'n':		return FID2_n;
//	case 'o':		return FID2_o;		//	[#438] AU AIREAT 2008.10.17	- 사용안함
	case '!':		return FID2_21h;	//	[#438] AU AIREAT 2008.10.17	- 'o' -> '!'(21h)로 변경.
	case 'p':		return FID2_p;
	case 'q':		return FID2_q;
	case 'r':		return FID2_r;
	case 's':		return FID2_s;
	case 't':		return FID2_t;
	case 'u':		return FID2_u;
	case 'v':		return FID2_v;
	case 'w':		return FID2_w;
	case 'x':		return FID2_x;
	default:		return 0xffffffff;
	}
}
// end of [#103]
