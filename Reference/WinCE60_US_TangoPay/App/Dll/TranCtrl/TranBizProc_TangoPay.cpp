#include "stdafx.h"
#include ".\Scr\ScrCtrl.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"
#include ".\Tran\TangoPayTypes.h"
#include "..\..\H\Common\TangoPayConfigurationManager.h"

#include ".\Common\Crypto.h"

// [RWC6-676] SKKim 2024.03.29 added new files

#include ".\Common\NHDbgApi.h"

#if (APP_TANGOPAY)

extern CTimeCheck g_TimeCheck;

//
// Private Functions
//

void CTranCmn::Load_TangoPayDefaultTransData()
{
	CString strTemp, strTemp2;

	if (m_pTangoPayConfigManager == NULL)
	{
		m_pTangoPayConfigManager = new CTangoPayConfigurationManager();
	}

	if (m_pTangoPayConfigManager->GetConfiguration(m_TangoPayConfig))
	{
		m_sTangoPayTransData.Reset();

		m_sTangoPayTransData.strTerminalID = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
		m_sTangoPayTransData.strTerminalID.Trim();

		m_sTangoPayTransData.strPartnerID = m_TangoPayConfig.strPartnerID;
		m_sTangoPayTransData.strPartnerID.Trim();

		m_sTangoPayTransData.strID.Format(L"%s_%s", m_sTangoPayTransData.strTerminalID, m_sTangoPayTransData.strPartnerID);

		m_sTangoPayTransData.strPhoneCountryCode = L"+1";
		//m_sTangoPayTransData.strPhoneCountryCode = L"%%2B1";

	}
}

BOOL CTranCmn::P_NH_IsTangoPayAvailable()
{
	BOOL bActiveTangoPayCondition = FALSE;

	// 1. TCP/IP
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) != NETWORK_DIALUP)
	{
		// 2. 해상도(Width)가 1024인 경우
		#ifdef UNDER_CE
		if (GetSystemMetrics(SM_CXSCREEN) == 1024)
		#endif
		{
			// 3. PublicKey & Enable
			if ((m_TangoPayConfig.strPublicKey.GetLength() > 0) && (m_TangoPayConfig.bEnable == true))
			{
				// 4. SPR 정상 및 용지 있음
				if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL && m_pDevCmn->fnSPR_GetPaperStatus() != SLIP_EMPTY_PAPER)
				{
					// 5. BNA OK
					// 6. Stacker가 Empty인 경우 (Stacker Open시 Stacker Status UNKNOWN 변경 후 Close시 상태 유지됨, Device Status만 Online으로 변경됨 (SP 문의 필요)
					// 6번 사유로 인해 BNA Device Status만 Check하도록 변경함
					//if (m_pDevCmn->fnBNA_GetDeviceStatus() == NORMAL && m_pDevCmn->fstrBNA_GetStackerStatus() == L"EMPTY")
					if (m_pDevCmn->fnBNA_GetDeviceStatus() == NORMAL)
					{
						bActiveTangoPayCondition = TRUE;
					}

				}
			}
		}
	}
	return bActiveTangoPayCondition;
}


BOOL CTranCmn::P_NH_WaitforFrontBillRemoval(bool bDispkayGuideScreen, DWORD nTimeout)
{
	CString strPositionStatus;

	DWORD	dwStart = GetTickCount();
	DWORD	dwElapsed = 0;;

	strPositionStatus = m_pDevCmn->fstrBNA_GetPositionStatus();

	NHDBG((L"P_NH_WaitforFrontBillRemoval - BNA Position Status (%s)\n", strPositionStatus));

	if (strPositionStatus == L"EMPTY")
		return TRUE;

	if (strPositionStatus == L"NOTSUPP")
	{
		LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_414202), INFO_SCR_TIMEOUT, 0, true);	// 투입구 지폐 존재 여부를 모르는 경우 5초 화면 표시하도록 추가 수정
		return TRUE;
	}

	if (bDispkayGuideScreen == true)
	{
		LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_414202), 0, 0, true);
	}

	// 5 mSec 이하는 5 mSec로 설정.
	if (nTimeout <= 5)		nTimeout = 5;

	while ((dwElapsed = GetTickCount() - dwStart) < (DWORD)nTimeout)
	{
		strPositionStatus = m_pDevCmn->fstrBNA_GetPositionStatus();

		if (strPositionStatus == L"EMPTY")
			return TRUE;

		Delay_Msg(100);
	}

	return FALSE;
}

// [RWC6-676] Start SKKim 2024.05.28
void CTranCmn::P_NH_TangoPay_DisplayHostError(CString strErrorCode, CString strErrorMsg)
{
	NHDBG((L"[P_NH_TangoPay_DisplayHostError]\n"));
	CString strMsg;

	// Error Code 가 없는 경우에는Communication Error로 표시
	if (strErrorCode.IsEmpty() == TRUE)
	{
		strMsg.Format(L"%s\n", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003353));
		strMsg += L"Communication failure";
	}
	else
	{
		strMsg.Format(L"%s(%s)\n", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003353), strErrorCode);
		strMsg += strErrorMsg;
	}

	LIB_UserPopUpNotice(SCR_ICON_STOP, strMsg, ABORT_SCR_TIMEOUT);

}
// [RWC6-676] End SKKim 2024.05.28


// Test Function
BIZ_RETURN CTranCmn::P_NH_TangoPay_ResetApiKey()
{
	BIZ_RETURN nRes = RES_NG;

	// ResetAPI는 Identify Server에 접속해야함. (UAT 서버는 미지원됨)
	CString enbinUrl;
	enbinUrl.Format(_T("%s:%d"), m_TangoPayConfig.strIdentifyServerURL, m_TangoPayConfig.nIdentifyPort);
	UpdateTangoPayServices(enbinUrl);

	if (m_pTangoPayService->ResetAPIKey(m_TangoPayConfig.strApiKey) == true)
	{
		// Reset성공시 기존 Public Key 정보 삭제 처리
		m_TangoPayConfig.strPublicKey.Empty();
		m_TangoPayConfig.strPK_CheckSum.Empty();

		if (m_pTangoPayConfigManager != NULL)
		{
			m_pTangoPayConfigManager->SaveConfiguration(m_TangoPayConfig);
			NHDBG((L"TangoPay ResetApikey - public key information updated\n"));
		}

		nRes = RES_OK;
	}

	return nRes;
}

BIZ_RETURN CTranCmn::P_NH_TangoPay_GetPublicKey()
{
	NHDBG((L"[P_NH_TangoPay_GetPublicKey]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_GET_PUBLICKEY, NVP4_NONE, L"P_G_PK");

	BIZ_RETURN nRes = RES_NG;

	if (m_pTangoPayService == NULL)
		return nRes;

	// PublicKey는 Identify Server에 접속해야함.
	CString enbinUrl;
	enbinUrl.Format(_T("%s:%d"), m_TangoPayConfig.strIdentifyServerURL, m_TangoPayConfig.nIdentifyPort);
	UpdateTangoPayServices(enbinUrl);

	TangoPayGetPublicKeyResponse response;

	if (m_pTangoPayService->GetPublicKeyRequest(m_TangoPayConfig.strApiKey, response) == true)
	{
		NHDBG((L"[P_NH_TangoPay_GetPublicKey - PublicKey : %s]\n", response.PublicKey));
		NHDBG((L"[P_NH_TangoPay_GetPublicKey - CheckSum(Base64) : %s]\n", response.CheckSum));

		// PublicKey에 대한 CheckSum 확인 (서버에서 checksum이 base64로 내려오므로 decoding후 비교하도록 추가 수정)
		CString strRecvCheckSum;
		char	szBuffer[4096]	=	{ 0, };
		char	szEnDecoded[4096] =	{ 0, };
		WideToMulti(szBuffer, response.CheckSum, sizeof(szBuffer));
		int nDecodeLen = base64_decode(szBuffer, strlen(szBuffer), (unsigned char*)szEnDecoded, sizeof(szEnDecoded));		// decoded base64
		strRecvCheckSum = MakeUnPack(szEnDecoded, nDecodeLen);

		NHDBG((L"[P_NH_TangoPay_GetPublicKey - CheckSum : %s]\n", strRecvCheckSum));

		CString strCalcHash, strTempCheckSum;
		CStringA input(response.PublicKey);

		strCalcHash = SHA_256_Encrypt((unsigned char *)input.GetBuffer(), response.PublicKey.GetLength(), 0);
		strCalcHash.MakeUpper();

		strTempCheckSum = strRecvCheckSum;
		strTempCheckSum.MakeUpper();

		NHDBG((L"[P_NH_TangoPay_GetPublicKey - Calculate SHA256 CheckSum : %s]\n", strCalcHash));
		
		if (strCalcHash.CompareNoCase(strTempCheckSum) == 0)
		{
			NHDBG((L"[P_NH_TangoPay_GetPublicKey - CheckSum OK]\n"));

			m_TangoPayConfig.strPublicKey = response.PublicKey;
			m_TangoPayConfig.strPK_CheckSum = strRecvCheckSum;	// original data save

			if (m_pTangoPayConfigManager != NULL)
			{
				m_pTangoPayConfigManager->SaveConfiguration(m_TangoPayConfig);
				NHDBG((L"TangoPay public key updated\n"));
			}

			nRes = RES_OK;
		}
		else
		{
			NHDBG((L"[P_NH_TangoPay_GetPublicKey - CheckSum NG]\n"));
			nRes = RES_TANGOPAY_CHECKSUM_ERR;
		}
	}

	return nRes;

}

BIZ_RETURN CTranCmn::P_NH_TangoPay_GetTerminalJWT(bool bDisplayProcessingScreen)
{
	NHDBG((L"[P_NH_TangoPay_GetTerminalJWT]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_GET_JWT, NVP4_NONE, L"P_G_JWT");

	BIZ_RETURN nRes = RES_NG;

	// Display Screen
	if (bDisplayProcessingScreen == true)
	{
		NHDBG((_T("SHOW SCREEN 116\n")));

		///////////////////////////////////
		// DISPLAY SCREEN

		m_pDevCmn->fnSCR_DisplayPrevSet(116);

		// Setting Screen
		{
			// Sub Title
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116001));

			// Under Guide
			m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116002));
		}

		m_pDevCmn->fnSCR_DisplayScreen(116);
	}

	// JWT는 Identify Server에 접속해야함.
	CString enbinUrl;
	enbinUrl.Format(_T("%s:%d"), m_TangoPayConfig.strIdentifyServerURL, m_TangoPayConfig.nIdentifyPort);
	UpdateTangoPayServices(enbinUrl);

	// Setting
	TangoPayGetTerminaljwtRequest request;

	CString strNonce = CryptoLib::GetNewUUID(false);

	CString strHmackey = CryptoLib::GetNewUUID(false);
	CString strHmackeyEncrypted = CryptoLib::RSAEncryptWithPublicKey(strHmackey, m_TangoPayConfig.strPublicKey);
	CString strHmackeyEncryptedBase64 = CryptoLib::EncodeToBase64(strHmackeyEncrypted);

	CString strChecksum = CryptoLib::GenerateHMACSHA256(strNonce + m_sTangoPayTransData.strID, strHmackey);
	//CString strChecksumBase64url = CryptoLib::EncodeToBase64(strChecksum, true);
	CString strChecksumBase64 = CryptoLib::EncodeToBase64(strChecksum, false);			// 서버 spec 변경 (base64url ->base64로 통일)

	request.Id = m_sTangoPayTransData.strID;
	request.Key = strHmackeyEncryptedBase64;
	request.Nonce = strNonce;
	request.CheckSum = strChecksumBase64;

	NHDBG((L"[P_NH_TangoPay_GetTerminalJWT Request - Id : %s]\n", request.Id));
	NHDBG((L"[P_NH_TangoPay_GetTerminalJWT Request - Key : %s]\n", strHmackey));
	NHDBG((L"[P_NH_TangoPay_GetTerminalJWT Request - Key (Encrypted) : %s]\n", request.Key));
	NHDBG((L"[P_NH_TangoPay_GetTerminalJWT Request - Nonce : %s]\n", request.Nonce));
	NHDBG((L"[P_NH_TangoPay_GetTerminalJWT Request - CheckSum : %s]\n", request.CheckSum));

	TangoPayGetTerminaljwtResponse response;

	if (m_pTangoPayService->GetTerminaljwtRequest(request, response) == true)
	{
		NHDBG((L"[P_NH_TangoPay_GetTerminalJWT - AccessToken : %s]\n", response.AccessToken));
		NHDBG((L"[P_NH_TangoPay_GetTerminalJWT - Nonce : %s]\n", response.Nonce));
		NHDBG((L"[P_NH_TangoPay_GetTerminalJWT - Signature : %s]\n", response.Signature));	// Base64 encoding

		// TODO: Verify Signature with public key
		m_sTangoPayTransData.strAccessToken = response.AccessToken;
		NHDBG((L"[P_NH_TangoPay_GetTerminalJWT - TransData AccessToken : %s]\n", m_sTangoPayTransData.strAccessToken));
		nRes = RES_OK;
	}
	else
	{
		NHDBG((L"[P_NH_TangoPay_GetTerminalJWT - Response Error]\n"));
	
		CString strLog;
		strLog.Format(L"R:%d,M:%s", response.resultinfo.Result, response.resultinfo.ErrMessage);
		NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_GET_JWT, response.resultinfo.ErrCode, strLog);

		// Host Error Message 추가
		if (bDisplayProcessingScreen == true)
		{
			if (response.resultinfo.strStatusCode != L"200")
				P_NH_TangoPay_DisplayHostError(response.resultinfo.strStatusCode, response.resultinfo.strStatusMsg);
			else
				P_NH_TangoPay_DisplayHostError(response.resultinfo.ErrCode, response.resultinfo.ErrMessage);				
		}

		g_sBizFlowInfo.nReasonforCancel = nRes;
	}

	return nRes;
}

BIZ_RETURN CTranCmn::P_NH_TangoPay_SelectLanguage()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_TangoPay_SelectLanguage]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_SELECTLANGUAGE, NVP4_NONE, L"P_S_LAN");

	// Default Language랑 상관없이 영어로 Select Language 화면 표시
	m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);

	BIZ_RETURN		nRes;
	CString			GetKeyStr;
	BOOL			bShowScreen = TRUE;

	g_TimeCheck.SetTargetTimeAfterSec(MAX_DEVSCR_TIME);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(418);

			// Setting Screen - OP와 상관없이 무조건 2개의 언어만 지원
			{
				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_102002));

				// English
				m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_ENGLISH));

				// Spanish
				m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_HISTORY_SPANISH));
			}

			m_pDevCmn->fnSCR_DisplayScreen(418, KEYIN_TIME_OUT, PIN_MENU_MODE);
			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_SELECTLANGUAGE, NVP4_NONE, L"CANCEL");

				nRes = RES_USER_EXIT;
				g_sBizFlowInfo.nReasonforCancel = nRes;
				return nRes;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_SELECTLANGUAGE, NVP4_NONE, L"TIMEOUT");

				nRes = RES_USER_TIMEOUT;
				g_sBizFlowInfo.nReasonforCancel = nRes;
				return nRes;
			}
			else if (GetKeyStr == L"ENGLISH")
			{
				m_pDevCmn->fnSCR_SetCurrentLangMode(ENG_MODE);
				return RES_OK;
			}
			else if (GetKeyStr == L"SPANISH")
			{
				m_pDevCmn->fnSCR_SetCurrentLangMode(SPN_MODE);
				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("MAX TIMEOUT\n")));
	m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);

	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_SELECTLANGUAGE, L"TIMEOUT", L"P_S_LAN");

	return RES_USER_TIMEOUT;
}


BIZ_RETURN CTranCmn::P_NH_TangoPay_Enter_PhoneNumber()
{
	NHDBG((L"[P_NH_TangoPay_Enter_PhoneNumber]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ENTER_PHONE, NVP4_NONE, L"P_PHONE");

	BIZ_RETURN nRes = RES_NG;
	int	 nRetryCount = 3;
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// ADA모드는 일단 미지원으로 처리
	if( IsAdaTransaction() )
		return RES_NG;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	NHDBG((L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// End of [#2413]

	// Transaction은 Transaction Server에 접속해야함.
	CString enbinUrl;
	enbinUrl.Format(_T("%s:%d"), m_TangoPayConfig.strTransactionServerURL, m_TangoPayConfig.nTransactionPort);
	UpdateTangoPayServices(enbinUrl);

	// [RWC6-676] SKKim 2024.05.28 사양 변경
	// 1. 입력자리수 오류에만 retry 적용
	// 2. 서버로부터 negative 수신시 retry없이 거래 취소 처리
	// 3. 등록되지 않은 user인 경우 화면 표시 후 거래 취소 처리
	while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDBG((_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(410);

			// Setting Screen
			{
				// TangoPay Logo
				//m_pDevCmn->fnSCR_DisplayString(20, m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_TOPCONER));

				// TextID defined in Ap.dat file
			}

			m_pDevCmn->fnSCR_DisplayScreen(410, KEYIN_TIME_OUT, PIN_MENU_MODE);

			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NHDBG((_T("PRESS Cancel EXIT\n")));

				nRes = RES_USER_EXIT;
				g_sBizFlowInfo.nReasonforCancel = nRes;
				return nRes;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));

				nRes = RES_USER_TIMEOUT;
				g_sBizFlowInfo.nReasonforCancel = nRes;
				return nRes;
			}
			else
			{
				if (GetKeyStr.GetLength() == 10)
				{
					m_sTangoPayTransData.strPhoneNumber = GetKeyStr;
					NHDBG((_T("  TangoPay US Phone Number - [%s]\n"), m_sTangoPayTransData.strPhoneNumber));

					// Display Screen
					{
						NHDBG((_T("SHOW SCREEN 116\n")));

						///////////////////////////////////
						// DISPLAY SCREEN

						m_pDevCmn->fnSCR_DisplayPrevSet(116);

						// Setting Screen
						{
							// Sub Title
							m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116001));

							// Under Guide
							m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116002));
						}

						m_pDevCmn->fnSCR_DisplayScreen(116);
					}

					// Get User를 해서 계정이 있는지 확인 처리
					TangoPayGetUserRequest request;
					TangoPayGetUserResponse response;

					request.eid = m_sTangoPayTransData.strAccessToken;

					NHDBG((L"[P_NH_TangoPay_Enter_PhoneNumber Get User - AccessToken : %s]\n", m_sTangoPayTransData.strAccessToken));
					NHDBG((L"[P_NH_TangoPay_Enter_PhoneNumber Get User - eid : %s]\n", request.eid));

					request.phoneNumber = m_sTangoPayTransData.strPhoneNumber;
					request.countryCode = m_sTangoPayTransData.strPhoneCountryCode;

					if (m_pTangoPayService->GetUserRequest(request, response) == true)
					{
						NHDBG((L"[P_NH_TangoPay_Enter_PhoneNumber Get User - retCode : %s]\n", response.retCode));
						NHDBG((L"[P_NH_TangoPay_Enter_PhoneNumber Get User - retMsg : %s]\n", response.retMsg));
						NHDBG((L"[P_NH_TangoPay_Enter_PhoneNumber Get User - userId : %s]\n", response.userId));

						// [RWC6-676] 정상 처리 조건 변경 (Pivot과 동일하게)
						// S00001의 의미는 전달받지 못함.
						if (response.retCode == L"S00001" && response.userId.GetLength() > 0)
						{
							m_sTangoPayTransData.strUserId = response.userId;
							return RES_OK;
						}
						else
						{
							// 위의 경우가 아닌 경우에는 미등록으로 판단하여 App Store 화면 표시 (Pivot과 동일)
							P_NH_TangoPay_DownloadAppGuideScreen();
							nRes = RES_NG;
							g_sBizFlowInfo.nReasonforCancel = nRes;
							return nRes;
						}
					}
					else
					{
						NHDBG((L"[P_NH_TangoPay_Enter_PhoneNumber: GetUser NG]\n"));

						CString strLog;
						strLog.Format(L"R:%d,M:%s", response.resultinfo.Result, response.resultinfo.ErrMessage);
						NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ENTER_PHONE, response.resultinfo.ErrCode, strLog);

						strLog.Format(L"RET:%s", response.retCode);
						NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ENTER_PHONE, response.resultinfo.ErrCode, strLog);

						// [RWC6-676] SKKim 2024.05.28 Host Error 추가
						if (response.resultinfo.strStatusCode != L"200")
						{
							// HTTP 오류인 경우에는 Host Error 표시 후 거래 취소
							P_NH_TangoPay_DisplayHostError(response.resultinfo.strStatusCode, response.resultinfo.strStatusMsg);
						}
						else
						{
							// 그 외의 경우에는 미등록이라고 파난하여 App Store 화면 표시 (Pivot과 동일 사양)
							P_NH_TangoPay_DownloadAppGuideScreen();
							nRes = RES_NG;
							g_sBizFlowInfo.nReasonforCancel = nRes;
							return nRes;							
						}
					}

					nRes = RES_NG;
					g_sBizFlowInfo.nReasonforCancel = nRes;
					return nRes;
				}
				else
				{
					NHDBG((L"length of entered string is not 10\n"));
				}

				nRetryCount--;
				if(nRetryCount>0)
				{
					LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_410201), ABORT_SCR_TIMEOUT, 0, true);
					bShowScreen = TRUE;
				}
			}
		}

		Delay_Msg(50);
	}

	if (nRetryCount <= 0)
	{
		LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_410301), ABORT_SCR_TIMEOUT);
		NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ENTER_PHONE, L"RTYOVER", L"P_PHONE");

		nRes = RES_NG;
		g_sBizFlowInfo.nReasonforCancel = nRes;
		return nRes;
	}

	NHDBG((_T("MAX TIMEOUT\n")));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ENTER_PHONE, L"TIMEOUT", L"P_PHONE");

	return nRes;
}


BIZ_RETURN CTranCmn::P_NH_TangoPay_Enter_OTP()
{
	NHDBG((L"[P_NH_TangoPay_Enter_OTP]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ENTER_OTP, NVP4_NONE, L"P_OTP");

	BIZ_RETURN nRes = RES_NG;
	int	 nRetryCount = 3;
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// ADA모드는 일단 미지원으로 처리
	if( IsAdaTransaction() )
		return RES_USER_EXIT;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_NG;

	NHDBG((L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// End of [#2413]

	while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDBG((_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(411);

			// Setting Screen
			{
				// TangoPay Logo
				//m_pDevCmn->fnSCR_DisplayString(20, m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_TOPCONER));

				// TextID defined in Ap.dat file
			}

			m_pDevCmn->fnSCR_DisplayScreen(411, KEYIN_TIME_OUT, PIN_MENU_MODE);

			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NHDBG((_T("PRESS Cancel EXIT\n")));

				nRes = RES_USER_EXIT;
				g_sBizFlowInfo.nReasonforCancel = nRes;
				return nRes;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDBG((_T("KEY IN TIMEOUT\n")));

				nRes = RES_USER_TIMEOUT;
				g_sBizFlowInfo.nReasonforCancel = nRes;
				return nRes;
			}
			else
			{
				if (GetKeyStr.GetLength() == 6)
				{
					m_sTangoPayTransData.strOTPCode = GetKeyStr.Left(5);		// 입력한 6자리 중 5자리만 OTP Code에 저장
					m_sTangoPayTransData.strOTPSeed = GetKeyStr.Right(1);		// OTP Code의 맨 끝 1자리를 seed로 사용함

					NHDBG((_T("  TangoPay OTP code - [%s]\n"), m_sTangoPayTransData.strOTPCode));

					NHDBG((_T("SHOW SCREEN 116\n")));

					///////////////////////////////////
					// DISPLAY SCREEN

					m_pDevCmn->fnSCR_DisplayPrevSet(116);

					// Setting Screen
					{
						// Sub Title
						m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116001));

						// Under Guide
						m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116002));
					}

					m_pDevCmn->fnSCR_DisplayScreen(116);

					TangoPayPreStagingVerifyRequest request;
					TangoPayPreStagingVerifyResponse response;

					request.eid = m_sTangoPayTransData.strAccessToken;

					request.PhoneNumber =	m_sTangoPayTransData.strPhoneNumber;
					request.CountryCode =	m_sTangoPayTransData.strPhoneCountryCode;
					request.OtpCode =		m_sTangoPayTransData.strOTPCode;
					request.OtpSeed =		m_sTangoPayTransData.strOTPSeed;

					bool bresult = m_pTangoPayService->PrestagingVerifyRequest(request, response);

					if ( bresult == true)
					{
						if (response.verify == true)
						{
							// PrestageVerify에서 유효한 Data는 TransactionId / ConfirmationNumber / PrestagingId 3개임.
							// 추가로 화면에 표시할 Receive Country는 Verify Value를 표시 (Validation에서는 Country정보가 안내려옴)
							m_sTangoPayTransData.strTransactionId = response.prestaingInfo_obj.transactionId;
							m_sTangoPayTransData.strConfirmationNumber = response.prestaingInfo_obj.confirmationNumber;
							m_sTangoPayTransData.strPrestagingId = response.prestaingInfo_obj.preStagingId;
							m_sTangoPayTransData.strReceiveCountry = response.prestaingInfo_obj.receiveCountry;

							NHDBG((L"[P_NH_TangoPay_Enter_OTP PrestagingVerifyRequest - TransactionId : %s]\n", response.prestaingInfo_obj.transactionId));
							NHDBG((L"[P_NH_TangoPay_Enter_OTP PrestagingVerifyRequest - ConfirmationNumber : %s]\n", response.prestaingInfo_obj.confirmationNumber));
							NHDBG((L"[P_NH_TangoPay_Enter_OTP PrestagingVerifyRequest - PrestagingId : %s]\n", response.prestaingInfo_obj.preStagingId));
							NHDBG((L"[P_NH_TangoPay_Enter_OTP PrestagingVerifyRequest - strReceiveCountry : %s]\n", response.prestaingInfo_obj.receiveCountry));

							return RES_OK;
						}
						else
						{
							NHDBG((L"[P_NH_TangoPay_Enter_OTP PrestagingVerifyRequest - verify false]\n"));
							NHDBG((L"[P_NH_TangoPay_Enter_OTP PrestagingVerifyRequest - retCode : %s]\n", response.retCode));
							NHDBG((L"[P_NH_TangoPay_Enter_OTP PrestagingVerifyRequest - retMsg : %s]\n", response.retMsg));

							// [RWC6-676] SKKim 2024.05.28 Host Error 추가
							P_NH_TangoPay_DisplayHostError(response.retCode, response.retMsg);
						}
					}
					else
					{
						NHDBG((L"[P_NH_TangoPay_Enter_OTP: PrestagingVerifyRequest NG]\n"));

						CString strLog;
						strLog.Format(L"R:%d,M:%s", response.resultinfo.Result, response.resultinfo.ErrMessage);
						NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ENTER_OTP, response.resultinfo.ErrCode, strLog);

						// [RWC6-676] SKKim 2024.05.28 Host Error 추가
						if (response.resultinfo.strStatusCode != L"200")
							P_NH_TangoPay_DisplayHostError(response.resultinfo.strStatusCode, response.resultinfo.strStatusMsg);
						else
							P_NH_TangoPay_DisplayHostError(response.resultinfo.ErrCode, response.resultinfo.ErrMessage);

					}

					nRetryCount--;
					bShowScreen = TRUE;
					continue;

				}
				else
				{
					NHDBG((L"length of entered string is not 6\n"));
				}

				nRetryCount--;
				if(nRetryCount>0)
				{
					LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_411201), INFO_SCR_TIMEOUT, 0, true);	
					bShowScreen = TRUE;
				}
			}
		}

		Delay_Msg(50);
	}

	if (nRetryCount <= 0)
	{
		LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_410301), ABORT_SCR_TIMEOUT, 0, true);
		NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ENTER_PHONE, L"RTYOVER", L"P_PHONE");

		nRes = RES_NG;
		g_sBizFlowInfo.nReasonforCancel = nRes;
		return nRes;
	}

	NHDBG((_T("MAX TIMEOUT\n")));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ENTER_PHONE, L"TIMEOUT", L"P_PHONE");

	return nRes;
}


BIZ_RETURN CTranCmn::P_NH_TangoPay_StagingValidation(bool bDisplayProcessingScreen)
{
	NHDBG((L"[P_NH_TangoPay_StagingValidation]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_STAGE_VALIDATION, NVP4_NONE, L"P_PRESTAGE");

	BIZ_RETURN nRes = RES_NG;

	// Display Screen
	if (bDisplayProcessingScreen == true)
	{
		NHDBG((_T("SHOW SCREEN 116\n")));

		///////////////////////////////////
		// DISPLAY SCREEN

		m_pDevCmn->fnSCR_DisplayPrevSet(116);

		// Setting Screen
		{
			// Sub Title
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116001));

			// Under Guide
			m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116002));
		}

		m_pDevCmn->fnSCR_DisplayScreen(116);
	}

	// validation
	TangoPayStagingValidationRequest request;
	TangoPayStagingValidationResponse response;

	request.eid = m_sTangoPayTransData.strAccessToken;

	request.MachineId = m_sTangoPayTransData.strTerminalID;
	request.PartnerId = m_sTangoPayTransData.strPartnerID;
	request.TransactionId = m_sTangoPayTransData.strTransactionId;
	request.ConfirmationNumber = m_sTangoPayTransData.strConfirmationNumber;
	request.PrestagingId = m_sTangoPayTransData.strPrestagingId;

	if (m_pTangoPayService->StagingValidationRequest(request, response) == true)
	{
		m_sTangoPayTransData.strMgiTransactionSessionId = response.mgiTransactionSessionId;
		m_sTangoPayTransData.strTotalAmount = response.SendAmountInfo_obj.totalAmountToCollect;
		m_sTangoPayTransData.strSendFees = response.SendAmountInfo_obj.totalSendFees;
		m_sTangoPayTransData.strSendTaxes = response.SendAmountInfo_obj.totalSendTaxes;
		m_sTangoPayTransData.strSendAmount = response.SendAmountInfo_obj.sendAmount;
		m_sTangoPayTransData.strSendCurrency = response.SendAmountInfo_obj.sendCurrency;
		m_sTangoPayTransData.strReceiveMethod = response.deliverOptionDisplayName;
		m_sTangoPayTransData.strReceiveAmount = response.ReceiveAmountInfo_obj.receiveAmount;
		m_sTangoPayTransData.strReceiveCurrency = response.ReceiveAmountInfo_obj.receiveCurrency;
		m_sTangoPayTransData.strReceiveName.Format(L"%s%s %s", response.receiverFirstName, response.receiverMiddleName, response.receiverLastName);

		NHDBG((L"[P_NH_TangoPay_StagingValidation StagingValidationRequest - strReceiveMethod : %s]\n", response.deliverOptionDisplayName));
		NHDBG((L"[P_NH_TangoPay_StagingValidation StagingValidationRequest - totalAmountToCollect : %s]\n", response.SendAmountInfo_obj.totalAmountToCollect));
		NHDBG((L"[P_NH_TangoPay_StagingValidation StagingValidationRequest - sendAmount : %s]\n", response.SendAmountInfo_obj.sendAmount));
		NHDBG((L"[P_NH_TangoPay_StagingValidation StagingValidationRequest - sendCurrency : %s]\n", response.SendAmountInfo_obj.sendCurrency));
		NHDBG((L"[P_NH_TangoPay_StagingValidation StagingValidationRequest - strReceiveAmount : %s]\n", response.ReceiveAmountInfo_obj.receiveAmount));
		NHDBG((L"[P_NH_TangoPay_StagingValidation StagingValidationRequest - strReceiveCurrency : %s]\n", response.ReceiveAmountInfo_obj.receiveCurrency));
		NHDBG((L"[P_NH_TangoPay_StagingValidation StagingValidationRequest - strReceiveName : %s]\n", m_sTangoPayTransData.strReceiveName));

		int nDisclosuresCount = response.disclosuresFirst_array.GetCount();
		sDisclosure temp;
		for(int i=0; i<nDisclosuresCount; i++)
		{
			POSITION pos = response.disclosuresFirst_array.FindIndex(i);
			temp = response.disclosuresFirst_array.GetAt(pos);
			m_sTangoPayTransData.strArr_FraudWarning[0].Add(temp.fraudWarning);
			m_sTangoPayTransData.strArr_Disclosures[0].Add(temp.disclosure);

		}

		nDisclosuresCount = response.disclosuresSecond_array.GetCount();
		for(int i=0; i<nDisclosuresCount; i++)
		{
			POSITION pos = response.disclosuresSecond_array.FindIndex(i);
			temp = response.disclosuresSecond_array.GetAt(pos);
			m_sTangoPayTransData.strArr_FraudWarning[1].Add(temp.fraudWarning);
			m_sTangoPayTransData.strArr_Disclosures[1].Add(temp.disclosure);
		}

		return RES_OK;
	}
	else
	{
		NHDBG((L"[P_NH_TangoPay_StagingValidation: Staging Validation NG]\n"));

		CString strLog;
		strLog.Format(L"R:%d,M:%s", response.resultinfo.Result, response.resultinfo.ErrMessage);
		NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_STAGE_VALIDATION, response.resultinfo.ErrCode, strLog);

		// [RWC6-676] SKKim 2024.05.28 Host Error 추가
		if (response.resultinfo.strStatusCode != L"200")
			P_NH_TangoPay_DisplayHostError(response.resultinfo.strStatusCode, response.resultinfo.strStatusMsg);
		else
			P_NH_TangoPay_DisplayHostError(response.resultinfo.ErrCode, response.resultinfo.ErrMessage);

		nRes = RES_NG;
		g_sBizFlowInfo.nReasonforCancel = nRes;
	}

	return nRes;
}


BIZ_RETURN CTranCmn::P_NH_TangoPay_PreTransactionSummary()
{
	NHDBG((L"[P_NH_TangoPay_PreTransactionSummary]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_PRE_TRANS_SUMMARY, NVP4_NONE, L"P_T_S");

	BIZ_RETURN	nRes = RES_NG;
	CString		GetKeyStr;
	CString		strValue, strDispAmount;
	BOOL		bShowScreen = TRUE;

	// ADA모드는 일단 미지원으로 처리
	if( IsAdaTransaction() )
		return RES_USER_EXIT;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_NG;

	NHDBG((L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// End of [#2413]

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDBG((_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(412);

			// Setting Screen
			{
				// TangoPay Logo
				//m_pDevCmn->fnSCR_DisplayString(20, m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_TOPCONER));

				// TextID defined in Ap.dat file

				// Sent To Name
				m_pDevCmn->fnSCR_DisplayString(1, m_sTangoPayTransData.strReceiveName);

				// Country
				m_pDevCmn->fnSCR_DisplayString(2, m_sTangoPayTransData.strReceiveCountry);

				// Total Amount to Deposit (TangoPay TODO:) 금액 단위 정보가 없음 - 현재는 cent가 포함되어 내려옴 (그대로 표시함)
				strDispAmount = m_sTangoPayTransData.strTotalAmount;
				//strDispAmount.Remove('.');
				//strValue.Format(L"%s %s", m_sTangoPayTransData.strSendCurrency, MakeMoneyCent(strDispAmount));
				strValue.Format(L"%s %s", m_sTangoPayTransData.strSendCurrency, strDispAmount);
				m_pDevCmn->fnSCR_DisplayString(3, strValue);

				// Send Amount (TangoPay TODO:) 금액 단위 정보가 없음 - 현재는 cent가 포함되어 내려옴 (그대로 표시함)
				strDispAmount = m_sTangoPayTransData.strSendAmount;
				//strDispAmount.Remove('.');
				//strValue.Format(L"%s %s", m_sTangoPayTransData.strSendCurrency, MakeMoneyCent(strDispAmount));
				strValue.Format(L"%s %s", m_sTangoPayTransData.strSendCurrency, strDispAmount);
				m_pDevCmn->fnSCR_DisplayString(4, strValue);

				// Total to Receiver (TangoPay TODO:) 금액 단위 정보가 없음 - Mexico인 경우 cent가 없이 내래옴 (그대로 표시함)
				strDispAmount = m_sTangoPayTransData.strReceiveAmount;
				//strValue.Format(L"%s %s", m_sTangoPayTransData.strReceiveCurrency, MakeMoneyAmount(strDispAmount));
				strValue.Format(L"%s %s", m_sTangoPayTransData.strReceiveCurrency, strDispAmount);
				m_pDevCmn->fnSCR_DisplayString(5, strValue);
			}

			m_pDevCmn->fnSCR_DisplayScreen(412, KEYIN_TIME_OUT, PIN_MENU_MODE);

			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDBG((L"GetKeyString(): [%s]\n", GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				nRes = RES_USER_EXIT;
				g_sBizFlowInfo.nReasonforCancel = nRes;
				return nRes;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				nRes = RES_USER_TIMEOUT;
				g_sBizFlowInfo.nReasonforCancel = nRes;
				return nRes;
			}
			else
			{
				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_PRE_TRANS_SUMMARY, L"TIMEOUT", L"P_T_S");

	return nRes;
}


BIZ_RETURN CTranCmn::P_NH_TangoPay_CustomerNoticeMessage(int nMsgType)
{
	NHDBG((L"[P_NH_TangoPay_CustomerNoticeMessage (%d)]\n", nMsgType));

	if (nMsgType == 0)
		NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_FRAUD_WARNING, NVP4_NONE, L"F_WARN");
	else
		NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_DISCLOURE, NVP4_NONE, L"DISCLOURE");

	BIZ_RETURN	nRes = RES_NG;
	CString		GetKeyStr;
	CString		strValue, strPrintData, strTemp;
	CStringArray strArr_Message;
	BOOL		bShowScreen = TRUE;
	int			nCurPage = 0;
	int			nTotalPage = 0;
	int			nTotalLineCount = 0;
	int			nLastPageRemainCount = 0;
	int			nScreenMaxLine = 15;	// 화면 Design이 현재 15Line까지 표시하도록 됨.
	BOOL		bPrinted = FALSE;

	// 현재 FraudWarning값은 통채로 내려오므로 AP에서 화면에 맞게 짤라서 처리해야함
	strArr_Message.RemoveAll();

	if (nMsgType == 0)
	{
		if (m_sTangoPayTransData.strArr_FraudWarning[0].GetCount() > 0)
		{
			strValue = m_sTangoPayTransData.strArr_FraudWarning[0].GetAt(0);
			SplitString(strValue, L"\r\n", strArr_Message);
		}
	}
	else
	{
		if (m_sTangoPayTransData.strArr_Disclosures[0].GetCount() > 0)
		{
			strValue = m_sTangoPayTransData.strArr_Disclosures[0].GetAt(0);
			SplitString(strValue, L"\r\n", strArr_Message);
		}
	}

	// 화면에 최대 15Line까지만 표시되도록 Desing 처리됨
	nTotalLineCount = strArr_Message.GetCount();

	nTotalPage = nTotalLineCount / nScreenMaxLine;

	nLastPageRemainCount = nTotalLineCount % nScreenMaxLine;

	// 마지막 Page에 내용은 없고 "\r\n"만 있는 경우에 대한 처리 추가
	for(int i=0; i<nLastPageRemainCount; i++)
	{
		strValue = strArr_Message.GetAt((nTotalPage*nScreenMaxLine) + i);
		strValue.Remove('\r');
		strValue.Remove('\n');
		if (strValue.GetLength() > 0)
		{
			nTotalPage++;
			break;
		}
	}

	// ADA모드는 일단 미지원으로 처리
	if( IsAdaTransaction() )
		return RES_USER_EXIT;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_NG;

	NHDBG((L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDBG((_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(413);

			// Setting Screen
			{
				// TangoPay Logo
				//m_pDevCmn->fnSCR_DisplayString(20, m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_TOPCONER));

				// APValue1
				if (nMsgType == 0)
					m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_413001));
				else
					m_pDevCmn->fnSCR_DisplayString(1, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_413002));

				// Fraud Warning Value
				strValue.Empty();
				int nStartIndex = nCurPage * nScreenMaxLine;
				for(int i=0; i<nScreenMaxLine; i++)
				{
					if ((nStartIndex+i) < nTotalLineCount)
					{
						strValue += strArr_Message.GetAt(nStartIndex+i);

						if ( i != nScreenMaxLine -1)
							strValue += L"\n";
					}
				}
				m_pDevCmn->fnSCR_DisplayString(2, strValue);

				// Page (Current / Total)
				strValue.Format(L"%d / %d", nCurPage + 1, nTotalPage);
				m_pDevCmn->fnSCR_DisplayString(3, strValue);

				// SKKim 2024.05.27 Fraud Warning message는 print 안함
				if (nMsgType == 0)
				{
					m_pDevCmn->fnSCR_DisplayImage(1, L"off");
				}
				else
				{
					// Print Button
					if (m_pDevCmn->fnSPR_GetPaperStatus() != SLIP_EMPTY_PAPER)
					{
						if (bPrinted == FALSE)
							m_pDevCmn->fnSCR_DisplayImage(1, L"on");
						else
							m_pDevCmn->fnSCR_DisplayImage(1, L"off");
					}
					else
						m_pDevCmn->fnSCR_DisplayImage(1, L"off");
				}

				// Continue Button
				if ((nCurPage + 1) == nTotalPage)
					m_pDevCmn->fnSCR_DisplayImage(2, L"on");
				else
					m_pDevCmn->fnSCR_DisplayImage(2, L"off");
			}

			m_pDevCmn->fnSCR_DisplayScreen(413, KEYIN_TIME_OUT, PIN_MENU_MODE);

			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDBG((L"GetKeyString(): [%s] Current : %d, Total : %d\n", GetKeyStr, nCurPage + 1, nTotalPage));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				nRes = RES_USER_EXIT;
				g_sBizFlowInfo.nReasonforCancel = nRes;
				return nRes;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				nRes = RES_USER_TIMEOUT;
				g_sBizFlowInfo.nReasonforCancel = nRes;
				return nRes;
			}
			else if (GetKeyStr == S_YES)
			{
				return RES_OK;
			}
			else if (GetKeyStr == L"PREV")
			{
				nCurPage--;

				if (nCurPage < 0)
					nCurPage = 0;

				bShowScreen = TRUE;

			}
			else if (GetKeyStr == L"NEXT")
			{
				nCurPage++;

				if ((nCurPage+1) >= nTotalPage)
					nCurPage = nTotalPage -1;

				bShowScreen = TRUE;
			}
			else if (GetKeyStr == L"PRINT")
			{
				NHDBG((L"[P_NH_TangoPay_CustomerNoticeMessage] Print Start\n"));

				bPrinted = TRUE;

				if (nMsgType == 1)
					NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_DISCLOURE, NVP4_NONE, L"D_PRINT");

				if (nMsgType == 0)
					LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_413201), 0);
				else
					LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_413202), 0);

				strPrintData.Empty();
				strValue.Empty();

				for(int i=1; i<=nTotalLineCount; i++)
				{
					strValue = strArr_Message.GetAt(i-1);
					strValue.Remove('\r');
					strValue.Remove('\n');

					if (strValue.GetLength() > 0)
					{
						// 40line이 넘어가는 경우는 개행 처리 후 print하도록 임시 수정
						if(strValue.GetLength() > 40)
						{
							NHDBG((_T("Print line count is over (%s)\n"), strValue));

							strTemp = strValue;
							strValue = strTemp.Left(40);
							AddPrintData(strPrintData, strValue);

							strValue = strTemp.Right(strTemp.GetLength() - 40);
						}

						AddPrintData(strPrintData, strValue);
						//NHDBG((_T("Print Data (%d)(%s)\n"), i, strValue));
					}
				}

				if (strPrintData.GetLength() > 0)
					m_pDevCmn->fnSPR_PrintReceipt(FALSE, strPrintData, K_1_WAIT);

				// Print 후 SPR 장애시 화면 추가
				if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
				{
					NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_DISCLOURE, NVP4_NONE, L"PRT_ERR");

					LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119201), ABORT_SCR_TIMEOUT);
				}

				bShowScreen = TRUE;

			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));
	if (nMsgType == 0)
		NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_FRAUD_WARNING, L"TIMEOUT", L"F_WARN");
	else
		NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_DISCLOURE, L"TIMEOUT", L"DISCLOURE");

	return nRes;
}


BIZ_RETURN CTranCmn::P_NH_TangoPay_AcceptCashProcess()
{
	NHDBG((L"[P_NH_TangoPay_AcceptCashProcess] start\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ACCEPT_CASH, NVP4_NONE, L"ACCEPT_CASH");

	BIZ_RETURN	nRes = RES_NG;
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;
	CString		strValue, strTemp;
	int			nTotalDepositAmount = 0;
	int			nCurDepositAmount = 0, nAmountCollected = 0, nTempAmountCollected = 0;
	bool		bStartCash_OK = false;
	BOOL		bBillCondition_OK = FALSE;
	CString     strError;

	// ADA모드는 일단 미지원으로 처리
	if( IsAdaTransaction() )
		return RES_USER_EXIT;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_NG;

	NHDBG((L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	nTotalDepositAmount = Asc2Int(m_sTangoPayTransData.strTotalAmount);

	// Cent가 포함된 금액이므로 /100처리함
	nTotalDepositAmount = nTotalDepositAmount / 100;
	CString strDisplayToTalAmount;
	strDisplayToTalAmount = Int2Asc(nTotalDepositAmount);
	m_sTangoPayTransData.strDepositedAmount = L"0";

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDBG((_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(414);

			// Setting Screen
			{
				// TangoPay Logo
				//m_pDevCmn->fnSCR_DisplayString(20, m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_TOPCONER));

				// TextID defined in Ap.dat file

				// 1. Total Deposit Amount
				strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyAmount(strDisplayToTalAmount));
				strValue = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_414001);
				strValue.Replace(L"XXX", strTemp);
				m_pDevCmn->fnSCR_DisplayString(1, strValue);	

				// 2. Current Deposited Amount
				strTemp.Format(L"%s%s", GetCurrencySymbol(), MakeMoneyAmount(m_sTangoPayTransData.strDepositedAmount));
				strValue = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_414002);
				strValue.Replace(L"XXX", strTemp);
				m_pDevCmn->fnSCR_DisplayString(2, strValue);	
			}

			m_pDevCmn->fnSCR_DisplayScreen(414, ADA_SCREEN_TIMEOUT, PIN_MENU_MODE);		// 입금대기는 30초가 짧아서 1분 Timeout 적용

			bStartCash_OK = H_NH_DM_StartCashIn();

			// BNA FLICKER ON
			if (bStartCash_OK == true)
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_BNA, FLICKER_ON);

			bShowScreen = FALSE;
		}

		if (bStartCash_OK == true)
		{
			// Listen for KeyPad
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				CString GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString();
				NHDBG((L"GetKeyString(): [%s]\n", GetKeyStr));

				// [RWC6-676] SKKim 2024.05.30 Confirm화면이 표시되는 사이에 고객이 지폐 입금 방지를 위해 Cancel후 Confirm화면을 표시하도록 수정
				m_pDevCmn->fnBNA_CancelAccept();
				H_NH_DM_ReturnCash();


				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					if (P_NH_TangoPay_CancelConfirmation() == RES_NG)	// (TangoPay TODO:)
					{
						nRes = RES_USER_EXIT;
						g_sBizFlowInfo.nReasonforCancel = nRes;
						m_sTangoPayTransData.strErrorReason = L"(AC)USER CANCELLED";
						NVDump(NVP1_ERROR, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ACCEPT_CASH, L"", L"AC_CAN");
						break;
					}
				}
				else if (GetKeyStr == S_TIMEOVER)
				{
					if (P_NH_TangoPay_CancelConfirmation() == RES_NG)	// (TangoPay TODO:)
					{
						nRes = RES_USER_TIMEOUT;
						g_sBizFlowInfo.nReasonforCancel = nRes;
						m_sTangoPayTransData.strErrorReason = L"(AC)TIME OVER";
						NVDump(NVP1_ERROR, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ACCEPT_CASH, L"", L"AC_T_O");
						break;
					}
				}

				bShowScreen = TRUE;
				Delay_Msg(50);
				continue;
			}
			
			// Listen for cash acceptor
			if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_BNA))
			{
				NHDBG((L"fnAPL_GetDeviceEvent(DEV_BNA): Detect BNA Event\n"));

				// BNA FLICKER OFF
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_BNA, FLICKER_OFF);

				// Check device event
				if (m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BNA))
				{
					// Refuse인 경우에는 Take Cash할때까지 대기하도록 로직 보완 필요
					if (m_pDevCmn->fstrBNA_GetStackerStatus() != L"EMPTY")
					{
						// Get the amount collected
						nCurDepositAmount = m_pDevCmn->fnBNA_GetLastCashInValue();

						nTempAmountCollected = nAmountCollected + nCurDepositAmount;

						NHDBG((L"Deposited Amount (%d), Collected Amount : %d, Target Amount : %d\n", nCurDepositAmount, nTempAmountCollected, nTotalDepositAmount));

						if (nTempAmountCollected <= nTotalDepositAmount)
						{
							// Store Cash중 장애가 나도 일단 입금된 금액으로 처리 후 후 정산하도록 함
							nAmountCollected = nTempAmountCollected;
							m_sTangoPayTransData.strDepositedAmount = Int2Asc(nAmountCollected);

							// Store Cash전 Please Wait 화면 표시
							m_pDevCmn->fnSCR_DisplayScreen(415);

							// Store the cash
							if (H_NH_DM_StoreCash() == false)
							{
								NHDBG((L"StoreCash Error\n"));

								nRes = RES_NG;
								g_sBizFlowInfo.nReasonforCancel = nRes;
								strError = m_pDevCmn->fstrBNA_GetErrorCode();
								m_sTangoPayTransData.strErrorReason.Format(L"(AC_4) %s", strError);
								NHDBG((L"StoreCash Error(%s)\n", m_sTangoPayTransData.strErrorReason));
								NVDump(NVP1_ERROR, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ACCEPT_CASH, strError, L"AC_ERR4");

								LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_414301), ABORT_SCR_TIMEOUT);
								return nRes;
							}

							if (nTempAmountCollected == nTotalDepositAmount)
							{
								NHDBG((L"Accept Cash OK\n"));
								return RES_OK;
							}
						}
						else
						{
							//  금액이.초과되었습니다  정확한 금액 을.넣어주세요
							LIB_UserPopUpNotice(SCR_ICON_INFO, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_414201), 0 , 0, true);

							// 입력 금액이 받을려고하는 금액보다 큰 경우 Rollback 처리 후 다시 재입금 대기하도록 함.
							if (H_NH_DM_ReturnCash() == false)
							{
								// cash error시 거래 종료 처리
								nRes = RES_NG;
								g_sBizFlowInfo.nReasonforCancel = nRes;
								strError = m_pDevCmn->fstrBNA_GetErrorCode();
								m_sTangoPayTransData.strErrorReason.Format(L"(AC_5) %s", strError);
								NHDBG((L"Return Cash Error(%s)\n", m_sTangoPayTransData.strErrorReason));
								NVDump(NVP1_ERROR, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ACCEPT_CASH, strError, L"AC_ERR5");

								LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_414301), ABORT_SCR_TIMEOUT);
								break;
							}
						}
					}
					else
					{
						if (m_pDevCmn->fbBNA_GetRefusedStatus() == FALSE)
						{
							strError = m_pDevCmn->fstrBNA_GetErrorCode();
							NHDBG((L"Accept In Cash Error(%s)\n", strError));

							nRes = RES_NG;
							g_sBizFlowInfo.nReasonforCancel = nRes;
							m_sTangoPayTransData.strErrorReason.Format(L"(AC_10) %s", strError);
							NVDump(NVP1_ERROR, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ACCEPT_CASH, strError, L"AC_ERR10");
							LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_414301), ABORT_SCR_TIMEOUT);
							break;
						}

						// 지폐 refuse인 경우에 대한 처리 추가
						bBillCondition_OK = P_NH_WaitforFrontBillRemoval();
						NHDBG((L"Refused - bill condition(%d)\n", bBillCondition_OK));

						m_pDevCmn->fnBNA_ClearErrorCode();
						m_pDevCmn->fnBNA_CancelAccept();
						H_NH_DM_ReturnCash();
					}
				}
				else
				{
					strError = m_pDevCmn->fstrBNA_GetErrorCode();

					if (m_pDevCmn->fbBNA_GetRefusedStatus() == FALSE)
					{
						// Refused가 아닌 경우에 9770195는 지폐가 투입구에 있을 때 발생하는 case이므로 다시 시도하도록 로직 추가
						if (strError != L"9770195")
						{
							NHDBG((L"Accept In Cash Error(%s)\n", strError));

							nRes = RES_NG;
							g_sBizFlowInfo.nReasonforCancel = nRes;
							m_sTangoPayTransData.strErrorReason.Format(L"(AC_3) %s", strError);
							NVDump(NVP1_ERROR, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ACCEPT_CASH, strError, L"AC_ERR3");
							LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_414301), ABORT_SCR_TIMEOUT);
							break;
						}
					}

					bBillCondition_OK = P_NH_WaitforFrontBillRemoval();
					NHDBG((L"Refused - bill condition 2(%d)\n", bBillCondition_OK));

					m_pDevCmn->fnBNA_ClearErrorCode();
					m_pDevCmn->fnBNA_CancelAccept();
					H_NH_DM_ReturnCash();

					NHDBG((L"Accept In Cash Error - clear error - retry start in  cash\n"));
				}
				
				bShowScreen = TRUE;
			}
		}
		else
		{
			if (m_pDevCmn->fbBNA_GetRefusedStatus() == FALSE)
			{
				strError = m_pDevCmn->fstrBNA_GetErrorCode();

				// BNA Device Error로 처리
				nRes = RES_NG;
				g_sBizFlowInfo.nReasonforCancel = nRes;
				m_sTangoPayTransData.strErrorReason.Format(L"(AC_2) %s", strError);
				NVDump(NVP1_ERROR, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ACCEPT_CASH, strError, L"AC_ERR2");

				NHDBG((L"Start In Cash Error(%s)\n", m_sTangoPayTransData.strErrorReason));
				LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_414301), ABORT_SCR_TIMEOUT);
				break;
			}

			bBillCondition_OK = P_NH_WaitforFrontBillRemoval();
			NHDBG((L"Refused - bill condition 3(%d)\n", bBillCondition_OK));

			m_pDevCmn->fnBNA_ClearErrorCode();
			m_pDevCmn->fnBNA_CancelAccept();
			H_NH_DM_ReturnCash();

			bShowScreen = TRUE;
		}

		Delay_Msg(50);
	}

	m_pDevCmn->fnBNA_CancelAccept();
	H_NH_DM_ReturnCash();

	NHDBG((_T("[P_NH_TangoPay_AcceptCashProcess] End\n")));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ACCEPT_CASH, L"END", L"ACCEPT_CASH");

	return nRes;
}


BIZ_RETURN CTranCmn::P_NH_TangoPay_TransactionSummary()
{
	NHDBG((L"[P_NH_TangoPay_TransactionSummary]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_TRANS_SUMMARY, NVP4_NONE, L"T_S");

	BIZ_RETURN	nRes = RES_OK;
	CString		GetKeyStr;
	CString		strValue, strDispAmount;
	BOOL		bShowScreen = TRUE;

	// ADA모드는 일단 미지원으로 처리
	if( IsAdaTransaction() )
		return RES_USER_EXIT;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_NG;

	NHDBG((L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// End of [#2413]

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDBG((_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(416);

			// Setting Screen
			{
				// TangoPay Logo
				//m_pDevCmn->fnSCR_DisplayString(20, m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_TOPCONER));

				// TextID defined in Ap.dat file

				// Send to
				strValue = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_416002);
				m_pDevCmn->fnSCR_DisplayString(1, strValue);

				strValue = m_sTangoPayTransData.strReceiveName;
				m_pDevCmn->fnSCR_DisplayString(2, strValue);

				// Total Received
				//strDispAmount = m_sTangoPayTransData.strDepositedAmount + L"00";
				//strDispAmount.Remove('.');
				//strValue.Format(L"%s %s", m_sTangoPayTransData.strSendCurrency, MakeMoneyCent(strDispAmount));	// Cent단위로 표시
				strValue = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_416003);
				m_pDevCmn->fnSCR_DisplayString(3, strValue);

				strDispAmount = m_sTangoPayTransData.strDepositedAmount + L"." + L"00";
				strValue.Format(L"%s %s", m_sTangoPayTransData.strSendCurrency, strDispAmount);	// Cent단위로 표시
				m_pDevCmn->fnSCR_DisplayString(4, strValue);

				// Send Fees
				strDispAmount = m_sTangoPayTransData.strSendFees;
				//strDispAmount.Remove('.');
				//strValue.Format(L"%s %s", m_sTangoPayTransData.strSendCurrency, MakeMoneyCent(strDispAmount));
				strValue = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_416004);
				m_pDevCmn->fnSCR_DisplayString(5, strValue);

				strValue.Format(L"%s %s", m_sTangoPayTransData.strSendCurrency, strDispAmount);
				m_pDevCmn->fnSCR_DisplayString(6, strValue);

				int nValueIndex = 6;
				// [RWC6-676] SKKim 2024.05.28 Send Tax 추가 (금액이 있는 경우에만표시만
				if (Asc2Int(m_sTangoPayTransData.strSendTaxes) > 0)
				{
					nValueIndex++;
					strValue = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_416005);
					m_pDevCmn->fnSCR_DisplayString(nValueIndex, strValue);

					nValueIndex++;
					strDispAmount = m_sTangoPayTransData.strSendTaxes;
					strValue.Format(L"%s %s", m_sTangoPayTransData.strSendCurrency, strDispAmount);
					m_pDevCmn->fnSCR_DisplayString(nValueIndex, strValue);
				}

				// Send Amount (TangoPay TODO:) 금액 단위 정보가 없음 - 현재는 cent가 포함되어 내려옴
				nValueIndex++;
				strValue = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_416006);
				m_pDevCmn->fnSCR_DisplayString(nValueIndex, strValue);

				nValueIndex++;
				strDispAmount = m_sTangoPayTransData.strSendAmount;
				//strDispAmount.Remove('.');
				//strValue.Format(L"%s %s", m_sTangoPayTransData.strSendCurrency, MakeMoneyCent(strDispAmount));
				strValue.Format(L"%s %s", m_sTangoPayTransData.strSendCurrency, strDispAmount);
				m_pDevCmn->fnSCR_DisplayString(nValueIndex, strValue);

				// Receive Method
				nValueIndex++;
				strValue = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_416007);
				m_pDevCmn->fnSCR_DisplayString(nValueIndex, strValue);

				nValueIndex++;
				strValue = m_sTangoPayTransData.strReceiveMethod;
				m_pDevCmn->fnSCR_DisplayString(nValueIndex, strValue);

				// Total to Receiver (TangoPay TODO:) 금액 단위 정보가 없음 - Mexico인 경우 cent가 없이 내래옴
				nValueIndex++;
				strValue = m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_416008);
				m_pDevCmn->fnSCR_DisplayString(nValueIndex, strValue);

				nValueIndex++;
				strDispAmount = m_sTangoPayTransData.strReceiveAmount;
				//strValue.Format(L"%s %s", m_sTangoPayTransData.strReceiveCurrency, MakeMoneyAmount(strDispAmount));
				strValue.Format(L"%s %s", m_sTangoPayTransData.strReceiveCurrency, strDispAmount);
				m_pDevCmn->fnSCR_DisplayString(nValueIndex, strValue);
			}

			m_pDevCmn->fnSCR_DisplayScreen(416, KEYIN_TIME_OUT, PIN_MENU_MODE);

			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDBG((L"GetKeyString(): [%s]\n", GetKeyStr));
			
			// 고객이 Cancel을 하는 경우 Stack된 Cash를 돌려줄수 없기때문에 무조건 OK 처리
			return RES_OK;
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_TRANS_SUMMARY, L"TIMEOUT", L"T_S_COMMIT");

	return nRes;
}



BIZ_RETURN CTranCmn::P_NH_TangoPay_Commit()
{
	NHDBG((L"[P_NH_TangoPay_Commit]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_COMMIT, NVP4_NONE, L"P_COMMIT");

	BIZ_RETURN nRes = RES_NG;

	// Display Screen
	NHDBG((_T("SHOW SCREEN 116\n")));

	///////////////////////////////////
	// DISPLAY SCREEN

	m_pDevCmn->fnSCR_DisplayPrevSet(116);

	// Setting Screen
	{
		// Sub Title
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116001));

		// Under Guide
		m_pDevCmn->fnSCR_DisplayString(5, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_116002));
	}

	m_pDevCmn->fnSCR_DisplayScreen(116);

	// commit
	TangoPaySendMoneyCommitRequest request;
	TangoPaySendMoneyCommitResponse response;

	request.eid = m_sTangoPayTransData.strAccessToken;

	request.TransactionId = m_sTangoPayTransData.strTransactionId;
	request.MgiTransactionSessionId = m_sTangoPayTransData.strMgiTransactionSessionId;

	if (m_pTangoPayService->CommitRequest(request, response) == true)
	{
		m_sTangoPayTransData.strReferenceNumber = response.referenceNumber;

		NHDBG((L"[P_NH_TangoPay_Commit CommitRequest - ReferenceNumber : %s]\n", response.referenceNumber));
		NHDBG((L"[P_NH_TangoPay_Commit CommitRequest - ReferenceNumberTextCode : %s]\n", response.referenceNumberTextCode));
		NHDBG((L"[P_NH_TangoPay_Commit CommitRequest - ReferenceNumberText : %s]\n", response.referenceNumberText));
		NHDBG((L"[P_NH_TangoPay_Commit CommitRequest - ReferenceNumberConsumerText : %s]\n", response.referenceNumberConsumerText));

		int nAgentReceiptsCount = response.agentreceipts_array.GetCount();
		AgentReceipt temp_AgentReceipt;
		for(int i=0; i<nAgentReceiptsCount; i++)
		{
			POSITION pos = response.agentreceipts_array.FindIndex(i);
			temp_AgentReceipt = response.agentreceipts_array.GetAt(pos);
			m_sTangoPayTransData.strArr_AgentReceipts.Add(temp_AgentReceipt.contents);
		}

		NHDBG((L"[P_NH_TangoPay_Commit CommitRequest - agentreceipts_array Count(%d)]\n", m_sTangoPayTransData.strArr_AgentReceipts.GetCount()));

		int nConsumerReceiptsCount = response.consumerreceipts_array.GetCount();
		ConsumerReceipt temp_ConsumerReceipt;
		for(int i=0; i<nConsumerReceiptsCount; i++)
		{
			POSITION pos = response.consumerreceipts_array.FindIndex(i);
			temp_ConsumerReceipt = response.consumerreceipts_array.GetAt(pos);
			m_sTangoPayTransData.strArr_ConsumerReceipts.Add(temp_ConsumerReceipt.contents);
		}
		NHDBG((L"[P_NH_TangoPay_Commit CommitRequest - consumerreceipts_array Count(%d)]\n", m_sTangoPayTransData.strArr_ConsumerReceipts.GetCount()));

		nRes = RES_OK;
	}
	else
	{
		NHDBG((L"[P_NH_TangoPay_Commit: CommitRequest NG]\n"));

		NVDump(NVP1_ERROR, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_COMMIT, NVP4_NONE, L"COMMIT_NG");

		// [RWC6-676] SKKim 2024.05.28 Host Error 추가
		if (response.resultinfo.strStatusCode != L"200")
		{
			m_sTangoPayTransData.strErrorReason.Format(L"Commit failure(%s)", response.resultinfo.strStatusCode);
			P_NH_TangoPay_DisplayHostError(response.resultinfo.strStatusCode, response.resultinfo.strStatusMsg);
		}
		else
		{
			m_sTangoPayTransData.strErrorReason.Format(L"Commit failure(%s)", response.resultinfo.ErrCode);
			P_NH_TangoPay_DisplayHostError(response.resultinfo.ErrCode, response.resultinfo.ErrMessage);
		}

		nRes = RES_NG;
		return nRes;
	}

	return nRes;
}


BIZ_RETURN CTranCmn::P_NH_TangoPay_TransactionCompleteData()
{
	NHDBG((L"[P_NH_TangoPay_TransactionCompleteData]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_TRANS_COMP, NVP4_NONE, L"TRANS_COMP");

	BIZ_RETURN	nRes = RES_NG;
	CString		GetKeyStr;
	CString		strValue, strDispTotalAmount, strDispRecvAmount;
	BOOL		bShowScreen = TRUE;

	// ADA모드는 일단 미지원으로 처리
	if( IsAdaTransaction() )
		return RES_USER_EXIT;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_NG;

	NHDBG((L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// End of [#2413]

	strDispTotalAmount = m_sTangoPayTransData.strTotalAmount;
	//strDispTotalAmount.Remove('.');

	strDispRecvAmount = m_sTangoPayTransData.strReceiveAmount;

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDBG((_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(417);

			// Setting Screen
			{
				// TangoPay Logo
				//m_pDevCmn->fnSCR_DisplayString(20, m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_TOPCONER));

				// TextID defined in Ap.dat file

				// Reference Number
				m_pDevCmn->fnSCR_DisplayString(1, m_sTangoPayTransData.strReferenceNumber);

				// Sent To Name
				m_pDevCmn->fnSCR_DisplayString(2, m_sTangoPayTransData.strReceiveName);

				// Total Amount to Deposit
				//strValue.Format(L"%s %s", m_sTangoPayTransData.strSendCurrency, MakeMoneyCent(strDispTotalAmount));
				strValue.Format(L"%s %s", m_sTangoPayTransData.strSendCurrency, strDispTotalAmount);
				m_pDevCmn->fnSCR_DisplayString(3, strValue);

				// Total to Receiver
				//strValue.Format(L"%s %s", m_sTangoPayTransData.strReceiveCurrency, MakeMoneyAmount(strDispRecvAmount));
				strValue.Format(L"%s %s", m_sTangoPayTransData.strReceiveCurrency, strDispRecvAmount);
				m_pDevCmn->fnSCR_DisplayString(4, strValue);
			}

			m_pDevCmn->fnSCR_DisplayScreen(417, KEYIN_TIME_OUT, PIN_MENU_MODE);

			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (GetKeyStr == L"RECEIPT")
			{
				NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_TRANS_COMP, NVP4_NONE, L"S_PRINT");
				return RES_TANGOPAY_PRINTRECEIPT;
			}
			else
			{
				// No Receipt로 처리 - Cancel Key도 No Receipt로 처리
				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_TRANS_COMP, L"TIMEOUT", L"TRANS_COMP");

	return nRes;
}

BIZ_RETURN CTranCmn::P_NH_TangoPay_ErrorPrintReceipt()
{
	NHDBG((L"[P_NH_TangoPay_ErrorPrintReceipt]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ERRORPRINT, NVP4_NONE, L"PRINT");

	BIZ_RETURN	nRes = RES_NG;
	CString strPrintData, strTextNo, strData;

	if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
	{
		NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ERRORPRINT, NVP4_NONE, L"ERR_PRT_1");

		LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119201), INFO_SCR_TIMEOUT);
		return RES_NG;
	}

	NHDBG((_T("SHOW SCREEN 119\n")));
	m_pDevCmn->fnSCR_DisplayPrevSet(119);

	// Setting Screen
	{
		// Sub Title
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119001));
	}

	// NH2600 / MX2600SE인 경우 Receipt Flicker가 존재하지 않아 SW적으로 위치 표시
#if(US_VERSION || CA_VERSION || MX_VERSION)
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
		m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
	else
		m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
#endif

	m_pDevCmn->fnSCR_DisplayScreen(119);

	// Make Error Print Data (Journal에 저장하는 값을 인자하도록 임시로 사양 정함) (TangoPay TODO:)

	// Terminal ID
	// 1. Terminal ID
	strData.Format(L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_001), m_sTangoPayTransData.strTerminalID);
	AddPrintData(strPrintData, strData);

	int nDataIndex = 10;

	// 10. Partner ID
	strTextNo.Format(L"%02d", nDataIndex);
	strData.Format(L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTextNo), m_sTangoPayTransData.strPartnerID);
	AddPrintData(strPrintData, strData);

	// 11. Phone Number
	nDataIndex++;
	strTextNo.Format(L"%02d", nDataIndex);
	strData.Format(L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTextNo), m_sTangoPayTransData.strPhoneNumber);
	AddPrintData(strPrintData, strData);

	// 12. Confirmation No
	nDataIndex++;
	strTextNo.Format(L"%02d", nDataIndex);
	strData.Format(L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTextNo), m_sTangoPayTransData.strConfirmationNumber);
	AddPrintData(strPrintData, strData);

	// 13. Reference No
	nDataIndex++;
	strTextNo.Format(L"%02d", nDataIndex);
	strData.Format(L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTextNo), m_sTangoPayTransData.strReferenceNumber);
	AddPrintData(strPrintData, strData);

	// 14. Total Amount (Server Received with '.' Cent)
	nDataIndex++;
	strTextNo.Format(L"%02d", nDataIndex);
	strData.Format(L"%-17.17s = %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTextNo), m_sTangoPayTransData.strSendCurrency, m_sTangoPayTransData.strTotalAmount);
	AddPrintData(strPrintData, strData);

	// 15. Deposited Amount
	nDataIndex++;
	strTextNo.Format(L"%02d", nDataIndex);
	strData.Format(L"%-17.17s = %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTextNo), m_sTangoPayTransData.strSendCurrency, m_sTangoPayTransData.strDepositedAmount + L".00");
	AddPrintData(strPrintData, strData);

	// 16. TextID 없음
	nDataIndex++;

	// 17. Send Amount with cent
	nDataIndex++;
	strTextNo.Format(L"%02d", nDataIndex);
	strData.Format(L"%-17.17s = %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTextNo), m_sTangoPayTransData.strSendCurrency, m_sTangoPayTransData.strSendAmount);
	AddPrintData(strPrintData, strData);

	// 18. Send Fees with cent
	nDataIndex++;
	strTextNo.Format(L"%02d", nDataIndex);
	strData.Format(L"%-17.17s = %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTextNo), m_sTangoPayTransData.strSendCurrency, m_sTangoPayTransData.strSendFees);
	AddPrintData(strPrintData, strData);

	// [RWC6-676] Start SKKim 2024.05.30 19. Taxes Field 추가
	nDataIndex++;
	if (Asc2Int(m_sTangoPayTransData.strSendTaxes) > 0)
	{
		strTextNo.Format(L"%02d", nDataIndex);
		strData.Format(L"%-17.17s = %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTextNo), m_sTangoPayTransData.strSendCurrency, m_sTangoPayTransData.strSendTaxes);
		AddPrintData(strPrintData, strData);
	}

	// 20. Receive Amount (without cent)
	nDataIndex++;
	strTextNo.Format(L"%02d", nDataIndex);
	strData.Format(L"%-17.17s = %s %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTextNo), m_sTangoPayTransData.strReceiveCurrency, m_sTangoPayTransData.strReceiveAmount);
	AddPrintData(strPrintData, strData);

	// 21. TextID 없음
	nDataIndex++;

	// 22. Receive Country
	nDataIndex++;
	strTextNo.Format(L"%02d", nDataIndex);
	strData.Format(L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTextNo), m_sTangoPayTransData.strReceiveCountry);
	AddPrintData(strPrintData, strData);

	// 23. Receive Name
	nDataIndex++;
	strTextNo.Format(L"%02d", nDataIndex);
	strData.Format(L"%-17.17s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTextNo), m_sTangoPayTransData.strReceiveName);
	AddPrintData(strPrintData, strData);

	// 24. Result
	nDataIndex++;
	strTextNo.Format(L"%02d", nDataIndex);
	strData.Format(L"%-10.10s = %s", m_pDevCmn->fstrSCR_GetStringFromTextID(T_MSG_JNL_TP + strTextNo), m_sTangoPayTransData.strErrorReason);
	AddPrintData(strPrintData, strData);

	// 25. 수동 정산을 위한 고객 Guide 문구 추가 필요 (TODO:)

	m_pDevCmn->fnSPR_PrintReceipt(TRUE, strPrintData, K_1_WAIT);

	// Print 후 SPR 장애시 화면 추가
	if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
	{
		NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ERRORPRINT, NVP4_NONE, L"ERR_PRT_1");

		LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119201), ABORT_SCR_TIMEOUT);
	}

	NHDBG((_T("SHOW SCREEN 120\n")));
	m_pDevCmn->fnSCR_DisplayPrevSet(120);

	// Setting Screen
	{
		// Sub Title
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_120001));
	}

	// NH2600 / MX2600SE인 경우 Receipt Flicker가 존재하지 않아 SW적으로 위치 표시
	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
#if(US_VERSION || CA_VERSION || MX_VERSION)			
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")	
		m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
	else
		m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
#endif
	// end of [#2327]

	m_pDevCmn->fnSCR_DisplayScreen(120);

	///////////////////////////////////
	// WAIT FOR CUSTOMER
	m_pDevCmn->fstrSCR_WaitTime(2);		// 너무 길어 2초로 변경

	// Print 장애시 화면 추가

	return nRes;
}


BIZ_RETURN CTranCmn::P_NH_TangoPay_PrintTransactionReceipt()
{
	NHDBG((L"[P_NH_TangoPay_PrintTransactionReceipt]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_PRINT_RECEIPT, NVP4_NONE, L"PRINT");

	BIZ_RETURN	nRes = RES_NG;

	if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
	{
		NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_PRINT_RECEIPT, NVP4_NONE, L"PRINT_ERR_1");

		LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119201), INFO_SCR_TIMEOUT);
		return RES_NG;
	}

	NHDBG((_T("SHOW SCREEN 119\n")));
	m_pDevCmn->fnSCR_DisplayPrevSet(119);

	// Setting Screen
	{
		// Sub Title
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119001));
	}

	// [#2327] NH KSK 2015.01.28
	// NH2600 / MX2600SE인 경우 Receipt Flicker가 존재하지 않아 SW적으로 위치 표시
#if(US_VERSION || CA_VERSION || MX_VERSION)			// [#2338] US Justin 2015.03.31 Enable HALO2 on Canadian and Mexican Versions
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
		m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
	else
		m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
#endif
	// end of [#2327]

	m_pDevCmn->fnSCR_DisplayScreen(119);

	CString strPrintData, strValue, strTemp;
	CStringArray strArr_PrintData;

	int nDataCount = m_sTangoPayTransData.strArr_ConsumerReceipts.GetCount();

	strArr_PrintData.RemoveAll();
	if (nDataCount > 0)
	{
		strValue = m_sTangoPayTransData.strArr_ConsumerReceipts.GetAt(0);
		SplitString(strValue, L"\r\n", strArr_PrintData);
	}

	nDataCount = strArr_PrintData.GetCount();

	strPrintData.Empty();
	strValue.Empty();

	for(int i=1; i<=nDataCount; i++)
	{
		strValue = strArr_PrintData.GetAt(i-1);
		strValue.Remove('\r');
		strValue.Remove('\n');

		if (strValue.GetLength() > 0)
		{
			// 40line이 넘어가는 경우는 40line만 print하도록 임시 수정
			if(strValue.GetLength() > 40)
			{
				NHDBG((_T("Print line count is over (%s)\n"), strValue));

				strTemp = strValue;
				strValue = strTemp.Left(40);
				AddPrintData(strPrintData, strValue);

				strValue = strTemp.Right(strTemp.GetLength() - 40);
			}

			AddPrintData(strPrintData, strValue);
			//NHDBG((_T("Print Data (%d)(%s)\n"), i, strValue));
		}
	}

	if (strPrintData.GetLength() > 0)
		m_pDevCmn->fnSPR_PrintReceipt(TRUE, strPrintData, K_1_WAIT);

	// Print 후 SPR 장애시 화면 추가
	if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
	{
		NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_ERRORPRINT, NVP4_NONE, L"ERR_PRT_1");

		LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_119201), ABORT_SCR_TIMEOUT);
	}

	NHDBG((_T("SHOW SCREEN 120\n")));
	m_pDevCmn->fnSCR_DisplayPrevSet(120);

	// Setting Screen
	{
		// Sub Title
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_120001));
	}

	// NH2600 / MX2600SE인 경우 Receipt Flicker가 존재하지 않아 SW적으로 위치 표시
	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
#if(US_VERSION || CA_VERSION || MX_VERSION)			
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")	
		m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
	else
		m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
#endif
	// end of [#2327]

	m_pDevCmn->fnSCR_DisplayScreen(120);

	///////////////////////////////////
	// WAIT FOR CUSTOMER
	m_pDevCmn->fstrSCR_WaitTime(2);		// 너무 길어 2초로 변경

	return nRes;
}


BIZ_RETURN CTranCmn::P_NH_TangoPay_CancelConfirmation()
{
	NHDBG((L"[P_NH_TangoPay_CancelConfirmation]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_CANCELCONFIRM, NVP4_NONE, L"C_CONF");

	BIZ_RETURN	nRes = RES_NG;
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// ADA모드는 일단 미지원으로 처리
	if( IsAdaTransaction() )
		return RES_USER_EXIT;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_NG;

	NHDBG((L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDBG((_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(419);

			// Setting Screen
			{
				// TangoPay Title Warning Message
				//m_pDevCmn->fnSCR_DisplayString(20, m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_TOPCONER));

				// TextID defined in Ap.dat file

			}

			m_pDevCmn->fnSCR_DisplayScreen(419, KEYIN_TIME_OUT, PIN_MENU_MODE);

			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			if (GetKeyStr == L"YES")
			{
				NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_CANCELCONFIRM, NVP4_NONE, L"S_YES");
				return RES_NG;
			}
			else
			{
				// YES가 아닌 경우 거래가 취소되지 않도록 OK 처리
				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_CANCELCONFIRM, L"TIMEOUT", L"C_CONF");

	return nRes;
}
BIZ_RETURN CTranCmn::P_NH_TangoPay_DownloadAppGuideScreen()
{
	NHDBG((L"[P_NH_TangoPay_DownloadAppGuideScreen]\n"));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_DOWN_APP_GUIDE, NVP4_NONE, L"D_A_G");

	BIZ_RETURN	nRes = RES_NG;
	CString		GetKeyStr;
	BOOL		bShowScreen = TRUE;

	// ADA모드는 일단 미지원으로 처리
	if( IsAdaTransaction() )
		return RES_USER_EXIT;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)									// ADA Mode....Jack removed.
		return RES_NG;

	NHDBG((L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);

	// Make QR Code for Download App address
	// Delete Existing QR Image
	DeleteFile(TANGOPAY_QR_DOWNLOAD_APP_INFO);		

	// Create QR Image
	MakeQRCodeImageFile(m_TangoPayConfig.strDownloadAppInfo, TANGOPAY_QR_DOWNLOAD_APP_INFO);
	if (!IsExistFile(TANGOPAY_QR_DOWNLOAD_APP_INFO))
	{
		NHDEBUG(DBG_CALL, (L"ERROR on Finding QR Image file [%s] RETURN RES_NG\n", TANGOPAY_QR_DOWNLOAD_APP_INFO));
		return RES_NG;
	}

	NHDBG((L"Found QR Image file [%s]\n", TANGOPAY_QR_DOWNLOAD_APP_INFO));

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDBG((_T("SHOW SCREEN\n")));

			m_pDevCmn->fnSCR_DisplayPrevSet(420);

			// Setting Screen
			{
				// TangoPay Title Warning Message
				//m_pDevCmn->fnSCR_DisplayString(20, m_Pin4.GetPin4FileName(PIN4_FILE_LOGO_TOPCONER));

				// TextID defined in Ap.dat file
				m_pDevCmn->fnAPL_DisplayQRCodeOnScreen(TANGOPAY_QR_DOWNLOAD_APP_INFO);

			}

			m_pDevCmn->fnSCR_DisplayScreen(420, KEYIN_TIME_OUT, PIN_MENU_MODE);

			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			// 아무 Key나 Timeout이나 받은 경우 OK 처리 (거래 취소를 위해)
			return RES_OK;
		}

		Delay_Msg(50);
	}

	NHDBG((_T("MAX TIMEOUT\n")));
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_DOWN_APP_GUIDE, L"TIMEOUT", L"C_CONF");


	return nRes;
}


BOOL CTranCmn::TangoPay_WriteJournalEntry(CString strJnlCode)
{
	CString strJnlData;
	CString strTemp;

	// Common Data
	// 1. Terminal ID
	strJnlData.AppendFormat(L"%s", m_sTangoPayTransData.strTerminalID);
	
	// 2. Partner ID
	strJnlData.AppendFormat(L"^%s", m_sTangoPayTransData.strPartnerID);

	// 3. Phone Number
	strJnlData.AppendFormat(L"^%s", m_sTangoPayTransData.strPhoneNumber);

	// 4. Confirmation Number
	strJnlData.AppendFormat(L"^%s", m_sTangoPayTransData.strConfirmationNumber);

	// 5. Reference Number
	strJnlData.AppendFormat(L"^%s", m_sTangoPayTransData.strReferenceNumber);

	// 6. Total Amount (Server Received with '.' Cent)
	int nAmount = Asc2Int(m_sTangoPayTransData.strTotalAmount);
	strJnlData.AppendFormat(L"^%d", nAmount);

	// 7. Deposited Amount (without Cent)
	strTemp = m_sTangoPayTransData.strDepositedAmount + L"00";
	nAmount = Asc2Int(strTemp);
	strJnlData.AppendFormat(L"^%d", nAmount);

	// 8. Send Currency
	strJnlData.AppendFormat(L"^%s", m_sTangoPayTransData.strSendCurrency);

	// 9. Send Amount with cent
	nAmount = Asc2Int(m_sTangoPayTransData.strSendAmount);
	strJnlData.AppendFormat(L"^%d", nAmount);

	// 10. Send Fees with cent
	nAmount = Asc2Int(m_sTangoPayTransData.strSendFees);
	strJnlData.AppendFormat(L"^%d", nAmount);

	// [RWC6-676] SKKim 2024.05.29 Tax Field 추가
	// 11. Send Taxes with cent
	nAmount = Asc2Int(m_sTangoPayTransData.strSendTaxes);
	strJnlData.AppendFormat(L"^%d", nAmount);

	// 12. Receive Amount (without cent)
	nAmount = Asc2Int(m_sTangoPayTransData.strReceiveAmount);
	strJnlData.AppendFormat(L"^%d", nAmount);

	// 13. Receive Currency
	strJnlData.AppendFormat(L"^%s", m_sTangoPayTransData.strReceiveCurrency);

	// 14. Receive Country
	strJnlData.AppendFormat(L"^%s", m_sTangoPayTransData.strReceiveCountry);

	// 15. Receive Name
	strJnlData.AppendFormat(L"^%s", m_sTangoPayTransData.strReceiveName);

	// 16. Result
	if (strJnlCode == TANGOPAY_TXN)
	{
		strJnlData.AppendFormat(L"^%s", L"OK");
	}
	else
	{
		strJnlData.AppendFormat(L"^%s", m_sTangoPayTransData.strErrorReason);
	}

	NHDBG((_T("TangoPay_WriteJournalEntry JnlData : %s\n"), strJnlData));

	return m_pDevCmn->m_JNLMgr.Save(strJnlCode, strJnlData);
}

#endif