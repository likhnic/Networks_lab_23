#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#define SERV_BUFFER 2


// function to send in chunks
void sendinChunks(int sockfd,char *res){

    char *buf;
    buf=(char *)malloc(sizeof(char)*SERV_BUFFER);
    int len=0;
    for(int i=0;i<strlen(res)+1;++i){
        buf[len++]=res[i];
        if(len==SERV_BUFFER){
            send(sockfd, buf, len, 0);
            len=0;
        }
    }
    if(len!=0){
        send(sockfd, buf, len, 0);
    }
    free(buf);
}

// recieved in chunks, returns 1 if client is closed or any error
int recvinChunks(int sockfd, char* recieved){

    char *buf;
    buf=(char *)malloc(sizeof(char)*SERV_BUFFER);
    int len=0;
    int recvSize;

    while((recvSize=recv(sockfd, buf, SERV_BUFFER, 0))>0){
        for(int i=0;i<recvSize;++i){
            recieved[len++]=buf[i];
        }
        if(buf[recvSize-1]=='\0')break;
    }
    free(buf);
    return (recvSize<=0);
}

// sends the command output in chunks
// cd, dir can have atmost 1 argument, else #### is sent
// pwd can't have any arguments else #### is sent
void exceute_command(char *cmd, int sockfd){

    int n=strlen(cmd);
    char *res = (char *)malloc(sizeof(char)*1000);
    char *token = (char *)malloc(sizeof(char)*1000);

    int lastind = -1;
    int len=0;
    for(int i=0;i<n;++i){
        if(lastind==-1){
            if(cmd[i]!=' '){
                token[len++]=cmd[i];
                lastind=i;
            }
        }
        else{
            if(cmd[i]!=' '){
                token[len++]=cmd[i];
                lastind=i;
            }
            else{
                break;
            }
        }
    }
    token[len]='\0';
    res[0]='\0';

    if(strcmp(token, "pwd")==0){
        res = strdup(getcwd(token, 1000));
        for(int i=lastind+1;i<n;++i){
            if(cmd[i]!=' '){
                res=strdup("####");
            }
        }
        if(res==NULL){
            res = strdup("####");
        }
    }
    else if(strcmp(token, "dir")==0){

        token = strdup(".");
        for(int i=lastind+1;i<n;++i){
            if(cmd[i]!=' '){
                token=strdup(cmd+i);
                break;
            }       
        }

        for(int i=strlen(token)-1;i>=0;--i){
            if(token[i]!=' '){
                token[i+1]='\0';
                break;
            }
        }

        if(token[0]=='\0'){
            token=strdup(".");
        }
        struct dirent *de;
        DIR *dr=opendir(token);

        if(dr==NULL){
            res = strdup("####");
        }
        else{
            char buff[SERV_BUFFER];
            int len=0;
            while((de=readdir(dr))!=NULL){
                for(int i=0;i<strlen(de->d_name)+1;++i){
                    buff[len++]=(de->d_name[i]=='\0'?'\t':de->d_name[i]);
                    if(len==SERV_BUFFER){
                        send(sockfd, buff, len,0);
                        len=0;
                    }
                }
            }
            if(len<=SERV_BUFFER-1){
                buff[len]='\0';
                send(sockfd, buff, len+1, 0);
            }
            else{
                send(sockfd, buff, len, 0);
                buff[0]='\0';
                send(sockfd, buff, 1, 0);
            }
            closedir(dr);
            return;
        }
    }
    else if(strcmp(token, "cd")==0){

        token = strdup(getenv("HOME"));
        for(int i=lastind+1;i<n;++i){
            if(cmd[i]!=' '){
                token=strdup(cmd+i);
                break;
            }       
        }

        for(int i=strlen(token)-1;i>=0;--i){
            if(token[i]!=' '){
                token[i+1]='\0';
                break;
            }
        }

        if(token[0]=='\0'){
            token = strdup(getenv("HOME"));
        }

        int ans = chdir(token);
        if(ans==-1){
            res = strdup("####");
        }
    }
    else{
        res=strdup("$$$$");
    }
    sendinChunks(sockfd, res);
    free(token);
    return;
}

int main()
{
	int sockfd, newsockfd ; 
	unsigned int clilen;
	struct sockaddr_in	cli_addr, serv_addr;


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(20000);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		return -1;
	}

	listen(sockfd, 5); 

	while (1) {

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,&clilen) ;

		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}

		if (fork() == 0) {

			close(sockfd);	
			
            char recieved[1000];
			strcpy(recieved,"LOGIN: ");
            // sent in chunks
            sendinChunks(newsockfd, recieved);

            int closed = recvinChunks(newsockfd, recieved);

            if(closed==1){
                close(newsockfd);
                break;
            }

            FILE *fp;
            fp=fopen("./users.txt", "r");

            char line[1000];
            int found=0;
            while(fgets(line, 1000, fp)!=NULL){
                line[strlen(line)-1]='\0';
                if(strcmp(line, recieved)==0){
                    found=1;
                    break;
                }
            }
            fclose(fp);

            if(found){
                strcpy(recieved, "FOUND");
                sendinChunks(newsockfd, recieved);
            }
            else{
                strcpy(recieved, "NOT-FOUND");
                sendinChunks(newsockfd, recieved);
            }

            found=0;
            while(1){
                closed = recvinChunks(newsockfd, recieved);
                if(closed==1){
                    found=1;
                    close(newsockfd);
                    break;
                }
                exceute_command(recieved, newsockfd);
            }
            if(found)break;

			close(newsockfd);
			exit(0);
		}

		close(newsockfd);
	}
	return 0;
}
			
