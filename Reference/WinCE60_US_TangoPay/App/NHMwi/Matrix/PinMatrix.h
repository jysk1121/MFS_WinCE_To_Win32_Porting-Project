#ifndef _KALIGNITE_DEVICE_PIN_MATRIX_H_
#define _KALIGNITE_DEVICE_PIN_MATRIX_H_
///////////////////////////////////////////////////////////////////////////////

#include ".\Common\StructDef.h"

// ----------------------------------------------------------------------------
// KALIGNITE PIN ACITVEX METHOD
// ----------------------------------------------------------------------------
METHODLIST	PinMethod[] = 
{
	{"OpenSessionSync"				, "NO"  },					// 
	{"ImportKey"					, "YES" },					// LoadKey
	{"LoadEncryptedKey"				, "YES" },					// LoadEncryptedKey
	{"ExtendedLoadKey"				, "YES" },					// ExtendedLoadKey
	{"ExtendedLoadEncryptedKey"		, "YES" },					// ExtendedLoadEncryptedKey
	{"LoadEncryptedIV"				, "YES" },					// LoadEncryptedIV
	{"GenerateMAC"					, "YES" },					// GenerateMAC
	{"GenerateMACClearIV"			, "YES" },					// GenerateMACClearIV
	{"Reset"						, "YES" },					// Reset
	{"ReadData"						, "YES" },					// ReadData
	{"CancelReadData"				, "YES" },					// CancelReadData
	{"ReadPin"						, "YES" },					// ReadPin
	{"CancelReadPin"				, "YES" },					// CancelReadPin
	{"DecryptCBC"					, "YES" },					// DecryptCBC
	{"DecryptECB"					, "YES" },					// DecryptECB
	{"DecryptRSA"					, "YES" },					// DecryptRSA
	{"DecryptCBCClearIV"			, "YES" },					// DecryptCBCClearIV
	{"EncryptCBC"					, "YES" },					// EncryptCBC
	{"EncryptECB"					, "YES" },					// EncryptECB
	{"EncryptRSA"					, "YES" },					// EncryptRSA
	{"EncryptCBCClearIV"			, "YES" },					// EncryptCBCClearIV
	{"GetFixedPIN"					, "YES" },					// GetFixedPIN
	{"GetVariablePIN"				, "YES" },					// GetVariablePIN
	{"BuildPinBlock"				, "YES" },					// BuildPinBlock // [#68] UK HWANG 2008.04.02 EMV Level2 
	{"ExportRSAIssuerSignedItem"	, "YES" },					// ExportRSAIssuerSignedItem	// [#554] NH KSK 2009.08.10
	{"ImportRSAPublicKey"			, "YES" },					// ImportRSAPublicKey			// [#554] NH KSK 2009.08.10
	{"StartKeyExchange"				, "YES" },					// StartKeyExchange				// [#554] NH KSK 2009.08.10
	{"ImportRSASignedDESKey"		, "YES" },					// ImportRSASignedDESKey		// [#554] NH KSK 2009.08.10
	{"InitializeEDM"				, "YES" },					// InitializeEDM				// [#2057] NH KSK 2011.05.04
	{"GenerateKCV"					, "YES" },					// GenerateKCV					// [#2253] NH KSK 2014.02.21

	// [#RWC-151] US Kook 2020.07.20 TR34
	{"GetCertificate"				, "YES" },					// GetCertificate
	{"InitializeEDMEx"				, "YES" },					// InitializeEDMEx
	{"ImportKeyBlock"				, "YES" },					// ImportKeyBlock
	{"LoadCertificateEx"			, "YES" },					// LoadCertificateEx
	{"ImportRSAEncipheredPKCS7KeyEx", "YES" }					// ImportRSAEncipheredPKCS7KeyEx
	// end of [#RWC-151]
};

// ----------------------------------------------------------------------------
// KALIGNITE PIN ACITVEX EVENT
// ----------------------------------------------------------------------------
ITEMLIST	PinEvent[] =
{
	"OpenComplete"					,							// OpenSession 완료
	"KeyImported"					,							// KeyImported
	"IVLoaded"						,							// IVLoaded
	"MACingComplete"				,							// MACingComplete
	"KeyPressed"					,							// KeyPressed
	"ReadDataCancelled"				,							// ReadDataCancelled
	"ReadDataComplete"				,							// EntryComplete
	"ResetComplete"					,							// ResetComplete
	"Timeout"						,							// Timeout
	"DeviceError"					,							// DeviceError
	"FatalError"					,							// FatalError
	"PinEntered"					,							// PinEntered
	"ReadPinCancelled"				,							// ReadPinCancelled
	"PinBuffered"					,							// PinBuffered
	"ReadPinComplete"				,							// ReadPinComplete
	"EncryptComplete"				,							// EncryptComplete
	"DecryptComplete"				,							// DecryptComplete
	"PinBlockComplete"				,							// PinBlockComplete
	"RSAIssuerSignedItemExported"	,							// RSAIssuerSignedItemExported	[#554] NH KSK 2009.08.10
	"IllegalKeyAccessed"			,							// IllegalKeyAccessed	[#554] NH KSK 2009.08.10
	"RSAPublicKeyImported"			,							// RSAPublicKeyImported	[#554] NH KSK 2009.08.10
	"KeyDeleted"					,							// KeyDeleted			[#554] NH KSK 2009.08.10
	"StartKeyExchangeComplete"		,							// StartKeyExchangeComplete			[#554] NH KSK 2009.08.10
	"RSASignedDESKeyImported"		,							// RSASignedDESKeyImported			[#554] NH KSK 2009.08.10
	"EDMInitialized"				,							// EDMInitialized					[#2057] NH KSK 2011.05.04
	"KCVGenerated"					,							// KCVGenerated						[#2253] NH KSK 2014.02.21

	// [#RWC-6151] US Kook 2020.07.20 TR34
	"GetCertificateComplete"		,
	"LoadCertificateExComplete"		,
	"RSAEncipheredKeyExImported"	,
	"KeyBlockImported"				
	// end of [#RWC6-151]
};	

// ----------------------------------------------------------------------------
// KALIGNITE PIN ACITVEX METHOD/EVENT COUNT
// ----------------------------------------------------------------------------
static const int PinMethodCount = sizeof(PinMethod) / sizeof(METHODLIST);
static const int PinEventCount  = sizeof(PinEvent) / sizeof(ITEMLIST);

// ----------------------------------------------------------------------------
// KALIGNITE PIN ACITVEX METHOD/EVENT MATRIX
// ----------------------------------------------------------------------------
ITEMLIST	PinMatrix[PinMethodCount][PinEventCount] = 
{
/*	-----------------------------------------------------------------------------------------------------------------------------------------------------------
	"NEXTWARE PIN"					 /		"OpenComplete"
									 /				"KeyImported"
									 /						"IVLoaded"		
									 /								"MACingComplete"		
									 /										"KeyPressed"		
									 /												"ReadDataCancelled"		
									 /														"ReadDataComplete"
									 /																"ResetComplete"		
									 /																		"Timeout"		
									 /																				"DeviceError"		
									 /																						"FatalError"		
									 /																								"PinEntered"		
									 /																										"ReadPinCancelled"		
									 /																												"PinBuffered"		
									 /																														"ReadPinComplete"		
									 /																																"EncryptComplete"		
									 /																																		"DecryptComplete"		
									 /																																				"PinBlockComplete"
									 /																																						"RSAIssuerSignedItemExported"	- [#554] NH KSK 2009.08.10
									 /																																								"IllegalKeyAccessed"	- [#554] NH KSK 2009.08.10
									 /																																										"RSAPublicKeyImported"	- [#554] NH KSK 2009.08.10
									 /																																												"KeyDeleted"	- [#554] NH KSK 2009.08.10
									 /																																														"StartKeyExchangeComplete"	- [#554] NH KSK 2009.08.10
									 /																																																"RSASignedDESKeyImported"	- [#554] NH KSK 2009.08.10
									 /																																																		"EDMInitialized"	- [#645] NH KJW 2010.08.24
									 /																																																				"KCVGenerated"	- [#2253] NH KSK 2014.02.21
									 /																																																						"GetCertificateComplete"
									 /																																																								"LoadCertificateExComplete"
									 /																																																										"RSAEncipheredKeyExImported"
									 /																																																												"KeyBlockImported"
      --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
	/*"OpenSessionSync"				*/	{	"YES",	""   ,	""   ,	""   ,	""	 ,	""   ,	""   ,  ""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"ImportKey"					*/	{	""   ,	"YES",	""   ,	""   ,	""	 ,	""   ,	""   ,  ""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"LoadEncryptedKey"			*/	{	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,  ""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"ExtendedLoadKey"				*/	{	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,  ""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"ExtendedLoadEncryptedKey"	*/	{	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,  ""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"LoadEncryptedIV"				*/	{	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"GenerateMAC"					*/	{	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"GenerateMACClearIV"			*/	{	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"Reset"						*/	{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"ReadData"					*/	{	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	"YES",	""   ,	"YES",	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"CancelReadData"				*/	{	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"ReadPin"						*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	"YES",	""   ,	"YES",	""   ,	"YES",	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },		// [#68] UK HWANG 2008.04.02 EMV Level2  
	/*"CancelReadPin"				*/	{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"DecryptCBC"					*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"DecryptECB"					*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"DecryptRSA"					*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"DecryptCBCClearIV"			*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"EncryptCBC"					*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"EncryptECB"					*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"EncryptRSA"					*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"EncryptCBCClearIV"			*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"GetFixedPIN"					*/	{	""   ,	""   ,	""	 ,	""   ,	"YES",	""   ,	""   ,	""   ,	"YES",	"YES",	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"GetVariablePIN"				*/	{	""   ,	""   ,	""	 ,	""   ,	"YES",	""   ,	""   ,	""   ,	"YES",	"YES",	"YES",	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"BuildPinBlock"				*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },		// [#68] UK HWANG 2008.04.02 EMV Level2 
	/*"ExportRSAIssuerSignedItem"	*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },		// [#554] NH KSK 2009.08.10
	/*"ImportRSAPublicKey"			*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },		// [#554] NH KSK 2009.08.10
	/*"StartKeyExchange"			*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },		// [#554] NH KSK 2009.08.10
	/*"ImportRSASignedDESKey"		*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	"YES",	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   },		// [#554] NH KSK 2009.08.10
	/*"InitializeEDM		"		*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   },		// [#2057] NH KSK 2011.05.04
	/*"GenerateKCV			"		*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   },		// [#2253] NH KSK 2014.02.21

	/*"GetCertificate"				*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   },		// [#RWC6-151] US Kook 2020.07.20 TR34
	/*"InitializeEDMEx"				*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   },		// 
	/*"ImportKeyBlock"				*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES"},		// 
	/*"LoadCertificateEx"			*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   },		// 
	/*"ImportRSAEncipheredPKCS7KeyEx"*/	{	""   ,	""   ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   }		// 	
	//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
};

// ============================================================================
///////////////////////////////////////////////////////////////////////////////
#endif // _KALIGNITE_DEVICE_PIN_MATRIX_H_

