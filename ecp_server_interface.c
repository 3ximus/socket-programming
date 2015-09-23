#include "udp/udp_server.h"
#include "resources.h"

int main(int argc, char** argv)
{
	int port;

	if(argc == 1)
		port = DEFAULT_PORT;
	else if(argc == 3)
		port = atoi(argv[2]);
	else
	{
		printf("ERROR: Wrong input format.\ninput: ./ECP [-p ECPport]\n");
		exit(1);
	}

	UDPserver(port);

	return 0;
}
