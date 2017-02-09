#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "timer.h"
#include <pthread.h>

#define STR_LEN 1000
#define thread_count 1000

unsigned int *seed;
int num_str;
int port;

/* Prototyping */
void *rw_array(void* rank);

int main(int argc, char* argv[]) {

	if (argc != 3) {
		printf("Incorrect number or args: %s <port#> <#_of_strings>\n", argv[0]);
		exit(0);
	}

	num_str = atoi(argv[2]);
	port = atoi(argv[1]);
	/* Initialize RNG */
	int i;
	seed = malloc(thread_count*sizeof(int));
	for (i = 0; i < thread_count; i ++) {
		seed[i] = i;
	}

	pthread_t* thread_handles;
	thread_handles =  malloc(thread_count*sizeof(pthread_t));
		
	//create threads
	int thread;
	for(thread = 0; thread < thread_count; thread++){
			
		int rc = pthread_create(&thread_handles[thread], NULL, rw_array, (void*)thread);
		if(rc != 0){
			perror("Error creating threads");
		}
	}

	for (thread = 0; thread< thread_count; thread++){
		pthread_join(thread_handles[thread], NULL);
	}
	free(thread_handles);
	free(seed);
	return 0;
}

void *rw_array(void* rank){

	long my_rank = (long) rank;

	struct sockaddr_in sock_var;
	int clientFileDescriptor=socket(AF_INET,SOCK_STREAM,0);

	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port = port;
	sock_var.sin_family=AF_INET;

	if(connect(clientFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0){

		// Find a random position in theArray for read or write
		int pos = rand_r(&seed[my_rank]) % num_str;
		int randNum = rand_r(&seed[my_rank]) % 100;	// write with 5% probability

		char str_cli[STR_LEN];
		char str_ser[STR_LEN];

		if (randNum >= 95) { // 5% are write operations, others are reads
			snprintf(str_cli, STR_LEN, "%d%s%d", pos, " ", 1);
			write(clientFileDescriptor, str_cli,STR_LEN);
			//read(clientFileDescriptor, str_ser, STR_LEN);
		} else {
			snprintf(str_cli, STR_LEN, "%d%s%d", pos, " ", 0);
			write(clientFileDescriptor, str_cli, sizeof(str_cli));
			//read(clientFileDescriptor, str_ser, STR_LEN);
		}
		
		read(clientFileDescriptor, str_ser, STR_LEN);
		//printf("%s\n", str_ser);

		close(clientFileDescriptor);

		return NULL;
	} else {
		return NULL;
	}
}