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
	char **parsed_cmd = (char **)malloc(6 * sizeof(char *));
	/* server */
	unsigned char *server_reply = NULL;
	struct sockaddr_in udp_addr;
	struct tes_server tes_info;
	/* generic */
	int sid = atoi(argv[1]), udp_socket, tcp_socket;
	size_t line_size = 0;

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
		parse_string(parsed_cmd, cmd, " \n", 6);

		if(strcmp(parsed_cmd[0],"list") == 0 && parsed_cmd[1] == NULL)
		{	
			char **topics = NULL;
			int i, ntopic, err;

			/* Send TQR request */
			server_reply = TQR_request(udp_socket, &udp_addr);
			/* check for errors */
			if ((err = check_for_errors((char *)server_reply, "AWT")) == -1){
				printf("[ERROR] Didn't receive correct reply\n");
				free(server_reply);
				continue;
			}
			else if (err == 1){
				printf("[ERROR] EOF\n");
				free(server_reply);
				continue;
			}
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
			/* this is to store AQT reply therefore 4 spaces are needed before data segment */
			char **parsed_reply_2 = (char **)malloc(4 * sizeof(char *));
			unsigned char *server_reply_ptr;
			char filename[10];
			/*char parsed_time[30];*/
			int quest_size, pdf_fd, written_bytes, i, offset = 0;

			if (parsed_cmd[1] == NULL){
				/* Handle error */
				printf("[ERROR] request with no topic\n");
				continue;
			}
			/* receives tes server (containing requested topic) information */
			server_reply = TER_request(udp_socket, parsed_cmd[1], &udp_addr);
			/* check for errors */
			if (check_for_errors((char *)server_reply, "AWTES") == -1){
				printf("[ERROR] Didn't receive correct reply\n");
				free(parsed_reply);
				free(parsed_reply_2);
				free(server_reply);
				continue;
			}
			
			server_reply[strlen((char *)server_reply) - 1] = '\0'; /* remove '\n' */
			parsed_reply = parseString((char *)server_reply, " \n");
			strcpy(tes_info.ip_addr, parsed_reply[1]); /* IP */
			tes_info.port = atoi(parsed_reply[2]); /* PORT */

			if ((tcp_socket = start_tcp_client(tes_info.ip_addr, tes_info.port)) == -1){
				/* if the tes server isn't online */
				perror("There is no TES server on that port.\n");
				free(server_reply);
				continue;
			}

			printf("TES server IP: [%s], Port: [%d]\n", tes_info.ip_addr, tes_info.port);

			/* send RQT request to TES server */
			server_reply = RQT_request(tcp_socket, sid);

			/* check for errors */
			if (check_for_errors((char *)server_reply, "AQT") == -1){
				printf("[ERROR] Didn't receive correct reply\n");
				free(parsed_reply);
				free(parsed_reply_2);
				free(server_reply);
				continue;
			}
			server_reply_ptr = server_reply;
			parse_string(parsed_reply_2, (char *)server_reply, " ", 4); /* again size is 4 due to reply format */

			strcpy(tes_info.qid, parsed_reply_2[1]); /* QID */
			strncpy(tes_info.time_limit, parsed_reply_2[2], 30); /* TIME */
			quest_size = atoi(parsed_reply_2[3]); /* SIZE */

			/* TODO make time string prettier */
			printf("This QID is: %s, and you have until %s to submit it.\n", tes_info.qid, tes_info.time_limit);

			/* build filename */
			strcpy(filename, tes_info.qid);
			strcat(filename, ".pdf");
			pdf_fd = open(filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
			if (pdf_fd == -1) {
				/* handle error */
				perror("[ERROR] Opening topics.txt file\n");
				free(parsed_reply);
				free(parsed_reply_2);
				free(server_reply);
				exit(-1);
			}
			/* calculate offset */
			for(i = 0; i < 4; i++) offset += strlen(parsed_reply_2[i]) + 1;
			/* write pdf */
			written_bytes = write(pdf_fd, server_reply_ptr + offset, quest_size);
			fsync(pdf_fd);

			printf("File Downloaded: \"%s\", file size: %d\n", filename, written_bytes);

			free(parsed_reply);
			free(parsed_reply_2);
			close(tcp_socket);
		}

		else if (strcmp(parsed_cmd[0],"submit") == 0){
			char **parsed_reply = (char **)malloc(4 * sizeof(char *));
			/* test if we have made a request before */
			if (tes_info.qid == NULL || tes_info.ip_addr == NULL || tes_info.port == 0){
				printf("[ERROR] No request was made before.\n");
				continue;
			}
			if(checkSubmitAnswer(parsed_cmd) == -1) {
				printf("[ERROR] Bad submit answer.\n");
				free(parsed_reply);
				continue;
			}
			if ((tcp_socket = start_tcp_client(tes_info.ip_addr, tes_info.port)) == -1){
				/*if the tes server isn't online */
				perror("[ERROR] There is no TES server on that port.\n");
				free(parsed_reply);
				continue;
			}
			server_reply = RQS_request(tcp_socket, sid, tes_info.qid, parsed_cmd);
			if (check_for_errors((char *)server_reply, "AQS") == -1){
				printf("Wrong answer\n");
				free(parsed_reply);
				free(server_reply);
				continue;
			}
			parse_string(parsed_reply, (char *)server_reply, " ", 4);

			printf("Score of QID: %s is %s", parsed_reply[1], parsed_reply[2]);
			free(parsed_reply);
			close(tcp_socket);
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
			/* use continue so it doesn't try to free server_reply 
			that has been freed on last cycle, caused double free */
			continue;
		}

		/* free at every cicle */
		free(server_reply);
	}
	free(parsed_cmd);
	return 0;
}

