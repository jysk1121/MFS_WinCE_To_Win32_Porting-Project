#ifndef _CONST_DEF_H_
#define _CONST_DEF_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------------
//
//                                 CONSTDEF.H
//
// - 본파일은 MWI ActiveX로 부터 제공된 장치제어 관련 상수 Definition Header
//   file 입니다.
// - 파일내용중 선언된 상수에 대한 변경/수정/삭제/추가 등의 내용이 필요한 경우
//   본파일을 직접 수정하는 것 이외에 MWI담당자와의 협의후 변경하여 주십시오.
// 
// - 본파일을 사용함에 있어 문제는 없으나 다음과 같은 사양을 권장합니다.
//   기종 : T1ATM With KALIGNITE
//                                                       [노틸러스효성 SE본부]
//
//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// DEVICE IDENTIFIER : ID
///////////////////////////////////////////////////////////////////////////////
#define		DEV_NONE					0x00000000			// 장치없음
#define		DEV_SPR						0x00000001			// 명세표부
#define		DEV_JPR						0x00000002			// 페이퍼저널부
#define		DEV_MCU						0x00000004			// 카드부
#define		DEV_CDU						0x00000008			// CDU			// 2004.01.15_1.2_2003
#define		DEV_PIN						0x00000010			// PIN			// 2004.01.15_1.2_2003

#define		DEV_SCR						0x00000100			// SCREEN : FLASH ACTIVEX

#define		DEV_DOR						0x00001000			// 뒷문
#define		DEV_SIU						0x00002000			// GUIDELIGHT & INDICATOR			
#define		DEV_LGT						DEV_SIU
#define		DEV_CMR						0x00004000			// USB 2 CAMERA
#define		DEV_SNS						0x00008000			// Sensor		// 2004.04.14
#define		DEV_NET						0x00010000			// [#419] [NH] KSK 2008.9.16	NETWORK ERROR
#define		DEV_SYS						0x00020000			// [#419] [NH] KSK 2008.9.16	SYSTEM ERROR
#define		DEV_RFID					0x00040000			// [#2325] NH KSK 2015.01.20

#define		DEV_BCR						0x00080000			// [#GLDV-3005] US Kook 2021.10.12 Support Side Car
#define		DEV_BNA						0x00100000			// [#GLDV-3005] US Kook 2021.10.12 Support Side Car

#define		DEV_MAIN					(DEV_MCU | DEV_SPR | DEV_JPR | DEV_CDU)
// [#68] HWANG 2008.04.02 EMV Level2
#define		MEDIA_IC					(0x00010000)
#define		MEDIA_MS					(0x00000000)
// end of [#68]

///////////////////////////////////////////////////////////////////////////////
// DEIVCE IDENTIFIER : NAME
///////////////////////////////////////////////////////////////////////////////
#define		DEVNM_NONE					("NONE")			// 장치없음
#define		DEVNM_MCU					("MCU")				// 카드부
#define		DEVNM_SPR					("SPR")				// 명세표부
#define		DEVNM_JPR					("JPR")				// 페이퍼저널부
#define		DEVNM_CDU					("CDU")				// 현금/수표출금부 2004.01.15_1.2_2003
#define		DEVNM_PIN					("PINPAD")			// PINPAD 2004.01.15_1.2_2003

#define		DEVNM_SCR					("SCR")				// SCREEN
#define		DEVNM_NET					("NET")				// NETWORK

#define		DEVNM_DOR					("RDOOR")			// 뒷문
#define		DEVNM_SIU					("SIU")				// SENSOR & INDICATOR			
#define		DEVNM_SNS					("SENSOR")			// Sensor		// 2004.04.14

#define		DEVNM_RFID					("RFID")			// [#2325] NH KSK 2015.01.20

#define		DEVNM_BCR					("BCR")				// [#GLDV-3005] US kook 2021.10.18 Support Side Car
#define		DEVNM_BNA					("BNA")				// [#GLDV-3005] US kook 2021.10.18 Support Side Car

#define		DEVNM_MAIN					("MAIN")			// 

#define		GET_DEVNAME_BY_ID(ID)			\
	((ID==DEV_MAIN)	? (DEVNM_MAIN)	:		\
	((ID==(DEV_SPR | DEV_MCU | DEV_CDU)) ? (DEVNM_MAIN) :					\
	((ID==(DEV_SPR | DEV_MCU | DEV_CDU | DEV_RFID)) ? (DEVNM_MAIN) :		\
	((ID==DEV_SPR)	? (DEVNM_SPR)	:		\
	((ID==DEV_JPR)	? (DEVNM_JPR)	:		\
	((ID==DEV_MCU)	? (DEVNM_MCU)	:		\
	((ID==DEV_CDU)	? (DEVNM_CDU)	:		\
	((ID==DEV_PIN)	? (DEVNM_PIN)	:		\
	((ID==DEV_RFID)	? (DEVNM_RFID)	:		\
	((ID==DEV_BCR)	? (DEVNM_BCR)	:		\
	((ID==DEV_BNA)	? (DEVNM_BNA)	:		\
	((ID==(DEV_BCR | DEV_BNA)) ? ("SIDECAR")	:		\
	("UNDEF") ))))))))))))

///////////////////////////////////////////////////////////////////////////////
// 함수처리결과 RETURN VALUE
// 디바이스 : -
///////////////////////////////////////////////////////////////////////////////
#define		R_NORMAL					0
#define		R_DOING						1
#define		R_ERROR						2
#define		R_TIMEOVER					3
#define		R_CANCEL					-1
#define		R_TIMEOUT					-2


///////////////////////////////////////////////////////////////////////////////
// 장치 디바이스 상태 정보
// 디바이스 : JPR / SPR / MCU
///////////////////////////////////////////////////////////////////////////////
#define		NORMAL						0					// 정상종료 
#define		DOING						1					// 현재 동작중
#define		DOWN						2					// 기기 장애
#define		NODEVICE					3					// 기기 미연결
#define		OFFLINE						4					// DEV OFFLINE

#define		GETSTR_DVCST(VAL)								\
	((VAL == NORMAL)					? ("NORMAL")	:	\
	((VAL == DOING)						? ("DOING")		:	\
	((VAL == DOWN)						? ("DOWN")		:	\
	((VAL == NODEVICE)					? ("NODEVICE")	:	\
	((VAL == OFFLINE)					? ("OFFLINE")	:	\
	("UNDEF") )))))


///////////////////////////////////////////////////////////////////////////////
// 저널 용지 잔량상태
// 디바이스 : JPR
///////////////////////////////////////////////////////////////////////////////
#define		JNL_NORMAL					0x00				// 저널 용지 정상
#define		JNL_EMPTY_PAPER				0x01				// 저널 용지 요보충		// 20030925_1.0_1078
#define		JNL_LOW_END					0x02				// 저널 용지 잔량소		// 20030925_1.0_1078


///////////////////////////////////////////////////////////////////////////////
// 매체 잔류상태
// 디바이스 : JPR / SPR / MCU
///////////////////////////////////////////////////////////////////////////////
#define		ST_NOT_DETECT				0x00				// 매체 미검지
#define		ST_SENSOR1					0x01				// 입구부(투입구) 매체검지
#define		ST_SENSOR2					0x02				// 경로상 매체검지


///////////////////////////////////////////////////////////////////////////////
// IC POWER 상태
// 디바이스 : MCU
///////////////////////////////////////////////////////////////////////////////
#define		IC_COLD_RESET				1					// COLD RESET
#define		IC_WARM_RESET				2					// WARM RESET
#define		IC_POWER_OFF				3					// POWER OFF


///////////////////////////////////////////////////////////////////////////////
// 명세표 용지 잔량상태
// 디바이스 : SPR
///////////////////////////////////////////////////////////////////////////////
#define		SLIP_NORMAL					0x00				// 명세표 용지 정상
#define		SLIP_EMPTY_PAPER			0x01				// 명세표 용지 요보충
#define		SLIP_LOW_END				0x02				// 명세표 용지 잔량소
#define		SLIP_UNKNOWN				0x03				// OFFLINE인 경우 알수 없음.


///////////////////////////////////////////////////////////////////////////////
// 장치디바이스 장찰탁 정보
// 디바이스 : CDU
///////////////////////////////////////////////////////////////////////////////
#define		NORMAL_POS					0					// 정상위치
#define		ABNORMAL_POS				1					// 탈착


///////////////////////////////////////////////////////////////////////////////
// 장치디바이스 카세트 상태정보
// 디바이스 : CDU
///////////////////////////////////////////////////////////////////////////////
#define		CST_NORMAL					0					// 정상
#define		CST_NEAR					1					// cassette near end
#define		CST_EMPTY					2					// cassette empty
#define		CST_SET_NG					3					// SET NG
#define		CST_FULL					4					// cassette full
#define		RB_ALREADY_FULL				5					// recycle box full


///////////////////////////////////////////////////////////////////////////////
// 장치디바이스 DISPENSER 장치구분
// 디바이스 : CDU
// 2004.01.20_1.2_2003 
// 20040315 JUNXI CHINA CDU 수정
///////////////////////////////////////////////////////////////////////////////
#define		DISPENSER_NOT_SET			0					// 현금출금부없음
#define		DISPENSER_1CASH				1					// 1 CST	2004.11.05 
#define		DISPENSER_2CASH				2					// 2 CST 
#define		DISPENSER_3CASH				3					// 3 CST
#define		DISPENSER_4CASH				4					// 4 CST


///////////////////////////////////////////////////////////////////////////////
// 장치디바이스 카세트 상태정보
// 디바이스 : CDU
// 2004.01.19_1.2_2003 장치정보추가
///////////////////////////////////////////////////////////////////////////////
#define		CDU_CST_REJECT				0					// Reject CST
#define		CDU_CST_1					1					// 1st    CST
#define		CDU_CST_2					2					// 2nd    CST
#define		CDU_CST_3					3					// 3rd    CST
#define		CDU_CST_4					4					// 4th    CST		// 20040315 JUNXI CHINA CDU
#define		CDU_CST_ALL					5					// ALL    CST


///////////////////////////////////////////////////////////////////////////////
// 출금가능 여부 
// 디바이스 : UCM / BRM
///////////////////////////////////////////////////////////////////////////////
#define		WITH_NOT_AVAILABLE			0					// 출금 불가능
#define		WITH_CASH_ONLY				1					// 만원만 출금 가능
#define		WITH_CHECK_ONLY				2					// 수표만 출금 가능
#define		WITH_BOTH_AVAILABLE			3					// 만원 + 수표 출금 가능

///////////////////////////////////////////////////////////////////////////////
// 출금가능 여부 
// 디바이스 : CDU 
// 중국기종 대비 현지화 2004.03.31
///////////////////////////////////////////////////////////////////////////////
#define		WITH_NOT_AVAILABLE			0					// 출금 불가능
#define		WITH_TYPE1_ONLY				1					// 100/20원만 출금 가능
#define		WITH_TYPE2_ONLY				2					// 50/10원만 출금 가능
#define		WITH_BOTH_AVAILABLE			3					// 100원 + 50원 출금 가능

//////////////////////////////////////////////////////////////////////////////
// 장치 디바이스 이전탈착 정보
// 디바이스 : BRM
///////////////////////////////////////////////////////////////////////////////
#define		NO_CHANGE					0					// 이전탈착없음
#define		CHANGE_POS					1					// 이전탈착


///////////////////////////////////////////////////////////////////////////////
// 현금 다권종 권종정보
// 디바이스 : CDU
// 중국기종 현지화	2004.03.31 
// 미국기종 MODIFY	2005.12.30
// 국가별 권종정보 DEFINE Basedef.h로 위치 이동 [#282] [NH] KSK 2008.6.11
///////////////////////////////////////////////////////////////////////////////
//#define		CASH_USD_100				100					// $100
//#define		CASH_USD_50					50					// $50
//#define		CASH_USD_20					20					// $20
//#define		CASH_USD_10					10					// $10
//#define		CASH_USD_5					5					// $5
//#define		CASH_USD_1					1					// $1

///////////////////////////////////////////////////////////////////////////////
// 현금 다권종 권종정보
// 디바이스 : CDU
// 중국기종 현지화	2004.03.31 
// 미국기종 MODIFY	2005.12.30
// 국가별 권종정보 MODIFY [#282] [NH] KSK 2008.6.11
///////////////////////////////////////////////////////////////////////////////
#define		CST_WITH_NOT_AVAILABLE		0x00				// NO CASSETTE AVAILABLE

///////////////////////////////////////////////////////////////////////////////
// BNA CST Status
///////////////////////////////////////////////////////////////////////////////
#define		BNA_CST_NORMAL				0					// Cassette is OK
#define		BNA_CST_HIGH				1 					// cassette near full
#define		BNA_CST_FULL				2					// cassette full
#define		BNA_CST_FATAL				3					// cassette error
#define		BNA_CST_MISSING				4					// cassette missing
#define		BNA_CST_UNKNOWN				5					// cassette unknown

///////////////////////////////////////////////////////////////////////////////
// 현금 권종 세팅
// 디바이스 : CDU
// 미국기종 CREATE	2005.12.30
// 국가별 권종정보 MODIFY [#282] [NH] KSK 2008.6.11
///////////////////////////////////////////////////////////////////////////////
#define		DENOMINATION_BIT8					0x80				// CASH_DENOMINATION8
#define		DENOMINATION_BIT7					0x40				// CASH_DENOMINATION7
#define		DENOMINATION_BIT6					0x20				// CASH_DENOMINATION6
#define		DENOMINATION_BIT5					0x10				// CASH_DENOMINATION5
#define		DENOMINATION_BIT4					0x08				// CASH_DENOMINATION4
#define		DENOMINATION_BIT3					0x04				// CASH_DENOMINATION3
#define		DENOMINATION_BIT2					0x02				// CASH_DENOMINATION2
#define		DENOMINATION_BIT1					0x01				// CASH_DENOMINATION1

///////////////////////////////////////////////////////////////////////////////
// 뒷문 열림상태 정보
// 디바이스 : DOR [RearDoor]
///////////////////////////////////////////////////////////////////////////////
#define		DOOR_OPENED					0					// 뒷문열림
#define		DOOR_CLOSED					1					// 뒷문닫힘

///////////////////////////////////////////////////////////////////////////////
// PINPAD 입력모드 설정
// 디바이스 : PINPAD
// 중국기종 현지화	2004.04.02 
///////////////////////////////////////////////////////////////////////////////
#define		PINMODE_NOT_USE				0					// 미사용처리
#define		PINMODE_NORMAL				1					// Normal 입력모드
#define		PINMODE_EPP					2					// Epp 입력모드

///////////////////////////////////////////////////////////////////////////////
// FLICKER INDEX 정보
// 디바이스 : LGT [LIGNT-FLICKER]
///////////////////////////////////////////////////////////////////////////////
#define		CDU_FLICKER					0x01				// CDU 지폐방출구
#define		PIN_FLICKER					0x02				// PIN 입력(NC3000)
#define		SPR_FLICKER					0x04				// 명세표 방출구
#define		MCU_FLICKER					0x08				// 카드 투입/방출구(명세표와동일)

//+++ DY Moh 2021-12-31
#define		SIDECAR_BNA_FLICKER			0x10                // Phisical Output Port for Sidecar BILL LED  +++ DY Moh 2021-12-31
#define		SIDECAR_COD_FLICKER			0x20                // Phisical Output Port for Sidecar COIN LED  +++ DY Moh 2021-12-31
#define		EPP_VB_FLICKER				0x40                // Phisical Output Port for EPP VB LED        +++ DY Moh 2021-12-31
#define		CDU_VB_FLICKER				0x80                // Phisical Output Port for CDU VB LED        +++ DY Moh 2021-12-31

#define		ALL_FLICKER					(SPR_FLICKER | MCU_FLICKER | CDU_FLICKER | PIN_FLICKER |	\
										 SIDECAR_BNA_FLICKER | SIDECAR_COD_FLICKER |				\
										 CDU_VB_FLICKER | EPP_VB_FLICKER)
										// 모든 FLICKER


///////////////////////////////////////////////////////////////////////////////
// FLICKER 조작 정보
// 디바이스 : LGT [LIGNT-FLICKER]
///////////////////////////////////////////////////////////////////////////////
#define		FLICKER_OFF					0					// FLICKER OFF
#define		FLICKER_ON					1					// FLICKER Flicking	// [#2327] NH KSK 2015.01.24 Flicking이라고 변경해야함.
#define		FLICKER_CONTINUE			2					// Flicker ON		// [#2327] NH KSK 2015.01.24

#define		GET_FLICKER_STATUS(VAL)			\
	((VAL == FLICKER_OFF)		? ("OFF")	:		\
	((VAL == FLICKER_ON)		? ("FLICKING")	:	\
	((VAL == FLICKER_CONTINUE)	? ("CONTINUE")	:	\
	("UNDEF") )))

///////////////////////////////////////////////////////////////////////////////
// INDICATOR(거래표시등) 조작 정보
// 디바이스 : LGT [LIGNT-INDICATOR]
///////////////////////////////////////////////////////////////////////////////
#define		IND_TRANENABLE				0x01				// 거래가능	켬
#define		IND_ALLOFF	 				0x00				// 취급표시등 모두다 끔

///////////////////////////////////////////////////////////////////////////////
// SPL 장치 정보 
// 디바이스 : SPL [7SEG/LED/RPL/INFORM]
// 비고     : 하단의 스펙은 T1ATM을 기준으로 작성되었음
///////////////////////////////////////////////////////////////////////////////
//       < SPL::LED >
//		┌────────┬────────────────────┐
//		│	  동작 상태   │               경고 / 이상부위          │
//		├────────┼────────────────────┤
//		│  취 급 중[0]   │    카    드[4]         명 세 표[8]     │
//		│  경    고[1]   │    통    장[5]         저    널[9]     │
//		│  이    상[2]   │    지    폐[6]         입금수표[10]    │
//		│  DUMMY   [3]   │    출금수표[7]         회 수 함[11]    │
//		└────────┴────────────────────┘
//		
//       < SPL::RPL >
//		┌────────┬────────────────────┐
//		│	  INDEX       │               CONTENTS                 │
//		├────────┼────────────────────┤
//		│       [0]      │    NORMAL  - 취급중                    │
//		│       [1]      │    CASH    - 현금                      │ 
//		│       [2]      │    LOSS    - 분실                      │
//		│       [3]      │    ALARM   - 알람                      │
//		│       [4]      │    RECEIPT - 명세표                    │
//		│       [5]      │    CALL    - 호출                      │
//		└────────┴────────────────────┘
//		  
//       < SPL::INFORM >
//		┌────────┬────────────────────┐
//		│	  INDEX       │               CONTENTS                 │
//		├────────┼────────────────────┤
//		│       [0]      │    LOSS    - 분실                      │
//		│       [1]      │    CASH    - 현금                      │ 
//		│       [2]      │    ALARM   - 알람                      │
//		│       [3]      │    RECEIPT - 명세표                    │
//		└────────┴────────────────────┘
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// MCD4 LED 조작 정보 
// 디바이스 : LED
///////////////////////////////////////////////////////////////////////////////
#define		LED_STR_OFF					("OFF")				// LED STR OFF
#define		LED_STR_ON					("ON")				// LED STR ON


///////////////////////////////////////////////////////////////////////////////
// UPS 장치 상태 정보 
// 디바이스 : UPS
///////////////////////////////////////////////////////////////////////////////
#define		POWER_NORMAL				0x00				// POWER정상
#define		EXT_AC_OFF					0x01				// 정전또는AC오프
#define		INT_BATTERY_LOW				0x02				// 밧데리LOW
#define		CLERK_POWER_OFF				0x04				// 계원전원오프
#define		POWER_ABNORMAL				0x08				// UPS상태불량
#define		POWER_DOWN_GOOD				0x10				// POWER DOWN 정상처리


///////////////////////////////////////////////////////////////////////////////
// SCREEN 장치 출력 정보 
// 디바이스 : SCR
///////////////////////////////////////////////////////////////////////////////
#define		SCR_HISTORY					(L"APHistory")			// 문자열 출력
#define		SCR_STRING					(L"APValue")			// 문자열 출력
#define		SCR_BTNONOFF				(L"APState")			// 버튼 ON-OFF값 출력

#define		SCR_ICON_STOP				(L"STOP")
#define		SCR_ICON_INFO				(L"INFO")
#define		SCR_ICON_QUESTION			(L"QUESTION")
#define		SCR_ICON_RECEIPT			(L"RECEIPT")
#define		SCR_ICON_RECEIPT_ERR		(L"RECEIPT_ERR")

///////////////////////////////////////////////////////////////////////////////
// 타임아웃 설정용 시간정보(초단위)
// 디바이스 : -
///////////////////////////////////////////////////////////////////////////////
#define		K_NO_WAIT					0
#define		K_1_WAIT					1
#define		K_5_WAIT					5
#define		K_10_WAIT					10
#define		K_15_WAIT					15
#define		K_20_WAIT					20
#define		K_25_WAIT					25
#define		K_30_WAIT					30
#define		K_35_WAIT					35
#define		K_40_WAIT					40
#define		K_45_WAIT					45
#define		K_60_WAIT					60
#define		K_65_WAIT					65
#define		K_120_WAIT					120		// [#537] AU_C KSK 2009.06.23
#define		K_180_WAIT					180
#define		K_300_WAIT					300
#define		K_600_WAIT					600
#define		K_WAIT						-1


///////////////////////////////////////////////////////////////////////////////
// CHECKDEVICEACTION FUNCTION RETURN VALUE
///////////////////////////////////////////////////////////////////////////////
#define		CHK_NORMAL					("NORMAL")			// 정상처리
#define		CHK_DOING					("DOING")			// 매체 미수취
#define		CHK_ERROR					("ERROR")			// 처리중 장애발생
#define		CHK_TIMEOVER				("TIMEOVER")		// 장치 무응답
#define		CHK_CANCEL					("CANCEL")			// 처리중 장애발생
#define		CHK_TIMEOUT					("TIMEOUT")			// 매체 미수취

#define		GET_DVC_ACT_NAME(RTNVAL)					\
	((RTNVAL==R_NORMAL)		? (CHK_NORMAL)		:		\
	((RTNVAL==R_DOING)		? (CHK_DOING)		:		\
	((RTNVAL==R_ERROR)		? (CHK_ERROR)		:		\
	((RTNVAL==R_TIMEOVER)	? (CHK_TIMEOVER)	:		\
	((RTNVAL==R_CANCEL)		? (CHK_CANCEL)		:		\
	((RTNVAL==R_TIMEOUT)	? (CHK_TIMEOUT)		:		\
	("UNDEF") ))))))

///////////////////////////////////////////////////////////////////////////////
// SCANDEVICEACTION설정 값관련
///////////////////////////////////////////////////////////////////////////////
#define		EVENT_ALL					0					// 전체이벤트 확인
#define		EVENT_IN					1					// 매체입력이벤트 확인
#define		EVENT_OUT					2					// 매체방출이벤트 확인


///////////////////////////////////////////////////////////////////////////////
// OPERATOR SWITCH설정 값관련
///////////////////////////////////////////////////////////////////////////////
#define		OPER_NONE					0					// NONE
#define		OPER_SUPERVISOR				1					// SUPERVISOR
#define		OPER_RUN					2					// RUN
#define		OPER_SUPERVISORCHANGED		11					// SUPERVISOR CHANGED
#define		OPER_RUNCHANGED				12					// RUN CHANGED
#define		OPER_MAINTENANCE			21					// RUN CHANGED

///////////////////////////////////////////////////////////////////////////////
// SIU ENHANCED AUDIO SENSOR VALUE DEFINITION					   // V05.00.05
///////////////////////////////////////////////////////////////////////////////
#define		SIU_NOTAVAILABLE			("NOTAVAILABLE")
#define		SIU_PRESENT					("PRESENT")
#define		SIU_NOTPRESENT				("NOTPRESENT")

#ifdef UNDER_CE
#  define ATM_PATH							_T("\\ATM")
#  define ATM2_PATH							_T("\\ATM2")
#  define MATM_PATH							"\\ATM"
#  define MATM2_PATH						"\\ATM2"
#else
#  define ATM_PATH							_T(".\\ATM")
#  define ATM2_PATH							_T(".\\ATM2")
#  define MATM_PATH							".\\ATM"
#  define MATM2_PATH						".\\ATM2"
#endif

#define		ATM_DATA_PATH				ATM_PATH _T("\\Data")								// ATM data folder
#define		ATM2_DATA_PATH				ATM2_PATH _T("\\Data")
#define		ADDITIONAL_UPDATE_FILE		_T("UpdateAppConfig.xml")						// [#2461] US Justin 2016.12.16 Additional Update

#define		ATM_EXTENDED_BIN_FILE		_T("\\ATM\\ExtendedBinList.dat")
#define		ATM2_EXTENDED_BIN_FILE		_T("\\ATM2\\ExtendedBinList.dat")

#define		POPMONEY_LOGO_FILE			ATM_DATA_PATH _T("\\Popmoneylogo.jpg")			// [#2350] POPMoneyLogo
#define		B4U_LOGO_FILE				ATM_DATA_PATH _T("\\B4UButton.png")				// [#RWC6-16] Bitload4U
#define		BITCOIN_LOGO_FILE			ATM_DATA_PATH _T("\\BitcoinButton.png")			// [#RWC6-68] PAI Bitcoin button
#define		CARDLESS_LOGO_FILE			ATM_DATA_PATH _T("\\CardlessButton.png")		// [#RWC6-68] PAI Bitcoin button
#define		LIBERTYX_LOGO_FILE			ATM_DATA_PATH _T("\\LibertyX.png")				// [#RWC6-59] US William 2019.10.09 LibertyX
#define		DIGITALMINT_LOGO_FILE		ATM_DATA_PATH _T("\\DigitalMint.png")			// [#RWC6-399] US ryan.payton 2022.07.27 DigitalMint
#define		CASHDEPOT_LOGO_FILE			ATM_DATA_PATH _T("\\CashDepot.png")				// [#RWC6-512] US ryan.payton 2023.04.24 Cash Depot

#define		AP_BG_WELCOME_FILE			ATM_DATA_PATH _T("\\AP_Background_Welcome_XXXX.png")	

#define		TANGOPAY_CONFIG_FILE		ATM2_DATA_PATH _T("\\TangoPayConfig.ini")			// [RWC6-676] SKKim 2024.04.01

///////////////////////////////////////////////////////////////////////////////
// EMV
///////////////////////////////////////////////////////////////////////////////
//#define		FILE_NO_LOAD				0					// NO LOAD FILE
//#define		FILE_RE_LOAD				1					// LOAD FILE

///////////////////////////////////////////////////////////////////////////////
// RECEIPT PRINT SEQUENCE	KSK 2011.05.04 Sequence가 무의미하여 해당 Flag 주석처리함
///////////////////////////////////////////////////////////////////////////////
//#define		PRINT_RECEIPT_NONE			0					// PRINT RECIPT NOT START
//#define		PRINT_RECEIPT_HEADER		1					// PRINT RECIPT HEADER
//#define		PRINT_RECEIPT_DATA			2					// PRINT RECIPT_DATA
//#define		PRINT_RECEIPT_COMPLETE		3					// PRINT RECIPT EJECT OK

// [#2226] US Justin 2013.10.23 AID Selection
#define GET_EMV_AID_POOLFILENAME(VER) ((VER==4)?(MATM_PATH "\\EMVINI\\TERM_DATA04POOL.ini"):(MATM_PATH "\\EMVINI\\TERM_DATA_POOL.ini") )		// [#2342] US Justin 2015.05.05 AID Selection....
#define GET_EMV_AID_WORKFILENAME(VER) ((VER==4)?(MATM_PATH "\\EMVINI\\TERM_DATA04.ini"):(MATM_PATH "\\EMVINI\\TERM_DATA05.ini") )				// [#2345] NH Justin Kernel5 and Kernel6 are same....
// End of [#2226]

// [#2342] US Justin 2015.05.04 AID Selection
// EMV COMMON VALUES

#define TAG_TAC_DENIAL				"D905"
#define TAG_TAC_ONLINE				"DA05"
#define TAG_TAC_DEFAULT				"D805"
#define TAG_EMV_APVERSION			"9F0902"

#define MAX_AIDLIST_SENDCNT			20
#define	AID_FIELD_DELIMITER			((char)0x1D)
#define	AID_FIELD_DELIMITER2		((char)0x3B)

#define	AID_LIST_INI_FILE			MATM_PATH L"\\EMVINI\\AIDLIST.ini"	// for ADA....

#define UPDATE_APPLICATION_NAME		"APPNAME"
#define	COMMONAID_LIST_INI_FILE		MATM_PATH "\\EMVINI\\COMMONAID.ini"

// !!!!!! IMPORTANT ====> Does not include Version Number "9F09" TAG
// EMV V4 is available only at Canada...............................
#define DEFAULT_TERMINAL_EMVDATA4	"D6099F02069F03069F3704D7089F02069F03068A029F1A0201245F2A0201249F1B0400000000DF0D0132DF0F0132DF0E04000013889F1E0830303030303030319F33036040009F40058000B050009F3501145F360100DF1001009F53015A"
#define DEFAULT_TERMINAL_EMVDATA5	"D60A9F37045A085F34019A03D7035F24039F1A0208405F2A0208409F1B0400000000DF0D0132DF0F0132DF0E04000013889F1E0830303030303030319F33036040009F40058F80B050019F3501145F360100DF1001009F530152"
// !!!!!! IMPORTANT ====> Does not include Version Number "9F09" TAG

#define GET_EMV_DEFAULT_TERMINALDATA(VER) ((VER==4)?(DEFAULT_TERMINAL_EMVDATA4):(DEFAULT_TERMINAL_EMVDATA5) )	
// End of [#2342]


// [#2389] US Justin... 
//-----------------------------------------------------------------------------
//	SCP <-> AP Delimiter
//-----------------------------------------------------------------------------
#define		SCR_CMD_DELIMITER					_T("\r")			
#define		SCR_RES_DELIMITER					((char)0x1F)		

// ----------------------------------------------------------------------------
// COMMON DEFINE
// ----------------------------------------------------------------------------
#define		FIELD_DELIMITER						((char)0x1C)
#define		GROUP_SEPERATOR						((char)0x1D)	
#define		PROXY_JNL_DELIMITER					((char)0x1D)	//	[#610] SOOK 2010.01.27 
#define		RECORD_SEPARATOR					((char)0x1E)	// [#2428] Justin 2016.05.24
#define		UNIT_DELIMITER						((char)0x1F)
#define		FIELD_SPACE							((char)0x20)

// ----------------------------------------------------------------------------
// RMS OTHER MESSAGE DEFINE
// ----------------------------------------------------------------------------
#define		RMS_OTHERMSG_DELIMITER				_T("~")

#define		RMS_OTHERMSG_FID_CURRENCYID			_T("A")
#define		RMS_OTHERMSG_FID_EXCHANGERATE		_T("B")
#define		RMS_OTHERMSG_FID_CALC_DISP_MXDOLLAR	_T("C")
#define		RMS_OTHERMSG_FID_DCC_EXCHANGERATE	_T("D")
#define		RMS_OTHERMSG_FID_DCC_CURRENCY		_T("E")
#define		RMS_OTHERMSG_FID_DCC_CONVERTEDAMT	_T("F")
#define		RMS_OTHERMSG_FID_DCC_CONVERTEDSCG	_T("G")
#define		RMS_OTHERMSG_FID_DCC_OFFERACCEPT	_T("H")
#define		RMS_OTHERMSG_FID_DCC_QUOTETYPE		_T("I")
#define		RMS_OTHERMSG_FID_DONATE_TO			_T("J")
#define		RMS_OTHERMSG_FID_AU_DOMESTICBIN_FALLBACK	_T("K")	// [#2379] AU KSK 2015.11.17 Domestic Bin으로 인한 FallBack 정보 추가
#define		RMS_OTHERMSG_FID_DISPENSED_COUNT	_T("L")
#define		RMS_OTHERMSG_FID_REJECTED_COUNT		_T("M")
#define		RMS_OTHERMSG_FID_BANKNAME			_T("N")
#define		RMS_OTHERMSG_FID_LOYALTY_FEE		_T("O")
#define		RMS_OTHERMSG_FID_LINEUSAGE_FEE		_T("P")
#define		RMS_OTHERMSG_FID_IVA_TAX			_T("Q")
#define		RMS_OTHERMSG_FID_RETURNED_ACCNT_NO	_T("R")	
#define		RMS_OTHERMSG_FID_EXCHANGE_FEE		_T("S")
#define		RMS_OTHERMSG_FID_TOTAL_FEE			_T("T")	
#define		RMS_OTHERMSG_FID_CUSTOMERTYPE		_T("U")
#define		RMS_OTHERMSG_FID_LTRY_AUTHRIZE_NUM	_T("V")
#define		RMS_OTHERMSG_FID_LTRY_CONFIRM_NUM	_T("W")
#define		RMS_OTHERMSG_FID_LTRY_AMOUNT		_T("X")
#define		RMS_OTHERMSG_FID_LOCALLOAN_NUMBER	_T("Z")	
#define		RMS_OTHERMSG_FID_PIN4_PHONENUMBER	_T("a")		// [#2405] US Justin 2016.03.21 HalCash Online
#define		RMS_OTHERMSG_FID_PIN4_PIN4CODE		_T("b")		// [#2405] US Justin 2016.03.21 HalCash Online
#define		RMS_OTHERMSG_FID_PIN4_SECRETCODE	_T("c")		// [#2405] US Justin 2016.03.21 HalCash Online
#define		RMS_OTHERMSG_FID_GENERAL_COMMENT	_T("d")		// [#2496] US Justin 2017.08.16
#define		RMS_OTHERMSG_FID_DCC_EXCHANGERATE_MARKUP	_T("m")	// [#2582] NH Kook 2019.09.23 VISA DCC

// ----------------------------------------------------------------------------
// OS VERSION에 따른 PATH DEFINE	[#156] KSK 2008.04.22 OS Version에 따른 PATH 지정
// ----------------------------------------------------------------------------
#define MEMORYSTICKPATH_BY_USING_OS_VERSION \
	((RegGetStr(L"SOFTWARE\\ATM", L"OSVersion")).CompareNoCase(L"V01.01.06") >= 0 ? L"\\USB" : ATM2_PATH)
#define LOGANDJNLPATH_BY_USING_OS_VERSION \
	((RegGetStr(L"SOFTWARE\\ATM", L"OSVersion")).CompareNoCase(L"V01.01.06") >= 0 ? ATM2_PATH : ATM_PATH)

// ----------------------------------------------------------------------------
// ADVERTISEMENT PATH
// ----------------------------------------------------------------------------
#define	ADVERTISEMENT_PATH				ATM2_PATH _T("\\Advertisement")
#define LTX_ADS_PATH					ATM_DATA_PATH _T("\\LibertyX")  // [#RWC6-23] US William 2019.09.09 GivePay Extra Ads	// [#RWC6-280] LibertyX Default Ads

// ----------------------------------------------------------------------------
// STORAGE CARD PATH
// ----------------------------------------------------------------------------
#define USB_PATH						_T("\\USB")
#define SDCARD_PATH						_T("\\STORAGE CARD")

// ----------------------------------------------------------------------------
// BACKUP PATHS
// ----------------------------------------------------------------------------
#define LOGBACKUP_DEST_PATH USB_PATH
#define JNLBACKUP_SRC_PATH	ATM2_PATH


// ----------------------------------------------------------------------------
// PATCH PATH
// ----------------------------------------------------------------------------
#define UPDATE_BASEDIR_60				_T("\\UPDATE\\NH2700CE")
#define UPDATE_BASEDIR_70				_T("\\UPDATE7")
#define MASTER_60						_T("MASTER")
#define MASTER_70						_T("MASTER7")
#define PATCH_60						_T("PATCH")
#define PATCH_70						_T("PATCH7")

// ----------------------------------------------------------------------------
// Screen PATH
// ----------------------------------------------------------------------------

#define	SCREEN_PATH							ATM_PATH _T("\\Screen")
#define	SCREEN_PATH_BACKS					ATM2_PATH _T("\\Screen\\Backs")
#define	SCREEN_PATH_BACKS_2800				ATM_PATH _T("\\Screen\\Backs_2800")	// [#2542] US Kook 2018.03.15 New backgrounds for MX2800
// [#2545] NH Kook 2018.04.11 Support customized images.
#define SCREEN_PATH_BACKS_CUSTOM			ATM_PATH _T("\\Screen\\Backs")

#define SCREEN_PATH_AD						ATM2_PATH _T("\\Advertisement")
#define SCREEN_PATH_AD_CUSTOM				ATM_PATH _T("\\Advertisement")

#define FILE_RCPT_HEADER					ATM2_PATH _T("\\ReceiptHeader.bmp")
#define FILE_RCPT_HEADER_CUSTOM				ATM_PATH _T("\\ReceiptHeader.bmp")
#define FILE_RCPT_HEADER_B4U				ATM2_PATH _T("\\ReceiptHeader_B4U.bmp")

#define FILE_DEFAULT_BACKS					ATM_PATH _T("\\Default_Backs.zip")
#define FILE_DEFAULT_BACKS_2800				ATM_PATH _T("\\Default_Backs_2800.zip")
#define FILE_DEFAULT_AD						ATM_PATH _T("\\Default_Advertisement.zip")
#define FILE_DEFAULT_RCPT_HEADER			ATM_PATH _T("\\Default_ReceiptHeader.zip")
// end of [#2545]

// [#2558] NH Justin 2018.06.12 Fix fof S/W Update Failure after "Format and Update"
#define	TTS_DB_PATH							ATM_PATH _T("\\TTS_DB")
#define FILE_TTS_DB							ATM_PATH _T("\\TTS_DB.zip")
// End of [#2558]

#define SCREENSHOTS_PATH					SDCARD_PATH _T("\\Screenshots")

// ----------------------------------------------------------------------------
// VDM PATH
// ----------------------------------------------------------------------------
#define	VDM_CDU_FULL_PATH				_T("\\ATM\\HSCdmDrv.dll")			// [#608] NH KSK 2010.01.12
#define	VDM_SPR_FULL_PATH				_T("\\ATM\\VDM_KSPR.dll")			// [#608] NH KSK 2010.01.12
#define	VDM_MCU_NHDIP_FULL_PATH			_T("\\ATM\\DIP4VDM.dll")			// [#608] NH KSK 2010.01.12
#define	VDM_MCU_MAGTEK_FULL_PATH		_T("\\ATM\\MagtekCommDLL4WCE.dll")	// [#608] NH KSK 2010.01.12
#define	VDM_MCU_SANKYO_FULL_PATH		_T("\\ATM\\NHVDM4SANDIP.dll")		// [#2073] NH KSK 2011.06.24
#define	VDM_KEYMGR_FULL_PATH			_T("\\ATM\\KeyMgr.dll")				// [#2259] NH KSK 2014.03.13
#define	VDM_RFID_VIVOPAYKII_FULL_PATH	_T("\\ATM\\NHIDC4RF_DLL.dll")		// [#2325] NH KSK 2015.01.25
#define VDM_BCR_FULL_PATH				_T("\\ATM\\NXBarcodeVDM.dll")		// [#GLDV-3005] US Kook 2021.10.12 Support Side Car
#define VDM_BNA_MEI_FULL_PATH			_T("\\ATM\\VDMMEI.dll")				// [#GLDV-3005] US Kook 2021.10.12 Support Side Car
#define VDM_BNA_JCM_FULL_PATH			_T("\\ATM\\VDM_BAM.dll")			// [#GLDV-3005] US Kook 2022.03.14 Support Side Car

// ----------------------------------------------------------------------------
// VDM TEST DISPENSE COUNT
// ----------------------------------------------------------------------------
#define		VDM_TEST_DISPENSE_COUNT			1	// [#2357] NH KSK 2015.07.11

// ----------------------------------------------------------------------------
// LOG FILE PATH
// ----------------------------------------------------------------------------
#define		CDU_SP_SENSOR_LOG_FILE		ATM2_PATH _T("\\LOG\\CDUM_SensorLog.log")		// [#2238] AU KMK 2013.12.18 CDU Sensor Log 파일 경로
// End of [#2389]

//-----------------------------------------------------------------------------
// SSL CERT Path. [#2093] NH PCS 2011.10.24
//-----------------------------------------------------------------------------
#define	SSL_CERT_SRCFILE				"\\USB\\rootcert.pem"
#define SSL_CERT_DESFILE				MATM_PATH "\\rootcert.pem"

//-----------------------------------------------------------------------------
// SSL Root CA File [#RWC6-26] US William 2019.09.05 Add CA root certs
//-----------------------------------------------------------------------------
#define CA_ROOT_CERTS_FILE_HYOSUNG		MATM_PATH "\\ca.pem"
#define CA_ROOT_CERTS_FILE_CUSTOMER		MATM_PATH "\\rootcert.pem"
#define CA_ROOT_CERTS_FILE_UPDATER		"\\UPDATER\\ca.pem"
#define CA_ROOT_CERTS_FILE				"\\ca.pem"

//-----------------------------------------------------------------------------
// SINFO.ini Path. [#2136] US PCS 2012.05.30
//-----------------------------------------------------------------------------
#define SINFO_DESFILE					MATM_PATH "\\SINFO.ini"
#define SINFO_SRCFILE					"\\USB\\SINFO.ini"

// ----------------------------------------------------------------------------
// DEFINE PIN VERSION
// ----------------------------------------------------------------------------
// SP Version V05.00.19에서는 VISA EP : V03.02.01 / PCI EP :V08.xx.xx로 되어 있음
// SP Version V05.02.01에서는 512K EP VERSION :V 08.20.xx로 되어 있으므로 호주(512K사용) 외에는Version뒤에 SPACE를 제거해야함
#define PIN_PCI50_EP_VERSION				L"V15.00.00"	// [#GLDV-2683] NH Kook 2019.11.25 Support PCI 5.0 EPP
#define PIN_PCI30_EP_VERSION				L"V10.00.00"	// [#2259] NZ KSK 2014.3.13 PCI3.0부터 RKT시 Random Number 필수 적용에 따른 로직 수정
#define PIN_PCI20_EP_VERSION				L"V09.00.00"	// [#2017] NZ KSK 2011.02.10
#define	PIN_512K_EP_VERSION					L"V08.20.00"
#define PIN_PCI_EP_VERSION					L"V08.00.00"
#define PIN_VISA_EP_VERSION					L"V03.02.01"

#define LOAD_DESKEY_USE						L"FUNCTION"
#define LOAD_MACKEY_USE						L"MACING"

// Working Keys
// - TMK
#define SINGLEDESKEY_WORKINGKEY_NAME		L"K1"
#define TRIPLEDESKEY_WORKINGKEY_NAME		L"K3"
#define LIBERTYX_WORKINGKEY_NAME			L"LibertyX_WK"
// - MAC
#define SINGLEMACKEY_WORKINGKEY_NAME		L"K4"
#define TRIPLEMACKEY_WORKINGKEY_NAME		L"K5"
// - PIN
#define TRIPLEDESKEY_2ND_WORKINGKEY_NAME	L"2ND_K3"		// [#2181] US KSK 2013.03.31
// - Data
#define TRIPLEDESKEY_3RD_WORKINGKEY_NAME	L"3RD_K3"		// [#2322] US Justin 2015.01.13

// - TR31 Working keys
#define TR31_ATM_WORKINGKEY_NAME			L"ATM_TR31WK"
#define TR31_DUALHOST_WORKINGKEY_NAME		L"DualHost_TR31WK"
#define TR31_LIBERTYX_WORKINGKEY_NAME		L"LibertyX_TR31WK"

// Master Keys
// - TMK
#define SINGLEDESKEY_NAME					L"MASTERK"
#define TRIPLEDESKEY_NAME					L"MASTERK3"
#define LIBERTYXKEY_NAME					L"LibertyX"	 // Additional TMK
// - MAC
#define SINGLEMACKEY_NAME					L"MACK"
#define TRIPLEMACKEY_NAME					L"MACK3"
#define TR31_ATM_TMACWORKINGKEY_NAME		L"ATM_TR31MK"
#define TR31_DUALHOST_TMACWORKINGKEYNAME	L"DualHost_TR31MK"
#define TR31_LTX_TMACWORKINGKEY_NAME		L"LibertyX_TR31MK"
// - PIN
#define SINGLEPINKEY_NAME					L"PINK"
#define TRIPLEPINKEY_NAME					L"PINK3"
// - Data
#define SINGLEDATAKEY_NAME					L"DATAK"
#define TRIPLEDATAKEY_NAME					L"DATAK3"

// - Mis-named PIN key
#define TRIPLEDESKEY_2ND_PIN_NAME			L"2ND_MASTERK3"	// [#2181] US KSK 2013.03.31

// Button Names
// - Triples
#define T_TMK_BUTTON		TRIPLEDESKEY_NAME
#define T_PINMK_BUTTON		TRIPLEDESKEY_2ND_PIN_NAME
#define T_MACMK_BUTTON		TRIPLEMACKEY_NAME
#define T_DATAMK_BUTTON		TRIPLEDATAKEY_NAME
// - Singles
#define S_TMK_BUTTON		SINGLEDESKEY_NAME
#define S_PINMK_BUTTON		SINGLEPINKEY_NAME
#define S_MACMK_BUTTON		SINGLEMACKEY_NAME
#define S_DATAMK_BUTTON		SINGLEDATAKEY_NAME

// [#RWC6-59] US William 2019.10.08 LibertyX
// Change the data key name to LibertyX
#if(APP_LIBERTYX)
	#define T_TMK_LTX_BUTTON	TRIPLEDESKEY_NAME L";" LIBERTYXKEY_NAME
#endif
// End of [#RWC6-59]

// ----------------------------------------------------------------------------
// DEFINE 512K PIN - RKT
// ----------------------------------------------------------------------------
#define PUBLICKEY_2048_BIT_MODULUS			L"3082010A0282010100"
#define	PUBLICKEY_2048_BIT_EXPONENT			L"0203010001"
#define RKT_TERMINAL_MASTER_KEY				L"0"
#define RKT_MAC_MASTER_KEY					L"1"
#define RKT_SINGLE_TMK						1
#define RKT_TRIPLE_TMK						3
#define RKT_SINGLE_MMK						7
#define RKT_TRIPLE_MMK						11

// ----------------------------------------------------------------------------
// CDU EP VERSION
// ----------------------------------------------------------------------------
#define CDU_L3_EP_VERSION					L"C01"
#define L_CDU_EP_VERSION					L"C02"
#define CDU_M_1LEVER_EP_VERSION				L"C03"
#define CDU_M_2LEVER_EP_VERSION				L"C04"
#define GBM10_EP_VERSION					L"C91"	// [#2025] NH KJW 2011.02.28
#define W_CDU_EP_VERSION					L"C92"	// [#2097] US PCS 2011.11.21 "WallMount Typed CDU"

// ----------------------------------------------------------------------------
// SPR EP VERSION
// ----------------------------------------------------------------------------
#define SPR_HWASUNG_EP_VERSION				L"HMK"	// [#2100] NH KSK 2011.12.02

// ----------------------------------------------------------------------------
// JNL Image File Path
// ----------------------------------------------------------------------------
#define JNL_IMAGE_FILE_PATH					_T("\\STORAGE CARD\\JNLImage")	
#define JNL_IMAGE_DELIMITER					((char)0x1E)

// ----------------------------------------------------------------------------
// Weather File Path & Name
// ----------------------------------------------------------------------------
#define	WEATHER_FILENAME					ATM_PATH L"\\WeatherInfo.dat"

// ----------------------------------------------------------------------------
// Resetting Master Password Special Key
// ----------------------------------------------------------------------------
#define RESET_MP_SPECIAL_KEY				L"CLEAR,.,00,CLEAR,CLEAR,CANCEL"

// ----------------------------------------------------------------------------
// Default Hot key to enter Operator mode
// ----------------------------------------------------------------------------
#define DEFAULT_OPMODE_HOTKEY				L"ENTER,CLEAR,CANCEL,1,2,3"

// ----------------------------------------------------------------------------
// Asian Font								[#2186] US KSK 2013.04.30 Font File Copy를 위한 Path Define
// ----------------------------------------------------------------------------
#define	ASIAN_FONT_SOURCE_PATH				ATM_PATH _T("\\Fonts")
#define	ASIAN_FONT_DEST_PATH				ATM2_PATH _T("\\Fonts")

// [#2458] AU KSK 2016.12.09
// ----------------------------------------------------------------------------
// MCU DEVICE TYPE
// ----------------------------------------------------------------------------
#define		MCU_NH_DIP				1					// Nautilus Hyosung Dip Card Reader
#define		MCU_MAGTEK_DIP			3					// Magtek Dip Card Reader
#define		MCU_SANKYO_DIP			4					// Sankyo Dip Card Reader
// end of [#2458]

// [#GLDV-3005] US Kook 2022.04.07 support side car
// ----------------------------------------------------------------------------
// BNA DEVICE TYPE
// ----------------------------------------------------------------------------
#define		BNATYPE_MEI					1					// MEI Bill Acceptor
#define		BNATYPE_JCM					3					// JCM Bill Acceptor
// end of [#GLDV-3005]

// [#2459] AU KSK 2016.12.14
// ----------------------------------------------------------------------------
// Dispense Mix Option AU Only
// ----------------------------------------------------------------------------
#define		CDM_MIX_MINIMUM_NUMBER_OF_BILLS		1		// SP Mix 1번 사용
#define		CDM_MIX_MAXIMUM_NUMBER_OF_BILLS		10		// SP Mix 10번 사용	
// end of [#2459]

// [#2472] NH Justin 2017.02.07 DCC Withdrawal Option
#define		DCC_WITHDRAWAL_SETTING_NONE			0
#define		DCC_WITHDRAWAL_SETTING_EMV_MASTER	1

#define		DCC_PRECHECK_OFF					0
#define		DCC_PRECHECK_DCCNA					1
#define		DCC_PRECHECK_DCCAVAIL				2
// End of [#2472]

// [#2492] NH Justin 2017.06.30 Fixing "OOS W/O Error Code" - Add Error Code Removing Options.
#define		CLEAR_ERROR_ALL						0
#define		CLEAR_ERROR_NETWORK					1
// End of [#2492]

// [#2549] NH Justin 2018.05.08 Add Perto Rico Option
#define		US_TERRITORY_US						0
#define		US_TERRITORY_PUERTORICO				1
#define		US_TERRITORY_GUAM					2
#define		US_TERRITORY_MARIANA				3
#define		US_TERRITORY_VIRGINISLAND			4
#define		US_TERRITORY_SAMOA					5
// End of [#2549]

// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Optioin
#define		REBOOT_OPTION_DISABLE				0
#define		REBOOT_OPTION_ONTIME				1
#define		REBOOT_OPTION_INTERVAL				2

#define		REBOOT_OPTION_MAX_INTERVAL			720		// 30 days (30*24)
// End of [#2558]

#define		SPR_EJECT_STRING					"%EJECT%"							// [#2562] NH Justin 2018.07.12 Reduce PAUSE INTERVAL Between PRINT and CUT



#define		INIT_BY_OPENSESSION					0
#define		INIT_BY_RESET						1


#endif // _CONST_DEF_H_
