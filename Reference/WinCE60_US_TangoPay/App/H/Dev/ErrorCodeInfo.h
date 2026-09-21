#ifndef __ERROR_CODE_INFO_H__
#define __ERROR_CODE_INFO_H__

#include ".\Dll\NHCollection.h"
#include ".\Dll\NHReadiniFile.h"
#include ".\Common\ConstDef.h"
#include ".\Dev\DevDefine.h"

#include <vector>
#include <algorithm>

#define		_MAX_SUPPORT_LANGUAGE	6
//#define ENG_MODE			1						// ENGLISH
//#define SPN_MODE			2						// SPANISH
//#define FRN_MODE			3						// FRENCH
//#define CHN_MODE			4						// CHINESE
//#define KOR_MODE			5						// KOREAN
//#define JPN_MODE			6						// JAPANESE

class CErrorCodeData
{
public:
	CString		m_strErrorCode;
	CString		m_strDesctiption[_MAX_SUPPORT_LANGUAGE];
	CString		m_strAction[_MAX_SUPPORT_LANGUAGE];

public:
	CErrorCodeData() {};
	~CErrorCodeData() {};

	/* Operation */
public:
	void Clear()
	{
		m_strErrorCode = L"";

		for (int i = 0; i < _MAX_SUPPORT_LANGUAGE; i++)
		{
			m_strDesctiption[i] = L"";
			m_strAction[i] = L"";
		}
	};

	void SetDescription(int nLanguage, CString strDesc)
	{
		if (nLanguage <= 0 || nLanguage > _MAX_SUPPORT_LANGUAGE)
			return;
	
		if (m_strDesctiption[nLanguage-1].GetLength() > 0)
			m_strDesctiption[nLanguage-1] += L"\n";

		m_strDesctiption[nLanguage-1] += strDesc;
	}

	CString GetDescription(int nLanguage)
	{
		if (nLanguage <= 0 || nLanguage > _MAX_SUPPORT_LANGUAGE)
			return L"";
		
		if (m_strDesctiption[nLanguage-1].GetLength() == 0)
			return m_strDesctiption[0];

		return m_strDesctiption[nLanguage-1];
	};

	void SetAction(int nLanguage, CString strAct)
	{
		if (nLanguage <= 0 || nLanguage > _MAX_SUPPORT_LANGUAGE)
			return;
	
		if (m_strAction[nLanguage-1].GetLength() > 0)
			m_strAction[nLanguage-1] += L"\n";

		m_strAction[nLanguage-1] += strAct;
	}

	CString GetAction(int nLanguage)
	{
		if (nLanguage <= 0 || nLanguage > _MAX_SUPPORT_LANGUAGE)
			return L"";
		
		if (m_strAction[nLanguage-1].GetLength() == 0)
			return m_strAction[0];

		return m_strAction[nLanguage-1];
	};

public:
	CErrorCodeData(const CErrorCodeData &ErrDesc)
	{
		m_strErrorCode = ErrDesc.m_strErrorCode;

		for (int i = 0; i < _MAX_SUPPORT_LANGUAGE; i++)
		{
			m_strDesctiption[i] = ErrDesc.m_strDesctiption[i];
			m_strAction[i] = ErrDesc.m_strAction[i];
		}
	};
	
	CErrorCodeData& operator= (const CErrorCodeData &ErrDesc)
	{
		m_strErrorCode = ErrDesc.m_strErrorCode;

		for (int i = 0; i < _MAX_SUPPORT_LANGUAGE; i++)
		{
			m_strDesctiption[i] = ErrDesc.m_strDesctiption[i];
			m_strAction[i] = ErrDesc.m_strAction[i];
		}

		return *this;
	};
};

class AFX_CLASS_EXPORT CErrorCodeInfo
{
public:
	CErrorCodeInfo();
	~CErrorCodeInfo();

public:
	/* Initialize */
	BOOL	Initialize(CWnd *pDevCmn);													// 초기화 한다.
	BOOL	Deinitialize();																// 해제 한다.

	/* Searching */
	BOOL	Find(CString strErrorCode);													// strErrorCode에 가장 근접한 Error Code를 검색한다.
	BOOL	FindOneOf(CString strErrorCode, BOOL fSmart=FALSE);							// fSmart가 true이면 가장 근접하는 Error Code를 검색하고,
																						// fSmart가 false이면 일치하는 Error Code를 검색한다.
	/* Moving */
	BOOL	MoveFirst();																// First Error code로 이동한다.
	BOOL	MoveLast();																	// Last Error Code로 이동한다.
	BOOL	MoveNext(UINT32 nSize=1);													// 정해진 크기만큼 떨어진 다음 Error Code로 이동한다.
	BOOL	MovePrev(UINT32 nSize=1);													// 정해진 크기만큼 떨어진 이전 Error Code로 이동한다.

	/* Information */
	CString GetErrorCode();																// 현재 위치의 ErrorCode를 가져온다.
	CString	GetDescription(int nLanguage);												// 언어에 해당하는 ErrorCode의 설명을 가져온다.
	CString GetAction(int nLanguage);													// 언어에 해당하는 ErrorCode의 Action을 가져온다.

private:
	BOOL	LoadErrorInformation();														// 주어진 정보 파일에서 에러 정보를 읽어 온다.

	/*	Variable  */
private:
	BOOL			m_bInit;		// Initialize.
	CWnd			*m_pDevCmn;		// DevCmn pointer
	CNHLinkedList< CErrorCodeData >	m_NHErrorCodes;		// Error Code 정보.
	CErrorCodeData		m_ErrorData;

	CMap< CString, LPCTSTR, CErrorCodeData, CErrorCodeData >	m_NHErrorCodesCMap;

	std::vector< CString > m_ECodeList;					// will have sorted error codes.
	std::vector< CString >::iterator m_ECodeListIter;
};

#endif __ERROR_CODE_INFO_H__
