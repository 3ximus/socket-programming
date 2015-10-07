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

/* 
 * Used to request the questionnarie from the TES server
 * Sends the sid of the user
 * Reply must be freed on client
 */
unsigned char *RQT_request(int, int);

/*
 * Used to submit questionnaries answers to the TES server
 * Sends the user sid, the questionnarie qid and the answers
 * Reply must be freed on the client
 */
unsigned char *RQS_request(int, int, char*, char **);

/*
 * Used to contact ECP server with score information
 * Sends the user sid, the questionnarie qid, the topic nr and the score obtained
 * Reply must be freed on the client
 */
unsigned char *IQR_request(int , const struct sockaddr_in* , int , char* , int , int );

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

/*
 * Used to reply to RQT request
 * Sends the pdf of the questionnarie to the user, together with its qid and a
 *  timestamp of the request
 * Reply is allocated here, must be freed on the server
 */
unsigned char *AQT_reply(int, const struct tm* delay, char *qid);

/*
 * Used to reply to a RQS request
 * Sends the questionnarie qid with the the score obtained
 * Reply is allocated here, must be freed on the server
 */
unsigned char *AQS_reply(char*, int);

/*
 * Used to send a ERR reply
 * Creates and returns the reply
 */
unsigned char *ERR_reply();

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
	char request[REQUEST_BUFFER_32];
	char *endptr;

	/* check if topic_number is in fact a number and is between 0 and TOPIC_NR (99) */
	/* This must be implemented on the server */
	long val = strtol(topic_number, &endptr, 10);
	if (endptr == topic_number || val < 1 || val > TOPIC_NR)
		printf("[ERROR] Request with no valid number\n");

	/* build request */
	sprintf(request, "TER %s\n", topic_number);

	/* contact server with built request */
	send_udp_request(fd, (unsigned char *)request, addr);
	server_reply = receive_udp_reply(fd, addr);
	return server_reply;
}

unsigned char *RQT_request(int fd, int sid){
	unsigned char *server_reply = NULL;
	char request[REQUEST_BUFFER_32];

	/* build request */
	sprintf(request, "RQT %d\n", sid);

	/* send request */
	send_tcp_request(fd, (unsigned char*)request);
	/* wait for reply */
	server_reply = receive_tcp_reply(fd, REPLY_BUFFER_OVER_9000);

	return server_reply;
}

unsigned char *RQS_request(int fd, int sid, char* qid, char **parsed_cmd){
	int n;
	unsigned char *server_reply = NULL;
	char request[REQUEST_BUFFER_32], sequence[10];
	memset(sequence, '\0', sizeof(sequence));

	/* build request */
	sprintf(request, "RQS %d %s ", sid,qid);

	/* upper case */
	for (n = 1; n < CMD_SIZE ;n++){
		if (parsed_cmd[n][0] > 'D')
			parsed_cmd[n][0] -= ('a' - 'A');
		strcat(sequence, parsed_cmd[n]);
		strcat(sequence, " ");
	}

	strcat(request, sequence);
	request[strlen((char *)request) - 1] = '\n';
	
	/* send request and wait for reply */
	send_tcp_request(fd, (unsigned char*)request);
	server_reply = receive_tcp_reply(fd, REPLY_BUFFER_128);

	return server_reply;
}

unsigned char *IQR_request(int fd, const struct sockaddr_in* addr, int sid, char* qid, int topic, int score){
	unsigned char *server_reply = NULL;
	char request[REQUEST_BUFFER_64];

	/* build request */
	sprintf(request,"IQR %d %s %d %d\n", sid,qid,topic,score);

	/* contact server with built request */
	send_udp_request(fd, (unsigned char *)request, addr);
	server_reply = receive_udp_reply(fd, addr);
	return server_reply;
}


/* ----- REPLIES -------- */


unsigned char* AWT_reply(){
	int ntopic, i;
	char str_ntopic[2];
	char **file_content = NULL, **parsed_line = NULL, *raw_content = NULL;
	unsigned char *server_reply = (unsigned char*)malloc(REPLY_BUFFER_1024 * sizeof(unsigned char));

	memset((void *)server_reply,'\0', REPLY_BUFFER_1024);

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

	return server_reply;
}

unsigned char *AWTES_reply(const int topic_number){
	char *file_content;
	unsigned char *server_reply = (unsigned char*)malloc(REPLY_BUFFER_128* sizeof(unsigned char));
	memset((void *)server_reply,'\0', REPLY_BUFFER_128);

	/* TODO verify valid topic */
	file_content = findTopic(topic_number);

	/* build reply */
	sprintf((char *) server_reply, "AWTES %s\n",file_content);

	free(file_content);
	return server_reply;
}

unsigned char *AQT_reply(int sid, const struct tm* expiration, char *qid){
	char sid_char[5], timestamp[30], timestamp_now[30], size_char[6];
	time_t now;
	int fd;
	/*size_t len = 0;*/
	ssize_t bytes_read, quest_size = 0;
	char read_buffer[REPLY_BUFFER_OVER_9000];
	struct tm *time_struct;
	unsigned char *server_reply = (unsigned char *)malloc(REPLY_BUFFER_OVER_9000 * sizeof(unsigned char)),
				  *server_reply_ptr = NULL,
				  placeholder[REPLY_BUFFER_OVER_9000],
				  *placeholder_ptr = NULL;
	/* zero buffer */
	memset((void *)server_reply,'\0', REPLY_BUFFER_OVER_9000);
	memset((void *)timestamp,'\0', 30);

	/* build reply */
	strncpy((char* )server_reply, "AQT ", 4);

	/* set time struct to be this exat moment */
	time(&now);
	time_struct = localtime((const time_t *)&now);

	/* convert current time to string format */
	strftime(timestamp_now, 30, "%d%b%Y_%H:%M:%S", time_struct);

	sprintf(sid_char, "%d", sid);
	/* qid */
	strcat((char *)server_reply, sid_char);
	strcat((char *)server_reply, "_");
	strcat((char *)server_reply, timestamp_now);
	strcat((char *)server_reply, " ");

	/* export qid */
	strcat(qid, sid_char);
	strcat(qid, "_");
	strcat(qid, timestamp_now);

	/* add expiration starting on this exact moment , doesnt fully support many days*/
	time_struct->tm_sec += expiration->tm_sec;
	time_struct->tm_min += (time_struct->tm_sec / 59) + expiration->tm_min;
	time_struct->tm_hour += (time_struct->tm_min / 59) + expiration->tm_hour;
	time_struct->tm_mday += (time_struct->tm_hour / 23) + expiration->tm_mday;
	/* cap values */
	time_struct->tm_sec = time_struct->tm_sec % 60;
	time_struct->tm_min = time_struct->tm_min % 60;
	time_struct->tm_hour = time_struct->tm_hour % 24;

	/* convert current time to string format and add it to the server reply*/
	strftime(timestamp, 30, "%d%b%Y_%H:%M:%S", time_struct);
	strcat((char *)server_reply, timestamp);
	strcat((char *)server_reply, " ");

	/* read file */
	if ((fd = open("2015_2016_Proj_SocketProg.pdf", O_RDONLY, S_IRUSR|S_IWUSR)) == -1){
		/* handle error */
		perror("[ERROR] Opening .pdf file\n");
		exit(-1);
	}
	placeholder_ptr = placeholder;
	while ((bytes_read = read(fd, read_buffer, REPLY_BUFFER_OVER_9000)) > 0){
		memcpy(placeholder_ptr, read_buffer, bytes_read);
		placeholder_ptr += bytes_read; /* move pointer */
		quest_size += bytes_read; /* increment size counter */
	}

	/* concatenate size */
	sprintf(size_char, "%d", (int)quest_size);
	strcat((char *)server_reply, size_char);
	strcat((char *)server_reply, " ");

	/* pdf content */
	server_reply_ptr = server_reply + strlen((char *)server_reply); /* pointer to the end of the server reply string */
	memcpy(server_reply_ptr, placeholder, quest_size);

	/* finish reply */
	strcat((char * )server_reply, "\n");
	close(fd);
	return server_reply;
}

unsigned char *AQS_reply(char* qid, int score){
	unsigned char *server_reply = (unsigned char *)malloc(REPLY_BUFFER_128 * sizeof(unsigned char));
	memset((void *)server_reply,'\0', REPLY_BUFFER_128);

	/* build reply */
	sprintf((char * )server_reply, "AQS %s %d\n",qid,score);
	
	return server_reply;
}

unsigned char *ERR_reply(){
	unsigned char *server_reply = (unsigned char*)malloc(5 * sizeof(unsigned char));
	strcpy((char *)server_reply, "ERR\n");
	return server_reply;
}

#endif
