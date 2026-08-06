#include "StdAfx.h"
#include "DEV_Define.h"
#include "DeviceSimUtil.h"
#include "DevSimPinPadDlg.h"
#include "CrypteraComm.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

#define	DLL_NNAME (L"EPPDLL4CRYPTERA.dll")


typedef int (*_OPENDEVICE)(int, DWORD);
typedef int (*_CLOSEDEVICE)();
typedef int (*_EXECUTECOMMAND)(int, LPBYTE, DWORD, LPBYTE, DWORD&, BOOL, BOOL);
typedef int (*_REGISTEVENTCALLBACKFUNCTION)(EventCallBackFunction);
typedef int (*_GETCOMMANDRESULTDATA)(LPBYTE, int, int*);


_OPENDEVICE OPENDEVICE = NULL;
_CLOSEDEVICE CLOSEDEVICE = NULL;
_EXECUTECOMMAND EXECUTECOMMAND = NULL;
_REGISTEVENTCALLBACKFUNCTION REGISTEVENTCALLBACKFUNCTION = NULL;
_GETCOMMANDRESULTDATA GETCOMMANDRESULTDATA = NULL;


CCrypteraComm::CCrypteraComm(void)
{
	m_hDLL = 0;
	m_hDLL = ::LoadLibrary(EAGLE_ATM_EPP_DLL);
	m_strLastError.Empty();

	// Load EPP Function
	OPENDEVICE = (_OPENDEVICE)GetProcAddress(m_hDLL, L"EPPOpenDevice");

	if(OPENDEVICE == NULL)
	{
		LOG(Error, _T("[CRYPTERA] Load Failed EPPOpenDevice"));
	}

	CLOSEDEVICE = (_CLOSEDEVICE)GetProcAddress(m_hDLL, L"EPPCloseDevice");

	if(CLOSEDEVICE == NULL)
	{
		LOG(Error, _T("[CRYPTERA] Load Failed EPPCloseDevice"));
	}

	EXECUTECOMMAND = (_EXECUTECOMMAND)GetProcAddress(m_hDLL, L"EPPExecuteCommand");

	if(EXECUTECOMMAND == NULL)
	{
		LOG(Error, _T("[CRYPTERA] Load Failed EPPExecuteCommand"));
	}

	REGISTEVENTCALLBACKFUNCTION = (_REGISTEVENTCALLBACKFUNCTION)GetProcAddress(m_hDLL, L"EPPRegistEventCallBackFunction");

	if(REGISTEVENTCALLBACKFUNCTION == NULL)
	{
		LOG(Error, _T("[CRYPTERA] Load Failed EPPRegistEventCallBackFunction"));
	}

	GETCOMMANDRESULTDATA = (_GETCOMMANDRESULTDATA)GetProcAddress(m_hDLL, L"EPPGetCommandResultData");

	if(GETCOMMANDRESULTDATA == NULL)
	{
		LOG(Error, _T("[CRYPTERA] Load Failed EPPGetCommandResultData"));
	}

	m_nEntryKeyStatus = ENTRY_KEY_STATUS_NONE;

	m_nEPPState = 0;

	m_bRemovalSensorStatus = FALSE;

	m_bEPPReady = FALSE;

	m_bEPPStuckState = FALSE;

	m_bPINMasterKey_Injected = FALSE;

	m_bPINWorkingKey_Injected = FALSE;

	m_bMACMasterKey_Injected = FALSE;

	m_bMACWorkingKey_Injected = FALSE;

	m_sRKL_TerminalInfo.Clear();

}

CCrypteraComm::~CCrypteraComm(void)
{
	if(m_hDLL)
		FreeLibrary(m_hDLL);
}

void RKL_TERMINAL_INFO::Clear()
{
	strEPP_SerialNo.Empty();
	strEPP_RanDomNumber.Empty();
}


int CCrypteraComm::OpenDevice(int nPortNum, DWORD dwBaudRate)
{

#if EMULATION_EPP_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	// 0 = success for this API, non-zero = error.
	return CDevSim::Confirm(_T("CREPP"), _T("OpenDevice")) ? 0 : CDevSim::ReadInt(_T("CREPP"), _T("OpenDeviceErrorCode"), -1);
#endif

	int nRet = 0;

	if(OPENDEVICE)
		nRet = OPENDEVICE(nPortNum, dwBaudRate);
	else
		nRet = 0xFFFE;

	if (nRet)
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
	
	m_nEntryKeyStatus = ENTRY_KEY_STATUS_NONE;

	return nRet;
}

int CCrypteraComm::CloseDevice(void)
{

#if EMULATION_EPP_DEVICE
	CDevSimPinPadDlg::Destroy();
	return CDevSim::Confirm(_T("CREPP"), _T("CloseDevice")) ? 0 : CDevSim::ReadInt(_T("CREPP"), _T("CloseDeviceErrorCode"), -1);
#endif

	int nRet = 0;

	if(CLOSEDEVICE)
		nRet = CLOSEDEVICE();
	else
		nRet = 0xFFFE;

	if (nRet)
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));

	m_nEntryKeyStatus = ENTRY_KEY_STATUS_NONE;

	return nRet;
}

int CCrypteraComm::ExecuteCommand(int nCommand, LPBYTE pData, DWORD dwLen, LPBYTE pResp, DWORD &dwRet, BOOL bCheckStatus, BOOL bWaitResponse)
{
	int nRet = 0;

	if(EXECUTECOMMAND)
		nRet = EXECUTECOMMAND(nCommand, pData, dwLen, pResp, dwRet, bCheckStatus, bWaitResponse);
	else
		nRet = 0xFFFE;

	if (nRet)
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
	else
		m_strLastError.Empty();

	return nRet;
}

int CCrypteraComm::RegistEventCallBackFunction(EventCallBackFunction pFunc)
{
#if EMULATION_EPP_DEVICE
	// No EPP DLL on the desktop, so hand the callback to the simulated PIN pad:
	// its buttons then raise EVENT_ENTRY exactly as the device would.
	CDevSimPinPadDlg::SetEventCallback((void*)pFunc);
	return 0;
#endif

	int nRet = 0;

	if(REGISTEVENTCALLBACKFUNCTION)
		nRet = REGISTEVENTCALLBACKFUNCTION(pFunc);
	else
		nRet = 0xFFFE;

	if (nRet)
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));

	return nRet;
}

int CCrypteraComm::GetCommandResultData(LPBYTE lpData, int nBuffLeng, int* nLength)
{
	int nRet = 0;

	if(GETCOMMANDRESULTDATA)
		nRet = GETCOMMANDRESULTDATA(lpData, nBuffLeng, nLength);
	else
		nRet = 0xFFFE;

	if (nRet)
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));

	return nRet;
}

int CCrypteraComm::StartEntryClear()
{
	BYTE pData[128] = { 0, };
	DWORD dwLength = 0;
	int nPos = 0;
	CString str;
	DWORD dwRet = 0;
	BYTE byRet[256] = { 0, };
	unsigned short usDataLeng = 0;
	unsigned long ulData = 0;

	memset(pData, 0, sizeof(pData));

	//1. Entry Mode(Normal Key)///////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned char);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;	dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;		dwLength++;
	pData[nPos++] = 2 & 0x00FF;					dwLength++;

	//2. Minimum Key Length///////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned short);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;	dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;		dwLength++;

	pData[nPos++] = (0 & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = 0 & 0x00FF;					dwLength++;

	//3. Maximum Key Length///////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned short);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;	dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;		dwLength++;

	pData[nPos++] = (0 & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = 0 & 0xFF;					dwLength++;

	//4. Terminate Keys///////////////////////////////////////////////////
	ulData = _tcstoul(_T("0"), NULL, 16);
	usDataLeng = sizeof(unsigned long);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;

	pData[nPos++] = (ulData & 0xFF000000) >> 24;	dwLength++;
	pData[nPos++] = (BYTE)((ulData & 0xFF0000) >> 16);			dwLength++;
	pData[nPos++] = (ulData & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = ulData & 0xFF;					dwLength++;

	//5. Terminate FDKeys///////////////////////////////////////////////////
	ulData = _tcstoul(_T("0"), NULL, 16);
	usDataLeng = sizeof(unsigned long);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;

	pData[nPos++] = (ulData & 0xFF000000) >> 24;	dwLength++;
	pData[nPos++] = (BYTE)((ulData & 0xFF0000) >> 16);			dwLength++;
	pData[nPos++] = (ulData & 0x0000FF00) >> 8;		dwLength++;
	pData[nPos++] = ulData & 0x00FF;				dwLength++;

	//6. Activate Keys///////////////////////////////////////////////////
	ulData = _tcstoul(_T("FFFFFFFF"), NULL, 16);
	usDataLeng = sizeof(unsigned long);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;

	pData[nPos++] = (ulData & 0xFF000000) >> 24;	dwLength++;
	pData[nPos++] = (BYTE)((ulData & 0x00FF0000) >> 16);		dwLength++;
	pData[nPos++] = (ulData & 0x0000FF00) >> 8;		dwLength++;
	pData[nPos++] = ulData & 0x00FF;				dwLength++;

	//7. Activate FDKeys///////////////////////////////////////////////////
	ulData = _tcstoul(_T("FFFFFFFF"), NULL, 16);
	usDataLeng = sizeof(unsigned long);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;

	pData[nPos++] = (ulData & 0xFF000000) >> 24;	dwLength++;
	pData[nPos++] = (BYTE)((ulData & 0x00FF0000) >> 16);		dwLength++;
	pData[nPos++] = (ulData & 0x0000FF00) >> 8;		dwLength++;
	pData[nPos++] = ulData & 0x00FF;				dwLength++;

	//8. Auton End///////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned char);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;
	pData[nPos++] = 0 & 0x00FF;						dwLength++;

	int nRet = ExecuteCommand(CMD_START_ENTRY, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		LOG(Error, _T("CMD_START_ENTRY Fail (%04X)"), (nRet & 0xFFFF));
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
	}

	m_nEntryKeyStatus = ENTRY_KEY_STATUS_ENABLE;

	return nRet;
}


BOOL CCrypteraComm::StartEntryReadPin()
{
#if EMULATION_EPP_DEVICE
	// Secure PIN entry: show the pad in masked mode, where digits are reported
	// without a key code just as the EPP does.
	CDevSimPinPadDlg::Show(TRUE);
	m_nEntryKeyStatus = ENTRY_KEY_STATUS_ENCRYPT;
	return TRUE;
#endif

	DWORD dwLength = 0;
	int nPos = 0;
	CString str;
	DWORD dwRet = 0;
	unsigned short usDataLeng = 0;
	unsigned long ulData = 0;

	BYTE byRet[256];
	BYTE pData[128];

	memset(pData, 0, sizeof(pData));
	memset(byRet, 0, sizeof(byRet));

	//1. Entry Mode(PIN Mode)///////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned char);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;	dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;		dwLength++;
	pData[nPos++] = 0 & 0x00FF;					dwLength++;

	//2. Minimum Key Length///////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned short);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;	dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;		dwLength++;

	pData[nPos++] = (4 & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = 4 & 0x00FF;					dwLength++;

	//3. Maximum Key Length///////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned short);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;	dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;		dwLength++;

	pData[nPos++] = (12 & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = 12 & 0xFF;					dwLength++;

	//4. Terminate Keys///////////////////////////////////////////////////
	ulData = _tcstoul(_T("00000C00"), NULL, 16);	// "ENTER / CANCEL"
	usDataLeng = sizeof(unsigned long);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;

	pData[nPos++] = (ulData & 0xFF000000) >> 24;	dwLength++;
	pData[nPos++] = (BYTE)((ulData & 0xFF0000) >> 16);			dwLength++;
	pData[nPos++] = (ulData & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = ulData & 0xFF;					dwLength++;

	//5. Terminate FDKeys///////////////////////////////////////////////////
	ulData = _tcstoul(_T("0"), NULL, 16);
	usDataLeng = sizeof(unsigned long);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;

	pData[nPos++] = (ulData & 0xFF000000) >> 24;	dwLength++;
	pData[nPos++] = (BYTE)((ulData & 0xFF0000) >> 16);			dwLength++;
	pData[nPos++] = (ulData & 0x0000FF00) >> 8;		dwLength++;
	pData[nPos++] = ulData & 0x00FF;				dwLength++;

	//6. Activate Keys///////////////////////////////////////////////////
	ulData = _tcstoul(_T("00001FFF"), NULL, 16);
	usDataLeng = sizeof(unsigned long);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;

	pData[nPos++] = (ulData & 0xFF000000) >> 24;	dwLength++;
	pData[nPos++] = (BYTE)((ulData & 0x00FF0000) >> 16);		dwLength++;
	pData[nPos++] = (ulData & 0x0000FF00) >> 8;		dwLength++;
	pData[nPos++] = ulData & 0x00FF;				dwLength++;

	//7. Activate FDKeys///////////////////////////////////////////////////
	ulData = _tcstoul(_T("00000000"), NULL, 16);
	usDataLeng = sizeof(unsigned long);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;

	pData[nPos++] = (ulData & 0xFF000000) >> 24;	dwLength++;
	pData[nPos++] = (BYTE)((ulData & 0x00FF0000) >> 16);		dwLength++;
	pData[nPos++] = (ulData & 0x0000FF00) >> 8;		dwLength++;
	pData[nPos++] = ulData & 0x00FF;				dwLength++;

	//8. Auto End///////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned char);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;
	pData[nPos++] = 1 & 0x00FF;						dwLength++;

	int nRet = ExecuteCommand(CMD_START_ENTRY, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		LOG(Error, _T("CMD_START_ENTRY Fail (%04X)"), (nRet & 0xFFFF));
		return FALSE;
	}

	m_nEntryKeyStatus = ENTRY_KEY_STATUS_ENCRYPT;

	return TRUE;
}


CString CCrypteraComm::MakePinBlock(CString strAccountNo)
{
	unsigned short usDataLeng = 0;
	DWORD dwLength = 0;
	int nPos = 0;
	DWORD dwRet = 0;
	BYTE byData = 0;
	unsigned short usValue = 0;
	unsigned short usLeng = 0;

	CString strTemp;

	char szData[64];
	BYTE byRet[128];
	BYTE pData[2048];

	memset(pData, 0, sizeof(pData));
	memset(szData, 0, sizeof(szData));
	memset(byRet, 0, sizeof(byRet));

	LOG(Info, _T("PIN Block Command - Start"));

	//1. PIN Format//////////////////////////////////////////////////
	usValue = 0x0004;	// ISO0 Format

	usDataLeng = sizeof(unsigned short);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	pData[nPos++] = (usValue & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usValue & 0x00FF;						dwLength++;

	//2. Custom Data//////////////////////////////////////////////////
	usDataLeng = strAccountNo.GetLength();
	// [PCI-SSF Fix] szData(64byte) 버퍼 크기를 넘는 sprintf로 인한 스택 버퍼 오버플로우 방지 - 사전 절단
	sprintf(szData, "%S", strAccountNo.Left(sizeof(szData) - 1));

	if(usDataLeng <= 0)
	{
		LOG(Error, _T("Account No is null"));
		return _T("");
	}

	if(usDataLeng > 13)
		usDataLeng = 13;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;
	memcpy(&pData[nPos], szData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//3. Padding Character///////////////////////////////////
	usDataLeng = 0;	// ISO0 Format은 이 Option과 상관없이 무조건 0xF로 padding하므로 의미 없음 (Spec은 byte인데 length는 short type???)

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;
	//pData[nPos++] = byData & 0x00FF;						dwLength++;

	//4. PEK register//////////////////////////////////////////////////
	memset(szData, 0, sizeof(szData));

	strTemp = WORKING_KEYNAME;
	usDataLeng = strTemp.GetLength();
	sprintf(szData, "%S", strTemp.Left(sizeof(szData) - 1));	// [PCI-SSF Fix] 스택 버퍼 오버플로우 방지 - 사전 절단

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;
	memcpy(&pData[nPos], szData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//5. XOR Data////////////////////////////////////////////////// (사용 안함)
	memset(szData, 0, sizeof(szData));
	memset(byRet, 0, sizeof(byRet));
	
	// 추가 확인 필요
	usDataLeng = 0;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	//6. 2nd PEK register//////////////////////////////////////////////////
	memset(szData, 0, sizeof(szData));
	
	usDataLeng = 0;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	nPos += usDataLeng;
	dwLength += usDataLeng;

	memset(byRet, 0, sizeof(byRet));

	int nRet = ExecuteCommand(CMD_READ_PIN_EXT, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		LOG(Error, _T("PIN Block Command is failed (%04X)"), (nRet & 0xFFFF));
	}
	else
	{
		if(dwRet)
		{
			usLeng = byRet[0] * 0x100;
			usLeng += byRet[1];

			strTemp = CUtil::ConvertHexToString(&byRet[2], usLeng);
			LOG(Info, _T("PIN Block Command is success (Length : %d)"), strTemp.GetLength());

			return strTemp;
		}
		else
		{
			LOG(Info, _T("PIN Block Command Data is null"));
		}
	}

	return _T("");
}


int CCrypteraComm::StopEntry()
{
#if EMULATION_EPP_DEVICE
	CDevSimPinPadDlg::Hide();
	m_nEntryKeyStatus = ENTRY_KEY_STATUS_DISABLE;
	return 0;
#endif

	BYTE pData[128] = { 0, };
	DWORD dwLength = 0;
	int nPos = 0;
	CString str;
	DWORD dwRet = 0;
	BYTE byRet[256] = { 0, };

	int nRet = 0;

	nRet = ExecuteCommand(CMD_CANCEL_ENTRY, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		LOG(Error, _T("CMD_CANCEL_ENTRY Fail (%04X)"), (nRet & 0xFFFF));
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
	}

	m_nEntryKeyStatus = ENTRY_KEY_STATUS_DISABLE;

	return nRet;
}

int CCrypteraComm::GetVersionInfo()
{
	BYTE pData[128];
	DWORD dwLength = 0;
	int nPos = 0;
	CString str;
	DWORD dwRet = 0;
	BYTE byRet[256] = { 0, };
	char szData[64] = { 0, };
	TCHAR wszData[64] = { 0, };
	unsigned short usLeng = 0;

	int nRet = 0;

	memset(pData, 0, sizeof(pData));

	nRet = ExecuteCommand(CMD_READ_SW_VERSION, pData, dwLength, byRet, dwRet);
	if(nRet)
	{
		LOG(Error, _T("CMD_READ_SW_VERSION Fail (%04X)"), (nRet & 0xFFFF));
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));

		return nRet;
	}
	else
	{
		if(dwRet)
		{
			usLeng = byRet[0] * 0x100;
			usLeng += byRet[1];

			memset(szData, 0, sizeof(szData));

			if(usLeng < sizeof(szData))
				memcpy(szData, &byRet[2], usLeng);
			else
				memcpy(szData, &byRet[2], sizeof(szData)-1);

			m_strREPP_FW_Version = CString(szData);
		}
	}


	return nRet;
}


int	CCrypteraComm::GetDeviceStatus()
{
	BYTE pData[128];
	DWORD dwLength = 0;
	int nPos = 0;
	DWORD dwRet = 0;
	BYTE byRet[256];

	memset(pData, 0, sizeof(pData));
	memset(byRet, 0, sizeof(byRet));

	//1. Status Level
	unsigned short usDatLeng = sizeof(unsigned char);
	pData[nPos++] = ((usDatLeng & 0xFF00) >> 8) & 0xFF;		dwLength++;			
	pData[nPos++] = (usDatLeng & 0x00FF) & 0xFF;			dwLength++;		
	pData[nPos++] = 0;										dwLength++;

	int nRet = ExecuteCommand(CMD_GET_DEVICE_STATUS, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
		LOG(Error, _T("Cryptera Get Status is failed (%s)"), m_strLastError);

		return 0;
	}

	// Stuck EPP State Check
	if (byRet[14] & 0x01)
		m_bEPPStuckState = TRUE;
	else
		m_bEPPStuckState = FALSE;

	// Removal Sensor Check
	if (byRet[14] & 0x08)
		m_bRemovalSensorStatus = TRUE;	// NORMAL Removal Sensor가 눌린 상태
	else
		m_bRemovalSensorStatus = FALSE;	// ERROR Removal Sensor가 눌리지 않은 상태

	// 0x01 : REMOVED
	// 0x02 : PREACTOVATED
	// 0x03 : ACTIVATED

	if (byRet[2] == 0x01)
		m_nEPPState = EPP_STATE_REMOVED;
	else if (byRet[2] == 0x02)
		m_nEPPState = EPP_STATE_PREACTIVATED;
	else if (byRet[2] == 0x03)
		m_nEPPState = EPP_STATE_ACTIVATED;

	return m_nEPPState;
}


BOOL CCrypteraComm::SetActivate(int nActiveCmd, CString strUserID, CString strActivationCode)
{
	DWORD dwLength = 0;
	int nPos = 0;
	DWORD dwRet = 0;
	CString strTemp;

	char szSpace[8] = {'.', '.', '.', '.', '.', '.', '.', '.'};
	BYTE byRet[256];
	BYTE byCode[16];
	char szData[64];
	BYTE pData[128];

	LOG(Info, _T("Activate Command - Start"));

	memset(pData, 0, sizeof(pData));
	memset(szData, 0x00, sizeof(szData));
	memset(byRet, 0, sizeof(byRet));
	memset(byCode, 0, sizeof(byCode));

	//1. OP ID
	unsigned short nLeng = strUserID.GetLength();

	if(nLeng <= 1)
		return FALSE;

	if(nLeng > 8)
		nLeng = 8;

	sprintf(szData, "%S", strUserID.Left(nLeng));

	if(nLeng < 8)
	{
		memcpy(&szData[nLeng], szSpace, 8-nLeng);
		nLeng = 8;
	}

	pData[nPos++] = (nLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = nLeng & 0x00FF;				dwLength++;
	memcpy(&pData[nPos], szData, nLeng);

	nPos += nLeng;
	dwLength += nLeng;

	//2. Active Code
	if (strActivationCode.IsEmpty() == FALSE)
	{
		strActivationCode.MakeUpper();
		nLeng = strActivationCode.GetLength();

		if(nLeng < 8)
			return FALSE;

		CUtil::ConvertStringToHex(strActivationCode, byCode);

		nLeng = 4;
		pData[nPos++] = (nLeng & 0xFF00) >> 8;		dwLength++;
		pData[nPos++] = nLeng & 0x00FF;				dwLength++;
		memcpy(&pData[nPos], byCode, nLeng);

		nPos += nLeng;
		dwLength += nLeng;
	}
	else
	{
		nLeng = 0;
		pData[nPos++] = (nLeng & 0xFF00) >> 8;		dwLength++;
		pData[nPos++] = nLeng & 0x00FF;				dwLength++;
	}

	int nRet = ExecuteCommand(nActiveCmd, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
		LOG(Error, _T("Activate Command - Fail (%s)"), m_strLastError);

		return FALSE;
	}
	else
	{
		if(dwRet)
		{
			LOG(Info, _T("Activate Command - Success"));
		}
		else
		{
			LOG(Error, _T("Activate Command Data is null"));
		}

		return TRUE;
	}

	return FALSE;
}


BOOL CCrypteraComm::Remove_RegisteredMasterKey(CString strMasterKeyName)
{
	unsigned short usDataLeng = 0;
	DWORD dwLength = 0;
	int nPos = 0;
	DWORD dwRet = 0;

	char szData[64];
	BYTE pData[2048];
	BYTE byRet[128];
	CString strTemp;

	LOG(Info, _T("Remove Register Command - Start"));


	//1. Target register//////////////////////////////////////////////////
	memset(pData, 0, sizeof(pData));
	memset(szData, 0, sizeof(szData));
	memset(byRet, 0, sizeof(byRet));

	// [PCI-SSF Fix] 스택 버퍼 오버플로우 및 szData 범위를 넘는 memcpy 읽기 방지 - 사전 절단
	sprintf(szData, "%S", strMasterKeyName.Left(sizeof(szData) - 1));
	usDataLeng = strMasterKeyName.GetLength();
	if (usDataLeng > sizeof(szData) - 1)
		usDataLeng = sizeof(szData) - 1;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	if(usDataLeng)
		memcpy(&pData[nPos], szData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	int nRet = ExecuteCommand(CMD_REMOVE_REGISTER, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		// Registered 된 Key가 없는 경우에는 성공 처리 필요 [$$$]
		LOG(Info, _T("Remove Register Command is failed (%04X)"), (nRet & 0xFFFF));
		return FALSE;
	}
	else
	{
		if(dwRet)
		{
			strTemp = CUtil::ConvertHexToString(byRet, dwRet);
			LOG(Info, _T("Remove Register Command Success"));

			// Clear Injected Status
			m_bPINMasterKey_Injected = FALSE;
			m_bPINWorkingKey_Injected = FALSE;

			return TRUE;
		}
		else
		{
			LOG(Info, _T("Remove Register Command Data is null"));
		}
	}

	return FALSE;
}


int CCrypteraComm::StartEntry_Master_PartKey()
{
	DWORD dwLength = 0;
	int nPos = 0;
	CString str;
	DWORD dwRet = 0;
	BYTE byRet[256];
	BYTE pData[128];
	unsigned short usDataLeng = 0;
	unsigned long ulData = 0;

	memset(pData, 0, sizeof(pData));
	memset(byRet, 0, sizeof(byRet));

	LOG(Info, _T("StartEntry_Master_PartKey - Start"));

	//1. Entry Mode(Enter key Part)///////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned char);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;	dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;		dwLength++;
	pData[nPos++] = 1 & 0x00FF;					dwLength++;

	//2. Minimum Key Length///////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned short);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;	dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;		dwLength++;

	pData[nPos++] = (0 & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = 32 & 0x00FF;				dwLength++;

	//3. Maximum Key Length///////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned short);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;	dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;		dwLength++;

	pData[nPos++] = (0 & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = 32 & 0xFF;					dwLength++;

	//4. Terminate Keys///////////////////////////////////////////////////
	ulData = _tcstoul(_T("FFFFFFFF"), NULL, 16);
	usDataLeng = sizeof(unsigned long);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;

	pData[nPos++] = (ulData & 0xFF000000) >> 24;	dwLength++;
	pData[nPos++] = (BYTE)((ulData & 0xFF0000) >> 16);			dwLength++;
	pData[nPos++] = (ulData & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = ulData & 0xFF;					dwLength++;

	//5. Terminate FDKeys///////////////////////////////////////////////////
	ulData = _tcstoul(_T("0"), NULL, 16);
	usDataLeng = sizeof(unsigned long);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;

	pData[nPos++] = (ulData & 0x00000000) >> 24;	dwLength++;
	pData[nPos++] = (BYTE)((ulData & 0x000000) >> 16);			dwLength++;
	pData[nPos++] = (ulData & 0x00000000) >> 8;		dwLength++;
	pData[nPos++] = ulData & 0x0000;				dwLength++;

	//6. Activate Keys///////////////////////////////////////////////////
	ulData = _tcstoul(_T("FFFFFFFF"), NULL, 16);
	usDataLeng = sizeof(unsigned long);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;

	pData[nPos++] = (ulData & 0xFF000000) >> 24;	dwLength++;
	pData[nPos++] = (BYTE)((ulData & 0x00FF0000) >> 16);		dwLength++;
	pData[nPos++] = (ulData & 0x0000FF00) >> 8;		dwLength++;
	pData[nPos++] = ulData & 0x00FF;				dwLength++;

	//7. Activate FDKeys///////////////////////////////////////////////////
	ulData = _tcstoul(_T("FFFFFFFF"), NULL, 16);
	usDataLeng = sizeof(unsigned long);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;

	pData[nPos++] = (ulData & 0xFF000000) >> 24;	dwLength++;
	pData[nPos++] = (BYTE)((ulData & 0x00FF0000) >> 16);		dwLength++;
	pData[nPos++] = (ulData & 0x0000FF00) >> 8;		dwLength++;
	pData[nPos++] = ulData & 0x00FF;				dwLength++;

	//8. Auton End///////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned char);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;
	pData[nPos++] = 1 & 0x00FF;						dwLength++;

	int nRet = ExecuteCommand(CMD_START_ENTRY, pData, dwLength, byRet, dwRet);
	if(nRet)
	{
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
		LOG(Error, _T("CMD_START_ENTRY  - Fail (%s)"), m_strLastError);

		return FALSE;
	}

	LOG(Info, _T("StartEntry_Master_PartKey - End"));
	
	return TRUE;
}


int	CCrypteraComm::GetKeyBlockHeaderData(KEYBLOCKHEADER *pHeader, int nType)
{
	int nData = 0;
	int nLeng = 0;
	USHORT usKBSize = 0;
	USHORT usOptSize = 0;

	//1. KBH Version
	pHeader->byVersion = 0x30;

	//2. KB Size
	pHeader->bySize[0] = 0x00;
	pHeader->bySize[1] = 0x0C;

	//3. Register Usage
	if (nType == 0)
	{
		// Store Part
		pHeader->byUsage[0] = 0x80;
		pHeader->byUsage[1] = 0x00;
		pHeader->byUsage[2] = 0x00;
		pHeader->byUsage[3] = 0x02;
	}
	else if (nType == 1)
	{
		// Store Final
		pHeader->byUsage[0] = 0x00;
		pHeader->byUsage[1] = 0x00;
		pHeader->byUsage[2] = 0x00;
		pHeader->byUsage[3] = 0x02;
	}
	else if (nType == 3)
	{
		pHeader->byUsage[0] = 0x00;
		pHeader->byUsage[1] = 0x00;
		pHeader->byUsage[2] = 0x00;
		pHeader->byUsage[3] = 0x01;
	}
	else
	{
		// Pin Working Key
		pHeader->byUsage[0] = 0x00;
		pHeader->byUsage[1] = 0x00;
		pHeader->byUsage[2] = 0x00;
		pHeader->byUsage[3] = 0x04;
	}


	//4. Register Type (TDES)
	if (nType == 3)
		pHeader->byType = 0x0A;
	else
		pHeader->byType = 0x03;

	//5. Access Righsts
	if (nType == 3)
		pHeader->byAccess = 0x0C;
	else
		pHeader->byAccess = 0x06;

	//6. Write Methods
	if (nType == 2)
	{
		pHeader->byWrMethod[0] = 0x00;
		pHeader->byWrMethod[1] = 0x02;
	}
	else if (nType == 3)
	{
		pHeader->byWrMethod[0] = 0x00;
		pHeader->byWrMethod[1] = 0x40;
	}
	else
	{
		pHeader->byWrMethod[0] = 0x00;
		pHeader->byWrMethod[1] = 0x08;
	}

	//7. Optinla Block Count
	pHeader->byOption = 0x00;

	return 0;
}


BOOL CCrypteraComm::Store_Master_PartKey()
{
	unsigned short usDataLeng = 0;
	DWORD dwLength = 0;
	int nPos = 0;
	char szData[64];
	DWORD dwRet = 0;

	BYTE byRet[128];
	BYTE pData[2048];
	CString strTemp;

	LOG(Info, _T("Store_Master_PartKey - Start"));

	memset(byRet, 0, sizeof(byRet));
	memset(pData, 0, sizeof(pData));

	//1. Key Block//////////////////////////////////////////////////////////////////
	KEYBLOCKHEADER sKBH;

	memset(pData, 0, sizeof(pData));
	memset(&sKBH, 0, sizeof(KEYBLOCKHEADER));

	GetKeyBlockHeaderData(&sKBH, 0);

	usDataLeng = sizeof(KEYBLOCKHEADER);

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;
	memcpy(&pData[nPos], &sKBH, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//2. Reg Name/////////////////////////////////////////////////////////////
	strTemp = MASTER_KEYNAME;
	usDataLeng = strTemp.GetLength();
	sprintf(szData, "%S", strTemp.Left(sizeof(szData) - 1));	// [PCI-SSF Fix] 스택 버퍼 오버플로우 방지 - 사전 절단

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;
	if(usDataLeng)
		memcpy(&pData[nPos], szData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//3. KCV Mode/////////////////////////////////////////////////////////////////
	unsigned short usData = 0x00;
	usDataLeng = sizeof(unsigned short);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	pData[nPos++] = (usData & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usData & 0x00FF;				dwLength++;

	//4. KCV Value////////////////////////////////////////////////////////////////
	memset(byRet, 0, sizeof(byRet));
	memset(szData, 0, sizeof(szData));

	usDataLeng = 0;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	// Length가 0인경우에는 값은 넣지 않고 length 정보만 입력한다.
	nPos += usDataLeng;
	dwLength += usDataLeng;

	int nRet = ExecuteCommand(CMD_STORE_ENTRY, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
		LOG(Error, _T("CMD_STORE_ENTRY  - Fail (%s)"), m_strLastError);

		return FALSE;
	}

	LOG(Info, _T("Store_Master_FinalKey - End"));

	return TRUE;
}


BOOL CCrypteraComm::Store_Master_FinalKey()
{
	unsigned short usDataLeng = 0;
	DWORD dwLength = 0;
	int nPos = 0;
	char szData[64];
	DWORD dwRet = 0;

	BYTE byRet[128];
	BYTE pData[2048];
	CString strTemp;

	LOG(Info, _T("Store_Master_FinalKey - Start"));

	memset(byRet, 0, sizeof(byRet));
	memset(pData, 0, sizeof(pData));

	//1. Key Block//////////////////////////////////////////////////////////////////
	KEYBLOCKHEADER sKBH;

	memset(pData, 0, sizeof(pData));
	memset(&sKBH, 0, sizeof(KEYBLOCKHEADER));

	GetKeyBlockHeaderData(&sKBH, 1);

	usDataLeng = sizeof(KEYBLOCKHEADER);

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;
	memcpy(&pData[nPos], &sKBH, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//2. Reg Name/////////////////////////////////////////////////////////////
	strTemp = MASTER_KEYNAME;
	usDataLeng = strTemp.GetLength();
	sprintf(szData, "%S", strTemp.Left(sizeof(szData) - 1));	// [PCI-SSF Fix] 스택 버퍼 오버플로우 방지 - 사전 절단

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;
	if(usDataLeng)
		memcpy(&pData[nPos], szData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//3. KCV Mode/////////////////////////////////////////////////////////////////
	unsigned short usData = 0x00;
	usDataLeng = sizeof(unsigned short);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	pData[nPos++] = (usData & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usData & 0x00FF;				dwLength++;

	//4. KCV Value////////////////////////////////////////////////////////////////
	usDataLeng = 0;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	// Length가 0인경우에는 값은 넣지 않고 length 정보만 입력한다.

	nPos += usDataLeng;
	dwLength += usDataLeng;

	int nRet = ExecuteCommand(CMD_STORE_ENTRY, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
		LOG(Error, _T("CMD_STORE_ENTRY Final  - Fail (%s)"), m_strLastError);

		return FALSE;
	}

	m_bPINMasterKey_Injected = TRUE;

	LOG(Info, _T("Store_Master_FinalKey - End"));

	return TRUE;
}


BOOL CCrypteraComm::Store_PIN_WorkingKey(CString strPINWorkingKey)
{
	unsigned short usDataLeng = 0;
	DWORD dwLength = 0;
	int nPos = 0;
	DWORD dwRet = 0;
	BYTE byRet[128];
	BYTE pData[2048];
	char szData[64];
	BYTE byRegData[64];
	CString strTemp;

	LOG(Info, _T("Store_PIN_WorkingKey - Start"));

	memset(pData, 0, sizeof(pData));
	memset(szData, 0, sizeof(szData));
	memset(byRet, 0, sizeof(byRet));
	memset(byRegData, 0, sizeof(byRegData));

	//1. Target register//////////////////////////////////////////////////
	strTemp = WORKING_KEYNAME;
	usDataLeng = strTemp.GetLength();
	sprintf(szData, "%S", strTemp.Left(sizeof(szData) - 1));	// [PCI-SSF Fix] 스택 버퍼 오버플로우 방지 - 사전 절단

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	if(usDataLeng)
		memcpy(&pData[nPos], szData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//2. Parent Reg Name/////////////////////////////////////////////////////////////
	memset(szData, 0, sizeof(szData));
	
	strTemp = MASTER_KEYNAME;
	usDataLeng = strTemp.GetLength();
	sprintf(szData, "%S", strTemp.Left(sizeof(szData) - 1));	// [PCI-SSF Fix] 스택 버퍼 오버플로우 방지 - 사전 절단

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	if(usDataLeng)
		memcpy(&pData[nPos], szData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//3. Length of the Register/////////////////////////////////////////////////////
	unsigned short usValue = 0;
	usDataLeng = sizeof(unsigned short);
	usValue = strPINWorkingKey.GetLength() / 2;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	pData[nPos++] = (usValue & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usValue & 0x00FF;						dwLength++;

	//4. Key Block Header//////////////////////////////////////////////////////////////////
	KEYBLOCKHEADER sKBH;

	memset(&sKBH, 0, sizeof(KEYBLOCKHEADER));

	GetKeyBlockHeaderData(&sKBH, 2);

	usDataLeng = sizeof(KEYBLOCKHEADER);

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;
	memcpy(&pData[nPos], &sKBH, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//5. Payload data////////////////////////////////////////////////
	memset(szData, 0, sizeof(szData));
	memset(byRegData, 0, sizeof(byRegData));

	usDataLeng = strPINWorkingKey.GetLength();

	if(usDataLeng)
	{
		CUtil::ConvertStringToHex(strPINWorkingKey, byRegData);
		usDataLeng = (usDataLeng+1) / 2;
	}

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	if(usDataLeng)
		memcpy(&pData[nPos], byRegData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//6. Control Value = 0 //////////////////////////////////////////////////////
	usDataLeng = 0;
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	//7. Create Option///////////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned char);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;
	pData[nPos++] = 1;		dwLength++;

	//8. Crypto Method/////////////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned char);

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;
	pData[nPos++] = 0x01;								dwLength++;


	//9. KCV Mode/////////////////////////////////////////////////////////////////
	unsigned short usData = 0;
	usDataLeng = sizeof(unsigned short);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	pData[nPos++] = (usData & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usData & 0x00FF;				dwLength++;

	//10. KCV Value////////////////////////////////////////////////////////////////
	usDataLeng = 0;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	nPos += usDataLeng;
	dwLength += usDataLeng;


	int nRet = ExecuteCommand(CMD_WRITE_REGISTER, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
		LOG(Error, _T("Store_PIN_WorkingKey  - Fail (%s)"), m_strLastError);

		return FALSE;
	}

	m_bPINWorkingKey_Injected = TRUE;

	LOG(Info, _T("Store_PIN_WorkingKey - End"));

	return TRUE;
}


CString CCrypteraComm::Get_KeyCheckValue(CString strKeyName)
{
	unsigned short usDataLeng = 0;
	DWORD dwLength = 0;
	int nPos = 0;
	DWORD dwRet = 0;

	BYTE byRet[128];
	char szData[64];
	BYTE pData[2048];
	CString strTemp;

	LOG(Info, _T("GetKCV Command - Start"));

	//1. Target register//////////////////////////////////////////////////
	memset(pData, 0, sizeof(pData));
	memset(szData, 0, sizeof(szData));
	memset(byRet, 0, sizeof(byRet));

	usDataLeng = strKeyName.GetLength();
	// [PCI-SSF Fix] 스택 버퍼 오버플로우 및 szData 범위를 넘는 memcpy 읽기 방지 - 사전 절단
	if (usDataLeng > sizeof(szData) - 1)
		usDataLeng = sizeof(szData) - 1;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	sprintf(szData, "%S", strKeyName.Left(sizeof(szData) - 1));
	memcpy(&pData[nPos], szData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	int nRet = ExecuteCommand(CMD_KEY_CHECK_VALUE, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		//m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));	// Check Sum Error는 Set하지 않음
		//LOG(Error, _T("Get CheckSum Value  - Fail (%s)"), m_strLastError);
		LOG(Error, _T("Get CheckSum Value  - Fail (%04X)"), nRet);

		return _T("");
	}
	else
	{
		if(dwRet)
		{
			strTemp = CUtil::ConvertHexToString(&byRet[2], 3);
			LOG(Info, _T("Get KCV is success (%s)"), strTemp);

			// Check Key Injected Status
			if (strKeyName == MASTER_KEYNAME)
				m_bPINMasterKey_Injected = TRUE;
			else if (strKeyName == WORKING_KEYNAME)
				m_bPINWorkingKey_Injected = TRUE;

			return strTemp;
		}
		else
		{
			// Check Key Injected Status
			if (strKeyName == MASTER_KEYNAME)
				m_bPINMasterKey_Injected = FALSE;
			else if (strKeyName == WORKING_KEYNAME)
				m_bPINWorkingKey_Injected = FALSE;

			LOG(Info, _T("Get KCV Data is null"));
		}
	}

	LOG(Info, _T("GetKCV Command - End"));

	return _T("");
}


CString	CCrypteraComm::GetEPPSerialNo()
{
	DWORD dwLength = 0;
	int nPos = 0;
	CString strEPPID;
	DWORD dwRet = 0;
	BYTE byRet[256];
	BYTE pData[128];
	char szData[64];
	unsigned short usLeng = 0;

	memset(pData, 0, sizeof(pData));
	memset(byRet, 0, sizeof(byRet));
	memset(szData, 0, sizeof(szData));

	LOG(Info, _T("Get ID Command Start"));

	int nRet = ExecuteCommand(CMD_READ_DEVICE_ID, pData, dwLength, byRet, dwRet);
	if(nRet)
	{
		LOG(Info, _T("Get ID Command is Failed (%04X)"), (nRet & 0xFFFF));
		return _T("");
	}
	else
	{
		if(dwRet)
		{
			usLeng = byRet[0] * 0x100;
			usLeng += byRet[1];

			memset(szData, 0, sizeof(szData));

			if(usLeng < sizeof(szData))
				memcpy(szData, &byRet[2], usLeng);
			else
				memcpy(szData, &byRet[2], sizeof(szData)-1);

			strEPPID.Format(_T("%S"), szData);
			LOG(Info, _T("Get ID Command is successful - %s"), strEPPID);
		}
		else
		{
			LOG(Info, _T("Get ID Command data is null"));
		}
	}

	LOG(Info, _T("Get ID Command End"));

	return strEPPID;
}


CString	CCrypteraComm::GetEPPNonCEValue(int nType)
{
	DWORD dwLength = 0;
	int nPos = 0;
	CString strNonceValue;
	DWORD dwRet = 0;
	unsigned short usDataLeng = 0;

	BYTE pData[128];
	BYTE byRet[256];
	char szTmp[256];
	char szReg[] = "ACTIVATION_NONCE";
	char szReg2[] = "RKL_NONCE";

	memset(szTmp, 0, sizeof(szTmp));
	memset(pData, 0, sizeof(pData));
	memset(byRet, 0, sizeof(byRet));

	if (nType == 0)
		strncpy_s(szTmp, sizeof(szTmp), szReg, strlen(szReg));
	else
		strncpy_s(szTmp, sizeof(szTmp), szReg2, strlen(szReg2));

	LOG(Info, _T("Get NONCE Command Start nType(%d)"), nType);

	//1. NONCE Reg Name///////////////////////////////////////////////////
	usDataLeng = strlen(szTmp);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;		dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;			dwLength++;
	memcpy(&pData[nPos], szTmp, usDataLeng);
	dwLength += usDataLeng;
	nPos += usDataLeng;

	int nRet = ExecuteCommand(CMD_GENERATE_NONCE, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		LOG(Info, _T("Get NONCE Command is failed (%04X)"), (nRet & 0xFFFF));
		return _T("");
	}
	else
	{
		if(dwRet)
		{
			usDataLeng = byRet[0] * 0x100;
			usDataLeng += byRet[1];

			strNonceValue = CUtil::ConvertHexToString(&byRet[2], usDataLeng);
			LOG(Info, _T("Get NONCE Command is successful - %s"), strNonceValue);
		}
		else
		{
			LOG(Info, _T("Get NONCE Command Data is null"));
		}
	}

	LOG(Info, _T("Get NONCE Command End"));

	return strNonceValue;
}

int CCrypteraComm::SetRegisterValue(CString strRegisterName, CString strValue)
{
	unsigned short usDataLeng = 0;
	BYTE pData[2048];
	DWORD dwLength = 0;
	int nPos = 0;
	char szData[64];
	TCHAR wszData[64] = {0};
	DWORD dwRet = 0;
	BYTE byRet[128];
	CString str;

	//1. Target register//////////////////////////////////////////////////
	memset(pData, 0, sizeof(pData));
	memset(szData, 0, sizeof(szData));
	memset(wszData, 0, sizeof(wszData));

	usDataLeng = strRegisterName.GetLength();
	if(usDataLeng <= 1)
	{
		LOG(Error, _T("Registry Key Length Error"))
		return -1;
	}

	if(usDataLeng > sizeof(wszData))
	{
		LOG(Error, _T("Registry Key Length Buffer overflow"))
		return -1;
	}

	_tcsncpy_s(wszData, sizeof(wszData)/sizeof(TCHAR), strRegisterName, strRegisterName.GetLength());
	WideCharToMultiByte(CP_ACP, 0, wszData, -1, szData, sizeof(szData), NULL, NULL);

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	if(usDataLeng)
		memcpy(&pData[nPos], szData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//2. Parent Reg Name/////////////////////////////////////////////////////////////
	memset(szData, 0, sizeof(szData));
	memset(wszData, 0, sizeof(wszData));

	usDataLeng = 0;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	if(usDataLeng)
		memcpy(&pData[nPos], szData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//3. Length of the Register/////////////////////////////////////////////////////
	unsigned short usValue = 0;
	usDataLeng = sizeof(unsigned short);
	usValue = 2;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	pData[nPos++] = (usValue & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usValue & 0x00FF;						dwLength++;

	//4. Key Block Header//////////////////////////////////////////////////////////////////
	KEYBLOCKHEADER sKBH;

	memset(&sKBH, 0, sizeof(KEYBLOCKHEADER));

	int nRet = GetKeyBlockHeaderData(&sKBH, 3);

	if(nRet)
	{
		LOG(Error, _T("GetKeyBlockHeaderData error : (%d)"), nRet);
		return -1;
	}

	usDataLeng = sizeof(KEYBLOCKHEADER);

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;
	memcpy(&pData[nPos], &sKBH, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//5. Payload data////////////////////////////////////////////////
	BYTE byRegData[64];
	memset(szData, 0, sizeof(szData));
	memset(wszData, 0, sizeof(wszData));
	memset(byRegData, 0, sizeof(byRegData));

	usDataLeng = 2;
	CUtil::ConvertStringToHex(strValue, byRegData);

	LOG(Info, _T("Registry Value (0x%02X 0x%02X)"), byRegData[0], byRegData[1]);

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	if(usDataLeng)
		memcpy(&pData[nPos], byRegData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//6. Control Value = 0 //////////////////////////////////////////////////////
	usDataLeng = 0;
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	//7. Create Option///////////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned char);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;
	pData[nPos++] = 0;									dwLength++;

	//8. Crypto Method/////////////////////////////////////////////////////////
	usDataLeng = 0;
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	//9. KCV Mode/////////////////////////////////////////////////////////////////
	usDataLeng = sizeof(unsigned short);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	pData[nPos++] = 0;									dwLength++;
	pData[nPos++] = 0;									dwLength++;

	//10. KCV Value////////////////////////////////////////////////////////////////
	memset(szData, 0, sizeof(szData));
	memset(wszData, 0, sizeof(wszData));

	usDataLeng = 0;
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;			dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;				dwLength++;

	nPos += usDataLeng;
	dwLength += usDataLeng;

	nRet = ExecuteCommand(CMD_WRITE_REGISTER, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		LOG(Error, _T("Set Registry Value => Fail (%04X)"), (nRet & 0xFFFF));
		return -1;
	}
	else
	{
		LOG(Info, _T("Set Registry Value => Success"));

		// Registry 적용을 위해 EPP Reboot 수행
		memset(pData, 0, sizeof(pData));
		memset(byRet, 0, sizeof(byRet));

		dwRet = 0;
		dwLength = 0;

		nRet = ExecuteCommand(CMD_EPP_REBOOT, pData, dwLength, byRet, dwRet);

		if (nRet)
		{
			LOG(Error, _T("EPP Reboot Command is fail (%04X)"), (nRet & 0xFFFF));
			return -2;
		}
		else
		{
			LOG(Info, _T("EPP Reboot is successful"));
		}

	}

	return 0;
}


CString CCrypteraComm::GetRegisterValue(CString strRegisterName)
{
	BYTE pData[2048];
	TCHAR wszData[64] = {0};
	char szData[64] = { 0, };
	BYTE byRet[128] = { 0, };

	unsigned short usDataLeng = 0;

	DWORD dwLength = 0;
	int nPos = 0;
	DWORD dwRet = 0;

	CString strRegName = STUCKKEY_PERIOD;
	CString strReturnValue;

	LOG(Info, _T("Read Register Command - Start"));

	//1. Target register//////////////////////////////////////////////////
	memset(pData, 0, sizeof(pData));
	memset(szData, 0, sizeof(szData));
	memset(wszData, 0, sizeof(wszData));

	usDataLeng = strRegName.GetLength();

	_tcsncpy_s(wszData, sizeof(wszData)/sizeof(TCHAR), strRegName, strRegName.GetLength());
	WideCharToMultiByte(CP_ACP, 0, wszData, -1, szData, sizeof(szData), NULL, NULL);

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	if(usDataLeng)
		memcpy(&pData[nPos], szData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	int nRet = ExecuteCommand(CMD_READ_REGISTER, pData, dwLength, byRet, dwRet);

	if(nRet)
	{
		LOG(Error, _T("Read Registry is failed (%04X)"), (nRet & 0xFFFF));
	}
	else
	{
		// Length Info(2) + Register Type(1) + Length Info(2) + Length(2) + Length Info(2) + Registry Value(2)
		LOG(Info, _T("Get Registry Value Success (0x%02X, 0x%02X)"), byRet[9], byRet[10]);

		// Convert To String
		strReturnValue = CUtil::ConvertHexToString(&byRet[9], 2);
		LOG(Info, _T("Get Registry Value Converted Hex To String (%s)"), strReturnValue);
	}

	return strReturnValue;
}


CString CCrypteraComm::GetLastError( void )
{
	return m_strLastError;
}



unsigned int CCrypteraComm::GetEPPCertificate(char* szCertVal, unsigned short usBufferSize, unsigned short *usCertSize)
{
	unsigned short usDataLeng = 0;
	DWORD dwLength = 0;
	int nPos = 0;
	DWORD dwRet = 0;

	BYTE byRet[2048];
	char szData[64];
	BYTE pData[2048];
	CString strTemp;
	unsigned short usLeng = 0;

	LOG(Info, _T("GetEPPCertificate Command - Start"));

	memset(pData, 0, sizeof(pData));
	memset(szData, 0, sizeof(szData));
	memset(byRet, 0, sizeof(byRet));

	//1. Certificate Type//////////////////////////////////////////////////
	//이 함수 호출 시 Certificate type = 'E' (Encryption Certificate)로 설정하여야 함.
	usDataLeng = sizeof(char);

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	pData[nPos++] = 'E';									dwLength++;

	int nRet = ExecuteCommand(CMD_GET_CERTIFICATE, pData, dwLength, byRet, dwRet);
	if(nRet)
	{
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
		LOG(Error, _T("Get EPP Certificate Value  - Fail (%s)"), m_strLastError);

		return nRet;
	}
	else
	{
		if(dwRet)
		{
			usLeng = byRet[0] * 0x100;
			usLeng += byRet[1];

			if(usLeng > usBufferSize)
			{
				nRet = 0xFF06;
				m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
				return nRet;		//Buffer is small
			}

			memcpy(szCertVal, &byRet[2], usLeng);
			*usCertSize = usLeng;
			LOG(Info, _T("GetEPPCertificate Command is successful"));

			return 0;
		}
		else
		{
			nRet = 0xFFFE;
			LOG(Info, _T("Get EPP Certificate is null"));
			m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
			return nRet;		//Certificate data is null
		}
	}

	LOG(Info, _T("GetEPPCertificate Command - End"));

	return 0;
}


unsigned int CCrypteraComm::LoadHostCertificate(char* szCertVal, unsigned short usCertSize)
{
	unsigned short usDataLeng = 0;
	DWORD dwLength = 0;
	int nPos = 0;
	DWORD dwRet = 0;

	BYTE byRet[2048];
	char szData[64];
	BYTE pData[2048];
	CString strTemp;
	unsigned short usLeng = 0;
	int nRet = 0;
	
	char szHostCertName[] = "HOST_CERT";
	unsigned short usCertNameSize = 0;

	usCertNameSize = strlen(szHostCertName);

	LOG(Info, _T("LoadHostCertificate Command - Start"));

	//Check Certificate Size
	if (usCertSize >= sizeof(pData))
	{
		LOG(Error, _T("Load Host Certificate size(%d) is bigger than buffer size(%d)"), usCertSize, sizeof(pData));
		nRet = 0xFF06;
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
		return nRet;
	}

	memset(pData, 0, sizeof(pData));
	memset(szData, 0, sizeof(szData));
	memset(byRet, 0, sizeof(byRet));


	//1. Certificate Value//////////////////////////////////////////////////
	usDataLeng = usCertSize;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	memcpy(&pData[nPos], szCertVal, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//2. RSA Keypair register //////////////////////////////////////////////////
	usDataLeng = usCertNameSize;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	if(usCertNameSize)
	{
		if ((usCertNameSize + usCertSize + 4) >= sizeof(pData))		//인증서 및 RSA Key pair register 크기의 합이 2048보다 크면 장애 처리 (4는 사이즈 2개)
		{
			LOG(Error, _T("Load Host Certificate name(%d) is bigger than buffer size(%d)"), usCertNameSize, sizeof(pData));
			nRet = 0xFF06;
			m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
			return nRet;
		}
		memcpy(&pData[nPos++], szHostCertName, usDataLeng);

		nPos += usDataLeng;
		dwLength += usDataLeng;
	}

	nRet = ExecuteCommand(CMD_LOAD_CERTIFICATE_EXTENDED, pData, dwLength, byRet, dwRet);
	if(nRet)
	{
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
		LOG(Error, _T("Load Host Certificate  - Fail (%s)"), m_strLastError);
		return nRet;
	}
	else
	{
		if(dwRet)
		{
			usLeng = byRet[0] * 0x100;
			usLeng += byRet[1];

			// Host 인증서 Load 후의 thumbprint값은 필요시 추후 대응 (불필요한 Data라 판단되어 주석처리함)
			//*usthumbsize = usLeng;
			//memcpy(thumbprint, &byRet[2], usLeng);
			LOG(Info, _T("LoadHostCertificate Command is successful"));

			return 0;
		}
		else
		{
			LOG(Info, _T("Load Host Certificate Thumbprint is null"));
			nRet = 0xFF06;
			m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
			return nRet;
		}
	}

	LOG(Info, _T("LoadHostCertificate Command - End"));

	return 0;
}



unsigned int CCrypteraComm::LoadRSAEncryptedKey4RKL(char *szKeyBlock, unsigned short usKeyBlocksize, char *szReceiptMsg, unsigned short *usReceiptMsgSize)
{
	unsigned short usDataLeng = 0;
	DWORD dwLength = 0;
	int nPos = 0;
	DWORD dwRet = 0;

	BYTE byRet[2048];
	char szData[64];
	BYTE pData[2240];
	CString strTemp;
	unsigned short usLeng = 0;
	int nRet = 0;

	KEYBLOCKHEADER sKBH;

	LOG(Info, _T("LoadRSAEncryptedKey4RKL Command - Start"));
	

	memset(pData, 0, sizeof(pData));
	memset(szData, 0, sizeof(szData));
	memset(byRet, 0, sizeof(byRet));
	memset(&sKBH, 0, sizeof(KEYBLOCKHEADER));

	sKBH.byVersion = 0x30;
	sKBH.bySize[1] = 0x0b;
	sKBH.byUsage[3] = 0x02;
	sKBH.byType = 0x03;
	sKBH.byAccess = 0x06;
	sKBH.byWrMethod[1] = 0x01;

	//1. RKL type //////////////////////////////////////////////////
	//0x0 : CRKL, 0x1 : TR34 => RKL에서는 0
	usDataLeng = sizeof(unsigned char);

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	pData[nPos++] = 0 & 0x00FF;								dwLength++;

	//2. Key Block Header //////////////////////////////////////////////////
	usDataLeng = sizeof(KEYBLOCKHEADER);
	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	memcpy(&pData[nPos], (char*)&sKBH, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//3. Register name //////////////////////////////////////////////////
	strTemp = MASTER_KEYNAME;
	usDataLeng = strTemp.GetLength();
	sprintf(szData, "%S", strTemp.Left(sizeof(szData) - 1));	// [PCI-SSF Fix] 스택 버퍼 오버플로우 방지 - 사전 절단

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	memcpy(&pData[nPos], szData, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	//4. Key Block //////////////////////////////////////////////////
	usDataLeng = usKeyBlocksize;

	pData[nPos++] = (usDataLeng & 0xFF00) >> 8;				dwLength++;
	pData[nPos++] = usDataLeng & 0x00FF;					dwLength++;

	memcpy(&pData[nPos], szKeyBlock, usDataLeng);

	nPos += usDataLeng;
	dwLength += usDataLeng;

	nRet = ExecuteCommand(CMD_LOAD_RSA_ENCIPHERED_KEY, pData, dwLength, byRet, dwRet);
	if(nRet)
	{
		m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
		LOG(Error, _T("Load RSA Encrypted Key  - Fail (%s)"), m_strLastError);
		return nRet;
	}
	else
	{
		if(dwRet)
		{
			usLeng = byRet[0] * 0x100;
			usLeng += byRet[1];

			*usReceiptMsgSize = usLeng;
			memcpy(szReceiptMsg, &byRet[2], usLeng);
			LOG(Info, _T("LoadRSAEncryptedKey4RKL Command is successful"));

			return 0;
		}
		else
		{
			LOG(Info, _T("Load RSA Encrypted Key Receipt Message is null"));
			nRet = 0xFF06;
			m_strLastError.Format(_T("E1%04X"), (nRet & 0xFFFF));
			return nRet;
		}
	}

	LOG(Info, _T("LoadRSAEncryptedKey4RKL Command - End"));

	return 0;
}
