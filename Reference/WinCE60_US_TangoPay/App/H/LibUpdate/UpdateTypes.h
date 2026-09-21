#pragma once

#include ".\Common\CopyList.hpp"
#include "UpdateConfig.h"
#include <math.h>

/**
 * Macros
 */
#define SIGNATURE_MAX_LEN		512
#define FSIGTYPE_HMAC256_B64ENCODED		L"hmac256-b64"
#define FSIGTYPE_RSASHA256_B64ENCODED	L"rsa256-b64"
#define FSIGTYPE_HMAC256				L"hmac256"
#define FSIGTYPE_RSASHA256				L"rsa256"
#define FSIGTYPE_NONE					L"none"

// INI values for Authentication Types
#define AUTHN_NAME_NONE		L"none"
#define AUTHN_NAME_BASIC	L"basic"

/**
 * Values type declarations
 */

typedef CString RELEASETYPE;
const RELEASETYPE RT_BUGFIX		= L"bugfix";
const RELEASETYPE RT_SECURITY	= L"security";
const RELEASETYPE RT_FEATURE	= L"feature";

typedef int SEVERITY;
const SEVERITY SEV_MINOR	 = 1;

typedef int SIGNATURETYPE;
const SIGNATURETYPE ST_HMAC256		= 1;
const SIGNATURETYPE ST_RSASHA256	= 2;
const SIGNATURETYPE ST_NONE			= 3;

typedef int AUTHN_TYPE;
const AUTHN_TYPE AUTHN_NONE		= 0;
const AUTHN_TYPE AUTHN_BASIC	= 1;

#define SIGRSA256_LEN		256u
#define SIGHMACSHA256_LEN	32u

#define MANIFEST_VERSION_1_0	L"1.0"	// Deprecated Q1 2021
#define MANIFEST_VERSION_1_X	L"1."	// This covers v1.1, v1.2, etc.
//#define MANIFEST_VERSION_1_2_0		// Not implemented, yet
#define MANIFEST_VERSION_2		L"2."

#define PKG_PATH L"pkg"

#define COMPONENT_AP L"ap"
#define COMPONENT_EP L"ep"
#define COMPONENT_SP L"sp"

#define CONTENTS_FILE L"contents.xml"
#define MANIFEST_FILE L"manifest.xml"
#define MANIFEST_SIG_RSA256  L"manifest.sig.rsa256"
#define MANIFEST_SIG_HMAC256 L"manifest.sig.hmac256"
#define MANIFEST_SIG_PKCS7   L"manifest.p7"


#define REMOTE_UPDATE_INIT_FILE L"\\ATM\\DO_REMOTE_UPDATE"

/**
 * Convert a version number to a long for comparison
 */
long double VersionStringToLong(CString version);
inline long double VersionStringToLong(CString version)
{
	int base = 100;	  // Support decimal components between 0 and 99
	int exponent = 6; // The number of decimal places to support
	long double digit = 0;
	long double number = 0;
	CString curr;
	int pos = 0;

	while (pos != -1)
	{
		if (exponent == 0)
		{
			break;
		}

		curr = version.Tokenize(L".", pos);
		digit = Asc2Int(curr);
		if (digit != 0 && digit >= 1 && digit <= 99)
		{
			digit = digit * pow((long double)base, (long double) exponent);
		}

		number += digit;
		exponent--;
	}

	return number;
};

/**
 * Data Models
 */

// The configuration object
typedef struct UpdateSourceInfo {
	CString			RepositoryBaseURL;
	CStringArray	Hives;

	// NB: If this is blank, then the config should apply to all TIDs
	CString			TerminalID;
	int				MinimumSeverity;

	CString			Username;
	CString			Password;
	AUTHN_TYPE		AuthenticationType;

	UpdateSourceInfo()
	{
		MinimumSeverity = 0;
		AuthenticationType = 0;
	};

	void Clear()
	{
		RepositoryBaseURL.Empty();
		Hives.RemoveAll();
		TerminalID.Empty();
		MinimumSeverity = 1;
		Username.Empty();
		Password.Empty();
		AuthenticationType = 0;
	};

	CString GetApComponentURL() const
	{
		return GetComponentURL(COMPONENT_AP);
	};

	CString GetComponentURL(CString component) const
	{
		CString url;
		for (int i = 0; i < Hives.GetCount(); i++)
		{
			url.AppendFormat(L"/%s", Hives.GetAt(i));
		}

		// Add AP component path
		url.AppendFormat(L"/%s/%s", PKG_PATH, component);

		return url;
	};

	CString GetLatestComponentPackageURL(CString component) const
	{
		CString url;
		for (int i = 0; i < Hives.GetCount(); i++)
		{
			url.AppendFormat(L"/%s", Hives.GetAt(i));
		}

		// Add AP component path
		url.AppendFormat(L"/%s/%s/latest", PKG_PATH, component);

		return url;
	};

	CString GetLatestComponentPackageManifestURL(CString component) const
	{
		CString url = GetLatestComponentPackageURL(component);
		url.AppendFormat(L"/%s", MANIFEST_FILE);

		return url;
	};

	CString GetContentsURL(CString component) const
	{
		CString url = GetComponentURL(COMPONENT_AP);
		url.AppendFormat(L"/%s", CONTENTS_FILE);

		return url;
	};

	CString GetHivesRMSString() const 
	{
		CString hiveString;
		for (int i = 0; i < Hives.GetCount(); i++)
		{
			hiveString.AppendFormat(L"%s,", Hives.GetAt(i));
		}

		return hiveString.Left(hiveString.GetLength() - 1); // Omit the trailing comma
	};

	CString ToHumanReadableString() const 
	{
		CString data;
		CString hiveString;
		CString displayedURL = RepositoryBaseURL;

		for (int i = 0; i < Hives.GetCount(); i++)
		{
			hiveString.AppendFormat(L"%s ", Hives.GetAt(i));
		}

		// Hide default URL
		if (RepositoryBaseURL == L"nhasoftware.s3.us-east-2.amazonaws.com")
		{
			displayedURL = L"Default - NHA Software";
		}
		
		data.Format(L"[Update]\n\nHost = %s\nComponent = %s\nTerminal ID = %s\nHives = %s\nSeverity = %d",
			displayedURL,
			COMPONENT_AP,
			TerminalID,
			hiveString,
			MinimumSeverity);

		if (AuthenticationType == AUTHN_BASIC)
		{

			data.AppendFormat(L"\nUsername = %s\nPassword = **********\nAuth Type = %s",
				Username,
				AUTHN_NAME_BASIC);
		}

		return data;
	};

} *LPUpdateSourceInfo;

/**
 * Represents the contents of a particular software component repo. These are usually found in the contents.xml file.
 */
struct Package {
	CString Version;
	CString ReadableVersion;
	CString Path;
	
	CString Component;

	CString GetManifestURL(LPUpdateSourceInfo source) const
	{
		CString url = source->GetComponentURL(Component);
		url.AppendFormat(L"/%s/%s", Path, MANIFEST_FILE);

		return url;
	};

	CString GetBaseURL(LPUpdateSourceInfo source) const
	{
		CString url = source->GetComponentURL(Component);
		url.AppendFormat(L"/%s", Path);

		return url;
	};

	CString GetDisplayName() const
	{
		if (ReadableVersion.IsEmpty())
		{
			return Version;
		}

		return ReadableVersion;
	}
};

typedef CList<Package, Package&> PackageList;

typedef CCopyList<CString, CString&> PEMCertificates;

/**
 * Represents the certificates which have been used in the signing chain for the package/manifest
 */
struct CertificateChain {
	CString ChainId;
	PEMCertificates Certificates;
};

typedef CCopyList<CertificateChain, CertificateChain&> ChainList;

/**
 * Represents a manifest for a specific version of software
 */
struct Version {
	CString Lineage;
	CString Territory;
	CString Name;
};

struct ManifestMetadata {
	Version Version;
	CString ReadableVersion;
	SIGNATURETYPE SigType;

	CString Model;
	SEVERITY Severity;
	RELEASETYPE Type;

	// The path with which the package was accessed
	CString BasePath;

	// Certs used in the signing process
	ChainList Chains;
};

struct ManifestItemSignature {
	SIGNATURETYPE SigType;
	byte Signature[SIGNATURE_MAX_LEN];
	unsigned int SignatureSize;
	CString CertChainId;
	CString ItemName;

	#pragma warning (suppress:4351)
	ManifestItemSignature(): Signature()
	{
		SignatureSize = 0u; 
	};

	ManifestItemSignature& operator=(const ManifestItemSignature& rhs) 
	{ 
		SignatureSize = rhs.SignatureSize;
		SigType = rhs.SigType;
		CertChainId = rhs.CertChainId;
		ItemName = rhs.ItemName;

		memcpy_s(Signature, SignatureSize, rhs.Signature, rhs.SignatureSize);

		return *this;
	}

};

typedef CCopyList<ManifestItemSignature, ManifestItemSignature&> SignatureList;

struct ManifestItem {
	SIGNATURETYPE SigType;
	byte Signature[SIGNATURE_MAX_LEN];
	unsigned int SignatureSize;
	CString Filename;

	#pragma warning (suppress:4351)
	ManifestItem(): Signature()
	{
		SignatureSize = 0u; 
	};

	CString GetURL(CString manifestBasePath) const
	{
		CString url;
		url.Format(L"%s/%s", manifestBasePath, Filename);

		return url;
	};
};

struct Manifest {
	CString FileVersion;
	ManifestMetadata Metadata;
	CList<ManifestItem, ManifestItem&> Contents;
	SignatureList Signatures;

	CString GetSignatureFileName() const
	{
		if (FileVersion.Find(MANIFEST_VERSION_1_0) == 0)
		{
			switch (Metadata.SigType)
			{
			case ST_RSASHA256:
				return MANIFEST_SIG_RSA256;
			case ST_HMAC256:
				return MANIFEST_SIG_HMAC256;
			default:
				return L"";
			}
		}
		else if (FileVersion.Find(MANIFEST_VERSION_1_X) == 0)
		{
			return MANIFEST_SIG_PKCS7;
		}

		return L"";
	};

	bool GetSignature(CString chainId, CString itemName, ManifestItemSignature &signature) const
	{
		ManifestItemSignature sig;
		POSITION pos = Signatures.GetHeadPosition();
		while (pos != NULL)
		{
			sig = Signatures.GetNext(pos);
			if (sig.CertChainId == chainId && sig.ItemName == itemName) 
			{
				signature = sig;
				return true;
			}
		}

		return false;
	};
};

struct PreStagingResult {
	CString SoftwareDirectory;
};