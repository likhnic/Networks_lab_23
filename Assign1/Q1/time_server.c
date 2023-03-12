#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>


int main(){

    int sockfd, newsockfd;
    unsigned int clilen;
    struct sockaddr_in	cli_addr, serv_addr;
    struct tm* local;
    char buff[100];

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){
        printf("Cannot create socket\n");
        return -1;
    } 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);

    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("Cannot bind\n");
        return -1;
    }

    listen(sockfd, 5);

    while(1){

        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

        if(newsockfd < 0){
            printf("Accept error\n");
            return -1;
        }
        
        time_t curr=time(NULL);
        local=localtime(&curr);

        sprintf(buff, "%02d-%02d-%4d, %02d:%02d:%02d", local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec);
        send(newsockfd, buff, strlen(buff)+1, 0);

        close(newsockfd);
    }

    return 0;
}