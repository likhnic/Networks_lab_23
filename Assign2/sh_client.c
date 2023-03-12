#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/poll.h>
#include <arpa/inet.h> 
#include <netinet/in.h> 
#define CLI_BUFF 10

void sendinChunks(int sockfd,char *res){

    char *buf;
    buf=(char *)malloc(sizeof(char)*CLI_BUFF);
    int len=0;
    for(int i=0;i<strlen(res)+1;++i){
        buf[len++]=res[i];
        if(len==CLI_BUFF){
            send(sockfd, buf, len, 0);
            len=0;
        }
    }
    if(len!=0){
        send(sockfd, buf, len, 0);
    }
    free(buf);
}

int recvinChunks(int sockfd, char* recieved){

    char *buf;
    buf=(char *)malloc(sizeof(char)*CLI_BUFF);
    int len=0;
    int recvSize;

    while((recvSize=recv(sockfd, buf, CLI_BUFF, 0))>0){
        for(int i=0;i<recvSize;++i){
            recieved[len++]=buf[i];
        }
        if(buf[recvSize-1]=='\0')break;
    }

    free(buf);
    return (recvSize<=0);
}

int main(){

    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd<0){
        printf("Socket creation failed\n");
        return -1;
    }

	serv_addr.sin_family = AF_INET;
	// serv_addr.sin_addr.s_addr = INADDR_ANY;
	inet_aton("127.0.0.1", &serv_addr.sin_addr); 
	serv_addr.sin_port = htons(20000);

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){
        printf("Connection failed\n");
        return -1;
    }

    char buf[CLI_BUFF];
    char recieved[201];
    // login is recieved in chunks as we don't know architecture of
    // send in server, like it can be sending in chunks, or atonce
    int closed = recvinChunks(sockfd, recieved);

    if(closed==1){
        close(sockfd);
        return 0;
    }

    char username[26];
    printf("%s", recieved);
    scanf("%s", username);
    scanf("%*c");

    sendinChunks(sockfd, username);
    closed = recvinChunks(sockfd, username);

    if(closed==1){
        close(sockfd);
        return 0;
    }

    if(strcmp(username, "NOT-FOUND")==0){
        printf("Invalid username\n");
        close(sockfd);
        return 0;
    }
    // printf("user %s\n", username);


    while(1){

        printf("Enter the shell command to be exceuted:$ ");
        scanf("%[^\n]s", recieved);
        scanf("%*c");
        if(strcmp(recieved, "exit")==0){
            break;
        }

        char *smallBuf = (char*)malloc(sizeof(char)*20);
        sendinChunks(sockfd, recieved);

        for(int i=0;i<20;++i)smallBuf[i]='\0';
        int recvSize;
        int len=0;
        for(int i=0;i<CLI_BUFF;++i)buf[i]='\0';

        // if recieved is #### print Error in running command
        // if recieved is $$$$ print Invalid command
        // else print all chars which are accumulated till len-5, as len=5 is used for check the 
        // above two commands, and later print every char individually
        while((recvSize=recv(sockfd, buf, CLI_BUFF, 0))>0){
            for(int i=0;i<recvSize;++i){
                smallBuf[len++]=buf[i];
                if(len==5 && smallBuf[0]=='#' && smallBuf[1]=='#' && smallBuf[2]=='#' && smallBuf[3]=='#' && smallBuf[4]=='\0'){
                    printf("Error in running command");
                    break;
                }
                else if(len==5 && smallBuf[0]=='$' && smallBuf[1]=='$' && smallBuf[2]=='$' && smallBuf[3]=='$' && smallBuf[4]=='\0'){
                    printf("Invalid command");
                    break;
                }
                else if(len==5)printf("%c%c%c%c%c",smallBuf[0],smallBuf[1],smallBuf[2],smallBuf[3],smallBuf[4]);
                else if(len>5) printf("%c",buf[i]);
                if(len==20){
                    len--;
                }
            }
            if(buf[recvSize-1]=='\0')break;
            for(int i=0;i<CLI_BUFF;++i)buf[i]='\0';
        }
        free(smallBuf);
        printf("\n");
        if(recvSize<=0){
            close(sockfd);
            return 0;
        }
    }

    close(sockfd);
    return 0;
}
