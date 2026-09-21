#pragma once

#include <vector>


//////////////////////////////////////////////////
/*				 DEVICE.CFG						*/
//////////////////////////////////////////////////
typedef struct T_EAGLE_CONFIG_DEVICE
{
// DEVICE_PORT
	CString					strPortNumber_CDM;
	CString					strPortNumber_CDR;
	CString					strPortNumber_DIO;
	CString					strPortNumber_EPP;
	CString					strPortNumber_SHT;
	CString					strPortNumber_PTR;
	CString					strPortNumber_MUB;

// DEVICE_EPP
	CString					strType_EPP;

// DEVICE_CDR
	CString					strEMV_Enable;

// DEVICE_CAM
	CString					strCAM_Enable;


}EAGLE_CONFIG_DEVICE, *LPEAGLE_CONFIG_DEVICE;


//////////////////////////////////////////////////
/*				 SYSTEM.CFG						*/
//////////////////////////////////////////////////
typedef struct T_EAGLE_CONFIG_SYSTEM
{
	// PASSWORD
	CString					strAdministrator_password;				// Operator's administrator password

	CString					strMaster_password;						// Operator's master password

	CString					strReplenishment_password;				// Operator's replenishment password

	CString					strBackUpLogIndex;						// Log Backup Index (00 ~ 500)

	CString					strSequence_Number;						// Sequence Number (0001 ~ 9999)

	CString					strRms_Password;

	CString					strLogMgr_Version;

	CString					strCaptureImageFile_Index;				// Capture Image File Index

}EAGLE_CONFIG_SYSTEM, *LPEAGLE_CONFIG_SYSTEM;


//////////////////////////////////////////////////
/*				 HOST.CFG						*/
//////////////////////////////////////////////////
typedef struct T_EAGLE_CONFIG_HOST
{
// TCP IP
	CString					strHostip;								// Host IP 1
	CString					strBackup_Hostip;						// Back UpHost IP 
	CString					strPort;								// Port
	CString					strBackupPort;							// Back Up Port

	CString					strUseDHCP;								// Use DHCP
	CString					strTerminalip;							// Terminal IP
	CString					strSubnet;								// Subnet Mask
	CString					strGateway;								// Gate Way
	CString					strDNS;									// DNS
	CString					strNetworkType;							// Network Type
	CString					strProtocolType;						// Protocol Type
	CString					strUseENQ;								// ENQ Option
	CString					strUseEOT;								// EOT Option
	CString					strUseTLS;								// use TLS
	CString					strUseCurl;								// use libcurl for the host line (LINE_CURL) instead of OpenSSL (LINE_TCP)

// MODEM
	CString					strCommunication_header;				// 호스트 통신시, communication header 사용여부 결정 : '0'-disable '1'-enable
	CString					strCommunicationID;						// 호스트 통신시, routing 되는 communications id
	CString					strPrimary_phone_number;				// Host Processor의 Primary Phone Number (space filled to the left)
	CString					strBackup_phone_number;					// Host Processor의 Backup Phone Number (space filled to the left)
	CString					strPre_dial;							// pre_dial 기능 사용 여부 '0'-disable '1'-enable
	CString					strSetup_String;						// Modem Setup String

	CString					strCrc_flg;								// '1' - TRUE, '0' - FALSE

// NET_MOTNITOR
	CString					strAtm_status_monitoring;				// ATM 상태 전송 여부
	CString					strHeartbeat_message;					// heartbeat message 사용 여부
	CString					strHeartbeat_frequency;					// heartbeat message 발생 주기, 분 단위 값

// AMS
	CString					strRms_en_flg;							// '0' - disable, '1' - enable
	CString					strRms_primary_num;

	CString					strRms_Status_en_flg;					// AMS Status Send
	CString					strRms_IP_Address;						// TCPIP IP info
	CString					strRms_Port;							// TCPIP port

	CString					strSchedule_journal_flg;				// 특정 시간에 자동으로 저널 데이터 백업 수행 여부 : '0'-disable '1'-enable
	CString					strSchedule_journal_count;				// 자동 저널 시간 설정 : 24hour

}EAGLE_CONFIG_HOST, *LPEAGLE_CONFIG_HOST;


//////////////////////////////////////////////////
/*				 BINLIST.CFG					*/
//////////////////////////////////////////////////
typedef struct T_EAGLE_CONFIG_ISO
{
// Block ISO
	int		nTotalCount;
	CString	strBLOCK_ISO[MAX_BIN_COUNT];							// ISO Block Data
}EAGLE_CONFIG_ISO, *LPEAGLE_CONFIG_ISO;


//////////////////////////////////////////////////
/*				 MESSAGE.CFG					*/
//////////////////////////////////////////////////
typedef struct T_EAGLE_CONFIG_MESSAGE
{
// OPTION_MSG
	CString					strMarketing_message[4];				// Marketing message line1~4

	CString					strLocation_message[4];					// Location message line1~3 (Store message)

	CString					strAttraction_message[3];				// Attraction message line1~3 (welcome message)

	CString					strFarewell_message[3];					// Farewell message line1~3 (Exit message)
}EAGLE_CONFIG_MESSAGE, *LPEAGLE_CONFIG_MESSAGE;


// OPTION.CFG
typedef struct T_EAGLE_CONFIG_OPTION
{
// TRANSACTION
	CString					strSaving_account;						// Saving 계좌 사용 여부 : '0'-disable '1'-enable
	CString					strCredit_account;						// Credit 계좌 사용 여부 : '0'-disable '1'-enable
	CString					strtransfer_Enable;						// Transfer 거래 사용 여부 : '0'-disable '1'-enable
	CString					strbalance_Enable;						// balance 거래 사용 여부 : '0'-disable '1'-enable
	CString					strFast_amount[6];						// Fast menu amount
	//CString					strExtended_amount;						// 금액의 확장 자리수 사용 여부 : '0'-disable '1'-enable
	CString					strMax_withdrawal_amount;				// 최대 인출 금액
	CString					strSurcharge_type;						// Surcharge type '0'-amount '1'-percent
	CString					strWithdrawalSurcharge_amount;			// Surcharge amount : *.**
	CString					strSurcharge_percentage;				// Surcharge percentage : 0.01~99.99%
	CString					strWithdrawal_PercentSurcharge_Option;	// '0'-lesser '1'-greater
	CString					strSurcharge_owner;						// Surcharge owner
	CString					strBalaceSurcharge_amount;				// Balance Surcharge amount

// ETC
	CString					strTerminal_ID;							// Terminal ID
	CString					strMachine_Serial_Number;				// 기기 시리얼 번호
	CString					strLanguage;							// 언어 설정 : '0'-Eng, '1'- SPN '2'-Hindi '3'-French Canadian
	CString					strLansupport[6];							// Englsih En/Disable
	CString					strScheduleDayClose;					// Schedule Auto Day Close En/Disable
	CString					strDayCloseTime;						// Auto Day Close Time : HHMM
	CString					strDayCloseYear;						// Auto Day Close Year
	CString					strDayCloseMonth;						// Auto Day Close Month
	CString					strDayCloseDay;							// Auto Day Close Day

	CString					strSound_Volume;						// 음성 안내 볼륨 선정 : 0~100

	CString					strRebootYear;							// Reboot Year
	CString					strRebootMonth;							// Reboot Month
	CString					strRebootDay;							// Reboot Day
	CString					strRebootTime;							// Reboot : HHMM

}EAGLE_CONFIG_OPTION, *LPEAGLE_CONFIG_OPTION;

// ADS.CFG
typedef struct T_EAGLE_CONFIG_ADS
{
// AD
	CString					strAdver_Interval_time;					// 광고 교대 시간 설정 (millisecond 단위)
	CString					strAdver_Screen_en[SUPPORT_ADVERTISE];		// '0'-disable  '1'-enable
}EAGLE_CONFIG_ADS, *LPEAGLE_CONFIG_ADS;

// TOTAL,CFG
typedef struct T_EAGLE_TOTAL_INFO
{
	void Clear(void);
	void SumData(CString strTransType, CString strAmount);

	// Transaction Total Information
	CString				Withdrawal_Count;					// 출금 거래 건수
	CString				BalanceInquiry_Count;				// 조회 거래 건수
	CString				Transfer_Count;						// 이체 거래 건수
	CString				Withdrawal_Amount;					// 출금 총 금액
}EAGLE_TOTAL_INFO, *LPEAGLE_TOTAL_INFO;

// 거래 중 Power Off 처리를 위해 마지막 거래 정보를 File에 저장
// 거래 완료 후 해당 값은 모두 초기화 처리함
// LAST TRANSACTION INFORMATION,CFG
typedef struct T_EAGLE_LAST_TRANS_INFO
{
	void Clear(void);

	// Transaction Total Information
	CString				m_strTrack1Data;
	CString				m_strTrack2Data;
	CString				m_strTrack3Data;
	CString				m_strShowCardNumber;				// 명세표 및 저널에 표시 또는 Print할 Card Number
	CString				m_strTransType;						// Host로 송신한 거래 종류
	CString				m_strFromAccountType;				// Host로 송신할 계좌 종류
	CString				m_strToAccountType;					// Host로 송신할 계좌 종류
	CString				m_strRequestAmount;					// Host로 송신한 요청금액
	CString				m_strDispenseAmount;				// 방출금액
	CString				m_strSurchargeAmount;				// Host로 송신 or 수신 받은 Surcharge (수신시 갱신됨)
	CString				m_strTransferAmount;

	CString				m_strAutorizationNumber;			// Host로부터 수신 받은 승인번호
	CString				m_strTransactionDate;				// Host로부터 수신 받은 거래 날짜
	CString				m_strTransactionTime;				// Host로부터 수신 받은 거래 시간
	CString				m_strBusinessDate;					// Host로부터 수신 받은 Business Date
	CString				m_strAvailableBalance;				// Host로부터 수신 받은 Available Balance
	CString				m_strBalance;						// Host로부터 수신 받은 Balance

	CString				m_strDispensedCount;
	CString				m_strRejectedCount;

	int					m_nIsReversal;
	CString				m_strReasonforReversal;
	int					m_nProtocol_processing_state;
	BOOL				m_bSaveEJLforTransRecord;

	// EMV Information
	CString				m_str_AID;
	CString				m_str_AID_Label;

	CString				m_strTLV_57;
	CString				m_strTLV_5A;
	CString				m_strTLV_8A;
	CString				m_strTLV_82;
	CString				m_strTLV_95;
	CString				m_strTLV_9A;
	CString				m_strTLV_9C;
	CString				m_strTLV_5F2A;
	CString				m_strTLV_5F34;
	CString				m_strTLV_9F02;
	CString				m_strTLV_9F03;
	CString				m_strTLV_9F10;
	CString				m_strTLV_9F18;
	CString				m_strTLV_9F1A;
	CString				m_strTLV_9F26;
	CString				m_strTLV_9F27;
	CString				m_strTLV_9F33;
	CString				m_strTLV_9F35;
	CString				m_strTLV_9F36;
	CString				m_strTLV_9F37;
	CString				m_strTLV_9F39;
	CString				m_strTLV_9F41;
	CString				m_strTLV_DF05;

	CString				m_strCST_Reject_Info[MAX_CST_COUNT];
	CString				m_strCST_Remain_Cnt;


}EAGLE_LAST_TRANS_INFO, *LPEAGLE_LAST_TRANS_INFO;

typedef struct T_EAGLE_CBX_INFO
{
	void Clear_Reject_Info(void);

	void Clear_CBX_Count();

	// Reject Information
	CString					strCBX_Reject_Reason[4];	// Reject 사유

	// CBX Count 및 잔량 관리 정보 위치 이동 (DEVICE -> CBX)
	CString					strCBX_Count;							// 카세트 개수 : '1' or '2' or '3' or 4
	CString					strCBX_Use[4];							// 카세트 용도 : '0'-Cash , '1'-Coupon
	CString					strCBX_Load_Count[4];					// 카세트 loaded number of bills
	CString					strCBX_Item_Count[4];					// 카세트 current number of bills
	CString					strCBX_Dispense_Count[4];				// 카세트 dispense number of bills
	CString					strCBX_Reject_Count[4];					// 카세트 reject count
	CString					strCBX_Denomination[4];					// 카세트의 권종

	CString					strRBN_Item_Count;						// Reject bin count

	//	CString					strDispType;							// Dispense Type(동일한 권종에 대한 방출 순서 결정)('0'-순차방출, '1'-균등방출)

}EAGLE_CBX_INFO, *LPEAGLE_CBX_INFO;
