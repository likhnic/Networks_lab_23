library: mysocket.o
	ar rcs libmsocket.a mysocket.o

mysocket.o: mysocket.c

server: server.c
	gcc -Wall -o server server.c -L. -lmsocket

client: client.c
	gcc -Wall -o client client.c -L. -lmsocket

remove:
	rm -f server client mysocket.o

clean:
	rm -f mysocket.o libmsocket.a server client