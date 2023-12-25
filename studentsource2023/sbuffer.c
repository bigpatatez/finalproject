/**
 * \author {AUTHOR}
 */

#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"
#include <pthread.h>
#include <unistd.h>


/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;/**< a structure containing the data */
    int r_data;
    int r_storage;
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */
};

pthread_cond_t peek_cond;
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
    pthread_cond_init(&peek_cond,NULL);
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

int sbuffer_peek(sbuffer_t *buffer, sensor_data_t* data)
{
    if (buffer == NULL) return SBUFFER_FAILURE;
    pthread_mutex_lock(&buff);
    while (buffer->head == NULL || buffer->head->r_data == 1) { // if the buffer is empty
        pthread_cond_wait(&buffer_filled,&buff);
    }
    printf("got signal from insert\n");

    *data = buffer->head->data;
    buffer->head->r_data = 1;
    if(data->id == 0)
    {
        eof = 1;
        printf("end of transmission data\n");
        pthread_cond_signal(&buffer_filled);
        pthread_mutex_unlock(&buff);
        return SBUFFER_NO_DATA;
    }
    pthread_cond_signal(&buffer_filled);
    pthread_mutex_unlock(&buff);
    return SBUFFER_SUCCESS;
/*if (buffer == NULL) return SBUFFER_FAILURE;
    pthread_mutex_lock(&buff);
    while(buffer->head ==NULL || buffer->head->r_data == 1)
    {
        pthread_cond_wait(&buffer_filled,&buff);
        printf("got signal from insert or storage manager removed the node\n");
    }
    *data = buffer->head->data;
    buffer->head->r_data = 1;
    pthread_cond_signal(&buffer_filled);
    if(data->id ==0)
    {
        eof = 1 ;
        pthread_mutex_unlock(&buff);
        return SBUFFER_NO_DATA;
    }
    pthread_mutex_unlock(&buff);
    return SBUFFER_SUCCESS;*/
}

int sbuffer_remove_and_peek(sbuffer_t* buffer, sensor_data_t* data)
{
    if(buffer == NULL) return SBUFFER_FAILURE;
    pthread_mutex_lock(&buff);
    if(eof == 1)
    {
        printf("storage end of file\n");
        pthread_mutex_unlock(&buff);
        return SBUFFER_NO_DATA;
    }
    while(buffer->head == NULL ||buffer->head->r_data == 0)
    {
        pthread_cond_wait(&buffer_filled,&buff);
    }
    sbuffer_node_t *dummy;
    dummy = buffer->head;
    *data = buffer->head->data;
    if(data->id ==0)
    {
        printf("end of transmission storage\n");
        //pthread_cond_signal(&buffer_filled);
        pthread_mutex_unlock(&buff);
        return SBUFFER_NO_DATA;
    }
    if (buffer->head == buffer->tail) // buffer has only one node
    {
        buffer->head = buffer->tail = NULL;

    } else  // buffer has many nodes empty
    {
        buffer->head = buffer->head->next;
    }
    free(dummy);
    pthread_cond_signal(&buffer_filled);
    pthread_mutex_unlock(&buff);
    return SBUFFER_SUCCESS;

    /*pthread_mutex_lock(&buff);
    while(buffer->head == NULL || buffer->head->r_data == 0)
    {
        pthread_cond_wait(&buffer_filled,&buff);
        printf("data inserted in buffer or datamanager read the node\n");
    }
    *data = buffer->head->data;
    sbuffer_node_t *dummy;
    dummy = buffer->head;
    if (buffer->head == buffer->tail) // buffer has only one node
    {
        buffer->head = buffer->tail = NULL;

    } else  // buffer has many nodes empty
    {
        buffer->head = buffer->head->next;
    }
    free(dummy);
    if(eof == 1)
    {
        pthread_mutex_unlock(&buff);
        return SBUFFER_NO_DATA;
    }
    printf("storage manager: freed the node\n");
    return SBUFFER_SUCCESS;*/
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data, int id) {

   /* if (buffer == NULL) return SBUFFER_FAILURE;
    if(id ==0)
    {
        pthread_mutex_lock(&buff);
        while (buffer->head == NULL) { // if the buffer is empty
            if(eof == 1)
            {
                pthread_mutex_unlock(&buff);
                return SBUFFER_NO_DATA;
            }
            pthread_cond_wait(&buffer_filled,&buff);
            printf("got signal from insert\n");
        }
        *data = buffer->head->data;
        if(data->id == 0)
        {
            eof = 1;
            pthread_cond_signal(&peek_cond);
            pthread_mutex_unlock(&buff);
            return SBUFFER_NO_DATA;
        }
        pthread_cond_signal(&peek_cond);
        while(1)
        {
            pthread_cond_wait(&peek_cond,&buff);
            break;
        }
        pthread_mutex_unlock(&buff);
        return SBUFFER_SUCCESS;
    }
    else
    {
        pthread_mutex_lock(&buff);
        while(1)
        {
            pthread_cond_wait(&peek_cond,&buff);
            break;
        }
        if(eof ==1)
        {
            pthread_mutex_unlock(&buff);
            return SBUFFER_NO_DATA;
        }

        sbuffer_node_t *dummy;
        dummy = buffer->head;
        *data = buffer->head->data;

        if (buffer->head == buffer->tail) // buffer has only one node
        {
            buffer->head = buffer->tail = NULL;

        } else  // buffer has many nodes empty
        {
            buffer->head = buffer->head->next;
        }

        free(dummy);
        pthread_cond_signal(&peek_cond);
        pthread_mutex_unlock(&buff);
        return SBUFFER_SUCCESS;
    }*/
  return SBUFFER_SUCCESS;
}


int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {

    if (buffer == NULL) return SBUFFER_FAILURE;
    pthread_mutex_lock(&insert);
    sbuffer_node_t *dummy;
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) return SBUFFER_FAILURE;
    dummy->data = *data;
    dummy->next = NULL;
    dummy->r_data = 0;
    dummy->r_storage =0;
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

    pthread_mutex_unlock(&insert);
    pthread_cond_broadcast(&buffer_filled);
    return SBUFFER_SUCCESS;
}

sbuffer_node_t * get_node_at_index(sbuffer_t* buffer, int index)
{
    if(buffer ==NULL){return NULL;}
    if(buffer->head == NULL){return NULL;}
    sbuffer_node_t * dummy = buffer->head;
    if (index == 0)
    {
        return dummy;
    }
    for(int i = 1; i <= index && dummy != NULL; i++)
    {
        if(dummy->next != NULL)
        {
            dummy = dummy->next;
        }
        else{return NULL;}
    }
    return dummy;
}


