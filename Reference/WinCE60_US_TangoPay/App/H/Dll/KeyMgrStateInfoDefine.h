#ifndef __KEYMGR_STATE_INFO_DEFINE_H__
#define __KEYMGR_STATE_INFO_DEFINE_H__


//////////////////////////////////////////////////////////////////////////
// Nautilus Hyosung EPP TYPE (Version)
//////////////////////////////////////////////////////////////////////////

enum EPP_TYPE
{
	EPP_NO_VISA = 0,		// 0 : No Visa
	EPP_VISA = 1,			// 1 : Visa 버전
	EPP_1ST_PCI = 2,		// 2 : PCI 1차 인증
	EPP_2ND_PCI = 3,		// 3 : PCI 2차 인증 , RKT 버전 시작, CITI 전용, USB
	EPP_3RD_PCI = 4,		// 4 : PCI 3차 인증 , RKT 버전 시작, SERIAL (SubInfo = 0)
	                        //   : PCI 3차 인증 (V08.20.00 ~ V08.21.00) (SubInfo = 1)
	EPP_NON_PCI = 5,		// 5 : NON PCI (V08.50.00 이상)
	EPP_4TH_PCI = 6,		// 6 : PCI 5차 인증 (V08.21.00 ~ V08.50.00)
	EPP_PCI_20  = 0x50		//PCI 2.0
};

//////////////////////////////////////////////////////////////////////////
// Nautilus Hyosung Key Meode
//////////////////////////////////////////////////////////////////////////

enum NH_KeyMode
{									//       <Injected Key>    <Algorithm>      <MAC>
	KM_INIT = -1,					// -1: Init
	KM00_MASTER_SDES_0MAC = 0,		//  0: Single Master Key,  Single DES,      NO Macing						//OLD : KEYMODE_NON_UNIQ_SDES
	KM01_MASTER_DDES_0MAC = 1,		//	1:   Dual Master Key,    Dual DES,      NO Macing						//OLD : KEYMODE_NON_UNIQ_DDES
	KM02_UNIQUE_SDES_0MAC = 2,		//  2:        Unique Key,         DES,      NO Macing						//OLD : KEYMODE_UNIQ_SDES
	KM03_MASTER_TDES_0MAC = 3,		//  3: Single Master Key,  Triple DES,      NO Macing						//OLD : KEYMODE_NON_UNIQ_TDES
	KM04_UNIQUE_TDES_0MAC = 4,		//  4:        Unique Key,  Triple DES,      NO Macing						//OLD : KEYMODE_UNIQ_TDES
	KM05_MASTER_SDES_SMAC = 5,		//  5: Single Master Key,         DES,  Single Macing						//OLD : KEYMODE_NON_UNIQ_SDES_MACING
	KM06_UNIQUE_SDES_SMAC = 6,		//  6:        Unique Key,         DES,  Single Macing						//OLD : KEYMODE_UNIQ_SDES_MACING
	KM07_MASTER_TDES_SMAC = 7,		//  7:        Master KEy,  Triple DES,  Single Macing						//OLD : KEYMODE_TDES_MACING
//	KM08_XXXXXX_XXXX_XXXX = 8,		//  8: NDC (Single DES)
//  KM09_XXXXXX_XXXX_XXXX = 9,		//  9: NDC (Triple DES)
	KM10_MASTER_TDES_SMAC = 10,		// 10:        Master Key,  Triple DES,  Single Macing						//OLD : NON USE
	KM11_MASTER_TDES_TMAC = 11,		// 11:        Master Key,  Triple DES,  Triple Macing (호주용   T-MAC)		//OLD : KEYMODE_TDES_TMACING
//	KM12_MASTER_TDES_TMAC = 12,		// 12:        Master Key,  Triple DES,  Triple Macing (캐나다영 T-MAC)		//OLD : NON USE
};

//////////////////////////////////////////////////////////////////////////
//	KEY PART AND TYPE
//////////////////////////////////////////////////////////////////////////

enum NH_KEY_PART
{
	KEY_PART_INIT = 0,
	MASTER_SDES_PART_A,				// Single Master Key Part A (16)
	MASTER_SDES_PART_B,				// Single Master Key Part B (16)
	MASTER_TDES_PART_A,				// Triple Master Key Part A (32)
	MASTER_TDES_PART_B,				// Triple Master Key Part B (32)
	UNIQUE_SDES_PART_A,				// Single Unique Key Part A (16)
	UNIQUE_SDES_PART_B,				// Single Unique Key Part B (16)
	UNIQUE_TDES_PART_A,				// Triple Unique Key Part A (32)
	UNIQUE_TDES_PART_B,				// Triple Unique Key Part B (32)
	MACKEY_SMAC_PART_A,				// Single MAC    Key Part A (16)				
	MACKEY_SMAC_PART_B,				// Single MAC	 Key Part B (16)
	MACKEY_TMAC_PART_A,				// Triple MAC	 Key Part A (32)
	MACKEY_TMAC_PART_B				// Triple MAC	 Key Part B (32)
};

//////////////////////////////////////////////////////////////////////////
// Sub Process Return Value
//////////////////////////////////////////////////////////////////////////

enum KeyMgrRet
{
	RET_OK = 0,
	RET_ERROR, 
	RET_CANCEL,
	RET_TIMEOVER,
	RET_EXIT
};

enum NH_KEY_TYPE
{
	TYPE_KEY_INIT = 0,
	TYPE_COMM_MASTERKEY,			// 범용 PCI Single Master Key
	TYPE_COMM_MASTERKEY3,			// 범용 PCI Triple Master Key
	TYPE_PIN_MASTERKEY,				// PCI PIN Single Master Key
	TYPE_PIN_MASTERKEY3,			// PCI PIN Triple Master Key
	TYPE_MAC_MASTERKEY,				// PCI MAC Single Master Key
	TYPE_MAC_MASTERKEY3,			// PCI MAC Triple Master Key
	TYPE_DATA_MASTERKEY,			// PCI DATA Single Master Key
	TYPE_DATA_MASTERKEY3,			// PCI DATA Triple Master Key
	TYPE_PCI20_MASTERKEY,
	TYPE_PCI20_MASTERKEY3,
	TYPE_PCI20_UNIQUEKEY,
	TYPE_PCI20_UNIQUEKEY3,
	TYPE_PCI20_MACKEY,
	TYPE_PCI20_MACKEY3,
};

enum NH_SHA_TYPE
{
	SHA_TYPE_NO_KEY,
	SHA_TYPE_SHA1,
	SHA_TYPE_SHA256
};

#endif //__KEYMGR_STATE_INFO_DEFINE_H__