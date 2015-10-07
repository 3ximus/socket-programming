#ifndef RESOURCES_H
#define RESOURCES_H

/*
 * This contains a list of common use functions throughout the project
 */

/* Common */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

/* System */
#include <unistd.h>
#include <signal.h>

/* Net */
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Default port if none is specified */
#define DEFAULT_PORT_ECP 58057
#define DEFAULT_PORT_TES 59000

/* Buffer sizes */

#define BUFFER_2048	2048
#define BUFFER_32 32
#define LOG_BUFFER_SIZE 256

#define REPLY_BUFFER_OVER_9000 2000000
#define REPLY_BUFFER_1024 1024
#define REPLY_BUFFER_128 128
#define REPLY_BUFFER_32 32
#define REQUEST_BUFFER_32 32
#define REQUEST_BUFFER_64 64
#define CMD_SIZE 6

/* Maximum number of topics */
#define TOPIC_NR 99
#define ANSW_NR	5
#define TRUE  1
#define FALSE 0

/* Default server log file */
#define UDP_SERVER_LOG "./ecp_server.log"
#define TCP_SERVER_LOG "./tes_server.log"
/* Topics file */
#define TOPICS_FILE "./topics.txt"
#define TES_CONFIG_FILE "./tes_server.config"

/* -------------------------------- */

/* Server structure, contains server information */
struct server{
	unsigned char name[50];
	unsigned int port;
};

/* tes server structure contains connection info */
struct tes_server{
	char qid[BUFFER_32];
	char ip_addr[16];
	int port;
	char time_limit[BUFFER_32];
};

/* tes server user-quest table */
struct user_table{
	int sid;
	time_t deadline;
	int score;
	char qid[BUFFER_32];
	int internal_qid;
};

/* -------------------------------- */

/*
 * Passes options passed to exec
 */
struct server* optParser(int, char**);

/* 
 * Returns ip adress associated with hostname:
 */
int getHostIP(const char *);

/* 
 * Return host information of ip adress
 * - hostname
 * - IP
 * - Port
 */
void printHostInfo(struct sockaddr_in);

/*
 * Log action on server
 * Types are unix standard 64 present in /usr/include/x86_64-linux-gnu/asm/unistd_64.h
 * for the first 4 types of log actions
 * 0 - read
 * 1 - write
 * 2 - open
 * 3 - close
 * From here extra ones were added
 * 4 - log
 * 5 - warning
 * 6 - errror
 */
void log_action(char*, char*, int);


/*
 * Returns the entire file content
 * Content must be freed
 */
char *readFromFile(const char *);

/*
 * Finds a topic number in the topics.txt file and returns the IP and PORT of the respective TES Server
 * If not found returns EOF
 */
char *findTopic(const int);

/*
 * Load tes server consiguration file
 */
int readServerConfig();

/*
 * Write to tes server config file
 */
void writeServerConfig(int);

/* 
 * Returns a table with separated strings
 */
char ** parseString(char*, const char*);

/*
 * Receives a recepient for the parsed string, the string to be parsed, the delimiter chars and the
 *  amount of parsed elements
 * Returns the ammount of parsed strings (vector size) or -1 if an error ocurred or invalid 
 *  arguments where passed
 */
int parse_string(char **, char*, const char*, int);

/*
 * Checks if answer is between A and D (not case sensitive)
 */
int checkSubmitAnswer(char **answ);

/*
 * Check a Reply for errors ( Handle ERR and EOF ) agains an expected value
 * Returns 0 on sucess
 * Returns 1 if EOF
 * Returns -1 if anything else is passed or ERR
 */
int check_for_errors(const char*, char*);

#endif
