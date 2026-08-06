// SoftwareUpdate.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "resource.h"
#include "SoftwareUpdate.h"

#include "../../Template/pkfuncs.h"
#include "NFileIO.h"

#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"


// OS Update
#include "diskio.h"
#include <Storemgr.h>
///////////////////

#define MAX_LOADSTRING 100


// FOR SCREEN
#define SCR_PREPARE				_T("PrevSetScreenNo")
#define SCR_SHOW				_T("APCenterMovie")
#define SCR_UPDATE				_T("APUpdate")
#define SCR_CMD_DELIMITER		_T("\r")

// USB PATH
#define USB_PATH				_T("\\USB\\SoftwarePackage")
#define CS130_OS_USB_PATH		_T("\\USB\\SoftwarePackage\\OS\\1280_800\\")
#define MF200_OS_USB_PATH		_T("\\USB\\SoftwarePackage\\OS\\1024_768\\")

#define CURRENT_SW_PATH			_T("\\NAND\\EagleCE_ATM")

// UPDATE TYPE
#define UPDATE_NONE				0
#define UPDATE_INSTALL_ZIP		1
#define UPDATE_INSTALL_FILE		2
#define UPDATE_UPDATE_ZIP		3
#define UPDATE_UPDATE_FILE		4
#define UPDATE_OS_FILE			5

HANDLE				ghEvent=NULL;		// never signaled.


BOOL	DeleteFileInDirectoryEx(LPCTSTR lpDstPath, LPCTSTR lpDstFile, int &nCurCnt, int nTotalCnt);
CString GetUpdateFileName(CString strUSBPath, int UpdateType);
BOOL	FileCopyFromUSB(CString strUSBPath, int &UpdateType);
BOOL	CountFileInDirectory(LPCTSTR lpDstPath, LPCTSTR lpDstFile, int &nCount);
BOOL	CopyFileInDirectoryEx(LPCTSTR lpSrcPath, LPCTSTR lpSrcFile, LPCTSTR lpDstPath, int &nCurCnt, int nTotalCnt, BOOL bDisplay = FALSE);

BOOL	IsFolder(CString strFileName);
void	DeleteFileBeforePatch(int UpdateType);

int		SearchProcessByName(const char *szToTerminate);


typedef struct{
	LPCTSTR pcszName;
	DWORD dwStartSec;
	DWORD dwTotalSizeBlock;
	DWORD dwByteOfBlock;
}IMAGE, *PIMAGE;

typedef struct{
	LPCTSTR pcszDevName;
	STOREINFO si;
	HANDLE hDevice;
	IMAGE mbr;
	IMAGE eboot;
	IMAGE nk;
}DISK_DEVICE_INFO, *PDISK_DEVICE_INFO;

DISK_DEVICE_INFO g_DiskArray[]=
{
// 	{	_T("DSK0:"), {0},	INVALID_HANDLE_VALUE, 
// 		{_T("MBR.NB0"), 0, 2, 512}, {_T("EBOOT.NB0"), 2,	0x400, 512}, {_T("NK.NB0"), 0x402, 0x40000, 512}	},
	{	_T("DSK1:"), {0},	INVALID_HANDLE_VALUE, 
		{_T("MBR.NB0"), 0, 2, 512}, {_T("EBOOT.NB0"), 2,	0x400, 512}, {_T("NK.NB0"), 0x402, 0x40000, 512}	},
// 	{	_T("DSK2:"), {0},	INVALID_HANDLE_VALUE, 
// 		{_T("MBR.NB0"), 0, 2, 512}, {_T("EBOOT.NB0"), 2,	0x400, 512}, {_T("NK.NB0"), 0x402, 0x40000, 512}	},
// 	{	_T("DSK3:"), {0},	INVALID_HANDLE_VALUE, 
// 		{_T("MBR.NB0"), 0, 2, 512}, {_T("EBOOT.NB0"), 2,	0x400, 512}, {_T("NK.NB0"), 0x402, 0x40000, 512}	},
};

#define dim(x) sizeof(x)/sizeof(x[0])

BOOL UpdateOS(int nUpdateType);
BOOL WriteImageToDisk(int nUpdateType, HANDLE hDisk, PIMAGE pImg, BOOL bDisplay);

void System_Reboot();


TCHAR szOut[256] = {0,};
TCHAR szFileFullNamePath[256] = {0,};


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	int				nUpdateType = UPDATE_NONE;
	CString			strSrc, StrDst;
	DWORD			dwSWUpdateType = UPDATE_KIND_SOFTWARE;
	CString			strSearchFile;
	BOOL			bOSResult = FALSE;

	ghEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

	// 로거 생성
	CEagleLogger::CreateInstance();
	CEagleLogger::GetInstance()->Initialize(_T("EagleCE_SWUpdate"), 30, 1024 * 1024);

	LOG(Info, _T("Software update start"));
	RETAILMSG(1, (_T("### [SW Update] Software update start ###\r\n")));

	CString strTemp;

	if (GetSystemMetrics(SM_CXSCREEN) == CS130_WIDTH_RESOLUTION)
		CUtil::CreateFileFromResource(hInstance, IDR_SCREEN_CS130, _T("\\Update.dat"));
	else
		CUtil::CreateFileFromResource(hInstance, IDR_SCREEN_MF200, _T("\\Update.dat"));

	WaitForSingleObject(ghEvent, 500);

	// Screen Instance 생성
	CSCR_Manager::CreateInstance();
	CSCR_Manager::GetInstance()->Initialize(hInstance, 0, 0, _T("UPDATE"), _T(""));
	
	CSCR_Manager::GetInstance()->SetVariable(SCR_PREPARE, _T("100"));
	CSCR_Manager::GetInstance()->SetVariable(_T("APValue1"), _T("Preparing to install"));
	CSCR_Manager::GetInstance()->SetVariable(SCR_SHOW, _T("100"));

	CString strSWUpdateType;
	strSWUpdateType = CUtil::GetSWUpdateType();

	// 0: USB에 의한 Software Update, 1: OS upgrade, 2: AMS 원격에 의한 Software Update
	if(_T("0") == strSWUpdateType)	// AMS 원격에 의한 Update라면, 파일 복사 불필요
	{
		//////////////////////////////////////////////////////////////////////////
		//	1. File Copy to \\SoftwarePackage From USB	
		if (!FileCopyFromUSB(USB_PATH, nUpdateType))	
		{
			LOG(Error, (_T("Failed to FileCopy From USB(%s)"), USB_PATH));
			goto ErrorReturn;
		}

		LOG(Info, _T("File copy success from USB to Root"));
	}
	else if (_T("1") == strSWUpdateType)	// OS
	{
		int nOSCurFileCount = 0, nOSTotalFileCount = 0;

		CreateDirectory(OS_UPDATE_POOL, NULL);

		// 해상도에 맞는 OS를 Copy하도록 로직 보완
		CString strOSFilePath;

		if (GetSystemMetrics(SM_CXSCREEN) == CS130_WIDTH_RESOLUTION)
			strOSFilePath = CS130_OS_USB_PATH;
		else
			strOSFilePath = MF200_OS_USB_PATH;

		CountFileInDirectory(strOSFilePath, _T("*.*"), nOSTotalFileCount);

		LOG(Info, _T("OS File total count is %d"), nOSTotalFileCount);

		if (!CopyFileInDirectoryEx(strOSFilePath, _T("*.*"), OS_UPDATE_POOL, nOSCurFileCount, nOSTotalFileCount))
		{
			LOG(Error, _T("Failed to FileCopy in ATM [OS]"));
			goto ErrorReturn;
		}

		nUpdateType = UPDATE_OS_FILE;

		LOG(Info, _T("Success to FileCopy in ATM [OS]"));
	}
	else
	{
		// UpdateType 검색
		for (int i = 1; i < 5; i++)
		{
			// Get Update File Name
			strSearchFile = GetUpdateFileName(UPDATE_POOL, i);

			// Check 
			if (CUtil::IsExistFile(strSearchFile))
			{
				nUpdateType = i;
				break;
			}
		}

		LOG(Info, _T("File copy success from AMS to Root"));
	}
	RETAILMSG(1, (_T("### [SW Update] Software update Type(%d) ###\r\n"), nUpdateType));

	WaitForSingleObject(ghEvent, 1000);

	//////////////////////////////////////////////////////////////////////////
	//	2. Unzip

	if (nUpdateType == UPDATE_INSTALL_ZIP)
	{
		LOG(Info, _T("File type is install zip file"));

		strSrc = GetUpdateFileName(UPDATE_POOL, UPDATE_INSTALL_ZIP);
		StrDst = GetUpdateFileName(UPDATE_POOL, UPDATE_INSTALL_FILE);

		LOG(Info, _T("Unzip path : src(%s), Dest(%s)"), strSrc, StrDst);

		if (!CUtil::UnzippingToFolder(strSrc, StrDst))
		{
			LOG(Error, _T("[SW Update] Failed to Unzip Install [%s] -> [%s]"), strSrc, StrDst);
			goto ErrorReturn;
		}

		LOG(Info, _T("[SW Update] Unzip success"));
		RETAILMSG(1, (_T("### [SW Update] Unzip success ###\r\n"), nUpdateType));

		nUpdateType = UPDATE_INSTALL_FILE;
	}
	else if (nUpdateType == UPDATE_UPDATE_ZIP)
	{
		LOG(Info, _T("File type is update zip file"));

		strSrc = GetUpdateFileName(UPDATE_POOL, UPDATE_UPDATE_ZIP);
		StrDst = GetUpdateFileName(UPDATE_POOL, UPDATE_UPDATE_FILE);

		if (!CUtil::UnzippingToFolder(strSrc, StrDst))
		{
			LOG(Error, _T("Failed to Unzip Update [%s] -> [%s]"), strSrc, StrDst);
			goto ErrorReturn;
		}

		LOG(Info, _T("[SW Update] Unzip success"));
		RETAILMSG(1, (_T("### [SW Update] Unzip success ###\r\n"), nUpdateType));

		nUpdateType = UPDATE_UPDATE_FILE;
	}

	// NAND Root로 Copy 완료 Signal Setting
	CUtil::Int_SetRegistry(UPDATE_PROC_REG_PATH, _T("FILE_PROC"), 1);

	// Main SW 종료 확인
	while(TRUE)
	{
		if (FindWindow(_T("__MFS_TERMINATE_WND__"), NULL) != NULL)
		{
			LOG(Info, _T("Waiting for Terminating software..."));
			WaitForSingleObject(ghEvent, 2000);
		}
		else
		{
			LOG(Info, _T("Terminated - Go Go Software process -"));
			break;
		}
	}

	WaitForSingleObject(ghEvent, 1000);

	//////////////////////////////////////////////////////////////////////////
	//	3. File Copy to NAND Flash from \\SoftwarePackage

	LOG(Info, _T("File copy start from Root to Flash memory"));

	if(UPDATE_KIND_OS_STRING == strSWUpdateType)
	{
		////////////////////////////////////////////////////////
		// OS Update
		///////////////////////////////////

		bOSResult = UpdateOS(nUpdateType);

		if(TRUE == bOSResult)
		{
			CSCR_Manager::GetInstance()->SetVariable(_T("APValue1"), _T("Success OS update"));
			CSCR_Manager::GetInstance()->SetVariable(SCR_UPDATE, _T(""));

			LOG(Info, _T("OS update success - ATM will be reboot"));
		}
		else
		{
			goto ErrorReturn;
		}
	}
	else
	{
		int	nCurFileCount = 0, nTotalFileCount = 0;

		if (nUpdateType == UPDATE_INSTALL_FILE)
		{
			DeleteFileBeforePatch(nUpdateType);	// Install 파일도 특정 File 삭제 가능하도록 적용

			LOG(Info, _T("File type is install files"));

			WaitForSingleObject(ghEvent, 1000);

			nCurFileCount = nTotalFileCount = 0;

			CreateDirectory(CURRENT_SW_PATH, NULL);

			// Delete EagleCE_ATM Folder
			CountFileInDirectory(CURRENT_SW_PATH, _T("*.*"), nTotalFileCount);

			LOG(Info, _T("File total count is %d"), nTotalFileCount);

			DeleteFileInDirectoryEx(CURRENT_SW_PATH, _T("*.*"), nCurFileCount, nTotalFileCount);

			LOG(Info, _T("Delete all old folder"));

			WaitForSingleObject(ghEvent, 1000);

			strSrc = GetUpdateFileName(UPDATE_POOL, UPDATE_INSTALL_FILE);
			StrDst = CURRENT_SW_PATH;

			nCurFileCount = nTotalFileCount = 0;
			CountFileInDirectory(strSrc, _T("*.*"), nTotalFileCount);
			LOG(Info, _T("Total Count File (%d)"), nTotalFileCount);

			if (!CopyFileInDirectoryEx(strSrc, _T("*.*"), StrDst, nCurFileCount, nTotalFileCount, TRUE))
			{
				LOG(Error, _T("Failed to File Copy in NAND flash memory [Install]"));
				goto ErrorReturn;
			}

			LOG(Info, _T("[SW Update] File copy success to NAND"));
			RETAILMSG(1, (_T("### [SW Update] File copy success to NAND ###\r\n"), nUpdateType));
		}
		else if (nUpdateType == UPDATE_UPDATE_FILE)
		{
			DeleteFileBeforePatch(nUpdateType);

			LOG(Info, _T("File type is update files"));

			WaitForSingleObject(ghEvent, 1000);

			CreateDirectory(CURRENT_SW_PATH, NULL);		// Update 매체 적용시 \\NAND\\EAGLE_CE Folder가 없을 경우 fail나는 현상 대책

			strSrc = GetUpdateFileName(UPDATE_POOL, UPDATE_UPDATE_FILE);
			StrDst = CURRENT_SW_PATH;

			nCurFileCount = nTotalFileCount = 0;
			CountFileInDirectory(strSrc, _T("*.*"), nTotalFileCount);

			LOG(Info, _T("File total count is %d"), nTotalFileCount);

			if (!CopyFileInDirectoryEx(strSrc, _T("*.*"), StrDst, nCurFileCount, nTotalFileCount, TRUE))
			{
				LOG(Error, _T("Failed to FileCopy in ATM [Update]"));
				goto ErrorReturn;
			}

			LOG(Info, _T("[SW Update] File copy success"));
			RETAILMSG(1, (_T("### [SW Update] File copy success to NAND ###\r\n"), nUpdateType));
		}
		else
		{
			LOG(Error, _T("Wrong Update Type [%d]"), nUpdateType);
			goto ErrorReturn;
		}

		CSCR_Manager::GetInstance()->SetVariable(_T("APValue1"), _T("Success software update"));
		CSCR_Manager::GetInstance()->SetVariable(SCR_UPDATE, _T(""));

		LOG(Info, _T("Software update success - ATM will be reboot"));
		RETAILMSG(1, (_T("### [SW Update] Software update success - ATM will be reboot ###\r\n"), nUpdateType));
	}

	CUtil::SetSWUpdateType(UPDATE_KIND_SOFTWARE_STRING);							// software update default 설정

	WaitForSingleObject(ghEvent, 2000);

	// Update 성공 후 EPP USB Driver쪽 Registry Setting 로직 추가
	CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_DRV_COMPORT_NAME, CRYPTERA_USB_DRV_COMPORT_VALUE);			// Comport Index Fix
	CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_BUF_IN_REG_NAME, CRYPTERA_USB_BUF_IN_REG_VALUE);				// Default is 8
	CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_BUF_OUT_REG_NAME, CRYPTERA_USB_BUF_OUT_REG_VALUE);			// Default is 8
	CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_CTRL_IN_BUF_REG_NAME, CRYPTERA_USB_CTRL_IN_BUF_REG_VALUE);	// Default is 4

	RegFlushKey(HKEY_LOCAL_MACHINE);
	CUtil::Sleep_Wait(3000);

	strTemp.Format(_T("[SW_Update] CRYPTERA EPP USB Driver - Set Registry Value"));
	LOG(Info, strTemp);

	//////////////////////////////////////////////////////////////////////////
	//	4. ATM reboot for Normal
	System_Reboot();

	return 0;

ErrorReturn:
	if(UPDATE_KIND_OS == dwSWUpdateType)
	{
		CSCR_Manager::GetInstance()->SetVariable(_T("APValue1"), _T("Failed OS update"));
		CSCR_Manager::GetInstance()->SetVariable(SCR_UPDATE, _T(""));

		LOG(Error, _T("OS update is failed - ATM will be reboot"));
	}
	else
	{
		CSCR_Manager::GetInstance()->SetVariable(_T("APValue1"), _T("Failed software update"));
		CSCR_Manager::GetInstance()->SetVariable(SCR_UPDATE, _T(""));

		LOG(Error, _T("[SW Update] Software update is failed - ATM will be reboot"));
		RETAILMSG(1, (_T("### [SW Update] Software update is failed - ATM will be reboot ###\r\n"), nUpdateType));
	}

	CUtil::SetSWUpdateType(UPDATE_KIND_SOFTWARE_STRING);							// software update default 설정

	WaitForSingleObject(ghEvent, 2000);

	// Update 실패후에도 EPP USB Driver쪽 Registry Setting 로직 추가
	CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_DRV_COMPORT_NAME, CRYPTERA_USB_DRV_COMPORT_VALUE);			// Comport Index Fix
	CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_BUF_IN_REG_NAME, CRYPTERA_USB_BUF_IN_REG_VALUE);				// Default is 8
	CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_BUF_OUT_REG_NAME, CRYPTERA_USB_BUF_OUT_REG_VALUE);			// Default is 8
	CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_CTRL_IN_BUF_REG_NAME, CRYPTERA_USB_CTRL_IN_BUF_REG_VALUE);	// Default is 4

	RegFlushKey(HKEY_LOCAL_MACHINE);
	CUtil::Sleep_Wait(3000);

	strTemp.Format(_T("[SW_Update] CRYPTERA EPP USB Driver - Set Registry Value"));
	LOG(Info, strTemp);

	//////////////////////////////////////////////////////////////////////////
	//	4. ATM reboot for Normal
	System_Reboot();

	return 0;
}


BOOL DeleteFileInDirectoryEx(LPCTSTR lpDstPath, LPCTSTR lpDstFile, int &nCurCnt, int nTotalCnt)
{
	CString strTemp;
	WIN32_FIND_DATA	FileData;
	HANDLE hSearch;
	WCHAR	strSearchDir[255];
	WCHAR	strNewDir[255];
	WCHAR	strSrcFile[255];

	BOOL bFinished = FALSE;
	BOOL bError = FALSE;
	int	 nFileCount = 0;

	memset(strSearchDir, 0, sizeof(strSearchDir));
	wcscpy(strSearchDir, lpDstPath);
	wcscat(strSearchDir, _T("\\"));
	wcscat(strSearchDir, lpDstFile);

	hSearch = FindFirstFile(strSearchDir, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		LOG(Error, _T("No. [%s\\%s] Directory."), lpDstPath, lpDstFile);
		return FALSE;
	}

	while (!bFinished)
	{
		//LOG(Info, (_T("[FIND][%03d] - [%s][%s]"), ++nFileCount, lpDstPath, FileData.cFileName));

		if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(lpDstFile, FileData.cFileName) != 0)
			{
				memset(strNewDir, 0, sizeof(strNewDir));

				wcscpy(strNewDir, lpDstPath);
				wcscat(strNewDir, _T("\\"));
				wcscat(strNewDir, FileData.cFileName);

				DeleteFileInDirectoryEx((LPCTSTR)strNewDir, _T("*.*"), nCurCnt, nTotalCnt);

				if (nTotalCnt > 0)
					nCurCnt++;

				// delete directory
				RemoveDirectory((LPCTSTR)strNewDir);

				WaitForSingleObject(ghEvent, 10);
			}
		}
		else
		{
			memset(strSrcFile, 0, sizeof(strSrcFile));

			// make source file full path
			wcscpy(strSrcFile, lpDstPath);
			wcscat(strSrcFile, _T("\\"));
			wcscat(strSrcFile, FileData.cFileName);

			if (nTotalCnt > 0)
				nCurCnt++;

			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
				SetFileAttributes(strSrcFile, FILE_ATTRIBUTE_NORMAL);

			DeleteFile(strSrcFile);
		}

		if (!FindNextFile(hSearch, &FileData))
		{
			bFinished = TRUE;

			if (GetLastError() == ERROR_NO_MORE_FILES)
			{
				//LOG(Info, _T("Found all of the files."));
			}
			else
			{
				//LOG(Info, _T("Unable to fine next file."));
				bError = TRUE;
			}
		}
	}

	// Close the Search handle
	if (!FindClose(hSearch))
	{
		LOG(Error, _T("Unable to close search handle"));
	}

	WaitForSingleObject(ghEvent, 10);

	if (bError)
		return FALSE;

	return TRUE;
}

BOOL FileCopyFromUSB(CString strUSBPath, int &UpdateType)
{
	int				i = 0;
	CString			strSrc, strDest;
	CString			strSearchFile;

	DeleteFileInDirectoryEx(UPDATE_POOL, _T("*.*"), i, 0);

	if (CUtil::IsExistFile(strUSBPath) == FALSE)
	{
		LOG(Error, _T("Failed to Find master.zip or patch.zip or folder in USB (%s)"), strUSBPath);
		return FALSE;
	}

	UpdateType = UPDATE_NONE;

	for (i = 1; i < 5; i++)
	{
		// Get Update File Name
		strSearchFile = GetUpdateFileName(strUSBPath, i);

		// Check 
		if (CUtil::IsExistFile(strSearchFile))
		{
			UpdateType = i;
			break;
		}
	}

	if (UpdateType == UPDATE_NONE)
	{
		LOG(Error, _T("Failed to Find master.zip or patch.zip or folder in USB (%s)"), strUSBPath);
		return FALSE;
	}

	CreateDirectory(UPDATE_POOL, NULL);

	strSrc = GetUpdateFileName(strUSBPath, UpdateType);
	strDest = GetUpdateFileName(UPDATE_POOL, UpdateType);

	// File Copy
	if (UpdateType == UPDATE_INSTALL_ZIP || UpdateType == UPDATE_UPDATE_ZIP)
	{
		if (!CopyFile(strSrc, strDest, FALSE))
		{
			LOG(Error, _T("Failed to Copy zip fil [%s] -> [%s]"), strSrc, strDest);
			return FALSE;
		}
	}
	// Folder Copy
	else
	{
		int	nCurFileCount = 0, nTotalFileCount = 0;

		CreateDirectory(strDest, NULL);

		WaitForSingleObject(ghEvent, 1000);

		nCurFileCount = nTotalFileCount = 0;
		CountFileInDirectory(strSrc, _T("*.*"), nTotalFileCount);
		LOG(Info, _T("Total Count File (%d)"), nTotalFileCount);

		if (!CopyFileInDirectoryEx(strSrc, _T("*.*"), strDest, nCurFileCount, nTotalFileCount))
		{
			LOG(Error, _T("Failed to Copy Directory [%s] -> [%s]"), strSrc, strDest);
			return FALSE;
		}
	}

	return TRUE;
}


CString GetUpdateFileName(CString strUSBPath, int UpdateType)
{
	CString str;

	if (UpdateType == UPDATE_INSTALL_ZIP)
		str.Format(_T("%s\\INSTALL.ZIP"), strUSBPath);
	else if (UpdateType == UPDATE_INSTALL_FILE)
		str.Format(_T("%s\\INSTALL"), strUSBPath);
	else if (UpdateType == UPDATE_UPDATE_ZIP)
		str.Format(_T("%s\\UPDATE.ZIP"), strUSBPath);
	else if (UpdateType == UPDATE_UPDATE_FILE)
		str.Format(_T("%s\\UPDATE"), strUSBPath);
	else
		str.Empty();

	return str;
}

BOOL	CountFileInDirectory(LPCTSTR lpDstPath, LPCTSTR lpDstFile, int &nCount)
{
	CString strTemp;
	WIN32_FIND_DATA	FileData;
	HANDLE hSearch;
	WCHAR	strSearchDir[255];
	WCHAR	strNewDir[255];

	BOOL bFinished = FALSE;
	BOOL bError = FALSE;
	int	 nFileCount = 0;

	memset(strSearchDir, 0, sizeof(strSearchDir));
	wcscpy(strSearchDir, lpDstPath);
	wcscat(strSearchDir, _T("\\"));
	wcscat(strSearchDir, lpDstFile);

	hSearch = FindFirstFile(strSearchDir, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		LOG(Error, _T("No. [%s\\%s] Directory."), lpDstPath, lpDstFile);
		return FALSE;
	}

	while (!bFinished)
	{
		RETAILMSG(0, (_T("[FIND][%03d] - [%s][%s]\n"), ++nFileCount, lpDstPath, FileData.cFileName));

		// Directory..
		if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(lpDstFile, FileData.cFileName) != 0)
			{
				memset(strNewDir, 0, sizeof(strNewDir));

				wcscpy(strNewDir, lpDstPath);
				wcscat(strNewDir, _T("\\"));
				wcscat(strNewDir, FileData.cFileName);

				CountFileInDirectory((LPCTSTR)strNewDir, _T("*.*"), nCount);

				nCount++;
			}
		}
		// File..
		else
		{
			nCount++;
		}

		if (!FindNextFile(hSearch, &FileData))
		{
			bFinished = TRUE;
			if (GetLastError() == ERROR_NO_MORE_FILES)
			{
				//LOG(Info, _T("Found all of the files."));
			}
			else
			{
				//LOG(Info, _T("Unable to fine next file."));
				bError = TRUE;
			}
		}
	}

	// Close the Search handle
	if (!FindClose(hSearch))
	{
		LOG(Error, _T("Unable to close search handle"));
	}

	WaitForSingleObject(ghEvent, 10);

	if (bError)
		return FALSE;

	return TRUE;
}

BOOL CopyFileInDirectoryEx(LPCTSTR lpSrcPath, LPCTSTR lpSrcFile, LPCTSTR lpDstPath, int &nCurCnt, int nTotalCnt, BOOL bDisplay)
{
	CString	strTemp, strTemp2;
	WIN32_FIND_DATA	FileData;
	HANDLE hSearch;
	WCHAR	strSearchDir[255];
	WCHAR	strNewDir[255];
	WCHAR	strSrcFile[255];
	WCHAR	strDstFile[255];

	BOOL bFinished = FALSE;
	BOOL bError = FALSE;
	int	 nFileCount = 0;

	memset(strSearchDir, 0, sizeof(strSearchDir));
	wcscpy(strSearchDir, lpSrcPath);
	wcscat(strSearchDir, _T("\\"));
	wcscat(strSearchDir, lpSrcFile);

	hSearch = FindFirstFile(strSearchDir, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		LOG(Error, _T("CopyFileInDirectoryEx - No. [%s\\%s] Directory."), lpSrcPath, lpSrcFile);
		return FALSE;
	}

	while (!bFinished)
	{
		RETAILMSG(0, (_T("[FIND][%03d] - [%s][%s]\n"), ++nFileCount, lpSrcPath, FileData.cFileName));
		
		if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(lpSrcFile, FileData.cFileName) != 0)
			{
				memset(strNewDir, 0, sizeof(strNewDir));

				wcscpy(strNewDir, lpSrcPath);
				wcscat(strNewDir, _T("\\"));
				wcscat(strNewDir, FileData.cFileName);

				// make destination file full path
				wcscpy(strDstFile, lpDstPath);
				wcscat(strDstFile, _T("\\"));
				wcscat(strDstFile, FileData.cFileName);

				// make new directory
				CreateDirectory(strDstFile, NULL);

				if (nTotalCnt > 0)
					nCurCnt++;

				WaitForSingleObject(ghEvent, 10);

				CopyFileInDirectoryEx((LPCTSTR)strNewDir, _T("*.*"), strDstFile, nCurCnt, nTotalCnt, bDisplay);
			}
		}
		else
		{
			memset(strSrcFile, 0, sizeof(strSrcFile));
			memset(strDstFile, 0, sizeof(strDstFile));

			// make source file full path
			wcscpy(strSrcFile, lpSrcPath);
			wcscat(strSrcFile, _T("\\"));
			wcscat(strSrcFile, FileData.cFileName);

			// make destination file full path
			wcscpy(strDstFile, lpDstPath);
			wcscat(strDstFile, _T("\\"));
			wcscat(strDstFile, FileData.cFileName);

			if (nTotalCnt > 0)
				nCurCnt++;

			if (bDisplay == TRUE)
			{
				// LCD Driver 부하를 줄이기 위해 5%단위로 표시하도록 로직 수정
				if (((nCurCnt*100/nTotalCnt) % 5) == 0)
				{
					strTemp.Format(_T("Copying in progress.....( %d %% )"), (nCurCnt*100/nTotalCnt));

					CSCR_Manager::GetInstance()->SetVariable(_T("APValue1"), strTemp);
					CSCR_Manager::GetInstance()->SetVariable(SCR_UPDATE, _T(""));
					WaitForSingleObject(ghEvent, 10);
				}
			}

			SetFileAttributes(strDstFile, FILE_ATTRIBUTE_NORMAL);

			DeleteFile(strDstFile);


			//RETAILMSG(1, (_T("[CopyFile] Call - FILENAME [%s] - Before\n"), strSrcFile));

			if (!CopyFile(strSrcFile, strDstFile, FALSE))
			{
				LOG(Error, _T("Failed to Copy [%s] -> [%s]"), strSrcFile, strDstFile);
				bError = TRUE;
			}

			//RETAILMSG(1, (_T("[CopyFile] Call - After\n")));
		}

		if (!FindNextFile(hSearch, &FileData))
		{
			bFinished = TRUE;
			if (GetLastError() == ERROR_NO_MORE_FILES)
			{
				//LOG(Info, _T("Found all of the files."));
			}
			else
			{
				//LOG(Info, _T("Unable to fine next file."));
				bError = TRUE;
			}
		}
	}

	// Close the Search handle
	if (!FindClose(hSearch))
	{
		LOG(Error, _T("Unable to close search handle"));
	}

	WaitForSingleObject(ghEvent, 10);

	if (bError)
		return FALSE;

	return TRUE;
}

BOOL	IsFolder(CString strFileName)
{
	WIN32_FIND_DATA	FileData;
	HANDLE			hSearch = NULL;

	hSearch = FindFirstFile(strFileName, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
		return FALSE;

	FindClose(hSearch);

	if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return TRUE;
	
	return FALSE;
}

void	DeleteFileBeforePatch(int UpdateType)
{
	int			Index, Count;
	CNFileIO	FileIO;
	CString		strDeleteProfFile;
	CString		strDeletePath, strDeleteFile;

	LOG(Info, _T("Check Delete File List"));

	strDeleteProfFile = GetUpdateFileName(UPDATE_POOL, UpdateType);
	strDeleteProfFile +=_T("\\DeleteList.txt");

	LOG(Info, _T("Delete list file is (%s)"), strDeleteProfFile);

	if (FileIO.Open(strDeleteProfFile))
	{
		while(FileIO.ReadString(strDeletePath))
		{
			strDeletePath.TrimRight(' ');

			if (IsFolder(strDeletePath))
			{
				DeleteFileInDirectoryEx(strDeletePath, _T("*.*"), Count, 0);
			}
			else
			{
				Index = strDeletePath.ReverseFind('\\');
				Count = strDeletePath.GetLength() - Index;

				if (Index != -1 && Count > 1)
				{
					strDeleteFile = strDeletePath.Right(Count-1);
					strDeletePath = strDeletePath.Left(Index);

					LOG(Info, _T("Delete path (%s) file(%s)"), strDeletePath, strDeleteFile);

					DeleteFileInDirectoryEx(strDeletePath, strDeleteFile, Count, 0);
				}
			}
		}

		FileIO.Close();
	}

	DeleteFile(strDeleteProfFile);

	LOG(Info, _T("DeleteFileBeforePatch End"));
}


int SearchProcessByName(const char *szToTerminate)
{
	HANDLE         hProcessSnap = INVALID_HANDLE_VALUE;
	BOOL           bRet      = FALSE; 
	PROCESSENTRY32 pe32      = {0}; 

	int iLenP,indx;
	char szToTermUpper[128];
	char szTemp[128];

	iLenP=strlen(szToTerminate);

	for(indx=0;indx<iLenP;indx++)	szToTermUpper[indx]=toupper(szToTerminate[indx]);
	szToTermUpper[iLenP]=0;

	hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

	if (hProcessSnap == INVALID_HANDLE_VALUE)		return 0;

	pe32.dwSize = sizeof(PROCESSENTRY32); 

	if (::Process32First(hProcessSnap, &pe32)) 
	{ 
		BOOL         bGotModule = FALSE; 
		MODULEENTRY32	me32       = {0}; 
		char*			pCharFound;

		do 
		{ 
			HANDLE hProcess = NULL;

			// Get the actual priority class. 
			hProcess = ::OpenProcess (PROCESS_ALL_ACCESS, 
				FALSE, pe32.th32ProcessID); 

			memset(szTemp, NULL, sizeof(szTemp));
			WideCharToMultiByte(CP_ACP, 0, pe32.szExeFile, -1, szTemp, sizeof(szTemp), NULL, NULL);

			pCharFound=NULL;
			pCharFound = strstr(_strupr(szTemp), szToTermUpper);

			CString strTemp;
			strTemp.Format(_T("Process Name : (%s) (%S) (%x)\n"), pe32.szExeFile, szToTermUpper, pCharFound);
			RETAILMSG(1,(strTemp));

			if(pCharFound != NULL)
			{
				if (hProcess != NULL)
				{
					bRet = TRUE;

					if (hProcess != NULL)
						CloseHandle(hProcess);

					break;
				}
			}
			else
			{
				if (hProcess != NULL)
					CloseHandle(hProcess);
			}

		} 
		while (::Process32Next(hProcessSnap, &pe32)); 
	} 
	else
	{
		bRet = FALSE;
	}

	::CloseToolhelp32Snapshot(hProcessSnap);

	return bRet;

}


/** ********************************************************************
* @brief OS Update
* @param HANDLE hDisk	디스크 드라이브 핸들
* @param PIMAGE pImg	이미지(OS 이미지, Boot 이미지 등)
* @retval TRUE: 성공, FALSE: 실패
************************************************************************/
BOOL UpdateOS(int nUpdateType)
{
	STOREINFO si={0};
	si.cbSize=sizeof(STOREINFO);
	BOOL bRet=TRUE;

	HANDLE hStore = FindFirstStore(&si);			

	while(hStore!=INVALID_HANDLE_VALUE && bRet)
	{
		for(int i=0; i<dim(g_DiskArray); i++)
		{	
			LOG(Info, _T("si.szDeviceName:[%s], g_DiskArray[%d].pcszDevName:[%s]"), si.szDeviceName, i, g_DiskArray[i].pcszDevName);
			if(wcscmp(si.szDeviceName, g_DiskArray[i].pcszDevName)==0)
			{
				memcpy(&g_DiskArray[i].si, &si, sizeof(STOREINFO));
				break;
			}
		}
		bRet=FindNextStore(hStore, &si);
	}

	FindCloseStore(hStore);

	g_DiskArray[0].hDevice = CreateFile(g_DiskArray[0].pcszDevName, 
		GENERIC_READ|GENERIC_WRITE,       
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
		);

	if(g_DiskArray[0].hDevice==INVALID_HANDLE_VALUE)
	{
//		SendMessage(pd->hDlg, 0x55AA, 0, (LPARAM)_T("CreateFile error"));
		LOG(Error, _T("Device CreateFile error"));
		return FALSE;
	}

	// Boot Loader Update
	bRet = WriteImageToDisk(nUpdateType, g_DiskArray[0].hDevice, &g_DiskArray[0].eboot, FALSE);

	// OS Update
	if(TRUE == bRet)
		bRet = WriteImageToDisk(nUpdateType, g_DiskArray[0].hDevice, &g_DiskArray[0].nk, TRUE);

	return bRet;
}


/** ********************************************************************
* @brief OS Update
* @param HANDLE hDisk	디스크 드라이브 핸들
* @param PIMAGE pImg	이미지(OS 이미지, Boot 이미지 등)
* @retval TRUE: 성공, FALSE: 실패
************************************************************************/
BOOL WriteImageToDisk(int nUpdateType, HANDLE hDisk, PIMAGE pImg, BOOL bDisplay)
{
	SG_REQ sg_req={ 0 };
	DWORD dwRetCB;
	unsigned char* WriteBuff = NULL;
	BOOL bRet=TRUE;
	CString strTemp;
	DWORD dwCurrentSize = 0;
	DWORD dwTotalFileSize = (pImg->dwTotalSizeBlock*pImg->dwByteOfBlock) / 1000;	// 진행률을 표시하기 위해 사이즈를 줄임.


//	wsprintf(szOut, _T("%s\\%s"), szFileFullNamePath, pImg->pcszName);
	wsprintf(szOut, _T("%s\\%s"), OS_UPDATE_POOL, pImg->pcszName);
	LOG(Info, _T("UPDATE File Path [%s]"), szOut);

	HANDLE hFile = CreateFile(szOut, 
		GENERIC_READ|GENERIC_WRITE,       
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
		);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		LOG(Error, _T("File CreateFile error.[%s]"), pImg->pcszName);
		return FALSE;
	}

	WriteBuff=( unsigned char*)malloc(pImg->dwByteOfBlock);
	if(WriteBuff==NULL)
	{
		LOG(Error, _T("Malloc memory pool error"));
		CloseHandle(hFile);
		return FALSE;
	}

	for(DWORD dwi=pImg->dwStartSec; dwi<pImg->dwStartSec+pImg->dwTotalSizeBlock; dwi++)
	{
		if(ReadFile(hFile, WriteBuff, pImg->dwByteOfBlock, &dwRetCB, 0) && dwRetCB==pImg->dwByteOfBlock)
		{
			sg_req.sr_start = dwi;   
			sg_req.sr_num_sec = 1;   
			sg_req.sr_num_sg = 1;   
			sg_req.sr_status = 0;   
			sg_req.sr_callback = NULL;   
			sg_req.sr_sglist[0].sb_len = pImg->dwByteOfBlock;   
			sg_req.sr_sglist[0].sb_buf = WriteBuff;

			if(DeviceIoControl(hDisk, IOCTL_DISK_WRITE, &sg_req, sizeof(sg_req), 0, 0, &dwRetCB, 0) && dwRetCB==pImg->dwByteOfBlock)   
			{
				if (bDisplay == TRUE)
				{
					// Display Driver 부하를 줄이기 위해 5% 단위로 표시하도록 로직 수정
					dwCurrentSize = ((dwi-pImg->dwStartSec)*pImg->dwByteOfBlock+pImg->dwByteOfBlock) / 1000;	// 진행률을 표시하기 위해 사이즈를 줄임.

					if (((dwCurrentSize*100/dwTotalFileSize) % 5) == 0)
					{
						strTemp.Format(_T("Processing OS update.....( %ld %%)"), (dwCurrentSize*100/dwTotalFileSize));
						CSCR_Manager::GetInstance()->SetVariable(_T("APValue1"), strTemp);
						CSCR_Manager::GetInstance()->SetVariable(SCR_UPDATE, _T(""));
					}
				}
			}
			else
			{
				LOG(Error, _T("DeviceIoControl DISK error"));
				bRet=FALSE;
				break;
			}
		}
		else
		{
			LOG(Error, _T("Read File error"));
			bRet=FALSE;
			break;
		}
	}
	CloseHandle(hFile);
	free(WriteBuff);

	if(bRet)
		wsprintf(szOut, _T("FINISH UPDATE OK"));
	else
		wsprintf(szOut, _T("UPDATE NG!!!!!"));

	LOG(Error, szOut);

	return bRet;
}

void System_Reboot()
{
	// Hardware Reset 시도 후 Software Reset 시도 (HW Reset은 신규 Main B/D만 지원되므로 두 개 모두 수행하도록 함
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

	//CUtil::Sleep_Wait(5000);
	WaitForSingleObject(ghEvent, 5000);

	LOG(Info, _T("Try to Software Reboot"));

	// Software Reboot 수행
	KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL);
}