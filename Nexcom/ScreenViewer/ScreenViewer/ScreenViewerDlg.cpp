// ScreenViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ScreenViewer.h"
#include "ScreenViewerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//*LEH
#define S_SAVE				_T("SAVE")
#define S_SELECT			_T("SELECT")
#define S_CANCEL			_T("CANCEL")
#define SCR_CMD_DELIMITER	_T("\r")
#define SCR_RES_DELIMITER		((char)0x1F)		// NH AIREAT 2008.05.20 scr res delimiter 변경 ':' -> 0x1F
////////////////////////////////////////

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
	m_nResolution = 1;
	m_strRootDir = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nVolLevel = 0;
}

void CScreenViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScreenViewerDlg)
	DDX_Control(pDX, IDC_OUTPUT, m_listOutPut);
	DDX_Control(pDX, IDC_COMBO_SCRNUM, m_comboScrNumber);
	DDX_Control(pDX, IDC_COMBO_LANGUAGE, m_comboLanguage);
	DDX_Radio(pDX, IDC_RADIO_640, m_nResolution);
	DDX_Text(pDX, IDC_EDIT_ROOT, m_strRootDir);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CScreenViewerDlg, CDialog)
	//{{AFX_MSG_MAP(CScreenViewerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_LOAD, OnBtnLoad)
	ON_BN_CLICKED(IDC_BTN_UNLOAD, OnBtnUnLoad)
	ON_BN_CLICKED(IDC_BTN_DIR, OnBtnDir)
	ON_BN_CLICKED(IDC_BTN_SHOW, OnBtnShow)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_NEXT, OnBtnNext)
	ON_BN_CLICKED(IDC_BTN_PREV, OnBtnPrev)
	ON_CBN_SELENDOK(IDC_COMBO_SCRNUM, OnSelendokComboScrnum)
	ON_BN_CLICKED(IDC_SETFOCUS, OnSetFocusScreen)
	ON_BN_CLICKED(IDC_SET_VOL_LEVEL, OnSetVolLevel)
	//}}AFX_MSG_MAP
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
	m_strRootDir = AfxGetApp()->GetProfileString(_T("Set"), _T("Folder"), NULL);
	m_strScreenRootFolder = m_strRootDir;
	m_comboLanguage.SetCurSel(0);
	((CButton*)GetDlgItem(IDC_APSTATE_1))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_2))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_3))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_4))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_5))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_6))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_7))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_8))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_9))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_10))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_11))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_12))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_13))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_14))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_15))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_16))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_17))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_18))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_19))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_APSTATE_20))->SetCheck(1);

	((CButton*)GetDlgItem(IDC_CHECK50))->SetCheck(50);

	(GetDlgItem(IDC_FOCUS_NUM))->SetWindowText(_T("0"));

	UpdateData(FALSE);

	SetTimer(1000, 100, NULL);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
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

void CScreenViewerDlg::OnBtnLoad() 
{
	UpdateData(TRUE);

	m_strScreenRootFolder = m_strRootDir;
	AfxGetApp()->WriteProfileString(_T("Setting"), _T("Folder"), m_strRootDir);
	// TODO: Add your control notification handler code here

	if (m_ScrCtrl.GetState())
	{
		MessageBox(_T("이미 Load 되어 있습니다."));
		return;
	}

	if (m_nResolution == 0)
		m_ScrCtrl.Initialize(::AfxGetInstanceHandle(), 1024, 768, _T("EAGLECE_ATM"), m_strScreenRootFolder);
	else if (m_nResolution == 1)
		m_ScrCtrl.Initialize(::AfxGetInstanceHandle(), 1280, 800, _T("EAGLECE_ATM"), m_strScreenRootFolder);
	else
	{
		MessageBox(_T("해상도를 선택해 주십시오"));
		return;
	}
//	m_ScrCtrl.Initialize(::AfxGetInstanceHandle(), 0, 0, _T("EAGLECE_ATM"), m_strScreenRootFolder);
	
	if (!m_ScrCtrl.GetState())
	{
		m_ScrCtrl.DeInitalize();
		MessageBox(_T("화면 Load에 실패하였습니다."));
		return;
	}

	CString strScrNumber;
	int nScreenCount = m_ScrCtrl.GetScreenCount();
	m_comboScrNumber.InitStorage(nScreenCount, 10);

	int count = m_ScrCtrl.GetScreenCount();
	strScrNumber = m_ScrCtrl.GetFirstScreenNumber();
	while(count > 0)
	{
		m_comboScrNumber.AddString(strScrNumber);
		count--;

		if (count <= 0)
			break;

		strScrNumber = m_ScrCtrl.GetNextScreenNumber();
	}

	RECT rc = m_ScrCtrl.GetPosValue4Camera();

	//*LEHTEST
	CString strTemp;
	strTemp.Format(_T("rc.left:[%d], rc.top:[%d], rc.right:[%d], rc.bottom:[%d]"), rc.left, rc.top, rc.right, rc.bottom);
//	AfxMessageBox(strTemp);
	////////////////////
}

void CScreenViewerDlg::OnBtnUnLoad() 
{
	// TODO: Add your control notification handler code here
	m_comboScrNumber.ResetContent();
	m_ScrCtrl.DeInitalize();
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
		
// 		WIN32_FIND_DATA	FileData;
// 		HANDLE			hSearch = NULL;
// 		CString			strSearchFile;
// 		
// 		// find 800_480 folder
// 		{
// 			strSearchFile.Format(_T("%s\\800_480"), pszPathName);
// 			
// 			hSearch = FindFirstFile(strSearchFile, &FileData);
// 			if (hSearch == INVALID_HANDLE_VALUE)
// 				GetDlgItem(IDC_RADIO_800)->EnableWindow(FALSE);
// 			else
// 				GetDlgItem(IDC_RADIO_800)->EnableWindow(TRUE);
// 			
// 			FindClose(hSearch);
// 		}

		m_strScreenRootFolder = pszPathName;
		GetDlgItem(IDC_EDIT_ROOT)->SetWindowText(m_strScreenRootFolder);
		AfxGetApp()->WriteProfileString(_T("Set"), _T("Folder"), m_strScreenRootFolder);
    }

	m_strScreenRootFolder.ReleaseBuffer();
}

void CScreenViewerDlg::OnBtnShow() 
{
	// TODO: Add your control notification handler code here
	int		nLocaleMode = 0;
	CString strScrNum;

	if (m_comboLanguage.GetCurSel() == -1)
	{
		MessageBox(_T("언어를 선택하여 주십시오"));
		return;
	}
	else
	{
		nLocaleMode = m_comboLanguage.GetCurSel() + 1;
	}
	
	if (m_comboScrNumber.GetCurSel() == -1)
	{
		MessageBox(_T("화면을 선택하여 주십시오"));
		return;
	}
	else
	{
		m_comboScrNumber.GetLBText(m_comboScrNumber.GetCurSel(), strScrNum);
	}

	int i;
	BOOL	bApState[20];
	BOOL	bApADAMode;
	CString	strApValue[20];
	CString strAPEditState[10];
	CString strTemp;

	{
		bApState[0] = ((CButton*)GetDlgItem(IDC_APSTATE_1))->GetCheck();
		bApState[1] = ((CButton*)GetDlgItem(IDC_APSTATE_2))->GetCheck();
		bApState[2] = ((CButton*)GetDlgItem(IDC_APSTATE_3))->GetCheck();
		bApState[3] = ((CButton*)GetDlgItem(IDC_APSTATE_4))->GetCheck();
		bApState[4] = ((CButton*)GetDlgItem(IDC_APSTATE_5))->GetCheck();
		bApState[5] = ((CButton*)GetDlgItem(IDC_APSTATE_6))->GetCheck();
		bApState[6] = ((CButton*)GetDlgItem(IDC_APSTATE_7))->GetCheck();
		bApState[7] = ((CButton*)GetDlgItem(IDC_APSTATE_8))->GetCheck();
		bApState[8] = ((CButton*)GetDlgItem(IDC_APSTATE_9))->GetCheck();
		bApState[9] = ((CButton*)GetDlgItem(IDC_APSTATE_10))->GetCheck();
		bApState[10] = ((CButton*)GetDlgItem(IDC_APSTATE_11))->GetCheck();
		bApState[11] = ((CButton*)GetDlgItem(IDC_APSTATE_12))->GetCheck();
		bApState[12] = ((CButton*)GetDlgItem(IDC_APSTATE_13))->GetCheck();
		bApState[13] = ((CButton*)GetDlgItem(IDC_APSTATE_14))->GetCheck();
		bApState[14] = ((CButton*)GetDlgItem(IDC_APSTATE_15))->GetCheck();
		bApState[15] = ((CButton*)GetDlgItem(IDC_APSTATE_16))->GetCheck();
		bApState[16] = ((CButton*)GetDlgItem(IDC_APSTATE_17))->GetCheck();
		bApState[17] = ((CButton*)GetDlgItem(IDC_APSTATE_18))->GetCheck();
		bApState[18] = ((CButton*)GetDlgItem(IDC_APSTATE_19))->GetCheck();
		bApState[19] = ((CButton*)GetDlgItem(IDC_APSTATE_20))->GetCheck();

		bApADAMode = ((CButton*)GetDlgItem(IDC_CHECK50))->GetCheck();

		(GetDlgItem(IDC_APVALUE_1))->GetWindowText(strApValue[0]);
		(GetDlgItem(IDC_APVALUE_2))->GetWindowText(strApValue[1]);
		(GetDlgItem(IDC_APVALUE_3))->GetWindowText(strApValue[2]);
		(GetDlgItem(IDC_APVALUE_4))->GetWindowText(strApValue[3]);
		(GetDlgItem(IDC_APVALUE_5))->GetWindowText(strApValue[4]);
		(GetDlgItem(IDC_APVALUE_6))->GetWindowText(strApValue[5]);
		(GetDlgItem(IDC_APVALUE_7))->GetWindowText(strApValue[6]);
		(GetDlgItem(IDC_APVALUE_8))->GetWindowText(strApValue[7]);
		(GetDlgItem(IDC_APVALUE_9))->GetWindowText(strApValue[8]);
		(GetDlgItem(IDC_APVALUE_10))->GetWindowText(strApValue[9]);
		(GetDlgItem(IDC_APVALUE_11))->GetWindowText(strApValue[10]);
		(GetDlgItem(IDC_APVALUE_12))->GetWindowText(strApValue[11]);
		(GetDlgItem(IDC_APVALUE_13))->GetWindowText(strApValue[12]);
		(GetDlgItem(IDC_APVALUE_14))->GetWindowText(strApValue[13]);
		(GetDlgItem(IDC_APVALUE_15))->GetWindowText(strApValue[14]);
		(GetDlgItem(IDC_APVALUE_16))->GetWindowText(strApValue[15]);
		(GetDlgItem(IDC_APVALUE_17))->GetWindowText(strApValue[16]);
		(GetDlgItem(IDC_APVALUE_18))->GetWindowText(strApValue[17]);
		(GetDlgItem(IDC_APVALUE_19))->GetWindowText(strApValue[18]);
		(GetDlgItem(IDC_APVALUE_20))->GetWindowText(strApValue[19]);

		(GetDlgItem(IDC_APEDITSTATE_1))->GetWindowText(strAPEditState[0]);
		(GetDlgItem(IDC_APEDITSTATE_2))->GetWindowText(strAPEditState[1]);
		(GetDlgItem(IDC_APEDITSTATE_3))->GetWindowText(strAPEditState[2]);
		(GetDlgItem(IDC_APEDITSTATE_4))->GetWindowText(strAPEditState[3]);
		(GetDlgItem(IDC_APEDITSTATE_5))->GetWindowText(strAPEditState[4]);
		(GetDlgItem(IDC_APEDITSTATE_6))->GetWindowText(strAPEditState[5]);
		(GetDlgItem(IDC_APEDITSTATE_7))->GetWindowText(strAPEditState[6]);
		(GetDlgItem(IDC_APEDITSTATE_8))->GetWindowText(strAPEditState[7]);
		(GetDlgItem(IDC_APEDITSTATE_9))->GetWindowText(strAPEditState[8]);
		(GetDlgItem(IDC_APEDITSTATE_10))->GetWindowText(strAPEditState[9]);
		
		for (i=0; i < 20; i++)
			strApValue[i].Replace(_T("\\n"), _T("\n"));
	}

	m_ScrCtrl.SetScreenLocale(nLocaleMode);
	
	m_ScrCtrl.SetVariable(_T("PrevSetScreenNo"), strScrNum);

	strTemp = _T("");
	for(i = 0; i<20; i++)
	{
		if (i != 0)
			strTemp += _T("\r");

		strApValue[i].Replace(_T("|"), _T("\r"));

		strTemp += strApValue[i];
	}

	m_ScrCtrl.SetVariable(_T("APValue1"), strTemp);
	//m_ScrCtrl.SetVariable(_T("APState1"), strTemp);

	strTemp = _T("");
	for(i=0; i<10; i++)
	{
		if (i != 0)
			strTemp += _T("\r");
		
		strTemp += strAPEditState[i];
	}

	m_ScrCtrl.SetVariable(_T("APEditState1"), strTemp);

	strTemp = _T("");
	for(i=0; i<20; i++)
	{
		if (i != 0)
			strTemp += _T("\r");

		if (bApState[i])
			strTemp += _T("on");
		else
			strTemp += _T("off");
	}

	// ADA Control을 위해 20~49번까지 off로 처리
	for(i=20; i<49; i++)
	{
		strTemp += _T("\r");
		strTemp += _T("off");
	}

	strTemp += _T("\r");
	if (bApADAMode == TRUE)
		strTemp += _T("on");
	else
		strTemp += _T("off");

	m_ScrCtrl.SetVariable(_T("APState1"), strTemp);

	(GetDlgItem(IDC_FOCUS_NUM))->GetWindowText(m_strFocusNum);

	m_strFocusNum.TrimLeft();
	m_strFocusNum.TrimRight();

	if (m_strFocusNum != _T("0"))
	{
		CString strCommand, strValue;

		strValue.Format(_T("APValue%s"), m_strFocusNum);
		strCommand = _T("APSetFocus");

		m_ScrCtrl.SetVariable(strCommand, strValue);
	}

/*	if(strScrNum == _T("507"))
	{
		ChangeVolumnScreen();
	}
	else if(strScrNum == _T("509") || strScrNum == _T("510") || strScrNum == _T("511"))
	{
		m_nCurrentFocusIndex = 1;
		m_ScrCtrl.SetVariable(_T("APSetFocus"), _T("APValue1"));
		m_ScrCtrl.SetVariable(_T("APValue1"), _T("106.244.224.181"));
		m_ScrCtrl.SetVariable(_T("APKeyDisable"), _T("APValue1"));
	}
	else if(strScrNum == _T("513"))
	{
		// Set Timeout
		m_ScrCtrl.SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		m_ScrCtrl.SetVariable(_T("APMaxTime"), _T("30"));
	}
	else if(strScrNum == _T("515"))
	{
		CString strCmd = _T("");
		m_nCurrentFocusIndex = 4;
		m_ScrCtrl.SetVariable(_T("APSetFocus"), _T("APValue4"));

		strCmd.Format(_T("%s%d"), _T("APEditState"), 1);
		m_ScrCtrl.SetVariable(strCmd, _T("select"));

		strCmd.Format(_T("%s%d"), _T("APEditState"), 2);
		m_ScrCtrl.SetVariable(strCmd, _T("ENABLE"));

		strCmd.Format(_T("%s%d"), _T("APEditState"), 3);
		m_ScrCtrl.SetVariable(strCmd, _T("disable"));

		strCmd.Format(_T("%s%d"), _T("APValue"), 4);
		m_ScrCtrl.SetVariable(strCmd, _T("DISABLE"));
	}
	else if(strScrNum == _T("901"))
	{
		CString strCommand = _T("");
		CString strMaxInChar = _T("");

//		m_nCurrentFocusIndex = 1;
//		m_ScrCtrl.SetVariable(_T("APSetFocus"), _T("APValue1"));
		
		// Title
		strCommand.Format(_T("%s%d"), _T("APValue"), 1);
		m_ScrCtrl.SetVariable(strCommand, strApValue[0]);
		
		// Current Value
		strCommand.Format(_T("%s%d"),  _T("APValue"), 3);
		m_ScrCtrl.SetVariable(strCommand, strApValue[2]);

		// Set MaxInChar
		strMaxInChar.Format(_T("%s%d"), _T("MaxInChar:"), 20);
		m_ScrCtrl.SetVariable(strCommand, strMaxInChar);
	}*/
	
	
	m_ScrCtrl.SetVariable(_T("APCenterMovie"), strScrNum);	

	// output

	m_listOutPut.AddString(_T(""));

	m_strOutput.Format(_T("[SetScreenLocale] -- [%d]"), nLocaleMode);
	m_listOutPut.AddString(m_strOutput);
	m_listOutPut.SetCurSel(m_listOutPut.GetCount()-1);
	
	m_strOutput.Format(_T("[APCenterMovie] -- [%s]"), strScrNum);
	m_listOutPut.AddString(m_strOutput);
	m_listOutPut.SetCurSel(m_listOutPut.GetCount()-1);
}

void CScreenViewerDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	
	{
		((CButton*)GetDlgItem(IDC_APSTATE_1))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_2))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_3))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_4))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_5))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_6))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_7))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_8))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_9))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_10))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_11))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_12))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_13))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_14))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_15))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_16))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_17))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_18))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_19))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_APSTATE_20))->SetCheck(0);

		(GetDlgItem(IDC_APVALUE_1))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_2))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_3))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_4))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_5))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_6))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_7))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_8))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_9))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_10))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_11))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_12))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_13))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_14))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_15))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_16))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_17))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_18))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_19))->SetWindowText(_T(""));
		(GetDlgItem(IDC_APVALUE_20))->SetWindowText(_T(""));
	}
}

#define UPPERCASE_MODE		_T("UPPERCASE")
#define LOWERCASE_MODE		_T("LOWERCASE")

void CScreenViewerDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	BOOL bResult = FALSE;
	CString strReturn = _T("");
	CString strNewText = _T("");
//	CString strCurrText = _T("106.244.224.181");	//*LEHTEST
	CString strCurrText = _T("");	//*LEHTEST
	CString strScreenNumber = _T("");	//*LEH
	CString strTitleID = _T("");	//*LEH
	CString strValue = _T("");
	CString strCmd = _T("");
	CString strTitle = _T("");
	static CString strTLS = _T("DISABLE");
	int nMaxFocusIndex = 4;

	if (nIDEvent == 1000)
	{
		if (m_ScrCtrl.GetUserInputCheck())
		{
			CString strName, strData;
			m_ScrCtrl.GetUserInputData(strName, strData);
			strData.Replace(0x1F, '^');
			m_strOutput.Format(_T("[FSCommand] -- [%s]-[%s]"), strName, strData);
			m_listOutPut.AddString(m_strOutput);
			m_listOutPut.SetCurSel(m_listOutPut.GetCount()-1);

		/*	if(strData.CompareNoCase(S_SELECT) == 0)
			{
				strScreenNumber = m_ScrCtrl.GetCurrentScreenNumber();
				strTitleID = _T("50702");	//*LEHTEST
			//	bResult = Operator_Input_Text(strTitleID, strCurrText, strNewText, _T("UPPERCASE"));
				if(1 == m_nCurrentFocusIndex)
					strTitle = _T("CBX #1");
				else if(2 == m_nCurrentFocusIndex)
					strTitle = _T("COUNTER");
				else if(3 == m_nCurrentFocusIndex)
					strTitle = _T("TCP/IP");
				else if(4 == m_nCurrentFocusIndex)
				{
					if(_T("ENABLE") == strTLS)
						strTLS = _T("DISABLE");
					else
						strTLS = _T("ENABLE");
				}

				if(1 == m_nCurrentFocusIndex)
					bResult = Operator_Input_Text(strTitle, strCurrText, strNewText, 15, LOWERCASE_MODE);
				//	bResult = Operator_Input_Number(strTitle, strCurrText, strNewText, 3, _T("2"));
				else if(2 == m_nCurrentFocusIndex)
					bResult = Operator_Input_Number(strTitle, strCurrText, strNewText, 2, _T("0"));
				else if(3 == m_nCurrentFocusIndex)
					bResult = Operator_Input_Number(strTitle, strCurrText, strNewText, 2, _T("0"));

				if( (1 == m_nCurrentFocusIndex) || (2 == m_nCurrentFocusIndex) || (3 == m_nCurrentFocusIndex) )
				{
					if(TRUE == bResult)
						strReturn = strNewText;
					else
						strReturn = strCurrText;

					//*LEHTEST
					CString strTemp;
					strTemp.Format(_T("ScreenNumber:[%s]"), strScreenNumber);
				//	AfxMessageBox(strTemp);
					/////////////////////

					m_ScrCtrl.SetVariable(_T("PrevSetScreenNo"), strScreenNumber);

					strCmd.Format(_T("%s%d"), _T("APValue"), m_nCurrentFocusIndex);
					m_ScrCtrl.SetVariable(strCmd, strReturn);
					strValue.Format(_T("%s%d"), _T("APValue"), m_nCurrentFocusIndex);
					m_ScrCtrl.SetVariable(_T("APSetFocus"), strValue);

					m_ScrCtrl.SetVariable(_T("APCenterMovie"), strScreenNumber);
				}
				else
				{
					if(strTLS == _T("DISABLE"))
					{
						strCmd.Format(_T("%s%d"), _T("APEditState"), 1);
						m_ScrCtrl.SetVariable(strCmd, _T("enable"));

						strCmd.Format(_T("%s%d"), _T("APEditState"), 2);
						m_ScrCtrl.SetVariable(strCmd, _T("enable"));

						strCmd.Format(_T("%s%d"), _T("APEditState"), 3);
						m_ScrCtrl.SetVariable(strCmd, _T("disable"));
					}
					else
					{
						strCmd.Format(_T("%s%d"), _T("APEditState"), 1);
						m_ScrCtrl.SetVariable(strCmd, _T("disable"));

						strCmd.Format(_T("%s%d"), _T("APEditState"), 2);
						m_ScrCtrl.SetVariable(strCmd, _T("disable"));

						strCmd.Format(_T("%s%d"), _T("APEditState"), 3);
						m_ScrCtrl.SetVariable(strCmd, _T("enable"));
					}

					strCmd.Format(_T("%s%d"), _T("APValue"), m_nCurrentFocusIndex);
					m_ScrCtrl.SetVariable(strCmd, strTLS);
					m_ScrCtrl.SetVariable(_T("APUpdate"), _T("SCREEN"));
				}
			}
			else if(strData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;
				if(m_nCurrentFocusIndex < 1)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				if( (strTLS == _T("DISABLE")) && (3 == m_nCurrentFocusIndex) )
					m_nCurrentFocusIndex--;
				else if( (strTLS == _T("ENABLE")) && (2 == m_nCurrentFocusIndex) )
					m_nCurrentFocusIndex = nMaxFocusIndex;

				if(1 == m_nCurrentFocusIndex)
				{
					strCmd.Format(_T("%s%d"), _T("APEditState"), 1);
					m_ScrCtrl.SetVariable(strCmd, _T("select"));

					strCmd.Format(_T("%s%d"), _T("APEditState"), 2);
					m_ScrCtrl.SetVariable(strCmd, _T("enable"));

					strCmd.Format(_T("%s%d"), _T("APEditState"), 3);
					m_ScrCtrl.SetVariable(strCmd, _T("disable"));
				}
				else if(2 == m_nCurrentFocusIndex)
				{
					strCmd.Format(_T("%s%d"), _T("APEditState"), 1);
					m_ScrCtrl.SetVariable(strCmd, _T("enable"));
					
					strCmd.Format(_T("%s%d"), _T("APEditState"), 2);
					m_ScrCtrl.SetVariable(strCmd, _T("select"));
					
					strCmd.Format(_T("%s%d"), _T("APEditState"), 3);
					m_ScrCtrl.SetVariable(strCmd, _T("disable"));
				}
				else if(3 == m_nCurrentFocusIndex)
				{
					strCmd.Format(_T("%s%d"), _T("APEditState"), 1);
					m_ScrCtrl.SetVariable(strCmd, _T("disable"));
					
					strCmd.Format(_T("%s%d"), _T("APEditState"), 2);
					m_ScrCtrl.SetVariable(strCmd, _T("disable"));
					
					strCmd.Format(_T("%s%d"), _T("APEditState"), 3);
					m_ScrCtrl.SetVariable(strCmd, _T("select"));
				}
				else
				{
					if(strTLS == _T("DISABLE"))
					{
						strCmd.Format(_T("%s%d"), _T("APEditState"), 1);
						m_ScrCtrl.SetVariable(strCmd, _T("enable"));

						strCmd.Format(_T("%s%d"), _T("APEditState"), 2);
						m_ScrCtrl.SetVariable(strCmd, _T("ENABLE"));

						strCmd.Format(_T("%s%d"), _T("APEditState"), 3);
						m_ScrCtrl.SetVariable(strCmd, _T("disable"));
					}
					else
					{
						strCmd.Format(_T("%s%d"), _T("APEditState"), 1);
						m_ScrCtrl.SetVariable(strCmd, _T("disable"));

						strCmd.Format(_T("%s%d"), _T("APEditState"), 2);
						m_ScrCtrl.SetVariable(strCmd, _T("disable"));

						strCmd.Format(_T("%s%d"), _T("APEditState"), 3);
						m_ScrCtrl.SetVariable(strCmd, _T("enable"));
					}
				}

				strValue.Format(_T("%s%d"), _T("APValue"), m_nCurrentFocusIndex);
				m_ScrCtrl.SetVariable(_T("APSetFocus"), strValue);
				m_ScrCtrl.SetVariable(_T("APKeyDisable"), strValue);

				m_ScrCtrl.SetVariable(_T("APUpdate"), _T("SCREEN"));
			}
			else if(strData == _T("NEXT"))
			{
				m_nCurrentFocusIndex++;
				if(m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 1;

				if( (strTLS == _T("DISABLE")) && (3 == m_nCurrentFocusIndex) )
					m_nCurrentFocusIndex = nMaxFocusIndex;
				else if( (strTLS == _T("ENABLE")) && (1 == m_nCurrentFocusIndex) )
					m_nCurrentFocusIndex = 3;

				if(1 == m_nCurrentFocusIndex)
				{
					strCmd.Format(_T("%s%d"), _T("APEditState"), 1);
					m_ScrCtrl.SetVariable(strCmd, _T("select"));
					
					strCmd.Format(_T("%s%d"), _T("APEditState"), 2);
					m_ScrCtrl.SetVariable(strCmd, _T("enable"));
					
					strCmd.Format(_T("%s%d"), _T("APEditState"), 3);
					m_ScrCtrl.SetVariable(strCmd, _T("disable"));
				}
				else if(2 == m_nCurrentFocusIndex)
				{
					strCmd.Format(_T("%s%d"), _T("APEditState"), 1);
					m_ScrCtrl.SetVariable(strCmd, _T("enable"));
					
					strCmd.Format(_T("%s%d"), _T("APEditState"), 2);
					m_ScrCtrl.SetVariable(strCmd, _T("select"));
					
					strCmd.Format(_T("%s%d"), _T("APEditState"), 3);
					m_ScrCtrl.SetVariable(strCmd, _T("disable"));
				}
				else if(3 == m_nCurrentFocusIndex)
				{
					strCmd.Format(_T("%s%d"), _T("APEditState"), 1);
					m_ScrCtrl.SetVariable(strCmd, _T("disable"));
					
					strCmd.Format(_T("%s%d"), _T("APEditState"), 2);
					m_ScrCtrl.SetVariable(strCmd, _T("disable"));
					
					strCmd.Format(_T("%s%d"), _T("APEditState"), 3);
					m_ScrCtrl.SetVariable(strCmd, _T("select"));
				}
				else
				{
					if(strTLS == _T("DISABLE"))
					{
						strCmd.Format(_T("%s%d"), _T("APEditState"), 1);
						m_ScrCtrl.SetVariable(strCmd, _T("enable"));
						
						strCmd.Format(_T("%s%d"), _T("APEditState"), 2);
						m_ScrCtrl.SetVariable(strCmd, _T("ENABLE"));
						
						strCmd.Format(_T("%s%d"), _T("APEditState"), 3);
						m_ScrCtrl.SetVariable(strCmd, _T("disable"));
					}
					else
					{
						strCmd.Format(_T("%s%d"), _T("APEditState"), 1);
						m_ScrCtrl.SetVariable(strCmd, _T("disable"));
						
						strCmd.Format(_T("%s%d"), _T("APEditState"), 2);
						m_ScrCtrl.SetVariable(strCmd, _T("disable"));
						
						strCmd.Format(_T("%s%d"), _T("APEditState"), 3);
						m_ScrCtrl.SetVariable(strCmd, _T("enable"));
					}
				}

				strValue.Format(_T("%s%d"), _T("APValue"), m_nCurrentFocusIndex);
				m_ScrCtrl.SetVariable(_T("APSetFocus"), strValue);
				m_ScrCtrl.SetVariable(_T("APKeyDisable"), strValue);
				m_ScrCtrl.SetVariable(_T("APUpdate"), _T("SCREEN"));
			}
			else if(strData.CompareNoCase(_T("TEST")) == 0)
			{
				m_ScrCtrl.SetVariable(_T("APState1"), _T("on"));
				m_ScrCtrl.SetVariable(_T("APUpdate"), _T(""));
			}
			else if( (strData.CompareNoCase(_T("UP")) == 0) || (strData.CompareNoCase(_T("DOWN")) == 0))
			{
				if(strData.CompareNoCase(_T("UP")) == 0)
				{
					m_nVolLevel++;
					if(m_nVolLevel > 10)
						m_nVolLevel = 10;
				}
				else
				{
					m_nVolLevel--;
					if(m_nVolLevel < 0)
						m_nVolLevel = 0;
				}

				ChangeVolumnScreen();
			}*/
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

void CScreenViewerDlg::OnBtnPrev() 
{
	// TODO: Add your control notification handler code here
	int nIndex = m_comboScrNumber.GetCurSel();
	if (nIndex-1 >= 0)
	{
		nIndex--;
		m_comboScrNumber.SetCurSel(nIndex);
	}

	OnBtnShow();
}

void CScreenViewerDlg::OnBtnNext() 
{
	// TODO: Add your control notification handler code here
	int nIndex = m_comboScrNumber.GetCurSel();
	if (nIndex+1 < m_comboScrNumber.GetCount())
	{
		nIndex++;
		m_comboScrNumber.SetCurSel(nIndex);
	}

	OnBtnShow();
}

void CScreenViewerDlg::OnSelendokComboScrnum() 
{
	// TODO: Add your control notification handler code here
	OnBtnShow();
}
/*
//*LEH
void CScreenViewerDlg::SetFocusControl(CString strMoveType)
{
	CString strAPValue = _T("");

	if(strMoveType.CompareNoCase(S_PREVMOVE) == 0)
	{
		g_nFocusIndex--;
		if(g_nFocusIndex <= 0)
			g_nFocusIndex = 4;
	}
	else
	{
		g_nFocusIndex++;
		if(g_nFocusIndex > 4)
			g_nFocusIndex = 1;
	}

	strAPValue.Format(_T("APValue%d"), g_nFocusIndex);
	m_ScrCtrl.SetVariable(_T("APSetFocus"), strAPValue);

	m_ScrCtrl.SetVariable(_T("APUpdate"), _T("SCREEN"));

	g_strCurrentEditbox= strAPValue;
}
*/

#define		S_TIMEOVER				_T("TIMEOVER")
#define		SUPERVISOR_SCREEN_TIMEOUT	_T("60")
#define S_ENTER				_T("ENTER")

int		NH_SplitString(CString cstrSrc, CString cstrSplit, CStringArray& acstrRet);

BOOL CScreenViewerDlg::Operator_Input_Text(CString strTitle, CString strInText, CString &strOutText, int nMaxInNumber/*=40*/, CString strInputDefaultMode/*=_T("NUMBER")*/)
{
	BOOL bShowScreen = TRUE;
	CString strCommand = _T("");
	CString strScrOutName = _T("");
	CString strScrOutData = _T("");
	CString strSaveData = _T("");
	CStringArray strTempArray;
	CString strGuide = _T("");
	CString strMaxInChar = _T("");

	int nStartIndex = 0;
	int nEndIndex = 0;

	while(TRUE)
	{
		if(TRUE == bShowScreen)
		{
			// 1. PrevSet
			m_ScrCtrl.SetVariable(_T("PrevSetScreenNo"), _T("900"));

			// 2. Title
		//	strTitle.Format(_T("APTextID:%s"), strTitleID);
			strCommand.Format(_T("%s%d"), _T("APValue"), 1);
			m_ScrCtrl.SetVariable(strCommand, strTitle);

			// Value Guide
			if(nMaxInNumber < 40)
			{
				strCommand.Format(_T("%s%d"), _T("APValue"), 2);
				strGuide.Format(_T("MAX INPUT LENGTH : %d"), nMaxInNumber);
				m_ScrCtrl.SetVariable(strCommand, strGuide);
			}

			// Set MaxInChar
			strCommand.Format(_T("%s%d"), _T("APValue"), 3);
			strMaxInChar.Format(_T("%s%d"), _T("MaxInChar:"), nMaxInNumber);
			m_ScrCtrl.SetVariable(strCommand, strMaxInChar);

			// 3. TableEditBox
			m_ScrCtrl.SetVariable(strCommand, strInText);

			// 4. SetInputMode
			m_ScrCtrl.SetVariable(_T("APSetInputMode"), strInputDefaultMode);

			// 5. Virtual KeyPad Button 
			m_ScrCtrl.SetVariable(_T("APSetFocus"), strInputDefaultMode);

			// 6. Timeout Set
			m_ScrCtrl.SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
			m_ScrCtrl.SetVariable(_T("APMaxTime"), SUPERVISOR_SCREEN_TIMEOUT);

			// 7. Display Screen
			m_ScrCtrl.SetVariable(_T("APCenterMovie"), _T("900"));

			bShowScreen = FALSE;
		}

		///////////////////////////////////
		// GET KEY STRING
		while (TRUE)
		{
			if (m_ScrCtrl.GetUserInputCheck() == TRUE)
			{
				m_ScrCtrl.GetUserInputData(strScrOutName, strScrOutData);
				break;
			}

			// 대기
			Sleep(100);
		}

//		CUtil::ParsingStringToStringArray(strInputValue, (CString)SCR_RES_DELIMITER, strTempArray);
		NH_SplitString(strScrOutData, SCR_RES_DELIMITER, strTempArray);

		if(strTempArray[0] == S_SAVE)
		{
			strOutText = strTempArray[1];
			return TRUE;
		}
		else if (strTempArray[0] == S_TIMEOVER)
		{
			strOutText = S_TIMEOVER;
			return FALSE;
		}
		else
		{
			strOutText = strInText;
			return FALSE;
		}

		Sleep(100);
	}

	return FALSE;
}

int	NH_SplitString(CString cstrSrc, CString cstrSplit, CStringArray& acstrRet)
{
	CString cstrTemp = cstrSrc;
	CString cstrData("");
	int		nIndex=0, nCnt=0;

	acstrRet.RemoveAll();										// 20030703 : 초기화하지 않으면 원래배열의 뒤에 추가됨
	
	while (TRUE)												// 20031030 : 무조건처리로변경
	{
		nIndex = cstrTemp.Find(cstrSplit);
		if(nIndex == -1)
		{
			acstrRet.Add(cstrTemp);								// 20031030 : 무조건처리로변경
			nCnt++;												// 20031030 : 마지막처리추가
// [CS#1] NH AIREAT 2008.3.14
			//return nCnt;
			break;
// end of [CS#1]
		}
		else
		{
			cstrData = cstrTemp.Left(nIndex);
			cstrTemp = cstrTemp.Right(cstrTemp.GetLength() - (nIndex + cstrSplit.GetLength()));  
			acstrRet.Add(cstrData);
			nCnt++;
		}
	}
	return nCnt;
}

void CScreenViewerDlg::OnSetFocusScreen() 
{
	CString strFocusNum;
	CString strCommand, strValue;

	(GetDlgItem(IDC_FOCUS_NUM))->GetWindowText(m_strFocusNum);

	//m_ScrCtrl.SetVariable(_T("PrevSetScreenNo"), _T("681"));

	m_strFocusNum.TrimLeft();
	m_strFocusNum.TrimRight();
	strValue.Format(_T("APValue%s"), m_strFocusNum);
	strCommand = _T("APSetFocus");

	m_ScrCtrl.SetVariable(strCommand, strValue);

//	strCommand = strValue;
//	strValue = _T("12345");
//
//	m_ScrCtrl.SetVariable(strCommand, strValue);

	//m_ScrCtrl.SetVariable(_T("APCenterMovie"), _T("681"));
	m_ScrCtrl.SetVariable(_T("APUpdate"), _T("SCREEN"));	
}
void CScreenViewerDlg::OnSetVolLevel() 
{
/*	CString strVolLevel = _T("");
	CString strResult = _T("");
	CString strInText = _T("123");
	CString strNewText = _T("");
	(GetDlgItem(IDC_VOL_LEVEL))->GetWindowText(strVolLevel);

	m_nVolLevel = _ttoi(strVolLevel);

	if(0 == m_nVolLevel)
	{
	//	Operator_Input_Number(_T("COUNTER"), _T("1"), strNewText, 2, _T("0"));
	//	Operator_Processing_Screen(_T(""), TRUE, _T("XXXXXX"));
		Operator_Input_Text(_T("SERIAL NO."), _T("A BC   "), strNewText, 10, _T("NUMBER"));
		strResult = strNewText;
	}
	else if(1 == m_nVolLevel)
	{
		Operator_Input_Number(_T("INPUT MAX WITHDRAWAL"), _T("999"), strNewText, 3, _T("1"));
	//	Operator_Processing_Screen(_T("PLEASE WAIT A MINUTE"));
	//	Operator_Input_Text(_T("SEARCH ERRORCODE"), _T(""), strNewText, 5, _T("LOWERCASE"));
		strResult = strNewText;
	}
	else if(2 == m_nVolLevel)
	{
		Operator_Input_Number(_T("CBX #1"), _T("123"), strNewText, 4, _T("2"));
		strResult = strNewText;
	}
	else if(3 == m_nVolLevel)
	{
		if(Operator_Input_Number(_T("CBX #1"), strInText, strNewText))
			strResult = strNewText;
		else
			strResult = strInText;
	}

	AfxMessageBox(strResult);
*/
}

void CScreenViewerDlg::ChangeVolumnScreen()
{
	CString strButtonInfo = _T("");
	int i=0;

	if(m_nVolLevel > 0)
	{
		strButtonInfo = _T("on");
		strButtonInfo += SCR_CMD_DELIMITER;
	}

	for(i=1; i<m_nVolLevel; i++)
	{
		strButtonInfo += _T("on");
		strButtonInfo += SCR_CMD_DELIMITER;
	}

	for(i=m_nVolLevel; i<10; i++)
	{
		strButtonInfo += _T("off");
		strButtonInfo += SCR_CMD_DELIMITER;
	}

	m_ScrCtrl.SetVariable(_T("APState1"), strButtonInfo);
	m_ScrCtrl.SetVariable(_T("APUpdate"), _T("SCREEN"));
}

BOOL CScreenViewerDlg::Operator_Input_Number(CString strTitleText, CString strInText, CString &strOutText, int nMaxInNumber /*=20*/, CString strInputType /*=_T("0")*/)
{
	CString strCommand;
	CString strScrOutName;
	CString strScrOutData;
	int		nResultCount = 0;
	CStringArray strTempArray;
	CString strMaxInChar = _T("");
	CString strGuide = _T("");

	// PrevSet
	m_ScrCtrl.SetVariable(_T("PrevSetScreenNo"), _T("901"));

	// Title
	strCommand.Format(_T("%s%d"), _T("APValue"), 1);
	m_ScrCtrl.SetVariable(strCommand, strTitleText);

	// Value Guide
	if(nMaxInNumber < 20)
	{
		strCommand.Format(_T("%s%d"), _T("APValue"), 2);
		strGuide.Format(_T("MAX INPUT LENGTH : %d"), nMaxInNumber);
		m_ScrCtrl.SetVariable(strCommand, strGuide);
	}
	
	// Current Value
	strCommand.Format(_T("%s%d"),  _T("APValue"), 3);
	m_ScrCtrl.SetVariable(strCommand, strInText);

	// Set MaxInChar
	strMaxInChar.Format(_T("%s%d"), _T("MaxInChar:"), nMaxInNumber);
	m_ScrCtrl.SetVariable(strCommand, strMaxInChar);

	// Set Timeout
	m_ScrCtrl.SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
	m_ScrCtrl.SetVariable(_T("APMaxTime"), SUPERVISOR_SCREEN_TIMEOUT);

	// Set InputType
	m_ScrCtrl.SetVariable(_T("APSetInputType"), strInputType);

	// Display Screen
	m_ScrCtrl.SetVariable(_T("APCenterMovie"), _T("901"));


	///////////////////////////////////
	// GET KEY STRING
	while (TRUE)
	{
//		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (m_ScrCtrl.GetUserInputCheck() == TRUE)
		{
			m_ScrCtrl.GetUserInputData(strScrOutName, strScrOutData);
			break;
		}

		// 대기
		Sleep(100);
	}

	nResultCount = NH_SplitString(strScrOutData, (CString)SCR_RES_DELIMITER, strTempArray);

	if (nResultCount >= 2)
	{
		if(strTempArray[0] == S_SAVE)
		{
			strOutText = strTempArray[1];
			return TRUE;
		}
		else
		{
			strOutText = strInText;
			return FALSE;
		}
	}

	return FALSE;
}

#define		SCR_TEXTVALUE_CMD		_T("APValue")			// 문자열 설정
#define		SCR_CTRLONOFF_CMD		_T("APState")			// 버튼 ON-OFF값 설정

BOOL CScreenViewerDlg::Operator_Processing_Screen(CString strProcessingText, BOOL bResultScreen/*=FALSE*/, CString strErrorCode/*=_T("000000")*/)
{
	CString strCommand = _T("");
	CString strResultCode = _T("");
	CString strScrOutName;
	CString strScrOutData;
	
	// Key Disable
	//CrypteraEPP_StopClearTextMode();
	
	// PrevSet
	m_ScrCtrl.SetVariable(_T("PrevSetScreenNo"), _T("902"));
	
	if(TRUE == bResultScreen)
	{
		strScrOutName.Empty();
		strScrOutData.Empty();
		
		// Key Enable
//		CrypteraEPP_EnterClearTextMode();
		
		// Set Timeout
		m_ScrCtrl.SetVariable(_T("APTimeoutMode"), _T("TimeoutLast"));
		m_ScrCtrl.SetVariable(_T("APMaxTime"), _T("120"));
		
		// Result
		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
		strResultCode.Format(_T("%s%s"), _T("RESULT : "), strErrorCode);
		m_ScrCtrl.SetVariable(strCommand, strResultCode);
		
		// Press Enter Key
		strCommand.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, 1);
		m_ScrCtrl.SetVariable(strCommand, _T("on"));

		m_ScrCtrl.SetVariable(_T("APSetBlinkMode"), _T("0"));
	}
	else
	{
		// Message
		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
		m_ScrCtrl.SetVariable(strCommand, strProcessingText);

		m_ScrCtrl.SetVariable(_T("APSetBlinkMode"), _T("1"));
		
		// Press Enter Key
		strCommand.Format(_T("%s%d"), SCR_CTRLONOFF_CMD, 1);
		m_ScrCtrl.SetVariable(strCommand, _T("off"));
	}
	
	// Display Screen
	m_ScrCtrl.SetVariable(_T("APCenterMovie"), _T("902"));
	
	if(TRUE == bResultScreen)
	{
		///////////////////////////////////
		// GET KEY STRING
		while (TRUE)
		{
//			CheckService();
			
			if (m_ScrCtrl.GetUserInputCheck() == TRUE)
			{
				m_ScrCtrl.GetUserInputData(strScrOutName, strScrOutData);
				
				if( (strScrOutData == S_ENTER) || (strScrOutData == S_TIMEOVER) )
				{
					return TRUE;
				}
			}
			
			// 대기
			Sleep(100);
		}
	}
	
	return FALSE;
}