#include "../resources.h"
#include "../comm_protocol.h"

/*
 * Starts a server on given or default port
 * Creates a new socket using TCP and IPv4 and stores its file descriptor on the 2nd argument
 * It accepts datagrams on any Internet interface.
 */
int start_tcp_server(int, int*);

/* Handle SIGTERM */
void sigterm_handler(int x){
	printf("[SERVER_MSG] Arrrhhh!!! You killed me with signal %d!!!\n", x);

	exit(0);
}

int start_tcp_server(int port, int *socket_fd)
{
	int fd, newfd, addrlen, n, nw, child_pid = 0;
	char received_buffer[BUFFER_32];
	char log_msg[60];
	unsigned char *reply_msg = NULL; /* must be freed */
	struct sockaddr_in addr;
	char **parsed_request; /* must be freed */


	/* TCP socket atribution create an endpoint for TCP communication. */
	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		printf("Error: socket()\n");
		exit(1);
	}

	*socket_fd = fd;

	memset((void *)&addr,(int)'\0', sizeof(addr));

	addr.sin_family = AF_INET; /* AF_INET -- IPv4 internet protocols (TCP,UDP) */
	addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Accept datagrams on any Internet interface on the system */
	addr.sin_port = htons(port);

	/* catch exit signal */
	if (signal(SIGTERM, sigterm_handler) == SIG_ERR)
		perror("[ERROR] Couldnt catch SIGTERM");

	/* 
	When a process wants to receive new incoming packets or connections,
    it should bind a socket to a local interface address. 
    */
	if(bind(fd,(struct sockaddr*) &addr, sizeof(addr)) == -1){
		printf("Error: bind()\nTheres already a TCP server in the specified port.\n");
		exit(1);
	}

	/* LOG */
	bzero(log_msg, 60);
	sprintf(log_msg, "Started server on port %d", port);
	log_action(TCP_SERVER_LOG, log_msg, 2);


	printf("\rTES server listening on port %d\n> ", port);
	fflush(stdout);

	/* listen for connections on a socket */
	if(listen(fd,5) == -1)
	{
		printf("Error: listen()\n");
		exit(1);
	}

	child_pid = fork();

	/* The while loop is only run on the child process, leaving the parent to return the child_pid value */
	if(child_pid == 0)
	{
		while(1) 
		{
			addrlen = sizeof(addr);

			/* accepts a connection on a socket */
			if((newfd = accept(fd,(struct sockaddr*)&addr,(unsigned int*)&addrlen)) == -1)
			{
				printf("Error: accept()\n");
				exit(1);
			}

			while((n = read(newfd,received_buffer,BUFFER_32)) != 0)
			{
				if(n == -1)
				{
					printf("Error: read()\n");
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
				log_action(TCP_SERVER_LOG, log_msg, 0);

				/* Handle requests */
				if (strcmp(parsed_request[0], "RQT") == 0){
					reply_msg = AQT_reply(1001);

					/* remove \n at the end */
					reply_msg[strlen((char *)reply_msg) - 1] = '\0';

					printf("\rSending AWT reply \"%s\"\n> ", reply_msg);
					fflush(stdout);
				}
				else 
					reply_msg = ERR_reply();

				while(n > 0)
				{	
					if((nw = write(newfd, reply_msg, n)) <= 0)
					{
						printf("Error: write()\n");
						exit(1);
					}
					n -= nw;
					reply_msg += nw;
				}

				free(reply_msg);
				free(parsed_request);
				close(newfd);
			}
		}	
	}
	/* This is left in here just in case, because socket is closed on the ecp_server_interface */
	close(fd);
	return child_pid;
}
