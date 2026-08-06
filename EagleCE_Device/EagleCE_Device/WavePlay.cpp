#include "stdafx.h"
#include "WavePlay.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"

#ifdef _WIN32_WCE
// The whole waveOutXxx implementation below is WinCE only.


//#define	MAX_VOLUME_LEVEL		10		// Common.h로 이동

// wave file added

#define MRCHECK(r,str,label)\
	if ((r != MMSYSERR_NOERROR)) { RETAILMSG(1, (TEXT(#str) TEXT(" failed. mr=%08x\r\n"), r)); mr = r; goto label;}

typedef struct
{
	DWORD   dwRiff;     // Type of file header.
	DWORD   dwSize;     // Size of file header.
	DWORD   dwWave;     // Type of wave.
} RIFF_FILEHEADER, *PRIFF_FILEHEADER;

// -----------------------------------------------------------------------------
//                              ChunkHeader
// -----------------------------------------------------------------------------
typedef struct
{
	DWORD   dwCKID;        // Type Identification for current chunk header.
	DWORD   dwSize;        // Size of current chunk header.
} RIFF_CHUNKHEADER, *PRIFF_CHUNKHEADER;

/*  Chunk Types  
*/
#define RIFF_FILE       mmioFOURCC('R','I','F','F')
#define RIFF_WAVE       mmioFOURCC('W','A','V','E')
#define RIFF_FORMAT     mmioFOURCC('f','m','t',' ')
#define RIFF_CHANNEL    mmioFOURCC('d','a','t','a')

BOOL ReadChunk(HANDLE fh, DWORD dwChunkType, PVOID * ppBuffer, DWORD * pdwSize, PDWORD pdwBytesLeft)
{
	DWORD dwBytesRead;
	PVOID pBuffer;
	RIFF_CHUNKHEADER Chunk;

	if ((!pdwBytesLeft) || (*pdwBytesLeft <= 0) || (!pdwSize) || (!ppBuffer)) 
	{
		LOG(Error, _T("Invalid parameter to ReadChunk()"));
		return FALSE;
	}

	// now scan for the format chunk
	while (*pdwBytesLeft > 0) 
	{
		// now read the wave header (or what we hope is the wave header)
		if (! ReadFile(fh, &Chunk, sizeof(Chunk), &dwBytesRead, NULL) || dwBytesRead < sizeof(Chunk)) 
		{
			RETAILMSG(1, (TEXT("Error reading chunk header\n")));
			return FALSE;
		}
		
		*pdwBytesLeft -= dwBytesRead;
		
		//RETAILMSG(1, (TEXT("Chunk: \"%c%c%c%c\" size=0x%08x\r\n"), 
		//	(Chunk.dwCKID >>  0) & 0xff, 
		//	(Chunk.dwCKID >>  8) & 0xff, 
		//	(Chunk.dwCKID >> 16) & 0xff, 
		//	(Chunk.dwCKID >> 24) & 0xff, 
		//	Chunk.dwSize));

		if (Chunk.dwCKID == dwChunkType) 
		{
			// found the desired chunk
			break;
		}

		// skip the data we don't know or care about...
		if (0xFFFFFFFF == SetFilePointer (fh, Chunk.dwSize, NULL, FILE_CURRENT)) 
		{
			LOG(Error,  _T("Error setting file pointer while scanning for chunk"));
			return FALSE;
		}

		*pdwBytesLeft -= Chunk.dwSize;
	}

	// found the desired chunk.
	// allocate a buffer and read in the data
	pBuffer = new BYTE[Chunk.dwSize];

	if (pBuffer == NULL) 
	{
		LOG(Error, _T("Unable to allocate chunk buffer"));
		return FALSE;
	}

	memset(pBuffer, 0, Chunk.dwSize);	// SKKim 초기화 로직 추가
	
	if (! ReadFile(fh, pBuffer, Chunk.dwSize, &dwBytesRead, NULL) || dwBytesRead < Chunk.dwSize) 
	{
		delete [] pBuffer;
		LOG(Error, _T("Unable to read chunk data"));
		return FALSE;
	}

	*pdwBytesLeft -= dwBytesRead;
	*ppBuffer = pBuffer;
	*pdwSize = Chunk.dwSize;

	return TRUE;
}

MMRESULT ReadWaveFile(LPCTSTR pszFilename, PWAVEFORMATEX * ppWFX, PDWORD pdwBufferSize, PBYTE * ppBufferBits)
{ 
	RIFF_FILEHEADER FileHeader;
	DWORD dwBytesRead;
	DWORD dwBufferSize;
	DWORD dwFormatSize;
	PBYTE pBufferBits = NULL;
	PWAVEFORMATEX pwfx = NULL;
	DWORD dwBytesInChunk;
	HANDLE fh;
	MMRESULT mmRet = MMSYSERR_ERROR;

	fh = CreateFile(pszFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	
	if( fh == INVALID_HANDLE_VALUE ) 
	{
		LOG(Error, _T("Error opening %s. Error code = 0x%08x"), pszFilename, GetLastError());
		return mmRet;
	}

	// Read file and determine sound format
	// Start with RIFF header:

	if (! ReadFile(fh, &FileHeader, sizeof(FileHeader), &dwBytesRead, NULL) || dwBytesRead < sizeof(FileHeader)) 
	{
		LOG(Error, _T("Error reading file header"));
		goto ERROR_EXIT;
	}

	if ( FileHeader.dwRiff != RIFF_FILE || FileHeader.dwWave != RIFF_WAVE) 
	{
		LOG(Error, _T("Invalid wave file header"));
		goto ERROR_EXIT;
	}

	dwBytesInChunk = FileHeader.dwSize;

	// load the wave format
	if (! ReadChunk(fh, RIFF_FORMAT, (PVOID*) &pwfx, &dwFormatSize, &dwBytesInChunk)) 
	{
		LOG(Error, _T("Unable to read format chunk"));
		goto ERROR_EXIT;
	}
	
	if (dwFormatSize < sizeof(PCMWAVEFORMAT)) 
	{
		LOG(Error, _T("Format record too small"));
		goto ERROR_EXIT;
	}

	// load the wave data
	if (! ReadChunk(fh, RIFF_CHANNEL, (PVOID*) &pBufferBits, &dwBufferSize, &dwBytesInChunk)) 
	{
		LOG(Error, _T("Unable to read format chunk"));
		goto ERROR_EXIT;
	}

	*ppWFX = pwfx;
	*pdwBufferSize = dwBufferSize;
	*ppBufferBits = pBufferBits;

	// Success
	mmRet = MMSYSERR_NOERROR;
	goto EXIT;

ERROR_EXIT:
	delete [] pBufferBits;
	delete [] pwfx;

EXIT:
	CloseHandle(fh);

	return mmRet;   
}



CWavePlay::CWavePlay() : m_eEventDone(FALSE, FALSE), m_ePlayDone(TRUE, TRUE)
{
	m_hWaveDevice = NULL;
	m_pWfx = NULL;
	m_bStopPlaying = TRUE;
	m_bPlay = FALSE;

}

CWavePlay::~CWavePlay()
{
	m_bExit = TRUE;
	
	if (m_hWaveDevice != NULL)
	{
		waveOutClose(m_hWaveDevice);
		m_hWaveDevice = NULL;
	}
}

void CWavePlay::CleanUp()
{
	int			i;
	MMRESULT	mmRtn;

	if (m_hWaveDevice != NULL)
	{
		// close handle
		waveOutClose(m_hWaveDevice);
		m_hWaveDevice = NULL;

		m_bStopPlaying = TRUE;
		m_ePlayDone.Set();
	}

	if (m_pWfx != NULL)
	{
		delete [] m_pWfx;
		m_pWfx = NULL;
	}

	if (m_pBufferBits != NULL)
	{
		delete [] m_pBufferBits;
		m_pBufferBits = NULL;
	}

	m_bPlay = FALSE;
}

BOOL CWavePlay::Play(CString strFileName)
{
	MMRESULT mr;
	DWORD dwBufferSize;
	BOOL bResult = FALSE;

	CAutoLock	Sync(m_eMethodUse);

	m_pWfx = NULL;
	m_pBufferBits = NULL;
	mr = ReadWaveFile(strFileName,&m_pWfx,&dwBufferSize,&m_pBufferBits);

	if (mr == MMSYSERR_NOERROR)
	{
		// Note: Cast to UINT64 below is to avoid potential DWORD overflow for large (>~4MB) files.
		m_dwDuration = 0;
		m_dwDuration = (DWORD)(((UINT64)dwBufferSize) * 1000 / m_pWfx->nAvgBytesPerSec);

		m_hWaveDevice = NULL;
		mr = waveOutOpen(&m_hWaveDevice, WAVE_MAPPER, m_pWfx, (DWORD)m_eEventDone.GetHandle(), NULL, CALLBACK_EVENT);

		if (mr == MMSYSERR_NOERROR)
		{
			memset(&m_hdr, 0, sizeof(m_hdr));
			m_hdr.dwBufferLength = dwBufferSize;
			m_hdr.lpData = (char *) m_pBufferBits;

			mr = waveOutPrepareHeader(m_hWaveDevice, &m_hdr, sizeof(m_hdr));

			if (mr == MMSYSERR_NOERROR)
			{
				mr = waveOutWrite(m_hWaveDevice, &m_hdr, sizeof(m_hdr));
				
				if (mr == MMSYSERR_NOERROR)
				{
					bResult = TRUE;
				}
			}
		}

	}
	else
	{
		// Error Logging
		LOG(Info, _T("ReadWaveFile - Error (%s) (%d)"), strFileName, mr);
	}

	if (bResult == TRUE)
	{
		// Play Start
		m_bStopPlaying = FALSE;
		m_ePlayDone.Reset();
		m_bPlay = TRUE;
	}
	else
	{
		CleanUp();
	}

	return bResult;
}

BOOL CWavePlay::Stop()
{
	MMRESULT	mmRtn;

	CAutoLock	Sync(m_eMethodUse);

	if (m_bPlay == TRUE)
	{
		m_bStopPlaying = TRUE;

		mmRtn = waveOutReset(m_hWaveDevice);

		if (mmRtn != MMSYSERR_NOERROR)
		{
			LOG(Error, _T("waveOutOpen failed with return code [%d]"), mmRtn);
		}

		m_ePlayDone.Wait(INFINITE);

		CleanUp();
	}

	return TRUE;
}

BOOL CWavePlay::IsPlaying(int nWaitMs /*=0*/)
{
	if (m_ePlayDone.Wait(nWaitMs) == WAIT_OBJECT_0)
		return FALSE;

	return TRUE;
}

BOOL CWavePlay::WaitToEndOfPlay()
{
	if (m_ePlayDone.Wait(INFINITE) == WAIT_OBJECT_0)
		return TRUE;

	return FALSE;	
}

BOOL CWavePlay::VolumeUp()
{
	return SetVolume(GetVolume()+1);
}

BOOL CWavePlay::VolumeDown()
{
	return SetVolume(GetVolume()-1);
}

BOOL CWavePlay::SetVolume(int nLevel)
{
	CAutoLock	Sync(m_eMethodUse);

	DWORD	dwVolumeValue = 0;

	// 스피커가 양쪽으로 연결될 경우에는 0 ~ 0xFFFF 영역에서 10단계로 제어
	if (nLevel < MIN_VOLUME_LEVEL || nLevel > (MAX_VOLUME_LEVEL))
	{
		LOG(Error, _T("Set Volume level is invalid value. (%d)"), nLevel);
		return FALSE;
	}

	dwVolumeValue = (DWORD)(0xFFFF / MAX_VOLUME_LEVEL * nLevel);

	LOG(Info, _T("SetVolume - Level : %d, Value :0x%X"), nLevel, dwVolumeValue);

	if (waveOutSetVolume(0, MAKELONG(dwVolumeValue, dwVolumeValue)) != MMSYSERR_NOERROR)
	{
		LOG(Error, _T("waveOutSetVolume failed. level(%d)"), nLevel);
		return FALSE;
	}

	return TRUE;
}

int CWavePlay::GetVolume()
{
	CAutoLock	Sync(m_eMethodUse);

	// 스피커가 양쪽으로 연결될 경우에는 0 ~ 0xFFFF 영역에서 10단계로 제어
	int		nLevel;
	DWORD	dwVolumeValue;

	if (waveOutGetVolume(0, &dwVolumeValue) != MMSYSERR_NOERROR)
	{
		LOG(Error, _T("waveOutGetVolume failed. so level is -10"));
		return -10;
	}

	dwVolumeValue = dwVolumeValue & 0xFFFF;
	nLevel = (int)(dwVolumeValue / (0xFFFF / MAX_VOLUME_LEVEL));

	return nLevel;
}

unsigned CWavePlay::ThreadHandlerProc(void)
{
	int			i = 0;
	MMRESULT	mmRtn;
	DWORD		dwReadBytes = 0;

	SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);

	m_bExit = FALSE;

	while (m_bExit == FALSE)
	{
		if (m_eEventDone.Wait(500) == WAIT_OBJECT_0)
		{
			if (m_bStopPlaying == FALSE && m_bPlay == TRUE)
			{
				if (m_hdr.dwFlags & WHDR_DONE)
				{
					m_hdr.dwFlags = 0;

					// Wait 1 sec for testing
					CUtil::Sleep_Wait(1000);

					mmRtn = waveOutUnprepareHeader(m_hWaveDevice, &m_hdr, sizeof(m_hdr));

					//if (mmRtn != MMSYSERR_NOERROR)
					//{
					//	LOG(Error, _T("[ThreadHandlerProc] 1 waveOutUnprepareHeader failed with return code [%d]."), mmRtn);
					//}

					m_bStopPlaying = TRUE;
				}
			}

			if (m_bStopPlaying == TRUE)
			{
				CleanUp();
			}
		}
	}

	return 0;
}

#else	// !_WIN32_WCE

// ---------------------------------------------------------------------------
// WAVE SIMULATOR (Win32)
//
// Audio playback is not exercised on the desktop, so CWavePlay reports success
// without touching the multimedia API. Playback completes immediately, which
// keeps callers that wait on IsPlaying()/WaitToEndOfPlay() from blocking.
// ---------------------------------------------------------------------------

CWavePlay::CWavePlay() : m_eEventDone(FALSE, FALSE), m_ePlayDone(TRUE, TRUE)
{
	m_pBufferBits		= NULL;
	m_bStopPlaying		= TRUE;
	m_bPlay				= FALSE;
	m_bExit				= FALSE;
	m_dwDuration		= 0;
	m_nSimVolumeLevel	= MAX_VOLUME_LEVEL;
}

CWavePlay::~CWavePlay()
{
	m_bExit = TRUE;
}

void CWavePlay::CleanUp()
{
	if (m_pBufferBits != NULL)
	{
		delete [] m_pBufferBits;
		m_pBufferBits = NULL;
	}

	m_bStopPlaying = TRUE;
	m_bPlay = FALSE;
	m_ePlayDone.Set();
}

BOOL CWavePlay::Play(CString strFileName)
{
	LOG(Info, _T("[WaveSim] Play - %s (simulated)"), strFileName);

	// Report the sound as finished right away so nothing waits on it.
	m_bPlay = FALSE;
	m_bStopPlaying = TRUE;
	m_ePlayDone.Set();

	return TRUE;
}

BOOL CWavePlay::Stop()
{
	m_bStopPlaying = TRUE;
	m_bPlay = FALSE;
	m_ePlayDone.Set();

	return TRUE;
}

BOOL CWavePlay::IsPlaying(int nWaitMs /*=0*/)
{
	if (m_ePlayDone.Wait(nWaitMs) == WAIT_OBJECT_0)
		return FALSE;

	return TRUE;
}

BOOL CWavePlay::WaitToEndOfPlay()
{
	if (m_ePlayDone.Wait(INFINITE) == WAIT_OBJECT_0)
		return TRUE;

	return FALSE;
}

BOOL CWavePlay::VolumeUp()
{
	return SetVolume(GetVolume()+1);
}

BOOL CWavePlay::VolumeDown()
{
	return SetVolume(GetVolume()-1);
}

BOOL CWavePlay::SetVolume(int nLevel)
{
	CAutoLock	Sync(m_eMethodUse);

	if (nLevel < MIN_VOLUME_LEVEL || nLevel > (MAX_VOLUME_LEVEL))
	{
		LOG(Error, _T("Set Volume level is invalid value. (%d)"), nLevel);
		return FALSE;
	}

	m_nSimVolumeLevel = nLevel;

	return TRUE;
}

int CWavePlay::GetVolume()
{
	CAutoLock	Sync(m_eMethodUse);

	return m_nSimVolumeLevel;
}

unsigned CWavePlay::ThreadHandlerProc(void)
{
	SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);

	m_bExit = FALSE;

	while (m_bExit == FALSE)
	{
		// Nothing to service: playback finishes synchronously in Play().
		m_eEventDone.Wait(500);
	}

	return 0;
}

#endif	// _WIN32_WCE
