#ifndef __NH_IMAGE_H__
#define __NH_IMAGE_H__

// Cross Compile
#ifdef UNDER_CE
#include <imaging.h>
#else
#include <Wingdi.h>
#include "ximage.h"
#endif

#include <afx.h>
#include "NHSyncObject.h"

#define MAX_SUPPORT_IMAGE_COUNT		10
//#define SCR_TRANSCOLOR				RGB(255, 0, 255)
#define SCR_TRANSCOLOR				RGB(154, 150, 53)

#define DECODE_FAIL	0
#define DECODE_INIT	1
#define DECODE_DONE	2

// 디코딩한 결과
#define RESULT_BMP  1	// JPG, BMP
#define RESULT_IMG  2	// PNG
#define RESULT_GIF	3	// GIF

// 이미지 정보
struct NHImageInfo
{
	int			nState;			// Deocde 상태 (init, done, fail)
	int			nResultType;	// Bitmap, Image 
	CString		FileName;
	RECT		srcRect;		// 0, 0, width, height
// Cross Compile
#ifdef UNDER_CE
	IImage*		pImage;
	IStream*	pStream;
#else
	CxImage*	pImage;
#endif
	HBITMAP		hBitmap;
};

class CNHImage
{
public:
	CNHImage();
	CNHImage(LPCTSTR pFileName[], int nCount, BOOL bDecodeNow);
	~CNHImage();

private:
	void	InitImage();
	BOOL	DecodeToHBitmap(int nIndex);
	BOOL	DecodeToIImage(int nIndex);
	BOOL	DecodeToGIF();

#ifndef UNDER_CE
	// Kept out of DecodeToGIF() because that function uses __try/__except and
	// MSVC forbids object unwinding inside an SEH frame (C2712). Creating a
	// CxImage with new needs unwinding, so it lives here instead.
	BOOL	DecodeGifWithCxImage();
#endif

	HRESULT CreateStreamOnFile(LPCTSTR pszFilename, IStream **ppStream);
//	BOOL	DecodeTest(int nIndex);

public:
	BOOL	SetDecodeInfo(LPCTSTR pFileName[], int nCount, BOOL bDecodeNow);
	BOOL	DecodeImage(int nIndex);
	BOOL	DecodeImage4Adv(int nIndex);	// V1.0.2.5 2018.08.06 - Adv를 위한 decode
	BOOL	DecodeImage4GIF();	// V1.0.2.4 2018.06.26 - GIF 추가
	void	ReleaseImage();
	BOOL	Draw(HDC hDC, int nImgIndex, RECT *pdstRect);
	int		GetImageCount();

private:
	static	CNHCritSec	m_hCriticalSync;
	static	int			m_nRefCnt;				// 객체 참조 횟수
	static	HBITMAP		m_hbmScrBuf;			// 전체 화면 Bitmap
	static  HBRUSH		m_hbrTrans;				// Transparent color for bitmap
	static	SIZE		m_siScreen;				// 전체 화면 크기
	
	NHImageInfo	m_Images[MAX_SUPPORT_IMAGE_COUNT];
	int			m_nImageCount;

};

#endif // __NH_IMAGE_H__