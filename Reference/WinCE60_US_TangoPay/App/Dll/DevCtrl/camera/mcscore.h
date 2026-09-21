#ifndef _MCSCORE_API_H_
#define _MCSCORE_API_H_

#define HJAPI


#define MCS_SUCESS				(1)
#define MCS_FAIL				(0)
#define MCS_ERR_CMD_FAIL		(9702012)	//TakePicture 명령 실패
#define MCS_ERR_COM_FAIL		(9702014)	//통신 장애
#define MCS_ERR_NO_CAM			(9702041)	//TakePicture 실패(카메라 찾을 수 없음)
#define MCS_ERR_NO_PATH			(9702042)	//TakePicture 실패 (잘못된 파일 경로)
#define MCS_ERR_ERR_FRAME		(9702045)	//TakePicture 실패 (정상 프래임 수신 실패)
#define MCS_ERR_ARG_ERR			(9702046)	//TakePicture 실패(잘못된 Input  파라메터)
#define MCS_ERR_ART_EXCEPTION	(9702048)	//TakePicture 실패(잘못된 Input  Exception)
#define MCS_SKIP_FRAME			(9702070)	//SKIP FRAME 초기화면 깨짐 방지 
#define MCS_ERR_PRE_ARG_ERR	     (9702071)	//Preview 실패(잘못된 Input  파라메터) 
#define MCS_ERROR_NO_USB_CAM	(0xFFFFFFFF)  //CAM USB 연결 해제됨.

#define MAIN_CAM		(1)
#define SUB_CAM			(2)

#define MCS_IMG_WIDTH					(320)
#define MCS_IMG_HEIGHT					(240)

HJAPI void MCSDLL_VERSION();


#define MCS_YES		(TRUE)
#define MCS_NO		(FALSE)

typedef struct s_MCS_Config {
	BOOL	bUseCaption ;			//Default 1
	BOOL	bUseBMP ;				//Default 0
	BOOL	bUseJPEG;				//Default 1
	BOOL	bLogEnable;				//Default 0
	char	CaptionColor ;			//Default 1
	char	JPEGQuality ;			//Default 2
	char	LogPath[MAX_PATH];		//Default "\\ATM2\\LOG\\"
	char	LogMaxFiles;			//Default 30
	char	SkipVidRenderFrame;		//Default 10   Range 0~30
} MCS_Config;

HJAPI int MCSCORE_Set_Config(MCS_Config	* pMcsConfig);
HJAPI int MCSCORE_Init();
HJAPI int MCSCORE_Deinit();
// nlX_width is mutiples of 8
HJAPI int MCSDDRAW_InitP ( long nlX, long nlY, long nlX_width, long nlY_height);
HJAPI int MCSDDRAW_InitE ( long nlX, long nlY, long nlX_width, long nlY_height);
HJAPI int MCSDDRAW_DeinitP ();
HJAPI int MCSDDRAW_DeinitE ();
HJAPI int MCSDDRAW_VideoRenderP(unsigned char *pCamData);
HJAPI int MCSDDRAW_VideoRenderE(unsigned char *pCamData);

/*	
	For JPEG Encoder Quality
	Encode quality level 0 is lowest, so the size is smaller.
	Quality level 1 is better than level 0, and the size is bigger than level 0
	Quality evel 2 is better than level 1, the size is bigger than level 1 ,and so on.
*/
#define QUALITY_LEVEL0   0x00000000
#define QUALITY_LEVEL1   0x00000001
#define QUALITY_LEVEL2   0x00000002
#define QUALITY_LEVEL3   0x00000003
#define QUALITY_LEVEL4   0x00000004

HJAPI int MCSIMG_ImgCaptureP(LPCSTR pFileName);
HJAPI int MCSIMG_ImgCaptureE(LPCSTR pFileName);
HJAPI int MCSIMG_GrayColorP(BOOL bGrayColor);
HJAPI int MCSIMG_GrayColorE(BOOL bGrayColor);



#define COLOR_BLACK			0x00	//검정색
#define COLOR_DEFALUT		0x01	//검정색(디폴트)
#define COLOR_GRAY			0x02	//회색
#define COLOR_WHITE			0x03	//흰색
#define COLOR_RED			0x04	//빨강색
#define COLOR_GREEN			0x05	//녹색
#define COLOR_BLUE			0x06	//파랑색
#define COLOR_PURPLE		0x07	//진보라색
#define COLOR_LIGHTPURPLE	0x08	//연보라색
#define COLOR_DARKGREEN		0x09	//짙은 녹색
#define COLOR_KHAKI			0x0A	//국방색
#define COLOR_BROWN			0x0B	//갈색
#define COLOR_LIGHTRED		0x0C	//옅은 빨강색
#define COLOR_SKYBLUE		0x0D	//하늘색
#define COLOR_YELLOW		0x0E	//노랑색
#define COLOR_PINK			0x0F	//분홍색
#define COLOR_ORANGE		0x10	//주황색

HJAPI int MCSTXT_InsertAndColorP(int nColor, long x, long y, TCHAR* szText);
HJAPI int MCSTXT_InsertAndColorE(int nColor, long x, long y, TCHAR* szText);


#define MCS_DP_NO_LOG	0	
#define MCS_DP_LOG_ONLY 1
#define MCS_DP_FILE_LOG 2

HJAPI void MCSDP_SET(int val);
HJAPI void __cdecl MCSDP(const char * szFormat, ...);

#endif