// ERRDBCtrl.h: interface for the CERRDBCtrl class.
// Function  : MFS Error code Database handle
// History	 : 1. New - 2017.11.17
//////////////////////////////////////////////////////////////////////

#pragma once

//#include "DEV_EJL_Define.h"
#include "..\\..\\..\\Include\Device\DEV_EJL_Define.h"

#include "DbSqlite.h"

class CERRDBCtrl  
{
public:
	CERRDBCtrl();
	virtual ~CERRDBCtrl();

	BOOL Open(LPCTSTR pszDbFile);
	
	BOOL Close();
	
	BOOL GetErrorInfo(LPSTR strCode, LPERRCODEINFO lpErrInfo);
	
	BOOL DeleteDB(LPCSTR pszDbFile);
	
	void MakeErrCode(BYTE err_id);
	
	LPCSTR GetErrCode(void);

	BOOL ExecuteSql(LPCSTR pszSQL);
	int GetCount(LPCSTR pszSQL);

	sqlite3* m_db;
	CHAR m_szSql[4096];
	WORD m_wFldCnt;

	CHAR m_szErrCd[7];

	CDbSQLite m_sqlite;
};
