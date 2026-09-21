#ifndef __PICASSO_IMAGE_H__
#define __PICASSO_IMAGE_H__

// FOR WINCE60 Define
#include ".\Common\WinCE60Def.h"

// Cross Compile
#ifdef UNDER_CE
#include <imaging.h>
#else
#include <Wingdi.h>
#include "ximage.h"
#endif

#include <afx.h>
#include ".\DLL\NHSyncObject.h"
#include "..\..\H\dll\AssetArchive.h"

#define MAX_SUPPORT_IMAGE_COUNT		10
#define SCR_TRANSCOLOR				RGB(255, 0, 255)

#define DECODE_FAIL	0
#define DECODE_INIT	1
#define DECODE_DONE	2

// 디코딩한 결과
#define RESULT_BMP  1	// BMP
#define RESULT_IMG  2	// PNG
#define RESULT_JPG	3	// JPG	// [#2416] NH KSK 2016.04.27

typedef enum DECODE_TYPE
{
	DT_NORMAL				= 0,
	DT_TRANSBITMAP			= 1
};

// 이미지 정보
struct PicassoImageInfo
{
	int			nState;			// Deocde 상태 (init, done, fail)
	int			nResultType;	// Bitmap, Image 
	CString		FileName;
	RECT		srcRect;		// 0, 0, width, height
// Cross Compile
#ifdef UNDER_CE
	IImage*		pImage;
#else
	CxImage*	pImage;
#endif
	HBITMAP		hBitmap;

	HGLOBAL		hgMemObj;		// to create a buffer for IIStream (for GloblAlloc())
	LPVOID		lpMemObj;		// to get a pointer of buffer above (for GlobalLock())
};

class CPicassoImage
{
public:
	CPicassoImage();
	CPicassoImage(LPCTSTR pFileName[], int nCount, BOOL bDecodeNow);
	~CPicassoImage();

private:
	void	InitImage();
	BOOL	DecodeToHBitmap(int nIndex);			// deprecated
	BOOL	DecodeToIImage(int nIndex);				// deprecated
	BOOL	DecodeAs(DECODE_TYPE type, int index);

	BOOL	GetFileSizeFromFileSystem(LPCTSTR pszFilename, unsigned int &size);
	HRESULT CreateStreamFromFileSystem(LPCTSTR pszFilename, IStream **ppStream);

public:
	BOOL	SetDecodeInfo(LPCTSTR pFileName[], int nCount, BOOL bDecodeNow);
	BOOL	DecodeImage(int nIndex);
	void	ReleaseImage(int index);
	BOOL	Draw(HDC hDC, int nImgIndex, RECT *pdstRect);
	int		GetImageCount();
	int		GetImageState(int nIndex);

private:
	static	CNHCritSec	m_hCriticalSync;
	static	int			m_nRefCnt;				// 객체 참조 횟수
	static	HBITMAP		m_hbmScrBuf;			// 전체 화면 Bitmap
	static  HBRUSH		m_hbrTrans;				// Transparent color for bitmap
	static	SIZE		m_siScreen;				// 전체 화면 크기

	PicassoImageInfo	m_Images[MAX_SUPPORT_IMAGE_COUNT];
	int			m_nImageCount;
};

HBITMAP MakeDIBSection(HDC hdc, int width, int height);

#endif // __PICASSO_IMAGE_H__