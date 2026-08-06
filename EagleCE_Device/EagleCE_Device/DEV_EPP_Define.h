//
// EPP 메시지 정의
//
#define	WM_EPP_NOTI_AUTH				(WM_USER + 500)
#define	WM_EPP_NOTI_PASSWORD			(WM_USER + 501)
//#define	WM_EPP_NOTI_NOMALKEY			(WM_USER + 502)
#define	WM_EPP_NOTI_GETPIN				(WM_USER + 503)

// ERROR ADDED GARURA
typedef enum {
	_ERR_SUCCESS = 0x00,
	_ERR_COMMUNICATION, 		// Communication Disconnected
	_ERR_NO_USBDRIVER,			// Can find USB Driver
	_ERR_NO_USBCABLE,			// Not Connect USB Cable
	_ERR_CREATE_HANDLE, 		// Creation of Handle Failure
	_ERR_CREATE_THREAD, 		// Creation of Thread Failure
	_ERR_BAD_PARAM, 			// Bad Parameter
	_ERR_NEED_EXTERNALAUTH, 	// External Authentication Required
	_ERR_NEED_PWDCHANGE, 		// Default Passwords Change Required
	_ERR_NEED_PEK, 				// Key Loading Required
	_ERR_NO_ADMINID, 			// Admin ID Required
	_ERR_NEED_PRESSEDSWITCH, 	// Pressing Removal Switch Required
	_ERR_UNKNOWN_RC, 			// Unknown Error Code
	_ERR_UNKNOWN_COMMAND, 		// Unknown Command
	_ERR_RCV_TIMEOUT, 			// Receive-Timeout Failure
	_ERR_ACK_FAILURE,			// ACK Check Failure
	_ERR_NAK_RECEIVE,			// NAC Received
	_ERR_UNKNOWN_RCV,			// Unknown Data Received
	_ERR_FAILURE, 				// Normal Failure
	_ERR_VERIFY_HOST_CERT,		// 
	_ERR_VERIFY_BOUND_HOST,
	_ERR_NO_INIT_KEY,
	_ERR_NOT_BOUNDED_HOST,
	_ERR_FAILED_HOST_AUTH,
	_ERR_VERIFY_NONCE,
	_ERR_VERIFY_KCV,
	_ERR_BOUNDED_HOST
} _E_ERR_CODE;

typedef enum {
	_NO_RCV_NOTI = 0x000,
	_PIN_ENTRY = 0x100,
	_PWDCHANGE_ENTRY = 0x200, // FOR PASSWORD CHANGE COMMAND
	_PWDVERIFY_ENTRY = 0x300,	// FOR PWD VERIFY ON EXTERNAL AUTHENTICATION
	_NORMAL_KEY_PRESS_EVENT = 0x400,
	_INVALID_NOTIFICATION = 0x500,
	_STRING_MSG = 0x010,
	_KEY_PRESS_EVENT = 0x020,
	_EXIT_ENTRY = 0x030,
	_CONTINUE_NEXT_ENTRY = 0x040,
	_EXCEPTION_EVENT = 0x050,
	_NEED_EXTERNAL_AUTH = 0x060,
} _E_NOTI_MSG_ID;

typedef enum {
	_ENTRY_START = 0x01,
	_ENTRY_LOW_LIMIT = 0x02,
	_ENTRY_UPPER_LIMIT = 0x03,
	_USER_CANCEL_EVENT = 0x04,
	_ENTRY_COMPLETE_EVENT = 0x05,
	_ENTRY_TIMEOUT_EVENT = 0x06,
	_PWD_VERIFY_FAIL_EVENT = 0x07,
	_INVALID_PWD_EVENT = 0x08,
	_UNKNOWN_DATA = 0x09,
} _E_NOTI_DATA_ID;

/*----------------------------------------------------------------------------------------------------*/
#define _PIN_ENTRY_START					_PIN_ENTRY|_STRING_MSG|_ENTRY_START
#define _PIN_ENTRY_LOWLIMIT					_PIN_ENTRY|_STRING_MSG|_ENTRY_LOW_LIMIT
#define _PIN_ENTRY_UPLIMIT					_PIN_ENTRY|_STRING_MSG|_ENTRY_UPPER_LIMIT
#define _PIN_ENTRY_UNKNOWN					_PIN_ENTRY|_STRING_MSG|_UNKNOWN_DATA
#define _PIN_ENTRY_KEYPRESS					_PIN_ENTRY|_KEY_PRESS_EVENT
#define _PIN_ENTRY_TIMEOUT					_PIN_ENTRY|_EXIT_ENTRY|_ENTRY_TIMEOUT_EVENT
#define _PIN_ENTRY_COMPLETE					_PIN_ENTRY|_EXIT_ENTRY|_ENTRY_COMPLETE_EVENT
#define _PIN_ENTRY_USERCANCEL				_PIN_ENTRY|_EXIT_ENTRY|_USER_CANCEL_EVENT
#define _PIN_ENTRY_EXITUNKNOWN				_PIN_ENTRY|_EXIT_ENTRY|_UNKNOWN_DATA
#define _PIN_ENTRY_EXCEPT_TIMEOUT			_PIN_ENTRY|_EXCEPTION_EVENT|_ENTRY_TIMEOUT_EVENT
#define _PIN_ENTRY_EXCEPT_UNKNOWN			_PIN_ENTRY|_EXCEPTION_EVENT|_UNKNOWN_DATA
/*----------------------------------------------------------------------------------------------------*/
#define _PWDCHANGE_ENTRY_START				_PWDCHANGE_ENTRY|_STRING_MSG|_ENTRY_START
#define _PWDCHANGE_ENTRY_LOWLIMIT			_PWDCHANGE_ENTRY|_STRING_MSG|_ENTRY_LOW_LIMIT
#define _PWDCHANGE_ENTRY_UPLIMIT			_PWDCHANGE_ENTRY|_STRING_MSG|_ENTRY_UPPER_LIMIT
#define _PWDCHANGE_ENTRY_UNKNOWN			_PWDCHANGE_ENTRY|_STRING_MSG|_UNKNOWN_DATA
#define _PWDCHANGE_ENTRY_KEYPRESS			_PWDCHANGE_ENTRY|_KEY_PRESS_EVENT
#define _PWDCHANGE_ENTRY_TIMEOUT			_PWDCHANGE_ENTRY|_EXIT_ENTRY|_ENTRY_TIMEOUT_EVENT
#define _PWDCHANGE_ENTRY_COMPLETE			_PWDCHANGE_ENTRY|_EXIT_ENTRY|_ENTRY_COMPLETE_EVENT
#define _PWDCHANGE_ENTRY_USERCANCEL			_PWDCHANGE_ENTRY|_EXIT_ENTRY|_USER_CANCEL_EVENT
#define _PWDCHANGE_ENTRY_EXITUNKNOWN		_PWDCHANGE_ENTRY|_EXIT_ENTRY|_UNKNOWN_DATA
#define _PWDCHANGE_ENTRY_CONTINUENEXT		_PWDCHANGE_ENTRY|_CONTINUE_NEXT_ENTRY
#define _PWDCHANGE_ENTRY_EXCEPT_VRFYFAIL	_PWDCHANGE_ENTRY|_EXCEPTION_EVENT|_PWD_VERIFY_FAIL_EVENT 
#define _PWDCHANGE_ENTRY_EXCEPT_INVAILDPWD	_PWDCHANGE_ENTRY|_EXCEPTION_EVENT|_INVALID_PWD_EVENT 
#define _PWDCHANGE_ENTRY_EXCEPT_TIMEOUT		_PWDCHANGE_ENTRY|_EXCEPTION_EVENT|_ENTRY_TIMEOUT_EVENT 
#define _PWDCHANGE_ENTRY_EXCEPT_UNKNOWN		_PWDCHANGE_ENTRY|_EXCEPTION_EVENT|_UNKNOWN_DATA
#define _PWDCHANGE_ENTRY_NEED_EXTERNAL_AUTH NULL
/*----------------------------------------------------------------------------------------------------*/
#define _PWDVERIFY_ENTRY_START				_PWDVERIFY_ENTRY|_STRING_MSG|_ENTRY_START
#define _PWDVERIFY_ENTRY_LOWLIMIT			_PWDVERIFY_ENTRY|_STRING_MSG|_ENTRY_LOW_LIMIT
#define _PWDVERIFY_ENTRY_UPLIMIT			_PWDVERIFY_ENTRY|_STRING_MSG|_ENTRY_UPPER_LIMIT
#define _PWDVERIFY_ENTRY_UNKNOWN			_PWDVERIFY_ENTRY|_STRING_MSG|_UNKNOWN_DATA
#define _PWDVERIFY_ENTRY_KEYPRESS			_PWDVERIFY_ENTRY|_KEY_PRESS_EVENT
#define _PWDVERIFY_ENTRY_TIMEOUT			_PWDVERIFY_ENTRY|_EXIT_ENTRY|_ENTRY_TIMEOUT_EVENT
#define _PWDVERIFY_ENTRY_COMPLETE			_PWDVERIFY_ENTRY|_EXIT_ENTRY|_ENTRY_COMPLETE_EVENT
#define _PWDVERIFY_ENTRY_USERCANCEL			_PWDVERIFY_ENTRY|_EXIT_ENTRY|_USER_CANCEL_EVENT
#define _PWDVERIFY_ENTRY_EXITUNKNOWN		_PWDVERIFY_ENTRY|_EXIT_ENTRY|_UNKNOWN_DATA
#define _PWDVERIFY_ENTRY_CONTINUENEXT		_PWDVERIFY_ENTRY|_CONTINUE_NEXT_ENTRY
#define _PWDVERIFY_ENTRY_EXCEPT_VRFYFAIL	_PWDVERIFY_ENTRY|_EXCEPTION_EVENT|_PWD_VERIFY_FAIL_EVENT 
#define _PWDVERIFY_ENTRY_EXCEPT_INVAILDPWD	_PWDVERIFY_ENTRY|_EXCEPTION_EVENT|_INVALID_PWD_EVENT 
#define _PWDVERIFY_ENTRY_EXCEPT_TIMEOUT		_PWDVERIFY_ENTRY|_EXCEPTION_EVENT|_ENTRY_TIMEOUT_EVENT 
#define _PWDVERIFY_ENTRY_EXCEPT_UNKNOWN		_PWDVERIFY_ENTRY|_EXCEPTION_EVENT|_UNKNOWN_DATA
/*----------------------------------------------------------------------------------------------------*/
#define _NORMAL_KEY_PRESS					_NORMAL_KEY_PRESS_EVENT

/************************************************************************/
/* Commands                                                             */
/************************************************************************/
#define DEV_RESET				0x30	// Device Reset Command
#define GET_VERSION				0x31	// Get Version Command 
#define GET_EPPSTATUS			0x32	// Get EPP Status Command
#define BUZZER_CONTROL			0x3a	// BUZZER Control Command 
#define REQ_PINENTRY			0x40	// PIN Entry Command
#define INPUT_CANCEL			0x42	// PIN Entry Cancel Command
#define EPP_SELFTEST			0x47	// EPP's self-test Command
#define CHG_PASSWORD			0xf1	// Change Password Command
#define VERIFY_PWD				0xf3	// External Authentication Command
#define REMOVAL_SWITCH			0xf6	// Removal Switch Control Command
#define SET_LOCAL_TIME			0xf7	// Set RTC local time Command
/************************************************************************/
/* Parameters                                                           */
/************************************************************************/
/*
*	Parameters for Removal Switch Control Command
*/
#define REMOVAL_SWON			0x30	// Activate Removal Switch
#define REMOVAL_SWOFF			0x31	// Deactivate Removal Switch
#define REQ_ARLOG_LEAST			0x32	// Get the Latest Log
#define REQ_ARLOG_ALL			0x33	// Get All Logs
/*
*	Parameters for Buzzer Control Command
*/
#define BUZZER_ACTIVE			0x31	// Activate Buzzer Sound
#define BUZZER_INACTIVE			0x30	// Deactivate Buzzer Sound
/*
*	Parameters for Get Version Command
*/
#define BL_VERSION				0x30	// 2BL Version
#define FW_VERSION				0X31	// Firmware Version

#define MFSCOMMEPPDLL_API __declspec(dllexport)


typedef struct _epp_result {
	DWORD error_cd;
	CHAR sResult[100];
}EPPRESULT, *LPEPPRESULT;

typedef struct _epp_status {
	DWORD	error_cd;
	BOOL	b_bpk;
	BOOL	b_fwdownload;
	BOOL	b_pwchange;
	BOOL	b_initkey;
	BOOL	b_pekload;
	BOOL	b_rswitch;
	BOOL	b_rswitch_active;
	BOOL	b_hostbound;
}EPPSTATUS, *LPEPPSTATUS;


typedef struct _epp_get_pin {
	BYTE	hPan[16];
	BYTE	hPEK1[16];						//PEK
	BYTE	hPEK2[16];
	UINT	nPinFormat;
}EPPGETPIN, *LPEPPGETPIN;

typedef struct _epp_get_pin_result{
	BYTE hEPin[8];						//Enciphered PIN Block
	BYTE hDPin[8];						//Decrypt Enciphered PIN Block	
}EPPGETPINRESULT, *LPEPPGETPINRESULT;



/* hError exchange pek.
0x30;	// normal
0x31;	// read bounded Host's cert failed
0x32;	// TIME OUT
0x33; 	// Broken communication sync
0x34;	// Host's cert verify failure
0x35;	// BIND_SignH verfy failure
0x36;	// NO INIT KEY
0x37;	// NOT BOUNDED HOST
0x38; 	// HOST Authentication fail.
0x39;	// KCV check failure
0x3a;	// Nonce Verify failure from epp
0x3b;	// verify failure the EPP's cert
0x3c;	// Nonce Verify failure from host
0x3d;	// get host private key error
0x3e;	// kcv mismaching
0x3f;	//invalid key value;
*/
typedef struct _epp_exchange_pek{
	BYTE	hPEK1[16];					//PEK 1
	BYTE	hPEK2[16];					//PEK 2
	CHAR	sBHostCertPath[260];
	CHAR	sBHostPrvPath[260];
	CHAR	sCACertPath[260];
	CHAR	hKCV[5];					//KCV 
	BYTE	hError;						//error code;
}EPPEXCHANGEPEK, *LPEXCHANGEPEK;


/*	hError rsak key exchange..

0x30; 	// normal
0x31;	// Broken communication sync
0x32;	// time out
0x33; 	// verify epp cert fail
0x34;	// verify signature fail
0x35;	// verify nonce fail
0x36;	// key component check fail	//1
0x37;	// ca' public key read fail	//2
0x38;	// epp's public key read fail	//3
0x39;	// epp's private key read fail	//4
0x3a;	// epp's private key verify fail //5
0x3b;	// ca's private key read fail //6

*/

typedef struct _epp_exchange_rsakey{
	CHAR	sCACertPath[260];			//for verify epp 
	CHAR	sCAPrvPath[260];
	CHAR	sEPPCertPath[260];
	CHAR	sEPPPrvPath[260];
	BYTE	Key1[32];					// key component 1 
	BYTE	Key2[32];					// Key component 2
	BYTE	hError;						//error code;
}EPPEXCHANGERSAKEY, *LPEXCHANGERSAKEY;


/* herror host bind
0x30;	// normal
0x31;	// host's cert read fail
0x32;	// sign faild with bounded RKLSK_host
0x33;	// time out
0x34;	// need external authentication
0x35;	// need change default password
0x36;	// already bounded host
0x37;	// BIND_SignH verfy fail
0x38;	// Unknown Error

*/

typedef struct _epp_host_bind{
	CHAR	sBHostCertPath[260];
	CHAR	sBHostPrvPath[260];
	BYTE	hError;						//error code;
}EPPHOSTBIND, *LPEPPHOSTBIND;

typedef struct _epp_replace_bindhost{
	CHAR	sBHostCertPath[260];
	CHAR	sBHostPrvPath[260];
	CHAR	sNHostCertPath[260];
	CHAR	sNHostPrvPath[260];
	BYTE	hError;						//error code;
}EPPREPLACEBINDHOST, *LPEPPREPLACEBINDHOST;


MFSCOMMEPPDLL_API BOOL __stdcall MFSCommEPP_OpenPort(HWND hwnd, int nPortNum);
MFSCOMMEPPDLL_API void __stdcall MFSCommEPP_ClosePort();
MFSCOMMEPPDLL_API BOOL __stdcall MFSCommEPP_Reset();
MFSCOMMEPPDLL_API BOOL __stdcall MFSCommEPP_GetVersion(BYTE TYPE, LPEPPRESULT lpResult);
MFSCOMMEPPDLL_API BOOL __stdcall MFSCommEPP_GetStatus(LPEPPSTATUS lpStatus);
MFSCOMMEPPDLL_API BOOL __stdcall MFSCommEPP_SelfTest();
MFSCOMMEPPDLL_API BOOL __stdcall MFSCommEPP_ExternalAuth();
MFSCOMMEPPDLL_API BOOL __stdcall MFSCommEPP_ChangePassword();
MFSCOMMEPPDLL_API BOOL __stdcall MFSCOMMEPP_NORMALKEYEVENT();
MFSCOMMEPPDLL_API BOOL __stdcall MFSCOMMEPP_NORMALKEYEVENTSTOP();
MFSCOMMEPPDLL_API BOOL __stdcall MFSCOMMEPP_SETLOCALTIME();
MFSCOMMEPPDLL_API BOOL __stdcall MFSCOMMEPP_GETPIN(LPEPPGETPIN lpeppgetpin);
//MFSCOMMEPPDLL_API BOOL __stdcall MFSCOMMEPP_GETPINCOMPLETE();
MFSCOMMEPPDLL_API BOOL __stdcall MFSCOMMEPP_GETPINCANCEL();
MFSCOMMEPPDLL_API BOOL __stdcall MFSCOMMEPP_PEKEXCHANGE(LPEXCHANGEPEK lpexchangepek);
MFSCOMMEPPDLL_API BOOL __stdcall MFSCOMMEPP_HOSTUNBIND();
MFSCOMMEPPDLL_API BOOL __stdcall MFSCOMMEPP_HOSTBIND(LPEPPHOSTBIND lpepphostbind);
//MFSCOMMEPPDLL_API BOOL __stdcall MFSCOMMEPP_REPLACEHOSTBIND(LPEPPREPLACEBINDHOST lpreplacebindhost);
MFSCOMMEPPDLL_API BOOL __stdcall MFSCOMMEPP_BUZZER(BOOL bOn);
MFSCOMMEPPDLL_API BOOL __stdcall MFSCOMMEPP_RSAKeyREPLACE(LPEXCHANGERSAKEY lpexchangersakey);