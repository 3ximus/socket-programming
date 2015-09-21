#include "resources.h"

/*
 * UDP Server API
 */
void UDPserver(int port)
{
	int fd, addrlen, nread;
	struct sockaddr_in addr;
	char buffer[BUFFER_SIZE];
	char **parsed_buffer;
	char msg[100];
	char log_msg[60];
	bzero(log_msg, 60);

	/* 
	Udp socket atribution.
	create an endpoint for UDP communication. 
	*/
	if((fd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		printf("Error: socket()\n");
		exit(1);
	}

	memset((void *)&addr,(int)'\0', sizeof(addr));

	addr.sin_family = AF_INET; /* AF_INET -- IPv4 internet protocols (TCP,UDP) */
	addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Accept datagrams on any Internet interface on the system */
	addr.sin_port = htons(port);

	/* 
	When a process wants to receive new incoming packets or connections,
    it should bind a socket to a local interface address. 
    */
	if(bind(fd,(struct sockaddr*) &addr, sizeof(addr)) == -1)
	{
		printf("Error: bind()\nTheres already a UDP server in the specified port.\n");
		exit(1);
	}

	/* LOG */
	sprintf(log_msg, "Started server on port %d", port);
	log_action(SERVER_LOG, log_msg, 2);

	while(1)
	{
		addrlen = sizeof(addr);

		/* Receive message from a socket */
		nread = recvfrom(fd,buffer,BUFFER_SIZE,0,(struct sockaddr*) &addr,(unsigned int *) &addrlen);
		parsed_buffer = parseString(buffer, " ");

		/* LOG */
		bzero(log_msg, 60);
		sprintf(log_msg, "Received request \"%s\" from \"%s\" at \"%s\"", buffer,
		 gethostbyaddr((char *)&addr.sin_addr, sizeof(struct in_addr),AF_INET)->h_name,
		 inet_ntoa(addr.sin_addr));
		log_action(SERVER_LOG, log_msg, 0);

		if(nread == -1)
		{
			printf("Error: recvfrom()\n");
			exit(1);
		}

		if(strcmp(parsed_buffer[0],"TQR") == 0)
		{	
			memset((void *)msg,(int)'\0',strlen(msg));
			/* TODO this should follow protocol with a AWT response */
			strcpy(msg, readFromFile("topics.txt")); /* CORRECT for buffer overflow */
		}
		else if (strcmp(parsed_buffer[0], "TER") == 0){
			/* TODO Look for available TES server */
			strcpy(msg, parsed_buffer[1]);
		}
		else
			/* Reply with an error response */
			strcpy(msg,"ERR");

		/* Send a message on a socket */
		if((sendto(fd,msg, sizeof(msg),0,(struct sockaddr*) &addr, addrlen)) == -1)
		{
			printf("Error: sendto()\n");
			exit(1);
		}
	}

	close(fd);
}
