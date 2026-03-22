#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#include <App_Mutex.h>
#include <App_Condition.h>

AppCondition::AppCondition()
{
	pthread_cond_init(&mCond, NULL);
}

AppCondition::~AppCondition()
{
	pthread_cond_destroy(&mCond);
}

void AppCondition::wait(AppMutex* pLock)
{
	pthread_cond_wait(&mCond, &pLock->mLock);
}

int AppCondition::timedWait(AppMutex* pLock, unsigned int usecs)
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

void AppCondition::signal()
{
	pthread_cond_signal(&mCond);
}

void AppCondition::broadcast()
{
	pthread_cond_broadcast(&mCond);
}

