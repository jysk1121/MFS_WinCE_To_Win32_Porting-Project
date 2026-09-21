// DEVPINIF.cpp

#include "stdafx.h"

//#define NH_DEBUG
#include "..\NHMWI.h"
#include "..\NHMWICtl.h"
#include "..\NHMWIPpg.h"

#include ".\Common\NHDbgApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// PIN ActiveX BUGREPORT
// ------------------------------------------------------------------------------------------------
// 1. Entry관련처리...
//		pinpad는 Entry를 켜는 시점은 알수 있으나, Entry가 꺼지는 시점의 확인이 어렵다.
//		연속으로 몇자리의 데이타가 들어오다가 스스로 엔트리가 꺼지기 때문이다.
//		이를 처리하기 위해서는
//		1) MWI에서 EntryOn시 MaxCount를 저장하여 데이타가 올라올때마다 길이를 확인하고
//		   기타키를 확인하는등의 데이타에 대한 검증을 함
//		=> 처리사양이 복잡하고, AP로직에 의한 처리가 아닌 임의처리가 생긴다.
//		2) AP로부터 Entry Flag를 Off하라는 임의의 처리를 위임함
//		=> 현재의 사양으로는 최선이라 생각되나, AP의 번거로움이 생김..또한 사양의 확정이 필요.
// ------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call to PIN
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevPinOpenConnection()
{
	int nReturn = R_NORMAL;

	NHDEBUG(1, (_T("OpenSessionSync() ... \n")));
	nReturn = m_pPin.OpenSessionSync(K_30_WAIT*1000);
	NHDEBUG(1, (L"\n"));
	NHDEBUG(1, (_T("OpenSessionSync() ... DONE (%d)\n"), nReturn));
	
	if (nReturn == R_NORMAL)
	{
		// This is to remove a fatal error generated after successful OpenSessionSync(), which was executed to open SP which was killed by force by GOMA.
		m_pDevCtrl->EvtQReset(DEV_PIN);

		ProcSetDeviceEvent(L"PINPAD", L"OpenComplete", NULL);

		DevPinUpdateStatus();									// Status Check
		m_nPINEntryMode = PINMODE_NOT_USE;						// Pin enter MODE Reset
		m_bPinEntry = ENTRY_DISABLED;
		m_strPinEppCardData.Empty();
	}
	else //error
	{
		ProcSetDeviceEvent(L"PINPAD", L"FatalError", NULL);
	}
	
	return nReturn;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevPinCloseConnection()
{
NHDEBUG(1, (_T("CNHMWICtrl::DevPinCloseConnection()(%s) \n"), "  "));

	if (m_bPinOpened)
	{
		NHDEBUG(1, (_T("CNHMWICtrl::DevPinEntryDisable() BEFORE CloseSessionSync() (%s) \n"), "  "));
		DevPinEntryDisable();
	}

	NHDEBUG(1, (_T("CNHMWICtrl::CloseSessionSync() (%s) \n"), "  "));
	return m_pPin.CloseSessionSync();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : CString DevPinGetErrorCode() 
// DESCRIPT  : 레지스트리에 저장된 장애코드를 확보한다.
// PARAMETER : 없음
// RETURN    : 장애코드
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevPinGetErrorCode() 
{
	CString strResult("");

	char szTemp[10] = {};
	memcpy(szTemp, m_pSPInform->SPErr.ErrCode_PIN, 7);
	strResult = CString(szTemp);

//	TCHAR szCode[256];		memset(szCode, 0x00, sizeof(szCode));
//	int nResult = 0;
//
//	// 레지스트리에서 장애코드를 확보한다.
//	nResult = RegQueryValueExt(_REGKEY_DEVERROR, _T("PINPAD"), REG_STR, sizeof(szCode), (LPVOID)szCode);
//	if (nResult)
//	{
//		strResult.Format(_T("%S0000000"), szCode);					// 2004.06.19
//		strResult = strResult.Left(7);							// 에러코드보정	: 2004.06.19
//	}
	NHDEBUG(1, (_T("ErrCode_PIN(%s) \n"), strResult));
	return strResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long DevPinClearErrorCode() 
// DESCRIPT  : 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevPinClearErrorCode() 
{
NHDEBUG(1, (_T("CNHMWICtrl::DevPinClearErrorCode()(%s) \n"), "  "));
	RegSetValueExt(_REGKEY_DEVERROR, _T("PINPAD"), REG_STR, 0, NULL);
	return NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 장치디바이스 상태조회
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevPinGetDeviceStatus()
{
	int nResult = NORMAL;										// 2004.11.05
	CString strStatus = m_pDevCtrl->GetDeviceStatus(_T("PINPADStDeviceStatus"));

	if		(strStatus.CompareNoCase(ST_DEVONLINE)	== 0)	nResult = NORMAL;
	else if	(strStatus.CompareNoCase(ST_DEVNODEVICE)== 0)	nResult = NODEVICE;
	else	nResult = DOWN;

	return nResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : PINPAD를 사용가능하도록 처리한다.
// PARAMETER : nEnableMode			- 입력모드 (1-평문입력무도, 2-EPP입력모드)
//			   nMinKeyCount			- 최소입력갯수(UserEntry시에는 무의미)
//			   nMaxKeyCount			- 최대입력갯수
//			   bAutoEnd				- 최대입력시 자동Disable처리 여부
//			   szActiveKeys			- 입력가능 키문자열
//			   szTerminatorKeys		- 입력종료 가능 문자열
//			   LPCTSTR szCardData	- EPP시 사용될 카드데이타(USerEntry시에는 무의미)
//			   nTimeout				- 입력대기 타임(초단위)
// RETURN    : 처리불가시 R_ERROR(2), 이외에는 함수의 처리결과
// REMARK	 : 1. 입력대기모드가 1/2가 아닌 경우엔 처리가 불가하다. 이러한 경우
//				  979280b장애를 발생하도록 한다.
// DATE		 : 2004.04.02
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevPinEntryEnable(int		nEnableMode,  
								  int		nMinKeyCount, 
								  int		nMaxKeyCount, 
								  BOOL		bAutoEnd, 
								  LPCTSTR	szActiveKeys, 
								  LPCTSTR	szTerminatorKeys, 
								  LPCTSTR	szCardData, 
								  int		nTimeout		)
{
//	m_pDevCtrl->EvtQReset(DEV_PIN);								// [#2133] NH KSK 2012.05.07 Queue Reset 처리를 APPINIF에서 하도록 수정 (Disable 처리 에러시 보완을 위해)

	m_strPinLastKey.Empty();
	m_strPinKeyData.Empty();
	m_nPINEntryMode = nEnableMode;
	m_strPinEppCardData = szCardData;

	NHDEBUG(1, (L"m_strPinEppCardData [%s] \n", m_strPinEppCardData));

	switch(m_nPINEntryMode)
	{
	case PINMODE_NORMAL:
		m_pPin.ReadData( nMaxKeyCount,							// 최대입력키
						 bAutoEnd,								// 자동종료여부
						 szActiveKeys,							// 입력처리가능키문자열
						 szTerminatorKeys,						// 입력종료처리가능키문자열
						 (nTimeout < 0) ? nTimeout : nTimeout * 1000);	
																// 대기시간(0이하이면 0으로처리)
		break;

	case PINMODE_EPP:
		m_pPin.ReadPin(	 nMinKeyCount,							// 최소입력키
						 nMaxKeyCount,							// 최대입력키
						 bAutoEnd,								// 자동종료여부
						 szActiveKeys,							// 입력처리가능키문자열
						 szTerminatorKeys,						// 입력종료처리가능키문자열
						 (nTimeout < 0) ? nTimeout : nTimeout * 1000);	
																// 대기시간(0이하이면 0으로처리)
		break;

	default:
		// 상위국(AP)에서 Entry Mode를 위 두가지만 사용하므로 죽은 코드임
		m_pDevCtrl->WriteMwiErrorCode(DEV_PIN, "PINPAD", 7, "979280B");
		ProcSetDeviceEvent(L"PINPAD", L"FatalError", NULL);
		
		m_bPinEntry = ENTRY_DISABLED;					//-- SJKWONNOTE 2006-07-19 --ADD//
		return R_ERROR;
	}

	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 키입력대기 취소처리
// PARAMETER : X
// RETURN    : R_NORMAL(0)
// REMARK	 : 미사용모드인 경우에는 처리하지 않음
//			   단지, 처리동기화(CheckDeviceAction)을 위해서 필요한 이벤트처리를 한다.
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevPinEntryDisable()
{
	// [#2133] NH KSK 2012.05.07 변수 두개 비교 처리 오류부분에 대해서 로직 수정 및 로그 보완
//	if (m_bPinEntry == ENTRY_DISABLED)
//	{
//		ProcSetDeviceEvent(L"PINPAD", L"ReadDataCancelled", NULL);
//		ProcSetDeviceEvent(L"PINPAD", L"ReadPinCancelled",	NULL);
//	}
//	else
//	if (m_nPINEntryMode == PINMODE_NORMAL)
//	{
//		m_pPin.CancelReadData();
//	}
//	else
//	if (m_nPINEntryMode == PINMODE_EPP)
//	{
//		m_pPin.CancelReadPin();
//
//	}
//	else
//	/* if (m_nPINEntryMode == PINMODE_NOT_USE)					// PINMODE_NOT_USE時 Default*/
//	{
//		ProcSetDeviceEvent(L"PINPAD", L"ReadDataCancelled", NULL);
//		ProcSetDeviceEvent(L"PINPAD", L"ReadPinCancelled",	NULL);
//	}

	// 1. m_bPinEntry 변수에 대해 EN/DISAB 비교
	if (m_bPinEntry == ENTRY_DISABLED)
	{
		// 이전 상태가 Disable인 경우에 상위국(AP)에서 Disable Call시 MWI에서 Cancel 처리
		ProcSetDeviceEvent(L"PINPAD", L"ReadDataCancelled", NULL);
		ProcSetDeviceEvent(L"PINPAD", L"ReadPinCancelled",	NULL);
	}
	else
	{
		// 이전Pin 상태가 Enable인 경우에는 Disable 처리해야 하므로 Mode 판단
		if (m_nPINEntryMode == PINMODE_NORMAL)
		{
			m_pPin.CancelReadData();
		}
		else if (m_nPINEntryMode == PINMODE_EPP)
		{
			m_pPin.CancelReadPin();
		}
		else	 /* (m_nPINEntryMode == PINMODE_NOT_USE)	// PINMODE_NOT_USE時 Default */
		{
			// [#2133] NH KSK 2012.06.07 Mode 설정이 안된 상태에서 상위국(AP)에서 내려올 경우 MWI가 Cancel 처리하므로 로그 추가
			CString	strLog;
			strLog.Format(L"MWI_MODE_ERR");
			NVDump('F', '1', "99", L"9999999", strLog);
			// end of [#2133]

			ProcSetDeviceEvent(L"PINPAD", L"ReadDataCancelled", NULL);
			ProcSetDeviceEvent(L"PINPAD", L"ReadPinCancelled",	NULL);
		}
	}
	// end of [#2133]
	
//	m_nPINEntryMode = PINMODE_NOT_USE;							// Pin enter MODE Reset	[#2133] NH KSK 2012.05.07 Event 수신을 받을때만 PIN MODE 변경하도록 수정
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 입력된 키값을 조회
// PARAMETER : X
// RETURN    : NORMAL MODE - 입력된 키값 그대로
//			   EPP MODE	   - EPP처리가 된 키값
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevPinGetKeyData()
{
NHDEBUG(1, (_T("CNHMWICtrl::DevPinGetKeyData()(%s) \n"), m_strPinKeyData));
	m_pDevCtrl->CheckDeviceAction(DEV_PIN, K_30_WAIT);
	
	CString strReturn("");

	strReturn.Format(_T("%s"), m_strPinKeyData);
	m_strPinKeyData.Empty();

	return strReturn;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Macing
// PARAMETER : Unpacked original data
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevPinMacing(LPCTSTR szData)
{
NHDEBUG(1, (_T("CNHMWICtrl::DevPinMacing()(%s) \n"), L" "));
	m_strPinMacingData.Empty();

	int keyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);

	// [#554] KSK 2009.08.11 512K 지원
	if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) >= 0)
	{
		switch (keyMode)
		{
		case KEYMODE_NON_UNIQ_SDES_MACING:
		case KEYMODE_TDES_MACING:
			m_pPin.GenerateMAC(_T("DESMAC"), SINGLEMACKEY_WORKINGKEY_NAME, NULL, NULL, NULL, 0, szData);
			break;
		case KEYMODE_TDES_TMACING:
			m_pPin.GenerateMAC(_T("TRIDESMAC"), TRIPLEMACKEY_WORKINGKEY_NAME, NULL, NULL, NULL, 0, szData);
			break;
		//case KEYMODE_TR31_MACING:
			//m_pPin.GenerateMAC(_T("DESMAC"), TR31_ATM_TMACWORKINGKEY_NAME, NULL, NULL, NULL, 0, szData);
			//break;
		//case KEYMODE_TR31_TMACING:
			//m_pPin.GenerateMAC(_T("TRIDESMAC"), TR31_ATM_TMACWORKINGKEY_NAME, NULL, NULL, NULL, 0, szData);
			//break;
		}
	}
	else
	{
		m_pPin.GenerateMAC(_T("DESMAC"), _T("K4"), NULL, NULL, NULL, 15, szData);		// [#4] NH PSC 2008.03.11 Macing
	}
	// end of [#554]

	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 입력된 키값을 조회
// PARAMETER : X
// RETURN    : NORMAL MODE - 입력된 키값 그대로
//			   EPP MODE	   - EPP처리가 된 키값
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevPinGetMacingData()
{
	CString strReturn;
	strReturn.Format(_T("%s"), m_strPinMacingData);
	m_strPinMacingData.Empty();
	return strReturn;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Load IV Key
// PARAMETER : Key name to load
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevPinLoadIV(LPCTSTR szKeyName)
{
	CString strKeyName("");										// Get Key name & Truncate space
	strKeyName.Format(_T("%s"), szKeyName);
	
	if (strKeyName.IsEmpty())									// Check Keyname
		strKeyName.Format(_T("%S"), "Vector");

	return m_pPin.ImportKey(strKeyName, NULL, _T("0123456789ABCDEF"), _T("CRYPT,FUNCTION,MACING,KEYENCKEY"));
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Load Key
// PARAMETER : Key name to load, KeyValue
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevPinLoadKey(LPCTSTR szKeyName, LPCTSTR szKeyValue)
{

NHDEBUG(1, (_T("CNHMWICtrl::DevPinLoadKey()(%s)(%s) \n"), szKeyName, szKeyValue));
	int nValueLen = wcslen(szKeyValue);
	if (nValueLen != 16 && nValueLen != 32)
	{
		CString strErrorCode = "1030100";
		m_pDevCtrl->WriteMwiErrorCode(DEV_PIN, "PINPAD", strErrorCode.GetLength(), "1030100");
		ProcSetDeviceEvent(L"PINPAD", L"FatalError", NULL);
		return R_ERROR;
	}
	
	return m_pPin.ImportKey(szKeyName, NULL, szKeyValue, _T("CRYPT,FUNCTION,MACING,KEYENCKEY"));
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Load Encrypted Key - 512K시에만 사용함 [#554] KSK 2009.08.11
// PARAMETER : Key name to load, KeyValue, EncKeyName
// RETURN    : 
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevPinLoadEncryptedKey(LPCTSTR KeyUse, LPCTSTR szKeyName, LPCTSTR szKeyValue, LPCTSTR szEncKeyName)
{
NHDEBUG(1, (_T("CNHMWICtrl::DevPinLoadEncryptedKey() (%s) (%s) (%s) (%s) \n"), KeyUse, szKeyName, szKeyValue, szEncKeyName));
	int nValueLen = wcslen(szKeyValue);

	if (nValueLen != 16 && nValueLen != 32)
	{
		CString strErrorCode = "1030100";
		m_pDevCtrl->WriteMwiErrorCode(DEV_PIN, "PINPAD", strErrorCode.GetLength(), "1030100");
		ProcSetDeviceEvent(L"PINPAD", L"FatalError", NULL);
		return R_ERROR;
	}

	return m_pPin.ImportKey(szKeyName, szEncKeyName, szKeyValue, KeyUse);		// [#554] KSK 2009.08.10
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Pre Entry Disable
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevPinPreEntryDisable()
{
	if (m_bPinEntry == ENTRY_ENABLED)							// DiableEntry before EntryEnable
	{
		PinEntryDisable();
		if (m_pDevCtrl->CheckDeviceAction(DEV_PIN, K_30_WAIT) != NORMAL)
		{
			SetEventMatrix(DEV_PIN, "ReadData");
			ProcSetDeviceEvent(L"PINPAD", L"FatalError", NULL);
			return R_ERROR;
		}
	}

	return R_NORMAL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------------
// DESCRIPT  : 
// PARAMETER : 디바이스 상태와 매체 상태를 업데이트 한다.
// RETURN    : 
// REMARK	 : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevPinUpdateStatus()
{
	if (!m_bPinOpened)											// 장치가Open되지 않았으면
		return R_NORMAL;										// 상태갱신은 의미없음
	
	CString strStatus = m_pPin.GetDeviceStatus();				// 장치상태조회 : 장치가 정상일경우에만 기타조회
	ProcSetDeviceStatus(L"PINPAD", L"StDeviceStatus", strStatus);// 2004.11.05
	
	return R_NORMAL;
}

// [#68] UK HWANG 2008.04.02 EMV Level2 
int CNHMWICtrl::DevPinBuildPinBlock(LPCTSTR CardPanData, LPCTSTR XORData, short Padchar, LPCTSTR Format, LPCTSTR Key, LPCTSTR KeyEncKey)
{
	if (!m_bPinOpened)											// 장치가Open되지 않았으면
		return R_NORMAL;										// 상태갱신은 의미없음
	
	return m_pPin.BuildPinBlock(CardPanData, NULL, 15, _T("ISO0"), Key, NULL);	// [#554] KSK 2009.08.11
}
// end of [#68]

// [#554] NH KSK 2009.06.11
// ----------------------------------------------------------------------------
// DESCRIPT  : DevPINExportRSAIssuerSignedItem
// PARAMETER : ExportItemType, KeyName
// RETURN    : 
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevPINExportRSAIssuerSignedItem(LPCTSTR ExportItemType, LPCTSTR KeyName)
{
NHDEBUG(1, (_T("CNHMWICtrl::DevPINExportRSAIssuerSignedItem() (%s) (%s)\n"), ExportItemType, KeyName));
	return m_pPin.ExportRSAIssuerSignedItem(ExportItemType, KeyName);
}

// ----------------------------------------------------------------------------
// DESCRIPT  : DevPINImportRSAPublicKey
// PARAMETER : Recv_HSM_PublicKey, Recv_HSM_Signature
// RETURN    : 
// ----------------------------------------------------------------------------
// [#GLDV-2797] AU Kook 2020.04.28 Support Host PK Removal
//int	CNHMWICtrl::DevPINImportRSAPublicKey(LPCTSTR Recv_HSM_PublicKey, LPCTSTR Recv_HSM_Signature)
  int	CNHMWICtrl::DevPINImportRSAPublicKey(LPCTSTR Recv_HSM_PublicKey, LPCTSTR Recv_HSM_Signature, LPCTSTR Usage)
// end of [#GLDV-2797]
{
NHDEBUG(1, (_T("CNHMWICtrl::DevPINImportRSAPublicKey() Recv_HSM_PublicKey(%s)\n Recv_HSM_Signature(%s)\n Usage(%s)\n"), Recv_HSM_PublicKey, Recv_HSM_Signature, Usage));

	// [#GLDV-2797] AU Kook 2020.04.28 Support Host PK Removal
  //return m_pPin.ImportRSAPublicKey(L"HSM_PK", Recv_HSM_PublicKey, L"RSAPUBLICVERIFY", L"", L"RSASSA_PKCS1_V1_5", Recv_HSM_Signature);
	return m_pPin.ImportRSAPublicKey(L"HSM_PK", Recv_HSM_PublicKey, Usage,				L"", L"RSASSA_PKCS1_V1_5", Recv_HSM_Signature);
	// end of [#GLDV-2797]
}

// ----------------------------------------------------------------------------
// DESCRIPT  : DevPINStartKeyExchange
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevPINStartKeyExchange()
{
NHDEBUG(1, (_T("CNHMWICtrl::DevPINStartKeyExchange()\n")));
	return m_pPin.StartKeyExchange();
}

// ----------------------------------------------------------------------------
// DESCRIPT  : DevPINImportRSAPublicKey
// PARAMETER : KeyType, Recv_EPP_RSACrypt, Recv_TMK_RSACrypt, Recv_HSM_Signature
// RETURN    : 
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevPINImportRSASignedDESKey(long KeyType, LPCTSTR Recv_EPP_RSACrypt, LPCTSTR Recv_TMK_RSACrypt, LPCTSTR Recv_HSM_Signature)
{
	CString EPPRSA_TMK;
	int		nReturn = 0;	// Default값 사양 정의 필요

	EPPRSA_TMK.Format(L"%s%s", Recv_EPP_RSACrypt, Recv_TMK_RSACrypt);
	NHDEBUG(1, (_T("CNHMWICtrl::DevPINImportRSASignedDESKey()\n")));
	m_strEPP_KeyCheckValue.Empty();		// 변수 초기화 KSK 2009.06.12

	CString strTemp, strTemp2;

	strTemp.Format(L"%s", Recv_EPP_RSACrypt);
	strTemp2.Format(L"Size1:%d", strTemp.GetLength());
	NVDump('O', '1', "93", L"9999999", strTemp2);

	strTemp2.Format(L"Size2:%d", EPPRSA_TMK.GetLength());
	NVDump('O', '1', "93", L"9999999", strTemp2);

	strTemp.Format(L"%s", Recv_HSM_Signature);
	strTemp2.Format(L"Size3:%d", strTemp.GetLength());
	NVDump('O', '1', "93", L"9999999", strTemp2);

	// [#2174] NH KSK 2013.01.17 RKT시 Key Type별로 Key 주입이 되도록 수정 (MAC Master Key 주입 실패도 추가 수정함)
	// Single Des는 사용하지 않으므로 T-DES만 지원하도록 하며 MAC은 Single, Triple 모두 지원하도록 코딩함
	// KSK 2009.08.11 RKT시에는 Key Type값은 사양 재 정의 필요함
//	if (KeyType == 1)	// SINGLE DES KEY
//		nReturn = m_pPin.ImportRSASignedDESKey(SINGLEDESKEY_NAME, NULL, L"RSAES_PKCS1_V1_5", EPPRSA_TMK, L"KEYENCKEY", NULL, L"RSASSA_PKCS1_V1_5", Recv_HSM_Signature);
//	else if (KeyType == 3)	// TRIPLE DES KEY
//		nReturn = m_pPin.ImportRSASignedDESKey(TRIPLEDESKEY_NAME, NULL, L"RSAES_PKCS1_V1_5", EPPRSA_TMK, L"KEYENCKEY", NULL, L"RSASSA_PKCS1_V1_5", Recv_HSM_Signature);
//	else if (KeyType == 5)	// SINGLE MAC KEY
//		nReturn = m_pPin.ImportRSASignedDESKey(TRIPLEDESKEY_NAME, NULL, L"RSAES_PKCS1_V1_5", EPPRSA_TMK, L"KEYENCKEY", NULL, L"RSASSA_PKCS1_V1_5", Recv_HSM_Signature);
	if (KeyType == 3)	// TRIPLE DES KEY
		nReturn = m_pPin.ImportRSASignedDESKey(TRIPLEDESKEY_NAME, NULL, L"RSAES_PKCS1_V1_5", EPPRSA_TMK, L"KEYENCKEY", NULL, L"RSASSA_PKCS1_V1_5", Recv_HSM_Signature);
	else if (KeyType == 7)	// SINGLE MAC KEY
		nReturn = m_pPin.ImportRSASignedDESKey(SINGLEMACKEY_NAME, NULL, L"RSAES_PKCS1_V1_5", EPPRSA_TMK, L"KEYENCKEY", NULL, L"RSASSA_PKCS1_V1_5", Recv_HSM_Signature);
	else if (KeyType == 11)	// TRIPLE MAC KEY
		nReturn = m_pPin.ImportRSASignedDESKey(TRIPLEMACKEY_NAME, NULL, L"RSAES_PKCS1_V1_5", EPPRSA_TMK, L"KEYENCKEY", NULL, L"RSASSA_PKCS1_V1_5", Recv_HSM_Signature);
	// end of [#2174]

	return nReturn;
}
// end of [#554]

// [#2057] NH KSK 2011.05.04
int	CNHMWICtrl::DevPINInitializeEDM()
{
NHDEBUG(1, (_T("CNHMWICtrl::DevPINInitializeEDM()\n")));
	return m_pPin.InitializeEDM();

	NVDump('O', '1', "93", L"", L"KEY_CLEARED"); // [#2306] NH KSK MWI Add NVRAM Log
}
// end of [#2057]

// [#2080] NH KSK 2011.07.13
// ----------------------------------------------------------------------------
// DESCRIPT  : DevPINGetExtraCaps
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevPINGetExtraCaps()
{
	m_pPin.GetExtraCaps(L"");

	return 0;
}
// end of [#2080]

// [#2253] NH KSK 2014.02.21
// ----------------------------------------------------------------------------
// DESCRIPT  : DevPINGetExtraCaps
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevPIN_GenerateKCV_512K(LPCTSTR KeyName)
{
	int nResult;
	
	nResult = m_pPin.GenerateKCV(KeyName, L"KCVZERO");

	return nResult;
}
// end of [#2253]


// [#RWC6-151] US Kook 2020.07.14 TR34
int	CNHMWICtrl::DevPINGetCertificate(LPCTSTR Certificate)
{
	m_strEPP_CertificateData.Empty();
	int nResult = m_pPin.GetCertificate(Certificate);
	return nResult;
}

int CNHMWICtrl::DevPINInitializeEDMEx(LPCTSTR Indent, LPCTSTR Key)
{
	int nResult = m_pPin.InitializeEDMEx(Indent, Key);

	return nResult;
}

int CNHMWICtrl::DevPINImportKeyBlock(LPCTSTR Key, LPCTSTR EncKey, LPCTSTR KeyBlock)
{
	int nResult = m_pPin.ImportKeyBlock(Key, EncKey, KeyBlock);

	return nResult;
}

int CNHMWICtrl::DevPINLoadCertificateEx(LPCTSTR LoadOption, LPCTSTR Signer, LPCTSTR CertificateData)
{
	int nResult = m_pPin.LoadCertificateEx(LoadOption, Signer, CertificateData);

	return nResult;
}

int CNHMWICtrl::DevPINImportRSAEncipheredPKCS7KeyEx(LPCTSTR RSAKeyIn, LPCTSTR Key, LPCTSTR Use, LPCTSTR LoadOption)
{
	m_strEPP_CertificateData.Empty();
	m_strEPP_KeyCheckValue.Empty();

	int nResult = m_pPin.ImportRSAEncipheredPKCS7KeyEx(RSAKeyIn, Key, Use, LoadOption);

	return nResult;
}

int CNHMWICtrl::DevPINStartAuthenticateSync(LPCTSTR MethodName, LPCTSTR Parameters)
{
	int nResult = m_pPin.StartAuthenticateSync(MethodName, Parameters);

	return nResult;
}

CString CNHMWICtrl::DevPINGetCertificateState()
{
	return m_pPin.GetCertificateState();
}

// properties for unbind
CString CNHMWICtrl::DevPINGetAuthenticateDataToSign()
{
	return m_pPin.GetAuthenticateDataToSign();
}

CString CNHMWICtrl::DevPINGetAuthenticateSigner()
{
	return m_pPin.GetAuthenticateSigner();
}

void CNHMWICtrl::DevPINSetAuthenticateSigner(LPCTSTR Signer)
{
	m_pPin.SetAuthenticateSigner(Signer);
}

void CNHMWICtrl::DevPINSetAuthenticateSigKey(LPCTSTR SigKey)
{
	m_pPin.SetAuthenticateSigKey(SigKey);
}

void CNHMWICtrl::DevPINSetAuthenticateSignedData(LPCTSTR SignedData)
{
	m_pPin.SetAuthenticateSignedData(SignedData);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Event : Accepted From PIN
///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CNHMWICtrl::OnFatalErrorNxpin(LPCTSTR Action, long Result) 
{
	NHDEBUG(1, (_T("Action: %s, Result: %x (%d)\n"), Action, Result, Result));
	m_nPINEntryMode = PINMODE_NOT_USE;							// Pin enter MODE Reset
	m_bPinEntry = ENTRY_DISABLED;
	m_strPinEppCardData.Empty();

	m_strEPP_ExportedKey.Empty();		// 변수 초기화 KSK 2009.06.12
	m_strEPP_Signed.Empty();			// 변수 초기화 KSK 2009.06.12
	m_strEPP_RandomNumber.Empty();		// 변수 초기화 KSK 2009.06.12 [#2259] NH KSK 2014.03.13
	m_strEPP_KeyCheckValue.Empty();		// 변수 초기화 KSK 2009.06.12
	// 모든 상태값을 업데이트한다.
	DevPinUpdateStatus();

// [#GLDV-2797] AU Kook 2020.05.26 Support Host PK Removal
	NVDump('F', '1', "93", L"9999999", DevPinGetErrorCode());

	if (Result == WFS_ERR_PIN_KEYNOTFOUND				// -400 : "HSM_PK" not found
		&& wcscmp(Action, L"CMD_PIN_IMPORT_RSA_PUBLIC_KEY") == 0)
	{		
		NHDEBUG(1, (_T("Set _HPKEMTY_ ECODE from result of -400.\n") ));
		NVDump('F', '1', "93", L"9999999", L"_HPKEMPTY_");

		// set it as "DeviceError" instead of "FatalError"
		// to prevent MWI to write "9792825" error code automatically. ("9792825": "ImportRSAPublicKey" command failure)
		ProcSetDeviceEvent(L"PINPAD", L"DeviceError", NULL);

		// need to clear existing error code first to write "HPKEMTY" error code newly.
		//DevPinClearErrorCode();		// this doesn't work as expected, set it "0000000" manually.
		char szTemp[8] = "0000000";
		memcpy(m_pSPInform->SPErr.ErrCode_PIN, szTemp, 7);
		m_pDevCtrl->WriteMwiErrorCode(DEV_PIN, "PINPAD", 7, "HPKEMTY");
	}
	else
	// end of [#GLDV-2797]
		ProcSetDeviceEvent(L"PINPAD", L"FatalError", NULL);


	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%x", Result);
	NVDump('F', '1', "93", L"9999999", L"FATAL_ERR");
	NVDump('F', '1', "93", L"9999999", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnDeviceErrorNxpin(LPCTSTR Action, long Result) 
{
NHDEBUG(1, (_T("***NHMWI***CNHMWICtrl::OnDeviceErrorNxpin() Action(%s) Result(%d)\n"), Action, Result));
	// KSK_2007-06-04오후 6:06:57
	// PIN ERROR시 CANCEL 처리하도록 수정
	/*
	m_nPINEntryMode = PINMODE_NOT_USE;							// Pin enter MODE Reset
	m_bPinEntry = ENTRY_DISABLED;
	m_strPinEppCardData.Empty();

	// 모든 상태값을 업데이트한다.								
	DevPinUpdateStatus();
	
	ProcSetDeviceEvent(L"PINPAD", L"DeviceError", NULL);
	*/
	if (m_nPINEntryMode == PINMODE_EPP)       // 2006.02.20
	{
		FireOnDisplayPinpadData(L"CANCEL");      // Fire Display Event to Container
	}

	m_nPINEntryMode = PINMODE_NOT_USE;       // Pin enter MODE Reset
	m_bPinEntry = ENTRY_DISABLED;
	m_strPinEppCardData.Empty();

	m_strEPP_ExportedKey.Empty();		// 변수 초기화 KSK 2009.06.12
	m_strEPP_Signed.Empty();			// 변수 초기화 KSK 2009.06.12
	m_strEPP_RandomNumber.Empty();		// 변수 초기화 KSK 2009.06.12	[#2259] NH KSK 2014.03.13
	m_strEPP_KeyCheckValue.Empty();		// 변수 초기화 KSK 2009.06.12

	// 모든 상태값을 업데이트한다.        
	DevPinUpdateStatus();
	ProcSetDeviceEvent(L"PINPAD", L"DeviceError", NULL);
	// end of KSK_2007-06-04오후 6:06:57

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%x", Result);
	NVDump('F', '1', "93", L"9999999", L"DEV_ERR");
	NVDump('F', '1', "93", L"9999999", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnLockGrantedNxpin() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnLockReleasedNxpin() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnLockTimeoutNxpin() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnEncryptCompleteNxpin(LPCTSTR CryptData) 
{
NHDEBUG(1, (_T("***NHMWI***CNHMWICtrl::OnEncryptCompleteNxpin()\n")));

	ProcSetDeviceEvent(L"PINPAD", L"EncryptComplete", NULL);
}

void CNHMWICtrl::OnDecryptCompleteNxpin(LPCTSTR CryptData) 
{
NHDEBUG(1, (_T("***NHMWI***CNHMWICtrl::OnDecryptCompleteNxpin()\n")));

	ProcSetDeviceEvent(L"PINPAD", L"DecryptComplete", NULL);
}

void CNHMWICtrl::OnMACingCompleteNxpin(LPCTSTR CryptData) 
{

	m_strPinMacingData.Empty();
	
	m_strPinMacingData.Format(_T("%s"), CryptData);
NHDEBUG(1, (_T("***NHMWI***CNHMWICtrl::OnMACingCompleteNxpin()(%s)\n"), m_strPinMacingData));
	ProcSetDeviceEvent(L"PINPAD", L"MACingComplete", NULL);
}

void CNHMWICtrl::OnKeyImportedNxpin() 
{
NHDEBUG(1, (_T("***NHMWI***CNHMWICtrl::OnKeyImportedNxpin()\n")));

	ProcSetDeviceEvent(L"PINPAD", L"KeyImported", NULL);
}

void CNHMWICtrl::OnReadPinCompleteNxpin() 
{
	m_bPinEntry = ENTRY_DISABLED;

	NHDEBUG(1, (_T("CNHMWICtrl::OnReadPinCompleteNxpin() m_strPinEppCardData (%s) \n"), m_strPinEppCardData));

	if ((m_strPinLastKey == "F1")	||
		(m_strPinLastKey == "F2")	||
		(m_strPinLastKey == "F3")	||
		(m_strPinLastKey == "F4")	||
		(m_strPinLastKey == "F5")	||
//		(m_strPinLastKey == "F6")	||							// Enter Emulation
		(m_strPinLastKey == "F7")	||
		(m_strPinLastKey == "F8"))
		;
	else
	{
		NHDEBUG(1, (L"[CNHMWICtrl::OnReadPinCompleteNxpin] m_strPinEppCardData [%s] \n", m_strPinEppCardData));

		if(m_strPinEppCardData.GetLength() > 0 ) // MS
		{
			// [#554] KSK 2009.08.11 512K시 LoadWorkingKey 방식변경으로 인해 수정
			if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) >= 0)
			{
				// Key Mode 1, 5
				if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_DDES ||
					MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING)
					m_pPin.BuildPinBlock(m_strPinEppCardData.GetBuffer(0), NULL, 15, _T("ISO0"), SINGLEDESKEY_WORKINGKEY_NAME, NULL);
				// Key Mode 3, 7, 11
				else 
				if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_TDES ||
					MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING	||
					MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)
					m_pPin.BuildPinBlock(m_strPinEppCardData.GetBuffer(0), NULL, 15, _T("ISO0"), TRIPLEDESKEY_WORKINGKEY_NAME, NULL);
			}
			else
			{
				m_pPin.BuildPinBlock(m_strPinEppCardData.GetBuffer(0), NULL, 15, _T("ISO0"), NULL, NULL);
				NHDEBUG(1, (L"[CNHMWICtrl::BuildPinBlock] m_strPinEppCardData 1 [%s] \n", m_strPinEppCardData));
			}
			// end of [#554]

			m_strPinEppCardData.ReleaseBuffer();
			m_strPinEppCardData.Empty();
		}
		else // IC
			ProcSetDeviceEvent(L"PINPAD", L"ReadPinComplete", NULL);
	}
}

void CNHMWICtrl::OnReadPinCancelledNxpin() 
{
	m_nPINEntryMode = PINMODE_NOT_USE;							// Pin enter MODE Reset
	m_bPinEntry = ENTRY_DISABLED;
	m_strPinEppCardData.Empty();
	ProcSetDeviceEvent(L"PINPAD", L"ReadPinCancelled", NULL);
}

void CNHMWICtrl::OnKeyPressedNxpin(LPCTSTR Key, long KeyCode) 
{
	//NHDEBUG(1, (_T("[%s], Prev: [%s] \n"), Key, m_strPinLastKey));
	m_strPinLastKey = Key;										// 2005.04.19
	
	FireOnDisplayPinpadData(Key);								// Fire Display Event to Container
}

void CNHMWICtrl::OnPinBlockCompleteNxpin(LPCTSTR PinBlock) 
{
/*
	CStringArray	SArray;

	SArray.RemoveAll();
	VarArrayToStringArray(&PinBlock, SArray);
	BYTE FormattedData[8];
	memset(FormattedData, 0x00, sizeof(FormattedData));

	for (int i = 0;i<sizeof(FormattedData);i++)
		FormattedData[i] = (BYTE)Asc2Int(SArray.GetAt(i));
	HexDump(TRACE_DUMP_MODE, TRACE_FILENAME, __FILE__, __LINE__, "OnDevPinPinFormatted(StringArray)", FormattedData, 8);
*/

	m_strPinKeyData = PinBlock;
	m_bPinEntry = ENTRY_DISABLED;								// 2004.04.24
	m_strPinEppCardData.Empty();
NHDEBUG(1, (_T("***NHMWI***CNHMWICtrl::OnPinBlockCompleteNxpin()(%s)\n"), m_strPinKeyData));
	ProcSetDeviceEvent(L"PINPAD", L"PinBlockComplete", NULL);		// 2004.04.24
}

void CNHMWICtrl::OnReadDataCompleteNxpin(LPCTSTR Data) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnReadDataCompleteNxpin() Data(%s) \n"), Data));

	m_bPinEntry = ENTRY_DISABLED;
	m_strPinKeyData = Data;
	ProcSetDeviceEvent(L"PINPAD", L"ReadDataComplete", Data);
}

void CNHMWICtrl::OnReadDataCancelledNxpin() 
{
	m_nPINEntryMode = PINMODE_NOT_USE;							// Pin enter MODE Reset
	m_bPinEntry = ENTRY_DISABLED;
	m_strPinEppCardData.Empty();
	ProcSetDeviceEvent(L"PINPAD", L"ReadDataCancelled", NULL);
}

void CNHMWICtrl::OnEDMInitializedNxpin() 
{
	// [#2057] NH KSK 2011.05.04
//NHDEBUG(1, (_T("***NHMWI***CNHMWICtrl::OnEDMInitializedNxpin()\n")));
	ProcSetDeviceEvent(L"PINPAD", L"EDMInitialized", NULL);
	// end of [#2057]
}

void CNHMWICtrl::OnDeviceStatusChangedNxpin(LPCTSTR Value) 
{
	DevPinUpdateStatus();										// 2004.11.05

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%s", Value);
	NVDump('O', '1', "93", L"", L"DEV_C_STS");
	NVDump('O', '1', "93", L"", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnTimeoutNxpin() 
{
	m_nPINEntryMode = PINMODE_NOT_USE;							// Pin enter MODE Reset
	m_bPinEntry = ENTRY_DISABLED;
	m_strPinEppCardData.Empty();
	ProcSetDeviceEvent(L"PINPAD", L"Timeout", NULL);
	FireOnDisplayPinpadData(L"TIMEOVER");								// AIREAT 2008.06.09 - PCI EPP TIMEOUT 처리 ScrCtrl 위임

	// [#2133] NH KSK 2012.05.07
	CString	strLog;
	strLog.Format(L"MWI_TIMEOUT");
	NVDump('F', '1', "99", L"0000000", strLog);
	// end of [#2133]
}

void CNHMWICtrl::OnIllegalKeyAccessedNxpin(LPCTSTR KeyName, long KeyCode) 
{
NHDEBUG(1, (_T("***NHMWI***CNHMWICtrl::OnIllegalKeyAccessedNxpin() kName(%s) kCode(%ud)\n"), KeyName, KeyCode));

	// [#2580] AU Kook 2019.07.18
	CString	strLog;
	strLog.Format(L"IKA_%s", KeyName);
	NVDump('F', '1', "93", L"", strLog);
	// end of [#2580]

	m_strEPP_ExportedKey.Empty();		// 변수 초기화
	m_strEPP_Signed.Empty();			// 변수 초기화
	m_strEPP_KeyCheckValue.Empty();		// 변수 초기화 KSK 2009.06.12

	// 모든 상태값을 업데이트한다.
	DevPinUpdateStatus();

	ProcSetDeviceEvent(L"PINPAD", L"IllegalKeyAccessed", NULL);
}

void CNHMWICtrl::OnResetCompleteNxpin() 
{
	m_bPinEntry = ENTRY_DISABLED;
	ProcSetDeviceEvent(L"PINPAD", L"ResetComplete", NULL);
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 
// PARAMETER : RKT 지원을 위한 EVENT 추가
// RETURN    : 
// REMARK	 : KSK 2009.06.11
// ----------------------------------------------------------------------------
void CNHMWICtrl::OnKeyDeletedNxpin() 
{
	m_strEPP_KeyCheckValue.Empty();		// 변수 초기화 KSK 2009.06.12

	// 모든 상태값을 업데이트한다.
	DevPinUpdateStatus();

	ProcSetDeviceEvent(L"PINPAD", L"KeyDeleted", NULL);
}

void CNHMWICtrl::OnRandomNumberGeneratedNxpin(LPCTSTR RandomData) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnVerifyLocalDESCompleteNxpin(long Result) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnStartKeyExchangeCompleteNxpin(LPCTSTR RandomItem) 
{
NHDEBUG(1, (_T("***NHMWI***CNHMWICtrl::OnStartKeyExchangeCompleteNxpin() RandomItem(%s)\n"), RandomItem));
	m_strEPP_RandomNumber.Format(L"%s", RandomItem);	// [#2259] NH KSK 2014.03.13

	// 모든 상태값을 업데이트한다.
	DevPinUpdateStatus();

	ProcSetDeviceEvent(L"PINPAD", L"StartKeyExchangeComplete", NULL);
}

void CNHMWICtrl::OnRSAPublicKeyImportedNxpin(LPCTSTR KeyCheckMode, LPCTSTR KeyCheckValue) 
{
NHDEBUG(1, (_T("***NHMWI***CNHMWICtrl::OnRSAPublicKeyImportedNxpin() KeyCheckMode(%s) KeyCheckValue(%s)\n"), KeyCheckMode, KeyCheckValue));
	// 모든 상태값을 업데이트한다.
	DevPinUpdateStatus();

	ProcSetDeviceEvent(L"PINPAD", L"RSAPublicKeyImported", NULL);	
}

void CNHMWICtrl::OnRSAIssuerSignedItemExportedNxpin(LPCTSTR Value, LPCTSTR RSASignatureAlgorithm, LPCTSTR Signature)
{
NHDEBUG(1, (_T("***NHMWI***CNHMWICtrl::OnRSAIssuerSignedItemExportedNxpin() Value(%s) RSASignatureAlgorithm(%s) Signature(%s)\n"), Value, RSASignatureAlgorithm, Signature));
	m_strEPP_ExportedKey.Format(L"%s", Value);
	m_strEPP_Signed.Format(L"%s", Signature);

	// 모든 상태값을 업데이트한다.
	DevPinUpdateStatus();

	ProcSetDeviceEvent(L"PINPAD", L"RSAIssuerSignedItemExported", NULL);
}

void CNHMWICtrl::OnRSASignedDESKeyImportedNxpin(LPCTSTR KeyLength, LPCTSTR KeyCheckMode, LPCTSTR KeyCheckValue)
{
NHDEBUG(1, (_T("***NHMWI***CNHMWICtrl::OnRSAIssuerSignedItemExportedNxpin() KeyLength(%s) KeyCheckMode(%s) KeyCheckValue(%s)\n"), KeyLength, KeyCheckMode, KeyCheckValue));

	m_strEPP_KeyCheckValue = KeyCheckValue;
	// 모든 상태값을 업데이트한다.
	DevPinUpdateStatus();
	
	ProcSetDeviceEvent(L"PINPAD", L"RSASignedDESKeyImported", NULL);	
}

void CNHMWICtrl::OnRSAKeyPairGeneratedNxpin()
{
	NHDEBUG(1, (_T("OnRSAKeyPairGeneratedNxpin()\n"), _T("")));
	
}

void CNHMWICtrl::OnRSAEPPSignedItemExportedNxpin(LPCTSTR Value, LPCTSTR SelfSignature, LPCTSTR Signature) 
{
	NHDEBUG(1, (_T("OnRSAEPPSignedItemExportedNxpin()\n"), _T("")));
	NHDEBUG(1, (_T("CertificateData : %s\n"), Value));
	NHDEBUG(1, (_T("CertificateData : %s\n"), SelfSignature));
	NHDEBUG(1, (_T("CertificateData : %s\n"), Signature));
	
}

void CNHMWICtrl::OnLoadCertificateCompleteNxpin(LPCTSTR CertificateData) 
{
	NHDEBUG(1, (_T("OnLoadCertificateCompleteNxpin()\n"), _T("")));
	NHDEBUG(1, (_T("CertificateData : %s\n"), CertificateData));
}

void CNHMWICtrl::OnGetCertificateCompleteNxpin(LPCTSTR CertificateData) 
{
	NHDEBUG(1, (_T("OnGetCertificateCompleteNxpin()\n"), _T("")));
	NHDEBUG(1, (_T("CertificateData : %s\n"), CertificateData));

	m_strEPP_CertificateData = CertificateData;

	DevPinUpdateStatus();
	ProcSetDeviceEvent(L"PINPAD", L"GetCertificateComplete", NULL);
}

void CNHMWICtrl::OnReplaceCertificateCompleteNxpin(LPCTSTR CertificateData) 
{
	NHDEBUG(1, (_T("OnReplaceCertificateCompleteNxpin()\n"), _T("")));
	NHDEBUG(1, (_T("CertificateData : %s\n"), CertificateData));

}

void CNHMWICtrl::OnRSAEncipheredKeyImportedNxpin(LPCTSTR KeyLength, LPCTSTR RSAData) 
{
	NHDEBUG(1, (_T("OnRSAEncipheredKeyImportedNxpin()\n"), _T("")));
	NHDEBUG(1, (_T("KeyLength : %s\n"), KeyLength));
	NHDEBUG(1, (_T("RSAData : %s\n"), RSAData));
}

void CNHMWICtrl::OnCertificateChangedNxpin(LPCTSTR Change) 
{
	NHDEBUG(1, (_T("OnCertificateChangedNxpin()\n"), _T("")));
	NHDEBUG(1, (_T("Change : %s\n"), Change));
}

void CNHMWICtrl::OnKCVGeneratedNxpin(LPCTSTR KeyName, LPCTSTR KCV) 
{
	// [#2253] NH KSK 2013.02.21
	NHDEBUG(1, (_T("KeyName(%s) KCV(%s)\n"), KeyName, KCV));

	m_strEPP_KeyCheckValue = KCV;

	// 모든 상태값을 업데이트한다.
	DevPinUpdateStatus();
	
	ProcSetDeviceEvent(L"PINPAD", L"KCVGenerated", NULL);	
	// end of [#2253]
}

void CNHMWICtrl::OnSecureKeyEntryCompleteNxpin(LPCTSTR KCV) 
{
	NHDEBUG(1, (_T("OnSecureKeyEntryCompleteNxpin()\n"), _T("")));
	NHDEBUG(1, (_T("KCV : %s\n"), KCV));
}

void CNHMWICtrl::OnSecureKeyEntryCancelledNxpin() 
{
	NHDEBUG(1, (_T("OnSecureKeyEntryCancelledNxpin()\n"), _T("")));
}

void CNHMWICtrl::OnSecureKeyEntryIncompleteNxpin() 
{
	NHDEBUG(1, (_T("OnSecureKeyEntryIncompleteNxpin()\n"), _T("")));
}

// [#RWC6-151] US Kook 2020.07.14 TR34
void CNHMWICtrl::OnKeyBlockImported()
{
	NHDEBUG(1, (_T("OnKeyBlockImported()\n"), _T("")));
	NVDump('F', '1', "93", L"9999999", L"KBImported");

	// 모든 상태값을 업데이트한다.
	DevPinUpdateStatus();
	ProcSetDeviceEvent(L"PINPAD", L"KeyBlockImported", NULL);
}

void CNHMWICtrl::OnRSAEncipheredKeyExImported(LPCTSTR KeyLength, LPCTSTR CertificateData, LPCTSTR RSAKeyCheckMode, LPCTSTR KeyCheckMode, LPCTSTR KeyCheckValue)
{
	NHDEBUG(1, (_T("OnRSAEncipheredKeyExImported()\n"), _T("")));
	NHDEBUG(1, (_T("KeyLength : %s\n"), KeyLength));
	NHDEBUG(1, (_T("CertificateData : %s\n"), CertificateData));
	NHDEBUG(1, (_T("RSAKeyCheckMode : %s\n"), RSAKeyCheckMode));
	NHDEBUG(1, (_T("KeyCheckMode : %s\n"), KeyCheckMode));
	NHDEBUG(1, (_T("KeyCheckValue : %s\n"), KeyCheckValue));
	NVDump('F', '1', "93", L"9999999", L"RSAEKImported");

	m_strEPP_CertificateData = CertificateData;
	m_strEPP_KeyCheckValue = KeyCheckValue;

	// 모든 상태값을 업데이트한다.
	DevPinUpdateStatus();
	ProcSetDeviceEvent(L"PINPAD", L"RSAEncipheredKeyExImported", NULL);
}

void CNHMWICtrl::OnLoadCertificateExComplete(LPCTSTR RSAKeyCheckMode, LPCTSTR RSAData)
{
	NHDEBUG(1, (_T("OnLoadCertificateExComplete()\n"), _T("")));
	NHDEBUG(1, (_T("RSAKeyCheckMode : %s\n"), RSAKeyCheckMode));
	NHDEBUG(1, (_T("RSAData : %s\n"), RSAData));
	NVDump('F', '1', "93", L"9999999", L"LoadCertExComplt");

	// 모든 상태값을 업데이트한다.
	DevPinUpdateStatus();
	ProcSetDeviceEvent(L"PINPAD", L"LoadCertificateExComplete", NULL);
}
