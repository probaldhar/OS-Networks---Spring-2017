all : mt-collatz

mt-collatz : time.o mt-collatz.o
	gcc -o mt-collatz time.o mt-collatz.o

mt-collatz.o : mt-collatz.h mt-collatz.c
	gcc -g -c mt-collatz.c -Wall

time.o : time.h time.c
	gcc -g -c time.c -Wall

clean :
	rm mt-collatz.o time.o mt-collatz
