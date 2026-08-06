#pragma once


#define MAX_CASSETTE	6


#pragma pack(1)


typedef struct _cdm_cfg_status 
{
	BYTE major_no;
	BYTE minor_no;
	BYTE cbx_type;
} CDMCFGSTATUS, *LPCDMCFGSTATUS;

typedef struct _cdm_status
{
	BYTE error_cd;
	BYTE reject_cd;
	BYTE sensor[8];
} CDMSTATUS, *LPCDMSTATUS;

typedef struct _cdm_cbx_item
{
	BYTE count[MAX_CASSETTE];
} CDMCBXITEM, *LPCDMCBXITEM;

typedef struct _cdm_multi_dispense
{
	BYTE error_cd;
	BYTE reject_cd;
	BYTE sensor[8];
	BYTE count[MAX_CASSETTE];
} CDMMULTIDISPENSE, *LPCDMMULTIDISPENSE;

typedef struct _cdm_last_dispense 
{
	BYTE last_cmd;
	BYTE last_error_cd;
	BYTE last_cbx;

	BYTE last_dispense_count[MAX_CASSETTE];
	BYTE last_divert_count[MAX_CASSETTE];
	BYTE last_pick_count[MAX_CASSETTE];
} CDMLASTDISPENSE, *LPCDMLASTDISPENSE;

typedef struct _cdm_diagnostic 
{
	BYTE error_cd;
	BYTE reject_cd;
	BYTE sensor[8];
	CHAR result_msg[200];
} CDMDIAGNOSTIC, *LPCDMDIAGNOSTIC;

typedef struct _cdm_set_bill_thickness 
{
	BYTE cbx_no;
	BYTE thickness;
	BYTE margin;
} CDMSETBILLTHICKNESS, *LPCDMSETBILLTHICKNESS;

typedef struct _cdm_get_bill_thickness 
{
	BYTE error_cd;
	BYTE thickness;
	BYTE margin;
} CDMGETBILLTHICKNESS, *LPCDMGETBILLTHICKNESS;

typedef struct _cdm_set_bill_size 
{
	BYTE cbx_no;
	BYTE size;
	BYTE margin;
} CDMSETBILLSIZE, *LPCDMSETBILLSIZE;

typedef struct _cdm_get_bill_size 
{
	BYTE error_cd;
	BYTE size;
	BYTE margin;
} CDMGETBILLSIZE, *LPCDMGETBILLSIZE;

typedef struct _cdm_learn 
{
	BYTE error_cd;
	BYTE reject_cd;
	BYTE size_avg;
	BYTE thickness_avg;
} CDMLEARN, *LPCDMLEARN;

// V01.00.00.03: Add Error description
typedef struct
{
	BYTE error_cd;
	TCHAR error_nm[128];
} CDMERRORINFO;

/** ********************************************
*	@struct	_cdm_count_info
*	@date	2017.11.13
*	@author MFS 
*	@brief	Cassette 매수 정보
***********************************************/
typedef struct _cdm_count_info
{
	BYTE byReqCount[MAX_CASSETTE];		///< 요청매수
	BYTE byPickupCount[MAX_CASSETTE];	///< Pickup 매수
	BYTE byRejectedCount[MAX_CASSETTE];	///< Reject 매수
}CDM_COUNT_INFO, *LPCDM_COUNT_INFO;

/** ********************************************
*	@struct	_cdm_bill_info
*	@date	2017.11.15
*	@author MFS 
*	@brief	지폐의 size와 thickness 정보
***********************************************/
typedef struct _cdm_bill_info
{
	BYTE bySize[MAX_CASSETTE];
	BYTE bySizeMargin[MAX_CASSETTE];
	BYTE byThickness[MAX_CASSETTE];
	BYTE byThicknessMargin[MAX_CASSETTE];
}CDM_BILL_INFO, *LPCDM_BILL_INFO;

/** ********************************************
*	@struct	_cdm_reject_info
*	@date	2017.12.19
*	@author MFS 
*	@brief	Reject 건수 및 사유 Read
***********************************************/
typedef struct _cdm_reject_info
{
	BYTE byLogLen;
	BYTE byRejectInfo[246];
}CDM_REJECT_INFO, *LPCDM_REJECT_INFO;

/** ********************************************
*	@struct	_cdm_recv_data
*	@date	2018.03.13
*	@author MFS 
*	@brief	대용량 recv data 구조체
***********************************************/
typedef struct _cdm_recv_data
{
	_cdm_recv_data() {
		hData = NULL;
		dwSize = 0;
		lpbBody = NULL;
	}
	~_cdm_recv_data() {
		if (hData) {
			GlobalUnlock(hData);
			GlobalFree(hData);
		}
	}

	DWORD	dwSize;
	LPBYTE	lpbBody;

	HGLOBAL hData;
}CDMRECVDATA, *LPCDMRECVDATA;



#pragma pack()
