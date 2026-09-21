/** *********************************************************
*	@file		EagleCE_ATMDlg.cpp
*	@brief		EagleCE ATM Main Dialog File
************************************************************/

#include "stdafx.h"
#include "EagleCE_ATM.h"
#include "EagleCE_ATMDlg.h"

#include "SVC_Manager.h"
#include "EagleSVCLib.h"
#include "EagleTritonMsg.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CEagleCE_ATMDlg 대화 상자

/** **********************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CEagleCE_ATMDlg::CEagleCE_ATMDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEagleCE_ATMDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//// 로거 생성
	CEagleLogger::CreateInstance();

	// Instance 생성
	CSVC_Manager::CreateInstance();
	
	// Instance 생성
	CSCR_Manager::CreateInstance();

	// Instance 생성
	CEagleTritonMsg::CreateInstance();

	// Instance 생성
	CEagleSVCLib::CreateInstance();

}

/** **********************************************************
*	@brief		소멸자
*	@retval		없음
************************************************************/
CEagleCE_ATMDlg::~CEagleCE_ATMDlg()
{
	CSVC_Manager::ReleaseInstance();

	CSCR_Manager::ReleaseInstance();

	CEagleTritonMsg::ReleaseInstance();

	CEagleSVCLib::ReleaseInstance();

	CEagleLogger::GetInstance()->Finalize();
	CEagleLogger::ReleaseInstance();
}

void CEagleCE_ATMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEagleCE_ATMDlg, CDialog)
	//}}AFX_MSG_MAP

	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()

END_MESSAGE_MAP()

// CEagleCE_ATMDlg 메시지 처리기

BOOL CEagleCE_ATMDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// Backup 후에 File 제거
	CString strPath;
	CString strFileName;

	// Customizing이 되지 않는 File은 고객이 건들 수 없도록 Resource화 함
	// Create Screen File
	strPath.Format(_T("%s\\Screen\\%d_%d"), CUtil::GetAppPath(), EAGLE_SCREEN_CX, EAGLE_SCREEN_CY);

	if (EAGLE_SCREEN_CX == CS130_WIDTH_RESOLUTION)
	{
		// Create Supervisor.dat file from resource
		strFileName = strPath + SCREEN_OP_DESIGN_FILE;
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_CS130_SCREEN_OP_DESIGN, strFileName);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);

		// Create Client_AdvScreen.dat file from resource
		strFileName = strPath + SCREEN_CLIENT_ADV_CTRL;
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_CS130_SCREEN_CLIENT_ADV_CTRL, strFileName);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);

		// Create HostErrorText.dat file from resource
		strFileName = strPath + TEXT_CLIENT_HOST_ERROR;
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_CS130_TEXT_CLIENT_HOST_ERROR, strFileName);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);

		// Create ReceiptPrintText.dat file from resource
		strFileName = strPath + TEXT_CLIENT_RECEIPTPRINT;
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_CS130_TEXT_CLIENT_RECEIPTPRINT, strFileName);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);

		// Create ScreenControl.dat file from resource
		strFileName = strPath + SCREEN_CLIENT_CTRL_FILE;
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_CS130_SCREEN_CLIENT_CTRL, strFileName);
	}
	else
	{
		// Create Supervisor.dat file from resource
		strFileName = strPath + SCREEN_OP_DESIGN_FILE;
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_MF200_SCREEN_OP_DESIGN, strFileName);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);

		// Create Client_AdvScreen.dat file from resource
		strFileName = strPath + SCREEN_CLIENT_ADV_CTRL;
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_MF200_SCREEN_CLIENT_ADV_CTRL, strFileName);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);

		// Create HostErrorText.dat file from resource
		strFileName = strPath + TEXT_CLIENT_HOST_ERROR;
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_MF200_TEXT_CLIENT_HOST_ERROR, strFileName);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);

		// Create ReceiptPrintText.dat file from resource
		strFileName = strPath + TEXT_CLIENT_RECEIPTPRINT;
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_MF200_TEXT_CLIENT_RECEIPTPRINT, strFileName);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);

		// Create ScreenControl.dat file from resource
		strFileName = strPath + SCREEN_CLIENT_CTRL_FILE;
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_MF200_SCREEN_CLIENT_CTRL, strFileName);
	}
	CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);

	// SSL 파일은 외부에서 접근하지 못하도록 Resource로 관리 - 변경될 일이 없으므로 File 미 존재시만 Resource에서 가져오도록 추가 보완
	// Create terminal-certificate.pem file from resource
	strFileName = CUtil::GetAppPath() + TERMINAL_CERTIFICATE_FILE;
	if (CUtil::IsExistFile(strFileName) == FALSE)
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_SSL_TERMINAL_CERTIFICATE, strFileName, RESOURCE_FORMAT_SSL);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Create terminal-privatekey.pem file from resource
	strFileName = CUtil::GetAppPath() + TERMINAL_PRIVATEKEY_FILE;
	if (CUtil::IsExistFile(strFileName) == FALSE)
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_SSL_TERMINAL_PRIVATEKEY, strFileName, RESOURCE_FORMAT_SSL);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Create ams-certificate.pem file from resource
	strFileName = CUtil::GetAppPath() + AMS_CERTIFICATE_FILE;
	if (CUtil::IsExistFile(strFileName) == FALSE)
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_SSL_AMS_CERTIFICATE, strFileName, RESOURCE_FORMAT_SSL);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}
	////////////////////////////////////////////////////////////////

	// ErrorCode DB 와 EJL DB가 존재하는지 Check

	// 1. EJL은 File이 존재하는 경우 기존 File 유지
	CString strDefaultDBFile;
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, EJL_DB_FILE_NAME);
	if (CUtil::IsExistFile(strPath) == FALSE)
	{
		// Default Folder에서 수동 Copy 수행
		strDefaultDBFile.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, EJL_DB_FILE_NAME);
		CopyFile(strDefaultDBFile, strPath, FALSE);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// ErrorCode DB는 추가 update 가능성이 있으므로 Power On시 Overwrite하도록 함
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, ERRORCODE_DB_FILE_NAME);
	{
		// Default Folder에서 수동 Copy 수행
		strDefaultDBFile.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, ERRORCODE_DB_FILE_NAME);
		CopyFile(strDefaultDBFile, strPath, FALSE);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Config File을 외부에서 접근하지 못하도록 Resource로 관리
	// Default의 File은 변경될 수 있으므로 Power On시 Overwrite하도록 함
	strPath = CUtil::GetAppPath() + EAGLE_ATM_CONFIG_DEFAULT_PATH;

	// Create ADS.CFG file from resource
	strFileName.Format(_T("%s\\%s"), strPath, ADS_CFG_PATH_NAME);
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_DEFAULT_CONFIG_ADS, strFileName, RESOURCE_FORMAT_CFG);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Create BINLIST.CFG file from resource
	strFileName.Format(_T("%s\\%s"), strPath, BINLIST_CFG_PATH_NAME);
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_DEFAULT_CONFIG_BINLIST, strFileName, RESOURCE_FORMAT_CFG);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Create CBXINFO.CFG file from resource
	strFileName.Format(_T("%s\\%s"), strPath, CBXINFO_CFG_PATH_NAME);
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_DEFAULT_CONFIG_CBXINFO, strFileName, RESOURCE_FORMAT_CFG);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Create DEVICE.CFG file from resource
	strFileName.Format(_T("%s\\%s"), strPath, DEVICE_CFG_PATH_NAME);
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_DEFAULT_CONFIG_DEVICE, strFileName, RESOURCE_FORMAT_CFG);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Create DISPENSE.CFG file from resource
	strFileName.Format(_T("%s\\%s"), strPath, DISPENSE_CFG_PATH_NAME);
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_DEFAULT_CONFIG_DISPENSE, strFileName, RESOURCE_FORMAT_CFG);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Create EMV_CONFIG.CFG file from resource
	strFileName.Format(_T("%s\\%s"), strPath, EAGLE_ATM_EMV_CONFIG_PATH_NAME);
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_DEFAULT_CONFIG_EMV, strFileName, RESOURCE_FORMAT_CFG);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Create EMV_US_COMM_AID.CFG file from resource
	strFileName.Format(_T("%s\\%s"), strPath, EAGLE_ATM_EMV_US_COMM_AID_PATH_NAME);
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_DEFAULT_CONFIG_EMV_US, strFileName, RESOURCE_FORMAT_CFG);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Create HOST.CFG file from resource
	strFileName.Format(_T("%s\\%s"), strPath, HOST_CFG_PATH_NAME);
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_DEFAULT_CONFIG_HOST, strFileName, RESOURCE_FORMAT_CFG);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	/// Create LASTTRANSINFO.CFG file from resource
	strFileName.Format(_T("%s\\%s"), strPath, LASTTRANSINFO_CFG_PATH_NAME);
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_DEFAULT_CONFIG_LASTTRANS, strFileName, RESOURCE_FORMAT_CFG);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Create MESSAGE.CFG file from resource
	strFileName.Format(_T("%s\\%s"), strPath, MESSAGE_CFG_PATH_NAME);
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_DEFAULT_CONFIG_MESSAGE, strFileName, RESOURCE_FORMAT_CFG);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Create OPTION.CFG file from resource
	strFileName.Format(_T("%s\\%s"), strPath, OPTION_CFG_PATH_NAME);
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_DEFAULT_CONFIG_OPTION, strFileName, RESOURCE_FORMAT_CFG);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Create SYSTEM.CFG file from resource
	strFileName.Format(_T("%s\\%s"), strPath, SYSTEM_CFG_PATH_NAME);
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_DEFAULT_CONFIG_SYSTEM, strFileName, RESOURCE_FORMAT_CFG);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	// Create TOTAL.CFG file from resource
	strFileName.Format(_T("%s\\%s"), strPath, TOTAL_CFG_PATH_NAME);
	{
		CUtil::CreateFileFromResource(::AfxGetInstanceHandle(), IDR_DEFAULT_CONFIG_TOTAL, strFileName, RESOURCE_FORMAT_CFG);
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	strPath.Format(_T("%s%s"), CUtil::GetAppPath(), PATH_LOG);
	CEagleSVCLib::GetInstance()->Add_Backup_LogFile(strPath, _T("POWERON"), FALSE);	// LOG를 남기지 않는다.

	//CUtil::Sleep_Wait(2000);

	CUtil::DeleteFileInDirectory(UPDATE_POOL, _T("*.*"));
	CUtil::DeleteFileInDirectory(PATH_TEMP_ABS, _T("*.*"));
	CUtil::DeleteFileInDirectory(PATH_LOG_ABS, _T("*.*"));

	::DeleteFile(_T("\\Update.dat"));
	::DeleteFile(_T("\\MasterScreenDesc.dat"));

	CUtil::Int_SetRegistry(UPDATE_PROC_REG_PATH, _T("FILE_PROC"), 0);

	// Create Capture Image Folder for Camera
	strPath.Format(_T("%s%s"), CUtil::GetAppPath(), CAPTURE_IMAGE_FILE_PATH);
	CreateDirectory(strPath, NULL);

	// Create Temp Capture Image Folader
	CreateDirectory(CAPTURE_IMAGE_FILE_PATH_ABS, NULL);

	//CUtil::Sleep_Wait(2000);

	CEagleLogger::GetInstance()->Initialize(_T("EagleCE_ATM"), 30, 1024 * 1024);

	LOG(Info, _T("================================================================================"));
	LOG(Info, _T("\t\t EagleCE Application Start"));
	LOG(Info, _T("================================================================================"));

	// Check USB memory Stick
	if (CUtil::IsExistFile(PATH_USB) == TRUE)
	{
		LOG(Info, _T("USB Memory Stick is detected"));
	}
	else
	{
		LOG(Info, _T("USB Memory Stick is not detected"));
	}

	CUtil::SetSWUpdateType(UPDATE_KIND_SOFTWARE_STRING);							// software update default 설정
	CUtil::Int_SetRegistry(UPDATE_PROC_REG_PATH, _T("FILE_PROC"), 0);		// software update prcessing 상태 default 설정

	// 프로그램 중복실행 방지
	HANDLE hEvent;
	hEvent = CreateEvent(NULL, FALSE, TRUE, AfxGetAppName());

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		LOG(Info, _T("Already Running EagleATM"));
		PostQuitMessage(WM_QUIT);
	}

	::MoveWindow(this->m_hWnd, 0, 0, 0, 0, FALSE);

	// Service 초기화
	CSVC_Manager::GetInstance()->SetNotifyWnd(this);
	CSVC_Manager::GetInstance()->Initialize();
	
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CEagleCE_ATMDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
}

BOOL CEagleCE_ATMDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	return CDialog::OnEraseBkgnd(pDC);
}

void CEagleCE_ATMDlg::OnPaint()
{
	CDialog::OnPaint();
}



