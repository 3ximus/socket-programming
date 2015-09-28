#include "resources.h"
#include "comm_protocol.h"

/*
 * Used for user server interaction
 * Features a shell with the commands: list , request, submit, help, exit
 */
int main(int argc, char *argv[])
{
	const struct server *ecp_server = optParser(argc, argv);
	int sid = atoi(argv[1]), udp_socket;
	size_t line_size = 0;
	char* cmd = NULL;
	char **parsed_cmd;
	unsigned char *server_reply = NULL;
	struct sockaddr_in udp_addr;

	printf("SID: %d\nECPname: %s\nECPport: %d\n",sid, ecp_server->name, ecp_server->port);

	udp_socket = start_udp_client(&udp_addr, ecp_server);

	while(1){
		printf("> ");
		

		/* TODO Still causes segmentation fault */
		if ((getline(&cmd, &line_size, stdin)) == -1){
			perror("[ERROR] no command\n");
			continue;
		}
		
		parsed_cmd = parseString(cmd, "\n");
		parsed_cmd = parseString(parsed_cmd[0], " ");

		if(strcmp(parsed_cmd[0],"list") == 0 && parsed_cmd[1] == NULL)
		{	
			char **topics = NULL;
			int i, ntopic;
			/* Send TQR request */

		 
			server_reply = TQR_request(udp_socket, &udp_addr);
			topics = parseString((char *)server_reply," ");
			ntopic = atoi(topics[1]);

			/* Print topics */
			for (i = 2; i < ntopic + 2; i++){
				/* add padding for protocol */
				printf("%d - %s\n", i - 1, topics[i]);
			}
			free(topics);
		}

		else if (strcmp(parsed_cmd[0], "request") == 0 && parsed_cmd[2] == NULL){
			char **parsed_reply = NULL;
			char *ip_addr;
			int port, tcp_socket;

			if (parsed_cmd[1] == NULL){
				/* Handle error */
				printf("[ERROR] request with no topic\n");
				free(parsed_cmd);
				continue;
			}
			/* receives tes server (containing requested topic) information */
			server_reply = TER_request(udp_socket, parsed_cmd[1], &udp_addr);

			parsed_reply = parseString((char *)server_reply, "\n");
			parsed_reply = parseString((char *)server_reply, " ");

			ip_addr = parsed_reply[1];
			port = atoi(parsed_reply[2]);

			if ((tcp_socket = start_tcp_client(ip_addr, port)) == -1){
				/* if the tes server isn't online */
				perror("There is no TES server on that port.\n");
				continue;
			}

			printf("TES server IP: %s\n", ip_addr);
			printf("TES server Port: %d\n", port);

			/* send RQT request to TES server */
			server_reply = RQT_request(tcp_socket, sid);

			printf("Server reply: %s\n", server_reply);
			/* save the pdf document */

			free(parsed_reply);

		}

		else if (strcmp(parsed_cmd[0],"submit") == 0){
			int i;
			char *sequence = (char *)malloc(sizeof(char) * 5); /* Array of 5 char for sequence */

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

		else if (strcmp(parsed_cmd[0], "help") == 0 && parsed_cmd[1] == NULL){
			printf("Questionnarie End User Application.\nReceives commands to interact with online questionnaires\n\n");
			printf("Commands:\n\tlist\t\t\t- List topics available.\n\trequest [topic-number]\t- Request for certain topic.\n\tsubmit [answer-list]\t- Submit answer in the format \"A B C D \".\n\thelp\t\t\t- Show this help screen.\n\texit\t\t\t- Exit application.\n\n");
		}
		else if (strcmp(parsed_cmd[0],"exit") == 0 && parsed_cmd[1] == NULL)
		{
			printf("Exiting...\n");
			free(cmd);
			exit(1);
		}
		else{
			printf("Wrong command \"%s\", or wrong command format.\n", parsed_cmd[0]);
			free(parsed_cmd);
			/* use continue so it doesn't try to free server_reply 
			that has been freed on last cycle, caused double free */
			continue;
		}

		/* free at every cicle */
		free(server_reply);
		free(parsed_cmd);
	}
	return 0;
}

