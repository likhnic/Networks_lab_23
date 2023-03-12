#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>


typedef struct{
    char *arr;
    int size;
}charstack;

typedef struct{
    float *arr;
    int size;
}floatstack;

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


float evaluateExpression(char *exp, int len){
    
    charstack operator_stack; // storing binary operators
    floatstack operand_stack; // storing real numbers

    operator_stack.arr = (char*)malloc(len*sizeof(char));
    operand_stack.arr = (float*)malloc(len*sizeof(float));

    operand_stack.size=0;
    operator_stack.size=0;

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
	int clilen;
	struct sockaddr_in	cli_addr, serv_addr;	

	char buf[100];   // will store the expression after removing spaces
    char userExp[100];	// will use this as buffer for communication

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
            int recvSize = recv(newsockfd, userExp, 100, 0);
            if(recvSize<=0){
                break;
            }
            int len=0;
            // removing spaces 
            for(int i=0;userExp[i]!='\0';++i){
                if(userExp[i]==' ')continue;
                buf[len++]=userExp[i];
            }
            buf[len]='\0';
            
            sprintf(buf, "%f", evaluateExpression(buf,strlen(buf)));
            int sendSize = send(newsockfd, buf, strlen(buf)+1, 0);
        }

		close(newsockfd);
	}
	return 0;
}
			

