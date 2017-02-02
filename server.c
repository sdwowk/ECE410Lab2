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
#define R 0
#define W 1

typedef struct {
	int ID; // arrayID
	int action; // R/W
} array_param;

pthread_mutex_t mutex;
int clientFileDescriptor;
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
	
	//char theArray[num_str][STR_LEN];
	char** theArray = malloc(num_str * sizeof(char *));
	int i;


	/* Fill in the initial values for theArray */
	for (i = 0; i < num_str; i ++) {
		//theArray[i] = malloc(STR_LEN * sizeof(char));
		theArray[i] = malloc(STR_LEN * sizeof(char));

		sprintf(theArray[i], "String %d: the initial value\n", i);
		fprintf(stderr, "%s", theArray[i]); 
	}

	struct sockaddr_in sock_var;
	int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	int clientFileDescriptor;

	pthread_t *thread_handles;
	thread_handles =  malloc(thread_count*sizeof(pthread_t));

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
				pthread_create(&thread_handles[i], NULL, client_operation, (void *)theArray);
			}
		}

		close(serverFileDescriptor);
	}
	else{
		printf("nsocket creation failed");
	}

	free(thread_handles);
	pthread_mutex_destroy(&mutex);
	return 0;
}

void *client_operation(void *args) {

	//char theArray[num_str][STR_LEN];
	//strncpy(theArray, args, num_str);

	char str_ser[STR_LEN];

	array_param id_rw;

	pthread_mutex_lock(&mutex); 

	read(clientFileDescriptor, &id_rw, sizeof(id_rw));

	if(id_rw.action == W) {
		sprintf(str_ser, "String %d has been modified by a write request\n", id_rw.ID);
		sprintf(((char **)args)[id_rw.ID], "String %d has been modified by a write request\n", id_rw.ID);
	} else {
		*str_ser = ((char **)args)[id_rw.ID];
	}
	printf("\nsending to client:%s\n", str_ser);
	write(clientFileDescriptor, str_ser, num_str);
	close(clientFileDescriptor);
	pthread_mutex_unlock(&mutex); 

}


