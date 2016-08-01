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

void OpcUaServer::setupNamespace()
{
/////////////////////////////////////////////////////
// Adding Modbus object
/////////////////////////////////////////////////////
	UA_NodeId busObjectNodeID;
	UA_ObjectTypeAttributes bus_objattr;
	UA_ObjectTypeAttributes_init( &bus_objattr );
	bus_objattr.description = UA_LOCALIZEDTEXT_ALLOC( "en_US", (char*)"Representation of Modbus industry data bus" );
	bus_objattr.displayName = UA_LOCALIZEDTEXT_ALLOC( "en_US", (char*)"Modbus" );

	UA_Server_addObjectTypeNode(
		m_pServer,
		UA_NODEID_STRING( 0, (char*)"Modbus" ),
		UA_NODEID_NUMERIC( 0, UA_NS0ID_OBJECTSFOLDER ),
		UA_NODEID_NUMERIC( 0, UA_NS0ID_ORGANIZES ),
		UA_QUALIFIEDNAME_ALLOC( 0, (char*)"Modbus_data_bus" ),
		bus_objattr,
		nullptr,
		&busObjectNodeID );

/////////////////////////////////////////////////////
// Adding PLC1 object
//
//	Modbus
//		|___PLC1
//
/////////////////////////////////////////////////////
	UA_NodeId PLC1_ObjectNodeID;
	UA_ObjectTypeAttributes PLC1_objattr;
	UA_ObjectTypeAttributes_init( &PLC1_objattr );
	PLC1_objattr.description = UA_LOCALIZEDTEXT_ALLOC( "en_US", ( char* )"Representation of PLC controller" );
	PLC1_objattr.displayName = UA_LOCALIZEDTEXT_ALLOC( "en_US", ( char* )"PLC1" );
	//UA_NodeId parentNodeID = UA_NODEID_NULL;// UA_NODEID_STRING( 0, ( char* )"Modbus industry data bus" );

	UA_Server_addObjectTypeNode(
		m_pServer,
		UA_NODEID_STRING( 0, ( char* )"PLC1" ),
		busObjectNodeID,
		UA_NODEID_NUMERIC( 0, UA_NS0ID_ORGANIZES ),
		UA_QUALIFIEDNAME_ALLOC( 0, ( char* )"PLC1" ),
		PLC1_objattr,
		nullptr,
		&PLC1_ObjectNodeID );

/////////////////////////////////////////////////////
// Adding var1 variable
//
//	Modbus
//		|___PLC1
//			|___var1
//
/////////////////////////////////////////////////////

	/* 1) Define the attribute of the myInteger variable node */
	UA_NodeId var_1_ID;
	UA_VariableAttributes var1_attr;
	UA_VariableAttributes_init( &var1_attr );
	UA_Int32 var1_myInteger = 123456;
	UA_Variant_setScalar( &var1_attr.value, &var1_myInteger, &UA_TYPES[UA_TYPES_UINT32] );
	var1_attr.description = UA_LOCALIZEDTEXT( "en_US", (char*)"var1 representation" );
	var1_attr.displayName = UA_LOCALIZEDTEXT( "en_US", (char*)"var1" );

	/* 2) Add the variable node to the information model */
	UA_NodeId var1_parentReferenceNodeId = UA_NODEID_NUMERIC( 0, UA_NS0ID_ORGANIZES );

	UA_DataSource var1_dateDataSource;
	var1_dateDataSource.handle = ( void* )this;
	var1_dateDataSource.read = NULL;
	var1_dateDataSource.write = NULL;

	UA_StatusCode retval = UA_Server_addDataSourceVariableNode(
		m_pServer,
		UA_NODEID_STRING( 0, ( char* )"var1" ),
		PLC1_ObjectNodeID,
		var1_parentReferenceNodeId,
		UA_QUALIFIEDNAME( 0, ( char* )"var1" ),
		UA_NODEID_NULL,
		var1_attr,
		var1_dateDataSource,
		&var_1_ID
	);

/////////////////////////////////////////////////////
// Adding var2 variable
//
//	Modbus
//		|___PLC1
//			|___var1
//			|___var2
//
/////////////////////////////////////////////////////
	/* 1) Define the attribute of the myInteger variable node */
	UA_NodeId var_2_ID;
	UA_VariableAttributes var2_attr;
	UA_VariableAttributes_init( &var2_attr );
	UA_Int32 var2_myInteger = 654321;
	UA_Variant_setScalar( &var2_attr.value, &var2_myInteger, &UA_TYPES[UA_TYPES_UINT32] );
	var2_attr.description = UA_LOCALIZEDTEXT( "en_US", ( char* )"var2 representation" );
	var2_attr.displayName = UA_LOCALIZEDTEXT( "en_US", ( char* )"var2" );

	/* 2) Add the variable node to the information model */
	UA_NodeId var2_parentReferenceNodeId = UA_NODEID_NUMERIC( 0, UA_NS0ID_ORGANIZES );

	UA_DataSource var2_dateDataSource;
	var2_dateDataSource.handle = ( void* )this;
	var2_dateDataSource.read = NULL;
	var2_dateDataSource.write = NULL;

	retval = UA_Server_addDataSourceVariableNode(
		m_pServer,
		UA_NODEID_STRING( 0, ( char* )"var2" ),
		PLC1_ObjectNodeID,
		var2_parentReferenceNodeId,
		UA_QUALIFIEDNAME( 0, ( char* )"var2" ),
		UA_NODEID_NULL,
		var2_attr,
		var2_dateDataSource,
		&var_2_ID
	);


/////////////////////////////////////////////////////
// Adding PLC2 object
//
//	Modbus
//		|___PLC1
//		|	|___var1
//		|	|___var2
//		|
//		|___PLC2
//
/////////////////////////////////////////////////////
	UA_NodeId PLC2_ObjectNodeID;
	UA_ObjectTypeAttributes PLC2_objattr;
	UA_ObjectTypeAttributes_init( &PLC2_objattr );
	PLC2_objattr.description = UA_LOCALIZEDTEXT_ALLOC( "en_US", ( char* )"Representation of PLC controller" );
	PLC2_objattr.displayName = UA_LOCALIZEDTEXT_ALLOC( "en_US", ( char* )"PLC2" );

	UA_Server_addObjectTypeNode(
		m_pServer,
		UA_NODEID_STRING( 0, ( char* )"PLC2" ),
		busObjectNodeID,
		UA_NODEID_NUMERIC( 0, UA_NS0ID_ORGANIZES ),
		UA_QUALIFIEDNAME_ALLOC( 0, ( char* )"PLC2" ),
		PLC2_objattr,
		nullptr,
		&PLC2_ObjectNodeID );

/////////////////////////////////////////////////////
// Adding var1 variable
//
//	Modbus
//		|___PLC1
//		|	|___var1
//		|	|___var2
//		|
//		|___PLC2
//			|___var1
//
/////////////////////////////////////////////////////

	/* 1) Define the attribute of the myInteger variable node */
	UA_NodeId var_11_ID;
	UA_VariableAttributes var11_attr;
	UA_VariableAttributes_init( &var11_attr );
	UA_Int32 var11_myInteger = 123456;
	UA_Variant_setScalar( &var11_attr.value, &var11_myInteger, &UA_TYPES[UA_TYPES_UINT32] );
	var11_attr.description = UA_LOCALIZEDTEXT( "en_US", (char*)"var1 representation" );
	var11_attr.displayName = UA_LOCALIZEDTEXT( "en_US", (char*)"var1" );

	/* 2) Add the variable node to the information model */
	UA_NodeId var11_parentReferenceNodeId = UA_NODEID_NUMERIC( 0, UA_NS0ID_ORGANIZES );

	UA_DataSource var11_dateDataSource;
	var11_dateDataSource.handle = ( void* )this;
	var11_dateDataSource.read = NULL;
	var11_dateDataSource.write = NULL;

	 retval = UA_Server_addDataSourceVariableNode(
		m_pServer,
		UA_NODEID_STRING( 0, ( char* )"var1" ),
		PLC2_ObjectNodeID,
		var11_parentReferenceNodeId,
		UA_QUALIFIEDNAME( 0, ( char* )"var1" ),
		UA_NODEID_NULL,
		var11_attr,
		var11_dateDataSource,
		&var_11_ID
	);

/////////////////////////////////////////////////////
// Adding var2 variable
//
//	Modbus
//		|___PLC1
//		|	|___var1
//		|	|___var2
//		|
//		|___PLC2
//			|___var1
//			|___var2
//
/////////////////////////////////////////////////////
	/* 1) Define the attribute of the myInteger variable node */
	UA_NodeId var_22_ID;
	UA_VariableAttributes var22_attr;
	UA_VariableAttributes_init( &var22_attr );
	UA_Int32 var22_myInteger = 654321;
	UA_Variant_setScalar( &var22_attr.value, &var22_myInteger, &UA_TYPES[UA_TYPES_UINT32] );
	var22_attr.description = UA_LOCALIZEDTEXT( "en_US", ( char* )"var2 representation" );
	var22_attr.displayName = UA_LOCALIZEDTEXT( "en_US", ( char* )"var2" );

	/* 2) Add the variable node to the information model */
	UA_NodeId var22_parentReferenceNodeId = UA_NODEID_NUMERIC( 0, UA_NS0ID_ORGANIZES );

	UA_DataSource var22_dateDataSource;
	var22_dateDataSource.handle = ( void* )this;
	var22_dateDataSource.read = NULL;
	var22_dateDataSource.write = NULL;

	retval = UA_Server_addDataSourceVariableNode(
		m_pServer,
		UA_NODEID_STRING( 0, ( char* )"var2" ),
		PLC2_ObjectNodeID,
		var22_parentReferenceNodeId,
		UA_QUALIFIEDNAME( 0, ( char* )"var2" ),
		UA_NODEID_NULL,
		var22_attr,
		var22_dateDataSource,
		&var_22_ID
	);

}

bool OpcUaServer::initialize()
{
	if ( !m_bServerInitialized )
	{
		m_serverConfig = UA_ServerConfig_standard;
		m_connectionConfig = UA_ConnectionConfig_standard;
		m_networkLayer = UA_ServerNetworkLayerTCP( m_connectionConfig, m_listeningTcpPort );
		m_serverConfig.networkLayers = &m_networkLayer;
		m_serverConfig.networkLayersSize = 1;

		m_pServer = UA_Server_new( m_serverConfig );
		UA_StatusCode retval = UA_Server_run_startup( m_pServer );

		if ( retval != UA_STATUSCODE_GOOD )
		{
			return false;
		}

		setupNamespace();

		m_bServerInitialized = true;
		return true;

	}

	return true;
}

void OpcUaServer::uninitialize()
{
	if ( m_bServerInitialized )
	{
		UA_StatusCode retval = UA_Server_run_shutdown( m_pServer );

		assert( retval == UA_STATUSCODE_GOOD );

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