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

char** theArray;
int num_str;
double total;

typedef struct {
    int readers;
    int writer;
    pthread_cond_t readers_proceed;
    pthread_cond_t writer_proceed;
    int pending_writers;
    pthread_mutex_t read_write_lock;
} mylib_rwlock_t;

void mylib_rwlock_init (mylib_rwlock_t *l) {
    l -> readers = l -> writer = l -> pending_writers = 0;
    pthread_mutex_init(&(l -> read_write_lock), NULL);
    pthread_cond_init(&(l -> readers_proceed), NULL);
    pthread_cond_init(&(l -> writer_proceed), NULL);
}

void mylib_rwlock_rlock(mylib_rwlock_t *l) {
    /* if there is a write lock or pending writers, perform
     * condition wait, else increment count of readers and grant
     * read lock */
    pthread_mutex_lock(&(l -> read_write_lock));
    while ((l -> pending_writers > 0) || (l -> writer > 0))
        pthread_cond_wait(&(l -> readers_proceed),
                &(l -> read_write_lock));
    l -> readers ++;
    pthread_mutex_unlock(&(l -> read_write_lock));
}

void mylib_rwlock_wlock(mylib_rwlock_t *l) {
    /* if there are readers or writers, increment pending
     * writers count and wait. On being woken, decrement pending
     * writers count and increment writer count */
    pthread_mutex_lock(&(l -> read_write_lock));
    while ((l -> writer > 0) || (l -> readers > 0)) {
        l -> pending_writers ++;
        pthread_cond_wait(&(l -> writer_proceed),
                &(l -> read_write_lock));
         l -> pending_writers --;
    }
    l -> writer ++;
    pthread_mutex_unlock(&(l -> read_write_lock));
}

void mylib_rwlock_unlock(mylib_rwlock_t *l) {
    /* if there is a write lock then unlock, else if there
     * are read locks, decrement count of read locks. If the count
     * is 0 and there is a pending writer, let it through, else if
     * there are pending readers, let them all go through */
    pthread_mutex_lock(&(l -> read_write_lock));
    if (l -> writer > 0)
        l -> writer = 0;
    else if (l -> readers > 0)
        l -> readers --;
    pthread_mutex_unlock(&(l -> read_write_lock));
    if ((l -> readers == 0) && (l -> pending_writers > 0))
        pthread_cond_signal(&(l -> writer_proceed));
    else if (l -> readers > 0)
        pthread_cond_broadcast(&(l -> readers_proceed));
}

mylib_rwlock_t *read_write_locks;

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
	read_write_locks = (mylib_rwlock_t *) malloc(sizeof(mylib_rwlock_t) * num_str);

	/* Fill in the initial values for theArray */
	for (i = 0; i < num_str; i ++) {
		theArray[i] = malloc(STR_LEN * sizeof(char));
		sprintf(theArray[i], "String %d: the initial value", i);
		mylib_rwlock_init(&read_write_locks[i]);		 
	}

	struct sockaddr_in sock_var;
	int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	int clientFileDescriptor;

	pthread_t *thread_handles;
	thread_handles =  malloc(thread_count*sizeof(pthread_t));

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
	free(thread_handles);
	free(read_write_locks);

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

	n = read(clientFileDescriptor, str_cli, sizeof(str_cli));
	if (n < 0){
		printf("\nError Reading from Client");
	}
	
	char* token;
	token = strtok_r(str_cli, " ", &temp);

	int pos = atoi(str_cli);

	int r_w = atoi(strtok_r(NULL, " ", &temp));

	if(r_w == 1) {
		mylib_rwlock_wlock(&read_write_locks[pos]);
		snprintf(str_ser,STR_LEN,"%s%d%s", "String ", pos, " has been modified by a write request");
		strcpy(theArray[pos],str_ser);
		mylib_rwlock_unlock(&read_write_locks[pos]);
		write(clientFileDescriptor, str_ser, STR_LEN);

	} else {
		mylib_rwlock_rlock(&read_write_locks[pos]);
		strcpy(str_ser, theArray[pos]);
		mylib_rwlock_unlock(&read_write_locks[pos]);
		write(clientFileDescriptor, str_ser, STR_LEN);

	}	

	GET_TIME(end);
	total += end - start;


	close(clientFileDescriptor);

	return NULL;
}
