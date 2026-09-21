 
// #define VideoProcAmp_Brightness              0
// #define VideoProcAmp_Contrast                1
// #define VideoProcAmp_Hue                     2
// #define VideoProcAmp_saturation              3
// #define VideoProcAmp_sharpness				4
// #define VideoProcAmp_gamma					5
// #define VideoProcAmp_colorenable				6
// #define VideoProcAmp_whitebalance			7
// #define VideoProcAmp_backlightcompensation	8
// #define VideoProcAmp_gain					9


//#define CameraControl_pan						0// VideoProcAmp+cameracontrol	10
//#define CameraControl_tilt					1// VideoProcAmp+cameracontrol	11
//#define CameraControl_roll					2// VideoProcAmp+cameracontrol	12
//#define CameraControl_zoom					3// VideoProcAmp+cameracontrol	13
//#define CameraControl_iris					4// VideoProcAmp+cameracontrol	14
//#define CameraControl_exposure				5// VideoProcAmp+cameracontrol	15
//#define CameraControl_focus					6// VideoProcAmp+cameracontrol  16
//#define CameraControl_flash					7// VideoProcAmp+cameracontrol  17
 
 
// using this flag set the White Balance by the IAMVideoProcAmp  interface





typedef  enum                             //0
{
	 VideoProcAmp_Brightness_1      = 20  , 
	 VideoProcAmp_Brightness_2		= 40  ,
	 VideoProcAmp_Brightness_3		= 60  ,
	 VideoProcAmp_Brightness_4		= 80  ,
	 VideoProcAmp_Brightness_5		= 100 ,
	 VideoProcAmp_Brightness_6		= 120 , 
	 VideoProcAmp_Brightness_7		= 140 ,
	 VideoProcAmp_Brightness_8		= 160 ,
	 VideoProcAmp_Brightness_9		= 180 ,
	 VideoProcAmp_Brightness_10		= 200 ,
	 VideoProcAmp_Brightness_11		= 220 , 
	 VideoProcAmp_Brightness_12		= 240 
	 
 } VideoProcAmpBrightness;

typedef  enum                             //1
{
	 VideoProcAmp_Contrast_1        = 20  , 
	 VideoProcAmp_Contrast_2		= 40  ,
	 VideoProcAmp_Contrast_3		= 60  ,
	 VideoProcAmp_Contrast_4		= 80  ,
	 VideoProcAmp_Contrast_5		= 100 ,
	 VideoProcAmp_Contrast_6		= 120 , 
	 VideoProcAmp_Contrast_7		= 140 ,
	 VideoProcAmp_Contrast_8		= 160 ,
	 VideoProcAmp_Contrast_9		= 180 ,
	 VideoProcAmp_Contrast_10		= 200 ,
	 VideoProcAmp_Contrast_11		= 220 , 
	 VideoProcAmp_Contrast_12		= 240 
	 
 } VideoProcAmpContrast;

typedef  enum                             //2
{
	 VideoProcAmp_Hue_1     = 20  , 
	 VideoProcAmp_Hue_2		= 40  ,
	 VideoProcAmp_Hue_3		= 60  ,
	 VideoProcAmp_Hue_4		= 80  ,
	 VideoProcAmp_Hue_5		= 100 ,
	 VideoProcAmp_Hue_6		= 120 , 
	 VideoProcAmp_Hue_7		= 140 ,
	 VideoProcAmp_Hue_8		= 160 ,
	 VideoProcAmp_Hue_9		= 180 ,
	 VideoProcAmp_Hue_10	= 200 ,
	 VideoProcAmp_Hue_11	= 220 , 
	 VideoProcAmp_Hue_12	= 240 
	 
 } VideoProcAmpHue;

typedef  enum                             //3
{
	 VideoProcAmp_Saturation_1      = 20  , 
	 VideoProcAmp_Saturation_2		= 40  ,
	 VideoProcAmp_Saturation_3		= 60  ,
	 VideoProcAmp_Saturation_4		= 80  ,
	 VideoProcAmp_Saturation_5		= 100 ,
	 VideoProcAmp_Saturation_6		= 120 , 
	 VideoProcAmp_Saturation_7		= 140 ,
	 VideoProcAmp_Saturation_8		= 160 ,
	 VideoProcAmp_Saturation_9		= 180 ,
	 VideoProcAmp_Saturation_10		= 200 ,
	 VideoProcAmp_Saturation_11		= 220 , 
	 VideoProcAmp_Saturation_12		= 240 
	 
 } VideoProcAmpSaturation;

typedef  enum                             //4
{
	 VideoProcAmp_Sharpness_1       = 20  , 
	 VideoProcAmp_Sharpness_2		= 40  ,
	 VideoProcAmp_Sharpness_3		= 60  ,
	 VideoProcAmp_Sharpness_4		= 80  ,
	 VideoProcAmp_Sharpness_5		= 100 ,
	 VideoProcAmp_Sharpness_6		= 120 , 
	 VideoProcAmp_Sharpness_7		= 140 ,
	 VideoProcAmp_Sharpness_8		= 160 ,
	 VideoProcAmp_Sharpness_9		= 180 ,
	 VideoProcAmp_Sharpness_10		= 200 ,
	 VideoProcAmp_Sharpness_11		= 220 , 
	 VideoProcAmp_Sharpness_12		= 240 
	 
 } VideoProcAmpSharpness;

typedef  enum                             //5
{
	 VideoProcAmp_Gamma_1       = 20  , 
	 VideoProcAmp_Gamma_2		= 40  ,
	 VideoProcAmp_Gamma_3		= 60  ,
	 VideoProcAmp_Gamma_4		= 80  ,
	 VideoProcAmp_Gamma_5		= 100 ,
	 VideoProcAmp_Gamma_6		= 120 , 
	 VideoProcAmp_Gamma_7		= 140 ,
	 VideoProcAmp_Gamma_8		= 160 ,
	 VideoProcAmp_Gamma_9		= 180 ,
	 VideoProcAmp_Gamma_10		= 200 ,
	 VideoProcAmp_Gamma_11		= 220 , 
	 VideoProcAmp_Gamma_12		= 240 
	 
 } VideoProcAmpGamma;

typedef  enum                             //6
{
	 VideoProcAmp_Colorenable_1     = 20  , 
	 VideoProcAmp_Colorenable_2		= 40  ,
	 VideoProcAmp_Colorenable_3		= 60  ,
	 VideoProcAmp_Colorenable_4		= 80  ,
	 VideoProcAmp_Colorenable_5		= 100 ,
	 VideoProcAmp_Colorenable_6		= 120 , 
	 VideoProcAmp_Colorenable_7		= 140 ,
	 VideoProcAmp_Colorenable_8		= 160 ,
	 VideoProcAmp_Colorenable_9		= 180 ,
	 VideoProcAmp_Colorenable_10	= 200 ,
	 VideoProcAmp_Colorenable_11	= 220 , 
	 VideoProcAmp_Colorenable_12	= 240 
	 
 } VideoProcAmpColorenable;

#define WB_AUTO                           0x0001
#define WB_FLUORESCENT                    0x0002
#define WB_DAYLIGHT                       0x0003
#define WB_CLOUDY                         0x0004
#define WB_INCANDESCENCE                  0x0005

typedef  enum                             //7
{
	
	VideoProcAmp_WB_Auto                = WB_AUTO ,
	VideoProcAmp_WB_Fluorescent			= WB_FLUORESCENT, 
	VideoProcAmp_WB_Daylight			= WB_DAYLIGHT, 
	VideoProcAmp_WB_Cloudy				= WB_CLOUDY, 
	VideoProcAmp_WB_Incandescence		= WB_INCANDESCENCE 

} VideoProcAmpWhiteBalance;

typedef  enum                             //8
{
	 VideoProcAmp_Backlight_Compensation_1      = 20  , 
	 VideoProcAmp_Backlight_Compensation_2		= 40  ,
	 VideoProcAmp_Backlight_Compensation_3		= 60  ,
	 VideoProcAmp_Backlight_Compensation_4		= 80  ,
	 VideoProcAmp_Backlight_Compensation_5		= 100 ,
	 VideoProcAmp_Backlight_Compensation_6		= 120 , 
	 VideoProcAmp_Backlight_Compensation_7		= 140 ,
	 VideoProcAmp_Backlight_Compensation_8		= 160 ,
	 VideoProcAmp_Backlight_Compensation_9		= 180 ,
	 VideoProcAmp_Backlight_Compensation_10		= 200 ,
	 VideoProcAmp_Backlight_Compensation_11		= 220 , 
	 VideoProcAmp_Backlight_Compensation_12		= 240 
	 
 } VideoProcAmpBacklight_Compensation;

typedef  enum                             //9
{
	 VideoProcAmp_Gain_1        = 20  , 
	 VideoProcAmp_Gain_2		= 40  ,
	 VideoProcAmp_Gain_3		= 60  ,
	 VideoProcAmp_Gain_4		= 80  ,
	 VideoProcAmp_Gain_5		= 100 ,
	 VideoProcAmp_Gain_6		= 120 , 
	 VideoProcAmp_Gain_7		= 140 ,
	 VideoProcAmp_Gain_8		= 160 ,
	 VideoProcAmp_Gain_9		= 180 ,
	 VideoProcAmp_Gain_10		= 200 ,
	 VideoProcAmp_Gain_11		= 220 , 
	 VideoProcAmp_Gain_12		= 240 
	 
 } VideoProcAmpGain;

typedef  enum                           //0      +     9
{
	 CameraControl_Pan_1          = 0x0000 ,
	 CameraControl_Pan_2          = 0x0000 ,
	 CameraControl_Pan_3          = 0x0000 ,
	 CameraControl_Pan_4          = 0x0000 ,
	 CameraControl_Pan_5          = 0x0000 ,
	 CameraControl_Pan_6          = 0x0000 ,
	 CameraControl_Pan_7          = 0x0000 ,
	 CameraControl_Pan_8          = 0x0000 ,
	 CameraControl_Pan_9          = 0x0000 ,
	 CameraControl_Pan_10         = 0x0000 ,
	 CameraControl_Pan_11         = 0x0000 ,
	 CameraControl_Pan_12         = 0x0000
	 
} CameraControlPan;

typedef  enum                           //1      +     9
{
	 CameraControl_Tilt_1          = 0x0000 ,
	 CameraControl_Tilt_2          = 0x0000 ,
	 CameraControl_Tilt_3          = 0x0000 ,
	 CameraControl_Tilt_4          = 0x0000 ,
	 CameraControl_Tilt_5          = 0x0000 ,
	 CameraControl_Tilt_6          = 0x0000 ,
	 CameraControl_Tilt_7          = 0x0000 ,
	 CameraControl_Tilt_8          = 0x0000 ,
	 CameraControl_Tilt_9          = 0x0000 , 
	 CameraControl_Tilt_10         = 0x0000 ,
	 CameraControl_Tilt_11         = 0x0000 ,
	 CameraControl_Tilt_12         = 0x0000
	 
} CameraControlTilt;

typedef  enum                           //2      +     9
{
	 CameraControl_Roll_1          = 0x0000 ,
	 CameraControl_Roll_2          = 0x0000 ,
	 CameraControl_Roll_3          = 0x0000 ,
	 CameraControl_Roll_4          = 0x0000 ,
	 CameraControl_Roll_5          = 0x0000 ,
	 CameraControl_Roll_6          = 0x0000 ,
	 CameraControl_Roll_7          = 0x0000 ,
	 CameraControl_Roll_8          = 0x0000 ,
	 CameraControl_Roll_9          = 0x0000 , 
	 CameraControl_Roll_10         = 0x0000 ,
	 CameraControl_Roll_11         = 0x0000 ,
	 CameraControl_Roll_12         = 0x0000
	 
} CameraControlRoll;

typedef  enum                           //3      +     9
{
	 CameraControl_Zoom_1X       =  1 ,
	 CameraControl_Zoom_2X       =  2 ,
	 CameraControl_Zoom_3X       =  3 ,
	 CameraControl_Zoom_4X       =  4 ,
	 CameraControl_Zoom_5X       =  5 ,
	 CameraControl_Zoom_6X       =  6 ,
	 CameraControl_Zoom_7X       =  7 ,
	 CameraControl_Zoom_8X       =  8 ,
	 CameraControl_Zoom_9X       =  9 ,
	 CameraControl_Zoom_10X      = 10 
	
} CameraControlZoom;

typedef  enum                           //4      +     9
{
	 CameraControl_Iris_1          = 0x0000 ,
	 CameraControl_Iris_2          = 0x0000 ,
	 CameraControl_Iris_3          = 0x0000 ,
	 CameraControl_Iris_4          = 0x0000 ,
	 CameraControl_Iris_5          = 0x0000 ,
	 CameraControl_Iris_6          = 0x0000 ,
	 CameraControl_Iris_7          = 0x0000 ,
	 CameraControl_Iris_8          = 0x0000 ,
	 CameraControl_Iris_9          = 0x0000 ,
	 CameraControl_Iris_10         = 0x0000 ,
	 CameraControl_Iris_11         = 0x0000 ,
	 CameraControl_Iris_12         = 0x0000
	 
} CameraControlIris;

typedef  enum                           //5      +     9
{
	 CameraControl_Exposure_1          = 0x0000 ,
	 CameraControl_Exposure_2          = 0x0000 ,
	 CameraControl_Exposure_3          = 0x0000 ,
	 CameraControl_Exposure_4          = 0x0000 ,
	 CameraControl_Exposure_5          = 0x0000 ,
	 CameraControl_Exposure_6          = 0x0000 ,
	 CameraControl_Exposure_7          = 0x0000 ,
	 CameraControl_Exposure_8          = 0x0000 ,
	 CameraControl_Exposure_9          = 0x0000 ,
	 CameraControl_Exposure_10         = 0x0000 ,
	 CameraControl_Exposure_11         = 0x0000 ,
	 CameraControl_Exposure_12         = 0x0000
	 
} CameraControlExposure;

typedef  enum                           //6      +     9
{
	 CameraControl_Focus_1          = 0x0000 ,
	 CameraControl_Focus_2          = 0x0000 ,
	 CameraControl_Focus_3          = 0x0000 ,
	 CameraControl_Focus_4          = 0x0000 ,
	 CameraControl_Focus_5          = 0x0000 ,
	 CameraControl_Focus_6          = 0x0000 ,
	 CameraControl_Focus_7          = 0x0000 ,
	 CameraControl_Focus_8          = 0x0000 ,
	 CameraControl_Focus_9          = 0x0000 ,
	 CameraControl_Focus_10         = 0x0000 ,
	 CameraControl_Focus_11         = 0x0000 ,
	 CameraControl_Focus_12         = 0x0000
	 
} CameraControlFocus;


#define FLASH_OFF                           0x0000
#define FLASH_ON                            0x0001


typedef  enum                           //7      +     9                           
{
	
	CameraControl_FlashOff        = FLASH_OFF , 
	CameraControl_FlashOn		  = FLASH_ON
	 
} CameraControlFlash;





#define CAMERA_CONTROL_EFFECT                         51
#define CAMERA_CONTROL_SAVEFORMAT                     54
#define CAMERA_CONTROL_EXIT                           55
#define CAMERA_CONTROL_AUTOFOCUS                      56
#define CAMERA_CONTROL_BROWSE                         57
#define CAMERA_CONTROL_MANUALFOCUS                    58
#define CAMERA_CONTROL_WINDOWFOCUS                    59
#define CAMERA_CONTROL_BURSTCOUNT                     60



 //over defined camera control effects
 
#define EFFECT_NORMAL                                0x0000
#define EFFECT_MONOCHROME                            0x0001
#define EFFECT_NEGATIVE                              0x0002
#define EFFECT_SEPIA                                 0x0003
#define EFFECT_SOLARISATION                          0x0004
#define EFFECT_UVSLOARISATION                        0x0005


#define CameraControl_Effect                    CAMERA_CONTROL_EFFECT
typedef  enum
{
	 
	 CameraControl_Effect_Normal               = EFFECT_NORMAL,
	 CameraControl_Effect_MonoChrome           = EFFECT_MONOCHROME,
	 CameraControl_Effect_Negative             = EFFECT_NEGATIVE,
	 CameraControl_Effect_Sepia                = EFFECT_SEPIA,
	 CameraControl_Effect_Solarisation         = EFFECT_SOLARISATION,
	 CameraControl_Effect_UVSolarisation       = EFFECT_UVSLOARISATION

} CameraControlEffect;







#define CameraControl_Exit          		CAMERA_CONTROL_EXIT
#define CameraControl_Browse          		CAMERA_CONTROL_BROWSE
#define CameraControl_AutoFocus          	CAMERA_CONTROL_AUTOFOCUS
#define CameraControl_ManualFocus          	CAMERA_CONTROL_MANUALFOCUS
#define CameraControl_WindowFocus          	CAMERA_CONTROL_WINDOWFOCUS
#define CameraControl_BurstCount            CAMERA_CONTROL_BURSTCOUNT
typedef  enum
{
	 
	 CameraControl_WindowFocus_0              = 0,
	 CameraControl_WindowFocus_1              = 1

} CameraControlWindowFocus;




