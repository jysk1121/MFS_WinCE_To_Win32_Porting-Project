#include "stdafx.h"
#include "UP_Emergency.h"
#include "NetworkHelper.h"
#include ".\Net\NetWork.h"
#include ".\Common\NHDbgApi.h"

#define DEFAULT_URL L"nhasoftware.s3.us-east-2.amazonaws.com"
#define DEFAULT_TERRITORY L"US"

#define IP_ACQUIRE_TIMEOUT_MS		(60 * 1000)
#define IP_ACQUIRE_ERR_SHOW_MS		(5 * 1000)

bool cdu_country_to_territory(TCHAR country[4], CString &countryCode);
bool cdu_country_to_territory(TCHAR country[4], CString &countryCode)
{
	bool result = true;
	if (country[0] == 'U')
	{
		countryCode = L"US";
	}
	else if (countryCode[0] == 'M')
	{
		countryCode = L"MEX";
	}
	else if (_tcsclen(country) == 1 && country[0] == 'A')
	{
		countryCode = L"CAN";
	}
	else if (_tcsclen(country) >= 1 && country[0] == 'C')
	{
		countryCode = L"CAN";
	}
	else 
	{
		result = false;
	}

	return result;
}


CUP_Emergency::CUP_Emergency(void)
{
	m_strName = STATE_UP_EMERGENCY_REMOTE;
	CUpdateState::Register(m_strName, this);
}

CUP_Emergency::~CUP_Emergency(void)
{
}

CString CUP_Emergency::Process(CUpdateStateInfo &StateInfo)
{
	UpdateSourceInfo config;
	Manifest manifest;
	NH_OS_VERSION	eOSVersion = m_SystemConfig.GetOSVersion();

	PrepareScreen(SCR_FILECTRL);
	ShowString(1, L"");
	ShowString(2, L"[ Gathering Configuration Data... ]");
	ShowString(3, L"");
	if (eOSVersion == NH_OS_MX5200SE || eOSVersion == NH_OS_MX2800SE)
		ShowString(6, L"AP Software Auto-Recovery");
	else
		ShowString(5, L"AP Software Auto-Recovery");
	ShowScreen(SCR_FILECTRL);
	WaitForMilliSecond(100);

	if (!IsExistFile(CA_ROOT_CERTS_FILE))
	{
		CNetWork::InstallCAFiles();
	}

	// Load NVRAM if possible
	GetMemoryAddress();
	nvramOk = !!CheckNVRam();

	if (!GetBestConfiguration(&config))
	{
		NHERROR((L"Error creating configuration\r\n"));
		return STATE_UP_FROM_USB;
	}

	ShowString(2, L"[ Waiting for a network connection... ]");
	ShowString(3, L"");
	UpdateScreen();
	WaitForMilliSecond(100);

	if (!WaitForNetworkConnection(config.RepositoryBaseURL))
	{
		NHERROR((L"Could not find a valid network configuration\r\n"));
		return STATE_UP_FROM_USB;
	}

	ShowString(2, L"[ Downloading Metadata... ]");
	ShowString(3, L"");
	UpdateScreen();
	WaitForMilliSecond(100);

	CUpdateRepoService service(&config);
	if (service.GetLatestPackageManifest(COMPONENT_AP, manifest) != REPOERR_OK)
	{
		NHERROR((L"Failed to download manifest for latest package\r\n"));
		return STATE_UP_FROM_USB;
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
		return STATE_UP_FROM_USB;
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
		return STATE_UP_FROM_USB;
	}

	NHINFO((L"Done downloading s/w!\r\n"));
	// Run the validation/update jobs now
	return STATE_UP_VALIDATE;
}

bool CUP_Emergency::GetBestConfiguration(UpdateSourceInfo* updateSources)
{
	CUpdateRepoConfigurationManager manager;
	CONFIGERR result = ERR_OK;
	CString apVersion;

	// 1. Get local configuration
	CUpdateRepoConfigurationManager::InstallDefaultConfigurationFiles();
	result = manager.GetConfiguration(updateSources);
	if (result == ERR_OK || result == ERR_TID_BAD)
	{
		NHINFO((L"Local configuration validated!\r\n"));
		return true;
	}

	// 2. Make default configuration
	updateSources->Clear();
	updateSources->MinimumSeverity = 0;
	updateSources->RepositoryBaseURL = DEFAULT_URL;
	updateSources->AuthenticationType = AUTHN_NONE;

	updateSources->Hives.Add(L"retail");

	if (m_SystemConfig.GetCEVersion() == WINCE_7)
	{
		updateSources->Hives.Add(L"SE70");
	}
	else 
	{
		updateSources->Hives.Add(L"SE60");
	}

	updateSources->Hives.Add(GetCountryCode());
	updateSources->Hives.Add(L"V");
	updateSources->Hives.Add(m_SystemConfig.GetMachineType());
	NHINFO((L"Built default configuration.\r\n"));

	// 3. Attempt to get best fit from NVRAM values (NVRAM may be uninitialized)
	if (nvramOk)
	{
		apVersion = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_AP_VERSION);
		if (!apVersion.IsEmpty())
		{
			updateSources->Hives.SetAt(3, apVersion.Left(1));
			NHINFO((L"Set version lineage.\r\n"));
			return true;
		}
	}

	return true;
}

bool CUP_Emergency::WaitForNetworkConnection(CString repoHost)
{
	if (nvramOk)
	{
		// Use configured values
		NetworkHelper::ConfigureNetworkInformation();

		if (NetworkHelper::WaitForIpAddressOrNetworkConnection(IP_ACQUIRE_TIMEOUT_MS, repoHost))
		{
			return true;
		}

		// Now try static
		NetworkHelper::ConfigureForStaticIp();

		if (NetworkHelper::WaitForIpAddressOrNetworkConnection(IP_ACQUIRE_TIMEOUT_MS, repoHost))
		{
			return true;
		}
	}

	// Fall back to DHCP
	NetworkHelper::ConfigureForDhcp();
	return NetworkHelper::WaitForIpAddressOrNetworkConnection(IP_ACQUIRE_TIMEOUT_MS, repoHost);
}

CString CUP_Emergency::GetCountryCode()
{
	CString countryCode;

	if (GetCountryCodeFromCDU(countryCode))
	{
		return countryCode;
	}

	if (GetCountryCodeFromNvram(countryCode))
	{
		return countryCode;
	}

	return DEFAULT_TERRITORY;
}

bool CUP_Emergency::GetCountryCodeFromCDU(CString &countryCode)
{
	typedef bool (*OpenCduFunc)();
	typedef bool (*CloseCduFunc)();
	typedef DWORD (*GetVersionFunc)(USHORT*, ULONG[4], TCHAR[4], USHORT*);

	USHORT cstNum = 0;
	ULONG cstIndex[4] = {};
	TCHAR country[4] = {};
	USHORT cstType = 0;
	bool result = false;
	HINSTANCE cduDiagnosticsDll = ::LoadLibrary(VDM_CDU_FULL_PATH);
	if (cduDiagnosticsDll == NULL)
	{
		return false;
	}

	OpenCduFunc openCdu = reinterpret_cast<OpenCduFunc>(GetProcAddress(cduDiagnosticsDll, _T("CDM_OpenDevice")));
	CloseCduFunc closeCdu = reinterpret_cast<CloseCduFunc>(GetProcAddress(cduDiagnosticsDll, _T("CDM_CloseDevice")));

	// DWORD ret = getCduVersion(USHORT* cstNum, ULONG[4] cstIndex, TCHAR[4] country, USHORT* cstType);
	GetVersionFunc getCduVersion = reinterpret_cast<GetVersionFunc>(GetProcAddress(cduDiagnosticsDll, _T("CDM_GetVersion")));

	if (openCdu == NULL || closeCdu == NULL || getCduVersion == NULL)
	{
		NHERROR((L"Unable to load CDU diagnostic functions.\r\n"));
		goto cleanup;
	}

	if (!openCdu())
	{
		NHERROR((L"CDU open failed.\r\n"));
		goto cleanup;
	}

	if (getCduVersion(&cstNum, cstIndex, country, &cstType) == 0)
	{
		NHINFO((L"CDU Country: %s\r\n", country));
		result = cdu_country_to_territory(country, countryCode);
	}
	else 
	{
		NHERROR((L"CDU version poll failed.\r\n"));
	}

	closeCdu();

cleanup:
	::FreeLibrary(cduDiagnosticsDll);

	return result;
}

bool CUP_Emergency::GetCountryCodeFromNvram(CString &countryCode)
{
	CString currencyId;
	bool result = false;

	if (!nvramOk)
	{
		goto cleanup;
	}

	currencyId = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID);
	if (currencyId.IsEmpty())
	{
		goto cleanup;
	}

	if (currencyId == L"USD")
	{
		countryCode = L"US";
	}
	else if (currencyId == L"CAD")
	{
		countryCode = L"CAN";
	}
	else if (currencyId == L"MXN")
	{
		countryCode = L"MEX";
	}

	result = true;

cleanup:
	
	return result;
}