#pragma once

#include <assert.h>

//-------------------------------------------------
// Main structure of Circular Queue Class:
//-------------------------------------------------

template <class Elem>
class CircularQueue
{
public:
	CircularQueue(int MaxSize=500);
	CircularQueue(const CircularQueue<Elem> &OtherCircularQueue);
	~CircularQueue(void);

	void       Clear(void);
	void       Enqueue(const Elem &Item);
	Elem       Dequeue(void);
	inline int ElemNum(void);

protected:
	Elem     *Data;
	const int MAX_NUM;
	int       Beginning;
	int       End;

	int       ElemCount;
};

//-------------------------------------------------
// Implementation of Circular Queue Class:
//-------------------------------------------------

template <class Elem>
CircularQueue<Elem>::CircularQueue(int MaxSize) :
	MAX_NUM( MaxSize )
{
	Data      = new Elem[MAX_NUM + 1];
	Beginning = 0;
	End       = 0;
	ElemCount = 0;
}

template <class Elem>
CircularQueue<Elem>::CircularQueue(const CircularQueue &OtherCircularQueue) :
	MAX_NUM( OtherCircularQueue.MAX_NUM )
{
	Beginning = OtherCircularQueue.Beginning;
	End       = OtherCircularQueue.End;
	ElemCount = OtherCircularQueue.ElemCount;

	Data      = new Elem[MAX_NUM + 1];
	for (int i = 0; i < MAX_NUM; i++)
		Data[i] = OtherCircularQueue.Data[i];
}

template <class Elem>
CircularQueue<Elem>::~CircularQueue(void)
{
	delete[] Data;
}

template <class Elem>
void CircularQueue<Elem>::Clear(void)
{
	Beginning = 0;
	End = 0;
	ElemCount = 0;
}

template <class Elem>
void CircularQueue<Elem>::Enqueue(const Elem &Item)
{
	assert( ElemCount < MAX_NUM );

	Data[ End++ ] = Item;
	++ElemCount;

	if (End > MAX_NUM)
		End -= (MAX_NUM + 1);
}

template <class Elem>
Elem CircularQueue<Elem>::Dequeue(void)
{
	assert( ElemCount > 0 );

	Elem ReturnValue = Data[ Beginning++ ];
	--ElemCount;

	if (Beginning > MAX_NUM)
		Beginning -= (MAX_NUM + 1);

	return ReturnValue;
}

template <class Elem>
inline int CircularQueue<Elem>::ElemNum(void)
{
	return ElemCount;
}
