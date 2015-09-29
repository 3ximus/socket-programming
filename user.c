#include "resources.h"
#include "comm_protocol.h"

/*
 * Used for user server interaction
 * Features a shell with the commands: list , request, submit, help, exit
 */
int main(int argc, char *argv[]){
	/* read arguments passed */
	const struct server *ecp_server = optParser(argc, argv);
	/* shell cmds */
	char* cmd = NULL;
	char **parsed_cmd;
	/* server */
	unsigned char *server_reply = NULL;
	struct sockaddr_in udp_addr;
	struct tes_server tes_info;
	/* generic */
	int sid = atoi(argv[1]), udp_socket, tcp_socket;
	size_t line_size = 0;

	/* inititate tes_server structure */
	tes_info.qid = 0;
	tes_info.port = 0;

	printf("SID: %d\nECPname: %s\nECPport: %d\n",sid, ecp_server->name, ecp_server->port);

	/* initiate a UDP client */
	udp_socket = start_udp_client(&udp_addr, ecp_server);

	while(1){
		printf("> ");

		/* TODO Still causes segmentation fault */
		if (getline(&cmd, &line_size, stdin) == -1){
			perror("[ERROR] no command\n");
			continue;
		}
		

		/* remove '\n' */
		cmd[strlen((char *)cmd) - 1] = '\0';
		parsed_cmd = parseString(cmd, " ");

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
			char **parsed_reply_2 = NULL;

			if (parsed_cmd[1] == NULL){
				/* Handle error */
				printf("[ERROR] request with no topic\n");
				free(parsed_cmd);
				continue;
			}
			/* receives tes server (containing requested topic) information */
			server_reply = TER_request(udp_socket, parsed_cmd[1], &udp_addr);

			/* remove '\n' */
			server_reply[strlen((char *)server_reply) - 1] = '\0';
			parsed_reply = parseString((char *)server_reply, " ");

			strcpy(tes_info.ip_addr, parsed_reply[1]);
			tes_info.port = atoi(parsed_reply[2]);

			if ((tcp_socket = start_tcp_client(tes_info.ip_addr, tes_info.port)) == -1){
				/* if the tes server isn't online */
				perror("There is no TES server on that port.\n");
				continue;
			}

			printf("TES server IP: [%s], Port: [%d]\n", tes_info.ip_addr, tes_info.port);

			/* send RQT request to TES server */
			server_reply = RQT_request(tcp_socket, sid);
			parsed_reply_2 = parseString((char *)server_reply, " ");

			tes_info.qid = atoi(parsed_reply_2[1]);
			strncpy(tes_info.time_limit, parsed_reply_2[2], 30);

			printf("This QID is: %d, and you have until %s to submit it.\n", tes_info.qid, tes_info.time_limit);

			/* TODO save the pdf document */

			free(parsed_reply);
			free(parsed_reply_2);
		}

		else if (strcmp(parsed_cmd[0],"submit") == 0){
			int i;
			char *sequence = (char *)malloc(sizeof(char) * 10); /* Array of 5 char for sequence */ 

			/* Only reads 5 char separated with " ", ignore the rest */
			for (i = 1; i < 6; i++)
				if (parsed_cmd[i] == NULL){
					/* Handle error */
					printf("[ERROR] Submit with nonexistent or incomplete sequence\n");
					break;
					/* this still executes the possible send string*/
				}
				else{
					if (strlen(parsed_cmd[i]) > 1 ){
						/* Handle error */
						printf("[ERROR] Bad sequence given\n");
						break;
						/* this still executes the possible send string*/
					}
					else{
						strcat(sequence, parsed_cmd[i]);
						strcat(sequence, " ");
					}
				}

			/* test if we have made a request before */
			if (tes_info.qid == 0 || tes_info.ip_addr == NULL || tes_info.port == 0)
			{
				printf("[ERROR] No request was made before.\n");
				free(sequence);
				continue;
			}
			
			if ((tcp_socket = start_tcp_client(tes_info.ip_addr, tes_info.port)) == -1){
				/*if the tes server isn't online */
				perror("[ERROR] There is no TES server on that port.\n");
				continue;
			}
			server_reply = RQS_request(tcp_socket, sid, tes_info.qid, sequence);
			
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
			printf("[ERROR] Wrong command \"%s\", or wrong command format.\n", parsed_cmd[0]);
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

