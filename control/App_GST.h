#ifndef _APP_GST_H_ 
#define _APP_GST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <time.h>
#include <semaphore.h>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h> 
#include <gst/app/gstappsink.h>
#include <gst/rtsp-server/rtsp-server.h>

#include "base/App_Thread.h"
#include "base/App_MsgQueue.h"
#include "include/App_Define.h"

using namespace std;

typedef struct
{

	GMainLoop *loop;

} GstFBDataFrameInfo_T;	// PortData

class AppGST : public AppThread
{
public:
	AppGST();
	virtual ~AppGST();
	virtual void mainLoop();
	virtual void initInstance();
	virtual void exitInstance();
	
	static AppGST* instance();
	void release();

	void setConfig(const AppConfig_T& cfg);
	void StartRTSPServer();
		
private:
	int GstInit();
	int GstExit();

public:
	GstFBDataFrameInfo_T	m_pGstFBDataFrameInfo;
	int		m_nFrameWidth;
	int		m_nFrameHeight;

private:
    static AppGST 	*m_pInstance;
    GstRTSPServer*   m_rtspServer = nullptr;
    char             m_rtspHost[64];
    int              m_rtspPort;
    int              m_fps;
    int              m_codec;		// 0=h264, 1=h265

private:
    struct ev_loop *m_EvLoop_XavierManager;
};

inline AppGST* AppGST::instance()
{
	if (!m_pInstance) {
		m_pInstance = new AppGST();	
	}
	return m_pInstance;
}

inline void AppGST::release()
{
	if (m_pInstance != NULL) {
		delete m_pInstance;
		m_pInstance = NULL;
	}
}
#endif // _APP_GST_H_


