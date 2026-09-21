#include "stdafx.h" // [#] NH KJW 2011.06.20 ScrCtl Win32/CHA_SDK build fix
#include ".\Common\WinCE60Def.h"

#ifdef UNDER_CE
#include <atlbase.h>
#define INITGUID
#include <windows.h>
#include <initguid.h>
#include "PicassoImage.h"
#else
#include "ImageStone.h"
#include "PicassoImage.h"
#endif

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1

//------------------------------------------------------------------
//	Static Initialization
//------------------------------------------------------------------

extern CAssetArchive* g_pTarScreenAsset;			// global variable used in ScrCtrl

//------------------------------------------------------------------
//	Implementation
//------------------------------------------------------------------

CNHCritSec	CPicassoImage::m_hCriticalSync;
int			CPicassoImage::m_nRefCnt = 0;			// 객체 참조 횟수
HBITMAP		CPicassoImage::m_hbmScrBuf = NULL;		// 전체 화면 Bitmap		// 2022.12 deprecated
HBRUSH		CPicassoImage::m_hbrTrans = NULL;		// transparent color for bitmap
SIZE		CPicassoImage::m_siScreen = {0, 0};		// 전체 화면 크기

CNHEvent	g_eWaitObject;

// [#2518] US Kook 2018.01.10 Workaround for camera issue with ::CreateCompatibleBitmap().
HBITMAP MakeDIBSection(HDC hdc, int width, int height)   
{
#ifdef UNDER_CE
	// Since WINCE70 OS has enough video memory, we can use CreateCompatibleBitmap() for faster drawing
	if (GetConfigFuncPointer()->GetCEVersion() == WINCE_7)
		return ::CreateCompatibleBitmap(hdc, width, height);
#endif

	BITMAPINFO  bmi;   
	LPVOID      pBits;   
	HBITMAP     hBitmap;   

	memset(&bmi.bmiHeader, 0, sizeof(BITMAPINFOHEADER));   

	bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);   
	bmi.bmiHeader.biBitCount    = 24;          // 칼라수  : 1, 4, 8, 16, 24, 31   
	bmi.bmiHeader.biWidth       = width;       //비트맵 너비   
	bmi.bmiHeader.biHeight      = height;      //비트매 높이   
	bmi.bmiHeader.biPlanes      = 1;   
	hBitmap = ::CreateDIBSection( hdc, &bmi, DIB_RGB_COLORS, (void **)&pBits, NULL, 0);   

	return hBitmap;   
}
// end of [#2518]

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImage
 FUNCTION NAME: CPicassoImage()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 디폴트 생성자
-------------------------------------------------------------------*/
CPicassoImage::CPicassoImage()
{
	{
		CNHAutoLock AutoSync(m_hCriticalSync);

		if (m_hbrTrans == NULL)
		{
			m_hbrTrans = CreateSolidBrush(SCR_TRANSCOLOR);
		}

		m_nRefCnt++;
	}

	InitImage();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImage
 FUNCTION NAME: CPicassoImage(LPCTSTR pFileName[], int nCount, BOOL bDecodeNow)
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : pFileName : 디코딩할 파일 이름의 배열
				nCount : pFileName의 개수 (최대 10개)
				bDecodeNow : 생성시 디코딩 수행 옵션
							 만약 FALSE이면 Draw시 Decoding이 수행된다.
 DESCRIPTION  : 1. 디코딩할 파일의 정보를 받는다.
                2. 생성시 디코딩을 수행한다.(옵션)
-------------------------------------------------------------------*/
CPicassoImage::CPicassoImage(LPCTSTR pFileName[], int nCount, BOOL bDecodeNow)
{
	SetDecodeInfo(pFileName, nCount, bDecodeNow);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImage
 FUNCTION NAME: ~CPicassoImage()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자
-------------------------------------------------------------------*/
CPicassoImage::~CPicassoImage()
{
	for (int i = 0; i < MAX_SUPPORT_IMAGE_COUNT; i++)
	{
		ReleaseImage(i);
	}

	{
		CNHAutoLock AutoSync(m_hCriticalSync);

		m_nRefCnt--;

		if (m_nRefCnt <= 0)
		{ 
			if (m_hbrTrans != NULL)
			{
				::DeleteObject(m_hbrTrans);
				m_hbrTrans = NULL;
			}
		}
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImage
 FUNCTION NAME: SetDecodeInfo(LPCTSTR pFileName[], int nCount, BOOL bDecodeNow)
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : TRUE : SUCCESSFUL, FALSE : FAIL
 PARAMETER    : pFileName : 디코딩할 파일 이름의 배열
				nCount : pFileName의 개수 (최대 10개)
				bDecodeNow : 생성시 디코딩 수행 옵션
							 만약 FALSE이면 Draw시 Decoding이 수행된다.
 DESCRIPTION  : 1. 디코딩할 파일의 정보를 받는다.
                2. 함수 호출시 디코딩을 수행한다.(옵션)
				- 이전에 디코딩한 파일들이 있다면 모두 해제 된다.
-------------------------------------------------------------------*/
BOOL CPicassoImage::SetDecodeInfo(LPCTSTR pFileName[], int nCount, BOOL bDecodeNow)
{
	CNHAutoLock AutoSync(m_hCriticalSync);

//	ReleaseImage();
	InitImage();

	m_nImageCount = nCount;

	for (int i = 0; i < nCount; i++)
	{
		if (i >= MAX_SUPPORT_IMAGE_COUNT)
			break;

		m_Images[i].FileName = pFileName[i];

		if (bDecodeNow == TRUE)
			DecodeImage(i);
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImage
 FUNCTION NAME: Draw(HDC hDC, int nImgIndex, RECT *pdestRect)
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : hDC : 장치 Device Context
				nImgIndex : 그려질 이미지 index (0 base) 
				pdestRect : 이미지를 DC에 표시할 영역
 DESCRIPTION  : 제공된 DC에 이미지를 그린다.
-------------------------------------------------------------------*/
BOOL CPicassoImage::Draw(HDC hDC, int nImgIndex, RECT *pdstRect)
{
	NHUIDBG(DBG_INFO, (L"\n"));

	if (nImgIndex < 0 || nImgIndex >= MAX_SUPPORT_IMAGE_COUNT)
		return FALSE;

	// 그때 그때 Decode 하는 놈들.. (광고, 로딩바)
	if (m_Images[nImgIndex].nState == DECODE_INIT)
		DecodeImage(nImgIndex);

	// 디코딩 성공 시 통과
	if (m_Images[nImgIndex].nState != DECODE_DONE)
		return FALSE;

	// Draw Bitmap
	if (m_Images[nImgIndex].hBitmap != NULL)	// Decoded Bitmap exists?
	{
		HDC hBufDC = ::CreateCompatibleDC(hDC);
		if (hBufDC != NULL)
		{
			RECT *psrcRect = &(m_Images[nImgIndex].srcRect);

			NHUIDBG(DBG_INFO, (L"[%s] drawing hBitmap to (%d, %d, %d, %d)\n", m_Images[nImgIndex].FileName,
				pdstRect->left, pdstRect->top, pdstRect->right, pdstRect->bottom));

			HGDIOBJ hOld = ::SelectObject(hBufDC, m_Images[nImgIndex].hBitmap);

			if (m_Images[nImgIndex].nResultType == RESULT_BMP)
			{
				TransparentBlt(hDC,     pdstRect->left, pdstRect->top, (pdstRect->right - pdstRect->left), (pdstRect->bottom - pdstRect->top),
							   hBufDC,  psrcRect->left, psrcRect->top, (psrcRect->right - psrcRect->left), (psrcRect->bottom - psrcRect->top),
							   SCR_TRANSCOLOR);
			}
			else
			{
				StretchBlt(hDC,     pdstRect->left, pdstRect->top, (pdstRect->right - pdstRect->left), (pdstRect->bottom - pdstRect->top),
							   hBufDC,  psrcRect->left, psrcRect->top, (psrcRect->right - psrcRect->left), (psrcRect->bottom - psrcRect->top),
							   SRCCOPY);
			}
		
			::SelectObject(hBufDC, hOld);
			::DeleteDC(hBufDC);

#ifdef UNDER_CE		// skip .Wait() for faster drawing on screen viewer
			g_eWaitObject.Wait(10);
#endif
		}
	}
	else if (m_Images[nImgIndex].pImage != NULL)
	{
		NHUIDBG(DBG_INFO, (L"[%s (pImage: 0x%08X)] drawing pImage to (%d, %d, %d, %d)\n", m_Images[nImgIndex].FileName, m_Images[nImgIndex].pImage,
			pdstRect->left, pdstRect->top, pdstRect->right, pdstRect->bottom));

#ifdef UNDER_CE
		HRESULT hr = m_Images[nImgIndex].pImage->Draw(hDC, pdstRect, NULL);
		NHUIDBG(DBG_INFO, (L"result: %d (%d)\n", hr, GetLastError()));
#else
		m_Images[nImgIndex].pImage->Draw(hDC, *pdstRect);
#endif
	}

	NHUIDBG(DBG_INFO, (L"Fin\n"));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImage
 FUNCTION NAME: GetImageCount()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 이미지의 개수
 PARAMETER    : 
 DESCRIPTION  : 현재 가지고 있는 이미지의 갯수를 제공한다.
-------------------------------------------------------------------*/
int	CPicassoImage::GetImageCount()
{
	return m_nImageCount;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImage
 FUNCTION NAME: GetImageCount()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 이미지 관련 변수들을 초기화 한다.
-------------------------------------------------------------------*/
void CPicassoImage::InitImage()
{
	m_nImageCount = 0;

	for (int i = 0; i < MAX_SUPPORT_IMAGE_COUNT; i++)
	{
		m_Images[i].nState = DECODE_INIT;
		m_Images[i].nResultType = 0;
		m_Images[i].FileName = _T("");
		
		m_Images[i].srcRect.left = 0;
		m_Images[i].srcRect.top = 0;
		m_Images[i].srcRect.right = m_siScreen.cx;
		m_Images[i].srcRect.bottom = m_siScreen.cy;

		m_Images[i].hBitmap = NULL;
// Cross Compile
#ifdef UNDER_CE
		m_Images[i].pImage = NULL;
#else
		m_Images[i].pImage = NULL;
#endif
		m_Images[i].hgMemObj = NULL;
		m_Images[i].lpMemObj = NULL;
	}

}

int CPicassoImage::GetImageState(int nIndex)
{
	CNHAutoLock AutoSync(m_hCriticalSync);

	if (nIndex < 0 || nIndex >= MAX_SUPPORT_IMAGE_COUNT)
		return 0;

	return m_Images[nIndex].nState;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImage
 FUNCTION NAME: GetImageCount()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 이미지의 개수
 PARAMETER    : 
 DESCRIPTION  : 디코딩한 이미지를 해제한다.
-------------------------------------------------------------------*/
void CPicassoImage::ReleaseImage(int index)
{
	CNHAutoLock AutoSync(m_hCriticalSync);

	int i = index;

	if (i < 0 || i >= MAX_SUPPORT_IMAGE_COUNT)
		return;

	{
		m_Images[i].nState = DECODE_INIT;
		m_Images[i].nResultType = 0;

		if (m_Images[i].hBitmap != NULL)
		{
			BOOL deleteResult = ::DeleteObject(m_Images[i].hBitmap);
			NHUIDBG(DBG_INFO, (L"DeleteObject(hBitmap) [%s]: %d (%d)\n", m_Images[i].FileName, deleteResult, GetLastError()));
			m_Images[i].hBitmap = NULL;

#ifdef UNDER_CE		// skip .Wait() for faster drawing on screen viewer
			g_eWaitObject.Wait(10);
#endif
		}

// Cross Compile
#ifdef UNDER_CE
		if (m_Images[i].pImage != NULL)
		{
			NHUIDBG(DBG_INFO, (L"pImage->Release() [%s]\n", m_Images[i].FileName));

			m_Images[i].pImage->Release();
			m_Images[i].pImage = NULL;
			g_eWaitObject.Wait(10);
		}
#else
		if (m_Images[i].pImage != NULL)
		{
			// [#2434] AU Kook 2016.08.08 change from 'delete' to '::delete' to fix an exception on showing PNG AniBox with ScreenViewer.
			//delete m_Images[i].pImage;
			m_Images[i].pImage->DestroyFrames();
			m_Images[i].pImage->Destroy();
			m_Images[i].pImage = NULL;
		}
#endif

		if (m_Images[i].lpMemObj != NULL)
		{
			NHUIDBG(DBG_INFO, (L"GlobalFree()\n"));
			GlobalFree(m_Images[i].hgMemObj);

			m_Images[i].hgMemObj = NULL;
			m_Images[i].lpMemObj = NULL;
		}
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImage
 FUNCTION NAME: DecodeImage()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 이미지를 디코딩 한다.
-------------------------------------------------------------------*/
BOOL CPicassoImage::DecodeImage(int nIndex)
{
	if (nIndex < 0 || nIndex >= MAX_SUPPORT_IMAGE_COUNT)
		return FALSE;

	if (m_Images[nIndex].nState == DECODE_DONE)
		return TRUE;

	ReleaseImage(nIndex);

	BOOL	bRes = FALSE;
	CString strPreFix = m_Images[nIndex].FileName.Right(3);
	strPreFix.MakeUpper();

	if (strPreFix == _T("BMP"))
	{
		m_Images[nIndex].nResultType = RESULT_BMP;
		bRes = DecodeAs(DT_TRANSBITMAP, nIndex);
	}
	else if (strPreFix == _T("PNG") || strPreFix == _T("JPG") || strPreFix == _T("GIF"))
	{
		m_Images[nIndex].nResultType = RESULT_IMG;
		bRes = DecodeAs(DT_NORMAL, nIndex);
	}

	if (bRes == FALSE)
		m_Images[nIndex].nState = DECODE_FAIL;

	return bRes;
}

BOOL CPicassoImage::DecodeToHBitmap(int nIndex)
{
	return DecodeAs(DT_TRANSBITMAP, nIndex);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImage
 FUNCTION NAME: DecodeToIImage()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoImage::DecodeToIImage(int nIndex)
{
	return DecodeAs(DT_NORMAL, nIndex);
}

BOOL CPicassoImage::DecodeAs(DECODE_TYPE type, int nIndex)
{
#ifdef UNDER_CE
	m_Images[nIndex].nState = DECODE_FAIL;

	BOOL	bResult = FALSE;
	IImagingFactory *pImgFactory = NULL;

	NHUIDBG(DBG_INFO, (L"COINIT\n"));
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if (FAILED(hr))
	{
		NHUIDBG(DBG_INFO, (L"COINIT FAILED:%d\n", hr));
		return FALSE;
	}

	if (SUCCEEDED(CoCreateInstance(CLSID_ImagingFactory, 
		NULL, 
		CLSCTX_INPROC_SERVER, 
		IID_IImagingFactory, 
		(void**) &pImgFactory)))
	{
		bool isReadSuccessful = false;

		// 1) read the image file from storage
		// 
		// Note: AP would crash when it loads JPEG files to 'm_Images[nIndex].pImage' using 'GlobalAlloc() -> CreateStreamOnHGlobal() -> CreateImageFromStream()'
		//       and 'm_Images[nIndex].pImage->Draw()' is executed in 'CPicassoImage::Draw()'. (PNG files would work!!)
		// 
		// Workaround #1: use CreateImageFromFile() for the files. (we don't have any reason to use stream or hglobal things for files in storage)
		// Workaround #2: decode it as 'm_Images[nIndex].hBitmap' (see codes for 'transparent bitmap' below)
		HANDLE hFile = CreateFile(m_Images[nIndex].FileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			NHUIDBG(DBG_INFO, (L"[%s] NOT FOUND from the path!\n", m_Images[nIndex].FileName));
		}
		else
		{
			CloseHandle(hFile);

			HRESULT hr = pImgFactory->CreateImageFromFile(m_Images[nIndex].FileName, &m_Images[nIndex].pImage);
			NHUIDBG(DBG_INFO, (L"CreateImageFromFile(): %d [pImage: 0x%08X]\n", hr, m_Images[nIndex].pImage));

			if (hr == S_OK)
			{
				isReadSuccessful = true;
			}
		}
		
		// 2) if file doesn't exist, find it from TAR asset
		if (!isReadSuccessful)
		{
			if (g_pTarScreenAsset == NULL)
			{
				NHERROR((_T("TAR asset not available!\n")));
				goto error;
			}

			unsigned int fileSize = g_pTarScreenAsset->GetFileSize(m_Images[nIndex].FileName);
			if (fileSize <= 0)
			{
				NHUIDBG(DBG_INFO, (L"[%s] not found from TAR Asset\n", m_Images[nIndex].FileName));
				goto error;
			}

			NHUIDBG(DBG_INFO, (L"[%s] found from TAR Asset\n", m_Images[nIndex].FileName));

			// create image resource from TAR asset
			m_Images[nIndex].hgMemObj = GlobalAlloc(GMEM_MOVEABLE, fileSize);
			if (m_Images[nIndex].hgMemObj != NULL)
			{
				m_Images[nIndex].lpMemObj = GlobalLock(m_Images[nIndex].hgMemObj);
				if (m_Images[nIndex].lpMemObj != NULL)
				{
					HRESULT hrRet = g_pTarScreenAsset->ReadFile(m_Images[nIndex].FileName, (BYTE*)m_Images[nIndex].lpMemObj);

					if (hrRet == S_OK)
					{
						IStream* pStream = NULL;

						HRESULT hr = CreateStreamOnHGlobal(m_Images[nIndex].hgMemObj, FALSE, &pStream);
						NHUIDBG(DBG_INFO, (L"CreateStreamOnHGlobal(): %d \n", hr));

						if (hr == S_OK)
						{
							hr = pImgFactory->CreateImageFromStream(pStream, &m_Images[nIndex].pImage);
							NHUIDBG(DBG_INFO, (L"CreateImageFromStream(): %d [pImage: 0x%08X]\n", hr, m_Images[nIndex].pImage));

							if (hr == S_OK)
							{
								isReadSuccessful = true;
							}

							NHUIDBG(DBG_INFO, (L"stream release\n"));
							pStream->Release();
						}
					}

					NHUIDBG(DBG_INFO, (L"global unlock\n"));
					GlobalUnlock(m_Images[nIndex].hgMemObj);
				}
				else
				{
					NHUIDBG(DBG_INFO, (L"GlobalLock() failure!\n", m_Images[nIndex].FileName));
					goto error;
				}
			}
			else
			{
				NHUIDBG(DBG_INFO, (L"GlobalAlloc() failure!\n", m_Images[nIndex].FileName));
				goto error;
			}
		}

		// load success
		if (isReadSuccessful)
		{
			ImageInfo imageInfo;

			hr = m_Images[nIndex].pImage->GetImageInfo(&imageInfo);
			NHUIDBG(DBG_INFO, (L"GetImageInfo(): %d [%dx%d]\n", hr, imageInfo.Width, imageInfo.Height));

			if (hr == S_OK)
			{
				m_Images[nIndex].srcRect.left = 0;
				m_Images[nIndex].srcRect.top = 0;

				if (m_Images[nIndex].srcRect.right <= m_siScreen.cx)
					m_Images[nIndex].srcRect.right = imageInfo.Width;

				if (m_Images[nIndex].srcRect.bottom <= m_siScreen.cy)
					m_Images[nIndex].srcRect.bottom = imageInfo.Height;

				if (type == DT_NORMAL)
				{
					m_Images[nIndex].nState = DECODE_DONE;
					bResult = TRUE;
				}
				else if (type == DT_TRANSBITMAP)
				{
					CNHAutoLock AutoSync(m_hCriticalSync);

					HDC hDC = ::GetDC(NULL);
					HDC hBufDC = ::CreateCompatibleDC(hDC);

					if (hBufDC != NULL)
					{
						if (m_Images[nIndex].hBitmap != NULL)
						{
							NHUIDBG(DBG_INFO, (L"!! Trying to MakeDIBSection() again !!! \n"));
						}

						m_Images[nIndex].hBitmap = MakeDIBSection(hDC, imageInfo.Width, imageInfo.Height);

						if (m_Images[nIndex].hBitmap)
						{
							HGDIOBJ hOld = SelectObject(hBufDC, m_Images[nIndex].hBitmap);

							// fill with color of 'transparent' then draw bitmap image on it
							FillRect(hBufDC, CRect(0, 0, imageInfo.Width, imageInfo.Height), m_hbrTrans);
							m_Images[nIndex].pImage->Draw(hBufDC, &m_Images[nIndex].srcRect, NULL);

							m_Images[nIndex].nState = DECODE_DONE;
							bResult = TRUE;

							::SelectObject(hBufDC, hOld);
						}
						else
						{
							NHUIDBG(DBG_INFO, (L"MakeDIBSection() Failed: %d\n", GetLastError()));
						}

						BOOL dcDeleted = ::DeleteDC(hBufDC);
						NHUIDBG(DBG_INFO, (L"dcDeleted: %d [%d]\n", dcDeleted, GetLastError()));
					}
					else
					{
						NHUIDBG(DBG_INFO, (L"CreateCompatibleDC() Failed: %d\n", GetLastError()));

					}

					int dcReleased = ::ReleaseDC(NULL, hDC);
					NHUIDBG(DBG_INFO, (L"dcReleased: %d [%d]\n", dcReleased, GetLastError()));

					NHUIDBG(DBG_INFO, (L"image release\n"));
					m_Images[nIndex].pImage->Release();
					m_Images[nIndex].pImage = NULL;
				}
			}
		}
	}

error:
	if (pImgFactory != NULL)
	{
		NHUIDBG(DBG_INFO, (L"imgfactory release\n"));
		pImgFactory->Release();
	}

	NHUIDBG(DBG_INFO, (L"COUNINIT\n"));
	CoUninitialize();

	return bResult;

#else // UNDER_CE

	CHECK_TIME_START;

	CxImage *pImage = NULL;
	CStringA fileName(m_Images[nIndex].FileName);

	CStringA fileExt = "png";
	if (fileName.GetLength() >= 4)
		fileExt = fileName.Right(3);

	int nImgType = CxImage::GetTypeIdFromName(fileExt);

	if (g_pTarScreenAsset)
	{
		unsigned int fileSize = g_pTarScreenAsset->GetFileSize(m_Images[nIndex].FileName);

		if (fileSize == 0)
		{
			// Fix this...
			pImage = new CxImage((const char *)fileName, nImgType);

			if (pImage == NULL)
			{
				return FALSE;
			}
		}
		else
		{
			BYTE *fileBuffer = new BYTE[fileSize]();
			if (g_pTarScreenAsset->ReadFile(m_Images[nIndex].FileName, fileBuffer) == S_OK)
			{
				pImage = new CxImage(fileBuffer, fileSize, nImgType);
			}
			delete [] fileBuffer;
		}
	}
	else
	{
		// Try from the file system
		pImage = new CxImage((const char *)fileName, nImgType);
	}

	if (pImage == NULL)
	{
		return FALSE;
	}

	m_Images[nIndex].nState = DECODE_DONE;
	m_Images[nIndex].pImage = pImage;

	m_Images[nIndex].nResultType = RESULT_IMG;			// since it doesn't have 'hBitmap' in this case, we're going to manage this as PNG.

	// Bitmap Buffer Testing using Screen Viewer
	/*
	{
		HDC hDC = ::GetDC(NULL);
		HDC hBufDC = ::CreateCompatibleDC(hDC);

		int nImageWidth = pImage->GetWidth();
		int nImageHeight = pImage->GetHeight();

		m_Images[nIndex].srcRect.left = 0;
		m_Images[nIndex].srcRect.top = 0;

		if (m_Images[nIndex].srcRect.right <= m_siScreen.cx)
			m_Images[nIndex].srcRect.right = nImageWidth;

		if (m_Images[nIndex].srcRect.bottom <= m_siScreen.cy)
			m_Images[nIndex].srcRect.bottom = nImageHeight;


		NHUIDBG(DBG_INFO, (L"!! image resolution [%dx%d] !!! \n", nImageWidth, nImageHeight));

		if (hBufDC != NULL)
		{
			if (m_Images[nIndex].hBitmap != NULL)
			{
				NHUIDBG(DBG_INFO, (L"!! Trying to MakeDIBSection() again !!! \n"));
			}

			m_Images[nIndex].hBitmap = MakeDIBSection(hDC, nImageWidth, nImageHeight);

			if (m_Images[nIndex].hBitmap)
			{
				HGDIOBJ hOld = SelectObject(hBufDC, m_Images[nIndex].hBitmap);

				// fill with color of 'transparent' then draw bitmap image on it
				//FillRect(hBufDC, CRect(0, 0, nImageWidth, nImageHeight), m_hbrTrans);
				pImage->Draw(hBufDC, m_Images[nIndex].srcRect);

				//m_Images[nIndex].nResultType = RESULT_BMP;

				SelectObject(hBufDC, hOld);
			}
			else
			{
				NHUIDBG(DBG_INFO, (L"MakeDIBSection() Failed: %d\n", GetLastError()));
			}

			BOOL dcDeleted = ::DeleteDC(hBufDC);
			NHUIDBG(DBG_INFO, (L"dcDeleted: %d [%d]\n", dcDeleted, GetLastError()));
		}
		else
		{
			NHUIDBG(DBG_INFO, (L"CreateCompatibleDC() Failed: %d\n", GetLastError()));

		}

		int dcReleased = ::ReleaseDC(NULL, hDC);
		NHUIDBG(DBG_INFO, (L"dcReleased: %d [%d]\n", dcReleased, GetLastError()));
	}
	*/

	CHECK_TIME_END;

	return TRUE;

#endif // UNDER_CE

}

BOOL CPicassoImage::GetFileSizeFromFileSystem(LPCTSTR pszFilename, unsigned int &size)
{
	BOOL result = FALSE;

	// Open the file
    HANDLE hFile = CreateFile(pszFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        goto error;
    }

    DWORD dwSize = GetFileSize(hFile, NULL);
    if (0xffffffff == dwSize)
    {
        goto error;
    }
	
	size = (unsigned int) dwSize;
	result = TRUE;

error:

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
	}

	return result;
}


/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImage
 FUNCTION NAME: CreateStreamFromFileSystem()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
HRESULT CPicassoImage::CreateStreamFromFileSystem(LPCTSTR pszFilename, IStream **ppStream)
{
    HRESULT hrRet = S_OK;
    HGLOBAL hg = NULL;
 //   HANDLE hFile = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;		// [#2022] NH KSK 2011.02.22
    DWORD dwSize = 0, dwRead = 0;			// [#2022] NH KSK 2011.02.22
    BYTE* pbLocked = NULL;

    // Open the file
    hFile = CreateFile(pszFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        //info(TEXT("CreateFile failed with GLE = %d"), GetLastError());
        hrRet = 0x80000000 + GetLastError();
        goto error;
    }

    dwSize = GetFileSize(hFile, NULL);
    if (0xffffffff == dwSize)
    {
        //info(TEXT("GetFileSize failed with GLE = %d"), GetLastError());
        hrRet = 0x80000000 + GetLastError();
        goto error;
    }

    // Open a memory object
    hg = GlobalAlloc(GMEM_MOVEABLE, dwSize);
    if (NULL == hg)
    {
        //info(TEXT("GlobalAlloc failed with GLE = %d"), GetLastError());
        hrRet = 0x80000000 + GetLastError();
        goto error;
    }

    // Ge a pointer to the memory we just allocated
    pbLocked = (BYTE*) GlobalLock(hg);
    if (NULL == pbLocked)
    {
        //info(TEXT("GlobalLock failed with GLE = %d"), GetLastError());
        hrRet = 0x80000000 + GetLastError();
        goto error;
    }

    // copy the file
    if (!ReadFile(hFile, pbLocked, dwSize, &dwRead, NULL))
    {
        //info(TEXT("ReadFile failed with GLE = %d"), GetLastError());
        hrRet = 0x80000000 + GetLastError();
        goto error;
    }

    GlobalUnlock(hg);
    
    // Create the stream
    hrRet = CreateStreamOnHGlobal(hg, TRUE, ppStream);

    CloseHandle(hFile);
    return hrRet;

error:
    if (pbLocked)
        GlobalUnlock(hg);
    if (hg)
        GlobalFree(hg);
//    if (hFile)
    if (hFile != INVALID_HANDLE_VALUE)		// [#2022] NH KSK 2011.02.22
        CloseHandle(hFile);

    return hrRet;
}
