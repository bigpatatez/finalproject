#include <stdio.h>
#include <stdlib.h>
#include "sbuffer.h"
#include "connmgr.h"
#include "pthread.h"
#include "lib/tcpsock.h"
sbuffer_t * buffer;

typedef struct
{
    int port;
    int max_conn;
    sbuffer_t * buff;
}connection;

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
    //pthread_t datamgr;
    //pthread_t storagemgr;

    pthread_create(&connmgr,NULL,&conmgr_init,c);
    printf("connection manager\n");

    int result = pthread_join(connmgr,NULL);
    if(result !=0)
    {
        printf("thread not joined correctly");
    }
    sbuffer_free(&buffer);
    free(c);
    c = NULL;
    return 0;
}
