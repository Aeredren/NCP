/*
 * server.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// filename, pkt size -> array of buffer containing file data, ready to be sent
char** bufferingFile (char filename, int pktSize);
// any int -> a 6 digit sequence number string
char* itoseq (int seqNb);
// sockaddr pointer, port -> socket number initialized and binded
int initSocket (struct sockaddr_in address, socklen_t sockaddr_in_length, int port);


int main(int argc, const char* argv[]) {
	if (argc != 2){
		printf ("Right usage is : server <PortNumber>\n");
		exit(EXIT_FAILURE);
	}
	int port_accept = atoi(argv[1]);

	struct sockaddr_in listen_addr_accept;
	socklen_t sockaddr_in_length = sizeof(struct sockaddr_in);
	int socket_accept = initSocket(listen_addr_accept,sockaddr_in_length, port_accept);


	if (fork()!=0){
		//printf("I'm the father\n");
	} else {

		//creating a pipe to communicate ack
		int pipefd[2];
		if (-1==pipe(pipefd){
			printf ("pipe creation failed\n");
			exit(EXIT_FAILURE);
		}

		if (0!=fork()){
			//printf("I'm the son\n");
			size_t ackSize=sizeof(char)*6
			char* ack = malloc(ackSize);
			
			while (1) {
				read (pipefd[0], ack, ackSize);
				printf("%s\n", ack);
			}
		}else {
			//printf("I'm the son's son\n");
			size_t ackSize=sizeof(char)*6
			char* ack = malloc(ackSize);
			while (1){
				select {
					recvfrom();
				}

				write(pipefd[1], ack, ackSize)
			}
		}
	}

	return(EXIT_SUCCESS);
}

char** bufferingFile (char filename, int pktSize){
	char** bufferArray;

	FILE* fp;

	return bufferArray;
}

char* itoseq (int seqNb){
	char* seqNbStr = malloc(7*sizeof(char));

	seqNb = seqNb%1000000;
	sprintf(seqNbStr, "%d", seqNb);
	int strSize = strlen(seqNbStr);
	int nbZero = 6-strSize;
	memmove(&seqNbStr[nbZero],&seqNbStr[0],strSize);
	for (int i=0; i<nbZero; i++) seqNbStr[i]='0';

	return seqNbStr;
}

int initSocket (struct sockaddr_in address, socklen_t sockaddr_in_length, int port){
	int sock;

	printf("Creating socket\n");
	if ((sock= socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket creation impossible");
		exit(EXIT_FAILURE);
	} else printf("socket created\n");
	memset((char*) &address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;
	if ( bind(sock, (struct sockaddr*) &address, sockaddr_in_length ) < 0) {
		perror("socket bind impossible");
		exit(EXIT_FAILURE);
	} else printf("socket binded \n" );

	return sock;
}

