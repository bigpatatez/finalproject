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

#ifndef TIMEOUT
#define TIMEOUT 5
#endif


typedef struct {
    tcpsock_t * client;
    int max_conn;
}args;
int server_conn_counter = 0 ;
pthread_mutex_t counter;

pthread_cond_t everyoneHere;
int client_conn_counter = 0 ;
sbuffer_t * b;



void * conmgr_routine(void * param)
{
    tcpsock_t * client ;
    args* a1 = (args*) param;
    client = a1->client;
    //int max_conn = a1->max_conn;

    sensor_data_t data;
    int bytes, result;

    pthread_mutex_lock(&counter);
    client_conn_counter++;
    printf("client conn counter: %d\n",client_conn_counter);
    pthread_mutex_unlock(&counter);


    do {

        // read sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *) &data.id, &bytes);
        // read temperature
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *) &data.value, &bytes);
        // read timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *) &data.ts, &bytes);
        if ((result == TCP_NO_ERROR) && bytes) {
            sbuffer_insert(b,&data);
            //printf("inserted in buffer");
            //printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
                   //(long int) data.ts);

        }
    } while (result == TCP_NO_ERROR); // add timeout condition

    if (result == TCP_CONNECTION_CLOSED)
        printf("Peer has closed connection\n");
    else
        printf("Error occured on connection to peer\n");

    tcp_close(&client);

    //if(server_conn_counter == max_conn)
    {
        pthread_mutex_lock(&counter);
        client_conn_counter--;
        printf("signaling main, client connection counter: %d\n",client_conn_counter);
        if(client_conn_counter == 0){pthread_cond_signal(&everyoneHere);}
        pthread_mutex_unlock(&counter);
    }
    return NULL ;
}


int conmgr_init(int argc, char *argv[],sbuffer_t * buffer) {
    tcpsock_t *server, *client;
    args a ;
    
    if(argc < 3) {
    	printf("Please provide the right arguments: first the port, then the max nb of clients");
    	return -1;
    }
    
    int MAX_CONN = atoi(argv[2]);
    a.max_conn = MAX_CONN;
    int PORT = atoi(argv[1]);

    b = buffer;
    pthread_t threadClient[MAX_CONN];

    pthread_mutex_init(&counter,NULL);

    pthread_cond_init(&everyoneHere,NULL);

    printf("Test server is started\n");
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    do {
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) exit(EXIT_FAILURE); // we can implement a condition variable here?
        // server not blocked anymore
        printf("Incoming client connection\n");
        server_conn_counter++;
        printf("%d\n",server_conn_counter);
        a.client = client;
        pthread_create(&threadClient[server_conn_counter],NULL,&conmgr_routine,&a);


    } while (server_conn_counter < MAX_CONN);



    while(client_conn_counter !=0 && server_conn_counter == MAX_CONN)
    {
        pthread_cond_wait(&everyoneHere,&counter);
    }
    printf("no more connections left\n");

    for (int i = 1; i < MAX_CONN; ++i) {
        int join_result = pthread_join(threadClient[i], NULL);
        if (join_result != 0) {
            fprintf(stderr, "Error joining thread: %d\n", join_result);
            exit(EXIT_FAILURE);
        }

    }
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("Test server is shutting down\n");

    return 0;
}




