#include "stdafx.h"
#include "DEV_Define.h"
#include "DeviceSimUtil.h"
#include "DEV_PRT.h"
#include "DEV_PRT_Define.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleDefine.h"


BOOL	g_bThreadExit = FALSE;

BOOL	m_bPTR_DoingThread = FALSE;

CDEV_PRT::CDEV_PRT()
{
	// DLL 경로 설정
	CString	strPath = _T("");

	// DLL 인스턴스 취득
	m_hDll = LoadLibrary(EAGLE_ATM_PRT_DLL);

	// API 취득
	LibPOS_Open = (POS_Open)GetProcAddress(m_hDll, _T("POS_Open"));

	LibPOS_Close = (POS_Close)GetProcAddress(m_hDll, _T("POS_Close"));

	LibPOS_Reset = (POS_Reset)GetProcAddress(m_hDll, _T("POS_Reset"));

	LibPOS_SetMode = (POS_SetMode)GetProcAddress(m_hDll, _T("POS_SetMode"));

	LibPOS_SetMotionUnit = (POS_SetMotionUnit)GetProcAddress(m_hDll, _T("POS_SetMotionUnit"));

	LibPOS_SetCharSetAndCodePage = (POS_SetCharSetAndCodePage)GetProcAddress(m_hDll, _T("POS_SetCharSetAndCodePage"));

	LibPOS_FeedLine = (POS_FeedLine)GetProcAddress(m_hDll, _T("POS_FeedLine"));

	LibPOS_SetLineSpacing = (POS_SetLineSpacing)GetProcAddress(m_hDll, _T("POS_SetLineSpacing"));

	LibPOS_SetRightSpacing = (POS_SetRightSpacing)GetProcAddress(m_hDll, _T("POS_SetRightSpacing"));

	LibPOS_PreDownloadBmpToRAM = (POS_PreDownloadBmpToRAM)GetProcAddress(m_hDll, _T("POS_PreDownloadBmpToRAM"));

	LibPOS_PreDownloadBmpsToFlash = (POS_PreDownloadBmpsToFlash)GetProcAddress(m_hDll, _T("POS_PreDownloadBmpsToFlash"));

	LibPOS_QueryStatus = (POS_QueryStatus)GetProcAddress(m_hDll, _T("POS_QueryStatus"));

	LibPOS_RTQueryStatus = (POS_RTQueryStatus)GetProcAddress(m_hDll, _T("POS_RTQueryStatus"));

	LibPOS_NETQueryStatus = (POS_NETQueryStatus)GetProcAddress(m_hDll, _T("POS_NETQueryStatus"));

	LibPOS_KickOutDrawer = (POS_KickOutDrawer)GetProcAddress(m_hDll, _T("POS_KickOutDrawer"));

	LibPOS_CutPaper = (POS_CutPaper)GetProcAddress(m_hDll, _T("POS_CutPaper"));

	LibPOS_StartDoc = (POS_StartDoc)GetProcAddress(m_hDll, _T("POS_StartDoc"));

	LibPOS_EndDoc = (POS_EndDoc)GetProcAddress(m_hDll, _T("POS_EndDoc"));

	LibPOS_EndSaveFile = (POS_EndSaveFile)GetProcAddress(m_hDll, _T("POS_EndSaveFile"));

	LibPOS_BeginSaveFile = (POS_BeginSaveFile)GetProcAddress(m_hDll, _T("POS_BeginSaveFile"));

	LibPOS_SetASB = (POS_SetASB)GetProcAddress(m_hDll, _T("POS_SetASB"));

	LibPOS_GetASB = (POS_GetASB)GetProcAddress(m_hDll, _T("POS_GetASB"));

	LibPOS_S_SetAreaWidth = (POS_S_SetAreaWidth)GetProcAddress(m_hDll, _T("POS_S_SetAreaWidth"));

	LibPOS_S_TextOut = (POS_S_TextOut)GetProcAddress(m_hDll, _T("POS_S_TextOut"));

	LibPOS_S_DownloadAndPrintBmp = (POS_S_DownloadAndPrintBmp)GetProcAddress(m_hDll, _T("POS_S_DownloadAndPrintBmp"));

	LibPOS_S_PrintBmpInRAM = (POS_S_PrintBmpInRAM)GetProcAddress(m_hDll, _T("POS_S_PrintBmpInRAM"));

	LibPOS_S_PrintBmpInFlash = (POS_S_PrintBmpInFlash)GetProcAddress(m_hDll, _T("POS_S_PrintBmpInFlash"));

	LibPOS_S_SetBarcode = (POS_S_SetBarcode)GetProcAddress(m_hDll, _T("POS_S_SetBarcode"));

	LibPOS_PL_SetArea = (POS_PL_SetArea)GetProcAddress(m_hDll, _T("POS_PL_SetArea"));

	LibPOS_PL_TextOut = (POS_PL_TextOut)GetProcAddress(m_hDll, _T("POS_PL_TextOut"));

	LibPOS_PL_DownloadAndPrintBmp = (POS_PL_DownloadAndPrintBmp)GetProcAddress(m_hDll, _T("POS_PL_DownloadAndPrintBmp"));

	LibPOS_PL_PrintBmpInRAM = (POS_PL_PrintBmpInRAM)GetProcAddress(m_hDll, _T("POS_PL_PrintBmpInRAM"));

	LibPOS_PL_SetBarcode = (POS_PL_SetBarcode)GetProcAddress(m_hDll, _T("POS_PL_SetBarcode"));

	LibPOS_PL_Print = (POS_PL_Print)GetProcAddress(m_hDll, _T("POS_PL_Print"));

	LibPOS_PL_Clear = (POS_PL_Clear)GetProcAddress(m_hDll, _T("POS_PL_Clear"));

	LibPOS_WriteFile = (POS_WriteFile)GetProcAddress(m_hDll, _T("POS_WriteFile"));

	LibPOS_ReadFile = (POS_ReadFile)GetProcAddress(m_hDll, _T("POS_ReadFile"));

	LibPOS_SetHandle = (POS_SetHandle)GetProcAddress(m_hDll, _T("POS_SetHandle"));

	LibPOS_GetVersionInfo = (POS_GetVersionInfo)GetProcAddress(m_hDll, _T("POS_GetVersionInfo"));

	m_hPort			= NULL;

	m_hWnd			= NULL;

	m_StatusThread	= NULL;

	memset(m_byPreviousPrintStatus, 0xFF, sizeof(m_byPreviousPrintStatus));	// 최초 1번 AP로 상태 값을 전달하기 위해 0xFF값으로 초기화

	memset(m_byPrintStatus, 0, sizeof(m_byPrintStatus));

	m_bPortOpened = FALSE;

	m_nPaperStatus = EAGLE_PRT_PAPER_EMPTY;	// Default Empty
	
	m_nPTRStatus = EAGLE_PRT_ERROR;			// Default Error

	m_strLastError = _T("");

	m_bPrinterIdle = TRUE;

	m_bPTRExecuteQuiryStatus = FALSE;

	m_strPRT_FW_Version = _T("00.00");
}


CDEV_PRT::~CDEV_PRT()
{
	if (m_hDll)
	{
		FreeLibrary(m_hDll);

		m_hDll = NULL;
	}
}


/** **********************************************************
*	@brief		Is DLL Loaded
*	@retval		없음
************************************************************/
BOOL CDEV_PRT::IsDllLoaded(void)
{
#if EMULATION_PTR_DEVICE
	return TRUE;
#endif

	return (NULL != m_hDll);
}


//BOOL CDEV_PRT::PTR_Open(HWND hWnd, LPCTSTR lpName, int nComBaudrate, int nComDataBits, int nComStopBits, int nComParity, int nParam)
BOOL CDEV_PRT::PTR_Open(HWND hWnd, LPCTSTR lpName)
{
#if EMULATION_PTR_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	m_bPortOpened = CDevSim::Confirm(_T("PRT"), _T("PTR_Open"));
	return m_bPortOpened;
#endif

	if (NULL == LibPOS_Open)
	{
		return FALSE;
	}

	// Port Num(ex> "COM1:"), Baudrate, DataBits, StopBits, Parity, FlowControl
	int nComBaudrate = 38400;
	int nComDataBits = 8;
	int nComStopBits = 1;
	int nComParity = 0;
	int nParam = 2;	//XON/XOFF


	m_hPort = LibPOS_Open(lpName, nComBaudrate, nComDataBits, nComStopBits, nComParity, nParam);

	m_hWnd = hWnd;

	if (INVALID_HANDLE_VALUE == m_hPort)
	{
		return FALSE;
	}

	m_StatusThread = AfxBeginThread((AFX_THREADPROC)PrinterStatus, this);

	if (NULL == m_StatusThread)
	{
		return FALSE;
	}

	m_bPortOpened = TRUE;

	return TRUE;
}

// Get Print Status
BOOL CDEV_PRT::GetPrintStatus(BYTE byPrintStatus[4])
{
#if EMULATION_PTR_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	CString strStatus = CDevSim::ReadString(_T("PRT"), _T("PrintStatus"), _T("0000"));
	CT2A szStatus(strStatus);

	memset(byPrintStatus, '0', 4);
	for (int nSimIdx = 0; nSimIdx < 4 && szStatus.m_psz[nSimIdx] != '\0'; nSimIdx++)
		byPrintStatus[nSimIdx] = (BYTE)szStatus.m_psz[nSimIdx];

	return TRUE;
#endif

	memcpy(byPrintStatus, m_byPrintStatus, sizeof(m_byPrintStatus));

	return TRUE;
}


UINT CDEV_PRT::PrinterStatus(LPVOID pParam)
{
#if EMULATION_PTR_DEVICE
	return TRUE;
#endif

	CDEV_PRT* pPRT = (CDEV_PRT*)pParam;

	pPRT->PrinterStatus();

	return 0;
}



void CDEV_PRT::PrinterStatus(void)
{
	int nReturn = 0;
	CString strTemp;
	char Status = 0;

	int nStatusFailCount = 0;
	int nRetryCount = 500;		// 50회 연속 실패시로 수정 (약 40분)

#if EMULATION_PTR_DEVICE
	return;
#endif

	LOG(Info, _T("PRT Status Thread Start"));

	while (TRUE)
	{
		if (m_bPortOpened == FALSE)
		{
			// Port Open Fail시에는 Quiry 하지 않도록 보완
			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_3000MS);
			continue;
		}

		if (g_bThreadExit == TRUE)
			break;

		if (m_bPTRExecuteQuiryStatus == TRUE)		// Print중에는 Status 명령 대기 처리
		{

			m_bPTR_DoingThread = TRUE;

			memset(m_byPrintStatus, 0, sizeof(m_byPrintStatus));
			m_byPrintStatus[0] = 0x24;								// 통신 실패시 함수 return이 정상이고 값이 0으로 채워지면 정상으로 처리되는 Bug Fix (Default : Paper Empty / Print Status Error)

			nReturn = PTR_RTQueryStatus(0, m_byPrintStatus);

			if (nReturn == EAGLE_PRT_SUCCESS)
			{
				nStatusFailCount = 0;	// Command가 한번이라도 성공시 Fail Count 초기화

				// Compare previous print status to current print status.
				if (m_byPreviousPrintStatus[0] != m_byPrintStatus[0])
				{
					LOG(Info, _T("DEV - Printer Status Changed = %02x"), (BYTE)m_byPrintStatus[0]);

					m_byPreviousPrintStatus[0] = m_byPrintStatus[0];

					// Check Paper Status
					if (m_byPrintStatus[0] & 0x04)
					{
						// Paper Empty
						m_nPaperStatus = EAGLE_PRT_PAPER_EMPTY;
						m_strLastError.Format(_T("P00003"));
					}
					else if (m_byPrintStatus[0] & 0x01)
					{
						// Paper Near
						m_nPaperStatus = EAGLE_PRT_PAPER_NEAR;
						m_strLastError.Format(_T("P00002"));
					}
					else
					{
						m_nPaperStatus = EAGLE_PRT_PAPER_NORMAL;
					}

					// Printer Status
					if (m_byPrintStatus[0] & 0x02)
					{
						// Cover Open
						m_nPTRStatus = EAGLE_PRT_ERROR;
						m_strLastError.Format(_T("P00001"));
					}
					else if (m_byPrintStatus[0] & 0x20)
					{
						m_nPTRStatus = EAGLE_PRT_ERROR;
						m_strLastError.Format(_T("P00004"));
					}
					else
					{
						m_nPTRStatus = EAGLE_PRT_NORMAL;
					}

					// Clear Code
					if ((m_nPaperStatus != EAGLE_PRT_PAPER_EMPTY) && (m_nPTRStatus == EAGLE_PRT_NORMAL))
					{
						m_strLastError.Empty();
					}

					::PostMessage(m_hWnd, WM_PRT_NOTIFY, (WPARAM)m_byPrintStatus, NULL);
				}
			}
			else
			{
				nStatusFailCount++;

				if (nStatusFailCount > nRetryCount)
				{
					// 통신장애로 판단
					m_nPTRStatus = EAGLE_PRT_ERROR;
					m_strLastError.Format(_T("P00007"));

					// Error Setting 후 Fail Count 초기화
					nStatusFailCount = 0;
					m_byPreviousPrintStatus[0] = (char)0xFF;	// 이전 값을 초기화

					LOG(Error, _T("PRT - Status Command is Failed (%d)"), nStatusFailCount);
				}
				CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
			}
		}

		//Notice: 1s이하로 설정하면 PRT_FAIL(1002) return
		m_bPTR_DoingThread = FALSE;
		CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);		// 2초로 조정
	}

	LOG(Info, _T("PRT Status Thread End"));
}

int CDEV_PRT::PTR_Close()
{
#if EMULATION_PTR_DEVICE
	return CDevSim::Confirm(_T("PRT"), _T("PTR_Close"));
#endif

	if (!m_hPort)
	{
		return FALSE;
	}

	g_bThreadExit = TRUE;

	if (m_StatusThread != NULL)
	{
		// Event 생성을 안해 하기 로직은 허수임 (주석을 적용해야 하나, 통신 장애시 Thread 종료가 오래 걸려 적용은 보류함)
		WaitForSingleObject(m_StatusThread, EAGLE_SLEEP_INTERVAL_5MIN);

		//DWORD	dwExitCode = STILL_ACTIVE;
		//int		nCount = 0;

		//while(dwExitCode == STILL_ACTIVE)
		//{
		//	GetExitCodeThread(m_StatusThread, &dwExitCode);

		//	if (nCount > 500)	// 5초 대기
		//		break;

		//	nCount++;

		//	CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
		//}

		CloseHandle(m_StatusThread);
		m_StatusThread = NULL;
	}

	if (NULL == LibPOS_Close)
	{
		return FALSE;
	}

	return LibPOS_Close();
}

int CDEV_PRT::PTR_Reset(int nPortType)
{
#if EMULATION_PTR_DEVICE
	return CDevSim::Confirm(_T("PRT"), _T("PTR_Reset"));
#endif

	// SNBC 요청에 의해 Reset Command 변경
	char Cmd_Reboot[4] = { 0, };
	int nSendLen = 3;

	Cmd_Reboot[0] =0x1B;
	Cmd_Reboot[1] =0x01;
	Cmd_Reboot[2] =0x23;

	if (NULL == LibPOS_WriteFile)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_WriteFile(m_hPort, Cmd_Reboot, nSendLen);

	if (EAGLE_PRT_SUCCESS != nReturn)
	{
		LOG(Info, _T("POS_WriteFile(Reset) is failed (%d)"), nReturn);
	}
	else
	{
		LOG(Info, _T("POS_WriteFile(Reset) is successful"));	
	}

	// SNBC에 의하면 Reset 후 적어도 4초 이상 대기해야 함.
	CUtil::Sleep_Wait(5000);	// 5초 Delay 처리

	m_PTRLock.Unlock();

	return nReturn;
}


char Cmd_Status[16] = { 0, };
char Cmd_RecvData[16] = { 0, };
int nRecvTimeOut = 3000;	// 1sec
int nSendLenIndex = 0;
int nSaveReturnValue = 0;


int CDEV_PRT::PTR_RTQueryStatus(int nPortType, char * pszStatus)
{
	// SNBC 요청에 의해 Status 구문 변경
	if (NULL == LibPOS_WriteFile)
	{
		return FALSE;
	}

	if (NULL == LibPOS_ReadFile)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	nSendLenIndex = 0;
	memset(Cmd_Status, 0, sizeof(Cmd_Status));

	Cmd_Status[nSendLenIndex++] =0x10;
	Cmd_Status[nSendLenIndex++] =0x04;
	Cmd_Status[nSendLenIndex++] =0x02;		// Status n=2
	Cmd_Status[nSendLenIndex++] =0x10;
	Cmd_Status[nSendLenIndex++] =0x04;
	Cmd_Status[nSendLenIndex++] =0x03;		// Status n=3
	Cmd_Status[nSendLenIndex++] =0x10;
	Cmd_Status[nSendLenIndex++] =0x04;
	Cmd_Status[nSendLenIndex++] =0x04;		// Status n=4
	Cmd_Status[nSendLenIndex++] =0x10;
	Cmd_Status[nSendLenIndex++] =0x04;
	Cmd_Status[nSendLenIndex++] =0x05;		// Status n=5	// 5번 BYTE에 모든게 다 존재함

	int nReturn = LibPOS_WriteFile(m_hPort, Cmd_Status, nSendLenIndex);

	if (nReturn == EAGLE_PRT_SUCCESS)
	{
		nSaveReturnValue = nReturn;

		memset(Cmd_RecvData, 0, sizeof(Cmd_RecvData));
		nReturn = LibPOS_ReadFile(m_hPort, Cmd_RecvData, 4, nRecvTimeOut);

		if (nReturn == EAGLE_PRT_SUCCESS)
		{
			// Status를 갱신
//			LOG(Info, _T("LibPOS_ReadFile return value (%d) Data (%s)"), nReturn, CUtil::ConvertHexToString((BYTE*)Cmd_RecvData, 4));
			pszStatus[0] = Cmd_RecvData[3];	// n=5번째 status return
		}

//		LOG(Info, _T("LibPOS_ReadFile return value (%d)"), nReturn);
	}
	else
	{
		if (nSaveReturnValue != nReturn)
		{
			nSaveReturnValue = nReturn;
			LOG(Info, _T("POS_WriteFile return value (%d)"), nReturn);
		}
	}	

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_SetMode(int nPortType, int nPrintMode)
{
	if (NULL == LibPOS_SetMode)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_SetMode(nPrintMode);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_StartDoc()
{
	if (NULL == LibPOS_StartDoc)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_StartDoc();

	m_PTRLock.Unlock();

	return nReturn;
}

CString CDEV_PRT::PTR_GetVersionInfo()
{
	if (NULL == LibPOS_GetVersionInfo)
	{
		return FALSE;
	}

	int nMajor = 0, nMinor = 0;

	m_PTRLock.Lock();

	int nReturn = LibPOS_GetVersionInfo(&nMajor, &nMinor);
	
	m_strPRT_FW_Version.Format(_T("%02d.%02d"), nMajor, nMinor);

	m_PTRLock.Unlock();

	return m_strPRT_FW_Version;
}

int CDEV_PRT::PTR_SetMotionUnit(int nPortType, int nHorizontalMU, int nVerticalMU)
{
	if (NULL == LibPOS_SetMotionUnit)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_SetMotionUnit(nHorizontalMU, nVerticalMU);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_P_SetAreaAndDirection(int nPortType, int nOrgx, int nOrgy, int nWidth, int nHeight, int nDirection)
{
	if (NULL == LibPOS_PL_SetArea)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_PL_SetArea(nOrgx, nOrgy, nWidth, nHeight, nDirection);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_SetLineSpacing(int nPortType, int nDistance)
{
	if (NULL == LibPOS_SetLineSpacing)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_SetLineSpacing(nDistance);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_SetRightSpacing(int nPortType, int nDistance)
{
	if (NULL == LibPOS_SetRightSpacing)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_SetRightSpacing(nDistance);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_P_Textout(int nPortType, char *pszData, int nOrgx, int nOrgy, int nWidthTimes, int nHeightTimes, int nFontType, int nFontStyle)
{
	if (NULL == LibPOS_PL_TextOut)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_PL_TextOut(pszData, nOrgx, nOrgy, nWidthTimes, nHeightTimes, nFontType, nFontStyle);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_S_Textout(int nPortType, char *pszData, int nOrgx, int nWidthTimes, int nHeightTimes, int nFontType, int nFontStyle)
{
	if (NULL == LibPOS_S_TextOut)
	{
		LOG(Info, _T("LibPOS_S_TextOut is NULL"));
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_S_TextOut(pszData, nOrgx, nWidthTimes, nHeightTimes, nFontType, nFontStyle);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_CutPaper(int nPortType, int nMode, int nDistance)
{
	if (NULL == LibPOS_CutPaper)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_CutPaper(nMode, nDistance);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_FeedLine(int nPortType)
{
	if (NULL == LibPOS_FeedLine)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_FeedLine();

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_S_SetBitmapFlash(int nPortType, char *pszPaths[], int nCount)
{
#if EMULATION_PTR_DEVICE
	return EAGLE_PRT_SUCCESS;
#endif

	if (NULL == LibPOS_PreDownloadBmpsToFlash)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_PreDownloadBmpsToFlash(pszPaths, nCount);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_S_PrintBitmapFlash(int nPortType, int nID, int nOrgx, int nMode)
{
	if (NULL == LibPOS_S_PrintBmpInFlash)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_S_PrintBmpInFlash(nID, nOrgx, nMode);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_S_SetBitmapRAM(int nPortType, char *pszPaths, int nID)
{
	if (NULL == LibPOS_PreDownloadBmpToRAM)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_PreDownloadBmpToRAM(pszPaths, nID);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_S_PrintBitmapRAM(int nPortType, int nID, int nOrgx, int nDensityMode)
{
	if (NULL == LibPOS_S_PrintBmpInRAM)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_S_PrintBmpInRAM(nID, nOrgx, nDensityMode);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_P_PrintBitmapRAM(int nPortType, int nID, int nOrgx, int nOrgy, int nDensityMode)
{
	if (NULL == LibPOS_PL_PrintBmpInRAM)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_PL_PrintBmpInRAM(nID, nOrgx, nOrgy, nDensityMode);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_S_DownloadPrintBmp(int  nPortType, char *pszPath, int nOrgx, int nMode)
{
	if (NULL == LibPOS_S_DownloadAndPrintBmp)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_S_DownloadAndPrintBmp(pszPath, nOrgx, nMode);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_S_SetAreaWidth(int nPortType, int nWidth)
{
	if (NULL == LibPOS_S_SetAreaWidth)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_S_SetAreaWidth(nWidth);

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_P_Print(int nPortType)
{
	if (NULL == LibPOS_PL_Print)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_PL_Print();

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_P_Clear(int nPortType)
{
	if (NULL == LibPOS_PL_Clear)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_PL_Clear();

	m_PTRLock.Unlock();

	return nReturn;
}

int CDEV_PRT::PTR_WriteFile(char *chSendData, int nSendLen)
{
	if (NULL == LibPOS_WriteFile)
	{
		return FALSE;
	}

	m_PTRLock.Lock();

	int nReturn = LibPOS_WriteFile(m_hPort, chSendData, nSendLen);

	if (EAGLE_PRT_SUCCESS != nReturn)
	{
		LOG(Info, _T("POS_WriteFile(Reset) is failed (%d)"), nReturn);
	}
	else
	{
		LOG(Info, _T("POS_WriteFile(Reset) is successful"));	
	}

	m_PTRLock.Unlock();
	return 0;
}

int CDEV_PRT::PTR_GetPaperStatus(void)
{
#if EMULATION_PTR_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	return CDevSim::ReadInt(_T("PRT"), _T("PaperStatus"), EAGLE_PRT_PAPER_NORMAL);
#endif

	return m_nPaperStatus;
}

int CDEV_PRT::PTR_GetStatus(void)
{
#if EMULATION_PTR_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	return CDevSim::ReadInt(_T("PRT"), _T("DeviceStatus"), EAGLE_PRT_NORMAL);
#endif

	return m_nPTRStatus;
}

// PTR Quiry Status Start
void CDEV_PRT::PTR_QuiryStatusStart()
{
	m_bPTRExecuteQuiryStatus = TRUE;
	//CUtil::Sleep_Wait(1000);

	// Polling Thread가 진행될때까지 대기
	//while(m_bPTR_DoingThread == FALSE)
	//{
	//	CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	//}

	//// Polling이 진행된 후에는 1cycle이 완료될때까지 대기
	//int nWaitCount = 0;
	//while(m_bPTR_DoingThread == TRUE)
	//{
	//	if (m_bPTR_DoingThread == FALSE)
	//		break;

	//	if (nWaitCount > 60)
	//	{
	//		// Polling Thread가 Dead Lock이 걸린 것으로 간주
	//		LOG(Error, _T("PTR_QuiryStatusStop - Failed Start Polling"));
	//		break;
	//	}

	//	nWaitCount++;
	//	CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	//}
}

// PTR Quiry Status Stop
void CDEV_PRT::PTR_QuiryStatusStop()
{
	m_bPTRExecuteQuiryStatus = FALSE;

	int nWaitCount = 0;

	LOG(Info, _T("PTR_QuiryStatusStop - start"));

	while(m_bPTR_DoingThread == TRUE)
	{
		if (m_bPTR_DoingThread == FALSE)
			break;

		if (nWaitCount > 60)
		{
			// Polling Thread가 Dead Lock이 걸린 것으로 간주
			LOG(Error, _T("PTR_QuiryStatusStop - Failed Stop Polling"));
			break;
		}

		nWaitCount++;
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	LOG(Info, _T("PTR_QuiryStatusStop - end"));

}

CString CDEV_PRT::GetLastError(void)
{
	return m_strLastError;
}


