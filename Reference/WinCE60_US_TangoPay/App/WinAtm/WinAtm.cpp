// WinAtm.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include <fstream>

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1

#include "WinAtm.h"

#include "MainFrm.h"

#include "WinAtmDoc.h"
#include "WinAtmView.h"

//#include ".\Common\MB2500DLL.h"	// 2008-01-14 V01.02.25 SRC-9			// [#573] NH AIREAT 2009.10.26 SW NVRAM
#include ".\Common\CmnLib.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CWinAtmApp

BEGIN_MESSAGE_MAP(CWinAtmApp, CWinApp)
	//{{AFX_MSG_MAP(CWinAtmApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinAtmApp construction

CWinAtmApp::CWinAtmApp()
	: CWinApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWinAtmApp object

CWinAtmApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWinAtmApp initialization
HWND				g_hWndDisplay;			// The command bar handle
HFONT				g_hLocalFont = NULL;
HANDLE				ghEvent=NULL;		// never signaled.

LRESULT CALLBACK	InitWinAtmWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

RECT				g_DrawRect;

BOOL CWinAtmApp::InitInstance()
{
#ifdef _DEBUG
	_CrtSetReportMode(_CRT_ASSERT,0);
#endif

	NHDEBUG(DBG_CALL, (_T("WinATM Start!\n")));

	// [#2394] NH KSK 2016.02.05 OS Background Image 변경시 WinAtm Exception 발생하는 오류 수정
	// Change Background Image (Splash -> Out of service)
	RETAILMSG(1, (L"CHANGE BACKGROUND IMAGE START\n"));
	{
		LONG	lResult;
		HKEY	hKeyResult;
		DWORD	dwValue = 2;	// from loader

		lResult = RegOpenKeyEx(HKEY_CURRENT_USER, L"ControlPanel\\Desktop", 0, KEY_ALL_ACCESS, &hKeyResult);
		if (lResult == ERROR_SUCCESS)
		{
			TCHAR szNewBg[256];
			memset(szNewBg, 0, sizeof(szNewBg));

			_tcscpy(szNewBg, L"\\Windows\\WindowsCE.jpg");

			RegSetValueEx(hKeyResult, L"Wallpaper", 0, REG_SZ, (CONST BYTE*)szNewBg, (sizeof(TCHAR)*(_tcsclen(szNewBg)+1)));

			RegCloseKey(hKeyResult);
		}

		// Update
		::PostMessage(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETDESKWALLPAPER, 0);
	}
	RETAILMSG(1, (L"CHANGE BACKGROUND IMAGE END\n"));
	// end of [#2394]

	//이미 실행중이면 바로 리턴해버린다.
	if(FindWindow(NULL, L"WinATM"))
		return FALSE;

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

#if (_WIN32_WCE < 0x600)
	LoadStdProfileSettings();  // Load standard INI file options (including MRU)
#endif


	// ============================================================
	// shows dummy screen
	{
		NHDEBUG(DBG_CALL, (_T("Creating Dummy Screen...\n")));

		WNDCLASS	wc = {};
		TCHAR		tszClassName[] = TEXT("__NH_INIT_WINATM_WND__");

		// Registry ClassInfo
		wc.style = 0;
		wc.lpfnWndProc = (WNDPROC) InitWinAtmWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = (HINSTANCE) m_hInstance;
		wc.hIcon = 0;
		wc.hCursor = 0;
		wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = 0;
		wc.lpszClassName = tszClassName;

#define GET_FONTSIZE(PointSize) (-::MulDiv((PointSize), GetDeviceCaps(NULL, LOGPIXELSY), 72))
		LOGFONT	lf;
		memset(&lf, 0, sizeof(lf));
		wcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("Arial"));
		lf.lfHeight = GET_FONTSIZE(24);
		lf.lfQuality = CLEARTYPE_QUALITY;
		lf.lfWeight = FW_BOLD;
		g_hLocalFont = ::CreateFontIndirect(&lf);

		RegisterClass(&wc);

		NH_SCR_CONFIG eFrontScreen = GetConfigFuncPointer()->GetScreenConfig(SCR_FRONT);
		g_DrawRect.left = 0;
		g_DrawRect.top = eFrontScreen.nHeight / 2;
		g_DrawRect.right = eFrontScreen.nWidth;
		g_DrawRect.bottom = eFrontScreen.nHeight;

		ghEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

		// create Client Wnd
		g_hWndDisplay = ::CreateWindowEx(NULL,				/* dwExStyle */
			tszClassName,				/* lpClassName */
			NULL,						/* lpWindowName */
			WS_POPUP,					/* dwStyle */
			eFrontScreen.nX,			/* x */
			eFrontScreen.nY,			/* y */
			eFrontScreen.nWidth,		/* nWidth */
			eFrontScreen.nHeight,		/* nHeight */
			NULL,						/* hWndParent */
			NULL,						/* hMenu */
			m_hInstance,				/* hInstance */
			NULL);						/* lpParam */

		// Hide Window - don't initially visible
		ShowWindow(g_hWndDisplay, SW_SHOW);
		UpdateWindow(g_hWndDisplay);

		// [#GLDV-2683] NH Kook 2019.11.25 Increase timer duration (90s -> 240s (+150s) to cover PCI 5.0 EPP firmware updating duration.
		// Kill this dummy window after 90 sec, when extract is finished and devices are being initialized.
// 		SetTimer(g_hWndDisplay, 90000, 90000, NULL);
		SetTimer(g_hWndDisplay, 90000, 240000, NULL);
	}

	// install images before RAMDISK is allocated
	// to avoid DATA ABORT exception which is resolvable by reboot.
	NHDEBUG(DBG_CALL, (_T("Installing Essential Files...\n")));
	InstallNextwareRegistries();
	InstallWinAtmEssentials();
	InstallImageResources();

	NHDEBUG(DBG_CALL, (_T("Installing Customized Files...\n")));
	InstallCustomerRcptHeader();
	InstallCustomerBacks();
	InstallCustomerAds();
	InstallExtendedBinList();
	InstallTTS_DB();				// [#2558] NH Justin 2018.06.12 Fix fof S/W Update Failure after "Format and Update"
	CNetWork::InstallCAFiles();

	NHDEBUG(DBG_CALL, (_T("Preinstallation Complete!!\n")));
	// ============================================================


// 	//////////////////////////////////////////////////////////////////////////
// 	//	MOVE OBJECT STORE SIZE FOR UPDATE FILES.
// 	{
// 		DWORD dwStorePages, dwRamPages, dwPageSize;
// 		if(GetSystemMemoryDivision(&dwStorePages, &dwRamPages, &dwPageSize) == TRUE)
// 		{
// 			DWORD storePage = (32768000 / dwPageSize);	// 32M
// 			//DWORD storePage = (50331648 / dwPageSize);	// 48M
// 			if (SetSystemMemoryDivision(storePage) != SYSMEM_CHANGED)
// 			{
// 				NHERROR((_T("[WINATM] Failed to SetSystemMemoryDivision()\n")));
// 			}
// 		}
// 		else
// 		{
// 			NHERROR((_T("[WINATM] Failed to GetSystemMemoryDivision()\n")));
// 		}
// 	}

	// [#2378] US Justin 2015.11.13 Multi-Step Update (TTS Engine is too big to expand with one trial)
	RETAILMSG(1, (L"Check UPDATE2/patch.zip for Multi Step UPdate\n"));
	if(IsExistFile( L"\\ATM\\Update2\\patch.zip") )
	{
		BOOL bMultiStepUpdate = FALSE;		
		RETAILMSG(1, (L"  ===> Found [ATM/UPDATE2/patch.zip]\n"));

		// Copy patch.zip to RMS Download Folder - "\\Update\\NH2700CE"
		CreateDirectory(_T("Update"), NULL);
		CreateDirectory(_T("Update\\NH2700CE"), NULL);

		if (IsExistFile(_T("Update\\NH2700CE")) == TRUE)				// TRUE if the folder is created correctly
		{
			RETAILMSG(1, (L"  ===> Direcvtory Created [UPDATE/NH2700CE]\n"));
			// File Copy : \ATM\Update2\patch.zip -> \Update\NH2700CE\patch.zip
			if (CopyFileInDirectory( _T("\\ATM\\Update2"), _T("patch.zip"), _T("Update\\NH2700CE") ) == TRUE)
			{
				RETAILMSG(1, (L"  ===> File Copied [ATM/Update2/patch.zip] => [Update/NH2700CE/patch.zip] ====> Update.exe\n"));
				bMultiStepUpdate = TRUE;
			}
		}

		// Remove Original file (UPDATE2/patch.zip) to prevent infinite Update/rebooting.
		if (DeleteFileInDirectory(_T("\\ATM\\Update2"), _T("*.*")) == TRUE)
		{
			RemoveDirectory(_T("\\ATM\\Update2"));
			RETAILMSG(1, (L"  ===> Original file and folder [ATM/UPDATE2/patch.zip] are removed\n"));
		}

		// Call Update.exe
		if(bMultiStepUpdate)
		{
			RETAILMSG(1, (L"  ===> Call Check UpdateLauncher\n"));

			CString strFileName = L"\\ATM\\UpdateLauncher.exe";
			HANDLE hFile = CreateFile((LPCTSTR)strFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hFile);
				CString strCmd = L"\\ATM\\UpdateLauncher.exe";
				STARTUPINFO si;
				ZeroMemory (&si, sizeof(si));
				si.cb = sizeof (si);
				si.wShowWindow = SW_HIDE;
				PROCESS_INFORMATION pi;				

				RegSetInt(L"SOFTWARE\\ATM\\APP", L"RMSFileUpdate", 3);			// let update.exe know this is for the second step update

				RETAILMSG(1, (L"  ===> Call UpdateLauncher and close\n"));
				CreateProcess(strCmd,NULL,NULL,NULL,NULL,FALSE,0,NULL,&si, &pi);

				Delay_Msg(5000);
				return FALSE;
			}
			// Continue ATM Process even "SW update (launching update.exe)" is failed.....
		}
	}
	// End of [#2378]

	RETAILMSG(1, (L"TEMPLATE CREATE\n"));
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CWinAtmDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CWinAtmView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	//메뉴를 없앤다.
	m_pMainWnd->SetMenu(NULL);

	m_pMainWnd->SetForegroundWindow();
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	// [#6] NH PSC 2008.03.24 AtmStart() 실행위치 이동 CMainFrm::OnTimer() -> CWinAtmApp::InitInstance()
	//Delay_Msg(1000);

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if(pFrame != NULL)
	{
		//pFrame->KillProcByName("OalInit.exe");						//1. AP loader Oalinit.exe를 닫는다.

		// // [#2050] NH KSK 2011.04.18 WatchDoc Execute
		HWND hRearWnd = ::FindWindow(NULL, L"WatchDog");
		if (!hRearWnd)
		{
			STARTUPINFO si;
			ZeroMemory (&si, sizeof(si));
			si.cb = sizeof (si);
			PROCESS_INFORMATION pi;
			CreateProcess(L"\\ATM\\WatchDog.exe",NULL,NULL,NULL,NULL,FALSE,0,NULL,&si, &pi);
		}			
		// end of [#2050]

		// [#2402] CA Justin 2016.03.11 Move Local Loan download file to ATM2/data folder
		CreateDirectory(ATM2_DATA_PATH, NULL);

		if (InstallAsianFonts() == true)
			pFrame->bNeedRebootOnInit = TRUE;		// [#2186] US KSK 2013.05.06

		pFrame->AtmStart();
	}
	else
	{
		AfxMessageBox(L"Main Frame is not created!!");
	}
	// end of [#6]

	return TRUE;
}

/**
* Installs additional Nextware registries that OS image doesn't contain by default due to later development.
* @returns true, always
*/
bool CWinAtmApp::InstallNextwareRegistries()
{
#ifndef UNDER_CE
	return true;
#endif

	RETAILMSG(1, (L"Installing Nextware Registries... \n"));

	// [#2325] NH KSK 2015.01.21 RFID 지원을 위한 Registry 설정
	// Registering the CLSID
	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}", L"", L"NXCardReaderX Control", 2);
	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\ProgID", L"", L"NXCardReaderX.NXRFIDReaderXCtrl.1", 2);
	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\InprocServer32", L"", L"\\ATM\\NXRFIDReader30.ocx", 2);
	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\InprocServer32", L"ThreadingModel", L"Apartment", 2);
	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\Control", L"", L"", 2);
	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\Implemented Categories", L"", L"", 2);
	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\Implemented Categories\\{7DD95801-9882-11CF-9FA9-00AA006C42C4}", L"@", L"", 2);
	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\MiscStatus", L"", L"0", 2);
	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\MiscStatus\\1", L"", L"131217", 2);
	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\ToolboxBitmap32", L"", L"\\ATM\\NXRFIDReader30.ocx, 1", 2);
	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\TypeLib", L"", L"{E686457D-5350-4AE1-9B4A-3B80BDAB7EED}", 2);
	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\Version", L"", L"1.0", 2);
	// end of [#2325]

	// [#GLDV-3005] US Kook 2021.11.08 Support Side Car
	// BCR
	RegSetStr(L"NXSCANNERPRINTER.NXScannerPrinterXCtrl.1", L"", L"NXScannerPrinterX Control", 2);
	RegSetStr(L"NXSCANNERPRINTER.NXScannerPrinterXCtrl.1\\CLSID", L"", L"{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}", 2);

	RegSetStr(L"CLSID\\{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}", L"", L"NXScannerPrinterX Control", 2);
	RegSetStr(L"CLSID\\{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}\\ProgID", L"", L"NXSCANNERPRINTERX.NXScannerPrinterXCtrl.1", 2);
	RegSetStr(L"CLSID\\{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}\\InprocServer32", L"", L"\\ATM\\NXScannerPrinter30.ocx", 2);
	RegSetStr(L"CLSID\\{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}\\InprocServer32", L"ThreadingModel", L"Apartment", 2);
	RegSetStr(L"CLSID\\{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}\\Control", L"", L"", 2);
	RegSetStr(L"CLSID\\{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}\\Implemented Categories", L"", L"", 2);
	RegSetStr(L"CLSID\\{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}\\Implemented Categories\\{7DD95801-9882-11CF-9FA9-00AA006C42C4}", L"@", L"", 2);
	RegSetStr(L"CLSID\\{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}\\MiscStatus", L"", L"0", 2);
	RegSetStr(L"CLSID\\{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}\\MiscStatus\\1", L"", L"131217", 2);
	RegSetStr(L"CLSID\\{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}\\ToolboxBitmap32", L"", L"\\ATM\\NxScannerPrinter30.ocx, 1", 2);
	RegSetStr(L"CLSID\\{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}\\TypeLib", L"", L"{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}", 2);
	RegSetStr(L"CLSID\\{3D6834E5-C6C8-4474-95DF-D836BCCCB1F2}\\Version", L"", L"1.0", 2);

	// BNA
	RegSetStr(L"CLSID\\{449BCF97-82D1-4BE2-BE54-2E95CCFCE737}", L"", L"NXCashAcceptorX Control", 2);
	RegSetStr(L"CLSID\\{449BCF97-82D1-4BE2-BE54-2E95CCFCE737}\\Control", L"", L"", 2);
	RegSetStr(L"CLSID\\{449BCF97-82D1-4BE2-BE54-2E95CCFCE737}\\InprocServer32", L"", L"\\ATM\\NXCashAcceptor30.ocx", 2);
	RegSetStr(L"CLSID\\{449BCF97-82D1-4BE2-BE54-2E95CCFCE737}\\InprocServer32", L"ThreadingModel", L"Apartment", 2);
	RegSetStr(L"CLSID\\{449BCF97-82D1-4BE2-BE54-2E95CCFCE737}\\MiscStatus", L"", L"0", 2);
	RegSetStr(L"CLSID\\{449BCF97-82D1-4BE2-BE54-2E95CCFCE737}\\MiscStatus\\1", L"", L"131217", 2);
	RegSetStr(L"CLSID\\{449BCF97-82D1-4BE2-BE54-2E95CCFCE737}\\ProgID", L"", L"NXCashAcceptorX.NXCashAcceptorXCtrl.1", 2);
	RegSetStr(L"CLSID\\{449BCF97-82D1-4BE2-BE54-2E95CCFCE737}\\ToolboxBitmap32", L"", L"\\ATM\\NXCashAcceptor30.ocx, 1", 2);
	RegSetStr(L"CLSID\\{449BCF97-82D1-4BE2-BE54-2E95CCFCE737}\\TypeLib", L"", L"{4B00B44F-51E5-403F-BFAD-29AA0AB9DCFD}", 2);
	RegSetStr(L"CLSID\\{449BCF97-82D1-4BE2-BE54-2E95CCFCE737}\\Version", L"", L"1.0", 2);

	// BNA, additional things
	RegSetStr(L"CLSID\\{832AD5D7-6AC0-4C0F-9956-E662BF35CE4D}", L"", L"CashInStatus Class", 2);
	RegSetStr(L"CLSID\\{832AD5D7-6AC0-4C0F-9956-E662BF35CE4D}\\InprocServer32", L"", L"\\ATM\\NXCashAcceptor30.ocx", 2);
	RegSetStr(L"CLSID\\{832AD5D7-6AC0-4C0F-9956-E662BF35CE4D}\\InprocServer32", L"ThreadingModel", L"Apartment", 2);
	RegSetStr(L"CLSID\\{832AD5D7-6AC0-4C0F-9956-E662BF35CE4D}\\ProgID", L"", L"NXCashAcceptorX.CashInStatus.1", 2);
	RegSetStr(L"CLSID\\{832AD5D7-6AC0-4C0F-9956-E662BF35CE4D}\\TypeLib", L"", L"{4B00B44F-51E5-403F-BFAD-29AA0AB9DCFD}", 2);
	RegSetStr(L"CLSID\\{832AD5D7-6AC0-4C0F-9956-E662BF35CE4D}\\VersionIndependentProgID", L"", L"NXCashAcceptorX.CashInStatus", 2);

	RegSetStr(L"CLSID\\{361F334D-6F5B-423F-A6EF-A9EAD4E8C133}", L"", L"ItemType Class", 2);
	RegSetStr(L"CLSID\\{361F334D-6F5B-423F-A6EF-A9EAD4E8C133}\\InprocServer32", L"", L"\\ATM\\NXCashAcceptor30.ocx", 2);
	RegSetStr(L"CLSID\\{361F334D-6F5B-423F-A6EF-A9EAD4E8C133}\\InprocServer32", L"ThreadingModel", L"Apartment", 2);
	RegSetStr(L"CLSID\\{361F334D-6F5B-423F-A6EF-A9EAD4E8C133}\\ProgID", L"", L"NXCashAcceptorX.ItemType.1", 2);
	RegSetStr(L"CLSID\\{361F334D-6F5B-423F-A6EF-A9EAD4E8C133}\\TypeLib", L"", L"{4B00B44F-51E5-403F-BFAD-29AA0AB9DCFD}", 2);
	RegSetStr(L"CLSID\\{361F334D-6F5B-423F-A6EF-A9EAD4E8C133}\\VersionIndependentProgID", L"", L"NXCashAcceptorX.ItemType", 2);

	RegSetStr(L"CLSID\\{CF220809-0530-4656-8845-7EE79F029674}", L"", L"LogicalUnit Class", 2);
	RegSetStr(L"CLSID\\{CF220809-0530-4656-8845-7EE79F029674}\\InprocServer32", L"", L"\\ATM\\NXCashAcceptor30.ocx", 2);
	RegSetStr(L"CLSID\\{CF220809-0530-4656-8845-7EE79F029674}\\InprocServer32", L"ThreadingModel", L"Apartment", 2);
	RegSetStr(L"CLSID\\{CF220809-0530-4656-8845-7EE79F029674}\\ProgID", L"", L"NXCashAcceptorX.LogicalUnit.1", 2);
	RegSetStr(L"CLSID\\{CF220809-0530-4656-8845-7EE79F029674}\\TypeLib", L"", L"{4B00B44F-51E5-403F-BFAD-29AA0AB9DCFD}", 2);
	RegSetStr(L"CLSID\\{CF220809-0530-4656-8845-7EE79F029674}\\VersionIndependentProgID", L"", L"NXCashAcceptorX.LogicalUnit", 2);

	RegSetStr(L"CLSID\\{D3A07BB6-76C5-4B99-AA4F-6828C8807819}", L"", L"P6Info Class", 2);
	RegSetStr(L"CLSID\\{D3A07BB6-76C5-4B99-AA4F-6828C8807819}\\InprocServer32", L"", L"\\ATM\\NXCashAcceptor30.ocx", 2);
	RegSetStr(L"CLSID\\{D3A07BB6-76C5-4B99-AA4F-6828C8807819}\\InprocServer32", L"ThreadingModel", L"Apartment", 2);
	RegSetStr(L"CLSID\\{D3A07BB6-76C5-4B99-AA4F-6828C8807819}\\ProgID", L"", L"NXCashAcceptorX.P6Info.1", 2);
	RegSetStr(L"CLSID\\{D3A07BB6-76C5-4B99-AA4F-6828C8807819}\\TypeLib", L"", L"{4B00B44F-51E5-403F-BFAD-29AA0AB9DCFD}", 2);
	RegSetStr(L"CLSID\\{D3A07BB6-76C5-4B99-AA4F-6828C8807819}\\VersionIndependentProgID", L"", L"NXCashAcceptorX.P6Info", 2);

	RegSetStr(L"CLSID\\{DBA92754-1E86-4ACE-A0C0-DD45D6F78339}", L"", L"Positions Class", 2);
	RegSetStr(L"CLSID\\{DBA92754-1E86-4ACE-A0C0-DD45D6F78339}\\InprocServer32", L"", L"\\ATM\\NXCashAcceptor30.ocx", 2);
	RegSetStr(L"CLSID\\{DBA92754-1E86-4ACE-A0C0-DD45D6F78339}\\InprocServer32", L"ThreadingModel", L"Apartment", 2);
	RegSetStr(L"CLSID\\{DBA92754-1E86-4ACE-A0C0-DD45D6F78339}\\ProgID", L"", L"NXCashAcceptorX.Positions.1", 2);
	RegSetStr(L"CLSID\\{DBA92754-1E86-4ACE-A0C0-DD45D6F78339}\\TypeLib", L"", L"{4B00B44F-51E5-403F-BFAD-29AA0AB9DCFD}", 2);
	RegSetStr(L"CLSID\\{DBA92754-1E86-4ACE-A0C0-DD45D6F78339}\\VersionIndependentProgID", L"", L"NXCashAcceptorX.Positions", 2);

	//RegSetStr(L"CLSID\\{F9D068CE-5DA4-4241-8DC0-384B190A20B9}", L"", L"NXCashAcceptorX Property Page", 2);
	//RegSetStr(L"CLSID\\{F9D068CE-5DA4-4241-8DC0-384B190A20B9}\\InprocServer32", L"", L"\\ATM\\NXCashAcceptor30.ocx", 2);

	RegSetStr(L"TypeLib\\{4B00B44F-51E5-403F-BFAD-29AA0AB9DCFD}", L"", L"", 2);
	RegSetStr(L"TypeLib\\{4B00B44F-51E5-403F-BFAD-29AA0AB9DCFD}\\1.0", L"", L"", 2);
	RegSetStr(L"TypeLib\\{4B00B44F-51E5-403F-BFAD-29AA0AB9DCFD}\\1.0\\0", L"", L"\\ATM\\NXCashAcceptor30.ocx", 2);
	RegSetStr(L"TypeLib\\{4B00B44F-51E5-403F-BFAD-29AA0AB9DCFD}\\1.0\\0\\win32", L"", L"\\ATM\\NXCashAcceptor30.ocx", 2);

	RegSetStr(L"NXCashAcceptorX.NXCashAcceptorXCtrl.1", L"", L"NXCashAcceptorX Control", 2);
	RegSetStr(L"NXCashAcceptorX.NXCashAcceptorXCtrl.1\\CLSID", L"", L"{449BCF97-82D1-4BE2-BE54-2E95CCFCE737}", 2);

	RegSetStr(L"NXCashAcceptorX.CashInStatus", L"", L"CashInStatus Class", 2);
	RegSetStr(L"NXCashAcceptorX.CashInStatus\\CLSID", L"", L"{832AD5D7-6AC0-4C0F-9956-E662BF35CE4D}", 2);
	RegSetStr(L"NXCashAcceptorX.CashInStatus\\CurVer", L"", L"NXCashAccptorX.CashInStatus.1", 2);
	RegSetStr(L"NXCashAcceptorX.CashInStatus.1", L"", L"CashInStatus Class", 2);
	RegSetStr(L"NXCashAcceptorX.CashInStatus.1\\CLSID", L"", L"{832AD5D7-6AC0-4C0F-9956-E662BF35CE4D}", 2);

	RegSetStr(L"NXCashAcceptorX.ItemType", L"", L"ItemType Class", 2);
	RegSetStr(L"NXCashAcceptorX.ItemType\\CLSID", L"", L"{361F334D-6F5B-423F-A6EF-A9EAD4E8C133}", 2);
	RegSetStr(L"NXCashAcceptorX.ItemType\\CurVer", L"", L"NXCashAccptorX.ItemType.1", 2);
	RegSetStr(L"NXCashAcceptorX.ItemType.1", L"", L"ItemType Class", 2);
	RegSetStr(L"NXCashAcceptorX.ItemType.1\\CLSID", L"", L"{361F334D-6F5B-423F-A6EF-A9EAD4E8C133}", 2);

	RegSetStr(L"NXCashAcceptorX.LogicalUnit", L"", L"LogicalUnit Class", 2);
	RegSetStr(L"NXCashAcceptorX.LogicalUnit\\CLSID", L"", L"{CF220809-0530-4656-8845-7EE79F029674}", 2);
	RegSetStr(L"NXCashAcceptorX.LogicalUnit\\CurVer", L"", L"NXCashAccptorX.LogicalUnit.1", 2);
	RegSetStr(L"NXCashAcceptorX.LogicalUnit.1", L"", L"LogicalUnit Class", 2);
	RegSetStr(L"NXCashAcceptorX.LogicalUnit.1\\CLSID", L"", L"{CF220809-0530-4656-8845-7EE79F029674}", 2);

	RegSetStr(L"NXCashAcceptorX.P6Info", L"", L"P6Info Class", 2);
	RegSetStr(L"NXCashAcceptorX.P6Info\\CLSID", L"", L"{D3A07BB6-76C5-4B99-AA4F-6828C8807819}", 2);
	RegSetStr(L"NXCashAcceptorX.P6Info\\CurVer", L"", L"NXCashAccptorX.P6Info.1", 2);
	RegSetStr(L"NXCashAcceptorX.P6Info.1", L"", L"P6Info Class", 2);
	RegSetStr(L"NXCashAcceptorX.P6Info.1\\CLSID", L"", L"{D3A07BB6-76C5-4B99-AA4F-6828C8807819}", 2);

	RegSetStr(L"NXCashAcceptorX.Positions", L"", L"Positions Class", 2);
	RegSetStr(L"NXCashAcceptorX.Positions\\CLSID", L"", L"{DBA92754-1E86-4ACE-A0C0-DD45D6F78339}", 2);
	RegSetStr(L"NXCashAcceptorX.Positions\\CurVer", L"", L"NXCashAccptorX.Positions.1", 2);
	RegSetStr(L"NXCashAcceptorX.Positions.1", L"", L"Positions Class", 2);
	RegSetStr(L"NXCashAcceptorX.Positions.1\\CLSID", L"", L"{DBA92754-1E86-4ACE-A0C0-DD45D6F78339}", 2);

	// additional flickers
	// BNA
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\SIU\\CAPS", L"CAP_GUID_BILLACCEPTOR", L"YES");
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\SIU\\CONF", L"GUID_BILLACCEPTOR", SIDECAR_BNA_FLICKER);

	// COD
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\SIU\\CAPS", L"CAP_GUID_COINDISPENSER", L"YES");
	RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\SIU\\CONF", L"GUID_COINDISPENSER", SIDECAR_COD_FLICKER);

	// TODO: VB Flickers on MX5400
	//RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\SIU\\CAPS", L"CAP_GUID_FINDPROPERONE", L"YES");
	//RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\SIU\\CONF", L"GUID_FINDPROPERONE", EPP_VB_FLICKER);
	//RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\SIU\\CAPS", L"CAP_GUID_FINDPROPERONE", L"YES");
	//RegSetInt(L"SOFTWARE\\ATM\\DevInfo\\SIU\\CONF", L"GUID_FINDPROPERONE", CDU_VB_FLICKER);
	// end of [#GLDV-3005]


	// NEXTWARE (BSTRACE)
	// 2008.05.29 JSW //////////////////////////////////////////////////////////////////////////////////////
	// NextWare Log trace기능 On/Off 옵션 조절
	// TraceType		BS Trace		NVRAM Log
	//     1			OFF				ON
	//     2			ON				OFF
	//    이외			ON				ON

	// [#2570] AU HJAHN 2018.08.17	NEXTWARE Log ON/OFF 방식 변경, 특정 폴더 있을 경우 ON / 없을 경우 OFF
	//								필요 시 Patch 로 생성/삭제할 수 있도록 해당 폴더 위치를 ATM 파티션 내부로 둠
	//#if NEXTWARE_BSTRACE_ON
	if (IsExistFile( _T("\\ATM\\BSTRACE_ON") ))
	// end of [#2570]
	{
		//BSTRACE ON
		RegSetInt(L"SOFTWARE\\Nextware", L"TraceType", 2);					// 2:BS Trace 남김
		RegSetInt(L"SOFTWARE\\Nextware", L"Tracelevel", 5);					// Trace Level 조정
		RegSetInt(L"SOFTWARE\\Nextware", L"TraceFileSize", 0x1400000);		// Trace Size (20MB)
 		//RegSetStr(L"SOFTWARE\\Nextware", L"TraceDirectory", L"\\ATM2");		// Trace Folder
	}
	else
	{
		//BSTRACE OFF
		RegSetInt(L"SOFTWARE\\Nextware", L"TraceType", 1);			// 1:BS Trace 남기지 않음
	}

	// 2018.07.25 Device 시뮬레이션 Registry 설정
#ifdef DEV_SIMULATION
	// audio sensor
	RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\SIU\\CAPS", L"CAP_SEN_ENHANCEDAUDIO", L"NO");
	// card reader simulator
	// RegSetStr(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\CardReader", L"provider", L"NXIdc",1);
	// cash dispenser reader simulator
	RegSetStr(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\CashDispenser", L"provider", L"NXCdm",1);
	// pin simulator
	//RegSetStr(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\Encryptor", L"provider", L"NXPin",1);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\PIN\\KeyMap", L"F1", 16);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\PIN\\KeyMap", L"F2", 27);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\PIN\\KeyMap", L"F3", 17);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\PIN\\KeyMap", L"F4", 26);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\PIN\\KeyMap", L"F5", 18);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\PIN\\KeyMap", L"F6", 25);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\PIN\\KeyMap", L"F7", 19);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\PIN\\KeyMap", L"F8", 24);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\PIN\\KeyMap", L".", 100);
	// spr simulator
	RegSetStr(L".DEFAULT\\XFS\\LOGICAL_SERVICES\\ReceiptPrinter", L"provider", L"NXPtr",1);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"RESOLUTION", 3);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"READFORM", 0);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"WRITEFORM", 1);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"EXTENTS", 0);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"CONTROLS", 285);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"PAPERSOURCE", 2);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"STACKER", 0);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"MAXRETRACTBINS", 2);
	RegSetStr(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"MAXRETRACTCOUNT", L"50,50");
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"IMAGETYPE", 0);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"FRONTIMAGECOLORFORMAT", 0);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"BACKIMAGECOLORFORMAT", 0);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"CODELINECOLORFORMAT", 0);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"IMAGESOURCE", 0);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"CHARSET", 3);
	RegSetInt(L"SOFTWARE\\NEXTWARE\\SPR_SIM", L"ETC", 8);
	RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NXPtr", L"FormDirectory", L"\\ATM\\");
#endif

	//Delay_Msg(100);
	RETAILMSG(1, (L"Installing Nextware Registries... DONE\n"));

	return true;
}

bool CWinAtmApp::InstallAsianFonts()
{
	// [#2186] US KSK 2013.04.30 \\ATM폴더에 Font가 있는 경우 \\ATM2 폴더로 Copy한다.
	// (폰트 매체는 \\ATM\\Fonts에 설치되야 함)
	if (IsExistFile(ASIAN_FONT_SOURCE_PATH) == FALSE)
		return false;

	// [#2434] AU Kook 2016.08.11 Skip copy if DESTPATH '\ATM2\Fonts' already exists.
	// count font files (*.ttc) and compare it.
	int nFileCountSrcFont = 0;
	CountFileInDirectory(ASIAN_FONT_SOURCE_PATH, _T("*.ttc"), nFileCountSrcFont);

	int nFileCountDestFont = 0;
	if (IsExistFile(ASIAN_FONT_DEST_PATH) == TRUE)
		CountFileInDirectory(ASIAN_FONT_DEST_PATH,	 _T("*.ttc"), nFileCountDestFont);

	// if both contain equal count of font files, remove SRCPATH '\ATM\FONT' and skip copy & reboot.
	if (nFileCountSrcFont == nFileCountDestFont)
	{
		if (DeleteFileInDirectory(ASIAN_FONT_SOURCE_PATH, _T("*.*")) == TRUE)
			RemoveDirectory(ASIAN_FONT_SOURCE_PATH);

		return true;
	}
	// end of [#2434]

	// Font Update된 File이 존재하므로 \\ATM2\\Fonts 폴더에 해당 File을 Copy한다.
	// \ATM2\Fonts 폴더 생성
	CreateDirectory(ASIAN_FONT_DEST_PATH, NULL);
	if (IsExistFile(ASIAN_FONT_DEST_PATH) == TRUE)	// Folder가 생성되면 TRUE로 return함
	{
		// \ATM\Fonts -> \ATM2\Fonts 파일 복사
		if (CopyFileInDirectory(ASIAN_FONT_SOURCE_PATH, _T("*.*"), ASIAN_FONT_DEST_PATH) == TRUE)
		{
			// \ATM\Fonts 폴더 내부 파일 삭제
			if (DeleteFileInDirectory(ASIAN_FONT_SOURCE_PATH, _T("*.*")) == TRUE)
			{
				// \ATM\Fonts 폴더 삭제
				RemoveDirectory(ASIAN_FONT_SOURCE_PATH);
				return true;
			}
		}
	}
	// end of [#2186]

	return false;
}


// [#2545] US Kook 2018.03.28 Extract device essentials like SP.
bool CWinAtmApp::InstallWinAtmEssentials()
{
	NHDEBUG(DBG_CALL, (L"  ===> extracting WINATM essentials...\n"));

	CString strEssentials = _T("\\ATM\\WinATM_Essentials.zip");
	if (IsExistFile(strEssentials) == TRUE)
		ExtractZIPAll(strEssentials, _T("\\ATM"), true);

	return true;
}
// end of [#2545]


// [#2545] US Kook 2018.03.28 Extract essential screen resources.
bool CWinAtmApp::InstallImageResources()
{
	CString		strImageResourceZipFile = _T("");
	CString		strDestFileName = _T("");

	int nWidth = GetSystemMetrics(SM_CXSCREEN);
	int nHeight = GetSystemMetrics(SM_CYSCREEN);

	// == Screen Resource Files ==
	// AP_[width].ZIP		: 640, 800, 1024
	// OP_[width].ZIP		: 640, 720, 800, 1024
	// SCR_DAT.ZIP			: Screen Datas of All ATMs.

	// Additional Files
	// AP_1024_W.ZIP		: Wyvern Exclusive Transaction Images
	// OP_1024_768.ZIP		: 1024x768 OP images

	NHDEBUG(DBG_CALL, (_T("  ===> Detected resolution : [%dx%d] \n"), nWidth, nHeight));
	if (nWidth == 1024 || nWidth == 800 || nWidth == 640)
	{
		/////////////////////////////////////////////////////
		// 1) Extract OP images for current screen width.
		/////////////////////////////////////////////////////
		NHDEBUG(DBG_CALL, (_T("  ===> extracting OP images for width of [%d]...\n"), nWidth));
		strImageResourceZipFile.Format(_T("%s\\OP_%d.zip"), SCREEN_PATH, nWidth);
		ExtractZIPAll(strImageResourceZipFile, SCREEN_PATH, true);

		if (nHeight == 1200)
		{
			// 1-a) extract 720 x 480 OP images for NH2700T, whose height is 1200.
			NHDEBUG(DBG_CALL, (L"  ===> extracting OP images for [720x480]...\n"));
			strImageResourceZipFile.Format(_T("%s\\OP_720.zip"), SCREEN_PATH);
			ExtractZIPAll(strImageResourceZipFile, SCREEN_PATH, true);
		}
		else if (nHeight == 768)
		{
			// 1-b) extract 1024 x 768 OP images.
			NHDEBUG(DBG_CALL, (L"  ===> extracting OP images for [1024x768]...\n"));
			strImageResourceZipFile.Format(_T("%s\\OP_1024_768.zip"), SCREEN_PATH);
			ExtractZIPAll(strImageResourceZipFile, SCREEN_PATH, true);
		}
		// [#GLDV-2505] Support MX-2800T.
		else if (nHeight == 1536)
		{
			NHDEBUG(DBG_CALL, (L"  ===> extracting OP images for [1024x1536]...\n"));

			// 1-a) extract 720 x 480 OP images.
			NHDEBUG(DBG_CALL, (L"  ===> extracting OP images for [720x480]...\n"));
			strImageResourceZipFile.Format(_T("%s\\OP_720.zip"), SCREEN_PATH);
			ExtractZIPAll(strImageResourceZipFile, SCREEN_PATH, true);

			// 1-b) extract 1024 x 768 OP images. (Key Manager)
			NHDEBUG(DBG_CALL, (L"  ===> extracting OP images for [1024x768]...\n"));
			strImageResourceZipFile.Format(_T("%s\\OP_1024_768.zip"), SCREEN_PATH);
			ExtractZIPAll(strImageResourceZipFile, SCREEN_PATH, true);
		}
		// end of [#GLDV-2505]


		/////////////////////////////////////////////////////
		// 2) Extract AP images for current screen width.
		/////////////////////////////////////////////////////
		if (GetConfigFuncPointer()->GetOSVersion() == NH_OS_MX2800SE
			|| GetConfigFuncPointer()->GetOSVersion() == NH_OS_MX2800_F_T)		// [#GLDV-2505] Support MX-2800T
		{
			NHDEBUG(DBG_CALL, (_T("  ===> extracting Wyvern Exclusive AP images...\n"), nWidth));
			strImageResourceZipFile.Format(_T("%s\\AP_1024_W.zip"), SCREEN_PATH, nWidth);
			ExtractZIPAll(strImageResourceZipFile, SCREEN_PATH, true);
		}
		else
		{
			NHDEBUG(DBG_CALL, (_T("  ===> extracting AP images for width of [%d]...\n"), nWidth));
			strImageResourceZipFile.Format(_T("%s\\AP_%d.zip"), SCREEN_PATH, nWidth);
			ExtractZIPAll(strImageResourceZipFile, SCREEN_PATH, true);
		}


		/////////////////////////////////////////////////////
		// 3) Extract AP images for current screen width.
		/////////////////////////////////////////////////////
		NHDEBUG(DBG_CALL, (_T("  ===> extracting Screen Datas...\n"), nWidth));
		strImageResourceZipFile.Format(_T("%s\\SCR_DAT.zip"), SCREEN_PATH, nWidth);
		ExtractZIPAll(strImageResourceZipFile, SCREEN_PATH, true);
	}

	return true;
}
// end of [#2545]

// [#2545] NH Kook 2018.04.11 Support Customized Images.
bool CWinAtmApp::InstallCustomerAds()
{
	// 1) update customer images if there are new ones.
	if (IsExistFile(SCREEN_PATH_AD_CUSTOM) == TRUE)
	{
		NHDEBUG(DBG_CALL, (L"  ===> Custom Ads Found at ATM Partition ! => Copy to ATM2 => Delete from ATM Partition\n"));
		CreateDirectoryWithIntermediate(SCREEN_PATH_AD);
		CopyFileInDirectory(SCREEN_PATH_AD_CUSTOM, _T("*.*"), SCREEN_PATH_AD);
		if (DeleteFileInDirectory(SCREEN_PATH_AD_CUSTOM, _T("*.*")) == TRUE)
			RemoveDirectory(SCREEN_PATH_AD_CUSTOM);
		return true;
	}
	
	// 2-a) customer images found => no need to work.
	if (IsExistFile(SCREEN_PATH_AD))
	{
		NHDEBUG(DBG_CALL, (L"  ===> Custom Ads Found at ATM2 Partition : RETURN!\n"));
		return true;
	}

	// 2-b) customer images not found => install default images.
	NHDEBUG(DBG_CALL, (L"  ===> NO Advertisement Found at ATM2 Partition\n"));
	if (IsExistFile(FILE_DEFAULT_AD))
	{
		NHDEBUG(DBG_CALL, (L"  ===> Extract Default Advertisement to ATM2 Partition.\n"));
		ExtractZIPAll(FILE_DEFAULT_AD, SCREEN_PATH_AD);
	}
	else
		NHDEBUG(DBG_CALL, (L"  ===> WARNING .... NO Default Advertisement\n"));

	return false;
}

bool CWinAtmApp::InstallCustomerBacks()
{
	// 1) update customer images if there are new ones.
	if (IsExistFile(SCREEN_PATH_BACKS_CUSTOM) == TRUE)
	{
		NHDEBUG(DBG_CALL, (L"  ===> Custom Backs Found at ATM Partition ! => Copy to ATM2 => Delete from ATM Partition\n"));
		CreateDirectoryWithIntermediate(SCREEN_PATH_BACKS);
		CopyFileInDirectory(SCREEN_PATH_BACKS_CUSTOM, _T("*.*"), SCREEN_PATH_BACKS);
		if (DeleteFileInDirectory(SCREEN_PATH_BACKS_CUSTOM, _T("*.*")) == TRUE)
			RemoveDirectory(SCREEN_PATH_BACKS_CUSTOM);
		return true;
	}

	// 2-a) customer images found => no need to work.
	if (IsExistFile(SCREEN_PATH_BACKS))
	{
		NHDEBUG(DBG_CALL, (L"  ===> Custom Backs Found at ATM2 Partition : RETURN!\n"));
		return true;
	}

	// 2-b) customer images not found => install default images.
	NHDEBUG(DBG_CALL, (L"  ===> NO Backs Found at ATM2 Partition\n"));
	if (GetConfigFuncPointer()->GetOSVersion() == NH_OS_MX2800SE
		|| GetConfigFuncPointer()->GetOSVersion() == NH_OS_MX2800_F_T)		// [#GLDV-2505] Support MX-2800T
	{
		if (IsExistFile(FILE_DEFAULT_BACKS_2800))
		{
			NHDEBUG(DBG_CALL, (L"  ===> Extract Default 2800 Backs to ATM2 Partition.\n"));
			ExtractZIPAll(FILE_DEFAULT_BACKS_2800, SCREEN_PATH_BACKS);
		}
		else
			NHDEBUG(DBG_CALL, (L"  ===> WARNING .... NO Default 2800 Backs\n"));
	}
	else
	{
		if (IsExistFile(FILE_DEFAULT_BACKS))
		{
			NHDEBUG(DBG_CALL, (L"  ===> Extract Default General Backs to ATM2 Partition.\n"));
			ExtractZIPAll(FILE_DEFAULT_BACKS, SCREEN_PATH_BACKS);
		}
		else 
			NHDEBUG(DBG_CALL, (L"  ===> WARNING .... NO Default General Backs\n"));
	}

	return false;
}

bool CWinAtmApp::InstallCustomerRcptHeader()
{
	// 1) update customer images if there are new ones.
	if (IsExistFile(FILE_RCPT_HEADER_CUSTOM) == TRUE)
	{
		NHDEBUG(DBG_CALL, (L"  ===> Custom Receipt Header Image Found at ATM Partition ! => Copy to ATM2  => Delete from ATM Partition\n"));
		//DeleteFile(FILE_RCPT_HEADER);
		CopyFile(FILE_RCPT_HEADER_CUSTOM, FILE_RCPT_HEADER, FALSE);
		DeleteFile(FILE_RCPT_HEADER_CUSTOM);
		return true;
	}

	// 2-a) customer images found => no need to work.
	if (IsExistFile(FILE_RCPT_HEADER))
	{
		NHDEBUG(DBG_CALL, (L"  ===> Custom Receipt Header Image Found at ATM2 Partition : RETURN!\n"));
		return true;
	}

	// 2-b) customer images not found => install default images.
	NHDEBUG(DBG_CALL, (L"  ===> NO Receipt Header Image Found at ATM2 Partition\n"));
	if (IsExistFile(FILE_DEFAULT_RCPT_HEADER))
	{
		NHDEBUG(DBG_CALL, (L"  ===> Extract Default Receipt Header Image to ATM2 Partition.\n"));
		ExtractZIPAll(FILE_DEFAULT_RCPT_HEADER, _T("\\ATM2"));
	}
	else
		NHDEBUG(DBG_CALL, (L"  ===> WARNING .... NO Default Receipt Header Image\n"));

	return false;
}
// end of [#2545]

// [#2558] NH Justin 2018.06.12 Fix fof S/W Update Failure after "Format and Update"
bool CWinAtmApp::InstallTTS_DB()
{
	NHDEBUG(DBG_CALL, (L"  ===> extracting TTS_DB.zip if exist...\n"));
	ExtractZIPAll(FILE_TTS_DB, TTS_DB_PATH, true);
	return true;
}


bool CWinAtmApp::InstallExtendedBinList()
{
	if (IsExistFile(ATM_EXTENDED_BIN_FILE) == TRUE)
	{
		NHDEBUG(DBG_CALL, (L"  ===> ExtendedBinList.dat file found. Moving to ATM2\n"));
		CopyFile(ATM_EXTENDED_BIN_FILE, ATM2_EXTENDED_BIN_FILE, false);
	}

	return true;
}

// [#2545] US Kook 2018.04.16 Show dummy screen while essential files are being copied.
LRESULT CALLBACK InitWinAtmWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch(wMsg)
	{
	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);

			int			nScr = 0;
			int			OldBkMode;
			HFONT		OldFont = NULL;
			COLORREF	OldTextColor = 0xffffffff;

			OldBkMode = ::SetBkMode(hdc, TRANSPARENT);

			if (g_hLocalFont)
				OldFont = (HFONT)::SelectObject(hdc, g_hLocalFont);

			OldTextColor = ::SetTextColor(hdc, RGB(255,255,255));

			//CString	strInfoMsg = _T("Getting MoniPlus Ready...\r\nDon't turn off your ATM");
			CString	strInfoMsg = _T("Please Wait While Loading...");
			DrawText(hdc, strInfoMsg, -1, &g_DrawRect, DT_CENTER);

			::SetTextColor(hdc, OldTextColor);
			if (OldFont)
				::SelectObject(hdc, OldFont);
			::SetBkMode(hdc, OldBkMode);

			EndPaint(hWnd, &ps);

			return 0;
		}

	case WM_TIMER:
		{
			KillTimer(g_hWndDisplay, wParam);
			DestroyWindow(g_hWndDisplay);
		}

	default:	break;
	}

	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}
// end of [#2545]

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();		// Added for WCE apps
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CWinAtmApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CWinAtmApp commands
// Added for WCE apps

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CenterWindow();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
