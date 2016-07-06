#pragma once
#include "../open62541.h"
#include <thread>
#include <atomic>
#include <assert.h>

namespace OPCConnection
{

class OpcUaServer
{
public:
	OpcUaServer( uint16_t tcpListeningPort = 16664 );
	virtual ~OpcUaServer();

	void run();
	void stop();

	void setTcpListeningPort( uint16_t tcpPort );

private:

	bool initialize();
	void uninitialize();
	void serverThread_main();

	bool						m_bServerInitialized;

	UA_ServerConfig				m_serverConfig;
	UA_ConnectionConfig			m_connectionConfig;
	UA_ServerNetworkLayer		m_networkLayer;
	UA_UInt16					m_listeningTcpPort;
	UA_Server*					m_pServer;

	std::thread					m_thread;
	std::atomic<bool>			m_bShuttingDown;
	
	uint32_t					m_uiDelayTarget_ms;
};

}
