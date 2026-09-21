#pragma once

#include "stdafx.h"
#include "Dev\DevDefine.h"
#include "Common\VATConfigurationManager.h"

#define PAI_CONFIG_DIR ATM_PATH L"\\Data\\QuickAssistant\\"
#define PAI_CONFIG_INI PAI_CONFIG_DIR L"Settings.ini"

/**
 * The PAI processor options
 */
typedef int PAIPROCESSOR;
const PAIPROCESSOR PAIPROC_FIS = 0;
const PAIPROCESSOR PAIPROC_SWITCH_COMMERCE = 1;
const PAIPROCESSOR PAIPROC_1ISO = 2;
const PAIPROCESSOR PAIPROC_COLUMBUS_DATA = 3;
const PAIPROCESSOR PAIPROC_OTHER = 4;

/**
 * The PAI Communication options
 */
typedef int PAICOMMUNICATION;
const PAICOMMUNICATION PAICOMM_INTERNET = 0;
const PAICOMMUNICATION PAICOMM_DIAL_UP = 1;

/**
 * The PAI Message Format options
 */
typedef int PAIMESSAGEFORMAT;
const PAIMESSAGEFORMAT PAIMSG_STANDARD1 = 0;
const PAIMESSAGEFORMAT PAIMSG_STANDARD3 = 1;

/**
 * The PAI RMS options
 */
typedef int PAIREMOTEMONITORING;
const PAIREMOTEMONITORING PAIRMS_NONE = 0;
const PAIREMOTEMONITORING PAIRMS_WIRELESS = 1;
const PAIREMOTEMONITORING PAIRMS_INTERNET = 2;
const PAIREMOTEMONITORING PAIRMS_OTHER = 3;

/**
 * The PAI options for Enable or Disable
 */
typedef int PAIENABLEMENT;
const PAIENABLEMENT PAI_DISABLE = 0;
const PAIENABLEMENT PAI_ENABLE = 1;

/**
 * A data structure containing configuration information pertaining to the PAI quick assistant
 */
typedef struct _pai_configuration
{
	// Processor
	PAIPROCESSOR Processor;

	// Communication Method
	PAICOMMUNICATION Communication;

	// Message Format
	PAIMESSAGEFORMAT Message;

	// Remote Monitoring (RMS)
	PAIREMOTEMONITORING RMS;

	// Revenue Add Features 1
	PAIENABLEMENT DualBalance;
	PAIENABLEMENT MastercardCashPickup;
	PAIENABLEMENT Popmoney;
	PAIENABLEMENT FIS_DCC;

	// Revenue Add Features 2
	PAIENABLEMENT Bitload4U;

	_pai_configuration::_pai_configuration()
	{
		Processor = PAIPROC_FIS;
		Communication = PAICOMM_INTERNET;
		Message = PAIMSG_STANDARD1;
		RMS = PAIRMS_WIRELESS;
		DualBalance = PAI_DISABLE;
		MastercardCashPickup = PAI_DISABLE;
		Popmoney = PAI_DISABLE;
		FIS_DCC = PAI_DISABLE;
		Bitload4U = PAI_DISABLE;
	}

} PAIConfiguration, *LPPAIConfiguration;

class AFX_EXT_CLASS CPAIConfigurationManager : private CVATConfigurationManager
{
public:
	CPAIConfigurationManager(void);
	~CPAIConfigurationManager(void);

	/**
	 * Retrieves the configuration options
	 * @param config[out] A pointer to a memory location to hold the configuration information
	 * @returns true if the operation was successful
	 */
	bool GetConfiguration(PAIConfiguration &config) const;

	/**
	 * Saves the configuration to the store
	 * @param config[in] A configuration object
	 * @retuns true if the operation was successful
	 */
	bool SaveConfiguration(const PAIConfiguration &config) const;

	/**
	 * Gets the Processor based on the configuration
	 * @param config[in] The PAI configuration
	 * @returns The Processor
	 */
	CString GetProcessor(const PAIConfiguration &config) const;

	/**
	 * Gets the Message Format based on the configuration
	 * @param config[in] The PAI configuration
	 * @returns The Message Format
	 */
	CString GetMessageFormat(const PAIConfiguration &config) const;

	/**
	 * Gets the RMS based on the configuration
	 * @param config[in] The PAI configuration
	 * @returns The RMS
	 */
	CString GetRMS(const PAIConfiguration &config) const;

private:
	bool LoadConfigurationFromIni(char *filename, PAIConfiguration &config) const;
	bool SaveConfigurationToIni(char *filename, const PAIConfiguration &config) const;
};