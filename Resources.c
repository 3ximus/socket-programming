#include "Resources.h"

/* Le a informação de input do utilizador */
Server getUserInputInfo(int argc, char *argv[])
{	
	Server ecp;

	const char* inputErr = "\n\nInput format: ./user SID [-n ECPname] [-p ECPport]\n\nSID: student identity number.\nECPname: name of ECP server (opt).\nECPport: well-known port of ECP server (opt).\n\n";

	switch(argc){
		case 1:
			printf("[ERROR]: Not enough arguments.%s",inputErr);
			exit(1);

		case 2:
			if(gethostname(ecp.name,sizeof(ecp.name))){
	 			printf("ERROR: gethostname()\n");
	 			exit(1);
			}
	 		ecp.port = DEFAULT_PORT;
	 		break;

		case 4:
			strcpy(ecp.name,argv[3]);
	 		ecp.port = DEFAULT_PORT;
	 		break;

		case 6:
			strcpy(ecp.name,argv[3]);
			ecp.port = atoi(argv[5]);
			break;

		default:
			printf("[ERROR]: Wrong input format.%s",inputErr);
			exit(1);
	}

	return ecp;
}

const char* readFromFile(const char *file_name)
{
   	char ch;
   	FILE *fp;
   	char *topics = (char *) malloc(100*sizeof(char));
   	int i;
 
    if((fp = fopen(file_name,"r")) == NULL)
    {
   	   printf("ERROR: fopen(). Failed to open file.\n");
   	   free(topics);
       exit(1);
    }

	i = 0;
	 
	while((ch = fgetc(fp)) != EOF){
	    topics[i] = ch;
	   	i++;
	}

	if(i == 0)
		strcpy(topics,"EOF");
	else
    	topics[i] = '\0';
    
    /* printf("file_content: %s\n",topics);*/

    fclose(fp);

    return topics;
}

/* 
Devolve um tabela de strings em que cada entrada foi separada por um simbolo de separacao.
input: char* msg = "Ole|Ola", char* delim = "|"
char **table = parseString(msg,delim);
output: char table[0] = Ole
		char table[1] = Ola
*/
char **parseString(char* msg , const char* delim)
{
	int i = 0; 

	char **tokens = (char **) malloc(TOPIC_NR*sizeof(char *));

	tokens[i] = strtok(msg,delim);

	while(tokens[i] != NULL){
		i++;
		tokens[i] = strtok(NULL,delim);
	}

	return tokens;
}

/* Faz o print dos topicos de topic.txt */
void printTopics(char **topics)
{	
	if((strcmp(topics[0],"AWT")) == 0)
	{
		int i = 2, j = 1;
		int topic_nr = atoi(topics[1]);

		for (; j <= topic_nr ; ++j)
		{
			printf("%d - ",j);

			topics[i] = strtok(topics[i],"|");

			while(topics[i] != NULL)
			{
				printf("%s ",topics[i]);
				topics[i] = strtok(NULL,"|");
			}

			i++;
			printf("\n");
		} 
	}		
	else
		printf("Failed answer\n");
}

/* Devolve o ip do host */
int getHostIP(const char *host_name)
{
	struct hostent *h;
	struct in_addr *a;

	if((h = gethostbyname(host_name)) == NULL)
	{
		printf("ERROR: gethostbyname()\n");
		exit(0);
	}

	a = (struct in_addr *) h->h_addr_list[0];

	/*printf("IP adress: %s (%X)\n",inet_ntoa(*a),ntohl(a->s_addr));*/

	return a->s_addr;
}

/* 
Devolve informacoes sobre a maquina que contactamos:
- Nome do computador
- IP
- Porta 
 */
void getHostAdress(struct sockaddr_in addr)
{
	struct hostent* h;

	h = gethostbyaddr((char *)&addr.sin_addr,sizeof(struct in_addr),AF_INET);

	if(h == NULL)
		printf("Sent by %s:%hu\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
	else
		printf("\nSent by, Server: %s\n\t IP  : %s\n\t Port: %hu\n\n",h->h_name,inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
}


/* Pode ser que deem jeito */
/*char **createTable(int rows, int row_length) 
{	
   int i;
   char **table = (char **) malloc(rows*sizeof(char *));

   for(i = 0; i < rows; i++)
       table[i] = (char *) malloc(row_length*sizeof(char));

   return table;
}

void freeTable(char **table, int rows)
{	
	int i;

    for(i = 0; i < rows; i++)
        free(table[i]);

    free(table);
}*/
