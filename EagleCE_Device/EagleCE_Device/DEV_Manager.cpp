#include "stdafx.h"
#include "DEV_Define.h"
#include "DEV_DIO_Define.h"
#include "DEV_PRT_Define.h"
#include "DEV_Manager.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleDataManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BOOL		m_bExit;
BOOL		m_bPlay;
int			m_nVolume;
int			m_nlanguage;		// 추후 다국어 Voice 지원을 위해 ... 현재는 영어만 지원

CString		m_strCurrPlayList;

// 인스턴스
CDEV_Manager* CDEV_Manager::m_pInstance = NULL;

CWavePlay		CDEV_Manager::m_WavePlay;
CMFSEvent		CDEV_Manager::m_eStartToPlay(FALSE, FALSE);
CMFSEvent		CDEV_Manager::m_eDoneToPlay(TRUE, TRUE);
CMFSLinkedList< CString >	CDEV_Manager::m_colWaveScrData[6+1];

MEMORYSTATUS memStatus;
#ifdef _WIN32_WCE
STORE_INFORMATION   si;
#endif

DWORD UsingVirtualMemory = 0;
DWORD UsingPhysMemory = 0;

// Config 임시 Buffer 생성 - Host와 Option만 Check
EAGLE_CONFIG_HOST		g_TempHost;
EAGLE_CONFIG_OPTION		g_TempOption;
EAGLE_CONFIG_SYSTEM		g_TempSystem;	// Password Change 추가



BOOL	m_bNetSettingThreadExit = FALSE;
BOOL	m_bExecNetSetting = FALSE;
BOOL	m_bForceReloadLan = FALSE;


/** **********************************************************
*	@brief		인스턴스 생성
*	@retval		없음
************************************************************/
void CDEV_Manager::CreateInstance()
{
	m_pInstance = new CDEV_Manager();
}


/** **********************************************************
*	@brief		인스턴스 취득
*	@retval		없음
************************************************************/
CDEV_Manager* CDEV_Manager::GetInstance()
{
	return m_pInstance;
}


/** **********************************************************
*	@brief		인스턴스 해제
*	@retval		없음
************************************************************/
void CDEV_Manager::ReleaseInstance()
{
	if (m_pInstance)
	{
		delete m_pInstance;
	}
}


/** **********************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CDEV_Manager::CDEV_Manager()
{
	// 통지 윈도우
	m_pNotifyWnd = NULL;

	m_bPlay = FALSE;
	m_nlanguage = 0;
	m_nVolume = 0;

	// 이전 서비스 모드
	m_nPrevServiceMode = 0;

	m_strCurrPlayList.Empty();

	// Network Setting Thread Create
	m_hNetworkSettingThread = NULL;
	m_hNetworkSettingThread = AfxBeginThread((AFX_THREADPROC)Thread_SetNetwork, this);

}


/** **********************************************************
*	@brief		소멸자
*	@retval		없음
************************************************************/
CDEV_Manager::~CDEV_Manager()
{
	m_bNetSettingThreadExit = TRUE;

	if (m_hNetworkSettingThread != NULL)
	{
		::TerminateThread(m_hNetworkSettingThread, 0);
		CloseHandle(m_hNetworkSettingThread);
		m_hNetworkSettingThread = NULL;
	}

	m_bExit = TRUE;
	WaitForExitThread();
}


/** **********************************************************
*	@brief		초기화
*	@retval		없음
************************************************************/
BOOL CDEV_Manager::Initialize()
{
#if (SUPPORT_EMV)
	m_DEV_CDR.Load_AIDs_InformFromFile();				// Load EMV Config
	m_DEV_CDR.Load_US_Common_AIDs_InformFromFile();		// Load EMV US CommAID
#endif

	return TRUE;
}


/** **********************************************************
*	@brief		App File Version 얻기
*	@retval		없음
************************************************************/
void CDEV_Manager::GetAPPVersion()
{
	CString strProductName, strProductVersion;

	if (m_strAP_Version.IsEmpty())
	{
		// Device 초기화시에 AP Version을 Read하도록 로직 추가 - 1초 소요됨
		CUtil::GetProductAndVersion(_T("041203b5"), strProductName, strProductVersion);

		m_strAP_Version = strProductVersion;
	}
}



/** **********************************************************
*	@brief		종결화
*	@retval		없음
************************************************************/
BOOL CDEV_Manager::Finalize()
{
	return TRUE;
}


/** **********************************************************
*	@brief		통지 윈도우 설정
*	@retval		없음
************************************************************/
void CDEV_Manager::SetNotifyWnd(CWnd* pNotifyWnd)
{
	m_pNotifyWnd = pNotifyWnd;
}


/** **********************************************************
*	@brief		Volume 설정
*	@retval		없음
************************************************************/
void CDEV_Manager::SetVolume(int nLevel)
{
	m_WavePlay.SetVolume(nLevel);

	m_nVolume = nLevel;
}

/** **********************************************************
*	@brief		Volume Up
*	@retval		없음
************************************************************/
void CDEV_Manager::SetVolumeUp()
{

	m_WavePlay.VolumeUp();

	m_nVolume++;
	if (m_nVolume > MAX_VOLUME_LEVEL)
		m_nVolume = MAX_VOLUME_LEVEL;
}

/** **********************************************************
*	@brief		Volume Down
*	@retval		없음
************************************************************/
void CDEV_Manager::SetVolumeDown()
{
	m_WavePlay.VolumeDown();

	m_nVolume--;
	if (m_nVolume < MIN_VOLUME_LEVEL)
		m_nVolume = MIN_VOLUME_LEVEL;
}


/** **********************************************************
*	@brief		현재 설정된 Volume값 조회
*	@retval		현재 설정된 Volume 값
************************************************************/
int CDEV_Manager::GetVolume()
{
	return m_nVolume;
}


/** **********************************************************
*	@brief		Wave File 초기화
*	@retval		없음
************************************************************/
void CDEV_Manager::ResetWaveFile()
{
	//LOG(Info, _T("ResetWaveFile - Start"));
	
	m_colWaveScrData[m_nlanguage].DeleteAll();
}


/** **********************************************************
*	@brief		Play할 wave file 목록을 추가
*	@retval		없음
************************************************************/
void CDEV_Manager::AddWaveFile(CString strFileName)
{
	//LOG(Info, _T("AddWaveFile - Start"));

	CString strWaveData;
	m_colWaveScrData[m_nlanguage].FindDataOfList(CLIENT_SCREEN_ADA_SCR_DISPLAY, strWaveData);

	if(strWaveData.GetLength())
	{
		strWaveData += _T(",");										//구분자 , 추가
		strWaveData = strWaveData + strFileName;					//data 추가
		m_colWaveScrData[m_nlanguage].PutOnTailOfList(CLIENT_SCREEN_ADA_SCR_DISPLAY, strWaveData);
	}
	else
	{
		m_colWaveScrData[m_nlanguage].PutOnTailOfList(CLIENT_SCREEN_ADA_SCR_DISPLAY, strFileName);
	}
}

char	*NumFile[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
					   "10", "11", "12", "13", "14", "15", "16", "17", "18", "19" };
char	*Num10File[] = { "0", "10", "20", "30", "40", "50", "60", "70", "80", "90" };

/** **********************************************************
*	@brief		Play할 숫자 wave file을 목록에 추가
*	@retval		없음
************************************************************/
void CDEV_Manager::AddNumberWaveFiles(int nNumber)
{
	CString strTemp;
	int	quotient = 0;

	if (nNumber >= 100) 
	{
		quotient = nNumber/100;
		strTemp = CString(NumFile[quotient]);
		strTemp += _T(".wav");

		AddWaveFile(strTemp);
		AddWaveFile(HUNDRED_WAVE_FILE);
		nNumber = nNumber % 100;
	}

	if ((0 < nNumber) && (nNumber < 20)) 
	{
		strTemp = CString(NumFile[nNumber]);
		strTemp += _T(".wav");
		AddWaveFile(strTemp);
	}
	else if(20 <= nNumber && nNumber <= 99) 
	{
		int nRemainder = 0;
		quotient = nNumber/10;
		strTemp = CString(Num10File[quotient]);
		strTemp += _T(".wav");
		AddWaveFile(strTemp);

		nRemainder = nNumber % 10;
		if (nRemainder) 
		{
			strTemp = CString(NumFile[nRemainder]);
			strTemp += _T(".wav");
			AddWaveFile(strTemp);
		}
	}
	else if(nNumber == 0 && quotient == 0){
		strTemp = CString(NumFile[nNumber]);
		strTemp += _T(".wav");
		AddWaveFile(strTemp);
	}
}


/** **********************************************************
*	@brief		Play할 금액 wave file을 목록에 추가
*	@retval		없음
************************************************************/
void CDEV_Manager::AddAmountWaveFiles(CString strNumber, BOOL bUseDecimalPoint)
{
	__int64	nDollar = 0;
	int nBillion = 0, nMillion = 0, nThousand = 0, nNumber = 0, nCent = 0;
	CString strBalance, strDollar, strCent;
	char	szDollar[32] = { 0, };

	int nDotIdx = strNumber.Find(_T("."));

	if(nDotIdx != -1)
	{
		strBalance = strNumber;
		strBalance.Remove(',');
		strBalance.Remove('.');

		strDollar = strBalance.Left(strBalance.GetLength()-2);
		strCent = strBalance.Right(2);
	}
	else
	{
		strDollar = strNumber;
	}

	// "-" Balance 처리 추가
	if (strDollar.Left(1) == _T("-"))
	{
		AddWaveFile(MINUS_WAVE_FILE);
		strDollar = strDollar.Mid(1);
	}

	for(int i=0; i<strDollar.GetLength(); i++)
	{
		if (strDollar.GetAt(i) != '0')
		{
			strDollar = strDollar.Mid(i);
			break;
		}
	}

	WideCharToMultiByte(CP_ACP, 0, strDollar, -1, szDollar, strDollar.GetLength(), NULL, NULL);

	nDollar = _atoi64(szDollar);
	nCent = _ttoi(strCent);

	if (nDollar > 9999999999) return; // 10자리 달라까지 지원(BILLION)

	if (nDollar >= 1000000)			// 백만 이상 일경우 -> MILLION, BILLION 사용
	{
		nNumber = (int)(nDollar / 1000000);

		if (nNumber > 999)			// 4자리이면 BILLION 사용
		{
			nBillion = nNumber / 1000;
			AddNumberWaveFiles(nBillion);
			AddWaveFile(BILLION_WAVE_FILE);

			nMillion = nNumber % 1000;
			if (nMillion)
			{
				AddNumberWaveFiles(nMillion);
				AddWaveFile(MILLION_WAVE_FILE);
			}
		}
		else
		{
			AddNumberWaveFiles(nNumber);
			AddWaveFile(MILLION_WAVE_FILE);
		}

		nNumber = (int)(nDollar % 1000000);    // 백만으로 나누었을 때, 나머지
		nThousand = nNumber / 1000;			// 나머지를 1000으로 나누었을 때 몫
		if(nThousand)
		{
			AddNumberWaveFiles(nThousand);
			AddWaveFile(THOUSAND_WAVE_FILE);
		}

		nNumber = nNumber % 1000;       // 나머지를 1000으로 나누었을 때의 나머지	

		if (nNumber >= 0)
		{
			AddNumberWaveFiles(nNumber);
		}
	}
	else if(nDollar >= 1000)	// 백만 미만일경우 -> THOUSAND 사용
	{	
		nThousand = (int)(nDollar / 1000);
		AddNumberWaveFiles(nThousand);
		AddWaveFile(THOUSAND_WAVE_FILE);

		nNumber = (int)(nDollar % 1000);

		if (nNumber >= 0)
		{
			AddNumberWaveFiles(nNumber);
		}
	}
	else if (nDollar >= 0)
	{
		AddNumberWaveFiles((int)nDollar);
		nNumber = (int)nDollar;
	}

	if (nNumber >= 0)
	{
		if (nNumber == 1)
			AddWaveFile(DOLLAR_WAVE_FILE);
		else
			AddWaveFile(DOLLARS_WAVE_FILE);
	}

	// Cent 표시
	if(nCent > 0)
	{
		AddNumberWaveFiles(nCent);

		if(nCent == 1)
			AddWaveFile(CENT_WAVE_FILE);
		else
			AddWaveFile(CENTS_WAVE_FILE);
	}
}


/** **********************************************************
*	@brief		Wave File Play
*	@retval		없음
************************************************************/
void CDEV_Manager::PlayWaveFile(BOOL bWaitUntilEnd)
{
	LOG(Info, _T("PlayWaveFile - Start"));

	if (m_bPlay == TRUE)
	{
		m_bPlay = FALSE;
		m_eDoneToPlay.Wait(INFINITE);
	}

	m_strCurrPlayList.Empty();
	m_colWaveScrData[m_nlanguage].FindDataOfList(CLIENT_SCREEN_ADA_SCR_DISPLAY, m_strCurrPlayList);

	if(m_strCurrPlayList.GetLength() == 0)
		return;

	m_bPlay = TRUE;
	m_eDoneToPlay.Reset();
	m_eStartToPlay.Set();

	if (bWaitUntilEnd == TRUE)
		m_eDoneToPlay.Wait(INFINITE);
}


/** **********************************************************
*	@brief		Wave File Play
*	@retval		없음
************************************************************/
void CDEV_Manager::StopPlayWaveFile()
{
	LOG(Info, _T("StopPlayWaveFile - Start"));

	if (m_bPlay == TRUE)
	{
		m_bPlay = FALSE;

		if (m_eDoneToPlay.Wait(INFINITE) != WAIT_OBJECT_0)
			return;
	}
}


/** **********************************************************
*	@brief		Wave File Play
*	@retval		없음
************************************************************/
BOOL CDEV_Manager::IsPlayingWaveFile()
{
//	if (m_eDoneToPlay.Wait(0) == WAIT_OBJECT_0)
//		return FALSE;

	if (m_bPlay == FALSE)
		return FALSE;

	return TRUE;
}


unsigned CDEV_Manager::ThreadHandlerProc(void)
{
	int				i;
	int				nCountOfPlayList;
	CString			strPlayFile;
	CStringArray	strPlayListArr;

	m_bExit = FALSE;

	while(m_bExit == FALSE)
	{
		if (m_eStartToPlay.Wait(100) == WAIT_OBJECT_0)
		{
			strPlayListArr.RemoveAll();
			CUtil::ParsingStringToStringArray(m_strCurrPlayList, _T(","), strPlayListArr);

			nCountOfPlayList = strPlayListArr.GetUpperBound();

			for (i = 0; i <= nCountOfPlayList && m_bPlay == TRUE; i++)
			{
				strPlayFile.Format(_T("%s%s%s"), CUtil::GetAppPath(), WAVE_FILE_PATH, strPlayListArr[i]);

				if (m_WavePlay.Play(strPlayFile) == TRUE)
				{
					while (m_bPlay == TRUE)
					{
						if (m_DEV_DIO.m_bAudioJackInserted == FALSE)
						{
							LOG(Info, _T("Audio Jack is removed"));
							break;
						}

						if (m_WavePlay.IsPlaying(50) == FALSE)
							break;
					}
				}

				if (m_DEV_DIO.m_bAudioJackInserted == FALSE)
					break;
			}

			m_WavePlay.Stop();

			m_eDoneToPlay.Set();
			m_bPlay = FALSE;
		}
	}

	return 0;
}


/** **********************************************************
*	@brief		System Reboot
*	@retval		없음
************************************************************/
void CDEV_Manager::System_Reboot(BOOL bForced)
{
	LOG(Info, _T("Start - System Reboot"));

	// Device에 문제가 발생하여 강제 Reboot시에는 Device Close 미수행하고 즉시 Hardware Reboot 수행
	if (bForced == FALSE)
	{
		// System Reboot전에 LED Close
		CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_ClosePort();
		CDEV_Manager::GetInstance()->m_DEV_DIO.Close();
		CDEV_Manager::GetInstance()->m_DEV_CDR.Close();
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_Close();

		if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsOpened() == TRUE)
			CDEV_Manager::GetInstance()->m_DEV_CAMERA.Close();

		CUtil::Sleep_Wait(2000);
	}
	else
	{
		LOG(Info, _T("Skip - Device close"));
	}

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

	// 20 sec dekay parameter
	/*
	buff[0]=0x1;
	buff[1]=0x2;
	buff[2]=0x1;
	buff[3]=0x5;
	buff[4]=0x14;
	buff[5]=0;
	buff[6]=0x1d;
	*/

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
#ifdef _WIN32_WCE
	KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL);
#else
	// Win32 simulator: never reboot the developer's PC - just log the request.
	LOG(Info, _T("[Simulator] Software reboot requested (ignored on Win32)"));
#endif

	CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_5MIN);
}


/** **********************************************************
*	@brief		Save Text Value
*	@retval		없음
************************************************************/
BOOL CDEV_Manager::SaveEJL_SetTextValue(DWORD dwJnl_Code, CString strTextValue, BOOL bUseLocalTime, CString strStackTime)
{
	FLSEJCOMMINFO	CommInfo;
	EJTEXTINFO		TextInfo;
	BOOL			bResult = FALSE;

	SYSTEMTIME	st;
	GetLocalTime(&st);

	memset(&CommInfo, 0, sizeof(CommInfo));
	memset(&TextInfo, 0, sizeof(TextInfo));

	if (bUseLocalTime == TRUE)
	{
		sprintf(CommInfo.log_date, "%04d/%02d/%02d", st.wYear, st.wMonth, st.wDay);
		sprintf(CommInfo.log_time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
		sprintf(CommInfo.log_srch_date, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	}
	else
	{
		// 상위국에서 내려준 Time을 사용하도록 함.

		// 자리수가 안맞을 경우 Journal 저장되지 않도록 처리
		if (strStackTime.GetLength() != 14)
			return FALSE;

		sprintf(CommInfo.log_date, "%S/%S/%S", strStackTime.Left(4), strStackTime.Mid(4,2), strStackTime.Mid(6,2));
		sprintf(CommInfo.log_time, "%S:%S:%S", strStackTime.Mid(8,2), strStackTime.Mid(10,2), strStackTime.Right(2));
		sprintf(CommInfo.log_srch_date, "%S", strStackTime);
	}

	CommInfo.jnl_cd = dwJnl_Code;

	sprintf(TextInfo.term_id, "%S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
	sprintf(TextInfo.text_value, "%S", strTextValue);

	//LOG(Info, _T("Set Text Value start in EJL"));

	bResult = CDEV_Manager::GetInstance()->m_DEV_EJL.InsertJnlData(&CommInfo, &TextInfo);

	LOG(Info, _T("Set Text Value in EJL - (Value : %s) End (Result : %d)"), strTextValue, bResult);

	// Status Journal 저장시에는 Power Off File 제거
	if (dwJnl_Code == power_on || dwJnl_Code == in_service || dwJnl_Code == out_of_service || dwJnl_Code == in_supervisor || dwJnl_Code == reboot)
		::DeleteFile(ATM_STATUS_MONITORING_FILE);

	return bResult;	
}


/** **********************************************************
*	@brief		SaveEJL_SetChangeValue  : 변경된 설정 값을 Journal에 저장하는 함수
*	@param		CString strOld : 이전 Data
*	@param		CString strNew : 새로운 Data
*	@retval		성공시 : TRUE, 실패시 FALSE
************************************************************/
BOOL CDEV_Manager::SaveEJL_SetChangeValue(CString strOld, CString strNew)
{
	//LOG(Info, _T("Set ChangeValue INFO in EJL - start"));

	BOOL bResult = FALSE;
	CStringA szTemp;

	szTemp.Empty();

	// 현재 시간 취득
	SYSTEMTIME st;
	GetLocalTime(&st);

	// Common Info
	FLSEJCOMMINFO CommInfo;
	memset(&CommInfo, 0, sizeof(CommInfo));

	sprintf(CommInfo.log_date, "%04d/%02d/%02d", st.wYear, st.wMonth, st.wDay);
	sprintf(CommInfo.log_time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
	CommInfo.jnl_cd = value_change;
	sprintf(CommInfo.log_srch_date, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	EJCHANGEINFO ChangeInfo;
	memset(&ChangeInfo, 0, sizeof(ChangeInfo));

	szTemp.Format("%S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
	sprintf(ChangeInfo.term_id, "%s", szTemp);
	sprintf(ChangeInfo.old_value, "%S", strOld);
	sprintf(ChangeInfo.new_value, "%S", strNew);

	bResult = CDEV_Manager::GetInstance()->m_DEV_EJL.InsertJnlData(&CommInfo, &ChangeInfo);

	LOG(Info, _T("Set ChangeValue INFO in EJL - End (Result : %d"), bResult);

	return bResult;
}


/** *********************************************************
*	@brief		Set_Network_Config : Network config 적용
*	@retval		없음
************************************************************/
BOOL CDEV_Manager::Set_Network_Config(BOOL bReloadLanCard)
{
	DWORD	max_timeout = GetTickCount() + (10 * 1000);	// 10초 Timeout

	if (m_bExecNetSetting == TRUE)
	{
		LOG(Info, _T("Set_Network_Config - Already running - return"));
		return FALSE;
	}

	m_bForceReloadLan = bReloadLanCard;
	m_bExecNetSetting = TRUE;

	// 10초 동안 완료 여부를 Check
	while(GetTickCount() < max_timeout)
	{
		if (m_bExecNetSetting == FALSE)
		{
			// 정상 수행시에는 TRUE return 처리
			// 여기서도 Flag 초기화 처리
			m_bForceReloadLan = FALSE;
			m_bExecNetSetting = FALSE;
			return TRUE;
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_500MS);
	}

	// Timeout인 경우에는 FALSE로 Return 처리
	m_bForceReloadLan = FALSE;
	m_bExecNetSetting = FALSE;

	LOG(Error, _T("Set_Network_Config - Setting Error - need to recovery"));

	// Recovery를 위해 System Reboot 수행
	CDEV_Manager::GetInstance()->System_Reboot(TRUE);	// 비정상 상황이므로 Device Port Close없이 재부팅하도록 함

	return FALSE;

	// Network 확인
	//int nProtocolType = 0;
	//int nNetworkType = 0;
	//BOOL bClearIPInfo = FALSE;

	//if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
	//{
	//	if (CUtil::Check_IP_Information() == FALSE)	// TCP/IP일 경우에만 Check하도록 수정
	//	{
	//		LOG(Info, _T("Cleared IP Information Registry due to OS update"));
	//		bClearIPInfo = TRUE;	// OS 설치에 따른 IP 정보 삭제됨
	//	}

	//	nNetworkType = 0;

	//	// Lan Card 초기화 처리

	//	if ((bReloadLanCard == TRUE) || (bClearIPInfo == TRUE))
	//	{
	//		// DHCP 설정
	//		if (CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP == S_ENABLE)
	//		{
	//			CUtil::SetDHCP(1);
	//		}
	//		else
	//		{
	//			CUtil::SetDHCP(0);

	//			// STATIC IP 설정
	//			CUtil::SetIPAddress(CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip);
	//			CUtil::SetSubnetMask(CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet);
	//			CUtil::SetGateway(CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway);
	//			CUtil::SetDNS(CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS);
	//		}

	//		RegFlushKey(HKEY_LOCAL_MACHINE);
	//		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);	// 1초 Delay 추가

	//		LOG(Info, _T("Reload Ethernet Device - Start"));
	//		CUtil::ResetEthernetDevice();	// reload Ethernet device
	//		LOG(Info, _T("Reload Ethernet Device - End"));
	//		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_5000MS);	// Reload가 완료된 후에는 IP 주소 등을 할당 받아야 하므로 5초 Delay
	//	}
	//}
	//else
	//{
	//	nNetworkType = 1;
	//}


	//if (CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType == _T("VISA"))
	//	nProtocolType = 0;
	//else
	//	nProtocolType = 1;


	//CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Set_Flow_LineType(nProtocolType, nNetworkType);

	//LOG(Info, _T("Set Network Config - Network Type (%s), DHCP : (%s)"), CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType, CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP);
}


/** *********************************************************
*	@brief		CheckMemoryStatus - Memory Check 함수
*	@retval		없음
************************************************************/
void CDEV_Manager::CheckMemoryStatus()
{
	memStatus.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&memStatus);

	// Storage Memory Check
#ifdef _WIN32_WCE
	GetStoreInformation(&si);
#endif

	UsingVirtualMemory = memStatus.dwTotalVirtual - memStatus.dwAvailVirtual;
	UsingPhysMemory = memStatus.dwTotalPhys - memStatus.dwAvailPhys;

#ifdef _WIN32_WCE
	LOG(Info, _T("[MEMORY CHECK] Using Virtual : (%dK), Using Physical : (%dK), Free Store Size (%dK) "), (UsingVirtualMemory / 1024), (UsingPhysMemory / 1024), (si.dwFreeSize / 1024));
#else
	LOG(Info, _T("[MEMORY CHECK] Using Virtual : (%dK), Using Physical : (%dK)"), (UsingVirtualMemory / 1024), (UsingPhysMemory / 1024));
#endif

}


/** *********************************************************
*	@brief		Save_PreviousTerminalInfo - Supervisor 진입 전 Terminal 정보 Save
*	@retval		없음
************************************************************/
void CDEV_Manager::Save_PreviousTerminalInfo()
{
	// 현재 정보를 임시 저장함

	// 1. HOST Information
	g_TempHost.strHostip = CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip;
	g_TempHost.strBackup_Hostip = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_Hostip;
	g_TempHost.strPort = CEagleDataManager::GetInstance()->m_Config.m_Host.strPort;
	g_TempHost.strBackupPort = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackupPort;

	g_TempHost.strNetworkType = CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType;
	g_TempHost.strProtocolType = CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType;

	g_TempHost.strUseENQ = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseENQ;
	g_TempHost.strUseEOT = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseEOT;
	g_TempHost.strUseTLS = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS;

	g_TempHost.strCommunication_header = CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunication_header;
	g_TempHost.strCommunicationID = CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunicationID;
	g_TempHost.strPrimary_phone_number = CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number;
	g_TempHost.strBackup_phone_number = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_phone_number;
	g_TempHost.strPre_dial = CEagleDataManager::GetInstance()->m_Config.m_Host.strPre_dial;
	g_TempHost.strSetup_String = CEagleDataManager::GetInstance()->m_Config.m_Host.strSetup_String;

	g_TempHost.strCrc_flg = CEagleDataManager::GetInstance()->m_Config.m_Host.strCrc_flg;

	g_TempHost.strAtm_status_monitoring = CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring;
	g_TempHost.strHeartbeat_message = CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_message;
	g_TempHost.strHeartbeat_frequency = CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_frequency;

	g_TempHost.strRms_en_flg = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg;
	g_TempHost.strRms_Status_en_flg = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg;
	g_TempHost.strRms_primary_num = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_primary_num;
	g_TempHost.strRms_IP_Address = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_IP_Address;
	g_TempHost.strRms_Port = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Port;

	g_TempHost.strSchedule_journal_flg = CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_flg;
	g_TempHost.strSchedule_journal_count = CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_count;

	// Terminal IP 정보 추가
	g_TempHost.strUseDHCP = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP;
	g_TempHost.strTerminalip = CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip;

	// 2. Option Information
	g_TempOption.strSaving_account = CEagleDataManager::GetInstance()->m_Config.m_Option.strSaving_account;
	g_TempOption.strCredit_account = CEagleDataManager::GetInstance()->m_Config.m_Option.strCredit_account;

	g_TempOption.strtransfer_Enable = CEagleDataManager::GetInstance()->m_Config.m_Option.strtransfer_Enable;
	g_TempOption.strbalance_Enable = CEagleDataManager::GetInstance()->m_Config.m_Option.strbalance_Enable;

	for(int i=0; i<6; i++)
	{
		g_TempOption.strFast_amount[i] = CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[i];
	}

	g_TempOption.strMax_withdrawal_amount = CEagleDataManager::GetInstance()->m_Config.m_Option.strMax_withdrawal_amount;


	g_TempOption.strSurcharge_type = CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_type;
	g_TempOption.strWithdrawalSurcharge_amount = CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount;

	g_TempOption.strWithdrawal_PercentSurcharge_Option = CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawal_PercentSurcharge_Option;
	g_TempOption.strSurcharge_percentage = CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_percentage;

	g_TempOption.strSurcharge_owner = CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_owner;

	g_TempOption.strTerminal_ID = CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID;
	g_TempOption.strMachine_Serial_Number = CEagleDataManager::GetInstance()->m_Config.m_Option.strMachine_Serial_Number;

	g_TempOption.strLansupport[1] = CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[1];	// Spanish
	g_TempOption.strLansupport[2] = CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[2];	// French


	g_TempOption.strScheduleDayClose = CEagleDataManager::GetInstance()->m_Config.m_Option.strScheduleDayClose;
	g_TempOption.strDayCloseTime = CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime;

	g_TempOption.strSound_Volume = CEagleDataManager::GetInstance()->m_Config.m_Option.strSound_Volume;

	g_TempOption.strRebootTime = CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime;

	// SYSTEM Password
	g_TempSystem.strMaster_password = CEagleDataManager::GetInstance()->m_Config.m_System.strMaster_password;
	g_TempSystem.strAdministrator_password = CEagleDataManager::GetInstance()->m_Config.m_System.strAdministrator_password;
	g_TempSystem.strReplenishment_password = CEagleDataManager::GetInstance()->m_Config.m_System.strReplenishment_password;
	g_TempSystem.strRms_Password = CEagleDataManager::GetInstance()->m_Config.m_System.strRms_Password;

}


/** *********************************************************
*	@brief		SaveEJL_SetChangedTerminalInfo - Supervisor Exit시 Terminal 정보 변경시 Save
*	@retval		없음
************************************************************/
void CDEV_Manager::SaveEJL_SetChangedTerminalInfo(int nDest)
{
	CString strTemp, strDest_Info, strTitle, strOldEJLData, strNewEJLData;

	// 변경된 값 확인
	if (nDest == 1)
		strDest_Info = _T("[AMS]");
	else if (nDest == 2)
		strDest_Info = _T("[HOST]");

	// 1. Host Information
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType;
	if (g_TempHost.strNetworkType.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("NETWORK TYPE");
		strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strNetworkType);
		strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	// 현재 Network Type정보를 바탕으로 변경사항 Check
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip;
		if (g_TempHost.strHostip.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("PRIMARY HOST ADDRESS");
			strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strHostip);
			strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}

		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strPort;
		if (g_TempHost.strPort.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("PRIMARY HOST PORT");
			strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strPort);
			strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}

		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_Hostip;
		if (g_TempHost.strBackup_Hostip.CompareNoCase(strTemp) != 0)
		{
			strTitle = _T("BACKUP HOST ADDRESS");
			strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strBackup_Hostip);
			strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}

		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackupPort;
		if (g_TempHost.strBackupPort.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("BACKUP HOST PORT");
			strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strBackupPort);
			strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}

		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType;
		if (g_TempHost.strProtocolType.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("PROTOCOL TYPE");
			strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strProtocolType);
			strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}

		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS;
		if (g_TempHost.strUseTLS.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("TLS");
			strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempHost.strUseTLS));
			strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}

		// Terminal IP 정보 추가
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP;

		if (g_TempHost.strUseDHCP.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("DHCP");
			strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempHost.strUseDHCP));
			strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}

		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip;

		if (g_TempHost.strTerminalip.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("TERMINAL IP");
			strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strTerminalip);
			strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}
	}
	else
	{
		// MODEM
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number;
		if (g_TempHost.strPrimary_phone_number.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("PRIMARY HOST PHONE NUMBER");
			strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strPrimary_phone_number);
			strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}

		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_phone_number;
		if (g_TempHost.strBackup_phone_number.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("BACKUP HOST PHONE NUMBER");
			strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strBackup_phone_number);
			strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}

		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strPre_dial;
		if (g_TempHost.strPre_dial.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("PRE-DIAL");
			strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempHost.strPre_dial));
			strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}

		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strSetup_String;
		if (g_TempHost.strSetup_String.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("SETUP STRING");
			strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strSetup_String);
			strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}


	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseENQ;
	if (g_TempHost.strUseENQ.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("ENQ");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_REQUIREDORNOT_STRING(g_TempHost.strUseENQ));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_REQUIREDORNOT_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseEOT;
	if (g_TempHost.strUseEOT.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("EOT");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_OPTION_STRING(g_TempHost.strUseEOT));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_OPTION_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunication_header;
	if (g_TempHost.strCommunication_header.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("COMMUNICATION HEADER");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempHost.strCommunication_header));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunicationID;
	if (g_TempHost.strCommunicationID.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("COMMUNICATION ID");
		strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strCommunicationID);
		strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strCrc_flg;
	if (g_TempHost.strCrc_flg.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("CRC");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempHost.strCrc_flg));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring;
	if (g_TempHost.strAtm_status_monitoring.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("STATUS MONITORING");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempHost.strAtm_status_monitoring));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_message;
	if (g_TempHost.strHeartbeat_message.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("HEARTBEAT");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempHost.strHeartbeat_message));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_frequency;
	if (g_TempHost.strHeartbeat_frequency.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("HEARTBEAT FREQUENCY");
		strOldEJLData.Format(_T("%s = %s MINUTES"), strTitle, g_TempHost.strHeartbeat_frequency);
		strNewEJLData.Format(_T("%s = %s MINUTES"), strTitle, strTemp);

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg;
	if (g_TempHost.strRms_en_flg.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("AMS");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempHost.strRms_en_flg));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg;
	if (g_TempHost.strRms_Status_en_flg.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("AMS STATUS");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempHost.strRms_Status_en_flg));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_IP_Address;
		if (g_TempHost.strRms_IP_Address.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("AMS ADDRESS");
			strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strRms_IP_Address);
			strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}

		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Port;
		if (g_TempHost.strRms_Port.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("AMS PORT");
			strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strRms_Port);
			strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}
	}
	else
	{
		// Modem
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_primary_num;
		if (g_TempHost.strRms_primary_num.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("AMS PHONE NUMBER");
			strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strRms_primary_num);
			strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}

	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_flg;
	if (g_TempHost.strSchedule_journal_flg.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("UPLOAD JOURNAL");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempHost.strSchedule_journal_flg));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_count;
	if (g_TempHost.strSchedule_journal_count.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("UPLOAD JOURNAL COUNT");
		strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempHost.strSchedule_journal_count);
		strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	// 2. Option
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strSaving_account;
	if (g_TempOption.strSaving_account.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("SAVINGS ACCOUNT");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempOption.strSaving_account));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strCredit_account;
	if (g_TempOption.strCredit_account.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("CREDIT ACCOUNT");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempOption.strCredit_account));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strbalance_Enable;
	if (g_TempOption.strbalance_Enable.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("INQUIRY TRANSACTION");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempOption.strbalance_Enable));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strtransfer_Enable;
	if (g_TempOption.strtransfer_Enable.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("TRANSFER TRANSACTION");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempOption.strtransfer_Enable));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	BOOL bChangedFastCash = FALSE;

	for(int i=0; i<6; i++)
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[i];
		if (g_TempOption.strFast_amount[i].CompareNoCase(strTemp) != 0)
		{
			//if (i < 3)
			//{
			//	strTitle.Format(_T("%sFAST AMOUNT FDK_L%d"), strDest_Info, i+1);
			//}
			//else
			//{
			//	strTitle.Format(_T("%sFAST AMOUNT FDK_R%d"), strDest_Info, i-2);
			//}

			//strOldEJLData.Format(_T("%s = %s%s"), strTitle, CURRENCY_SYMBOL, g_TempOption.strFast_amount[i]);
			//strNewEJLData.Format(_T("%s = %s%s"), strTitle, CURRENCY_SYMBOL, strTemp);

			//SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
			bChangedFastCash = TRUE;
			break;
		}
	}

	if (bChangedFastCash == TRUE)
	{
		strOldEJLData = strDest_Info;
		strOldEJLData += _T("FASH AMOUNT = ");

		strNewEJLData = strOldEJLData;

		for(int i=0; i<6; i++)
		{
			strOldEJLData += CURRENCY_SYMBOL;
			strOldEJLData += g_TempOption.strFast_amount[i];

			if (i != 5)
				strOldEJLData += _T(", ");

			strNewEJLData += CURRENCY_SYMBOL;
			strNewEJLData += CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[i];

			if (i != 5)
				strNewEJLData += _T(", ");
		}

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strMax_withdrawal_amount;
	if (g_TempOption.strMax_withdrawal_amount.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("MAX WITHDRAWAL AMOUNT");
		strOldEJLData.Format(_T("%s = %s%s"), strTitle, CURRENCY_SYMBOL, g_TempOption.strMax_withdrawal_amount);
		strNewEJLData.Format(_T("%s = %s%s"), strTitle, CURRENCY_SYMBOL, strTemp);

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_type;
	if (g_TempOption.strSurcharge_type.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("SURCHARGE TYPE");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_SURCHARGETYPE_STRING(g_TempOption.strSurcharge_type));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_SURCHARGETYPE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	if (CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_type == _T("0"))
	{
		// Fixed Amount Option
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount;
		if (g_TempOption.strWithdrawalSurcharge_amount.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("SURCHARGE AMOUNT");
			strOldEJLData.Format(_T("%s = %s%s"), strTitle, CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(g_TempOption.strWithdrawalSurcharge_amount));
			strNewEJLData.Format(_T("%s = %s%s"), strTitle, CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strTemp));

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}
	}
	else
	{
		// Percentage Option
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawal_PercentSurcharge_Option;
		if (g_TempOption.strWithdrawal_PercentSurcharge_Option.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("SURCHARGE METHOD");
			strOldEJLData.Format(_T("%s = %s"), strTitle, GET_PERCENTAGEMETHOD_STRING(g_TempOption.strWithdrawal_PercentSurcharge_Option));
			strNewEJLData.Format(_T("%s = %s"), strTitle, GET_PERCENTAGEMETHOD_STRING(strTemp));

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}

		// Percentage
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_percentage;
		if (g_TempOption.strSurcharge_percentage.CompareNoCase(strTemp) != 0)
		{
			strTitle = strDest_Info;
			strTitle += _T("SURCHARGE PERCENTAGE");
			strOldEJLData.Format(_T("%s = %s"), strTitle, CUtil::ConvertFromValueToAmountwithCent(g_TempOption.strSurcharge_percentage));
			strNewEJLData.Format(_T("%s = %s"), strTitle, CUtil::ConvertFromValueToAmountwithCent(strTemp));

			SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
		}
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_owner;
	if (g_TempOption.strSurcharge_owner.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("TERMINAL OWNER");
		strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempOption.strSurcharge_owner);
		strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID;
	if (g_TempOption.strTerminal_ID.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("TERMINAL ID");
		strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempOption.strTerminal_ID);
		strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strMachine_Serial_Number;
	if (g_TempOption.strMachine_Serial_Number.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("SERIAL NUMBER");
		strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempOption.strMachine_Serial_Number);
		strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[1];
	if (g_TempOption.strLansupport[1].CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("SPANISH");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempOption.strLansupport[1]));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[2];
	if (g_TempOption.strLansupport[2].CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("FRENCH");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempOption.strLansupport[2]));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strScheduleDayClose;
	if (g_TempOption.strScheduleDayClose.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("SCHEDULE DAY CLOSE");
		strOldEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(g_TempOption.strScheduleDayClose));
		strNewEJLData.Format(_T("%s = %s"), strTitle, GET_ENDISABLE_STRING(strTemp));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime;
	if (g_TempOption.strDayCloseTime.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("SCHEDULE DAY CLOSE TIME");
		strOldEJLData.Format(_T("%s = %s:%s"), strTitle, g_TempOption.strDayCloseTime.Left(2), g_TempOption.strDayCloseTime.Right(2));
		strNewEJLData.Format(_T("%s = %s:%s"), strTitle, strTemp.Left(2), strTemp.Right(2));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strSound_Volume;
	if (g_TempOption.strSound_Volume.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("SOUND VOLUME");
		strOldEJLData.Format(_T("%s = %s"), strTitle, g_TempOption.strSound_Volume);
		strNewEJLData.Format(_T("%s = %s"), strTitle, strTemp);

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime;
	if (g_TempOption.strRebootTime.CompareNoCase(strTemp) != 0)
	{
		strTitle = strDest_Info;
		strTitle += _T("REBOOT TIME");
		strOldEJLData.Format(_T("%s = %s:%s"), strTitle, g_TempOption.strRebootTime.Left(2), g_TempOption.strRebootTime.Right(2));
		strNewEJLData.Format(_T("%s = %s:%s"), strTitle, strTemp.Left(2), strTemp.Right(2));

		SaveEJL_SetChangeValue(strOldEJLData, strNewEJLData);
	}

	// SYSTEM Password - Password는 누출되면 안되므로 Information으로 저장하도록 함
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_System.strMaster_password;
	if (g_TempSystem.strMaster_password.CompareNoCase(strTemp) != 0)
	{
		SaveEJL_SetTextValue(text_field, _T("MASTER PASSWORD IS CHANGED"));
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_System.strAdministrator_password;
	if (g_TempSystem.strAdministrator_password.CompareNoCase(strTemp) != 0)
	{
		SaveEJL_SetTextValue(text_field, _T("ADMIN. PASSWORD IS CHANGED"));
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_System.strReplenishment_password;
	if (g_TempSystem.strReplenishment_password.CompareNoCase(strTemp) != 0)
	{
		SaveEJL_SetTextValue(text_field, _T("REPLENISHMENT PASSWORD IS CHANGED"));
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_System.strRms_Password;
	if (g_TempSystem.strRms_Password.CompareNoCase(strTemp) != 0)
	{
		SaveEJL_SetTextValue(text_field, _T("AMS PASSWORD IS CHANGED"));
	}
}


UINT CDEV_Manager::Thread_SetNetwork( LPVOID pParam )
{
	CDEV_Manager* pDev_Manager = (CDEV_Manager*)pParam;

	pDev_Manager->Thread_SetNetwork();

	return 0;
}


void CDEV_Manager::Thread_SetNetwork( void )
{
	LOG(Info, _T("[CDEV_Manager] Network Setting Thread Start"));

	// Network 확인
	int nProtocolType = 0;
	int nNetworkType = 0;
	BOOL bClearIPInfo = FALSE;

	while (TRUE)
	{
		// Close시 Thread 종료
		if(m_bNetSettingThreadExit)
		{
			break;
		}

		if (m_bExecNetSetting == TRUE)
		{
			// Network 확인
			nProtocolType = 0;
			nNetworkType = 0;
			bClearIPInfo = FALSE;

			LOG(Info, _T("Set Network Config Thread - Setting Start"));

			if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
			{
#ifdef _WIN32_WCE
				if (CUtil::Check_IP_Information() == FALSE)	// TCP/IP일 경우에만 Check하도록 수정
				{
					LOG(Info, _T("Cleared IP Information Registry due to OS update"));
					bClearIPInfo = TRUE;	// OS 설치에 따른 IP 정보 삭제됨
				}
#endif	// _WIN32_WCE

				// Values match TypeOfLine in EagleCE_Host/LineCtrl.h, which this
				// project does not include: 0 = LINE_TCP (TLS driven through
				// OpenSSL directly), 2 = LINE_CURL (same wire protocol over
				// libcurl, CURLOPT_CONNECT_ONLY). Opt in through HOST.CFG
				// use_curl - default 0 keeps the OpenSSL line.
				if (CEagleDataManager::GetInstance()->m_Config.m_Host.strUseCurl == S_ENABLE)
					nNetworkType = 2;
				else
					nNetworkType = 0;

				// Lan Card 초기화 처리
				//
				// Win32 desktop: skipped on purpose. Windows owns the adapter,
				// CUtil's setters write the WinCE "Comm\...\Parms\TcpIp" registry
				// layout that does not exist here, and ResetEthernetDevice()
				// drives an NDIS handle no desktop has. Re-running it would put
				// junk in HKLM without changing a single real address, so the
				// simulator reads the live settings instead - see
				// CSVC_Manager::Initialize().
#ifdef _WIN32_WCE
				if ((m_bForceReloadLan == TRUE) || (bClearIPInfo == TRUE))
				{
					// DHCP 설정
					if (CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP == S_ENABLE)
					{
						CUtil::SetDHCP(1);
					}
					else
					{
						CUtil::SetDHCP(0);

						// STATIC IP 설정
						CUtil::SetIPAddress(CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip);
						CUtil::SetSubnetMask(CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet);
						CUtil::SetGateway(CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway);
						CUtil::SetDNS(CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS);
					}

					RegFlushKey(HKEY_LOCAL_MACHINE);
					CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);	// 1초 Delay 추가

					LOG(Info, _T("Reload Ethernet Device - Start"));

					CUtil::ResetEthernetDevice();					// Reload Ethernet device

					LOG(Info, _T("Reload Ethernet Device - End"));

					CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);	// Reload가 완료된 후에는 IP 주소 등을 할당 받아야 하므로 1초 Delay
				}
#endif	// _WIN32_WCE
			}
			else
			{
				nNetworkType = 1;
			}


			if (CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType == _T("VISA"))
				nProtocolType = 0;
			else
				nProtocolType = 1;


			CDEV_Manager::GetInstance()->m_DEV_HOST.HOST_Set_Flow_LineType(nProtocolType, nNetworkType);

			LOG(Info, _T("Set Network Config Thread - Setting End - Network Type (%s), DHCP : (%s)"), CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType, CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP);

			m_bExecNetSetting = FALSE;
			m_bForceReloadLan = FALSE;
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);
	}

	LOG(Info, _T("[CDEV_Manager] Network Setting Thread Exit"));
}
