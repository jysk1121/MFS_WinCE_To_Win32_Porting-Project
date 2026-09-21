#ifndef __REPOSITORY_H__
#define __REPOSITORY_H__

#define DllImport			__declspec(dllimport)
#define DllExport			__declspec(dllexport)

#ifndef _CMNLIB_OWNER_
#define DllUseport			DllImport
#pragma comment(lib, "Cmnlib.lib")
#else
#define DllUseport			DllExport
#endif

#ifndef UNDER_CE
//#  undef _T
//#  define _T(str) str

# define LPSHORT short*
#endif


// [#573] NH AIREAT 2009.10.26 SW NVRAM
//////////////////////////////////////////////////////////////////////
//	MB2500DLL.DLL API
// SIU Function
DllUseport	bool	SIUInit(void);									// Get Safe Door Switch Status
DllUseport	bool	SIUDeInit(void);								// Get Safe Door Switch Status
DllUseport	bool	IsSafeDoorOpen(void);							// Get Safe Door Switch Status
DllUseport	bool	IsSideCarDoorOpen(void);						// Get Side Car Door Switch Status
DllUseport	bool	IsADASwitchOn(void);							// get ADA Switch status
DllUseport	bool	IsOperatorSwitchOn(void);						// get Operator Switch Status
DllUseport	bool	IsShutdownSwitchOn(void);						// get Shutdown Switch status
DllUseport	void	ClearOperatorSwitch(void);						// Clear Operator Switch Status
DllUseport	void	ClearShutdownSwitch(void);						// Clear Shutdown Switch Status
DllUseport	void	SetGuideLight(BYTE byFlicker, BYTE byOnOff);	// Set Guide Light
DllUseport	void	SetHaloLedControl(BYTE byLedColor, BYTE byLedMode, BYTE byMcuControl);	// [#2205] US KSK 2013.06.28
DllUseport	bool	IsNearSensorOn(void);							// get Near Sensor Status			// [iTM] AIREAT 2009.12.09

// Anti-Skimming
DllUseport	bool	AntiSkimmingInit(void);
DllUseport	bool	AntiSkimmingDeInit(void);
DllUseport	bool	IsAntiSkimmingSensorOn(void);					// get AntiSkimming Sensor Status	// [iTM] AIREAT 2009.12.09
DllUseport	void	SetAntiSkimmingDetectTime(DWORD dwMillisec);	// set Anti-Skimming Sensor detect time

// Function Key
DllUseport	bool	FDKInit(void);
DllUseport	bool	FDKDeInit(void);
DllUseport	BYTE	GetFunctionKeyCode(void);						// Get Function Key Input
DllUseport	void	ClearFunctionKeyBuffer(void);					// Clear Function Bey Buffer

// NVRAM
DllUseport	LPVOID	GetSystemAddr(int index , int size );			//;;getLPointer for System Configuration
DllUseport	bool	FreeSystemAddr(LPVOID	lpv_nvram);				// Release NV_RAM LPointer

DllUseport	LPVOID	GetJNLAddr(int index);							//;;get LPointer for Journel
DllUseport	bool	FreeJNLAddr(LPVOID	lpv_nvram);					// Release NV_RAM LPointer

DllUseport	int		WriteLogNVRAM(unsigned char *ptrlogbuff, int size, int level);	//;;Write Log data into NV-RAM
DllUseport	LPVOID	GetLOGAddr(int index);                           
DllUseport   bool	FreeLOGAddr(LPVOID	lpv_nvram);

// Reserved NVRAM, 090729---------------------------------
DllUseport	LPVOID	GetRsvdAddr(void);								//;;get LPointer for Reserved NVRAM
DllUseport	bool	FreeRsvdAddr(LPVOID	lpv_nvram);					// Release Reserved NVRAM LPointer
//-------------------------------------------------------

// Write Synchronzation Function
DllUseport	void	NvramCopyByte(LPBYTE pbDest, BYTE bValue);
DllUseport	void	NvramCopyWord(LPWORD pwDest, WORD wValue);
DllUseport	void	NvramCopyDword(LPDWORD  pdwDest, DWORD dwValue);
DllUseport	void	NvramCopyMemory(LPVOID pDest, LPVOID pSrc, DWORD dwSize);
DllUseport	void	NvramSetMemory(LPVOID pDest, int c, DWORD dwSize);
// end of [#573]

DllUseport	void	NvramCopyShort(LPSHORT psDest, SHORT sValue);	// [#2270] AU KSK 2014.05.30

#endif //__REPOSITORY_H__