#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "Resources.h"
#include "UDP.h"

int main(int argc, char *argv[])
{
	Server ecp = getUserInputInfo(argc,argv);
	long int sid = atoi(argv[1]);

	char cmd[50];
	printf("SID: %ld\nECPname: %s\nECPport: %d\n",sid,ecp.name,ecp.port);

	while(1)
	{
		printf("> ");
		scanf("%s",cmd);

		if(strcmp(cmd,"list") == 0)
		{	
			char *buffer = UDPclient("TQR",ecp);
			char **topics = parseString(buffer," ");
			printTopics(topics);

			/* Limpa a heap */
			free(buffer);
			free(topics);
		}

		else if (strcmp(cmd,"request") == 0)
		{
			char req_msg[] = "TER  ";
			char *buffer = NULL;

			scanf(" %c",&req_msg[4]);
			buffer = UDPclient(req_msg,ecp);
			printf("%s\n",buffer);

			free(buffer);
			/* TODO */
		}

		else if (strcmp(cmd,"submit") == 0)
		{
			printf("submit\n");
			/* TODO: Falta ler seq. de respostas e enviar em TCP */
		}

		else if (strcmp(cmd,"exit") == 0)
		{
			printf("Exiting....\n");
			exit(1);
		}
		else
			printf("No command available.\n");
	}
	
	return 0;
}

