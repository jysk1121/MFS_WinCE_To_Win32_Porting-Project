#include "stdafx.h"
#include "UpdateRepoService.h"

#include ".\Common\Markup.h"
#include ".\Common\NHDbgApi.h"

#define XML_SETUP(xml, data)					\
bool enterFailed = false;						\
{												\
	if (!xml.SetDoc(data))						\
	{											\
		return false;							\
	}											\
}

#define ENTER_CURRENT_XMLELEMENT	\
	xml.IntoElem();

#define ENTER_XMLELEMENT(name, mandatory)					\
{															\
	if (xml.GetTagName() == name)							\
	{														\
		xml.IntoElem();										\
	}														\
	else if (!xml.FindElem(name) || !xml.IntoElem())		\
	{														\
		enterFailed = true;									\
		if (mandatory) return false;						\
	}														\
} 

#define EXIT_XMLELEMENT(name)	\
{								\
	if (!enterFailed)			\
	{							\
		enterFailed = false;	\
		xml.OutOfElem();		\
	}							\
}

#define FOREACH_XMLELEM(block)								\
	while (!enterFailed && xml.FindElem())					\
	{														\
		block												\
	}

#define ON_XMLTAGNAME(name, block)			\
	if (xml.GetTagName() == name)			\
	{										\
		block								\
		continue;							\
	}

/*
 * Serialization
 */

/**
 * Deserialize the XML response into an array of package objects
 * @param[in] data the raw XML string
 * @param[in,out] packages a list of packages
 */
bool CUpdateRepoService::DeserializeContents(CString data, PackageList &packages, CString component)
{
	CMarkup xml;
	XML_SETUP(xml, data);

	ENTER_XMLELEMENT(L"contents", true);

	// Deserialize the items
	int curr = 0;
	FOREACH_XMLELEM(
	{
		ON_XMLTAGNAME(L"package", 
		{
			Package package;
			package.Version = xml.GetAttrib(L"version");
			package.Path = xml.GetAttrib(L"path");
			package.ReadableVersion = xml.GetData();
			package.Component = component;

			packages.AddTail(package);
			curr++;
		});
	});

	return true;
}

bool CUpdateRepoService::DeserializeManifest(CString data, Manifest &manifest)
{
	CMarkup xml;
	XML_SETUP(xml, data);

	xml.FindElem(L"manifest");
	manifest.FileVersion = xml.GetAttrib(L"version");
	xml.IntoElem();

	ENTER_XMLELEMENT(L"metadata", true);
	FOREACH_XMLELEM(
	{
		ON_XMLTAGNAME(L"version", 
		{
			manifest.Metadata.Version.Lineage = xml.GetAttrib(L"lineage");
			manifest.Metadata.Version.Territory = xml.GetAttrib(L"territory");
			manifest.Metadata.Version.Name = xml.GetData();
		});
		ON_XMLTAGNAME(L"version_human", { manifest.Metadata.ReadableVersion = xml.GetData(); });
		ON_XMLTAGNAME(L"model", { manifest.Metadata.Model = xml.GetData(); });
		ON_XMLTAGNAME(L"severity", { manifest.Metadata.Severity = Asc2Int(xml.GetData()); });
		ON_XMLTAGNAME(L"type", { manifest.Metadata.Type = xml.GetData(); });
		ON_XMLTAGNAME(L"sig_type", 
		{ 
			CString sigType = xml.GetData();
			if (sigType == FSIGTYPE_HMAC256)
			{
				manifest.Metadata.SigType = ST_HMAC256;
			}
			else if (sigType = FSIGTYPE_RSASHA256)
			{
				manifest.Metadata.SigType = ST_RSASHA256;
			}
			else if (sigType == FSIGTYPE_NONE)
			{
				manifest.Metadata.SigType = ST_NONE;
			}
		});

		ON_XMLTAGNAME(L"chains",
		{
			// Get all certs in chain
			ENTER_XMLELEMENT(L"chains", true);
			FOREACH_XMLELEM(
			{
				ON_XMLTAGNAME(L"chain",
				{
					CertificateChain chain;
					chain.ChainId = xml.GetAttrib(L"id");

					ENTER_CURRENT_XMLELEMENT
					FOREACH_XMLELEM(
					{
						ON_XMLTAGNAME(L"cert",
						{
							chain.Certificates.AddTail( xml.GetData() );
						});
					});
					EXIT_XMLELEMENT(L"chain");

					manifest.Metadata.Chains.AddHead(chain);
				});
			});
			EXIT_XMLELEMENT(L"chains");
		});
	});
	EXIT_XMLELEMENT(L"metadata");

	ENTER_XMLELEMENT(L"contents", true);
	FOREACH_XMLELEM(
	{
		ON_XMLTAGNAME(L"item", 
		{
			ManifestItem item;
			item.Filename = xml.GetData();
			CString sigType = xml.GetAttrib(L"sig_type");
			if (sigType == FSIGTYPE_HMAC256_B64ENCODED)
			{
				item.SigType = ST_HMAC256;
				item.SignatureSize = SIGHMACSHA256_LEN;

				// Convert and set signature
				CString b64Sig = xml.GetAttrib(L"sig");
				int b64Len = b64Sig.GetLength() + 1;
				char *b64 = new char[b64Len]();
				WideToMulti(b64, b64Sig, b64Len);
				base64_decode(b64, b64Len, item.Signature, sizeof(item.Signature));
				memset(b64, 0, b64Len);
				delete [] b64;
			}
			else if (sigType == FSIGTYPE_RSASHA256_B64ENCODED)
			{
				item.SigType = ST_RSASHA256;

				// Convert and set signature
				CString b64Sig = xml.GetAttrib(L"sig");
				int b64Len = b64Sig.GetLength() + 1;
				char *b64 = new char[b64Len]();
				WideToMulti(b64, b64Sig, b64Len);
				unsigned int sigSize = base64_decode(b64, b64Len, item.Signature, sizeof(item.Signature));
				memset(b64, 0, b64Len);
				delete [] b64;

				item.SignatureSize = sigSize;
			}
			else if (sigType == FSIGTYPE_NONE)
			{
				item.SigType = ST_NONE;
			}

			manifest.Contents.AddTail(item);
		});
	});
	EXIT_XMLELEMENT(L"contents");

	// Signatures!
	ENTER_XMLELEMENT(L"signatures", false);
	FOREACH_XMLELEM(
	{
		ON_XMLTAGNAME(L"sig",
		{
			ManifestItemSignature signature;

			signature.CertChainId = xml.GetAttrib(L"chain");
			signature.ItemName = xml.GetAttrib(L"item");

			CString sigType = xml.GetAttrib(L"sig_type");
			if (sigType == FSIGTYPE_RSASHA256_B64ENCODED)
			{
				signature.SigType = ST_RSASHA256;

				// Convert and set signature
				CString b64Sig = xml.GetData();
				int b64Len = b64Sig.GetLength() + 1;
				char *b64 = new char[b64Len]();
				WideToMulti(b64, b64Sig, b64Len);
				unsigned int sigSize = base64_decode(b64, b64Len, signature.Signature, sizeof(signature.Signature));
				memset(b64, 0, b64Len);
				delete [] b64;

				signature.SignatureSize = sigSize;
			}
			else if (sigType == FSIGTYPE_NONE)
			{
				signature.SigType = ST_NONE;
			}

			manifest.Signatures.AddHead(signature);
		});
	});
	EXIT_XMLELEMENT(L"item");

	return true;
}