#include <stdio.h>
#include <pthread.h>


void* runner(void *arg){

    int tid = *(int *)arg;
    // int tid = (long)arg;

    printf("Hello %d\n", tid);

    // int *a = (int *)arg;
    // printf("a[0], a[1], %d, %d\n", a[0], a[1]);
    // a[0]++;

    int x = 2;
    int arr[]={3, 4, 6};
    pthread_exit((void *)x);
}

int main(){

    pthread_t p1, p2;
    int arg=5;

    int a[] = {5, 6};
    pthread_create(&p1, NULL, runner, &arg);
    // pthread_create(&p1, NULL, runner, a);
    void *status;
    pthread_join(p1, &status);
    // int *arr = (int *) status;
    printf("Status %ld\n", (long)status);
    // printf("Arr %d\n", arr[0]);
    printf("a[0], a[1], %d, %d\n", a[0], a[1]);

}