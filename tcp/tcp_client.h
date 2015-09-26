#include "../resources.h"

/*
 * Create a socket for a new tcp client
 * Return socket file descriptor
 * Builds the sockaddr_in argument to contain the ecp server IP (IPv4) and Port
 * Always close the socket
 */

int start_tcp_client(const char *ip_addr, int port);

/*
 *  TODO:
 */
int send_tcp_request(int fd, const char *request);

/* COMMMENTS */
unsigned char *receive_tcp_reply(int fd);

/* --------------------------- */

int start_tcp_client(const char *ip_addr, int port)
{
	int fd, n;
	struct sockaddr_in addr;

	/* Ignora o sinal SIGPIPE */
	if((signal(SIGPIPE,SIG_IGN)) == SIG_ERR)
		exit(1);

	/* Atribuicao da socket TCP */
	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		printf("Error: socket()\n");
		exit(1);
	}

	memset((void *)&addr,(int)'\0',sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip_addr);
	addr.sin_port = htons(port);

	/* Initiate a connection on a socket */
	if((n = connect(fd, (struct sockaddr*)&addr, sizeof(addr)))== -1)
	{
		printf("Error: connect().\nThere is no TCP server listening on that port.\n");
		exit(1);
	}
	return fd;
}

int send_tcp_request(int fd, const char *request){
	int nbytes = strlen(request), nleft, nwritten;

	nleft = nbytes;

	while(nleft > 0)
	{
		if((nwritten = write(fd, request, nleft)) <= 0)
		{	
			perror("Error: write()\n");
			exit(1);
		}

		nleft -= nwritten;
		request += nwritten;

	}
	return nwritten;
}

unsigned char *receive_tcp_reply(int fd){

	int nleft = REPLY_BUFFER_1024, nread;
	unsigned char *reply_buffer[REPLY_BUFFER_1024], *reply_ptr;
	unsigned char *reply  = (unsigned char *)malloc(REPLY_BUFFER_OVER_9000 * sizeof(unsigned char));

	/* point to the beginning reply */
	reply_ptr = reply;

	while(nleft > 0)
	{
		if((nread = read(fd, reply_buffer, nleft)) == -1)
		{
			perror("Error: read()\n");
			exit(1);
		}
		
		nleft -= nread;
		/* copy the buffer to the reply reply */
		memcpy(reply_ptr, reply_buffer, REPLY_BUFFER_1024);
		/* move the "writing head" forward */
		reply_ptr += nread;
	
		if(nread == 0)
		{
			printf("Closed by peer\n");
			break;
		}
	}
	close(fd);
	return reply;
}