// EJLDBCtrl.h: interface for the EJLDBCtrl class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

//#include "DEV_EJL_Define.h"	구조체 통신하는 부분에 대해서는 소스를 하나로 하기 위해 위치 변경
#include "..\\..\\..\\Include\Device\DEV_EJL_Define.h"

#include "sqlite3i.h"
#include "sqlite3.h"

#define EJ_MAX_COUNT	999999

class EJLDBCtrl  
{
public:
	// 생성자
	EJLDBCtrl();
	// 소멸자
	virtual ~EJLDBCtrl();

	// Open
	BOOL Open(LPCTSTR pszDbFile);
	// Close
	BOOL Close();
	// Get Short Journal Code
	//BYTE GetShrtJnlCd(BYTE jnl_cd);
	// Insert Record
	BOOL InsertRecord(LPFLSEJCOMMINFO lpCommInfo, DWORD nCnt, LPVOID lpJnlDt);
	// Read First Record
	BOOL ReadFirstRecord(int nIndex, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt);
	// Read Last Record
	BOOL ReadLastRecord(int nIndex, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt);
	// Read Record
	BOOL ReadCurrentRecord(int nJnlNo, int nIndex, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt);
	// Search Record
	BOOL SearchCurrentRecord(BYTE bySrchType, LPSTR pSrchFrom, LPSTR pSrchTo, int nJnlNo, int nIndex, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt);
	// Read Record
	BOOL ReadRecord(LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt);
	// Get Previous Record
	BOOL GetPrevRec(LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt);
	// Get Next Record
	BOOL GetNextRec(LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt);
	// Get Record Count
	BOOL GetRecordCount(LPDWORD lpRecCnt);
	// Get Previous Record Count
	BOOL GetPrevRecordCount(int nJnlNo, LPDWORD lpRecCnt);
	// Get Next Record Count
	BOOL GetNextRecordCount(int nJnlNo, LPDWORD lpRecCnt);
	// Get Print Record Count
	BOOL GetPrintRecordCount(LPDWORD lpRecCnt, int nAuditJnlNo);
	// Get Print Previous Record Count
	BOOL GetPrintPrevRecordCount(int nJnlNo, LPDWORD lpRecCnt, int nAuditJnlNo);
	// Get Print Next Record Count
	BOOL GetPrintNextRecordCount(int nJnlNo, LPDWORD lpRecCnt, int nAuditJnlNo);
	// Get Search Record Count
	BOOL GetSearchRecordCount(BYTE bySrchType, LPSTR pSrchFrom, LPSTR pSrchTo, LPDWORD lpRecCnt);
	// Get Search Previous Record Count
	BOOL GetSearchPrevRecordCount(BYTE bySrchType, LPSTR pSrchFrom, LPSTR pSrchTo, int nJnlNo, LPDWORD lpRecCnt);
	// Get Search Next Record Count
	BOOL GetSearchNextRecordCount(BYTE bySrchType, LPSTR pSrchFrom, LPSTR pSrchTo, int nJnlNo, LPDWORD lpRecCnt);
	// Get First Journal Number
	BOOL GetFirstJnlNo(LPINT pJnlNo);
	// Get Last Journal Number
	BOOL GetLastJnlNo(LPINT pJnlNo);
	// Get Search First Journal Number
	BOOL GetSearchFirstJnlNo(BYTE bySrchType, LPSTR pSrchFrom, LPINT pJnlNo);
	// Get Search Last Journal Number
	BOOL GetSearchLastJnlNo(BYTE bySrchType, LPSTR pSrchTo, LPINT pJnlNo);
	// Delete DB
	BOOL DeleteDB(LPCSTR pszDbFile);
	// Make Error Code
	void MakeErrCode(BYTE err_id);
	// Get Error Code
	LPCSTR GetErrCode(void);

	BOOL InsertCommField(LPFLSEJCOMMINFO lpCommInfo);
	BOOL InsertChangeValueField(LPFLSEJCOMMINFO lpCommInfo, LPEJCHANGEINFO lpJnlData);
//	BOOL InsertKeyField(LPFLSEJCOMMINFO lpCommInfo, LPEJCHANGEINFO lpJnlData);
	BOOL InsertAddBillField(LPFLSEJCOMMINFO lpCommInfo, LPFLSEJADDBILLINFO lpJnlData);
	BOOL InsertCstCloseField(LPFLSEJCOMMINFO lpCommInfo, LPFLSEJCSTCLINFO lpJnlData);
	BOOL InsertDayCloseField(LPFLSEJCOMMINFO lpCommInfo, LPFLSEJDAYCLINFO lpJnlData);
	BOOL InsertTransField(LPFLSEJCOMMINFO lpCommInfo, LPFLSEJTRANINFO lpJnlData);
	BOOL InsertTextField(LPFLSEJCOMMINFO lpCommInfo, LPEJTEXTINFO lpJnlData);
	BOOL InsertDenomField(LPFLSEJCOMMINFO lpCommInfo, LPFLSEJSETDENOMINFO lpJnlData);
	BOOL InsertTestDispenseField(LPFLSEJCOMMINFO lpCommInfo, LPFLSEJTESTDISPENSE lpJnlData);	// V1.0.0.2 2021.02.23 LEH - TestDispense 추가

	BOOL ExecuteSql(LPCSTR pszSQL);

	BOOL GetCommField(LPFLSEJCOMMINFO lpCommInfo, sqlite3_stmt* stmt);
	BOOL GetPwdField(LPEJCHANGEINFO lpJnlData, sqlite3_stmt* stmt);
	BOOL GetKeyField(LPEJCHANGEINFO lpJnlData, sqlite3_stmt* stmt);
	BOOL GetAddBillField(LPFLSEJADDBILLINFO lpJnlData, sqlite3_stmt* stmt);
	BOOL GetCstCloseField(LPFLSEJCSTCLINFO lpJnlData, sqlite3_stmt* stmt);
	BOOL GetDayCloseField(LPFLSEJDAYCLINFO lpJnlData, sqlite3_stmt* stmt);
	BOOL GetTransField(LPFLSEJTRANINFO lpJnlData, sqlite3_stmt* stmt);
	BOOL GetTextField(LPEJTEXTINFO lpJnlData, sqlite3_stmt* stmt);
	BOOL GetDenomField(LPFLSEJSETDENOMINFO lpJnlData, sqlite3_stmt* stmt);
	BOOL GetTestDispenseField(LPFLSEJTESTDISPENSE lpJnlData, sqlite3_stmt* stmt);

	INT	GetLastRecordIndex(void);
	void GetDBSize(void);

	// Clear Information
	void ClearInfo(void);

	BOOL InsertMgrRecord(LPFLSDATAINFO lpDataInfo);
	BOOL UpdateMgrRecord(DWORD dwPropBit, LPFLSDATAINFO lpDataInfo);
	BOOL ReadMgrRecord(LPFLSDATAINFO lpDataInfo);

	sqlite3* m_db;
	CHAR m_szErrCd[7];
	CHAR m_szSql[4096];
	
	CHAR m_szTemp[4096];

};
