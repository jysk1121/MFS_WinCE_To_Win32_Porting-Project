// ScreenViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ScreenViewer.h"
#include "ScreenViewerDlg.h"
#include "..\H\Dll\AssetArchive.h"

long long milliseconds_now() {
	static LARGE_INTEGER s_frequency;
	static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
	if (s_use_qpc) {
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return (1000LL * now.QuadPart) / s_frequency.QuadPart;
	} else {
		return GetTickCount();
	}
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

/////////////////////////////////////////////////////////////////////////////
// CScreenViewerDlg dialog

CScreenViewerDlg::CScreenViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScreenViewerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScreenViewerDlg)
	m_nScreenResourceLocation = SCREEN_NOT_FOUND;
	m_strRootDir = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScreenViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScreenViewerDlg)
	DDX_Control(pDX, IDC_OUTPUT, m_lsbStatusMessage);
	DDX_Control(pDX, IDC_COMBO_SCRNUM, m_cboScrList_Trx);
	DDX_Control(pDX, IDC_COMBO_COUNTRY, m_comboCountry);
	DDX_Text(pDX, IDC_EDIT_ROOT, m_strRootDir);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_CHECK_OP_1024_600, m_chkRes1024600_Op);
	DDX_Control(pDX, IDC_COMBO_SCRNUM_OP, m_cboScrList_Op);
	DDX_Control(pDX, IDC_CHECK_OP_800_600, m_chkRes800600_Op);
	DDX_Control(pDX, IDC_CHECK_OP_1024_768, m_chkRes1024768_Op);
	DDX_Control(pDX, IDC_CHECK_OP_1024_768_W, m_chkRes1024768_W_Op);
	DDX_Control(pDX, IDC_CHECK_OP_640_480, m_chkRes640480_Op);
	DDX_Control(pDX, IDC_CHECK_OP_720_480, m_chkRes720480_Op);
	DDX_Control(pDX, IDC_BUTTON_LANGUAGE_OP, m_btnLang_Op);
	DDX_Control(pDX, IDC_COMBO_SCRNUM_KEYMGR, m_cboScrList_KeyMgr);
	DDX_Control(pDX, IDC_CHECK_KEYMGR_640_480, m_chkRes640480_KeyMgr);
	DDX_Control(pDX, IDC_CHECK_KEYMGR_800_600, m_chkRes800600_KeyMgr);
	DDX_Control(pDX, IDC_CHECK_KEYMGR_1024_600, m_chkRes1024600_KeyMgr);
	DDX_Control(pDX, IDC_CHECK_KEYMGR_1024_768, m_chkRes1024768_KeyMgr);
	DDX_Control(pDX, IDC_CHECK_TRX_640_480, m_chkRes640480_Trx);
	DDX_Control(pDX, IDC_CHECK_TRX_1024_768, m_chkRes1024768_Trx);
	DDX_Control(pDX, IDC_CHECK_TRX_1024_768_W, m_chkRes1024768_W_Trx);
	DDX_Control(pDX, IDC_CHECK_TRX_800_600, m_chkRes800600_Trx);
	DDX_Control(pDX, IDC_CHECK_TRX_1024_600, m_chkRes1024600_Trx);
	DDX_Control(pDX, IDC_BUTTON_LANGUAGE_TRX, m_btnLang_Trx);
	DDX_Control(pDX, IDC_BUTTON_LANGUAGE_KEYMGR, m_btnLang_KeyMgr);
}

BEGIN_MESSAGE_MAP(CScreenViewerDlg, CDialog)
	//{{AFX_MSG_MAP(CScreenViewerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_LOAD, OnBtnLoadScreen_Trx)
	ON_BN_CLICKED(IDC_BTN_DIR, OnBtnDir)
	ON_BN_CLICKED(IDC_BTN_SHOW, OnBtnApplyValues)
	ON_BN_CLICKED(IDC_BUTTON1, OnButtonClearData)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_NEXT, OnBtnNextScr_Trx)
	ON_BN_CLICKED(IDC_BTN_PREV, OnBtnPrevScr_Trx)
	ON_CBN_SELENDOK(IDC_COMBO_SCRNUM, OnSelendokComboScrnum)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_INPUTALPHA, OnBnClickedSetVariable)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_LOAD_OP, &CScreenViewerDlg::OnBnClickedBtnLoadOp)
	ON_CBN_SELCHANGE(IDC_COMBO_SCRNUM_OP, &CScreenViewerDlg::OnCbnSelchangeComboScrnumOp)
	ON_BN_CLICKED(IDC_CHECK_OP_1024_600, &CScreenViewerDlg::OnBnClickedCheckOp1024600)
	ON_BN_CLICKED(IDC_CHECK_OP_800_600, &CScreenViewerDlg::OnBnClickedCheckOp800600)
	ON_BN_CLICKED(IDC_CHECK_OP_1024_768, &CScreenViewerDlg::OnBnClickedCheckOp1024768)
	ON_BN_CLICKED(IDC_CHECK_OP_1024_768_W, &CScreenViewerDlg::OnBnClickedCheckOp1024768_W)
	ON_BN_CLICKED(IDC_CHECK_OP_720_480, &CScreenViewerDlg::OnBnClickedCheckOp720480)
	ON_BN_CLICKED(IDC_CHECK_OP_640_480, &CScreenViewerDlg::OnBnClickedCheckOp640480)
	ON_BN_CLICKED(IDC_BUTTON_LANGUAGE_OP, &CScreenViewerDlg::OnBnClickedButtonLanguageOp)
	ON_BN_CLICKED(IDC_BTN_LOAD_KEYMGR, &CScreenViewerDlg::OnBnClickedBtnLoadKeymgr)
	ON_BN_CLICKED(IDC_CHECK_TRX_640_480, &CScreenViewerDlg::OnBnClickedCheckTrx640480)
	ON_BN_CLICKED(IDC_BUTTON_LANGUAGE_TRX, &CScreenViewerDlg::OnBnClickedButtonLanguageTrx)
	ON_BN_CLICKED(IDC_BUTTON_LANGUAGE_KEYMGR, &CScreenViewerDlg::OnBnClickedButtonLanguageKeymgr)
	ON_BN_CLICKED(IDC_CHECK_TRX_1024_768_W, &CScreenViewerDlg::OnBnClickedCheckTrx1024768_W)
	ON_BN_CLICKED(IDC_CHECK_TRX_1024_768, &CScreenViewerDlg::OnBnClickedCheckTrx1024768)
	ON_BN_CLICKED(IDC_CHECK_TRX_800_600, &CScreenViewerDlg::OnBnClickedCheckTrx800600)
	ON_BN_CLICKED(IDC_CHECK_TRX_1024_600, &CScreenViewerDlg::OnBnClickedCheckTrx1024600)
	ON_BN_CLICKED(IDC_BTN_PREV_OP, &CScreenViewerDlg::OnBnClickedBtnPrevOp)
	ON_BN_CLICKED(IDC_BTN_NEXT_OP, &CScreenViewerDlg::OnBnClickedBtnNextOp)
	ON_BN_CLICKED(IDC_BTN_PREV_KEYMGR, &CScreenViewerDlg::OnBnClickedBtnPrevKeymgr)
	ON_BN_CLICKED(IDC_BTN_NEXT_KEYMGR, &CScreenViewerDlg::OnBnClickedBtnNextKeymgr)
	ON_BN_CLICKED(IDC_CHECK_KEYMGR_640_480, &CScreenViewerDlg::OnBnClickedCheckKeymgr640480)
	ON_BN_CLICKED(IDC_CHECK_KEYMGR_800_600, &CScreenViewerDlg::OnBnClickedCheckKeymgr800600)
	ON_BN_CLICKED(IDC_CHECK_KEYMGR_1024_600, &CScreenViewerDlg::OnBnClickedCheckKeymgr1024600)
	ON_BN_CLICKED(IDC_CHECK_KEYMGR_1024_768, &CScreenViewerDlg::OnBnClickedCheckKeymgr1024768)
	ON_BN_CLICKED(IDC_APSTATE_1, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_2, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_3, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_4, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_5, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_6, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_7, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_8, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_9, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_10, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_11, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_12, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_13, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_14, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_15, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_16, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_17, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_18, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_19, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_20, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_39, &CScreenViewerDlg::OnBnClickedApState)
	ON_BN_CLICKED(IDC_APSTATE_43, &CScreenViewerDlg::OnBnClickedApState)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScreenViewerDlg message handlers

BOOL CScreenViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	InitControls();
	InitLogger();

	ReadChosenResolutions();
	UpdateData(FALSE);

	// waiting key-pressed event
	SetTimer(1000, 500, NULL);
	

	return TRUE;  // return TRUE  unless you set the focus to a control
}

DWORD _stdcall CScreenViewerDlg::LoggerThread( void *pThreadHandler )
{
	CScreenViewerDlg *scrViewerDlg = static_cast<CScreenViewerDlg*> (pThreadHandler);

	DWORD ret = 0;

	while (true)
	{
		if (scrViewerDlg->m_qLogMessages.empty())
		{
			// no logs to be written
		}
		else
		{
			while (!scrViewerDlg->m_qLogMessages.empty())
			{
				CString strLogMessage = scrViewerDlg->m_qLogMessages.front();
				scrViewerDlg->m_lsbStatusMessage.AddString(strLogMessage);
				scrViewerDlg->m_qLogMessages.pop();
			}
			scrViewerDlg->m_lsbStatusMessage.SetCurSel(scrViewerDlg->m_lsbStatusMessage.GetCount()-1);
			scrViewerDlg->m_lsbStatusMessage.UpdateWindow();
		}

		Sleep(200);
	}

	return ret;
}


BOOL CScreenViewerDlg::IsScreenVisible(TYPE_OF_SCREEN scr, TYPE_OF_RESOLUTION res)
{
	switch (scr)
	{
	case SCR_TRX:		return m_bShowTrxScreens[res];
	case SCR_OP:		return m_bShowOpScreens[res];
	case SCR_KEYMGR:	return m_bShowKeyMgrScreens[res];
	default:
		return FALSE;
	}

	return FALSE;
}

BOOL CScreenViewerDlg::IsResolutionAvailable(TYPE_OF_SCREEN scr, TYPE_OF_RESOLUTION res)
{
	switch (scr)
	{
	case SCR_TRX:
		if (res == RES_720_480)
			return FALSE;

	case SCR_OP:
		break;

	case SCR_KEYMGR:
		if (res == RES_720_480)
			return FALSE;

	default:
		break;
	}

	return TRUE;
}

CScrCtrl** CScreenViewerDlg::GetScrCtrlPtr(TYPE_OF_SCREEN scr, TYPE_OF_RESOLUTION res)
{
	//
	// m_pScreens_Trx[]									[SCR1]		[SCR2]		[SCR3]		[SCR4]		[SCR5]
	// ^CScrCtrl**										^CScrCtrl*	^CScrCtrl*	^CScrCtrl*	^CScrCtrl*	^CScrCtrl*
	CScrCtrl** pScrCtrl = NULL;

	switch (scr)
	{
	case SCR_TRX:		pScrCtrl = &m_pScreens_Trx[res];		break;
	case SCR_OP:		pScrCtrl = &m_pScreens_Op[res];			break;
	case SCR_KEYMGR:	pScrCtrl = &m_pScreens_KeyMgr[res];		break;
	default:
		break;
	}

	return pScrCtrl;
}

void CScreenViewerDlg::DetectResourceType()
{
	UpdateData(TRUE);

	WriteStatus(_T("Detecting resource location from [%s]..."), m_strRootDir);
	AfxGetApp()->WriteProfileString(_T("Setting"), _T("Folder"), m_strRootDir);

	// 1) /DAT/SCREEN (code)
	CString strScreenDataFolder;
	strScreenDataFolder.Format(L"%s\\%s\\%s", m_strRootDir, L"DAT", L"SCREEN");
	if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(strScreenDataFolder))
	{
		WriteStatus(_T("./DAT/SCREEN folder found."));
		m_nScreenResourceLocation = FROM_CODE_DAT_SCREEN;
		m_strScreenRootFolder = strScreenDataFolder;
		return;
	}

	// 2) /SCREEN/SCREEN.TAR (extracted MASTER.ZIP, current style)
	if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(SCREEN_ASSET_TARBALL))
	{
		WriteStatus(_T("./SCREEN/SCREEN.TAR file found. (extracted MASTER.ZIP, new-style)"));
		m_nScreenResourceLocation = FROM_MASTER_TARBALL;

		// adjustment for 'this->basePath' in AssetArchive.
		m_strScreenRootFolder = m_strRootDir + "\\SCREEN";
		return;
	}

	// 3) /SCREEN (extracted MASTER.ZIP, old style)
	strScreenDataFolder.Format(L"%s\\%s", m_strRootDir, L"SCREEN");
	if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(strScreenDataFolder))
	{
		WriteStatus(_T("./SCREEN folder found. (extracted MASTER.ZIP, old-style)"));
		m_nScreenResourceLocation = FROM_MASTER_SCREEN;
		m_strScreenRootFolder = m_strRootDir + "\\SCREEN";
		
		return;
	}

	WriteStatus(_T("Screen resource not found"));
	m_nScreenResourceLocation = SCREEN_NOT_FOUND;
}

void CScreenViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CScreenViewerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CScreenViewerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CScreenViewerDlg::PrepareScreenResource()
{
	if (m_bResourceReady)
	{
		WriteStatus(L"Resource already prepared");
		return TRUE;
	}

	CString strCountry;
	switch (m_comboCountry.GetCurSel())
	{
	case 0:		strCountry = _T("US");		break;
	case 1:		strCountry = _T("CA");		break;
	case 2:		strCountry = _T("AU");		break;
	case 3:		strCountry = _T("MX");		break;
	}
	AfxGetApp()->WriteProfileString(_T("Setting"), _T("Country"), strCountry);
	WriteStatus(L"Country Written: [%s]", strCountry);

	DetectResourceType();
	switch (m_nScreenResourceLocation)
	{
	case FROM_CODE_DAT_SCREEN:
		WriteStatus(L"Resource Type: Source Code");
		DestructResourceFolder();
		ConstructResourceFolder();
		break;

	case FROM_MASTER_SCREEN:
		WriteStatus(L"Resource Type: Master.zip");
		break;

	case FROM_MASTER_TARBALL:
		WriteStatus(L"Resource Type: Tarball");
		break;
	
	case SCREEN_NOT_FOUND:
	default:
		AfxMessageBox(L"Screen resource not found. Please select the folder of the source, or extracted Master.zip.");
		return FALSE;
	}

	m_bResourceReady = TRUE;
	return TRUE;
}

void CScreenViewerDlg::ConstructResourceFolder()
{
	long long loadStartTime;

	WriteStatus(L"Constructing resource structure...");
	loadStartTime = milliseconds_now();

	// 2013.05.30 KMK 해당 국가에 맞는 스크립트를 실행하여 별도의 디렉토리 구조를 만들어준다
	// 뷰어용 Junction / Hardlink 생성
	CString strCountry = AfxGetApp()->GetProfileString(_T("Setting"), _T("Country"), NULL);

	CString strScriptOption;
	if (strCountry.GetLength() > 0)
	{
		strScriptOption.Format(L"\"%s\" %s", m_strScreenRootFolder, strCountry);
		m_strScreenRootFolder += "\\.restructured_" + strCountry;

		// MX2800SE
		strScriptOption += L" 2800";
	}

	SHELLEXECUTEINFO ShExecInfo;
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO); 
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS; 
	ShExecInfo.hwnd = NULL; 
	ShExecInfo.lpVerb = NULL; 
	ShExecInfo.lpFile = L"makeHardLink.bat";
	ShExecInfo.lpParameters = strScriptOption;
	ShExecInfo.lpDirectory = NULL; 
	ShExecInfo.nShow = FALSE;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

	WriteStatus(L"Construction completed: %lli ms", milliseconds_now() - loadStartTime);
}

void CScreenViewerDlg::DestructResourceFolder()
{
	long long loadStartTime;

	WriteStatus(L"Cleaning existing resource structure...");
	loadStartTime = milliseconds_now();

	// 기존 디렉토리 삭제
	CString strScriptOption;
	strScriptOption.Format(L"\"%s\"", m_strScreenRootFolder);

	SHELLEXECUTEINFO ShExecInfo;
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO); 
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS; 
	ShExecInfo.hwnd = NULL; 
	ShExecInfo.lpVerb = NULL; 
	ShExecInfo.lpFile = L"delHardLink_ALL.bat";        
	ShExecInfo.lpParameters = strScriptOption;
	ShExecInfo.lpDirectory = NULL; 
	ShExecInfo.nShow = FALSE;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

	WriteStatus(L"Cleaning completed: %lli ms", milliseconds_now() - loadStartTime);
	m_bResourceReady = FALSE;
}

CString	CScreenViewerDlg::GetScreenSection(TYPE_OF_SCREEN scr, TYPE_OF_RESOLUTION res)
{
	CString strScreenSection;
	switch (scr)
	{
	case SCR_TRX:
		switch (res)
		{
		case RES_640_480:			strScreenSection = _T("1500_FRONT_FUNCTION");			break;
		//case RES_720_480:			strScreenSection = _T("UNKNOWN");						break;
		case RES_800_600:			strScreenSection = _T("1800SE_FRONT_FUNCTION");			break;
		case RES_1024_600:			strScreenSection = _T("FRONT_FUNCTION");				break;
		case RES_1024_768:			strScreenSection = _T("5200_FRONT_FUNCTION");			break;
		case RES_1024_768_W:		strScreenSection = _T("2800_FRONT_FUNCTION");			break;
		default:																			break;
		}
		break;

	case SCR_OP:
		switch (res)
		{
		case RES_640_480:			strScreenSection = _T("1500_REAR_FUNCTION");			break;
		case RES_720_480:			strScreenSection = _T("REAR_TOUCH");					break;
		case RES_800_600:			strScreenSection = _T("1800SE_REAR_FUNCTION");			break;
		case RES_1024_600:			strScreenSection = _T("REAR_FUNCTION");					break;
		case RES_1024_768:			strScreenSection = _T("5200_REAR_FUNCTION");			break;
		case RES_1024_768_W:		strScreenSection = _T("2800_REAR_FUNCTION");			break;
		default:																			break;
		}
		break;

	case SCR_KEYMGR:
		switch (res)
		{
		case RES_640_480:			strScreenSection = _T("1500_KEYMGR_FUNCTION");			break;
		//case RES_720_480:			strScreenSection = _T("UNKNOWN");						break;
		case RES_800_600:			strScreenSection = _T("1800SE_KEYMGR_FUNCTION");		break;
		case RES_1024_600:			strScreenSection = _T("KEYMGR_FUNCTION");				break;
		case RES_1024_768:			strScreenSection = _T("5200_KEYMGR_FUNCTION");			break;
		case RES_1024_768_W:		strScreenSection = _T("2800_KEYMGR_FUNCTION");			break;
		default:																			break;
		}
		break;
	}

	return strScreenSection;
}

CComboBox* CScreenViewerDlg::GetCBOScrListPtr(TYPE_OF_SCREEN scr)
{
	CComboBox* cboScrList = NULL;
	switch (scr)
	{
	case SCR_TRX:		cboScrList = &m_cboScrList_Trx;		break;
	case SCR_OP:		cboScrList = &m_cboScrList_Op;		break;
	case SCR_KEYMGR:	cboScrList = &m_cboScrList_KeyMgr;	break;
	default:
		break;
	}
	return cboScrList;
}

BOOL CScreenViewerDlg::UnloadScreen(TYPE_OF_SCREEN scr)
{
	if (m_bScreenLoaded[scr] == TRUE)
	{
		WriteStatus(_T("Loaded screen found. Unloading it..."));

		GetCBOScrListPtr(scr)->ResetContent();
		ActivateScreenControls(scr, FALSE);
		m_bScreenLoaded[scr] = FALSE;

		for (int res=0; res<RES_COUNT; res++)
		{
			if (!IsResolutionAvailable(scr, (TYPE_OF_RESOLUTION)res))
				continue;

			CScrCtrl** pScrCtrl = GetScrCtrlPtr(scr, (TYPE_OF_RESOLUTION)res);
			if (*pScrCtrl)
			{
				CScrCtrl::DestroyScreenEngine(*pScrCtrl);
				*pScrCtrl = NULL;
			}
		}
	}

	if (m_nScreenResourceLocation == FROM_CODE_DAT_SCREEN)
	{
		// no need to "DestructResourceFolder" for unloading since they're links to original files, not copied-old-files.
		//	DestructResourceFolder();
	}

	return TRUE;
}

BOOL CScreenViewerDlg::LoadScreen(TYPE_OF_SCREEN scr)
{
	if (!PrepareScreenResource())
		return FALSE;


	WriteStatus(_T("Loading screens... - [%s]"), GET_SCR_STRING(scr));
	long long loadStartTime = milliseconds_now();
	for (int res=0; res<RES_COUNT; res++)
	{
		if (!IsResolutionAvailable(scr, (TYPE_OF_RESOLUTION)res))
			continue;

		CScrCtrl** pScrCtrl = GetScrCtrlPtr(scr, (TYPE_OF_RESOLUTION)res);
		if (*pScrCtrl == NULL)
		{
			CScrCtrl* tmpScr = CScrCtrl::CreateScreenEngine(SCR_TYPE_PICASSO, 0, 0, GET_RES_WIDTH(res), GET_RES_HEIGHT(res));
			if (tmpScr != NULL)
			{
				*pScrCtrl = tmpScr;		// allocate CScrCtrl* to m_pScreens_XXX
				(*pScrCtrl)->Initialize();
				(*pScrCtrl)->LoadScreens(m_strScreenRootFolder, GetScreenSection(scr, (TYPE_OF_RESOLUTION)res), GET_RES_WIDTH(res), GET_RES_HEIGHT(res));

				WriteStatus(_T("Loading screen [%d_%d]..."), GET_RES_WIDTH(res), GET_RES_HEIGHT(res));
			}
		}
	}

	// since LoadScreens() runs in parallel, it would be better to wait "LOAD_OK" here instead of next to LoadScreens() for speed (like multiplexing)
	for (int res=0; res<RES_COUNT; res++)
	{
		CScrCtrl** pScrCtrl = GetScrCtrlPtr(scr, (TYPE_OF_RESOLUTION)res);
		if (*pScrCtrl)
		{
			CString strCmd, strData;
			while ((*pScrCtrl)->GetUserInputCheck(INFINITE))
			{
				if ((*pScrCtrl)->GetUserInputData(strCmd, strData))
				{
					//WriteStatus(_T("GetUserInputData(strCmd, strData): %s, %s"), strCmd, strData);		// to see what it say
					if (strData.Left(4) == _T("LOAD"))
						break;
				}
			}
		}

		UpdateScreenVisibility(scr, (TYPE_OF_RESOLUTION)res);
	}

	WriteStatus(L"Screen load completed - [%s]: %lli ms", GET_SCR_STRING(scr), (milliseconds_now() - loadStartTime));
	m_bScreenLoaded[scr] = TRUE;

	ActivateScreenControls(scr, TRUE);

	InitScreenList(scr);
	LoadPresetValues(scr);
	ApplyScreenValues(scr);

	return TRUE;
}

int CALLBACK MyBrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg) 
	{
		case BFFM_INITIALIZED:
			::SendMessage( hwnd, BFFM_SETSELECTION, TRUE, lpData );
			break;
	}
	return 0;
}

void CScreenViewerDlg::OnBtnDir() 
{
	// TODO: Add your control notification handler code here
	ITEMIDLIST  *pidlBrowse;
    TCHAR       pszPathName[MAX_PATH];

    BROWSEINFO BrInfo;
    BrInfo.hwndOwner = NULL;//parent window handle
	BrInfo.pidlRoot = NULL;

 	LPTSTR pFolderName = m_strScreenRootFolder.GetBuffer(m_strScreenRootFolder.GetLength());
//	m_strScreenRootFolder.ReleaseBuffer();		// ** test code **

    memset(&BrInfo, 0, sizeof(BrInfo));
    BrInfo.pszDisplayName = pszPathName;
    BrInfo.lpszTitle = _T("Root Directory for WinCE Screen Viewer");
    BrInfo.ulFlags = BIF_RETURNONLYFSDIRS;
	BrInfo.lpfn = MyBrowseCallbackProc;
	BrInfo.lParam = (LPARAM)pFolderName;
	pidlBrowse = ::SHBrowseForFolder(&BrInfo);    

    if( pidlBrowse != NULL)
    {
        ::SHGetPathFromIDList(pidlBrowse, pszPathName);  // get folder name
		
		m_strScreenRootFolder = pszPathName;
		GetDlgItem(IDC_EDIT_ROOT)->SetWindowText(m_strScreenRootFolder);
		AfxGetApp()->WriteProfileString(_T("Setting"), _T("Folder"), m_strScreenRootFolder);
    }

	m_strScreenRootFolder.ReleaseBuffer();
}

void CScreenViewerDlg::OnBtnApplyValues() 
{
	// TODO: Add your control notification handler code here
	//LoadPresetValues(SCR_TRX);
	ApplyScreenValues(SCR_TRX);

	//LoadPresetValues(SCR_OP);
	ApplyScreenValues(SCR_OP);

	//LoadPresetValues(SCR_KEYMGR);
	ApplyScreenValues(SCR_KEYMGR);
}

void CScreenViewerDlg::OnButtonClearData() 
{
	// TODO: Add your control notification handler code here
	int nCntStates = sizeof(m_nIdApStates) / sizeof(int);
	int nCntValues = sizeof(m_nIdApValues) / sizeof(int);

	for (int i=0; i<nCntStates; i++) {
		((CButton*)GetDlgItem(m_nIdApStates[i]))->SetCheck(BST_CHECKED);
	}

	for (int i=0; i<nCntValues; i++) {
		(GetDlgItem(m_nIdApValues[i]))->SetWindowText(_T(""));
	}
}

void CScreenViewerDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == 1000)
	{
		for (int scr=0; scr<SCR_COUNT; scr++)
		{
			if (!m_bScreenLoaded[scr])
				continue;

			for (int res=0; res<RES_COUNT; res++)
			{
				if (!IsResolutionAvailable((TYPE_OF_SCREEN)scr, (TYPE_OF_RESOLUTION)res))
					continue;

				CScrCtrl** pScrCtrl = GetScrCtrlPtr((TYPE_OF_SCREEN)scr, (TYPE_OF_RESOLUTION)res);
				if (*pScrCtrl == NULL)
					continue;

				if ((*pScrCtrl)->GetUserInputCheck())
				{
					CString strName, strData;
					(*pScrCtrl)->GetUserInputData(strName, strData);
					strData.Replace(0x1F, '^');
					WriteStatus(_T("[FSCommand] -- [%s]-[%s]"), strName, strData);

					// send "APCenterMovie" again to enable button input
					ApplyScreenValues((TYPE_OF_SCREEN)scr);
				}
			}
		}
	}
	
	CDialog::OnTimer(nIDEvent);
}

BOOL CScreenViewerDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN) 
	{
        if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) 
			return TRUE;
    }
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CScreenViewerDlg::ShowPrevScreen(TYPE_OF_SCREEN scr)
{
	CComboBox* pCbScrList = GetCBOScrListPtr(scr);
	int nIndex = pCbScrList->GetCurSel();
	if (nIndex-1 >= 0)
	{
		nIndex--;
		pCbScrList->SetCurSel(nIndex);
	}

	LoadPresetValues(scr);
	OnBtnApplyValues();
}

void CScreenViewerDlg::ShowNextScreen(TYPE_OF_SCREEN scr)
{
	CComboBox* pCbScrList = GetCBOScrListPtr(scr);
	int nIndex = pCbScrList->GetCurSel();
	if (nIndex+1 < pCbScrList->GetCount())
	{
		nIndex++;
		pCbScrList->SetCurSel(nIndex);
	}

	LoadPresetValues(scr);
	OnBtnApplyValues();
}

void CScreenViewerDlg::OnSelendokComboScrnum() 
{
	// TODO: Add your control notification handler code here
	OnBtnApplyValues();
}

void CScreenViewerDlg::OnClose()
{
	if (m_nScreenResourceLocation == FROM_CODE_DAT_SCREEN)	// 신구조인 경우
	{
		DestructResourceFolder();
	}

	CDialog::OnClose();
}

CString CScreenViewerDlg::ReadCurrentAPValues()
{
	CString strApValue;

	m_arAPValues.RemoveAll();
	(GetDlgItem(IDC_APVALUE_1))->GetWindowText(strApValue);		m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_2))->GetWindowText(strApValue);		m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_3))->GetWindowText(strApValue);		m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_4))->GetWindowText(strApValue);		m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_5))->GetWindowText(strApValue);		m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_6))->GetWindowText(strApValue);		m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_7))->GetWindowText(strApValue);		m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_8))->GetWindowText(strApValue);		m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_9))->GetWindowText(strApValue);		m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_10))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_11))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_12))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_13))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_14))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_15))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_16))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_17))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_18))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_19))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_20))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_21))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_22))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_23))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_24))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_25))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_26))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_27))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_28))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_29))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);
	(GetDlgItem(IDC_APVALUE_30))->GetWindowText(strApValue);	m_arAPValues.Add(strApValue);

	CString strAPValues;
	for (int i=0; i<m_arAPValues.GetCount(); i++)
	{
		if (i != 0)
			strAPValues += _T("\r");

		strAPValues += m_arAPValues.GetAt(i);
		strAPValues.Replace(_T("\\n"), _T("\n"));
	}

	return strAPValues;
}

CString CScreenViewerDlg::ReadCurrentAPStates()
{
	m_arAPStates.RemoveAll();
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_2))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_3))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_4))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_5))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_6))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_7))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_8))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_9))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_10))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_11))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_12))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_13))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_14))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_15))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_16))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_17))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_18))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_19))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_20))->GetCheck());

	// 21 ~ 38 (dummy)
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());

	// 39
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_39))->GetCheck());

	// 40 ~ 42 (dummy)
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck());

	// 43
	m_arAPStates.Add(((CButton*)GetDlgItem(IDC_APSTATE_43))->GetCheck());

	CString strAPStates;
	for (int i=0; i<m_arAPStates.GetCount(); i++)
	{
		if (i != 0)
			strAPStates += _T("\r");

		if (m_arAPStates.GetAt(i))
			strAPStates += _T("on");
		else
			strAPStates += _T("off");
	}

	return strAPStates;
}

void CScreenViewerDlg::InitLogger()
{
	m_hLogger = CreateThread(NULL,		/* Must be NULL */ 
		0,								/* Stack size */ 
		LoggerThread,					/* Thread Start Routine */ 
		(void*)this,					/* Thread Parameter */ 
		NULL,							/* Creation Flag */ 
		&m_nLoggerThreadId);			/* Thread ID */ 
}

void CScreenViewerDlg::WriteStatus(LPCWSTR lpszFmt, ...)
{
	WCHAR tszInfo[2048] = {};

	va_list args;
	va_start(args, lpszFmt);
	vswprintf_s(tszInfo, 2048, lpszFmt, args);
	va_end(args);

	SYSTEMTIME	ti;
	GetLocalTime(&ti);

	CString strTimestampMessage;
	strTimestampMessage.Format(L"[%02d/%02d %02d:%02d:%02d.%03d] %s", ti.wMonth, ti.wDay, ti.wHour, ti.wMinute, ti.wSecond, ti.wMilliseconds, tszInfo);

	m_qLogMessages.push(strTimestampMessage);
}

// save previous window size!
static int xPos[RES_COUNT] = { -1, -1, -1, -1, -1, -1 };
static int yPos[RES_COUNT] = { -1, -1, -1, -1, -1, -1 };

void CScreenViewerDlg::UpdateScreenVisibility(TYPE_OF_SCREEN scr, TYPE_OF_RESOLUTION res)
{
	CScrCtrl** pScrCtrl = GetScrCtrlPtr(scr, res);
	if (*pScrCtrl == NULL)
		return;

	ReadChosenResolutions();
	if (IsScreenVisible(scr, res))
	{
		//WriteStatus(_T("Show screen - [%s] [%dx%d]"), GET_SCR_STRING(scr), GET_RES_WIDTH(res), GET_RES_HEIGHT(res));
		
		CString strLoadOrReload;
		GetDlgItem(IDC_BTN_LOAD)->GetWindowText(strLoadOrReload);

		// initial positions for each window
		if (strLoadOrReload == L"Load")
		{
			xPos[res] = GET_SCRPOS_X(res);
			yPos[res] = GET_SCRPOS_Y(res);
		}

		// save current positions when screens are being reloaded
		if (m_bScreenLoaded[scr] == TRUE)
		{
			// where's the best location for each screen?
			RECT rect = (*pScrCtrl)->GetScreenRect();
			//WriteStatus(_T("Rect for res[%d]: [%d, %d, %d, %d"), res, rect.bottom, rect.left, rect.right, rect.top);

			xPos[res] = rect.left;
			yPos[res] = rect.top;
		}

		int nBorderThickness = GetSystemMetrics(SM_CXSIZEFRAME) * 2;
		nBorderThickness += 4;	// required for accurate window size, but where it comes from?

		//WriteStatus(_T("Move to: [%d, %d]"), xPos[res], yPos[res]);

		(*pScrCtrl)->MoveScreen(xPos[res], yPos[res],
			GET_RES_WIDTH(res) + nBorderThickness,
			GET_RES_HEIGHT(res) + nBorderThickness + GetSystemMetrics(SM_CYCAPTION));

	}
	else
	{
		//if (m_bScreenLoaded[scr])	// write 'Hide' status for loaded screens only
		//	WriteStatus(_T("Hide screen - [%s] [%dx%d]"), GET_SCR_STRING(scr), GET_RES_WIDTH(res), GET_RES_HEIGHT(res));
		
		(*pScrCtrl)->HideWindow();
	}
}

void CScreenViewerDlg::LoadPresetValues(TYPE_OF_SCREEN scr)
{
	OnButtonClearData();

	CString strCountry = AfxGetApp()->GetProfileString(_T("Setting"), _T("Country"), NULL);
	WriteStatus(L"Load Preset for [%s]", strCountry);

	// "Preset" would be managed by file later, hard-coded for now

	// TRX test
	if (scr == SCR_TRX)
	{
		CComboBox* pCbScrList = GetCBOScrListPtr(SCR_TRX);
		int nIndex = pCbScrList->GetCurSel();
		if (nIndex == -1)	return;

		CString strScrNum;
		pCbScrList->GetLBText(pCbScrList->GetCurSel(), strScrNum);

		if (strScrNum == L"101" || strScrNum == L"401" || strScrNum == L"402")
		{
			// NH2700
			(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("Insert card and do not\nremove until prompted."));
			// MX2800SE
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Insert card and do not\nremove until prompted."));
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("CASH_NO"));
			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("RECEIPT_NO"));

			// AU Surcharge notice
			if (strCountry == L"AU")
			{
				(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("This ATM applies a direct charge of XX.XX(1) to cash withdrawals and\nXX.XX(2) to balance enquiries initiated with Australian cards."));

				if (strScrNum == L"401")
				{
					(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("Please insert and remove your membership card\nto redeem points in lieu of paying a surcharge."));
					(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T(""));

				}
				else if (strScrNum == L"402")
				{
					(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("Insert card and do not\nremove until prompted."));
					(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T(""));
				}
			}

			((CButton*)GetDlgItem(IDC_APSTATE_1))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_2))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_3))->SetCheck(BST_CHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_4))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_5))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_6))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_7))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_8))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_9))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_10))->SetCheck(BST_UNCHECKED);

			//// KSK TEST CODE
			//strTemp.Format(_T("%d|%d|%d|-|%d|C|%d|%d"), 2010, 11, 30, 9, 100, 110);
			//m_pScrCtrl->SetVariable(_T("APWeather1"), strTemp);

			//strTemp.Format(_T("%d|%d|%d|-|%d|C|%d|%d"), 2010, 12, 1, 10, 200, 210);
			//m_pScrCtrl->SetVariable(_T("APWeather2"), strTemp);
			//// end of TEST CODE
		}
		else if (strScrNum == L"102")
		{
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Language Selection"));
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Please select the language."));

			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("English"));
			(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("Español"));
			(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("Français"));
			(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("中文"));
			(GetDlgItem(IDC_APVALUE_8))->SetWindowText(_T("한국어"));
			(GetDlgItem(IDC_APVALUE_9))->SetWindowText(_T("日本語"));
		}
		else if (strScrNum == L"103")
		{
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Password Input"));
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Enter your Personal Identification Number(PIN).\nThen press ENTER."));
			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("Please use PIN pad to protect your information"));

			if (strCountry == L"AU")
			{
				(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("Please make sure that there are no unauthorized\nthings such as a camera around the PIN pad"));
			}
		}
		else if (strScrNum == L"105")
		{
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Transaction Selection"));
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("What would you like to do?\nPlease make a selection."));

			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("WITHDRAWAL"));
			(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("BALANCE"));
			(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("TRANSFER"));
		}
		else if (strScrNum == L"106")
		{
			(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("Yes"));
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Digital"));
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("No"));
			
			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("Would you like to receive a receipt?"));

			(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("SAVEPAPER_TREE"));
			(GetDlgItem(IDC_APVALUE_8))->SetWindowText(_T("It's easy being GREEN\nYou can save paper to \nsave tree and save the earth."));
			(GetDlgItem(IDC_APVALUE_9))->SetWindowText(_T("You can reduce 0.49g carbon\nfootprint at this time."));
			(GetDlgItem(IDC_APVALUE_10))->SetWindowText(_T(""));


			(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("SAVEPAPER_DRECEIPT"));
			(GetDlgItem(IDC_APVALUE_8))->SetWindowText(_T("Introducing the Hyosung\nMoniPlus DigitalReceipt.\n\nView your secure transaction on\nyour mobile device by scanning\nthe QR code"));
			(GetDlgItem(IDC_APVALUE_9))->SetWindowText(_T("Download the app from your iPhone or\nAndroid device."));
			(GetDlgItem(IDC_APVALUE_10))->SetWindowText(_T("Search : MoniPlus DigitalReceipt"));

		}
		else if (strScrNum == L"107")
		{
			(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("RECEIPT_ERR"));
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Receipt Not Available"));

			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Balance details displayed on screen.\nWould you like to continue transaction?"));
		}
		else if (strScrNum == L"108")
		{
			(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("INFO"));
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Timeout"));

			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Do you need more time?"));
		}
		else if (strScrNum == L"109")
		{
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Fee Notice"));
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Do you wish to continue ?"));

			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("The owner of this terminal,\n"));
			(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("OWNER"));
			(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("charges a fee of "));
			(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("$ 100"));

			(GetDlgItem(IDC_APVALUE_10))->SetWindowText(_T("for Balance Inquiry."));

			(GetDlgItem(IDC_APVALUE_13))->SetWindowText(_T("This fee is in addition to any fee\nyour financial institution may charge."));

			if (strCountry == L"AU")
			{
				(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("ATM Operator Fee Notification"));
				(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Do you wish to continue ?"));

				(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("If you continue, you will be charged"));
				(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("$ 100"));
				(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("for this transaction by "));
				(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("OWNER"));

				(GetDlgItem(IDC_APVALUE_8))->SetWindowText(_T("Contact1"));
				(GetDlgItem(IDC_APVALUE_9))->SetWindowText(_T("Contact2"));
				(GetDlgItem(IDC_APVALUE_10))->SetWindowText(_T("Contact3"));

				(GetDlgItem(IDC_APVALUE_11))->SetWindowText(_T("Your card Issuer may also\ncharge you a fee for using this ATM"));
			}
		}
		else if (strScrNum == L"110")
		{
			if (strCountry == L"AU")
			{
				(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Fee Notice"));
				(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Do you want to continue ?"));

				(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("USE OF THIS ATM INCURS\nA CONVENIENCE FEE OF"));
				(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("$ 100"));
				(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("IN ADDITION YOUR BANK MAY ALSO\nCHARGE YOU A FEE"));

				(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("OWNER"));
				(GetDlgItem(IDC_APVALUE_8))->SetWindowText(_T("Contact1"));
				(GetDlgItem(IDC_APVALUE_9))->SetWindowText(_T("Contact2"));
				(GetDlgItem(IDC_APVALUE_10))->SetWindowText(_T("Contact3"));

				(GetDlgItem(IDC_APVALUE_11))->SetWindowText(_T("Your card Issuer may also\ncharge you a fee for using this ATM"));
			}
		}
		else if (strScrNum == L"111")
		{
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Account Selection"));
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Which account would you like to withdraw from?"));

			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("CHECKING"));
			(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("SAVING"));
			(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("CREDIT"));
		}
		else if (strScrNum == L"112")
		{
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Amount Selection"));
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Please select the amount\nyou would like to withdraw."));

			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("   $ 100   "));
			(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("   $ 200   "));
			(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("   $ 300   "));
			(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("   $ 400   "));
			(GetDlgItem(IDC_APVALUE_8))->SetWindowText(_T("   $ 500   "));
			(GetDlgItem(IDC_APVALUE_9))->SetWindowText(_T("   $ 600   "));
		}
		else if (strScrNum == L"113")
		{
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Amount Input"));
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Enter the amount you would like to withdraw\nThen press ENTER."));

			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("DOLLAR"));
			(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("INT_DOLLAR"));
			(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("Multiple of amount"));
			(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("$ 20, $ 50, $ 100"));
			(GetDlgItem(IDC_APVALUE_8))->SetWindowText(_T("Max amount"));
			(GetDlgItem(IDC_APVALUE_9))->SetWindowText(_T("$ 600"));

			(GetDlgItem(IDC_APVALUE_10))->SetWindowText(_T("1"));
			(GetDlgItem(IDC_APVALUE_11))->SetWindowText(_T("4"));
			(GetDlgItem(IDC_APVALUE_12))->SetWindowText(_T("off"));

			if (strCountry == L"AU")
			{
				(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("Multiple of amount :"));
				(GetDlgItem(IDC_APVALUE_8))->SetWindowText(_T("Amount up to:"));
			}
		}
		else if (strScrNum == L"116")
		{
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Connecting to the host\n(Configuration)"));
			(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("Please wait"));

			((CButton*)GetDlgItem(IDC_APSTATE_1))->SetCheck(BST_CHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_2))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_3))->SetCheck(BST_UNCHECKED);
		}
		else if (strScrNum == L"117")
		{
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("I'm dispensing your money.\nPlease wait !"));
		}
		else if (strScrNum == L"118")
		{
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Please take your money."));
		}
		else if (strScrNum == L"119")
		{
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Printing your transaction record"));
		}
		else if (strScrNum == L"120")
		{
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Please take your receipt."));
		}
		else if (strScrNum == L"121")
		{
			(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("         1         2         3         4"));
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("1234567890123456789012345678901234567890"));
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("3   567890123    89012345678901234567890"));
			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("4   5678901234567890123456    1234567890"));
			(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("5   5678     4567    2345678901234567890"));
			(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("6   5678901234567890      7890     67890"));
			(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("7   5678901    6789012345678901234    90"));
		}
		else if (strScrNum == L"122")
		{
			(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("STOP"));

			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Transaction has been cancelled."));
			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("ATM error during transaction."));

			if (strCountry == L"AU")
			{
				//(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Your transaction has been cancelled."));
				(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("DONTMOVE"));

				(GetDlgItem(IDC_APVALUE_8))->SetWindowText(_T("Card removed during transaction."));
				(GetDlgItem(IDC_APVALUE_9))->SetWindowText(_T("Please try again."));
			}
		}
		else if (strScrNum == L"123")
		{
			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("Thank you for using our service."));

			if (strCountry == L"AU")
			{
				(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("Exit Message 1\nExit Message 2\nExit Message 3"));
			}
		}
		else if (strScrNum == L"125")
		{
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Please wait a moment."));
		}
		else if (strScrNum == L"126")
		{
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("AID Selection"));
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Select application ID"));
		}
		else if (strScrNum == L"127")
		{
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Please remove your card."));
		}
		else if (strScrNum == L"128")
		{
			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("Your chip card is not valid.\nDo you want to do Magnetic Stripe\ntransaction?"));
		}
		else if (strScrNum == L"131" || strScrNum == L"135")
		{
			(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("Currency Conversion Option"));
			
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Amount requested"));
			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("20.00 USD"));
			(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("Surcharge Fee"));
			(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("2.00 USD"));
			(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("Total Amount"));
			(GetDlgItem(IDC_APVALUE_8))->SetWindowText(_T("22.00 USD"));
			(GetDlgItem(IDC_APVALUE_9))->SetWindowText(_T("Exchange Rate"));
			(GetDlgItem(IDC_APVALUE_10))->SetWindowText(_T("1 USD = 1200 KRW"));
			(GetDlgItem(IDC_APVALUE_11))->SetWindowText(_T("Account Charge"));
			(GetDlgItem(IDC_APVALUE_12))->SetWindowText(_T("26400 KRW"));
			(GetDlgItem(IDC_APVALUE_13))->SetWindowText(_T("Exchange Rate Mark-Up"));
			(GetDlgItem(IDC_APVALUE_14))->SetWindowText(_T("9.99 %"));

			(GetDlgItem(IDC_APVALUE_17))->SetWindowText(_T("Charge my account\n22.00 USD"));
			(GetDlgItem(IDC_APVALUE_18))->SetWindowText(_T("Charge my account\n26400 KRW"));

			if (strCountry == L"AU")
			{
				//(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("This ATM offers conversion to your home currency"));
				(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("Please choose the currency to be charged to your account"));

				(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Make sure you understand the costs of currency conversion\nas they may be different depending on whether you select\nyour home currency or the transaction currency"));
				
				(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Cash"));
				(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("Fee"));
				//(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("Total"));
				(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("Total without conversion"));
				(GetDlgItem(IDC_APVALUE_9))->SetWindowText(_T("Exchange"));
				(GetDlgItem(IDC_APVALUE_11))->SetWindowText(_T("Total with conversion"));

				(GetDlgItem(IDC_APVALUE_17))->SetWindowText(_T("Continue without\nConversion"));
				(GetDlgItem(IDC_APVALUE_18))->SetWindowText(_T("Continue with\nConversion"));
			}
		}
		else if (strScrNum == L"137")
		{				
			// dreceipt
		}
		else if (strScrNum == L"138")
		{
			(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("RECEIPT_ERR"));
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Receipt Not Available"));
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Would you like to continue transaction?"));
			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("Digital\nReceipt"));
			(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("No Receipt"));

			if (strCountry == L"AU")
			{
				(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Sorry, we are unable to print a receipt.\nWould you like to continue transaction?"));
				(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("Digital"));
			}
		}
		else if (strScrNum == L"164")
		{
			if (strCountry == L"AU")
			{
				(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("Loyalty Point Service Notice"));
				(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("Loyalty Point Service is currently not available"));

				(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("Would you like to continue transaction"));
				(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("without the service?"));

				(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("Miscellaneous error"));
			}
		}
		else if (strScrNum == L"170")
		{
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Connecting to the host\n(Redeem Points)"));
			(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("Please wait"));

			((CButton*)GetDlgItem(IDC_APSTATE_1))->SetCheck(BST_CHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_2))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_3))->SetCheck(BST_UNCHECKED);
		}
		else if (strScrNum == L"170")
		{
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("Connecting to the host\n(Redeem Points)"));
			(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("Please wait"));

			((CButton*)GetDlgItem(IDC_APSTATE_1))->SetCheck(BST_CHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_2))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_APSTATE_3))->SetCheck(BST_UNCHECKED);
		}

	}


	// OP test
	else if (scr == SCR_OP)
	{
		CComboBox* pCbScrList = GetCBOScrListPtr(SCR_OP);
		int nIndex = pCbScrList->GetCurSel();
		if (nIndex == -1)	return;

		CString strScrNum;
		pCbScrList->GetLBText(pCbScrList->GetCurSel(), strScrNum);

		if (strScrNum == L"600")
		{
			(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("Multiline Test"));
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("AllowRowCol:4:40"));
		}
		else if (strScrNum == L"901")
		{
			(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T("MX-9900SE"));
			(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T("HYOSUNG"));
			(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T("0000000000"));
			(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T("STANDARD 1"));
			(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T("TCP/IP"));
			(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T("V99.99.99"));
			(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T("11/11/2020 11:11:11"));
			(GetDlgItem(IDC_APVALUE_8))->SetWindowText(_T("NORMAL"));

			(GetDlgItem(IDC_APVALUE_19))->SetWindowText(_T("NO"));
			(GetDlgItem(IDC_APVALUE_21))->SetWindowText(_T("NO"));

			// 1500
			(GetDlgItem(IDC_APVALUE_27))->SetWindowText(_T("QUICK\nCONFIG."));

			((CButton*)GetDlgItem(IDC_APSTATE_9))->SetCheck(BST_UNCHECKED);
		}
	}

	else if (scr == SCR_KEYMGR)
	{
		CComboBox* pCbScrList = GetCBOScrListPtr(SCR_KEYMGR);
		int nIndex = pCbScrList->GetCurSel();
		if (nIndex == -1)	return;

		CString strScrNum;
		pCbScrList->GetLBText(pCbScrList->GetCurSel(), strScrNum);
	}
}

void CScreenViewerDlg::ApplyScreenValues(TYPE_OF_SCREEN scr)
{
	if (m_bScreenLoaded[scr] == FALSE)
		return;

	UpdateData(TRUE);
	CComboBox* pCbScrList = GetCBOScrListPtr(scr);
	if (pCbScrList->GetCurSel() == -1)
	{
		MessageBox(_T("화면을 선택하여 주십시오"));
		return;
	}

	CString strScrNum;
	pCbScrList->GetLBText(pCbScrList->GetCurSel(), strScrNum);

	for (int res=0; res<RES_COUNT; res++)
	{
		if (!IsResolutionAvailable(scr, (TYPE_OF_RESOLUTION)res))
			continue;

		CString strBackName;
		if (scr == SCR_TRX)
		{
			// there's no available backgrounds when it's TARBALL mode... use something instead
			if (m_nScreenResourceLocation == FROM_MASTER_TARBALL)
				strBackName.Format(_T("%s\\%d_%d\\AP_Back_LTRY_ADA.jpg"), m_strScreenRootFolder, /*GET_RES_WIDTH(res)*/ 1024, /*GET_RES_HEIGHT(res)*/ 600);
			else
				strBackName.Format(_T("%s\\Backs\\%d_%d\\Function\\Back_1.jpg"), m_strScreenRootFolder, GET_RES_WIDTH(res), GET_RES_HEIGHT(res));
		}

		CScrCtrl** pScrCtrl = GetScrCtrlPtr(scr, (TYPE_OF_RESOLUTION)res);

		if ((*pScrCtrl) == NULL)
		{
			WriteStatus(_T("!!! SOMETHING IS WRONG !!!"));
		}
		else
		{
			(*pScrCtrl)->SetVariable(_T("PrevSetScreenNo"), strScrNum);
			(*pScrCtrl)->SetVariable(_T("kindLanguage"), GET_LANG_STRING(m_nLanguage[scr]));
			(*pScrCtrl)->SetVariable(_T("APValue1"), ReadCurrentAPValues());
			(*pScrCtrl)->SetVariable(_T("APState1"), ReadCurrentAPStates());

			CString strCmd, strData;
			(GetDlgItem(IDC_VARCMD))->GetWindowText(strCmd);
			(GetDlgItem(IDC_VARDATA))->GetWindowText(strData);
			(*pScrCtrl)->SetVariable(L"APTranAdv", L"1");				// Number of Advertisement
			(*pScrCtrl)->SetVariable(L"APTranAdvTime", L"3600");		// Refresh Time (Second) => refresh every hour. (60*60)
			(*pScrCtrl)->SetVariable(strCmd, strData);

			(*pScrCtrl)->SetVariable(_T("APBackImage"), strBackName);		// APBackImage seems to be placed in where between PrevSetScreenNo ~ APCenterMovie.
			(*pScrCtrl)->SetVariable(_T("APCenterMovie"), strScrNum);	

			//(*pScrCtrl)->SetActiveScreen();		// I'm not sure if it is better to show screen in front of others
			UpdateScreenVisibility(scr, (TYPE_OF_RESOLUTION)res);
		}
	}

	// output
	WriteStatus(_T("[SetScreenLocale] -- [%s]"), GET_LANG_STRING(m_nLanguage[scr]));
	WriteStatus(_T("[APCenterMovie] -- [%s]"), strScrNum);
}

void CScreenViewerDlg::ActivateScreenControls(TYPE_OF_SCREEN scr, BOOL bOnOff)
{
	switch (scr)
	{
	case SCR_TRX:
		m_chkRes640480_Trx.EnableWindow(bOnOff);
		m_chkRes800600_Trx.EnableWindow(bOnOff);
		m_chkRes1024600_Trx.EnableWindow(bOnOff);
		m_chkRes1024768_Trx.EnableWindow(bOnOff);
		m_chkRes1024768_W_Trx.EnableWindow(bOnOff);
		break;
	
	case SCR_OP:
		m_chkRes640480_Op.EnableWindow(bOnOff);
		m_chkRes720480_Op.EnableWindow(bOnOff);
		m_chkRes800600_Op.EnableWindow(bOnOff);
		m_chkRes1024600_Op.EnableWindow(bOnOff);
		m_chkRes1024768_Op.EnableWindow(bOnOff);
		m_chkRes1024768_W_Op.EnableWindow(bOnOff);
		(GetDlgItem(IDC_INPUTALPHA))->EnableWindow(bOnOff);
		(GetDlgItem(IDC_VARCMD))->EnableWindow(bOnOff);
		(GetDlgItem(IDC_VARDATA))->EnableWindow(bOnOff);
		break;
	
	case SCR_KEYMGR:
		m_chkRes640480_KeyMgr.EnableWindow(bOnOff);
		m_chkRes800600_KeyMgr.EnableWindow(bOnOff);
		m_chkRes1024600_KeyMgr.EnableWindow(bOnOff);
		m_chkRes1024768_KeyMgr.EnableWindow(bOnOff);
		break;
	}

	CComboBox* cbScrList = GetCBOScrListPtr(scr);
	if (cbScrList)
		cbScrList->EnableWindow(bOnOff);
}

void CScreenViewerDlg::InitScreenList(TYPE_OF_SCREEN scr)
{
	CComboBox* pCbScrList = GetCBOScrListPtr(scr);
	if (pCbScrList == NULL)
		return;

	CScrCtrl** pScrCtrl = GetScrCtrlPtr(scr, RES_1024_600);	// use 1024x600 as standard for now :)
	if (*pScrCtrl == NULL)
		return;

	int nScreenCount = (*pScrCtrl)->GetScreenCount();
	if (nScreenCount == 0)
		return;

	pCbScrList->InitStorage(nScreenCount, 10);

	CString strScrNumber =	(*pScrCtrl)->GetFirstScreenNumber();
	while(nScreenCount > 0)
	{
		pCbScrList->AddString(strScrNumber);

		nScreenCount--;
		if (nScreenCount <= 0)
			break;

		strScrNumber = (*pScrCtrl)->GetNextScreenNumber();
	}

	switch (scr)
	{
	case SCR_TRX:
		pCbScrList->SetCurSel(pCbScrList->SelectString(-1, _T("101")));				break;
	case SCR_OP:
		pCbScrList->SetCurSel(pCbScrList->SelectString(-1, _T("901")));				break;
	case SCR_KEYMGR:
		pCbScrList->SetCurSel(pCbScrList->SelectString(-1, _T("S_KEYMGRMAIN")));		break;
	}

	// adjust width of dropdown listbox for Key Manager screen which has long screen name.
	pCbScrList->SetDroppedWidth(GetRequiredComboDroppedWidth(*pCbScrList));
}

void CScreenViewerDlg::InitControls()
{
	m_strRootDir = AfxGetApp()->GetProfileString(_T("Setting"), _T("Folder"), NULL);
	m_strScreenRootFolder = m_strRootDir;

	CString strCountry = AfxGetApp()->GetProfileString(_T("Setting"), _T("Country"), NULL);
	WriteStatus(L"Init Country: [%s]", strCountry);

	if (strCountry.IsEmpty())
		m_comboCountry.SetCurSel(0);	// 2013.05.30 KMK 국가정보 기본값을 US로 설정
	else
		m_comboCountry.SetCurSel(m_comboCountry.SelectString(-1, strCountry));

	for (int scr=0; scr<SCR_COUNT; scr++)
	{
		m_bScreenLoaded[scr] = FALSE;
		m_nLanguage[scr] = 0;

		ActivateScreenControls((TYPE_OF_SCREEN)scr, FALSE);
	}

	for (int res=0; res<RES_COUNT; res++)
	{
		m_pScreens_Trx[res] = NULL;
		m_pScreens_Op[res] = NULL;
		m_pScreens_KeyMgr[res] = NULL;

		m_bShowTrxScreens[res] = FALSE;
		m_bShowOpScreens[res] = FALSE;
		m_bShowKeyMgrScreens[res] = FALSE;
	}

	m_bResourceReady = FALSE;

	// open 1024x600 window as default
	// show all op screens for now
	m_chkRes1024600_Trx.SetCheck(TRUE);		

	m_chkRes640480_Op.SetCheck(TRUE);
	m_chkRes720480_Op.SetCheck(TRUE);
	m_chkRes800600_Op.SetCheck(TRUE);
	m_chkRes1024600_Op.SetCheck(TRUE);
	m_chkRes1024768_Op.SetCheck(TRUE);
	m_chkRes1024768_W_Op.SetCheck(TRUE);

	m_chkRes1024600_KeyMgr.SetCheck(TRUE);

	m_btnLang_Trx.SetWindowText(GET_LANG_STRING(m_nLanguage[SCR_TRX]));
	m_btnLang_Op.SetWindowText(GET_LANG_STRING(m_nLanguage[SCR_OP]));
	m_btnLang_KeyMgr.SetWindowText(GET_LANG_STRING(m_nLanguage[SCR_KEYMGR]));

	(GetDlgItem(IDC_VARCMD))->SetWindowText(L"APTranAdv1");
	(GetDlgItem(IDC_VARDATA))->SetWindowText(L"c:\\qrcode.png");

	OnButtonClearData();
}

void CScreenViewerDlg::ReadChosenResolutions()
{
	m_bShowTrxScreens[RES_640_480] = m_chkRes640480_Trx.GetCheck();
	m_bShowTrxScreens[RES_800_600] = m_chkRes800600_Trx.GetCheck();
	m_bShowTrxScreens[RES_1024_600] = m_chkRes1024600_Trx.GetCheck();
	m_bShowTrxScreens[RES_1024_768] = m_chkRes1024768_Trx.GetCheck();
	m_bShowTrxScreens[RES_1024_768_W] = m_chkRes1024768_W_Trx.GetCheck();

	m_bShowOpScreens[RES_640_480] = m_chkRes640480_Op.GetCheck();
	m_bShowOpScreens[RES_720_480] = m_chkRes720480_Op.GetCheck();
	m_bShowOpScreens[RES_800_600] = m_chkRes800600_Op.GetCheck();
	m_bShowOpScreens[RES_1024_600] = m_chkRes1024600_Op.GetCheck();
	m_bShowOpScreens[RES_1024_768] = m_chkRes1024768_Op.GetCheck();
	m_bShowOpScreens[RES_1024_768_W] = m_chkRes1024768_W_Op.GetCheck();

	m_bShowKeyMgrScreens[RES_640_480] = m_chkRes640480_KeyMgr.GetCheck();
	m_bShowKeyMgrScreens[RES_800_600] = m_chkRes800600_KeyMgr.GetCheck();
	m_bShowKeyMgrScreens[RES_1024_600] = m_chkRes1024600_KeyMgr.GetCheck();
	m_bShowKeyMgrScreens[RES_1024_768] = m_chkRes1024768_KeyMgr.GetCheck();
}


// from: https://stackoverflow.com/questions/41088124/how-do-you-control-the-width-of-the-dropdown-listbox-in-a-combobox
int CScreenViewerDlg::GetRequiredComboDroppedWidth(CComboBox& rCombo)
{
	CString    str;
	CSize      sz;
	int        dx = 0;
	TEXTMETRIC tm;
	CDC*       pDC = rCombo.GetDC();
	CFont*     pFont = rCombo.GetFont();

	// Select the listbox font, save the old font
	if (pDC == NULL)
	{
		WriteStatus(_T("It shouldn't be happened..."));
	}

	CFont* pOldFont = pDC->SelectObject(pFont);
	// Get the text metrics for avg char width
	pDC->GetTextMetrics(&tm);

	for (int i = 0; i < rCombo.GetCount(); i++)
	{
		rCombo.GetLBText(i, str);
		sz = pDC->GetTextExtent(str);

		// Add the avg width to prevent clipping
		sz.cx += tm.tmAveCharWidth;

		if (sz.cx > dx)
			dx = sz.cx;
	}
	// Select the old font back into the DC
	pDC->SelectObject(pOldFont);
	rCombo.ReleaseDC(pDC);

	// Adjust the width for the vertical scroll bar and the left and right border.
	dx += ::GetSystemMetrics(SM_CXVSCROLL) + 2 * ::GetSystemMetrics(SM_CXEDGE);

	return dx;
}


void CScreenViewerDlg::OnBnClickedApState()
{
	// TODO: Add your control notification handler code here
	OnBtnApplyValues();
}

void CScreenViewerDlg::OnBnClickedSetVariable()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	CString strCmd, strData;
	(GetDlgItem(IDC_VARCMD))->GetWindowText(strCmd);
	(GetDlgItem(IDC_VARDATA))->GetWindowText(strData);

	CComboBox* pCbScrList = GetCBOScrListPtr(SCR_OP);
	CString strScrNum;
	pCbScrList->GetLBText(pCbScrList->GetCurSel(), strScrNum);

	for (int res=0; res<RES_COUNT; res++)
	{
		if (!IsResolutionAvailable(SCR_OP, (TYPE_OF_RESOLUTION)res))
			continue;

		CScrCtrl** pScrCtrl = GetScrCtrlPtr(SCR_OP, (TYPE_OF_RESOLUTION)res);
		if (*pScrCtrl == NULL)
		{
			WriteStatus(_T("OP screen not loaded!"));
			return;
		}

		(*pScrCtrl)->SetVariable(_T("PrevSetScreenNo"), strScrNum);
		(*pScrCtrl)->SetVariable(_T("APValue1"), ReadCurrentAPValues());
		(*pScrCtrl)->SetVariable(_T("APState1"), ReadCurrentAPStates());

		(*pScrCtrl)->SetVariable(L"APTranAdv", L"1");				// Number of Advertisement
		(*pScrCtrl)->SetVariable(L"APTranAdvTime", L"3600");		// Refresh Time (Second) => refresh every hour. (60*60)
		(*pScrCtrl)->SetVariable(strCmd, strData);

		(*pScrCtrl)->SetVariable(_T("APCenterMovie"), strScrNum);	

	}
}