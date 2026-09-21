#include "stdafx.h"
#include "SCR_Resource.h"

///////////////////////////////////////////////////////////
//	CNResource
//
CNResource::CNResource()
{
	m_eResType = RES_NONE;
	m_strID = _T("");
	m_hGDIObj = NULL;
}

CNResource::CNResource(RES_TYPE type, CString strID, HGDIOBJ hHandle)
{
	m_eResType = type;
	m_strID = strID;
	m_hGDIObj = hHandle;
}

CNResource::~CNResource()
{
	if (m_hGDIObj)
		DeleteObject(m_hGDIObj);
}

CString	CNResource::GetID()
{
	return m_strID;
}

RES_TYPE CNResource::GetTpye()
{
	return m_eResType;
}

HGDIOBJ CNResource::GetHandle()
{
	return m_hGDIObj;
}

///////////////////////////////////////////////////////////
//	CNPicture
//

CNPicture::CNPicture(CString strID, CString strFileName)
{
	m_eResType = RES_PICTURE;
	m_strID = strID;
	m_strFileName = strFileName;
	m_ImageSize.cx = 0;
	m_ImageSize.cy = 0;
	m_curFrame = 0;
	m_bAnimationImage = FALSE;

	LPCTSTR	filename[1];
	filename[0] = (LPCTSTR)strFileName;

	m_Image.SetDecodeInfo(filename, 1, TRUE);
	m_nFrameCount = m_Image.GetImageCount();

	if (m_nFrameCount > 1)
	{
		m_bAnimationImage = TRUE;
	}
}

BOOL CNPicture::IsAnimationImage()
{
	return m_bAnimationImage;
}

CNPicture::~CNPicture()
{
}

BOOL CNPicture::DrawImage(HDC hDC, int left, int top, int right, int bottom)
{
	RECT rcRect = {left, top, right, bottom};
	
	m_Image.Draw(hDC, m_curFrame, &rcRect);

	if (m_bAnimationImage == TRUE)
	{
		m_curFrame++;
		if (m_curFrame >= m_nFrameCount)
			m_curFrame = 0;
	}

	return TRUE;
}

// V1.0.2.4 2018.07.16 - 이미지의 파일명을 가져오기 위해 필요함.(gif)
CString CNPicture::GetImageFileName()
{
	return m_strFileName;
}

///////////////////////////////////////////////////////////
//	CNLocaleText
//

CNLocaleText::CNLocaleText(CString strID, int nSupportLocale)
{
	m_eResType = RES_LOCALETEXT;
	m_strID = strID;
	m_nLocale = nSupportLocale;
	m_pLocaleText = NULL;
	
	if (m_nLocale > 0)
		m_pLocaleText = (CString*) new CString[m_nLocale];
}

CNLocaleText::~CNLocaleText()
{
	if (m_pLocaleText)
		delete [] m_pLocaleText;
}

BOOL	CNLocaleText::AddLocaleText(int Locale, LPCTSTR pText)
{
	if (Locale <= 0 || Locale > m_nLocale)
		return FALSE;

	m_pLocaleText[Locale-1] = pText;

	return TRUE;
}

CString CNLocaleText::GetLocaleText(int Locale)
{
	// check index
	if (Locale <= 0 || Locale > m_nLocale)
	{
		if (m_nLocale >= 0)
			return (m_pLocaleText[0]);
		else
			return _T("");
	}

	return (m_pLocaleText[Locale-1]);
}

///////////////////////////////////////////////////////////
//	CNColor
//
CNColor::CNColor(CString strID, int nRed, int nBlue, int nGreen)
{
	m_eResType = RES_COLOR;
	m_strID = strID;
	m_crColor = RGB(nRed, nBlue, nGreen);
}

CNColor::~CNColor()
{
}