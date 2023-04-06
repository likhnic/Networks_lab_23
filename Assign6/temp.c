#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

int main(){

    struct hostent *host;
    char buf[255] = "www.google.com";

    host = gethostbyname(buf);
    if(host == NULL){
        printf("Error: %s\n", hstrerror(h_errno));
        exit(1);
    }

    printf("IP address of %s is %s, %d\n", host->h_name, inet_ntoa(*((struct in_addr *)host->h_addr_list[0])), host->h_length);

}