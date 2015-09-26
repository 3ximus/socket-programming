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

3. submit
> TODO

4. help
    - Shows help screen, listing commands and their effects

5. exit
    - Exits application

** TO NOTE: **
All memory should be freed on every cicle (after command is processed)


## ECP Server:
This serves a shell waiting for user to execute the exit command, terminating the server

On this implementation the *udp_server* is responsible for handling requests and serving the appropriate reply, this isnt ideal, it should be the *ecp_server_interface* handling this.

** TO NOTE: **
When creating a new server with start_udp_server(...) the return value (child_pid) should be caught, because it controls the child process where the server is actually running;


## TES Server:
(empty)

## Protocol:
(empty)

* * *

# To Do:
- Tratar das replyes ERR e EOF
> Tanto envia-las (server) como analisa-las (user ou client?)
- Acabar o request
- Fazer o submit

* * *
# Bugs:
- Será que a maneira como o server esta feito (loop do write dentro do loop do read) nao criará problemas quando o read nao conseguir ler tudo de uma vez? Quando isto acontecer a request nao vai ser analisada (parsed) na totalidade... Talvez nao haja problema pois as request sao pequenas... Mas se houver bugs isto poderá ser uma razao.