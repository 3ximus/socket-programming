#include "resources.h"
/*
 * UDP client API
 */
char* UDPclient(const char *msg, const struct server *ecp)
{
	int fd, n, addrlen;
	struct sockaddr_in addr;
	char buffer[BUFFER_SIZE];
	char *answer = (char *) malloc(BUFFER_SIZE * sizeof(char));

	/*Atribuicao da socket UDP */
	if((fd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		printf("Error: socket()\n");
		free(answer);
		exit(1);
	}

	memset((void *)&addr,(int)'\0', sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = getHostIP(ecp->name);
	addr.sin_port = htons(ecp->port);

	if((n = sendto(fd, msg, strlen(msg),0,(struct sockaddr *) &addr, sizeof(addr))) == -1)
	{
		printf("Error: sendto()\n");
		free(answer);
		close(fd);
		exit(1);
	}

	addrlen = sizeof(addr);

	if((n = recvfrom(fd,buffer,BUFFER_SIZE,0,(struct sockaddr*) &addr,(unsigned int *)&addrlen))==-1)
	{
		printf("Error: recvfrom()\n");
		free(answer);
		close(fd);
		exit(1);
	}	

	/* write(1,"echo: ",6);

	if(n > 0)
	write(1,buffer,n);
	else
	write(1,"NULL",4);	

	printf("\n");
	printHostInfo(addr);*/

	close(fd);

	strcpy(answer,buffer); /* CORRECT for buffer overflow */

	return answer;
}
