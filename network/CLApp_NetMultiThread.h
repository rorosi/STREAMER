/**********************************************************
* Copyright(C) 2023 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2023.01.27
* FileName : CLApp_NetMultiThread.h
* Auther   : Garnet Kim
* Modify   : 2023-01-27
**********************************************************/
#ifndef	_CLAPP_NETWORK_MULTI_THREAD_H_
#define _CLAPP_NETWORK_MULTI_THREAD_H_

#include <pthread.h>
#include <string>

#include "base/CLApp_Mutex.h"
#include "base/CLApp_Condition.h"

using namespace std;

class CLAppNetMultiThread
{
public:
	typedef pthread_t ThreadID;

    enum Priority { HIGH, MIDDLE, LOW };
    enum Policy { REALTIME, NORMAL  };

    CLAppNetMultiThread(int index);
	virtual ~CLAppNetMultiThread();

	virtual void multiLoop(int index = 0)=0;

	void multiStart();
	void multiStop();
	void multiJoin();
	bool multiRun();
	
private:
	static void* mutliEntry(void* arg);

private:
	enum ThreadState{
		INITIALIZED=0,
		RUNNING,
		STOPPED,
		DONE,
	};

	CLAppMutex m_Lock;
	CLAppCondition m_Cond;
	ThreadID m_Tid;
	ThreadState m_State;
	static bool m_bExit;
	
protected:
	Priority m_Priority;
	Policy 	m_Policy;
	int		m_nIndex;
};

#endif	//_CLAPP_NETWORK_MULTI_THREAD_H_
