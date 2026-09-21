#pragma once

#define	EJ_FULL			0x01
#define	EJ_ALARM		0x02

// Define Journal Error Code
#define	ssm_jnl_normal							0
#define ssm_jnl_err_db1_open					1
#define ssm_jnl_err_db1_read					2
#define ssm_jnl_err_db1_insert					3
#define ssm_jnl_err_db1_srch					4
#define ssm_jnl_err_db1_close					5
#define	ssm_jnl_err_full						6

// Error Code DB이므로 일단 error code list에서 추가하지 않음
#define ssm_jnl_err_db3_open					7
#define	ssm_jnl_err_db3_get_error				8
#define ssm_jnl_err_db3_close					9


//
typedef enum {
	transaction		= 0x01,
	add_bill		= 0x02,
	cst_close		= 0x03,
	day_close		= 0x04,
	denom_info		= 0x05,
	value_change	= 0x06,
	text_field		= 0x07,
	withdrawal	    = 0x10,
	balance			= 0x11,
	transfer		= 0x12,
	power_on		= 0x20,
	in_service		= 0x21,
	out_of_service	= 0x22,
	in_supervisor	= 0x23,
	reboot			= 0x24,
	test_dispense	= 0x25,
	unknown			= 0xff
} FLSEJLCODE;

typedef enum {
	JNL_NO = 1,
	TRAN_SEQ_NO = 2,
	DATE_RANGE = 3
} SRCHTYPE;

#define dim(x)	(sizeof(x) / sizeof(x[0]))

#pragma pack(1)

typedef struct jnl_title {
	USHORT code;
	CHAR Msg[100];
} JNLTTL;


typedef struct _jnl_comm {
	DWORD	jnl_no;
	CHAR	log_date[16];
	CHAR	log_time[16];
	DWORD	jnl_cd;
	CHAR	log_srch_date[16];
} FLSEJCOMMINFO, *LPFLSEJCOMMINFO;

//HOST 와 Data Sequence Match	
typedef struct _jnl_tran {
	DWORD	jnl_no;
	CHAR	term_id[16];
	CHAR	tran_seq_no[8];
	CHAR	tran_cd[16];
	CHAR	card_tr2[64];
	CHAR	auth_no[16];
	CHAR	tran_date[16];
	CHAR	tran_time[16];
	CHAR	business_date[16];
	CHAR	account_type[32];
	CHAR	req_amt[16];
	CHAR	disp_amt[16];
	CHAR	surc_amt[16];
	CHAR	bal_amt[32];
	CHAR	avail_bal_amt[32];
	CHAR	disp_cnt[32];
	CHAR	reject_cnt[32];
	CHAR	err_code_1[8];		
	CHAR	err_code_2[8];		
	CHAR	err_code_3[8];
	CHAR	err_msg[2048];

	CHAR	is_print[2];
	CHAR	is_ams_upload[2];
	CHAR	is_host_upload[2];

	CHAR    cst1_reject_info[128];
	CHAR    cst2_reject_info[128];
	CHAR    cst3_reject_info[128];
	CHAR    cst4_reject_info[128];
	CHAR    cst_remain_cnt[100];

	CHAR	emv_data[1024];				// EMV Data 영역 추가
	CHAR	dcc_data[1024];				// DCC Data 영역 추가

	CHAR	cam_img_filename[256];		// Camera File Name 영역 추가

} FLSEJTRANINFO, *LPFLSEJTRANINFO;

//added by 2nd - 추가할 것 - not use 
typedef struct _jnl_err_info {
	CHAR	err_code_1[8];
	CHAR	err_code_2[8];
	CHAR	err_code_3[8];
	CHAR	err_msg[100];
} FLSEJERRINFO, *LPFLSEJERRINFO;

typedef struct _jnl_add_bill {
	DWORD	jnl_no;
	CHAR	term_id[16];
	CHAR	start_date[16];
	CHAR	start_time[16];
	CHAR	add_bill_cst1[8];
	CHAR	remain_cst1[8];
	CHAR	add_bill_cst2[8];
	CHAR	remain_cst2[8];
	CHAR	add_bill_cst3[8];
	CHAR	remain_cst3[8];
	CHAR	add_bill_cst4[8];
	CHAR	remain_cst4[8];
	CHAR	is_print[2];
	CHAR	is_ams_upload[2];
	CHAR	is_host_upload[2];

	CHAR	cst_count[2];		// Added
} FLSEJADDBILLINFO, *LPFLSEJADDBILLINFO;

typedef struct _jnl_cst_close {
	DWORD	jnl_no;
	CHAR	term_id[16];
	CHAR	start_date[16];
	CHAR	start_time[16];

	CHAR	cst_close_type[2];

	CHAR	load_bill_cst1[8];
	CHAR	disp_bill_cst1[8];
	CHAR	rej_bill_cst1[8];
	CHAR	rem_bill_cst1[8];
	CHAR	load_amt_cst1[16];
	CHAR	disp_amt_cst1[16];
	CHAR	rem_amt_cst1[16];
	CHAR	denom_cst1[8];

	CHAR	load_bill_cst2[8];
	CHAR	disp_bill_cst2[8];
	CHAR	rej_bill_cst2[8];
	CHAR	rem_bill_cst2[8];
	CHAR	load_amt_cst2[16];
	CHAR	disp_amt_cst2[16];
	CHAR	rem_amt_cst2[16];
	CHAR	denom_cst2[8];

	CHAR	load_bill_cst3[8];
	CHAR	disp_bill_cst3[8];
	CHAR	rej_bill_cst3[8];
	CHAR	rem_bill_cst3[8];
	CHAR	load_amt_cst3[16];
	CHAR	disp_amt_cst3[16];
	CHAR	rem_amt_cst3[16];
	CHAR	denom_cst3[8];

	CHAR	load_bill_cst4[8];
	CHAR	disp_bill_cst4[8];
	CHAR	rej_bill_cst4[8];
	CHAR	rem_bill_cst4[8];
	CHAR	load_amt_cst4[16];
	CHAR	disp_amt_cst4[16];
	CHAR	rem_amt_cst4[16];
	CHAR	denom_cst4[8];

	CHAR	is_print[2];
	CHAR	is_ams_upload[2];
	CHAR	is_host_upload[2];

	CHAR	cst_count[2];		// Added
} FLSEJCSTCLINFO, *LPFLSEJCSTCLINFO;

typedef struct _jnl_day_close {
	DWORD	jnl_no;
	CHAR	term_id[16];
	CHAR	start_date[16];
	CHAR	start_time[16];

	CHAR	day_close_type[2];
	CHAR	day_close_Result[2];

	CHAR	host_with_cnt[8];
	CHAR	host_bal_cnt[8];
	CHAR	host_trans_cnt[8];
	CHAR	total_host_amt[16];
	CHAR	term_with_cnt[8];
	CHAR	term_bal_cnt[8];
	CHAR	term_trans_cnt[8];
	CHAR	total_term_amt[16];

	CHAR	is_print[2];
	CHAR	is_ams_upload[2];
	CHAR	is_host_upload[2];
} FLSEJDAYCLINFO, *LPFLSEJDAYCLINFO;

// struncture 누락분 추가
typedef struct _jnl_set_denomination {
	DWORD	jnl_no;
	CHAR	term_id[16];
	CHAR	start_date[16];
	CHAR	start_time[16];

	CHAR	olddenom_cst1[8];
	CHAR	olddenom_cst2[8];
	CHAR	olddenom_cst3[8];
	CHAR	olddenom_cst4[8];
	CHAR	newdenom_cst1[8];
	CHAR	newdenom_cst2[8];
	CHAR	newdenom_cst3[8];
	CHAR	newdenom_cst4[8];

	CHAR	is_print[2];
	CHAR	is_ams_upload[2];
	CHAR	is_host_upload[2];

	CHAR	cst_count[2];		// Added
} FLSEJSETDENOMINFO, *LPFLSEJSETDENOMINFO;

// [V1.0.2.10] 2021.03.02 LEH - TestDispense 추가
typedef struct _jnl_test_dispense {
	DWORD	jnl_no;
	CHAR	term_id[16];
	CHAR	start_date[16];
	CHAR	start_time[16];

	CHAR	test_disp_result[2];	// 1:success, 0:fail

	CHAR	test_req_bill_cst[4][8];
	CHAR	test_pickup_bill_cst[4][8];
	CHAR	test_rej_bill_cst[4][8];
	CHAR	rem_bill_cst[4][8];
	CHAR	denom_cst[4][8];

	CHAR	is_print[2];
	CHAR	is_ams_upload[2];
	CHAR	is_host_upload[2];

	CHAR	cst_count[2];		// Added
	CHAR	err_code[8];
} FLSEJTESTDISPENSE, *LPFLSEJTESTDISPENSE;

// - not use
typedef struct _jnl_key_chg {
	CHAR	old_chk_digit[8];
	CHAR	new_chk_digit[8];
} FLSEJKEYINFO, *LPFLSEJKEYINFO;

// - not use
typedef struct _jnl_pwd {
	CHAR	old_pwd[8];
	CHAR	new_pwd[8];
} FLSEJPWDINFO, *LPFLSEJPWDINFO;

typedef struct _jnl_change {
	DWORD	jnl_no;
	CHAR	term_id[16];
	CHAR	old_value[256];
	CHAR	new_value[256];

	CHAR	is_print[2];
	CHAR	is_ams_upload[2];
	CHAR	is_host_upload[2];
} EJCHANGEINFO, *LPEJCHANGEINFO;

typedef struct _jnl_text {
	DWORD	jnl_no;
	CHAR	term_id[16];
	CHAR	text_value[1024];

	CHAR	is_print[2];
	CHAR	is_ams_upload[2];
	CHAR	is_host_upload[2];
} EJTEXTINFO, *LPEJTEXTINFO;

typedef struct _jnl_version {
	BYTE	major;
	BYTE	minor;
} FLSEJVERSION, *LPFLSEJVERSION;


/*---------------------------------------------------
Define Update Field ID
----------------------------------------------------*/
#define	PRINTED_JNL_CNT			0x00000001
#define CURRENT_JNL_CNT			0x00000010
#define	UPLOADED_AMS_JNL_CNT	0x00000100
#define	UPLOADED_HOST_JNL_CNT	0x00001000


//added at 13/10/2004 for statistical data
typedef struct _data {
	DWORD	nJnlCnt;				//for management of journal no
	DWORD	nPrintedJnlIndex;		//for management of printed journal's index
	DWORD	nUploadedAMSIndex;		//for management of uploaded AMS journal's index 
	DWORD	nUploadedHOSTIndex;		//for management of uploaded HOST journal's index (Reserved)
} FLSDATAINFO, *LPFLSDATAINFO;

// - not use
typedef struct _stat_cst_close {
	DWORD	tot_disp_cnt_cst_1;
	DWORD	tot_rej_cnt_cst_1;
	DWORD	tot_rem_cnt_cst_1;
	DWORD	tot_disp_amt_cst_1;
	DWORD	tot_rem_amt_cst_1;
	DWORD	tot_disp_cnt_cst_2;
	DWORD	tot_rej_cnt_cst_2;
	DWORD	tot_rem_cnt_cst_2;
	DWORD	tot_disp_amt_cst_2;
	DWORD	tot_rem_amt_cst_2;
	DWORD	tot_disp_cnt_cst_3;
	DWORD	tot_rej_cnt_cst_3;
	DWORD	tot_rem_cnt_cst_3;
	DWORD	tot_disp_amt_cst_3;
	DWORD	tot_rem_amt_cst_3;
	DWORD	tot_disp_cnt_cst_4;
	DWORD	tot_rej_cnt_cst_4;
	DWORD	tot_rem_cnt_cst_4;
	DWORD	tot_disp_amt_cst_4;
	DWORD	tot_rem_amt_cst_4;
	CHAR	st_date_time[20];		// DD/MM/YYYY HH:MM:SS 
} FLSEJSTATCSTCLINFO, *LPFLSEJSTATCSTCLINFO;

//통계 - not use
typedef struct _stat_day_close {
	DWORD	tot_tran_with_cnt;
	DWORD	tot_bal_cnt;
	DWORD	tot_trans_cnt;
	DWORD	tot_with_amt;
	CHAR	st_date_time[20];		// DD/MM/YYYY HH:MM:SS 
} FLSEJSTATDAYCLINFO, *LPFLSEJSTATDAYCLINFO;

// - not use
typedef union _jnl_info {
	FLSEJTRANINFO		tran_info;
	FLSEJERRINFO		err_info;
	FLSEJADDBILLINFO	add_bill_info;
	FLSEJCSTCLINFO		cst_close_info;
	FLSEJDAYCLINFO		day_close_info;
	EJCHANGEINFO		change_info;
	FLSEJSETDENOMINFO	denomination_info;
	//FLSEJKEYINFO		key_info;
	//FLSEJPWDINFO		pwd_info;
	FLSEJVERSION		version_info;
	FLSEJSTATCSTCLINFO	stat_cst_close_info;
	FLSEJSTATDAYCLINFO	stat_day_close_info;
	EJTEXTINFO			text_info;
	FLSEJTESTDISPENSE			test_dispense_info;
} FLSEJINFO, *LPFLSEJINFO;

//added at 17/11/2017 for Errorcode data - not use
typedef struct _jnl_err_code {
	CHAR	err_code[8];
	CHAR	err_desc[1024];
	CHAR	err_recovery[1024];
} ERRCODEINFO, *LPERRCODEINFO;

#pragma pack()