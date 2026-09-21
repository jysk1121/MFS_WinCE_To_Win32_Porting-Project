#pragma once

#include "FileMgr.h"
#include "LibUpdate/UpdateRepoConfigurationManager.h"
#include "LibUpdate/UpdateRepoService.h"

/**
 * Updates the AP from the "latest" tag in the repository because the update failed
 */
class CUP_Emergency : public CFileMgr
{
public:
	CUP_Emergency(void);
	~CUP_Emergency(void);

	/**
	 * Loads the update files from the repository's latest tag
	 * @param[in,out] StateInfo the state information shared among the update components
	 * @returns a string representing the success of the process
	 */
	CString Process(CUpdateStateInfo &StateInfo);

private:
	bool nvramOk;

	bool GetBestConfiguration(UpdateSourceInfo* updateSources);
	bool WaitForNetworkConnection(CString repoHost);
	CString GetCountryCode();
	bool GetCountryCodeFromCDU(CString &countryCode);
	bool GetCountryCodeFromNvram(CString &countryCode);
};

CUP_Emergency	g_CUP_Emergency;
