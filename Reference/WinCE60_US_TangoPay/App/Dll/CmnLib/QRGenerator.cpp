//	---------------------------------------------------------------------------
//		
//		QRGenerator
//
//		Create:			15/05/2013
//		Last update:	15/05/2013
//
//		Author:	TWOTM
//
//
//		Note:
//
//		/o ULTRAMUNDUM FOUNDATION - all rights reserved
//	---------------------------------------------------------------------------


//	-------------------------------------------------------
//	Includes
//	-------------------------------------------------------

#include "stdafx.h"
#include <math.h>
// NH Kook 2020.02.12 we don't need to include below headers.
//#include <string.h>
//#include <errno.h>
//#include <conio.h>
//#include <ctype.h>
//#include <stdio.h>
//#include <stddef.h>
//#include <stdlib.h>
//#include <wchar.h>


#include "LibQREncode/qrencode.h"
//	-------------------------------------------------------
#define NH_DEBUG

#define	DBG_CALL		1
#define DBG_INFO		1

#include ".\Common\CmnLib.h"
#include ".\Common\NHDbgApi.h"

//	-------------------------------------------------------
//	DEFines
//	-------------------------------------------------------

// #define OUT_FILE_PIXEL_PRESCALER	8											// Prescaler (number of pixels in bmp file for each QRCode pixel, on each dimension)
#define OUT_FILE_PIXEL_PRESCALER	1											// NH Kook 2020.02.12 For file size optimization. "1" seems enough since Picasso shows it as scaled.

#define PIXEL_COLOR_R				0x0											// Color of bmp pixels
#define PIXEL_COLOR_G				0x0
#define PIXEL_COLOR_B				0x0

// NH Kook 2020.02.12 Already defined in another file.
/*
// BMP defines
typedef unsigned short	WORD;
typedef unsigned long	DWORD;
typedef signed long		LONG;

#define BI_RGB			0L	
*/

#pragma pack(push, 2)

// NH Kook 2020.02.12 Already defined in another file.
// typedef struct  
// 	{
// 	WORD    bfType;
// 	DWORD   bfSize;
// 	WORD    bfReserved1;
// 	WORD    bfReserved2;
// 	DWORD   bfOffBits;
// 	} BITMAPFILEHEADER;
// 
// typedef struct 
// 	{
// 	DWORD      biSize;
// 	LONG       biWidth;
// 	LONG       biHeight;
// 	WORD       biPlanes;
// 	WORD       biBitCount;
// 	DWORD      biCompression;
// 	DWORD      biSizeImage;
// 	LONG       biXPelsPerMeter;
// 	LONG       biYPelsPerMeter;
// 	DWORD      biClrUsed;
// 	DWORD      biClrImportant;
// 	} BITMAPINFOHEADER;

#pragma pack(pop)
//	-------------------------------------------------------


//	-------------------------------------------------------
//	Main
//	-------------------------------------------------------
int WINAPI MakeQRCodeImageFile(CString pText, CString pFilePath)
{
	CStringA		strTextA(pText);
	CStringA		strFilePathA(pFilePath);

	unsigned int	unDataBytes, unPixels;
	unsigned char*	pRGBData;
	QRcode*			pQRC;
	FILE*			f;

	QRecLevel		eccLevel = QR_ECLEVEL_H;
#if (US_VERSION || AU_VERSION)
	eccLevel = QR_ECLEVEL_M;
#endif

	unsigned int	unBiIndex = 0;

	/*
	* Create a symbol from the string. The library automatically parses the input
	* string and encodes in a QR Code symbol.
	* @warning This function is THREAD UNSAFE when pthread is disabled.
	* @param string input string. It must be NUL terminated.
	* @param version version of the symbol. If 0, the library chooses the minimum
	*                version for the given input data.
	* @param level error correction level.
	* @param hint tell the library how non-alphanumerical characters should be
	*             encoded. If QR_MODE_KANJI is given, kanji characters will be
	*             encoded as Shif-JIS characters. If QR_MODE_8 is given, all of
	*             non-alphanumerical characters will be encoded as is. If you want
	*             to embed UTF-8 string, choose this.
	* @param casesensitive case-sensitive(1) or not(0).
	* @return an instance of QRcode class. The version of the result QRcode may
	*         be larger than the designated version. On error, NULL is returned,
	*         and errno is set to indicate the error. See Exceptions for the
	*         details.
	* @throw EINVAL invalid input object.
	* @throw ENOMEM unable to allocate memory for input objects.
	* @throw ERANGE input data is too large.
	*/

	// Compute QRCode


	if (pQRC = QRcode_encodeString((const char*)strTextA, 0, eccLevel, QR_MODE_8, 1))
	{
		unsigned int unRowSize = (int)ceil((double)pQRC->width / 32.0) * 4;
		unPixels = pQRC->width * pQRC->width; // number of Pixels per row
		unDataBytes = unRowSize * pQRC->width; // Total Bytes in the pixel array

		// Allocate pixels buffer
		if (!(pRGBData = (unsigned char*)malloc(unDataBytes)))
		{
			NHDEBUG(1, (_T("OUT OF MEMORY \n")));
			// 			printf("Out of memory");
			// 			exit(-1);
			return 1;
		}

		// Preset to white
		memset(pRGBData, 0x00, unDataBytes);

		// Prepare bmp headers
		NHDEBUG(1, (_T("Preparing BMP Header \n")));
		BITMAPFILEHEADER kFileHeader;
		kFileHeader.bfType = 0x4d42;  // "BM"
		kFileHeader.bfSize =	sizeof(BITMAPFILEHEADER) +
								sizeof(BITMAPINFOHEADER) +
								(2 * sizeof(RGBQUAD))    +
								unDataBytes;

		kFileHeader.bfReserved1 = 0;
		kFileHeader.bfReserved2 = 0;
		kFileHeader.bfOffBits =	sizeof(BITMAPFILEHEADER) + // File Header
								sizeof(BITMAPINFOHEADER) + // Bitmap information header
								(2 * sizeof(RGBQUAD));     // Color map with two colors

		RGBQUAD		bmiColors[2];

		// White
		bmiColors[0].rgbBlue = 0xFF;
		bmiColors[0].rgbGreen = 0xFF;
		bmiColors[0].rgbRed = 0xFF;
		bmiColors[0].rgbReserved = 0x00;

		// Black
		bmiColors[1].rgbBlue = 0;
		bmiColors[1].rgbGreen = 0;
		bmiColors[1].rgbRed = 0;
		bmiColors[1].rgbReserved = 0;

		BITMAPINFOHEADER kInfoHeader;
		kInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
		kInfoHeader.biWidth = pQRC->width;
		kInfoHeader.biHeight = pQRC->width;
		kInfoHeader.biPlanes = 1;
		kInfoHeader.biBitCount = 1;
		kInfoHeader.biSizeImage = unDataBytes;
		kInfoHeader.biXPelsPerMeter = 0;
		kInfoHeader.biYPelsPerMeter = 0;
		kInfoHeader.biCompression = BI_RGB;
		kInfoHeader.biClrUsed = 2;
		kInfoHeader.biClrImportant = 2;


		// Convert QrCode bits to bmp pixels
		NHDEBUG(1, (_T("Converting QR Code bits to BMP pixels \n")));

		BYTE temp = 0;
		BYTE qrData = 0;
		int qrIndex = unPixels - pQRC->width;
		int pixelsMapped = 0;
		int skip = 0;
		for(unsigned int y = 0; y < unDataBytes; y++)
		{
			for (int x = 0; x < 8; x++)
			{
				if (pixelsMapped == pQRC->width)
				{
					// end of row. Skip pointer to next row
					skip = unRowSize - (y % unRowSize);

					pixelsMapped = 0;
					break;
				}

				if (qrIndex < 0)
				{
					// Don't underrun data[]
					break;
				}

				/*
				 * MSB 76543210 LSB
				 *     |||||||`- 1=black/0=white
				 *     ||||||`-- data and ecc code area
				 *     |||||`--- format information
				 *     ||||`---- version information
				 *     |||`----- timing pattern
				 *     ||`------ alignment pattern
				 *     |`------- finder pattern and separator
				 *     `-------- non-data modules (format, timing, etc.)
				 */
				qrData = pQRC->data[qrIndex++];

				temp |= (qrData & 1) << (7 - x);
				pixelsMapped++;
			}

			pRGBData[y] = temp;
			temp = 0;

			// Skip the padding bytes, if set
			// Subtract one from the padding because the for loop will increment 'y' also
			// This indicates we're on a new row
			if (skip > 0)
			{
				y += (skip - 1);
				skip = 0;

				qrIndex -= pQRC->width * 2;
			}
		}

		// Output the bmp file
		NHDEBUG(1, (_T("Writing BMP file: %s\n"), pFilePath));
		if (!(fopen_s(&f, strFilePathA, "wb")))
		{
			fwrite(&kFileHeader, sizeof(BITMAPFILEHEADER), 1, f);
			fwrite(&kInfoHeader, sizeof(BITMAPINFOHEADER), 1, f);
			fwrite(bmiColors, sizeof(RGBQUAD), 2, f);
			fwrite(pRGBData, sizeof(unsigned char), unDataBytes, f);

			fclose(f);
			NHDEBUG(1, (_T("DONE \n")));
		}
		else
		{
			//printf("Unable to open file");
			//exit(-1);
			NHDEBUG(1, (_T("Unable to open file\n")));
		}

		// Free data

		free(pRGBData);
		QRcode_free(pQRC);
	}
	else
	{
		//printf("NULL returned");
		//exit(-1);
		NHDEBUG(1, (_T("NULL returned \n")));
	}

	return 0;
}
//	-------------------------------------------------------
