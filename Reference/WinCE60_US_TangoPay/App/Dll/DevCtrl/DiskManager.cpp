#include "stdafx.h"
#include ".\Dev\DiskManager.h"
#include "FilePatternsConsts.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1
//#define DRY_RUN

// Partition mount directories
#ifdef UNDER_CE
	#define ATM1	L"\\ATM"
	#define ATM2	L"\\ATM2"
	#define OS		L"\\"
#else
	#define ATM1	L".\\ATM"
	#define ATM2	L".\\ATM2"
	#define OS		L".\\"
#endif

// The main function that checks if two given strings 
// match. The first string may contain wildcard characters 
// Stolen from: https://www.geeksforgeeks.org/wildcard-character-matching/
bool match(const wchar_t *first, const wchar_t * second);
bool match(const wchar_t *first, const wchar_t * second) 
{ 
    // If we reach at the end of both strings, we are done 
    if (*first == '\0' && *second == '\0') 
        return true; 
  
    // Make sure that the characters after '*' are present 
    // in second string. This function assumes that the first 
    // string will not contain two consecutive '*' 
    if (*first == '*' && *(first+1) != '\0' && *second == '\0') 
        return false; 
  
    // If the first string contains '?', or current characters 
    // of both strings match, or they are 32 ascii postitions away (i.e. different cases).
    if (*first == '?' || *first == *second || _wcsnicmp(first, second, 1) == 0)  
        return match(first+1, second+1); 
  
    // If there is *, then there are two possibilities 
    // a) We consider current character of second string 
    // b) We ignore current character of second string. 
    if (*first == '*') 
        return match(first+1, second) || match(first, second+1); 
    return false; 
} 


CDiskManager::CDiskManager(void)
{
	this->config = new CNHConfig();
}

CDiskManager::~CDiskManager(void)
{
	if (this->config != NULL) delete this->config;
	this->config = NULL;
}

/**
 * Gathers data about the disk utilization of the disk represented by the 'disk' parameter.
 * @param[in] disk The disk for which to return information
 * @param[out] useageData The struct to receive disk usage information
 * @returns bool Whether or not the operation was successful
 */
bool CDiskManager::GetDiskUtilization(APSTORAGESECTION disk, DiskUsageInformation &usageData)
{
	LPCWSTR mountLocation = CDiskManager::GetDirectoryNameForSection(disk);
	NHDEBUG(DBG_CALL, (L"Gathering disk usage for disk: %s\r\n", mountLocation));

	ULARGE_INTEGER availableToCaller;
	ULARGE_INTEGER totalBytes;
	ULARGE_INTEGER totalFreeBytes;

	if (!GetDiskFreeSpaceEx(mountLocation, &availableToCaller, &totalBytes, &totalFreeBytes))
	{
		NHERROR((L"Attempt to get disk free space failed with code: %d\r\n", GetLastError()));
		return false;
	}

	usageData.DiskName = mountLocation;
	usageData.TotalCapacityBytes = totalBytes.QuadPart;
	usageData.UsedCapacityBytes = totalBytes.QuadPart - totalFreeBytes.QuadPart;
	usageData.FreeCapacityBytes = totalFreeBytes.QuadPart;

	NHDEBUG(DBG_CALL, (L"Disk Data: free_bytes=%s, total_bytes=%s, total_caller=%s, percent_utilized:%.2f%%\r\n", 
		BytesToHumanReadable(totalFreeBytes.QuadPart, BU_MEBIBYTE), 
		BytesToHumanReadable(totalBytes.QuadPart, BU_MEBIBYTE), 
		BytesToHumanReadable(availableToCaller.QuadPart, BU_MEBIBYTE),
		usageData.PercentUtilized() * 100.0));

	return true;
}

/**
 * Removes all journal files in the journal DB destination directories
 * @returns true if the operation is successful
 */
bool CDiskManager::CleanupJournalFiles(int &count, CDevCmn* pDevcmn)
{
	count = 0;

	// Delete files in the journal DB, by calling existing CleanJNL()
	pDevcmn->m_JNLMgr.ClearJNL();

	return true;
}

/**
 * Deletes files which are not present in a list of known-required system files.
 * @param count[out] The number of files deleted
 * @returns true if the operation is successful
 */
bool CDiskManager::CleanupSystemFiles(int &count)
{
	count = 0;

	// Delete files in System
	DeleteFilesInDirectoryNotMatchingPattern(SYSFILE1_ROOT, SYSFILE1_PATTERNS, SYSFILE1_PATTERNS_MAX, count);

	return true;
}

/**
 * Removes files in ad directories which do not match a recognized filename pattern.
 * @param count[out] The number of files deleted
 * @returns true if the operation is successful
 */
bool CDiskManager::CleanupAdvertisements(int &count)
{
	count = 0;

	// Delete files in ADS root 1
	DeleteFilesInDirectoryNotMatchingPattern(ADS_ROOT_1, ADS_PATTERNS, ADS_PATTERNS_MAX, count);

	// Delete files in ADS root 2
	DeleteFilesInDirectoryNotMatchingPattern(ADS_ROOT_2, ADS_PATTERNS, ADS_PATTERNS_MAX, count);

	return true;
}

/**
 * Removes all logs in the logging destination directories
 * @returns true if the operation is successful
 */
bool CDiskManager::CleanupLogs()
{
	int count = 0;

	// Delete files in the logs dir
	DeleteFilesInDirectoryNotMatchingPattern(LOGS_ROOT, NULL, 0, count);

	return true;
}

LPCWSTR CDiskManager::GetDirectoryNameForSection(APSTORAGESECTION disk)
{
	switch (disk)
	{
	case AP_DISK_ATM:
		return ATM1;
	case AP_DISK_ATM2:
		return ATM2;
	case AP_DISK_OS:
		return OS;
	}

	// Return a default disk to avoid errors in the core lib call
	return ATM1;
}

bool CDiskManager::DeleteFilesInDirectoryNotMatchingPattern(LPCTSTR directory, const wchar_t * const patterns[], const int numPatterns, int &count)
{
	CStringArray files;

	if (!GetFilesInDirectory(directory, files))
	{
		NHERROR((L"Failed to get list of files in %s\r\n", directory));
		return false;
	}

	// Check all files in the results
	for (int i = 0; i < files.GetSize(); i++)
	{
		CString filename = files.GetAt(i);
	
		// Only delete a file if it DOES NOT match a pattern in the pattern list
		if (!FileMatchesPatternList(filename, patterns, numPatterns))
		{
#ifndef DRY_RUN
			if (!DeleteFile(filename))
			{
				NHERROR((L"Failed to delete %s\r\n", filename));
			}
			else 
			{
				NHDEBUG(DBG_CALL, (L"Deleted %s\r\n", filename));
				count++;
			}
#else
			NHWARN((L"Deleted %s\r\n", filename));
#endif
		}
	}

	return true;
}

bool CDiskManager::FileMatchesPatternList(LPCTSTR filepath, const wchar_t * const patterns[], const int numPatterns)
{
	// If no patterns, assume the file does not match
	if (patterns == NULL || numPatterns == 0)
	{
		return false;
	}

	// File component
	CString filename = wcsrchr(filepath, '\\');
	filename = filename.Mid(1); // Remove leading back slash

	for (int i = 0; i < numPatterns; i++)
	{
		const wchar_t *pattern = patterns[i];

		// Check for exact matches
		if (_wcsicmp(filename, pattern) == 0)
		{
			return true;
		}

		// Substitute the template variables
		CString renderedPattern = SubstituteTemplateVariables(pattern);

		// Check wildcards. Note: Matching entire filepath here...
		if (match(renderedPattern, filepath))
		{
			return true;
		}
	}

	// No match
	return false;
}

CString CDiskManager::SubstituteTemplateVariables(CString source)
{
	NH_SCR_CONFIG frontScreenResolution = this->config->GetScreenConfig(SCR_FRONT);

	CString screenWidth(L"");
	screenWidth.Format(L"%d", frontScreenResolution.nWidth);

	CString screenHeight(L"");
	screenHeight.Format(L"%d", frontScreenResolution.nHeight);

	// Replace template variables
	CString templatePattern(source);
	templatePattern.Replace(L"{{screen.res.w}}", screenWidth);
	templatePattern.Replace(L"{{screen.res.h}}", screenHeight);

	return templatePattern;
}