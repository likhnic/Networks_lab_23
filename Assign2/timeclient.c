#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/poll.h>
#include <arpa/inet.h> 
#include <netinet/in.h> 
  
int main() { 
    int sockfd; 
    struct sockaddr_in serv_addr; 
    struct pollfd fd_set[1];
  
    // Creating socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        printf("socket creation failed"); 
        return -1;
    } 
  
    memset(&serv_addr, 0, sizeof(serv_addr)); 
      
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(8181); 
    inet_aton("127.0.0.1", &serv_addr.sin_addr); 
    
    int n;
    socklen_t len; 
    len=sizeof(serv_addr);
    char currTime[100];

    for(int i=0;i<100;++i)currTime[i]='\0';

    memset(fd_set, 0, sizeof(fd_set));

    fd_set[0].fd = sockfd;
    fd_set[0].events = POLLIN;

    int timeout=3000;
    int loop=0;
    int recieved=0;
    do{
        // sending address to server
        int sendSize = sendto(sockfd, currTime, 100, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        printf("Polling...\n");
        n = poll(fd_set, 1, timeout);
        loop++;
        if(n<=0){
            continue;
        }
        else if(fd_set[0].revents==POLLIN){
            int recvSize = recvfrom(sockfd, currTime, 100, 0, (struct sockaddr *)&serv_addr, &len);      
            recieved=1;          
            break;
        }
    }while(loop<5);

    if(recieved==0){
        printf("Timeout exceeded\n");
        return 0;
    }
    printf("Date and Time is %s\n", currTime);

    close(sockfd); 
    return 0; 
} 
