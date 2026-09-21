
// FOR WINCE60 Define
//#include ".\Common\WinCE60Def.h"	// [#685] NH KSK 2011.01.24

// Cross Compile
#ifdef UNDER_CE
#include <atlbase.h>
#define INITGUID
#include <windows.h>
#include <initguid.h>
#include "NHImage.h"
#include "stdafx.h"
#else // UNDER_CE
#include "stdafx.h"
#include "NHImage.h"
#include "ImageStone.h"
#endif // UNDER_CE

CNHCritSec	CNHImage::m_hCriticalSync;
int			CNHImage::m_nRefCnt = 0;			// 객체 참조 횟수
HBITMAP		CNHImage::m_hbmScrBuf = NULL;		// 전체 화면 Bitmap
HBRUSH		CNHImage::m_hbrTrans = NULL;		// transparent color for bitmap
SIZE		CNHImage::m_siScreen = {0, 0};		// 전체 화면 크기


/*-------------------------------------------------------------------
 CLASS    NAME: CNHImage
 FUNCTION NAME: CNHImage()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 디폴트 생성자
-------------------------------------------------------------------*/
CNHImage::CNHImage()
{
#ifdef UNDER_CE
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif
	
	{
		CNHAutoLock AutoSync(m_hCriticalSync);

		if (m_hbmScrBuf == NULL)
		{
			m_siScreen.cx = GetSystemMetrics(SM_CXSCREEN);
			m_siScreen.cy = GetSystemMetrics(SM_CYSCREEN);

			HDC dc = GetDC(NULL);
			m_hbmScrBuf = CreateCompatibleBitmap(dc, m_siScreen.cx, m_siScreen.cy);
			ReleaseDC(NULL, dc);
		}

		if (m_hbrTrans == NULL)
		{
			m_hbrTrans = CreateSolidBrush(SCR_TRANSCOLOR);
		}

		m_nRefCnt++;
	}

	InitImage();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHImage
 FUNCTION NAME: CNHImage(LPCTSTR pFileName[], int nCount, BOOL bDecodeNow)
 RETURN TYPE  : 
 PARAMETER    : pFileName : 디코딩할 파일 이름의 배열
				nCount : pFileName의 개수 (최대 10개)
				bDecodeNow : 생성시 디코딩 수행 옵션
							 만약 FALSE이면 Draw시 Decoding이 수행된다.
 DESCRIPTION  : 1. 디코딩할 파일의 정보를 받는다.
                2. 생성시 디코딩을 수행한다.(옵션)
-------------------------------------------------------------------*/
CNHImage::CNHImage(LPCTSTR pFileName[], int nCount, BOOL bDecodeNow)
{
#ifdef UNDER_CE
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif

	SetDecodeInfo(pFileName, nCount, bDecodeNow);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHImage
 FUNCTION NAME: ~CNHImage()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자
-------------------------------------------------------------------*/
CNHImage::~CNHImage()
{
	ReleaseImage();

	{
		CNHAutoLock AutoSync(m_hCriticalSync);

		m_nRefCnt--;

		if (m_nRefCnt <= 0)
		{ 
			if (m_hbmScrBuf != NULL)
				DeleteObject(m_hbmScrBuf);

			if (m_hbrTrans != NULL)
				DeleteObject(m_hbrTrans);
		}
	}

#ifdef UNDER_CE
	CoUninitialize();
#endif
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHImage
 FUNCTION NAME: SetDecodeInfo(LPCTSTR pFileName[], int nCount, BOOL bDecodeNow)
 RETURN TYPE  : TRUE : SUCCESSFUL, FALSE : FAIL
 PARAMETER    : pFileName : 디코딩할 파일 이름의 배열
				nCount : pFileName의 개수 (최대 10개)
				bDecodeNow : 생성시 디코딩 수행 옵션
							 만약 FALSE이면 Draw시 Decoding이 수행된다.
 DESCRIPTION  : 1. 디코딩할 파일의 정보를 받는다.
                2. 함수 호출시 디코딩을 수행한다.(옵션)
				- 이전에 디코딩한 파일들이 있다면 모두 해제 된다.
-------------------------------------------------------------------*/
BOOL CNHImage::SetDecodeInfo(LPCTSTR pFileName[], int nCount, BOOL bDecodeNow)
{
	int	i;

	ReleaseImage();
	InitImage();

	m_nImageCount = nCount;

	for (i = 0; i < nCount; i++)
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
 CLASS    NAME: CNHImage
 FUNCTION NAME: Draw(HDC hDC, int nImgIndex, RECT *pdestRect)
 RETURN TYPE  : 
 PARAMETER    : hDC : 장치 Device Context
				nImgIndex : 그려질 이미지 index (0 base) 
				pdestRect : 이미지를 DC에 표시할 영역
 DESCRIPTION  : 제공된 DC에 이미지를 그린다.
-------------------------------------------------------------------*/
BOOL CNHImage::Draw(HDC hDC, int nImgIndex, RECT *pdstRect)
{
	if (nImgIndex < 0 || nImgIndex >= MAX_SUPPORT_IMAGE_COUNT)
		return FALSE;

	// 그때 그때 Decode 하는 놈들.. (광고, 로딩바)
	if (m_Images[nImgIndex].nState == DECODE_INIT)
		DecodeImage(nImgIndex);

	// 디코딩 성공, 화면 버퍼가 있을때 통과
	if (!(m_Images[nImgIndex].nState == DECODE_DONE && m_hbmScrBuf != NULL))
		return FALSE;

	RECT *psrcRect = &(m_Images[nImgIndex].srcRect);

	// Draw Bitmap
	if (m_Images[nImgIndex].nResultType == RESULT_BMP &&
	    m_Images[nImgIndex].hBitmap != NULL)
	{
		HDC hBufDC = CreateCompatibleDC(hDC);
		if (hBufDC != NULL)
		{
			HGDIOBJ hOld = SelectObject(hBufDC, m_Images[nImgIndex].hBitmap);

			// Copy Screen Buf -> Screen
			TransparentBlt(hDC,     pdstRect->left, pdstRect->top, (pdstRect->right - pdstRect->left), (pdstRect->bottom - pdstRect->top),
						   hBufDC,  psrcRect->left, psrcRect->top, (psrcRect->right - psrcRect->left), (psrcRect->bottom - psrcRect->top),
						   SCR_TRANSCOLOR);
		
			SelectObject(hBufDC, hOld);
			DeleteDC(hBufDC);
		}
	}
	else if (m_Images[nImgIndex].nResultType == RESULT_IMG &&
			 m_Images[nImgIndex].pImage != NULL)
	{
		HDC hBufDC = CreateCompatibleDC(hDC);
		if (hBufDC != NULL)
		{
			HGDIOBJ hOld = SelectObject(hBufDC, m_hbmScrBuf);

			// copy Screen -> Screen Buf - scaleing
			BitBlt(hBufDC, psrcRect->left, psrcRect->top, (pdstRect->right - pdstRect->left), (pdstRect->bottom - pdstRect->top),
				   hDC,	   pdstRect->left, pdstRect->top, SRCCOPY);

// Cross Compile
#ifdef UNDER_CE
			// draw png on Screen Buf
			m_Images[nImgIndex].pImage->Draw(hBufDC, CRect(psrcRect->left, psrcRect->top, (pdstRect->right - pdstRect->left), (pdstRect->bottom - pdstRect->top)), NULL);
#else
			m_Images[nImgIndex].pImage->Draw(hBufDC, CRect(psrcRect->left, psrcRect->top, (pdstRect->right - pdstRect->left), (pdstRect->bottom - pdstRect->top)));
#endif
			// copy Screen Buf -> Screen
			BitBlt(hDC,    pdstRect->left, pdstRect->top, (pdstRect->right - pdstRect->left), (pdstRect->bottom - pdstRect->top),
				   hBufDC, psrcRect->left, psrcRect->top,  SRCCOPY);

			SelectObject(hBufDC, hOld);			
			DeleteDC(hBufDC);
		}
	}
	// V1.0.2.3 2018.06.27 - GIF Animation 추가
	else if (m_Images[nImgIndex].nResultType == RESULT_GIF &&
		m_Images[nImgIndex].pImage != NULL)
	{
		HDC hBufDC = CreateCompatibleDC(hDC);
		if (hBufDC != NULL)
		{
			HGDIOBJ hOld = SelectObject(hBufDC, m_hbmScrBuf);
			
			// copy Screen -> Screen Buf - scaleing
			BitBlt(hBufDC, psrcRect->left, psrcRect->top, (pdstRect->right - pdstRect->left), (pdstRect->bottom - pdstRect->top),
				hDC,	   pdstRect->left, pdstRect->top, SRCCOPY);
			
			// Cross Compile
#ifdef UNDER_CE
			// draw png on Screen Buf
			m_Images[nImgIndex].pImage->Draw(hBufDC, CRect(psrcRect->left, psrcRect->top, (pdstRect->right - pdstRect->left), (pdstRect->bottom - pdstRect->top)), NULL);
#else
			m_Images[nImgIndex].pImage->Draw(hBufDC, CRect(psrcRect->left, psrcRect->top, (pdstRect->right - pdstRect->left), (pdstRect->bottom - pdstRect->top)));
#endif
			// copy Screen Buf -> Screen
			BitBlt(hDC,    pdstRect->left, pdstRect->top, (pdstRect->right - pdstRect->left), (pdstRect->bottom - pdstRect->top),
				hBufDC, psrcRect->left, psrcRect->top,  SRCCOPY);
			
			SelectObject(hBufDC, hOld);			
			DeleteDC(hBufDC);
		}
	}
	////////////////////////////

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHImage
 FUNCTION NAME: GetImageCount()
 RETURN TYPE  : 이미지의 개수
 PARAMETER    : 
 DESCRIPTION  : 현재 가지고 있는 이미지의 갯수를 제공한다.
-------------------------------------------------------------------*/
int	CNHImage::GetImageCount()
{
	return m_nImageCount;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHImage
 FUNCTION NAME: GetImageCount()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 이미지 관련 변수들을 초기화 한다.
-------------------------------------------------------------------*/
void CNHImage::InitImage()
{
	int i;

	m_nImageCount = 0;

	for (i = 0; i < MAX_SUPPORT_IMAGE_COUNT; i++)
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
		m_Images[i].pStream = NULL;
#else
		m_Images[i].pImage = NULL;
#endif
	}

}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHImage
 FUNCTION NAME: GetImageCount()
 RETURN TYPE  : 이미지의 개수
 PARAMETER    : 
 DESCRIPTION  : 디코딩한 이미지를 해제한다.
-------------------------------------------------------------------*/
void CNHImage::ReleaseImage()
{
	int i;

	for (i = 0; i < m_nImageCount; i++)
	{
		m_Images[i].nState = DECODE_INIT;
		m_Images[i].nResultType = 0;

		if (m_Images[i].hBitmap != NULL)
			DeleteObject(m_Images[i].hBitmap);

// Cross Compile
#ifdef UNDER_CE
		if (m_Images[i].pImage != NULL)
			m_Images[i].pImage->Release();

		if (m_Images[i].pStream != NULL)
			m_Images[i].pStream->Release();
#else
		if (m_Images[i].pImage != NULL)
		{
			delete m_Images[i].pImage;
		}

#endif
	}
}

BOOL CNHImage::DecodeImage(int nIndex)
{
	BOOL	bRes = FALSE;
	CString strPreFix;
	
	if (nIndex < 0 || nIndex >= MAX_SUPPORT_IMAGE_COUNT)
		return FALSE;

	if (m_Images[nIndex].nState == DECODE_DONE)
		return TRUE;
	else
	{
		ReleaseImage();

		strPreFix = m_Images[nIndex].FileName.Right(3);
		strPreFix.MakeUpper();

		if (strPreFix == _T("BMP")  || strPreFix == _T("JPG"))
		{
			m_Images[nIndex].nResultType = RESULT_BMP;
			bRes = DecodeToHBitmap(nIndex);
		}
		else if (strPreFix == _T("PNG"))
		{
			m_Images[nIndex].nResultType = RESULT_IMG;
			bRes = DecodeToIImage(nIndex);
		}

		if (bRes == FALSE)
			m_Images[nIndex].nState = DECODE_FAIL;
	}

	return bRes;
}

BOOL CNHImage::DecodeImage4GIF()
{
	BOOL	bRes = FALSE;
	
	bRes = DecodeToGIF();
	
	return bRes;
}

/*
BOOL CNHImage::DrawPNG(int nIndex, HDC hDC, RECT destRect)
{
	BOOL	bResult = FALSE;
	IImagingFactory *pImgFactory = NULL;
	IImage *pImage = NULL;

	if (SUCCEEDED(CoCreateInstance(CLSID_ImagingFactory, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_IImagingFactory, 
                          (void**) &pImgFactory)))
	{
		ImageInfo imageInfo;

		if (SUCCEEDED(pImgFactory->CreateImageFromFile(m_Images[nIndex].FileName, &pImage)) &&
			SUCCEEDED(pImage->GetImageInfo(&imageInfo)))
		{
			pImage->Draw(hDC, &destRect, NULL);

			pImage->Release();
		}
		pImgFactory->Release();
	}

	return bResult;
}
*/

BOOL CNHImage::DecodeToHBitmap(int nIndex)
{
// Cross Compile
#ifdef UNDER_CE

	IImagingFactory *pImgFactory = NULL;
	IImage *pImage = NULL;
	BOOL bResult = FALSE;

	if (SUCCEEDED(CoCreateInstance(CLSID_ImagingFactory, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_IImagingFactory, 
                          (void**) &pImgFactory)))
	{
		ImageInfo imageInfo;

		if (SUCCEEDED(pImgFactory->CreateImageFromFile(m_Images[nIndex].FileName, &pImage)) &&
			SUCCEEDED(pImage->GetImageInfo(&imageInfo)))
		{
			// [#618] NH AIREAT 2010.02.17 CHECK IMAGE RESOLUTION
			if (((long)imageInfo.Width <= m_siScreen.cx) && ((long)imageInfo.Height <= m_siScreen.cy))
			{
				CWindowDC dc(NULL);
				CDC dcBitmap;
				dcBitmap.CreateCompatibleDC(&dc);
				m_Images[nIndex].hBitmap = CreateCompatibleBitmap(dc.GetSafeHdc(), imageInfo.Width, imageInfo.Height);
				if (m_Images[nIndex].hBitmap)
				{
					m_Images[nIndex].srcRect.left = 0;
					m_Images[nIndex].srcRect.top = 0;
					
					if (m_Images[nIndex].srcRect.right <= m_siScreen.cx)
						m_Images[nIndex].srcRect.right = imageInfo.Width;

					if (m_Images[nIndex].srcRect.bottom <= m_siScreen.cy)
						m_Images[nIndex].srcRect.bottom = imageInfo.Height;

					HGDIOBJ hOldBitmap = dcBitmap.SelectObject(m_Images[nIndex].hBitmap);
					
					FillRect(dcBitmap.GetSafeHdc(), CRect(0, 0, imageInfo.Width, imageInfo.Height), m_hbrTrans);
					pImage->Draw(dcBitmap.GetSafeHdc(), &m_Images[nIndex].srcRect, NULL);
					
					dcBitmap.SelectObject(hOldBitmap);

					m_Images[nIndex].nState = DECODE_DONE;
					bResult = TRUE;
				}
			}
			else
				m_Images[nIndex].nState = DECODE_FAIL;
			// end of [#618]

			pImage->Release();
		}
		pImgFactory->Release();
	}

	return bResult;
	
#else // UNDER_CE

	FCObjImage *pImage = NULL;

	pImage = (FCObjImage*) new FCObjImage;
	if (pImage != NULL)
	{
		if (!pImage->Load(m_Images[nIndex].FileName))
		{
			pImage->Destroy();
			delete pImage;
			return FALSE;
		}

		CWindowDC dc(NULL);
		CDC dcBitmap;
		dcBitmap.CreateCompatibleDC(&dc);
		m_Images[nIndex].hBitmap = CreateCompatibleBitmap(dc.GetSafeHdc(), pImage->Width(), pImage->Height());
		if (m_Images[nIndex].hBitmap == NULL)
		{
			delete pImage;
			return FALSE;
		}

		m_Images[nIndex].srcRect.left = 0;
		m_Images[nIndex].srcRect.top = 0;
		
		if (m_Images[nIndex].srcRect.right <= m_siScreen.cx)
			m_Images[nIndex].srcRect.right = pImage->Width();

		if (m_Images[nIndex].srcRect.bottom <= m_siScreen.cy)
			m_Images[nIndex].srcRect.bottom = pImage->Height();

		HGDIOBJ hOldBitmap = dcBitmap.SelectObject(m_Images[nIndex].hBitmap);
		
		FillRect(dcBitmap.GetSafeHdc(), CRect(0, 0, pImage->Width(), pImage->Height()), m_hbrTrans);
		FCWin32::DrawImage(*pImage, dcBitmap.GetSafeHdc(), m_Images[nIndex].srcRect);
		
		dcBitmap.SelectObject(hOldBitmap);

		m_Images[nIndex].nState = DECODE_DONE;

		pImage->Destroy();
		delete pImage;
		return TRUE;
	}

	return FALSE;

#endif // UNDER_CE
}

BOOL CNHImage::DecodeToIImage(int nIndex)
{
// Cross Compile
#ifdef UNDER_CE

	BOOL	bResult = FALSE;
	IImagingFactory *pImgFactory = NULL;


	if (SUCCEEDED(CoCreateInstance(CLSID_ImagingFactory, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_IImagingFactory, 
                          (void**) &pImgFactory)))
	{
		ImageInfo imageInfo;

		if (SUCCEEDED(CreateStreamOnFile(m_Images[nIndex].FileName, &m_Images[nIndex].pStream)))
		{
			if (SUCCEEDED(pImgFactory->CreateImageFromStream(m_Images[nIndex].pStream, &m_Images[nIndex].pImage)) &&
				SUCCEEDED(m_Images[nIndex].pImage->GetImageInfo(&imageInfo)))
			{
				m_Images[nIndex].srcRect.left = 0;
				m_Images[nIndex].srcRect.top = 0;
				
				if (m_Images[nIndex].srcRect.right <= m_siScreen.cx)
					m_Images[nIndex].srcRect.right = imageInfo.Width;

				if (m_Images[nIndex].srcRect.bottom <= m_siScreen.cy)
					m_Images[nIndex].srcRect.bottom = imageInfo.Height;

				m_Images[nIndex].nState = DECODE_DONE;
				bResult = TRUE;
			}
		}
		pImgFactory->Release();
	}

	return bResult;

#else // UNDER_CE

	CxImage *pImage = NULL;

	CString strExtension = m_Images[nIndex].FileName.Right(3);
	strExtension.MakeLower();

	int type = CxImage::GetTypeIdFromName(strExtension);
	pImage = new CxImage(m_Images[nIndex].FileName, type);
	if (pImage != NULL)
	{
		m_Images[nIndex].nState = DECODE_DONE;
		m_Images[nIndex].pImage = pImage;
		return TRUE;
	}

	return FALSE;

#endif // UNDER_CE
}


BOOL CNHImage::DecodeToGIF()
{
	BOOL				bRes = FALSE;
#ifdef UNDER_CE
	IImagingFactory		*pImgFactory = NULL;
	IStream				*pStream = NULL;
	IImageDecoder		*pImageDecoder = NULL;
	IBitmapImage		*pBitmapImage = NULL;
	IImageSink			*pImageSink = NULL;

		// Create ImagingFactory
    if (SUCCEEDED(CoCreateInstance(CLSID_ImagingFactory, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_IImagingFactory, 
                          (void**) &pImgFactory)))
	{
//		RETAILMSG(1, (L"GIF : %s\n", m_Images[0].FileName));
		if (SUCCEEDED(CreateStreamOnFile(m_Images[0].FileName, &pStream)))
		{
			if (SUCCEEDED(pImgFactory->CreateImageDecoder(pStream, DecoderInitFlagBuiltIn1st, &pImageDecoder)))
			{
				HRESULT	hr = S_OK;
				int		i;
				GUID	pGuid[100];
				UINT	nCount = 0;
				UINT	nFrame = 0;
				ImageInfo ii;

				memset(pGuid, 0, sizeof(pGuid));

				pImageDecoder->GetImageInfo(&ii);
				pImageDecoder->GetFrameDimensionsCount(&nCount);
				pImageDecoder->GetFrameDimensionsList(pGuid, nCount);
				pImageDecoder->GetFrameCount(&pGuid[0], &nFrame);

//				RETAILMSG(1, (L"GIF FRAME (%d)\n", nFrame));

				for (i = 0; i < (int)nFrame; i++)
				{
					if (i >= MAX_SUPPORT_IMAGE_COUNT)
						break;

					bRes = FALSE;

					pImageDecoder->SelectActiveFrame(&pGuid[0], i);

					if (SUCCEEDED(pImgFactory->CreateNewBitmap(ii.Width, ii.Height, ii.PixelFormat, &pBitmapImage)))
					{
						if (SUCCEEDED(pBitmapImage->QueryInterface(IID_IImageSink, (void**)&pImageSink)))
						{
							if (SUCCEEDED(pBitmapImage->QueryInterface(IID_IImage, (void**)&m_Images[i].pImage)))
							{
								if (SUCCEEDED(pImageDecoder->BeginDecode(pImageSink, NULL)))
								{
									while (E_PENDING == (hr = pImageDecoder->Decode()))
										Sleep(0);

									if (SUCCEEDED(pImageDecoder->EndDecode(hr)))
									{
										m_Images[i].nState = DECODE_DONE;
										m_Images[i].nResultType = RESULT_GIF;
										m_Images[i].FileName = m_Images[0].FileName;
										m_nImageCount = i + 1;

 										bRes = TRUE;
									}
								}
							}
						}
						pImageSink->Release();
					}
					pBitmapImage->Release();

					if (bRes != TRUE)
						break;
				}
				pImageDecoder->Release();
			}
			pStream->Release();
		}
		pImgFactory->Release();
	}
#else // UNDER_CE
	CFile file;
	int nFrame = 0;
	CString strTemp;
	
	if (file.Open(m_Images[0].FileName, CFile::modeRead)==0)
	{
		strTemp.Format(_T("GIF can't Read!. FileName:[%s]"), m_Images[0].FileName);
		AfxMessageBox(strTemp);
		return FALSE;
	}
	
	DWORD m_binarysize=file.GetLength();
	BYTE *pBuf=new BYTE[m_binarysize];
	file.ReadHuge(pBuf, m_binarysize);
	file.Close();
	
	CxImage *pCxImage = new CxImage;
	
	if(pCxImage->Decode(pBuf, m_binarysize, CXIMAGE_FORMAT_GIF) == FALSE)
	{
		if(pCxImage->Decode(pBuf, m_binarysize, CXIMAGE_FORMAT_BMP) == FALSE)
		{
			AfxMessageBox(_T("ImageFile is abnormal."));
			delete pCxImage;
			return FALSE;
		}
	}
	
	m_Images[0].pImage = pCxImage;
	m_Images[0].nState = DECODE_DONE;
	m_Images[0].nResultType = RESULT_GIF;
	
	nFrame = pCxImage->GetNumFrames();
	
	for(int i=1; i < nFrame; i++)
	{
		if (i >= MAX_SUPPORT_IMAGE_COUNT)
			break;

		CxImage *newImage = new CxImage();
		newImage->SetFrame(i);
		
		if(newImage->Decode(pBuf, m_binarysize, CXIMAGE_FORMAT_GIF) == TRUE)
		{
			m_Images[i].pImage = newImage;
			m_Images[i].nState = DECODE_DONE;
			m_Images[i].nResultType = RESULT_GIF;
			m_Images[i].FileName = m_Images[0].FileName;
			m_nImageCount = i + 1;
		}
		else
		{
			AfxMessageBox(_T("Decode is failed."));
			delete newImage;
			return FALSE;
		}
	}
	
	delete []pBuf;

	bRes = TRUE;

#endif // UNDER_CE

	return bRes;
}


/*
BOOL CNHImage::DecodeTest(int nIndex)
{
	HRESULT	hr = S_OK;
	IImagingFactory		*pImagingFactory = NULL;
	IStream				*pStream = NULL;
	IImageDecoder		*pImageDecoder = NULL;
	IBitmapImage		*pBitmapImage = NULL;
	IImageSink			*pImageSink = NULL;
	ImageInfo ii;

    TCHAR *tszError;
    TCHAR tszNotReg[] = TEXT("REGDB_E_CLASSNOTREG");
    TCHAR tszNoAgg[] = TEXT("CLASS_E_NOAGGREGATION");
    TCHAR tszUnknown[] = TEXT("unknown hr");

	// Create ImagingFactory
    hr = CoCreateInstance(CLSID_ImagingFactory, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_IImagingFactory, 
                          (void**) &pImagingFactory);
    if (FAILED(hr))
    {
        switch(hr)
        {
        case REGDB_E_CLASSNOTREG:
            tszError = tszNotReg;
            break;
        case CLASS_E_NOAGGREGATION:
            tszError = tszNoAgg;
            break;
        default:
            tszError = tszUnknown;
            break;
        }
        //info(TEXT("CoCreateInstance failed, hr: 0x%08x (%s)"), hr, tszError);
        goto finish;
    }

	// Create IStream;
	if (FAILED(hr = CreateStreamOnFile(m_Images[nIndex].FileName, &pStream)))
	{
		//info(TEXT("CreateStreamOnFile failed, hr: 0x%08x"), hr);
		goto finish;
	}

	// Create ImageDecoder
	if (FAILED(hr = pImagingFactory->CreateImageDecoder(pStream, DecoderInitFlagBuiltIn1st, &pImageDecoder)))
	{
		//info(TEXT("CreateImageDecoder failed, hr: 0x%08x"), hr);
		goto finish;
	}

	pImageDecoder->GetImageInfo(&ii);
	ii.PixelFormat = PixelFormat16bppRGB565;

	if (FAILED(hr = pImagingFactory->CreateNewBitmap(ii.Width, ii.Height, ii.PixelFormat, &pBitmapImage)))
	{
		//info(TEXT("CreateNewBitmap failed, hr = 0x%08x"), hr);
		goto finish;
	}

	if (FAILED(hr = pBitmapImage->QueryInterface(IID_IImageSink, (void**)&pImageSink)))
	{
		//info(TEXT("QueryInterface for ImageSink from BitmapImage failed, hr: 0x%08x"), hr);
		goto finish;
	}
	
	//ColorPalette	transparent;
	//transparent.Flags = PALFLAG_HASALPHA;
	//transparent.Count = 1;
	//transparent.Entries[0] = MAKEARGB(0, 1, 1, 1);

	//pImageSink->SetPalette(&transparent);

	if (FAILED(hr = pImageDecoder->BeginDecode(pImageSink, NULL)))
	{
		//info(TEXT("BeginDecode into Bitmap Image failed, hr = 0x%08d"), hr);
		goto finish;
	}
	
	while (E_PENDING == (hr = pImageDecoder->Decode()))
		Sleep(0);
	
	hr = pImageDecoder->EndDecode(hr);	
	if (FAILED(hr))
	{
		//info(TEXT("Decoding failed, hr = 0x%08x"), hr);
		goto finish;
	}
	
	pImageSink->Release();
	pImageSink = NULL;
	
	if (FAILED(hr = pBitmapImage->QueryInterface(IID_IImage, (void**)&m_Images[nIndex].pImage)))
	{
		//info(TEXT("QueryInterface for Image from BitmapImage failed, hr: 0x%08x"), hr);
		goto finish;
	}

finish:

	if (pBitmapImage)
		pBitmapImage->Release();

	if (pImageDecoder)
		pImageDecoder->Release();

	if (pStream)
		pStream->Release();

	if (pImagingFactory)
		pImagingFactory->Release();

	if (FAILED(hr))
		return FALSE;

	m_Images[nIndex].nState = DECODE_DONE;

	return TRUE;
}
*/

HRESULT CNHImage::CreateStreamOnFile(LPCTSTR pszFilename, IStream **ppStream)
{
    HRESULT hrRet = S_OK;
    HGLOBAL hg = NULL;
    HANDLE hFile = NULL;
    DWORD dwSize = 0, dwRead = 0;
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

	// [#684] NH KSK 2011.01.17 코드소나 지적사항 대책
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
	// end of [#684]

    return hrRet;
}
