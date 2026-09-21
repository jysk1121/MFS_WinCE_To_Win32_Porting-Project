#pragma once

/**
 * NOTE: This needs to be moved to a config file, so it can be edited
 * easily
 */

//
// Logs
//

#define LOGS_ROOT L"\\ATM2\\LOG"

//
// Ads
//

#define ADS_ROOT_1 L"\\ATM\\ADVERTISEMENT"
#define ADS_ROOT_2 L"\\ATM2\\Advertisement"

const int ADS_PATTERNS_MAX = 2;
const wchar_t * const ADS_PATTERNS[] = {
	L"ADV_0?.jpg",
	L"TranADV_0?.jpg",
};

//
// System Files
//


#define SYSFILE1_ROOT L"\\ATM"

const int SYSFILE1_PATTERNS_MAX = 35;
const wchar_t * const SYSFILE1_PATTERNS[] = {
	/* \\ATM */
	L"\\ATM\\*.dll",
	L"\\ATM\\*.exe",
	L"\\ATM\\*.ocx",
	L"\\ATM\\*.pem",
	L"\\ATM\\*.DWL",
	L"\\ATM\\*.ini",
	L"\\ATM\\*.dat",
	L"\\ATM\\*.idc",
	L"\\ATM\\*.wav",
	L"\\ATM\\*.bin",
	L"\\ATM\\*.ttu",
	L"\\ATM\\*.bat",
	L"\\ATM\\*.wfm",
	L"\\ATM\\*.default",
	L"\\ATM\\spr_temp.bmp",
	L"\\ATM\\TTS_DB\\*",
	L"\\ATM\\ERRORCODE\\*",
	L"\\ATM\\EMVINI\\*",

	/* \\ATM\\WAVE */
	L"\\ATM\\WAVE\\*.dat",
	L"\\ATM\\WAVE\\*.wav",

	/* \\ATM\\Data */
	L"\\ATM\\Data\\*.xml",
	L"\\ATM\\Data\\*.ini",
	L"\\ATM\\Data\\*.wav",
	L"\\ATM\\Data\\*Logo.jpg",
	L"\\ATM\\Data\\Pin4LogoSml.jpg",
	L"\\ATM\\Data\\DCC_DomesticBin.bin",
	L"\\ATM\\Data\\*.pem",
	L"\\ATM\\Data\\LibertyX.png",
	L"\\ATM\\Data\\DigitalMint.png",
	L"\\ATM\\Data\\AP_Background_Welcome_{{screen.res.w}}_{{screen.res.h}}.png",
	L"\\ATM\\Data\\*Button.png",

	/* \\ATM\\Data\\GivePay */
	L"\\ATM\\Data\\GivePay\\{{screen.res.w}}_{{screen.res.h}}\\*.png",
	L"\\ATM\\Data\\GivePay\\{{screen.res.w}}_{{screen.res.h}}\\*.jpg",
	
	/* \\ATM\\SCREEN\\ */
	L"\\ATM\\SCREEN\\*.dat",
	L"\\ATM\\SCREEN\\*.bmp",
	L"\\ATM\\SCREEN\\screen.tar"
};
