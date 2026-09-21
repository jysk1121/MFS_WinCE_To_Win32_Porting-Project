#include "stdafx.h"
#include "ScreenMgr.h"
#include "resource.h"
#include ".\Scr\ScrCtrl.h"

// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CScrMgr
 FUNCTION NAME: CScrMgr()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CScrMgr::CScrMgr()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrMgr
 FUNCTION NAME: ~CScrMgr()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CScrMgr::~CScrMgr()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrMgr
 FUNCTION NAME: LoadScreen()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 화면을 로드한다.
-------------------------------------------------------------------*/
BOOL CScrMgr::LoadScreen()
{
	//////////////////////////////////////////////////////////////////////////
	//	SCREEN INITIALIZE

	BOOL	bRes = TRUE;

	// make screen file
	CreateFileFromResource(IDR_MASTER, L"\\MasterScreenDesc.dat");
	CreateFileFromResource(IDR_SCREEN, L"\\Update.dat");

	WaitForMilliSecond(100);

	NH_SCR_CONFIG eUpdateScreenConfig = m_SystemConfig.GetScreenConfig(SCR_UPDATE);

	m_pScrCtrl->LoadScreens(eUpdateScreenConfig.strLoadPath,
							eUpdateScreenConfig.strLoadSection,
							eUpdateScreenConfig.nWidth,
							eUpdateScreenConfig.nHeight);

	m_pScrCtrl->GetUserInputCheck(INFINITE);
	m_pScrCtrl->ClearUserInputData();
	m_pScrCtrl->SetActiveScreen();

	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrMgr
 FUNCTION NAME: CreateFileFromResource()
 RETURN TYPE  : 
 PARAMETER    : nResID				:	Resource ID
				strSaveFileName		:	저장할 파일이름.
 DESCRIPTION  : TEXT 리소스를 파일로 변환한다.
-------------------------------------------------------------------*/
void CScrMgr::CreateFileFromResource(int nResID, CString strSaveFileName)
{
	FILE			*Stream;

	DeleteFile(strSaveFileName);

	// 리소스 ID와 타입으로 리소스 정보를 읽어 온다.
	HRSRC	resInfo = FindResource(m_hInstance, MAKEINTRESOURCE(nResID), _T("TXT"));

	HGLOBAL	hRes = LoadResource(m_hInstance, resInfo);

	LPSTR	lpRes = (LPSTR)LockResource(hRes);

	int nSize = SizeofResource(m_hInstance, resInfo);

	if ( (Stream = _wfopen(strSaveFileName, L"w+t")) != NULL)
	{
		fwrite ((char*)lpRes, sizeof(char), nSize, Stream);
		
		fflush(Stream);
		fclose(Stream);
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrMgr
 FUNCTION NAME: GetKeyString()
 RETURN TYPE  :
 PARAMETER    : 
 DESCRIPTION  : 화면으로 부터 입력을 받는다.
-------------------------------------------------------------------*/
CString	CScrMgr::GetKeyString(DWORD dwWaitSec)
{
	CString strCommand, strData;
	DWORD	dwProcTime = 10;
	COleDateTime endTime = COleDateTime::GetCurrentTime();

	if (dwWaitSec == INFINITE)
		dwWaitSec = MAX_DEVSCR_TIME;

	COleDateTimeSpan period(0, 0, 0, dwWaitSec);
	endTime += period;

	while (1)
	{
		if (m_pScrCtrl->GetUserInputCheck(5))
		{
			m_pScrCtrl->GetUserInputData(strCommand, strData);

			break;
		}

		Delay_Msg(dwProcTime);

		if (COleDateTime::GetCurrentTime() > endTime)
		{
			strData = L"TIMEOVER";
			break;
		}
	}

	return strData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrMgr
 FUNCTION NAME: LoadScreen()
 RETURN TYPE  :
 PARAMETER    : 
 DESCRIPTION  : 화면 표시를 준비 한다.
-------------------------------------------------------------------*/
void CScrMgr::PrepareScreen(CString strScreen)
{
	m_pScrCtrl->SetVariable(L"PrevSetScreenNo", strScreen);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrMgr
 FUNCTION NAME: LoadScreen()
 RETURN TYPE  :
 PARAMETER    : 
 DESCRIPTION  : 화면을 표시한다.
-------------------------------------------------------------------*/
void CScrMgr::ShowScreen(CString strScreen)
{
	m_pScrCtrl->SetVariable(L"APCenterMovie", strScreen);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrMgr
 FUNCTION NAME: LoadScreen()
 RETURN TYPE  :
 PARAMETER    : 
 DESCRIPTION  : 현재 화면을 Refresh 한다.
-------------------------------------------------------------------*/
void CScrMgr::UpdateScreen()
{
	m_pScrCtrl->SetVariable(L"APUpdate", L"SCREEN");
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrMgr
 FUNCTION NAME: LoadScreen()
 RETURN TYPE  :
 PARAMETER    : 
 DESCRIPTION  : 화면의 스트링을 제어한다.
-------------------------------------------------------------------*/
void CScrMgr::ShowString(int index, CString strStr)
{
	CString strName;

	strName.Format(L"APValue%d", index);

	m_pScrCtrl->SetVariable(strName, strStr);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrMgr
 FUNCTION NAME: LoadScreen()
 RETURN TYPE  :
 PARAMETER    : 
 DESCRIPTION  : 화면의 버튼을 제어한다.
-------------------------------------------------------------------*/
void CScrMgr::ShowButton(int index, BOOL bOn)
{
	CString strName;

	strName.Format(L"APState%d", index);
	
	m_pScrCtrl->SetVariable(strName, ((bOn) ? L"on" : L"off"));
}
