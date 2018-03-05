all : myshell slow

myshell : myshell.o parse.o
	gcc -o myshell parse.o myshell.o

slow : slow.o
	gcc -o slow slow.o

parse.o : parse.h parse.c
	gcc -g -c parse.c -Wall

myshell.o : myshell.h myshell.c
	gcc -g -c myshell.c -Wall

slow.o : slow.c
	gcc -g -c slow.c -Wall

clean :
	rm myshell.o parse.o slow.o myshell slow
