CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -g
EXECS =	ecp_server_interface tes_server_interface user

all: $(EXECS)

ecp_server_interface: resources.c 

tes_server_interface: resources.c 

user: resources.c

clean:
	rm $(EXECS)

