#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <poll.h>
#define BUFFS 3  // can be anything

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

// sends Send Load to server to get load
int getLoad(struct sockaddr_in serv_addr, char *ip, char *port){
    char buff[100];
    int ssockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ((connect(ssockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr))) < 0) {
        printf("Unable to connect to server\n");
        return -1;
    }
    strcpy(buff,"Send Load");
    int sendSize = send(ssockfd, buff, 100, 0);
    int closed = recvinChunks(ssockfd, buff);
    if(closed){
        close(ssockfd);
        return -1;
    }
    printf("Load Received from %s:%s %s\n",ip,port, buff);
    close(ssockfd);
    return atoi(buff);
}

int main(int argv,char *argc[]){


    int sockfd;
    struct sockaddr_in serv1_aadr, serv2_addr;
    struct sockaddr_in cli_addr,serv_addr;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){
        printf("Cannot create socket\n");
        return -1;
    }

    char ip[20]="127.0.0.1";


    // for lb
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argc[1]));

    // for serv1
    serv1_aadr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv1_aadr.sin_addr);
    serv1_aadr.sin_port = htons(atoi(argc[2]));

    // for serv2
    serv2_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv2_addr.sin_addr);
    serv2_addr.sin_port = htons(atoi(argc[3]));

    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("Cannot bind\n");
        return -1;
    }

    int server_load[2]={0,0};
    int currTimeOut=5000;

    struct pollfd fd_set[1];
    memset(fd_set, 0, sizeof(fd_set));
    fd_set[0].fd = sockfd;
    fd_set[0].events = POLLIN;
    
    listen(sockfd, 5);

    while(1){

        int newsockfd;
        unsigned int clilen;
        clilen = sizeof(cli_addr);

        time_t beginTime = time(NULL);
        // call timeout for currTimeOut time, which is 5s and reduces if
        // client sends a connect request
        int ret = poll(fd_set, 1, currTimeOut);
        if(ret<=0){
            currTimeOut = 5000;

            server_load[0]=getLoad(serv1_aadr, ip, argc[2]);
            if(server_load[0]==-1){
                close(newsockfd);
                return 0;
            }

            server_load[1]=getLoad(serv2_addr, ip, argc[3]);
            if(server_load[1]==-1){
                close(newsockfd);
                return 0;
            }
            
        }
        if(fd_set[0].revents==POLLIN){

            newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
            if(newsockfd<0){
                printf("Cannot accept\n");
                return -1;
            }
            if(fork()==0){
                close(sockfd);
                char buff[100];
                buff[0]='\0';

                struct sockaddr_in servc_addr;
                int port=2;

                if(server_load[0]<server_load[1]){
                    servc_addr = serv1_aadr;
                }
                else{
                    servc_addr = serv2_addr;
                    port=3;
                }

                // sends Send Time to appropriate server and gets back time
                strcpy(buff, "Send Time");
                int ssockfd = socket(AF_INET, SOCK_STREAM, 0);
                if ((connect(ssockfd, (struct sockaddr *) &servc_addr,sizeof(servc_addr))) < 0) {
                    printf("Unable to connect to server\n");
                    return -1;
                }
                printf("Sending Client request to %s:%s\n",ip,argc[port]);
                int sendSize = send(sockfd, buff, 100, 0);
                int closed = recvinChunks(ssockfd, buff);
                if(closed){
                    close(ssockfd);
                    close(newsockfd);
                    return 0;
                }
                close(ssockfd);
                send(newsockfd, buff, 100, 0);
                close(newsockfd);
                return 0;
            }
            close(newsockfd);
            time_t endTime=time(NULL);
            currTimeOut = currTimeOut-(endTime-beginTime)*1000;
            if(currTimeOut<0)currTimeOut=5000;
        }
    }

    close(sockfd);
    return 0;
}