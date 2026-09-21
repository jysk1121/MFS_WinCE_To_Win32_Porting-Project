#ifndef __MULTI_STRING_H__
#define __MULTI_STRING_H__

#include "..\..\H\Dll\NHCollection.h"
#include "..\..\H\Dll\NHReadiniFile.h"
#include "..\..\H\Dll\AssetArchive.h"
#include "..\..\H\Common\ConstDef.h"
#include "..\..\H\Dev\DevDefine.h"

#define		_MAX_SUPPORT_LANGUAGE	6
#define		AP_TEXT_ID				_T("APTextID:")

class CLocaleString;

//------------------------------------------------------------------
//	CMultiString Define
//------------------------------------------------------------------

class AFX_CLASS_EXPORT CMultiString
{
public:
	CMultiString();
	~CMultiString();

private:
	void	LoadLocaleString();
public:
	BOOL	Initialize();
	BOOL	Deinitialize();
	void	SetLocale(int Locale);
	BOOL	HasAPTextID(CString strTextID);
	CString	GetAPTextIDString(CString strTextID);				// APTextID에 해당하는 문자열을 반환한다.
	CString GetStringByTextID(CString strTextID);				// APTextID에 해당하는 문자열 반환 ("APTextID:" 불필요)

	void	SetLoadingFileName(CString strFileName);			// [#2368] NH KSK 2015.10.16 다른 Path에 있는 File도 Loading 가능하도록 Interface 추가

	// TAR Assets
	CAssetArchive*	m_pTarScreenAsset;
	void			SetTarAsset(void* asset);

private:
	int		m_nCurrentLocale;
	CNHPtrLinkedList< CLocaleString >	m_LocaleStrings;
	CMap< CString, LPCTSTR, CLocaleString*, CLocaleString* >	m_LocaleStringsCMap;
	CString	m_strLoadingFileName;								// [#2368] NH KSK 2015.10.16
};

//------------------------------------------------------------------
//	CLocaleString Define
//------------------------------------------------------------------

class AFX_CLASS_EXPORT CLocaleString
{
public:
	CLocaleString(CString strID, int nSupportLocale);
	~CLocaleString();

public:
	CString	GetID();
	BOOL	AddLocaleText(int Locale, LPCTSTR pText);
	CString GetLocaleText(int Locale);

private:
	CString		m_strID;
	int			m_nLocale;
	CString		*m_pLocaleText;
};


#endif __MULTI_STRING_H__
