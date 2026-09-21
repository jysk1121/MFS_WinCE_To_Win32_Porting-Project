#pragma once

#include "stdafx.h"
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/pkcs12.h>
#include <openssl/err.h>
#include <openssl/pkcs7.h>
#include "NHDbgApi.h"
#include <afxtempl.h>

#include <string>
#include <vector>

typedef struct _validate_data_file_rsa
{
	// File to validate
	CString FilePath;

	// The signature to validate
	unsigned char * Signature;
	unsigned int SignatureSize;

	CString PublicKeyFilePath;
	// The serial number of the key if the file is a PKCS7 package
	const char * KeySerial;

	_validate_data_file_rsa()
	{
		KeySerial = NULL;
	};

} ValidateDataInputFileRSA;

typedef struct _validate_data_file_rsa_cert
{
	// File to validate
	CString FilePath;

	// The signature to validate
	unsigned char * Signature;
	unsigned int SignatureSize;

	X509* Cert;

	_validate_data_file_rsa_cert()
	{
		Signature = NULL;
		SignatureSize = NULL;
		Cert = NULL;
	};

} ValidateDataInputFileRSACert;

typedef struct _validate_data_file_hmac
{
	// File to validate
	CString FilePath;

	// The signature to validate
	unsigned char * Signature;
	unsigned int SignatureSize;

	CString Secret;

} ValidateDataInputFileHMAC;

typedef struct _validate_data_data_rsa
{
	unsigned char * InputData;
	unsigned int InputDataSize;

	// The signature to validate
	unsigned char * Signature;
	unsigned int SignatureSize;

	CString PublicKeyFilePath;
	// The serial number of the key if the file is a PKCS7 package
	const char * KeySerial;

	_validate_data_data_rsa()
	{
		KeySerial = NULL;
		Signature = NULL;
		SignatureSize = NULL;
		InputData = NULL;
		InputDataSize = 0;
	};

} ValidateDataInputDataRSA;

typedef struct _validate_data_data_rsa_cert
{
	unsigned char * InputData;
	unsigned int InputDataSize;

	// The signature to validate
	unsigned char * Signature;
	unsigned int SignatureSize;

	X509* Cert;

	_validate_data_data_rsa_cert()
	{
		Signature = NULL;
		SignatureSize = NULL;
		InputData = NULL;
		InputDataSize = 0;
		Cert = NULL;
	};

} ValidateDataInputDataRSACert;

typedef struct _validate_data_data_hmac
{
	// File to validate
	unsigned char * InputData;
	unsigned int InputDataSize;

	// The signature to validate
	unsigned char * Signature;
	unsigned int SignatureSize;

	CString Secret;

} ValidateDataInputDataHMAC;


//
// Helpers!
//

#ifdef __cplusplus
extern "C" {  // only need to export C interface if
	// used by C++ source code
#endif

__declspec( dllimport ) unsigned int hash_data_sha256(const char * data, unsigned int len, unsigned char * buffer);
__declspec( dllimport ) unsigned int hmac_data_sha256(const unsigned char * key, int key_len, const unsigned char * data, unsigned int len, unsigned char * buffer);
__declspec( dllimport ) char* alloc_string_from_unicode(CString input);
__declspec( dllimport ) unsigned char* alloc_base64_decoded_string(CString data, int *len);
__declspec( dllimport ) X509* alloc_x509_pem(char * pem_data);
__declspec( dllimport ) X509* alloc_x509_pem_cstring(CString pemData);
__declspec( dllimport ) bool verify_cert_chain(CList<CString, CString&> *certList, X509 *rootCert);
__declspec( dllimport ) void X509_STORE_add_certs(X509_STORE* store, STACK_OF(X509)* certs);
__declspec( dllimport ) int PKCS7_verify_one_chain(PKCS7* pkcs7, X509_STORE* store, BIO* indata, BIO* out);

#ifdef __cplusplus
}
#endif

class AFX_CLASS_EXPORT CryptoLib
{
private:
	// stolen from: https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c
	// TODO: import and use 'https://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c' when performance needs to be optimized

	// stolen from: https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
	static std::string base64_encode(const std::string & in);
	static std::string base64_decode(const std::string & in);

	// stolen from: https://stackoverflow.com/questions/57292210/what-are-the-exact-base64-url-decoding-rules-and-implementation-using-openssl-an
	static std::string base64_url_encode(const std::string & in);
	static std::string base64_url_decode(const std::string & in);

public:
	/**
	 * Gets the X509 cert data from the file at the path
	 * @param[in] filePath the filename of the RSA public key
	 * @param[in] serial the serial of the certificate
     * @param[out] pkey the public key which is read from the package. Must use X509_free(pkey) after use
	 * @returns true if successful
	 */
	static bool GetPublicKeyFromFilename(CString filePath, const char * serial, X509** pkey);

	/**
	 * Gets the RSA cert from the PKCS7 (PFX, P7B, PKCS7) file
	 * @param[in] filePath the path to the package
	 * @param[in] serialNumber the serial number of the cert
	 * @param[out] pkey the public key which is read from the package. Must use X509_free(pkey) after use
	 * @returns true if successful
	 */
	static bool GetRSAKeyDataFromPKCS7(CString filePath, const char * serialNumber, X509** pkey);

	/**
	 * Gets the RSA cert from the PKCS12 (PFX, PKCS12) file
	 * @param[in] filePath the path to the package
	 * @param[out] pkey the public key which is read from the package. Must use X509_free(pkey) after use
	 * @returns true if successful
	 */
	static bool GetRSAKeyDataFromPKCS12(CString filePath, X509** pkey);

	/**
	 * Gets the RSA key data from the appropriate RSA key.
	 * @param[in] filePath the filename of the RSA public key
	 * @param[out] data the buffer into which the function will set the contents of the RSA key
	 * @param[out] pkey the public key which is read from the package. Must use X509_free(pkey) after use
	 * @returns true if successful
	 */
	static bool GetRSAKeyData(CString filePath, X509** pkey);

	/**
	 * Retrieves all the certificates from the keystore at the file path
	 * @param[in] filePath The path at which the keystore resides
	 * @param[out] certs the stack of certificates within the keystore
	 */
	static bool GetCertificatesFromKeyStore(CString filePath, STACK_OF(X509) **certs);

	// Validation functions
	static bool ValidateFileRSA256(ValidateDataInputFileRSA input);

	static bool ValidateFileRSA256(ValidateDataInputFileRSACert &input);

	static bool ValidateDataRSA256(ValidateDataInputDataRSA input);

	static bool ValidateDataRSA256(ValidateDataInputDataRSACert &input);

	static bool ValidateFileHMAC256(ValidateDataInputFileHMAC input);

	static bool ValidateDataHMAC256(ValidateDataInputDataHMAC input);

	/**
	 * Reads the contents of the file and returns a buffer with the data. If buffer is NULL, the pointer returned by the function 
	 * has been newly allocated on the heap. Otherwise, the file contents will be placed into the buffer pointed to by buffer
	 * @param[in] filePath the path to the file
	 * @param[in,out] buffer the pointer to the buffer
	 * @param[in] offset the offset of the file to start reading
	 * @param[in,out] size the size of the buffer returned by the function if buffer is NULL, or the number of bytes read if buffer is not null.
	 * @returns the pointer to the buffer
	 */
	static unsigned char * GetFileContents(CString filePath, unsigned char * buffer, int offset, int &size);

	/**
	 * Generates a new UUID
	 * @param[in] includeHyphens flag to include hyphens (default: true)
	 * @returns new UUID
	 */
	static CString GetNewUUID(bool includeHyphens = true);

	/**
	 * Encodes given data to Base64 format
	 * @param[in] strDataHex the hex-string formatted data to encode to Base64
	 *            (i.e. "bffdcff17bed7037b2b015222a98a8a27ad83aae7ff9382f8e9d2ca09b53a212")
	 * @param[in] useBase64Url flag to use base64url format
	 * @returns Base64-encoded hexString
	 *          (i.e. "v/3P8XvtcDeysBUiKpioonrYOq5/+Tgvjp0soJtTohI=")
	 */
	static CString EncodeToBase64(CString strDataHex, bool useBase64Url = false);

	/**
	 * Generates HMACSHA256 result for given data using given HMAC key
	 * @param[in] strData the string data to calculate MAC
	 *            (i.e. "d230b3dfd007484ca712acbc580f6503D0000003_1")
	 * @param[in] strHmacKey hex-string formatted HMAC key
	 *            (i.e. "54dfeb2a27424db1b9879ad325fa0772")
	 * @returns hex-string formatted HMACSHA256 result
	 *          (i.e. "BFFDCFF17BED7037B2B015222A98A8A27AD83AAE7FF9382F8E9D2CA09B53A212")
	 */
	static CString GenerateHMACSHA256(CString strData, CString strHmacKey);

	/**
	 * Encrypts given data with public key using RSA encryption with OAEP padding
	 * (SHA256 as hash function, MGF1 (with SHA256) as mask generating function)
	 * @param[in] strData the string data to be encrypted
	 *            (i.e. "d230b3dfd007484ca712acbc580f6503D0000003_1")
	 * @param[in] strPublicKey a one-line public key string including line feed character ('\n') inclduing "BEGIN/END PUBLIC KEY"
	              (i.e. "-----BEGIN PUBLIC KEY-----\nMI ... ... ... AB\n-----END PUBLIC KEY-----")
	 * @returns hex-string formatted string
	 *          (i.e. "12E15556410B9E62F9F9DD080501BB...")
	 */
	static CString RSAEncryptWithPublicKey(CString strData, CString strPublicKey);

	static CString UCharToHexStr(unsigned char* data, int dataLen);
};