#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define NUM_STR 1024
#define STR_LEN 1000

/* Global Variables */
char theArray[NUM_STR][STR_LEN];
int* seed;
pthread_mutex_t mutex;
char buf[STR_LEN];
char* clientGarbage;
int numRequests;

/* Prototyping */
void *Operate(void* clientFileDescriptor);

int main(int argc, char* argv[]) {

	if(argc != 2){
		perror("incorrect number of args: %s <size of array>", argv[0]);
	}

	struct sockaddr_in sock_var;
	int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	int clientFileDescriptor;
	int i = 0;
	pthread_t t[strtol(argv[2],NULL,10)];

	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port=3000;   //strtol(argv[1],NULL,10);
	sock_var.sin_family=AF_INET;

	setsockopt(serverFileDescriptor,SOL_SOCKET, SO_REUSEADDR,&i,sizeof(i) );

	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	{
		printf("nsocket has been created");
		listen(serverFileDescriptor,2000); 

		numRequests = 0;
		while(numRequests < argv[1]) {
			if((clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL)) != -1){
				printf("nConnected to client %dn",clientFileDescriptor);
				pthread_create(&t,NULL,Operate,(void *)clientFileDescriptor);
			}
		}
		close(serverFileDescriptor);
	}
	else{
		printf("\nsocket creation failed\n");
	}
	return 0;
}

void *Operate(void* clientFileDescriptor) {
	long my_rank = (long) clientFileDescriptor;
	
	// Wait for client
	while(read(clientFileDescriptor, clientGarbage, STR_LEN) != 0) {}

	// Find a random position in theArray for read or write
	int pos = rand_r(&seed[my_rank]) % NUM_STR;
	int randNum = rand_r(&seed[my_rank]) % 100;	// write with 5% probability
	
	pthread_mutex_lock(&mutex); 
	
	if (randNum >= 95) {// 5% are write operations, others are reads
		snprintf(buf, sizeof(buf), "%s%d%s", "String ", pos, " has been modified by a write request");
		strcpy(theArray[pos], buf);
	}

	// return the value read or written
	write(clientFileDescriptor, theArray[pos], STR_LEN); 
	numRequests ++;
	
	pthread_mutex_unlock(&mutex);
		
	return NULL;
}