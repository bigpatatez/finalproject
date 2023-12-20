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
    int datamgr;
    int storagemgr;
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */
};

pthread_cond_t buffer_filled;
pthread_mutex_t buff;
pthread_mutex_t insert;
int eof = 0 ;

int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    pthread_cond_init(&buffer_filled,NULL);
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

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data, int read) {

    if (buffer == NULL) return SBUFFER_FAILURE;

    pthread_mutex_lock(&buff);
    while (buffer->head == NULL) { // if the buffer is empty
        if(eof == 1)
        {
            return SBUFFER_NO_DATA;
        }
        pthread_cond_wait(&buffer_filled,&buff);
        printf("got signal from insert\n");
    }

    sbuffer_node_t *dummy;
    dummy = buffer->head;
    *data = buffer->head->data;

    if(read == 1)
    {
        while(dummy->datamgr == 1) // if the flag is set
        {
            while(dummy->next == NULL)// if there is no next one then we must wait for something to be inserted into the buffer
            {
                pthread_cond_wait(&buffer_filled,&buff);
            }
            dummy = dummy->next;// the dummy is the next one
        }
        *data = dummy->data;
        dummy->datamgr = 1;
    }
    else
    {
        while(dummy->storagemgr == 1) // if the flag is set
        {
            while(dummy->next == NULL)// if there is no next one then we must wait for something to be inserted into the buffer
            {
                pthread_cond_wait(&buffer_filled,&buff);
            }
            dummy = dummy->next;// the dummy is the next one
        }
        *data = dummy->data;
        dummy->storagemgr = 1;
    }
    if(dummy->storagemgr == 1 && dummy->datamgr == 1)
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
        pthread_mutex_unlock(&buff);
        return SBUFFER_SUCCESS;
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
    dummy->datamgr = 0;
    dummy->storagemgr = 0;
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


