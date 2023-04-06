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

int checksum(void *b, int len)
{   
    unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;
 
    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main(int argc, char const *argv[]){

    if(argc == 1){
        printf("Please enter a hostname or IP address\n");
        exit(1);
    }

    struct hostent *host;
    host = gethostbyname(argv[1]);

    if(host == NULL){
        printf("Error: %s\n", hstrerror(h_errno));
        exit(1);
    }
    // get host ip address
    char *host_ip = strdup(inet_ntoa(*((struct in_addr *)host->h_addr_list[0])));

    // send icmp packet to host keeping intial ttl as 1 using raw sockets and increase it 
    // to get the routers to host
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if(sockfd < 0){
        printf("Error: %s\n", strerror(errno));
        exit(1);
    }

    struct sockaddr_in host_addr;
    host_addr.sin_family = host->h_addrtype;
    host_addr.sin_port = 0;
    host_addr.sin_addr.s_addr = *((unsigned long *)host->h_addr);  
    
    struct icmp *icmph;

    char buf[1024];
    memset(buf, 0, sizeof(buf));


    // set socket for a timeout of 1 second
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    int ttl = 1;

    while(1){
        
        // set ttl in socket
        setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        // set icmp header for time exceeded
        icmph = (struct icmp *)buf;
        icmph->icmp_type = ICMP_ECHO;
        icmph->icmp_code = 0;
        
        icmph->icmp_cksum = 0;
        
        icmph->icmp_cksum = checksum(icmph, sizeof(struct icmp));

        sendto(sockfd, buf, sizeof(struct icmp), 0, (struct sockaddr *)&host_addr, sizeof(host_addr));

        // receive icmp packet
        struct sockaddr_in from;
        int fromlen = sizeof(from);

        int n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);

        if(n < 0){
            perror("recvfrom");
            exit(1);
        }

        // get the router ip address
        char *router_ip = inet_ntoa(from.sin_addr);

        // print the router name and ip address
        printf("%s\n", router_ip);

        // check if the router is the host
        if(strcmp(router_ip, host_ip) == 0){
            break;
        }

        ttl++;
    }
    

}