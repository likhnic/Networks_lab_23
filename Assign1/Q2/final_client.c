#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUF_SIZE 3

int main(){


	int sockfd ; //socket descriptor
	struct sockaddr_in	serv_addr;

	int i;
	char buf[BUF_SIZE]; // will use this as buffer for communication

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
        int endline=0;
        int loop=0;
        // loop for getting new expression
        while(1){
            int count=0;
            for(int i=0;i<BUF_SIZE;++i)buf[i]='\0';
            // take in the stream of size BUF_SIZE and send to server
            while(1){
                char c;
                scanf("%c",&c);
                // if new line is encountered then break and ask for a new expression 
                // after sending current expression
                if(c=='\n'){
                    buf[count++]='\0';
                    endline=1;
                    break;
                }
                buf[count++]=c;
                if(count==BUF_SIZE)break;
            }
            // loop will take care of -1, as when user just enters -1 we need to 
            // stop asking for an expression
            loop++;
            if(strcmp(buf,"-1")==0 && loop==1){
                loop=-1;
                break;
            }
            int sendSize = send(sockfd,buf,BUF_SIZE,0);
            if(endline)break;
        }
        if(loop==-1)break;
        float answer;
        // recieve answer from server
        int recvSize = recv(sockfd,(char*)&answer,sizeof(answer),0);
        printf("Evaluated answer: %f\n",answer);

    }
		
	close(sockfd);
	return 0;

}