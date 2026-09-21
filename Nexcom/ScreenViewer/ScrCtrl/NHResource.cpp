#include "stdafx.h"
#include "NHResource.h"

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
//	m_hPicture = NULL;
//	m_pImage = NULL;

	LPCTSTR	filename[1];
	filename[0] = (LPCTSTR)strFileName;

	m_Image.SetDecodeInfo(filename, 1, TRUE);
	m_nFrameCount = m_Image.GetImageCount();

	if (m_nFrameCount > 1)
	{
		m_bAnimationImage = TRUE;
	}

#if 0
	// decoding Image
	CNHImageDecoder	ImageDecoder;

	HDC hDC = GetDC(NULL);
	RETAILMSG(1, (L"DeviceCaps : BitsPixel(%d), nColors(%d)\n", GetDeviceCaps(hDC, BITSPIXEL), GetDeviceCaps(hDC, COLORRES)));	
	ReleaseDC(NULL, hDC);

	CString strType = m_strFileName.Right(3);
	strType.MakeUpper();

	if (strType == _T("PNG"))
	{
		ImageDecoder.Decode(m_strFileName, &m_pImage, &m_ImageSize);
	}
	else
	{
		ImageDecoder.Decode(m_strFileName, &m_pImage);
		//m_hPicture = ImageDecoder.Decode(strFileName, &m_ImageSize);
		
	}
#endif
}

BOOL CNPicture::IsAnimationImage()
{
	return m_bAnimationImage;
}

#if 0
CNPicture::CNPicture(CString strID, IMG_TYPE ImgType, IImage *pImage)
{
	m_eResType = RES_PICTURE;
	m_strID = strID;
	m_pImage = pImage;
	m_nImgType = ImgType;

	m_hPicture = NULL;
}

CNPicture::CNPicture(CString strID, IMG_TYPE ImgType, HBITMAP hImage,int width, int height)
{
	m_eResType = RES_PICTURE;
	m_strID = strID;
	m_pImage = NULL;
	m_nImgType = ImgType;

	m_hPicture = hImage;
}
#endif

CNPicture::~CNPicture()
{
#if 0
	if (m_hPicture != NULL)
	{
		DeleteObject(m_hPicture);
		m_hPicture = NULL;
	}

	if (m_pImage)
		m_pImage->Release();
#endif
}

BOOL CNPicture::DrawImage(HDC hDC, int left, int top, int right, int bottom)
{
	RECT rcRect = {left, top, right, bottom};
	
	//m_Image.Draw(hDC, m_curFrame, CRect(left, top, right, bottom));
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