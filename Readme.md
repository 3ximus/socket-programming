Questionnaire Service:
============================

# Structure:

## User:

1. list
    - Sends TQR request to server and waits for a AWT reply
    > TQR format : "TQR\n"
    > AWT format : "AWT [topics ammount] [topics-list]\n"

2. request
    - Sends a TER request and waits for a AWTES reply
    > TER format : "TER [topic-number]\n"
    > AWTES format : "AWTES [TES-Server-IP] [TES-Server-Port]\n"
    - Then sends a RQT request and waits for AQT reply
    > RQT format : "RQT [sid]"
    > AQT format : "AQT [qid] [time] [size] [data]"

3. submit
> TODO

4. help
    - Shows help screen, listing commands and their effects

5. exit
    - Exits application

## ECP Server:
This serves a shell waiting for user to execute the exit command, terminating the server


## TES Server:
This serves a shell waiting for user to execute the exit command, terminating the server

** TO NOTE **
This server defines the amount of time allowed on the submissions

## Protocol:
Use all xx_request() functions to communicate with the server if using the client, never communicate directly.
Use all xx_reply() functions to build the server replies.

* * *

# To Do:
- **[HIGH]** adicionar checks em todo o lado para nao haver erros
> apanhar erros nas funcoes usadas, nao aceder a posicoes de parsed strings que podem nao existir, etc
- **[HIGH]** Tratar das replyes ERR e EOF
> Tanto envia-las (server) como analisa-las (user ou client?)
- **[HIGH]** Fazer com que o request receba pdf
> Implica fazer os timestamps nos pedidos e atribuicao dos qid
- **[HIGH]** Fazer o submit
- **[MEDIUM]** Arranjar maneira de calcular o offset do pdf na reply, por agora usei um valor precalculado qu nao vai resultar se a reply tiver um tamanho diferente
- **[MEDIUM]** Fazer timeout nas ligaceos UDP
- **[LOW]** corrigir parseOpt
- **[LOW]** corrigir parseString

* * *
# Bugs:
- **[HIGH]** nao da para fazer um request sem um list primeiro??
- **[HIGH]** request nao parece que recebe bem a reply com o pdf, aumentei o tamanho para poder conter o ficheiro mas este nao e passado na totalidade...
- **[MEDIUM]** a parte do server que esvreve para a socket pode nao estar preparada para enviar o ficheiro pdf
- **[MEDIUM]** temos de limpar os buffers no cliente / servidor que tem as request e replies porque de vez em quando aparece lixo na mensagem
- **[MEDIUM]** request com numero invalido (mt grande) fica preso no servidor ecp pq ele fecha a ligacao.
- **[MEDIUM]** corrigir as funcoes que leem os argumentos para que nao aceitem qualquer opcao (-p -n, etc..)

* * *
# Notes:
- When creating a new server with start_udp_server(...) or start_tcp_server the return value (child_pid) should be caught, because it controls the child process where the server is actually running;


