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
#define		CMD_LOAD RSA ENCIPHERED KEY				0x073F
#define		CMD_SRKL_IMPORT SIGNED KEY				0xA29
#define		CMD_SRKL_EXPORT SIGNED ITEM				0xA27
#define		CMD_SRKL_LOAD SIGNED ITEMS				0xA28
#define		CMD_SET_SIGNERS							0x0A84
#define		CMD_GET_SIGNERS							0x0A83
#define		CMD_AUTHENTICATION_WRAP					0x0A85

//Event IDs
#define		EVENT_ENTRY							0xA14
#define		EVENT_FLOAT_FRAME					0xA86
#define		EVENT_STATUS						0xA1B

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



#pragma pack(pop)



#endif