all: server1 server2

server1: server1.o
	gcc -Wall ./obj/server1.o -o ./bin/server1.x86_64

server1.o: ./src/server1.c
	gcc -Wall -c ./src/server1.c -o ./obj/server1.o

server2: server2.o
	gcc -Wall ./obj/server2.o -o ./bin/server2.x86_64

server2.o: ./src/server2.c
	gcc -Wall -c ./src/server2.c -o ./obj/server2.o

clean:
	rm -fv ./obj/*  ./bin/server*
