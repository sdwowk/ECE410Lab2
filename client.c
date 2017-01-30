#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define STR_LEN 1000

int NUM_REQUESTS;
void *Operate(void* clientFileDescriptor);

int main(int argc, char* argv[]) {

	struct sockaddr_in sock_var;
	int clientFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	pthread_t t[strtol(argv[2],NULL,10)];


	if(argc != 3){
		perror("incorrect number of args");
	}

	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port=strtol(argv[1],NULL,10);
	sock_var.sin_family=AF_INET;
	char write_str[1000];

	NUM_REQUESTS = strtol(argv[2],NULL,10);
	while(NUM_REQUESTS > 0){
		if(connect(clientFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0){
			printf("Connected to server %dn",clientFileDescriptor);
			pthread_create(&t,NULL,Operate,(void *)clientFileDescriptor);
			
		}
		else{
			printf("\nsocket creation failed\n");
		}
	}
	return 0;
}

void* Operate(void* clientFileDescriptor){
	char str_ser[STR_LEN];
	write(clientFileDescriptor, "", STR_LEN);
	read(clientFileDescriptor,str_ser,20);
	printf("String from Server: %s",str_ser);

	close(clientFileDescriptor);
	NUM_REQUESTS--;

}