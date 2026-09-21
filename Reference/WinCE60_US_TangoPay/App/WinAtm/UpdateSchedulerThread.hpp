#pragma once

#include "Common/NHDbgApi.h"
#include "MainFrm.h"

#ifdef _DEBUG
#  define LOOP_SLEEP_TIME_MILIS 1000 * 60		// 1 minute
#  define WINDOW_MIN 0
#  define WINDOW_MAX 24
#  define STALE_CACHE_ELAPSED_MILIS 1000 // 1 second
#else
#  define LOOP_SLEEP_TIME_MILIS 1000 * 60 * 30	// 30 minutes
#  define WINDOW_MIN 3
#  define WINDOW_MAX 4
#  define STALE_CACHE_ELAPSED_MILIS 1000 * 60 * 60 * 6 // 6 hours
#endif

/**
 * Class which handles the coordination of the update scheduling
 */
class CUpdateScheduler
{
private:
	CMainFrame* frame;

	unsigned long long lastCacheRefresh;

	unsigned long long GetUnixTimestamp()
	{
		const __int64 UNIX_TIME_START = 116444736000000000; //January 1, 1970 (start of Unix epoch) in "ticks" // A Windows tick is 100 nanoseconds. Windows epoch 1601-01-01T00:00:00Z, 11644473600 seconds before Unix epoch 1970-01-01T00:00:00Z.
		const __int64 TICKS_PER_MILLISECOND = 10000; //a tick is 100ns

		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);

		ULARGE_INTEGER li;
		li.LowPart  = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;

		return (li.QuadPart - UNIX_TIME_START) / TICKS_PER_MILLISECOND;	//Convert ticks since 1/1/1970 into seconds
	}

	bool IsCacheStale()
	{
		// Has x seconds past since last update
		return (GetUnixTimestamp() - lastCacheRefresh) >= STALE_CACHE_ELAPSED_MILIS;
	}

	/**
	 * @returns true when the system time is between 3 and 4 am
	 */
	bool IsWithinUpdateWindow()
	{

		SYSTEMTIME time;
		GetLocalTime(&time);

		return time.wHour >= WINDOW_MIN && time.wHour <= WINDOW_MAX;
	}

	/**
	 * @returns true when the system date/time is within the update window (cache update) and the upgrade window (s/w update)
	 */
	bool IsWithinUpgradeWindow()
	{
		SYSTEMTIME time;
		GetSystemTime(&time);
		int dow = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_DOW);

		return IsWithinUpdateWindow() && time.wDayOfWeek == dow;
	}

	bool ShouldAttemptUpgrade(Manifest &manifest)
	{
		bool isInUpgradeWindow = false;
		bool isWithinSeverityLimits = false;
		bool isAutoUpgradeEnabled = false;

		if (frame->m_pDevCmn->AtmStatus == ATM_CUSTOM)
		{
			// Do not update during a transaction
			isInUpgradeWindow = frame->m_pDevCmn->TranStatus == TRAN_OPEN || frame->m_pDevCmn->TranStatus == TRAN_IDLE;
			NHDBG((L"In upgrade window: %d\r\n", isInUpgradeWindow));
		}

		if (manifest.Metadata.Severity == 9)
		{
			// Bypass other checks
			NHERROR((L"Emergency upgrade\r\n"));
			return isInUpgradeWindow;
		}

		if (manifest.Metadata.Severity >= frame->m_updateConfiguration->MinimumSeverity)
		{
			NHINFO((L"Severity of package %s is higher or equal to the minimum severity\r\n", manifest.Metadata.ReadableVersion));
			isWithinSeverityLimits = true;
		}

		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_ENABLE) == ENABLE)
		{
			isAutoUpgradeEnabled = true;
		}

		return isInUpgradeWindow && isAutoUpgradeEnabled && isWithinSeverityLimits && IsWithinUpgradeWindow();
	}

	void UpdateCache()
	{
		NHDBG((L"Cache update started\r\n"));

		frame->m_updateCache->ClearCache();
		if (frame->m_updateRepo->GetComponentContents(COMPONENT_AP, frame->m_updateCache->Packages) != ERR_OK)
		{
			NHWARN((L"Update Cache refresh failed\r\n"));
			return;
		}

		lastCacheRefresh = GetUnixTimestamp();
		NHDBG((L"Cache update finished\r\n"));
	}

	bool HasNewerVersion(Manifest &m)
	{
		CRepoLocalCache* cache = frame->m_updateCache;
		CString currentVersionName = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_AP_VERSION);
		CString currentMwiVersionName = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_MWI_VERSION);
		CString destinationVersionKey;

		NHDBG((L"Downloading latest version manifest\r\n"));

		if (frame->m_updateRepo->GetLatestPackageManifest(COMPONENT_AP, m))
		{
			NHERROR((L"Error downloading latest version\r\n"));
			return false;
		}

		// The third decimal of current MWI version is equal to the fourth decimal of current AP version.
		if (currentMwiVersionName.Right(2) != L"00")
		{
			currentVersionName.AppendFormat(L".%s", currentMwiVersionName.Right(2));
		}

		// To match the AP version.
		destinationVersionKey.Format(L"%s%s", m.Metadata.Version.Lineage, m.Metadata.Version.Name);

		NHDBG((L"Update if %s != %s\r\n", destinationVersionKey, currentVersionName));

		if (destinationVersionKey != currentVersionName)
		{
			NHDBG((L"Version (%s) is newer than: %s\r\n", destinationVersionKey, currentVersionName));
			return true;
		}

		NHDBG((L"No new software version\r\n"));
		
		return false;
	}

	int UpdateToVersion(CString version)
	{
		char *swVersion = NULL;
		CFile file;

		if (version.IsEmpty())
		{
			NHINFO((L"Called without a parameter\r\n"));
			return CUpdateScheduler::INVALID_ARGS;
		}

		// Set registry settings
		if (!file.Open(REMOTE_UPDATE_INIT_FILE, CFile::modeReadWrite | CFile::modeCreate))
		{
			return false;
		}

		swVersion = new char[version.GetLength() + 1]();
		WideToMulti(swVersion, version, version.GetLength());

		file.Write(swVersion, version.GetLength());
		file.Close();
	
		delete [] swVersion;

		frame->TerminateATM();

		return CUpdateScheduler::OK;
	}

public:
	enum 
	{
		INVALID_ARGS,
		OK,
		UPDATE_PROC_FAIL,
	};

	CUpdateScheduler(CMainFrame* frame)
	{
		this->frame = frame;
	}

	~CUpdateScheduler()
	{

	}

	DWORD Run()
	{
		CString newVersionName;
		Manifest manifest;
		NHDBG((L"Starting updater thread\r\n"));

		// Main thread loop
		while (true)
		{
			Delay_Msg(LOOP_SLEEP_TIME_MILIS);

			if (!frame->m_remoteUpdatesAvailable)
			{
				// Remote updates are not available at this time
				continue;
			}

 			if (IsCacheStale() || IsWithinUpdateWindow())
			{
				// Update every x hours and if the system is within the update window
				UpdateCache();

				// Only upgrade the s/w if the system is within the upgrade window
				if (HasNewerVersion(manifest) && ShouldAttemptUpgrade(manifest))
				{
					newVersionName = manifest.Metadata.ReadableVersion; // This is the version that will be used for the update (and is in the package cache)
					NHDBG((L"Attempting update to %s\r\n", newVersionName));
					return UpdateToVersion(newVersionName);
				}
			}
		}

		return CUpdateScheduler::OK;
	};
};

/**
 * This thread runs forever, and continually polls the date/time to determine
 * if it should initiate a remote update.
 * @param[in] args a pointer to the MainFrame object
 */
DWORD UpdateSchedulerThread(LPVOID args);
DWORD UpdateSchedulerThread(LPVOID args)
{
	if (args == NULL)
	{
		return CUpdateScheduler::INVALID_ARGS;
	}

	CMainFrame* frame = static_cast<CMainFrame*>(args);

	CUpdateScheduler scheduler(frame);

	return scheduler.Run();
}