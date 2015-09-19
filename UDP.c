#include "UDP.h"
#include "Resources.h"

char* UDPclient(const char *msg, Server ecp)
{
	int fd, n, addrlen;
	struct sockaddr_in addr;
	char buffer[BUFFER_SIZE];
	char *answer = (char *) malloc(BUFFER_SIZE*sizeof(char));

	/*Atribuicao da socket UDP */
	if((fd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		printf("Error: socket()\n");
		free(answer);
		exit(1);
	}

	memset((void *)&addr,(int)'\0',sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = getHostIP(ecp.name);
	addr.sin_port = htons(ecp.port);

	if((n = sendto(fd, msg, strlen(msg),0,(struct sockaddr *) &addr,sizeof(addr))) == -1)
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
	getHostAdress(addr);*/

	close(fd);

	strcpy(answer,buffer);

	return answer;
}

void UDPserver(int port)
{
	int fd, addrlen, nread;
	struct sockaddr_in addr;
	char buffer[BUFFER_SIZE];
	char msg[100];

	/* 
	Udp socket atribution.
	create an endpoint for UDP communication. 
	*/
	if((fd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		printf("Error: socket()\n");
		exit(1);
	}

	memset((void *)&addr,(int)'\0',sizeof(addr));

	addr.sin_family = AF_INET; /* AF_INET -- IPv4 internet protocols (TCP,UDP) */
	addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Accept datagrams on any Internet interface on the system */
	addr.sin_port = htons(port);	

	/* 
	When a process wants to receive new incoming packets or connections,
    it should bind a socket to a local interface address. 
    */
	if(bind(fd,(struct sockaddr*) &addr,sizeof(addr)) == -1)
	{
		printf("Error: bind()\nTheres already a UDP server in the specified port.\n");
		exit(1);
	}

	while(1)
	{
		addrlen = sizeof(addr);

		/* Receive message from a socket */
		nread = recvfrom(fd,buffer,BUFFER_SIZE,0,(struct sockaddr*) &addr,(unsigned int *) &addrlen);

		if(nread == -1)
		{
			printf("Error: recvfrom()\n");
			exit(1);
		}

		printf("%s\n",buffer);

		char **server_cmd = parseString(buffer," ");

		if(strcmp(server_cmd[0],"TQR") == 0)
		{	
			memset((void *)msg,(int)'\0',strlen(msg));	
			strcpy(msg, readFromFile("topics.txt"));
		}
		else if(strcmp(server_cmd[0],"TER") == 0)
		{	
			strcpy(msg, server_cmd[1]); /* devolve o numero do topico*/

			/* TODO */
			/* Pensar em como estruturar o topics.txt para o readFromFile funcionar
		    de forma consistente */   
		}
			
		/* Send a message on a socket */
		if((sendto(fd,msg,sizeof(msg),0,(struct sockaddr*) &addr, addrlen)) == -1)
		{
			printf("Error: sendto()\n");
			free(server_cmd);
			close(fd);
			exit(1);
		}

		free(server_cmd);
	}

	close(fd);
}
