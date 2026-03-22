#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/App_Define.h"
#include "base/App_Command.h"
#include "App_StreamManager.h"

///////////////////////////////////////////////////////////////////////

AppStreamManager* AppStreamManager::m_pInstance = NULL;

///////////////////////////////////////////////////////////////////////
AppStreamManager::AppStreamManager()
{
	m_pRTSPServer = NULL;
	m_pRTPSender  = NULL;

	memset(&m_config, 0, sizeof(AppConfig_T));
	m_config.mode      = MODE_RTSP;
	m_config.source    = SOURCE_TEST;
	m_config.port      = DEFAULT_RTP_PORT;
	m_config.width     = DEFAULT_WIDTH;
	m_config.height    = DEFAULT_HEIGHT;
	m_config.fps       = DEFAULT_FPS;
	m_config.bitrate   = DEFAULT_BITRATE;
	m_config.codec     = CODEC_H264;
	m_config.rtsp_port = DEFAULT_RTSP_PORT;
	strncpy(m_config.host,      DEFAULT_RTP_HOST,  sizeof(m_config.host) - 1);
	strncpy(m_config.device,    DEFAULT_DEVICE,    sizeof(m_config.device) - 1);
	strncpy(m_config.rtsp_host, DEFAULT_RTSP_HOST, sizeof(m_config.rtsp_host) - 1);
}

AppStreamManager::~AppStreamManager()
{

}

void AppStreamManager::initInstance()
{
	AppThread::initInstance();

	if (m_config.mode == MODE_RTSP) {
		m_pRTSPServer = AppRTSPServer::instance();

		if (m_pRTSPServer != NULL) {
			m_pRTSPServer->setConfig(m_config);
			m_pRTSPServer->start();
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

void AppStreamManager::exitInstance()
{
	if (m_pRTSPServer != NULL) {
		m_pRTSPServer->stop();
		m_pRTSPServer->join();
		m_pRTSPServer->release();
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

void AppStreamManager::mainLoop()
{
	while (isRunning())
	{
		usleep(10000);
    }
}
