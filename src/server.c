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

#define ACK_LENGTH 6
#define SEG_SIZE 1024
#define INIT_WIN_SIZE 5

// filename, pkt size -> array of buffer containing file data, ready to be sent
char** bufferingFile (char* filename, int pktSize);
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

	size_t synSize = 16*sizeof(char);
	char* synBuffer = malloc(synSize);
	int port_com = port_accept;

	while (1) {
		recvfrom(socket_accept, synBuffer, synSize, 0, (struct sockaddr*) &listen_addr_accept, &sockaddr_in_length );
		if (strncmp("SYN", synBuffer, 3) != 0) {
			printf("This is not a SYN\n");
			exit(EXIT_FAILURE);
		}else{
			printf("SYN received\n");
		}

		port_com++;
		printf("port for socket_com : %d\n",port_com );

		//Creation et bind du socket_com
		struct sockaddr_in listen_addr_com;
		int socket_com = initSocket(listen_addr_com,sockaddr_in_length, port_com);


		if (fork()!=0){
			printf("I'm the father\n");

			close (socket_com);
			// syn-ack
			sprintf(synBuffer, "%s%d", "SYN-ACK", port_com);
			sendto(socket_accept, (const char *)synBuffer, strlen(synBuffer), MSG_CONFIRM, (const struct sockaddr *) &listen_addr_accept, sockaddr_in_length);
			printf("SYN-ACK send\n");
			//ack
			recvfrom(socket_accept, synBuffer, synSize, 0, (struct sockaddr*) &listen_addr_accept, &sockaddr_in_length );
			if (strncmp("ACK", synBuffer, 3) != 0) {
				printf("This is not an ACK\n");
				exit(EXIT_FAILURE);
			}else printf("ACK received\n");

		} else {
			printf("I'm the son\n");
			close (socket_accept);

			size_t filenameSize = 128*sizeof(char);
			char* filename = malloc(filenameSize);
			recvfrom(socket_com, filename, filenameSize, 0, (struct sockaddr*) &listen_addr_com, &sockaddr_in_length );
			//char** segmentArray = bufferingFile (filename, SEG_SIZE);

			size_t ackSize = sizeof(char)*ACK_LENGTH;
			char* ack = malloc(ackSize);
			int window = INIT_WIN_SIZE;

			while (1) {
				for (window, window>0, window--){
					//send the messages
				}

				//select timeout on 1 ack

				//Choose what to do
			}
			return(EXIT_SUCCESS);
		}
	}

	return(EXIT_SUCCESS);
}

char** bufferingFile (char* filename, int pktSize){
	char** bufferArray;

	FILE* fp;

	return bufferArray;
}

char* itoseq (int seqNb){
	char* seqNbStr = malloc(7*sizeof(char));

	seqNb = seqNb%1000000;
	sprintf(seqNbStr, "%d", seqNb);
	int strSize = strlen(seqNbStr);
	int nbZero = ACK_LENGTH-strSize;
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

