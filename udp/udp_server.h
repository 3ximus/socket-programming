#include "../resources.h"
#include "../comm_protocol.h"



/*
 * Starts a server on given or default port
 * Creates a new socket using UDP and IPv4 and stores its file descriptor on the 2nd argument
 * It accepts datagrams on any Internet interface
 * Binds the newly created socket to the defined interface (in our case any available on the system)
 * This forks the process and waits for incoming requests in the child process, returning the pid of the child.
 * Important: If child_pid is not caught the server is lost and needs to be shutdown manually
 * Note: Produces LOG entry
 */
int start_udp_server(int, int*);


/* ----------------------------------------------- */


/* Handle SIGTERM */
void sigterm_handler(int x){
	printf("[SERVER_MSG] Arrrhhh!!! You killed me with signal %d!!!\n", x);

	exit(0);
}


int start_udp_server(int port, int *socket_fd){
	int fd, addrlen, nread, child_pid = 0;
	char received_buffer[REQUEST_BUFFER_32];
	char log_msg[60];
	char **parsed_request; /* must be freed */
	unsigned char *reply_msg = NULL; /* must be freed */
	struct sockaddr_in addr;

	/* Udp socket atribution create an endpoint for UDP communication. */
	if((fd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		printf("Error: socket()\n");
		exit(1);
	}

	*socket_fd = fd;

	memset((void *)&addr,(int)'\0', sizeof(addr));

	addr.sin_family = AF_INET; /* AF_INET -- IPv4 internet protocols (TCP,UDP) */
	addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Accept datagrams on any Internet interface on the system */
	addr.sin_port = htons(port);

	if (signal(SIGTERM, sigterm_handler) == SIG_ERR)
			perror("[ERROR] Couldnt catch SIGTERM");

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
	log_action(UDP_SERVER_LOG, log_msg, 2);

	printf("\rECP server listening on port %d\n> ", port);
	fflush(stdout);
	

	child_pid = fork();

	/* The while loop is only run on the child process, leaving the parent to return the child_pid value */
	while(child_pid == 0){

		/* Receive request message */
		addrlen = sizeof(addr);
		nread = recvfrom(fd, received_buffer,BUFFER_2048,0,(struct sockaddr*) &addr,(unsigned int *) &addrlen);

		if(nread == -1){
			perror("Error: recvfrom()");
			free(parsed_request);
			exit(1);
		}

		/* parsed received buffer */
		parsed_request = parseString(received_buffer, "\n");
		parsed_request = parseString(received_buffer, " ");
		
		/* Print request */
		printf("\rGot Request %s from %s:%d\n> ", parsed_request[0], inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		fflush(stdout);
		
		/* LOG */		
		bzero(log_msg, 60);
		sprintf(log_msg, "Received request \"%s\" from \"%s\" at \"%s\":%d", parsed_request[0],
	 	gethostbyaddr((char *)&addr.sin_addr, sizeof(struct in_addr),AF_INET)->h_name,
	 	inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		log_action(UDP_SERVER_LOG, log_msg, 0);
	

		if(strcmp(parsed_request[0],"TQR") == 0){		
			reply_msg = AWT_reply();
			
			/* remove \n at the end */
			reply_msg[strlen((char *)reply_msg) - 1] = '\0';
			
			printf("\rSending AWT reply \"%s\"\n> ", reply_msg);
			fflush(stdout);
		}
		else if (strcmp(parsed_request[0], "TER") == 0){
			int topic_nr = atoi(parsed_request[1]);

			reply_msg = AWTES_reply(topic_nr);

			/* remove \n at the end */
			reply_msg[strlen((char *)reply_msg) - 1] = '\0';

			printf("\rSending AWTES reply \"%s\"\n> ", reply_msg);
			fflush(stdout);

		}
		else
			reply_msg = ERR_reply();

		/* Send reply message */
		if((sendto(fd, reply_msg, strlen((char *)reply_msg),0,(struct sockaddr*) &addr, addrlen)) == -1){
			perror("Error: sendto()");
			exit(1);
		}

		/* Free allocated memory */
		free(reply_msg);
		free(parsed_request);
	}

	/* This is left in here just in case, because socket is closed on the ecp_server_interface */
	close(fd);
	return child_pid;
}
