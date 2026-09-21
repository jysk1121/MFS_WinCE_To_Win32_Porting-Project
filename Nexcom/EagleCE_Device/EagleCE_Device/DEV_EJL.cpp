#include "stdafx.h"
#include "DEV_Define.h"
#include "DEV_EJL.h"
#include "DeviceSimUtil.h"
//#include "DEV_EJL_Define.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"


/** **********************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CDEV_EJL::CDEV_EJL()
{
	// DLL 경로 설정
	CString	strPath = _T("");

	// API 취득
	m_hDll						= LoadLibrary(EAGLE_ATM_EJR_DLL);

	EJL_Start					= (EJL_START)					GetProcAddress(m_hDll, _T("EJL_Start"));
	EJL_Close					= (EJL_CLOSE)					GetProcAddress(m_hDll, _T("EJL_Close"));
	EJL_InsertJnl				= (EJL_INSERTJNL)				GetProcAddress(m_hDll, _T("EJL_InsertJnl"));
	EJL_GetCurrJnl				= (EJL_GETCURRJNL)				GetProcAddress(m_hDll, _T("EJL_GetCurrJnl"));
	EJL_GetPrevJnl				= (EJL_GETPREVJNL)				GetProcAddress(m_hDll, _T("EJL_GetPrevJnl"));
	EJL_GetSearchCurrJnl		= (EJL_GETSEARCHCURRJNL)		GetProcAddress(m_hDll, _T("EJL_GetSearchCurrJnl"));
	EJL_GetSearchPrevJnl		= (EJL_GETSEARCHPREVJNL)		GetProcAddress(m_hDll, _T("EJL_GetSearchPrevJnl"));
	EJL_GetLastJnlNo			= (EJL_GETLASTJNLNO)			GetProcAddress(m_hDll, _T("EJL_GetLastJnlNo"));
	EJL_GetSearchLastJnlNo		= (EJL_GETSEARCHLASTJNLNO)		GetProcAddress(m_hDll, _T("EJL_GetSearchLastJnlNo"));
	EJL_GetVersion				= (EJL_GETVERSION)				GetProcAddress(m_hDll, _T("EJL_GetVersion"));
	EJL_GetErrInfo				= (EJL_GETERRINFO)				GetProcAddress(m_hDll, _T("EJL_GetErrInfo"));

	EJL_Delete					= (EJL_DELETE)					GetProcAddress(m_hDll, _T("EJL_Delete"));

	EJL_GetAuditJnlNo			= (EJL_GETAUDITJNLNO)			GetProcAddress(m_hDll, _T("EJL_GetPrintedAuditJnlNo"));
	EJL_SetAuditJnlNo			= (EJL_SETAUDITJNLNO)			GetProcAddress(m_hDll, _T("EJL_SetPrintedAuditJnlNo"));
	EJL_GetAuditAMSJnlNo		= (EJL_GETAUDITAMSJNLNO)		GetProcAddress(m_hDll, _T("EJL_GetAuditAMSJnlNo"));
	EJL_SetAuditAMSJnlNo		= (EJL_SETAUDITAMSJNLNO)		GetProcAddress(m_hDll, _T("EJL_SetAuditAMSJnlNo"));
	EJL_GetAuditHOSTJnlNo		= (EJL_GETAUDITHOSTJNLNO)		GetProcAddress(m_hDll, _T("EJL_GetAuditHOSTJnlNo"));
	EJL_SetAuditHOSTJnlNo		= (EJL_SETAUDITHOSTJNLNO)		GetProcAddress(m_hDll, _T("EJL_SetAuditHOSTJnlNo"));

	// The DLL fills m_szErrorCode through an LPSTR out-parameter. Zero it up
	// front so that a failed/absent DLL can never expose uninitialised bytes
	// through GetLastError().
	memset(m_szErrorCode, 0, sizeof(m_szErrorCode));

	m_bOpened = FALSE;

}


/** **********************************************************
*	@brief		소멸자
*	@retval		없음
************************************************************/
CDEV_EJL::~CDEV_EJL()
{
	m_bOpened = FALSE;

	if (m_hDll)
	{
		FreeLibrary(m_hDll);

		m_hDll = NULL;
	}
}


/** **********************************************************
*	@brief		열기
*	@retval		없음
************************************************************/
BOOL CDEV_EJL::Open(HWND hWnd)
{
#if EMULATION_EJL_DEVICE
	// Simulated journal: the WinCE/ARM DLL is absent, report the device as ready.
	m_bOpened = TRUE;
	memset(m_szErrorCode, 0, sizeof(m_szErrorCode));
	return TRUE;
#endif

	if (!m_hDll)
	{
		m_bOpened = FALSE;

		// Leave a meaningful code behind: callers log GetLastError() straight
		// after a failed Open(), and the DLL never got a chance to set one.
		strncpy(m_szErrorCode, "Z0DLL", sizeof(m_szErrorCode) - 1);
		m_szErrorCode[sizeof(m_szErrorCode) - 1] = '\0';

		return FALSE;
	}

	m_bOpened = TRUE;

	return EJL_Start(m_szErrorCode);
}


/** **********************************************************
*	@brief		닫기
*	@retval		없음
************************************************************/
BOOL CDEV_EJL::Close()
{
#if EMULATION_EJL_DEVICE
	m_bOpened = FALSE;
	return TRUE;
#endif

	m_bOpened = FALSE;

	if (!m_hDll)
	{
		return FALSE;
	}

	return EJL_Close(m_szErrorCode);
}


/** **********************************************************
*	@brief		버전 취득
*	@retval		없음
************************************************************/
CString  CDEV_EJL::GetVersion(void)
{
#if EMULATION_EJL_DEVICE
	return CDevSim::ReadString(_T("EJL"), _T("Version"), _T("1.0"));
#endif

	if (m_bOpened == FALSE)
		return _T("");

	if (!m_hDll)
	{
		return _T("");
	}


	FLSEJVERSION Version;

	EJL_GetVersion(&Version);


	CString strVersion = _T("");

	strVersion.Format(_T("%d.%d"), Version.major, Version.minor);

	return strVersion;
}


/** **********************************************************
*	@brief		마지막 에러 취득
*	@retval		없음
************************************************************/
CString CDEV_EJL::GetLastError(void)
{
	// The buffer is filled by the DLL through an LPSTR out-parameter, so it is
	// not guaranteed to be terminated. Bound the conversion to the buffer size
	// rather than letting CString run past the end of the member.
	CHAR szSafe[sizeof(m_szErrorCode) + 1] = { 0, };

	memcpy(szSafe, m_szErrorCode, sizeof(m_szErrorCode));
	szSafe[sizeof(m_szErrorCode)] = '\0';

	return CString(szSafe);
}


/** **********************************************************
*	@brief		상세 Journal Data 취득
*	@retval		없음
************************************************************/
BOOL CDEV_EJL::GetDetailJnlData(int nJnlNo, LPFLSEJCOMMINFO lpCommInfo, LPFLSEJINFO lpJnlInfo)
{
#if EMULATION_EJL_DEVICE
	// No stored journal to hand back - say so rather than leaving the
	// caller-supplied structures uninitialised.
	return FALSE;
#endif

	if (m_bOpened == FALSE)
		return FALSE;

	//LOG(Info, _T("GetDetailJnlData - Jnl No - %d"), nJnlNo);

	return EJL_GetCurrJnl(nJnlNo, lpCommInfo, lpJnlInfo, m_szErrorCode);
}


/** **********************************************************
*	@brief		Journal Data 검색
*	@retval		없음
************************************************************/
BOOL CDEV_EJL::GetSearchJnlDataList(int nType, LPCTSTR pszFrom, LPCTSTR pszTo, LPFLSEJCOMMINFO lpCommInfo, int nCommCount, int& nJnlCount)
{
#if EMULATION_EJL_DEVICE
	nJnlCount = 0;	// empty journal
	return TRUE;
#endif

	if (m_bOpened == FALSE)
		return FALSE;

	nJnlCount = 0;


	CString strFrom(pszFrom);

	CString strTo(pszTo);


	CT2A szFrom(pszFrom);

	CT2A szTo(pszTo);

	LOG(Info, _T("GetSearchJnlDataList - From - %s / To - %s"), pszFrom, pszTo);

	// 마지막 저널 번호 취득
	int nJnlNo = 0;

	if (strTo.IsEmpty())
	{
		if (FALSE == EJL_GetLastJnlNo(&nJnlNo, m_szErrorCode))
		{
			return FALSE;
		}
	}
	else
	{
		if (FALSE == EJL_GetSearchLastJnlNo(nType, szTo.m_psz, &nJnlNo, m_szErrorCode))
		{
			return FALSE;
		}
	}


	LPFLSEJCOMMINFO lpTempCommInfo = new FLSEJCOMMINFO;

	LPFLSEJINFO pTempJnlInfo = new FLSEJINFO;

	LOG(Info, _T("GetSearchJnlDataList - LastJnlNo - %d"), nJnlNo);

	for (int nCnt = 0; nCnt < nCommCount; nCnt++)
	{
		memset(lpTempCommInfo, 0, sizeof(FLSEJCOMMINFO));
		memset(pTempJnlInfo, 0, sizeof(FLSEJINFO));

		if (0 == nCnt)
		{
			if (FALSE == EJL_GetSearchCurrJnl(nType, szFrom.m_psz, szTo.m_psz, nJnlNo, lpTempCommInfo, pTempJnlInfo, m_szErrorCode))
			{
				break;
			}
		}
		else
		{
			if (FALSE == EJL_GetSearchPrevJnl(nType, szFrom.m_psz, szTo.m_psz, nJnlNo, lpTempCommInfo, pTempJnlInfo, m_szErrorCode))
			{
				break;
			}
		}

		nJnlNo = lpTempCommInfo->jnl_no;

		memcpy(&lpCommInfo[nCnt], lpTempCommInfo, sizeof(FLSEJCOMMINFO));

		nJnlCount++;
	}

	LOG(Info, _T("GetSearchJnlDataList - Existed Journal Count - %d"), nJnlCount);

	delete lpTempCommInfo;

	delete pTempJnlInfo;


	return TRUE;
}


/** **********************************************************
*	@brief		프린트 Journal 번호 취득
*	@retval		없음
************************************************************/
BOOL CDEV_EJL::GetPrintJnlNoList(LPINT pJnlNoList, int nListCount, int& nJnlCount)
{
#if EMULATION_EJL_DEVICE
	nJnlCount = 0;	// nothing pending to print
	return TRUE;
#endif

	if (m_bOpened == FALSE)
		return FALSE;

	nJnlCount = 0;


	// 마지막 저널 번호 취득
	int nJnlNo = 0;

	if (FALSE == EJL_GetLastJnlNo(&nJnlNo, m_szErrorCode))
	{
		return FALSE;
	}


	// 감사 저널 번호 취득
	int nAuditJnlNo = 0;

	if (FALSE == EJL_GetAuditJnlNo(&nAuditJnlNo, m_szErrorCode))
	{
		return FALSE;
	}


	LPFLSEJCOMMINFO lpTempCommInfo = new FLSEJCOMMINFO;

	LPFLSEJINFO pTempJnlInfo = new FLSEJINFO;


	for (int nCnt = 0; nCnt < nListCount; nCnt++)
	{
		memset(lpTempCommInfo, 0, sizeof(FLSEJCOMMINFO));
		memset(pTempJnlInfo, 0, sizeof(FLSEJINFO));

		if (0 == nCnt)
		{
			if (FALSE == EJL_GetCurrJnl(nJnlNo, lpTempCommInfo, pTempJnlInfo, m_szErrorCode))
			{
				break;
			}
		}
		else
		{
			if (FALSE == EJL_GetPrevJnl(nJnlNo, lpTempCommInfo, pTempJnlInfo, m_szErrorCode))
			{
				break;
			}
		}

		nJnlNo = lpTempCommInfo->jnl_no;


		// 감사 저널 번호 체크
		if (nJnlNo <= nAuditJnlNo)
		{
			break;
		}

		pJnlNoList[nCnt] = nJnlNo;

		nJnlCount++;
	}


	delete lpTempCommInfo;

	delete pTempJnlInfo;


	return TRUE;
}


/** **********************************************************
*	@brief		프린트 Journal Data 취득
*	@retval		없음
************************************************************/
BOOL CDEV_EJL::GetPrintJnlDataList(LPFLSEJCOMMINFO lpCommInfo, int nCommCount, int& nJnlCount)
{
#if EMULATION_EJL_DEVICE
	nJnlCount = 0;	// nothing pending to print
	return TRUE;
#endif

	if (m_bOpened == FALSE)
		return FALSE;

	nJnlCount = 0;


	// 마지막 저널 번호 취득
	int nJnlNo = 0;

	if (FALSE == EJL_GetLastJnlNo(&nJnlNo, m_szErrorCode))
	{
		return FALSE;
	}


	// 감사 저널 번호 취득
	int nAuditJnlNo = 0;

	if (FALSE == EJL_GetAuditJnlNo(&nAuditJnlNo, m_szErrorCode))
	{
		return FALSE;
	}


	LPFLSEJCOMMINFO lpTempCommInfo = new FLSEJCOMMINFO;

	LPFLSEJINFO pTempJnlInfo = new FLSEJINFO;


	for (int nCnt = 0; nCnt < nCommCount; nCnt++)
	{
		memset(lpTempCommInfo, 0, sizeof(FLSEJCOMMINFO));
		memset(pTempJnlInfo, 0, sizeof(FLSEJINFO));

		if (0 == nCnt)
		{
			if (FALSE == EJL_GetCurrJnl(nJnlNo, lpTempCommInfo, pTempJnlInfo, m_szErrorCode))
			{
				break;
			}
		}
		else
		{
			if (FALSE == EJL_GetPrevJnl(nJnlNo, lpTempCommInfo, pTempJnlInfo, m_szErrorCode))
			{
				break;
			}
		}

		nJnlNo = lpTempCommInfo->jnl_no;


		// 감사 저널 번호 체크
		if (nJnlNo <= nAuditJnlNo)
		{
			break;
		}

		memcpy(&lpCommInfo[nCnt], lpTempCommInfo, sizeof(FLSEJCOMMINFO));

		nJnlCount++;
	}


	delete lpTempCommInfo;

	delete pTempJnlInfo;


	return TRUE;
}


/** **********************************************************
*	@brief		프린트 다음 Journal Data 취득
*	@retval		없음
************************************************************/
BOOL CDEV_EJL::GetPrintNextJnlDataList(int nJnlNo, LPFLSEJCOMMINFO lpCommInfo, int nCommCount, int& nJnlCount)
{
#if EMULATION_EJL_DEVICE
	nJnlCount = 0;	// nothing pending to print
	return TRUE;
#endif

	if (m_bOpened == FALSE)
		return FALSE;

	nJnlCount = 0;


	// 감사 저널 번호 취득
	int nAuditJnlNo = 0;

	if (FALSE == EJL_GetAuditJnlNo(&nAuditJnlNo, m_szErrorCode))
	{
		return FALSE;
	}


	LPFLSEJCOMMINFO lpTempCommInfo = new FLSEJCOMMINFO;

	LPFLSEJINFO pTempJnlInfo = new FLSEJINFO;


	for (int nCnt = 0; nCnt < nCommCount; nCnt++)
	{
		memset(lpTempCommInfo, 0, sizeof(FLSEJCOMMINFO));
		memset(pTempJnlInfo, 0, sizeof(FLSEJINFO));

		if (FALSE == EJL_GetPrevJnl(nJnlNo, lpTempCommInfo, pTempJnlInfo, m_szErrorCode))
		{
			break;
		}

		nJnlNo = lpTempCommInfo->jnl_no;


		// 감사 저널 번호 체크
		if (nJnlNo <= nAuditJnlNo)
		{
			break;
		}

		memcpy(&lpCommInfo[nCnt], lpTempCommInfo, sizeof(FLSEJCOMMINFO));

		nJnlCount++;
	}


	delete lpTempCommInfo;

	delete pTempJnlInfo;


	return TRUE;
}


/** **********************************************************
*	@brief		프린트 Journal Data 설정
*	@retval		없음
************************************************************/
BOOL CDEV_EJL::SetPrintJnlData(int nJnlNo)
{
#if EMULATION_EJL_DEVICE
	return TRUE;
#endif

	if (m_bOpened == FALSE)
		return FALSE;

	return EJL_SetAuditJnlNo(nJnlNo, m_szErrorCode);
}


/** **********************************************************
*	@brief		Journal Data 추가
*	@retval		없음
************************************************************/
BOOL CDEV_EJL::InsertJnlData(LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt)
{
#if EMULATION_EJL_DEVICE
	// Journal writes happen on every transaction step - succeed silently.
	return TRUE;
#endif

	BOOL bResult = FALSE;

	if (m_bOpened == FALSE)
		return FALSE;

	bResult = EJL_InsertJnl(lpCommInfo, lpJnlDt, m_szErrorCode);

	if (bResult == FALSE)
	{
		LOG(Error, _T("InsertJnlData is failed (%S)"), m_szErrorCode);
	}

	return bResult;
}


/** **********************************************************
*	@brief		Delete
*	@retval		없음
************************************************************/
BOOL CDEV_EJL::Delete(void)
{
#if EMULATION_EJL_DEVICE
	return TRUE;
#endif

	if (m_bOpened == FALSE)
		return FALSE;

	return EJL_Delete(m_szErrorCode);
}


/** **********************************************************
*	@brief		Get Errorcode Info
*	@retval		없음
************************************************************/
BOOL CDEV_EJL::GetErrorCodeInfo(LPCTSTR pszErrcode, LPERRCODEINFO lpErrInfo)
{
#if EMULATION_EJL_DEVICE
	// No error-code database available; caller falls back to its own text.
	return FALSE;
#endif

	if (m_bOpened == FALSE)
		return FALSE;

	if (!m_hDll)
	{
		return FALSE;
	}

	CString strErrcode(pszErrcode);
	CT2A szErrcode(pszErrcode);

	return EJL_GetErrInfo(szErrcode.m_psz, lpErrInfo, m_szErrorCode);
}

/** **********************************************************
*	@brief		GetLastJournalNo : 현재 저장된 저널 개수
*	@retval		없음
************************************************************/
int CDEV_EJL::GetLastJournalNo()
{
#if EMULATION_EJL_DEVICE
	// Must stay below MAX_JOURNAL_COUNT or IsTerminal_Error() reports
	// "journal full" and the terminal goes out of service.
	return CDevSim::ReadInt(_T("EJL"), _T("LastJournalNo"), 0);
#endif

	if (m_bOpened == FALSE)
		return 0;

	int nLastJnlNo = 0;

	if (FALSE == EJL_GetLastJnlNo(&nLastJnlNo, m_szErrorCode))
	{
		LOG(Error, _T("EJL_GetLastJnlNo return false (%S)"), m_szErrorCode);
	}
		
	return nLastJnlNo;
}


/** **********************************************************
*	@brief		Supervisor에서 Printed한 저널 No 구하기
*	@retval		없음
************************************************************/
int CDEV_EJL::GetPrintedJnlNo()
{
#if EMULATION_EJL_DEVICE
	return CDevSim::ReadInt(_T("EJL"), _T("PrintedJnlNo"), 0);
#endif

	if (m_bOpened == FALSE)
		return 0;

	int nPrintededJnlNo = 0;

	if (FALSE == EJL_GetAuditJnlNo(&nPrintededJnlNo, m_szErrorCode))
	{
		LOG(Error, _T("GetPrintedJnlNo return false (%S)"), m_szErrorCode);
	}

	return nPrintededJnlNo;
}


/** **********************************************************
*	@brief		Supervisor에서 Printed한 저널 No Set
*	@retval		성공 : TRUE, 실패 : FALSE
************************************************************/
BOOL CDEV_EJL::SetPrintedJnlNo(int nPrintededJnlNo)
{
#if EMULATION_EJL_DEVICE
	return TRUE;
#endif

	if (m_bOpened == FALSE)
		return FALSE;

	if (FALSE == EJL_SetAuditJnlNo(nPrintededJnlNo, m_szErrorCode))
	{
		LOG(Error, _T("SetPrintedJnlNo return false (%S)"), m_szErrorCode);
		return FALSE;
	}

	return TRUE;
}


/** **********************************************************
*	@brief		AMS로 Upload한 저널 No 구하기
*	@retval		없음
************************************************************/
int CDEV_EJL::GetUploadedAMSJnlNo()
{
#if EMULATION_EJL_DEVICE
	return CDevSim::ReadInt(_T("EJL"), _T("UploadedAMSJnlNo"), 0);
#endif

	if (m_bOpened == FALSE)
		return 0;

	int nUploadedJnlNo = 0;

	if (FALSE == EJL_GetAuditAMSJnlNo(&nUploadedJnlNo, m_szErrorCode))
	{
		LOG(Error, _T("GetUploadedAMSJnlNo return false (%S)"), m_szErrorCode);
	}

	return nUploadedJnlNo;
}


/** **********************************************************
*	@brief		AMS로 Upload한 Jnl No Set
*	@retval		성공 : TRUE, 실패 : FALSE
************************************************************/
BOOL CDEV_EJL::SetUploadedAMSJnlNo(int nUploadedJnlNo)
{
#if EMULATION_EJL_DEVICE
	return TRUE;
#endif

	if (m_bOpened == FALSE)
		return FALSE;

	if (FALSE == EJL_SetAuditAMSJnlNo(nUploadedJnlNo, m_szErrorCode))
	{
		LOG(Error, _T("SetUploadedAMSJnlNo return false (%S)"), m_szErrorCode);
		return FALSE;
	}

	return TRUE;
}


/** **********************************************************
*	@brief		HOST로 Upload한 저널 No 구하기
*	@retval		없음
************************************************************/
int CDEV_EJL::GetUploadedHOSTJnlNo()
{
#if EMULATION_EJL_DEVICE
	return CDevSim::ReadInt(_T("EJL"), _T("UploadedHOSTJnlNo"), 0);
#endif

	if (m_bOpened == FALSE)
		return 0;

	int nUploadedJnlNo = 0;

	if (FALSE == EJL_GetAuditHOSTJnlNo(&nUploadedJnlNo, m_szErrorCode))
	{
		LOG(Error, _T("GetUploadedHOSTJnlNo return false (%S)"), m_szErrorCode);
	}

	return nUploadedJnlNo;
}


/** **********************************************************
*	@brief		HOST로 Upload한 Jnl No Set
*	@retval		성공 : TRUE, 실패 : FALSE
************************************************************/
BOOL CDEV_EJL::SetUploadedHOSTJnlNo(int nUploadedJnlNo)
{
#if EMULATION_EJL_DEVICE
	return TRUE;
#endif

	if (m_bOpened == FALSE)
		return FALSE;

	if (FALSE == EJL_SetAuditHOSTJnlNo(nUploadedJnlNo, m_szErrorCode))
	{
		LOG(Error, _T("SetUploadedHOSTJnlNo return false (%S)"), m_szErrorCode);
		return FALSE;
	}

	return TRUE;
}
