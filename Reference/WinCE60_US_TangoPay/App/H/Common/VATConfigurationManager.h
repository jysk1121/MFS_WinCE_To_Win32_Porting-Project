#pragma once

#include "StdAfx.h"

/**
 * A base class for a standardized configuration manager for value-added transactions.
 */
class AFX_EXT_CLASS CVATConfigurationManager
{
public:
	CVATConfigurationManager(void);
	~CVATConfigurationManager(void);

protected:
	/**
	 * Merges the config files based on the presence of default, override, or base configuration files
	 * @param defaultFile[in] The configuration file used as the default configuration when no settings are set
	 * @param configFile[in] The primary settings file
	 * @param overrideFile[in] The file which overrides all settings in the primary settings
	 * @returns true when the operation is successful
	 */
	bool MergeDefaultOptionsToConfig(CString defaultFile, CString configFile, CString overrideFile) const;
};