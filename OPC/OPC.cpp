#pragma comment(lib, "Ws2_32.lib")
# include <winsock2.h>
#include <stdio.h>
#include <malloc.h>
#include "OPCClientHandler\OpcUaServer.h"
#include "open62541.h"

using namespace std;

int main(int argc, char* argv[])
{
	OPCConnection::OpcUaServer opcserv;

	opcserv.setTcpListeningPort( 16664 );
	opcserv.run();

	for ( ;; ) // hanging the main thread
	{ 
		std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) ); 
	}
	
	return 0;
}

