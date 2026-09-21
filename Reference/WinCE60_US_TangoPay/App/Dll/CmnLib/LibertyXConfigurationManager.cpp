#include "stdafx.h"
#include ".\Common\CmnLib.h"
#include ".\Common\LibertyXConfigurationManager.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

#define LIBERTYX_OVERRIDE_FILE	ATM_PATH L"\\Data\\LibertyX.ini"
#define LIBERTYX_DEFAULT_CONFIG	ATM_PATH L"\\Data\\LibertyX_Default.ini"

#define DEFAULT_HEARTBEAT_INTERVAL 60


CLibertyXConfigurationManager::CLibertyXConfigurationManager(void)
{
}

CLibertyXConfigurationManager::~CLibertyXConfigurationManager(void)
{
}

bool CLibertyXConfigurationManager::GetConfiguration(LXConfiguration &config)
{
	if (MergeDefaultOptionsToConfig(LIBERTYX_DEFAULT_CONFIG, LIBERTYX_CONFIG_INI, LIBERTYX_OVERRIDE_FILE))
	{
		CStringA filename(LIBERTYX_CONFIG_INI);
		return LoadConfigurationFromIni(filename.GetBuffer(), config);
	}

	NVDump('F', 'C', "00", L"LX_CONF", L"NG_0");
	return false;
}

bool CLibertyXConfigurationManager::SaveConfiguration(LXConfiguration config)
{
	CStringA filename(LIBERTYX_CONFIG_INI);
	return SaveConfigurationToIni(filename.GetBuffer(), config);
}

bool CLibertyXConfigurationManager::IsLibertyXAvailable()
{
	return true;
}

/**
 * Detemines the mode of operator for Key Manager given the EPP version and the status of DCC Dual Host
 * If the EPP EP version is >= 10.x:
 *     Display LTX key in Master key section
 * Else:
 *     If Dual Host is enabled:
 *         Disable LTX Key
 *     Else:
 *         Remove 2ndMasterKey
 *		   Display LTX key in PIN key section
 */
void CLibertyXConfigurationManager::ReconfigureKeyManagerRegistrySettings()
{
#if (APP_LIBERTYX)
	const int pci3MajorVersion = Asc2Int(CString(PIN_PCI30_EP_VERSION).Mid(1, 2));
	const CString strEPPFWVer = MemGetVersion(_MEMKEY_EPVERSION, L"PIN");
	const int strEPPMajorVer = Asc2Int(strEPPFWVer.Mid(1, 2));

	// NB: There is a bug in the EP V8.21 that prevents multiple keys of the same
	// usage from being configured.  Only EP V10 or greater can support this feature.
	bool eppSupportsMultiKey = strEPPMajorVer >= pci3MajorVersion;

	if (eppSupportsMultiKey)
	{
		// Add the LTX key to the Master key button set
		RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"TripleKeyName",	T_TMK_LTX_BUTTON);		

		// Enable the DCC Dual host key in PIN buttons
		RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"PINKeyName3",	T_PINMK_BUTTON);
	}
	else
	{
		// Disable the LTX Master key. It is not supported with old EPP EP versions
		RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"TripleKeyName",	T_TMK_BUTTON);

		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) == ENABLE)
		{
			// Dual host is enabled, so we should disable the LTX PIN key and enable the DCC PIN key
			RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"PINKeyName3",	T_PINMK_BUTTON);
		}
		else
		{
			// Dual host is disabled, so we should disable the DCC PIN key and enable the LTX PIN key
			RegSetStr(L"Software\\ATM\\DevInfo\\KeyMgr", L"PINKeyName3",	LIBERTYXKEY_NAME);
		}
	}
#endif
}

bool CLibertyXConfigurationManager::LoadConfigurationFromIni(char * filename, LXConfiguration &config)
{
	char* buffer = NULL;
	bool failed = false;
	int res = 0;

	// General Settings
	res = GetIniString(filename, "General", "location_id", &buffer);
	failed |= res == -1;
	if (res == 1)
	{
		config.LocationID = CString(buffer);
		delete [] buffer;
	}

	res = GetIniString(filename, "General", "enabled", &buffer);
	failed |= res == -1;
	if (res == 1)
	{
		config.BuyBitcoinEnabled = strcmp(buffer, "true") == 0;
		delete [] buffer;
	}

	// Host Communication Settings
	res = GetIniString(filename, "HostComm", "tls", &buffer);
	failed |= res == -1;
	if (res == 1)
	{
		config.TlsEnabled = strcmp("true", buffer) == 0 ? true : false;
		delete [] buffer;
	}

	res = GetIniString(filename, "HostComm", "host_proto", &buffer);
	failed |= res == -1;
	if (res == 1)
	{
		config.HostProtocol = atoi(buffer);
		delete [] buffer;
	}

	res = GetIniString(filename, "HostComm", "routing_id", &buffer);
	failed |= res == -1;
	if (res == 1)
	{
		config.RoutingID = CString(buffer);
		delete [] buffer;
	}

	res = GetIniString(filename, "HostComm", "env_type", &buffer);
	failed |= res == -1;
	if (res == 1)
	{
		config.EnvelopeType = atoi(buffer);
		delete [] buffer;
	}

	res = GetIniString(filename, "HostComm", "host_dependent_mode", &buffer);
	if (res == 1)
	{
		config.HostDepModeEnabled = strcmp("true", buffer) == 0 ? true : false;
		delete [] buffer;
	}

	// Host 1 Settings
	res = GetIniString(filename, "Host1", "url", &buffer);
	failed |= res == -1;
	if (res == 1)
	{
		config.Host1Url = CString(buffer);
		delete [] buffer;
	}

	res = GetIniString(filename, "Host1", "port", &buffer);
	failed |= res == -1;
	if (res == 1)
	{
		config.Host1Port = atoi(buffer);
		delete [] buffer;
	}

	// Host 2 Settings
	res = GetIniString(filename, "Host2", "url", &buffer);
	failed |= res == -1;
	if (res == 1)
	{
		config.Host2Url = CString(buffer);
		delete [] buffer;
	}

	res = GetIniString(filename, "Host2", "port", &buffer);
	failed |= res == -1;
	if (res == 1)
	{
		config.Host2Port = atoi(buffer);
		delete [] buffer;
	}

	// API Settings
	res = GetIniString(filename, "API", "url", &buffer);
	failed |= res == -1;
	if (res == 1)
	{
		config.APIUrl = CString(buffer);
		delete [] buffer;
	}

	res = GetIniString(filename, "API", "username", &buffer);
	failed |= res == -1;
	if (res == 1)
	{
		config.Username = CString(buffer);
		delete [] buffer;
	}

	res = GetIniString(filename, "API", "password", &buffer);
	failed |= res == -1;
	if (res == 1)
	{
		config.Password = CString(buffer);
		delete [] buffer;
	}

	res = GetIniString(filename, "API", "heartbeat_interval", &buffer);
	if (res == 1)
	{
		config.HeartbeatIntervalMinutes = atoi(buffer);
		delete [] buffer;
	}

	if (config.HeartbeatIntervalMinutes <= 0)
	{
		config.HeartbeatIntervalMinutes = DEFAULT_HEARTBEAT_INTERVAL;
	}

	// Cash Out Settings
	res = GetIniString(filename, "CashOut", "enabled", &buffer);
	if (res == 1)
	{
		config.SellBitcoinEnabled = strcmp(buffer, "true") == 0;
		delete [] buffer;
	}

	//
	// Forced Migrations
	//

	// 2020-11-03 Update CDS IP to a hostname, if present in 6.2.4 (removing disabling of cert validation)
	if (config.Host1Url == L"208.35.209.1")
	{
		NHDBG((L"Updating host 1 to CDS hostname\r\n"));
		config.Host1Url = L"atm.columbusdata.net";
	}

	if (config.Host2Url == L"208.35.209.1")
	{
		NHDBG((L"Updating host 1 to CDS hostname\r\n"));
		config.Host2Url = L"atm.columbusdata.net";
	}

	return !failed;
}

bool CLibertyXConfigurationManager::SaveConfigurationToIni(char * filename, LXConfiguration config)
{
	char buffer[256] = {};

	// General Settings
	WideToMulti(buffer, config.LocationID, config.LocationID.GetLength() + 1);
	SetIniString(filename, "General", "location_id", buffer);
	memset(buffer, 0, sizeof(buffer));

	SetIniString(filename, "General", "enabled", config.BuyBitcoinEnabled ? "true" : "false");

	// Host Communication Settings
	SetIniString(filename, "HostComm", "tls", config.TlsEnabled ? "true" : "false");

	sprintf_s(buffer, sizeof(buffer), "%d", config.HostProtocol);
	SetIniString(filename, "HostComm", "host_proto", buffer);
	memset(buffer, 0, sizeof(buffer));

	WideToMulti(buffer, config.RoutingID, config.RoutingID.GetLength() + 1);
	SetIniString(filename, "HostComm", "routing_id", buffer);
	memset(buffer, 0, sizeof(buffer));

	sprintf_s(buffer, sizeof(buffer), "%d", config.EnvelopeType);
	SetIniString(filename, "HostComm", "env_type", buffer);
	memset(buffer, 0, sizeof(buffer));

	// Host 1 Settings
	WideToMulti(buffer, config.Host1Url, config.Host1Url.GetLength() + 1);
	SetIniString(filename, "Host1", "url", buffer);
	memset(buffer, 0, sizeof(buffer));

	sprintf_s(buffer, sizeof(buffer), "%d", config.Host1Port);
	SetIniString(filename, "Host1", "port", buffer);
	memset(buffer, 0, sizeof(buffer));

	// Host 2 Settings
	WideToMulti(buffer, config.Host2Url, config.Host2Url.GetLength() + 1);
	SetIniString(filename, "Host2", "url", buffer);
	memset(buffer, 0, sizeof(buffer));

	sprintf_s(buffer, sizeof(buffer), "%d", config.Host2Port);
	SetIniString(filename, "Host2", "port", buffer);
	memset(buffer, 0, sizeof(buffer));

	// API Settings
	WideToMulti(buffer, config.APIUrl, config.APIUrl.GetLength() + 1);
	SetIniString(filename, "API", "url", buffer);
	memset(buffer, 0, sizeof(buffer));

	WideToMulti(buffer, config.Username, config.Username.GetLength() + 1);
	SetIniString(filename, "API", "username", buffer);
	memset(buffer, 0, sizeof(buffer));

	WideToMulti(buffer, config.Password, config.Password.GetLength() + 1);
	SetIniString(filename, "API", "password", buffer);
	memset(buffer, 0, sizeof(buffer));

	sprintf_s(buffer, sizeof(buffer), "%d", config.HeartbeatIntervalMinutes);
	SetIniString(filename, "API", "heartbeat_interval", buffer);
	memset(buffer, 0, sizeof(buffer));

	// Cash Out Settings
	SetIniString(filename, "CashOut", "enabled", config.SellBitcoinEnabled ? "true" : "false");

	return true;
}