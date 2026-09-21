#include "stdafx.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"

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
//	Global Variable
//------------------------------------------------------------------
extern CTimeCheck	g_TimeCheck;

//------------------------------------------------------------------
//	Implement
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_BIZ_RKT_Proc()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_BIZ_RKT_Proc(int nKeyType, BOOL bShow)
{
	NVDump('O', 'C', "00", L"P_RKT", L"Start");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_BIZ_RKT_Proc]\n"));

	CString		strTemp;
	int			nEnableCount = 0;
	int			nRefreshTime = 0;

	if (bShow == TRUE)
	{
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

			{
				int	i, nIndex;
				CString strCmd, strData;

				nEnableCount = 0;

				if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER)
				{
					for (i = 0; i < 6; i++)
					{
						if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_TRAN_ADV_ENDISFLAG1+i))
							nEnableCount++;
					}

					if (nEnableCount > 0)
					{
						NH_SCR_CONFIG eFrontConfig = m_pDevCmn->m_pConfig->GetScreenConfig(SCR_FRONT);

						m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
						m_pDevCmn->fnSCR_DisplayImage(2, TRUE);
						m_pDevCmn->fnSCR_DisplayImage(3, TRUE);

						strData.Format(L"%d", nEnableCount);
						m_pDevCmn->fnSCR_SetDisplayData(L"APTranAdv", strData);

						nRefreshTime = MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_TRAN_ADV_DISP_TIME);
						strData.Format(L"%d", nRefreshTime);
						m_pDevCmn->fnSCR_SetDisplayData(L"APTranAdvTime", strData);

						nIndex = 1;
						for (i = 0; i < 6; i++)
						{
							if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_TRAN_ADV_ENDISFLAG1+i))
							{
								strCmd.Format(L"APTranAdv%d", nIndex++);
								strData.Format(L"%s\\%d_%d\\TRANADV_%02d.jpg", ADVERTISEMENT_PATH, eFrontConfig.nWidth, eFrontConfig.nHeight, i+1);
								m_pDevCmn->fnSCR_SetDisplayData(strCmd, strData);

							}
						}
					}
				}

				if (nEnableCount == 0)
				{
					m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
					m_pDevCmn->fnSCR_DisplayImage(2, FALSE);
					m_pDevCmn->fnSCR_DisplayImage(3, FALSE);
				}
			}
		}

		m_pDevCmn->fnSCR_DisplayScreen(116);
	}

	if (RKT_Process_R0() != T_OK)
		return RES_NG;

	// [#2174] NH KSK 2013.01.17 RKT 성공/실패와 상관없이 종료시 KEKTYPE을 Default로 설정
//	if (RKT_Process_R1(nKeyType) != T_OK)
//		return RES_NG;
//	if (RKT_Process_R2() != T_OK)
//		return RES_NG;

	if (RKT_Process_R1(nKeyType) != T_OK)
	{
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"KEKType", 1);
		return RES_NG;
	}

	if (RKT_Process_R2() != T_OK)
	{
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"KEKType", 1);
		return RES_NG;
	}

	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"KEKType", 1);
	// end of [#2174]

	return RES_OK;
}

int	CTranCmn::RKT_Process_R0()
{
	CString	strtemp;
	CString strTemp2;
	CString HSM_PublicKey;
	CString HSM_SignedValue;
	char	szBuffer[1024]	=	{ 0, };
	char	szEnDecoded[1024] =	{ 0, };
	int		nPackedSize		= 0;
	int		nRecvIndex		= 0;
	int		nDecodeLen		= 0;

	if (m_pDevCmn->fnPIN_RKT_ExportRSAIssuerSignedItem(L"EPPID", L"") != T_OK)
	{
		NVDump('O', 'C', "76", L"", L"EPPIDFAIL");
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R0*** fnPIN_RKT_ExportRSAIssuerSignedItem(EPP_ID) Fail\n")));
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Fail to EPP ID"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		return T_ERROR;
	}

	// Get EPP Serial Number
	strtemp = m_pDevCmn->fnPIN_RKT_GetExportedKey();
	NHDEBUG(1, (_T("***RKT_Process_R0*** fnPIN_RKT_GetExportedKey() Before EPP_SK(%s)\n"), strtemp));
	MakePack(strtemp, RKT_AuthReq.EPP_SerialNumber);

	strtemp.Empty();
	memset(m_szTemp, 0, sizeof(m_szTemp));
	memset(szBuffer, 0, sizeof(szBuffer));
	memset(szEnDecoded, 0, sizeof(szEnDecoded));

	// Get EPP Serial Number SignedValue
	strtemp = m_pDevCmn->fnPIN_RKT_GetExportedSignedValue();
	NHDEBUG(1, (_T("***RKT_Process_R0*** fnPIN_RKT_GetExportedSignedValue() Before EPP_Sign_SK(%s)\n"), strtemp));
	WideToMulti(m_szTemp, strtemp, sizeof(m_szTemp));
	nPackedSize = MakePack(m_szTemp, szBuffer, strlen(m_szTemp));
	base64_encode(szBuffer, nPackedSize, szEnDecoded, sizeof(szEnDecoded));		// encoded base64

	RKT_AuthReq.EPP_Sign_SK.Format(L"%S", szEnDecoded);
	NHDEBUG(1, (_T("***RKT_Process_R0*** fnPIN_RKT_GetExportedSignedValue() After EPP_Sign_SK(%s)\n"), RKT_AuthReq.EPP_Sign_SK));

	strtemp.Empty();
	memset(m_szTemp, 0, sizeof(m_szTemp));
	memset(szBuffer, 0, sizeof(szBuffer));
	memset(szEnDecoded, 0, sizeof(szEnDecoded));

	if (m_pDevCmn->fnPIN_RKT_ExportRSAIssuerSignedItem(L"PUBLICKEY", L"") != T_OK)
	{
		NVDump('F', 'C', "76", L"", L"EPPPKFAIL");
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R0*** fnPIN_RKT_ExportRSAIssuerSignedItem(EPP_PUBLICKEY) Fail\n")));
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Fail to EPP PUBLICKEY"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		return T_ERROR;
	}

	// Get EPP PublicKey (SP에서 270Byte가 올라옴)
	strtemp = m_pDevCmn->fnPIN_RKT_GetExportedKey();
	NHDEBUG(1, (_T("***RKT_Process_R0*** fnPIN_RKT_GetExportedKey() Before EPP_PublicKey(%s)\n"), strtemp));
	WideToMulti(m_szTemp, strtemp, sizeof(m_szTemp));
	nPackedSize = MakePack(m_szTemp, szBuffer, strlen(m_szTemp));

	strTemp2.Format(L"SIZE1:%d", nPackedSize);
	NVDump('O', 'C', "76", L"", strTemp2);

	if (nPackedSize != 270)
	{
		NVDump('O', 'C', "76", L"", L"EPPPKSIZEERR");
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R0*** fnPIN_RKT_GetExportedKey() EPP Size Error(%d)\n"), nPackedSize));
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("SIZE ERROR(EPP PUBLICKEY)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		return T_ERROR;
	}

	// 256 Byte만 송신하도록 수정
	base64_encode(&szBuffer[9], nPackedSize-14, szEnDecoded, sizeof(szEnDecoded));  // encoded base64
	RKT_AuthReq.EPP_PublicKey.Format(L"%S", szEnDecoded);
	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R0*** fnPIN_RKT_GetExportedKey() After EPP_PublicKey(%s)\n"), RKT_AuthReq.EPP_PublicKey));

	strtemp.Empty();
	memset(m_szTemp, 0, sizeof(m_szTemp));
	memset(szBuffer, 0, sizeof(szBuffer));
	memset(szEnDecoded, 0, sizeof(szEnDecoded));

	strtemp = m_pDevCmn->fnPIN_RKT_GetExportedSignedValue();
	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R0*** fnPIN_RKT_GetExportedSignedValue() Before EPP_Sign_PK(%s)\n"), strtemp));
	WideToMulti(m_szTemp, strtemp, sizeof(m_szTemp));
	nPackedSize = MakePack(m_szTemp, szBuffer, strlen(m_szTemp));
	
	strTemp2.Format(L"SIZE2:%d", nPackedSize);
	NVDump('O', 'C', "76", L"", strTemp2);

	base64_encode(szBuffer, nPackedSize, szEnDecoded, sizeof(szEnDecoded));		// encoded base64

	RKT_AuthReq.EPP_Sign_PK.Format(L"%S", szEnDecoded);
	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R0*** fnPIN_RKT_GetExportedSignedValue() After EPP_Sign_PK(%s)\n"), RKT_AuthReq.EPP_Sign_PK));

	strTemp2.Format(L"SIZE3:%d", RKT_AuthReq.EPP_Sign_PK.GetLength());
	NVDump('O', 'C', "76", L"", strTemp2);

	// Send Host Make
	RKT_AuthReq.RecordFormat = L"H";
	RKT_AuthReq.ApplicationType = L"0";
	RKT_AuthReq.MessageDelimiter = L".";

	m_strSendData = L"H0.";

	RKT_AuthReq.BankID.Format(L"%-6.6s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));
	m_strSendData += RKT_AuthReq.BankID;

	RKT_AuthReq.TerminalID.Format(L"%-8.8s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID));
	m_strSendData += FIELD_DELIMITER + RKT_AuthReq.TerminalID;

	RKT_AuthReq.RequestType = L"R0";
	m_strSendData += FIELD_DELIMITER + RKT_AuthReq.RequestType;

	m_strSendData += FIELD_DELIMITER + RKT_AuthReq.EPP_SerialNumber;
	m_strSendData += FIELD_DELIMITER + RKT_AuthReq.EPP_Sign_SK;

	m_strSendData += FIELD_DELIMITER + RKT_AuthReq.EPP_PublicKey;
	m_strSendData += FIELD_DELIMITER + RKT_AuthReq.EPP_Sign_PK;

	////////////////////////////////////////////
	// 3. Make Buffer
	////////////////////////////////////////////
	// Initialize Send Buffer
	m_nSendLength = 0;												// Send Length
	//memset(m_arSendBuffer, 0, sizeof(m_arSendBuffer));			// Send Buffer
	memset(m_arSendBuffer, 0, NETBUF_SEND_SIZE);					// Send Buffer [###2]

	memset(m_szTemp, NULL, sizeof(m_szTemp));
	WideToMulti(m_szTemp, m_strSendData, sizeof(m_szTemp));

	memcpy(m_arSendBuffer, m_szTemp, m_strSendData.GetLength());
	m_nSendLength = m_strSendData.GetLength();

	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R0*** fnNET_RKTConnectOpen() First (%s)\n"), m_strSendData));
	if (m_pDevCmn->fnNET_RKTConnectOpen() != T_OK)
	{
		NVDump('O', 'C', "76", L"", L"CONNECTFAIL(R0)");
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R0*** fnNET_RKTConnectOpen() Fail\n")));
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Connection Failure (R0)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		return T_ERROR;
	}
	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R0*** fnNET_RKTConnectOpen() Success\n")));

	if (m_pDevCmn->fnNET_RKTSendData(m_arSendBuffer, m_nSendLength, K_30_WAIT, Int2Asc(RKT_MODE_R0)) != T_OK)
	{
		NVDump('O', 'C', "76", L"", L"SENDFAIL(R0)");
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R0*** fnNET_RKTSendData() Fail\n")));
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Transmission Failure (R0)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}

	NVDump('O', 'C', "76", L"", L"SENDOK(R0)");
	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R0*** fnNET_RKTSendData() Success\n")));

	////////////////////////////////////////////
	// Initialize Receive Buffer
	//m_nRecvLength = NETBUF_SEND_SIZE;
	//memset(m_arRecvBuffer, 0, NETBUF_SEND_SIZE);
	m_nRecvLength = NETBUF_RECV_SIZE;		// [###2]
	memset(m_arRecvBuffer, 0, NETBUF_RECV_SIZE);

	m_pDevCmn->fnNET_RKTRecvData(m_arRecvBuffer, &m_nRecvLength, K_30_WAIT);
	// copy to receive data
	m_strRecvData.Format(L"%S", m_arRecvBuffer);
	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R0*** fnNET_RKTReceive Data(%s) \n"), m_strRecvData));

	m_strArrRecvData.RemoveAll();
	nRecvIndex = 0;

	SplitString(m_strRecvData, FIELD_DELIMITER, m_strArrRecvData);

	if(m_strArrRecvData[nRecvIndex].GetLength() >= 1)
		RKT_AuthResp.RecordFormat = m_strArrRecvData[nRecvIndex].Mid(0,1);

	if(m_strArrRecvData[nRecvIndex].GetLength() >= 2)
		RKT_AuthResp.ApplicationType = m_strArrRecvData[nRecvIndex].Mid(1,1);

	if(m_strArrRecvData[nRecvIndex].GetLength() >= 3)
		RKT_AuthResp.MessageDelimiter = m_strArrRecvData[nRecvIndex].Mid(2,1);

	if(m_strArrRecvData[nRecvIndex].GetLength() >= 9)
		RKT_AuthResp.BankID	= m_strArrRecvData[nRecvIndex].Mid(3,6);

	nRecvIndex++;

	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_AuthResp.TerminalID = m_strArrRecvData[nRecvIndex++];
	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_AuthResp.ResponseType = m_strArrRecvData[nRecvIndex++];
	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_AuthResp.ResponseCode = m_strArrRecvData[nRecvIndex++];
	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_AuthResp.HSM_PublicKey = m_strArrRecvData[nRecvIndex++];
	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_AuthResp.HSM_Sign_PK = m_strArrRecvData[nRecvIndex];

	// Check Validation Recv Data
	if (RKT_AuthReq.RecordFormat != RKT_AuthResp.RecordFormat)
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R0) Format Error(1)\n")));
		NVDump('O', 'C', "76", L"", L"RDFORMATERR(R0)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("RecordFormat error (R0)"));
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}
	else if (RKT_AuthReq.ApplicationType != RKT_AuthResp.ApplicationType)
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R0) Format Error(2)\n")));
		NVDump('O', 'C', "76", L"", L"APPTYPEERR(R0)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("App Type error (R0)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}
	else if (RKT_AuthReq.BankID != RKT_AuthResp.BankID)
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R0) Format Error(3)\n")));
		NVDump('O', 'C', "76", L"", L"BANKIDERR(R0)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("BankID error (R0)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}
	else if (RKT_AuthReq.TerminalID != RKT_AuthResp.TerminalID)
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R0) Format Error(4)\n")));
		NVDump('O', 'C', "76", L"", L"TMLIDERR(R0)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("TerminalID error (R0)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}
	else if (RKT_AuthReq.RequestType != RKT_AuthResp.ResponseType)
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R0) Format Error(5)\n")));
		NVDump('O', 'C', "76", L"", L"RSPTYPEERR(R0)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Response Type error (R0)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}
	else if (RKT_AuthResp.ResponseCode != L"000")
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R0) Format Error(6)\n")));
		NVDump('O', 'C', "76", L"", L"RSPCODEERR(R0)");
		CString JnlData;
		JnlData.Format(L"ResponseCode(%s) error (R0)", RKT_AuthResp.ResponseCode);
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, JnlData);	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}

	return T_OK;
}

int	CTranCmn::RKT_Process_R1(int nRKTType)
{
	CString	strtemp;
	CString strTemp2;
	CString HSM_PublicKey;
	CString HSM_SignedValue;
	char	szBuffer[4096]	=	{ 0, };	// KSK 2012.03.01 Code Sonar 지적사항 대책
	char	szEnDecoded[4096] =	{ 0, };	// KSK 2012.03.01 Code Sonar 지적사항 대책
	int		nPackedSize		= 0;
	int		nRecvIndex		= 0;
	int		nDecodeLen		= 0;

	strtemp.Empty();
	memset(m_szTemp, 0, sizeof(m_szTemp));
	memset(szBuffer, 0, sizeof(szBuffer));
	memset(szEnDecoded, 0, sizeof(szEnDecoded));

	strtemp = RKT_AuthResp.HSM_PublicKey;
	WideToMulti(m_szTemp, strtemp, sizeof(m_szTemp));
	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** First before HSM_PublicKey(%s)\n"), strtemp));
	nDecodeLen = base64_decode(m_szTemp, strlen(m_szTemp), (unsigned char*)szEnDecoded, sizeof(szEnDecoded));		// decoded base64

	strTemp2.Format(L"SIZE4:%d", nDecodeLen);
	NVDump('O', 'C', "76", L"", strTemp2);

	HSM_PublicKey.Format(L"%s%s%s", PUBLICKEY_2048_BIT_MODULUS, MakeUnPack(szEnDecoded, nDecodeLen), PUBLICKEY_2048_BIT_EXPONENT);

	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** First real after HSM_PublicKey(%s)\n"), HSM_PublicKey));

	strtemp.Empty();
	memset(m_szTemp, 0, sizeof(m_szTemp));
	memset(szBuffer, 0, sizeof(szBuffer));
	memset(szEnDecoded, 0, sizeof(szEnDecoded));

	strtemp = RKT_AuthResp.HSM_Sign_PK;
	WideToMulti(m_szTemp, strtemp, sizeof(m_szTemp));
	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** First Before HSM_Sign_PK(%s)\n"), strtemp));
	nDecodeLen = base64_decode(m_szTemp, strlen(m_szTemp), (unsigned char*)szEnDecoded, sizeof(szEnDecoded));		// decoded base64

	strTemp2.Format(L"SIZE5:%d", nDecodeLen);
	NVDump('O', 'C', "76", L"", strTemp2);

	HSM_SignedValue = MakeUnPack(szEnDecoded, nDecodeLen);

	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** First After HSM_Sign_PK(%s)\n"), HSM_SignedValue));
	NVDump('O', 'C', "76", L"", L"RECVOK(R0)");

	// ImportHSMPublicKey
	if (m_pDevCmn->fnPIN_RKT_ImportRSAPublicKey(HSM_PublicKey, HSM_SignedValue) == T_OK)
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnPIN_RKT_ImportRSAPublicKey() OK \n")));
		NVDump('O', 'C', "76", L"", L"HOSTPKOK");

		// Start Exchange
		if (m_pDevCmn->fnPIN_RKT_StartKeyExchange() == T_OK)
		{
			NVDump('O', 'C', "76", L"", L"STRKEYOK");
			NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnPIN_RKT_StartKeyExchange() OK \n")));

			// Set Response Code
			RKT_TMKReq.ResponseCode = L"000";

			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_RKT_RANDOMNUMBER_ENABLE) == ENABLE)	// 사양변경에 따라 Random Option2 제거
			{
				// Get RSACrypto
				strtemp.Empty();
				memset(m_szTemp, 0, sizeof(m_szTemp));
				memset(szBuffer, 0, sizeof(szBuffer));
				memset(szEnDecoded, 0, sizeof(szEnDecoded));

				strtemp = m_pDevCmn->fnPIN_RKT_GetEPP_RandomNumber();		// [#2259] NH KSK 2014.03.13
				strTemp2.Format(L"R:%s", strtemp.Left(4));
				NVDump('O', 'C', "76", L"", strTemp2);
				strTemp2.Format(L"RANDOM:%d", RegGetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"RKT_RandomNumber"));
				NVDump('O', 'C', "76", L"", strTemp2);
				WideToMulti(m_szTemp, strtemp, sizeof(m_szTemp));
				NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnPIN_RKT_GetEPP_RandomNumber() before (%s) \n"), strtemp));
				nPackedSize = MakePack(m_szTemp, szBuffer, strlen(m_szTemp));
				base64_encode(szBuffer, nPackedSize, szEnDecoded, sizeof(szEnDecoded));		// encoded base64

				RKT_TMKReq.EPP_RSACrypt_Repp.Format(L"%S", szEnDecoded);
			}
		}
		else
		{
			NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnPIN_RKT_StartKeyExchange() Fail \n")));
			NVDump('O', 'C', "76", L"", L"STRKEYERR");
			m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, L"StartExchange error (R1)");	// 2009.10.29 RKT를 위해 US Version에 OA 추가			
			// Set Response Code
			RKT_TMKReq.ResponseCode = L"801";
		}
	}
	else
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnPIN_RKT_ImportRSAPublicKey() Fail \n")));
		NVDump('O', 'C', "76", L"", L"HOSTPKERR");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, L"ImportPublicKey error (R1)");	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		// Set Response Code
		RKT_TMKReq.ResponseCode = L"806";	// 추후 상세 분류 필요
	}

	////////////////////////////////////////////
	// 3. Make Buffer
	////////////////////////////////////////////
	// Initialize Send Buffer
	m_nSendLength = 0;												// Send Length
	//memset(m_arSendBuffer, 0, sizeof(m_arSendBuffer));				// Send Buffer
	memset(m_arSendBuffer, 0, NETBUF_SEND_SIZE);					// Send Buffer [###2]

	RKT_TMKReq.RecordFormat = L"H";
	RKT_TMKReq.ApplicationType = L"0";
	RKT_TMKReq.MessageDelimiter = L".";
	m_strSendData = L"H0.";

	RKT_TMKReq.BankID.Format(L"%-6.6s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));
	m_strSendData += RKT_TMKReq.BankID;

	RKT_TMKReq.TerminalID.Format(L"%-8.8s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID));
	m_strSendData += FIELD_DELIMITER + RKT_TMKReq.TerminalID;

	RKT_TMKReq.RequestType = L"R1";
	m_strSendData += FIELD_DELIMITER + RKT_TMKReq.RequestType;

	// [#2174] NH KSK 2013.01.17 MASTER KEY / MAC MASTER KEY 주입시 Registry값 변경 필요
//	if (nRKTType == TC_EXTENDED_RKT_TMK)		// Terminal Master Key
//		RKT_TMKReq.KeyType = RKT_TERMINAL_MASTER_KEY;
//	else										// Mac Master Key
//		RKT_TMKReq.KeyType = RKT_MAC_MASTER_KEY;
	if (nRKTType == TC_EXTENDED_RKT_TMK)		// Terminal Master Key
	{
		RKT_TMKReq.KeyType = RKT_TERMINAL_MASTER_KEY;
		// [#2256] NH KSK 2014.02.28 T-DES, T-MAC에서 Option에 따라 KEKType 변경 필요
//		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"KEKType", 1);		
		if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)
		{
			if (RegGetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"TMACOption") == 0)
			{
				// 하나의 MasterKey를 PIN / MAC 동시 사용할 경우
				RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"KEKType", 3);
			}
			else
			{
				// 하나의 MasterKey를 PIN용으로만 사용할 경우
				RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"KEKType", 1);
			}
		}
		else
		{
			// 하나의 MasterKey를 PIN용으로만 사용할 경우
			RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"KEKType", 1);
		}
		// end of [#2256]
	}
	else										// Mac Master Key
	{
		RKT_TMKReq.KeyType = RKT_MAC_MASTER_KEY;
		// 하나의 MasterKey를 MAC용으로만 사용할 경우
		RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"KEKType", 7);
	}
	// end of [#2174]

	m_strSendData += FIELD_DELIMITER + RKT_TMKReq.ResponseCode;

	if (RKT_TMKReq.ResponseCode == L"000")
	{
		m_strSendData += FIELD_DELIMITER + RKT_TMKReq.KeyType;

		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_RKT_RANDOMNUMBER_ENABLE) == ENABLE)
			m_strSendData += FIELD_DELIMITER + RKT_TMKReq.EPP_RSACrypt_Repp;
	}

	// copy to send buffer
	sprintf((char*)m_arSendBuffer, "%S", m_strSendData);
	m_nSendLength = m_strSendData.GetLength();

	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnNET_RKTSendData(R1) second (%s)\n"), m_strSendData));
	if (m_pDevCmn->fnNET_RKTSendData(m_arSendBuffer, m_nSendLength, K_30_WAIT, Int2Asc(RKT_MODE_R1)) != T_OK)
	{
		NHDEBUG(1, (_T("***RKT_Process_R1*** fnNET_RKTSendData() Fail\n")));
		NVDump('O', 'C', "76", L"", L"SENDFAIL(R1)");

		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Transmission Failure (R1)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}
	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnNET_RKTSendData(R1) Success\n")));
	NVDump('O', 'C', "76", L"", L"SENDOK(R1)");

	////////////////////////////////////////////
	// Initialize Receive Buffer
	m_nRecvLength = NETBUF_RECV_SIZE;		// [###2]
	memset(m_arRecvBuffer, 0, NETBUF_RECV_SIZE);

	m_pDevCmn->fnNET_RKTRecvData(m_arRecvBuffer, &m_nRecvLength, K_30_WAIT);

	// copy to receive data
	m_strRecvData.Format(L"%S", m_arRecvBuffer);
	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R1) second (%s)\n"), m_strRecvData));

	m_strArrRecvData.RemoveAll();
	nRecvIndex = 0;

	SplitString(m_strRecvData, FIELD_DELIMITER, m_strArrRecvData);

	if(m_strArrRecvData[nRecvIndex].GetLength() >= 1)
		RKT_TMKResp.RecordFormat		= m_strArrRecvData[nRecvIndex].Mid(0,1);

	if(m_strArrRecvData[nRecvIndex].GetLength() >= 2)
		RKT_TMKResp.ApplicationType		= m_strArrRecvData[nRecvIndex].Mid(1,1);

	if(m_strArrRecvData[nRecvIndex].GetLength() >= 3)
		RKT_TMKResp.MessageDelimiter	= m_strArrRecvData[nRecvIndex].Mid(2,1);

	if(m_strArrRecvData[nRecvIndex].GetLength() >= 9)
		RKT_TMKResp.BankID	= m_strArrRecvData[nRecvIndex].Mid(3,6);

	nRecvIndex++;

	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_TMKResp.TerminalID = m_strArrRecvData[nRecvIndex++];
	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_TMKResp.ResponseType = m_strArrRecvData[nRecvIndex++];
	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_TMKResp.ResponseCode = m_strArrRecvData[nRecvIndex++];
	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_TMKResp.KeyType = m_strArrRecvData[nRecvIndex++];
	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_TMKResp.HSM_RSACrypt_TMK = m_strArrRecvData[nRecvIndex++];
	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_TMKResp.HSM_Sign_TMK = m_strArrRecvData[nRecvIndex++];
	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_TMKResp.HSM_KCV = m_strArrRecvData[nRecvIndex++];

	// Check Validation Recv Data
	if (RKT_TMKReq.RecordFormat != RKT_TMKResp.RecordFormat)
	{
		NHDEBUG(1, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R1) Format Error(1)\n")));
		NVDump('O', 'C', "76", L"", L"RDFORMATERR(R1)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("RecordFormat error (R1)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}
	else if (RKT_TMKReq.ApplicationType != RKT_TMKResp.ApplicationType)
	{
		NHDEBUG(1, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R1) Format Error(2)\n")));
		NVDump('O', 'C', "76", L"", L"APPTYPEERR(R1)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("App Type error (R1)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}
	else if (RKT_TMKReq.BankID != RKT_TMKResp.BankID)
	{
		NHDEBUG(1, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R1) Format Error(3)\n")));
		NVDump('O', 'C', "76", L"", L"BANKIDERR(R1)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("BankID error (R1)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}
	else if (RKT_TMKReq.TerminalID != RKT_TMKResp.TerminalID)
	{
		NHDEBUG(1, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R1) Format Error(4)\n")));
		NVDump('O', 'C', "76", L"", L"TMLIDERR(R1)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("TerminalID error (R1)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}
	else if (RKT_TMKReq.RequestType != RKT_TMKResp.ResponseType)
	{
		NHDEBUG(1, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R1) Format Error(5)\n")));
		NVDump('O', 'C', "76", L"", L"RSPTYPEERR(R1)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Response Type error (R1)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}
	else if (RKT_TMKResp.ResponseCode != L"000")
	{
		NHDEBUG(1, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R1) Format Error(6)\n")));
		NVDump('O', 'C', "76", L"", L"RSPCODEERR(R1)");
		CString JnlData;
		JnlData.Format(L"ResponseCode(%s) error (R1)", RKT_TMKResp.ResponseCode);
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, JnlData);	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}
	else if (RKT_TMKReq.KeyType != RKT_TMKResp.KeyType)
	{
		NHDEBUG(1, (_T("***RKT_Process_R1*** fnNET_RKTRecvData(R1) Format Error(7)\n")));
		NVDump('O', 'C', "76", L"", L"KEYTYPEERR(R1)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Key Type error (R1)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}

	NVDump('O', 'C', "76", L"", L"RECVOK(R1)");

	return T_OK;
}

int	CTranCmn::RKT_Process_R2()
{
	CString	strtemp;
	CString strTemp2;
	CString HSM_RSARandom;
	CString RecvedTMK;
	CString RecvedSignature;
	char	szBuffer[4096]	=	{ 0, };	// KSK 2012.03.01 Code Sonar 지적사항 대책
	char	szEnDecoded[4096] =	{ 0, };	// KSK 2012.03.01 Code Sonar 지적사항 대책
	int		nPackedSize		= 0;
	int		nRecvIndex		= 0;
	int		nDecodeLen		= 0;
	int		nKeyType = 0;

	strtemp.Empty();
	memset(m_szTemp, 0, sizeof(m_szTemp));
	memset(szBuffer, 0, sizeof(szBuffer));
	memset(szEnDecoded, 0, sizeof(szEnDecoded));

	strtemp = RKT_TMKResp.HSM_RSACrypt_TMK;
	WideToMulti(m_szTemp, strtemp, sizeof(m_szTemp));
	nDecodeLen = base64_decode(m_szTemp, strlen(m_szTemp), (unsigned char*)szEnDecoded, sizeof(szEnDecoded));		// decoded base64

	strTemp2.Format(L"SIZE6:%d", nDecodeLen);
	NVDump('O', 'C', "76", L"", strTemp2);

	RecvedTMK = MakeUnPack(szEnDecoded, nDecodeLen);

	strtemp.Empty();
	memset(m_szTemp, 0, sizeof(m_szTemp));
	memset(szBuffer, 0, sizeof(szBuffer));
	memset(szEnDecoded, 0, sizeof(szEnDecoded));

	strtemp = RKT_TMKResp.HSM_Sign_TMK;
	WideToMulti(m_szTemp, strtemp, sizeof(m_szTemp));
	nDecodeLen = base64_decode(m_szTemp, strlen(m_szTemp), (unsigned char*)szEnDecoded, sizeof(szEnDecoded));		// decoded base64

	strTemp2.Format(L"SIZE7:%d", nDecodeLen);
	NVDump('O', 'C', "76", L"", strTemp2);

	RecvedSignature = MakeUnPack(szEnDecoded, nDecodeLen);

	// Load TMK
	if (RKT_TMKResp.KeyType == RKT_TERMINAL_MASTER_KEY || RKT_TMKResp.KeyType == RKT_MAC_MASTER_KEY)
	{
		if (RKT_TMKResp.KeyType == RKT_TERMINAL_MASTER_KEY)	
		{
			nKeyType = RKT_TRIPLE_TMK;
		}
		else if (RKT_TMKResp.KeyType == RKT_MAC_MASTER_KEY)
		{
			if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING)
				nKeyType = RKT_SINGLE_MMK;
			else
				nKeyType = RKT_TRIPLE_MMK;
		}

		strTemp2.Format(L"RANDOM1:%d", RegGetInt(L"SOFTWARE\\ATM\\DevInfo\\PINPAD", L"RKT_RandomNumber"));
		NVDump('O', 'C', "76", L"", strTemp2);

		int nReturn = m_pDevCmn->fnPIN_RKT_ImportRSASignedDESKey(nKeyType, HSM_RSARandom, RecvedTMK, RecvedSignature);
		if (nReturn == T_OK)
		{
			NVDump('O', 'C', "76", L"", L"TMKLOADOK");
			NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnPIN_RKT_ImportRSASignedDESKey() OK HSM_RSARandom(%s) RecvedTMK(%s), RecvedSignature(%s)\n"), HSM_RSARandom, RecvedTMK, RecvedSignature));

			RKT_TMKResultReq.EPP_KCV = m_pDevCmn->fnPIN_RKT_GetEPP_KCV().Left(6);	// KCV Value 사양서 변경 (16BYTE -> 6BYTE)

			// V99.04.08 ACI(Solution 업체)에서 KCV값을 올려주지 못하므로 SPACE인 경우 정상 처리하도록 수정
			if ((RKT_TMKResp.HSM_KCV.Left(6) == RKT_TMKResultReq.EPP_KCV.Left(6)) || (RKT_TMKResp.HSM_KCV.Left(6) == L"      "))
			{
				NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnPIN_RKT_ImportRSASignedDESKey() KCV OK (%s)\n"), RKT_TMKResultReq.EPP_KCV));
				NVDump('O', 'C', "76", L"", L"KCVOK");	
				RKT_TMKResultReq.ResponseCode = L"000";
			}
			else
			{
				NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnPIN_RKT_ImportRSASignedDESKey() KCV ERROR HOST (%s) EPP (%s)\n"), RKT_TMKResp.HSM_KCV, RKT_TMKResultReq.EPP_KCV));
				NVDump('O', 'C', "76", L"", L"KCVERR");	

				CString JnlData;
				JnlData.Format(L"TMK KCV Failed A(%6.6s) H(%6.6s)", RKT_TMKResultReq.EPP_KCV, RKT_TMKResp.HSM_KCV);
				m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, JnlData);	// 2009.10.29 RKT를 위해 US Version에 OA 추가
				RKT_TMKResultReq.ResponseCode = L"811";	// KCV Failed
			}
		}
		else
		{
			NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnPIN_RKT_ImportRSASignedDESKey() ERROR\n")));
			NVDump('O', 'C', "76", L"", L"TMKLOADFAIL");
			m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Fail to TMK Loading (R1)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가

			// Set Response Code
			RKT_TMKResultReq.ResponseCode = L"812";	// TMK Loading Fail
		}
	}
	else
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnPIN_RKT_ImportRSASignedDESKey() KEY TYPE ERROR\n")));
		NVDump('O', 'C', "76", L"", L"TMKTYPEERR(R1)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Key Mode not supported (R1)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가		
		// Set Response Code
		RKT_TMKResultReq.ResponseCode = L"809";
	}

	/////////////////////////////////////////////////////////////////////////////
	//	Send Host Variable Initialize
	/////////////////////////////////////////////////////////////////////////////
	m_nSendLength = 0;												// Send Length
	//memset(m_arSendBuffer, 0, sizeof(m_arSendBuffer));			// Send Buffer
	memset(m_arSendBuffer, 0, NETBUF_SEND_SIZE);					// Send Buffer [###2]

	// Send Data
	RKT_TMKResultReq.RecordFormat = L"H";
	RKT_TMKResultReq.ApplicationType = L"0";
	RKT_TMKResultReq.MessageDelimiter = L".";
	m_strSendData = L"H0.";

	RKT_TMKResultReq.BankID.Format(L"%-6.6s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));
	m_strSendData += RKT_TMKResultReq.BankID;

	RKT_TMKResultReq.TerminalID.Format(L"%-8.8s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID));
	m_strSendData += FIELD_DELIMITER + RKT_TMKResultReq.TerminalID;

	RKT_TMKResultReq.RequestType = L"R2";
	m_strSendData += FIELD_DELIMITER + RKT_TMKResultReq.RequestType;

	m_strSendData += FIELD_DELIMITER + RKT_TMKResultReq.ResponseCode;
	m_strSendData += FIELD_DELIMITER + RKT_TMKResultReq.EPP_KCV;

	// copy to send buffer
	sprintf((char*)m_arSendBuffer, "%S", m_strSendData);
	m_nSendLength = m_strSendData.GetLength();

	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnNET_SendvData(R2) (%s)\n"), m_strSendData));
	if (m_pDevCmn->fnNET_RKTSendData(m_arSendBuffer, m_nSendLength, K_30_WAIT, Int2Asc(RKT_MODE_R2)) != T_OK)
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnNET_RKTSendData(R2) Fail\n")));
		NVDump('O', 'C', "76", L"", L"SENDFAIL(R2)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Transmission Failure (R2)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		m_pDevCmn->fnNET_RKTConnectClose();
		return T_ERROR;
	}

	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnNET_RKTSendData(R2) Success\n")));
	NVDump('O', 'C', "76", L"", L"SENDOK(R2)");

	////////////////////////////////////////////
	// Initialize Receive Buffer
	//m_nRecvLength = NETBUF_SEND_SIZE;
	//memset(m_arRecvBuffer, 0, NETBUF_SEND_SIZE);
	m_nRecvLength = NETBUF_RECV_SIZE;		// [###2]
	memset(m_arRecvBuffer, 0, NETBUF_RECV_SIZE);

	m_pDevCmn->fnNET_RKTRecvData(m_arRecvBuffer, &m_nRecvLength, K_30_WAIT);

	m_pDevCmn->fnNET_RKTConnectClose();

	// copy to receive data
	m_strRecvData.Format(L"%S", m_arRecvBuffer);
	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnNET_RKTRecvData(R2) last (%s)\n"), m_strRecvData));

	m_strArrRecvData.RemoveAll();
	nRecvIndex = 0;

	SplitString(m_strRecvData, FIELD_DELIMITER, m_strArrRecvData);

	if(m_strArrRecvData[nRecvIndex].GetLength() >= 1)
		RKT_TMKResultResp.RecordFormat		= m_strArrRecvData[nRecvIndex].Mid(0,1);

	if(m_strArrRecvData[nRecvIndex].GetLength() >= 2)
		RKT_TMKResultResp.ApplicationType		= m_strArrRecvData[nRecvIndex].Mid(1,1);

	if(m_strArrRecvData[nRecvIndex].GetLength() >= 3)
		RKT_TMKResultResp.MessageDelimiter	= m_strArrRecvData[nRecvIndex].Mid(2,1);

	if(m_strArrRecvData[nRecvIndex].GetLength() >= 9)
		RKT_TMKResultResp.BankID	= m_strArrRecvData[nRecvIndex].Mid(3,6);

	nRecvIndex++;

	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_TMKResultResp.TerminalID = m_strArrRecvData[nRecvIndex++];
	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_TMKResultResp.ResponseType = m_strArrRecvData[nRecvIndex++];
	if (m_strArrRecvData.GetSize() > nRecvIndex)
		RKT_TMKResultResp.ResponseCode = m_strArrRecvData[nRecvIndex++];

	// Check Validation Receive Data
	if (RKT_TMKResultReq.RecordFormat != RKT_TMKResultResp.RecordFormat)
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnNET_RKTRecvData(R2) Format Error(1)\n")));
		NVDump('O', 'C', "76", L"", L"RDFORMATERR(R2)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("RecordFormat error (R2)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		return T_ERROR;
	}
	else if (RKT_TMKResultReq.ApplicationType != RKT_TMKResultResp.ApplicationType)
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnNET_RKTRecvData(R2) Format Error(2)\n")));
		NVDump('O', 'C', "76", L"", L"APPTYPEERR(R2)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("App Type error (R2)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		return T_ERROR;
	}
	else if (RKT_TMKResultReq.BankID != RKT_TMKResultResp.BankID)
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnNET_RKTRecvData(R2) Format Error(3)\n")));
		NVDump('O', 'C', "76", L"", L"BANKIDERR(R2)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("BankID error (R2)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		return T_ERROR;
	}
	else if (RKT_TMKResultReq.TerminalID != RKT_TMKResultResp.TerminalID)
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnNET_RKTRecvData(R2) Format Error(4)\n")));
		NVDump('O', 'C', "76", L"", L"TMLIDERR(R2)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("TerminalID error (R2)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		return T_ERROR;
	}
	else if (RKT_TMKResultReq.RequestType != RKT_TMKResultResp.ResponseType)
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnNET_RKTRecvData(R2) Format Error(5)\n")));
		NVDump('O', 'C', "76", L"", L"RSPTYPEERR(R2)");
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("Response Type error (R2)"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가
		return T_ERROR;
	}
	else if (RKT_TMKResultResp.ResponseCode != L"000")	// 사양 협의 필요함
	{
		NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnNET_RKTRecvData(R2) Response Error(%s)\n"), RKT_TMKResultResp.ResponseCode));
		NVDump('O', 'C', "76", L"", L"RSPCODEERR(R2)");

		CString JnlData;
		JnlData.Format(L"ResponseCode(%s) error (R2)", RKT_TMKResultResp.ResponseCode);
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, JnlData);	// 2009.10.29 RKT를 위해 US Version에 OA 추가

		if (RKT_TMKResultResp.ResponseCode == L"811")
		{
			// HOST의 KCV 와 ATM의 KCV가 다를 경우 MASTER KEY를 CLEAR하여 거래 불가로 만든다 (사양 협의 필요)
			return T_RKT_FAIL;
		}

		return T_ERROR;
	}

	NHDEBUG(DBG_INFO, (_T("***RKT_Process_R2*** fnNET_RKTRecvData(R2) RKT Success\n")));
	NVDump('O', 'C', "76", L"", L"RKTPRCOK");
	m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, _T("RKT Processing Success"));	// 2009.10.29 RKT를 위해 US Version에 OA 추가

	return T_OK;
}

