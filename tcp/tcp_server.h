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

int start_tcp_server(int port, int *socket_fd) {
	int fd, newfd, addrlen, n, nw, child_pid = 0;
	char received_buffer[BUFFER_32];
	unsigned char *reply_ptr;
	unsigned char *reply_msg = NULL; /* must be freed */
	struct sockaddr_in addr;
	char **parsed_request; /* must be freed */


	/* TCP socket atribution create an endpoint for TCP communication. */
	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
		perror("Error: socket()\n");
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
		perror("Error: bind()\nTheres already a TCP server in the specified port.\n");
		exit(1);
	}

	printf("\rTES server listening on port %d\n> ", port);
	fflush(stdout);

	/* listen for connections on a socket */
	if(listen(fd,5) == -1) {
		perror("Error: listen()\n");
		exit(1);
	}

	child_pid = fork();

	/* The while loop is only run on the child process, leaving the parent to return the child_pid value */
	if(child_pid == 0) {
		while(1) {
			addrlen = sizeof(addr);

			/* accepts a connection on a socket */
			if((newfd = accept(fd,(struct sockaddr*)&addr,(unsigned int*)&addrlen)) == -1){
				printf("Error: accept()\n");
				exit(1);
			}

			while((n = read(newfd,received_buffer,BUFFER_32)) != 0){
				if(n == -1)
				{
					perror("Error: read()\n");
					exit(1);
				}
				
				/* parsed received buffer */
				parsed_request = parseString(received_buffer, "\n");
				parsed_request = parseString(received_buffer, " ");

				/* Print request */
				printf("\rGot Request %s from %s:%d\n> ", parsed_request[0], inet_ntoa(addr.sin_addr),
				 ntohs(addr.sin_port));
				fflush(stdout);

				/* Handle requests */
				if (strcmp(parsed_request[0], "RQT") == 0){
					struct tm expiration_time;
					int qid;

					memset((void *)&expiration_time, 0, sizeof(struct tm));

					/* set expiration time */
					expiration_time.tm_min = 10;
					/* set qid */
					qid = 1001;

					/* TODO PASS CORRECT QID AND TIME DELAY*/
					reply_msg = AQT_reply(qid, (const struct tm *)&expiration_time);

					/* remove \n at the end */
					reply_msg[strlen((char *)reply_msg) - 1] = '\0';

					printf("\rSending AQT reply \"%s\"\n> ", reply_msg);
					fflush(stdout);
				}
				else 
					reply_msg = ERR_reply();

				/* point to begining of reply */
				reply_ptr = reply_msg;
				/* set number of bytes of reply */
				n = REPLY_BUFFER_1024;
				while(n > 0)
				{	
					if((nw = write(newfd, reply_ptr, n)) <= 0)
					{
						perror("Error: write()\n");
						exit(1);
					}
					n -= nw;
					reply_ptr += nw;
				}

				free(reply_msg);
				free(parsed_request);
			}
			close(newfd);
		}	
	}
	/* This is left in here just in case, because socket is closed on the ecp_server_interface */
	close(fd);
	return child_pid;
}
