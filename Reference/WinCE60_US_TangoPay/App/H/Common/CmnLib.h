//////////////////////////////////////////////////////////////////////
#if !defined(_CMNLIB_H_INCLUDED_)
#define _CMNLIB_H_INCLUDED_

#define DllImport			__declspec(dllimport)
#define DllExport			__declspec(dllexport)

#ifndef _CMNLIB_OWNER_
#define DllUseport			DllImport
#pragma comment(lib, "Cmnlib.lib")
#else
#define DllUseport			DllExport
#endif

#if UNDER_CE
#  pragma comment (lib, "ws2.lib")
#else
#  pragma comment (lib, "Version.lib")
#  pragma comment (lib, "ws2_32.lib")
#endif

// OpenSSL
#pragma comment (lib, "libcrypto.lib")
#pragma comment (lib, "libssl.lib")

#include "CeFileFind.h"
#include "..\Dev\DevDefine.h"	// [#338] [NH] KSK 2008.6.25
#include "Repository.h"
#include "NHConfig.h"
#include "ConstDef.h"	// [#2388] US Justin include Constant
#include ".\..\..\Dll\CmnLib\zip.h"	// [#2012] zip, unzip을 CmnLib로 공용화함
#include ".\..\..\Dll\CmnLib\unzip.h"

#ifdef UNDER_CE
#  define FILE_EXISTS IsExistFile
#else
#  include "Shlwapi.h"
#  define FILE_EXISTS PathFileExists
#endif

//////////////////////////////////////////////////////////////////////
//	ASCII
//////////////////////////////////////////////////////////////////////
#define	STRBUFFSIZE			4096

//////////////////////////////////////////////////////////////////////
//	ASCII
//////////////////////////////////////////////////////////////////////
#define A_NUL				0x00
#define A_SOH				0x01
#define A_STX				0x02
#define A_ETX				0x03
#define A_EOT				0x04
#define A_ENQ				0x05
#define A_ACK				0x06
#define A_BEL				0x07
#define A_BS				0x08
#define A_HT				0x09
#define A_LF				0x0a
#define A_VT				0x0b
#define A_FF				0x0c
#define A_CR				0x0d
#define A_SO				0x0e
#define A_SI				0x0f
#define A_DLE				0x10
#define A_DC1				0x11
#define A_DC2				0x12
#define A_DC3				0x13
#define A_DC4				0x14
#define A_NAK				0x15
#define A_SYN				0x16
#define A_ETB				0x17
#define A_CAN				0x18
#define A_EM				0x19
#define A_SUB				0x1a
#define A_ESC				0x1b
#define A_FS				0x1c
#define A_GS				0x1d
#define A_RS				0x1e
#define A_US				0x1f

//////////////////////////////////////////////////////////////////////
//	ZERO & SPACE
//////////////////////////////////////////////////////////////////////
#define ZERO				"0"
#define ZERO2				"00"
#define ZERO3				"000"
#define ZERO4				"0000"
#define ZERO5				"00000"
#define ZERO6				"000000"
#define ZERO7				"0000000"
#define ZERO8				"00000000"
#define ZERO9				"000000000"
#define ZERO10				"0000000000"
#define ZERO11				"00000000000"
#define ZERO12				"000000000000"
#define ZERO13				"0000000000000"
#define ZERO14				"00000000000000"
#define ZERO15				"000000000000000"
#define ZERO16				"0000000000000000"

#define SPACE				" "
#define SPACE2				"  "
#define SPACE3				"   "
#define SPACE4				"    "
#define SPACE5				"     "
#define SPACE6				"      "
#define SPACE7				"       "
#define SPACE8				"        "
#define SPACE9				"         "
#define SPACE10				"          "
#define SPACE11				"           "
#define SPACE12				"            "
#define SPACE13				"             "
#define SPACE14				"              "
#define SPACE15				"               "
#define SPACE16				"                "

// [#554] KSK 2009.08.11
// PIN 512K 지원으로 인해 KEY MODE DEFINE 위치 변경 TranCmnDefine.h -> CmnLib.h
/////////////////////////////////////////////////////////////////////////////
//	Keymode
/////////////////////////////////////////////////////////////////////////////
/*keymode,									key1,			key2,			key3,			key4			key5
-----------------------------------------------------------------------------------------------------------------------
0:Non Unique Key, DES, Single Master Key	Working Key		-				-				-				-
1:Non Unique Key, DES, Dual Master Key		Working Key		C-Key			-				-				-
2:Unique Key, DES							Working Key		-				-				-				-
3:Non Unique Key, TDES						Working Key1	Working Key2	Working Key3	-				-
4:Unique Key, TDES							Working Key1	Working Key2	Working Key3	-				-
5:Non Unique Key, Macing					Working Key		-				-				Macing Key		-
6:Unique Key, Macing						Working Key		-				-				Macing Key		-
7:TDES, Macing								Working Key1	Working Key2	Working Key3	Macing Key		-
11:TDES, TMacing							Working Key1	Working Key2	Working Key3	Macing key1		Macing Key2
12:TR31										Working KB		-				-				-				-
13:TR31, MACing								Working KB		MACing KB		-				-				-
*/
#define KEYMODE_NON_UNIQ_SDES			0	// 0:Non Unique Key, DES, Single Master Key	
#define KEYMODE_NON_UNIQ_DDES			1	// 1:Non Unique Key, DES, Dual Master Key		
#define KEYMODE_UNIQ_SDES				2	// 2:Unique Key, DES							
#define KEYMODE_NON_UNIQ_TDES			3	// 3:Non Unique Key, TDES						
#define KEYMODE_UNIQ_TDES				4	// 4:Unique Key, TDES							
#define KEYMODE_NON_UNIQ_SDES_MACING	5	// 5:Non Unique Key, Macing					
#define KEYMODE_UNIQ_SDES_MACING		6	// 6:Unique Key, Macing						
#define KEYMODE_TDES_MACING				7	// 7:TDES, Macing								
#define KEYMODE_TDES_TMACING			11	// 11:TDES, TMacing
#define KEYMODE_TR31					12	// 12:TR31
#define KEYMODE_TR31_MACING				13	// 13:TR31, MACing
#define KEYMODE_TR31_TMACING			14	// 14:TR31, TMACing
// end of [#554]

// [#2452] NH KSK 2016.11.10
#define SERIAL_LOG_TYPE		1
#define FILE_LOG_TYPE		2
// end of [#2452]

// [#2205] US KSK 2013.06.28
//Halo LED COLOR
#define HALOLED_COLOR_CYAN			0x0
#define HALOLED_COLOR_BLUE			0x1
#define HALOLED_COLOR_YELLOWGREEN	0x2
#define HALOLED_COLOR_WHITE			0x3
#define HALOLED_COLOR_GREEN			0x4
#define HALOLED_COLOR_PINK			0x5
#define SKIP_HALOLED_COLOR			0x6

//Halo LED MODEs
#define OFF_HALOLED_MODE			0x0		
#define ON_HALOLED_MODE				0x1
#define FLICKING_HALOLED_MODE		0x2
#define FADE_HALOLED_MODE			0x3
#define SKIP_HALOLED_MODE			0x4

//MCU LED MODE
#define OFF_MCULED					0x0
#define ON_MCULED					0x1
#define FLICKER_MCULED				0x2
#define SKIP_MCULED					0x3
// end of [#2205]

// [#2234] NH KMK 2014.02.04 MoniView Timeout
#define TIMEOUT_RMS_CONNECT_TCPIP	30
#define TIMEOUT_RMS_RECEIVE_TCPIP	10
// end of [#2234]

// [#2326] US Kook 2015.08.28 Support MoniAir
#define TYPE_LAN					0
// end of [#2326]

////////////////////////////////////////
// definitions for NVDump() parameter
// todo : make them have the best naming rule

// 1st - status
#define NVP1_ERROR							'F'		// status
#define NVP1_OTHER							'O'

// fixed - unit1
#define NVP_UNIT1_AP						'1'

// 2nd - unit2
#define NVDUMP_UNIT2_MAINFRM				'A'
#define NVDUMP_UNIT2_WINATMVIEW				'B'
#define NVP2_TRANCTRL						'C'
#define NVDUMP_UNIT2_SCRCTRL				'D'
#define NVDUMP_UNIT2_DEVCTRL				'E'
#define NVDUMP_UNIT2_NETCTRL				'G'
#define NVDUMP_UNIT2_NETCTRL_TAPI			'H'
#define NVDUMP_UNIT2_NETCTRL_MODEM			'I'
#define NVDUMP_UNIT2_NETCTRL_CCOMM			'J'
#define NVDUMP_UNIT2_NETCTRL_VISAII			'K'
// 'L' not used
#define NVDUMP_UNIT2_MWI					'M'
#define NVDUMP_UNIT2_MWI_DEVCTRL			'N'
#define NVDUMP_UNIT2_MWI_EVTQUE				'O'
#define NVDUMP_UNIT2_MWI_MATRIX				'P'
#define NVDUMP_UNIT2_ADACTRL				'Q'
// 'R' not used
#define NVDUMP_UNIT2_TANGOPAY				'S'		// [RWC6-676] SKKim 2024.03.27
#define NVDUMP_UNIT2_NETCTRL_RMS			'T'

// 3rd - function
#define NVP3_TR34							"TR"

// [RWC6-676] Start SKKim 2024.03.27
#define NVP3_TANGPAY_GET_PUBLICKEY			"01"
#define NVP3_TANGPAY_GET_JWT				"02"
#define NVP3_TANGPAY_GET_USER				"03"
#define NVP3_TANGPAY_ENTER_PHONE			"04"
#define NVP3_TANGPAY_ENTER_OTP				"05"
#define NVP3_TANGPAY_PRESTAGE_VERIFY		"06"
#define NVP3_TANGPAY_STAGE_VALIDATION		"07"
#define NVP3_TANGPAY_PRE_TRANS_SUMMARY		"08"
#define NVP3_TANGPAY_FRAUD_WARNING			"09"
#define NVP3_TANGPAY_DISCLOURE				"10"
#define NVP3_TANGPAY_ACCEPT_CASH			"11"
#define NVP3_TANGPAY_TRANS_SUMMARY			"12"
#define NVP3_TANGPAY_COMMIT					"13"
#define NVP3_TANGPAY_TRANS_COMP				"14"
#define NVP3_TANGPAY_PRINT_RECEIPT			"15"
#define NVP3_TANGPAY_SELECTLANGUAGE			"16"
#define NVP3_TANGPAY_CANCELCONFIRM			"17"
#define NVP3_TANGPAY_ERRORPRINT				"18"
#define NVP3_TANGPAY_DOWN_APP_GUIDE			"19"
// [RWC6-676] End SKKim 2024.04.02


// 4th - error code
#define NVP4_NONE							L""

// end of NVDump() definitions
////////////////////////////////////////


// [#419] [NH] KSK 2008.9.11
typedef struct tagErrorInfo{		// Error Stack Detail
	char ProcCount[ERRORPROCCOUNT_SIZE];		// Procedure Count
	char ErrorCode[ERRORCODE_SIZE];				// Error Code
	char ErrorString[ERRORSTRING_SIZE];			// Error String
} ERRSTACK, NEAR* NPERRSTACKINFO, FAR* LPERRSTACKINFO;
// end of [#419]

//////////////////////////////////////////////////////////////////////
//	Delay.cpp
//////////////////////////////////////////////////////////////////////
DllUseport	int		WINAPI Delay_Msg(DWORD time = 10);
DllUseport	ULONG	WINAPI TimerSet(int nSecond);
DllUseport	int		WINAPI CheckTimer(ULONG nSetTime);

/**
 * Finds the section and key in the INI file in FileName and provides the value in a 
 * newly allocated string in Value. Caller is responsible for freeing the memory allocated
 * by this function with `delete []`
 *
 * @param[in] FileName a pointer to a null-terminated string representing the filename of the INI file
 * @param[in] Section a pointer to a null-terminated string representing the INI section name
 * @param[in] Key a pointer to a null-terminated string representing the INI key name
 * @param[out] Value a double pointer to a pointer which will contain the a null-terminated string containing the value of the 
 *   INI key. If the function call is unsuccessful, this value will be set to NULL
 * @returns 1 on success and -1 on failure.
 */
DllUseport	int		WINAPI GetIniString(char *FileName, char *Section, char *Key, char **Value);
DllUseport	int		WINAPI SetIniString(char *FileName, char *Section, char *Key, char *Value);

DllUseport	CString WINAPI GetPStr(char *FileName, char *Section, char *Key, char *Default);
DllUseport	int		WINAPI GetPInt(char *FileName, char *Section, char *Key, int nDefault);

DllUseport CString WINAPI GetFileReadVersion(CString strLocalFileName, BOOL bUseLowVersion = FALSE);	// [#151] KSK 2008.04.21 File Version Read 함수 추가

//	[#81] UK KGS 2008.04.07 AID List 
class AFX_CLASS_EXPORT CINIFile 
{
private:
	char* m_szFileName;
	FILE* fp;
	
	BOOL GetNextSection(char* szBuf);
	BOOL GetNextKey(char* szBuf);
	BOOL GetValue(char* szBuf);

public:
	
	BOOL GetPrivateProfileString(char* lpAppName, char* lpKeyName, char* lpReturnedString, DWORD nSize);

	CINIFile(char* szFileName);
	
};
// end of [#81]

// [#2373] US Justin 2015.09.22 Support Barcode Printing
class COneStripe
{
public:
	COneStripe();
	virtual ~COneStripe();

private:
	float m_fLocX, m_fWidth;

public:
	void GetStripeInfo(float*fLoc, float*fWidth);
	void AssignLocAndWidth(float fLocX, float fWidth);
};

class CBarStripe  
{
public:
	CBarStripe();
	virtual ~CBarStripe();

private:
	COneStripe* m_pStripes;
	CSize m_szBarStripe;

	int m_nStripeNumber;
	void  InitializeData();
	
public:
	CSize GenerateStripes(int nWidth, int nHeight, LPCTSTR sData);
	int	  GetNumberOfStripe();
	BOOL  GetStripeInfo(int nOrder, int*nLocX, int*nWidth, int*nHeight);
};

DllUseport	BOOL	WINAPI CreateNumberBarcode(	LPCTSTR strData, int nImgWidth, int nImgHeight, LPCTSTR strFileName);
// End of [#2373]
DllUseport	void	WINAPI SaveScreenToFile(LPCTSTR szFileName);

// [#189] NH KGS 2008.05.09 저널 Clear 후 -1값 표시 오류 수정
#define MUST_BE_POSITIVE_NUMBER(NUMBER) \
		(NUMBER < 0 ? 0 : NUMBER)
// end of [#189]

//////////////////////////////////////////////////////////////////////
//	Registry.cpp
//////////////////////////////////////////////////////////////////////
#define REG_INT				REG_DWORD_LITTLE_ENDIAN
#define REG_STR				REG_SZ

DllUseport	int		WINAPI RegQueryValueExt(	LPCTSTR hKeyName, 
												LPCTSTR lpszValueName, 
												DWORD dwType,
					 							DWORD dwLen, 
												LPVOID lpszData);
// [#2325] NH KSK 2015.01.28 HKEY_USER쪽 접근을 위해 함수 Parameter 변경
//DllUseport	int		WINAPI RegSetValueExt(		LPCTSTR hKeyName, 
//												LPCTSTR lpszValueName, 
//												DWORD dwType, 
//												DWORD dwLen, 
//												LPVOID lpszData);
DllUseport	int		WINAPI RegSetValueExt(		LPCTSTR hKeyName, 
										  LPCTSTR lpszValueName, 
										  DWORD dwType, 
										  DWORD dwLen, 
										  LPVOID lpszData,
										  int	nHKeyPos = 0);
// end of [#2325]

DllUseport	int		WINAPI RegGetInt(LPCTSTR hKeyName, LPCTSTR lpszValueName, int InitValue = NULL);
DllUseport	CString	WINAPI RegGetStr(LPCTSTR hKeyName, LPCTSTR lpszValueName, LPCTSTR InitValue = NULL);

// [#2325] NH KSK 2015.02.06
//DllUseport	int		WINAPI RegSetInt(LPCTSTR hKeyName, LPCTSTR lpszValueName, int SetValue);
//DllUseport	int		WINAPI RegSetStr(LPCTSTR hKeyName, LPCTSTR lpszValueName, CString SetValue);
DllUseport	int		WINAPI RegSetInt(LPCTSTR hKeyName, LPCTSTR lpszValueName, int SetValue, int nHKeyPos = 0);
DllUseport	int		WINAPI RegSetStr(LPCTSTR hKeyName, LPCTSTR lpszValueName, CString SetValue, int nHKeyPos = 0);
// end of [#2325]

/////////////////////////////////////////////////////////////////////////////////////////////////
DllUseport	int		WINAPI AtmGetIpAddress(CString& IpAddr, int type = TYPE_LAN);
DllUseport	int		WINAPI AtmGetGateWay(CString& pGateway, int type = TYPE_LAN);
DllUseport	int		WINAPI AtmGetSubnetMask(CString& pSubnet, int type = TYPE_LAN);
//DllUseport	int		WINAPI AtmGetDNS(CString& pDNS, int type = TYPE_LAN);
DllUseport	int		WINAPI AtmGetDNS(CString& pDNS, CString& pDNS2, int type = TYPE_LAN); // [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
// [#404] [NH] KSK 2008.8.21
DllUseport	int		WINAPI AtmGetDhcpIpAddress(CString& IpAddr, int type = TYPE_LAN);
DllUseport	int		WINAPI AtmGetDhcpGateWay(CString& pGateway, int type = TYPE_LAN);
DllUseport	int		WINAPI AtmGetDhcpSubnetMask(CString& pSubnet, int type = TYPE_LAN);
//DllUseport	int		WINAPI AtmGetDhcpDNS(CString& pDNS, int type = TYPE_LAN);
DllUseport	int		WINAPI AtmGetDhcpDNS(CString& pDNS, CString& pDNS2, int type = TYPE_LAN); // [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
// end of [#404]

DllUseport	int		WINAPI AtmSetIpAddress(LPCTSTR pIpAddr);
DllUseport	int		WINAPI AtmSetGateWay(LPCTSTR pSubnet);
DllUseport	int		WINAPI AtmSetSubnetMask(LPCTSTR pGateway);
//DllUseport	int		WINAPI AtmSetDNS(LPCTSTR pDNS);
DllUseport	int		WINAPI AtmSetDNS(LPCTSTR pDNS, LPCTSTR pDNS2); // [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
DllUseport	int		WINAPI AtmSetDHCP(DWORD pDHCP, int type = TYPE_LAN);

DllUseport	int		WINAPI ReloadLanCard();
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//	Trace.cpp
//////////////////////////////////////////////////////////////////////
DllUseport  void	WINAPI NHDebugTimeStampPrint(void);
DllUseport  void	WINAPI NHDebugPrintf(LPCWSTR lpszFmt, ...);
DllUseport	void	WINAPI NVDump(char cStatus, char szSubUnit, char* szFunction, LPCTSTR szErr, LPCTSTR szDummy, int nTraceLevel=0);
DllUseport	void	WINAPI NVDumpF(char cStatus, char szSubUnit, char* szFunction, LPCTSTR szErr, LPCWSTR lpszFmt, ...);
DllUseport	void	WINAPI HexaDump(BYTE *pData, int nLen, int nSection, int nDumpType, BOOL bConvertedBinary, BOOL bInitFlag = FALSE);	// [#2452] NH KSK 2016.11.10 Log관련 함수이므로 Trace.cpp로 위치 이동(HexDump_byCha  함수명 변경)
DllUseport	CString	WINAPI String2Hex(CString input);

DllUseport	BOOL	WINAPI DeleteFileInDirectory(LPCTSTR lpDstPath, LPCTSTR lpDstFile);

// [#2186] US KSK 2013.04.30 Font File Copy를 위해 함수 추가
DllUseport	BOOL	WINAPI IsExistFile(CString strFilePath);
DllUseport	BOOL	WINAPI CountFileInDirectory(LPCTSTR lpDstPath, LPCTSTR lpDstFile, int &nCount);
DllUseport	BOOL	WINAPI CopyFileInDirectory(LPCTSTR lpSrcPath, LPCTSTR lpSrcFile, LPCTSTR lpDstPath);
/**
 * Walks the directory tree starting at the provided directory and provides a list
 * of all files.
 * @param lpSrcPath[in] the starting point of the directory walk
 * @param paFileList[out] a list of file paths in the directory
 * @returns true if the operation was successful
 */
DllUseport	BOOL	WINAPI GetFilesInDirectory(LPCTSTR lpSrcPath, CStringArray &paFileList);
// end of [#2186]

DllUseport	BOOL	WINAPI CreateDirectoryWithIntermediate(LPCTSTR strDstPath);
DllUseport  BOOL	WINAPI ExtractZIPAll(CString pZipFileName, CString pDestFolder, bool pDeleteZipAfterExtract = false);		// [#2545] US Justin 2018.04.13

DllUseport VOID		WINAPI WriteAlphiLogToFile(CString pMessage); // [#2497] NH woooZ 2017.09.26  ALPHI server

//////////////////////////////////////////////////////////////////////
//	TypeCvt.cpp
//////////////////////////////////////////////////////////////////////
DllUseport	char*	WINAPI Strcpy(void* dBuffc, void* sBuffc);
DllUseport	char*	WINAPI Strcpy(void* dBuffc, LPCTSTR sBuffc);
DllUseport	char*	WINAPI Strcpy(void* dBuffc, void* sBuffc,   int slen);
DllUseport	char*	WINAPI Strcpy(void* dBuffc, LPCTSTR sBuffc, int slen);
DllUseport	char*	WINAPI StrcpyRight(void* dBuffc, void* sBuffc,   int slen, int dlen, char InitChar = '0');
DllUseport	char*	WINAPI StrcpyRight(void* dBuffc, LPCTSTR sBuffc, int slen, int dlen, char InitChar = '0');
DllUseport	LPCTSTR WINAPI Strcpy(CString& dBuffc, void* sBuffc);
DllUseport	LPCTSTR WINAPI Strcpy(CString& dBuffc, void* sBuffc, int slen);
DllUseport	int		WINAPI Strlen(void* sBuffc);
DllUseport	void	WINAPI AppendFormatCenter(CString& dest, int width, LPCTSTR format, ...);
DllUseport	CString	WINAPI CenterLines(CString data, int width, CString sep);

DllUseport	int		WINAPI Asc2Int(void* lpvoid, int length);
DllUseport	int		WINAPI Asc2Int(LPCTSTR lpvoid, int length);
DllUseport	int		WINAPI Asc2Int(LPCTSTR lpvoid);
DllUseport	int		WINAPI Asc2IntHighOrder(LPCTSTR lpvoid, void* kBuffc, int inLen, int outLen);
DllUseport	double	WINAPI Asc2Float(void* lpvoid, int length);
DllUseport	double	WINAPI Asc2Float(LPCTSTR lpvoid, int length);
DllUseport	double	WINAPI Asc2Float(LPCTSTR lpvoid);

// [#2317] US Justin 2015.01.05 Add Add Long integer Conversion.
DllUseport	long	WINAPI Asc2Long(void* lpvoid, int length);
DllUseport	long	WINAPI Asc2Long(LPCTSTR lpvoid, int length);
DllUseport	long	WINAPI Asc2Long(LPCTSTR lpvoid);
// End of [#2317]

DllUseport	long long	WINAPI Asc2LongLong(void* lpvoid, int length);
DllUseport	long long	WINAPI Asc2LongLong(LPCTSTR lpvoid, int length);
DllUseport	long long	WINAPI Asc2LongLong(LPCTSTR lpvoid);


DllUseport	CString	WINAPI Asc2String(void* lpvoid, int inLen,int outLen);
DllUseport	char*	WINAPI Int2Asc(int Value, void* Target, int len, int mode = 10, char InitChar = '0');
DllUseport	CString	WINAPI Int2Asc(int Value, int len = 0, int mode = 10, char InitChar = '0');
//DllUseport  char*   WINAPI ULong2Asc(ULONG Value, void *Target, int len, int mode, char InitChar = '0');	[#2022] NH KSK 2011.02.22 Code Sonar 지적사항 대책으로 사용 안하는 함수 주석처리

DllUseport	int		WINAPI SplitString(CString cstrSrc, CString cstrSplit, CStringArray& acstrRet);
DllUseport	CString	WINAPI GetString(void* inbuff, int inLen,int outLen);
DllUseport	int		WINAPI Compress(void* sBuff, int slen);
DllUseport	int		WINAPI MarkAmount(void* inbuff,int inLen, char Value);
DllUseport	int		WINAPI EditString(void* inBufc, int inLen, char* fmt, void* outBufc, char MoveChar = '#' , char HiddenChar = '*' , char HidenMoveChar = '*' , int HiddenFlg = TRUE);
DllUseport	int		WINAPI EditString(void* inBufc, int inLen, char* fmt, CString& outBufc, char MoveChar = '#' , char HiddenChar = '*' , char HidenMoveChar = '*' , int HiddenFlg = TRUE);
DllUseport	int		WINAPI EditString(LPCTSTR inBufc, char* fmt, CString& outBufc, char MoveChar = '#' , char HiddenChar = '*' , char HidenMoveChar = '*' , int HiddenFlg = TRUE);
DllUseport	CString	WINAPI EditString(void* inBufc, int inLen, char* fmt, char MoveChar = '#' , char HiddenChar = '*' , char HidenMoveChar = '*' , int HiddenFlg = TRUE);
DllUseport	CString	WINAPI EditString(LPCTSTR inBufc, char* fmt, char MoveChar = '#' , char HiddenChar = '*' , char HidenMoveChar = '*' , int HiddenFlg = TRUE);
//DllUseport	int		WINAPI Sprintf(void* dst,int len,char* fmt,...);	[#2022] NH KSK 2011.02.22 Code Sonar 지적사항 대책으로 사용 안하는 함수 제거
//DllUseport	CString	WINAPI GetSprintf(char* fmt,...);			KSK 2012.03.01 Code Snonar 지적사항 대책
DllUseport	int		WINAPI FindChar(void* inbuf,int len, char chr);
DllUseport	int		WINAPI FindNotNum(void* inbuf,int len);
DllUseport	int		WINAPI IsChar(void* inbuf,int len, char chr);
DllUseport	int		WINAPI IsSpace(void* inbuf,int len);
DllUseport	int		WINAPI IsZero(void* inbuf,int len);
DllUseport	int		WINAPI IsZero(CString strString);		// [#] AIREAT 2009.03.10
DllUseport	int		WINAPI IsNum(void* inbuf,int len);
DllUseport	int		WINAPI IsNum(CString strInbuf);			// KSK 2010.01.04 CString형 Numeric Check 함수 추가
DllUseport	int		WINAPI IsUnPack(void* inbuf,int len);
DllUseport	int		WINAPI Zero2Space(void* sBuff, int slen);
DllUseport	int		WINAPI IsNull(void *inbuf,int len);
DllUseport	int		WINAPI Lower2Upper(void* sBuff, int slen);
DllUseport	int		WINAPI Upper2Lower(void* sBuff, int slen);
DllUseport  int		WINAPI LTrim(void *inbuf, int len, int ch);

DllUseport	void	WINAPI Byte2Asc(BYTE byteptr, void* strptrc, int mode);
DllUseport	CString	WINAPI Byte2Asc(BYTE byteptr, int mode);
DllUseport	void	WINAPI Byte2Ascn(BYTE *byteptr, void* strptrc, int blen, int mode);
DllUseport	CString	WINAPI Byte2Ascn(BYTE *byteptr, int blen, int mode);
DllUseport  int		WINAPI ByteHighOrder2Int(BYTE* Src, int inLen);
DllUseport	void	WINAPI Asc2Byte(void* ascptrc, BYTE *byteptr);
DllUseport	void	WINAPI Asc2Byten(void* ascptrc, BYTE *byteptr, int blen);

DllUseport	BYTE	WINAPI MakeBcc(void* asc, int length);
DllUseport	int		WINAPI MakePack(void* Src, void* Dest, int Slen, char OperMode = '=', int LowNibbleFlg = FALSE);
DllUseport	int		WINAPI MakePack(CString &strSrc, CString &strDest, char OperMode= '=', int LowNibbleFlg = FALSE);
DllUseport	int		WINAPI MakeUnPack(CString &strSrc, CString &strDest, int AscFlg = TRUE);
//DllUseport	int		WINAPI MakeUnPack(void* Src, void* Dest, int Slen, int AscFlg = TRUE);		// [#2001] NH KSK 2010.11.15
//DllUseport	int		WINAPI MakeUnPack(void* Src, CString& Dest, int Slen, int AscFlg = TRUE);	// [#2001] NH KSK 2010.11.15
DllUseport	CString	WINAPI MakeUnPack(void* Src, int Slen, int AscFlg = TRUE);

DllUseport	int		WINAPI IsValidDateTime(int nMonth, int nDay, int nYear, int nHour=0, int nMin=0, int nSec=0);		// [#508] NH AIREAT 2008.03.03
DllUseport	void	WINAPI GetDate(void* gDate);
DllUseport	CString	WINAPI GetDate();
DllUseport	void	WINAPI GetTime(void* gTime);
DllUseport	CString	WINAPI GetTime();
DllUseport	void	WINAPI GetDateTime(void* gDate, void* gTime);
DllUseport	void	WINAPI PutDate(void* pDate);
DllUseport	void	WINAPI PutDate(LPCTSTR pDate);
DllUseport	void	WINAPI PutTime(void* pTime);
DllUseport	void	WINAPI PutTime(LPCTSTR pTime);
DllUseport	void	WINAPI PutDateTime(void* pDate, void* pTime);
DllUseport	void	WINAPI PutDateTime(LPCTSTR pDate, LPCTSTR pTime);

DllUseport	CString WINAPI MakeMoneySpace(int nTotalLength, CString strAmount);
DllUseport	CString WINAPI MakeMoneyCent(CString strAmount, int opt = TRUE);
DllUseport	CString WINAPI MakeMoneyAmount(CString strSrcAmount, int opt = TRUE);	// [RWC6-699] SKKim 2024.04.25 Cent가 없는 Amount Value값 처리

DllUseport	void	WINAPI WideToMulti(LPSTR multi, LPCWSTR wide, int size);
DllUseport	void	WINAPI MultiToWide(LPWSTR wide, LPCSTR multi, int size);

DllUseport	int		WINAPI GetCmnLocalDate(CString strDate, int *pnMonth, int *pnDay, int *pnYear);							// [#508] NH AIREAT 2008.03.03
DllUseport	CString WINAPI GetCmnLocalDate(CString strMonth, CString strDate, CString strYear, TCHAR chDelimiter = '/');	// [#508] NH AIREAT 2008.03.03
DllUseport	CString WINAPI GetCmnLocalDate(int nMonth, int nDay, int nYear, TCHAR chDelimiter = '/');						// [#508] NH AIREAT 2008.03.03
DllUseport  unsigned int WINAPI crc16 (unsigned char *string, int length );													// [#4] NH PSC 2008.03.10
DllUseport  unsigned char WINAPI lrc(unsigned char *string, int length);
DllUseport	CString WINAPI GetCurrencySymbol(int nLength = 0, int nBracket = 0, int nCustomerType = 0);											// [#316] NH 2008.06.19

//////////////////////////////////////////////////////////////////////
//	NVRam.cpp
//////////////////////////////////////////////////////////////////////
DllUseport	int		WINAPI GetMemoryAddress();
DllUseport	void	WINAPI FreeMemoryAddress();

DllUseport	CString	WINAPI GetOldFastCashValue();	// [#268] [NH] KSK 2008.6.9

DllUseport  CString WINAPI MemGetVersion(int hKeyName, LPCTSTR strValueName);
DllUseport	CString WINAPI GetSPVersion(LPCTSTR strValueName);
DllUseport	CString WINAPI GetEPVersion(LPCTSTR strValueName);


//DllUseport	void	WINAPI NVDump(char cStatus, char szSubUnit, char* szFunction, LPCTSTR szErr, LPCTSTR szDummy, int nTraceLevel=0);	// [#2452] NH KSK 2016.11.10 Log관련 함수이므로 위쪽으로 위치 이동 (실제 구현부분은 원래부터 trace.cpp에 구현됨)
DllUseport	int		WINAPI MemSetStr(int hKeyName, int lpszName, LPCTSTR lpszValue);
DllUseport	int		WINAPI MemSetInt(int hKeyName, int lpszName, int nValue);
DllUseport	int		WINAPI MemGetInt(int hKeyName, int lpszName, int nInitValue = 0);
DllUseport	CString	WINAPI MemGetStr(int hKeyName, int lpszName, LPCTSTR InitValue = NULL);

//DllUseport	void	WINAPI AddData(int hKeyName, int lpszName, int nIndex, WORD wValue);
DllUseport	void	WINAPI AddData(int hKeyName, int lpszName, int nIndex, DWORD wValue);	// [#514] [MX] Mexico 지원을 위해 Parameter 변경
DllUseport	void	WINAPI MemSetTotal(int hKeyName, int lpszName, int nIndex, WORD wValue);
DllUseport	DWORD	WINAPI MemGetTotal(int hKeyName, int lpszName, int nIndex);
DllUseport  void	WINAPI MemClearTotal();
DllUseport	void	WINAPI MemClearDualHostDCCTotal();							// [#2185] US Jusitn 2013.05.14
DllUseport  void	WINAPI ClearAllNVRam();
DllUseport  void	WINAPI LoadCustomerNvramValueExt(LPCTSTR filename);
DllUseport  void	WINAPI ClearAPNVRam();										// [#192] KSK 2008.05.15
DllUseport  BOOL	WINAPI CheckNVRam();
DllUseport	int		WINAPI IsValidMod10Data(void* data, int len);				// [#173] [NH] KSK 2008.04.28
DllUseport  bool	WINAPI LoadSettingsFromUSBOrSDCard();						// [#RWC6-72] US William 2019.12.09 PAI INI file

// [#537] KSK 2009.06.22 Base64 Logic 추가
DllUseport	int		WINAPI base64_decode(char* text, int inputlen, unsigned char* dst, int dstlen);
DllUseport	int		WINAPI base64_encode(char* input, int inputlen, char* dst, int dstlen);
// [#419] [NH] KSK 2008.9.11

DllUseport	CString	WINAPI GetJSONNodeValue(CString sRcv, CString sNodeName, BOOL bMultiple=FALSE);		// [#2513] Justin (bMultiple==TRUE : continuing search if found value is empty)

// [#2136] US/CA PCS 2012.05.30 MD5 Logic 추가
DllUseport	CString WINAPI GetMD5(CString strValue);
DllUseport	CString WINAPI GetSurchTableLicense(CString strValue);
// end of [#2136]

// [#2292] US Justin 2014.09.19 Add TDL TLV Logic
DllUseport	int		WINAPI TLV_FillLength(BYTE *pTargetBuffer, int nLength);
DllUseport	int		WINAPI TLV_FillValueString(BYTE *pTargetBuffer, CString strValue);
DllUseport	int		WINAPI TLV_FillValueBuffer(BYTE *pTargetBuffer, BYTE *pBufValue, int nLenValue);

DllUseport	int		WINAPI TLV_GetTagIDandType(BYTE *pTargetBuffer, int*nTagType, int *nBufLoc);
DllUseport	int		WINAPI TLV_GetDataLength(BYTE *pTargetBuffer, int *nBufLoc);
DllUseport	int		WINAPI TLV_GetDataString(BYTE *pTargetBuffer, CString *strData, int nLen);
DllUseport	int		WINAPI Dollar2Cent(LPCTSTR lpvoid);
// End of [#2292]
DllUseport	CString	WINAPI GetCenteredString(CString strIn, int nMaxCol);						// [#2496] US Justin 2017.08.17
DllUseport	CString	WINAPI GetDistributedString(CString strIn1, CString strIn2, int nMaxCol);	// [#2557] US Justin 2018.07.24

// ERROR VERSION
DllUseport	char*	WINAPI GetErrVersion();
//DllUseport	void	WINAPI SetErrVersion();	// 내부적으로만 처리하도록 수정 /*KSK 2008.9.25 */ [#419] [NH] KSK 2008.9.25

// Current Error Stack Position
//DllUseport	int		WINAPI GetCrtErrStack(ERRSTACK* pEditBuff, bool pClearCommand = FALSE);
DllUseport	int		WINAPI GetCrtErrStackCount();
DllUseport	char*	WINAPI GetCrtErrProCount();
DllUseport	char*	WINAPI GetCrtErrCode();
DllUseport	char*	WINAPI GetCrtErrMessage();

DllUseport	void	WINAPI SetCrtErrStack(char* pErrorCode, char* pErrorString);
DllUseport	void	WINAPI ClrCrtErrStack();

// Error Summary Position
// Error Summary용으로만 사용
DllUseport	void	WINAPI SetErrSum(CString strErrCode, int nDevid = 0);
//DllUseport	void	WINAPI SortErrSum();	// 내부적으로만 처리하도록 수정 /*KSK 2008.9.25 */
DllUseport	void	WINAPI ClearErrSum();

DllUseport	int		WINAPI GetErrSumTotalCount();
DllUseport	int		WINAPI GetErrSumStackCount(int nindex);
DllUseport	CString	WINAPI GetErrSumErrCode(int nindex);
DllUseport	CString	WINAPI GetErrSumDate();
DllUseport	CString	WINAPI GetErrSumTime();
// end of [#419]

// [#534] NH AIREAT 2009.06.04 : Reject Total
DllUseport	void	WINAPI	CheckRejectTotalVersion();							// Reject Total version을 Check하여 버전에 맞게 고친다.
DllUseport	void	WINAPI	ClearRejectTotal();									// 누적된 모든 매수를 Clear한다.
DllUseport	void	WINAPI	GetRejTotalStartDate(int *Year, int *Month, int *Day);		// Reject Total을 시작한 날짜
DllUseport	void	WINAPI	GetRejTotalStartTime(int *Hour, int *Minute, int *Sec);		// Reject Total을 시작한 시간

DllUseport	int		WINAPI	GetSumOfDispensed(int nCst);						// DISPENSED 된 누적 매수
DllUseport	int		WINAPI	GetSumOfRejected(int nCst);							// REJECTED  된 누적 매수
DllUseport	int		WINAPI	GetSumOfDetectedSkew(int nCst);						// SKEW   SENSOR에 감지되어 REJECT된 누적 매수
DllUseport	int		WINAPI	GetSumOfDetectedGap(int nCst);						// GAP	  SENSOR에 감지되어 REJECT된 누적 매수
DllUseport	int		WINAPI	GetSumOfDetectedLong(int nCst);						// LONG	  SENSOR에 감지되어 REJECT된 누적 매수
DllUseport	int		WINAPI	GetSumOfDetectedShort(int nCst);					// SHORT  SENSOR에 감지되어 REJECT된 누적 매수
DllUseport	int		WINAPI	GetSumOfDetectedDouble(int nCst);					// DOUBLE SENSOR에 감지되어 REJECT된 누적 매수
DllUseport	int		WINAPI	GetSumOfTestRejected(int nCst);						// VDM에서 TEST DISPENSE시  REJECT된 누적 매수

DllUseport	void	WINAPI	AddSumOfDispensed(int nCst, int nCount);			// DISPENSED 된 매수를 누적 매수에 더한다.
DllUseport	void	WINAPI	AddSumOfRejected(int nCst, int nCount);				// REJECTED  된 매수를 누적 매수에 더한다.
DllUseport	void	WINAPI	AddSumOfDetectedSkew(int nCst, int nCount);			// SKEW	  SENSOR에 감지되어 REJECT된 매수를 누적 매수에 더한다.
DllUseport	void	WINAPI	AddSumOfDetectedGap(int nCst, int nCount);			// GAP	  SENSOR에 감지되어 REJECT된 매수를 누적 매수에 더한다.
DllUseport	void	WINAPI	AddSumOfDetectedLong(int nCst, int nCount);			// LONG	  SENSOR에 감지되어 REJECT된 매수를 누적 매수에 더한다.
DllUseport	void	WINAPI	AddSumOfDetectedShort(int nCst, int nCount);		// SHORT  SENSOR에 감지되어 REJECT된 매수를 누적 매수에 더한다.
DllUseport	void	WINAPI	AddSumOfDetectedDouble(int nCst, int nCount);		// DOUBLE SENSOR에 감지되어 REJECT된 매수를 누적 매수에 더한다.
DllUseport	void	WINAPI	AddSumOfTestRejected(int nCst, int nCount);			// VDM에서 TEST DISPENSE시  REJECT된 매수를 누적 매수에 더한다.
// end of [#534]

DllUseport	void	WINAPI	CheckSystemMemory(CString	DebugLocation);
DllUseport	void	WINAPI	AddPrintData(CString &strPrintData, CString strData);
DllUseport	void	WINAPI	AddPrintData(CString &strPrintData, LPCTSTR lpszFormat, ...);

// TOUCH VIBRATION CONTROL FUNCTION
DllUseport	int		WINAPI	SetTouchVibration(int bTouchControl);

// NHConfig Function Pointer
DllUseport	CNHConfig*		WINAPI	GetConfigFuncPointer();

DllUseport	DWORD	WINAPI	ConvEncryMode2Value(BOOL bEncryMode);	// [#2095] NH KSK 2011.10.27

// [#2012] NH KJW 2010.01.10 zip, unzip을 CmnLib로 공용화
// ZIP
DllUseport	HZIP	WINAPI	zCreateZip(void *buf,unsigned int len, const char *password);
DllUseport	HZIP	WINAPI	zCreateZip(const TCHAR *fn, const char *password);
DllUseport	ZRESULT WINAPI	zZipAdd(HZIP hz,const TCHAR *dstzn, const TCHAR *fn);
DllUseport	ZRESULT WINAPI	zZipAdd(HZIP hz,const TCHAR *dstzn, void *src,unsigned int len);
DllUseport	ZRESULT WINAPI	zZipAddFolder(HZIP hz,const TCHAR *dstzn);
DllUseport	ZRESULT WINAPI	zZipGetMemory(HZIP hz, void **buf, unsigned long *len);
DllUseport	ZRESULT WINAPI	zCloseZip(HZIP hz);

// UNZIP
DllUseport	HZIP	WINAPI	uzOpenZip(const TCHAR *fn, const char *password);
DllUseport	HZIP	WINAPI	uzOpenZip(void *z,unsigned int len, const char *password);
DllUseport	ZRESULT WINAPI	uzGetZipItem(HZIP hz, int index, ZIPENTRY *ze);
DllUseport	ZRESULT WINAPI	uzUnzipItem(HZIP hz, int index, const TCHAR *fn);
DllUseport	ZRESULT WINAPI	uzUnzipItem(HZIP hz, int index, void *z,unsigned int len);
DllUseport	ZRESULT WINAPI	uzCloseZip(HZIP hz);
DllUseport	UINT	WINAPI	uzFormatZipMessage(ZRESULT code, TCHAR *buf,unsigned int len);

DllUseport	UINT32	WINAPI	uzCrc32( UINT32 crc, const BYTE* pBuf, int nLen );

DllUseport	BYTE	WINAPI	lrc8(PBYTE pBuf, int nLen);

// Functions to swap the endian of 16 and 32 bit values
DllUseport	UINT16	WINAPI	SwapEndian(UINT16 val);
DllUseport	UINT32	WINAPI	SwapEndian24(UINT32 val);
DllUseport	UINT32	WINAPI	SwapEndian(UINT32 val);

DllUseport  BOOL	WINAPI IsUniqueATMType(CString strATMType);		// [#2571] NH Justin 2018.08.03 Fix Machine Type Setting Problem - 1024*600 Type ATMs

DllUseport	CString	WINAPI	SHA_256_Encrypt(unsigned char *data, int len, int nFormat /* 0:HexDecimal Str(9d2afa..), 1:BASE64, 2:Binary*/, unsigned char*sEncrypted=NULL);							// [#2448] 2016.11.18 Justin Add SHA-256 Encryption	// [#2524]

// [#2515] US Justin 2017.11.09 Mastercard phase2 development
DllUseport	CString	WINAPI	Encrypt_And_Base64Encode(LPCTSTR sOrgData, unsigned char * key, int nKeyType /*0:Private, 1:Public, 2:Symmetric, 3:Symmetric w/Separate IV */, unsigned char* ivIN=NULL);	// [#2535]
DllUseport	CString	WINAPI	Encrypt_And_Base64Encode_AES_128_ECB(LPCTSTR sOrgData, unsigned char * key, int nKeyType /*0:Private, 1:Public, 2:Symmetric, 3:Symmetric w/Separate IV */, unsigned char* ivIN=NULL);	// [#2535]
DllUseport	int		WINAPI	Base64Decode_And_Decrypt(LPCTSTR sOrgData, unsigned char * key, int nKeyType /*0:Private, 1:Public, 2:Symmetric, 3:Symmetric w/Separate IV */, unsigned char*decrypted, unsigned char* ivIN=NULL);	// [#2535]
DllUseport	int		WINAPI	Base64Decode_And_Decrypt_AES_128_ECB(LPCTSTR sOrgData, unsigned char * key, int nKeyType, unsigned char*decrypted, unsigned char* ivIN);		// [#2535]
DllUseport	CString WINAPI	Cal_Signature_And_Base64Encode(LPCTSTR sOrgData, unsigned char * prvkey, long key_len, int nKeyFormat /*0:PEM, 1:DER*/);
// End of [#2515]

DllUseport	int		WINAPI	GetKeyFromSharedKey(unsigned char *out, char *shared, int sharedLen, unsigned char *salt, int saltLen, int iterations, int length, int padding);		// [#2535] US Justin 2018.03.05 GivePay Online
DllUseport	int		WINAPI	AES_Decrypt_128_ECB(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext);
DllUseport	int		WINAPI	AES_Encrypt_128_ECB(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext);
// end of [#2012]

DllUseport	int		WINAPI	Lz4Compress(CString filePath);
DllUseport	int		WINAPI	Lz4Decompress(CString archivePath, CString dstPath);
DllUseport	int		WINAPI	MakeQRCodeImageFile(CString pText, CString pFilePath);

DllUseport	unsigned short  WINAPI generate_rand(void);	// [RWC6-676] SKKim 2024.04.08

//////////////////////////////////////////////////////////////////////
#endif
