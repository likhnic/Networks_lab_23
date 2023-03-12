#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
	int sockfd ; //socket descriptor
	struct sockaddr_in	serv_addr;

	int i;
	char buf[100]; // will use this as buffer for communication

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Unable to create socket\n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);


	if ((connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr))) < 0) {
		printf("Unable to connect to server\n");
		return -1;
	}

    while(1){

        printf("Enter the expression: ");
        scanf("%[^\n]s",buf);
        scanf("%*c");

        // if string is -1 then break
        if(strcmp("-1",buf)==0){
            break;
        }
        
        // send the expression
        int sendSize = send(sockfd, buf, strlen(buf)+1, 0);

        for(i=0; i < 100; i++) buf[i] = '\0';
        recv(sockfd, buf, 100, 0);
        printf("Evaluated Answer: %s\n", buf);

    }
		
	close(sockfd);
	return 0;

}
