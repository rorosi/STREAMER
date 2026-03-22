#ifndef _APP_STREAM_MANAGER_H_
#define _APP_STREAM_MANAGER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "base/App_Thread.h"
#include "control/App_RTSPServer.h"
#include "control/App_RTPSender.h"
#include "include/App_Define.h"

using namespace std;

class AppStreamManager : public AppThread
{
public:
	AppStreamManager();
	virtual ~AppStreamManager();
	virtual void mainLoop();
	virtual void initInstance();
	virtual void exitInstance();

	static AppStreamManager* instance();
	void release();

	void setConfig(const AppConfig_T& cfg);

private:
    static AppStreamManager	*m_pInstance;

private:
	AppRTSPServer	*m_pRTSPServer;
	AppRTPSender	*m_pRTPSender;
	AppConfig_T		 m_config;

};

inline void AppStreamManager::setConfig(const AppConfig_T& cfg)
{
	m_config = cfg;
}

inline AppStreamManager* AppStreamManager::instance()
{
	if (!m_pInstance) {
		m_pInstance = new AppStreamManager();
	}
	return m_pInstance;
}

inline void AppStreamManager::release()
{
	if (m_pInstance != NULL) {
		delete m_pInstance;
		m_pInstance = NULL;
	}
}
#endif // _APP_STREAM_MANAGER_H_
