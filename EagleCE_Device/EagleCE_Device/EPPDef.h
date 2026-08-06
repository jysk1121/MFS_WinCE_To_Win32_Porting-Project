#ifndef _EPP_DEF_H_
#define	_EPP_DEF_H_

#pragma once

//Commands
#define		CMD_READ_SW_VERSION						0x016 
#define		CMD_READ_DEVICE_ID						0x017
#define		CMD_WRITE_REGISTER						0x0A07
#define		CMD_READ_REGISTER						0xA06
#define		CMD_CREATE_REGISTER						0x1A8
#define		CMD_REMOVE_REGISTER						0x1A6
#define		CMD_REMOVE_REGISTER_AUTHENTICATED		0x1AC
#define		CMD_RESIZE_REGISTER						0x1A7
#define		CMD_GET_REGISTER_INFO					0x0A03
#define		CMD_EPP_REBOOT							0xA05
#define		CMD_TEST_INTERNAL_FUNCTIONS				0x0A04 
#define		CMD_START_ENTRY							0x0A00
#define		CMD_START_ENTRY_AUTHENTICATED			0x0A33
#define		CMD_CANCEL_ENTRY						0x0A01
#define		CMD_STORE_ENTRY							0x0A02
#define		CMD_STOP_ENTRY							0x0A3F
#define		CMD_READ_PIN_EXT						0x0A0D
#define		CMD_STORE_FRAME							0x0A81
#define		CMD_READ_FRAME							0x0A82
#define		CMD_KEY_CHECK_VALUE						0x0A0F
#define		CMD_VALIDATE_MAC						0x0A18
#define		CMD_GET_RANDOM_BLOCK					0xA19
#define		CMD_CRYPT_DATA_EXTENDED					0xA23
#define		CMD_CALCULATE_MAC_EXTENDED				0xA24
#define		CMD_LOAD_TR31							0x789
#define		CMD_EMV_IMPORT_KEY						0x0510
#define		CMD_LOCAL_PIN_VERIFICATION				0x04B
#define		CMD_DEVICE_GETLOG						0x793
#define		CMD_DEVICE_CLEAR_LOG					0x794
#define		CMD_GET_DEVICE_STATUS					0xA13
#define		CMD_TEST_KEYBOARD						0x27
#define		CMD_DECOMMISSION						0x1A2
#define		CMD_GET_PCI_REPORT						0xA1A
#define		CMD_INIT_COUNTERS						0x07F
#define		CMD_READ_COUNTERS						0x7E
#define		CMD_CHECK_STORAGE						0x1A4
#define		CMD_GET_CFSHEADER						0x1A9
#define		CMD_DEVICE_INITIALIZE					0x01ab
#define		CMD_DEVICE_ACTIVATE						0x790
#define		CMD_DEVICE_PREACTIVATE					0x791
#define		CMD_GENERATE_RSA_KEYPAIR_CERT_REQUEST	0x0A2A
#define		CMD_GENERATE_NONCE						0x0A44
#define		CMD_DETACH_HOST_EXTENDED				0x749
#define		CMD_LOAD_CERTIFICATE_EXTENDED			0x748
#define		CMD_GET_CERTIFICATE						0x0743
#define		CMD_LOAD_RSA_ENCIPHERED_KEY				0x073F
#define		CMD_SRKL_IMPORT_SIGNED_KEY				0xA29
#define		CMD_SRKL_EXPORT_SIGNED_ITEM				0xA27
#define		CMD_SRKL_LOAD_SIGNED_ITEMS				0xA28
#define		CMD_SET_SIGNERS							0x0A84
#define		CMD_GET_SIGNERS							0x0A83
#define		CMD_AUTHENTICATION_WRAP					0x0A85

//Event IDs
#define		EVENT_ENTRY							0xA14
#define		EVENT_FLOAT_FRAME					0xA86
#define		EVENT_STATUS						0xA1B

/**< notification command definication */
#define		PIN_ENTRY				0x30
#define		PWD_ENTRY				0x31
#define		NPWD_VERIFY				0x32/**< 알림 command */
#define		NORMAL_KEY				0x33/**< Normal key pressed */
#define		SECUKEY_ENTRY			0x34/*Master Key/Session Key를 지원하기 위해 추가 2015.10.27*/


/**< notification parameter definition */
#define		NOTICE_MESSAGE			0x30
#define		EFFECTKEY_PRESS			0x31
#define		KEYIN_COMPLETE			0x32
#define		NEXTIN_CONTINUE			0x33
#define		EXCEPT_MESSAGE			0x34/**< 예외 상황 발생 : key input일때 KEYIN_COMPLETE와 같은 효과
									다만 예외 상황이므로 예외 상황 CODE값을 정의하고 참조 */
/**< noticefication message code */
#define		KEYENTRY_START			0x30
#define		KEYENTRY_LACK			0x31/**< key입력부족(under 4) */
#define		KEYENTRY_OVER			0x32/**< key입력포화(over 12) */

/**< exception code definition */
#define		KEYENTRY_TIMEOUT		0x00
#define		PWDVERIFY_FAIL			0x01
#define		OVERALL_TIMEOUT1		0x02/**< PIN entry overall timeout(30 sec) */
#define		OVERALL_TIMEOUT2		0x03/**< PIN encryption overall timeout(3600 sec) */
//----------------------------------------------------------------------------------

//CRC
#define		CRC_POLYNOMIAL						0x8005  /* CRC polynomial */

#pragma pack(push, 1)
typedef struct _StartEntry
{
	BYTE	L1[2];
	BYTE	byMode;
	BYTE	L2[2];
	BYTE	MinKeys[2];
	BYTE	L3[2];
	BYTE	MaxKeys[2];
	BYTE	L4[2];
	BYTE	TerminateKeys[4];
	BYTE	L5[2];
	BYTE	TerminateFDKs[4];
	BYTE	L6[2];
	BYTE	ActivateKeys[4];
	BYTE	L7[2];
	BYTE	ActivateFDKs[4];
	BYTE	L8[2];
	BYTE	byautoEnd;
}STRATENTRY, *LPSTARTENTRY;

typedef struct _KeyBlockHeader
{
	BYTE byVersion;
	BYTE bySize[2];
	BYTE byUsage[4];
	BYTE byType;
	BYTE byAccess;
	BYTE byWrMethod[2];
	BYTE byOption;
}KEYBLOCKHEADER, *LPKEYBLOCKHEADER;

typedef struct _OptionalKeyBlockTMK
{
	BYTE byID[2];
	BYTE bySize[2];
	BYTE byPayload[256];
}OPTIONALKEYBLOCKTMK, *LPOPTIONALKEYBLOCKTMK;

typedef struct _CreateRegParam
{
	unsigned short	usRegNameLength;
	char			*szRegName;
	unsigned short	usParentRegLength;
	char			*szParentName;
	unsigned short	usRegSizeLength;
	unsigned short	usRegSize;
	unsigned short	usKeyBlockLength;
	BYTE			*byKeyBlock;
}CREATEREGPARAM, *LPCREATEREGPARAM;

typedef struct _WriteRegParam
{
	unsigned short	usRegNameLength;
	char			*szRegName;
	unsigned short	usParentRegLength;
	char			*szParentName;
	unsigned short	usRegSizeLength;
	unsigned short	usRegSize;
	unsigned short	usKBHLength;
	KEYBLOCKHEADER	*pKBH;
	unsigned short	usPayloadLength;
	BYTE			*byPayload;
	unsigned short	usControlLength;
	BYTE			*byControlValue;
	unsigned short	usCreateOptionLength;
	BYTE			byCreateOption;
	unsigned short	usCryptoMethodLength;
	BYTE			byCryptoMethod;
	unsigned short	usKCVModeLength;
	unsigned short	usKCVMode;
	unsigned short	usKCVLength;
	BYTE			*byKCV;
}WRITEREGPARAM, *LPWRITEREGPARAM;

typedef struct _ReadRegParam
{
	unsigned short	usRegNameLength;
	char			*szRegName;
}READREGPARAM, *LPREADREGPARAM;

typedef struct _GetRegInfoParam
{
	unsigned short	usRegNameLength;
	char			*szRegName;
	unsigned short	usFilterLength;
	BYTE			*byFileter;
}GETREGINFOPARAM, *LPGETREGINFOPARAM;

typedef struct _PINBlockParam
{
	unsigned short	usFormatLength;
	unsigned short	usPINFormat;
	unsigned short	usCustomDataLength;
	char			*szCustomData;
	unsigned short	usPaddingCharLength;
	BYTE			byPaddingChar;
	unsigned short	usRegNameLength;
	char			*szRegName;
	unsigned short	usXORDataLength;
	BYTE			*byXORData;
	unsigned short	us2ndRegNameLength;
	char			*sz2ndRegName;
}PINBLOCKPARAM, *LPPINBLOCKPARAM;

#pragma pack(pop)

#endif