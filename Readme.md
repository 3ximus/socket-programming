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
    > RQT format : "RQT [sid]\n"
    > AQT format : "AQT [qid] [time] [size] [data]\n"

3. submit
    - Sends a RQS request and waits for AQS reply:
    > RQS format: "RQS [answer-sequence]\n"
    > AQS format: "AQS [qid] [score]\n"
    *NOTE* Here the server comunicates the score to the ecp server
    > IQR format : "IQR [sid] [qid] [topic-name] [score]\n"
    > AWI format : "AWI [qid]\n"

4. help
    - Shows help screen, listing commands and their effects

5. exit
    - Exits application

## ECP Server:
This serves a shell waiting for user to execute the exit command, terminating the server


## TES Server:
This serves a shell waiting for user to execute the exit command, terminating the server

*TO NOTE* This server defines the amount of time allowed on the submissions

## Usage
Open ECP server:

`./ecp_server_interface`

To exit type "exit" and press "return".

Open TES server:

`./tes_server_interface -p 5700x`
"where x is number of the topic"

To exit type "exit" and press "return".

Open USER:

`./user xxx`
"to open on local host, with xxx being SID"

`./user xxx -n yyy -p zzz`
"to connet to host yyy on port zzz with SID xxx"

## Protocol:
Use all xx_request() functions to communicate with the server if using the client, never communicate directly.
Use all xx_reply() functions to build the server replies.

# To Do:
- **[HIGH]** adicionar checks em todo o lado para nao haver erros
> apanhar erros nas funcoes usadas, nao aceder a posicoes de parsed strings que podem nao existir, etc

# Bugs / Known Issues:
- **[HIGH]** score nao e bem calculado

# Notes:
- When creating a new server with start_udp_server(...) or start_tcp_server the return value (child_pid) should be caught, because it controls the child process where the server is actually running;
