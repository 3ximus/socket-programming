#ifndef UDP_H
#define UDP_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include "Resources.h"

#define BUFFER_SIZE	2048

char *UDPclient(const char *msg, Server ecp);
void UDPserver(int port);

#endif
