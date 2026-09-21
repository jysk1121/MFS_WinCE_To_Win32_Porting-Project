#include "stdafx.h"
#include "SystemCheck.h"

//#define NH_DEBUG
#include ".\Common\NHDbgApi.h"

#define DBG_INFO		1

//CSystemCheck theSystemCheck;

//-----------------------------------------------------------------------------
// Local definitions
//-----------------------------------------------------------------------------

// These are the pixel formats this app supports.  Most display adapters
// with overlay support will recognize one or more of these formats.
// We start with YUV format, then work down to RGB. (All 16 bpp.)

static DDPIXELFORMAT ddpfOverlayFormats[] = {
    //{sizeof(DDPIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('Y','U','Y','V'),0,0,0,0,0},  // YUYV
    //{sizeof(DDPIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('U','Y','V','Y'),0,0,0,0,0},  // UYVY
    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16,  0x7C00, 0x03e0, 0x001F, 0},        // 16-bit RGB 5:5:5
    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16,  0xF800, 0x07e0, 0x001F, 0}         // 16-bit RGB 5:6:5
};

#define PF_TABLE_SIZE (sizeof(ddpfOverlayFormats) / sizeof(ddpfOverlayFormats[0]))

BOOL	g_bExitIdleThread = FALSE;

void theIdleProce(LPVOID pvParmas);

CSystemCheck::CSystemCheck()
{
	m_pDD = NULL;
	m_pDDSPrimary = NULL;
	m_pDDSOverlay = NULL;

	m_hbmOverlay = NULL;

	memset(m_chPowHistory, -1, HISTORY_MAX);
	m_chPowHistory[0] = 0;
	memset(m_chMemHistory, -1, HISTORY_MAX);
	m_chMemHistory[0] = 0;

	m_hpenDarkGreen = NULL;
	m_hpenYellow = NULL;
	m_hpenRed = NULL;

	m_nPhysInitMemory = 0;
	m_nPhysCurMemory = 0;
	m_nPhysAvailMemory = 0;

	m_hbrushTransparent = NULL;
}

CSystemCheck::~CSystemCheck()
{
	m_bDoing = FALSE;
	g_bExitIdleThread = TRUE;

	if (m_hbrushTransparent != NULL)
	{
		DeleteObject(m_hbrushTransparent);
		m_hbrushTransparent = NULL;
	}

	if (m_hpenRed != NULL)
	{
		DeleteObject(m_hpenRed);
		m_hpenRed = NULL;
	}

	if (m_hpenYellow != NULL)
	{
		DeleteObject(m_hpenYellow);
		m_hpenYellow = NULL;
	}

	if (m_hpenDarkGreen != NULL)
	{
		DeleteObject(m_hpenDarkGreen);
		m_hpenDarkGreen = NULL;
	}

	if (m_hbmOverlay != NULL)
	{
		DeleteObject(m_hbmOverlay);
		m_hbmOverlay = NULL;
	}

	if (m_pDDSOverlay != NULL)
	{
		m_pDDSOverlay->UpdateOverlay(NULL, m_pDDSPrimary, NULL, DDOVER_HIDE, NULL);
		m_pDDSOverlay->Release();
		m_pDDSOverlay = NULL;
	}

	if (m_pDDSPrimary != NULL)
	{
		m_pDDSPrimary->Release();
		m_pDDSPrimary = NULL;
	}

	if (m_pDD != NULL)
	{
		m_pDD->Release();
		m_pDD = NULL;
	}
}

BOOL CSystemCheck::CreateSurface()
{
	DDSURFACEDESC       ddsd;
	DDCAPS              ddcaps;
	HRESULT				hRet;
	DWORD               dwUpdateFlags = 0;
	DDOVERLAYFX         ovfx;

	m_hpenDarkGreen = ::CreatePen(PS_SOLID, 1, RGB(0, 127, 0));
	if (m_hpenDarkGreen == NULL)
	{
		NHERROR((_T("Create DrakGreen Pen FAILED.\n")));
		return FALSE;
	}

	m_hpenYellow = ::CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
	if (m_hpenYellow == NULL)
	{
		NHERROR((_T("Create Yellow Pen FAILED.\n")));
		return FALSE;
	}

	m_hpenRed = ::CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	if (m_hpenRed == NULL)
	{
		NHERROR((_T("Create Red Pen FAILED.\n")));
		return FALSE;
	}

	m_hbrushTransparent = ::CreateSolidBrush(SCR_TRANSCOLOR);
	if (m_hbrushTransparent == NULL)
	{
		NHERROR((_T("Create Transparent Brush FAILED.\n")));
		return FALSE;
	}

    hRet = DirectDrawCreate(NULL, &m_pDD, NULL);
    if (hRet != DD_OK)
	{
		NHERROR((_T("DirectDrawCreate FAILED. err(0x%08X)\n"), hRet));
		return FALSE;
	}

    hRet = m_pDD->SetCooperativeLevel(NULL, DDSCL_NORMAL);
    if (hRet != DD_OK)
	{
		NHERROR((_T("SetCooperativeLevel FAILED. err(0x%08X)\n"), hRet));
		return FALSE;
	}

    // Get a primary surface interface pointer (only needed for init.)
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    hRet = m_pDD->CreateSurface(&ddsd, &m_pDDSPrimary, NULL);
    if (hRet != DD_OK)
	{
		NHERROR((_T("CreateSurface FAILED. err(0x%08X)\n"), hRet));
		return FALSE;
	}

    // See if we can support overlays.
    memset(&ddcaps, 0, sizeof(ddcaps));
    ddcaps.dwSize = sizeof(ddcaps);
    hRet = m_pDD->GetCaps(&ddcaps, NULL);
    if (hRet != DD_OK)
	{
		NHERROR((_T("GetCaps FAILED. err(0x%08X)\n"), hRet));
		return FALSE;
	}

    if (ddcaps.dwOverlayCaps == 0)
	{
		NHERROR((_T("Overlays are not supported in hardware.!!\n")));
		return FALSE;
	}

    // Get alignment info to compute our overlay surface size.

#if 1
	m_rcPrimary.left = 0;
    //m_rcPrimary.top = GetSystemMetrics(SM_CYSCREEN) - 100;
	m_rcPrimary.top = 0;
	m_rcPrimary.right = GRAPHIC_WIDTH + (SQUARE_SIZE * 4) + INFO_WIDTH;
	//m_rcPrimary.bottom = GetSystemMetrics(SM_CYSCREEN);
	m_rcPrimary.bottom = 100;
#else
    m_rcPrimary.left = 0;
	m_rcPrimary.top = 600 - 100;
	m_rcPrimary.right = 1024;
	m_rcPrimary.bottom = 600;
#endif

    if (ddcaps.dwAlignSizeSrc != 0)
	    m_rcPrimary.right += m_rcPrimary.right % ddcaps.dwAlignSizeSrc;

	m_rcOverlay.left = 0;
	m_rcOverlay.top = 0;
	m_rcOverlay.right = (m_rcPrimary.right - m_rcPrimary.left);
	m_rcOverlay.bottom = (m_rcPrimary.bottom - m_rcPrimary.top);	

	NHUIDBG(DBG_INFO, (_T("Overlay Size : (%d, %d, %d, %d), Primary Pos : (%d, %d, %d, %d)\n"),
							m_rcOverlay.left, m_rcOverlay.top, m_rcOverlay.right, m_rcOverlay.bottom,
							m_rcPrimary.left, m_rcPrimary.top, m_rcPrimary.right, m_rcPrimary.bottom));
    
    // Create the overlay flipping surface. We will attempt the pixel formats
    // in our table one at a time until we find one that jives.

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_FLIP;
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_BACKBUFFERCOUNT | 
                   DDSD_PIXELFORMAT;
	ddsd.dwWidth = (m_rcOverlay.right - m_rcOverlay.left);
	ddsd.dwHeight = (m_rcOverlay.bottom - m_rcOverlay.top);
    ddsd.dwBackBufferCount = 2;

    int i = 0;
    do 
	{
	    ddsd.ddpfPixelFormat = ddpfOverlayFormats[i];
	    hRet = m_pDD->CreateSurface(&ddsd, &m_pDDSOverlay, NULL);
    } while (hRet != DD_OK && (++i < PF_TABLE_SIZE));

    if (hRet != DD_OK)
	{
		NHERROR((_T("Unable to create overlay surface.!! err(0x%08X)\n"), hRet));
		return FALSE;
	}

	// Set the flags we'll send to UpdateOverlay
    dwUpdateFlags = DDOVER_SHOW;

    // Does the overlay hardware support source color keying?
    // If so, we can hide the black background around the image.
    // This probably won't work with YUV formats
    memset(&ovfx, 0, sizeof(ovfx));
    ovfx.dwSize = sizeof(ovfx);
    if (ddcaps.dwOverlayCaps & DDOVERLAYCAPS_CKEYSRC)
    {
		NHUIDBG(DBG_INFO, (_T("Overlay support a source color key.\n")));
        dwUpdateFlags |= DDOVER_KEYSRCOVERRIDE;

        // Create an overlay FX structure so we can specify a source color key.
        // This information is ignored if the DDOVER_SRCKEYOVERRIDE flag 
        // isn't set.
        ovfx.dckSrcColorkey.dwColorSpaceLowValue= DDColorMatch(m_pDDSOverlay, SCR_TRANSCOLOR); // black as the color key
        ovfx.dckSrcColorkey.dwColorSpaceHighValue = ovfx.dckSrcColorkey.dwColorSpaceLowValue;
		m_pDDSOverlay->SetColorKey(DDCKEY_SRCBLT, &ovfx.dckSrcColorkey);
    }

	HDC	hDCOverlay = NULL;
	m_pDDSOverlay->GetDC(&hDCOverlay);
	if (hDCOverlay == NULL)
	{
		NHERROR((_T("GetDC for Overlay FAILED.\n")));
		return FALSE;
	}

	m_hbmOverlay = CreateCompatibleBitmap(hDCOverlay, (m_rcOverlay.right-m_rcOverlay.left), (m_rcOverlay.bottom-m_rcOverlay.top));
	if (m_hbmOverlay == NULL)
	{
		m_pDDSOverlay->ReleaseDC(hDCOverlay);
		NHERROR((_T("CreateCompatibleBitmap FAILED.\n")));
		return FALSE;
	}

	FillRect(hDCOverlay, &m_rcOverlay, m_hbrushTransparent);

	m_pDDSOverlay->ReleaseDC(hDCOverlay);

	hRet = m_pDDSOverlay->UpdateOverlay(&m_rcOverlay, m_pDDSPrimary, &m_rcPrimary, dwUpdateFlags, &ovfx);

	// Calcuate graphic postion
	int	nTemp;
	
	m_rcGraphic.left = m_rcOverlay.left + SQUARE_SIZE;
	m_rcGraphic.right = m_rcOverlay.left + GRAPHIC_WIDTH;

	m_rcGraphic.top = m_rcOverlay.top;
	m_rcGraphic.bottom = m_rcOverlay.bottom;

	nTemp = m_rcPrimary.bottom - m_rcPrimary.top;
	if (nTemp > GRAPHIC_HEIGHT)
	{
		m_rcGraphic.top += (int)(nTemp - GRAPHIC_HEIGHT)/2;
		m_rcGraphic.bottom = m_rcGraphic.top + GRAPHIC_HEIGHT;
	}

	NHUIDBG(DBG_INFO, (_T("DRAW GRAPIC : (%d, %d, %d, %d)\n"), m_rcGraphic.left, m_rcGraphic.top, m_rcGraphic.right, m_rcGraphic.bottom));
	return TRUE;
}

DWORD CSystemCheck::DDColorMatch(IDirectDrawSurface * pdds, COLORREF rgb)
{
    COLORREF                rgbT;
    HDC                     hdc;
    DWORD                   dw = CLR_INVALID;
    DDSURFACEDESC           ddsd;
    HRESULT                 hres;

    //
    //  Use GDI SetPixel to color match for us
    //
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);     // Save current pixel value
        SetPixel(hdc, 0, 0, rgb);       // Set our value
        pdds->ReleaseDC(hdc);
    }
    else
    {
        return dw;
    }
    //
    // Now lock the surface so we can read back the converted color
    //
    ddsd.dwSize = sizeof(ddsd);
    while ((hres = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
        ;
    if (hres == DD_OK)
    {
        dw = *(DWORD *) ddsd.lpSurface;                 // Get DWORD
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
            dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;  // Mask it to bpp
        pdds->Unlock(NULL);
    }
    //
    //  Now put the color that was there back.
    //
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }
    return dw;
}

void CSystemCheck::Measure()
{
	static DWORD	dwLastThreadTime = 0;
	static DWORD	dwLastTickTime = 0;
	DWORD	dwCpuPower;
	DWORD	dwCurrentThreadTime = 0;
	DWORD	dwCurrentTickTime = 0;

	MEMORYSTATUS ms;
	ms.dwLength = sizeof(MEMORYSTATUS);

	FILETIME	ftCreationTime;
	FILETIME	ftExitTime;
	FILETIME	ftKernelTime;
	FILETIME	ftUserTime;

	SuspendThread(m_hIdleThread);

	dwCurrentTickTime = GetTickCount();
	GetThreadTimes(m_hIdleThread, &ftCreationTime, &ftExitTime, &ftKernelTime, &ftUserTime);
	dwCurrentThreadTime = GetThreadTick(&ftKernelTime, &ftUserTime);

	// calculate cpu power
	if (dwCurrentTickTime != dwLastTickTime || dwLastThreadTime != 0 || dwLastTickTime != 0)
		dwCpuPower = 100 - (((dwCurrentThreadTime - dwLastThreadTime) * 100) / (dwCurrentTickTime - dwLastTickTime));
	else
		dwCpuPower = 0;	//avoid 0 div

	// memory history
	GlobalMemoryStatus(&ms);

	// Shift history
	memmove(m_chPowHistory+1, m_chPowHistory, HISTORY_MAX-1);
	memmove(m_chMemHistory+1, m_chMemHistory, HISTORY_MAX-1);

	// save history
	m_chPowHistory[0] = (char)dwCpuPower;
	m_chMemHistory[0] = (char)ms.dwMemoryLoad;
	
	m_nPhysCurMemory = (ms.dwTotalPhys - ms.dwAvailPhys);
	m_nPhysAvailMemory = ms.dwAvailPhys;
	if (m_nPhysInitMemory == 0)
		m_nPhysInitMemory = m_nPhysCurMemory;

	// save status;
	dwLastTickTime = dwCurrentTickTime;
	dwLastThreadTime = dwCurrentThreadTime;

	ResumeThread(m_hIdleThread);
}

DWORD CSystemCheck::GetThreadTick(FILETIME* a, FILETIME* b)
{
	__int64	a64 = 0;
	__int64 b64 = 0;

	a64 = a->dwHighDateTime;
	a64 <<= 32;
	a64 += a->dwLowDateTime;

	b64 = b->dwHighDateTime;
	b64 <<= 32;
	b64 += b->dwLowDateTime;

	a64 += b64;

	// nano sec to milli sec
	a64 /= 10000;

	return (DWORD)a64;
}

BOOL CSystemCheck::UpdateSurface()
{
	HDC			hDCOverlay = NULL;

	m_pDDSOverlay->GetDC(&hDCOverlay);
	if (hDCOverlay != NULL)
	{
		HDC hDCImage = CreateCompatibleDC(hDCOverlay);
		if (hDCImage != NULL)
		{
			int		ii;
			int		nDrawWidth, nDrawHeight;
			POINT	pnt[HISTORY_MAX];

			HPEN	hOldPen;
			HBITMAP	hOldBitmap;

			RECT	rcBackGrahpic;

			static int nXLine = 0;
			nXLine = (nXLine+MOVE_SIZE) % SQUARE_SIZE;

			// Get Width, Height of draw rectangle
			nDrawWidth = (m_rcGraphic.right - m_rcGraphic.left);
			nDrawHeight = (m_rcGraphic.bottom - m_rcGraphic.top);

			hOldBitmap = (HBITMAP)SelectObject(hDCImage, m_hbmOverlay);

			FillRect(hDCImage, &m_rcOverlay, m_hbrushTransparent);
			//FillRect(hDCImage, &m_rcOverlay, (HBRUSH)GetStockObject(BLACK_BRUSH));

			// paint a table
			{
				rcBackGrahpic = m_rcGraphic;
				rcBackGrahpic.left -= MOVE_SIZE;
				rcBackGrahpic.top -= MOVE_SIZE;
				rcBackGrahpic.right += MOVE_SIZE;
				rcBackGrahpic.bottom += MOVE_SIZE;
				FillRect(hDCImage, &rcBackGrahpic, (HBRUSH)GetStockObject(BLACK_BRUSH));
				
				hOldPen = (HPEN)SelectObject(hDCImage, m_hpenDarkGreen);

				pnt[0].x = m_rcGraphic.left;
				pnt[0].y = m_rcGraphic.top;
				pnt[1].x = m_rcGraphic.right;
				pnt[1].y = m_rcGraphic.top;
				Polyline(hDCImage, pnt, 2);

				pnt[2].x = m_rcGraphic.right;
				pnt[2].y = m_rcGraphic.bottom;
				pnt[3].x = m_rcGraphic.left;
				pnt[3].y = m_rcGraphic.bottom;
				Polyline(hDCImage, pnt, 2);
				
				for (ii = 0; ii <= nDrawHeight; ii+=SQUARE_SIZE)
				{
					pnt[0].x = m_rcGraphic.left;
					pnt[1].x = m_rcGraphic.right;
					pnt[0].y = pnt[1].y = ii + m_rcGraphic.top;
					Polyline(hDCImage, &pnt[0], 2);
				}

				for (ii = SQUARE_SIZE; ii <= nDrawWidth + nXLine; ii+=SQUARE_SIZE)
				{
					pnt[0].x = pnt[1].x = ii + m_rcGraphic.left - nXLine;
					pnt[0].y = m_rcGraphic.top;
					pnt[1].y = m_rcGraphic.bottom;
					Polyline(hDCImage, &pnt[0], 2);
				}

				SelectObject(hDCImage, hOldPen);
			}

			// paint cpu power
			{
				memset(pnt, 0, sizeof(POINT) * HISTORY_MAX);
				POINT *pPoint = &pnt[0];
				int nXpos = m_rcGraphic.right;
				for (ii = 0; (ii < HISTORY_MAX) && (nXpos >= m_rcGraphic.left); ii++, nXpos-=(MOVE_SIZE*2), pPoint++)
				{
					LONG	lHeight = (100 - m_chPowHistory[ii]);
					pPoint->x = nXpos;
					pPoint->y = m_rcGraphic.top + (lHeight * nDrawHeight / 100);
				}
				
				hOldPen = (HPEN)SelectObject(hDCImage, m_hpenRed);
				Polyline(hDCImage, &pnt[0], ii);
				SelectObject(hDCImage, hOldPen);
			}

			// paint memory load
			{
				memset(pnt, 0, sizeof(POINT) * HISTORY_MAX);
				POINT *pPoint = &pnt[0];
				int nXpos = m_rcGraphic.right;
				for (ii = 0; (ii < HISTORY_MAX) && (nXpos >= m_rcGraphic.left); ii++, nXpos-=(MOVE_SIZE*2), pPoint++)
				{
					LONG	lHeight = (100 - m_chMemHistory[ii]);
					pPoint->x = nXpos;
					pPoint->y = m_rcGraphic.top + (lHeight * nDrawHeight / 100);
				}
				
				hOldPen = (HPEN)SelectObject(hDCImage, m_hpenYellow);
				Polyline(hDCImage, &pnt[0], ii);
				SelectObject(hDCImage, hOldPen);
			}

			// Paint Info Text
			{
				CString		strInfo;
				RECT		rcInfo, rcTemp;
				COLORREF	hOldTextColor;
				
				strInfo.Format(_T("CPU : %d %%\nInit  : %8d\nCur  : %8d\nDiff  : %8d\nAvail : %8d"),
									(int)m_chPowHistory[0],
									m_nPhysInitMemory,
									(int)m_nPhysCurMemory,
									(int)(m_nPhysCurMemory - m_nPhysInitMemory),
									m_nPhysAvailMemory);

				rcInfo.left = m_rcGraphic.right + SQUARE_SIZE;
				rcInfo.right = rcInfo.left + INFO_WIDTH;
				rcInfo.top = m_rcGraphic.top;
				rcInfo.bottom = m_rcGraphic.bottom;

				int OldBkMode = ::SetBkMode(hDCImage, TRANSPARENT);
				hOldTextColor = (COLORREF)SetTextColor(hDCImage, RGB(0, 0, 0));
				rcTemp = rcInfo;
				rcTemp.left -= 1;
				rcTemp.right -= 1;
				DrawText(hDCImage, strInfo, -1, &rcTemp, DT_LEFT);

				rcTemp = rcInfo;
				rcTemp.top -= 1;
				rcTemp.bottom -= 1;
				DrawText(hDCImage, strInfo, -1, &rcTemp, DT_LEFT);

				rcTemp = rcInfo;
				rcTemp.left += 1;
				rcTemp.right += 1;
				DrawText(hDCImage, strInfo, -1, &rcTemp, DT_LEFT);

				rcTemp = rcInfo;
				rcTemp.top += 1;
				rcTemp.bottom += 1;
				DrawText(hDCImage, strInfo, -1, &rcTemp, DT_LEFT);

				SetTextColor(hDCImage, hOldTextColor);

				hOldTextColor = (COLORREF)SetTextColor(hDCImage, RGB(255, 255, 255));
				DrawText(hDCImage, strInfo, -1, &rcInfo, DT_LEFT);
				SetTextColor(hDCImage, hOldTextColor);

				::SetBkMode(hDCImage, OldBkMode);
			}
			
			if (BitBlt(hDCOverlay, 0, 0, (m_rcOverlay.right - m_rcOverlay.left), (m_rcOverlay.bottom - m_rcOverlay.top), hDCImage, 0, 0, SRCCOPY) == FALSE)
			{
				NHERROR((_T("BitBlt FAILED.\n")));
			}

			SelectObject(hDCImage, hOldBitmap);

			DeleteDC(hDCImage);
		}

		m_pDDSOverlay->ReleaseDC(hDCOverlay);
	}
	else
	{
		NHERROR((_T("GetDC for Overlay FAILED.\n")));
	}

	return TRUE;    
}

unsigned CSystemCheck::ThreadHandlerProc(void)
{
	m_bDoing = TRUE;
	RETAILMSG(1, (TEXT("[CALL] (%s)\n"),TEXT(__FUNCTION__)));

	if (CreateIdleThread() == FALSE)
	{
		NHERROR((_T("CreateIdleThread FAILED.\n")));
		return -1;
	}

	if (CreateSurface() == FALSE)
	{
		NHERROR((_T("CreateSurace FAILED.\n")));
		return -1;
	}

	RETAILMSG(1, (TEXT("[CALL] START (%s)\n"),TEXT(__FUNCTION__)));

	while (m_bDoing)
	{
		m_eMethodUse.Wait(2000);
		Measure();
		UpdateSurface();
	}

	RETAILMSG(1, (TEXT("[CALL] END (%s)\n"),TEXT(__FUNCTION__)));

	return 0;
}

BOOL CSystemCheck::CreateIdleThread()
{
	DWORD	ThreadID;
	m_hIdleThread = CreateThread(NULL,
								0,
								(LPTHREAD_START_ROUTINE)theIdleProce,
								NULL,
								CREATE_SUSPENDED,
								&ThreadID);
	if (m_hIdleThread == NULL)
	{
		NHERROR((_T("IDLE Thread Create FAILED.\n")));
		return FALSE;
	}

	SetThreadPriority(m_hIdleThread, THREAD_PRIORITY_IDLE);
	ResumeThread(m_hIdleThread);

	return TRUE;
}

void theIdleProce(LPVOID pvParmas)
{
	while (g_bExitIdleThread == FALSE);
}
