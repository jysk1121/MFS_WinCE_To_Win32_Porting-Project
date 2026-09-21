#include "StdAfx.h"
#include ".\Common\CmnLib.h"
#include "..\..\H\Dev\MultiString.h"
#include "..\..\H\Dll\AssetArchive.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1


/*-------------------------------------------------------------------
 CLASS    NAME: CMultiString
 FUNCTION NAME: CMultiString()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CMultiString::CMultiString()
{
	m_nCurrentLocale = 0;
	m_strLoadingFileName.Empty();
	m_pTarScreenAsset = NULL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMultiString
 FUNCTION NAME: ~CMultiString()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자.
-------------------------------------------------------------------*/
CMultiString::~CMultiString()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMultiString
 FUNCTION NAME: Initialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 Locale 설정.
-------------------------------------------------------------------*/
BOOL CMultiString::Initialize()
{
	LoadLocaleString();
	return TRUE;
}

void CMultiString::SetTarAsset(void* asset)
{
	m_pTarScreenAsset = static_cast<CAssetArchive*>(asset);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMultiString
 FUNCTION NAME: Deinitialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 Locale 설정.
-------------------------------------------------------------------*/
BOOL CMultiString::Deinitialize()
{
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMultiString
 FUNCTION NAME: SetLocale()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 Locale 설정.
-------------------------------------------------------------------*/
void CMultiString::SetLocale(int Locale)
{
	m_nCurrentLocale = Locale;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMultiString
 FUNCTION NAME: LoadLocaleString()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Locale String을 파일로 부터 읽어 들인다.
-------------------------------------------------------------------*/
void CMultiString::LoadLocaleString()
{
	CNHReadiniFile	Readini;
	PINIVALUE	pwIniValue;
	CString		strSupportLanguage, strLoadCountry;
	CString		strTemp;
	CLocaleString	*pLocaleText = NULL;

	// Get Country Short cut
#if	(US_VERSION)
	strLoadCountry = _T("US");
#elif (AU_VERSION)
	strLoadCountry = _T("AU");
#elif (CA_VERSION)
	strLoadCountry = _T("CA");
#elif (MX_VERSION)
	strLoadCountry = _T("MX");	// [#2115] MX KSK 2012.02.05
#endif

	// open Screen Text File
	CString MultiTextName;
	// [#2368] NH KSK 2015.10.16
//	MultiTextName.Format(L"%s\\MultiText.dat", SCREEN_PATH);
	if (m_strLoadingFileName.IsEmpty())	// File Location을 설정하지 않을 경우 기존 호환을 위해 SCREEN FOLDER 사용
		MultiTextName.Format(L"%s\\MultiText.dat", SCREEN_PATH);
	else
		MultiTextName.Format(L"%s", m_strLoadingFileName);
	// end of [#2368]

	// try TAR first.
	BOOL bDatOpened = FALSE;
	if (m_pTarScreenAsset)
	{
		unsigned int fileSize = m_pTarScreenAsset->GetFileSize(MultiTextName);
		if (fileSize > 0)
		{
			BYTE* pTarBuffer = new BYTE[fileSize];
			m_pTarScreenAsset->ReadFile(MultiTextName, pTarBuffer);
			bDatOpened = Readini.Open(pTarBuffer, CNHReadiniFile::modeRead, CNHReadiniFile::modeLittleUNICODE);
			delete[] pTarBuffer;
		}
	}
	else
	{
		bDatOpened = Readini.Open(MultiTextName, CNHReadiniFile::modeRead, CNHReadiniFile::modeLittleUNICODE);	// [#11] NH KSK 2010.09.14
	}

	if (bDatOpened)	
	{
		// Load Screen Text
		while (NULL != (pwIniValue = Readini.ReadiniValue()))
		{
			// [SUPPORT] section
			if (pwIniValue->Section == _T("SUPPORT"))
			{
				if (pwIniValue->Key.Left(2) == strLoadCountry && _ttoi(pwIniValue->Values[0]) == 1)
				{
					strSupportLanguage += pwIniValue->Key.Right(3);
					strSupportLanguage += _T(",");
				}
			}		
			// [TEXT] section
			else if (pwIniValue->Section == _T("TEXT"))
			{
				if (pwIniValue->Key == _T("ID"))
				{
					pwIniValue->Values[0].MakeUpper();
					pLocaleText = (CLocaleString*) new CLocaleString(pwIniValue->Values[0], LANGUAGE_MODE_MAX);
					if (pLocaleText)
					{
// 						m_LocaleStrings.PutOnTailOfList(pLocaleText->GetID(), pLocaleText);
						m_LocaleStringsCMap.SetAt(pLocaleText->GetID(), pLocaleText);
					}
				}
				else if (strSupportLanguage.Find(pwIniValue->Key.Right(3)) != -1)
				{
					strTemp = pwIniValue->Key.Left(2);
					if (strTemp == strLoadCountry || strTemp == _T("US"))
					{
						strTemp = pwIniValue->Key.Right(3);

						if (pLocaleText)	// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
						{
							if (strTemp == _T("ENG"))
								pLocaleText->AddLocaleText(ENG_MODE, pwIniValue->Values[0]);
							else if (strTemp == _T("SPN"))
								pLocaleText->AddLocaleText(SPN_MODE, pwIniValue->Values[0]);
							else if (strTemp == _T("FRN"))
								pLocaleText->AddLocaleText(FRN_MODE, pwIniValue->Values[0]);
							else if (strTemp == _T("CHN"))
								pLocaleText->AddLocaleText(CHN_MODE, pwIniValue->Values[0]);
							else if (strTemp == _T("KOR"))
								pLocaleText->AddLocaleText(KOR_MODE, pwIniValue->Values[0]);
							else if (strTemp == _T("JPN"))
								pLocaleText->AddLocaleText(JPN_MODE, pwIniValue->Values[0]);
						}
					}
				}
			}
		}
	}
	else
	{
		NHDEBUG(1, (_T("Multi text file open failed.\n")));
	}

	Readini.Close();

	return;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMultiString
 FUNCTION NAME: HasAPTextID()
 RETURN TYPE  : BOOL
 PARAMETER    : 
 DESCRIPTION  : Returns true when the string has been registered
-------------------------------------------------------------------*/
BOOL CMultiString::HasAPTextID(CString strTextID)
{
	CLocaleString	*pText = NULL;

	if (!_tcsncmp(strTextID, AP_TEXT_ID, 9) && (strTextID.GetLength() > 9))
	{
		CString strKey = strTextID.Mid(9);
		strKey.MakeUpper();

		return m_LocaleStringsCMap.Lookup(strKey, pText);
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMultiString
 FUNCTION NAME: GetAPTextIDString()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : APTextID에 해당하는 문자열을 반환한다..
-------------------------------------------------------------------*/
CString	CMultiString::GetAPTextIDString(CString strTextID)
{
	CString strTextString = _T("");
	CLocaleString	*pText = NULL;

	if (!_tcsncmp(strTextID, AP_TEXT_ID, 9) && (strTextID.GetLength() > 9))
	{
		CString strKey = strTextID.Mid(9);
		strKey.MakeUpper();		// KSK 2012.12.21
// 		if (m_LocaleStrings.FindDataOfList(strKey, (CLocaleString**)&pText))
// 			strTextString = pText->GetLocaleText(m_nCurrentLocale);
		if (m_LocaleStringsCMap.Lookup(strKey, pText))
		{
			strTextString = pText->GetLocaleText(m_nCurrentLocale);
		}
	}

	return strTextString;
}

// [#2220] NH KMK 2014.02.10 
/*-------------------------------------------------------------------
CLASS    NAME: CMultiString
FUNCTION NAME: GetStringByTextID()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : APTextID에 해당하는 문자열을 반환한다.
			   GetAPTextIDString() 함수와 달리 파라미터 값에 "APTextID:" 문자열을 필요로 하지 않는다.
			   (TextID 자체를 파라미터로 받음)
-------------------------------------------------------------------*/
CString	CMultiString::GetStringByTextID(CString strTextID)
{
	CString strTextString = _T("");
	CLocaleString	*pText = NULL;

	strTextID.MakeUpper();

// 	if (m_LocaleStrings.FindDataOfList(strTextID, (CLocaleString**)&pText))
// 		strTextString = pText->GetLocaleText(m_nCurrentLocale);
	if (m_LocaleStringsCMap.Lookup(strTextID, pText))
		strTextString = pText->GetLocaleText(m_nCurrentLocale);

	return strTextString;
}
// end of [#2220]

// [#2368] NH KSK 2015.10.16
/*-------------------------------------------------------------------
CLASS    NAME: CMultiString
FUNCTION NAME: SetLoadingFileName()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Screen File이외의 위치에 있는 File 사용을 위해 Full Path 및 File Name을 Set함
-------------------------------------------------------------------*/
void CMultiString::SetLoadingFileName(CString strFileName)
{
	m_strLoadingFileName = strFileName;
}
// end of [#2368]

/*-------------------------------------------------------------------
 CLASS    NAME: CLocaleString
 FUNCTION NAME: CLocaleString()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CLocaleString::CLocaleString(CString strID, int nSupportLocale)
{
	m_strID = strID;
	m_nLocale = nSupportLocale;
	m_pLocaleText = NULL;

	if (m_nLocale > 0)
		m_pLocaleText = (CString*) new CString[m_nLocale];
}

/*-------------------------------------------------------------------
 CLASS    NAME: CLocaleString
 FUNCTION NAME: ~CLocaleString()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자.
-------------------------------------------------------------------*/
CLocaleString::~CLocaleString()
{
	if (m_pLocaleText != NULL)
		delete [] m_pLocaleText;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CLocaleString
 FUNCTION NAME: GetID()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : ID를 조회한다.
-------------------------------------------------------------------*/
CString CLocaleString::GetID()
{
	return m_strID;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CLocaleString
 FUNCTION NAME: AddLocaleText()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Locale 별 스트링 등록
-------------------------------------------------------------------*/
BOOL CLocaleString::AddLocaleText(int Locale, LPCTSTR pText)
{
	if (Locale <= 0 || Locale > m_nLocale)
		return FALSE;

	m_pLocaleText[Locale-1] = pText;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CLocaleString
 FUNCTION NAME: GetLocaleText()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Locale 별 스트링 조회
-------------------------------------------------------------------*/
CString CLocaleString::GetLocaleText(int Locale)
{
	// check Locale
	if (Locale <= 0 || Locale > m_nLocale)
	{
		if (m_nLocale >= 0)
			return m_pLocaleText[0];	// Default String.
		
		return _T("");
	}

	return m_pLocaleText[Locale-1];
}
