#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <fcntl.h>

int main(int argc, const char* argv[]){

    if(argc <= 3){
        printf("Please provide all fields\n");
        exit(1);
    }

    struct hostent *host;
    host = gethostbyname(argv[1]);

    int n = atoi(argv[2]);

    int T = atoi(argv[3]);

    if(host == NULL){
        printf("Error: %s\n", hstrerror(h_errno));
        exit(1);
    }

    // get host ip address
    char *host_ip = strdup(inet_ntoa(*((struct in_addr *)host->h_addr_list[0])));

    

}