#pragma once
#include ".\Net\HTTP.h"
#include "UpdateTypes.h"

#define REPO_ERROR_BASE 9000

typedef int REPOERR;

const REPOERR  REPOERR_OK				= 0;
const REPOERR  REPOERR_COMMUNICATION	(REPO_ERROR_BASE + 1);
const REPOERR  REPOERR_PARSE			(REPO_ERROR_BASE + 2);
const REPOERR  REPOERR_SIGVALIDATION	(REPO_ERROR_BASE + 3);
const REPOERR  REPOERR_MORE				(REPO_ERROR_BASE + 4);
const REPOERR  REPOERR_UNKNOWN			(REPO_ERROR_BASE + 5);
const REPOERR  REPOERR_VERSIONNOTSUPP	(REPO_ERROR_BASE + 6);

/**
 * Manages the communications with the update repository
 */
class AFX_CLASS_EXPORT CUpdateRepoService
{
public:
	CUpdateRepoService(UpdateSourceInfo *source);
	~CUpdateRepoService(void);

	/**
	 * Retrieves the list of packages from the component's contents.xml file.
	 * @param[in] component the component within the update hive to retrieve software packages from.
	 * @param[out] packages a list of packages
	 * @returns true if successful. If false, count is set to the number of packages available and packages is not modified.
	 */
	REPOERR GetComponentContents(CString component, PackageList &packages);

	/**
	 * Gets a list of upgradable software versions.
	 * @param[in] package the package for which to download the manifest
	 * @param[out] manifest the manifest into which to populate the data
	 * @returns a REPOERR
	 */
	REPOERR GetPackageManifest(Package const &package, Manifest &manifest);

	/**
	 * Gets the manifest for the latest version
	 * @param[in] component the component within the update hive to retrieve software packages from.
	 * @param[out] manifest the manifest into which to populate the data
	 * @returns a REPOERR
	 */
	REPOERR GetLatestPackageManifest(CString component, Manifest &manifest);

	/**
	 * Downloads the software from the repository tand validates it.
	 * @param[in] manifest the software version record to pre-stage
	 * @param[out] result the result of the pre-staging operation
	 * @returns a REPOERR
	 */
	REPOERR StageAndValidateSoftwareUpgrade(Manifest const &manifest, PreStagingResult &result);

private:
	// The update source to use
	UpdateSourceInfo *source;

	// The HTTP service
	HTTP* http;
	CHTTPBasicAuthenticator* authenticator;

private:

	/**
	 * Downloads the software from the repository to the pre-staging directory.
	 * @param[in] manifest the software version record to pre-stage
	 * @param[out] result the result of the pre-staging operation
	 * @returns a REPOERR
	 */
	REPOERR DownloadSoftware(Manifest const &manifest, PreStagingResult &result);

	/**
	 * Validates the software upgrade package and stages the upgrade, if successful.
	 * @param[in] result the result of the pre-staging operation
	 * @returns a REPOERR
	 */
	REPOERR ValidateStagedSoftware(PreStagingResult const &result, Manifest const &manifest);

	/**
	 * Validates the software upgrade package and stages the upgrade, if successful.
	 * @param[in] result the result of the pre-staging operation
	 * @returns a REPOERR
	 */
	REPOERR ValidateStagedSoftwareV11(PreStagingResult const &result, Manifest const &manifest);

	/**
	 * Deserialize the XML response into an array of package objects
	 * @param[in] data the raw XML string
	 * @param[in,out] packages a list of packages
	 * @param[in] component the component in which the packages are
	 */
	bool DeserializeContents(CString data, PackageList &packages, CString component);

	/**
	 * Deserialize the XML response into the manifest model
	 * @param[in] data the raw XML string
	 * @param[in,out] manifest the manifest model
	 * @returns true if successful
	 */
	bool DeserializeManifest(CString data, Manifest &manifest);

	/**
	 * Validates the manifest data with the signature file
	 * @param[in] manifest the manifest to validate
	 * @param[in] fileData the raw xml data buffer for the manifest
	 * @param[in] fileLen the len of the buffer
	 * @return true if successful
	 */
	bool ValidateManifest(Manifest const & manifest, unsigned char * fileData, unsigned int fileLen);

	/**
	 * Validates the manifest data with the signature file
	 * @param[in] manifest the manifest to validate
	 * @param[in] fileData the raw xml data buffer for the manifest
	 * @param[in] fileLen the len of the buffer
	 * @return true if successful
	 */
	bool ValidateManifestV11(Manifest const & manifest, unsigned char * fileData, unsigned int fileLen);

	/**
	 * Analyzes the chains provided and retrieves the ID and leaf of the first valid chain.
	 * @param chains [in] the list of chains from the manifest file
	 * @param rootStoreFile [in] the file path of the root key store to validate against
	 * @param chainId [out] the chain ID which first passed validation
	 * @param leafCert [out] the leaf cert of the chain. NB: *Must call X509_free after use*
	 */
	bool GetValidatedChainId(const ChainList &chains, CString rootStoreFile, CString &chainId, X509 **leafCert);
};
