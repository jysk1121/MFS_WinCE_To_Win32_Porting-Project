// EagleCE_ATM.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "EagleCE_ATM.h"
#include "EagleCE_ATMDlg.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"

#ifdef _WIN32_WCE
#include "../../Template/pwindbas.h"
#endif // _WIN32_WCE

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#ifdef _WIN32_WCE
extern "C" BOOL IsAPIReady(DWORD hAPI);
#endif

// CEagleCE_ATMApp

BEGIN_MESSAGE_MAP(CEagleCE_ATMApp, CWinApp)
END_MESSAGE_MAP()


// CEagleCE_ATMApp 생성
CEagleCE_ATMApp::CEagleCE_ATMApp()
	: CWinApp()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CEagleCE_ATMApp 개체입니다.
CEagleCE_ATMApp theApp;

// CEagleCE_ATMApp 초기화

BOOL CEagleCE_ATMApp::InitInstance()
{

#ifdef _WIN32_WCE
	// IsAPIReady()/SH_SHELL/SH_GDI/SH_WMGR are WinCE kernel APIs used to wait until
	// the shell, GDI and window manager subsystems have finished starting. On Win32
	// those subsystems are already up before WinMain runs, so the wait is skipped.
	while( !IsAPIReady(SH_SHELL | SH_GDI | SH_WMGR))
	{
		Sleep(1000);
		RETAILMSG(1, (_T("### must delay one second ###\r\n")));
	}
#endif

	RETAILMSG(1, (_T("### API READY DONE\r\n")));

	CString strTemp;
	strTemp.Format(_T("### Software Update Type is : %s\r\n"), CUtil::GetSWUpdateType());
	RETAILMSG(1, (strTemp));

	if (CUtil::GetSWUpdateType() == UPDATE_KIND_OS_STRING)
	{
		RETAILMSG(1, (_T("Execute OS Update...\r\n")));

		// CHECK USB Memory Stick and Software Update File
		if (CUtil::IsExistFile(EXE_SWUPDATE_EAGLECE) == TRUE)
		{
			// Execute Terminate_EagleCE
			STARTUPINFO si;
			ZeroMemory (&si, sizeof(si));
			si.cb = sizeof (si);
			si.wShowWindow = SW_HIDE;
			PROCESS_INFORMATION pi;
			ZeroMemory (&pi, sizeof(pi));

			CString strPath;
			strPath.Format(_T("%s%s"), CUtil::GetAppPath(), EXE_TERMINATE_EAGLECE);

			CreateProcess(strPath, NULL,NULL,NULL,NULL,FALSE,0,NULL,&si, &pi);

			CUtil::Sleep_Wait(2000);		// 2초 후에 EagleCE ATM Exit
			return FALSE;
		}
		else
		{
			RETAILMSG(1, (_T("Can not Find Software Update Software from USB\r\n")));			
		}

		// 그렇지 않은 경우 EagleCE ATM 기동
	}

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화 
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.

	SetRegistryKey(_T("EagleCE_ATM"));

	CEagleCE_ATMDlg dlg;
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
