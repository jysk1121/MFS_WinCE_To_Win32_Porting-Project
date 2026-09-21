#include "stdafx.h"
#include "UP_Update.h"
#include "NFileIO.h"
//#include "UnZip.h"
#include <StoreMgr.h>

#define NH_DEBUG		// Debug Message On/Off	[#11] NH KSK 2010.10.08 Flash Rom Copy시 Delay를 위해 Debug Mode로 Release함
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1

// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Update
 FUNCTION NAME: CUP_Update()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUP_Update::CUP_Update()
{
	// 자신을 등록한다.
	m_strName = STATE_UP_UPDATE;
	CUpdateState::Register(m_strName, this);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Update
 FUNCTION NAME: ~CUP_Update()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUP_Update::~CUP_Update()
{
}


/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Update
 FUNCTION NAME: Process()
 RETURN TYPE  : 이동 할 다음 STATE
 PARAMETER    : STATE간 공유하는 정보.
 DESCRIPTION  : STATE의 정의된 동작을 처리하는 진입접 이다.
-------------------------------------------------------------------*/
CString CUP_Update::Process(CUpdateStateInfo &StateInfo)
{
	if (UpdateFile(StateInfo) == FALSE)	
	{
		DeleteFile(L"\\ATM\\WinAtm.exe");
		DeleteFile(L"\\ATM\\WinAtm.exe");

		NHERROR((_T("UPDATE FAILED.. RETRY.. SO DELETE WINATM.EXE\n")));
	}
	
	return STATE_UP_REBOOT;
}

BOOL CUP_Update::UpdateFile(CUpdateStateInfo &StateInfo)
{
	CString	strTitle;
	CString	strSrc, strDst;

	//////////////////////////////////////////////////////////////////////////
	//	GET INFORMATION

	strTitle = GetUpdateTitle(StateInfo, StateInfo.m_strUpdateRoot);	// [#11] NH KSK 2010.10.2
	StateInfo.m_strUpdateTitle = strTitle;

	// show screen.
	ShowString(1, strTitle);
	UpdateScreen();
	WaitForMilliSecond(10);


	//////////////////////////////////////////////////////////////////////////
	// UPDATE

	if ((StateInfo.m_eUpdateType == UT_MASTER_ZIP) ||
	    (StateInfo.m_eUpdateType == UT_MASTER_FILE))
	{
		// 2018.04.20 Disable format due to its instability on the old board.
// 		// 2018.04.05 Format before full erase.
// 		/////////////////////
// 		// begin of FORMAT //
// 		/////////////////////
// 
//		// NH Kook 2018.04.05
//		// begin of PARTITION FORMAT
//		{
//			HANDLE atmStorage = NULL;
//			HANDLE atmPartition = NULL;
//
//			// capsulate below by 'try/catch' to prevent popup window
//			// since there could be an exception throwed by unexpected error during parition work.
//			try
//			{
//				ShowString(2, L"[ Formatting... ]   0%\r");
//				UpdateScreen();
//				WaitForMilliSecond(10);
//
//				NHERROR((_T("@@@ EXECUTING OpenStore()! @@\n")));
//				atmStorage = OpenStore(_T("DSK1:"));
//				if (atmStorage == INVALID_HANDLE_VALUE)
//				{
//					NHERROR((_T("@@@ FAILED TO OpenStore()! @@\n"), GetLastError()));
//					goto PartitionFormatFinished;
//				}
//
//				NHERROR((_T("@@@ EXECUTING OpenPartition()! @@\n")));
//				atmPartition = OpenPartition(atmStorage, _T("Part02"));		// ATM : Part02, ATM2 : Part01
//				if (atmPartition == INVALID_HANDLE_VALUE)
//				{
//					NHERROR((_T("@@@ FAILED TO OpenPartition()! err(0x%08X)@@\n"), GetLastError()));
//					goto PartitionFormatFinished;
//				}
//
//				NHERROR((_T("@@@ EXECUTING DismountPartition()! @@\n")));
//				// need to dismount before format it.
//				if (DismountPartition(atmPartition) == FALSE)
//				{
//					NHERROR((_T("@@@ FAILED TO DismountPartition()! err(0x%08X)@@\n"), GetLastError()));
//					goto PartitionFormatFinished;
//				}
//
//				NHERROR((_T("@@@ EXECUTING FormatPartition()! @@\n")));
//				if (FormatPartition(atmPartition) == FALSE)
//				{				
//					NHERROR((_T("@@@ FAILED TO FormatPartition()! err(0x%08X)@@\n"), GetLastError()));
//					goto PartitionFormatFinished;
//				}
//
//				// remount after format.
//				NHERROR((_T("@@@ EXECUTING MountPartition()! @@\n")));
//				if (MountPartition(atmPartition) == FALSE)
//				{
//					NHERROR((_T("@@@ FAILED TO MountPartition()! err(0x%08X)@@\n"), GetLastError()));
//					goto PartitionFormatFinished;
//				}
//			}
//			catch (CException* ex)
//			{
//				NHERROR((_T("@@@ EXCEPTION THROWED DURING PARTITION FORMAT! err(0x%08X)@@\n"), GetLastError()));
//
//				TCHAR szMsg[1024] = {0, };
//				ex->GetErrorMessage(szMsg, 1024);
//				NHERROR((_T("@@@ %s\n"), szMsg));
//
//				goto PartitionFormatFinished;
//			}
//
//PartitionFormatFinished:
//			NHERROR((_T("@@@ Closing handles related to partition...\n"), GetLastError()));
//			CloseHandle(atmPartition);
//			CloseHandle(atmStorage);
//
//			ShowString(2, L"[ Formatting... ]   100%\r");
//			UpdateScreen();
//			WaitForMilliSecond(10);
//			///////////////////
//			// end of FORMAT //
//			///////////////////
//		}

		// Get source and destination path
		strSrc = GetUpdateFileName(UT_MASTER_FILE, StateInfo.m_strUpdateRoot);
		strDst = L"\\ATM";

		// Delete ATM Folder
		NHDEBUG(DBG_INFO, (L" -- DELETING ATM FOLDER...\n"));
		ShowString(2, L"[ Cleaning... ]");
		ShowString(3, L"");
		UpdateScreen();
		WaitForMilliSecond(100);
		ShDeleteDirectory(L"\\ATM");	// there should be an error since '\ATM' is a partition, not deletable.
		NHDEBUG(DBG_INFO, (L" -- DELETING ATM FOLDER... DONE\n"));

		int nFilesInAtmPartition = 0;
		CountFileInDirectory(L"\\ATM", L"*.*", nFilesInAtmPartition);
		NHDEBUG(DBG_INFO, (L" -- REMAINING FILES IN ATM PARTITION: %d\n", nFilesInAtmPartition));

		if (nFilesInAtmPartition > 1)
			return FALSE;
		
		NHDEBUG(DBG_INFO, (L" -- COPY FILES TO ATM FOLDER: (%s) -> (%s)\n", strSrc, strDst));
		// copy files to ATM folder
		NHDEBUG(DBG_INFO, (L" -- COPYING UPDATE FILES...\n"));
		ShowString(2, L"[ Installing... ]");
		ShowString(3, L"");
		UpdateScreen();
		WaitForMilliSecond(100);
		if (ShCopyDirectory(strSrc + L"\\*.*", strDst))
			NHDEBUG(DBG_INFO, (L" -- COPYING UPDATE FILES... DONE\n"));
		else
			NHDEBUG(DBG_INFO, (L" -- COPYING UPDATE FILES... FAILED\n"));

		ShowString(2, L"[ Finalizing... ]");
		ShowString(3, L"");
		UpdateScreen();
		WaitForMilliSecond(100);
	}
	else if ((StateInfo.m_eUpdateType == UT_PATCH_ZIP) ||
			 (StateInfo.m_eUpdateType == UT_PATCH_FILE))
	{
		int nCurFileCount = 0;
		int nTotalFileCount = 0;
		
		// Delete ATM Folder files
		DeleteFileBeforePatch(StateInfo, StateInfo.m_strUpdateRoot);	// [#11] NH KSK 2010.10.1

		WaitForMilliSecond(2000);

		// Get source and destination path
		strSrc = GetUpdateFileName(UT_PATCH_FILE, StateInfo.m_strUpdateRoot);
		strDst = L"\\ATM";

		NHDEBUG(DBG_INFO, (L" -- COPY FILES TO ATM FOLDER. src(%s) -> dst(%s)\n", strSrc, strDst));
		nCurFileCount = nTotalFileCount = 0;
		CountFileInDirectory(strSrc, L"*.*", nTotalFileCount);
		if (CopyFileInDirectory(strSrc, L"*.*", strDst, TRUE, nCurFileCount, nTotalFileCount) == FALSE)
		{
			NHERROR((_T("[UPDATE] Failed to FileCopy in ATM [PATCH FILE]\n")));
			return FALSE;
		}
	}

#if 0
	//////////////////////////////////////////////////////////////////////////
	// DELETE IMAGE FOLDER
	
	int nTemp;

	DeleteFileInDirectory(L"\\ATM\\SCREEN\\640_480", L"*.*", FALSE, nTemp, 0);
	DeleteFileInDirectory(L"\\ATM\\SCREEN\\800_600", L"*.*", FALSE, nTemp, 0);
	DeleteFileInDirectory(L"\\ATM\\ADVERTISEMENT\\640_480", L"*.*", FALSE, nTemp, 0);
	DeleteFileInDirectory(L"\\ATM\\ADVERTISEMENT\\800_600", L"*.*", FALSE, nTemp, 0);
#endif

	StateInfo.m_bUpdateSuccessful = TRUE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Update
 FUNCTION NAME: GetUpdateTitle()
 RETURN TYPE  : Update Title
 PARAMETER    : 
 DESCRIPTION  : Update의 Title을 가져온다.
-------------------------------------------------------------------*/
CString	CUP_Update::GetUpdateTitle(CUpdateStateInfo &StateInfo, CString strUpdateRoot)
{
	CNFileIO	FileIO;
	CString		strTitle, strFile;

	CString		strMasterDir = MASTER_60;
	CString		strPatchDir = PATCH_60;

	if (m_SystemConfig.GetCEVersion() == WINCE_7)
	{
		strMasterDir = MASTER_70;
		strPatchDir = PATCH_70;
	}

	// [#11] NH KSK 2010.10.2
	// [#GLDV-2853] read UpdateInfo.dat from RAMDISK, which was extracted/copied from USB/SDCARD
	if (StateInfo.m_eUpdateType == UT_MASTER_FILE || StateInfo.m_eUpdateType == UT_MASTER_ZIP)
		strFile.Format(L"%s\\%s\\UpdateInfo.dat", strUpdateRoot, strMasterDir);		// \UPDATE\NH2700CE\MASTER\Updateinfo.dat
	else if (StateInfo.m_eUpdateType == UT_PATCH_FILE || StateInfo.m_eUpdateType == UT_PATCH_ZIP)
		strFile.Format(L"%s\\%s\\UpdateInfo.dat", strUpdateRoot, strPatchDir);		// \UPDATE\NH2700CE\PATCH\Updateinfo.dat

	NHDEBUG(DBG_INFO, (L" -- Update Info Path (%s)\n", strFile));
	if (FileIO.Open(strFile))
	{
		FileIO.ReadString(strTitle);
		NHDEBUG(DBG_INFO, (L" -- File Open Success (Title : %s)\n", strTitle));
		FileIO.Close();
	}
	else
		NHDEBUG(DBG_INFO, (L" -- UpdateInfo File Open Failed (Title : %s)\n", strTitle));

	DeleteFile(strFile);			// [#223] AIREAT 2008.05.21 UpdateInfo.dat가 ATM Copy 방지.
	return strTitle;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUP_Update
 FUNCTION NAME: DeleteFileBeforePatch()
 RETURN TYPE  : Update Title
 PARAMETER    : 
 DESCRIPTION  : DeleteProf.dat에 명기된 파일을 지운다.
-------------------------------------------------------------------*/
void CUP_Update::DeleteFileBeforePatch(CUpdateStateInfo &StateInfo, CString strUpdateRoot)
{
	int			Index, Count;
	CNFileIO	FileIO;
	CString		strDeleteProfFile;
	CString		strDeletePath, strDeleteFile;

	CString		strMasterDir = MASTER_60;
	CString		strPatchDir = PATCH_60;

	if (m_SystemConfig.GetCEVersion() == WINCE_7)
	{
		strMasterDir = MASTER_70;
		strPatchDir = PATCH_70;
	}

	if (StateInfo.m_eUpdateType == UT_MASTER_FILE || StateInfo.m_eUpdateType == UT_MASTER_ZIP)
		strDeleteProfFile.Format(L"%s\\%s\\DeleteProf.dat", strMasterDir, strUpdateRoot);
	else if (StateInfo.m_eUpdateType == UT_PATCH_FILE || StateInfo.m_eUpdateType == UT_PATCH_ZIP)
		strDeleteProfFile.Format(L"%s\\%s\\DeleteProf.dat", strPatchDir, strUpdateRoot);	// KSK 2016.05.12 Bug Fix

	NHDEBUG(DBG_INFO, (L" -- DeleteProf Path (%s)\n", strDeleteProfFile));

	if (FileIO.Open(strDeleteProfFile))
	{
		NHDEBUG(DBG_INFO, (L" -- DeleteProf File Open Success\n"));

		while(FileIO.ReadString(strDeletePath))
		{
			strDeletePath.TrimRight(' ');

			NHDEBUG(DBG_INFO, (L" -- DeleteProf File Info Read Success (%s)\n", strDeletePath));

			if (IsFolder(strDeletePath))
			{
				DeleteFileInDirectory(strDeletePath, L"*.*", TRUE, Count, 0);
				NHDEBUG(DBG_INFO, (L" -- DeleteProf Folder Detected\n", strDeletePath));
			}
			else
			{
				Index = strDeletePath.ReverseFind('\\');
				Count = strDeletePath.GetLength() - Index;
				
				if (Index != -1 && Count > 1)
				{
					strDeleteFile = strDeletePath.Right(Count-1);
					strDeletePath = strDeletePath.Left(Index);

					NHDEBUG(DBG_INFO, (L" -- DeleteProf File Information (%s) (%s)\n", strDeleteFile, strDeletePath));
					
					DeleteFileInDirectory(strDeletePath, strDeleteFile, TRUE, Count, 0);
				}
			}
		}

		FileIO.Close();
	}
	else
		NHDEBUG(DBG_INFO, (L" -- DeleteProf File Open Failed\n"));

	DeleteFile(strDeleteProfFile);			// [#223] AIREAT 2008.05.21 UpdateInfo.dat가 ATM Copy 방지.
}
