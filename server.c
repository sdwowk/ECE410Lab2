#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "timer.h"

#define STR_LEN 1000
#define thread_count 1000

pthread_mutex_t mutex;
char** theArray;
int num_str;
double total;

/* Prototyping */
void *client_operation(void *args);

int main(int argc, char* argv[]) {

	if (argc != 3) {
		printf("Incorrect number or args: %s <port#> <#_of_strings>\n", argv[0]);
		exit(0);
	}

	/* Initiliazing theArray */
	int num_str = atoi(argv[2]);
	theArray = (char**) malloc(num_str*sizeof(char*));
	int i;

	/* Timing variables */
	pthread_mutex_init(&mutex, NULL);	

	/* Fill in the initial values for theArray */
	for (i = 0; i < num_str; i ++) {
		theArray[i] = malloc(STR_LEN * sizeof(char));
		sprintf(theArray[i], "String %d: the initial value", i);
		 
	}

	struct sockaddr_in sock_var;
	int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	int clientFileDescriptor;

	pthread_t *thread_handles;
	thread_handles =  malloc(thread_count*sizeof(pthread_t));
	pthread_mutex_init(&mutex, NULL);

	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port = strtol(argv[1],NULL,10);;
	sock_var.sin_family=AF_INET;
	
	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0) {

		listen(serverFileDescriptor,2000); 
		
		while(1) {
			//time start
			total = 0;
			for(i = 0; i < thread_count ; i++) {    //can support 1000 clients at a time

				clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
				pthread_create(&thread_handles[i], NULL, client_operation, (void *)clientFileDescriptor);

			}

		printf("socket has been created\n");

			for(i = 0; i < thread_count; i++) {
				pthread_join(thread_handles[i], NULL);
			}

			printf("Total Time: %f\n", total);
		}

		close(serverFileDescriptor);
	
	} else {
		printf("socket creation failed\n");
	}

	for(i = 0; i < num_str; i++){
		free(theArray[i]);
	}
	free(theArray);
	pthread_mutex_destroy(&mutex);
	free(thread_handles);

	return 0;
}

void *client_operation(void *args) {
	double start, end;

	int clientFileDescriptor = (int) args;

	char* temp;
	char str_ser[STR_LEN];

	char str_cli[STR_LEN];
	int n;
	
	GET_TIME(start);
	
	pthread_mutex_lock(&mutex); 

	n = read(clientFileDescriptor, str_cli, sizeof(str_cli));
	if (n < 0){
		printf("\nError Reading from Client");
	}
	
	char* token;
	token = strtok_r(str_cli, " ", &temp);

	int pos = atoi(str_cli);

	int r_w = atoi(strtok_r(NULL, " ", &temp));

	if(r_w == 1) {
		snprintf(str_ser,STR_LEN,"%s%d%s", "String ", pos, " has been modified by a write request");
		strcpy(theArray[pos],str_ser);
	} else {
		strcpy(str_ser, theArray[pos]);
	}	

	pthread_mutex_unlock(&mutex);

	GET_TIME(end);
	total += end - start;

	write(clientFileDescriptor, str_ser, STR_LEN);

	close(clientFileDescriptor);

	return NULL;
}
