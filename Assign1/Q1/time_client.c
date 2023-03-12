#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>


int main(){

    int sockfd;
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){
        printf("Cannot create socket\n");
        return -1;
    } 

    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(20000);

	if ((connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr))) < 0) {
		printf("Unable to connect to server\n");
		return -1;
	}

    char buf[100];
	for(int i=0; i < 100; i++) buf[i] = '\0';
	recv(sockfd, buf, 100, 0);

    printf("Server local date and time: %s\n", buf);

    return 0;
}