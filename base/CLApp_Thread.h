/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_Thread.h
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#ifndef	_CLAPP_THREAD_H_
#define _CLAPP_THREAD_H_

#include <pthread.h>
#include <string>
#include "CLApp_Mutex.h"
#include "CLApp_Condition.h"

using namespace std;

class CLAppThread
{
public:
	typedef pthread_t ThreadID;

    enum Priority { HIGH, MIDDLE, LOW };
    enum Policy { REALTIME, NORMAL  };

    CLAppThread();
	virtual ~CLAppThread();

	virtual void mainLoop()=0;
	virtual void initInstance();
	virtual void exitInstance();

	void start();
	void stop();
	void join();
	bool isRunning();
	
	ThreadID getThreadID() {return pthread_self();}

	void msleep(unsigned long msec);

	static void killAllThread()
	{
		mbExit = true;
	}

private:
	enum {MAX_THREAD_CNT=20,};
	enum ThreadState{
		INITIALIZED=0,
		RUNNING,
		STOPPED,
		DONE,
	};

	CLAppMutex mLock;
	CLAppCondition mCond;
	ThreadID mTid;
	ThreadState mState;
	static bool mbExit;

	static void* entry(void* arg);
	
protected:
	Priority mPriority;
	Policy mPolicy;

public:	
	int	mTskId;
};

#endif	//_CLAPP_THREAD_H_
