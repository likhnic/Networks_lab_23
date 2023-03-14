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

    if(my_connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        printf("Error in connecting"); 
        exit(1);
    }

    char *buf2 = strdup("Hello World! This is a test message!");
    my_send(sockfd, buf2, strlen(buf2)+1, 0);

    buf2 = strdup("Hello World! This is a test message!");
    my_send(sockfd, buf2, strlen(buf2)+1, 0);

    buf2 = strdup("Hello World! This is a test message!");
    my_send(sockfd, buf2, strlen(buf2)+1, 0);

    buf2 = strdup("Hello World! This is a test message!");
    my_send(sockfd, buf2, strlen(buf2)+1, 0);

    char buf[1000];
    int len = my_recv(sockfd, buf, 1000, 0);
    printf("%s\n", buf);

    my_recv(sockfd, buf, 1000, 0);
    printf("%s\n", buf);

    my_recv(sockfd, buf, 1000, 0);
    printf("%s\n", buf);

    my_recv(sockfd, buf, 1000, 0);
    printf("%s\n", buf);

    my_close(sockfd);
}