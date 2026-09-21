#ifndef __N_RESOURCE_H__
#define __N_RESOURCE_H__

#ifdef UNDER_CE
#include <imaging.h>
#endif

#include "SCR_Image.h"

typedef enum RES_TYPE
{
	RES_NONE=0,
	RES_PEN,
	RES_FONT,
	RES_BRUSH,
	RES_PICTURE,
	RES_LOCALETEXT,
	RES_COLOR,
	RES_MAX
};

///////////////////////////////////////////////////////////
//	CNResource
//
class CNResource
{
public:
	CNResource();
	CNResource(RES_TYPE type, CString strID, HGDIOBJ hHandle=NULL);
	~CNResource();

public:
	RES_TYPE	GetTpye();
	CString		GetID();
	HGDIOBJ		GetHandle();
	
protected:
	RES_TYPE	m_eResType;
	CString		m_strID;
	HGDIOBJ		m_hGDIObj;
};

typedef CNResource	CNPen;
typedef CNResource	CNFont;
typedef CNResource	CNBrush;

///////////////////////////////////////////////////////////
//	CNPicture
//
typedef enum IMG_TYPE
{
	NH_IMG_UNKNOWN = 0,
	NH_IMG_BMP,
	NH_IMG_JPG,
	NH_IMG_PNG,
	NH_IMG_MAX
};

class CNPicture : public CNResource
{
public:
	CNPicture(CString strID, CString strFileName);
//	CNPicture(CString strID, IMG_TYPE ImgType, IImage *pImage);
//	CNPicture(CString strID, IMG_TYPE ImgType, HBITMAP hImage, int width, int height);
	~CNPicture();

public:
//	IImage*		GetHandle();
//	void		SetImage(IMG_TYPE ImgType, IImage *pImage);
//	IMG_TYPE	GetImageType();

//	HBITMAP		GetBitmapHandle();
	BOOL		IsAnimationImage();
	BOOL		DrawImage(HDC hDC, int left, int top, int right, int bottom);
	CString		GetImageFileName();	// 이미지의 파일명을 가져오기 위해 필요함.(gif)
	
private:
//	IImage		*m_pImage;
//	IMG_TYPE	m_nImgType;
	CNHImage	m_Image;
	int			m_nFrameCount;
	int			m_curFrame;
	BOOL		m_bAnimationImage;

	CString		m_strFileName;
	HBITMAP		m_hPicture;
	SIZE		m_ImageSize;
};

///////////////////////////////////////////////////////////
//	CNLocaleText
//
class CNLocaleText : public CNResource
{
public:
	CNLocaleText(CString strID, int nSupportLocale);
	~CNLocaleText();

public:
	BOOL	AddLocaleText(int Locale, LPCTSTR pText);
	CString GetLocaleText(int Locale);

private:
	int			m_nLocale;
	CString		*m_pLocaleText;
};

///////////////////////////////////////////////////////////
//	CNColor
//
class CNColor : public CNResource
{
public:
	CNColor(CString strID, int nRed, int nBlue, int nGreen);
	~CNColor();

public:
	COLORREF	m_crColor;
};


#endif __N_RESOURCE_H__