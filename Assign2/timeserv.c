#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <time.h>
#include <netinet/in.h> 
  
#define MAXLINE 1024 
  
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr, cliaddr; 
    char buf[100];
      
    // Create socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        printf("socket creation failed"); 
        return -1;
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(8181); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  sizeof(servaddr)) < 0 ) 
    { 
        printf("bind failed"); 
        return -1;
    } 
    
    printf("\nServer Running....\n");
    while(1){

        time_t curr=time(NULL);
        struct tm* local=localtime(&curr);

        socklen_t len=sizeof(cliaddr);
        recvfrom(sockfd, buf, 100, 0, (struct sockaddr *)&cliaddr, &len);
        sprintf(buf, "%02d-%02d-%4d, %02d:%02d:%02d", local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec);
        sendto(sockfd, buf, strlen(buf)+1, 0, (struct sockaddr *)&cliaddr, len);
    }
    close(sockfd);
    return 0; 
}
