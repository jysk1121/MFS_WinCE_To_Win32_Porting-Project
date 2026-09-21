#ifndef __NH_SCREEN_ID_DEF_H__
#define __NH_SCREEN_ID_DEF_H__

#include "ScreenDef_utf16.h"

/* AP Screen
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////  ADD DESCRIPTION WHEN A SCREEN IS ADDED
////  ADD DESCRIPTION WHEN A SCREEN IS ADDED
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

001	Initializing
002	Out of Service
003 In Operator Mode
004	ADA Processing
005 RMS Processing
006	Stop Attacking
007 Out of service with QR
008	Just Background (?)

009 ~ 099		: NOT USED

100 AddOnAP + ATM WELCOME SCREEN
101	Welcome
102 Select Language
103	Enter PIN
104
105	Select Transaction
106	Select Receipt
107	YES/NO => Continue reversal?, Enhanced Coupon?, Another Transaction?, Error Receipt(AU) , Prebalance?, Need more time?, Select Receipt?(Dual Balance)
108  
109	Surcharge
110 AU Surcharge
111	Select Account(CW, BI)
112 Select Amount
113	Enter Amount (CW, TR)
114	Select From Account(TR)
115	Select To Account(TR)
116 Host Connect
117	Dispense Cash
118	Take Cash
119 Print Receipt
120 Take Receipt
121	Display transaction result on Screen
122	ERRO (Warning)
123	Thank You
124	
125 EMV IC Processing
126 EMV Select AID
127	EMV Remove Card
128	EMV Continue Fallback?
129	
130	MX Bank Fee Notice, MX 1024*600
131	
132 MX Exchange Fee Notice, MX 1024*600
133	MX Surcharge Fee Notice, MX 1024*600
134	MX Final Confirmation, MX 1024*600
135 DCC Offer
136	MX No Surcharge Fee Notice, MX 1024*600
137 Displaying Digital Receipt
138	Select Receipt (Paper, Digital, No)
139	MX Exchange Rate for Domestic Card, MX 1024*600
143 Cardtronics  Select Account for Balance including Dual Balance, US Only
144	Select Account(BI) with zero Fee Notice (BI) US Only
146 Cardtronics  Alternative Pre-Dual Balance, US Only
147 POPMoney Enter Access Code
153 Cardless Transaction Selection (POPMONEY / PIN4)
154 PIN4 US Mobile Phone Number
155 PIN4 International Country Code / Phone Number
156 PIN4 4 digit code / seceret Code
157 WalPay 6 digit ID
158 Local Loan, Select DayClose or Transaction
159 Cardtronics ALLPOINT Surcharge Notice (similar to 109)
160 Just.Cash Enter Cash Code
161 Paypal/Just.Cash PleaseWait (Get Token / Get withdrawal ticket / Update Surcharge and Transaction)
162 Paypal Display QR
163 General Selection
164 Loyalty Point Service Notice
165 GivePay Selection
166 GivePay Amount Input
167 GivePay Phone Number Input
168 GivePay Summary
169 GivePay ZipCode
170 Connect to Host (ALPHI)
171 B4U QR
172 B4U Pequest
173 B4U Receipt

174 ~ 179		: NOT USED

180 Denomination Selection

181 ~ 199		: NOT USED
200 RESET MASTER PASSWPRD

203 ~ 343		: NOT USED

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////  ADD DESCRIPTION WHEN A SCREEN IS ADDED
////  ADD DESCRIPTION WHEN A SCREEN IS ADDED
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
*/

/*------------------------------------------------------------------
	SCREEN HELPER MACRO
------------------------------------------------------------------*/
#define GET_ENDIS_STRING(EN) ((EN)?(_T("ENABLE")):(_T("DISABLE")))			// 1-ENABLE, 0-DISABLE

//[#2160] CA KMK 2012.12.05 FrenchOP, 미사용
//#define GET_ACTPAS_STRING(EN) ((EN)?(_T("ACTIVE")):(_T("PASSIVE")))				// 1-ACTIVE, 0-PASSIVE

#define GET_DIALTCP_STRING(EN) (((EN)==1)?(_T("DIALUP")):(_T("TCP/IP")))		// 1-DIALUP, 2-TCP/IP
#define GET_SURMANNER_STRING(EN) ((EN)?(_T("GREATER")):(_T("LESSER")))			// 1-GREATER, 0-LESSER	[#112] US KGS 2008.04.15 Surcharge Percent Menu
#define GET_CHKTYPE_STRING(EN) (((EN)==1)?(_T("TYPE_A")):(_T("TYPE_B")))		// 1-TYPE A, 2-TYPE B	[#410] [US] KSK 2008.9.8
#define GET_TOTALTYPE_STRING(EN) (((EN)==AUTO_DAY_TOTAL)?(_T("DAY TOTAL")):(_T("TRIAL DAY TOTAL")))			// AUTO_DAY_TOTAL("DAY TOTAL"), AUTO_TRIAL_DAY_TOTAL("TRIAL DAY TOTAL")
#define	GET_SURCHARGELOCATION_STRING(EN) (((EN)==0)?(_T("BEGINNING")):(_T("AFTER AMOUNT")))	// 0-BEGINNING, 1-AFTER AMOUNT	[#443] [US] KSK 2008.11.04
#define GET_EPP_OPTION_STRING(EN)			(((EN==0))?(_T("ALWAYS")):(_T("ONLY TRANSACTION")))			// 0-ALWAYS, 1-ONLY TRANSACTION		[#604] NH KSK 2010.01.05
#define GET_CST_SOUND_OPTION_STRING(EN)		(((EN==0))?(_T("ON")):(_T("OFF")))							// 0-CST SOUND ON, 1-CST SOUND OFF	[#604] NH KSK 2010.01.05
#define GET_REVERSE_ENDIS_STRING(EN)			(((EN==0))?(_T("ENABLE")):(_T("DISABLE")))				// 0-ENABLE, 1-DISABLE	KSK 2010.02.02
#define GET_REVERSE_ENDIS_STRING_RCPT(EN)		(((EN==0))?(T_ENABLE_RCPT):(T_DISABLE_RCPT))	// 0-ENABLE, 1-DISABLE	[#2160] CA KMK 2012.11.26 FrenchOP

//[#2160] CA KMK 2012.12.05 FrenchOP, 미사용
// #define GET_SUROPT_STRING(EN)					((EN)?(_T("USE_LOCAL_AMOUNT")):(_T("USE_HOST_AMOUNT")))  //[#2092] US PCS 2011.10.25

#define GET_ENHANCED_COUPON_TYPE_STRING(EN)		(((EN==1))?(_T("ENHANCED COUPON")):(_T("BASIC COUPON")))	// [#633] NH KSK 2010.03.31

// [#620] US KSK 2010.02.22
#define GET_ENHANCED_COUPON_OPTION_STRING(OPTION)	\
	((OPTION==0) ? (_T("NOT USE"))	:			\
	((OPTION==1) ? (_T("ALWAYS"))	:			\
	((OPTION==2) ? (_T("USE TIME"))	:	(_T("NOT DEFINED"))	)))
#define GET_ENHANCED_COUPON_CUT_OPTION_STRING(OPTION)	(((OPTION==1))?(_T("AFTER RECEIPT")):(_T("NOT USE")))
// end of [#620]

// [#2150] US Justin 2012.10.03
#define GET_STD1_DCC_OPTION_STRING(EN)				(((EN==1))?(_T("HOST DETERMINE")):(_T("DISABLE")))
#define GET_STD1_PINCHANGE_OPTION_STRING(OPTION)	\
	((OPTION==0) ? (_T("DISABLE"))	:			\
	((OPTION==1) ? (_T("ENABLE"))	:			\
	((OPTION==2) ? (_T("HOST DETERMINE"))	:	(_T("DISABLE"))	)))
// End of [#2150]

// [#302] [MX] KSK 2008.6.17
#define GET_LANGUAGE_STRING(SELECT_LANGUAGE)	\
			((SELECT_LANGUAGE==6) ? (_T("JAPANESE")) :	\
				((SELECT_LANGUAGE==5) ? (_T("KOREAN")) :	\
					((SELECT_LANGUAGE==4) ? (_T("CHINESE")) : \
						((SELECT_LANGUAGE==3) ? (_T("FRENCH")) : \
							((SELECT_LANGUAGE==2) ? (_T("SPANISH")) : (_T("ENGLISH")))))))
// end of [#302]

// [#2326] US Kook 2015.08.25 compacting receipt
#define GET_LANGUAGE_STRING_SHORT(SELECT_LANGUAGE)	\
	((SELECT_LANGUAGE==6) ? (_T("JPN")) :	\
	((SELECT_LANGUAGE==5) ? (_T("KOR")) :	\
	((SELECT_LANGUAGE==4) ? (_T("CHN")) : \
	((SELECT_LANGUAGE==3) ? (_T("FRN")) : \
	((SELECT_LANGUAGE==2) ? (_T("SPN")) : (_T("ENG")))))))
// end of [#2326]

#define GET_EMV_TRAN_STRING(EN) (((EN) ==0)?(_T("MS FIRST")):(_T("IC FIRST")))

// [#546] SOOK 2009.07.28 다기능 BIN LIST 기능 추가함
#define GET_BIN_ACTION_STRING(ACTION)	\
			((ACTION == 1) ? (_T("WITHDRAWAL SURCHARGE")) :	\
				((ACTION == 2) ? (_T("BALANCE SURCHARGE")) :	\
					((ACTION == 3) ? (_T("ACCEPT CARD")):	\
						((ACTION == 4) ? (_T("DECLINE CARD")) :	\
							((ACTION == 5) ? (_T("AWARD COUPON")) :	\
								((ACTION == 6) ? (_T("PIN CHANGE")): (_T("BLOCK SURCHARGE")) ))))))

//[#610] SOOK 2010.01.13
	// [1-VISA_FRAMED], [2-STANDARD], [3-ACK_CONTROLED]
#define GET_TCPIP_TYPE_STRING(TYPE)	\
	((TYPE == 1) ? (_T("VISA_FRAMED")) : \
				((TYPE == 2) ? (_T("STANDARD")) : (_T("ACK_CONTROLED")) ))
#define GET_PREDIAL_OPTION_STRING(TYPE)	\
	((TYPE == 0) ? (_T("AFTER CARD")) : \
				((TYPE == 1) ? (_T("AFTER PIN")) : (_T("AFTER ACCOUNT")) ))
#define GET_SCH_JOURNAL_OPTION_STRING(OPTION)\
	((OPTION == 0) ? (_T("COUNT")) : \
				((OPTION == 1) ? (_T("MONTHLY")) : (_T("DAILY")) ))
//[#610]
// [#236] NZ AIREAT 2008.05.27

// [#2549] NH Justin 2018.05.08 Add Perto Rico Option
#define GET_US_TERRITORY_OPTION_STRING(OPTION)	\
	((OPTION == US_TERRITORY_US) ? (_T("US")) : \
	((OPTION == US_TERRITORY_PUERTORICO) ? (_T("PUERTO RICO")) : \
	((OPTION == US_TERRITORY_GUAM) ? (_T("GUAM")) : \
	((OPTION == US_TERRITORY_MARIANA) ? (_T("N. MARIANA I.")) : \
	((OPTION == US_TERRITORY_VIRGINISLAND) ? (_T("US VIRGIN I.")) : \
	((OPTION == US_TERRITORY_SAMOA) ? (_T("A. SAMOA")) : ( _T("") ) ))))))
// End of [#2549]

// [#2160] CA KMK 2012.12.07 French OP 매크로 함수 분할
// 문자열을 반환하는 매크로 함수(상단의 _STRING 매크로), 저널 저장 시 파라매터 값으로 사용
// ID값을 반환하는 매크로 함수(하단의 _APTEXTID 매크로), 명세표 출력 시 사용
// 추후 통합...
#define GET_ENDIS_APTEXTID(EN) ((EN)?(T_ENABLE):(T_DISABLE))						
#define GET_ENDIS_APTEXTID2(EN) (((EN)==ENABLE)?(T_ENABLE):(T_DISABLE))
#define GET_ENDIS_APTEXTID_RCPT(EN) ((EN)?(T_ENABLE_RCPT):(T_DISABLE_RCPT))		
#define GET_DIALTCP_APTEXTID(EN) (((EN)==1)?(T_DIALUP):(T_TCP_IP))
#define GET_DIALTCP_APTEXTID_PAI(EN) (((EN)==1)?(T_DIALUP):(T_TCP_IP_PAI))
#define GET_SURMANNER_APTEXTID(EN) ((EN)?(T_GREATER):(T_LESSER))			
#define GET_CHKTYPE_APTEXTID(EN) (((EN)==1)?(T_TYPE_A):(T_TYPE_B))		
#define GET_TOTALTYPE_APTEXTID(EN) (((EN)==AUTO_DAY_TOTAL)?(T_DAY_TOTAL):(T_TRIAL_DAY_TOTAL))			
#define	GET_SURCHARGELOCATION_APTEXTID(EN) (((EN)==0)?(T_BEGINNING):(T_AFTER_AMOUNT))	
#define GET_EPP_OPTION_APTEXTID(EN)		(((EN==0))?(T_ALWAYS):(T_ONLY_TRANSACTION))	
// [#2333] NH KSK 2015.03.06
#define GET_EPP_LED_GUIDE_OPTION_APTEXTID(OPTION) \
	((OPTION==0) ? (T_ALWAYS)			:		\
	((OPTION==1) ? (T_ONLY_IDLE)		:		\
	((OPTION==2) ? (T_ONLY_TRANSACTION) : (T_NOT_DEFINED) )))
// end of [#2333]
#define GET_CST_SOUND_OPTION_APTEXTID(EN)		(((EN==0))?(T_ON):(T_OFF))							
#define GET_REVERSE_ENDIS_APTEXTID(EN)			(((EN==0))?(T_ENABLE):(T_DISABLE))				
#define GET_REVERSE_ENDIS_APTEXTID_RCPT(EN)		(((EN==0))?(T_ENABLE_RCPT):(T_DISABLE_RCPT))	
#define GET_ENHANCED_COUPON_TYPE_APTEXTID(EN)		(((EN==1))?(T_ENHANCED_COUPON):(T_BASIC_COUPON))	
#define GET_ENHANCED_COUPON_OPTION_APTEXTID(OPTION)	\
	((OPTION==0) ? (T_NOT_USE)	:			\
	((OPTION==1) ? (T_ALWAYS)	:			\
	((OPTION==2) ? (T_USE_TIME)	:	(T_NOT_DEFINED)	)))
#define GET_ENHANCED_COUPON_CUT_OPTION_APTEXTID(OPTION)	(((OPTION==1))?(T_AFTER_RECEIPT):(T_NOT_USE))
// [#2499] US Justin 2017.08.21 Dual Host DCC
#define GET_STD1_DYNAMICFLOW_OPTION_APTEXTID(OPTION)	\
	((OPTION==0) ? (T_DISABLE)	:			\
	((OPTION==1) ? (T_ENABLE)	:			\
	((OPTION==2) ? (T_USE_DUALHOST)	:	(T_DISABLE)	)))
// End of [#2499]
#define GET_STD1_DCC_OPTION_APTEXTID(EN)				(((EN==1))?(T_HOST_DETERMINE):(T_DISABLE))
#define GET_STD1_PINCHANGE_OPTION_APTEXTID(OPTION)	\
	((OPTION==0) ? (T_DISABLE)	:			\
	((OPTION==1) ? (T_ENABLE)	:			\
	((OPTION==2) ? (T_HOST_DETERMINE)	:	(T_DISABLE)	)))
#define GET_LANGUAGE_APTEXTID(SELECT_LANGUAGE)	\
	((SELECT_LANGUAGE==6) ? (T_JAPANESE) :	\
	((SELECT_LANGUAGE==5) ? (T_KOREAN) :	\
	((SELECT_LANGUAGE==4) ? (T_CHINESE) : \
	((SELECT_LANGUAGE==3) ? (T_FRENCH) : \
	((SELECT_LANGUAGE==2) ? (T_SPANISH) : (T_ENGLISH))))))
#define GET_DUAL_RECEIPT_APTEXTID(SELECT_LANGUAGE)	\
	((SELECT_LANGUAGE==6) ? (T_JAPANESE) :	\
	((SELECT_LANGUAGE==5) ? (T_KOREAN) :	\
	((SELECT_LANGUAGE==4) ? (T_CHINESE) : \
	((SELECT_LANGUAGE==3) ? (T_FRENCH) : \
	((SELECT_LANGUAGE==2) ? (T_SPANISH) : \
	((SELECT_LANGUAGE==1) ? (T_ENGLISH) : (T_DISABLE)))))))
#define GET_EMV_TRAN_APTEXTID(EN) (((EN) ==0)?(T_MS_FIRST):(T_IC_FIRST))	
#define GET_BIN_ACTION_APTEXTID(ACTION)	\
	((ACTION == 1) ? (T_WITHDRAWAL_SURCHARGE) :	\
	((ACTION == 2) ? (T_BALANCE_SURCHARGE) :	\
	((ACTION == 3) ? (T_ACCEPT_CARD):	\
	((ACTION == 4) ? (T_DECLINE_CARD) :	\
	((ACTION == 5) ? (T_AWARD_COUPON) :	\
	((ACTION == 6) ? (T_PIN_CHANGE): (T_BLOCK_SURCHARGE) ))))))
#define GET_TCPIP_TYPE_APTEXTID(TYPE)	\
	((TYPE == 1) ? (T_VISA_FRAMED) : \
	((TYPE == 2) ? (T_STANDARD) : (T_ACK_CONTROLLED) ))
#define GET_PREDIAL_OPTION_APTEXTID(TYPE)	\
	((TYPE == 0) ? (T_AFTER_CARD) : \
	((TYPE == 1) ? (T_AFTER_PIN) : (T_AFTER_ACCOUNT) ))
#define GET_SCH_JOURNAL_OPTION_APTEXTID(OPTION)\
	((OPTION == 0) ? (T_COUNT) : \
	((OPTION == 1) ? (T_MONTHLY) : (T_DAILY) ))
// end of [#2160]

// [#2435] NH KSK 2016.07.14 Added Kernel V6
// [#2226] US Justin 2013.10.23 Add Kernel Version
//#define GET_EMV_KERNELVERSION_APTEXTID(VER)	\
//	((VER == 4) ? (T_EMV_KERNEL_VER4) :	\
//	((VER == 5) ? (T_EMV_KERNEL_VER5): (T_NOT_DEFINED) ))
#define GET_EMV_KERNELVERSION_APTEXTID(VER)	\
	((VER == 4) ? (T_EMV_KERNEL_VER4) :	\
	((VER == 5) ? (T_EMV_KERNEL_VER5) : \
	((VER == 6) ? (T_EMV_KERNEL_VER6) : \
	((VER == 7) ? (T_EMV_KERNEL_VER7) : (T_NOT_DEFINED) ))))
// End of [#2226]
// end of [#2435]

// [#2354] US Justin 2015.07.01 Surcharge Notice Option
#define GET_SURCHARGENOTICE_OPTION(ACTION)	\
	((ACTION == 0) ? (T_NOTICE_AMOUNT) :	\
	((ACTION == 1) ? (T_NOTICE_PERCENTAGE): (T_NOTICE_BOTH) ))
// End of [#2354]

// [#2434] Kook 2016.07.22 Receipt Optimizing with refactoring
#define GET_HALO_LEDMODE_APTEXTID(MODE) \
	((MODE == OFF_HALOLED_MODE) ? (T_OFF) : \
	((MODE == ON_HALOLED_MODE) ? (T_ON) : \
	((MODE == FLICKING_HALOLED_MODE) ? (T_FLICKING) : \
	((MODE == FADE_HALOLED_MODE) ? (T_FADE) : ( _T("") ) ))))

#define GET_HALO_COLOR_APTEXTID(COLOR) \
	((COLOR == HALOLED_COLOR_CYAN) ? (T_CYAN) : \
	((COLOR == HALOLED_COLOR_BLUE) ? (T_BLUE) : \
	((COLOR == HALOLED_COLOR_YELLOWGREEN) ? (T_YELLOWGREEN) : \
	((COLOR == HALOLED_COLOR_WHITE) ? (T_WHITE) : \
	((COLOR == HALOLED_COLOR_GREEN) ? (T_GREEN) : \
	((COLOR == HALOLED_COLOR_PINK) ? (T_PINK) : ( _T("") ) ))))))
// end of [#2434]

// [#2472] US Justin 2017.02.29 DCC Withdrawal Option
#define GET_DCC_WITHOPTION_APTEXTID(EN) ((EN)?(T_DCC_WITHOPT_EMVMASTER):(T_DCC_WITHOPT_OFF))
// End of [#2472]

// [#2481] US Justin 2017.04.21 DCC Withdrawal Option
#define GET_FB4UNKNOWNAID_APTEXTID(EN) ((EN)?(T_UNKNOWNAID_MAGTR):(T_UNKNOWNAID_FALLBACK))
// End of [#2481]

// [#2490] NH Justin 2017.06.15 Standard1 Dynamic Flow MAC Option
#define GET_STD1_DF_MAC_APTEXTID(EN) (((EN)==1)?(T_ENABLE):(T_DISABLE))	
// End of [#2490]

// [#RWC6-16]
#define GET_B4U_ENABLE_SVC_OPTION(EN) ((EN==1)?(T_ENABLE):(T_DISABLE))		
// end of [#RWC6-16]

// [#2496] US Justin 2017.08.17 Add BitCoin
#define GET_JUSTCASH_ENABLE_SVC_OPTION(ACTION)	\
	((ACTION == 0) ? (T_DISABLE) :	\
	((ACTION == JUSTCASH_SERVICE_CARDLESS) ? (T_JUSTCASH_CARDLESS) : \
	((ACTION == JUSTCASH_SERVICE_DIGITAL_CURRENCY) ? (T_JUSTCASH_DIGITALCUR) : (T_JUSTCASH_BOTH) ) ) )
// End of [#2354]

// [#2538] NH Justin 2018.03.09 MoniMobile Cash Balance Option
#define GET_ENDIS_MONIMOBILE_CASHBALANCE(EN) ((EN==1)?(T_ENABLE):(T_DISABLE))						
// End of [#2538]

// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Optioin
#define GET_REBOOT_OPTION_APTEXTID(OPTION) \
	((OPTION == REBOOT_OPTION_DISABLE) ? (T_DISABLE) : \
	((OPTION == REBOOT_OPTION_ONTIME) ? (T_REBOOT_ONTIME) : \
	((OPTION == REBOOT_OPTION_INTERVAL) ? (T_REBOOT_INTERVAL) : (T_DISABLE) )))
// End of [#2558]

// [#2569] US Justin 2018.07.30 Add DCC OPTION
#define GET_DCC_CUST_OPTION_STRING(OPTION)	\
	((OPTION == DCC_CUSTOMOPTION_GENERAL) ? (_T("GENERAL")) :	\
	((OPTION == DCC_CUSTOMOPTION_CIBC) ? (_T("CIBC")) :	\
	((OPTION == DCC_CUSTOMOPTION_CARDTRONICS) ? (_T("CARDTRONICS")) :	\
	((OPTION == DCC_CUSTOMOPTION_CDS) ? (_T("CDS")):	\
	((OPTION == DCC_CUSTOMOPTION_PLANET) ? (_T("PLANET")) :	\
	((OPTION == DCC_CUSTOMOPTION_MONEX) ? (_T("MONEX")): \
	((OPTION == DCC_CUSTOMOPTION_BANSI) ? (_T("CIBC (MX)")): \
	((OPTION == DCC_CUSTOMOPTION_MAX) ? (_T("DEFAULT")): (_T("DEFAULT")) ))))))))
// End of [#2569]

// [#J006] GivePay Enhancement 5
#define GET_GIVEPAY_APPEAR_OPTION(OPTION)	\
	((OPTION == GIVEPAY_APPEAR_INDIVIDUAL) ? (T_INDI) :	\
	((OPTION == GIVEPAY_APPEAR_PIGGYBACK) ? (T_PIGGYBACK) : (T_BOTH) ))
// End of [#J006]

// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
#define GET_DYNAMICFLOW_DCC_TYPE_STRING(OPTION)	\
	((OPTION == DYNAMICFLOW_DCC_TYPE_BOTH) ? (_T("BOTH")) :	\
	((OPTION == DYNAMICFLOW_DCC_TYPE_MASTERONLY) ? (_T("MASTER ONLY")) :	\
	((OPTION == DYNAMICFLOW_DCC_TYPE_VISAONLY) ? (_T("VISA ONLY")):	(_T("BOTH")) )))

#define GET_DUALHOST_DCC_TYPE_STRING(OPTION)	\
	((OPTION == DUALHOST_DCC_TYPE_BOTH) ? (_T("BOTH")) :	\
	((OPTION == DUALHOST_DCC_TYPE_MASTERONLY) ? (_T("MASTER ONLY")) :	\
	((OPTION == DUALHOST_DCC_TYPE_VISAONLY) ? (_T("VISA ONLY")):	(_T("BOTH")) )))
// end of [#RWC6-2, #2585]

// [#RWC6-27]
#define GET_DISPENSE_STYLE_STRING(OPTION)	\
	((OPTION == DISPENSE_STYLE_LEAST) ? (_T("LEAST # OF BILLS")) :	\
	((OPTION == DISPENSE_STYLE_EVEN) ? (_T("EVEN # OF BILLS")) :	\
	((OPTION == DISPENSE_STYLE_BALANCED) ? (_T("BALANCED AMOUNT")) :	\
	((OPTION == DISPENSE_STYLE_MOST) ? (_T("MOST # OF BILLS")):	(_T("LEAST # OF BILLS")) ))))
// end of [#RWC6-27]

// move screedDescType.h
/*------------------------------------------------------------------
	SCREEN TIMEOUT MODE
------------------------------------------------------------------*/
//#define SCR_TMODE_FIRST		0
//#define SCR_TMODE_LAST		1
/*------------------------------------------------------------------
	MOVE SCREEN PROCEDURE REASON
------------------------------------------------------------------*/
#define MOVE_PROC_TIMEOUT		1						// 화면 Time out
#define MOVE_PROC_EXIT			2						// 사용자 Cancel 선택.
#define MOVE_PROC_SELECT		3						// 사용자 버튼 선택.
// end of [#236]

/*------------------------------------------------------------------
	FS COMMAND RETURN VALUE
------------------------------------------------------------------*/
#define S_ENTER				_T("ENTER")					// ENTER
#define S_RETURN			_T("RETURN")				// RETURN
#define S_EXIT				_T("EXIT")					// EXIT
#define S_CANCEL			_T("CANCEL")				// CANCEL
#define S_TIMEOVER			_T("TIMEOVER")				// TIME OVER

/*------------------------------------------------------------------
	AP SCREEN TEXT ID
-------------------------------------------------------------------*/

/*------------------------------------------------------------------
	AP Common TextID
-------------------------------------------------------------------*/
#define	T_APMSG_000008		_T("APTextID:000008")				// "WITHDRAWAL"
#define	T_APMSG_000009		_T("APTextID:000009")				// "BALANCE INQUIRY"
#define	T_APMSG_000010		_T("APTextID:000010")				// "TRANSFER"
#define	T_APMSG_000011		_T("APTextID:000011")				// "FROM"
#define	T_APMSG_000012		_T("APTextID:000012")				// "TO"
#define	T_APMSG_000013		_T("APTextID:000013")				// "CHECKING"
#define	T_APMSG_000014		_T("APTextID:000014")				// "SAVINGS"
#define	T_APMSG_000015		_T("APTextID:000015")				// "CREDIT"
#define	T_APMSG_000016		_T("APTextID:000016")				// "[RECEIPT NOT AVAILABLE]"
#define	T_APMSG_000018		_T("APTextID:000018")				// "BALANCE ENQUIRY"	[#544] [AU] SOOK 2009.07.21


/*------------------------------------------------------------------
	Screen Number 001												
-------------------------------------------------------------------*/
#define	T_APMSG_001001		_T("APTextID:001001")				// "Printing your transaction record"
#define	T_APMSG_001002		_T("APTextID:001002")				// "Over max amount. Please try again!"
#define	T_APMSG_001003		_T("APTextID:001003")				// "Too low amount. Please try again!"
#define	T_APMSG_001004		_T("APTextID:001004")				// "Indispensable amount. Please try again!" [#191] NH KSK 2008.05.13
//#define	T_APMSG_001005		_T("APTextID:001005")				// "Thank you for using our service." -> 004 화면으로 이동
//#define	T_APMSG_001006		_T("APTextID:001006")				// "Time is over! Please try again!"  -> 004 화면으로 이동
#define	T_APMSG_001008		_T("APTextID:001008")				// "Configuration error!"
#define	T_APMSG_001009		_T("APTextID:001009")				// "ATM error during transaction"
#define	T_APMSG_001010		_T("APTextID:001010")				// "Program abnormal"
#define	T_APMSG_001011		_T("APTextID:001011")				// "Your transaction is cancelled"
#define	T_APMSG_001012		_T("APTextID:001012")				// "Please wait! Change operator mode now."
#define	T_APMSG_001013		_T("APTextID:001013")				// "Please wait! Change service mode now."
#define	T_APMSG_001014		_T("APTextID:001014")				// "We are sorry. Unable to dispense cash."
#define	T_APMSG_001015		_T("APTextID:001015")				// "Do not remove your card"	// [#279] UK KGS 2008.06.11 카드 수취 불가 안내 화면
#define	T_APMSG_001016		_T("APTextID:001016")				// "Failed to select the AID"	// [#279] UK KGS 2008.06.11 카드 수취 불가 안내 화면
#define	T_APMSG_001017		_T("APTextID:001017")				// "We are sorry. Door is opened."
#define	T_APMSG_001018		_T("APTextID:001018")				// "Key data error from host."
#define	T_APMSG_001019		_T("APTextID:001019")				// "Must enter at least 6 digits"
#define	T_APMSG_001020		_T("APTextID:001020")				// "Must enter at least 3 digits"
#define T_APMSG_001021		_T("APTextID:001021")				// "The amount you have entered is invalid..Please try again!"

//[#257] [MX] KSK 2008.6.5 SPANISH 미지원 문구 추가
/*------------------------------------------------------------------
	Screen Number 003												
-------------------------------------------------------------------*/
#define T_APMSG_003002		_T("APTextID:003002")				// "Power off while dispensing"
#define T_APMSG_003003		_T("APTextID:003003")				// "Cassette is not defined"
#define T_APMSG_003004		_T("APTextID:003004")				// "DOOR CHECK"
#define T_APMSG_003005		_T("APTextID:003005")				// "LIGHT ERROR"
#define T_APMSG_003006		_T("APTextID:003006")				// "PIN ERROR"
#define T_APMSG_003007		_T("APTextID:003007")				// "SENSOR CHECK"
#define T_APMSG_003008		_T("APTextID:003008")				// "SCREEN ERROR"
#define T_APMSG_003009		_T("APTextID:003009")				// "OUT OF RECEIPT"
#define T_APMSG_003010		_T("APTextID:003010")				// "RECEIPT PRINTER ERROR"
#define T_APMSG_003011		_T("APTextID:003011")				// "CDU RESET FAIL"
#define T_APMSG_003012		_T("APTextID:003012")				// "DENOMINATION IS NOT SET"
#define T_APMSG_003013		_T("APTextID:003013")				// "CASH DISPENSER ERROR"
#define T_APMSG_003014		_T("APTextID:003014")				// "CARD READER ERROR"
#define T_APMSG_003015		_T("APTextID:003015")				// "CDU LINE OFF"
#define T_APMSG_003016		_T("APTextID:003016")				// "NOTE DETECTED"
#define T_APMSG_003017		_T("APTextID:003017")				// "RECORD FORMAT ERROR"
#define T_APMSG_003018		_T("APTextID:003018")				// "ROUTING ID ERROR"
#define T_APMSG_003019		_T("APTextID:003019")				// "TERMINAL ID ERROR"
#define T_APMSG_003020		_T("APTextID:003020")				// "RESPONSE CODE ERROR"
#define T_APMSG_003021		_T("APTextID:003021")				// "FIRST DES KEY ERROR"
#define T_APMSG_003022		_T("APTextID:003022")				// "SECOND DES KEY ERROR"
#define T_APMSG_003023		_T("APTextID:003023")				// "THIRD DES KEY ERROR"
#define T_APMSG_003024		_T("APTextID:003024")				// "FOURTH DES KEY ERROR"
#define T_APMSG_003025		_T("APTextID:003025")				// "TRANS SEQUENCE NO. ERROR"
#define T_APMSG_003026		_T("APTextID:003026")				// "TRANSACTION DATE ERROR"
#define T_APMSG_003027		_T("APTextID:003027")				// "TRANSACTION TIME ERROR"
#define T_APMSG_003028		_T("APTextID:003028")				// "RETRIVAL REFNO. ERROR"
#define T_APMSG_003029		_T("APTextID:003029")				// "SYSTEM TRACE AUDIT NO. ERROR"
#define T_APMSG_003030		_T("APTextID:003030")				// "NETWORK ID CODE ERROR"
#define T_APMSG_003031		_T("APTextID:003031")				// "SETTLEMENT ERROR"
#define T_APMSG_003032		_T("APTextID:003032")				// "MAC MISMATCH"
#define T_APMSG_003033		_T("APTextID:003033")				// "CRC MISMATCH"
#define T_APMSG_003034		_T("APTextID:003034")				// "NO CRC IS RECEIVED"
#define T_APMSG_003035		_T("APTextID:003035")				// "HOST MAC RESULT ERROR"
#define T_APMSG_003036		_T("APTextID:003036")				// "NO MAC IS RECEIVED"
#define T_APMSG_003037		_T("APTextID:003037")				// "FIRST FIELD ID CODE ERROR"
#define T_APMSG_003038		_T("APTextID:003038")				// "SURCHARGE AMOUNT ERROR"
#define T_APMSG_003039		_T("APTextID:003039")				// "BUSINESS DATE ERROR"
#define T_APMSG_003040		_T("APTextID:003040")				// "AUTHORIZATION NO. ERROR"
#define T_APMSG_003041		_T("APTextID:003041")				// "LEDGER BALANCE ERROR"
#define T_APMSG_003042		_T("APTextID:003042")				// "Reject Bin is Full"
#define T_APMSG_003043		_T("APTextID:003043")				// "NVRAM is broken"
#define T_APMSG_003044		_T("APTextID:003044")				// "CDU Firmware Incorrect"
#define T_APMSG_003045		_T("APTextID:003045")				// "SURCHARGE OWNER is not set"
#define T_APMSG_003046		_T("APTextID:003046")				// "SURCHARGE AMOUNT is not set"
#define T_APMSG_003047		_T("APTextID:003047")				// "MASTER KEY INDEX ERROR"
#define T_APMSG_003048		_T("APTextID:003048")				// "MASTER KEY Empty"
#define T_APMSG_003049		_T("APTextID:003049")				// "HOST PHONE NO. is not set"
#define T_APMSG_003050		_T("APTextID:003050")				// "RMS PASSWORD is not set"
#define T_APMSG_003051		_T("APTextID:003051")				// "RMS PHONE NO. is not set"
#define T_APMSG_003052		_T("APTextID:003052")				// "TERMINAL ID is not set"
#define T_APMSG_003053		_T("APTextID:003053")				// "COMMUNICATION ID is not set"
#define T_APMSG_003054		_T("APTextID:003054")				// "ROUTING ID is not set"
#define T_APMSG_003055		_T("APTextID:003055")				// "Change Master Password"
#define T_APMSG_003056		_T("APTextID:003056")				// "HOST IP is not set"
#define T_APMSG_003057		_T("APTextID:003057")				// "RMS IP is not set"
#define T_APMSG_003058		_T("APTextID:003058")				// "Cassette is not properly set"
#define T_APMSG_003059		_T("APTextID:003059")				// "Cash cassette is low"
#define T_APMSG_003060		_T("APTextID:003060")				// "Number of Bill is 0 (Empty)"
#define T_APMSG_003061		_T("APTextID:003061")				// "Door is opened"
#define T_APMSG_003062		_T("APTextID:003062")				// "Modem Dial Connection Timeout"
#define T_APMSG_003063		_T("APTextID:003063")				// "TCP/IP Connection Timeout"
#define T_APMSG_003064		_T("APTextID:003064")				// "No Dial Tone"
#define T_APMSG_003065		_T("APTextID:003065")				// "No Answer"
#define T_APMSG_003066		_T("APTextID:003066")				// "Line Busy"
#define T_APMSG_003067		_T("APTextID:003067")				// "No Carrier(No ENQ Received)"
#define T_APMSG_003068		_T("APTextID:003068")				// "No ACK/NAK Received"
#define T_APMSG_003069		_T("APTextID:003069")				// "No Message Data Received"
#define T_APMSG_003070		_T("APTextID:003070")				// "No EOT received from HOST"
#define T_APMSG_003071		_T("APTextID:003071")				// "NAK retry exceed"
#define T_APMSG_003072		_T("APTextID:003072")				// "Send retry exceed"
#define T_APMSG_003073		_T("APTextID:003073")				// "Modem Initialize fail"
#define T_APMSG_003074		_T("APTextID:003074")				// "Modem open fail"
#define T_APMSG_003075		_T("APTextID:003075")				// "Modem is not responding"
#define T_APMSG_003076		_T("APTextID:003076")				// "ERROR"
#define T_APMSG_003077		_T("APTextID:003077")				// "MSG"
#define T_APMSG_003078		_T("APTextID:003078")				// "Invalid Transaction"
#define T_APMSG_003079		_T("APTextID:003079")				// "CONVENIENCE FEE AMOUNT WRONG SIZE"
#define T_APMSG_003080		_T("APTextID:003080")				// "CONVENIENCE FEE AMOUNT ERROR"
#define	T_APMSG_003081		_T("APTextID:003081")				// "TERMINAL ID"
#define	T_APMSG_003082		_T("APTextID:003082")				// "ERROR CODE"
#define	T_APMSG_003083		_T("APTextID:003083")				// "Reversal transaction failure"
#define T_APMSG_003084		_T("APTextID:003084")				// "CARD READER ERROR"
//#define T_APMSG_003085		_T("APTextID:003085")				// "IC CARD DENIAL"
//#define T_APMSG_003086		_T("APTextID:003086")				// "IC CARD ONLINE DATA ERROR"
#define T_APMSG_003087		_T("APTextID:003087")				// "IC CARD ERROR"

#define T_APMSG_003088		_T("APTextID:003088")				// "Failed to write Journal"	// [#462] [NH] KSK 2008.12.09
#define T_APMSG_003089		_T("APTextID:003089")				// "Invalid Exchange Rate"		// [#514] [MX] KSK 2009.3.4

#define T_APMSG_003090		_T("APTextID:003090")				// "FIFTH DES KEY ERROR"		// [#565] NH KSK 2009.8.20
//#define T_APMSG_003091		_T("APTextID:003091")				// "SUB RESPONSE CODE ERROR"	// [#560] NH KSK 2009.8.20	KSK 2009.8.24 추후 작업
#define T_APMSG_003092		_T("APTextID:003092")				// "SSL Connection failure."		// [#711] NH PCS 2011.10.18	WinCE5.0과 동일하게 맞춤.		

#define T_APMSG_003093		_T("APTextID:003093")				// "RFID READER ERROR"	[#2325] NH KSK 2015.01.24

#define T_APMSG_003358		_T("APTextID:003358")				// "Please Notify Attendant"		

#define	T_APMSG_003100		_T("APTextID:0031")					// STANDARD2 EPS HOST DENIAL MSG	// [#310] [MX] KSK 2008.6.18 HOST DENIAL MESSAGE
#define	T_APMSG_003200		_T("APTextID:0032")					// STANDARD3 HOST DENIAL MSG	// [#310] [MX] KSK 2008.6.18 HOST DENIAL MESSAGE
#define	T_APMSG_003300		_T("APTextID:0033")					// STANDARD1 HOST DENIAL MSG	// [#310] [MX] KSK 2008.6.18 HOST DENIAL MESSAGE KSK 2009.4.25
// end of [#257]

// [#2011] NH KJW 2011.01.19
#define	T_APMSG_003359		_T("APTextID:003359")				// "INTERFACE VERSION MISMATCH"
#define	T_APMSG_003363		_T("APTextID:003363")				// "BASE64 ENCODE ERROR"
#define	T_APMSG_003364		_T("APTextID:003364")				// "BASE64 DECODE ERROR"
#define	T_APMSG_003365		_T("APTextID:003365")				// "ZIP COMPRESS ERROR"
#define	T_APMSG_003366		_T("APTextID:003366")				// "ZIP DECOMPRESS ERROR"
// end of [#2011]

#define	T_APMSG_003401		_T("APTextID:003401")				// "Remote Out of service"			// [#2277] NH Justin 2014.06.10 Remote in/out of service
#define	T_APMSG_003402		_T("APTextID:003402")				// "Change service password"		// [#2278] NH Justin 2014.06.10 Check Default Service Password
#define	T_APMSG_003403		_T("APTextID:003403")				// "Change Operator Password"		// [#2278] NH Justin 2014.06.10 Check Default Operator Password


#define	T_APMSG_003353		_T("APTextID:003353")				// "Host error"						// [RWC6-676] SKKim 2024.05.28


// [#250] NZ AIREAT 2008.05.30 - String Define 추가.
/*------------------------------------------------------------------
	Screen Number 004												
-------------------------------------------------------------------*/
#define T_APMSG_004001		_T("APTextID:004001")				// TRANSACTION CANCELLED (NZ / AU)
																// TRANSACTION IS BEEN CANCELLED (US / MX / CA)
#define	T_APMSG_001005		_T("APTextID:001005")				// "Thank you for using our service."
#define	T_APMSG_001006		_T("APTextID:001006")				// "Time is over! Please try again!"
// end of [#250]

// [#250] NZ AIREAT 2008.06.09 - 015 SCR 삭제. 005 SCR으로 통합.
/*------------------------------------------------------------------
	Screen Number 005												
-------------------------------------------------------------------*/
#define T_APMSG_005001		_T("APTextID:005001")				// Sorry! I can't recognize that card. Please try again.
#define T_APMSG_005002		_T("APTextID:005002")				// Sorry, The PAN data fails a Modulus-10 check. Please contact your CARD Issuer.
#define T_APMSG_005003		_T("APTextID:005003")				// We apologise. But this card cannot be used in this network. Please contact your bank.
#define T_APMSG_005004		_T("APTextID:005004")				// Sorry! I can't recognise that card. Please try again.//[#544] [AU] SOOK 2009.07.21

// end of [#250]

// [#249] MX KSK 2008.05.29 명세표 인자시 스페인어 지원
/*------------------------------------------------------------------
	NOTICE DATA FOR SHOW
-------------------------------------------------------------------*/
#define T_NOTICE_TITLE		_T("APTextID:N_TITLE")				// Title
#define T_NOTICE_LINE1		_T("APTextID:N_LINE1")				// Line 1
#define T_NOTICE_LINE2		_T("APTextID:N_LINE2")				// Line 2
#define T_NOTICE_LINE3		_T("APTextID:N_LINE3")				// Line 3
#define T_NOTICE_LINE4		_T("APTextID:N_LINE4")				// Line 4
#define T_NOTICE_LINE5		_T("APTextID:N_LINE5")				// Line 5
#define T_NOTICE_LINE6		_T("APTextID:N_LINE6")				// Line 6
#define T_NOTICE_LINE7		_T("APTextID:N_LINE7")				// Line 7
#define T_NOTICE_LINE8		_T("APTextID:N_LINE8")				// Line 8
#define T_NOTICE_LINE9		_T("APTextID:N_LINE9")				// Line 9

/*------------------------------------------------------------------
	HISTORY DATA
-------------------------------------------------------------------*/
#define T_HISTORY_ENGLISH		_T("APTextID:H_ENGLISH")			// English
#define T_HISTORY_SPANISH		_T("APTextID:H_SPANISH")			// Spanish
#define T_HISTORY_FRENCH		_T("APTextID:H_FRENCH")				// French
#define T_HISTORY_CHINESE		_T("APTextID:H_CHINESE")			// Chinese
#define T_HISTORY_KOREAN		_T("APTextID:H_KOREAN")				// Korean
#define T_HISTORY_JAPANESE		_T("APTextID:H_JAPANESE")			// Japanese

#define T_HISTORY_WITHDRAWAL	_T("APTextID:H_WITHDRAWAL")			// Widthdrawal
#define T_HISTORY_BALANCE		_T("APTextID:H_BALANCE")			// Balance
#define T_HISTORY_TRANSFER		_T("APTextID:H_TRANSFER")			// Transfer
#define T_HISTORY_PINCHANGE		_T("APTextID:H_PINCHANGE")			// Pin Change			// [#2150] US Justin 2012.10.04 Dynamic Flow Add Pin Change
#define T_HISTORY_JC_BITCOIN	_T("APTextID:H_JC_BITCOIN")			// Just.Cash BitCoin	// [#2496] US Justin 2017.08.14
#define T_HISTORY_GIVEPAY		_T("APTextID:H_GIVEPAY")			// GivePay				// [#2530] US Justin 2018.01.24

#define T_HISTORY_RECEIPT		_T("APTextID:H_RECEIPT")			// Receipt

#define T_HISTORY_CHECKING		_T("APTextID:H_CHECKING")			// Checking
#define T_HISTORY_SAVING		_T("APTextID:H_SAVING")				// Saving
#define T_HISTORY_CREDIT		_T("APTextID:H_CREDIT")				// Credit
#define T_HISTORY_BOTH			_T("APTextID:H_BOTH")				// Both Checking and Saving

// [#2186] US KMK 2013.04.17 미사용
/*
#define T_HISTORY_FROM_CHECKING		_T("APTextID:H_FROM_CHECKING")		// From Checking
#define T_HISTORY_FROM_SAVING		_T("APTextID:H_FROM_SAVING")		// From Saving
#define T_HISTORY_FROM_CREDIT		_T("APTextID:H_FROM_CREDIT")		// From Credit

#define T_HISTORY_TO_CHECKING	_T("APTextID:H_TO_CHECKING")	// To Checking
#define T_HISTORY_TO_SAVING		_T("APTextID:H_TO_SAVING")		// To Saving
#define T_HISTORY_TO_CREDIT		_T("APTextID:H_TO_CREDIT")		// To Credit
*/
// end of [#2186]

/*------------------------------------------------------------------
	RECEIPT DATA
-------------------------------------------------------------------*/
#define	T_MSG_SHOW_001		_T("APTextID:T_SHOW_001")				// "Nautilus Hyosung America Inc."
#define	T_MSG_SHOW_002		_T("APTextID:T_SHOW_002")				// "1025 S.Belt Line Rd. COPPELL TX"

#define	T_MSG_SHOW_101		_T("APTextID:T_SHOW_101")				// "ATM INDUSTRY ASSOCIATION"
#define	T_MSG_SHOW_102		_T("APTextID:T_SHOW_102")				// "ATMIA 11th Annual Conference & Expo"
#define	T_MSG_SHOW_103		_T("APTextID:T_SHOW_103")				// "ATMs 2010 :"
#define	T_MSG_SHOW_104		_T("APTextID:T_SHOW_104")				// "Opportunities for Enhanced Value"

#define	T_MSG_RECE_001		_T("APTextID:T_RECE_001")				// "TERMINAL #"
#define	T_MSG_RECE_002		_T("APTextID:T_RECE_002")				// "SEQUENCE #"
#define	T_MSG_RECE_003		_T("APTextID:T_RECE_003")				// "AUTH #"
#define	T_MSG_RECE_004		_T("APTextID:T_RECE_004")				// "DATE & TIME"
#define	T_MSG_RECE_005		_T("APTextID:T_RECE_005")				// "BUSINESS DATE"
#define	T_MSG_RECE_006		_T("APTextID:T_RECE_006")				// "CARD NUMBER"
#define	T_MSG_RECE_007		_T("APTextID:T_RECE_007")				// "TRANSACTION"
#define	T_MSG_RECE_008		_T("APTextID:T_RECE_008")				// "ACCOUNT"
#define	T_MSG_RECE_009		_T("APTextID:T_RECE_009")				// "DISPENSED AMOUNT"
#define	T_MSG_RECE_010		_T("APTextID:T_RECE_010")				// "REQUESTED AMOUNT"
#define	T_MSG_RECE_011		_T("APTextID:T_RECE_011")				// "TRANSFER AMOUNT"
#define	T_MSG_RECE_012		_T("APTextID:T_RECE_012")				// "TOTAL AMOUNT"
#define	T_MSG_RECE_013		_T("APTextID:T_RECE_013")				// "BALANCE UNAVAIlABLE"
#define	T_MSG_RECE_014		_T("APTextID:T_RECE_014")				// "LEDGER BALANCE"
#define	T_MSG_RECE_015		_T("APTextID:T_RECE_015")				// "AVAILABLE BALANCE"
#define	T_MSG_RECE_016		_T("APTextID:T_RECE_016")				// "SURCHARGE PAID TO"
#define	T_MSG_RECE_017		_T("APTextID:T_RECE_017")				// "  * TRANSACTION CANCELLED"
#define	T_MSG_RECE_018		_T("APTextID:T_RECE_018")				// "  * ERROR CODE"
#define	T_MSG_RECE_019		_T("APTextID:T_RECE_019")				// "----------------CUT HERE---------------"
#define	T_MSG_RECE_020		_T("APTextID:T_RECE_020")				// " CUSTOMER NUMBER "
#define	T_MSG_RECE_021		_T("APTextID:T_RECE_021")				// "ATM Fee          "

// [#2115] MX KSK 2012.02.02
#define	T_MSG_RECE_022		_T("APTextID:T_RECE_022")				// "EXCHANGE RATE    "
#define	T_MSG_RECE_023		_T("APTextID:T_RECE_023")				// "IVA (TAX)        " // [#RWC6-331] CIBC screen updates
#define	T_MSG_RECE_024		_T("APTextID:T_RECE_024")				// "BANK NAME        "
#define	T_MSG_RECE_025		_T("APTextID:T_RECE_025")				// "LOYALTY FEE      "
#define	T_MSG_RECE_026		_T("APTextID:T_RECE_026")				// "LINE USAGE FEE   "
#define	T_MSG_RECE_027		_T("APTextID:T_RECE_027")				// "ACCOUNT NUMBER   "
#define	T_MSG_RECE_028		_T("APTextID:T_RECE_028")				// "ARQC             "
#define	T_MSG_RECE_029		_T("APTextID:T_RECE_029")				// "ARPC		     "
#define	T_MSG_RECE_030		_T("APTextID:T_RECE_030")				// "EXCHANGE FEE" //[#2137] MX PCS 2012.07.06
// end of [#2115]

// [#2150] US Justin 2012.10.04 
#define	T_MSG_RECE_031		_T("APTextID:T_RECE_031")				// "Exchange Rate"
#define	T_MSG_RECE_032		_T("APTextID:T_RECE_032")				// "Account Charges"
// End of [#2150]
#define	T_MSG_RECE_033		_T("APTextID:T_RECE_033")				// "Service Fee" (DCC) [#2202] CA Justin 2013.06.20 Surcharge => Service Fee (CIBC Only)
#define	T_MSG_RECE_034		_T("APTextID:T_RECE_034")				// "Withdrawal"  (DCC) [#2202] CA Justin 2013.06.20 Disensed Amount => Withdrawal (CIBC Only)
#define	T_MSG_RECE_037		_T("APTextID:T_RECE_037")				// "ACCESS CODE (POP MONEY)  [#2350] US Justin 2015.06.19 Access Code

#define	T_MSG_RECE_042		_T("APTextID:T_RECE_042")				// "TSI"				[#2478] NH Justin 2017.04.10 Print TSI, TVR, and TC on a receipt.
#define	T_MSG_RECE_043		_T("APTextID:T_RECE_043")				// "TC"					[#2478] NH Justin 2017.04.10 Print TSI, TVR, and TC on a receipt.
#define	T_MSG_RECE_044		_T("APTextID:T_RECE_044")				// "AAC"				[#2478] NH Justin 2017.04.10 Print TSI, TVR, and TC on a receipt.
#define	T_MSG_RECE_045		_T("APTextID:T_RECE_045")				// "TVR"				[#2478] NH Justin 2017.04.10 Print TSI, TVR, and TC on a receipt.
#define	T_MSG_RECE_046		_T("APTextID:T_RECE_046")				// "Public Key"			[#2496] NH Justin 2017.08.17 Add Bitcoin
#define	T_MSG_RECE_047		_T("APTextID:T_RECE_047")				// "Private Key"		[#2496] NH Justin 2017.08.17 Add Bitcoin
#define	T_MSG_RECE_048		_T("APTextID:T_RECE_048")				// "Unit Price"			[#2496] NH Justin 2017.08.17 Add Bitcoin
#define	T_MSG_RECE_049		_T("APTextID:T_RECE_049")				// "Quantity"			[#2496] NH Justin 2017.08.17 Add Bitcoin
#define	T_MSG_RECE_050		_T("APTextID:T_RECE_050")				// "Total"				[#2496] NH Justin 2017.08.17 Add Bitcoin
#define	T_MSG_RECE_051		_T("APTextID:T_RECE_051")				// "Wallet"				[#2503] NH Justin 2017.09.15 Add Ethereum

#define	T_MSG_RECE_052		_T("APTextID:T_RECE_052")				// "Exchange Rate Mark-up"
#define	T_MSG_RECE_053		_T("APTextID:T_RECE_053")				// "XXXXX"

#define T_MSG_RECE_054		_T("APTextID:T_RECE_054")				// "TRANSACTION AMOUNT" // Cardtronics custom receipt for DCC
#define T_MSG_RECE_055		_T("APTextID:T_RECE_055")				// "TOTAL TRANSACTION AMOUNT" // Cardtronics custom receipt for DCC

// [#RWC6-276] Updates to Planet Payment DCC screen/receipt
#define	T_MSG_RECE_056		_T("APTextID:T_RECE_056")				// "Cash Withdrawal"
#define	T_MSG_RECE_057		_T("APTextID:T_RECE_057")				// "Access Fee"
#define	T_MSG_RECE_058		_T("APTextID:T_RECE_058")				// "Total Amount"
#define	T_MSG_RECE_059		_T("APTextID:T_RECE_059")				// "Exchange Rate"
#define	T_MSG_RECE_060		_T("APTextID:T_RECE_060")				// "Transaction Amount"
// end of [#RWC6-276]

#define	T_MSG_RECE_061		_T("APTextID:T_RECE_061")				// "EQUIVALENT AMOUNT"
#define T_MSG_RECE_062		_T("APTextID:T_RECE_062")				// "SURCHARGE"
#define T_MSG_RECE_063		_T("APTextID:T_RECE_063")				// "MARK-UP"

/*--------------------------------------------------------------------
  DCC Receipts
----------------------------------------------------------------------*/
#define	T_MSG_RECE_821		_T("APTextID:T_RECE_821")				// "ATM Fee          "
#define	T_MSG_RECE_831		_T("APTextID:T_RECE_831")				// "Exchange Rate"
#define	T_MSG_RECE_832		_T("APTextID:T_RECE_832")				// "Account Charges"
#define	T_MSG_RECE_852		_T("APTextID:T_RECE_852")				// "Exchange Rate Mark-up"
#define	T_MSG_RECE_856		_T("APTextID:T_RECE_856")				// "Cash Withdrawal"
// end of [RWC6-607

#define	T_MSG_DATA_001		_T("APTextID:T_DATA_001")				// "WITHDRAWAL"
#define	T_MSG_DATA_002		_T("APTextID:T_DATA_002")				// "BALANCE"
#define	T_MSG_DATA_003		_T("APTextID:T_DATA_003")				// "TRANSFER"
#define	T_MSG_DATA_005		_T("APTextID:T_DATA_005")				// "FROM"
#define	T_MSG_DATA_006		_T("APTextID:T_DATA_006")				// "TO"
#define	T_MSG_DATA_007		_T("APTextID:T_DATA_007")				// "CHECKING"
#define	T_MSG_DATA_008		_T("APTextID:T_DATA_008")				// "SAVINGS"
#define	T_MSG_DATA_009		_T("APTextID:T_DATA_009")				// "CREDIT"

#define	T_MSG_DATA_010		_T("APTextID:T_DATA_010")				// "APPROVED"
#define	T_MSG_DATA_011		_T("APTextID:T_DATA_011")				// "DECLINED"
#define	T_MSG_DATA_012		_T("APTextID:T_DATA_012")				// "PARTIAL REVERSAL"
#define	T_MSG_DATA_013		_T("APTextID:T_DATA_013")				// "FULL REVERSAL"

#define	T_MSG_DATA_014		_T("APTextID:T_DATA_014")				// "PIN CHANGE"				// [#2150] US Justin 2012.10.09
#define	T_MSG_DATA_016		_T("APTextID:T_DATA_016")				// "DUAL BALANCE"			// [#2292] US Justin 2014.10.10 Dual Balance
#define	T_MSG_DATA_017		_T("APTextID:T_DATA_017")				// "Popmoney WITHDRAWAL"	// [#2350] US Justin 2015.06.19 Add POP MOney Transaction Type

#define	T_MSG_DATA_018		_T("APTextID:T_DATA_018")				// "LOCAL CASH"				// [#2385] US Justin 2016.01.11 Add Canadian Local Loan
#define	T_MSG_DATA_019		_T("APTextID:T_DATA_019")				// "CASH PICK-UP"			// [#2405] US Justin 2016.03.21 HalCash Online	// [#2470]

#define	T_MSG_DATA_020		_T("APTextID:T_DATA_020")				// "Please visit another Pin4 terminal to"		// [#2405] US Justin 2016.03.21 HalCash Online
#define	T_MSG_DATA_021		_T("APTextID:T_DATA_021")				// "complete this transaction"					// [#2405] US Justin 2016.03.21 HalCash Online
#define	T_MSG_DATA_022		_T("APTextID:T_DATA_022")				// "Please have sender contact sender agent"	// [#2405] US Justin 2016.03.21 HalCash Online
#define	T_MSG_DATA_023		_T("APTextID:T_DATA_023")				// "to initiate claim for balance of funds."	// [#2405] US Justin 2016.03.21 HalCash Online
#define	T_MSG_DATA_025		_T("APTextID:T_DATA_025")				// "Just.Cash"									// [#2445] US Justin 2016.09.28 Just.Cash
#define	T_MSG_DATA_026		_T("APTextID:T_DATA_026")				// "Cardless Cash"								// [#2446] US Justin 2016.09.30 Paypal CCA
#define	T_MSG_DATA_027		_T("APTextID:T_DATA_027")				// "Bitcoin"									// [#2496] US Justin 2017.08.17 Bitcoin
#define	T_MSG_DATA_028		_T("APTextID:T_DATA_028")				// "Ethereum"			
#define	T_MSG_DATA_029		_T("APTextID:T_DATA_029")				// "Bitload4U"	// [#2503]


// [#2419] US Justin 2016.04.20 Assign OP Menu Dymically
#define	T_OPVAP_TDL			_T("APTextID:T_OPVAP_TDL")				// "TDL OPTION"
#define T_OPVAP_LIBERTYX	_T("APTextID:T_OPVAP_LIBERTYX")			// "LibertyX" [#RWC6-59] US William 2019.10.03 LibertyX
// End of [#2419]

#define T_OPVAP_DIGITALMINT	_T("APTextID:T_OPVAP_DIGITALMINT")
#define T_OPVAP_CASHDEPOT	_T("APTextID:T_OPVAP_CASHDEPOT")

#define	T_OPVAP_JUSTCASH	_T("APTextID:T_OPVAP_JUSTCASH")			// "Just.Cash"									// [#2445] US Justin 2016.09.29 just.cash
#define	T_OPVAP_PAYDIANT	_T("APTextID:T_OPVAP_PAYDIANT")			// "Paypal CCA"									// [#2445] US Justin 2016.09.29 Paydiant CCA

#define	T_OPVAP_DUALHOSTDCC	_T("APTextID:T_OPVAP_DUALHOSTDCC")		// "DUAL HOST DCC"								// [#2471] US Justin 2017.02.21
#define	T_OPVAP_POPMONEY	_T("APTextID:T_OPVAP_POPMONEY")			// "POPMONEY"									// [#2471] US Justin 2017.02.21
#define	T_OPVAP_PIN4		_T("APTextID:T_OPVAP_PIN4")				// "CASH PICK-UP"								// [#2471] US Justin 2017.02.21 [#2470]

#define	T_OPVAP_GIVEPAY		_T("APTextID:T_OPVAP_GIVEPAY")			// "GivePay"									// [#2535] US Justin 2018.03.05 GivePay Online
#define T_OPVAP_B4U			_T("APTextID:T_OPVAP_B4U")				// "B4U"										// [#RWC6-16] Bitload4U
#define T_OPVAP_TANGOPAY	_T("APTextID:T_OPVAP_TANGOPAY")			// "TangoPay"									// [RWC6-676] SKKim 2024.03.28


// [#2283] Justin "Currency COnversion" => "Withdrawal" (MasterCard Requirement)
/*
//#define	T_MSG_DATA_015		_T("APTextID:T_DATA_015")			// "Currency Conversion"	// [#2150] US Justin 2012.10.09  
REMOVED the following Text from "MultiText.dat"
ID="T_DATA_015"
US_ENG="CURRENCY CONVERSION"
US_SPN="CONVERSION DE MONEDA"
US_FRN="CONVERSION MONETAIRE"
US_CHN="CURRENCY CONVERSION"
US_KOR="CURRENCY CONVERSION"
US_JPN="CURRENCY CONVERSION"
*/
// End of [#2283]

/*------------------------------------------------------------------
	Screen Number 002												
-------------------------------------------------------------------*/
#define	T_APMSG_002001		_T("APTextID:002001")				// "ERROR CODE"

/*------------------------------------------------------------------
	Screen Number 101												
-------------------------------------------------------------------*/
#define	T_APMSG_101001		_T("APTextID:101001")				// "Please insert your card and remove your card..."
#define	T_APMSG_101002		_T("APTextID:101002")				// "Please insert your card and remove quickly"

// CA Only
#define	T_APMSG_101003		_T("APTextID:101003")				// "To proceed your transaction,please remove the inserted card"
#define	T_APMSG_101004		_T("APTextID:101004")				// "Please insert your card and do not remove your carduntil the transaction has finished."
#define	T_APMSG_101005		_T("APTextID:101005")				// "Please insert your card and do not remove it."
#define	T_APMSG_101006		_T("APTextID:101006")				// "To proceed your transaction,please remove the inserted card"

// MX Only
// [#2115] MX KSK 2012.02.07
#define	T_APMSG_101007		_T("APTextID:101007")				// "Please insert your cardTHIS ATM ONLY DISPENSES US DOLLARS"
#define	T_APMSG_101008		_T("APTextID:101008")				// "To proceed your transaction,remove the inserted cardTHIS ATM ONLY DISPENSES US DOLLARS"
#define	T_APMSG_101009		_T("APTextID:101009")				// "Please insert your card and do not remove it.THIS ATM ONLY DISPENSES US DOLLARS"
// end of [#2115]

#define	T_APMSG_101010		_T("APTextID:101010")				// "Insert your card or touch the card on the NFC"	[#2325] NH KSK 2015.01.27
#define	T_APMSG_101011		_T("APTextID:101011")				// "Insert your card and remove it or touch the card on the NFC"	[#2325] NH KSK 2015.01.27
#define	T_APMSG_101012		_T("APTextID:101012")				// "For Cardless transaction Press the Popmoney button"				[#2374] US Justin 2015.10.19
#define	T_APMSG_101013		_T("APTextID:101013")				// "Cardless ATM"													[#2396] US Justin 2016.02.10
#define	T_APMSG_101014		_T("APTextID:101014")				// "For Cardless transaction Press the Cardless button"				[#2396] US Justin 2016.02.17
#define	T_APMSG_101015		_T("APTextID:101015")				// "For Cardless transaction Press the Pin4 button"					[#2405] US Justin 2016.03.18
#define	T_APMSG_101018		_T("APTextID:101018")				// "For Just.Cash transaction Press the Just.Cash button."			[#2445] US Justin
#define	T_APMSG_101019		_T("APTextID:101019")				// "For Paypal CCA transaction Press the Paypal CCA button."		[#2446] US Justin

#define	T_APMSG_101020		_T("APTextID:101020")				// [#2487] AU KSK 2017.06.08
#define	T_APMSG_101021		_T("APTextID:101021")				// [#2487] AU KSK 2017.06.08
#define	T_APMSG_101022		_T("APTextID:101022")				// [#2487] AU KSK 2017.06.08
#define	T_APMSG_101023		_T("APTextID:101023")				// [#2487] AU KSK 2017.06.08

#define	T_APMSG_101024		_T("APTextID:101024")				// "Withdraw cash using BTC"	// [#RWC6-16]

#define T_APMSG_101025		_T("APTextID:101025")				// [#RWC6-59] US William 2019.10.09 LibertyX

#define T_APMSG_101026		_T("APTextID:101026")				// [#RWC6-399] US ryan.payton 2022.07.27 DigitalMint
#define T_APMSG_101027		_T("APTextID:101027")				// [#RWC6-502] US ryan.payton 2023.01.12 Just.Cash
#define T_APMSG_101028		_T("APTextID:101028")				// [#RWC6-502] US ryan.payton 2023.01.12 Just.Cash

#define T_APMSG_101030		_T("APTextID:101030")				// [RWC6-676] SKKim 2024.04.19 TangoNet

/*------------------------------------------------------------------
	Screen Number 102												
-------------------------------------------------------------------*/
#define	T_APMSG_102001		_T("APTextID:102001")				// "Language Selection"
#define	T_APMSG_102002		_T("APTextID:102002")				// "Please select the language."

/*------------------------------------------------------------------
	Screen Number 103												
-------------------------------------------------------------------*/
#define	T_APMSG_103001		_T("APTextID:103001")				// "Password Input"
#define	T_APMSG_103002		_T("APTextID:103002")				// "Enter your Personal Identification Number"
#define	T_APMSG_103003		_T("APTextID:103003")				// "Please use PIN pad to protect your information"
#define	T_APMSG_103004		_T("APTextID:103004")				// "PIN too short. Please re-enter"
#define	T_APMSG_103005		_T("APTextID:103005")				// "PIN too short or PIN cleared."
#define	T_APMSG_103006		_T("APTextID:103006")				// "Please make sure that there are no unauthorizedthings such as a camera around the PIN pad"
#define	T_APMSG_103007		_T("APTextID:103007")				// "PIN Change"
#define	T_APMSG_103008		_T("APTextID:103008")				// "Enter your new Personal Identification Number.Then press 'ENTER'"
#define	T_APMSG_103009		_T("APTextID:103009")				// "Enter your new Personal Identification Number again.Then press 'ENTER'"
#define	T_APMSG_103010		_T("APTextID:103010")				// "Re-enter your Personal Identification Number"	// [#RWC6-85] Dual Balance Screen Changes - PAI

/*------------------------------------------------------------------
	Screen Number 104												
-------------------------------------------------------------------*/
#define	T_APMSG_104001		_T("APTextID:104001")				// "Balance"
#define	T_APMSG_104002		_T("APTextID:104002")				// "Would you like to check your balance ?"

/*------------------------------------------------------------------
	Screen Number 105												
-------------------------------------------------------------------*/
#define	T_APMSG_105001		_T("APTextID:105001")				// "Transaction"
#define	T_APMSG_105002		_T("APTextID:105002")				// "What would you like to do?"
#define	T_APMSG_105003		_T("APTextID:105003")				// "PIN CHANGE"
#define	T_APMSG_105004		_T("APTextID:105004")				// "Purchase Cryptocurrency"		// [#2496] US Justin	[#2503][#2510]
#define	T_APMSG_105005		_T("APTextID:105005")				// "GivePay\nGift Card"				// [#2530] US Justin

/*------------------------------------------------------------------
	Screen Number 106												
-------------------------------------------------------------------*/
#define	T_APMSG_106002		_T("APTextID:106002")				// "Would you like to receive a receipt?"

//[#2219] 2013.09.04 Justin 2013.04.25 Digital Receipt
#define	T_APMSG_106003		_T("APTextID:106003")				// "Paper\nReceipt"
#define	T_APMSG_106004		_T("APTextID:106004")				// "Digital\nReceipt"
#define	T_APMSG_106005		_T("APTextID:106005")				// "No receipt"

// [#2518] US Kook 2018.01.24 Support MX-2800SE
#define	T_APMSG_106103		_T("APTextID:106103")				// "Paper Receipt"
#define	T_APMSG_106104		_T("APTextID:106104")				// "Digital Receipt"
#define	T_APMSG_106201		_T("APTextID:106201")				// "It's easy being GREENYou can save paper to save tree and save the earth."
#define	T_APMSG_106202		_T("APTextID:106202")				// "You can reduce 0.49g carbonfootprint at this time."
#define	T_APMSG_106203		_T("APTextID:106203")				// ""
#define	T_APMSG_106301		_T("APTextID:106301")				// "Introducing the HyosungMoniPlus DigitalReceipt.View your secure transaction onyour mobile device by scanningthe QR code"
#define	T_APMSG_106302		_T("APTextID:106302")				// "Download the app from your iPhone orAndroid device."
#define	T_APMSG_106303		_T("APTextID:106303")				// "Search : MoniPlus DigitalReceipt"
// end of [#2518]

// End of [#2219]

/*------------------------------------------------------------------
	Screen Number 107												
-------------------------------------------------------------------*/
#define	T_APMSG_107001		_T("APTextID:107001")				// "Receipt Not Available"
#define	T_APMSG_107002		_T("APTextID:107002")				// "Would you like to continue transaction?"
#define	T_APMSG_107003		_T("APTextID:107003")				// "No Receipt"
#define	T_APMSG_107004		_T("APTextID:107004")				// "Would you like to continue transaction?"
#define	T_APMSG_107101		_T("APTextID:107101")				// "Another Transaction"
#define	T_APMSG_107102		_T("APTextID:107102")				// "Would you like to perform another transaction?"
#define	T_APMSG_107201		_T("APTextID:107201")				// "Reversal Transaction"
#define	T_APMSG_107202		_T("APTextID:107202")				// "Would you like to stop reversal transaction?"
#define	T_APMSG_107301		_T("APTextID:107301")				// "Coupon"
#define	T_APMSG_107302		_T("APTextID:107302")				// "Do you want to receive coupon?"		

//#define	T_APMSG_107401		_T("APTextID:107401")				// "Digital Receipt"			[#2219] 2013.09.04 NH KSK 2013.05.09
//#define	T_APMSG_107402		_T("APTextID:107402")				// "No Receipt"					[#2219] 2013.09.04 NH KSK 2013.05.09

/*------------------------------------------------------------------
	Screen Number 108												
-------------------------------------------------------------------*/
#define	T_APMSG_108001		_T("APTextID:108001")				// "Timeout"
#define	T_APMSG_108002		_T("APTextID:108002")				// "Do you need more time?"
#define	T_APMSG_108003		_T("APTextID:108003")				// "Balance Inquiry"
#define	T_APMSG_108004		_T("APTextID:108004")				// "Would you like to check your balance ?"
#define	T_APMSG_108005		_T("APTextID:108005")				// "Receipt Selection"						[#2309] US Justin Dual Balance Receipt - Title 
#define	T_APMSG_108006		_T("APTextID:108006")				// "Would you like to receive a receipt?"	[#2309] US Justin Dual Balance Receipt - Sub Title

/*------------------------------------------------------------------
	Screen Number 109												
-------------------------------------------------------------------*/
#define	T_APMSG_109001		_T("APTextID:109001")				// "FEE Notice"
#define	T_APMSG_109002		_T("APTextID:109002")				// "Do you wish to continue ?"

#define	T_APMSG_109101		_T("APTextID:109101")				// "FEE NOTICE:The owner of this terminal"
#define	T_APMSG_109102		_T("APTextID:109102")				// "charges a fee of..."
//#define	T_APMSG_109103		_T("APTextID:109103")			// "or"											// [#2354] US Justin Not used anymore
//#define	T_APMSG_109104		_T("APTextID:109104")			// "of the transaction amount, whichever is"	// [#2354] US Justin Not used anymore
//#define	T_APMSG_109105		_T("APTextID:109105")			// "LESSER"										// [#2354] US Justin Not used anymore
//#define	T_APMSG_109106		_T("APTextID:109106")			// "GREATER"									// [#2354] US Justin Not used anymore	
#define	T_APMSG_109107		_T("APTextID:109107")				// "for CASH WITHDRAWALS."
#define	T_APMSG_109108		_T("APTextID:109108")				// "This fee is in addition to any fee"
#define	T_APMSG_109109		_T("APTextID:109109")				// "for Balance Inquiry."						// [#2150] US Justin 2012.09.28 Add Dynamic Flow
#define	T_APMSG_109110		_T("APTextID:109110")				// "Admin fee is charged on foreign or..."		// [#RWC6-548] US ryan.payton 2023.05.23 CashDepot Improvements from phase 1 testing
#define	T_APMSG_109111		_T("APTextID:109111")				// "for PIN Change."							// [#2150] US Justin 2012.09.28 Add Dynamic Flow
#define	T_APMSG_109112		_T("APTextID:109112")				// "Additional Admin Fee"						// [#RWC6-548] US ryan.payton 2023.05.23 CashDepot Improvements from phase 1 testing
//#define	T_APMSG_109110		_T("APTextID:109110")				// "for Currency Conversion."					// [#2150] US Justin 2012.09.28 Add Dynamic Flow	// [#2204] NH Justin 2013.06.26 No changes on surcharge notice
//#define	T_APMSG_109112		_T("APTextID:109112")				// "charges a service fee of..."				// [#2192] US Justin 2013.05.02 DCC Text...			// [#2204] NH Justin 2013.06.26 No changes on surcharge notice

// [#2071] AU KSK 2011.06.17
/*------------------------------------------------------------------
Screen Number 110												
-------------------------------------------------------------------*/
#define	T_APMSG_110001		_T("APTextID:110001")				// "Fee Notice"
#define	T_APMSG_110002		_T("APTextID:110002")				// "Do you want to continue ?"
#define	T_APMSG_110101		_T("APTextID:110101")				// "USE OF THIS ATM INCURSA CONVENIENCE FEE OF"
#define	T_APMSG_110102		_T("APTextID:110102")				// "IN ADDITION YOUR BANK MAY ALSOCHARGE YOU A FEE"
// end of [#2071]

/*------------------------------------------------------------------
	Screen Number 111												
-------------------------------------------------------------------*/
#define	T_APMSG_111001		_T("APTextID:111001")				// "Account"
#define	T_APMSG_111101		_T("APTextID:111101")				// "Which account would you like to withdrwal from?"
#define	T_APMSG_111201		_T("APTextID:111201")				// "Which account would you like to see details for?"
#define	T_APMSG_111301		_T("APTextID:111301")				// "Which account would you like to donate from?"
// [#2186] US KMK 2013.04.15 Asian Language Support, 미사용 부분 주석처리 (DAT에 해당 내용 없음)
// #define	T_APMSG_111301		_T("APTextID:111301")				// "Which account would you like to transfer from?"
// #define	T_APMSG_111302		_T("APTextID:111302")				// "Which account would you like to transfer to?"

/*------------------------------------------------------------------
	Screen Number 112												
-------------------------------------------------------------------*/
#define	T_APMSG_112001		_T("APTextID:112001")				// "Amount Selection"
#define	T_APMSG_112002		_T("APTextID:112002")				// "Please tell me the amount"
#define	T_APMSG_112003		_T("APTextID:112003")				// "IMPORTANT:THIS ATMONLY DISPENSESUS DOLLARS"
#define	T_APMSG_112004		_T("APTextID:112004")				// "Other"				// [#RWC6-12]
#define	T_APMSG_112005		_T("APTextID:112005")				// "Other/Choose Bills" // [#RWC6-12]

/*------------------------------------------------------------------
	Screen Number 113												
-------------------------------------------------------------------*/
#define	T_APMSG_113001		_T("APTextID:113001")				// "Amount Input"

#define	T_APMSG_113101		_T("APTextID:113101")				// "Enter the amount you would like to withdraw"
#define	T_APMSG_113102		_T("APTextID:113102")				// "Multiple of amount"
#define	T_APMSG_113103		_T("APTextID:113103")				// "Max amount:"
#define	T_APMSG_113104		_T("APTextID:113104")				// "Over max amount. retry"
#define	T_APMSG_113105		_T("APTextID:113105")				// "Too low amount. retry"
#define	T_APMSG_113106		_T("APTextID:113106")				// "Indispensable amount. retry"
#define	T_APMSG_113107		_T("APTextID:113107")				// "Unable to dispense cash. retry"
#define	T_APMSG_113108		_T("APTextID:113108")				// "Over max amount. cancelled"
#define	T_APMSG_113109		_T("APTextID:113109")				// "Too low amount. cancelled"
#define	T_APMSG_113110		_T("APTextID:113110")				// "Indispensable amount. cancelled"
#define	T_APMSG_113111		_T("APTextID:113111")				// "Unable to dispense cash. cancelled"
#define	T_APMSG_113112		_T("APTextID:113112")				// "Transaction has been cancelled."
#define T_APMSG_113113		_T("APTextID:113113")				// "The amount you have entered is invalid.Please try again!"
#define T_APMSG_113114		_T("APTextID:113114")				// "The ATM cannot process this order at this time.Please locate another ATM to complete yourtransaction or try again later"	// [#2565] US Justin 2018.07.17 Changing Guidance of PIN4 Withdrawal Amount
#define	T_APMSG_113115		_T("APTextID:113115")				// "The minimum for B4U service is $XXXX.retry"	// [#RWC6-16]

#define	T_APMSG_113201		_T("APTextID:113201")				// "Enter the amount you would like to transfer."

#define	T_APMSG_113300		_T("APTextID:113300")				// "Customer Number Input."
#define	T_APMSG_113301		_T("APTextID:113301")				// "Enter the Customer Number."
#define	T_APMSG_113302		_T("APTextID:113302")				// "Must enter at least 6 digits."

#define	T_APMSG_113400		_T("APTextID:113400")				// "Password Input"
#define	T_APMSG_113401		_T("APTextID:113401")				// "Enter the Password."
#define	T_APMSG_113402		_T("APTextID:113402")				// "Must enter at least 3 digits."

#define	T_APMSG_113600		_T("APTextID:113600")				// "IMPORTANT:THIS ATMONLY DISPENSESUS DOLLARS"	// [#2105] MX KSK 2012.01.31

#define	T_APMSG_113700		_T("APTextID:113700")				// "Enter the amount you would like to purchase Digital Currency"	// [#2496]

/*------------------------------------------------------------------
	Screen Number 114												
-------------------------------------------------------------------*/
#define	T_APMSG_114001		_T("APTextID:114001")				// ""
#define	T_APMSG_114002		_T("APTextID:114002")				// "Which account would you like to transfer from?"

/*------------------------------------------------------------------
	Screen Number 115												
-------------------------------------------------------------------*/
#define	T_APMSG_115001		_T("APTextID:115001")				// "Account Selection"
#define	T_APMSG_115002		_T("APTextID:115002")				// "Which account would you like to transfer to?"

/*------------------------------------------------------------------
	Screen Number 116												
-------------------------------------------------------------------*/
#define	T_APMSG_116001		_T("APTextID:116001")				// "Connection to the host"
#define	T_APMSG_116002		_T("APTextID:116002")				// "Please wait"
#define	T_APMSG_116101		_T("APTextID:116101")				// "Configuration"
#define	T_APMSG_116102		_T("APTextID:116102")				// "Health Check"
#define	T_APMSG_116103		_T("APTextID:116103")				// "Detail health Check"
#define	T_APMSG_116104		_T("APTextID:116104")				// "Transaction"
#define	T_APMSG_116105		_T("APTextID:116105")				// "Reversal"
#define	T_APMSG_116106		_T("APTextID:116106")				// "Day total"
#define	T_APMSG_116107		_T("APTextID:116107")				// "Trial day total"
#define	T_APMSG_116108		_T("APTextID:116108")				// "Extended Configuration"
#define	T_APMSG_116109		_T("APTextID:116109")				// "Remote Key Transfer"	[#2075] NH KSK 2011.06.27
#define	T_APMSG_116110		_T("APTextID:116110")				// "GivePay"				[#2535] US Justin 2018.04.16

/*------------------------------------------------------------------
	Screen Number 117												
-------------------------------------------------------------------*/
#define	T_APMSG_117001		_T("APTextID:117001")				// "I'm dispensing your money. please wait!"
#define	T_APMSG_117002		_T("APTextID:117002")				// "Please wait!"

/*------------------------------------------------------------------
	Screen Number 118												
-------------------------------------------------------------------*/
#define	T_APMSG_118001		_T("APTextID:118001")				// "Please take your money"
#define	T_APMSG_118002		_T("APTextID:118002")				// "Partial dispensed. Please take your money"

/*------------------------------------------------------------------
	Screen Number 119
-------------------------------------------------------------------*/
#define	T_APMSG_119001		_T("APTextID:119001")				// "Printing your transaction record"
#define	T_APMSG_119002		_T("APTextID:119002")				// "IMPORTANT: Please take your receipt and paper cryptocurrency wallet."	// [#RWC6-16] JustCash screen change and receipt change
#define	T_APMSG_119003		_T("APTextID:119003")				// "IMPORTANT: Please take your receipt."									// [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash
#define	T_APMSG_119201		_T("APTextID:119201")				// "RECEIPT ERROR"

/*------------------------------------------------------------------
	Screen Number 120												
-------------------------------------------------------------------*/
#define	T_APMSG_120001		_T("APTextID:120001")				// "Please take your receipt"

/*------------------------------------------------------------------
	Screen Number 122
-------------------------------------------------------------------*/
#define	T_APMSG_122001		_T("APTextID:122001")				// "Your transaction is cancelled."
#define	T_APMSG_122002		_T("APTextID:122002")				// "Time is over!"

#define	T_APMSG_122101		_T("APTextID:122101")				// "I can't recognize that card."
#define	T_APMSG_122102		_T("APTextID:122102")				// "The PAN data fails a Modulus-10 check"
#define	T_APMSG_122103		_T("APTextID:122103")				// "This card cannot be used in this network."

#define	T_APMSG_122201		_T("APTextID:122201")				// "ATM error during transaction"
#define	T_APMSG_122202		_T("APTextID:122202")				// "Failure of Point Redemption"											// [#2541] 2018.03.19 HJAHN ALPHI 거래 실패 시, 팝업 문구 변경

#define	T_APMSG_122301		_T("APTextID:122301")				// "Failed to select the AID."
#define	T_APMSG_122302		_T("APTextID:122302")				// "Fail to continue IC Transaction"										// [#2391] US Justin 2016.01.26 Support Multi Transaction for EMV

#define	T_APMSG_122401		_T("APTextID:122401")				// "PIN Same as original"
#define	T_APMSG_122402		_T("APTextID:122402")				// "Two new pin is different"
#define	T_APMSG_122403		_T("APTextID:122403")				// "The transaction could not be completed due to problems with the card."	// [#2554] NH Justin 2018.06.05 Change Fallback Disabled Error Message

#define	T_APMSG_122501		_T("APTextID:122501")				// "Card removed during transaction."  										// [#2544] AU HJAHN 2018.04.13 
#define	T_APMSG_122502		_T("APTextID:122502")				// "Please try again."														// [#2544] AU HJAHN 2018.04.13

#define	T_APMSG_122600		_T("APTextID:122600")				// "TOS sent to the phone number on file."									// [#RWC6-399] US ryan.payton 2022.08.30 DigitalMint

/*------------------------------------------------------------------
	Screen Number 123
-------------------------------------------------------------------*/
#define T_APMSG_123001		_T("APTextID:123001")				// "Thank you for using our service"
#define T_APMSG_123002		_T("APTextID:123002")				// "Thank you"									// [#2288] NH Justin 2014.08.15 Loan Disbursement
#define T_APMSG_123003		_T("APTextID:123003")				// "Thank you for choosing DigitalMint"			// [#RWC6-399] US ryan.payton 2022.08.11 DigitalMint
#define T_APMSG_123004		_T("APTextID:123004")				// "Your Paper Wallet has been fully funded!"	// [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash

/*------------------------------------------------------------------
Screen Number 124
-------------------------------------------------------------------*/
#define	T_APMSG_124001		_T("APTextID:124001")				// ""

/*------------------------------------------------------------------
Screen Number 125
-------------------------------------------------------------------*/
#define	T_APMSG_125001		_T("APTextID:125001")				// ""
#define	T_APMSG_125002		_T("APTextID:125002")				// ""// [#2164] MX Justin 2012.11.12 Change Mexico Mode Screen Text [MX ONLY]

/*------------------------------------------------------------------
Screen Number 126
-------------------------------------------------------------------*/
#define	T_APMSG_126001		_T("APTextID:126001")				// "AID Selection"
#define	T_APMSG_126002		_T("APTextID:126002")				// "Select application ID"

/*------------------------------------------------------------------
Screen Number 127
-------------------------------------------------------------------*/
#define	T_APMSG_127001		_T("APTextID:127001")				// "Remove your card"
#define	T_APMSG_127002		_T("APTextID:127002")				// "To proceed your transaction"
#define	T_APMSG_127003		_T("APTextID:127003")				// "CHIP CARD ERROR"
#define	T_APMSG_127004		_T("APTextID:127004")				// "Transaction is approved."
#define	T_APMSG_127005		_T("APTextID:127005")				// "Transaction ERROR."		// [#2215] 
#define	T_APMSG_127006		_T("APTextID:127006")				// "Transaction is declined."	// [#2188] 2013.09.10
#define	T_APMSG_127007		_T("APTextID:127007")				// "Transaction is completed."	// [#2391] 2016.01.26

/*------------------------------------------------------------------
Screen Number 128
-------------------------------------------------------------------*/
#define	T_APMSG_128001		_T("APTextID:128001")				// ""

// [#2115] MX KSK 2012.02.01
/*------------------------------------------------------------------
Screen Number 129
-------------------------------------------------------------------*/
#define	T_APMSG_129001		_T("APTextID:129001")				// "Fee notice"
#define	T_APMSG_129002		_T("APTextID:129002")				// "DO YOU AGREE?"
#define	T_APMSG_129003		_T("APTextID:129003")				// "FOR PERFORMING THIS TRANSACTION"
#define	T_APMSG_129004		_T("APTextID:129004")				// "Bansi, S.A." - Cardtronics만 사용
#define	T_APMSG_129005		_T("APTextID:129005")				// "WILL CHARGE YOU $X.XX PESOS TAX INCLUDED"
#define	T_APMSG_129006		_T("APTextID:129006")				// "The Exchange Rate is: ### pesos for 1 dollar"

/*------------------------------------------------------------------
Screen Number 130
-------------------------------------------------------------------*/
#define	T_APMSG_130001		_T("APTextID:130001")				// "Fee notice"
#define	T_APMSG_130002		_T("APTextID:130002")				// "DO YOU AGREE?"
#define	T_APMSG_130003		_T("APTextID:130003")				// "FOR PERFORMING THIS TRANSACTION"
#define	T_APMSG_130004		_T("APTextID:130004")				// "WILL CHARGE YOU $XXX PESOS TAX INCLUDED"
#define	T_APMSG_130005		_T("APTextID:130005")				// "FOR ACCESS TO YOUR LINE OF CREDIT"
#define	T_APMSG_130006		_T("APTextID:130006")				// "ADDITIONALLY, YOUR BANK"
#define	T_APMSG_130007		_T("APTextID:130007")				// "AND TO ACCESS YOUR LINE OF CREDIT"

/*------------------------------------------------------------------
Screen Number 131
-------------------------------------------------------------------*/
#define	T_APMSG_131001		_T("APTextID:131001")				// "Transaction Summary"
#define	T_APMSG_131002		_T("APTextID:131002")				// "Do you wish to continue?"
#define	T_APMSG_131003		_T("APTextID:131003")				// "Amount Requested(US)"
#define	T_APMSG_131004		_T("APTextID:131004")				// "Equivalent to (Pesos)"
#define	T_APMSG_131005		_T("APTextID:131005")				// "Exchange Rate(Pesos)"
// end of [#2115]

// [#2138] MX KMK 2012.07.12 
/*------------------------------------------------------------------
Screen Number 132
-------------------------------------------------------------------*/
#define	T_APMSG_132001		_T("APTextID:132001")				// "Exchange Fee Notice"
#define	T_APMSG_132002		_T("APTextID:132002")				// "AMOUNT REQUESTED    US  $#XXX.XX#"
#define	T_APMSG_132003		_T("APTextID:132003")				// "#SURCHARGE_OWNER#  will access an US Dollar"
#define	T_APMSG_132004		_T("APTextID:132004")				// "Exchange Fee of   #XX.XX#%   of the Requested of"
#define	T_APMSG_132005		_T("APTextID:132005")				// "withdrawal. This is in addition to any other fees"
#define	T_APMSG_132006		_T("APTextID:132006")				// "associated with this transaction."
#define	T_APMSG_132007		_T("APTextID:132007")				// "DO YOU WISH TO CONTINUE?"
// end of [#2138]

// [#2137] MX PCS 2012.07.24
/*------------------------------------------------------------------
Screen Number 133
-------------------------------------------------------------------*/
#define	T_APMSG_133001		_T("APTextID:133001")				// "Fee notice"
#define	T_APMSG_133002		_T("APTextID:133002")				// "DO YOU AGREE?"
#define	T_APMSG_133003		_T("APTextID:133003")				// "FOR PERFORMING THIS TRANSACTION"
#define	T_APMSG_133004		_T("APTextID:133004")				// "OWNER"
#define	T_APMSG_133005		_T("APTextID:133005")				// "WILL CHARGE YOU $X.XX TAX INCLUDED."	//[#2164] MX Justin 2012.11.08 Change Screen Text
//#define	T_APMSG_133006		_T("APTextID:133006")				// "TAX INCLUDED"

// [#2149] MX Justin 2012.09.21
/*------------------------------------------------------------------
Screen Number 134
-------------------------------------------------------------------*/
#define	T_APMSG_134001		_T("APTextID:134001")				// "Make a selection"
#define	T_APMSG_134002		_T("APTextID:134002")				// "Are you sure you want to continue with this transaction?"
// End of [#2149]

// [#2150] US Justin 2012.10.01 Dynamic Currency Withdrawal Option
/*------------------------------------------------------------------
Screen Number 135
-------------------------------------------------------------------*/
#define T_APMSG_135001		_T("APTextID:135001")				// "You can choose to be charged for this withdrawal in local currency or directly in your home currency"
#define T_APMSG_135002		_T("APTextID:135002")				// "Would you like to have this withdrawal converted to your home currency at the rate shown?"
#define T_APMSG_135003		_T("APTextID:135003")				// "Cash Withdrawal"
#define T_APMSG_135004		_T("APTextID:135004")				// "Access Fee"
#define T_APMSG_135005		_T("APTextID:135005")				// "Total"
#define T_APMSG_135006		_T("APTextID:135006")				// "Exchange Rate"
#define T_APMSG_135007		_T("APTextID:135007")				// "Account Charge"
#define T_APMSG_135008		_T("APTextID:135008")				// "Total Transaction Amount"
#define T_APMSG_135009		_T("APTextID:135009")				// "Exchange Rate Mark-Up" // [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message (also for STD1)
// End of [#2150]
// [#2180] NH Justin 2013.02.06 Split Dual Host
#define T_APMSG_135011		_T("APTextID:135011")				// "XXXXX%"
#define T_APMSG_135013		_T("APTextID:135013")				// "Decline Offer"
#define T_APMSG_135014		_T("APTextID:135014")				// "Accept Offer"
#define T_APMSG_135015		_T("APTextID:135015")				// "Accept without Conversion"
#define T_APMSG_135016		_T("APTextID:135016")				// "Accept with Conversion"
// End of [#2180]
#define T_APMSG_135022		_T("APTextID:135022")				// "Make sure you understand the costs of currency conversionas they may be different depending on whether you selectyour home currency or the transaction currency" // [#2360]
// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
#define T_APMSG_135033		_T("APTextID:135033")				// Visa DCC Disclaimer - "I have been offered a choice of currencies for withdrawalif accepted, I agree to pay in my home currency."
// End of [#RWC6-2, #2585]
#define T_APMSG_135043		_T("APTextID:135043")				// "Please choose the currency to be charged to your account." // [#GSCJSD-5967] US ryan.payton 2023.02.01 Fixed Cardtronics being out of MC DCC compliance

// [#2496] US Justin 2017.08.16 Add Bit Coin
#define T_APMSG_135101		_T("APTextID:135101")				// "Bitcoin Purchase Summary"
#define T_APMSG_135102		_T("APTextID:135102")				// "Price"
#define T_APMSG_135103		_T("APTextID:135103")				// "Quantity"
#define T_APMSG_135104		_T("APTextID:135104")				// "Whole Unit Price"
#define T_APMSG_135105		_T("APTextID:135105")				// "Please Make a selection"
#define T_APMSG_135106		_T("APTextID:135106")				// "Decline"
#define T_APMSG_135107		_T("APTextID:135107")				// "Approve"
// End of [#2496]

// [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash
#define T_APMSG_135111		_T("APTextID:135111")				// "Bitcoin Pre-Purchase Summary"
#define T_APMSG_135112		_T("APTextID:135112")				// "Minimum Amount"
#define T_APMSG_135113		_T("APTextID:135113")				// "Maximum Amount"
#define T_APMSG_135114		_T("APTextID:135114")				// "Confirm"
#define T_APMSG_135115		_T("APTextID:135115")				// "Insert More Cash" // [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash
// End of [#RWC6-477]
/*--------------------------------------------------------------------
/* Screen Number 135 DCC CUSTOM CASHDEPOT
----------------------------------------------------------------------*/
#define T_APMSG_135802		_T("APTextID:135802")				// "Would you like to have this withdrawal converted to your home currency at the rate shown?"
#define T_APMSG_135803		_T("APTextID:135803")				// "Cash Withdrawal"
#define T_APMSG_135804		_T("APTextID:135804")				// "Access Fee"
#define T_APMSG_135805		_T("APTextID:135805")				// "Total"
#define T_APMSG_135807		_T("APTextID:135807")				// "Account Charge"
#define T_APMSG_135808		_T("APTextID:135808")				// "Total Transaction Amount"
#define T_APMSG_135809		_T("APTextID:135809")				// "Exchange Rate Mark-Up" // [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message (also for STD1)
#define T_APMSG_135813		_T("APTextID:135813")				// "Decline Offer"
#define T_APMSG_135814		_T("APTextID:135814")				// "Accept Offer"
#define T_APMSG_135815		_T("APTextID:135815")				// "Accept without Conversion"
#define T_APMSG_135816		_T("APTextID:135816")				// "Accept with Conversion"
#define T_APMSG_135822		_T("APTextID:135822")				// "Make sure you understand the costs of currency conversionas they may be different depending on whether you selectyour home currency or the transaction currency" // [#2360]
#define T_APMSG_135833		_T("APTextID:135833")				// Visa DCC Disclaimer - "I have been offered a choice of currencies for withdrawalif accepted, I agree to pay in my home currency."
#define T_APMSG_135843		_T("APTextID:135843")				// "Please choose the currency to be charged to your account." // [#GSCJSD-5967] US ryan.payton 2023.02.01 Fixed Cardtronics being out of MC DCC compliance
// End of [RWC6-607]

// [#2159] US Justin 2012.10.19 Display surcharge even surcharge amount is zero .. MEXICO
/*------------------------------------------------------------------
Screen Number 136
-------------------------------------------------------------------*/
#define	T_APMSG_136001		_T("APTextID:136001")				// "Fee notice"
#define	T_APMSG_136002		_T("APTextID:136002")				// "Please note that as a X_OWNER client,"
#define	T_APMSG_136003		_T("APTextID:136003")				// "if you use your X_OWNER debit card at this ATM"
#define	T_APMSG_136004		_T("APTextID:136004")				// "you will not be charged a fee for a cash withdrawal,"
#define	T_APMSG_136005		_T("APTextID:136005")				// "a balance inquiry or a PIN change."
#define	T_APMSG_136006		_T("APTextID:136006")				// "Continue"
// End of [#2159]

// [#2223] NH Justin 2013.10.17 Change Receipt Error with QR Code
/*------------------------------------------------------------------
Screen Number 138
-------------------------------------------------------------------*/
#define	T_APMSG_138001		_T("APTextID:138001")				// "Receipt Selection"
#define	T_APMSG_138002		_T("APTextID:138002")				// "Would you like to continue transaction?"
#define	T_APMSG_138003		_T("APTextID:138003")				// Digital \n Receipt
#define	T_APMSG_138103		_T("APTextID:138103")				// Digital Receipt
#define	T_APMSG_138004		_T("APTextID:138004")				// No Receipt
// End of [#2223]

// [#2266] MX Justin 2014.05.07 Exchange Rate for Domestic Card (USD ATM
/*------------------------------------------------------------------
Screen Number 139
-------------------------------------------------------------------*/
#define	T_APMSG_139001		_T("APTextID:139001")				// "Withdrawal Summary"
#define	T_APMSG_139002		_T("APTextID:139002")				// "Do you agree?"
#define	T_APMSG_139003		_T("APTextID:139003")				// "Amount requested in dollars"
#define	T_APMSG_139004		_T("APTextID:139004")				// "Exchange Rate"
#define	T_APMSG_139005		_T("APTextID:139005")				// "Amount requested in pesos"
// end of [#2266]

// [#2309] US Justin Cardtronics Dual Balance Flow
/*------------------------------------------------------------------
Screen Number 143
-------------------------------------------------------------------*/
// [#2444] US Justin 2016.09.01 Enable Dual Balance to All Customer
#define	T_APMSG_143001		_T("APTextID:143001")				// "Balance Inquiry"
#define	T_APMSG_143002		_T("APTextID:143002")				// "Would you like to get allavailable Account Balances?"
#define	T_APMSG_143003		_T("APTextID:143003")				// "Yes Continue"
#define	T_APMSG_143004		_T("APTextID:143004")				// "Just Checking"
#define	T_APMSG_143005		_T("APTextID:143005")				// "Just Saving"
#define	T_APMSG_143006		_T("APTextID:143006")				// "Main Menu"
#define	T_APMSG_143101		_T("APTextID:143101")				// "Balance Inquiry"
#define	T_APMSG_143102		_T("APTextID:143102")				// "Would you like to check all your balances?"
#define	T_APMSG_143103		_T("APTextID:143103")				// "Yes"
#define	T_APMSG_143104		_T("APTextID:143104")				// "Checking only"
#define	T_APMSG_143105		_T("APTextID:143105")				// "Saving only"
#define	T_APMSG_143106		_T("APTextID:143106")				// "Main Menu"
// End of [#2444]

// [#RWC6-28] PAI Needs to Remove Dual Balance Question and Use Dual Balance Screen Directly When Enable
#define	T_APMSG_143107		_T("APTextID:143107")				// "Credit only"	// [#RWC6-70] PAI Needs Dual Balance for Credit
#define	T_APMSG_143108		_T("APTextID:143108")				// "Yes - All Accounts"		
#define	T_APMSG_143109		_T("APTextID:143109")				// "No - Continue"		
#define T_APMSG_143110		_T("APTextID:143110")				// Withdrawal/Main Menu
// end of [#RWC6-28]

#define	T_APMSG_143010		_T("APTextID:143010")				// "THE OWNER OF THIS ATM DOES NOTCHARGE A FEE FOR A BALANCEINQUIRY, BUT YOUR FINANCIALINSTITUTION MAY CHARGE A FEE."

// [#2444] US Justin 2016.09.01 Enable Dual Balance to All Customer
#define	T_APMSG_146001		_T("APTextID:146001")				// "Receipt Option"
#define	T_APMSG_146002		_T("APTextID:146002")				// "Display Account Balance"
#define	T_APMSG_146003		_T("APTextID:146003")				// "On Screen"
#define	T_APMSG_146004		_T("APTextID:146004")				// "On Receipt"
#define	T_APMSG_146005		_T("APTextID:146005")				// "Main Menu"
#define	T_APMSG_146101		_T("APTextID:146101")				// "Receipt Option "
#define	T_APMSG_146102		_T("APTextID:146102")				// "Would you like to receive a receipt?"
#define	T_APMSG_146103		_T("APTextID:146103")				// "Paper Receipt"
#define	T_APMSG_146104		_T("APTextID:146104")				// "No Receipt"
#define	T_APMSG_146105		_T("APTextID:146105")				// "Main Menu"
// End of [#2444]

// [#2350] US Justin POP Money
/*------------------------------------------------------------------
Screen Number 147
-------------------------------------------------------------------*/
#define	T_APMSG_147001		_T("APTextID:147001")				// PLEASE ENTER YOUR 11 DIGIT ACCESSCODE AND PRESS ENTER
#define	T_APMSG_147101		_T("APTextID:147101")				// Invalid Access Code,Please Re-Enter your Access Code
// End of [#2350]

// [#2396] US Justin Pin4 Transaction
/*------------------------------------------------------------------
Screen Number 153, 154,155,156
-------------------------------------------------------------------*/
#define	T_APMSG_153001		_T("APTextID:153001")				// Transactin Selection
#define	T_APMSG_153002		_T("APTextID:153002")				// Please Choose Your Cardless Option
#define	T_APMSG_153003		_T("APTextID:153003")				// Please Choose Your Bitcoin Option
#define	T_APMSG_153004		_T("APTextID:153004")				// BUY CRYPTO
#define	T_APMSG_153005		_T("APTextID:153005")				// SELL FOR CASH
#define	T_APMSG_154001		_T("APTextID:154001")				// Please Enter your 10-Digit US Mobile Phone Number
#define	T_APMSG_154002		_T("APTextID:154002")				// [XXX-XXX-XXXX]
#define	T_APMSG_154003		_T("APTextID:154003")				// Press ENTER when finished.
#define	T_APMSG_154004		_T("APTextID:154004")				// "International Numbers
#define T_APMSG_154005		_T("APTextID:154005")				// "Please enter a valid phone numberincluding an Area Code."
#define	T_APMSG_155001		_T("APTextID:155001")				// Please Enter your Country Code
#define	T_APMSG_155002		_T("APTextID:155002")				// Press ENTER when finished.
#define	T_APMSG_155101		_T("APTextID:155101")				// Please Enter your Mobile Phone Number
#define	T_APMSG_155201		_T("APTextID:155201")				// Please Enter Valid Country Code
#define	T_APMSG_155202		_T("APTextID:155202")				// Please enter a valid phone number
#define	T_APMSG_155301		_T("APTextID:155301")				// Please Enter last 4 digit of
#define	T_APMSG_156001		_T("APTextID:156001")				// Please Enter your 4-digit Pin4 Code
#define	T_APMSG_156002		_T("APTextID:156002")				// This code was provided to you in the text message from Pin4
#define	T_APMSG_156003		_T("APTextID:156003")				// Press ENTER when finished.
#define	T_APMSG_156101		_T("APTextID:156101")				// Please Enter your secret 4-digit Pin
#define	T_APMSG_156102		_T("APTextID:156102")				// This is the 4 digit PIN provided by the sender
#define	T_APMSG_156201		_T("APTextID:156201")				// Please Enter Valid Number
#define	T_APMSG_156401		_T("APTextID:156401")				// For security purposes please reenter your secret code	// [#2515] US Justin 2017.11.15 PIN4 Prestaging
// End of [#2396]

#define	T_APMSG_158101		_T("APTextID:158101")				// "Close Error"				// [#2428] US Justin 2016.05.25

#define	T_APMSG_159001		_T("APTextID:159001")				// ALLPOINT Surcharge Notice	// [#2437] Supporting Cardtronics ALLPOINT Surcharge Notice

// [#2445] US Justin Just.Cash
/*------------------------------------------------------------------
Screen Number 160
-------------------------------------------------------------------*/
#define	T_APMSG_160001		_T("APTextID:160001")				// CASH CODE
#define	T_APMSG_160002		_T("APTextID:160002")				// Please Enter your Cash Code and Press ENTER.
#define	T_APMSG_160101		_T("APTextID:160101")				// Invalid Cash Code, Please Re-Enter your Cash Code
// End of [#2445]

// [#2449] US Justin Paydiant
/*------------------------------------------------------------------
Screen Number 161
-------------------------------------------------------------------*/
#define	T_APMSG_161001		_T("APTextID:161001")				// Please wait
#define	T_APMSG_161002		_T("APTextID:161002")				// Please proceed the transactio with your phone
#define	T_APMSG_161101		_T("APTextID:161101")				// Please wait while connec to B4U server	// [#RWC6-16] Bitload 4 U. A new cryptocurrency feature for CE 6.0 ATMs
// End of [#2449]

// [#2503] US Justin Ethereum
/*------------------------------------------------------------------
Screen Number 163
-------------------------------------------------------------------*/
#define	T_APMSG_163001		_T("APTextID:163001")				// Make a selection
#define	T_APMSG_163002		_T("APTextID:163002")				// NOTICE: Buying Bitcoin is risky... // [#RWC6-16] JustCash screen change and receipt change
#define	T_APMSG_163003		_T("APTextID:163003")				// Personal Verification \n Entering last 4 digits of one item 		// [#2539]
#define	T_APMSG_163101		_T("APTextID:163101")				// Bitcoin
#define	T_APMSG_163102		_T("APTextID:163102")				// Ethereum
#define	T_APMSG_163103		_T("APTextID:163103")				// SSN 				// [#2539]
#define	T_APMSG_163104		_T("APTextID:163104")				// Phone Number 	// [#2539]
#define	T_APMSG_163105		_T("APTextID:163105")				// Birthday\n(MMYY)	// [#2539]
#define	T_APMSG_163106		_T("APTextID:163106")				// Birthday (MMYY)	// [#2539]
// End of [#2503]

// [#2557] NH Justin 2018.06.12 GivePay Enhancement
#define	T_APMSG_165001		_T("APTextID:165001")				// Give a Gift
#define	T_APMSG_165002		_T("APTextID:165002")				// Pay for Wireless
//#define	T_APMSG_165003		_T("APTextID:165003")				// Terms of Use
#define	T_APMSG_165004		_T("APTextID:165004")				// Would you like to add an Animated Greeting for XXXXX?
#define	T_APMSG_165005		_T("APTextID:165005")				// Personalize your Gift Card with a Video Greeting*
#define	T_APMSG_165006		_T("APTextID:165006")				// $2.00 XXXXX Convenience Delivery Fee		// [#2574]
#define	T_APMSG_165007		_T("APTextID:165007")				// Free Convenience Delivery Fee			// [#2574]

#define	T_APMSG_165901		_T("APTextID:165901")				// Skip
#define	T_APMSG_165902		_T("APTextID:165902")				// Main Menu
#define	T_APMSG_165903		_T("APTextID:165903")				// Back
#define	T_APMSG_165904		_T("APTextID:165904")				// More Choices
#define	T_APMSG_165905		_T("APTextID:165905")				// Authorize Text
#define	T_APMSG_165906		_T("APTextID:165906")				// View Terms
#define	T_APMSG_165907		_T("APTextID:165907")				// Confirm
#define	T_APMSG_165908		_T("APTextID:165908")				// No		// [#2574] US Justin GivePay Enhancement3
#define	T_APMSG_165909		_T("APTextID:165909")				// Yes		// [#2574] US Justin GivePay Enhancement3

#define	T_APMSG_166001		_T("APTextID:166001")				// Please enter amount then press enter
#define	T_APMSG_166002		_T("APTextID:166002")				// Bill Zip Code required to completeyour transaction
#define	T_APMSG_166901		_T("APTextID:166901")				// Invalid Code.Please Re-Enter
#define	T_APMSG_166902		_T("APTextID:166902")				// Invalid Amount.Please try again		// [#2574]

#define	T_APMSG_168001		_T("APTextID:168001")				// Sending From: 
#define	T_APMSG_168002		_T("APTextID:168002")				// Sending To: 
#define	T_APMSG_168003		_T("APTextID:168003")				// Paying to mobile number
#define	T_APMSG_168004		_T("APTextID:168004")				// Includes $XXXXX Convenience Fee 
#define	T_APMSG_168005		_T("APTextID:168005")				// Personal Greeting - 
#define	T_APMSG_168006		_T("APTextID:168006")				// Total Amount Debited to your card  
// End of [#2557]

// [#RWC6-16] Bitload 4 U. A new cryptocurrency feature for CE 6.0 ATMs
/*------------------------------------------------------------------
Screen Number 172
-------------------------------------------------------------------*/
#define	T_APMSG_172001		_T("APTextID:172001")				// How do you want your receipt?
#define	T_APMSG_172002		_T("APTextID:172002")				// Display
#define	T_APMSG_172003		_T("APTextID:172003")				// Print
#define	T_APMSG_172004		_T("APTextID:172004")				// Exchange Cryptocurrency today!
#define	T_APMSG_172005		_T("APTextID:172005")				// Enter the amount of USD below:
#define	T_APMSG_172006		_T("APTextID:172006")				// In Multiples of $XXX
#define	T_APMSG_172007		_T("APTextID:172007")				// up to $XXXX per exchange
#define	T_APMSG_172008		_T("APTextID:172008")				// There is a fee of $XXXX for this transaction
#define	T_APMSG_172009		_T("APTextID:172009")				// Do you accept this fee?
#define	T_APMSG_172010		_T("APTextID:172010")				// I accept, continue
#define	T_APMSG_172011		_T("APTextID:172011")				// Cancel
#define	T_APMSG_172012		_T("APTextID:172012")				// Scan this QR code using your Digital wallet to complete the transaction
#define	T_APMSG_172013		_T("APTextID:172013")				// Transaction ID: XXXX
#define	T_APMSG_172014		_T("APTextID:172014")				// Time to complete
#define	T_APMSG_172015		_T("APTextID:172015")				// Take a picture of the Transaction ID, then go to this website to retrieve a record of your transaction:
#define	T_APMSG_172016		_T("APTextID:172016")				// Check Result
#define	T_APMSG_172017		_T("APTextID:172017")				// Please wait while connecting to B4U
#define	T_APMSG_172018		_T("APTextID:172018")				// Enter the amount of CAD below:
#define	T_APMSG_172101		_T("APTextID:172101")				// Receipt Option
#define	T_APMSG_172102		_T("APTextID:172102")				// Enter Amount
#define	T_APMSG_172103		_T("APTextID:172103")				// Fee Acceptance
#define	T_APMSG_172104		_T("APTextID:172104")				// Scan QR
#define	T_APMSG_172105		_T("APTextID:172105")				// Dispense Money
#define	T_APMSG_172106		_T("APTextID:172106")				// Receipt
#define	T_APMSG_172107		_T("APTextID:172107")				// Bitload4U
#define	T_APMSG_172108		_T("APTextID:172108")				// If you have not received the cash...
#define	T_APMSG_172109		_T("APTextID:172109")				// Printing your Bitload4U transaction report
// end of [#RWC6-16]

// [#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA
/*------------------------------------------------------------------
	Screen Number 180												
-------------------------------------------------------------------*/
#define	T_APMSG_180001		_T("APTextID:180001")				// "Denomination Selection"
//#define	T_APMSG_180002		_T("APTextID:180002")				// "Selected Withdrawal Amount = "
#define	T_APMSG_180002		_T("APTextID:180002")				// "Total = "
#define	T_APMSG_180003		_T("APTextID:180003")				// "Total Accounted = "
#define	T_APMSG_180004		_T("APTextID:180004")				// "Continue"
#define	T_APMSG_180005		_T("APTextID:180005")				// "Exit"
#define	T_APMSG_180006		_T("APTextID:180006")				// "Bill Not Available"	// [#RWC6-12] Denomination Selection Support
#define	T_APMSG_180007		_T("APTextID:180007")				// "Max Amount = "		// [#RWC6-12] Denomination Selection Support
#define	T_APMSG_180008		_T("APTextID:180008")				// "Min Amount = "		// [#RWC6-12] Denomination Selection Support
#define	T_APMSG_180009		_T("APTextID:180009")				// "Entering Amount:"	// [#RWC6-12] Denomination Selection Support
// end of [2584]

// [#2494] AU Kook 2017.08.03 ATS Demo
/*------------------------------------------------------------------
Screen Number 170
-------------------------------------------------------------------*/
#define	T_APMSG_170001		_T("APTextID:170001")				// "Connection to the host"
#define	T_APMSG_170002		_T("APTextID:170002")				// "Please wait"
#define	T_APMSG_170003		_T("APTextID:170003")				// "Enquiry Points"			// not used
#define	T_APMSG_170004		_T("APTextID:170004")				// "Redeem Points"				
#define	T_APMSG_170005		_T("APTextID:170005")				// "Reverse Redemption"

#define	T_APMSG_170101		_T("APTextID:170101")				// "YOU QUALIFY FOR A SPECIAL DISCOUNT. THE ATM TRANSACTION FEE HAS BEEN REDUCED TO $ 0.00."
#define	T_APMSG_170102		_T("APTextID:170102")				// "Your loyalty point has been reverted successfully."


// [#2541] 2018.03.19 HJAHN 
//		1. Change Screen Number ( 151 -> 164 ) 
//		2. ALPHI Detailed error messages
/*------------------------------------------------------------------
Screen Number 164	- Display ALPHI Failure Notice
-------------------------------------------------------------------*/
#define	T_APMSG_164001		_T("APTextID:164001")				// "Loyalty Point Service Notice"
#define	T_APMSG_164002		_T("APTextID:164002")				// "Loyalty Point Service is currently not available."
#define	T_APMSG_164003		_T("APTextID:164003")				// "Insufficient Points Available."

#define	T_APMSG_164101		_T("APTextID:164101")				// "Would you like to continue transaction"
#define	T_APMSG_164102		_T("APTextID:164102")				// "without the service?"
// end of [#2494]

#define	T_APMSG_164206		_T("APTextID:164206")				// "Miscellaneous error"
#define	T_APMSG_164212		_T("APTextID:164212")				// "Declined by points server"
#define	T_APMSG_164214		_T("APTextID:164214")				// "Card number invalid"
#define	T_APMSG_164233		_T("APTextID:164233")				// "Card expired"
#define	T_APMSG_164236		_T("APTextID:164236")				// "Account status invalid"
#define	T_APMSG_164251		_T("APTextID:164251")				// "Insufficient points"
#define	T_APMSG_164291		_T("APTextID:164291")				// "Points server comms fail"
// End of [#2541]

/*------------------------------------------------------------------
	Screen Number 201												
-------------------------------------------------------------------*/
#define T_APMSG_201012		_T("APTextID:201012")				// "FEE NOTICE to INTERAC cardholders:"			// [#] CA PSC 2008.07.04
#define T_APMSG_201013		_T("APTextID:201013")				// "charges a fee of..."						// [#] CA PSC 2008.07.04
#define T_APMSG_201014		_T("APTextID:201014")				// "for CASH WITHDRAWALS."						// [#] CA PSC 2008.07.04
#define	T_APMSG_201102		_T("APTextID:201102")				// "If you continue, you will be charged"							// [#347] AU AIREAT 2008.06.26
#define	T_APMSG_201103		_T("APTextID:201103")				// "For this transaction by the"									// [#347] AU AIREAT 2008.06.26
#define	T_APMSG_201104		_T("APTextID:201104")				// "Your card Issuer may also charge you a fee for using this ATM"	// [#347] AU AIREAT 2008.06.26
#define	T_APMSG_201300		_T("APTextID:201300")				// "<ATM OWNER> WILL CHARGE YOU"
#define	T_APMSG_201301		_T("APTextID:201301")				// "<ATM OWNER> WILL OFFER AN INCENTIVE OF"
#define	T_APMSG_201400		_T("APTextID:201400")				// "YOUR ISSUER WILL CHARGE YOU"
#define	T_APMSG_201401		_T("APTextID:201401")				// "YOUR ISSUER WILL REDUCE THIS BY"
#define	T_APMSG_201500		_T("APTextID:201500")				// "THE NET CHARGE IS"

// [#285] [MX] KSK 2008.6.12
// 권종 추가에 따른 Screen Text Layout 수정
/*------------------------------------------------------------------
	Screen Number 204												
-------------------------------------------------------------------*/
#define	T_APMSG_204001		_T("APTextID:204001")				// "Enter the amount you would like to withdraw Then press ENTER."
#define	T_APMSG_204002		_T("APTextID:204002")				// "MULTIPLE OF AMOUNT IS"
#define	T_APMSG_204003		_T("APTextID:204003")				// "MAX AMOUNT"
#define	T_APMSG_204005		_T("APTextID:204005")				// "Amount up to:"

// [#2224] US Justin 2013.10.17 Generic Verification V2.11
/*------------------------------------------------------------------
Screen Number 316
-------------------------------------------------------------------*/
#define	T_APMSG_316001		_T("APTextID:316001")				// "Continue"
#define	T_APMSG_316002		_T("APTextID:316002")				// "Must enter at least 1 digit."
// End of [#2224]

// [#2494] AU woooz 2017.08.03
#define	T_APMSG_401001		_T("APTextID:401001")				// Please insert and remove your membership card
																// to redeem points in lieu of paying a surcharge.
#define	T_APMSG_401002		_T("APTextID:401002")				// Your Loyalty Card has been successfully read.

#define	T_APMSG_402001		_T("APTextID:402001")				// "Please insert your card and remove your card..."

// [#RWC6-399] US ryan.payton 2022.08.29 DigitalMint
/*------------------------------------------------------------------
Screen Number 370
-------------------------------------------------------------------*/
#define T_APMSG_370000		_T("APTextID:370000")

/*------------------------------------------------------------------
Screen Number 371
-------------------------------------------------------------------*/
#define T_APMSG_371000		_T("APTextID:371000")

/*------------------------------------------------------------------
Screen Number 380
-------------------------------------------------------------------*/
#define T_APMSG_380000		_T("APTextID:380000")

/*------------------------------------------------------------------
Screen Number 381
-------------------------------------------------------------------*/
#define T_APMSG_381000		_T("APTextID:381000")

/*------------------------------------------------------------------
Screen Number 384
-------------------------------------------------------------------*/
#define T_APMSG_384000		_T("APTextID:384000")
#define T_APMSG_384001		_T("APTextID:384001")
#define T_APMSG_384002		_T("APTextID:384002")
#define T_APMSG_384003		_T("APTextID:384003")
#define T_APMSG_384004		_T("APTextID:384004")
#define T_APMSG_384005		_T("APTextID:384005")

/*------------------------------------------------------------------
Screen Number 385
-------------------------------------------------------------------*/
#define T_APMSG_385000		_T("APTextID:385000")
#define T_APMSG_385001		_T("APTextID:385001")
#define T_APMSG_385002		_T("APTextID:385002")
#define T_APMSG_385003		_T("APTextID:385003")
#define T_APMSG_385004		_T("APTextID:385004")
#define T_APMSG_385005		_T("APTextID:385005")
// End of [#RWC6-399]

// [RWC6-411] US william.wallace 2022.09.06 DigitalMint
/*------------------------------------------------------------------
Screen Number 390
-------------------------------------------------------------------*/
#define T_APMSG_390000		_T("APTextID:390000")
#define T_APMSG_390001		_T("APTextID:390001")
#define T_APMSG_390002		_T("APTextID:390002")
#define T_APMSG_390003		_T("APTextID:390003")
// End of [RWC6-411]

// [#RWC6-506] US ryan.payton 2023.04.26 Cash Depot
/*------------------------------------------------------------------
Screen Number 399
-------------------------------------------------------------------*/
#define T_APMSG_399001		_T("APTextID:399001")
#define T_APMSG_399002		_T("APTextID:399002")
#define T_APMSG_399003		_T("APTextID:399003")

/*------------------------------------------------------------------
Screen Number 402
-------------------------------------------------------------------*/
#define T_APMSG_402000		_T("APTextID:402000")

/*------------------------------------------------------------------
Screen Number 404
-------------------------------------------------------------------*/
#define T_APMSG_404000		_T("APTextID:404000")

/*------------------------------------------------------------------
Screen Number 408
-------------------------------------------------------------------*/
#define T_APMSG_408000		_T("APTextID:408000")
// End of [#RWC6-506]

// [RWC6-676] Start SKKim 2024.04.16
/*------------------------------------------------------------------
Screen Number 410
-------------------------------------------------------------------*/
#define	T_APMSG_410201		_T("APTextID:410201")				// Please enter a valid phone number
#define	T_APMSG_410301		_T("APTextID:410301")				// Retry exceeded.

/*------------------------------------------------------------------
Screen Number 411
-------------------------------------------------------------------*/
#define	T_APMSG_411201		_T("APTextID:411201")				// Please enter a valid TangoPay code

/*------------------------------------------------------------------
Screen Number 412
-------------------------------------------------------------------*/

/*------------------------------------------------------------------
Screen Number 413
-------------------------------------------------------------------*/
#define	T_APMSG_413001		_T("APTextID:413001")				// MONEY TRANSFER FRAUD WARINING
#define	T_APMSG_413002		_T("APTextID:413002")				// PRE-PAYMENT DISCLOSURE
#define	T_APMSG_413201		_T("APTextID:413201")				// Printing fraud warning messages
#define	T_APMSG_413202		_T("APTextID:413202")				// Printing pre-payment discloure

/*------------------------------------------------------------------
Screen Number 414
-------------------------------------------------------------------*/
#define	T_APMSG_414001		_T("APTextID:414001")				// Total amount to deposit : XXX
#define	T_APMSG_414002		_T("APTextID:414002")				// Current deposited amount : XXX
#define	T_APMSG_414201		_T("APTextID:414201")				// The deposited amount has been exceeded.Insert the  amount exactly.
#define	T_APMSG_414202		_T("APTextID:414202")				// Please re-insert the bill correctly.

#define	T_APMSG_414301		_T("APTextID:414301")				// Device error

/*------------------------------------------------------------------
Screen Number 416
-------------------------------------------------------------------*/
#define	T_APMSG_416002		_T("APTextID:416002")				// Send to
#define	T_APMSG_416003		_T("APTextID:416003")				// Total Received
#define	T_APMSG_416004		_T("APTextID:416004")				// Send Fees
#define	T_APMSG_416005		_T("APTextID:416005")				// Send Taxes
#define	T_APMSG_416006		_T("APTextID:416006")				// Send Amount
#define	T_APMSG_416007		_T("APTextID:416007")				// Receive Method
#define	T_APMSG_416008		_T("APTextID:416008")				// Total to Receiver


// [RWC6-676] End SKKim 2024.04.16

// [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash
/*------------------------------------------------------------------
Screen Number 501
-------------------------------------------------------------------*/
#define T_APMSG_501000		_T("APTextID:501000")

/*------------------------------------------------------------------
Screen Number 507
-------------------------------------------------------------------*/
#define T_APMSG_507000		_T("APTextID:507000")
#define T_APMSG_507001		_T("APTextID:507001")

/*------------------------------------------------------------------
Screen Number 508
-------------------------------------------------------------------*/
#define T_APMSG_508000		_T("APTextID:508000")
// End of [#RWC6-477]

/*------------------------------------------------------------------
	OP SCREEN TEXT ID
------------------------------------------------------------------*/

/*------------------------------------------------------------------
	OP Common TextID
-------------------------------------------------------------------*/
#define T_INVALID_PIN			_T("APTextID:9999001")				// "INVALID PIN"
#define T_INVALID_DATA			_T("APTextID:9999002")				// "INVALID DATA"
#define T_INVALID_ERROR			_T("APTextID:9999003")				// "INVALID ERROR CODE"
#define T_PROCESS_WAIT			_T("APTextID:9999010")				// "PROCESSING.. WAIT A MOMENT!"
#define T_PROCESS_WAIT_2		_T("APTextID:9999011")				// "PROCESSING.. WAIT A MOMENT!"
#define T_DEVICE_ERR			_T("APTextID:9999020")				// "DEVICE ERROR"
#define T_OPERATE_NG			_T("APTextID:9999030")				// "OPERATION FAILURE"
#define T_OPERATE_OK			_T("APTextID:9999031")				// "OPERATION SUCCESSFUL!"
#define T_OPERATE_OK_SPRNG		_T("APTextID:9999041")				// "OPERATION SUCCESSFUL!(PRINTER ERROR)"
#define T_INSERT_OVER			_T("APTextID:9999050")				// "INSERTED COUNT OVER LIMIT"
#define	T_NVRAMCLEAR_OK			_T("APTextID:9999060")				// "CLEAR ALL.. WAIT UNTIL RESTART"
#define	T_REBOOT_OK				_T("APTextID:9999061")				// "REBOOT.. WAIT UNTIL RESTART"
#define	T_NVRAMCLEARSETTING_OK	_T("APTextID:9999062")				// "CLEAR SETTING.. WAIT UNTIL RESTART"
#define	T_DOORSTATUS_NG			_T("APTextID:9999070")				// "TRY AGAIN. MUST OPEN SAFE DOOR"
#define T_NO_JNL_DATA			_T("APTextID:9999080")				// NO JOURNAL DATA
#define T_NO_USB_SLOT			_T("APTextID:9999090")				// CANNOT FIND USB DRIVE
// [#97] US KGS 2008.04.08 VDM 진입시 Device 종료 안내 문구 표시
#define T_TERMINATING_MCU		_T("APTextID:9999901")			// TERMINATING DevIDC30
#define T_TERMINATING_EPP		_T("APTextID:9999902")			// TERMINATING EPP
#define T_TERMINATING_SIU		_T("APTextID:9999903")			// TERMINATING SIU
#define T_TERMINATING_CDU		_T("APTextID:9999904")			// TERMINATING CDU
#define T_TERMINATING_SPR		_T("APTextID:9999905")			// TERMINATING SPR
// end of [#97]
// [#184] NH KGS 2008.05.06 Device Error 안내 메시지 장치별 세부 분류
#define T_PRINTER_ERROR			_T("APTextID:9999906")			// "PRINTER ERROR"
#define T_CDU_ERROR				_T("APTextID:9999907")			// "CDU ERROR"
// end of [#184]

// [#396] [US] KSK 2008.8.6
#define T_CERTIFICATION_SUCCESS	_T("APTextID:9999908")			// "CERTIFICATION SUCCESS"
#define T_CERTIFICATION_FAIL	_T("APTextID:9999909")			// "CERTIFICATION FAIL"
// end of [#396]

// [#452] [NH] KSK 2008.11.24
#define T_DENOMINATION_ERROR	_T("APTextID:9999910")			// "DENOMINATION ERROR"
#define T_DUPLICATION_ERROR		_T("APTextID:9999911")			// "DUPLICATE ERROR"
// end of [#452]

// [#453] [NH] KSK 2008.11.25
#define T_HEARTBEAT_SENT		_T("APTextID:9999912")			// "HEARTBEAT BEING SENT"

#define T_OVERMAX_DENOMINATION	_T("APTextID:9999913")			// OVER 40 MULTIPLE OF DENOMINATION
#define T_OVERMAX_DISPENSELIMIT	_T("APTextID:9999914")			// OVER DISPENSE LIMIT
#define T_W_OVERMAX_DISPENSELIMIT	_T("APTextID:9999930")		// [#2098] US KSK 2011.11.28

#define	T_DEVICE_INITIALIZING	_T("APTextID:9999926")
//#define T_ERROR_ACT_DESC		_T("APTextID:9999927")			// [#2160] CA KMK 2012.12.04 사용안함
//#define T_C_OVERMAX_DENOMINATION	_T("APTextID:9999928")		// [#2165] NH KSK 2012.11.13 사용안함
//#define T_MPW_RESET_COMPLETE	_T("APTextID:9999929")			// [#2165] NH KSK 2012.11.13 사용안함

#define	T_TOO_LOW				_T("APTextID:9999927")			// [#RWC6-135] PAI: Credit Additional Surcharge Options
#define	T_TOO_HIGH				_T("APTextID:9999928")			// [#RWC6-135] PAI: Credit Additional Surcharge Options

#define T_FAIL_DELETE			_T("APTextID:9999091")				// FAIL TO DELETE FILE [#650] NH KJW 2010.08.30
#define T_FAIL_FIND				_T("APTextID:9999093")				// FAIL TO FIND FILE [#650] NH KJW 2010.08.30

#define T_NO_RESOURCE_DATA		_T("APTextID:9999094")				// FAIL TO FIND RESOURCE [#2013] NH KJW 2011.01.23
#define T_WRONG_EPP				_T("APTextID:9999095")				// NOT 512K EPP Notice   [#2185] US Justin 2013.04.11 DUAL HOST DCC

#define T_FAIL_SET_CURRENCY_ID	_T("APTextID:9999096")				// FAIL TO SET CURRENCY ID
#define T_FAIL_CLEAR_MK		_T("APTextID:9999098")				// FAIL TO CLEAR MASTER KEY
#define T_MISSING_INPUT_VALUE	_T("APTextID:9999099")				// MISSING INPUT VALUE

// [#2160] CA KMK 2012.10.22 FrenchOP, AtmClerkCtrl.cpp의 내용
#define T_OPEN_SPR				_T("APTextID:9999931")				// "OPEN SPR"
#define T_SUCCESS_OPEN_SPR		_T("APTextID:9999932")				// "SUCCESS TO OPEN SPR PORT"
#define T_FAIL_OPEN_SPR			_T("APTextID:9999933")				// "FAIL TO OPEN SPR PORT"
#define T_OPEN_MCU				_T("APTextID:9999934")				// "OPEN MCU"
#define T_SUCCESS_OPEN_MCU		_T("APTextID:9999935")				// "SUCCESS TO OPEN MCU PORT"
#define T_FAIL_OPEN_MCU			_T("APTextID:9999936")				// "FAIL TO OPEN MCU PORT"
#define T_OPEN_CDU				_T("APTextID:9999937")				// "OPEN CDU"
#define T_SUCCESS_OPEN_CDU		_T("APTextID:9999938")				// "SUCCESS TO OPEN CDU PORT"
#define T_FAIL_OPEN_CDU			_T("APTextID:9999939")				// "FAIL TO OPEN CDU PORT"

#define T_INITIALIZING_PIN		_T("APTextID:9999940")				// "INITIALIZING PIN"
#define T_SUCCESS_INIT_PIN		_T("APTextID:9999941")				// "SUCCESS TO INITIALIZE PIN"
#define T_FAIL_INIT_PIN			_T("APTextID:9999942")				// "FAIL TO INITIALIZE PIN"

#define T_FAIL_ENTER_KEYMGR		_T("APTextID:9999943")				// "FAIL TO ENTER KEY MANAGEMENT"

#define T_INITIALIZING_CDU		_T("APTextID:9999944")				// "INITIALIZING CDU"
#define T_CLOSING_CDU			_T("APTextID:9999945")				// "CLOSING CDU PORT"

#define T_FAIL_INIT				_T("APTextID:9999946")				// "FAIL TO INITIALIZE"
#define T_FAIL_READ				_T("APTextID:9999947")				// "FAIL TO READ"
#define T_SUCCESS_READ			_T("APTextID:9999948")				// "SUCCESS TO READ"
#define T_REMOVE_CARD			_T("APTextID:9999949")				// "REMOVE CARD..."
#define T_FAIL_LATCH			_T("APTextID:9999950")				// "FAIL TO LATCH"
#define T_FAIL_POWER_OFF		_T("APTextID:9999951")				// "FAIL TO POWER OFF"
#define T_SUCCESS_POWER_OFF		_T("APTextID:9999952")				// "SUCCESS TO POWER OFF"
#define T_FAIL_UNLATCH			_T("APTextID:9999953")				// "FAIL TO UNLATCH"
#define T_SUCCESS_UNLATCH		_T("APTextID:9999954")				// "SUCCESS TO UNLATCH"
#define T_FAIL_POWER_ON			_T("APTextID:9999955")				// "FAIL TO POWER ON"
#define T_SUCCESS_POWER_ON		_T("APTextID:9999956")				// "SUCCESS TO POWER ON"
#define T_SUCCESS_LATCH			_T("APTextID:9999957")				// "SUCCESS TO LATCH"

#define T_INITIALIZING_DEVICE	_T("APTextID:9999958")				// "INITIALIZING DEVICES"
#define T_CLOSING_MCU			_T("APTextID:9999959")				// "CLOSING MCU PORT"
#define T_CLOSING_SPR			_T("APTextID:9999960")				// "CLOSING SPR PORT"

#define T_CANCEL_AGING_TEST		_T("APTextID:9999961")				// "CANCEL TO AGING TEST"
#define T_FAIL_MODEM_TEST		_T("APTextID:9999962")				// "FAIL TO MODEM TEST"
#define T_SUCCESS_MODEM_TEST	_T("APTextID:9999963")				// "SUCCESS TO MODEM TEST"
#define T_TEST_MODEM			_T("APTextID:9999964")				// "TEST MODEM"
#define T_FAIL_INIT_CDU			_T("APTextID:9999965")				// "FAIL TO INITIALIZE CDU"
#define T_SUCCESS_INIT_CDU		_T("APTextID:9999966")				// "SUCCESS TO INITIALIZE CDU"
#define T_INIT_CDU				_T("APTextID:9999967")				// "INITIALIZE CDU"
#define T_FAIL_INIT_MCU			_T("APTextID:9999968")				// "FAIL TO INITIALIZE MCU"
#define T_SUCCESS_INIT_MCU		_T("APTextID:9999969")				// "SUCCESS TO INITIALIZE MCU"
#define T_INIT_MCU				_T("APTextID:9999970")				// "INITIALIZE MCU"
#define T_FAIL_PRINT			_T("APTextID:9999971")				// "FAIL TO PRINT"
#define T_SUCCESS_PRINT			_T("APTextID:9999972")				// "SUCCESS TO PRINT"
#define T_TEST_PRINTING			_T("APTextID:9999973")				// "TEST PRINTING"
#define T_CANCEL_DISPENSER		_T("APTextID:9999974")				// "CANCEL TO DISPENSER"
#define T_FAIL_DISPENSING		_T("APTextID:9999975")				// "FAIL TO DISPENSING"
#define T_SUCCESS_DISPENSING	_T("APTextID:9999976")				// "SUCCESS TO DISPENSING"
#define T_DISPENSING			_T("APTextID:9999977")				// "DISPENSING"
#define T_CANCEL_PRINTER		_T("APTextID:9999978")				// "CANCEL TO PRINTER"
#define T_CANCEL_INIT			_T("APTextID:9999979")				// "CANCEL TO INITIALIZE"
#define T_INIT_SPR				_T("APTextID:9999981")				// "INITIALIZE SPR"
#define T_FAIL_INIT_SPR			_T("APTextID:9999982")				// "FAIL TO INITIALIZE SPR"
#define T_SUCCESS_INIT_SPR		_T("APTextID:9999983")				// "SUCCESS TO INITIALIZE SPR"

#define T_CONF_CHANGED_REBOOT	_T("APTextID:9999980")				// "ATM will be rebooting (Configuration is changed)"
#define T_CONNECTING			_T("APTextID:9999984")				// "CONNECTING..."
// end of [#2160]

// [#2325] NH KSK 2015.01.25
#define T_OPEN_RFID				_T("APTextID:9999985")				// "OPEN RFID"
#define T_SUCCESS_OPEN_RFID		_T("APTextID:9999986")				// "SUCCESS TO OPEN RFID PORT"
#define T_FAIL_OPEN_RFID		_T("APTextID:9999987")				// "FAIL TO OPEN RFID PORT"
#define T_FAIL_INIT_RFID		_T("APTextID:9999988")				// "FAIL TO INITIALIZE RFID"
#define T_SUCCESS_INIT_RFID		_T("APTextID:9999989")				// "SUCCESS TO INITIALIZE RFID"
#define T_INIT_RFID				_T("APTextID:9999990")				// "INITIALIZE RFID"
#define T_CLOSING_RFID			_T("APTextID:9999991")				// "CLOSING RFID PORT"
#define T_TERMINATING_RFID		_T("APTextID:9999992")				// TERMINATING RFID	[#2325] NH KSK 2015.01.25
// end of [#2325]

#define	T_APMSG_EXIT			_T("APTextID:T_EXIT")				// Exit							// [#2529] NH Justin 2018.02.20 Add 2800 SE

// [#2160] CA KMK 2012.11.15 French OP, AtmClerkCtrl.cpp AddPrintData()부분
#define T_MSG_CLERK_001		_T("APTextID:T_CLERK_001")		// "          VERSION INFORMATION           "
#define T_MSG_CLERK_002		_T("APTextID:T_CLERK_002")		// "TODAY"
#define T_MSG_CLERK_003		_T("APTextID:T_CLERK_003")		// "------------- AP VERSION ---------------"
#define T_MSG_CLERK_004		_T("APTextID:T_CLERK_004")		// "AP"
#define T_MSG_CLERK_005		_T("APTextID:T_CLERK_005")		// "OS"
#define T_MSG_CLERK_006		_T("APTextID:T_CLERK_006")		// "RMS"
#define T_MSG_CLERK_007		_T("APTextID:T_CLERK_007")		// "B-L"
#define T_MSG_CLERK_008		_T("APTextID:T_CLERK_008")		// Change to "MWI"  (OLD:"T-P")
#define T_MSG_CLERK_009		_T("APTextID:T_CLERK_009")		// "------------- VDM VERSION ---------------"
#define T_MSG_CLERK_010		_T("APTextID:T_CLERK_010")		// "CDU         : %-s", strTempArray[5]
#define T_MSG_CLERK_011		_T("APTextID:T_CLERK_011")		// "SPR         : %-s", strTempArray[6]
#define T_MSG_CLERK_012		_T("APTextID:T_CLERK_012")		// "MCU         : %-s", strTempArray[7]
#define T_MSG_CLERK_013		_T("APTextID:T_CLERK_013")		// "------------- SP VERSION ---------------"
#define T_MSG_CLERK_014		_T("APTextID:T_CLERK_014")		// "PIN         : %-s", strTempArray[11]
#define T_MSG_CLERK_015		_T("APTextID:T_CLERK_015")		// "SIU         : %-s", strTempArray[12]
#define T_MSG_CLERK_016		_T("APTextID:T_CLERK_016")		// "------------- EP VERSION ---------------"
#define T_MSG_CLERK_017		_T("APTextID:T_CLERK_017")		// "          SURCHARGE TABLE PRINT         "
#define T_MSG_CLERK_018		_T("APTextID:T_CLERK_018")		// "   THE TABLE CONTAINS ONLY 10 ELEMENTS  "
#define T_MSG_CLERK_019		_T("APTextID:T_CLERK_019")		// " PLEASE BE COMPLIANT TO FORMAT OF FILE  "
#define T_MSG_CLERK_020		_T("APTextID:T_CLERK_020")		// "Withdrawal ~$"
#define T_MSG_CLERK_021		_T("APTextID:T_CLERK_021")		// "Surcharge $"
#define T_MSG_CLERK_022		_T("APTextID:T_CLERK_022")		// "          ERROR CODE INFORMATION        "
#define T_MSG_CLERK_023		_T("APTextID:T_CLERK_023")		// "-------------- ERROR CODE --------------"
#define T_MSG_CLERK_024		_T("APTextID:T_CLERK_024")		// "------------- DESCRIPTION --------------"
#define T_MSG_CLERK_025		_T("APTextID:T_CLERK_025")		// "---------- CORRECTIVE ACTION -----------"
#define T_MSG_CLERK_026		_T("APTextID:T_CLERK_026")		// "            REJECT ANALYSIS             "
#define T_MSG_CLERK_027		_T("APTextID:T_CLERK_027")		// "DISPENSED"
#define T_MSG_CLERK_028		_T("APTextID:T_CLERK_028")		// "REJECTED"
#define T_MSG_CLERK_029		_T("APTextID:T_CLERK_029")		// "SKEW"
#define T_MSG_CLERK_030		_T("APTextID:T_CLERK_030")		// "START"
#define T_MSG_CLERK_031		_T("APTextID:T_CLERK_031")		// "GAP TOO CLOSE"
#define T_MSG_CLERK_032		_T("APTextID:T_CLERK_032")		// "LONG NOTE"
#define T_MSG_CLERK_033		_T("APTextID:T_CLERK_033")		// "SHORT NOTE"
#define T_MSG_CLERK_034		_T("APTextID:T_CLERK_034")		// "DOUBLE DETECT"
#define T_MSG_CLERK_035		_T("APTextID:T_CLERK_035")		// "TEST DISPENSED"
#define T_MSG_CLERK_036		_T("APTextID:T_CLERK_036")		// "            * ERROR SUMMARY *           "
#define T_MSG_CLERK_037		_T("APTextID:T_CLERK_037")		// "     NO      ERROR CODE      COUNT      "

#define T_MSG_CLERK_038		_T("APTextID:T_CLERK_038")		// "EMV KERNEL"		// [#2220] NH KMK 2013.10.15
#define T_MSG_CLERK_039		_T("APTextID:T_CLERK_039")		// "KMR         : %-s", strTempArray[19]	// [#2259] NH KSK 2014.03.14
#define T_MSG_CLERK_040		_T("APTextID:T_CLERK_040")		// "RFID"		// [#2325] NH KSK 2015.01.26
// end of [#2160]

// [#2540] NH JUstin 2018.03.19 Offer Acceptance Report
#define T_MSG_CLERK_OPTOUTSUMMARY			_T("APTextID:T_CLERK_OPTOUT_SUMMARY")				// "           * OPT-OUT SUMMARY *          "		
#define T_CLERK_OPTOUT_SURCHARGE_TR			_T("APTextID:T_CLERK_OPTOUT_SURCHARGE_TR")			// "Surcharge Assessed Tr."
#define T_CLERK_OPTOUT_SURCHARGE_DECLINE	_T("APTextID:T_CLERK_OPTOUT_SURCHARGE_DECLINE")		// "    Surcharge Declines"
#define T_CLERK_OPTOUT_DCC_TR				_T("APTextID:T_CLERK_OPTOUT_DCC_TR")				// "DCC Transactions"
#define T_CLERK_OPTOUT_DCC_DECLINE			_T("APTextID:T_CLERK_OPTOUT_DCC_DECLINE")			// "    DCC Opt-Outs"
// End of [#2540]

// [#2160] CA KMK 2012.11.21 FrenchOP, 명세표 print all setup의 AddPrintData() 부분
#define T_RECE_PRT_ALLSETUP			_T("APTextID:T_RECE_PRT_001")		// "	            PRINT ALL SETUP             "
#define T_RECE_TODAY				_T("APTextID:T_RECE_PRT_002")		// "TODAY              "
#define T_RECE_MACHINEKIND			_T("APTextID:T_RECE_PRT_003")		// "MACHINE KIND       "
#define T_RECE_COUNTRY				_T("APTextID:T_RECE_PRT_004")		// "COUNTRY            "
#define T_RECE_HOST_PROC			_T("APTextID:T_RECE_PRT_005")		// "HOST PROCESSOR     "
#define T_RECE_NETWORKTYPE			_T("APTextID:T_RECE_PRT_006")		// "NETWORK TYPE       "
#define T_RECE_VISA_FRAMED_OPT		_T("APTextID:T_RECE_PRT_007")		// "VISA FRAMED OPTION "
#define T_RECE_TCPIP_TYPE_DEF		_T("APTextID:T_RECE_PRT_008")		// "TCPIP TYPE DEFINED "
#define T_RECE_SSL					_T("APTextID:T_RECE_PRT_009")		// "SSL                "
#define T_RECE_SSL_VER				_T("APTextID:T_RECE_PRT_010")		// "SSL VERSION        "
#define T_RECE_SSL_CERT				_T("APTextID:T_RECE_PRT_011")		// "SSL CERTIFY        "
#define T_RECE_1_VER_INFO			_T("APTextID:T_RECE_PRT_012")		// "1. VERSION INFORMATION"
#define T_RECE_AP					_T("APTextID:T_RECE_PRT_013")		// "AP  "
#define T_RECE_RMS					_T("APTextID:T_RECE_PRT_014")		// "RMS "
#define T_RECE_MWI					_T("APTextID:T_RECE_PRT_015")		// "T-P "  => Change to "MWI"
#define T_RECE_CDU_VDM				_T("APTextID:T_RECE_PRT_016")		// "CDU VDM "
#define T_RECE_SPR_VDM				_T("APTextID:T_RECE_PRT_017")		// "SPR VDM "
#define T_RECE_MCU_VDM				_T("APTextID:T_RECE_PRT_018")		// "MCU VDM "
#define T_RECE_CDU_SP				_T("APTextID:T_RECE_PRT_019")		// "CDU SP"
#define T_RECE_MCU_SP				_T("APTextID:T_RECE_PRT_020")		// "MCU SP"
#define T_RECE_SIU_SP				_T("APTextID:T_RECE_PRT_021")		// "SIU SP"
#define T_RECE_CDU_EP				_T("APTextID:T_RECE_PRT_022")		// "CDU EP"
#define T_RECE_MCU_EP				_T("APTextID:T_RECE_PRT_023")		// "MCU EP"
#define T_RECE_SIU_EP				_T("APTextID:T_RECE_PRT_024")		// "SIU EP"
#define T_RECE_ANTISKIM_EP			_T("APTextID:T_RECE_PRT_025")		// "ANTI SKIMMING EP"
#define T_RECE_2_SYSTEM_SETUP		_T("APTextID:T_RECE_PRT_026")		// "2. SYSTEM SETUP"
#define T_RECE_CUR_REJECT			_T("APTextID:T_RECE_PRT_027")		// "CURRENT # OF REJECT   "
#define T_RECE_CUR_BILL				_T("APTextID:T_RECE_PRT_028")		// "CURRENT # OF BILLS"
#define T_RECE_SN					_T("APTextID:T_RECE_PRT_029")		// "SERIAL NUMBER   "
#define T_RECE_SPK_VOL				_T("APTextID:T_RECE_PRT_030")		// "SPEAKER VOLUME  "
#define T_RECE_HOST_MODEM_INITSTR	_T("APTextID:T_RECE_PRT_031")		// "HOST MODEM INITIAL STRING"
#define T_RECE_RMS_MODEM_INITSTR	_T("APTextID:T_RECE_PRT_032")		// "RMS MODEM INITIAL STRING"
#define T_RECE_TML_DHCP				_T("APTextID:T_RECE_PRT_033")		// "TERMINAL DHCP        "
#define T_RECE_TML_IP_ADDR			_T("APTextID:T_RECE_PRT_034")		// "TERMINAL IP ADDRESS  "
#define T_RECE_TML_GATEWAY			_T("APTextID:T_RECE_PRT_035")		// "TERMINAL GATEWAY     "
#define T_RECE_TML_SUBNET_MASK		_T("APTextID:T_RECE_PRT_036")		// "TERMINAL SUBNET MASK "
#define T_RECE_TML_DNS_SERVER		_T("APTextID:T_RECE_PRT_037")		// "TERMINAL DNS SERVER  "
#define T_RECE_DEF_LANG				_T("APTextID:T_RECE_PRT_038")		// "DEFAULT LANGUAGE"
#define T_RECE_ENGLISH				_T("APTextID:T_RECE_PRT_039")		// "ENGLISH         "
#define T_RECE_SPANISH				_T("APTextID:T_RECE_PRT_040")		// "SPANISH         "
#define T_RECE_FRENCH				_T("APTextID:T_RECE_PRT_041")		// "FRENCH          "
#define T_RECE_CARDREADER_TYPE		_T("APTextID:T_RECE_PRT_042")		// "CARD READER TYPE"
#define T_RECE_ANTISKIM				_T("APTextID:T_RECE_PRT_043")		// "ANTI SKIMMING   "
#define T_RECE_SHUTTER				_T("APTextID:T_RECE_PRT_044")		// "SHUTTER         "
#define T_RECE_TIME_THRES			_T("APTextID:T_RECE_PRT_045")		// "TIME THRESHOLD  "
#define T_RECE_3_CUST_SETUP			_T("APTextID:T_RECE_PRT_046")		// "3. CUSTOMER SETUP"
#define T_RECE_WC_MSG				_T("APTextID:T_RECE_PRT_047")		// "WELCOME MESSAGE"
#define T_RECE_EXIT_MSG				_T("APTextID:T_RECE_PRT_048")		// "EXIT MESSAGE"
#define T_RECE_MARKETING_MSG		_T("APTextID:T_RECE_PRT_049")		// "MARKETING MESSAGE"
#define T_RECE_STORE_MSG			_T("APTextID:T_RECE_PRT_050")		// "STORE MESSAGE"
#define T_RECE_PROC_MSG				_T("APTextID:T_RECE_PRT_051")		// "PROCESSOR MESSAGE"
#define T_RECE_RCPT_HEADER			_T("APTextID:T_RECE_PRT_052")		// "RECEIPT HEADER"
#define T_RECE_RCPT_TAIL			_T("APTextID:T_RECE_PRT_053")		// "RECEIPT TAIL"
#define T_RECE_RCPT_ADDR_PHONE_NUM	_T("APTextID:T_RECE_PRT_054")		// "RECEIPT ADDRESS & PHONE NUMBER"
#define T_RECE_ATM_OPER_FEE			_T("APTextID:T_RECE_PRT_055")		// "ATM OPERATOR FEE  "
#define T_RECE_ATM_OPER_FEE_OWNER	_T("APTextID:T_RECE_PRT_056")		// "ATM OPERATOR FEE OWNER"
#define T_RECE_ATM_OPER_FEE_CINFO	_T("APTextID:T_RECE_PRT_057")		// "ATM OPERATOR FEE CONTACT INFO"
#define T_RECE_WITHDRAW_AMOUNT		_T("APTextID:T_RECE_PRT_058")		// "WITHDRAWAL AMOUNT "
#define T_RECE_BALAMT				_T("APTextID:T_RECE_PRT_059")		// "BALANCE    AMOUNT "
#define T_RECE_SCHGOWNER			_T("APTextID:T_RECE_PRT_060")		// "SURCHARGE OWNER "
#define T_RECE_SCHGMODE				_T("APTextID:T_RECE_PRT_061")		// "SURCHARGE MODE  "
#define T_RECE_SCHGAMT				_T("APTextID:T_RECE_PRT_062")		// "SURCHARGE AMOUNT"
#define T_RECE_BANKNAME_FEEPRINT	_T("APTextID:T_RECE_PRT_063")		// "BANK NAME & FEE PRINT "
#define T_RECE_ADD_SCHG_MODE		_T("APTextID:T_RECE_PRT_064")		// "ADDITIONAL SURCHARGE MODE"
#define T_RECE_PCT_MNR				_T("APTextID:T_RECE_PRT_065")		// "PERCENT MANNER"
#define T_RECE_PCT_AMT				_T("APTextID:T_RECE_PRT_066")		// "PERCENT AMOUNT"
#define T_RECE_BINLIST_TOTCNT		_T("APTextID:T_RECE_PRT_067")		// "BIN LIST TOTAL COUNT "
#define T_RECE_BIN_TOTCNT			_T("APTextID:T_RECE_PRT_068")		// "BIN TOTAL COUNT "
#define T_RECE_EXTDBIN_TOTCNT		_T("APTextID:T_RECE_PRT_069")		// "EXTENDED BIN TOTAL COUNT "
#define T_RECE_MOD10				_T("APTextID:T_RECE_PRT_070")		// "MOD10            "
#define T_RECE_EMV					_T("APTextID:T_RECE_PRT_071")		// "EMV              "
#define T_RECE_LATCHOPT				_T("APTextID:T_RECE_PRT_072")		// "LATCH OPTION       "
#define T_RECE_AIDLIST_CNT			_T("APTextID:T_RECE_PRT_073")		// "AID LIST COUNT   "
#define T_RECE_SEL_RCPT				_T("APTextID:T_RECE_PRT_074")		// "SELECT RECEIPT   "
#define T_RECE_REV_RETRYCNT			_T("APTextID:T_RECE_PRT_075")		// "REVERSAL RETRY COUNT "
#define T_RECE_BAL_ATSTART			_T("APTextID:T_RECE_PRT_076")		// "BALANCE AT START  "
#define T_RECE_CONTAFTER_PREBAL		_T("APTextID:T_RECE_PRT_077")		// "CONTINUE AFTER PRE-BALANCE"
#define T_RECE_PREDIAL				_T("APTextID:T_RECE_PRT_081")		// "PRE DIALING "
#define T_RECE_PREDIAL_TYPE			_T("APTextID:T_RECE_PRT_082")		// "PRE DIALING TYPE "	-> "  TYPE"
#define T_RECE_NEEDMORETIME			_T("APTextID:T_RECE_PRT_083")		// "NEED MORE TIME    "
#define T_RECE_RCPT_PAPER_LOW_SENS	_T("APTextID:T_RECE_PRT_084")		// "RECEIPT PAPER LOW SENSOR "
#define T_RECE_EPP_FLKR_OPT			_T("APTextID:T_RECE_PRT_085")		// "EPP FLICKER OPTION"
#define T_RECE_CST_SND_OPT			_T("APTextID:T_RECE_PRT_086")		// "CST SOUND OPTION  "
#define T_RECE_NTC					_T("APTextID:T_RECE_PRT_087")		// "NOTICE "
#define T_RECE_NTC_TITLE			_T("APTextID:T_RECE_PRT_088")		// "NOTICE TITLE"
#define T_RECE_NTC_MSG				_T("APTextID:T_RECE_PRT_089")		// "NOTICE MESSAGE"
#define T_RECE_CHG_BGR				_T("APTextID:T_RECE_PRT_090")		// "CHANGE BACKGROUND "
#define T_RECE_DEF_SCR				_T("APTextID:T_RECE_PRT_091")		// "DEFAULT SCREEN    "
#define T_RECE_BACK_SCR				_T("APTextID:T_RECE_PRT_092")		// "BACK. SCREEN"
#define T_RECE_WC_ADV_RFRSH_TIME	_T("APTextID:T_RECE_PRT_093")		// "WELCOME ADV. REFRESH TIME "
#define T_RECE_WC_ADV				_T("APTextID:T_RECE_PRT_094")		// "WELCOME ADV."
#define T_RECE_TRANS_ADV_RFRSH_TIME	_T("APTextID:T_RECE_PRT_095")		// "TRANSACTION ADV. REFRESH TIME "
#define T_RECE_TRANS_ADV			_T("APTextID:T_RECE_PRT_096")		// "TRANSACTION ADV."
#define T_RECE_BRDNAME				_T("APTextID:T_RECE_PRT_097")		// "BRAND NAME          "
#define T_RECE_CPNTYPE				_T("APTextID:T_RECE_PRT_098")		// "COUPON TYPE "
#define T_RECE_CPN					_T("APTextID:T_RECE_PRT_099")		// "COUPON"
#define T_RECE_MSG					_T("APTextID:T_RECE_PRT_100")		// "MESSAGE"
#define T_RECE_OPT					_T("APTextID:T_RECE_PRT_101")		// "OPTION"
#define T_RECE_STARTTIME			_T("APTextID:T_RECE_PRT_102")		// "START TIME "
#define T_RECE_ENDTIME				_T("APTextID:T_RECE_PRT_103")		// "END TIME   "
#define T_RECE_AWARD_CPN			_T("APTextID:T_RECE_PRT_104")		// "AWARD COUPON"
#define T_RECE_ONBIN				_T("APTextID:T_RECE_PRT_105")		// "ON BIN"
#define T_RECE_4_TRANS_SETUP		_T("APTextID:T_RECE_PRT_106")		// "4. TRANSACTION SETUP"
#define T_RECE_DISPENSE_LIMIT		_T("APTextID:T_RECE_PRT_107")		// "DISPENSE LIMIT "
#define T_RECE_CURRENCY_ID			_T("APTextID:T_RECE_PRT_108")		// "CURRENCY ID    "
#define T_RECE_DENOM				_T("APTextID:T_RECE_PRT_109")		// "DENOMINATION"
#define T_RECE_FASTCASH				_T("APTextID:T_RECE_PRT_110")		// "FAST CASH"
#define T_RECE_LOW_CURRENCY_CHK		_T("APTextID:T_RECE_PRT_111")		// "LOW CURRENCY CHECK "
#define T_RECE_5_HOST_SETUP			_T("APTextID:T_RECE_PRT_112")		// "5. HOST SETUP"
#define T_RECE_TML					_T("APTextID:T_RECE_PRT_113")		// "TERMINAL #           "
#define T_RECE_STA_MON				_T("APTextID:T_RECE_PRT_114")		// "STATUS MONITORING    "
#define T_RECE_COMM_HEADER			_T("APTextID:T_RECE_PRT_115")		// "COMMUNICATION HEADER "
#define T_RECE_COMM_ID				_T("APTextID:T_RECE_PRT_116")		// "COMMUNICATION ID     "
#define T_RECE_CRC					_T("APTextID:T_RECE_PRT_117")		// "CRC                  "
#define T_RECE_ROUTING_ID			_T("APTextID:T_RECE_PRT_118")		// "ROUTING ID           "
#define T_RECE_TML_STA				_T("APTextID:T_RECE_PRT_119")		// "TERMINAL STATUS      "
#define T_RECE_REV_AT_HOST_ERR		_T("APTextID:T_RECE_PRT_120")		// "REVERSAL AT HOST ERROR "
#define T_RECE_RSN_FOR_REV			_T("APTextID:T_RECE_PRT_121")		// "REASON FOR REVERSAL  "
#define T_RECE_EJ_UL				_T("APTextID:T_RECE_PRT_122")		// "EJ UPLOAD            "
#define T_RECE_AUTO_DAYTOTAL		_T("APTextID:T_RECE_PRT_123")		// "AUTO DAY TOTAL       "
#define T_RECE_AUTO_DAYTOTAL_TYPE	_T("APTextID:T_RECE_PRT_124")		// "AUTO DAY TOTAL TYPE  "
#define T_RECE_AUTO_DAYTOTAL_TIME	_T("APTextID:T_RECE_PRT_125")		// "AUTO DAY TOTAL TIME  "
#define T_RECE_HOST_1ST_PHONE		_T("APTextID:T_RECE_PRT_126")		// "HOST 1ST PHONE #"
#define T_RECE_HOST_2ND_PHONE		_T("APTextID:T_RECE_PRT_127")		// "HOST 2ND PHONE #"
#define T_RECE_HOST_USE_URL			_T("APTextID:T_RECE_PRT_128")		// "HOST USE URL         "
#define T_RECE_HOST_1				_T("APTextID:T_RECE_PRT_129")		// "HOST[1]"
#define T_RECE_HOST_2				_T("APTextID:T_RECE_PRT_130")		// "HOST[2]"
#define T_RECE_URL_NAME				_T("APTextID:T_RECE_PRT_131")		// "URL NAME"
#define T_RECE_IP_ADDR				_T("APTextID:T_RECE_PRT_132")		// "IP ADDRESS   "
#define T_RECE_PORTNO				_T("APTextID:T_RECE_PRT_133")		// "PORT NO"
#define T_RECE_HEALTH_CHK_MSG		_T("APTextID:T_RECE_PRT_134")		// "HEALTH CHECK MESSAGE "
#define T_RECE_HEALTH_CHK_INTERVAL	_T("APTextID:T_RECE_PRT_135")		// "HEALTH CHECK INTERVAL"	-> "INTERVAL"
#define T_RECE_CONF_DL_ATSTART		_T("APTextID:T_RECE_PRT_136")		// "CONFIG DOWNLOAD AT START"
#define T_RECE_RMS_CONN				_T("APTextID:T_RECE_PRT_137")		// "RMS CONNECTION       "
#define T_RECE_RMS_STATUS_SEND		_T("APTextID:T_RECE_PRT_138")		// "RMS STATUS SEND      "
#define T_RECE_RMS_RING_CNT			_T("APTextID:T_RECE_PRT_139")		// "RMS RING COUNT "
#define T_RECE_RMS_1ST_PHONE		_T("APTextID:T_RECE_PRT_140")		// "RMS 1ST PHONE # "
#define T_RECE_RMS_2ND_PHONE		_T("APTextID:T_RECE_PRT_141")		// "RMS 2ND PHONE # "
#define T_RECE_RMS_SEND_INTERVAL	_T("APTextID:T_RECE_PRT_142")		// "RMS SEND INTERVAL    "
#define T_RECE_ATM_LISTEN_PORTNO	_T("APTextID:T_RECE_PRT_143")		// "ATM LISTENING PORT NO"
#define T_RECE_RMS_USE_URL			_T("APTextID:T_RECE_PRT_144")		// "RMS USE URL          "
#define T_RECE_RMS_URL_NAME			_T("APTextID:T_RECE_PRT_145")		// "RMS URL NAME    "
#define T_RECE_RMS_IP_ADDR			_T("APTextID:T_RECE_PRT_146")		// "RMS IP ADDRESS       "
#define T_RECE_RMS_LISTEN_PORTNO	_T("APTextID:T_RECE_PRT_147")		// "RMS LISTENING PORT NO"
#define T_RECE_SCHED_JNL_UL			_T("APTextID:T_RECE_PRT_148")		// "SCHEDULED JOURNAL UPLOAD "
#define T_RECE_JNL_UL_TYPE			_T("APTextID:T_RECE_PRT_149")		// "JOURNAL UPLOAD TYPE  "
#define T_RECE_JNL_UL_CNT			_T("APTextID:T_RECE_PRT_150")		// "JOURNAL UPLOAD COUNT "
#define T_RECE_JNL_UL_DAY			_T("APTextID:T_RECE_PRT_151")		// "JOURNAL UPLOAD DAY   "
#define T_RECE_JNL_UL_HOUR			_T("APTextID:T_RECE_PRT_152")		// "JOURNAL UPLOAD HOUR  "
#define T_RECE_RKT					_T("APTextID:T_RECE_PRT_153")		// "RKT                  "
#define T_RECE_RKT_RAND_NUM			_T("APTextID:T_RECE_PRT_154")		// "RKT RANDOM NUMBER    "
#define T_RECE_KEYMODE				_T("APTextID:T_RECE_PRT_155")		// "KEY MODE "
#define T_RECE_MK_CHKVAL			_T("APTextID:T_RECE_PRT_156")		// "MASTER KEY CHECK VALUE"
#define T_RECE_MAC_KEY_CHKVAL		_T("APTextID:T_RECE_PRT_157")		// "MAC KEY CHECK VALUE"
#define T_RECE_MK_INDEX				_T("APTextID:T_RECE_PRT_158")		// "MASTER KEY INDEX "
#define T_RECE_KEYCHECK				_T("APTextID:T_RECE_PRT_159")		// "KEY CHECK "
#define T_RECE_MKEY					_T("APTextID:T_RECE_PRT_160")		// "MKEY"
#define T_RECE_CHK_DIGIT			_T("APTextID:T_RECE_PRT_161")		// "CHECK DIGIT"
#define T_RECE_ACK_CTRLD			_T("APTextID:T_RECE_PRT_162")		// "ACK CONTROLLED"
#define T_RECE_AFTER_ACCT			_T("APTextID:T_RECE_PRT_163")		// "AFTER ACCOUNT"
#define T_RECE_AFTER_CARD			_T("APTextID:T_RECE_PRT_164")		// "AFTER CARD"
#define T_RECE_AFTER_PIN			_T("APTextID:T_RECE_PRT_165")		// "AFTER PIN"
#define T_RECE_AUSTRALIA			_T("APTextID:T_RECE_PRT_166")		// "AUSTRALIA"
#define T_RECE_CANADA				_T("APTextID:T_RECE_PRT_167")		// "CANADA"
#define T_RECE_DAYTOTAL				_T("APTextID:T_RECE_PRT_168")		// "DAY TOTAL"
#define T_RECE_DES_MAC				_T("APTextID:T_RECE_PRT_169")		// "DES, MAC"
#define T_RECE_DIALUP				_T("APTextID:T_RECE_PRT_170")		// "DIALUP"
#define T_RECE_DISABLE				_T("APTextID:T_RECE_PRT_171")		// "DISABLE"
#define T_RECE_DUAL_MK_KEY			_T("APTextID:T_RECE_PRT_172")		// "DUAL MASTER KEY"
#define T_RECE_ENABLE				_T("APTextID:T_RECE_PRT_173")		// "ENABLE"
#define T_RECE_EOT_OPTIONAL			_T("APTextID:T_RECE_PRT_174")		// "EOT OPTIONAL"
#define T_RECE_EPS					_T("APTextID:T_RECE_PRT_175")		// "EPS"
#define T_RECE_GENERAL				_T("APTextID:T_RECE_PRT_176")		// "GENERAL"
#define T_RECE_MAGTEK_DIP			_T("APTextID:T_RECE_PRT_177")		// "MAGTEK DIP"
#define T_RECE_MEXICO				_T("APTextID:T_RECE_PRT_178")		// "MEXICO"
#define T_RECE_NEWZEALAND			_T("APTextID:T_RECE_PRT_179")		// "NEW ZEALAND"
#define T_RECE_NH_DIP				_T("APTextID:T_RECE_PRT_180")		// "NH DIP"
#define T_RECE_NO_ENQ_REQ			_T("APTextID:T_RECE_PRT_181")		// "NO ENQ REQUIRED"
#define T_RECE_NO_ENQ_EOT_REQ		_T("APTextID:T_RECE_PRT_182")		// "NO ENQ/EOT REQUIRED"
#define T_RECE_NO_EOT_REQ			_T("APTextID:T_RECE_PRT_183")		// "NO EOT REQUIRED"
#define T_RECE_SANKYO_DIP			_T("APTextID:T_RECE_PRT_184")		// "SANKYO DIP"
#define T_RECE_SCR					_T("APTextID:T_RECE_PRT_185")		// "SCREEN"
#define T_RECE_SNGL_KEY_DES			_T("APTextID:T_RECE_PRT_186")		// "SINGLE KEY(DES)"
#define T_RECE_STD					_T("APTextID:T_RECE_PRT_187")		// "STANDARD"
#define T_RECE_STD1					_T("APTextID:T_RECE_PRT_188")		// "STANDARD1"
#define T_RECE_STD2					_T("APTextID:T_RECE_PRT_189")		// "STANDARD2"
#define T_RECE_STD3					_T("APTextID:T_RECE_PRT_190")		// "STANDARD3"
#define T_RECE_TCPIP				_T("APTextID:T_RECE_PRT_191")		// "TCP/IP"
#define T_RECE_TDES					_T("APTextID:T_RECE_PRT_192")		// "TDES"
#define T_RECE_MAC					_T("APTextID:T_RECE_PRT_193")		// "MAC"
#define T_RECE_TMAC					_T("APTextID:T_RECE_PRT_194")		// "TMAC"
#define T_RECE_TRIALDAYTOTAL		_T("APTextID:T_RECE_PRT_195")		// "TRIAL DAY TOTAL"
#define T_RECE_TYPE_A				_T("APTextID:T_RECE_PRT_196")		// "TYPE A "
#define T_RECE_TYPE_B				_T("APTextID:T_RECE_PRT_197")		// "TYPE B"
#define T_RECE_DES					_T("APTextID:T_RECE_PRT_198")		// "DES"
#define T_RECE_UNIQ_KEY				_T("APTextID:T_RECE_PRT_199")		// "UNIQUE KEY"
#define T_RECE_UNKNOWN				_T("APTextID:T_RECE_PRT_200")		// "UNKNOWN"
#define T_RECE_USA					_T("APTextID:T_RECE_PRT_201")		// "USA"
#define T_RECE_V23					_T("APTextID:T_RECE_PRT_202")		// "V23"
#define T_RECE_V3					_T("APTextID:T_RECE_PRT_203")		// "V3"
#define T_RECE_VISA_FRAMED			_T("APTextID:T_RECE_PRT_204")		// "VISA FRAMED"
#define T_RECE_GREATER				_T("APTextID:T_RECE_PRT_205")		// "GREATER"
#define T_RECE_LESSER				_T("APTextID:T_RECE_PRT_206")		// "LESSER"
#define T_RECE_NOT_USED				_T("APTextID:T_RECE_PRT_207")		// "NOT USED"
#define T_RECE_PCT					_T("APTextID:T_RECE_PRT_208")		// "PERCENT"
#define T_RECE_TABLE				_T("APTextID:T_RECE_PRT_209")		// "TABLE"
#define T_RECE_PIN_EP				_T("APTextID:T_RECE_PRT_210")		// "PIN EP"
#define T_RECE_PIN_SP				_T("APTextID:T_RECE_PRT_211")		// "PIN SP"
#define T_RECE_SPR_SP				_T("APTextID:T_RECE_PRT_212")		// "SPR SP"
#define T_RECE_SPR_EP				_T("APTextID:T_RECE_PRT_213")		// "SPR EP"
#define T_RECE_HOUR					_T("APTextID:T_RECE_PRT_214")		// "HOUR"
#define T_RECE_B_L					_T("APTextID:T_RECE_PRT_215")		// "B-L"
#define T_RECE_OS					_T("APTextID:T_RECE_PRT_216")		// "OS"
#define T_RECE_SEC					_T("APTextID:T_RECE_PRT_217")		// "sec"
// [#2186] US KMK 2013.04.15	SUPERVISOR LANGUAGE 추가
#define T_RECE_SUPV_LANG			_T("APTextID:T_RECE_PRT_218")		// "SUPERVISOR LANGUAGE" 
// end of [#2186]
// end of [#2160]

// [#2205] US KSK 2013.06.28
#define T_RECE_INSERVICE			_T("APTextID:T_RECE_PRT_219")		// "IN SERVICE"
#define T_RECE_TRANSACTION			_T("APTextID:T_RECE_PRT_220")		// "TRANSACTION"
#define T_RECE_DISPENSING			_T("APTextID:T_RECE_PRT_221")		// "DISPENSING"
#define T_RECE_OUTOFSERVICE			_T("APTextID:T_RECE_PRT_222")		// "OUT OF SERVICE"
// end of [#2205]

// [#2208] US Justin 2013.07.08 Add Print all setup info
#define T_RECE_DYNAMICFLOW			_T("APTextID:T_RECE_PRT_223")		// "Dynamic Flow" 
#define T_RECE_PINCHANGE			_T("APTextID:T_RECE_PRT_224")		// "PIN Change" 
#define T_RECE_CURRENCYCONVERSION	_T("APTextID:T_RECE_PRT_225")		// "Currency Conversion" 
#define T_RECE_DCCOPTION			_T("APTextID:T_RECE_PRT_226")		// "Currency Conversion OPTION" 
#define T_RECE_DCCDISCLAIMER		_T("APTextID:T_RECE_PRT_227")		// "Currency Conversion Disclaimer" 
// End of [#2208]

#define T_RECE_CONTINUEFALLBACK		_T("APTextID:T_RECE_PRT_228")		// "Continue Fallback"		// [#2209] US Justin 2013.07.11 Add Print All setup info
#define T_RECE_RECEIPTONSCREEN		_T("APTextID:T_RECE_PRT_229")		// "REceipt on Screen"		// [#2209] US Justin 2013.07.11 Add Print All setup info

#define T_RECE_DF1STCALL			_T("APTextID:T_RECE_PRT_230")		// [#2242] US Justin 2013.12.18 DF 1st Call Option

#define T_RECE_KEYMGR_VDM			_T("APTextID:T_RECE_PRT_231")		// "KEYMGR"	[#2259] NH KSK 2014.03.14
// [#2160] CA KMK 2012.10.08

// [#2292] US Justin 2014.10.13 Cardtronics TDL Option (Print All Setup)
#define T_RECE_DUALBALANCE			_T("APTextID:T_RECE_PRT_235")		

#define T_RECE_TDL_DYNAMICSURCHARGE	_T("APTextID:T_RECE_PRT_236")		
#define T_RECE_TDL_DCC				_T("APTextID:T_RECE_PRT_237")		
#define T_RECE_TDL_ADDITIONALCRFEE	_T("APTextID:T_RECE_PRT_238")		
// End of [#2292]

// [#2310] KSK Support TSL
// [#2320] US Justin 2015.01.14 Support NON-"Version Negotiation" Processor
//#define T_RECE_SSLV3_AND_ABOVE	_T("APTextID:T_RECE_PRT_239")		// "SSL V3 AND ABOVE"
//#define T_RECE_TLSV10_AND_ABOVE	_T("APTextID:T_RECE_PRT_240")		// "TLS V1.0 AND ABOVE"
//#define T_RECE_TLSV11_AND_ABOVE	_T("APTextID:T_RECE_PRT_241")		// "TLS V1.1 AND ABOVE"
//#define T_RECE_TLSV12				_T("APTextID:T_RECE_PRT_242")		// "TLS V1.2"
#define T_RECE_CONN_UPTO_SSL_V30	_T("APTextID:T_RECE_PRT_239")		// "SSL V3.0"
#define T_RECE_CONN_UPTO_TLS_V10	_T("APTextID:T_RECE_PRT_240")		// "UP TO TLS V1.0"
#define T_RECE_CONN_UPTO_TLS_V11	_T("APTextID:T_RECE_PRT_241")		// "UP TO TLS V1.1"
#define T_RECE_CONN_UPTO_TLS_V12	_T("APTextID:T_RECE_PRT_242")		// "TLS v1.2"
#define T_RECE_CONN_UPTO_TLS_V13	_T("APTextID:T_RECE_PRT_307")		// "TLS v1.3"
// End of [#2320]
// End of [#2310]

#define T_RECE_BALANCEFEE_NOTICE	_T("APTextID:T_RECE_PRT_243")		// [#2309] US Jusitn 2014.11.17 Balance Fee Notice

// [#2313] US Justin 2011.11.25 Print all setup for Event notification
#define T_RECE_EVENT_NOTIFICATION	_T("APTextID:T_RECE_PRT_244")		// "NOTIFICATION" 
#define T_RECE_SSLENABLE			_T("APTextID:T_RECE_PRT_245")		// "HTTPS" 
#define T_RECE_HOSTURL				_T("APTextID:T_RECE_PRT_246")		// "HOST URL" 
#define T_RECE_HOSTPORT				_T("APTextID:T_RECE_PRT_247")		// "HOST PORT" 
#define T_RECE_USERNAME				_T("APTextID:T_RECE_PRT_248")		// "USERNAME" 
#define T_RECE_PASSWORD				_T("APTextID:T_RECE_PRT_249")		// "PASSWORD" 
// End of [#2313]

#define T_RECE_DOMESTIC_BIN			_T("APTextID:T_RECE_PRT_250")		// "DOMESTIC BIN"	// [#2317] Domestic BIN

#define T_RECE_RFID_SP				_T("APTextID:T_RECE_PRT_251")		// "RFID SP"	// [#2325] NH KSK 2015.01.26
#define T_RECE_RFID_EP				_T("APTextID:T_RECE_PRT_252")		// "RFID EP"	// [#2325] NH KSK 2015.01.26
#define T_RECE_RFID_VDM				_T("APTextID:T_RECE_PRT_253")		// "RFID VDM "	// [#2325] NH KSK 2015.01.26

#define T_RECE_EPP_LED_GUIDE_OPT	_T("APTextID:T_RECE_PRT_254")		// "EPP LED GUIDE" [#2333] NH KSK 2015.03.06
#define T_RECE_PRECENTNOTICE		_T("APTextID:T_RECE_PRT_255")		// "PERCENTAGE NOTICE" [#2335] US Jusitn 2015.03.12 Percentage Surcharge Notice
#define T_RECE_COMMON_AID			_T("APTextID:T_RECE_PRT_256")		// "USE COMMON AID"    [#2342] US Jusitn 2015.05.08 Use common AID
#define T_RECE_DCCLOCALSURCHARGE	_T("APTextID:T_RECE_PRT_257")		// "DCC LOCAL SURCHARGE"	[#2349] US Justin DCC Look up Local Surcharge


#define T_RECE_MONIMOBILEQR			_T("APTextID:T_RECE_PRT_262")		// "MONIMOBILE			[#2432] US Justin MoniMobile QR Print All setup
#define T_RECE_DIGITAL_RECEIPT		_T("APTextID:T_RECE_PRT_263")		// "DIGITAL RECEIPT"	// Trunk 호환성을 위해 변경
#define T_RECE_TDL_ALLPOINTSURCHARGE _T("APTextID:T_RECE_PRT_264")		// "ALLPOINT SURCHARGE" // [#2437] US Justin
#define T_RECE_PAYPAL_CCA			_T("APTextID:T_RECE_PRT_265")		// "PAYPAL CCA"			// [#2446] US Justin
#define T_RECE_JUSTCASH				_T("APTextID:T_RECE_PRT_266")		// "JUST.CASH"			// [#2448] US Justin
#define T_RECE_EMV_LAN_SELECT		_T("APTextID:T_RECE_PRT_267")		// "LANGUAGE SELECTION"	// [#2425] AU KSK 2016.12.26

#define T_RECE_DCC_WITH_OPTION		_T("APTextID:T_RECE_PRT_273")		// "DCC CARD VERIFICATION"	[#2472] US Justin 2017.02.09 DCC Withdrawal Option
#define T_RECE_FALLBACK4UNKNOWNAID	_T("APTextID:T_RECE_PRT_274")		// "UNKNOWN IC CARD"		[#2481] US Jusitn 2017.04.15 Fallback Option 4 unknown AID
#define T_RECE_STD1_DF_MAC			_T("APTextID:T_RECE_PRT_275")		// "MAC"					[#2490] NH Justin 2017.06.15 STD1 Dynamid Flow MAC Option

#define T_RECE_FALLBACK_ENDISABLE	_T("APTextID:T_RECE_PRT_276")		// "FALLBACK"				[#2517] US Justin 2017.11.30 EMV Fallback Enable/Diable Option
#define T_RECE_TOTAL_AMOUNT_OPTION	_T("APTextID:T_RECE_PRT_277")		// "PRINT TOTAL AMOUNT" //  [#2525] AU HJ AHN 2018.01.10 Print Option 추가 (TOTAL AMOUNT En/Disable ) - AU ONLY

#define T_RECE_ANTI_SHLDR_SURF		_T("APTextID:T_RECE_PRT_278")		// "Live-Feed"				[#2518] US Kook 2018.01.23 Support MX-2800SE 	// [#2548] NH Justin 2018.04.27 Change Name MoniVision, Live-Feed, Journal Pic
#define T_RECE_FACE_MEMORIZE		_T("APTextID:T_RECE_PRT_279")		// "Journal Pic"			[#2518] US Kook 2018.01.23 Support MX-2800SE	// [#2548] NH Justin 2018.04.27 Change Name MoniVision, Live-Feed, Journal Pic
#define T_RECE_CAMERA				_T("APTextID:T_RECE_PRT_280")		// "MoniVision"				2018.02.23										// [#2548] NH Justin 2018.04.27 Change Name MoniVision, Live-Feed, Journal Pic
#define T_RECE_FREESPACE			_T("APTextID:T_RECE_PRT_281")		// "FREE SPACE"				2018.03.13
#define T_RECE_KILOBYTE				_T("APTextID:T_RECE_PRT_282")		// "KB"						2018.03.13

#define T_RECE_DENOMINATION_SELECT	_T("APTextID:T_RECE_PRT_283")		// "DENOMINATION SELECT"	[#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA

#define T_RECE_DYNAMICFLOW_DCC_TYPE	_T("APTextID:T_RECE_PRT_284")		// "DYNAMIC FLOW DCC TYPE"	[#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
#define T_RECE_DUALHOST_DCC_TYPE	_T("APTextID:T_RECE_PRT_285")		// "DUAL HOST DCC TYPE"		[#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message

// [#RWC6-16] Bitload 4 U. A new cryptocurrency feature for CE 6.0 ATMs
#define T_RECE_B4U_TXN_INFO			_T("APTextID:T_RECE_PRT_286")		// "BTC TRANSACTION INFORMATION"
#define T_RECE_B4U_BTC_ADDRESS		_T("APTextID:T_RECE_PRT_287")		// "BTC ADDRESS"
#define T_RECE_B4U_BTC_CHARGED		_T("APTextID:T_RECE_PRT_288")		// "BTC CHARGED"
#define T_RECE_B4U_FEE_CHARGED		_T("APTextID:T_RECE_PRT_289")		// "FEE CHARGED"
#define T_RECE_B4U_BTC_RATE			_T("APTextID:T_RECE_PRT_290")		// "BTC RATE"
#define T_RECE_B4U_TXN_APPROVED		_T("APTextID:T_RECE_PRT_291")		// "Your Transaction is Approved"
#define T_RECE_B4U_DISC_1_01		_T("APTextID:T_RECE_PRT_292")		// "Contact support@b4ufinancial.com if you"
#define T_RECE_B4U_DISC_1_02		_T("APTextID:T_RECE_PRT_293")		// "didn’t receive money owed for this with"
#define T_RECE_B4U_DISC_1_03		_T("APTextID:T_RECE_PRT_294")		// "the picture of this receipt."
#define T_RECE_B4U_DISC_2_01		_T("APTextID:T_RECE_PRT_295")		// "Your transaction will be credited..."
#define T_RECE_B4U_DISC_2_02		_T("APTextID:T_RECE_PRT_296")		// "subject to verification, collection and"
#define T_RECE_B4U_DISC_2_03		_T("APTextID:T_RECE_PRT_297")		// "the Rules and Regulations for B4U"
#define T_RECE_B4U_DISC_2_04		_T("APTextID:T_RECE_PRT_298")		// "Financial and as otherwise provided by"
#define T_RECE_B4U_DISC_2_05		_T("APTextID:T_RECE_PRT_299")		// "law. Update to your account balance is"
#define T_RECE_B4U_DISC_2_06		_T("APTextID:T_RECE_PRT_300")		// "controlled by your wallet and the"
#define T_RECE_B4U_DISC_2_07		_T("APTextID:T_RECE_PRT_301")		// "blockchain and may require up 24 hours."
#define T_RECE_B4U_DISC_2_08		_T("APTextID:T_RECE_PRT_302")		// "Please retain this receipt until you "
#define T_RECE_B4U_DISC_2_09		_T("APTextID:T_RECE_PRT_303")		// "confirm receipt of funds."
#define T_RECE_B4U_BTC_TXN_ID		_T("APTextID:T_RECE_PRT_304")		// "TRANSACTION ID"
#define T_RECE_B4U_BTC_TXN_HASH		_T("APTextID:T_RECE_PRT_305")		// "TRANSACTION HASH"
#define T_RECE_B4U_FINANCIAL		_T("APTextID:T_RECE_PRT_306")		// "B4U Financial"
// end of [#RWC6-16]

//#define T_RECE_PRT_307			_T("APTextID:T_RECE_PRT_307")		// "TLS v1.3"
#define T_RECE_DUAL_RCPT			_T("APTextID:T_RECE_PRT_308")		// "DUAL RECEIPT   "		[#RWC6-290] US Brandon 2021.07.06 Support dual-language receipts

#define T_RECE_QRBALANCE			_T("APTextID:T_RECE_PRT_QRBALANCE")		// "QR BALANCE"			[#2538] NH Justin 2018.03.09 MoniMobile Cash Balance Option
#define T_RECE_US_TERRITORY			_T("APTextID:T_RECE_PRT_USTERRITORY")	// "US TERRITORY"		[#2549] NH Justin 2018.05.08 Add Perto Rico Option

#define T_RECE_SCHEDULED_REBOOT		_T("APTextID:T_RECE_PRT_S_REBOOT")		// "SCHEDULED REBOOT"	[#2558] NH Justin 2018.06.14 Add Scheduled Reboot Optioin
#define T_RECE_REBOOT_TIME			_T("APTextID:T_RECE_PRT_REBOOT_T")		// "DAILY REBOOT TIME"	[#2558] NH Justin 2018.06.14 Add Scheduled Reboot Optioin
#define T_RECE_REBOOT_INTERVAL		_T("APTextID:T_RECE_PRT_REBOOT_I")		// "INTERVAL"			[#2558] NH Justin 2018.06.14 Add Scheduled Reboot Optioin

#define T_RECE_RMS_SEND_RETRY		_T("APTextID:T_RECE_PRT_RMSSENDRETRY")	// "COMM. ERROR RESEND"	[#2559] NH Justin 2018.06.19 Retry Status Send if "send" failed
#define T_RECE_MINUTES				_T("APTextID:T_RECE_PRT_MINUTES")		// "MINUTE(S)"			[#2559] NH Justin 2018.06.19 Retry Status Send if "send" failed

#define T_RECE_LIBERTYX				_T("APTextID:T_RECE_PRT_LIBERTYX")		// LibertyX				[#RWC6-59] US William 2019.11.06 LibertyX
#define T_RECE_LTX_LOC				_T("APTextID:T_RECE_PRT_LTX_LOC")		// Location ID			[#RWC6-59] US William 2019.11.06 LibertyX
#define T_RECE_LTX_RID				_T("APTextID:T_RECE_PRT_LTX_RID")		// Routing ID			[#RWC6-59] US William 2019.11.06 LibertyX
#define T_RECE_LTX_H1				_T("APTextID:T_RECE_PRT_LTX_H1")		// Host 1				[#RWC6-59] US William 2019.11.06 LibertyX
#define T_RECE_LTX_H2				_T("APTextID:T_RECE_PRT_LTX_H2")		// Host 2				[#RWC6-59] US William 2019.11.06 LibertyX
#define T_RECE_LTX_HP1				_T("APTextID:T_RECE_PRT_LTX_HP1")		// Host Port 1			[#RWC6-59] US William 2019.11.06 LibertyX
#define T_RECE_LTX_HP2				_T("APTextID:T_RECE_PRT_LTX_HP2")		// Host Port 2			[#RWC6-59] US William 2019.11.06 LibertyX
#define T_RECE_LTX_TLS				_T("APTextID:T_RECE_PRT_LTX_TLS")		// TLS option			[#RWC6-59] US William 2019.11.06 LibertyX
#define T_RECE_LTX_PROTO			_T("APTextID:T_RECE_PRT_LTX_PROTO")		// Protocol				[#RWC6-59] US William 2019.11.06 LibertyX
#define T_RECE_LTX_ENV				_T("APTextID:T_RECE_PRT_LTX_ENV")		// Envelope				[#RWC6-59] US William 2019.11.06 LibertyX
#define T_RECE_LTX_URL				_T("APTextID:T_RECE_PRT_LTX_URL")		// API URL				[#RWC6-59] US William 2019.11.06 LibertyX
#define T_RECE_LTX_USER				_T("APTextID:T_RECE_PRT_LTX_USER")		// Username				[#RWC6-59] US William 2019.11.06 LibertyX

#define T_RECE_TANGONET				_T("APTextID:T_RECE_PRT_TANGONET")		// TangoNet				[#RWC6-676] SKKim 2024.05.20 TangoNet
#define T_RECE_TANGONET_IDEN_URL	_T("APTextID:T_RECE_PRT_IDENTITY")		// TangoNet				[#RWC6-676] SKKim 2024.05.20 TangoNet
#define T_RECE_TANGONET_TRANS_URL	_T("APTextID:T_RECE_PRT_TRANSACTION")	// TangoNet				[#RWC6-676] SKKim 2024.05.20 TangoNet
#define T_RECE_TANGONET_PARTNERID	_T("APTextID:T_RECE_PRT_PARTNERID")		// TangoNet				[#RWC6-676] SKKim 2024.05.20 TangoNet
#define T_RECE_TANGONET_APIKEY		_T("APTextID:T_RECE_PRT_APIKEY")		// TangoNet				[#RWC6-676] SKKim 2024.05.20 TangoNet
#define T_RECE_TANGONET_PUBLICKEY	_T("APTextID:T_RECE_PRT_PUBLICKEY")		// TangoNet				[#RWC6-676] SKKim 2024.05.20 TangoNet
#define T_RECE_TANGONET_EXIST		_T("APTextID:T_RECE_PRT_EXIST")			// TangoNet				[#RWC6-676] SKKim 2024.05.20 TangoNet
#define T_RECE_TANGONET_NOT_EXIST	_T("APTextID:T_RECE_PRT_NOT_EXIST")		// TangoNet				[#RWC6-676] SKKim 2024.05.20 TangoNet


#define T_RECE_RU_SCHEDULED			_T("APTextID:T_RECE_PRT_RU_SCHEDULED")	// SCHEDULED UPDATE		[#RWC6-149] Support Remote Updates on PAS
#define T_RECE_RU_DOW				_T("APTextID:T_RECE_PRT_RU_DOW")		// DAY OF WEEK			[#RWC6-149] Support Remote Updates on PAS 
#define T_RECE_RU_CURRCONFIG		_T("APTextID:T_RECE_PRT_RU_CURRCONFIG")	// CURRENT CONFIG		[#RWC6-149] Support Remote Updates on PAS
#define T_RECE_RU_HOST				_T("APTextID:T_RECE_PRT_RU_HOST")		// HOST					[#RWC6-149] Support Remote Updates on PAS
#define T_RECE_RU_HIVES				_T("APTextID:T_RECE_PRT_RU_HIVES")		// HIVES				[#RWC6-149] Support Remote Updates on PAS
#define T_RECE_RU_SEVERITY			_T("APTextID:T_RECE_PRT_RU_SEVERITY")	// SEVERITY				[#RWC6-149] Support Remote Updates on PAS

#define T_RECE_PRT_RMS_TLS			_T("APTextID:T_RECE_PRT_RMS_TLS")		// RMS TLS				[#RWC6-67] US William 2019.11.12 LibertyX

#define T_RECE_PRT_DCC_DISCLAIMER		_T("APTextID:T_RECE_PRT_DCC_DISCLAIMER")	// I have been offered a choice of\ncurrencies for this withdrawal.\nThis currency conversion service\nwas offered by X_OWNER
#define T_RECE_PRT_DCC_DISCLAIMER_MC	_T("APTextID:T_RECE_PRT_DCC_DISCLAIMER_MC")	// DCC and exchange rates are provided by X_OWNER
#define T_RECE_PRT_DCC_DECLINE			_T("APTextID:T_RECE_PRT_DCC_DECLINE")

/*------------------------------------------------------------------
JOURNAL DATA
-------------------------------------------------------------------*/
#define T_MSG_JNL_001		_T("APTextID:T_JNL_001")				//	"TERMINAL #"
#define T_MSG_JNL_002		_T("APTextID:T_JNL_002")				//	"COMMUNICATION ID"
#define T_MSG_JNL_003		_T("APTextID:T_JNL_003")				//	"ROUTING ID"
#define T_MSG_JNL_004		_T("APTextID:T_JNL_004")				//	"CASSETTE          :  CST1 CST2 CST3 CST4"
#define T_MSG_JNL_005		_T("APTextID:T_JNL_005")				//	"REMAINING COUNT"
#define T_MSG_JNL_006		_T("APTextID:T_JNL_006")				//	"ADDITION  COUNT"
#define T_MSG_JNL_007		_T("APTextID:T_JNL_007")				//	"START TIME"
#define T_MSG_JNL_008		_T("APTextID:T_JNL_008")				//	"END   TIME"
#define T_MSG_JNL_009		_T("APTextID:T_JNL_009")				//	"[ 1. HOST TOTALS ]"
#define T_MSG_JNL_010		_T("APTextID:T_JNL_010")				//	"COUNT OF WITHDRAWAL"
#define T_MSG_JNL_011		_T("APTextID:T_JNL_011")				//	"COUNT OF BALANCE"
#define T_MSG_JNL_012		_T("APTextID:T_JNL_012")				//	"COUNT OF TRANSFER"
#define T_MSG_JNL_013		_T("APTextID:T_JNL_013")				//	"COUNT OF CHECK CASH"
#define T_MSG_JNL_014		_T("APTextID:T_JNL_014")				//	"AMOUNT OF DISPENSE"
#define T_MSG_JNL_015		_T("APTextID:T_JNL_015")				//	"AMOUNT OF CHECK CASH"
#define T_MSG_JNL_016		_T("APTextID:T_JNL_016")				//	"[ 2. TERMINAL TOTALS ]"
#define T_MSG_JNL_017		_T("APTextID:T_JNL_017")				//	"COUNT OF NON-CASH"
#define T_MSG_JNL_018		_T("APTextID:T_JNL_018")				//	"COUNT OF REVERSAL"
#define T_MSG_JNL_019		_T("APTextID:T_JNL_019")				//	"AMOUNT OF NON-CASH"
#define T_MSG_JNL_020		_T("APTextID:T_JNL_020")				//	"AMOUNT OF SURCHARGE"
#define T_MSG_JNL_021		_T("APTextID:T_JNL_021")				//	"START TIME"
#define T_MSG_JNL_022		_T("APTextID:T_JNL_022")				//	"END   TIME"
#define T_MSG_JNL_023		_T("APTextID:T_JNL_023")				//	"DENOMINATION"
#define T_MSG_JNL_024		_T("APTextID:T_JNL_024")				//	"INITIAL"
#define T_MSG_JNL_025		_T("APTextID:T_JNL_025")				//	"REMAINING"
#define T_MSG_JNL_026		_T("APTextID:T_JNL_026")				//	"REJECTED"
#define T_MSG_JNL_027		_T("APTextID:T_JNL_027")				//	"JAMMED"
#define T_MSG_JNL_028		_T("APTextID:T_JNL_028")				//	"DISPENSED"
#define T_MSG_JNL_029		_T("APTextID:T_JNL_029")				//	"INITIAL AMOUNT"
#define T_MSG_JNL_030		_T("APTextID:T_JNL_030")				//	"REMAINING AMOUNT"
#define T_MSG_JNL_031		_T("APTextID:T_JNL_031")				//	"OLD DENO."
#define T_MSG_JNL_032		_T("APTextID:T_JNL_032")				//	"NEW DENO."
#define T_MSG_JNL_033		_T("APTextID:T_JNL_033")				//	"OLD MSG. FORMAT"
#define T_MSG_JNL_034		_T("APTextID:T_JNL_034")				//	"NEW MSG. FORMAT"
#define T_MSG_JNL_035		_T("APTextID:T_JNL_035")				//	"SEQUENCE #"
#define T_MSG_JNL_036		_T("APTextID:T_JNL_036")				//	"AUTH.    #"
#define T_MSG_JNL_037		_T("APTextID:T_JNL_037")				//	"DATE & TIME"
#define T_MSG_JNL_038		_T("APTextID:T_JNL_038")				//	"BUSINESS DATE"
#define T_MSG_JNL_039		_T("APTextID:T_JNL_039")				//	"CUSTOMER NUMBER"
#define T_MSG_JNL_040		_T("APTextID:T_JNL_040")				//	"CARD NUMBER"
#define T_MSG_JNL_041		_T("APTextID:T_JNL_041")				//	"TRANSACTION"
#define T_MSG_JNL_042		_T("APTextID:T_JNL_042")				//	"WITHDRAWAL"
#define T_MSG_JNL_043		_T("APTextID:T_JNL_043")				//	"BALANCE INQUIRY"
#define T_MSG_JNL_044		_T("APTextID:T_JNL_044")				//	"TRANSFER"
#define T_MSG_JNL_046		_T("APTextID:T_JNL_046")				//	"NON CASH WITH."
#define T_MSG_JNL_047		_T("APTextID:T_JNL_047")				//	"PIN CHANGE"
#define T_MSG_JNL_048		_T("APTextID:T_JNL_048")				//	"CURRENCY CONVERSION"
#define T_MSG_JNL_049		_T("APTextID:T_JNL_049")				//	"FROM"
#define T_MSG_JNL_050		_T("APTextID:T_JNL_050")				//	"ACCOUNT"
#define T_MSG_JNL_051		_T("APTextID:T_JNL_051")				//	"TO"
#define T_MSG_JNL_052		_T("APTextID:T_JNL_052")				//	"DISPENSED AMOUNT"
#define T_MSG_JNL_053		_T("APTextID:T_JNL_053")				//	"REQUESTED AMOUNT"
#define T_MSG_JNL_054		_T("APTextID:T_JNL_054")				//	"TRANSFER AMOUNT"
#define T_MSG_JNL_055		_T("APTextID:T_JNL_055")				//	"SURCHARGE AMOUNT"
#define T_MSG_JNL_056		_T("APTextID:T_JNL_056")				//	"BALANCE UNAVAILABLE"
#define T_MSG_JNL_057		_T("APTextID:T_JNL_057")				//	"AVAILABLE BALANCE"
#define T_MSG_JNL_058		_T("APTextID:T_JNL_058")				//	"LEDGER BALANCE"
#define T_MSG_JNL_059		_T("APTextID:T_JNL_059")				//	"ERROR CODE"
#define T_MSG_JNL_060		_T("APTextID:T_JNL_060")				//	"LOGIN MODE"
#define T_MSG_JNL_061		_T("APTextID:T_JNL_061")				//	"OLD PASSWORD"
#define T_MSG_JNL_062		_T("APTextID:T_JNL_062")				//	"NEW PASSWORD"
#define T_MSG_JNL_063		_T("APTextID:T_JNL_063")				//	"UNKNOWN"
#define T_MSG_JNL_064		_T("APTextID:T_JNL_064")				//	"OPERATOR"
#define T_MSG_JNL_065		_T("APTextID:T_JNL_065")				//	"SERVICE"
#define T_MSG_JNL_066		_T("APTextID:T_JNL_066")				//	"MASTER"
#define T_MSG_JNL_067		_T("APTextID:T_JNL_067")				//	"RMS"
#define T_MSG_JNL_068		_T("APTextID:T_JNL_068")				//	"HOST"
#define T_MSG_JNL_069		_T("APTextID:T_JNL_069")				//	"USER NAME"
#define T_MSG_JNL_070		_T("APTextID:T_JNL_070")				//	"PARAMETER"
#define T_MSG_JNL_071		_T("APTextID:T_JNL_071")				//	"OLD"
#define T_MSG_JNL_072		_T("APTextID:T_JNL_072")				//	"NEW"
#define T_MSG_JNL_073		_T("APTextID:T_JNL_073")				//	"AID"
#define T_MSG_JNL_074		_T("APTextID:T_JNL_074")				//	"NAME"
#define T_MSG_JNL_075		_T("APTextID:T_JNL_075")				//	"ARQC"
#define T_MSG_JNL_076		_T("APTextID:T_JNL_076")				//	"ARPC"
#define T_MSG_JNL_077		_T("APTextID:T_JNL_077")				//	"SERVICE CODE"
#define T_MSG_JNL_078		_T("APTextID:T_JNL_078")				//	"TC"
#define T_MSG_JNL_079		_T("APTextID:T_JNL_079")				//	"POS ENTRY MODE"
#define T_MSG_JNL_080		_T("APTextID:T_JNL_080")				//	"TVR"
#define T_MSG_JNL_081		_T("APTextID:T_JNL_081")				//	"IAC"
#define T_MSG_JNL_082		_T("APTextID:T_JNL_082")				//	"SURCHARGE PAID TO"
#define T_MSG_JNL_083		_T("APTextID:T_JNL_083")				//	"HOST RESULT"
#define T_MSG_JNL_084		_T("APTextID:T_JNL_084")				//	"OK"
#define T_MSG_JNL_085		_T("APTextID:T_JNL_085")				//	"ERROR"

#define T_MSG_JNL_086		_T("APTextID:T_JNL_086")				//	"TOTAL ATM OPERATOR FEE"
#define T_MSG_JNL_087		_T("APTextID:T_JNL_087")				//	"AMOUNT OF ATM FEE"
#define T_MSG_JNL_088		_T("APTextID:T_JNL_088")				//	"REJECTED(APPROX)"

#define T_MSG_JNL_092		_T("APTextID:T_JNL_092")				//	"Popmoney WITHDRAWAL"

#define T_MSG_JNL_094		_T("APTextID:T_JNL_094")				//	"ACCESS CODE"

// [#2405] US Justin 2016.03.21 HalCash Online
#define T_MSG_JNL_099		_T("APTextID:T_JNL_099")				//	"CASH PICK-UP"		// [#2470]

// [#2353] AU KSK 2015.06.28
#define T_MSG_JNL_104		_T("APTextID:T_JNL_104")				//	"PAN SEQ.NO"
#define T_MSG_JNL_105		_T("APTextID:T_JNL_105")				//	"AIP"
#define T_MSG_JNL_106		_T("APTextID:T_JNL_106")				//	"ARC"
#define T_MSG_JNL_107		_T("APTextID:T_JNL_107")				//	"TRANS. TYPE"
#define T_MSG_JNL_108		_T("APTextID:T_JNL_108")				//	"TSI"
#define T_MSG_JNL_109		_T("APTextID:T_JNL_109")				//	"IAD"
#define T_MSG_JNL_110		_T("APTextID:T_JNL_110")				//	"CID"
#define T_MSG_JNL_111		_T("APTextID:T_JNL_111")				//	"ATC"
#define T_MSG_JNL_112		_T("APTextID:T_JNL_112")				//	"UNPREDICTABLE NO."
#define T_MSG_JNL_113		_T("APTextID:T_JNL_113")				//	"TRANS. CURRENCY"
#define T_MSG_JNL_114		_T("APTextID:T_JNL_114")				//	"TERMINAL COUNTRY"
// end of [#2353]

#define T_MSG_JNL_117		_T("APTextID:T_JNL_117")				//	"Just.Cash	// [#2445] US Justin 2016.09.28 just.cash
#define T_MSG_JNL_118		_T("APTextID:T_JNL_118")				//	"CASH CODE	// [#2445] US Justin 2016.09.28 just.cash
#define T_MSG_JNL_119		_T("APTextID:T_JNL_119")				//	"Paypal CCA	// [#2446] US Justin 2016.09.29 Paypal CCA
#define T_MSG_JNL_120		_T("APTextID:T_JNL_120")				//	"ORDER NUMBER" or "Pin4 Code" 	// [#2470] 
#define T_MSG_JNL_121		_T("APTextID:T_JNL_121")				//	"GivePay"	// [#2535] 

// [#RWC6-59] US William 2019.10.09 LibertyX
// LibertyX
#define T_MSG_JNL_LTX			_T("APTextID:T_JNL_LTX_")
#define T_MSG_JNL_LTX_DISPENSE	_T("APTextID:T_JNL_LTX_DISPENSE_") // [#RWC6-224] US William 2020.10.14 LibertyX Dispense

// [#RWC6-399/#RWC6-413] US zach.morningstar 2022.09.21 DigitalMint
#define T_MSG_JNL_DM                    _T("APTextID:T_JNL_DM_")
#define T_MSG_JNL_DM_15                 _T("APTextID:T_JNL_DM_15")                  // "TRANSACTION MODE"
#define T_MSG_JNL_DM_MODE_DEBIT         _T("APTextID:T_JNL_DM_MODE_DEBIT")          // "DEBIT"
#define T_MSG_JNL_DM_MODE_CASH          _T("APTextID:T_JNL_DM_MODE_CASH")           // "CASH"
#define T_MSG_JNL_DM_MODE_UNKNOWN       _T("APTextID:T_JNL_DM_MODE_UNKNOWN")        // "UNKNOWN"
// end of [#RWC6-399/#RWC6-413]

// [RWC6-676] Start SKKim 2024.05.09
#define T_MSG_JNL_TP                    _T("APTextID:T_JNL_TP_")

// [RWC6-676] End SKKim 2024.05.09

// JnlMgr.cpp의 g_CESupportEJKindCode[] 변수, TranBizLib.cpp의 g_CESupportPRTKindCode 변수에서 사용
#define T_ADDCASH_CST1					_T("APTextID:T_JNL_ADDCASH_CST1")			/* ADD CASH #1 */
#define T_ADDCASH_CST2					_T("APTextID:T_JNL_ADDCASH_CST2")			/* ADD CASH #2 */
#define T_ADDCASH_CST1_2				_T("APTextID:T_JNL_ADDCASH_CST1_2")			/* ADD CASH #1,2 */
#define T_ADDCASH_CST3					_T("APTextID:T_JNL_ADDCASH_CST3")			/* ADD CASH #3 */
#define T_ADDCASH_CST1_3				_T("APTextID:T_JNL_ADDCASH_CST1_3")			/* ADD CASH #1,3 */
#define T_ADDCASH_CST2_3				_T("APTextID:T_JNL_ADDCASH_CST2_3")			/* ADD CASH #2,3 */
#define T_ADDCASH_CST1_2_3				_T("APTextID:T_JNL_ADDCASH_CST1_2_3")		/* ADD CASH #1,2,3 */
#define T_ADDCASH_CST4					_T("APTextID:T_JNL_ADDCASH_CST4")			/* ADD CASH #4 */
#define T_ADDCASH_CST1_4				_T("APTextID:T_JNL_ADDCASH_CST1_4")			/* ADD CASH #1,4 */
#define T_ADDCASH_CST2_4				_T("APTextID:T_JNL_ADDCASH_CST2_4")			/* ADD CASH #2,4 */
#define T_ADDCASH_CST1_2_4				_T("APTextID:T_JNL_ADDCASH_CST1_2_4")		/* ADD CASH #1,2,4 */
#define T_ADDCASH_CST3_4				_T("APTextID:T_JNL_ADDCASH_CST3_4")			/* ADD CASH #3,4 */
#define T_ADDCASH_CST1_3_4				_T("APTextID:T_JNL_ADDCASH_CST1_3_4")		/* ADD CASH #1,3,4 */
#define T_ADDCASH_CST2_3_4				_T("APTextID:T_JNL_ADDCASH_CST2_3_4")		/* ADD CASH #2,3,4 */
#define T_ADDCASH_CST1_2_3_4			_T("APTextID:T_JNL_ADDCASH_CST1_2_3_4")		/* ADD CASH #1,2,3,4 */

#define T_REMOTE_ADDCASH_CST1			_T("APTextID:T_JNL_REMOTE_ADDCASH_CST1")		/* REMOTE ADD CASH #1 */
#define T_REMOTE_ADDCASH_CST2			_T("APTextID:T_JNL_REMOTE_ADDCASH_CST2")		/* REMOTE ADD CASH #2 */
#define T_REMOTE_ADDCASH_CST1_2			_T("APTextID:T_JNL_REMOTE_ADDCASH_CST1_2")		/* REMOTE ADD CASH #1,2 */
#define T_REMOTE_ADDCASH_CST3			_T("APTextID:T_JNL_REMOTE_ADDCASH_CST3")		/* REMOTE ADD CASH #3 */
#define T_REMOTE_ADDCASH_CST1_3			_T("APTextID:T_JNL_REMOTE_ADDCASH_CST1_3")		/* REMOTE ADD CASH #1,3 */
#define T_REMOTE_ADDCASH_CST2_3			_T("APTextID:T_JNL_REMOTE_ADDCASH_CST2_3")		/* REMOTE ADD CASH #2,3 */
#define T_REMOTE_ADDCASH_CST1_2_3		_T("APTextID:T_JNL_REMOTE_ADDCASH_CST1_2_3")	/* REMOTE ADD CASH #1,2,3 */
#define T_REMOTE_ADDCASH_CST4			_T("APTextID:T_JNL_REMOTE_ADDCASH_CST4")		/* REMOTE ADD CASH #4 */
#define T_REMOTE_ADDCASH_CST1_4			_T("APTextID:T_JNL_REMOTE_ADDCASH_CST1_4")		/* REMOTE ADD CASH #1,4 */
#define T_REMOTE_ADDCASH_CST2_4			_T("APTextID:T_JNL_REMOTE_ADDCASH_CST2_4")		/* REMOTE ADD CASH #2,4 */
#define T_REMOTE_ADDCASH_CST1_2_4		_T("APTextID:T_JNL_REMOTE_ADDCASH_CST1_2_4")	/* REMOTE ADD CASH #1,2,4 */
#define T_REMOTE_ADDCASH_CST3_4			_T("APTextID:T_JNL_REMOTE_ADDCASH_CST3_4")		/* REMOTE ADD CASH #3,4 */
#define T_REMOTE_ADDCASH_CST1_3_4		_T("APTextID:T_JNL_REMOTE_ADDCASH_CST1_3_4")	/* REMOTE ADD CASH #1,3,4 */
#define T_REMOTE_ADDCASH_CST2_3_4		_T("APTextID:T_JNL_REMOTE_ADDCASH_CST2_3_4")	/* REMOTE ADD CASH #2,3,4 */
#define T_REMOTE_ADDCASH_CST1_2_3_4		_T("APTextID:T_JNL_REMOTE_ADDCASH_CST1_2_3_4")	/* REMOTE ADD CASH #1,2,3,4 */

#define T_JNL_DAY_TOTAL					_T("APTextID:T_JNL_DAY_TOTAL")					/* DAY TOTAL */
#define T_JNL_REMOTE_DAY_TOTAL			_T("APTextID:T_JNL_REMOTE_DAY_TOTAL")			/* REMOTE DAY TOTAL */
#define T_JNL_CST_TOTAL					_T("APTextID:T_JNL_CST_TOTAL")					/* CASSETTE TOTAL */
#define T_JNL_REMOTE_CST_TOTAL			_T("APTextID:T_JNL_REMOTE_CST_TOTAL")			/* REMOTE CASSETTE TOTAL */
#define T_JNL_TRIALDAY_TOTAL			_T("APTextID:T_JNL_TRIALDAY_TOTAL")				/* TRIAL DAY TOTAL */
#define T_JNL_REMOTE_TRIALDAY_TOTAL		_T("APTextID:T_JNL_REMOTE_TRIALDAY_TOTAL")		/* REMOTE TRIAL DAY TOTAL */
#define T_JNL_TRIALCST_TOTAL			_T("APTextID:T_JNL_TRIALCST_TOTAL")				/* TRIAL CASSETTE TOTAL */

// [#GLDV-3005] US Kook 2022.04.14 support side car
#define T_JNL_CASHIN_TOTAL				_T("APTextID:T_JNL_CASHIN_TOTAL")				/* CASH IN TOTAL */
#define T_JNL_TRIALCASHIN_TOTAL			_T("APTextID:T_JNL_TRIAL_CASHIN_TOTAL")			/* TRIAL CASH IN TOTAL */
// end of [#GLDV-3005]

// [#2405] US Justin 2016.03.21 HalCash Online
#define T_JNL_PIN4_DAY_TOTAL			_T("APTextID:T_JNL_PIN4_DAY_TOTAL")				/* PIN4 DAY TOTAL */
#define T_JNL_PIN4_TRIALDAY_TOTAL		_T("APTextID:T_JNL_PIN4_TRIALDAY_TOTAL")		/* PIN4 TRIAL DAY TOTAL */
// End of [#2405]

#define T_JNL_POPMONEY_DAY_TOTAL		_T("APTextID:T_JNL_POPMONEY_DAY_TOTAL")			/* POPMONEY DAY TOTAL */
#define T_JNL_POPMONEY_TRIALDAY_TOTAL	_T("APTextID:T_JNL_POPMONEY_TRIALDAY_TOTAL")	/* POPMONEY TRIAL DAY TOTAL */
// End of [#2442]

// [#2445] US Justin 2016.09.22 Just.Cash
#define T_JNL_JUSTCASH_DAY_TOTAL		_T("APTextID:T_JNL_JUSTCASH_DAY_TOTAL")			/* JUST.CASH DAY TOTAL */
#define T_JNL_JUSTCASH_TRIALDAY_TOTAL	_T("APTextID:T_JNL_JUSTCASH_TRIALDAY_TOTAL")	/* JUST.CASH TRIAL DAY TOTAL */
// End of [#2445]

// [#2496] US Justin 2017.08.17 BitCoin
#define T_JNL_DIGITALCUR_DAY_TOTAL		_T("APTextID:T_JNL_DIGITALCUR_DAY_TOTAL")		/* DIGITAL CURRENCY DAY TOTAL */
#define T_JNL_DIGITALCUR_TRIALDAY_TOTAL	_T("APTextID:T_JNL_DIGITALCUR_TRIALDAY_TOTAL")	/* DIGITAL CURRENCY TRIAL DAY TOTAL */
// End of [#2496]

// [#2446] US Justin 2016.09.29 Paypal CCA
#define T_JNL_PAYPAL_DAY_TOTAL			_T("APTextID:T_JNL_PAYPAL_DAY_TOTAL")			/* Palpal CCA DAY TOTAL */
#define T_JNL_PAYPAL_TRIALDAY_TOTAL		_T("APTextID:T_JNL_PAYPAL_TRIALDAY_TOTAL")		/* Paypal CCA TRIAL DAY TOTAL */
// End of [#2446]

// [#RWC6-16] Bitload4U
#define T_JNL_B4U_DAY_TOTAL				_T("APTextID:T_JNL_B4U_DAY_TOTAL")				/* B4U DAY TOTAL */
#define T_JNL_B4U_TRIALDAY_TOTAL		_T("APTextID:T_JNL_B4U_TRIALDAY_TOTAL")			/* B4U TRIAL DAY TOTAL */
// End of [#RWC6-16]

// [#RWC6-188] B4U Log journal entry for B4U for any transaction result after the amount entry screen
#define T_JNL_B4U_TXN					_T("APTextID:T_JNL_B4U_TXN")					/* B4U TRANSACTION */	
#define T_JNL_B4U_TXN_ERROR_FAIL		_T("APTextID:T_JNL_B4U_TXN_ERROR_FAIL")			/* B4U TRANSACTION FAIL */
#define T_JNL_B4U_TXN_ERROR_CANCEL		_T("APTextID:T_JNL_B4U_TXN_ERROR_CANCEL")		/* B4U TRANSACTION CANCEL */
#define T_JNL_B4U_TXN_ERROR_TIMEOUT		_T("APTextID:T_JNL_B4U_TXN_ERROR_TIMEOUT")		/* B4U TRANSACTION TIMEOUT */
// End of [#RWC6-188]

#define T_CHANGE_DENOM_CST1				_T("APTextID:T_JNL_CHANGE_DENOM_CST1")			/* CHANGE DENOMINATION OF CASSETTE #1 */
#define T_CHANGE_DENOM_CST2				_T("APTextID:T_JNL_CHANGE_DENOM_CST2")			/* CHANGE DENOMINATION OF CASSETTE #2 */
#define T_CHANGE_DENOM_CST1_2			_T("APTextID:T_JNL_CHANGE_DENOM_CST1_2")		/* CHANGE DENOMINATION OF CASSETTE #1,2 */
#define T_CHANGE_DENOM_CST3				_T("APTextID:T_JNL_CHANGE_DENOM_CST3")			/* CHANGE DENOMINATION OF CASSETTE #3 */
#define T_CHANGE_DENOM_CST1_3			_T("APTextID:T_JNL_CHANGE_DENOM_CST1_3")		/* CHANGE DENOMINATION OF CASSETTE #1,3 */
#define T_CHANGE_DENOM_CST2_3			_T("APTextID:T_JNL_CHANGE_DENOM_CST2_3")		/* CHANGE DENOMINATION OF CASSETTE #2,3 */
#define T_CHANGE_DENOM_CST1_2_3			_T("APTextID:T_JNL_CHANGE_DENOM_CST1_2_3")		/* CHANGE DENOMINATION OF CASSETTE #1,2,3 */
#define T_CHANGE_DENOM_CST4				_T("APTextID:T_JNL_CHANGE_DENOM_CST4")			/* CHANGE DENOMINATION OF CASSETTE #4 */
#define T_CHANGE_DENOM_CST1_4			_T("APTextID:T_JNL_CHANGE_DENOM_CST1_4")		/* CHANGE DENOMINATION OF CASSETTE #1,4 */
#define T_CHANGE_DENOM_CST2_4			_T("APTextID:T_JNL_CHANGE_DENOM_CST2_4")		/* CHANGE DENOMINATION OF CASSETTE #2,4 */
#define T_CHANGE_DENOM_CST1_2_4			_T("APTextID:T_JNL_CHANGE_DENOM_CST1_2_4")		/* CHANGE DENOMINATION OF CASSETTE #1,2,4 */
#define T_CHANGE_DENOM_CST3_4			_T("APTextID:T_JNL_CHANGE_DENOM_CST3_4")		/* CHANGE DENOMINATION OF CASSETTE #3,4 */
#define T_CHANGE_DENOM_CST1_3_4			_T("APTextID:T_JNL_CHANGE_DENOM_CST1_3_4")		/* CHANGE DENOMINATION OF CASSETTE #1,3,4 */
#define T_CHANGE_DENOM_CST2_3_4			_T("APTextID:T_JNL_CHANGE_DENOM_CST2_3_4")		/* CHANGE DENOMINATION OF CASSETTE #1,3,4 */
#define T_CHANGE_DENOM_CST1_2_3_4		_T("APTextID:T_JNL_CHANGE_DENOM_CST1_2_3_4")	/* CHANGE DENOMINATION OF CASSETTE #1,2,3,4 */

#define T_CHANGE_PROCESSOR				_T("APTextID:T_JNL_CHANGE_PROCESSOR")			/* CHANGE MESSAGE FORMAT */

#define T_CANCEL_AT_READ_CARD			_T("APTextID:T_JNL_CANCEL_AT_READ_CARD")		/* CANCEL AT CARD READING */
#define T_CANCEL_AT_SELECT_LANGUAGE		_T("APTextID:T_JNL_CANCEL_AT_SELECT_LANGUAGE")	/* CANCEL AT LANGUAGE SELECTION */
#define T_CANCEL_AT_INPUT_PIN			_T("APTextID:T_JNL_CANCEL_AT_INPUT_PIN")		/* CANCEL AT PIN INPUT */
#define T_CANCEL_AT_SELECT_TRAN			_T("APTextID:T_JNL_CANCEL_AT_SELECT_TRAN")		/* CANCEL AT TRANSACTION SELECTION */
#define T_CANCEL_AT_CONFIRM_FEE			_T("APTextID:T_JNL_CANCEL_AT_CONFIRM_FEE")		/* CANCEL AT SURCHARGE CONFIRMATION */
#define T_CANCEL_AT_RECV_RECEIPT		_T("APTextID:T_JNL_CANCEL_AT_RECV_RECEIPT")		/* CANCEL AT RECEIPT PRINTING SELECTION */
#define T_CANCEL_AT_RECEIPT_ERROR		_T("APTextID:T_JNL_CANCEL_AT_RECEIPT_ERROR")	/* CANCEL AT CONTINUATION ON PRINT ERROR */
#define T_CANCEL_AT_RECEIPT_SCREEN		_T("APTextID:T_JNL_CANCEL_AT_RECEIPT_SCREEN")	/* CANCEL AT RECEIPT DISPLAY ON SCRREN */
#define T_CANCEL_AT_FROM_ACCOUNT		_T("APTextID:T_JNL_CANCEL_AT_FROM_ACCOUNT")		/* CANCEL AT FROM-ACCOUNT SELECTION */
#define T_CANCEL_AT_TO_ACCOUNT			_T("APTextID:T_JNL_CANCEL_AT_TO_ACCOUNT")		/* CANCEL AT TO-ACCOUNT SELECTION */
#define T_CANCEL_AT_FASTCASH			_T("APTextID:T_JNL_CANCEL_AT_FASTCASH")			/* CANCEL AT FASTCASH SELECTION */
#define T_CANCEL_AT_OTHER_AMOUNT		_T("APTextID:T_JNL_CANCEL_AT_OTHER_AMOUNT")		/* CANCEL AT OTHER AMOUNT INPUT */
#define T_CANCEL_AT_TRANSFER_AMOUNT		_T("APTextID:T_JNL_CANCEL_AT_TRANSFER_AMOUNT")	/* CANCEL AT TRANSFER AMOUNT INPUT */

#define T_OPERATOR_ACTION				_T("APTextID:T_JNL_OPERATOR_ACTION")			/* EVENT */
#define T_CHANGE_EXCHANGE_RATE			_T("APTextID:T_JNL_CHANGE_EXCHANGE_RATE")		/* EVENT */

#define T_CHANGE_MASTER_PWD				_T("APTextID:T_JNL_CHANGE_MASTER_PWD")			/* CHANGE MASTER PASSWORD */
#define T_CHANGE_OPERATOR_PWD			_T("APTextID:T_JNL_CHANGE_OPERATOR_PWD")		/* CHANGE OPERATOR PASSWORD */
#define T_CHANGE_SUPERVISOR_PWD			_T("APTextID:T_JNL_CHANGE_SUPERVISOR_PWD")		/* CHANGE SERVICE PASSWORD */

#define T_POWER_ON_SYSTEM				_T("APTextID:T_JNL_POWER_ON_SYSTEM")			/* SYSTEM POWER ON */
#define T_POWER_OFF_SYSTEM				_T("APTextID:T_JNL_POWER_OFF_SYSTEM")			/* SYSTEM POWER OFF */		// [#2536]
#define T_CHANGE_SERVICE_MODE			_T("APTextID:T_JNL_CHANGE_SERVICE_MODE")		/* CHANGE INTO SERVICE MODE */
#define T_CHANGE_ERROR_MODE				_T("APTextID:T_JNL_CHANGE_ERROR_MODE")			/* CHANGE INTO ERROR MODE */
#define T_CHANGE_OP_MODE				_T("APTextID:T_JNL_CHANGE_OP_MODE")				/* CHANGE INTO OPERATOR MODE */
#define T_CHANGE_OP_RESET				_T("APTextID:T_JNL_CHANGE_OP_RESET")			/* RESET OPERATOR */
#define T_REBOOT_SYSTEM					_T("APTextID:T_JNL_REBOOT_SYSTEM")				/* SYSTEM REBOOT */
#define T_CHANGE_DIAG_MODE				_T("APTextID:T_JNL_CHANGE_DIAG_MODE")			/* CHANGE INTO DIAGNOSTICS MODE */

#define T_NORMAL_TRX					_T("APTextID:T_JNL_NORMAL_TRX")					/* NORMAL TRANSACTION */
#define T_TRX_ERROR						_T("APTextID:T_JNL_TRX_ERROR")					/* TRANSACTION ERROR */
#define T_REVERSAL_TRX					_T("APTextID:T_JNL_REVERSAL_TRX")				/* REVERSAL TRANSACTION */
#define T_TRX_NOT_APPROVED				_T("APTextID:T_JNL_TRX_NOT_APPROVED")			/* TRANSACTION IS NOT APPROVED */
#define T_COMMUNICATION_ERROR			_T("APTextID:T_JNL_COMMUNICATION_ERROR")		/* COMMUNICATION ERROR */

#define T_MONIVIEW_ACTION				_T("APTextID:T_JNL_MONIVIEW_ACTION")			/* EVENT */
#define T_HOST_ACTION			    	_T("APTextID:T_JNL_HOST_ACTION")				/* EVENT */
#define T_CHANGE_PARAMETER 	    		_T("APTextID:T_JNL_CHANGE_PARAMETER")			/* CHANGE PARAMETER */
#define T_TRX_EMV_DATA 	    			_T("APTextID:T_JNL_TRX_EMV_DATA")				/* EMV DATA        */
#define T_ALPHI_TRX_ERROR    			_T("APTextID:T_JNL_ALPHI_TRX_ERROR")			/* ALPHI TRANSACTION ERROR */

#define T_LTX_TXN_NORMAL				_T("APTextID:T_LTX_TXN_NORMAL")					/* LibertyX Normal TXN */
#define T_LTX_TXN_ERROR					_T("APTextID:T_LTX_TXN_ERROR")					/* LibertyX Error TXN */
#define T_LTX_TXN_DISPENSE				_T("APTextID:T_LTX_TXN_DISPENSE")	

// [#RWC6-399] US ryan.payton 2022.08.02 DigitalMint
#define T_DM_TXN_NORMAL					_T("APTextID:T_DM_TXN_NORMAL")					/* DigitalMint Normal TXN */
#define T_DM_TXN_ERROR					_T("APTextID:T_DM_TXN_ERROR")					/* DigitalMint Error TXN */
// End of [#RWC6-188]

// [RWC6-676] Start SKKim 2024.05.09
#define T_TP_TXN_NORMAL					_T("APTextID:T_TP_TXN_NORMAL")					/* TangoPay Normal TXN */
#define T_TP_TXN_ERROR					_T("APTextID:T_TP_TXN_ERROR")					/* TangoPay Error TXN */
// [RWC6-676] End SKKim 2024.05.09

// end of [#2160]

// [#RWC6-512] US ryan.payton 2023.04.25 [PAI] Software Enhancement
/*------------------------------------------------------------------
Screen Number 714
-------------------------------------------------------------------*/
#define T_TCPIP					_T("APTextID:714000")				// "TCP/IP"
#define T_TCPIP_PAI				_T("APTextID:714001")				// "Internet"
// End of [#RWC6-512]

// [#2160] CA KMK 2012.10.22 FrenchOP, Screen 736
/*------------------------------------------------------------------
Screen Number 736
-------------------------------------------------------------------*/
#define T_VISA_FRAMED			_T("APTextID:736028")				// "VISA FRAMED"
#define T_STANDARD				_T("APTextID:736029")				// "STANDARD"
#define T_ACK_CONTROLLED		_T("APTextID:736030")				// "ACK CONTROLLED"
#define T_GENERAL				_T("APTextID:736031")				// "GENERAL"
#define T_NO_ENQ_REQUIRED		_T("APTextID:736032")				// "NO ENQ REQUIRED"
#define T_EOT_OPTIONAL			_T("APTextID:736033")				// "EOT OPTIONAL"
// #define T_NO_ENQ_EOT_REQUIRED	_T("APTextID:736034")				// "NO ENQ / EOT REQUIRED"	973번과 겹침
#define T_NO_EOT_REQUIRED		_T("APTextID:736035")				// "NO EOT REQUIRED"
#define T_STANDARD1				_T("APTextID:736036")				// "STANDARD1"
#define T_STANDARD2				_T("APTextID:736037")				// "STANDARD2"
#define T_STANDARD3				_T("APTextID:736038")				// "STANDARD3"
#define T_EPS					_T("APTextID:736039")				// "EPS"
// end of [#2160]

// [#2160] CA KMK 2012.10.22 FrenchOP, Screen 973
/*------------------------------------------------------------------
Screen Number 973
-------------------------------------------------------------------*/
// #define T_GENERAL				_T("APTextID:973006")				// "GENERAL"			736번과 겹침
#define T_REQUIRED				_T("APTextID:973007")				// "REQUIRED"
// #define T_NO_ENQ_REQUIRED		_T("APTextID:973008")				// "NO ENQ REQUIRED"	736번과 겹침
#define T_NOT_REQUIRED			_T("APTextID:973009")				// "NOT REQUIRED"
// #define T_EOT_OPTIONAL			_T("APTextID:973010")				// "EOT OPTIONAL"		736번과 겹침
#define T_OPTIONAL				_T("APTextID:973011")				// "OPTIONAL"
// #define T_NO_EOT_REQUIRED		_T("APTextID:973012")				// "NO EOT REQUIRED"	736번과 겹침
#define T_NO_ENQ_EOT_REQUIRED	_T("APTextID:973013")				// "NO ENQ/EOT REQUIRED"
// end of [#2160]

// [#2160] CA KMK 2012.10.22 FrenchOP, Screen 741, 810, 817, 824
/*------------------------------------------------------------------
Screen Number 741
-------------------------------------------------------------------*/
#define T_UNKNOWN				_T("APTextID:741004")				// "UNKNOWN"
/*------------------------------------------------------------------
Screen Number 810
-------------------------------------------------------------------*/
//	#define T_UNDEFINED_ERROR_CODE	_T("APTextID:810005")				// "Undefined error code" 겹침.
#define T_CALL_ATTENDANT		_T("APTextID:810006")				// "Call your attendant"

// [#2371] US Justin 2015.09.08
/*------------------------------------------------------------------
Screen Number 756
-------------------------------------------------------------------*/
#define T_FULLCUT				_T("APTextID:756003")				// "Full Cut"
#define T_PARTIALCUT			_T("APTextID:756004")				// "Partial Cut"
// End of [#2371]

/*------------------------------------------------------------------
Screen Number 817
-------------------------------------------------------------------*/
#define T_AFTER_CARD	_T("APTextID:817F2")						// "AFTER CARD"
#define T_AFTER_PIN		_T("APTextID:817F4")						// "AFTER PIN"
#define T_AFTER_ACCOUNT	_T("APTextID:817005")						// "AFTER ACCOUNT"

/*------------------------------------------------------------------
Screen Number 824
-------------------------------------------------------------------*/
#define T_DOOR_OPEN				_T("APTextID:824006")				// "DOOR OPEN"
#define T_DOOR_CLOSE			_T("APTextID:824007")				// "DOOR CLOSE"
#define T_JACK_DETECTED			_T("APTextID:824008")				// "JACK DETECTED"
#define T_JACK_EMPTY			_T("APTextID:824009")				// "JACK EMPTY"
#define T_DETECTED				_T("APTextID:824010")				// "DETECTED"
#define T_NO_DETECTED			_T("APTextID:824011")				// "NO DETECTED"
// end of [#2160]



// [#2160] CA KMK 2012.10.22 FrenchOP, Screen 845
/*------------------------------------------------------------------
Screen Number 845
-------------------------------------------------------------------*/
#define T_CDU_CONN_FAIL			_T("APTextID:845007")				// "CDU CONNECTION FAILURE"
#define T_CASSETTE				_T("APTextID:845008")				// "Cassette"
// end of [#2160]


/*------------------------------------------------------------------
Screen Number 847
-------------------------------------------------------------------*/
#define T_CANNOT_BE_ENABLED		_T("APTextID:847012")
#define T_CANNOT_BE_DISABLED	_T("APTextID:847013")
// [#2160] CA KMK 2012.10.22 FrenchOP, Screen 847
// #define T_DISABLE			_T("APTextID:T_DISABLE")			// "DISABLE"
// #define T_ENABLE				_T("APTextID:T_ENABLE")				// "ENABLE"
#define T_SCREEN				_T("APTextID:847016")				// "SCREEN"

// end of [#2160]

// [#2160] CA KMK 2012.10.22 FrenchOP, Screen 872
/*------------------------------------------------------------------
Screen Number 872
-------------------------------------------------------------------*/
#define T_VERSION_23			_T("APTextID:872007")				// "VERSION 23"
#define T_VERSION_3				_T("APTextID:872008")				// "VERSION 3"
/* 다른 화면과 문구가 겹침.
#define T_DISABLE				_T("APTextID:872009")				// "DISABLE"
#define T_ENABLE				_T("APTextID:872010")				// "ENABLE"
*/
// end of [#2160]

/*------------------------------------------------------------------
Screen Number 899
-------------------------------------------------------------------*/
#define T_OPMSG_899001		_T("APTextID:899001")			// "Please wait while initializing..."


/*------------------------------------------------------------------
	Screen Number 901
-------------------------------------------------------------------*/
#define T_COMMU_CHANGED_REBOOT	_T("APTextID:901101")			// "CONFIGURATION IS CHANGED. REBOOTING."	// [#153] NH KGS 2008.04.21 Network 설정 변경후 In-service시 Reboot시 안내 문구 추가
#define T_INITALIZING_DEVICE	_T("APTextID:901103")			// "INITALIZING_DEVICE"				// [#184] NH KGS 2008.05.06 Device Error 안내 메시지 장치별 세부 분류
// [#2160] CA KMK 2012.10.22 French OP, Screen 901
#define T__STANDARD_1			_T("APTextID:901001")			// " STANDARD 1"
#define T__STANDARD_2			_T("APTextID:901002")			// " STANDARD 2"
#define T__STANDARD_3			_T("APTextID:901003")			// " STANDARD 3"
#define T__EPS					_T("APTextID:901004")			// " EPS"
#define T__UNKNOWN				_T("APTextID:901005")			// " UNKNOWN"
#define T__DIALUP				_T("APTextID:901006")			// " DIAL-UP"
#define T__TCPIP				_T("APTextID:901007")			// " TCP/IP"
#define T__TCPIP_SSL			_T("APTextID:901008")			// " TCP/IP SSL"
// #define T_ERROR_SCR901		_T("APTextID:901009")			// "ERROR"
#define T_UNDEFINED_ERROR_CODE	_T("APTextID:901010")			// "Undefined error code"
#define T_UNKNOWN_ERROR_CODE	_T("APTextID:901011")			// "Unknown error code"
// #define T_NORMAL				_T("APTextID:901012")			// "NORMAL"
// #define T_MISSING			_T("APTextID:901013")			// "MISSING"
// #define T_LOW				_T("APTextID:901014")			// "LOW"
// #define T_EMPTY				_T("APTextID:901015")			// "EMPTY"
// #define T_NO					_T("APTextID:901016")			// "NO"
// #define T_DOUBLE_MINUS		_T("APTextID:901017")			// "--"
// #define T_FULL				_T("APTextID:901018")			// "FULL"
#define T_MASTER				_T("APTextID:901019")			// "MASTER"
#define T_SERVICE				_T("APTextID:901020")			// "SERVICE"
#define T_OPERATOR				_T("APTextID:901021")			// "OPERATOR"
// end of [#2160]

#define T_QUICK_CINFIG			_T("APTextID:901037")			// "QUICK CONFIGURATION"		[#2267] US KSK 2014.05.12
#define T_RESET					_T("APTextID:901038")			// "RESET"		[#2267] US KSK 2014.05.12
#define T_ERRORCODE				_T("APTextID:901040")			// "ERROR CODE"	[#2267] US KSK 2014.05.12

/*------------------------------------------------------------------
Screen Number 905,906,907
-------------------------------------------------------------------*/
// [#651] NH KSK 2010.09.07
#define T_OPERATOR_PASSWORD		_T("APTextID:905004")			// OPERATOR PASSWORD
#define T_SERVICE_PASSWORD		_T("APTextID:906004")			// SERVICE PASSWORD
#define T_CURRENT_PASSWORD		_T("APTextID:907004")			// CURRENT PASSWORD
// end of [#651]

/*------------------------------------------------------------------
	Screen Number 907
-------------------------------------------------------------------*/
// [#154] NH KGS 2008.04.21 Password 변경 메뉴에서 안내문구 추가
#define T_INVALID_PASSWORD		_T("APTextID:907101")			// INVALID PASSWORD
#define T_VERIFICATION_ERROR	_T("APTextID:907102")			// VERIFICATION ERROR
// end of [#154]

// [#563] CA KSK 2009.8.20
/*------------------------------------------------------------------
	Screen Number 941
-------------------------------------------------------------------*/
#define T_STANDARD3_OPTION				_T("APTextID:941F6")	
#define T_STANDARD1_OPTION				_T("APTextID:941F61")
#define T_NORMAL_FEE_BUTTON_NAME		_T("APTextID:941F21")	        //[#2137] MX PCS 2012.07.26 "SURCHARGE MODE"
#define T_MEXICO_FEE_BUTTON_NAME		_T("APTextID:941F22")			//[#2137] MX PCS 2012.07.26 "SURCHARGE OWNER"
// end of [#563]

/*------------------------------------------------------------------
	Screen Number 946
-------------------------------------------------------------------*/
#define	T_SURCHARGEMODE_CANNOT_BE_CHANGED	_T("APTextID:946110")		// [#322] MX KSK 2008.6.23 "SURCHARGE MODE CAN NOT BE DISABLED"
/*------------------------------------------------------------------
	Screen Number 951
-------------------------------------------------------------------*/
#define T_MODEM_PARAMETER					_T("APTextID:951105")		
#define T_TERMINAL_IPADDRESS				_T("APTextID:951205")		

/*------------------------------------------------------------------
	Screen Number 954
-------------------------------------------------------------------*/
//#define T_ENGLISH_CANNOT_BE_DISABLED		_T("APTextID:954021")		// [#188] NH KGS 2008.05.08 English Disable 불가능 하도록 수정// [#2206] NH Justin 2013.06.28 Not use
#define T_CANNOT_BE_CHANGED					_T("APTextID:954023")		// [#302] MX KSK 2008.6.17 "LANGUAGE CAN NOT BE CHANGED"

/*------------------------------------------------------------------
Screen Number 959
-------------------------------------------------------------------*/
#define T_PERCENT_WARNING_TEXT1				_T("APTextID:959005")
#define T_PERCENT_WARNING_TEXT2				_T("APTextID:959006")

// [#416] NZ AIREAT 2008.09.08
/*------------------------------------------------------------------
	Screen Number 961
-------------------------------------------------------------------*/
// US / CA/ MX
#define T_TELEPHONE_NUMBER_AMERICA			_T("APTextID:961103")		// TELEPHONE NUMBER		[#416] NZ AIREAT
#define T_HOST_ADDRESS_AMERICA				_T("APTextID:961203")		// HOST ADDRESS			[#416] NZ AIREAT

// NZ/ AU
#define T_TELEPHONE_NUMBER					_T("APTextID:961101")		// TELEPHONE NUMBER		[#416] NZ AIREAT
#define T_HOST_ADDRESS						_T("APTextID:961201")		// HOST ADDRESS			[#416] NZ AIREAT
// end of [#416]

#define T_ROUTING_ID						_T("APTextID:961106")		////[#545] SOOK 2009.08.04 AU 사양 추가 
#define T_CONFIG_DOWNLOAD					_T("APTextID:961109")		//[#545] SOOK 2009.08.04 AU 사양 추가 

// [#RWC6-121] US William Remote Updates
#define T_SUNDAY			_T("APTextID:SUNDAY")
#define T_MONDAY			_T("APTextID:MONDAY")
#define T_TUESDAY			_T("APTextID:TUESDAY")
#define T_WEDNESDAY			_T("APTextID:WEDNESDAY")
#define T_THURSDAY			_T("APTextID:THURSDAY")
#define T_FRIDAY			_T("APTextID:FRIDAY")
#define T_SATURDAY			_T("APTextID:SATURDAY")

#define SUNDAY		0
#define MONDAY		1
#define TUESDAY		2
#define WEDNESDAY	3
#define THURSDAY	4
#define FRIDAY		5
#define SATURDAY	6

#define GET_STRING_FOR_DOW(OPTION) \
	( OPTION == SUNDAY		? T_SUNDAY :	\
	( OPTION == MONDAY		? T_MONDAY :	\
	( OPTION == TUESDAY		? T_TUESDAY :	\
	( OPTION == WEDNESDAY	? T_WEDNESDAY :	\
	( OPTION == THURSDAY	? T_THURSDAY :	\
	( OPTION == FRIDAY		? T_FRIDAY :	\
	( OPTION == SATURDAY	? T_SATURDAY:	T_SUNDAY )))))))

// End of [#RWC6-121]

// [#2160] CA KMK 2012.10.22 French OP, Screen 966, 976, 979
/*------------------------------------------------------------------
Screen Number 966, 976, 979
-------------------------------------------------------------------*/
/* 겹침
#define	T_ENABLE							_T("APTextID:966004")		// "ENABLE"
#define	T_DISABLE							_T("APTextID:966005")		// "DISABLE"

#define	T_DISABLE							_T("APTextID:976007")		// "DISABLE"
#define	T_ENABLE							_T("APTextID:976008")		// "ENABLE"
#define	T_VERSION_23						_T("APTextID:976009")		// "VERSION 23"
#define	T_VERSION_3							_T("APTextID:976010")		// "VERSION 3"

#define	T_DISABLE							_T("APTextID:979007")		// "DISABLE"
#define	T_ENABLE							_T("APTextID:979008")		// "ENABLE"
*/
// end of [#2160]
#define	T_SSL_CERTIFICATE_WARNING			_T("APTextID:976011")		// "We strongly recommend enabling certificate validation. SSL/TLS without certificate validation does not provide the full level of protection that SSL/TLS can provide."	// [#RWC608]

//#define	T_DENOMINATION_SELECT				_T("APTextID:981004")		// [#RWC6-12]


// [#2166] NH KSK 2012.11.13
/*------------------------------------------------------------------
Screen Number 992
-------------------------------------------------------------------*/
#define	T_SUPERVISOR_ENGLISH_NAME	_T("APTextID:992101")
#define	T_SUPERVISOR_FRENCH_NAME	_T("APTextID:992102")
// end of [#2166]

/*------------------------------------------------------------------
	Screen Number 993
-------------------------------------------------------------------*/
//#define	T_VDM_EXEC_MSG			_T("APTextID:993003")			// "Please Wait..."	// [#219] KSK 2008.5.20 사용 안함

/*------------------------------------------------------------------
Screen Number 821
-------------------------------------------------------------------*/
#define T_VDM_NHDIP_BUTTON_NAME		_T("APTextID:821F2")
#define T_VDM_MAGTEKDIP_BUTTON_NAME	_T("APTextID:821F21")

#define T_VDM_MODEM_BUTTON_NAME		_T("APTextID:821F7")
#define T_VDM_TCPIP_BUTTON_NAME		_T("APTextID:821F71")

// [#2160] CA KMK 2012.10.22 FrenchOP, Screen 821
//#define T_NORMAL_SCR821				_T("APTextID:821006")		// "N O R M A L"
//#define T_UNLIMIT					_T("APTextID:821007")		// "UNLIMIT"
// end of [#2160]
#define T_NORMAL_SCR821				_T("APTextID:821004")		// "N O R M A L"	[#2166] NH KSK 2012.12.21 검사 지적사항 대응
#define T_UNLIMIT					_T("APTextID:821005")		// "UNLIMIT"		[#2166] NH KSK 2012.12.21 검사 지적사항 대응


// [#541] AIREAT 2009.07.09 KEY MGR 통합.
/*------------------------------------------------------------------
	Screen Number 860
-------------------------------------------------------------------*/
#define T_KEYMGR_EXEC_MSG		_T("APTextID:860003")			// "Please Wait..."
#define T_TERMINATING_PIN		_T("APTextID:860004")			// "TERMINATING PIN"
// end of [#541]

// [#546] SOOK 2009.07.28 다기능 BIN LIST 기능 추가함
/*------------------------------------------------------------------
	Screen Number 858
-------------------------------------------------------------------*/
#define T_NO_BIN_ENTITY_MSG		_T("APTextID:858102")		//"THIS ATM HAS NO BIN ENTITY"
#define T_CARD_DECLINE_MSG		_T("APTextID:858103")		//"ALL CARD WILL BE DECLINED"
#define T_NO_BIN_DATA_MSG		_T("APTextID:858104")		//"NO BIN DATA" [#546] SOOK 2009.08.31
/*------------------------------------------------------------------
	Screen Number 869
-------------------------------------------------------------------*/
#define T_BIN_INVALID_MSG		_T("APTextID:855101")		//"INVALID BIN"
#define T_ACTION_INVALID_MSG	_T("APTextID:855102")		//"INVALID ACTION"
#define T_VALUE_INVALID_MSG		_T("APTextID:855103")		//"INVALID VALUE"
#define T_OVER_MAXIMUN_MSG		_T("APTextID:855104")		//"OVER THE MAXIMUM OF BIN COUNT"
#define T_BIN_NOT_FOUND_MSG		_T("APTextID:857101")		//"BIN NOT FOUND"
//end of [#546]

/*------------------------------------------------------------------
	Screen Number 736 [iTM] KSK 2010.01.22
-------------------------------------------------------------------*/
#define T_736002_MSG		_T("APTextID:736002")
#define T_736003_MSG		_T("APTextID:736003")
#define T_736004_MSG		_T("APTextID:736004")
#define T_736005_MSG		_T("APTextID:736005")
#define T_736006_MSG		_T("APTextID:736006")
#define T_736007_MSG		_T("APTextID:736007")
#define T_736008_MSG		_T("APTextID:736008")
#define T_736009_MSG		_T("APTextID:736009")
#define T_736010_MSG		_T("APTextID:736010")
#define T_736011_MSG		_T("APTextID:736011")
#define T_736012_MSG		_T("APTextID:736012")
#define T_736013_MSG		_T("APTextID:736013")
#define T_736014_MSG		_T("APTextID:736014")
#define T_736015_MSG		_T("APTextID:736015")
#define T_736016_MSG		_T("APTextID:736016")
#define T_736017_MSG		_T("APTextID:736017")
#define T_736018_MSG		_T("APTextID:736018")
#define T_736019_MSG		_T("APTextID:736019")
#define T_736020_MSG		_T("APTextID:736020")
#define T_736021_MSG		_T("APTextID:736021")
#define T_736022_MSG		_T("APTextID:736022")
#define T_736023_MSG		_T("APTextID:736023")
#define T_736024_MSG		_T("APTextID:736024")
#define T_736025_MSG		_T("APTextID:736025")
#define T_736026_MSG		_T("APTextID:736026")
#define T_736027_MSG		_T("APTextID:736027")
#define T_736028_MSG		_T("APTextID:736028")			
#define T_736040_MSG		_T("APTextID:736040")			// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
#define T_758001_MSG		_T("APTextID:758001")			// Do you want to remove?

//////////////////////////////////////////////////////////////////////////////////////////////////
//[#2130] MX PCS 2012.04.16
/*------------------------------------------------------------------
Screen Number 947
-------------------------------------------------------------------*/
#define	T_NORMAL_BIN_TITLE_NAME		_T("APTextID:947001")					// "BIN LIST"
//#define	T_BANSI_BIN_TITLE_NAME		_T("APTextID:947005")					// "BANSI BIN LIST" [#2147] Mx KSK 2012.08.23
// end of [#2130]
////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
//[#2130] MX PCS 2012.04.16 //[#2137] MX PCS 2012.07.26 " 아래의 리스트 더 이상 사용 안함."
/*------------------------------------------------------------------
Screen Number 941
-------------------------------------------------------------------*/
// [#2137] MX PCS 2012.07.25 " BIN LIST 제거 및 Surcharge Owner 이름 적용."
// #define	T_NORMAL_BIN_BUTTON_NAME	_T("APTextID:941F31")					// "BIN LIST"
// #define	T_BANSI_BIN_BUTTON_NAME		_T("APTextID:941F32")					// "BANSI BIN LIST"
// end of [#2130]
////////////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////////////////
/*------------------------------------------------------------------
Common Parts		[#2160] CA KMK 2012.11.26
-------------------------------------------------------------------*/
#define T_ENABLE			_T("APTextID:ENABLE")			// "ENABLE"
#define T_DISABLE			_T("APTextID:DISABLE")			// "DISABLE"
#define T_ENABLE_RCPT		_T("APTextID:ENABLE_RCPT")		// "ENABLE"		명세표용 특수문자 없는 문구
#define T_DISABLE_RCPT		_T("APTextID:DISABLE_RCPT")		// "DISABLE"	명세표용 특수문자 없는 문구

#define T_PREVIOUS			_T("APTextID:PREVIOUS")			// "PREVIOUS"	[#2539] US Justin 2018.03.29
#define T_NEXT				_T("APTextID:NEXT")				// "NEXT"		[#2539] US Justin 2018.03.29

// [#2160] CA KMK 2012.12.05
#define T_DIALUP			_T("APTextID:DIALUP")			// "DIALUP"
#define T_TCP_IP			_T("APTextID:TCP_IP")			// "TCP/IP"
#define T_TCP_IP_PAI		_T("APTextID:TCP_IP_PAI")		// "INTERNET"
#define T_GREATER			_T("APTextID:GREATER")			// "GREATER"
#define T_ENG_ONLY_GREATER	_T("APTextID:ENG_GREATER")		// "GREATER" [#2487] AU KSK 2017.06.08
#define T_LESSER			_T("APTextID:LESSER")			// "LESSER"
#define T_ENG_ONLY_LESSER	_T("APTextID:LESSER")			// "LESSER"  [#2487] AU KSK 2017.06.08
#define T_TYPE_A			_T("APTextID:TYPE A")			// "TYPE_A"
#define T_TYPE_B			_T("APTextID:TYPE B")			// "TYPE_B"
#define T_DAY_TOTAL			_T("APTextID:DAY TOTAL")		// "DAY TOTAL"
#define T_TRIAL_DAY_TOTAL	_T("APTextID:TRIAL DAY TOTAL")	// "TRIAL DAY TOTAL"
#define T_BEGINNING			_T("APTextID:BEGINNING")		// "BEGINNING"
#define T_AFTER_AMOUNT		_T("APTextID:AFTER AMOUNT")		// "AFTER AMOUNT"
#define T_ALWAYS			_T("APTextID:ALWAYS")			// "ALWAYS"
#define T_ONLY_TRANSACTION	_T("APTextID:ONLY TRANSACTION")	// "ONLY TRANSACTION"
#define T_ONLY_IDLE			_T("APTextID:ONLY IDLE")		// "ONLY IDLE"		// [#2333] NH KSK 2015.03.06
#define T_ON				_T("APTextID:ON")				// "ON"
#define T_OFF				_T("APTextID:OFF")				// "OFF"
#define T_ENHANCED_COUPON	_T("APTextID:ENHANCED COUPON")	// "ENHANCED COUPON"
#define T_BASIC_COUPON		_T("APTextID:BASIC COUPON")		// "BASIC COUPON"
#define T_NOT_USE			_T("APTextID:NOT USE")			// "NOT USE"
#define T_USE_TIME			_T("APTextID:USE TIME")			// "USE TIME"
#define T_NOT_DEFINED		_T("APTextID:NOT DEFINED")		// "NOT DEFINED"
#define T_AFTER_RECEIPT		_T("APTextID:AFTER RECEIPT")	// "AFTER RECEIPT"
#define T_HOST_DETERMINE	_T("APTextID:HOST DETERMINE")	// "HOST DETERMINE"
#define T_JAPANESE			_T("APTextID:JAPANESE")			// "JAPANESE"
#define T_KOREAN			_T("APTextID:KOREAN")			// "KOREAN"
#define T_CHINESE			_T("APTextID:CHINESE")			// "CHINESE"
#define T_FRENCH			_T("APTextID:FRENCH")			// "FRENCH"
#define T_SPANISH			_T("APTextID:SPANISH")			// "SPANISH"
#define T_ENGLISH			_T("APTextID:ENGLISH")			// "ENGLISH"
#define T_MS_FIRST			_T("APTextID:MS FIRST")			// "MS FIRST"
#define T_IC_FIRST			_T("APTextID:IC FIRST")			// "IC FIRST"
#define T_WITHDRAWAL_SURCHARGE	_T("APTextID:WITHDRAWAL SURCHARGE")		// "WITHDRAWAL SURCHARGE"
#define T_BALANCE_SURCHARGE	_T("APTextID:BALANCE SURCHARGE")	// "BALANCE SURCHARGE"
#define T_ACCEPT_CARD		_T("APTextID:ACCEPT CARD")		// "ACCEPT CARD"
#define T_DECLINE_CARD		_T("APTextID:DECLINE CARD")		// "DECLINE CARD"
#define T_AWARD_COUPON		_T("APTextID:AWARD COUPON")		// "AWARD COUPON"
#define T_PIN_CHANGE		_T("APTextID:PIN CHANGE")		// "PIN CHANGE"
#define T_BLOCK_SURCHARGE	_T("APTextID:BLOCK SURCHARGE")	// "BLOCK SURCHARGE"
#define T_COUNT				_T("APTextID:COUNT")			// "COUNT"
#define T_MONTHLY			_T("APTextID:MONTHLY")			// "MONTHLY"
#define T_DAILY				_T("APTextID:DAILY")			// "DAILY"
#define T_AFTER				_T("APTextID:AFTER")			// "AFTER"
// end of [#2160]

// [#2205] US KSK 2013.06.28
#define T_SOLID				_T("APTextID:SOLID")			// "SOLID"
#define T_FADE				_T("APTextID:FADE")				// "FADE"
#define T_FLICKING			_T("APTextID:FLICKING")			// "FLICKING"
#define T_CYAN				_T("APTextID:CYAN")				// "CYAN"
#define T_BLUE				_T("APTextID:BLUE")				// "BLUE"
#define T_YELLOWGREEN		_T("APTextID:YELLOWGREEN")		// "YELLOWGREEN"
#define T_GREEN				_T("APTextID:GREEN")			// "GREEN"
#define T_WHITE				_T("APTextID:WHITE")			// "WHITE"
#define T_PINK				_T("APTextID:PINK")				// "PINK"
// end of [#2205]

// [#2226] NH Justin 2013.10.23 Add Kernel Version
#define T_EMV_KERNEL_VER4	_T("APTextID:KERNEL VER4")		// "V4.0"
#define T_EMV_KERNEL_VER5	_T("APTextID:KERNEL VER5")		// "V5.5"
#define T_EMV_KERNEL_VER6	_T("APTextID:KERNEL VER6")		// "V6.0"	// [#2435] NH KSK 2016.07.14
#define T_EMV_KERNEL_VER7	_T("APTextID:KERNEL VER7")		// "V7.0"	// [#] US William 2019.09.05
// End of [#2226]

#define	T_PIGGYBACK _T("APTextID:PIGGY")	// "PIGGYBACK ONLY"
#define	T_INDI		_T("APTextID:INDI")		// "INDIVIDUAL ONLY"
#define	T_BOTH		_T("APTextID:BOTH")		// "BOTH"

// [#2354] US Justin 2015.07.01
#define	T_NOTICE_AMOUNT				_T("APTextID:NOTICE_AMOUNT")
#define	T_NOTICE_PERCENTAGE			_T("APTextID:NOTICE_PERCENTAGE")
#define	T_NOTICE_BOTH				_T("APTextID:NOTICE_BOTH")
// End of [#2354]

// [#2449] US Justin 2016.11.09
#define	T_CREDENTIAL_OK				_T("APTextID:CREDENTIAL_OK")
#define	T_CREDENTIAL_NG				_T("APTextID:CREDENTIAL_NG")
// End of [#2449]

// [#2472] US Justin 2017.02.09 DCC WITHDRAWAL OPTION
#define	T_DCC_WITHOPT_OFF			_T("APTextID:DCC_WITHOPT_OFF")
#define	T_DCC_WITHOPT_EMVMASTER		_T("APTextID:DCC_WITHOPT_EMVMASTER")
// End of [#2472]

// [#2481] US Justin 2017.04.21 Unknown IC Card Fallback Option
#define	T_UNKNOWNAID_FALLBACK		_T("APTextID:UNKNOWNAID_FALLBACK")
#define	T_UNKNOWNAID_MAGTR			_T("APTextID:UNKNOWNAID_MAGTR")
// End of [#2472]

// [#2496] US Justin 2017.08.17 Just.Cash Bitcoin
#define	T_JUSTCASH_CARDLESS			_T("APTextID:JUSTCASH_CARDLESS")
#define	T_JUSTCASH_DIGITALCUR		_T("APTextID:JUSTCASH_DIGITALCUR")
#define T_JUSTCASH_BOTH				_T("APTextID:JUSTCASH_BOTH")
// End of [#2496]

// [#2499] 
#define	T_USE_DUALHOST				_T("APTextID:USE_DUAL_HOST")
// #enf of [#2499]

// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Optioin
#define	T_REBOOT_ONTIME				_T("APTextID:REBOOT ON TIME")
#define	T_REBOOT_INTERVAL			_T("APTextID:REBOOT INTERVAL")
// ENd of [#2558]

#endif __NH_SCREEN_ID_DEF_H__
