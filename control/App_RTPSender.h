#ifndef _APP_RTP_SENDER_H_
#define _APP_RTP_SENDER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <gst/gst.h>

#include "base/App_Thread.h"
#include "base/App_MsgQueue.h"
#include "include/App_Define.h"

using namespace std;

typedef struct
{
	GMainLoop *loop;

} GstRTPDataFrameInfo_T;

typedef struct {
	string      host;
	int         port;
	int         width;
	int         height;
	int         framerate;
	int         bitrate;	// kbps
	int         codec;		// 0=h264, 1=h265
	int         source;		// SourceType_E
	string      device;		// v4l2 device path
	GstElement* pipeline;
	GstBus*     bus;
	bool        active;
} RtpStreamInfo_T;

class AppRTPSender : public AppThread
{
public:
	AppRTPSender();
	virtual ~AppRTPSender();
	virtual void mainLoop();
	virtual void initInstance();
	virtual void exitInstance();

	static AppRTPSender* instance();
	void release();

	void setConfig(const AppConfig_T& cfg);

	bool StartStream();
	bool StopStream();
	bool RestartStream();

private:
	std::string buildSourceBin();	// V4L2 교체 포인트: 이 함수만 수정
	std::string buildPipeline();
	int  GstInit();
	int  GstExit();
	void pollBusErrors();

private:
	static AppRTPSender*  m_pInstance;

	GstRTPDataFrameInfo_T   m_pGstRTPDataFrameInfo;
	RtpStreamInfo_T         m_stream;
};

inline AppRTPSender* AppRTPSender::instance()
{
	if (!m_pInstance) {
		m_pInstance = new AppRTPSender();
	}
	return m_pInstance;
}

inline void AppRTPSender::release()
{
	if (m_pInstance != NULL) {
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

#endif // _APP_RTP_SENDER_H_
