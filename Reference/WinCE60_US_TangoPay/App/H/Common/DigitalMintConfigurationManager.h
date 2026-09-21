#pragma once

#include "stdafx.h"
#include "Dev\DevDefine.h"
#include "Common\VATConfigurationManager.h"

#define DIGITALMINT_CONFIG_INI ATM2_PATH L"\\Data\\DigitalMint.ini"

/**
 * The DM transaction mode of operation
 */
typedef int DMTRANSACTIONMODE;
const DMTRANSACTIONMODE DMTXNMODE_UNREGISTERED = 0;
const DMTRANSACTIONMODE DMTXNMODE_ATM = 1;
const DMTRANSACTIONMODE DMTXNMODE_SIDECAR = 2;
const DMTRANSACTIONMODE DMTXNMODE_MAX = 3;

/**
 * A data structure containing configuration information pertaining to the DigitalMint transaction
 */
typedef struct _dm_configuration
{
	// General Settings
	bool				IsEnabled;
	DMTRANSACTIONMODE	TransactionMode;
	CString				DefaultCoin;

	// API Settings - Sidecar
	CString				SidecarAPIUrl;
	CString				SidecarAPISecret;
	CString				SidecarAPIKey;
	CString				SidecarPlatformHash;

	// API Settings - ATM
	CString				AtmAPIUrl;
	CString				AtmAPISecret;
	CString				AtmAPIKey;
	CString				AtmPlatformHash;

	_dm_configuration::_dm_configuration()
	{
		IsEnabled = false;
		TransactionMode = DMTXNMODE_UNREGISTERED;
		DefaultCoin = "";
	}

	CString GetTransactionMode() const
	{
		switch (TransactionMode)
		{
		case DMTXNMODE_UNREGISTERED:
			return L"Unregistered";
		case DMTXNMODE_ATM:
			return L"ATM";
		case DMTXNMODE_SIDECAR:
			return L"Sidecar";
		default:
			return L"Unknown";
		}
	};

	BOOL IsSidecarConfigured() const
	{
		return TransactionMode == DMTXNMODE_SIDECAR
			&& IsEnabled
			&& !SidecarAPISecret.IsEmpty()
			&& !SidecarAPIKey.IsEmpty()
			&& !SidecarPlatformHash.IsEmpty();
	};

	BOOL IsAtmConfigured() const
	{
		return (TransactionMode == DMTXNMODE_SIDECAR || TransactionMode == DMTXNMODE_ATM)
			&& IsEnabled
			&& !DefaultCoin.IsEmpty()
			&& !AtmAPISecret.IsEmpty()
			&& !AtmAPIKey.IsEmpty()
			&& !AtmPlatformHash.IsEmpty();
	};

} DMConfiguration, *LPDMConfiguration;

class AFX_EXT_CLASS CDigitalMintConfigurationManager : private CVATConfigurationManager
{
public:
	CDigitalMintConfigurationManager(void);
	~CDigitalMintConfigurationManager(void);

	/**
	 * Retrieves the most recent configuration options
	 * @param config[out] A pointer to a memory location to hold the configuration information
	 * @returns true if the operation was successful
	 */
	bool GetConfiguration(DMConfiguration &config) const;

	/**
	 * Saves the configuration to the store
	 * @param config[in] A configuration object
	 * @retuns true if the operation was successful
	 */
	bool SaveConfiguration(const DMConfiguration &config) const;

	/**
	 * Gets the API Url based on the configuration
	 * @param config[in] The current DigitalMint configuration
	 * @returns The API Url
	 */
	CString APIUrl(const DMConfiguration &config) const;

	/**
	 * Gets the API Secret based on the configuration
	 * @param config[in] The current DigitalMint configuration
	 * @returns The API Secret
	 */
	CString APISecret(const DMConfiguration &config) const;

	/**
	 * Gets the API Key based on the configuration
	 * @param config[in] The current DigitalMint configuration
	 * @returns The API Key
	 */
	CString APIKey(const DMConfiguration &config) const;

	/**
	 * Gets the Platform Hash based on the configuration
	 * @param config[in] The current DigitalMint configuration
	 * @returns The Platform Hash
	 */
	CString PlatformHash(const DMConfiguration &config) const;

private:
	bool LoadConfigurationFromIni(char *filename, DMConfiguration &config) const;
	bool SaveConfigurationToIni(char *filename, const DMConfiguration &config) const;
};