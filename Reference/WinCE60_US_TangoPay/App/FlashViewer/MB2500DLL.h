#ifndef __MB2500DLL_API_H__
#define __MB2500DLL_API_H__


////////////////////////////////////////
// Function Type Define
typedef void	(VOID_FUNCTION_VOID)(void);
typedef void	(VOID_FUNCTION_BYTE_BYTE)(BYTE, BYTE);
typedef void	(VOID_FUNCTION_LPBYTE_BYTE)(LPBYTE, BYTE);
typedef void	(VOID_FUNCTION_LPWORD_WORD)(LPWORD, WORD);
typedef void	(VOID_FUNCTION_LPDWORD_DWORD)(LPDWORD, DWORD);
typedef void	(VOID_FUNCTION_LPVOID_LPVOID_DWORD)(LPVOID, LPVOID, DWORD);
typedef void	(VOID_FUNCTION_LPVOID_INT_DWORD)(LPVOID, int, DWORD);
typedef bool	(BOOL_FUNCTION_VOID)(void);
typedef bool	(BOOL_FUNCTION_LPVOID)(LPVOID);
typedef int		(INT_FUNCTION_UCHAR_INT_INT)(unsigned char*, int, int);
typedef BYTE	(BYTE_FUNCTION_VOID)(void);
typedef LPVOID	(LPVOID_FUNCTION_INT)(int);
typedef LPVOID	(LPVOID_FUNCTION_VOID)(void);
typedef LPVOID	(LPVOID_FUNCTION_INT_INT)(int, int);


////////////////////////////////////////
// MB2500DLL API
typedef struct _MB2500DLL_FUNCTION_
{
	// SIU Function
	BOOL_FUNCTION_VOID					*pfnSIUInit;
	BOOL_FUNCTION_VOID					*pfnSIUDeInit;
	BOOL_FUNCTION_VOID					*pfnIsSafeDoorOpen;
	BOOL_FUNCTION_VOID					*pfnIsADASwitchOn;
	BOOL_FUNCTION_VOID					*pfnIsOperatorSwitchOn;
	BOOL_FUNCTION_VOID					*pfnIsShutdownSwitchOn;
	VOID_FUNCTION_VOID					*pfnClearOperatorSwitch;
	VOID_FUNCTION_VOID					*pfnClearShutdownSwitch;
	VOID_FUNCTION_BYTE_BYTE				*pfnSetGuideLight;
	BOOL_FUNCTION_VOID					*pfnIsNearSensorOn;				// [iTM] AIREAT 2009.12.09
	BOOL_FUNCTION_VOID					*pfnIsAntiSkimmingSensorOn;		// [iTM] AIREAT 2009.12.09

	// Function Key
	BOOL_FUNCTION_VOID					*pfnFDKInit;
	BOOL_FUNCTION_VOID					*pfnFDKDeInit;
	BYTE_FUNCTION_VOID					*pfnGetFunctionKeyCode;
	VOID_FUNCTION_VOID					*pfnClearFunctionKeyBuffer;

	// System NVRAM
	LPVOID_FUNCTION_INT_INT				*pfnGetSystemAddr;
	BOOL_FUNCTION_LPVOID				*pfnFreeSystemAddr;
	
	// JNL NVRAM
	LPVOID_FUNCTION_INT					*pfnGetJNLAddr;
	BOOL_FUNCTION_LPVOID				*pfnFreeJNLAddr;

	// Log NVRAM
	INT_FUNCTION_UCHAR_INT_INT			*pfnWriteLogNVRAM;
	LPVOID_FUNCTION_INT					*pfnGetLOGAddr;
	BOOL_FUNCTION_LPVOID				*pfnFreeLOGAddr;

	// Reserved NVRAM
	LPVOID_FUNCTION_VOID				*pfnGetRsvdAddr;
	BOOL_FUNCTION_LPVOID				*pfnFreeRsvdAddr;


	// Write Synchronzation Function
	VOID_FUNCTION_LPBYTE_BYTE			*pfnNvramCopyByte;
	VOID_FUNCTION_LPWORD_WORD			*pfnNvramCopyWord;
	VOID_FUNCTION_LPDWORD_DWORD			*pfnNvramCopyDword;
	VOID_FUNCTION_LPVOID_LPVOID_DWORD	*pfnNvramCopyMemory;
	VOID_FUNCTION_LPVOID_INT_DWORD		*pfnNvramSetMemory;
	
} MB2500DLL_FUNCTION, *PMB2500DLL_FUNCTION;


////////////////////////////////////////
// class CMB2500APi
class CMB2500DLLAPi
{
public:
	CMB2500DLLAPi();
	~CMB2500DLLAPi();

public:
	// SIU Function
	bool	SIUInit(void);									// Get Safe Door Switch Status
	bool	SIUDeInit(void);								// Get Safe Door Switch Status
	bool	IsSafeDoorOpen(void);							// Get Safe Door Switch Status
	bool	IsADASwitchOn(void);							// get ADA Switch status
	bool	IsOperatorSwitchOn(void);						// get Operator Switch Status
	bool	IsShutdownSwitchOn(void);						// get Shutdown Switch status
	void	ClearOperatorSwitch(void);						// Clear Operator Switch Status
	void	ClearShutdownSwitch(void);						// Clear Shutdown Switch Status
	void	SetGuideLight(BYTE byFlicker, BYTE byOnOff);	// Set Guide Light
	bool	IsNearSensorOn(void);							// get Near Sensor status
	bool	IsAntiSkimmingSensorOn(void);					// get AntiSkimming Sensor status

	// Function Key
	bool	FDKInit(void);
	bool	FDKDeInit(void);
	BYTE	GetFunctionKeyCode(void);						// Get Function Key Input
	void	ClearFunctionKeyBuffer(void);					// Clear Function Bey Buffer

	// NVRAM
	LPVOID	GetSystemAddr(int index , int size );			//;;getLPointer for System Configuration
	bool	FreeSystemAddr(LPVOID	lpv_nvram);				// Release NV_RAM LPointer

	LPVOID	GetJNLAddr(int index);							//;;get LPointer for Journel
	bool	FreeJNLAddr(LPVOID	lpv_nvram);					// Release NV_RAM LPointer

	int		WriteLogNVRAM(unsigned char *ptrlogbuff, int size, int level);	//;;Write Log data into NV-RAM
	LPVOID	GetLOGAddr(int index);                           
   bool	FreeLOGAddr(LPVOID	lpv_nvram);

	// Reserved NVRAM, 090729---------------------------------
	LPVOID	GetRsvdAddr(void);								//;;get LPointer for Reserved NVRAM
	bool	FreeRsvdAddr(LPVOID	lpv_nvram);					// Release Reserved NVRAM LPointer
	//-------------------------------------------------------

	// Write Synchronzation Function
	void	NvramCopyByte(LPBYTE pbDest, BYTE bValue);
	void	NvramCopyWord(LPWORD pwDest, WORD wValue);
	void	NvramCopyDword(LPDWORD  pdwDest, DWORD dwValue);
	void	NvramCopyMemory(LPVOID pDest, LPVOID pSrc, DWORD dwSize);
	void	NvramSetMemory(LPVOID pDest, int c, DWORD dwSize);

private:
	HINSTANCE				m_hMB2500DLL;
	MB2500DLL_FUNCTION		m_MB2500DLL_FUNC;
};

#endif //__MB2500DLL_API_H__