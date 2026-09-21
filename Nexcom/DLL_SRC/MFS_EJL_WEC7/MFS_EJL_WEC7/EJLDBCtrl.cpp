#include "stdafx.h"
#include "EJLDBCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DB_TABLE_JNL_INFO			("TBL_JNL_INFO")

#define DB_TABLE_JNL_MGR			("TBL_JNL_MGR")

#define DB_TABLE_CHANGE_INFO		("TBL_CHANGE_INFO")

//#define DB_TABLE_KEY_INFO			("TBL_KEY_INFO")

#define DB_TABLE_ADD_BILL_INFO		("TBL_ADD_BILL_INFO")

#define DB_TABLE_CST_CLOSE_INFO		("TBL_CST_CLOSE_INFO")

#define DB_TABLE_DAY_CLOSE_INFO		("TBL_DAY_CLOSE_INFO")

#define DB_TABLE_TRANS_INFO			("TBL_TRANS_INFO")

#define DB_TABLE_TEXT_INFO			("TBL_TEXT_INFO")

#define DB_TABLE_DENOM_INFO			("TBL_DENOM_INFO")


#define DB_TABLE_TEST_DISPENSE_INFO				("TBL_TEST_DISPENSE_INFO")	// V1.0.0.2 2021.03.02 LEH - TestDispense 추가


/************************************************************
*	@brief		생성자
*	@retval		Nothing
************************************************************/
EJLDBCtrl::EJLDBCtrl()
{
	m_db = NULL;
	memset(m_szErrCd, 0x00, sizeof(m_szErrCd));
	strcpy(m_szErrCd, "J00000");
	memset(m_szSql, 0, sizeof(m_szSql));
}


/************************************************************
*	@brief		소멸자
*	@retval		Nothing
************************************************************/
EJLDBCtrl::~EJLDBCtrl()
{
}


/************************************************************
*	@brief		Get Short Journal Code
*	@retval		Nothing - not used
************************************************************/
//BYTE EJLDBCtrl::GetShrtJnlCd(BYTE jnl_cd)
//{
//	return jnl_cd;
//}


/************************************************************
*	@brief		Open
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::Open(LPCTSTR pszDbFile)
{
	CString strQuery;

	bool bRet = false;

	if (SQLITE_OK != _sqlite3_open(pszDbFile, &m_db))
	{
		return FALSE;
	}

	// JNL_MGR Table Create
	// Create table : Journal Info
/*	sprintf(m_szSql, "CREATE TABLE %s ("		\
			"TOTAL_JNL_CNT			INTEGER PRIMARY KEY,"		\
			"PRINTED_JNL_NO			INTEGER,"					\
			"UPLOADED_AMS_JNL_NO	INTEGER,"					\
			"UPLOADED_HOST_JNL_NO	INTEGER);", 
			DB_TABLE_JNL_MGR);
	
	ExecuteSql(m_szSql);

//	sprintf(m_szSql, ("INSERT INTO %s VALUES (0, 0, 0, 0, 0)"), DB_TABLE_JNL_MGR);

//	ExecuteSql(m_szSql);


	// Create table : Journal Info
	sprintf(m_szSql, "CREATE TABLE %s ("		\
		"JNL_NO				INTEGER PRIMARY KEY,"			\
		"LOG_DATE			TEXT,"							\
		"LOG_TIME			TEXT,"							\
		"JNL_CD				INTEGER,"						\
		"LOG_SRCH_DATE		TEXT);", 
		DB_TABLE_JNL_INFO);

	ExecuteSql(m_szSql);
*/
	// Create table : Change Info
	sprintf(m_szSql, "CREATE TABLE %s ("					\
		"JNL_NO				INTEGER PRIMARY KEY,"			\
		"TERM_ID			TEXT	DEFAULT		'',"		\
		"OLD_VALUE			TEXT	DEFAULT		'',"		\
		"NEW_VALUE			TEXT	DEFAULT		'',"		\
		"IS_PRINT			TEXT	DEFAULT		'',"		\
		"IS_AMS_UPLOAD		TEXT	DEFAULT		'',"		\
		"IS_HOST_UPLOAD		TEXT	DEFAULT		'');", 
		DB_TABLE_CHANGE_INFO);

	ExecuteSql(m_szSql);

	// Create table : Add Bill Info
	sprintf(m_szSql, "CREATE TABLE %s ("					\
		"JNL_NO				INTEGER PRIMARY KEY,"			\
		"TERM_ID			TEXT	DEFAULT		'',"		\
		"START_DATE			TEXT	DEFAULT		'',"		\
		"START_TIME			TEXT	DEFAULT		'',"		\
		"ADD_BILL_CST1		TEXT	DEFAULT		'',"		\
		"REMAIN_CST1		TEXT	DEFAULT		'',"		\
		"ADD_BILL_CST2		TEXT	DEFAULT		'',"		\
		"REMAIN_CST2		TEXT	DEFAULT		'',"		\
		"ADD_BILL_CST3		TEXT	DEFAULT		'',"		\
		"REMAIN_CST3		TEXT	DEFAULT		'',"		\
		"ADD_BILL_CST4		TEXT	DEFAULT		'',"		\
		"REMAIN_CST4		TEXT	DEFAULT		'',"		\
		"IS_PRINT			TEXT	DEFAULT		'',"		\
		"IS_AMS_UPLOAD		TEXT	DEFAULT		'',"		\
		"IS_HOST_UPLOAD		TEXT	DEFAULT		'');", 
		DB_TABLE_ADD_BILL_INFO);

	ExecuteSql(m_szSql);

	// Create table : Cassette Close
	sprintf(m_szSql, "CREATE TABLE %s ("					\
		"JNL_NO				INTEGER PRIMARY KEY,"			\
		"TERM_ID			TEXT	DEFAULT		'',"		\
		"START_DATE			TEXT	DEFAULT		'',"		\
		"START_TIME			TEXT	DEFAULT		'',"		\
		"CST_CLOSE_TYPE		TEXT	DEFAULT		'',"		\
		"LOAD_BILL_CST1		TEXT	DEFAULT		'',"		\
		"DISP_BILL_CST1		TEXT	DEFAULT		'',"		\
		"REJ_BILL_CST1		TEXT	DEFAULT		'',"		\
		"REM_BILL_CST1		TEXT	DEFAULT		'',"		\
		"LOAD_AMT_CST1		TEXT	DEFAULT		'',"		\
		"DISP_AMT_CST1		TEXT	DEFAULT		'',"		\
		"REM_AMT_CST1		TEXT	DEFAULT		'',"		\
		"DENOM_CST1			TEXT	DEFAULT		'',"		\
		"LOAD_BILL_CST2		TEXT	DEFAULT		'',"		\
		"DISP_BILL_CST2		TEXT	DEFAULT		'',"		\
		"REJ_BILL_CST2		TEXT	DEFAULT		'',"		\
		"REM_BILL_CST2		TEXT	DEFAULT		'',"		\
		"LOAD_AMT_CST2		TEXT	DEFAULT		'',"		\
		"DISP_AMT_CST2		TEXT	DEFAULT		'',"		\
		"REM_AMT_CST2		TEXT	DEFAULT		'',"		\
		"DENOM_CST2			TEXT	DEFAULT		'',"		\
		"LOAD_BILL_CST3		TEXT	DEFAULT		'',"		\
		"DISP_BILL_CST3		TEXT	DEFAULT		'',"		\
		"REJ_BILL_CST3		TEXT	DEFAULT		'',"		\
		"REM_BILL_CST3		TEXT	DEFAULT		'',"		\
		"LOAD_AMT_CST3		TEXT	DEFAULT		'',"		\
		"DISP_AMT_CST3		TEXT	DEFAULT		'',"		\
		"REM_AMT_CST3		TEXT	DEFAULT		'',"		\
		"DENOM_CST3			TEXT	DEFAULT		'',"		\
		"LOAD_BILL_CST4		TEXT	DEFAULT		'',"		\
		"DISP_BILL_CST4		TEXT	DEFAULT		'',"		\
		"REJ_BILL_CST4		TEXT	DEFAULT		'',"		\
		"REM_BILL_CST4		TEXT	DEFAULT		'',"		\
		"LOAD_AMT_CST4		TEXT	DEFAULT		'',"		\
		"DISP_AMT_CST4		TEXT	DEFAULT		'',"		\
		"REM_AMT_CST4		TEXT	DEFAULT		'',"		\
		"DENOM_CST4			TEXT	DEFAULT		'',"		\
		"IS_PRINT			TEXT	DEFAULT		'',"		\
		"IS_AMS_UPLOAD		TEXT	DEFAULT		'',"		\
		"IS_HOST_UPLOAD		TEXT	DEFAULT		'');", 
		DB_TABLE_CST_CLOSE_INFO);

	ExecuteSql(m_szSql);

	// Create table : Day Close
	sprintf(m_szSql, "CREATE TABLE %s ("					\
		"JNL_NO				INTEGER PRIMARY KEY,"			\
		"TERM_ID			TEXT	DEFAULT		'',"		\
		"START_DATE			TEXT	DEFAULT		'',"		\
		"START_TIME			TEXT	DEFAULT		'',"		\
		"DAY_CLOSE_TYPE		TEXT	DEFAULT		'',"		\
		"DAY_CLOSE_RESULT	TEXT	DEFAULT		'',"		\
		"HOST_WITH_CNT		TEXT	DEFAULT		'',"		\
		"HOST_BAL_CNT		TEXT	DEFAULT		'',"		\
		"HOST_TRANS_CNT		TEXT	DEFAULT		'',"		\
		"TOTAL_HOST_AMT		TEXT	DEFAULT		'',"		\
		"TERM_WITH_CNT		TEXT	DEFAULT		'',"		\
		"TERM_BAL_CNT		TEXT	DEFAULT		'',"		\
		"TERM_TRANS_CNT		TEXT	DEFAULT		'',"		\
		"TOTAL_TERM_AMT		TEXT	DEFAULT		'',"		\
		"IS_PRINT			TEXT	DEFAULT		'',"		\
		"IS_AMS_UPLOAD		TEXT	DEFAULT		'',"		\
		"IS_HOST_UPLOAD		TEXT	DEFAULT		'');", 
		DB_TABLE_DAY_CLOSE_INFO);

	ExecuteSql(m_szSql);

	// Create table : Denom Info
	sprintf(m_szSql, "CREATE TABLE %s ("					\
		"JNL_NO				INTEGER PRIMARY KEY,"			\
		"TERM_ID			TEXT	DEFAULT		'',"		\
		"START_DATE			TEXT	DEFAULT		'',"		\
		"START_TIME			TEXT	DEFAULT		'',"		\
		"OLDDENOM_CST1		TEXT	DEFAULT		'',"		\
		"OLDDENOM_CST2		TEXT	DEFAULT		'',"		\
		"OLDDENOM_CST3		TEXT	DEFAULT		'',"		\
		"OLDDENOM_CST4		TEXT	DEFAULT		'',"		\
		"NEWDENOM_CST1		TEXT	DEFAULT		'',"		\
		"NEWDENOM_CST2		TEXT	DEFAULT		'',"		\
		"NEWDENOM_CST3		TEXT	DEFAULT		'',"		\
		"NEWDENOM_CST4		TEXT	DEFAULT		'',"		\
		"IS_PRINT			TEXT	DEFAULT		'',"		\
		"IS_AMS_UPLOAD		TEXT	DEFAULT		'',"		\
		"IS_HOST_UPLOAD		TEXT	DEFAULT		'')", 
		DB_TABLE_DENOM_INFO);

	ExecuteSql(m_szSql);

	// Create table : Transaction Info
	sprintf(m_szSql, "CREATE TABLE %s ("					\
		"JNL_NO				INTEGER PRIMARY KEY,"			\
		"TERM_ID			TEXT	DEFAULT		'',"		\
		"TRAN_SEQ_NO		TEXT	DEFAULT		'',"		\
		"TRAN_CD			TEXT	DEFAULT		'',"		\
		"CARD_TR2			TEXT	DEFAULT		'',"		\
		"AUTH_NO			TEXT	DEFAULT		'',"		\
		"TRAN_DATE			TEXT	DEFAULT		'',"		\
		"TRAN_TIME			TEXT	DEFAULT		'',"		\
		"BUSINESS_DATE		TEXT	DEFAULT		'',"		\
		"ACCOUNT_TYPE		TEXT	DEFAULT		'',"		\
		"REQ_AMT			TEXT	DEFAULT		'',"		\
		"DISP_AMT			TEXT	DEFAULT		'',"		\
		"SURC_AMT			TEXT	DEFAULT		'',"		\
		"BAL_AMT			TEXT	DEFAULT		'',"		\
		"AVAIL_BAL_AMT		TEXT	DEFAULT		'',"		\
		"DISPENSED_CNT		TEXT	DEFAULT		'',"		\
		"REJECTED_CNT		TEXT	DEFAULT		'',"		\
		"ERR_CODE_1			TEXT	DEFAULT		'',"		\
		"ERR_CODE_2			TEXT	DEFAULT		'',"		\
		"ERR_CODE_3			TEXT	DEFAULT		'',"		\
		"ERR_MSG			TEXT	DEFAULT		'',"		\
		"IS_PRINT			TEXT	DEFAULT		'',"		\
		"IS_AMS_UPLOAD		TEXT	DEFAULT		'',"		\
		"IS_HOST_UPLOAD		TEXT	DEFAULT		'');",
		DB_TABLE_TRANS_INFO);

	ExecuteSql(m_szSql);

	// Create table : Text Info
	sprintf(m_szSql, "CREATE TABLE %s ("					\
		"JNL_NO				INTEGER PRIMARY KEY,"			\
		"TERM_ID			TEXT	DEFAULT		'',"		\
		"TEXT_VALUE			TEXT	DEFAULT		'',"		\
		"IS_PRINT			TEXT	DEFAULT		'',"		\
		"IS_AMS_UPLOAD		TEXT	DEFAULT		'',"		\
		"IS_HOST_UPLOAD		TEXT	DEFAULT		'');", 
		DB_TABLE_TEXT_INFO);

	ExecuteSql(m_szSql);

	// V1.0.0.2 2021.02.23 LEH - Test Dispense 추가
	// Create table : Test Dispense Info
	sprintf(m_szSql, "CREATE TABLE %s ("					\
		"JNL_NO					INTEGER PRIMARY KEY,"		\
		"TERM_ID				TEXT	DEFAULT		'',"	\
		"START_DATE				TEXT	DEFAULT		'',"	\
		"START_TIME				TEXT	DEFAULT		'',"	\
		"TEST_DISP_RESULT		TEXT	DEFAULT		'',"	\
		"TEST_REQ_BILL_CST1		TEXT	DEFAULT		'',"	\
		"TEST_PICKUP_BILL_CST1	TEXT	DEFAULT		'',"	\
		"TEST_REJ_BILL_CST1		TEXT	DEFAULT		'',"	\
		"REM_BILL_CST1			TEXT	DEFAULT		'',"	\
		"DENOM_CST1				TEXT	DEFAULT		'',"	\
		"TEST_REQ_BILL_CST2		TEXT	DEFAULT		'',"	\
		"TEST_PICKUP_BILL_CST2	TEXT	DEFAULT		'',"	\
		"TEST_REJ_BILL_CST2		TEXT	DEFAULT		'',"	\
		"REM_BILL_CST2			TEXT	DEFAULT		'',"	\
		"DENOM_CST2				TEXT	DEFAULT		'',"	\
		"TEST_REQ_BILL_CST3		TEXT	DEFAULT		'',"	\
		"TEST_PICKUP_BILL_CST3	TEXT	DEFAULT		'',"	\
		"TEST_REJ_BILL_CST3		TEXT	DEFAULT		'',"	\
		"REM_BILL_CST3			TEXT	DEFAULT		'',"	\
		"DENOM_CST3				TEXT	DEFAULT		'',"	\
		"TEST_REQ_BILL_CST4		TEXT	DEFAULT		'',"	\
		"TEST_PICKUP_BILL_CST4	TEXT	DEFAULT		'',"	\
		"TEST_REJ_BILL_CST4		TEXT	DEFAULT		'',"	\
		"REM_BILL_CST4			TEXT	DEFAULT		'',"	\
		"DENOM_CST4				TEXT	DEFAULT		'',"	\
		"IS_PRINT				TEXT	DEFAULT		'',"	\
		"IS_AMS_UPLOAD			TEXT	DEFAULT		'',"	\
		"IS_HOST_UPLOAD			TEXT	DEFAULT		'')", 
		DB_TABLE_TEST_DISPENSE_INFO);

	ExecuteSql(m_szSql);
	//////////////////////////////////////////////

	// 신규 추가된 Table에 대해서 Column을 추가하는 로직 추가 (이전 버전 호환성 처리 - 없을 경우 비정상 종료됨)
	// Added Field for CST1 Reject Info
	memset(m_szSql, 0, sizeof(m_szSql));
	sprintf(m_szSql, "ALTER TABLE %s ADD "		\
		"CST1_REJECT_INFO		TEXT	DEFAULT		''", 
		DB_TABLE_TRANS_INFO);
	ExecuteSql(m_szSql);

	// Added Field for CST2 Reject Info
	memset(m_szSql, 0, sizeof(m_szSql));
	sprintf(m_szSql, "ALTER TABLE %s ADD "		\
		"CST2_REJECT_INFO		TEXT	DEFAULT		''", 
		DB_TABLE_TRANS_INFO);
	ExecuteSql(m_szSql);

	// Added Field for CST3 Reject Info
	memset(m_szSql, 0, sizeof(m_szSql));
	sprintf(m_szSql, "ALTER TABLE %s ADD "		\
		"CST3_REJECT_INFO		TEXT	DEFAULT		''", 
		DB_TABLE_TRANS_INFO);
	ExecuteSql(m_szSql);

	// Added Field for CST4 Reject Info
	memset(m_szSql, 0, sizeof(m_szSql));
	sprintf(m_szSql, "ALTER TABLE %s ADD "		\
		"CST4_REJECT_INFO		TEXT	DEFAULT		''", 
		DB_TABLE_TRANS_INFO);
	ExecuteSql(m_szSql);

	// Added Field for CST Remain Info
	memset(m_szSql, 0, sizeof(m_szSql));
	sprintf(m_szSql, "ALTER TABLE %s ADD "		\
		"CST_REMAIN_CNT		TEXT	DEFAULT		''", 
		DB_TABLE_TRANS_INFO);
	ExecuteSql(m_szSql);

	// Added Field for EMV Info
	memset(m_szSql, 0, sizeof(m_szSql));
	sprintf(m_szSql, "ALTER TABLE %s ADD "		\
		"EMVDATA		TEXT	DEFAULT		''", 
		DB_TABLE_TRANS_INFO);
	ExecuteSql(m_szSql);

	// Added Field for DCC Info
	memset(m_szSql, 0, sizeof(m_szSql));
	sprintf(m_szSql, "ALTER TABLE %s ADD "		\
		"DCCDATA		TEXT	DEFAULT		''", 
		DB_TABLE_TRANS_INFO);
	ExecuteSql(m_szSql);

	// Added Field for CAM Image Info
	memset(m_szSql, 0, sizeof(m_szSql));
	sprintf(m_szSql, "ALTER TABLE %s ADD "		\
		"CAM_IMG_INFO		TEXT	DEFAULT		''", 
		DB_TABLE_TRANS_INFO);
	ExecuteSql(m_szSql);

	// Add Cash / CBX Close / Set Denomination / Test Dispense 에 대해서는 CBX Count Field 추가
	// 1. Add Cash
	memset(m_szSql, 0, sizeof(m_szSql));
	sprintf(m_szSql, "ALTER TABLE %s ADD "		\
		"CST_COUNT		TEXT	DEFAULT		''", 
		DB_TABLE_ADD_BILL_INFO);
	ExecuteSql(m_szSql);

	// 2. CBX Close
	memset(m_szSql, 0, sizeof(m_szSql));
	sprintf(m_szSql, "ALTER TABLE %s ADD "		\
		"CST_COUNT		TEXT	DEFAULT		''", 
		DB_TABLE_CST_CLOSE_INFO);
	ExecuteSql(m_szSql);

	// 3. Set Denomination
	memset(m_szSql, 0, sizeof(m_szSql));
	sprintf(m_szSql, "ALTER TABLE %s ADD "		\
		"CST_COUNT		TEXT	DEFAULT		''", 
		DB_TABLE_DENOM_INFO);
	ExecuteSql(m_szSql);
	
	// 4. Test Dispense
	memset(m_szSql, 0, sizeof(m_szSql));
	sprintf(m_szSql, "ALTER TABLE %s ADD "		\
		"CST_COUNT		TEXT	DEFAULT		''", 
		DB_TABLE_TEST_DISPENSE_INFO);
	ExecuteSql(m_szSql);

	// Added Field for Error Code Info
	memset(m_szSql, 0, sizeof(m_szSql));
	sprintf(m_szSql, "ALTER TABLE %s ADD "		\
		"ERR_CODE		TEXT	DEFAULT		''", 
		DB_TABLE_TEST_DISPENSE_INFO);
	ExecuteSql(m_szSql);
	////////////////////////////////////////
	return TRUE;
}


/************************************************************
*	@brief		Close
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::Close()
{
	return (SQLITE_OK == _sqlite3_close(m_db));
}


/************************************************************
*	@brief		Insert Record
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::InsertRecord(LPFLSEJCOMMINFO lpCommInfo, DWORD nCnt, LPVOID lpJnlDt)
{
	lpCommInfo->jnl_no = nCnt;

	BYTE jnl_cd = (BYTE)lpCommInfo->jnl_cd;


	BOOL bResult = TRUE;

	bResult &= InsertCommField(lpCommInfo);


	if (jnl_cd == transaction || jnl_cd == withdrawal || jnl_cd == balance || jnl_cd == transfer)	// header 세분화
	{
		bResult &= InsertTransField(lpCommInfo, (LPFLSEJTRANINFO)lpJnlDt);
	}
	else if (jnl_cd == value_change)
	{
		bResult &= InsertChangeValueField(lpCommInfo, (LPEJCHANGEINFO)lpJnlDt);
	}
	else if (jnl_cd == add_bill)
	{
		bResult &= InsertAddBillField(lpCommInfo, (LPFLSEJADDBILLINFO)lpJnlDt);
	}
	else if (jnl_cd == cst_close)
	{
		bResult &= InsertCstCloseField(lpCommInfo, (LPFLSEJCSTCLINFO)lpJnlDt);
	}
	else if (jnl_cd == day_close)
	{
		bResult &= InsertDayCloseField(lpCommInfo, (LPFLSEJDAYCLINFO)lpJnlDt);
	}
	else if (jnl_cd == text_field || jnl_cd == power_on || jnl_cd == in_service || jnl_cd == out_of_service || jnl_cd == in_supervisor || jnl_cd == reboot)
	{
		bResult &= InsertTextField(lpCommInfo, (LPEJTEXTINFO)lpJnlDt);
	}
	else if (jnl_cd == denom_info)
	{
		bResult &= InsertDenomField(lpCommInfo, (LPFLSEJSETDENOMINFO)lpJnlDt);
	}
	// V1.0.0.2 2021.03.02 LEH - TestDispense 추가
	else if (jnl_cd == test_dispense)
	{
		bResult &= InsertTestDispenseField(lpCommInfo, (LPFLSEJTESTDISPENSE)lpJnlDt);
	}


	if (FALSE == bResult)
	{
		return FALSE;
	}


	return TRUE;
}


/************************************************************
*	@brief		Insert Common Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::InsertCommField(LPFLSEJCOMMINFO lpCommInfo)
{
	sprintf(m_szSql, ("INSERT INTO %s VALUES (%d, '%s', '%s', %d, '%s')"),
		DB_TABLE_JNL_INFO,
		lpCommInfo->jnl_no,
		lpCommInfo->log_date,
		lpCommInfo->log_time,
		lpCommInfo->jnl_cd,
		lpCommInfo->log_srch_date);

	return ExecuteSql(m_szSql);
}


/************************************************************
*	@brief		Insert InsertChangeValue Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::InsertChangeValueField(LPFLSEJCOMMINFO lpCommInfo, LPEJCHANGEINFO lpJnlData)
{
	//sprintf(m_szSql, ("INSERT INTO %s VALUES (%d, '%s', '%s', '%s', '%s', '%s', '%s')"),
	//	DB_TABLE_CHANGE_INFO,
	//	lpCommInfo->jnl_no,
	//	lpJnlData->term_id,
	//	lpJnlData->old_value,
	//	lpJnlData->new_value,
	//	lpJnlData->is_print,
	//	lpJnlData->is_ams_upload,
	//	lpJnlData->is_host_upload);

	//return ExecuteSql(m_szSql);

	#define DB_TABLE_CHANGEVALUE_FIELD_COUNT	7

	CString strDBTableChangeValueFieldName[DB_TABLE_CHANGEVALUE_FIELD_COUNT] = {
		_T("JNL_NO"), 
		_T("TERM_ID"), _T("OLD_VALUE"), _T("NEW_VALUE"), _T("IS_PRINT"), _T("IS_AMS_UPLOAD"), 
		_T("IS_HOST_UPLOAD")
	};

	CString strSQL, strTemp;
	CString strJNL[DB_TABLE_CHANGEVALUE_FIELD_COUNT] = {_T(""),};
	int i=0;
	int nIndex = 0;

	strJNL[nIndex++].Format(_T("%d"), lpCommInfo->jnl_no);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->term_id);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->old_value);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->new_value);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_print);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_ams_upload);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_host_upload);

	// ex) INSERT INTO TBL_TRANS_INFO(JNL_NO,TERM_ID) VALUES(1002, '456')
	// 위와 같은 쿼리문을 만들기 위함.
	strSQL.Format(_T("INSERT INTO %S("), DB_TABLE_CHANGE_INFO);

	for(i=0; i<DB_TABLE_CHANGEVALUE_FIELD_COUNT; i++)
	{
		strTemp.Format(_T("%s"), strDBTableChangeValueFieldName[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_CHANGEVALUE_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(") VALUES(");

	for(i=0; i<DB_TABLE_CHANGEVALUE_FIELD_COUNT; i++)
	{
		if(0 == i)
			strTemp.Format(_T("%s"), strJNL[i]);	// JNL_NO는 숫자이므로
		else
			strTemp.Format(_T("'%s'"), strJNL[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_CHANGEVALUE_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(")");
	///////////////////////////////////////////////////////////////

	sprintf(m_szSql, "%S", strSQL);

	return ExecuteSql(m_szSql);
}


/************************************************************
*	@brief		Insert Key Info
*	@retval		Nothing
************************************************************/
//BOOL EJLDBCtrl::InsertKeyField(LPFLSEJCOMMINFO lpCommInfo, LPEJCHANGEINFO lpJnlData)
//{
//	sprintf(m_szSql, ("INSERT INTO %s VALUES (%d, '%s', '%s', '%s', '%s', '%s', '%s')"),
//		DB_TABLE_CHANGE_INFO,
//		lpJnlData->jnl_no,
//		lpJnlData->term_id,
//		lpJnlData->old_value,
//		lpJnlData->new_value,
//		lpJnlData->is_print,
//		lpJnlData->is_ams_upload,
//		lpJnlData->is_host_upload);
//
//	return ExecuteSql(m_szSql);
//}


/************************************************************
*	@brief		Insert Add Bill Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::InsertAddBillField(LPFLSEJCOMMINFO lpCommInfo, LPFLSEJADDBILLINFO lpJnlData)
{
	//sprintf(m_szSql, ("INSERT INTO %s VALUES (%d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')"),
	//	DB_TABLE_ADD_BILL_INFO,
	//	lpCommInfo->jnl_no,
	//	lpJnlData->term_id,
	//	lpJnlData->start_date,
	//	lpJnlData->start_time,

	//	lpJnlData->add_bill_cst1,
	//	lpJnlData->remain_cst1,
	//	lpJnlData->add_bill_cst2,
	//	lpJnlData->remain_cst2,
	//	lpJnlData->add_bill_cst3,
	//	lpJnlData->remain_cst3,
	//	lpJnlData->add_bill_cst4,
	//	lpJnlData->remain_cst4,

	//	lpJnlData->is_print,
	//	lpJnlData->is_ams_upload,
	//	lpJnlData->is_host_upload,
	//	
	//	lpJnlData->cst_count);


	//return ExecuteSql(m_szSql);

	#define DB_TABLE_ADDBILL_FIELD_COUNT	16

	CString strDBTableAddBillFieldName[DB_TABLE_ADDBILL_FIELD_COUNT] = {
		_T("JNL_NO"), 
		_T("TERM_ID"), _T("START_DATE"), _T("START_TIME"), _T("ADD_BILL_CST1"), _T("REMAIN_CST1"), 
		_T("ADD_BILL_CST2"), _T("REMAIN_CST2"), _T("ADD_BILL_CST3"), _T("REMAIN_CST3"), _T("ADD_BILL_CST4"), 
		_T("REMAIN_CST4"), _T("IS_PRINT"), _T("IS_AMS_UPLOAD"), _T("IS_HOST_UPLOAD"), _T("CST_COUNT")
	};

	CString strSQL, strTemp;
	CString strJNL[DB_TABLE_ADDBILL_FIELD_COUNT] = {_T(""),};
	int i=0;
	int nIndex = 0;

	strJNL[nIndex++].Format(_T("%d"), lpCommInfo->jnl_no);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->term_id);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->start_date);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->start_time);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->add_bill_cst1);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->remain_cst1);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->add_bill_cst2);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->remain_cst2);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->add_bill_cst3);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->remain_cst3);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->add_bill_cst4);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->remain_cst4);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_print);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_ams_upload);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_host_upload);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->cst_count);

	// ex) INSERT INTO TBL_TRANS_INFO(JNL_NO,TERM_ID) VALUES(1002, '456')
	// 위와 같은 쿼리문을 만들기 위함.
	strSQL.Format(_T("INSERT INTO %S("), DB_TABLE_ADD_BILL_INFO);

	for(i=0; i<DB_TABLE_ADDBILL_FIELD_COUNT; i++)
	{
		strTemp.Format(_T("%s"), strDBTableAddBillFieldName[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_ADDBILL_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(") VALUES(");

	for(i=0; i<DB_TABLE_ADDBILL_FIELD_COUNT; i++)
	{
		if(0 == i)
			strTemp.Format(_T("%s"), strJNL[i]);	// JNL_NO는 숫자이므로
		else
			strTemp.Format(_T("'%s'"), strJNL[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_ADDBILL_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(")");
	///////////////////////////////////////////////////////////////

	sprintf(m_szSql, "%S", strSQL);

	return ExecuteSql(m_szSql);
}


/************************************************************
*	@brief		Insert Cassette Close Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::InsertCstCloseField(LPFLSEJCOMMINFO lpCommInfo, LPFLSEJCSTCLINFO lpJnlData)
{
	//sprintf(m_szSql, ("INSERT INTO %s VALUES (%d, '%s', '%s', '%s', '%s'," \
	//								"'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', " \
	//								"'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', " \
	//								"'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', " \
	//								"'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', " \
	//								"'%s', '%s', '%s', 's')"),
	//	DB_TABLE_CST_CLOSE_INFO,
	//	lpCommInfo->jnl_no,

	//	lpJnlData->term_id,
	//	lpJnlData->start_date,
	//	lpJnlData->start_time,
	//	lpJnlData->cst_close_type,

	//	lpJnlData->load_bill_cst1,
	//	lpJnlData->disp_bill_cst1,
	//	lpJnlData->rej_bill_cst1,
	//	lpJnlData->rem_bill_cst1,
	//	lpJnlData->load_amt_cst1,
	//	lpJnlData->disp_amt_cst1,
	//	lpJnlData->rem_amt_cst1,
	//	lpJnlData->denom_cst1,

	//	lpJnlData->load_bill_cst2,
	//	lpJnlData->disp_bill_cst2,
	//	lpJnlData->rej_bill_cst2,
	//	lpJnlData->rem_bill_cst2,
	//	lpJnlData->load_amt_cst2,
	//	lpJnlData->disp_amt_cst2,
	//	lpJnlData->rem_amt_cst2,
	//	lpJnlData->denom_cst2,

	//	lpJnlData->load_bill_cst3,
	//	lpJnlData->disp_bill_cst3,
	//	lpJnlData->rej_bill_cst3,
	//	lpJnlData->rem_bill_cst3,
	//	lpJnlData->load_amt_cst3,
	//	lpJnlData->disp_amt_cst3,
	//	lpJnlData->rem_amt_cst3,
	//	lpJnlData->denom_cst3,

	//	lpJnlData->load_bill_cst4,
	//	lpJnlData->disp_bill_cst4,
	//	lpJnlData->rej_bill_cst4,
	//	lpJnlData->rem_bill_cst4,
	//	lpJnlData->load_amt_cst4,
	//	lpJnlData->disp_amt_cst4,
	//	lpJnlData->rem_amt_cst4,
	//	lpJnlData->denom_cst4,

	//	lpJnlData->is_print,
	//	lpJnlData->is_ams_upload,
	//	lpJnlData->is_host_upload,

	//	lpJnlData->cst_count);

	//return ExecuteSql(m_szSql);

	#define DB_TABLE_CBXCLOSE_FIELD_COUNT	41

	CString strDBTableCBXCloseFieldName[DB_TABLE_CBXCLOSE_FIELD_COUNT] = {
		_T("JNL_NO"), 
		_T("TERM_ID"), _T("START_DATE"), _T("START_TIME"), _T("CST_CLOSE_TYPE"), _T("LOAD_BILL_CST1"), 
		_T("DISP_BILL_CST1"), _T("REJ_BILL_CST1"), _T("REM_BILL_CST1"), _T("LOAD_AMT_CST1"), _T("DISP_AMT_CST1"), 
		_T("REM_AMT_CST1"), _T("DENOM_CST1"), _T("LOAD_BILL_CST2"), _T("DISP_BILL_CST2"), _T("REJ_BILL_CST2"), 
		_T("REM_BILL_CST2"), _T("LOAD_AMT_CST2"), _T("DISP_AMT_CST2"), _T("REM_AMT_CST2"), _T("DENOM_CST2"), 
		_T("LOAD_BILL_CST3"), _T("DISP_BILL_CST3"), _T("REJ_BILL_CST3"), _T("REM_BILL_CST3"), _T("LOAD_AMT_CST3"), 
		_T("DISP_AMT_CST3"), _T("REM_AMT_CST3"), _T("DENOM_CST3"), _T("LOAD_BILL_CST4"), _T("DISP_BILL_CST4"), 
		_T("REJ_BILL_CST4"), _T("REM_BILL_CST4"), _T("LOAD_AMT_CST4"), _T("DISP_AMT_CST4"), _T("REM_AMT_CST4"), 
		_T("DENOM_CST4"),_T("IS_PRINT"), _T("IS_AMS_UPLOAD"), _T("IS_HOST_UPLOAD"), _T("CST_COUNT")
	};

	CString strSQL, strTemp;
	CString strJNL[DB_TABLE_CBXCLOSE_FIELD_COUNT] = {_T(""),};
	int i=0;
	int nIndex = 0;

	strJNL[nIndex++].Format(_T("%d"), lpCommInfo->jnl_no);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->term_id);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->start_date);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->start_time);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->cst_close_type);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->load_bill_cst1);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->disp_bill_cst1);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->rej_bill_cst1);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->rem_bill_cst1);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->load_amt_cst1);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->disp_amt_cst1);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->rem_amt_cst1);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->denom_cst1);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->load_bill_cst2);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->disp_bill_cst2);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->rej_bill_cst2);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->rem_bill_cst2);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->load_amt_cst2);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->disp_amt_cst2);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->rem_amt_cst2);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->denom_cst2);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->load_bill_cst3);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->disp_bill_cst3);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->rej_bill_cst3);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->rem_bill_cst3);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->load_amt_cst3);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->disp_amt_cst3);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->rem_amt_cst3);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->denom_cst3);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->load_bill_cst4);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->disp_bill_cst4);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->rej_bill_cst4);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->rem_bill_cst4);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->load_amt_cst4);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->disp_amt_cst4);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->rem_amt_cst4);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->denom_cst4);


	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_print);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_ams_upload);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_host_upload);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->cst_count);


	// ex) INSERT INTO TBL_TRANS_INFO(JNL_NO,TERM_ID) VALUES(1002, '456')
	// 위와 같은 쿼리문을 만들기 위함.
	strSQL.Format(_T("INSERT INTO %S("), DB_TABLE_CST_CLOSE_INFO);

	for(i=0; i<DB_TABLE_CBXCLOSE_FIELD_COUNT; i++)
	{
		strTemp.Format(_T("%s"), strDBTableCBXCloseFieldName[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_CBXCLOSE_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(") VALUES(");

	for(i=0; i<DB_TABLE_CBXCLOSE_FIELD_COUNT; i++)
	{
		if(0 == i)
			strTemp.Format(_T("%s"), strJNL[i]);	// JNL_NO는 숫자이므로
		else
			strTemp.Format(_T("'%s'"), strJNL[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_CBXCLOSE_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(")");
	///////////////////////////////////////////////////////////////

	sprintf(m_szSql, "%S", strSQL);

	return ExecuteSql(m_szSql);
}


/************************************************************
*	@brief		Insert Day Close Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::InsertDayCloseField(LPFLSEJCOMMINFO lpCommInfo, LPFLSEJDAYCLINFO lpJnlData)
{
	//sprintf(m_szSql, ("INSERT INTO %s VALUES (%d, '%s', '%s', '%s', " \
	//								"'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', " \
	//								"'%s', '%s', '%s')"),
	//	DB_TABLE_DAY_CLOSE_INFO,
	//	lpCommInfo->jnl_no,

	//	lpJnlData->term_id,
	//	lpJnlData->start_date,
	//	lpJnlData->start_time,

	//	lpJnlData->day_close_type,
	//	lpJnlData->day_close_Result,

	//	lpJnlData->host_with_cnt,
	//	lpJnlData->host_bal_cnt,
	//	lpJnlData->host_trans_cnt,
	//	lpJnlData->total_host_amt,
	//	lpJnlData->term_with_cnt,
	//	lpJnlData->term_bal_cnt,
	//	lpJnlData->term_trans_cnt,
	//	lpJnlData->total_term_amt,

	//	lpJnlData->is_print,
	//	lpJnlData->is_ams_upload,
	//	lpJnlData->is_host_upload
	//	);

	//return ExecuteSql(m_szSql);

	#define DB_TABLE_DAYCLOSE_FIELD_COUNT	17

	CString strDBTableDayCloseFieldName[DB_TABLE_DAYCLOSE_FIELD_COUNT] = {
		_T("JNL_NO"), 
		_T("TERM_ID"), _T("START_DATE"), _T("START_TIME"), _T("DAY_CLOSE_TYPE"), _T("DAY_CLOSE_RESULT"), 
		_T("HOST_WITH_CNT"), _T("HOST_BAL_CNT"), _T("HOST_TRANS_CNT"), _T("TOTAL_HOST_AMT"), _T("TERM_WITH_CNT"), 
		_T("TERM_BAL_CNT"), _T("TERM_TRANS_CNT"), _T("TOTAL_TERM_AMT"), _T("IS_PRINT"), _T("IS_AMS_UPLOAD"), 
		_T("IS_HOST_UPLOAD")
	};

	CString strSQL, strTemp;
	CString strJNL[DB_TABLE_DAYCLOSE_FIELD_COUNT] = {_T(""),};
	int i=0;
	int nIndex = 0;

	strJNL[nIndex++].Format(_T("%d"), lpCommInfo->jnl_no);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->term_id);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->start_date);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->start_time);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->day_close_type);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->day_close_Result);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->host_with_cnt);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->host_bal_cnt);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->host_trans_cnt);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->total_host_amt);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->term_with_cnt);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->term_bal_cnt);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->term_trans_cnt);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->total_term_amt);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_print);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_ams_upload);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_host_upload);


	// ex) INSERT INTO TBL_TRANS_INFO(JNL_NO,TERM_ID) VALUES(1002, '456')
	// 위와 같은 쿼리문을 만들기 위함.
	strSQL.Format(_T("INSERT INTO %S("), DB_TABLE_DAY_CLOSE_INFO);

	for(i=0; i<DB_TABLE_DAYCLOSE_FIELD_COUNT; i++)
	{
		strTemp.Format(_T("%s"), strDBTableDayCloseFieldName[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_DAYCLOSE_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(") VALUES(");

	for(i=0; i<DB_TABLE_DAYCLOSE_FIELD_COUNT; i++)
	{
		if(0 == i)
			strTemp.Format(_T("%s"), strJNL[i]);	// JNL_NO는 숫자이므로
		else
			strTemp.Format(_T("'%s'"), strJNL[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_DAYCLOSE_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(")");
	///////////////////////////////////////////////////////////////

	sprintf(m_szSql, "%S", strSQL);

	return ExecuteSql(m_szSql);
}


/************************************************************
*	@brief		Insert Transaction Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::InsertTransField(LPFLSEJCOMMINFO lpCommInfo, LPFLSEJTRANINFO lpJnlData)
{
	//sprintf(m_szSql, ("INSERT INTO %s VALUES (%d, " \
	//							"'%s', '%s', '%s', '%s', '%s', " \
	//							"'%s', '%s', '%s', '%s', '%s', " \
	//							"'%s', '%s', '%s', '%s', '%s', " \
	//							"'%s', '%s', '%s', '%s', '%s', " \
	//							"'%s', '%s', '%s', '%s', '%s', " \
	//							"'%s', '%s', '%s', '%s', '%s', " \
	//							"'%s')"),
	//	DB_TABLE_TRANS_INFO,
	//	lpCommInfo->jnl_no,

	//	lpJnlData->term_id,
	//	lpJnlData->tran_seq_no,
	//	lpJnlData->tran_cd,
	//	lpJnlData->card_tr2,
	//	lpJnlData->auth_no,

	//	lpJnlData->tran_date,
	//	lpJnlData->tran_time,
	//	lpJnlData->business_date,
	//	lpJnlData->account_type,
	//	lpJnlData->req_amt,

	//	lpJnlData->disp_amt,
	//	lpJnlData->surc_amt,
	//	lpJnlData->bal_amt,
	//	lpJnlData->avail_bal_amt,
	//	lpJnlData->disp_cnt,

	//	lpJnlData->reject_cnt,
	//	lpJnlData->err_code_1,
	//	lpJnlData->err_code_2,
	//	lpJnlData->err_code_3,
	//	lpJnlData->err_msg,

	//	lpJnlData->is_print,
	//	lpJnlData->is_ams_upload,
	//	lpJnlData->is_host_upload,
	//	lpJnlData->cst1_reject_info,
	//	lpJnlData->cst2_reject_info,

	//	lpJnlData->cst3_reject_info,
	//	lpJnlData->cst4_reject_info,
	//    lpJnlData->cst_remain_cnt,
	//    lpJnlData->emv_data,
	//    lpJnlData->dcc_data,
	//	lpJnlData->cam_img_filename
	//	);

	//return ExecuteSql(m_szSql);

	#define DB_TABLE_TRANS_FIELD_COUNT	32

	CString strDBTableTransFieldName[DB_TABLE_TRANS_FIELD_COUNT] = {
		_T("JNL_NO"), 
		_T("TERM_ID"), _T("TRAN_SEQ_NO"), _T("TRAN_CD"), _T("CARD_TR2"), _T("AUTH_NO"), 
		_T("TRAN_DATE"), _T("TRAN_TIME"), _T("BUSINESS_DATE"), _T("ACCOUNT_TYPE"), _T("REQ_AMT"), 
		_T("DISP_AMT"), _T("SURC_AMT"), _T("BAL_AMT"), _T("AVAIL_BAL_AMT"), _T("DISPENSED_CNT"), 
		_T("REJECTED_CNT"), _T("ERR_CODE_1"), _T("ERR_CODE_2"), _T("ERR_CODE_3"), _T("ERR_MSG"), 
		_T("IS_PRINT"), _T("IS_AMS_UPLOAD"), _T("IS_HOST_UPLOAD"), _T("CST1_REJECT_INFO"), _T("CST2_REJECT_INFO"), 
		_T("CST3_REJECT_INFO"), _T("CST4_REJECT_INFO"), _T("CST_REMAIN_CNT"), _T("EMVDATA"), _T("DCCDATA"), 
		_T("CAM_IMG_INFO")
	};

	CString strSQL, strTemp;
	CString strJNL[DB_TABLE_TRANS_FIELD_COUNT] = {_T(""),};
	int i=0;
	int nIndex = 0;

	strJNL[nIndex++].Format(_T("%d"), lpCommInfo->jnl_no);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->term_id);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->tran_seq_no);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->tran_cd);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->card_tr2);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->auth_no);
	
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->tran_date);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->tran_time);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->business_date);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->account_type);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->req_amt);
	
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->disp_amt);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->surc_amt);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->bal_amt);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->avail_bal_amt);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->disp_cnt);
	
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->reject_cnt);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->err_code_1);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->err_code_2);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->err_code_3);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->err_msg);
	
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_print);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_ams_upload);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_host_upload);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->cst1_reject_info);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->cst2_reject_info);
	
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->cst3_reject_info);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->cst4_reject_info);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->cst_remain_cnt);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->emv_data);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->dcc_data);
	
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->cam_img_filename);
	
	// ex) INSERT INTO TBL_TRANS_INFO(JNL_NO,TERM_ID) VALUES(1002, '456')
	// 위와 같은 쿼리문을 만들기 위함.
	strSQL.Format(_T("INSERT INTO %S("), DB_TABLE_TRANS_INFO);

	for(i=0; i<DB_TABLE_TRANS_FIELD_COUNT; i++)
	{
		strTemp.Format(_T("%s"), strDBTableTransFieldName[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_TRANS_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(") VALUES(");
	
	for(i=0; i<DB_TABLE_TRANS_FIELD_COUNT; i++)
	{
		if(0 == i)
			strTemp.Format(_T("%s"), strJNL[i]);	// JNL_NO는 숫자이므로
		else
			strTemp.Format(_T("'%s'"), strJNL[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_TRANS_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(")");
	///////////////////////////////////////////////////////////////

	sprintf(m_szSql, "%S", strSQL);

	return ExecuteSql(m_szSql);
}


/************************************************************
*	@brief		Insert Error Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::InsertTextField(LPFLSEJCOMMINFO lpCommInfo, LPEJTEXTINFO lpJnlData)
{
	//sprintf(m_szSql, ("INSERT INTO %s VALUES (%d, '%s', '%s', '%s', '%s', '%s')"),
	//	DB_TABLE_TEXT_INFO,
	//	lpCommInfo->jnl_no,
	//	lpJnlData->term_id,
	//	lpJnlData->text_value,
	//	lpJnlData->is_print,
	//	lpJnlData->is_ams_upload,
	//	lpJnlData->is_host_upload
	//	);

	//return ExecuteSql(m_szSql);

	#define DB_TABLE_TEXT_FIELD_COUNT	6

	CString strDBTableTextFieldName[DB_TABLE_TEXT_FIELD_COUNT] = {
		_T("JNL_NO"), 
		_T("TERM_ID"), _T("TEXT_VALUE"), _T("IS_PRINT"), _T("IS_AMS_UPLOAD"), _T("IS_HOST_UPLOAD")
	};

	CString strSQL, strTemp;
	CString strJNL[DB_TABLE_TEXT_FIELD_COUNT] = {_T(""),};
	int i=0;
	int nIndex = 0;

	strJNL[nIndex++].Format(_T("%d"), lpCommInfo->jnl_no);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->term_id);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->text_value);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_print);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_ams_upload);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_host_upload);


	// ex) INSERT INTO TBL_TRANS_INFO(JNL_NO,TERM_ID) VALUES(1002, '456')
	// 위와 같은 쿼리문을 만들기 위함.
	strSQL.Format(_T("INSERT INTO %S("), DB_TABLE_TEXT_INFO);

	for(i=0; i<DB_TABLE_TEXT_FIELD_COUNT; i++)
	{
		strTemp.Format(_T("%s"), strDBTableTextFieldName[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_TEXT_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(") VALUES(");

	for(i=0; i<DB_TABLE_TEXT_FIELD_COUNT; i++)
	{
		if(0 == i)
			strTemp.Format(_T("%s"), strJNL[i]);	// JNL_NO는 숫자이므로
		else
			strTemp.Format(_T("'%s'"), strJNL[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_TEXT_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(")");
	///////////////////////////////////////////////////////////////

	sprintf(m_szSql, "%S", strSQL);

	return ExecuteSql(m_szSql);

}


/************************************************************
*	@brief		Insert Denomination Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::InsertDenomField( LPFLSEJCOMMINFO lpCommInfo, LPFLSEJSETDENOMINFO lpJnlData )
{
	//sprintf(m_szSql, ("INSERT INTO %s VALUES (%d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')"),
	//	DB_TABLE_DENOM_INFO,
	//	lpCommInfo->jnl_no,
	//	lpJnlData->term_id,
	//	lpJnlData->start_date,
	//	lpJnlData->start_time,

	//	lpJnlData->olddenom_cst1,
	//	lpJnlData->olddenom_cst2,
	//	lpJnlData->olddenom_cst3,
	//	lpJnlData->olddenom_cst4,

	//	lpJnlData->newdenom_cst1,
	//	lpJnlData->newdenom_cst2,
	//	lpJnlData->newdenom_cst3,
	//	lpJnlData->newdenom_cst4,

	//	lpJnlData->is_print,
	//	lpJnlData->is_ams_upload,
	//	lpJnlData->is_host_upload,

	//	lpJnlData->cst_count);

	//return ExecuteSql(m_szSql);


	#define DB_TABLE_DENOM_FIELD_COUNT	16

	CString strDBTableDenomFieldName[DB_TABLE_DENOM_FIELD_COUNT] = {
		_T("JNL_NO"), 
		_T("TERM_ID"), _T("START_DATE"), _T("START_TIME"), _T("OLDDENOM_CST1"), _T("OLDDENOM_CST2"), 
		_T("OLDDENOM_CST3"), _T("OLDDENOM_CST4"), _T("NEWDENOM_CST1"), _T("NEWDENOM_CST2"), _T("NEWDENOM_CST3"), 
		_T("NEWDENOM_CST4"), _T("IS_PRINT"), _T("IS_AMS_UPLOAD"), _T("IS_HOST_UPLOAD"), _T("CST_COUNT")
	};

	CString strSQL, strTemp;
	CString strJNL[DB_TABLE_DENOM_FIELD_COUNT] = {_T(""),};
	int i=0;
	int nIndex = 0;

	strJNL[nIndex++].Format(_T("%d"), lpCommInfo->jnl_no);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->term_id);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->start_date);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->start_time);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->olddenom_cst1);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->olddenom_cst2);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->olddenom_cst3);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->olddenom_cst4);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->newdenom_cst1);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->newdenom_cst2);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->newdenom_cst3);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->newdenom_cst4);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_print);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_ams_upload);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_host_upload);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->cst_count);

	// ex) INSERT INTO TBL_TRANS_INFO(JNL_NO,TERM_ID) VALUES(1002, '456')
	// 위와 같은 쿼리문을 만들기 위함.
	strSQL.Format(_T("INSERT INTO %S("), DB_TABLE_DENOM_INFO);

	for(i=0; i<DB_TABLE_DENOM_FIELD_COUNT; i++)
	{
		strTemp.Format(_T("%s"), strDBTableDenomFieldName[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_DENOM_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(") VALUES(");

	for(i=0; i<DB_TABLE_DENOM_FIELD_COUNT; i++)
	{
		if(0 == i)
			strTemp.Format(_T("%s"), strJNL[i]);	// JNL_NO는 숫자이므로
		else
			strTemp.Format(_T("'%s'"), strJNL[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_DENOM_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(")");
	///////////////////////////////////////////////////////////////

	sprintf(m_szSql, "%S", strSQL);

	return ExecuteSql(m_szSql);

}


/************************************************************
*	@brief		Insert TestDispense Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::InsertTestDispenseField(LPFLSEJCOMMINFO lpCommInfo, LPFLSEJTESTDISPENSE lpJnlData)
{
	//	sprintf(m_szSql, ("INSERT INTO %s VALUES (%d, '%s', '%s', '%s', '%s'," \
	//										"'%s', '%s', '%s', '%s', '%s', " \
	//										"'%s', '%s', '%s', '%s', '%s', " \
	//										"'%s', '%s', '%s', '%s', '%s', " \
	//										"'%s', '%s', '%s', '%s', '%s', " \
	//										"'%s', '%s', '%s', '%s')"),
	//	DB_TABLE_TEST_DISPENSE_INFO,
	//	lpCommInfo->jnl_no,

	//	lpJnlData->term_id,
	//	lpJnlData->start_date,
	//	lpJnlData->start_time,
	//	lpJnlData->test_disp_result,

	//	lpJnlData->test_req_bill_cst[0],
	//	lpJnlData->test_pickup_bill_cst[0],
	//	lpJnlData->test_rej_bill_cst[0],
	//	lpJnlData->rem_bill_cst[0],
	//	lpJnlData->denom_cst[0],
	//	
	//	lpJnlData->test_req_bill_cst[1],
	//	lpJnlData->test_pickup_bill_cst[1],
	//	lpJnlData->test_rej_bill_cst[1],
	//	lpJnlData->rem_bill_cst[1],
	//	lpJnlData->denom_cst[1],

	//	lpJnlData->test_req_bill_cst[2],
	//	lpJnlData->test_pickup_bill_cst[2],
	//	lpJnlData->test_rej_bill_cst[2],
	//	lpJnlData->rem_bill_cst[2],
	//	lpJnlData->denom_cst[2],

	//	lpJnlData->test_req_bill_cst[3],
	//	lpJnlData->test_pickup_bill_cst[3],
	//	lpJnlData->test_rej_bill_cst[3],
	//	lpJnlData->rem_bill_cst[3],
	//	lpJnlData->denom_cst[3],

	//	lpJnlData->is_print,
	//	lpJnlData->is_ams_upload,
	//	lpJnlData->is_host_upload,

	//	lpJnlData->cst_count);

	//return ExecuteSql(m_szSql);

	#define DB_TABLE_TESTDISPENSE_FIELD_COUNT	30

	CString strDBTableTestDispenseFieldName[DB_TABLE_TESTDISPENSE_FIELD_COUNT] = {
		_T("JNL_NO"), 
		_T("TERM_ID"), _T("START_DATE"), _T("START_TIME"), _T("TEST_DISP_RESULT"), _T("TEST_REQ_BILL_CST1"), 
		_T("TEST_PICKUP_BILL_CST1"), _T("TEST_REJ_BILL_CST1"), _T("REM_BILL_CST1"), _T("DENOM_CST1"), _T("TEST_REQ_BILL_CST2"), 
		_T("TEST_PICKUP_BILL_CST2"), _T("TEST_REJ_BILL_CST2"), _T("REM_BILL_CST2"), _T("DENOM_CST2"), _T("TEST_REQ_BILL_CST3"), 
		_T("TEST_PICKUP_BILL_CST3"), _T("TEST_REJ_BILL_CST3"), _T("REM_BILL_CST3"), _T("DENOM_CST3"), _T("TEST_REQ_BILL_CST4"), 
		_T("TEST_PICKUP_BILL_CST4"), _T("TEST_REJ_BILL_CST4"), _T("REM_BILL_CST4"), _T("DENOM_CST4"), _T("IS_PRINT"), 
		_T("IS_AMS_UPLOAD"), _T("IS_HOST_UPLOAD"), _T("CST_COUNT"), _T("ERR_CODE")
	};

	CString strSQL, strTemp;
	CString strJNL[DB_TABLE_TESTDISPENSE_FIELD_COUNT] = {_T(""),};
	int i=0;
	int nIndex = 0;

	strJNL[nIndex++].Format(_T("%d"), lpCommInfo->jnl_no);

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->term_id);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->start_date);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->start_time);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->test_disp_result);

	for(i=0; i<4; i++)
	{
		strJNL[nIndex++].Format(_T("%S"), lpJnlData->test_req_bill_cst[i]);
		strJNL[nIndex++].Format(_T("%S"), lpJnlData->test_pickup_bill_cst[i]);
		strJNL[nIndex++].Format(_T("%S"), lpJnlData->test_rej_bill_cst[i]);
		strJNL[nIndex++].Format(_T("%S"), lpJnlData->rem_bill_cst[i]);
		strJNL[nIndex++].Format(_T("%S"), lpJnlData->denom_cst[i]);
	}

	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_print);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_ams_upload);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->is_host_upload);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->cst_count);
	strJNL[nIndex++].Format(_T("%S"), lpJnlData->err_code);

	// ex) INSERT INTO TBL_TRANS_INFO(JNL_NO,TERM_ID) VALUES(1002, '456')
	// 위와 같은 쿼리문을 만들기 위함.
	strSQL.Format(_T("INSERT INTO %S("), DB_TABLE_TEST_DISPENSE_INFO);

	for(i=0; i<DB_TABLE_TESTDISPENSE_FIELD_COUNT; i++)
	{
		strTemp.Format(_T("%s"), strDBTableTestDispenseFieldName[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_TESTDISPENSE_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(") VALUES(");

	for(i=0; i<DB_TABLE_TESTDISPENSE_FIELD_COUNT; i++)
	{
		if(0 == i)
			strTemp.Format(_T("%s"), strJNL[i]);	// JNL_NO는 숫자이므로
		else
			strTemp.Format(_T("'%s'"), strJNL[i]);
		strSQL += strTemp;

		if(i < (DB_TABLE_TESTDISPENSE_FIELD_COUNT-1))
			strSQL += _T(",");
	}
	strSQL += _T(")");
	///////////////////////////////////////////////////////////////

	sprintf(m_szSql, "%S", strSQL);

	return ExecuteSql(m_szSql);
}


/************************************************************
*	@brief		Execute Insert SQL
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::ExecuteSql(LPCSTR pszSQL)
{
	CString strtemp(pszSQL);

	sqlite3_stmt* stmt;

	if (SQLITE_OK != _sqlite3_prepare(m_db, strtemp, -1, &stmt, 0))
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
*	@brief		Get Common Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetCommField(LPFLSEJCOMMINFO lpCommInfo, sqlite3_stmt* stmt)
{
	int nIndex = 0;

	lpCommInfo->jnl_no = _sqlite3_column_int(stmt, nIndex++);

	//CT2A szLogDate((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpCommInfo->log_date, szLogDate);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpCommInfo->log_date, m_szTemp);

	//CT2A szLogTime((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpCommInfo->log_time, szLogTime);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpCommInfo->log_time, m_szTemp);

	lpCommInfo->jnl_cd = _sqlite3_column_int(stmt, nIndex++);

	//CT2A szSerchDate((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpCommInfo->log_srch_date, szSerchDate);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpCommInfo->log_srch_date, m_szTemp);

	return TRUE;
}


/************************************************************
*	@brief		Get Password Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetPwdField(LPEJCHANGEINFO lpJnlData, sqlite3_stmt* stmt)
{
	int nIndex = 0;

	lpJnlData->jnl_no = _sqlite3_column_int(stmt, nIndex++);

	//CT2A szTermID((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->term_id, szTermID);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->term_id, m_szTemp);
	
	//CT2A szOldVal((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->old_value, szOldVal);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->old_value, m_szTemp);
	
	//CT2A szNewVal((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->new_value, szNewVal);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->new_value, m_szTemp);

	//CT2A szIsPrint((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_print, szIsPrint);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_print, m_szTemp);
	
	//CT2A szIsAmsUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_ams_upload, szIsAmsUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_ams_upload, m_szTemp);
	
	//CT2A szIsHostUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_host_upload, szIsHostUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_host_upload, m_szTemp);

	return TRUE;
}


/************************************************************
*	@brief		Get Key Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetKeyField(LPEJCHANGEINFO lpJnlData, sqlite3_stmt* stmt)
{
	int nIndex = 0;

	lpJnlData->jnl_no = _sqlite3_column_int(stmt, nIndex++);

	//CT2A szTermID((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->term_id, szTermID);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->term_id, m_szTemp);

	//CT2A szOldVal((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->old_value, szOldVal);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->old_value, m_szTemp);

	//CT2A szNewVal((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->new_value, szNewVal);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->new_value, m_szTemp);

	//CT2A szIsPrint((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_print, szIsPrint);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_print, m_szTemp);

	//CT2A szIsAmsUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_ams_upload, szIsAmsUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_ams_upload, m_szTemp);

	//CT2A szIsHostUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_host_upload, szIsHostUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_host_upload, m_szTemp);

	return TRUE;
}


/************************************************************
*	@brief		Get Add Bill Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetAddBillField(LPFLSEJADDBILLINFO lpJnlData, sqlite3_stmt* stmt)
{
	int nIndex = 0;

	lpJnlData->jnl_no = sqlite3_column_int(stmt, nIndex++);

	//CT2A szTermID((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->term_id, szTermID);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->term_id, m_szTemp);

	//CT2A szStartDate((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->start_date, szStartDate);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->start_date, m_szTemp);

	//CT2A szStartTime((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->start_time, szStartTime);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->start_time, m_szTemp);

	//CT2A szAddBillCST1((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->add_bill_cst1, szAddBillCST1);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->add_bill_cst1, m_szTemp);

	//CT2A szRemainCST1((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->remain_cst1, szRemainCST1);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->remain_cst1, m_szTemp);

	//CT2A szAddBillCST2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->add_bill_cst2, szAddBillCST2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->add_bill_cst2, m_szTemp);

	//CT2A szRemainCST2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->remain_cst2, szRemainCST2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->remain_cst2, m_szTemp);

	//CT2A szAddBillCST3((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->add_bill_cst3, szAddBillCST3);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->add_bill_cst3, m_szTemp);

	//CT2A szRemainCST3((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->remain_cst3, szRemainCST3);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->remain_cst3, m_szTemp);

	//CT2A szAddBillCST4((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->add_bill_cst4, szAddBillCST4);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->add_bill_cst4, m_szTemp);

	//CT2A szRemainCST4((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->remain_cst4, szRemainCST4);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->remain_cst4, m_szTemp);

	//CT2A szIsPrint((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_print, szIsPrint);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_print, m_szTemp);

	//CT2A szIsAmsUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_ams_upload, szIsAmsUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_ams_upload, m_szTemp);

	//CT2A szIsHostUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_host_upload, szIsHostUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_host_upload, m_szTemp);

	//CT2A szCSTCount((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->cst_count, szCSTCount);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->cst_count, m_szTemp);

	return TRUE;
}


/************************************************************
*	@brief		Get Cassette Close Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetCstCloseField(LPFLSEJCSTCLINFO lpJnlData, sqlite3_stmt* stmt)
{
	int nIndex = 0;

	lpJnlData->jnl_no = _sqlite3_column_int(stmt, nIndex++);

	//CT2A szTermID((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->term_id, szTermID);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->term_id, m_szTemp);

	//CT2A szStartDate((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->start_date, szStartDate);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->start_date, m_szTemp);

	//CT2A szStartTime((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->start_time, szStartTime);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->start_time, m_szTemp);

	//CT2A szCloseType((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->cst_close_type, szCloseType);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->cst_close_type, m_szTemp);
	

	//CST1
	//CT2A szLoadCST1((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->load_bill_cst1, szLoadCST1);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->load_bill_cst1, m_szTemp);

	//CT2A szDispCST1((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->disp_bill_cst1, szDispCST1);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->disp_bill_cst1, m_szTemp);

	//CT2A szRejCST1((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->rej_bill_cst1, szRejCST1);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->rej_bill_cst1, m_szTemp);

	//CT2A szRemCST1((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->rem_bill_cst1, szRemCST1);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->rem_bill_cst1, m_szTemp);

	//CT2A szLoadAmt1((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->load_amt_cst1, szLoadAmt1);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->load_amt_cst1, m_szTemp);

	//CT2A szDispAmt1((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->disp_amt_cst1, szDispAmt1);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->disp_amt_cst1, m_szTemp);

	//CT2A szRemAmt1((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->rem_amt_cst1, szRemAmt1);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->rem_amt_cst1, m_szTemp);

	//CT2A szDenom1((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->denom_cst1, szDenom1);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->denom_cst1, m_szTemp);


	//CST2
	//CT2A szLoadCST2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->load_bill_cst2, szLoadCST2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->load_bill_cst2, m_szTemp);

	//CT2A szDispCST2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->disp_bill_cst2, szDispCST2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->disp_bill_cst2, m_szTemp);

	//CT2A szRejCST2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->rej_bill_cst2, szRejCST2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->rej_bill_cst2, m_szTemp);

	//CT2A szRemCST2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->rem_bill_cst2, szRemCST2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->rem_bill_cst2, m_szTemp);

	//CT2A szLoadAmt2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->load_amt_cst2, szLoadAmt2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->load_amt_cst2, m_szTemp);

	//CT2A szDispAmt2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->disp_amt_cst2, szDispAmt2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->disp_amt_cst2, m_szTemp);

	//CT2A szRemAmt2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->rem_amt_cst2, szRemAmt2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->rem_amt_cst2, m_szTemp);

	//CT2A szDenom2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->denom_cst2, szDenom2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->denom_cst2, m_szTemp);


	//CST3
	//CT2A szLoadCST3((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->load_bill_cst3, szLoadCST3);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->load_bill_cst3, m_szTemp);

	//CT2A szDispCST3((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->disp_bill_cst3, szDispCST3);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->disp_bill_cst3, m_szTemp);

	//CT2A szRejCST3((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->rej_bill_cst3, szRejCST3);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->rej_bill_cst3, m_szTemp);

	//CT2A szRemCST3((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->rem_bill_cst3, szRemCST3);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->rem_bill_cst3, m_szTemp);

	//CT2A szLoadAmt3((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->load_amt_cst3, szLoadAmt3);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->load_amt_cst3, m_szTemp);

	//CT2A szDispAmt3((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->disp_amt_cst3, szDispAmt3);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->disp_amt_cst3, m_szTemp);

	//CT2A szRemAmt3((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->rem_amt_cst3, szRemAmt3);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->rem_amt_cst3, m_szTemp);

	//CT2A szDenom3((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->denom_cst3, szDenom3);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->denom_cst3, m_szTemp);


	//CST4
	//CT2A szLoadCST4((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->load_bill_cst4, szLoadCST4);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->load_bill_cst4, m_szTemp);

	//CT2A szDispCST4((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->disp_bill_cst4, szDispCST4);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->disp_bill_cst4, m_szTemp);

	//CT2A szRejCST4((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->rej_bill_cst4, szRejCST4);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->rej_bill_cst4, m_szTemp);

	//CT2A szRemCST4((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->rem_bill_cst4, szRemCST4);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->rem_bill_cst4, m_szTemp);

	//CT2A szLoadAmt4((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->load_amt_cst4, szLoadAmt4);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->load_amt_cst4, m_szTemp);

	//CT2A szDispAmt4((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->disp_amt_cst4, szDispAmt4);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->disp_amt_cst4, m_szTemp);

	//CT2A szRemAmt4((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->rem_amt_cst4, szRemAmt4);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->rem_amt_cst4, m_szTemp);

	//CT2A szDenom4((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->denom_cst4, szDenom4);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->denom_cst4, m_szTemp);

	//
	//CT2A szIsPrint((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_print, szIsPrint);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_print, m_szTemp);

	//CT2A szIsAmsUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_ams_upload, szIsAmsUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_ams_upload, m_szTemp);

	//CT2A szIsHostUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_host_upload, szIsHostUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_host_upload, m_szTemp);

	//CT2A szCSTCount((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->cst_count, szCSTCount);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->cst_count, m_szTemp);

	return TRUE;
}


/************************************************************
*	@brief		Get Day Close Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetDayCloseField(LPFLSEJDAYCLINFO lpJnlData, sqlite3_stmt* stmt)
{
	int nIndex = 0;

	lpJnlData->jnl_no = _sqlite3_column_int(stmt, nIndex++);

	//CT2A szTermID((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->term_id, szTermID);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->term_id, m_szTemp);

	//CT2A szStartDate((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->start_date, szStartDate);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->start_date, m_szTemp);

	//CT2A szStartTime((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->start_time, szStartTime);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->start_time, m_szTemp);


	//CT2A szCloseType((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->day_close_type, szCloseType);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->day_close_type, m_szTemp);

	//CT2A szCloseResult((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->day_close_Result, szCloseResult);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->day_close_Result, m_szTemp);


	//CT2A szHostWith((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->host_with_cnt, szHostWith);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->host_with_cnt, m_szTemp);

	//CT2A szHostBal((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->host_bal_cnt, szHostBal);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->host_bal_cnt, m_szTemp);

	//CT2A szHostTrans((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->host_trans_cnt, szHostTrans);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->host_trans_cnt, m_szTemp);

	//CT2A szTotalHost((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->total_host_amt, szTotalHost);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->total_host_amt, m_szTemp);

	//CT2A szTermWith((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->term_with_cnt, szTermWith);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->term_with_cnt, m_szTemp);

	//CT2A szTermBal((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->term_bal_cnt, szTermBal);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->term_bal_cnt, m_szTemp);

	//CT2A szTermTrans((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->term_trans_cnt, szTermTrans);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->term_trans_cnt, m_szTemp);

	//CT2A szTotalTerm((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->total_term_amt, szTotalTerm);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->total_term_amt, m_szTemp);

	//
	//CT2A szIsPrint((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_print, szIsPrint);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_print, m_szTemp);

	//CT2A szIsAmsUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_ams_upload, szIsAmsUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_ams_upload, m_szTemp);

	//CT2A szIsHostUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_host_upload, szIsHostUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_host_upload, m_szTemp);

	return TRUE;
}


/************************************************************
*	@brief		Get Transaction Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetTransField(LPFLSEJTRANINFO lpJnlData, sqlite3_stmt* stmt)
{
	int nIndex = 0;

	lpJnlData->jnl_no = _sqlite3_column_int(stmt, nIndex++);

	//CT2A szTermID((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->term_id, szTermID);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->term_id, m_szTemp);

	//CT2A szTranSeq((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->tran_seq_no, szTranSeq);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->tran_seq_no, m_szTemp);

	//CT2A szTranCode((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->tran_cd, szTranCode);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->tran_cd, m_szTemp);

	//CT2A szCardTr2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->card_tr2, szCardTr2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->card_tr2, m_szTemp);

	//CT2A szAuthNo((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->auth_no, szAuthNo);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->auth_no, m_szTemp);

	//CT2A szTranDate((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->tran_date, szTranDate);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->tran_date, m_szTemp);

	//CT2A szTranTime((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->tran_time, szTranTime);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->tran_time, m_szTemp);

	//CT2A szBusinessDate((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->business_date, szBusinessDate);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->business_date, m_szTemp);

	//CT2A szAccountType((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->account_type, szAccountType);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->account_type, m_szTemp);

	//CT2A szReqAmt((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->req_amt, szReqAmt);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->req_amt, m_szTemp);

	//CT2A szDispAmt((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->disp_amt, szDispAmt);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->disp_amt, m_szTemp);

	//CT2A szSurcAmt((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->surc_amt, szSurcAmt);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->surc_amt, m_szTemp);

	//CT2A szBalAmt((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->bal_amt, szBalAmt);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->bal_amt, m_szTemp);

	//CT2A szAvailBalAmt((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->avail_bal_amt, szAvailBalAmt);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->avail_bal_amt, m_szTemp);

	//CT2A szDispCnt((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->disp_cnt, szDispCnt);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->disp_cnt, m_szTemp);

	//CT2A szRejectCnt((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->reject_cnt, szRejectCnt);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->reject_cnt, m_szTemp);

	//CT2A szErr1((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->err_code_1, szErr1);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->err_code_1, m_szTemp);

	//CT2A szErr2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->err_code_2, szErr2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->err_code_2, m_szTemp);

	//CT2A szErr3((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->err_code_3, szErr3);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->err_code_3, m_szTemp);

	//CT2A szErrMsg((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->err_msg, szErrMsg);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->err_msg, m_szTemp);

	//
	//CT2A szIsPrint((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_print, szIsPrint);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_print, m_szTemp);

	//CT2A szIsAmsUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_ams_upload, szIsAmsUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_ams_upload, m_szTemp);


	//CT2A szIsHostUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_host_upload, szIsHostUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_host_upload, m_szTemp);

	//CT2A szCST1_Reject_Info((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->cst1_reject_info, szCST1_Reject_Info);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->cst1_reject_info, m_szTemp);

	//CT2A szCST2_Reject_Info((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->cst2_reject_info, szCST2_Reject_Info);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->cst2_reject_info, m_szTemp);

	//CT2A szCST3_Reject_Info((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->cst3_reject_info, szCST3_Reject_Info);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->cst3_reject_info, m_szTemp);

	//CT2A szCST4_Reject_Info((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->cst4_reject_info, szCST4_Reject_Info);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->cst4_reject_info, m_szTemp);

	//CT2A szCST_Remain_Cnt((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->cst_remain_cnt, szCST_Remain_Cnt);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->cst_remain_cnt, m_szTemp);

	//CT2A szEMV_Data((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->emv_data, szEMV_Data);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->emv_data, m_szTemp);

	//CT2A szDCC_Data((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->dcc_data, szDCC_Data);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->dcc_data, m_szTemp);

	//CT2A szCAM_Img_Info((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->cam_img_filename, szCAM_Img_Info);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->cam_img_filename, m_szTemp);

	return TRUE;
}


/************************************************************
*	@brief		Get Error Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetTextField(LPEJTEXTINFO lpJnlData, sqlite3_stmt* stmt)
{
	int nIndex = 0;

	lpJnlData->jnl_no = _sqlite3_column_int(stmt, nIndex++);

	//CT2A szTermID((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->term_id, szTermID);

	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->term_id, m_szTemp);

	//CT2A szTextVal((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->text_value, szTextVal);

	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->text_value, m_szTemp);

	//
	//CT2A szIsPrint((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_print, szIsPrint);

	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_print, m_szTemp);

	//CT2A szIsAmsUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_ams_upload, szIsAmsUp);

	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_ams_upload, m_szTemp);


	//CT2A szIsHostUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_host_upload, szIsHostUp);

	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_host_upload, m_szTemp);


	return TRUE;
}


/************************************************************
*	@brief		Get Denomination Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetDenomField( LPFLSEJSETDENOMINFO lpJnlData, sqlite3_stmt* stmt )
{
	int nIndex = 0;

	lpJnlData->jnl_no = _sqlite3_column_int(stmt, nIndex++);

	//CT2A szTermID((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->term_id, szTermID);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->term_id, m_szTemp);

	//CT2A szStartDate((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->start_date, szStartDate);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->start_date, m_szTemp);

	//CT2A szStartTime((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->start_time, szStartTime);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->start_time, m_szTemp);


	//CT2A szOldDenomCST1((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->olddenom_cst1, szOldDenomCST1);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->olddenom_cst1, m_szTemp);

	//CT2A szOldDenomCST2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->olddenom_cst2, szOldDenomCST2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->olddenom_cst2, m_szTemp);

	//CT2A szOldDenomCST3((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->olddenom_cst3, szOldDenomCST3);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->olddenom_cst3, m_szTemp);

	//CT2A szOldDenomCST4((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->olddenom_cst4, szOldDenomCST4);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->olddenom_cst4, m_szTemp);

	
	//CT2A szNewDenomCST1((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->newdenom_cst1, szNewDenomCST1);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->newdenom_cst1, m_szTemp);

	//CT2A szNewDenomCST2((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->newdenom_cst2, szNewDenomCST2);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->newdenom_cst2, m_szTemp);

	//CT2A szNewDenomCST3((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->newdenom_cst3, szNewDenomCST3);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->newdenom_cst3, m_szTemp);

	//CT2A szNewDenomCST4((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->newdenom_cst4, szNewDenomCST4);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->newdenom_cst4, m_szTemp);

	//
	//CT2A szIsPrint((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_print, szIsPrint);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_print, m_szTemp);

	//CT2A szIsAmsUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_ams_upload, szIsAmsUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_ams_upload, m_szTemp);

	//CT2A szIsHostUp((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->is_host_upload, szIsHostUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->is_host_upload, m_szTemp);

	//CT2A szCSTCount((LPCTSTR)_sqlite3_column_text(stmt, nIndex++));
	//strcpy(lpJnlData->cst_count, szCSTCount);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, nIndex++));
	strcpy(lpJnlData->cst_count, m_szTemp);

	return TRUE;
}


/************************************************************
*	@brief		Get Test Dispense Info
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetTestDispenseField(LPFLSEJTESTDISPENSE lpJnlData, sqlite3_stmt* stmt)
{
	int index = 0;

	// common field
	lpJnlData->jnl_no = _sqlite3_column_int(stmt, index++);

	//CT2A szTermID((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->term_id, szTermID);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
	strcpy(lpJnlData->term_id, m_szTemp);

	//CT2A szStartDate((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->start_date, szStartDate);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
	strcpy(lpJnlData->start_date, m_szTemp);

	//CT2A szStartTime((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->start_time, szStartTime);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
	strcpy(lpJnlData->start_time, m_szTemp);

	//CT2A szTestDispResult((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->test_disp_result, szTestDispResult);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
	strcpy(lpJnlData->test_disp_result, m_szTemp);


	//CST1
	for(int i=0; i<4; i++)
	{
		//CT2A szTestReqCST1((LPCTSTR)_sqlite3_column_text(stmt, index++));
		//strcpy(lpJnlData->test_req_bill_cst[0], szTestReqCST1);
		memset(m_szTemp, 0, sizeof(m_szTemp));
		sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
		strcpy(lpJnlData->test_req_bill_cst[i], m_szTemp);

		//CT2A szTestPickupCST1((LPCTSTR)_sqlite3_column_text(stmt, index++));
		//strcpy(lpJnlData->test_pickup_bill_cst[0], szTestPickupCST1);
		memset(m_szTemp, 0, sizeof(m_szTemp));
		sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
		strcpy(lpJnlData->test_pickup_bill_cst[i], m_szTemp);

		//CT2A szTestRejCST1((LPCTSTR)_sqlite3_column_text(stmt, index++));
		//strcpy(lpJnlData->test_rej_bill_cst[0], szTestRejCST1);
		memset(m_szTemp, 0, sizeof(m_szTemp));
		sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
		strcpy(lpJnlData->test_rej_bill_cst[i], m_szTemp);

		//CT2A szRemCST1((LPCTSTR)_sqlite3_column_text(stmt, index++));
		//strcpy(lpJnlData->rem_bill_cst[0], szRemCST1);
		memset(m_szTemp, 0, sizeof(m_szTemp));
		sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
		strcpy(lpJnlData->rem_bill_cst[i], m_szTemp);

		//CT2A szDenom1((LPCTSTR)_sqlite3_column_text(stmt, index++));
		//strcpy(lpJnlData->denom_cst[0], szDenom1);
		memset(m_szTemp, 0, sizeof(m_szTemp));
		sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
		strcpy(lpJnlData->denom_cst[i], m_szTemp);
	}


	//CST2
	//CT2A szTestReqCST2((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->test_req_bill_cst[1], szTestReqCST2);

	//CT2A szTestPickupCST2((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->test_pickup_bill_cst[1], szTestPickupCST2);

	//CT2A szTestRejCST2((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->test_rej_bill_cst[1], szTestRejCST2);

	//CT2A szRemCST2((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->rem_bill_cst[1], szRemCST2);

	//CT2A szDenom2((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->denom_cst[1], szDenom2);


	////CST3
	//CT2A szTestReqCST3((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->test_req_bill_cst[2], szTestReqCST3);

	//CT2A szTestPickupCST3((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->test_pickup_bill_cst[2], szTestPickupCST3);

	//CT2A szTestRejCST3((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->test_rej_bill_cst[2], szTestRejCST3);

	//CT2A szRemCST3((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->rem_bill_cst[2], szRemCST3);

	//CT2A szDenom3((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->denom_cst[2], szDenom3);


	////CST4
	//CT2A szTestReqCST4((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->test_req_bill_cst[3], szTestReqCST4);

	//CT2A szTestPickupCST4((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->test_pickup_bill_cst[3], szTestPickupCST4);

	//CT2A szTestRejCST4((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->test_rej_bill_cst[3], szTestRejCST4);

	//CT2A szRemCST4((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->rem_bill_cst[3], szRemCST4);

	//CT2A szDenom4((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->denom_cst[3], szDenom4);


	//
	//CT2A szIsPrint((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->is_print, szIsPrint);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
	strcpy(lpJnlData->is_print, m_szTemp);

	//CT2A szIsAmsUp((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->is_ams_upload, szIsAmsUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
	strcpy(lpJnlData->is_ams_upload, m_szTemp);

	//CT2A szIsHostUp((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->is_host_upload, szIsHostUp);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
	strcpy(lpJnlData->is_host_upload, m_szTemp);

	//CT2A szCSTCount((LPCTSTR)_sqlite3_column_text(stmt, index++));
	//strcpy(lpJnlData->cst_count, szCSTCount);
	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
	strcpy(lpJnlData->cst_count, m_szTemp);

	memset(m_szTemp, 0, sizeof(m_szTemp));
	sprintf(m_szTemp, "%S", _sqlite3_column_text(stmt, index++));
	strcpy(lpJnlData->err_code, m_szTemp);

	return TRUE;
}


/************************************************************
*	@brief		Get Record Count
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetRecordCount(LPDWORD lpRecCnt)
{
	sprintf(m_szSql, ("SELECT COUNT(*) FROM %s"), DB_TABLE_JNL_INFO);

	CString strQuery(m_szSql);

	sqlite3_stmt* stmt;

	int rc = _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL);

	if (SQLITE_OK != rc)
	{
		return FALSE;
	}

	rc = _sqlite3_step(stmt);

	strQuery.Format(L"code = %d", rc);

	if (SQLITE_ROW != rc)
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}

	*lpRecCnt = _sqlite3_column_int(stmt, 0);

	strQuery.Format(L"ret = %d", *lpRecCnt);

	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Previous Record Count
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetPrevRecordCount(int nJnlNo, LPDWORD lpRecCnt)
{
	sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO < %d"), DB_TABLE_JNL_INFO, nJnlNo);

	CString strQuery(m_szSql);

	sqlite3_stmt* stmt;

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	*lpRecCnt = (WORD)_sqlite3_column_int(stmt, 0);


	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Next Record Count
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetNextRecordCount(int nJnlNo, LPDWORD lpRecCnt)
{
	sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO > %d"), DB_TABLE_JNL_INFO, nJnlNo);

	CString strQuery(m_szSql);

	sqlite3_stmt* stmt;

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	*lpRecCnt = (WORD)_sqlite3_column_int(stmt, 0);


	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Print Record Count
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetPrintRecordCount(LPDWORD lpRecCnt, int nAuditJnlNo)
{
	sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO > %d"), DB_TABLE_JNL_INFO, nAuditJnlNo);

	CString strQuery(m_szSql);

	sqlite3_stmt* stmt;

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	*lpRecCnt = (WORD)_sqlite3_column_int(stmt, 0);


	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Print Previous Record Count
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetPrintPrevRecordCount(int nJnlNo, LPDWORD lpRecCnt, int nAuditJnlNo)
{
	sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO < %d AND JNL_NO > %d "), DB_TABLE_JNL_INFO, nJnlNo, nAuditJnlNo);

	CString strQuery(m_szSql);

	sqlite3_stmt* stmt;

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	*lpRecCnt = (WORD)_sqlite3_column_int(stmt, 0);


	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Print Next Record Count
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetPrintNextRecordCount(int nJnlNo, LPDWORD lpRecCnt, int nAuditJnlNo)
{
	sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO > %d AND JNL_NO > %d"), DB_TABLE_JNL_INFO, nJnlNo, nAuditJnlNo);

	CString strQuery(m_szSql);

	sqlite3_stmt* stmt;

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	*lpRecCnt = (WORD)_sqlite3_column_int(stmt, 0);


	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Search Record Count
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetSearchRecordCount(BYTE bySrchType, LPSTR pSrchFrom, LPSTR pSrchTo, LPDWORD lpRecCnt)
{
	if (!strlen(pSrchFrom))
	{
		return FALSE;
	}


	sqlite3_stmt* stmt;


	if (JNL_NO == bySrchType)
	{
		if (!strlen(pSrchTo))
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO >= %s"), DB_TABLE_JNL_INFO, pSrchFrom);
		}
		else
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO >= %s AND JNL_NO <= %s"), DB_TABLE_JNL_INFO, pSrchFrom, pSrchTo);
		}
	}

	else if (TRAN_SEQ_NO == bySrchType)
	{
		if (!strlen(pSrchTo))
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE TRAN_SEQ_NO >= %s"), DB_TABLE_TRANS_INFO, pSrchFrom);
		}
		else
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE TRAN_SEQ_NO >= %s AND TRAN_SEQ_NO <= %s"), DB_TABLE_TRANS_INFO, pSrchFrom, pSrchTo);
		}
	}

	else if (DATE_RANGE == bySrchType)
	{
		if (!strlen(pSrchTo))
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE LOG_SRCH_DATE >= %s"), DB_TABLE_JNL_INFO, pSrchFrom);
		}
		else
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE LOG_SRCH_DATE >= %s AND LOG_SRCH_DATE <= %s"), DB_TABLE_JNL_INFO, pSrchFrom, pSrchTo);
		}
	}

	else
	{
		return FALSE;
	}


	CString strQuery(m_szSql);

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	*lpRecCnt = (WORD)_sqlite3_column_int(stmt, 0);


	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}


	return TRUE;
}


/************************************************************
*	@brief		Get Search Previous Record Count
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetSearchPrevRecordCount(BYTE bySrchType, LPSTR pSrchFrom, LPSTR pSrchTo, int nJnlNo, LPDWORD lpRecCnt)
{
	if (!strlen(pSrchFrom))
	{
		return FALSE;
	}


	sqlite3_stmt* stmt;


	if (JNL_NO == bySrchType)
	{
		if (!strlen(pSrchTo))
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO < %d AND JNL_NO >= %s"), DB_TABLE_JNL_INFO, nJnlNo, pSrchFrom);
		}
		else
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO < %d AND JNL_NO >= %s AND JNL_NO <= %s"), DB_TABLE_JNL_INFO, nJnlNo, pSrchFrom, pSrchTo);
		}
	}

	else if (TRAN_SEQ_NO == bySrchType)
	{
		if (!strlen(pSrchTo))
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO < %d AND TRAN_SEQ_NO >= %s"), DB_TABLE_TRANS_INFO, nJnlNo, pSrchFrom);
		}
		else
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO < %d AND TRAN_SEQ_NO >= %s AND TRAN_SEQ_NO <= %s"), DB_TABLE_TRANS_INFO, nJnlNo, pSrchFrom, pSrchTo);
		}
	}

	else if (DATE_RANGE == bySrchType)
	{
		if (!strlen(pSrchTo))
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO < %d AND LOG_SRCH_DATE >= %s"), DB_TABLE_JNL_INFO, nJnlNo, pSrchFrom);
		}
		else
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO < %d AND LOG_SRCH_DATE >= %s AND LOG_SRCH_DATE <= %s"), DB_TABLE_JNL_INFO, nJnlNo, pSrchFrom, pSrchTo);
		}
	}

	else
	{
		return FALSE;
	}


	CString strQuery(m_szSql);

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	*lpRecCnt = (WORD)_sqlite3_column_int(stmt, 0);


	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}


	return TRUE;
}


/************************************************************
*	@brief		Get Search Next Record Count
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetSearchNextRecordCount(BYTE bySrchType, LPSTR pSrchFrom, LPSTR pSrchTo, int nJnlNo, LPDWORD lpRecCnt)
{
	if (!strlen(pSrchFrom))
	{
		return FALSE;
	}


	sqlite3_stmt* stmt;


	if (JNL_NO == bySrchType)
	{
		if (!strlen(pSrchTo))
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO > %d AND JNL_NO >= %s"), DB_TABLE_JNL_INFO, nJnlNo, pSrchFrom);
		}
		else
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO > %d AND JNL_NO >= %s AND JNL_NO <= %s"), DB_TABLE_JNL_INFO, nJnlNo, pSrchFrom, pSrchTo);
		}
	}

	else if (TRAN_SEQ_NO == bySrchType)
	{
		if (!strlen(pSrchTo))
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO > %d AND TRAN_SEQ_NO >= %s"), DB_TABLE_TRANS_INFO, nJnlNo, pSrchFrom);
		}
		else
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO > %d AND TRAN_SEQ_NO >= %s AND TRAN_SEQ_NO <= %s"), DB_TABLE_TRANS_INFO, nJnlNo, pSrchFrom, pSrchTo);
		}
	}

	else if (DATE_RANGE == bySrchType)
	{
		if (!strlen(pSrchTo))
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO > %d AND LOG_SRCH_DATE >= %s"), DB_TABLE_JNL_INFO, nJnlNo, pSrchFrom);
		}
		else
		{
			sprintf(m_szSql, ("SELECT COUNT(*) FROM %s WHERE JNL_NO > %d AND LOG_SRCH_DATE >= %s AND LOG_SRCH_DATE <= %s"), DB_TABLE_JNL_INFO, nJnlNo, pSrchFrom, pSrchTo);
		}
	}

	else
	{
		return FALSE;
	}


	CString strQuery(m_szSql);

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	*lpRecCnt = (WORD)_sqlite3_column_int(stmt, 0);


	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}


	return TRUE;
}


/************************************************************
*	@brief		Get First Journal Number
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetFirstJnlNo(LPINT pJnlNo)
{
	sprintf(m_szSql, ("SELECT JNL_NO FROM %s ORDER BY JNL_NO ASC"), DB_TABLE_JNL_INFO);

	CString strQuery(m_szSql);

	sqlite3_stmt* stmt;

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	*pJnlNo = _sqlite3_column_int(stmt, 0);


	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Last Journal Number
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetLastJnlNo(LPINT pJnlNo)
{
	sprintf(m_szSql, ("SELECT JNL_NO FROM %s ORDER BY JNL_NO DESC"), DB_TABLE_JNL_INFO);

	CString strQuery(m_szSql);

	sqlite3_stmt* stmt;

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	*pJnlNo = _sqlite3_column_int(stmt, 0);


	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Search First Journal Number
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetSearchFirstJnlNo(BYTE bySrchType, LPSTR pSrchFrom, LPINT pJnlNo)
{
	if (!strlen(pSrchFrom))
	{
		return FALSE;
	}


	if (JNL_NO == bySrchType)
	{
		sprintf(m_szSql, ("SELECT JNL_NO FROM %s WHERE JNL_NO >= %s ORDER BY JNL_NO ASC"), DB_TABLE_JNL_INFO, pSrchFrom);
	}

	else if (TRAN_SEQ_NO == bySrchType)
	{
		sprintf(m_szSql, ("SELECT JNL_NO FROM %s WHERE TRAN_SEQ_NO >= %s ORDER BY JNL_NO ASC"), DB_TABLE_TRANS_INFO, pSrchFrom);
	}

	else if (DATE_RANGE == bySrchType)
	{
		sprintf(m_szSql, ("SELECT JNL_NO FROM %s WHERE LOG_SRCH_DATE >= %s ORDER BY JNL_NO ASC"), DB_TABLE_JNL_INFO, pSrchFrom);
	}

	else
	{
		return FALSE;
	}


	CString strQuery(m_szSql);

	sqlite3_stmt* stmt;

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	*pJnlNo = _sqlite3_column_int(stmt, 0);


	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Last Journal Number
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::GetSearchLastJnlNo(BYTE bySrchType, LPSTR pSrchTo, LPINT pJnlNo)
{
	if (!strlen(pSrchTo))
	{
		return FALSE;
	}


	if (JNL_NO == bySrchType)
	{
		sprintf(m_szSql, ("SELECT JNL_NO FROM %s WHERE JNL_NO <= %s ORDER BY JNL_NO DESC"), DB_TABLE_JNL_INFO, pSrchTo);
	}

	else if (TRAN_SEQ_NO == bySrchType)
	{
		sprintf(m_szSql, ("SELECT JNL_NO FROM %s WHERE TRAN_SEQ_NO <= %s ORDER BY JNL_NO DESC"), DB_TABLE_TRANS_INFO, pSrchTo);
	}

	else if (DATE_RANGE == bySrchType)
	{
		sprintf(m_szSql, ("SELECT JNL_NO FROM %s WHERE LOG_SRCH_DATE <= %s ORDER BY JNL_NO DESC"), DB_TABLE_JNL_INFO, pSrchTo);
	}

	else
	{
		return FALSE;
	}


	CString strQuery(m_szSql);

	sqlite3_stmt* stmt;

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	*pJnlNo = _sqlite3_column_int(stmt, 0);


	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Clear Information
*	@retval		Nothing
************************************************************/
void EJLDBCtrl::ClearInfo()
{
	//memset(&m_tran_info,0,sizeof(m_tran_info));
	//memset(&m_cstcl_info,0,sizeof(m_cstcl_info));
	//memset(&m_daycl_info,0,sizeof(m_daycl_info));
	//memset(&m_addbill_info,0,sizeof(m_addbill_info));
	//memset(&m_key_info,0,sizeof(m_key_info));
	//memset(&m_pwd_info,0,sizeof(m_pwd_info));
}


/************************************************************
*	@brief		Read First Record
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::ReadFirstRecord(int nIndex, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlData)
{
	sqlite3_stmt* stmt;

	sprintf(m_szSql, ("SELECT * FROM %s ORDER BY JNL_NO ASC"), DB_TABLE_JNL_INFO);

	CString strQuery(m_szSql);

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	for (int nCnt = 0; nCnt <= nIndex; nCnt++)
	{
		if (SQLITE_ROW != _sqlite3_step(stmt))
		{
			_sqlite3_finalize(stmt);

			return FALSE;
		}
	}


	// Get Common Info
	if (FALSE == GetCommField(lpCommInfo, stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	// Release Statement Object
	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}


	return ReadRecord(lpCommInfo, lpJnlData);
}


/************************************************************
*	@brief		Read Last Record
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::ReadLastRecord(int nIndex, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlData)
{
	sqlite3_stmt* stmt;

	sprintf(m_szSql, ("SELECT * FROM %s ORDER BY JNL_NO DESC"), DB_TABLE_JNL_INFO);

	CString strQuery(m_szSql);

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	for (int nCnt = 0; nCnt <= nIndex; nCnt++)
	{
		if (SQLITE_ROW != _sqlite3_step(stmt))
		{
			_sqlite3_finalize(stmt);

			return FALSE;
		}
	}


	// Get Common Info
	if (FALSE == GetCommField(lpCommInfo, stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	// Release Statement Object
	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}


	return ReadRecord(lpCommInfo, lpJnlData);
}


/************************************************************
*	@brief		Read Current Record
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::ReadCurrentRecord(int nJnlNo, int nIndex, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlData)
{
	sqlite3_stmt* stmt;


	if (nIndex >= 0)
	{
		sprintf(m_szSql, ("SELECT * FROM %s WHERE JNL_NO >= %d ORDER BY JNL_NO ASC"), DB_TABLE_JNL_INFO, nJnlNo);
	}
	else
	{
		sprintf(m_szSql, ("SELECT * FROM %s WHERE JNL_NO <= %d ORDER BY JNL_NO DESC"), DB_TABLE_JNL_INFO, nJnlNo);
	}

	CString strQuery(m_szSql);

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	for (int nCnt = 0; nCnt <= abs(nIndex); nCnt++)
	{
		if (SQLITE_ROW != _sqlite3_step(stmt))
		{
			_sqlite3_finalize(stmt);

			return FALSE;
		}
	}


	// Get Common Info
	if (FALSE == GetCommField(lpCommInfo, stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	// Release Statement Object
	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}


	return ReadRecord(lpCommInfo, lpJnlData);
}


/************************************************************
*	@brief		Search Current Record
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::SearchCurrentRecord(BYTE bySrchType, LPSTR pSrchFrom, LPSTR pSrchTo, int nJnlNo, int nIndex, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlData)
{
	if (!strlen(pSrchFrom))
	{
		return FALSE;
	}

	sqlite3_stmt* stmt;

	if (JNL_NO == bySrchType)
	{
		if (!strlen(pSrchTo))
		{
			if (nIndex >= 0)
			{
				sprintf(m_szSql, ("SELECT * FROM %s WHERE JNL_NO >= %s AND JNL_NO >= %d ORDER BY JNL_NO ASC"), DB_TABLE_JNL_INFO, pSrchFrom, nJnlNo);
			}
			else
			{
				sprintf(m_szSql, ("SELECT * FROM %s WHERE JNL_NO >= %s AND JNL_NO <= %d ORDER BY JNL_NO DESC"), DB_TABLE_JNL_INFO, pSrchFrom, nJnlNo);
			}
		}
		else
		{
			if (nIndex >= 0)
			{
				sprintf(m_szSql, ("SELECT * FROM %s WHERE JNL_NO >= %s AND JNL_NO <= %s AND JNL_NO >= %d ORDER BY JNL_NO ASC"), DB_TABLE_JNL_INFO, pSrchFrom, pSrchTo, nJnlNo);
			}
			else
			{
				sprintf(m_szSql, ("SELECT * FROM %s WHERE JNL_NO >= %s AND JNL_NO <= %s AND JNL_NO <= %d ORDER BY JNL_NO DESC"), DB_TABLE_JNL_INFO, pSrchFrom, pSrchTo, nJnlNo);
			}
		}
	}
	else if (TRAN_SEQ_NO == bySrchType)
	{
		if (!strlen(pSrchTo))
		{
			if (nIndex >= 0)
			{
				sprintf(m_szSql, ("SELECT * FROM %s WHERE TRAN_SEQ_NO => %s AND JNL_NO >= %d ORDER BY JNL_NO ASC"), DB_TABLE_TRANS_INFO, pSrchFrom, nJnlNo);
			}
			else
			{
				sprintf(m_szSql, ("SELECT * FROM %s WHERE TRAN_SEQ_NO >= %s AND JNL_NO <= %d ORDER BY JNL_NO DESC"), DB_TABLE_TRANS_INFO, pSrchFrom, nJnlNo);
			}
		}
		else
		{
			if (nIndex >= 0)
			{
				sprintf(m_szSql, ("SELECT * FROM %s WHERE TRAN_SEQ_NO >= %s AND TRAN_SEQ_NO <= %s AND JNL_NO >= %d ORDER BY JNL_NO ASC"), DB_TABLE_TRANS_INFO, pSrchFrom, pSrchTo, nJnlNo);
			}
			else
			{
				sprintf(m_szSql, ("SELECT * FROM %s WHERE TRAN_SEQ_NO >= %s AND TRAN_SEQ_NO <= %s AND JNL_NO <= %d ORDER BY JNL_NO DESC"), DB_TABLE_TRANS_INFO, pSrchFrom, pSrchTo, nJnlNo);
			}
		}
	}
	else if (DATE_RANGE == bySrchType)
	{
		if (!strlen(pSrchTo))
		{
			if (nIndex >= 0)
			{
				sprintf(m_szSql, ("SELECT * FROM %s WHERE LOG_SRCH_DATE => %s AND JNL_NO >= %d ORDER BY JNL_NO ASC"), DB_TABLE_JNL_INFO, pSrchFrom, nJnlNo);
			}
			else
			{
				sprintf(m_szSql, ("SELECT * FROM %s WHERE LOG_SRCH_DATE >= %s AND JNL_NO <= %d ORDER BY JNL_NO DESC"), DB_TABLE_JNL_INFO, pSrchFrom, nJnlNo);
			}
		}
		else
		{
			if (nIndex >= 0)
			{
				sprintf(m_szSql, ("SELECT * FROM %s WHERE LOG_SRCH_DATE >= %s AND LOG_SRCH_DATE <= %s AND JNL_NO >= %d ORDER BY JNL_NO ASC"), DB_TABLE_JNL_INFO, pSrchFrom, pSrchTo, nJnlNo);
			}
			else
			{
				sprintf(m_szSql, ("SELECT * FROM %s WHERE LOG_SRCH_DATE >= %s AND LOG_SRCH_DATE <= %s AND JNL_NO <= %d ORDER BY JNL_NO DESC"), DB_TABLE_JNL_INFO, pSrchFrom, pSrchTo, nJnlNo);
			}
		}
	}
	else
	{
		return FALSE;
	}


	CString strQuery(m_szSql);

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}

	for (int nCnt = 0; nCnt <= abs(nIndex); nCnt++)
	{
		if (SQLITE_ROW != _sqlite3_step(stmt))
		{
			_sqlite3_finalize(stmt);

			return FALSE;
		}
	}

	int nSearchJnlNo = _sqlite3_column_int(stmt, 0);

	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}


	strQuery.Format(L"SELECT * FROM TBL_JNL_INFO WHERE JNL_NO = %d", nSearchJnlNo);

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}

	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	// Get Common Info
	if (FALSE == GetCommField(lpCommInfo, stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	// Release Statement Object
	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}


	return ReadRecord(lpCommInfo, lpJnlData);
}


/************************************************************
*	@brief		Read Record
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::ReadRecord(LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlData)
{
	BOOL bResult = TRUE;


	sqlite3_stmt* stmt;

	// Get table name
	BYTE jnl_cd = (BYTE)lpCommInfo->jnl_cd;

	const char* pszTableName = NULL;

	if (jnl_cd == transaction || jnl_cd == withdrawal || jnl_cd == balance || jnl_cd == transfer)	// header 세분화
	{
		pszTableName = DB_TABLE_TRANS_INFO;
	}
	else if (jnl_cd == value_change)
	{
		pszTableName = DB_TABLE_CHANGE_INFO;
	}
	else if (jnl_cd == cst_close)
	{
		pszTableName = DB_TABLE_CST_CLOSE_INFO;
	}
	else if (jnl_cd == day_close)
	{
		pszTableName = DB_TABLE_DAY_CLOSE_INFO;
	}
	else if (jnl_cd == add_bill)
	{
		pszTableName = DB_TABLE_ADD_BILL_INFO;
	}
	else if (jnl_cd == text_field || jnl_cd == power_on || jnl_cd == in_service || jnl_cd == out_of_service || jnl_cd == in_supervisor || jnl_cd == reboot)
	{
		pszTableName = DB_TABLE_TEXT_INFO;
	}
	else if (jnl_cd == denom_info)
	{
		pszTableName = DB_TABLE_DENOM_INFO;
	}
	// V1.0.0.2 2021.03.02 LEH - TestDispense 추가
	else if (jnl_cd == test_dispense)
	{
		pszTableName = DB_TABLE_TEST_DISPENSE_INFO;
	}
	else
	{
	//	return FALSE;
		// CommInfo에는 저널이 있는데, 저널코드를 알지 못할 경우 처리
		lpCommInfo->jnl_cd = unknown;
		return TRUE;
	}


	sprintf(m_szSql, ("SELECT * FROM %s WHERE JNL_NO = %d"), pszTableName, lpCommInfo->jnl_no);

	CString strQuery(m_szSql);

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}


	// Get Info
	if (jnl_cd == transaction || jnl_cd == withdrawal || jnl_cd == balance || jnl_cd == transfer)	// header 세분화
	{
		bResult = GetTransField((LPFLSEJTRANINFO)lpJnlData, stmt);
	}
	else if (jnl_cd == value_change)
	{
		bResult = GetPwdField((LPEJCHANGEINFO)lpJnlData, stmt);
	}
	else if (jnl_cd == cst_close)
	{
		bResult = GetCstCloseField((LPFLSEJCSTCLINFO)lpJnlData, stmt);
	}
	else if (jnl_cd == day_close)
	{
		bResult = GetDayCloseField((LPFLSEJDAYCLINFO)lpJnlData, stmt);
	}
	else if (jnl_cd == add_bill)
	{
		bResult = GetAddBillField((LPFLSEJADDBILLINFO)lpJnlData, stmt);
	}
	else if (jnl_cd == text_field || jnl_cd == power_on || jnl_cd == in_service || jnl_cd == out_of_service || jnl_cd == in_supervisor || jnl_cd == reboot)
	{
		bResult = GetTextField((LPEJTEXTINFO)lpJnlData, stmt);
	}
	else if (jnl_cd == denom_info)
	{
		bResult = GetDenomField((LPFLSEJSETDENOMINFO)lpJnlData, stmt);
	}
	// V1.0.0.2 2021.03.02 LEH - TestDispense 추가
	else if (jnl_cd == test_dispense)
	{
		bResult = GetTestDispenseField((LPFLSEJTESTDISPENSE)lpJnlData, stmt);
	}
	else
	{
		bResult = FALSE;
	}


	// Release Statement Object
	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}


	return bResult;
}


/************************************************************
*	@brief		Make Error Code
*	@retval		Nothing
************************************************************/
void EJLDBCtrl::MakeErrCode(BYTE err_id)
{
	sprintf(m_szErrCd, "J0%04d", err_id);
}


/************************************************************
*	@brief		Get Error Code
*	@retval		Nothing
************************************************************/
LPCSTR EJLDBCtrl::GetErrCode(void)
{
	return m_szErrCd;
}


/************************************************************
*	@brief		Delete DB
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::DeleteDB(LPCSTR pszDbFile)
{
	CString strtemp(pszDbFile);

	return DeleteFile(strtemp);
}


/************************************************************
*	@brief		InsertMgrRecord
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::InsertMgrRecord(LPFLSDATAINFO lpDataInfo)
{
	BOOL bResult = TRUE;

	if (lpDataInfo != NULL)
	{
		sprintf(m_szSql, ("INSERT INTO %s VALUES (%d, %d, %d %d)"),
			DB_TABLE_JNL_MGR,
			lpDataInfo->nJnlCnt,
			lpDataInfo->nPrintedJnlIndex,
			lpDataInfo->nUploadedAMSIndex,
			lpDataInfo->nUploadedHOSTIndex
			);

		bResult &= ExecuteSql(m_szSql);
	}

	return bResult;
}


/************************************************************
*	@brief		UpdateMgrRecord
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::UpdateMgrRecord(DWORD dwPropBit, LPFLSDATAINFO lpDataInfo)
{
	BOOL bResult = TRUE;

	if (lpDataInfo != NULL)
	{
		if (dwPropBit & CURRENT_JNL_CNT)
		{
			sprintf(m_szSql, ("UPDATE %s SET TOTAL_JNL_CNT = %d"),
				DB_TABLE_JNL_MGR,
				lpDataInfo->nJnlCnt);

			bResult &= ExecuteSql(m_szSql);
		}

		if (dwPropBit & PRINTED_JNL_CNT)
		{
			sprintf(m_szSql, ("UPDATE %s SET PRINTED_JNL_NO = %d"),
				DB_TABLE_JNL_MGR,
				lpDataInfo->nPrintedJnlIndex);

			bResult &= ExecuteSql(m_szSql);
		}

		if (dwPropBit & UPLOADED_AMS_JNL_CNT)
		{
			sprintf(m_szSql, ("UPDATE %s SET UPLOADED_AMS_JNL_NO = %d"),
				DB_TABLE_JNL_MGR,
				lpDataInfo->nUploadedAMSIndex);

			bResult &= ExecuteSql(m_szSql);
		}

		if (dwPropBit & UPLOADED_HOST_JNL_CNT)
		{
			sprintf(m_szSql, ("UPDATE %s SET UPLOADED_HOST_JNL_NO = %d"),
				DB_TABLE_JNL_MGR,
				lpDataInfo->nUploadedHOSTIndex);

			bResult &= ExecuteSql(m_szSql);
		}
	}

	return bResult;
}


/************************************************************
*	@brief		ReadMgrRecord
*	@retval		Nothing
************************************************************/
BOOL EJLDBCtrl::ReadMgrRecord(LPFLSDATAINFO lpDataInfo)
{
	sqlite3_stmt* stmt;

	sprintf(m_szSql, ("SELECT * FROM %s"), DB_TABLE_JNL_MGR);

	CString strQuery(m_szSql);

	if (SQLITE_OK != _sqlite3_prepare(m_db, strQuery, -1, &stmt, NULL))
	{
		return FALSE;
	}


	if (SQLITE_ROW != _sqlite3_step(stmt))
	{
		_sqlite3_finalize(stmt);

		return FALSE;
	}

	lpDataInfo->nJnlCnt = _sqlite3_column_int(stmt, 0);
	lpDataInfo->nPrintedJnlIndex = _sqlite3_column_int(stmt, 1);
	lpDataInfo->nUploadedAMSIndex = _sqlite3_column_int(stmt, 2);
	lpDataInfo->nUploadedHOSTIndex = _sqlite3_column_int(stmt, 3);

	if (SQLITE_OK != _sqlite3_finalize(stmt))
	{
		return FALSE;
	}


	return TRUE;
}