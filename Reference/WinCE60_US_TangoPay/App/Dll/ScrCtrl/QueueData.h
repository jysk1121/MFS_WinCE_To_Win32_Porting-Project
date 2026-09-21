#ifndef __QUEUE_DATA_H__
#define __QUEUE_DATA_H__

class CQueueData
{
public:
	CQueueData()
	{
		m_strCmd = _T("");
		m_strData = _T("");
	};

	CQueueData(CString Cmd, CString Data)
	{
		m_strCmd = Cmd;
		m_strData = Data;
	};

	void SetData(CString strCmd, CString strData)
	{
		m_strCmd = strCmd;
		m_strData = strData;
	};

	CQueueData(const CQueueData &Msg)
	{
		m_strCmd = Msg.m_strCmd;
		m_strData = Msg.m_strData;
	};

	CQueueData& operator= (const CQueueData &Msg)
	{
		this->m_strCmd = Msg.m_strCmd;
		this->m_strData = Msg.m_strData;
		
		return *this;
	};

public:
	CString		m_strCmd;
	CString		m_strData;
};

#endif //__QUEUE_DATA_H__