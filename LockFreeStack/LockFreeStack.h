#pragma once

struct LoggingStruct
{
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

	LoggingStruct LoggingPush(T InData, int InThreadID);
	LoggingStruct LoggingPop(int InThreadID);

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
inline LoggingStruct LockFreeStack<T>::LoggingPush(T InData, int InThreadID)
{
	LockFreeStackNode* t;
	LockFreeStackNode* newNode = new LockFreeStackNode();
	newNode->data = InData;
	do
	{
		t = _Top;
		newNode->next = t;
	} while (_InterlockedCompareExchange64((long long*)&_Top, (long long)newNode, (long long)t) != (long long)t);

	LoggingStruct loggingStruct;
	loggingStruct.PushPopValue = (unsigned long long)InData;
	loggingStruct.PushNode = (unsigned long long)newNode;
	loggingStruct.Top = (unsigned long long)t;
	loggingStruct.ThreadID = (unsigned long long)InThreadID;
	return loggingStruct;
}

template<typename T>
inline LoggingStruct LockFreeStack<T>::LoggingPop(int InThreadID)
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

	LoggingStruct loggingStruct;
	loggingStruct.PopNode = (unsigned long long)t;

	delete t;

	loggingStruct.PushPopValue = (unsigned long long)retData;
	loggingStruct.Top = (unsigned long long)newTop;
	loggingStruct.ThreadID = (unsigned long long)InThreadID;

	return loggingStruct;
}
