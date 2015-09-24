#ifndef COMM_PROTOCOL_H
#define COMM_PROTOCOL_H

/*
 * This contains a list of requests and replies that define the project
 * 	protocol for this project.
 * All requests must be processed by calling <protocol>_request, server should
 *  never be contacted directly.
 * These functions don't free any memory passed to them, any allocated memory should be
 *  freed after the call.
 * Some functions return dinamically allocated memory that needs to be freed.
 */

#include "udp/udp_client.h"
#include "tcp/tcp_client.h"
#include "resources.h"

/* FUNCTION INDEX - DECLARATIONS */

/* REQUESTS
 * These are client side functions, only to be called on the client 
 */

/*
 * Used to request list of topics from ECP server
 * Sends a TQR request to server and returns server reply
 * Reply must be freed
 */
unsigned char *TQR_request(int, const struct sockaddr_in*);

/*
 * Used to request the TES server IP and PORT that host a questionarie passed on topic_number
 * Sends a TER request followed by topic_number to ECP server and returns server reply
 * Reply must be freed
 */
unsigned char *TER_request(int, const char *topic_number, const struct sockaddr_in*);

/* COMMENTS */
unsigned char *RQT_request(int, const char *student_id, const struct sockaddr_in*);

/* COMMENTS */
unsigned char *RQS_request(int, const char *sid_answer_sequence, const struct sockaddr_in*);

/* REPLIES
 * These are server side functions, only to be called on the server
 */

/*
 * Used to reply to an TQR request, listing all topics in file topics.txt.
 * Sends a AWT reply in the format AWT n Tn, where n is the number of topics
 * 	and Tn is the name of each topic.
 */
unsigned char *AWT_reply();

/*
 * COMMENTS
 */
unsigned char *AWTES_reply();


/* UTILS */
unsigned char *check_reply_for_errors(unsigned char *);

/* ------------------------------------------- */

/* FUNCTION DEFENITIONS */

/* ----- REQUESTS -------- */

unsigned char *TQR_request(int fd, const struct sockaddr_in* addr){
	unsigned char *server_reply = NULL;
	char *request = "TQR\n";	

	/* contact ecp server with TQR request */
	send_udp_request(fd, (unsigned char *)request, addr);
	server_reply = receive_udp_reply(fd, addr);
	server_reply = check_reply_for_errors(server_reply);
	return server_reply;
}

unsigned char *TER_request(int fd, const char *topic_number, const struct sockaddr_in* addr){
	unsigned char *server_reply = NULL;
	char request[REQUEST_BUFFER_SIZE] = "TER ";
	char *endptr;

	/* check if topic_number is in fact a number and is between 0 and TOPIC_NR (99) */
	/* This must be implemented on the server */
	long val = strtol(topic_number, &endptr, 10);
	if (endptr == topic_number || val < 1 || val > TOPIC_NR)
		printf("[ERROR] Request with no valid number\n");

	/* attach topic number to request */
	strncat(request, topic_number, 6);
	/* terminate request */
	strncat(request, "\n", 1);

	/* contact server with built request */
	send_udp_request(fd, (unsigned char *)request, addr);
	server_reply = receive_udp_reply(fd, addr);
	server_reply = check_reply_for_errors(server_reply);
	return server_reply;
}

/* ----- REPLIES -------- */


unsigned char* AWT_reply(){
	int ntopic, i;
	char str_ntopic[2];
	char **file_content = NULL, **parsed_line = NULL, *raw_content = NULL;
	unsigned char *server_reply = (unsigned char*)malloc(BIG_REPLY_BUFFER * sizeof(unsigned char));
	bzero(server_reply, BIG_REPLY_BUFFER);

	strncpy((char *)server_reply, "AWT ", 4);

	/* raw data from topics file */
	raw_content = readFromFile(TOPICS_FILE);

	/* organize topics (1 per line) */
	file_content = parseString(raw_content, "\n");

	/* count lines in file */
	ntopic = 0;
	while (file_content[ntopic] != NULL)
		ntopic++;

	/* place number of topics in the reply */
	sprintf(str_ntopic, "%d", ntopic);
	strcat((char *)server_reply, str_ntopic);

	/* add titles to the reply */
	for(i = 0; i < ntopic; i++){
		parsed_line = parseString(file_content[i], " ");
		strcat((char *)server_reply, " ");
		strcat((char *)server_reply, parsed_line[0]);
		free(parsed_line);
	}

	/* Frees memory */
	free(file_content);
	free(raw_content);

	strcat((char *)server_reply, "\n");

	/* TODO This must make the call to sendto a socket fd */
	return server_reply;
}

unsigned char *AWTES_reply(){
	unsigned char *server_reply = (unsigned char*)malloc(SMALL_REPLY_BUFFER * sizeof(unsigned char));
	/* TODO */
	return server_reply;
}

unsigned char *check_reply_for_errors(unsigned char *server_reply){
	/* TODO check if reply is an ERR or an EOF and react accordingly */
	return server_reply;
}

#endif
