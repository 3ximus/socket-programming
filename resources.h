#ifndef RESOURCES_H
#define RESOURCES_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 58058
#define BUFFER_SIZE	2048
#define LOG_BUFFER_SIZE 128
#define TOPIC_NR	 99
#define SERVER_LOG "./server.log"

struct server{
	char name[50];
	int port;
};

struct server* optParser(int, char**);
int getHostIP(const char *);
void printHostInfo(struct sockaddr_in);
void log_action(char*, char*, int);
const char* readFromFile(const char *);
char ** parseString(char*, const char*);
void printTopics(char**);
char **createTable(int, int);
void freeTable(char **, int);

#endif
