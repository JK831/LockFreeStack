// LockFreeStack.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <process.h>

#define NUM_NODE 200000
#define NUM_WORKER 10

using NodeData = int;

struct Node
{
    NodeData data;
    Node* next;
};

class LockFreeStack
{
public:
    LockFreeStack();
    void Push(Node* InNode);
    Node* Pop();

private:
    Node* _Top;
};

Node* g_Nodes1 = nullptr;
LockFreeStack g_Stack;

unsigned int WorkerThread1(void* pvParam)
{
    while (true)
    {
        for (int iCnt = 0; iCnt < NUM_NODE; ++iCnt)
        {
            g_Stack.Push(&g_Nodes1[iCnt]);
        }
		for (int iCnt = 0; iCnt < NUM_NODE; ++iCnt)
		{
			g_Stack.Pop();
		}
    }
}

unsigned int WorkerThread2(void* pvParam)
{
	while (true)
	{
		Node* topNode = g_Stack.Pop();
        g_Stack.Push(topNode);
	}
}

int main()
{
    g_Nodes1 = new Node[NUM_NODE];
    HANDLE workerThreads[NUM_WORKER] = { 0, };

    for (int iCnt = 0; iCnt < NUM_NODE; ++iCnt)
    {
        g_Nodes1[iCnt].data = iCnt;
        g_Stack.Push(&g_Nodes1[iCnt]);
    }
    
    for (int iCnt = 0; iCnt < NUM_WORKER; ++iCnt)
    {
        workerThreads[iCnt] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread2, NULL, 0, NULL);
        if (workerThreads[iCnt] == 0)
        {
            wprintf(L"Failed to operate _beginthreadex.\n");
            __debugbreak();
        }
    }

    ::WaitForMultipleObjects(NUM_WORKER, workerThreads, TRUE, INFINITE);
}

LockFreeStack::LockFreeStack() : _Top(nullptr)
{
}

void LockFreeStack::Push(Node* InNode)
{
    Node* t;
    do 
    {
        t = _Top;
        InNode->next = t;
    } while (_InterlockedCompareExchange64((long long*)&_Top, (long long)InNode, (long long)t) != (long long)t);
}

Node* LockFreeStack::Pop()
{
    Node* t;
    Node* newTop;
    Node* deletedTop;
    do 
    {
        t = _Top;
		if (t == nullptr)
			return nullptr;
        newTop = t->next;
    } while ((Node*)_InterlockedCompareExchange64((long long*)&_Top, (long long)newTop, (long long)t)
        != t);

    /** t가 꺼내진 상황이므로 Pop 수행 후 아래의 조건문을 수행하는 사이에 다른 thread에 의해 t의 데이터가 바뀔 일은 없다. */
    if (newTop != t->next)
    {
        __debugbreak();
    }

    return t;
}
