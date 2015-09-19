CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic
EXECS =	ECP user 

all: $(EXECS)

ECP: Resources.c UDP.c 

user: Resources.c UDP.c 

clean:
	rm $(EXECS)

