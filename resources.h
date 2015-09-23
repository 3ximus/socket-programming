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
#define DEFAULT_PORT 58058

/* Buffer sizes */
#define BUFFER_SIZE	2048
#define LOG_BUFFER_SIZE 256

#define BIG_REPLY_BUFFER 1024
#define SMALL_REPLY_BUFFER 128
#define REQUEST_BUFFER_SIZE 32

/* Maximum number of topics */
#define TOPIC_NR	 99

/* Default server log file */
#define SERVER_LOG "./server.log"
/* Topics file */
#define TOPICS_FILE "./topics.txt"

/* -------------------------------- */

/* Server structure, contains server information */
struct server{
	unsigned char name[50];
	unsigned int port;
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
 * Read a line from file
 * Returns file content
 * Number of lines read are placed on line_number
 * Note: content needs to be freed
 */
char **readFromFile(const char *, int *);

/* 
Returns a table with separated strings
input: char* msg = "Ole|Ola", char* delim = "|"

output: char table[0] = Ole
		char table[1] = Ola
*/
char ** parseString(char*, const char*);

/*
 * Creates a Table from fixed intput
 */
char **createTable(int, int);

/*
 * Free table memory
 */
void freeTable(char **, int);

/* 
 * Dumps raw memory in hex byte and printable format
 */
void dump(const unsigned char *, const unsigned int);

#endif
