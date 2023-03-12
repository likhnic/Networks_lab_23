#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUF_SIZE 5   // change this to change server buffer


// This code assumes valid expression, only one paranthesis, which was told to assume
// The commented code at bottom will work even for multiple paranthesis
typedef struct {
    char *data; 
    int capacity; 
    int size; 
} String;


void init(String *s) {
    s->capacity = 1; 
    s->size = 0; 
    s->data = malloc(s->capacity); 
}

void push_back(String *s, char c) {
    if (s->size == s->capacity) {
        s->capacity*=2;
        s->data = realloc(s->data,s->capacity);
    }
    s->data[s->size] = c; 
    s->size++; 
}


float getOpAns(float x,float y, char c){

    if(c=='+')return x+y;
    if(c=='-')return x-y;
    if(c=='*')return x*y;
    if(c=='/')return x/y;
    return x;
}


float evaluateExpression(char *exp, int len){

    float prevNumber = 0;
    char prevOp = '+';

    float outerNum = 0;
    char outerOp = '+';

    for(int i=0;i<len;++i){

        char c = exp[i];
        // checking if the current character is a digit, or - preceeded with some operator
        // in that case we can scan the string till we get next operator
        // add the number in floatstack
        if((i==0 && c=='-') || (i && (exp[i-1]<'0' || exp[i-1]>'9') && exp[i-1]!=')' && c=='-') || (c<='9' && c>='0') || c=='.'){
            char num[len+1];
            int numsize=0;
            int sign=1;
            if(c=='-')sign=-1;

            for(int j=i+(sign==-1);;++j){
                if(j==len){
                    i=len-1;
                    break;
                }
                if(exp[j]=='+' || exp[j]=='-' || exp[j]=='*' || exp[j]=='/' || exp[j]==')'){
                    i=j-1;
                    break;
                }
                num[numsize++]=exp[j];
            }
            num[numsize]='\0';
            float curr=sign*atof(num);
            prevNumber = getOpAns(prevNumber, curr, prevOp);
        }
        // add ( to operator_stack
        else if(c=='('){
            outerNum = prevNumber;
            outerOp = prevOp;
            prevOp='+';
            prevNumber=0;
        }
        // pop top 2 elements of operand_stack and top element of operator_stack 
        // and do operation and push to operand_stack
        // do this till we encounter (
        else if(c==')'){
            prevNumber = getOpAns(outerNum, prevNumber, outerOp);
        }
        // pop top 2 elements of operand_stack and top element of operator_stack 
        // and do operation and push to operand_stack
        // do this till we encounter ( or the operator_stack is empty
        else if(c=='+' || c=='-' || c=='*' || c=='/'){
            prevOp = c;
        }

    }
    return prevNumber;
}

int main()
{
	int sockfd, newsockfd ; //socket descriptors
	unsigned int clilen;
	struct sockaddr_in	cli_addr, serv_addr;	

    char userExp[BUF_SIZE];	// will use this as buffer for communication, taking it to be BUF_SIZE, 
                        // since typical lengths will be less than this
    String buf; // we will use this to store whole string 

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(20000);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		printf("bind to local address failed\n");
		return -1;
	}

	listen(sockfd, 5); 

	while (1) {

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,&clilen) ;

		if (newsockfd < 0) {
			printf("Accept error\n");
			return -1;
		}

        while(1){
            // recieve till socket is closed in client
            int recvSize;
            // if last char is '\0' of recieved part then break
            // since it means we already got complete string

            char prevop='+';
            float prevNum=0;
            char outerop='+';
            float outerNum=0;
            int startnum=0;
            float carrynum=0;
            int decimal=0;
            long long int divider=10;
            while((recvSize = recv(newsockfd, userExp, BUF_SIZE, 0))>0){
                printf("hello %s\n", userExp);
                for(int i=0;i<recvSize;++i){
                    if(userExp[i]==')'){
                        prevNum = getOpAns(prevNum, carrynum, prevop);
                        prevNum = getOpAns(outerNum, prevNum, outerop);
                        startnum=0;
                        decimal=0;
                        divider=10;
                        carrynum=0;
                    }
                    if(userExp[i]=='('){
                        outerNum=prevNum;
                        outerop=prevop;
                        prevop='+';
                        prevNum=0;
                    }
                    if(userExp[i]=='+' || userExp[i]=='-' || userExp[i]=='*' || userExp[i]=='/'){
                        if(startnum==1)prevNum = getOpAns(prevNum, carrynum, prevop);
                        carrynum=0;
                        divider=10;
                        decimal=0;
                        prevop=userExp[i];
                        startnum=0;
                    }
                    if(userExp[i]>='0' && userExp[i]<='9' && !decimal){
                        startnum=1;
                        carrynum*=10;
                        carrynum+=(userExp[i]-'0');
                    }
                    if(userExp[i]>='0' && userExp[i]<='9' && decimal){
                        startnum=1;
                        carrynum+=((1.0*(userExp[i]-'0'))/divider);
                        divider*=10;
                    }
                    if(userExp[i]=='.'){
                        startnum=1;
                        decimal=1;
                    }
                    printf("%f, %f, %c\n", prevNum, carrynum, userExp[i]);
                    if(userExp[i]=='\0')break;
                }
            }
            if(userExp[recvSize-1]=='\0'){
                if(startnum!=0){
                    prevNum = getOpAns(prevNum, carrynum, prevop);
                }
                float answer=prevNum;
                send(newsockfd, (char*)&answer, sizeof(answer), 0);
                prevop='+';
                prevNum=0;
                outerop='+';
                outerNum=0;
                startnum=0;
                carrynum=0;
                decimal=0;
                divider=10;
                printf("Fu %f\n", answer);
            }
            // if recsize == 0 then client socket is closed
            if(recvSize<=0){
                break;
            }
            // assuming final floating point number has maximum lenght of BUF_SIZE, as
            // more than 10^BUF_SIZE float can't handle
        }

		close(newsockfd);
	}
    close(sockfd);
	return 0;
}


/*

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUF_SIZE 40

// commented code for only one paranthesis, 
// uncommented for any number of parantheses

typedef struct{
    char *arr;
    int size;
}charstack;

typedef struct{
    float *arr;
    int size;
}floatstack;

typedef struct {
    char *data; 
    int capacity; 
    int size; 
} String;


void init(String *s) {
    s->capacity = 1; 
    s->size = 0; 
    s->data = malloc(s->capacity); 
}

void push_back(String *s, char c) {
    if (s->size == s->capacity) {
        s->capacity*=2;
        s->data = realloc(s->data,s->capacity);
    }
    s->data[s->size] = c; 
    s->size++; 
}


void getOpAns(charstack *operator_stack, floatstack *operand_stack){
    float op1 = operand_stack->arr[operand_stack->size-1];
    float op2 = operand_stack->arr[operand_stack->size-2];
    char op = operator_stack->arr[operator_stack->size-1];
    operand_stack->size-=2;
    operator_stack->size--;

    if(op=='+'){
        operand_stack->arr[operand_stack->size++] = op1+op2;
    }
    else if(op=='-'){
        operand_stack->arr[operand_stack->size++] = op2-op1;
    }
    else if(op=='*'){
        operand_stack->arr[operand_stack->size++] = op1*op2;
    }
    else if(op=='/'){
        operand_stack->arr[operand_stack->size++] = op2/op1;
    }
}


// assuming valid expression -- no divide by zeros and 
// number can be at max 100 chars long as we have float
float evaluateExpression(char *userExp, int len){
    
    charstack operator_stack; // storing binary operators
    floatstack operand_stack; // storing real numbers

    operator_stack.arr = (char*)malloc(len*sizeof(char));
    operand_stack.arr = (float*)malloc(len*sizeof(float));

    operand_stack.size=0;
    operator_stack.size=0;

    char* exp;
    exp=(char*)malloc(sizeof(char)*len);

    len=0;
    // removing spaces
    for(int i=0;i<strlen(userExp);++i){
        if(userExp[i]==' ')continue;
        exp[len++]=userExp[i];
    }
    exp[len]='\0';

    for(int i=0;i<len;++i){

        char c = exp[i];
        // checking if the current character is a digit, or - preceeded with some operator
        // in that case we can scan the string till we get next operator
        // add the number in floatstack
        if((i==0 && c=='-') || (i && (exp[i-1]<'0' || exp[i-1]>'9') && exp[i-1]!=')' && c=='-') || (c<='9' && c>='0') || c=='.'){
            char num[len+1];
            int numsize=0;
            int sign=1;
            if(c=='-')sign=-1;

            for(int j=i+(sign==-1);;++j){
                if(j==len){
                    i=len-1;
                    break;
                }
                if(exp[j]=='+' || exp[j]=='-' || exp[j]=='*' || exp[j]=='/' || exp[j]==')'){
                    i=j-1;
                    break;
                }
                num[numsize++]=exp[j];
            }
            num[numsize]='\0';
            operand_stack.arr[operand_stack.size++]=sign*atof(num);
        }
        // add ( to operator_stack
        else if(c=='('){
            operator_stack.arr[operator_stack.size++]=c;
        }
        // pop top 2 elements of operand_stack and top element of operator_stack 
        // and do operation and push to operand_stack
        // do this till we encounter (
        else if(c==')'){
            while(operator_stack.arr[operator_stack.size-1]!='('){
                getOpAns(&operator_stack, &operand_stack);
            }
            operator_stack.size--;
        }
        // pop top 2 elements of operand_stack and top element of operator_stack 
        // and do operation and push to operand_stack
        // do this till we encounter ( or the operator_stack is empty
        else if(c=='+' || c=='-' || c=='*' || c=='/'){
            while(operator_stack.size!=0 && operator_stack.arr[operator_stack.size-1]!='('){
                getOpAns(&operator_stack, &operand_stack);
            }
            operator_stack.arr[operator_stack.size++]=c;
        }

    }

    // pop top 2 elements of operand_stack and top element of operator_stack 
    // and do operation and push to operand_stack
    // do this till operator_stack is empty
    while(operator_stack.size!=0){
        getOpAns(&operator_stack, &operand_stack);
    }

    return operand_stack.arr[operand_stack.size-1];
}

int main()
{
	int sockfd, newsockfd ; //socket descriptors
	unsigned int clilen;
	struct sockaddr_in	cli_addr, serv_addr;	

    char userExp[BUF_SIZE];	// will use this as buffer for communication, taking it to be BUF_SIZE, 
                        // since typical lengths will be less than this
    String buf; // we will use this to store whole string 

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(20000);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		printf("bind to local address failed\n");
		return -1;
	}

	listen(sockfd, 5); 

	while (1) {

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,&clilen) ;

		if (newsockfd < 0) {
			printf("Accept error\n");
			return -1;
		}

        while(1){
            // recieve till socket is closed in client
            int recvSize;
            init(&buf);
            // if last char is '\0' of recieved part then break
            // since it means we already got complete string
            while((recvSize = recv(newsockfd, userExp, BUF_SIZE, 0))>0){
                for(int i=0;i<recvSize;++i){
                    push_back(&buf,userExp[i]);
                }
                if(userExp[recvSize-1]=='\0')break;
            }
            push_back(&buf, '\0');

            // if recsize == 0 then client socket is closed
            if(recvSize<=0){
                break;
            }
            
            // assuming final floating point number has maximum lenght of BUF_SIZE, as
            // more than 10^BUF_SIZE float can't handle
            float answer= evaluateExpression(buf.data,strlen(buf.data));
            send(newsockfd, (char*)&answer, sizeof(answer), 0);
        }

		close(newsockfd);
	}
    close(sockfd);
	return 0;
}

*/	
