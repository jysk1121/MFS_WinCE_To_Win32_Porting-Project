// ScreenViewer_WinCEDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "ScreenViewer_WinCE.h"
#include "ScreenViewer_WinCEDlg.h"

#include "../../EagleCE_Device/EagleCE_Device/DEV_Manager.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"

#ifdef _WIN32_WCE
#include "../../Template/pkfuncs.h"
#endif // _WIN32_WCE

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


extern HWND			g_hWndDisplay;			// V1.0.2.4 2018.07.19 - Clinet Wnd Handle


// CScreenViewer_WinCEDlg 대화 상자

CScreenViewer_WinCEDlg::CScreenViewer_WinCEDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScreenViewer_WinCEDlg::IDD, pParent)
	, m_ScreenNum(_T("047"))
	, m_strBackUpCount(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScreenViewer_WinCEDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_ScreenNum);
	DDX_Text(pDX, IDC_EDIT2, m_strBackUpCount);
}

BEGIN_MESSAGE_MAP(CScreenViewer_WinCEDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON2, &CScreenViewer_WinCEDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CScreenViewer_WinCEDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CScreenViewer_WinCEDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CScreenViewer_WinCEDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CScreenViewer_WinCEDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CScreenViewer_WinCEDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &CScreenViewer_WinCEDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &CScreenViewer_WinCEDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, &CScreenViewer_WinCEDlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON11, &CScreenViewer_WinCEDlg::OnBnClickedButton11)
	ON_BN_CLICKED(IDC_BUTTON12, &CScreenViewer_WinCEDlg::OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON13, &CScreenViewer_WinCEDlg::OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON14, &CScreenViewer_WinCEDlg::OnBnClickedButton14)
	ON_BN_CLICKED(IDC_BUTTON15, &CScreenViewer_WinCEDlg::OnBnClickedButton15)
	ON_BN_CLICKED(IDC_BUTTON16, &CScreenViewer_WinCEDlg::OnBnClickedButton16)
	ON_BN_CLICKED(IDC_BUTTON17, &CScreenViewer_WinCEDlg::OnBnClickedButton17)
	ON_BN_CLICKED(IDC_BUTTON18, &CScreenViewer_WinCEDlg::OnBnClickedButton18)
	ON_BN_CLICKED(IDC_BUTTON19, &CScreenViewer_WinCEDlg::OnBnClickedButton19)
	ON_BN_CLICKED(IDC_BUTTON20, &CScreenViewer_WinCEDlg::OnBnClickedButton20)
	ON_BN_CLICKED(IDC_BUTTON21, &CScreenViewer_WinCEDlg::OnBnClickedButton21)
	ON_BN_CLICKED(IDC_BUTTON22, &CScreenViewer_WinCEDlg::OnBnClickedButton22)
	ON_BN_CLICKED(IDC_BUTTON23, &CScreenViewer_WinCEDlg::OnBnClickedButton23)
	ON_BN_CLICKED(IDC_BUTTON24, &CScreenViewer_WinCEDlg::OnBnClickedButton24)
	ON_BN_CLICKED(IDC_BUTTON25, &CScreenViewer_WinCEDlg::OnBnClickedButton25)
	ON_BN_CLICKED(IDC_REBOOT, &CScreenViewer_WinCEDlg::OnBnClickedReboot)
	ON_BN_CLICKED(IDC_BUTTON26, &CScreenViewer_WinCEDlg::OnBnClickedButton26)
	ON_BN_CLICKED(IDC_BUTTON27, &CScreenViewer_WinCEDlg::OnBnClickedButton27)
	ON_BN_CLICKED(IDC_BUTTON28, &CScreenViewer_WinCEDlg::OnBnClickedButton28)
	ON_BN_CLICKED(IDC_BUTTON29, &CScreenViewer_WinCEDlg::OnBnClickedButton29)
	ON_BN_CLICKED(IDC_BUTTON30, &CScreenViewer_WinCEDlg::OnBnClickedButton30)
	ON_BN_CLICKED(IDC_BUTTON31, &CScreenViewer_WinCEDlg::OnBnClickedButton31)
	ON_BN_CLICKED(IDC_BUTTON32, &CScreenViewer_WinCEDlg::OnBnClickedButton32)
	ON_BN_CLICKED(IDC_BUTTON33, &CScreenViewer_WinCEDlg::OnBnClickedButton33)
	ON_BN_CLICKED(IDC_BUTTON34, &CScreenViewer_WinCEDlg::OnBnClickedButton34)
	ON_BN_CLICKED(IDC_NEXT, &CScreenViewer_WinCEDlg::OnBnClickedNext)
	ON_BN_CLICKED(IDC_PREV, &CScreenViewer_WinCEDlg::OnBnClickedPrev)
END_MESSAGE_MAP()


// CScreenViewer_WinCEDlg 메시지 처리기
BOOL CScreenViewer_WinCEDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	// 로거 생성
	CEagleLogger::CreateInstance();
	CEagleLogger::GetInstance()->Initialize(_T("ScreenViewer"), 30, 1024 * 1024);

	// Instance 생성
	CSCR_Manager::CreateInstance();

	if (!CSCR_Manager::GetInstance()->Initialize(::AfxGetInstanceHandle(), 0, 0, _T("EAGLECE_ATM"), CUtil::GetAppPath() + _T("\\Screen")))
	{
		AfxMessageBox(_T("Screen Load Failed"));
	}

	int nScreenCount = CSCR_Manager::GetInstance()->GetScreenCount();
	CString strCurrentScreenNumber;

	int nIndex = 0;
	int i = 0;

	for(i=0; i<nScreenCount; i++)
	{
		if (i == 0)
		{
			strCurrentScreenNumber = CSCR_Manager::GetInstance()->GetFirstScreenNumber();
			LOG(Info, _T("Screen Number List : %s / %d"), strCurrentScreenNumber, nScreenCount);
		}
		else
		{
			strCurrentScreenNumber = CSCR_Manager::GetInstance()->GetNextScreenNumber();
			LOG(Info, _T("Screen Number List : %s / %d"), strCurrentScreenNumber, nScreenCount);
		}
	}

	int nValue = 0;
	CString strTemp;
	nValue = CUtil::Int_GetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_BUF_IN_REG_NAME);

	strTemp.Format(_T("[ScreenViewer] CRYPTERA EPP USB Drvier - Buffer In Registry Value - (0x%x)"), nValue);
	LOG(Info, strTemp);

	if (nValue == 0)
	{
		// Registry값이 적용되지 않았으므로 Registry 설정 후 해당 Registry값을 USB Driver가 적용하도록 System Reboot 수행
		CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_DRV_COMPORT_NAME, CRYPTERA_USB_DRV_COMPORT_VALUE);			// Comport Index Fix
		CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_BUF_IN_REG_NAME, CRYPTERA_USB_BUF_IN_REG_VALUE);				// Default is 8
		CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_BUF_OUT_REG_NAME, CRYPTERA_USB_BUF_OUT_REG_VALUE);			// Default is 8
		CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_CTRL_IN_BUF_REG_NAME, CRYPTERA_USB_CTRL_IN_BUF_REG_VALUE);	// Default is 4

		RegFlushKey(HKEY_LOCAL_MACHINE);
		CUtil::Sleep_Wait(3000);

		strTemp.Format(_T("[Screen_Viewer] CRYPTERA EPP USB Drvier - Set Registry Value"));
		LOG(Info, strTemp);

		OnBnClickedReboot();
	}

	// 장치 관리자 인스턴스 생성
	CDEV_Manager::CreateInstance();

	CUtil::Sleep_Wait(2000);

	// EPP Open
	if (0 == CDEV_Manager::GetInstance()->m_DEV_CREPP.OpenDevice(0, 115200))
	{
		LOG(Info, _T("Connect - CREPP - Succeeded"));		
	}
	else
	{
		LOG(Info, _T("Connect - CREPP - Failure"));
	}


	// Camera Open
	if(TRUE == CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsExistCameraDriver())
	{
		if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CAMERA.Open(g_hWndDisplay))
		{
			LOG(Error, _T("Connect - CAMERA - Failed")); 
		}
		else
		{
			LOG(Info, _T("Connect - CAMERA - Succeeded"));
		}
	}
	else
	{
		LOG(Info, _T("Connect - CAMERA - Driver is not exist."));
	}

	//i = 0;

	//MEMORYSTATUS memStatus;
	//DWORD UsingVirtualMemory = 0;
	//DWORD UsingPhysMemory = 0;
	//int nAgingCnt = 0;

	//while(TRUE)
	//{
	//	if ( i == 0)
	//	{
	//		strCurrentScreenNumber = CSCR_Manager::GetInstance()->GetFirstScreenNumber();
	//	}
	//	else
	//	{
	//		strCurrentScreenNumber = CSCR_Manager::GetInstance()->GetNextScreenNumber();
	//	}

	//	LOG(Info, _T("Display Screen : %s"), strCurrentScreenNumber);

	//	if (strCurrentScreenNumber.IsEmpty())
	//	{
	//		LOG(Info, _T("Can not Find Screen Number - Stop Aging"));
	//		break;
	//	}

	//	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), strCurrentScreenNumber);

	//	CUtil::Sleep_Wait(1000);

	//	memStatus.dwLength = sizeof(MEMORYSTATUS);
	//	GlobalMemoryStatus(&memStatus);

	//	UsingVirtualMemory = memStatus.dwTotalVirtual - memStatus.dwAvailVirtual;
	//	UsingPhysMemory = memStatus.dwTotalPhys - memStatus.dwAvailPhys;

	//	LOG(Info, _T("[MEMORY CHECK] Virtual Memory : (%dK) / (%dK)"), (UsingVirtualMemory / 1024), (memStatus.dwTotalVirtual / 1024));
	//	LOG(Info, _T("[MEMORY CHECK] Physical Memory : (%dK) / (%dK)"), (UsingPhysMemory / 1024), (memStatus.dwTotalPhys / 1024));

	//	i++;

	//	if ( i >= nScreenCount)
	//	{
	//		i = 0;
	//		nAgingCnt++;
	//	}

	//	//if (nAgingCnt > 200)		// 200인 경우 약 6시간 이므로 3일 Aging을 위해 Count 변경
	//	if (nAgingCnt > 2000)
	//	{
	//		LOG(Info, _T("Aging Count is over - Stop Aging"));
	//		break;
	//	}
	//}

	//CSCR_Manager::GetInstance()->DeInitalize();
	
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CScreenViewer_WinCEDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_SCREENVIEWER_WINCE_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_SCREENVIEWER_WINCE_DIALOG));
	}
}
#endif


void CScreenViewer_WinCEDlg::OnBnClickedButton2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("001"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("002"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("003"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("004"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("005"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton7()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("006"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton8()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("007"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton9()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("008"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton10()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("009"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton11()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("010"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton12()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("011"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton13()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("012"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton14()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("013"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton15()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("014"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton16()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("015"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton17()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("016"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton18()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("017"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton19()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("018"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton20()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("019"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton21()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("020"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton22()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("021"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton23()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("022"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton24()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("023"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton25()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("024"));
}

void CScreenViewer_WinCEDlg::OnBnClickedReboot()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// Hardware Reset 시도 후 Software Reset 시도 (HW Reset은 신규 Main B/D만 지원되므로 두 개 모두 수행하도록 함)
	DCB PortDCB;
	COMMTIMEOUTS CommTimeouts;
	HANDLE hPort;
	LPCTSTR pcszComPort= _T("COM5:");
	char buff[7] = { 0, };
	DWORD dwNumBytesWritten;


	hPort = CreateFile (pcszComPort, GENERIC_READ|GENERIC_WRITE, 0,  NULL,  OPEN_EXISTING, 0, NULL);
	if ( hPort == INVALID_HANDLE_VALUE )
	{
		LOG(Error, _T("Unable to CreateFile to the port"));
		goto error;
	}

	PortDCB.DCBlength = sizeof (DCB);    
	GetCommState (hPort, &PortDCB);
	PortDCB.BaudRate = 115200;              // Current baud
	PortDCB.fBinary = TRUE;               // Binary mode; no EOF check
	PortDCB.fParity = TRUE;               // Enable parity checking
	PortDCB.fOutxCtsFlow = FALSE;         // No CTS output flow control
	PortDCB.fOutxDsrFlow = FALSE;         // No DSR output flow control
	PortDCB.fDtrControl = FALSE;

	// DTR flow control type
	PortDCB.fDsrSensitivity = FALSE;      // DSR sensitivity
	PortDCB.fTXContinueOnXoff = TRUE;     // XOFF continues Tx
	PortDCB.fOutX = FALSE;                // No XON/XOFF out flow control
	PortDCB.fInX = FALSE;                 // No XON/XOFF in flow control
	PortDCB.fErrorChar = FALSE;           // Disable error replacement
	PortDCB.fNull = FALSE;                // Disable null stripping
	PortDCB.fRtsControl = FALSE;

	// RTS flow control
	PortDCB.fAbortOnError = FALSE;        // Do not abort reads/writes on
	// error
	PortDCB.ByteSize = 8;                 // Number of bits/byte, 4-8
	PortDCB.Parity = NOPARITY;            // 0-4=no,odd,even,mark,space
	PortDCB.StopBits = ONESTOPBIT;        // 0,1,2 = 1, 1.5, 2

	if (!SetCommState (hPort, &PortDCB))
	{
		LOG(Error, _T("Unable to set the time-out parameters"));
		goto error;
	}

	GetCommTimeouts (hPort, &CommTimeouts);
	CommTimeouts.ReadIntervalTimeout = MAXDWORD; 
	CommTimeouts.ReadTotalTimeoutMultiplier = 0; 
	CommTimeouts.ReadTotalTimeoutConstant = 0;   
	CommTimeouts.WriteTotalTimeoutMultiplier = 10; 
	CommTimeouts.WriteTotalTimeoutConstant = 1000;   

	if (!SetCommTimeouts (hPort, &CommTimeouts))
	{
		LOG(Error, _T("Unable to set the time-out parameters"));
		goto error;
	}

	EscapeCommFunction (hPort, SETDTR);
	EscapeCommFunction (hPort, SETRTS);

	buff[0]=1;
	buff[1]=2;
	buff[2]=1;
	buff[3]=5;
	buff[4]=2;
	buff[5]=0;
	buff[6]=0xb;

	if(!WriteFile(hPort, &buff, sizeof(buff), &dwNumBytesWritten, NULL) || dwNumBytesWritten!=sizeof(buff))
	{
		LOG(Error, _T("Unable to write to the port"));
	}

error:
	if(hPort)
		CloseHandle(hPort);

	LOG(Info, _T("Try to Hardware Reboot"));

	CUtil::Sleep_Wait(5000);

	LOG(Info, _T("Try to Software Reboot"));

	// Software Reboot 수행
	KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL);

}

void CScreenViewer_WinCEDlg::OnBnClickedButton26()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	WIN32_FIND_DATA	FileData;
	HANDLE			hSearch = INVALID_HANDLE_VALUE;
	CString strSrcFile;
	CString strDestFile;
	int nLoopCount = 100000;		// 10만번 Test
	int nCurrentCnt = 0;

	RETAILMSG(1,(_T("Backup Loop Test is started\n")));

	UpdateData(TRUE);

	hSearch = FindFirstFile(_T("\\Trace\\*.log"), &FileData);

	if (hSearch != INVALID_HANDLE_VALUE)
	{

		CreateDirectory(_T("\\USB\\TEST_USB"), NULL);

		while(nCurrentCnt < nLoopCount)
		{
			CUtil::DeleteFileInDirectory(_T("\\USB\\TEST_USB"), _T("*.*"));

			while(TRUE)
			{
				strSrcFile.Format(_T("\\Trace\\%s"), FileData.cFileName);
				strDestFile.Format(_T("\\USB\\TEST_USB\\%s"), FileData.cFileName);
				::CopyFile(strSrcFile, strDestFile, FALSE);

				if (FindNextFile(hSearch, &FileData) == FALSE)
					break;
			}

			FindClose(hSearch);

			CUtil::Sleep_Wait(100);

			nCurrentCnt++;

			m_strBackUpCount.Format(_T("%d"), nCurrentCnt);
			UpdateData(FALSE);
		}
	}

	RETAILMSG(1,(_T("Backup Loop Test is ended\n")));
}

BOOL bPTRPortOpen = FALSE;

void CScreenViewer_WinCEDlg::OnBnClickedButton27()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	// SNBC에서 송부한 File Read 후 해당 Data를 Write File을 통해 송신
	CFile binFile;
	CString strFileName = _T("\\NAND\\Korea.dat");

	if (FALSE == binFile.Open(strFileName, CFile::modeRead | CFile::typeBinary))
	{
		AfxMessageBox(_T("file open is failed"));
		return;
	}

	DWORD dwLength = (DWORD)binFile.GetLength();
	DWORD dwFileIndex = 0;
	char* lpData = NULL;
	DWORD dwRead = 0;
	BYTE  byErrorCode = 0;
	HANDLE	hFile;

	lpData = new char[dwLength+1];

	binFile.Close();

	hFile = ::CreateFile(strFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		AfxMessageBox(_T("CreateFile is failed"));

		if(lpData != NULL)
			delete[] lpData;

		return;
	}

	if (FALSE == ReadFile(hFile, lpData, dwLength, &dwRead, NULL))
	{
		AfxMessageBox(_T("ReadFile is failed"));

		if(lpData != NULL)
			delete[] lpData;

		::CloseHandle(hFile);

		return;
	}

	::CloseHandle(hFile);

	if (bPTRPortOpen == FALSE)
	{
		CString strComPort;
		int nRet = 0;

		strComPort.Format(_T("COM7:"));
		nRet = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_Open(GetSafeHwnd(), strComPort);

		if (!nRet)
		{
			AfxMessageBox(_T("Receipt Printer Port Open Failed"));
			return;
		}
	}

	// 10번 Print 후 Cut함.
//	for(int i=0; i<10; i++)
//	{
//		// Send Data
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_WriteFile(lpData, (int)dwRead);
//	}

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_CutPaper((0x0002), 1, 0);

	AfxMessageBox(_T("WriteFile Execute"));

	delete[] lpData;
}

void CScreenViewer_WinCEDlg::OnBnClickedButton28()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("025"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton29()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("026"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton30()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("027"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton31()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("028"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton32()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("029"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton33()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), _T("030"));
}

void CScreenViewer_WinCEDlg::OnBnClickedButton34()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), m_ScreenNum);
}

void CScreenViewer_WinCEDlg::OnBnClickedNext()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	int nScrNum = 0;

	nScrNum = CUtil::StringToInt(m_ScreenNum);
	nScrNum++;
	
	m_ScreenNum.Format(_T("%03d"), nScrNum);

	UpdateData(FALSE);

	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), m_ScreenNum);

}

void CScreenViewer_WinCEDlg::OnBnClickedPrev()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	int nScrNum = 0;

	nScrNum = CUtil::StringToInt(m_ScreenNum);
	nScrNum--;

	m_ScreenNum.Format(_T("%03d"), nScrNum);

	UpdateData(FALSE);

	CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), m_ScreenNum);
}
