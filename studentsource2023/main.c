#include <stdio.h>
#include <stdlib.h>
#include "sbuffer.h"
#include "connmgr.h"
#include "pthread.h"
#include "datamgr.h"
#include "lib/tcpsock.h"
sbuffer_t * buffer;


typedef struct
{
    int port;
    int max_conn;
    sbuffer_t * buff;
}connection;

void * storagemgr_init(void * args)
{
    sensor_data_t * d = malloc(sizeof(sensor_data_t));
    sbuffer_t * b = (sbuffer_t *)args;
    while(sbuffer_remove(b,d,0) != SBUFFER_NO_DATA)
    {
        printf("storagemgr removed element\n");
    }
    free(d);
    return NULL;
}

int main(int argc, char *argv[])
{
    if(argc < 3) {
        printf("Please provide the right arguments: first the port, then the max nb of clients");
        return -1;
    }
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

    sbuffer_free(&buffer);
    free(c);
    c = NULL;

    return 0;
}
