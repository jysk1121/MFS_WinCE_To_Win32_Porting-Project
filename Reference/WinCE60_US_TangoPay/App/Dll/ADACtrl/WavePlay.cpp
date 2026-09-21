#include "stdafx.h"
#include ".\Ada\WavePlay.h"

//#define NH_DEBUG
#include ".\Common\NHDbgApi.h"

#define DBG_INFO	1
#define DBG_CALL	1

#define	MAX_VOLUME_LEVEL		10

CWavePlay::CWavePlay() : m_eEventDone(FALSE, FALSE), m_ePlayDone(TRUE, TRUE)
{
	m_hWaveDevice = NULL;
	m_pWfx = NULL;
	m_bStopPlaying = TRUE;
	m_bPlay = FALSE;

	Resume();
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
	NHDEBUG(DBG_CALL, (L"CALL\n"));

	int			i;
	MMRESULT	mmRtn;

	m_WaveFile.Close();

	if (m_hWaveDevice != NULL)
	{
		// release buffer
		for (i = 0; i < MAX_WAVE_BUF; i++)
		{
			if (m_WaveBuf[i].Header.dwFlags != 0)
			{
				mmRtn = waveOutUnprepareHeader(m_hWaveDevice, &m_WaveBuf[i].Header, sizeof(m_WaveBuf[i].Header));
				if (mmRtn != MMSYSERR_NOERROR)
					NHERROR((_T("waveOutUnprepareHeader failed with return code [%d]. buffer index[%d]\n"), mmRtn, i));
			}
		}

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

	m_bPlay = FALSE;
}

BOOL CWavePlay::Play(CString strFileName)
{
	int			i;
	int			nDevNum;
	HRESULT		hResult;
	DWORD		dwBufferLength;
	DWORD		dwReadBytes;
	MMRESULT	mmRtn;
	BOOL		bRes = TRUE;

	CNHAutoLock	Sync(m_eMethodUse);

	NHDEBUG(DBG_CALL, (L"CALL : FILE[%s]\n", strFileName));

	nDevNum = waveOutGetNumDevs();
	if (nDevNum == 0)
	{
		NHERROR((_T("waveOutGetNumDevs reported zero device.\n")));
		return FALSE;
	}

	CleanUp();
	hResult = m_WaveFile.Create(strFileName, GENERIC_READ, OPEN_EXISTING, 0, (LPVOID*)&m_pWfx, NULL);
	if (hResult != ERROR_SUCCESS)
	{
		NHERROR((_T("Could not open [%s]\n"), strFileName));
		return FALSE;
	}

	// buffer size needs to be a multiple of the block alignment (tight loop in driver makes some assumptions)
	dwBufferLength = ((DWORD)((m_pWfx->nSamplesPerSec/50)+1)) * m_pWfx->nBlockAlign;
	if (dwBufferLength < m_pWfx->nBlockAlign)
		dwBufferLength = m_pWfx->nBlockAlign;

	if (dwBufferLength > WAVE_BUFFER_SIZE)
	{
		NHDEBUG(DBG_INFO, (_T("Calculation buffer size [%d]. but it's very larger than the buffer size[%d]\n"), dwBufferLength, WAVE_BUFFER_SIZE));
		dwBufferLength = WAVE_BUFFER_SIZE;
	}

	NHDEBUG(DBG_INFO, (_T("Buffer Size is [%d]\n"), dwBufferLength));

	// set infomation for the playback buffers.
	for (i = 0; i < MAX_WAVE_BUF; i++)
	{
		memset(&m_WaveBuf[i], 0, sizeof(WAVEBUF));
		
		// read data
		dwReadBytes = 0;
		m_WaveFile.ReadData(m_WaveBuf[i].Buffer, dwBufferLength, &dwReadBytes);

		if (dwReadBytes != dwBufferLength)
		{
			NHDEBUG(DBG_INFO, (_T("Read Data size[%d]. Request size[%d]\n"), dwReadBytes, dwBufferLength));
			break;
		}

		m_WaveBuf[i].Header.dwBufferLength = dwBufferLength;
		m_WaveBuf[i].Header.lpData = m_WaveBuf[i].Buffer;
		m_WaveBuf[i].nNextBufIndex = i+1;
		m_WaveBuf[i].bFilledData = true;
	}

	if (i == 0 && m_WaveBuf[0].bFilledData == false)
	{
		NHERROR((_T("Could not read a buffer's worth of data from the file\n")));
		return FALSE;
	}
	else
	{
		m_WaveBuf[i-1].nNextBufIndex = 0;
	}

	mmRtn = waveOutOpen(&m_hWaveDevice, 0, m_pWfx, (DWORD)m_eEventDone.GetHandle(), NULL, CALLBACK_EVENT);
	if (mmRtn == MMSYSERR_NOERROR)
	{
		for (i = 0; i < MAX_WAVE_BUF; i++)
		{
			if (m_WaveBuf[i].bFilledData != true)
				break;

			mmRtn = waveOutPrepareHeader(m_hWaveDevice, &m_WaveBuf[i].Header, sizeof(m_WaveBuf[i].Header));
			if (mmRtn != MMSYSERR_NOERROR)
			{
				NHERROR((_T("waveOutPrepareHeader failed with return code [%d]. buffer index[%d]\n"), mmRtn, i));
				bRes = FALSE;
				break;
			}
		}

		if (bRes == TRUE)
		{
			m_bStopPlaying = FALSE;
			m_ePlayDone.Reset();
			m_nCurBufIndex = 0;

			for (i = 0; i < MAX_WAVE_BUF; i++)
			{
				if (m_WaveBuf[i].bFilledData != true)
					break;

				mmRtn = waveOutWrite(m_hWaveDevice, &m_WaveBuf[i].Header, sizeof(m_WaveBuf[i].Header));
				if (mmRtn != MMSYSERR_NOERROR)
				{
					NHERROR((_T("waveOutWrite failed with return code [%d]. buffer index[%d]\n"), mmRtn, i));
					bRes = FALSE;
					break;
				}
			}
		}
	}
	else
	{
		NHERROR((_T("waveOutOpen failed with return code [%d]\n"), mmRtn));
		bRes = FALSE;
	}

	if (bRes == FALSE)
	{
		CleanUp();
	}
	else
	{
		m_bPlay = TRUE;
	}

	return bRes;
}

BOOL CWavePlay::Stop()
{
	MMRESULT	mmRtn;

	CNHAutoLock	Sync(m_eMethodUse);

	NHDEBUG(DBG_CALL, (L"CALL\n"));

	if (m_bPlay == TRUE)
	{
		m_bStopPlaying = TRUE;

		mmRtn = waveOutReset(m_hWaveDevice);
		if (mmRtn != MMSYSERR_NOERROR)
		{
			NHERROR((_T("waveOutReset failed with return code[%d]\n"), mmRtn));
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
	NHDEBUG(DBG_CALL, (L"CALL\n"));

	if (m_ePlayDone.Wait(INFINITE) == WAIT_OBJECT_0)
		return TRUE;

	return FALSE;	
}

BOOL CWavePlay::VolumeUp()
{
	NHDEBUG(DBG_CALL, (L"CALL\n"));

	return SetVolume(GetVolume()+1);
}

BOOL CWavePlay::VolumeDown()
{
	NHDEBUG(DBG_CALL, (L"CALL\n"));

	return SetVolume(GetVolume()-1);
}

BOOL CWavePlay::SetVolume(int nLevel)
{
	CNHAutoLock	Sync(m_eMethodUse);

	NHDEBUG(DBG_CALL, (L"CALL\n"));

	DWORD	dwVolumeValue;

	if (nLevel < 0 || nLevel > MAX_VOLUME_LEVEL)
	{
		NHERROR((_T("Set Volume level is invalid value. (%d)\n"), nLevel));
		return FALSE;
	}

	dwVolumeValue = (DWORD)(0xFFFF / MAX_VOLUME_LEVEL * nLevel);
	
	NHDEBUG(1, (_T("SetLevel: Level(%d), Value(%d)\n"), nLevel, dwVolumeValue));

	if (waveOutSetVolume(0, MAKELONG(dwVolumeValue, dwVolumeValue)) != MMSYSERR_NOERROR)
	{
		NHERROR((_T("waveOutSetVolume failed. level(%d)\n"), nLevel));
		return FALSE;
	}

	return TRUE;
}

int CWavePlay::GetVolume()
{
	CNHAutoLock	Sync(m_eMethodUse);

	NHDEBUG(DBG_CALL, (L"CALL\n"));

	int		nLevel;
	DWORD	dwVolumeValue;

	if (waveOutGetVolume(0, &dwVolumeValue) != MMSYSERR_NOERROR)
	{
		NHERROR((_T("waveOutGetVolume failed. so level is -10\n")));
		return -10;
	}

	dwVolumeValue = dwVolumeValue & 0xFFFF;
	nLevel = (int)(dwVolumeValue / (0xFFFF / MAX_VOLUME_LEVEL));

	NHDEBUG(DBG_INFO, (L"GetLevel: Level(%d), Value(%d)\n", nLevel, dwVolumeValue));
	return nLevel;
}

unsigned CWavePlay::ThreadHandlerProc(void)
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	int			i = 0;	// [#2022] NH KSK 2011.02.21 Code Sonar 지적항목 대책
	MMRESULT	mmRtn;
	DWORD		dwReadBytes = 0;	// [#2022] NH KSK 2011.02.21 Code Sonar 지적항목 대책

	SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);

	m_bExit = FALSE;

	while (m_bExit == FALSE)
	{
		if (m_eEventDone.Wait(500) == WAIT_OBJECT_0)
		{
			if (m_bStopPlaying == FALSE && m_bPlay == TRUE)
			{
				if ((m_WaveBuf[m_nCurBufIndex].bFilledData == true) &&
					(m_WaveBuf[m_nCurBufIndex].Header.dwFlags & WHDR_DONE))
				{
					// [#GLDV-2853] AU Kook 2021.03.23 waveOutUnprepareHeader() returns MMSYSERR_INVALPARAM on WEC7 when variables below are uncommented.
					//m_WaveBuf[m_nCurBufIndex].Header.dwFlags = 0;
					//m_WaveBuf[m_nCurBufIndex].bFilledData = false;

					mmRtn = waveOutUnprepareHeader(m_hWaveDevice, &m_WaveBuf[m_nCurBufIndex].Header, sizeof(m_WaveBuf[m_nCurBufIndex].Header));
					if (mmRtn == MMSYSERR_NOERROR)
					{
						m_WaveFile.ReadData(m_WaveBuf[m_nCurBufIndex].Buffer, m_WaveBuf[m_nCurBufIndex].Header.dwBufferLength, &dwReadBytes);
						if (dwReadBytes != m_WaveBuf[m_nCurBufIndex].Header.dwBufferLength)
						{
							NHDEBUG(DBG_INFO, (_T("END OF FILE\n")));
							m_bStopPlaying = TRUE;
						}
						else
						{
							m_WaveBuf[m_nCurBufIndex].bFilledData = true;
							mmRtn = waveOutPrepareHeader(m_hWaveDevice, &m_WaveBuf[m_nCurBufIndex].Header, sizeof(m_WaveBuf[m_nCurBufIndex].Header));
							if (mmRtn == MMSYSERR_NOERROR)
							{
								//m_WaveBuf[m_nCurBufIndex].Header.dwFlags &= ~WHDR_DONE;
								mmRtn = waveOutWrite(m_hWaveDevice, &m_WaveBuf[m_nCurBufIndex].Header, sizeof(m_WaveBuf[m_nCurBufIndex].Header));
								if (mmRtn != MMSYSERR_NOERROR)
								{
									NHERROR((_T("waveOutWrite failed with return code [%d]. buffer index[%d]\n"), mmRtn, i));
									m_bStopPlaying = TRUE;
									m_WaveBuf[m_nCurBufIndex].bFilledData = false;
								}
								m_nCurBufIndex = m_WaveBuf[m_nCurBufIndex].nNextBufIndex;
							}
							else
							{
								NHERROR((_T("waveOutPrepareHeader failed with return code [%d]. buffer index[%d]\n"), mmRtn, i));
								m_bStopPlaying = TRUE;
								m_WaveBuf[m_nCurBufIndex].bFilledData = false;
							}
						}
					}
					else
					{
						NHERROR((_T("waveOutUnprepareHeader failed with return code [%d]. buffer index[%d]\n"), mmRtn, i));
						NHDEBUG(DBG_INFO, (_T("END OF FILE\n")));
						m_bStopPlaying = TRUE;
					}
				}
			}

			if (m_bStopPlaying == TRUE)
			{
				int		nDone = 0xFFFFFFFF;

				for (i = 0; i < MAX_WAVE_BUF; i++)
				{
					nDone &= ~(1 << i);

					if (m_WaveBuf[i].bFilledData == true)
					{
						if (m_WaveBuf[i].Header.dwFlags & WHDR_DONE)
							nDone |= (1 << i);
					}
					else
					{
						nDone |= (1 << i);
					}
				}

				if (nDone == 0xFFFFFFFF)
					m_ePlayDone.Set();
			}
		}
	}

	return 0;
}
