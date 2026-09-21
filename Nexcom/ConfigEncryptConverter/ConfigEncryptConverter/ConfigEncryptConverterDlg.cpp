
// ConfigEncryptConverterDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "ConfigEncryptConverter.h"
#include "ConfigEncryptConverterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


const unsigned int crc_table[256] = 
{
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241, 
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440, 
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40, 
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841, 
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40, 
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41, 
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641, 
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040, 
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240, 
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441, 
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41, 
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840, 
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41, 
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40, 
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640, 
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041, 
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240, 
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441, 
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41, 
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840, 
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41, 
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40, 
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640, 
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041, 
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241, 
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440, 
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40, 
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841, 
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40, 
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41, 
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641, 
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CConfigEncryptConverterDlg 대화 상자




CConfigEncryptConverterDlg::CConfigEncryptConverterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigEncryptConverterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CConfigEncryptConverterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CConfigEncryptConverterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_CONFIG_FILE_DIR, &CConfigEncryptConverterDlg::OnBnClickedBtnConfigFileDir)
	ON_BN_CLICKED(IDC_BTN_ENCRYPT_FILE_DIR, &CConfigEncryptConverterDlg::OnBnClickedBtnEncryptFileDir)
	ON_BN_CLICKED(IDC_BTN_ENCRYP, &CConfigEncryptConverterDlg::OnBnClickedBtnEncryp)
	ON_BN_CLICKED(IDC_BTN_DECRYPT, &CConfigEncryptConverterDlg::OnBnClickedBtnDecrypt)
END_MESSAGE_MAP()


// CConfigEncryptConverterDlg 메시지 처리기

BOOL CConfigEncryptConverterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	m_strConfigFileDir = _T("D:\\");
	GetDlgItem(IDC_ED_CONFIG_FILE_DIR)->SetWindowText(m_strConfigFileDir);

	m_strEncryptFileDir = _T("D:\\");
	GetDlgItem(IDC_ED_ENCRYPT_FILE_DIR)->SetWindowText(m_strEncryptFileDir);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CConfigEncryptConverterDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CConfigEncryptConverterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CConfigEncryptConverterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CALLBACK MyBrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		break;
	}
	return 0;
}

void CConfigEncryptConverterDlg::OnBnClickedBtnConfigFileDir()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ITEMIDLIST  *pidlBrowse;
	TCHAR       pszPathName[MAX_PATH];

	BROWSEINFO BrInfo;
	BrInfo.hwndOwner = NULL;//parent window handle
	BrInfo.pidlRoot = NULL;

	LPTSTR pFolderName = m_strConfigFileDir.GetBuffer(m_strConfigFileDir.GetLength());

	memset(&BrInfo, 0, sizeof(BrInfo));
	BrInfo.pszDisplayName = pszPathName;
	BrInfo.lpszTitle = _T("Root Directory for WinCE Screen Viewer");
	BrInfo.ulFlags = BIF_RETURNONLYFSDIRS;
	BrInfo.lpfn = MyBrowseCallbackProc;
	BrInfo.lParam = (LPARAM)pFolderName;
	pidlBrowse = ::SHBrowseForFolder(&BrInfo);

	if (pidlBrowse != NULL)
	{
		::SHGetPathFromIDList(pidlBrowse, pszPathName);  // get folder name

		m_strConfigFileDir = pszPathName;
		GetDlgItem(IDC_ED_CONFIG_FILE_DIR)->SetWindowText(m_strConfigFileDir);
	}

	m_strConfigFileDir.ReleaseBuffer();
}

void CConfigEncryptConverterDlg::OnBnClickedBtnEncryptFileDir()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ITEMIDLIST  *pidlBrowse;
	TCHAR       pszPathName[MAX_PATH];

	BROWSEINFO BrInfo;
	BrInfo.hwndOwner = NULL;//parent window handle
	BrInfo.pidlRoot = NULL;

	LPTSTR pFolderName = m_strEncryptFileDir.GetBuffer(m_strEncryptFileDir.GetLength());

	memset(&BrInfo, 0, sizeof(BrInfo));
	BrInfo.pszDisplayName = pszPathName;
	BrInfo.lpszTitle = _T("Root Directory for WinCE Screen Viewer");
	BrInfo.ulFlags = BIF_RETURNONLYFSDIRS;
	BrInfo.lpfn = MyBrowseCallbackProc;
	BrInfo.lParam = (LPARAM)pFolderName;
	pidlBrowse = ::SHBrowseForFolder(&BrInfo);

	if (pidlBrowse != NULL)
	{
		::SHGetPathFromIDList(pidlBrowse, pszPathName);  // get folder name

		m_strEncryptFileDir = pszPathName;
		GetDlgItem(IDC_ED_ENCRYPT_FILE_DIR)->SetWindowText(m_strEncryptFileDir);
	}

	m_strEncryptFileDir.ReleaseBuffer();
}

void CConfigEncryptConverterDlg::OnBnClickedBtnEncryp()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BOOL	bResult = TRUE;
	CString strFileName;
	CString strTemp;

	CString strConfigData, strAllConfigData;
	unsigned int nCRC = 0;
	BYTE byKey_16[32] = { 0, };
	BYTE *pbyAllConfigData = NULL;
	BYTE *pbyEncryptConfigData = NULL;
	DWORD dwAllConfigDataLen = 0, dwEncryptConfigDataLen = 0;

	// 1. ADS.CFG
	strConfigData = MergeConfigData(m_strConfigFileDir, ADS_CFG_PATH_NAME);
	if (strConfigData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Backup Config File(ADS) is failed."));
		return ;
	}

	strAllConfigData += strConfigData;

	// 2. BINLIST.CFG
	strConfigData = MergeConfigData(m_strConfigFileDir, BINLIST_CFG_PATH_NAME);
	if (strConfigData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Backup Config File(BINLIST) is failed."));
		return;
	}

	strAllConfigData += strConfigData;

	// 3. DEVICE.CFG
	strConfigData = MergeConfigData(m_strConfigFileDir, DEVICE_CFG_PATH_NAME);
	if (strConfigData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Backup Config File(DEVICE) is failed."));
		return;
	}
	
	strAllConfigData += strConfigData;

	// 4. EMV_CONFIG.CFG
	strConfigData = MergeConfigData(m_strConfigFileDir, EAGLE_ATM_EMV_CONFIG_PATH_NAME);
	if (strConfigData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Backup Config File(EMV_CONFIG) is failed."));
		return;
	}

	strAllConfigData += strConfigData;

	// 5. EMV_US_COMM_AID.CFG
	strConfigData = MergeConfigData(m_strConfigFileDir, EAGLE_ATM_EMV_US_COMM_AID_PATH_NAME);
	if (strConfigData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Backup Config File(EMV_US_COMM_AID) is failed."));
		return;
	}

	strAllConfigData += strConfigData;

	// 6. HOST.CFG
	strConfigData = MergeConfigData(m_strConfigFileDir, HOST_CFG_PATH_NAME);
	if (strConfigData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Backup Config File(HOST) is failed."));
		return;
	}

	strAllConfigData += strConfigData;

	// 7. MESSAGE.CFG
	strConfigData = MergeConfigData(m_strConfigFileDir, MESSAGE_CFG_PATH_NAME);
	if (strConfigData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Backup Config File(MESSAGE) is failed."));
		return;
	}

	strAllConfigData += strConfigData;

	// 8. OPTION.CFG
	strConfigData = MergeConfigData(m_strConfigFileDir, OPTION_CFG_PATH_NAME);
	if (strConfigData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Backup Config File(OPTION) is failed."));
		return;
	}

	strAllConfigData += strConfigData;

	// 9. SYSTEM.CFG
	strConfigData = MergeConfigData(m_strConfigFileDir, SYSTEM_CFG_PATH_NAME);
	if (strConfigData.IsEmpty() == TRUE)
	{
		AfxMessageBox(_T("Backup Config File(SYSTEM) is failed."));
		return;
	}

	strAllConfigData += strConfigData;

	// Merge 한 파일 암호화 및 새 파일 생성
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwWriten = 0;

	strFileName.Format(_T("%s\\%s"), m_strEncryptFileDir, BACKUP_CONFIGFILE_NAME);

	hFile = CreateFile(strFileName,
		GENERIC_WRITE,          // Open for writing
		0,                      // Do not share
		NULL,                   // No security
		CREATE_ALWAYS,			// create
		FILE_ATTRIBUTE_NORMAL,  // Normal file
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		AfxMessageBox(_T("Encrypt Config File is failed."));
		return;
	}

	dwAllConfigDataLen = strAllConfigData.GetLength();

	if (0 == (dwAllConfigDataLen % DATA_PADDING_SIZE))
		dwEncryptConfigDataLen = dwAllConfigDataLen;
	else
		dwEncryptConfigDataLen = (dwAllConfigDataLen / DATA_PADDING_SIZE + 1) * DATA_PADDING_SIZE;

	pbyEncryptConfigData = new BYTE[dwEncryptConfigDataLen + 2 + 1];		// +2 : CRC 2Byte 추가함.
	memset(pbyEncryptConfigData, 0x00, dwEncryptConfigDataLen + 2 + 1);

	pbyAllConfigData = new BYTE[dwEncryptConfigDataLen + 1];
	memset(pbyAllConfigData, 0x00, dwEncryptConfigDataLen + 1);

	// Key Information
	ConvertStringToHex(CONFIG_KEY_32, byKey_16);

	sprintf((char*)pbyAllConfigData, "%S", strAllConfigData);
	EncryptKeyData(pbyAllConfigData, dwEncryptConfigDataLen, pbyEncryptConfigData, byKey_16);

	nCRC = crc16(pbyEncryptConfigData, dwEncryptConfigDataLen);
	pbyEncryptConfigData[dwEncryptConfigDataLen] = ((nCRC >> 8) & 0xff);
	pbyEncryptConfigData[dwEncryptConfigDataLen + 1] = (nCRC & 0xff);

	dwEncryptConfigDataLen += 2;

	if (!WriteFile(hFile, pbyEncryptConfigData, dwEncryptConfigDataLen, &dwWriten, NULL))
	{
		AfxMessageBox(_T("EncryptConfig WriteFile is failed."));
		bResult = FALSE;
	}

	if(dwEncryptConfigDataLen != dwWriten)
	{
		AfxMessageBox(_T("Data Length is mismatch."));
		bResult = FALSE;
	}

	CloseHandle(hFile);

// 	if (TRUE == bResult)
// 	{
// 		CString strDestFilePath;
// 
// 		strDestFilePath.Format(_T("%s\\%s"), m_strEncryptFileDir, BACKUP_CONFIGFILE_NAME);
// 
// 		int bCopyResult = CopyFile(strFileName, strDestFilePath, FALSE);
// 		strTemp.Format(_T("Backup Config File(BackupConfig.dat) Result: [%d]"), bCopyResult);
// 		AfxMessageBox(strTemp);
// 	}
// 
// 	DeleteFile(strFileName);

	if (pbyAllConfigData != NULL)
	{
		delete[] pbyAllConfigData;
		pbyAllConfigData = NULL;
	}

	if (pbyEncryptConfigData != NULL)
	{
		delete[] pbyEncryptConfigData;
		pbyEncryptConfigData = NULL;
	}

	if (TRUE == bResult)
		AfxMessageBox(_T("Config Encryption is success."));
}


CString CConfigEncryptConverterDlg::MergeConfigData(CString strConfigDirectory, CString strFileName)
{
	CString strTempData;
	CString strFindStartKey, strFindEndKey;
	CString strConfigData;
	CString strFilePath;
	CString strTemp;
	BYTE *pbyConfigData = NULL;
	int nConfigDataLen = 0;
	BOOL bRet = FALSE;

	CFile datFile;
	CFileException e;

	strFilePath.Format(_T("%s\\%s"), strConfigDirectory, strFileName);

	bRet = datFile.Open(strFilePath, CFile::modeRead, &e);
	if (FALSE == bRet)
	{
		strTemp.Format(_T("[%s] file Open is failed."), strFilePath);
		AfxMessageBox(strTemp);
		strConfigData.Empty();

		return strConfigData;
	}

	nConfigDataLen = (int)datFile.GetLength();

	if (0 == nConfigDataLen)
	{
		strTemp.Format(_T("[%s] file length is zero."), strFilePath);
		AfxMessageBox(strTemp);

		datFile.Close();
		strConfigData.Empty();

		return strConfigData;
	}

	pbyConfigData = new BYTE[nConfigDataLen + 1];
	memset(pbyConfigData, 0x00, nConfigDataLen + 1);

	datFile.Read(pbyConfigData, nConfigDataLen);

	datFile.Close();

	strFindStartKey.Format(_T("[%s]\r\n"), strFileName);
	strFindEndKey.Format(_T("[/%s]\r\n"), strFileName);

	strConfigData += strFindStartKey;

	strTempData = pbyConfigData;
	strConfigData += strTempData;

	strConfigData += strFindEndKey;

	if (pbyConfigData != NULL)
	{
		delete[] pbyConfigData;
		pbyConfigData = NULL;
	}

	return strConfigData;
}

void CConfigEncryptConverterDlg::OnBnClickedBtnDecrypt()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BOOL bResult = TRUE;
	BOOL bFileSeparateResult = TRUE;
	CString strFilePath;
	CString strTemp;

	CFile datFile;
	CFileException e;
	CString strAllConfigData;
	BYTE *pbyAllConfigData = NULL;
	BYTE *pbyEncryptConfigData = NULL;
	DWORD dwEncryptDataLen = 0;
	BYTE byKey_16[32] = { 0, };
	BOOL bRet = FALSE;
	unsigned int nCalcCRC = 0, nCRCFromFile = 0;

	strFilePath.Format(_T("%s\\%s"), m_strEncryptFileDir, BACKUP_CONFIGFILE_NAME);

	bRet = datFile.Open(strFilePath, CFile::modeRead, &e);
	if (FALSE == bRet)
	{
		strTemp.Format(_T("MergeFile Open is failed."));
		AfxMessageBox(strTemp);
		return;
	}

	dwEncryptDataLen = (DWORD)datFile.GetLength();

	if (0 == dwEncryptDataLen)
	{
		strTemp.Format(_T("MergeFile data length is zero."));
		AfxMessageBox(strTemp);
		datFile.Close();
		return;
	}

	pbyEncryptConfigData = new BYTE[dwEncryptDataLen + 1];
	memset(pbyEncryptConfigData, 0x00, dwEncryptDataLen + 1);

	datFile.Read(pbyEncryptConfigData, dwEncryptDataLen);

	datFile.Close();

	dwEncryptDataLen -= 2;

	nCRCFromFile = (pbyEncryptConfigData[dwEncryptDataLen] << 8) | pbyEncryptConfigData[dwEncryptDataLen + 1];
	nCalcCRC = crc16(pbyEncryptConfigData, dwEncryptDataLen);

	if (nCRCFromFile == nCalcCRC)
	{
		pbyAllConfigData = new BYTE[dwEncryptDataLen + 1];
		memset(pbyAllConfigData, 0x00, dwEncryptDataLen + 1);

		// Key Information
		ConvertStringToHex(CONFIG_KEY_32, byKey_16);
		DecryptKeyData(pbyEncryptConfigData, dwEncryptDataLen, pbyAllConfigData, byKey_16);

		strAllConfigData = pbyAllConfigData;

		// 1. ADS.CFG
		bFileSeparateResult = SeparateConfigData(m_strConfigFileDir, ADS_CFG_PATH_NAME, strAllConfigData);
		strTemp.Format(_T("Upload Config File(ADS) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
		{
			AfxMessageBox(strTemp);
			bResult = FALSE;
		}

		// 2. BINLIST.CFG
		bFileSeparateResult = SeparateConfigData(m_strConfigFileDir, BINLIST_CFG_PATH_NAME, strAllConfigData);
		strTemp.Format(_T("Upload Config File(BINLIST) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
		{
			AfxMessageBox(strTemp);
			bResult = FALSE;
		}

		// 3. DEVICE.CFG
		bFileSeparateResult = SeparateConfigData(m_strConfigFileDir, DEVICE_CFG_PATH_NAME, strAllConfigData);
		strTemp.Format(_T("Upload Config File(DEVICE) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
		{
			AfxMessageBox(strTemp);
			bResult = FALSE;
		}

		// 4. EMV_CONFIG.CFG
		bFileSeparateResult = SeparateConfigData(m_strConfigFileDir, EAGLE_ATM_EMV_CONFIG_PATH_NAME, strAllConfigData);
		strTemp.Format(_T("Upload Config File(EMV_CONFIG) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
		{
			AfxMessageBox(strTemp);
			bResult = FALSE;
		}

		// 5. EMV_US_COMM_AID.CFG
		bFileSeparateResult = SeparateConfigData(m_strConfigFileDir, EAGLE_ATM_EMV_US_COMM_AID_PATH_NAME, strAllConfigData);
		strTemp.Format(_T("Upload Config File(EMV_US_COMM_AID) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
		{
			AfxMessageBox(strTemp);
			bResult = FALSE;
		}

		// 6. HOST.CFG
		bFileSeparateResult = SeparateConfigData(m_strConfigFileDir, HOST_CFG_PATH_NAME, strAllConfigData);
		strTemp.Format(_T("Upload Config File(HOST) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
		{
			AfxMessageBox(strTemp);
			bResult = FALSE;
		}

		// 7. MESSAGE.CFG
		bFileSeparateResult = SeparateConfigData(m_strConfigFileDir, MESSAGE_CFG_PATH_NAME, strAllConfigData);
		strTemp.Format(_T("Upload Config File(MESSAGE) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
		{
			AfxMessageBox(strTemp);
			bResult = FALSE;
		}

		// 8. OPTION.CFG
		bFileSeparateResult = SeparateConfigData(m_strConfigFileDir, OPTION_CFG_PATH_NAME, strAllConfigData);
		strTemp.Format(_T("Upload Config File(OPTION) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
		{
			AfxMessageBox(strTemp);
			bResult = FALSE;
		}

		// 9. SYSTEM.CFG
		bFileSeparateResult = SeparateConfigData(m_strConfigFileDir, SYSTEM_CFG_PATH_NAME, strAllConfigData);
		strTemp.Format(_T("Upload Config File(SYSTEM) Result: [%d]"), bFileSeparateResult);
		if(FALSE == bFileSeparateResult)
		{
			AfxMessageBox(strTemp);
			bResult = FALSE;
		}

// 		if(TRUE == bResult)
// 		{
// 			BOOL bCopyResult = CUtil::CopyFileInDirectory(strTempConfigDir, _T("*.CFG"), strDestDirectory);
// 			strTemp.Format(_T("Upload Config File(Config files) Result: [%d]"), bCopyResult);
// 			AfxMessageBox(strTemp);
// 		}
// 
// 		CUtil::DeleteFileInDirectory(strTempConfigDir, _T("*.CFG"));

		if (pbyAllConfigData != NULL)
		{
			delete[] pbyAllConfigData;
			pbyAllConfigData = NULL;
		}
	}
	else
	{
		strTemp.Format(_T("CheckSum is mismatch."));
		AfxMessageBox(strTemp);
		bResult = FALSE;
	}

	if (pbyEncryptConfigData != NULL)
	{
		delete[] pbyEncryptConfigData;
		pbyEncryptConfigData = NULL;
	}

	if (TRUE == bResult)
		AfxMessageBox(_T("Config Decryption is success."));
}

BOOL CConfigEncryptConverterDlg::SeparateConfigData(CString strConfigDirectory, CString strFileName, CString strAllConfigData)
{
	int nConfigIndex[2] = { 0, };
	int nUploadConfigDataLen = 0;
	int nUploadConfigDataStartIndex = 0;
	CString strUploadConfigData;
	CString strFilePath;
	CString strFindStartKey, strFindEndKey;
	CString strTemp;
	BYTE *pbyUploadConfigData = NULL;
	BOOL bReturn = TRUE;

	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwWriten = 0;

	strFilePath.Format(_T("%s\\%s"), strConfigDirectory, strFileName);

	strFindStartKey.Format(_T("[%s]"), strFileName);
	strFindEndKey.Format(_T("[/%s]"), strFileName);

	nConfigIndex[0] = strAllConfigData.Find(strFindStartKey);
	nConfigIndex[1] = strAllConfigData.Find(strFindEndKey);

	if ((nConfigIndex[0] != -1) && (nConfigIndex[1] != -1))
	{
		hFile = CreateFile(strFilePath,
			GENERIC_WRITE,          // Open for writing
			0,                      // Do not share
			NULL,                   // No security
			CREATE_ALWAYS,			// create
			FILE_ATTRIBUTE_NORMAL,  // Normal file
			NULL);

		if (INVALID_HANDLE_VALUE == hFile)
		{
			strTemp.Format(_T("[%s] CreateFile is failed."), strFilePath);
			AfxMessageBox(strTemp);
			return FALSE;
		}

		nUploadConfigDataStartIndex = nConfigIndex[0] + strFindStartKey.GetLength() + 2;	// +2 : \r\n 까지 포함하기 위함.
		nUploadConfigDataLen = nConfigIndex[1] - nUploadConfigDataStartIndex;
		strUploadConfigData = strAllConfigData.Mid(nUploadConfigDataStartIndex, nUploadConfigDataLen);

		pbyUploadConfigData = new BYTE[nUploadConfigDataLen + 1];
		memset(pbyUploadConfigData, 0x00, nUploadConfigDataLen + 1);

		sprintf((char*)pbyUploadConfigData, "%S", strUploadConfigData);

		if (!WriteFile(hFile, pbyUploadConfigData, strUploadConfigData.GetLength(), &dwWriten, NULL))
		{
			strTemp.Format(_T("[%s] Upload WriteFile is failed."), strFilePath);
			AfxMessageBox(strTemp);
			bReturn = FALSE;
		}

		CloseHandle(hFile);
	}
	else
	{
		strTemp.Format(_T("[%s] ConfigIndex is not found. nConfigIndex[0]:[%d], nConfigIndex[1]:[%d]"), strFilePath, nConfigIndex[0], nConfigIndex[1]);
		AfxMessageBox(strTemp);
	}

	if (pbyUploadConfigData != NULL)
	{
		delete[] pbyUploadConfigData;
		pbyUploadConfigData = NULL;
	}

	return bReturn;
}

// Make Pack
BYTE CConfigEncryptConverterDlg::MakePack(BYTE bySrc)
{
	BYTE byRet = 0;

	if ((bySrc >= 'a') && (bySrc <= 'f'))
		byRet = ((bySrc - 0x57) & 0x0f);
	else if ((bySrc >= 'A') && (bySrc <= 'F'))
		byRet = ((bySrc - 0x37) & 0x0f);
	else
		byRet = (bySrc & 0x0f);	

	return byRet;
}


void CConfigEncryptConverterDlg::ConvertStringToHex(CString strSrc, BYTE *pByDest)
{
	int nSrcLength = strSrc.GetLength() / 2;

	for(int i = 0; i < nSrcLength; i++)
	{
		pByDest[i] = ((MakePack(strSrc.GetAt(2*i)) << 4)) | ((MakePack(strSrc.GetAt((2*i)+1)) & 0x0F));
	}
}

void CConfigEncryptConverterDlg::EncryptKeyData(BYTE *pbyInData, int nInDataSize, BYTE *pbyOutData, BYTE *pbyKey)
{
	int nRet = 0;
	CDES pDes;
	int nTimes = nInDataSize / 8;
	BYTE byTmp1[8] = {0,}, byTmp2[8] = {0,}, byTmp3[8] = {0,};

	for(int i = 0; i < nTimes; i++)
	{
		memcpy(byTmp1, &pbyInData[i*8], 8);

		pDes.ENCRYPT(byTmp1, byTmp2, pbyKey);
		pDes.DECRYPT(byTmp2, byTmp3, &pbyKey[8]);
		pDes.ENCRYPT(byTmp3, byTmp1, pbyKey);

		memcpy(&pbyOutData[i*8], byTmp1, 8);
	}
}


void CConfigEncryptConverterDlg::DecryptKeyData(BYTE *pbyInData, int nInDataSize, BYTE *pbyOutData, BYTE *pbyKey)
{
	int nRet = 0;
	CDES pDes;
	int nTimes = nInDataSize / 8;
	BYTE byTmp1[8] = {0,}, byTmp2[8] = {0,}, byTmp3[8] = {0,};

	for(int i = 0; i < nTimes; i++)
	{
		memcpy(byTmp1, &pbyInData[i*8], 8);
		pDes.DECRYPT(byTmp1, byTmp2, pbyKey);
		pDes.ENCRYPT(byTmp2, byTmp3, &pbyKey[8]);
		pDes.DECRYPT(byTmp3, byTmp2, pbyKey);

		memcpy(&pbyOutData[i*8], byTmp2, 8);
	}
}

unsigned int CConfigEncryptConverterDlg::crc16 (unsigned char *string, int length ) 
{ 
	unsigned int crc; 
	crc = 0x0000;	  /* initial CRC value 0x0000 */ 

	while (length--) 
	{ 
		crc = ( crc >> 8 ) ^ crc_table[(crc ^ (int) *string++) & 0xff];   
		/* This performs the modulo-2 division by using the look up table. */ 
	} 

	return(crc); /* return the calculated CRC */ 
}
