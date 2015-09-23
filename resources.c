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
	 		ecp->port = DEFAULT_PORT;
	 		break;

		case 4:
			strcpy((char *)ecp->name,argv[3]); /* CORRECT for buffer overflow */
	 		ecp->port = DEFAULT_PORT;
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
 * Log action on server
 * Types are unix standard 64 present in /usr/include/x86_64-linux-gnu/asm/unistd_64.h
 * for the first 4 types of log actions
 * 0 - read
 * 1 - write
 * 2 - open
 * 3 - close
 * From here extra ones were added
 * 4 - log
 * 5 - warning
 * 6 - errror
 */
void log_action(char* file_path, char* msg, int type){
	char buffer[LOG_BUFFER_SIZE];
	time_t now;
	struct tm *time_struct;
	int fd;
	bzero(buffer, LOG_BUFFER_SIZE);

	/* Try to open open or create log file */
	fd = open(file_path, O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR);
	if (fd == -1){
		/* handle error */
		perror("[ERROR] Opening server log file\n");
		exit(-1);
	}
	
	time(&now); /* Get number of seconds since epoch. */
	
	time_struct = localtime((const time_t *)&now); /* Convert to tm struct. */
	strftime(buffer, LOG_BUFFER_SIZE, "%m/%d/%Y %H:%M:%S> ", time_struct);

	switch(type){
		printf("here");
		case 0:
			strcat(buffer, "[READ] :");
			break;
		case 1:
			strcat(buffer, "[WRITE] :");
			break;
		case 2:
			strcat(buffer, "[OPEN] :");
			break;
		case 3:
			strcat(buffer, "[CLOSE] :");
			break;
		case 4:
			strcat(buffer, "[LOG] :");
			break;
		case 5:
			strcat(buffer, "[WARNING] :");
			break;
		case 6:
			strcat(buffer, "[ERROR] :");
			break;
		default:
			perror("[ERROR] On log action, wrong type\n");
			exit(-1);
	}
	strcat(buffer, msg);
	strcat(buffer, "\n");
	write(fd, buffer, strlen(buffer)); /* Write buffer to log. */
	close(fd);
}

/*
 * Read a line from file
 * Returns file content
 * Number of lines read are placed on line_number
 * Note: content needs to be freed
 */
char **readFromFile(const char *file_name, int *line_number){
	char read_buffer[100], accumulator_buffer[BUFFER_SIZE];
	char **content;
	int fd, bytes_read = 100;

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
	content = parseString(accumulator_buffer, "\n");

	/* count lines in file */
	*line_number = 0;
	while (content[*line_number] != NULL)
		*line_number = *line_number + 1;

	close(fd);
	/* return number of lines in file / topics */
	return content;
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
 * Creates a Table from fixed intput
 */
char **createTable(int rows, int row_length) {	
   int i;
   char **table = (char **) malloc(rows * sizeof(char *));

   for(i = 0; i < rows; i++){
       table[i] = (char *) malloc(row_length * sizeof(char));
       bzero(table[i], rows);
   }
   return table;
}

/*
 * Free table memory
 */
void freeTable(char **table, int rows){	
	int i;
    for(i = 0; i < rows; i++)
        free(table[i]);

    free(table);
}

/* 
 * Dumps raw memory in hex byte and printable format
 */
void dump(const unsigned char* data_buffer, const unsigned int length) {
	unsigned char byte;
	unsigned int i, j;
	for (i = 0; i < length; i++) {
		byte = data_buffer[i];
		printf("%02x ", data_buffer[i]);	/* Display in hex byte */
		if (((i%16) == 15) || (i == length - 1)) {
			for (j = 0; j< 15 - (i%16); j++)
				printf("   ");
			printf("| ");
			for (j = (i-(i%16)); j <= i; j++) { /* Display printable bytes from line */
				byte = data_buffer[j];
				if ((byte > 31) && (byte < 127)) /* Outside printable range */
					printf("%c", byte);
				else
					printf(".");
			}
			printf("\n");
		}
	}
}