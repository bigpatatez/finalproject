/**
 * \author {AUTHOR}
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "lib/tcpsock.h"
#include <pthread.h>
#include "connmgr.h"
#include "sbuffer.h"

#ifndef TIMEOUT
#define TIMEOUT 5
#endif

typedef struct
{
    int port;
    int max_conn;
    sbuffer_t* b;
}mainArgs;

int server_conn_counter = 0 ;
pthread_mutex_t counter;


int client_conn_counter = 0 ;
int MAX_CONN;
sbuffer_t* b;

void * conmgr_routine(void * param)
{
    tcpsock_t * client ;
    client = (tcpsock_t*)param;

    sensor_data_t data;
    int bytes, result;
    int i = 0;
    do {
        // read sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *) &data.id, &bytes);
        //read temperature
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *) &data.value, &bytes);
        // read timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *) &data.ts, &bytes);

        if(i == 0)
        {
            pthread_mutex_lock(&counter);
            char string[500];
            snprintf(string,sizeof(string),"Sensor node %d has opened a new connection",data.id);
            write_to_log_process(string);
            i = 1;
            pthread_mutex_unlock(&counter);
        }
        if ((result == TCP_NO_ERROR) && bytes) {
            printf("Got from client:\n sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
                   (long int) data.ts);
            sbuffer_insert(b,&data);
        }
    } while (result == TCP_NO_ERROR  );

    if (result == TCP_CONNECTION_CLOSED)
    {
        pthread_mutex_lock(&counter);
        printf("Peer has closed connection\n");
        char string[500];
        snprintf(string,sizeof(string),"Sensor node %d has closed the connection",data.id);
        write_to_log_process(string);
        pthread_mutex_unlock(&counter);
    }
    else
    {
        pthread_mutex_lock(&counter);
        printf("Error occured on connection to peer\n");
        printf("Error code : %d\n",result);
        fflush(stdout);
        char string[500];
        snprintf(string,sizeof(string),"An error occurred with connection to sensor node %d",data.id);
        write_to_log_process(string);
        pthread_mutex_unlock(&counter);
    }

    tcp_close(&client);

    return CONMGR_SUCCESS;
}


void * conmgr_init(void* arguments){

    tcpsock_t *server, *client;
    int PORT;
    mainArgs* m ;
    m = (mainArgs*) arguments;

    PORT = m->port;
    printf("%d\n",PORT);
    MAX_CONN = m->max_conn;
    printf("%d\n",MAX_CONN);
    b = m->b;

    pthread_t threadClient[MAX_CONN];
    pthread_mutex_init(&counter,NULL);


    printf("Test server is started\n");
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR)exit(EXIT_FAILURE);
    do {
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR)exit(EXIT_FAILURE);
        printf("Incoming client connection\n");
        pthread_mutex_lock(&counter);
        client_conn_counter++;
        server_conn_counter++;
        pthread_mutex_unlock(&counter);
        pthread_create(&threadClient[server_conn_counter-1],NULL,&conmgr_routine,(void*)client);

    } while (server_conn_counter < MAX_CONN);


    for (int i = 0; i < MAX_CONN; ++i) {
        int join_result = pthread_join(threadClient[i], NULL);
        if (join_result != 0) {
            fprintf(stderr, "Error joining thread: %d\n", join_result);
            exit(EXIT_FAILURE);
        }
        fflush(stdout);
    }

    sensor_data_t data ;
    data.id = 0;
    sbuffer_insert(b,&data);

    if (tcp_close(&server) != TCP_NO_ERROR)
    {
        printf("error while closing tcp connection\n");
        exit(EXIT_FAILURE);
    }

    printf("Test server is shutting down\n");
    pthread_mutex_destroy(&counter);

    return CONMGR_SUCCESS;
}




