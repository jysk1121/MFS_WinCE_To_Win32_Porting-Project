// FlashViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FlashViewer.h"
#include "FlashViewerDlg.h"
#include "MB2500DLL.h"
#include "..\update\pwindbas.h"	// [2ND]

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_SHOW		1001
#define TIMER_START		1002
#define TIMER_CHECK		1003

// CFlashViewerDlg dialog

CMB2500DLLAPi	gMB2500DLL;

CFlashViewerDlg::CFlashViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFlashViewerDlg::IDD, pParent)
	, m_strFlashX(_T(""))
	, m_strFlashY(_T(""))
	, m_strFlashWidth(_T(""))
	, m_strFlashHeight(_T(""))
	, m_strPicassoX(_T(""))
	, m_strPicassoY(_T(""))
	, m_strPicassoWidth(_T(""))
	, m_strPicassoHeight(_T(""))
	, m_bFlashEngineSelect(FALSE)
	, m_strTimeout(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pPicasso = NULL;
	m_pFlash = NULL;
}

void CFlashViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FLASH_X, m_strFlashX);
	DDX_Text(pDX, IDC_EDIT_FLASH_Y, m_strFlashY);
	DDX_Text(pDX, IDC_EDIT_FLASH_WIDTH, m_strFlashWidth);
	DDX_Text(pDX, IDC_EDIT_FLASH_HEIGHT, m_strFlashHeight);
	DDX_Text(pDX, IDC_EDIT_PICASSO_X, m_strPicassoX);
	DDX_Text(pDX, IDC_EDIT_PICASSO_Y, m_strPicassoY);
	DDX_Text(pDX, IDC_EDIT_PICASSO_WIDTH, m_strPicassoWidth);
	DDX_Text(pDX, IDC_EDIT_PICASSO_HEIGHT, m_strPicassoHeight);
	DDX_Radio(pDX, IDC_RADIO_FLASH, m_bFlashEngineSelect);
	DDX_Control(pDX, IDC_COMBO1, m_ctlScreenNumber);
	DDX_Control(pDX, IDC_OUTPUT, m_listOutPut);
	DDX_Control(pDX, IDC_COMBO_LANGUAGE, m_Language);
	DDX_Text(pDX, IDC_TIMEOUT, m_strTimeout);
}

BEGIN_MESSAGE_MAP(CFlashViewerDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_FLASH_CREATE, &CFlashViewerDlg::OnBnClickedButtonFlashCreate)
	ON_BN_CLICKED(IDC_BUTTON_PICASSO_CREATE, &CFlashViewerDlg::OnBnClickedButtonPicassoCreate)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_FLASH_LOAD_MOVIE, &CFlashViewerDlg::OnBnClickedButtonFlashLoadMovie)
	ON_BN_CLICKED(IDC_BUTTON_FLASH_MOVE_WINDOW, &CFlashViewerDlg::OnBnClickedButtonFlashMoveWindow)
	ON_BN_CLICKED(IDC_BUTTON_PICASSO_LOAD_SCREEN, &CFlashViewerDlg::OnBnClickedButtonPicassoLoadScreen)
	ON_BN_CLICKED(IDC_BUTTON_PICASSO_MOVE_WINDOW, &CFlashViewerDlg::OnBnClickedButtonPicassoMoveWindow)
	ON_BN_CLICKED(IDC_CTL_SHOW, &CFlashViewerDlg::OnBnClickedCtlShow)
	ON_BN_CLICKED(IDC_CTL_UPDATE, &CFlashViewerDlg::OnBnClickedCtlUpdate)
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, &CFlashViewerDlg::OnBnClickedButtonRight)
	ON_BN_CLICKED(IDC_BUTTON_LEFT, &CFlashViewerDlg::OnBnClickedButtonLeft)
	ON_WM_TIMER()
END_MESSAGE_MAP()


int CFlashViewerDlg::RegSetValueExt(HKEY hKey, LPCTSTR hKeyName, LPCTSTR lpszValueName, DWORD dwType, DWORD dwLen, LPVOID lpszData)
{
	HKEY	hKeyResult = 0;
	DWORD	dwDisposition = 0;
	LONG	lResult;

	if (dwType == REG_DWORD_LITTLE_ENDIAN)
		dwLen = sizeof(int);

	// Open Key 
	lResult = RegOpenKeyEx(hKey,hKeyName,0,KEY_ALL_ACCESS,&hKeyResult);

	if (lResult == ERROR_SUCCESS) 
	{
		// Set Value On Open Key 
		lResult = RegSetValueEx(hKeyResult,lpszValueName,0,dwType,(CONST BYTE*)lpszData,dwLen);
	}
	else 
	{
		// Create New Key 
		lResult = RegCreateKeyEx(hKey,hKeyName,0,NULL,
			  			         REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,
								 NULL, &hKeyResult, &dwDisposition);

		if (lResult == ERROR_SUCCESS)
			lResult = RegSetValueEx(hKeyResult,lpszValueName,0,dwType,(CONST BYTE*)lpszData,dwLen);
	}

	// Close Key 
	RegCloseKey(hKeyResult);

	if (lResult == ERROR_SUCCESS) 
		return TRUE;
    else 
		return FALSE;
}

// CFlashViewerDlg message handlers

BOOL CFlashViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//////////////////////////////////////////////////////////////////////////
	//	MOVE OBJECT STORE SIZE FOR UPDATE FILES.
	{
		{

			DWORD dwStorePages, dwRamPages, dwPageSize;
			if(GetSystemMemoryDivision(&dwStorePages, &dwRamPages, &dwPageSize) == TRUE)
			{
				dwRamPages = dwStorePages;
				dwStorePages = (DWORD)(20000000 / dwPageSize);	// 20M

				SetSystemMemoryDivision(dwStorePages);
			}
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CString strKey;
	CString strName;
	CString strValue;
	LPVOID lpValue;

	strKey = L"CLSID\\{D27CDB6E-AE6D-11cf-96B8-444553540000}\\InprocServer32";
	strName = L"";
	strValue = L"\\ATM\\IEActiveX.dll";
	lpValue = strValue.GetBuffer(0);
	RegSetValueExt(HKEY_CLASSES_ROOT, strKey, strName, REG_SZ, ((strValue.GetLength()+1)*2), lpValue);
	strValue.ReleaseBuffer();

	strKey = L"CLSID\\{D27CDB6E-AE6D-11cf-96B8-444553540000}\\ToolboxBitmap32";
	strName = L"";
	strValue = L"\\ATM\\IEActiveX.dll, 1";
	lpValue = strValue.GetBuffer(0);
	RegSetValueExt(HKEY_CLASSES_ROOT, strKey, strName, REG_SZ, ((strValue.GetLength()+1)*2), lpValue);
	strValue.ReleaseBuffer();

	strKey = L"CLSID\\{D27CDB70-AE6D-11cf-96B8-444553540000}\\InprocServer32";
	strName = L"";
	strValue = L"\\ATM\\IEActiveX.dll";
	lpValue = strValue.GetBuffer(0);
	RegSetValueExt(HKEY_CLASSES_ROOT, strKey, strName, REG_SZ, ((strValue.GetLength()+1)*2), lpValue);
	strValue.ReleaseBuffer();

	strKey = L"CLSID\\{D27CDB70-AE6D-11cf-96B8-444553540000}\\ToolboxBitmap32";
	strName = L"";
	strValue = L"\\ATM\\IEActiveX.dll, 1";
	lpValue = strValue.GetBuffer(0);
	RegSetValueExt(HKEY_CLASSES_ROOT, strKey, strName, REG_SZ, ((strValue.GetLength()+1)*2), lpValue);
	strValue.ReleaseBuffer();

	strKey = L"TypeLib\\{D27CDB6B-AE6D-11CF-96B8-444553540000}\\1.0\\0";
	strName = L"";
	strValue = L"\\ATM\\IEActiveX.dll";
	lpValue = strValue.GetBuffer(0);
	RegSetValueExt(HKEY_CLASSES_ROOT, strKey, strName, REG_SZ, ((strValue.GetLength()+1)*2), lpValue);
	strValue.ReleaseBuffer();

	strKey = L"TypeLib\\{D27CDB6B-AE6D-11CF-96B8-444553540000}\\1.0\\0\\win32";
	strName = L"";
	strValue = L"\\ATM\\IEActiveX.dll";
	lpValue = strValue.GetBuffer(0);
	RegSetValueExt(HKEY_CLASSES_ROOT, strKey, strName, REG_SZ, ((strValue.GetLength()+1)*2), lpValue);
	strValue.ReleaseBuffer();

	strKey = L"CLSID\\{1171A62F-05D2-11D1-83FC-00A0C9089C5A}\\InprocServer32";
	strName = L"";
	strValue = L"\\ATM\\IEActiveX.dll";
	lpValue = strValue.GetBuffer(0);
	RegSetValueExt(HKEY_CLASSES_ROOT, strKey, strName, REG_SZ, ((strValue.GetLength()+1)*2), lpValue);
	strValue.ReleaseBuffer();

	strKey = L"SOFTWARE\\Microsoft\\Shell\\OnTop";
	strName = L"";
	int nValue = 1;
	RegSetValueExt(HKEY_LOCAL_MACHINE, strKey, strName, REG_DWORD_LITTLE_ENDIAN, sizeof(int), &nValue);

	// TODO: Add extra initialization here
	m_strFlashX = L"0";
	m_strFlashY = L"0";
	m_strFlashWidth = L"1024";
	m_strFlashHeight = L"600";

#if 0
	m_strPicassoX = L"0";
	m_strPicassoY = L"720";
	m_strPicassoWidth = L"720";
	m_strPicassoHeight = L"480";
#else
	m_strPicassoX = L"0";
	m_strPicassoY = L"0";
	m_strPicassoWidth = L"1024";
	m_strPicassoHeight = L"600";
#endif

	m_strTimeout = L"5";

	m_bFlashEngineSelect = FALSE;

//	SetTimer(TIMER_CHECK, 100, NULL);

	//SetTimer(TIMER_SHOW, 100, NULL);

	UpdateData(FALSE);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CFlashViewerDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_FLASHVIEWER_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_FLASHVIEWER_DIALOG));
	}
}
#endif

void CFlashViewerDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if (m_pFlash != NULL)
	{
		CScrCtrl::DestroyScreenEngine(m_pFlash);
		m_pFlash = NULL;
	}

	if (m_pPicasso != NULL)
	{
		CScrCtrl::DestroyScreenEngine(m_pPicasso);
		m_pPicasso = NULL;
	}

	CDialog::OnClose();
}

void CFlashViewerDlg::OnBnClickedButtonFlashCreate()
{
	if (m_pFlash != NULL)
		return;

	UpdateData(TRUE);

	int	nX = _ttoi(m_strFlashX);
	int	nY = _ttoi(m_strFlashY);
	int	nWidth = _ttoi(m_strFlashWidth);
	int	nHeight = _ttoi(m_strFlashHeight);

	m_pFlash = CScrCtrl::CreateScreenEngine(SCR_TYPE_FLASH_LITE, 0, 0, nWidth, nHeight);
	m_pFlash->MoveScreen(nX, nY, nWidth, nHeight);
	m_pFlash->Initialize();
}

void CFlashViewerDlg::OnBnClickedButtonFlashLoadMovie()
{
	if (m_pFlash == NULL)
	{
		MessageBox(L"Please create \"Adobe Flash Lite Engine\".");
		return;
	}

	// select File
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, L"Flash(*.swf)|*.swf|All Files(*.*)|*.*||");
	if(dlg.DoModal() != IDOK)
		return;

	UpdateData(TRUE);

	int	nWidth = _ttoi(m_strFlashWidth);
	int	nHeight = _ttoi(m_strFlashHeight);

	CString strFilePath = dlg.GetPathName();
	//m_pFlash->LoadScreens(L"\\atm\\screen\\flash", L"main.swf", nWidth, nHeight);
	m_pFlash->LoadScreens(strFilePath, L"", nWidth, nHeight);
}


void CFlashViewerDlg::OnBnClickedButtonFlashMoveWindow()
{
	if (m_pFlash == NULL)
	{
		MessageBox(L"Please create \"Adobe Flash Lite Engine\".");
		return;
	}

	UpdateData(TRUE);

	int	nX = _ttoi(m_strFlashX);
	int	nY = _ttoi(m_strFlashY);
	int	nWidth = _ttoi(m_strFlashWidth);
	int	nHeight = _ttoi(m_strFlashHeight);

	m_pFlash->MoveScreen(nX, nY, nWidth, nHeight);
}

void CFlashViewerDlg::OnBnClickedButtonPicassoCreate()
{
	if (m_pPicasso != NULL)
		return;

	UpdateData(TRUE);

	int	nX = _ttoi(m_strPicassoX);
	int	nY = _ttoi(m_strPicassoY);
	int	nWidth = _ttoi(m_strPicassoWidth);
	int	nHeight = _ttoi(m_strPicassoHeight);

	m_pPicasso = CScrCtrl::CreateScreenEngine(SCR_TYPE_PICASSO, 0, 0, nWidth, nHeight);
	m_pPicasso->Initialize();
	m_pPicasso->MoveScreen(nX, nY, nWidth, nHeight);
}

void CFlashViewerDlg::OnBnClickedButtonPicassoLoadScreen()
{

	if (m_pPicasso == NULL)
	{
		MessageBox(L"Please create \"Nautilus Hyosung Picasso Engine\".");
		return;
	}
	

	UpdateData(TRUE);

	int	nWidth = _ttoi(m_strPicassoWidth);
	int	nHeight = _ttoi(m_strPicassoHeight);

	//m_pPicasso->LoadScreens(_T("\\ATM\\Screen"), _T("ATM"), nWidth, nHeight);
	m_pPicasso->LoadScreens(_T("\\ATM\\Screen"), _T("FRONT_FUNCTION"), nWidth, nHeight);
	m_pPicasso->GetUserInputCheck(INFINITE);

	CString strScreenName;
	int		nScreenCount;
	int		nCount;
	
	nCount = nScreenCount = m_pPicasso->GetScreenCount();
	m_ctlScreenNumber.InitStorage(nScreenCount, 10);

	strScreenName = m_pPicasso->GetFirstScreenNumber();
	while(nCount > 0)
	{
		m_ctlScreenNumber.AddString(strScreenName);
		nCount--;

		strScreenName = m_pPicasso->GetNextScreenNumber();
	}
}

void CFlashViewerDlg::OnBnClickedButtonPicassoScreenShow()
{
	this->SetTimer(TIMER_SHOW, 100, NULL);
}

void CFlashViewerDlg::OnBnClickedButtonPicassoLoadScreenWhite()
{	
}

void CFlashViewerDlg::OnBnClickedButtonPicassoLoadScreenMount()
{
}

void CFlashViewerDlg::OnBnClickedButtonPicassoLoadScreenBlack()
{
/*
	OnBnClickedButtonPicassoCreate();

	UpdateData(TRUE);

	int	nWidth = _ttoi(m_strPicassoWidth);
	int	nHeight = _ttoi(m_strPicassoHeight);

	m_pPicasso->LoadScreens(_T("\\ATM\\PentaUI"), _T("BLACK"), nWidth, nHeight);
	m_pPicasso->GetUserInputCheck(INFINITE);

	CString strScreenName;
	int		nScreenCount;
	int		nCount;
	
	nCount = nScreenCount = m_pPicasso->GetScreenCount();
	m_ctlScreenNumber.InitStorage(nScreenCount, 10);

	strScreenName = m_pPicasso->GetFirstScreenNumber();
	while(nCount > 0)
	{
		m_ctlScreenNumber.AddString(strScreenName);
		nCount--;

		strScreenName = m_pPicasso->GetNextScreenNumber();
	}

	m_ctlScreenNumber.SetCurSel(0);
*/	
}


void CFlashViewerDlg::OnBnClickedButtonPicassoMoveWindow()
{
	if (m_pPicasso == NULL)
	{
		MessageBox(L"Please create \"Nautilus Hyosung Picasso Engine\".");
		return;
	}

	UpdateData(TRUE);

	int	nX = _ttoi(m_strPicassoX);
	int	nY = _ttoi(m_strPicassoY);
	int	nWidth = _ttoi(m_strPicassoWidth);
	int	nHeight = _ttoi(m_strPicassoHeight);

	m_pPicasso->MoveScreen(nX, nY, nWidth, nHeight);
}

void CFlashViewerDlg::OnBnClickedCtlUpdate()
{
	UpdateData(TRUE);

	int nCount;
	CScrCtrl *pScrCtrl = NULL;
	CString	strScreenType;

	BOOL bFlash = ((CButton*)GetDlgItem(IDC_RADIO_FLASH))->GetCheck();

	if (bFlash)
	{
		pScrCtrl = m_pFlash;
		strScreenType = L"FLASH";
	}
	else
	{
		pScrCtrl = m_pPicasso;
		strScreenType = L"PICASSO";
	}

	if (pScrCtrl == NULL)
	{
		m_strOutput.Format(L"[%s][ERROR]--[NEED TO CREATE ENGINE]", strScreenType);
		m_listOutPut.AddString(m_strOutput);
		nCount = m_listOutPut.GetCount();
		if (nCount > 0)
			m_listOutPut.SetCurSel(nCount-1);
		return;
	}

	CString strCurrentScrNumber;

	m_ctlScreenNumber.GetWindowText(strCurrentScrNumber);

	m_listOutPut.AddString(L"");
	pScrCtrl->SetActiveScreen();
	
	pScrCtrl->SetVariable(L"APUpdateStart", strCurrentScrNumber);
	m_strOutput.Format(L"[%s][APUpdateStart]--[%s]", strScreenType, strCurrentScrNumber);
	m_listOutPut.AddString(m_strOutput);
	nCount = m_listOutPut.GetCount();
	if (nCount > 0)
		m_listOutPut.SetCurSel(nCount-1);

	// -- Language
	{
		CString strLanguage;
		int nSelect = m_Language.GetCurSel();
		if (nSelect == 0)
			strLanguage = L"ENGLISH";
		else if (nSelect == 1)
			strLanguage = L"FRENCH";
		else if (nSelect == 2)
			strLanguage = L"SPANISH";
		else
			strLanguage = L"ENGLISH";

		pScrCtrl->SetVariable(L"kindLanguage", strLanguage);
		m_strOutput.Format(L"[%s][kindLanguage]--[%s]", strScreenType, strLanguage);
		m_listOutPut.AddString(m_strOutput);
		nCount = m_listOutPut.GetCount();
		if (nCount > 0)
			m_listOutPut.SetCurSel(nCount-1);
	}

	// -- Variable/State --
	{
		int		i;
		BOOL	bApState[18];
		CString strApValue[24];
		CString strTemp;

		bApState[0] = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
		bApState[1] = ((CButton*)GetDlgItem(IDC_CHECK2))->GetCheck();
		bApState[2] = ((CButton*)GetDlgItem(IDC_CHECK3))->GetCheck();
		bApState[3] = ((CButton*)GetDlgItem(IDC_CHECK4))->GetCheck();
		bApState[4] = ((CButton*)GetDlgItem(IDC_CHECK5))->GetCheck();
		bApState[5] = ((CButton*)GetDlgItem(IDC_CHECK6))->GetCheck();
		bApState[6] = ((CButton*)GetDlgItem(IDC_CHECK7))->GetCheck();
		bApState[7] = ((CButton*)GetDlgItem(IDC_CHECK8))->GetCheck();
		bApState[8] = ((CButton*)GetDlgItem(IDC_CHECK9))->GetCheck();
		bApState[9] = ((CButton*)GetDlgItem(IDC_CHECK10))->GetCheck();
		bApState[10] = ((CButton*)GetDlgItem(IDC_CHECK11))->GetCheck();
		bApState[11] = ((CButton*)GetDlgItem(IDC_CHECK12))->GetCheck();
		bApState[12] = ((CButton*)GetDlgItem(IDC_CHECK13))->GetCheck();
		bApState[13] = ((CButton*)GetDlgItem(IDC_CHECK14))->GetCheck();
		bApState[14] = ((CButton*)GetDlgItem(IDC_CHECK15))->GetCheck();
		bApState[15] = ((CButton*)GetDlgItem(IDC_CHECK16))->GetCheck();
		bApState[16] = ((CButton*)GetDlgItem(IDC_CHECK17))->GetCheck();
		bApState[17] = ((CButton*)GetDlgItem(IDC_CHECK18))->GetCheck();

		strTemp = L"";
		for (i = 0; i < 18; i++)
		{
			if (i != 0)
				strTemp += L"\r";

			if (bApState[i])
				strTemp += L"on";
			else
				strTemp += L"off";
		}

		pScrCtrl->SetVariable(L"APState1", strTemp);
		m_strOutput.Format(L"[%s][APState1]--[%s]", strScreenType, strTemp);
		m_listOutPut.AddString(m_strOutput);
		nCount = m_listOutPut.GetCount();
		if (nCount > 0)
			m_listOutPut.SetCurSel(nCount-1);

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
		(GetDlgItem(IDC_APVALUE_21))->GetWindowText(strApValue[20]);
		(GetDlgItem(IDC_APVALUE_22))->GetWindowText(strApValue[21]);
		(GetDlgItem(IDC_APVALUE_23))->GetWindowText(strApValue[22]);
		(GetDlgItem(IDC_APVALUE_24))->GetWindowText(strApValue[23]);

		for (i=0; i < 24; i++)
			strApValue[i].Replace(_T("\\n"), _T("\n"));

		strTemp = L"";
		for (i = 0; i < 24; i++)
		{
			if (i != 0)
				strTemp += L"\r";

			strTemp += strApValue[i];
		}

		pScrCtrl->SetVariable(L"APValue1", strTemp);
		m_strOutput.Format(L"[%s][APValue1]--[%s]", strScreenType, strTemp);
		m_listOutPut.AddString(m_strOutput);
		nCount = m_listOutPut.GetCount();
		if (nCount > 0)
			m_listOutPut.SetCurSel(nCount-1);

	}

	pScrCtrl->SetVariable(L"APMaxTime", m_strTimeout);
	m_strOutput.Format(L"[%s][APMaxTime]--[%s]", strScreenType, m_strTimeout);
	m_listOutPut.AddString(m_strOutput);
	nCount = m_listOutPut.GetCount();
	if (nCount > 0)
		m_listOutPut.SetCurSel(nCount-1);

	pScrCtrl->SetVariable(L"APUpdate", strCurrentScrNumber);
	m_strOutput.Format(L"[%s][APUpdate]--[%s]", strScreenType, strCurrentScrNumber);
	m_listOutPut.AddString(m_strOutput);
	
	nCount = m_listOutPut.GetCount();
	if (nCount > 0)
		m_listOutPut.SetCurSel(nCount-1);
}

void CFlashViewerDlg::OnBnClickedCtlShow()
{
	UpdateData(TRUE);

	int nCount;
	CScrCtrl *pScrCtrl = NULL;
	CString	strScreenType;

	BOOL bFlash = ((CButton*)GetDlgItem(IDC_RADIO_FLASH))->GetCheck();

	if (bFlash)
	{
		pScrCtrl = m_pFlash;
		strScreenType = L"FLASH";
	}
	else
	{
		pScrCtrl = m_pPicasso;
		strScreenType = L"PICASSO";
	}

	if (pScrCtrl == NULL)
	{
		m_strOutput.Format(L"[%s][ERROR]--[NEED TO CREATE ENGINE]", strScreenType);
		m_listOutPut.AddString(m_strOutput);
		nCount = m_listOutPut.GetCount();
		if (nCount > 0)
			m_listOutPut.SetCurSel(nCount-1);
		return;
	}

	CString strCurrentScrNumber;

	m_ctlScreenNumber.GetWindowText(strCurrentScrNumber);

	m_listOutPut.AddString(L"");
	pScrCtrl->SetActiveScreen();
	pScrCtrl->SetVariable(L"PrevSetScreenNo", strCurrentScrNumber);
	m_strOutput.Format(L"[%s][PrevSetScreenNo]--[%s]", strScreenType,strCurrentScrNumber);
	m_listOutPut.AddString(m_strOutput);
	nCount = m_listOutPut.GetCount();
	if (nCount > 0)
		m_listOutPut.SetCurSel(nCount-1);

	// -- Language
	{
		CString strLanguage;
		int nSelect = m_Language.GetCurSel();
		if (nSelect == 0)
			strLanguage = L"ENGLISH";
		else if (nSelect == 1)
			strLanguage = L"FRENCH";
		else if (nSelect == 2)
			strLanguage = L"SPANISH";
		else
			strLanguage = L"ENGLISH";

		pScrCtrl->SetVariable(L"kindLanguage", strLanguage);
		m_strOutput.Format(L"[%s][kindLanguage]--[%s]", strScreenType, strLanguage);
		m_listOutPut.AddString(m_strOutput);
		nCount = m_listOutPut.GetCount();
		if (nCount > 0)
			m_listOutPut.SetCurSel(nCount-1);
	}

	// -- Variable/State --
	{
		int		i;
		BOOL	bApState[18];
		CString strApValue[24];
		CString strTemp;

		bApState[0] = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
		bApState[1] = ((CButton*)GetDlgItem(IDC_CHECK2))->GetCheck();
		bApState[2] = ((CButton*)GetDlgItem(IDC_CHECK3))->GetCheck();
		bApState[3] = ((CButton*)GetDlgItem(IDC_CHECK4))->GetCheck();
		bApState[4] = ((CButton*)GetDlgItem(IDC_CHECK5))->GetCheck();
		bApState[5] = ((CButton*)GetDlgItem(IDC_CHECK6))->GetCheck();
		bApState[6] = ((CButton*)GetDlgItem(IDC_CHECK7))->GetCheck();
		bApState[7] = ((CButton*)GetDlgItem(IDC_CHECK8))->GetCheck();
		bApState[8] = ((CButton*)GetDlgItem(IDC_CHECK9))->GetCheck();
		bApState[9] = ((CButton*)GetDlgItem(IDC_CHECK10))->GetCheck();
		bApState[10] = ((CButton*)GetDlgItem(IDC_CHECK11))->GetCheck();
		bApState[11] = ((CButton*)GetDlgItem(IDC_CHECK12))->GetCheck();
		bApState[12] = ((CButton*)GetDlgItem(IDC_CHECK13))->GetCheck();
		bApState[13] = ((CButton*)GetDlgItem(IDC_CHECK14))->GetCheck();
		bApState[14] = ((CButton*)GetDlgItem(IDC_CHECK15))->GetCheck();
		bApState[15] = ((CButton*)GetDlgItem(IDC_CHECK16))->GetCheck();
		bApState[16] = ((CButton*)GetDlgItem(IDC_CHECK17))->GetCheck();
		bApState[17] = ((CButton*)GetDlgItem(IDC_CHECK18))->GetCheck();

		strTemp = L"";
		for (i = 0; i < 18; i++)
		{
			if (i != 0)
				strTemp += L"\r";

			if (bApState[i])
				strTemp += L"on";
			else
				strTemp += L"off";
		}

		pScrCtrl->SetVariable(L"APState1", strTemp);
		m_strOutput.Format(L"[%s][APState1]--[%s]", strScreenType, strTemp);
		m_listOutPut.AddString(m_strOutput);
		nCount = m_listOutPut.GetCount();
		if (nCount > 0)
			m_listOutPut.SetCurSel(nCount-1);

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
		(GetDlgItem(IDC_APVALUE_21))->GetWindowText(strApValue[20]);
		(GetDlgItem(IDC_APVALUE_22))->GetWindowText(strApValue[21]);
		(GetDlgItem(IDC_APVALUE_23))->GetWindowText(strApValue[22]);
		(GetDlgItem(IDC_APVALUE_24))->GetWindowText(strApValue[23]);

		for (i=0; i < 24; i++)
			strApValue[i].Replace(_T("\\n"), _T("\n"));

		strTemp = L"";
		for (i = 0; i < 24; i++)
		{
			if (i != 0)
				strTemp += L"\r";

			strTemp += strApValue[i];
		}

		pScrCtrl->SetVariable(L"APValue1", strTemp);
		m_strOutput.Format(L"[%s][APValue1]--[%s]", strScreenType, strTemp);
		m_listOutPut.AddString(m_strOutput);
		nCount = m_listOutPut.GetCount();
		if (nCount > 0)
			m_listOutPut.SetCurSel(nCount-1);

	}

	pScrCtrl->SetVariable(L"APMaxTime", m_strTimeout);
	m_strOutput.Format(L"[%s][APMaxTime]--[%s]", strScreenType, m_strTimeout);
	m_listOutPut.AddString(m_strOutput);
	nCount = m_listOutPut.GetCount();
	if (nCount > 0)
		m_listOutPut.SetCurSel(nCount-1);

	pScrCtrl->SetVariable(L"APCenterMovie", strCurrentScrNumber);
	m_strOutput.Format(L"[%s][APCenterMovie]--[%s]", strScreenType, strCurrentScrNumber);
	m_listOutPut.AddString(m_strOutput);
	
	nCount = m_listOutPut.GetCount();
	if (nCount > 0)
		m_listOutPut.SetCurSel(nCount-1);
}

void CFlashViewerDlg::OnBnClickedButtonRight()
{
	int nIndex = m_ctlScreenNumber.GetCurSel() + 1;
	if (nIndex < m_ctlScreenNumber.GetCount())
		m_ctlScreenNumber.SetCurSel(nIndex);

	OnBnClickedCtlShow();
}

void CFlashViewerDlg::OnBnClickedButtonLeft()
{
	int nIndex = m_ctlScreenNumber.GetCurSel() - 1;
	if (nIndex >= 0)
		m_ctlScreenNumber.SetCurSel(nIndex);

	OnBnClickedCtlShow();
}

#define		SHOW_WHITE	1

void CFlashViewerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	if (nIDEvent == TIMER_CHECK)
	{
		if (m_pPicasso != NULL)
		{
			if (m_pPicasso->GetUserInputCheck())
			{
				CString strName, strData;
				m_pPicasso->GetUserInputData(strName, strData);
				strData.Replace(0x1F, '^');
				m_strOutput.Format(_T("[PICASSO][FSCommand] -- [%s]-[%s]"), strName, strData);
				m_listOutPut.AddString(m_strOutput);
				m_listOutPut.SetCurSel(m_listOutPut.GetCount()-1);

				int nCount = m_listOutPut.GetCount();
				if (nCount > 0)
					m_listOutPut.SetCurSel(nCount-1);
			}
		}

		if (m_pFlash != NULL)
		{
			if (m_pFlash->GetUserInputCheck())
			{
				CString strName, strData;
				m_pFlash->GetUserInputData(strName, strData);
				strData.Replace(0x1F, '^');
				m_strOutput.Format(_T("[FLASH][FSCommand] -- [%s]-[%s]"), strName, strData);
				m_listOutPut.AddString(m_strOutput);
				m_listOutPut.SetCurSel(m_listOutPut.GetCount()-1);

				int nCount = m_listOutPut.GetCount();
				if (nCount > 0)
					m_listOutPut.SetCurSel(nCount-1);
			}
		}
	}
	else if (nIDEvent == TIMER_SHOW)
	{
		KillTimer(TIMER_SHOW);

		OnBnClickedButtonFlashCreate();

		UpdateData(TRUE);

		int	nWidth = _ttoi(m_strFlashWidth);
		int	nHeight = _ttoi(m_strFlashHeight);

		m_pFlash->LoadScreens(L"\\atm\\test\\test.swf", L"", nWidth, nHeight);
	}

	CDialog::OnTimer(nIDEvent);
}
