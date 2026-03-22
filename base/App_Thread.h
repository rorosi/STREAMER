#ifndef	_APP_THREAD_H_
#define _APP_THREAD_H_

#include <pthread.h>
#include <string>
#include "App_Mutex.h"
#include "App_Condition.h"

using namespace std;

class AppThread
{
public:
	typedef pthread_t ThreadID;

    enum Priority { HIGH, MIDDLE, LOW };
    enum Policy { REALTIME, NORMAL  };

    AppThread();
	virtual ~AppThread();

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

	AppMutex mLock;
	AppCondition mCond;
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

#endif	//_APP_THREAD_H_
