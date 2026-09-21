#include "stdafx.h"
#include "ERRDBCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DB_TABLE_ERR_INFO			("TBL_ERR_INFO")

/************************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CERRDBCtrl::CERRDBCtrl()
{
}


/************************************************************
*	@brief		소멸자
*	@retval		없음
************************************************************/
CERRDBCtrl::~CERRDBCtrl()
{
}


/************************************************************
*	@brief		Open
*	@retval		없음
************************************************************/
BOOL CERRDBCtrl::Open(LPCTSTR pszDbFile)
{
	CString strtemp(pszDbFile);

	int rc = _sqlite3_open(pszDbFile, &m_db);

	if ( rc != SQLITE_OK )
	{
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Close
*	@retval		없음
************************************************************/
BOOL CERRDBCtrl::Close()
{
	return (SQLITE_OK == _sqlite3_close(m_db));
}


/************************************************************
*	@brief		Get Errorcode Info
*	@retval		없음
************************************************************/
BOOL CERRDBCtrl::GetErrorInfo(LPSTR strCode, LPERRCODEINFO lpErrInfo)
{
	sprintf(m_szSql, ("SELECT * FROM %s WHERE ErrorCode = '%s'" ), DB_TABLE_ERR_INFO, strCode);

	CString strQuery(m_szSql);

	sqlite3_stmt* stmt;

	int rc = _sqlite3_prepare(m_db, strQuery, -1, &stmt, 0 );

	if ( stmt == NULL ) {
		return FALSE;
	}
	else {

		CString szError;
		int rc = _sqlite3_step(stmt);

		if ( rc != SQLITE_ROW )
		{
			_sqlite3_finalize(stmt);
			return FALSE;
		}

		CT2A szCode((LPCTSTR)_sqlite3_column_text(stmt, 0));
		strcpy(lpErrInfo->err_code, szCode);

		CT2A szDesc((LPCTSTR)_sqlite3_column_text(stmt, 1));
		strcpy(lpErrInfo->err_desc, szDesc);

		CT2A szRecovery((LPCTSTR)_sqlite3_column_text(stmt, 2));
		strcpy(lpErrInfo->err_recovery, szRecovery);
	}

	_sqlite3_finalize(stmt);

	return TRUE;
}


/************************************************************
*	@brief		Execute Insert SQL
*	@retval		없음
************************************************************/
BOOL CERRDBCtrl::ExecuteSql(LPCSTR pszSQL)
{
	CString strQuery(pszSQL);

	sqlite3_stmt* stmt;

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, 0))
	{
		return FALSE;
	}

	if (SQLITE_DONE != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);
		return FALSE;
	}

	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Count
*	@retval		없음
************************************************************/
int CERRDBCtrl::GetCount(LPCSTR pszSQL)
{
	int nCount = 0;

	CString strQuery(pszSQL);

	sqlite3_stmt* stmt;

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, 0))
	{
		return -1;
	}

	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);
		return -1;
	}

	nCount = _sqlite3_column_int(stmt, 0);

	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return -1;
	}

	return nCount;
}


//----------------------------------------------------------
//	Function : After back-up the full journal,
//				Clear Journal DB & Set journal count to 1
//----------------------------------------------------------
BOOL CERRDBCtrl::DeleteDB(LPCSTR pszDbFile)
{
	return TRUE;
}


/************************************************************
*	@brief		Make Error Code
*	@retval		없음
************************************************************/
void CERRDBCtrl::MakeErrCode(BYTE err_id)
{
	sprintf(m_szErrCd, "J0%04d", err_id);
}


/************************************************************
*	@brief		Get Error Code
*	@retval		없음
************************************************************/
LPCSTR CERRDBCtrl::GetErrCode(void)
{
	return m_szErrCd;
}
