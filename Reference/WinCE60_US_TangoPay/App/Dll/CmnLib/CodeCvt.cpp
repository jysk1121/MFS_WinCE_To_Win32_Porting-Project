#include "stdafx.h"
#include ".\Common\CmnLib.h"

#define	SO			0x0e
#define	SI			0x0f
#define	KSCSPACE	0xa1a1
#define IBMSPACE	0x4040

////////////////////////////////////////
/// main function
////////////////////////////////////////

void WINAPI WideToMulti(LPSTR multi, LPCWSTR wide, int size)
{
    WideCharToMultiByte(CP_ACP, 0, wide, -1, multi, size, NULL, NULL);
}

void WINAPI MultiToWide(LPWSTR wide, LPCSTR multi, int size)
{
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)multi, -1, wide, size);
}
