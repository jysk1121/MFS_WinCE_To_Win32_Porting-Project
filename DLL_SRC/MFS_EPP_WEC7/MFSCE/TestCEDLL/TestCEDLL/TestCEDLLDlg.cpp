// TestCEDLLDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestCEDLL.h"
#include "TestCEDLLDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CTestCEDLLDlg dialog
static CTestCEDLLDlg	*pDlgDev;

CTestCEDLLDlg::CTestCEDLLDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestCEDLLDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	pDlgDev = this;
}

void CTestCEDLLDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_PORT_NUM, m_ctrlPortNum);
	DDX_Control(pDX, IDC_LIST_RESULT, m_ctrlResult);
}

BEGIN_MESSAGE_MAP(CTestCEDLLDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CTestCEDLLDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CTestCEDLLDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_GET_VERSION, &CTestCEDLLDlg::OnBnClickedButtonGetVersion)
	ON_BN_CLICKED(IDC_BUTTON_START_ENTRY, &CTestCEDLLDlg::OnBnClickedButtonStartEntry)
	ON_BN_CLICKED(IDC_BUTTON_STOP_ENTRY, &CTestCEDLLDlg::OnBnClickedButtonStopEntry)
	ON_BN_CLICKED(IDOK, &CTestCEDLLDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CTestCEDLLDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CTestCEDLLDlg message handlers

BOOL CTestCEDLLDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	if(	m_ctrlPortNum.GetCount())
		m_ctrlPortNum.SetCurSel(0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CTestCEDLLDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_TESTCEDLL_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_TESTCEDLL_DIALOG));
	}
}
#endif

void CTestCEDLLDlg::ShowResult(CString str)
{
	m_ctrlResult.AddString(str);
	int nCount = m_ctrlResult.GetCount();
	if(nCount > 0)
		m_ctrlResult.SetCurSel(nCount-1);
}

void CTestCEDLLDlg::ShowHexResult(CString strTag, byte *lpByte, unsigned short usDataLength)
{
	CString str, strT;
	str.Empty();
	str = strTag;
	for(int i = 0; i < usDataLength; i++)
	{
		strT.Format(_T("%02X"), (lpByte[i] & 0xFF));
		str += strT;
	}
	m_ctrlResult.AddString(str);

	int nCount = m_ctrlResult.GetCount();
	if(nCount > 0)
		m_ctrlResult.SetCurSel(nCount-1);
}


void CTestCEDLLDlg::OnBnClickedButtonOpen()
{
	CString str;
	ShowResult(_T("Open Port Command"));
	int nNum = 0;
	DWORD dwBaud = 115200;
	str.Empty();

	nNum = m_ctrlPortNum.GetCurSel() + 1;
	int nRet = m_pCommLib.OpenDevice(nNum, dwBaud);
	if(nRet)
		str.Format(_T("  => Fail (%04X)"), (nRet & 0xFFFF));
	else
		str = _T("  => Success");
	ShowResult(str);

	ShowResult(_T("Regist Call Back Function"));
	nRet = m_pCommLib.RegistEventCallBackFunction((EventCallBackFunction)EventControlFunc);
	if(nRet)
		str.Format(_T("  => Fail (%04X)"), (nRet & 0xFFFF));
	else
		str = _T("  => Success");
	ShowResult(str);
}

void CTestCEDLLDlg::OnBnClickedButtonClose()
{
	CString str;
	ShowResult(_T("Close Port Command"));
	int nRet = m_pCommLib.CloseDevice();
	if(nRet)
		str.Format(_T("  => Fail (%04X)"), (nRet & 0xFFFF));
	else
		str = _T("  => Success");
	ShowResult(str);
}

void CTestCEDLLDlg::OnBnClickedButtonGetVersion()
{
	int nRet = 0;
	CString str;
	ShowResult(_T("Send Version Command"));
	DWORD dwRet = 0;
	BYTE byRet[256];

	BYTE pData[128];
	DWORD dwLength = 0;

	memset(pData, 0, sizeof(pData));
	dwLength = 1;
	WORD Packet_length = 0;

	dwLength = 0;

	pData[dwLength++] = 0x02;	//STX
	pData[dwLength++] = 0x30;	Packet_length++;		//MTYPE			
	pData[dwLength++] = 0x31;	Packet_length++;		//CMD
	pData[dwLength++] = 0x31;	Packet_length++;		//Parameter
	pData[dwLength++] = 00;		Packet_length++;		//Length Low
	pData[dwLength++] = 00;		Packet_length++;		//Length High
	pData[dwLength++] = 0x00;		//ETX

	BYTE CRC = CalcLRC (&pData[1], Packet_length);	/* remainder = 0 	*/
	pData[dwLength++] = CRC;

	nRet = m_pCommLib.ExecuteCommand(0x31, pData, dwLength, byRet, dwRet, 1, 1);
	if(nRet)
	{
		str.Format(_T("  => Fail (%04X)"), (nRet & 0xFFFF));
		ShowResult(str);
		return;
	}
	else
	{
		str = _T("  => Success");
		ShowHexResult(str, byRet, (unsigned short)dwRet);
	}
}

void CTestCEDLLDlg::OnBnClickedButtonStartEntry()
{
	BYTE pData[128];
	DWORD dwLength = 0;
	int nPos = 0;
	CString str;

	memset(pData, 0, sizeof(pData));
	WORD wPkt_length = 0;

	pData[0] = 0x02;	//STX
	pData[1] = 0x30;	//MTYPE
	pData[2] = 0xb1;	//CMD
	pData[3] = 0x30;	//Parameter
	pData[4] = 00;		//Length Low
	pData[5] = 00;		//Length High

	pData[6] = 0x03;		//ETX

	int i = 0;
	BYTE CRC = CalcLRC (&pData[1], 5);	/* remainder = 0 	*/
	pData[7] = CRC;

	dwLength = 8;

	ShowResult(_T("Start Entry Command"));
	DWORD dwRet = 0;
	BYTE byRet[128];
	int nRet = m_pCommLib.ExecuteCommand(0xb1, pData, dwLength, byRet, dwRet, 0, 1);
	if(nRet)
	{
		str.Format(_T("  => Fail (%04X)"), (nRet & 0xFFFF));
		ShowResult(str);
		return;
	}
	else
	{
		str = _T("  => Success");
		ShowResult(str);
	}
}

void CTestCEDLLDlg::OnBnClickedButtonStopEntry()
{
	BYTE pData[128];
	DWORD dwLength = 0;
	int nPos = 0;
	CString str;

	memset(pData, 0, sizeof(pData));
	WORD wPkt_length = 0;

	pData[0] = 0x02;	//STX
	pData[1] = 0x30;	//MTYPE
	pData[2] = 0xb1;	//CMD
	pData[3] = 0x30;	//Parameter
	pData[4] = 00;		//Length Low
	pData[5] = 00;		//Length High

	pData[6] = 0x03;		//ETX

	ShowResult(_T("Stop Entry Command"));
	int i = 0;
	BYTE CRC = CalcLRC (&pData[1], 5);	/* remainder = 0 	*/
	pData[7] = CRC;

	dwLength = 8;

	DWORD dwRet = 0;
	BYTE byRet[128];
	int nRet = m_pCommLib.ExecuteCommand(0x42, pData, dwLength, byRet, dwRet, 0, 1);
	if(nRet)
	{
		str.Format(_T("  => Fail (%04X)"), (nRet & 0xFFFF));
		ShowResult(str);
		return;
	}
	else
	{
		str = _T("  => Success");
		ShowResult(str);
	}
}

void CTestCEDLLDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	OnOK();
}

void CTestCEDLLDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	OnCancel();
}

//#define		EVENT_ENTRY				0xA14
//#define		EVENT_FLOAT_FRAME		0xA86
//#define		EVENT_STATUS			0xA1B

/**< notification command definication */
#define		PIN_ENTRY				0x30
#define		PWD_ENTRY				0x31
#define		NPWD_VERIFY				0x32/**< 알림 command */
#define		NORMAL_KEY				0x33/**< Normal key pressed */
#define		SECUKEY_ENTRY			0x34/*Master Key/Session Key를 지원하기 위해 추가 2015.10.27*/


/**< notification parameter definition */
#define		NOTICE_MESSAGE			0x30
#define		EFFECTKEY_PRESS			0x31
#define		KEYIN_COMPLETE			0x32
#define		NEXTIN_CONTINUE			0x33
#define		EXCEPT_MESSAGE			0x34/**< 예외 상황 발생 : key input일때 KEYIN_COMPLETE와 같은 효과
									다만 예외 상황이므로 예외 상황 CODE값을 정의하고 참조 */
/**< noticefication message code */
#define		KEYENTRY_START			0x30
#define		KEYENTRY_LACK			0x31/**< key입력부족(under 4) */
#define		KEYENTRY_OVER			0x32/**< key입력포화(over 12) */

/**< exception code definition */
#define		KEYENTRY_TIMEOUT		0x00
#define		PWDVERIFY_FAIL			0x01
#define		OVERALL_TIMEOUT1		0x02/**< PIN entry overall timeout(30 sec) */
#define		OVERALL_TIMEOUT2		0x03/**< PIN encryption overall timeout(3600 sec) */
//----------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////
//Event Control Functions
int CTestCEDLLDlg::EventControlFunc(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength)
{
	pDlgDev->eventHandler(usEventID, usParam, lpByte, usDataLength);
	return 0;
}

int CTestCEDLLDlg::eventHandler(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength)
{
	CString str;
	str.Format(_T("%04X : "), usEventID & 0xFFFF);
	ShowHexResult(str, lpByte, usDataLength);

	switch(usEventID)
	{
	case NORMAL_KEY:
		PressKeyHandler(usParam, lpByte, usDataLength);
		break;

	case PIN_ENTRY:
		break;

	case PWD_ENTRY:
		break;

	case NPWD_VERIFY:
		break;

	case SECUKEY_ENTRY:
		break;

	default:
		break;
	}

	return 0;
}

int CTestCEDLLDlg::PressKeyHandler(unsigned short usParam, byte *lpByte, unsigned short usDataLength)
{
	if(usDataLength < 1)
		return -1;

	BYTE byParam = (BYTE)usParam;
	BYTE byKey = lpByte[0];

	CString str;
	if(byParam == EFFECTKEY_PRESS)
	{
		if((byKey >= 0x30) && (byKey <= 0x39))
			byKey -= 0x30;
		else if((byKey >= 0x41) && (byKey <= 0x46))
			byKey = byKey - 0x41 + 10;

		str.Format(L"Key Pressed : %X", byKey & 0xFF);
		ShowResult(str);
	}
	return 0;
}

BYTE CTestCEDLLDlg::CalcLRC(unsigned char *pData, DWORD dwLen)
{
	unsigned char	lrc = 0;
	DWORD			i = 0;

	for(; i < dwLen; i++)
		lrc ^= *pData++;

	return lrc;
}

