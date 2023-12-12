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


typedef struct {
    tcpsock_t * client;
    sbuffer_t* b;
}args;


int server_conn_counter = 0 ;
pthread_mutex_t counter;

pthread_cond_t everyoneHere;
int client_conn_counter = 0 ;


void * conmgr_routine(void * param)
{
    tcpsock_t * client ;
    sbuffer_t * buffer;
    args* a1 = (args*) param;
    client = a1->client;
    buffer = a1->b;
    int sd ;
    tcp_get_sd(client,&sd);

    sensor_data_t data;
    int bytes, result;

    pthread_mutex_lock(&counter);
    client_conn_counter++;
    printf("client conn counter: %d\n",client_conn_counter);
    pthread_mutex_unlock(&counter);

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    do {
        printf("receiving\n");
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
            printf("yes");
            sbuffer_insert(buffer,&data);
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
        printf("%d",result);
    }

    tcp_close(&client);

    pthread_mutex_lock(&counter);
    client_conn_counter--;
    printf("signaling main, client connection counter: %d\n",client_conn_counter);
    if(client_conn_counter == 0){pthread_cond_signal(&everyoneHere);}
    pthread_mutex_unlock(&counter);

    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    return NULL ;
}


int conmgr_init(int max_conn, int port,sbuffer_t* buff) {
    tcpsock_t *server, *client;
    args * a = malloc(sizeof(args));
    a ->b = buff;
    int PORT = port;

    pthread_t threadClient[max_conn];

    pthread_mutex_init(&counter,NULL);

    pthread_cond_init(&everyoneHere,NULL);

    printf("Test server is started\n");
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    do {
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) exit(EXIT_FAILURE); // we can implement a condition variable here?
        printf("Incoming client connection\n");
        server_conn_counter++;
        printf("%d\n",server_conn_counter);
        a->client = client;
        pthread_create(&threadClient[server_conn_counter],NULL,&conmgr_routine,&a);

    } while (server_conn_counter < max_conn);


    while(client_conn_counter !=0 && server_conn_counter == max_conn)
    {
        pthread_cond_wait(&everyoneHere,&counter);
    }
    printf("no more connections left\n");

    for (int i = 1; i < max_conn; ++i) {
        int join_result = pthread_join(threadClient[i], NULL);
        if (join_result != 0) {
            fprintf(stderr, "Error joining thread: %d\n", join_result);
            exit(EXIT_FAILURE);
        }
    }

    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("Test server is shutting down\n");
    free(a);
    return 0;
}




