#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#define BUFFS 5   // can be anything


int recvinChunks(int sockfd, char* recieved){

    char *buf;
    buf=(char *)malloc(sizeof(char)*BUFFS);
    int len=0;
    int recvSize;

    while((recvSize=recv(sockfd, buf, BUFFS, 0))>0){
        for(int i=0;i<recvSize;++i){
            recieved[len++]=buf[i];
        }
        if(buf[recvSize-1]=='\0')break;
    }
    free(buf);
    return (recvSize<=0);
}

int main(int argv, char *argc[]){

    srand(atoi(argc[1]));

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
    serv_addr.sin_port = htons(atoi(argc[1]));

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

        int closed = recvinChunks(newsockfd, buff);
        if(closed){
            close(newsockfd);
            return 0;
        }

        // compares buff and sends appropriate buffer
        if(strcmp(buff, "Send Load")==0){
            sprintf(buff, "%d", rand() % 100 + 1);
            send(newsockfd, buff, strlen(buff)+1, 0);
            printf("Load sent %s\n", buff);
        }
        else if(strcmp(buff, "Send Time")==0){
            sprintf(buff, "%02d-%02d-%4d, %02d:%02d:%02d", local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec);
            send(newsockfd, buff, strlen(buff)+1, 0);
        }
        close(newsockfd);
    }

    return 0;
}