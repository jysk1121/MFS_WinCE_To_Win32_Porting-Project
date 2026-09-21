#pragma once

#include "stdafx.h"
#include "Dev\DevDefine.h"
#include "Common\VATConfigurationManager.h"

// // [RWC6-676] Start SKKim 2024.04.05 added new files

#define TANGOPAY_CONFIG_INI		ATM2_PATH L"\\Data\\TangoPayConfig.ini"
#define TANGOPAY_PUBLIC_KEY		ATM2_PATH L"\\Data\\TangoPayPublicKey.dat"

/**
 * A data structure containing configuration information pertaining to TangoPay
 */
typedef struct _tangopay_configuration
{
	bool	bEnable;
	CString strPartnerID;

	CString strIdentifyServerURL;
	int		nIdentifyPort;

	CString strTransactionServerURL;
	int		nTransactionPort;

	CString strApiKey;
	CString strPublicKey;
	CString strPK_CheckSum;

	CString strDownloadAppInfo;

	_tangopay_configuration::_tangopay_configuration()
	{
		bEnable = false;
		strPartnerID.Empty();

		strIdentifyServerURL.Empty();
		nIdentifyPort = 0;

		strTransactionServerURL.Empty();
		nTransactionPort = 0;

		strApiKey.Empty();
		strPublicKey.Empty();
		strPK_CheckSum.Empty();
		strDownloadAppInfo.Empty();
	}

} TangoPayConfiguration, *LPTangoPayConfiguration;

class AFX_EXT_CLASS CTangoPayConfigurationManager : private CVATConfigurationManager
{
public:
	CTangoPayConfigurationManager(void);
	~CTangoPayConfigurationManager(void);

	void CheckConfigData();

	/**
	 * Retrieves the configuration options
	 * @param config[out] A pointer to a memory location to hold the configuration information
	 * @returns true if the operation was successful
	 */
	bool GetConfiguration(TangoPayConfiguration &config) const;

	/**
	 * Saves the configuration to the store
	 * @param config[in] A configuration object
	 * @retuns true if the operation was successful
	 */
	bool SaveConfiguration(const TangoPayConfiguration &config) const;

private:
	bool LoadConfigurationFromIni(char *filename, TangoPayConfiguration &config) const;
	bool SaveConfigurationToIni(char *filename, const TangoPayConfiguration &config) const;
};