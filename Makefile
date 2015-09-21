CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic
EXECS =	ecp_api user 

all: $(EXECS)

ecp_api: resources.c

user: resources.c

clean:
	rm $(EXECS)

