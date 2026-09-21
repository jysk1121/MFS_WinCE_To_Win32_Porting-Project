#ifndef __SCREEN_CONTROL_TYPE_H__
#define __SCREEN_CONTROL_TYPE_H__

typedef enum SCREEN_TYPE
{
	SCR_TYPE_NONE = 0,
	SCR_TYPE_FLASH_LITE,			// Adobe Flash Lite 30
	SCR_TYPE_PICASSO,				// Nautilus Hyosung Screen Drawing Engine
	SCR_TYPE_MAX
};

///////////////////////////////////////
// Variable String Define for CScrCtrl

#define PREV_SET_SCREEN_NO		_T("PrevSetScreenNo")
#define AP_CENTER_MOVIE			_T("APCenterMovie")

#define AP_VALUE				_T("APValue")
#define AP_STATE				_T("APState")
#define AP_BYPASS				_T("BYPASS")


/*
#define AP_TEXT_ID				_T("APTextID:")
#define KIND_LANGUAGE			_T("kindLanguage")
#define AP_MAX_TIME				_T("APMaxTime")
#define AP_TIMEOUT_MODE			_T("APTimeoutMode")
#define AP_TIMEOUT_FIRST		_T("TimeoutFirst")
#define AP_TIMEOUT_LAST			_T("TimeoutLast")
#define AP_TIMEOUT_STOP			_T("TimeoutStop")
#define AP_UPDATE				_T("APUpdate")
#define AP_SETFOCUS				_T("APSetFocus")
#define TIMEOVER				_T("TIMEOVER")
#define HIDE					_T("hide")
#define ON						_T("on")
#define OFF						_T("off")
#define FOCUS					_T("FOCUS")
#define ENABLE					_T("ENABLE")
#define DISABLE					_T("DISABLE")
#define BLINKING				_T("BLINKING")
#define MAX_IN_CHAR				_T("MaxInChar:")
*/

#endif //__SCREEN_CONTROL_TYPE_H__