#include "../resources.h"
#include "../comm_protocol.h"

/*
 * Starts a server on given or default port
 * Creates a new socket using TCP and IPv4 and stores its file descriptor on the 2nd argument
 * It accepts datagrams on any Internet interface.
 */
int start_tcp_server(int, int*);

int start_tcp_server(int port, int *socket_fd)
{
	int fd, newfd, addrlen, n, nw, child_pid = 0;
	char *ptr, buffer[BUFFER_2048];
	struct sockaddr_in addr;


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

	/* 
	When a process wants to receive new incoming packets or connections,
    it should bind a socket to a local interface address. 
    */
	if(bind(fd,(struct sockaddr*) &addr, sizeof(addr)) == -1){
		printf("Error: bind()\nTheres already a TCP server in the specified port.\n");
		exit(1);
	}

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

			while((n = read(newfd,buffer,BUFFER_2048))!=0)
			{
				if(n == -1)
				{
					printf("Error: read()\n");
					exit(1);
				}
			}	
			printf("\rGot message %s\n> ", buffer);
			fflush(stdout);

			ptr = &buffer[0];
			while(n > 0)
			{	
				if((nw = write(newfd,ptr,n)) <= 0)
				{
					printf("Error: write()\n");
					exit(1);
				}
				n -= nw;
				ptr += nw;
			}


			close(newfd);
		}	
	}
	/* This is left in here just in case, because socket is closed on the ecp_server_interface */
	close(fd);
	return child_pid;
}
