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

** TO NOTE: **
All memory must be freed on every cicle (after command is processed)


## ECP Server:
This serves a shell waiting for user to execute the exit command, terminating the server


## TES Server:
This serves a shell waiting for user to execute the exit command, terminating the server

## Protocol:
Use all xx_request() functions to communicate with the server if using the client, never communicate directly.
Use all xx_reply() functions to build the server replies.

* * *

# To Do:
- **[HIGH]** Tratar das replyes ERR e EOF
> Tanto envia-las (server) como analisa-las (user ou client?)
- Fazer com que o request receba pdf
> Implica fazer os timestamps nos pedidos e atribuicao dos qid
- Fazer o submit
- Fazer timeout nas ligaceos UDP

* * *
# Bugs:

- **[HIGH]** a parte do server que esvreve para a socket nao está preparada para enviar o ficheiro pdf
- **[MEDIUM]** request com numero invalido (mt grande) fica preso no servidor ecp pq ele fecha a ligacao.
- **[MEDIUM]** as funcoes que leem os argumentos para que nao aceitem qualquer opcao (-p -n, etc..)
- **[LOW]** Será que a maneira como o server esta feito (loop do write dentro do loop do read) nao criará problemas quando o read nao conseguir ler tudo de uma vez? Quando isto acontecer a request nao vai ser analisada (parsed) na totalidade... Talvez nao haja problema pois as request sao pequenas... Mas se houver bugs isto poderá ser uma razao.

* * *
# Notes:
- When creating a new server with start_udp_server(...) or start_tcp_server the return value (child_pid) should be caught, because it controls the child process where the server is actually running;


