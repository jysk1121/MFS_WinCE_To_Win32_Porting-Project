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
 FUNCTION NAME: BIZ_SendHost()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_SendHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_SendHost]\n"));

	BIZ_RETURN	nRes;

	m_strSendData.Empty();

	m_strMacSendResult.Empty();		// [#2135] NH KSK 2012.05.07
	
	m_nSendLength = 0;


// [#RWC6-59] US William 2019.10.08 LibertyX
#if (APP_LIBERTYX)
	if ( m_HostConfig == HC_LIBERTYX )
	{
		if ( m_LXConfig.HostProtocol == LX_STANDARD1 )
		{
			nRes = BIZ_STD1_SendHost();
		}
		else if ( m_LXConfig.HostProtocol == LX_STANDARD3 )
		{
			nRes = BIZ_STD3_SendHost();
		}
		else 
		{
			nRes = RES_NG;
		}
	}
	else
#endif
// End of [#RWC6-59]

	// [#2185] US Justin 2013.05.03 PAI Dual Host DCC...
	if( m_HostConfig == HC_DUALHOST )
	{
		nRes = BIZ_STD1_SendHost();
	}
	else
#if (APP_PIN4_CASHPICKUP)		// [#2471] US Justin 2017.02.01 Enable Pin4 to all customers
	if(TranCode == TC_PIN4)
	{
		//nRes = BIZ_Pin4_SendHost(PIN4_REQ_TRANSACTION);
		nRes = BIZ_Pin4_SendHost();							// [#2515] US Justin
	}
	// [#2515] US Justin
	/*
	else if( (TranCode==TC_REVERSAL) && (WITHDRAWAL_PIN4==MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE)) )
	{
		nRes = BIZ_Pin4_SendHost(PIN4_REQ_REVERSAL);
	}
	*/
	else
#endif
// [#2445] US Justin 2016.09.28 Just.Cash
#if (APP_JUST_CASH)
	if ((TranCode == TC_JUSTCASH) || (TranCode == TC_JUSTCASH_BITCOIN)) // [#2496] Justin 2017.08.15 Add Bit Coin
	{
		nRes = BIZ_JustCash_SendHost();
	}
	else
#endif
// End of [#2445]
#if (APP_PAYDIANT_CCA)	// [#2446] US Justin 2016.09.29 Paypay CCA
	if(TranCode == TC_PAYPALCCA)
	{
		nRes = BIZ_PaypalCCA_SendHost();
	}	
	else
#endif					// End of [#2446]
#if (APP_B4U)
	if (TranCode == TC_B4U)
	{
		if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)			nRes = BIZ_STD1_SendHost();
		else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)		nRes = BIZ_STD3_SendHost();
		else																					nRes = RES_NG;
	}
	else
#endif
#if (APP_GPAY_GIFTCARD_PURCHASE)		// [#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
	if( TranCode == TC_GP_BUY_CARD ) 
	{
		nRes = BIZ_GivePay_SendHost();
	}	
	else
#endif
		// End of [#2513]
	{
		// KSK 2010.02.02
		if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
			nRes = BIZ_STD1_SendHost();
		else
		if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
			nRes = BIZ_STD3_SendHost();
		else
			nRes = BIZ_STD2_SendHost();
		// end of KSK 2010.02.02
	}
	// End of [#2185]

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_RecvHost()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_RecvHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_RecvHost]\n"));

	BIZ_RETURN	nRes;

	// [#RWC6-59] US William 2019.10.08 LibertyX
#if (APP_LIBERTYX)
	if ( m_HostConfig == HC_LIBERTYX )
	{
		if ( m_LXConfig.HostProtocol == LX_STANDARD1 )
		{
			nRes = BIZ_STD1_RecvHost();
		}
		else if ( m_LXConfig.HostProtocol == LX_STANDARD3 )
		{
			nRes = BIZ_STD3_RecvHost();
		}
		else 
		{
			nRes = RES_NG;
		}
	}
	else
#endif
// End of [#RWC6-59]
	// [#2185] US Justin 2013.05.03 PAI Dual Host DCC...
	if( m_HostConfig == HC_DUALHOST )
	{
		nRes = BIZ_STD1_RecvHost();
	}
	else
//#endif
#if (APP_PIN4_CASHPICKUP)		// [#2471] US Justin 2017.02.01 Enable Pin4 to all customers
	if(TranCode == TC_PIN4)
	{
		//nRes = BIZ_Pin4_RecvHost(PIN4_REQ_TRANSACTION);
		nRes = BIZ_Pin4_RecvHost();					// [#2515]
	}	
	// [#2515] US Justin
	/*
	else if( (TranCode==TC_REVERSAL) && (WITHDRAWAL_PIN4==MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE)) )
	{
		nRes = BIZ_Pin4_RecvHost(PIN4_REQ_REVERSAL);
	}
	*/ // End of [#1515]
	else
#endif
// [#2445] US Justin 2016.09.28 Just.Cash
#if (APP_JUST_CASH)
	if ((TranCode == TC_JUSTCASH) || (TranCode == TC_JUSTCASH_BITCOIN)) // [#2496] Justin 2017.08.15 Add Bit Coin
	{
		nRes = BIZ_JustCash_RecvHost();
	}
	else
#endif
// End of [#2445]
#if (APP_PAYDIANT_CCA)	// [#2446] US Justin 2016.09.28 Paypal 
	if(TranCode == TC_PAYPALCCA)
	{
		nRes = BIZ_PaypalCCA_RecvHost();
	}	
	else
#endif					// End of [#2446]
#if (APP_B4U)
	if(TranCode == TC_B4U)
	{
		if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)			nRes = BIZ_STD1_RecvHost();
		else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)		nRes = BIZ_STD3_RecvHost();
		else																					nRes = RES_NG;
	}
	else
#endif
#if (APP_GPAY_GIFTCARD_PURCHASE)		// [#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
	if( TranCode == TC_GP_BUY_CARD ) 
	{
		nRes = BIZ_GivePay_RecvHost();
	}	
	else
#endif									// End of [#2513]
	{
		// KSK 2010.02.02
		if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
			nRes = BIZ_STD1_RecvHost();
		else
		if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
			nRes = BIZ_STD3_RecvHost();
		else
			nRes = BIZ_STD2_RecvHost();
		// end of KSK 2010.02.02
	}
	// End of [#2185]

	return nRes;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_SendData()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_SendData()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_SendData] Length(%d), Data(%c%c)\n", m_nSendLength, m_arSendBuffer[0], m_arSendBuffer[1]));

	HexaDump(m_arSendBuffer, m_nSendLength, 1, FALSE, FILE_LOG_TYPE);	// [#2452] NH KSK 2016.11.10

#ifdef APP_LOCAL_MODE
	m_pDevCmn->TranResult = TRUE;
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);
	return RES_OK;
#endif

	// KSK 2010.02.02 전역 Buffer를 사용하므로 전역 Buffer를 곧바로 사용하도록 수정 (전역 Buffer 초기화 문제)
	// [#2185] US Justin 2013.05.06 Support Dual Host DCC
	//return m_pDevCmn->fnNET_SendData(m_arSendBuffer, m_nSendLength, K_300_WAIT);
	int nTranHostType = TRANHOST_ATM;
	if(m_HostConfig == HC_DUALHOST)	nTranHostType = TRANHOST_DUALHOST;
	else if (m_HostConfig == HC_LIBERTYX) nTranHostType = TRANHOST_LIBERTYX;

	CString strExtraHostInfo = L"";

	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_SendData] nTranHostType=[%d], strExtraHostInfo = [%s]\n", nTranHostType, strExtraHostInfo));
	return m_pDevCmn->fnNET_SendData(m_arSendBuffer, m_nSendLength, K_300_WAIT, nTranHostType, strExtraHostInfo);
	// End of [#2467]
	// End of [#2185]
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_RecvData()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_RecvData()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_RecvData]\n"));

#ifdef APP_LOCAL_MODE
	return RES_OK;
#endif

	m_nRecvLength = NETBUF_RECV_SIZE;
	memset(m_arRecvBuffer, 0x00, NETBUF_RECV_SIZE);

	// [#2185] US Justin 2013.05.06 Support Dual Host DCC
	//m_pDevCmn->fnNET_RecvData( m_arRecvBuffer, &m_nRecvLength, K_180_WAIT );
	int nTranHostType = TRANHOST_ATM;
	if(m_HostConfig == HC_DUALHOST)	nTranHostType = TRANHOST_DUALHOST;

	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_RecvData] nTranHostType=[%d]\n", nTranHostType));
	m_pDevCmn->fnNET_RecvData( m_arRecvBuffer, &m_nRecvLength, K_180_WAIT, nTranHostType);
	// End of [#2185]

	HexaDump(m_arRecvBuffer, m_nRecvLength, 2, FALSE, FILE_LOG_TYPE);	// [#2452] NH KSK 2016.11.10
	// end of [#2054]

	if (!m_nRecvLength)
	{
		m_nRecvLength = 0;
		memset(m_arRecvBuffer, 0, NETBUF_RECV_SIZE);
		NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_RecvData]  Data Length is ZERO.... ERROR RETURN \n"));
		return T_RECVERROR;
	}
	// end of [#2012]
	return RES_OK;
}

// [#2396] US Justin 2016.02.17 Pin4
#if (APP_PIN4_CASHPICKUP) // (APP_CUSTOM_PAI)		[#2471] US Justin 2017.02.01 Open to all customers
BIZ_RETURN	CTranCmn::BIZ_Pin4_SendHost()		// [#2515] US Justin Remove Parameter
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_Pin4_SendHost() \n" ));

	// [#2515] US Justin
	NHDEBUG(DBG_CALL, (L"    Pin4 Step = [%d]\n", m_Pin4.m_nTransactionStep));
	if( m_Pin4.m_nTransactionStep == PIN4_TRAN_AUTH_PROC )
	{
		BIZ_RETURN	nRes;
		if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)			nRes = BIZ_STD1_SendHost();
		else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)		nRes = BIZ_STD3_SendHost();
		else																					nRes = BIZ_STD2_SendHost();
		return nRes;
	}
	// End of [#2515]

	// Initialize Parmeters
	CString strTemp;
	m_pDevCmn->TranResult = FALSE;
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, FALSE);
	m_nSendLength = 0;												// Send Length
	//memset(m_arSendBuffer, 0, sizeof(m_arSendBuffer));			// Send Buffer
	memset(m_arSendBuffer, 0, NETBUF_SEND_SIZE);

	// Set necessary parameters of PIN4 class
	if( m_Pin4.m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )
	{
		// Increase Sequence Number
		m_pDevCmn->fnAPL_AddSerialNo(m_HostConfig);
		m_Pin4.m_strTranSeqNo = m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig);

		// Assign transaction Amount
		int nAmt = Asc2Int(m_sUserSelection.strMoney);
		strTemp.Format(L"%d", nAmt);
		m_Pin4.m_strTranAmount = strTemp;

		// Assign Transaction data
		m_Pin4.SetTransactionData();
	}

	CString strStatusMonitoringField = L"";
	if( (m_Pin4.m_nTransactionStep == PIN4_TRAN_AUTH_PIN4) || (m_Pin4.m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4) )
	{
		// Make STD3 Status Monitoring Field
		m_strSendData.Empty();
		BIZ_STD3_Make_StatusMonitoringField();	
		strStatusMonitoringField = m_strSendData;
		NHDEBUG(DBG_CALL, (L"Status Monitoring Header = [%s]\n", strStatusMonitoringField));
	}

	// Make Body
	CString strSendBody = m_Pin4.MakeRequestMessage(strStatusMonitoringField);

	// Header
	if(m_Pin4.m_nTransactionStep == PIN4_TRAN_GETTOKEN)
	{
		// [#2576] US Justin 2018.09.17 Add Pin4 HTTP Header Parsing Logic 
		//m_strSendData.Format( L"POST /%s HTTP/1.1\r\nHost: %s:%s\r\nContent-type: application/json\r\nAccept: application/json\r\nContent-length: %d\r\n\r\n", 
		//	m_Pin4.m_strAuthHostProcess, m_Pin4.m_strAuthHostIP, m_Pin4.m_strAuthHostPort, strSendBody.GetLength() );
		m_strSendData.Format( L"POST /%s HTTP/1.1\r\nHost: %s:%s\r\nConnection: close\r\nContent-type: application/json\r\nAccept: application/json\r\nContent-length: %d\r\n\r\n", 
			m_Pin4.m_strAuthHostProcess, m_Pin4.m_strAuthHostIP, m_Pin4.m_strAuthHostPort, strSendBody.GetLength() );
		// End of [#2576]
	}
	else if( (m_Pin4.m_nTransactionStep == PIN4_TRAN_AUTH_PIN4) || (m_Pin4.m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4) )
	{
		// [#2576] US Justin 2018.09.07 Add Pin4 HTTP Header Parsing Logic - add "Connection: close\r\n"
		//m_strSendData.Format( L"POST /%s HTTP/1.1\r\nHost: %s:%s\r\nContent-type: text/xml\r\nContent-length: %d\r\n\r\n", 
		//	m_Pin4.m_strHostProcess, m_Pin4.m_strHostIP, m_Pin4.m_strHostPort, strSendBody.GetLength() );
		m_strSendData.Format( L"POST /%s HTTP/1.1\r\nHost: %s:%s\r\nConnection: close\r\nContent-type: text/xml\r\nContent-length: %d\r\n\r\n", 
			m_Pin4.m_strHostProcess, m_Pin4.m_strHostIP, m_Pin4.m_strHostPort, strSendBody.GetLength() );
		// End of [#2576]
	}

	// Body
	m_strSendData += strSendBody;
	NHDEBUG(DBG_CALL, (L"  [Pin4 Send Data] Data Length(%d)\n", m_strSendData.GetLength()) );

	////////////////////////////////////////////
	// Set Buffer
	memset(m_szTemp, NULL, sizeof(m_szTemp));
	WideToMulti(m_szTemp, m_strSendData, sizeof(m_szTemp));

	memcpy(m_arSendBuffer, m_szTemp, m_strSendData.GetLength());
	m_nSendLength = m_strSendData.GetLength();

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Save request message to file (overwite previous transaction. Leave the latest transaction only)
	CString strLogFileName;
	strLogFileName.Format( _T("%s\\PIN4_Req%02d.dat"), ATM_DATA_PATH, m_Pin4.m_nTransactionStep );
	NHDEBUG(DBG_CALL, (L"LOG FILE = [%s]\n", strLogFileName));
	CFile cf;
	if( cf.Open( strLogFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary ) == FALSE )
		NHDEBUG(DBG_CALL, (L"[FAIL TO OPEN(%s)]\n", strLogFileName));
	else
	{
		cf.Write(m_arSendBuffer, m_nSendLength );
		cf.Close();
	}
	// Save request message to file (overwite previous transaction. Leave the latest transaction only)
	//////////////////////////////////////////////////////////////////////////////////////////////////

	#ifdef APP_LOCAL_MODE
		Delay_Msg(1000);
		return RES_OK;
	#endif

	CString strPin4HostInfo = L"";			// IP, PORT, SSL Option, Host Cert
	if(m_Pin4.m_nTransactionStep == PIN4_TRAN_GETTOKEN)
	{
		strPin4HostInfo.Format(L"%s,%s,", m_Pin4.m_strAuthHostIP, m_Pin4.m_strAuthHostPort);
		if( m_Pin4.m_strAuthHostSSL == L"1" )	strTemp.Format( L"%d,0", CONN_UPTO_TLS_V12);		
		else									strTemp = _T("0,0");
		strPin4HostInfo += strTemp;
	}
	else if( (m_Pin4.m_nTransactionStep == PIN4_TRAN_AUTH_PIN4) || (m_Pin4.m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4) )
	{
		strPin4HostInfo.Format(L"%s,%s,", m_Pin4.m_strHostIP, m_Pin4.m_strHostPort);
		if( m_Pin4.m_strHostSSL == L"1" )
		{
			// [#2524] US Justin 2018.01.02 Download Certificate
			/*
			if(m_Pin4.m_strHostSSLCert.GetLength() > 0)	strTemp.Format( L"%d,%s", CONN_UPTO_TLS_V12_USE_CERT, m_Pin4.GetPin4FileName(PIN4_FILE_SSLCERT) );
			else										strTemp.Format( L"%d,0", CONN_UPTO_TLS_V12);		
			*/
			CString strCertFile = m_Pin4.GetPin4FileName(PIN4_FILE_SSLCERT);
			if(strCertFile.GetLength()>4)
				strTemp.Format( L"%d,%s", CONN_UPTO_TLS_V12_USE_CERT, strCertFile );
			else				
				strTemp.Format( L"%d,0",  CONN_UPTO_TLS_V12);		
			// End of [#2524]
		}
		else
			strTemp = _T("0,0");
		strPin4HostInfo += strTemp;
	}
	NHDEBUG(DBG_CALL, (L"  [Pin4 Host Information] = [%s]\n", strPin4HostInfo) );

	int nReqType = TRANHOST_PIN4_GETTOKEN;			// PIN4 Req Type (to determine the terminate string)
	if(     m_Pin4.m_nTransactionStep == PIN4_TRAN_GETTOKEN)		nReqType = TRANHOST_PIN4_GETTOKEN;
	else if(m_Pin4.m_nTransactionStep == PIN4_TRAN_AUTH_PIN4)		nReqType = TRANHOST_PIN4_TRANSACTION;
	else if(m_Pin4.m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4)	nReqType = TRANHOST_PIN4_REVERSAL;

	if( m_pDevCmn->fnNET_SendData(m_arSendBuffer, m_nSendLength, K_300_WAIT, nReqType, strPin4HostInfo) != RES_OK )
	{
		m_pDevCmn->fnAPL_StackError(m_pDevCmn->fstrNET_GetErrorCode(), m_pDevCmn->fstrNET_GetErrorMsg(), DEV_NET);		// [#2554] US Justin 2018.06.07 Leave Send Error Code
		return RES_HOST_SEND_ERR;
	}
	return RES_OK;
}

BIZ_RETURN CTranCmn::BIZ_Pin4_RecvHost()		// [#2515] US Justin Remove Parameter
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_Pin4_RecvHost()]\n"));

	// [#2515] US Justin
	NHDEBUG(DBG_CALL, (L"    Pin4 Step = [%d]\n", m_Pin4.m_nTransactionStep));
	if( m_Pin4.m_nTransactionStep == PIN4_TRAN_AUTH_PROC )
	{
		BIZ_RETURN	nRes;
		if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)			nRes = BIZ_STD1_RecvHost();
		else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)		nRes = BIZ_STD3_RecvHost();
		else																					nRes = BIZ_STD2_RecvHost();		
		return nRes;
	}
	// End of [#2515]

	CString strXMLString = L"";
		
	#ifdef APP_LOCAL_MODE
		strXMLString = m_Pin4.MakeDemoResponseMessage();
		Delay_Msg(1000);
	#else
		m_nRecvLength = NETBUF_RECV_SIZE;
		memset(m_arRecvBuffer, 0x00, NETBUF_RECV_SIZE);

		int nReqType = TRANHOST_PIN4_GETTOKEN;			// PIN4 Req Type (to determine the terminate string)
		if(     m_Pin4.m_nTransactionStep == PIN4_TRAN_GETTOKEN)		nReqType = TRANHOST_PIN4_GETTOKEN;
		else if(m_Pin4.m_nTransactionStep == PIN4_TRAN_AUTH_PIN4)		nReqType = TRANHOST_PIN4_TRANSACTION;
		else if(m_Pin4.m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4)	nReqType = TRANHOST_PIN4_REVERSAL;

		m_pDevCmn->fnNET_RecvData( m_arRecvBuffer, &m_nRecvLength, K_180_WAIT, nReqType ); 

		if (!m_nRecvLength)
		{
			m_nRecvLength = 0;
			memset(m_arRecvBuffer, 0, NETBUF_RECV_SIZE);
			m_Pin4.SetErrorCodeAndDesc(L"900");
			m_pDevCmn->fnAPL_StackError(m_Pin4.m_strErrorCode, m_Pin4.m_strErrorDesc, DEV_NET);
			NHDEBUG(DBG_INFO, (L"Pin4 : no data received\n"));
			return RES_HOST_RECV_ERR;
		}
		strXMLString = (LPCSTR) m_arRecvBuffer;	

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Save response message to file (overwite previous transaction. Leave the latest transaction only)
		CString strLogFileName;
		strLogFileName.Format( _T("%s\\PIN4_Res%02d.dat"), ATM_DATA_PATH, m_Pin4.m_nTransactionStep );
		NHDEBUG(DBG_CALL, (L"LOG FILE = [%s]\n", strLogFileName));

		CFile cf;
		if( cf.Open( strLogFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary ) == FALSE )
			NHDEBUG(DBG_CALL, (L"[FAIL TO OPEN(%s)]\n", strLogFileName));
		else
		{
			int nSize = strXMLString.GetLength();
			char *pBuffer = new char[nSize+1];
			memset(pBuffer, 0, nSize+1);
			WideToMulti( pBuffer, strXMLString, nSize);
			cf.Write(pBuffer, nSize );
			delete [] pBuffer;
			cf.Close();
		}
		// Save response message to file (overwite previous transaction. Leave the latest transaction only)
		///////////////////////////////////////////////////////////////////////////////////////////////////
	#endif

	if( strXMLString.GetLength() <= 1024)
		NHDEBUG(DBG_CALL, (L"RECEIVED DATA = \n%s\n", strXMLString ));
	else
		NHDEBUG(DBG_CALL, (L"RECEIVED DATA, Length = [%d]\n%s\n...................\n%s\n", strXMLString.GetLength(), strXMLString.Left(450), strXMLString.Right(450) ));

	int nParseResult = m_Pin4.ParseReceivedData(strXMLString);
	if( nParseResult != PIN4_RESULT_OK)
	{
		m_pDevCmn->fnAPL_StackError(m_Pin4.m_strErrorCode, m_Pin4.m_strErrorDesc, DEV_NET);
		NHDEBUG(DBG_CALL, (L"[Received Data Error : Code[%s], Desc[%s]\n", m_Pin4.m_strErrorCode, m_Pin4.m_strErrorDesc));
		if(nParseResult==PIN4_RCV_ERROR)
			return RES_HOST_RECV_ERR;
		else
			return RES_HOST_DENIED;
	}

	// Approved
	m_pDevCmn->TranResult = TRUE;
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);
	NHDEBUG(DBG_CALL, (L"m_Pin4 host comminication OK\n"));
	return RES_OK;
}
// End of [#2396]
#endif

// [#2445] US Justin 2016.09.28 Just.Cash
#if (APP_JUST_CASH)		
BIZ_RETURN	CTranCmn::BIZ_JustCash_SendHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_JustCash_SendHost() \n"));
	NHDEBUG(DBG_CALL, (L"    Just.Cash Step = [%d]\n", m_JustCashData.m_nTransactionStep));

	if( (m_JustCashData.m_nTransactionStep == JUSTCASH_CARDLESS_TRAN_HOSTAPPROVAL) || 
		(m_JustCashData.m_nTransactionStep == JUSTCASH_DIGITAL_CUR_HOSTAPPROVAL) )		// [#2496] US Justin 2017.08.14 Add Bit Coin
	{
		BIZ_RETURN	nRes;
		if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)			nRes = BIZ_STD1_SendHost();
		else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)		nRes = BIZ_STD3_SendHost();
		else																					nRes = BIZ_STD2_SendHost();
		return nRes;
	}

	// Initialize Parmeters
	m_pDevCmn->TranResult = FALSE;
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, FALSE);
	m_nSendLength = 0;												// Send Length
	//memset(m_arSendBuffer, 0, sizeof(m_arSendBuffer));				// Send Buffer
	memset(m_arSendBuffer, 0, NETBUF_SEND_SIZE);

	// Make Body
	CString strErrorCode = L"";
	if( m_JustCashData.m_nTransactionStep == JUSTCASH_CARDLESS_TRAN_UPDATETRAN )
		strErrorCode = m_pDevCmn->fstrAPL_GetErrorCode();
	
	// [#2528] US Justin 2018.01.29	Just.Cash Change Login Method
	// US William 2020.08.11 Add card has for KYC on purchase call
	CString strLast6Digit = L" ";
	if(m_JustCashData.m_nTransactionStep == JUSTCASH_DIGITAL_CUR_COMMIT || m_JustCashData.m_nTransactionStep == JUSTCASH_DIGITAL_CUR_PURCHASE)
	{
		CString strTrack2Data = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2);
		int nResult = strTrack2Data.Find(L"=");
		if(nResult > 6 )
		{
			strTrack2Data = strTrack2Data.Left(nResult);
			strLast6Digit = strTrack2Data.Right(6);
			NHDEBUG(DBG_CALL, (L"Card Last 6 digit = [%s]\n", strLast6Digit));
		}
	}
	// End of [#2528]

	CString strSendProcess = m_JustCashData.MakeRequestMessage(MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID), strErrorCode, strLast6Digit);
	CString	strSendBody = L"";

	// Header
	m_strSendData.Format( L"POST /%s HTTP/1.1\r\nConnection: close\r\nHost: %s:%s\r\nContent-type: application/json\r\nAccept: application/json\r\nContent-length: %d\r\n\r\n", 
		strSendProcess, m_JustCashData.m_strHostIP, m_JustCashData.m_strHostPort, strSendBody.GetLength() );

	// Body
	m_strSendData += strSendBody;
	NHDEBUG(DBG_CALL, (L"  [Just.Cash Send Data] : Length=[%d]\n%s", m_strSendData.GetLength(), m_strSendData) );

	////////////////////////////////////////////
	// Set Buffer
	memset(m_szTemp, NULL, sizeof(m_szTemp));
	WideToMulti(m_szTemp, m_strSendData, sizeof(m_szTemp));

	memcpy(m_arSendBuffer, m_szTemp, m_strSendData.GetLength());
	m_nSendLength = m_strSendData.GetLength();

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Save request message to file (overwite previous transaction. Leave the latest transaction only)
	CString strLogFileName;
	strLogFileName.Format( _T("%s\\JustCash_Req%02d.dat"), ATM_DATA_PATH, m_JustCashData.m_nTransactionStep );
	NHDEBUG(DBG_CALL, (L"LOG FILE = [%s]\n", strLogFileName));
	CFile cf;
	if( cf.Open( strLogFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary ) == FALSE )
		NHDEBUG(DBG_CALL, (L"[FAIL TO OPEN(%s)]\n", strLogFileName));
	else
	{
		cf.Write(m_arSendBuffer, m_nSendLength );
		cf.Close();
	}
	// Save request message to file (overwite previous transaction. Leave the latest transaction only)
	//////////////////////////////////////////////////////////////////////////////////////////////////

	#ifdef APP_LOCAL_MODE
		Delay_Msg(1000);
		return RES_OK;
	#endif

	// IP, PORT, SSL Option, Host Cert
	CString strTemp, strHostInfo;
	strHostInfo.Format(L"%s,%s,", m_JustCashData.m_strHostIP, m_JustCashData.m_strHostPort);
	if( m_JustCashData.m_strHostSSL == L"1" )	strTemp.Format( L"%d,0", CONN_UPTO_TLS_V12);		
	else										strTemp = _T("0,0");
	strHostInfo += strTemp;
	NHDEBUG(DBG_CALL, (L"  [Just.Cash Host Information] = [%s]\n", strHostInfo) );

	if( m_pDevCmn->fnNET_SendData(m_arSendBuffer, m_nSendLength, K_300_WAIT, TRANHOST_JUSTCASH_TRANSACTION, strHostInfo) != RES_OK )
		return RES_HOST_SEND_ERR;

	return RES_OK;
}

BIZ_RETURN CTranCmn::BIZ_JustCash_RecvHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_JustCash_RecvHost()]\n"));
	if( (m_JustCashData.m_nTransactionStep == JUSTCASH_CARDLESS_TRAN_HOSTAPPROVAL) || 
		(m_JustCashData.m_nTransactionStep == JUSTCASH_DIGITAL_CUR_HOSTAPPROVAL) )		// [#2496] US Justin 2017.08.14 Add Bit Coin
	{
		BIZ_RETURN	nRes;
		if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)			nRes = BIZ_STD1_RecvHost();
		else if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)		nRes = BIZ_STD3_RecvHost();
		else																					nRes = BIZ_STD2_RecvHost();		
		return nRes;
	}
		
	CString strXMLString = L"";

	#ifdef APP_LOCAL_MODE
		strXMLString = m_JustCashData.MakeDemoResponseMessage();
		Delay_Msg(1000);
	#else
		m_nRecvLength = NETBUF_RECV_SIZE;
		memset(m_arRecvBuffer, 0x00, NETBUF_RECV_SIZE);

		m_pDevCmn->fnNET_RecvData( m_arRecvBuffer, &m_nRecvLength, K_180_WAIT, TRANHOST_JUSTCASH_TRANSACTION ); 
		if (!m_nRecvLength)
		{
			m_nRecvLength = 0;
			memset(m_arRecvBuffer, 0, NETBUF_RECV_SIZE);
			m_JustCashData.SetErrorCodeAndDesc(L"900");
			m_pDevCmn->fnAPL_StackError(m_JustCashData.m_strErrorCode, m_JustCashData.m_strErrorDesc, DEV_NET);
			NHDEBUG(DBG_INFO, (L"Just.Cash : no data received\n"));
			return RES_HOST_RECV_ERR;
		}
		strXMLString = (LPCSTR) m_arRecvBuffer;	

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Save response message to file (overwite previous transaction. Leave the latest transaction only)
		CString strLogFileName;
		strLogFileName.Format( _T("%s\\JustCash_Res%02d.dat"), ATM_DATA_PATH, m_JustCashData.m_nTransactionStep );
		NHDEBUG(DBG_CALL, (L"LOG FILE = [%s]\n", strLogFileName));

		CFile cf;
		if( cf.Open( strLogFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary ) == FALSE )
			NHDEBUG(DBG_CALL, (L"[FAIL TO OPEN(%s)]\n", strLogFileName));
		else
		{
			int nSize = strXMLString.GetLength();
			char *pBuffer = new char[nSize+1];
			memset(pBuffer, 0, nSize+1);
			WideToMulti( pBuffer, strXMLString, nSize);
			cf.Write(pBuffer, nSize );
			delete [] pBuffer;
			cf.Close();
		}
		// Save response message to file (overwite previous transaction. Leave the latest transaction only)
		///////////////////////////////////////////////////////////////////////////////////////////////////
	#endif
	
	if( strXMLString.GetLength() <= 1024)
		NHDEBUG(DBG_CALL, (L"RECEIVED DATA = \n%s\n", strXMLString ));
	else
		NHDEBUG(DBG_CALL, (L"RECEIVED DATA, Length = [%d]\n%s\n...................\n%s\n", strXMLString.GetLength(), strXMLString.Left(450), strXMLString.Right(450) ));


	if( m_JustCashData.ParseReceivedData(strXMLString) != TRUE )
	{
		m_pDevCmn->fnAPL_StackError(m_JustCashData.m_strErrorCode, m_JustCashData.m_strErrorDesc, DEV_NET);
		NHDEBUG(DBG_CALL, (L"[Received Data Error : Code[%s], Desc[%s]\n", m_JustCashData.m_strErrorCode, m_JustCashData.m_strErrorDesc));
		return RES_HOST_DENIED;
	}
	m_pDevCmn->TranResult = TRUE;
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);
	NHDEBUG(DBG_CALL, (L"Just.Cash host comminication OK\n"));
	return RES_OK;
}
#endif
// End of [#2445]

// [#2446] US Justin 2016.09.29 Paypal CCA
#if (APP_PAYDIANT_CCA)		
BIZ_RETURN	CTranCmn::BIZ_PaypalCCA_SendHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_PaypalCCA_SendHost() \n" ));
	NHDEBUG(DBG_CALL, (L"    PaypalCCA_Tran Step = [%d]\n", m_PayPalCCA.m_nTransactionStep));

	if( m_PayPalCCA.m_nTransactionStep == PAYPAL_TRAN_HOSTAPPROVAL )
		return BIZ_STD1_SendHost();

	// Initialize Parameters
	m_pDevCmn->TranResult = FALSE;
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, FALSE);
	m_nSendLength = 0;												// Send Length
	//memset(m_arSendBuffer, 0, sizeof(m_arSendBuffer));				// Send Buffer
	memset(m_arSendBuffer, 0, NETBUF_SEND_SIZE);

	// Make Body
	CString strSendBody = m_PayPalCCA.MakeRequestMessage(L"");

	// Header
	CString strHostURL;
	if( m_PayPalCCA.m_strHostSSL == L"1")	strHostURL = L"https://";
	else									strHostURL = L"http://";
	strHostURL += m_PayPalCCA.m_strHostIP + L"/" + m_PayPalCCA.m_strHostProcess;

	m_strSendData.Format( L"POST %s HTTP/1.1\r\nHost: %s:%s\r\nContent-type: text/xml\r\nContent-length: %d\r\n\r\n", 
		strHostURL, m_PayPalCCA.m_strHostIP, m_PayPalCCA.m_strHostPort, strSendBody.GetLength() );

	// Body
	m_strSendData += strSendBody;
	NHDEBUG(DBG_CALL, (L"  [Paypal Send Data] Data Length(%d)\n", m_strSendData.GetLength()) );

	////////////////////////////////////////////
	// Set Buffer
	memset(m_szTemp, NULL, sizeof(m_szTemp));
	WideToMulti(m_szTemp, m_strSendData, sizeof(m_szTemp));

	memcpy(m_arSendBuffer, m_szTemp, m_strSendData.GetLength());
	m_nSendLength = m_strSendData.GetLength();

	#ifdef APP_LOCAL_MODE
		Delay_Msg(1000);
		return RES_OK;
	#endif

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Save request message to file (overwite previous transaction. Leave the latest transaction only)
	CString strLogFileName;
	strLogFileName.Format( _T("%s\\PPCCA_Req%02d.dat"), ATM_DATA_PATH, m_PayPalCCA.m_nTransactionStep );
	NHDEBUG(DBG_CALL, (L"LOG FILE = [%s]\n", strLogFileName));
	CFile cf;
	if( cf.Open( strLogFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary ) == FALSE )
		NHDEBUG(DBG_CALL, (L"[FAIL TO OPEN(%s)]\n", strLogFileName));
	else
	{
		cf.Write(m_arSendBuffer, m_nSendLength );
		cf.Close();
	}
	// Save request message to file (overwite previous transaction. Leave the latest transaction only)
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// IP, PORT, SSL Option, Host Cert
	CString strTemp, strHostInfo;			
	strHostInfo.Format(L"%s,%s,", m_PayPalCCA.m_strHostIP, m_PayPalCCA.m_strHostPort);
	if( m_PayPalCCA.m_strHostSSL == L"1" )	strTemp.Format( L"%d,0", CONN_UPTO_TLS_V12);		
	else									strTemp = _T("0,0");
	strHostInfo += strTemp;

	NHDEBUG(DBG_CALL, (L"  [Paypal.Cash Host Information] = [%s]\n", strHostInfo) );

	if( m_pDevCmn->fnNET_SendData(m_arSendBuffer, m_nSendLength, K_300_WAIT, TRANHOST_PAYPAL_TRANSACTION, strHostInfo) != RES_OK )
		return RES_HOST_SEND_ERR;
	return RES_OK;
}

BIZ_RETURN CTranCmn::BIZ_PaypalCCA_RecvHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_PaypalCCA_RecvHost()]\n"));
	NHDEBUG(DBG_CALL, (L"    PaypalCCA_Tran Step = [%d]\n", m_PayPalCCA.m_nTransactionStep));

	if( m_PayPalCCA.m_nTransactionStep == PAYPAL_TRAN_HOSTAPPROVAL )
		return BIZ_STD1_RecvHost();

	CString strXMLString = L"";
	
	#ifdef APP_LOCAL_MODE
		Delay_Msg(1000);
		strXMLString = m_PayPalCCA.MakeDemoResponseMessage();
	#else
		
		m_nRecvLength = NETBUF_RECV_SIZE;
		memset(m_arRecvBuffer, 0x00, NETBUF_RECV_SIZE);
		m_pDevCmn->fnNET_RecvData( m_arRecvBuffer, &m_nRecvLength, K_180_WAIT, TRANHOST_PAYPAL_TRANSACTION ); 
		if (!m_nRecvLength)
		{
			m_nRecvLength = 0;
			memset(m_arRecvBuffer, 0, NETBUF_RECV_SIZE);
			m_PayPalCCA.SetErrorCodeAndDesc(L"900");
			m_pDevCmn->fnAPL_StackError(m_PayPalCCA.m_strErrorCode, m_PayPalCCA.m_strErrorDesc, DEV_NET);
			NHDEBUG(DBG_INFO, (L"Paypal Transaction : no data received from ATM processor\n"));
			return RES_HOST_RECV_ERR;
		}
		NHDEBUG(DBG_CALL, (L"RECEIVED DATA LENGH = [%d] BYTES\n", m_nRecvLength ));

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Save response message to file (overwite previous transaction. Leave the latest transaction only)
		CString strLogFileName;
		strLogFileName.Format( _T("%s\\PPCCA_Res%02d.dat"), ATM_DATA_PATH, m_PayPalCCA.m_nTransactionStep );
		NHDEBUG(DBG_CALL, (L"LOG FILE = [%s]\n", strLogFileName));
		CFile cf;
		if( cf.Open( strLogFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary ) == FALSE )
			NHDEBUG(DBG_CALL, (L"[FAIL TO OPEN(%s)]\n", strLogFileName));
		else
		{
			cf.Write(m_arRecvBuffer, m_nRecvLength );
			cf.Close();
		}
		// Save response message to file (overwite previous transaction. Leave the latest transaction only)
		///////////////////////////////////////////////////////////////////////////////////////////////////

		strXMLString = CString(m_arRecvBuffer);	
		NHDEBUG(DBG_CALL, (L"Length of strXMLString = [%d]\n", strXMLString.GetLength() ));
	#endif

	if( m_PayPalCCA.ParseReceivedData(strXMLString) != TRUE )
	{
		// Eet communication error for update transaction in OTHER Message
		//if( (m_PayPalCCA.m_nTransactionStep != PAYPAL_TRAN_FULLREVERSAL) && (m_PayPalCCA.m_nTransactionStep != PAYPAL_TRAN_UPDATETRAN) )
		if( m_PayPalCCA.m_nTransactionStep != PAYPAL_TRAN_UPDATETRAN )
			m_pDevCmn->fnAPL_StackError(m_PayPalCCA.m_strErrorCode, m_PayPalCCA.m_strErrorDesc, DEV_NET);
		/*
		else
		{
			// Leave Paydiant Cloud Update Result : PROC COUNT=7 FAIL TO UPDATE TO PAYPAL
			CString strTemp;
			strTemp.Format(L"7%c%5.5s(%2.2s)%c%s",  UNIT_DELIMITER, m_PayPalCCA.m_strErrorCode.Left(5), m_PayPalCCA.m_strErrorCode.Right(2), 
													UNIT_DELIMITER, m_PayPalCCA.m_strErrorDesc);
			CString strExistMsg = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG);		
			if(strExistMsg.GetLength() > 0)
				strExistMsg += CString(UNIT_DELIMITER);
			strExistMsg += strTemp;
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strExistMsg);		
		}
		*/
		NHDEBUG(DBG_CALL, (L"[Received Data Error : Code[%s], Desc[%s]\n", m_PayPalCCA.m_strErrorCode, m_PayPalCCA.m_strErrorDesc));
		return RES_HOST_DENIED;
	}

	// Approved
	m_pDevCmn->TranResult = TRUE;
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);
	NHDEBUG(DBG_CALL, (L"Paypal Transaction Approved\n"));
	return RES_OK;
}
#endif
// End of [#2446]

// [#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
#if (APP_GPAY_GIFTCARD_PURCHASE)		
BIZ_RETURN	CTranCmn::BIZ_GivePay_SendHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_GivePay_SendHost() \n"));
	NHDEBUG(DBG_CALL, (L"    GivePay Step = [%d]\n", m_GivePayData.m_nTransactionStep));

	Delay_Msg(300);		// Time Delay (Make Stable for Continuous Connection)
	CString strTemp, strDestURL, strDestPort;

	// Initialize Parmeters
	m_pDevCmn->TranResult = FALSE;
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, FALSE);
	m_nSendLength = 0;												// Send Length
	memset(m_arSendBuffer, 0, NETBUF_SEND_SIZE);

	// Body
	CString	strSendBody = m_GivePayData.MakeGPRequestMessage(MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));

	if( m_GivePayData.m_nTransactionStep == GPAY_TRAN_DOWNLOAD_IMAGES )
	{
		strDestURL = strSendBody;
		strDestPort = L"443";
		strSendBody = _T("");
		int nLoc = m_GivePayData.m_sImageURL.Find(strDestURL);
		m_strSendData.Format( L"GET %s HTTP/1.1\r\n", m_GivePayData.m_sImageURL.Mid(nLoc+strDestURL.GetLength()) );									
		m_strSendData += strTemp;		// [#2574] US Justin GivePay Enhancement3 Request CLOSE after exchanging Data
	}
	else
	{
		strDestURL	= m_GivePayData.m_strHostIP;
		strDestPort = m_GivePayData.m_strHostPort;

		// Header
		if( m_GivePayData.m_nTransactionStep == GPAY_TRAN_ENROLLMENT )
		{
			m_strSendData.Format( L"POST /api/v1/devices/activate HTTP/1.1\r\n");
			m_strSendData.Append(L"Content-type: application/json\r\n");
		}
		else if( m_GivePayData.m_nTransactionStep == GPAY_TRAN_GETTOKEN )
		{
			m_strSendData.Format( L"POST /connect/token HTTP/1.1\r\n");
			m_strSendData.Append(L"Content-type: application/x-www-form-urlencoded\r\n");
		}
		else if( (m_GivePayData.m_nTransactionStep == GPAY_TRAN_DOWNLOAD_FEES) ||
				 (m_GivePayData.m_nTransactionStep == GPAY_TRAN_DOWNLOAD_CATEGORY) ||
				 (m_GivePayData.m_nTransactionStep == GPAY_TRAN_DOWNLOAD_CARDS) ||
				 (m_GivePayData.m_nTransactionStep == GPAY_TRAN_DOWNLOAD_GREETINGS) ||
				 (m_GivePayData.m_nTransactionStep == GPAY_TRAN_DOWNLOAD_WIRELESS) 	 )
		{
			m_strSendData.Format( L"GET /%s HTTP/1.1\r\n", strSendBody);
			strSendBody = _T("");	// Download Contents => Using URL / No Body
		}
		else if( m_GivePayData.m_nTransactionStep == GPAY_TRAN_PURCHASECARD_STAGE )
		{
			m_strSendData.Format( L"POST /api/v1/orders HTTP/1.1\r\n");
			m_strSendData.Append(L"Content-type: application/json\r\n");
		}
		else if( m_GivePayData.m_nTransactionStep == GPAY_TRAN_PURCHASECARD_APPROVAL )
		{
			m_strSendData.Format( L"POST /api/v1/orders/%s/pay/cc HTTP/1.1\r\n", m_GivePayData.m_strStg_OrderId);
			m_strSendData.Append(L"Content-type: application/json\r\n");
		}
		
		// Add Token
		if( (m_GivePayData.m_nTransactionStep!=GPAY_TRAN_ENROLLMENT) && (m_GivePayData.m_nTransactionStep!=GPAY_TRAN_GETTOKEN) )
		{
			m_strSendData.AppendFormat(L"Authorization: %s %s\r\n", m_GivePayData.m_strGPTokenType, m_GivePayData.m_strGPToken );
		}

		m_strSendData.AppendFormat(L"Accept: application/json\r\n");

	}

	m_strSendData.AppendFormat(L"Host: %s\r\n", strDestURL);
	m_strSendData.Append(L"Connection: close\r\n");

	if (strSendBody.GetLength() > 0)
	{
		m_strSendData.AppendFormat(L"Content-length: %d\r\n",  strSendBody.GetLength() );
	}

	m_strSendData.Append(L"\r\n");
	// Add Body
	m_strSendData += strSendBody;

	NHDEBUG(DBG_CALL, (L"  [GivePay Send Data] : Length=[%d]\n%s\n", m_strSendData.GetLength(), m_strSendData) );

	////////////////////////////////////////////
	// Set Buffer
	memset(m_szTemp, NULL, sizeof(m_szTemp));
	WideToMulti(m_szTemp, m_strSendData, sizeof(m_szTemp));

	memcpy(m_arSendBuffer, m_szTemp, m_strSendData.GetLength());
	m_nSendLength = m_strSendData.GetLength();

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Save request message to file (overwite previous transaction. Leave the latest transaction only)
	CString strLogFileName;
	strLogFileName.Format( _T("%s\\GivePay_Req%02d.dat"), ATM_DATA_PATH, m_GivePayData.m_nTransactionStep );
	NHDEBUG(DBG_CALL, (L"LOG FILE = [%s]\n", strLogFileName));
	CFile cf;
	if( cf.Open( strLogFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary ) == FALSE )
		NHDEBUG(DBG_CALL, (L"[FAIL TO OPEN(%s)]\n", strLogFileName));
	else
	{
		cf.Write(m_arSendBuffer, m_nSendLength );
		cf.Close();
	}
	// Save request message to file (overwite previous transaction. Leave the latest transaction only)
	//////////////////////////////////////////////////////////////////////////////////////////////////

	#ifdef APP_LOCAL_MODE
		Delay_Msg(1000);
		return RES_OK;
	#endif

	// IP, PORT, SSL Option, Host Cert
	CString strHostInfo;
	strHostInfo.Format(L"%s,%s,%d,0", strDestURL, strDestPort, CONN_UPTO_TLS_V12_USE_CERT);

	// Optional Terminate String (Assign at least 2 characters)
	if( m_GivePayData.m_nTransactionStep == GPAY_TRAN_GETTOKEN )
		strHostInfo += _T(",\"}");
	// [#2574] US Justin GivePay Enhancement3
	else if( m_GivePayData.m_nTransactionStep == GPAY_TRAN_DOWNLOAD_IMAGES )		
	{
		strHostInfo += _T(", ,5");				// Download Time Out : Default 5
	}
	else	
		strHostInfo += _T(",\"__abp\":true}");	
	// End of [#2574]

	NHDEBUG(DBG_CALL, (L"  [GivePay Host Information] = [%s]\n", strHostInfo) );

	if( m_pDevCmn->fnNET_SendData(m_arSendBuffer, m_nSendLength, K_300_WAIT, TRANHOST_GIVEPAY_TRANSACTION, strHostInfo) != RES_OK )
		return RES_HOST_SEND_ERR;

	return RES_OK;
}

BIZ_RETURN CTranCmn::BIZ_GivePay_RecvHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_GivePay_RecvHost()]\n"));

	Delay_Msg(300);		// Time Delay (Make Stable for Continuous Connection)
	CString strXMLString = L"";

	#ifdef APP_LOCAL_MODE
		strXMLString = m_GivePayData.MakeDemoResponseMessage();
		Delay_Msg(1000);
	#else
		m_nRecvLength = NETBUF_RECV_SIZE;
		memset(m_arRecvBuffer, 0x00, NETBUF_RECV_SIZE);

		m_pDevCmn->fnNET_RecvData( m_arRecvBuffer, &m_nRecvLength, K_180_WAIT, TRANHOST_GIVEPAY_TRANSACTION );

		if (!m_nRecvLength)
		{
			m_nRecvLength = 0;
			memset(m_arRecvBuffer, 0, NETBUF_RECV_SIZE);
			m_GivePayData.SetErrorCodeAndDesc(L"900");
			m_pDevCmn->fnAPL_StackError(m_GivePayData.m_strErrorCode, m_GivePayData.m_strErrorDesc, DEV_NET);
			NHDEBUG(DBG_INFO, (L"m_GivePayData : no data received\n"));
			return RES_HOST_RECV_ERR;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Save response message to file (overwite previous transaction. Leave the latest transaction only)
		CString strLogFileName;
		strLogFileName.Format( _T("%s\\GivePay_Res%02d.dat"), ATM_DATA_PATH, m_GivePayData.m_nTransactionStep );
		NHDEBUG(DBG_CALL, (L"LOG FILE = [%s]\n", strLogFileName));

		CFile cf;
		if( cf.Open( strLogFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary ) == FALSE )
			NHDEBUG(DBG_CALL, (L"[FAIL TO OPEN(%s)]\n", strLogFileName));
		else
		{
			cf.Write(m_arRecvBuffer, m_nRecvLength );
			cf.Close();
		}
		// Save response message to file (overwite previous transaction. Leave the latest transaction only)
		///////////////////////////////////////////////////////////////////////////////////////////////////

		strXMLString = (LPCSTR) m_arRecvBuffer;	
	#endif	

	//if( m_nRecvLength <= 1024)
	//	NHDEBUG(DBG_CALL, (L"RECEIVED DATA = \n%s\n", strXMLString ));
	//else
		NHDEBUG(DBG_CALL, (L"RECEIVED DATA, Length = [%d], First 100 bytes = \n%s\n", m_nRecvLength, strXMLString.Left(100) ));

	m_GivePayData.m_bTempDG911 = FALSE; // [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix
	if( m_GivePayData.ParseGPReceivedData(strXMLString, m_arRecvBuffer, m_nRecvLength ) != TRUE )
	{
		m_pDevCmn->fnAPL_StackError(m_GivePayData.m_strErrorCode, m_GivePayData.m_strErrorDesc, DEV_NET);
		NHDEBUG(DBG_CALL, (L"[Received Data Error : Code[%s], Desc[%s]\n", m_GivePayData.m_strErrorCode, m_GivePayData.m_strErrorDesc));
		// [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix
		if (m_GivePayData.m_strErrorCode.CompareNoCase(L"DG91100") == 0) // if DG91100 error occurs, add activating stage on GP flow
			m_GivePayData.m_bTempDG911 = TRUE; // TODO once test works fine, add method that handles all errors
		// end of [#RWC6-1, #2583]
		return RES_HOST_DENIED;
	}

	m_pDevCmn->TranResult = TRUE;
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);
	NHDEBUG(DBG_CALL, (L"GivePay host comminication OK\n"));
	return RES_OK;
}
#endif
// End of [#2513]
