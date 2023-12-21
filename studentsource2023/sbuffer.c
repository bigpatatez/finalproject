/**
 * \author {AUTHOR}
 */

#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"
#include <pthread.h>

/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;/**< a structure containing the data */
    int readers;  /**< an int to keep track of the readers that read the node */
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */
    int index[2];               /**< an array to keep track of where the different readers are in the buffer*/
};

pthread_cond_t remove_cond;
pthread_cond_t buffer_filled;
pthread_mutex_t buff;
pthread_mutex_t insert;
int eof = 0 ;
int readers = 0;
int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    (*buffer)->index[0] = 0;
    (*buffer)->index[1] = 0;
    pthread_cond_init(&buffer_filled,NULL);
    pthread_cond_init(&remove_cond,NULL);
    pthread_mutex_init(&buff,NULL);
    pthread_mutex_init(&insert,NULL);
    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {

    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }
    while ((*buffer)->head) {
        sbuffer_node_t *dummy;
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data, int id) {

    if (buffer == NULL) return SBUFFER_FAILURE;

    pthread_mutex_lock(&buff);
    // get the element based on the index
    while (buffer->head == NULL) { // if the buffer is empty
        /*if(eof == 1)
        {
            pthread_mutex_unlock(&buff);
            return SBUFFER_NO_DATA;
        }*/
        pthread_cond_wait(&buffer_filled,&buff);
        printf("got signal from insert\n");
    }

    sbuffer_node_t *dummy;
    dummy = buffer->head;
    *data = buffer->head->data;
    readers++;

    if(readers == 2)
    {
        if (buffer->head == buffer->tail) // buffer has only one node
        {
            buffer->head = buffer->tail = NULL;
            if (data->id == 0) {
                free(dummy);
                printf("end of transmission detected\n");
                eof = 1;
                pthread_mutex_unlock(&buff);
                return SBUFFER_NO_DATA;
            }
        } else  // buffer has many nodes empty
        {
            buffer->head = buffer->head->next;
        }
        free(dummy);
        pthread_cond_signal(&remove_cond);
        pthread_mutex_unlock(&buff);
        return SBUFFER_SUCCESS;
    }
    else
    {
        pthread_cond_wait(&remove_cond,&buff);
    }

    pthread_mutex_unlock(&buff);
    return SBUFFER_SUCCESS;

}


int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) return SBUFFER_FAILURE;
    pthread_mutex_lock(&insert);
    dummy->data = *data;
    dummy->next = NULL;

    if (buffer->tail == NULL) // buffer empty (buffer->head should also be NULL
    {
        buffer->head = buffer->tail = dummy;
    }
    else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }
    printf("buffer is filled with something: signaling remove\n");
    pthread_cond_broadcast(&buffer_filled);
    pthread_mutex_unlock(&insert);
    return SBUFFER_SUCCESS;
}
sbuffer_node_t * get_node_at_index(sbuffer_t* buffer, int id)
{
    if(buffer ==NULL){return NULL;}
    if(buffer->head == NULL){return NULL;}
    sbuffer_node_t * dummy = buffer->head;
    int index = buffer->index[id];
    for(int i = 0; )
    return NULL;
}


