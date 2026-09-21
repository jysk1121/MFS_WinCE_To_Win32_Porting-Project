#include "stdafx.h"
#include ".\Common\NHDbgApi.h"
#include "UpdateRepoConfigurationManager.h"
#include ".\Common\Crypto.h"
#include ".\Dll\NHReadiniFile.h"

#include <openssl/pkcs7.h>
#include <openssl/x509v3.h>

#if (US_VERSION)
#  define COUNTRY_CODE L"US"
#elif (MX_VERSION)
#  define COUNTRY_CODE L"MX"
#elif (CA_VERSION)
#  define COUNTRY_CODE L"CA"
#elif (AU_VERSION)
#  define COUNTRY_CODE L"AU"
#else
#  define COUNTRY_CODE L"US"
#endif

#define NVERR(func, err) NVDump('F', 'V', func, L"", err);
#define NVINF(func, msg) NVDump('O', 'V', func, L"", msg);

// [Win32-fixes] US ryan.payton 2023.01.19 Win32 not seeing AP_TYPE from BaseDef_US.h
#ifndef UNDER_CE
	// ----------------------------------------------------------------------------
	//////////////////////////// Versions /////////////////////////////////////////
	// ----------------------------------------------------------------------------
	// [#2291] Available Versions in US
	//	V06.XX.XX		: Generic NH Version
	//	C06.XX.XX		: Cardtronics Version
	//	P06.XX.XX		: PAI Version
	//	T06.XX.XX		: DEMO Version
    //  D06.XX.XX       : DYNAMICDCC   RWC6-645
	#ifdef APP_LOCAL_MODE
		#define AP_TYPE _T("T")
		#define APCODE "T\0"
	#elif EMV_TEST_MODE
		#define AP_TYPE _T("X")
		#define APCODE "X\0"
	#else
		#if (APP_CUSTOM_DYNAMICDCC)  // RWC6-645 DYNAMICDCC
			#define AP_TYPE _T("D")
			#define APCODE "D\0"
			#undef APP_CUSTOM_CASHDEPOT
			#define APP_CUSTOM_CASHDEPOT  1
		#elif (APP_CUSTOM_CASHDEPOT)
			#define AP_TYPE _T("B")
			#define APCODE "B\0"
		#elif (APP_CUSTOM_CARDTRONICS)
			#define AP_TYPE _T("C")
			#define APCODE "C\0"
		#elif (APP_CUSTOM_PAI)
			#define AP_TYPE _T("P")
			#define APCODE "P\0"
		#else
			#define AP_TYPE _T("V")
			#define APCODE "V\0"
		#endif
	#endif
	// End of [#2291]
#endif
// End of [Win32-fixes]

CUpdateRepoConfigurationManager::CUpdateRepoConfigurationManager(void)
{
}

CUpdateRepoConfigurationManager::~CUpdateRepoConfigurationManager(void)
{
}

/**
 * Returns the terminal's active update repository configuration.
 * @param[out] config the configuration data
 * @returns a config error
 */
CONFIGERR CUpdateRepoConfigurationManager::GetConfiguration(UpdateSourceInfo *config)
{
	CString tid;
	CONFIGERR res = ERR_OK;
	char *configData = NULL;

	NVINF("00", L"Open Config");

	if (!FILE_EXISTS(WUPDATE_SOURCES))
	{
		NHERROR((L"No sources file found\r\n"));
		NVERR("00", L"NG0");
		res = ERR_NOFILE;
		goto cleanup;
	}

#ifndef NO_VALIDATION
	if (!ValidateUpdateSourcesList(WUPDATE_SOURCES, &configData))
	{
		NHERROR((L"Source file signature is invalid\r\n"));
		NVERR("00", L"NG1");
		res = ERR_SIGVALIDATION;
		goto cleanup;
	}
#endif

	config->Clear();

	if (!DeserializeConfiguration(configData, config))
	{
		NHERROR((L"Deserialization error\r\n"));
		NVERR("00", L"NG2");
		res = ERR_MISSINGVALUES;
		goto cleanup;
	}

	// Terminal ID must match the value in the config (or it can be blank)
	if (!config->TerminalID.IsEmpty())
	{
#ifdef UNDER_CE
		tid = RegGetStr(_T("SOFTWARE\\ATM\\APP"), _T("TerminalID"), L"");
		if (tid.IsEmpty() && CheckNVRam())
		{
			tid = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
		}
		tid.Trim();
#else
		tid = L"test";
#endif

		// If the TID is empty, then the config applies for all TIDs
		if (config->TerminalID != tid)
		{
			NVERR("00", L"NG3");
			res = ERR_TID_BAD;
			goto cleanup;
		}
	}

cleanup:

	if (configData != NULL)
	{
		memset(configData, 0, strlen(configData)); // This is a zero-terminated string
		delete [] configData;
		configData = NULL;
	}

	return res;
}

void CUpdateRepoConfigurationManager::InstallDefaultConfigurationFiles()
{
	// [Win32-fixes] US ryan.payton 2023.01.19 Fix for Win32 ATM mount directory
	CString defaultSource, updateSource;

#ifdef UNDER_CE
	defaultSource.Format(L"%s%s", L"\\ATM", L"\\update-sources.up.default");
	updateSource.Format(L"%s%s", L"\\ATM", L"\\update-sources.up");
#else
	defaultSource.Format(L"%s%s", L".\\ATM", L"\\update-sources.up.default");
	updateSource.Format(L"%s%s", L".\\ATM", L"\\update-sources.up");
#endif
	// End of [Win32-fixes]

	// Move update configs from /ATM to /ATM2, if present and config files do not exist
	bool defaultsExist = !!IsExistFile(defaultSource);
	bool newConfigsExist = !!IsExistFile(updateSource);
	bool configsExist = !!IsExistFile(WUPDATE_SOURCES);

	if (defaultsExist && !configsExist)
	{
		CopyFile(defaultSource, WUPDATE_SOURCES, FALSE);
		NHDBG((L"New update source files installed\r\n"));
	}
	else if (newConfigsExist)
	{
		CopyFile(updateSource, WUPDATE_SOURCES, FALSE);
		DeleteFile(updateSource);
		NHDBG((L"New update source files installed\r\n"));
	}
}

bool CUpdateRepoConfigurationManager::ValidateUpdateSourcesList(CString filename, char **validatedData)
{
	bool result = false;
	char *fileContents = NULL;
	int sourceLen = 0;
	STACK_OF(X509) *certs = NULL;
	X509_VERIFY_PARAM* verifyParams = NULL;
	X509_STORE* caStore = X509_STORE_new();
	BIO* pkcs7Data = BIO_new(BIO_s_mem());
	BIO* embeddedData = BIO_new(BIO_s_mem());
	PKCS7* pkcs7 = NULL;
	BUF_MEM *memptr = NULL;

	*validatedData = NULL;
	fileContents = (char *)CryptoLib::GetFileContents(filename, NULL, 0, sourceLen);

	// Get trusted certs
	if (!CryptoLib::GetCertificatesFromKeyStore(GetCodeSigningKeyStoreFilePath(), &certs))
	{
		NHERROR((L"Error getting trust store\r\n"));
		NVERR("01", L"NG1");
		goto cleanup;
	}

	verifyParams = X509_STORE_get0_param(caStore);
	X509_VERIFY_PARAM_set_purpose(verifyParams, X509_PURPOSE_ANY);
	X509_STORE_add_certs(caStore, certs); // Load CA trust

	// Load PKCS7 data from web response into the BIO for PEM parsing
	BIO_write(pkcs7Data, fileContents, sourceLen);
	BIO_seek(pkcs7Data, 0);

	pkcs7 = PEM_read_bio_PKCS7(pkcs7Data, NULL, NULL, NULL);
	if (!pkcs7)
	{
		NHERROR((L"Error parsing pkcs7 data\r\n"));
		NVERR("01", L"NG2");
		goto cleanup;
	}

	result = PKCS7_verify_one_chain(pkcs7, caStore, NULL, embeddedData) != 0;
	if (!result)
	{
		NVERR("01", L"NG3");
		goto cleanup;
	}

	// Contents are verified. Now read the embedded data into the out param
	BIO_seek(embeddedData, 0);
	BIO_get_mem_ptr(embeddedData, &memptr);
	*validatedData = new char[memptr->length + 1]();
	BIO_read(embeddedData, *validatedData, memptr->length);

cleanup:

	if (fileContents != NULL)
	{
		memset(fileContents, 0, sourceLen);
		delete [] fileContents;
		fileContents = NULL;
	}

	if (caStore != NULL)
	{
		X509_STORE_free(caStore);
		caStore = NULL;
	}

	if (certs != NULL)
	{
		sk_X509_pop_free(certs, X509_free);
		certs = NULL;
	}

	if (pkcs7Data != NULL)
	{
		BIO_free(pkcs7Data);
		pkcs7Data = NULL;
	}

	if (embeddedData != NULL)
	{
		BIO_free(pkcs7Data);
		pkcs7Data = NULL;
	}

	return result;
}

bool CUpdateRepoConfigurationManager::DeserializeConfiguration(char *data, UpdateSourceInfo *config)
{
	BYTE parsedFlags = 0x00;
	BYTE requiredFlags = 0x0B;
	PINIVALUE iniValue = NULL;
	CString authType, tmpHive;

	CNHReadiniFile iniFile;

	if (!iniFile.Open((BYTE *)data, CNHReadiniFile::modeRead, CNHReadiniFile::modeASCII))
	{
		NHERROR((L"INI failed to parse\r\n"));
		return false;
	}

	while (NULL != (iniValue = iniFile.ReadiniValue()))
	{
		if (iniValue->Section == L"UPDATE")
		{
			if (iniValue->Key == L"REPO")
			{
				config->RepositoryBaseURL = iniValue->Values[0];
				parsedFlags |= 1 << 0;
			}
			else if (iniValue->Key == L"MIN_SEVERITY")
			{
				config->MinimumSeverity = Asc2Int(iniValue->Values[0]);
				parsedFlags |= 1 << 1;
			}
			else if (iniValue->Key == L"TERMINAL_ID")
			{
				config->TerminalID = iniValue->Values[0];
				parsedFlags |= 1 << 2;
			}
			else if (iniValue->Key == L"HIVES")
			{
				for (int i = 0; i < iniValue->nValue; i++)
				{
					tmpHive = iniValue->Values[i];
					// Do template replacement
					InteroplateTemplateString(tmpHive);

					config->Hives.Add(tmpHive);
				}
				
				parsedFlags |= 1 << 3;
			}
			else if (iniValue->Key == L"USERNAME")
			{
				config->Username = iniValue->Values[0];
				parsedFlags |= 1 << 4;
			}
			else if (iniValue->Key == L"PASSWORD")
			{
				config->Password = iniValue->Values[0];
				parsedFlags |= 1 << 5;
			}
			else if (iniValue->Key == L"AUTH_TYPE")
			{
				authType = iniValue->Values[0];
				parsedFlags |= 1 << 6;
			}
		}
	}

	// Value will be AUTHN_NONE by default
	if (authType == AUTHN_NAME_BASIC)
	{
		config->AuthenticationType = AUTHN_BASIC;
	}

	return (parsedFlags & requiredFlags) == requiredFlags;
}

void CUpdateRepoConfigurationManager::InteroplateTemplateString(CString &input)
{
	CNHConfig config;
	NH_CE_VERSION osVersion = config.GetCEVersion();

	input.Replace(L"$territory", COUNTRY_CODE);
	input.Replace(L"$model", config.GetMachineType());
#ifdef AP_TYPE
	input.Replace(L"$lineage", AP_TYPE);
#endif

	switch (osVersion)
	{
	case WINCE_6:
		input.Replace(L"$os", L"SE60");
		break;
	case WINCE_7:
		input.Replace(L"$os", L"SE70");
		break;
	}
}
