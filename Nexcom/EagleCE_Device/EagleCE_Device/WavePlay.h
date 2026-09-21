#ifndef __WAVE_PLAY_H__
#define __WAVE_PLAY_H__

#include "../../Template/MFSCtrlThread.h"

class CWavePlay : public CMFSCtrlThread
{
public:
	CWavePlay();
	~CWavePlay();

public:
	BOOL	Play(CString strFileName);
	BOOL	Stop();
	BOOL	IsPlaying(int nWaitMs=0);
	BOOL	WaitToEndOfPlay();

	BOOL	VolumeUp();
	BOOL	VolumeDown();
	BOOL	SetVolume(int nLevel);
	int		GetVolume();

private:
	void	CleanUp();

private:
#ifdef _WIN32_WCE
	HWAVEOUT		m_hWaveDevice;
	LPWAVEFORMATEX	m_pWfx;
	PBYTE			m_pBufferBits;
	WAVEHDR			m_hdr;
#else
	// Win32: wave playback is simulated, so no multimedia handles are kept.
	PBYTE			m_pBufferBits;
	int				m_nSimVolumeLevel;	// volume remembered by the simulator
#endif

	CMFSEvent		m_eEventDone;
	CMFSEvent		m_ePlayDone;

	BOOL			m_bStopPlaying;
	BOOL			m_bExit;
	BOOL			m_bPlay;

	DWORD			m_dwDuration;

	/* THREAD */
protected:
	unsigned ThreadHandlerProc(void);	// 스레드의 Handler.
};

#endif //__WAVE_PLAY_H__