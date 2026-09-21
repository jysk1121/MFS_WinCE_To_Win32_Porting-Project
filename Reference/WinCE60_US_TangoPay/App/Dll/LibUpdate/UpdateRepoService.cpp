#include "stdafx.h"
#include <openssl/pkcs7.h>
#include <openssl/x509v3.h>
#include "UpdateRepoService.h"
#include ".\Common\Crypto.h"

#include ".\Common\NHDbgApi.h"

// The package staging location for the master files
#ifdef UNDER_CE
#  define UPDATE_FILE_STAGE_DIR_60 L"\\UPDATE\\NH2700CE"
#  define UPDATE_FILE_STAGE_DIR_70 L"\\UPDATE7"
#else
#  define UPDATE_FILE_STAGE_DIR_60 L".\\staging"
#  define UPDATE_FILE_STAGE_DIR_70 L".\\staging"
#endif

#define NVERR(func, err) NVDump('F', 'V', func, L"", err);
#define NVINF(func, msg) NVDump('O', 'V', func, L"", msg);

CString GetUpdateFileDirectory()
{
	NH_CE_VERSION ceVersion = GetConfigFuncPointer()->GetCEVersion();
	if (ceVersion == WINCE_6)
	{
		return UPDATE_FILE_STAGE_DIR_60;
	}
	else 
	{
		return UPDATE_FILE_STAGE_DIR_70;
	}
}

CUpdateRepoService::CUpdateRepoService(UpdateSourceInfo *source)
{
	this->source = source;
	if (source->AuthenticationType == AUTHN_BASIC)
	{
		this->authenticator = new CHTTPBasicAuthenticator(source->Username, source->Password);
	}
	else
	{
		this->authenticator = NULL;
	}

	this->http = new HTTP(source->RepositoryBaseURL, this->authenticator);
}

CUpdateRepoService::~CUpdateRepoService(void)
{
	if (this->http != NULL)
	{
		delete this->http;
		this->http = NULL;
	}

	if (this->authenticator != NULL)
	{
		delete this->authenticator;
		this->authenticator = NULL;
	}
}


/**
 * Retrieves the list of packages from the component's contents.xml file.
 * @param[in] component the component within the update hive to retrieve software packages from.
 * @param[out] packages a list of packages 
 * @returns true if successful. If false, count is set to the number of packages available and packages is not modified.
 */
REPOERR CUpdateRepoService::GetComponentContents(CString component, PackageList &packages)
{
	NVINF("02", L"GET:" + component);
	CString url = source->GetContentsURL(component);

	HttpResponse response;
	HttpRequest contentsRequest;
	contentsRequest.UseCache = true;
	contentsRequest.Url = url;
	contentsRequest.Headers.Append(L"Connection: close\r\n");
	HTTPERR err = http->SendGetRequest(&response, contentsRequest);
	if (err != HTTP_NO_ERR || response.StatusCode != L"200")
	{
		NHERROR((L"Error while downloading contents\r\n"));
		NVERR("02", L"NG0");
		return REPOERR_COMMUNICATION;
	}

	if (!DeserializeContents(response.ResponseData, packages, component))
	{
		NHERROR((L"Deserialization of contents error\r\n"));
		NVERR("02", L"NG1");
		return REPOERR_PARSE;
	}

	return REPOERR_OK;
}

/**
 * Gets a list of upgradeable software versions.
 * @param[in] package the package for which to download the manifest
 * @param[out] manifest the manifest into which to populate the data
 * @returns a REPOERR
 */
REPOERR CUpdateRepoService::GetPackageManifest(Package const &package, Manifest &manifest)
{
	CString url = package.GetManifestURL(source);
	HttpResponse response;
	HttpRequest manifestRequest;
	HTTPERR err;
	unsigned char * manifestData;

	NVINF("03", L"DL:" + package.Version);

	manifestRequest.UseCache = false;
	manifestRequest.Url = url;
	manifestRequest.Headers.AppendFormat(L"Connection: close\r\n");
	err = http->SendGetRequest(&response, manifestRequest);
	if (err != HTTP_NO_ERR || response.StatusCode != L"200")
	{
		NHERROR((L"Error while downloading manifest\r\n"));
		NVERR("03", L"NG0");
		return REPOERR_COMMUNICATION;
	}

	if (!DeserializeManifest(response.ResponseData, manifest))
	{
		NHERROR((L"Deserialization of manifest error\r\n"));
		NVERR("03", L"NG1");
		return REPOERR_PARSE;
	}

	manifest.Metadata.BasePath = package.GetBaseURL(source);

#ifndef NO_VALIDATION
	// Validate
	manifestData = new unsigned char[response.ContentLength + 1]();
	WideToMulti((char *)manifestData, response.ResponseData, response.ContentLength);
	bool result = false;
	if (manifest.FileVersion.Find(MANIFEST_VERSION_1_0) == 0)
	{
		result = ValidateManifest(manifest, manifestData, response.ContentLength);
	}
	else if (manifest.FileVersion.Find(MANIFEST_VERSION_1_X) == 0)
	{
		result = ValidateManifestV11(manifest, manifestData, response.ContentLength);
	}
	else if (manifest.FileVersion.Find(MANIFEST_VERSION_2) == 0)  // Not supported
	{
		result = false;
	}

	delete [] manifestData;
	if (!result)
	{
		NHERROR((L"Manifest validation failed\r\n"));
		NVERR("03", L"NG2");
		return REPOERR_SIGVALIDATION;
	}
#endif

	return REPOERR_OK;
}

/**
 * Gets the manifest for the latest version
 * @param[in] component the component within the update hive to retrieve software packages from.
 * @param[out] manifest the manifest into which to populate the data
 * @returns a REPOERR
 */
REPOERR CUpdateRepoService::GetLatestPackageManifest(CString component, Manifest &manifest)
{
	CString url = source->GetLatestComponentPackageURL(component);
	CString manifestUrl = source->GetLatestComponentPackageManifestURL(component);
	HttpResponse response;
	HttpRequest manifestRequest;
	HTTPERR err;
	unsigned char * manifestData;

	NVINF("04", L"DL:" + component);

	manifestRequest.UseCache = true;
	manifestRequest.Url = manifestUrl;
	manifestRequest.Headers.Append(L"Connection: close\r\n");
	err = http->SendGetRequest(&response, manifestRequest);
	if (err != HTTP_NO_ERR || response.StatusCode != L"200")
	{
		NHERROR((L"Error while downloading manifest\r\n"));
		NVERR("04", L"NG0");
		return REPOERR_COMMUNICATION;
	}

	if (!DeserializeManifest(response.ResponseData, manifest))
	{
		NHERROR((L"Deserialization of manifest error\r\n"));
		NVERR("04", L"NG1");
		return REPOERR_PARSE;
	}

	manifest.Metadata.BasePath = url;

#ifndef NO_VALIDATION
	// Validate
	manifestData = new unsigned char[response.ContentLength + 1]();
	WideToMulti((char *)manifestData, response.ResponseData, response.ContentLength);
	bool result = false;
	if (manifest.FileVersion.Find(MANIFEST_VERSION_1_0) == 0)
	{
		result = ValidateManifest(manifest, manifestData, response.ContentLength);
	}
	else if (manifest.FileVersion.Find(MANIFEST_VERSION_1_X) == 0)
	{
		result = ValidateManifestV11(manifest, manifestData, response.ContentLength);
	}
	else if (manifest.FileVersion.Find(MANIFEST_VERSION_2) == 0)  // Not supported
	{
		result = false;
	}

	delete [] manifestData;
	if (!result)
	{
		NHERROR((L"Manifest validation failed\r\n"));
		NVERR("04", L"NG2");
		return REPOERR_SIGVALIDATION;
	}
#endif

	return REPOERR_OK;
}

/**
 * Downloads the software from the repository and validates it.
 * @param[in] manifest the software version record to pre-stage
 * @param[out] result the result of the pre-staging operation
 * @returns a REPOERR
 */
REPOERR CUpdateRepoService::StageAndValidateSoftwareUpgrade(Manifest const &manifest, PreStagingResult &result)
{
	REPOERR err = REPOERR_OK;

	NVINF("05", L"STAGE:" + manifest.Metadata.ReadableVersion);

	if ((err = DownloadSoftware(manifest, result)) != REPOERR_OK)
	{
		NVERR("05", L"NG0");
		goto cleanup;
	}

#ifndef NO_VALIDATION
	if (manifest.FileVersion.Find(MANIFEST_VERSION_1_0) == 0)
	{
		err = ValidateStagedSoftware(result, manifest);
	}
	else if (manifest.FileVersion.Find(MANIFEST_VERSION_1_X) == 0)
	{
		err = ValidateStagedSoftwareV11(result, manifest);
	}
	else if (manifest.FileVersion.Find(MANIFEST_VERSION_2) == 0)
	{
		err = REPOERR_VERSIONNOTSUPP;
	}

	if (err != REPOERR_OK)
	{
		NVERR("05", L"NG1");
		goto cleanup;
	}
#endif

cleanup:
	return err;
}

/**
 * Downloads the software from the repository to the pre-staging directory.
 * @param[in] version the software version record to pre-stage
 * @param[out] result the result of the pre-staging operation
 * @returns a REPOERR
 */
REPOERR CUpdateRepoService::DownloadSoftware(Manifest const &manifest, PreStagingResult &result)
{
	NVINF("06", L"DL:" + manifest.Metadata.ReadableVersion);
	CString stagingDirectory = GetUpdateFileDirectory();

	if (!FILE_EXISTS(stagingDirectory))
	{
		if (!CreateDirectoryWithIntermediate(stagingDirectory))
		{
			NVERR("06", L"NG0");
			return REPOERR_UNKNOWN;
		}
	}

	int count = manifest.Contents.GetCount();
	for (int i = 0; i < count; i++)
	{
		POSITION pos = manifest.Contents.FindIndex(i);
		ManifestItem item = manifest.Contents.GetAt(pos);

		CString destFile;
		destFile.Format(L"%s\\%s", stagingDirectory, item.Filename);
		CString url = item.GetURL(manifest.Metadata.BasePath);

		HttpRequest req;
		req.Url = url;
		req.UseCache = false;
		req.Headers.Append(L"Connection: close\r\n");
		if (http->DownloadFile(req, destFile) != HTTP_NO_ERR)
		{
			NVERR("06", L"NG1:" + item.Filename);
			NHERROR((L"Error while downloading file: %s\r\n", item.Filename));
			return REPOERR_COMMUNICATION;
		}
	}

	// Always set to the staging dir for now
	result.SoftwareDirectory = stagingDirectory;
	
	return REPOERR_OK;
}

bool CUpdateRepoService::ValidateManifest(Manifest const & manifest, unsigned char * fileData, unsigned int fileLen)
{
	HttpResponse response;
	HttpRequest request;
	HTTPERR err;
	bool result = false;
	ValidateDataInputDataHMAC hmac;
	ValidateDataInputDataRSA rsa;

	NVINF("07", manifest.Metadata.ReadableVersion);

	// Get signature file
	request.Url.Format(L"%s/%s", manifest.Metadata.BasePath, manifest.GetSignatureFileName());
	request.UseCache = false;
	request.Headers.Append(L"Connection: close\r\n");
	err = http->SendGetRequest(&response, request);
	if (err != HTTP_NO_ERR || response.StatusCode != L"200")
	{
		NHERROR((L"Error while downloading manifest signature\r\n"));
		NVERR("07", L"NG0");
		return false;
	}

	// Validate signature
	switch (manifest.Metadata.SigType)
	{
#ifdef _DEBUG
	case ST_HMAC256:
		hmac.InputData = fileData;
		hmac.InputDataSize = fileLen;
		hmac.Secret = L"password";
		hmac.Signature = response.BinaryData;
		hmac.SignatureSize = response.ContentLength;
		result = CryptoLib::ValidateDataHMAC256(hmac);
		break;
	case ST_NONE:
		result = true;
		break;
#endif
	case ST_RSASHA256:
		rsa.InputData = fileData;
		rsa.InputDataSize = fileLen;
		rsa.PublicKeyFilePath = GetCodeSigningKeyStoreFilePath();
		rsa.KeySerial = TARGET_CERT_SERIAL;
		rsa.Signature = response.BinaryData;
		rsa.SignatureSize = response.ContentLength;
		result = CryptoLib::ValidateDataRSA256(rsa);
		break;
	}

	if (!result)
	{
		NVERR("07", L"NG1");
	}

	return result;
}

bool CUpdateRepoService::ValidateManifestV11(Manifest const & manifest, unsigned char * fileData, unsigned int fileLen)
{
	HttpResponse response;
	HttpRequest request;
	HTTPERR err;
	bool result = false;
	PKCS7* pkcs7 = NULL;
	X509_VERIFY_PARAM* verifyParams = NULL;
	X509_STORE* caStore = X509_STORE_new();
	STACK_OF(X509)* certs = NULL;
	BIO* pkcs7Response = BIO_new(BIO_s_mem());
	BIO* manifestData = BIO_new(BIO_s_mem());

	NVINF("07", manifest.Metadata.ReadableVersion);

	// Get signature file
	request.Url.Format(L"%s/%s", manifest.Metadata.BasePath, manifest.GetSignatureFileName());
	request.UseCache = false;
	request.Headers.Append(L"Connection: close\r\n");
	err = http->SendGetRequest(&response, request);
	if (err != HTTP_NO_ERR || response.StatusCode != L"200")
	{
		NHERROR((L"Error while downloading manifest signature\r\n"));
		NVERR("07", L"NG0");
		goto cleanup;
	}

	// Get trusted certs
	if (!CryptoLib::GetCertificatesFromKeyStore(GetCodeSigningKeyStoreFilePath(), &certs))
	{
		NHERROR((L"Error getting trust store\r\n"));
		NVERR("07", L"NG2");
		goto cleanup;
	}

	verifyParams = X509_STORE_get0_param(caStore);
	X509_VERIFY_PARAM_set_purpose(verifyParams, X509_PURPOSE_ANY);
	X509_STORE_add_certs(caStore, certs); // Load CA trust

	// Load PKCS7 data from web response into the BIO for PEM parsing
	BIO_write(pkcs7Response, response.BinaryData, response.ContentLength);
	BIO_seek(pkcs7Response, 0);

	// Load manifest data
	BIO_write(manifestData, fileData, fileLen);
	BIO_seek(manifestData, 0);

	pkcs7 = PEM_read_bio_PKCS7(pkcs7Response, NULL, NULL, NULL);
	if (!pkcs7)
	{
		NHERROR((L"Error parsing pkcs7 data\r\n"));
		NVERR("07", L"NG3");
		goto cleanup;
	}

	result = PKCS7_verify_one_chain(pkcs7, caStore, manifestData, NULL) != 0;

	if (!result)
	{
		NVERR("07", L"NG1");
	}

cleanup:

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

	if (pkcs7 != NULL)
	{
		PKCS7_free(pkcs7);
		pkcs7 = NULL;
	}

	if (pkcs7Response != NULL)
	{
		BIO_free(pkcs7Response);
		pkcs7Response = NULL;
	}

	if (manifestData != NULL)
	{
		BIO_free(manifestData);
		manifestData = NULL;
	}

	return result;
}

/**
 * Validates the software upgrade package and stages the upgrade, if successful.
 * @param[in] result the result of the pre-staging operation
 * @returns a REPOERR
 */
REPOERR CUpdateRepoService::ValidateStagedSoftware(PreStagingResult const &result, Manifest const &manifest)
{
	ValidateDataInputFileHMAC hmac;
	ValidateDataInputFileRSA rsa;
	bool validationResult = false;

	NVINF("08", manifest.Metadata.ReadableVersion);

	// Loop through all files in manifest and validate!
	for (int i = 0; i < manifest.Contents.GetCount(); i++)
	{
		POSITION pos =  manifest.Contents.FindIndex(i);
		ManifestItem item =  manifest.Contents.GetAt(pos);

		CString fileLocation;
		fileLocation.Format(L"%s\\%s", result.SoftwareDirectory, item.Filename);

		switch (item.SigType)
		{
#ifdef _DEBUG // Only support HMAC256 and NONE in debug mode for security purposes
		case ST_HMAC256:
			hmac.FilePath = fileLocation;
			hmac.Secret = L"password";
			hmac.Signature = item.Signature;
			hmac.SignatureSize = item.SignatureSize;
			validationResult = CryptoLib::ValidateFileHMAC256(hmac);
			break;
		case ST_NONE:
			validationResult = true;
			break;
#endif
		case ST_RSASHA256:
			rsa.FilePath = fileLocation;
			rsa.PublicKeyFilePath = GetCodeSigningKeyStoreFilePath();
			rsa.KeySerial = TARGET_CERT_SERIAL;
			rsa.Signature = item.Signature;
			rsa.SignatureSize = item.SignatureSize;
			validationResult = CryptoLib::ValidateFileRSA256(rsa);
			break;
		default:
			validationResult = false;
			break;
		}

		if (!validationResult)
		{
			NVERR("07", L"NG0:" + item.Filename);
			return REPOERR_SIGVALIDATION;
		}
	}

	return REPOERR_OK;
}

/**
 * Validates the software upgrade package and stages the upgrade, if successful.
 * @param[in] result the result of the pre-staging operation
 * @returns a REPOERR
 */
REPOERR CUpdateRepoService::ValidateStagedSoftwareV11(PreStagingResult const &result, Manifest const &manifest)
{
	REPOERR err = REPOERR_OK;
	bool validationResult = false;
	CString validatedChainId;
	X509 *leafCert;
	NVINF("08", manifest.Metadata.ReadableVersion);

	// Loops through chains to find the first one that validates and get the chain ID
	if (!GetValidatedChainId(manifest.Metadata.Chains, GetCodeSigningKeyStoreFilePath(), validatedChainId, &leafCert))
	{
		// No chains!
		NVERR("08", L"NG1");
		err = REPOERR_SIGVALIDATION;
		goto cleanup;
	}

	// Loop through all files in manifest and validate!
	for (int i = 0; i < manifest.Contents.GetCount(); i++)
	{
		POSITION pos =  manifest.Contents.FindIndex(i);
		ManifestItem item =  manifest.Contents.GetAt(pos);
		ValidateDataInputFileRSACert rsa;
		ManifestItemSignature signature;

		CString fileLocation;
		fileLocation.Format(L"%s\\%s", result.SoftwareDirectory, item.Filename);

		// Get signature for the item with the matching chain ID
		if (!manifest.GetSignature(validatedChainId, item.Filename, signature))
		{	
			NHERROR((L"Signature for chain id %s not found\r\n", validatedChainId));
			err = REPOERR_SIGVALIDATION;
			goto cleanup;
		}

		// 1.1.0 does not support HMAC signatures
		switch (item.SigType)
		{
		case ST_RSASHA256:
			rsa.FilePath = fileLocation;
			rsa.Signature = signature.Signature;
			rsa.SignatureSize = signature.SignatureSize;
			rsa.Cert = leafCert;
			validationResult = CryptoLib::ValidateFileRSA256(rsa);
			break;
		}

		if (!validationResult)
		{
			NVERR("08", L"NG0:" + item.Filename);
			err = REPOERR_SIGVALIDATION;
			goto cleanup;
		}
	}

cleanup:

	if (leafCert)
	{
		X509_free(leafCert);
		leafCert = NULL;
	}

	return err;
}

bool CUpdateRepoService::GetValidatedChainId(const ChainList &chains, CString rootStoreFile, CString &chainId, X509 **leafCert)
{
	bool result = false;
	CertificateChain currentChain;
	POSITION pos = chains.GetHeadPosition();
	STACK_OF(X509) *certs = sk_X509_new_null();

	if (!CryptoLib::GetCertificatesFromKeyStore(GetCodeSigningKeyStoreFilePath(), &certs))
	{
		NHERROR((L"Failed to find any certs!\r\n"));
		goto cleanup;
	}

	while (pos != NULL)
	{
		X509 *currentRoot = NULL;
		currentChain = chains.GetNext(pos);

		for (int i = 0; i < sk_X509_num(certs); i++)
		{
			currentRoot = sk_X509_value(certs, i);
			
			if (verify_cert_chain((CList<CString, CString&>*)&currentChain.Certificates, currentRoot))
			{
				NHINFO((L"Found a valid chain!\r\n"));

				chainId = currentChain.ChainId;
				*leafCert = alloc_x509_pem_cstring(currentChain.Certificates.GetTail());

				result = true;
				goto cleanup;
			}
		}
	}

cleanup:

	if (certs != NULL)
	{
		sk_X509_pop_free(certs, X509_free);
		certs = NULL;
	}

	return result;
}