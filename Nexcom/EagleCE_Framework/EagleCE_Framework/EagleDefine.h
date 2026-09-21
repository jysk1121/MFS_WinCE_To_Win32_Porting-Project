#pragma once

#include <Windows.h>

#define EAGLE_SLEEP_INTERVAL				(100)
#define EAGLE_SLEEP_INTERVAL_200MS			(200)
#define EAGLE_SLEEP_INTERVAL_500MS			(500)

#define EAGLE_SLEEP_INTERVAL_1000MS			(1000)
#define EAGLE_SLEEP_SCREEN_INTERVAL			(2000)
#define EAGLE_SLEEP_INTERVAL_3000MS			(3000)
#define EAGLE_SLEEP_INTERVAL_5000MS			(5000)

#define EAGLE_SLEEP_INTERVAL_10SEC			(10000)
#define EAGLE_SLEEP_INTERVAL_15SEC			(15000)
#define EAGLE_SLEEP_INTERVAL_30SEC			(30000)
#define EAGLE_SLEEP_INTERVAL_1MIN			(6000)

//#define CRYPTERA_EPP_OPEN_DELAY_INTERVAL	(15000)
#define EAGLE_SLEEP_INTERVAL_5MIN			(300000)

#define ADA_TIMEOUT_INFINITE				(86400)

#define CR		(char)0x0D
#define ESC		(char)0x1B		// Multiple Setup Write시 Skip값
#define FS		(char)0x1C
#define GS		(char)0x1D
#define RS		(char)0x1E
#define US		(char)0x1F

#define ZERO_4	_T("0000")
#define ZERO_8	_T("00000000")
#define ZERO_12	_T("000000000000")

#define SPACE_2		_T("  ")
#define SPACE_7		_T("       ")
#define SPACE_8		_T("        ")
#define SPACE_15	_T("               ")

#define S_ENABLE	_T("1")
#define S_DISABLE	_T("0")

#define ENABLE_REVERSAL	1

#define GET_ENDISABLE_INT(NVALUE)	(((NVALUE) == 1) ? (_T("ENABLE")) : (_T("DISABLE")))
#define GET_ENDISABLE_STRING(STR)	((_ttoi(STR) == 1) ? (_T("ENABLE")) : (_T("DISABLE")))
#define GET_USENOTUSE_STRING(STR)	((_ttoi(STR) == 1) ? (_T("USE")) : (_T("NOT USE")))
#define GET_REQUIREDORNOT_STRING(STR)	((_ttoi(STR) == 1) ? (_T("REQUIRED")) : (_T("NOT REQUIRED")))

#define GET_SURCHARGETYPE_STRING(STR)	((_ttoi(STR) == 1) ? (_T("PERCENTAGE")) : (_T("FIXED AMOUNT")))
#define GET_PERCENTAGEMETHOD_STRING(STR)	((_ttoi(STR) == 1) ? (_T("GREATER")) : (_T("LESSER")))

#define GET_EPP_ACTIVATE__STRING(nValue)	(( nValue == 1) ? (_T("REMOVED")) :			\
											(( nValue == 2) ? (_T("PREACTIVATED")) :	\
											(( nValue == 3) ? (_T("ACTIVATED")) :	(_T("NOT DEFINED"))	)))

#define GET_OPTION_STRING(STR)	((_ttoi(STR) == 0) ? (_T("NOT REQUIED")) :			\
								((_ttoi(STR) == 1) ? (_T("REQUIRED")) :	\
								((_ttoi(STR) == 2) ? (_T("OPTIONAL")) :	(_T("NOT DEFINED"))	)))


/** **********************************************************
*	@brief Define EPP Master Key Sequence
************************************************************/
#define		READY_INJECT_KEY_PART_A		0
#define		START_INJECT_PART_A			1
#define		IMPORT_INJECT_PART_A		2
#define		READY_INJECT_KEY_PART_B		3
#define		START_INJECT_PART_B			4
#define		IMPORT_INJECT_PART_B		5
#define		RESULT_FINAL_KEY_INFO		6



/** **********************************************************
*	@brief Define error device type
************************************************************/
#define ERROR_DEVICE_IDC		0x00000001
#define ERROR_DEVICE_CDM		0x00000010
#define ERROR_DEVICE_PTR		0x00000100
#define ERROR_DEVICE_EPP		0x00001000
#define ERROR_DEVICE_CAM		0x00010000
#define ERROR_DEVICE_NET		0x00100000
#define ERROR_DEVICE_SIU		0x01000000
#define ERROR_DEVICE_MUB		0x10000000


/** **********************************************************
*	@brief	Define AP ErrorCode List
************************************************************/
#define		ERROR_TERMINALID					_T("Z00001")
#define		ERROR_HOST_IP						_T("Z00002")
#define		ERROR_TERMINAL_IP					_T("Z00003")
#define		ERROR_PHONE_NUMBER					_T("Z00004")
#define		ERROR_MASTER_PW_DEFAULT				_T("Z00005")
#define		ERROR_ADMIN_PW_DEFAULT				_T("Z00006")
#define		ERROR_REPLENISHMENT_PW_DEFAULT		_T("Z00007")


// CST Error
#define		ERROR_CST1_SETTING					_T("Z10001")
#define		ERROR_CST2_SETTING					_T("Z10002")
#define		ERROR_CST3_SETTING					_T("Z10003")
#define		ERROR_CST4_SETTING					_T("Z10004")
#define		ERROR_ALL_CST_SETTING				_T("Z10005")

#define		ERROR_CST1_EMPTY					_T("Z10006")
#define		ERROR_CST2_EMPTY					_T("Z10007")
#define		ERROR_CST3_EMPTY					_T("Z10008")
#define		ERROR_CST4_EMPTY					_T("Z10009")
#define		ERROR_ALL_CST_EMPTY					_T("Z10010")

#define		ERROR_CST1_COUNT_EMPTY				_T("Z10011")
#define		ERROR_CST2_COUNT_EMPTY				_T("Z10012")
#define		ERROR_CST3_COUNT_EMPTY				_T("Z10013")
#define		ERROR_CST4_COUNT_EMPTY				_T("Z10014")
#define		ERROR_ALL_CST_COUNT_EMPTY			_T("Z10015")

#define		ERROR_INVALID_DENOMINATION			_T("Z10020")


// Sensor Error
#define		ERROR_FRONT_DOOR_OPEN		_T("Z20001")
#define		ERROR_SAFE_DOOR_OPEN		_T("Z20002")


// Network Error
#define		ERROR_NET_CONNECT_FAIL	_T("Z30001")
#define		ERROR_NET_ENQ_FAIL		_T("Z30002")
#define		ERROR_NET_ACK_FAIL		_T("Z30003")
#define		ERROR_NET_DATA_FAIL		_T("Z30004")
#define		ERROR_NET_EOT_FAIL		_T("Z30005")
#define		ERROR_CRC_MISMATCH		_T("Z30006")


// Host Response Message Error
#define		ERROR_INVALID_TERMIANID	_T("Z31001")
#define		ERROR_INVALID_TRANCODE	_T("Z31002")
#define		ERROR_INVALID_SEQNO		_T("Z31003")
#define		ERROR_INVALID_AUTHNO	_T("Z31004")
#define		ERROR_INVALID_TRANDATE	_T("Z31005")
#define		ERROR_INVALID_TRANDTIME	_T("Z31006")
#define		ERROR_INVALID_BUSSDATE	_T("Z31007")
#define		ERROR_INVALID_RESPCODE	_T("Z31008")

// Z3HXXX - Host Declined Error Code
#define		ERROR_HOST_DECLINED_CODE _T("Z3H")

#define		ERROR_REVERSAL_FAILED		_T("Z32001")
#define		POWEROFF_REVERSAL			_T("Z32002")
#define		POWEROFF_DISPENSING			_T("Z32003")
#define		POWEROFF_DISPENSING_COMP	_T("Z32004")

// EMV IC Error
#define		ERROR_EMV_MISSING_DATA	_T("Z33001")
#define		ERROR_EMV_IC_DECLINED	_T("Z33002")
#define		ERROR_EMV_IC_TIMEOUT	_T("Z33003")


// Journal Error
#define		ERROR_JOURNAL_FULL		_T("Z40001")

// PINPAD STATE ERROR
#define		ERROR_PINPAD_STATE		_T("Z50001")
#define		ERROR_MASTERKEY_EMPTY	_T("Z50002")
#define		ERROR_PINKEY_EMPTY		_T("Z50003")

#define		ERROR_STUCK_EPP			_T("Z50004")

// Device Port Error
#define		ERROR_CDM_PORTOPEN		_T("ZF0001")
#define		ERROR_PTR_PORTOPEN		_T("ZF0002")
#define		ERROR_IDC_PORTOPEN		_T("ZF0003")
#define		ERROR_EPP_PORTOPEN		_T("ZF0004")
#define		ERROR_SIU_PORTOPEN		_T("ZF0005")
#define		ERROR_CAM_OPENFAIL		_T("ZF0006")
#define		ERROR_EJL_OPENFAIL		_T("ZF0007")
#define		ERROR_MUB_OPENFAIL		_T("ZF0008")

// Unknown Error Code
#define		ERROR_UNKNOWN_ERRORCODE	_T("ZFFFFF")

/** **********************************************************
*	@brief		언어 종류
************************************************************/
typedef enum
{
	EAGLE_LANGUAGE_ENGLISH = 1,
	EAGLE_LANGUAGE_SPANISH,
	EAGLE_LANGUAGE_FRENCH,
	EAGLE_LANGUAGE_CHINESES,
	EAGLE_LANGUAGE_KOREAN,
	EAGLE_LANGUAGE_JAPANESES,
} EAGLE_LANGUAGE_CODE;


/** **********************************************************
*	@brief		거래 종류
************************************************************/
typedef enum
{
	EAGLE_TRAN_CODE_DOWNLOAD_CONFIG,
	EAGLE_TRAN_CODE_WITHDRAWAL,
	EAGLE_TRAN_CODE_TRANSFER,
	EAGLE_TRAN_CODE_BALANCE,
	EAGLE_TRAN_CODE_REVERSAL,
	EAGLE_TRAN_CODE_HOST_TOTALS,
	EAGLE_TRAN_CODE_GET_HOST_TOTALS,

	EAGLE_TRAN_CODE_CBX_TOTALS,
	EAGLE_TRAN_CODE_TRIAL_CBX_TOTALS,

} EAGLE_TRAN_CODE;


/** **********************************************************
*	@brief		커맨드 종류
************************************************************/
typedef enum
{
	EAGLE_CMD_CL_CHANGE_CUSTOM,
	EAGLE_CMD_CL_INSERT_CARD,

	EAGLE_CMD_CL_EMV_CARD_READ,
	EAGLE_CMD_CL_EMV_MAKEAPPLISTS,
	EAGLE_CMD_CL_EMV_FINALSELECT,
	EAGLE_CMD_CL_EMV_MULTIAPPSELECT,
	EAGLE_CMD_CL_EMV_FALLBACK,

	EAGLE_CMD_CL_REMOVE_CARD,
	EAGLE_CMD_CL_READ_CARD,
	EAGLE_CMD_CL_SELECT_LANGUAGE,
	EAGLE_CMD_CL_READ_PIN,
	EAGLE_CMD_CL_SELECT_TRANSACTION,
	EAGLE_CMD_CL_SELECT_ACCOUNT,
	EAGLE_CMD_CL_SELECT_TRANSFERACCOUNT,
	EAGLE_CMD_CL_SELECT_AMOUNT,
	EAGLE_CMD_CL_ADA_CONFIRMAMOUNT,
	EAGLE_CMD_CL_CONFIRM_SURCHARGE,
	EAGLE_CMD_CL_ENTER_WITHDRAWALAMOUNT,
	EAGLE_CMD_CL_ENTER_TRANSFERAMOUNT,
	EAGLE_CMD_CL_SELECT_RECEIPT,
	EAGLE_CMD_CL_CONFIRM_RECEIPTERROR,
	EAGLE_CMD_CL_CONFIRM_RECEIPTSCREEN,
	EAGLE_CMD_CL_HOST_PROCESSING,
	EAGLE_CMD_CL_EMV_REMOVECARD,
	EAGLE_CMD_CL_CASH_DISPENSING,
	EAGLE_CMD_CL_CASH_DISPENSING_COMPLETE,
	EAGLE_CMD_CL_CASH_DISPENSING_ERROR,
	EAGLE_CMD_CL_REVERSAL,
	EAGLE_CMD_CL_PRINT_RECEIPT,
	EAGLE_CMD_CL_SCREEN_RECEIPT,


	EAGLE_CMD_AMS_ACCEPT_OK,
	EAGLE_CMD_AMS_STATUS_SEND,



	EAGLE_CMD_OP_INITIALIZE,
	EAGLE_CMD_OP_CDM_RESET,
	EAGLE_CMD_OP_CDM_TEST,
	EAGLE_CMD_OP_CDM_LEARNBILL,
	EAGLE_CMD_OP_CDR_RESET,
	EAGLE_CMD_OP_CDR_READCARD,
	EAGLE_CMD_OP_DIO_LAMPON,
	EAGLE_CMD_OP_DIO_LAMPOFF,
	EAGLE_CMD_OP_DIO_LAMPFLICK,
	EAGLE_CMD_OP_PRT_RESET,
	EAGLE_CMD_OP_PRT_TESTPRINT,
	EAGLE_CMD_OP_SND_TESTSOUND,
	EAGLE_CMD_OP_JNL_VIEW_PRINT,
	EAGLE_CMD_OP_JNL_PRINTLASTX,
	EAGLE_CMD_OP_JNL_PRINT_PRINT,
	EAGLE_CMD_OP_JNL_BACKUP,
	EAGLE_CMD_OP_JNL_DELETE,
	EAGLE_CMD_OP_RPT_PRINT,
} EAGLE_COMMAND_TYPE;


/** **********************************************************
*	@brief		상태 종류
************************************************************/
typedef enum
{
	EAGLE_STATUS_NONE,
	EAGLE_STATUS_SUCCEEDED,
	EAGLE_STATUS_FAILED,
	EAGLE_STATUS_RUNNING,
} EAGLE_STATUS_TYPE;


/** **********************************************************
*	@brief		HOST 관련 정의
************************************************************/
#define		NET_SEND_BUFF_SIZE	(1024 * 4)		/**< Send Max Buffer Size 4KB */
#define		NET_RECV_BUFF_SIZE	(1024 * 4)		/**< Receive Max Buffer Size 4KB */

#define REVERSAL_REASON_INCORRECT_DISPENSE		_T("01")
#define REVERSAL_REASON_PROTOCOL_ERROR			_T("02")
#define REVERSAL_REASON_LOST_EOT_ERROR			_T("03")
#define REVERSAL_REASON_MAC_ERROR				_T("04")
#define REVERSAL_REASON_CUSTOMER_CANCEL			_T("05")
#define REVERSAL_REASON_ICC_DECLINED			_T("07")
#define REVERSAL_REASON_ICC_CARD_NOT_REMOVED	_T("11")


/** **********************************************************
*	@brief		고객 정보
************************************************************/
struct EAGLE_CLIENT_INFO
{
	// 클리어
	void			Clear(void);

	// 거래 시작
	BOOL			m_bIsTransaction;

	// Card Data
	CString			m_strAccountNo;			// PinBlock 생성을 위한 계좌번호

	// Pin Number
	CString			m_strPinBlock;			// 실제 Pin Block은 ASC 16자리임 (Hex값으로는 8BYTE)

	// 에러
	BOOL			m_bIsSetError;

	int				m_nErrorDevice;

	// 에러 코드
	//DWORD			m_dwError;
	CString			m_strErrorCode;
	CString			m_strErrorMsg;

	// Reversal Error
	BOOL			m_bIsSetReversalError;

	// 영수증 출력
	BOOL			m_bIsPrintReceipt;
	
	// BIN Result
	BOOL			m_bIsBlockBin;

	int				m_nSoundVolume;

	int				m_nProcessCount;

	BOOL			m_bPowerOffFlag;

	BOOL			m_bEMV_CardExist;

	BOOL			m_bEMV_Transaction;

	BOOL			m_bExec2ndGAC;

	CString			m_strTag57Data;		// for masking
	CString			m_strTag5AData;		// for masking

	BOOL			m_bEMVDeclined;		// Host 송신 전 IC Error시 Message 표시용

	BOOL			m_bEMV_SoundNotice;

	BOOL			m_bRecvConfigInfo;
};



