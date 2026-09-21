#pragma once

#include ".\Common\CmnLib.h"
#include ".\Common\NHConfig.h"
#include ".\Dev\Devcmn.h"

// The following types and defs are used to determine which partition
// should be used by teh disk operations.
typedef int APSTORAGESECTION;
const APSTORAGESECTION AP_DISK_ATM	= 1;
const APSTORAGESECTION AP_DISK_ATM2	= 2;
const APSTORAGESECTION AP_DISK_OS	= 3;

typedef int BYTEUNIT;
const BYTEUNIT BU_BYTE     = 1;
const BYTEUNIT BU_KILOBYTE = BU_BYTE * 1000;
const BYTEUNIT BU_MEGABYTE = BU_KILOBYTE * 1000;
const BYTEUNIT BU_GIGABYTE = BU_MEGABYTE * 1000;

const BYTEUNIT BU_KIBIBYTE = BU_BYTE * 1024;
const BYTEUNIT BU_MEBIBYTE = BU_KIBIBYTE * 1024;
const BYTEUNIT BU_GIBIBYTE = BU_MEBIBYTE * 1024;

/**
 * A data structure which contains information about the
 * disk utilization
 */
typedef struct _disk_usage_information {
	CString				DiskName;
	unsigned long long	TotalCapacityBytes;
	unsigned long long	UsedCapacityBytes;
	unsigned long long	FreeCapacityBytes;

	_disk_usage_information(unsigned long long total = 0, unsigned long long used = 0, unsigned long long free = 0)
		: TotalCapacityBytes(total), UsedCapacityBytes(used), FreeCapacityBytes(free)
	{

	}

	double PercentUtilized()
	{
		return (double) UsedCapacityBytes / (double) TotalCapacityBytes;
	}

	_disk_usage_information operator+(const _disk_usage_information& adden) const
	{
		return _disk_usage_information(
			TotalCapacityBytes + adden.TotalCapacityBytes,
			UsedCapacityBytes + adden.UsedCapacityBytes,
			FreeCapacityBytes + adden.FreeCapacityBytes);
	}

} DiskUsageInformation, *LPDiskUsageInformation;

inline CString SuffixForByteUnit(BYTEUNIT unit)
{
	switch (unit)
	{
	case BU_BYTE:		return L"B";
	case BU_KILOBYTE:	return L"KB";
	case BU_MEGABYTE:	return L"MB";
	case BU_GIGABYTE:	return L"GB";

	case BU_KIBIBYTE:	return L"KiB";
	case BU_MEBIBYTE:	return L"MiB";
	case BU_GIBIBYTE:	return L"GiB";
	}

	return L"";
};

inline CString BytesToHumanReadable(unsigned long long bytes, BYTEUNIT unit)
{
	double value = (double) bytes / (double) unit;

	CString readableValue;
	readableValue.Format(L"%.2f%s", value, SuffixForByteUnit(unit));
	return readableValue;
};

/**
 * Disk Manager provides service methods for various operations of the
 * device flash disk. File system operations other than those directly related
 * to management operations, i.e. cleanup, are not exposed by this class.
 */
class AFX_EXT_CLASS CDiskManager
{
public:
	CDiskManager(void);
	~CDiskManager(void);

	CNHConfig* config;

	/**
	 * Gathers data about the disk utilization of the disk represented by the 'disk' parameter.
	 * @param[in] disk The disk for which to return information
	 * @param[out] useageData The struct to receive disk usage information
	 * @returns bool Whether or not the operation was successful
	 */
	bool GetDiskUtilization(APSTORAGESECTION disk, DiskUsageInformation &usageData);

	/**
	 * Removes all journal files in the journal DB destination directories
	 * @returns true if the operation is successful
	 */
	bool CleanupJournalFiles(int &count, CDevCmn* devcmn);

	/**
	 * Deletes files which are not present in a list of known-required system files.
	 * @param count[out] The number of files deleted
	 * @returns true if the operation is successful
	 */
	bool CleanupSystemFiles(int &count);

	/**
	 * Removes files in ad directories which do not match a recognized filename pattern.
	 * @param count[out] The number of files deleted
	 * @returns true if the operation is successful
	 */
	bool CleanupAdvertisements(int &count);

	/**
	 * Removes all logs in the logging destination directories
	 * @returns true if the operation is successful
	 */
	bool CleanupLogs();


private:
	static LPCWSTR GetDirectoryNameForSection(APSTORAGESECTION disk);

	bool DeleteFilesInDirectoryNotMatchingPattern(LPCTSTR directory, const wchar_t * const patterns[], const int numPatterns, int &count);

	bool FileMatchesPatternList(LPCTSTR filepath, const wchar_t * const patterns[], const int numPatterns);

	CString SubstituteTemplateVariables(CString source);
};
