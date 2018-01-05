all: node 

node: functions.o node.o
	gcc -o node functions.o node.c

node.o: node.h node.c
	gcc -g -c node.c -Wall

functions.o: node.h functions.c
	gcc -g -c functions.c -Wall

clean:
	rm *.o node 