/////////////////////////////////////////////////////////////////////////////
#if !defined(_Profiledef_H_)
#define _Profiledef_H_

#include "..\Dev\DevDefine.h"

// Machine Type
#define ATM				1			// ATM
#define CDP				2			// CDP
#define CD				3			// CD

// Machine Kind
#define NANOCASH		1			// NANOCASH
#define MCD				2			// MCD
#define TTW				3			// TTW
#define MCD4			4			// MCD4
#define TTW2			5			// TTW2 (7040)
#define MB2500N			6			// MB2500N
#define MBTTW			7			// MBTTW
#define WTTW			8			// WTTW (5040)
#define CATM			9			// CATM (9030)

// Spr Type
#define SPR_T_TYPE		1			// THERMAL TYPE
#define SPR_D_TYPE		2			// DOT     TYPE
#define SPR_TF_TYPE		3			// THERMAL(FORM) TYPE
#define SPR_DF_TYPE		4			// DOT    (FORM) TYPE

// Jpr Type
#define JPR_T_TYPE		1			// THERMAL TYPE
#define JPR_D_TYPE		2			// DOT     TYPE

// Mcu Type
#define MCU_M_TYPE		1			// MOTOR TYPE
#define MCU_S_TYPE		2			// SWIPE TYPE
#define MCU_D_TYPE		3			// DIP   TYPE
#define MCU_M_NS_TYPE	4			// MOTOR TYPE(NO SHUTTER)

// Device
#define NOT_SET			0			// NOT SET
#define BRM_SET			1			// BRM
#define CDU_SET			2			// CDU
#define PBM_SET		 	1			// AUTO TURN
#define PBM_NAT_SET 	2			// NO	AUTO	TURN
#define SOUND_SET		1			// SOUND
#define PC_SOUND_SET	2			// PC SPEAKER
#define PCI_CAMERA1_SET	1			// PCI TYPE CAMERA1
#define USB_CAMERA1_SET	2			// USB TYPE CAMERA1
#define PCI_CAMERA2_SET	3			// PCI TYPE CAMERA2
#define USB_CAMERA2_SET	4			// USB TYPE CAMERA2
#define ICCARD_SET		1			// IC CARD
#define SPL_SET			1			// SPL
#define VFD_SET			1			// VFD

// Key type
#define DRIVER_TYPE		1
#define TOUCH_TYPE		2

// Monitor
#define ONE_LCD			0
#define DUAL_LCD		1

// Op Device
#define OP_FRONT		0
#define OP_REAR			1
#define OP_TSPL			2
#define OP_HSPL			3

#pragma pack(1)

/////////////////////////////////////////////////////////////////////////////
// NVRAM MEMORY STRUCTURE
/////////////////////////////////////////////////////////////////////////////

// KSK 2009.12.23 structure 사용 안함
typedef struct tagShareData{
	char chOP_COMMAND[150];			// OP_COMMAND			//  150
	char chOP_RESULT[40];			// OP_RESULT			//  190
	char chOP_MODE[10];				// OP_MODE				//  200
	char chOP_APSTATUS[10];			// OP_APSTATUS			//  210
	char chOP_STATUS[250];			// OP_STATUS			//  460
	char chOP_HEADER[300];			// OP_HEADER			//  760
	char chOP_TOTAL[10];			// OP_TOTAL				//  770
	char chOP_ADDCASH[30];			// OP_ADDCASH			//  800
	char chOP_JOURNAL[26];			// OP_JOURNAL			//  826
	char chOP_REPORT[26];			// OP_REPORT			//  852
	char chOP_CUSTOMERSETUP[84];	// OP_CUSTOMERSETUP		//  938
	char chOP_SYSTEMSETUP[180];		// OP_SYSTEMSETUP		// 1118
	char chOP_HOSTSETUP[250];		// OP_HOSTSETUP			// 1368
	char chOP_TRANSETUP[16];		// OP_TRANSETUP			// 1384
	char chOP_FASTCASH[40];			// OP_FASTCASH			// 1424
	char chOP_DENOMINATION[20];		// OP_DENOMINATION		// 1444
	char chOP_DISPCASH[100];		// OP_DISPCASH			// 1544
	char chOP_VIEWJOURNAL[300];		// OP_VIEWJOURNAL		// 1844
	char chOP_BINLIST[400];			// OP_BINLIST			// 2244
	char chOP_PRINTBYINDEX[6];		// OP_PRINTBYINDEX		// 2250
	char chOP_PRINTBYDATE[10];		// OP_PRINTBYINDEX		// 2260
	char chOP_RECEIPTHEADER1[42];   // OP_RECEIPTHEADER1	// 2302
	char chOP_RECEIPTHEADER2[42];   // OP_RECEIPTHEADER2	// 2344
	char chOP_RECEIPTTAIL[42];      // OP_RECEIPTTAIL		// 2386
	char chOP_RECEIPTADDRESS1[42];  // OP_RECEIPTADDRESS1	// 2428
	char chOP_RECEIPTADDRESS2[42];  // OP_RECEIPTADDRESS2	// 2470
	char chOP_RECEIPTADDRESS3[42];  // OP_RECEIPTADDRESS3	// 2512
	char chOP_RECEIPTPHONENO[42];   // OP_RECEIPTPHONENO	// 2554
	char chOP_VFDTEXTNO[1];         // OP_VFDTEXTNO			// 2555
	char chOP_VFDDISPLAYTEXT[30];   // OP_VFDDISPLAYTEXT	// 2585
	char chOP_EJCANCEL[10];			// OP_EJCANCEL			// 2595
	char chOP_SEARCHJOURNAL[14];	// OP_SEARCHJOURNAL		// 2609
	char chOP_NVRAMCHECKSTATUS[10]; // OP_NVRAMCHECKSTATUS	// 2619
	char chOP_RebootOption[22];     // OP_REBOOTOPTION  EX) DISABLE/EVERYDAY/24		// 2641
} SHAREDATA, NEAR* NPSHAREDATA, FAR* LPSHAREDATA;
// end of KSK 2009.12.23

typedef struct tagDeviceInfo {
	WORD	wNVRamReset;			// 0:NOT_SET, 1:RESET DONE	// KSK 2009.12.23 사용 안함
	WORD	wLogBackupIdx;			// 0~99 log file index		//---사용함---//
	WORD	MachineType;			// 0:NOT_SET, 1:ATM, 2:CDP, 3:CD	// KSK 2009.12.23 사용 안함 (Optional2에서 재정의하여 사용함)
	WORD	MachineKind;			// 0:NOT_SET, 1:NANOCASH, 2:MCD, 3:TTW, 4:MCD4, 5:TTW2, 6:MB2500N, 7:MBTTW	// KSK 2009.12.23 사용 안함 (Optional2재정의하여 사용함)
	WORD	SprHandler;				// 0:NOT_SET, 1:THERMAL, 2:DOT, 3:THERMAL(FORM), 4:DOT(FORM)	// KSK 2009.12.23 사용 안함

	WORD	JprHandler;				// 0:NOT_SET, 1:THERMAL, 2:DOT	// KSK 2009.12.23 사용 안함
	WORD	McuHandler;				// 0:NOT_SET, 1:MCU_M_TYPE, 2:MCU_S_TYPE, 3:MCU_D_TYPE, 4:MCU_M_NS_TYPE	// KSK 2009.12.23 사용 안함
	WORD	CashHandler;			// 0:NOT_SET, 1:BRM, 2:CDU		// KSK 2009.12.23 사용 안함
	WORD	PBHandler;				// 0:NOT_SET, 1:PBM, 2:PBM(NO AUTO TURN)	// KSK 2009.12.23 사용 안함
	
	WORD	KeyHandler;				// 0:NOT_SET, 1:DRIVER_TYPE, 2:TOUCH_TYPE	// KSK 2009.12.23 사용 안함
	WORD	SoundDevice;			// 0:NOT_SET, 1:INSTALL						// KSK 2009.12.23 사용 안함
	WORD	CameraDevice;			// 0:NOT_SET, 1:PCI TYPE CAMERA1, 2:USB TYPE CAMERA1, 3:PCI TYPE CAMERA2, 4:USB TYPE CAMERA2	// KSK 2009.12.23 사용 안함
	WORD	ICCardDevice;			// 0:NOT_SET, 1:INSTALL	// KSK 2009.12.23 사용 안함
	
	WORD	PowerHandler;			// 0:NOT_SET, 1:PNC_TYPE, 2:UPS_TYPE	// KSK 2009.12.23 사용 안함
	WORD	SplDevice;				// 0:NOT_SET, 1:INSTALL(NCR TYPE)		// KSK 2009.12.23 사용 안함
	WORD	VfdDevice;				// 0:NOT_SET, 1:INSTALL					// KSK 2009.12.23 사용 안함
	WORD	DualDevice;				// 0:NOT_SET, 1:DUAL MONITOR			// KSK 2009.12.23 사용 안함
	
	WORD	DvrDevice;				// 0:NOT_SET, 1:DVR						// KSK 2009.12.23 사용 안함
	WORD	OpDevice;				// 0:NOT_SET, 1:OP_TOUCH, 2:OP_TSPL, 3:OP_HSPL	// KSK 2009.12.23 사용 안함
	WORD	McuDevice;				// 0:NOT_SET, 1:MCU_M_OMRON, 2:MCU_M_KDE, 3:MCU_M_HYOSUNG, 4:MCU_M_SANKYO, 5:MCU_D_MAGTEK	// KSK 2009.12.23 사용 안함
	WORD	CduShutter;				// 0:NO SHUTTER, 1: SHUTTER EXISTS		// KSK 2009.12.23 사용 안함

	WORD wTotalSetCashCst1;			// _MEM_VAR_DEVINFO_TOTALSETCASHCST1	// KSK 2009.12.23 사용 안함
	WORD wTotalSetCashCst2;			// _MEM_VAR_DEVINFO_TOTALSETCASHCST2	// KSK 2009.12.23 사용 안함
	WORD wTotalSetCashCst3;			// _MEM_VAR_DEVINFO_TOTALSETCASHCST3	// KSK 2009.12.23 사용 안함
	WORD wTotalSetCashCst4;			// _MEM_VAR_DEVINFO_TOTALSETCASHCST4	// KSK 2009.12.23 사용 안함
	
	WORD wDispCashCst1;				// _MEM_VAR_DEVINFO_DISPCASHCST1		// KSK 2009.12.23 사용 안함
	WORD wDispCashCst2;				// _MEM_VAR_DEVINFO_DISPCASHCST2		// KSK 2009.12.23 사용 안함
	WORD wDispCashCst3;				// _MEM_VAR_DEVINFO_DISPCASHCST3		// KSK 2009.12.23 사용 안함
	WORD wDispCashCst4;				// _MEM_VAR_DEVINFO_DISPCASHCST4		// KSK 2009.12.23 사용 안함
	
	WORD wResetSysConfig;			// _MEM_VAR_DEVINFO_RESETSYSCONFIG		// KSK 2009.12.23 사용 안함
	WORD wRejectTotCnt;				// _MEM_VAR_DEVINFO_REJECTTOTCNT		// KSK 2009.12.23 사용 안함

	WORD wRejectCnt1;				// _MEM_VAR_DEVINFO_REJECTCNT1			// KSK 2009.12.23 사용 안함
	WORD wRejectCnt2;				// _MEM_VAR_DEVINFO_REJECTCNT2			// KSK 2009.12.23 사용 안함
	WORD wRejectCnt3;				// _MEM_VAR_DEVINFO_REJECTCNT3			// KSK 2009.12.23 사용 안함
	WORD wRejectCnt4;				// _MEM_VAR_DEVINFO_REJECTCNT4			// KSK 2009.12.23 사용 안함

	WORD wCashUserCount1;			// _MEM_VAR_DEVINFO_CSTUSERCOUNT1		// KSK 2009.12.23 사용 안함
	WORD wCashUserCount2;			// _MEM_VAR_DEVINFO_CSTUSERCOUNT2		// KSK 2009.12.23 사용 안함
	WORD wCashUserCount3;			// _MEM_VAR_DEVINFO_CSTUSERCOUNT3		// KSK 2009.12.23 사용 안함
	WORD wCashUserCount4;			// _MEM_VAR_DEVINFO_CSTUSERCOUNT4		// KSK 2009.12.23 사용 안함

	WORD wCashCustomerCount1;		// _MEM_VAR_DEVINFO_CSTDISPCOUNT1		// KSK 2009.12.23 사용 안함
	WORD wCashCustomerCount2;		// _MEM_VAR_DEVINFO_CSTDISPCOUNT2		// KSK 2009.12.23 사용 안함
	WORD wCashCustomerCount3;		// _MEM_VAR_DEVINFO_CSTDISPCOUNT3		// KSK 2009.12.23 사용 안함
	WORD wCashCustomerCount4;		// _MEM_VAR_DEVINFO_CSTDISPCOUNT4		// KSK 2009.12.23 사용 안함

	WORD wCassette1;				// _MEM_VAR_DEVINFO_CASSETTE1			// KSK 2009.12.23 사용 안함
	WORD wCassette2;				// _MEM_VAR_DEVINFO_CASSETTE2			// KSK 2009.12.23 사용 안함
	WORD wCassette3;				// _MEM_VAR_DEVINFO_CASSETTE3			// KSK 2009.12.23 사용 안함
	WORD wCassette4;				// _MEM_VAR_DEVINFO_CASSETTE4			// KSK 2009.12.23 사용 안함

	WORD wRjtCassette1;				// _MEM_VAR_DEVINFO_RJTCASSETTE1		// KSK 2009.12.23 사용 안함
	WORD wRjtCassette2;				// _MEM_VAR_DEVINFO_RJTCASSETTE2		// KSK 2009.12.23 사용 안함
	WORD wRjtCassette3;				// _MEM_VAR_DEVINFO_RJTCASSETTE3		// KSK 2009.12.23 사용 안함
	WORD wRjtCassette4;				// _MEM_VAR_DEVINFO_RJTCASSETTE4		// KSK 2009.12.23 사용 안함

	WORD wDispensedNote1;			// _MEM_VAR_DEVINFO_DISPNOTE1			// KSK 2009.12.23 사용 안함
	WORD wDispensedNote2;			// _MEM_VAR_DEVINFO_DISPNOTE2			// KSK 2009.12.23 사용 안함
	WORD wDispensedNote3;			// _MEM_VAR_DEVINFO_DISPNOTE3			// KSK 2009.12.23 사용 안함
	WORD wDispensedNote4;			// _MEM_VAR_DEVINFO_DISPNOTE4			// KSK 2009.12.23 사용 안함

	WORD wVolumeLevel;				// _MEM_VAR_DEVINFO_VOLUMELEVEL			//---사용함---//

	WORD wCurrentDispCnt1;			// _MEM_VAR_DEVINFO_DISPENSE_CST1		// KSK 2009.12.23 사용 안함
	WORD wCurrentDispCnt2;			// _MEM_VAR_DEVINFO_DISPENSE_CST2		// KSK 2009.12.23 사용 안함
	WORD wCurrentDispCnt3;			// _MEM_VAR_DEVINFO_DISPENSE_CST3		// KSK 2009.12.23 사용 안함
	WORD wCurrentDispCnt4;			// _MEM_VAR_DEVINFO_DISPENSE_CST4		// KSK 2009.12.23 사용 안함		//  120

	WORD wJackDetect;				// _MEM_VAR_DEVINFO_JACKDETECT			// KSK 2009.12.23 사용 안함		//  122
	WORD wRebootFlag;				// _MEM_VAR_DEVINFO_REBOOT_FLAG			// KSK 2009.12.23 사용 안함		//  124
	char chReboot[10];				// _MEM_VAR_DEVINFO_REBOOT				// KSK 2009.12.23 사용 안함		//  134
	char chRebootDay[10];			// _MEM_VAR_DEVINFO_REBOOT_DAY			// KSK 2009.12.23 사용 안함		//  144
	char chRebootTime[4];			// _MEM_VAR_DEVINFO_REBOOT_DAY			// KSK 2009.12.23 사용 안함		//  148

	char chLogBackupDate[10];		// _MEM_VAR_DEVINFO_LOGBACKUPDATE		// KSK 2009.12.23 사용 안함		//  158
	char chAPVersion[10];			// _MEM_VAR_DEVINFO_APVERSION			// KSK 2009.12.23 사용 안함		//  168
	char chMWIVersion[10];			// _MEM_VAR_DEVINFO_MWIVERSION			// KSK 2009.12.23 사용 안함		//  178
	char chCSTDenomination[20];		// _MEM_VAR_DEVINFO_CSTDENOM			// KSK 2009.12.23 사용 안함		//  198
	WORD wSPRPaperStatus;			//_MEM_VAR_DEVINFO_SPRSP_PAPERSTAT		// KSK 2009.12.23 사용 안함		//  200

} DEVICEINFO, NEAR* NPDEVICEINFO, FAR* LPDEVICEINFO;

typedef struct tagNetworkInfo{

	WORD	BranchInOut;			// Branch In/Out						// KSK 2009.12.23 사용 안함
	WORD	AutoBranchOnOff;		// Auto Branch On/Off					// KSK 2009.12.23 사용 안함
	WORD	All24OnOff;				// All 24On/Off							// KSK 2009.12.23 사용 안함
	
	WORD	All365OnOff;			// All 365On/Off						// KSK 2009.12.23 사용 안함
	WORD	Interface;				// IOC, HDLC, SWP, SDLC, TCPIP, CCL, BAS	// KSK 2009.12.23 사용 안함
	WORD	InterfaceKind;			// 0:CUP(WH)				// 2004.05.08	// KSK 2009.12.23 사용 안함
									// 1:BOC(SD)				// 2004.05.08
									// 2:ABC(SD)				// 2004.06.18
									// 3:ABC(FZ,XM,ZJ)			// 2004.06.25
									// 4:RCC(GZ)				// 2004.11.05
									// 5:ABC(ZJG)				// 2004.11.05
									// 6:CCB(JX)				// 2004.11.05
									// 7:ABC(BJ,FZ,XM)			// 2004.10.24
									// 8:POST(SC)				// 2005.02.28
									// 9:SDB(SZ)				// 2005.04.27
									//10:CUP(TJ)				// 2005.05.12
									//11:CUP(BCTC)				// 2005.08.17
									//12:CUP(JN)				// 2005.09.11
									//13:CUP(WH8583)			// 2005.10.20
									//14:CUP(XX)				// 2005.09.11
									//15:CUP(XX)				// 2005.09.11
									//16:CUP(XX)				// 2005.09.11
									//17:CUP(XX)				// 2005.09.11
									//18:CUP(XX)				// 2005.09.11
									//19:CUP(XX)				// 2005.09.11
									//20:CUP(XX)				// 2005.09.11
									//21:CUP(XX)				// 2005.09.11
									//22:CUP(XX)				// 2005.09.11
									//23:CUP(XX)				// 2005.09.11
									//24:CUP(XX)				// 2005.09.11
									//25:CUP(XX)				// 2005.09.11
									//26:CUP(XX)				// 2005.09.11
									//27:CUP(XX)				// 2005.09.11
									//28:CUP(XX)				// 2005.09.11
									//29:CUP(XX)				// 2005.09.11
									//30:CUP(XX)				// 2005.09.11

									//1000:CB(JN)				// 2005.10.20 : DC

									//2000:CB(JN)				// 2005.10.20 : FT
	WORD	DeviceAddr;				// DA										// KSK 2009.12.23 사용 안함
	WORD	SelectAddr;				// SA										// KSK 2009.12.23 사용 안함
	
	WORD	SendRecvSize;			// Send/Recv SIZE							// KSK 2009.12.23 사용 안함
	WORD	LineSpeed;				// LINE SPEED								// KSK 2009.12.23 사용 안함
	WORD	PollCheck;				// POLL CHECK ON/OFF						// KSK 2009.12.23 사용 안함
	WORD	Point;					// PTP, MTP									// KSK 2009.12.23 사용 안함
	
	WORD	Duplex;					// HALF_DUPLEX, FULL_DUPLEX					// KSK 2009.12.23 사용 안함
	WORD	TimeFiller;				// MTF, FTF									// KSK 2009.12.23 사용 안함
	WORD	RsType;					// RS-422, RS-232C							// KSK 2009.12.23 사용 안함
	WORD	Nrz;					// NRZ, NRZI								// KSK 2009.12.23 사용 안함

	WORD	Clk;					// EXCLK, INCLK								// KSK 2009.12.23 사용 안함
	WORD wStatusMonitorFlag;		// _MEM_VAR_APP_STATUSMONITORINGENABLE		//---사용함---//
	WORD wCommHeaderFlag;			// _MEM_VAR_APP_COMMUNICATIONHEADERENABLE	//---사용함---//
	WORD wENQTimeOut;				// _MEM_VAR_APP_ENQTIMEOUT					// KSK 2009.12.23 사용 안함으로 수정

	WORD wFormatOption;				// _MEM_VAR_NETINFO_MSGDEFINE_FORMATOPTION
	WORD wReceTimeOut;				// _MEM_VAR_APP_TIMEDEFINE_RECVTIMEOUT		// KSK 2009.12.23 사용 안함
	WORD wSendTimeOut;				// _MEM_VAR_APP_TIMEDEFINE_SENDTIMEOUT		// KSK 2009.12.23 사용 안함
	WORD wOpenRetryTime;			// _MEM_VAR_APP_TIMEDEFINE_OPENRETRYTIME	// KSK 2009.12.23 사용 안함
	
	WORD wLineRetryTime;			// _MEM_VAR_APP_TIMEDEFINE_LINERETRYTIME	// KSK 2009.12.23 사용 안함
	WORD wMACFlag;					// _MEM_VAR_APP_MACFLAG						//---사용함---//
	WORD wConnectTimer;				// _MEM_VAR_APP_CONNECTTIMER				// KSK 2009.12.23 사용 안함
	WORD wModemSpeaker;				// _MEM_VAR_APP_MODEMSPEAKER				// KSK 2009.12.23 사용 안함
	
	WORD wHealthSendDelayInterval;	// _MEM_VAR_APP_HSDELAYINT					//---사용함---//
	WORD wHealthEnableFlag;			// _MEM_VAR_NETINFO_HEALTHYFLAG				//---사용함---//
	WORD wRMSRingCount;				// _MEM_VAR_APP_RMSRINGCNT					//---사용함---//
	WORD wRMSSendFlag;				// _MEM_VAR_APP_RMSSENDFLAG					//---사용함---//
	
	WORD wRMSStatusSend;			// _MEM_VAR_APP_RMSSTATUSSEND				//---사용함---//
	WORD wRMSCommand;															//---사용함---//
	WORD wDummy2;																// KSK 2009.12.23 사용 안함																
	WORD wDummy3;																// KSK 2009.12.23 사용 안함

	char	AtmSerialNum[10];		// Atm Serial Number						// KSK 2009.12.23 사용 안함
	char	BranchNum[10];			// Branch Number							// KSK 2009.12.23 사용 안함
	char	SubBranchNum[10];		// Sub Branch Number						// KSK 2009.12.23 사용 안함
	char	RoomNum[10];				// Room Number							// KSK 2009.12.23 사용 안함
	char	MachineMakeDate[10];		// Machine Make Data					// KSK 2009.12.23 사용 안함
	char	MachineMakeNum[18];		// Machine Make Number						// KSK 2009.12.23 사용 안함

	char	BpIpAddress[16];		// BP Ip Address							// KSK 2009.12.23 사용 안함
	char	BpPortNo[6];			// BP Port No								// KSK 2009.12.23 사용 안함
	char	BpIpAddress2[16];		// BP Ip Address2							// KSK 2009.12.23 사용 안함
	char	BpPortNo2[6];			// BP Port No2								// KSK 2009.12.23 사용 안함
	char	BpIpAddress3[16];		// BP Ip Address3							// KSK 2009.12.23 사용 안함
	char	BpPortNo3[6];			// BP Port No3								// KSK 2009.12.23 사용 안함
	char	BpIpAddress4[16];		// BP Ip Address4							// KSK 2009.12.23 사용 안함
	char	BpPortNo4[6];			// BP Port No4								// KSK 2009.12.23 사용 안함
    char	AtmIPAddress[16];		// Atm IP Address							// KSK 2009.12.23 사용 안함
    char	AtmSubnetMask[16];		// Atm Subnet Mask							// KSK 2009.12.23 사용 안함
    char	AtmGateWay[16];			// Atm GateWay								// KSK 2009.12.23 사용 안함
	char	AtmPortNo[6];			// Atm Port No								// KSK 2009.12.23 사용 안함

	char chCommID[10];				// _MEM_VAR_APP_COMMUNICATIONID				//---사용함---//
	char chCommHeader[10];			// _MEM_VAR_APP_COMMHEADER					//---사용함---//
	char chCustomerRetError[6];		// _MEM_VAR_APP_CUSTOMRETERR				//---사용함---//
	char chEOTCheckMode[10];		// _MEM_VAR_APP_EOTCHECKMODE				//---사용함---//
	char chModemVendor[2];			// _MEM_VAR_APP_MODEMVENDOR					// KSK 2009.12.23 사용 안함으로 수정
	char chAccountNumber[30];		// _MEM_VAR_APP_ACCOUNTNUMBER				// KSK 2009.12.23 사용 안함
	char chRequestType[4];			// _MEM_VAR_APP_REQUESTTYPE					// KSK 2009.12.23 사용 안함
	char chPhoneNumber1[22];		// _MEM_VAR_APP_PHONENODEFINE1				//---사용함---//
	char chPhoneNumber2[22];		// _MEM_VAR_APP_PHONENODEFINE2				//---사용함---//
	char chModemInitialStr[50];		// _MEM_VAR_APP_MODEMINITSTR				//---사용함---//
	char chModemTestNum[22];		// _MEM_VAR_APP_MODEMTESTNO					// KSK 2009.12.23 사용 안함
	char chLocalDate[10];			// _MEM_VAR_APP_LOCALDATE					// KSK 2009.12.23 사용 안함
	char chLocalTime[8];			// _MEM_VAR_APP_LOCALTIME					// KSK 2009.12.23 사용 안함
	char chHealthMsgTimer[8];		// _MEM_VAR_APP_HEALTHMSGTIMER				// KSK 2009.12.23 사용 안함
	char chDESKey1[18];				// _MEM_VAR_APP_DESKEY1						//---사용함---//
	char chDESKey2[18];				// _MEM_VAR_APP_DESKEY2						//---사용함---//
	char chDESKey3[18];				// _MEM_VAR_APP_DESKEY3						//---사용함---//
	char chDESKey4[18];				// _MEM_VAR_APP_DESKEY4						//---사용함---//
	char chRMSPassword[10];			// _MEM_VAR_APP_RMSPASSWORD					//---사용함---//
	char chRMSPhoneNum1[22];		// _MEM_VAR_APP_RMSPHONENO1					//---사용함---//
	char chRMSPhoneNum2[22];		// _MEM_VAR_APP_RMSPHONENO2					//---사용함---//
	char chRMSModemSpeed[8];		// _MEM_VAR_APP_RMSMODEMSPEED				// KSK 2009.12.23 사용 안함
} NETWORKINFO, NEAR* NPNETWORKINFO, FAR* LPNETWORKINFO;

typedef struct tagTransInfo{
	WORD	ClerkTransOnOff;		// Clerk Transaction On/Off					// KSK 2009.12.23 사용 안함
	WORD	TransPowerOnOff;		// Transaction Power Off					// KSK 2009.12.23 사용 안함
	WORD	DeviceExecOff;			// Device Exec Off							// KSK 2009.12.23 사용 안함
	WORD	DeviceSprCnt;			// Spr Error Count							// KSK 2009.12.23 사용 안함

	WORD	DeviceJprCnt;			// Jpr Error Count							// KSK 2009.12.23 사용 안함
	WORD	DeviceMcuCnt;			// Mcu Error Count							// KSK 2009.12.23 사용 안함
	WORD	DeviceCduCnt;			// Cdu Error Count							// KSK 2009.12.23 사용 안함
	WORD	DevicePbmCnt;			// Pbm Error Count							// KSK 2009.12.23 사용 안함
	
	WORD	DeviceSprRetractCnt;	// Spr Retract Count						// KSK 2009.12.23 사용 안함
	WORD	DeviceJprRetractCnt;	// Jpr Retract Count						// KSK 2009.12.23 사용 안함
	WORD	DeviceMcuRetractCnt;	// Mcu Retract Count						// KSK 2009.12.23 사용 안함
	WORD	DeviceCduRetractCnt;	// Cdu Retract Count						// KSK 2009.12.23 사용 안함
	
	WORD	DevicePbmRetractCnt;	// Pbm Retract Count						// KSK 2009.12.23 사용 안함
	WORD	wPinInputMode;			// _MEM_VAR_APP_PININPUTMODE				// KSK 2009.12.23 사용 안함
	WORD	wReversalFlag;			// _MEM_VAR_APP_REVERSALFLAG				//---사용함---//
	WORD	wLocalMode;				// _MEM_VAR_APP_LOCALMODE					// KSK 2009.12.23 사용 안함

	char	ApVersion[16];			// Ap Version								// KSK 2009.12.23 사용 안함
	char	YYYYMMDD[10];			// Date										// KSK 2009.12.23 사용 안함
	char	SerialNo[8];			// Serial No								// KSK 2009.12.23 사용 안함
	char	ProcCount[2];			// Process Count							//---사용함---//
	char	EjrSerialNo[10];		// Elec Journal Serial No					// KSK 2009.12.23 사용 안함
	char	DeviceYYYYMMDD[10];		// Device Error Date						// KSK 2009.12.23 사용 안함
	char	chLastDispAmount[10];	// _MEM_VAR_APP_LASTDISPNOTES				// KSK 2009.12.23 사용 안함
	char	chLocalTranDate[10];	// _MEM_VAR_APP_LOCALTRANDATE				//---사용함---//
	char	chLocalTranTime[8];		// _MEM_VAR_APP_LOCALTRANTIME				//---사용함---//
	char	chRequestedAmount[10];	// _MEM_VAR_APP_REQUESTEDAMT				//---사용함---//
	char	chDispensedAmount[10];	// _MEM_VAR_APP_DISPENSEDAMT				//---사용함---//
	char	chSurchargeAmount[10];	// _MEM_VAR_APP_SURCHARGEAMT				//---사용함---//
	char chSurchargeFlag[2];		// _MEM_VAR_APP_SURCHARGEFLAG				//---사용함---//
	char chRetrievalRefNo[14];		// _MEM_VAR_APP_RETREFNO					//---사용함---//
	char chAmountMixCount[14];		// _MEM_VAR_APP_AMOUNTMIXCOUNT				// KSK 2009.12.23 사용 안함
	char chTotalStartTime[16];		// _MEM_VAR_APP_TOTALSTARTTIME				//---사용함---//
	char chLastAddCashTime[16];		// _MEM_VAR_APP_LASTADDCASHTIME				//---사용함---//
} TRANSINFO, NEAR* NPTRANSINFO, FAR* LPTRANSINFO;

/////////////////////////////////////////////////////////////////////////////
// Total
/////////////////////////////////////////////////////////////////////////////
//---사용함---//
typedef struct tagLogWFormat{		// Withdraw
	WORD	wCount;					// Count
	DWORD	dwAmount;				// Amount
	char	Dummy;					// Dummy
} LOGW;

typedef struct tagWITHTOTALINFO{	// Withdraw Total/Subtotal Info
	LOGW    DATA[16];				// 0:CWD
									// 1:CWC
									// 2-16:CWC2-16
} WITHTOTALINFO, NEAR* NPWITHTOTALINFO, FAR* LPWITHTOTALINFO;

typedef struct tagLogIFormat{		// Inquiry
	WORD	wCount;					// Count
	DWORD	dwAmount;				// Amount
	char	Dummy;					// Dummy
} LOGI;

typedef struct tagINQTOTALINFO{		// Inquiry Total/Subtotal Info
	LOGI    DATA[2];				// 0:INQ
									// 1-16:INQ1-16
} INQTOTALINFO, NEAR* NPINQTOTALINFO, FAR* LPINQTOTALINFO;

typedef struct tagLogTFormat{		// Transfer
	WORD	wCount;					// Count
	DWORD	dwAmount;				// Amount
	char	Dummy;					// Dummy
} LOGT;

typedef struct tagTRANSTOTALINFO{	// Transfer Total/Subtotal Info
	LOGT	DATA[2];				// 0:TFR
									// 1-16:TFR1-16
} TRANSTOTALINFO, NEAR* NPTRANSTOTALINFO, FAR* LPTRANSTOTALINFO;

typedef struct tagLogDFormat{		// Deposit 
	WORD	wCount;					// Count
	DWORD	dwAmount;				// Amount
	char	Dummy;					// Dummy
} LOGD;

typedef struct tagDEPTOTALINFO{		// Deposit Total/Subtotal Info
	LOGD    DATA[2];				// 0:DEP
									// 1:DPC
									// 2-16:DEP2-16
} DEPTOTALINFO, NEAR* NPDEPTOTALINFO, FAR* LPDEPTOTALINFO;

typedef struct tagCHKNVRAMMEM{		// CHECK NVRAM BROKEN
	char    Chk_NVRamBrokenArea[8];				
} CHKNVRAMMEM, NEAR* NPCHKNVRAMMEM, FAR* LPCHKNVRAMMEM;

//---사용함---//
// [#127] KSK 2008.04.21 AP/RMS/MWI/VDM Version 추가
typedef struct tagAPPVERSIONINFO{		// CHECK NVRAM BROKEN
	char	AP_Version[16];
	char	RMS_Version[16];
	char	MWI_Version[16];
	char	VDM_Version[16];
	char	OS_Version[16];
} APPVERSIONINFO, NEAR* NPAPPVERSIONINFO, FAR* LPAPPVERSIONINFO;
// end of [#127]

typedef struct tagAPPAtmInfoCmd{
	WORD wENGMode;					// _MEM_VAR_APP_ENGMODE				//---사용함---//
	WORD wSPNMode;					// _MEM_VAR_APP_SPNMODE				//---사용함---//
	WORD wCHNMode;					// _MEM_VAR_APP_CHNMODE				// KSK 2009.12.23 사용 안함	// [#2186] 2013.04.16 KMK 사용함
	WORD wKORMode;					// _MEM_VAR_APP_KORMODE				// KSK 2009.12.23 사용 안함	// [#2186] 2013.04.16 KMK 사용함
	WORD wJAPMode;					// _MEM_VAR_APP_JAPMODE				// KSK 2009.12.23 사용 안함	// [#2186] 2013.04.16 KMK 사용함
	WORD wFRNMode;					// _MEM_VAR_APP_FRNMODE				//---사용함---// 
	WORD wISO1Flag;					// _MEM_VAR_APP_ISO1_FLAG			//---사용함---// (현재 RMS에서만 사용함 - ATM 기능 구현 안됨)
	WORD wISO2Flag;					// _MEM_VAR_APP_ISO2_FLAG			//---사용함---// (현재 RMS에서만 사용함 - ATM 기능 구현 안됨)
	WORD wISO3Flag;					// _MEM_VAR_APP_ISO3_FLAG			//---사용함---// (현재 RMS에서만 사용함 - ATM 기능 구현 안됨)
	WORD wLowCurrencyCheck;			// _MEM_VAR_APP_LOWCRCYCHECK		//---사용함---// 
	WORD wSurchargeEnable;			// _MEM_VAR_APP_SURCHARGEENABLE
	WORD wTrialDayTotalFlag;		// _MEM_VAR_APP_TRIALDAYTOTAL		// KSK 2009.12.23 사용 안함 (Optional에서 새로 정의해서 사용)
	WORD wFrontJnlIndex;			// _MEM_VAR_APP_FRONTJNLINDEX		// KSK 2009.12.23 사용 안함으로 수정
	WORD wRearJnlIndex;				// _MEM_VAR_APP_REARJNLINDEX		// KSK 2009.12.23 사용 안함
	WORD wJournalCycled;			// _MEM_VAR_APP_JNLONECYCLED		// KSK 2009.12.23 사용 안함
	WORD wVoiceGuidance;			// _MEM_VAR_APP_VOICEGUIDANCE		// KSK 2009.12.23 사용 안함
	WORD wSelectReceipt;			// _MEM_VAR_APP_SELECTRECEIPT		//---사용함---//
	WORD wSurchargeDisplay;			// _MEM_VAR_APP_SURCHARGEDISPLAY	//---사용함---//
	WORD wCancelCode;				// _MEM_VAR_APP_CANCELCODE			// KSK 2009.12.23 사용 안함
	WORD wATMStatus;				// _MEM_VAR_APP_ATMSTATUS			//---사용함---//

	char chATMSerialNo[22];			// _MEM_VAR_APP_ATMSERIALNO			//---사용함---//
	char chMessageType[10];			// _MEM_VAR_APP_MSGTYPE				//---사용함---//
	char chISO1Data[78];			// _MEM_VAR_APP_ISO1DATA			//---사용함---//
	char chISO2Data[42];			// _MEM_VAR_APP_ISO2DATA			//---사용함---//
	char chISO3Data[108];			// _MEM_VAR_APP_ISO3DATA			//---사용함---//
	char chDispenseLimit[8];		// _MEM_VAR_APP_DISPENSELIMIT		//---사용함---//
	char chRoutingID[8];			// _MEM_VAR_APP_ROUTINGID			//---사용함---//
	char chTerminalID[22];			// _MEM_VAR_APP_TERMINALID			//---사용함---//
	// [#268] [NH] KSK 2008.6.9 FASTCASH VALUE 수정
//	char chFastCash[26];			// _MEM_VAR_APP_FASTCASH
	char chFastCashDummy[2];											//---사용함---//
	DWORD dwFastCash[6];												//---사용함---//
	// end of [#268]
	char chOperatorPW[10];			// _MEM_VAR_APP_OPERATORPW			//---사용함---//
	char chServicePW[10];			// _MEM_VAR_APP_SERVICEPW			//---사용함---//
	char chMasterPW[10];			// _MEM_VAR_APP_MASTERPW			//---사용함---//
	char chSurchargeAmount[10];		// _MEM_VAR_APP_SURCHAREAMT			//---사용함---//
	char chSurchargeOwner[22];		// _MEM_VAR_APP_OPSURCHARGEOWNER	//---사용함---//
	char chTrialDayTime[6];			// _MEM_VAR_APP_TRIALDAYTIME		// KSK 2009.12.23 사용 안함으로 수정
	char chTranCardData[100];		// _MEM_VAR_APP_TRANSACTION_TRANCARD	// KSK 2009.12.23 사용 안함
	char chSequenceNo[6];			// _MEM_VAR_APP_SERIALNO			//---사용함---//
	char chOPSurchargeAmount[8];	// _MEM_VAR_APP_OPSURCHAREAMT		// KSK 2009.12.23 사용 안함
	char chReceiptHeader1[42];		// _MEM_VAR_APP_RECEIPT_HEADER1		//---사용함---//
	char chReceiptHeader2[42];		// _MEM_VAR_APP_RECEIPT_HEADER2		//---사용함---//
	char chReceiptHeader3[42];		// _MEM_VAR_APP_RECEIPT_HEADER3		//---사용함---//
	char chReceiptHeader4[42];		// _MEM_VAR_APP_RECEIPT_HEADER4		// KSK 2009.12.23 사용 안함
	char chReceiptAddress1[42];		// _MEM_VAR_APP_RECEIPT_ADDRESS1	//---사용함---//
	char chReceiptAddress2[42];		// _MEM_VAR_APP_RECEIPT_ADDRESS2	//---사용함---//
	char chReceiptAddress3[42];		// _MEM_VAR_APP_RECEIPT_ADDRESS3	//---사용함---//
	char chReceiptPhoneNo[42];		// _MEM_VAR_APP_RECEIPT_PHONENO		//---사용함---//
	char chErrorCode[8];			// _MEM_VAR_APP_ERRORCODE			// KSK 2009.12.23 사용 안함
} APPATMINFO, NEAR* NPAPPATMINFO, FAR* LPAPPATMINFO;

typedef struct tagAPPErrTrans{
	WORD wTransResult;				// _MEM_VAR_APP_TRANSRESULT			//---사용함---//

	char chTerminalNumber[16];		// _MEM_VAR_APP_TERMINALNUMBER		// KSK 2009.12.23 사용 안함
	char chTransNumber[6];			// _MEM_VAR_APP_TRANSNUMBER			// KSK 2009.12.23 사용 안함
	char chTransType[4];			// _MEM_VAR_APP_TRANSTYPE			//---사용함---//
	char chFromAccount[4];			// _MEM_VAR_APP_TRANSFROMACCOUNT	//---사용함---//
	char chToAccount[4];			// _MEM_VAR_APP_TRANSTOACCOUNT		//---사용함---//
	char chBankCode[6];				// _MEM_VAR_APP_TRANSBANKCODE		//---사용함---//
	char chBankAccount[18];			// _MEM_VAR_APP_TRANSBANKACCOUNT	//---사용함---//
	char chHostDate[10];				// _MEM_VAR_APP_TRANSHOSTDATE	//---사용함---//
	char chHostTime[8];				// _MEM_VAR_APP_TRANSHOSTTIME		//---사용함---//
	char chAvailBalance[10];			// _MEM_VAR_APP_TRANSAVAILBALANCE	//---사용함---//
	char chRetrievalNum[14];		// _MEM_VAR_APP_TRANSRETRIEVALNUM		//---사용함---//
	char chTraceAuditNum[8];		// _MEM_VAR_APP_TRANSTRACEAUDITNUM		//---사용함---//
	char chHostNetID[4];			// _MEM_VAR_APP_TRANSHOSTNETID			//---사용함---//
	char chSettleDate[10];			// _MEM_VAR_APP_TRANSSETTLEDATE			//---사용함---//
	char chSurcharge[6];			// _MEM_VAR_APP_TRANSSURCHARGE		// KSK 2009.12.23 사용 안함
	char chRequestedAmount[10];		// _MEM_VAR_APP_TRANSREQAMOUNT			//---사용함---//
	char chDispensedAmount[10];		// _MEM_VAR_APP_TRANSDISPAMOUNT			//---사용함---//
	char chRemainedAmount[12];		// _MEM_VAR_APP_TRANSREMAINAMOUNT		//---사용함---//
	char chProCount[4];				// _MEM_VAR_APP_TRANSPROCCOUNT			//---사용함---// TRANS 구조체와 중복됨 (추후 한개로 통일 필요)
//	char chErrorCode[8];			// _MEM_VAR_APP_TRANSERRORCODE		// KSK 2009.12.23 사용 안함으로 수정
//	char chCardData[18];			// _MEM_VAR_APP_TRANSCARDDATA			//---사용함---//
	char chCardData[26];			// _MEM_VAR_APP_TRANSCARDDATA			//---사용함---// KSK 2010CardData 19자리 처리를 위해 수정
	char chNonCashValue[6];			// _MEM_VAR_APP_TRANSNONCASHVALUE		//---사용함---//
	char chNonCashType[14];			// _MEM_VAR_APP_TRANSNONCASHTYPE		//---사용함---//
	char chOtherMessage[78];		// _MEM_VAR_APP_TRANSOTHERMSG			//---사용안하고 확장하여 사용하도록 변경함
	char chTrack1Data[80];			// _MEM_VAR_APP_TRANSTRACK1				//---사용함---//
	char chTrack2Data[42];			// _MEM_VAR_APP_TRANSTRACK2				//---사용함---//
	char chTrack3Data[108];			// _MEM_VAR_APP_TRANSTRACK3				//---사용함---//
} APPERRTRANS, NEAR* NPAPPERRTRANS, FAR* LPAPPERRTRANS;

typedef struct tagAPPHostCmd{
	WORD wHC02BinList;				// _MEM_VAR_APP_HC02BINList			// KSK 2009.12.23 사용 안함
	WORD wHC13MACKey;				// _MEM_VAR_APP_HC13MACKey			// KSK 2009.12.23 사용 안함
	WORD wHC14CashPosition;			// _MEM_VAR_APP_HC14CashPosition	// KSK 2009.12.23 사용 안함
	WORD wHC15DetailStatus;			// _MEM_VAR_APP_HC15DetailStatus	//---사용함---//
	WORD wHC16HostTotal;			// _MEM_VAR_APP_HC16HostTotal		// KSK 2009.12.23 사용 안함
	WORD wHC17DataUpdate;			// _MEM_VAR_APP_HC17DataUpdate		// KSK 2009.12.23 사용 안함
	WORD wHC18ExtBINList;			// _MEM_VAR_APP_HC18ExtBINList		// KSK 2009.12.23 사용 안함
	WORD wHC19CouponTotal;			// _MEM_VAR_APP_HC19CouponTotal		// KSK 2009.12.23 사용 안함
	WORD wHC20CouponData;			// _MEM_VAR_APP_HC20CouponData		// KSK 2009.12.23 사용 안함
	WORD wHC21PrintGraphic;			// _MEM_VAR_APP_HC21PrintGraphic	// KSK 2009.12.23 사용 안함
	WORD wHC22ScreenGraphic;		// _MEM_VAR_APP_HC22ScreenGraphic	// KSK 2009.12.23 사용 안함
	WORD wHC23ServiceTotals;		// _MEM_VAR_APP_HC23ServiceTotals	// KSK 2009.12.23 사용 안함
	WORD wHC24CouponTotals;			// _MEM_VAR_APP_HC24CouponTotals	// KSK 2009.12.23 사용 안함
	WORD wHC25ForceConfiguration;	// _MEM_VAR_APP_HC25ForceConfiguration	// KSK 2009.12.23 사용 안함
	WORD wHC26UpdateConfiguration;	// _MEM_VAR_APP_HC26UpdateConfiguration	// KSK 2009.12.23 사용 안함
	WORD wHC27UpdateConfigData;		// _MEM_VAR_APP_HC27UpdateConfigData	// KSK 2009.12.23 사용 안함
	WORD wHC28UpdateExtBINList;		// _MEM_VAR_APP_HC28UpdateExtBINList	// KSK 2009.12.23 사용 안함
} APPHOSTCMD, NEAR* NPAPPHOSTCMD, FAR* LPAPPHOSTCMD;

//---사용함---//
typedef struct tagBINListFormat{
	char	chBINData[10];
} BINLIST;

//---사용함---//
typedef struct tagAPPBINInfo{
	WORD wSetBINList;				// _MEM_VAR_APP_SETBINLIST
	WORD wBINFound;					// _MEM_VAR_APP_BINFOUND
	BINLIST chBINList[BINLIST_SIZE];// _MEM_VAR_APP_BINLIST (0 ~ 19)
} APPBININFO, NEAR* NPAPPBININFO, FAR* LPAPPBININFO;

//---사용함---//	// Local Mode에서만 사용함
typedef struct tagAPPSIMInfo{
	char chLocalHealthMsg[100];		// _MEM_VAR_APP_LC_HEALTH
	char chLocalConfigMsg[100];		// _MEM_VAR_APP_LC_CONFIG
	char chLocalTotalMsg[100];		// _MEM_VAR_APP_LC_TOTAL
	char chLocalWithDrawalMsg[200];	// _MEM_VAR_APP_LC_WITHDRAWAL
	char chLocalInquiryMsg[200];	// _MEM_VAR_APP_LC_INQUIRY
	char chLocalTransferMsg[200];	// _MEM_VAR_APP_LC_TRANSFER
	char chLocalDepositMsg[50];		// _MEM_VAR_APP_LC_DEPOSIT
	char chLocalReversalMsg[50];	// _MEM_VAR_APP_LC_REVERSAL
} APPSIMINFO, NEAR* NPAPPSIMINFO, FAR* LPAPPSIMINFO;

//---사용함---//
// [#519] [NH] KSK 2009.3.10
typedef	struct tagAPPExtendJNLInfo 
{
	char chCalcReqMXDollar[12];
	char chCalcDispMXDollar[12];
	char chTransDispensedCount[16];
	char chTransRejectedCount[16];
	char chLoyaltyFee[10];			// [#631] MX KSK 2010.03.29 LOYALTY, LINE USAGE FEE, IVA TAX, BANK NAME
	char chLineUsageFee[10];		// [#631] MX KSK 2010.03.29 LOYALTY, LINE USAGE FEE, IVA TAX, BANK NAME
	char chIVATAX[10];				// [#631] MX KSK 2010.03.29 LOYALTY, LINE USAGE FEE, IVA TAX, BANK NAME
	char chBankName[24];			// [#631] MX KSK 2010.03.29 LOYALTY, LINE USAGE FEE, IVA TAX, BANK NAME
	char chExtendOtherMessage[200];	// [#631] MX KSK 2010.03.30 Other Message size 변경 (78 -> 200)
	char chReturnedAccountNumber[4];// [#2015] MX KSK 2012.02.02

	WORD wTransactionCountFlag;			// [#2137] MX PCS 2012.07.26 USD 방출시 First 와 Second를 구분하기 위한 Flag (Power Off 처리를 위해)
	WORD wCustomerType;					// [#2137] MX PCS 2012.07.24	
	char chExchangeFee[10];				// [#2137] MX PCS 2012.06.28 INTERNATIONAL TRANSFER FEE for "qbE"
// 	char chExchangeTotalAmount[20];		// [#2137] MX PCS 2012.07.23 Total Mount From Host.
	char chTotalFee[20];				// [#2142] MX KMK 2012.08.16 Total Fee From Host.
	char chExchangeRate[20];			// [#2137] MX PCS 2012.07.23 Exchange Rate From Host.
	char chReversalExchangeFee[20];		// [#2147] MX KSK 2012.08.18

} APPEXTENDJNLINFO, NEAR* NPAPPEXTENDJNLINFO, FAR *LPAPPEXTENDJNLINFO;
// end of [#519]

typedef struct tagNVRAM2{
	SHAREDATA		ShareData;			// KSK 2009.12.23 사용 안함			// 2641
	DEVICEINFO		Device;				//---사용함---//					//  200
	NETWORKINFO		Network;			//---사용함---//
	TRANSINFO		Transaction;		//---사용함---//
	WITHTOTALINFO	WithTotal;			//---사용함---//
	INQTOTALINFO	InqTotal;			//---사용함---//
	TRANSTOTALINFO	TransTotal;			//---사용함---//
	DEPTOTALINFO	DepTotal;			// KSK 2009.12.23 사용 안함
	CHKNVRAMMEM		ChkNVRam;		// CHECK NVRAM BROKEN
	APPVERSIONINFO	APVersionInfo;	// [#127] KSK 2008.04.21
} NVRAM2, NEAR* NPNVRAM2_ADDR, FAR* LPNVRAM2_ADDR;

typedef struct tagNVRAM3{
	APPATMINFO			APPATMInfo;		//---사용함---//
	APPERRTRANS			APPErrTrans;	//---사용함---//
	APPHOSTCMD			APPHostCmd;		//---사용함---//
	APPBININFO			APPBINInfo;		//---사용함---//
	APPSIMINFO			APPSIMInfo;		// Local Mode에서만 사용함
	APPEXTENDJNLINFO	APPExtendJNLInfo;	// [#519] [NH] KSK 2009.3.10
} NVRAM3, NEAR* NPNVRAM3_ADDR, FAR* LPNVRAM3_ADDR;

//---사용함---//
typedef	struct tagCurrentErrInfo {
	char ProcCount[ERRORPROCCOUNT_SIZE];		// Procedure Count
	char ErrorCode[ERRORCODE_SIZE];				// Error Code
	char ErrorString[ERRORSTRING_SIZE];			// Error String
}CURERRINFO, NEAR* NPCURERRINFO, FAR* LPCURERRINFO;

//---사용함---//
typedef	struct tagErrSumInfo{
	DWORD wDevice_id;
	char  chErrorCode[ERRORCODE_SIZE];
	WORD  wStackCount;
} ERRSUMINFO, NEAR* NPERRSUMINFO, FAR* LPERRSUMINFO;
// end of [#419]

//---사용함---//
// [#419] [NH] KSK 2008.9.10
typedef struct tagNVRAM4{
	char		wErrVersion[8];					// V010304
	WORD		wCurErrCount;
	CURERRINFO	CurErrStack[CURERRSTACK_MAX_SIZE];
	char		cErrSumDate[ERRORDATE_SIZE];
	char		cErrSumTime[ERRORTIME_SIZE];
	WORD		wErrSumTotalCount;
	ERRSUMINFO	ErrSumStack[ERRORSTACK_MAX_SIZE];

} NVRAM4, NEAR* NPNVRAM4_ADDR, FAR* LPNVRAM4_ADDR;
// end of [#419]

//---사용함---//
// NVRAM 6 영역을 사용하도록 structure 추가
// 광고 IMAGE
typedef struct tagAdvInfo{
	char	Adv_EndisFlag[6];
	char	Adv_Title1[50];
	char	Coupon_EndisFlag[6];
	char	Coupon1_Text1[50];
	char	Coupon1_Text2[50];
	char	Adv_Title2[50];
	char	Coupon2_Text1[50];
	char	Coupon2_Text2[50];
	char	Adv_Title3[50];
	char	Coupon3_Text1[50];
	char	Coupon3_Text2[50];
	char	Adv_Title4[50];
	char	Coupon4_Text1[50];
	char	Coupon4_Text2[50];
	char	Adv_Title5[50];
	char	Coupon5_Text1[50];
	char	Coupon5_Text2[50];
	char	Adv_Title6[50];
	char	Coupon6_Text1[50];
	char	Coupon6_Text2[50];
	WORD	Adv_GuideDisp_Time;
	WORD	TranAdv_Disp_Time;
	char	TranAdv_EndisFlag[6];
	WORD	ChangeBackFlag;
	WORD	ChangeDefaultBack;
	char	ChangeBackTypeFlag[6];
} ADVINFO, NEAR* NPADVINFO, FAR* LPADVINFO;

//---사용함---//
typedef struct tagTCPIPInfo{
	char	ATM_IpAddr[16];
	char	ATM_Subnet[16];
	char	ATM_Gateway[16];
	char	ATM_DNS[16];
	WORD	ATM_DHCP;
	WORD	IsHostUseURL;
	char	Host1Name[256];
	WORD	Host1PortNo;
	char	Host2Name[256];
	WORD	Host2PortNo;
	WORD	NetworkType;
	WORD	SSL_Enable;
	WORD	TCPIP_Type;
	WORD	CRC_Enable;
	WORD	IsRMSUseURL;
	char	RMSName[256];				// RMS Destination IP or URL
	WORD	RMSPortNo;					// RMS Destination Port No
	WORD	RMSSvrPortNo;				// ATM Listen Port No
	WORD	RMSConnectMode;				// Active or Passive
	// RMS SEND INTERVAL 추가
	WORD	RMSSendInterval;
//	WORD	SSLVersion;					// KSK 2010.06.16 WINCE5.0 적용이 불가하여 위치 수정함 (OPT2영역으로)
} TCPIPINFO, NEAR* NPTCPIPINFO, FAR* LPTCPIPINFO;

//---사용함---//
// [#4] NH PSC 2008.03.10 NVRam이 정리 되기전까지 기타 옵션은 Optional setting section에 추가
typedef struct tabOptionalSetting{
	WORD	dwReversalRetryCount;		// _MEM_VAR_OPT_REVERSAL_RETRY
	WORD	NetflowState;				// _MEM_VAR_OPT_NETFLOW_STATE
	WORD	NetflowTranCode;			// _MEM_VAR_OPT_NETFLOW_TRANCODE
	WORD	MOD10_Enable;				// _MEM_VAR_OPT_MOD10_ENABLE
	char	chDESKey5[18];				// _MEM_VAR_OPT_APP_DESKEY5
	char	chDESKey6[18];				// _MEM_VAR_OPT_APP_DESKEY6
	char	chCstTotalStartTime[16];	// _MEM_VAR_OPT_CASSETTETOTAL_STARTTIME
	char	chIssuerFee[10];			// _MEM_VAR_OPT_APP_ISSUERFEE :'k' field추가 PSC 2008.04.04
	WORD	dwResonForReversal;			// _MEM_VAR_OPT_REASONFORREVERSAL(Only use standard#3)	[#12] NH PSC 2008.03.24
//	[#59] NH KGS 2008.03.27 NVRAM for Accounts, NUA, Welcome Message
	WORD	dwAccountChecking;			
	WORD	dwAccountSavings;	
	WORD	dwAccountCreditCard;
	char	chNetworkUserAddress[16];
	// [#192] UK AIREAT 2008.5.16
	// Length changed (50 -> 30)
	char	chWelcomeMessage1[30];
	char	chWelcomeMessage2[30];
	char	chWelcomeMessage3[30];
	char	chWelcomeMessage4[30];		// 사용하지 않음.
	char	chWelcomeDummy[30];			// 사용하지 않음.
	// end of [#192]
//	end of [#59]
	WORD	bPreBalance;				// [#94] US KGS 2008.04.10 Pre Balance Menu
	// [#112] US KGS 2008.04.15 Percent Surcharge Menu
	WORD	bPercentSurcharge;
	WORD	dPerSurchargeManner;
	WORD	dSurchargePercent;
	// end of [#112]

	WORD	bContinueTrans;				// [#239] MX KSK 2008.5.27 After PreBalance, continue transaction
	WORD	dwWithdrawalSurcharge;		// [#239] MX KSK 2008.5.27 Cash Withdrawal Surcharge
	WORD	dwBalanceSurcharge;			// [#239] MX KSK 2008.5.27 Balance Surcharge
	WORD	dwPinChangeSurcharge;		// [#239] MX KSK 2008.5.27 PIN Chagne Surcharge
	WORD	dwDefaultLanguage;			// [#239] MX KSK 2008.5.27 Default Language

	char	chStoreMessage1[40];		// [#337] AU AIREAT 2008.06.25 - Add Store Msg1
	char	chStoreMessage2[40];		// [#337] AU AIREAT 2008.06.25 - Add Store Msg2
	char	chStoreMessage3[40];		// [#337] AU AIREAT 2008.06.25 - Add Store Msg3
	char	chStoreMessage4[40];		// [#337] AU AIREAT 2008.06.25 - Add Store Msg4

	char	chMarketingMessage1[40];	// [#337] AU AIREAT 2008.06.25 - Add Marketing Msg1
	char	chMarketingMessage2[40];	// [#337] AU AIREAT 2008.06.25 - Add Marketing Msg2
	char	chMarketingMessage3[40];	// [#337] AU AIREAT 2008.06.25 - Add Marketing Msg3
	char	chMarketingMessage4[40];	// [#337] AU AIREAT 2008.06.25 - Add Marketing Msg4

	char	chExitMessage1[30];			// [#337] AU AIREAT 2008.06.25 - Add Exit Msg1
	char	chExitMessage2[30];			// [#337] AU AIREAT 2008.06.25 - Add Exit Msg2
	char	chExitMessage3[30];			// [#337] AU AIREAT 2008.06.25 - Add Exit Msg3

	char	chProcessorMessage1[40];	// [#337] AU AIREAT 2008.06.25 - Add Processor Msg1
	char	chProcessorMessage2[40];	// [#337] AU AIREAT 2008.06.25 - Add Processor Msg2
	char	chProcessorMessage3[40];	// [#337] AU AIREAT 2008.06.25 - Add Processor Msg3
	char	chProcessorMessage4[40];	// [#337] AU AIREAT 2008.06.25 - Add Processor Msg4

	long	dwInquirySurchargeAmount;	// [#337] AU AIREAT 2008.06.25 - Add Inquiry Surcharge Filed
	
	WORD	bCheckCashing;						// [#366] [NH] KSK 2008.7.15 - Add Check Cashing Option
	WORD	bPINOption;							// [#366] [NH] KSK 2008.7.15 - Add Check Cashing Option
	char	chCheckCashingCustomerNumber[16];	// [#366] [NH] KSK 2008.7.15 - Add Customer Number for Check Cashing
	WORD	bCheckCashingAvailable;				// [#378] [NH] KSK 2008.7.18 - Add Check Cashing Available
	char	chCertificatinKey[32];				// [#396] [US] KSK 2008.8.6 - Add Certification Key for Check Cashing
	WORD	bEMVAvailable;						// [#397] [NH] psc 2008.08.01 - EMV Available option
	WORD	bEMVEnable;							// [#397] [NH] psc 2008.08.01 - EMV En/Disable option
	WORD	nSurchargeWarningLocation;
	WORD	bEMVTransactionOption;				// [#397] [NH] psc 2008.08.01 - EMV transaction option(MS/IC first transaction)
	WORD	dCheckCashingType;					// [#410] [US] KSK 2008.8.29 - Add Check Cashing Type

	WORD	dwAutoDayTotal;						// [#416] [NZ] AIREAT 2008.09.08 - Auto Day Total En/Disable
	WORD	dwAutoDayTotalType;					// [#416] [NZ] AIREAT 2008.09.08 - Auto Day Total Type (Trial or Real)
	WORD	dwAutoDayTotalTime_Year;			// [#416] [NZ] AIREAT 2008.09.08 - Auto Day Total Time Year(0000-2359)
	WORD	dwAutoDayTotalTime_Month;			// [#416] [NZ] AIREAT 2008.09.08 - Auto Day Total Time Month(0000-2359)
	WORD	dwAutoDayTotalTime_Day;				// [#416] [NZ] AIREAT 2008.09.08 - Auto Day Total Time Day(0000-2359)
	WORD	dwAutoDayTotalTime_Hour;			// [#416] [NZ] AIREAT 2008.09.08 - Auto Day Total Time Hour(0000-2359)
	WORD	dwAutoDayTotalTime_Min;				// [#416] [NZ] AIREAT 2008.09.08 - Auto Day Total Time Min(0000-2359)
	char	chDayTotalLastDate[8];				// [#416] [NZ] AIREAT 2008.09.08 - Day Total Last Date YYYYMMDD - last successed date
} OPTINALSETTING, NEAR* NPOPTIONALSETTING, FAR* LPOPTIONALSETTING;
// end if [#4]

//---사용함---//
typedef	struct tagNVRAM6 {
	ADVINFO			APPAdvInfo;
	TCPIPINFO		APPTCPIPInfo;
	OPTINALSETTING	APPOptionalSetting;	// [#4] NH PSC 2008.03.10 Optional setting값 추가.
} NVRAM6, NEAR* NPNVRAM6_ADDR, FAR* LPNVRAM6_ADDR;

//---사용함---//
typedef struct tagEmvInfo{
	char	EMV_APPLICATION_ID[40];		// [#68] UK HWANG 2008.04.02 EMV Level2
} EMVINFO, NEAR* NPEMVINFO, FAR* LPEMVINFO;

//---사용함---//
// [#68] UK HWANG 2008.04.02 EMV Level2 ; [#397] NH PSC 2008.12.15 EMV 거래시 Reversal 전문에 Tag 추가
typedef struct tagEmvReversal{
	char CRYPTOGRAM_INFO_DATA_TLV[8];					//    8 bytes
	char TERMINAL_VERIFY_RESULT_TLV[16];				//   24 bytes
	char APP_CRYPTOGRAM_TLV[22];						//   46 bytes
	char AMOUNT_AUTHORIZED_TLV[18];						//   64 bytes					
	char AMOUNT_OTHER_TLV[18];							//   82 bytes
	char APPLICATION_ID_TLV[38];						//  120 bytes
	char APP_INTERCHANGE_PRO_TLV[10];					//  130 bytes
	char APP_TRAN_COUNTER_TLV[10];						//  140
	char TERMINAL_COUNTRY_CODE_TLV[10];					//  150
	char TRANSACTION_CURRENCY_CODE_TLV[10];				//  160
	char TRANSACTION_DATE_TLV[12];						//  172   
	char TRANSACTION_TYPE_TLV[8];						//  180
	char RANDOM_NUMBER_TLV[14];							//  194
	char ISSUER_APP_DATA_TLV[70];						//  264
	char IFD_SERIAL_NUMBER_TLV[22];						//  286
	char ISSUER_SCRIPT_ID_TLV[10];						//  296
	char ISSUER_SCRIPT_RESULTS_TLV[262];				//  558
	char TRANSACTION_STATUS_INFO_TLV[8];				//  566
	// KSK 2009.8.31 캐나다 Interac 최소 사양으로 인해 Field 추가
	char APP_EXPIREDDATE_TLV[8];						//  574
	char APP_ACCOUNT_NUMBER[64];						//  638
	char APP_ACCOUNT_NUMBER_SEQ_NUMBER[16];				//  654
	char POS_ENTRY_MODE[8];								//  662
	char TERMINAL_CAPABILITIES[16];						//  678
	char TERMINAL_TYPE[8];								//  686
	char TRACK2_EQU_DATA[256];							//  942
	char TRANSACTION_SEQ_COUNTER[16];					//  958
	// end of KSK 2009.8.31
	char TRAN_AC_REQ_VALUE[32];		// [#2115] MX KSK 2012.01.28										//  990
	char EMV_APPLICATION_NAME[40];	// [#2252] US Justin 2014.02.04 BUG Fix .. Long AID and Name		// 1030
} EMV_REVERSAL, NEAR* NPEMV_REVERSAL, FAR* LPEMV_REVERSAL;
// end of [#68]

// [#2342] US Justin 2015.05.07 AID Selection
typedef struct tagEmvConfig{
	char chEnabledAID[1024];							// _MEM_VAR_EMV_CONFIG_ENABLEDAID
														// 16(longest AID)+1(separator)=17, 17*60=1020
} EMV_CONFIG, NEAR* NPEMV_CONFIG, FAR* LPEMV_CONFIG;
// end of [#2342]

typedef	struct tagNVRAM7 {
	EMVINFO			EMVInfo;		//---사용함---//													//   40 bytes	//   40 bytes
	EMV_REVERSAL	EMVREVERSAL;	// [#68] UK HWANG 2008.04.02 EMV Level2		//---사용함---//		// 1030 bytes	// 1070 bytes 
	EMV_CONFIG		EMVCONFIG;		// [#2342] US Justin 2015.05.07 AID Seleceiont						// 1024 bytes	// 2094 bytes
} NVRAM7, NEAR* NPNVRAM7_ADDR, FAR* LPNVRAM7_ADDR;

//---사용함---//
// [#434] AU AIREAT 2008.10.14
// NVRAM 8번 영역으로 OPTIONAL SETTING 확장.
typedef struct tabOptionalSetting2{
	char	chSurchargeOwnerContact1[26];		// [#434] [AU] AIREAT 2008.10.14 - contact line 1
	char	chSurchargeOwnerContact2[26];		// [#434] [AU] AIREAT 2008.10.14 - contact line 2
	char	chSurchargeOwnerContact3[26];		// [#434] [AU] AIREAT 2008.10.14 - contact line 3											// 78 bytes

	char	chRMSModemInitialString[50];		// [#468] NH AIREAT 2008.12.19
	char	chMachineKind[12];					// [#476] [NH] KSK 2008.1.5 MACHINE KIND 추가 (5050CE, 5000CE, 5300CE, 1800CE)
	WORD	wMultiCurrencyFlag;					// [#514] [MX] KSK 2009.3.4
	char	chExchangeRate[12];					// [#514] [MX] KSK 2009.3.4
	char	chCurrencyID[4][6];					// [#514] [MX] KSK 2009.3.11 Cassette별 CurrencyID를 set한다.(현재는 CST1만 사용한다)		// 178 (78+100) bytes
											
	WORD	wPreDialFlag;						// [#525] US KSK 2009.05.21 PreDial Flag
	WORD	wPreDialOption;						// [#525] US KSK 2009.05.21 PreDial Location Option	After Card(0) / Pin(1) / Account(2)
	WORD	wNeedMoreTime;						// [#528] AU AIREAT 2009.06.02 Need More Time option 추가.
	WORD	wAwardCouponFlag[6];				// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
	WORD	wTerminalStatusFieldEnable;			// [#558] KSK 2009.08.17
	WORD	wReversalAtHostError;				// [#558] KSK 2009.08.17
	WORD	wStandard1OptionAvailable;			// [#558] KSK 2009.08.17
	WORD	wPaperLowSensorEnable;				// [#596] SOOK 2009.12.11 SPR PAPER LOW SENSOR EN/DISABLE 기능 추가 0: Enable, 1:Disable
	WORD	wEPPFlickerOption;					// [#604] NH KSK 2010.01.05
	WORD	wCSTSoundOption;					// [#604] NH KSK 2010.01.05
	WORD	wEmvLatchEnable;					// [#605] NH KSK 2010.01.06			0: Enable, 1:Disable
	WORD	wReasonForReversal;					// [#606] NH KSK 2010.01.06
	WORD	wScheduledJournalEnable;			// [#613] AU_C KSK 2010.01.18
	WORD	wScheduledJournalOption;			// [#613] AU_C KSK 2010.01.18	0: Count, 1: Monthly, 2: Daily
	WORD	wScheduledJournalCount;				// [#613] AU_C KSK 2010.01.18
	WORD	wScheduledJournal_Year;				// [#613] AU_C KSK 2010.01.18
	WORD	wScheduledJournal_Month;			// [#613] AU_C KSK 2010.01.18
	WORD	wScheduledJournal_Day;				// [#613] AU_C KSK 2010.01.18
	WORD	wScheduledJournal_Hour;				// [#613] AU_C KSK 2010.01.18
	WORD	wConfigurationOption;				// [#615] AU_C KSK 2010.01.21		0: Enable, 1:Disable
	WORD	wScheduledJournal_NextDay;			// [#613] AU_C KSK 2010.02.01
	WORD	wScheduledJournalAvailable;			// KSK 2010.02.02 Available Field 추가														// 232 (178+54)	
												
	char	chADBrandName[12];					// [#616] AU_C SOOK 2010.02.05 AD 사양 추가 KSK Bug Fix
	WORD	wTransactionAdver1Enable;			// [#616] AU_C SOOK 2010.02.05 AD 사양 추가 
	WORD	wTransactionAdver2Enable;			// [#616] AU_C SOOK 2010.02.05 AD 사양 추가 
	WORD	wBankNameFeePrint;					// [#631] [MX] 2010.03.17 JERRY _MEM_VAR_APP_BALANCE_NAME_FEE	//---사용함---// 
	WORD	wNoticeServiceEnable;
	char	chNoticeServiceTitle[30];
	char	chNoticeServiceMsg[3][40];																										// 402 (232+170)
												
	WORD	wTouchVibrationEnable;
	WORD	SSLVersion;							// KSK 2010.06.16 WINCE5.0 적용이 불가하여 위치 수정함 (OPT2영역으로)
	WORD	wMexico_DPSelection;				// WINCE5.0과 맞추기 위해
	WORD	wCountrySelection;					// [#2042] AU KSK 2011.04.02
	WORD	wCardReaderType;					// [#2073] NH KSK 2011.06.24
	WORD	wESUEnable;							// [#2074] NH KSK 2011.06.25 AntiSkimming = ESU
	WORD	wRKTAvailable;						// [#2075] NH KSK 2011.06.27
	WORD	wRKTEnable;							// [#2075] NH KSK 2011.06.27
	WORD	wRKTRandomNumberEnable;				// [#2075] NH KSK 2011.06.27
	WORD	wEJUPLOADEnable;					// [#2076] NH KSK 2011.06.28
	WORD	wESUShutterEnable;					// [#2074] NH KSK 2011.07.09
	WORD	wESUTimeThreshold;					// [#2074] NH KSK 2011.07.09
	WORD	wESURecoveryTime;					// [#2085] NH KSK 2011.07.22
	WORD	wSurchargeIgnore;					// [#2092] US PCS 2011.10.24 US RBS Host Surcharge ignoring when AP`s configuring with host.
	WORD	SSL_Certify_Enable;					// [#2093] NH PCS 2011.10.24 "SSL Cert. Download From USB"
	// [#2012] NH KJW 2011.01.25
	WORD	dwLotteryAvailable;
	//char	chLotteryCertificatinKey[32];
	char	chCALocalLoanCertificatinKey[32];	// [#2389] CA Justin 2016.01.18 Use for Canadian Rapid Cash Activation
	WORD	dwLotteryEnable;
	WORD	dwLotterySSLEnable;
	WORD	dwLotterySSLVersion;
	WORD	dwLotteryIsHostURL;
	char	chLotteryHost1Name[256];	
	WORD	dwLotteryHost1Port;
	char	chLotteryHost2Name[256];	
	WORD	dwLotteryHost2Port;			
	char	chLotteryHost1PhoneNo[22];	// [#2051] NH KJW 2011.04.22
	char	chLotteryHost2PhoneNo[22];	// [#2051] NH KJW 2011.04.22	// 1136 bytes														// 1034 (402+632) bytes
	// end of [#2012]

	// [#2150] US Justin 2012.09.27 Add Dynamic Flow
	WORD	dwDynamicFlowEnable;
	char	chDynamicFlowPartnerID[8];
	WORD	dwDynamicFlowServicePinChange;
	WORD	dwDynamicFlowServiceMPT;
	WORD	dwDynamicFlowServiceDCC;
	char	chDCCDisclaimer1[40];
	char	chDCCDisclaimer2[40];
	char	chDCCDisclaimer3[40];
	// End of [#2150]
	WORD	dwSupervisorLanguage;	// [#2166] NH KSK 2012.11.13 Add Supervisor Language Option
	WORD	dwReceiptOnScreen;		// [#2183] NH Justin 2013.04.05 Receipt Option															// 1172 (1032+140) bytes
												

	// [#2185] US Justin 2013.04.10 Dual Host DCC
	WORD	dwDualHostDCCEnable;
	WORD	dwDualHostDCCSSLEnable;
	WORD	dwDualHostDCCSSLVersion;
	WORD	dwDualHostDCCIsHostURL;
	char	chDualHostDCCHost1Name[256];
	WORD	dwDualHostDCCHost1Port;
	char	chDualHostDCCHost2Name[256];
	WORD	dwDualHostDCCHost2Port;																											// 1696 (1172+524) bytes
	// End of [#2185]

	WORD	dwDCCCustomOption;		// [#2187] NH Justin 2013.04.18 DCC Custom Option
	WORD	dwDualDCCTran;			// [#2185] US Justin 2013.05.14 Dual Host DCC

	// [#2202] US Justin 2013.06.20 Add extra DCC Disclaimer
	char	chDCCDisclaimer4[40];
	char	chDCCDisclaimer5[40];
	char	chDCCDisclaimer6[40];
	// End of [ #2202]

	// [#2205] US KSK 2013.06.28
	WORD	dwHaloLedMode_InService;
	WORD	dwHaloLedColor_InService;
	WORD	dwHaloLedMode_Transaction;
	WORD	dwHaloLedColor_Transaction;
	WORD	dwHaloLedMode_Dispensing;
	WORD	dwHaloLedColor_Dispensing;
	WORD	dwHaloLedMode_OutofService;
	WORD	dwHaloLedColor_OutofService;
	WORD	dwHaloLedDefaultFlag;
	// end of [#2205]

	WORD	dwEMVContinueFallBack;		// [#2208] US Justin 2013.07.08 Add FallContinue Option (US Only)
	WORD	dwExchangeFee_Percent;		// [#2210] US Justin 2013.07.12 Exchange Fee format USD/Percent (MX ONLY)
	WORD	dwEMVKernelVersion;			// [#2226] US Justin 2013.10.22 Add Kernel Version
	char	chEMVAIDEnDisable[60];		// [#2226] US Justin 2013.10.23 AID En/Disable
	WORD	dwDynamicFlow1stCall;		// [#2242] US Justin 2013.12.17 DF 1st call option														// 1910 (1696+214) bytes

	
	WORD	dwRMSTimeout_Connect_TcpIp;	// [#2234] NH KMK 2013.11.21 MoniView Timeout
	WORD	dwRMSTimeout_Receive_TcpIp;	// [#2234] NH KMK 2013.11.21 MoniView Timeout
	
	WORD	dwCDUMNegativeCountEnable;	// [#2241] AU KMK 2013.12.16 CDU-M 논리매수 차감 Option

	WORD	dwDigitalReceiptOption;		// _MEM_VAR_OPT2_DIGITAL_RECEIPT_OPTION [#2263] CA Justin 2014.0505 Digital Receipt Option

	WORD	dwInitialCSTCntfromAP[4];	// [#2270] AU KSK 2014.05.26 CST Remain Count를 AP에서 관리하도록 Option 처리 (단, View용도로만 사용함), - 처리를 위해 SHORT을 사용함
	SHORT	dwCurrentCSTCntfromAP[4];	// [#2270] AU KSK 2014.05.26 CST Remain Count를 AP에서 관리하도록 Option 처리 (단, View용도로만 사용함), - 처리를 위해 SHORT을 사용함

	char	chHotKey[42];				//_MEM_VAR_OPT2_HOT_KEY_STRING	// [#2283] NH Justin Hot Key Information

	WORD	dwDonationEnable;			// OLD : _MEM_VAR_OPT2_STANDARD_DONATION_ENABLE	// [#2289] US Justin 2014.08.08 Add Donation
										// NEW : _MEM_VAR_OPT2_NH_DONATION_TYPE			// [#2441] US Justin.. Change Name
	WORD	dwCustomizedDualBalance;	//_MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE	// [#2292] US Justin 2014.09.17 Cardtronics Dual Balance Option
	WORD	dwCustomizedTIRSurcharge;	//_MEM_VAR_OPT2_CUSTOMIZED_TIRSURCHARGE	// [#2292] US Justin 2014.09.17 Cardtronics Surcharge Segmentation
	WORD	dwCustomizedDCC;			//_MEM_VAR_OPT2_CUSTOMIZED_DCC			// [#2292] US Justin 2014.09.17 Cardtronics DCC
	char	chCustomizedCRSurcharge[10];//_MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE// [#2292] US Justin 2014.09.17 Cardtronics Credit Withdrawal Surcharge

	WORD	dwBalanceNoFeeNotice;		//_MEM_VAR_OPT2_BALANCE_NOFEE_NOTICE	// [#2309] US Justin 2014.11.14 Balance No Fee Notice			// 1992 (1910+82) bytes

	WORD	dwEventNoticeEnable;		// _MEM_VAR_OPT2_EVENTNOTICE_ENABLE		// [#2313] US Justin 2014.11.25 Event Notice
	WORD	dwEventNoticeSSLEnable;		// _MEM_VAR_OPT2_EVENTNOTICE_SSL_ENABLE	// [#2313] US Justin 2014.11.25 Event Notice
	char	chEventNoticeHostName[256];	// _MEM_VAR_OPT2_EVENTNOTICE_HOSTNAME	// [#2313] US Justin 2014.11.25 Event Notice
	WORD	dwEventNoticeHostPort;		// _MEM_VAR_OPT2_EVENTNOTICE_HOSTPORTNO	// [#2313] US Justin 2014.11.25 Event Notice
	char	chEventNoticeUserName[20];	// _MEM_VAR_OPT2_EVENTNOTICE_USERNAME	// [#2313] US Justin 2014.11.25 Event Notice
	char	chEventNoticePassword[20];	// _MEM_VAR_OPT2_EVENTNOTICE_PASSWORD	// [#2313] US Justin 2014.11.25 Event Notice					// 2294 ( 1992+302) bytes

	WORD	dwDecimalSurcharge;			//_MEM_VAR_OPT2_DECIMAL_SURCHARGE		// [#2316] US Justin 2014.12.17 Decimal Surcharge				// 2296
	WORD	dwPercentSurchargeNotice;	//_MEM_VAR_OPT2_PERCENT_SURCHARGE_NOTICE// [#2335] US Justin 2015.03.11 Percent Surcharge Notice		// 2298
	WORD	dwRFIDDeviceEnable;			//_MEM_VAR_OPT2_RFID_DEVICE_ENABLE		// [#2333] NH KSK 2015.03.06 EPP LED GUIDE OPTION				// 2300
	WORD	dwEPPLEDGuideOption;		//_MEM_VAR_OPT2_EPP_LED_GUIDE_OPTION	// [#2333] NH KSK 2015.03.06 EPP LED GUIDE Option				// 2302																						// 2302
	WORD	dwUseUSCommonAID;			//_MEM_VAR_OPT2_USE_USCOMMONAID			// [#2341] US Justin 2015.04.29 US Common AID Option			// 2304

	WORD	dwDualHostDonationEnable;	// OLD : _MEM_VAR_OPT2_DUALHOST_DONATION_ENABLE		// [#2347] US Justin 2015.05.27 Dual Host Donation	// 2306	
										// NEW : _MEM_VAR_OPT2_NH_DONATION_APPEAR			// [#2441] US Justin Enable Both Donations
	WORD	dwDualHostDonationSSLEnable;// _MEM_VAR_OPT2_DUALHOST_DONATION_SSL_ENABLE		// [#2347] US Justin 2015.05.27 Dual Host Donation	// 2308
	char	chDualHostDonationHostName[256];// _MEM_VAR_OPT2_DUALHOST_DONATION_HOSTNAME		// [#2347] US Justin 2015.05.27 Dual Host Donation	// 2564	
	WORD	dwDualHostDonationHostPort;		// _MEM_VAR_OPT2_DUALHOST_DONATION_HOSTPORTNO	// [#2347] US Justin 2015.05.27 Dual Host Donation	// 2566

	WORD	dwDCCLookupLocalSurcharge;	// _MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE		// [#2349] US Justin 2015.06.05 DCC Local Surcharge		// 2568
	WORD	dwPOPMoneyEnable;			// _MEM_VAR_OPT2_POPMONEY_ENABLE				// [#2350] US Justin 2015.06.17 PAI POP Money			// 2570
	WORD	dwPOPMoneyMaxAmt;			// _MEM_VAR_OPT2_POPMONEY_MAXAMT				// [#2350] US Justin 2015.06.17 PAI POP Money			// 2572
	WORD	dwWithdrawalType;			// _MEM_VAR_OPT2_WITHDRAWAL_TYPE				// [#2350] US Justin 2015.06.18 PAI POP Money			// 2574
	WORD	dwAddonLoanDisbursement;	// _MEM_VAR_OPT2_ADDON_LOANDISBURSEMENT_ENABLE	// [#2351] US Justin 2015.06.23 Addon Loan Disbursement	// 2576
	WORD	dwDonationMaxAmount;		// _MEM_VAR_OPT2_DONATION_MAX_AMT				// [#2353] US Justin 2015.06.24 PAI Donation			// 2578
	WORD	dwDonationMinAmount;		// _MEM_VAR_OPT2_DONATION_MIN_AMT				// [#2351] US Justin 2015.06.24 PAI Donation			// 2580
	WORD	dw4000WReceiptFullCut;		// _MEM_VAR_OPT2_4000W_RECEIPT_FULLCUT			// [#2371] US Justin 2015.09.08 4000W Partial/FullCut	// 2582

	WORD	wMoniAirEnable;						// _MEM_VAR_OPT2_MONIAIR_ENABLE					// [#2326] US Kook 2015.03.20 Support MoniAir	// 2584
	WORD	wMoniAirCurrentCarrier;				// _MEM_VAR_OPT2_MONIAIR_CURRENT_CARRIER		// [#2326] US Kook 2015.06.08 Support MoniAir	// 2586
	WORD	wMoniAirConnectionMode;				// _MEM_VAR_OPT2_MONIAIR_CONNECTION_MODE		// [#2326] US Kook 2015.04.01 Support MoniAir	// 2588
	DWORD	dwMoniAirUsageSent;					// _MEM_VAR_OPT2_MONIAIR_USAGE_SENT				// [#2326] US Kook 2015.04.01 Support MoniAir	// 2592
	DWORD	dwMoniAirUsageRecv;					// _MEM_VAR_OPT2_MONIAIR_USAGE_RECV				// [#2326] US Kook 2015.04.01 Support MoniAir	// 2596
	char	chMoniAirUsageLastResetDate[16];	// _MEM_VAR_OPT2_MONIAIR_USAGE_LASTRESETDATE	// [#2326] US Kook 2015.08.24 Support MoniAir	// 2612
	char	chMoniAirMDN[16];					// _MEM_VAR_OPT2_MONIAIR_MDN					// [#2326] US Kook 2015.09.11 Support MoniAir	// 2628
	WORD	wMoniAirActivationStatus;			// _MEM_VAR_OPT2_MONIAIR_ACTIVATION_STATUS		// [#2326] US Kook 2015.10.12 Support MoniAir	// 2630

	WORD	dwMobileAppQRCode;			// _MEM_VAR_OPT2_MOBILEAPP_QRCODE				// [#2382] US Justin 2015.12.03 Mobiie App QR Code		// 2632
	WORD	dwLocalLoanDisbursement;	// _MEM_VAR_OPT2_LOCAL_LOAN_ENABLE				// [#2385] US Justin 2015.12.30 Mocal Loan Disbursement	// 2634
	WORD	dwLocalLoanDisAvailable;	// _MEM_VAR_OPT2_LOCAL_LOAN_AVAILABLE			// [#2389] US Justin 2016.01.18 Mocal Loan Disbursement	// 2636
	WORD	dwCDUBinding;				// _MEM_VAR_OPT2_CDU_BINDING					// [#2392] US Justin 2016.01.27 CDU BINDING				// 2638

	WORD	dwPin4Enable;				// _MEM_VAR_OPT2_PIN4_ENABLE					// [#2396] US Justin 2016.02.10 PAI Pin4				// 2640
	WORD	dwPin4MaxAmt;				// _MEM_VAR_OPT2_PIN4_MAXAMT					// [#2396] US Justin 2016.02.10 PAI PIn4				// 2642
	WORD	wWalPayEnable;				// _MEM_VAR_OPT2_WALPAY_ENABLE					// [#2415] US Justin 2016.04.15 Cash Depot Walpay		// 2644
	WORD	wWalPayMaxAmt;				// _MEM_VAR_OPT2_WALPAY_MAXAMT					// [#2415] US Justin 2016.04.15 Cash Depot Walpay		// 2646
	char	chWalPayMachineCode[7];		// _MEM_VAR_OPT2_WALPAY_MACHINECODE				// [#2418] US Justin 2016.04.19 Walpay Machine Code		// 2653
	WORD	wWalPayEnterAmount;			// _MEM_VAR_OPT2_WALPAY_ENTERAMOUNT				// [#2427] US Justin 2016.05.23 Cash Depot Walpay		// 2655
	WORD	wEMVLanguageSelection;		// _MEM_VAR_OPT2_EMV_LANGUAGESELECTION			// [#2425] US Justin 2016.05.23 EMV Language Selection	// 2657
	WORD	wLocalLoanHealthCheck;		// _MEM_VAR_OPT2_LOCAL_LOAN_HEALTHCHECK			// [#2428] US Justin 2016.05.23 Local Loan Health Check	// 2659
	WORD	wCustomizedSurcharge;		// _MEM_VAR_OPT2_CUSTOMIZED_SURCHARGE			// [#2437] US Justin 2016.07.22 ALLPOINT SURCHARGE		// 2661

	WORD	wJustCashEnable;			// _MEM_VAR_OPT2_JUSTCASH_ENABLE				// [#2445] US Justin 2016.09.22 Just.Cash				// 2663
	WORD	wPaydiantCCAEnable;			// _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE			// [#2446] US Justin 2016.09.29 Paydiant (Paypal) CCA	// 2665

	WORD	wDispenseMixOption;			// _MEM_VAR_OPT2_DISPENSE_MIX_OPTION			// [#2459] AU KSK 2016.12.23							// 2667
	WORD	wDualHostLoanEnable;		// _MEM_VAR_OPT2_DH_LOAN_ENABLE					// [#2467] US Justin 2017.01.20 Dual Host Loan			// 2669
	WORD	wDualHostLoanUpdateBalance;	// _MEM_VAR_OPT2_DH_LOAN_UPDATE_BALANCE			// [#2467] US Justin 2017.01.20 Dual Host Loan			// 2671

	WORD	wDCCOptionEligible;			// _MEM_VAR_OPT2_DCC_OPTION_ELIGIBLE			// [#2472] US Justin 2017.02.07 DCC Withdrawal Option	// 2673
	WORD	wDCCOPtionMaxWithdrawal;	// _MEM_VAR_OPT2_DCC_OPTION_MAXWITHDRAWAL		// [#2472] US Justin 2017.02.07 DCC Withdrawal Option	// 2675
	WORD	wDCCOptionFastCash1;		// _MEM_VAR_OPT2_DCC_OPTION_FASTCASH1			// [#2472] US Justin 2017.02.07 DCC Withdrawal Option	// 2677
	WORD	wDCCOptionFastCash2;		// _MEM_VAR_OPT2_DCC_OPTION_FASTCASH2			// [#2472] US Justin 2017.02.07 DCC Withdrawal Option	// 2679
	WORD	wDCCOptionFastCash3;		// _MEM_VAR_OPT2_DCC_OPTION_FASTCASH3			// [#2472] US Justin 2017.02.07 DCC Withdrawal Option	// 2681
	WORD	wDCCOptionFastCash4;		// _MEM_VAR_OPT2_DCC_OPTION_FASTCASH4			// [#2472] US Justin 2017.02.07 DCC Withdrawal Option	// 2683
	WORD	wDCCOptionFastCash5;		// _MEM_VAR_OPT2_DCC_OPTION_FASTCASH5			// [#2472] US Justin 2017.02.07 DCC Withdrawal Option	// 2685
	WORD	wDCCOptionFastCash6;		// _MEM_VAR_OPT2_DCC_OPTION_FASTCASH6			// [#2472] US Justin 2017.02.07 DCC Withdrawal Option	// 2687

	WORD	wFallback4UnknownAID;		// _MEM_VAR_OPT2_FALLBACK_4_UNKNOWNAID			// [#2481] US Justin 2017.04.21 Fallback Option 4 unknownAID	// 2689
	WORD	wStd1DynamicFlowMAC;		// _MEM_VAR_OPT2_STD1_DYNAMICFLOW_MAC			// [#2490] NH Justin 2017.06.14 Standard1 Dynamic Flow MAC		// 2691

	WORD	wAlphiEnDisable;			// _MEM_VAR_OPT2_ALPHI_ENDISABLE				// [#2497] AU Kook 2017.08.07 ATS Official			// 2693
	char	chAlphiServerAddress[40];	// _MEM_VAR_OPT2_ALPHI_SERVER_ADDRESS			// 2733
	WORD	wAlphiServerPort;			// _MEM_VAR_OPT2_ALPHI_SERVER_PORT				// 2735
	char	chAlphiID[11];				// _MEM_VAR_OPT2_ALPHI_ID						// 2746
	char	chAlphiTerminalID[8];		// _MEM_VAR_OPT2_ALPHI_TERMINAL_ID				// 2754
	char	chAlphiOperatorID[15];		// _MEM_VAR_OPT2_ALPHI_OPERATOR_ID				// 2769
	char	chAlphiTDESKey[32];			// _MEM_VAR_OPT2_ALPHI_TDES_KEY					// 2801
	char	chAlphiExtraInformation[40];// _MEM_VAR_OPT2_ALPHI_EXTRA_INFORMATION		// end of [#2497]									// 2841
	WORD	dwAlphiMsgAuditNumber;		// _MEM_VAR_OPT2_ALPHI_MSG_AUDITNUMBER			// [#2497] NH woooZ 2017.09.14  ALPHI server		// 2843
	WORD	wAlphiReversalMaxRetry;		// _MEM_VAR_OPT2_ALPHI_REVERSAL_MAX_RETRY		// 2845
	WORD	wAlphiCardDataToBeUsed;		// _MEM_VAR_OPT2_ALPHI_CARDDATA_TO_BE_USED		// 2847
	char	wAlphiCurrencyCode[3];		// _MEM_VAR_OPT2_ALPHI_CURRENCY_CODE			// [#2497] NH woooZ 2017.10.25 ALPHI country code	// 2850 

	WORD	wFallbackEnableDisable;		// _MEM_VAR_OPT2_FALLBACK_ENABLE_DISABLE		// [#2517] US Justin 2017.11.29 EMV Fallback Enable/Diable Option	// 2852

	WORD	wAlphiSSLEnDisable;			// _MEM_VAR_OPT2_ALPHI_SSL_ENDISABLE			// 2854
	WORD	wPrintTotalAmountOption;	// _MEM_VAR_OPT2_PRINT_TOTAL_AMOUNT_OPTION		// [#2525] AU HJ AHN 2018.01.10 TOTAL AMOUNT Option	// 2856
	WORD	wAlphiSSLCertEnDisable;		// _MEM_VAR_OPT2_ALPHI_SSL_CERT_ENDISABLE		// [#2497] NH woooZ 2018.01.19  ALPHI server // 2858

	// [#2518] US Kook 2018.01.14 Support Camera
	WORD	wCamAntiShldrSurfEnable;	// _MEM_VAR_OPT2_CAM_ANTI_SHLDR_SURF_ENABLE		// 2860
	WORD	wCamFaceMemorizeEnable;		// _MEM_VAR_OPT2_CAM_FACE_MEMORIZE_ENABLE		// 2862
	WORD	wCamFaceMemorizeIndex;		// _MEM_VAR_OPT2_CAM_FACE_MEMORIZE_INDEX		// 2864
	// end of [#2518]

	WORD	wGivePayEnableDisable;		// _MEM_VAR_OPT2_GIVEPAY_ENABLE_DISABLE			// [#2530] US Justin 2018.01.24 Just.Cash			// 2866

	WORD	wCamEnable;					// _MEM_VAR_OPT2_CAM_ENABLE						// 2018.02.23										// 2868	

	WORD	wMoniMobileCashBalance;		// _MEM_VAR_OPT2_MONIMOBILE_CASHBALANCE			// [#2538] NH Justin 2018.03.09						// 2870	

	char	chWalPayButtonText[21];		// _MEM_VAR_OPT2_WALPAY_BUTTONTEXT				// [#2539] US Justin 2018.03.29 Walpay Button Text	// 2891

	WORD	wUSTerritoryOption;			// _MEM_VAR_OPT2_US_TERRITORY_OPTION			// [#2549] NH Justin 2018.05.07 US Territory Option	// 2893	
																	
	WORD	wRebootOption;				// _MEM_VAR_OPT2_REBOOT_OPTION					// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Option	// 2895
	WORD	wRebootTime;				// _MEM_VAR_OPT2_REBOOT_TIME					// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Option	// 2897
	WORD	wRebootInterval;			// _MEM_VAR_OPT2_REBOOT_INTERVAL				// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Option	// 2899

	WORD	wRMSSendRetry;				// _MEM_VAR_OPT2_RMSSEND_RETRY					// [#2559] NH Justin 2018.06.18 Retry Status Send if "send" failed	// 2901
	WORD	wGievPayConfigTime;			// _MEM_VAR_OPT2_GIVEPAY_CONFIG_TIME			// [#2574] US Justin GivePay Enhancement3, Add Download Config Time	// 2903
	WORD	wGievPayUpSell;				// _MEM_VAR_OPT2_GIVEPAY_UPSELL					// [#J006] US Justin GivePay Enhancement5, Add Upsell Option		// 2905

	//WORD	wGievPayEnrolled;			// _MEM_VAR_OPT2_GIVEPAY_ENROLLED				// [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6

	WORD	wDenominationSelectEnable;	// _MEM_VAR_OPT2_DENOMINATION_SELECT_ENABLE		// [#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA 

	WORD	wDynamicFlowDCCType;		// _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE			// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
	WORD	wDualHostDCCType;			// _MEM_VAR_OPT2_DUALHOST_DCC_TYPE				// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message

	char	chAtmDns2nd[16];			// _MEM_VAR_OPT2_ATM_DNS_2ND					// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI

	WORD	wRMSSendSilent;				// _MEM_VAR_OPT2_RMSSEND_SILENT_AFTER_TRAN		// [#2580] AU Kook 2018.11.06 RMS Test									// 2929
	char	chRKTEPPSerialNumber[8];	// _MEM_VAR_OPT2_RKT_EPP_SERIAL_NUMBER			// [#2580] AU Kook 2019.07.08 Support RKT EPP Serial Number Inquiry		// 2937

	WORD	RMSTLSEnabled;				// [#RWC6-67] US William 2019.10.24 RMS TLS		// 2939
	WORD	RMSTLSPortNumber;			// [#RWC6-67] US William 2019.10.25 RMS TLS		// 2941

	WORD	wB4UEnabled;				// _MEM_VAR_OPT2_B4U_ENABLED					// [#RWC6-16] Bitload4U		// 2943

	WORD	wDispenseStyle;				// _MEM_VAR_OPT2_DISPENSE_STYLE					// [#RWC6-27]				// 2945

	char	chMasterPassword[256];		// _MEM_VAR_OPT2_MASTERPW						// [#RWC6-56] US William 2019.12.26 Hashing passwords					// 3201 bytes
	char	chOperatorPassword[256];	// _MEM_VAR_OPT2_OPERATORPW						// [#RWC6-56] US William 2019.12.26 Hashing passwords					// 3457 bytes
	char	chServicePassword[256];		// _MEM_VAR_OPT2_SERVICEPW						// [#RWC6-56] US William 2019.12.26 Hashing passwords					// 3713 bytes
	WORD	wPasswordLength;			// _MEM_VAR_OPT2_PWLENGTH						// [#RWC6-56] US William 2020.01.06 Hashing passwords					// 3715 bytes
	WORD	wLogonAttempts;				// _MEM_VAR_OPT2_INVALIDLOGONATTEMPTS			// [#RWC6-56] US William 2020.01.06 Hashing passwords					// 3717 bytes
	DWORD	dwLockoutExpriation;		// _MEM_VAR_OPT2_LOCKOUTEXPIRATION				// [#RWC6-56] US William 2020.01.06 Hashing passwords					// 3721 bytes
	WORD	wScheduleRemoteUpdatesEnabled;	// _MEM_VAR_OPT2_SCHEDULE_U2D_ENABLE		// [#RWC6-121] US William 2020.02.03 Remote Updates						// 3723 bytes
	WORD	wScheduledUpdateDoW;			// _MEM_VAR_OPT2_SCHEDULE_U2D_DOW			// [#RWC6-121] US William 2020.02.03 Remote Updates						// 3725 bytes
	DWORD	dwLibertyXSequenceNumber;	// _MEM_VAR_OPT2_LIBERTYX_SEQUENCE				// [#RWC6-216] US William 2020.08.21 LibertyX Sequence					// 3729 bytes

	WORD	wDualReceipt;				// _MEM_VAR_OPT2_DUAL_RECEIPT					// [#RWC6-290] US Brandon 2021.07.06 Support dual-language receipts		// 3731 bytes
	
	WORD	wInServiceWhenCSTsShort;	// _MEM_VAR_OPT2_INSERVICE_WHENCSTSSHORT		// [#RWC6-319] US Brandon 2021.07.06 No cash out-of-service option		// 3733 bytes
	WORD	wInServiceWhenCSTsEmpty;	// _MEM_VAR_OPT2_INSERVICE_WHENCSTSEMPTY		// [#RWC6-319] US Brandon 2021.07.06 No cash out-of-service option		// 3735 bytes

	WORD	wOPAccessCount;				//_MEM_VAR_OPT2_OPACCESSCOUNT					// [#RWC6-359] US Matt 2022.01.14 Quick Configuration no TID

	WORD	wRKTProtocol;				// _MEM_VAR_OPT2_RKT_PROTOCOL					// [#RWC6-151] US Kook 2020.08.04 TR-34									// 3737 bytes

	WORD	wSideCarEnabled;			// _MEM_VAR_OPT2_SIDECAR_ENABLE					// [#GLDV-3005] US Kook 2021.10.22 Support Side Car						// 3733 bytes
	WORD	wSideCarBnaType;			// _MEM_VAR_OPT2_SIDECAR_BNATYPE				// [#GLDV-3005] US Kook 2021.10.22 Support Side Car						// 3735 bytes
	char	chSideCarCashInTotalTime[16];// _MEM_VAR_OPT2_SIDECAR_CASHINTOTAL_TIME		// [#GLDV-3005] US Kook 2021.10.22 Support Side Car						// 3751 bytes

// ****************주의 사항 **************************************
//NVRAM에 변수 추가 시 OP, RMS 및 HOST에서 업데이트 가능한 항목이면
//CDevCmn::SaveChangeParameter() 함수에 해당 항목 추가해 주세욤 !!!!!
//******************************************************************

} OPTINALSETTING2, NEAR* NPOPTIONALSETTING2, FAR* LPOPTIONALSETTING2;
// end of [#434]

// [#434] AU AIREAT 2008.10.14
typedef	struct tagNVRAM8 {
	OPTINALSETTING2	APPOptionalSetting2;		//---사용함---//
} NVRAM8, NEAR* NPNVRAM8_ADDR, FAR* LPNVRAM8_ADDR;
// end of [#434]

//---사용함---//
// [#534] NH AIREAT 2009.06.04 : Reject Total
typedef struct tagRejectTotal{
	char		szRejTotVersion[8];						// [#534] AIREAT 2009.06.04 : V010100
	int			nDateTime[6];							// [#534] AIREAT 2009.06.04 : 0(Year), 1(Month), 2(Day), 3(Hour), 4(Minute), 5(Second)
	DWORD		dwDispensed[MAX_CST_COUNT];				// [#534] AIREAT 2009.06.04 : DISPENSED COUNT, SP정의와 같이 (Max 6개?)
	DWORD		dwRejected[MAX_CST_COUNT];				// [#534] AIREAT 2009.06.04 : REJECTED COUNT, SP정의와 같이 (Max 6개?)
	DWORD		dwDetectedSkew[MAX_CST_COUNT];			// [#534] AIREAT 2009.06.04 : DETECTED SKEW SUM COUNT, SP정의와 같이 (Max 6개?)
	DWORD		dwDetectedGap[MAX_CST_COUNT];			// [#534] AIREAT 2009.06.04 : DETECTED GAP SUM COUNT, SP정의와 같이 (Max 6개?)
	DWORD		dwDetectedLong[MAX_CST_COUNT];			// [#534] AIREAT 2009.06.04 : DETECTED LONG SUM COUNT, SP정의와 같이 (Max 6개?)
	DWORD		dwDetectedShort[MAX_CST_COUNT];			// [#534] AIREAT 2009.06.04 : DETECTED SHORT SUM COUNT, SP정의와 같이 (Max 6개?)
	DWORD		dwDetectedDouble[MAX_CST_COUNT];		// [#534] AIREAT 2009.06.04 : DETECTED DOUBLE SUM COUNT, SP정의와 같이 (Max 6개?)
	DWORD		dwTestRejected[MAX_CST_COUNT];			// KSK 2009.07.17 VDM에서 TEST DISPENSE시 UPDATE 영역 추가
} REJECT_TOTAL, NEAR* NPREJECT_TOTAL, FAR* LPREJECT_TOTAL;
// end of [#534]

//---사용함---//
// [#534] NH AIREAT 2009.06.04
typedef struct tagNVRAM9 {
	REJECT_TOTAL	RejectTotal;
} NVRAM9, NEAR* NPNVRAM9_ADDR, FAR* LPNVRAM9_ADDR;
// end of [#534]

// [#620] US KSK 2010.02.18
typedef	struct tagEnhancedCoupon{
	// [#620] Coupon 관련 Setup 정보 추가
	WORD	wEnhancedCouponAvailable;
	WORD	wEnhancedCouponUse;
	WORD	wEnhancedCouponCutOption;
	WORD	wEnhancedCouponOption[6];
	WORD	wEnhancedCouponPrintStartTime[6];
	WORD	wEnhancedCouponPrintEndTime[6];
	char	chEnhancedCouponText[6][16][40];
	// end of [#620]
} ENHANCEDCOUPON, NEAR* NPENHANCEDCOUPON, FAR* LPENHANCEDCOUPON;

// [#2014] NH KJW 2011.04.26
typedef struct tagLogLFormat{		// Lottery
	WORD	wCount;					// Count
	DWORD	dwAmount;				// Amount
	char	Dummy;					// Dummy
} LOGL;

typedef struct tagLOTTERYTOTALINFO{	// Lottery Total/Subtotal Info
	LOGL	DATA[2];				// 0:LOTTERY
									// 1:REVERSAL of LOTTERY
} LOTTERYTOTALINFO, NEAR* NPLOTTERYTOTALINFO, FAR* LPLOTTERYTOTALINFO;
// end of [#2014]

// [#2185] US Justin 2013.05.14
typedef struct tagLogDHFormat{		// DualHostDCC
	WORD	wCount;					// Count
	DWORD	dwAmount;				// Amount
	char	Dummy;					// Dummy
} LOGDH;

typedef struct tagDHDCCTOTALINFO{	// DualHostDcc Total/Subtotal Info
	LOGDH	DATA[3];				// 0:Dual Host DCC
									// 1:Surcharge
									// 2:Reversal
} DHDCCTOTALINFO, NEAR* NPDHDCCTOTALINFO, FAR* LPDHDCCTOTALINFO;
// end of [#2185]

typedef struct tagNVRAM12 {
	ENHANCEDCOUPON	EnhancedCoupon;
	LOTTERYTOTALINFO	LotteryTotal;	// [#2014] NH KJW 2011.04.26
	DHDCCTOTALINFO		DHDCCTotal;		// [#2185] US Justin 2013.05.14
} NVRAM12, NEAR* NPNVRAM12_ADDR, FAR* LPNVRAM12_ADDR;
// end of [#620]

typedef struct tabOptionalSetting3 {
	// _MEM_VAR_OPT2_TR31_WKB
	// [#RWC6-150] US William 2021.03.29 TR-31
	char	chTR31WorkingKeyBlock[128];	// 128 bytes (0 + 128)

	// _MEM_VAR_OPT2_TR31_MKB
	// [#RWC6-150] US William 2021.03.29 TR-31
	char	chTR31MACKeyBlock[128];		// 256 bytes (128 + 128)

	// _MEM_VAR_OPT3_CASHDEPOT_ISHOSTUSEURL
	WORD	dwCashDepotIsHostURL;		// 258 bytes (256 + 2)

	// _MEM_VAR_OPT3_CASHDEPOT_HOSTNAME
	char	chCashDepotHostName[256];	// 514 bytes (258 + 256)

} OPTINALSETTING3, NEAR* NPOPTIONALSETTING3, FAR* LPOPTIONALSETTING3;

typedef struct tagNVRAM13 {
	OPTINALSETTING3 APPOptionalSetting3;
} NVRAM13, NEAR* NPNVRAM13_ADDR, FAR* LPNVRAM13_ADDR;

#pragma pack()

/////////////////////////////////////////////////////////////////////////////
#endif