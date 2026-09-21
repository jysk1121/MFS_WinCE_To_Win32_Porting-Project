/////////////////////////////////////////////////////////////////////////////
// AtmMainLib.cpp : Implementation of the CWinAtmCtrl ActiveX Control class.
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <winioctl.h>
#include <Tlhelp32.h>
#include <Afxdisp.h>

#include "MainFrm.h"
#include "NvramBackupDefine.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

#define IOCTL_HAL_REBOOT CTL_CODE(FILE_DEVICE_HAL, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)
extern "C" __declspec(dllimport) BOOL KernelIoControl(
    DWORD dwIoControlCode, 
    LPVOID lpInBuf, 
    DWORD nInBufSize, 
    LPVOID lpOutBuf, 
    DWORD nOutBufSize, 
    LPDWORD lpBytesReturned
);

//////////////////////////////////////////////////////////////////////////
// Execute SWUpdate
//////////////////////////////////////////////////////////////////////////
int CMainFrame::ExecuteSWUpdate()
{
	NHDEBUG(1, (_T("ExecuteSWUpdate()\n")));

	// 2008-01-15 V01.02.25 SRC-10
	// SWUpdate.exe File을 자기자신도 update가 가능하도록 update.exe로 변경함
	CString	strCmd = _T("");
	CString strFileName = L"\\ATM\\UpdateLauncher.exe";
	HANDLE hFile = CreateFile((LPCTSTR)strFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);

		strCmd = L"\\ATM\\UpdateLauncher.exe";

		STARTUPINFO si;
		ZeroMemory (&si, sizeof(si));
		si.cb = sizeof (si);
		si.wShowWindow = SW_HIDE;
		PROCESS_INFORMATION pi;
		
		CreateProcess(strCmd,NULL,NULL,NULL,NULL,FALSE,0,NULL,&si, &pi);

		// [#539] NH KSK 2009.07.08
		// 강제 Kill을 할 경우 죽지 않는 현상이 발생하므로 CloseSession 추가
		// m_pDevCmn might be null at this point
		if (m_pDevCmn != NULL)
		{
			m_pDevCmn->AtmDoing = FALSE;	// Atm Stop
		}

		// Kill SP Processes
		KillProcByGoma(L"NHCDM.EXE NHDIP4WCE.EXE NHIDC4MTKWCE.EXE NHDIP4SANDIPCE.EXE NHIDC4RFCE.EXE NW_KSPR.EXE MyPinPad.exe NH_SIU.exe");

		NHDEBUG(1, (_T("fnAPL_UnLoadDevice()\n")));
		if (m_pDevCmn != NULL)
		{
			m_pDevCmn->fnAPL_UnLoadDevice();
		}
		// end of [#539]

		//1. 타 프로세스 kill
		NHWARN((L"Killing DevIDC30.exe\r\n"));
		KillProcByName("DevIDC30.exe");		// [#11] NH KSK 2010.09.29 Process가 NHDIP4WCE가 아닌지 확인 필요
		NHWARN((L"Killing MyPinPad.exe\r\n"));
		KillProcByName("MyPinPad.exe");
		NHWARN((L"Killing NH_SIU.exe\r\n"));
		KillProcByName("NH_SIU.exe");
		NHWARN((L"Killing NHCDM.exe\r\n"));
		KillProcByName("NHCDM.exe");
		NHWARN((L"Killing NW_KSPR.exe\r\n"));
		KillProcByName("NW_KSPR.exe");

		// [#11] NH KSK 2010.09.29
		NHWARN((L"Killing NHDIP4WCE.exe\r\n"));
		KillProcByName("NHDIP4WCE.exe");
		KillProcByName("NHIDC4MTKWCE.exe");
		NHWARN((L"Killing NHDIP4SANDIPCE.exe\r\n"));
		KillProcByName("NHDIP4SANDIPCE.exe");	// [#2074] NH KSK 2011.07.12
		NHWARN((L"Killing NHIDC4RFCE.exe\r\n"));
		KillProcByName("NHIDC4RFCE.exe");		// [#2325] NH KSK 2015.01.25
		// end of [#11]

		// [#GLDV-3005] US Kook 2022.01.20 Side Car
		NHWARN((L"Killing NW_BCR_CE.exe\r\n"));
		KillProcByName("NW_BCR_CE.exe");
		NHWARN((L"Killing NW_CIM_CE.exe\r\n"));
		KillProcByName("NW_CIM_CE.exe");
		NHWARN((L"Killing NW_CIM_CE_JCM.exe\r\n"));
		KillProcByName("NW_CIM_CE_JCM.exe");
		// end of [#GLDV-3005]

		// [#2050] NH KSK 2011.04.18 WatchDog 프로세스 kill
		NHWARN((L"Killing WatchDog.exe\r\n"));
		KillProcByName("WatchDog.exe");
		
		//WinATM Close한다.
		NHWARN((L"Killing WinATM.exe\r\n"));
		KillProcByName("WinATM.exe");
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// Terminate ATM
//////////////////////////////////////////////////////////////////////////
int	CMainFrame::TerminateATM(CString strGuideText, BOOL bMsgDisplay, BOOL bShutdown)
{
	// [#2536] NH Kook 2018.03.06 Support ATM Shutdown
	if (bShutdown)
		m_pDevCmn->m_JNLMgr.Save(POWER_OFF_SYSTEM);
	else
	// end of [#2536]
		m_pDevCmn->m_JNLMgr.Save(REBOOT_SYSTEM);	// [#3] NH AIREAT 2008.3.10
	SaveLogData();								// [#2131] NH PCS 2012.03.27 "S/W Reboot시 Log 남기도록 수정함." 
	m_pDevCmn->fnSCR_SetActiveMode(SCR_REAR);

	CString strTemp = _T("");
	if (!strGuideText.GetLength())
		strTemp = L"ATM will restart\nfor resource management.";
	else
		strTemp = strGuideText;

	m_pDevCmn->fnSCR_DisplayPrevSet(899);
	m_pDevCmn->fnSCR_DisplayString(1, strTemp);
	m_pDevCmn->fnSCR_DisplayScreen(899);

	// [#2536] NH Kook 2018.03.06 Support ATM Shutdown
	if (bShutdown)
	{
		m_pDevCmn->AtmDoing = FALSE;								// Atm Stop
		m_pDevCmn->fnAPL_UnLoadDevice();

		NVDump('O', 'A', "10", L"", L"SHUTDOWN FIN.");

		m_pDevCmn->fnSCR_DisplayString(1, _T(""));
		m_pDevCmn->fnSCR_DisplayScreen(899);
		Delay_Msg(500);								// wait until screen is updated.

		ClerkNoticeToUser(_T("It is now safe to turn off ATM."));

		while (true)
		{
			Delay_Msg(1000);
		}
	}
	else
	// end of [#2536]
	{
		//for (int i = 60; i > 0; i--)
		for (int i = 5; i > 0; i--)				// [#2546] NH Justin 2018.04.19 Reduce Reboot Time : 60 => 5 seconds
		{
			strTemp.Format(L"After %02d seconds.", i);
			m_pDevCmn->fnSCR_SetDisplayData(L"APUpdateStart", L"899");
			m_pDevCmn->fnSCR_DisplayString(2, strTemp);
			m_pDevCmn->fnSCR_SetDisplayData(L"APUpdate", L"899");
			m_pDevCmn->fstrSCR_WaitTime(1);
		}

		/////////////////////////////////////////////////////////////////////////////
		m_pDevCmn->AtmDoing = FALSE;								// Atm Stop
		/////////////////////////////////////////////////////////////////////////////

		// [#2ND] AIREAT 2009.01.20
		#if UNDER_CE
		{
			#define IOCTL_SYSTEM_RESET	1
			#define IOCTL_MODEM_RESET	2
			HANDLE hReset = CreateFile(L"RST1:", GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, NULL, NULL);
			//if (hReset != NULL)
			if (hReset != INVALID_HANDLE_VALUE)		// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
			{
				DWORD dwBuffer = IOCTL_SYSTEM_RESET;
				DWORD dwWritten;
				WriteFile(hReset, &dwBuffer, 1, &dwWritten, NULL);
				CloseHandle(hReset);
			}
		}
		#endif
		// end of [#2ND]
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: CMainFrame::SaveLogData
// RETURN TYPE  : CString : backup된 로그 파일명
// DESCRIPTION  : 장애 발생시 NVRam에 있는 로그 (64K)를 Flash rom 에 file로 저장한다.
// 파일명 규칙 : \\ATM2\\LOG\\01_20060605.log   (file index는 00~99까지 순환)
//////////////////////////////////////////////////////////////////////////
CString CMainFrame::SaveLogData()			// [#31] NH AIREAT 2008.03.18  BackupLogData -> SaveLogData 변경.
{
	CString strLogPath = _T("");
	int nFileIndex = MemGetInt(_MEM_FLD_INI_DEVINFO, _MEM_VAR_DEVINFO_LOGFILE_INDEX);

 	// [#156] KSK 2008.04.22 OS Version에 따른 PATH 지정
	strLogPath.Format(_T("%s\\LOG"), JNLBACKUP_SRC_PATH);
	// end of [#156]

	nFileIndex++;
	if(nFileIndex >= 20)
		nFileIndex = 0;

	// [##2387] NH KSK 2016.01.14 NVRAM Log에 있는 Time과 BackUp시의 시간 동기화처리 (CTime -> SystemTime으로 수정)
//	CTime time = CTime::GetCurrentTime();
//	int nYear = time.GetYear();
//	int nMonth = time.GetMonth();
//	int nDay = time.GetDay();
	//2. 시(1)분(1)초(1)
	SYSTEMTIME  lTime;
	GetLocalTime(&lTime);
	// end of [#2387]

	//index가 같은 파일이 있으면 지운다.
	CString strBackupFile;
	strBackupFile.Format(L"%s\\%02d_*.log", strLogPath, nFileIndex);
	
	CCeFileFind	fFinder;
	BOOL bFinding = fFinder.FindFile(strBackupFile);
	// 위의 파일명으로 파일이 찾아지게 되면 지운다.
	CString filename = "";
	while(bFinding)
	{
		bFinding = fFinder.FindNextFile();
		filename = fFinder.GetFilePath();
		DeleteFile(filename);
	}
	fFinder.Close();	// CFileFind 객체변수 close

//	strBackupFile.Format(L"%s\\%02d_%04d%02d%02d.log", strLogPath, nFileIndex,nYear, nMonth, nDay);
	strBackupFile.Format(L"%s\\%02d_%04d%02d%02d.log", strLogPath, nFileIndex,lTime.wYear, lTime.wMonth, lTime.wDay);	// [#2387] NH KSK 2016.01.14

	// [#2567] AU HJAHN 2018.07.25 LOG 파일 저장 실패로 인한 파일 저장 방식 변경( Log 0kb 오류 ) 
	//		1. NVRAM 복사 실패 시, NVDUMP 추가 후 1회 복사 시도 
	//		2. codesonar 지적 사항 수정 ( Null Pointer Dereference )
	
	/*
	pLog pNVRAM;
	HANDLE hFile = INVALID_HANDLE_VALUE;	// [#2022] NH KSK 2011.02.22
	DWORD dwBytesWritten = 0; //, dwPos;

	pNVRAM=	(pLog)GetLOGAddr(0);                                                                                                          	
  // Open the existing file.

	hFile = CreateFile (strBackupFile,    // Open NVRAM.LOG
                      GENERIC_WRITE,           // Open for reading
                      0,                      // Do not share
                      NULL,                   // No security
                      CREATE_ALWAYS,          // Existing file only
                      FILE_ATTRIBUTE_NORMAL,  // Normal file
                      NULL);                  // No template file

	if (hFile == INVALID_HANDLE_VALUE)
	{
	// Your error-handling code goes here.
		strBackupFile = "";
		return strBackupFile;
	}

	// Append the first file to the end of the second file.

	dwPos = SetFilePointer (hFile, 0, NULL, FILE_END);
	WriteFile (hFile,(LPCVOID )pNVRAM, sizeof(sLog), &dwBytesWritten, NULL);
	// Close both files.

	CloseHandle (hFile);
	FreeLOGAddr(pNVRAM);
	*/

	int nRet = 0; 
	DWORD dwBytesWritten = 0;
	pLog pNVLog;

	if( (pNVLog = (pLog)GetLOGAddr(0)) != NULL )
	{
		HANDLE hFile = INVALID_HANDLE_VALUE;

		hFile = CreateFile (strBackupFile,    // Open NVRAM.LOG
			GENERIC_WRITE,           // Open for reading
			0,                      // Do not share
			NULL,                   // No security
			CREATE_ALWAYS,          // Existing file only
			FILE_ATTRIBUTE_NORMAL,  // Normal file
			NULL);                  // No template file

		if (hFile != INVALID_HANDLE_VALUE)
		{

			SetFilePointer (hFile, 0, NULL, FILE_END);
			nRet = WriteFile (hFile,(LPCVOID )pNVLog, sizeof(sLog), &dwBytesWritten, NULL);
				
			CloseHandle (hFile);
		}
		FreeLOGAddr(pNVLog);
	}

	//	NVRAM 복사 실패 시, NVDUMP 추가 후 1회 복사 시도
	if( nRet == FALSE )
	{
		NVDump('O', 'A', "10", L"", L"FAIL:SAVELOG");	

		// 재시도
		if( (pNVLog = (pLog)GetLOGAddr(0)) != NULL )
		{
			HANDLE hFile = INVALID_HANDLE_VALUE;

			hFile = CreateFile (strBackupFile,    // Open NVRAM.LOG
				GENERIC_WRITE,           // Open for reading
				0,                      // Do not share
				NULL,                   // No security
				CREATE_ALWAYS,          // Existing file only
				FILE_ATTRIBUTE_NORMAL,  // Normal file
				NULL);                  // No template file

			if (hFile != INVALID_HANDLE_VALUE)
			{
				SetFilePointer (hFile, 0, NULL, FILE_END);
				nRet = WriteFile (hFile,(LPCVOID )pNVLog, sizeof(sLog), &dwBytesWritten, NULL);

				CloseHandle (hFile);
			}

			FreeLOGAddr(pNVLog);
		}
	}
	// end of [#2567]

	// [#2238] AU KMK 2013.12.18 CDU Sensor Log 파일 백업
	CString strCDUMSensorLogBackupFile;
//	strCDUMSensorLogBackupFile.Format(L"%s\\%02d_%04d%02d%02d_CDUM_SensorLog.log", strLogPath, nFileIndex, nYear, nMonth, nDay);
	strCDUMSensorLogBackupFile.Format(L"%s\\%02d_%04d%02d%02d_CDUM_SensorLog.log", strLogPath, nFileIndex, lTime.wYear, lTime.wMonth, lTime.wDay);	// [#2387] NH KSK 2016.01.14

	// 파일이름 변경 \ATM2\LOG\CDUM_Sensorlog.log -> \ATM2\LOG\00_20131216_CDUM_Sensorlog.log
	if (IsExistFile(CDU_SP_SENSOR_LOG_FILE))
	{
		CopyFile(CDU_SP_SENSOR_LOG_FILE, strCDUMSensorLogBackupFile, FALSE);
		DeleteFile(CDU_SP_SENSOR_LOG_FILE);		// SP에서 로그를 매번 신규생성하므로 AP에서 삭제하지 않아도 됨
	}
	// end of [#2238]

	MemSetInt(_MEM_FLD_INI_DEVINFO, _MEM_VAR_DEVINFO_LOGFILE_INDEX, nFileIndex);
	NVDump('O', 'A', "10", L"", L"LOGBACKUP");
	
	return strBackupFile;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: CMainFrame::BackupLogData
// RETURN TYPE  : 
// PARAMETER    : 
// DESCRIPTION  : 저장된 로그와 NVRAM의 로그를 USB로 복사한다.
//////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::BackupLogData(CString strDestPath)
{
	WIN32_FIND_DATA	FileData;
	HANDLE			hSearch = INVALID_HANDLE_VALUE;		// [#2022] NH KSK 2011.02.22
	CString			strLOGRoot;
	CString			strSrcFileName, strDestFileName, strSaveFileName;
	CTime			CurTime = CTime::GetCurrentTime();

	strLOGRoot.Format(_T("%s\\LOG"), JNLBACKUP_SRC_PATH);	// [#156] KSK 2008.04.22 OS Version에 따른 PATH 지정

	CreateDirectory(strDestPath, NULL);

	///////////////////////////////////////////////////
	// Delete previous saved file
	strSrcFileName.Format(_T("%s\\*.*"), strDestPath);
	hSearch = FindFirstFile(strSrcFileName, &FileData);
	if (hSearch != INVALID_HANDLE_VALUE)
	{
		while(1)
		{
			strSrcFileName.Format(_T("%s\\%s"), strDestPath, FileData.cFileName);
			
			DeleteFile(strSrcFileName);
			
			if (!FindNextFile(hSearch, &FileData))
				break;
		}
		
		FindClose(hSearch);
	}

	///////////////////////////////////////////////////
	// copy log file from LOG folder to USB folder
	// make file name for search.
	strSrcFileName.Format(_T("%s\\*.log"), strLOGRoot);
	hSearch = FindFirstFile(strSrcFileName, &FileData);
	if (hSearch != INVALID_HANDLE_VALUE)
	{
		BOOL bSuccess = FALSE;
		
		while(1)
		{
			strSrcFileName.Format(_T("%s\\%s"), strLOGRoot, FileData.cFileName);
			strDestFileName.Format(_T("%s\\%s"), strDestPath, FileData.cFileName);
			
			if (!CopyFile(strSrcFileName, strDestFileName, FALSE))
				break;
			
			if (!FindNextFile(hSearch, &FileData))
			{
				bSuccess = TRUE;
				break;
			}
		}
		
		FindClose(hSearch);

		if (!bSuccess)
			return FALSE;
	}

#ifdef MEM_LEAK_CHECK
	strSrcFileName.Format(L"\\ATM\\MEM_HIS.DAT");
	strDestFileName.Format(L"%s\\MEM_HIS.DAT", strDestPath);
	CopyFile(strSrcFileName, strDestFileName, FALSE);
#endif
	
	///////////////////////////////////////////////////
	// copy log file from LOG block to USB folder
	{
		pLog pNVLog;
		
		strSaveFileName.Format(_T("%s\\CLOG.log"), strDestPath);

		// open file
		// [#2567] AU HJAHN 2018.07.25 CLOG 파일 저장 실패로 인한 파일 저장 방식 변경( CLog 0kb 오류 ) 
		//		1. fwrite -> WRITEFILE 로 변경
		//		2. 파일 쓰기 실패 시, FALSE 로 리턴
		//		3. NVRAM 복사 실패 시, NVDUMP 추가 후 1회 복사 시도 
//		FILE			*Stream;
// 		if ( (Stream = _wfopen(strSaveFileName, L"w+b")) != NULL)
// 		{
// 			pNVLog = (pLog)GetLOGAddr(0);
// 
// 			fwrite((void*)pNVLog, sizeof(char), sizeof(sLog), Stream);
// 			
// 			fflush(Stream);
// 			fclose(Stream);
// 
// 			FreeLOGAddr(pNVLog);
// 		}
// 		else
// 			return FALSE;

		BOOL bReturn = FALSE;
		for (int retryCnt = 0; retryCnt < 2; retryCnt++)
		{
			if( (pNVLog = (pLog)GetLOGAddr(0)) != NULL )
			{ 
				HANDLE hFile = INVALID_HANDLE_VALUE;
				hFile = CreateFile (strSaveFileName,    // CLOG.log				
					GENERIC_WRITE,						// Open for Writing
					0,								    // Do not share
					NULL,								// No security
					CREATE_ALWAYS,						// Existing file only
					FILE_ATTRIBUTE_NORMAL,				// Normal file
					NULL);							    // No template file

				if (hFile != INVALID_HANDLE_VALUE)
				{
					DWORD dwBytesWritten = 0;

					SetFilePointer (hFile, 0, NULL, FILE_END);
					bReturn = WriteFile(hFile, (LPCVOID)pNVLog, sizeof(sLog), &dwBytesWritten, NULL);

					CloseHandle (hFile);
				}

				FreeLOGAddr(pNVLog);
			}

			if (bReturn == TRUE)
			{
				NHDEBUG(1, (_T("BACKUPLOGDATA Success.\n")));
				break;
			}
			else
			{
				NHDEBUG(1, (_T("BACKUPLOGDATA Fail.\n")));
				NVDump('F', 'A', "10", L"", L"FAIL:BACKUPLOG");	//	NVRAM 복사 실패 시, NVDUMP 추가 후 1회 복사 시도
			}
		}
		// end of [#2567]
	}

	// [#2220] NH KMK 2013.10.22 LOG Backup 시 SP/EP 버전을 별도 파일에 기록한다
	// 위에서 strDestPath에 있는 모든 파일(*.*)을 이미 삭제하므로 기존 파일 유무는 확인할 필요 없음

	// 버전 정보 파일은 CVER.log
	strSaveFileName.Format(_T("%s\\CVER.log"), strDestPath);

	// 버전 로그 형식은 따로 없음
	CString strVersion;
	strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"CDM");	//	CDU SP
	strVersion += FIELD_DELIMITER;
	strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"SPR");	//	SPR SP
	strVersion += FIELD_DELIMITER;
	strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"IDC");	//	MCU SP
	strVersion += FIELD_DELIMITER;
	strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"PIN");	//	PIN SP
	strVersion += FIELD_DELIMITER;
	strVersion += MemGetVersion(_MEMKEY_SPVERSION, L"SIU");	//	SIU SP
	strVersion += FIELD_DELIMITER;

	strVersion += MemGetVersion(_MEMKEY_EPVERSION, L"CDM");	//	CDU EP
	strVersion += FIELD_DELIMITER;
	strVersion += MemGetVersion(_MEMKEY_EPVERSION, L"SPR");	//	SPR EP
	strVersion += FIELD_DELIMITER;
	strVersion += MemGetVersion(_MEMKEY_EPVERSION, L"IDC");	//	MCU EP
	strVersion += FIELD_DELIMITER;
	strVersion += MemGetVersion(_MEMKEY_EPVERSION, L"PIN");	//	PIN EP
	strVersion += FIELD_DELIMITER;
 	strVersion += "N/A";									//	SIU EP Version은 없음 (N/A)

	// file write
	CFile cverFile;
	if (cverFile.Open(strSaveFileName, CFile::modeCreate|CFile::modeWrite) == TRUE)
	{
		cverFile.Write(strVersion, strVersion.GetLength() * 2);
		cverFile.Close();
	}
	else
		return FALSE;
	// end of [#2220]

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: CMainFrame::KillProcByName
// RETURN TYPE  : int
// PARAMETER    : const char *szToTerminate
// DESCRIPTION  : 타 process를 강제로 kill한다.
//////////////////////////////////////////////////////////////////////////
int CMainFrame::KillProcByName(const char *szToTerminate)
{
    HANDLE         hProcessSnap = INVALID_HANDLE_VALUE;	// [#2022] NH KSK 2011.02.22 
    BOOL           bRet      = 0; 
    PROCESSENTRY32 pe32      = {0}; 

	int iLenP,indx;
	char szToTermUpper[128];
	char szTemp[128];

	iLenP=strlen(szToTerminate);

	for(indx=0;indx<iLenP;indx++)	szToTermUpper[indx]=toupper(szToTerminate[indx]);
	szToTermUpper[iLenP]=0;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

    if (hProcessSnap == INVALID_HANDLE_VALUE)		return 0;

    pe32.dwSize = sizeof(PROCESSENTRY32); 
    if (Process32First(hProcessSnap, &pe32)) 
    { 
        BOOL         bGotModule = FALSE; 
        MODULEENTRY32	me32       = {0}; 
		 char*			pCharFound;

        do 
        { 
			HANDLE hProcess = NULL;	// [#2022] NH KSK 2011.02.22

			NHINFO((L"Killing process with ID: %d\r\n", pe32.th32ProcessID));

            // Get the actual priority class. 
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID); 

			memset(szTemp, NULL, sizeof(szTemp));
			WideToMulti(szTemp, pe32.szExeFile, sizeof(szTemp));
			
			pCharFound=NULL;
			pCharFound = strstr(_strupr(szTemp), szToTermUpper);
			if(pCharFound != NULL)
			{
				if (hProcess != NULL)		// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대응
				{
					TerminateProcess(hProcess, 0);
					CloseHandle (hProcess);
				}					// end of [#2022]
			}
			else
			{
				if (hProcess != NULL)		// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대응
					CloseHandle(hProcess);
			}

        } 
        while (Process32Next(hProcessSnap, &pe32)); 
        bRet = 1; 
    } 
    else         bRet = 0;
	
#ifdef UNDER_CE
	CloseToolhelp32Snapshot(hProcessSnap);
#endif

	return bRet;

}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: CMainFrame::KillProcByGoma
// RETURN TYPE  : int
// PARAMETER    : Process name (Supports multiple values like: "A.exe B.exe C.exe")
// DESCRIPTION  : Goma will kill given process.
//////////////////////////////////////////////////////////////////////////
int CMainFrame::KillProcByGoma(CString sExeName)
{
	// Execute SPClear.bat
	NHDEBUG(1, (_T("Teminating: %s ...\n"), sExeName));
	SHELLEXECUTEINFO ShExecInfo = {0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = _T("\\ATM\\Goma.exe");
	ShExecInfo.lpParameters = sExeName;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL; 
	ShellExecuteEx(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess, 10 * 1000);
	NHDEBUG(1, (_T("Teminating ... DONE\n"), sExeName));

	return 0;
}

BOOL CMainFrame::KillProcess(CString sExeName)
{
	/************************************************************************/
	/* Eocs_2007_0823 : Kill Process by Exe Name
	/************************************************************************/
	sExeName.MakeUpper();

	HANDLE hSnapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPPROCESS, 0 );

	if ( (int)hSnapshot != -1 )
	{
		PROCESSENTRY32 pe32 ;
		pe32.dwSize=sizeof(PROCESSENTRY32);
		BOOL bContinue ;
		CString strProcessName;

		if ( Process32First ( hSnapshot, &pe32 ) )
		{
			do
			{
				strProcessName = pe32.szExeFile; //strProcessName이 프로세스 이름;
				strProcessName.MakeUpper();
				if( ( strProcessName.Find(sExeName,0) != -1 ) )
				{
					HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, 0, pe32.th32ProcessID );
					if( hProcess )
					{
						DWORD       dwExitCode;
						GetExitCodeProcess( hProcess, &dwExitCode);
						TerminateProcess( hProcess, dwExitCode);
						CloseHandle(hProcess);
						CloseHandle( hSnapshot );

						return TRUE;
					}
					return FALSE;
				}
				bContinue = Process32Next ( hSnapshot, &pe32 );

			} while ( bContinue );
		}

		CloseHandle( hSnapshot );
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// Reset Device Of
//////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::ResetDeviceOf(int AllorErrDev)
{
	BOOL nRet = FALSE;

	m_pDevCmn->m_JNLMgr.Save(CHANGE_OP_RESET);	// [#3] NH AIREAT 2008.3.10

	if(MemGetInt(_MEM_FLD_SP_CDM,_MEM_VAR_CDMSP_FLAGENQSEND))
	{
		MemSetInt(_MEM_FLD_SP_CDM,_MEM_VAR_CDMSP_FLAGENQSEND, 0);
		NVDump('O', 'A', "03", L"", L"ENQFlagClr" );
	}
	
	if(AllorErrDev == ALLDEV)
	{
		// [#2325] NH KSK 2015.01.25
//		if(m_pDevCmn->fnAPL_ResetDevice(DEV_MAIN))
//			nRet = TRUE;
//		else
//			nRet = FALSE;
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
		{
			if (m_pDevCmn->fnAPL_ResetDevice(DEV_MAIN | DEV_RFID))
				nRet = TRUE;
			else
				nRet = FALSE;
		}
		else
		{
			if (m_pDevCmn->fnAPL_ResetDevice(DEV_MAIN))
				nRet = TRUE;
			else
				nRet = FALSE;
		}
		// end of [#2325]
	}
	else
	{
		if(m_pDevCmn->fnAPL_ResetDevice(m_pDevCmn->fnAPL_GetErrorDevice(m_pDevCmn->DeviceStatus) | m_pDevCmn->DeviceDownDevice))
			nRet = TRUE;
		else
			nRet = FALSE;
	}
	return nRet;
}

int	CMainFrame::GetCurrnetRMSLineType()
{
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP)
		return RMS_LINE_DIALUP;

	return RMS_LINE_TCP;
}

// [#462] [NH] KSK 2008.12.09 Added Journal Error Code
//#define SYSTEM_ERROR_COUNT	12
//#define SYSTEM_ERROR_COUNT	13
//#define SYSTEM_ERROR_COUNT	14	// [#2115] [MX] KSK 2012.02.08
//#define SYSTEM_ERROR_COUNT	13	// [#2137] [MX] KSK 2012.07.25
#define SYSTEM_ERROR_COUNT	16	// [#2277], [#2278] NH Justin 2014.06.10 Check Service and Operator Passwords, Remote Status

CString	SystemErrTbl[SYSTEM_ERROR_COUNT] = {
	L"FFFFFFF", L"2000500", L"F000200", L"F000300", L"F000900", 
	L"F000A00", L"F000B00", L"F000D00", L"F000E00", L"F000F00", 
	L"F001000", L"F001600", L"F00FF00", L"F002100", L"F002200",			// [#2115] [MX] KSK 2012.02.08 [#2137] MX KSK 2012.07.25 // [#2278] NH Justin 2014.06.10
	L"F002000"															// [#2277] Remote Status
};
// end of [#462]

// [#419] [NH] 2008.9.18
BOOL CMainFrame::Check_NVRamData()
{
	// SYSTEM 장애를 SET하는 부분과 자동 복구하는 부분을 나눔
	CString strTemp;
	int		i=0;
	CLoginManager login;

	if (!m_pDevCmn->fnAPL_CheckError())
	{
		// SYSTEM ERROR 존재 여부 CHECK
		strTemp = m_pDevCmn->fstrAPL_GetErrorCode();
		for(i=0; i<SYSTEM_ERROR_COUNT; i++)
		{
			if (strTemp.CompareNoCase(SystemErrTbl[i]) == 0)
			{
				// 자동 복구 조건 CHECK
				switch(i)
				{
				case 0:	// "FFFFFFF"
					// 자동 복구 안함
					break;

				case 1:	// 20005 Currency Code
					// [#2042] AU KSK 2011.04.02
					#if (AU_VERSION)
						if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
						{
							if (m_pDevCmn->m_strCurrencyID == CURRENCY_TYPE)
								m_pDevCmn->fnAPL_ClearError();
						}
						else
						{
							if (m_pDevCmn->m_strCurrencyID == CURRENCY_TYPE_NZ)
								m_pDevCmn->fnAPL_ClearError();
						}
					#elif (MX_VERSION)
						// [#2115] MX KSK 2012.02.08
						if (m_pDevCmn->m_strCurrencyID == CURRENCY_TYPE || m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
							m_pDevCmn->fnAPL_ClearError();
						// end of [#2115]
					#else
						if (m_pDevCmn->m_strCurrencyID == CURRENCY_TYPE)
							m_pDevCmn->fnAPL_ClearError();
					#endif
					// end of [#2042]
					break;

				case 2:
					if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE) == 0 ||
						MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER).GetLength() > 0)
						m_pDevCmn->fnAPL_ClearError();
					break;

				case 3:
					// [#440] [NH] KSK 2008.10.28
					if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE) == 0 ||
						Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT)) != 0)
					// end of [#440]
						m_pDevCmn->fnAPL_ClearError();
					break;

				// KSK 2009.08.13 KEY 관련 ERROR인 경우 자동 복구하도록 LOGIC 추가
				case 4:	// "F000900"
					if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX) >= 0 && MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX) <= 15)
						m_pDevCmn->fnAPL_ClearError();
					break;

				case 5:	// "F000A00"
					// [#2172] NH KSK 2012.12.27
					if (Chk_Key[__min(MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX), MAX_PIN_KEYINDEX)] != _T(""))
						m_pDevCmn->fnAPL_ClearError();
					// end of [#2172]
					// [#2314] US Justin 2014.12.11 Fix Checking MasterKey checksum Routine
					else
					{
						if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_ENABLE)==ENABLE)&&(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_HYOSUNG_TYPE) )
							m_pDevCmn->fnAPL_ClearError();
					}
					// End of [#2314]
					break;
				// end of KSK 2009.08.13

				case 6:
					{
						if ((MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
							Asc2Int(MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1)) > 0) ||
							(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_TCPIP &&
							MemGetStr(_MEM_FLD_TCPIPINFO,_MEM_VAR_TCPIP_HOST1NAME).GetLength() > 0))
							m_pDevCmn->fnAPL_ClearError();
					}
					break;

				case 7:
					if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSFLAG) == 0 ||
						MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPASSWORD).GetLength() > 0)
						m_pDevCmn->fnAPL_ClearError();
					break;

				case 8:
					if ((MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSSTATUSSEND) == 0) ||
						(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP &&
						 MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPHONENO1).GetLength() > 0) ||
						(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_TCPIP &&
						 MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSNAME).GetLength() > 0))
						m_pDevCmn->fnAPL_ClearError();
					break;

				case 9:
					strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
					if (strTemp != TERMINAL_ID_DEFAULT && strTemp.GetLength() > 0)
						m_pDevCmn->fnAPL_ClearError();
					break;

				case 10:
					if(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
					{
						// STANDARD3
						if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE) == 0 ||
							MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID).GetLength() > 0)
							m_pDevCmn->fnAPL_ClearError();
					}
					else
					{
						// STANDARD1,2
						if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID).GetLength() > 0)
							m_pDevCmn->fnAPL_ClearError();
					}
					break;

				case 11:
					if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MASTERPW) != MASTERPW_DEFAULT)
						m_pDevCmn->fnAPL_ClearError();
					break;

				case 12:// "F00FF00"	[#462] [NH] KSK 2008.12.09
					// 자동 복구 안함 (SYSTEM REBOOT을 해야하므로)
					break;

				// [#2278] NH Justin 2014.06.10
				case 13:
					if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SERVICEPW) != SERVICEPW_DEFALUT)
						m_pDevCmn->fnAPL_ClearError();
					break;

				case 14:
					if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_OPERATORPW) != OPERATORPW_DEFALUT)
						m_pDevCmn->fnAPL_ClearError();
					break;
				// End of [#2278]

				// [#2277] NH Justin 204.06.10
				case 15:
					if (m_pDevCmn->m_nRemoteStatusCmd==0)
						m_pDevCmn->fnAPL_ClearError();
					break;
				// End of [#2277]
				}
				break;
			}
		}
	}

	if (m_pDevCmn->fnAPL_CheckError())
	{
		////////// 0. NVRAM BROKEN  ///////////////////////////////////////////////////////////////////////
		// if System Error exist, System error code set
		if (Check_BrokenNVRAM() == TRUE)	// KSK 2008.9.18
		{
			m_pDevCmn->fnAPL_StackError(_T("FFFFFFF"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003043), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
			return FALSE;
		}

		// [#181] KSK 2008.05.02 CURRENCY ID CHECK
		////////// 1. Currency ID  ///////////////////////////////////////////////////////////////////////
		// [#2042] AU KSK 2011.04.02
		#if (AU_VERSION)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AU_COUNTRY_SELECTION) == COUNTRY_AUS)
			{
				if (m_pDevCmn->m_strCurrencyID != CURRENCY_TYPE)
				{
					m_pDevCmn->fnAPL_StackError(_T("2000500"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003044), DEV_CDU);	// [#419] [NH] KSK 2008.9.16
					NVDump('F', 'A', "11", L"", L"1:2000500" );
					return FALSE;
				}
			}
			else
			{
				if (m_pDevCmn->m_strCurrencyID != CURRENCY_TYPE_NZ)
				{
					m_pDevCmn->fnAPL_StackError(_T("2000500"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003044), DEV_CDU);	// [#419] [NH] KSK 2008.9.16
					NVDump('F', 'A', "11", L"", L"1:2000500" );
					return FALSE;
				}
			}
		#elif (MX_VERSION)
			// [#2115] MX KSK 2012.02.08
			if (!(m_pDevCmn->m_strCurrencyID == CURRENCY_TYPE || m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE))
			{
				m_pDevCmn->fnAPL_StackError(_T("2000500"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003044), DEV_CDU);
				NVDump('F', 'A', "11", L"", L"1:2000500" );
				return FALSE;
			}
			// end of [#2115]
		#else
			if (m_pDevCmn->m_strCurrencyID != CURRENCY_TYPE)
			{
				m_pDevCmn->fnAPL_StackError(_T("2000500"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003044), DEV_CDU);	// [#419] [NH] KSK 2008.9.16
				NVDump('F', 'A', "11", L"", L"1:2000500" );
				return FALSE;
			}
		#endif
		// end of [#2042]

		////////// 2. Surcharge owner ///////////////////////////////////////////////////////////////////////
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER);
		if(!strTemp.GetLength())
		{
			if(MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE) == ENABLE)
			{
				m_pDevCmn->fnAPL_StackError(_T("F000200"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003045), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
				NVDump('F', 'A', "11", L"", L"1:F000200" );
				return FALSE;
			}
		}

		////////// 3. Surcharge Amount ///////////////////////////////////////////////////////////////////////
		// [#440] [NH] KSK 2008.10.28
		#if !(AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
			//strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT);
			//if(!strTemp.GetLength())
			if	((MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE) == ENABLE)  &&
				 (Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT)) == 0))
			{
				m_pDevCmn->fnAPL_StackError(_T("F000300"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003046), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
				NVDump('F', 'A', "11", L"", L"1:F000300" );
				return FALSE;
			}
			// end of [#440]
		#endif

		////////// 4. Master Key Index ///////////////////////////////////////////////////////////////////////
		int nTemp = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX);
		if(nTemp < 0 || nTemp > 15 )
		{
			m_pDevCmn->fnAPL_StackError(_T("F000900"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003047), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
			NVDump('F', 'A', "11", L"", L"1:F000900" );
			return FALSE;
		}

#ifdef UNDER_CE
		////////// 5. Master Key Check ///////////////////////////////////////////////////////////////////////
		if (Chk_Key[__min(MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX), MAX_PIN_KEYINDEX)] == _T(""))	// [#2172] NH KSK 2012.12.27
		{
			// [#2314] US Justin 2014.12.11 Fix Checking MasterKey checksum Routine
			/*
			// [#2075] NH KSK 2011.07.12
			if ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_AVAILABLE) == 0) && (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE))
			{
				m_pDevCmn->fnAPL_StackError(_T("F000A00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003048), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
				NVDump('F', 'A', "11", L"", L"1:F000A00" );
				NHDEBUG(1, (_T("Check CheckSum (%d)\n"), MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX)));
				return FALSE;
			}
			// end of [#2075]
			*/
			if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_ENABLE)!=ENABLE)||(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)!=MSG_HYOSUNG_TYPE) )
			{
				m_pDevCmn->fnAPL_StackError(_T("F000A00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003048), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
				NVDump('F', 'A', "11", L"", L"1:F000A00" );
				NHDEBUG(1, (_T("Check CheckSum (%d)\n"), MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX)));
				return FALSE;
			}
			// End of [#2314]
		}
#endif

		///////// 6. Host Phone Number or Host IP Address ///////////////////////////////////////////////////////////////////////
		#ifndef APP_LOCAL_MODE
			// Host Phone Number
			strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1);
			if(!strTemp.GetLength() || strTemp == HOSTPHONE1_DEFAULT)	// [#426] [NH] KSK 2008.9.24
			{
				if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP)
				{
					m_pDevCmn->fnAPL_StackError(_T("F000B00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003049), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
					NVDump('F', 'A', "11", L"", L"1:F000B00" );
					return FALSE;
				}
			}
			// TCP/IP INVALID CHECK
			// FIELD 호환성을 위해 OS Version을 확인한 후에 INVALID를 CHECK한다. // NVRAM이 깨질 경우를 고려해 Registery에서 직접 읽어와서 확인한다.
#ifdef UNDER_CE
			if ((RegGetStr(L"SOFTWARE\\ATM", L"OSVersion")).CompareNoCase(L"V01.01.06") >= 0)
#endif
			{
				strTemp = MemGetStr(_MEM_FLD_TCPIPINFO,_MEM_VAR_TCPIP_HOST1NAME);
				if(!strTemp.GetLength())
				{
					if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_TCPIP)
					{
						m_pDevCmn->fnAPL_StackError(_T("F000B00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003056), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
						NVDump('F', 'A', "11", L"", L"1:F000B00" );
						return FALSE;
					}
				}
			}
		#endif

		////////// 7. RMS Password ///////////////////////////////////////////////////////////////////////
		strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPASSWORD);
		if(!strTemp.GetLength())
		{
			if(MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSFLAG) == ENABLE) //RMS Enable이면
			{
				m_pDevCmn->fnAPL_StackError(_T("F000D00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003050), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
				NVDump('F', 'A', "11", L"", L"1:F000D00" );
				return FALSE;
			}
		}

		////////// 8. RMS Phone Number or RMS IP Address ///////////////////////////////////////////////////////////////////////
		#ifndef APP_LOCAL_MODE
			// RMS Phone Number
			strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPHONENO1);
			if(!strTemp.GetLength())
			{
				if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP)
				{
					if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSSTATUSSEND) == ENABLE)
					{
						m_pDevCmn->fnAPL_StackError(_T("F000E00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003051), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
						NVDump('F', 'A', "11", L"", L"1:F000E00" );
						return FALSE;
					}
				}
			}

			// RMS IP Address CHECK
			// FIELD 호환성을 위해 OS Version을 확인한 후에 INVALID를 CHECK한다. // NVRAM이 깨질 경우를 고려해 Registery에서 직접 읽어와서 확인한다.
			if ((RegGetStr(L"SOFTWARE\\ATM", L"OSVersion")).CompareNoCase(L"V01.01.06") >= 0)
			{
				strTemp = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSNAME);
				if(!strTemp.GetLength())
				{
					if( (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE)==NETWORK_TCPIP) &&(MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSSTATUSSEND)==ENABLE) )
					{
						m_pDevCmn->fnAPL_StackError(_T("F000E00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003057), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
						NVDump('F', 'A', "11", L"", L"1:F000E00" );
						return FALSE;
					}
				}
			}
		#endif

		////////// 9. Terminal ID ///////////////////////////////////////////////////////////////////////
		#ifndef APP_LOCAL_MODE
			strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
			if(strTemp == TERMINAL_ID_DEFAULT || !strTemp.GetLength())	// [#398] [NH] KSK 2008.8.6
			{
				m_pDevCmn->fnAPL_StackError(_T("F000F00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003052), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
				NVDump('F', 'A', "11", L"", L"1:F000F00" );
				return FALSE;
			}
		#endif

		////////// 10 Communication ID (Triton) or Routing ID(STD1 and STD3)///////////////////////////////////////////////////////////////////////
		if(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
		{
			strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID);

			if(!strTemp.GetLength())
			{
				if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE) == ENABLE)
				{
					m_pDevCmn->fnAPL_StackError(_T("F001000"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003053), DEV_SYS);	// [#419] [NH] KSK 2008.9.16	
					NVDump('F', 'A', "11", L"", L"1:F001000" );
					return FALSE;
				}
			}
		}
		else
		{
			strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID);

			if(!strTemp.GetLength())
			{
				m_pDevCmn->fnAPL_StackError(_T("F001000"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003054), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
				NVDump('F', 'A', "11", L"", L"1:F001000" );
				return FALSE;
			}
		}

		////////// 11. Master Password ///////////////////////////////////////////////////////////////////////
		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MASTERPW) == MASTERPW_DEFAULT)	// [#419] [NH] KSK 2008.9.18
		{
			m_pDevCmn->fnAPL_StackError(_T("F001600"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003055), DEV_SYS);	// [#419] [NH] KSK 2008.9.16
			NVDump('F', 'A', "11", L"", L"1:F001600" );
			return FALSE;
		}

		////////// 12. Check Journal Status ///////////////////////////////////////////////////////////////////////
		// [#462] [NH] KSK 2008.12.09
		if (m_pDevCmn->m_JNLMgr.GetJNLStatus() == DOWN)
		{
			m_pDevCmn->fnAPL_StackError(_T("F00FF00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003088), DEV_SYS);
			NVDump('F', 'A', "11", L"", L"1:F00FF00" );
			return FALSE;
		}
		// end of [#462]


		////////// 13. Service Password ///////////////////////////////////////////////////////////////////////
		//[#2278] US Justin 2014.06.10 Check Default Service and Operator Passwords.
		#ifndef APP_LOCAL_MODE	// [#2300] US KSK 2014.11.04	Local Mode시에는 Password 관련 Check하지 않도록 보완 처리
			#if !(AU_VERSION)	// KSK 2017.01.09 AU는 미적용
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SERVICEPW) == SERVICEPW_DEFALUT)
			{
				m_pDevCmn->fnAPL_StackError(_T("F002100"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003402), DEV_SYS);
				NVDump('F', 'A', "11", L"", L"1:F002100" );
				return FALSE;
			}
			#endif	// end of KSK 2017.01.09
		#endif					// end of [#2300]
		// End of [#2278]

		////////// 14. Operator Password ///////////////////////////////////////////////////////////////////////
		//[#2278] US Justin 2014.06.10 Check Default Service and Operator Passwords.
		#ifndef APP_LOCAL_MODE	// [#2300] US KSK 2014.11.04	Local Mode시에는 Password 관련 Check하지 않도록 보완 처리
			#if !(AU_VERSION)	// KSK 2017.01.09 AU는 미적용
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_OPERATORPW) == OPERATORPW_DEFALUT)
			{
				m_pDevCmn->fnAPL_StackError(_T("F002200"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003403), DEV_SYS);
				NVDump('F', 'A', "11", L"", L"1:F002200" );
				return FALSE;
			}
			#endif	// end of KSK 2017.01.09
		#endif					// end of [#2300]
		// End of [#2278]

		////////// 15. Remote ATM Status ///////////////////////////////////////////////////////////////////////
		// [#2277] NH Justin 204.06.10
		#if (US_VERSION || CA_VERSION || MX_VERSION)		// US, CA, MX
			if (m_pDevCmn->m_nRemoteStatusCmd != 0)
			{
				m_pDevCmn->fnAPL_StackError(_T("F002000"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003401), DEV_SYS);
				NVDump('F', 'A', "11", L"", L"1:F002000" );
				return FALSE;
			}
		#endif
		// End of [#2277]
	}	
	return TRUE;
}

// [#8] KSK 2008.03.19 RMS / HEALTH CHECK 송신 일원화
int	CMainFrame::Main_NMS_Proc()
{
	BOOL bResult = FALSE;

	// [RWC6-438] WSW Update DM on ATM state
	#if(APP_DIGITALMINT)
		// Procedure determines whether update should be sent or not
		m_pTranCmn->F_NH_DigitalMintReportDeviceStatusFlow(false);

		// [#RWC6-465] US ryan.payton 2022.12.22 DigitalMint check cassette status
		CString strAcceptorStatus;

		COleDateTime timeNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan timeDiff = timeNow - m_tLastAcceptorStatusCheck;

		// Has time elapsed two seconds?
		if((UINT)timeDiff.GetTotalSeconds() >= 2)
		{
			strAcceptorStatus = m_pDevCmn->fstrBNA_GetAcceptorStatus();
			m_tLastAcceptorStatusCheck = COleDateTime::GetCurrentTime();
		}

		if(!m_bAcceptorStatusChanged && strAcceptorStatus == L"NOACCEPT")
		{
			m_bAcceptorStatusChanged = true;
			m_pTranCmn->F_NH_DigitalMintReportDeviceEvent(DMEC_CashCassetteRemoved);
		}

		if(m_bAcceptorStatusChanged && strAcceptorStatus == L"OK")
		{
			m_bAcceptorStatusChanged = false;
			m_pTranCmn->F_NH_DigitalMintReportDeviceEvent(DMEC_CashCassetteInserted);
		}
		// End of [#RWC6-465]
	#endif
	// End of [RWC6-438]

	// [#2035] NH KSK 2011.03.22 ATM_TRAN 모드 삭제로 인한 Code 변경
	// [#113] AIREAT 2008.05.07 for rms connection close
//	if (m_pDevCmn->AtmStatus == ATM_CLERK || m_pDevCmn->AtmStatus == ATM_TRAN)
	if (m_pDevCmn->AtmStatus == ATM_CLERK)
	{
		m_pDevCmn->fnNET_RMSConnectClose();
		return T_OK;
	}
	// end of [#113]

/////////////////////////////////////////////////////////////////////////////
//	Make Flag Procedure(Non Condition Procedure)
/////////////////////////////////////////////////////////////////////////////

	// [#45] KSK 2008.03.19
	// Memory 초과시 REBOOT LOGIC 재검증 필요
	// 기존 ATM_READY에서 Memory 초과로 "OUT OF MEMORY" 였을 경우
	// REBOOT가 되지 않았음 (Virtual Memory도 같이 Check 필요)

	if (m_pDevCmn->AtmStatus != ATM_CLERK)
	{	
		BOOL	bReboot = FALSE;
		MEMORYSTATUS memStatus;
		memStatus.dwLength = sizeof(MEMORYSTATUS);
		GlobalMemoryStatus(&memStatus);

		// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Optioin
		if( m_tNextReboot < COleDateTime::GetCurrentTime() )
		{
			bReboot = TRUE;
		}
		else if ((memStatus.dwMemoryLoad >= 85) || (m_pTranCmn->m_nTransactionCount >= 1000))
		{
			// Reboot between 4 and 4:30 if the memory usage is greater than 85%
			if ((GetTime() > "040000") && (GetTime() < "043000"))
			{
				NHERROR((L"Rebooting. Memory usage is %d\r\n", memStatus.dwMemoryLoad));
				bReboot = TRUE;
			}
			// Reboot instantly if the memory usage is greater than 90%, to avoid screen crash
			else if (memStatus.dwMemoryLoad >= 90)
			{
				NHERROR((L"Memory Usage: %d of %d\r\n", memStatus.dwAvailVirtual, memStatus.dwTotalVirtual));
				NHERROR((L"Rebooting NOW! Memory usage is %d\r\n", memStatus.dwMemoryLoad));
				bReboot = TRUE;
			}
		}
		// End of [#2558]

		if (bReboot == TRUE)
		{
			Main_SetAtmStatus(ATM_REBOOT);	// [#93] KSK 2008.04.08
			return T_EXIT;
		}
	}

	// end of [#45]

	// [#171] [NH] KSK 2008.04.28 HealthCheck Interval Bug Fix
	// Health Check and Interval Send 위치 변경
	if (m_pDevCmn->HostConfigCmd)
	{
		//RETAILMSG(1, (L"HOST  Config Command [%d]\n", m_pDevCmn->HostConfigCmd));

		// HOST에서 DETAIL HEALTH CHECK 송신하라는 명령이 왔을 경우
		if (m_pDevCmn->HostConfigCmd == TC_DETAILSTATUS)				// STANDARD2에만 해당됨
		{
			m_pDevCmn->Disable_CardReader_RFID();		// [#2449] US Justin 

			m_pTranCmn->BIZ_HealthCheckProc(TRUE);
			m_pDevCmn->HostConfigCmd = 0;								// [#2176] NH KSK 2013.01.25
		}
		else			// [#144] KSK 2008.04.20
		if (m_pDevCmn->HostConfigCmd == TC_RECONFIGURATION)				// STANDARD1,2에만 해당됨
		{
			// [#145] KSK 2008.04.20 MAC/CRC 장애시 Configuration 처리로 인해
			// fnAPP_SetConfigRequestInitiator() FLAG SET 부분을 NMS로 이동
			m_pDevCmn->HostOpenFlag = FALSE;				
			m_pDevCmn->HostOpenRetryTime = 0;					// Clear
			// end of [#145]

			m_pDevCmn->Disable_CardReader_RFID();		// [#2449] US Justin 

			ConnectHost();

			//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CONFIGURATION_ENABLE) == 1)
			{
				// Option이 Disable인 경우 Success로 변수 설정
				m_pDevCmn->HostOpenFlag = TRUE;
				m_pDevCmn->HostOpenRetryTime = 0;
				m_pDevCmn->HostConfigCmd = 0;	// [#2176] NH KSK 2013.01.25
			}
			
			if (m_pDevCmn->HostOpenFlag == FALSE)	// Open Fail시에 ATM_READY로 Mode 전환
			{
				m_pDevCmn->HostConfigCmd = 0;	// [#431] [NH] KSK 2008.10.02
				Main_SetAtmStatus(ATM_READY);
				return T_EXIT;
			}
			// end of [#415]
		}			// end of [#144]
		else							// [#2075] NH KSK 2011.06.27
		if (m_pDevCmn->HostConfigCmd == TC_EXTENDED_RKT_TMK || m_pDevCmn->HostConfigCmd == TC_EXTENDED_RKT_MMK)
		{
			if (m_pDevCmn->HostOpenFlag == FALSE)	// 개국 이전에는 Command 무시
			{
				m_pDevCmn->HostConfigCmd = 0;
				return T_OK;
			}

			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_ENABLE) == 0 && MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_PROTOCOL) != RKT_PROTOCOL_BASIC)
			{
				m_pDevCmn->HostConfigCmd = 0;
				return T_OK;
			}

			m_pDevCmn->Disable_CardReader_RFID();		// [#2449] US Justin 

			if (m_pTranCmn->P_BIZ_RKT_Proc(m_pDevCmn->HostConfigCmd) == RES_OK)
			{
				// Success 처리
				// RKT TMK SUCCESS
				NHDEBUG(1, (_T("***TranMainProc***CTranCmn::fnAPP_TranRKTProc() TMK SUCCESS\n")));

				// KCV값을 Update해야한다.
				if (m_pDevCmn->HostConfigCmd == TC_EXTENDED_RKT_TMK)
					MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0, m_pDevCmn->fnPIN_RKT_GetEPP_KCV());
				else
					MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM15, m_pDevCmn->fnPIN_RKT_GetEPP_KCV());

				Chk_Key.RemoveAll();

				// [#2172] NH KSK 2012.12.27
				for(int i=0; i<= MAX_PIN_KEYINDEX; i++)
					Chk_Key.Add(MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0+i));
				// end of [#2172]

				// RKT SUCCESS시 Configuration을 재 시도하도록 한다.
				m_pDevCmn->HostOpenFlag = FALSE;
				m_pDevCmn->HostOpenRetryTime = 0;					// Clear
				m_pDevCmn->HostConfigCmd = 0;	// [#2176] NH KSK 2013.01.25 Command 처리 완료 후 Configuration 수행

				ConnectHost();

				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CONFIGURATION_ENABLE) == 1)
				{
					// Option이 Disable인 경우 Success로 변수 설정
					m_pDevCmn->HostOpenFlag = TRUE;
					m_pDevCmn->HostOpenRetryTime = 0;
					m_pDevCmn->HostConfigCmd = 0;								// [#2176] NH KSK 2013.01.25
				}
				else if (m_pDevCmn->HostOpenFlag == FALSE)	// Open Fail시에 ATM_READY로 Mode 전환
				{
					m_pDevCmn->HostConfigCmd = 0;	// [#431] [NH] KSK 2008.10.02
					Main_SetAtmStatus(ATM_READY);
					return T_EXIT;
				}
			}
			else
			{
				m_pDevCmn->HostConfigCmd = 0;								// [#2176] NH KSK 2013.01.25
			}
		}								// end of [#2075]
		else	// [#560] NH KSK 2009.8.20
		if (m_pDevCmn->HostConfigCmd == TC_EXTENDED_AID_UPDATE)
		{
			if (m_pDevCmn->HostOpenFlag == FALSE)	// 개국 이전에는 Command 무시
			{
				m_pDevCmn->HostConfigCmd = 0;
				return T_OK;
			}

			m_pDevCmn->Disable_CardReader_RFID();		// [#2449] US Justin 

			// [#2081] NH KSK 2011.06.27 EMV AID Update 함수화
			if (m_pTranCmn->BIZ_EMV_UpdateAIDListProc() == RES_OK)
			{
				// Journal Save
				m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, L"SUCCESS AID LIST UPDATE");
			}
			else
			{
				m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, L"FAIL AID LIST UPDATE");
			}
			// end of [#2081]

			m_pDevCmn->HostConfigCmd = 0;								// [#2176] NH KSK 2013.01.25
		}	// end of [#560]
		else
		if (m_pDevCmn->HostConfigCmd == TC_EXTENDED_EJUPLOAD)	// [#2076] NH KSK 2011.06.28
		{
			if (m_pDevCmn->HostOpenFlag == FALSE)	// 개국 이전에는 Command 무시
			{
				m_pDevCmn->HostConfigCmd = 0;
				return T_OK;
			}

			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EJUPLOAD_ENABLE) == 0)
			{
				m_pDevCmn->HostConfigCmd = 0;
				return T_OK;
			}

			m_pDevCmn->Disable_CardReader_RFID();		// [#2449] US Justin 

			// Intialize variable
			m_pTranCmn->m_nUploadLastJnlCount = 0;

			while(TRUE)
			{
				// HOST에서 EJ Upload Flag Off시 종료 처리
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EJUPLOAD_ENABLE) == 0)
					break;

				// Journal Upload할 Data가 없을 경우 종료 처리
				if (m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_HOST) <= 0)
					break;

				// 통신 장애 발생한 경우 종료 처리
				if (m_pTranCmn->BIZ_HostConfigExtendedProc(TRAN_EXTENDED_EJUPLOAD, TC_EXTENDED_EJUPLOAD) == FALSE)
				{
					if (m_pTranCmn->m_nUploadLastJnlCount > 0)	// Upload한 Journal이 존재하나 송신 Fail이므로 이전 Index로 설정한다.
						m_pDevCmn->m_JNLMgr.SetUploadedIndex(UP_HOST, m_pTranCmn->m_nUploadLastJnlCount);					

					break;
				}
			}

			m_pDevCmn->HostConfigCmd = 0;								// [#2176] NH KSK 2013.01.25
		}	// end of [#2076]
		// [#2449] US Justin 2016.11.08				
		#if (APP_PAYDIANT_CCA)
		else if (m_pDevCmn->HostConfigCmd == TC_EXTENDED_ADDSVC_CONFIG)	
		{
			if (m_pDevCmn->HostOpenFlag == FALSE)
			{
				m_pDevCmn->HostConfigCmd = 0;
				return T_OK;
			}

			m_pDevCmn->Disable_CardReader_RFID();

			m_pTranCmn->BIZ_HostConfigExtendedProc(TRAN_EXTENDED_ADDSVC_CONFIG, TC_EXTENDED_ADDSVC_CONFIG);

		m_pDevCmn->HostConfigCmd = 0;
		}	
		#endif			// end of [#2449]
		else if (m_pDevCmn->HostConfigCmd == TC_TR34_LOAD)
		{
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_ENABLE) == 0 || MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_PROTOCOL) != RKT_PROTOCOL_TR34)
			{
				m_pDevCmn->HostConfigCmd = 0;
				return T_OK;
			}

			m_pDevCmn->Disable_CardReader_RFID();		// [#2449] US Justin 

			if (m_pTranCmn->P_BIZ_TR34_Transport_Proc(TRUE) == RES_OK)
			{
				// Success 처리
				// RKT TMK SUCCESS
				NHDEBUG(1, (_T("***TranMainProc***CTranCmn::P_BIZ_TR34_Proc() SUCCESS\n")));

				Chk_Key.RemoveAll();

				// [#2172] NH KSK 2012.12.27
				for(int i=0; i<= MAX_PIN_KEYINDEX; i++)
					Chk_Key.Add(MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0+i));
				// end of [#2172]

				// RKT SUCCESS시 Configuration을 재 시도하도록 한다.
				m_pDevCmn->HostOpenFlag = FALSE;
				m_pDevCmn->HostOpenRetryTime = 0;					// Clear
				m_pDevCmn->HostConfigCmd = 0;	// [#2176] NH KSK 2013.01.25 Command 처리 완료 후 Configuration 수행

				ConnectHost();

				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CONFIGURATION_ENABLE) == 1)
				{
					// Option이 Disable인 경우 Success로 변수 설정
					m_pDevCmn->HostOpenFlag = TRUE;
					m_pDevCmn->HostOpenRetryTime = 0;
					m_pDevCmn->HostConfigCmd = 0;								// [#2176] NH KSK 2013.01.25
				}
				else if (m_pDevCmn->HostOpenFlag == FALSE)	// Open Fail시에 ATM_READY로 Mode 전환
				{
					m_pDevCmn->HostConfigCmd = 0;	// [#431] [NH] KSK 2008.10.02
					Main_SetAtmStatus(ATM_READY);
					return T_EXIT;
				}
			}
			else
			{
				m_pDevCmn->HostConfigCmd = 0;								// [#2176] NH KSK 2013.01.25
			}
		}
		else if (m_pDevCmn->HostConfigCmd == TC_TR34_UNBIND)
		{
			if (m_pDevCmn->HostOpenFlag == FALSE)
			{
				m_pDevCmn->HostConfigCmd = 0;
				return T_OK;
			}

			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_ENABLE) == 0 || MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_PROTOCOL) != RKT_PROTOCOL_TR34)
			{
				m_pDevCmn->HostConfigCmd = 0;
				return T_OK;
			}

			if (m_pTranCmn->P_BIZ_TR34_Unbind_Proc(TRUE) == RES_OK)
			{
				NHDEBUG(1, (_T("TR34 Unbind command successful\n")));

				// This is pretty significant, so we should reboot the ATM
				m_pDevCmn->HostConfigCmd = 0;
				Main_SetAtmStatus(ATM_REBOOT);
				return T_EXIT;
			}
			else 
			{
				m_pDevCmn->HostConfigCmd = 0;
			}
		}
		else
		{
			// Not Defined Command 수신 시 Command 초기화 처리
			m_pDevCmn->HostConfigCmd = 0;								// [#2176] NH KSK 2013.01.25
		}

//		m_pDevCmn->HostConfigCmd = 0;	[#2176] NH KSK 2013.01.25
		return T_OK;
	}

	/////////////////////////////////////////////////////////////////////////////
	//	HEALTH CHECK INTERVAL Procedure
	/////////////////////////////////////////////////////////////////////////////
	if (((int)m_pDevCmn->fnAPL_GetHealthGapTime() >= (m_pDevCmn->m_DelayTime)))
	{
		// Interval로 인한 HEALTH CHECK 송신하는 경우 [HOST]
		// KSK 2009.12.17 Health Check Bug Fix
		// 기존 WINCE 사양 : ATM_CUSTOM에서만 Inverval 주기로 송신함.
		// 변경 WINCE 사양 : ATM_CLERK이 아닌 경우 Interval 주기로 송신함. (XP와 통일)
		// WINXP 사양 : ATM_CLERK이 아닌 경우 Interval 주기로 송신함.
		// POS 사양 : ATM_CUSTOM인 경우 Interval 주기로 송신함.
		//			  ERROR 모드에서는 정시간(Interval 주기와 상관없이) 주기로 송신함.
		if ((m_pDevCmn->HostOpenFlag) &&
			((m_pDevCmn->AtmStatus == ATM_CUSTOM) || (m_pDevCmn->AtmStatus == ATM_ERROR) || (m_pDevCmn->AtmStatus == ATM_READY)))
		{
			if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_HEALTHYFLAG))
			{
				m_pDevCmn->Disable_CardReader_RFID();		// [#2492] US Justin 

				m_pTranCmn->BIZ_HealthCheckProc(TRUE);
				m_pDevCmn->fnAPL_SetHealthCheckTimer();					// [#583] NH KSK 2009.11.24 Sec로 변경됨에 따라 Refresh Time 설정 위치 수정 (송신 후)

				// [#2002] NH KSK 2010.10.30 ATM_CUSTOM 모드에서Health Check 중 통신 장애 시 장애를 Clear하도록 함 (장애가 없는 상태임으로 판단함)
				if (m_pDevCmn->AtmStatus == ATM_CUSTOM)
					m_pDevCmn->fnAPL_ClearError(CLEAR_ERROR_NETWORK);	// [#2492] NH Justin 2017.06.30 "OOS WO ERROR Code" bug fix / Remove Network Error ONLY
				// end of [#2002]

				Main_SetAtmStatus(ATM_READY);							// [#93] KSK 2008.04.08
				return T_EXIT;
			}
		}
	}
	// end of [#171]

	/////////////////////////////////////////////////////////////////////////////
	//	AUTO DAY TOTAL STATUSE Procedure
	//
	//	1. 하루에 한번만 한다.
	//  2. 이전(전날 혹은 몇일전)에 하지 못했으면 오늘 수행한다.
	//     단 오늘 수행할 시간(Auto day total)보다 현재 수행하는 시간이 크면 오늘 것도 수행 한것으로 간주한다.
	//     단 오늘 수행할 시간(Auto day total)보다 현재 수행하는 시간이 작으면 오늘 수행 시간에 다시 수행한다.
	/////////////////////////////////////////////////////////////////////////////
	if ((m_pDevCmn->AtmStatus == ATM_CUSTOM || m_pDevCmn->AtmStatus == ATM_ERROR || m_pDevCmn->AtmStatus == ATM_READY) &&
		(m_pDevCmn->AtmStatus == m_pDevCmn->AtmStatusSave) &&
		(m_pDevCmn->HostOpenFlag == TRUE) &&				// [#461] [NH] KSK 2008.12.03
		(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_ENABLE) == ENABLE))
	{
		COleDateTime	AutoTotalTime, CurTime;
		CurTime = COleDateTime::GetCurrentTime();

		AutoTotalTime.SetDateTime(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_YEAR),		// Year
								  MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_MONTH),	// Month
								  MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_DAY),		// Day
								  MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_HOUR),		// Hour
								  MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_MIN),		// Minute
								  0);	// Second


		if (CurTime >= AutoTotalTime)
		{
			int		nCurTime, nAutoTotalTime;
			//BOOL	bOccurError = FALSE;
			CString	strJnlMsg;
			CString strAutoDayTotal_ErrorCode;
			// make string:2008-10-11 -> int:20081011 or string 12:30 -> int:1230
			nCurTime = ((CurTime.GetHour() * 100) + CurTime.GetMinute());
			nAutoTotalTime = ((AutoTotalTime.GetHour() * 100) + AutoTotalTime.GetMinute());

			//bOccurError = !m_pDevCmn->fnAPL_CheckError();

			// DISABLE PIN
			//m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);

			// DISABLE MCU, FLICKER
			if (m_pDevCmn->AtmStatus == ATM_CUSTOM)
				m_pDevCmn->Disable_CardReader_RFID();		// [#2492] US Justin 
			
			// Update Auto Day Total Time.
			COleDateTime		NextAutoTotalTime;
			NextAutoTotalTime.SetDateTime(CurTime.GetYear(), CurTime.GetMonth(),  CurTime.GetDay(), 0, 0, 0);

			// +1 day
			if (nCurTime >= nAutoTotalTime)
			{
				COleDateTimeSpan	spanOneDay(1, 0, 0, 0);		// 1 day.
				NextAutoTotalTime += spanOneDay;
			}

			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_YEAR,	NextAutoTotalTime.GetYear());		// Year
			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_MONTH,	NextAutoTotalTime.GetMonth());		// Month
			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_DAY,		NextAutoTotalTime.GetDay());		// Day

			if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TYPE) == AUTO_DAY_TOTAL)
			{
				m_pTranCmn->BIZ_DayTotalProc(TRUE, TRUE);
			}
			else
			{
				m_pTranCmn->BIZ_DayTotalProc(TRUE, FALSE);
			}
			// end of [#442]
			
			//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
			if (m_pDevCmn->TranResult == FALSE)
					strAutoDayTotal_ErrorCode = m_pDevCmn->m_pNetWork->GetErrorCode();
			// [#639] NH KSK 2010.05.03 DAY TOTAL MAC ERROR시 CONFIGURATION 수행하도록 수정
			// 현재는 STANDARD3일 경우에만 설정하도록 되어져 있으므로, MESSAGE FORMAT별 분기처리는 하지 않음
			if (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) == REVERSAL_CONF)	// reversal flag가 2이면 configuration만 한다.
			{
				m_pDevCmn->HostOpenFlag = FALSE;
				m_pDevCmn->HostOpenRetryTime = 0;
				
				if (!m_pTranCmn->BIZ_HostConfigProc(FALSE))
				{
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CONFIGURATION_ENABLE) == 1)
					{
						// Option이 Disable인 경우 Success로 변수 설정
						m_pDevCmn->HostOpenFlag = TRUE;
						m_pDevCmn->HostOpenRetryTime = 0;
					}
				}
				// REVERSAL FLAG CLEAR
				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
				
				// DAY TOTAL FAIL이므로 TranResult를 Fail로 설정
				m_pDevCmn->TranResult = FALSE;
			}
			// end of [#639]
			//end of [#2000]

			if (!m_pDevCmn->TranResult)
			{
				// when total failed, JNL log
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TYPE) == AUTO_DAY_TOTAL)
					strJnlMsg.Format(L"Auto Day Total - Failed(%7.7s)", strAutoDayTotal_ErrorCode);
				else
					strJnlMsg.Format(L"Auto Trial Day Total - Failed(%7.7s)", strAutoDayTotal_ErrorCode);

				m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, strJnlMsg);
			}

			// [#2492] NH Justin 2017.06.30 "OOS WO ERROR Code" bug fix / Remove Network Error ONLY
			/*
			// ENABLE MCU, FLICKER
			if (m_pDevCmn->AtmStatus == ATM_CUSTOM)
			{
				m_pDevCmn->fnMCU_CardEnDisable(ENABLE, FALSE);

				// [#2325] NH KSK 2015.01.21 EMV Enable이므로 RFID도 Enable 처리
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
				{
					if (m_pDevCmn->fnRFID_GetDeviceStatus() == NORMAL)
					{
						m_pDevCmn->fnRFID_EntryEnable();
						m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
					}
				}
				// end of [#2325]
			}

			// ENABLE PIN
			m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);

			if (!bOccurError)
			{
				m_pDevCmn->fnAPL_ClearError();
				Main_SetAtmStatus(ATM_READY);
				return T_EXIT;
			}
			*/

			if (m_pDevCmn->AtmStatus == ATM_CUSTOM)
				m_pDevCmn->fnAPL_ClearError(CLEAR_ERROR_NETWORK);

			Main_SetAtmStatus(ATM_READY);
			return T_EXIT;
			// End of [#2492] 
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	//	RMS INTERVAL STATUSE Procedure
	/////////////////////////////////////////////////////////////////////////////
	if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSFLAG))
	{
		// Interval로 인한 STATUS를 송신하는 경우 [RMS]
		if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSSTATUSSEND))
		{
			COleDateTime		CurTime;
			COleDateTime		NextRmsSendTime;
			COleDateTimeSpan	SendInterval;
			int					nInterval = 0;

			// Get Current Time
			CurTime = COleDateTime::GetCurrentTime();

			// Get RMS Send Interval Value (1-24 Hour)
			nInterval = __max(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSSENDINTERVAL), 1);
			nInterval = __min(nInterval, 24);

			// Get TimeSpan
			NextRmsSendTime = m_RMSSendTime + COleDateTimeSpan(0, nInterval, 0, 0);

			// [#2559] NH Justin 2018.06.18 Retry Status Send if "send" failed
			BOOL bSendFailRetry = FALSE;
			#if( US_VERSION || CA_VERSION || MX_VERSION )
			if( (GetCurrnetRMSLineType() == RMS_LINE_TCP) && (m_bLastRMSSendResult == FALSE) )
			{
				int nReSendRetrial = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSSEND_RETRY); 
				if(nReSendRetrial>0)
				{
					COleDateTime tRMSReTial = m_RMSSendTime + COleDateTimeSpan(0, 0, nReSendRetrial, 0);
					if( tRMSReTial <= CurTime )
					{
						NHDEBUG(1, (_T("===== RMS Send Retry. INTERVAL = [%d] Minutes \n"), nReSendRetrial));
						bSendFailRetry = TRUE;
					}
				}
			}
			#endif
			// End of [#2559]
			
			// Check Send Time
			// 1. 초기 부팅 시, ERROR or CUSTOM
			// 2. Interval이 지난 다음.
			// [#2113] NH KSK 2012.01.03 Interval이 된 경우이거나, 최초 P/G 구동시 송신하는 사양
			// [#2559] NH Justin 2018.06.18 Retry Status Send if "send" failed, PARAMETER(bSendFailRetry) ADDED
			if (NextRmsSendTime <= CurTime || m_bRMSStatusInitSend == FALSE || bSendFailRetry == TRUE)
			{
				m_bRMSStatusInitSend = TRUE;	// [#2113] NH KSK 2012.01.03 초기 수행후에는 항상 TRUE임

				NHDEBUG(1, (_T("GO~GO~GO~ Status Send -- INTERVAL\n")));

				// [#2492] JUSTIN
				if (m_pDevCmn->AtmStatus == ATM_CUSTOM)
					m_pDevCmn->Disable_CardReader_RFID();
				// End of [#2492] 

				ProcRMSCtrl(RMS_CONNECT);

				// [#2559] NH Justin 2018.06.18 Reboot - Setup download/reboot command on Status Send.
				// Main_SetAtmStatus(ATM_READY);		// 화면 갱신 안되는 문제가 있어 READY에서 다시 시작
				if( MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND) == RMSCMD_REBOOT)
				{
					NHDEBUG(DBG_CALL, (L"RMS REBOOT COMMAND - Download Setup or similar situations\n"));
					Main_SetAtmStatus(ATM_REBOOT);	
				}
				else
					Main_SetAtmStatus(ATM_READY);		// 화면 갱신 안되는 문제가 있어 READY에서 다시 시작
				// End of [#2559]

				return T_EXIT;
			}

			// [#613] AU_C KSK 2010.01.18 Scheduled Journal Upload 기능 추가
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_ENABLE) == ENABLE)
			{
				BOOL bUploadJournal = FALSE;		// [#2492] NH Justin 2017.06.30 "OOS WO ERROR Code" bug fix

				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_OPTION) == 0)		// Count Option일 경우
				{
					// case 1 : Journal Count : 1, Remain Count : 1
					int nUploadJounalCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_COUNT);
					int nJournalRemainCount = m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS);

					if ( nUploadJounalCount <= nJournalRemainCount)
					{
						// Check Journal Remain Count
						if ((nLastUploadedJNLCount + 1 <= nJournalRemainCount) || (nLastUploadedJNLCount == 0))
						{
							//BOOL bOccurError = !m_pDevCmn->fnAPL_CheckError();
							
							// DISABLE PIN
							//m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
							
							// DISABLE MCU, FLICKER
							if (m_pDevCmn->AtmStatus == ATM_CUSTOM)
								m_pDevCmn->Disable_CardReader_RFID();		// [#2492] US Justin 
 												
							// Journal Upload를 수행한다.
							ProcRMSCtrl(RMS_CONNECT, RMS_REQ_JNL);
							
							if (nUploadJounalCount > m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS))
							{
								// Journal Upload가 성공한 경우 or Count보다 작게 남아 있을 경우
								nLastUploadedJNLCount = 0;
							}
							else
							{
								// Journal Upload가 실패한 경우 현재 Journal Count를 save함.
								nLastUploadedJNLCount = m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS);
							}

							// [#2492] NH Justin 2017.06.30 "OOS WO ERROR Code" bug fix / Remove Network Error ONLY
							bUploadJournal = TRUE;		// Handle at the bottom
							/*
							// ENABLE MCU, FLICKER
							if (m_pDevCmn->AtmStatus == ATM_CUSTOM)
							{
								m_pDevCmn->fnMCU_CardEnDisable(ENABLE, FALSE);

								// [#2325] NH KSK 2015.01.21 EMV Enable이므로 RFID도 Enable 처리
								if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
								{
									if (m_pDevCmn->fnRFID_GetDeviceStatus() == NORMAL)
									{
										m_pDevCmn->fnRFID_EntryEnable();
										m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
									}
								}
								// end of [#2325]
							}
							
							// ENABLE PIN
							m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
							
//							if (!bOccurError)
							{
								m_pDevCmn->fnAPL_ClearError();
								Main_SetAtmStatus(ATM_READY);
								return T_EXIT;
							}
							*/
							// End of [#2492]
						}
					}
				}
				else if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_OPTION) == 1)		// Monthly Optin일 경우
				{
					COleDateTime	ScheduledJNLUploadTime, CurTime;
					CurTime = COleDateTime::GetCurrentTime();
					
					ScheduledJNLUploadTime.SetDateTime( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_YEAR),	// Year
														MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_MONTH),	// Month
														MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_NEXT_DAY),		// Next Day 2010.02.01
														MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_HOUR),	// Hour
														0,																				// Minute
														0);																				// Second
					
					
					if (CurTime >= ScheduledJNLUploadTime)
					{
						int		nCurTime, nScheduledJNLUploadTime;
						//BOOL	bOccurError = FALSE;
						CString	strJnlMsg;	

						// make string:2008-10-11 -> int:20081011 or string 12:30 -> int:1230
						nCurTime = ((CurTime.GetHour() * 100) + CurTime.GetMinute());
						nScheduledJNLUploadTime = ((ScheduledJNLUploadTime.GetHour() * 100) + ScheduledJNLUploadTime.GetMinute());
											
						// +1 Month
						int nNextYear  = CurTime.GetYear();
						int nNextMonth = CurTime.GetMonth() + 1;
						int nNextDay   = CurTime.GetDay();

						if (nNextMonth > 12)
						{
							// 12월을 넘어 가면 1년을 더한다 (2999년이 지나면 어떡하지?)
							nNextYear++;
							nNextMonth = 1;
						}

						while(!IsValidDateTime(nNextMonth, nNextDay, nNextYear))
						{
							nNextDay--;
							if (nNextDay <= 0)
								nNextDay = 1;
						}

						NHDEBUG(1, (_T("SHEDULED JOURNAL UPLOAD -> Current Month [%d/%d/%d] / Next Month [%d/%d/%d]\n"), 
										CurTime.GetYear(), CurTime.GetMonth(),  CurTime.GetDay(),
										nNextYear, nNextMonth, nNextDay));
						
						MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_YEAR,		nNextYear);		// Year
						MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_MONTH,	nNextMonth);	// Month
						MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_NEXT_DAY, nNextDay);		// Next Day 2010.02.01

						if (m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS) > 0)	// Upload할 Journal이 없을 경우는 skip한다.
						{
							//bOccurError = !m_pDevCmn->fnAPL_CheckError();
							
							// DISABLE PIN
							//m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
							
							// DISABLE MCU, FLICKER
							if (m_pDevCmn->AtmStatus == ATM_CUSTOM)
								m_pDevCmn->Disable_CardReader_RFID();		// [#2492] US Justin 
							
							// Journal Upload를 수행한다.
							ProcRMSCtrl(RMS_CONNECT, RMS_REQ_JNL);

							// [#2492] NH Justin 2017.06.30 "OOS WO ERROR Code" bug fix / Remove Network Error ONLY
							bUploadJournal = TRUE;		// Handle at the bottom
							/*						
							// ENABLE MCU, FLICKER
							if (m_pDevCmn->AtmStatus == ATM_CUSTOM)
							{
								m_pDevCmn->fnMCU_CardEnDisable(ENABLE, FALSE);

								// [#2325] NH KSK 2015.01.21 EMV Enable이므로 RFID도 Enable 처리
								if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
								{
									if (m_pDevCmn->fnRFID_GetDeviceStatus() == NORMAL)
									{
										m_pDevCmn->fnRFID_EntryEnable();
										m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
									}
								}
								// end of [#2325]
							}
							
							// ENABLE PIN
							m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
							
//							if (!bOccurError)
							{
								m_pDevCmn->fnAPL_ClearError();
								Main_SetAtmStatus(ATM_READY);
								return T_EXIT;
							}
							*/
							// End of [#2492]
						}
					}
				}
				else if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_OPTION) == 2)		// Daily Optin일 경우
				{
					COleDateTime	ScheduledJNLUploadTime, CurTime;
					CurTime = COleDateTime::GetCurrentTime();
					
					ScheduledJNLUploadTime.SetDateTime( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_YEAR),	// Year
														MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_MONTH),	// Month
														MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_NEXT_DAY),	// Next Day 2010.02.01
														MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_HOUR),	// Hour
														0,																				// Minute
														0);																				// Second
					
					
					if (CurTime >= ScheduledJNLUploadTime)
					{
						int		nCurTime, nScheduledJNLUploadTime;
						//BOOL	bOccurError = FALSE;
						CString	strJnlMsg;
						
						// make string:2008-10-11 -> int:20081011 or string 12:30 -> int:1230
						nCurTime = ((CurTime.GetHour() * 100) + CurTime.GetMinute());
						nScheduledJNLUploadTime = ((ScheduledJNLUploadTime.GetHour() * 100) + ScheduledJNLUploadTime.GetMinute());
										
						// Update Journal Upload Time.
						COleDateTime		NextScheduledJNLUploadTime;
						NextScheduledJNLUploadTime.SetDateTime(CurTime.GetYear(), CurTime.GetMonth(),  CurTime.GetDay(), 0, 0, 0);
						
						// +1 day
						if (nCurTime >= nScheduledJNLUploadTime)
						{
							COleDateTimeSpan	spanOneDay(1, 0, 0, 0);		// 1 day.
							NextScheduledJNLUploadTime += spanOneDay;
						}

						NHDEBUG(1, (_T("SHEDULED JOURNAL UPLOAD -> Current Month [%d/%d/%d] / Next Month [%d/%d/%d]\n"), 
											CurTime.GetYear(), CurTime.GetMonth(),  CurTime.GetDay(),
											NextScheduledJNLUploadTime.GetYear(), NextScheduledJNLUploadTime.GetMonth(), NextScheduledJNLUploadTime.GetDay()));
						
						MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_YEAR,		NextScheduledJNLUploadTime.GetYear());		// Year
						MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_MONTH,	NextScheduledJNLUploadTime.GetMonth());		// Month
						MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_NEXT_DAY,	NextScheduledJNLUploadTime.GetDay());		// Next Day 2010.02.01

						if (m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS) > 0)	// Upload할 Journal이 없을 경우는 skip한다.
						{
							//bOccurError = !m_pDevCmn->fnAPL_CheckError();
							
							// DISABLE PIN
							//m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
							
							// DISABLE MCU, FLICKER
							if (m_pDevCmn->AtmStatus == ATM_CUSTOM)
								m_pDevCmn->Disable_CardReader_RFID();		// [#2492] US Justin 
							
							// Journal Upload를 수행한다.
							ProcRMSCtrl(RMS_CONNECT, RMS_REQ_JNL);
							
							// [#2492] NH Justin 2017.06.30 "OOS WO ERROR Code" bug fix / Remove Network Error ONLY
							bUploadJournal = TRUE;		// Handle at the bottom
							/*						
							// ENABLE MCU, FLICKER
							if (m_pDevCmn->AtmStatus == ATM_CUSTOM)
							{
								m_pDevCmn->fnMCU_CardEnDisable(ENABLE, FALSE);

								// [#2325] NH KSK 2015.01.21 EMV Enable이므로 RFID도 Enable 처리
								if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
								{
									if (m_pDevCmn->fnRFID_GetDeviceStatus() == NORMAL)
									{
										m_pDevCmn->fnRFID_EntryEnable();
										m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
									}
								}
								// end of [#2325]
							}
							
							// ENABLE PIN
							m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
							
//							if (!bOccurError)
							{
								m_pDevCmn->fnAPL_ClearError();
								Main_SetAtmStatus(ATM_READY);
								return T_EXIT;
							}
							*/
							// End of [#2492]
						}
					}
				}

				// [#2492] NH Justin 2017.06.30 "OOS WO ERROR Code" bug fix / Remove Network Error ONLY
				if(bUploadJournal)
				{
					// m_pDevCmn->fnAPL_ClearError();	// DO NOT CLEAR ERROR.  +++ RMS CONNECTION DO NOT LEAVE NETWORK ERROR +++
					Main_SetAtmStatus(ATM_READY);
					return T_EXIT;
				}
				// End of [#2492]
			}
		}
	}

	// [#2574] US Justin GivePay Enhancement3
	#if(APP_GPAY_GIFTCARD_PURCHASE)
	if( (m_pTranCmn->m_GivePayData.m_bGivePayAvailable==TRUE) && (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_ENABLE_DISABLE)==ENABLE) &&	// GivePay is enabled
		(m_pDevCmn->AtmStatus == ATM_CUSTOM || m_pDevCmn->AtmStatus == ATM_ERROR || m_pDevCmn->AtmStatus == ATM_READY) &&									// In, Ready, or Out of Service
		(m_pDevCmn->AtmStatus == m_pDevCmn->AtmStatusSave) && (m_pDevCmn->HostOpenFlag == TRUE) )
	{
		if( m_pTranCmn->m_GivePayData.NeedDailyUpdate() )
		{
			NHDEBUG(1, (_T("GivePay Daily Downloading .....\n"))); 
			
			// DISABLE MCU, FLICKER
			if (m_pDevCmn->AtmStatus == ATM_CUSTOM)
				m_pDevCmn->Disable_CardReader_RFID();	
			
			m_pTranCmn->P_NH_GivePay_DownloadData(GPAY_DOWNLOAD_ALL);

			// [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6
			// TODO add NG here
			//if (m_pTranCmn->P_NH_GivePay_DownloadData(GPAY_DOWNLOAD_ALL) == RES_OK)
			//m_pTranCmn->P_NH_GivePay_DownloadData(GPAY_DOWNLOAD_CMN_IMAGE);
				//MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_ENROLLED, TRUE);
			//else 
				//MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_ENROLLED, FALSE);
			// end of [#RWC6-1, #2583]

			if (m_pDevCmn->AtmStatus == ATM_CUSTOM)
				m_pDevCmn->fnAPL_ClearError(CLEAR_ERROR_NETWORK);

			Main_SetAtmStatus(ATM_READY);
			return T_EXIT;
		}
	}
	#endif
	// End of [#2574]

	// [#122] NH AIREAT 2008.04.22 RMS Wait
	if (m_pDevCmn->AtmStatus == m_pDevCmn->AtmStatusSave)
	{		
		/////////////////////////////////////////////////////////////////////////////
		//	RMS CONNECT Procedure
		/////////////////////////////////////////////////////////////////////////////
		
		if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSFLAG))
		{
			// if enabled RMS, it work Server Mode
			if (m_pDevCmn->fnNET_RMSConnectOpen(RMS_LISTEN) == RMS_OK)
			{
				if (m_pDevCmn->fbNET_RMSConnectCheck())
				{
					NHDEBUG(1, (_T("GO~GO~GO~ RMS PROCEDURE\n")));
					Main_SetAtmStatus(ATM_RMSACTIVE);	// [#93] KSK 2008.04.08
					return T_EXIT;
				}

				return T_OK;
			}
		}

		m_pDevCmn->fnNET_RMSConnectClose();
		
		return T_OK;
	}
	else
		m_pDevCmn->fnNET_RMSConnectClose();

	// end of [#122]


	/////////////////////////////////////////////////////////////////////////////
	//	HEALTH CHECK (HOST/RMS) Procedure
	/////////////////////////////////////////////////////////////////////////////

	if (m_pDevCmn->AtmStatus == ATM_CUSTOM && m_pDevCmn->AtmStatusSave != ATM_TRAN)
	{
		if (bSendErrorAfterRecover == TRUE)		// [#115] KSK 2008.04.16
		{
			// ERROR에서 복구되어 송신하는 경우
			if (m_pDevCmn->HostOpenFlag && MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_HEALTHYFLAG))
			{
				m_pDevCmn->Disable_CardReader_RFID();		// [#2492] US Justin 

				m_pTranCmn->BIZ_HealthCheckProc(TRUE);
				m_pDevCmn->fnAPL_SetHealthCheckTimer();					// [#583] NH KSK 2009.11.24 Sec로 변경됨에 따라 Refresh Time 설정 위치 수정 (송신 후)
			}

			// RMS STATUS SEND
			if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSSTATUSSEND) && m_pDevCmn->HostOpenFlag)			// [#122] NH AIREAT 2008.04.22 FLAG 변경
			{
				NHDEBUG(1, (_T("GO~GO~GO~ Status Send - RELEASE\n")));

				m_pDevCmn->Disable_CardReader_RFID();		// [#2492] US Justin 

				ProcRMSCtrl(RMS_CONNECT);
			}
		}
		
		bSendErrorAfterRecover = FALSE;		// [#115] KSK 2008.04.16
	}
	else
	if (m_pDevCmn->AtmStatus == ATM_ERROR)
	{
		if (!m_pDevCmn->fnAPL_CheckError())			// [#113] KSK 2008.04.15 ERRORMODE시 RMS / HEALTH CHECK 송신 위치 변경
		{
			if (bSendErrorAfterRecover == FALSE)	// [#115] KSK 2008.04.16
			{
				// 통신장애시("Dxxxxxx")에는 ERROR MODE로 오지 않으나, REVERSAL FAIL시에는 오므로 무조건 보낸다.
				// ERROR가 발생하여 송신하는 경우 (Configuration Fail시에는 송신 안함)
				if (m_pDevCmn->HostOpenFlag	&& MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_HEALTHYFLAG))
				{
					m_pDevCmn->Disable_CardReader_RFID();		// [#2492] US Justin 

					m_pTranCmn->BIZ_HealthCheckProc(TRUE);
					m_pDevCmn->fnAPL_SetHealthCheckTimer();					// [#583] NH KSK 2009.11.24 Sec로 변경됨에 따라 Refresh Time 설정 위치 수정 (송신 후)
				}

				// RMS STATUS SEND
				if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSSTATUSSEND) && m_pDevCmn->HostOpenFlag)			// [#122] NH AIREAT 2008.04.22
				{
					NHDEBUG(1, (_T("GO~GO~GO~ Status Send - OCCUR\n")));

					m_pDevCmn->Disable_CardReader_RFID();		// [#2492] US Justin 

					ProcRMSCtrl(RMS_CONNECT);
				}
				bSendErrorAfterRecover = TRUE;		// [#115] KSK 2008.04.16
			}
		}
		// end of [#70]
	}

	return T_OK;
}

// [#93] KSK 2008.04.08 Create Atm Mode Status Set Function
void CMainFrame::Main_SetAtmStatus(int nStatus)
{
	if ((nStatus < 0) || (nStatus > (MODE_COUNT-1)))	return;

	if (m_pDevCmn->AtmStatus != nStatus)
	{
		m_pDevCmn->AtmStatus = nStatus;
		MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSTATUS, m_pDevCmn->AtmStatus);
		NHDEBUG(1, (_T("Main_SetAtmStatus -> [%S]\n"), GETSTR_ATMMODE(nStatus)));
	}

	if (m_pDevCmn->AtmStatus != m_pDevCmn->AtmStatusSave)
	{
		NVDump('O', 'A', "01", L"", CString(GETSTR_ATMMODE(nStatus)));
		NVDump('O', 'A', "01", L"", L"MEM_STATUS");
	}
}
// end of [#93]

// [#253] NZ AIREAT 2008.06.01
/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: NVRAMBackUp()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : NVRAM의 AP영역을 Backup 한다.
-------------------------------------------------------------------*/
BOOL CMainFrame::NVRAMBackUp()
{
	NHDEBUG(DBG_INFO, (_T("NVRAMBackUp()\n")));

	FILE					*Stream = NULL;
	CString					strSaveFileRoot, strSaveFileName;
	CString					strTerminalID;
	int						nNumWritten = 0, i;
	LPVOID					pNvramArea = NULL;
	BOOL					bContinue = FALSE, bSuccess = FALSE;

	NVRAM_BACKUP_HEADER		NvramHeader;
	NVRAM_BACKUP_RECORD		NvramRecord;

	// get TerminalID
	strTerminalID = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTerminalID.TrimLeft();
	strTerminalID.TrimRight();

	// Create \\USB\\TerminalID
	strSaveFileRoot.Format(L"%s\\%s", LOGBACKUP_DEST_PATH, strTerminalID);
	CreateDirectory(strSaveFileRoot, NULL);

	// Create \\USB\\TerminalID\\NVRAM
	strSaveFileRoot += L"\\NVRAM";
	CreateDirectory(strSaveFileRoot, NULL);

	// Open File.
	strSaveFileName.Format(L"%s\\NvramBackup.dat", strSaveFileRoot);

	if ((Stream = _wfopen(strSaveFileName, L"w+b")) != NULL)
	{
		memset(&NvramHeader, 0, sizeof(NvramHeader));

		// [#2046] AU KJW 2011.04.19
#if !(AU_VERSION)
		NHDEBUG(DBG_INFO, (L"!(AU_VERSION)\n"));
		NvramHeader.bIdentity = FILE_ID;
#else
		NHDEBUG(DBG_INFO, (L"AU_VERSION\n"));
		NvramHeader.bIdentity = FILE_ID_AU_NZ;
#endif
		// end of [#2046]

		// write header
		nNumWritten = fwrite((void*)&NvramHeader, sizeof(char), sizeof(NvramHeader), Stream);
		if (nNumWritten == sizeof(NVRAM_BACKUP_HEADER))
		{
			bContinue = TRUE;
			for (i = 0; (i < COUNT_ACCESS_NVRAM && bContinue); i++)
			{
				pNvramArea = NULL;
				memset(&NvramRecord, 0, sizeof(NvramRecord));
				
				// Get Memroy pointer.
				pNvramArea = GetSystemAddr(gnAccessNvramID[i], 4096);

				if (pNvramArea != NULL)
				{
					NvramRecord.nID = gnAccessNvramID[i];
					NvramRecord.nSize = sizeof(NvramRecord.Data);
					memcpy(&NvramRecord.Data[0], pNvramArea, NvramRecord.nSize);
					
					// Encrypt XOR
					NvramRecord.nTI = m_pRmsCtrl->RMS_EncryptXOR(NvramRecord.Data, NvramRecord.nSize);

					// Calc CRC-16
					NvramRecord.nCrc = crc16((unsigned char*)NvramRecord.Data, NvramRecord.nSize);

					// file write.
					nNumWritten = fwrite((void*)&NvramRecord, sizeof(char), sizeof(NvramRecord), Stream);
					if (nNumWritten != sizeof(NvramRecord))
					{
						bContinue = FALSE;
					}
					
					FreeSystemAddr(pNvramArea);
					NvramHeader.nCountRecode++;
				}
				else
					bContinue = FALSE;
			}

			if (i >= COUNT_ACCESS_NVRAM)
			{
				// rewrite header.
				fseek(Stream, 0L, SEEK_SET);
				nNumWritten = fwrite((void*)&NvramHeader, sizeof(char), sizeof(NvramHeader), Stream);
				if (nNumWritten == sizeof(NvramHeader))
					bSuccess = TRUE;
			}
			NHDEBUG(DBG_INFO, (L"after NVRAM backup bSuccess[%d]\n", bSuccess));

			// [#2046] AU KJW 2011.04.19
#if (AU_VERSION)
			if( bSuccess == TRUE )
			{
				bSuccess = FALSE;

				LPBinBlock pBinBlock = new BinBlock;
				if( pBinBlock )
				{
					memset( pBinBlock, 0x00, sizeof(BinBlock) );
					NHDEBUG(DBG_INFO, (L"after memset( pBinBlock...)\n"));
					m_pDevCmn->m_BINMgr.GetBinBlock( pBinBlock );
					NHDEBUG(DBG_INFO, (L"after m_pDevCmn->m_BINMgr.GetBinBlock()\n"));
					BYTE nBinXOR = m_pRmsCtrl->RMS_EncryptXOR( (BYTE*)pBinBlock, sizeof(BinBlock) );
					NHDEBUG(DBG_INFO, (L"nBinXOR[0x%02x]\n", nBinXOR));
					UINT16 nBinCRC = crc16( (unsigned char*)pBinBlock, sizeof(BinBlock) );
					NHDEBUG(DBG_INFO, (L"nBinCRC[0x%04x]\n", nBinCRC));
					fseek(Stream, 0L, SEEK_END);
					nNumWritten = fwrite((void*)&nBinCRC, sizeof(nBinCRC), 1, Stream);
					NHDEBUG(DBG_INFO, (L"nNumWritten[%d],sizeof(nBinCRC)[%d]\n", nNumWritten, sizeof(nBinCRC)));
					if( nNumWritten == 1)
					{
						nNumWritten = fwrite((void*)&nBinXOR, sizeof(nBinXOR), 1, Stream);
						NHDEBUG(DBG_INFO, (L"nNumWritten[%d],sizeof(nBinXOR)[%d]\n", nNumWritten, sizeof(nBinXOR)));
						if( nNumWritten == 1 )
						{
							nNumWritten = fwrite((void*)pBinBlock, sizeof(BinBlock), 1, Stream);
							NHDEBUG(DBG_INFO, (L"nNumWritten[%d],sizeof(BinBlock)[%d]\n", nNumWritten, sizeof(BinBlock)));
							if( nNumWritten == 1 )
							{
								bSuccess = TRUE;
							}
						}
					}
					delete pBinBlock;
					pBinBlock = NULL;
				}
			}

			NHDEBUG(DBG_INFO, (L"bSuccess[%s]\n", bSuccess?L"TRUE":L"FALSE"));
#endif
			// end of [#2046]
		}//if (nNumWritten == sizeof(NVRAM_BACKUP_HEADER))
		fclose(Stream);
	}

	if (bSuccess)
		return TRUE;

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: NVRAMRestore()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Backup된 NVRAM 영역을 복구한다.
-------------------------------------------------------------------*/
BOOL CMainFrame::NVRAMRestore()
{
	NHDEBUG(DBG_INFO, (L"NVRAMRestore()\n"));

	FILE					*Stream = NULL;
	CString					strSaveFileName;
	CString					strTerminalID;
	int						nNumRead = 0, i;
	LPVOID					pNvramArea = NULL;
	BOOL					bContinue = FALSE, bSuccess = FALSE;
	unsigned int			nCrc;

	NVRAM_BACKUP_HEADER		NvramHeader;
	NVRAM_BACKUP_RECORD		NvramRecord;

	// [#2046] AU KJW 2011.04.19
#if (AU_VERSION)
	LPBinBlock				pBinBlock = new BinBlock;
	BYTE					nBinXOR = 0x00;
#endif
	// end of [#2046]

	// get TerminalID
	strTerminalID = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTerminalID.TrimLeft();
	strTerminalID.TrimRight();

	strSaveFileName.Format(L"%s\\%s\\NVRAM\\NvramBackup.dat", LOGBACKUP_DEST_PATH, strTerminalID);
	if ((Stream = _wfopen(strSaveFileName, L"rb")) != NULL)
	{
		/////////////////////////////////////////////
		// check value
		{
			memset(&NvramHeader, 0, sizeof(NvramHeader));
			
			// 1. Read Header
			nNumRead = fread((void*)&NvramHeader, sizeof(char), sizeof(NvramHeader), Stream);
			if (nNumRead == sizeof(NVRAM_BACKUP_HEADER))
			{
				// 2. check ID
				// [#2046] AU KJW 2011.04.19
#if (AU_VERSION)
				if ( (NvramHeader.bIdentity == FILE_ID) || (NvramHeader.bIdentity == FILE_ID_AU_NZ) )
#else
				if (NvramHeader.bIdentity == FILE_ID)
#endif
				// end of [#2046]
				{
					for (i = 0; i < NvramHeader.nCountRecode; i++)
					{
						memset(&NvramRecord, 0, sizeof(NvramRecord));
						
						// 3. Read Record
						nNumRead = fread((void*)&NvramRecord, sizeof(char), sizeof(NvramRecord), Stream);
						if (nNumRead != sizeof(NvramRecord))
							break;
						
						// 4. check value
						if (NvramRecord.nSize > sizeof(NvramRecord.Data))
							break;
						
						// 5. check CRC
						nCrc = crc16((unsigned char*)NvramRecord.Data, NvramRecord.nSize);
						if (nCrc != (unsigned int)NvramRecord.nCrc)
							break;
					}
					
					// 6. check count of Record
					if (i >= NvramHeader.nCountRecode)
						bContinue = TRUE;

	NHDEBUG(DBG_INFO, (L"after NVRAM bContinue[%d]\n", bContinue));

					// [#2046] AU KJW 2011.04.16
#if (AU_VERSION)
					if( NvramHeader.bIdentity == FILE_ID_AU_NZ )
					{
						if( pBinBlock && bContinue == TRUE )
						{
							bContinue = FALSE;
							UINT16 nBinCRC = 0x00;

							NHDEBUG(DBG_INFO, (L"before fread((void*)&nBinCRC...)\n"));
							if( 1 == fread((void*)&nBinCRC, sizeof(nBinCRC), 1, Stream) )
							{
								NHDEBUG(DBG_INFO, (L"before fread((void*)&nBinXOR...)\n"));
								if( 1 == fread((void*)&nBinXOR, sizeof(nBinXOR), 1, Stream) )
								{
									NHDEBUG(DBG_INFO, (L"before fread((void*)pBinBlock...)\n"));
									if( 1 == fread((void*)pBinBlock, sizeof(BinBlock), 1, Stream) )
									{
										if( nBinCRC == crc16((unsigned char*)pBinBlock, sizeof(BinBlock)) )
											bContinue = TRUE;
									}
								}
							}
						}//if( pBinBlock )

						NHDEBUG(DBG_INFO, (L"after BIN bContinue[%d]\n", bContinue));
					}//if( NvramHeader.bIdentity == FILE_ID_AU_NZ )
#endif
					// end of [#2046]
				}
			}
		}
		
		/////////////////////////////////////////////
		// Load Data
		if (bContinue)
		{
			fseek(Stream, 0L, SEEK_SET);
			memset(&NvramHeader, 0, sizeof(NvramHeader));
			
			// 1. Read Header
			nNumRead = fread((void*)&NvramHeader, sizeof(char), sizeof(NvramHeader), Stream);
			if (nNumRead == sizeof(NVRAM_BACKUP_HEADER))
			{
				// 2. check ID
				// [#2046] AU KJW 2011.04.19
#if (AU_VERSION)
				if ( (NvramHeader.bIdentity == FILE_ID) || (NvramHeader.bIdentity == FILE_ID_AU_NZ) )
#else
				if (NvramHeader.bIdentity == FILE_ID)
#endif
				// end of [#2046]
				{
					for (i = 0; i < NvramHeader.nCountRecode; i++)
					{
						memset(&NvramRecord, 0, sizeof(NvramRecord));
						
						// 3. Read Record
						nNumRead = fread((void*)&NvramRecord, sizeof(char), sizeof(NvramRecord), Stream);
						if (nNumRead != sizeof(NvramRecord))
							break;
						
						// 4. Decrypt XOR
						m_pRmsCtrl->RMS_DecryptXOR(NvramRecord.nTI, NvramRecord.Data, NvramRecord.nSize);

						// Get Memory pointer.
						pNvramArea = GetSystemAddr(NvramRecord.nID, 4096);
						if (pNvramArea == NULL)
							break;

						NvramCopyMemory(pNvramArea, &NvramRecord.Data[0], NvramRecord.nSize);					// [#573] NH AIREAT 2009.10.09 SW NVRAM

						FreeSystemAddr(pNvramArea);
					}
					
					// 5. check count of Record
					if (i >= NvramHeader.nCountRecode)
						bSuccess = TRUE;

						NHDEBUG(DBG_INFO, (L"after NVRAM restore bSuccess[%d]\n", bSuccess));

					// [#2046] AU KJW 2011.04.19
#if (AU_VERSION)
					if( NvramHeader.bIdentity == FILE_ID_AU_NZ )
					{
						NHDEBUG(DBG_INFO, (L"NvramHeader.bIdentity == FILE_ID_AU_NZ\n"));
						if( pBinBlock && bSuccess == TRUE )
						{
							bSuccess = FALSE;
							m_pRmsCtrl->RMS_DecryptXOR(nBinXOR, (BYTE*)pBinBlock, sizeof(BinBlock));
							NHDEBUG(DBG_INFO, (L"after RMS_DecryptXOR(nBinXOR...)\n"));
							if( m_pDevCmn->m_BINMgr.SetBinBlock( pBinBlock ) == TRUE )
								bSuccess = TRUE;
						}
						NHDEBUG(DBG_INFO, (L"after BIN restore bSuccess[%d]\n", bSuccess));
					}
#endif
					// end of [#2046]
				}
			}
		}
		fclose(Stream);
	}

	// [#2046] AU KJW 2011.04.20
#if (AU_VERSION)
	if( pBinBlock )
		delete pBinBlock;
#endif
	// end of [#2046]
	
	if (bSuccess)
		return TRUE;

	return FALSE;
}
// end of [#253]

// [#95] NH KGS 2008.04.08 Network 설정 변경후 In-service시 Auto Reboot
BOOL CMainFrame::StoreNetworkInformation()
{
	NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::StoreNetworkInformation() \n")));
	
	m_netInfo.nLineType			= MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE);
	m_netInfo.szEOTCheckMode	= MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE);
	m_netInfo.nTCPIPFlowControl	= MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE);
	m_netInfo.szMessageType		= MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE);
	
	m_netInfo.nATM_DHCP			= MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DHCP);
	m_netInfo.szATM_IpAddr		= MemGetStr(_MEM_FLD_TCPIPINFO,_MEM_VAR_TCPIP_ATM_IPADDR);
	m_netInfo.szATM_Subnet		= MemGetStr(_MEM_FLD_TCPIPINFO,_MEM_VAR_TCPIP_ATM_SUBNET);
	m_netInfo.szATM_Gateway		= MemGetStr(_MEM_FLD_TCPIPINFO,_MEM_VAR_TCPIP_ATM_GATEWAY);
	m_netInfo.szATM_DNS			= MemGetStr(_MEM_FLD_TCPIPINFO,_MEM_VAR_TCPIP_ATM_DNS);
	
	m_netInfo.szTerminalID		= MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);	// [#398] [NH] KSK 2008.8.6

	// [#539] US KSK 2009.07.06
	m_netInfo.szHostPhoneNo1	= MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1);
	m_netInfo.szHostAddress1	= MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1NAME);
	m_netInfo.nHostPortNo1		= MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1PORTNO);
	m_netInfo.nStandard3StatusMonitoringFlag = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_STATUSMONITORINGENABLE);
	m_netInfo.nStandard3CommunicationFlag = MemGetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE);
	m_netInfo.szStandard3CommunicationID = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID);
	m_netInfo.nStandard3CRCFlag	= MemGetInt(_MEM_FLD_TCPIPINFO,_MEM_VAR_TCPIP_CRCENABLE);
	m_netInfo.nSSLFlag			= MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_SSLENABLE);
	m_netInfo.szStandard1RoutingID = MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_ROUTINGID);
	m_netInfo.nKeyMode			= MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);
	m_netInfo.nKeyIndex			= MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX);

	// [#2172] NH KSK 2012.12.27
	m_netInfo.strKeyCheckSum	= Chk_Key[__min(MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX), MAX_PIN_KEYINDEX)];
	m_netInfo.strMacKeyCheckSum = Chk_Key[MAX_PIN_KEYINDEX];
	// end of [#2172]

	// [#2226] US Justin 2013.10.23 EMV Kernel Version and AID Selection
	m_netInfo.nEMVKernelVer		= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_KERNELVERSION);
	//m_netInfo.szEMVAIDStatus	= MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_AIDENDISABLE);
	m_netInfo.szEMVAIDStatus	= MemGetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID);			// [#2342] US Justin 2015.05.07
	// [#2226]

	m_netInfo.nCDUBindingOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDU_BINDING);			// [#2392] US Justin 2016.01.27
	
	NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::StoreNetworkInformation() return \n")));
	
	return TRUE;
}

int CMainFrame::NetworkInformationHasChanged()
{
	NHDEBUG(1, (_T("***ClerkCtrl***CMainFrame::NetworkInformationHasChanged() \n")));

	// Check Reboot
	if (m_netInfo.nLineType				!= MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE))
		return 1;	// reboot

	// [#2226] US Justin 2013.10.23 Kernel Selection
	else if(m_netInfo.nEMVKernelVer	!= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_KERNELVERSION))
		return 1;
	// End of [#2226]

	else if (m_netInfo.szEOTCheckMode		!= MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE)
			 || m_netInfo.nTCPIPFlowControl	!= MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE)
			 || m_netInfo.szMessageType		!= MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)
			 || m_netInfo.szTerminalID		!= MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID)
			 || m_netInfo.szHostPhoneNo1	!= MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1)
			 || m_netInfo.szHostAddress1	!= MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1NAME)
			 || m_netInfo.nHostPortNo1		!= MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1PORTNO)
			 || m_netInfo.nStandard3StatusMonitoringFlag	!= MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_STATUSMONITORINGENABLE)
			 || m_netInfo.nStandard3CommunicationFlag		!= MemGetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE)
			 || m_netInfo.szStandard3CommunicationID		!= MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID)
			 || m_netInfo.nStandard3CRCFlag	!= MemGetInt(_MEM_FLD_TCPIPINFO,_MEM_VAR_TCPIP_CRCENABLE)
			 || m_netInfo.nSSLFlag			!= MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_SSLENABLE)
			 || m_netInfo.szStandard1RoutingID != MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_ROUTINGID)	// auto configuration
			 || m_netInfo.nKeyMode			!= MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE)
			 || m_netInfo.nKeyIndex			!= MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX)
//			 || m_netInfo.strKeyCheckSum	!= Chk_Key[MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX)])
			 || m_netInfo.strKeyCheckSum	!= Chk_Key[__min(MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX), MAX_PIN_KEYINDEX)])	// [#2172] NH KSK 2012.12.27
		return 2;

	// [#2226] US Justin 2013.10.23 AID Selection
	//CString strEnabledAIDList = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_AIDENDISABLE);
	CString strEnabledAIDList = MemGetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID);				// [#2342] US Justin 2015.05.07

	if(m_netInfo.szEMVAIDStatus.CompareNoCase(strEnabledAIDList) != 0 )
		m_pDevCmn->Create_EMV_Termdata_From_POOL();
	// End of [#2226]

	// MAC CHECK SUM값이 변경되었을 경우 AUTO CONFIGURATION하도록 수정
	if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			||
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)
	{
//		if (m_netInfo.strMacKeyCheckSum != Chk_Key[15])
		if (m_netInfo.strMacKeyCheckSum != Chk_Key[MAX_PIN_KEYINDEX])	// [#2172] NH KSK 2012.12.27
			return 2;
	}

	// [#2392] US Justin 2016.01.27
	if( m_netInfo.nCDUBindingOption != MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDU_BINDING) )
		return 1;			// Reboot if CDU Binding option is changed.
	// End of [#2392]

	return FALSE;
}
// end of [#96]

//BOOL CMainFrame::ConnectHost()//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
BOOL CMainFrame::ConnectHost(int nConfigOption)//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
{
	BOOL bResult = FALSE;

	if (m_pDevCmn->fnAPL_CheckHostOpen())		
		return TRUE;

	if (m_pDevCmn->HostOpenRetryTime)			
		return FALSE;

	m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);		// [#24] KSK 2008.03.11 Configuration 시도시 Key Disable

	// [#2185] US Justin 2013.05.14 Support Dual Host DCC - Configuration beginning.
	//#if (APP_CUSTOM_PAI)		// [#2499] US Justin 2017.08.21 Enable Dual Host DCC for all customers.
	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) == ENABLE )
	{
		m_pDevCmn->HostOpenRetryTime = 0;	
		m_pTranCmn->m_HostConfig = HC_DUALHOST;
		m_pTranCmn->BIZ_HostConfigProc(TRUE, nConfigOption);
	}
	//#endif
	// End of [#2185]

	// Set host configuration back to ATM mode after possibly changing it for other host configuration attempts above
	m_pTranCmn->m_HostConfig = HC_ATM;

	if (m_pTranCmn->BIZ_HostConfigProc(TRUE, nConfigOption) == TRUE) //[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
	{														// Host OK
		// KSK 2010.06.18 아래 Flag를 BIZ_HostConfigProc에서 설정하도록 위치 변경
		// 사유 : Mac Error시에도 Configuration을 수행하므로 중복 Code 방지를 위함.
		//m_pDevCmn->HostOpenFlag = TRUE;						// Setting TRUE
		//m_pDevCmn->HostOpenRetryTime = 0;					// Clear
		NVDump('O', 'C', "73", L"", L"4:TRAN_OPEN" );

		// [#2449] US Justin 2016.11.08 Paydiant Online
		#if (APP_PAYDIANT_CCA)
			#ifdef APP_LOCAL_MODE
				Delay_Msg(3000);		//  DEMO MODE => NOT CALLING EXTENDED CONFIGURATION.
			#else
				if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE)==ENABLE)&&(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE) )
					m_pTranCmn->BIZ_HostConfigExtendedProc(TRAN_EXTENDED_ADDSVC_CONFIG, TC_EXTENDED_ADDSVC_CONFIG);
			#endif
		#endif
		// End of [#2449]

		// [#2535] US Justin 2018.04.16 GivePay Online
		#if(APP_GPAY_GIFTCARD_PURCHASE)
			if( (m_pTranCmn->m_GivePayData.m_bGivePayAvailable==TRUE) && (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_ENABLE_DISABLE)==ENABLE) )
			{
				m_pTranCmn->P_NH_GivePay_DownloadData(GPAY_DOWNLOAD_ALL);
				//m_pTranCmn->P_NH_GivePay_DownloadData(GPAY_DOWNLOAD_CMN_IMAGE);			// [#2574] US Justin GivePay Enhancement3
			}
		#endif
		// End of [#2535]

			
#if (APP_CUSTOM_PAI)
		// Download MCCP symmetric key
		// Perform a test transaction on each config
		if (m_pTranCmn->m_Pin4.m_bPin4Available == TRUE && MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE) == ENABLE)
		{
			m_pTranCmn->BIZ_MCCPSymmetricKeyConfig();
		}
#endif

	// [#RWC6-59] US William 2019.10.08 LibertyX
	// Get the LibertyX working key
#if (APP_LIBERTYX)
		if (m_pTranCmn->m_LXIsConfigured && m_pTranCmn->m_LXConfig.BuyBitcoinEnabled)
		{
			if (m_pTranCmn->m_LXConfig.HostDepModeEnabled)
			{
				m_pTranCmn->m_HostConfig = HC_LTX_HOST_DEP_MODE;
			}
			else
			{
				m_pTranCmn->m_HostConfig = HC_LIBERTYX;
			}

			if (m_pTranCmn->BIZ_HostConfigProc(TRUE, nConfigOption) != TRUE)
			{
				// Disable the LTX transaction if the host config fails
				m_pTranCmn->m_LXIsConfigured = false;
				NHERROR((L"LTX Configuration failed\r\n"));
				NVDump('F', 'C', "73", L"", L"5:LTX_HOST_ERROR" );
				bResult &= FALSE;
			}
		}
#endif

		bResult &= TRUE;
	}
	else
	{
		// Host NG
		m_pDevCmn->HostOpenFlag = FALSE;					// Setting FALSE
		m_pDevCmn->HostOpenRetryTime = 1;					// Set Time
		NVDump('F', 'C', "73", L"", L"5:HOST_ERROR" );

		SetTimer(TIMER_THREAD, 60000, NULL);
		m_pDevCmn->fnAPL_ClearError();				// [#100] KSK 2008.04.14 Configuration Fail시 "SC"가 남지 않도록 수정
		bResult = FALSE;
	}

	// Set host configuration back to ATM mode after possibly changing it for other host configuration attempts above
	m_pTranCmn->m_HostConfig = HC_ATM;

	m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);		// [#24] KSK 2008.03.11 Configuration Fail시 Key Enable이 필요함
	return bResult;
}

void CMainFrame::GetWeatherInfo()
{
	int				nIndex;
	CNHReadiniFile	Readini;
	PINIVALUE		pwIniValue;

	// open Screen Text File
	if (Readini.Open(WEATHER_FILENAME, CNHReadiniFile::modeRead, CNHReadiniFile::modeASCII))
	{
		// Clear
		for (int i = 0; i < 2; i++)
		{
			m_pTranCmn->m_sWeatherInfo[i].nYear = 0;
			m_pTranCmn->m_sWeatherInfo[i].nMonth = 0;
			m_pTranCmn->m_sWeatherInfo[i].nDay = 0;
			m_pTranCmn->m_sWeatherInfo[i].strDayOfWeek = L"";
			m_pTranCmn->m_sWeatherInfo[i].nWeatherCode = -1;
			m_pTranCmn->m_sWeatherInfo[i].strTemperatureType = L"";
			m_pTranCmn->m_sWeatherInfo[i].nHighestTemp = 0;
			m_pTranCmn->m_sWeatherInfo[i].nLowestTemp = 0;
		}

		while (NULL != (pwIniValue = Readini.ReadiniValue()))
		{
			if (pwIniValue->Section == L"WEATHERINFO")
			{
				if (pwIniValue->Key == L"TEMPERATURETYPE")
				{
					if (Asc2Int(pwIniValue->Values[0]) == 1)
					{
						m_pTranCmn->m_sWeatherInfo[0].strTemperatureType = L"C";
						m_pTranCmn->m_sWeatherInfo[1].strTemperatureType = L"C";
					}
					else
					{
						m_pTranCmn->m_sWeatherInfo[0].strTemperatureType = L"F";
						m_pTranCmn->m_sWeatherInfo[1].strTemperatureType = L"F";
					}
				}
			}
			else
			if (pwIniValue->Section.Left(7) == L"WEATHER" && pwIniValue->Section.GetLength() == 8)
			{
				nIndex = Asc2Int(pwIniValue->Section.Right(1));

//				RETAILMSG(1, (L"WEATHER INDEX [%d] - VALUE[%s]\n", nIndex, pwIniValue->Values[0]));

				if (nIndex <= 2 && nIndex > 0)
				{
					if (pwIniValue->Key == L"YEAR")
						m_pTranCmn->m_sWeatherInfo[nIndex-1].nYear = Asc2Int(pwIniValue->Values[0]);
					else if (pwIniValue->Key == L"MONTH")
						m_pTranCmn->m_sWeatherInfo[nIndex-1].nMonth = Asc2Int(pwIniValue->Values[0]);
					else if (pwIniValue->Key == L"DAY")
						m_pTranCmn->m_sWeatherInfo[nIndex-1].nDay = Asc2Int(pwIniValue->Values[0]);
					else if (pwIniValue->Key == L"DAYOFWEEK")
						m_pTranCmn->m_sWeatherInfo[nIndex-1].strDayOfWeek = pwIniValue->Values[0];
					else if (pwIniValue->Key == L"WEATHERCODE")
						m_pTranCmn->m_sWeatherInfo[nIndex-1].nWeatherCode = Asc2Int(pwIniValue->Values[0]);
//					else if (pwIniValue->Key == L"UNIT")
//						m_pTranCmn->m_sWeatherInfo[nIndex-1].strWeatherUnit = pwIniValue->Values[0];
					else if (pwIniValue->Key == L"HIGHESTTEMP")
					{
						m_pTranCmn->m_sWeatherInfo[nIndex-1].nHighestTemp = Asc2Int(pwIniValue->Values[0]);

						// [#2008] NH KSK 2010.11.30 -값일 경우 처리 추가
						if (m_pTranCmn->m_sWeatherInfo[nIndex-1].nHighestTemp >= 0x8000)
							m_pTranCmn->m_sWeatherInfo[nIndex-1].nHighestTemp = m_pTranCmn->m_sWeatherInfo[nIndex-1].nHighestTemp - (0xffff + 1);
						// end of [#2008]
					}
					else if (pwIniValue->Key == L"LOWESTTEMP")
					{
						m_pTranCmn->m_sWeatherInfo[nIndex-1].nLowestTemp = Asc2Int(pwIniValue->Values[0]);

						// [#2008] NH KSK 2010.11.30 -값일 경우 처리 추가
						if (m_pTranCmn->m_sWeatherInfo[nIndex-1].nLowestTemp >= 0x8000)
							m_pTranCmn->m_sWeatherInfo[nIndex-1].nLowestTemp = m_pTranCmn->m_sWeatherInfo[nIndex-1].nLowestTemp - (0xffff + 1);
						// end of [#2008]
					}
				}
			}
		}

		Readini.Close();
	}
}

void CMainFrame::GetNoticeInfo()
{
// 	CNHReadiniFile	Readini;
// 	PINIVALUE		pwIniValue;
// 	CString			strDataFileName;
// 
// 	strDataFileName = L"\\ATM\\NoticeInfo.dat";
// 
// 	// open Screen Text File
// 	if (Readini.Open(strDataFileName, CNHReadiniFile::modeRead, CNHReadiniFile::modeASCII))
// 	{
// 		// Clear
// 		m_pTranCmn->m_sNoticeInfo.strTitle = L"";
// 		m_pTranCmn->m_sNoticeInfo.strLine1 = L"";
// 		m_pTranCmn->m_sNoticeInfo.strLine2 = L"";
// 		m_pTranCmn->m_sNoticeInfo.strLine3 = L"";
// 
// 		while (NULL != (pwIniValue = Readini.ReadiniValue()))
// 		{
// 			if (pwIniValue->Section == L"NOTICE")
// 			{
// 				if (pwIniValue->Key == L"TITLE")
// 					m_pTranCmn->m_sNoticeInfo.strTitle = pwIniValue->Values[0].Left(28);
// 				else if (pwIniValue->Key == L"LINE1")
// 					m_pTranCmn->m_sNoticeInfo.strLine1 = pwIniValue->Values[0].Left(56);
// 				else if (pwIniValue->Key == L"LINE2")
// 					m_pTranCmn->m_sNoticeInfo.strLine2 = pwIniValue->Values[0].Left(56);
// 				else if (pwIniValue->Key == L"LINE3")
// 					m_pTranCmn->m_sNoticeInfo.strLine3 = pwIniValue->Values[0].Left(56);
// 			}
// 		}
// 
// 		Readini.Close();
// 	}

	// NVRAM에서 Read한다. (사양 Title : Max : 30, Line : Max : 40)
	m_pTranCmn->m_sNoticeInfo.strTitle = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_TITLE);
	m_pTranCmn->m_sNoticeInfo.strLine1 = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_MSG1);
	m_pTranCmn->m_sNoticeInfo.strLine2 = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_MSG2);
	m_pTranCmn->m_sNoticeInfo.strLine3 = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_MSG3);
}

// [#2580] AU Kook 2019.08.06
void CMainFrame::SynchronizeKCV()
{
	NHDEBUG(1, (_T(" \n")));
	NVDump('O', 'A', "11", L"", L"SyncKCV");	

	if (m_pDevCmn->fnPIN_GetDeviceStatus() != NORMAL)
	{
		NVDump('O', 'A', "11", L"", L"EPPErrClrKCV");	
		MemSetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX, 0);
		MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECK, L"");
		Chk_Key.RemoveAll();

		for(int i=0; i<= MAX_PIN_KEYINDEX; i++)
		{
			MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0+i, L"");
			Chk_Key.Add(MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0+i));
		}

		return;
	}

	m_pDevCmn->fnAPL_DeviceEnDisable(DEV_PIN, DISABLE);
	Delay_Msg(50);

	// [#2314] 512K EPP Synchronize KCV
	CString strTemp, strKCV;
	if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) >= 0)
	{
		strTemp = m_pDevCmn->fnPIN_Get_KCV_512K(TRIPLEDESKEY_NAME);						// MWI에서 장애시 "ERROR"로 올려줌
		if( (strTemp != L"ERROR")&&(strTemp.GetLength()>=6) )
		{
			strKCV.Format(L"%s %s", strTemp.Left(2), strTemp.Mid(2,2));
			MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0, strKCV);			// 512K Master Key는 Key Index 0에 Overwrite함

			// Check MAC Checksum
			strTemp = L"";
			if(MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING)
				strTemp = m_pDevCmn->fnPIN_Get_KCV_512K(SINGLEMACKEY_NAME);	
			else if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)
			{
				if (RegGetInt(L"SOFTWARE\\ATM\\DevInfo\\KeyMgr", L"TMACOption") == 1)
					strTemp = m_pDevCmn->fnPIN_Get_KCV_512K(TRIPLEMACKEY_NAME);
			}

			if((strTemp != L"ERROR")&&(strTemp.GetLength()>=6) )
			{
				strKCV.Format(L"%s %s", strTemp.Left(2), strTemp.Mid(2,2));
				MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM15, strKCV);		// 512K MAC Key는 Key Index 15에 Overwrite함
			}
		}
		else
		{
			// Error to get checksum.... => reset Checksum bytes.
			NHDEBUG(1, (_T("***ModeCtrl*** ERROR to GET KCV Reset all Checksum\n") ));
			MemSetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX, 0);
			MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECK, L"");
			for(int nKeyNum=0; nKeyNum<= MAX_PIN_KEYINDEX; nKeyNum++)
				MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0+nKeyNum, L"");
		}

		// [#2319] US Justin 2015.01.06 Leave 2nd Master Key Check Sum in Slot 1
		strTemp = m_pDevCmn->fnPIN_Get_KCV_512K(TRIPLEDESKEY_2ND_PIN_NAME);	
		if( (strTemp != L"ERROR")&&(strTemp.GetLength()>=6) )
		{
			strKCV.Format(L"%s %s", strTemp.Left(2), strTemp.Mid(2,2));
			MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM1, strKCV);	// 2nd_Master Key => Store in INDEX1
		}

#if(APP_LIBERTYX)
		strTemp = m_pDevCmn->fnPIN_Get_KCV_512K(LIBERTYXKEY_NAME);	
		if( (strTemp != L"ERROR")&&(strTemp.GetLength()>=6) )
		{
			strKCV.Format(L"%s %s", strTemp.Left(2), strTemp.Mid(2,2));
			MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM2, strKCV);	// LibertyX Key => Store in INDEX2
		}
#endif
	}
	// End of [#2314]

	// -- GET CHECK KEY VALUE --
	Chk_Key.RemoveAll();					// [#29] NH PSC 2008.03.18 메모리 Leak 대책으로 위치 이동

	// [#2172] NH KSK 2012.12.27
	for(int i=0; i<= MAX_PIN_KEYINDEX; i++)
		Chk_Key.Add(MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0+i));
	// end of [#2172]

	m_pDevCmn->fnAPL_DeviceEnDisable(DEV_PIN, ENABLE);
	Delay_Msg(50);
}
// end of [#2580]


// [#2011] NH KJW 2011.01.13
#if (US_VERSION)

// [#2403] US Justin 2016.03.14 Linq3 Optional Compile
#if (APP_LOTTERY)
BOOL CMainFrame::Main_IsLotteryServiceAvailable()
{
	/////////////////////////////////////////
	// 1. 전문 송수신이 완료 되었는지 검사
	// Provisioning 수신이 완료 되었는지
	if( Main_IsLotteryPreTransactionCompleted() == FALSE )
	{
		NHDEBUG(DBG_INFO, (_T("Main_IsLotteryPreTransactionCompleted() == FALSE\n")));
		return FALSE;
	}

	////////////////////////////////////////
	// 2. Lottery Parameter 검사
	if( Main_IsLotteryParametersEnabled() == FALSE )
	{
		NHDEBUG(DBG_INFO, (_T("Main_IsLotteryParametersEnabled() == FALSE\n")));
		return FALSE;
	}

	////////////////////////////////////////
	// 3. 필요 장치 검사
	if( Main_IsLotteryDeviceAvailable() == FALSE )
	{
		NHDEBUG(DBG_INFO, (_T("Main_IsLotteryDeviceAvailable() == FALSE\n")));
		return FALSE;
	}

	return TRUE;
}

BOOL CMainFrame::Main_IsLotteryPreTransactionCompleted()
{
	// Provisioning 수신이 완료 되었는지
	if( m_pTranCmn->m_pLotteryUpdater->m_bProvisioningUpdated == FALSE )
	{
		NHDEBUG(DBG_INFO, (_T("m_pTranCmn->m_pLotteryUpdater->m_bProvisioningUpdated == FALSE\n")));
		return FALSE;
	}

	// Game Parameter 수신이 완료 되었는지
	if( m_pTranCmn->m_pLotteryUpdater->m_bGameParameterUpdated == FALSE )
	{
		NHDEBUG(DBG_INFO, (_T("m_pTranCmn->m_pLotteryUpdater->m_bGameParameterUpdated == FALSE\n")));
		return FALSE;
	}

	// Resource 수신이 완료 되었는지
	if( m_pTranCmn->m_pLotteryUpdater->m_bResourceUpdated == FALSE )
	{
		NHDEBUG(DBG_INFO, (_T("m_pTranCmn->m_pLotteryUpdater->m_bResourceUpdated == FALSE\n")));
		return FALSE;
	}

	return TRUE;
}

//
BOOL CMainFrame::Main_IsLotteryParametersEnabled()
{

	// [#2144] Justin 2012.08.10 LOTTERY Enable condition on Welcome Screen => check only enable status and number of available games
	/*
	// Provisioning Parameter 검사
	if( m_pTranCmn->m_pLotteryParameter->IsLotteryEnabled() == FALSE )
	{
		NHDEBUG(DBG_INFO, (_T("m_pTranCmn->m_pLotteryParameter->IsLotteryEnabled() == FALSE\n")));
		return FALSE;
	}

	// Game Parameter 검사
	if( m_pTranCmn->m_pGameManager->IsGameAvailable() == FALSE )
	{
		NHDEBUG(DBG_INFO, (_T("m_pTranCmn->m_pGameManager->IsGameAvailable() == FALSE\n")));
		return FALSE;
	}
	*/
	if(  m_pTranCmn->m_pLotteryParameter->m_bEnabled == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[Lottery Enabled(%s)]\n", m_pTranCmn->m_pLotteryParameter->m_bEnabled?_T("TRUE"):_T("FALSE")));
		return FALSE;
	}

	if(  m_pTranCmn->m_pLotteryParameter->m_arstrGamesAvail.GetCount() <= 0 )
	{
		NHDEBUG(DBG_INFO, (L"[Lottery number of available Games() <= 0 ]\n"));
		return FALSE;
	}
	// End of [#2144]

	return TRUE;
}

// Lottery 설정상 서비스 가능한지 여부가 변경되었는지 판단하는 함수.
BOOL CMainFrame::Main_IsLotteryParametersChanged()
{
	static BOOL sbLotteryParametersOkSave = FALSE;
	BOOL bLotteryParametersOk = Main_IsLotteryParametersEnabled();

	if( sbLotteryParametersOkSave != bLotteryParametersOk )
	{
		NHDEBUG(DBG_INFO, (_T("Main_IsLotteryParametersChanged() (%s)->(%s)\n"), sbLotteryParametersOkSave?_T("TRUE"):_T("FALSE"), bLotteryParametersOk?_T("TRUE"):_T("FALSE")));
		sbLotteryParametersOkSave = bLotteryParametersOk;
		return TRUE;
	}

	return FALSE;
}

BOOL CMainFrame::Main_IsLotteryDeviceAvailable()
{
	// SPR
	if( (m_pDevCmn->DeviceDownDevice & DEV_SPR) || (m_pDevCmn->SlipStatus != SLIP_NORMAL) )
	{
		NHDEBUG(DBG_INFO, (_T("Main_IsLotteryDeviceAvailable() DeviceDownDevice(0x%02x), SlipStatus(%d)\n"), m_pDevCmn->DeviceDownDevice, m_pDevCmn->SlipStatus));
		return FALSE;
	}

	return TRUE;
}

BOOL CMainFrame::Main_IsLotteryDeviceChanged()
{
	static BOOL sbLotteryDeviceOkSave = FALSE;
	BOOL bLotteryDeviceOk = Main_IsLotteryDeviceAvailable();

	if( sbLotteryDeviceOkSave != bLotteryDeviceOk )
	{
		NHDEBUG(DBG_INFO, (_T("Main_IsLotteryDeviceChanged() (%s)->(%s)\n"), sbLotteryDeviceOkSave?_T("TRUE"):_T("FALSE"), bLotteryDeviceOk?_T("TRUE"):_T("FALSE")));
		sbLotteryDeviceOkSave = bLotteryDeviceOk;
		return TRUE;
	}

	return FALSE;
}

// end of [#2011]

// [#2011] NH KJW 2011.01.12 Provisioning, GameParameter, Resource 를 수행하는 루틴 추가
BOOL CMainFrame::Main_UpdateLotteryService()
{
	/////////////////////////////////////////////////////////////////////////////
	//	Lottery Provisioning/Game Parameter/Resource (HOST/RMS) Procedure
	/////////////////////////////////////////////////////////////////////////////

	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_LOTTERY_ENABLE) == TRUE )
	{
		// [#2144] US Justin 2012.09.24 Check update only after waiting time is expired...
		if( m_pTranCmn->m_pLotteryParameter->IsExpiredUpdateWaitingTime() == FALSE )
			return TRUE;
		// End of [#2144]

		if( m_pTranCmn->m_pLotteryParameter->IsExpiredRefreshTime() )
		{
			NHDEBUG(1, (_T("IsExpiredRefreshTime = TRUE\n")));
			m_pTranCmn->m_pLotteryUpdater->m_bProvisioningUpdated = FALSE;
			m_pTranCmn->m_pLotteryUpdater->m_bGameParameterUpdated = FALSE;
			m_pTranCmn->m_pLotteryUpdater->m_bResourceUpdated = FALSE;
		}

		/*
		1. 서비스 중에 refresh time이 지난 경우
		   1.1 Provisioning이 완료 안된 경우
		   1.2 Provisioning은 완료 되었으나
		       1.2.1 GameParameter가 완료 안된 경우
			   1.2.2 GameParameter가 완료 된 경우
		       1.2.1 Resource가 완료 안된 경우
			   1.2.2 Resource가 완료 된 경우
		*/

		// READY -> CUSTOM 으로 전환직전에 Provisioning미완료시 무조건 Provisioning수행이 아니라,
		// IsExpiredUpdateWaitingTime() TRUE시에만 수행함.
		// [#2144] US Justin 2012.09.24 Check update only after waiting time is expired...
		//if( m_pTranCmn->m_pLotteryParameter->IsExpiredUpdateWaitingTime() == FALSE )
		//	return TRUE;
		// End of [#2144]

		// Provisioning이 완료 안된 경우
		if( m_pTranCmn->m_pLotteryUpdater->m_bProvisioningUpdated == FALSE )
		{
			NHDEBUG(1, (_T("m_bProvisioningUpdated == FALSE\n")));
			m_pDevCmn->fnMCU_CardEnDisable(DISABLE, FALSE);

			// [#2325] NH KSK 2015.01.21 EMV Enable이므로 RFID도 Enable 처리
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
			{
				{
					m_pDevCmn->fnRFID_EntryDisable();
					m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
				}
			}
			// end of [#2325]

			if( m_pTranCmn->BIZ_LTRY_ProvisioningProc(TRUE) == TRUE )
			{
				NHDEBUG(1, (_T("[ProvisioningUpdated Successfully]\n")));
				m_pTranCmn->m_pLotteryUpdater->m_bProvisioningUpdated = TRUE;
			}
			else
			{
				// Provisioning 실패시, 다음번 Provisioning 시도 시간을 설정한다.
				// 다음번 provisioning 시도 시간이 될 때까지는 Lottery 서비스는 불가함.
				// Lottery 서비스 불가시에는 ATM거래를 기본으로 동작함.
				m_pTranCmn->m_pLotteryParameter->SetUpdateWaitingTime( LTRY_NUM_UPDATE_WAIT_SEC );
			}

			m_pTranCmn->m_pLotteryUpdater->SaveDat();
		}
		
		// Provisioning은 완료 되었으나, Game Parameters가 완료 안된 경우
		if( m_pTranCmn->m_pLotteryUpdater->m_bProvisioningUpdated && m_pTranCmn->m_pLotteryUpdater->m_bGameParameterUpdated == FALSE )
		{
			NHDEBUG(1, (_T("m_bProvisioningUpdated && m_bGameParameterUpdated == FALSE\n")));
			m_pDevCmn->fnMCU_CardEnDisable(DISABLE, FALSE);

			// [#2325] NH KSK 2015.01.21 EMV Enable이므로 RFID도 Enable 처리
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
			{
				{
					m_pDevCmn->fnRFID_EntryDisable();
					m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
				}
			}
			// end of [#2325]

			if( m_pTranCmn->BIZ_LTRY_GameParameterProc(TRUE) == TRUE )
			{
				m_pTranCmn->m_pLotteryUpdater->m_bGameParameterUpdated = TRUE;
			}
			else
			{
				m_pTranCmn->m_pLotteryParameter->SetUpdateWaitingTime( LTRY_NUM_UPDATE_WAIT_SEC );
			}

			m_pTranCmn->m_pLotteryUpdater->SaveDat();
		}
		
#ifdef APP_LOCAL_MODE
		if( m_pTranCmn->m_pLotteryUpdater->m_bProvisioningUpdated && m_pTranCmn->m_pLotteryUpdater->m_bResourceUpdated == FALSE )
		{
			m_pTranCmn->m_pLotteryUpdater->m_bResourceUpdated = TRUE;
			m_pTranCmn->m_pLotteryUpdater->SaveDat();
			m_pTranCmn->m_pLotteryParameter->SetUpdateWaitingTime( LTRY_NUM_UPDATE_WAIT_SEC );
		}
#else
		// Provisioning과 Game Parameters는 완료 되었으나, Resource가 완료 안된 경우
		// Resource Req/Resp는 1회 수행시 1개 파일만 다운로드한다.
		// 다운로드 성공시 다시 이 함수 진입시 다음파일을 다운로드한다.
		if( m_pTranCmn->m_pLotteryUpdater->m_bProvisioningUpdated && 
			m_pTranCmn->m_pLotteryUpdater->m_bGameParameterUpdated && 
			m_pTranCmn->m_pLotteryUpdater->m_bResourceUpdated == FALSE )
		{
			NHDEBUG(1, (_T("m_bProvisioningUpdated && m_bResourceUpdated == FALSE\n")));
			m_pDevCmn->fnMCU_CardEnDisable(DISABLE, FALSE);

			// [#2325] NH KSK 2015.01.21 EMV Enable이므로 RFID도 Enable 처리
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
			{
				{
					m_pDevCmn->fnRFID_EntryDisable();
					m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
				}
			}
			// end of [#2325]

			CString str(_T(""));
			if( m_pTranCmn->BIZ_LTRY_ResourceProc(TRUE) == TRUE ) // Resource Req/Resp 수행
			{
				if( m_pTranCmn->m_pResourcePool->IsResourceValid() == TRUE ) // Lottery Serivce Scope 리소스 local에 존재
				{
					m_pTranCmn->m_pLotteryUpdater->m_bResourceUpdated = TRUE;
				}
				else
				{
					m_pTranCmn->m_pLotteryParameter->SetUpdateWaitingTime( LTRY_NUM_UPDATE_WAIT_SEC );
				}
			}
			else
			{
				m_pTranCmn->m_pLotteryParameter->SetUpdateWaitingTime( LTRY_NUM_UPDATE_WAIT_SEC );
			}

			m_pTranCmn->m_pLotteryUpdater->SaveDat();
		}
#endif
		m_pTranCmn->m_pLotteryParameter->SetUpdateWaitingTime( LTRY_NUM_UPDATE_WAIT_SEC );  // [#2144] US Justin 2012.09.24 Bug Fix in cas ATM Time is future.(Expired time passed. => Infinite Loop)
	}

	return TRUE;
}
#endif
// End of [#2403]

#endif
// end of [#2011]
