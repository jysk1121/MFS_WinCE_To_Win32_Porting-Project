#include "stdafx.h"
#include ".\Common\NHDbgApi.h"
#include "UP_Remote.h"
#include "NetworkHelper.h"
#include "LibUpdate/UpdateRepoConfigurationManager.h"
#include "LibUpdate/UpdateRepoService.h"

#define IP_ACQUIRE_TIMEOUT_MS	(60 * 1000)
#define IP_ACQUIRE_POLL_RATE_MS	(1 * 1000)
#define IP_ACQUIRE_ERR_SHOW_MS	(5 * 1000)

CUP_Remote::CUP_Remote(void)
{
	m_strName = STATE_UP_FROM_REMOTE;
	CUpdateState::Register(m_strName, this);
}

CUP_Remote::~CUP_Remote(void)
{
}

/**
 * Load the update from the remote repository.
 * @param[in,out] StateInfo the state information shared among the update components
 * @returns a string representing the success of the process
 */
CString CUP_Remote::Process(CUpdateStateInfo &StateInfo)
{
	StateInfo.m_eUpdateType = UT_NONE;
	CString	destinationVersion = RegGetStr(_T("SOFTWARE\\ATM\\APP"), _T("RemoteDownloadVersion"), L"");
	if (destinationVersion.IsEmpty())
	{
		NHERROR((L"Destination version is empty\r\n"));
		return STATE_UP_REBOOT;
	}

	PrepareScreen(SCR_FILECTRL);
	ShowString(2, L"[ Waiting for a network connection... ]");
	ShowString(3, L"");
	ShowScreen(SCR_FILECTRL);
	WaitForMilliSecond(100);

	// Load NVRAM if possible
	GetMemoryAddress();
	nvramOk = !!CheckNVRam();

	if (nvramOk)
	{
		// This call uses info in the NVRAM, so if the NVRAM check fails, do not load the networking info.
		NetworkHelper::ConfigureNetworkInformation();
	}

	UpdateSourceInfo config;
	CUpdateRepoConfigurationManager manager;
	if (manager.GetConfiguration(&config) != ERR_OK)
	{
		NHERROR((L"Failed to get local configuration\r\n"));
		return STATE_UP_REBOOT;
	}

	if (!NetworkHelper::WaitForIpAddressOrNetworkConnection(IP_ACQUIRE_TIMEOUT_MS, config.RepositoryBaseURL))
	{
		NHERROR((L"Failed to get a network connection\r\n"));

		ShowString(2, L"Failed to acquire an");
		ShowString(3, L"IP address.");
		UpdateScreen();
		WaitForMilliSecond(IP_ACQUIRE_ERR_SHOW_MS);

		return STATE_UP_REBOOT;
	}

	ShowString(2, L"[ Updating Packages... ]");
	ShowString(3, L"");
	UpdateScreen();
	WaitForMilliSecond(100);

	CUpdateRepoService service(&config);
	NHINFO((L"Getting applicable versions from repo\r\n"));
	PackageList packages;
	if (service.GetComponentContents(COMPONENT_AP, packages) != REPOERR_OK)
	{
		NHERROR((L"Failed to download package list\r\n"));
		return STATE_UP_REBOOT;
	}

	bool foundVersion = false;
	Package destinationPackage;
	for (int i = 0; i < packages.GetCount(); i++)
	{
		POSITION pos = packages.FindIndex(i);
		Package p = packages.GetAt(pos);

		if (p.ReadableVersion == destinationVersion)
		{
			// Found the right one!
			foundVersion = true;
			destinationPackage = p;

			break;
		}
	}

	if (!foundVersion)
	{
		NHERROR((L"Destination package version not found: %s\r\n", destinationVersion));
		return STATE_UP_REBOOT;
	}

	ShowString(2, L"[ Downloading Metadata... ]");
	ShowString(3, L"");
	UpdateScreen();
	WaitForMilliSecond(100);

	// Download the manifest and stage
	Manifest manifest;
	if (service.GetPackageManifest(destinationPackage, manifest) != REPOERR_OK)
	{
		NHERROR((L"Manifest download failed\r\n"));
		return STATE_UP_REBOOT;
	}

	ShowString(2, L"[ Downloading S/W... ]");
	ShowString(3, L"");
	UpdateScreen();
	WaitForMilliSecond(100);

	// Download the software packages
	PreStagingResult result;
	if (service.StageAndValidateSoftwareUpgrade(manifest, result) != REPOERR_OK)
	{
		NHERROR((L"Staging failed\r\n"));
		return STATE_UP_REBOOT;
	}

	// Get Update Type
	CString strTemp;
	for (int i = 0; i < UT_MAX; i++)
	{
		strTemp = GetUpdateFileName((UpdateType)i, result.SoftwareDirectory);

		if (IsExistFile(strTemp) == TRUE)
		{
			StateInfo.m_eUpdateType = (UpdateType)i;
			break;
		}
	}

	if (StateInfo.m_eUpdateType == UT_NONE)
	{
		// No valid files found
		NHERROR((L"No valid files found\r\n"));
		return STATE_UP_REBOOT;
	}

	NHINFO((L"Done downloading s/w!\r\n"));
	// Run the validation/update jobs now
	return STATE_UP_VALIDATE;
}