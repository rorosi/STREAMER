/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_BufferFrame.cpp
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <string>

#include "CLApp_Time.h"
#include "CLApp_BufferFrame.h"

//#define USING_MEMPHY_QUEUE			1

#define MAX_FRAME_BUFFER_SIZE		(16*1024*1024)

CLAppBufferFrame::CLAppBufferFrame()
{
	mpBufferFrame.buffer = NULL;
	init();
}

CLAppBufferFrame::CLAppBufferFrame(const CLAppBufferFrame& frame)
{
	mpBufferFrame.buffer = NULL;
	init();
	*this = frame;
}

CLAppBufferFrame::CLAppBufferFrame(const CLAppBufferFrame* pFrame) 
{
	mpBufferFrame.buffer = NULL;
	init();
	*this = pFrame;
}

CLAppBufferFrame::CLAppBufferFrame(const buffer_frame_t& frame)
{
	mpBufferFrame.buffer = NULL;
	init();
	*this = frame;
}

CLAppBufferFrame::CLAppBufferFrame(const buffer_frame_t* pFrame)
{
	mpBufferFrame.buffer = NULL;
	init();
	*this = pFrame;
}

CLAppBufferFrame::~CLAppBufferFrame()
{
#ifdef USING_MEMPHY_QUEUE
	mpBufferFrame.buffer = NULL;
#else
	if(mpBufferFrame.buffer != NULL)
	{
		delete [] mpBufferFrame.buffer;
		mpBufferFrame.buffer = NULL;
	}
#endif	
}

CLAppBufferFrame& CLAppBufferFrame::operator=(const CLAppBufferFrame& frame)
{
	CLAppBufferFrame* p = (CLAppBufferFrame*)&frame;

	setDate(p->getDate());
	setFlag(p->getFlag());
	setFrame(p->getFrame(), p->getSize(), p->getFlag());	
	setResol(p->getWidth(), p->getHeight());
	setChannel(p->getChannel());
	
	return *this;
}

CLAppBufferFrame& CLAppBufferFrame::operator=(const CLAppBufferFrame* pFrame)
{
	if(pFrame != NULL)
	{
		CLAppBufferFrame* p = (CLAppBufferFrame*)pFrame;
		setDate(p->getDate());
		setFlag(p->getFlag());
		setFrame(p->getFrame(), p->getSize(), p->getFlag());	
		setResol(p->getWidth(), p->getHeight());
		setType(p->getType());
		setChannel(p->getChannel());
	}
	return *this;
}

CLAppBufferFrame& CLAppBufferFrame::operator=(const buffer_frame_t* pImage)
{
	if(pImage != NULL)
	{
		setDate(pImage->date);
		setFlag((int)pImage->flag);
		setFrame(pImage->buffer, pImage->size, pImage->flag);		
		setResol(pImage->width, pImage->height);
		setType(pImage->type);
		setChannel(pImage->ch);
	}
	return *this;
}

CLAppBufferFrame& CLAppBufferFrame::operator=(const buffer_frame_t& image)
{
	setDate(image.date);
	setFlag((int)image.flag);
	setFrame(image.buffer, image.size, image.flag);
	setResol(image.width, image.height);
	setType(image.type);
	setChannel(image.ch);

	return *this;
}

bool CLAppBufferFrame::operator<(const CLAppBufferFrame& frame)
{
	struct timeval t = ((CLAppBufferFrame*)&frame)->getDate();

	if(mpBufferFrame.date.tv_sec < t.tv_sec)
	{
		return true;
	}
	else if(mpBufferFrame.date.tv_sec == t.tv_sec)
	{
		if(mpBufferFrame.date.tv_usec < t.tv_usec)
		{
			return true;
		}
	}
	return false;
}

bool CLAppBufferFrame::operator>(const CLAppBufferFrame& frame)
{
	struct timeval t = ((CLAppBufferFrame*)&frame)->getDate();

	if(mpBufferFrame.date.tv_sec > t.tv_sec)
	{
		return true;
	}
	else if(mpBufferFrame.date.tv_sec == t.tv_sec)
	{
		if(mpBufferFrame.date.tv_usec > t.tv_usec)
		{
			return true;
		}
	}
	return true;
}

bool CLAppBufferFrame::operator==(const CLAppBufferFrame& frame)
{
	struct timeval t = ((CLAppBufferFrame*)&frame)->getDate();

	if(mpBufferFrame.date.tv_sec == t.tv_sec &&
		mpBufferFrame.date.tv_usec == t.tv_usec)
		return true;

	return false;
}

bool CLAppBufferFrame::operator!=(const CLAppBufferFrame& frame)
{
	struct timeval t = ((CLAppBufferFrame*)&frame)->getDate();

	if(mpBufferFrame.date.tv_sec != t.tv_sec ||
		mpBufferFrame.date.tv_usec != t.tv_usec)
		return true;

	return false;
}


void CLAppBufferFrame::init()
{
#ifndef USING_MEMPHY_QUEUE	
	if(mpBufferFrame.buffer != NULL)
	{
		delete [] mpBufferFrame.buffer;
	}
#endif
	mpBufferFrame.date.tv_sec = 0;
	mpBufferFrame.date.tv_usec = 0;
	
	mpBufferFrame.size = 0;
	mpBufferFrame.buffer = NULL;
	mpBufferFrame.flag = 0;
	mpBufferFrame.width = 0;
	mpBufferFrame.height = 0;
	mpBufferFrame.type = 0;
	mpBufferFrame.ch = 0;
}

struct timeval CLAppBufferFrame::getDate()
{
	return mpBufferFrame.date;
}

int CLAppBufferFrame::getFlag()
{
	return (int)mpBufferFrame.flag;
}

int CLAppBufferFrame::getSize()
{
	return mpBufferFrame.size;
}

unsigned char* CLAppBufferFrame::getFrame()
{
	return mpBufferFrame.buffer;
}

unsigned short CLAppBufferFrame::getWidth()
{
	return mpBufferFrame.width;
}

unsigned short CLAppBufferFrame::getHeight()
{
	return mpBufferFrame.height;
}

int CLAppBufferFrame::getType()
{
	return mpBufferFrame.type;
}

int CLAppBufferFrame::getChannel()
{
	return mpBufferFrame.ch;
}

void CLAppBufferFrame::setDate(const struct timeval date)
{
	mpBufferFrame.date = date;
}

void CLAppBufferFrame::setDate(CLAppDateTime date)
{
	date.getTime((struct timeval*)&mpBufferFrame.date);
}

void CLAppBufferFrame::setFlag(int e)
{
	mpBufferFrame.flag = e;
}

void CLAppBufferFrame::setFrame(unsigned char* pBuffer, int nSize, int iFlag)
{
	if(pBuffer != NULL && nSize > 0)
	{
		if(nSize > MAX_FRAME_BUFFER_SIZE)
			nSize = MAX_FRAME_BUFFER_SIZE;

#ifdef USING_MEMPHY_QUEUE
		mpBufferFrame.buffer = NULL;
		mpBufferFrame.size = 0;

		mpBufferFrame.buffer = new unsigned char[nSize];
#else
		if(mpBufferFrame.buffer != NULL)
		{
			delete [] mpBufferFrame.buffer;

			mpBufferFrame.buffer = NULL;
			mpBufferFrame.size = 0;
		}

		mpBufferFrame.buffer = new unsigned char[nSize];
#endif		
		if (mpBufferFrame.buffer == NULL) {
			mpBufferFrame.size = 0;
			mpBufferFrame.flag = 0;
		}
		else {
			memcpy(mpBufferFrame.buffer, pBuffer, nSize);
			mpBufferFrame.size = nSize;
			mpBufferFrame.flag = iFlag;
		}
	}		
} 

void CLAppBufferFrame::setResol(unsigned short iWidth, unsigned short iHeight)
{
	mpBufferFrame.width = iWidth;
	mpBufferFrame.height = iHeight;
}

void CLAppBufferFrame::setType(int iType)
{// 1:RAW, 2:UYVY, 3:RGB
	mpBufferFrame.type = iType;
}


void CLAppBufferFrame::setChannel(int iCh)
{
	mpBufferFrame.ch = iCh;
}
