Questionnaire Service:
============================

# Structure:

## User:
User shell commands work as follows:
    * **list**
	Sends TQR request to server and waits for a AWT reply
	> TQR format : "TQR\n"
	> AWT format : "AWT [topics ammount] [topics-list]\n"

    + **request**
	Sends a TER request and waits for a AWTES reply
	> TER format : "TER [topic-number]\n"
	> AWTES format : "AWTES [TES-Server-IP] [TES-Server-Port]\n"

    - **submit**

    > TODO

    4. **help**

        Shows help screen, listing commands and their effects

    5. **exit**
	Exits application

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
- Fazer a reply AWTES
- Tratar das replyes ERR e EOF
> Tanto envia-las (server) como analisa-las (user ou client?)
- Comecar a fazer o servidor TES
> Usar talvez o mesmo esquema implementado no ecp_server_interface <-> udp_server

* * *

# Bugs:
- Multiplas chamadas do list estragam o 1º argumento a nao ser que se reinicie o servidor, portanto suponho que o problema seja no servidor.