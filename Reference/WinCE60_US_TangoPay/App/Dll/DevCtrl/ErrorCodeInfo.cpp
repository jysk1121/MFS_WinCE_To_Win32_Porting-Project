#include "StdAfx.h"

//#define NH_DEBUG
#include ".\Common\NHDbgApi.h"

#include ".\Common\CmnLib.h"
#include ".\Dev\ErrorCodeInfo.h"


/*-------------------------------------------------------------------
 CLASS    NAME: CErrorCodeInfo
 FUNCTION NAME: CErrorCodeInfo()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CErrorCodeInfo::CErrorCodeInfo()
{
	m_bInit = FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CErrorCodeInfo
 FUNCTION NAME: ~CErrorCodeInfo()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자.
-------------------------------------------------------------------*/
CErrorCodeInfo::~CErrorCodeInfo()
{
	Deinitialize();
}

BOOL CErrorCodeInfo::Initialize(CWnd *pDevCmn)
{
	m_pDevCmn = pDevCmn;

	if (LoadErrorInformation())
		m_bInit = TRUE;

	return TRUE;
}

BOOL CErrorCodeInfo::Deinitialize()
{
	m_bInit = FALSE;
	m_ErrorData.Clear();

	//m_NHErrorCodes.DeleteAll();				// [#414] AIREAT 2008.09.05

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CErrorCodeInfo
 FUNCTION NAME: Find()
 RETURN TYPE  : BOOL : 성공 여부
 PARAMETER    : 검색하고자 하는 Error Code(Length : 1 ~ 7)
 DESCRIPTION  : strErrorCode에 가장 근접한 ErrorCode를 검색한다.
-------------------------------------------------------------------*/
BOOL CErrorCodeInfo::Find(CString strErrorCode)
{
	if (!m_bInit)
		return FALSE;

	int nKeyLen = strErrorCode.GetLength();

	if(nKeyLen < 1)
		return FALSE;
	else if(nKeyLen > 7)		// Length 7이상일 경우 7자 이상은 버린다.
		strErrorCode = strErrorCode.Left(7);

	if (m_ErrorData.m_strErrorCode == strErrorCode)
		return TRUE;

	return FindOneOf(strErrorCode, TRUE);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CErrorCodeInfo
 FUNCTION NAME: FindOneOf()
 RETURN TYPE  : BOOL : 성공 여부
 PARAMETER    : srtErrorCode - 검색하고자 하는 Error Code,
				fSmart - true(근접한 코드), false(일치하는 코드)
 DESCRIPTION  : 근접하거나 일치하는 ErrorCode를 검색한다.
-------------------------------------------------------------------*/
BOOL CErrorCodeInfo::FindOneOf(CString strErrorCode, BOOL fSmart)
{
	if (!m_bInit)
		return FALSE;

	CErrorCodeData	TempData;

	if (m_ErrorData.m_strErrorCode == strErrorCode)
		return TRUE;

	int nMinLen = strErrorCode.GetLength();
	if (fSmart)									// true일 경우 가장 근접한 Error Code를 찾기위해 1자리까지 같은지 검색하기 위한 조건 설정.
		nMinLen = 1;

	while (strErrorCode.GetLength() >= nMinLen && nMinLen > 0)	// nMinLen Length내에서 검색
	{
		TempData.Clear();

// 		if (m_NHErrorCodes.FindDataOneOfList(strErrorCode, TempData))			// [#414] AIREAT 2008.09.05
		if (m_NHErrorCodesCMap.Lookup(strErrorCode, TempData))
		{
			m_ErrorData = TempData;
			m_ECodeListIter = std::find(m_ECodeList.begin(), m_ECodeList.end(), strErrorCode);
			return TRUE;
		}
		strErrorCode = strErrorCode.Left(strErrorCode.GetLength()-1);
	}
	
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CErrorCodeInfo
 FUNCTION NAME: MoveFirst()
 RETURN TYPE  : BOOL : 성공 여부
 PARAMETER    : 
 DESCRIPTION  : 현재 위치에서 처음 Error Code로 이동한다.
-------------------------------------------------------------------*/
BOOL CErrorCodeInfo::MoveFirst()
{
	if (!m_bInit)
		return FALSE;

// 	if (m_NHErrorCodes.MoveFirst(m_ErrorData))
// 		return TRUE;

	m_ECodeListIter = m_ECodeList.begin();
	if (m_NHErrorCodesCMap.Lookup(*m_ECodeListIter, m_ErrorData))
		return TRUE;

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CErrorCodeInfo
 FUNCTION NAME: MoveLast()
 RETURN TYPE  : BOOL : 성공 여부
 PARAMETER    : 
 DESCRIPTION  : 현재 위치에서 마지막 Error Code로 이동한다.
-------------------------------------------------------------------*/
BOOL CErrorCodeInfo::MoveLast()
{
	if (!m_bInit)
		return FALSE;

	//if (m_NHErrorCodes.MoveLast(m_ErrorData))
	//	return TRUE;

	m_ECodeListIter = m_ECodeList.end() - 1;
	if (m_NHErrorCodesCMap.Lookup(*m_ECodeListIter, m_ErrorData))
		return TRUE;

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CErrorCodeInfo
 FUNCTION NAME: MoveNext()
 RETURN TYPE  : BOOL : 성공 여부
 PARAMETER    : 이동 크기.
 DESCRIPTION  : 현재 위치에서 정해진 크기만큼 다음 Error Code로 이동한다.
-------------------------------------------------------------------*/
BOOL CErrorCodeInfo::MoveNext(UINT32 nSize/* =1 */)
{
	if (!m_bInit)
		return FALSE;

// 	if (m_NHErrorCodes.MoveNext(m_ErrorData, nSize))
// 		return TRUE;
	
	// go one step until the end point.
	for (UINT32 step = 1; step <= nSize; step++)
	{
		if (m_ECodeListIter != m_ECodeList.end() - 1)
		{
			m_ECodeListIter++;
		}
	}

	if (m_NHErrorCodesCMap.Lookup(*m_ECodeListIter, m_ErrorData))
		return TRUE;

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CErrorCodeInfo
 FUNCTION NAME: MovePrev()
 RETURN TYPE  : BOOL : 성공 여부
 PARAMETER    : 이동 크기.
 DESCRIPTION  : 현재 위치에서 정해진 크기만큼 이전 Error Code로 이동한다.
-------------------------------------------------------------------*/
BOOL CErrorCodeInfo::MovePrev(UINT32 nSize/* =1 */)
{
	if (!m_bInit)
		return FALSE;

	//if (m_NHErrorCodes.MovePrev(m_ErrorData, nSize))
	//	return TRUE;

	// go one step back until the beginning point.
	for (UINT32 step = 1; step <= nSize; step++)
	{
		if (m_ECodeListIter != m_ECodeList.begin())
		{
			m_ECodeListIter--;
		}
	}

	if (m_NHErrorCodesCMap.Lookup(*m_ECodeListIter, m_ErrorData))
		return TRUE;

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CErrorCodeInfo
 FUNCTION NAME: GetErrorCode()
 RETURN TYPE  : CString : Error Code
 PARAMETER    : 
 DESCRIPTION  : 현재 위치에 있는 Error Code를 조회한다..
-------------------------------------------------------------------*/
CString CErrorCodeInfo::GetErrorCode()
{
	if (!m_bInit)
		return L"";

	return m_ErrorData.m_strErrorCode;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CErrorCodeInfo
 FUNCTION NAME: GetDescription()
 RETURN TYPE  : CString : Error Description
 PARAMETER    : Language Index
 DESCRIPTION  : 현재 위치에 있는 Error Code의 Description을 조회한다.
-------------------------------------------------------------------*/
CString CErrorCodeInfo::GetDescription(int nLanguage)
{
	if (!m_bInit)
		return L"";

	CString	strDescription = m_ErrorData.GetDescription(nLanguage);

	if (strDescription.IsEmpty())
		strDescription = L"Undefined error code";

	return strDescription;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CErrorCodeInfo
 FUNCTION NAME: GetAction()
 RETURN TYPE  : CString : Error 조치 요령.
 PARAMETER    : Language Index
 DESCRIPTION  : 현재 위치에 있는 Error Code의 action을 조회 한다..
-------------------------------------------------------------------*/
CString CErrorCodeInfo::GetAction(int nLanguage)
{
	if (!m_bInit)
		return L"";
	
	CString strAction = m_ErrorData.GetAction(nLanguage);

	if (strAction.IsEmpty())
		strAction = L"Call your service attendant";

	return strAction;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CErrorCodeInfo
 FUNCTION NAME: GetAction()
 RETURN TYPE  : BOOL : 성공 여부
 PARAMETER    : 
 DESCRIPTION  : 파일에서 주어진 ErrorCode를 읽어 들인다.
-------------------------------------------------------------------*/
BOOL CErrorCodeInfo::LoadErrorInformation()
{
	CNHReadiniFile	Readini;
	PINIVALUE		pwIniValue;
	CErrorCodeData	ErrorData;
	CString strDataFileName;

	for (int i=1; i <= _MAX_SUPPORT_LANGUAGE; i++)
	{
		strDataFileName = ATM_PATH L"\\ERRORCODE\\";		// 디렉토리 수정.

		// Error info File Name.
		switch(i)
		{
		case ENG_MODE:	strDataFileName += "ErrorCode_ENG.dat";	break;
		case SPN_MODE:	strDataFileName += "ErrorCode_SPN.dat";	break;
		case FRN_MODE:  strDataFileName += "ErrorCode_FRN.dat";	break;
		case CHN_MODE:  strDataFileName += "ErrorCode_CHN.dat";	break;
		case KOR_MODE:  strDataFileName += "ErrorCode_KOR.dat";	break;
		case JPN_MODE:  strDataFileName += "ErrorCode_JPN.dat";	break;
		}

		// open Screen Text File
		if (!Readini.Open(strDataFileName, CNHReadiniFile::modeRead, CNHReadiniFile::modeLittleUNICODE))
			continue;

		ErrorData.Clear();
		
		while (NULL != (pwIniValue = Readini.ReadiniValue()))
		{
			// [TEXT] section
			if (pwIniValue->Section == L"ERROR")
			{
				if (pwIniValue->Key == L"ERROR")
				{
					// start
					if (Asc2Int(pwIniValue->Values[0]))
					{
						ErrorData.Clear();
					}
					// end
					else
					{
						// [#382] NH JSW 2008.07.23
						// Add Data..
						if( ErrorData.m_strErrorCode.GetLength() == 7 )
						{
// 							m_NHErrorCodes.PutOnTailOfList(ErrorData.m_strErrorCode, ErrorData);
							m_NHErrorCodesCMap.SetAt(ErrorData.m_strErrorCode, ErrorData);
						}
						else
							ErrorData.Clear();
						// end of [#382]
					}
				}
				else if (pwIniValue->Key == L"CODE")
				{
					//m_NHErrorCodes.FindDataOfList(pwIniValue->Values[0], ErrorData);

// 					if (!m_NHErrorCodes.FindDataOfList(pwIniValue->Values[0], ErrorData))	// 중복 삽입하지 않도록 수정함. 2008.07.24 PSC
// 						ErrorData.m_strErrorCode = pwIniValue->Values[0];
					if (!m_NHErrorCodesCMap.Lookup(pwIniValue->Values[0], ErrorData))
					{
						ErrorData.m_strErrorCode = pwIniValue->Values[0];
						m_ECodeList.push_back(ErrorData.m_strErrorCode);
					}
				}
				else if (pwIniValue->Key == L"DESC")
				{
					pwIniValue->Values[0].TrimRight();	// [#382] NH JSW 2008.07.28 Memory 사용량 감소
					ErrorData.SetDescription(i, pwIniValue->Values[0].Left(40));
				}
				else if (pwIniValue->Key == L"ACTS")
				{
					pwIniValue->Values[0].TrimRight();	// [#382] NH JSW 2008.07.28 Memory 사용량 감소
					ErrorData.SetAction(i, pwIniValue->Values[0].Left(40));
				}
			}
		}

		Readini.Close();
	}

	// 오름 차순 정렬
	//m_NHErrorCodes.Sort();
	std::sort(m_ECodeList.begin(), m_ECodeList.end());
	m_ECodeListIter = m_ECodeList.begin();

	return TRUE;
}
