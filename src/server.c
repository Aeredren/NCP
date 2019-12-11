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

#define SEQ_NUMBER_LENGTH 6
#define DATA_LENGTH 1018

// filename, pkt size -> array of buffer containing file data, ready to be sent
char** bufferingFile (char* filename, int pktSize);
// any int -> a 6 digit sequence number string
char* itoseq (int seqNb);
// sockaddr pointer, port -> socket number initialized and binded
int initSocket (struct sockaddr_in address, socklen_t sockaddr_in_length, int port);


int main(int argc, const char* argv[]) {
	/*if (argc != 2){
		printf ("Right usage is : server <PortNumber>\n");
		exit(EXIT_FAILURE);
	}
	int port_accept = atoi(argv[1]);

	struct sockaddr_in listen_addr_accept;
	socklen_t sockaddr_in_length = sizeof(struct sockaddr_in);
	int socket_accept = initSocket(listen_addr_accept,sockaddr_in_length, port_accept);

	if (fork()!=0){
		printf("I'm the father\n");
		sleep(15);
	} else {
		if (0!=fork()){
			sleep(3);
			printf("I'm the son\n");
			sleep(12);
		}else {
			sleep(5);
			printf("I'm the son's son\n");
			sleep(10);
		}
	}
	return(EXIT_SUCCESS);*/

	char** buffedArray;
	buffedArray = bufferingFile("test.jpeg", 1024);
}

char** bufferingFile (char* filename, int pktSize){
	FILE* fp;
	int fileSize;
	char* seqNb;
	int nbBuff;
	int j=0;

	//Open file
	sprintf (filename, "./files/%s", filename);
	printf("%s",filename);
	fp = fopen(filename, "r");

	if (fp == NULL)
	{
		printf("\nFile open failed!\n");
		exit(0);
	} else printf("\nFile Successfully opened!\n");

	//Find file size
	fseek(fp, 0L, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	//copy of file into the buffer
	char buffer[fileSize];
	fread(buffer,fileSize,1,fp);

	//Number of buffers to create
	nbBuff = (fileSize/1018)+1;

	//Creation of buffer to return
	char bufferArray[nbBuff][pktSize];

	for(int i = 0; i< nbBuff-1; i++)
	{
		seqNb = itoseq(i);
		j = i*DATA_LENGTH;
		memcpy(bufferArray[i],seqNb, SEQ_NUMBER_LENGTH);
		memcpy(bufferArray[i]+SEQ_NUMBER_LENGTH, buffer+j,DATA_LENGTH);

	}
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
