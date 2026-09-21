#ifndef __PICASSO_CONTROL_H__
#define __PICASSO_CONTROL_H__

#include "stdafx.h"
#include ".\Dll\NHCollection.h"
#include ".\PicassoResource.h"


//------------------------------------------------------------------
//	Request Type Define
//------------------------------------------------------------------
typedef enum REQ_TYPE
{
	REQ_NONE=0,
	REQ_INITIAL,
	REQ_DEINITIAL,
	REQ_DRAW,
	REQ_LOCALE,
	REQ_SETVALUE,
	REQ_MOUSEEVENT,
	REQ_KEYEVENT,
	REQ_SETFOCUS,
	REQ_BTNFIND,
	REQ_FSCMD,
	REQ_ALLFSCMD,
	REQ_BINACT,
	REQ_BLINKING,
	REQ_FULLFSCMD,
	REQ_MAX
};


//------------------------------------------------------------------
//	CPicassoRequest Define
//------------------------------------------------------------------
class CPicassoRequest
{
public:
	CPicassoRequest();
	CPicassoRequest(REQ_TYPE type, CString Name, CString Data);
	~CPicassoRequest();

	CPicassoRequest(const CPicassoRequest &rRequest);
public:
	void		Set(REQ_TYPE type, LPCTSTR Name, LPCTSTR Data);
	REQ_TYPE	GetKind();
	CString		GetName();
	CString		GetData();
	void		SetKinD(REQ_TYPE type);
	void		SetName(CString Name);
	void		SetData(CString Data);

private:
	REQ_TYPE	m_ReqType;
	
public:
	CString		m_Name;
	CString		m_Data;
	
	HDC			m_hDC;			// for REQ_DRAW
	RECT		m_rcUpdate;
	int			m_nLocale;		// for REQ_LOCALE
	POINT		m_Point;		// for Mouse point
};


//------------------------------------------------------------------
//	CPicassoRequest Define
//------------------------------------------------------------------
class CPicassoEventHandler
{
public:
	CPicassoEventHandler(CPicassoEventHandler *pNext=0);
	~CPicassoEventHandler();

public:
	void	SetNextHandler(CPicassoEventHandler *pNext);
	
	BOOL	AddRequest(REQ_TYPE type, LPCTSTR Name, LPCTSTR Data);
	BOOL	AddRequest(CPicassoRequest &rRequest);
	BOOL	GetRequest(CPicassoRequest &rRequest);
	BOOL	ClearRequest(void);

	void	AddInvalidRect(RECT rcRect);
	RECT	GetInvalidRect();
	void	ClearInvalidRect();

	void	SetRequestQueue(CNHLinkedList<CPicassoRequest> *pRequestQueue);
	void	SetInvalidRect(RECT *pRect);

public:
	BOOL	Handler(CPicassoRequest &pRequest);
public:
	virtual BOOL	Initialize(void);
	virtual BOOL	Deinitialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetLocaleNum(int Locale);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	MouseEvent(POINT &point);
	virtual BOOL	MakeAllFSCMD(CString &Data);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	FindActBtn(CString &strKey);
	virtual BOOL	BinAct(CString &strAct);
	virtual BOOL	SetBlinking(void);

protected:
	CNHLinkedList<CPicassoRequest>* GetRequestQueue();

private:
	CNHLinkedList<CPicassoRequest>	*m_pRequests;
	CPicassoEventHandler						*m_pNextHandler;
	RECT							*m_pInvalidRect;
};


//------------------------------------------------------------------
//	Control Type Define
//------------------------------------------------------------------
typedef enum CTL_TYPE
{
	CTL_TEXTBOX = 0,
	CTL_MULTITEXTBOX,
	CTL_HIDETEXTBOX,
	CTL_EFFECTTEXTBOX,
	CTL_EDITTEXTBOX,
	CTL_TABLEEDITTEXTBOX,
	CTL_BUTTONBOX,
	CTL_VKEYBUTTONBOX,
	CTL_CHANGEPWBOX,
	CTL_SWKEYBOX,
	CTL_ALPHAKEYBOX,
	CTL_TABLEKEYBOX,
	CTL_NUMBERKEYBOX,
	CTL_TOGGLEBOX,
	CTL_THREESTATEBOX,
	CTL_EJNLBOX,
	CTL_DATEBOX,
	CTL_IPBOX,
	CTL_BINBOX,
	CTL_CENTBOX,
	CTL_DOLLARBOX,
	CTL_ANI,
	CTL_ADVBOX,
	CTL_TRANADVBOX,
	CTL_BININDEXBOX,
	CTL_WEATHERBOX,
	CTL_VKEYBOARDBOX,
	CTL_IMAGELISTBOX,
	CTL_BUTTONIMAGELISTBOX,
	CTL_AMOUNTBOX,
	CTL_KEYLISTBOX,
	CTL_CUSTOMEDITBOX,	// [#2065] NH KJW 2011.05.26		// [#2350] US Justin Change Name
	CTL_DECIMALBOX,	// [#2316] Justin 2014.12.16 Add DecimalBox
	CTL_MAX
};


//------------------------------------------------------------------
//	CPicassoControl Define
//------------------------------------------------------------------
class CPicassoControl : public CPicassoEventHandler
{
public:
	CPicassoControl(CTL_TYPE type);
	~CPicassoControl();

public:
	CTL_TYPE	GetType();
	LPCTSTR		GetName();
	void		SetName(LPCTSTR str);
	int			GetLocaleNum();
	RECT		GetDisplayRect();
	RECT		GetMousePointRect();
	void		SetDisplayRect(int left, int top, int width, int height, int nLoadWidth, int nLoadHeight);
	void		SetAutoFocus(BOOL bAuto=TRUE);
	void		SetFocus(BOOL bFocus);
	BOOL		IsUpdateControl(RECT &rcUpdate);

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetLocaleNum(int Locale);

protected:
	RECT		GetAdjustScreenReate(int left, int top, int width, int height, int nLoadWidth, int nLoadHeight);

protected:
	RECT				m_rcDisplay;

protected:
	CString		m_strName;
	CTL_TYPE	m_CtlType;

	BOOL		m_bAutoFocus;
	BOOL		m_bFocus;

	int			m_nScaleUnit;

	static int	m_nLocaleNum;
};


//------------------------------------------------------------------
//	CPicassoShape Define
//------------------------------------------------------------------
class CPicassoShape : public CPicassoControl
{
public:
	CPicassoShape(CTL_TYPE type);
	~CPicassoShape();

public:
	void	SetPen(CPicassoResource *pNormal, CPicassoResource *pSelect=NULL);
	void	SetBrush(CPicassoResource *pNormal, CPicassoResource *pSelect=NULL);
	void	SetPicture(CPicassoResource *pPicture);
	void	SetRoundSize(int nWidth, int nHeight);
	void	SetIcon(CPicassoResource *pPicture, int pWidth = -1, int pHeight = -1);

public:
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetBlinking(void);

protected:
	int				m_nRoundWidth, m_nRoundHeight;
	CPicassoPen		*m_pPen, *m_pFocusPen;
	CPicassoBrush	*m_pBrush, *m_pFocusBrush;
	CPicassoPicture	*m_pPicture;
	CPicassoPicture	*m_pIcon;
	int				m_nIconWidth, m_nIconHeight;
};


//------------------------------------------------------------------
//	Text Align Define
//------------------------------------------------------------------
typedef enum TEXTALIGN
{
	LEFT=0,
	CENTER,
	RIGHT
};


//------------------------------------------------------------------
//	Text Show Type Define
//------------------------------------------------------------------
typedef enum TEXTTYPE
{
	TTYPE_TEXT=0,
	TTYPE_PASSWORD,
	TTYPE_ADDRESS	// "." "," 입력 가능한 TYPE
};


//------------------------------------------------------------------
//	CPicassoTextBox Define
//------------------------------------------------------------------
class CPicassoTextBox : public CPicassoShape
{
public:
	CPicassoTextBox(CTL_TYPE type=CTL_TEXTBOX);
	~CPicassoTextBox();

public:
	void	SetBlinkMode(BOOL BlinkOn);
	void	SetLocaleText(CPicassoResource *pLocaleText);
	void	SetText(LPCTSTR	pText);
	void	SetTextAlign(TEXTALIGN Align=LEFT);
	void	SetMultiLine(BOOL bMultiline=TRUE);
	void	SetTextColor(CPicassoColor *pColor, CPicassoColor *pFocusColor);
	void	SetFont(CPicassoFont *pNormal, CPicassoFont *pSelect=NULL);
	
	// [#2186] US KMK 2013.04.09 Asian Language Support
	void	SetFonts(CPicassoFont *pNormals[MAX_LOCALE], CPicassoFont *pSelects[MAX_LOCALE]);
	// end of [#2186]

	void	SetActKey(LPCTSTR pKey);
	void	SetAct(REQ_TYPE type, LPCTSTR pName, LPCTSTR pData);
	void	SetTextType(TEXTTYPE type);
	LPCTSTR		GetInputText(void);
	//void	SetResourcesQueue(CNHPtrLinkedList< CPicassoResource >	*pResourcesQueue);
	void	SetResourcesQueue(CMap< CString, LPCTSTR, CPicassoResource*, CPicassoResource* >	*pResourcesQueueCMap);
	void	SetOutline(int nWidth, CPicassoColor *pColor);

	// EVENT HANDLER
public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	SetBlinking(void);

	// CPicassoTextBox
protected:
	virtual CString GetDrawText();
	virtual BOOL	IsActKey(CString strKey);

protected:
	CPicassoLocaleText	*m_pLocaleText;
	CString			m_strText;

	CPicassoColor			*m_pColor, *m_pFocusColor;
	CPicassoFont			*m_pFont, *m_pFocusFont;

	// [#2186] US KMK 2013.04.09 Asian Language Support
	CPicassoFont			*m_pFonts[MAX_LOCALE], *m_pFocusFonts[MAX_LOCALE];
	// end of [#2186]

	TEXTALIGN		m_Align;
	BOOL			m_bMultiline;
	CPicassoRequest		m_Act;
	//CString			m_ActKey;
	CStringArray		m_arActKey;
	TEXTTYPE		m_TextType;

	int				m_nOutlineWidth;
	CPicassoColor	*m_pOutlineColor;

	BOOL		m_bBlinking;		// 0-show org text, 1-show Blink
	BOOL		m_bBlinkMode;		// Blink On/OFF
	int			m_nBlinkCount;
	BOOL		m_bShow;

	//CNHPtrLinkedList< CPicassoResource >	*m_pResourcesQueue;
	CMap< CString, LPCTSTR, CPicassoResource*, CPicassoResource* >	*m_pResourcesQueueCMap;

};

typedef struct
{
	CPicassoColor		*pColor;
	CPicassoColor		*pFocusColor;
	CPicassoFont		*pFont;
	CPicassoFont		*pFocusFont;
	CPicassoLocaleText	*pLocaleText;
	CString				strText;
	RECT				rcDraw;
} MULTI_STRING;  

#define MAX_MULTI_STRING	10

//------------------------------------------------------------------
//	CPicassoMulitTextBox Define
//------------------------------------------------------------------
class CPicassoMultiTextBox : public CPicassoShape
{
public:
	CPicassoMultiTextBox(CTL_TYPE type=CTL_MULTITEXTBOX);
	~CPicassoMultiTextBox();

public:
	void	SetMultiText(int index, LPCTSTR	pText);
	void	SetMultiLocaleText(int index, CPicassoResource *pLocaleText);
	void	SetMultiFont(int index, CPicassoFont *pNormal, CPicassoFont *pSelect=NULL);
	void	SetMultiTextColor(int index, CPicassoColor *pColor, CPicassoColor *pFocusColor);
	void	SetMultiTextAlign(TEXTALIGN alignValue);

	// EVENT HANDLER
public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	//virtual BOOL	SetVariable(CString &strName, CString &strData);
	//virtual BOOL	SetBlinking(void);

protected:
	RECT		GetDrawTextRect();
	int			GetDrawTextLength();
	CString		GetDrawText(int index);

protected:
	MULTI_STRING	m_MultiString[MAX_MULTI_STRING];
	TEXTALIGN		m_Align;
	BOOL			m_bCalcDrawTextRect;
/*
public:
	void	SetBlinkMode(BOOL BlinkOn);
	void	SetLocaleText(CPicassoResource *pLocaleText);
	void	SetText(LPCTSTR	pText);
	void	SetTextAlign(TEXTALIGN Align=LEFT);
	void	SetMultiLine(BOOL bMultiline=TRUE);
	void	SetTextColor(CPicassoColor *pColor, CPicassoColor *pFocusColor);
	void	SetFont(CPicassoFont *pNormal, CPicassoFont *pSelect=NULL);
	void	SetActKey(LPCTSTR pKey);
	void	SetAct(REQ_TYPE type, LPCTSTR pName, LPCTSTR pData);
	void	SetTextType(TEXTTYPE type);
	LPCTSTR		GetInputText(void);
	void	SetResourcesQueue(CNHPtrLinkedList< CPicassoResource >	*pResourcesQueue);
	void	SetOutline(int nWidth, CPicassoColor *pColor);

	// EVENT HANDLER
public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	SetBlinking(void);

	// CPicassoTextBox
protected:
	virtual CString GetDrawText();
	virtual BOOL	IsActKey(CString strKey);

protected:
	CPicassoLocaleText	*m_pLocaleText;
	CString			m_strText;

	CPicassoColor			*m_pColor, *m_pFocusColor;
	CPicassoFont			*m_pFont, *m_pFocusFont;
	TEXTALIGN		m_Align;
	BOOL			m_bMultiline;
	CPicassoRequest		m_Act;
	//CString			m_ActKey;
	CStringArray		m_arActKey;
	TEXTTYPE		m_TextType;

	int				m_nOutlineWidth;
	CPicassoColor	*m_pOutlineColor;

	BOOL		m_bBlinking;		// 0-show org text, 1-show Blink
	BOOL		m_bBlinkMode;		// Blink On/OFF
	int			m_nBlinkCount;
*/
	CNHPtrLinkedList< CPicassoResource >	*m_pResourcesQueue;

};


class CPicassoHideTextBox : public CPicassoTextBox
{
public:
	CPicassoHideTextBox(CTL_TYPE type=CTL_HIDETEXTBOX);
	~CPicassoHideTextBox();

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);

private:
	BOOL		m_bShow;
};

//------------------------------------------------------------------
//	Text Effective Type Define
//------------------------------------------------------------------
typedef enum EFFECTTYPE
{
	EFFECT_SLIDING_LEFT=0,
	EFFECT_SLIDING_RIGHT,
	EFFECT_TYPING_LEFT,
	EFFECT_TYPING_CENTER
};


//------------------------------------------------------------------
//	CPicassoEffectTextBox Define
//------------------------------------------------------------------
class CPicassoEffectTextBox : public CPicassoTextBox
{
public:
	CPicassoEffectTextBox(CTL_TYPE type=CTL_EFFECTTEXTBOX);
	~CPicassoEffectTextBox();

public:
	void	SetEffectType(EFFECTTYPE type);

public:
	// EVENT HANDLER
public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetBlinking(void);

protected:
	RECT	m_rcDrawText;
	int		m_widthDrawText;
	int		m_heightDrawText;
	POINT	m_curPos;
	int		m_moveWidth;
	EFFECTTYPE		m_EffectType;
	UINT	m_uTextOutFormat;
	int		m_ShowTextCount;
};


typedef enum EDITBOXTYPE
{
	EDIT_BOX_NORMAL = 0,
	EDIT_BOX_CENT,
	EDIT_BOX_DATE,
	EDIT_BOX_PASSWORD,
	EDIT_BOX_DECIMAL			// [#2316] Justin
};

//------------------------------------------------------------------
//	CPicassoEditTextBox Define
//------------------------------------------------------------------
class CPicassoEditTextBox : public CPicassoTextBox
{
public:
	CPicassoEditTextBox(CTL_TYPE type=CTL_EDITTEXTBOX);
	~CPicassoEditTextBox();

public:
	void	SetMaxInputChar(int nMaxChar);
	void	SetMinInputChar(int nChar);		// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
	void	SetAutoRun(BOOL bAutoRun);

public:
	//virtual BOOL	Handler(CPicassoRequest &Request);
public:
	virtual BOOL	Initialize(void);
	virtual BOOL	MouseEvent(POINT &point);
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	MakeAllFSCMD(CString &Data);

protected:
	virtual CString GetDrawText();

public:
	BOOL		m_bAutoRun;
	int			m_nMaxInputCharOrigin;
	int			m_nMaxInputChar;
	int			m_nMinInputChar;	// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
	CString		m_strTemp;
	EDITBOXTYPE	m_eEditBoxType;
};

//------------------------------------------------------------------
//	CPicassoWeatherBox Define
//------------------------------------------------------------------
#define MAX_WEATHER_CODE	10
#define MAX_WEATHER_DATA	2

struct WEATHER_DATA
{
	int		nYear;
	int		nMonth;
	int		nDay;
	CString strDayOfWeek;
	int		nWeatherCode;
	BOOL	bCelsius;
	int		nMax;
	int		nMin;
};

class CPicassoWeatherBox : public CPicassoTextBox
{
public:
	CPicassoWeatherBox(CTL_TYPE type=CTL_WEATHERBOX);
	~CPicassoWeatherBox();

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetVariable(CString &strName, CString &strData);

public:
	void SetBackImage(BOOL bCelsius, CPicassoResource *pImg);
	void SetWeatherCodeImage(int nCode, CPicassoResource *pImg);
	void SetBackRect(int left, int top, int width, int height, int nLoadWidth, int nLoadHeight);
	void SetTitleRect(int nIndex, int left, int top, int width, int height, int nLoadWidth, int nLoadHeight);
	void SetDateRect(int nIndex, int left, int top, int width, int height, int nLoadWidth, int nLoadHeight);
	void SetWeatherRect(int nIndex, int left, int top, int width, int height, int nLoadWidth, int nLoadHeight);
	void SetTemperatureRect(int nIndex, int left, int top, int width, int height, int nLoadWidth, int nLoadHeight);
	void SetNormalFontColor(CPicassoResource *pFont, CPicassoResource *pColor);
	void SetAccentFontColor(CPicassoResource *pFont, CPicassoResource *pColor);

private:
	void PrintText(HDC hDC, CString strText, UINT nAlign, RECT *pRect, CPicassoFont *pFont, CPicassoColor *pColor);

private:
	CPicassoPicture*	m_pWeatherBackImage[2];
	CPicassoPicture*	m_pWeatherCodeImage[MAX_WEATHER_CODE];

	RECT				m_rcBack;
	RECT				m_rcTitle[MAX_WEATHER_DATA];
	RECT				m_rcDate[MAX_WEATHER_DATA];
	RECT				m_rcWeather[MAX_WEATHER_DATA];
	RECT				m_rcTemperature[4];

	BOOL				m_bShow;

	CPicassoFont		*m_pNormalFont;
	CPicassoFont		*m_pAccentFont;

	CPicassoColor		*m_pNormalColor;
	CPicassoColor		*m_pAccentColor;

	WEATHER_DATA		m_sWeatherData[MAX_WEATHER_DATA];
};

//------------------------------------------------------------------
//	CPicassoVKeyBoardBox Define
//------------------------------------------------------------------
struct VKeyboardButton
{
	BOOL		bShow;	// 1- show, 0 - hide
	CRect		rcRect;
	CPicassoPicture	*pImage;
	
	BOOL		bDrawText;
	CPicassoFont		*pFont;
	CPicassoColor		*pColor;
};

#define MAX_KEYBOARD_TYPE	4
#define MAX_KEYBOARD_ROW	4
#define MAX_KEYBOARD_COL	10

class CPicassoVKeyBoardBox : public CPicassoTextBox
{
public:
	CPicassoVKeyBoardBox(CTL_TYPE type=CTL_VKEYBOARDBOX);
	~CPicassoVKeyBoardBox();

public:
	void	SetButtonGap(int nGap, int nLoadWidth, int nLoadHeight);
	void	SetLineHeight(int nHeight, int nLoadWidth, int nLoadHeight);
	
	void	SetNormalButtonImage(CPicassoPicture *pImage);
	void	SetNormalButtonSize(int nWidth, int nHeight, int nLoadWidth, int nLoadHeight);
	void	SetNormalButtonFont(CPicassoFont *pFont);
	void	SetNormalButtonTextColor(CPicassoColor *pColor);

//	void	SetControlButtonImage(CPicassoPicture *pImage);
	void	SetControlButtonSize(int nWidth, int nHeight, int nLoadWidth, int nLoadHeight);

//	void	SetControlButtonFont(CPicassoFont *pFont);
//	void	SetControlButtonTextColor(CPicassoColor *pColor);

	void	SetClearButtonImage(CPicassoPicture *pImage);
	void	Set123ButtonImage(CPicassoPicture *pImage);
	void	SetABCButtonImage(CPicassoPicture *pImage);
	void	SetCapsButtonImage(CPicassoPicture *pImage);
	void	SetSymbol1ButtonImage(CPicassoPicture *pImage);
	void	SetSymbol2ButtonImage(CPicassoPicture *pImage);

	void	SetSpaceButtonImage(CPicassoPicture *pImage);
	void	SetSpaceButtonSize(int nWidth, int nHeight, int nLoadWidth, int nLoadHeight);

	void	SetUpArrowImage(CPicassoPicture *pImage);
	void	SetDownArrowImage(CPicassoPicture *pImage);
	void	SetLeftArrowImage(CPicassoPicture *pImage);
	void	SetRightArrowImage(CPicassoPicture *pImage);
	void	SetArrowButtonSize(int nWidth, int nHeight, int nLoadWidth, int nLoadHeight);

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	MouseEvent(POINT &point);
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	SetBlinking(void);

private:
	BOOL	m_bSetButtonPosition;
	int		m_nButtonGap;
	int		m_nLineHeight;

	CPicassoPicture*	m_pNormalButtonImage;
//	CPicassoPicture*	m_pControlButtonImage;
	CPicassoPicture*	m_pClearButtonImage;
	CPicassoPicture*	m_p123ButtonImage;
	CPicassoPicture*	m_pABCButtonImage;
	CPicassoPicture*	m_pCapsButtonImage;
	CPicassoPicture*	m_pSymbol1ButtonImage;
	CPicassoPicture*	m_pSymbol2ButtonImage;
	CPicassoPicture*	m_pSpaceButtonImage;
	CPicassoPicture*	m_pUpArrowImage;
	CPicassoPicture*	m_pDownArrowImage;
	CPicassoPicture*	m_pLeftArrowImage;
	CPicassoPicture*	m_pRightArrowImage;

	CPicassoFont*		m_pNormalButtonFont;
//	CPicassoFont*		m_pControlButtonFont;

	CPicassoColor*	m_pNormalButtonTextColor;
//	CPicassoColor*	m_pControlButtonTextColor;

	SIZE	m_stNormalButton;
	SIZE	m_stControlButton;
	SIZE	m_stSpaceButton;
	SIZE	m_stArrowButton;

	int		m_nKeyboardType;

	VKeyboardButton	m_stKeyBoardButton[MAX_KEYBOARD_ROW][MAX_KEYBOARD_COL];
};

//------------------------------------------------------------------
//	Table Element info Define
//------------------------------------------------------------------
struct TableElement
{
	int		nRow;
	int		nCol;
	BOOL	bAllowSelect;
	CRect	rcRect;
	CString	strText;
};


//------------------------------------------------------------------
//	Define for CPicassoTableEditTextBox
//------------------------------------------------------------------
#define MAX_TABLE_ROW	6
#define MAX_TABLE_COL	41

#define DISABLE_TYPE	0
#define ENABLE_TYPE		1
#define SELECT_TYPE		2
#define MAX_TYPE		3


//------------------------------------------------------------------
//	CPicassoTableEditTextBox Define
//------------------------------------------------------------------
class CPicassoTableEditTextBox : public CPicassoShape
{
public:
	CPicassoTableEditTextBox(CTL_TYPE type=CTL_TABLEEDITTEXTBOX);
	~CPicassoTableEditTextBox();	

public:
	void	SetAllowAmount(int nRow, int nColumn);
	void	SetGuideTextColor(CPicassoColor *pNormal, CPicassoColor *pSelect);
	void	SetGuideFont(CPicassoFont *pNormal, CPicassoFont *pSelect);
	void	SetElementPen(CPicassoPen *pDisable, CPicassoPen *pEnable, CPicassoPen *pSelect);
	void	SetElementFont(CPicassoFont *pDisable, CPicassoFont *pEnable, CPicassoFont *pSelect);
	void	SetElementBrush(CPicassoBrush *pDisable, CPicassoBrush *pEnable, CPicassoBrush *pSelect);
	void	SetElementTextColor(CPicassoColor *pDisable, CPicassoColor *pEnable, CPicassoColor *pSelect);

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	MakeAllFSCMD(CString &Data);

private:
	TableElement	m_TableElement[MAX_TABLE_ROW][MAX_TABLE_COL];
	TableElement	*m_pCurElement;
	
	int				m_nAllowRow;
	int				m_nAllowColumn;

	CPicassoPen			*m_pGuideFont[2];
	CPicassoColor			*m_pGuideTextColor[2];

	CPicassoPen			*m_pElementPen[MAX_TYPE];
	CPicassoFont			*m_pElementFont[MAX_TYPE];
	CPicassoBrush			*m_pElementBrush[MAX_TYPE];
	CPicassoColor			*m_pElementTextColor[MAX_TYPE];
};


//------------------------------------------------------------------
//	CPicassoButtonBox Define
//------------------------------------------------------------------
class CPicassoButtonBox : public CPicassoTextBox
{
public:
	CPicassoButtonBox(CTL_TYPE type=CTL_BUTTONBOX);
	~CPicassoButtonBox();

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	MouseEvent(POINT &point);			// [2ND] NH AIREAT 2008.11.12
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	FindActBtn(CString &strKey);

protected:
	BOOL	m_bShow;
};


//------------------------------------------------------------------
//	CPicassoVirtualKeyButtonBox Define
//------------------------------------------------------------------
class CPicassoVirtualKeyButtonBox : public CPicassoButtonBox
{
public:
	CPicassoVirtualKeyButtonBox(CTL_TYPE type=CTL_VKEYBUTTONBOX);
	~CPicassoVirtualKeyButtonBox();

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
};


//------------------------------------------------------------------
//	Change Password Box Type Define
//------------------------------------------------------------------
typedef enum CHANGEPWTYPE
{
	CPTYPE_CUR=0,
	CPTYPE_NEW,
	CPTYPE_NEWRE,
	CPTYPE_MAX
};

//------------------------------------------------------------------
//	CPicassoChangePasswordBox Define
//------------------------------------------------------------------
class CPicassoChangePasswordBox : public CPicassoEditTextBox
{
public:
	CPicassoChangePasswordBox(CTL_TYPE type=CTL_CHANGEPWBOX);
	~CPicassoChangePasswordBox();

public:
	void	SetPWBoxRect(CHANGEPWTYPE type, int left, int top, int width, int height, int nLoadWidth, int nLoadHeight);

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	MakeAllFSCMD(CString &Data);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);

private:
	CHANGEPWTYPE		m_curInputBox;
	CPicassoEditTextBox	m_ChangePW[CPTYPE_MAX];
};


//------------------------------------------------------------------
//	CPicassoSoftwareKeyBox Define
//------------------------------------------------------------------
class CPicassoSoftwareKeyBox : public CPicassoTextBox
{
public:
	CPicassoSoftwareKeyBox(CTL_TYPE type=CTL_SWKEYBOX);
	~CPicassoSoftwareKeyBox();

public:
	void	SetNumberSize(SIZE Number);
	void	SetGuideSize(SIZE Gudie);
	void	SetSpaceSize(SIZE Space);

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	KeyEvent(CString &strKey);

private:
	SIZE	m_sizeNumber;
	SIZE	m_sizeGuide;
	SIZE	m_sizeSpace;
	int		m_nSwKeyDataIndex;
};


//------------------------------------------------------------------
//	Virtual Key Element Define
//------------------------------------------------------------------
struct VKeyElement
{
	int		nUseType;	// 0 - None, 1 - Guide, 2 - VKey
	int		nRow;
	int		nCol;
	CRect	rcRect;
	CString	strText;
};

//------------------------------------------------------------------
//	Define for CPicassoVirtualKeyBox
//------------------------------------------------------------------

#define USE_KEY_NONE		0
#define USE_KEY_GUIDE		1
#define USE_KEY_VKEY		2
#define	USE_KEY_UNDER_GUIDE	3

#define MAX_KEY_ROW			6
#define MAX_KEY_COL			19

//------------------------------------------------------------------
//	CPicassoVirtualKeyBox Define
//------------------------------------------------------------------
class CPicassoVirtualKeyBox : public CPicassoShape
{
public:
	CPicassoVirtualKeyBox(CTL_TYPE type);
	~CPicassoVirtualKeyBox();

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);

public:
	void	SetGuideFont(CPicassoFont *pNormal, CPicassoFont *pSelect = NULL);
	void	SetGuideBrush(CPicassoBrush *pNormal, CPicassoBrush *pSelect = NULL);
	void	SetGuidePen(CPicassoPen *pNormal, CPicassoPen *pSelect = NULL);
	void	SetGuideTextColor(CPicassoColor *pNormal, CPicassoColor *pSelect = NULL);

	void	SetVKeyFont(CPicassoFont *pNormal, CPicassoFont *pSelect = NULL);
	void	SetVKeyBrush(CPicassoBrush *pNormal, CPicassoBrush *pSelect = NULL);
	void	SetVKeyPen(CPicassoPen *pNormal, CPicassoPen *pSelect = NULL);
	void	SetVKeyTextColor(CPicassoColor *pNormal, CPicassoColor *pSelect = NULL);

protected:
	VKeyElement		m_KeyTable[MAX_KEY_ROW][MAX_KEY_COL];
	VKeyElement		*m_pCurKey;

	BOOL			m_bShow;
	int				m_nAllowRow;
	int				m_nAllowCol;

	CPicassoFont			*m_pGuideNormalFont, *m_pGuideFocusFont;
	CPicassoBrush			*m_pGuideNormalBrush, *m_pGuideFocusBrush;
	CPicassoPen			*m_pGuideNormalPen, *m_pGuideFocusPen;
	CPicassoColor			*m_pGuideNormalTextColor, *m_pGuideFocusTextColor;

	CPicassoFont			*m_pVKeyNormalFont,	*m_pVKeyFocusFont;
	CPicassoBrush			*m_pVKeyNormalBrush, *m_pVKeyFocusBrush;
	CPicassoPen			*m_pVKeyNormalPen, *m_pVKeyFocusPen;
	CPicassoColor			*m_pVKeyNormalTextColor, *m_pVKeyFocusTextColor;
};

////////////////////////////////////////////////
//	CNNumberKeyBox
//

typedef enum VNUMBERKEY_TYPE
{
	NUMBER_KEY_NUMBER = 0,
	NUMBER_KEY_TELE_NUMBER,
};

class CPicassoNumberKeyBox : public CPicassoVirtualKeyBox
{
public:
	CPicassoNumberKeyBox(CTL_TYPE type=CTL_NUMBERKEYBOX);
	~CPicassoNumberKeyBox();

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	KeyEvent(CString &strKey);

public:
	void	SetNumberKeyType(VNUMBERKEY_TYPE type);

private:
	VNUMBERKEY_TYPE	m_nNumberKeyType;
};


//------------------------------------------------------------------
//	Virtual Key Type Define
//------------------------------------------------------------------
typedef enum VKEY_TYPE
{
	VKEY_ALPHA_UPPER = 0,
	VKEY_ALPHA_LOWER,
	VKEY_ALPHA_NUMBER,
};


//------------------------------------------------------------------
//	CPicassoAlphaKeyBox Define
//------------------------------------------------------------------
class CPicassoAlphaKeyBox : public CPicassoVirtualKeyBox
{
public:
	CPicassoAlphaKeyBox(CTL_TYPE type=CTL_ALPHAKEYBOX);
	~CPicassoAlphaKeyBox();

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	MouseEvent(POINT &point);
	virtual BOOL	KeyEvent(CString &strKey);

public:
	void	SetAlphaKeyType(VKEY_TYPE	type);

private:
	VKEY_TYPE	m_nAlphaKeyType;
	BOOL	m_bKeyByMouse;
};


//------------------------------------------------------------------
//	CPicassoTableKeyBox Define
//------------------------------------------------------------------
class CPicassoTableKeyBox : public CPicassoVirtualKeyBox
{
public:
	CPicassoTableKeyBox(CTL_TYPE type=CTL_TABLEKEYBOX);
	~CPicassoTableKeyBox();

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	MouseEvent(POINT &point);
	virtual BOOL	KeyEvent(CString &strKey);

private:
	BOOL	m_bKeyByMouse;
};


//------------------------------------------------------------------
//	CPicassoToggleBox Define
//------------------------------------------------------------------
class CPicassoToggleBox : public CPicassoTextBox
{
public:
	CPicassoToggleBox(CTL_TYPE type=CTL_TOGGLEBOX);
	~CPicassoToggleBox();

public:
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	MakeAllFSCMD(CString &Data);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetBlinking(void);

	void	SetEnableText(LPCTSTR lpText);
	void	SetDisableText(LPCTSTR lpText);

private:
	CString		m_strEnableText;
	CString		m_strDisableText;
};


//------------------------------------------------------------------
//	Define for CPicassoThreeStateBox
//------------------------------------------------------------------
#define STATE_HIDE	0
#define STATE_ON	1
#define STATE_OFF	2


//------------------------------------------------------------------
//	CPicassoThreeStateBox Define
//------------------------------------------------------------------
class CPicassoThreeStateBox : public CPicassoTextBox
{
public:
	CPicassoThreeStateBox(CTL_TYPE type=CTL_THREESTATEBOX);
	~CPicassoThreeStateBox();

public:
	void	SetStateImage(CPicassoPicture *pHide, CPicassoPicture *pOn, CPicassoPicture *pOff);
	void	SetStatePen(CPicassoPen *pHide, CPicassoPen *pOn, CPicassoPen *pOff);
	void	SetStateBrush(CPicassoBrush *pHide, CPicassoBrush *pOn, CPicassoBrush *pOff);
	void	SetStateFont(CPicassoFont *pHide, CPicassoFont *pOn, CPicassoFont *pOff);
	void	SetStateTextColor(CPicassoColor *pHide, CPicassoColor *pOn, CPicassoColor *pOff);
	void	SetStateBlink(BOOL bHide, BOOL bOn, BOOL bOff);
public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetBlinking(void);

private:
	CPicassoPicture		*m_pStateImage[3];
	CPicassoPen			*m_pStatePen[3];
	CPicassoBrush			*m_pStateBrush[3];
	CPicassoFont			*m_pStateFont[3];
	CPicassoColor			*m_pStateTextColor[3];
	BOOL			m_bStateBlink[3];

	int				m_nCurState;
};


//------------------------------------------------------------------
//	CPicassoEJNLBox Define
//------------------------------------------------------------------
class CPicassoEJNLBox : public CPicassoTextBox
{
public:
	CPicassoEJNLBox(CTL_TYPE type=CTL_EJNLBOX);
	~CPicassoEJNLBox();

public:
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);

protected:
	virtual CString GetDrawText();
};


//------------------------------------------------------------------
//	CPicassoDateBox Define
//------------------------------------------------------------------
class CPicassoDateBox : public CPicassoTextBox
{
public:
	CPicassoDateBox(CTL_TYPE type=CTL_DATEBOX);
	~CPicassoDateBox();

private:
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	MakeAllFSCMD(CString &Data);

private:
	CString			m_strTemp;
};


//------------------------------------------------------------------
//	CPicassoIPBox Define
//------------------------------------------------------------------
class CPicassoIPBox : public CPicassoEditTextBox
{
public:
	CPicassoIPBox(CTL_TYPE type=CTL_IPBOX);
	~CPicassoIPBox();

private:
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);

private:
	CString			m_strTemp;
};


//------------------------------------------------------------------
//	CPicassoCentBox Define
//------------------------------------------------------------------
class CPicassoCentBox : public CPicassoEditTextBox
{
public:
	CPicassoCentBox(CTL_TYPE type=CTL_CENTBOX);
	~CPicassoCentBox();

protected:
	virtual CString GetDrawText();
	virtual BOOL DrawControl(HDC hDC, RECT &rcUpdate);
};


//------------------------------------------------------------------
//	CPicassoDollarBox Define
//------------------------------------------------------------------
class CPicassoDollarBox : public CPicassoEditTextBox
{
public:
	CPicassoDollarBox(CTL_TYPE type=CTL_DOLLARBOX);
	~CPicassoDollarBox();

protected:
	virtual CString GetDrawText();
};

//------------------------------------------------------------------
//	CPicassoAmountBox Define
//------------------------------------------------------------------
typedef enum SYMBOL_TYPE
{
	SYMBOL_NONE = 0,
	SYMBOL_DOLLAR,
};

typedef enum AMOUNT_TYPE
{
	AMOUNT_INT = 0,
	AMOUNT_INT_CENT,
	AMOUNT_INT_DOLLAR,
	AMOUNT_STRING,
};

class CPicassoAmountBox : public CPicassoTextBox
{
public:
	CPicassoAmountBox(CTL_TYPE type=CTL_AMOUNTBOX);
	~CPicassoAmountBox();

public:
	void	SetCurrencySymbolName(CString strName);
	void	SetAmountTypeName(CString strName);
	void	SetInputMinName(CString strName);
	void	SetInputMaxName(CString strName);
	void	SetAutoRunName(CString strName);

protected:
	virtual BOOL	Initialize(void);
	virtual BOOL	MouseEvent(POINT &point);
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	MakeAllFSCMD(CString &Data);

	virtual CString GetDrawText();

private:
	CString			m_strCurrencySymbolName;
	SYMBOL_TYPE		m_eSymbolType;
	CString			m_strAmountTypeName;
	AMOUNT_TYPE		m_eAmountType;
	CString			m_strInputMinName;
	int				m_nInputMin;
	CString			m_strInputMaxName;
	int				m_nInputMax;
	CString			m_strAutoRunName;
	BOOL			m_bAutoRun;
	CString			m_strTemp;
};

//------------------------------------------------------------------
//	BIN Command Type Define
//------------------------------------------------------------------
typedef enum BINTYPE
{
	BNTYPE_INDEX=0,
	BNTYPE_BIN,
	BNTYPE_ADD,
	BNTYPE_DEL,
	BNTYPE_EDT,
	BNTYPE_MAX
};

//------------------------------------------------------------------
//	CPicassoBinBox Define
//------------------------------------------------------------------
class CPicassoBinBox : public CPicassoTextBox
{
public:
	CPicassoBinBox(CTL_TYPE type=CTL_BINBOX);
	~CPicassoBinBox();

public:
	void	SetBinBoxRect(BINTYPE type, int left, int top, int width, int height, int nLoadWidth, int nLoadHeight);

protected:
	virtual BOOL	Initialize(void);
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	BinAct(CString &strAct);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);

private:
	BINTYPE			m_curInput;
	BINTYPE			m_BinAct;
	CPicassoEditTextBox	m_BinItem[BNTYPE_MAX];
};

//------------------------------------------------------------------
//	Define for CPicassoAnimation
//------------------------------------------------------------------
#define MAX_ANI		10

//------------------------------------------------------------------
//	CPicassoRequest Define
//------------------------------------------------------------------
class CPicassoAnimation : public CPicassoShape
{
public:
	CPicassoAnimation(CTL_TYPE type=CTL_ANI);
	~CPicassoAnimation();

public:
	BOOL SetAniImage(int Count, CString strPrefix, CString strExt);

protected:
	virtual BOOL	Initialize(void);
	virtual BOOL	Deinitialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	SetBlinking(void);

private:
	void	ImageRelease();

private:
	BOOL		m_bShow;
	int			m_nImageIndex;
	int			m_nLoadImage;
	int			m_nBlinkCount;
	
	CPicassoImage	m_Image;
};

//------------------------------------------------------------------
//	CPicassoTranAdvBox Define
//------------------------------------------------------------------
#define MAX_SUPPORT_TRANADV	10

class CPicassoTranAdvBox : public CPicassoTextBox
{
public:
	CPicassoTranAdvBox(CTL_TYPE type=CTL_TRANADVBOX);
	~CPicassoTranAdvBox();

protected:
	virtual BOOL	Initialize(void);
	virtual BOOL	Deinitialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	SetBlinking(void);

private:
	CString			m_strFileName[MAX_SUPPORT_TRANADV];
	CPicassoImage	m_Image[MAX_SUPPORT_TRANADV];
	int				m_nTranAdvCount;
	int				m_nCurIndex;
	DWORD			m_dwTranAdvShowTime;		//sec
	DWORD			m_dwShowTime;
};


//------------------------------------------------------------------
//	CPicassoAdvBox Define
//------------------------------------------------------------------
class CPicassoAdvBox : public CPicassoTextBox
{
public:
	CPicassoAdvBox(CTL_TYPE type=CTL_ADVBOX);
	~CPicassoAdvBox();

public:
	void	SetFileName(CString strFileName);
	BOOL	DecodeImage();
	BOOL	IsDecode();

protected:
	virtual BOOL	Initialize(void);
	virtual BOOL	Deinitialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetVariable(CString &strName, CString &strData);

private:
	void	ImageRelease();

private:
	CString		m_strFileName;
	CPicassoImage	m_Image;
};


//------------------------------------------------------------------
//	CPicassoBinIndexBox Define
//------------------------------------------------------------------
class CPicassoBinIndexBox : public CPicassoEditTextBox
{
public:
	CPicassoBinIndexBox(CTL_TYPE type=CTL_BININDEXBOX);
	~CPicassoBinIndexBox();

private:
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	BinAct(CString &strAct);

private:
	BINTYPE			m_BinAct;
};

//------------------------------------------------------------------
//	CPicassoImageListBox Define
//------------------------------------------------------------------
struct ImageList
{
	BOOL		bSet;
	CString		strName;
	CPicassoPicture *pImage;
};

#define MAX_IMAGE_LIST	10

class CPicassoImageListBox : public CPicassoTextBox
{
public:
	CPicassoImageListBox(CTL_TYPE type=CTL_IMAGELISTBOX);
	~CPicassoImageListBox();

public:
	void SetImageList(CString strName, CPicassoPicture *pImage);
	void SetInitList(CString strName);

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);

protected:
	int			m_nCurImageList;
	CString		m_strInitName;
	ImageList	m_arImageList[MAX_IMAGE_LIST];
};

//------------------------------------------------------------------
//	CPicassoButtonImageListBox Define
//------------------------------------------------------------------

class CPicassoButtonImageListBox : public CPicassoImageListBox
{
public:
	CPicassoButtonImageListBox(CTL_TYPE type=CTL_BUTTONIMAGELISTBOX);
	~CPicassoButtonImageListBox();

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	FindActBtn(CString &strKey);
	virtual BOOL	MouseEvent(POINT &point);
	virtual BOOL	KeyEvent(CString &strKey);

protected:
	BOOL	m_bShow;
};

//------------------------------------------------------------------
//	CPicassoKeyListBox Define
//------------------------------------------------------------------

struct KeyList
{
	BOOL		bSet;
	CString		strName;
};

#define MAX_KEY_LIST	20

class CPicassoKeyListBox : public CPicassoTextBox
{
public:
	CPicassoKeyListBox(CTL_TYPE type=CTL_KEYLISTBOX);
	~CPicassoKeyListBox();

public:
	void	SetKeyList(CString strKeyName);

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	KeyEvent(CString &strKey);

protected:
	KeyList		m_KeyList[MAX_KEY_LIST];
};

// [#2065] NH KJW 2011.05.26 Telephone Number Box 추가
// [#2350] US Justin Change Name For General Use
///////////////// USAGE /////////////////////////////
///////////////// USAGE /////////////////////////////
//    Key    : DispFormat 
//    Displaying : N=>Display Number,	*=>Mask Number with *
//	  OTHER  : All other characters
//    EXAMPLE
//         DispFormat=NNN-***-N*N*	    (Entered string=1234567890)
//         ON SCREEN => 123-***-7*9*
///////////////// USAGE /////////////////////////////
///////////////// USAGE /////////////////////////////
//------------------------------------------------------------------
//	CPicassoCustomEditBox Define
//------------------------------------------------------------------
class CPicassoCustomEditBox : public CPicassoEditTextBox		// [#2350] US Justin Change Name
{
public:
	CPicassoCustomEditBox(CTL_TYPE type=CTL_CUSTOMEDITBOX);
	~CPicassoCustomEditBox();

	void	SetDisplayFormat(CString strDispFormat);			// [#2350] US Justin Support Custom Display

protected:
	virtual CString GetDrawText();

	// [#2350] US Justin Support Custom Display
	CString m_strDisplayFormat;
	int		m_nDefinedDisplayNumber;
	// End of [#2350]
};
// end of [#2065]

// [#2316] US Justin 2014.12.16 Add Decimal Box
////////////////////////////////////////////////
//	CPicassoDecimalBox
//
class CPicassoDecimalBox : public CPicassoEditTextBox
{
public:
	CPicassoDecimalBox(CTL_TYPE type=CTL_DECIMALBOX);
	~CPicassoDecimalBox();

protected:
	virtual CString GetDrawText();

};
// End of [#2316]

#endif __PICASSO_CONTROL_H__