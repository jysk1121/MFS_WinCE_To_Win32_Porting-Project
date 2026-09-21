#ifndef __NH_SCREEN_DESC_TYPE_H__
#define __NH_SCREEN_DESC_TYPE_H__

// 화면시 기준이 되는 화면 사이즈
// 다른 해상도의 화면에 대한 스케일링의 기준이됨. 
#define BASIC_SCREEN_X		(1280)
#define BASIC_SCREEN_Y		(800)

#define	MAX_LINE_VALUE	(255)

#define MASTER_SCREEN_FILE	_T("\\MasterScreenDesc.dat")

#define SCR_COORD_MAX		(9999999)
#define SCR_COORD_MIN		(-1)

// [#236] NZ AIREAT 2008.05.27
/*------------------------------------------------------------------
	SCREEN TIMEOUT MODE
------------------------------------------------------------------*/
#define SCR_TMODE_STOP		0								// Timeout 모드 정지.
#define SCR_TMODE_FIRST		1								// 화면이 표시되고 정해진 시간 Timeout
#define SCR_TMODE_LAST		2								// 마지막 입력 후 정해진 시간 Timeout
// end of [#236]

//////////////////////////////////////////
//	Locale Define.
//
enum _NH_LOCALE
{
	NH_ENGLISH=0,
	NH_SPANISH,
	NH_MAXLOCALE
};


#define DES_ENGLISH			_T("ENGLISH")
#define DES_SPANISH			_T("SPANISH")

//////////////////////////////////////////
//	Resource String Define
#define DES_RESOURCE		_T("RESOURCE")
#define DES_FILE			_T("FILE")
#define DES_UFILE			_T("UFILE")
#define DES_SCRTEXT			_T("SCRTEXT")
#define DES_FONT			_T("FONT")
#define DES_FIXED_FONT		_T("FIXEDFONT")
#define DES_PEN				_T("PEN")
#define DES_BRUSH			_T("BRUSH")
#define DES_PICTURE			_T("PICTURE")
#define DES_COLOR			_T("COLOR")
#define DES_ROUND			_T("ROUND")	//*LEH

//////////////////////////////////////////
//	Screen String Define
#define DES_SCREEN			_T("SCREEN")
#define DES_ID				_T("ID")
#define DES_BACK			_T("BACK")
#define DES_TEXTBOX			_T("TEXTBOX")
#define DES_EFFECTTEXTBOX	_T("EFFECTTEXTBOX")
#define DES_EDITBOX			_T("EDITBOX")
#define DES_CHANGE_PW_BOX	_T("CHANGEPWBOX")
#define DES_BUTTON			_T("BUTTON")
#define DES_VKEYBUTTON		_T("VKEYBUTTON")
#define DES_TOGGLE_BOX		_T("TOGGLEBOX")
#define DES_EJNL_BOX		_T("EJNLBOX")
#define DES_DATE_BOX		_T("DATEBOX")
#define DES_IP_BOX			_T("IPBOX")
#define DES_BIN_BOX			_T("BINBOX")
#define DES_CENT_BOX		_T("CENTBOX")
#define DES_DOLLAR_BOX		_T("DOLLARBOX")
#define DES_ADV_BOX			_T("ADVBOX")
#define DES_ANI_BOX			_T("ANIBOX")
#define DES_TABLE_EDITBOX	_T("TABLEEDITBOX")
#define DES_ALPHA_KEYBOX	_T("ALPHAKEYBOX")
#define DES_TABLE_KEYBOX	_T("TABLEKEYBOX")
#define DES_THREE_STATEBOX	_T("THREESTATEBOX")
#define DES_BININDEX_BOX	_T("BININDEXBOX")		//[#543]SOOK 2009.07.19 BIN INDEX BOX 추가 
#define DES_DECIMAL_BOX		_T("DECIMALBOX")		//[#774]Justin 2014.12.16 Add Decimal Box
#define DES_CENTCLEARL_BOX	_T("CENTCLEARBOX")		//[#810]Justin 2015.10.09 
#define DES_KEYTEST_BOX		_T("KEYTESTBOX")
#define DES_INPUTEDIT_BOX	_T("INPUTEDITBOX")
#define DES_THREETYPE_BOX	_T("THREETYPEBOX")
#define DES_GIF_BOX			_T("GIFBOX")
#define DES_IMAGE_BOX		_T("IMAGEBOX")
#define DES_EDITBUTTON_BOX	_T("EDITBUTTONBOX")

//////////////////////////////////////////
//	Control String Define
#define DES_NAME			_T("NAME")
#define DES_AFOCUS			_T("AFOCUS")
#define DES_POS				_T("POS")
#define DES_POS_CUR_BOX		_T("POSCURPWBOX")
#define DES_POS_NEW_BOX		_T("POSNEWPWBOX")
#define DES_POS_NEW_RE_BOX	_T("POSNEWREPWBOX")
#define DES_POS_INDEX		_T("POSINDEX")
#define DES_POS_BIN			_T("POSBIN")
#define DES_TEXT			_T("TEXT")
#define DES_TEXTID			_T("TEXTID")
#define DES_TEXTSET			_T("TEXTSET")
#define DES_LEFT			_T("LEFT")
#define DES_CENTER			_T("CENTER")
#define DES_RIGHT			_T("RIGHT")
#define DES_ACT				_T("ACT")
#define DES_ACT_KEY			_T("ACTKEY")
#define DES_FSCMD			_T("FSCMD")
#define DES_ALLFSCMD		_T("ALLFSCMD")
#define DES_FULLFSCMD		_T("FULLFSCMD")							// [#75] NH KGS 2008.04.02 Full FS Command 추가

#define DES_KEYEVENT		_T("KEYEVENT")
#define DES_SETFOCUS		_T("SETFOCUS")
#define DES_BTNFIND			_T("BTNFIND")
#define DES_BINACT			_T("BINACT")
#define DES_SETVALUE		_T("SETVALUE")
#define DES_MAXINCHAR		_T("MAXINCHAR")
#define DES_MINCHAR			_T("MININCHAR")							// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
#define DES_TYPE			_T("TYPE")
#define DES_PASSWORD		_T("PASSWORD")
#define DES_IPADDRESS		_T("IPADDRESS")							// [#470] NH AIREAT 2008.12.22 : IP ADDRESS Type 추가.
#define DES_AUTO_RUN		_T("AUTORUN")
#define DES_ENABLE_TEXT		_T("ENABLETEXT")
#define DES_DISABLE_TEXT	_T("DISABLETEXT")
#define DES_BLINK			_T("BLINK")
#define DES_FRAME			_T("FRAME")
#define DES_EFFECT			_T("EFFECT")
#define DES_SLIDING			_T("SLIDING")
#define DES_TYPING			_T("TYPING")
#define DES_ALWAYS_SHOW		_T("ALWAYSSHOW")

#define DES_LOCALE			_T("LOCALE")
#define DES_COUNT			_T("COUNT")
#define DES_TEXT			_T("TEXT")
#define DES_SUPPORT			_T("SUPPORT")							// [#469][NH] AIREAT 2008.12.20 AP,OP Text 다국어 통합 작업
#define DES_ID				_T("ID")

// V1.0.2.4 2018.07.11 - Screen Engine 구조 변경으로 인한 추가
#define DES_CONTROL			_T("CONTROL")
#define DES_CONTROL_ID		_T("CONTROLID")
/////////////////////////////////////////////////////

// [SWKEY]
#define DES_SW_KEY			_T("SWKEY")
#define DES_NUMBER_SIZE		_T("NUMBERSIZE")
#define DES_GUIDE_SIZE		_T("GUIDESIZE")
#define DES_SPACE_SIZE		_T("SPACESIZE")

// [TABLEEDITBOX]
#define DES_ALLOW_ROW_COL	_T("ALLOWROWCOLUMN")
#define DES_ELEMENT_PEN		_T("ELEMENTPEN")
#define DES_ELEMENT_BRUSH	_T("ELEMENTBRUSH")
#define DES_ELEMENT_FONT	_T("ELEMENTFONT")
#define DES_GUIDE_FONT		_T("GUIDEFONT")
#define DES_ELEMENT_TEXT_COLOR	_T("ELEMENTTEXTCOLOR")

// [VIRTUALKEYBOX]
#define DES_GUIDE_FONT		_T("GUIDEFONT")
#define DES_GUIDE_BRUSH		_T("GUIDEBRUSH")
#define DES_GUIDE_PEN		_T("GUIDEPEN")
#define DES_GUIDE_TEXT_COLOR	_T("GUIDETEXTCOLOR")
#define DES_VKEY_FONT		_T("VKEYFONT")
#define DES_VKEY_BRUSH		_T("VKEYBRUSH")
#define DES_VKEY_PEN		_T("VKEYPEN")
#define DES_VKEY_TEXT_COLOR	_T("VKEYTEXTCOLOR")
#define DES_VKEY_TYPE		_T("VKEYTYPE")

// [THREESTATEBOX]
#define DES_STATE_IMAGE		_T("STATEIMAGE")
#define DES_STATE_PEN		_T("STATEPEN")
#define DES_STATE_BRUSH		_T("STATEBRUSH")
#define DES_STATE_FONT		_T("STATEFONT")
#define DES_STATE_TEXT_COLOR	_T("STATETEXTCOLOR")
#define DES_STATE_BLINK		_T("STATEBLINK")

//////////////////////////////////////////
//	SetVariable Define..
#define DES_PREV_SET_SCREEN_NO		_T("PrevSetScreenNo")
#define DES_AP_CENTER_MOVIE			_T("APCenterMovie")
#define DES_AP_VALUE				_T("APValue")
#define DES_AP_STATE				_T("APState")
#define DES_AP_TEXT_ID				_T("APTextID:")
#define DES_KIND_LANGUAGE			_T("kindLanguage")
#define DES_AP_MAX_TIME				_T("APMaxTime")
#define DES_AP_TIMEOUT_MODE			_T("APTimeoutMode")				// [#236] NZ AIREAT 2008.05.27
#define DES_AP_TIMEOUT_FIRST		_T("TimeoutFirst")				// [#236] NZ AIREAT 2008.05.27
#define DES_AP_TIMEOUT_LAST			_T("TimeoutLast")				// [#236] NZ AIREAT 2008.05.27
#define DES_AP_TIMEOUT_STOP			_T("TimeoutStop")				// [#236] NZ AIREAT 2008.06.04
#define DES_AP_UPDATE				_T("APUpdate")
#define DES_AP_SETFOCUS				_T("APSetFocus")				// [#434] AU AIREAT 2008.10.14
#define DES_TIMEOVER				_T("TIMEOVER")
#define DES_HIDE					_T("hide")
#define DES_ON						_T("on")
#define DES_OFF						_T("off")
#define DES_FOCUS					_T("FOCUS")
#define DES_ENABLE					_T("ENABLE")
#define DES_DISABLE					_T("DISABLE")
#define DES_BLINKING				_T("BLINKING")
#define DES_AP_SET_BLINKMODE		_T("APSetBlinkMode")
#define DES_MAX_IN_CHAR				_T("MaxInChar:")
#define DES_AP_SET_INPUTMODE		_T("APSetInputMode")
#define DES_AP_KEY_DISABLE			_T("APKeyDisable")
#define DES_AP_SET_INPUTTYPE		_T("APSetInputType")
#define DES_DISABLE_TYPE			_T("disable")
#define DES_ENABLE_TYPE				_T("enable")
#define DES_SELECT_TYPE				_T("select")
#define DES_AP_EDIT_STATE			_T("APEditState")

//////////////////////////////////////////
//	Key Emulration
#define DES_ENTER					_T("ENTER")
#define DES_CLEAR					_T("CLEAR")
#define DES_CANCEL					_T("CANCEL")
#define DES_STAR					_T("STAR")						// [#470] NH AIREAT 2009.01.06 : '.' 입력 선택 적용.
#define DES_SHARP					_T("SHARP")						// [#470] NH AIREAT 2009.01.06 : '.' 입력 선택 적용.
#define DES_CAPS					_T("CAPS")						// [#470] NH AIREAT 2009.01.06 : '.' 입력 선택 적용.
#define DES_UP						_T("UP")
#define DES_DOWN					_T("DOWN")

#define EMUL_FUNCKEY_L1				_T("L1")
#define EMUL_FUNCKEY_L2				_T("L2")
#define EMUL_FUNCKEY_L3				_T("L3")
#define EMUL_FUNCKEY_L4				_T("L4")
#define EMUL_FUNCKEY_R1				_T("R1")
#define EMUL_FUNCKEY_R2				_T("R2")
#define EMUL_FUNCKEY_R3				_T("R3")
#define EMUL_FUNCKEY_R4				_T("R4")

#define EMUL_FUNCKEY_PREFIX			_T("FDK_")

//////////////////////////////////////////
//	Bin List Act
#define DES_BIN_ADD					_T("BINADD")
#define DES_BIN_DEL					_T("BINDEL")
#define DES_BIN_EDT					_T("BINEDT")

#endif __NH_SCREEN_DESC_TYPE_H__
