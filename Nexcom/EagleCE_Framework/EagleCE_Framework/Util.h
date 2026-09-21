#pragma once

//#include <GdiPlus.h>
#include <gdiplusenums.h>
#include <gdipluspixelformats.h>
#include <gdiplusimaging.h>

#include "zip.h"
#include "unzip.h"

#include "DES.h"

/************************************************************
* Utility Class
************************************************************/
class CUtil
{
public:
	// 응용 프로그램 경로 취득
	static CString	GetAppPath(void);

	// 응용 프로그램 경로 취득
	static CString	GetKeyPressWaveFilePath(void);

	static CString	GetNoticeWaveFilePath(void);

	// Get Product Name And Product Version
	static BOOL		GetProductAndVersion(CString strResourceLanguageID, CString& strProductName, CString& strProductVersion);

	// 
	static int		ParsingStringToStringArray(CString strSource, CString strSeparator, CStringArray &strarrDestination);

	// 
	static int		IsNumeric(CString strValue);

	// 
	static int		IsHexValue(CString strValue);

	//
	static int		StringToInt(CString strValue);

	//
	static CString	IntToString(int nValue);

	//
	static BOOL		StringToBOOL(CString strValue);
	//
	static CString	ConvertFromValueToAmountwithCent(CString strValue, BOOL bUseCent = TRUE);

	// Files
	static BOOL		IsExistFile(CString strFilePath);

	static BOOL		DeleteFileInDirectory(LPCTSTR lpDstPath, LPCTSTR lpDstFile);

	static BOOL		CopyFileInDirectory(LPCTSTR lpSrcPath, LPCTSTR lpSrcFile, LPCTSTR lpDstPath);

	//
	static void		Sleep_Wait(DWORD dwTime);

	//
	static UINT16	SwapEndian(UINT16 val);

	//
	static UINT32	SwapEndian24(UINT32 val);

	//
	static UINT32	SwapEndian(UINT32 val);

	//
	static BOOL		ZippingFromFolder(CString SrcFolder, CString ZipFileName);

	//
	static BOOL		ZippingFromFile(CString strSrcFolder, CString strSrcFileName, CString ZipFileName);

	//
	static BOOL		UnzippingToFolder(CString ZipFile, CString DestFolder);

	// zip wrapper function
	static HZIP		Wrapper_CreateZip(void *buf,unsigned int len, const char *password);

	//
	static HZIP		Wrapper_CreateZip(const TCHAR *fn, const char *password);

	//
	static ZRESULT	Wrapper_ZipAdd(HZIP hz,const TCHAR *dstzn, const TCHAR *fn);

	//
	static ZRESULT	Wrapper_ZipAdd(HZIP hz,const TCHAR *dstzn, void *src,unsigned int len);

	//
	static ZRESULT	Wrapper_ZipAddFolder(HZIP hz,const TCHAR *dstzn);

	//
	static ZRESULT	Wrapper_ZipGetMemory(HZIP hz, void **buf, unsigned long *len);

	//
	static ZRESULT	Wrapper_CloseZip(HZIP hz);

	// unzip function

	static HZIP		Wrapper_OpenZip(const TCHAR *fn, const char *password);

	//
	static HZIP		Wrapper_OpenZip(void *z,unsigned int len, const char *password);

	//
	static ZRESULT	Wrapper_GetZipItem(HZIP hz, int index, ZIPENTRY *ze);

	//
	static ZRESULT	Wrapper_UnzipItem(HZIP hz, int index, const TCHAR *fn);

	//
	static ZRESULT	Wrapper_UnzipItem(HZIP hz, int index, void *z,unsigned int len);

	//
	static UINT		Wrapper_FormatZipMessage(ZRESULT code, TCHAR *buf,unsigned int len);

	// Set IP Address
	static  BOOL	SetIPAddress(CString strIPAddress);

	// Set Gateway
	static  BOOL	SetGateway(CString strGateway);

	// Set Subnet Mask
	static  BOOL	SetSubnetMask(CString strSubnetMask);

	// Set DNS
	static  BOOL	SetDNS(CString strDNS);

	// Set DHCP
	static  BOOL	SetDHCP(DWORD dwDHCP);

	// Set LCD rotation information
	static	BOOL	SetLCDLotationInfo(DWORD dwValue);

	// Check IP Information
	static  BOOL	Check_IP_Information();

	// Get DHCP
	static  BOOL	GetDHCP();

	// Get Static IP Address
	static  BOOL	GetStaticIPAddress(CString &strIPAddress);

	// Get Static Subnet Mask
	static  BOOL	GetStaticSubnetMask(CString &strSubnetMask);

	// Get Static Gateway
	static  BOOL	GetStaticGateway(CString &strGateway);

	// Get Static DNS
	static  BOOL	GetStaticDNS(CString &strDNS);

	// Get DHCP IP ADDRESS
	static  BOOL	GetDHCP_IP(CString &strIPAddress);

	// Get DHCP Subnet Mask
	static  BOOL	GetDHCP_SubnetMask(CString &strSubnetMask);

	// Get DHCP GATEWAY
	static  BOOL	GetDHCP_GateWay(CString &strGateWay);

	// Get DHCP IP ADDRESS
	static  BOOL	GetDHCP_DNS(CString &strDNS);

	// Get LCD Rotation Information
	static	int		GetLCDLotationInfo();

	// Reset Ethernet Device
	static	BOOL	ResetEthernetDevice();

	static int		RegQueryValueExt(CString strKeyName, CString strValueName, DWORD dwType, DWORD dwLen, LPVOID lpszData);

	static int		RegSetValueExt(CString strKeyName, CString strValueName, DWORD dwType, DWORD dwLen, LPVOID lpszData, int nHKeyPos = 0);

	// Set Registry Value
	static	BOOL	Int_SetRegistry(CString strKeyName, CString strValueName, int nValue);

	// Set Registry Value
	static	BOOL	String_SetRegistry(CString strKeyName, CString strValueName, CString strValue);

	// Get Registry Value
	static	int		Int_GetRegistry(CString strKeyName, CString strValueName, int nValue = 0);

	// Get Registry Value
	static	CString	String_GetRegistry(CString strKeyName, CString strValueName);

	// Convert from Hex value to String
	static	CString	ConvertHexToString(BYTE *pByte, int nLength);

	// Make Pack
	static	BYTE	MakePack(BYTE bySrc);

	// Convert from String to Hex
	static	void	ConvertStringToHex(CString strSrc, BYTE *pByDest);

	static	CString IntToStringEx(int nValue, int nStringLen);

	static	BOOL	SetSWUpdateType(DWORD dwValue);		// 0 : USB를 통한 update, 1 : AMS 원격으로 update

	static	BOOL	GetSWUpdateType(DWORD &dwValue);

	static	BOOL	SetSWUpdateType(CString strValue);		// 0 : USB를 통한 update, 1 : OS Update, 2 : AMS 원격으로 update

	static	CString GetSWUpdateType();

	static	BOOL	GetUniqueKey(CString &strUniqueKey);

	// T-DES
	static void		EncryptKeyData(BYTE *pbyInData, int nInDataSize, BYTE *pbyOutData, BYTE *pbyKey);

	static void		DecryptKeyData(BYTE *pbyInData, int nInDataSize, BYTE *pbyOutData, BYTE *pbyKey);

	static void		GetCheckSumForCryptoKey(BYTE *pbyOutData, BYTE *pbyKey);

	// Base64
	static int		Base64_Encoded(char *text, int numBytes, char *encodedText);

	static int		Base64_Decoded(char *text, unsigned char *dst, int numBytes);

	static void		CreateFileFromResource(HINSTANCE hInstance, int nResID, CString strSaveFileName, CString strResourceFormat=RESOURCE_FORMAT_TXT);

	static BOOL		Check_Valid_IP_Format(CString strValue);

	static unsigned int crc16(unsigned char *string, int length);

};
