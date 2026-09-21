#ifndef __NH_CONFIG_H__
#define __NH_CONFIG_H__

#ifdef	_ATMCE_US
#include "..\h\Common\BaseDef_US.h"
#elif defined	_ATMCE_CA		// 국가코드 캐나다
#include "..\h\Common\BaseDef_CA.h"
#elif defined  _ATMCE_AU
#include "..\h\Common\BaseDef_AU.h"
#elif defined  _ATMCE_MX	// [#2115] MX KSK 2012.01.11
#include "..\h\Common\BaseDef_MX.h"
#endif

#pragma warning(disable:4251)	// [#2032] NH KJW 2011.03.15 AFX_CLASS_EXPORT CNHConfig클래스의 멤버가 템플릿일 경우 나는 warning이며, private으로 내부만 사용시에는 문제없어 disable함.
/*-------------------------------------------------
         NH OS Version Define
---------------------------------------------------*/

typedef enum NH_OS_VERSION
{
	NH_OS_UNKNOWN				= 0,
	NH_OS_DOVE					= 1,	// -- TESTING OS
	NH_OS_NH2700_B				= 2,	// NH2700(iTM) Basic
	NH_OS_NH2700_L				= 3,	// NH2700(iTM) Luxury
	NH_OS_NH2700_T_T			= 4,	// NH2700(iTM) Touch TTW	[#2064] NH KSK 2011.05.18
	NH_OS_NH1800SE_B			= 5,	// NH1800SE			// [#11] NH KSK 2010.09.14
	NH_OS_NH2700_F_T			= 6,	// NH2700(iTM) Function TTW	[#2064] NH KSK 2011.05.18
	NH_OS_NH1500SE				= 7,	// [#2267] US KSK 2014.05.08 Naming 확정
	NH_OS_NH1800_POS			= 8,	// [#2267] US KSK 2014.05.08 Naming 미정
	NH_OS_NH2100T_POS			= 9,	// [#2267] US KSK 2014.05.08 Naming 미정
	NH_OS_MX5200SE				= 10,	// [#2300] US KSK 2014.11.03 MX3000 기종 추가		// [#2315] US Justin 2014.12.11 Change Machine Name.
	NH_OS_MX2800SE				= 11,	// [#2518] US Kook 2017.12.21 Support MX-2800SE
	NH_OS_MX2800_F_T			= 12	// [#GLDV-2505] US Kook 2019.05.13 Support MX-2800T
};

typedef enum NH_CE_VERSION
{
	WINCE_6,
	WINCE_7,
	MAX
};

typedef enum BASEDIR_TYPE {
	BASEDIR_ROOT,
	BASEDIR_USB,
	BASEDIR_SDCARD,
	BASEDIR_MAX
};

/*-------------------------------------------------
         NH SCREEN Information Define
---------------------------------------------------*/
typedef enum NH_SCR_TYPE
{
	SCR_FRONT	=	0,		// Front Screen
	SCR_REAR	=	1,		// Rear Screen
	SCR_KEYMGR	=	2,		// KeyMgr Screen
	SCR_UPDATE	=	3,		// Update Screen
	SCR_LOCATION_MAX
};

typedef enum NH_SCR_ENGINE
{
	SCR_ENG_NONE		= 0,
	SCR_ENG_FLASH_LITE	= 1,		// Adobe Flash Lite 30
	SCR_ENG_PICASSO		= 2,		// Nautilus Hyosung Screen Drawing Engine
	SCR_ENG_MAX
};

typedef struct
{
	NH_SCR_ENGINE	eEngine;
	BOOL			bCreateEngine;
	CString			strLoadPath;
	CString			strLoadSection;
	int				nX;
	int				nY;
	int				nWidth;
	int				nHeight;
	CString			strUSBUpdateSymbol;
	CString			strSDUpdateSymbol;
} NH_SCR_CONFIG;

class AFX_CLASS_EXPORT CNHConfig
{
public:
	CNHConfig();
	~CNHConfig();

public:
	NH_OS_VERSION	GetOSVersion();
	CString			GetMachineType();
	CString			GetBootLoaderVersion();
	CString			GetTouchPanelVersion();
	NH_SCR_CONFIG	GetScreenConfig(NH_SCR_TYPE eScreenType);
	NH_CE_VERSION	GetCEVersion();

	CString			GetUpdatePath(BASEDIR_TYPE eBaseDir);			// [#GLDV-2853] AU Kook 2021.03.10 WEC7

	/**
	 * Returns true when the Camera upkit is supported
	 */
	bool			SupportsCamera();

private:
	BOOL			GetRegValue(HKEY hKey, LPCTSTR strKeyName, LPCTSTR strValueName, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
	void			SetScreenConfig();


private:
	NH_OS_VERSION	m_eOSVersion;
	NH_CE_VERSION	m_eCEVersion;
	NH_SCR_CONFIG	m_eScreenConfig[SCR_LOCATION_MAX];
	CString			m_strTouchPanelVersion;
	CString			m_strBootLoaderVersion;
	DWORD			m_usbMode;
};

#endif __NH_CONFIG_H__