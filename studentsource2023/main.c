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

void* connection_routine(void* args)
{
    //smething happens around here that makes evrything crash :/
    printf("routine\n");
    connection * a = (connection *) args;
    int port = a->port;
    int max_conn = a->max_conn;
    conmgr_init(max_conn,port,buffer);
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
    //pthread_t datamgr;
    //pthread_t storagemgr;

    conmgr_init(MAX_CONN,c->port,buffer);

    pthread_create(&connmgr,NULL,&connection_routine,&c);
    printf("connection manager\n");

    pthread_join(connmgr,NULL);
    sbuffer_free(&buffer);
    free(c);
}
