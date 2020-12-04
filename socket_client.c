#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include <errno.h>
#include <pthread.h>
#define PORT 9242

typedef struct CThread CThread;
struct CThread {
    pthread_t thread_ID;
    void *thread_func;
};

static int connected = 0;
static char* IPv4 = "127.0.0.1";
static int sockfd = -2;
static CThread thread_send;
static CThread thread_recv;

void disconnect(){
    if(sockfd != -2)
        close(sockfd);
    connected = 0;
    printf("\nDéconnexion !\n");
}

void* sendThread(void* voidptr) {
    int buffer_len = 9999;
    char buffer[buffer_len];
    while(connected){
        fgets(&buffer[0], buffer_len, stdin);
        if(buffer[0] == '\0') continue;
        int ret = send(sockfd, &buffer[0], strlen(&buffer[0]), 0);
        if(ret == -1) {
            printf("\nErreur send() : %d\n", errno);
            break;
        }
        memset(&buffer[0],0,strlen(&buffer[0]));
    }
    disconnect();
    return (void*)NULL;
}

void* recvThread(void *voidptr){
    int buffer_len = 1000;
    char buffer[buffer_len];
    while(connected){
        int ret = read(sockfd, &buffer[0], buffer_len*sizeof(char));
        if(ret < 0) break;
        if(buffer[0] == '\0') continue;
        printf("\n%s\n", &buffer[0]);
        memset(&buffer[0],0,strlen(&buffer[0]));
    }
    disconnect();
    return (void*)NULL;
}

int createThread(CThread* thread){
    int retc, retd;
    while(1){
        retc = pthread_create(&thread->thread_ID, NULL, thread->thread_func, (void*)NULL);
        retd = pthread_detach(thread->thread_ID);
        if(retc != 0 || retd != 0){
            printf("Impossible de créer le Thread ...\n");
            return -1;
        }
        printf("Thread n°%ld créé !\n", thread->thread_ID);
        break;
    }
    return 0;
}
   
int main(int argc, char* argv[]) {
    struct sockaddr_in serv_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error: %d\n", errno); 
        exit(EXIT_FAILURE);
    } 
    printf("Socket créé\n");
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT);
       
    if(inet_pton(AF_INET, IPv4, &serv_addr.sin_addr) <= 0)  {
        printf("\nInvalid address / Address not supported\n"); 
        exit(EXIT_FAILURE);
    }
    printf("structure créé\n");
    while(1){
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
            printf("\nConnection Failed ! error: %d\n", errno);
            printf("Reconnexion dans 5 secondes...\n");
            sleep(5);
            continue;
        }
        connected = 1;
        printf("Connecté au serveur %s !\n", IPv4);
        break;
    }

    thread_send.thread_ID = 0;
    thread_send.thread_func = &sendThread;
    thread_recv.thread_ID = 0;
    thread_recv.thread_func = &recvThread;

    if(createThread(&thread_send) < 0) {
        exit(EXIT_FAILURE);
    }
    if(createThread(&thread_recv) < 0) {
        exit(EXIT_FAILURE);
    }
    while(1) sleep(100);
    return 0; 
} 
