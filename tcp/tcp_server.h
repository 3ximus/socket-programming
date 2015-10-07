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
	int fd, newfd, addrlen, bytes_read, bytes_written, bytes_to_write = 0, child_pid = 0;
	char received_buffer[REQUEST_BUFFER_64], qid[BUFFER_32], request[REQUEST_BUFFER_64], *request_ptr;
	unsigned char *reply_ptr;
	unsigned char *reply_msg = NULL; /* must be freed */
	struct sockaddr_in addr;
	int sid, topic;
	char **parsed_request = (char **)malloc(sizeof(char *) * 10); /* must be freed */

	memset(qid, '\0', BUFFER_32);

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
			/* set in the begining */
			memset(request, '\0', REQUEST_BUFFER_64);
			request_ptr = request;
			

			/* reading cycle */
			while((bytes_read = read(newfd, received_buffer, REQUEST_BUFFER_64)) != 0){
				if(bytes_read == -1){
					perror("Error: read()\n");
					exit(1);
				}
				/* left here in case we decide to separate read routine */
				memcpy(request_ptr, received_buffer, bytes_read);
				request_ptr += bytes_read;
				

				/* parse request */
				parse_string(parsed_request, request, " \n", 10);

				/* Print request */
				printf("\rGot Request %s from %s:%d\n> ", parsed_request[0], inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
				fflush(stdout);

				/* Handle requests */
				if (strcmp(parsed_request[0], "RQT") == 0){
					struct tm expiration_time;
					memset((void *)&expiration_time, 0, sizeof(struct tm));

					/* set expiration time */
					expiration_time.tm_min = 10;

					sid = atoi(parsed_request[1]);

					/* TODO PASS CORRECT QID AND TIME DELAY*/
					reply_msg = AQT_reply(sid, (const struct tm *)&expiration_time, qid);
					bytes_to_write = REPLY_BUFFER_OVER_9000;
					printf("\rSending AQT reply: TOO BIG TO SHOW\n> ");
					fflush(stdout);
					break;
				}
				else if (strcmp(parsed_request[0], "RQS") == 0) {
					unsigned char *server_reply = NULL;
					struct sockaddr_in udp_addr;
					struct server ecp_server;
					int score, udp_fd;
					/* check if sid and qid dont match */

					if (sid != atoi(parsed_request[1]) && strcmp(parsed_request[2], qid) != 0){
						reply_msg = ERR_reply();
						bytes_to_write = 5;
						printf("\rSending ERR reply\n> ");
						fflush(stdout);
						break;
					}
					/* TODO check deadline */

					/* TODO Calculate score */
					score = 1000;
					topic = 1;

					reply_msg = AQS_reply(qid, score);
					bytes_to_write = REPLY_BUFFER_32;
					printf("\rSending AQS reply: %s> ", reply_msg);
					fflush(stdout);

					/* ECP */
					/* initiate a UDP client */
					ecp_server.port = DEFAULT_PORT_ECP;
					gethostname((char *)ecp_server.name, sizeof(ecp_server.name));
					udp_fd = start_udp_client(&udp_addr, &ecp_server);
					
					/* contact ecp */
					server_reply = IQR_request(udp_fd, &udp_addr, sid, qid, topic, score);
					printf("\rSending IQR request: %s> ",server_reply);
					fflush(stdout);

					free(server_reply);
					
					break;
				}
				else{
					reply_msg = ERR_reply();
					bytes_to_write = 5;
					printf("\rSending ERR reply\n> ");
					fflush(stdout);
					break;
				}
			}

			/* point to begining of reply */
			reply_ptr = reply_msg;

			/* writing cycle */
			while(bytes_to_write > 0)
			{	
				if((bytes_written = write(newfd, reply_ptr, bytes_to_write)) <= 0)
				{
					perror("Error: write()\n");
					exit(1);
				}
				bytes_to_write -= bytes_written;
				reply_ptr += bytes_written;
			}
			free(reply_msg);
			close(newfd);
		}	
	}
	free(parsed_request);
	/* This is left in here just in case, because socket is closed on the ecp_server_interface */
	close(fd);
	return child_pid;
}
