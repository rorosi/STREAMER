#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <string>
#include <sys/mount.h>
#include <signal.h>
#include <mtd/mtd-user.h>
#include <sys/sysinfo.h>

#include "include/App_Define.h"
#include "base/App_Command.h"
#include "App_GSTManager.h"

///////////////////////////////////////////////////////////////////////

AppGSTManager* AppGSTManager::m_pInstance = NULL;

///////////////////////////////////////////////////////////////////////
AppGSTManager::AppGSTManager()
{
	m_pGST_1     = NULL;
	m_pRTPSender = NULL;

	memset(&m_config, 0, sizeof(AppConfig_T));
	m_config.mode      = MODE_RTSP;
	m_config.source    = SOURCE_TEST;
	m_config.port      = DEFAULT_RTP_PORT;
	m_config.width     = DEFAULT_WIDTH;
	m_config.height    = DEFAULT_HEIGHT;
	m_config.fps       = DEFAULT_FPS;
	m_config.bitrate   = DEFAULT_BITRATE;
	m_config.codec     = 0;
	m_config.rtsp_port = DEFAULT_RTSP_PORT;
	strncpy(m_config.host,      DEFAULT_RTP_HOST,  sizeof(m_config.host) - 1);
	strncpy(m_config.device,    DEFAULT_DEVICE,    sizeof(m_config.device) - 1);
	strncpy(m_config.rtsp_host, DEFAULT_RTSP_HOST, sizeof(m_config.rtsp_host) - 1);
}

AppGSTManager::~AppGSTManager()
{

}

void AppGSTManager::initInstance()
{
	AppThread::initInstance();

	if (m_config.mode == MODE_RTSP) {
		m_pGST_1 = AppGST::instance();

		if (m_pGST_1 != NULL) {
			m_pGST_1->setConfig(m_config);
			m_pGST_1->start();
		} else {
			//nothing to do
		}
	} else {	// MODE_RTP
		m_pRTPSender = AppRTPSender::instance();

		if (m_pRTPSender != NULL) {
			m_pRTPSender->setConfig(m_config);
			m_pRTPSender->start();
		} else {
			//nothing to do
		}
	}
}

void AppGSTManager::exitInstance()
{
	if (m_pGST_1 != NULL) {
		m_pGST_1->stop();
		m_pGST_1->join();
		m_pGST_1->release();
	} else {
		//nothing to do
	}

	if (m_pRTPSender != NULL) {
		m_pRTPSender->stop();
		m_pRTPSender->join();
		m_pRTPSender->release();
	} else {
		//nothing to do
	}

	AppThread::exitInstance();
}	

void AppGSTManager::mainLoop()
{
	while (isRunning())
	{
		usleep(10000);
    }
}
