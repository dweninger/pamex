CC = cc -g

all: interpreter

interpreter: policy-interpreter.c policy-interpreter.h
	${CC} -o $@ $^

clean:
	rm -f interpreter
