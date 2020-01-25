/*
 * server.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>

#define RTT_PERCENT 0.125
#define RTT_PERCENT_DEVIATED 0.25
#define RTO_FACTOR 1.5
#define MAX_TIMEOUT 15000

#define SEQ_NUMBER_LENGTH 6
#define DATA_LENGTH 1494
#define ACK_LENGTH 6
#define SEG_SIZE 1500
#define INIT_WIN_SIZE 5
#define INITIAL_TIMEOUT 500


// filename, pkt size -> array of buffer containing file data, ready to be sent
void bufferingFile(char** bufferArray, FILE* fp, int fileSize, int nbBuff);
// any int -> a 6 digit sequence number string
char* itoseq (int seqNb);
// sockaddr pointer, port -> socket number initialized and binded
int initSocket (struct sockaddr_in address, socklen_t sockaddr_in_length, int port);
// convert an ack to its seqnumber string
void acktoseq (char* ack);

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

			FILE* fp;
			int fileSize;
			char nameBuff[100];
			int nbBuff;
			char** bufferArray;

			//Open file
			sprintf (nameBuff, "./files/%s", filename);
			fp = fopen(nameBuff, "r");

			if (fp == NULL)
			{
				printf("\nFile open failed!\n");
				exit(0);
			} else printf("\nFile Successfully opened!\n");

			//Find file size
			fseek(fp, 0L, SEEK_END);
			fileSize = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			//Number of buffers to create
			nbBuff = (fileSize/DATA_LENGTH)+2;
			printf("Number of segment : %d\n", nbBuff );

			bufferArray = malloc(sizeof(char*)*nbBuff);
			for (int i =0; i<nbBuff;i++){
				bufferArray[i]=malloc(sizeof(char)*SEG_SIZE);
			}

			bufferingFile(bufferArray, fp, fileSize, nbBuff);

			size_t ackSize = sizeof(char)*ACK_LENGTH;
			char* ack = malloc(ackSize);
			int lastAck = 0; // is the last ack message. Also, lastAck+1 is the beginning of the sliding window.
			int currentAck = 0;
			int maxWindow = INIT_WIN_SIZE;
			int window = INIT_WIN_SIZE;
			int step=1;
			int RTT=INITIAL_TIMEOUT;
			int RTTs=INITIAL_TIMEOUT;
			int RTTd=INITIAL_TIMEOUT/2;
			int RTO=INITIAL_TIMEOUT;
			int sendSize=SEG_SIZE;
			int lastSeg = 0;
			int endIsAck = 0;
			int isRetransmit = 0; // boolean for RTT Karn's algorithm
			int isFirstLoop = 0; // boolean for knowing if a given loop is at loop 1 or loop > 1
			int duplicateAck = 0; // number of time we get a duplicate ack

			// INIT FD_SET AND TIMEVAL
			fd_set read_set;
			FD_ZERO(&read_set);
			FD_SET(socket_com, &read_set);
			struct timeval timeout; // timeout and timeout_copy because timeout is undefined after select() whereas timeout_copy is not
			clock_t start, stop;// start-stop = Rond Time Trip
			timeout.tv_sec=0;
			timeout.tv_usec=INITIAL_TIMEOUT;

			while (!endIsAck){
				isFirstLoop=1;
				for (window; window>0; window--){
					if (isFirstLoop) {
						//gettimeofday(&start, NULL); // reset the starting time
						start=clock();
						isFirstLoop=0; // reset the first loop flag
						isRetransmit=0; // reset the retransmition flag
					}
					//send the messages
					printf("sending : %d\n", lastSeg);
					sendto(socket_com, bufferArray[lastSeg],sendSize, MSG_CONFIRM, (const struct sockaddr *) &listen_addr_com, sockaddr_in_length);
					if (lastSeg<nbBuff-1) lastSeg++;

				}

				//select timeout on 1 ack
				int selector = select(socket_com+1, &read_set, NULL, NULL, &timeout);
				switch (selector) {
					case -1 : // if error
						exit(EXIT_FAILURE);
						break;
					case 0 : //if we timed out
						printf ("TIMEOUT, resend %d\n", lastAck+1);
						sendto(socket_com, bufferArray[lastAck+1],sendSize, MSG_CONFIRM, (const struct sockaddr *) &listen_addr_com, sockaddr_in_length);
						FD_ZERO(&read_set);
						FD_SET(socket_com, &read_set);

						// put timeout to timeout*RTT_TIMEOUT to avoid karn's algorithm infinite loop
						RTO = RTO*RTO_FACTOR;
						// put retransmition flag to 1
						isRetransmit=1;
						maxWindow=maxWindow/2+1;
						step=1;
						if (window>maxWindow) window=maxWindow;
						break;
					default : //if a ack is receive
						recvfrom(socket_com, ack, ackSize+3, 0, (struct sockaddr*) &listen_addr_com, &sockaddr_in_length );
						memmove (&ack[0], &ack[3], 7); // remove the ACK of ACK000001
						currentAck=atoi(ack);
						printf ("ack is : %s | ack value is : %d\n", ack, currentAck);
						if (currentAck>lastAck){
							// Calculate new Rtt and set timeout only if we never retransmit
							if (!isRetransmit) { 
								stop=clock();// get the time at receiving
								RTT = 1000000*((double)(stop)-(double)(start))/(CLOCKS_PER_SEC);
								RTTs = (1-RTT_PERCENT)*RTTs + RTT_PERCENT*RTT;
								RTTd = (1-RTT_PERCENT_DEVIATED)*RTTs + RTT_PERCENT_DEVIATED*(RTT-RTTs);
								RTO=RTTs+4*RTTd;
								// augment window size
								maxWindow+=step;
								window+=step;
							}
							duplicateAck=0;
							window+=currentAck-lastAck;
							if (window>maxWindow) window=maxWindow;
							lastAck=currentAck;
							printf ("Last ack = %d\n", lastAck);
							if (currentAck==nbBuff-1){
								sendSize = fileSize%DATA_LENGTH+SEQ_NUMBER_LENGTH;
								printf ("SEND SIZE SET TO LAST SEGMENT SIZE %d", sendSize);
							}
						}else if (currentAck==lastAck) {
							duplicateAck++;
							if (duplicateAck>=2) {
								printf ("3 DUPLICATE, resend %d\n", lastAck+1);
								sendto(socket_com, bufferArray[lastAck+1],sendSize, MSG_CONFIRM, (const struct sockaddr *) &listen_addr_com, sockaddr_in_length);
								isRetransmit=1;
								maxWindow=maxWindow/2+1;
								step=1;
								if (window>maxWindow) window=maxWindow;
								duplicateAck=0;
								//RTO = RTO*RTO_FACTOR;
							}
						}
						break;
				}

				if (RTO > MAX_TIMEOUT) RTO=MAX_TIMEOUT;
				timeout.tv_usec=RTO;	
				printf("RTT:%d,%d,%d | RTO:%d | TIMEOUT:%ld | window:%d\n", RTT, RTTs, RTTd, RTO, timeout.tv_usec, maxWindow);

				if (lastAck == nbBuff-1) endIsAck = 1;
			}

			printf ("SENDING FIN, nbBuff=%d\n",nbBuff);
			sendto(socket_com, "FIN", 3, MSG_CONFIRM, (const struct sockaddr *) &listen_addr_com, sockaddr_in_length);

			sleep(1);
			close(socket_com);
			return(EXIT_SUCCESS);
		}
	}

	return(EXIT_SUCCESS);
}

void bufferingFile(char** bufferArray, FILE* fp, int fileSize, int nbBuff){
	int j=0;
	int i;
	char* seqNb;

	//copy of file into the buffer
	//char buffer[fileSize];
	//fread(buffer,fileSize,1,fp);

	bzero(bufferArray[0], SEG_SIZE);

	for(i = 1; i< nbBuff-1; i++)
	{
		seqNb = itoseq(i);
		j = (i-1)*DATA_LENGTH;
		memcpy(bufferArray[i],seqNb, SEQ_NUMBER_LENGTH);
		fread (bufferArray[i]+SEQ_NUMBER_LENGTH, 1, DATA_LENGTH, fp);
		fseek(fp, DATA_LENGTH, j);
	}

	seqNb = itoseq(i);
	j = (i-1)*DATA_LENGTH;
	memcpy(bufferArray[i],seqNb, SEQ_NUMBER_LENGTH);
	fread (bufferArray[i]+SEQ_NUMBER_LENGTH, 1, fileSize-j, fp);
	
	printf ("SEND SIZE SET TO LAST SEGMENT SIZE %d", (fileSize-j+SEQ_NUMBER_LENGTH));

	return;
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

void acktoseq (char* ack) {
	memmove (&ack[0], &ack[3], 6);
}
