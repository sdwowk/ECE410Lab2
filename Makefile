

CFLAGS = -lm -pthread -lpthread

server: server.c 
	gcc -g -Wall -o server server.c $(CFLAGS)

client: client.c
	gcc -g -Wall -o client client.c $(CFLAGS)

server_multi: server_multi.c
	gcc -g -Wall -o server_multi server_multi.c $(CFLAGS)


	


