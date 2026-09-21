// APPINIF.cpp

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
// Method : Called From AP
///////////////////////////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
long CNHMWICtrl::PinInitialize() 
{
NHDEBUG(1, (_T("CNHMWICtrl::PinInitialize()(%s) \n"), "  "));

	int nResult = 0;

	if (m_bPinOpened)											// 기오픈시 다시 클로즈한다.
	{
		// 2019.07.17 AU Kook
		NHDEBUG(1, (_T("CNHMWICtrl::PinEntryDisable before DevPinCloseConnection()(%s) \n"), "  "));
		PinEntryDisable();
		// end of 2019.07.13

//y		DevPinPreEntryDisable();
		m_pDevCtrl->EvtQReset(DEV_PIN);
		nResult = DevPinCloseConnection();
		if (nResult != R_NORMAL)
		{
			return nResult;
		}
	}

	m_bPinOpened = TRUE;

	SetEventMatrix(DEV_PIN, "OpenSessionSync");
	nResult = DevPinOpenConnection();
	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
long CNHMWICtrl::PinDeinitialize() 
{
NHDEBUG(1, (_T("CNHMWICtrl::PinDeinitialize()(%s) \n"), "  "));

	int nResult = 0;
	if (m_bPinOpened)
	{
		// KSK_2007-08-16오전 9:37:14
		PinEntryDisable();
//		DevPinPreEntryDisable();								// 2004.04.26
		// end of KSK_2007-08-16오전 9:37:14
		m_bPinOpened = FALSE;										
		nResult = DevPinCloseConnection();
	}
	else
	{
		m_bPinOpened = FALSE;										
	}
	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR PinGetErrorCode() 
// DESCRIPT  : 장애코드를 조회한다.
// PARAMETER : 없음
// RETURN    : 장애코드
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::PinGetErrorCode() 
{
NHDEBUG(1, (_T("CNHMWICtrl::PinGetErrorCode()(%s) \n"), "  "));

	CString strResult("");
	strResult = DevPinGetErrorCode();
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long PinClearErrorCode() 
// DESCRIPT  : 카드부의 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
long CNHMWICtrl::PinClearErrorCode() 
{
NHDEBUG(1, (_T("CNHMWICtrl::PinClearErrorCode()(%s) \n"), "  "));

	return DevPinClearErrorCode();
}

// ----------------------------------------------------------------------------
// DESCRIPT  : PINPAD장치상태조회처리
// PARAMETER : 없음
// RETURN    : NORMAL(0), NODEVICE(1), DOWN(2)
// ----------------------------------------------------------------------------
long CNHMWICtrl::PinGetDeviceStatus() 
{
	return DevPinGetDeviceStatus();								// 2004.11.05
}

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
long CNHMWICtrl::PinEntryDisable() 
{
	// 2007.08.14 전체적으로 변경 처리함.
	// KSK_2007-08-16오전 9:37:14
	int nReturn = R_NORMAL;
	
	if (m_bPinEntry == ENTRY_ENABLED)							// DiableEntry before EntryEnable
	{
		if (m_nPINEntryMode == PINMODE_NORMAL)
		{
			SetEventMatrix(DEV_PIN, "CancelReadData");
		}
		else
		if (m_nPINEntryMode == PINMODE_EPP)
		{
			SetEventMatrix(DEV_PIN, "CancelReadPin");
		}

		nReturn = DevPinEntryDisable();

		if (m_pDevCtrl->CheckDeviceAction(DEV_PIN, K_30_WAIT) != NORMAL)
		{
			// [#2133] NH KSK 2012.05.07 Middleware 무응답시 MWI CheckDeviceAction안에서 Fatal로 Setting하므로 Event 설정 안하도록 수정
//			SetEventMatrix(DEV_PIN, "ReadData");
//			ProcSetDeviceEvent(L"PINPAD", L"FatalError", L"");
			// end of [#2133]

			return R_ERROR;
		}
	}
	// end of KSK_2007-08-16오전 9:37:14
	return nReturn;

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
long CNHMWICtrl::PinEntryEnable(long	nEnableMode, 
								long	nMinKeyCount, 
								long	nMaxKeyCount, 
								BOOL	bAutoEnd, 
								LPCTSTR szActiveKeys, 
								LPCTSTR szTerminatorKeys, 
								LPCTSTR szCardData, 
								long	nTimeout		) 
{
NHDEBUG(1, (_T("CNHMWICtrl::PinEntryEnable() [nEnableMode(%d), nMinKeyCount(%d), nMaxKeyCount(%d), bAutoEnd(%d), szActiveKeys(%s), szTerminatorKeys(%s), szCardData(%s), nTimeout(%d)] \n"), 
											nEnableMode, 
											nMinKeyCount, 
											nMaxKeyCount, 
											bAutoEnd, 
											szActiveKeys, 
											szTerminatorKeys, 
											szCardData, 
											nTimeout));

	m_pDevCtrl->EvtQReset(DEV_PIN);							// [#2133] NH KSK 2012.05.07 Queue Reset 처리를 APPinIF에서 하도록 수정 (Disable 처리 에러시 보완을 위해)

	if (PinEntryDisable() == R_ERROR)						// 2004.04.26
		return R_ERROR;

	switch(nEnableMode)
	{
	case PINMODE_NORMAL:									// MODE : Normal Input mode
		SetEventMatrix(DEV_PIN, "ReadData");
		break;
	case PINMODE_EPP:										// MODE : Epp Input mode
		SetEventMatrix(DEV_PIN, "ReadPin");
		break;
	default:
		// 상위국(AP)에서 상위 2가지로만 사용하므로 이 부분은 죽은 코드임
		SetEventMatrix(DEV_PIN, "ReadData");
		m_pDevCtrl->WriteMwiErrorCode(DEV_PIN, "PINPAD", 7, "979280B");
		ProcSetDeviceEvent((LPCTSTR)DEVNM_PIN, (LPCTSTR)"FatalError", NULL);
		
		SysFreeString((BSTR)szActiveKeys);
		SysFreeString((BSTR)szTerminatorKeys);
		SysFreeString((BSTR)szCardData);

		return R_ERROR;
	}

	m_bPinEntry = ENTRY_ENABLED;								// Set Enable Flag

	long nRet;
	nRet = DevPinEntryEnable(	nEnableMode, 
								nMinKeyCount, 
								nMaxKeyCount, 
								bAutoEnd, 
								szActiveKeys, 
								szTerminatorKeys, 
								szCardData,
								nTimeout		);

	SysFreeString((BSTR)szActiveKeys);
	SysFreeString((BSTR)szTerminatorKeys);
	SysFreeString((BSTR)szCardData);

	return nRet;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::PinGetPinKeyData() 
{
NHDEBUG(1, (_T("CNHMWICtrl::PinGetPinKeyData()(%s) \n"), "  "));

	CString strResult("");
	strResult = DevPinGetKeyData();

NHDEBUG(1, (_T("CNHMWICtrl::PinGetPinKeyData() strResult(%s) \n"), strResult));
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 송수신데이타를 Macing처리한다.
// PARAMETER : MAC하고자 하는 원본 데이타(UnPackData)
// RETURN    : 
// ----------------------------------------------------------------------------
long CNHMWICtrl::PinMacingData(LPCTSTR szMacData) 
{
NHDEBUG(1, (_T("CNHMWICtrl::PinMacingData() szMacData(%s) \n"), szMacData));

	int nResult = 0;

	// 2007.08.14 KJK DISABLE 처리 통합
	// KSK_2007-08-16오전 9:37:14
	PinEntryDisable();
//	DevPinPreEntryDisable();
	// end of KSK_2007-08-16오전 9:37:14

	SetEventMatrix(DEV_PIN, "GenerateMAC");
	nResult = DevPinMacing(szMacData);
	CheckDeviceAction(DEV_PIN, K_30_WAIT);	// Macing 실패시 SP에서 내부적으로 Retry가 있으므로 timeout을 늘려야함. (추후 SP와 timeout 협의 필요)

	SysFreeString((BSTR)szMacData);

	return nResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Macing처리된 데이타를 조회한다.
// PARAMETER : X
// RETURN    : Macing UnPack Data
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::PinGetMacingData() 
{

	CString strResult("");										// 2004.04.25
	strResult = DevPinGetMacingData();

NHDEBUG(1, (_T("CNHMWICtrl::PinGetMacingData() strResult(%s) \n"), strResult));
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// DESCRIPT  : LoadIV
// PARAMETER : KeyName to load
// RETURN    : X
// ----------------------------------------------------------------------------
long CNHMWICtrl::PinLoadIV(LPCTSTR szKeyName) 
{
NHDEBUG(1, (_T("CNHMWICtrl::PinLoadIV() szKeyName(%s) \n"), szKeyName));

	int nResult = 0;

	// 2007.08.14 KJK DISABLE 처리 통합
	// KSK_2007-08-16오전 9:37:14
	PinEntryDisable();
//	DevPinPreEntryDisable();
	// end of KSK_2007-08-16오전 9:37:14

//	SetEventMatrix(DEV_PIN, "LoadIV");
	SetEventMatrix(DEV_PIN, "ImportKey");
	nResult = DevPinLoadIV(szKeyName);
	CheckDeviceAction(DEV_PIN, K_30_WAIT);

	SysFreeString((BSTR)szKeyName);

	return nResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : LoadKey
// PARAMETER : KeyName to load, KeyValue
// RETURN    : X
// ----------------------------------------------------------------------------
long CNHMWICtrl::PinLoadKey(LPCTSTR szKeyName, LPCTSTR szKeyValue) 
{
NHDEBUG(1, (_T("CNHMWICtrl::PinLoadKey() szKeyName(%s) szKeyValue(%s) \n"), szKeyName, szKeyValue));

	int nResult = 0;

	// KSK_2007-08-16오전 9:37:14
	PinEntryDisable();
//	DevPinPreEntryDisable();
	// end of KSK_2007-08-16오전 9:37:14
	
	SetEventMatrix(DEV_PIN, "ImportKey");

	nResult = DevPinLoadKey(szKeyName, szKeyValue);

	if (nResult == WFS_SUCCESS)
	{
		CString strResult;
		strResult = CheckDeviceAction(DEV_PIN, K_30_WAIT);
		if (strResult == "NORMAL")
			nResult = WFS_SUCCESS;
		else if (strResult == "ERROR")
			nResult = WFS_ERR_CONNECTION_LOST;
		else
			nResult = WFS_ERR_HARDWARE_ERROR;
	}

	SysFreeString((BSTR)szKeyName);
	SysFreeString((BSTR)szKeyValue);

	return nResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : LoadEncryptedKey
// PARAMETER : KeyName to load, KeyValue, EncKeyName
// RETURN    : 
// ----------------------------------------------------------------------------
long CNHMWICtrl::PinLoadEncryptedKey(LPCTSTR KeyUse, LPCTSTR strKeyName, LPCTSTR strKeyValue, LPCTSTR strEncKeyName) 
{
NHDEBUG(1, (_T("CNHMWICtrl::PinLoadEncryptedKey() KeyUse(%s) strKeyName(%s) strKeyValue(%s) strEncKeyName(%s) \n"), KeyUse, strKeyName, strKeyValue, strEncKeyName));

	int nResult = 0;

	// 2007.08.14 KJK DISABLE 처리 통합
	// KSK_2007-08-16오전 9:37:14
	PinEntryDisable();
//	DevPinPreEntryDisable();
	// end of KSK_2007-08-16오전 9:37:14
	
	SetEventMatrix(DEV_PIN, "ImportKey");
	nResult = DevPinLoadEncryptedKey(KeyUse, strKeyName, strKeyValue, strEncKeyName);
//[#679] SOOK Working Download 실패 저널 오류 수정 2010.11.26
	//SOOK 2010.11.26 공장 실거래 테스트 
//	CheckDeviceAction(DEV_PIN, K_30_WAIT);
	if (nResult == WFS_SUCCESS)
	{
		CString strResult;
		strResult = CheckDeviceAction(DEV_PIN, K_30_WAIT);
		if (strResult == "NORMAL")
			nResult = WFS_SUCCESS;
		else if (strResult == "ERROR")
			nResult = WFS_ERR_CONNECTION_LOST;
		else
			nResult = WFS_ERR_HARDWARE_ERROR;
	}
//end of [#679]

	SysFreeString((BSTR)KeyUse);
	SysFreeString((BSTR)strKeyName);
	SysFreeString((BSTR)strKeyValue);
	SysFreeString((BSTR)strEncKeyName);

	return nResult;
}

// [#68] UK HWANG 2008.04.02 EMV Level2 
long CNHMWICtrl::PinBuildPinBlock(LPCTSTR CardPanData, LPCTSTR XORData, short Padchar, LPCTSTR Format, LPCTSTR Key, LPCTSTR KeyEncKey)
{
	NHDEBUG(1, (_T("CNHMWICtrl::PinBuildPinBlock() CustomerData(%s) XORData(%s) Padchar(%d) Format(%s) Key(%s) KeyEncKey(%s) \n"), 
		CardPanData, XORData, Padchar, Format, Key, KeyEncKey));

	SetEventMatrix(DEV_PIN, "BuildPinBlock");

	long nRet = DevPinBuildPinBlock(CardPanData, XORData, Padchar, Format, Key, KeyEncKey);

	SysFreeString((BSTR)CardPanData);
	SysFreeString((BSTR)XORData);
	SysFreeString((BSTR)Format);
	SysFreeString((BSTR)Key);
	SysFreeString((BSTR)KeyEncKey);

	return nRet;
}
// end of [#68]
// [#554] NH KSK 2009.08.10	RKT 관련 함수 추가
long CNHMWICtrl::RKT_ExportRSAIssuerSignedItem(LPCTSTR ExportItemType, LPCTSTR KeyName)
{
	NHDEBUG(1, (_T("CNHMWICtrl::RKT_ExportRSAIssuerSignedItem  ExportItemType(%s) KeyName(%s)\n"), ExportItemType, KeyName));
	
	int nResult = 0;

	m_strEPP_ExportedKey.Empty();
	m_strEPP_Signed.Empty();

	SetEventMatrix(DEV_PIN, "ExportRSAIssuerSignedItem");

	// 비동기함수이나 MWI에서 동기화 함수로 처리함. (AP는 그게 편할것 같아서 그렇게 했음 KSK 2009.06.12)
	nResult = DevPINExportRSAIssuerSignedItem(ExportItemType, KeyName);

	NHDEBUG(1, (_T("CNHMWICtrl::RKT_ExportRSAIssuerSignedItem  nResult(%x)\n"), nResult));

	if (nResult == WFS_SUCCESS)
	{
		CString strResult;
		strResult = CheckDeviceAction(DEV_PIN, K_30_WAIT);
		if (strResult == "NORMAL")
			nResult = WFS_SUCCESS;
		else
			nResult = WFS_ERR_HARDWARE_ERROR;
	}

	SysFreeString((BSTR)ExportItemType);
	SysFreeString((BSTR)KeyName);

	return nResult;
}

// [#GLDV-2797] AU Kook 2020.04.28 Support Host PK Removal
//long CNHMWICtrl::RKT_ImportRSAPublicKey(LPCTSTR Recv_HSM_PublicKey, LPCTSTR Recv_HSM_Signature)
  long CNHMWICtrl::RKT_ImportRSAPublicKey(LPCTSTR Recv_HSM_PublicKey, LPCTSTR Recv_HSM_Signature, LPCTSTR Usage)
// end of [#GLDV-2797]
{
	NHDEBUG(1, (_T("CNHMWICtrl::RKT_ImportRSAPublicKey  Recv_HSM_PublicKey(%s) Recv_HSM_Signature(%s) Usage(%s)\n"), Recv_HSM_PublicKey, Recv_HSM_Signature, Usage));

	int nResult = 0;

	SetEventMatrix(DEV_PIN, "ImportRSAPublicKey");

// [#GLDV-2797] AU Kook 2020.04.28 Support Host PK Removal
//	nResult = DevPINImportRSAPublicKey(Recv_HSM_PublicKey, Recv_HSM_Signature);
	nResult = DevPINImportRSAPublicKey(Recv_HSM_PublicKey, Recv_HSM_Signature, Usage);
// end of [#GLDV-2797]

	if (nResult == WFS_SUCCESS)
	{
		CString strResult;
		strResult = CheckDeviceAction(DEV_PIN, K_30_WAIT);
		if (strResult == "NORMAL")
			nResult = WFS_SUCCESS;
		else
			nResult = WFS_ERR_HARDWARE_ERROR;

		NHDEBUG(1, (_T("CNHMWICtrl::RKT_ImportRSAPublicKey  strResult(%s) \n"), strResult));
	}

	SysFreeString((BSTR)Recv_HSM_PublicKey);
	SysFreeString((BSTR)Recv_HSM_Signature);
	SysFreeString((BSTR)Usage);

	return nResult;
}

long CNHMWICtrl::RKT_StartKeyExchange()
{
	
	NHDEBUG(1, (_T("CNHMWICtrl::RKT_StartKeyExchange() \n")));
	
	int nResult = 0;

	m_strEPP_RandomNumber.Empty();	// [#2259] Random Number RSA 미지원으로 인해 변수 Name 변경
	
	SetEventMatrix(DEV_PIN, "StartKeyExchange");
	
	nResult = DevPINStartKeyExchange();
	
	if (nResult == WFS_SUCCESS)
	{
		CString strResult;
		strResult = CheckDeviceAction(DEV_PIN, K_30_WAIT);
		if (strResult == "NORMAL")
			nResult = WFS_SUCCESS;
		else
			nResult = WFS_ERR_HARDWARE_ERROR;
	}
	
	return nResult;
}

long CNHMWICtrl::RKT_ImportRSASignedDESKey(long KeyType, LPCTSTR Recv_EPP_RSACrypt, LPCTSTR Recv_TMK_RSACrypt, LPCTSTR Recv_HSM_Signature)
{
	
	NHDEBUG(1, (_T("CNHMWICtrl::RKT_ImportRSASignedDESKey  KeyType(%d), Recv_EPP_RSACrypt(%s) Recv_TMK_RSACrypt(%s) Recv_HSM_Signature(%s)\n"), KeyType, Recv_EPP_RSACrypt, Recv_TMK_RSACrypt, Recv_HSM_Signature));
	
	int nResult = 0;
	
	SetEventMatrix(DEV_PIN, "ImportRSASignedDESKey");
	
	nResult = DevPINImportRSASignedDESKey(KeyType, Recv_EPP_RSACrypt, Recv_TMK_RSACrypt, Recv_HSM_Signature);
	
	if (nResult == WFS_SUCCESS)
	{
		CString strResult;
		strResult = CheckDeviceAction(DEV_PIN, K_30_WAIT);
		if (strResult == "NORMAL")
			nResult = WFS_SUCCESS;
		else
			nResult = WFS_ERR_HARDWARE_ERROR;
	}

	SysFreeString((BSTR)Recv_EPP_RSACrypt);
	SysFreeString((BSTR)Recv_TMK_RSACrypt);
	SysFreeString((BSTR)Recv_HSM_Signature);
	
	return nResult;
}

BSTR CNHMWICtrl::RKT_GetExportedKey()
{
	NHDEBUG(1, (_T("CNHMWICtrl::RKT_GetExportedKey  (%s)\n"), m_strEPP_ExportedKey));
	
	return m_strEPP_ExportedKey.AllocSysString();
}

BSTR CNHMWICtrl::RKT_GetExportedSignedValue()
{
	NHDEBUG(1, (_T("CNHMWICtrl::RKT_GetExportedSignedValue  (%s)\n"), m_strEPP_Signed));
	
	return m_strEPP_Signed.AllocSysString();
}

BSTR CNHMWICtrl::RKT_GetEPP_RandomNumber()
{
	NHDEBUG(1, (_T("CNHMWICtrl::RKT_GetEPP_RandomNumber(%s)\n"), m_strEPP_RandomNumber));		// [#2259] NH KSK 2014.03.13
	
	return m_strEPP_RandomNumber.AllocSysString();	// [#2259] NH KSK 2014.03.13
}

BSTR CNHMWICtrl::RKT_GetEPP_KCV()
{
	NHDEBUG(1, (_T("CNHMWICtrl::RKT_GetEPP_KCV(%s)\n"), m_strEPP_KeyCheckValue));
	
	return m_strEPP_KeyCheckValue.AllocSysString();

}
// end of [#554]

// [#2057] NH KSK 2011.05.04\
// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
long CNHMWICtrl::PinInitializeEDM() 
{
NHDEBUG(1, (_T("CNHMWICtrl::PinInitializeEDM() \n")));

	int nResult = R_ERROR;

	if(!m_bPinOpened)
		return nResult;

	SetEventMatrix(DEV_PIN, "InitializeEDM");

	nResult = DevPINInitializeEDM();

	if (m_pDevCtrl->CheckDeviceAction(DEV_PIN, K_30_WAIT) != NORMAL)
	{
//NHDEBUG(1, (_T("CNHMWICtrl::PinInitializeEDM() != NORMAL \n")));
		return R_ERROR;
	}

//NHDEBUG(1, (_T("CNHMWICtrl::PinInitializeEDM() return \n")));
	return R_NORMAL;
}
// end of [#2057]

// [#2080] NH KSK 2011.07.13
// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
long CNHMWICtrl::RKT_GetExtraCaps()
{
	NHDEBUG(1, (_T("CNHMWICtrl::RKT_GetExtraCaps \n")));

	int nResult = 0;

	DevPINGetExtraCaps();

	NHDEBUG(1, (_T("CNHMWICtrl::RKT_GetExtraCaps() return \n")));

	return nResult;
}

// [#2253] NH KSK 2014.02.21
// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::PIN_GenerateKCV_512K(LPCTSTR KeyName)
{
	NHDEBUG(1, (_T("CNHMWICtrl::PIN_GenerateKCV_512K (%s)\n"), KeyName));

	int nResult = 0;

	m_strEPP_KeyCheckValue.Empty();

	SetEventMatrix(DEV_PIN, "GenerateKCV");

	nResult = DevPIN_GenerateKCV_512K(KeyName);

	NHDEBUG(1, (_T("CNHMWICtrl::PIN_GenerateKCV_512K() == Result(%d) \n"), nResult));

	if (nResult == WFS_SUCCESS)
	{
		// Check Device Action 수행
		if (m_pDevCtrl->CheckDeviceAction(DEV_PIN, K_30_WAIT) == NORMAL)
		{
			NHDEBUG(1, (_T("CNHMWICtrl::PIN_GenerateKCV_512K() == NORMAL \n")));
		}
		else
		{
			// Command 실패 시 ERROR로 Return
			m_strEPP_KeyCheckValue = L"ERROR";
			NHDEBUG(1, (_T("CNHMWICtrl::PIN_GenerateKCV_512K() == ERROR \n")));
		}
	}
	else
	{
		// Command 실패 시 ERROR로 Return
		m_strEPP_KeyCheckValue = L"ERROR";
	}

	NHDEBUG(1, (_T("CNHMWICtrl::PIN_GenerateKCV_512K(%s)(%s) return \n"), KeyName, m_strEPP_KeyCheckValue));

	return m_strEPP_KeyCheckValue.AllocSysString();
}

long CNHMWICtrl::PinGetCertificate(LPCTSTR Certificate)
{
	SetEventMatrix(DEV_PIN, "GetCertificate");
	
	int nResult = DevPINGetCertificate(Certificate);
	if (nResult == WFS_SUCCESS)
	{
		CString strResult;
		strResult = CheckDeviceAction(DEV_PIN, K_30_WAIT);
		if (strResult != "NORMAL")
		{
			NHDEBUG(1, (_T("CheckDeviceAction() returns: %s \n"), strResult));
			nResult = WFS_ERR_HARDWARE_ERROR;
		}
	}

	NHDEBUG(1, (_T("nResult: %d \n"), nResult));
	return nResult;
}

long CNHMWICtrl::PinInitializeEDMEx(LPCTSTR Indent, LPCTSTR Key)
{
	SetEventMatrix(DEV_PIN, "InitializeEDMEx");
	
	int nResult = DevPINInitializeEDMEx(Indent, Key);
	if (nResult == WFS_SUCCESS)
	{
		CString strResult;
		strResult = CheckDeviceAction(DEV_PIN, K_30_WAIT);
		if (strResult != "NORMAL")
		{
			NHDEBUG(1, (_T("CheckDeviceAction() returns: %s \n"), strResult));
			nResult = WFS_ERR_HARDWARE_ERROR;
		}
	}

	NHDEBUG(1, (_T("nResult: %d \n"), nResult));
	return nResult;
}

long CNHMWICtrl::PinImportKeyBlock(LPCTSTR Key, LPCTSTR EncKey, LPCTSTR KeyBlock)
{
	SetEventMatrix(DEV_PIN, "ImportKeyBlock");

	int nResult = DevPINImportKeyBlock(Key, EncKey, KeyBlock);
	if (nResult == WFS_SUCCESS)
	{
		CString strResult;
		strResult = CheckDeviceAction(DEV_PIN, K_30_WAIT);
		if (strResult != "NORMAL")
		{
			NHDEBUG(1, (_T("CheckDeviceAction() returns: %s \n"), strResult));
			nResult = WFS_ERR_HARDWARE_ERROR;
		}
	}

	NHDEBUG(1, (_T("nResult: %d \n"), nResult));
	return nResult;
}

long CNHMWICtrl::PinLoadCertificateEx(LPCTSTR LoadOption, LPCTSTR Signer, LPCTSTR CertificateData)
{
	SetEventMatrix(DEV_PIN, "LoadCertificateEx");

	int nResult = DevPINLoadCertificateEx(LoadOption, Signer, CertificateData);
	if (nResult == WFS_SUCCESS)
	{
		CString strResult;
		strResult = CheckDeviceAction(DEV_PIN, K_30_WAIT);
		if (strResult != "NORMAL")
		{
			NHDEBUG(1, (_T("CheckDeviceAction() returns: %s \n"), strResult));
			nResult = WFS_ERR_HARDWARE_ERROR;
		}
	}

	NHDEBUG(1, (_T("nResult: %d \n"), nResult));
	return nResult;
}

long CNHMWICtrl::PinImportRSAEncipheredPKCS7KeyEx(LPCTSTR RSAKeyIn, LPCTSTR Key, LPCTSTR Use, LPCTSTR LoadOption)
{
	SetEventMatrix(DEV_PIN, "ImportRSAEncipheredPKCS7KeyEx");

	int nResult = DevPINImportRSAEncipheredPKCS7KeyEx(RSAKeyIn, Key, Use, LoadOption);
	if (nResult == WFS_SUCCESS)
	{
		CString strResult;
		strResult = CheckDeviceAction(DEV_PIN, K_30_WAIT);
		if (strResult != "NORMAL")
		{
			NHDEBUG(1, (_T("CheckDeviceAction() returns: %s \n"), strResult));
			nResult = WFS_ERR_HARDWARE_ERROR;
		}
	}

	NHDEBUG(1, (_T("nResult: %d \n"), nResult));
	return nResult;
}

long CNHMWICtrl::PinStartAuthenticateSync(LPCTSTR MethodName, LPCTSTR Parameters)
{
	//SetEventMatrix(DEV_PIN, "PinStartAuthenticateSync");

	int nResult = DevPINStartAuthenticateSync(MethodName, Parameters);
	if (nResult == WFS_SUCCESS)
	{
		//CString strResult;
		//strResult = CheckDeviceAction(DEV_PIN, K_30_WAIT);
		//if (strResult != "NORMAL")
		//{
		//	NHDEBUG(1, (_T("CheckDeviceAction() returns: %s \n"), strResult));
		//	nResult = WFS_ERR_HARDWARE_ERROR;
		//}
	}

	NHDEBUG(1, (_T("nResult: %d \n"), nResult));
	return nResult;
}

//
// TR34 PROPERTIES
//

// GETTERS
BSTR CNHMWICtrl::PinGetCertificateData()							{	return m_strEPP_CertificateData.AllocSysString();			}
BSTR CNHMWICtrl::PinGetCertificateState()							{	return DevPINGetCertificateState().AllocSysString();		}
BSTR CNHMWICtrl::PinGetAuthenticateDataToSign()						{	return DevPINGetAuthenticateDataToSign().AllocSysString();	}
BSTR CNHMWICtrl::PinGetAuthenticateSigner()							{	return DevPINGetAuthenticateSigner().AllocSysString();		}

// SETTERS
void CNHMWICtrl::PinSetAuthenticateSigner(LPCTSTR Signer)			{ 	DevPINSetAuthenticateSigner(Signer);						}
void CNHMWICtrl::PinSetAuthenticateSigKey(LPCTSTR SigKey)			{	DevPINSetAuthenticateSigKey(SigKey);						}
void CNHMWICtrl::PinSetAuthenticateSignedData(LPCTSTR SignedData)	{	DevPINSetAuthenticateSignedData(SignedData);				}
