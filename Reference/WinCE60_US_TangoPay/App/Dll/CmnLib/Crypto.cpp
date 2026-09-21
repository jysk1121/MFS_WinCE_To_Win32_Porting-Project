#pragma once

#include "stdafx.h"
#include "..\..\H\Common\CmnLib.h"
#include "..\..\H\Common\Crypto.h"
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/pkcs12.h>
#include <crypto/asn1.h>
#include <openssl/err.h>
#include "..\..\H\Common\NHDbgApi.h"
#include <openssl/rand.h>

#define VERIFICATION_SEGMENT_SIZE 1024 * 10

#define OPENSSL_ERR { \
	long err = 0; \
	while ((err = ERR_get_error()) != 0) { \
		NHINFO((L"OpenSSL Error: %d : %S\r\n", err, ERR_error_string(err, NULL))); \
	} \
}

#define OPENSSL_ERR_GOTO(label) { \
	long err = 0; \
	while ((err = ERR_get_error()) != 0) { \
		NHINFO((L"OpenSSL Error: %d : %S\r\n", err, ERR_error_string(err, NULL))); \
	} \
	goto label; \
}

#define OPENSSL_PRINT_X509_ERR(ctx) {int e=X509_STORE_CTX_get_error(ctx);NHERROR((L"Verification error: %S\r\n", X509_verify_cert_error_string(e)));}

//
// Helpers!
//

unsigned int hash_data_sha256(const char * data, unsigned int len, unsigned char * buffer)
{
	bool result = false;
	EVP_MD_CTX *ctx = NULL;
	const EVP_MD *md = EVP_sha256();
	unsigned int size = EVP_MD_size(md);

	if (buffer == NULL)
	{
		// Length of the hash
		return size;
	}

	ctx = EVP_MD_CTX_create();

	if (EVP_DigestInit_ex(ctx, md, NULL) != 1)
	{
		size = 0;
		goto cleanup;
	}

	if (EVP_DigestUpdate(ctx, data, len) != 1)
	{
		size = 0;
		goto cleanup;
	}

	if (EVP_DigestFinal_ex(ctx, buffer, &size) != 1)
	{
		memset(buffer, 0, size);
		size = 0;
		goto cleanup;
	}

cleanup:
	if (ctx) EVP_MD_CTX_destroy(ctx);

	return size;
}

unsigned int hmac_data_sha256(const unsigned char * key, int key_len, const unsigned char * data, unsigned int len, unsigned char * buffer)
{
	bool result = false;
	HMAC_CTX *ctx = NULL;
	const EVP_MD *md = EVP_sha256();
	unsigned int size = EVP_MD_size(md);

	if (buffer == NULL)
	{
		// Length of the hash
		return size;
	}

	ctx = HMAC_CTX_new();

	if (HMAC_Init_ex(ctx, key, key_len, md, NULL) != 1)
	{
		size = 0;
		goto cleanup;
	}

	if (HMAC_Update(ctx, data, len) != 1)
	{
		size = 0;
		goto cleanup;
	}

	if (HMAC_Final(ctx, buffer, &size) != 1)
	{
		memset(buffer, 0, size);
		size = 0;
		goto cleanup;
	}

cleanup:
	if (ctx) HMAC_CTX_free(ctx);

	return size;
}

char* alloc_string_from_unicode(CString input) 
{
	int len = input.GetLength();
	char *buffer = new char[len + 1]();
	WideToMulti(buffer, input, len);

	return buffer;
}

unsigned char* alloc_base64_decoded_string(CString data, int *len)
{
	int inputLen = data.GetLength();
	int encodedLength = (3 * inputLen) / 4;
	char *input = alloc_string_from_unicode(data);
	unsigned char *output = new unsigned char[encodedLength + 1]();

	*len = base64_decode(input, data.GetLength(), output, encodedLength); // Compliant: input is a null-terminated string

	memset(input, 0, inputLen);
	delete [] input;

	return output;
}

X509* alloc_x509_pem(char * pem_data)
{
	X509* cert = NULL;
	BIO *cert_bio = BIO_new(BIO_s_mem());
	int data_len = strlen(pem_data);

	BIO_write(cert_bio, pem_data, data_len);
	cert = PEM_read_bio_X509(cert_bio, NULL, NULL, NULL);
	if (cert == NULL)
	{
		NHERROR((L"LTX chain root PEM parsing failed\r\n"));
		OPENSSL_ERR;
	}

	BIO_free_all(cert_bio);
	return cert;
}

X509* alloc_x509_pem_cstring(CString pemData) 
{
	X509 *cert = NULL;
	int len = pemData.GetLength();
	char *pem_data = new char[len + 1]();

	WideToMulti(pem_data, pemData, len);
	cert = alloc_x509_pem(pem_data);

	memset(pem_data, 0, len);
	delete [] pem_data;

	return cert;
}

bool verify_cert_chain(CList<CString, CString&> *certList, X509 *rootCert)
{
	bool result = false;
	POSITION pos = certList->GetTailPosition();
	X509 *tmp = NULL;
	CString tmpPemData = L"";
	X509_STORE_CTX *ctx = NULL;
	STACK_OF(X509) *chainCerts = sk_X509_new_null();
	X509_STORE *trustedRoots = X509_STORE_new();

	// Add trust and certs
	X509_STORE_add_cert(trustedRoots, rootCert);

	do
	{
		tmpPemData = certList->GetPrev(pos);
		tmp = alloc_x509_pem_cstring(tmpPemData);
		if (tmp == NULL)
		{
			NHERROR((L"Chain incomplete. PEM data invalid\r\n"));
			goto cleanup;
		}

		sk_X509_push(chainCerts, tmp);
	} while (pos != NULL);

	// Verify! (tmp is set to the leaf cert)
	ctx = X509_STORE_CTX_new();
	X509_STORE_CTX_init(ctx, trustedRoots, tmp, chainCerts);

	if (X509_verify_cert(ctx) != 1) 
	{
		NHERROR((L"Chain verification failed for leaf cert\r\n"));
		OPENSSL_PRINT_X509_ERR(ctx);
		goto cleanup;
	}

	result = true;

cleanup:
	if (ctx != NULL)			X509_STORE_CTX_free(ctx);
	if (trustedRoots != NULL)	X509_STORE_free(trustedRoots);
	if (chainCerts != NULL)		sk_X509_pop_free(chainCerts, X509_free);

	return result;
}

void X509_STORE_add_certs(X509_STORE* store, STACK_OF(X509)* certs)
{
	X509* cert = NULL;
	for (int i = 0; i < sk_X509_num(certs); i++)
	{
		cert = sk_X509_value(certs, i);
		if (!X509_STORE_add_cert(store, cert))
		{
			NHERROR((L"Error adding certificate to store\r\n"));
			OPENSSL_ERR
		}
	}
}

/**
 * This function will verify that at least one chain in the PKCS7 signer info is valid, and it will
 * verify ALL signatures with the signing certsificates within the PKCS7 structure. Please note that
 * this implementation is based on the references at 
 * https://github.com/openssl/openssl/blob/038f4dc68edd16f719ce5cf140eda2fb5b86a62a/crypto/pkcs7/pk7_smime.c#L212
 */
int PKCS7_verify_one_chain(PKCS7* pkcs7, X509_STORE* store, BIO* indata, BIO* out)
{
	STACK_OF(X509) *signers;
	STACK_OF(PKCS7_SIGNER_INFO) *sinfos;
	X509_STORE_CTX *cert_ctx = NULL;
	int result = 1;
	int err = 0;
	int verified = 0;

	sinfos = PKCS7_get_signer_info(pkcs7);

	if (!sinfos || !sk_PKCS7_SIGNER_INFO_num(sinfos))
	{
		NHERROR((L"No signers in pkcs7 data\r\n"));
		return 0;
	}

	signers = PKCS7_get0_signers(pkcs7, NULL, 0);
	if (signers == NULL)
		return 0;

	cert_ctx = X509_STORE_CTX_new();
	for (int k = 0; k < sk_X509_num(signers); k++) 
	{
		X509_VERIFY_PARAM *verifyparam = NULL;
		X509 *signer;
		PKCS7_SIGNER_INFO *si;
		ASN1_TYPE *timestamp = NULL;
		time_t utctime = 0;
		struct tm t = {};

		si = sk_PKCS7_SIGNER_INFO_value(sinfos, k);
		signer = sk_X509_value(signers, k);
		if (!X509_STORE_CTX_init(cert_ctx, store, signer, pkcs7->d.sign->cert)) 
		{
			NHERROR((L"Could not initialize store context from pkcs7 data\r\n"));
			goto err;
		}
		X509_STORE_CTX_set_default(cert_ctx, "smime_sign");

		timestamp = PKCS7_get_signed_attribute(si, NID_pkcs9_signingTime);
		if (timestamp != NULL && timestamp->type == V_ASN1_UTCTIME)
		{
			if (ASN1_TIME_to_tm(timestamp->value.utctime, &t) == 1)
			{
				utctime = _mktime64(&t);
				verifyparam = X509_STORE_CTX_get0_param(cert_ctx);
				X509_VERIFY_PARAM_set_time(verifyparam, utctime);
			}
		}

		result = X509_verify_cert(cert_ctx);
		if (result <= 0)
		{
			err = X509_STORE_CTX_get_error(cert_ctx);
			NHERROR((L"Certificate chain verification errors: %S\r\n", X509_verify_cert_error_string(err)));
		}
		X509_STORE_CTX_cleanup(cert_ctx);

		if (result == 1)
		{
			verified++;
		}
	}

	if (verified < 1) 
	{
		NHERROR((L"Verified 0 chains\r\n"));
		goto err;
	}

	// Verify signatures
	result = PKCS7_verify(pkcs7, NULL, store, indata, out, PKCS7_NOVERIFY);
	if (!result)
	{
		NHERROR((L"PKCS7 verification failed\r\n"));
		OPENSSL_ERR
	}

err:
	X509_STORE_CTX_free(cert_ctx);
	sk_X509_pop_free(signers, X509_free);

	return result;
}

/**
 * Gets the RSA cert from the certificate file
 * @param[in] filePath the filename of the RSA public key
 * @param[in] serialNumber the serial number of the cert
 * @param[out] pkey the public key which is read from the package. Must use X509_free(pkey) after use
 * @returns true if successful
 */
bool CryptoLib::GetPublicKeyFromFilename(CString filePath, const char * serialNumber, X509** pkey)
{
	bool result = false;
	CString last3 = filePath.Right(3);

	if (last3 == L"pfx" || last3 == L"p12")
	{
		GetRSAKeyDataFromPKCS12(filePath, pkey);
	}
	else if (last3 == L"pem")
	{
		GetRSAKeyData(filePath, pkey);
	}
	else if (last3 == L"p7b" || last3 == L"spc")
	{
		// Assume PKCS7
		GetRSAKeyDataFromPKCS7(filePath, serialNumber, pkey);
	}
	if (*pkey == NULL)
	{
		NHERROR((L"Root cert %s not found\r\n", filePath));
		goto cleanup;
	}

	result = true;

cleanup:
	return result;
}

/**
 * Gets the RSA cert from the PKCS7 (PFX, P7B, PKCS7) file
 * @param[in] filePath the path to the package
 * @param[in] serialNumber the serial number of the cert
 * @param[out] pkey the public key which is read from the package. Must use X509_free(pkey) after use
 * @returns true if successful
 */
bool CryptoLib::GetRSAKeyDataFromPKCS7(CString filePath, const char * serialNumber, X509** pkey)
{
	bool result = false;
	BIO* pkcs7Data = NULL;
	PKCS7* pkcs7 = NULL;
	int ans1Type = 0;
	STACK_OF(X509) *certs = NULL;
	BIGNUM *expectedSerial = NULL;
	BIGNUM *serial = BN_new();
	int size = 0;
	X509 *x;

	unsigned char * fileData = GetFileContents(filePath, NULL, 0, size);
	if (size <= 0)
	{
		NHERROR((L"No data from file\r\n"));
		goto cleanup;
	}

	pkcs7Data = BIO_new(BIO_s_mem());
	BIO_write(pkcs7Data, fileData, size);

	if (!d2i_PKCS7_bio(pkcs7Data, &pkcs7))
	{
		OPENSSL_ERR;
		NHERROR((L"Cannot decode PKCS7. Attempting PEM load\r\n"));

		// Reset and read from PEM
		BIO_reset(pkcs7Data);
		BIO_write(pkcs7Data, fileData, size);
		if (!PEM_read_bio_PKCS7(pkcs7Data, &pkcs7, NULL, NULL))
		{
			NHERROR((L"PEM deserialization failed\r\n"));
			OPENSSL_ERR_GOTO(cleanup);
		}
	}

	ans1Type = OBJ_obj2nid(pkcs7->type);
	if (ans1Type == NID_pkcs7_signed)
	{
		certs = pkcs7->d.sign->cert;
	}
	else if (ans1Type == NID_pkcs7_signedAndEnveloped)
	{
		certs = pkcs7->d.signed_and_enveloped->cert;
	}

	if (certs == NULL)
	{
		NHERROR((L"No certs found in package\r\n"));
		goto cleanup;
	}

	expectedSerial = BN_new();
	if (BN_dec2bn(&expectedSerial, serialNumber) == 0)
	{
		NHERROR((L"Could not parse hex serial input\r\n"));
		OPENSSL_ERR_GOTO(cleanup);
	}

	for (int i = 0; i < sk_X509_num(certs); i++)
	{
		x = sk_X509_value(certs, i);

		X509_NAME *name = X509_get_subject_name(x);
		NHDBG((L"Found cert: %S\r\n", X509_NAME_oneline(name, NULL, 0)));
		
		// NB: Should be replaced with ASN1_INTEGER_get_int64 in the future (OpenSSL 1.1.0)
		if (!ASN1_INTEGER_to_BN(X509_get0_serialNumber(x), serial))
		{
			NHERROR((L"Could not parse serial number of cert\r\n"));
			OPENSSL_ERR_GOTO(cleanup);
		}

		if (BN_cmp(serial, expectedSerial) == 0) // Are equal
		{
			// Read key data into buffer
			*pkey = X509_dup(x);
			result = true;
			goto cleanup;
		}
	}

cleanup:
	if (fileData)
	{
		memset(fileData, 0, size);
		delete [] fileData;
	}

	if (expectedSerial) BN_free(expectedSerial);
	if (pkcs7) PKCS7_free(pkcs7);
	if (pkcs7Data) BIO_free_all(pkcs7Data);

	return result;
};

/**
 * Gets the RSA cert from the PKCS12 (PFX, PKCS12) file
 * @param[in] filePath the path to the package
 * @param[out] pkey the public key which is read from the package. Must use X509_free(pkey) after use
 * @returns true if successful
 */
bool CryptoLib::GetRSAKeyDataFromPKCS12(CString filePath, X509** pkey)
{
	bool result = false;
	BIO* pkcs12Data = NULL;
	PKCS12 *pkcs12 = PKCS12_new();
	int ans1Type = 0;
	X509* cert = NULL;
	EVP_PKEY *privateKey = NULL;
	int size = 0;

	unsigned char * fileData = GetFileContents(filePath, NULL, 0, size);
	if (size <= 0)
	{
		NHERROR((L"No data from file\r\n"));
		goto cleanup;
	}

	pkcs12Data = BIO_new(BIO_s_mem());
	BIO_write(pkcs12Data, fileData, size);

	if (i2d_PKCS12_bio(pkcs12Data, pkcs12) != 1)
	{
		NHERROR((L"PKCS12 file could not be opened\r\n"));
		OPENSSL_ERR_GOTO(cleanup);
	}

	if (PKCS12_parse(pkcs12, "", &privateKey, &cert, /* ca */ NULL) != 1)
	{
		NHERROR((L"Could not parse PKCS12 file\r\n"));
		OPENSSL_ERR_GOTO(cleanup);
	}

	*pkey = cert;
	result = true;

cleanup:
	if (fileData)
	{
		memset(fileData, 0, size);
		delete [] fileData;
	}

	if (pkcs12Data) BIO_free_all(pkcs12Data);
	if (pkcs12) PKCS12_free(pkcs12);
	if (privateKey) EVP_PKEY_free(privateKey);

	return result;
};


/**
 * Gets the RSA key data from the appropriate RSA key.
 * @param[in] filePath the filename of the RSA public key
 * @param[out] data the buffer into which the function will set the contents of the RSA key
 * @param[out] pkey the public key which is read from the package. Must use X509_free(pkey) after use
 * @returns true if successful
 */
bool CryptoLib::GetRSAKeyData(CString filePath, X509** pkey)
{
	bool result = false;
	BIO* pubKeyBio = NULL;
	unsigned char * keyFileContents;
	int size = 0;

	keyFileContents = GetFileContents(filePath, NULL, 0, size);
	if (size <= 0)
	{
		goto cleanup;
	}

	// Get public key
	pubKeyBio = BIO_new(BIO_s_mem());
	BIO_write(pubKeyBio, keyFileContents, size);
	X509 *cert = PEM_read_bio_X509(pubKeyBio, NULL, NULL, NULL);
	if (cert == NULL)
	{
		goto cleanup;
	}

	result = true;

cleanup:

	if (keyFileContents)
	{
		memset(keyFileContents, 0, size);
		delete [] keyFileContents;
	}
	return result;
};

// Validation functions
// NB: This should be refactored to move some common logic out of this function
bool CryptoLib::ValidateFileRSA256(ValidateDataInputFileRSA input)
{
	//unsigned char * fileData;
	int fileSize = 0;
	int res = 0;
	int read = 0;
	int segmentSize = VERIFICATION_SEGMENT_SIZE;
	unsigned char segment[VERIFICATION_SEGMENT_SIZE] = {};
	EVP_MD_CTX *mdctx = NULL;
	X509* cert = NULL;
	EVP_PKEY* pubKey = NULL;

	if (!GetPublicKeyFromFilename(input.PublicKeyFilePath, input.KeySerial, &cert))
	{
		NHERROR((L"Certificate %s not found\r\n", input.PublicKeyFilePath));
		goto cleanup;
	}

	pubKey = X509_get0_pubkey(cert);

	if (!FILE_EXISTS(input.FilePath))
	{
		NHERROR((L"No file for item found: %s\r\n", input.FilePath));
		goto cleanup;
	}

	//Do signature verification
	mdctx = EVP_MD_CTX_create();
	if (mdctx == NULL) 
	{
		goto cleanup;
	}

	res = EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, pubKey);
	if (res != 1)
	{
		goto cleanup;
	}

	while (segmentSize == VERIFICATION_SEGMENT_SIZE)
	{
		GetFileContents(input.FilePath, segment, read, segmentSize);
		
		res = EVP_DigestVerifyUpdate(mdctx, segment, segmentSize);
		if (res != 1)
		{
			goto cleanup;
		}

		read += segmentSize;
	}

	res = EVP_DigestVerifyFinal(mdctx, input.Signature, input.SignatureSize);
	if (res != 1)
	{
		NHERROR((L"Signature verification failed\r\n"));
		OPENSSL_ERR_GOTO(cleanup);
	}

cleanup:
	if (segment) memset(segment, 0, sizeof(segment));
	if (mdctx != NULL) EVP_MD_CTX_destroy(mdctx);
	if (cert) X509_free(cert);

	// Return true if the verify command resulted in 1.
	return res == 1;
};

bool CryptoLib::ValidateFileRSA256(ValidateDataInputFileRSACert &input)
{
	int fileSize = 0;
	int read = 0;
	int segmentSize = VERIFICATION_SEGMENT_SIZE;
	unsigned char segment[VERIFICATION_SEGMENT_SIZE] = {};
	EVP_MD_CTX *mdctx = NULL;
	EVP_PKEY* leafKey = NULL;
	int res = 0;

	if (!FILE_EXISTS(input.FilePath))
	{
		NHERROR((L"No file for item found: %s\r\n", input.FilePath));
		goto cleanup;
	}

	leafKey = X509_get0_pubkey(input.Cert);
	X509_NAME* name = X509_get_subject_name(input.Cert);
	NHINFO((L"Using %S for signature verification\r\n", X509_NAME_oneline(name, NULL, 0)));

	//Do signature verification
	mdctx = EVP_MD_CTX_create();
	if (mdctx == NULL) 
	{
		goto cleanup;
	}

	res = EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, leafKey);
	if (res != 1)
	{
		goto cleanup;
	}

	while (segmentSize == VERIFICATION_SEGMENT_SIZE)
	{
		GetFileContents(input.FilePath, segment, read, segmentSize);

		res = EVP_DigestVerifyUpdate(mdctx, segment, segmentSize);
		if (res != 1)
		{
			goto cleanup;
		}

		read += segmentSize;
	}

	res = EVP_DigestVerifyFinal(mdctx, input.Signature, input.SignatureSize);
	if (res != 1)
	{
		NHERROR((L"Signature verification failed\r\n"));
		OPENSSL_ERR_GOTO(cleanup);
	}

cleanup:
	if (segment) memset(segment, 0, sizeof(segment));
	if (mdctx != NULL) EVP_MD_CTX_destroy(mdctx);;

	// Return true if the verify command resulted in 1.
	return res == 1;
};

bool CryptoLib::ValidateDataRSA256(ValidateDataInputDataRSA input)
{
	EVP_MD_CTX *mdctx = NULL;
	X509* cert = NULL;
	EVP_PKEY* pubKey = NULL;
	int res = 0;

	if (!GetPublicKeyFromFilename(input.PublicKeyFilePath, input.KeySerial, &cert))
	{
		NHERROR((L"Certificate %s not found\r\n", input.PublicKeyFilePath));
		goto cleanup;
	}

	pubKey = X509_get0_pubkey(cert);

	//Do signature verification
	mdctx = EVP_MD_CTX_create();
	if (mdctx == NULL) 
	{
		goto cleanup;
	}

	res = EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, pubKey);
	if (res != 1)
	{
		NHERROR((L"Signature verification init failed\r\n"));
		OPENSSL_ERR_GOTO(cleanup);
		goto cleanup;
	}

	res = EVP_DigestVerifyUpdate(mdctx, input.InputData, input.InputDataSize);
	if (res != 1)
	{
		goto cleanup;
	}

	res = EVP_DigestVerifyFinal(mdctx, input.Signature, input.SignatureSize);
	if (res != 1)
	{
		NHERROR((L"Signature verification failed\r\n"));
		OPENSSL_ERR_GOTO(cleanup);
	}

cleanup:
	if (mdctx != NULL) EVP_MD_CTX_destroy(mdctx);

	if (cert) X509_free(cert);

	// Return true if the verify command resulted in 1.
	return res == 1;
};

bool CryptoLib::ValidateDataRSA256(ValidateDataInputDataRSACert  &input)
{
	EVP_MD_CTX *mdctx = NULL;
	EVP_PKEY* leafKey = NULL;
	int res = 0;
	
	leafKey = X509_get0_pubkey(input.Cert);
	if (leafKey == NULL) 
	{
		NHERROR((L"Cannot find pub key in X509 container\r\n"));
		goto cleanup;
	}

	//Do signature verification
	mdctx = EVP_MD_CTX_create();
	if (mdctx == NULL) 
	{
		goto cleanup;
	}

	res = EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, leafKey);
	if (res != 1)
	{
		goto cleanup;
	}

	res = EVP_DigestVerifyUpdate(mdctx, input.InputData, input.InputDataSize);
	if (res != 1)
	{
		goto cleanup;
	}

	res = EVP_DigestVerifyFinal(mdctx, input.Signature, input.SignatureSize);
	if (res != 1)
	{
		NHERROR((L"Signature verification failed\r\n"));
		OPENSSL_ERR_GOTO(cleanup);
	}

cleanup:
	if (mdctx != NULL) EVP_MD_CTX_destroy(mdctx);

	// Return true if the verify command resulted in 1.
	return res == 1;
};

bool CryptoLib::ValidateFileHMAC256(ValidateDataInputFileHMAC input)
{
	unsigned char * fileData = NULL;
	bool res = false;
	int fileSize = 0;
	ValidateDataInputDataHMAC dataInput = {};

	if (!FILE_EXISTS(input.FilePath))
	{
		NHERROR((L"No file for item found: %s\r\n", input.FilePath));
		goto cleanup;
	}

	// Read file contents
	fileData = GetFileContents(input.FilePath, NULL, 0, fileSize);
	if (fileSize <= 0 || fileData == NULL)
	{
		NHERROR((L"Could not read %s\r\n", input.FilePath));
		goto cleanup;
	}
	
	dataInput.InputData = fileData;
	dataInput.InputDataSize = fileSize;
	dataInput.Secret = input.Secret;
	dataInput.Signature = input.Signature;
	dataInput.SignatureSize = input.SignatureSize;

	res = ValidateDataHMAC256(dataInput);

cleanup:
	if (fileData)
	{
		memset(fileData, 0, fileSize);
		delete [] fileData;
		fileData = NULL;
	}

	return res;
};

bool CryptoLib::ValidateDataHMAC256(ValidateDataInputDataHMAC input)
{
	unsigned char mac[EVP_MAX_MD_SIZE] = {};
	bool res = false;
	unsigned int hashSize = 0u;
	int secretLen = input.Secret.GetLength() + 1;
	char * secret = new char[secretLen]();

	// Convert secret
	WideToMulti(secret, input.Secret, secretLen);

	HMAC(EVP_sha256(), secret, 8, input.InputData, input.InputDataSize, mac, &hashSize);

	// Compare digests
	if (input.SignatureSize != hashSize)
	{
		goto cleanup;
	}

	for (unsigned int i = 0; i < hashSize; i++)
	{
		if (mac[i] != input.Signature[i])
		{
			goto cleanup;
		}
	}

	res = true;

cleanup:
	memset(mac, 0, hashSize);	// This is a static array used by OpenSSL (NOT THREAD SAFE)

	memset(secret, 0, secretLen);
	delete [] secret;

	return res;
};

/**
 * Reads the contents of the file and returns a buffer with the data. If buffer is NULL, the pointer returned by the function 
 * has been newly allocated on the heap. Otherwise, the file contents will be placed into the buffer pointed to by buffer
 * @param[in] filePath the path to the file
 * @param[in,out] buffer the pointer to the buffer
 * @param[in] offset the offset of the file to start reading
 * @param[in,out] size the size of the buffer returned by the function if buffer is NULL, or the number of bytes read if buffer is not null.
 * @returns the pointer to the buffer
 */
unsigned char * CryptoLib::GetFileContents(CString filePath, unsigned char * buffer, int offset, int &size)
{
	HANDLE file = INVALID_HANDLE_VALUE;
	DWORD fileSize = 0;
	DWORD read = 0;
	bool myBuffer = (buffer == NULL);
	if (!FILE_EXISTS(filePath))
	{
		NHWARN((L"File %s does not exist\r\n", filePath));
		goto cleanup;
	}

	file = CreateFile(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		NHERROR((L"RSA validation file not opened\r\n"));
		goto cleanup;
	}

	// Get file size and fix buffer
	fileSize = GetFileSize(file, NULL);
	if (myBuffer)
	{
		// Create the buffer here
		buffer = new unsigned char[fileSize]();
		size = fileSize;
	}

	// Move the pointer to the specified byte in the file
	if (offset > 0 && offset < fileSize)
	{
		SetFilePointer(file, offset, NULL, FILE_BEGIN);	
	}

	if (!ReadFile(file, buffer, size, &read, NULL))
	{
		NHERROR((L"Error while reading file: %d\r\n", GetLastError()));
		goto cleanup;
	}

	// Provide the size of the new buffer, only if this function allocated it.
	size = (int)read;

cleanup:
	if (file != INVALID_HANDLE_VALUE)
	{
		CloseHandle(file);
		file = NULL;
	}

	if (size <= 0) // Indicates error, so we should clean the buffer
	{
		if (buffer)
		{
			memset(buffer, 0, fileSize);
		}

		if (myBuffer)
		{
			delete [] buffer;
			buffer = NULL;
		}
	}

	return buffer;
};


bool CryptoLib::GetCertificatesFromKeyStore(CString filePath, STACK_OF(X509) **certs)
{
	bool result = false;
	BIO* pkcs7Data = NULL;
	PKCS7* pkcs7 = NULL;
	int ans1Type = 0;
	int size = 0;
	X509 *x;

	unsigned char * fileData = GetFileContents(filePath, NULL, 0, size);
	if (size <= 0)
	{
		NHERROR((L"No data from file\r\n"));
		goto cleanup;
	}

	pkcs7Data = BIO_new(BIO_s_mem());
	BIO_write(pkcs7Data, fileData, size);

	if (!d2i_PKCS7_bio(pkcs7Data, &pkcs7))
	{
		OPENSSL_ERR;
		NHERROR((L"Cannot decode PKCS7. Attempting PEM load\r\n"));

		// Reset and read from PEM
		BIO_reset(pkcs7Data);
		BIO_write(pkcs7Data, fileData, size);
		if (!PEM_read_bio_PKCS7(pkcs7Data, &pkcs7, NULL, NULL))
		{
			NHERROR((L"PEM deserialization failed\r\n"));
			OPENSSL_ERR_GOTO(cleanup);
		}
	}

	ans1Type = OBJ_obj2nid(pkcs7->type);
	if (ans1Type == NID_pkcs7_signed)
	{
		*certs = pkcs7->d.sign->cert;
	}
	else if (ans1Type == NID_pkcs7_signedAndEnveloped)
	{
		*certs = pkcs7->d.signed_and_enveloped->cert;
	}

	if (*certs == NULL)
	{
		NHERROR((L"No certs found in package\r\n"));
		goto cleanup;
	}

	NHINFO((L"Found %d certs in store\r\n", sk_X509_num(*certs)));
	*certs = X509_chain_up_ref(*certs);

	result = true;

cleanup:
	if (fileData)
	{
		memset(fileData, 0, size);
		delete [] fileData;
	}

	if (pkcs7) PKCS7_free(pkcs7);
	if (pkcs7Data) BIO_free_all(pkcs7Data);

	return result;
}

CString CryptoLib::GetNewUUID(bool includeHyphens)
{
	CString strUUID = L"";
	CString strTemp;

	RAND_poll();

	for(int i=0; i<16; i++)
	{
		if (includeHyphens)
		{
			// add 4 "-" 
			if( (i==4)||(i==6)||(i==8)||(i==10) )
				strUUID += L"-";
		}

		// Generate 16 Random Numbers
		unsigned char btRand = NULL;
		RAND_bytes(&btRand, 1);

		// Adjust Version and Variant
		if(i==6)		btRand = ((btRand & 0x0f) | 0x40);	// Version : Make Version "4" (Random)
		else if(i==8)	btRand = ((btRand & 0x3f) + 0x80);	// Variant : Range 0x80 ~ 0xbf

		// Combine UUID
		strTemp.Format(L"%02x", btRand);
		strUUID += strTemp;
	}

	return strUUID;
}

CString CryptoLib::RSAEncryptWithPublicKey(CString strData, CString strPublicKey)
{
	unsigned char* chData = NULL;
	unsigned char* chPublicKey = NULL;
	unsigned char* uchDataEncrypted = NULL;

	CString strReturn = L"";

	NHDBG((L"Data:       %s\n", strData));
	NHDBG((L"PublicKey:  %s\n", strPublicKey));

	if (strPublicKey.IsEmpty())
	{
		NHDEBUG(DBG_INFO, (L"ERROR: Public Key empty\n"));
		goto cleanup;
	}

	chData = (unsigned char*)alloc_string_from_unicode(strData);
	chPublicKey = (unsigned char*)alloc_string_from_unicode(strPublicKey);

	BIO *keybio;
	keybio = BIO_new_mem_buf(chPublicKey, -1);

	if (keybio == NULL)
	{
		NHDEBUG(DBG_INFO, (L"ERROR: keybio is null\n"));
		goto cleanup;
	}

	EVP_PKEY* evp_key = PEM_read_bio_PUBKEY(keybio, NULL, NULL, NULL);
	if (evp_key == NULL)
	{
		NHDEBUG(DBG_INFO, (L"ERROR: PEM_read_bio_PUBKEY\n"));
		goto cleanup;
	}

	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(evp_key, NULL);
	if (ctx == NULL)
	{
		NHDEBUG(DBG_INFO, (L"ERROR: EVP_PKEY_CTX_new\n"));
		goto cleanup;
	}

	if (EVP_PKEY_encrypt_init(ctx) <= 0)
	{
		NHDEBUG(DBG_INFO, (L"ERROR: EVP_PKEY_encrypt_init\n"));
		goto cleanup;
	}

	if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
		NHDEBUG(DBG_INFO, (L"ERROR: EVP_PKEY_CTX_set_rsa_padding\n"));
		goto cleanup;
	}

	if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0) {
		NHDEBUG(DBG_INFO, (L"ERROR: EVP_PKEY_CTX_set_rsa_oaep_md\n"));
		goto cleanup;
	}

	if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) <= 0) {
		NHDEBUG(DBG_INFO, (L"ERROR: EVP_PKEY_CTX_set_rsa_mgf1_md\n"));
		goto cleanup;
	}

	size_t nLenDetermined;
	if (EVP_PKEY_encrypt(ctx, NULL, &nLenDetermined, chData, strlen((char*)chData)) <= 0) {
		NHDEBUG(DBG_INFO, (L"ERROR: EVP_PKEY_encrypt\n"));
		goto cleanup;
	}

	NHDEBUG(DBG_INFO, (L"Length of ciphertext: [%d]\n", nLenDetermined) );
	uchDataEncrypted = new unsigned char[nLenDetermined]();

	if (EVP_PKEY_encrypt(ctx, uchDataEncrypted, &nLenDetermined, chData, strlen((char*)chData)) <= 0) {
		NHDEBUG(DBG_INFO, (L"ERROR: EVP_PKEY_encrypt\n"));
		goto cleanup;
	}

	NHDEBUG(DBG_INFO, (L"EVP_PKEY_encrypt (datalen=[%d]): [%d]\n", strlen((char*)chData), nLenDetermined) );
	if (nLenDetermined > 0)
		strReturn = MakeUnPack(uchDataEncrypted, nLenDetermined);

cleanup:
	if (chData != NULL)
		delete[] chData;
	if (chPublicKey != NULL)
		delete[] chPublicKey;
	if (uchDataEncrypted != NULL)
		delete[] uchDataEncrypted;

	EVP_PKEY_CTX_free(ctx);

	NHDEBUG(DBG_INFO, (L"Encrypted data: [%s]\n", strReturn));
	return strReturn;
}

CString CryptoLib::GenerateHMACSHA256(CString strData, CString strHmacKey)
{
	int nHmacLen = hmac_data_sha256(NULL, 0, NULL, 0, NULL);
	if (nHmacLen == 0)
	{
		NHDEBUG(DBG_INFO, (L"Failed to get default length\n") );
		return _T("");
	}

	char *chData = alloc_string_from_unicode(strData);
	char* chHmacKey = alloc_string_from_unicode(strHmacKey);

	NHDEBUG(DBG_INFO, (L"Data:		[%s]\n", strData) );
	NHDEBUG(DBG_INFO, (L"HMACKey:	[%s]\n", strHmacKey) );

	unsigned char *uchMacResult = NULL;
	uchMacResult = new unsigned char[nHmacLen+1]();
	hmac_data_sha256((unsigned char*)chHmacKey, strHmacKey.GetLength(), (unsigned char*)chData, strData.GetLength(), uchMacResult);

	CString strHmacResult = UCharToHexStr(uchMacResult, nHmacLen);
	NHDEBUG(DBG_INFO, (L"HMACSHA256 RESULT: [%s]\n", strHmacResult) );

	if (chData != NULL)
		delete[] chData;
	if (chHmacKey != NULL)
		delete[] chHmacKey;
	if (uchMacResult != NULL)
		delete[] uchMacResult;

	return strHmacResult;
}

/*
Base64 translates 24 bits into 4 ASCII characters at a time. First,
3 8-bit bytes are treated as 4 6-bit groups. Those 4 groups are
translated into ASCII characters. That is, each 6-bit number is treated
as an index into the ASCII character array.

If the final set of bits is less 8 or 16 instead of 24, traditional base64
would add a padding character. However, if the length of the data is
known, then padding can be eliminated.

One difference between the "standard" Base64 is two characters are different.
See RFC 4648 for details.
This is how we end up with the Base64 URL encoding.
*/

const char base64_url_alphabet[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'
};

std::string CryptoLib::base64_url_encode(const std::string & in) {
	std::string out;
	int val =0, valb=-6;
	size_t len = in.length();
	unsigned int i = 0;
	for (i = 0; i < len; i++) {
		unsigned char c = in[i];
		val = (val<<8) + c;
		valb += 8;
		while (valb >= 0) {
			out.push_back(base64_url_alphabet[(val>>valb)&0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6) {
		out.push_back(base64_url_alphabet[((val<<8)>>(valb+8))&0x3F]);
	}
	return out;
}

std::string CryptoLib::base64_url_decode(const std::string & in) {
	std::string out;
	std::vector<int> T(256, -1);
	unsigned int i;
	for (i =0; i < 64; i++) T[base64_url_alphabet[i]] = i;

	int val = 0, valb = -8;
	for (i = 0; i < in.length(); i++) {
		unsigned char c = in[i];
		if (T[c] == -1) break;
		val = (val<<6) + T[c];
		valb += 6;
		if (valb >= 0) {
			out.push_back(char((val>>valb)&0xFF));
			valb -= 8;
		}
	}
	return out;
}


std::string CryptoLib::base64_encode(const std::string &in) {

	std::string out;
	unsigned int i;

	int val = 0, valb = -6;
	for (i = 0; i < in.length(); i++) {
		unsigned char c = in[i];
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0) {
			out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val>>valb)&0x3F]);
			valb -= 6;
		}
	}
	if (valb>-6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val<<8)>>(valb+8))&0x3F]);
	while (out.size()%4) out.push_back('=');
	return out;
}

std::string CryptoLib::base64_decode(const std::string &in) {

	std::string out;
	unsigned int i;

	std::vector<int> T(256,-1);
	for (i=0; i<64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

	int val=0, valb=-8;
	for (i = 0; i < in.length(); i++) {
		unsigned char c = in[i];
		if (T[c] == -1) break;
		val = (val << 6) + T[c];
		valb += 6;
		if (valb >= 0) {
			out.push_back(char((val>>valb)&0xFF));
			valb -= 8;
		}
	}
	return out;
}



CString CryptoLib::EncodeToBase64(CString strDataHex, bool useBase64Url)
{
	// CString to char array
	char *chData = alloc_string_from_unicode(strDataHex);

	// "303132" -> { 0x30, 0x31, 0x32 }
	int nDataLen = (strDataHex.GetLength() / 2);
	unsigned char* chDataPacked = new unsigned char[nDataLen]();
	MakePack(chData, chDataPacked, strDataHex.GetLength());
	NHDBG((L"Input: %s\n", UCharToHexStr((unsigned char*)chDataPacked, nDataLen)));

	// { 0x30, 0x31, 0x32 } -> "012"
	std::string data_original (reinterpret_cast< char const* > (chDataPacked), nDataLen);

	// encode to base64
	std::string data_base64encoded = useBase64Url ? CryptoLib::base64_url_encode(data_original) : CryptoLib::base64_encode(data_original);

	// std::string -> CString
	CString strDataBase64Encoded = data_base64encoded.c_str();
	NHDBG((L"Encoded to Base64: %s\n", strDataBase64Encoded));

	// release buffer
	delete[] chData;
	delete[] chDataPacked;

	return strDataBase64Encoded;
}

CString CryptoLib::UCharToHexStr(unsigned char* data, int dataLen)
{
	CString strResult;

	for (int i = 1; i <= dataLen; i++)
	{
		CString strTemp;
		strTemp.Format(_T("%02X"), data[i-1]);
		strResult += strTemp;
	}

	return strResult;
}
