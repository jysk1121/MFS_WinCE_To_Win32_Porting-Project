#include "stdafx.h"
#include "PicassoResource.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		0
#define DBG_INFO		0


//------------------------------------------------------------------
//	CPicassoResource Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoResource
 FUNCTION NAME: CPicassoResource()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 생성자
-------------------------------------------------------------------*/
CPicassoResource::CPicassoResource()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_eResType = RES_NONE;
	m_strID = _T("");
	m_hGDIObj = NULL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoResource
 FUNCTION NAME: CPicassoResource()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    :    type : Resource 종류
                  strID : Resource ID
				hHandle : Resource의 Window Handle
 DESCRIPTION  : 생성자
-------------------------------------------------------------------*/
CPicassoResource::CPicassoResource(RES_TYPE type, CString strID, HGDIOBJ hHandle)
{
	NHUIDBG(DBG_CALL, (_T("Type(%d), ID(%s), Handle(0x%08X)\n"), type, strID, hHandle));

	m_eResType = type;
	m_strID = strID;
	m_hGDIObj = hHandle;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoResource
 FUNCTION NAME: ~CPicassoResource()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 소멸자
-------------------------------------------------------------------*/
CPicassoResource::~CPicassoResource()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_hGDIObj)
		DeleteObject(m_hGDIObj);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoResource
 FUNCTION NAME: GetID()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 리소스 아이디
 PARAMETER    : 
 DESCRIPTION  : 리소스의 아이디를 조회한다.
-------------------------------------------------------------------*/
CString	CPicassoResource::GetID()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return m_strID;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoResource
 FUNCTION NAME: GetTpye()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : Resource Type
 PARAMETER    : 
 DESCRIPTION  : 리소스의 타입을 조회한다.
-------------------------------------------------------------------*/
RES_TYPE CPicassoResource::GetTpye()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return m_eResType;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoResource
 FUNCTION NAME: GetHandle()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : Resource's Windows Handle
 PARAMETER    : 
 DESCRIPTION  : 리소스의 핸들을 조회한다.
-------------------------------------------------------------------*/
HGDIOBJ CPicassoResource::GetHandle()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return m_hGDIObj;
}


//------------------------------------------------------------------
//	CPicassoPicture Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoPicture
 FUNCTION NAME: CPicassoPicture()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    :       strID : Picture의 ID
                strFileName : Picture의 FileName
 DESCRIPTION  : 생성자
-------------------------------------------------------------------*/
CPicassoPicture::CPicassoPicture(CString strID)
{
	NHUIDBG(DBG_CALL, (_T("ID(%s)\n"), strID));

	m_eResType = RES_PICTURE;
	m_strID = strID;

	for (int i = 0; i < MAX_LOCALE; i++)
	{
		m_sImages[i].bAdded = FALSE;
		m_sImages[i].bDecoded = FALSE;
		m_sImages[i].nFrameCount = 0;
		m_sImages[i].nCurFrame = 0;
		m_sImages[i].bAnimationImage = FALSE;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoPicture
 FUNCTION NAME: ~CPicassoPicture()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 소멸자
-------------------------------------------------------------------*/
CPicassoPicture::~CPicassoPicture()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoPicture
 FUNCTION NAME: IsAnimationImage()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  :  ture : support animation
                false : don't support animation
 PARAMETER    : 
 DESCRIPTION  : 이미지가 animation을 지원하는지 조회한다.
-------------------------------------------------------------------*/
BOOL CPicassoPicture::IsAnimationImage(int Locale)
{
	// [#2186] US KMK 2013.04.11 Asian Language Support
	// 리스트 마지막 언어 선택 시 이미지 누락되는 현상 수정
	// Locale은 1(ENG_MODE)부터 6(JPN_MODE)까지 있음
// 	if (Locale <= 0 || Locale >= MAX_LOCALE)
// 		return FALSE;
	if (Locale <= 0 || Locale > MAX_LOCALE)
		return FALSE;
	// end of [#2186]

	return m_sImages[Locale-1].bAnimationImage;
}

BOOL CPicassoPicture::AddImage(int Locale, CString strFileName)
{
	// [#2186] US KMK 2013.04.11 Asian Language Support
	// 리스트 마지막 언어 선택 시 이미지 누락되는 현상 수정
	// Locale은 1(ENG_MODE)부터 6(JPN_MODE)까지 있음
// 	if (Locale <= 0 || Locale >= MAX_LOCALE)
// 		return FALSE;
	if (Locale <= 0 || Locale > MAX_LOCALE)
		return FALSE;
	// end of [#2186]

	LPCTSTR	filename[1];
	filename[0] = (LPCTSTR)strFileName;

	m_sImages[Locale-1].bAdded = TRUE;
//	m_sImages[Locale-1].strFileName = strFileName;
	m_sImages[Locale-1].Image.SetDecodeInfo(filename, 1, FALSE);

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoPicture
 FUNCTION NAME: DrawImage()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  :  true : Image is draw.
                false : Image isn't draw.
 PARAMETER    :    hDC : Device Context Handle
                  left : position of the left side of the window
				   top : position of the top of the window
				 right : position of the right side of the widnow
				bottom : position of the bottom of the window
 DESCRIPTION  : 주어진 DC에 이미지를 주어진 크기로 Draw한다.
-------------------------------------------------------------------*/
BOOL CPicassoPicture::DrawImage(int Locale, HDC hDC, int left, int top, int right, int bottom)
{
	// [#2186] US KMK 2013.04.11 Asian Language Support
	// 리스트 마지막 언어 선택 시 이미지 누락되는 현상 수정
	// Locale은 1(ENG_MODE)부터 6(JPN_MODE)까지 있음
// 	if (Locale <= 0 || Locale >= MAX_LOCALE)
// 		return FALSE;
	if (Locale <= 0 || Locale > MAX_LOCALE)
		return FALSE;
	// end of [#2186]

	if (m_sImages[Locale-1].bAdded == FALSE)
	{
		if (m_sImages[0].bAdded == FALSE)
			return FALSE;

		Locale = 1;
	}

	RECT rcRect = {left, top, right, bottom};
	
	m_sImages[Locale-1].Image.Draw(hDC, m_sImages[Locale-1].nCurFrame, &rcRect);
	if (m_sImages[Locale-1].bDecoded == FALSE)
	{

		m_sImages[Locale-1].bDecoded = TRUE;
		m_sImages[Locale-1].nFrameCount = m_sImages[Locale-1].Image.GetImageCount();

		if (m_sImages[Locale-1].nFrameCount > 1)
			m_sImages[Locale-1].bAnimationImage = TRUE;
	}

	if (m_sImages[Locale-1].bAnimationImage == TRUE)
	{
		m_sImages[Locale-1].nCurFrame++;
		if (m_sImages[Locale-1].nCurFrame >= m_sImages[Locale-1].nFrameCount)
			m_sImages[Locale-1].nCurFrame = 0;
	}

	return TRUE;
}


//------------------------------------------------------------------
//	CPicassoLocaleText Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoLocaleText
 FUNCTION NAME: CPicassoLocaleText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    :          strID : string ID
				nSupportLocale : string ID가 지원하는 Locale 수
 DESCRIPTION  : 생성자
-------------------------------------------------------------------*/
CPicassoLocaleText::CPicassoLocaleText(CString strID, int nSupportLocale)
{
	NHUIDBG(DBG_CALL, (_T("ID(%s), Support Locale Count(%d)\n"), strID, nSupportLocale));

	m_eResType = RES_LOCALETEXT;
	m_strID = strID;
	m_nLocale = nSupportLocale;
	m_pLocaleText = NULL;
	
	if (m_nLocale > 0)
		m_pLocaleText = (CString*) new CString[m_nLocale];
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoLocaleText
 FUNCTION NAME: ~CPicassoLocaleText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 소멸자
-------------------------------------------------------------------*/
CPicassoLocaleText::~CPicassoLocaleText()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_pLocaleText)
		delete [] m_pLocaleText;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoPicture
 FUNCTION NAME: AddLocaleText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  :  true : successful
                false : failed to add new text
 PARAMETER    : 
 DESCRIPTION  : Locale에 해당하는 Text를 추가한다.
-------------------------------------------------------------------*/
BOOL CPicassoLocaleText::AddLocaleText(int Locale, LPCTSTR pText)
{
	NHUIDBG(DBG_CALL, (_T("Locale(%d), Text(%s)\n"), Locale, pText));

	if (Locale <= 0 || Locale > m_nLocale)
		return FALSE;

	m_pLocaleText[Locale-1] = pText;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoPicture
 FUNCTION NAME: AddLocaleText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : CString : the text of matching locale
 PARAMETER    : Locale : 조회하고자 하는 Locale
 DESCRIPTION  : Locale에 해당하는 Text를 가져온다.
-------------------------------------------------------------------*/
CString CPicassoLocaleText::GetLocaleText(int Locale)
{
	NHUIDBG(DBG_CALL, (_T("Locale(%d)\n"), Locale));

	CString strTemp;

	// check index
	if (Locale <= 0 || Locale > m_nLocale)
	{
		if (m_nLocale >= 0)
			return (m_pLocaleText[0]);
		else
			return _T("");
	}

	strTemp = m_pLocaleText[Locale-1];
	if (strTemp.IsEmpty() == TRUE)
	{
		if (m_nLocale >= 0)
			strTemp = m_pLocaleText[0];
	}

	return strTemp;
}


//------------------------------------------------------------------
//	CPicassoColor Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoColor
 FUNCTION NAME: CPicassoColor()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : constructor
-------------------------------------------------------------------*/
CPicassoColor::CPicassoColor(CString strID, int nRed, int nBlue, int nGreen)
{
	NHUIDBG(DBG_CALL, (_T("ID(%s), RGB(%d, %d, %d)\n"), strID, nRed, nBlue, nGreen));

	m_eResType = RES_COLOR;
	m_strID = strID;
	m_crColor = RGB(nRed, nBlue, nGreen);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoColor
 FUNCTION NAME: ~CPicassoColor()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : destructor
-------------------------------------------------------------------*/
CPicassoColor::~CPicassoColor()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

// [#2529] NH Justin 2018.02.22 Add 2800 SE
CPicassoPosition::CPicassoPosition(CString strID, int nXX, int nYY, int nWidth, int nHeight)
{
	NHUIDBG(DBG_CALL, (_T("CPicassoPosition, ID(%s), pos(%d, %d, %d, %d)\n"), strID, nXX, nYY, nWidth, nHeight));

	m_eResType = RES_POSITION;
	m_strID = strID;
	m_nXX = nXX;
	m_nYY = nYY;
	m_nWid = nWidth;
	m_nHgt = nHeight;
}

CPicassoPosition::~CPicassoPosition()
{
	NHUIDBG(DBG_CALL, (_T("~CPicassoPosition\n")));
}
// End of [#2529]

// [#RWC6-399] US ryan.payton 2022.09.15 DigitalMint
CPicassoTextSet::~CPicassoTextSet()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

CPicassoTextSet::CPicassoTextSet(CString strID, int nMulti, CString strAlignment, CString strColor)
{
	NHUIDBG(DBG_CALL, (_T("CPicassoTextSet, ID(%s), TextSet(%d, %s, %s)\n"), strID, nMulti, strAlignment, strColor));

	m_eResType = RES_TEXTSET;
	m_strID = strID;
	m_nMulti = nMulti;
	m_strAlignment = strAlignment;
	m_strColor = strColor;
}
// End of [#RWC6-399]