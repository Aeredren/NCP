all: serveur1 serveur2 serveur3

serveur1: serveur.o
	gcc -Wall ./obj/serveur.o -o ./bin/serveur1-TeamTCDP

serveur.o: ./src/serveur.c
	gcc -Wall -c ./src/serveur.c -o ./obj/serveur.o

serveur2: serveur1
	ln ./bin/serveur1-TeamTCDP ./bin/serveur2-TeamTCDP

serveur3: serveur1
	ln ./bin/serveur1-TeamTCDP ./bin/serveur3-TeamTCDP

clean:
	rm -fv ./obj/*  ./bin/serveur*
