#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "timer.h"

#define STR_LEN 1000
#define thread_count 1000
#define R 0
#define W 1

typedef struct {
	int ID; // arrayID
	int action; // R/W
} array_param;

int *seed;
int num_str;
int clientFileDescriptor;

/* Prototyping */
void *rw_array(void* rank);

int main(int argc, char* argv[]) {

	if (argc != 3) {
		printf("Incorrect number or args: %s <port#> <#_of_strings>\n", argv[0]);
		exit(0);
	}

	num_str = atoi(argv[2]);

	/* Initialize RNG */
	int i;
	seed = malloc(thread_count*sizeof(int));
	for (i = 0; i < thread_count; i ++) {
		seed[i] = i;
	}

	pthread_t* thread_handles;
	thread_handles =  malloc(thread_count*sizeof(pthread_t));

	struct sockaddr_in sock_var;
	int clientFileDescriptor=socket(AF_INET,SOCK_STREAM,0);

	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port = strtol(argv[1],NULL,10);
	sock_var.sin_family=AF_INET;

	if(connect(clientFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0) {
		
		double start, end, total;

		//time start
		GET_TIME(start);
		
		//create threads
		int thread;
		for(thread = 0; thread < thread_count; thread++){
			int rc = pthread_create(&thread_handles[thread], NULL, rw_array, (void*) thread);

			if(rc != 0){
				perror("Error creating threads");
			}
		}

		//close threads
		for(thread = 0; thread < thread_count; thread++) {
			pthread_join(thread_handles[thread], NULL);
		}

		//time end
		GET_TIME(end);
		
		//calculate total elapsed time
		total = end - start;

		printf("Total time: %e\n", total);

	} else {
		printf("socket creation failed");
	}

	free(thread_handles);
	free(seed);
	return 0;
}

void *rw_array(void* rank){

	long my_rank = (long) rank;
	char str_ser[STR_LEN];

	// Find a random position in theArray for read or write
	int pos = rand_r(&seed[my_rank]) % num_str;
	int randNum = rand_r(&seed[my_rank]) % 100;	// write with 5% probability
	
	array_param id_rw;
	id_rw.ID = pos;

	if (randNum >= 95) { // 5% are write operations, others are reads
		id_rw.action = W;
		write(clientFileDescriptor, id_rw.ID, sizeof(int));
		read(clientFileDescriptor, str_ser, STR_LEN);
	} else {
		id_rw.action = R;
		write(clientFileDescriptor, id_rw.ID, sizeof(int));
		read(clientFileDescriptor, str_ser, STR_LEN);
	}
	printf("%s\n", str_ser);

	return NULL;
}