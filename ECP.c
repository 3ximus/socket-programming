#include "UDP.h"
#include "Resources.h"

int main(int argc, char** argv)
{
	int port;

	/* TODO */
	/* Tornar mais robusto com a utilizacao das flags */
	
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
