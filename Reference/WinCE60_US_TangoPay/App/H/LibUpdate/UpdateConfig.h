#pragma once

// Determines how the RSA key is loaded for rsa file signatures
#define RSA_KEY_MODE_SYSROOT		1  // Loads the key from sysroots.p7b using the TARGET_CERT_SERIAL as the cert
#define RSA_KEY_MODE_LOCALKEY		2  // Loads the key from /ATM2/pub.pem. ONLY USED FOR DEBUGGING

#ifdef UNDER_CE
#  define RSA_KEY_MODE	RSA_KEY_MODE_SYSROOT
#else
#  define RSA_KEY_MODE  RSA_KEY_MODE_LOCALKEY
#endif

// Ensures the validation mode is not insecure when building in release mode
#ifndef _DEBUG
#  if (RSA_KEY_MODE == RSA_KEY_MODE_LOCALKEY) 
#    // LOCALKEY should only be used for debugging, since the key can be replaces
//#    error Key mode is insecure for release mode
#  endif
#endif

#define TARGET_CERT_SERIAL "34780451062069440493643447953463395044"
#ifdef UNDER_CE
#  if (RSA_KEY_MODE == RSA_KEY_MODE_LOCALKEY)
#    define VALIDATION_RSA_CERT_FILE_SE60 L"\\ATM2\\pub.pem"
#  elif (RSA_KEY_MODE == RSA_KEY_MODE_SYSROOT)
#    define VALIDATION_RSA_CERT_FILE_SE60 L"\\Windows\\MX5300CE2ndCert.spc"
#    define VALIDATION_RSA_CERT_FILE_SE70 L"\\Windows\\ciroots.p7b"
#  endif
#else
#  if (RSA_KEY_MODE == RSA_KEY_MODE_LOCALKEY)
#    define VALIDATION_RSA_CERT_FILE_SE60 L".\\pub.spc"
#    define VALIDATION_RSA_CERT_FILE_SE70 L".\\pub.spc"
#  elif (RSA_KEY_MODE == RSA_KEY_MODE_SYSROOT)
#    error Validation mode is not valid for this platform
#  endif
#endif

inline CString GetCodeSigningKeyStoreFilePath();
inline CString GetCodeSigningKeyStoreFilePath()
{
	if (IsExistFile(VALIDATION_RSA_CERT_FILE_SE70))
	{
		return VALIDATION_RSA_CERT_FILE_SE70;
	}
	else if (IsExistFile(VALIDATION_RSA_CERT_FILE_SE60))
	{
		return VALIDATION_RSA_CERT_FILE_SE60;
	}

	return VALIDATION_RSA_CERT_FILE_SE60;
}