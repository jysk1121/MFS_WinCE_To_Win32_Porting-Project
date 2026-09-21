#ifndef __PICASSO_RESOURCE_H__
#define __PICASSO_RESOURCE_H__

#ifdef UNDER_CE
#include <imaging.h>
#endif

#include "PicassoImage.h"

typedef enum RES_TYPE
{
	RES_NONE=0,
	RES_PEN,
	RES_FONT,
	RES_BRUSH,
	RES_PICTURE,
	RES_LOCALETEXT,
	RES_COLOR,
	RES_POSITION,	// [#2529] NH Justin 2018.02.22 Add 2800 SE
	RES_TEXTSET,	// [#RWC6-399] US ryan.payton 2022.09.15 DigitalMint
	RES_MAX
};

///////////////////////////////////////////////////////////
//	CPicassoResource
//
class CPicassoResource
{
public:
	CPicassoResource();
	CPicassoResource(RES_TYPE type, CString strID, HGDIOBJ hHandle=NULL);
	~CPicassoResource();

public:
	RES_TYPE	GetTpye();
	CString		GetID();
	HGDIOBJ		GetHandle();
	
protected:
	RES_TYPE	m_eResType;
	CString		m_strID;
	HGDIOBJ		m_hGDIObj;
};

typedef CPicassoResource	CPicassoPen;
typedef CPicassoResource	CPicassoFont;
typedef CPicassoResource	CPicassoBrush;

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

struct PicassoPictureInfo
{
	BOOL			bAdded;
	BOOL			bDecoded;
	CPicassoImage	Image;
	int				nFrameCount;
	int				nCurFrame;
	BOOL			bAnimationImage;
//	CString			strFileName;
};

#define MAX_LOCALE	6

class CPicassoPicture : public CPicassoResource
{
public:
	CPicassoPicture(CString strID);
	~CPicassoPicture();

public:
	BOOL		AddImage(int Locale, CString strFileName);
	BOOL		IsAnimationImage(int Locale);
	BOOL		DrawImage(int Locale, HDC hDC, int left, int top, int right, int bottom);
	
private:
	PicassoPictureInfo	m_sImages[MAX_LOCALE];
	int			m_nLocale;

//	CPicassoImage	m_Image;
//	int			m_nFrameCount;
//	int			m_curFrame;
//	BOOL		m_bAnimationImage;

//	CString		m_strFileName;
//	HBITMAP		m_hPicture;
//	SIZE		m_ImageSize;
};

///////////////////////////////////////////////////////////
//	CNLocaleText
//
class CPicassoLocaleText : public CPicassoResource
{
public:
	CPicassoLocaleText(CString strID, int nSupportLocale);
	~CPicassoLocaleText();

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
class CPicassoColor : public CPicassoResource
{
public:
	CPicassoColor(CString strID, int nRed, int nBlue, int nGreen);
	~CPicassoColor();

public:
	COLORREF	m_crColor;
};

// [#2529] NH Justin 2018.02.22 Add 2800 SE
class CPicassoPosition : public CPicassoResource
{
public:
	CPicassoPosition(CString strID, int nXX, int nYY, int nWidth, int nHeight);
	~CPicassoPosition();

public:
	int m_nXX, m_nYY, m_nWid, m_nHgt;
};
// End of [#2529]

// [#RWC6-399] US ryan.payton 2022.09.15 DigitalMint
class CPicassoTextSet : public CPicassoResource
{
public:
	CPicassoTextSet(CString strID, int nMulti, CString strAlignment, CString strColor);
	~CPicassoTextSet();

public:
	int m_nMulti;
	CString m_strAlignment, m_strColor;
};
// End of [#RWC6-399]

#endif __PICASSO_RESOURCE_H__