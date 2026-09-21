/////////////////////////////////////////////////////////////////////////////
//#if !defined(_DEVDEFINE_H_INCLUDED_)
//#define _DEVDEFINE_H_INCLUDED_

#ifndef __DEV_DEFINE_H__
#define __DEV_DEFINE_H__

/////////////////////////////////////////////////////////////////////////////
//	Cash Condition Count Define(Mecha)
/////////////////////////////////////////////////////////////////////////////
#define CDU_EMPTYCASHCNT	0						// FOR US, FIX COUNT AS 0
													// EMPTY CASH COUNT : 2004.11.05
/////////////////////////////////////////////////////////////////////////////
//	Timer
/////////////////////////////////////////////////////////////////////////////
#define	TIMER_THREAD			1					// TIMER THREAD

// [#369] [NH] KSK 2008.7.18
#define	TRANTYPE_WITHDRAWAL			_T("CW")
#define	TRANTYPE_INQUIRY			_T("BI")
#define	TRANTYPE_TRANSFER			_T("TR")
#define	TRANTYPE_NONCASH_WITHDRAW	_T("NW")		// [#2496] US Justin 2017.08.15 Add Non Cash (BitCoin)

#define	TRANTYPE_PINCHANGE			_T("PC")		// [#2150] US Justin 2012.10.04 Add Pin Change Transaction
#define	TRANTYPE_DCC				_T("DC")		// [#2150] US Justin 2012.10.09 Add Dynamic Currency Conversion Transaction
// end of [#369]
#define TRANTYPE_PAYPAL				_T("MW")		// [#2446] US Justin 2016.09.29 Paydiant (Paypal) CCA		// Mobile Withdrawal		

#define TRANTYPE_POPMONEY			_T("PM")		// [#2350] US Justin 2015.06.19 Add POP Money				=> Journal Only 
#define TRANTYPE_PIN4				_T("P4")		// [#2396] US Justin 2016.02.17 Pin4						=> Journal Only
#define TRANTYPE_JUSTCASH			_T("JC")		// [#2445] US Justin 2016.09.28 Just.Cash					=> Journal Only
#define TRANTYPE_GIVEPAY			_T("GP")		// [#2535] US Justin 2018.04.10 GivePay Online	
#define TRANTYPE_B4U				_T("BU")		// [#RWC6-16] Bitload 4 U. A new cryptocurrency feature for CE 6.0 ATMs

//////////////////////////////////////////////////////////////////////////////////
// Message Type (1: Tranax, 2: CSP200(Diebold), 3: Triton) => ALL VISA II PROTOCOL
//////////////////////////////////////////////////////////////////////////////////
#define	MSG_HYOSUNG_TYPE				_T("1")		// MSG TRANAX TYPE
#define	MSG_CSP200_TYPE					_T("2")		// MSG CSP200 TYPE
#define	MSG_TRITON_TYPE					_T("3")		// MSG TRITON TYPE
#define	MSG_EPS_TYPE					_T("5")		// [#98] NH 2008.04.08 KGS EPS MessageFormat 선택 메뉴 추가

#define	BINLIST_SIZE			20
#define BINLIST_START_INDEX		3503

/////////////////////////////////////////////////////////////////////////////
//	NVRAM Error Information Size Define
/////////////////////////////////////////////////////////////////////////////
#define ERRORSTACK_MAX_SIZE		50
#define	CURERRSTACK_MAX_SIZE	10		// [#419] [NH] KSK 2008.9.16
#define	ERRORDATE_SIZE			10
#define	ERRORTIME_SIZE			10
#define	ERRORPROCCOUNT_SIZE		2
#define	ERRORCODE_SIZE			8
#define ERRORSTRING_SIZE		42

// [#534] AIREAT 2009.06.04 : Reject Total
//////////////////////////////////////////////////////////////////////////
//	NVRAM REJECT TOTAL INFORMATION SIZE DEFINE
//////////////////////////////////////////////////////////////////////////
#define MAX_CST_COUNT			6		// SP 정의와 같게 유지.

// end of [#534]

#define WITHTOTALINFO_DATA_SIZE	 16
#define INQTOTALINFO_DATA_SIZE	 2
#define TRANSTOTALINFO_DATA_SIZE 2
#define DHDCCTOTALINFO_DATA_SIZE 3		// [#2185] US Justin 2013.05.14 Dual Host DCC

/////////////////////////////////////////////////////////////////////////////
// SEND/RECEIVE MESSAGE ORDER FOR MULTI TRANSACTON.
// [#2137] MX PCS 2012.07.23
/////////////////////////////////////////////////////////////////////////////
#define MX_NOT_SEND					0
#define MX_1ST_SEND					1
#define MX_2ND_SEND					2

#define MX_NOT_SET					0
#define MX_DOMESTIC_TYPE			1
#define MX_INTERNATIONAL_TYPE		2
// end of [#2137]
/////////////////////////////////////////////////////////////////////////////

//	RMS command
/////////////////////////////////////////////////////////////////////////////
#define	RMSCMD_IDLE					0
#define RMSCMD_RMSACTIVE			1
#define	RMSCMD_REBOOT				2
#define RMSCMD_SWUPDATE				3
#define RMSCMD_INITALLDEV			4
#define RMSCMD_INITERRDEV			5
#define RMSCMD_SETADDCASH			6		// [#530] NH KSK 2009.06.03
#define RMSCMD_EXECCSTTOTAL			7		// [#531] NH KSK 2009.06.03
#define RMSCMD_EXECDAYTOTAL			8
#define RMSCMD_EXECTRIALDAYTOTAL	9
#define RMSCMD_CHANGEDATETIME		10		////[#611] SOOK 2010.01.16 Change ATM Time추가 
#define RMSCMD_SWUPDATE_REPO		11

// 불필요 Define 삭제 KSK 2009.1.5
//#define _CDMSP_FLD_DISPENSEDNOTES				_T("DISPENSED_NOTES")
//#define _CDMSP_FLD_REJECTEDNOTES				_T("REJECTED_NOTES")

/////////////////////////////////////////////////////////////////////////////
//	FIELD SEPERATING NUMBER DEFINE
/////////////////////////////////////////////////////////////////////////////
#define _FIELD_NO						0
#define _VAR_DEV_NO						500
#define _VAR_APP_NO						1000
#define _VAR_NET_NO						1500
#define _VAR_SUM_NO						2000
#define _VAR_ERR_NO						2500
#define	_VAR_VERSION_NO					3000
#define _VAR_PINSP_NO					3500
#define	_VAR_ADVINFO_NO					4000
#define	_VAR_TCPIPINFO_NO				4500
#define _VAR_OPT_NO						5000
#define	_VAR_EMVINFO_NO					5500
#define	_VAR_EMV_REVERSAL				6000
#define _VAR_OPT2_NO					6500			// [#434] NH AIREAT 2008.10.14
#define _VAR_ENAHNCED_COUPON			7000			// [#620] US KSK 2010.02.18
#define _VAR_EMV_CONFIG					7500			// [#2342] US Justin 2015.05.07 AID Selection
#define _VAR_OPT3_NO					8000


/////////////////////////////////////////////////////////////////////////////
//	FIELD NAME DEFINITION
/////////////////////////////////////////////////////////////////////////////
#define	_MEM_FLD_INI_DEVINFO		_FIELD_NO + 1		// DEVICE.INI
#define	_MEM_FLD_INI_NETWORKINFO	_FIELD_NO + 2		// NETWORK.INI
#define	_MEM_FLD_INI_TRANSINFO		_FIELD_NO + 3		// TRANS.INI
#define _MEM_FLD_APP_ATMINFO		_FIELD_NO + 4		//
#define	_MEM_FLD_APP_ERRTRANS		_FIELD_NO + 5		//
#define	_MEM_FLD_APP_HOSTCMD		_FIELD_NO + 6		//
#define _MEM_FLD_APP_BININFO		_FIELD_NO + 7		//
#define _MEM_FLD_APP_SIMINFO		_FIELD_NO + 8		//
#define	_MEM_FLD_INI_WITHTOTALINFO	_FIELD_NO + 9		// WITHDRAWAL.INI
#define	_MEM_FLD_INI_INQTOTALINFO	_FIELD_NO + 10		// INQTOTALINFO
#define	_MEM_FLD_INI_TRANSTOTALINFO	_FIELD_NO + 11		// TRANSTOTALINFO
#define	_MEM_FLD_INI_DEPTOTALINFO	_FIELD_NO + 12		// DEPTOTALINFO
#define _MEM_FLD_APP_ERRSTCK		_FIELD_NO + 13		//
#define _MEM_FLD_SP_CDM				_FIELD_NO + 14		// CDU SP
#define _MEM_FLD_SP_PIN				_FIELD_NO + 15		// PIN SP
#define	_MEM_FLD_ADVINFO			_FIELD_NO + 16		// 광고 IMAGE INFO
#define	_MEM_FLD_TCPIPINFO			_FIELD_NO + 17		// TCPIP INFO
#define _MEM_FLD_OPTIONALSETTING	_FIELD_NO + 18		// [#4] NH PSC 2008.03.10 Optional Setting
#define	_MEM_FLD_EMVINFO			_FIELD_NO + 19		// EMV INFO
#define	_MEM_FLD_APP_VERSIONINFO	_FIELD_NO + 20		// [#127] KSK 2008.04.21
#define	_MEM_FLD_EMVREVERSAL		_FIELD_NO + 21		// [#68] UK HWANG 2008.04.02 EMV Level2
#define _MEM_FLD_OPTIONALSETTING_2	_FIELD_NO + 22		// [#434] AU AIREAT 2008.10.14
#define _MEM_FLD_ENHANCED_COUPON	_FIELD_NO + 23		// [#620] US KSK 2010.02.18
#define	_MEM_FLD_INI_LOTTERYINFO	_FIELD_NO + 24		// LOTTERY.INI // [#2014] NH KJW 2011.02.23
#define	_MEM_FLD_INI_DHDCCINFO		_FIELD_NO + 25		// [#2185] US Justin 2013.05.14 Dual Host DCC
#define	_MEM_FLD_EMVCONFIG			_FIELD_NO + 26		// [#2342] US Justin 2015.05.07 AID Selection
#define _MEM_FLD_OPTIONALSETTING_3	_FIELD_NO + 27

/////////////////////////////////////////////////////////////////////////////
//	VARIABLE NAME DEFINITION
//  DEVICE
/////////////////////////////////////////////////////////////////////////////
#define	_MEM_VAR_DEVINFO_DISPCASHCST1		_VAR_DEV_NO+5	// _T("DISPCASHCST1")	// SP AREA
#define	_MEM_VAR_DEVINFO_DISPCASHCST2		_VAR_DEV_NO+6	// _T("DISPCASHCST2")	// SP AREA
#define	_MEM_VAR_DEVINFO_DISPCASHCST3		_VAR_DEV_NO+7	// _T("DISPCASHCST3")	// SP AREA
#define	_MEM_VAR_DEVINFO_DISPCASHCST4		_VAR_DEV_NO+8	// _T("DISPCASHCST4")	// SP AREA
// [Reject Note]
#define _MEM_VAR_DEVINFO_RJTCASSETTE1		_VAR_DEV_NO+21	// _T("Cassette1")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTCASSETTE2		_VAR_DEV_NO+22	// _T("Cassette2")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTCASSETTE3		_VAR_DEV_NO+23	// _T("Cassette3")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTCASSETTE4		_VAR_DEV_NO+24	// _T("Cassette4")	// SP AREA
// [Volume Level]
#define _MEM_VAR_DEVINFO_VOLUMELEVEL		_VAR_DEV_NO+29	// _T("VolumeLevel")
// [LOGFILE INDEX]
#define _MEM_VAR_DEVINFO_LOGFILE_INDEX		_VAR_DEV_NO+30

//RMS 관련 추가
#define _MEM_VAR_CDMSP_NUMBEROFCST			_VAR_DEV_NO+31
#define _MEM_VAR_CDMSP_CSTCOUNT1			_VAR_DEV_NO+32
#define _MEM_VAR_CDMSP_CSTCOUNT2			_VAR_DEV_NO+33
#define _MEM_VAR_CDMSP_CSTCOUNT3			_VAR_DEV_NO+34
#define _MEM_VAR_CDMSP_CSTCOUNT4			_VAR_DEV_NO+35
#define _MEM_VAR_CDMSP_CSTVALUE1			_VAR_DEV_NO+36
#define _MEM_VAR_CDMSP_CSTVALUE2			_VAR_DEV_NO+37
#define _MEM_VAR_CDMSP_CSTVALUE3			_VAR_DEV_NO+38
#define _MEM_VAR_CDMSP_CSTVALUE4			_VAR_DEV_NO+39
#define _MEM_VAR_CDMSP_CSTSTATUS1			_VAR_DEV_NO+40
#define _MEM_VAR_CDMSP_CSTSTATUS2			_VAR_DEV_NO+41
#define _MEM_VAR_CDMSP_CSTSTATUS3			_VAR_DEV_NO+42
#define _MEM_VAR_CDMSP_CSTSTATUS4			_VAR_DEV_NO+43
#define _MEM_VAR_DEVINFO_JACKDETECT			_VAR_DEV_NO+44
#define _MEM_VAR_CDMSP_FLAGENQSEND			_VAR_DEV_NO+45
#define _MEM_VAR_CDMSP_CSTTYPE1				_VAR_DEV_NO+46
#define _MEM_VAR_CDMSP_CSTTYPE2				_VAR_DEV_NO+47
#define _MEM_VAR_CDMSP_CSTTYPE3				_VAR_DEV_NO+48
#define _MEM_VAR_CDMSP_CSTTYPE4				_VAR_DEV_NO+49
#define _MEM_VAR_DEVINFO_SPRSP_PAPERSTAT	_VAR_DEV_NO+50	// KSK 2009.12.23 사용안하는걸로 수정함 (Device에게 직접 Read하도록 수정)
#define _MEM_VAR_CDMSP_FLAGCDULOGBACKUP		_VAR_DEV_NO+51	// [Denomination 시재사고 대책] 2007.11.26 V01.02.19
#define _MEM_VAR_CDMSP_CST1_LOWSENSOR		_VAR_DEV_NO+52	// [#513] [NH] KSK 2009.3.2 Cassette1 Low Sensor
#define _MEM_VAR_CDMSP_CST2_LOWSENSOR		_VAR_DEV_NO+53	// [#513] [NH] KSK 2009.3.2 Cassette1 Low Sensor
#define _MEM_VAR_CDMSP_CST3_LOWSENSOR		_VAR_DEV_NO+54	// [#513] [NH] KSK 2009.3.2 Cassette1 Low Sensor
#define _MEM_VAR_CDMSP_CST4_LOWSENSOR		_VAR_DEV_NO+55	// [#513] [NH] KSK 2009.3.2 Cassette1 Low Sensor
#define	_MEM_VAR_CDMSP_CST1_T_DISP_CNT		_VAR_DEV_NO+56	// KSK 2009.3.30
#define	_MEM_VAR_CDMSP_CST2_T_DISP_CNT		_VAR_DEV_NO+57	// KSK 2009.3.30
#define	_MEM_VAR_CDMSP_CST3_T_DISP_CNT		_VAR_DEV_NO+58	// KSK 2009.3.30
#define	_MEM_VAR_CDMSP_CST4_T_DISP_CNT		_VAR_DEV_NO+59	// KSK 2009.3.30
#define _MEM_VAR_CDMSP_CST1_T_REJ_CNT		_VAR_DEV_NO+60	// KSK 2009.3.30
#define _MEM_VAR_CDMSP_CST2_T_REJ_CNT		_VAR_DEV_NO+61	// KSK 2009.3.30
#define _MEM_VAR_CDMSP_CST3_T_REJ_CNT		_VAR_DEV_NO+62	// KSK 2009.3.30
#define _MEM_VAR_CDMSP_CST4_T_REJ_CNT		_VAR_DEV_NO+63	// KSK 2009.3.30

// [#526]
#define _MEM_VAR_DEVINFO_RJTSKEW_CST1			_VAR_DEV_NO+64	// _T("Cassette1")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTSKEW_CST2			_VAR_DEV_NO+65	// _T("Cassette2")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTSKEW_CST3			_VAR_DEV_NO+66	// _T("Cassette3")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTSKEW_CST4			_VAR_DEV_NO+67	// _T("Cassette4")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTGAPTOCLOSE_CST1		_VAR_DEV_NO+68	// _T("Cassette1")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTGAPTOCLOSE_CST2		_VAR_DEV_NO+69	// _T("Cassette2")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTGAPTOCLOSE_CST3		_VAR_DEV_NO+70	// _T("Cassette3")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTGAPTOCLOSE_CST4		_VAR_DEV_NO+71	// _T("Cassette4")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTLONGNOTE_CST1		_VAR_DEV_NO+72	// _T("Cassette1")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTLONGNOTE_CST2		_VAR_DEV_NO+73	// _T("Cassette2")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTLONGNOTE_CST3		_VAR_DEV_NO+74	// _T("Cassette3")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTLONGNOTE_CST4		_VAR_DEV_NO+75	// _T("Cassette4")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTSHORTNOTE_CST1		_VAR_DEV_NO+76	// _T("Cassette1")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTSHORTNOTE_CST2		_VAR_DEV_NO+77	// _T("Cassette2")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTSHORTNOTE_CST3		_VAR_DEV_NO+78	// _T("Cassette3")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTSHORTNOTE_CST4		_VAR_DEV_NO+79	// _T("Cassette4")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTDOUBLE_CST1			_VAR_DEV_NO+80	// _T("Cassette1")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTDOUBLE_CST2			_VAR_DEV_NO+81	// _T("Cassette2")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTDOUBLE_CST3			_VAR_DEV_NO+82	// _T("Cassette3")	// SP AREA
#define _MEM_VAR_DEVINFO_RJTDOUBLE_CST4			_VAR_DEV_NO+83	// _T("Cassette4")	// SP AREA
// end of [#526]

#define	_MEM_VAR_CDMSP_CST1_T_CSTOUT_CNT		_VAR_DEV_NO+84	// [#2357] NH KSK 2015.07.11 Cash Out Count를 Get하기 위해 Define
#define	_MEM_VAR_CDMSP_CST2_T_CSTOUT_CNT		_VAR_DEV_NO+85	// [#2357] NH KSK 2015.07.11 Cash Out Count를 Get하기 위해 Define
#define	_MEM_VAR_CDMSP_CST3_T_CSTOUT_CNT		_VAR_DEV_NO+86	// [#2357] NH KSK 2015.07.11 Cash Out Count를 Get하기 위해 Define
#define	_MEM_VAR_CDMSP_CST4_T_CSTOUT_CNT		_VAR_DEV_NO+87	// [#2357] NH KSK 2015.07.11 Cash Out Count를 Get하기 위해 Define
#define _MEM_VAR_DEVINFO_CDU_AUTHFLAG			_VAR_DEV_NO+88	// _T("CE부 인증성공 여부")	// SP AREA	// [###] US KSK 2015.12.17
#define _MEM_VAR_DEVINFO_CDU_SUPPORTAR			_VAR_DEV_NO+89	// [#RWC6-82] NH Kook 2021.12.27 Support CDU Auto Recovery (Jam Clear)

/////////////////////////////////////////////////////////////////////////////
//	VARIABLE NAME DEFINITION
//  PIN SP
/////////////////////////////////////////////////////////////////////////////
#define _MEM_VAR_PINSP_KEYMODE				_VAR_PINSP_NO+1
#define	_MEM_VAR_PINSP_KEYINDEX				_VAR_PINSP_NO+2
#define _MEM_VAR_PINSP_KEYCHECK				_VAR_PINSP_NO+3
#define	_MEM_VAR_PINSP_KEYCHECKSUM0			_VAR_PINSP_NO+4
#define	_MEM_VAR_PINSP_KEYCHECKSUM1			_VAR_PINSP_NO+5
#define	_MEM_VAR_PINSP_KEYCHECKSUM2			_VAR_PINSP_NO+6
#define	_MEM_VAR_PINSP_KEYCHECKSUM3			_VAR_PINSP_NO+7
#define	_MEM_VAR_PINSP_KEYCHECKSUM4			_VAR_PINSP_NO+8
#define	_MEM_VAR_PINSP_KEYCHECKSUM5			_VAR_PINSP_NO+9
#define	_MEM_VAR_PINSP_KEYCHECKSUM6			_VAR_PINSP_NO+10
#define	_MEM_VAR_PINSP_KEYCHECKSUM7			_VAR_PINSP_NO+11
#define	_MEM_VAR_PINSP_KEYCHECKSUM8			_VAR_PINSP_NO+12
#define	_MEM_VAR_PINSP_KEYCHECKSUM9			_VAR_PINSP_NO+13
#define	_MEM_VAR_PINSP_KEYCHECKSUM10		_VAR_PINSP_NO+14
#define	_MEM_VAR_PINSP_KEYCHECKSUM11		_VAR_PINSP_NO+15
#define	_MEM_VAR_PINSP_KEYCHECKSUM12		_VAR_PINSP_NO+16
#define	_MEM_VAR_PINSP_KEYCHECKSUM13		_VAR_PINSP_NO+17
#define	_MEM_VAR_PINSP_KEYCHECKSUM14		_VAR_PINSP_NO+18
#define	_MEM_VAR_PINSP_KEYCHECKSUM15		_VAR_PINSP_NO+19

/////////////////////////////////////////////////////////////////////////////
//	VARIABLE NAME DEFINITION
//  NETWORK INFO
/////////////////////////////////////////////////////////////////////////////
// [NETWORK]
#define	_MEM_VAR_NETINFO_STATUSMONITORINGENABLE		_VAR_NET_NO+1	// _T("StatusMonitoring")
#define	_MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE	_VAR_NET_NO+2	// _T("CommunicationHeader")
#define	_MEM_VAR_NETINFO_COMMUNICATIONID			_VAR_NET_NO+3	// _T("CommunicationId")
#define _MEM_VAR_NETINFO_EOTCHECKMODE				_VAR_NET_NO+4	// _T("EOTCheckMode")
//#define _MEM_VAR_NETINFO_ENQTIMEOUT					_VAR_NET_NO+5	// _T("ENQTIMEOUT")			- KSK 2009.12.23사용안함으로 수정
//#define _MEM_VAR_NETINFO_MODEMVENDOR				_VAR_NET_NO+6	// _T("ModemVendor")			- KSK 2009.12.23사용안함으로 수정
//#define _MEM_VAR_NETINFO_MSGDEFINE_FORMATOPTION		_VAR_NET_NO+7	// _T("FormatOption")		- KSK 2009.12.23사용안함으로 수정
#define _MEM_VAR_NETINFO_MACFLAG					_VAR_NET_NO+8	// _T("MacFlag")
//#define _MEM_VAR_NETINFO_CONNECTTIMER				_VAR_NET_NO+9	// _T("ModemConnectTimer")		- KSK 2009.12.23사용안함으로 수정
#define _MEM_VAR_NETINFO_PHONENODEFINE1				_VAR_NET_NO+10	// _T("HostPhoneNum1")
#define _MEM_VAR_NETINFO_PHONENODEFINE2				_VAR_NET_NO+11	// _T("HostPhoneNum2")
#define _MEM_VAR_NETINFO_MODEMINITSTR				_VAR_NET_NO+12	// _T("ModemInitString")
//#define _MEM_VAR_NETINFO_MODEMSPEAKER				_VAR_NET_NO+13	// _T("ModemSpeakerMode")		- KSK 2009.12.23사용안함으로 수정
//#define _MEM_VAR_NETINFO_HEALTHMSGTIMER				_VAR_NET_NO+14	// _T("HostHealthMsgTimer")	- KSK 2009.12.23사용안함으로 수정
#define _MEM_VAR_NETINFO_HSDELAYINT					_VAR_NET_NO+15	// _T("HealthySendInterval")
#define _MEM_VAR_NETINFO_HEALTHYFLAG				_VAR_NET_NO+16	// _T("HealthyEnable")
#define _MEM_VAR_NETINFO_DESKEY1					_VAR_NET_NO+17	// _T("DesKey1")
#define _MEM_VAR_NETINFO_DESKEY2					_VAR_NET_NO+18	// _T("DesKey2")
#define _MEM_VAR_NETINFO_DESKEY3					_VAR_NET_NO+19	// _T("DesKey3")
#define _MEM_VAR_NETINFO_DESKEY4					_VAR_NET_NO+20	// _T("DesKey4")
// [RMS]
#define _MEM_VAR_NETINFO_RMSRINGCNT					_VAR_NET_NO+21	// _T("RmsRingCnt")	- [#401] [NH] KSK 2008.8.14
#define _MEM_VAR_NETINFO_RMSFLAG					_VAR_NET_NO+22	// _T("RmsModeEnable")
#define _MEM_VAR_NETINFO_RMSSTATUSSEND				_VAR_NET_NO+23	// _T("RmsStatusSendEnable")
#define _MEM_VAR_NETINFO_RMSPASSWORD				_VAR_NET_NO+24	// _T("RmsPassword")
#define _MEM_VAR_NETINFO_RMSPHONENO1				_VAR_NET_NO+25	// _T("RmsPhoneNum1")
#define _MEM_VAR_NETINFO_RMSPHONENO2				_VAR_NET_NO+26	// _T("RmsPhoneNum2")
#define _MEM_VAR_NETINFO_RMSCOMMAND					_VAR_NET_NO+27  

/////////////////////////////////////////////////////////////////////////////
// TRANS AP
/////////////////////////////////////////////////////////////////////////////
//#define	_MEM_VAR_APP_SERIALNO					_VAR_APP_NO+1	// char	SerialNo[7];		- sequence no로 변경해야함(ERRORTRANS)	[#371] [US] KSK 2008.7.16 ATMINFO쪽 값을 참조하도록 수정
//#define	_MEM_VAR_APP_PROCCOUNT					_VAR_APP_NO+2	// char	ProcCount[2];			- KSK 2010.01.04 사용 안함으로 수정
//#define	_MEM_VAR_APP_LASTDISPNOTES				_VAR_APP_NO+3								- KSK 2009.12.23 사용 안함으로 수정
#define	_MEM_VAR_APP_REVERSALFLAG				_VAR_APP_NO+4	// _T("ReversalFlag")			// Reversal 실행 체크 변수
#define	_MEM_VAR_APP_LOCALTRANDATE				_VAR_APP_NO+5	// _T("LocalTranDate")			- 수정필요 (ERRORTRANS)
#define	_MEM_VAR_APP_LOCALTRANTIME				_VAR_APP_NO+6	// _T("LocalTranTime")			- 수정필요 (ERRORTRANS)
#define	_MEM_VAR_APP_REQUESTEDAMT				_VAR_APP_NO+7	// _T("RequestedAmount")		- 수정필요 (ERRORTRANS)

// [#632] NH KSK 2010.03.29 Index 중복되는 Bug Fix
#define	_MEM_VAR_APP_DISPENSEDAMT				_VAR_APP_NO+8	// _T("DispensedAmount")		- 수정필요 (ERRORTRANS)
#define	_MEM_VAR_APP_SURCHARGEAMT				_VAR_APP_NO+9	// _T("SurchargeAmount")
#define	_MEM_VAR_APP_SURCHARGEFLAG				_VAR_APP_NO+10	// _T("SurchargeFlag")			- 수정필요(검토필요)
#define	_MEM_VAR_APP_RETREFNO					_VAR_APP_NO+11	// _T("RetrievalRefNo")			- 수정필요 (ERRORTRANS)
#define	_MEM_VAR_APP_TOTALSTARTTIME				_VAR_APP_NO+12	// _T("TotalStartTime")	
#define	_MEM_VAR_APP_LASTADDCASHTIME			_VAR_APP_NO+13	// _T("LastAddCashTime")		- 수정필요 (검토필요)

/////////////////////////////////////////////////////////////////////////////
//  TRANS Error Print & MDB Save 용 Registry 저당 데이터 (ERRORTRANS)
/////////////////////////////////////////////////////////////////////////////
#define	_MEM_VAR_APP_TRANSTYPE					_VAR_APP_NO+14	// _T("TransType")
#define	_MEM_VAR_APP_TRANSFROMACCOUNT			_VAR_APP_NO+15	// _T("TransFromAccount")
#define	_MEM_VAR_APP_TRANSTOACCOUNT				_VAR_APP_NO+16	// _T("TransToAccount")
#define	_MEM_VAR_APP_TRANSBANKCODE				_VAR_APP_NO+17	// _T("TransBankCode")
#define	_MEM_VAR_APP_TRANSBANKACCOUNT			_VAR_APP_NO+18	// _T("TransBankAccount")
#define	_MEM_VAR_APP_TRANSHOSTDATE				_VAR_APP_NO+19	// _T("TransHostDate")
#define	_MEM_VAR_APP_TRANSHOSTTIME				_VAR_APP_NO+20	// _T("TransHostTime")
#define	_MEM_VAR_APP_TRANSAVAILBALANCE			_VAR_APP_NO+21	// _T("TransAvailBalance")
#define	_MEM_VAR_APP_TRANSRETRIEVALNUM			_VAR_APP_NO+22	// _T("TransRetrievalNum")
#define	_MEM_VAR_APP_TRANSTRACEAUDITNUM			_VAR_APP_NO+23	// _T("TransTraceAuditNum")
#define	_MEM_VAR_APP_TRANSHOSTNETID				_VAR_APP_NO+24	// _T("TransHostNetID")
#define	_MEM_VAR_APP_TRANSSETTLEDATE			_VAR_APP_NO+25	// _T("TransHostSettleDate")
#define	_MEM_VAR_APP_TRANSREQAMOUNT				_VAR_APP_NO+26	// _T("TransRequestedAmount"
#define	_MEM_VAR_APP_TRANSDISPAMOUNT			_VAR_APP_NO+27	// _T("TransDispensedAmount"
#define	_MEM_VAR_APP_TRANSREMAINAMOUNT			_VAR_APP_NO+28	// _T("TransRemainAmount")
#define	_MEM_VAR_APP_TRANSPROCCOUNT				_VAR_APP_NO+29	// _T("TransProcCount")
#define	_MEM_VAR_APP_TRANSRESULT				_VAR_APP_NO+30	// _T("TransResult")
#define	_MEM_VAR_APP_TRANSCARDDATA				_VAR_APP_NO+31	// _T("TransCardData")
#define	_MEM_VAR_APP_TRANSNONCASHVALUE			_VAR_APP_NO+32	// _T("TransNonCashValue")
#define	_MEM_VAR_APP_TRANSNONCASHTYPE			_VAR_APP_NO+33	// _T("TransNonCashType")
#define	_MEM_VAR_APP_TRANSOTHERMSG				_VAR_APP_NO+34	// _T("TransOtherMessage")
#define	_MEM_VAR_APP_TRANSTRACK1				_VAR_APP_NO+35	// _T("TransTrack1Data")
#define	_MEM_VAR_APP_TRANSTRACK2				_VAR_APP_NO+36	// _T("TransTrack2Data")
#define	_MEM_VAR_APP_TRANSTRACK3				_VAR_APP_NO+37	// _T("TransTrack3Data")

/////////////////////////////////////////////////////////////////////////////
// FOR _MEM_FLD_APP_TRANS
// Host Command Save for each Codes
/////////////////////////////////////////////////////////////////////////////
#define	_MEM_VAR_APP_HC15DetailStatus			_VAR_APP_NO+38	// _T("DetailStatus")

/////////////////////////////////////////////////////////////////////////////
//	AP/SP Profile Variable Define
/////////////////////////////////////////////////////////////////////////////

// [ATMINFO]
#define _MEM_VAR_APP_ATMSERIALNO 				_VAR_APP_NO+39	// _T("ATMSerialNum")	KSK 2009.12.23 사용 안함
#define _MEM_VAR_APP_MSGTYPE					_VAR_APP_NO+40	// _T("SELECTHOST")
#define _MEM_VAR_APP_ENGMODE					_VAR_APP_NO+41	// _T("EnglishMode")
#define _MEM_VAR_APP_SPNMODE					_VAR_APP_NO+42	// _T("SpanishMode")
#define _MEM_VAR_APP_FRNMODE					_VAR_APP_NO+43	// _T("FrenchMode")
#define _MEM_VAR_APP_CHNMODE					_VAR_APP_NO+44	// _T("ChineseMode")
#define _MEM_VAR_APP_KORMODE					_VAR_APP_NO+45	// _T("KoreanMode")
#define _MEM_VAR_APP_JAPMODE					_VAR_APP_NO+46	// _T("JapaneseMode")
#define _MEM_VAR_APP_ISO1_FLAG					_VAR_APP_NO+47	// _T("CardIso1")				- RMS만 사용
#define _MEM_VAR_APP_ISO2_FLAG					_VAR_APP_NO+48	// _T("CardIso2")				- RMS만 사용
#define _MEM_VAR_APP_ISO3_FLAG					_VAR_APP_NO+49	// _T("CardIso3")				- RMS만 사용
#define _MEM_VAR_APP_ISO1DATA					_VAR_APP_NO+50	// _T("CardIso1")
#define _MEM_VAR_APP_ISO2DATA					_VAR_APP_NO+51	// _T("CardIso2")
#define _MEM_VAR_APP_ISO3DATA					_VAR_APP_NO+52	// _T("CardIso3")
#define _MEM_VAR_APP_DISPENSELIMIT				_VAR_APP_NO+53	// _T("DispenseLimit")
#define _MEM_VAR_APP_LOWCRCYCHECK				_VAR_APP_NO+54	// _T("LowCurrencyCheck")
#define _MEM_VAR_APP_ROUTINGID					_VAR_APP_NO+55	// _T("RoutingID")
#define _MEM_VAR_APP_TERMINALID					_VAR_APP_NO+56	// _T("TerminalId")
#define	_MEM_VAR_APP_SERVICEPW					_VAR_APP_NO+57	// WORD	ServicePassWord;
#define	_MEM_VAR_APP_OPERATORPW					_VAR_APP_NO+58	// WORD	OperatorPassWord;
#define	_MEM_VAR_APP_MASTERPW					_VAR_APP_NO+59	// WORD	MasterPassWord;
#define _MEM_VAR_APP_SURCHARGEENABLE			_VAR_APP_NO+60	// _T("SurchargeEnable")
#define _MEM_VAR_APP_SURCHAREAMT				_VAR_APP_NO+61	// _T("SurchargeAmount")
#define _MEM_VAR_APP_OPSURCHARGEOWNER			_VAR_APP_NO+62	// _T("SurchargeOwner")
// [VOLUME]
#define	_MEM_VAR_APP_SEQUENCENO					_VAR_APP_NO+63	// _T("SEQUENCENO")
#define _MEM_VAR_APP_SURCHARGEDISPLAY			_VAR_APP_NO+64	// _T("SurchargeDisplay")
#define _MEM_VAR_APP_RECEIPT_HEADER1			_VAR_APP_NO+65	// _T("RECEIPT_H1")
#define _MEM_VAR_APP_RECEIPT_HEADER2			_VAR_APP_NO+66	// _T("RECEIPT_H2")
#define _MEM_VAR_APP_RECEIPT_HEADER3			_VAR_APP_NO+67	// _T("RECEIPT_TAIL")
#define _MEM_VAR_APP_RECEIPT_ADDRESS1			_VAR_APP_NO+68	// _T("Receipt_Address1")
#define _MEM_VAR_APP_RECEIPT_ADDRESS2			_VAR_APP_NO+69	// _T("Receipt_Address2")
#define _MEM_VAR_APP_RECEIPT_ADDRESS3			_VAR_APP_NO+70	// _T("Receipt_Address3")
#define _MEM_VAR_APP_RECEIPT_PHONENO			_VAR_APP_NO+71	// _T("Receipt_PhoneNumber")
#define _MEM_VAR_APP_ATMSTATUS					_VAR_APP_NO+72 // _T("ATMSTATUS")
#define _MEM_VAR_APP_SELECTRECEIPT				_VAR_APP_NO+73	// _T("SelectReceipt")	// [#354] CA PSC 2008.07.02 Receipt Select Option 처리

// [LOCAL_MESSAGE]
#define _MEM_VAR_APP_LC_HEALTH					_VAR_APP_NO+74	// _T("HEALTH")
#define _MEM_VAR_APP_LC_CONFIG					_VAR_APP_NO+75	// _T("CONFIGURATION")
#define _MEM_VAR_APP_LC_TOTAL					_VAR_APP_NO+76	// _T("TOTAL")
#define _MEM_VAR_APP_LC_WITHDRAWAL				_VAR_APP_NO+77	// _T("WITHDRAWAL")
#define _MEM_VAR_APP_LC_INQUIRY					_VAR_APP_NO+78	// _T("INQUIRY")
#define _MEM_VAR_APP_LC_TRANSFER				_VAR_APP_NO+79	// _T("TRANSFER")
#define _MEM_VAR_APP_LC_DEPOSIT					_VAR_APP_NO+80	// _T("DEPOSIT")
#define _MEM_VAR_APP_LC_REVERSAL				_VAR_APP_NO+81	// _T("REVERSAL")

#define _MEM_VAR_APP_FASTCASHDUMMY				_VAR_APP_NO+82	// _T("FastCashDummy")	// [#268] [NH] KSK 2008.6.9	Index 위치 수정
#define _MEM_VAR_APP_FASTCASH1					_VAR_APP_NO+83	// _T("FastCash1")		// [#268] [NH] KSK 2008.6.9	Index 위치 수정
#define _MEM_VAR_APP_FASTCASH2					_VAR_APP_NO+84	// _T("FastCash2")		// [#268] [NH] KSK 2008.6.9	Index 위치 수정
#define _MEM_VAR_APP_FASTCASH3					_VAR_APP_NO+85	// _T("FastCash3")		// [#268] [NH] KSK 2008.6.9	Index 위치 수정
#define _MEM_VAR_APP_FASTCASH4					_VAR_APP_NO+86	// _T("FastCash4")		// [#268] [NH] KSK 2008.6.9	Index 위치 수정
#define _MEM_VAR_APP_FASTCASH5					_VAR_APP_NO+87	// _T("FastCash5")		// [#268] [NH] KSK 2008.6.9	Index 위치 수정
#define _MEM_VAR_APP_FASTCASH6					_VAR_APP_NO+88	// _T("FastCash6")		// [#268] [NH] KSK 2008.6.9	Index 위치 수정

#define _MEM_VAR_APP_CALC_REQ_MXDOLLAR			_VAR_APP_NO+89	// [#519] [NH] KSK 2009.3.10 // [#2137] MX PCS 2012.07.17 환율 삭제 후 호환위해 남겨둠.
#define _MEM_VAR_APP_CALC_DISP_MXDOLLAR			_VAR_APP_NO+90	// [#519] [NH] KSK 2009.3.10 // [#2137] MX PCS 2012.07.17 환율 삭제 후 호환위해 남겨둠.
#define _MEM_VAR_APP_TRANS_DISP_COUNT			_VAR_APP_NO+91	// KSK 2009.3.30
#define _MEM_VAR_APP_TRANS_REJ_COUNT			_VAR_APP_NO+92	// KSK 2009.3.30

// [#127] KSK 2008.04.21
// AP/RMS/MWI/VDM/OS Version 영역 추가
#define _MEM_VAR_APP_AP_VERSION					_VAR_APP_NO+93	// AP  VERSION INFO
#define _MEM_VAR_APP_RMS_VERSION				_VAR_APP_NO+94	// RMS VERSION INFO
#define _MEM_VAR_APP_MWI_VERSION				_VAR_APP_NO+95	// MWI VERSION INFO
#define _MEM_VAR_APP_VDM_VERSION				_VAR_APP_NO+96	// VDM VERSION INFO
#define _MEM_VAR_APP_OS_VERSION					_VAR_APP_NO+97	// OS  VERSION INFO
// end of [#127]

// [#631] MX KSK 2010.03.29 멕시코 Field 추가 (BANK NAME & BANK FEE)
#define _MEM_VAR_APP_BANK_NAME					_VAR_APP_NO+98
#define _MEM_VAR_APP_LOYALTY_FEE				_VAR_APP_NO+99
#define _MEM_VAR_APP_LINE_USAGE_FEE				_VAR_APP_NO+100
#define _MEM_VAR_APP_IVATAX						_VAR_APP_NO+101
// end of [#631]

#define _MEM_VAR_APP_RETURNED_ACCNT_NO			_VAR_APP_NO+102		// [#2015] MX KSK 2012.02.02 멕시코 '&' 필드 수신시 RMS저널의 Other메시지에 추가

#define _MEM_VAR_APP_LOG_VERSION				_VAR_APP_NO+103		// [#2137] MX PCS 2012.07.19 "WinCE5.0과 맞춤."

#define	_MEM_VAR_APP_TRANSACTIONCNT_FLAG		_VAR_APP_NO+104		// [#2137] MX PCS 2012.07.19 "WinCE5.0과 맞춤."
#define	_MEM_VAR_APP_CUSTOMER_TYPE				_VAR_APP_NO+105		// [#2137] MX PCS 2012.07.19 "WinCE5.0과 맞춤."
#define	_MEM_VAR_APP_EXCHANGE_FEE				_VAR_APP_NO+106		// [#2137] MX PCS 2012.07.19 "WinCE5.0과 맞춤."
//#define	_MEM_VAR_APP_EXCHANGE_TOTAL_AMOUNT		_VAR_APP_NO+107		// [#2137] MX PCS 2012.07.19 "WinCE5.0과 맞춤."
#define	_MEM_VAR_APP_TOTAL_FEE					_VAR_APP_NO+107		// [#2142] MX KMK 2012.08.16 "WinCE5.0과 맞춤."
#define	_MEM_VAR_APP_EXCHANGE_RATE				_VAR_APP_NO+108		// [#2137] MX PCS 2012.07.19 "WinCE5.0과 맞춤."
#define	_MEM_VAR_APP_REVERSAL_EXCHANGE_FEE		_VAR_APP_NO+109		// [#2147] MX KSK 2012.08.18



// [BINLIST]
// [#24] KSK 2008.03.11 다른 DEFINE은 INDEX가 무의미 하지만 BINLIST는 INDEX가 유효한 값임
// 3501 ~ 부터 시작해야하고 다른쪽에서도 DEFINE시 3500번 INDEX는 사용하지 말아야함.
#define _MEM_VAR_APP_SETBINLIST					_VAR_APP_NO+2501	// _T("SetBinList")
#define _MEM_VAR_APP_BINLIST					_VAR_APP_NO+2503	// _T("BinList") 503 ~ 602
// end of [#24]

/////////////////////////////////////////////////////////////////////////////
//	WITHDRAWAL SUM INFO
/////////////////////////////////////////////////////////////////////////////
#define _MEM_VAR_SUM_WITH_AMOUNT				_VAR_SUM_NO		// WITHDRAWALINFO
#define _MEM_VAR_SUM_WITH_COUNT					_VAR_SUM_NO+10	// WITHDRAWALINFO

#define _MEM_VAR_SUM_INQ_AMOUNT					_VAR_SUM_NO+20	// INQUIRY INFO
#define _MEM_VAR_SUM_INQ_COUNT					_VAR_SUM_NO+30	// INQUIRY INFO

#define _MEM_VAR_SUM_TRANS_AMOUNT				_VAR_SUM_NO+40	// TRANSFER INFO
#define _MEM_VAR_SUM_TRANS_COUNT				_VAR_SUM_NO+50	// TRANSFER INFO

// [#2014] NH KJW 2011.02.23
#define _MEM_VAR_SUM_deprecated_lottery1			_VAR_SUM_NO+60	// LOTTERYINFO [deprecated]
#define _MEM_VAR_SUM_deprecated_lottery2			_VAR_SUM_NO+70	// LOTTERYINFO [deprecated]
// end of [#2014]

// [#2185] US Justin 2013.05.14 Dual Host DCC
#define _MEM_VAR_SUM_DHDCC_AMOUNT				_VAR_SUM_NO+80	// DUALHOSTDCCINFO
#define _MEM_VAR_SUM_DHDCC_COUNT				_VAR_SUM_NO+90	// DUALHOSTDCCINFO
// end of [#2185]

/////////////////////////////////////////////////////////////////////////////
//	VERSION INFORMATION
/////////////////////////////////////////////////////////////////////////////
#define _MEMKEY_SPVERSION						_VAR_VERSION_NO
#define _MEMKEY_EPVERSION						_VAR_VERSION_NO + 1

/////////////////////////////////////////////////////////////////////////////
//	ADVERTISEMENT INFORMATION
/////////////////////////////////////////////////////////////////////////////
#define _MEM_VAR_ADV_ENDISFLAG1					_VAR_ADVINFO_NO
#define _MEM_VAR_ADV_ENDISFLAG2					_VAR_ADVINFO_NO + 1
#define _MEM_VAR_ADV_ENDISFLAG3					_VAR_ADVINFO_NO + 2
#define _MEM_VAR_ADV_ENDISFLAG4					_VAR_ADVINFO_NO + 3
#define _MEM_VAR_ADV_ENDISFLAG5					_VAR_ADVINFO_NO + 4
#define _MEM_VAR_ADV_ENDISFLAG6					_VAR_ADVINFO_NO + 5
#define _MEM_VAR_COUPON1_ENDISFLAG				_VAR_ADVINFO_NO + 6
#define _MEM_VAR_COUPON2_ENDISFLAG				_VAR_ADVINFO_NO + 7
#define _MEM_VAR_COUPON3_ENDISFLAG              _VAR_ADVINFO_NO + 8
#define _MEM_VAR_COUPON4_ENDISFLAG              _VAR_ADVINFO_NO + 9
#define _MEM_VAR_COUPON5_ENDISFLAG              _VAR_ADVINFO_NO + 10
#define _MEM_VAR_COUPON6_ENDISFLAG              _VAR_ADVINFO_NO + 11
#define _MEM_VAR_ADV_TITLE1                     _VAR_ADVINFO_NO + 12
#define	_MEM_VAR_COUPON1_TEXT1					_VAR_ADVINFO_NO + 13
#define	_MEM_VAR_COUPON1_TEXT2					_VAR_ADVINFO_NO + 14
#define _MEM_VAR_ADV_TITLE2						_VAR_ADVINFO_NO + 15
#define	_MEM_VAR_COUPON2_TEXT1					_VAR_ADVINFO_NO + 16
#define	_MEM_VAR_COUPON2_TEXT2					_VAR_ADVINFO_NO + 17
#define _MEM_VAR_ADV_TITLE3						_VAR_ADVINFO_NO + 18
#define	_MEM_VAR_COUPON3_TEXT1					_VAR_ADVINFO_NO + 19
#define	_MEM_VAR_COUPON3_TEXT2					_VAR_ADVINFO_NO + 20
#define _MEM_VAR_ADV_TITLE4						_VAR_ADVINFO_NO + 21
#define	_MEM_VAR_COUPON4_TEXT1					_VAR_ADVINFO_NO + 22
#define	_MEM_VAR_COUPON4_TEXT2					_VAR_ADVINFO_NO + 23
#define _MEM_VAR_ADV_TITLE5						_VAR_ADVINFO_NO + 24
#define	_MEM_VAR_COUPON5_TEXT1					_VAR_ADVINFO_NO + 25
#define	_MEM_VAR_COUPON5_TEXT2					_VAR_ADVINFO_NO + 26
#define _MEM_VAR_ADV_TITLE6						_VAR_ADVINFO_NO + 27
#define	_MEM_VAR_COUPON6_TEXT1					_VAR_ADVINFO_NO + 28
#define	_MEM_VAR_COUPON6_TEXT2					_VAR_ADVINFO_NO + 29
#define	_MEM_VAR_ADV_GUIDEDISP_TIME				_VAR_ADVINFO_NO + 30
#define _MEM_VAR_TRAN_ADV_DISP_TIME				_VAR_ADVINFO_NO + 31
#define _MEM_VAR_TRAN_ADV_ENDISFLAG1			_VAR_ADVINFO_NO + 32
#define _MEM_VAR_TRAN_ADV_ENDISFLAG2			_VAR_ADVINFO_NO + 33
#define _MEM_VAR_TRAN_ADV_ENDISFLAG3			_VAR_ADVINFO_NO + 34
#define _MEM_VAR_TRAN_ADV_ENDISFLAG4			_VAR_ADVINFO_NO + 35
#define _MEM_VAR_TRAN_ADV_ENDISFLAG5			_VAR_ADVINFO_NO + 36
#define _MEM_VAR_TRAN_ADV_ENDISFLAG6			_VAR_ADVINFO_NO + 37
#define _MEM_VAR_BACK_CHANGE_ENDISFLAG			_VAR_ADVINFO_NO + 38
#define _MEM_VAR_BACK_DEFAULT_TYPE				_VAR_ADVINFO_NO + 39
#define _MEM_VAR_BACK_ENDISFLAG_TYPE1			_VAR_ADVINFO_NO + 40
#define _MEM_VAR_BACK_ENDISFLAG_TYPE2			_VAR_ADVINFO_NO + 41
#define _MEM_VAR_BACK_ENDISFLAG_TYPE3			_VAR_ADVINFO_NO + 42
#define _MEM_VAR_BACK_ENDISFLAG_TYPE4			_VAR_ADVINFO_NO + 43
#define _MEM_VAR_BACK_ENDISFLAG_TYPE5			_VAR_ADVINFO_NO + 44
#define _MEM_VAR_BACK_ENDISFLAG_TYPE6			_VAR_ADVINFO_NO + 45

/////////////////////////////////////////////////////////////////////////////
//	TCPIP INFORMATION
/////////////////////////////////////////////////////////////////////////////
#define _MEM_VAR_TCPIP_ATM_IPADDR				_VAR_TCPIPINFO_NO
#define _MEM_VAR_TCPIP_ATM_SUBNET				_VAR_TCPIPINFO_NO + 1
#define	_MEM_VAR_TCPIP_ATM_GATEWAY				_VAR_TCPIPINFO_NO + 2
#define	_MEM_VAR_TCPIP_ATM_DNS					_VAR_TCPIPINFO_NO + 3
#define _MEM_VAR_TCPIP_ATM_DHCP					_VAR_TCPIPINFO_NO + 4
#define	_MEM_VAR_TCPIP_ISHOSTUSEURL				_VAR_TCPIPINFO_NO + 5
#define	_MEM_VAR_TCPIP_HOST1NAME				_VAR_TCPIPINFO_NO + 6
#define _MEM_VAR_TCPIP_HOST1PORTNO				_VAR_TCPIPINFO_NO + 7
#define	_MEM_VAR_TCPIP_HOST2NAME				_VAR_TCPIPINFO_NO + 8
#define	_MEM_VAR_TCPIP_HOST2PORTNO				_VAR_TCPIPINFO_NO + 9
#define _MEM_VAR_TCPIP_NETWORKTYPE				_VAR_TCPIPINFO_NO + 10
#define _MEM_VAR_TCPIP_SSLENABLE				_VAR_TCPIPINFO_NO + 11
#define _MEM_VAR_TCPIP_TYPE						_VAR_TCPIPINFO_NO + 12
#define _MEM_VAR_TCPIP_CRCENABLE				_VAR_TCPIPINFO_NO + 13
#define _MEM_VAR_TCPIP_ISRMSUSEURL				_VAR_TCPIPINFO_NO + 14
#define _MEM_VAR_TCPIP_RMSNAME					_VAR_TCPIPINFO_NO + 15
#define _MEM_VAR_TCPIP_RMSPORTNO				_VAR_TCPIPINFO_NO + 16
#define	_MEM_VAR_TCPIP_RMSSVRPORTNO				_VAR_TCPIPINFO_NO + 17
#define	_MEM_VAR_TCPIP_RMSCONNECTMODE			_VAR_TCPIPINFO_NO + 18
#define	_MEM_VAR_TCPIP_RMSSENDINTERVAL			_VAR_TCPIPINFO_NO + 19
//#define _MEM_VAR_TCPIP_SSLVERSION				_VAR_TCPIPINFO_NO + 20		// KSK 2010.06.16 WINCE5.0 적용이 불가하여 위치 수정함 (OPT2영역으로)

// [#4] NH PSC 2008.03.10 NVRam이 정리 되기전까지 기타 옵션은 Optional setting section에 추가
/////////////////////////////////////////////////////////////////////////////
//	OPTIONAL SETTING
/////////////////////////////////////////////////////////////////////////////
#define _MEM_VAR_OPT_REVERSAL_RETRY				_VAR_OPT_NO
#define _MEM_VAR_OPT_NETFLOW_STATE				_VAR_OPT_NO + 1
#define _MEM_VAR_OPT_NETFLOW_TRANCODE			_VAR_OPT_NO + 2
#define _MEM_VAR_OPT_MOD10_ENABLE				_VAR_OPT_NO + 3
#define _MEM_VAR_OPT_APP_DESKEY5				_VAR_OPT_NO + 4
#define _MEM_VAR_OPT_APP_DESKEY6				_VAR_OPT_NO + 5
#define _MEM_VAR_OPT_CASSETTETOTAL_STARTTIME	_VAR_OPT_NO + 6
#define _MEM_VAR_OPT_APP_ISSUERFEE				_VAR_OPT_NO + 7			// 'k' field : Issuer fee 추가 PSC 2008.04.04
#define _MEM_VAR_OPT_REASONFORREVERSAL			_VAR_OPT_NO + 8
//#define _MEM_VAR_OPT_ADVERTISEMENT_TIMER		_VAR_OPT_NO + 9			// [#14] NH AIREAT 2008.03.26  광고 Timer 추가. -- > _MEM_VAR_ADV_GUIDEDISP_TIME로 대체
//	[#59] NH KGS 2008.03.27 NVRAM for Accounts, NUA, Welcome Message
#define _MEM_VAR_OPT_ACCOUNT_CHECKING			_VAR_OPT_NO + 9			
#define _MEM_VAR_OPT_ACCOUNT_SAVINGS			_VAR_OPT_NO + 10		
#define _MEM_VAR_OPT_ACCOUNT_CREDITCARD			_VAR_OPT_NO + 11		
#define _MEM_VAR_OPT_NETWORKUSERADDRESS			_VAR_OPT_NO + 12		// UK에서만 사용하나 UK Project삭제로 인해 사용 안함		
#define _MEM_VAR_OPT_WELCOME_MESSAGE1			_VAR_OPT_NO + 13		
#define _MEM_VAR_OPT_WELCOME_MESSAGE2			_VAR_OPT_NO + 14		
#define _MEM_VAR_OPT_WELCOME_MESSAGE3			_VAR_OPT_NO + 15		
#define _MEM_VAR_OPT_WELCOME_MESSAGE4			_VAR_OPT_NO + 16		// [#192] UK AIREAT 2008.05.16
// end of [#59]
#define _MEM_VAR_OPT_PREBALANCE					_VAR_OPT_NO + 17	// [#94] US KGS 2008.04.10 Pre Balance Menu
// [#112] US KGS 2008.04.15 Percent Surcharge Menu
#define _MEM_VAR_OPT_PERSURCHARGE_ENABLE		_VAR_OPT_NO + 18	
#define _MEM_VAR_OPT_PERSURCHARGE_MANNER		_VAR_OPT_NO + 19	
#define _MEM_VAR_OPT_PERSURCHARGE_PERCENT		_VAR_OPT_NO + 20	
// end of [#112]
// [#239] MX KSK 2008.5.27
#define	_MEM_VAR_OPT_CONTINUETRANS_ENABLE		_VAR_OPT_NO + 21
//#define	_MEM_VAR_OPT_WITHDRAWAL_SURCHARGE		_VAR_OPT_NO + 22	// [#2146] MX KSK 2012.08.22
//#define _MEM_VAR_OPT_BALANCE_SURCHARGE			_VAR_OPT_NO + 23	// [#2146] MX KSK 2012.08.22
//#define	_MEM_VAR_OPT_PINCHANGE_SURCHARGE		_VAR_OPT_NO + 24	// [#2146] MX KSK 2012.08.22
#define	_MEM_VAR_OPT_DEFAULT_LANGUAGE			_VAR_OPT_NO + 25
// end of [#239]
// [#337] AU AIREAT 2008.06.25
#define _MEM_VAR_OPT_STORE_MESSAGE1				_VAR_OPT_NO + 26
#define _MEM_VAR_OPT_STORE_MESSAGE2				_VAR_OPT_NO + 27
#define _MEM_VAR_OPT_STORE_MESSAGE3				_VAR_OPT_NO + 28
#define _MEM_VAR_OPT_STORE_MESSAGE4				_VAR_OPT_NO + 29
#define _MEM_VAR_OPT_MARKETING_MESSAGE1			_VAR_OPT_NO + 30
#define _MEM_VAR_OPT_MARKETING_MESSAGE2			_VAR_OPT_NO + 31
#define _MEM_VAR_OPT_MARKETING_MESSAGE3			_VAR_OPT_NO + 32
#define _MEM_VAR_OPT_MARKETING_MESSAGE4			_VAR_OPT_NO + 33
#define _MEM_VAR_OPT_EXIT_MESSAGE1				_VAR_OPT_NO + 34
#define _MEM_VAR_OPT_EXIT_MESSAGE2				_VAR_OPT_NO + 35
#define _MEM_VAR_OPT_EXIT_MESSAGE3				_VAR_OPT_NO + 36
#define _MEM_VAR_OPT_PROCESSOR_MESSAGE1			_VAR_OPT_NO + 37
#define _MEM_VAR_OPT_PROCESSOR_MESSAGE2			_VAR_OPT_NO + 38
#define _MEM_VAR_OPT_PROCESSOR_MESSAGE3			_VAR_OPT_NO + 39
#define _MEM_VAR_OPT_PROCESSOR_MESSAGE4			_VAR_OPT_NO + 40
#define _MEM_VAR_OPT_INQUIRY_SURCHARGE			_VAR_OPT_NO + 41
// end of [#337]

#define	_MEM_VAR_OPT_PINOPTION_ENABLE			_VAR_OPT_NO + 43		// [#366] [NH] KSK 2008.7.15

/**
 * 
 * 2019-12-31 
 * Check Cashing app is deprecated
 * [_VAR_OPT_NO + 43, _VAR_OPT_NO + 46]
 */

#define _MEM_VAR_OPT_EMV_AVAILABLE				_VAR_OPT_NO + 47		// [#397] [NH] psc 2008.08.01
#define _MEM_VAR_OPT_EMV_ENABLE					_VAR_OPT_NO + 48		// [#397] [NH] psc 2008.08.01
#define _MEM_VAR_OPT_SURCHARGE_WARNINGLOCATION	_VAR_OPT_NO + 49
#define _MEM_VAR_OPT_EMV_TRANSACTIONOPTION		_VAR_OPT_NO + 50		// [#397] [NH] psc 2008.08.21
// Deprecate 2019-12-31: Check Cashing: _VAR_OPT_NO + 51
#define _MEM_VAR_OPT_AUTODAYTOTAL_ENABLE		_VAR_OPT_NO + 52		// [#416] [NZ] AIREAT 2008.09.08
#define _MEM_VAR_OPT_AUTODAYTOTAL_TYPE			_VAR_OPT_NO + 53		// [#416] [NZ] AIREAT 2008.09.08
#define _MEM_VAR_OPT_AUTODAYTOTAL_TIME_YEAR		_VAR_OPT_NO + 54		// [#416] [NZ] AIREAT 2008.09.08
#define _MEM_VAR_OPT_AUTODAYTOTAL_TIME_MONTH	_VAR_OPT_NO + 55		// [#416] [NZ] AIREAT 2008.09.08
#define _MEM_VAR_OPT_AUTODAYTOTAL_TIME_DAY		_VAR_OPT_NO + 56		// [#416] [NZ] AIREAT 2008.09.08
#define _MEM_VAR_OPT_AUTODAYTOTAL_TIME_HOUR		_VAR_OPT_NO + 57		// [#416] [NZ] AIREAT 2008.09.08
#define _MEM_VAR_OPT_AUTODAYTOTAL_TIME_MIN		_VAR_OPT_NO + 58		// [#416] [NZ] AIREAT 2008.09.08
#define _MEM_VAR_OPT_DAYTOTAL_LASTDATE			_VAR_OPT_NO + 59		// [#416] [NZ] AIREAT 2008.09.08

/////////////////////////////////////////////////////////////////////////////
//	EMV INFORMATION
/////////////////////////////////////////////////////////////////////////////
//#define _MEM_VAR_EMV_SERIALNUMBER					_VAR_EMVINFO_NO		// 사용 안함
#define _MEM_VAR_EMV_APPLICATION_ID					_VAR_EMVINFO_NO + 1 // [#68] UK HWANG 2008.04.02 EMV Level2

// [#68] UK HWANG 2008.04.02 EMV Level2
//////////////////////////////////////////////////////////////////////////
// EMV REVERSAL INFORMATION
//////////////////////////////////////////////////////////////////////////
#define _MEM_VAR_CRYPTOGRAM_INFO_DATA_TLV			_VAR_EMV_REVERSAL
#define _MEM_VAR_TERMINAL_VERIFY_RESULT_TLV			_VAR_EMV_REVERSAL + 1
#define _MEM_VAR_APP_CRYPTOGRAM_TLV					_VAR_EMV_REVERSAL + 2
#define _MEM_VAR_AMOUNT_AUTHORIZED_TLV				_VAR_EMV_REVERSAL + 3
#define _MEM_VAR_AMOUNT_OTHER_TLV					_VAR_EMV_REVERSAL + 4
#define _MEM_VAR_APPLICATION_ID_TLV					_VAR_EMV_REVERSAL + 5
#define _MEM_VAR_APP_INTERCHANGE_PRO_TLV			_VAR_EMV_REVERSAL + 6
#define _MEM_VAR_APP_TRAN_COUNTER_TLV				_VAR_EMV_REVERSAL + 7
#define _MEM_VAR_TERMINAL_COUNTRY_CODE_TLV			_VAR_EMV_REVERSAL + 8
#define _MEM_VAR_TRANSACTION_CURRENCY_CODE_TLV		_VAR_EMV_REVERSAL + 9
#define _MEM_VAR_TRANSACTION_DATE_TLV				_VAR_EMV_REVERSAL + 10
#define _MEM_VAR_TRANSACTION_TYPE_TLV				_VAR_EMV_REVERSAL + 11
#define _MEM_VAR_RANDOM_NUMBER_TLV					_VAR_EMV_REVERSAL + 12
#define _MEM_VAR_ISSUER_APP_DATA_TLV				_VAR_EMV_REVERSAL + 13
#define _MEM_VAR_IFD_SERIAL_NUMBER_TLV				_VAR_EMV_REVERSAL + 14
#define _MEM_VAR_ISSUER_SCRIPT_ID_TLV				_VAR_EMV_REVERSAL + 15
#define _MEM_VAR_ISSUER_SCRIPT_RESULTS_TLV			_VAR_EMV_REVERSAL + 16
#define _MEM_VAR_TRANSACTION_STATUS_INFO_TLV		_VAR_EMV_REVERSAL + 17
// KSK 2009.8.31 캐나다 Interac 최소 사양으로 인해 Field 추가
#define _MEM_VAR_APP_EXPIRED_DATE_TLV				_VAR_EMV_REVERSAL + 18
#define _MEM_VAR_APP_ACCOUNT_NUMBER_TLV				_VAR_EMV_REVERSAL + 19
#define _MEM_VAR_APP_ACCOUNT_NUMBER_SEQ_NUMBER_TLV	_VAR_EMV_REVERSAL + 20
#define _MEM_VAR_POS_ENTRY_MODE_TLV					_VAR_EMV_REVERSAL + 21
#define _MEM_VAR_TERMINAL_CAPABILITIES_TLV			_VAR_EMV_REVERSAL + 22
#define _MEM_VAR_TERMINAL_TYPE_TLV					_VAR_EMV_REVERSAL + 23
#define _MEM_VAR_TRACK2_EQU_DATA_TLV				_VAR_EMV_REVERSAL + 24
#define _MEM_VAR_TRANSACTION_SEQ_COUNTER_TLV		_VAR_EMV_REVERSAL + 25
// end of [#68]
#define _MEM_VAR_AC_REQ_9F26_VALUE					_VAR_EMV_REVERSAL + 26	// [#2115] MX KSK 2012.01.28
#define _MEM_VAR_EMV_APPLICATION_NAME				_VAR_EMV_REVERSAL + 27	// [#2252] US Justin 2014.02.07 Long AID and Name Bug Fix

#define _MEM_VAR_EMV_CONFIG_ENABLEDAID				_VAR_EMV_CONFIG	+ 0		// [#2342] US Justin 2015.05.07 AID Selection

// [#434] NH AIREAT 2008.10.14
/////////////////////////////////////////////////////////////////////////////
//	OPTIONAL SETTING 2
/////////////////////////////////////////////////////////////////////////////
#define _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT1		_VAR_OPT2_NO
#define _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT2		_VAR_OPT2_NO + 1
#define _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT3		_VAR_OPT2_NO + 2
// end of [#434]

#define	_MEM_VAR_OPT2_RMSMODEMINITIALSTRING			_VAR_OPT2_NO + 3		// [#468][NH] AIREAT 2008.12.19
#define _MEM_VAR_OPT2_MACHINEKIND					_VAR_OPT2_NO + 4		// [#476] [NH] KSK 2009.1.5

#define _MEM_VAR_OPT2_MULTI_CURRENCY_ENABLE			_VAR_OPT2_NO + 5		// [#514][MX] KSK 2009.3.4
//#define _MEM_VAR_OPT2_EXCHANGE_RATE					_VAR_OPT2_NO + 6		// [#514][MX] KSK 2009.3.4	// [#2137] MX KSK 2012.07.25
#define _MEM_VAR_OPT2_CST1_CURRENCYID				_VAR_OPT2_NO + 7		// [#514][MX] KSK 2009.3.11
#define _MEM_VAR_OPT2_CST2_CURRENCYID				_VAR_OPT2_NO + 8		// [#514][MX] KSK 2009.3.11
#define _MEM_VAR_OPT2_CST3_CURRENCYID				_VAR_OPT2_NO + 9		// [#514][MX] KSK 2009.3.11
#define _MEM_VAR_OPT2_CST4_CURRENCYID				_VAR_OPT2_NO + 10		// [#514][MX] KSK 2009.3.11

#define _MEM_VAR_OPT2_PREDIALENDISABLE				_VAR_OPT2_NO + 11		// [#525] US KSK 2009.05.21
#define _MEM_VAR_OPT2_PREDIALOPTION					_VAR_OPT2_NO + 12		// [#525] US KSK 2009.05.21

#define _MEM_VAR_OPT2_NEEDMORETIME					_VAR_OPT2_NO + 13		// [#528] AU AIREAT 2009.06.02

#define	_MEM_VAR_OPT2_AWARD_COUPON1_ENABLE			_VAR_OPT2_NO + 14		// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
#define	_MEM_VAR_OPT2_AWARD_COUPON2_ENABLE			_VAR_OPT2_NO + 15		// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
#define	_MEM_VAR_OPT2_AWARD_COUPON3_ENABLE			_VAR_OPT2_NO + 16		// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
#define	_MEM_VAR_OPT2_AWARD_COUPON4_ENABLE			_VAR_OPT2_NO + 17		// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
#define	_MEM_VAR_OPT2_AWARD_COUPON5_ENABLE			_VAR_OPT2_NO + 18		// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
#define	_MEM_VAR_OPT2_AWARD_COUPON6_ENABLE			_VAR_OPT2_NO + 19		// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함

#define _MEM_VAR_OPT2_TERMINAL_STATUS_FIELD_ENABLE	_VAR_OPT2_NO + 20		// [#558] KSK 2009.08.17
#define _MEM_VAR_OPT2_REVERSAL_AT_HOSTERROR			_VAR_OPT2_NO + 21	// [#558] KSK 2009.08,17
#define _MEM_VAR_OPT2_STANDARD1OPTION_AVAILABLE		_VAR_OPT2_NO + 22		// [#558] KSK 2009.08.17

#define _MEM_VAR_OPT2_PAPER_LOW_SENSOR_ENABLE		_VAR_OPT2_NO + 23		//[#596] SOOK 2009.12.11 SPR PAPER LOW SENSOR EN/DISABLE 기능 추가
#define _MEM_VAR_OPT2_EPP_FLICKER_OPTION			_VAR_OPT2_NO + 24		//[#604] NH KSK 2010.01.05
#define _MEM_VAR_OPT2_CST_SOUND_OPTION				_VAR_OPT2_NO + 25		//[#604] NH KSK 2010.01.05
#define _MEM_VAR_OPT2_EMV_LATCH_ENABLE				_VAR_OPT2_NO + 26		//[#605] NH KSK 2010.01.06
#define _MEM_VAR_OPT2_REASONFORREVERSAL_ENABLE		_VAR_OPT2_NO + 27		//[#606] NH KSK 2010.01.06

#define _MEM_VAR_OPT2_SCHEDULED_JOURNAL_ENABLE		_VAR_OPT2_NO + 28		//[#613] AU_C KSK 2010.01.18
#define _MEM_VAR_OPT2_SCHEDULED_JOURNAL_OPTION		_VAR_OPT2_NO + 29		//[#613] AU_C KSK 2010.01.18
#define _MEM_VAR_OPT2_SCHEDULED_JOURNAL_COUNT		_VAR_OPT2_NO + 30		//[#613] AU_C KSK 2010.01.18
#define _MEM_VAR_OPT2_SCHEDULED_JOURNAL_YEAR		_VAR_OPT2_NO + 31		//[#613] AU_C KSK 2010.01.18
#define _MEM_VAR_OPT2_SCHEDULED_JOURNAL_MONTH		_VAR_OPT2_NO + 32		//[#613] AU_C KSK 2010.01.18
#define _MEM_VAR_OPT2_SCHEDULED_JOURNAL_DAY			_VAR_OPT2_NO + 33		//[#613] AU_C KSK 2010.01.18
#define _MEM_VAR_OPT2_SCHEDULED_JOURNAL_HOUR		_VAR_OPT2_NO + 34		//[#613] AU_C KSK 2010.01.18
#define _MEM_VAR_OPT2_CONFIGURATION_ENABLE			_VAR_OPT2_NO + 35		//[#615] AU_C KSK 2010.01.18
#define _MEM_VAR_OPT2_SCHEDULED_JOURNAL_NEXT_DAY	_VAR_OPT2_NO + 36		//[#613] AU_C KSK 2010.02.01
#define _MEM_VAR_OPT2_SCHEDULED_AVAILABLE			_VAR_OPT2_NO + 37		//[#613] AU_C KSK 2010.02.01
#define _MEM_VAR_OPT2_AD_BRAND_NAME					_VAR_OPT2_NO + 38		//[#616] AU_C SOOK 2010.02.05
#define _MEM_VAR_OPT2_TRANSACTION_ADVER1_ENABLE		_VAR_OPT2_NO + 39		//[#616] AU_C SOOK 2010.02.05
#define _MEM_VAR_OPT2_TRANSACTION_ADVER2_ENABLE		_VAR_OPT2_NO + 40		//[#616] AU_C SOOK 2010.02.05
#define _MEM_VAR_OPT2_BANK_NAME_FEE_PRINT			_VAR_OPT2_NO + 41     // [#631] MX, 2010.03.29 JERRY
#define _MEM_VAR_OPT2_NOTICE_SERVICE_ENABLE			_VAR_OPT2_NO + 42
#define _MEM_VAR_OPT2_NOTICE_SERVICE_TITLE			_VAR_OPT2_NO + 43
#define _MEM_VAR_OPT2_NOTICE_SERVICE_MSG1			_VAR_OPT2_NO + 44
#define _MEM_VAR_OPT2_NOTICE_SERVICE_MSG2			_VAR_OPT2_NO + 45
#define _MEM_VAR_OPT2_NOTICE_SERVICE_MSG3			_VAR_OPT2_NO + 46
#define _MEM_VAR_OPT2_TOUCHVIBRATION_ENABLE			_VAR_OPT2_NO + 47
#define _MEM_VAR_OPT2_SSLVERSION					_VAR_OPT2_NO + 48		// KSK 2010.06.16 WINCE50 추가를 위해 위치 변경
//#define _MEM_VAR_OPT2_MX_DP_SELECTION				_VAR_OPT2_NO + 49		// WINCE5.0과 맞추기 위해 [#2146] MX KSK 2012.08.22
#define _MEM_VAR_OPT2_AU_COUNTRY_SELECTION			_VAR_OPT2_NO + 50		// [#2042] AU KSK 2011.04.02
#define _MEM_VAR_OPT2_CARDREADER_SELECTION			_VAR_OPT2_NO + 51		// [#2073] NH KSK 2011.06.24
#define _MEM_VAR_OPT2_ANTISKIMMING_ENABLE			_VAR_OPT2_NO + 52		// [#2074] NH KSK 2011.06.25
#define _MEM_VAR_OPT2_RKT_AVAILABLE					_VAR_OPT2_NO + 53		// [#2075] NH KSK 2011.06.27
#define _MEM_VAR_OPT2_RKT_ENABLE					_VAR_OPT2_NO + 54		// [#2075] NH KSK 2011.06.27
#define _MEM_VAR_OPT2_RKT_RANDOMNUMBER_ENABLE		_VAR_OPT2_NO + 55		// [#2075] NH KSK 2011.06.27
#define _MEM_VAR_OPT2_EJUPLOAD_ENABLE				_VAR_OPT2_NO + 56		// [#2076] NH KSK 2011.06.28
#define _MEM_VAR_OPT2_ANTISKIMMING_SHUTTER_ENABLE	_VAR_OPT2_NO + 57		// [#2074] NH KSK 2011.06.25
#define _MEM_VAR_OPT2_ANTISKIMMING_TIMETHRESHOLD	_VAR_OPT2_NO + 58		// [#2074] NH KSK 2011.06.25
#define _MEM_VAR_OPT2_ANTISKIMMING_AUTORECOVERYTIME	_VAR_OPT2_NO + 59		// [#2085] NH KSK 2011.07.22
#define _MEM_VAR_OPT2_PERSURCHARGE_IGNORE			_VAR_OPT2_NO + 60		// [#2092] US PCS 2011.10.24 'IGNORE CONFIG SURCAHRGE VALUE'
#define _MEM_VAR_OPT2_SSL_CERTIFY					_VAR_OPT2_NO + 61		// [#2093] NH PCS 2011.10.24 "SSL Certify 

/**
 * Deprecation notice: Lottery
 * 
 * 2019-12-31 
 * Lottery app is deprecated
 * [_VAR_OPT2_NO + 62, _VAR_OPT2_NO + 73]
 */

#define _MEM_VAR_OPT2_DF_ENABLE						_VAR_OPT2_NO + 74		// [#2150] US Justin 2012.09.27 Dynamic Flow En/Disable
#define _MEM_VAR_OPT2_DF_PARTNERID					_VAR_OPT2_NO + 75		// [#2150] US Justin 2012.09.27 Dynamic Flow Partner ID
#define _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE			_VAR_OPT2_NO + 76		// [#2150] US Justin 2012.09.27 Dynamic Flow Available Services (PIN Change => available without DF ?????)
#define _MEM_VAR_OPT2_DF_SERVICE_MPT				_VAR_OPT2_NO + 77		// [#2150] US Justin 2012.09.27 Dynamic Flow Available Services (Mobile Phone Top up => available without DF ?????)
#define _MEM_VAR_OPT2_DF_SERVICE_DCC				_VAR_OPT2_NO + 78		// [#2150] US Justin 2012.09.27 Dynamic Flow Available Services (DCC)
#define _MEM_VAR_OPT2_DF_DCC_DISCLAIMER1			_VAR_OPT2_NO + 79		// [#2150] US Justin 2012.10.08 Add DCC Disclaimer L1
#define _MEM_VAR_OPT2_DF_DCC_DISCLAIMER2			_VAR_OPT2_NO + 80		// [#2150] US Justin 2012.10.08 Add DCC Disclaimer L2
#define _MEM_VAR_OPT2_DF_DCC_DISCLAIMER3			_VAR_OPT2_NO + 81		// [#2150] US Justin 2012.10.08 Add DCC Disclaimer L3
#define _MEM_VAR_OPT2_SUPERVISOR_LANGUAGE			_VAR_OPT2_NO + 82		// [#2166] NH KSK 2012.11.13
#define _MEM_VAR_OPT2_RECEIPT_ON_SCREEN				_VAR_OPT2_NO + 83		// [#2183] US Justin 2013.04.05 Add Receipt on Screen Option

#define	_MEM_VAR_OPT2_DUALHOSTDCC_ENABLE			_VAR_OPT2_NO + 84		// [#2185] US Justin 2013.04.10 Dual Host DCC Service En/Disable
#define _MEM_VAR_OPT2_DUALHOSTDCC_SSL_ENABLE		_VAR_OPT2_NO + 85		// [#2185] US Justin 2013.04.10 Dual Host DCC SSL En/Disable
#define _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION		_VAR_OPT2_NO + 86		// [#2185] US Justin 2013.04.10 Dual Host DCC SSL Version
#define	_MEM_VAR_OPT2_DUALHOSTDCC_ISHOSTUSEURL		_VAR_OPT2_NO + 87		// [#2185] US Justin 2013.04.10 Dual Host DCC Host IP or URL
#define	_MEM_VAR_OPT2_DUALHOSTDCC_HOST1NAME			_VAR_OPT2_NO + 88		// [#2185] US Justin 2013.04.10 Dual Host DCC Host Ip Address 1
#define _MEM_VAR_OPT2_DUALHOSTDCC_HOST1PORTNO		_VAR_OPT2_NO + 89		// [#2185] US Justin 2013.04.10 Dual Host DCC Host Port 1
#define	_MEM_VAR_OPT2_DUALHOSTDCC_HOST2NAME			_VAR_OPT2_NO + 90		// [#2185] US Justin 2013.04.10 Dual Host DCC Host IP Address 2
#define	_MEM_VAR_OPT2_DUALHOSTDCC_HOST2PORTNO		_VAR_OPT2_NO + 91		// [#2185] US Justin 2013.04.10 Dual Host DCC Host Port 2
#define	_MEM_VAR_OPT2_DCC_CUSTOMOPTION				_VAR_OPT2_NO + 92		// [#2187] NH Justin 2013.04.18 DCC Custom Option
#define	_MEM_VAR_OPT2_DCC_DUALHOSTTAN				_VAR_OPT2_NO + 93		// [#2185] US Justin 2013.05.14 Dual Host Tran

// [#2202] NH Justin 2013.06.20 Add DCC DISCLAIMER (3 more lines)
#define _MEM_VAR_OPT2_DF_DCC_DISCLAIMER4			_VAR_OPT2_NO + 94		// [#2150] US Justin 2013.06.20 Add DCC Disclaimer L4
#define _MEM_VAR_OPT2_DF_DCC_DISCLAIMER5			_VAR_OPT2_NO + 95		// [#2150] US Justin 2013.06.20 Add DCC Disclaimer L5
#define _MEM_VAR_OPT2_DF_DCC_DISCLAIMER6			_VAR_OPT2_NO + 96		// [#2150] US Justin 2013.06.20 Add DCC Disclaimer L6
// End of [#2202]

// [#2205] US KSK 2013.06.28
#define _MEM_VAR_OPT2_HALO_LEDMODE_INSERVICE		_VAR_OPT2_NO + 97
#define _MEM_VAR_OPT2_HALO_LEDCOLOR_INSERVICE		_VAR_OPT2_NO + 98
#define _MEM_VAR_OPT2_HALO_LEDMODE_TRANSACTION		_VAR_OPT2_NO + 99
#define _MEM_VAR_OPT2_HALO_LEDCOLOR_TRANSACTION		_VAR_OPT2_NO + 100
#define _MEM_VAR_OPT2_HALO_LEDMODE_DISPENSING		_VAR_OPT2_NO + 101
#define _MEM_VAR_OPT2_HALO_LEDCOLOR_DISPENSING		_VAR_OPT2_NO + 102
#define _MEM_VAR_OPT2_HALO_LEDMODE_OUTOFSERVICE		_VAR_OPT2_NO + 103
#define _MEM_VAR_OPT2_HALO_LEDCOLOR_OUTOFSERVICE	_VAR_OPT2_NO + 104
#define _MEM_VAR_OPT2_HALO_LEDCOLOR_DEFAULT			_VAR_OPT2_NO + 105
// end of [#2205]

#define _MEM_VAR_OPT2_EMV_CONTINUE_FALLBACK			_VAR_OPT2_NO + 106		// [#2208] US Justin 2013.07.08 Add FallContinue Option (US Only)
#define _MEM_VAR_OPT2_EXCHANGEFEE_PERCENTAGE		_VAR_OPT2_NO + 107		// [#2210] MX Justin 2013.07.12 Exchange Fee Format (MX Only)
#define _MEM_VAR_OPT2_EMV_KERNELVERSION				_VAR_OPT2_NO + 108		// [#2226] NH Justin 2013.10.22 Add Kernel Version
#define _MEM_VAR_OPT2_EMV_AIDENDISABLE				_VAR_OPT2_NO + 109		// [#2226] NH Justin 2013.10.23 Selecting AID
#define _MEM_VAR_OPT2_DF_1STCALL_OPTION				_VAR_OPT2_NO + 110		// [#2242] NH Justin 2013.12.17 Dynamic Flow 1st call option

#define _MEM_VAR_OPT2_RMS_TIMEOUT_CONNECT_TCPIP		_VAR_OPT2_NO + 111		// [#2234] NH KMK 2013.11.21 MoniView TCP/IP Timeout (CONNECT)
#define _MEM_VAR_OPT2_RMS_TIMEOUT_RECEIVE_TCPIP		_VAR_OPT2_NO + 112		// [#2234] NH KMK 2013.11.21 MoniView TCP/IP Timeout (RECEIVE)

#define _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE	_VAR_OPT2_NO + 113		// [#2241] AU KMK 2013.12.16 CDU-M 논리매수 차감 Option
#define _MEM_VAR_OPT2_DIGITAL_RECEIPT_OPTION		_VAR_OPT2_NO + 114		// [#2263] CA Justin 2014.0505 Digital Receipt Option ******* CAUTION : REVERSED, 0=Enable(Default), other(1)=Disable....

#define _MEM_VAR_OPT2_CST1_INITIALCOUNT_AP			_VAR_OPT2_NO +115		// [#2270] AU KSK 2014.05.26 NegativeCount Option이 Enable시 Remain CST Count값을 표시하기 위함
#define _MEM_VAR_OPT2_CST2_INITIALCOUNT_AP			_VAR_OPT2_NO +116		// [#2270] AU KSK 2014.05.26 NegativeCount Option이 Enable시 Remain CST Count값을 표시하기 위함
#define _MEM_VAR_OPT2_CST3_INITIALCOUNT_AP			_VAR_OPT2_NO +117		// [#2270] AU KSK 2014.05.26 NegativeCount Option이 Enable시 Remain CST Count값을 표시하기 위함
#define _MEM_VAR_OPT2_CST4_INITIALCOUNT_AP			_VAR_OPT2_NO +118		// [#2270] AU KSK 2014.05.26 NegativeCount Option이 Enable시 Remain CST Count값을 표시하기 위함
#define _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP			_VAR_OPT2_NO +119		// [#2270] AU KSK 2014.05.26 NegativeCount Option이 Enable시 Remain CST Count값을 표시하기 위함
#define _MEM_VAR_OPT2_CST2_CURRENTCOUNT_AP			_VAR_OPT2_NO +120		// [#2270] AU KSK 2014.05.26 NegativeCount Option이 Enable시 Remain CST Count값을 표시하기 위함
#define _MEM_VAR_OPT2_CST3_CURRENTCOUNT_AP			_VAR_OPT2_NO +121		// [#2270] AU KSK 2014.05.26 NegativeCount Option이 Enable시 Remain CST Count값을 표시하기 위함
#define _MEM_VAR_OPT2_CST4_CURRENTCOUNT_AP			_VAR_OPT2_NO +122		// [#2270] AU KSK 2014.05.26 NegativeCount Option이 Enable시 Remain CST Count값을 표시하기 위함

#define _MEM_VAR_OPT2_HOT_KEY_STRING				_VAR_OPT2_NO +123		// [#2283] NH Justin 2014.07.01 Hot Key String
// Deprecated 2020-01-02: Donations : [_VAR_OPT2_NO + 124]
#define _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE		_VAR_OPT2_NO +125		// [#2292] US Justin 2014.09.17 Cardtronics Dual Balance Option
#define _MEM_VAR_OPT2_CUSTOMIZED_TIRSURCHARGE		_VAR_OPT2_NO +126		// [#2292] US Justin 2014.09.17 Cardtronics Surcharge Segmentation
#define _MEM_VAR_OPT2_CUSTOMIZED_DCC				_VAR_OPT2_NO +127		// [#2292] US Justin 2014.09.17 Cardtronics DCC
#define _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE	_VAR_OPT2_NO +128		// [#2292] US Justin 2014.09.17 Cardtronics Credit Withdrawal Surcharge

#define _MEM_VAR_OPT2_BALANCE_NOFEE_NOTICE			_VAR_OPT2_NO +129		// [#2309] US Justin 2014.11.14 No Fee Notice for Balance Inquiry

/**
 * Deprecation notice: Event Notice
 * 
 * 2020-01-02 
 * Event Notice app is deprecated
 * [_VAR_OPT2_NO + 130, _VAR_OPT2_NO + `35]
 */

#define _MEM_VAR_OPT2_DECIMAL_SURCHARGE				_VAR_OPT2_NO +136		// [#2316] NH Justin 2014.12.17 Decimal Surcharge
#define _MEM_VAR_OPT2_PERCENT_SURCHARGE_NOTICE		_VAR_OPT2_NO +137		// [#2335] NH Justin 2015.03.11 Percentage Surcharge Notice

#define _MEM_VAR_OPT2_RFID_DEVICE_ENABLE			_VAR_OPT2_NO +138		// [#2325] NH KSK 2015.01.20 RFID En/Disable
#define _MEM_VAR_OPT2_EPP_LED_GUIDE_OPTION			_VAR_OPT2_NO +139		// [#2333] NH KSK 2015.03.06 EPP LED GUIDE Option
#define _MEM_VAR_OPT2_USE_USCOMMONAID				_VAR_OPT2_NO +140		// [#2341] US Justin 2015.04.29 US Common AID Option

/**
 * Deprecation notice: Donations
 * 
 * 2020-01-02
 * Donations app is deprecated
 * [_VAR_OPT2_NO + 141, _VAR_OPT2_NO + 144]
 */

#define	_MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE		_VAR_OPT2_NO +145		// [#2349] US Justin 2015.06.05 DCC Lookup Use Local Surcharge
#define	_MEM_VAR_OPT2_POPMONEY_ENABLE				_VAR_OPT2_NO +146		// [#2350] US Justin 2015.06.17 POP Money
#define	_MEM_VAR_OPT2_POPMONEY_MAXAMT				_VAR_OPT2_NO +147		// [#2350] US Justin 2015.06.17 POP Money
#define	_MEM_VAR_OPT2_WITHDRAWAL_TYPE				_VAR_OPT2_NO +148		// [#2350] US Justin 2015.06.17 POP Money
// Deprecation 2019-12-31: Check cashing/local loan removed: [_VAR_OPT2_NO +149]
/**
 * Deprecation notice: Donations
 * 
 * 2020-01-02
 * Donations app is deprecated
 * [_VAR_OPT2_NO + 150, _VAR_OPT2_NO + 151]
 */

#define	_MEM_VAR_OPT2_4000W_RECEIPT_FULLCUT			_VAR_OPT2_NO +152		// [#2371] US Justin 2015.09.08 4000W Partial/FullCut

/**
 * Deprecation notice: MoniAir
 * 
 * 2019-12-31 
 * MoniAir app is deprecated
 * [_VAR_OPT2_NO + 153, _VAR_OPT2_NO + 160]
 */

#define	_MEM_VAR_OPT2_MOBILEAPP_QRCODE				_VAR_OPT2_NO +161		// [#2382] US Justin 2015.12.03 MobileAppQRCode

/**
 * Deprecation notice: Local Loan
 * 
 * 2019-12-31 
 * MoniAir app is deprecated
 * [_VAR_OPT2_NO + 162, _VAR_OPT2_NO + 165]
 */

#define	_MEM_VAR_OPT2_CDU_BINDING					_VAR_OPT2_NO +165		// [#2392] US Justin 2016.01.27 CDU Binding Option

#define	_MEM_VAR_OPT2_PIN4_ENABLE					_VAR_OPT2_NO +166		// [#2396] US Justin 2016.02.10 PAI Pin4
#define	_MEM_VAR_OPT2_PIN4_MAXAMT					_VAR_OPT2_NO +167		// [#2396] US Justin 2016.02.10 PAI Pin4

/**
 * Deprecation notice: Walpay
 * 
 * 2020-01-02
 * Walpay app is deprecated
 * [_VAR_OPT2_NO + 168, _VAR_OPT2_NO + 171]
 */

#define _MEM_VAR_OPT2_EMV_LANGUAGESELECTION			_VAR_OPT2_NO +172		// [#2425] US Justin 2016.05.23
// Deprecation 2019-12-31: Check cashing/local loan removed: [_VAR_OPT2_NO + 173]
#define _MEM_VAR_OPT2_CUSTOMIZED_SURCHARGE			_VAR_OPT2_NO +174		// [#2437] US Justin 2016.07.22
#define _MEM_VAR_OPT2_JUSTCASH_ENABLE				_VAR_OPT2_NO +175		// [#2445] US Justin 2016.09.22
#define _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE			_VAR_OPT2_NO +176		// [#2446] US Justin 2016.09.29 Paydiant (Paypal) CCA

#define _MEM_VAR_OPT2_DISPENSE_MIX_OPTION			_VAR_OPT2_NO +177		// [#2459] AU KSK 2016.12.23
// Deprecation 2019-12-31: Check cashing/local loan removed: [_VAR_OPT2_NO + 178, _VAR_OPT2_NO + 179]
#define	_MEM_VAR_OPT2_DCC_OPTION_ELIGIBLE			_VAR_OPT2_NO +180		// [#2472] US Justin 2017.02.07 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_MAXWITHDRAWAL		_VAR_OPT2_NO +181		// [#2472] US Justin 2017.02.07 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_FASTCASH1			_VAR_OPT2_NO +182		// [#2472] US Justin 2017.02.07 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_FASTCASH2			_VAR_OPT2_NO +183		// [#2472] US Justin 2017.02.07 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_FASTCASH3			_VAR_OPT2_NO +184		// [#2472] US Justin 2017.02.07 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_FASTCASH4			_VAR_OPT2_NO +185		// [#2472] US Justin 2017.02.07 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_FASTCASH5			_VAR_OPT2_NO +186		// [#2472] US Justin 2017.02.07 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_FASTCASH6			_VAR_OPT2_NO +187		// [#2472] US Justin 2017.02.07 DCC Withdrawal Option
#define _MEM_VAR_OPT2_FALLBACK_4_UNKNOWNAID			_VAR_OPT2_NO +188		// [#2481] US Justin 2017.04.21 Fallback Option 4 Unknown AID
#define _MEM_VAR_OPT2_STD1_DYNAMICFLOW_MAC			_VAR_OPT2_NO +189		// [#2490] NH Justin 2017.06.14 Standard1 Dynamic Flow MAC

#define _MEM_VAR_OPT2_ALPHI_ENDISABLE				_VAR_OPT2_NO +190		// [#2494] AU Kook 2017.08.07 ATS Demo
#define _MEM_VAR_OPT2_ALPHI_SERVER_ADDRESS			_VAR_OPT2_NO +191		// [#2497] AU Kook 2017.08.25 ATS ALPHI
#define _MEM_VAR_OPT2_ALPHI_SERVER_PORT				_VAR_OPT2_NO +192		// [#2497] AU Kook 2017.08.25 ATS ALPHI
#define _MEM_VAR_OPT2_ALPHI_ID						_VAR_OPT2_NO +193		// [#2497] AU Kook 2017.08.25 ATS ALPHI
#define _MEM_VAR_OPT2_ALPHI_TERMINAL_ID				_VAR_OPT2_NO +194		// [#2497] AU Kook 2017.08.25 ATS ALPHI
#define _MEM_VAR_OPT2_ALPHI_OPERATOR_ID				_VAR_OPT2_NO +195		// [#2497] AU Kook 2017.08.25 ATS ALPHI
#define _MEM_VAR_OPT2_ALPHI_TDES_KEY				_VAR_OPT2_NO +196		// [#2497] AU Kook 2017.08.25 ATS ALPHI
#define _MEM_VAR_OPT2_ALPHI_EXTRA_INFORMATION		_VAR_OPT2_NO +197		// [#2497] AU Kook 2017.08.25 ATS ALPHI
#define _MEM_VAR_OPT2_ALPHI_MSG_AUDITNUMBER			_VAR_OPT2_NO +198		// [#2497] NH woooZ 2017.09.14  ALPHI server
#define _MEM_VAR_OPT2_ALPHI_REVERSAL_MAX_RETRY		_VAR_OPT2_NO +199		// [#2497] AU Kook 2017.09.18 ATS ALPHI
#define _MEM_VAR_OPT2_ALPHI_CARDDATA_TO_BE_USED		_VAR_OPT2_NO +200		// [#2497] AU Kook 2017.09.18 ATS ALPHI
#define _MEM_VAR_OPT2_ALPHI_CURRENCY_CODE			_VAR_OPT2_NO +201		// [#2497] NH woooZ 2017.10.25 ALPHI currency code

#define _MEM_VAR_OPT2_FALLBACK_ENABLE_DISABLE		_VAR_OPT2_NO +202		// [#2517] US Justin 2017.11.29 EMV Fallback Enable/Diable Option

#define _MEM_VAR_OPT2_ALPHI_SSL_ENDISABLE			_VAR_OPT2_NO +203		// [#2517] AU Kook 2017.12.18 ATS ALPHI
#define _MEM_VAR_OPT2_PRINT_TOTAL_AMOUNT_OPTION		_VAR_OPT2_NO +204		// [#2525] AU HJ AHN 2018.01.10 Print Option 추가 (TOTAL AMOUNT En/Disable ) - AU ONLY
#define _MEM_VAR_OPT2_ALPHI_SSL_CERT_ENDISABLE		_VAR_OPT2_NO +205		// [#Test] NH woooz 2018.01.18 "SSL Certify 

#define _MEM_VAR_OPT2_CAM_ANTI_SHLDR_SURF_ENABLE	_VAR_OPT2_NO +206		// [#2518] US Kook 2018.01.14 Support Camera
#define _MEM_VAR_OPT2_CAM_FACE_MEMORIZE_ENABLE		_VAR_OPT2_NO +207		// [#2518] US Kook 2018.01.14 Support Camera
#define _MEM_VAR_OPT2_CAM_FACE_IMAGE_INDEX			_VAR_OPT2_NO +208		// [#2518] US Kook 2018.01.14 Support Camera

#define _MEM_VAR_OPT2_GIVEPAY_ENABLE_DISABLE		_VAR_OPT2_NO +209		// [#2530] US Justin 2018.01.24 GivePay Enable/Disable
#define _MEM_VAR_OPT2_CAM_ENABLE					_VAR_OPT2_NO +210		// [#2518] US Kook 2018.02.23 Support Camera
#define _MEM_VAR_OPT2_MONIMOBILE_CASHBALANCE		_VAR_OPT2_NO +211		// [#2538] NH Justin 2018.03.09 MoniMobile Cash Balance Option
#define _MEM_VAR_OPT2_WALPAY_BUTTONTEXT				_VAR_OPT2_NO +212		// [#2539] US Justin 2018.03.29 Walpay Button Text
#define _MEM_VAR_OPT2_US_TERRITORY_OPTION			_VAR_OPT2_NO +213		// [#2549] NH Justin 2018.05.07 US Territory Option	

#define _MEM_VAR_OPT2_REBOOT_OPTION					_VAR_OPT2_NO +214		// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Option
#define _MEM_VAR_OPT2_REBOOT_TIME					_VAR_OPT2_NO +215		// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Option
#define _MEM_VAR_OPT2_REBOOT_INTERVAL				_VAR_OPT2_NO +216		// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Option

#define _MEM_VAR_OPT2_RMSSEND_RETRY					_VAR_OPT2_NO +217		// [#2559] NH Justin 2018.06.18 Retry Status Send if "send" failed
#define	_MEM_VAR_OPT2_GIVEPAY_CONFIG_TIME			_VAR_OPT2_NO +218		// [#2574] US Justin GivePay Enhancement3, Add Download Config Time
#define	_MEM_VAR_OPT2_GIVEPAY_UPSELL				_VAR_OPT2_NO +219		// [#J006] US Justin GivePay Enhancement5, Add Upsell Option

//#define	_MEM_VAR_OPT2_GIVEPAY_ENROLLED				_VAR_OPT2_NO +220		// [#J006] US Justin GivePay Enhancement5, Add Upsell Option

#define	_MEM_VAR_OPT2_DENOMINATION_SELECT_ENABLE	_VAR_OPT2_NO +220		// [#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA

#define	_MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE			_VAR_OPT2_NO +221		// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
#define	_MEM_VAR_OPT2_DUALHOST_DCC_TYPE				_VAR_OPT2_NO +222		// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
#define	_MEM_VAR_OPT2_ATM_DNS_2ND					_VAR_OPT2_NO +223		// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI

#define	_MEM_VAR_OPT2_RMSSEND_SILENT				_VAR_OPT2_NO +224		// [#2580] AU Kook 2018.11.06 RMS Test
#define	_MEM_VAR_OPT2_RKT_EPP_SERIAL_NUMBER			_VAR_OPT2_NO +225		// [#2580] AU Kook 2019.07.08 Support RKT EPP S/N Inquiry

#define _MEM_VAR_OPT2_B4U_ENABLED					_VAR_OPT2_NO +226		// [#RWC6-16] Bitload4U
#define _MEM_VAR_OPT2_RMSTLSENABLE					_VAR_OPT2_NO +227		// [#RWC6-67] US William 2019.10.21 MoniView TLS
#define _MEM_VAR_OPT2_RMSTLSPORT					_VAR_OPT2_NO +228		// [#RWC6-67] US William 2019.10.25 MoniView TLS

#define _MEM_VAR_OPT2_DISPENSE_STYLE				_VAR_OPT2_NO +229		// [#RWC6-27]

#define _MEM_VAR_OPT2_MASTERPW						_VAR_OPT2_NO +230		// [#RWC6-56] US William 2019.12.26 Hashing passwords
#define _MEM_VAR_OPT2_OPERATORPW					_VAR_OPT2_NO +231		// [#RWC6-56] US William 2019.12.26 Hashing passwords
#define _MEM_VAR_OPT2_SERVICEPW						_VAR_OPT2_NO +232		// [#RWC6-56] US William 2019.12.26 Hashing passwords
#define _MEM_VAR_OPT2_PWLENGTH						_VAR_OPT2_NO +233		// [#RWC6-56] US William 2020.01.06 Hashing passwords
#define _MEM_VAR_OPT2_INVALIDLOGONATTEMPTS			_VAR_OPT2_NO +234		// [#RWC6-56] US William 2020.01.06 Hashing passwords
#define _MEM_VAR_OPT2_LOCKOUTEXPIRATION				_VAR_OPT2_NO +235		// [#RWC6-56] US William 2020.01.06 Hashing passwords

#define _MEM_VAR_OPT2_SCHEDULE_U2D_ENABLE			_VAR_OPT2_NO +236		// [#RWC6-121] US William 2020.02.03 Remote Updates
#define _MEM_VAR_OPT2_SCHEDULE_U2D_DOW				_VAR_OPT2_NO +237		// [#RWC6-121] US William 2020.02.03 Remote Updates

#define _MEM_VAR_OPT2_LIBERTYX_SEQUENCE				_VAR_OPT2_NO +238		// [#RWC6-216] US William 2020.08.21 LibertyX Sequence

#define _MEM_VAR_OPT2_DUAL_RECEIPT					_VAR_OPT2_NO +239		// [#RWC6-290] US Brandon 2021.07.06 Support dual-language receipts

#define _MEM_VAR_OPT2_INSERVICE_WHENCSTSSHORT		_VAR_OPT2_NO +240		// [#RWC6-319] US Brandon 2021.10.19 No cash out-of-service option
#define _MEM_VAR_OPT2_INSERVICE_WHENCSTSEMPTY		_VAR_OPT2_NO +241		// [#RWC6-319] US Brandon 2021.10.19 No cash out-of-service option

#define _MEM_VAR_OPT2_OPACCESSCOUNT					_VAR_OPT2_NO +242		//[#RWC6-359] US Matt 2022.01.14 Quick Configuration no TID
#define _MEM_VAR_OPT2_RKT_PROTOCOL					_VAR_OPT2_NO +243		// [#RWC6-151] US Kook 2020.07.14 TR-34

#define _MEM_VAR_OPT2_SIDECAR_SUPPORT				_VAR_OPT2_NO +244		// [#GLDV-3005] US Kook 2021.10.22 Support Side Car
#define _MEM_VAR_OPT2_SIDECAR_BNATYPE				_VAR_OPT2_NO +245		// [#GLDV-3005] US Kook 2021.10.22 Support Side Car
#define _MEM_VAR_OPT2_SIDECAR_CASHINTOTAL_TIME		_VAR_OPT2_NO +246		// [#GLDV-3005] US Kook 2021.10.22 Support Side Car

// [#620] US KSK 2010.02.18
//////////////////////////////////////////////////////////////////////////
// ENHANCED COUPON INFORMATION
//////////////////////////////////////////////////////////////////////////
#define _MEM_VAR_ENHANCED_COUPON_AVAILABLE			_VAR_ENAHNCED_COUPON
#define _MEM_VAR_ENHANCED_COUPON_ENABLE				_VAR_ENAHNCED_COUPON + 1
#define _MEM_VAR_ENHANCED_COUPON_CUT_OPTION			_VAR_ENAHNCED_COUPON + 2
#define _MEM_VAR_ENHANCED_COUPON1_OPTION			_VAR_ENAHNCED_COUPON + 3
#define _MEM_VAR_ENHANCED_COUPON2_OPTION			_VAR_ENAHNCED_COUPON + 4
#define _MEM_VAR_ENHANCED_COUPON3_OPTION			_VAR_ENAHNCED_COUPON + 5
#define _MEM_VAR_ENHANCED_COUPON4_OPTION			_VAR_ENAHNCED_COUPON + 6
#define _MEM_VAR_ENHANCED_COUPON5_OPTION			_VAR_ENAHNCED_COUPON + 7
#define _MEM_VAR_ENHANCED_COUPON6_OPTION			_VAR_ENAHNCED_COUPON + 8
#define _MEM_VAR_ENHANCED_COUPON1_PRINT_START_TIME	_VAR_ENAHNCED_COUPON + 9
#define _MEM_VAR_ENHANCED_COUPON2_PRINT_START_TIME	_VAR_ENAHNCED_COUPON + 10
#define _MEM_VAR_ENHANCED_COUPON3_PRINT_START_TIME	_VAR_ENAHNCED_COUPON + 11
#define _MEM_VAR_ENHANCED_COUPON4_PRINT_START_TIME	_VAR_ENAHNCED_COUPON + 12
#define _MEM_VAR_ENHANCED_COUPON5_PRINT_START_TIME	_VAR_ENAHNCED_COUPON + 13
#define _MEM_VAR_ENHANCED_COUPON6_PRINT_START_TIME	_VAR_ENAHNCED_COUPON + 14
#define _MEM_VAR_ENHANCED_COUPON1_PRINT_END_TIME	_VAR_ENAHNCED_COUPON + 15
#define _MEM_VAR_ENHANCED_COUPON2_PRINT_END_TIME	_VAR_ENAHNCED_COUPON + 16
#define _MEM_VAR_ENHANCED_COUPON3_PRINT_END_TIME	_VAR_ENAHNCED_COUPON + 17
#define _MEM_VAR_ENHANCED_COUPON4_PRINT_END_TIME	_VAR_ENAHNCED_COUPON + 18
#define _MEM_VAR_ENHANCED_COUPON5_PRINT_END_TIME	_VAR_ENAHNCED_COUPON + 19
#define _MEM_VAR_ENHANCED_COUPON6_PRINT_END_TIME	_VAR_ENAHNCED_COUPON + 20
#define _MEM_VAR_ENHANCED_COUPON1_TEXT1				_VAR_ENAHNCED_COUPON + 21
#define _MEM_VAR_ENHANCED_COUPON1_TEXT2				_VAR_ENAHNCED_COUPON + 22
#define _MEM_VAR_ENHANCED_COUPON1_TEXT3				_VAR_ENAHNCED_COUPON + 23
#define _MEM_VAR_ENHANCED_COUPON1_TEXT4				_VAR_ENAHNCED_COUPON + 24
#define _MEM_VAR_ENHANCED_COUPON1_TEXT5				_VAR_ENAHNCED_COUPON + 25
#define _MEM_VAR_ENHANCED_COUPON1_TEXT6				_VAR_ENAHNCED_COUPON + 26
#define _MEM_VAR_ENHANCED_COUPON1_TEXT7				_VAR_ENAHNCED_COUPON + 27
#define _MEM_VAR_ENHANCED_COUPON1_TEXT8				_VAR_ENAHNCED_COUPON + 28
#define _MEM_VAR_ENHANCED_COUPON1_TEXT9				_VAR_ENAHNCED_COUPON + 29
#define _MEM_VAR_ENHANCED_COUPON1_TEXT10			_VAR_ENAHNCED_COUPON + 30
#define _MEM_VAR_ENHANCED_COUPON1_TEXT11			_VAR_ENAHNCED_COUPON + 31
#define _MEM_VAR_ENHANCED_COUPON1_TEXT12			_VAR_ENAHNCED_COUPON + 32
#define _MEM_VAR_ENHANCED_COUPON1_TEXT13			_VAR_ENAHNCED_COUPON + 33
#define _MEM_VAR_ENHANCED_COUPON1_TEXT14			_VAR_ENAHNCED_COUPON + 34
#define _MEM_VAR_ENHANCED_COUPON1_TEXT15			_VAR_ENAHNCED_COUPON + 35
#define _MEM_VAR_ENHANCED_COUPON1_TEXT16			_VAR_ENAHNCED_COUPON + 36
#define _MEM_VAR_ENHANCED_COUPON2_TEXT1				_VAR_ENAHNCED_COUPON + 37
#define _MEM_VAR_ENHANCED_COUPON2_TEXT2				_VAR_ENAHNCED_COUPON + 38
#define _MEM_VAR_ENHANCED_COUPON2_TEXT3				_VAR_ENAHNCED_COUPON + 39
#define _MEM_VAR_ENHANCED_COUPON2_TEXT4				_VAR_ENAHNCED_COUPON + 40
#define _MEM_VAR_ENHANCED_COUPON2_TEXT5				_VAR_ENAHNCED_COUPON + 41
#define _MEM_VAR_ENHANCED_COUPON2_TEXT6				_VAR_ENAHNCED_COUPON + 42
#define _MEM_VAR_ENHANCED_COUPON2_TEXT7				_VAR_ENAHNCED_COUPON + 43
#define _MEM_VAR_ENHANCED_COUPON2_TEXT8				_VAR_ENAHNCED_COUPON + 44
#define _MEM_VAR_ENHANCED_COUPON2_TEXT9				_VAR_ENAHNCED_COUPON + 45
#define _MEM_VAR_ENHANCED_COUPON2_TEXT10			_VAR_ENAHNCED_COUPON + 46
#define _MEM_VAR_ENHANCED_COUPON2_TEXT11			_VAR_ENAHNCED_COUPON + 47
#define _MEM_VAR_ENHANCED_COUPON2_TEXT12			_VAR_ENAHNCED_COUPON + 48
#define _MEM_VAR_ENHANCED_COUPON2_TEXT13			_VAR_ENAHNCED_COUPON + 49
#define _MEM_VAR_ENHANCED_COUPON2_TEXT14			_VAR_ENAHNCED_COUPON + 50
#define _MEM_VAR_ENHANCED_COUPON2_TEXT15			_VAR_ENAHNCED_COUPON + 51
#define _MEM_VAR_ENHANCED_COUPON2_TEXT16			_VAR_ENAHNCED_COUPON + 52
#define _MEM_VAR_ENHANCED_COUPON3_TEXT1				_VAR_ENAHNCED_COUPON + 53
#define _MEM_VAR_ENHANCED_COUPON3_TEXT2				_VAR_ENAHNCED_COUPON + 54
#define _MEM_VAR_ENHANCED_COUPON3_TEXT3				_VAR_ENAHNCED_COUPON + 55
#define _MEM_VAR_ENHANCED_COUPON3_TEXT4				_VAR_ENAHNCED_COUPON + 56
#define _MEM_VAR_ENHANCED_COUPON3_TEXT5				_VAR_ENAHNCED_COUPON + 57
#define _MEM_VAR_ENHANCED_COUPON3_TEXT6				_VAR_ENAHNCED_COUPON + 58
#define _MEM_VAR_ENHANCED_COUPON3_TEXT7				_VAR_ENAHNCED_COUPON + 59
#define _MEM_VAR_ENHANCED_COUPON3_TEXT8				_VAR_ENAHNCED_COUPON + 60
#define _MEM_VAR_ENHANCED_COUPON3_TEXT9				_VAR_ENAHNCED_COUPON + 61
#define _MEM_VAR_ENHANCED_COUPON3_TEXT10			_VAR_ENAHNCED_COUPON + 62
#define _MEM_VAR_ENHANCED_COUPON3_TEXT11			_VAR_ENAHNCED_COUPON + 63
#define _MEM_VAR_ENHANCED_COUPON3_TEXT12			_VAR_ENAHNCED_COUPON + 64
#define _MEM_VAR_ENHANCED_COUPON3_TEXT13			_VAR_ENAHNCED_COUPON + 65
#define _MEM_VAR_ENHANCED_COUPON3_TEXT14			_VAR_ENAHNCED_COUPON + 66
#define _MEM_VAR_ENHANCED_COUPON3_TEXT15			_VAR_ENAHNCED_COUPON + 67
#define _MEM_VAR_ENHANCED_COUPON3_TEXT16			_VAR_ENAHNCED_COUPON + 68
#define _MEM_VAR_ENHANCED_COUPON4_TEXT1				_VAR_ENAHNCED_COUPON + 69
#define _MEM_VAR_ENHANCED_COUPON4_TEXT2				_VAR_ENAHNCED_COUPON + 70
#define _MEM_VAR_ENHANCED_COUPON4_TEXT3				_VAR_ENAHNCED_COUPON + 71
#define _MEM_VAR_ENHANCED_COUPON4_TEXT4				_VAR_ENAHNCED_COUPON + 72
#define _MEM_VAR_ENHANCED_COUPON4_TEXT5				_VAR_ENAHNCED_COUPON + 73
#define _MEM_VAR_ENHANCED_COUPON4_TEXT6				_VAR_ENAHNCED_COUPON + 74
#define _MEM_VAR_ENHANCED_COUPON4_TEXT7				_VAR_ENAHNCED_COUPON + 75
#define _MEM_VAR_ENHANCED_COUPON4_TEXT8				_VAR_ENAHNCED_COUPON + 76
#define _MEM_VAR_ENHANCED_COUPON4_TEXT9				_VAR_ENAHNCED_COUPON + 77
#define _MEM_VAR_ENHANCED_COUPON4_TEXT10			_VAR_ENAHNCED_COUPON + 78
#define _MEM_VAR_ENHANCED_COUPON4_TEXT11			_VAR_ENAHNCED_COUPON + 79
#define _MEM_VAR_ENHANCED_COUPON4_TEXT12			_VAR_ENAHNCED_COUPON + 80
#define _MEM_VAR_ENHANCED_COUPON4_TEXT13			_VAR_ENAHNCED_COUPON + 81
#define _MEM_VAR_ENHANCED_COUPON4_TEXT14			_VAR_ENAHNCED_COUPON + 82
#define _MEM_VAR_ENHANCED_COUPON4_TEXT15			_VAR_ENAHNCED_COUPON + 83
#define _MEM_VAR_ENHANCED_COUPON4_TEXT16			_VAR_ENAHNCED_COUPON + 84
#define _MEM_VAR_ENHANCED_COUPON5_TEXT1				_VAR_ENAHNCED_COUPON + 85
#define _MEM_VAR_ENHANCED_COUPON5_TEXT2				_VAR_ENAHNCED_COUPON + 86
#define _MEM_VAR_ENHANCED_COUPON5_TEXT3				_VAR_ENAHNCED_COUPON + 87
#define _MEM_VAR_ENHANCED_COUPON5_TEXT4				_VAR_ENAHNCED_COUPON + 88
#define _MEM_VAR_ENHANCED_COUPON5_TEXT5				_VAR_ENAHNCED_COUPON + 89
#define _MEM_VAR_ENHANCED_COUPON5_TEXT6				_VAR_ENAHNCED_COUPON + 90
#define _MEM_VAR_ENHANCED_COUPON5_TEXT7				_VAR_ENAHNCED_COUPON + 91
#define _MEM_VAR_ENHANCED_COUPON5_TEXT8				_VAR_ENAHNCED_COUPON + 92
#define _MEM_VAR_ENHANCED_COUPON5_TEXT9				_VAR_ENAHNCED_COUPON + 93
#define _MEM_VAR_ENHANCED_COUPON5_TEXT10			_VAR_ENAHNCED_COUPON + 94
#define _MEM_VAR_ENHANCED_COUPON5_TEXT11			_VAR_ENAHNCED_COUPON + 95
#define _MEM_VAR_ENHANCED_COUPON5_TEXT12			_VAR_ENAHNCED_COUPON + 96
#define _MEM_VAR_ENHANCED_COUPON5_TEXT13			_VAR_ENAHNCED_COUPON + 97
#define _MEM_VAR_ENHANCED_COUPON5_TEXT14			_VAR_ENAHNCED_COUPON + 98
#define _MEM_VAR_ENHANCED_COUPON5_TEXT15			_VAR_ENAHNCED_COUPON + 99
#define _MEM_VAR_ENHANCED_COUPON5_TEXT16			_VAR_ENAHNCED_COUPON + 100
#define _MEM_VAR_ENHANCED_COUPON6_TEXT1				_VAR_ENAHNCED_COUPON + 101
#define _MEM_VAR_ENHANCED_COUPON6_TEXT2				_VAR_ENAHNCED_COUPON + 102
#define _MEM_VAR_ENHANCED_COUPON6_TEXT3				_VAR_ENAHNCED_COUPON + 103
#define _MEM_VAR_ENHANCED_COUPON6_TEXT4				_VAR_ENAHNCED_COUPON + 104
#define _MEM_VAR_ENHANCED_COUPON6_TEXT5				_VAR_ENAHNCED_COUPON + 105
#define _MEM_VAR_ENHANCED_COUPON6_TEXT6				_VAR_ENAHNCED_COUPON + 106
#define _MEM_VAR_ENHANCED_COUPON6_TEXT7				_VAR_ENAHNCED_COUPON + 107
#define _MEM_VAR_ENHANCED_COUPON6_TEXT8				_VAR_ENAHNCED_COUPON + 108
#define _MEM_VAR_ENHANCED_COUPON6_TEXT9				_VAR_ENAHNCED_COUPON + 109
#define _MEM_VAR_ENHANCED_COUPON6_TEXT10			_VAR_ENAHNCED_COUPON + 110
#define _MEM_VAR_ENHANCED_COUPON6_TEXT11			_VAR_ENAHNCED_COUPON + 111
#define _MEM_VAR_ENHANCED_COUPON6_TEXT12			_VAR_ENAHNCED_COUPON + 112
#define _MEM_VAR_ENHANCED_COUPON6_TEXT13			_VAR_ENAHNCED_COUPON + 113
#define _MEM_VAR_ENHANCED_COUPON6_TEXT14			_VAR_ENAHNCED_COUPON + 114
#define _MEM_VAR_ENHANCED_COUPON6_TEXT15			_VAR_ENAHNCED_COUPON + 115
#define _MEM_VAR_ENHANCED_COUPON6_TEXT16			_VAR_ENAHNCED_COUPON + 116
// end of [#620]

//////////////////////////////////////////////////////////////////////////
// Optional Function 3
//////////////////////////////////////////////////////////////////////////
#define _MEM_VAR_OPT3_TR31_WKB						(_VAR_OPT3_NO + 0)		// [#RWC6-150] US William 2021.03.29 TR-31
#define _MEM_VAR_OPT3_TR31_MKB						(_VAR_OPT3_NO + 1)		// [#RWC6-150] US William 2021.03.29 TR-31
#define _MEM_VAR_OPT3_CASHDEPOT_ISHOSTUSEURL		(_VAR_OPT3_NO + 2)
#define _MEM_VAR_OPT3_CASHDEPOT_HOSTNAME			(_VAR_OPT3_NO + 3)


/////////////////////////////////////////////////////////////////////////////
//	ATM Mode Define
/////////////////////////////////////////////////////////////////////////////
//#define	MODE_COUNT		9
#define	MODE_COUNT		10		// [#369] [US] KSK 2008.7.16
//CString AtmStatusName[MODE_COUNT] = {
//			"ATM_INIT       ",
//			"ATM_ERROR      ", 
//			"ATM_CLERK      ",
//			"ATM_READY      ", 
//			"ATM_CUSTOM     ",
//			"ATM_TRAN       ", 
//			"ATM_DOWN       ", 
//			"ATM_REBOOT     ",
//			"ATM_RMSACTIVE  ",
//			"ATM_REMOTEUPDATE", // [#RWC6-121] US William 2020.02.24 Remote Updates
//};

#define ATM_INIT			0						// INIT		MODE
#define ATM_ERROR			1						// ERROR	MODE
#define ATM_CLERK			2						// CLERK	MODE
#define ATM_READY			3						// READY	MODE
#define ATM_CUSTOM			4						// CUSTOM	MODE 
#define ATM_TRAN			5						// TRAN		MODE
#define ATM_DOWN			6						// DOWN		MODE	[#24] KSK 2008.04.08
#define ATM_REBOOT			7						// REBOOT	MODE
#define ATM_RMSACTIVE		8						// RMS ACTIVE MODE
#define ATM_REMOTEUDPATE	9						// REMOTE UPDATE [#RWC6-121] US William 2020.02.24 Add remote updates

#define	GETSTR_ATMMODE(VAL)			\
	((VAL == ATM_INIT)				? ("ATM_INIT")	:		\
	((VAL == ATM_ERROR)				? ("ATM_ERROR")	:	\
	((VAL == ATM_CLERK)				? ("ATM_CLERK")	:	\
	((VAL == ATM_READY)				? ("ATM_READY")	:	\
	((VAL == ATM_CUSTOM)			? ("ATM_CUSTOM")	:	\
	((VAL == ATM_TRAN)				? ("ATM_TRAN")	:	\
	((VAL == ATM_DOWN)				? ("ATM_DOWN")	:	\
	((VAL == ATM_REBOOT)			? ("ATM_REBOOT")	:	\
	((VAL == ATM_RMSACTIVE)			? ("ATM_RMSACTIVE")	:	\
	((VAL == ATM_REMOTEUDPATE)		? ("ATM_REMOTEUDPATE")	:	\
	("UNDEF") ))))))))))

/////////////////////////////////////////////////////////////////////////////
//	Host Mode Define
/////////////////////////////////////////////////////////////////////////////
#define HOST_OFFLINE 		0						// OFFLINE
#define HOST_WAITONLINE		1						// WAITONLINE
#define HOST_ONLINE			2						// ONLINE

/////////////////////////////////////////////////////////////////////////////
//	Buffer Size
/////////////////////////////////////////////////////////////////////////////

// [#2012] NH KJW 2011.01.19 heap할당을 위한 버퍼사이즈
//#define	NETBUFFSIZE		2048
//#define NETBUFFSIZE_HUGE	(1024 * 10)
//#define NETBUF_SEND_SIZE	(1024 * 4)
#define NETBUF_SEND_SIZE	(1024 * 10)				// [#2515] US Justin 2017.11.20 Increase Send Buffer Size
#define NETBUF_RECV_SIZE	(1024 * 300)
// end of [#2012]

/////////////////////////////////////////////////////////////////////////////
//	Device Check Time
/////////////////////////////////////////////////////////////////////////////
#define MAX_DEVRSP_TIME		365						// MAX DEVICE RESPONSE TIME
#define MAX_DEVSCR_TIME		900						// MAX SCREEN RESPONSE TIME 15MIN (15*60)  // [#265] AIREAT 2008.06.09 - Value 값 변경 (125->900)
#define MID_DEVRSP_TIME		125						// MID DEVICE RESPONSE TIME(TAKE TIME + 5 SEC : 2003.10.29)
#define MIN_DEVRSP_TIME		5						// MIN DEVICE RESPONSE TIME

#define TRAN_WITH_NOT		0x00

/////////////////////////////////////////////////////////////////////////////
//	Spr & Jpr Header Information
/////////////////////////////////////////////////////////////////////////////
#define HEADER_NORMAL		0x00					// SPR & JPR HEADER NORMAL
#define	SLIP_HEAD_UP		0x01					// SPR HEAD UP
#define	JNL_HEAD_UP			0x10					// JPR HEAD UP

/////////////////////////////////////////////////////////////////////////////
//	On/Off
/////////////////////////////////////////////////////////////////////////////
#define ON					1
#define OFF					0

/////////////////////////////////////////////////////////////////////////////
//	En/Disable(Mcu, Pbm)
/////////////////////////////////////////////////////////////////////////////
#define	ENABLE				1
#define	DISABLE				2

//////////////////////////////////////////////////////////////////////////
#define ERRDEV				0
#define ALLDEV				1
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//	Pin Input Mode
/////////////////////////////////////////////////////////////////////////////
#define	PIN_DISABLE_MODE	1
#define	PIN_MENU_MODE		2
#define	PIN_PASSWORD_MODE	3
#define	PIN_AMOUNT_MODE		4
#define	PIN_ALL_MODE		5
#define	PIN_AMOUNT_FTN_MODE	6		// [#2530] US Justin 2018.01.24 

#define	PIN_NUM_ALL_PAD			_T("1,2,3,4,5,6,7,8,9,0,.,00,CANCEL,CLEAR,ENTER")
#define	PIN_PASSWORD_PAD		_T("1,2,3,4,5,6,7,8,9,0,CANCEL,CLEAR,ENTER,F8")
#define	PIN_AMOUNT_PAD			_T("1,2,3,4,5,6,7,8,9,0,CANCEL,CLEAR,ENTER,F8")
#define	PIN_AMOUNT_FTN_PAD		_T("1,2,3,4,5,6,7,8,9,0,CANCEL,CLEAR,ENTER,F1,F2,F3,F4,F5,F6,F7,F8")		// [#2530] US Justin 2018.01.24 

#define	PIN_ADA_PAD				_T("1,2,3,4,5,6,7,8,9,0,.,00,000,CANCEL,CLEAR,ENTER")
#define	PIN_ADA2_PAD			_T("1,2,3,4,5,6,7,8,9,0,CANCEL,CLEAR,ENTER")
#define	PIN_ALL_PAD				_T("1,2,3,4,5,6,7,8,9,0,.,00,000,CANCEL,CLEAR,ENTER,F1,F2,F3,F4,F5,F6,F7,F8")

/////////////////////////////////////////////////////////////////////////////
//	Track Information
/////////////////////////////////////////////////////////////////////////////
#define ISO1_TRACK			0x01
#define ISO2_TRACK			0x02
#define ISO3_TRACK			0x04
#define JIS_TRACK			0x08
#define ALL_TRACK			0x0f

/////////////////////////////////////////////////////////////////////////////
//	Language Mode
/////////////////////////////////////////////////////////////////////////////
#define ENG_MODE			1						// ENGLISH
#define SPN_MODE			2						// SPANISH
#define FRN_MODE			3						// FRENCH
#define CHN_MODE			4						// CHINESE
#define KOR_MODE			5						// KOREAN
#define JPN_MODE			6						// JAPANESE
#define LANGUAGE_MODE_MAX	6						// Language max		// [#469][NH] AIREAT 2008.12.20 AP,OP Text 다국어 통합 작업

/////////////////////////////////////////////////////////////////////////////
//	SPR Image Printing Command
/////////////////////////////////////////////////////////////////////////////
// [#2373] US Justin 2015.09.22 Support Coupon Image Printing
#define	CMD_PRINT_HEADER_IMAGE			_T("HEADERIMAGE")
#define	CMD_PRINT_COUPON_IMAGE_BCD		_T("COUPONIMAGE_BCD")
#define	CMD_PRINT_COUPON_IMAGE_QR		_T("COUPONIMAGE_QR")
#define	CMD_PRINT_COUPON_IMAGE_LARGEQR	_T("COUPONIMAGE_LARGEQR")	// [#2503] US Justin 2017.09.15 Add Ethereum.. Large QR Image

// End of [#2373]

/////////////////////////////////////////////////////////////////////////////
//	Customer Set [#105] KSK 2008.04.14
// 아래의 String값을 사용하면 안됨 (OP에서 설정 변경시 VISA FRAMED OPTION으로 Setting해야함)
/////////////////////////////////////////////////////////////////////////////
//#define CITIBANK_MODE					_T("CITIBANK")	// KSK 2008.7.10 사용안함
//#define GENERAL_MODE					_T("GENERAL")	// KSK 2008.7.10 사용안함
#define LYNK_MODE						_T("LYNK")

/////////////////////////////////////////////////////////////////////////////
//	VISA FRAMED OPTION Set [#105] KSK 2008.04.14
/////////////////////////////////////////////////////////////////////////////
#define	EOT_OPTIONAL			_T("2")
#define	EOT_REQUIRED			_T("3")
#define	NO_EOT_REQUIRED			_T("4")
#define	NO_ENQ_REQUIRED			_T("5")
#define NO_ENQ_EOT_REQUIRED		_T("6") //[#556] SOOK 2009.08.12 NO ENQ/EOT REQUIRED 추가 

/////////////////////////////////////////////////////////////////////////////
//	TCP/IP TYPE Set	[#105] KSK 2008.04.14
/////////////////////////////////////////////////////////////////////////////
#define VISA_FRAMED_TCPIP				1
#define STANDARD_TCPIP					2
#define ACK_CONTROLLED_TCPIP			3
#define	VISA_FRAMED_WITHOUT_ENQ_TCPIP	4

/////////////////////////////////////////////////////////////////////////////
//	NETWORK TYPE						[#2434] AU Kook 2016.07.22 refactoring
/////////////////////////////////////////////////////////////////////////////
#define NETWORK_DIALUP					1
#define NETWORK_TCPIP					2
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// SSL VERSION
/////////////////////////////////////////////////////////////////////////////
// [#2310] NH KSK 2014.11.18 TLS 지원을 위해 Option 변경 (SSL V3, TLS1.0, TLS1.1, TLS1,2)
//#define SSL_VER_23					1	// [#2310] NH KSK 2014.11.18 Not Support
//#define SSL_VER_3						2	// [#2310] NH KSK 2014.11.18 Change Define Value
#define MODEM_HOST_DEV_TEST				3	// [#2016] NH KSK 2011.02.09
#define MODEM_RMS_DEV_TEST				4	// [#2016] NH KSK 2011.02.09	RMS DEV TEST는 현재 사용하지 않지만, 필요시 추후 지원 예정
#define MODEM_HOST_LINE_TEST			5	// [#2016] NH KSK 2011.02.09
#define MODEM_RMS_LINE_TEST				6	// [#2016] NH KSK 2011.02.09	RMS LINE TEST는 현재 사용하지 않지만, 필요시 추후 지원 예정
#define TCPIP_HOST_LINE_TEST			7	// [#2016] NH KSK 2011.02.09
//#define TCPIP_HOST_SSL_VER23_LINE_TEST	8	// [#2016] NH KSK 2011.02.09	[#2310] NH KSK 2014.11.18 Not Support
//#define TCPIP_HOST_SSL_VER3_LINE_TEST	9	// [#2016] NH KSK 2011.02.09		[#2310] NH KSK 2014.11.18 Change Define Value
/////////////////////////////////////////////////////////////////////////////
//	RKT DEFINE	[#2075] NH KSK 2011.06.27
/////////////////////////////////////////////////////////////////////////////
#define RKT_CONNECTION					10
#define RKT_MODE_R0						11
#define RKT_MODE_R1						12
#define RKT_MODE_R2						13

// [#RWC6-151] US Kook 2020.07.14 TR34
#define RKT_PROTOCOL_BASIC					0
#define RKT_PROTOCOL_TR34					1

////////////////////////////////////////////////////////////////////////////
// SSL CERT. OPTION [#2093] NH PCS 2011.11.1
//#define	SSL_VER_23_USE_CERTIFY					14	// [#2093] NH PCS 2011.10.28
//#define SSL_VER_3_USE_CERTIFY						15  // [#2093] NH PCS 2011.10.28
//#define SSL_VER_23_USE_CERTIFY_LINE_TEST			16  // [#2093] NH PCS 2011.10.28 "Needed for Diagnostic Mode of CERT."
//#define SSL_VER_3_USE_CERTIFY_LINE_TEST			17  // [#2093] NH PCS 2011.10.28 "Needed for Diagnostic Mode of CERT."
////////////////////////////////////////////////////////////////////////////
// end of [#2093]

//[#2320] NH Justin 2015.01.14 Support NON-"Version Negotiation" Processor
/*
//#define SSL_VER3_AND_ABOVE						18	// [#2310] NH KSK 2014.11.18 Added Option
//#define TLS_VER10_AND_ABOVE						19	// [#2310] NH KSK 2014.11.18 Added Option
//#define TLS_VER11_AND_ABOVE						20	// [#2310] NH KSK 2014.11.18 Added Option
//#define TLS_VER12									21	// [#2310] NH KSK 2014.11.18 Added Option
//#define SSL_VER3_AND_ABOVE_LINE_TEST				22	// [#2310] NH KSK 2014.11.18 Added Option
//#define TLS_VER10_AND_ABOVE_LINE_TEST				23	// [#2310] NH KSK 2014.11.18 Added Option
//#define TLS_VER11_AND_ABOVE_LINE_TEST				24	// [#2310] NH KSK 2014.11.18 Added Option
//#define TLS_VER12_LINE_TEST						25	// [#2310] NH KSK 2014.11.18 Added Option
//#define SSL_VER3_AND_ABOVE_USE_CERTIFY			26  // [#2310] NH KSK 2014.11.18 Added Option
//#define TLS_VER10_AND_ABOVE_USE_CERTIFY			27  // [#2310] NH KSK 2014.11.18 Added Option
//#define TLS_VER11_AND_ABOVE_USE_CERTIFY			28	// [#2310] NH KSK 2014.11.18 Added Option
//#define TLS_VER12_USE_CERTIFY						29	// [#2310] NH KSK 2014.11.18 Added Option
//#define SSL_VER3_AND_ABOVE_USE_CERTIFY_LINE_TEST	30  // [#2310] NH KSK 2014.11.18 Added Option
//#define TLS_VER10_AND_ABOVE_USE_CERTIFY_LINE_TEST	31	// [#2310] NH KSK 2014.11.18 Added Option
//#define TLS_VER11_AND_ABOVE_USE_CERTIFY_LINE_TEST	28	// [#2310] NH KSK 2014.11.18 Added Option
//#define TLS_VER12_USE_CERTIFY_LINE_TEST				29	// [#2310] NH KSK 2014.11.18 Added Option
*/

#define CONN_UPTO_SSL_NONE							0

#define CONN_UPTO_SSL_V30							40
#define CONN_UPTO_TLS_V10							41
#define CONN_UPTO_TLS_V11							42
#define CONN_UPTO_TLS_V12							43
#define CONN_UPTO_TLS_V13							44

#define CONN_UPTO_SSL_V30_LINE_TEST					60
#define CONN_UPTO_TLS_V10_LINE_TEST					61
#define CONN_UPTO_TLS_V11_LINE_TEST					62
#define CONN_UPTO_TLS_V12_LINE_TEST					63
#define CONN_UPTO_TLS_V13_LINE_TEST					64

#define CONN_UPTO_SSL_V30_USE_CERT					80
#define CONN_UPTO_TLS_V10_USE_CERT					81
#define CONN_UPTO_TLS_V11_USE_CERT					82
#define CONN_UPTO_TLS_V12_USE_CERT					83
#define CONN_UPTO_TLS_V13_USE_CERT					84

#define CONN_UPTO_SSL_V30_USE_CERT_LINE_TEST		100
#define CONN_UPTO_TLS_V10_USE_CERT_LINE_TEST		101
#define CONN_UPTO_TLS_V11_USE_CERT_LINE_TEST		102
#define CONN_UPTO_TLS_V12_USE_CERT_LINE_TEST		103
#define CONN_UPTO_TLS_V13_USE_CERT_LINE_TEST		104
// End of [#2320]

// [#2497] AU Kook 2017.10.10 ATS ALPHI
#define CONN_UPTO_SSL_V30_USE_CERT_ALPHI			120
#define CONN_UPTO_TLS_V10_USE_CERT_ALPHI			121
#define CONN_UPTO_TLS_V11_USE_CERT_ALPHI			122
#define CONN_UPTO_TLS_V12_USE_CERT_ALPHI			123
// end of [#2497]
#define CONN_UPTO_TLS_V12_ALPHI						124  // [#2497] NH woooZ 2018.01.19  ALPHI server
// end of [#2310]

/////////////////////////////////////////////////////////////////////////////
// COUNTRY OPTION (AU, NZ only)
/////////////////////////////////////////////////////////////////////////////
// [#2042] AU KSK 2011.04.02
#define COUNTRY_AUS						1
#define COUNTRY_NZ						2
// end of [#2042]

/////////////////////////////////////////////////////////////////////////////
// [#2136] NH PCS 2012.05.21
#define NOT_USED							0
#define PERCENT_MODE						1
#define TABLE_MODE							2
//end of [#2136]
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//	PERCENT SURCHARGE MANNER Set	[#112] US KGS 2008.04.15 Percent Surcharge Menu
/////////////////////////////////////////////////////////////////////////////
#define LESSER							0
#define GREATER							1

// [#2183] NH Justin 2013.04.05 Receipt on Screen Option 
#define RECEIPT_ON_SCREEN_ENABLE		0
#define RECEIPT_ON_SCREEN_DISABLE		1
// End of [#2183]

// [#2187] NH Justin 2013.04.18 DCC Custom Option
#define DCC_CUSTOMOPTION_DEFAULT		-1				// [#2292] US Justin 2014.10.01 ADD TDL
#define DCC_CUSTOMOPTION_GENERAL		0
#define DCC_CUSTOMOPTION_CIBC			1
#define DCC_CUSTOMOPTION_CARDTRONICS	2				// [#2292] US Justin 2014.10.01 ADD TDL
#define DCC_CUSTOMOPTION_CDS			3				// [#2304] US Justin 2014.11.07 ADD CDS DCC OPTION
#define DCC_CUSTOMOPTION_PLANET			4				// [#2569] US Justin 2018.07.30 Add Planet Payment Option
#define DCC_CUSTOMOPTION_MONEX			5				// [#2569] US Justin 2018.07.30 Add Monex Option
#define DCC_CUSTOMOPTION_BANSI			6				// BANSi Mexico
#define DCC_CUSTOMOPTION_MAX			7				// [#2569] US Justin 2018.07.30
// End of [#2187]

// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
#define DYNAMICFLOW_DCC_TYPE_BOTH			0	// default value for general ATM
#define DYNAMICFLOW_DCC_TYPE_MASTERONLY		1		
#define DYNAMICFLOW_DCC_TYPE_VISAONLY		2

#define DUALHOST_DCC_TYPE_BOTH			0	// default value for general ATM
#define DUALHOST_DCC_TYPE_MASTERONLY	1		
#define DUALHOST_DCC_TYPE_VISAONLY		2
// end of [#RWC6-2, #2585]

// [#RWC6-27]
#define DISPENSE_STYLE_LEAST			1	// default value for general ATM
#define DISPENSE_STYLE_EVEN				2
#define DISPENSE_STYLE_BALANCED			3		
#define DISPENSE_STYLE_MOST				10
// end of [#RWC6-27]

// [#2185] NH Justin 2013.05.06	Host Type Definition.
#define TRANHOST_ATM					0
#define TRANHOST_DUALHOST				2
#define TRANHOST_PIN4_TRANSACTION		5				// [#2405] US Justin 2016.03.16 HAL Cash Online.
#define TRANHOST_PIN4_REVERSAL			6				// [#2405] US Justin 2016.03.16 HAL Cash Online.
#define TRANHOST_JUSTCASH_TRANSACTION	7				// [#2445] US Justin 2016.09.28 Just.Cash
#define TRANHOST_PAYPAL_TRANSACTION		8				// [#2446] US Justin 2016.09.29 Paypal CCA
#define TRANHOST_GIVEPAY_TRANSACTION	12				// [#2513] US Justin 2017.11.03 GivePay 
#define TRANHOST_PIN4_GETTOKEN			13				// [#2515] US Justin 2017.11.14 PIN4 Prestaging
#define TRANHOST_LIBERTYX				14				// [#RWC6-59] US William 2019.10.08 LibertyX

#define MASTERKEY_ATM					0
#define MASTERKEY_DUALHOST				1
#define MASTERKEY_LIBERTYX				2				// [#RWC6-59] US William 2019.10.08 LibertyX
// End of [#2185]
#define TRANHOST_ALPHI					30				// [#2497] NH woooZ 2017.09.13  ALPHI server

// [#2226] NH Justin 2013.10.22 Add Kernel Version
#define EMV_KERNEL_V4					4
#define EMV_KERNEL_V5					5
#define EMV_KERNEL_V6					6				//[#2435] NH KSK 2016.07.14 Added EMV Kernel V6
#define EMV_KERNEL_V7					7				// [#RWC6-50] US William 2019.09.05 Add EMV Kernel V7
// End of [#2226]

// [#2242] NH Justin 2013.12.17 
#define DYNAMICFLOW_1STCALL_ON			0
#define DYNAMICFLOW_1STCALL_OFF			1
// End of [#2242]

// [#2499] US Justin 2017.08.21
#define DYNAMICFLOW_DISABLE				0
#define DYNAMICFLOW_ENABLE				1
#define DYNAMICFLOW_DUALHOST			2
// End of [#2499]

// [#2341] US Justin 2015.04.29
#define COMMONAID_USE_COMMON			0
#define COMMONAID_USE_ALL				1
#define COMMONAID_USE_INTERNATIONAL		2
// End of [#2341]

// [#2350] US Justin 2015.06.19
#define WITHDRAWAL_ATM					0
#define WITHDRAWAL_POPMONEY				1
#define WITHDRAWAL_PIN4					4		// [#2396] US Justin 2016.02.10 Pin4
#define WITHDRAWAL_JUSTCASH				6		// [#2445] US Justin 2016.09.27 Just.Cash
#define WITHDRAWAL_PAYPAL				7		// [#2446] US Justin 2016.09.29 Paypay CCA
#define WITHDRAWAL_DIGITAL_CUR_JC		8		// [#2496] US Justin 2017.08.16 Just.Cash Bitcoin
#define WITHDRAWAL_PURCHSE_GCARD_GP		9		// [#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
#define WITHDRAWAL_B4U					10
#define WITHDRAWAL_DIGITAL_CUR_CASH_JC	11		// [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash
// End of [#2350]

// [#2354] US Justin 2015.07.01
#define SURCHARGENOTICE_AMOUNT			0
#define SURCHARGENOTICE_PERCENTAGE		1
#define SURCHARGENOTICE_BOTH			2
// End of [#2354]

// [#2517] US Justin 2017.11.29 EMV Fallback Enable/Diable Option
#define EMV_FALLBACK_ENABLE				0
#define EMV_FALLBACK_DISABLE			1
// End of [#2517]

// [#J006] Justin GivePay Enhancement
#define RMS_GIVEPAY_PIGGYBACK		0 // [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6
#define RMS_GIVEPAY_INDIVIDUAL		1 // [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6
#define RMS_GIVEPAY_BOTH			2

#define GIVEPAY_APPEAR_BOTH				0 // [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6
#define GIVEPAY_APPEAR_INDIVIDUAL		1 // [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6
#define GIVEPAY_APPEAR_PIGGYBACK		2
// End of [#J006]


/////////////////////////////////////////////////////////////////////////////
//	TranCtrl Return Value
/////////////////////////////////////////////////////////////////////////////
#define T_OK				0						// OK
#define T_EXIT				1						// EXIT
#define T_CANCEL			2						// CANCEL
#define T_TIMEOVER			3						// TIME OVER
#define T_INPUTOVER			4						// INPUT OVER
#define T_INPUTERROR		5						// INPUT ERROR
#define T_ERROR				6						// DEVICE  ERROR(DEVICE ERROR MSG)
#define T_SENDERROR			7						// SEND    ERROR(BALANCE CONFIRM MSG)
#define T_RECVERROR			8						// RECV    ERROR(BALANCE CONFIRM MSG)
#define T_HOSTERROR			9						// RECV HSOT DENY
#define T_USER_CANCEL		10						// USER CANCELLATION [#56] UK JSW 2008.04.02
#define T_MX_USER_CANCEL	11						// [#631] MX KSK 2010.03.30
#define T_RKT_FAIL			12						// RKT KCV FAIL [#2075] NH KSK 2011.06.28
#define T_PROGRAM			98						// PROGRAM ERROR(REQUEST BANK)
#define T_MSG				99						// CANCEL(MSG)

// [#68] UK HWANG 2008.04.02 EMV Level2
/////////////////////////////////////////////////////////////////////////////
//	Transaction status Define
/////////////////////////////////////////////////////////////////////////////
#define IC_CARD_READ_STATUS			(0)
#define FALLBACK_STATUS				(1)
#define LANGUAGE_SELECT_STATUS		(2)
#define AID_LIST_DIS_STATUS			(3)
#define PASSWORD_ENTRY_STATUS		(4)
#define TRANSACTION_SELECT_STATUS	(5)
// end of [#68]

/////////////////////////////////////////////////////////////////////////////
//	Screen Button Information
/////////////////////////////////////////////////////////////////////////////
#define S_CONTINUE			_T("CONTINUE")				// CONTINUE
#define S_CONFIRM			_T("CONFIRM")				// CONFIRM
#define S_CLEAR				_T("CLEAR")					// CLEAR
#define S_OTHER				_T("OTHER")					// OTHER
#define	S_YES				_T("YES")					// YES
#define	S_NO				_T("NO")					// NO

#define	S_CHECKING			S_ACCOUNT_CHECKING			// [#240] NH 2008.05.28 국가별 Checking Account 표현
#define	S_SAVINGS			_T("SAVINGS")				// SAVINGS
#define	S_CREDITCARD		_T("CREDITCARD")			// CREDIT CARD

/////////////////////////////////////////////////////////////////////////////
//	Op User
/////////////////////////////////////////////////////////////////////////////
#define LOGIN_OPERATOR			1						// OP LOGIN OPERATOR
#define LOGIN_SERVICE			2						// OP LOGIN SERVICE
#define LOGIN_MASTER			3						// OP LOGIN MASTER

/////////////////////////////////////////////////////////////////////////////
// NETSTATE DEFINE	[#358] [NH] KSK 2008.7.10
/////////////////////////////////////////////////////////////////////////////
#define		INIT_STS		0
#define		CONN_STS		1
#define		ENQIN_STS		2	// Only use VISA FRAMED
#define		ACKIN_STS		3	// use VISA FRAMED & ACK CONTROLLED
#define		HIGHLENIN_STS	4	// use STANDARD / ACK CONTROLLED
#define		LOWLENIN_STS	5	// use STANDARD / ACK CONTROLLED
#define		STXIN_STS		6	// Only use VISA FRAMED
#define		ETXIN_STS		7	// Only use VISA FRAMED
#define		EOTWAIT_STS		8	// use VISA FRAMED & ACK CONTROLLED
#define		RKT_SEND_STS	9	// [#2075] NH KSK 2011.06.27

/////////////////////////////////////////////////////////////////////////////
//	Card Information
/////////////////////////////////////////////////////////////////////////////
// [#372] [NH] KSK 2008.7.18
// 삭제하지말고 주석처리만 할 것
/*
typedef struct	tagISO2Form
{
	BYTE MembershipNo[16];							// 000
	BYTE Seperator;									// 016
	BYTE EndDay[4];									// 017
	BYTE ServiceNo[3];								// 021
	BYTE ScrambleNo[4];								// 024
	BYTE Dummy[228];								// 028(256)
} MCISO2;

typedef struct	tagISO3Form
{
	BYTE FormatCode[2]; 							// 000
	BYTE IdNo[2]; 									// 002
	BYTE BankNo[3];									// 004
	BYTE Separator1;								// 007
	BYTE AccountNo[16];								// 008
	BYTE AccountCheck;								// 024
	BYTE Separator2;								// 025
	BYTE Nation[3]; 								// 026
	BYTE CashCode[3];								// 029
	BYTE Unknown1[15];								// 032
	BYTE ValidCode;									// 047
	BYTE Password[6];								// 048
	BYTE Trade;										// 054
	BYTE Tran1[2];									// 055
	BYTE Tran2[4];									// 057
	BYTE OtherData[27];								// 061
	BYTE Remark[16];								// 088
	BYTE Dummy[152];								// 104(256)
} MCISO3;
*/
// end of [#372]
/////////////////////////////////////////////////////////////////////////////
//	Magnetic Card Area
/////////////////////////////////////////////////////////////////////////////
//typedef struct	tagMCVW 
//{
//	BYTE TrInfo;
//	WORD JISsize;
//	BYTE JISBuff[2];         
//	WORD ISO1size;               
//	BYTE ISO1Buff[256];     
//	WORD ISO2size;              
//	BYTE ISO2Buff[256];    
//	WORD ISO3size;             
//	BYTE ISO3Buff[256];   
//} MCAP;

typedef struct tagNH_CARDDATA
{
	CString	strISO1Data;
	CString	strISO2Data;
	CString	strISO3Data;

	CString	strBankID;
	CString	strAccountNo;
	CString	strShowNumber;
	
} NH_CARDDATA, *PNH_CARDDATA;

typedef struct	tagNH_USERSELECTION
{
	CString		strPassword;
	CString		strMoney;

	// [#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA
	int			nCST1Cnt;		
	int			nCST2Cnt;		
	int			nCST3Cnt;	
	int			nCST4Cnt;		
	int			nCST1Denom;	
	int			nCST2Denom;		
	int			nCST3Denom;	
	int			nCST4Denom;		
	// end of [#RWC6-12, #2584]

	int			nPrintReceipt;	// [#2219] 2013.09.04 NH Justin Digital Receipt, Change Variable type and Name: BOOL(bPrint...) => Int (nPrint...)
	BOOL		bScreenDisplay;	// 호주의 경우 사용자가 화면 출력을 선택하였을 때만 화면 출력하고 
								// 그 외의 경우는 화면 출력하지 않는다 //[#2000] 2010.10.17 SOOK 호주 사양 적용 (거래 FLOW )
	CString		strNewPassword;	// [#2150] US Justin 2012.10.01 Add PIN Change Transaction
} NH_USERSELECTION, *PNH_USERSELECTION;


//////////////////////////////////////////////////////////////////////////
// EMV AID Struct Define
//////////////////////////////////////////////////////////////////////////
//[#68] UK HWANG 2008.04.02 EMV Level2
#define EMV_MAX_AIDLIST			(7)

typedef struct tagAIDLIST
{
	int			nSelFlag;
	int			nAID;
	int			nSelAid;
	DWORD		selectedAID[EMV_MAX_AIDLIST];
	DWORD		nRemain;
	char		AidName[32][17];
	char		ApplicationID[32][33];
	char		DefaultAID[32][33];

	// [#559] [CA] KSK 2009.8.19 ASF 처리를 위한 structure구성
	int			nASFCount;
	char		Priority[32];
	char		IssuerCountryCode[32][4];
	char		ASF_ApplicationSelectFlag[32];
	int			nResult;		// PRIMARY Sequence인지 SECONDARY SEQUENCE인지 판단하기 위한 변수
	// end of [#559]
} AIDLIST;
// end of [#68]

/////////////////////////////////////////////////////////////////////////////
//	Button F1 ~ F8 Define
/////////////////////////////////////////////////////////////////////////////
#define	BTN_F1_SET			0x01
#define	BTN_F2_SET			0x02
#define	BTN_F3_SET			0x04
#define	BTN_F4_SET			0x08
#define	BTN_F5_SET			0x10
#define	BTN_F6_SET			0x20
#define	BTN_F7_SET			0x40
#define	BTN_F8_SET			0x80

#define	MENU_SELECT_SIZE	8

/////////////////////////////////////////////////////////////////////////////
//	ElecJournal Form(TXT)
/////////////////////////////////////////////////////////////////////////////
#define	SPR_MAX_COL			40		
#define	JPR_MAX_ROW			35
#define	JPR_MAX_COL			40						// JPR SIZE > SPR SIZE(ElecJournal)

/////////////////////////////////////////////////////////////////////////////
//	Journal Code
/////////////////////////////////////////////////////////////////////////////
#define ADDCASH_CST1					L"A1"
#define ADDCASH_CST2					L"A2"
#define ADDCASH_CST1_2					L"A3"
#define ADDCASH_CST3					L"A4"
#define ADDCASH_CST1_3					L"A5"
#define ADDCASH_CST2_3					L"A6"
#define ADDCASH_CST1_2_3				L"A7"
#define ADDCASH_CST4					L"A8"
#define ADDCASH_CST1_4					L"A9"
#define ADDCASH_CST2_4					L"AA"
#define ADDCASH_CST1_2_4				L"AB"
#define ADDCASH_CST3_4					L"AC"
#define ADDCASH_CST1_3_4				L"AD"
#define ADDCASH_CST2_3_4				L"AE"
#define ADDCASH_CST1_2_3_4				L"AF"

#define POPMONEY_DAY_TOTAL				L"B3"	// [#2442] US Justin 2016.08.24 POPMONEY Day Total
#define POPMONEY_TRIALDAY_TOTAL			L"B4"	// [#2442] US Justin 2016.08.24 POPMONEY Trial Day Total
#define JUSTCASH_DAY_TOTAL				L"B5"	// [#2445] US Justin 2016.09.22 JUSTCASH Day Total
#define JUSTCASH_TRIALDAY_TOTAL			L"B6"	// [#2445] US Justin 2016.09.22 JUSTCASH Trial Day Total
#define PAYPAL_DAY_TOTAL				L"B7"	// [#2446] US Justin 2016.09.29 Paypal CCA Day Total
#define PAYPAL_TRIALDAY_TOTAL			L"B8"	// [#2446] US Justin 2016.09.29 Paypal CCA Trial Day Total

#define DAY_TOTAL						L"BA"
#define CST_TOTAL						L"BB"
#define TRIALDAY_TOTAL					L"BC"
#define TRIALCST_TOTAL					L"BD"
#define DIGITALCUR_DAY_TOTAL			L"BG"	// [#2496] US Justin 2017.08.17 Digital Currency Day Total
#define DIGITALCUR_TRIALDAY_TOTAL		L"BH"	// [#2445] US Justin 2017.08.17 Digital Currency Trial Day Total

#define CASHIN_TOTAL					L"BI"	// [#GLDV-3005] US Kook 2022.04.14 support side car
#define TRIALCASHIN_TOTAL				L"BJ"	// [#GLDV-3005] US Kook 2022.04.14 support side car

#define PIN4_DAY_TOTAL					L"BW"	// [#2405] US Justin 2016.03.21 HalCash Day Total
#define PIN4_TRIALDAY_TOTAL				L"BX"	// [#2405] US Justin 2016.01.11 HalCash Trial Day Total

#define B4U_DAY_TOTAL					L"C1"	// B4U Day Total
#define B4U_TRIALDAY_TOTAL				L"C2"	// B4U Trial Day Total
#define B4U_TXN							L"C3"	// B4U Transaction
#define B4U_TXN_ERROR_FAIL				L"C4"	// B4U Transaction Error (Fail)
#define B4U_TXN_ERROR_CANCEL			L"C5"	// B4U Transaction Error (Cancel)
#define B4U_TXN_ERROR_TIMEOUT			L"C6"	// B4U Transaction Error (Timeout)

#define CHANGE_PARAMETER				L"CP"	//[#610] SOOK 2009.12.21 Configuration Change Journal 저장 

#define CHANGE_DENOM_CST1				L"D1"
#define CHANGE_DENOM_CST2				L"D2"
#define CHANGE_DENOM_CST1_2				L"D3"
#define CHANGE_DENOM_CST3				L"D4"
#define CHANGE_DENOM_CST1_3				L"D5"
#define CHANGE_DENOM_CST2_3				L"D6"
#define CHANGE_DENOM_CST1_2_3			L"D7"
#define CHANGE_DENOM_CST4				L"D8"
#define CHANGE_DENOM_CST1_4				L"D9"
#define CHANGE_DENOM_CST2_4				L"DA"
#define CHANGE_DENOM_CST1_2_4			L"DB"
#define CHANGE_DENOM_CST3_4				L"DC"
#define CHANGE_DENOM_CST1_3_4			L"DD"
#define CHANGE_DENOM_CST2_3_4			L"DE"
#define CHANGE_DENOM_CST1_2_3_4			L"DF"

// [#530] NH KSK 2009.06.09
#define REMOTE_ADDCASH_CST1				L"E1"
#define REMOTE_ADDCASH_CST2				L"E2"
#define REMOTE_ADDCASH_CST1_2			L"E3"
#define REMOTE_ADDCASH_CST3				L"E4"
#define REMOTE_ADDCASH_CST1_3			L"E5"
#define REMOTE_ADDCASH_CST2_3			L"E6"
#define REMOTE_ADDCASH_CST1_2_3			L"E7"
#define REMOTE_ADDCASH_CST4				L"E8"
#define REMOTE_ADDCASH_CST1_4			L"E9"
#define REMOTE_ADDCASH_CST2_4			L"EA"
#define REMOTE_ADDCASH_CST1_2_4			L"EB"
#define REMOTE_ADDCASH_CST3_4			L"EC"
#define REMOTE_ADDCASH_CST1_3_4			L"ED"
#define REMOTE_ADDCASH_CST2_3_4			L"EE"
#define REMOTE_ADDCASH_CST1_2_3_4		L"EF"
// end of [#530]

#define REMOTE_DAY_TOTAL				L"FA"	// [#532] NH KSK 2009.06.09
#define REMOTE_CST_TOTAL				L"FB"	// [#531] NH KSK 2009.06.09
#define REMOTE_TRIALDAY_TOTAL			L"FC"	// [#532] NH KSK 2009.06.09

//#define CHANGE_MESSAGE				L"HA"
#define CHANGE_PROCESSOR				L"HB"

#define LIBERTYX_TXN					L"LT"	// [#RWC6-59] US William 2019.10.28 LibertyX
#define LIBERTYX_TXN_ERROR				L"LX"	// [#RWC6-59] US William 2019.10.28 LibertyX
#define LIBERTYX_TXN_DISPENSE			L"LD"	// [#RWC6-224] US William 2020.10.12 Liberty Dispense

#define DIGITALMINT_TXN					L"MT"	// [#RWC6-399] US ryan.payton 2022.08.02 DigitalMint
#define DIGITALMINT_TXN_ERROR			L"MX"	// [#RWC6-399] US ryan.payton 2022.08.02 DigitalMint

#define TANGOPAY_TXN					L"WA"	// [#RWC6-676] SKKim 2024.05.09 TangoPay (Pivot과 동일한 코드로 설정)
#define TANGOPAY_TXN_ERROR				L"WX"	// [#RWC6-676] SKKim 2024.05.29


#define CANCEL_AT_READ_CARD				L"K0"
#define CANCEL_AT_SELECT_LANGUAGE		L"K1"
#define CANCEL_AT_INPUT_PIN				L"K2"
#define CANCEL_AT_SELECT_TRAN			L"K3"
#define CANCEL_AT_CONFIRM_FEE			L"K4"
#define CANCEL_AT_RECV_RECEIPT			L"K5"
#define CANCEL_AT_RECEIPT_ERROR			L"K6"
#define CANCEL_AT_RECEIPT_SCREEN		L"K7"
#define CANCEL_AT_FROM_ACCOUNT			L"K8"
#define CANCEL_AT_TO_ACCOUNT			L"K9"
#define CANCEL_AT_FASTCASH				L"KA"
#define CANCEL_AT_OTHER_AMOUNT			L"KB"
#define CANCEL_AT_TRANSFER_AMOUNT		L"KC"

#define OPERATOR_ACTION					L"OA"
#define MONIVIEW_ACTION					L"OB"	//[#610] SOOK 2010.01.18 Configuration Change Journal 저장 
#define HOST_ACTION						L"OC"	//[#610] SOOK 2010.01.18 Configuration Change Journal 저장 
#define TRX_EMV_DATA					L"OD"	// [#2115] MX KSK 2012.02.04 Journal에 EMV DATA 저장 
#define ALPHI_TRX_ERROR					L"OE"	// [#2541] AU HJAHN 2018.03.19 ALPHI Detail error messages 

#define CHANGE_MASTER_PWD				L"PA"
#define CHANGE_OPERATOR_PWD				L"PB"
#define CHANGE_SUPERVISOR_PWD			L"PC"

#define CHANGE_EXCHANGE_RATE			L"RA"	// KSK 2009.3.30

#define POWER_ON_SYSTEM					L"SA"
#define CHANGE_SERVICE_MODE				L"SB"
#define CHANGE_ERROR_MODE				L"SC"
#define CHANGE_OP_MODE					L"SD"
#define CHANGE_OP_RESET					L"SE"
#define REBOOT_SYSTEM					L"SF"
#define CHANGE_DIAG_MODE				L"SV"
#define POWER_OFF_SYSTEM				L"SG"

#define NORMAL_TRX						L"TA"
#define TRX_ERROR						L"TB"
#define REVERSAL_TRX					L"TC"
//#define POWER_FAIL_TRX				L"TD"	// KSK 2010.03.30 사용 안함
#define TRX_NOT_APPROVED				L"TE"
#define COMMUNICATION_ERROR				L"TF"

// [#416] [NZ] AIREAT 2008.09.08
#define AUTO_DAY_TOTAL						1
#define AUTO_TRIAL_DAY_TOTAL				2
// end of [#416]

// [#546] SOOK 2009.07.24 다기능 BIN LIST 기능 추가함 

#define ACTION_BLOCK_SURCHARGE			0
#define ACTION_WITHDRAWAL_SURCHARGE		1
#define ACTION_BALANCE_SURCHARGE		2
#define ACTION_ACCEPT_CARD				3
#define ACTION_DECLINE_CARD				4
#define ACTION_AWARD_COUPON				5
#define ACTION_PIN_CHANGE				6
//end of [#546]

// [#2487] AU KSK 2017.06.08 Define Surcharge Mode at Idle Screen
#define NO_DISPLAY_SURCHARGE				0
#define STANDARD_SURCHARGE					1
#define BINLISTED_SURCHARGE					2
#define PERCENTAGE_N_BINLISTED_SURCHARGE	3
#define PERCENTAGE_SURCHARGE				4
// end of [#2487]

// [#2134] NH KSK 2012.05.07
#define WORKINGKEY_INIT					0
#define WORKINGKEY_DOWNLOAD_SUCCESS		1
#define WORKINGKEY_DOWNLOAD_FAIL		2
#define PIN_MAC_BLANK_ERROR				3
// end of [#2134]

// [#2518] US Kook 2018.01.14 Support Camera for MX-2800SE

#define UNIT_KILOBYTE						(1024)										// 1,024 bytes
#define UNIT_MEGABYTE						(1024 * UNIT_KILOBYTE)						// 1,048,576 bytes (=1 MB)
#define UNIT_GIGABYTE						(ULONGLONG)(1024 * UNIT_MEGABYTE)			// 1,073,741,824 bytes (=1 GB)
#define MONIAIR_DATA_USAGE_LIMIT			(ULONGLONG)((4 * UNIT_GIGABYTE)-1)			// 4,294,967,295 bytes (=3.99 GB)

#define		NH_CAM_FACE_IMAGE_LOW_SPACE_SIZE		(2 * UNIT_MEGABYTE)
#define		NH_CAM_FACE_IMAGE_CNT_MAX_LOW_SPACE		(100)		// limit max count when ATM2 has low space. (< 2MB)
#define		NH_CAM_FACE_IMAGE_CNT_MAX				(10000)		// WinCE has a limit of 999 files max per folder.
#define		NH_CAM_FACE_IMAGE_CNT_PER_DIR			(500)

#define		NH_CAM_FACE_IMAGE_FOLDER		ATM2_PATH _T("\\JNL\\CAMIMAGE")
#define		NH_CAM_CONFIG_FILE_PATH			ATM_PATH _T("\\hucConfig.ini")

#define		NH_CAM_RESOLUTION_WIDTH			(320)
#define		NH_CAM_RESOLUTION_HEIGHT		(240)

#define		NH_CAM_POSITION_X_LEFT			(0)
#define		NH_CAM_POSITION_X_CENTER		((1024 - NH_CAM_RESOLUTION_WIDTH) / 2)
#define		NH_CAM_POSITION_X_RIGHT			(1024 - NH_CAM_RESOLUTION_WIDTH)
#define		NH_CAM_POSITION_Y_TOP			(0)
#define		NH_CAM_POSITION_Y_CENTER		((768 - NH_CAM_RESOLUTION_HEIGHT) / 2)
#define		NH_CAM_POSITION_Y_BOTTOM		(768 - NH_CAM_RESOLUTION_HEIGHT)

// Non-Wyvern (NW) positions
#define		NH_CAM_NW_RESOLUTION_WIDTH		(240)
#define		NH_CAM_NW_RESOLUTION_HEIGHT		(180)
#define		NH_CAM_NW_POSITION_X_CENTER		(0)
#define		NH_CAM_NW_POSITION_Y_BOTTOM		(768 - NH_CAM_NW_RESOLUTION_HEIGHT)

#define		NH_CAM_ECODE					_T("97020")

typedef enum NH_CAM_TYPE
{
	CAM_MAIN		= 0,
	CAM_SUB			= 1
};

typedef enum NH_CAM_TAKEN_TIME
{
	TIME_PIN_ENTERED		= 0,
	TIME_CASH_DISPENSED		= 1,
	TIME_RECEIPT_PROCESSING	= 2,
	TIME_NO_VALUE			= 9
};
// end of [#2518]

/////////////////////////////////////////////////////////////////////////////
#endif
