/////////////////////////////////////////////////////////////////////////////
#if !defined(_DEVDEFINE_H_INCLUDED_)
#define _DEVDEFINE_H_INCLUDED_

/////////////////////////////////////////////////////////////////////////////
//	Cash Condition Count Define(Mecha)
/////////////////////////////////////////////////////////////////////////////
#define CDU_EMPTYCASHCNT	0						// FOR US, FIX COUNT AS 0
													// EMPTY CASH COUNT : 2004.11.05
/////////////////////////////////////////////////////////////////////////////
//	Timer
/////////////////////////////////////////////////////////////////////////////
#define	TIMER_THREAD			1					// TIMER THREAD

/////////////////////////////////////////////////////////////////////////////
//	Transaction Types
/////////////////////////////////////////////////////////////////////////////
#define	TRAN_REVERSAL					13			// REVERSAL
#define	TRAN_TRAN						20			// TRANSACTION		(INQ,TFR,CWD,PIN)

// [#369] [NH] KSK 2008.7.18
#define	TRANTYPE_WITHDRAWAL			_T("CW")
#define	TRANTYPE_INQUIRY			_T("BI")
#define	TRANTYPE_TRANSFER			_T("TR")
#define	TRANTYPE_CHECKCASHING		_T("CC")
// end of [#369]
#define	TRANTYPE_DCC				_T("DC")		// [#742] US Justin 2013.12.26 DCC
#define	TRAN_DF_OK					_T("OK")		// [#742] US Justin 2013.12.26 DCC

//////////////////////////////////////////////////////////////////////////////////
// Message Type (1: Tranax, 2: CSP200(Diebold), 3: Triton) => ALL VISA II PROTOCOL
//////////////////////////////////////////////////////////////////////////////////
#define	MSG_HYOSUNG_TYPE				_T("1")		// MSG TRANAX TYPE
#define	MSG_CSP200_TYPE					_T("2")		// MSG CSP200 TYPE
#define	MSG_TRITON_TYPE					_T("3")		// MSG TRITON TYPE
#define	MSG_EPS_TYPE					_T("5")		// [#98] NH 2008.04.08 KGS EPS MessageFormat 선택 메뉴 추가

#define	BINLIST_SIZE			20
#define BINLIST_START_INDEX		3503

// [#742] US Justin 2013.12.24 DCC
#define	SURCHARGE_DISP_NORMAL			0
#define	SURCHARGE_DISP_PRECENT			1
#define	SURCHARGE_DISP_TABLE			2
#define	SURCHARGE_DISP_DYNAMIC1ST		3
#define	SURCHARGE_DISP_DYNAMIC2ND		4
#define	SURCHARGE_DISP_DYNAMICDCC		5
#define	SURCHARGE_DISP_BALANCEINQUIRY	6
#define	SURCHARGE_DISP_PINCHANGE		7
// End of [#742]
// [#762] US Justin 2014.10.21 Add TDL
#define	SURCHARGE_DISP_TDL_SEGMENTATION	8
#define	SURCHARGE_DISP_TDL_DCC			9
// End of [#762]

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
#define DHDCCTOTALINFO_DATA_SIZE 3		// [#742] US Justin 2014.01.16 Dual Host DCC

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
#define _VAR_EMV_CONFIG					7500			// [#789] US Justin 2015.05.19 AID Selection


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
#define	_MEM_FLD_INI_DHDCCINFO		_FIELD_NO + 24		// [#742] US Justin 2014.01.16 Dual Host DCC
#define	_MEM_FLD_EMVCONFIG			_FIELD_NO + 25		// [#789] US Justin 2015.05.19 AID Selection

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
#define _MEM_VAR_APP_ISO1_FLAG					_VAR_APP_NO+47 // _T("CardIso1")				- RMS만 사용
#define _MEM_VAR_APP_ISO2_FLAG					_VAR_APP_NO+48	// _T("CardIso2")				- RMS만 사용
#define _MEM_VAR_APP_ISO3_FLAG					_VAR_APP_NO+49	// _T("CardIso3")				- RMS만 사용
#define _MEM_VAR_APP_ISO1DATA					_VAR_APP_NO+50 // _T("CardIso1")
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

#define _MEM_VAR_APP_CALC_REQ_MXDOLLAR			_VAR_APP_NO+89	// [#519] [NH] KSK 2009.3.10
#define _MEM_VAR_APP_CALC_DISP_MXDOLLAR			_VAR_APP_NO+90	// [#519] [NH] KSK 2009.3.10
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

// [#659] MX KJW 2010.09.16 멕시코 '&' 필드 수신시 RMS저널의 Other메시지에 추가
#define _MEM_VAR_APP_RETURNED_ACCNT_NO			_VAR_APP_NO+102
// end of [#659]

#define _MEM_VAR_APP_LOG_VERSION				_VAR_APP_NO+103		// [#708] NH KSK 2011.10.24
// end of [#632]

#define	_MEM_VAR_APP_TRANSACTIONCNT_FLAG		_VAR_APP_NO+104			// [#726] MX KSK 2012.07.17
#define	_MEM_VAR_APP_CUSTOMER_TYPE				_VAR_APP_NO+105			// [#726] MX KSK 2012.07.17
#define	_MEM_VAR_APP_EXCHANGE_FEE				_VAR_APP_NO+106			// [#726] MX KSK 2012.07.17
//#define	_MEM_VAR_APP_EXCHANGE_TOTAL_AMOUNT		_VAR_APP_NO+107		// [#726] MX KSK 2012.07.17 [#729] MX KSK 2012.08.09 Total Fee로 변경됨
#define	_MEM_VAR_APP_TOTAL_FEE					_VAR_APP_NO+107			// [#726] MX KSK 2012.07.17 [#729] MX KSK 2012.08.09 Total Fee로 변경됨
#define	_MEM_VAR_APP_EXCHANGE_RATE				_VAR_APP_NO+108			// [#726] MX KSK 2012.07.17
#define	_MEM_VAR_APP_REVERSAL_EXCHANGE_FEE		_VAR_APP_NO+109			// [#729] MX KSK 2012.08.14

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

// [#742] US Justin 2014.01.16 Dual Host DCC
#define _MEM_VAR_SUM_DHDCC_AMOUNT				_VAR_SUM_NO+60	// DUALHOSTDCCINFO
#define _MEM_VAR_SUM_DHDCC_COUNT				_VAR_SUM_NO+70	// DUALHOSTDCCINFO
// end of [#742]


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
#define	_MEM_VAR_OPT_WITHDRAWAL_SURCHARGE		_VAR_OPT_NO + 22
#define _MEM_VAR_OPT_BALANCE_SURCHARGE			_VAR_OPT_NO + 23
#define	_MEM_VAR_OPT_PINCHANGE_SURCHARGE		_VAR_OPT_NO + 24
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
//#define _MEM_VAR_OPT_DID_CONFIGURATION			_VAR_OPT_NO + 42		// [#353] AU AIREAT 2008.06.29 -- 사용하지 않음.
#define	_MEM_VAR_OPT_CHECKCASHING_ENABLE		_VAR_OPT_NO + 42		// [#366] [NH] KSK 2008.7.15
#define	_MEM_VAR_OPT_PINOPTION_ENABLE			_VAR_OPT_NO + 43		// [#366] [NH] KSK 2008.7.15
#define	_MEM_VAR_OPT_CHECKCASHING_CUSTOMERNO	_VAR_OPT_NO + 44		// [#366] [NH] KSK 2008.7.15
#define	_MEM_VAR_OPT_CHECKCASHING_AVAILABLE		_VAR_OPT_NO + 45		// [#378] [NH] KSK 2008.7.15
#define	_MEM_VAR_OPT_CHECKCASHING_CERTIFYKEY	_VAR_OPT_NO + 46		// [#396] [US] KSK 2008.8.6
#define _MEM_VAR_OPT_EMV_AVAILABLE				_VAR_OPT_NO + 47		// [#397] [NH] psc 2008.08.01
#define _MEM_VAR_OPT_EMV_ENABLE					_VAR_OPT_NO + 48		// [#397] [NH] psc 2008.08.01
#define _MEM_VAR_OPT_SURCHARGE_WARNINGLOCATION	_VAR_OPT_NO + 49
#define _MEM_VAR_OPT_EMV_TRANSACTIONOPTION		_VAR_OPT_NO + 50		// [#397] [NH] psc 2008.08.21
#define _MEM_VAR_OPT_CHECKCASHING_TYPE			_VAR_OPT_NO + 51		// [#410] [US] KSK 2008.08.29
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
//#define _MEM_VAR_EMV_SERIALNUMBER					_VAR_EMVINFO_NO
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

#define _MEM_VAR_AC_REQ_9F26_VALUE					_VAR_EMV_REVERSAL + 26		// [#706] MX KSK 2011.10.19
#define _MEM_VAR_EMV_APPLICATION_NAME				_VAR_EMV_REVERSAL + 27		// [#745] US Justin 2014.02.04 Long AID and Name Bug Fix

#define _MEM_VAR_EMV_CONFIG_ENABLEDAID				_VAR_EMV_CONFIG	+ 0			// [#789] US Justin 2015.05.19 AID Selection

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
#define _MEM_VAR_OPT2_EXCHANGE_RATE					_VAR_OPT2_NO + 6		// [#514][MX] KSK 2009.3.4
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

// [#631] MX, 2010.03.29 JERRY
#define _MEM_VAR_OPT2_BANK_NAME_FEE_PRINT			_VAR_OPT2_NO + 41     
// end of [#631]

//[#648] NH KJW 2010.08.24
#define _MEM_VAR_OPT2_NOTICE_SERVICE_ENABLE			_VAR_OPT2_NO + 42
#define _MEM_VAR_OPT2_NOTICE_SERVICE_TITLE			_VAR_OPT2_NO + 43
#define _MEM_VAR_OPT2_NOTICE_SERVICE_MSG1			_VAR_OPT2_NO + 44
#define _MEM_VAR_OPT2_NOTICE_SERVICE_MSG2			_VAR_OPT2_NO + 45
#define _MEM_VAR_OPT2_NOTICE_SERVICE_MSG3			_VAR_OPT2_NO + 46
#define _MEM_VAR_OPT2_TOUCHVIBRATION_ENABLE			_VAR_OPT2_NO + 47
#define _MEM_VAR_OPT2_SSLVERSION					_VAR_OPT2_NO + 48
// end of [#648]

#define _MEM_VAR_OPT2_MX_DP_SELECTION				_VAR_OPT2_NO + 49	// [#669] MX KSK 2010.10.30

#define _MEM_VAR_OPT2_AU_COUNTRY_SELECTION			_VAR_OPT2_NO + 50	// [#692] AU KSK 2011.03.03

//[#710][#711] NH PCS 2011.10.25 "WinCE5.0과 동일하게 맞춤."
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
#define _MEM_VAR_OPT2_SSL_CERTIFY					_VAR_OPT2_NO + 61		// [#2092] US PCS 2011.10.24 "SSL Certify 
//end of [#710][#711] NH PCS 2011.10.25
#define _MEM_VAR_OPT2_MX_EXCHANGEFEEFORMAT			_VAR_OPT2_NO + 62		// [#734] MX Justin 2012.12.13 Exchange Fee Format

// [#742] NH Justin 2013.12.20 DCC
#define _MEM_VAR_OPT2_DF_ENABLE						_VAR_OPT2_NO + 63
#define _MEM_VAR_OPT2_DF_1STCALL_OPTION				_VAR_OPT2_NO + 64

#define _MEM_VAR_OPT2_DF_PARTNERID					_VAR_OPT2_NO + 65
#define _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE			_VAR_OPT2_NO + 66
#define _MEM_VAR_OPT2_DF_SERVICE_MPT				_VAR_OPT2_NO + 67
#define _MEM_VAR_OPT2_DF_SERVICE_DCC				_VAR_OPT2_NO + 68
#define _MEM_VAR_OPT2_DF_DCC_DISCLAIMER1			_VAR_OPT2_NO + 69
#define _MEM_VAR_OPT2_DF_DCC_DISCLAIMER2			_VAR_OPT2_NO + 70
#define _MEM_VAR_OPT2_DF_DCC_DISCLAIMER3			_VAR_OPT2_NO + 71
#define _MEM_VAR_OPT2_DF_DCC_DISCLAIMER4			_VAR_OPT2_NO + 72
#define _MEM_VAR_OPT2_DF_DCC_DISCLAIMER5			_VAR_OPT2_NO + 73
#define _MEM_VAR_OPT2_DF_DCC_DISCLAIMER6			_VAR_OPT2_NO + 74

#define	_MEM_VAR_OPT2_DCC_CUSTOMOPTION				_VAR_OPT2_NO + 75
#define	_MEM_VAR_OPT2_DCC_DUALHOSTTAN				_VAR_OPT2_NO + 76

#define	_MEM_VAR_OPT2_DUALHOSTDCC_ENABLE			_VAR_OPT2_NO + 77
#define _MEM_VAR_OPT2_DUALHOSTDCC_SSL_ENABLE		_VAR_OPT2_NO + 78
#define _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION		_VAR_OPT2_NO + 79
#define	_MEM_VAR_OPT2_DUALHOSTDCC_ISHOSTUSEURL		_VAR_OPT2_NO + 80
#define	_MEM_VAR_OPT2_DUALHOSTDCC_HOST1NAME			_VAR_OPT2_NO + 81
#define _MEM_VAR_OPT2_DUALHOSTDCC_HOST1PORTNO		_VAR_OPT2_NO + 82
#define	_MEM_VAR_OPT2_DUALHOSTDCC_HOST2NAME			_VAR_OPT2_NO + 83
#define	_MEM_VAR_OPT2_DUALHOSTDCC_HOST2PORTNO		_VAR_OPT2_NO + 84

#define _MEM_VAR_OPT2_EMV_CONTINUE_FALLBACK			_VAR_OPT2_NO + 85

#define _MEM_VAR_OPT2_EMV_KERNELVERSION				_VAR_OPT2_NO + 86
#define _MEM_VAR_OPT2_EMV_AIDENDISABLE				_VAR_OPT2_NO + 87
// End of [#742]

#define _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE		_VAR_OPT2_NO + 88		// [#762] US Justin 2014.10.21 Cardtronics Dual Balance Option
#define _MEM_VAR_OPT2_CUSTOMIZED_TIRSURCHARGE		_VAR_OPT2_NO + 89		// [#762] US Justin 2014.10.21 Cardtronics Surcharge Segmentation
#define _MEM_VAR_OPT2_CUSTOMIZED_DCC				_VAR_OPT2_NO + 90		// [#762] US Justin 2014.10.21 Cardtronics DCC
#define _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE	_VAR_OPT2_NO + 91		// [#762] US Justin 2014.10.21 Cardtronics Credit Withdrawal Surcharge

#define _MEM_VAR_OPT2_BALANCE_NOFEE_NOTICE			_VAR_OPT2_NO + 92		// [#766] US Justin 2014.11.24 No Fee Notice for Balance Inquiry
#define _MEM_VAR_OPT2_HOT_KEY_STRING				_VAR_OPT2_NO + 93		// [#772] NH Justin 2014.12.10 Hot Key String
#define _MEM_VAR_OPT2_DECIMAL_SURCHARGE				_VAR_OPT2_NO + 94		// [#774] NH Justin 2014.12.15 Decimal Surcharge
#define _MEM_VAR_OPT2_PERCENT_SURCHARGE_NOTICE		_VAR_OPT2_NO + 95		// [#782] NH Justin 2015.03.13 Percentage Surcharge Notice
#define _MEM_VAR_OPT2_USE_USCOMMONAID				_VAR_OPT2_NO + 96		// [#788] US Justin 2015.05.19 US Common AID Option
#define	_MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE		_VAR_OPT2_NO + 97		// [#794] US Justin 2015.06.09 DCC Lookup Use Local Surcharge
#define _MEM_VAR_OPT2_EMV_LANGUAGESELECTION			_VAR_OPT2_NO + 98		// [#825] US Justin 2016.05.27
#define _MEM_VAR_OPT2_CUSTOMIZED_SURCHARGE			_VAR_OPT2_NO + 99		// [#833] US Justin 2016.07.25

#define _MEM_VAR_OPT2_DISPENSE_MIX_OPTION			_VAR_OPT2_NO +100		// [#846] AU KSK 2017.01.12

#define	_MEM_VAR_OPT2_DCC_OPTION_ELIGIBLE			_VAR_OPT2_NO +101		// [#854] US Justin 2017.02.22 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_MAXWITHDRAWAL		_VAR_OPT2_NO +102		// [#854] US Justin 2017.02.22 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_FASTCASH1			_VAR_OPT2_NO +103		// [#854] US Justin 2017.02.22 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_FASTCASH2			_VAR_OPT2_NO +104		// [#854] US Justin 2017.02.22 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_FASTCASH3			_VAR_OPT2_NO +105		// [#854] US Justin 2017.02.22 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_FASTCASH4			_VAR_OPT2_NO +106		// [#854] US Justin 2017.02.22 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_FASTCASH5			_VAR_OPT2_NO +107		// [#854] US Justin 2017.02.22 DCC Withdrawal Option
#define	_MEM_VAR_OPT2_DCC_OPTION_FASTCASH6			_VAR_OPT2_NO +108		// [#854] US Justin 2017.02.22 DCC Withdrawal Option

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

/////////////////////////////////////////////////////////////////////////////
//	ATM Mode Define
/////////////////////////////////////////////////////////////////////////////
#define ATM_INIT			0						// INIT		MODE
#define ATM_ERROR			1						// ERROR	MODE
#define ATM_CLERK			2						// CLERK	MODE
#define ATM_READY			3						// READY	MODE
#define ATM_CUSTOM			4						// CUSTOM	MODE 
#define ATM_TRAN			5						// TRAN		MODE
#define ATM_DOWN			6						// DOWN		MODE	[#24] KSK 2008.04.08
#define ATM_REBOOT			7						// REBOOT	MODE
#define ATM_RMSACTIVE		8						// RMS ACTIVE MODE
#define	ATM_CHECKCASHING	9						// CHECK CASHING MODE	[#369] [US] KSK 2008.7.16

/////////////////////////////////////////////////////////////////////////////
//	Host Mode Define
/////////////////////////////////////////////////////////////////////////////
#define HOST_OFFLINE 		0						// OFFLINE
#define HOST_WAITONLINE		1						// WAITONLINE
#define HOST_ONLINE			2						// ONLINE

/////////////////////////////////////////////////////////////////////////////
//	Transaction Mode Define
/////////////////////////////////////////////////////////////////////////////
#define TRAN_IDLE 			0						// IDLE

/////////////////////////////////////////////////////////////////////////////
//	Buffer Size
/////////////////////////////////////////////////////////////////////////////
#define	NETBUFFSIZE			2048

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

#define	PIN_NUM_ALL_PAD			_T("1,2,3,4,5,6,7,8,9,0,.,00,CANCEL,CLEAR,ENTER")
#define	PIN_PASSWORD_PAD		_T("1,2,3,4,5,6,7,8,9,0,CANCEL,CLEAR,ENTER,F8")

#define	PIN_AMOUNT_PAD			_T("1,2,3,4,5,6,7,8,9,0,CANCEL,CLEAR,ENTER,F8")
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
#define	CMD_PRINT_IMAGE					_T("PRINTIMAGE")

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

// [#648] NH KJW 2010.08.30
/////////////////////////////////////////////////////////////////////////////
// SSL VERSION
/////////////////////////////////////////////////////////////////////////////
//#define SSL_VER_23						1
//#define SSL_VER_3						2
// end of [#648]

// [#709] NH KSK 2011.10.20 Certify 기능 추가
//#define	SSL_VER_23_USE_CERTIFY			3
//#define SSL_VER_3_USE_CERTIFY			4
// end of [#709]

//[#775] NH Justin 2015.01.14 Support NON-"Version Negotiation" Processor
/*
//#define SSL_VER3_AND_ABOVE						18	// [#767] NH KSK 2014.11.18 Added Option
//#define TLS_VER10_AND_ABOVE						19	// [#767] NH KSK 2014.11.18 Added Option
//#define TLS_VER11_AND_ABOVE						20	// [#767] NH KSK 2014.11.18 Added Option
//#define TLS_VER12									21	// [#767] NH KSK 2014.11.18 Added Option
//#define SSL_VER3_AND_ABOVE_LINE_TEST				22	// [#767] NH KSK 2014.11.18 Added Option
//#define TLS_VER10_AND_ABOVE_LINE_TEST				23	// [#767] NH KSK 2014.11.18 Added Option
//#define TLS_VER11_AND_ABOVE_LINE_TEST				24	// [#767] NH KSK 2014.11.18 Added Option
//#define TLS_VER12_LINE_TEST						25	// [#767] NH KSK 2014.11.18 Added Option
//#define SSL_VER3_AND_ABOVE_USE_CERTIFY			26  // [#767] NH KSK 2014.11.18 Added Option
//#define TLS_VER10_AND_ABOVE_USE_CERTIFY			27  // [#767] NH KSK 2014.11.18 Added Option
//#define TLS_VER11_AND_ABOVE_USE_CERTIFY			28	// [#767] NH KSK 2014.11.18 Added Option
//#define TLS_VER12_USE_CERTIFY						29	// [#767] NH KSK 2014.11.18 Added Option
//#define SSL_VER3_AND_ABOVE_USE_CERTIFY_LINE_TEST	30  // [#767] NH KSK 2014.11.18 Added Option
//#define TLS_VER10_AND_ABOVE_USE_CERTIFY_LINE_TEST	31	// [#767] NH KSK 2014.11.18 Added Option
//#define TLS_VER11_AND_ABOVE_USE_CERTIFY_LINE_TEST	28	// [#767] NH KSK 2014.11.18 Added Option
//#define TLS_VER12_USE_CERTIFY_LINE_TEST			29	// [#767] NH KSK 2014.11.18 Added Option
*/

#define CONN_UPTO_SSL_V30							40
#define CONN_UPTO_TLS_V10							41
#define CONN_UPTO_TLS_V11							42
#define CONN_UPTO_TLS_V12							43

#define CONN_UPTO_SSL_V30_LINE_TEST					60
#define CONN_UPTO_TLS_V10_LINE_TEST					61
#define CONN_UPTO_TLS_V11_LINE_TEST					62
#define CONN_UPTO_TLS_V12_LINE_TEST					63

#define CONN_UPTO_SSL_V30_USE_CERT					80
#define CONN_UPTO_TLS_V10_USE_CERT					81
#define CONN_UPTO_TLS_V11_USE_CERT					82
#define CONN_UPTO_TLS_V12_USE_CERT					83
	
#define CONN_UPTO_SSL_V30_USE_CERT_LINE_TEST		100
#define CONN_UPTO_TLS_V10_USE_CERT_LINE_TEST		101
#define CONN_UPTO_TLS_V11_USE_CERT_LINE_TEST		102
#define CONN_UPTO_TLS_V12_USE_CERT_LINE_TEST		103
// End of [#775]

// [#692] AU KSK 2011.03.03
#define COUNTRY_AUS						1
#define COUNTRY_NZ						2
// end of [#692]

// [#669] MX KSK 2010.10.30
/////////////////////////////////////////////////////////////////////////////
// DATA PROCESSOR OPTION
/////////////////////////////////////////////////////////////////////////////
#define MX_CARDTRONICS						0
#define MX_PROSA							1
// end of [#669]

// [#726] MX KSK 2012.07.23
/////////////////////////////////////////////////////////////////////////////
// MX CUSTOMER TYPE OPTION
/////////////////////////////////////////////////////////////////////////////
#define MX_DOMESTIC_TYPE					1
#define MX_INTERNATIONAL_TYPE				2

/////////////////////////////////////////////////////////////////////////////
// [#725] NH PCS 2012.05.21
#define NOT_USED							0
#define PERCENT_MODE						1
#define TABLE_MODE							2
//end of [#725]
/////////////////////////////////////////////////////////////////////////////

// [#734] MX Justin 2012.12.13
/////////////////////////////////////////////////////////////////////////////
// MX Transaction Option
/////////////////////////////////////////////////////////////////////////////
//#define MX_FINALCONFIRM_ON					0
//#define MX_FINALCONFIRM_OFF					1
#define MX_EXCHANGEFEE_AMOUNT				0
#define MX_EXCHANGEFEE_PERCENT				1
// End of [#734]

/////////////////////////////////////////////////////////////////////////////
//	PERCENT SURCHARGE MANNER Set	[#112] US KGS 2008.04.15 Percent Surcharge Menu
/////////////////////////////////////////////////////////////////////////////
#define LESSER							0
#define GREATER							1

// [#742] NH Justin 2013.12.23 DCC
#define DCC_CUSTOMOPTION_DEFAULT			-1					// [#762] US Justin 2014.10.21 ADD TDL
#define DCC_CUSTOMOPTION_GENERAL			0
#define DCC_CUSTOMOPTION_CIBC				1
#define DCC_CUSTOMOPTION_CARDTRONICS		2					// [#762] US Justin 2014.10.21 ADD TDL
#define DCC_CUSTOMOPTION_CDS				3					// [#765] US Justin 2014.11.07 ADD CDS
#define	STR_DCC_CUSTOMOPTION_GENERAL		_T("GENERAL")
#define	STR_DCC_CUSTOMOPTION_CIBC			_T("CIBC")
#define	STR_DCC_CUSTOMOPTION_CDS			_T("CDS")			// [#765] US Justin 2014.11.07 ADD CDS
#define	STR_DCC_CUSTOMOPTION_CARDTRONICS	_T("CARDTRONICS")	// [#826] US Justin 2016.05.27 ADD Cardtronics STD1 Option


#define TRANHOST_ATM					0
#define TRANHOST_DUALHOST				1

#define MASTERKEY_ATM					0
#define MASTERKEY_DUALHOST				1

#define EMV_KERNEL_V4					4
#define EMV_KERNEL_V5					5
#define EMV_KERNEL_V6					6						// [#830] NH JUSTIN Added EMV Kernel V6

#define DYNAMICFLOW_1STCALL_ON			0
#define DYNAMICFLOW_1STCALL_OFF			1
// End of [#742]

// [#788] US Justin 2015.05.19
#define COMMONAID_USE_COMMON			0
#define COMMONAID_USE_ALL				1
#define COMMONAID_USE_INTERNATIONAL		2
// End of [#788]

// [798] US Justin 2015.07.02
#define SURCHARGENOTICE_AMOUNT			0
#define SURCHARGENOTICE_PERCENTAGE		1
#define SURCHARGENOTICE_BOTH			2
// End of [#798]

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
#define T_US_MAINMENU		12						// [#766] US Justin 2014.11.24
#define T_MULTIEMV_ERRROR	13						// [#785] US Justin 2015.05.01
#define T_EMVTCERROR		14						// [#810] US Justin 2015.10.12
#define T_PROGRAM			98						// PROGRAM ERROR(REQUEST BANK)
#define T_MSG				99						// CANCEL(MSG)
#define T_RETRY				100						// [#793] NH 2015.06.09 RKT Retry 추가

// [#742] US Justin 2013.12.26 DCC
#define T_DCC_USE_LOCALCURRENCY	100
#define T_DCC_USE_HOMECURRENCY	101
// End of [#742]

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
#define		RKT_SEND_STS	9	// [#793] NH KSK 2015.06.09

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
typedef struct	tagMCVW 
{
	BYTE TrInfo;
	WORD JISsize;
	BYTE JISBuff[2];         
	WORD ISO1size;               
	BYTE ISO1Buff[256];     
	WORD ISO2size;              
	BYTE ISO2Buff[256];    
	WORD ISO3size;             
	BYTE ISO3Buff[256];   
} MCAP;

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

#define DAY_TOTAL								L"BA"
#define REMOTE_DAY_TOTAL						L"FA"	// [#532] NH KSK 2009.06.09
#define CST_TOTAL								L"BB"
#define REMOTE_CST_TOTAL						L"FB"	// [#531] NH KSK 2009.06.09
#define TRIALDAY_TOTAL							L"BC"
#define REMOTE_TRIALDAY_TOTAL					L"FC"	// [#532] NH KSK 2009.06.09
#define TRIALCST_TOTAL							L"BD"
#define	CHECKCASHING_DAY_TOTAL					L"BI"	// [#369] [US] KSK 2008.7.18
#define REMOTE_CHECKCASHING_DAY_TOTAL			L"FI"	// [#532] NH KSK 2009.06.09
#define	CHECKCASHING_TRIALDAY_TOTAL				L"BK"	// [#369] [US] KSK 2008.7.18
#define	REMOTE_CHECKCASHING_TRIALDAY_TOTAL		L"FK"	// [#532] NH KSK 2009.06.09

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

//#define CHANGE_MESSAGE					L"HA"
#define CHANGE_PROCESSOR				L"HB"

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

#define CHANGE_MASTER_PWD				L"PA"
#define CHANGE_OPERATOR_PWD				L"PB"
#define CHANGE_SUPERVISOR_PWD			L"PC"

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
//#define POWER_FAIL_TRX					L"TD"	// KSK 2010.03.30 사용 안함
#define TRX_NOT_APPROVED				L"TE"
#define COMMUNICATION_ERROR				L"TF"

#define CHANGE_EXCHANGE_RATE			L"RA"	// KSK 2009.3.30

#define CHANGE_PARAMETER			L"CP"	//[#610] SOOK 2009.12.21 Configuration Change Journal 저장 
#define MONIVIEW_ACTION				L"OB"	//[#610] SOOK 2010.01.18 Configuration Change Journal 저장 
#define HOST_ACTION					L"OC"	//[#610] SOOK 2010.01.18 Configuration Change Journal 저장

#define TRX_EMV_DATA				L"OD"	// [#706] MX KSK 2011.01.14 Journal에 EMV DATA 저장 

#define CHANGE_USER_OPERATOR		1		//[#610] SOOK 2009.12.21 Configuration Change Journal 저장 
#define CHANGE_USER_SERVICE			2		//[#610] SOOK 2009.12.21 Configuration Change Journal 저장 
#define CHANGE_USER_MASTER			3		//[#610] SOOK 2009.12.21 Configuration Change Journal 저장 
#define CHANGE_USER_MONIVIEW		4		//[#610] SOOK 2009.12.21 Configuration Change Journal 저장 
#define CHANGE_USER_HOST			5		//[#610] SOOK 2009.12.21 Configuration Change Journal 저장 

// [#410] [US] KSK 2008.8.29
#define	CHECKCASHING_STANDARD1_FORMAT			1
#define	CHECKCASHING_TRANAX_FORMAT				2
// end of [#410]

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
/////////////////////////////////////////////////////////////////////////////
#endif
