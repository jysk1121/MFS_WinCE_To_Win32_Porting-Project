#pragma once

#include "stdafx.h"
#include "Dev\DevDefine.h"
#include "Common\VATConfigurationManager.h"

#define LIBERTYX_CONFIG_INI		ATM2_PATH L"\\Data\\LibertyX.ini"

typedef int LXHOSTPROTO;
const LXHOSTPROTO LX_STANDARD1	= 0;
const LXHOSTPROTO LX_STANDARD2	= 1;
const LXHOSTPROTO LX_STANDARD3	= 2;
const LXHOSTPROTO LX_EPS		= 3;

typedef int LXENVTYPE;
const LXENVTYPE LX_VISAII	= 0;
const LXENVTYPE LX_STANDARD	= 1;
const LXENVTYPE LX_ACK		= 2;

/**
 * A data structure containing configuration information pertaining to LibertyX
 */
typedef struct _lx_configuration {
	// General Settings
	bool	BuyBitcoinEnabled;
	bool	SellBitcoinEnabled;
	CString LocationID;

	// API Settings
	CString	Username;
	CString	Password;
	CString	APIUrl;

	// Host Settings
	CString	Host1Url;
	int		Host1Port;
	CString Host2Url;
	int		Host2Port;

	// Host Communication Settings
	bool		TlsEnabled;
	LXHOSTPROTO	HostProtocol;
	LXENVTYPE	EnvelopeType;
	CString		RoutingID;
	bool		HostDepModeEnabled;

	int HeartbeatIntervalMinutes;

	_lx_configuration::_lx_configuration()
	{
		BuyBitcoinEnabled = false;
		Host1Port = 0;
		Host2Port = false;
		TlsEnabled = false;
		HostProtocol = LX_STANDARD1;
		EnvelopeType = LX_VISAII;
		SellBitcoinEnabled = false;
		HeartbeatIntervalMinutes = 0;
	}

	bool IsEnabled() const
	{
		return BuyBitcoinEnabled || SellBitcoinEnabled;
	}

	CString GetHostProtocolName()
	{
		switch (HostProtocol)
		{
		case LX_STANDARD1:
			return L"Standard 1";
		case LX_STANDARD2:
			return L"Standard 2";
		case LX_STANDARD3:
			return L"Standard 3";
		case LX_EPS:
			return L"EPS";
		}

		return L"Unknown";	
	};

	CString GetHostEnvelopeName()
	{
		switch (EnvelopeType)
		{
		case LX_VISAII:
			return L"VISA Framed";
		case LX_STANDARD:
			return L"Standard";
		case LX_ACK:
			return L"ACK Controlled";
		}

		return L"Unknown";
	};
} LXConfiguration, *LPLXConfiguration;

class AFX_EXT_CLASS CLibertyXConfigurationManager : CVATConfigurationManager
{
public:
	CLibertyXConfigurationManager(void);
	~CLibertyXConfigurationManager(void);

	/**
	 * Retrieves the most recent configuration options
	 * @param config[out] A pointer to a memory location to hold the configuration information
	 * @returns true if the operation was successful
	 */
	bool GetConfiguration(LXConfiguration &config);

	/**
	 * Saves the configuration to the store
	 * @param config[in] A configuration object
	 * @retuns true if the operation was successful
	 */
	bool SaveConfiguration(LXConfiguration config);

	/**
	 * Determines if LibertyX is available on the ATM.
	 * @returns true if LibertyX is available
	 */
	bool IsLibertyXAvailable();

	/**
	 * Detemines the mode of operator for Key Manager given the EPP version and the status of DCC Dual Host
	 * If the EPP EP version is >= 10.x:
	 *     Display LTX key in Master key section
	 * Else:
	 *     If Dual Host is enabled:
	 *         Disable LTX Key
	 *     Else:
	 *         Remove 2ndMasterKey
	 *		   Display LTX key in DATA key section
	 */
	static void ReconfigureKeyManagerRegistrySettings();

private:
	bool LoadConfigurationFromIni(char * filename, LXConfiguration &config);
	bool SaveConfigurationToIni(char * filename, LXConfiguration config);
};