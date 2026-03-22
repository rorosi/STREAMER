/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_Condition.cpp
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#include <CLApp_Mutex.h>
#include <CLApp_Condition.h>

CLAppCondition::CLAppCondition()
{
	pthread_cond_init(&mCond, NULL);
}

CLAppCondition::~CLAppCondition()
{
	pthread_cond_destroy(&mCond);
}

void CLAppCondition::wait(CLAppMutex* pLock)
{
	pthread_cond_wait(&mCond, &pLock->mLock);
}

int CLAppCondition::timedWait(CLAppMutex* pLock, unsigned int usecs)
{
	int result;
	struct timespec to;
	struct timeval now;

	result=gettimeofday(&now, NULL);

	to.tv_sec=usecs/1000000+now.tv_sec;
	int tv_usecs=(usecs%1000000)+now.tv_usec;
	to.tv_nsec=tv_usecs*1000;
	if (tv_usecs>=1000000)
	{
		to.tv_sec ++;
		to.tv_nsec-=1000000000;
	}

	result = pthread_cond_timedwait(&mCond, &pLock->mLock, &to);

	if (result==ETIMEDOUT)
	{
		return 1;
	}
	return 0;
}

void CLAppCondition::signal()
{
	pthread_cond_signal(&mCond);
}

void CLAppCondition::broadcast()
{
	pthread_cond_broadcast(&mCond);
}

