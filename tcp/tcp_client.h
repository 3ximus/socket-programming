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
int send_tcp_request(int fd, const unsigned char *request);

/* COMMMENTS */
unsigned char *receive_tcp_reply(int fd, int);

/* --------------------------- */

int start_tcp_client(const char *ip_addr, int port){
	int fd, n;
	struct sockaddr_in addr;

	/* Ignora o sinal SIGPIPE */
	if((signal(SIGPIPE,SIG_IGN)) == SIG_ERR)
		exit(1);

	/* Atribuicao da socket TCP */
	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("[ERROR] socket()");
		exit(1);
	}

	memset((void *)&addr,(int)'\0',sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip_addr);
	addr.sin_port = htons(port);

	/* Initiate a connection on a socket */
	if((n = connect(fd, (struct sockaddr*)&addr, sizeof(addr)))== -1){
		return -1;
	}
	return fd;
}

int send_tcp_request(int fd, const unsigned char *request){
	int nbytes = strlen((char*)request), nleft, nwritten;

	nleft = nbytes;

	while(nleft > 0){
		if((nwritten = write(fd, request, nleft)) <= 0){	
			perror("[ERROR] write()");
			exit(1);
		}

		nleft -= nwritten;
		request += nwritten;

	}
	return nwritten;
}

unsigned char *receive_tcp_reply(int fd, int reply_buff_size){

	int nread/*, chr*/;
	unsigned char *reply_buffer = (unsigned char *)malloc(reply_buff_size * sizeof(unsigned char));
	unsigned char *reply_ptr;
	unsigned char *reply  = (unsigned char *)malloc(reply_buff_size * sizeof(unsigned char));

	/* point to the beginning reply */
	reply_ptr = reply;
 	
	while(1){
		if((nread = read(fd, reply_buffer, reply_buff_size)) == -1){
			perror("[ERROR] read()");
			exit(1);
		}
		/* copy the buffer to the reply ptr */
		memcpy(reply_ptr, reply_buffer, nread);
		/* move the "writing head" forward */
		reply_ptr += nread;
		
		if (reply_buff_size > REPLY_BUFFER_128 && reply_buffer[nread - 1] == '\n')
			break;

		/* if we read 0 bytes means EOF reached */
		if(nread == 0)
			break;
	}
	close(fd);
	free(reply_buffer);
	return reply;
}
