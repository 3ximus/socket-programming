#include "../resources.h"

/*
 * Create a socket for a new udp client
 * Return socket file descriptor
 * Builds the sockaddr_in argument to contain the ecp server IP (IPv4) and Port
 * Always close the socket
 */
int start_udp_client();

/*
 * Send a request to a given ecp server defined in a sockaddr_in
 */
int send_udp_request(int, const unsigned char *, const struct sockaddr_in *);

/*
 * Check the server reply for a given request
 * Returns pointer to allocated server reply, must be freed
 */
unsigned char *receive_udp_reply(int fd, const struct sockaddr_in *addr);


/* --------------------------- */

int start_udp_client(struct sockaddr_in *addr, const struct server *ecp_server){
	int fd;
	/*Atribuicao da socket UDP */
	if((fd = socket(AF_INET,SOCK_DGRAM,0)) == -1){
		perror("Error: creating socket()\n");
		exit(1);
	}

	memset((void *)addr, (int)'\0', sizeof(struct sockaddr_in));

	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = getHostIP((char*)ecp_server->name);
	addr->sin_port = htons(ecp_server->port);

	return fd;
}

int send_udp_request(int fd, const unsigned char *request, const struct sockaddr_in *addr){
	int n;

	/* Send server request */
	if((n = sendto(fd, request, strlen((char*)request), 0,(struct sockaddr *)addr, sizeof(struct sockaddr))) == -1)
	{
		perror("Error: sendto()\n");
		close(fd);
		exit(1);
	}
	return n;
}

unsigned char *receive_udp_reply(int fd, const struct sockaddr_in *addr){
	int n;
	socklen_t slen = sizeof(struct sockaddr_in);
	unsigned char server_reply[BIG_REPLY_BUFFER];
	unsigned char *returned_server_reply = (unsigned char *) malloc(BIG_REPLY_BUFFER * sizeof(unsigned char));

	/* receive server reply */
	if((n = recvfrom(fd, server_reply, BIG_REPLY_BUFFER, 0, (struct sockaddr*)addr,&slen)) == -1){
		perror("Error: recvfrom()\n");
		/*free(returned_server_reply);*/
		close(fd);
		exit(1);
	}

	memcpy(returned_server_reply, server_reply, n);

	return returned_server_reply;
}
