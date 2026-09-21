// ScreenViewer_WinCE.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "ScreenViewer_WinCE.h"
#include "ScreenViewer_WinCEDlg.h"

//#include "windev.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CScreenViewer_WinCEApp

BEGIN_MESSAGE_MAP(CScreenViewer_WinCEApp, CWinApp)
END_MESSAGE_MAP()

#ifdef _WIN32_WCE
extern "C" BOOL IsAPIReady(DWORD hAPI);
#endif

// CScreenViewer_WinCEApp 생성
CScreenViewer_WinCEApp::CScreenViewer_WinCEApp()
	: CWinApp()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CScreenViewer_WinCEApp 개체입니다.
CScreenViewer_WinCEApp theApp;

// CScreenViewer_WinCEApp 초기화

BOOL CScreenViewer_WinCEApp::InitInstance()
{

	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화 
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("로컬 응용 프로그램 마법사에서 생성된 응용 프로그램"));

#ifdef _WIN32_WCE
	// WinCE kernel wait for shell/GDI/window-manager startup; not needed on Win32.
	while( !IsAPIReady(SH_SHELL | SH_GDI | SH_WMGR))
	{
		Sleep(1000);
		RETAILMSG(1, (L"### must delay one second ###\r\n"));
	}
#endif

	RETAILMSG(1, (L"### API READY DONE\r\n"));

	CScreenViewer_WinCEDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 여기에 [확인]을 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}

	// 대화 상자가 닫혔으므로 응용 프로그램의 메시지 펌프를 시작하지 않고  응용 프로그램을 끝낼 수 있도록 FALSE를
	// 반환합니다.
	return FALSE;
}
