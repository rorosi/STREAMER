/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_Thread.cpp
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>
#include <errno.h>

#include "CLApp_Thread.h"


bool CLAppThread::mbExit = false;

CLAppThread::CLAppThread() 
    : mPriority(MIDDLE), mPolicy(REALTIME)
{
	CL_THREAD_SAFE_BLOCK(&mLock);
	mState=INITIALIZED;
	mTid = 0;
}

CLAppThread::~CLAppThread()
{
	CL_THREAD_UNSAFE_BLOCK(&mLock);
	if (mState!=INITIALIZED && mState!=DONE)
	{
		//dlog_info("### thread \"%s\" closed abnormal.",getThreadName());
	}
}

void CLAppThread::start()
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
		pthread_attr_setstacksize(attr, 1024);		
		pthread_attr_setschedparam(attr, &schedParam);
		pthread_create(&mTid, attr, CLAppThread::entry, this);
		pthread_attr_destroy(attr);
	}
}

void CLAppThread::stop()
{
	CL_THREAD_SAFE_BLOCK(&mLock);

	if (mState != DONE)
	{
		mState = STOPPED;
	}
}

void CLAppThread::join()
{
	CL_THREAD_SAFE_BLOCK(&mLock);
	
	while (mState!=DONE)
	{
		mCond.wait(&mLock);
	}
}

bool CLAppThread::isRunning()
{
	CL_THREAD_SAFE_BLOCK(&mLock);
	return (mState == RUNNING && mbExit == false);
}

void* CLAppThread::entry(void* arg)
{
	CLAppThread* pCamThread = static_cast<CLAppThread*>(arg);

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

void CLAppThread::msleep(unsigned long msec)
{
	usleep(msec*1000);
}

void CLAppThread::initInstance()
{
}

void CLAppThread::exitInstance()
{
	
}
