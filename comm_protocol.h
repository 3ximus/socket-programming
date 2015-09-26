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
 * Reply must be freed on the client
 */
unsigned char *TQR_request(int, const struct sockaddr_in*);

/*
 * Used to request the TES server IP and PORT that host a questionarie passed on topic_number
 * Sends a TER request followed by topic_number to ECP server and returns server reply
 * Reply must be freed on the client
 */
unsigned char *TER_request(int, const char *topic_number, const struct sockaddr_in*);

/* TODO */
unsigned char *RQT_request(int, int);

/* TODO */
unsigned char *RQS_request(int, int, const char *sid_answer_sequence);

/* REPLIES
 * These are server side functions, only to be called on the server
 */

/*
 * Used to reply to an TQR request, listing all topics in file topics.txt.
 * Creates a AWT reply in the format AWT n Tn, where n is the number of topics
 * 	and Tn is list with the name of each topic.
 * Reply is allocated here, must be freed on the server
 */
unsigned char *AWT_reply();

/*
 * Used to reply to TER request
 * Creates a AWTES reply in the format AWTES IPTES portTES, where IPTES is the IP of the TES server
 *  containing the requested topic and portTES is the respective port
 * It reads the information on the topics file, if not found the reply is "EOF"
 * Reply is allocated here, must be freed on the server
 */
unsigned char *AWTES_reply(const int topic_number);

unsigned char *AQT_reply();

unsigned char *QID_reply();

/*
 * Used to send a ERR reply
 * Creates and returns the reply
 */
unsigned char *ERR_reply();

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
	return server_reply;
}

unsigned char *TER_request(int fd, const char *topic_number, const struct sockaddr_in* addr){
	unsigned char *server_reply = NULL;
	char request[REQUEST_BUFFER_32] = "TER ";
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
	return server_reply;
}

unsigned char *RQT_request(int fd, int sid){
	unsigned char *server_reply = NULL;
	char request[REQUEST_BUFFER_32] = "RQT ";
	char char_sid[6];

	sprintf(char_sid, "%d", sid);
	strcat(request, char_sid);
	strcat(request, "\n");

	send_tcp_request(fd, request);

	server_reply = receive_tcp_reply(fd);

	printf("THE REPLY IS: %s\n", server_reply);

	return server_reply;
}

/* ----- REPLIES -------- */


unsigned char* AWT_reply(){
	int ntopic, i;
	char str_ntopic[2];
	char **file_content = NULL, **parsed_line = NULL, *raw_content = NULL;
	unsigned char *server_reply = (unsigned char*)malloc(REPLY_BUFFER_1024 * sizeof(unsigned char));
	bzero(server_reply, REPLY_BUFFER_1024);

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

unsigned char *AWTES_reply(const int topic_number){
	unsigned char *server_reply = (unsigned char*)malloc(REPLY_BUFFER_128* sizeof(unsigned char));
	char *file_content;

	bzero(server_reply, REPLY_BUFFER_128);

	strncpy((char *)server_reply, "AWTES ", 6);

	/* TODO verify valid topic */
	
	file_content = findTopic(topic_number);

	strcat((char *)server_reply, file_content);
	strcat((char *)server_reply, "\n");

	free(file_content);
	return server_reply;
}

unsigned char *AQT_reply(int qid){
	char qid_char[5];
	unsigned char *server_reply = (unsigned char *)malloc(REPLY_BUFFER_OVER_9000 * sizeof(unsigned char));
	bzero(server_reply, REPLY_BUFFER_OVER_9000);

	strncpy((char* )server_reply, "AQT ", 4);
	sprintf(qid_char, "%d", qid);
	strcat((char *)server_reply, qid_char);

	/* TODO rest of reply */
	
	strcat((char *)server_reply, "\n");
	return server_reply;
}


unsigned char *ERR_reply(){
	unsigned char *server_reply = (unsigned char*)malloc(5 * sizeof(unsigned char));
	strcpy((char *)server_reply, "ERR\n");
	return server_reply;
}

#endif
