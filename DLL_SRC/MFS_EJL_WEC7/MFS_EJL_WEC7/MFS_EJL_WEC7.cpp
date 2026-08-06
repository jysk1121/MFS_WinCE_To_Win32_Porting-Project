// MFS_EJL_WEC7.cpp : 해당 DLL의 초기화 루틴을 정의합니다.
//

#include "stdafx.h"
#include "MFS_EJL_WEC7.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEV_EJL_API __declspec(dllexport)	

#include "EJLDBCtrl.h"

#include "ERRDBCtrl.h"


#define DB_FILE1	("MFS_EJL.db")	// Elec Journal DB

#define DB_FILE3	("MFS_ERR.db")	// Error Code DB


EJLDBCtrl *pEJLDBCtrl = NULL;

CERRDBCtrl *pERRDBCtrl = NULL;

static char *unicodeToUtf8(const WCHAR *zWideFilename);
//
//TODO: 이 DLL이 MFC DLL에 대해 동적으로 링크되어 있는 경우
//		MFC로 호출되는 이 DLL에서 내보내지는 모든 함수의
//		시작 부분에 AFX_MANAGE_STATE 매크로가
//		들어 있어야 합니다.
//
//		예:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 일반적인 함수 본문은 여기에 옵니다.
//		}
//
//		이 매크로는 MFC로 호출하기 전에
//		각 함수에 반드시 들어 있어야 합니다.
//		즉, 매크로는 함수의 첫 번째 문이어야 하며 
//		개체 변수의 생성자가 MFC DLL로
//		호출할 수 있으므로 개체 변수가 선언되기 전에
//		나와야 합니다.
//
//		자세한 내용은
//		MFC Technical Note 33 및 58을 참조하십시오.
//


// CMFS_EJL_WEC7App

BEGIN_MESSAGE_MAP(CMFS_EJL_WEC7App, CWinApp)
END_MESSAGE_MAP()


// CMFS_EJL_WEC7App 생성

CMFS_EJL_WEC7App::CMFS_EJL_WEC7App()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CMFS_EJL_WEC7App 개체입니다.

CMFS_EJL_WEC7App theApp;


// CMFS_EJL_WEC7App 초기화

BOOL CMFS_EJL_WEC7App::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}


/*--------------------------------------------------------------------
/* Function : DB Mount & Create & Open
/* Parameter: BOOL - 0:Fail, 1:Success 
---------------------------------------------------------------------*/
DEV_EJL_API BOOL __stdcall EJL_Start(LPSTR pszErrCd)
{
	pEJLDBCtrl = new EJLDBCtrl;
	pERRDBCtrl = new CERRDBCtrl;

	TCHAR path[_MAX_PATH] = {0,};
	CHAR strName[_MAX_PATH] = {0,};
	CString strPath, strDBname;
	
	// 3rd argument is a character count, not a byte count.
	GetModuleFileName(NULL, path, _countof(path));

	strPath = path;
	strPath = strPath.Left(strPath.ReverseFind('\\'));

	// Open Transaction Info DB
	strDBname.Format(_T("%s\\Config\\MFS_EJL.db"), strPath);
	if (FALSE == pEJLDBCtrl->Open(strDBname))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_open);
		strcpy_s(pszErrCd, 6, pEJLDBCtrl->GetErrCode());

		return FALSE;
	}

	// Open Errorcode Info DB
	strDBname.Format(_T("%s\\Config\\MFS_ERR.db"), strPath);
	if (FALSE == pERRDBCtrl->Open(strDBname))
	{
		pERRDBCtrl->MakeErrCode(ssm_jnl_err_db3_open);
		strcpy_s(pszErrCd, 6, pERRDBCtrl->GetErrCode());

		return FALSE;
	}

	return TRUE;
}

/*--------------------------------------------------------------------
/* Function : Close DB
/* Parameter: BOOL - 0:Fail, 1:Success
---------------------------------------------------------------------*/
BOOL WINAPI EJL_Close(LPSTR pszErrCd)
{
	if (FALSE == pEJLDBCtrl->Close())
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_close);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	if (pEJLDBCtrl)
	{
		delete pEJLDBCtrl;
	}
	
	if (FALSE == pERRDBCtrl->Close())
	{
		pERRDBCtrl->MakeErrCode(ssm_jnl_err_db3_close);
		strcpy(pszErrCd, pERRDBCtrl->GetErrCode());
		return FALSE;
	}

	if (pERRDBCtrl)
	{
		delete pERRDBCtrl;
	}

	return TRUE;
}


/*---------------------------------------------------------------------
/* Function : Insert Record
			  Check Alarm Threshold & Journal FULL
			  lpJnlInfo : Indicates to Journal full or alarming signal
---------------------------------------------------------------------*/
DEV_EJL_API BOOL __stdcall EJL_InsertJnl(LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt, LPSTR pszErrCd)
{
	FLSDATAINFO data_info;
	memset(&data_info, 0, sizeof(data_info));

	if (FALSE == pEJLDBCtrl->ReadMgrRecord(&data_info))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_read);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	data_info.nJnlCnt++;

	if (FALSE == pEJLDBCtrl->InsertRecord(lpCommInfo, data_info.nJnlCnt, lpJnlDt))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_insert);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	if (data_info.nJnlCnt >= EJ_MAX_COUNT)
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_full);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Current Journal
*	@retval		Nothing
************************************************************/
DEV_EJL_API BOOL __stdcall EJL_GetCurrJnl(int nJnlNo, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt, LPSTR pszErrCd)
{
	if (FALSE == pEJLDBCtrl->ReadCurrentRecord(nJnlNo, 0, lpCommInfo, lpJnlDt))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_read);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Previous Journal
*	@retval		Nothing
************************************************************/
DEV_EJL_API BOOL __stdcall EJL_GetPrevJnl(int nJnlNo, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt, LPSTR pszErrCd)
{
	if (FALSE == pEJLDBCtrl->ReadCurrentRecord(nJnlNo, -1, lpCommInfo, lpJnlDt))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_read);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Search Current Journal
*	@retval		Nothing
************************************************************/
DEV_EJL_API BOOL __stdcall EJL_GetSearchCurrJnl(BYTE bySrchType, LPSTR pSrchFrom, LPSTR pSrchTo, int nJnlNo, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt, LPSTR pszErrCd)
{
	if (FALSE == pEJLDBCtrl->SearchCurrentRecord(bySrchType, pSrchFrom, pSrchTo, nJnlNo, 0, lpCommInfo, lpJnlDt))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_srch);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get Search Previous Journal
*	@retval		Nothing
************************************************************/
DEV_EJL_API BOOL __stdcall EJL_GetSearchPrevJnl(BYTE bySrchType, LPSTR pSrchFrom, LPSTR pSrchTo, int nJnlNo, LPFLSEJCOMMINFO lpCommInfo, LPVOID lpJnlDt, LPSTR pszErrCd)
{
	if (FALSE == pEJLDBCtrl->SearchCurrentRecord(bySrchType, pSrchFrom, pSrchTo, nJnlNo, -1, lpCommInfo, lpJnlDt))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_srch);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get last journal number
*	@retval		Nothing
************************************************************/
DEV_EJL_API BOOL __stdcall EJL_GetLastJnlNo(LPINT pJnlNo, LPSTR pszErrCd)
{
	if (FALSE == pEJLDBCtrl->GetLastJnlNo(pJnlNo))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_read);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	return TRUE;
}


/************************************************************
*	@brief		Get search last journal number
*	@retval		Nothing
************************************************************/
DEV_EJL_API BOOL __stdcall EJL_GetSearchLastJnlNo(BYTE bySrchType, LPSTR pSrchTo, LPINT pJnlNo, LPSTR pszErrCd)
{
	if (FALSE == pEJLDBCtrl->GetSearchLastJnlNo(bySrchType, pSrchTo, pJnlNo))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_read);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	return TRUE;
}


/*--------------------------------------------------------------------
/* Function : Get DLL Version
return : Major Ver: 0x00, Minor Ver: 0x00
---------------------------------------------------------------------*/
void WINAPI EJL_GetVersion(LPFLSEJVERSION lpEJDLLVer)
{
// V02.00: 2017.11.16
//		- EJL DB update, ERR Code DB Add
//		- MAX Jnl Count = 999,999

	lpEJDLLVer->major = 0x02;
	//lpEJDLLVer->minor = 0x00;
	//lpEJDLLVer->minor = 0x01;
	//lpEJDLLVer->minor = 0x02;
	lpEJDLLVer->minor = 0x03;		// Added Camera Info Field.
}


/*--------------------------------------------------------------------
/* Function : Get Error Info
/* Parameter: BOOL - 0:Fail, 1:Success
/* Date		: 17/11/2017
---------------------------------------------------------------------*/
BOOL WINAPI EJL_GetErrInfo(LPSTR strCode, LPERRCODEINFO lpErrInfo, LPSTR pszErrCd)
{
	if (FALSE == pERRDBCtrl->GetErrorInfo(strCode, lpErrInfo))
	{
		pERRDBCtrl->MakeErrCode(ssm_jnl_err_db3_get_error);
		strcpy(pszErrCd, pERRDBCtrl->GetErrCode());
		return FALSE;
	}

	return TRUE;
}


/*-------------------------------------------------------------------
//added at 08/11/2004
//When Journal Full happens, after uploading to RMS server, delete
// When journal database files broken(j01), 강제로 파일 삭제 기능 추가(v02.01)
--------------------------------------------------------------------*/
DEV_EJL_API BOOL __stdcall EJL_Delete(LPSTR pszErrCd)
{

	// 해당함수 Bug로 인해 Delete는 상위국에서 제어하는 걸로 변경
	return FALSE;

	//TCHAR path[_MAX_PATH] = {0,};
	//CHAR strName[_MAX_PATH] = {0,};
	//CString strPath, strDBname;

	//GetModuleFileName(NULL, path, sizeof(path));

	//strPath = path;
	//strPath = strPath.Left(strPath.ReverseFind('\\'));

	////////////////////////////////////////////////////////////////////////////
	//// Delete EJL DB File
	//if (FALSE == pEJLDBCtrl->Close())
	//{
	//	pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_close);
	//	strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
	//	return FALSE;
	//}

	//strDBname.Format(_T("%s\\Config\\MFS_EJL.db"), strPath);
	//strcpy(strName, unicodeToUtf8(strDBname));
	//if (FALSE == pEJLDBCtrl->DeleteDB(strName))
	//{
	//	pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_delete_db);
	//	strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
	//	return FALSE;
	//}

	////////////////////////////////////////////////////////////////////////////
	//// Re-Open DB
	//strDBname.Format(_T("%s\\Config\\MFS_EJL.db"), strPath);
	//strcpy(strName, unicodeToUtf8(strDBname));
	//if (FALSE == pEJLDBCtrl->Open(strDBname))
	//{
	//	pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_open);
	//	strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
	//	return FALSE;
	//}

	//return TRUE;
}


/************************************************************
*	@brief		Get unaudit journal number
*	@retval		Nothing
************************************************************/
DEV_EJL_API BOOL __stdcall EJL_GetPrintedAuditJnlNo(LPINT pJnlNo, LPSTR pszErrCd)
{
	FLSDATAINFO data_info;
	memset(&data_info, 0, sizeof(data_info));

	if (FALSE == pEJLDBCtrl->ReadMgrRecord(&data_info))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_read);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	*pJnlNo = data_info.nPrintedJnlIndex;

	return TRUE;
}


/************************************************************
*	@brief		Set audit journal number
*	@retval		Nothing
************************************************************/
DEV_EJL_API BOOL __stdcall EJL_SetPrintedAuditJnlNo(int nJnlNo, LPSTR pszErrCd)
{
	FLSDATAINFO data_info;
	memset(&data_info, 0, sizeof(data_info));

	if (FALSE == pEJLDBCtrl->ReadMgrRecord(&data_info))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_read);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	//if ((int)data_info.nPrintedJnlIndex < nJnlNo)
	if ((int)data_info.nPrintedJnlIndex <= nJnlNo)
	{
		data_info.nPrintedJnlIndex = nJnlNo;

		if (FALSE == pEJLDBCtrl->UpdateMgrRecord(PRINTED_JNL_CNT, &data_info))
		{
			pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_insert);
			strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
			return FALSE;
		}
	}

	return TRUE;
}


/************************************************************
*	@brief		Get unaudit journal number
*	@retval		Nothing
************************************************************/
DEV_EJL_API BOOL __stdcall EJL_GetAuditAMSJnlNo(LPINT pJnlNo, LPSTR pszErrCd)
{
	FLSDATAINFO data_info;
	memset(&data_info, 0, sizeof(data_info));

	if (FALSE == pEJLDBCtrl->ReadMgrRecord(&data_info))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_read);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	*pJnlNo = data_info.nUploadedAMSIndex;

	return TRUE;
}


/************************************************************
*	@brief		Set audit journal number
*	@retval		Nothing
************************************************************/
DEV_EJL_API BOOL __stdcall EJL_SetAuditAMSJnlNo(int nUploadedJnlNo, LPSTR pszErrCd)
{
	FLSDATAINFO data_info;
	memset(&data_info, 0, sizeof(data_info));

	if (FALSE == pEJLDBCtrl->ReadMgrRecord(&data_info))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_read);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	if ((int)data_info.nUploadedAMSIndex <= nUploadedJnlNo)
	{
		data_info.nUploadedAMSIndex = nUploadedJnlNo;		

		if (FALSE == pEJLDBCtrl->UpdateMgrRecord(UPLOADED_AMS_JNL_CNT, &data_info))
		{
			pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_insert);
			strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
			return FALSE;
		}
	}

	return TRUE;
}


/************************************************************
*	@brief		Get unaudit journal number
*	@retval		Nothing
************************************************************/
DEV_EJL_API BOOL __stdcall EJL_GetAuditHOSTJnlNo(LPINT pJnlNo, LPSTR pszErrCd)
{
	FLSDATAINFO data_info;
	memset(&data_info, 0, sizeof(data_info));

	if (FALSE == pEJLDBCtrl->ReadMgrRecord(&data_info))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_read);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	*pJnlNo = data_info.nUploadedHOSTIndex;

	return TRUE;
}


/************************************************************
*	@brief		Set audit journal number
*	@retval		Nothing
************************************************************/
DEV_EJL_API BOOL __stdcall EJL_SetAuditHOSTJnlNo(int nUploadedJnlNo, LPSTR pszErrCd)
{
	FLSDATAINFO data_info;
	memset(&data_info, 0, sizeof(data_info));

	if (FALSE == pEJLDBCtrl->ReadMgrRecord(&data_info))
	{
		pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_read);
		strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
		return FALSE;
	}

	if ((int)data_info.nUploadedHOSTIndex <= nUploadedJnlNo)
	{
		data_info.nUploadedHOSTIndex = nUploadedJnlNo;

		if (FALSE == pEJLDBCtrl->UpdateMgrRecord(UPLOADED_HOST_JNL_CNT, &data_info))
		{
			pEJLDBCtrl->MakeErrCode(ssm_jnl_err_db1_insert);
			strcpy(pszErrCd, pEJLDBCtrl->GetErrCode());
			return FALSE;
		}
	}

	return TRUE;
}


/*
** Convert microsoft unicode to UTF-8.  Space to hold the returned string is
** obtained from malloc().
*/
//static char *unicodeToUtf8(const WCHAR *zWideFilename){
//	int nByte = 0;
//	char *zFilename = NULL;
//
//	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, 0, 0, 0, 0);
//	zFilename = (char*)malloc( nByte );
//
//	if( zFilename==0 )
//		return 0;
//
//	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, zFilename, nByte, 0, 0);
//
//	if( nByte == 0 )
//	{
//		free(zFilename);
//		zFilename = 0;
//	}
//
//	return zFilename;
//}