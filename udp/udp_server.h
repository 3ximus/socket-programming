#include "../resources.h"
#include "../comm_protocol.h"

/*
 * UDP Server API
 */
void UDPserver(int port){
	int fd, addrlen, nread;
	struct sockaddr_in addr;
	char received_buffer[REQUEST_BUFFER_SIZE];
	char **parsed_request; /* must be freed */
	char log_msg[60];
	unsigned char *reply_msg = NULL; /* must be freed */

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
	if(bind(fd,(struct sockaddr*) &addr, sizeof(addr)) == -1){
		printf("Error: bind()\nTheres already a UDP server in the specified port.\n");
		exit(1);
	}

	/* LOG */
	bzero(log_msg, 60);
	sprintf(log_msg, "Started server on port %d", port);
	log_action(SERVER_LOG, log_msg, 2);

	while(1){
		
		/* Receive message from a socket */
		nread = recvfrom(fd, received_buffer,BUFFER_SIZE,0,(struct sockaddr*) &addr,(unsigned int *) &addrlen);
		addrlen = sizeof(addr);

		/* parsed received buffer */
		parsed_request = parseString(received_buffer, "\n");
		parsed_request = parseString(received_buffer, " ");

		/* LOG */
		bzero(log_msg, 60);
		sprintf(log_msg, "Received request \"%s\" from \"%s\" at \"%s\"", parsed_request[0],
		 gethostbyaddr((char *)&addr.sin_addr, sizeof(struct in_addr),AF_INET)->h_name,
		 inet_ntoa(addr.sin_addr));
		log_action(SERVER_LOG, log_msg, 0);

		if(nread == -1){
			perror("Error: recvfrom()");
			free(parsed_request);
			exit(1);
		}

		if(strcmp(parsed_request[0],"TQR") == 0){
			reply_msg = AWT_reply();
			printf("%s", reply_msg);
		}
		else if (strcmp(parsed_request[0], "TER") == 0){
			/*reply_msg = AWTES_reply();*/

		}
		else
			/* Reply with an error response */
			strcpy((char *)reply_msg,"ERR\n");

		/* Send reply message */
		if((sendto(fd, reply_msg, strlen((char *)reply_msg),0,(struct sockaddr*) &addr, addrlen)) == -1){
			perror("Error: sendto()");
			exit(1);
		}

		/* Free allocated memory */
		free(reply_msg);
		free(parsed_request);
	}

	close(fd);
}
