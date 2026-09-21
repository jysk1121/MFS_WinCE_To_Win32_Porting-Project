/////////////////////////////////////////////////////////////////////////////
// AtmClerkDiskManagement.cpp : Implementation of the CWinAtmCtrl ActiveX Control class.
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MainFrm.h"
#include "AtmClerkCtrl.h"

#include ".\Dev\DiskManager.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

// Cleanup Operation Flags
#define CLEANUP_JRNL	1
#define CLEANUP_SYS		1 << 2
#define CLEANUP_ADS		1 << 3
#define CLEANUP_LOGS	1 << 4

CString CMainFrame::ClerkDiskManagementScreenDataMake(int screenNumber)
{
	switch (screenNumber)
	{
	case 1000: return ClerkGetDiskManagementData();
	}

	return CString(L"");
}

CString CMainFrame::ClerkGetDiskManagementData()
{
	DiskUsageInformation atmUsage, atm2Usage, totalUsage;
	CDiskManager* manager = new CDiskManager();

	manager->GetDiskUtilization(AP_DISK_ATM, atmUsage);
	manager->GetDiskUtilization(AP_DISK_ATM2, atm2Usage);
	
	totalUsage = atmUsage + atm2Usage;

	CString data(L"");
	data.Format(L"%s%s%s%s%.2f%s%s%s%s%s%.2f%s%s%s%s%s%.2f",
		BytesToHumanReadable(atmUsage.UsedCapacityBytes, BU_MEBIBYTE), SCR_CMD_DELIMITER,	// ATM - Used
		BytesToHumanReadable(atmUsage.FreeCapacityBytes, BU_MEBIBYTE), SCR_CMD_DELIMITER,	// ATM - Free
		atmUsage.PercentUtilized() * 100, SCR_CMD_DELIMITER,								// ATM - Percent
		BytesToHumanReadable(atm2Usage.UsedCapacityBytes, BU_MEBIBYTE), SCR_CMD_DELIMITER,	// ATM2 - Used
		BytesToHumanReadable(atm2Usage.FreeCapacityBytes, BU_MEBIBYTE), SCR_CMD_DELIMITER,	// ATM2 - Free
		atm2Usage.PercentUtilized() * 100, SCR_CMD_DELIMITER,								// ATM2 - Percent
		BytesToHumanReadable(totalUsage.UsedCapacityBytes, BU_MEBIBYTE), SCR_CMD_DELIMITER,	// TTL - Used
		BytesToHumanReadable(totalUsage.FreeCapacityBytes, BU_MEBIBYTE), SCR_CMD_DELIMITER,	// TTL - Free
		totalUsage.PercentUtilized() * 100);								// TTL - Percent


	delete manager;
	manager = NULL;

	return data;
}

void CMainFrame::ClerkRunCleanupOperation(int operationFlags, int &count)
{
	int current;
	bool success = true;
	ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);
	
	CDiskManager* manager = new CDiskManager();
	
	// Journal Files
	if (success && ((operationFlags & CLEANUP_JRNL) == CLEANUP_JRNL))
	{
		success &= manager->CleanupJournalFiles(current, m_pDevCmn);
		count += current;
	}

	// System Files
	if (success && ((operationFlags & CLEANUP_SYS) == CLEANUP_SYS))
	{
		success &= manager->CleanupSystemFiles(current);
		count += current;
	}
	
	// Ads
	if (success && ((operationFlags & CLEANUP_ADS) == CLEANUP_ADS))
	{
		success &= manager->CleanupAdvertisements(current);
		count += current;
	}

	// Logs
	if (success && ((operationFlags & CLEANUP_LOGS) == CLEANUP_LOGS))
	{
		success &= manager->CleanupLogs();
	}

	ClerkNoticeToUser( success ? T_OPERATE_OK : T_OPERATE_NG);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDiskManagementMainProc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 1000 PROC
-------------------------------------------------------------------*/
void CMainFrame::ClerkDiskManagementMainProc()
{
	int removedFiles;
	NHDEBUG(DBG_CALL, (L"START\n"));

	while(TRUE)
	{
		///////////////////////////////////
		// WAIT FOR USER INPUT
		ClerkWaitScreenInput_byCha();
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

		//-----------------------------------
		// PROCESS KEY STRING
		//-----------------------------------
		// F1
		if (m_OpInfo.strReturn == L"AUTOCLEAN")
		{
			this->ClerkRunCleanupOperation( CLEANUP_JRNL | CLEANUP_SYS | CLEANUP_ADS | CLEANUP_LOGS, removedFiles );
		}
		// F2
		else if (m_OpInfo.strReturn == L"CLEANUPJRNL")
		{
			this->ClerkRunCleanupOperation( CLEANUP_JRNL, removedFiles );
		}
		// F4
		else if (m_OpInfo.strReturn == L"CLEANUPSYS")
		{
			this->ClerkRunCleanupOperation( CLEANUP_SYS, removedFiles );
		}
		// F6
		else if (m_OpInfo.strReturn == L"CLEANUPADS")
		{
			this->ClerkRunCleanupOperation( CLEANUP_ADS, removedFiles );
		}
		// F8
		else if (m_OpInfo.strReturn == L"CLEANUPLOGS")
		{
			this->ClerkRunCleanupOperation( CLEANUP_LOGS, removedFiles );
		}

		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 846;
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = OP_TIMEOUT_SCREEN_NUMBER;
			break;
		}

	} // end of while

	NHDEBUG(DBG_CALL, (L"END\n"));
}