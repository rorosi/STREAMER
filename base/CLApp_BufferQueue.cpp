/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_BufferQueue.cpp
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <time.h>
#include <vector>
#include <deque>
#include <list>

#include <CLApp_BufferQueue.h>

buffer_queue_t gCLAppControlDataQueue = {
	 count:0, seqId:0, mutex:NULL, queue:NULL 
};

buffer_queue_t gCLAppNetworkDataQueue[MAX_NET_QUEUE_SIZE] = {
	 { count:0, seqId:0, mutex:NULL, queue:NULL },
	 { count:0, seqId:0, mutex:NULL, queue:NULL },
	 { count:0, seqId:0, mutex:NULL, queue:NULL },
	 { count:0, seqId:0, mutex:NULL, queue:NULL },	 
};

CLAppBufferQueue::CLAppBufferQueue(buffer_queue_t* pQueue, char* qName) : 
mpQueue(pQueue)
{
	if(mpQueue->mutex == NULL && mpQueue->queue == NULL)
	{
		mpQueue->mutex = new pthread_mutex_t;
		pthread_mutex_init(mpQueue->mutex, NULL);
		mpQueue->queue = new list_queue_t;
		mpQueue->seqId = 0;
	}
}

CLAppBufferQueue::~CLAppBufferQueue()
{
	deleteAll();

	if(mpQueue->mutex != NULL)
	{
		pthread_mutex_destroy(mpQueue->mutex);
		delete mpQueue->mutex;
		mpQueue->mutex = NULL;
	}

	if(mpQueue->queue != NULL)
	{
		delete mpQueue->queue;
		mpQueue->queue = NULL;
	}
}

bool CLAppBufferQueue::getData(size_t index, CLAppBufferFrame* pFrame)
{
	bool result = false;

	if(pFrame != NULL)
	{
		lock();
        	if(index < mpQueue->queue->size())
		{
			list_queue_t::iterator mi = mpQueue->queue->begin();
			advance(mi, index);
			*pFrame = *mi;
			mpQueue->queue->erase(mi);
			result = true;
		}

		unlock();
	}

	return result;
}

bool CLAppBufferQueue::peekData(size_t index, CLAppBufferFrame* pFrame)
{
	bool result = false;

	if(pFrame != NULL)
	{
		lock();                
        	if (index < mpQueue->queue->size())
		{
			list_queue_t::iterator mi = mpQueue->queue->begin();
			advance(mi, index);
			*pFrame = *mi;
			result = true;
		}
		unlock();
	}

	return result;
}

bool CLAppBufferQueue::getFirstData(CLAppBufferFrame* pFrame)
{
	bool result = false;

	if(pFrame != NULL)
	{
		lock();

		if(mpQueue->queue->size() > 0)
		{ 
			*pFrame = mpQueue->queue->front();
			mpQueue->queue->pop_front();
			result = true;
		}
		unlock();
	}
	return result;
}

bool CLAppBufferQueue::getLastData(CLAppBufferFrame* pFrame)
{
	bool result = false;

	if(pFrame != NULL)
	{
		lock();

		if(mpQueue->queue->size() > 0)
		{ 
			*pFrame = mpQueue->queue->back();
			mpQueue->queue->pop_back();
			result = true;
		}
		unlock();
	}
	return result;
}

bool CLAppBufferQueue::peekFirstData(CLAppBufferFrame* pFrame)
{
	bool result = false;

	if(pFrame != NULL)
	{
		lock();

		if(mpQueue->queue->size() > 0)
		{
			*pFrame = mpQueue->queue->front();
			result = true;
		}
		unlock();
	}
	return result;
}

bool CLAppBufferQueue::peekLastData(CLAppBufferFrame* pFrame)
{
	bool result = false;

	if(pFrame != NULL)
	{
		lock();
		if(mpQueue->queue->size() > 0)
		{
			*pFrame = mpQueue->queue->back();
			result = true;
		}
		unlock();
	}

	return result;
}

bool CLAppBufferQueue::deleteData(size_t index)
{
	bool result = false;

	lock();

	if(index < mpQueue->queue->size())
	{
		list_queue_t::iterator mi = mpQueue->queue->begin();
		advance(mi, index);

		mpQueue->queue->erase(mi);
		result = true;                
	}
	unlock();
	
	return result;
}

size_t CLAppBufferQueue::addData(CLAppBufferFrame* pFrame, bool wait)
{
	size_t index = 0;

	lock();
	
	if (wait) {
		while (mpQueue->queue->size() >= mpQueue->count ) {
			unlock();
			::usleep(500);
			lock();
		}
	}
	else {
		if(mpQueue->queue->size() >= mpQueue->count){
			mpQueue->queue->pop_front();
		}
	}

	mpQueue->queue->push_back(*pFrame);

	index = mpQueue->queue->size() - 1;
	unlock();

	return index;
}

void CLAppBufferQueue::deleteAll()
{
	lock();
	while(mpQueue->queue->size())
	{
		mpQueue->queue->pop_front();
	}		
	unlock();
}

void CLAppBufferQueue::deleteFirstData()
{
	lock();
	if(mpQueue->queue->size() > 0)
	{
		mpQueue->queue->pop_front();		
	}
	unlock();	
}

void CLAppBufferQueue::deleteLastData()
{
	lock();
	if(mpQueue->queue->size() > 0)
	{
		mpQueue->queue->pop_back();
	}
	unlock();
}

size_t CLAppBufferQueue::getDataCount()
{
	size_t count;

	lock();
	count = mpQueue->queue->size();
	unlock();

	return count;
}

void CLAppBufferQueue::setMaxDataCount(size_t size)
{
	lock();
	mpQueue->count = size;

	while(mpQueue->queue->size() > mpQueue->count)
	{
		mpQueue->queue->pop_front();
	}
	unlock();
}

bool CLAppBufferQueue::isFull()
{
	return (getDataCount() >= mpQueue->count);
}

bool CLAppBufferQueue::isEmpty()
{
	bool bResult = false;

	lock();
	bResult = mpQueue->queue->empty();
	unlock();

	return bResult;
}

void CLAppBufferQueue::lock()
{
	pthread_mutex_lock(mpQueue->mutex);
}

void CLAppBufferQueue::unlock()
{
	pthread_mutex_unlock(mpQueue->mutex);
}

