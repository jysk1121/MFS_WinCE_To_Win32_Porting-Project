#ifndef __PICASSO_CTRL_DESCRIPTION_H__
#define __PICASSO_CTRL_DESCRIPTION_H__

// 화면시 기준이 되는 화면 사이즈
// 다른 해상도의 화면에 대한 스케일링의 기준이됨. 
#define BASIC_SCREEN_X		(640)
#define BASIC_SCREEN_Y		(480)

#define MX5200SE_VIRTUAL_SCREEN_Y	(600)	// [#2300] US KSK 2014.11.04		// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE

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

#define DES_ENGLISH			_T("ENGLISH")
#define DES_SPANISH			_T("SPANISH")
#define DES_FRENCH			_T("FRENCH")

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
#define DES_ROUND			_T("ROUND")
#define DES_COLOR			_T("COLOR")
#define DES_LOAD_CONTROL	_T("LOADCONTROL")
#define DES_ICON			_T("ICON")
#define DES_EXTRA_FONT		_T("EXTRAFONT")		// [#2186] US KMK 2013.04.08 Asian Language Support
#define DES_POSITION		_T("POSITION")		// [#2529] NH Justin 2018.02.22 Add 2800 SE

//////////////////////////////////////////
//	Screen String Define
#define DES_SCREEN			_T("SCREEN")
#define DES_ID				_T("ID")
#define DES_BACK			_T("BACK")
#define DES_TEXTBOX			_T("TEXTBOX")
#define DES_MULTITEXTBOX	_T("MULTITEXTBOX")
#define DES_HIDETEXTBOX		_T("HIDETEXTBOX")
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
#define DES_TRANADV_BOX		_T("TRANADVBOX")
#define DES_ANI_BOX			_T("ANIBOX")
#define DES_TABLE_EDITBOX	_T("TABLEEDITBOX")
#define DES_ALPHA_KEYBOX	_T("ALPHAKEYBOX")
#define DES_NUMBER_KEYBOX	_T("NUMBERKEYBOX")
#define DES_TABLE_KEYBOX	_T("TABLEKEYBOX")
#define DES_THREE_STATEBOX	_T("THREESTATEBOX")
#define DES_BININDEX_BOX	_T("BININDEXBOX")		//[#543]SOOK 2009.07.19 BIN INDEX BOX 추가 
#define DES_WEATHER_BOX		_T("WEATHERBOX")
#define DES_CUSTOMEDIT_BOX	_T("CUSTOMEDITBOX")		// [#2065] NH KJW 2011.05.26	// [#2350] US Justin Change Name
#define DES_DECIMAL_BOX		_T("DECIMALBOX")		// [#2316]Justin 2014.12.17 Add Decimal Box
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
#define DES_OUTLINE			_T("OUTLINE")

#define DES_KEYEVENT		_T("KEYEVENT")
#define DES_SETFOCUS		_T("SETFOCUS")
#define DES_BTNFIND			_T("BTNFIND")
#define DES_BINACT			_T("BINACT")
#define DES_SETVALUE		_T("SETVALUE")
#define DES_MAXINCHAR		_T("MAXINCHAR")
#define DES_MINCHAR			_T("MININCHAR")							// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
#define DES_TYPE			_T("TYPE")
#define DES_PASSWORD		_T("PASSWORD")
#define DES_ADDRESS			_T("ADDRESS")							// "." "," 입력 가능한 TYPE
#define DES_AUTO_RUN		_T("AUTORUN")
#define DES_ENABLE_TEXT		_T("ENABLETEXT")
#define DES_DISABLE_TEXT	_T("DISABLETEXT")
#define DES_BLINK			_T("BLINK")
#define DES_FRAME			_T("FRAME")
#define DES_EFFECT			_T("EFFECT")
#define DES_SLIDING			_T("SLIDING")
#define DES_TYPING			_T("TYPING")
#define DES_DISPFORMAT		_T("DISPFORMAT")						// [#2350] US Justin 2015.06.17 Display Format

#define DES_LOCALE			_T("LOCALE")
#define DES_COUNT			_T("COUNT")
#define DES_TEXT			_T("TEXT")
#define DES_SUPPORT			_T("SUPPORT")							// [#469][NH] AIREAT 2008.12.20 AP,OP Text 다국어 통합 작업
#define DES_ID				_T("ID")
#define DES_OVERRIDE		_T("OVERRIDE")

// [MULTITEXT]
#define DES_MULTITEXT_COLOR		_T("MULTITEXTCOLOR")
#define DES_MULTITEXT_ALIGN		_T("MULTITEXTALIGN")


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

// [VKEYBOARDBOX]
#define DES_VKEYBOARD_BOX			_T("VKEYBOARDBOX")
#define DES_BUTTON_GAP				_T("BUTTONGAP")
#define DES_LINE_HEIGHT				_T("LINEHEIGHT")
#define DES_NORMAL_BTN_FONT			_T("NORMALBTNFONT")
#define DES_NORMAL_BTN_TXT_COLOR	_T("NORMALBTNTEXTCOLOR")
#define DES_NORMAL_BTN_IMAGE		_T("NORMALBTNIMAGE")
#define DES_NORMAL_BTN_SIZE			_T("NORMALBTNSIZE")
//#define DES_CONTROL_BTN_FONT		_T("CONTROLBTNFONT")
//#define DES_CONTROL_BTN_TXT_COLOR	_T("CONTROLBTNTEXTCOLOR")
//#define DES_CONTROL_BTN_IMAGE		_T("CONTROLBTNIMAGE")
#define DES_CONTROL_CLEAR_IMAGE		_T("CONTROLCLEARIMAGE")
#define DES_CONTROL_123_IMAGE		_T("CONTROL123IMAGE")
#define DES_CONTROL_ABC_IMAGE		_T("CONTROLABCIMAGE")
#define DES_CONTROL_CAPS_IMAGE		_T("CONTROLCAPSIMAGE")
#define DES_CONTROL_SYMBOL1_IMAGE	_T("CONTROLSYMBOL1IMAGE")
#define DES_CONTROL_SYMBOL2_IMAGE	_T("CONTROLSYMBOL2IMAGE")
#define DES_CONTROL_BTN_SIZE		_T("CONTROLBTNSIZE")
#define DES_SPACE_BTN_IMAGE			_T("SPACEBTNIMAGE")
#define DES_SPACE_BTN_SIZE			_T("SPACEBTNSIZE")
#define DES_ARROW_LEFT_IMAGE		_T("ARROWLEFTIMAGE")
#define DES_ARROW_RIGHT_IMAGE		_T("ARROWRIGHTIMAGE")
#define DES_ARROW_UP_IMAGE			_T("ARROWUPIMAGE")
#define DES_ARROW_DOWN_IMAGE		_T("ARROWDOWNIMAGE")
#define DES_ARROW_BTN_SIZE			_T("ARROWBTNSIZE")
#define DES_BACK_POS				_T("BACKPOS")
#define DES_TITLE_POS				_T("TITLEPOS")
#define DES_DATE_POS				_T("DATEPOS")
#define DES_WEATHER_POS				_T("WEATHERPOS")
#define DES_TEMPER_POS				_T("TEMPERPOS")
#define DES_NORMAL_SET				_T("NORMALSET")
#define DES_ACCENT_SET				_T("ACCENTSET")
#define DES_BACK_IMG				_T("BACKIMG")
#define DES_WEATHER_IMG				_T("WEATHERIMG")

//////////////////////////////////////////
//	SetVariable Define..
#define DES_PREV_SET_SCREEN_NO		_T("PrevSetScreenNo")
#define DES_AP_CENTER_MOVIE			_T("APCenterMovie")
#define DES_AP_VALUE				_T("APValue")
#define DES_AP_STATE				_T("APState")
#define DES_AP_ADV					_T("APAdv")
#define DES_AP_NOTICE				_T("APNotice")
#define DES_AP_SWITCH_TIME			_T("APSwitchTime")
#define DES_AP_TEXT_ID				_T("APTextID:")
#define DES_KIND_LANGUAGE			_T("kindLanguage")
#define DES_AP_WEATHER				_T("APWeather")
#define DES_AP_TRANADV				_T("APTranAdv")
#define DES_BACK_IMAGE				_T("APBackImage")
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
#define DES_MAX_IN_CHAR				_T("MaxInChar:")
#define DES_ALLOW_ROWCOL			_T("AllowRowCol:")
#define DES_EDIT_BOX_TYPE			_T("EditBoxType:")

//////////////////////////////////////////
//	Key Emulration
#define DES_EPP_KEY					_T("EPP_KEY")
#define DES_ENTER					_T("ENTER")
#define DES_SELECT					_T("SELECT")
#define DES_CLEAR					_T("CLEAR")
#define DES_CANCEL					_T("CANCEL")
#define DES_STAR					_T("STAR")						// [#470] NH AIREAT 2009.01.06 : '.' 입력 선택 적용.
#define DES_SHARP					_T("SHARP")						// [#470] NH AIREAT 2009.01.06 : '.' 입력 선택 적용.
#define DES_CAPS					_T("CAPS")						// [#470] NH AIREAT 2009.01.06 : '.' 입력 선택 적용.
#define DES_UP						_T("UP")
#define DES_DOWN					_T("DOWN")
#define DES_LEFT					_T("LEFT")
#define DES_RIGHT					_T("RIGHT")

//////////////////////////////////////////
//	Bin List Act
#define DES_BIN_ADD					_T("BINADD")
#define DES_BIN_DEL					_T("BINDEL")
#define DES_BIN_EDT					_T("BINEDT")

//////////////////////////////////////////
//	Image List
#define DES_IMAGELIST_BOX			_T("IMAGELISTBOX")
#define DES_INIT_LIST				_T("INITLIST")
#define DES_LIST_SET				_T("LISTSET")

//////////////////////////////////////////
//	Amount Box
#define DES_AMOUNT_BOX				_T("AMOUNTBOX")
#define DES_SYMBOL_NAME				_T("SYMBOLNAME")
#define DES_AMOUNT_NAME				_T("AMOUNTNAME")
#define DES_INPUT_MAX_NAME			_T("INPUTMAXNAME")
#define DES_INPUT_MIN_NAME			_T("INPUTMINNAME")
#define DES_AUTO_RUN_NAME			_T("AUTORUNNAME")

//////////////////////////////////////////
//	Button Image List
#define DES_BUTTON_IMAGELIST_BOX	_T("BUTTONIMAGELISTBOX")

//////////////////////////////////////////
//	Key List
#define DES_KEY_LIST_BOX			_T("KEYLISTBOX")

// [#13] NH KSK 2010.10.11 KEY TYPE 추가
// ADA : ENTER -> CLEAR -> CANCEL
// ISO : CANCEL -> CLEAR -> ENTER
//////////////////////////////////////////
//	Key List
#define	 DES_KEY_TYPE			_T("KEYTYPE_")
#define	 DES_KEY_TYPE_ADA		_T("KEYTYPE_ADA")
#define	 DES_KEY_TYPE_ISO		_T("KEYTYPE_ISO")

// OS Type
#define  DES_OS_TYPE			_T("OSTYPE_")
#define  DES_OS_TYPE_60			_T("OSTYPE_60")
#define  DES_OS_TYPE_70			_T("OSTYPE_70")

#endif __PICASSO_CTRL_DESCRIPTION_H__
