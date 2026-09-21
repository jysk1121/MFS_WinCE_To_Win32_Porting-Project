#ifndef __NH_KEYMGR_STATE_INFO_H__
#define __NH_KEYMGR_STATE_INFO_H__

#include "KeyMgrStateInfoDefine.h"

// ----------------------------------------------------------------------------
//	State Information 저장하는 Class
// ----------------------------------------------------------------------------

class AFX_CLASS_EXPORT CKeyMgrStateInfo
{
// Constructor
public:
	~CKeyMgrStateInfo();
//protected:
	CKeyMgrStateInfo();

// Attributes
public:
	// Information
	EPP_TYPE		m_eEppType;
	int				m_nSubInfo;
	NH_KeyMode		m_eKeyMode;
	int				m_nKeyIndex;
	BYTE			m_byPinFormat;
	BYTE			m_byPinPadData;
	BYTE			m_byPartNumber;
	BYTE			m_nKeyCheck[8];

	int				m_nTMACOption;
	BOOL			m_bIsNDC;

	// Injection
	NH_KEY_PART		m_eKeyPart;
	int				m_nInjectKeyIndex;

	CString			m_StrKeyMgrEndReason;

	NH_KEY_TYPE		m_eKeyType;
	CString			m_strKeyName;
	BOOL			m_bShowBtn;

	int				m_nMultiKeyIndex;		// 멀티키용 인덱스

	//PCI 5.0 에서 추가.
	CString			m_strNextState;
	CString			m_strPreviousState;

	BOOL			m_bClearEPP;

	BOOL			IsPCI50()
	{
		return (0x08 == m_nSubInfo && EPP_PCI_20 <= m_eEppType);
	}

	BOOL			IsSupportTR31();
	BOOL			m_bTR31;

	CString			m_strEPVer;

	DWORD			m_dwRemovalCurrentState;     //현재 RemovalState 상태.
	BOOL			m_bRemovalSupport;			 //Removal state 지원 여부.

	// Operations
public:
	void Clear();
};


#endif //__NH_KEYMGR_STATE_INFO_H__