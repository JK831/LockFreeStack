// LockFreeStack.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <process.h>

#define NUM_NODE 10000000
#define NUM_WORKER 6

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
    bool PopAndDelete();

private:
    Node* _Top;
};

Node* g_Nodes1[NUM_NODE];
LockFreeStack g_Stack;

unsigned int WorkerThread1(void* pvParam)
{
    while (g_Stack.PopAndDelete())
    {
        
    }

    __debugbreak();
    return 0;
}

unsigned int WorkerThread2(void* pvParam)
{
	while (true)
	{
		Node* topNode = g_Stack.Pop();
        g_Stack.Push(topNode);
	}

    return 0;
}

int main()
{
    HANDLE workerThreads[NUM_WORKER] = { 0, };

    for (int iCnt = 0; iCnt < NUM_NODE; ++iCnt)
    {
        g_Nodes1[iCnt] = new Node();
        g_Nodes1[iCnt]->data = iCnt;
        g_Stack.Push(g_Nodes1[iCnt]);
    }
    
#pragma region Test Worker1
    for (int iCnt = 0; iCnt < NUM_WORKER; ++iCnt)
    {
        workerThreads[iCnt] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread1, NULL, CREATE_SUSPENDED, NULL);
        if (workerThreads[iCnt] == 0)
        {
               wprintf(L"Failed to operate _beginthreadex.\n");
            __debugbreak();
        }
    }
	for (int iCnt = 0; iCnt < NUM_WORKER; ++iCnt)
	{
        ::ResumeThread(workerThreads[iCnt]);
	}
#pragma endregion

#pragma region Test Worker2
    /*for (int iCnt = 0; iCnt < NUM_WORKER; ++iCnt)
    {
        workerThreads[iCnt] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread2, NULL, 0, NULL);
        if (workerThreads[iCnt] == 0)
        {
            wprintf(L"Failed to operate _beginthreadex.\n");
            __debugbreak();
        }
    }*/
#pragma endregion
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
    do 
    {
        t = _Top;
		if (t == nullptr)
			return nullptr;

        newTop = t->next;
    } while ((Node*)_InterlockedCompareExchange64((long long*)&_Top, (long long)newTop, (long long)t)
        != t);

    // TODO: Memory logging

    /** t�� ���� thread�� ���� �������� ��Ȳ�̹Ƿ� Pop ���� �� �Ʒ��� ���ǹ��� �����ϴ� ���̿� �ٸ� thread�� ���� t�� �����Ͱ� �ٲ� ���� ����. */
    if (newTop != t->next)
    {
        // ABA
        __debugbreak();
    }

    return t;
}

bool LockFreeStack::PopAndDelete()
{
	Node* t;
	Node* newTop;
	do
	{
		t = _Top;
		if (t == nullptr)
			return false;

		newTop = t->next; // �ٸ� thread�� t�� delete �ߴٸ� ���� �߻�
	} while ((Node*)_InterlockedCompareExchange64((long long*)&_Top, (long long)newTop, (long long)t)
		!= t);

    delete t;

    return true;
}
