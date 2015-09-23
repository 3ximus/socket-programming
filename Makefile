CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic
EXECS =	ecp_server_interface user 

all: $(EXECS)

ecp_server_interface: resources.c

user: resources.c

clean:
	rm $(EXECS)

