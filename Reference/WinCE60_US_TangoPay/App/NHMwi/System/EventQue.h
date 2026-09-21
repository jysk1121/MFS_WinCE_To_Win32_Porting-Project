#if !defined(AFX_EVENTQUE_H_EDITED_BY_XXX_IN_NAUTILUS_HYOSUNG__INCLUDED_)
#define AFX_EVENTQUE_H_EDITED_BY_XXX_IN_NAUTILUS_HYOSUNG__INCLUDED_
//////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(1)
	typedef struct _tag_EventData
	{
		CString		strEventType;					// Event Type
		CString		strQueOwner;					// Event Queue Owner Name							
		CString		strEventName;					// Device Event : Name
		CString		strEventLastCmd;				// Device Event : Last Command 
		CString		strDataName;					// Data   Event : Data Name
		CString		strDataValue;					// Data   Event : Data Value
	}EVENTDATA, *LPEVENTDATA;
#pragma pack()
	
class CEventQue										// 2002.12.28 전면 개보수
{
// Constructor
public:
	CEventQue();

// Attributes
public:

private:
	CPtrList	m_EventQue;
	CString		m_strOwnerName;

// Functions
public:
	void		Init(LPCSTR szOwnerName = "NAUTILUS");
	void		ResetQue();										// Que를 초기화한다.

	BOOL		IsEmpty();										// 큐가 비었는지를 확인한다.
	int			GetEventCount();								// 큐에 있는 이벤트의 갯수를 리턴한다.

	BOOL		PutData(char* szType, char* szName, char* szEvent, char* szValue);
	BOOL		GetData(char* szType,  char* szName,  char* szEvent,  char* szValue);
	BOOL		CheckDeviceDataExist(char* szType,  char* szName,  char* szEvent,  char* szValue);
	BOOL		PumpingEventToError();							// 장애이벤트가 올때까지 큐를 펌핑한다.

private:

// Destructor
public:
	virtual ~CEventQue();


};

//////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_EVENTQUE_H_EDITED_BY_XXX_IN_NAUTILUS_HYOSUNG__INCLUDED_)
