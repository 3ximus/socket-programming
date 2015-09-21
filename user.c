#include "resources.h"
#include "udp_client.h"

void printHelpScreen();

int main(int argc, char *argv[])
{
	const struct server *ecp = optParser(argc, argv);

	long int sid = atoi(argv[1]);

	char cmd[50];
	char **topics = NULL;
	unsigned char *server_response; /* may be cause for memory leak */
	char request_buffer[10];
	char **parsed_cmd;
	bzero(cmd, 50);

	printf("SID: %ld\nECPname: %s\nECPport: %d\n",sid, ecp->name, ecp->port);

	while(1)
	{
		printf("> ");
		if (fgets(cmd, 50, stdin) == NULL){
			printf("[ERROR] unexistent command\n");
			continue;
		}
		parsed_cmd = parseString(cmd, "\n");
		parsed_cmd = parseString(parsed_cmd[0], " ");

		if(strcmp(parsed_cmd[0],"list") == 0)
		{	
			/* Send TQR request */
			server_response = UDPclient("TQR",ecp);
			topics = parseString((char *)server_response," ");
			printTopics(topics);
			bzero(server_response, strlen((char*)server_response)); /* clear server response */
		}

		else if (strcmp(parsed_cmd[0], "request") == 0){
			strcpy(request_buffer, "TER ");			

			if (parsed_cmd[1] == NULL){
				/* Handle error */
				printf("[ERROR] request with no topic\n");
				continue;
			}
			
			strncat(request_buffer, parsed_cmd[1], 6);
			server_response = UDPclient(request_buffer, ecp);
		}

		else if (strcmp(parsed_cmd[0],"submit") == 0){
			int i;
			char *sequence = (char *)malloc(sizeof(char) * 5); /* Array of 5 char for sequence */
			bzero(sequence, 5);

			/* Only reads 5 char separated with " ", ignore the rest */
			for (i = 1; i < 6; i++)
				if (parsed_cmd[i] == NULL){
					/* Handle error */
					printf("[ERROR] submit with nonexistent or incomplete sequence\n");
					break;
					/* this still executes the possible send string*/
				}
				else{
					if (strlen(parsed_cmd[i]) > 1 ){
						/* Handle error */
						printf("[ERROR] bad sequence given\n");
						break;
						/* this still executes the possible send string*/
					}
					else{
						strcat(sequence, parsed_cmd[i]);
					}
				}
			/* TODO: Send string sequence */
			free(sequence);
		}

		else if (strcmp(parsed_cmd[0], "help") == 0)
			printHelpScreen();

		else if (strcmp(parsed_cmd[0],"exit") == 0)
		{
			printf("Exiting...\n");

			/* free memory */
			free(parsed_cmd);
			free(topics);
			exit(1);
		}
		else
			printf("Wrong command \"%s\".\n", parsed_cmd[0]);
	}
	return 0;
}

/*
 * Prints help Screen
 */
void printHelpScreen(){
	printf("Questionnarie End User Application.\nReceives commands to interact with online questionnaires\n\n");
	printf("Commands:\n\tlist\t\t\t- List topics available.\n\trequest [topic-number]\t- Request for certain topic.\n\tsubmit [answer-list]\t- Submit answer in the format \"A B C D \".\n\thelp\t\t\t- Show this help screen.\n\texit\t\t\t- Exit application.\n\n");
}
