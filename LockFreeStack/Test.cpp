#include <iostream>
#include <Windows.h>
#include <process.h>
#include <vector>
#include "LockFreeStack.h"

#define NUM_LOG 1000000
#define NUM_PREPUSH 20000000
#define NUM_PUSHPOP 10
#define NUM_WORKER 16

LockFreeStack<int> g_Stack;
std::vector<LoggingStruct> g_Log;
unsigned long long g_Seq;

unsigned int WorkerThread1(void* pvParam)
{
    int threadID = (int)pvParam;
    int pushData = 0;

    while (true)
    {
        for (int iCnt = 0; iCnt < NUM_PUSHPOP; ++iCnt)
        {
            /*LoggingStruct* loggingStruct = new LoggingStruct();

            g_Stack.LoggingPush(pushData++, threadID, loggingStruct);
            
            loggingStruct->Seq = _InterlockedIncrement(&g_Seq);
            g_Log[threadID].push_back(loggingStruct);*/
        }

		for (int iCnt = 0; iCnt < NUM_PUSHPOP; ++iCnt)
		{
			/*LoggingStruct* loggingStruct = new LoggingStruct();

			g_Stack.LoggingPop(threadID, loggingStruct);

			loggingStruct->Seq = _InterlockedIncrement(&g_Seq);
			g_Log[threadID].push_back(loggingStruct);*/
		}
    }
    

    return 0;
}

unsigned int WorkerThread2(void* pvParam)
{
    int threadID = (int)pvParam;
    
    while (true)
    {
        LoggingStruct loggingStruct = g_Stack.LoggingPop(threadID);
        
        g_Log.push_back(loggingStruct);
        //g_Stack.Pop();
    }

    return 0;
}

int main()
{
    HANDLE workerThreads[NUM_WORKER] = { 0, };


#pragma region Test Worker1
    g_Log.reserve(NUM_WORKER);
    g_Log.reserve(NUM_PREPUSH);
    for (int iCnt = 0; iCnt < NUM_PREPUSH; ++iCnt)
    {
        g_Stack.Push(iCnt);
    }
    for (int iCnt = 0; iCnt < NUM_WORKER; ++iCnt)
    {
        workerThreads[iCnt] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread2, (void*)iCnt, CREATE_SUSPENDED, NULL);
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