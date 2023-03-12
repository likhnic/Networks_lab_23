#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int calLen(void *buf){
    return strlen(buf);
}

int main(){

    // int x = 64;
    // void *p = &x;

    char *c="Hello";
    void *buf = c;
    // printf("Strlen %ld\n", strlen(p));
    // printf("Got %c\n", *((char *)(p)));
    printf("%d\n", calLen(c));
}