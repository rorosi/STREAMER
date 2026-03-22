#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>
#include <errno.h>

#include "App_Thread.h"


bool AppThread::mbExit = false;

AppThread::AppThread() 
    : mPriority(MIDDLE), mPolicy(REALTIME)
{
	CL_THREAD_SAFE_BLOCK(&mLock);
	mState=INITIALIZED;
	mTid = 0;
}

AppThread::~AppThread()
{
	CL_THREAD_UNSAFE_BLOCK(&mLock);
	if (mState!=INITIALIZED && mState!=DONE)
	{
		//dlog_info("### thread \"%s\" closed abnormal.",getThreadName());
	}
}

void AppThread::start()
{
	CL_THREAD_SAFE_BLOCK(&mLock);

        struct sched_param schedParam;
        int schedPolicy = SCHED_OTHER;
        int schedMaxPriority = sched_get_priority_max(SCHED_FIFO);

        if (mPolicy == REALTIME) {
            switch(mPriority) {
            case HIGH :
                schedParam.sched_priority = schedMaxPriority;
                schedPolicy = SCHED_FIFO;
                break;
            case LOW :
                schedParam.sched_priority = schedMaxPriority - 2;
                schedPolicy = SCHED_FIFO;
                break;
            case MIDDLE :
            default :
                schedParam.sched_priority = schedMaxPriority - 1;
                schedPolicy = SCHED_FIFO;
                break;
            }
        }
        else {
            schedParam.sched_priority = 0;
            schedPolicy = SCHED_OTHER;
        }

	if(schedPolicy == SCHED_OTHER){

	}	
	
	{
		pthread_attr_t attr[1];
		pthread_attr_init(attr);
		pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);
		pthread_attr_setstacksize(attr, 2 * 1024 * 1024);	// 2MB
		pthread_attr_setschedparam(attr, &schedParam);
		pthread_create(&mTid, attr, AppThread::entry, this);
		pthread_attr_destroy(attr);
	}
}

void AppThread::stop()
{
	CL_THREAD_SAFE_BLOCK(&mLock);

	if (mState != DONE)
	{
		mState = STOPPED;
	}
}

void AppThread::join()
{
	CL_THREAD_SAFE_BLOCK(&mLock);
	
	while (mState!=DONE)
	{
		mCond.wait(&mLock);
	}
}

bool AppThread::isRunning()
{
	CL_THREAD_SAFE_BLOCK(&mLock);
	return (mState == RUNNING && mbExit == false);
}

void* AppThread::entry(void* arg)
{
	AppThread* pCamThread = static_cast<AppThread*>(arg);

	{
		CL_THREAD_SAFE_BLOCK(&pCamThread->mLock);
		pCamThread->mState=RUNNING;
	}
	
	pCamThread->initInstance();
	pCamThread->mainLoop();
	pCamThread->exitInstance();

	{
		CL_THREAD_SAFE_BLOCK(&pCamThread->mLock);
		pCamThread->mState=DONE;
		pCamThread->mCond.signal();
	}

	return NULL;
}

void AppThread::msleep(unsigned long msec)
{
	usleep(msec*1000);
}

void AppThread::initInstance()
{
}

void AppThread::exitInstance()
{
	
}
