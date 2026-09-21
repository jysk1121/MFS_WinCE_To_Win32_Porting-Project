#pragma once
#include "UpdateTypes.h"

// Default sources file
#ifdef UNDER_CE
#  define UPDATE_SOURCES	"\\ATM2\\update-sources.up"
#  define WUPDATE_SOURCES	L"\\ATM2\\update-sources.up"
#else
#  define UPDATE_SOURCES	 ".\\update-sources.up"
#  define WUPDATE_SOURCES	 L".\\update-sources.up"
#endif

typedef int CONFIGERR;
const CONFIGERR ERR_OK				= 0;
const CONFIGERR ERR_SIGVALIDATION	= 1;
const CONFIGERR ERR_NOFILE			= 2;
const CONFIGERR ERR_MISSINGVALUES	= 3;
const CONFIGERR ERR_NOSIGFILE		= 4;
const CONFIGERR ERR_TID_BAD			= 5;

/**
 * Manages the configuration of the update repository service. This class is responsible for
 * reading and validating the update-sources.def provisioning file.
 */
class AFX_CLASS_EXPORT CUpdateRepoConfigurationManager
{
public:
	CUpdateRepoConfigurationManager(void);
	~CUpdateRepoConfigurationManager(void);

	/**
	 * Returns the terminal's active update repository configuration.
	 * @param[out] config the configuration data
	 * @returns a config error
	 */
	CONFIGERR GetConfiguration(UpdateSourceInfo *config);

	/**
	 * Installs the default configuration files if no runtime config is configured
	 */
	static void InstallDefaultConfigurationFiles();

private:
	/**
	 * Validates the file contents and sets validatedData to a new pointer with a null-terminated
	 * char array of the validated pkcs7 contents
	 * @param[in] filename the filename to read
	 * @param[out] validatedData a null-terminated string of the validated configuration file. Pointer is created with `new`
	 */
	bool ValidateUpdateSourcesList(CString filename, char **validatedData);

	bool DeserializeConfiguration(char *data, UpdateSourceInfo *config);

	static void InteroplateTemplateString(CString &input);
};
