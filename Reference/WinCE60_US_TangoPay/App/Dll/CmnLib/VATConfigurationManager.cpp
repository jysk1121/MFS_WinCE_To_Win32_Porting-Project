#include "StdAfx.h"
#include ".\Common\CmnLib.h"
#include ".\Common\VATConfigurationManager.h"

CVATConfigurationManager::CVATConfigurationManager()
{

}

CVATConfigurationManager::~CVATConfigurationManager()
{

}

bool CVATConfigurationManager::MergeDefaultOptionsToConfig(CString defaultFile, CString configFile, CString overrideFile) const
{
	bool defaultFileExists = FILE_EXISTS(defaultFile);
	bool configFileExists = FILE_EXISTS(configFile);
	bool overrideFileExists = FILE_EXISTS(overrideFile);

	/**
	 * Notes:
	 * 1. If LIBERTYX_OVERRIDE_FILE exists, copy it over the LIBERTYX_CONFIG_INI
	 * 2. If LIBERTYX_CONFIG_INI does not exist, copy LIBERTYX_DEFAULT_CONFIG to LIBERTYX_CONFIG_INI
	 * 
	 * This process allows for defaults and overrides.
	 */

	// Copy the override file over the existing config
	if (overrideFileExists)
	{
		CopyFile(overrideFile, configFile, false);
		return !!DeleteFile(overrideFile);
	}

	// Copy default file if the running config doesn't exist
	if (!configFileExists && defaultFileExists)
	{
		return !!CopyFile(defaultFile, configFile, false);
	}

	return true;
}