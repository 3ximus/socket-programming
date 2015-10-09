#include "resources.h"

/*
 * Passes options passed to exec
 */
void optParser(int argc, char *argv[], int *sid, struct ecp_server* ecp){
	int arg ;
	char inputErr[] = "\n\nInput format: ./user SID [-n ECPname] [-p ECPport]\n\nSID: student identity number.\nECPname: name of ECP server (opt).\nECPport: well-known port of ECP server (opt).\n\n";

	if (argc < 2){
		printf("[ERROR]: Not enough arguments.%s",inputErr);
		exit(1);
	}
	if ((*sid = atoi(argv[1])) == 0){
		printf("[ERROR Insert a valid SID");
		exit(1);
	}
	ecp->port = DEFAULT_PORT_ECP;
	if (gethostname((char*)ecp->name, sizeof(ecp->name)) == -1) {
		perror("[ERROR] Getting hostname.");
		exit(1);
	}
	for (arg = 2; arg < argc; arg++){
		if (strcmp(argv[arg], "-n") == 0 && arg != argc - 1){
			strncpy((char*)ecp->name, argv[arg +1], sizeof(ecp->name));
			arg++;
		}
		else if (strcmp(argv[arg], "-p") == 0 && arg != argc - 1){
			if ((ecp->port = atoi(argv[arg +1])) == 0){
				printf("[ERROR Insert a valid port");
				exit(1);	
			}
			arg++;
		}
		else {
			printf("[ERROR] Unknown argument %s\n", argv[arg]);
			exit(1);
		}
	}
}

/* 
 * Returns ip adress associated with hostname:
 */
int getHostIP(const char *host_name){
	struct hostent *h;
	struct in_addr *a;

	if((h = gethostbyname(host_name)) == NULL){
		printf("ERROR: gethostbyname()\n");
		exit(0);
	}

	a = (struct in_addr *) h->h_addr_list[0];

	/*printf("IP adress: %s (%X)\n",inet_ntoa(*a),ntohl(a->s_addr));*/

	return a->s_addr;
}

/* 
 * Return host information of ip adress
 * - hostname
 * - IP
 * - Port
 */
void printHostInfo(struct sockaddr_in addr){
	struct hostent* h;

	h = gethostbyaddr((char *)&addr.sin_addr, sizeof(struct in_addr),AF_INET);

	if(h == NULL)
		printf("Sent by %s:%hu\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
	else
		printf("\nSent by, Server: %s\n\t IP  : %s\n\t Port: %hu\n\n",h->h_name,inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
}

/*
 * Returns the entire file content
 * Content must be freed
 */
char *readFromFile(const char *file_name){
	int fd, bytes_read = 100;
	char read_buffer[BUFFER_1024];
	char *accumulator_buffer = (char *) malloc(BUFFER_2048 * sizeof(char ));

	fd = open(file_name, O_RDONLY, S_IRUSR|S_IWUSR);
	if (fd == -1){
		/* handle error */
		perror("[ERROR] Opening topics.txt file");
		exit(-1);
	}
	/* while EOF isnt reached */
	while((bytes_read = read(fd, read_buffer, BUFFER_1024)) > 0){
		/* build file content on the accumulator */
		strcat(accumulator_buffer, read_buffer);
	}
	close(fd);
	return accumulator_buffer; 	/* return raw file data */
}

/*
 * Finds a topic number in the topics.txt file and returns the IP and PORT of the respective TES Server
 * If not found returns EOF
 */
char *findTopic(const int search_me){
	size_t len = 0;
	ssize_t bytes_read;
	int i = 1;
	FILE *fd;
	char *read_buffer = NULL,
		 **parsed_line = (char **)malloc(TOPIC_NR * sizeof(char *)),
		 *content = (char *)malloc(BUFFER_32 * sizeof(char));
	memset((void *)content,'\0', BUFFER_32);
	
	if ((fd = fopen(TOPICS_FILE, "r")) == NULL){
		perror("[ERROR] Couldn't find topics file");
		exit(1);
	}
	while (i <= search_me){
		if ((bytes_read = getline(&read_buffer, &len, fd)) == -1){
			free(parsed_line);
			free(read_buffer);
			fclose(fd);
			return NULL;
		}
		i++;
	}
	if (3 != parse_string(parsed_line, read_buffer, " \n", 3))
		return NULL;
	
	sprintf(content, "%s %s", parsed_line[1], parsed_line[2]); /* add IP and PORT */
	if (content[strlen(content) - 1] == '\n')
		content[strlen(content) -1] = '\0';

	free(parsed_line);
	free(read_buffer);
	fclose(fd);

	return content;
}

int readServerConfig(){
	FILE *fd;
	int qid;
	fd = fopen(TES_CONFIG_FILE, "r");

	fscanf(fd, "qid = %d", &qid);
	return qid;
}

void writeServerConfig(int qid){
	FILE *fd;
	fd = fopen(TES_CONFIG_FILE, "w");
	fprintf(fd, "qid = %d", qid);
}

/* 
 * Devolve um tabela de strings em que cada entrada foi separada por simbolo de separacao.
 */
char **parseString(char* msg , const char* delim){
	int i = 0; 

	char **tokens = (char **) malloc(TOPIC_NR * sizeof(char *));

	tokens[i] = strtok(msg,delim);

	while(tokens[i] != NULL){
		i++;
		tokens[i] = strtok(NULL,delim);
	}

	return tokens;
}

/*
 * Receives a recepient for the parsed string, the string to be parsed, the delimiter chars and the
 *  amount of parsed elements
 * Returns the ammount of parsed strings (vector size) or -1 if an error ocurred or invalid 
 *  arguments where passed
 */
int parse_string(char **parsed, char *string, const char *delim, int amount){
	int token_n = 0;
	char *token;
	int i;
	/* error check */
	if (parsed == NULL || string == NULL || delim == NULL || amount == 0)
		return -1;
	/* first token */
	token = strtok(string, delim);
	/* keep parsing */
	while (token_n < amount && token != NULL){
		parsed[token_n] = token;
		token_n++;

		token = strtok(NULL, delim);
	}
	/* set the rest to NULL */
	for (i = token_n; i < amount; ++i)
		parsed[i] = NULL;
	return token_n;
}

/*
 * Checks if answer is between A and D (not case sensitive)
 */
int checkSubmitAnswer(char **answ){
	int n;

	for (n=1; n<CMD_SIZE;n++){
		if (answ[n] == NULL)
			return -1;
		if (strlen(answ[n]) != 1)
			return -1;
		if((answ[n][0] < 'A' || answ[n][0] > 'D') && (answ[n][0] < 'a' || answ[n][0] > 'd'))
			return -1;
	}
	return 0;
}

/*
 * Check a Reply for errors ( Handle ERR and EOF ) agains an expected value
 * Returns 0 on sucess
 * Returns 1 if EOF
 * Returns -1 if anything else is passed or ERR
 */
int check_for_errors(const char* original, char* expected){
	int n;
	char parsed[7];
	memset((void*)parsed, '\0', 7);
	for (n = 0; n < 7; n++)
		if (original[n] != '\0' && original[n] != '\n' && original[n] != ' ')
			parsed[n] = original[n];
		else break;
	if (strcmp(parsed, expected) == 0)
		return 0;
	else if (strcmp(parsed, "EOF") == 0)
		return 1;
	else
		return -1;
}

/* 
 * Calculate the score of a given answer
 */
int calculate_score(int topic, int internal_qid, char **parsed_request){
	int c = 0, score = 0;
	FILE* fd;
	size_t len = 0;
	char path[BUFFER_32], *read_buffer = NULL;

	sprintf(path, "quest/%d/T%dQ%dA.txt", topic, topic, internal_qid);
	if ((fd = fopen(path, "r")) == NULL){
		perror("[ERROR] File not found.");
		exit(1);
	}
	getline(&read_buffer, &len, fd);
	for (c = 0; c < 5; c++)
		if (parsed_request[c][0] == read_buffer[c*2])
			score += 200;

	free(read_buffer);
	fclose(fd);
	return score;
}
