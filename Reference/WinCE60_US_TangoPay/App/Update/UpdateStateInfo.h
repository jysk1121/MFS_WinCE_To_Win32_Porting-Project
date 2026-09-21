#ifndef __NH_UPDATE_STATE_INFO_H__
#define __NH_UPDATE_STATE_INFO_H__


enum UpdateSource
{
	FROM_NONE = -1,			// Update Source : None
	FROM_USB = 0,			// Update Source : From USB
	FROM_RMS = 1,			// Update Source : From RMS (may be : RMS)
	FROM_LOADER = 2,		// Update Source : From LOADER
	FROM_UPDATE2 = 3,		// Update Source : From Update2		// [#2378] US Justin 2015.11.12 Dual Step Software Update
	FROM_REMOTE = 4,
	FROM_MAX
};

//enum OSVersion
//{
//	OS_5050,				// [WINCE_50] OS Version : V01.01.06 이전 버전.				   (USB:ATM2)
//	OS_5000,				// [WINCE_50] OS Version : V01.01.06 포함 V01.01.99 이전 버전. (USB:USB)
//	OS_1800_2ND,			// [WINCE_60] OS Version : V03.01.00 포함 V05.XX.XX 이전 버전. (USB:USB), (SD:)
//	OS_ITM,					// [WINCE_60] OS Version : V05.XX.XX						   (USB:USB), (SD:)
//};

enum UpdateType
{
	UT_NONE = -1,			// Update Type : None
	UT_MASTER_ZIP = 0,		// Update Type : Master Zip
	UT_MASTER_FILE,			// Update Type : Master File
	UT_PATCH_ZIP,			// Update Type : Patch Zip
	UT_PATCH_FILE,			// Update Type : Patch File
	UT_MAX					// Update Type : Max

};

// ----------------------------------------------------------------------------
//	State Information 저장하는 Class
// ----------------------------------------------------------------------------

class CUpdateStateInfo
{
// Constructor
public:
	~CUpdateStateInfo();
//protected:
	CUpdateStateInfo();

// Attributes
public:
	BOOL			m_bUpdateSuccessful;			// Update 결과.

	// UP_Init
	UpdateSource	m_eUpdateSource;				// Update 호출 Source.
//	OSVersion		m_eOSVersion;					// OS Version.
	
	// UP_FromUSB, UP_FromRMS, UP_FromRMS
	UpdateType		m_eUpdateType;					// Update Type.

	CString			m_strUSBUpdateSrc;				// USB UPDATE FOLDER PATH
	CString			m_strRMSUpdateSrc;				// RMS UPDATE FOLDER PATH
	CString			m_strSDUpdateSrc;				// SD CARD ROOT PATH

	CString			m_strUpdateRoot;				// UPDATE ROOT PATH
	CString			m_strUpdateTitle;

// Operations
public:
	void Clear();
	void GetGlobalVariable();
};

#endif