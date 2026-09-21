#pragma once

template <typename T1 , typename T2>
class CCopyList : public CList<T1,T2>
{
public:

	CCopyList(){}

	CCopyList (const CCopyList& rhs) { Copy(rhs); }

	CCopyList& operator = (const CCopyList& rhs) 
	{
		if (this != &rhs)
		{
			Copy(rhs);
		}

		return *this;
	}

	~CCopyList(){}

private:

	void Copy(const CCopyList& rhs)
	{
		RemoveAll();

		POSITION pos = rhs.GetHeadPosition();
		while (pos != NULL)
		{
			T1 obj = rhs.GetNext(pos);
			AddTail( obj );
		}
	}
};