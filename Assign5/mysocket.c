#include "mysocket.h"

pthread_t R, S;

int send_count=0;
int recv_count=0;

int recv_in=0;
int send_in=0;

int recv_out=0;
int send_out=0;

message **send_buffer;
message **recv_buffer;

pthread_mutex_t recvMutex, sendMutex, tcpLock = PTHREAD_MUTEX_INITIALIZER;

int MyTCP = -1;

int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    int ret = bind(sockfd, addr, addrlen);
    return ret;
}

int my_listen(int sockfd, int backlog){
    int ret = listen(sockfd, backlog);
    return ret;
}

int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){

    int newsockfd = accept(sockfd, addr, addrlen);
    if(newsockfd == -1){
        return -1;
    }
    pthread_mutex_lock(&tcpLock);
    MyTCP = newsockfd;
    pthread_mutex_unlock(&tcpLock);

    return MyTCP;
}

int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    int done = connect(sockfd, addr, addrlen);

    if(done == -1){
        return -1;
    }
    pthread_mutex_lock(&tcpLock);
    MyTCP = sockfd;
    pthread_mutex_unlock(&tcpLock);

    return MyTCP;
}


void *recvThread(void *arg){

    while(1){
        pthread_mutex_lock(&tcpLock);
        while(MyTCP == -1){
            pthread_mutex_unlock(&tcpLock);
            sleep(1);
            pthread_mutex_lock(&tcpLock);
        }
        pthread_mutex_unlock(&tcpLock);

        // recieve the first 4 bytes which is the len of the message
        int len;
        recv(MyTCP, &len, sizeof(int), 0);

        // make a buffer of that size and recieve the actual message

        char *buf = (char *)malloc(len);
        int i=0;
        while(i<len){
            int size = (len-i>1000)?1000:(len-i);
            recv(MyTCP, buf+i, size, 0);
            i+=size;
        }

        // store the message in the recv_buffer
        pthread_mutex_lock(&recvMutex);
        while(recv_count==MAX_MESSAGE_TABLE_SIZE){
            pthread_mutex_unlock(&recvMutex);
            sleep(1);
            pthread_mutex_lock(&recvMutex);
        }
        pthread_mutex_unlock(&recvMutex);

        // first copy it to message struct
        message *msg = (message *)malloc(sizeof(message));
        msg->buf = buf;
        msg->len = len;
        msg->flags = 0;

        // then copy it to the recv_buffer
        pthread_mutex_lock(&recvMutex);
        recv_buffer[recv_in] = msg;
        recv_in = (recv_in+1)%MAX_MESSAGE_TABLE_SIZE;
        recv_count++;
        pthread_mutex_unlock(&recvMutex);
    }

    return NULL;
}


void* sendThread(void *arg){

    while(1){
        pthread_mutex_lock(&tcpLock);
        while(MyTCP==-1){
            pthread_mutex_unlock(&tcpLock);
            sleep(1);
            pthread_mutex_lock(&tcpLock);
        }
        pthread_mutex_unlock(&tcpLock);

        pthread_mutex_lock(&tcpLock);
        while(send_count==0){
            pthread_mutex_unlock(&tcpLock);
            sleep(1);
            pthread_mutex_lock(&tcpLock);
        }
        pthread_mutex_unlock(&tcpLock);
        
        // lock mutex to access send_buffer
        pthread_mutex_lock(&sendMutex);
        message *msg = send_buffer[send_out];
        send_out = (send_out+1)%MAX_MESSAGE_TABLE_SIZE;
        send_count--;
        pthread_mutex_unlock(&sendMutex);

        // send fitrst 4 bytes the len of the message
        int len = msg->len;
        send(MyTCP, &len, sizeof(int), 0);
        // then send the actual buf, in multiple calls of size<=1000 bytes
        int i=0;
        while(i<len){
            int size = (len-i>1000)?1000:(len-i);
            send(MyTCP, msg->buf+i, size, 0);
            i+=size;
        }

        // free the message
        free(msg->buf);
        free(msg);
    }

    return NULL;
}

int my_socket(int domain, int type, int protocol){

    // if the type is not SOCK_MyTCP, return -1
    if(type!=SOCK_MyTCP)return -1;

    int sockfd = socket(domain, SOCK_STREAM, protocol);
    // if error while creating socket, return -1
    if(sockfd<0)return -1;

    // initializing the send and recv buffers
    send_buffer = (message **)malloc(sizeof(message *)*MAX_MESSAGE_TABLE_SIZE);
    recv_buffer = (message **)malloc(sizeof(message *)*MAX_MESSAGE_TABLE_SIZE);

    recv_count = 0;
    send_count = 0;

    // initializing the mutexes
    pthread_mutex_init(&recvMutex, NULL);
    pthread_mutex_init(&sendMutex, NULL);

    // creating two threads R and S to recieve and send respectively
    pthread_create(&R, NULL, recvThread, NULL);
    pthread_create(&S, NULL, sendThread, NULL);


    return sockfd;    

}

ssize_t my_send(int sockfd, void *buf, size_t len, int flags){

    // store buf in local variable, so as to decrease time in lock
    message *msg = (message *)malloc(sizeof(message));
    msg->buf = (void *)malloc(len);
    memcpy(msg->buf, buf, len);
    msg->len = len;
    msg->flags = flags;

    // if send buffer is full wait till it becomes empty and then add the message
    pthread_mutex_lock(&sendMutex);
    while(send_count==MAX_MESSAGE_TABLE_SIZE){
        pthread_mutex_unlock(&sendMutex);
        sleep(1);
        pthread_mutex_lock(&sendMutex);
    }
    pthread_mutex_unlock(&sendMutex);

    pthread_mutex_lock(&sendMutex);
    send_buffer[send_in] = msg;
    send_in = (send_in+1)%MAX_MESSAGE_TABLE_SIZE;
    send_count++;
    pthread_mutex_unlock(&sendMutex);
    
    return len;
}

ssize_t my_recv(int sockfd, void *buf, size_t len, int flags){

    // block until there is something to recieve
    // lock mutex to access recv_buffer
    pthread_mutex_lock(&recvMutex);
    while(recv_count==0){
        pthread_mutex_unlock(&recvMutex);
        sleep(1);
        pthread_mutex_lock(&recvMutex);
    }
    pthread_mutex_unlock(&recvMutex);

    // copy the message from the recv_buffer to buf
    pthread_mutex_lock(&recvMutex);
    message *msg = recv_buffer[recv_out];
    recv_out = (recv_out+1)%MAX_MESSAGE_TABLE_SIZE;
    recv_count--;
    pthread_mutex_unlock(&recvMutex);


    memcpy(buf, msg->buf, msg->len);
    free(msg->buf);
    free(msg);

    return len;
}



int my_close(int fd){

    // close the socket
    sleep(5);

    // async cancel the threads
    pthread_cancel(R);
    pthread_cancel(S);
    close(fd);

    return 0;

}