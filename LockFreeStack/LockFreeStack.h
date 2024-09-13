#pragma once

struct LoggingStruct
{
	unsigned long long Seq;
	unsigned long long PushPopValue;
	unsigned long long PushNode;
	unsigned long long PopNode;
	unsigned long long Top;
	unsigned long long ThreadID;
};

template <typename T>
class LockFreeStack
{
	struct LockFreeStackNode
	{
		T data;
		LockFreeStackNode* next;
	};
public:
	LockFreeStack();
	void Push(T InData);
	T Pop();

	void LoggingPush(T InData, int InThreadID, LoggingStruct* InLoggingStruct);
	T LoggingPop(int InThreadID, LoggingStruct* InLoggingStruct);

private:
	LockFreeStackNode* _Top;
};

template <typename T>
LockFreeStack<T>::LockFreeStack() : _Top(nullptr)
{
}

template <typename T>
void LockFreeStack<T>::Push(T InData)
{
	LockFreeStackNode* t;
	LockFreeStackNode* newNode = new LockFreeStackNode();
	newNode->data = InData;
	do
	{
		t = _Top;
		newNode->next = t;
	} while (_InterlockedCompareExchange64((long long*)&_Top, (long long)newNode, (long long)t) != (long long)t);
}

template <typename T>
T LockFreeStack<T>::Pop()
{
	LockFreeStackNode* t;
	LockFreeStackNode* newTop;
	T retData;
	char e = 'E';
	do
	{
		t = _Top;
		if (t == nullptr)
			throw e;

		newTop = t->next;
	} while ((LockFreeStackNode*)_InterlockedCompareExchange64((long long*)&_Top, (long long)newTop, (long long)t)
		!= t);

	retData = t->data;

	///** t가 현재 thread에 의해 꺼내어진 상황이므로 Pop 수행 후 아래의 조건문을 수행하는 사이에 다른 thread에 의해 t의 데이터가 바뀔 일은 없다. */
	//if (newTop != t->next)
	//{
	//	// ABA
	//	__debugbreak();
	//}

	delete t;

	return retData;
}

template<typename T>
inline void LockFreeStack<T>::LoggingPush(T InData, int InThreadID, LoggingStruct* InLoggingStruct)
{
	LockFreeStackNode* t;
	LockFreeStackNode* newNode = new LockFreeStackNode();
	newNode->data = InData;
	do
	{
		t = _Top;
		newNode->next = t;
	} while (_InterlockedCompareExchange64((long long*)&_Top, (long long)newNode, (long long)t) != (long long)t);

	InLoggingStruct->PushPopValue = (unsigned long long)InData;
	InLoggingStruct->PushNode = (unsigned long long)newNode;
	InLoggingStruct->Top = (unsigned long long)t;
	InLoggingStruct->ThreadID = (unsigned long long)InThreadID;
}

template<typename T>
inline T LockFreeStack<T>::LoggingPop(int InThreadID, LoggingStruct* InLoggingStruct)
{
	LockFreeStackNode* t;
	LockFreeStackNode* newTop;
	T retData;
	char e = 'E';
	do
	{
		t = _Top;
		if (t == nullptr)
			throw e;

		newTop = t->next;
	} while ((LockFreeStackNode*)_InterlockedCompareExchange64((long long*)&_Top, (long long)newTop, (long long)t)
		!= t);

	retData = t->data;
	delete t;

	InLoggingStruct->PushPopValue = (unsigned long long)retData;
	InLoggingStruct->PopNode = (unsigned long long)t;
	InLoggingStruct->Top = (unsigned long long)newTop;
	InLoggingStruct->ThreadID = (unsigned long long)InThreadID;

	return retData;
}
