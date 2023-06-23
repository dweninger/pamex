CC = gcc
CFLAGS = -fPIC -Wall -Werror

all: pam_pamex.so

pam_pamex.so: pam_pamex.o
	$(CC) -shared -o $@ $^

pam_pamex.o: pam_pamex.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f pam_pamex.o pam_pamex.so