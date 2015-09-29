#include "resources.h"

/*
 * Passes options passed to exec
 */
struct server *optParser(int argc, char *argv[]){	
	struct server *ecp = (struct server *)malloc(sizeof(struct server));

	char* inputErr = "\n\nInput format: ./user SID [-n ECPname] [-p ECPport]\n\nSID: student identity number.\nECPname: name of ECP server (opt).\nECPport: well-known port of ECP server (opt).\n\n";

	switch(argc){
		case 1:
			printf("[ERROR]: Not enough arguments.%s",inputErr);
			exit(1);

		case 2:
			if(gethostname((char *)ecp->name, sizeof(ecp->name))){
				printf("ERROR: gethostname()\n");
				exit(1);
			}
	 		ecp->port = DEFAULT_PORT_ECP;
	 		break;

		case 4:
			strcpy((char *)ecp->name,argv[3]); /* CORRECT for buffer overflow */
	 		ecp->port = DEFAULT_PORT_ECP;
	 		break;

		case 6:
			strcpy((char *)ecp->name,argv[3]); /* CORRECT for buffer overflow */
			ecp->port = atoi(argv[5]);
			break;

		default:
			printf("[ERROR]: Wrong input format.%s",inputErr);
			exit(1);
	}

	return ecp;
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
	char read_buffer[100];
	char *accumulator_buffer = (char *) malloc(BUFFER_2048 * sizeof(char ));

	fd = open(file_name, O_RDONLY, S_IRUSR|S_IWUSR);
	if (fd == -1){
		/* handle error */
		perror("[ERROR] Opening topics.txt file\n");
		exit(-1);
	}
	/* while EOF isnt reached */
	while((bytes_read = read(fd, read_buffer, 100)) > 0){
		/* build file content on the accumulator */
		strcat(accumulator_buffer, read_buffer);
	}

	close(fd);

	/* return raw file data */
	return accumulator_buffer;
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
	char *read_buffer = NULL, **parsed_line, *content = (char *)malloc(BUFFER_32 * sizeof(char));
	
	memset((void *)content,'\0', BUFFER_32);

	fd = fopen(TOPICS_FILE, "r");
	
	while (i <= search_me){
		bytes_read = getline(&read_buffer, &len, fd);
		if (bytes_read == -1){
			strcpy(read_buffer, "EOF");
			break;
		}
		i++;
	}
	parsed_line = parseString(read_buffer, " ");
	/* add IP */
	strcpy(content, parsed_line[1]);
	strcat(content, " ");
	/* add port */
	strcat(content, parsed_line[2]);

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
Devolve um tabela de strings em que cada entrada foi separada por simbolo de separacao.
input: char* msg = "Ole|Ola", char* delim = "|"

output: char table[0] = Ole
		char table[1] = Ola
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
 * Checks if answer is between A and D (not case sensitive)
 */
int checkSubmitAnswer(char *answ){
	if(((strcmp(answ,"A")) == 0) || ((strcmp(answ,"a")) == 0))
		return 0;

	if(((strcmp(answ,"B")) == 0) || ((strcmp(answ,"b")) == 0))
		return 0;

	if(((strcmp(answ,"D")) == 0) || ((strcmp(answ,"c")) == 0))
		return 0;

	if(((strcmp(answ,"D")) == 0) || ((strcmp(answ,"d")) == 0))
		return 0;

	return 1;
}

