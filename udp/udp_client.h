#include "../resources.h"
/*
 * UDP client API
 */
unsigned char* UDPclient(const unsigned char *request, const struct server *ecp)
{
	int fd, n, addrlen;
	struct sockaddr_in addr;
	unsigned char server_reply[SERVER_BUFFER_SIZE];
	unsigned char *returned_server_reply = (unsigned char *) malloc(BUFFER_SIZE * sizeof(unsigned char));

	/*Atribuicao da socket UDP */
	if((fd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		printf("Error: socket()\n");
		free(returned_server_reply);
		exit(1);
	}

	memset((void *)&addr,(int)'\0', sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = getHostIP((char *)ecp->name);
	addr.sin_port = htons(ecp->port);

	/* Send server request */
	if((n = sendto(fd, request, strlen((char*)request), 0,(struct sockaddr *) &addr, sizeof(addr))) == -1)
	{
		printf("Error: sendto()\n");
		free(returned_server_reply);
		close(fd);
		exit(1);
	}

	addrlen = sizeof(addr);

	/* receive server reply */
	if((n = recvfrom(fd, server_reply,BUFFER_SIZE,0,(struct sockaddr*) &addr,(unsigned int *)&addrlen))==-1)
	{
		printf("Error: recvfrom()\n");
		free(returned_server_reply);
		close(fd);
		exit(1);
	}

	close(fd);

	memcpy(returned_server_reply, server_reply, n); /* CORRECT for buffer overflow */

	return returned_server_reply;
}
