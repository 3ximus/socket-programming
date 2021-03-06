#include "../resources.h"
#include "../comm_protocol.h"

/* Global Variable */
struct user_table *user_info;

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
	int fd, newfd, addrlen, ret; /* socket */
	int bytes_read, bytes_written, bytes_to_write = 0; /* I/O */
	int child_pid = 0, topic; /* other */

	char received_buffer[REQUEST_BUFFER_64], request[REQUEST_BUFFER_64], *request_ptr;
	char **parsed_request = (char **)malloc(sizeof(char *) * 10); /* must be freed */
	unsigned char *reply_ptr, *reply_msg = NULL; /* must be freed */
	struct sockaddr_in addr;

	/* catch signals */
	if (signal(SIGTERM, sigterm_handler) == SIG_ERR) perror("[ERROR] Couldnt catch SIGTERM");
	if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) perror("[ERROR] Couldnt catch SIGCHLD");

	/* TCP socket atribution create an endpoint for TCP communication. */
	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
		perror("Error: socket()");exit(1);}

	*socket_fd = fd;
	memset((void *)&addr,(int)'\0', sizeof(addr));
	addr.sin_family = AF_INET; /* AF_INET -- IPv4 internet protocols (TCP,UDP) */
	addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Accept datagrams on any Internet interface on the system */
	addr.sin_port = htons(port);
	topic = port % 100; /* get topic number from the port where it is running */

	/* bind to a socket file descriptor to a given port */
	if(bind(fd,(struct sockaddr*) &addr, sizeof(addr)) == -1){
		perror("[ERROR] bind()\nThere's already a TCP server in the specified port.");exit(1);}

	printf("\rTES server listening on port %d\n> ", port); fflush(stdout);

	/* listen for connections on a socket */
	if(listen(fd,5) == -1) {perror("[ERROR] listen()");exit(1);}
	if ((child_pid = fork()) == -1) {perror("[ERROR] in fork");exit(1);} /* fork to a new process, leaving the interface to the user */
	/* The while loop is only run on the child process, leaving the parent to return the child_pid value */
	if(child_pid == 0) {
		/* map memory for the golbal variable in order to be shared between parent and child */
		user_info = (struct user_table*)mmap(NULL, USER_TABLE_SIZE *sizeof(struct user_table), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

		while(1) {
			addrlen = sizeof(addr); /* sets the addrlen to be the size of the socket address structure */
			do newfd = accept(fd,(struct sockaddr*)&addr,(unsigned int*)&addrlen); /* accepts a connection on a socket */
			while (newfd == -1 && errno == EINTR); /* while we dont accept a connection */

			if (newfd == -1){perror("[ERROR] accept()");exit(1);}
			memset((void*)request, '\0', REQUEST_BUFFER_64);
			request_ptr = request;

			if ((child_pid = fork()) == -1) {perror("[ERROR] in fork");exit(1);} /* fork to handle each connection */
			else if (child_pid == 0){
				/* reading cycle */
				while((bytes_read = read(newfd, received_buffer, REQUEST_BUFFER_64)) != 0){
					if(bytes_read == -1){ /* handle error */
						perror("[ERROR] read()");exit(1);}
					memcpy(request_ptr, received_buffer, bytes_read);
					request_ptr += bytes_read;
					parse_string(parsed_request, request, " \n", 10); /* parse request */

					/* Print request */
					printf("\rGot Request %s from %s:%d\n> ", parsed_request[0], inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
					fflush(stdout);

					/* Handle requests */
					if (strcmp(parsed_request[0], "RQT") == 0){
						int table_pos;
						time_t now;
						time(&now);

						/* search the sid on the user_info table */
						for (table_pos = 0; table_pos < USER_TABLE_SIZE; table_pos++){
							if (user_info[table_pos].sid == 0){
								user_info[table_pos].sid = atoi(parsed_request[1]);
								user_info[table_pos].deadline = now + 600; /* time now + 10 minutes */
								break;
							}
						}

						/* set expiration time */
						reply_msg = AQT_reply(&user_info[table_pos], now, topic);
						bytes_to_write = REPLY_BUFFER_OVER_9000;
						printf("\rSending AQT reply: TOO BIG TO SHOW\n> ");fflush(stdout);
						break;
					}
					else if (strcmp(parsed_request[0], "RQS") == 0) {
						unsigned char *server_reply = NULL;
						struct sockaddr_in udp_addr;
						struct ecp_server ecp;
						int udp_fd, table_pos, is_error = 0;
						time_t now;
						time(&now);
						
						/* search the sid on the user_info table */
						for (table_pos = 0; table_pos < USER_TABLE_SIZE; table_pos++){
							if (user_info[table_pos].sid == 0 && table_pos == USER_TABLE_SIZE - 1){
								is_error = 1; /* no sid found and end of table reached */
								break;
							}
							if (user_info[table_pos].sid != atoi(parsed_request[1]))
								continue;
							if (user_info[table_pos].sid == atoi(parsed_request[1]) && strcmp(parsed_request[2], user_info[table_pos].qid) == 0)
								break; /* user found */
						}
						if (is_error == 1){ /* user unexistent or not wating for any reply */
							reply_msg = ERR_reply();
							bytes_to_write = 5;
							is_error = 0;
							printf("\rSending ERR reply\n> ");fflush(stdout);
							break;
						}
						
						if (user_info[table_pos].deadline < now)
							user_info[table_pos].score = -1; /* deadline missed */
						else
							user_info[table_pos].score = calculate_score(topic, user_info[table_pos].internal_qid, parsed_request);

						reply_msg = AQS_reply(user_info[table_pos].qid, user_info[table_pos].score);
						bytes_to_write = REPLY_BUFFER_32;
						printf("\rSending AQS reply: %s> ", reply_msg);
						fflush(stdout);

						ecp.port = DEFAULT_PORT_ECP; /* this may be problematic */
						gethostname((char *)ecp.name, sizeof(ecp.name));
						udp_fd = start_udp_client(&udp_addr, &ecp); /* initiate a UDP client *//* initiate a UDP client */
						
						/* contact ecp */
						printf("\rSending IQR request\n> ");fflush(stdout);
						server_reply = IQR_request(udp_fd, &udp_addr, user_info[table_pos].sid,user_info[table_pos].qid, topic, user_info[table_pos].score);
						printf("\rGot AWI reply: %s> ", server_reply);fflush(stdout);

						/* set this position as available again */
						user_info[table_pos].sid = 0;
						strcpy(user_info[table_pos].qid, "\0");
						user_info[table_pos].deadline = 0;
						user_info[table_pos].score = 0;

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
						perror("Error: write()");exit(1);}
					bytes_to_write -= bytes_written;
					reply_ptr += bytes_written;
				}
				free(reply_msg);
				close(newfd);
				exit(1); /* exit child process */
			} /* else if */
			printf("Launched new communication on PID: %d\n", child_pid);
			do (ret = close(newfd)); while (ret == -1 && errno == EINTR);
			if (ret == -1){perror("[ERROR] Closing file descriptor");exit(1);}
		} /* while */
		munmap(user_info, USER_TABLE_SIZE * sizeof(struct user_table)); /* free the mapped memory */
	}
	free(parsed_request);
	/* This is left in here just in case, because socket is closed on the ecp_server_interface */
	close(fd);
	return child_pid;
}
