#include "OpcUaServer.h"

namespace OPCConnection
{

OpcUaServer::OpcUaServer( uint16_t tcpListeningPort )
	: m_listeningTcpPort(static_cast<UA_UInt16>(tcpListeningPort)), m_bServerInitialized(false),
		m_bShuttingDown(false), m_uiDelayTarget_ms(1), m_pServer(nullptr)
{
	initialize();
}

OpcUaServer::~OpcUaServer()
{
	uninitialize();
}

void OpcUaServer::run()
{
	if ( !m_bServerInitialized )
	{
		assert( !" Trying to run uninitialized server!" );
		return;
	}

	m_thread = std::move(
		std::thread( &OpcUaServer::serverThread_main, this )
		);
}

void OpcUaServer::stop()
{
	m_bShuttingDown = true;
	m_thread.join();
}

void OpcUaServer::setTcpListeningPort( uint16_t tcpPort )
{
	m_listeningTcpPort = static_cast<UA_UInt16>(tcpPort);
}

bool OpcUaServer::initialize()
{
	if ( !m_bServerInitialized )
	{
		m_serverConfig = UA_ServerConfig_standard;
		m_networkLayer = UA_ServerNetworkLayerTCP( m_connectionConfig, m_listeningTcpPort );
		m_serverConfig.networkLayers = &m_networkLayer;
		m_serverConfig.networkLayersSize = 1;

		m_pServer = UA_Server_new( m_serverConfig );
		UA_StatusCode retval = UA_Server_run_startup( m_pServer );

		if ( retval != UA_STATUSCODE_GOOD )
		{
			return false;
		}

		m_bServerInitialized = true;
		return true;

	}

	return true;
}

void OpcUaServer::uninitialize()
{
	if ( m_bServerInitialized )
	{
		if ( m_pServer == nullptr )
			UA_Server_delete( m_pServer );

		m_networkLayer.deleteMembers( &m_networkLayer );
	}

	m_bServerInitialized = false;
}

void OpcUaServer::serverThread_main()
{
	UA_UInt16 allowedDelay = 0;

	while ( !m_bShuttingDown )
	{
		allowedDelay = UA_Server_run_iterate( m_pServer, false );

		std::this_thread::sleep_for( 
			std::chrono::milliseconds( allowedDelay < m_uiDelayTarget_ms ? allowedDelay : m_uiDelayTarget_ms )
			);
	}
}

}