

CFLAGS = -lm -pthread -lpthread

server: server.c 
	gcc -g -Wall -o server server.c $(CFLAGS)

client: client.c
	gcc -g -Wall -o client client.c $(CFLAGS)



	


