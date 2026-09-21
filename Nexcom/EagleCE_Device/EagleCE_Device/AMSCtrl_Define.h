#ifndef __AMS_RETURN_CODE_H__
#define __AMS_RETURN_CODE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define NETBUF_SEND_SIZE	(1024 * 16)	// FlowCtrl.h에 중복 정의되어 있음.
//#define NETBUF_RECV_SIZE	(1024 * 16)	// FlowCtrl.h에 중복 정의되어 있음.
#define BUFF_SEND_SIZE		(1024 * 4)
#define FILE_SIZE_2K		(1024 * 2)
#define FILE_SIZE_1K		(1024)		// AMS로 File 전송시 1KB 초과하는 경우 Socket Send가 delay되는 현상 수정
#define FILE_SIZE_512		(512)

#define AMS_OK	0
#define AMS_ERR	1

#define AMS_LINE_TCP	0
#define AMS_LINE_DIALUP	1

#define AMS_LISTEN		0	// LISTEN - AMS enable		ATM <- AMS
#define AMS_CONNECT		1	// CONNECT - status Send	ATM -> AMS

#define OPEN_LISTEN		_T("LISTEN")
#define OPEN_CONNECT	_T("CONNECT")

//#define AMS_CODEPAGE	1252		// Latin I Code Page

#define MAX_CBX_COUNT_AMS	4

#define MAX_SETUP_FIELD_COUNT	256

//////////////////////////////////////////
//	AMS MAIN COMMAND DEFINE
//////////////////////////////////////////
#define AMS_REQ_STATUS				_T("A")
	#define AMS_REQ_SUB_STATUS			_T("A")
	#define AMS_REQ_SUB_AUTHENTICATION	_T("B")
	#define AMS_REQ_SUB_INQUIRY_ADD_CMD	_T("C")

#define AMS_REQ_DEVICE_AND_SYSTEM	_T("B")
	#define AMS_REQ_SUB_DEV_RESET		_T("A")
	#define AMS_REQ_SUB_SYS_REBOOT		_T("B")

#define AMS_REQ_PRINT_JNL			_T("C")

#define AMS_REQ_LAST_X_JNL			_T("D")

#define AMS_REQ_SETUP_READ			_T("F")
	#define AMS_SUB_SETUP_READ_ALL		_T("A")

#define AMS_REQ_SETUP_WRITE			_T("G")
	#define AMS_SUB_SETUP_WRITE_ALL		_T("A")

#define AMS_REQ_FILE_UPDATE			_T("H")
	#define AMS_SUB_SW_UPDATE			_T("A")
	#define AMS_SUB_ADV_UPDATE			_T("B")

#define AMS_REQ_REMOTE_CLOSE		_T("I")
	#define AMS_SUB_CBX_CLOSE			_T("B")

#define AMS_REQ_LOG_UPLOAD			_T("M")

#define AMS_REQ_IMG_UPLOAD			_T("N")
	#define AMS_SUB_CAMIMG_UPLOAD		_T("A")

#define AMS_REQ_CHANGE_LOCALTIME	_T("T")

//#define AMS_REQ_PRINT_IMG_UPLOAD	_T("J")

#define LEN_LENGTH	5	// AMSCtrl.h에 중복 정의되어 있음.

#pragma pack(1)

///////////////////////////////////////////////////////////
// AMS Command COMMON
typedef struct _ams_cmd_header_
{
	BYTE byLength[LEN_LENGTH];
	BYTE byCommand[4];
	BYTE byParam[4];
	BYTE byTerminalID[16];
	BYTE byTerminalPW[21];
} AMS_CMD_HEADER, *PAMS_CMD_HEADER;
#define LEN_AMS_CMD_HEADER	(sizeof(AMS_CMD_HEADER) - LEN_LENGTH)

///////////////////////////////////////////////////////////
// AMS Response COMMON
typedef struct _ams_res_header_
{
	BYTE byLength[LEN_LENGTH];
	BYTE byCommand[4];
	BYTE bySubCommand[4];
	BYTE bySerialNum[16];
	BYTE byTerminalID[16];
	BYTE byTerminalPW[21];
} AMS_RES_HEADER, *PAMS_RES_HEADER;
#define LEN_AMS_RES_HEADER	(sizeof(AMS_RES_HEADER) - LEN_LENGTH)

// AMS Response Common Field
typedef struct _ams_res_common_
{
	BYTE byCommonFieldItemCount[5];	// Common Field Count
	BYTE byModeStatus;
	BYTE byErrorCode[7];
	BYTE byRejectItemCount[5];
	BYTE byCstCount;
	BYTE byLoadedBill_1[5];
	BYTE byDispensedBill_1[5];
	BYTE byRemainedBill_1[5];
	BYTE byRejectedBill_1[5];
	BYTE byDenomination_1[4];
	BYTE byCstStatus_1;
	BYTE byLoadedBill_2[5];
	BYTE byDispensedBill_2[5];
	BYTE byRemainedBill_2[5];
	BYTE byRejectedBill_2[5];
	BYTE byDenomination_2[4];
	BYTE byCstStatus_2;
	BYTE byLoadedBill_3[5];
	BYTE byDispensedBill_3[5];
	BYTE byRemainedBill_3[5];
	BYTE byRejectedBill_3[5];
	BYTE byDenomination_3[4];
	BYTE byCstStatus_3;
	BYTE byLoadedBill_4[5];
	BYTE byDispensedBill_4[5];
	BYTE byRemainedBill_4[5];
	BYTE byRejectedBill_4[5];
	BYTE byDenomination_4[4];
	BYTE byCstStatus_4;
	BYTE byAPVersion[16];
	BYTE byAMSAgentVersion[16];
	BYTE byCDR_FW_Version[16];
	BYTE byCDM_FW_Version[16];
	BYTE byPTR_FW_Version[16];
	BYTE byEPP_FW_Version[16];
	BYTE bySIU_FW_Version[16];
	BYTE byMediaStatusCount[5];
	BYTE byPTRMediaStatus;
	BYTE byJournalRecordCount[8];	// AMS로 올린 Journal 개수
	BYTE byJournalTotalCount[8];	// 현재 저장되어 있는 jnl 개수
} AMS_RES_COMMON, *PAMS_RES_COMMON;


///////////////////////////////////////////////////////////
// AMS Status Command
typedef struct _ams_cmd_status_
{
	AMS_CMD_HEADER amsHeader;
} AMS_CMD_STATUS, *PAMS_CMD_STATUS;
// sizeof(AMS_CMD_STATUS) - length.
#define LEN_AMS_CMD_STATUS	(sizeof(AMS_CMD_STATUS) - LEN_LENGTH)

// AMS System Command
typedef struct _ams_cmd_system_
{
	AMS_CMD_HEADER amsHeader;
} AMS_CMD_SYSTEM, *PAMS_CMD_SYSTEM;

// AMS Journal Command
typedef struct _ams_cmd_journal_
{
	AMS_CMD_HEADER amsHeader;		// 35 byte
	BYTE byRequestedCount[7];	// 42 byte
} AMS_CMD_JOURNAL, *PAMS_CMD_JOURNAL;


// AMS Setup Read Command
typedef struct _ams_cmd_setup_read_
{
	AMS_CMD_HEADER amsHeader;
} AMS_CMD_SETUP_READ, *PAMS_CMD_SETUP_READ;

// AMS S/W Upload
typedef struct _ams_cmd_sw_upload_
{
	AMS_CMD_HEADER amsHeader;
	BYTE byFileName[30];
	BYTE byIteration[4];
	BYTE byFileData[FILE_SIZE_2K+1];
} AMS_CMD_SW_UPLOAD, *PAMS_CMD_SW_UPLOAD;

// AMS Balancing Command
typedef struct _ams_cmd_balancing_
{
	AMS_CMD_HEADER amsHeader;		// 35 byte
} AMS_CMD_BALANCING, *PAMS_CMD_BALANCING;


// AMS Log Command
typedef struct _ams_cmd_log_
{
	AMS_CMD_HEADER amsHeader;		// 35 byte
	BYTE byDate[8];
} AMS_CMD_LOG, *PAMS_CMD_LOG;

///////////////////////////////////////////////////////////
// AMS Status Response
typedef struct _ams_res_status_
{
	AMS_RES_HEADER amsHeader;
	AMS_RES_COMMON amsCommon;
} AMS_RES_STATUS, *PAMS_RES_STATUS;

#define LEN_AMS_RES_STATUS	(sizeof(AMS_RES_STATUS) - LEN_LENGTH)

// AMS System Response
typedef struct _ams_res_system_
{
	AMS_RES_HEADER amsHeader;
} AMS_RES_SYSTEM, *PAMS_RES_SYSTEM;

#define LEN_AMS_RES_SYSTEM	(sizeof(AMS_RES_SYSTEM) - LEN_LENGTH)

// AMS Journal Response
typedef struct _ams_res_journal_
{
	AMS_RES_HEADER amsHeader;
	BYTE byCurrentRecordCount[7];
	BYTE byJournalData[BUFF_SEND_SIZE];
} AMS_RES_JOURNAL, *PAMS_RES_JOURNAL;

typedef struct _ams_res_setup_write_
{
	AMS_RES_HEADER amsHeader;
} AMS_RES_SETUP_WRITE, *PAMS_RES_SETUP_WRITE;
// sizeof(AMS_RES_STATUS) - length.
#define LEN_AMS_RES_SETUP_WRITE	(sizeof(AMS_RES_SETUP_WRITE) - LEN_LENGTH)

// AMS Balancing Response
typedef struct _ams_res_balancing_
{
	AMS_RES_HEADER amsHeader;
	BYTE byCloseData[BUFF_SEND_SIZE];
} AMS_RES_BALANCING, *PAMS_RES_BALANCING;

// AMS Log Response
typedef struct _ams_res_log_
{
	AMS_RES_HEADER amsHeader;
	BYTE byFileName[256+1];
	BYTE byIteration[4+1];
	BYTE byFileData[FILE_SIZE_1K+1];
} AMS_RES_LOG, *PAMS_RES_LOG;


#pragma pack()

#endif //__AMS_RETURN_CODE_H__