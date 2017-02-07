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
#define thread_count 1000

pthread_mutex_t mutex;
char** theArray;
int num_str;

/* Prototyping */
void *client_operation(void *args);

int main(int argc, char* argv[]) {

	if (argc != 3) {
		printf("Incorrect number or args: %s <port#> <#_of_strings>\n", argv[0]);
		exit(0);
	}

	/* Initiliazing theArray */
	int num_str = atoi(argv[2]);
	theArray = malloc(num_str*sizeof(char));
	int i;

	char** theArray = malloc(num_str * sizeof(char *));
	pthread_mutex_init(&mutex, NULL);	

	/* Fill in the initial values for theArray */
	for (i = 0; i < num_str; i ++) {
		theArray[i] = malloc(STR_LEN * sizeof(char *));
		sprintf(theArray[i], "String %d: the initial value\n", i);
		 
	}

	printf("%s\n", theArray[325]);


	struct sockaddr_in sock_var;
	int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	int clientFileDescriptor;

	pthread_t *thread_handles;
	thread_handles =  malloc(thread_count*sizeof(pthread_t));
	pthread_mutex_init(&mutex, NULL);

	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port = strtol(argv[1],NULL,10);;
	sock_var.sin_family=AF_INET;
	
	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	{
		listen(serverFileDescriptor,2000); 
		while(1)        //loop infinity
		{
			for(i = 0; i < 1000 ; i++) {    //can support 1000 clients at a time
			
				clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
				printf("nConnected to client %d\n",clientFileDescriptor);
				pthread_create(&thread_handles[i], NULL, client_operation, (void *)clientFileDescriptor);
			}
		}

		close(serverFileDescriptor);
	}
	else{
		printf("nsocket creation failed");
	}

	pthread_mutex_destroy(&mutex);
	free(thread_handles);
	return 0;
}

void *client_operation(void *args) {

	int clientFileDescriptor = (int) args;

	char temp[STR_LEN];
	char* str_ser = malloc(STR_LEN * sizeof(char));

	char* str_cli[STR_LEN];
	perror("Pre read");
	read(clientFileDescriptor, str_cli, sizeof(str_cli));
	perror("Post read");

	printf("%s\n", str_cli);

	char* token = malloc(STR_LEN * sizeof(char));
	token = strtok_r(str_cli, " ", temp);

	int pos = atoi(str_cli);

	int r_w = atoi(strtok_r(NULL, " ", temp));

	printf("%d\n", pos);
	printf("%d\n", r_w);

	pthread_mutex_lock(&mutex); 

	if(r_w == 1) {
		sprintf(str_ser, "String %d has been modified by a write request", pos);
		sprintf(theArray[pos], "String %d has been modified by a write request", pos);
	} else {
	printf("%d\n", pos);

		str_ser = theArray[pos];
	printf("%s\n", theArray[pos]);

	}	

	printf("\nsending to client:%s\n", str_ser);
	write(clientFileDescriptor, str_ser, STR_LEN);

	pthread_mutex_unlock(&mutex);

	close(clientFileDescriptor);
}
