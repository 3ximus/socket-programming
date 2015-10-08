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
	int fd, newfd, addrlen, afd, maxfd, counter; /* sockets */
	fd_set rfds; /* set of file descriptors to be read */
	enum{IDLE, BUSY} state;

	int bytes_read, bytes_written, bytes_to_write = 0; /* I/O */
	int child_pid = 0, topic; /* other */

	char received_buffer[REQUEST_BUFFER_64], request[REQUEST_BUFFER_64], *request_ptr;
	char **parsed_request = (char **)malloc(sizeof(char *) * 10); /* must be freed */
	unsigned char *reply_ptr, *reply_msg = NULL; /* must be freed */
	struct sockaddr_in addr;

	/* catch exit signal */
	if (signal(SIGTERM, sigterm_handler) == SIG_ERR) perror("[ERROR] Couldnt catch SIGTERM");

	/* TCP socket atribution create an endpoint for TCP communication. */
	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
		perror("Error: socket()");
		exit(1);
	}

	*socket_fd = fd;
	memset((void *)&addr,(int)'\0', sizeof(addr));
	addr.sin_family = AF_INET; /* AF_INET -- IPv4 internet protocols (TCP,UDP) */
	addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Accept datagrams on any Internet interface on the system */
	addr.sin_port = htons(port);
	topic = port % 100; /* get topic number from the port where it is running */

	/* bind to a socket file descriptor to a given port */
	if(bind(fd,(struct sockaddr*) &addr, sizeof(addr)) == -1){
		perror("Error: bind()\nTheres already a TCP server in the specified port.");
		exit(1);
	}

	printf("\rTES server listening on port %d\n> ", port); fflush(stdout);

	/* listen for connections on a socket */
	if(listen(fd,5) == -1) {
		perror("Error: listen()");
		exit(1);
	}

	child_pid = fork(); /* fork to a new process, leaving the interface to the user */
	/* The while loop is only run on the child process, leaving the parent to return the child_pid value */
	if(child_pid == 0) {
		struct user_table *user_info = (struct user_table *)malloc(sizeof(struct user_table));
		state = IDLE; /* start in an idle state */

		while(1) {
			FD_ZERO(&rfds); /* zero out the set */
			FD_SET(fd, &rfds); /* add the socket fd to the set */
			maxfd = fd;

			if (state == BUSY){
				FD_SET(afd, &rfds); /* add the active file descriptor to the set */
				maxfd = MAX(maxfd, afd);
			}
			/* blocks until one of the file descriptors in the set rfds is ready to be read and returns the number of file descriptors ready */
			counter = select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval*)NULL);
			if (counter <= 0) { /* handle error */
				perror("[ERROR] Error on select.");
				exit(1);
			}

			for (;counter;counter--){
				if (FD_ISSET(fd, &rfds)){ /* fd is ready to be read */
					addrlen = sizeof(addr); /* sets the addrlen to be the size of the socket address structure */
					/* accepts a connection on a socket */
					if((newfd = accept(fd,(struct sockaddr*)&addr,(unsigned int*)&addrlen)) == -1){
						printf("Error: accept()\n");
						exit(1);
					}
					
					switch(state){ /* switch states after we accepted an incomming communication */
						case BUSY:
							/* notify client that this is busy ? */
							close(newfd);
							break;
						case IDLE:
							afd = newfd; /* the new active file descriptor is the new socket fd */
							state = BUSY; /* switch state */
							break;
					}
				} /* if */
				else if (FD_ISSET(afd, &rfds)){
					/* set in the begining */
					memset((void*)request, '\0', REQUEST_BUFFER_64);
					request_ptr = request;

					/* reading cycle */
					while((bytes_read = read(newfd, received_buffer, REQUEST_BUFFER_64)) != 0){
						if(bytes_read == -1){ /* handle error */
							perror("Error: read()");
							exit(1);
						}
						memcpy(request_ptr, received_buffer, bytes_read);
						request_ptr += bytes_read;
						parse_string(parsed_request, request, " \n", 10); /* parse request */

						/* Print request */
						printf("\rGot Request %s from %s:%d\n> ", parsed_request[0], inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
						fflush(stdout);

						/* Handle requests */
						if (strcmp(parsed_request[0], "RQT") == 0){
							time_t now;
							time(&now);

							/* set expiration time */
							user_info->deadline = now + 600; /* time now + 10 minutes */
							user_info->sid = atoi(parsed_request[1]);
							reply_msg = AQT_reply(user_info, now, topic);
							bytes_to_write = REPLY_BUFFER_OVER_9000;
							printf("\rSending AQT reply: TOO BIG TO SHOW\n> ");
							fflush(stdout);
							break;
						}
						else if (strcmp(parsed_request[0], "RQS") == 0) {
							unsigned char *server_reply = NULL;
							struct sockaddr_in udp_addr;
							struct ecp_server ecp;
							int udp_fd;
							time_t now;
							time(&now);

							/* check if sid and qid dont match */
							if (user_info->sid != atoi(parsed_request[1]) && strcmp(parsed_request[2], user_info->qid) != 0){
								reply_msg = ERR_reply();
								bytes_to_write = 5;
								printf("\rSending ERR reply\n> ");
								fflush(stdout);
								break;
							}
							if (user_info->deadline < now)
								user_info->score = -1; /* deadline missed */
							else 
								user_info->score = calculate_score(topic, user_info->internal_qid, parsed_request);

							reply_msg = AQS_reply(user_info->qid, user_info->score);
							bytes_to_write = REPLY_BUFFER_32;
							printf("\rSending AQS reply: %s> ", reply_msg);
							fflush(stdout);

							/* ECP */
							/* initiate a UDP client */
							ecp.port = DEFAULT_PORT_ECP;
							gethostname((char *)ecp.name, sizeof(ecp.name));
							udp_fd = start_udp_client(&udp_addr, &ecp);
							
							/* contact ecp */
							printf("\rSending IQR request\n> ");
							fflush(stdout);
							server_reply = IQR_request(udp_fd, &udp_addr, user_info->sid, user_info->qid, topic, user_info->score);
							printf("\rGot AWI reply: %s> ", server_reply);
							fflush(stdout);

							free(server_reply);
							close(udp_fd);
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

					reply_ptr = reply_msg; /* point to begining of reply */

					while(bytes_to_write > 0){	/* writing cycle */
						if((bytes_written = write(newfd, reply_ptr, bytes_to_write)) <= 0){
							perror("Error: write()");
							exit(1);
						}
						bytes_to_write -= bytes_written;
						reply_ptr += bytes_written;
					}
					free(reply_msg);
					close(afd);
					state = IDLE; /* we will be idle again */
				} /* else if */
			} /* for */
		} /* while */
		free(user_info);
	}
	free(parsed_request);
	/* This is left in here just in case, because socket is closed on the ecp_server_interface */
	close(fd);
	return child_pid;
}
