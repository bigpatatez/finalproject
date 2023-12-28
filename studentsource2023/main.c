#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "sbuffer.h"
#include "connmgr.h"
#include "pthread.h"
#include "datamgr.h"
#include "sensor_db.h"

sbuffer_t * buffer;

typedef struct
{
    int port;
    int max_conn;
    sbuffer_t * buff;
}connection;

int fd[2];
int seq = 0;
pthread_mutex_t logging;
int write_to_log_process(char *msg)
{
    pthread_mutex_lock(&logging);
    char send[500] = {0};
    char * timestamp;

    time_t ltime;
    ltime = time(NULL);
    timestamp = asctime(localtime(&ltime));
    timestamp[strlen(timestamp)-1] = '\0';

    snprintf(send,sizeof(send),"%d - %s - %s\n",seq,timestamp,msg);
    //printf("sending message\n");
    write(fd[1],send,sizeof(send));
    printf("sent message\n");
    seq++;
    pthread_mutex_unlock(&logging);
    return 1;

}
int logger()
{
    char received[500] = {0};
    FILE* f = fopen("gateway.log","w");
    close(fd[1]);
    while(read(fd[0],received,500)>0)
    {
        fprintf(f,"%s",received);
        fflush(f);
    }
    printf("logger closing the pipe\n");
    close(fd[0]);
    fclose(f);
    return 0;
}



int main(int argc, char *argv[])
{
    if(argc < 3) {
        printf("Please provide the right arguments: first the port, then the max nb of clients");
        return -1;
    }
    pthread_mutex_init(&logging,NULL);
    if(pipe(fd) == -1)
    {
        perror("Pipe creation failed\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    if(pid == 0)
    {
        logger();
    }
    else{

        close(fd[0]);
        connection * c  = malloc(sizeof(connection));
        c->port = atoi(argv[1]);
        int MAX_CONN = atoi(argv[2]);
        c->max_conn = MAX_CONN;
        sbuffer_init(&buffer);
        c->buff = buffer;

        pthread_t connmgr;
        pthread_t datamgr;
        pthread_t storagemgr;

        pthread_create(&connmgr,NULL,&conmgr_init,c);
        printf("connection manager\n");
        pthread_create(&datamgr,NULL,&datamgr_init,(void*)buffer);
        printf("data manager\n");
        pthread_create(&storagemgr,NULL,&storagemgr_init,(void*)buffer);


        int result = pthread_join(connmgr,NULL);
        if(result !=0)
        {
            printf("connection manager thread not joined correctly\n");
        }
        result = pthread_join(datamgr,NULL);
        if(result !=0)
        {
            printf("data manager thread not joined correctly\n");
        }
        result=pthread_join(storagemgr,NULL);
        if(result !=0)
        {
            printf("storage manager not joined correctly\n");
        }
        printf("all threads joined\n");
        close(fd[1]);
        wait(NULL);
        sbuffer_free(&buffer);
        free(c);
        printf("sensor gateway shutting off\n");
    }
    return 0;
}
