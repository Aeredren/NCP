all: server 

server: server.o
	gcc -Wall ./obj/server.o -o ./bin/server.x86_64

server.o: ./src/server.c
	gcc -Wall -c ./src/server.c -o ./obj/server.o

clean:
	rm -fv ./obj/*  ./bin/server.x86_64
