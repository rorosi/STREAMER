/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_BufferQueue.h
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#ifndef _CLAPP_BUFFER_QUEUE_H_
#define _CLAPP_BUFFER_QUEUE_H_

#include <list>
#include "CLApp_BufferFrame.h"

#define MAX_NET_QUEUE_SIZE		(16)

typedef list<CLAppBufferFrame>	list_queue_t;

typedef struct _tagBufferQueue
{
	size_t			count;
	size_t			seqId;
	pthread_mutex_t	*mutex;
	list_queue_t		*queue;
} buffer_queue_t;

class CLAppBufferQueue
{
public:
	CLAppBufferQueue(buffer_queue_t* pQueue, char* qName);
	~CLAppBufferQueue();

	bool getData(size_t index, CLAppBufferFrame* pFrame);
	bool peekData(size_t index, CLAppBufferFrame* pFrame);
	bool deleteData(size_t index);
	size_t addData(CLAppBufferFrame* pFrame, bool wait = false);
	void deleteAll();
	size_t getDataCount();
	bool isFull();
	bool isEmpty();
	bool getFirstData(CLAppBufferFrame* pFrame);
	bool getLastData(CLAppBufferFrame* pFrame);
	bool peekFirstData(CLAppBufferFrame* pFrame);
	bool peekLastData(CLAppBufferFrame* pFrame);
	void deleteFirstData();
	void deleteLastData();
	void setMaxDataCount(size_t size);
		
private:
	void lock();
	void unlock();

private:
	buffer_queue_t*	mpQueue;	
};

extern buffer_queue_t gCLAppControlDataQueue;
extern buffer_queue_t gCLAppNetworkDataQueue[MAX_NET_QUEUE_SIZE];

class CLAppCtrlPrmsBuffer : public CLAppBufferQueue
{
public:
	CLAppCtrlPrmsBuffer() : CLAppBufferQueue(&gCLAppControlDataQueue, (char*)"CtrlPrmQueue") {}
};

class CLAppNetworkBuffer : public CLAppBufferQueue
{
public:
	CLAppNetworkBuffer(int index) : CLAppBufferQueue(&gCLAppNetworkDataQueue[index], (char*)"NetDataQueue") {}
};

#endif //_CLAPP_BUFFER_QUEUE_H_
