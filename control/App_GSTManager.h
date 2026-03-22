#ifndef _APP_GST_MANAGER_H_ 
#define _APP_GST_MANAGER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "base/App_Thread.h"
#include "control/App_GST.h"
#include "control/App_RTPSender.h"
#include "include/App_Define.h"

using namespace std;

class AppGSTManager : public AppThread
{
public:
	AppGSTManager();
	virtual ~AppGSTManager();
	virtual void mainLoop();
	virtual void initInstance();
	virtual void exitInstance();

	static AppGSTManager* instance();
	void release();

	void setConfig(const AppConfig_T& cfg);

private:
    static AppGSTManager 	*m_pInstance;

private:
	AppGST		*m_pGST_1;
	AppRTPSender	*m_pRTPSender;
	AppConfig_T		 m_config;

};

inline void AppGSTManager::setConfig(const AppConfig_T& cfg)
{
	m_config = cfg;
}

inline AppGSTManager* AppGSTManager::instance()
{
	if (!m_pInstance) {
		m_pInstance = new AppGSTManager();	
	}
	return m_pInstance;
}

inline void AppGSTManager::release()
{
	if (m_pInstance != NULL) {
		delete m_pInstance;
		m_pInstance = NULL;
	}
}
#endif // _APP_GST_MANAGER_H_


