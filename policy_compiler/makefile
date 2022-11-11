CC = cc -g
LEX = flex
YACC = bison

all: parser

parser: semantics.c policy.c policy.tab.c
	${CC} -D TEST -o $@ $^

policy.tab.c policy.tab.h: policy.y
	${YACC} -vd policy.y		

policy.c: policy.l
	${LEX} -o $*.c $<

policy.o: policy.c policy.tab.h
