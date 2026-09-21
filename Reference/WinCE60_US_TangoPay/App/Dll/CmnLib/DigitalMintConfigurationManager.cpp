#include "stdafx.h"
#include ".\Common\CmnLib.h"
#include ".\Common\DigitalMintConfigurationManager.h"
#include ".\Common\NHDbgApi.h"

#define DIGITALMINT_OVERRIDE_FILE	ATM_PATH L"\\DATA\\DigitalMint.ini"
#define DIGITALMINT_DEFAULT_CONFIG	ATM_PATH L"\\DATA\\DigitalMint_Default.ini"

CDigitalMintConfigurationManager::CDigitalMintConfigurationManager(void)
{
}

CDigitalMintConfigurationManager::~CDigitalMintConfigurationManager(void)
{
}

bool CDigitalMintConfigurationManager::GetConfiguration(DMConfiguration &config) const
{
	if (MergeDefaultOptionsToConfig(DIGITALMINT_DEFAULT_CONFIG, DIGITALMINT_CONFIG_INI, DIGITALMINT_OVERRIDE_FILE))
	{
		CStringA filename(DIGITALMINT_CONFIG_INI);
		return LoadConfigurationFromIni(filename.GetBuffer(), config);
	}

	NVDump('F', 'C', "1O", L"DM_CONF", L"NG_0");
	return false;
}

bool CDigitalMintConfigurationManager::SaveConfiguration(const DMConfiguration &config) const
{
	CStringA filename(DIGITALMINT_CONFIG_INI);
	return SaveConfigurationToIni(filename.GetBuffer(), config);
}

//
// Private Functions
//

bool CDigitalMintConfigurationManager::LoadConfigurationFromIni(char *filename, DMConfiguration &config) const
{
	char *buffer = NULL;
	bool failed = false;
	int res = 0;

	// General Settings
	res = GetIniString(filename, "General", "enabled", &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.IsEnabled = strcmp(buffer, "true") == 0;
		delete[] buffer;
	}

	res = GetIniString(filename, "General", "txn_mode", &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.TransactionMode = (DMTRANSACTIONMODE)atoi(buffer);
		delete[] buffer;

		if (config.TransactionMode >= DMTXNMODE_MAX || config.TransactionMode < DMTXNMODE_UNREGISTERED)
		{
			config.TransactionMode = DMTXNMODE_UNREGISTERED;
		}
	}

	res = GetIniString(filename, "General", "default_coin", &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.DefaultCoin = CString(buffer);
		delete[] buffer;
	}

	// API Settings - Sidecar
	res = GetIniString(filename, "Sidecar", "url", &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.SidecarAPIUrl = CString(buffer);
		delete[] buffer;
	}

	res = GetIniString(filename, "Sidecar", "secret", &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.SidecarAPISecret = CString(buffer);
		delete[] buffer;
	}

	res = GetIniString(filename, "Sidecar", "key", &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.SidecarAPIKey = CString(buffer);
		delete[] buffer;
	}

	res = GetIniString(filename, "Sidecar", "platform_hash", &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.SidecarPlatformHash = CString(buffer);
		delete[] buffer;
	}

	// API Settings - ATM
	res = GetIniString(filename, "ATM", "url", &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.AtmAPIUrl = CString(buffer);
		delete[] buffer;
	}

	res = GetIniString(filename, "ATM", "secret", &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.AtmAPISecret = CString(buffer);
		delete[] buffer;
	}

	res = GetIniString(filename, "ATM", "key", &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.AtmAPIKey = CString(buffer);
		delete[] buffer;
	}

	res = GetIniString(filename, "ATM", "platform_hash", &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.AtmPlatformHash = CString(buffer);
		delete[] buffer;
	}

	return !failed;
}

bool CDigitalMintConfigurationManager::SaveConfigurationToIni(char *filename, const DMConfiguration &config) const
{
	char buffer[256] = {};

	// General Settings
	SetIniString(filename, "General", "enabled", config.IsEnabled ? "true" : "false");

	sprintf_s(buffer, sizeof(buffer), "%d", config.TransactionMode);
	SetIniString(filename, "General", "txn_mode", buffer);
	memset(buffer, 0, sizeof(buffer));

	WideToMulti(buffer, config.DefaultCoin, config.DefaultCoin.GetLength() + 1);
	SetIniString(filename, "General", "default_coin", buffer);
	memset(buffer, 0, sizeof(buffer));

	// API Settings - Sidecar
	WideToMulti(buffer, config.SidecarAPIUrl, config.SidecarAPIUrl.GetLength() + 1);
	SetIniString(filename, "Sidecar", "url", buffer);
	memset(buffer, 0, sizeof(buffer));

	WideToMulti(buffer, config.SidecarAPIKey, config.SidecarAPIKey.GetLength() + 1);
	SetIniString(filename, "Sidecar", "key", buffer);
	memset(buffer, 0, sizeof(buffer));

	WideToMulti(buffer, config.SidecarAPISecret, config.SidecarAPISecret.GetLength() + 1);
	SetIniString(filename, "Sidecar", "secret", buffer);
	memset(buffer, 0, sizeof(buffer));

	WideToMulti(buffer, config.SidecarPlatformHash, config.SidecarPlatformHash.GetLength() + 1);
	SetIniString(filename, "Sidecar", "platform_hash", buffer);
	memset(buffer, 0, sizeof(buffer));

	// API Settings - ATM
	WideToMulti(buffer, config.AtmAPIUrl, config.AtmAPIUrl.GetLength() + 1);
	SetIniString(filename, "ATM", "url", buffer);
	memset(buffer, 0, sizeof(buffer));

	WideToMulti(buffer, config.AtmAPIKey, config.AtmAPIKey.GetLength() + 1);
	SetIniString(filename, "ATM", "key", buffer);
	memset(buffer, 0, sizeof(buffer));

	WideToMulti(buffer, config.AtmAPISecret, config.AtmAPISecret.GetLength() + 1);
	SetIniString(filename, "ATM", "secret", buffer);
	memset(buffer, 0, sizeof(buffer));

	WideToMulti(buffer, config.AtmPlatformHash, config.AtmPlatformHash.GetLength() + 1);
	SetIniString(filename, "ATM", "platform_hash", buffer);
	memset(buffer, 0, sizeof(buffer));

	return true;
}

CString CDigitalMintConfigurationManager::APIUrl(const DMConfiguration &config) const
{
	if (config.TransactionMode == DMTXNMODE_SIDECAR)
	{
		NHDBG((L"Sidecar API Url: %s%s", config.SidecarAPIUrl, SCR_CMD_DELIMITER));
		return config.SidecarAPIUrl;
	}
	else if (config.TransactionMode == DMTXNMODE_ATM)
	{
		NHDBG((L"ATM API Url: %s%s", config.AtmAPIUrl, SCR_CMD_DELIMITER));
		return config.AtmAPIUrl;
	}
	else
	{
		NHDBG((L"API Url Empty!%s", SCR_CMD_DELIMITER));
		return L"";
	}
}

CString CDigitalMintConfigurationManager::APISecret(const DMConfiguration &config) const
{
	if (config.TransactionMode == DMTXNMODE_SIDECAR)
	{
		NHDBG((L"Sidecar API Secret: %s%s", config.SidecarAPISecret, SCR_CMD_DELIMITER));
		return config.SidecarAPISecret;
	}
	else if (config.TransactionMode == DMTXNMODE_ATM)
	{
		NHDBG((L"ATM API Secret: %s%s", config.AtmAPISecret, SCR_CMD_DELIMITER));
		return config.AtmAPISecret;
	}
	else
	{
		NHDBG((L"API Secret Empty!%s", SCR_CMD_DELIMITER));
		return L"";
	}
}

CString CDigitalMintConfigurationManager::APIKey(const DMConfiguration &config) const
{
	if (config.TransactionMode == DMTXNMODE_SIDECAR)
	{
		NHDBG((L"Sidecar API Key: %s%s", config.SidecarAPIKey, SCR_CMD_DELIMITER));
		return config.SidecarAPIKey;
	}
	else if (config.TransactionMode == DMTXNMODE_ATM)
	{
		NHDBG((L"ATM API Key: %s%s", config.AtmAPIKey, SCR_CMD_DELIMITER));
		return config.AtmAPIKey;
	}
	else
	{
		NHDBG((L"API Key Empty!%s", SCR_CMD_DELIMITER));
		return L"";
	}
}

CString CDigitalMintConfigurationManager::PlatformHash(const DMConfiguration &config) const
{
	if (config.TransactionMode == DMTXNMODE_SIDECAR)
	{
		NHDBG((L"Sidecar Platform Hash: %s%s", config.SidecarPlatformHash, SCR_CMD_DELIMITER));
		return config.SidecarPlatformHash;
	}
	else if (config.TransactionMode == DMTXNMODE_ATM)
	{
		NHDBG((L"ATM Platform Hash: %s%s", config.AtmPlatformHash, SCR_CMD_DELIMITER));
		return config.AtmPlatformHash;
	}
	else
	{
		NHDBG((L"API Platform Hash Empty!%s", SCR_CMD_DELIMITER));
		return L"";
	}
}