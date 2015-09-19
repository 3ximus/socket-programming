#ifndef RESOURCES_H
#define RESOURCES_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 58058
#define TOPIC_NR	 99

typedef struct server{
	char name[50];
	int port;
}Server;

Server getUserInputInfo(int argc, char *argv[]);
int getHostIP(const char *host_name);
void getHostAdress(struct sockaddr_in addr);
const char* readFromFile(const char *file_name);
char ** parseString(char* msg , const char* delim);
void printTopics(char **topics);
/*void freeTable(char **table, int rows);
char **createTable(int rows, int row_length);*/

#endif
