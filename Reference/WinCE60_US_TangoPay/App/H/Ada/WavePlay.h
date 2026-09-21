#ifndef __WAVE_PLAY_H__
#define __WAVE_PLAY_H__

#include ".\Ada\WaveFile.h"
#include ".\Common\NHCtrlThread.h"

#define	MAX_WAVE_BUF			12
#define WAVE_BUFFER_SIZE		256

typedef struct
{
	bool	bFilledData;
	WAVEHDR	Header;
	char	Buffer[WAVE_BUFFER_SIZE];
	int		nNextBufIndex;
}WAVEBUF, *LPWAVEBUF;

class AFX_CLASS_EXPORT CWavePlay : public CNHCtrlThread
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
	CWaveFile		m_WaveFile;
	HWAVEOUT		m_hWaveDevice;
	LPWAVEFORMATEX	m_pWfx;
	WAVEBUF			m_WaveBuf[MAX_WAVE_BUF];

	CNHEvent		m_eEventDone;
	CNHEvent		m_ePlayDone;

	BOOL			m_bStopPlaying;
	BOOL			m_bExit;
	BOOL			m_bPlay;

	int				m_nCurBufIndex;

	/* THREAD */
protected:
	unsigned ThreadHandlerProc(void);	// 스레드의 Handler.
};

#endif //__WAVE_PLAY_H__