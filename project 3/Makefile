all: bbserver bbpeer

bbserver: server.o
	gcc -o bbserver server.o

server.o: server.c
	gcc -g -c server.c -Wall

bbpeer: bbpeer.o
	gcc -pthread -o bbpeer bbpeer.o

bbpeer.o: peer.h bbpeer.c
	gcc -pthread -g -c bbpeer.c -Wall

clean:
	rm bbserver bbpeer server.o bbpeer.o 