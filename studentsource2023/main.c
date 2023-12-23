#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "sbuffer.h"
#include "connmgr.h"
#include "pthread.h"
#include "datamgr.h"
#include "lib/tcpsock.h"
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
    /*if(strcmp(msg,"child") == 0)
    {
        // this needs to change
        close(fd[1]);
        read(fd[0],received,sizeof(received));
        char * timestamp;

        time_t ltime;
        ltime = time(NULL);
        timestamp = asctime(localtime(&ltime));
        timestamp[strlen(timestamp)-1] = '\0';

        char buffer[100];
        snprintf(buffer,sizeof(buffer),"%d - %s - %s\n",seq,timestamp,received);
        printf("\nwriting to file: %s",buffer);
        fflush(stdout);
        fprintf(f, "%s",buffer);
        printf("wrote to file\n");
        fflush(stdout);
        seq++;
        return 2;
    }*/

    char send[1024];
    char * timestamp;
    pthread_mutex_lock(&logging);
    time_t ltime;
    ltime = time(NULL);
    timestamp = asctime(localtime(&ltime));
    snprintf(send,sizeof(send),"%d - %s - %s\n",seq,timestamp,msg);
    //strncpy(send,msg,sizeof(send));
    printf("sending message\n");
    write(fd[1],send,sizeof(send));
    printf("sent message\n");
    seq++;
    pthread_mutex_unlock(&logging);
    /*if(strcmp(msg,"Data file is closed") == 0)
    {
        wait(NULL);
        close(fd[1]);
    }*/
    return 1;

}

void logger()
{
    char received;
    FILE* f = fopen("gateway.log","w");
    // receive messages from the other threads
    close(fd[1]);
    // this part needs to change
    while(read(fd[0],&received,1))
    {
        fwrite(&buffer,1,1,f);
        fflush(f);
    }
    close(fd[0]);
    fclose(f);
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

        // can implement an array and check whether all threads joined correctly
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
        printf("connection manager and data manager joined\n");

        wait(NULL);
        close(fd[1]);
        sbuffer_free(&buffer);
        free(c);
        c = NULL;
    }


    /**TODO:  need to test the storage manager and the datamanager with multiple clients--> test edge cases as much as possible
     *             implement storage manager and dont forget to flush
     *             implement the logger -> use the functions inside main?? think ab it
     */


    return 0;
}
