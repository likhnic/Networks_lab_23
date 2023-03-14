#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mysocket.h"


int main(){

    int sockfd = my_socket(AF_INET, SOCK_MyTCP, 0);
    if(sockfd == -1){
        printf("Error in creating socket"); 
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(my_bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        printf("Error in binding socket"); 
        exit(1);
    }

    if(my_listen(sockfd, 10) == -1){
        printf("Error in listening"); 
        exit(1);
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int newsocfd = my_accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);

    if(newsocfd == -1){
        printf("Error in accepting"); 
        exit(1);
    }

    char buf[1000];
    int recvSize = my_recv(newsocfd, buf, 1000, 0);
    printf("%s\n", buf);

    my_recv(newsocfd, buf, 1000, 0);
    printf("%s\n", buf);

    my_recv(newsocfd, buf, 1000, 0);
    printf("%s\n", buf);

    my_recv(newsocfd, buf, 1000, 0);
    printf("%s\n", buf);

    char *buf2 = strdup("Hello World! This is a test message!");
    my_send(newsocfd, buf2, strlen(buf2)+1, 0);

    my_send(newsocfd, buf2, strlen(buf2)+1, 0);

    my_send(newsocfd, buf2, strlen(buf2)+1, 0);

    my_send(newsocfd, buf2, strlen(buf2)+1, 0);
    

    my_close(sockfd);

}