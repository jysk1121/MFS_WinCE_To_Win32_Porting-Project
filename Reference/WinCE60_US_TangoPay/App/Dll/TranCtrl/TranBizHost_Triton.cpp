#include "stdafx.h"
#include ".\Tran\TranCmn.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#if (APP_CUSTOM_PAI == 1)
#  include ".\Common\LibertyXConfigurationManager.h"
#endif

#define	DBG_CALL		1
#define DBG_INFO		1

//------------------------------------------------------------------
//	Define Symbol
//------------------------------------------------------------------
// CDU STATUS MONITORING FIELD 부분 수정 / ERROR CODE도 수정 필요
// SMF 설정값 보완(CDU/CST상태)
//#define		TRITON_ERRORCODE_EMUL_COUNT	291		// [#514] [MX] KSK 2009.3.4 Multi Currency Error시 Triton Error Code 추가해야함 (확인사항)
//#define		TRITON_ERRORCODE_EMUL_COUNT	335		// [#2079] NH KSK 2011.07.15 Anti Skimming 및 Sankyo Error Code 추가
#define		TRITON_ERRORCODE_EMUL_COUNT	336			// [#2093] NH PCS 2011.10.28 "SSL Connection Failure 추가."

CString	Triton_ErrorCode_Tbl[TRITON_ERRORCODE_EMUL_COUNT][2] = 
{
	/* 0 ~ 4	*/	L"4000000", L"016", L"4001100", L"135", L"4001400", L"135", L"4001500", L"135", L"4001800", L"135",
	/* 5 ~ 9	*/	L"4001900", L"135", L"4001C00", L"135", L"4001D00", L"135", L"4002100", L"135", L"4002200", L"135",
	/* 10 ~ 14	*/	L"4002300", L"135", L"4002800", L"135", L"4002900", L"135", L"4002A00", L"135", L"4002B00", L"135",
	/* 15 ~ 19	*/	L"4003000",	L"042", L"4003100", L"044", L"4003200", L"044", L"4003300", L"053", L"4003400", L"035",
	/* 20 ~ 24	*/	L"4003600", L"134", L"4003700", L"035", L"4003800", L"052", L"4003900", L"044", L"4003A00", L"016",
	/* 25 ~ 29	*/	L"4003B00", L"135", L"4004000", L"033", L"4004100", L"048", L"4004200", L"038", L"4004300", L"048",
	/* 30 ~ 34	*/	L"4004400", L"048", L"4004500", L"039", L"4004600", L"055", L"4004700", L"191", L"4004900", L"142",
	/* 35 ~ 39	*/	L"4004A00", L"038", L"4004B00", L"048", L"4004D00", L"033", L"4004E00", L"033", L"4005100", L"142",
	/* 40 ~ 44	*/	L"4005200", L"034", L"4005400", L"055", L"4005500", L"037", L"4005600", L"045", L"4005900", L"054",
	/* 45 ~ 49	*/	L"4005B00", L"033", L"4005D00", L"035", L"4006000", L"135", L"4006100", L"135", L"4006200", L"035",
	/* 50 ~ 54	*/	L"4006300", L"035", L"4006A00", L"038", L"4006B00", L"135", L"4007000", L"135", L"4007200", L"135",
	/* 55 ~ 59	*/	L"4007300", L"035", L"4007A00", L"038", L"4007B00", L"135", L"4007C00", L"033", L"4007D00", L"033",
	/* 60 ~ 64	*/	L"4008000", L"135", L"4008100", L"135", L"4008200", L"035", L"4008F00", L"037", L"4009A00", L"038",
	/* 65 ~ 69	*/	L"4009D00", L"033", L"4009F00", L"033", L"400AC00", L"038", L"400C700", L"038", L"400C800", L"038",
	/* 70 ~ 74	*/	L"400C900", L"038", L"400D000", L"049", L"400D100", L"049", L"9712000", L"055", L"9712100", L"055",
	/* 75 ~ 79	*/	L"9712200", L"055", L"9712300", L"055", L"9712400", L"055", L"9712500", L"055", L"9712600", L"055",
	/* 80 ~ 84	*/	L"9719000", L"128", L"9719100", L"128", L"9719200", L"128", L"9719300", L"128", L"9719400", L"128",
	/* 85 ~ 89	*/	L"9719500", L"128", L"9719600", L"128", L"9719700", L"038", L"9719800", L"128", L"971A000", L"128",
	/* 90 ~ 94	*/	L"971A100", L"128", L"971A200", L"128", L"971A300", L"039", L"971A400", L"128", L"971A500", L"128",
	/* 95 ~ 99	*/	L"971B000", L"128", L"971C000", L"016", L"971DX00", L"128", L"9744700", L"191", L"9745B00" ,L"033",
	/* 100 ~ 104*/	L"9749F00", L"033", L"9747C00", L"033", L"9740010", L"129", L"9740012", L"129", L"9740016", L"129",
	/* 105 ~ 109*/	L"9740017", L"129", L"9740019", L"129", L"9740020", L"016", L"9740025", L"016", L"9740030", L"016",
	/* 110 ~ 114*/	L"9740031", L"129", L"97400FF", L"129", L"9740101", L"016", L"9740102", L"016", L"2010100", L"138",
	/* 115 ~ 119*/	L"2080100", L"138", L"2010200", L"138", L"2080200", L"138", L"2010300", L"141", L"2080300", L"141",
	/* 120 ~ 124*/	L"2010400", L"195", L"2080400", L"195", L"2010500", L"138", L"2080500", L"138", L"2010600", L"139",
	/* 125 ~ 129*/	L"2080600", L"139", L"2010700", L"183", L"2080700", L"183", L"2010800", L"139", L"2080800", L"139",
	/* 130 ~ 134*/	L"2010900", L"138", L"2080900", L"138", L"2010A00", L"139", L"2080A00", L"139", L"9720000", L"139",
	/* 135 ~ 139*/	L"2DN0000", L"140", L"9701010", L"206", L"9701012", L"203", L"9701016", L"203", L"9701017", L"203",
	/* 140 ~ 144*/	L"9701031", L"206", L"9701040", L"206", L"9701111", L"203", L"D000100", L"136", L"D000200", L"016",
	/* 145 ~ 149*/	L"D000300", L"016", L"D000400", L"000", L"D000500", L"000", L"D000600", L"000", L"D000700", L"000",
	/* 150 ~ 154*/	L"D000800", L"000", L"D000900", L"000", L"D001000", L"000", L"D001100", L"000", L"D001200", L"000",
	/* 155 ~ 159*/	L"D001300", L"000", L"D001400", L"000", L"D001500", L"000", L"D001600", L"000", L"D001700", L"000",
	/* 160 ~ 164*/	L"D001800", L"000", L"D001900", L"000", L"D002000", L"000", L"D002100", L"000", L"D002200", L"000",
	/* 165 ~ 169*/	L"D002300", L"000", L"D002400", L"000", L"D003900", L"000", L"D005100", L"000", L"D005200", L"000",
	/* 170 ~ 174*/	L"D005300", L"000", L"D005400", L"000", L"D005500", L"000", L"D005700", L"000", L"D005800", L"000",
	/* 175 ~ 179*/	L"D005900", L"000", L"D006100", L"000", L"D007500", L"000", L"D007800", L"000", L"D008000", L"000",
	/* 180 ~ 184*/	L"D008300", L"000", L"D008600", L"000", L"D009100", L"000", L"D009200", L"000", L"D009300", L"188",
	/* 185 ~ 189*/	L"D009400", L"192", L"D009500", L"192", L"D009600", L"192", L"D009700", L"004", L"D009800", L"004",
	/* 190 ~ 194*/	L"D009900", L"004", L"D009A00", L"004", L"D009B00", L"004", L"D009C00", L"004", L"D009D00", L"004",
	/* 195 ~ 199*/	L"D009E00", L"004", L"D009F00", L"004", L"D00A000", L"004", L"D00A100", L"004", L"D00A200", L"004",
	/* 200 ~ 204*/	L"D00A300", L"004", L"D00A400", L"004", L"D00A500", L"004", L"D00A600", L"004", L"D00A700", L"004",
	/* 205 ~ 209*/	L"D00A800", L"004", L"D00A900", L"004", L"D011100", L"000", L"D022200", L"000", L"D030000", L"192",
	/* 210 ~ 214*/	L"D100000", L"011", L"D110000", L"001", L"D120000", L"192", L"D130000", L"001", L"D150000", L"001",
	/* 215 ~ 219*/	L"D170000", L"192", L"D170100", L"192", L"D170200", L"192", L"D170200", L"192", L"D170300", L"192",
	/* 220 ~ 224*/	L"D170400", L"192", L"D170500", L"192", L"D170600", L"192", L"D170700", L"192", L"D170800", L"192",
	/* 225 ~ 229*/	L"D180000", L"006", L"D190000", L"006", L"D200000", L"007", L"D210000", L"136", L"D220000", L"015",
	/* 230 ~ 234*/	L"D320000", L"001", L"1103910", L"001", L"1106910", L"001", L"1101910", L"001", L"1105910", L"001",
	/* 235 ~ 239*/	L"1102910", L"001", L"1102920", L"001", L"2000400", L"001", L"2001300", L"001", L"2001400", L"138",
	/* 240 ~ 244*/	L"2001500", L"183", L"C101000", L"128", L"F000100", L"384", L"2000100", L"156", L"2000200", L"384",
	/* 245 ~ 249*/	L"2000300", L"048", L"POWERAC", L"001", L"POWERBA", L"001", L"POWERAB", L"001", L"991@@91", L"001",
	/* 250 ~ 254*/	L"9799499", L"128", L"9799901", L"001", L"9799902", L"001", L"9799903", L"001", L"9799904", L"001",
	/* 255 ~ 259*/	L"9799905", L"001", L"9799907", L"001", L"9799908", L"001", L"9799301", L"196", L"9799601", L"001",
	/* 260 ~ 264*/	L"8217091", L"001", L"8218091", L"001", L"9791100", L"001", L"9791200", L"001", L"9791300", L"001",
	/* 265 ~ 269*/	L"9791400", L"001", L"9791500", L"001", L"9791600", L"001", L"9791800", L"001", L"9791A00", L"001",
	/* 270 ~ 274*/	L"9791B00", L"001", L"9792100", L"001", L"9792200", L"001", L"9792300", L"001", L"9792400", L"001",
	/* 275 ~ 279*/	L"9792500", L"001", L"9792600", L"001", L"9792800", L"001", L"9792A00", L"001", L"9792B00", L"001",
	/* 280 ~ 284*/	L"1030100", L"001", L"9742800", L"033", L"D00C100", L"000", L"D00C200", L"000", L"D410000", L"000",
	/* 285 ~ 289*/	L"D410100", L"000", L"971A600", L"000", L"971A700", L"000", L"FFFFFFF", L"000", L"F00FF00", L"148",
	/* 290 ~ 294*/	L"F007F00", L"000", L"3109500", L"196", L"3109600", L"196", L"9723010", L"196", L"9723012", L"196",	// [#2079] Sankyo / Anti Skimming Error Code 추가
	/* 295 ~ 299*/	L"9723013", L"196", L"9723019", L"196", L"9723040", L"196", L"9723048", L"196", L"9723058", L"196",
	/* 300 ~ 304*/	L"9311200", L"196", L"9312300", L"196", L"9313200", L"196", L"9313300", L"196", L"9341100", L"196",
	/* 305 ~ 309*/	L"9341300", L"196", L"93A5600", L"196", L"93A9500", L"196", L"3100100", L"196", L"3100200", L"196",
	/* 310 ~ 314*/	L"3100400", L"196", L"3101000", L"196", L"3102000", L"196", L"3102100", L"196", L"3102200", L"196",
	/* 315 ~ 319*/	L"3102300", L"196", L"3102400", L"196", L"3102500", L"196", L"3102700", L"196", L"3103000", L"196",
	/* 320 ~ 324*/	L"3106000", L"196", L"3106100", L"196", L"3106200", L"196", L"3106300", L"196", L"3106400", L"196",
	/* 325 ~ 329*/	L"3106500", L"196", L"3106600", L"196", L"3106900", L"196", L"3107000", L"196", L"3107100", L"196",
	/* 330 ~ 334*/	L"3107200", L"196", L"3107600", L"196", L"3109500", L"196", L"3109600", L"196", L"3109900", L"196",		// end of [#2079]
	/* 335 ~ 348*/	L"D150100", L"001" // [#2093] NH PCS 2011.10.28 "SSL Connection Failure."	
};

//------------------------------------------------------------------
//	Implement
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD3_SendHost()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD3_SendHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_SendHost]\n"));

	////////////////////////////////////////////////////////////////////////////
	//	Transaction Result Initialize
	m_pDevCmn->TranResult = FALSE;								// Host Result
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, FALSE);

	// [#12] NH PSC 2008.03.24 reson for reversal 초기값을 0으로 셋팅한다.	// [#397] NH PSC 2008.12.10 Standard1 protocol도 EMV 거래시 'n' 필드 추가
	// 1: Incorrect dispense, 2: Protocol error, 3: Lost eot, 4: Mac error, 
	// 5: Customer cancellation, 6: VAS challenge/response error, 7: ICC declined transaction
	if(TranCode != TC_REVERSAL)		// [#68] UK JSW 2008.05.16 수정
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 2);

// [#2137] MX PCS 2012.07.19
#if (MX_VERSION)
	if (TranCode == TC_INQUIRY || TranCode == TC_WITHDRAWAL || TranCode == TC_TDL_DCC_TRANSACTION)
	{
		if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
		{
			if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG) == MX_1ST_SEND)	// 첫번째 전문송신에서 카운트 증가.
				m_pDevCmn->fnAPL_AddSerialNo(m_HostConfig);											// Add Serial No
		}
		else
		{
			m_pDevCmn->fnAPL_AddSerialNo(m_HostConfig);												// Add Serial No
		}
	}
#else
	/////////////////////////////////////////////////////////////////////////////
	// [#2350] US Justin 2015.06.19 Add POP MONEY
	// [#2448] US Justin 2016.11.21 Add Just.Cash
	// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
	// [#2515] US Justin 2017.11.20 PIN4 Prestaging => Not increase Serial Number, it is increased at PIN4 Prestaging
	if (TranCode == TC_WITHDRAWAL			|| TranCode == TC_INQUIRY	|| TranCode == TC_TRANSFER	||
		TranCode == TC_TDL_DCC_TRANSACTION	|| TranCode == TC_POPMONEY	|| TranCode == TC_JUSTCASH	||
		TranCode == TC_JUSTCASH_BITCOIN		|| TranCode == TC_B4U		|| TranCode == TC_DIGITALMINT)
	{
		m_pDevCmn->fnAPL_AddSerialNo(m_HostConfig); // Add Serial No
	}
#endif
// End of [#2137]

	/////////////////////////////////////////////////////////////////////////////
	BIZ_STD3_MakeHostMsg();						// Send Host Make

	m_strTritonPrtMsgArray.RemoveAll();							// V01.02.28 'p' Field 초기화 추가	[#22] KSK 2008.03.14

	// [#2220] NH KMK 2014.01.27 APP_SHOW_MODE 관련 로직 미사용하므로 주석처리
// #ifdef APP_SHOW_MODE	// Local Mode에서 거래속도 향상을 위해 수정
// 	m_pDevCmn->fstrSCR_WaitTime(2);
// #endif
	// end of [#2220]

#ifdef APP_LOCAL_MODE
	m_pDevCmn->fstrSCR_WaitTime(1);		// KSK 2010.02.27 공장용시에는 Delay 주면 안됨 (추후 Delay 삭제 필요)
	m_pDevCmn->TranResult = TRUE;
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);
	return RES_OK;
#endif

	// [#2135] NH KSK 2012.05.07 Pin Block or Mac Result가 SPACE시에 거래 취소 후 저널 저장하도록 수정
	// PIN Block(16)이 SPACE 이거나 Mac Result(8)가 SPACE인 경우 거래 취소
	if (m_sUserSelection.strPassword.Left(16) == L"                " || m_strMacSendResult.Left(8) == L"        ")
	{
		// PIN 장애 발생 및 PIN SP 재기동 하도록 Flag 설정
		if (m_pDevCmn->nPINErrorFlag == WORKINGKEY_INIT)
			m_pDevCmn->nPINErrorFlag = PIN_MAC_BLANK_ERROR;

		m_pDevCmn->fnAPL_StackError(_T("9799904"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003006), DEV_PIN);
		return RES_HOST_SEND_ERR;
	}
	// end of [#2135]

	if (BIZ_SendData() != RES_OK)
	{
		// EOT 미수신시 Configuration / Inquiry / Transfer / Extended Configuration 에서는 정상처리함
		// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
		// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
		if (TranCode == TC_OPEN				|| TranCode == TC_INQUIRY	|| TranCode == TC_TRANSFER	||
			TranCode == TC_JUSTCASH_BITCOIN	|| TranCode == TC_PIN4		|| TranCode == TC_B4U		||
			TranCode == TC_DIGITALMINT)
		{
			if (m_pDevCmn->fstrNET_GetErrorCode().Left(5) == L"D2200")
				return RES_OK; // KSK 2009.8.24
		}

		// KSK 2010.03.13 위치 변경 EMV 거래 중 D2200시에 정상 처리해야함
		// EMV Default Approval Process because of Host Send/Receive Error
		// Withdrawal and Inquiry Transacion
		//////////////////////////////////////////////////////////////////////////
		if(m_pDevCmn->fnMCU_IsEmvTransaction())		// [#397] [NH] psc 2008.08.01 for EMV transaction
		{
			// [#2365] US Justin 2015.07.24 Add DCC+ on Canadian AP
			// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
			if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_TDL_DCC_TRANSACTION || TranCode == TC_JUSTCASH_BITCOIN || TranCode == TC_DIGITALMINT)
			{
				if (m_bOnlineProcessing == FALSE)
				{
					// KSK 2010.08.29 통신 장애인 경우 Default 처리 추가
					m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

					if (m_pDevCmn->fnEMV_Trans_OnlineProcess(0, AAC) == EMV_RSLT_OK)
					{
						m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
						m_pDevCmn->fnEMV_Trans_Completion();
					}

					BIZ_EMV_MakeICDataforReversal();	// KSK 2009.8.31 송신 에러시에 

					m_bOnlineProcessing = TRUE;
				}
			}
		}

		if (m_pDevCmn->TranStatus == TRAN_TRAN || m_pDevCmn->TranStatus == TRAN_REVERSAL)
		{
			if ((m_pDevCmn->fstrNET_GetErrorCode().Left(5) == L"D1704") || (m_pDevCmn->fstrNET_GetErrorCode().Left(5) == L"D1706"))
			{
				if (LIB_IsReversalCondition())				// [#439] NH AIREAT 2008.10.21
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);					// REVERSAL FLAG SET
			}
			else if (m_pDevCmn->fstrNET_GetErrorCode().Left(5) == L"D2200")
			{
				if (LIB_IsReversalCondition())				// [#439] NH AIREAT 2008.10.21
				{
					if (BIZ_RecvData() == RES_OK)
						BIZ_STD3_AnalHostData();

					// REVERSAL FLAG SET
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
				}
			}

			m_pDevCmn->fnAPL_StackError(m_pDevCmn->fstrNET_GetErrorCode(), m_pDevCmn->fstrNET_GetErrorMsg(), DEV_NET);	// [#419] [NH] KSK 2008.9.16
			return RES_HOST_SEND_ERR;
		}
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_MakeHostMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD3_MakeHostMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_MakeHostMsg]\n"));

	////////////////////////////////////////////
	// 1. Make Message
	if (m_HostConfig == HC_LIBERTYX) // [#RWC6-59] US William 2019.10.09 LibertyX
	{
		BIZ_STD3_MakeHeader_LibertyX();
	}
	else
	{
		BIZ_STD3_MakeHeader();			// Send Host Make Header
	}

	BIZ_STD3_MakeBody();			// Send Host Make Message

	////////////////////////////////////////////
	// 2. Macing
	// [#4] NH PSC 2008.03.10 Mac result 계산결과 전문에 추가
	// keymode: 5, 6, 7, 11 - 현재 standard 2는 TMAC 없음.
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	// [#2350] US Justin 2015.06.19 Add POP MOney
	// [#2365] US Justin 2015.07.24 Add DCC+ in Canadian AP
	// [#2415] US Justin 2016.04.15 Add WalPay
	// [#2448] US Justin 2016.11.21 Add Just.Cash
	// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
	// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
	if((MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING	|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)			&&
		(MemGetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_MACFLAG) == 1))
	{
		if (TranCode == TC_INQUIRY				|| TranCode == TC_TRANSFER			|| TranCode == TC_WITHDRAWAL		||
			TranCode == TC_REVERSAL				|| TranCode == TC_TOTAL				|| TranCode == TC_TRIALTOTAL		||
			TranCode == TC_POPMONEY				|| TranCode == TC_TDL_TIRSURCHARGE	|| TranCode == TC_TDL_DCC_LOOKUP	||
			TranCode == TC_TDL_DCC_TRANSACTION	|| TranCode == TC_TDL_DCC_REVERSAL	|| TranCode == TC_JUSTCASH			||
			TranCode == TC_JUSTCASH_BITCOIN		|| TranCode == TC_PIN4				|| TranCode == TC_DIGITALMINT)
		{
			memset(m_szTemp, NULL, sizeof(m_szTemp));
			WideToMulti(m_szTemp, m_strSendData, m_strSendData.GetLength());
			WORD tempLen = m_strSendData.GetLength();

			// VISA TYPE은 STX와 ETX를 붙여서 MAC Result를 계산한다.
			if((MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE) == VISA_FRAMED_TCPIP)	||		// if(VSISA TYPE: 1) else (STATDARD: 2 or ACK_CONTROLED: 3)
				(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP) )	// if Dialup
			{
				memmove(&m_szTemp[1], &m_szTemp[0], tempLen);
				m_szTemp[0] = 0x02;
				m_szTemp[tempLen+1] = 0x03;
				tempLen += 2;
			}
			else	// (type: 2 or 3)
			{
				memmove(&m_szTemp[2], &m_szTemp[0], tempLen);
				m_szTemp[0] = (tempLen& 0xFF00) >> 8;
				m_szTemp[1] = tempLen & 0x00FF;
				tempLen += 2;
			}
			m_pDevCmn->fnPIN_MacingData(MakeUnPack(m_szTemp, tempLen));		// KSK 2010.04.12 MakeUnPack함수에서 1KB가 넘어갈 경우 1KB만 처리되는 Bug가 있음 (추후 Logic 변경 필요)

			// [#2135] NH KSK 2012.05.07 Mac Result가 SPACE인 경우 PIN SP 재기동 하도록 보완
//			CString MacData;
//			MacData = m_pDevCmn->fstrPIN_GetMacingData();
			m_strMacSendResult.Format(L"%8.8s", m_pDevCmn->fstrPIN_GetMacingData());
			// end of [#2135]

			CString strDefaultMAC;
			int retMacKeyFind = -1;
			strDefaultMAC.Format(L"%c^000000000%c", FIELD_DELIMITER, FIELD_DELIMITER);
			retMacKeyFind = m_strSendData.Find(strDefaultMAC);

			// [#2135] NH KSK 2012.05.07
			if(retMacKeyFind != -1)
			{
				// FS + ^ 뒤부터 MAC값을 넣는다.
//				m_strSendData.SetAt(retMacKeyFind+2, MacData.GetAt(0));
//				m_strSendData.SetAt(retMacKeyFind+3, MacData.GetAt(1));
//				m_strSendData.SetAt(retMacKeyFind+4, MacData.GetAt(2));
//				m_strSendData.SetAt(retMacKeyFind+5, MacData.GetAt(3));
//				m_strSendData.SetAt(retMacKeyFind+6, L' ');
//				m_strSendData.SetAt(retMacKeyFind+7, MacData.GetAt(4));
//				m_strSendData.SetAt(retMacKeyFind+8, MacData.GetAt(5));
//				m_strSendData.SetAt(retMacKeyFind+9, MacData.GetAt(6));
//				m_strSendData.SetAt(retMacKeyFind+10, MacData.GetAt(7));
				m_strSendData.SetAt(retMacKeyFind+2, m_strMacSendResult.GetAt(0));
				m_strSendData.SetAt(retMacKeyFind+3, m_strMacSendResult.GetAt(1));
				m_strSendData.SetAt(retMacKeyFind+4, m_strMacSendResult.GetAt(2));
				m_strSendData.SetAt(retMacKeyFind+5, m_strMacSendResult.GetAt(3));
				m_strSendData.SetAt(retMacKeyFind+6, L' ');
				m_strSendData.SetAt(retMacKeyFind+7, m_strMacSendResult.GetAt(4));
				m_strSendData.SetAt(retMacKeyFind+8, m_strMacSendResult.GetAt(5));
				m_strSendData.SetAt(retMacKeyFind+9, m_strMacSendResult.GetAt(6));
				m_strSendData.SetAt(retMacKeyFind+10, m_strMacSendResult.GetAt(7));
			}
			// end of [#2007]
			// end of [#2135]
		}
	}
	// end of [#4]

	// [#4] NH PSC 2008.03.10 Make CRC result
	if(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_CRCENABLE) == ENABLE)		// CRC option is Enabled
	{
		// [#2007] NH KSK 2010.11.29 송신시에 ub0000이 올 수 있으므로 CRC Check 조건 변경 (WINCE5.0에 수정된 내용 적용)
		// CRC check
//		int retCrcKeyFind = m_strSendData.Find(L"ub0000");
		CString strCRCDefault;
		int retCrcKeyFind = -1;

		strCRCDefault.Format(L"%cub0000%c", FIELD_DELIMITER, FIELD_DELIMITER);	
		retCrcKeyFind = m_strSendData.Find(strCRCDefault);			
		// end of [#2007]

		if(retCrcKeyFind != -1)
		{
			char temp_snddata[2048] = "";
			sprintf(temp_snddata, "%S", m_strSendData);
			WORD tempLen = strlen(temp_snddata);
			// VISA TYPE을 제외한 나머지 프로토콜은 Length 2바이트를 앞에 붙여서 CRC 계산을 한다.
			if(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE) != VISA_FRAMED_TCPIP	&&		// if(VSISA TYPE: 1) else (STATDARD: 2 or ACK_CONTROLED: 3)
				MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_TCPIP)	
			{
				memmove(&temp_snddata[2], &temp_snddata[0], tempLen);
				temp_snddata[0] = (tempLen& 0xFF00) >> 8;
				temp_snddata[1] = tempLen & 0x00FF;
				tempLen += 2;
			}
			UINT crc = crc16((BYTE*)temp_snddata, tempLen);

			CString	temp = _T("");

			// [#2007] NH KSK 2010.11.29
//			temp.Format(L"ub%04X", crc);
//			m_strSendData.Replace(L"ub0000", temp);
			temp.Format(L"%cub%04X%c", FIELD_DELIMITER, crc, FIELD_DELIMITER);
			m_strSendData.Replace(strCRCDefault, temp);
			// end of [#2007]
		}
	}
	// end of [#4]

	////////////////////////////////////////////
	// 3. Make Buffer
	memset(m_szTemp, NULL, sizeof(m_szTemp));
	WideToMulti(m_szTemp, m_strSendData, sizeof(m_szTemp));

	////////////////////////////////////////////
	// Initialize Send Buffer
	m_nSendLength = 0;												// Send Length
	// [#2012] NH KJW 2011.01.19 Lottery 기능지원을 위해 버퍼크기 변경 10K
	//memset(m_arSendBuffer, 0, sizeof(m_arSendBuffer));				// Send Buffer
	memset(m_arSendBuffer, 0, NETBUF_SEND_SIZE);
	// end of [#2012]

	// [#2012] NH KJW 2011.02.09
	//memcpy(m_arSendBuffer, m_szTemp, m_strSendData.GetLength());
	//m_nSendLength = m_strSendData.GetLength();
	m_nSendLength = __min( NETBUF_SEND_SIZE, m_strSendData.GetLength() );
	memcpy(m_arSendBuffer, m_szTemp, m_nSendLength);
	// end of [#2012]

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_MakeHeader()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD3_MakeHeader()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_MakeHeader]\n"));

	CString strTmp;

	if ( MemGetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE) == 1) 
	{
		// 1. Communications Identifier 8 ASCII
		m_sSTD3_CommHeader.m_strCommsHeaderID_8 = MemGetStr(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_COMMUNICATIONID).Left(8);
		m_sSTD3_CommHeader.m_strCommsHeaderID_8 += "        ";
		m_sSTD3_CommHeader.m_strCommsHeaderID_8 = m_sSTD3_CommHeader.m_strCommsHeaderID_8.Left(8);

		// 2. Terminal Identifier 2 ACSII
		m_sSTD3_CommHeader.m_strTerminalID_2 = "td";		// Fixed Field

		// 3. Software Version Number 2자리 효성체계의 마지막 2자리를 넣음
		strTmp = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_AP_VERSION);	// [#127] KSK 2008.04.21
		m_sSTD3_CommHeader.m_strSoftwareVersionNo_2 = strTmp.Right(2);

		// 4. Encryption Mode Flag
		// keymode: 3, 4, 7, 11
		// [#554] KSK 2009.08.10 m_KeyMode삭제
		if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_TDES || 
			MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_TDES		|| 
			MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING	|| 
			MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)
		{
			m_sSTD3_CommHeader.m_strEncryptionModeFlag_1 = "2";
		}
		else if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TR31			||
				 MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TR31_MACING	||
				 MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TR31_TMACING)
		{
			m_sSTD3_CommHeader.m_strEncryptionModeFlag_1 = "3";
		}
		else
		{
			m_sSTD3_CommHeader.m_strEncryptionModeFlag_1 = "0";
		}

		// 5. Information Header 7 ACSII , Unused Must be space filled.
		m_sSTD3_CommHeader.m_strInformationHeader_7 = "       ";

		m_strSendData = m_sSTD3_CommHeader.m_strCommsHeaderID_8 +
						m_sSTD3_CommHeader.m_strTerminalID_2 +
						m_sSTD3_CommHeader.m_strSoftwareVersionNo_2 +
						m_sSTD3_CommHeader.m_strEncryptionModeFlag_1 +
						m_sSTD3_CommHeader.m_strInformationHeader_7;
		m_strSendData += FIELD_DELIMITER;
	}

	strTmp = MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID);
	strTmp += "               ";
	m_sSTD3_CommHeader.m_strTerminalID_15.Format(L"%15.15s", strTmp);

	m_strSendData += m_sSTD3_CommHeader.m_strTerminalID_15;
	m_strSendData += FIELD_DELIMITER;

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_MakeHeader_LibertyX()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Makes a header for the LibertyX HostConfig
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD3_MakeHeader_LibertyX()
{
#if (APP_LIBERTYX)
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_MakeHeader_LibertyX]\n"));

	CString strTmp;
	strTmp = m_LXConfig.LocationID;
	strTmp += "               ";
	m_sSTD3_CommHeader.m_strTerminalID_15.Format(L"%15.15s", strTmp);

	m_strSendData += m_sSTD3_CommHeader.m_strTerminalID_15;
	m_strSendData += FIELD_DELIMITER;
#endif
	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_MakeBody()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD3_MakeBody()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_MakeBody]\n"));

	switch (TranCode)
	{
		case TC_INQUIRY:
		case TC_TRANSFER:
		case TC_WITHDRAWAL:
		case TC_POPMONEY:			// [#2350] US Justin 2015.06.19 Add POP Money
		case TC_JUSTCASH:			// [#2448] US Justin 2016.11.21 Add Just.Cash
		case TC_JUSTCASH_BITCOIN:	// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
		case TC_PIN4:				// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
		case TC_DIGITALMINT:
			BIZ_STD3_MakeTransMsg();
			break;
		case TC_REVERSAL:
			BIZ_STD3_MakeReversalMsg();
			break;
		case TC_TOTAL:
		case TC_TRIALTOTAL:
			BIZ_STD3_MakeTotalMsg();
			break;
		case TC_OPEN:
		case TC_HEALTHCHK:
			BIZ_STD3_MakeConfigMsg();
			break;
		// [#2292] US Justin 2014.09.25 Add TDL FLOW
		case TC_TDL_TIRSURCHARGE:
		case TC_TDL_DCC_LOOKUP:
		case TC_TDL_DCC_TRANSACTION:
		case TC_TDL_DCC_REVERSAL:
			BIZ_STD3_MakeTDLMsg();
			break;
		// End of [#2292]
		default:
			break;
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_MakeTransMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD3_MakeTransMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_MakeTransMsg]\n"));

	BYTE			szTrancode[3] = { 0, };
	CString			strTemp;
	int				nAmount;

	switch (TranCode)
	{
		case TC_WITHDRAWAL:
		case TC_POPMONEY:			// [#2350] US Justin 2015.06.18 Add POP MONEY
		case TC_JUSTCASH:			// [#2448] US Justin 2016.11.21 Add Just.Cash
		case TC_PIN4:				// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
			szTrancode[0] = '1';
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE, TRANTYPE_WITHDRAWAL);
			break;
		case TC_TRANSFER:
			szTrancode[0] = '2';
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE, TRANTYPE_TRANSFER);
			break;
		case TC_INQUIRY:
			szTrancode[0] = '3';
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE, TRANTYPE_INQUIRY);
			break;
		case TC_JUSTCASH_BITCOIN:	// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
		case TC_DIGITALMINT:
			szTrancode[0] = '4';
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE, TRANTYPE_NONCASH_WITHDRAW);
			break;
		default:
			break;
	}

	// [#RWC6-59] US William 2019.10.09 LibertyXs
	if ( TranCode == TC_WITHDRAWAL &&
		(m_HostConfig == HC_LIBERTYX || m_HostConfig == HC_LTX_HOST_DEP_MODE) )
	{
		// LibertyX only is a non-cash withdrawal
		szTrancode[0] = '4';
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE, TRANTYPE_NONCASH_WITHDRAW);

		// Set LTX source account to Checking
		m_SourceAccount = S_CHECKING;
	}

	// Source Account
	if (m_SourceAccount == S_CHECKING){
		szTrancode[1] = '1';
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSFROMACCOUNT, L"CA");
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTOACCOUNT, L"SA");
	}
	else if (m_SourceAccount == S_SAVINGS){
		szTrancode[1] = '2';
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSFROMACCOUNT, L"SA");
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTOACCOUNT, L"CA");
	}
	else if (m_SourceAccount == S_CREDITCARD){
		szTrancode[1] = '5';
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSFROMACCOUNT, L"CR");
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTOACCOUNT, L"CA");
	}

	m_sSTD3_TranReq.m_strTransactionCode_2.Format(L"%2S", (char*)szTrancode);
	m_strSendData += m_sSTD3_TranReq.m_strTransactionCode_2;
	m_strSendData += FIELD_DELIMITER;

	// Sequence Number 
	m_sSTD3_TranReq.m_strSequenceNumber_4.Format(L"%4.4s", m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig));	// [#371] [US] KSK 2008.7.16 ATMINFO쪽 값을 참조하도록 수정
	m_strSendData += m_sSTD3_TranReq.m_strSequenceNumber_4;
	m_strSendData += FIELD_DELIMITER;

	// ISO CARD Track2 37 Numeric ( Variable )
	if (m_sCardData.strISO2Data.GetLength() > 0)
		m_sSTD3_TranReq.m_strTrack2_37 = m_sCardData.strISO2Data;
	// [#459] [NH] KSK 2008.11.26
	// 카드에서 읽은 만큼만 Host로 전송하도록 수정
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO2DATA, m_sSTD3_TranReq.m_strTrack2_37);
	m_strSendData += m_sSTD3_TranReq.m_strTrack2_37;
	m_strSendData += FIELD_DELIMITER;

	// Amount 1  8Numeric ( Withdraw or transfer amount )
	nAmount = Asc2Int(m_sUserSelection.strMoney);

	m_sSTD3_TranReq.m_strAmount1_8.Format(L"%8.8d", nAmount);
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT,	m_sSTD3_TranReq.m_strAmount1_8);	// [#169] [NH] KSK 2008.04.25

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	m_strSendData += m_sSTD3_TranReq.m_strAmount1_8;
	m_strSendData += FIELD_DELIMITER;

	// Amount 2  8Numeric ( Surcharge amount as set by terminal. )
#if ( AU_VERSION)	// [#2069] NH KSK 2011.06.13
	m_sSTD3_TranReq.m_strAmount2_8.Format(L"%8.8d", SurchargeAmount);
#else
	if (MemGetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEDISPLAY))
	{
		// [#2150] US Justin 2012.10.02 Put Displayed Surcharge Amount
		m_sSTD3_TranReq.m_strAmount2_8.Format(L"%8.8d", m_nDisplayedSurchargeAmount);
		// End of [#2150]
	}
	else
	{
		m_sSTD3_TranReq.m_strAmount2_8.Format(L"%S","00000000");
	}
#endif

	// save percent value
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT, m_sSTD3_TranReq.m_strAmount2_8);				// [#304] NH AIREAT 08.06.15 - Reversal Surcharge 버그 수정.

	m_strSendData += m_sSTD3_TranReq.m_strAmount2_8;
	m_strSendData += FIELD_DELIMITER;

	/************************************************************************/
	/* Make PIN Block                                                       */
	/************************************************************************/
	if (m_sUserSelection.strPassword.IsEmpty())		// [#11] NH KSK 2010.10.5	[#2021] US KSK 2011.02.14
	{
#if (MX_VERSION)	 // [#2137] MX KSK 2012.07.25 1st Call에서만 Pin Block 생성을 하고 2nd Call에서는 1st에서 만든 PIN Block을 그대로 사용함
		if ((m_pDevCmn->m_strCurrencyID == CURRENCY_TYPE) || 
			((m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE) && (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG) == MX_1ST_SEND)))
		{
			/* Make PIN Block */
			// [#2150] US Justin 2012.10.04 Make Common function...
			/*
			// [#554] KSK 2009.08.11 512K 지원
			//if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) >= 0)
			{
				// Key Mode 1, 5
				if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_DDES ||
					MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING)
					m_pDevCmn->m_pMwi->PinBuildPinBlock(m_sCardData.strAccountNo,  NULL, 15, _T("ISO0"), SINGLEDESKEY_WORKINGKEY_NAME, NULL);
				else 
					if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_TDES ||
						MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING	||
						MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)
						m_pDevCmn->m_pMwi->PinBuildPinBlock(m_sCardData.strAccountNo,  NULL, 15, _T("ISO0"), TRIPLEDESKEY_WORKINGKEY_NAME, NULL);
			}
			else
			{
				m_pDevCmn->m_pMwi->PinBuildPinBlock(m_sCardData.strAccountNo,  NULL, 15, _T("ISO0"), NULL, NULL);
			}
			// end of [#554]
			*/
			m_pDevCmn->fnAPL_BuildPinBlockWithAccountNo(m_sCardData.strAccountNo);
			// End of [#2150]

			/* Check Build Pin Block Command */
			m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);

			/* Get PIN Block */
			m_sUserSelection.strPassword = m_pDevCmn->fstrPIN_GetPinKeyData();
		}
#else
		/* Make PIN Block */

		// [#RWC6-59] US William 2019.10.09 LibertyX
		// Change master keys depending on the host configuration
		int nMasterKeyName = MASTERKEY_ATM;
		if (m_HostConfig == HC_LIBERTYX)
		{
			nMasterKeyName = MASTERKEY_LIBERTYX;
		}

		m_pDevCmn->fnAPL_BuildPinBlockWithAccountNo(m_sCardData.strAccountNo, nMasterKeyName);
		// End of [#2150]

		/* Check Build Pin Block Command */
		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);

		// [#2135] NH KSK 2012.05.07 Pin Block Result가 SPACE인 경우 PIN SP 재기동 하도록 보완
		/* Get PIN Block */
//		m_sUserSelection.strPassword = m_pDevCmn->fstrPIN_GetPinKeyData();
		m_sUserSelection.strPassword.Format(L"%16.16s", m_pDevCmn->fstrPIN_GetPinKeyData());
		// end of [#2135]
#endif	// end of [#2137]
	}

	// PIN Block  16 Alphanumeric	(Variable)
	m_sSTD3_TranReq.m_strPINBlock_16.Format(L"%16.16s", m_sUserSelection.strPassword.Left(16));
	m_strSendData += m_sSTD3_TranReq.m_strPINBlock_16;
	m_strSendData += FIELD_DELIMITER;

	// Miscellaneous 1
	// Triton_Make_Miscellaneous();
	m_strSendData += FIELD_DELIMITER;

	// Miscellaneous 2
	// Triton_Make_Miscellaneous();
	m_strSendData += FIELD_DELIMITER;

	// Status Monitoring Field
	if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_STATUSMONITORINGENABLE) == 1)
	{
		BIZ_STD3_Make_StatusMonitoringField();
		m_strSendData += FIELD_DELIMITER;
	}

	// Make MiscellaneousField
	BIZ_STD3_Make_MiscellaneousField();

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_MakeReversalMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD3_MakeReversalMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_MakeReversalMsg]\n"));

	CString		strTemp = L"";
	int			nAmount = 0;

	// Transaction Code  Reversal : 29
	m_sSTD3_ReversalReq.m_strTransactionCode_2 = L"29";
	m_strSendData += m_sSTD3_ReversalReq.m_strTransactionCode_2;
	m_strSendData += FIELD_DELIMITER;

	// Sequence Number 4Numeric
	m_sSTD3_ReversalReq.m_strSequenceNumber_4.Format(L"%4.4s", m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig));	// [#371] [US] KSK 2008.7.16 ATMINFO쪽 값을 참조하도록 수정
	m_strSendData += m_sSTD3_ReversalReq.m_strSequenceNumber_4;
	m_strSendData += FIELD_DELIMITER;

	// ISO CARD Track2 37 Numeric ( Variable )
	m_sSTD3_ReversalReq.m_strTrack2_37 = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO2DATA);
	m_strSendData += m_sSTD3_ReversalReq.m_strTrack2_37;
	m_strSendData += FIELD_DELIMITER;

	// Amount 1  8Numeric (Requested Withdraw amount)
	m_sSTD3_ReversalReq.m_strAmount1_8 = ZERO8;
	m_sSTD3_ReversalReq.m_strAmount1_8 += MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT);	// [#169] [NH] KSK 2008.04.25
	m_sSTD3_ReversalReq.m_strAmount1_8 = m_sSTD3_ReversalReq.m_strAmount1_8.Right(8);
	m_strSendData += m_sSTD3_ReversalReq.m_strAmount1_8;
	m_strSendData += FIELD_DELIMITER;

	// Amount 2  8Numeric (Surcharge amount as set by terminal)
	strTemp.Format(L"%s", MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT));
	nAmount = Asc2Int(strTemp);
	m_sSTD3_ReversalReq.m_strAmount2_8.Format(L"%8.8d",nAmount);
	m_strSendData += m_sSTD3_ReversalReq.m_strAmount2_8;
	m_strSendData += FIELD_DELIMITER;

	// Amount 3  8Numeric (Actual Dispensed amount)
	m_sSTD3_ReversalReq.m_strAmount3_8 = ZERO8;
	m_sSTD3_ReversalReq.m_strAmount3_8 += MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT);
	m_sSTD3_ReversalReq.m_strAmount3_8 = m_sSTD3_ReversalReq.m_strAmount3_8.Right(8);
	m_strSendData += m_sSTD3_ReversalReq.m_strAmount3_8;
	m_strSendData += FIELD_DELIMITER;

	// Status Monitoring Field
	if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_STATUSMONITORINGENABLE) == 1 && 
		m_pDevCmn->AtmStatus != ATM_INIT)		// [#258] NZ AIREAT 2008.06.05 - POWER OFF REVERSAL 일 경우 SMF 보내지 않는다.
	{
		BIZ_STD3_Make_StatusMonitoringField();
		m_strSendData += FIELD_DELIMITER;
	}

	// Make MiscellaneousField
	BIZ_STD3_Make_MiscellaneousField();

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_MakeTotalMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD3_MakeTotalMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_MakeTotalMsg]\n"));

	CString			strTemp = L"";

	// Transaction Code : Day Total("51") Trial Day Total("50")
	if( TranCode == TC_TOTAL )
		m_sSTD3_TotalReq.m_strTransactionCode_2 = L"51";
	else
		m_sSTD3_TotalReq.m_strTransactionCode_2 = L"50";

	m_strSendData = m_strSendData + m_sSTD3_TotalReq.m_strTransactionCode_2;
	m_strSendData += FIELD_DELIMITER;
	// Status Monitoring Field
	if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_STATUSMONITORINGENABLE) == 1)
	{
		BIZ_STD3_Make_StatusMonitoringField();
		m_strSendData += FIELD_DELIMITER;
	}

	strTemp.Format(L"%ld", MemGetTotal(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_COUNT, 0));
	strTemp = ZERO4 + strTemp;
	strTemp = strTemp.Right(4);
	m_sSTD3_TotalReq.m_strTotalWithdrawals_4 = strTemp;
	m_strSendData = m_strSendData + m_sSTD3_TotalReq.m_strTotalWithdrawals_4;

	// Number of Inquiries 4 Numeric
	strTemp.Format(L"%ld", MemGetTotal(_MEM_FLD_INI_INQTOTALINFO, _MEM_VAR_SUM_INQ_COUNT, 0));

	strTemp = ZERO4 + strTemp;
	strTemp = strTemp.Right(4);
	m_sSTD3_TotalReq.m_strTotalInquiries_4 = strTemp;
	m_strSendData = m_strSendData + m_sSTD3_TotalReq.m_strTotalInquiries_4;

	// Number of Transfers 4 Numeric
	strTemp.Format(L"%ld", MemGetTotal(_MEM_FLD_INI_TRANSTOTALINFO, _MEM_VAR_SUM_TRANS_COUNT, 0)); // String Format
	strTemp = ZERO4 + strTemp;
	strTemp = strTemp.Right(4);
	m_sSTD3_TotalReq.m_strTotalTransfers_4 = strTemp;
	m_strSendData = m_strSendData + m_sSTD3_TotalReq.m_strTotalTransfers_4;

	// Settlement 12 Numeric ( Total amount of all withdrawals. )
	strTemp.Format(L"%ld", MemGetTotal(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_AMOUNT, 0)*100); //2008-01-15 V01.02.25 SRC-4
	strTemp = ZERO12 + strTemp;
	strTemp = strTemp.Right(12);
	m_sSTD3_TotalReq.m_strSettlement_12 = strTemp;
	m_strSendData = m_strSendData + m_sSTD3_TotalReq.m_strSettlement_12;
	m_strSendData += FIELD_DELIMITER;

	// Make MiscellaneousField
	BIZ_STD3_Make_MiscellaneousField();

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_MakeConfigMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD3_MakeConfigMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_MakeConfigMsg]\n"));

	// 7. Transaction Code : "60" Configuration Table Download Request Message
	m_sSTD3_ConfigReq.m_strTransactionCode_2 = "60";
	m_strSendData = m_strSendData + m_sSTD3_ConfigReq.m_strTransactionCode_2;
	m_strSendData += FIELD_DELIMITER;

	if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_STATUSMONITORINGENABLE) == 1)
	{
		BIZ_STD3_Make_StatusMonitoringField();
		m_strSendData += FIELD_DELIMITER;
	}

	// Make MiscellaneousField
	BIZ_STD3_Make_MiscellaneousField();

	return RES_OK;
}

// [#2292] US Justin 2014.09.25 Add TDL
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_MakeTDLMsg()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD3_MakeTDLMsg()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_MakeTDLMsg]\n"));
	CString strTmp;

	///////////////////////////////////////////////////
	///////////////////////////////////////////////////
	// Assign Values for TDL request
	///////////////////////////////////////////////////
	///////////////////////////////////////////////////
	if (TranCode == TC_TDL_TIRSURCHARGE)
	{
		if (m_sUserSelection.strPassword.IsEmpty() )
		{
			m_pDevCmn->fnAPL_DeviceEnDisable(DEV_PIN, DISABLE);
			Delay_Msg(50);
			m_pDevCmn->fnAPL_BuildPinBlockWithAccountNo(m_sCardData.strAccountNo);
			m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);
			m_sUserSelection.strPassword.Format(L"%16.16s", m_pDevCmn->fstrPIN_GetPinKeyData());
		}
	}
	else if(TranCode==TC_TDL_DCC_LOOKUP)
	{
		// PAN (Personal Account Number)
		strTmp = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2);
		int nLoc = strTmp.Find(L"=");
		if (nLoc < 0)		m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_PAN = strTmp;
		else				m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_PAN = strTmp.Left(nLoc);

		// Local Currency Code (int)
		strTmp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID);
		if(strTmp == L"MXN")		m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_LocalCurrency = L"484";
		else if(strTmp == L"CAD")	m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_LocalCurrency = L"124";
		else						m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_LocalCurrency = L"840";	// Default USD

		// Requested Amount (Double)
		m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_RequestAmount.Format(L"%0.2f", 0.01*Asc2Int(m_sUserSelection.strMoney) );

		// Selected Language
		int nSavedLangMode = m_pDevCmn->fnSCR_GetCurrentLangMode();
		if(nSavedLangMode==SPN_MODE)		m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_Language = _T("Spanish");
		else if(nSavedLangMode==FRN_MODE)	m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_Language = _T("French");
		else if(nSavedLangMode==CHN_MODE)	m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_Language = _T("Chinese");
		else if(nSavedLangMode==KOR_MODE)	m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_Language = _T("Korean");
		else if(nSavedLangMode==JPN_MODE)	m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_Language = _T("Japanese");
		else								m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_Language = _T("English");
	}
	else if (TranCode==TC_TDL_DCC_TRANSACTION)
	{
		// Account
		if (m_SourceAccount == S_SAVINGS)			m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_Account	= _T("2");
		else if (m_SourceAccount == S_CREDITCARD)	m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_Account	= _T("3");
		else										m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_Account	= _T("1");

		m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_RequestAmount		= m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_RequestAmount;
		// Set on Transaction Flow ////////////////////////////
		// m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_OfferAcceptance		=> Withdrawal Proc;	
		// m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_SurchargeAmount		=> DCC Offer Screen;
		// m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_ConvertedAmount		=> DCC Offer Screen;
		// m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_ConvertedSurcharge	=> DCC Offer Screen;
		// m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_PresentedAmount		=> DCC Offer Screen;
		m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_HomeCurrencyCode		= m_STD3_TDL_Data.m_DCC_Lookup.m_strRes_DCCOfferHomeCurrencyCode;
		m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_LocalCurrency		= m_STD3_TDL_Data.m_DCC_Lookup.m_strReq_LocalCurrency;
		m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_Track2				= MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2);

		m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_ivaTaxAmount							= m_STD3_TDL_Data.m_DCC_Lookup.m_ivaTaxAmount;
		m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_surchargePlusIvaTaxAmount			= m_STD3_TDL_Data.m_DCC_Lookup.m_surchargePlusIvaTaxAmount;
		m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_convertedIvaTaxAmount				= m_STD3_TDL_Data.m_DCC_Lookup.m_convertedIvaTaxAmount;
		m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_convertedSurchargePlusIvaTaxAmount	= m_STD3_TDL_Data.m_DCC_Lookup.m_convertedSurchargePlusIvaTaxAmount;

		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX, m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_ivaTaxAmount);

		// PIN BLOCK 
		if (m_sUserSelection.strPassword.IsEmpty() )
		{
			m_pDevCmn->fnAPL_DeviceEnDisable(DEV_PIN, DISABLE);
			Delay_Msg(50);
			m_pDevCmn->fnAPL_BuildPinBlockWithAccountNo(m_sCardData.strAccountNo);
			m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);
			m_sUserSelection.strPassword.Format(L"%16.16s", m_pDevCmn->fstrPIN_GetPinKeyData());
		}
		m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_PINBlock.Format(L"%s", m_sUserSelection.strPassword.Left(16));

		// EMV Data
		CString strEMVData = _T(" ");
		if(m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			TranCode = TC_WITHDRAWAL;	// The following EMV Logic Chieck Transaction Type
			BIZ_EMV_MakeICDataforTrans(MSG_TRITON_TYPE, &strEMVData);
			TranCode = TC_TDL_DCC_TRANSACTION;
		}
		else
		{
			if (m_pDevCmn->fnMCU_IsEmvEnable())
			{
				strTmp = m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_Track2;
				int nResult = strTmp.Find(L"=");
				if( (strTmp.GetLength() > nResult + 5) && (nResult != -1) )
				{
					//if  (strTmp.GetAt(nResult+5) == '2' || strTmp.GetAt(nResult+5) == '6')		// EMV CARD CHECK
					//if( (m_pDevCmn->m_bUnKnownAID_SkipPE==FALSE) && (strTmp.GetAt(nResult+5) == '2' || strTmp.GetAt(nResult+5) == '6') )		// [#2436] US Justin 2016.07.22
					if( (m_pDevCmn->m_bUnKnownAID_SkipPE==FALSE) && LIB_IsICCard(strTmp) )		// [#2517]
					{
						strEMVData = L"9F390192";												// Fall Back

						// [#2519] NH Justin 2017.12.08 Leave Fallback Indicator in the journal
						CString strOtherMsg = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG);
						if(strOtherMsg.Find(L"F.BACK") < 0)
						{
							if(strOtherMsg.GetLength()>0)
								strOtherMsg += CString(UNIT_DELIMITER);

							CString sNewOtherMsg = L"";
							sNewOtherMsg.Format(L"%s%sF.BACK", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_GENERAL_COMMENT);
							strOtherMsg += sNewOtherMsg;
							MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strOtherMsg );
						}
						// End of [#2519]
					}
				}
			}
		}
		m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_EMVData				= strEMVData;	

		// Fill Legacy Parameters
		{
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE, TRANTYPE_DCC);
			// Transaction Code
			if( m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_Account == L"1")				
			{
				m_sSTD3_TranReq.m_strTransactionCode_2 = L"11";
				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSFROMACCOUNT, L"CA");
			}
			else if( m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_Account == L"2")		
			{
				m_sSTD3_TranReq.m_strTransactionCode_2 = L"12";
				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSFROMACCOUNT, L"SA");
			}
			else
			{
				m_sSTD3_TranReq.m_strTransactionCode_2 = L"15";
				MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSFROMACCOUNT, L"CR");
			}
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTOACCOUNT, L"CA");
			// Sequence Number
			m_sSTD3_TranReq.m_strSequenceNumber_4.Format(L"%4.4s", m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig));
			// Track2
			m_sSTD3_TranReq.m_strTrack2_37 = m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_Track2;
			MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO2DATA, m_sSTD3_TranReq.m_strTrack2_37);
			// Amount 1 Requested Amount (Cent)
			m_sSTD3_TranReq.m_strAmount1_8.Format(L"%08d", Dollar2Cent(m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_RequestAmount) );
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT,	m_sSTD3_TranReq.m_strAmount1_8);
			// Amount 2 Surcharge
			m_sSTD3_TranReq.m_strAmount2_8.Format(L"%08d",Dollar2Cent(m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_SurchargeAmount) );
		}
	}
	else if (TranCode==TC_TDL_DCC_REVERSAL)
	{
		// Reversal Reason
		if(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) >= 1 && 	MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 11)
			m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_ReversalReason.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL));
		// Dispensed Amount
		m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_DispensedAmount.Format(L"%0.2f",  0.01 * Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT)) );
		m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_OfferAcceptance			= m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_OfferAcceptance;
		m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_RequestAmount			= m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_RequestAmount;
		m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_LocalCurrency			= m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_LocalCurrency;
		m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_SurchargeAmount			= m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_SurchargeAmount;
		m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_ivaTaxAmount			= m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_ivaTaxAmount;
		m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_PresentedAmount			= m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_PresentedAmount;
		m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_Track2					= m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_Track2;

		// EMV Data
		CString strEMVData = _T(" ");
		if(m_pDevCmn->fnMCU_IsEmvTransaction())
		{
			TranCode = TC_REVERSAL;	// The following EMV Logic Chieck Transaction Type
			BIZ_EMV_MakeICDataforTrans(MSG_TRITON_TYPE, &strEMVData);
			TranCode = TC_TDL_DCC_REVERSAL;
		}
		else
		{
			if (m_pDevCmn->fnMCU_IsEmvEnable())
			{
				strTmp = m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_Track2;
				int nResult = strTmp.Find(L"=");
				if( (strTmp.GetLength() > nResult + 5) && (nResult != -1) )
				{
					//if (strTmp.GetAt(nResult+5) == '2' || strTmp.GetAt(nResult+5) == '6')		// EMV CARD CHECK
					//if( (m_pDevCmn->m_bUnKnownAID_SkipPE==FALSE) && (strTmp.GetAt(nResult+5) == '2' || strTmp.GetAt(nResult+5) == '6') )		// [#2436] US Justin 2016.07.22
					if( (m_pDevCmn->m_bUnKnownAID_SkipPE==FALSE) && LIB_IsICCard(strTmp) )		// [#2517]
					{
						strEMVData = L"9F390192";												// Fall Back

						// [#2519] NH Justin 2017.12.08 Leave Fallback Indicator in the journal
						CString strOtherMsg = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG);
						if(strOtherMsg.Find(L"F.BACK") < 0)
						{
							if(strOtherMsg.GetLength()>0)
								strOtherMsg += CString(UNIT_DELIMITER);

							CString sNewOtherMsg = L"";
							sNewOtherMsg.Format(L"%s%sF.BACK", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_GENERAL_COMMENT);
							strOtherMsg += sNewOtherMsg;
							MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strOtherMsg );
						}
						// End of [#2519]					
					}
				}
			}
		}
		m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_EMVData					= strEMVData;

		// Fill Legacy Parameters
		{
			// Transaction Code  Reversal : 29
			m_sSTD3_ReversalReq.m_strTransactionCode_2 = L"29";
			// Sequence Number 4Numeric
			m_sSTD3_ReversalReq.m_strSequenceNumber_4.Format(L"%4.4s", m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig));
			// ISO CARD Track2 37 Numeric ( Variable )
			m_sSTD3_ReversalReq.m_strTrack2_37 = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO2DATA);
			// Amount 1  8Numeric (Requested Withdraw amount)
			m_sSTD3_ReversalReq.m_strAmount1_8.Format(L"%08d", Dollar2Cent(m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_RequestAmount) );
			// Amount 2  8Numeric (Surcharge amount as set by terminal)
			m_sSTD3_ReversalReq.m_strAmount2_8.Format(L"%08d", Dollar2Cent(m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_SurchargeAmount) );
			// Amount 3  8Numeric (Actual Dispensed amount)
			m_sSTD3_ReversalReq.m_strAmount3_8.Format(L"%08d", Dollar2Cent(m_STD3_TDL_Data.m_DCC_Reversal.m_strReq_DispensedAmount) );
		}
	}

	m_STD3_TDL_Data.m_bTranResult = FALSE;		// Transaction result
	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	// MAKING Message
	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////

	m_strSendData += L"97";
	m_strSendData += FIELD_DELIMITER;

	// Sequence Number
	m_strSendData += L"    ";
	m_strSendData += FIELD_DELIMITER;

	// Track 2 Data
	if (TranCode == TC_TDL_TIRSURCHARGE)
		m_strSendData += m_sCardData.strISO2Data;
	else								// [#2340] US Justin 2015.04.24 Cardtronics DCC+ Patch. Fill with 37 spaces for Track2 data.
		m_strSendData += L"                                     ";
	m_strSendData += FIELD_DELIMITER;

	// Amount 1
	m_strSendData += L"        ";
	m_strSendData += FIELD_DELIMITER;
	// Amount 2
	m_strSendData += L"        ";
	m_strSendData += FIELD_DELIMITER;

	// Pin Block
	if (TranCode == TC_TDL_TIRSURCHARGE)	m_strSendData += m_sUserSelection.strPassword;
	else									m_strSendData += L"                ";
	m_strSendData += FIELD_DELIMITER;

	// [#2340] US Justin 2015.04.24 Cardtronics DCC+ Patch. Add FS to match Triton unit and Add ub field
	m_strSendData += FIELD_DELIMITER;		
	m_strSendData += FIELD_DELIMITER;

	// [#2365] US Justin 2015.07.24 Add DCC+ on Canadian AP
	if(((MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING || 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			|| 
		MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING))		&&
		MemGetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_MACFLAG, 0) == 1)
	{
		m_strSendData += L"^000000000";
		m_strSendData += FIELD_DELIMITER;
	}
	// End of [#2365]

	if(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_CRCENABLE) == ENABLE)		// CRC option is Enabled
	{
		m_strSendData += L"ub0000";
		m_strSendData += FIELD_DELIMITER;
	}
	// End of [#2340]

	// TDL TAG
	m_strSendData += m_STD3_TDL_Data.MakeTDLTag(TranCode, MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID), m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig));
	m_strSendData += FIELD_DELIMITER;

	// [#2365] US Justin 2015.07.24 Add DCC+ for Canadia AP
	/*
	// [#2340] US Justin 2015.04.24 Cardtronics DCC+ Patch. Add FS to match Triton unit and Add ub field
	//m_strSendData += FIELD_DELIMITER;		
	if(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_CRCENABLE) == ENABLE)		// CRC option is Enabled
	{
		CString strCRCDefault;
		strCRCDefault.Format(L"%cub0000%c", FIELD_DELIMITER, FIELD_DELIMITER);	
		int retCrcKeyFind = m_strSendData.Find(strCRCDefault);			

		if(retCrcKeyFind != -1)
		{
			char temp_snddata[4096] = "";
			sprintf(temp_snddata, "%S", m_strSendData);
			WORD tempLen = strlen(temp_snddata);
			// VISA TYPE을 제외한 나머지 프로토콜은 Length 2바이트를 앞에 붙여서 CRC 계산을 한다.
			if(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE) != VISA_FRAMED_TCPIP && MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == 2)	// if(VSISA TYPE: 1) else (STATDARD: 2 or ACK_CONTROLED: 3)
			{
				memmove(&temp_snddata[2], &temp_snddata[0], tempLen);
				temp_snddata[0] = (tempLen& 0xFF00) >> 8;
				temp_snddata[1] = tempLen & 0x00FF;
				tempLen += 2;
			}
			UINT crc = crc16((BYTE*)temp_snddata, tempLen);

			CString	temp = _T("");
			temp.Format(L"%cub%04X%c", FIELD_DELIMITER, crc, FIELD_DELIMITER);
			m_strSendData.Replace(strCRCDefault, temp);
		}
	}
	// ENd of [#2340]
	*/ 
	// End of [#2365]

	return RES_OK;
}
// End of [#2292]

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_Make_MiscellaneousField()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD3_Make_MiscellaneousField()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_Make_MiscellaneousField]\n"));

	int nMisellIndex, i;
	BYTE chMicellaneous;
	CString strTmp("");
	UINT nMiscellaneous_FID2 = 0,nTmp = 0;

	switch (TranCode)
	{
		case TC_INQUIRY:
		case TC_TRANSFER:
		case TC_WITHDRAWAL:			// Transaction Message
		case TC_POPMONEY:			// [#2350] US Justin 2015.06.18 Add POP MONEY
		case TC_JUSTCASH:			// [#2448] US Justin 2016.11.21 Add Just.Cash
		case TC_JUSTCASH_BITCOIN:	// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
		case TC_PIN4:				// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
		case TC_DIGITALMINT:
			nMisellIndex = MISCELLANEOUS_REQ_TRAN;
			break;
		case TC_REVERSAL:			// Reversal Message
			nMisellIndex = MISCELLANEOUS_REQ_REVERSAL;
			break;
		case TC_TOTAL:				// HOST Total Message
		case TC_TRIALTOTAL:
			nMisellIndex = MISCELLANEOUS_REQ_TOTAL;
			break;
		case TC_OPEN:				// Configuration Message
		case TC_HEALTHCHK:
			nMisellIndex = MISCELLANEOUS_REQ_CONFIG;
			break;
		default:
			return RES_NG;
	}

#if SUPPORT_CHANGE_PARAMETER_JNL		//[#610] SOOK 2010.01.14 Configuration 저널 저장 
	m_pDevCmn->SetPreviousParameter();		//memory leak 때문에 변경 사항만 따로 저장하는 것으로 수정함. 
#endif									//end of [#610]

	for( i = 0; chMiscellaneous_Requests[nMisellIndex][i] != 0; i++)
	{
		chMicellaneous = (BYTE)chMiscellaneous_Requests[nMisellIndex][i];
		switch( chMicellaneous ) 
		{
		case '#':
			break;

		// 28 Digit Account Number(i.e. Checking,Savings,etc) Left justified, padded with spaces.
		case '&':
			break;

		// PIN Change PIN Block. 16 alphanumeric. ANSI standard encrypted PIN block.
		case '@':
			break;

		// Terminal is Triple-DES capable ( 2: Terminal is Triple-DES, '[2' )
		case '[':	// [#4] NH PSC 2008.03.10 DES capability, if receive 2 working keys then '2' else '1'
			switch (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE))
			{
				// keymode: 3, 4, 7, 11
				case KEYMODE_NON_UNIQ_TDES:
				case KEYMODE_UNIQ_TDES:
				case KEYMODE_TDES_MACING:
				case KEYMODE_TDES_TMACING:
					if ((m_bFirstKeyReceived == TRUE) && (m_bSecondKeyReceived == TRUE))
						m_strSendData += L"[2";
					else
						m_strSendData += L"[0";

					m_strSendData += FIELD_DELIMITER;
					break;
			}
			break;

		// MAC(Message Authentication Code) Result. 9ASCII characters	^A423 34CD
		case '^':
			// [#4] NH PSC 2008.03.10 MAC result의 임시값을 채운다. 전문이 다 만들어진 후에 실제 MAC result 계산.
			// keymode: 5, 6, 7, 11 and MAC flag is true
			if(((MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING || 
				MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING		|| 
				MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING			|| 
				MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING))		&&
				MemGetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_MACFLAG, 0) == 1)
			{
				m_strSendData += L"^000000000";
				m_strSendData += FIELD_DELIMITER;
			}
			break;

		case 'a':
			break;

		case 'e':
			break;

		case 'g':
			break;

		case 'i':
			break;
		case 'j':
			break;
		case 'k':
			// [#66] NH PSC 2008.04.07 'k' field 추가
			if(Asc2Int(MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_APP_ISSUERFEE)) == 0)
				break;
			else
			{
				CString strIssuerFee;
				strIssuerFee.Format(L"k%09d", MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_APP_ISSUERFEE));
				m_strSendData += strIssuerFee;
				m_strSendData += FIELD_DELIMITER;
			}
			break;
			// end of [#66]
		case 'l':
			break;

		case 'n':
			// [#12] NH PSC 2008.03.24 reason for reversal 값 셋팅.
			// 1: Incorrect dispense, 2: Protocol error, 3: Lost eot, 4: Mac error, 
			// 5: Customer cancellation, 6: VAS challenge/response error, 7: ICC declined transaction
			if(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) >= 1 && 
				MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 11)		// KSK 2010.01.04 7->11 변경
			{
				CString strResonforReversal;
				strResonforReversal.Format(L"n%02d", MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL));
				m_strSendData += strResonforReversal;
				m_strSendData += FIELD_DELIMITER;
			}
			// end of [#12]
			break;
		case 'o':
			break;

#if (MX_VERSION)	// [#2015] MX, KSK 2012.02.02 qbA ~ qbD 필드 추가 (WINCE5.0 Porting
					// [#2137] MX, PCS 2012.06.26 qbE~  qdG 필드 추가. 
		case 'q':
			nMiscellaneous_FID2 = chMiscellaneous_Requests[nMisellIndex][i];
			nMiscellaneous_FID2 &= 0xFFFFFF00;

			// 'b' 
			if ((nMiscellaneous_FID2 & FID2_b) == FID2_b)
			{

				if(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE).GetLength() > 0)
				{
					strTmp.Format(L"%s",MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE));

					m_strSendData += L"qbA "; 
					m_strSendData += strTmp;
					m_strSendData += FIELD_DELIMITER;
				}
				if(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE).GetLength() > 0)
				{
					// [#2147] MX KSK 2012.08.18 USD 방출모드에서는 Reversal시에만 qbB를 송신하도록 수정
					if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
					{
						if (TranCode == TC_REVERSAL)
						{
							strTmp.Format(L"%s",MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));

							m_strSendData += L"qbB "; 
							m_strSendData += strTmp;
							m_strSendData += FIELD_DELIMITER;
						}
					}
					else
					{
						strTmp.Format(L"%s",MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));

						m_strSendData += L"qbB "; 
						m_strSendData += strTmp;
						m_strSendData += FIELD_DELIMITER;
					}
					// end of [#2147]
				}
				if(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX).GetLength() > 0)
				{
					strTmp.Format(L"%s",MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX));

					m_strSendData += L"qbC "; 
					m_strSendData += strTmp;
					m_strSendData += FIELD_DELIMITER;
				}

				// [#2147] MX KSK 2012.08.18 Host에서 수신받은 qbE, qbF,qbG 송신하도록 수정 (USD 방출모드시에만)
				if ((TranCode == TC_REVERSAL) && (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE))
				{
					if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE).GetLength() > 0)
					{
						int nDispensedAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));
						int nRequestedAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));

						if ((nDispensedAmt != 0) && (nDispensedAmt != nRequestedAmt))
							strTmp.Format(L"%s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_REVERSAL_EXCHANGE_FEE));
						else
							strTmp.Format(L"%s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE));

						m_strSendData += L"qbE"; 
						m_strSendData += strTmp;
						m_strSendData += FIELD_DELIMITER;
					}
					if (MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE).GetLength() > 0)
					{
						strTmp.Format(L"%s",MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE));

						m_strSendData += L"qbG"; 
						m_strSendData += strTmp;
						m_strSendData += FIELD_DELIMITER;
					}
				}
				// end of [###1]
			}
			break;
#endif			// end of [#2015]

		case 'u': // Two character FIDs
			// Make the second FID
			nMiscellaneous_FID2 = chMiscellaneous_Requests[nMisellIndex][i];
			nMiscellaneous_FID2 &= 0xFFFFFF00;

			// 'b' 16bit CRC 4 ASCII
			if ( (nMiscellaneous_FID2 & FID2_b) == FID2_b )
			{
				// [#4] NH PSC 2008.03.10 CRC 처리추가, CRC필드 default값 처리, 실제 CRC계산은 나중에한다.
				if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_CRCENABLE) == ENABLE)
				{
					m_strSendData += L"ub0000";
					m_strSendData += FIELD_DELIMITER;
				}
			}
			// 'd' Encoded EMV data block ASCII characers. Variable length.
			if ( (nMiscellaneous_FID2 & FID2_d) == FID2_d )
			{
				if(m_pDevCmn->fnMCU_IsEmvTransaction())
				{
					BIZ_EMV_MakeICDataforTrans(MSG_TRITON_TYPE);		// [#2237] US Justin 2013.12.05 Dual Host DCC EMV Field Separator Bug Fix
				}
				else
				{
					// MS or FallBack 거래
					if (m_pDevCmn->fnMCU_IsEmvEnable())
					{

						// [#2116] NH KSK 2012.03.05 Power Off Reversal시 Track2 Data 감지가 안되어 NVRAM에서 Read하도록 수정함
						// Service Code가 "2" or "6"인 경우 POS Entry Mode 전송하도록 함
						// Check IC Card Information
						//int nResult = m_sCardData.strISO2Data.Find(L"=");	// "="이 없는 경우는 위쪽에서 return함	
						//if (m_sCardData.strISO2Data.GetAt(nResult+5) == '2' || m_sCardData.strISO2Data.GetAt(nResult+5) == '6')			// EMV CARD CHECK

						strTmp = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2);
						int nResult = strTmp.Find(L"=");

						// [#2137] MX KSK 2012.07.27 Track2 Data가 잘못된 경우 예외처리 추가
						if (strTmp.GetLength() <= nResult + 5)
							break;
						// end of [#2137]

						if (nResult != -1)
						{
							//if (strTmp.GetAt(nResult+5) == '2' || strTmp.GetAt(nResult+5) == '6')			// EMV CARD CHECK
							//if( (m_pDevCmn->m_bUnKnownAID_SkipPE==FALSE) && (strTmp.GetAt(nResult+5) == '2' || strTmp.GetAt(nResult+5) == '6') )		// [#2436] US Justin 2016.07.22
							if( (m_pDevCmn->m_bUnKnownAID_SkipPE==FALSE) && LIB_IsICCard(strTmp) )		// [#2517]

							{
#if !(MX_VERSION)				// [#2015] MX KSK 2012.02.02 International로 인한 FALLBACK은 9F39 Tag 송신 안함 (멕시코 특이사양 - 고객 요청사항)
								strTmp.Format(L"ud9F390192");
								m_strSendData += strTmp;
								m_strSendData += FIELD_DELIMITER;
#else
								//if (bMX_MSTransFlag == FALSE)			// [#2291] Justin Disable
								{
									// International이 아닌 경우에는 이전 사양 유지 (9F39 Tag 송신)
									//strTmp.Format(L"ud9F390192");
									strTmp.Format(L"ud9F390190");		// [#2291] Justin Change Fall Back Tag "92" => 90" in Mexico AP (requested by Customer)
									m_strSendData += strTmp;
									m_strSendData += FIELD_DELIMITER;
								}
								/*
								else
								{
									// International인 경우에는 9F39 Tag 송신하지 않음
								}
								*/
#endif							// end of [#2015]

								// [#2519] NH Justin 2017.12.08 Leave Fallback Indicator in the journal
								CString strOtherMsg = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG);
								if(strOtherMsg.Find(L"F.BACK") < 0)
								{
									if(strOtherMsg.GetLength()>0)
										strOtherMsg += CString(UNIT_DELIMITER);

									CString sNewOtherMsg = L"";
									sNewOtherMsg.Format(L"%s%sF.BACK", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_GENERAL_COMMENT);
									strOtherMsg += sNewOtherMsg;
									MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strOtherMsg );
								}
								// End of [#2519]
							}
						}
						// end of [#2116]
					}
				}
			}

			// [#397] NH PSC 2008.12.15 EMV 거래시 'uh'필드 추가
			// 'h' Encoded EMV untagged-data block. ASCII characters. Variable length
			if ( (nMiscellaneous_FID2 & FID2_h) == FID2_h )
			{
				strTmp.Empty();
				strTmp = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_SCRIPT_RESULTS_TLV);

				if( strTmp.GetLength() != 0 )
				{
					m_strSendData += L"uh" + strTmp;
					m_strSendData += FIELD_DELIMITER;
				}

				strTmp.Empty();
			}

			break;
		case 'v':
			// Make the second FID
//			nMiscellaneous_FID2 = chMiscellaneous_Requests[nMisellIndex][i];
//			nMiscellaneous_FID2 &= 0xFFFFFF00;

			// 'a' Value Added Service Terminal ID 15ASCII characters. 
//			if ( (nMiscellaneous_FID2 & FID2_a) == FID2_a )
//				;
//			// 'b' Postal code of Terminal for Value Added Service 12ASCII
//			if ( (nMiscellaneous_FID2 & FID2_b) == FID2_b )
//				;
//			// 'c' Loyalty card information for Value Added Service3. 80 ASCII
//			if ( (nMiscellaneous_FID2 & FID2_c) == FID2_c )
//				;
//			// 'd' VAS Challenge/Response. 20 ASCII
//			if ( (nMiscellaneous_FID2 & FID2_d) == FID2_d )
//				;
			break;

		case 'x':
			// Make the second FID
//			nMiscellaneous_FID2 = chMiscellaneous_Requests[nMisellIndex][i];
//			nMiscellaneous_FID2 &= 0xFFFFFF00;

			// 'b'
//			if ( (nMiscellaneous_FID2 & FID2_b) == FID2_b )
//				;

			// 'd'
//			if ( (nMiscellaneous_FID2 & FID2_d) == FID2_d )
//				;
			// 'e'
//			if ( (nMiscellaneous_FID2 & FID2_e) == FID2_e )
//				;

			// 'g'
//			if ( (nMiscellaneous_FID2 & FID2_g) == FID2_g )
//				;

			// 'r'
//			if ( (nMiscellaneous_FID2 & FID2_r) == FID2_r )
//				;
			// 's'
//			if ( (nMiscellaneous_FID2 & FID2_s) == FID2_s )
//				;

			// 'u'
//			if ( (nMiscellaneous_FID2 & FID2_u) == FID2_u )
//				;
			// 'v'
//			if ( (nMiscellaneous_FID2 & FID2_v) == FID2_v )
//				;
			break;

			// [#583] NH KSK 2009.11.22 추가 Field는 아직 Coding 안되어져 있음
		}
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_Make_StatusMonitoringField()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD3_Make_StatusMonitoringField()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_Make_StatusMonitoringField]\n"));

	int nStatus = 0;
	int nCount = 0;
	int nIndex = 0;
	int nDenomination = 0;
	int nDivider = 0;
	int nNotesLoadedCount = 0;
	int nNotesDispensed = 0;
	int nRejectCount = 0;
	int nTotalNotesPurged = 0;
	CString strVersion;

	// 1. Program Version Number (10 ASCII) // AP Version
	// PJH 2006.07.05 Edit Host Total 실패로 고침.(확인필요 Genpass), KA는 Triton기기에서 딴 것임.
	// PC Based Version: XXMMNNNRRR, XX=Product Code

	// [#2345] US Justin Change Version for STD3 for PAI
	CString sVersionPreFix = L"CE";
	CString sOSVerPrefix = L"OS";		// [#2405] US Justin 
	CString sRMSVerPrefix = L"RM";		// [#2455] NH Justin PAI Standard3 Status Message Customzing

	#if (APP_CUSTOM_PAI == 1)

		// AP Version Prefix	: NO "P" "C" "X" "V" : ATM TYPE + EMV Status
		// 1800CE :B			5000CE:D		5300CE:F
		// 1800SE :G			2700  :H		2700T :J		4000W  :K
		// 5000SE :L			Halo  :M		5200SE:O		Halo II:Q
		// 5300SE :R			2800SE:S			1500SE:N

		// ATM TYPE
		CString strStoredMachineKind = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND);
		sVersionPreFix = L"H";				// 2700 : Default.
		if		(strStoredMachineKind == L"NH1800SE")	sVersionPreFix = L"G";
		else if	(strStoredMachineKind == L"NH2700T")	sVersionPreFix = L"J";
		else if	(strStoredMachineKind == L"MX4000W")	sVersionPreFix = L"K";
		else if	(strStoredMachineKind == L"MX5000SE")	sVersionPreFix = L"L";
		else if	(strStoredMachineKind == L"NH2600")		sVersionPreFix = L"M";
		else if	(strStoredMachineKind == L"NH1500SE")	sVersionPreFix = L"N";
		else if	(strStoredMachineKind == L"MX5200SE")	sVersionPreFix = L"O";
		else if	(strStoredMachineKind == L"MX2600SE")	sVersionPreFix = L"Q";
		else if	(strStoredMachineKind == L"MX5300SE")	sVersionPreFix = L"R";
		else if	(strStoredMachineKind == L"MX2800SE")	sVersionPreFix = L"S";		//NH Justin 2018.08.07 Add MX2800SE in Status Monitoring Field

		// EMV Enabled Status
		if (P_EMV_CheckTransMode() == RES_EMV_IC_FIRST)	sVersionPreFix += L"E";
		else											sVersionPreFix += L"P";

		// OS Version Prefix
		CString sVerType = L" ";
		BOOL bAvailDCC, bAvailPopmoney, bMCCPSymmKeyPresent, bUnused1, bMCCashPickup, libertyXEnabled, b4UEnabled, ccSegmentationEnabled, ccaEnabled;					// [#2527] US Justin 2018.01.15 Add PAI Cash Pickup Status
		bAvailDCC = bAvailPopmoney = bMCCPSymmKeyPresent = bUnused1 = bMCCashPickup = libertyXEnabled = b4UEnabled  = ccSegmentationEnabled, ccaEnabled = FALSE;		// [#2527] US Justin 2018.01.15 Add PAI Cash Pickup Status
	
		if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) == ENABLE )							bAvailDCC = TRUE;
		if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_ENABLE)    == ENABLE )							bAvailPopmoney = TRUE;

		ccaEnabled = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE) == ENABLE;

		// [#RWC6-57] US William 2019.09.18 PAI MCCP Key presence
		if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE) == ENABLE 
			&& m_Pin4.m_bPin4Available 
			&& m_Pin4.HaveSymmetricKey())			bMCCPSymmKeyPresent = TRUE;

		if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE)== ENABLE) && (m_Pin4.m_bPin4Available==TRUE) )
		{
			bMCCashPickup = TRUE;
		}

		b4UEnabled = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_B4U_ENABLED) == ENABLE;
		ccSegmentationEnabled = Asc2Int(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE)) != 0;

		LXConfiguration config;
		CLibertyXConfigurationManager manager;
		if (manager.GetConfiguration(config)) 
		{
			libertyXEnabled = config.IsEnabled();
		}

		//
		// Table version code
		//

		// NONE  
		if(      !bAvailDCC && !bAvailPopmoney && !bMCCPSymmKeyPresent && !bUnused1 )	sVerType = L" ";

		// One Service
		else if( !bAvailDCC &&  bAvailPopmoney && !bMCCPSymmKeyPresent && !bUnused1 )	sVerType = L"a";
		else if( !bAvailDCC && !bAvailPopmoney &&  bMCCPSymmKeyPresent && !bUnused1 )	sVerType = L"b";
		else if( !bAvailDCC && !bAvailPopmoney && !bMCCPSymmKeyPresent &&  bUnused1 )	sVerType = L"c";
		else if(  bAvailDCC && !bAvailPopmoney && !bMCCPSymmKeyPresent && !bUnused1 )	sVerType = L"d";

		// Two Services
		else if(  bAvailDCC &&  bAvailPopmoney && !bMCCPSymmKeyPresent && !bUnused1 )	sVerType = L"q";
		else if(  bAvailDCC && !bAvailPopmoney &&  bMCCPSymmKeyPresent && !bUnused1 )	sVerType = L"g";
		else if(  bAvailDCC && !bAvailPopmoney && !bMCCPSymmKeyPresent &&  bUnused1 )	sVerType = L"h";
		else if( !bAvailDCC &&  bAvailPopmoney &&  bMCCPSymmKeyPresent && !bUnused1 )	sVerType = L"j";
		else if( !bAvailDCC &&  bAvailPopmoney && !bMCCPSymmKeyPresent &&  bUnused1 )	sVerType = L"l";
		else if( !bAvailDCC && !bAvailPopmoney &&  bMCCPSymmKeyPresent &&  bUnused1 )	sVerType = L"m";

		// Three Services
		else if(  bAvailDCC &&  bAvailPopmoney &&  bMCCPSymmKeyPresent && !bUnused1 )	sVerType = L"i";
		else if(  bAvailDCC &&  bAvailPopmoney && !bMCCPSymmKeyPresent &&  bUnused1 )	sVerType = L"k";
		else if(  bAvailDCC && !bAvailPopmoney &&  bMCCPSymmKeyPresent &&  bUnused1 )	sVerType = L"n";
		else if( !bAvailDCC &&  bAvailPopmoney &&  bMCCPSymmKeyPresent &&  bUnused1 )	sVerType = L"o";

		// Four Services
		else if(  bAvailDCC &&  bAvailPopmoney &&  bMCCPSymmKeyPresent &&  bUnused1 )	sVerType = L"p";

		if (P_EMV_CheckTransMode() == RES_EMV_IC_FIRST)
		{
			if( sVerType == L" " )	sVerType = L"E";
			else					sVerType.MakeUpper();
		}
		sOSVerPrefix.Format(L"O%s", sVerType);


		//
		// Firmware version code
		//

		sRMSVerPrefix = L"R";	

		// NONE  
		if(      !libertyXEnabled && !b4UEnabled && !ccSegmentationEnabled && !ccaEnabled )	sRMSVerPrefix = L"R";

		// One Service
		else if( !libertyXEnabled &&  b4UEnabled && !ccSegmentationEnabled && !ccaEnabled )	sRMSVerPrefix = L"c";
		else if( !libertyXEnabled && !b4UEnabled &&  ccSegmentationEnabled && !ccaEnabled )	sRMSVerPrefix = L"g";
		else if(  libertyXEnabled && !b4UEnabled && !ccSegmentationEnabled && !ccaEnabled )	sRMSVerPrefix = L"a";
		else if(  !libertyXEnabled && !b4UEnabled && !ccSegmentationEnabled &&  ccaEnabled )	sRMSVerPrefix = L"k";

		// Two Services
		else if(  libertyXEnabled &&  b4UEnabled && !ccSegmentationEnabled && !ccaEnabled )	sRMSVerPrefix = L"d";
		else if(  libertyXEnabled && !b4UEnabled &&  ccSegmentationEnabled && !ccaEnabled )	sRMSVerPrefix = L"h";
		else if( !libertyXEnabled &&  b4UEnabled &&  ccSegmentationEnabled && !ccaEnabled )	sRMSVerPrefix = L"i";
		else if(  libertyXEnabled && !b4UEnabled && !ccSegmentationEnabled &&  ccaEnabled )	sRMSVerPrefix = L"l";
		else if( !libertyXEnabled &&  b4UEnabled && !ccSegmentationEnabled &&  ccaEnabled )	sRMSVerPrefix = L"m";
		else if( !libertyXEnabled && !b4UEnabled &&  ccSegmentationEnabled &&  ccaEnabled )	sRMSVerPrefix = L"o";

		// Three Services
		else if(  libertyXEnabled &&  b4UEnabled &&  ccSegmentationEnabled && !ccaEnabled )	sRMSVerPrefix = L"j";
		else if(  libertyXEnabled && !b4UEnabled &&  ccSegmentationEnabled &&  ccaEnabled )	sRMSVerPrefix = L"p";
		else if(  libertyXEnabled &&  b4UEnabled && !ccSegmentationEnabled &&  ccaEnabled )	sRMSVerPrefix = L"n";
		else if( !libertyXEnabled &&  b4UEnabled &&  ccSegmentationEnabled &&  ccaEnabled )	sRMSVerPrefix = L"q";

		// Four Services
		else if(  libertyXEnabled &&  b4UEnabled &&  ccSegmentationEnabled &&  ccaEnabled )	sRMSVerPrefix = L"s";

#if(APP_PRESTAGIN_PIN4)
		if (bMCCashPickup) 
		{
			if (sRMSVerPrefix == L"R")
			{
				sRMSVerPrefix = "b";
			}
			else 
			{
				// If other services are enabled, this flag will make it uppercase
				sRMSVerPrefix.MakeUpper();
			}
		}
#endif

		// Network Type
		//  [#2554] NH Justin 2018.06.05 PAI Customizing STD3 Status Monitoring Field 
		/*
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP)	sRMSVerPrefix += L"a";
		else																				sRMSVerPrefix += L"A";
		*/
		BOOL bDualBalanceEnable = FALSE;
		#if (APP_DUALBALANCE)
			if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE)==ENABLE )
				bDualBalanceEnable = TRUE;
		#endif

		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP)	
		{
			if(bDualBalanceEnable)	sRMSVerPrefix += L"b";
			else					sRMSVerPrefix += L"a";
		}
		else
		{
			if(bDualBalanceEnable)	sRMSVerPrefix += L"B";
			else					sRMSVerPrefix += L"A";
		}
		// End of [#2554]
		// End of [#2455]

	#endif
	// End of [#2345]

	// [#41] KSK 2008.03.18	VERSION 변경 KA -> CE / KC -> OS / KD -> RM(RMS Version)
	strVersion = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_AP_VERSION);	// [#127] KSK 2008.04.21
	// [#2338] US Justin 2015.03.27 "AP" and "AE"(EMV Enabled) for PAI
	//m_sSTD3_StatusField.m_strProgramVersionNo_10.Format(L"CE-%2.2s%2.2s.%2.2s",				  strVersion.Mid(1,2), strVersion.Mid(4,2), strVersion.Mid(7,2));
	// [#2453] US Justin Add Application Type
	//m_sSTD3_StatusField.m_strProgramVersionNo_10.Format(L"%s-%2.2s%2.2s.%2.2s", sVersionPreFix, strVersion.Mid(1,2), strVersion.Mid(4,2), strVersion.Mid(7,2));
	#if (US_VERSION || CA_VERSION || MX_VERSION)
		m_sSTD3_StatusField.m_strProgramVersionNo_10.Format(L"%s-%1.1s%1.1s%2.2s.%2.2s", sVersionPreFix, strVersion.Mid(0,1), strVersion.Mid(2,1), strVersion.Mid(4,2), strVersion.Mid(7,2));
	#else
		m_sSTD3_StatusField.m_strProgramVersionNo_10.Format(L"%s-%2.2s%2.2s.%2.2s", sVersionPreFix, strVersion.Mid(1,2), strVersion.Mid(4,2), strVersion.Mid(7,2));
	#endif
	// End of [#2453]
	// End of [#2338]
	m_strSendData += m_sSTD3_StatusField.m_strProgramVersionNo_10;

	// 2. Table Version Number (10 ASCII)
	strVersion = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_OS_VERSION);	// [#127] KSK 2008.04.21
	// [#2405] US Justin 2016.03.22 Change OS Version Prefix for PAI
	//m_sSTD3_StatusField.m_strTableVersionNo_10.Format(L"OS-%2.2s%2.2s.%2.2s",				  strVersion.Mid(1,2), strVersion.Mid(4,2), strVersion.Mid(7,2)); // [#41] KSK 2008.03.18
	  m_sSTD3_StatusField.m_strTableVersionNo_10.Format(L"%s-%2.2s%2.2s.%2.2s", sOSVerPrefix, strVersion.Mid(1,2), strVersion.Mid(4,2), strVersion.Mid(7,2)); // [#41] KSK 2008.03.18
	m_strSendData += m_sSTD3_StatusField.m_strTableVersionNo_10;

	// 3. Firmware Version Number (10 ASCII)
	strVersion = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_RMS_VERSION);
	// [#2455] NH Justin PAI Standard3 Status Message Customzing
	//m_sSTD3_StatusField.m_strFirmwareVersionNo_10.Format(L"RM%2.2s.%2.2s0%2.2s",		          strVersion.Mid(1,2), strVersion.Mid(4,2), strVersion.Mid(7,2));	// [#41] KSK 2008.03.18
	  m_sSTD3_StatusField.m_strFirmwareVersionNo_10.Format(L"%s%2.2s.%2.2s0%2.2s", sRMSVerPrefix, strVersion.Mid(1,2), strVersion.Mid(4,2), strVersion.Mid(7,2));	// [#41] KSK 2008.03.18
	m_strSendData += m_sSTD3_StatusField.m_strFirmwareVersionNo_10;

	// 4. Alarm, Chest door open (1 ASCII) 0: False 1: True, <space>: Not supported, X: Not installed
	if (m_pDevCmn->fnDOR_GetDoorStatus() == DOOR_OPENED)	// [#483] [NH] KSK 2009.2.6
		m_sSTD3_StatusField.m_strAlarm_ChestDoorOpen_1 = L"1";
	else
		m_sSTD3_StatusField.m_strAlarm_ChestDoorOpen_1 = L"0";
	m_strSendData += m_sSTD3_StatusField.m_strAlarm_ChestDoorOpen_1;

	// 5. Alarm, Top door open (1 ASCII) 0: False 1: True <space> Not supported X : Not installed
	//X에서 SPACE로 변경
	m_sSTD3_StatusField.m_strAlarm_TopDoorOpen_1 = L" "; // Not supported
	m_strSendData += m_sSTD3_StatusField.m_strAlarm_TopDoorOpen_1;

	//5050 WinCE는 Supervisor switch가 없으므로 Not Supported가 맞음.
	// 6. Alarm, Supervisor active (1 ASCII) 0: False 1: True <space> Not supported
	m_sSTD3_StatusField.m_strAlarm_SupervisonActive_1 = L" ";	//one space

	m_strSendData += m_sSTD3_StatusField.m_strAlarm_SupervisonActive_1;

	// 7. Receipt printer, paper status (1 ASCII) 0: OK 1: LOW 2: OUT <space> Not supported
	if( m_pDevCmn->fnSPR_GetPaperStatus() == SLIP_NORMAL )
		m_sSTD3_StatusField.m_strReceiptPrinter_PaperStatus_1 = L"0";
	else if(m_pDevCmn->fnSPR_GetPaperStatus() == SLIP_LOW_END)
		m_sSTD3_StatusField.m_strReceiptPrinter_PaperStatus_1 = L"1";
	else if(m_pDevCmn->fnSPR_GetPaperStatus() == SLIP_EMPTY_PAPER)
		m_sSTD3_StatusField.m_strReceiptPrinter_PaperStatus_1 = L"2";
	else // PJH 2006.07.05 Added for default
		m_sSTD3_StatusField.m_strReceiptPrinter_PaperStatus_1 = L"0";
	m_strSendData += m_sSTD3_StatusField.m_strReceiptPrinter_PaperStatus_1;

	// 8. Receipt Printer, Ribbon status (1 ASCII) 0: OK 1: LOW 2: OUT <space> Not supported T: thermal
	m_sSTD3_StatusField.m_strReceiptPrinter_RibbonStatus_1 = L"T";
	m_strSendData += m_sSTD3_StatusField.m_strReceiptPrinter_RibbonStatus_1;

	// 9. Journal Printer, Paper status (1 ASCII) 0: OK 1: LOW 2: OUT <space> Not supported, X: Not installed
	// PJH_REV8 2007.02.07 Bug Fix 'X' -> '0' EJ 도 하나의 JOURNAL PRINTER로 본다.
	m_sSTD3_StatusField.m_strJournalPrinter_PaperStatus_1 = L"0";
	m_strSendData += m_sSTD3_StatusField.m_strJournalPrinter_PaperStatus_1;

	// 10. Journal Printer, Ribbon status (1 ASCII) 0: OK 1: LOW 2: OUT <space> Not supported, T: Thermal, X: Not installed
	// PJH 2006.07.05 Added
	// KSK 2009.12.29 EJ가 있으므로 항상 Status는 OK로 올린다.
	m_sSTD3_StatusField.m_strJournalPrinter_RibbonStatus_1 = L"0";
	// end of KSK 2009.12.29

	m_strSendData += m_sSTD3_StatusField.m_strJournalPrinter_RibbonStatus_1;

	// 11. Note status, Dispenser (1 ASCII) 0: OK 1: LOW 2: OUT <space> Not supported
	// This is the cumulative summary of all cassettes in the dispenser.
	// Empty individual cassettes are not reflected here
	// [#178] NH PSC 2008.04.29 CDU 상태필드 변경
	// '2'(Out) - "20002", "F0001"장애시,
	// '1'(Low) - 논리매수 100매이하(모든 카세트가 100매이하일때 '1'), Low Sensor detected(모든 카세트가 100매이하일때 '1')
	// '0'(OK ) - '2'나 '1'이외의 경우 모두 해당됨

	// [#507] [NH] KSK 2009.2.23 논리적인 매수만 가지고 NoteStatus를 송신하도록 수정
	BOOL	bCheckLowStatus = FALSE;
	int		nNoteCount = m_pDevCmn->fnCDU_GetNumberOfAllCSTs();
	int		CstNoteStatus = m_pDevCmn->fnCDU_GetAllCSTStatus();

	NHDEBUG(1, (_T("***TranHostProc*** SENSOR INFORMATION [%x] [%x] [%x] [%x] \n"), MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_LOWSENSOR)
																					, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST2_LOWSENSOR)
																					, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST3_LOWSENSOR)
																					, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST4_LOWSENSOR)));

	if ( nNoteCount <= 0)	// 전체 매수가 0인 경우 (음수값과 양수값 합이 0인 경우에는 문제 소지 있음) - KSK 2009.3.2
		m_sSTD3_StatusField.m_strNoteStatus_Dispenser_1 = L"2";	// OUT OF SERVICE
	else
	{
		if ((m_pDevCmn->CSTCnt > 0) && (m_pDevCmn->CSTCnt <= CDU_MAX_CST_COUNT))
		{
			for(int i=0; i<m_pDevCmn->CSTCnt; i++)	// 논리적인 매수가 LOW인 경우 Check
			{
				if ((m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1 + i) > CDU_NEARCASHCNT) && (MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_LOWSENSOR + i) == 1))
				{
					// 논리적인 매수 100매 초과 && low sensor 미감지시 normal
					bCheckLowStatus = FALSE;
					break;
				}
				else if ((m_pDevCmn->fnCDU_GetNumberOfCash(CDU_CST_1 + i) <= CDU_NEARCASHCNT) || (MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_LOWSENSOR + i) == 0))
				{
					// 논리적인 매수 100매 이하 or low sensor 감지시 low
					bCheckLowStatus = TRUE;
				}
			}

			if (bCheckLowStatus == TRUE)	// 전체 Cassette가 LOW이면
			{
				// POS와 동일하게 사양 처리 (사양서 update) "20002" 장애가 발생하더라도 notestatus는 "low"로 송신하도록 수정 KSK 2009.3.9
				m_sSTD3_StatusField.m_strNoteStatus_Dispenser_1 = L"1";	// LOW
			}
			else
				m_sSTD3_StatusField.m_strNoteStatus_Dispenser_1 = L"0";	// OK
		}
		else
			m_sSTD3_StatusField.m_strNoteStatus_Dispenser_1 = L"2";	// OUT OF SERVICE	CSTCNT값이 0 or 4보다 큰 경우 OUT으로 return
	}
	// end of [#507]

	// end of [#177]
	m_strSendData += m_sSTD3_StatusField.m_strNoteStatus_Dispenser_1;

	// 12. Receipt Printer (1 ASCII) 0: OK 1: Needs attention 2: Out of Service <space> Not supported X : Not installed
	//m_Triton_StatusMsg.m_strReceiptPrinter_1;
	if (m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_SPR))
		m_sSTD3_StatusField.m_strReceiptPrinter_1 = L"2";
	else 
	{
		nStatus = m_pDevCmn->fnSPR_GetPaperStatus();
		switch( nStatus )
		{
		case SLIP_EMPTY_PAPER:
			m_sSTD3_StatusField.m_strReceiptPrinter_1 = L"2";
			break;
		case SLIP_LOW_END:
			m_sSTD3_StatusField.m_strReceiptPrinter_1 = L"1";
			break;
		default:
			m_sSTD3_StatusField.m_strReceiptPrinter_1 = L"0";
			break;
		}
	}
	m_strSendData += m_sSTD3_StatusField.m_strReceiptPrinter_1;

	// 13. Journal Printer (1 ASCII) 0: OK 1: Needs attention 2: Out of Service <space> Not supported X : Not installed
	m_sSTD3_StatusField.m_strJournalPrinter_1 = L"0";
	m_strSendData += m_sSTD3_StatusField.m_strJournalPrinter_1;

	// 14. Dispenser (1 ASCII) 0: OK 1: Needs attention 2: Out of Service <space> Not supported X : Not installed
	// KSK_2007-07-22오전 11:39:38
	// Terminal Status Set (Dispenser)
	// [#178] NH PSC 2008.04.29 CDU 상태필드 변경
	// '2' (Out of Service) - "20001" Error, Note status == 2 or 1(Low Check disable), Feed Fail(Can not be recovered), Note Jam error 및 단선
	// '0' (OK)             - Note status == 1(Low Check enable), 그외 '2'의 조건 이외의 경우

	if ((m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_CDU)) ||	// CDU 장애 시
		(CstNoteStatus == CST_SET_NG) ||	// CST 탈착 시 (20001)
		(CstNoteStatus == CST_EMPTY)  ||	// 논리적인 금액이 없을 경우 (F0001)
		(CstNoteStatus == CST_NEAR))		// Low Sensor Check Enable시 (20002)
		m_sSTD3_StatusField.m_strDispenser_1 = L"2";		// out of service
	else
		m_sSTD3_StatusField.m_strDispenser_1 = L"0";		// ok
	// end of [#177]

	m_strSendData += m_sSTD3_StatusField.m_strDispenser_1;

	// 15. Communications System (1 ASCII) 0: OK 1: Needs attention 2: Out of Service
	m_sSTD3_StatusField.m_strCommunicationsSystem_1 = L"0";
	m_strSendData += m_sSTD3_StatusField.m_strCommunicationsSystem_1;

	// 16. Card Reader (1 ASCII) 0: OK 1: Needs attention 2: Out of Service
	if (m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_MCU))
		m_sSTD3_StatusField.m_strCardReader_1 = L"2";
	else
		m_sSTD3_StatusField.m_strCardReader_1 = L"0";
	m_strSendData += m_sSTD3_StatusField.m_strCardReader_1;

	// 17. Cards retained (3 ASCII)  000-999, <space> Not supported
	m_sSTD3_StatusField.m_strCardsRetained_3 = L"   ";
	m_strSendData += m_sSTD3_StatusField.m_strCardsRetained_3;

	// 18. Electronics system (2 ASCII) XX -  , <space> Not supported 
	m_sSTD3_StatusField.m_strElectronicsSystem_2 = L"  ";
	m_strSendData += m_sSTD3_StatusField.m_strElectronicsSystem_2;

	// Error Conversion Needed for Triton PJH 2006.03.29
	// 19. Current Error Code for Terminal (3 ASCII) XXX represents the value of the error code '000' No problems

	// KSK_2007-07-22오전 11:39:38
	// Triton errorcode convert 작업이 필요함
	// PJH_REV4 CURRENT ERROR가 없으면, 현재 상태를 구분하여 장애를 올린다. 2007.02.07
	CString strError = m_pDevCmn->fstrAPL_GetErrorCode();
	if ((strError != L"") && (strError != L"0000000"))
	{
		int i;
		for(i=0; i<TRITON_ERRORCODE_EMUL_COUNT; i++)
		{
			if (!strError.CompareNoCase(Triton_ErrorCode_Tbl[i][0]))
			{
				// error code emulator code search
				m_sSTD3_StatusField.m_strCurrentErrorCode_3 = Triton_ErrorCode_Tbl[i][1];
				break;
			}
		}
		if (i == TRITON_ERRORCODE_EMUL_COUNT)
		{
			// error code emulator code is not search
			m_sSTD3_StatusField.m_strCurrentErrorCode_3 = L"000";
		}
	}
	else // ERROR CODE가 없을 경우
	{
		m_sSTD3_StatusField.m_strCurrentErrorCode_3 = L"000";
	}

	m_strSendData += m_sSTD3_StatusField.m_strCurrentErrorCode_3;

	// 20. Communications failures (3 ASCII)
	m_sSTD3_StatusField.m_strCommunicatioinsFailures_3 = L"000";
	m_strSendData += m_sSTD3_StatusField.m_strCommunicatioinsFailures_3;

	//////////////////////////////////////////////////////////////////////////
	//	Default Cassette A, B, C, D set all '0'
	//  Denomination(3 ASCII), Notes loaded (4 ASCII), 
	//  Notes dispensed(4 ASCII), Reject events(3 ASCII)
	//////////////////////////////////////////////////////////////////////////
	// PJH 2006.07.05 Default Value '0' -> ' ' 변경 Triton 기기 참조: Genpass
	//Denomination(3 ASCII)
	m_sSTD3_StatusField.m_strCassetteA_Denomination_3 = L"000";
	m_sSTD3_StatusField.m_strCassetteB_Denomination_3 = L"000";
	m_sSTD3_StatusField.m_strCassetteC_Denomination_3 = L"000";
	m_sSTD3_StatusField.m_strCassetteD_Denomination_3 = L"000";
	//Notes loaded (4 ASCII)
	m_sSTD3_StatusField.m_strCassetteA_NotesLoaded_4 = L"0000";
	m_sSTD3_StatusField.m_strCassetteB_NotesLoaded_4 = L"0000";
	m_sSTD3_StatusField.m_strCassetteC_NotesLoaded_4 = L"0000";
	m_sSTD3_StatusField.m_strCassetteD_NotesLoaded_4 = L"0000";
	//Notes dispensed(4 ASCII)
	m_sSTD3_StatusField.m_strCassetteA_NotesDispensed_4 = L"0000";
	m_sSTD3_StatusField.m_strCassetteB_NotesDispensed_4 = L"0000";
	m_sSTD3_StatusField.m_strCassetteC_NotesDispensed_4 = L"0000";
	m_sSTD3_StatusField.m_strCassetteD_NotesDispensed_4 = L"0000";
	//Reject events(3 ASCII)
	m_sSTD3_StatusField.m_strCassetteA_RejectEvents_3 = L"000";
	m_sSTD3_StatusField.m_strCassetteB_RejectEvents_3 = L"000";
	m_sSTD3_StatusField.m_strCassetteC_RejectEvents_3 = L"000";
	m_sSTD3_StatusField.m_strCassetteD_RejectEvents_3 = L"000";
	//////////////////////////////////////////////////////////////////////////
	int nNumberofCst = m_pDevCmn->fnCDU_GetNumberOfCST();
	// Cassette A 
	if( nNumberofCst >= 1)
	{
		// 21. Cassette A Denomination (3 ASCII) 000-99K 'K' indicate thousands. 01K - 1000
		nDenomination = m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_1);
		// DENOMINATION REPORT 오류 수정
		// 권종이 $10인 경우 1K로 보내줘야함 ($25인 경우에는 사양적으로 문제소지 있음)
		////////////////////////////////////////////////////////////////////////////////////
		if( (nDenomination%10) == 0 )
		{
			nDivider = nDenomination / 10;
			m_sSTD3_StatusField.m_strCassetteA_Denomination_3.Format(L"%02dK", nDivider);
		}
		else
		{
			nDivider = nDenomination * 100;		// [#555] NH KSK 2009.08.11 $1 -> 100, $5 -> 500으로 변경하도록 사양 변경 (NHA협의사항)
			m_sSTD3_StatusField.m_strCassetteA_Denomination_3.Format(L"%03d", nDivider);
		}

		// 22. Cassette A notes loaded (4 ASCII) 0000-9999
		// [#2270] AU KSK 2014.05.26
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			nNotesLoadedCount = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_1);
		else
			nNotesLoadedCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_INITIALCOUNT_AP);
		// end of [#2270]
		m_sSTD3_StatusField.m_strCassetteA_NotesLoaded_4.Format(L"%4.4d", nNotesLoadedCount);

		// 23. Cassette A notes dispensed (4 ASCII) 0000-9999
		nNotesDispensed = m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_1);
		m_sSTD3_StatusField.m_strCassetteA_NotesDispensed_4.Format(L"%4.4d",nNotesDispensed);

		// 24. Cassette A reject events (3 ASCII) 000-999
		nRejectCount = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_1);
		m_sSTD3_StatusField.m_strCassetteA_RejectEvents_3.Format(L"%3.3d",nRejectCount);
	}
	// [#564] NH KSK 2009.8.20 보완 처리
	m_strSendData += m_sSTD3_StatusField.m_strCassetteA_Denomination_3.Right(3)	+
						m_sSTD3_StatusField.m_strCassetteA_NotesLoaded_4.Right(4)	+
						m_sSTD3_StatusField.m_strCassetteA_NotesDispensed_4.Right(4) +
						m_sSTD3_StatusField.m_strCassetteA_RejectEvents_3.Right(3);
	// end of [#564]

	// Cassette B
	if( nNumberofCst >= 2)
	{
		// 25. Cassette B Denomination (3 ASCII) 000-99K 'K' indicate thousands. 01K - 1000
		nDenomination = m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_2);
		// DENOMINATION REPORT 오류 수정
		// 권종이 $10인 경우 1K로 보내줘야함
		if( (nDenomination%10) == 0 )
		{
			nDivider = nDenomination / 10;
			m_sSTD3_StatusField.m_strCassetteB_Denomination_3.Format(L"%02dK", nDivider);
		}
		else
		{
			nDivider = nDenomination * 100;		// [#555] NH KSK 2009.08.11 $1 -> 100, $5 -> 500으로 변경하도록 사양 변경 (NHA협의사항)
			m_sSTD3_StatusField.m_strCassetteB_Denomination_3.Format(L"%03d", nDivider);
		}

		// 26. Cassette B notes loaded (4 ASCII) 0000-9999
		// [#2270] AU KSK 2014.05.26
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			nNotesLoadedCount = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_2);
		else
			nNotesLoadedCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST2_INITIALCOUNT_AP);
		// end of [#2270]
		m_sSTD3_StatusField.m_strCassetteB_NotesLoaded_4.Format(L"%4.4d", nNotesLoadedCount);

		// 27. Cassette B notes dispensed (4 ASCII) 0000-9999
		nNotesDispensed = m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_2);
		m_sSTD3_StatusField.m_strCassetteB_NotesDispensed_4.Format(L"%4.4d",nNotesDispensed) ;

		// 28. Cassette B reject events (3 ASCII) 000-999
		nRejectCount = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_2);
		m_sSTD3_StatusField.m_strCassetteB_RejectEvents_3.Format(L"%3.3d",nRejectCount);
	}
	// [#564] NH KSK 2009.8.20 보완 처리
	m_strSendData += m_sSTD3_StatusField.m_strCassetteB_Denomination_3.Right(3)	+
						m_sSTD3_StatusField.m_strCassetteB_NotesLoaded_4.Right(4)	+
						m_sSTD3_StatusField.m_strCassetteB_NotesDispensed_4.Right(4) +
						m_sSTD3_StatusField.m_strCassetteB_RejectEvents_3.Right(3);
	// end of [#564]

	// Cassette C
	if( nNumberofCst >= 3)
	{
		// 29. Cassette C Denomination (3 ASCII) 000-99K 'K' indicate thousands. 01K - 1000
		nDenomination = m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_3);
		// DENOMINATION REPORT 오류 수정
		// 권종이 $10인 경우 1K로 보내줘야함
		////////////////////////////////////////////////////////////////////////////////////
		if( (nDenomination%10) == 0 )
		{
			nDivider = nDenomination / 10;
			m_sSTD3_StatusField.m_strCassetteC_Denomination_3.Format(L"%02dK", nDivider);
		}
		else
		{
			nDivider = nDenomination * 100;		// [#555] NH KSK 2009.08.11 $1 -> 100, $5 -> 500으로 변경하도록 사양 변경 (NHA협의사항)
			m_sSTD3_StatusField.m_strCassetteC_Denomination_3.Format(L"%03d", nDivider);
		}

		// 30. Cassette C notes loaded (4 ASCII) 0000-9999
		// [#2270] AU KSK 2014.05.26
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			nNotesLoadedCount = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_3);
		else
			nNotesLoadedCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST3_INITIALCOUNT_AP);
		// end of [#2270]
		m_sSTD3_StatusField.m_strCassetteC_NotesLoaded_4.Format(L"%4.4d", nNotesLoadedCount);

		// 31. Cassette C notes dispensed (4 ASCII) 0000-9999
		nNotesDispensed = m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_3);
		m_sSTD3_StatusField.m_strCassetteC_NotesDispensed_4.Format(L"%4.4d",nNotesDispensed) ;

		// 32. Cassette C reject events (3 ASCII) 000-999
		nRejectCount = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_3);
		m_sSTD3_StatusField.m_strCassetteC_RejectEvents_3.Format(L"%3.3d",nRejectCount);
	}
	// [#564] NH KSK 2009.8.20 보완 처리
	m_strSendData += m_sSTD3_StatusField.m_strCassetteC_Denomination_3.Right(3)	+
						m_sSTD3_StatusField.m_strCassetteC_NotesLoaded_4.Right(4)	+
						m_sSTD3_StatusField.m_strCassetteC_NotesDispensed_4.Right(4) +
						m_sSTD3_StatusField.m_strCassetteC_RejectEvents_3.Right(3);
	// end of [#564]

	// Cassette D
	if( nNumberofCst >= 4)
	{
		// 33. Cassette D Denomination (3 ASCII) 000-99K 'K' indicate thousands. 01K - 1000
		nDenomination = m_pDevCmn->fnCDU_GetValueOfCash(CDU_CST_4);
		// DENOMINATION REPORT 오류 수정
		// 권종이 $10인 경우 1K로 보내줘야함
		if( (nDenomination%10) == 0 )
		{
			nDivider = nDenomination / 10;
			m_sSTD3_StatusField.m_strCassetteD_Denomination_3.Format(L"%02dK", nDivider);
		}
		else
		{
			nDivider = nDenomination * 100;		// [#555] NH KSK 2009.08.11 $1 -> 100, $5 -> 500으로 변경하도록 사양 변경 (NHA협의사항)	
			m_sSTD3_StatusField.m_strCassetteD_Denomination_3.Format(L"%03d", nDivider);
		}

		// 34. Cassette D notes loaded (4 ASCII) 0000-9999
		// [#2270] AU KSK 2014.05.26
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			nNotesLoadedCount = m_pDevCmn->fnCDU_GetNumberOfSetCash(CDU_CST_4);
		else
			nNotesLoadedCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST4_INITIALCOUNT_AP);
		// end of [#2270]
		m_sSTD3_StatusField.m_strCassetteD_NotesLoaded_4.Format(L"%4.4d", nNotesLoadedCount);

		// 35. Cassette D notes dispensed (4 ASCII) 0000-9999
		nNotesDispensed = m_pDevCmn->fnCDU_GetNumberOfDispenseCash(CDU_CST_4);
		m_sSTD3_StatusField.m_strCassetteD_NotesDispensed_4.Format(L"%4.4d",nNotesDispensed) ;

		// 36. Cassette D reject events (3 ASCII) 000-999
		nRejectCount = m_pDevCmn->fnCDU_GetNumberOfReject(CDU_CST_4);
		m_sSTD3_StatusField.m_strCassetteD_RejectEvents_3.Format(L"%3.3d",nRejectCount);
	}
	// [#564] NH KSK 2009.8.20 보완 처리
	m_strSendData += m_sSTD3_StatusField.m_strCassetteD_Denomination_3.Right(3)		+
						m_sSTD3_StatusField.m_strCassetteD_NotesLoaded_4.Right(4)		+
						m_sSTD3_StatusField.m_strCassetteD_NotesDispensed_4.Right(4)	+
						m_sSTD3_StatusField.m_strCassetteD_RejectEvents_3.Right(3);

	nTotalNotesPurged = Asc2Int(m_sSTD3_StatusField.m_strCassetteA_RejectEvents_3) + 
						Asc2Int(m_sSTD3_StatusField.m_strCassetteB_RejectEvents_3) +
						Asc2Int(m_sSTD3_StatusField.m_strCassetteC_RejectEvents_3) +
						Asc2Int(m_sSTD3_StatusField.m_strCassetteD_RejectEvents_3) ;

	m_sSTD3_StatusField.m_strTotalNotesPurged_3.Format(L"%3.3d", nTotalNotesPurged);
	m_strSendData += m_sSTD3_StatusField.m_strTotalNotesPurged_3.Right(3);	// [#564] NH KSK 2009.8.20 보완처리

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: BIZ_STD3_RecvHost()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD3_RecvHost()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_RecvHost]\n"));

	//////////////////////////////////////////
	// 1. Recv Data
	if (BIZ_RecvData() != RES_OK)
	{
		if(m_pDevCmn->fnMCU_IsEmvTransaction())		// [#397] [NH] psc 2008.08.01 for EMV transaction
		{
			// [#2365] US Justin 2015.07.24 Add DCC+ on Canadian AP
			// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
			if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_TDL_DCC_TRANSACTION || TranCode == TC_JUSTCASH_BITCOIN || TranCode == TC_DIGITALMINT)
			{
				if (m_bOnlineProcessing == FALSE)
				{
					// KSK 2010.08.29 통신 장애인 경우 Default 처리 추가
					m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

					if (m_pDevCmn->fnEMV_Trans_OnlineProcess(0, AAC) == EMV_RSLT_OK)
					{
						m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
						m_pDevCmn->fnEMV_Trans_Completion();
					}

					BIZ_EMV_MakeICDataforReversal();	// KSK 2009.8.31 송신 에러시에 

					m_bOnlineProcessing = TRUE;
				}
			}
		}

		// 여기에 들어올수 있는 조건은 Data가 0일때 뿐이다.
		// 들어오면 잘못된 Logic임
		// ErrorCode는 추후 협의 필요
		m_pDevCmn->fnAPL_StackError(m_pDevCmn->fstrNET_GetErrorCode(), m_pDevCmn->fstrNET_GetErrorMsg(), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;
	}

	//////////////////////////////////////////
	// 2. Analyze to Recv Data
	{
		BIZ_RETURN	nRes;
		nRes = BIZ_STD3_AnalHostData();					// Recv Anal	[#492] [NH] KSK 2009.2.5

		if (nRes != RES_OK)
			return nRes;
	}

	//////////////////////////////////////////
	// 3. Check Error

#ifdef APP_LOCAL_MODE		// LOCAL Mode 시에 에러체크 없음.
	m_pDevCmn->TranResult = TRUE;
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);


	// KSK 2008.10.16 Local Mode시 Reversal 처리 추가
	if (LIB_IsReversalCondition())				// [#439] NH AIREAT 2008.10.21
		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

	return RES_OK;
#endif

	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	INIT_STS);			// [#358] [NH] KSK 2008.7.10

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_AnalHostData()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD3_AnalHostData()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_AnalHostData]\n"));

	int		Cnt = 0;
	CString	strTemp;

	BIZ_RETURN nRes = RES_HOST_RECV_ERR;		// Default는 HOST ERROR로 SET함

	m_strArrRecvData.RemoveAll();			// V01.02.28 [#22] KSK 2008.03.14

#ifdef	APP_LOCAL_MODE
	switch (TranCode)
	{
		case TC_OPEN:
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_CONFIG));
			break;
		case TC_HEALTHCHK:
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_HEALTH));
			break;
		case TC_TOTAL:
		case TC_TRIALTOTAL:
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_TOTAL));
			break;
		case TC_REVERSAL:
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_REVERSAL));
			break;
		case TC_WITHDRAWAL:
		case TC_JUSTCASH_BITCOIN:
		case TC_PIN4:	// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_WITHDRAWAL));
			break;
		case TC_INQUIRY:
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_INQUIRY));
			break;
		case TC_TRANSFER:
			m_strRecvData.Format(L"%s", MemGetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_TRANSFER));
			break;
	}

	SplitString(m_strRecvData, ".", m_strArrRecvData);

#else
	m_strRecvData.Format(L"%S", m_arRecvBuffer);
	SplitString(m_strRecvData, FIELD_DELIMITER, m_strArrRecvData);
#endif

#ifndef APP_LOCAL_MODE // [#321] NH JSW 2008.06.20
	// [#4] NH PSC 2008.03.10 MAC result, CRC 체크
	// CRC 검증
	if(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_CRCENABLE) == ENABLE)		// CRC option is Enabled
	{
		//[#2007] NH KSK 2010.11.29 CRC 및 MAC 검증 버그 FIX (WINCE5.0에 수정된 내용 적용)
		//		int retCrcKeyFind = m_strRecvData.Find(L"ub");
		int retCrcKeyFind = -1;
		int startIndex = 0; 

		while(1)
		{
			retCrcKeyFind = m_strRecvData.Find(L"ub", startIndex );

			if ( retCrcKeyFind == -1) break;
			else if ( retCrcKeyFind > 0 && m_strRecvData.GetAt(retCrcKeyFind -1) == (TCHAR)0x1C) break; //찾은 경우 
			else startIndex += 2; //처음에 "ub" 나오는 것은 어떻게 하지? 있을 수 없다고 보고 가야 되나?

		} 
		//end of [#2007]

		if(retCrcKeyFind != -1)
		{	// CRC check
			strTemp.Format(L"%S", m_arRecvBuffer);
			m_strCrcResult_16 = m_strRecvData.Mid(retCrcKeyFind+2, 4);

			memset(m_szTemp, NULL, sizeof(m_szTemp));
			WideToMulti(m_szTemp, strTemp, strTemp.GetLength());
			memcpy(&m_szTemp[retCrcKeyFind+2], "0000", 4);

			WORD tempLen = strTemp.GetLength();

			// VISA TYPE을 제외한 나머지 프로토콜은 Length 2바이트를 앞에 붙여서 CRC 계산을 한다.
			if(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE) != VISA_FRAMED_TCPIP	&&		// if(VSISA TYPE: 1) else (STATDARD: 2 or ACK_CONTROLED: 3)
				MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_TCPIP)	
			{
				memmove(&m_szTemp[2], &m_szTemp[0], tempLen);
				m_szTemp[0] = (tempLen& 0xFF00) >> 8;
				m_szTemp[1] = tempLen & 0x00FF;
				tempLen += 2;
			}

			UINT retCRC16 = crc16((BYTE*)m_szTemp, tempLen);
			CString strTempCrc;

			strTempCrc.Format(L"%04X", retCRC16);

			if(m_strCrcResult_16.CompareNoCase(strTempCrc) != 0)
			{
				if(m_pDevCmn->fnMCU_IsEmvTransaction())		// [#397] [NH] psc 2008.08.01 for EMV transaction
				{
					// [#2496] US Justin 2017.08.15 Add Bitcoin
					if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_TDL_DCC_TRANSACTION || TranCode == TC_JUSTCASH_BITCOIN || TranCode == TC_DIGITALMINT)
					{
						if (m_bOnlineProcessing == FALSE)
						{
							// KSK 2010.08.29 CRC ERROR인 경우 Default 처리 추가
							m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

							if (m_pDevCmn->fnEMV_Trans_OnlineProcess(0, AAC) == EMV_RSLT_OK)
							{
								m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
								m_pDevCmn->fnEMV_Trans_Completion();
							}

							BIZ_EMV_MakeICDataforReversal();	// KSK 2009.8.31 송신 에러시에 

							m_bOnlineProcessing = TRUE;
						}
					}
				}

				if (LIB_IsReversalCondition())					// [#439] NH AIREAT 2008.10.21
				{
					// Reversal Information Set
					// REVERSAL FLAG SET
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
				}
				m_pDevCmn->fnAPL_StackError(_T("D4100"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003033), DEV_NET);	// [#419] [NH] KSK 2008.9.16
				return RES_HOST_RECV_ERR;
			}
		}
		else
		{
			if(m_pDevCmn->fnMCU_IsEmvTransaction())		// [#397] [NH] psc 2008.08.01 for EMV transaction
			{
				// [#2496] US Justin 2017.08.15 Add Bitcoin
				if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_TDL_DCC_TRANSACTION || TranCode == TC_JUSTCASH_BITCOIN || TranCode == TC_DIGITALMINT)
				{
					if (m_bOnlineProcessing == FALSE)
					{
						// KSK 2010.08.29 CRC ERROR인 경우 Default 처리 추가
						m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

						if (m_pDevCmn->fnEMV_Trans_OnlineProcess(0, AAC) == EMV_RSLT_OK)
						{
							m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
							m_pDevCmn->fnEMV_Trans_Completion();
						}

						BIZ_EMV_MakeICDataforReversal();	// KSK 2009.8.31 송신 에러시에 

						m_bOnlineProcessing = TRUE;
					}
				}
			}

			if (LIB_IsReversalCondition())				// [#439] NH AIREAT 2008.10.21
			{
				// Reversal Information Set
				// REVERSAL FLAG SET
				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
			}
			m_pDevCmn->fnAPL_StackError(_T("D4101"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003034), DEV_NET);	// [#419] [NH] KSK 2008.9.16	
			return RES_HOST_RECV_ERR;
		}
	}

	// MAC 검증
	// keymode: 5, 6, 7, 11 and 출금, 이체, 조회, 토탈, 트라이얼토탈, 취소거래 and Mac flag enabled
	// [#554] KSK 2009.08.10 m_KeyMode삭제
	// [#2350] US Justin 2015.06.18 Add POP MONEY
	// [#2365] US Justin 2015.07.24 Add DCC+ in Canadian AP
	// [#2415] US Justin 2016.04.16 Add WalPay
	// [#2448] US Justin 2016.11.21 Add Just.Cash
	// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
	// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
	if ((MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_NON_UNIQ_SDES_MACING ||
		 MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_UNIQ_SDES_MACING ||
		 MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_MACING ||
		 MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING) &&
		(TranCode == TC_WITHDRAWAL			|| TranCode == TC_TRANSFER			|| TranCode == TC_INQUIRY				|| TranCode == TC_TOTAL				||
		 TranCode == TC_TRIALTOTAL			|| TranCode == TC_REVERSAL			|| TranCode == TC_POPMONEY				|| TranCode == TC_JUSTCASH			||
		 TranCode == TC_TDL_TIRSURCHARGE	|| TranCode == TC_TDL_DCC_LOOKUP	|| TranCode == TC_TDL_DCC_TRANSACTION	|| TranCode == TC_TDL_DCC_REVERSAL	||
		 TranCode == TC_JUSTCASH_BITCOIN	|| TranCode == TC_PIN4				|| TranCode == TC_DIGITALMINT) &&
		MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_MACFLAG) == 1) // host MAC enabled(host received)
	{
		// [#2007] KSK 2010.11.29 CRC 및 MAC 검증 버그 FIX (WINCE5.0에 수정된 내용 적용)
		//		int retMacKeyFind = m_strRecvData.Find(L"^");
		//		int retCrcKeyFind = m_strRecvData.Find(L"ub");
		int retMacKeyFind = -1;
		int startIndex = 0; 

		while(1)
		{
			retMacKeyFind = m_strRecvData.Find(L"^", startIndex );

			if ( retMacKeyFind == -1) break;
			else if ( retMacKeyFind > 0 && m_strRecvData.GetAt(retMacKeyFind -1) == (TCHAR)0x1C) break; //찾은 경우 
			else startIndex += 1; //처음에 "ub" 나오는 것은 어떻게 하지? 있을 수 없다고 보고 가야 되나?

		} 
		//end of [#2007]

		if(retMacKeyFind != -1)
		{	// MAC check
			m_strMacResult_8 = m_strRecvData.Mid(retMacKeyFind+1, 4);
			m_strMacResult_8 += m_strRecvData.Mid(retMacKeyFind+6, 4);

			strTemp.Format(L"%S", m_arRecvBuffer);
			strTemp.Replace(strTemp.Mid(retMacKeyFind, 10), L"^000000000");

			//[#2007] NH KSK 2010.11.29 CRC 및 MAC 검증 버그 FIX (WINCE5.0에 수정된 내용 적용)
//			if(retCrcKeyFind != -1)
//			{
//				strTemp.Replace(strTemp.Mid(retCrcKeyFind, 6), L"ub0000");
//			}

			if(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_CRCENABLE) == ENABLE)		// CRC option is Enabled
			{
				int retCrcKeyFind = -1;
				startIndex = 0; 

				while(1)
				{
					retCrcKeyFind = m_strRecvData.Find(L"ub", startIndex );

					if ( retCrcKeyFind == -1) break;
					else if ( retCrcKeyFind > 0 && m_strRecvData.GetAt(retCrcKeyFind -1) == (TCHAR)0x1C) break; //찾은 경우 
					else startIndex += 2; //처음에 "ub" 나오는 것은 어떻게 하지? 있을 수 없다고 보고 가야 되나?

				} 

				if(retCrcKeyFind != -1)
				{
					strTemp.Replace(strTemp.Mid(retCrcKeyFind, 6), L"ub0000");
				}
			}
			//end of [#2007]

			memset(m_szTemp, 0, sizeof(m_szTemp));
			WideToMulti(m_szTemp, strTemp, strTemp.GetLength());
			WORD tempLen = strTemp.GetLength();

			// VISA TYPE은 STX와 ETX를 붙여서 MAC Result를 계산한다.
			if((MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE) == VISA_FRAMED_TCPIP)	||		// if(VSISA TYPE: 1) else (STATDARD: 2 or ACK_CONTROLED: 3)
				(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP) )	// if Dialup
			{
				memmove(&m_szTemp[1], &m_szTemp[0], tempLen);
				m_szTemp[0] = 0x02;
				m_szTemp[tempLen+1] = 0x03;
				tempLen += 2;
			}
			else	// (type: 2 or 3)
			{
				memmove(&m_szTemp[2], &m_szTemp[0], tempLen);
				m_szTemp[0] = (char)((tempLen& 0xFF00) >> 8);
				m_szTemp[1] = (char)(tempLen & 0x00FF);
				tempLen += 2;
			}

			m_pDevCmn->fnPIN_MacingData(MakeUnPack(m_szTemp, (int)tempLen));
			CString PinMacingData = m_pDevCmn->fstrPIN_GetMacingData();

			memset(m_szTemp, NULL, sizeof(m_szTemp));
			WideToMulti(m_szTemp, PinMacingData, PinMacingData.GetLength());

			if(m_strMacResult_8.Left(8).CompareNoCase(L"        ") == 0)		// host MAC result check error일경우 configuration 만 수행 reversal 하지 않는다.
			{
				if(m_pDevCmn->fnMCU_IsEmvTransaction())		// [#397] [NH] psc 2008.08.01 for EMV transaction
				{
					// [#2496] US Justin 2017.08.15 Add Bitcoin
					if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_TDL_DCC_TRANSACTION || TranCode == TC_JUSTCASH_BITCOIN || TranCode == TC_DIGITALMINT)
					{
						if (m_bOnlineProcessing == FALSE)
						{
							// KSK 2010.08.29 MAC ERROR인 경우 Default 처리 추가
							m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

							if (m_pDevCmn->fnEMV_Trans_OnlineProcess(0, AAC) == EMV_RSLT_OK)
							{
								m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
								m_pDevCmn->fnEMV_Trans_Completion();
							}

							BIZ_EMV_MakeICDataforReversal();	// KSK 2009.8.31 송신 에러시에 

							m_bOnlineProcessing = TRUE;
						}
					}
				}

				// [#289] NH PSC 2008.06.12 MAC, CRC reversal 처리 수정
				if(TranCode == TC_REVERSAL)
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_CONF_TRAN);	// reversal flag가 2이면 configuration만 한다.
				else
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_CONF);	// reversal flag가 2이면 configuration만 한다.

				m_pDevCmn->fnAPL_StackError(_T("D00C1"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003035), DEV_NET);	// [#419] [NH] KSK 2008.9.16

				if(TranCode != TC_REVERSAL)	//	[#12] NH KGS 2008.05.22 Reversal 제외
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 4);	// [#12] NH PSC 2008.03.24 reson for reversal 값 셋팅.

				return RES_HOST_RECV_ERR;
			}
			else if (PinMacingData.Left(8).CompareNoCase(m_strMacResult_8.Left(8)) != 0)
			{
				// [#2496] US Justin 2017.08.15 Add Bitcoin
				if(m_pDevCmn->fnMCU_IsEmvTransaction())		// [#397] [NH] psc 2008.08.01 for EMV transaction
				{
					if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_TDL_DCC_TRANSACTION || TranCode == TC_JUSTCASH_BITCOIN || TranCode == TC_DIGITALMINT)
					{
						if (m_bOnlineProcessing == FALSE)
						{
							// KSK 2010.08.29 MAC ERROR인 경우 Default 처리 추가
							m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

							if (m_pDevCmn->fnEMV_Trans_OnlineProcess(0, AAC) == EMV_RSLT_OK)
							{
								m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
								m_pDevCmn->fnEMV_Trans_Completion();
							}

							BIZ_EMV_MakeICDataforReversal();	// KSK 2009.8.31 송신 에러시에 

							m_bOnlineProcessing = TRUE;
						}
					}
				}

				if (LIB_IsReversalCondition())				// [#439] NH AIREAT 2008.10.21
				{
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_CONF_TRAN);	// reversal flag가 3이면 configuration 후 reversal을 한다.
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 4);				// [#12] NH PSC 2008.03.24 reson for reversal 값 셋팅.
				}
				else if(TranCode == TC_REVERSAL)
				{
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_CONF_TRAN);	// reversal flag가 3이면 configuration 후 reversal을 한다.
				}
				// [#362] NH AIREAT 2008.07.14 - 
				else if(TranCode == TC_TRANSFER || TranCode == TC_INQUIRY)
				{
					MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_CONF);	// reversal flag가 2이면 configuration 만 한다.
				}
				// Day Total, Trial Day Total 시 _MEM_VAR_APP_REVERSALFLAG가 아닌 OpenFlag로 Configuration 처리.
				else if (TranCode == TC_TOTAL || TranCode == TC_TRIALTOTAL)
				{					
					m_pDevCmn->HostOpenFlag = FALSE;
					m_pDevCmn->HostOpenRetryTime = 0;					// KSK 2009.08.10 보완처리 Clear
				}
				// end of [#362]

				m_pDevCmn->fnAPL_StackError(_T("D00C2"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003032), DEV_NET);	// [#419] [NH] KSK 2008.9.16
				return RES_HOST_RECV_ERR;
			}
		}
		else
		{
			if(m_pDevCmn->fnMCU_IsEmvTransaction())		// [#397] [NH] psc 2008.08.01 for EMV transaction
			{
				// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
				if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_TDL_DCC_TRANSACTION || TranCode == TC_JUSTCASH_BITCOIN || TranCode == TC_DIGITALMINT)
				{
					if (m_bOnlineProcessing == FALSE)
					{
						// KSK 2010.08.29 MAC ERROR인 경우 Default 처리 추가
						m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

						if (m_pDevCmn->fnEMV_Trans_OnlineProcess(0, AAC) == EMV_RSLT_OK)
						{
							m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
							m_pDevCmn->fnEMV_Trans_Completion();
						}

						BIZ_EMV_MakeICDataforReversal();	// KSK 2009.8.31 송신 에러시에 

						m_bOnlineProcessing = TRUE;
					}
				}
			}

			if (LIB_IsReversalCondition())				// [#439] NH AIREAT 2008.10.21
			{
				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_CONF_TRAN);	// reversal flag가 3이면 configuration 후 reversal을 수행한다.
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 4);				// [#12] NH PSC 2008.03.24 reson for reversal 값 셋팅.
			}
			else if(TranCode == TC_REVERSAL)
			{
				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_CONF_TRAN);	// reversal flag가 3이면 configuration 후 reversal을 한다.
			}
			// [#362] NH AIREAT 2008.07.14 - 
			// Day Total, Trial Day Total 시 _MEM_VAR_APP_REVERSALFLAG가 아닌 OpenFlag로 Configuration 처리.
			else if(TranCode == TC_TRANSFER || TranCode == TC_INQUIRY)
			{
				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_CONF);		// reversal flag가 3이면 configuration 후 reversal을 한다.
			}
			else if (TranCode == TC_TOTAL || TranCode == TC_TRIALTOTAL)
			{

				m_pDevCmn->HostOpenFlag = FALSE;
				m_pDevCmn->HostOpenRetryTime = 0;					// KSK 2009.08.10 보완처리 Clear
			}
			// end of [#362]
			m_pDevCmn->fnAPL_StackError(_T("D00C2"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003036), DEV_NET);	// [#419] [NH] KSK 2008.9.16
			return RES_HOST_RECV_ERR;
		}
	}
#endif  // end of [#321] #ifndef APP_LOCAL_MODE

	switch (TranCode)
	{
		case TC_OPEN:
		case TC_HEALTHCHK:
			nRes = BIZ_STD3_AnalConfigMsg(Cnt);
			break;
		case TC_WITHDRAWAL:
		case TC_INQUIRY:
		case TC_TRANSFER:
		case TC_POPMONEY:			// [#2350] US Justin 2015.06.19 Add POP MONEY
		case TC_JUSTCASH:			// [#2448] US Justin 2016.11.21 Add Just.Cash
		case TC_JUSTCASH_BITCOIN:	// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
		case TC_PIN4:				// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
		case TC_DIGITALMINT:
			nRes = BIZ_STD3_AnalTranMsg(Cnt);
			break;
		case TC_REVERSAL:
			nRes = BIZ_STD3_AnalReversalMsg(Cnt);
			break;
		case TC_TOTAL:
		case TC_TRIALTOTAL:
			nRes = BIZ_STD3_AnalTotalMsg(Cnt);
			break;
		// [#2292] US Justin 2014.09.25 Add TDL FLOW
		case TC_TDL_TIRSURCHARGE:
		case TC_TDL_DCC_LOOKUP:
		case TC_TDL_DCC_TRANSACTION:
		case TC_TDL_DCC_REVERSAL:
			nRes = BIZ_STD3_AnalTDLMsg(Cnt);
			break;
		// End of [#2292]
		default:
			break;
	}

	return nRes;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_AnalConfigMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::BIZ_STD3_AnalConfigMsg(int nIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_AnalConfigMsg]\n"));

	int nKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);

	// 1.Information Header
	// NB: LibertyX currently does not support Comm Header
	if (m_HostConfig != HC_LIBERTYX) // [#RWC6-59] US William 2019.10.09 LibertyX
	{
		if ( MemGetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE) == 1) 
		{
			if (m_strArrRecvData.GetSize() > nIndex)
				m_sSTD3_ConfigResp.m_strInformation_Header_12 = m_strArrRecvData[nIndex++];
		}
	}
	
	// 2.Terminal ID 15Alphanumeric
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_ConfigResp.m_strTerminalID_15 = m_strArrRecvData[nIndex++];
	// 3.Transaction Code
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_ConfigResp.m_strTransactionCode_2 = m_strArrRecvData[nIndex++];

#ifdef	APP_LOCAL_MODE	// Local Mode시 Error Check 없음
	return RES_OK;
#endif

	// [#266] [MX] KSK 2008.6.9
	// Response Value Check Logic 수정
	if( m_sSTD3_CommHeader.m_strTerminalID_15 != m_sSTD3_ConfigResp.m_strTerminalID_15)
	{
		// Terminal ID Error Check
		m_pDevCmn->fnAPL_StackError(_T("D00B000"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003019), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;					
	}

	if( m_sSTD3_ConfigReq.m_strTransactionCode_2 != m_sSTD3_ConfigResp.m_strTransactionCode_2)
	{
		// Transaction Code Error Check
		m_pDevCmn->fnAPL_StackError(_T("D00B100"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003020), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;
	}

	BIZ_STD3_AnalMiscellaneousField(nIndex);

	if (nKeyMode == KEYMODE_TR31 || nKeyMode == KEYMODE_TR31_MACING || nKeyMode == KEYMODE_TR31_TMACING)
	{
		if( m_sSTD3_ConfigResp.m_strFieldIDCode6_1 != L"<" )
		{
			// Working Key1 Error Check
			m_pDevCmn->fnAPL_StackError(_T("D00B500"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003037), DEV_NET);	// [#419] [NH] KSK 2008.9.16
			return RES_HOST_RECV_ERR;
		}
	}
	else 
	{
		if( m_sSTD3_ConfigResp.m_strFieldIDCode1_1 != L"~" )
		{
			// Working Key1 Error Check
			m_pDevCmn->fnAPL_StackError(_T("D00B200"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003037), DEV_NET);	// [#419] [NH] KSK 2008.9.16
			return RES_HOST_RECV_ERR;
		}

		if( m_sSTD3_ConfigResp.m_strEncryptedPINKey_16.GetLength() != 16 )
		{
			m_pDevCmn->fnAPL_StackError(_T("D00B300"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003021), DEV_NET);	// [#419] [NH] KSK 2008.9.16
			return RES_HOST_RECV_ERR;
		}

		if( m_sSTD3_ConfigResp.m_strFieldIDCode3_1 == '!')
		{
			// Surcharge Amount Error Check
			if( m_sSTD3_ConfigResp.m_strSurchargeAmount_8.GetLength() != 8 )
			{
				m_pDevCmn->fnAPL_StackError(_T("D00B400"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003038), DEV_NET);	// [#419] [NH] KSK 2008.9.16
				return RES_HOST_RECV_ERR;
			}
		}
	}

	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_AnalTranMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD3_AnalTranMsg(int nIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_AnalTranMsg]\n"));

	int			nResponseCodeIndex = -1;
	int			nEMVResult = 0;
	BOOL		fReversal = FALSE;	// V01.02.25 SRC-1
	BOOL		fCancel = FALSE;	// V01.02.25 SRC-1

	// Before checking Transaction Message, Set the Transaction Error by default
	// 1.Information Header
	// NB: LibertyX currently does not support Comm Header
	if (m_HostConfig != HC_LIBERTYX) // [#RWC6-59] US William 2019.10.09 LibertyX
	{
		if ( MemGetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE) == 1) 
		{
			if (m_strArrRecvData.GetSize() > nIndex)
				m_sSTD3_TranResp.m_strInformation_Header_12 = m_strArrRecvData[nIndex++];
		}
	}

	// Multi-Block Indicator
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_TranResp.m_strMulti_Block_Indicator_1 = m_strArrRecvData[nIndex++];

	// Terminal ID Check
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_TranResp.m_strTerminalID_15 = m_strArrRecvData[nIndex++];

	// Transaction Code Error Check
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_TranResp.m_strTransactionCode_2 = m_strArrRecvData[nIndex++];

	// Sequence Number 
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_TranResp.m_strSequenceNumber_4 = m_strArrRecvData[nIndex++];

	// Transaction Response Code Error Check
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_TranResp.m_strResponseCode_3 = m_strArrRecvData[nIndex++];

	// Authorization Number Error Check
	if (m_strArrRecvData.GetSize() > nIndex)
	{
		m_sSTD3_TranResp.m_strAuthorizationNum_8 = m_strArrRecvData[nIndex++];
		// Triton인 경우 Auditnum / netid가 존재하지 않으므로, authorizationNum을 짤라서 넣는다.
		// journal에 std1과 같게하기 위함.
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM, m_sSTD3_TranResp.m_strAuthorizationNum_8.Left(6));
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID, m_sSTD3_TranResp.m_strAuthorizationNum_8.Right(2));
	}

	// Transaction Date MMDDYY (Set by Processor) Error Check
	if (m_strArrRecvData.GetSize() > nIndex)
	{
		m_sSTD3_TranResp.m_strTransactionDate_6 = m_strArrRecvData[nIndex++];
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE, m_sSTD3_TranResp.m_strTransactionDate_6);
	}
	// Transaction Time HHMMSS  TransactionTime
	if (m_strArrRecvData.GetSize() > nIndex)
	{
		m_sSTD3_TranResp.m_strTransactionTime_6 = m_strArrRecvData[nIndex++];
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME, m_sSTD3_TranResp.m_strTransactionTime_6);
	}

	// BUSINSES DATE Error Check MMDDYY, AU - DDMMYY
	if (m_strArrRecvData.GetSize() > nIndex)
	{
		m_sSTD3_TranResp.m_strBusinessDate_6 = m_strArrRecvData[nIndex++];
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSSETTLEDATE, m_sSTD3_TranResp.m_strBusinessDate_6);
	}

	// [#196] NH 2008.05.15 JSW Response Code가 000인지 체크하는 부분 수신 데이터 정합성 체크 이후로 위치 이동

	// Amount1 Balance amount
	if (m_strArrRecvData.GetSize() > nIndex)
	{
		m_sSTD3_TranResp.m_strAmount1_8 = m_strArrRecvData[nIndex++];
		MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT, m_sSTD3_TranResp.m_strAmount1_8);
	}

	// Amount2 Amount of Actual Surcharge
	// An ASCII minus sign (e.g."-0000100") in the first position represents a credit.
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_TranResp.m_strAmount2_8 = m_strArrRecvData[nIndex++];

	if (m_strArrRecvData.GetSize() > nIndex)
	{
		nEMVResult = BIZ_STD3_AnalMiscellaneousField(nIndex);	// [#492] [NH] KSK 2009.2.5
	}

	if(m_pDevCmn->fnMCU_IsEmvTransaction())		// [#397] [NH] psc 2008.08.01 for EMV transaction
	{
		// [#2496] US Justin 2017.08.15 Add Just.Cash Bitcoin
		if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_JUSTCASH_BITCOIN || TranCode == TC_DIGITALMINT)
		{
			if(m_strRecvData.Find(L"ud") == -1 || m_strArrRecvData.GetSize() <= nIndex)		// ud field가 없거나 다음 data가 없는 경우 error 처리
			{
				// [#2372] US Justin 2015.09.11 TC for NON EMV Transaction
				/*
				if (m_bOnlineProcessing == FALSE)
				{
					// KSK 2010.08.29 ud 미수신 인 경우 Default 처리 추가
					m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

					if (m_pDevCmn->fnEMV_Trans_OnlineProcess(0, AAC) == EMV_RSLT_OK)
					{
						m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
						m_pDevCmn->fnEMV_Trans_Completion();
					}

					BIZ_EMV_MakeICDataforReversal();	// KSK 2009.8.31 송신 에러시에 

					m_bOnlineProcessing = TRUE;
				}
				nEMVResult = EMV_ERROR_ONLINE_DATA;
				*/
				if (m_bOnlineProcessing == FALSE)
				{
					CString strEMVTag4NonEMVDataCase = _T("8A023030910A00000000000000000000");			// MTIP 06 / 05 / 01 Test Case (8A:3030, 91:00000000000000000000)
					//CString strEMVTag4NonEMVDataCase = _T("8A023030"); // [#RWC6-22] EMV ADVT v7.0 test purpose only
					nEMVResult = BIZ_EMV_Anal_ICDataforTrans(strEMVTag4NonEMVDataCase);
				}
				// End of [#2372]
			}
		}
	}

#ifdef APP_LOCAL_MODE
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT, m_sSTD3_TranResp.m_strAmount2_8);
	return RES_OK;
#endif

	// Response Message Error Checking
	// Terminal ID Check
	if( m_sSTD3_CommHeader.m_strTerminalID_15 != m_sSTD3_TranResp.m_strTerminalID_15)
	{
		m_pDevCmn->fnAPL_StackError(_T("D00B000"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003019), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	// Transaction Code Error Check
	if( m_sSTD3_TranReq.m_strTransactionCode_2 !=m_sSTD3_TranResp.m_strTransactionCode_2)
	{
		m_pDevCmn->fnAPL_StackError(_T("D00B100"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003020), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	// Sequence Number Error Check
	if( m_sSTD3_TranReq.m_strSequenceNumber_4 != m_sSTD3_TranResp.m_strSequenceNumber_4 )
	{
		m_pDevCmn->fnAPL_StackError(_T("D00B500"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003025), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	// Authorization Number Error Check
	if( m_sSTD3_TranResp.m_strAuthorizationNum_8.GetLength() != 8 )
	{
		m_pDevCmn->fnAPL_StackError(_T("D00B700"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003040), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	// Transaction Date MMDDYY (Set by Processor) Error Check
	if( m_sSTD3_TranResp.m_strTransactionDate_6.GetLength() != 6 )
	{
		m_pDevCmn->fnAPL_StackError(_T("D00B800"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003026), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if( m_sSTD3_TranResp.m_strTransactionTime_6.GetLength() != 6 )
	{
		m_pDevCmn->fnAPL_StackError(_T("D00B900"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003027), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	// BUSINSES DATE Error Check MMDDYY
	if( m_sSTD3_TranResp.m_strBusinessDate_6.GetLength() != 6 )
	{
		m_pDevCmn->fnAPL_StackError(_T("D00BA00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003039), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if( m_sSTD3_TranResp.m_strAmount1_8.GetLength() != 8 )
	{
		m_pDevCmn->fnAPL_StackError(_T("D00BB00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003041), DEV_NET);	// [#419] [NH] KSK 2008.9.16

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	if( m_sSTD3_TranResp.m_strAmount2_8.GetLength() != 8 )
	{
		m_pDevCmn->fnAPL_StackError(_T("D00BC00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003038), DEV_NET);	// [#419] [NH] KSK 2008.9.16	

		if (LIB_IsReversalCondition())
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);

		return RES_HOST_RECV_ERR;
	}

	// Transaction Response Code Error Check
	// [#196] JSW 2008.5.15
	if( m_sSTD3_TranResp.m_strResponseCode_3 != "000")
	{
		// [#2291] US Justin 2014.08.27 EMV online Processing is skipped when ATM has USD mode... Need to correct response code for BI
		BOOL bDeniedTran = TRUE;
		
		//#if( (MX_VERSION)&&(APP_TDL_OPTION) )			// [#2340] US Justin Enable Balance Inquiry 085 Logic for MX regardless of TDL_OPTION
		#if( MX_VERSION )
			if( (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)&&(MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG) == MX_1ST_SEND )&& 
				(m_pDevCmn->fnMCU_IsEmvTransaction())&&(TranCode==TC_INQUIRY)&&(m_sSTD3_TranResp.m_strResponseCode_3=="085") )
				bDeniedTran = FALSE;
		#endif

		if(bDeniedTran)
		{
			CString strErrorCode, strErrorMsg;

			MemSetStr(_MEM_FLD_OPTIONALSETTING,_MEM_VAR_OPT_APP_ISSUERFEE, L"000000000");

			if( (nResponseCodeIndex = BIZ_STD3_CheckResponseCode(m_sSTD3_TranResp.m_strResponseCode_3)) != -1 )
			{
				strErrorCode.Format(L"DA0%-3.3S0",(LPCSTR)&TRITON_RSP_TBL[nResponseCodeIndex]);
				CString	strTemp;
				strTemp.Format(_T("%s%02d"), T_APMSG_003200, nResponseCodeIndex);
				strErrorMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(strTemp);
				m_pDevCmn->fnAPL_StackError(strErrorCode, strErrorMsg, DEV_NET); // Triton Error Format	// [#419] [NH] KSK 2008.9.16
			}
			else
			{
				strErrorCode.Format(L"DA0%-3.3s0",m_sSTD3_TranResp.m_strResponseCode_3);
				m_pDevCmn->fnAPL_StackError(strErrorCode, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003078), DEV_NET);	// [#419] [NH] KSK 2008.9.16

			}
			return RES_HOST_RECV_ERR;
		}
		// End of [#2291]
	}

	// Reversal Transaction Data Set
	if (LIB_IsReversalCondition())
	{
		// REVERSAL FLAG SET
		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
	}

	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT, m_sSTD3_TranResp.m_strAmount2_8);	// [#196] [NH] 2008.05.15 JSW 데이터 정합성 체크 후 Host측 Surcharge 설정

	// EMV ERROR시에 Cancel Proc을 태우도록 한다 (Error Set도 해야함)
	if (m_pDevCmn->fnMCU_IsEmvTransaction() && (nEMVResult != EMV_TR_SUCCESS))
	{
		if (nEMVResult == EMV_ERROR_ONLINE_DATA)
		{
			/* EMV ERROR CODE STACK */ /* 9730100 */
			// EMV On-line Script result is reversal 
			m_pDevCmn->fnAPL_StackError(L"9730100", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003087), DEV_MCU);	// KSK 2010.04.05 "IC CARD ERROR"로 문구 통일
		}
		else
		{
			/* EMV ERROR CODE STACK */ /* 9730200 */
			// EMV On-line Script result is reversal
			m_pDevCmn->fnAPL_StackError(L"9730200", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003087), DEV_MCU);	// KSK 2010.04.05 "IC CARD ERROR"로 문구 통일
		}

		/* Setting Reason for reversal : ICC DECLINED */
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 7);	// 캐나다 OpenSolution과 협의함 (2 -> 7 수정)

		return RES_EMV_ERROR_REVERSAL;
	}

	//////////////////////////////////////////////////////////////////////////
	//	Transaction Response Message No Error! Normal Transaction 
	//////////////////////////////////////////////////////////////////////////
	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_AnalReversalMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD3_AnalReversalMsg(int nIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_AnalReversalMsg]\n"));

	// 1.Information Header
	// NB: LibertyX currently does not support Comm Header
	if ( MemGetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE) == 1
		&& m_HostConfig != HC_LIBERTYX) // [#RWC6-59] US William 2019.10.09 LibertyX 
	{
		nIndex++;
	}

	// Terminal ID
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_ReversalResp.m_strTerminalID_15 = m_strArrRecvData[nIndex++];

	// Transaction Code
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_ReversalResp.m_strTransactionCode_2 = m_strArrRecvData[nIndex++];	

	// Sequence Number
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_ReversalResp.m_strSequenceNumber_4 = m_strArrRecvData[nIndex++];	

	// Transaction Response Code
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_ReversalResp.m_strResponseCode_3 = m_strArrRecvData[nIndex++];	

	// [#266] [MX] KSK 2008.6.9

#ifdef	APP_LOCAL_MODE	// Local Mode시 Error Check 없음
	return RES_OK;
#endif

	if( m_sSTD3_CommHeader.m_strTerminalID_15 != m_sSTD3_ReversalResp.m_strTerminalID_15)
	{
		// Terminal ID Error Check
		m_pDevCmn->fnAPL_StackError(_T("D00B000"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003019), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		
		return RES_HOST_RECV_ERR;
	}

	if( m_sSTD3_ReversalReq.m_strTransactionCode_2 != m_sSTD3_ReversalResp.m_strTransactionCode_2 )
	{
		// Transaction Code Error Check
		m_pDevCmn->fnAPL_StackError(_T("D00B100"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003019), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;
	}

	if( m_sSTD3_ReversalReq.m_strSequenceNumber_4 != m_sSTD3_ReversalResp.m_strSequenceNumber_4)
	{
		// Sequence Number Error Check
		m_pDevCmn->fnAPL_StackError(_T("D00B500"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003025), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;
	}

	BIZ_STD3_AnalMiscellaneousField(nIndex);

	if( m_sSTD3_ReversalResp.m_strResponseCode_3 != "000" )
	{
		CString strErrorCode, strErrorMsg;
		INT		nResponseCodeIndex = -1;

		if( (nResponseCodeIndex = BIZ_STD3_CheckResponseCode(m_sSTD3_ReversalResp.m_strResponseCode_3)) != -1 )
		{
			strErrorCode.Format(L"DA0%-3.3S0",(LPCSTR)&TRITON_RSP_TBL[nResponseCodeIndex]);
			// [#310] [MX] KSK 2008.6.18
			CString	strTemp;
			strTemp.Format(_T("%s%02d"), T_APMSG_003200, nResponseCodeIndex);
			strErrorMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(strTemp);
			// end of [#310]
			m_pDevCmn->fnAPL_StackError(strErrorCode, strErrorMsg, DEV_NET); // Triton Error Format	// [#419] [NH] KSK 2008.9.16
		}
		else
		{
			strErrorCode.Format(L"DA0%-3.3s0",m_sSTD3_ReversalResp.m_strResponseCode_3);
			m_pDevCmn->fnAPL_StackError(strErrorCode, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003078), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		}
		
		return RES_HOST_RECV_ERR;
	}

	////////////////////////////////////////////////////////////////////////
	// Reversal Response Message No Error! 
	////////////////////////////////////////////////////////////////////////
	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_AnalTotalMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD3_AnalTotalMsg(int nIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_AnalTotalMsg]\n"));

	// 1.Information Header
	// NB: LibertyX currently does not support Comm Header
	if ( MemGetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE) == 1
		&& m_HostConfig != HC_LIBERTYX ) // [#RWC6-59] US William 2019.10.09 LibertyX 
	{
		nIndex++;
	}

	// Terminal ID Check
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_TotalResp.m_strTerminalID_15 = m_strArrRecvData[nIndex++];

	// Transaction Code
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_TotalResp.m_strTransactionCode_2 = m_strArrRecvData[nIndex++];	

	// BUSINESS DATE
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sSTD3_TotalResp.m_strBusinessDate_6 = m_strArrRecvData[nIndex++];	

	// Number of Withdrawals : Total number of withdrawals
	// since the last request for totals.
	if (m_strArrRecvData.GetSize() > nIndex)
	{
		m_sSTD3_TotalResp.m_strNumberOfWithdrawals_4 = m_strArrRecvData[nIndex].Mid(0, 4);
		m_sSTD3_TotalResp.m_strNumberOfInquiries_4 = m_strArrRecvData[nIndex].Mid(4, 4);
		m_sSTD3_TotalResp.m_strNumberOfTransfers_4 = m_strArrRecvData[nIndex].Mid(8, 4);
		m_sSTD3_TotalResp.m_strSettlement_8 = m_strArrRecvData[nIndex++].Mid(12, 8);
	}

	// [#266] [MX] KSK 2008.6.9
#ifdef	APP_LOCAL_MODE	// Local Mode시 Error Check 없음
	return RES_OK;
#endif

	// KSK 2010.02.27 Host Total시 Error 발생 시 방안 필요 (OP에 Error Code Display?)
	if( m_sSTD3_CommHeader.m_strTerminalID_15 != m_sSTD3_TotalResp.m_strTerminalID_15)
	{
		// Terminal ID Error Check
		m_pDevCmn->fnAPL_StackError(_T("D00B000"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003019), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;
	}

	if( m_sSTD3_TotalReq.m_strTransactionCode_2 != m_sSTD3_TotalResp.m_strTransactionCode_2 )
	{
		// Transaction Code Error Check
		m_pDevCmn->fnAPL_StackError(_T("D00B100"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003020), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;
	}

	if( m_sSTD3_TotalResp.m_strBusinessDate_6.GetLength() != 6 )
	{
		// BUSINESS DATE Error Check MMDDYY
		m_pDevCmn->fnAPL_StackError(_T("D00BF00"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003039), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;
	}

	if (IsNum(m_sSTD3_TotalResp.m_strSettlement_8) != TRUE )	// KSK 2010.01.04
	{
		// Settlement : Total amount of all withdrawals.
		// Represents the amount in the smallest possible unit of currency.
		// m_strNumberOfWithdrawals_4 , m_strNumberOfInquiries_4, m_strNumberOfTransfers_4값은
		m_pDevCmn->fnAPL_StackError(_T("D00C000"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003031), DEV_NET);	// [#419] [NH] KSK 2008.9.16
		return RES_HOST_RECV_ERR;
	}

	//////////////////////////////////////////////////////////////////////////
	//	Host Total Response Message No Error! 
	//////////////////////////////////////////////////////////////////////////

	BIZ_STD3_AnalMiscellaneousField(nIndex);

	m_pDevCmn->TranResult = TRUE;						// Host Ok
	MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);

	return RES_OK;
}

// [#2292] US Justin 2014.09.30 Add TDL
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_AnalTDLMsg()
RETURN TYPE  : 
PARAMETER    : int nIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_STD3_AnalTDLMsg(int nIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_AnalTDLMsg]\n"));
    
	int	nEMVResult = EMV_TR_SUCCESS;
	CString strTID = _T("");
	CString strTranCode = _T("");
	CString strResCode = _T("");

	// 1.Information Header
	// NB: LibertyX currently does not support Comm Header
	if ( MemGetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE) == 1
		&& m_HostConfig != HC_LIBERTYX ) // [#RWC6-59] US William 2019.10.09 LibertyX 
	{
		nIndex++;
	}

	// 00. Multi-Block Indicator
	if (m_strArrRecvData.GetSize() > nIndex)		nIndex++;

	// 01. Terminal ID 
	if (m_strArrRecvData.GetSize() > nIndex)		strTID = m_strArrRecvData[nIndex++];

	// 02. Transaction Code
	if (m_strArrRecvData.GetSize() > nIndex)		strTranCode = m_strArrRecvData[nIndex++];

	// 03. Sequence Number => SKIP : Not Available
	if (m_strArrRecvData.GetSize() > nIndex)		nIndex++;

	// 04. Transaction Response Code : Available Only for Surcharge Segmentation
	if (m_strArrRecvData.GetSize() > nIndex)		strResCode = m_strArrRecvData[nIndex++];

	// 05. Authorization Number Error Check => SKIP : Not Available
	if (m_strArrRecvData.GetSize() > nIndex)		nIndex++;

	// 06. Transaction Date MMDDYY (Set by Processor) Error Check : SKIP, Available only for Surcharge Segmentation
	if (m_strArrRecvData.GetSize() > nIndex)		nIndex++;

	// 07. Transaction Time HHMMSS  TransactionTime : SKIP, Available only for Surcharge Segmentation
	if (m_strArrRecvData.GetSize() > nIndex)		nIndex++;

	// 08. BUSINSES DATE Error Check MMDDYY, AU - DDMMYY : SKIP, Available only for Surcharge Segmentation
	if (m_strArrRecvData.GetSize() > nIndex)		nIndex++;

	// 09. Amount1 Balance amount => SKIP : Not Available
	if (m_strArrRecvData.GetSize() > nIndex)		nIndex++;

	// 10. Amount2 Amount of Actual Surcharge => SKIP : Not Available
	if (m_strArrRecvData.GetSize() > nIndex)		nIndex++;

	// TIR Surcharge => CHeck Error before Parsing Mis Fields.  (NO NEED EMV PROCESSING)
	if( TranCode==TC_TDL_TIRSURCHARGE )
	{
		if( (m_sSTD3_CommHeader.m_strTerminalID_15!=strTID) || (strTranCode!=L"97") || (strResCode!="000") )
		{
			NHDEBUG(DBG_CALL, (L"ERROR : TID[%s] or TranCode[%s] or ResCode[%s]\n", strTID, strTranCode, strResCode));
			return RES_HOST_RECV_ERR;
		}
	}
	// Parsing Misc. Field "qa:Surcharge Segmentation" and "qc:DCC DAta"
	if (m_strArrRecvData.GetSize() > nIndex)		
		BIZ_STD3_AnalMiscellaneousField(nIndex);

	if( TranCode==TC_TDL_TIRSURCHARGE )
	{
		// [#2299] US Justin 2014.10.24  cancel transaction if ATM does not get a surcharge
		if(m_STD3_TDL_Data.m_bTranResult != TRUE)
		{
			NHDEBUG(DBG_CALL, (L"ERROR : Dynamic Surcuarege Parse Error\n"));
			return RES_HOST_RECV_ERR;
		}		
		// End of [#2299]

		m_pDevCmn->TranResult = TRUE;
		MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);
		return RES_OK;
	}
	else if( TranCode==TC_TDL_DCC_LOOKUP )
	{
		if( (m_STD3_TDL_Data.m_bTranResult != TRUE)||(strTranCode!=L"97") )
		{
			NHDEBUG(DBG_CALL, (L"ERROR : DCC LOOK UP Not Parse Error or Trancode[%s] Error\n", strTranCode));
			return RES_HOST_RECV_ERR;
		}		
		m_pDevCmn->TranResult = TRUE;
		MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);
		return RES_OK;
	}
	else if( TranCode == TC_TDL_DCC_TRANSACTION)
	{
		// Fill Legacy Data Buffer when communication is successful.
		if( (m_STD3_TDL_Data.m_bTranResult == TRUE)&&(strTranCode==L"97") )
		{
			// Terminal ID
			m_sSTD3_TranResp.m_strTerminalID_15 = strTID;
			// Transaction Code
			m_sSTD3_TranResp.m_strTransactionCode_2 = m_sSTD3_TranReq.m_strTransactionCode_2;
			// Sequence Number 
			m_sSTD3_TranResp.m_strSequenceNumber_4.Format(L"%04d", Asc2Int(m_STD3_TDL_Data.m_strSeqNum));
			// Transaction Response Code
			if(m_STD3_TDL_Data.m_DCC_Transaction.m_strRes_AuthCode.GetLength()>0)
				m_sSTD3_TranResp.m_strResponseCode_3.Format(L"%03d", Asc2Int(m_STD3_TDL_Data.m_DCC_Transaction.m_strRes_AuthCode) );
			// Authorization Number Error Check
			m_sSTD3_TranResp.m_strAuthorizationNum_8.Format(L"%08s", m_STD3_TDL_Data.m_DCC_Transaction.m_strRes_AuthNumber);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM, m_sSTD3_TranResp.m_strAuthorizationNum_8.Left(6));
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID, m_sSTD3_TranResp.m_strAuthorizationNum_8.Right(2));
			// Transaction Date MMDDYY, Time HHMMSS
			if (m_STD3_TDL_Data.m_DCC_Transaction.m_strRes_TranDateTime.GetLength()==12)
			{
				m_sSTD3_TranResp.m_strTransactionDate_6 = m_STD3_TDL_Data.m_DCC_Transaction.m_strRes_TranDateTime.Left(6);
				m_sSTD3_TranResp.m_strTransactionTime_6 = m_STD3_TDL_Data.m_DCC_Transaction.m_strRes_TranDateTime.Right(6);
			}
			else
			{
				m_sSTD3_TranResp.m_strTransactionDate_6 = _T("000000");
				m_sSTD3_TranResp.m_strTransactionTime_6 = _T("000000");
			}
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE, m_sSTD3_TranResp.m_strTransactionDate_6);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME, m_sSTD3_TranResp.m_strTransactionTime_6);
			// BUSINSES DATE Error Check MMDDYY
			if (m_STD3_TDL_Data.m_DCC_Transaction.m_strRes_BusDate.GetLength()==10)
			{
				CString sBD = m_STD3_TDL_Data.m_DCC_Transaction.m_strRes_BusDate;
				m_sSTD3_TranResp.m_strBusinessDate_6.Format(L"%s%s%s", sBD.Mid(5,2), sBD.Mid(8,2), sBD.Mid(2,2) );
			}
			else
				m_sSTD3_TranResp.m_strBusinessDate_6 = _T("000000");
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSSETTLEDATE, m_sSTD3_TranResp.m_strBusinessDate_6);
			// Amount 1 Balance....
			m_sSTD3_TranResp.m_strAmount1_8 = _T("        ");		// Not available
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT, m_sSTD3_TranResp.m_strAmount1_8);
			// Amount 2 Actual Surcharge
			m_sSTD3_TranResp.m_strAmount2_8.Format(L"%08d", Dollar2Cent(m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_SurchargeAmount) );
		}

		// Checkk EMV Data and process
		if( m_pDevCmn->fnMCU_IsEmvTransaction() )
		{
			if( m_STD3_TDL_Data.m_DCC_Transaction.m_strRes_EMVData.GetLength() <= 0 )
			{
				// [#2372] US Justin 2015.09.11 TC for NON EMV Transaction
				/*
				NHDEBUG(DBG_CALL, (L" ERROR - TDL EMV DCC Transaction => EMV DATA ERROR\n"));
				if (m_bOnlineProcessing == FALSE)
				{
					m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);
					if (m_pDevCmn->fnEMV_Trans_OnlineProcess(1, AAC) == EMV_RSLT_OK)
					{
						m_pDevCmn->m_bDisplayDeclined = TRUE;
						m_pDevCmn->fnEMV_Trans_Completion();
					}
					BIZ_EMV_MakeICDataforReversal();			// NVRAM SET
					m_bOnlineProcessing = TRUE;
				}
				nEMVResult = EMV_ERROR_ONLINE_DATA;
				*/

				CString strEMVTag4NonEMVDataCase = _T("8A023030910A00000000000000000000");			// MTIP 06 / 05 / 01 Test Case (8A:3030, 91:00000000000000000000)
				//CString strEMVTag4NonEMVDataCase = _T("8A023030"); // [#RWC6-22] EMV ADVT v7.0 test purpose only
				nEMVResult = BIZ_EMV_Anal_ICDataforTrans(strEMVTag4NonEMVDataCase);
				// End of [#2372]
			}
			else
				nEMVResult = BIZ_EMV_Anal_ICDataforTrans(m_STD3_TDL_Data.m_DCC_Transaction.m_strRes_EMVData);
		}

		// Check TID
		if( m_sSTD3_CommHeader.m_strTerminalID_15 != m_sSTD3_TranResp.m_strTerminalID_15)
		{
			NHDEBUG(DBG_CALL, (L"TID ERROR req[%s], rcv[%s]\n", m_sSTD3_CommHeader.m_strTerminalID_15, m_sSTD3_TranResp.m_strTerminalID_15));
			m_pDevCmn->fnAPL_StackError(_T("D00B000"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003019), DEV_NET);
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
			return RES_HOST_RECV_ERR;
		}
		// Sequence Number Error Check
		if( m_sSTD3_TranReq.m_strSequenceNumber_4 != m_sSTD3_TranResp.m_strSequenceNumber_4 )
		{
			NHDEBUG(DBG_CALL, (L"Seq Num. ERROR req[%s], rcv[%s]\n", m_sSTD3_TranReq.m_strSequenceNumber_4, m_sSTD3_TranResp.m_strSequenceNumber_4));
			m_pDevCmn->fnAPL_StackError(_T("D00B500"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003025), DEV_NET);
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
			return RES_HOST_RECV_ERR;
		}
		// Authorization Number Error Check ... Not sure the length
		if( m_sSTD3_TranResp.m_strAuthorizationNum_8.GetLength() != 8 )
		{
			NHDEBUG(DBG_CALL, (L"Auth Number Error [%s]\n",  m_sSTD3_TranResp.m_strAuthorizationNum_8));
			m_pDevCmn->fnAPL_StackError(_T("D00B700"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003040), DEV_NET);
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
			return RES_HOST_RECV_ERR;
		}
		// Response Code
		if( m_sSTD3_TranResp.m_strResponseCode_3 != "000")
		{
			CString strErrorCode, strErrorMsg;
			MemSetStr(_MEM_FLD_OPTIONALSETTING,_MEM_VAR_OPT_APP_ISSUERFEE, L"000000000");
			int nResponseCodeIndex = -1;
			if( (nResponseCodeIndex = BIZ_STD3_CheckResponseCode(m_sSTD3_TranResp.m_strResponseCode_3)) != -1 )
			{
				strErrorCode.Format(L"DA0%-3.3S0",(LPCSTR)&TRITON_RSP_TBL[nResponseCodeIndex]);
				CString	strTemp;
				strTemp.Format(_T("%s%02d"), T_APMSG_003200, nResponseCodeIndex);
				strErrorMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(strTemp);
				m_pDevCmn->fnAPL_StackError(strErrorCode, strErrorMsg, DEV_NET);
			}
			else
			{
				strErrorCode.Format(L"DA0%-3.3s0",m_sSTD3_TranResp.m_strResponseCode_3);
				m_pDevCmn->fnAPL_StackError(strErrorCode, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003078), DEV_NET);

			}
			return RES_HOST_RECV_ERR;
		}
		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);
		MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT, m_sSTD3_TranResp.m_strAmount2_8);
		if (m_pDevCmn->fnMCU_IsEmvTransaction() && (nEMVResult != EMV_TR_SUCCESS))
		{
			if (nEMVResult == EMV_ERROR_ONLINE_DATA)	m_pDevCmn->fnAPL_StackError(L"9730100", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003087), DEV_MCU);
			else										m_pDevCmn->fnAPL_StackError(L"9730200", m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003087), DEV_MCU);
			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 7);
			return RES_EMV_ERROR_REVERSAL;
		}

		m_pDevCmn->TranResult = TRUE;						// Host Ok
		MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);
		return RES_OK;
	}
	else if(TranCode == TC_TDL_DCC_REVERSAL)
	{
		// Fill Legacy Data Buffer
		if( (m_STD3_TDL_Data.m_bTranResult == TRUE)&&(strTranCode==L"97") )
		{
			// Terminal ID
			m_sSTD3_ReversalResp.m_strTerminalID_15 = strTID;
			// Transaction Code
			m_sSTD3_ReversalResp.m_strTransactionCode_2 = m_sSTD3_ReversalReq.m_strTransactionCode_2;
			// Sequence Number 
			m_sSTD3_ReversalResp.m_strSequenceNumber_4.Format(L"%04d", Asc2Int(m_STD3_TDL_Data.m_strSeqNum));

			// Transaction Response Code
			if(m_STD3_TDL_Data.m_DCC_Transaction.m_strRes_AuthCode.GetLength()>0)
				m_sSTD3_ReversalResp.m_strResponseCode_3.Format(L"%03d", Asc2Int(m_STD3_TDL_Data.m_DCC_Reversal.m_strRes_AuthCode) );
		}

		// Check TID
		if( m_sSTD3_CommHeader.m_strTerminalID_15 != m_sSTD3_ReversalResp.m_strTerminalID_15)
		{
			NHDEBUG(DBG_CALL, (L"TID ERROR req[%s], rcv[%s]\n", m_sSTD3_CommHeader.m_strTerminalID_15, m_sSTD3_ReversalResp.m_strTerminalID_15));
			m_pDevCmn->fnAPL_StackError(_T("D00B000"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003019), DEV_NET);
			return RES_HOST_RECV_ERR;
		}
		// Sequence Number Error Check
		if( m_sSTD3_ReversalReq.m_strSequenceNumber_4 != m_sSTD3_ReversalResp.m_strSequenceNumber_4)
		{
			NHDEBUG(DBG_CALL, (L"Seq Num. ERROR req[%s], rcv[%s]\n", m_sSTD3_ReversalReq.m_strSequenceNumber_4, m_sSTD3_ReversalResp.m_strSequenceNumber_4));
			m_pDevCmn->fnAPL_StackError(_T("D00B500"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003025), DEV_NET);
			return RES_HOST_RECV_ERR;
		}
		// Check Response Code
		if( m_sSTD3_ReversalResp.m_strResponseCode_3 != "000" )
		{
			CString strErrorCode, strErrorMsg;
			int		nResponseCodeIndex = -1;
			if( (nResponseCodeIndex = BIZ_STD3_CheckResponseCode(m_sSTD3_ReversalResp.m_strResponseCode_3)) != -1 )
			{
				strErrorCode.Format(L"DA0%-3.3S0",(LPCSTR)&TRITON_RSP_TBL[nResponseCodeIndex]);
				CString	strTemp;
				strTemp.Format(_T("%s%02d"), T_APMSG_003200, nResponseCodeIndex);
				strErrorMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(strTemp);
				m_pDevCmn->fnAPL_StackError(strErrorCode, strErrorMsg, DEV_NET);
			}
			else
			{
				strErrorCode.Format(L"DA0%-3.3s0",m_sSTD3_ReversalResp.m_strResponseCode_3);
				m_pDevCmn->fnAPL_StackError(strErrorCode, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003078), DEV_NET);
			}			
			return RES_HOST_RECV_ERR;
		}
		m_pDevCmn->TranResult = TRUE;
		MemSetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSRESULT, TRUE);
		return RES_OK;
	}
	return RES_HOST_RECV_ERR;
}
// End of [#2292]


/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_AnalMiscellaneousField()
RETURN TYPE  : 
PARAMETER    : int nCurrentIndex
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD3_AnalMiscellaneousField(int nCurrentIndex)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_AnalMiscellaneousField]\n"));

	int nIndex = 0;
	int nMisellIndex = 0, nKey2Index = 0, i = 0;
	CString strMicellaneous;
	CString strTmp, strTmp2;
	BYTE chMicellaneous;
	UINT nMiscellaneous_FID2 = 0, nTmp = 0;
	bool bReceivedErrorWorkingKey = false;		// [#408] NH AIREAT 2008.08.26 Working Key JNL
	bool bReceivedWorkingKey = false;			// [#4] NH PSC 2008.03.10 working key received flag
	bool bReceivedFirstWorkingKey = false;		// [#4] NH PSC 2008.03.10 working key#1 received flag for DES capability
	bool bReceivedSecondWorkingKey = false;		// [#4] NH PSC 2008.03.10 working key#1 received flag for DES capability

	UINT	nFID2 = 0;
	int		nEMVResult  = EMV_TR_SUCCESS;	// [#492] [NH] KSK 2009.2.5

	switch (TranCode)
	{
		case TC_INQUIRY:
		case TC_TRANSFER:
		case TC_WITHDRAWAL:				// Transaction Message
		case TC_POPMONEY:				// [#2350] US Justin 2015.06.19 Add POP Money
		case TC_JUSTCASH:				// [#2449] US Justin 2016.11.21 Add Just.Cash
		case TC_TDL_TIRSURCHARGE:		// [#2292] US Justin 2014.09.30 Add TDL
		case TC_TDL_DCC_LOOKUP:			// [#2292] US Justin 2014.09.30 Add TDL
		case TC_TDL_DCC_TRANSACTION:	// [#2292] US Justin 2014.09.30 Add TDL
		case TC_TDL_DCC_REVERSAL:		// [#2292] US Justin 2014.09.30 Add TDL
		case TC_JUSTCASH_BITCOIN:		// [#2496] US Justin 2017.08.15 Add Bitcoin
		case TC_PIN4:					// [#2515] US Justin 2017.11.20 Add PIN4 Prestaging / TranCode == TC_PIN4
		case TC_DIGITALMINT:
			nMisellIndex = MISCELLANEOUS_REQ_TRAN;
			break;
		case TC_REVERSAL:				// Reversal Message
			nMisellIndex = MISCELLANEOUS_REQ_REVERSAL;
			break;
		case TC_TOTAL:					// HOST Total Message
		case TC_TRIALTOTAL:
			nMisellIndex = MISCELLANEOUS_REQ_TOTAL;
			break;
		case TC_OPEN:					// Configuration Message
		case TC_HEALTHCHK:
			nMisellIndex = MISCELLANEOUS_REQ_CONFIG;
			break;
	}

	for ( nIndex = nCurrentIndex; nIndex < m_strArrRecvData.GetSize(); nIndex++,i = 0 )
	{
		strMicellaneous = m_strArrRecvData[nIndex];

		if( strMicellaneous == "")
			continue;

		chMicellaneous = (BYTE)strMicellaneous.GetAt(0);

		while(chMiscellaneous_Responses[nMisellIndex][i] != 0)
		{
			// search current table
			if( chMicellaneous == (BYTE)chMiscellaneous_Responses[nMisellIndex][i])
				break;

			i++;

			// search next table
			if( chMiscellaneous_Responses[nMisellIndex][i] == 0)
				chMicellaneous = 0x00;
		}

		switch( chMicellaneous ) 
		{
			// Surcharge amount to be displayed to customer during Surcharge Notification screen
			// 8 Numeric ASCII ex) 00032767 ! CAUTION cannot be above 00032767 
		case '!':

#if (MX_VERSION)	// [#2137] MX KSK 2012.07.25
			break;
#endif				// end of [#2137]
			strTmp = strMicellaneous.Mid(1);

			if( strTmp.GetLength() != 8)			break;

			// [#2316] NH Justin 2014.12.16 Not updating Surcharge amount if "Percentage" is used.
			#if ( US_VERSION || CA_VERSION || AU_VERSION)	// KSK 2016.06.21
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE) == PERCENT_MODE)
				break;
			#endif
			// End of [#2316]

			m_sSTD3_ConfigResp.m_strSurchargeAmount_8 = strTmp; // 2008-01-15 V01.02.25
			m_sSTD3_ConfigResp.m_strFieldIDCode3_1 = L"!";

			// [RWC6-129] US William Prevent Modification unless ATM Config
			if (m_HostConfig != HC_ATM)
			{
				break;
			}
			
			MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHAREAMT, strTmp);

#if (AU_VERSION)	// [#2015] MX KSK 2012.02.02
			break;
#endif												// end of [#2015]

			// 금액이 0이거나 모두 space일때,Surcharge ENABLE FLAG 를 0으로 바꾼다.
			if ((Asc2Int(strTmp) == 0) || (strTmp == L"        "))
				MemSetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEENABLE, 0);
			else
				MemSetInt(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SURCHARGEENABLE, 1);

			break;
			// 12 Digit (alphanumeric) Sequence Number
		case '#':
			break;
			// 28 Digit Account Number(i.e. Checking,Savings,etc) Left justified, padded with spaces.
		case '&':
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.02
			strTmp = strMicellaneous.Mid(1);

			// 2010.09.20 소스리뷰 반영
			// 우측 공백제거
			strTmp.TrimRight();

			// 우측 공백제거 후 28자리 이상이면, 수신값 무시.
			if( strTmp.GetLength() > 28 )			break;
			// end of 소스리뷰

			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_RETURNED_ACCNT_NO, strTmp.Right(4));
#endif				// end of [#2115]
			break;
			// MAC enable or disable field. 1ASCII 0:Disable 1:Enable
		case '/':		// [#4] NH PSC 2008.03.10 MAC field enable
			strTmp = strMicellaneous.Mid(1);

			if( strTmp.GetLength() != 1)
				MemSetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_MACFLAG, 0);
			else
				MemSetInt(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_MACFLAG, Asc2Int(strTmp));

			break;
			// PIN Change PIN Block. 16 alphanumeric. ANSI standard encrypted PIN block.
		case '@':
			break;
			// Terminal is Triple-DES capable (if 2: Terminal is Triple DES - '[2', else 1: - '[0')
		case '[':
			break;
			// MAC(Message Authentication Code) Result. 9ASCII characters	^A423 34CD
		case '^':
			// Mac result 는 데이터 수신시에 처음으로 계산된다. 여기서 계산하지 않음.
			break;
		case '{':
			strTmp = strMicellaneous.Mid(1);

			if(strTmp.GetLength() != 16)
			{
				bReceivedErrorWorkingKey = true;		// [#408] AU AIREAT 2008.08.26
				break;
			}

			m_strEncryptedPINKey2_16 = strTmp;
			m_sSTD3_ConfigResp.m_strEncryptedPINKey2_16 = m_strEncryptedPINKey2_16;
			m_sSTD3_ConfigResp.m_strFieldIDCode2_1 = L"{";

			MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2, (LPCTSTR)strTmp);
			// [#4] NH PSC 2008.03.10 fnAPP_LoadWorkingKey() 함수 call 위치수정.
			bReceivedWorkingKey = true;
			bReceivedSecondWorkingKey = true;
			break;
			// DES Communications key(PIN Working Key). 16ASCII characters long, encrypted under Master key.
			// Will be used starting with the next transaction. 
			// PIN Working Key 1
		case '~':
			strTmp = strMicellaneous.Mid(1);

			if(strTmp.GetLength() != 16)
			{
				bReceivedErrorWorkingKey = true;		// [#408] AU AIREAT 2008.08.26
				break;
			}

			m_sSTD3_ConfigResp.m_strEncryptedPINKey_16 = strTmp;
			m_sSTD3_ConfigResp.m_strFieldIDCode1_1 = L"~";

			// Working Key1. This Key is encrypted by the PIN Master Key 
			// before being transmitted to the terminal
			// [#4] NH PSC 2008.03.10 Macing
			// [#554] KSK 2009.08.10 m_KeyMode삭제
			switch (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE))
			{
				// keymode: 0, 1, 2, 5, 6
			case KEYMODE_NON_UNIQ_SDES:
			case KEYMODE_NON_UNIQ_DDES:
			case KEYMODE_UNIQ_SDES:
			case KEYMODE_NON_UNIQ_SDES_MACING:
			case KEYMODE_UNIQ_SDES_MACING:
				MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1, (LPCTSTR)strTmp);
				break;

				// keymode : 3, 4, 7, 11
			case KEYMODE_NON_UNIQ_TDES:
			case KEYMODE_UNIQ_TDES:
			case KEYMODE_TDES_MACING:
			case KEYMODE_TDES_TMACING:
				// Miscellaneous Field ID '{', the absence of this FID in a Response Message.
				// will set Key2 equal to Key1 Key2 = Key1
				// 혹시 DES 2 가 안내려 올경우에 여기서 한번더 찾아 세팅함. HWANG-DO 2006.05.02
				MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1, (LPCTSTR)strTmp);

				int	search_idx = nIndex+1;
				if (search_idx < m_strArrRecvData.GetSize())
					strMicellaneous = m_strArrRecvData[search_idx];

				nKey2Index = strMicellaneous.Find('{');
				// 다음 WorkingKey가 없을 경우에만 실행한다. 
				if (nKey2Index == -1)
				{
					// WorkingKey1을 Working2로 Copy한다
					m_strEncryptedPINKey2_16 = strTmp;
					MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2, m_strEncryptedPINKey2_16);
					m_sSTD3_ConfigResp.m_strEncryptedPINKey2_16 = m_strEncryptedPINKey2_16;
					m_sSTD3_ConfigResp.m_strFieldIDCode2_1 = L"~";
				}

				m_strEncryptedPINKey3_16 = strTmp;
				MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY3, m_strEncryptedPINKey3_16);
				break;
			}
			bReceivedWorkingKey = true;			// [#4] NH PSC 2008.03.10 fnAPP_LoadWorkingKey() call 위치 변경.
			bReceivedFirstWorkingKey = true;
			break;

			// MAC Working Key Right Block for Triple-DES capable terminals
		case '}':
			// Working Key1. This Key is encrypted by the PIN Master Key 
			// before being transmitted to the terminal
			// [#4] NH PSC 2008.03.10 Macing receive MAC working.
			strTmp = strMicellaneous.Mid(1);

			if(strTmp.GetLength() != 16)
			{
				bReceivedErrorWorkingKey = true;		// [#408] AU AIREAT 2008.08.26
				break;
			}

			// [#554] KSK 2009.08.10 m_KeyMode삭제
			if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TDES_TMACING)
			{
				// Miscellaneous Field ID 'm', the absence of this FID in a Response Message.
				MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_APP_DESKEY5, strTmp.Left(16));
				m_sSTD3_ConfigResp.m_strEncryptedPINKey5_16 = strTmp.Left(16);
				m_sSTD3_ConfigResp.m_strFieldIDCode5_1 = L"}";
			}
			bReceivedWorkingKey = true;
			// end of [#4]
			break;
		case '<':
			
			strTmp = strMicellaneous.Mid(1);

			// Validate
			if (strTmp.IsEmpty())
			{
				// No working received
				bReceivedErrorWorkingKey = true;
				break;
			}

			m_sSTD3_ConfigResp.m_strFieldIDCode6_1 = L"<";
			m_sSTD3_ConfigResp.m_strTR31PINKey6_V = strTmp;

			if (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TR31			||
				MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TR31_MACING	||
				MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE) == KEYMODE_TR31_TMACING)
			{
				MemSetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_TR31_WKB, m_sSTD3_ConfigResp.m_strTR31PINKey6_V);
			}

			bReceivedWorkingKey = true;

			break;
		case 'a':
			break;
		case 'b':
			strTmp.Format(L"%s",strMicellaneous.Mid(1));

			// [#466] [NH] KSK 2008.12.19 'b' field 사양 : "b00000000___________" -> space를 뺀 numeric은 8byte임.
			// KSK 2009.2.19 Bug Fix
			strTmp.TrimLeft();
			strTmp.TrimRight();

			if (strTmp.GetLength() != 8)				break;

			memset(m_szTemp, 0, sizeof(m_szTemp));
			WideToMulti(m_szTemp, strTmp, sizeof(m_szTemp));

			if (m_szTemp[0] != '-')
			{
				if (IsNum(strTmp) == TRUE)	// KSK 2010.01.04
				{
					m_sSTD3_TranResp.m_strAvailableBalance_8 = strTmp;
					MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSAVAILBALANCE, m_sSTD3_TranResp.m_strAvailableBalance_8); // V01.02.25 SRC-2
				}
			}
			else
			{
				// 마이너스가 올 경우 처리
				if (IsNum(&m_szTemp[1], strTmp.GetLength()-1) == TRUE)
				{
					m_sSTD3_TranResp.m_strAvailableBalance_8 = strTmp;
					MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSAVAILBALANCE, m_sSTD3_TranResp.m_strAvailableBalance_8); // V01.02.25 SRC-2
				}
			}
			// end of KSK 2009.2.19
			// end of [#466]
			break;
		case 'c':
			// [RWC6-129] US William Prevent Modification unless ATM Config
			if (m_HostConfig == HC_ATM)
			{
				// [#447] [NH] KSK 2008.11.14 AUTO DAY TOTAL TIME SET
				strTmp.Format(L"%s",strMicellaneous.Mid(1));
				if (strTmp.GetLength() != 4)				break;

				if (IsNum(strTmp) != TRUE)	break;		// KSK 2010.01.04

				int	nHour = Asc2Int(strTmp.Left(2));
				int nMin  = Asc2Int(strTmp.Right(2));

				if (nHour < 0 || nHour >= 24)	break;
				if (nMin < 0 || nMin >= 60)		break;

				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_HOUR, nHour);			
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_MIN, nMin);
				// end of [#447]
			}
			break;
		case 'd':
#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16 // [#325] AU AIREAT 2008.06.27	// [#2069] NH KSK 2011.06.13
			// 'd' + type + line
			if( strMicellaneous.GetLength() < 3)
				break;
			else
			{
				chMicellaneous = (BYTE)strMicellaneous.GetAt(1);
				nMiscellaneous_FID2 = chMiscellaneous_Responses[nMisellIndex][i] & 0xFFFFFF00;	// [#103] NH JSW 2008.04.12
				if( (BIZ_STD3_GetFID(chMicellaneous) &  nMiscellaneous_FID2) != BIZ_STD3_GetFID(chMicellaneous))
					break;

				int i = 0, nIndex = 0, nMsgLen = 0;
				CString	strMsg, strtemp;
				int nLineCount = Asc2Int(CString(strMicellaneous.GetAt(2)));

				// Get Message Data & Len
				strMsg	= strMicellaneous.Mid(3);
				nMsgLen = strMsg.GetLength();

				// 's' store message(max: 4 lines)
				if (chMicellaneous == 's' )
				{
					if (nLineCount > 4)		break;

					for(i=0; i<4; i++)
						MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_STORE_MESSAGE1+i, L"");			// all clear (linecount만큼만 refesh해야하는지 문의 필요
						
					for(i=0; i<nLineCount; i++)
					{
						if ((nMsgLen - nIndex) <= 0)	break;

						strtemp = strMsg.Mid(nIndex, 40);

						if (strtemp.GetLength() > 0)
						{
							MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_STORE_MESSAGE1+i, strtemp);	// setup
							nIndex += strtemp.GetLength();
						}
					}
				}
				// 'm' marketing message(max: 4 lines)
				else if(chMicellaneous == 'm')
				{
					if (nLineCount > 4)		break;

					for(i=0; i<4; i++)
						MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MARKETING_MESSAGE1+i, L"");			// all clear (linecount만큼만 refesh해야하는지 문의 필요
					
					for(i=0; i<nLineCount; i++)
					{
						if ((nMsgLen - nIndex) <= 0)	break;
						
						strtemp = strMsg.Mid(nIndex, 40);
						
						if (strtemp.GetLength() > 0)
						{
							MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MARKETING_MESSAGE1+i, strtemp);	// setup
							nIndex += strtemp.GetLength();
						}
					}
				}
				// 'w' welcome message(max: 3 lines)
				else if(chMicellaneous == 'w')
				{
					if (nLineCount > 3)		break;

					for(i=0; i<3; i++)
						MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WELCOME_MESSAGE1+i, L"");			// all clear (linecount만큼만 refesh해야하는지 문의 필요
					
					for(i=0; i<nLineCount; i++)
					{
						if ((nMsgLen - nIndex) <= 0)	break;
						
						strtemp = strMsg.Mid(nIndex, 30);
						
						if (strtemp.GetLength() > 0)
						{
							MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WELCOME_MESSAGE1+i, strtemp);	// setup
							nIndex += strtemp.GetLength();
						}
					}
				}
				// 'e' exit message(max: 3 lines)
				else if(chMicellaneous == 'e')
				{
					if (nLineCount > 3)		break;
					
					for(i=0; i<3; i++)
						MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EXIT_MESSAGE1+i, L"");			// all clear (linecount만큼만 refesh해야하는지 문의 필요
					
					for(i=0; i<nLineCount; i++)
					{
						if ((nMsgLen - nIndex) <= 0)	break;
						
						strtemp = strMsg.Mid(nIndex, 30);
						
						if (strtemp.GetLength() > 0)
						{
							MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EXIT_MESSAGE1+i, strtemp);	// setup
							nIndex += strtemp.GetLength();
						}
					}
				}
				// end of [#541]
			}
#endif		// end if [#325]

			break;

		case 'e':
			strTmp.Format(L"%s",strMicellaneous.Mid(1));

			// [#466] [NH] KSK 2008.12.19 'e' field 정합성 check 추가

			// KSK 2009.2.19 Bug Fix
			strTmp.TrimLeft();
			strTmp.TrimRight();

			if (strTmp.GetLength() != 12)
			{
				// Length 정보가 안맞을 경우 POS랑 같이 Amount1값을 무시하도록 한다.
				if (m_pDevCmn->TranStatus == TRAN_TRAN)
				{
					m_sSTD3_TranResp.m_strAmount1_Ext12.Empty();
					MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT, L"            ");
				}
				else if ((m_pDevCmn->TranStatus == TRAN_TOTAL) || (m_pDevCmn->TranStatus == TRAN_TRIALTOTAL))
				{
					m_sSTD3_TotalResp.m_strSettlement_Ext12.Empty();
				}

				break;
			}

			memset(m_szTemp, 0, sizeof(m_szTemp));
			WideToMulti(m_szTemp, strTmp, sizeof(m_szTemp));

			if (m_pDevCmn->TranStatus == TRAN_TRAN)
			{
				if (m_szTemp[0] != '-')
				{
					if (IsNum(m_szTemp) == TRUE)	// KSK 2010.01.04
					{
						m_sSTD3_TranResp.m_strAmount1_Ext12 = strTmp;
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT, m_sSTD3_TranResp.m_strAmount1_Ext12); // V01.02.25 SRC-2
					}
					else
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT, L"            ");
				}
				else
				{
					if (IsNum(&m_szTemp[1], strTmp.GetLength()-1) == TRUE)
					{
						m_sSTD3_TranResp.m_strAmount1_Ext12 = strTmp;
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT, m_sSTD3_TranResp.m_strAmount1_Ext12); // V01.02.25 SRC-2
					}
					else
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT, L"            ");
				}
			}
			else
				if ((m_pDevCmn->TranStatus == TRAN_TOTAL) || (m_pDevCmn->TranStatus == TRAN_TRIALTOTAL))
					m_sSTD3_TotalResp.m_strSettlement_Ext12 = strTmp;
			// end of [#466]
			break;
		case 'f':
			break;
		case 'g':
			break;
		case 'h':
			break;
		case 'i':
			break;
		case 'j':
			break;
		case 'k':
			// [#66] NH PSC 2008.04.07 'k' field 처리
			if( strMicellaneous.GetLength() < 10 )		break;

			// [RWC6-129] US William Prevent Modification unless ATM Config
			if (m_HostConfig == HC_ATM)
			{
				strTmp.Format(L"%s", strMicellaneous.Mid(1));
				if(Asc2Int(strTmp) == 0 || strTmp.GetLength() != 9)
					MemSetStr(_MEM_FLD_OPTIONALSETTING,_MEM_VAR_OPT_APP_ISSUERFEE, L"000000000");
				else
					MemSetStr(_MEM_FLD_OPTIONALSETTING,_MEM_VAR_OPT_APP_ISSUERFEE, strTmp.Left(9));
			}
			break;
			// end of [#66]
		case 'l':
			break;
			// MAC Working Key Left Block for Triple-DES capable terminals
		case 'm':
			strTmp = strMicellaneous.Mid(1);

			if( strTmp.GetLength() != 16)
			{
				bReceivedErrorWorkingKey = true;		// [#408] AU AIREAT 2008.08.26
				break;
			}

			// Macing Key1.(keymode: 5, 6, 7, 11)
			// [#554] KSK 2009.08.10 m_KeyMode삭제
			switch (MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE))
			{
			case KEYMODE_NON_UNIQ_SDES_MACING:
			case KEYMODE_UNIQ_SDES_MACING:
			case KEYMODE_TDES_MACING:
			case KEYMODE_TDES_TMACING:
				// Miscellaneous Field ID 'm', the absence of this FID in a Response Message.
				MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY4, strTmp.Left(16));
				m_sSTD3_ConfigResp.m_strEncryptedPINKey4_16 = strTmp.Left(16);
				m_sSTD3_ConfigResp.m_strFieldIDCode4_1 = L"m";
				break;
			}
			bReceivedWorkingKey = true;
			break;
		case 'n':
			break;
		case 'o':
			break;
		case 'p':
			// Printing Data Set
			{
				// KSK_2007-07-24오전 11:00:19
				int nCnt = 0;
				CString strCnt = "";
				if (strMicellaneous.GetLength() < 2)	break;

				// 1. Get Line Count
				strCnt.Format(L"%s",strMicellaneous.Mid(1,1));
				nCnt = Asc2Int(strCnt);

				//			if (nCnt < 1)							break;
				if (nCnt < 1 || nCnt > 4)				break;	// [#583] NH KSK 2009.11.24	(사양서상 range 1 ~ 4)

				// 2. Get Message Data
				strTmp.Format(L"%s",strMicellaneous.Mid(2));

				CString			strTmp2;
				CStringArray	strTmpArray;

				// 3. Separator
				strTmp2.Format(L"%x", 0x0d);
				SplitString(strTmp, (char)0x0d, strTmpArray);

				if (!strTmpArray.GetSize())				break;

				// Line Count 값이 실제 데이터 값보다 클 경우에 최대 값은 실제 값으로 계산함.
				if (nCnt > strTmpArray.GetSize())
					nCnt = strTmpArray.GetSize();

				for (int i = 0; i < nCnt; i++)
				{
					m_strTritonPrtMsgArray.Add(strTmpArray[i]);
				}
				break;
			}
		case 'q':
#if (US_VERSION || CA_VERSION || MX_VERSION)	// [#2292] US Justin 2014.09.30 Add TDL		// [#2365] CA Justin 2015.07.27 Implement DCC+ on Canadian AP
			// Make the second FID
			if(strMicellaneous.GetLength() < 2)				break;

			chMicellaneous = (BYTE)strMicellaneous.GetAt(1);
			nMiscellaneous_FID2 = chMiscellaneous_Responses[nMisellIndex][i] & 0xFFFFFF00;
			if( (BIZ_STD3_GetFID(chMicellaneous) &  nMiscellaneous_FID2) != BIZ_STD3_GetFID(chMicellaneous))
				break;

			if( (chMicellaneous=='a')||(chMicellaneous=='c') )
			{
				if(m_STD3_TDL_Data.ParseTDLTag(strMicellaneous))
				{
					if( m_STD3_TDL_Data.m_strWorkingKey.GetLength() == 32 )
					{
						MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY1, m_STD3_TDL_Data.m_strWorkingKey.Left(16) );
						MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_DESKEY2, m_STD3_TDL_Data.m_strWorkingKey.Right(16));
						bReceivedFirstWorkingKey = true;
						bReceivedSecondWorkingKey = true;
						bReceivedWorkingKey = true;	
					}
					/*
					if( (TranCode == TC_TDL_DCC_TRANSACTION)&&(m_pDevCmn->fnMCU_IsEmvTransaction()) )
					{
						strTmp.Format(L"%s", m_STD3_TDL_Data.m_DCC_Transaction.m_strRes_EMVData);
						if( strTmp.GetLength() <= 0)
						{
							if (m_bOnlineProcessing == FALSE)
							{
								m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

								if (m_pDevCmn->fnEMV_Trans_OnlineProcess(1, AAC) == EMV_RSLT_OK)
								{
									m_pDevCmn->m_bDisplayDeclined = TRUE;
									m_pDevCmn->fnEMV_Trans_Completion();
								}
								BIZ_EMV_MakeICDataforReversal();		// EMV
								m_bOnlineProcessing = TRUE;
							}
							nEMVResult = EMV_ERROR_ONLINE_DATA;
							break;
						}
						nEMVResult = BIZ_EMV_Anal_ICDataforTrans(strTmp);
					}
					*/
				}
			}
#endif
#if (MX_VERSION)	// [#2115] MX KSK 2012.02.02 qbA ~ qbD 필드 추가 , Maguire: NULL or 0 일경우 모두 인자 안함
					// [#2137] MX PCS 2012.06.26 qbE, qbF, qbG 필드 추가 

			else if (chMicellaneous=='b')
			{
				chMicellaneous = (BYTE)strMicellaneous.GetAt(2);

	//			if(chMicellaneous < 0x41 || chMicellaneous > 0x47) // A ~ F 검증 // [#2137] MX PCS 2012.06.26 "qbE,F,G" 필드 추가.
	//				break;

				// [#2147] MX KSK 2012.08.18 검증하지 않도록 수정
	//			if (chMicellaneous == 'A' || chMicellaneous == 'B' || chMicellaneous == 'C')
	//			{
	//				if(0x20 != (BYTE)strMicellaneous.GetAt(3)) // [SP] 검증
	//					break;
	//			}
				// end of [#2147]

				strTmp.Format(L"%s",strMicellaneous.Mid(3));
				strTmp.TrimLeft();
				strTmp.TrimRight();
		
				//////////////////////////////////////////////////////////////////////////////////////////////////////////
				// [#2137] 판단 로직 " 음수/NULL 조건"
				if(chMicellaneous == 'A' || chMicellaneous == 'B' || chMicellaneous == 'C')
				{
					// [#2147] MX KSK 2012.08.18 값이 0인 경우 설정하지 않도록 수정
					if (Asc2Int(strTmp) <= 0)
						break;
					// end of [#2147]
				}
				
				// [#2399] MX Justin 2016.02.26 Assign qbE and qbF even they are zeros. (no surcharge)=> Prosa Request
				/*
				if(chMicellaneous == 'E' || chMicellaneous == 'F' || chMicellaneous == 'G')
				{
					// [#2147] MX KSK 2012.08.18 값이 0인 경우 설정하지 않도록 수정
					if (Asc2Int(strTmp) <= 0)
						break;
					// end of [#2147]
				}			
				*/
				if(chMicellaneous == 'G')
				{
					// [#2147] MX KSK 2012.08.18 값이 0인 경우 설정하지 않도록 수정
					if (Asc2Int(strTmp) <= 0)
						break;
					// end of [#2147]
				}
				// [#2399]

				//////////////////////////////////////////////////////////////////////////////////////////////////////////
				// 'A'
				if (chMicellaneous == 'A') // qbA (Loyalty Fee) , 10 Length
				{
					if(strTmp.GetLength() > 0 && strTmp.GetLength() <= 9)
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LOYALTY_FEE, strTmp);
				}
				else if(chMicellaneous == 'B') // qbB (Line Fee) , 10 Length
				{
					if(strTmp.GetLength() > 0 && strTmp.GetLength() <= 9)
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE, strTmp);
				}
				else if(chMicellaneous == 'C') // qbC (IVA Tax) , 10 Length
				{
					if(strTmp.GetLength() > 0 && strTmp.GetLength() <= 9)
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_IVATAX, strTmp);
				}
				else if(chMicellaneous == 'D') // qbD (Bank Name) , 24 Length
				{
					if(strTmp.GetLength() > 0 && strTmp.GetLength() <= 20)
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_BANK_NAME, strTmp);
				}
				//////////////////////////////////////////////////////////////////////////////////////////////////////////
				//[#2137] MX PCS 2012.06.26
				else if(chMicellaneous == 'E') // qbE (Exchange Fee) , 10 Length
				{
	// 				if ( (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE) && (TranCode == TC_WITHDRAWAL) ) // [#2139] MX KMK 2012.08.07 출금시에만 저장함
					if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)	// [#2142] MX KMK 2012.08.16 5.0 소스 반영(무조건 저장 후 표시 여부만 확인)
					{
						if(strTmp.GetLength() > 0 && strTmp.GetLength() <= 9)
							MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_FEE, strTmp);
					}
				}			
				else if (chMicellaneous == 'F') //qbF(Total Fee)
				{
	// 				if ( (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE) && (TranCode == TC_WITHDRAWAL) ) // [#2139] MX KMK 2012.08.07 출금시에만 저장함
					if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)	// [#2142] MX KMK 2012.08.16 5.0 소스 반영(무조건 저장 후 표시 여부만 확인)

					{
						if (strTmp.GetLength() > 0 && strTmp.GetLength() <= 9)
							MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TOTAL_FEE, strTmp);		// [#2142] MX KMK 2012.08.16
					}				
				}
				else if (chMicellaneous == 'G') //qbG(Exchange Rate)
				{
					if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
					{
						if (strTmp.GetLength() > 0 && strTmp.GetLength() <= 19)
							MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE, strTmp);
					}				
				}
				// end of [#2137]
				/////////////////////////////////////////////////////////////////////////////////////////////////////////
				else
					break;
			}
		// end of [#2115]
#endif
			// End of [#2292]
			break;
		case 'r':
			break;
		case 's':
			break;
		case 't':
			strTmp.Format(L"%s",strMicellaneous.Mid(1));

			if (strTmp.GetLength() != 6)				break;

#ifndef APP_AGING_MODE
			PutTime(strTmp.Left(6));
	#if SUPPORT_CHANGE_PARAMETER_JNL
			m_pDevCmn->m_JNLMgr.Save(HOST_ACTION, L"ATM Date & Time Changed"); //[#610] SOOK 2010.01.16 Configuration Change 저널 추가 
	#endif
#endif
			break;
			// [#103] NH PSC 2008.04.23 Misc. 필드2 식별 버그 수정
		case 'u': // Two character FIDs
			// Make the second FID
			if(strMicellaneous.GetLength() < 2)				break;

			chMicellaneous = (BYTE)strMicellaneous.GetAt(1);
			nMiscellaneous_FID2 = chMiscellaneous_Responses[nMisellIndex][i] & 0xFFFFFF00;	// [#103] NH JSW 2008.04.12
			if( (BIZ_STD3_GetFID(chMicellaneous) &  nMiscellaneous_FID2) != BIZ_STD3_GetFID(chMicellaneous))
				break;
			//nMiscellaneous_FID2 &= 0xFFFFFF00;

			// 'a' 6 Numeric ASCII MMDDYY
			if (chMicellaneous == 'a' )
			{
				if( strMicellaneous.GetLength() == 8)
				{
#ifndef APP_AGING_MODE
					strTmp.Format(L"%s",strMicellaneous.Mid(2));
					SYSTEMTIME	ti, orgTi;

					GetLocalTime(&ti);
					GetLocalTime(&orgTi);
					ti.wMonth   = Asc2Int(strTmp.Left(2));
					ti.wDay		= Asc2Int(strTmp.Mid(2, 2));
					ti.wYear	= 2000 + Asc2Int(strTmp.Right(2));
					SetLocalTime(&ti);

					// [#416] NZ AIREAT 2008.09.16
					if (orgTi.wYear != ti.wYear || orgTi.wMonth != ti.wMonth || orgTi.wDay != ti.wDay)
					{
						MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_YEAR, 2000);
	#if (AU_VERSION || US_VERSION || CA_VERSION)	// [#2037] AU KSK 2011.03.31	// [#2457] US Justin 2016.12.09 Support Moniview Journal Uploading	// [#2500] CA Justin 2017.08.22 Support Moniview Journal Uploading
						MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_YEAR, 2000);	// [#613] AU_C KSK 2010.01.18 System Date 변경 시 무조건 Send하도록 함
	#endif

	#if SUPPORT_CHANGE_PARAMETER_JNL
						m_pDevCmn->m_JNLMgr.Save(HOST_ACTION, L"ATM Date & Time Changed"); //[#610] SOOK 2010.01.16 Configuration Change 저널 추가 
	#endif
					}
#endif
				}
			}
			// 'b' 16bit CRC 4 ASCII
//			else if ( chMicellaneous == 'b' )
//				;
			// 'd' Encoded EMV data block ASCII characers. Variable length.
			else if ( chMicellaneous == 'd' )
			{
				// [#2180] MX Justin 2013.03.08 EMV Transaction
				/*
				if(m_pDevCmn->fnMCU_IsEmvTransaction())
				{
					strTmp.Format(L"%s",strMicellaneous.Mid(2));

					if( strTmp.GetLength() <= 0)
					{
						if (m_bOnlineProcessing == FALSE)
						{
							// "ud" Field만 온 경우 Error를 내도록 한다. KSK 2009.4.17
							// KSK 2010.08.29 Default 처리 추가
							m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

							if (m_pDevCmn->fnEMV_Trans_OnlineProcess(1, AAC) == EMV_RSLT_OK)
								m_pDevCmn->fnEMV_Trans_Completion();

							BIZ_EMV_MakeICDataforReversal();

							m_bOnlineProcessing = TRUE;
						}
						nEMVResult = EMV_ERROR_ONLINE_DATA;
						break;
					}

					nEMVResult = BIZ_EMV_Anal_ICDataforTrans(strTmp);	// [#492] [NH] KSK 2009.2.5
				}
				*/
				BOOL bCheckEMV = TRUE;

			#if (MX_VERSION)
				if( (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)&&(MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSACTIONCNT_FLAG) == MX_1ST_SEND) )
				{
					// [#2291] MX Justin 2014.09.02 
					if(m_pDevCmn->fnMCU_IsEmvTransaction())
						m_sSTD3_TranResp.m_strEMVTag = strMicellaneous.Mid(2);
					// End of [#2291]
					bCheckEMV = FALSE;				
				}
			#endif
				if(bCheckEMV)
				{
					if(m_pDevCmn->fnMCU_IsEmvTransaction())
					{
						strTmp.Format(L"%s",strMicellaneous.Mid(2));

						if( strTmp.GetLength() <= 0)
						{
							if (m_bOnlineProcessing == FALSE)
							{
								// "ud" Field만 온 경우 Error를 내도록 한다. KSK 2009.4.17
								// KSK 2010.08.29 Default 처리 추가
								m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

								if (m_pDevCmn->fnEMV_Trans_OnlineProcess(1, AAC) == EMV_RSLT_OK)
								{
									m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
									m_pDevCmn->fnEMV_Trans_Completion();
								}

								BIZ_EMV_MakeICDataforReversal();

								m_bOnlineProcessing = TRUE;
							}
							nEMVResult = EMV_ERROR_ONLINE_DATA;
							break;
						}
						nEMVResult = BIZ_EMV_Anal_ICDataforTrans(strTmp);	// [#492] [NH] KSK 2009.2.5
					}
				}
				// End of [#2180]
			}
			// 'e' DATA Working Key Left Block 16 ASCII characters. 
			// Each character represents a 4-bit hexadecimal numeral (0-9, A-F)
//			else if ( chMicellaneous == 'e' )
//				;
			// 'f' DATA Working Key Right Block
//			else if ( chMicellaneous == 'f' )
//				;
			// 'g' Host Transaction Trace Number 12 ASCII characters. 
//			else if ( chMicellaneous == 'g' )
//				;
			// 'h' Encoded EMV untagged-data block. ASCII characters. Variable length
//			else if ( chMicellaneous == 'h' )
//				;
			// 'i' Motorized card reader commands. 2 Numeric characters.
//			else if ( chMicellaneous == 'i' )
//				;
#if (AU_VERSION) // [#631] MX, 2010.03.29 JERRY: 'u!' field 추가
			else if ( chMicellaneous == '!' )
			{
				strTmp.Format(L"%s",strMicellaneous.Mid(2));

				if (strTmp.GetLength() != 8)
					break;

				if (IsNum(strTmp) == FALSE)		// KSK 2010.04.09 Numeric이 아닌 경우 모두 Invalid 처리
					break;

				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_INQUIRY_SURCHARGE, Asc2Int(strTmp));
			}
#endif

			break;
			// [#103] NH PSC 2008.04.23 Misc. 필드2 식별 버그 수정
		case 'v':
			// Make the second FID
			if(strMicellaneous.GetLength() < 3)				break;

			chMicellaneous = (BYTE)strMicellaneous.GetAt(1);
			nMiscellaneous_FID2 = chMiscellaneous_Responses[nMisellIndex][i] & 0xFFFFFF00;	// [#103] NH JSW 2008.04.12
			if( (BIZ_STD3_GetFID(chMicellaneous) &  nMiscellaneous_FID2) != BIZ_STD3_GetFID(chMicellaneous))
				break;
			//nMiscellaneous_FID2 &= 0xFFFFFF00;

			// 'a' Value Added Service Terminal ID 15ASCII characters. 
//			if ( chMicellaneous == 'a' )
//				;
			// 'b' Postal code of Terminal for Value Added Service 12ASCII
//			else if ( chMicellaneous == 'b' )
//				;
			// 'c' Loyalty card information for Value Added Service3. 80 ASCII
//			else if ( chMicellaneous == 'c' )
//				;
			// 'd' VAS Challenge/Response. 20 ASCII
//			else if ( chMicellaneous == 'd' )
//				;
			break;
		case 'w':
			{
				// [RWC6-129] US William Prevent Modification unless ATM Config
				if (m_HostConfig != HC_ATM) break;

				// [#108] NH AIREAT 2008.04.15	(최대 7자리)

				int	HostDispenseLimit = Asc2Int(strMicellaneous.Mid(1));

				// 사양서는 $1 - $999    (dollar)
				if (HostDispenseLimit < 1 || HostDispenseLimit > 999)
					break;

				CString strDispenseLimit;
				strDispenseLimit.Format(L"%05d00", HostDispenseLimit);
				MemSetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_DISPENSELIMIT, strDispenseLimit);

				// end of [#108]
			}
			break;
			// [#103] NH PSC 2008.04.23 Misc. 필드2 식별 버그 수정
		case 'x':
			// Make the second FID
			if(strMicellaneous.GetLength() < 3)				break;

			chMicellaneous = (BYTE)strMicellaneous.GetAt(1);
			nMiscellaneous_FID2 = chMiscellaneous_Responses[nMisellIndex][i] & 0xFFFFFF00;	// [#103] NH JSW 2008.04.12
			if( (BIZ_STD3_GetFID(chMicellaneous) &  nMiscellaneous_FID2) != BIZ_STD3_GetFID(chMicellaneous))
				break;

			// 'a'
//			if ( chMicellaneous == 'a' )
//				;
			// 'b'
//			else if ( chMicellaneous == 'b' )
//				;
			// 'c'
//			else if ( chMicellaneous == 'c' )
//				;
			// 'd'
//			else if ( chMicellaneous == 'd' )
//				;
			// 'e'
//			else if ( chMicellaneous == 'e' )
//				;
			// 'f'
//			else if ( chMicellaneous == 'f' )
//				;
			// 'g'
//			else if ( chMicellaneous == 'g' )
//				;
			// 'h'
//			else if ( chMicellaneous == 'h' )
//				;
			// 'i'
//			else if ( chMicellaneous == 'i' )
//				;
			// 'n'
//			else if ( chMicellaneous == 'n' )
//				;
			// 'p'
//			else if ( chMicellaneous == 'p' )
//				;
			// 'q'
//			else if ( chMicellaneous == 'q' )
//				;
			// 'r'
//			else if ( chMicellaneous == 'r' )
//				;
			// 's'
//			else if ( chMicellaneous == 's' )
//				;
			// 't'
//			else if ( chMicellaneous == 't' )
//				;
			// 'u'
//			else if ( chMicellaneous == 'u' )
//				;
			// 'v'
//			else if ( chMicellaneous == 'v' )
//				;
			break;
		case 'y':
			break;
		case 'z':
			break;
		}
	}
#if SUPPORT_CHANGE_PARAMETER_JNL		//[#610] SOOK 2010.01.14 Configuration 저널 저장 
		m_pDevCmn->SaveChangeParameter(CHANGE_USER_HOST);		//memory leak 때문에 변경 사항만 따로 저장하는 것으로 수정함. 
#endif		//end of [#610]

	// [#408] AU AIREAT 2008.08.26
#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
	if (bReceivedErrorWorkingKey == true)
	{
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, L"Download Working Key - Fail(wrong msg)");
	}
	else
#endif
	// end of [#408]
	// [#4] NH PSC 2008.03.10 fnAPP_LoadWorkingKey() call 위치 변경.
	if(bReceivedWorkingKey == true)
	{
		// [#408] NH AIREAT 2008.08.26	
		int nWorkingRes;
		CString strOperatorAction;
		
		// Determine which key to import
		// [#RWC6-59] US William 2019.10.11 LibertyX
		int nMKeyName = MASTERKEY_ATM;
		switch (m_HostConfig)
		{
		case HC_DUALHOST: nMKeyName = MASTERKEY_DUALHOST; break;
		case HC_LIBERTYX: nMKeyName = MASTERKEY_LIBERTYX; break;
		}

		// Load working key
		nWorkingRes = LIB_LoadWorkingKey(nMKeyName);

		if (nWorkingRes == T_OK)
		{
			strOperatorAction.Format(L"Download Working Key - Success");

			if (m_pDevCmn->nPINErrorFlag == WORKINGKEY_INIT)
				m_pDevCmn->nPINErrorFlag = WORKINGKEY_DOWNLOAD_SUCCESS;		// [#2010] SOOK 2011.01.14 WORKING KEY LOAD 실패 시 버그 FIX	[#2134] NH KSK 2012.05.07
		}
		else
		{
			strOperatorAction.Format(L"Download Working Key - Fail(%d)", nWorkingRes);

			if (m_pDevCmn->nPINErrorFlag == WORKINGKEY_INIT)
				m_pDevCmn->nPINErrorFlag = WORKINGKEY_DOWNLOAD_FAIL;		// [#2134] NH KSK 2012.05.07
		}
		m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, strOperatorAction);

		// end of [#408]

		m_bFirstKeyReceived = bReceivedFirstWorkingKey;
		m_bSecondKeyReceived = bReceivedSecondWorkingKey;
	}

	return nEMVResult;
}


/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_STD3_CheckResponseCode()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_STD3_CheckResponseCode(CString strResponseCode)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_CheckResponseCode]\n"));

	int nCodeIndex = 0;

	for( nCodeIndex =0; nCodeIndex < sizeof(TRITON_RSP_TBL) / sizeof(_RSP_TBL); nCodeIndex++)
	{
		if( (LPCSTR)TRITON_RSP_TBL[nCodeIndex].CODE == strResponseCode )
			return nCodeIndex;
	}
	return -1;
}

// [#103] NH 2008.04.23 PSC
UINT CTranCmn::BIZ_STD3_GetFID(char fid2)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_STD3_GetFID]\n"));

	switch(fid2)
	{
	case 'a':		return FID2_a;
	case 'b':		return FID2_b;
	case 'c':		return FID2_c;
	case 'd':		return FID2_d;
	case 'e':		return FID2_e;
	case 'f':		return FID2_f;
	case 'g':		return FID2_g;
	case 'h':		return FID2_h;
	case 'i':		return FID2_i;
	case 'j':		return FID2_j;
	case 'k':		return FID2_k;
	case 'l':		return FID2_l;
	case 'm':		return FID2_m;
	case 'n':		return FID2_n;
		//	case 'o':		return FID2_o;		//	[#438] AU AIREAT 2008.10.17	- 사용안함
	case '!':		return FID2_21h;	//	[#438] AU AIREAT 2008.10.17	- 'o' -> '!'(21h)로 변경.
	case 'p':		return FID2_p;
	case 'q':		return FID2_q;
	case 'r':		return FID2_r;
	case 's':		return FID2_s;
	case 't':		return FID2_t;
	case 'u':		return FID2_u;
	case 'v':		return FID2_v;
	case 'w':		return FID2_w;
	case 'x':		return FID2_x;
	default:		return 0xffffffff;
	}
}
// end of [#103]