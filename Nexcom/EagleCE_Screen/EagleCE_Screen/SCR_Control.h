/** *************************************************************
*	@file		SCR_Control.h
*	@author		MFS corporation
*	@brief		Text Box, Edit Box 등 Resource control을 관리한다.
*****************************************************************/
#ifndef __N_CONTROL_H__
#define __N_CONTROL_H__

#include "stdafx.h"
#include ".\SCR_Collection.h"
#include ".\SCR_Resource.h"

#define		NH_INDENT				3
#define		MAX_REQ					20

/** *********************************
Text 입력모드 종류 
************************************/
typedef enum INPUT_MODE
{
	PASSWORD	= 0x01,
	UPPERCASE	= 0x02,
	LOWERCASE	= 0x04,
	NUMBER		= 0x08,
	DATEMODE	= 0x10,
	TIMEMODE	= 0x20,
	SPECIALCHAR	= 0x40,
	HEXADECIMAL = 0x80,
};

/** *********************************
 Req Type 
************************************/
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
	REQ_MAX,
	REQ_SETINPUTMODE,
	REQ_KEYDISABLE,
	REQ_SETINPUTTYPE,
	REQ_SETBLINKMODE
};

/** *************************************************************
*	@class		CRequest
*	@author		MFS corporation
*	@brief		명령(요청)의 종류, Data 등을 세팅하고 가져온다.
*****************************************************************/
class CRequest
{
public:
	CRequest();
	CRequest(REQ_TYPE type, CString Name, CString Data);
	CRequest(const CRequest &rRequest);
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

/** *************************************************************
*	@class		CEventHandler
*	@author		MFS corporation
*	@brief		EventHandler 클래스
*****************************************************************/
class CEventHandler
{
public:
	CEventHandler(CEventHandler *pNext=0);

public:
	void	SetNextHandler(CEventHandler *pNext);
	
	BOOL	AddRequest(REQ_TYPE type, LPCTSTR Name, LPCTSTR Data);
	BOOL	AddRequest(CRequest &rRequest);
	BOOL	GetRequest(CRequest &rRequest);
	BOOL	ClearRequest(void);

	void	AddInvalidRect(RECT rcRect);
	RECT	GetInvalidRect();
	void	ClearInvalidRect();

public:
	BOOL	Handler(CRequest &pRequest);
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
	virtual BOOL	SetInputMode(CString &strData);
	virtual BOOL	SetKeyDisable(CString &strName);
	virtual BOOL	SetInputType(CString &strData);
	virtual BOOL	SetAPBlinkMode(BOOL bBlinkOn);

private:
	static CNHLinkedList<CRequest>	m_Requests;
	CEventHandler						*m_pNextHandler;
	static	RECT						m_InvalidRect;
};

/** *********************************
CTL Type
************************************/
typedef enum CTL_TYPE
{
	CTL_TEXTBOX = 0,
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
	CTL_BININDEXBOX,
	CTL_DECIMALBOX,	
	CTL_CENTCLEARBOX,
	CTL_MAX,
	CTL_KEYTESTBOX,
	CTL_INPUTEDITBOX,
	CTL_THREETYPEBOX,
	CTL_GIFBOX,
	CTL_IMAGEBOX,
	CTL_EDITBUTTONBOX
};

/** *************************************************************
*	@class		CNControl
*	@author		MFS corporation
*	@brief		NControl 클래스
*****************************************************************/
class CNControl : public CEventHandler
{
public:
	CNControl(CTL_TYPE type);
	~CNControl();

public:
	CTL_TYPE	GetType();
	LPCTSTR		GetName();
	void		SetName(LPCTSTR str);
	int			GetLocaleNum();
	RECT		GetDisplayRect();
	void		SetDisplayRect(int left, int top, int width, int height);
	void		SetAutoFocus(BOOL bAuto=TRUE);
	void		SetFocus(BOOL bFocus);
	BOOL		IsUpdateControl(RECT &rcUpdate);
	void		SetNameEx(LPCTSTR str1, LPCTSTR str2);

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetLocaleNum(int Locale);

private:
	// 주어진 좌표(디자인시)에 대하여 현재 스크린 해상도의 좌표로 변환한다. 
	RECT		GetAdjustScreenRate(int left, int top, int width, int height);

private:
	static float		m_fScreenRateX;		// current screen X에 대한 800의 비율 
	static float		m_fScreenRateY;		// current screen Y에 대한 480의 비율 

protected:
	CString		m_strName;
	CString		m_strNameEx[2];
	CTL_TYPE	m_CtlType;

	RECT		m_rcDisplay;

	BOOL		m_bAutoFocus;
	BOOL		m_bFocus;

	static int	m_nLocaleNum;
	static int	m_nRefCount;
};

/** *************************************************************
*	@class		CNShape
*	@author		MFS corporation
*	@brief		Pen, Brush, Picture 등을 관리한다.
*****************************************************************/
class CNShape : public CNControl
{
public:
	CNShape(CTL_TYPE type);
	~CNShape();

public:
	void	SetPen(CNResource *pNormal, CNResource *pSelect=NULL);
	void	SetBrush(CNResource *pNormal, CNResource *pSelect=NULL);
	void	SetPicture(CNResource *pPicture);
	void	SetRoundSize(int nWidth, int nHeight);	//*LEH

public:
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetBlinking(void);

protected:
	CNPen		*m_pPen, *m_pFocusPen;
	CNBrush		*m_pBrush, *m_pFocusBrush;
	CNPicture	*m_pPicture;
	int			m_nRoundWidth, m_nRoundHeight;	//*LEH
};

/** TEXT ALIGN */
typedef enum TEXTALIGN
{
	LEFT=0,
	CENTER,
	RIGHT
};

/** TEXT Vertical ALIGN */
typedef enum TEXTVERTALIGN
{
	TOP = 0,
	CENTER_V,
	BOTTOM
};

/** TEXT TYPE */
typedef enum TEXTTYPE
{
	TTYPE_TEXT=0,
	TTYPE_PASSWORD,
	TTYPE_IPADDRESS
};

/** *************************************************************
*	@class		CNTextBox
*	@author		MFS corporation
*	@brief		TextBox 클래스
*****************************************************************/
class CNTextBox : public CNShape
{
public:
	CNTextBox(CTL_TYPE type=CTL_TEXTBOX);
	~CNTextBox();

public:
	void	SetBlinkMode(BOOL BlinkOn);
	void	SetLocaleText(CNResource *pLocaleText);
	void	SetText(LPCTSTR	pText);
	void	SetTextAlign(TEXTALIGN Align=LEFT);
	void	SetMultiLine(BOOL bMultiline=TRUE);
	void	SetTextColor(CNColor *pColor, CNColor *pFocusColor);
	void	SetFont(CNFont *pNormal, CNFont *pSelect=NULL);
	void	SetActKey(LPCTSTR pKey);
	void	SetAct(REQ_TYPE type, LPCTSTR pName, LPCTSTR pData);
	void	SetTextType(TEXTTYPE type);
	LPCTSTR		GetInputText(void);

	void SetBlinkOption(BOOL BlinkOn);

	void SetTextVerticalAlign(TEXTVERTALIGN Align);		//Add vertical alligment
public:
	// EVENT HANDLER
public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	SetBlinking(void);
	virtual BOOL	SetAPBlinkMode(BOOL bBlinkOn);	///< Blink 수행을 설정

	// CNTextBox
protected:
	virtual CString GetDrawText();		// 현재 뿌려질 텍스트를 가져온다.

protected:
	CNLocaleText	*m_pLocaleText;
	CString			m_strText;

	CNColor			*m_pColor, *m_pFocusColor;
	CNFont			*m_pFont, *m_pFocusFont;
	TEXTALIGN		m_Align;
	TEXTVERTALIGN	m_VertAlign;	//Add vertical alligment
	BOOL			m_bMultiline;
	CRequest		m_Act;
	CString			m_ActKey;
	TEXTTYPE		m_TextType;

	BOOL		m_bBlinking;		// 0-show org text, 1-show Blink
	BOOL		m_bBlinkMode;		// Blink On/OFF
	int			m_nBlinkCount;
	BOOL		m_bBlinkOption;		// Enable Blinking in script

	BOOL		m_bShow;

};

/** EFFECT TYPE */
typedef enum EFFECTTYPE
{
	EFFECT_SLIDING_LEFT=0,
	EFFECT_SLIDING_RIGHT,
	EFFECT_TYPING_LEFT,
	EFFECT_TYPING_CENTER
};

/** *************************************************************
*	@class		CNEffectTextBox
*	@author		MFS corporation
*	@brief		NEffectTextBox 클래스
*****************************************************************/
class CNEffectTextBox : public CNTextBox
{
public:
	CNEffectTextBox(CTL_TYPE type=CTL_EFFECTTEXTBOX);
	~CNEffectTextBox();

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

/** *************************************************************
*	@class		CNEditTextBox
*	@author		MFS corporation
*	@brief		NEditTextBox 클래스
*****************************************************************/
class CNEditTextBox : public CNTextBox
{
public:
	CNEditTextBox(CTL_TYPE type=CTL_EDITTEXTBOX);
	~CNEditTextBox();

public:
	void	SetMaxInputChar(int nMaxChar);
	void	SetMinInputChar(int nChar);
	void	SetAutoRun(BOOL bAutoRun);

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	MakeAllFSCMD(CString &Data);
	virtual BOOL	SetKeyDisable(CString &strName);

public:
	BOOL		m_bAutoRun;
	int			m_nMaxInputCharOrigin;
	int			m_nMaxInputChar;
	int			m_nMinInputChar;
	CString		m_strTemp;
	BOOL		m_bKeyDisable;
};

/** *************************************************************
*	@struct		TableElement
*	@author		MFS corporation
*	@brief		TableElement 구조체
*****************************************************************/
struct TableElement
{
	int		nRow;
	int		nCol;
	BOOL	bAllowSelect;
	CRect	rcRect;
	CString	strText;
};

#define MAX_TABLE_ROW	6
#define MAX_TABLE_COL	41

#define DISABLE_TYPE	0
#define ENABLE_TYPE		1
#define SELECT_TYPE		2
#define MAX_TYPE		3

/** *************************************************************
*	@class		CNTableEditTextBox
*	@author		MFS corporation
*	@brief		Table 형식으로 된 Edit box 처리를 담당한다.
*****************************************************************/
class CNTableEditTextBox : public CNShape
{
public:
	CNTableEditTextBox(CTL_TYPE type=CTL_TABLEEDITTEXTBOX);
	~CNTableEditTextBox();	

public:
	void	SetAllowAmount(int nRow, int nColumn);
	void	SetGuideTextColor(CNColor *pNormal, CNColor *pSelect);
	void	SetGuideFont(CNFont *pNormal, CNFont *pSelect);
	void	SetGuideBrush(CNBrush *pNormal, CNBrush *pSelect);
	void	SetElementPen(CNPen *pDisable, CNPen *pEnable, CNPen *pSelect);
	void	SetElementFont(CNFont *pDisable, CNFont *pEnable, CNFont *pSelect);
	void	SetElementBrush(CNBrush *pDisable, CNBrush *pEnable, CNBrush *pSelect);
	void	SetElementTextColor(CNColor *pDisable, CNColor *pEnable, CNColor *pSelect);

	//*LEH
	void	ConvInputKeyToNewText(CString strOrgKey, CString &strNewCode, BOOL &bMoveCursor);	///< 입력한 Key를 입력모드에 해당하는 Text로 변환
	void	SetNewText(CString strKey, CString strNewKeyCode, BOOL bMoveCursor, BOOL &bInsertText, BOOL &bInputCharAfterBlank, BOOL bTabAfterBlank);	///< 변환된 NewText를 TableEditBox에 입력
	void	SaveCurrentText(CString strKey, BOOL &bInsertText, CStringArray &arrSaveText, BOOL bTextInputForBackspace = FALSE, BOOL bBackspace = FALSE);									///< 현재 위치의 문자부터 마지막 문자까지 임시 저장한다.
	void	InputSaveText(CStringArray &arrSaveText, int nCurrCol, CString strKey, BOOL bInsertText, BOOL &bInputCharAfterBlank, BOOL bBackspace = FALSE);	///< 문자열 중간에 insert/delete 후, 임시저장된 문자열을 insert/delete 다음 위치부터 채워준다.
	void	InitializeValue();		///< 문자 입력 화면에서 관련 변수 초기화
	void	SetCurrentTextLength();	///< 문자 입력 화면에서 현재 문자열 길이 산출
	///////////////////////////////
public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	MakeAllFSCMD(CString &Data);
	virtual BOOL	SetInputMode(CString &strData);

private:
	TableElement	m_TableElement[MAX_TABLE_ROW][MAX_TABLE_COL];
	TableElement	*m_pCurElement;
	
	int				m_nAllowRow;
	int				m_nAllowColumn;

	CNPen			*m_pGuideFont[2];
	CNColor			*m_pGuideTextColor[2];
	CNBrush			*m_pGuideBrush[2];

	CNPen			*m_pElementPen[MAX_TYPE];
	CNFont			*m_pElementFont[MAX_TYPE];
	CNBrush			*m_pElementBrush[MAX_TYPE];
	CNColor			*m_pElementTextColor[MAX_TYPE];

	//*LEH
	BOOL			m_bSameKey;
	BOOL			m_bKeyNotZero;
	BOOL			m_bCharInputComplete;
	BOOL			m_bInsertText;
	BOOL			m_bInputCharAfterBlank;
	CString			m_strPrevInputKey;
	BOOL			m_bTextInputForBackspace;		///< 현재 입력중인 문자열인지 판단하는 변수
	CString			m_strPrevKey;
	CString			m_strPrevNewChar;
	//////////////////////////////////////
};

/** *************************************************************
*	@class		CNButtonBox
*	@author		MFS corporation
*	@brief		NButtonBox 클래스
*****************************************************************/
class CNButtonBox : public CNTextBox
{
public:
	CNButtonBox(CTL_TYPE type=CTL_BUTTONBOX);
	~CNButtonBox();

public:
	//virtual BOOL	Handler(CRequest &Request);
	void	SetTextType(CString strType);
public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	MouseEvent(POINT &point);
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	FindActBtn(CString &strKey);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	MakeAllFSCMD(CString &Data);

protected:
	BOOL	m_bShow;
	BOOL	m_bAlwaysShow;
};

/** *************************************************************
*	@class		CNVKeyButtonBox
*	@author		MFS corporation
*	@brief		NVKeyButtonBox 클래스
*****************************************************************/
class CNVKeyButtonBox : public CNButtonBox
{
public:
	CNVKeyButtonBox(CTL_TYPE type=CTL_VKEYBUTTONBOX);
	~CNVKeyButtonBox();

public:
	//virtual BOOL	Handler(CRequest &Request);
public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	MouseEvent(POINT &point);
};

/** CHANGEPW TYPE */
typedef enum CHANGEPWTYPE
{
	CPTYPE_CUR=0,
	CPTYPE_NEW,
	CPTYPE_NEWRE,
	CPTYPE_MAX
};

/** *************************************************************
*	@class		CNChangePWBox
*	@author		MFS corporation
*	@brief		NChangePWBox 클래스
*****************************************************************/
class CNChangePWBox : public CNEditTextBox
{
public:
	CNChangePWBox(CTL_TYPE type=CTL_CHANGEPWBOX);
	~CNChangePWBox();

public:
	void	SetPWBoxRect(CHANGEPWTYPE type, int left, int top, int width, int height);

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	MakeAllFSCMD(CString &Data);

private:
	CHANGEPWTYPE	m_curInputBox;
	CNEditTextBox	m_ChangePW[CPTYPE_MAX];
};

/** *************************************************************
*	@class		CNSWKeyBox
*	@author		MFS corporation
*	@brief		NSWKeyBox 클래스
*****************************************************************/
class CNSWKeyBox : public CNTextBox
{
public:
	CNSWKeyBox(CTL_TYPE type=CTL_SWKEYBOX);
	~CNSWKeyBox();

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

/** *************************************************************
*	@struct		VKeyElement
*	@author		MFS corporation
*	@brief		VKeyElement 구조체
*****************************************************************/
struct VKeyElement
{
	int		nUseType;	// 0 - None, 1 - Guide, 2 - VKey
	int		nRow;
	int		nCol;
	CRect	rcRect;
	CString	strText;
};

#define USE_KEY_NONE		0
#define USE_KEY_GUIDE		1
#define USE_KEY_VKEY		2
#define	USE_KEY_UNDER_GUIDE	3

#define MAX_KEY_ROW			6
#define MAX_KEY_COL			19

/** *************************************************************
*	@class		CNVirtualKeyBox
*	@author		MFS corporation
*	@brief		NVirtualKeyBox 클래스
*****************************************************************/
class CNVirtualKeyBox : public CNShape
{
public:
	CNVirtualKeyBox(CTL_TYPE type);
	~CNVirtualKeyBox();

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);

public:
	void	SetGuideFont(CNFont *pNormal, CNFont *pSelect = NULL);
	void	SetGuideBrush(CNBrush *pNormal, CNBrush *pSelect = NULL);
	void	SetGuidePen(CNPen *pNormal, CNPen *pSelect = NULL);
	void	SetGuideTextColor(CNColor *pNormal, CNColor *pSelect = NULL);

	void	SetVKeyFont(CNFont *pNormal, CNFont *pSelect = NULL);
	void	SetVKeyBrush(CNBrush *pNormal, CNBrush *pSelect = NULL);
	void	SetVKeyPen(CNPen *pNormal, CNPen *pSelect = NULL);
	void	SetVKeyTextColor(CNColor *pNormal, CNColor *pSelect = NULL);

protected:
	VKeyElement		m_KeyTable[MAX_KEY_ROW][MAX_KEY_COL];
	VKeyElement		*m_pCurKey;

	BOOL			m_bShow;
	int				m_nAllowRow;
	int				m_nAllowCol;

	CNFont			*m_pGuideNormalFont, *m_pGuideFocusFont;
	CNBrush			*m_pGuideNormalBrush, *m_pGuideFocusBrush;
	CNPen			*m_pGuideNormalPen, *m_pGuideFocusPen;
	CNColor			*m_pGuideNormalTextColor, *m_pGuideFocusTextColor;

	CNFont			*m_pVKeyNormalFont,	*m_pVKeyFocusFont;
	CNBrush			*m_pVKeyNormalBrush, *m_pVKeyFocusBrush;
	CNPen			*m_pVKeyNormalPen, *m_pVKeyFocusPen;
	CNColor			*m_pVKeyNormalTextColor, *m_pVKeyFocusTextColor;
};

/** VKEY TYPE */
typedef enum VKEY_TYPE
{
	VKEY_ALPHA_UPPER = 0,
	VKEY_ALPHA_LOWER,
	VKEY_ALPHA_NUMBER,
};

/** *************************************************************
*	@class		CNAlphaKeyBox
*	@author		MFS corporation
*	@brief		NAlphaKeyBox 클래스
*****************************************************************/
class CNAlphaKeyBox : public CNVirtualKeyBox
{
public:
	CNAlphaKeyBox(CTL_TYPE type=CTL_ALPHAKEYBOX);
	~CNAlphaKeyBox();

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	KeyEvent(CString &strKey);

public:
	void	SetAlphaKeyType(VKEY_TYPE	type);
	void	ChangeInputMode();							///< 입력모드 변환

private:
	VKEY_TYPE	m_nAlphaKeyType;
};

/** *************************************************************
*	@class		CNTableKeyBox
*	@author		MFS corporation
*	@brief		NTableKeyBox 클래스
*****************************************************************/
class CNTableKeyBox : public CNVirtualKeyBox
{
public:
	CNTableKeyBox(CTL_TYPE type=CTL_TABLEKEYBOX);
	~CNTableKeyBox();

public:
	virtual BOOL	Initialize(void);
	virtual BOOL	KeyEvent(CString &strKey);
};

/** *************************************************************
*	@class		CNToggleBox
*	@author		MFS corporation
*	@brief		NToggleBox 클래스
*****************************************************************/
class CNToggleBox : public CNTextBox
{
public:
	CNToggleBox(CTL_TYPE type=CTL_TOGGLEBOX);
	~CNToggleBox();

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

////////////////////////////////////////////////
//	CNTreeStateBox
//
#define STATE_HIDE	0
#define STATE_ON	1
#define STATE_OFF	2

/** *************************************************************
*	@class		CNThreeStateBox
*	@author		MFS corporation
*	@brief		NThreeStateBox 클래스
*****************************************************************/
class CNThreeStateBox : public CNTextBox
{
public:
	CNThreeStateBox(CTL_TYPE type=CTL_THREESTATEBOX);
	~CNThreeStateBox();

public:
	void	SetStateImage(CNPicture *pHide, CNPicture *pOn, CNPicture *pOff);
	void	SetStatePen(CNPen *pHide, CNPen *pOn, CNPen *pOff);
	void	SetStateBrush(CNBrush *pHide, CNBrush *pOn, CNBrush *pOff);
	void	SetStateFont(CNFont *pHide, CNFont *pOn, CNFont *pOff);
	void	SetStateTextColor(CNColor *pHide, CNColor *pOn, CNColor *pOff);
	void	SetStateBlink(BOOL bHide, BOOL bOn, BOOL bOff);
public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetBlinking(void);

private:
	CNPicture		*m_pStateImage[3];
	CNPen			*m_pStatePen[3];
	CNBrush			*m_pStateBrush[3];
	CNFont			*m_pStateFont[3];
	CNColor			*m_pStateTextColor[3];
	BOOL			m_bStateBlink[3];

	int				m_nCurState;
};

/** *************************************************************
*	@class		CNEJNLBox
*	@author		MFS corporation
*	@brief		NEJNLBox 클래스
*****************************************************************/
class CNEJNLBox : public CNTextBox
{
public:
	CNEJNLBox(CTL_TYPE type=CTL_EJNLBOX);
	~CNEJNLBox();

public:
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
// Define in CNTextBox
protected:
	virtual CString GetDrawText();		// 현재 뿌려질 텍스트를 가져온다.
};

/** *************************************************************
*	@class		CNDateBox
*	@author		MFS corporation
*	@brief		NDateBox 클래스
*****************************************************************/
class CNDateBox : public CNTextBox
{
public:
	CNDateBox(CTL_TYPE type=CTL_DATEBOX);
	~CNDateBox();

private:
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	MakeAllFSCMD(CString &Data);

private:
	CString			m_strTemp;
};

/** *************************************************************
*	@class		CNIPBox
*	@author		MFS corporation
*	@brief		NIPBox 클래스
*****************************************************************/
class CNIPBox : public CNEditTextBox
{
public:
	CNIPBox(CTL_TYPE type=CTL_IPBOX);
	~CNIPBox();

private:
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);

private:
	CString			m_strTemp;
};


/** *************************************************************
*	@class		CNCentBox
*	@author		MFS corporation
*	@brief		NCentBox 클래스
*****************************************************************/
class CNCentBox : public CNEditTextBox
{
public:
	CNCentBox(CTL_TYPE type=CTL_CENTBOX);
	~CNCentBox();

	// Define in CNTextBox
protected:
	virtual CString GetDrawText();		// 현재 뿌려질 텍스트를 가져온다.

};

/** *************************************************************
*	@class		CNDollarBox
*	@author		MFS corporation
*	@brief		NDollarBox 클래스
*****************************************************************/
class CNDollarBox : public CNEditTextBox
{
public:
	CNDollarBox(CTL_TYPE type=CTL_DOLLARBOX);
	~CNDollarBox();

// Define in CNTextBox
protected:
	virtual CString GetDrawText();		// 현재 뿌려질 텍스트를 가져온다.
	virtual BOOL	KeyEvent(CString &strKey);		// [#810] US Justin... Clear => remove entire entired amount
};

/** BINTYPE */
typedef enum BINTYPE
{
	BNTYPE_INDEX=0,
	BNTYPE_BIN,
	BNTYPE_ADD,
	BNTYPE_DEL,
	BNTYPE_EDT,
	BNTYPE_MAX
};

/** *************************************************************
*	@class		CNBinBox
*	@author		MFS corporation
*	@brief		NBinBox 클래스
*****************************************************************/
class CNBinBox : public CNTextBox
{
public:
	CNBinBox(CTL_TYPE type=CTL_BINBOX);
	~CNBinBox();

public:
	void	SetBinBoxRect(BINTYPE type, int left, int top, int width, int height);

protected:
	virtual BOOL	Initialize(void);
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	BinAct(CString &strAct);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);

private:
	BINTYPE			m_curInput;
	BINTYPE			m_BinAct;
	CNEditTextBox	m_BinItem[BNTYPE_MAX];
};

#define MAX_ANI		10

/** *************************************************************
*	@class		CNAnimation
*	@author		MFS corporation
*	@brief		NAnimation 클래스
*****************************************************************/
class CNAnimation : public CNShape
{
public:
	CNAnimation(CTL_TYPE type=CTL_ANI);
	~CNAnimation();

public:
	BOOL SetAniImage(int Count, CString strPrefix);

protected:
	virtual BOOL	Initialize(void);
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
	
	CNHImage	m_Image;

};

/** *************************************************************
*	@class		CNAdvBox
*	@author		MFS corporation
*	@brief		NAdvBox 클래스
*****************************************************************/
class CNAdvBox : public CNTextBox
{
public:
	CNAdvBox(CTL_TYPE type=CTL_ADVBOX);
	~CNAdvBox();

public:
	void	SetFileName(CString strFileName);
	BOOL	DecodeImage();
	BOOL	IsDecode();

protected:
	virtual BOOL	Initialize(void);
	virtual BOOL	Deinitialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);

private:
	void	ImageRelease();

private:
	CString		m_strFileName;
	CNHImage	m_Image;
};

/** *************************************************************
*	@class		CNBinIndexBox
*	@author		MFS corporation
*	@brief		NBinIndexBox 클래스
*****************************************************************/
class CNBinIndexBox : public CNEditTextBox
{
public:
	CNBinIndexBox(CTL_TYPE type=CTL_BININDEXBOX);
	~CNBinIndexBox();

private:
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	BinAct(CString &strAct);

private:
	BINTYPE			m_BinAct;

};

/** *************************************************************
*	@class		CNDecimalBox
*	@author		MFS corporation
*	@brief		NDecimalBox 클래스
*****************************************************************/
class CNDecimalBox : public CNEditTextBox
{
public:
	CNDecimalBox(CTL_TYPE type=CTL_DECIMALBOX);
	~CNDecimalBox();

	// Define in CNTextBox
protected:
	virtual CString GetDrawText();		// 현재 뿌려질 텍스트를 가져온다.

};

/** *************************************************************
*	@class		CNCentClearBox
*	@author		MFS corporation
*	@brief		NCentClearBox 클래스
*****************************************************************/
class CNCentClearBox : public CNEditTextBox
{
public:
	CNCentClearBox(CTL_TYPE type=CTL_CENTBOX);
	~CNCentClearBox();

	// Define in CNTextBox
protected:
	virtual CString GetDrawText();		// 현재 뿌려질 텍스트를 가져온다.
	virtual BOOL	KeyEvent(CString &strKey);		// Clear => remove entire entired amount

};

/** ********************************************
*	@class	CNKeyTestBox
*	@date	2017.11.17
*	@author MFS 
*	@brief	PinPad의 Key 정보를 보여주기 위한 클래스
***********************************************/
class CNKeyTestBox : public CNEditTextBox
{
public:
	CNKeyTestBox(CTL_TYPE type=CTL_KEYTESTBOX);
	~CNKeyTestBox();

protected:
	virtual CString GetDrawText();					// 현재 보여질 텍스트를 가져온다.
	virtual BOOL	KeyEvent(CString &strKey);
};

/** *************************************************************
*	@class		CNInputEditBox
*	@author		MFS corporation
*	@brief		NInputEditBox 클래스
*****************************************************************/
class CNInputEditBox : public CNEditTextBox
{
public:
	CNInputEditBox(CTL_TYPE type=CTL_INPUTEDITBOX);
	~CNInputEditBox();

	// Define in CNTextBox
protected:
	virtual CString GetDrawText();
	virtual BOOL	SetInputType(CString &strData);

public:
	int m_nInputType;

};

/** *************************************************************
*	@class		CNThreeTypeBox
*	@author		MFS corporation
*	@brief		EditBox를 활성/비활성/선택 에 따라 다르게 보여주기 위한 클래스
*****************************************************************/
#define STATE_DISABLE	0
#define STATE_ENABLE	1
#define STATE_SELECT	2

class CNThreeTypeBox : public CNEditTextBox
{
public:
	CNThreeTypeBox(CTL_TYPE type=CTL_THREETYPEBOX);
	~CNThreeTypeBox();

public:
	void	SetStatePen(CNPen *pDisable, CNPen *pEnable, CNPen *pSelect);
	void	SetStateBrush(CNBrush *pDisable, CNBrush *pEnable, CNBrush *pSelect);
	void	SetStateFont(CNFont *pDisable, CNFont *pEnable, CNFont *pSelect);
	void	SetStateTextColor(CNColor *pDisable, CNColor *pEnable, CNColor *pSelect);


public:
	virtual BOOL	Initialize(void);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);

private:
	CNPen			*m_pStatePen[3];
	CNBrush			*m_pStateBrush[3];
	CNFont			*m_pStateFont[3];
	CNColor			*m_pStateTextColor[3];
	int				m_nCurState;

};


/** *************************************************************
*	@class		CNGifBox
*	@author		MFS corporation
*	@brief		NGifBox 클래스
*****************************************************************/
class CNGifBox : public CNShape
{
public:
	CNGifBox(CTL_TYPE type=CTL_GIFBOX);
	~CNGifBox();

public:
	BOOL SetAniImage(CString strFileName);

protected:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	SetBlinking(void);

private:
//	void	ImageRelease();

private:
	BOOL		m_bShow;
	int			m_nImageIndex;
	int			m_nLoadImage;
	int			m_nBlinkCount;
	
	CNHImage	m_Image;

};


/** *************************************************************
*	@class		CNImageBox
*	@author		MFS corporation
*	@brief		CNImageBox 클래스
*****************************************************************/
class CNImageBox : public CNTextBox
{
public:
	CNImageBox(CTL_TYPE type = CTL_IMAGEBOX);
	~CNImageBox();

protected:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	SetVariable(CString &strName, CString &strData);

private:
	CNHImage	m_Image;
	BOOL		m_bShow;
	BOOL		m_bImageShow;
};


/** *************************************************************
*	@class		CNEditButtonBox
*	@author		MFS corporation
*	@brief		NEditButtonBox 클래스
*****************************************************************/
class CNEditButtonBox : public CNTextBox
{
public:
	CNEditButtonBox(CTL_TYPE type=CTL_EDITBUTTONBOX);
	~CNEditButtonBox();

public:
	//virtual BOOL	Handler(CRequest &Request);
public:
	virtual BOOL	Initialize(void);
	virtual BOOL	DrawControl(HDC hDC, RECT &rcUpdate);
	virtual BOOL	MouseEvent(POINT &point);
	virtual BOOL	KeyEvent(CString &strKey);
	virtual BOOL	SetVariable(CString &strName, CString &strData);
	virtual BOOL	FindActBtn(CString &strKey);
	virtual BOOL	SetCtlFocus(CString &strName, CString &strData);
	virtual BOOL	MakeAllFSCMD(CString &Data);

protected:
	BOOL	m_bShow;
};

#endif __N_CONTROL_H__