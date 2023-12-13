/**
 * \author {AUTHOR}
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "lib/tcpsock.h"
#include <pthread.h>
#include <unistd.h>
#include "connmgr.h"
#include "sbuffer.h"
#include <signal.h>
#include <bits/types/struct_timeval.h>
#include <sys/socket.h>

#ifndef TIMEOUT
#define TIMEOUT 5
#endif

typedef struct
{
    int port;
    int max_conn;
    sbuffer_t* b;
}mainArgs;

//typedef struct {


//}args ;

int server_conn_counter = 0 ;
pthread_mutex_t counter;

pthread_cond_t everyoneHere;
int client_conn_counter = 0 ;
int MAX_CONN;
sbuffer_t* b;

void * conmgr_routine(void * param)
{
    tcpsock_t * client ;
    client = (tcpsock_t*)param;

    int sd ;
    tcp_get_sd(client,&sd);

    sensor_data_t data;
    int bytes, result;

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
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

        if ((result == TCP_NO_ERROR) && bytes && data.id != 0 && data.ts >= 0 && data.value >= 0) {
            sbuffer_insert(b,&data);
            printf("%d\n",setsockopt(sd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof timeout));
            printf("inserted in buffer\n");
            printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
                   (long int) data.ts);
        }
    } while (result == TCP_NO_ERROR  );
    // add timeout condition from tcp socket library --> you should implement it there because if you do it here it keeps reading while receiving the timeout

    if (result == TCP_CONNECTION_CLOSED)
        printf("Peer has closed connection\n");
    else
    {
        printf("Error occured on connection to peer\n");
        printf("Error code : %d\n",result);
    }

    tcp_close(&client);

    pthread_mutex_lock(&counter);
    client_conn_counter--;
    printf("client connection counter: %d\n",client_conn_counter);
    printf("server connection counter: %d\n",server_conn_counter);
    if(client_conn_counter==0)
    {
        pthread_cond_signal(&everyoneHere);
    }
    printf("signaled main\n");
    fflush(stdout);
    pthread_mutex_unlock(&counter);

    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    return NULL ;
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
    pthread_cond_init(&everyoneHere,NULL);

    printf("Test server is started\n");
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR)exit(EXIT_FAILURE);
    do {
        printf("in the loop");
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR)exit(EXIT_FAILURE);
        printf("Incoming client connection\n");
        pthread_mutex_lock(&counter);
        client_conn_counter++;
        server_conn_counter++;
        printf("client conn counter: %d\n",client_conn_counter);
        printf("server conn counter: %d\n",server_conn_counter);
        pthread_mutex_unlock(&counter);
        pthread_create(&threadClient[server_conn_counter],NULL,&conmgr_routine,(void*)client);

    } while (server_conn_counter < MAX_CONN);


    //while(client_conn_counter != 0)
    {
        //printf("waiting");
        pthread_cond_wait(&everyoneHere,&counter);
    }
    printf("no more connections left\n");
    fflush(stdout);

    for (int i = 1; i < MAX_CONN; ++i) {
        int join_result = pthread_join(threadClient[i], NULL);
        if (join_result != 0) {
            fprintf(stderr, "Error joining thread: %d\n", join_result);
            exit(EXIT_FAILURE);
        }
    }

    if (tcp_close(&server) != TCP_NO_ERROR)
    {
        printf("error while closing tcp conn\n");
        exit(EXIT_FAILURE);
    }

    printf("Test server is shutting down\n");
    pthread_mutex_destroy(&counter);
    pthread_cond_destroy(&everyoneHere);


    return 0;
}




