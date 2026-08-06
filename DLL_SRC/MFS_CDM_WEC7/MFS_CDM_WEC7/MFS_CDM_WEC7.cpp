// MFS_CDM_WEC7.cpp : 해당 DLL의 초기화 루틴을 정의합니다.
//

#include "stdafx.h"
#include "MFS_CDM_WEC7.h"

#include "CommCdm.h"

#define MFSCOMMCDMDLL_API __declspec(dllexport)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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


// CMFS_CDM_WEC7App

BEGIN_MESSAGE_MAP(CMFS_CDM_WEC7App, CWinApp)
END_MESSAGE_MAP()


// CMFS_CDM_WEC7App 생성

CMFS_CDM_WEC7App::CMFS_CDM_WEC7App()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CMFS_CDM_WEC7App 개체입니다.

CMFS_CDM_WEC7App theApp;

// CDM Control Class
CCommCdm g_cdm;

// CMFS_CDM_WEC7App 초기화

BOOL CMFS_CDM_WEC7App::InitInstance()
{
	CWinApp::InitInstance();

	CCommCdm::CreateInstance();

	return TRUE;
}

//MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_OpenPort(HWND hWnd, int nPortNum)
MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_OpenPort(HWND hWnd, int nPortNum, int nWriteLogLevel)
{
	return g_cdm.OpenPort(hWnd, nPortNum, nWriteLogLevel);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_ClosePort()
{
	return g_cdm.ClosePort();
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_Reset()
{
	return g_cdm.Reset();
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_CfgStatus(LPCDMCFGSTATUS lpCfgStatus, LPBYTE lpbyErrorCd)
{
	return g_cdm.CfgStatus(lpCfgStatus, lpbyErrorCd);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_Status(LPCDMSTATUS lpStatus)
{
	return g_cdm.Status(lpStatus);
}

//MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_MultiDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult)
MFSCOMMCDMDLL_API int __stdcall MFSCommCDM_MultiDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult, LPBYTE lpbyCEInfo)
{
	return g_cdm.MultiDispense(lpCbxItem, lpResult, lpbyCEInfo);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_LastDispense(LPCDMLASTDISPENSE lpResult, LPBYTE lpbyErrorCd)
{
	return g_cdm.LastDispense(lpResult, lpbyErrorCd);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_Diagnostic(LPCDMDIAGNOSTIC lpResult)
{
	return g_cdm.Diagnostic(lpResult);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_SetBillThickness(LPCDMSETBILLTHICKNESS lpSet, LPBYTE lpbyErrorCd)
{
	return g_cdm.SetBillThickness(lpSet, lpbyErrorCd);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_GetBillThickness(BYTE byCbxNo, LPCDMGETBILLTHICKNESS lpGet)
{
	return g_cdm.GetBillThickness(byCbxNo, lpGet);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_SetBillSize(LPCDMSETBILLSIZE lpSet, LPBYTE lpbyErrorCd)
{
	return g_cdm.SetBillSize(lpSet, lpbyErrorCd);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_GetBillSize(BYTE byCbxNo, LPCDMGETBILLSIZE lpGet)
{ 
	return g_cdm.GetBillSize(byCbxNo, lpGet);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_GetDLLVersion(LPBYTE major_num,LPBYTE minor_num)
{
	return g_cdm.GetDLLversion(major_num,minor_num);	
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_LearnBill(BYTE byCbxNo, BYTE byItemCount, LPCDMLEARN lpResult, LPBYTE lpbyCEInfo)
{
	return g_cdm.Learn(byCbxNo,byItemCount, lpResult, lpbyCEInfo);
}

//MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_TestDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult)
MFSCOMMCDMDLL_API int __stdcall MFSCommCDM_TestDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult, LPBYTE lpbyCEInfo)
{
	return g_cdm.TestiDispense(lpCbxItem, lpResult, lpbyCEInfo);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_CheckFirmware(LPSTR szPath, LPBYTE lpbyErrorCd)
{
	return g_cdm.CheckFirmware(szPath, lpbyErrorCd);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_UpdateFirmware(LPCTSTR szPath, LPBYTE lpbyErrorCd, LPBYTE lpbyCEInfo)
{
	return g_cdm.UpdateFirmware(szPath, lpbyErrorCd, lpbyCEInfo);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_GetRejectLog(LPCDM_REJECT_INFO lpRejectLog, LPBYTE lpbyErrorCd)
{
	return g_cdm.GetRejectLog(lpRejectLog, lpbyErrorCd);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_GetDispenseLog(LPCDMRECVDATA lpRecvData, LPBYTE lpbyErrorCd)
{
	return g_cdm.GetDispenseLog(lpRecvData, lpbyErrorCd);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_CEInfoCertification(LPBYTE lpbyCEInfo, LPBYTE lpbyErrorCd)
{
	return g_cdm.CEInfoCertification(lpbyCEInfo, lpbyErrorCd);
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_IsSendDispense()
{
	return g_cdm.IsSendDispense();
}

MFSCOMMCDMDLL_API BOOL __stdcall MFSCommCDM_GetExistCEInfo(LPBYTE lpbyCEInfo, LPBYTE lpbyErrorCd, BOOL& bExist)
{
	return g_cdm.GetExistCEInfo(lpbyCEInfo, lpbyErrorCd, bExist);
}