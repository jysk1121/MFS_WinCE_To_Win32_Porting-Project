#include "stdafx.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"
#include "..\..\H\Tran\TranBizTR34.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------
//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

#define DBG_UNIT		NVP2_TRANCTRL
#define DBG_FUNC		"TR"

//------------------------------------------------------------------
//	Define Symbol
//------------------------------------------------------------------

//------------------------------------------------------------------
//	Global Variable
//------------------------------------------------------------------
extern CTimeCheck	g_TimeCheck;

//------------------------------------------------------------------
//	Implement
//------------------------------------------------------------------

BIZ_RETURN CTranCmn::P_BIZ_TR34_Bind_Proc(BOOL bShow)
{
	NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"Bind");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_BIZ_TR34_Bind_Proc]\n"));

	if (bShow == TRUE)
	{
		TR34_ShowProcessingScreen();
	}

	int nRes = T_ERROR;

	// check TR34 capability then execute following TR34 processes
	if (TR34_Process_Preparation() == T_OK)
	{
		if (m_pDevCmn->fnNET_RKTConnectOpen() != T_OK)
		{
			NHDEBUG(DBG_INFO, (_T("fnNET_RKTConnectOpen() Fail\n")));
			NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"CONNFAIL");
			m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Connection Failure"));
			return RES_NG;
		}

		NHDEBUG(DBG_INFO, (_T("fnNET_RKTConnectOpen() Success\n")));
		NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"CONNOK");

		nRes = TR34_Process_Bind();

		m_pDevCmn->fnNET_RKTConnectClose();
	}

	return nRes == T_OK ? RES_OK : RES_NG;
}

BIZ_RETURN CTranCmn::P_BIZ_TR34_Transport_Proc(BOOL bShow)
{
	NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"Transport");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_BIZ_TR34_Transport_Proc]\n"));

	if (bShow == TRUE)
	{
		TR34_ShowProcessingScreen();
	}

	int nRes = T_ERROR;
	bool isBound = TR34_IsEppBound();

	// check TR34 capability then execute following TR34 processes
	if (TR34_Process_Preparation() == T_OK)
	{
		if (m_pDevCmn->fnNET_RKTConnectOpen() != T_OK)
		{
			NHDEBUG(DBG_INFO, (_T("fnNET_RKTConnectOpen() Fail\n")));
			NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"CONNFAIL");
			m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Connection Failure"));
			return RES_NG;
		}

		NHDEBUG(DBG_INFO, (_T("fnNET_RKTConnectOpen() Success\n")));
		NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"CONNOK");

		if (!isBound)
		{
			nRes = TR34_Process_Bind();
		}
		else 
		{
			nRes = T_OK;
		}

		if (nRes == T_OK) 
		{
			nRes = TR34_Process_Transport();
		}

		if (nRes == T_OK)
		{
			nRes = TR34_Process_Verification();
		}

		// need to delete imported key when KCV is mismatched?
		if (TR34_curPhase == TR34::Phase::VERIFICATION && nRes != T_OK)
		{
			// Would 'CLEAR KEY' enough for this case? Do we need UNBIND as well?
			m_pDevCmn->fnPIN_ClearAllKeys();
		}

		m_pDevCmn->fnNET_RKTConnectClose();
	}

	return nRes == T_OK ? RES_OK : RES_NG;
}

BIZ_RETURN CTranCmn::P_BIZ_TR34_Unbind_Proc(BOOL bShow)
{
	NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"Unbind");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_BIZ_TR34_Unbind_Proc]\n"));

	if (bShow == TRUE)
	{
		TR34_ShowProcessingScreen();
	}

	int nRes = T_ERROR;

	if (!TR34_IsEppBound())
	{
		NHDBG((L"Skipping unbind operation because EPP is not bound\r\n"));
		return RES_OK;
	}

	// check TR34 capability then execute following TR34 processes
	if (TR34_Process_Preparation() == T_OK)
	{
		if (m_pDevCmn->fnNET_RKTConnectOpen() != T_OK)
		{
			NHDEBUG(DBG_INFO, (_T("fnNET_RKTConnectOpen() Fail\n")));
			NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"CONNFAIL");
			m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Connection Failure"));
			return RES_NG;
		}

		NHDEBUG(DBG_INFO, (_T("fnNET_RKTConnectOpen() Success\n")));
		NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"CONNOK");

		nRes = TR34_Process_Unbind();

		m_pDevCmn->fnNET_RKTConnectClose();
	}

	return nRes == T_OK ? RES_OK : RES_NG;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: TR34_Process_Preparation()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Initialize TR34 phase.
-------------------------------------------------------------------*/
int CTranCmn::TR34_InitializePhase(TR34::Phase::Value phase)
{
	NHDEBUG(DBG_INFO, (_T("Initializing Phase [%s] ... \n"), GET_PHASE_STRING(phase)));

	TR34_curPhase = phase;

	switch (phase)
	{
	case TR34::Phase::BIND:
		TR34_SetResCode(TR34::ResCode::SUCCESSFUL);	// 'Bind Request' doesn't have response code.
		TR34_BindReq.CTepp = L"";		// what would it be as default?
		break;

	case TR34::Phase::TRANSPORT:
		TR34_SetResCode(TR34::ResCode::ATM_CTEPP_ERR);
		TR34_TransportReq.RTepp = TR34_RTEPP_EMPTY;
		break;

	case TR34::Phase::VERIFICATION:
		TR34_SetResCode(TR34::ResCode::ATM_KBPK_LOAD_ERR);
		TR34_VerificationReq.KCV = TR34_KCV_EMPTY;
		break;

	case TR34::Phase::UNBIND:
		TR34_SetResCode(TR34::ResCode::ATM_CTEPP_ERR);
		TR34_UnbindReq.RTepp = TR34_RTEPP_EMPTY;
		break;

// 	case TR34::Phase::REBIND:
	default:
		TR34_SetResCode(TR34::ResCode::UNKNOWN);
		TR34_curPhase = TR34::Phase::UNINIT;
		break;
	}

	return T_OK;
}


/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: TR34_Process_Preparation()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Prepares TR34. Currently it retrieves EPP cert from EPP.
-------------------------------------------------------------------*/
int CTranCmn::TR34_Process_Preparation()
{
	// random number mode should be ASN.1 to get RTepp
	if (TR34_GetRNMode() != TR34_RNMODE_ASN1)
	{
		NHDEBUG(DBG_INFO, (_T("TR34_GetRNMode() not ASN.1 \n")));
		NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"RNMode!ASN1");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("TR34 RN mode error"));
		return T_ERROR;
	}

	// check existence of CTepp
	if (m_pDevCmn->fnPIN_TR34_GetCertificate(TR34::GetCertOpt::PublicEncKey()) != WFS_SUCCESS)
	{
		NHDEBUG(DBG_INFO, (_T("fstrPIN_TR34_GetCertificateData() Fail\n")));
		NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"CTeppFAIL");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Fail to get EPP Cert"));
		return T_ERROR;
	}

	return T_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: TR34_Process_Bind()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Performs TR34 Bind Phase.
-------------------------------------------------------------------*/
int	CTranCmn::TR34_Process_Bind()
{
	TR34_InitializePhase(TR34::Phase::BIND);

#ifdef DEBUG
/*
	// test purpose
	TR34_UnbindResp.UBThsm = L"MIIEPAYJKoZIhvcNAQcCoIIELTCCBCkCAQExDTALBglghkgBZQMEAgEwWwYJKoZIhvcNAQcBoE4ETDBKMEExCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxUUjM0IFNhbXBsZXMxGzAZBgNVBAMTElRSMzQgU2FtcGxlIENBIEtSRAIFNAAAAAehggHYMIIB1DCBvQIBATANBgkqhkiG9w0BAQsFADBBMQswCQYDVQQGEwJVUzEVMBMGA1UEChMMVFIzNCBTYW1wbGVzMRswGQYDVQQDExJUUjM0IFNhbXBsZSBDQSBLREgXDTEwMTEwMjE3MzMzMFoXDTEwMTIwMjE3MzMzMFowSDAWAgU0AAAACBcNMTAxMTAyMTcyODEzWjAWAgU0AAAAChcNMTAxMTAyMTczMTQ2WjAWAgU0AAAACxcNMTAxMTAyMTczMzI1WjANBgkqhkiG9w0BAQsFAAOCAQEANvBqPIisvPqfjjsIUO7gmpz3tbKRiG5RDTSf5fBcG9t9nznk6mUIgo8u0+55Y8hYdFJ5XDlGKwYNW5csmnte+JChk8VyJdHIjVbu0dA/fpp1hw1gTRXgEv/XuFBupLoU57UQGMFtjZ77asXFFWhrE04WsdZ/Hov0PI/JpguWFK3M6a9pwnqUU9QmNE9rFEUO5YOCFHQeq/f4fxUqkxn62e07SBoRPAM2PSmt0C4wMTopOvwYe3JSmPsUxdmXlnhaJswZzwfCvJojuPb27hmgB5BPS/Yy3P3n8oJfMS/mKOPQxxzVC7CO5ATipfARoLWrTyphJ14lAJ2uAGYO/zLWwzGCAdowggHWAgEBMEowQTELMAkGA1UEBhMCVVMxFTATBgNVBAoTDFRSMzQgU2FtcGxlczEbMBkGA1UEAxMSVFIzNCBTYW1wbGUgQ0EgS0RIAgU0AAAABjALBglghkgBZQMEAgGgZTAYBgkqhkiG9w0BCQMxCwYJKoZIhvcNAQcBMBgGCiqGSIb3DQEJGQMxCgQIfeocAIlOJGowLwYJKoZIhvcNAQkEMSIEIIeYFo5vfzEY7ehSK2M237Vs/fldtwY8tyMO8AtNZm0aMA0GCSqGSIb3DQEBAQUABIIBAE80v8n2d8D3kBFwR7HqYM/TMltuf10kfDrB8LYMqLLpJXOhQctjYBetCTDQ0kK75szZyaapV1cjmowsmfwejK6IrS1qtueiVsjFLmqROECzQiqSdSZ/iPZ82Brdkwd//jD20n2XYIpdmBUhSL7XD65DPz963KcSYARf9bPkK1wKFB9ozwsW4YeuT2Rv0QpwCBJEKspvIpKM8D8pJQHT+3cEMGurGVQtvXaG396YuOJsqg4mLN+92YRSBY61rRrlFxX4ARwtn6a9RuHW8P+dOTYkT9t0msZByYdJrk8V2oyQVtM8wqN6incGM24kRrcZvoU5lsEz9brY6Uz/wvC+JB0=";
	// give EPP freedom
	if (m_pDevCmn->fnPIN_TR34_StartAuthenticateSync(TR34::StartAuthOpt::InitializeEDMEx()) == WFS_SUCCESS)
	{
		CString strAuthDataToSign = m_pDevCmn->fstrPIN_TR34_GetAuthenticateDataToSign();
		CString strAuthSigner = m_pDevCmn->fstrPIN_TR34_GetAuthenticateSigner();

		NHDEBUG(DBG_INFO, (_T("strAuthDataToSign: (%s) \n"), strAuthDataToSign));
		NHDEBUG(DBG_INFO, (_T("strAuthSigner: (%s) \n"), strAuthSigner));

		m_pDevCmn->fnPIN_TR34_SetAuthenticateSigner(strAuthSigner);
		m_pDevCmn->fnPIN_TR34_SetAuthenticateSigKey(L"MASTERK3");
		m_pDevCmn->fnPIN_TR34_SetAuthenticateSignedData(TR34_Base64Decode(TR34_UnbindResp.UBThsm));
		m_pDevCmn->fnPIN_TR34_InitializeEDMEx(L"", L"");		// InitializeEDMEx() requires empty string ("") as its parameters
	}
*/
#endif

	// set CTepp as base64 encoded
	TR34_BindReq.CTepp = TR34_Base64Encode(m_pDevCmn->fstrPIN_TR34_GetCertificateData());
	NHDEBUG(DBG_INFO, (_T("TR34_BindReq.CTepp (%s) \n"), TR34_BindReq.CTepp));

	if (TR34_BuildMessage() != T_OK)					return T_ERROR;
	if (TR34_SendMessage() != T_OK)						return T_ERROR;
	if (TR34_RecvMessage() != T_OK)						return T_ERROR;
	if (TR34_ParseMessage() != T_OK)					return T_ERROR;
	if (TR34_ValidateMessage() != T_OK)					return T_ERROR;

	return T_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: TR34_Process_Transport()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Performs TR34 Key Transport Phase.
-------------------------------------------------------------------*/
int CTranCmn::TR34_Process_Transport()
{
	CString strRTepp;

	if (TR34_GetResCode() == TR34::ResCode::SUCCESSFUL && !TR34_BindResp.CThsm.IsEmpty())
	{
		// load CThsm into EPP
		int nReason = m_pDevCmn->fnPIN_TR34_LoadCertificateEx(L"NEWHOST", L"CA", TR34_Base64Decode(TR34_BindResp.CThsm));
		NHDEBUG(DBG_INFO, (_T("fnPIN_TR34_LoadCertificateEx(NEWHOST, CA) Result: (%d)\n"), nReason));
		if (nReason != WFS_SUCCESS)
		{
			goto transaction;
		}
	}

	TR34_InitializePhase(TR34::Phase::TRANSPORT);

	if (m_pDevCmn->fnPIN_RKT_StartKeyExchange() != T_OK)
	{
		NHDEBUG(DBG_INFO, (_T("fnPIN_RKT_StartKeyExchange() Fail \n")));
		NVDump(NVP1_ERROR, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"KeyExchERR");

		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, L"StartKeyExchange error");

		TR34_SetResCode(TR34::ResCode::ATM_KEYEXCH_ERR);

		goto transaction;
	}
	
	NHDEBUG(DBG_INFO, (_T("fnPIN_RKT_StartKeyExchange() OK \n")));
	NVDump(NVP1_ERROR, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"KeyExchOK");

	strRTepp = m_pDevCmn->fnPIN_RKT_GetEPP_RandomNumber();						// RTepp = 18 bytes header + 16 bytes RN
	if (strRTepp.GetLength() >= (TR34_RN_ASN1_HEADER_LENGTH + TR34_curRNLength) * 2)	// 34 bytes => 68 ASCII characters
	{
		NVDumpF(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"R:%s", strRTepp.Mid((TR34_RN_ASN1_HEADER_LENGTH * 2), 14));
	}
	else
	{
		TR34_SetResCode(TR34::ResCode::ATM_RTEPP_ERR);
		NVDumpF(NVP1_ERROR, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"InvldRlen:%d", strRTepp.GetLength());
		NVDumpF(NVP1_ERROR, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"R:%s", strRTepp.Left(__min(strRTepp.GetLength(), 14)));

		goto transaction;
	}

	TR34_SetResCode(TR34::ResCode::SUCCESSFUL);
	TR34_TransportReq.RTepp = TR34_Base64Encode(strRTepp);
	NHDEBUG(DBG_INFO, (_T("TR34_TransportReq.RTepp: (%s) \n"), TR34_TransportReq.RTepp));

transaction:
	if (TR34_BuildMessage() != T_OK)					return T_ERROR;
	if (TR34_SendMessage() != T_OK)						return T_ERROR;
	if (TR34_RecvMessage() != T_OK)						return T_ERROR;
	if (TR34_ParseMessage() != T_OK)					return T_ERROR;
	if (TR34_ValidateMessage() != T_OK)					return T_ERROR;

	return T_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: TR34_Process_Verification()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Performs TR34 Key Verification Phase.
-------------------------------------------------------------------*/
int CTranCmn::TR34_Process_Verification()
{
	TR34_InitializePhase(TR34::Phase::VERIFICATION);

	// Load KThsm(KBPK) into EPP
	int nReason = m_pDevCmn->fnPIN_TR34_ImportRSAEncipheredPKCS7KeyEx(TR34_Base64Decode(TR34_TransportResp.KThsm), TR34_TransportResp.KeyName, L"ANSTR31MASTER", L"RANDOM_CRL");
	NHDEBUG(DBG_INFO, (_T("fnPIN_TR34_ImportRSAEncipheredPKCS7KeyEx() Result: (%d)\n"), nReason));
	if (nReason == WFS_SUCCESS)
	{
		// KCV generated!
		TR34_SetResCode(TR34::ResCode::SUCCESSFUL);
		TR34_VerificationReq.KCV = m_pDevCmn->fnPIN_RKT_GetEPP_KCV();
		TR34_VerificationReq.KeyName = TR34_TransportResp.KeyName;
	}

	NHDEBUG(DBG_INFO, (_T("TR34_VerificationReq.KCV (%s) \n"), TR34_VerificationReq.KCV));

	if (TR34_BuildMessage() != T_OK)					return T_ERROR;
	if (TR34_SendMessage() != T_OK)						return T_ERROR;
	if (TR34_RecvMessage() != T_OK)						return T_ERROR;
	if (TR34_ParseMessage() != T_OK)					return T_ERROR;
	if (TR34_ValidateMessage() != T_OK)					return T_ERROR;

	return T_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: TR34_Process_Unbind()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Unbind EPP
-------------------------------------------------------------------*/
int CTranCmn::TR34_Process_Unbind()
{
	TR34_InitializePhase(TR34::Phase::UNBIND);

	if (m_pDevCmn->fnPIN_TR34_StartAuthenticateSync(TR34::StartAuthOpt::InitializeEDMEx()) != WFS_SUCCESS)
	{
		TR34_SetResCode(TR34::ResCode::ATM_RTEPP_ERR);
		NHDEBUG(DBG_INFO, (_T("fnPIN_TR34_StartAuthenticateSync() Fail \n")));
		NVDump(NVP1_ERROR, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"AuthError");

		return T_ERROR;
	}

	TR34_SetResCode(TR34::ResCode::SUCCESSFUL);

	CString strAuthDataToSign = m_pDevCmn->fstrPIN_TR34_GetAuthenticateDataToSign();
	CString strAuthSigner = m_pDevCmn->fstrPIN_TR34_GetAuthenticateSigner();

	TR34_UnbindReq.RTepp = TR34_Base64Encode(strAuthDataToSign);

	if (TR34_BuildMessage() != T_OK)					return T_ERROR;
	if (TR34_SendMessage() != T_OK)						return T_ERROR;
	if (TR34_RecvMessage() != T_OK)						return T_ERROR;
	if (TR34_ParseMessage() != T_OK)					return T_ERROR;
	if (TR34_ValidateMessage() != T_OK)					return T_ERROR;

	if (TR34_GetResCode() == TR34::ResCode::SUCCESSFUL)
	{
		NHDEBUG(DBG_INFO, (_T("strAuthDataToSign: (%s) \n"), strAuthDataToSign));
		NHDEBUG(DBG_INFO, (_T("strAuthSigner: (%s) \n"), strAuthSigner));

		m_pDevCmn->fnPIN_TR34_SetAuthenticateSigner(strAuthSigner);
		m_pDevCmn->fnPIN_TR34_SetAuthenticateSigKey(L"MASTERK3");
		m_pDevCmn->fnPIN_TR34_SetAuthenticateSignedData(TR34_Base64Decode(TR34_UnbindResp.UBThsm));
		m_pDevCmn->fnPIN_TR34_InitializeEDMEx(L"", L"");		// InitializeEDMEx() requires empty string ("") as its parameters
	}

	return T_OK;
}


/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: TR34_SendMessage()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Construct message for current phase.
-------------------------------------------------------------------*/
int CTranCmn::TR34_BuildMessage()
{
	m_strSendData.Empty();
	m_strSendData += TR34_HeaderReq.GetStr();

	switch (TR34_curPhase)
	{
	case TR34::Phase::BIND:				m_strSendData += TR34_BindReq.GetStr();				break;
	case TR34::Phase::TRANSPORT:		m_strSendData += TR34_TransportReq.GetStr();		break;
	case TR34::Phase::VERIFICATION:		m_strSendData += TR34_VerificationReq.GetStr();		break;
	case TR34::Phase::UNBIND:			m_strSendData += TR34_UnbindReq.GetStr();			break;
	//case TR34::Phase::REBIND:
	default:
		NHDEBUG(DBG_INFO, (_T("Message build error! (%d) \n"), TR34_curPhase));
		return T_ERROR;
	}

	return T_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: TR34_SendMessage()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Send message to HOST.
-------------------------------------------------------------------*/
int CTranCmn::TR34_SendMessage()
{
	memset(m_arSendBuffer, NULL, NETBUF_SEND_SIZE);
	WideToMulti((char*)m_arSendBuffer, m_strSendData, NETBUF_SEND_SIZE);
	m_nSendLength = m_strSendData.GetLength();

	if (m_pDevCmn->fnNET_RKTSendData(m_arSendBuffer, m_nSendLength, K_30_WAIT, Int2Asc(RKT_MODE_R0)) != T_OK)
	{
		NHDEBUG(DBG_INFO, (_T("fnNET_RKTSendData() Fail\n")));
		NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"SENDFAIL");

		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Transmission Failure"));
		return T_ERROR;
	}

	NHDEBUG(DBG_INFO, (_T("fnNET_RKTSendData() Success\n")));
	NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"SENDOK");

	return T_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: TR34_RecvMessage()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Receive message from HOST.
-------------------------------------------------------------------*/
int CTranCmn::TR34_RecvMessage()
{
	m_nRecvLength = NETBUF_RECV_SIZE;
	memset(m_arRecvBuffer, NULL, NETBUF_RECV_SIZE);

	if (m_pDevCmn->fnNET_RKTRecvData(m_arRecvBuffer, &m_nRecvLength, K_30_WAIT) != T_OK)
	{
		NHDEBUG(DBG_INFO, (_T("fnNET_RKTRecvData() Fail\n")));
		NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"RECVFAIL");

		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Transmission Failure"));
		return T_ERROR;
	}

	//m_strRecvData.Format(L"%S", m_arRecvBuffer);		// CString.Format() limits data length up to 1024, shouldn't be used here
	m_strRecvData = m_arRecvBuffer;

	NHDEBUG(DBG_INFO, (_T("fnNET_RKTRecvData (%s) \n"), m_strRecvData));
	NVDump(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"RECVOK");

	return T_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: TR34_ParseMessage()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Parse TR34 message received from HOST.
-------------------------------------------------------------------*/
int CTranCmn::TR34_ParseMessage()
{
	NHDEBUG(DBG_INFO, (_T("(%s) \n"), L""));

	TR34_HeaderResp.Clear();
	m_strArrRecvData.RemoveAll();
	SplitString(m_strRecvData, FIELD_DELIMITER, m_strArrRecvData);

	int nFieldIndex = 0;
	if(m_strArrRecvData[nFieldIndex].GetLength() >= 1)		TR34_HeaderResp.RecordFormat = m_strArrRecvData[nFieldIndex].Mid(0,1);			// "H"
	if(m_strArrRecvData[nFieldIndex].GetLength() >= 2)		TR34_HeaderResp.ApplicationType = m_strArrRecvData[nFieldIndex].Mid(1,1);		// "0"
	if(m_strArrRecvData[nFieldIndex].GetLength() >= 3)		TR34_HeaderResp.MessageDelimiter = m_strArrRecvData[nFieldIndex].Mid(2,1);		// "."
	if(m_strArrRecvData[nFieldIndex].GetLength() >= 9)		TR34_HeaderResp.BankID	= m_strArrRecvData[nFieldIndex].Mid(3,6);				// Bank ID
	nFieldIndex++;																															// FS
	if (m_strArrRecvData.GetSize() > nFieldIndex)			TR34_HeaderResp.TerminalID = m_strArrRecvData[nFieldIndex];						// Terminal ID
	nFieldIndex++;																															// FS

	switch (TR34_curPhase)
	{
	case TR34::Phase::BIND:																													// [Bind Response]
		{
			if (m_strArrRecvData.GetSize() > nFieldIndex)			TR34_BindResp.ResponseType = m_strArrRecvData[nFieldIndex++];			// Response Type // FS
			if (m_strArrRecvData.GetSize() > nFieldIndex)			TR34_BindResp.ResponseCode = m_strArrRecvData[nFieldIndex++];			// Response Code // FS
			if (m_strArrRecvData.GetSize() > nFieldIndex)			TR34_BindResp.CThsm = m_strArrRecvData[nFieldIndex++];					// CThsm
		}
		break;

	case TR34::Phase::TRANSPORT:																												// [Key Transport Response]
		{
			if (m_strArrRecvData.GetSize() > nFieldIndex)			TR34_TransportResp.ResponseType = m_strArrRecvData[nFieldIndex++];		// Response Type // FS
			if (m_strArrRecvData.GetSize() > nFieldIndex)			TR34_TransportResp.KeyName = m_strArrRecvData[nFieldIndex++];			// Key Name // FS
			if (m_strArrRecvData.GetSize() > nFieldIndex)			TR34_TransportResp.ResponseCode = m_strArrRecvData[nFieldIndex++];		// Response Code // FS
			if (m_strArrRecvData.GetSize() > nFieldIndex)			TR34_TransportResp.KThsm = m_strArrRecvData[nFieldIndex++];				// KThsm
		}
		break;

	case TR34::Phase::VERIFICATION:																											// [Key Verification Response]
		{
			if (m_strArrRecvData.GetSize() > nFieldIndex)			TR34_VerificationResp.ResponseType = m_strArrRecvData[nFieldIndex++];	// Response Type // FS
			if (m_strArrRecvData.GetSize() > nFieldIndex)			TR34_VerificationResp.ResponseCode = m_strArrRecvData[nFieldIndex++];	// Response Code // FS
		}
		break;

	case TR34::Phase::UNBIND:																													// [Unbind Response]
		{
			if (m_strArrRecvData.GetSize() > nFieldIndex)			TR34_UnbindResp.ResponseType = m_strArrRecvData[nFieldIndex++];			// Response Type // FS
			if (m_strArrRecvData.GetSize() > nFieldIndex)			TR34_UnbindResp.ResponseCode = m_strArrRecvData[nFieldIndex++];			// Response Code // FS
			if (m_strArrRecvData.GetSize() > nFieldIndex)			TR34_UnbindResp.UBThsm = m_strArrRecvData[nFieldIndex++];				// UBThsm
		}
		break;

//	not supported yet
// 	case REBIND:	break;
	default: break;
	}

	return T_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: TR34_ValidateMessage()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Validate TR34 message received from HOST.
-------------------------------------------------------------------*/
int CTranCmn::TR34_ValidateMessage()
{
	NHDEBUG(DBG_INFO, (_T("(%s) \n"), L""));

	TR34::ErrType::Value eCode = TR34::ErrType::SUCCESSFUL;

	if (TR34_HeaderReq.RecordFormat != TR34_HeaderResp.RecordFormat)						eCode = TR34::ErrType::MISMATCH_RECORDFORMAT;
	else if (TR34_HeaderReq.ApplicationType != TR34_HeaderResp.ApplicationType)				eCode = TR34::ErrType::MISMATCH_APPTYPE;
	else if (TR34_HeaderReq.BankID != TR34_HeaderResp.BankID)								eCode = TR34::ErrType::MISMATCH_BANKID;
	else if (TR34_HeaderReq.TerminalID != TR34_HeaderResp.TerminalID)						eCode = TR34::ErrType::MISMATCH_TERMINALID;

	if (eCode == TR34::ErrType::SUCCESSFUL)
	{
		switch (TR34_curPhase)
		{
		case TR34::Phase::BIND:																																// [Bind Response]
			{
				if (TR34_BindReq.RequestType != TR34_BindResp.ResponseType)					eCode = TR34::ErrType::MISMATCH_REQRESTYPE;
				else if (TR34_GetResCode() != TR34::ResCode::SUCCESSFUL)					eCode = TR34::ErrType::NOT_APPROVED_BIND;	// ignore 901 for test purpose
			}
			break;

		case TR34::Phase::TRANSPORT:																															// [Key Transport Response]
			{
				if (TR34_TransportReq.RequestType != TR34_TransportResp.ResponseType)		eCode = TR34::ErrType::MISMATCH_REQRESTYPE;
				else if (TR34_TransportResp.KeyName.IsEmpty())								eCode = TR34::ErrType::NOT_APPROVED_TRANSPORT;
				else if (TR34_GetResCode() != TR34::ResCode::SUCCESSFUL)					eCode = TR34::ErrType::NOT_APPROVED_TRANSPORT;
			}
			break;

		case TR34::Phase::VERIFICATION:																														// [Key Verification Response]
			{
				if (TR34_VerificationReq.RequestType != TR34_VerificationResp.ResponseType)	eCode = TR34::ErrType::MISMATCH_REQRESTYPE;
				else if (TR34_GetResCode() != TR34::ResCode::SUCCESSFUL)					eCode = TR34::ErrType::NOT_APPROVED_VERIFICATION;
			}
			break;

		case TR34::Phase::UNBIND:																																// [Unbind Response]
			{
				if (TR34_UnbindReq.RequestType != TR34_UnbindResp.ResponseType)				eCode = TR34::ErrType::MISMATCH_REQRESTYPE;
				else if (TR34_GetResCode() != TR34::ResCode::SUCCESSFUL)					eCode = TR34::ErrType::NOT_APPROVED_UNBIND;
			}
			break;

			//	not supported yet
			// 	case REBIND:	break;
		default:
			break;
		}
	}

	NHDEBUG(DBG_INFO, (_T("eCode set to: (%d)\n"), eCode));

	TR34_WriteJournal(eCode);
	if (eCode != TR34::ErrType::SUCCESSFUL)
	{
		NHDEBUG(DBG_INFO, (_T("Validation Error! (%d) \n"), eCode));
		NVDumpF(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"TR34VldErr:%d", eCode);

		return T_ERROR;
	}

	return T_OK;
}

int CTranCmn::TR34_GetRNMode()
{
	NHDEBUG(DBG_INFO, (_T("(%s) \n"), L""));

	int nRNOption = RegGetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"RKT_RandomNumber");
	TR34_curRNLength = RegGetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"RKT_RandomNumberLength");

	NHDEBUG(DBG_INFO, (_T("RKT_RandomNumber: (0x%x)\n"), nRNOption));
	NHDEBUG(DBG_INFO, (_T("RKT_RandomNumberLength: (0x%x)\n"), TR34_curRNLength));
	NVDumpF(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"RN_OPT:%d", nRNOption);
	NVDumpF(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"RN_LEN:[0x%x]", TR34_curRNLength);

	if ((TR34_curRNLength & TR34_RNMODE_ASN1_FLAG) == TR34_RNMODE_ASN1_FLAG)
	{
		TR34_curRNLength -= TR34_RNMODE_ASN1_FLAG;

		if (TR34_curRNLength == 0)		// it only occurs when RandomNumberLength is 0x80
		{
			NHDEBUG(DBG_INFO, (_T("RNLength Error! Adjusted as ASN.1 default \n")));
			NVDumpF(NVP1_OTHER, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"ZERO_RN_ADJ");

			TR34_curRNLength = TR34_RN_DEFAULT_LENGTH;
			RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"RKT_RandomNumberLength", TR34_RN_DEFAULT_LENGTH | 0x80);
		}

		return TR34_RNMODE_ASN1;
	}


	return TR34_RNMODE_NORMAL;
}

int CTranCmn::TR34_WriteJournal(TR34::ErrType::Value eCode)
{
	NHDEBUG(DBG_INFO, (_T("eCode: (%d) \n"), eCode));

	CString strJnlData = _T("");

	switch (eCode)
	{
	case TR34::ErrType::MISMATCH_RECORDFORMAT:		strJnlData.Format(_T("RecordFormat error"));		break;
	case TR34::ErrType::MISMATCH_APPTYPE:			strJnlData.Format(_T("AppType error"));				break;
	case TR34::ErrType::MISMATCH_BANKID:			strJnlData.Format(_T("BankID error"));				break;
	case TR34::ErrType::MISMATCH_TERMINALID:		strJnlData.Format(_T("TerminalID error"));			break;
	case TR34::ErrType::MISMATCH_REQRESTYPE:		strJnlData.Format(_T("ResponseType error"));		break;

	case TR34::ErrType::NOT_APPROVED_BIND:			strJnlData.Format(_T("ResponseCode(%s) error"), TR34_BindResp.ResponseCode);			break;
	case TR34::ErrType::NOT_APPROVED_TRANSPORT:		strJnlData.Format(_T("ResponseCode(%s) error"), TR34_TransportResp.ResponseCode);		break;
	case TR34::ErrType::NOT_APPROVED_VERIFICATION:	strJnlData.Format(_T("ResponseCode(%s) error"), TR34_VerificationResp.ResponseCode);	break;
	case TR34::ErrType::NOT_APPROVED_UNBIND:		strJnlData.Format(_T("ResponseCode(%s) error"), TR34_UnbindResp.ResponseCode);			break;

	case TR34::ErrType::SUCCESSFUL:					strJnlData.Format(_T("TR34 Phase Completed"));	break;
	default:										strJnlData.Format(_T("Unknown error (%d)"), eCode);	break;
	}

	strJnlData.Format(_T("[%s] %s"), GET_PHASE_STRING(TR34_curPhase), strJnlData);

	NHDEBUG(DBG_INFO, (_T("write as: \"%s\"\n"), strJnlData));
	m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, strJnlData);

	return T_OK;
}

CString CTranCmn::TR34_Base64Encode(CString hexData)
{
	char	szBuffer[STRBUFFSIZE]	= { 0, };
	char	szEncoded[STRBUFFSIZE]	= { 0, };
	int		nPackedSize				= 0;

	WideToMulti(szEncoded, hexData, sizeof(szEncoded));
	nPackedSize = MakePack(szEncoded, szBuffer, strlen(szEncoded));

	memset(szEncoded, 0, sizeof(szEncoded));
	base64_encode(szBuffer, nPackedSize, szEncoded, sizeof(szEncoded));
	CString strEncoded(szEncoded);

	//NHDEBUG(DBG_INFO, (_T("=> given: (%d bytes)\n(%s)\n\n"), hexData.GetLength(), hexData));
	//NHDEBUG(DBG_INFO, (_T("=> encoded as: (%d bytes)\n(%s)\n\n"), strEncoded.GetLength(), strEncoded));

	return strEncoded;
}

CString CTranCmn::TR34_Base64Decode(CString base64Data)
{
	char	szBuffer[STRBUFFSIZE]	= { 0, };
	char	szDecoded[STRBUFFSIZE]	= { 0, };
	int		nDecodeLen				= 0;

	NHDEBUG(DBG_INFO, (_T("=> given: (%d bytes)\n(%s)\n\n"), base64Data.GetLength(), base64Data));
	if (base64Data.GetLength() == 0)
	{
		NHDEBUG(DBG_INFO, (_T("=> string not found!\n\n")));
		NVDumpF(NVP1_ERROR, NVP2_TRANCTRL, NVP3_TR34, NVP4_NONE, L"B64D_EMP");
		return L"";
	}

	WideToMulti(szBuffer, base64Data, sizeof(szBuffer));
	nDecodeLen = base64_decode(szBuffer, base64Data.GetLength(), (unsigned char*)szDecoded, sizeof(szDecoded));
	CString strDecoded = MakeUnPack(szDecoded, nDecodeLen);

	//NHDEBUG(DBG_INFO, (_T("=> given: (%d bytes)\n(%s)\n\n"), base64Data.GetLength(), base64Data));
	//NHDEBUG(DBG_INFO, (_T("=> decoded as: (%d bytes)\n(%s)\n\n"), strDecoded.GetLength(), strDecoded));

	return strDecoded;
}

void CTranCmn::TR34_SetResCode(TR34::ResCode::Value resCode)
{
	CString strCode;

	if (resCode == TR34::ResCode::SUCCESSFUL)		strCode = L"000";
	else											strCode.Format(L"%d", resCode);

	switch (TR34_curPhase)
	{
	case TR34::Phase::BIND:				/*TR34_BindReq.ResponseCode = strCode;*/		break;
	case TR34::Phase::TRANSPORT:		TR34_TransportReq.ResponseCode = strCode;		break;
	case TR34::Phase::VERIFICATION:		TR34_VerificationReq.ResponseCode = strCode;	break;
	case TR34::Phase::UNBIND:			TR34_UnbindReq.ResponseCode = strCode;			break;

// 	case TR34::Phase::REBIND:
	default:																			break;
	}

	NHDEBUG(DBG_INFO, (_T("Current Phase: [%s], Set Response Code as : [%s]\n"), GET_PHASE_STRING(TR34_curPhase), strCode));
}

int CTranCmn::TR34_GetResCode()
{
	CString strCode;

	switch (TR34_curPhase)
	{
	case TR34::Phase::BIND:				strCode = TR34_BindResp.ResponseCode;			break;
	case TR34::Phase::TRANSPORT:		strCode = TR34_TransportResp.ResponseCode;		break;
	case TR34::Phase::VERIFICATION:		strCode = TR34_VerificationResp.ResponseCode;	break;
	case TR34::Phase::UNBIND:			strCode = TR34_UnbindResp.ResponseCode;			break;

// 	case TR34::Phase::REBIND:
	default:																			break;
	}

	if (strCode.IsEmpty())
		strCode.Format(_T("%d"), TR34::ResCode::UNKNOWN);

	NHDEBUG(DBG_INFO, (_T("Current Phase: [%s], Received Response Code : [%s]\n"), GET_PHASE_STRING(TR34_curPhase), strCode));
	return _ttoi(strCode);
}

void CTranCmn::TR34_ShowProcessingScreen()
{
	CString strTemp;
	NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

	///////////////////////////////////
	// DISPLAY SCREEN

	m_pDevCmn->fnSCR_DisplayPrevSet(116);

	// Setting Screen
	{
		strTemp.Format(L"%s\n", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116001));
		strTemp += m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116109);

		// Sub Title
		m_pDevCmn->fnSCR_DisplayString(3, strTemp);

		// Under Guide
		m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116002));
	}

	m_pDevCmn->fnSCR_DisplayScreen(116);
}

bool CTranCmn::TR34_IsEppBound()
{
	// check EPP bind status. we can get "PUBLICHOSTKEY" certificate when EPP is bound.
	NHDEBUG(DBG_INFO, (_T("CHECKING EPP BIND STATUS ...\n") ));
	if (m_pDevCmn->fnPIN_TR34_GetCertificate(TR34::GetCertOpt::PublicHostKey()) == WFS_SUCCESS)
	{
		CString strCThsm = m_pDevCmn->fstrPIN_TR34_GetCertificateData();
		//NHDEBUG(DBG_INFO, (_T("CThsm in EPP: (%s) \n"), strCThsm));

		if (strCThsm.GetLength() > 0)
		{
			NHDEBUG(DBG_INFO, (_T("EPP IS BOUND! \n") ));
			return true;
		}
		else
		{
			NHDEBUG(DBG_INFO, (_T("EPP IS NOT BOUND YET! \n") ));
			return false;
		}
	}
	else 
	{
		NHERROR((L"Error while attempting to get certificate. EPP is considered unbound\r\n"));
	}

	return false;
}