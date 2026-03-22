/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_BufferFrame.h
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#ifndef _CLAPP_BUFFER_FRAME_H_
#define _CLAPP_BUFFER_FRAME_H_

#include <time.h>
#include <sys/types.h>

#include "CLApp_Time.h"

typedef struct _tagBufferFrameType
{
	struct timeval	date;
	unsigned short	width;
	unsigned short	height;
	int 		ch;
	int			type;			// 0:None, 1:RAW, 2:UYVY, 3:RGB
	int			size;
	int			flag;
	unsigned char*	buffer;
	
} buffer_frame_t;

//class CLAppRingBuffer;

class CLAppBufferFrame
{
	//friend class CLAppRingBuffer;

public:
	CLAppBufferFrame();
	CLAppBufferFrame(const CLAppBufferFrame& frame);
	CLAppBufferFrame(const CLAppBufferFrame* pFrame);
	CLAppBufferFrame(const buffer_frame_t& frame);
	CLAppBufferFrame(const buffer_frame_t* pFrame);
	virtual ~CLAppBufferFrame();

	CLAppBufferFrame& operator=(const CLAppBufferFrame& frame);
	CLAppBufferFrame& operator=(const CLAppBufferFrame* pFrame);
	CLAppBufferFrame& operator=(const buffer_frame_t* pFrame);
	CLAppBufferFrame& operator=(const buffer_frame_t& frame);
	bool operator<(const CLAppBufferFrame& frame);
	bool operator>(const CLAppBufferFrame& frame);
	bool operator==(const CLAppBufferFrame& frame);
	bool operator!=(const CLAppBufferFrame& frame);
	
	void init();
	struct timeval getDate();
	int getFlag();
	int getSize();
	unsigned char* getFrame();
	unsigned short getWidth();
	unsigned short getHeight();
	int getType();
	int getChannel();
	
	void setDate(const struct timeval date);
	void setDate(CLAppDateTime date);
	void setFlag(int e);
	void setFrame(unsigned char* pBuffer, int nSize, int iFlag);
	void setResol(unsigned short iWidth, unsigned short iHeight);
	void setType(int iType);
	void setParams(unsigned char* pParams, int nSize);
	void setChannel(int iCh);

private:
	buffer_frame_t		mpBufferFrame;
};

#endif // _CLAPP_BUFFER_FRAME_H_