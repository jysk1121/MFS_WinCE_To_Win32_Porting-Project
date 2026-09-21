#pragma once

#include "..\\..\\include\\Device\DEV_EJL_Define.h"

/************************************************************
* EJL
************************************************************/
class CDEV_EJL
{
public:
	// 생성자
	CDEV_EJL();
	// 소멸자
	virtual ~CDEV_EJL();

	// 열기
	BOOL Open(HWND hWnd);
	// 닫기
	BOOL Close();

	// 버전 취득
	CString GetVersion(void);

	// 마지막 에러 취득
	CString GetLastError(void);

	// 상세 Journal Data 취득
	BOOL GetDetailJnlData(int nJnlNo, LPFLSEJCOMMINFO lpCommInfo, LPFLSEJINFO lpJnlInfo);

	// 검색 Journal Data 취득
	BOOL GetSearchJnlDataList(int nType, LPCTSTR pszFrom, LPCTSTR pszTo, LPFLSEJCOMMINFO lpCommInfo, int nCommCount, int& nJnlCount);

	// 프린트 Journal 번호 취득
	BOOL GetPrintJnlNoList(LPINT pJnlNoList, int nListCount, int& nJnlCount);

	// 프린트 Journal Data 취득
	BOOL GetPrintJnlDataList(LPFLSEJCOMMINFO lpCommInfo, int nCommCount, int& nJnlCount);

	// 프린트 다음 Journal Data 취득
	BOOL GetPrintNextJnlDataList(int nJnlNo, LPFLSEJCOMMINFO lpCommInfo, int nCommCount, int& nJnlCount);

	// 프린트 Journal Data 설정
	BOOL SetPrintJnlData(int nJnlNo);

	// Journal Data 추가
	BOOL InsertJnlData(LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt);

	// Delete
	BOOL Delete(void);

	// Get Errorcode Info
	BOOL GetErrorCodeInfo(LPCTSTR pszErrcode, LPERRCODEINFO lpErrInfo);

	// 현재 저널 개수 구하기
	int GetLastJournalNo();

	// Supervisor에서 Print한 저널 No 구하기
	int GetPrintedJnlNo();

	// Supervisor에서 Print한 Jnl No Set
	BOOL SetPrintedJnlNo(int nUploadedJnlNo);

	// AMS로 Upload한 저널 No 구하기
	int GetUploadedAMSJnlNo();

	// AMS로 Upload한 Jnl No Set
	BOOL SetUploadedAMSJnlNo(int nUploadedJnlNo);

	// HOST로 Upload한 저널 No 구하기
	int GetUploadedHOSTJnlNo();

	// HOST로 Upload한 Jnl No Set
	BOOL SetUploadedHOSTJnlNo(int nUploadedJnlNo);


protected:
	typedef BOOL (WINAPI *EJL_START)(LPSTR pszErrCd);
	typedef BOOL (WINAPI *EJL_CLOSE)(LPSTR pszErrCd);
	typedef BOOL (WINAPI *EJL_INSERTJNL)(LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt, LPSTR pszErrCd);
	typedef BOOL (WINAPI *EJL_GETCURRJNL)(int nJnlNo, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt, LPSTR pszErrCd);
	typedef BOOL (WINAPI *EJL_GETPREVJNL)(int nJnlNo, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt, LPSTR pszErrCd);
	typedef BOOL (WINAPI *EJL_GETSEARCHCURRJNL)(BYTE bySrchType, LPSTR pSrchFrom, LPSTR pSrchTo, int nJnlNo, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt, LPSTR pszErrCd);
	typedef BOOL (WINAPI *EJL_GETSEARCHPREVJNL)(BYTE bySrchType, LPSTR pSrchFrom, LPSTR pSrchTo, int nJnlNo, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt, LPSTR pszErrCd);
	typedef BOOL (WINAPI *EJL_GETLASTJNLNO)(LPINT pJnlNo, LPSTR pszErrCd);
	typedef BOOL (WINAPI *EJL_GETSEARCHLASTJNLNO)(BYTE bySrchType, LPSTR pSrchTo, LPINT pJnlNo, LPSTR pszErrCd);
	typedef void (WINAPI *EJL_GETVERSION)(LPFLSEJVERSION lpEJDLLVer);
	typedef BOOL (WINAPI *EJL_GETERRINFO)(LPSTR strCode, LPERRCODEINFO lpErrInfo, LPSTR pszErrCd);

	typedef BOOL (WINAPI *EJL_DELETE)(LPSTR pszErrCd);

	typedef BOOL (WINAPI *EJL_GETAUDITJNLNO)(LPINT pJnlNo, LPSTR pszErrCd);
	typedef BOOL (WINAPI *EJL_SETAUDITJNLNO)(int nJnlNo, LPSTR pszErrCd);
	typedef BOOL (WINAPI *EJL_GETAUDITAMSJNLNO)(LPINT pJnlNo, LPSTR pszErrCd);
	typedef BOOL (WINAPI *EJL_SETAUDITAMSJNLNO)(int nJnlNo, LPSTR pszErrCd);
	typedef BOOL (WINAPI *EJL_GETAUDITHOSTJNLNO)(LPINT pJnlNo, LPSTR pszErrCd);
	typedef BOOL (WINAPI *EJL_SETAUDITHOSTJNLNO)(int nJnlNo, LPSTR pszErrCd);


	HMODULE						m_hDll;
	EJL_START					EJL_Start;
	EJL_CLOSE					EJL_Close;
	EJL_INSERTJNL				EJL_InsertJnl;
	EJL_GETCURRJNL				EJL_GetCurrJnl;
	EJL_GETPREVJNL				EJL_GetPrevJnl;
	EJL_GETSEARCHCURRJNL		EJL_GetSearchCurrJnl;
	EJL_GETSEARCHPREVJNL		EJL_GetSearchPrevJnl;
	EJL_GETLASTJNLNO			EJL_GetLastJnlNo;
	EJL_GETSEARCHLASTJNLNO		EJL_GetSearchLastJnlNo;
	EJL_GETVERSION				EJL_GetVersion;
	EJL_GETERRINFO				EJL_GetErrInfo;

	EJL_GETAUDITJNLNO			EJL_GetAuditJnlNo;
	EJL_SETAUDITJNLNO			EJL_SetAuditJnlNo;
	EJL_GETAUDITAMSJNLNO		EJL_GetAuditAMSJnlNo;
	EJL_SETAUDITAMSJNLNO		EJL_SetAuditAMSJnlNo;
	EJL_GETAUDITHOSTJNLNO		EJL_GetAuditHOSTJnlNo;
	EJL_SETAUDITHOSTJNLNO		EJL_SetAuditHOSTJnlNo;

	EJL_DELETE					EJL_Delete;

	CHAR						m_szErrorCode[6];

	BOOL						m_bOpened;
};
